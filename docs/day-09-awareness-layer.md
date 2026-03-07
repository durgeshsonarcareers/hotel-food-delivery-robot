# Hotel Robot Project - Day 9
## Awareness Layer: Ultrasonic + IR Edge Sensors

## Goal
Add a basic awareness layer while waiting for encoder sensors:
- front distance awareness using HC-SR04
- edge/cliff detection using 4 downward IR sensors
- structured telemetry from Arduino
- Raspberry Pi monitor for live sensor visibility

## Hardware Used
- Arduino Uno
- Raspberry Pi 4
- HC-SR04 ultrasonic sensor
- 4x IR obstacle avoidance sensors mounted downward as edge/cliff sensors
- Existing L298N + DC motor setup

## Wiring Used
### Ultrasonic
- D8 -> TRIG
- D9 -> ECHO

### IR edge sensors
- D10 -> Front Left
- D11 -> Front Right
- D12 -> Rear Left
- D13 -> Rear Right

## Key Learning
Initial logic assumption was wrong.
For the current downward-mounted IR setup, actual behavior is:

- `0 = floor present`
- `1 = edge / no floor`

So the correct interpretation for cliff detection is:
- `EDGE_ACTIVE_STATE = HIGH`

This was confirmed using a dedicated debug sketch that printed raw sensor states and interpreted edge states.

## Files Added / Updated
- `firmware/arduino/motor_controller/motor_controller_with_ir_ultrasonic.ino`
- `comms/sensor_monitor.py`
- separate debug sketch created for raw IR truth testing

## What Was Implemented
### Arduino
- ultrasonic read logic
- 4x IR sensor read logic
- structured telemetry output:
  - ultrasonic distance
  - front edge summary
  - rear edge summary
  - per-sensor edge state

### Raspberry Pi
- terminal-based sensor monitor
- live display of:
  - ultrasonic status
  - front/rear edge summary
  - per-sensor edge state

## Validation Result
Day 9 awareness layer is working:
- all sensors report OK when floor is present
- edge conditions are detected correctly
- ultrasonic gives usable readings, with occasional `NO ECHO` during bench testing

## Known Limitations
- HC-SR04 still shows intermittent `NO ECHO`
- IR sensors are sensitive to mounting angle, height, and test surface
- monitor is terminal-based, not a graphical dashboard
- teleop and monitor cannot run at the same time on the same serial port

## Outcome
Robot now has a working awareness layer, which becomes the base for motion safety logic.

## Next Step
Use this awareness layer to build Arduino-side safety gating for unsafe motion commands.
