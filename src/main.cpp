// Signal K application template file.
//
// This application demonstrates core SensESP concepts in a very
// concise manner. You can build and upload the application as is
// and observe the value changes on the serial port monitor.
//
// You can use this source file as a basis for your own projects.
// Remove the parts that are not relevant to you, and add your own code
// for external hardware libraries.

#include <FunctionalInterrupt.h>
#include <LiquidCrystal_I2C.h>
#include <Preferences.h>
#include <Wire.h>

#include "sensesp/sensors/analog_input.h"
#include "sensesp/sensors/constant_sensor.h"
#include "sensesp/sensors/digital_input.h"
#include "sensesp/sensors/sensor.h"
#include "sensesp/signalk/signalk_output.h"
#include "sensesp/system/lambda_consumer.h"
#include "sensesp_app_builder.h"

using namespace sensesp;

// void setupGypsyParameter();

reactesp::ReactESP app;
Preferences preferences;

// Mosfet relay drivers

#define DRIVE_UP 15
#define DRIVE_DOWN 12

// Sense lines

#define SENSE_UP 16
#define SENSE_DOWN 13
#define SENSE_GYPSY_PULSE 18

// Buttons

#define UP_BUTTON 32
#define DOWN_BUTTON 23
#define MENU_BUTTON 26

volatile int chainCount = 0;
int lastChainCount = chainCount;
//  int countDir = 1;
// float gypsyCirum_m = 0.45;

unsigned int ignore_interval_ms_ = 10;
elapsedMillis since_last_event_;

LiquidCrystal_I2C lcd(0x3F, 16, 2);

enum gypsyStatus {
  idle = 0,
  up = 1,
  down = 2,
  freefall = 3,
  bound = 4,
  fault = 5
} gypsyStatus;

const char* gypsyStatusDescription[] = {"IDLE    ", "UP      ", "DOWN    ",
                                        "FREEFALL", "BOUND   ", "FAULT   "};

// The gypsyTimer will reset the gypsyState to idle (from any non-fault state)
// if no gypsy pulses are received. A gypsy pulse restarts the timer
static TimerHandle_t gypsyTimer;

void gypsyTimerCallback(xTimerHandle myTimer) {
  // we did not get an expected pulse, check that drive is gone
  if (gypsyStatus != fault) {
    gypsyStatus = idle;
  }
}

void gypsyInterruptHandler(void) {
  boolean driveUp, driveDown;

  if (since_last_event_ > ignore_interval_ms_) {
    since_last_event_ = 0;
    driveUp = digitalRead(SENSE_UP);
    driveDown = digitalRead(SENSE_DOWN);

    // we have a real gypsy pulse, restart the gypsyTimer

    // xTimerResetFromISR(gypsyTimer, &otherTaskWoken);

    if (driveUp & driveDown) {
      gypsyStatus = bound;
    } else {
      if (driveUp) {
        gypsyStatus = up;
        chainCount--;
      } else if (driveDown) {
        gypsyStatus = down;
        chainCount++;
      } else {
        gypsyStatus = freefall;
        chainCount++;
      }
      if (chainCount < 0) {
        chainCount = 0;
      }
    }
  }
}

// The setup function performs one-time application initialization.
void setup() {
  gypsyStatus = idle;

  // initialise drive mosfets

  pinMode(DRIVE_UP, OUTPUT);
  pinMode(DRIVE_DOWN, OUTPUT);

  digitalWrite(DRIVE_DOWN, LOW);
  digitalWrite(DRIVE_UP, LOW);

  // initialise Sense lines

  pinMode(SENSE_UP, INPUT);
  pinMode(SENSE_DOWN, INPUT);
  pinMode(SENSE_GYPSY_PULSE, INPUT);

  // initialise buttons

  pinMode(MENU_BUTTON, INPUT);
  pinMode(UP_BUTTON, INPUT);
  pinMode(DOWN_BUTTON, INPUT);

#ifndef SERIAL_DEBUG_DISABLED
  SetupSerialDebug(115200);
#endif

  Wire.begin();
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);

  // recover previous stored chainCount (just in case the chain counter was
  // powered off with chain out)

  preferences.begin("ChainCounter", false);
  chainCount = preferences.getInt("counter", 0);
  lastChainCount = chainCount;

  // Construct the global SensESPApp() object
  SensESPAppBuilder builder;
  sensesp_app = (&builder)
                    // Set a custom hostname for the app.
                    ->set_hostname("Chain_Counter")
                    // Optionally, hard-code the WiFi and Signal K server
                    // settings. This is normally not needed.
                    //->set_wifi("My WiFi SSID", "my_wifi_password")
                    //->set_sk_server("192.168.10.3", 80)
                    ->get_app();

  // GPIO number to use for the analog input
  const uint8_t kAnalogInputPin = 36;
  // Define how often (in milliseconds) new samples are acquired
  const unsigned int kAnalogInputReadInterval = 500;
  // Define the produced value at the maximum input voltage (3.3V).
  // A value of 3.3 gives output equal to the input voltage.
  const float kAnalogInputScale = 3.3;

  // Create a new Analog Input Sensor that reads an analog input pin
  // periodically.
  auto* analog_input = new AnalogInput(
      kAnalogInputPin, kAnalogInputReadInterval, "", kAnalogInputScale);

  // Add an observer that prints out the current value of the analog input
  // every time it changes.
  analog_input->attach([analog_input]() {
    debugD("Analog input value: %f", analog_input->get());
  });

  // Set GPIO pin 15 to output and toggle it every 650 ms

  const uint8_t kDigitalOutputPin = 2;
  const unsigned int kDigitalOutputInterval = 650;
  pinMode(kDigitalOutputPin, OUTPUT);
  app.onRepeat(kDigitalOutputInterval, [kDigitalOutputPin]() {
    digitalWrite(kDigitalOutputPin, !digitalRead(kDigitalOutputPin));
  });

  // Read GPIO 14 every time it changes

  const uint8_t kDigitalInput1Pin = 14;
  auto* digital_input1 =
      new DigitalInputChange(kDigitalInput1Pin, INPUT_PULLUP, CHANGE);

  // Connect the digital input to a lambda consumer that prints out the
  // value every time it changes.

  // Test this yourself by connecting pin 15 to pin 14 with a jumper wire and
  // see if the value changes!

  digital_input1->connect_to(new LambdaConsumer<bool>(
      [](bool input) { debugD("Digital input value changed: %d", input); }));

  // Create another digital input, this time with RepeatSensor. This approach
  // can be used to connect external sensor library to SensESP!

  const uint8_t kDigitalInput2Pin = 13;
  const unsigned int kDigitalInput2Interval = 1000;

  // Configure the pin. Replace this with your custom library initialization
  // code!
  pinMode(kDigitalInput2Pin, INPUT_PULLUP);

  // Define a new RepeatSensor that reads the pin every 100 ms.
  // Replace the lambda function internals with the input routine of your custom
  // library.

  // Again, test this yourself by connecting pin 15 to pin 13 with a jumper
  // wire and see if the value changes!

  auto* digital_input2 = new RepeatSensor<bool>(
      kDigitalInput2Interval,
      [kDigitalInput2Pin]() { return digitalRead(kDigitalInput2Pin); });

  // Connect the analog input to Signal K output. This will publish the
  // analog input value to the Signal K server every time it changes.
  analog_input->connect_to(new SKOutputFloat(
      "sensors.analog_input.voltage",         // Signal K path
      "/sensors/analog_input/voltage",        // configuration path, used in the
                                              // web UI and for storing the
                                              // configuration
      new SKMetadata("V",                     // Define output units
                     "Analog input voltage")  // Value description
      ));

  // Connect digital input 2 to Signal K output.
  digital_input2->connect_to(new SKOutputBool(
      "sensors.digital_input2.value",          // Signal K path
      "/sensors/digital_input2/value",         // configuration path
      new SKMetadata("",                       // No units for boolean values
                     "Digital input 2 value")  // Value description
      ));

  // create the repeat reaction to increment the chain counter
  /*
  app.onRepeat(400, []() {
    chainCount += countDir;
    if (chainCount == 250 or chainCount == 0) {
      countDir *= -1;
    }
  });
  */

  // Read buttons and control driver
  app.onRepeat(200, []() {
    boolean up_button, down_button, menu_button;

    up_button = !digitalRead(UP_BUTTON);
    down_button = !digitalRead(DOWN_BUTTON);
    menu_button = !digitalRead(MENU_BUTTON);

    boolean drive = up_button;
    digitalWrite(DRIVE_UP, drive);
    drive = down_button;
    digitalWrite(DRIVE_DOWN, drive);
  });

  // setup configurable parameters

  // Create the ConstantSensor object for the Gypsy circumference
  auto* gypsyCircumferenceSensor = new IntConstantSensor(
      400,  // default to 400mm gypsy circumference
      10,   // report value to SignalK every 10 seconds
      "/sensors/Gypsy_Circumference_mm"  // mm value may be updated using this
                                         // RESTful path
  );
  gypsyCircumferenceSensor->connect_to(new SKOutputInt(
      "navigation.anchor.gypsyCircumference",  // the SignalK parameter key for
                                               // this value
      "",  // the SignalK parameter key cannot be updated
      new SKMetadata("mm", "Gypsy Circumference"))  // metadata for this value
  );

  // Create the ConstantSensor object for the chain length (in metres)
  auto* chainLengthConstantSensor = new IntConstantSensor(
      100,                       // default to 100 meters of chanin
      10,                        // report value to SignalK every 10 seconds
      "/Sensors/Chain_Length_m"  // metre value can be updated using this
                                 // RESTful path
  );
  chainLengthConstantSensor->connect_to(new SKOutputInt(
      "navigation.anchor.chainLength",  // the SignalK parameter key for this
                                        // value
      "",  // the SignalK parameter key cannot be updated
      new SKMetadata("m", "Chain Length"))  // metadata for Chain Length value
  );

  // update the LCD panel every 250ms with the currrent status

  app.onRepeat(250, [gypsyCircumferenceSensor]() {
    float circum_m = gypsyCircumferenceSensor->get_value() / 1000.0;
    float chainOut = chainCount * circum_m;
    /*
        Serial.print(circum_m);
        Serial.print("  ");
        Serial.println(chainOut);
    */
    lcd.setCursor(0, 1);
    lcd.printf("%5.1fM", chainOut);
    lcd.setCursor(9, 0);
    lcd.printf("%3d", chainCount);

    //    return;

    // read buttons

    lcd.setCursor(14, 0);
    if (digitalRead(UP_BUTTON)) {
      lcd.print("-");
    } else {
      lcd.print("U");
    }

    lcd.setCursor(15, 0);
    if (digitalRead(DOWN_BUTTON)) {
      lcd.print("-");
    } else {
      lcd.print("D");
    }

    lcd.setCursor(13, 0);
    if (digitalRead(MENU_BUTTON)) {
      lcd.print("-");
    } else {
      lcd.print("M");
    }

    // read sense lines

    lcd.setCursor(14, 1);
    if (digitalRead(SENSE_UP)) {
      lcd.print("U");
    } else {
      lcd.print("-");
    }

    lcd.setCursor(15, 1);
    if (digitalRead(SENSE_DOWN)) {
      lcd.print("D");
    } else {
      lcd.print("-");
    }

    // gypsy status
    lcd.setCursor(0, 0);
    lcd.print(gypsyStatusDescription[gypsyStatus]);

    // save chainCount to non-volatile memory

    if (lastChainCount != chainCount) {
      preferences.putInt("counter", chainCount);
      lastChainCount = chainCount;
    }
  });

  // report gypsy count periodically
  // Create a RepeatSensor with int output that report the chain counter.
  int report_interval_ms = 1000;
  auto chainCountCallback = [](RepeatSensor<int>* sensor) {
    sensor->emit(chainCount);
  };
  auto* chainCountSensor =
      new RepeatSensor<int>(report_interval_ms, chainCountCallback);
  chainCountSensor->connect_to(new SKOutputInt("navigation.anchor.gypsyCount",
                                               "/sensors/anchor/gypsyCount"));

  // Create a RepeatSensor with float output that report the chain length out.
  auto chainOutCallback = [gypsyCircumferenceSensor](
                              RepeatSensor<float>* sensor) {
    sensor->emit(chainCount * gypsyCircumferenceSensor->get_value() / 1000.0);
  };
  auto* chainOutSensor =
      new RepeatSensor<float>(report_interval_ms, chainOutCallback);
  chainOutSensor->connect_to(new SKOutputFloat(
      "navigation.anchor.chainOut", "/sensors/anchor/chainOut",
      new SKMetadata("m", "Chain Out")));

  // create the timers we need

  gypsyTimer = xTimerCreate("Gypsy timer", 1000 / portTICK_PERIOD_MS, pdFALSE,
                            (void*)0, gypsyTimerCallback);

  // set the Gypsy pulse interrupt handler

  app.onInterrupt(SENSE_GYPSY_PULSE, FALLING, gypsyInterruptHandler);

  // Start networking, SK server connections and other SensESP
  // internals
  sensesp_app->start();
}

void loop() { app.tick(); }
