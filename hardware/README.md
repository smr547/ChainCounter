## Hardware

Hardware compontents of the Chain Counter

Please review the [KiCad directory](./KiCad) for hardware design documentation

## Old chain counter
The old chain counter is a Qwick CHC 1202M ([see Manual](./quick_1202M_manual.pdf))

![IMG_0927](https://github.com/smr547/ChainCounter/assets/4327895/1345576a-cf31-4eb9-8c73-51068b0c889a)
![IMG_0926](https://github.com/smr547/ChainCounter/assets/4327895/8e1020f8-83a6-4571-946b-01ef2b5e4a13)

# Test bed

A benchtop test bed has been built it comprises

## Current design

- 12V input from lead acid battery
- Windlass electromechanical emulator -- Tamiya plastic moulded gearbox and motor casing. Included is a FA-30 type 3 volt electric motor, axle with 2 crank arm's) Gearbox 344:1 reduction, approx 30 RPM.
- Reed switch and magnet on crank arm to emulate windlass chain counter pickup
- Dual relays wired as reversing relays (12V solenoid, 3.3V power distribution to windlass motor)
- UP/DOWN buttons to power reversing relays (emulating forward hand control on boat)
- 12V to 3.3V DC/DC converter to power windlass motor
- 12V to 5V DC/DC converter to power MCU and related logic
- ESP32 dev module
- 2x16 LCD display module with I2C interface
- 3 minature push buttons to emulate chain counter controls -- UP, DOWN and MENU
- 2 x resistor divider circuits providing 12V to 3.3V logic shifting to sense reversing relay solenoid state
- 2 x MOSFET driver circuits to allow chain counter logic to drive reversing relays

## Planned improvements

- replace reversing relay sence with robust, fully fitered, TVS protected sense logic (with Schmitt trigger)
- replace reed switch interface with similar filtered interface
- possibly delete MOSFET driver -- too dangerous in on-boat situation (dirving very high powered motor)
- optional configuration as "windlass sensor only" with no display (SignalK telemetry on and optional sensor pass-through to regular chain counter)