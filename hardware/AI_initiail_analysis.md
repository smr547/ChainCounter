This is a really solid bench rig, Sten — it has exactly the right ingredients to de-risk the boat install: real 12 V noise sources, inductive loads, direction control, and a pulse pickup.

Below is how I’d “productise” the test bed into a repeatable experimental platform for your QP-on-core1 / SensESP+SignalK-on-core0 architecture, plus some concrete design improvements that map directly to boat-grade robustness.

## Turn the test bed into a set of test “capabilities”

Think in terms of what you can prove on the bench:

### Sensing integrity

- Reed pulses: clean edges, no double-counting, tolerant of bounce + EMI
- Relay state sense: correct UP/DOWN detection, tolerant of transients
- Brownout / reset behavior: no spurious counts, state recovery

### Control safety

- Ensure your ESP32 cannot energise the windlass relays unexpectedly
- Ensure any firmware crash can’t “hold a relay on”
- Ensure “sensor-only” mode is electrically safe by design

###Software architecture

- QP active objects (core 1) do hard real-time-ish pulse counting + state machine
- Core 0 does network + SensESP publishing and never blocks core 1
- Clear, testable message boundary between cores

That gives you a checklist of experiments you can run and re-run as you iterate hardware and firmware.

## Hardware improvements: make the inputs boringly reliable

You’re already planning the right thing: “robust, filtered, TVS protected, Schmitt trigger.” Here’s the shape of that, in practical blocks.

### Relay-coil sense (12 V -> logic)

- Your divider works for the bench, but on the boat you’ll want:
- Input protection: series resistor + TVS (to ground) + clamp to rails (or use a protected buffer IC)
- Low-pass filtering: RC to kill coil kick / wiring spikes / contact hash
- Schmitt trigger cleanup: so you get one crisp transition no matter what the waveform does
- Key point: the filter must be before the Schmitt, otherwise you’ll still trigger on spikes.

### Reed switch input

Reed switches are classic troublemakers:
- mechanical bounce (multi edges)
- long wiring acting as an antenna
- magnet “dwell” that can chatter around the threshold as the arm moves

Typical robust approach:

- bias + RC filter sized so bounce is invisible
- Schmitt trigger (or use ESP32 GPIO with good hysteresis? I’d still prefer external Schmitt)
- optionally software debounce as a second layer (time gate or state machine rules)
- And if you later move to Hall sensors, the same input conditioning block still helps.

### Grounding + power sanity (quiet MCU in a nasty 12 V world)

- On the boat, even “just sense” lines can get abused.
- Star-ish grounding between power conversion, relay currents, and logic ground
- Keep relay coil currents out of MCU ground return paths
- Add bulk capacitance near relay supply and near DC/DC outputs

Consider an ESP32 brownout strategy: either avoid brownout by power design, or handle it explicitly in the state machine (see software section)

## Control outputs: your instinct (“too dangerous”) is right

- Driving reversing relays from the ESP32 is fine on the bench, but for boat safety I’d push it toward a “can’t do harm” design:

Safer patterns (choose one)

- Sensor-only hardware: no outputs connected to any control path, full stop.
- Output gating / interlock: ESP32 outputs can request control but a physical enable (key switch / guarded toggle) must be on for power to reach relay coils.
- Fail-off driver topology: ensure that on reset / boot / crashed firmware, outputs are guaranteed off (pull-downs, driver enable pin, etc.), and ideally require a deliberate “armed” sequence.

If you keep outputs at all, make “default off” a hardware property, not just code.

## Dual-core software partition that fits your rig

Given your QP-on-core1 plan, I’d structure it like this:

- Core 1 (QP/C++): deterministic engine
- Active objects you likely want:
- AO_WindlassIO: owns GPIO sampling (reed, relay sense), edge detection, debouncing
- AO_WindlassSM: the windlass state machine (Idle, Up, Down, Fault, etc.), count integration, zeroing logic, sanity checks
- AO_Console/LCD (optional): UI state + button handling + LCD updates (or keep UI on core0 if it’s not time-critical)

Core 1 outputs events like:

- PULSE(n) or COUNT_DELTA(+1/-1)
- DIR_CHANGED(UP/DOWN/STOP)
- FAULT(brownout, impossible transitions, stuck input, etc.)

Core 0 (Arduino/SensESP): 

- networking + telemetry
- Takes events from core 1 (queue/ringbuffer)
- Publishes to SignalK at a controlled rate (daily/hourly logic later is easy)
- Handles Wi-Fi reconnect, mDNS, OTA, logging — all the “may block” stuff

This boundary is where you win: core 1 stays clean no matter what Wi-Fi does.

## Experiments your test bed can run right now (high value)

Here are a few “bench proofs” that pay off later on the boat:

- Pulse integrity under switching noise
- Run the motor, switch direction relays, spam the buttons, watch for false pulses.
- Criterion: count changes only when magnet passes reed.

Bounce characterisation

- Log raw reed transitions with timestamps for a few minutes.
- You’ll quickly see what debounce window you actually need.

Brownout + recovery

- Momentarily dip the 5 V rail (or pull the plug) while motor/relays are active.
- Criterion: no extra counts on reset; on reboot, state is sane (e.g., “Unknown/Stopped” until confirmed).

Direction correctness

- Verify you never attribute pulses to the wrong direction across transitions.
- State machine rule: direction must be stable for X ms before accepting pulses as + or −.

UI separation sanity

- Hammer menu buttons while pulsing fast; ensure counts remain correct.

## “Windlass sensor only” mode: design it as a first-class product

If this might become the preferred boat installation:

- Make the board and harness explicitly “sense-only”
- Provide a pass-through interface if needed (with proper protection/isolation depending on the existing chain counter input style)

Publish everything to SignalK:

- chain count / length
- direction
- motor running
- voltage (optional but useful)
- fault flags (reed bounce, brownout, impossible states)

If you want to keep momentum: paste your current GPIO assignment + wiring sketch (just text is fine), and I’ll propose:

- a concrete input conditioning circuit (values/ranges, not just blocks),
- the QP event model between AO_WindlassIO and AO_WindlassSM,
- and the core0/core1 message transport (ring buffer vs queue) that plays nicely with ESP32.



# Clutch emulator: 

## mechanical options (from easiest to most realistic)
### “Finger clutch” (quickest)

- Put a small knurled knob (or printed wheel) on the output shaft / gypsy shaft so you can spin it by hand while the motor is stopped.
- Pros: trivial, no extra parts, immediately tests “free fall”
- Cons: not self-sustaining, not speed-repeatable

### Slip clutch using friction washers (good repeatability)

- Add a friction interface between gearbox output and the gypsy shaft:
- Stack: hub → felt/rubber washer → smooth washer → spring washer (or wave washer) → nut
- Tighten nut to set torque threshold (so the “gypsy” can overrun the motor)
- Pros: adjustable “clutch tightness”, repeatable, simple
- Cons: some build effort, needs a shaft/hub arrangement

### One-way bearing / ratchet (very “windlass-like”)

Use a small one-way clutch (sprag/roller bearing) or a ratchet mechanism so the gypsy can:

- be driven by motor in “UP” direction, but
- overrun freely in “DOWN / freefall” direction (or vice versa depending on geometry)
- Pros: feels like a real windlass with back-driving behaviour
- Cons: sourcing + fitting complexity

###Electromagnetic “clutch release” (fancy, but powerful for tests)

- A small solenoid pulls a friction pad off the gypsy hub so it freewheels when “clutch released”.
- Pros: you can test “fast drop” under firmware control
- Cons: adds another inductive load + control channel

### Add a “drop load” so freefall isn’t just hand-spinning

To get meaningful behaviour you want the gypsy to spin itself.
Two easy bench loads:

- Weight-on-string: wrap cord around the gypsy; hang a weight so it accelerates and spins down.
- Spring spool: a preloaded spring (tape-measure style) that pulls line out.

This gives you:

- acceleration and deceleration
- potential overspeed
- pulse rates higher than your motor drive case
- What this enables you to validate (software rules)

Once you have freefall, you should treat the system as having two independent “truth sources”:

- Relay sense (commanded direction / motor expected state)
- Pulse stream (chain is actually moving)

That leads to useful states/events:

- Detect “chain moving with motor OFF”

If pulses arrive while both relays indicate OFF:

- classify as FREEFALL / MANUAL PAYOUT
- publish that to SignalK (super valuable)
- optionally raise an alert if it persists unexpectedly

Overspeed and bounce robustness

- Fast drop can make pulses come much faster than motor-driven.
- confirm your pulse capture path (interrupt or PCNT) doesn’t miss edges
- confirm debounce doesn’t “eat” real pulses at high speed
- confirm your direction logic remains correct when “direction is unknown” (motor off)

Direction inference when motor is off

- Real windlasses can freefall only “down” (payout), so you can safely assume direction = DOWN in freefall mode if the clutch release is active (or if motor is off and pulses present and your clutch emulator is engaged).

### Hardware note: this pushes you toward ESP32 PCNT

- If you haven’t already: freefall is the perfect reason to use the ESP32 pulse counter (PCNT) peripheral instead of pure GPIO interrupts + debounce, especially if your pulse rates get high.
- You can still apply debounce/qualification in software by sampling PCNT counts at a fixed tick (e.g. 10–20 ms) and applying sanity checks.

### Minimal “clutch emulator” recommendation

If you want fastest progress with good realism:

- build friction slip clutch + weight-on-string load.
- It’s cheap, tunable, and gives you repeatable freefall tests without needing special bearings.

If you tell me how your “gypsy” is currently implemented (is it just the Tamiya output shaft with a crank arm + magnet?), I can suggest a concrete mechanical layout (even just with off-the-shelf washers + M3/M4 hardware), and the exact software state transitions you can add for FREEFALL / MANUAL PAYOUT.