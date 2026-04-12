# Day 13 – Hardware Stabilization and Runtime Restore

## Objective
Resume the hotel food delivery robot project after a long gap, revalidate hardware and software, restore Raspberry Pi teleop control, and clean up Arduino firmware structure.

## Main Issues Found
- Bottom-board wiring previously caused board power collapse / dim power behavior.
- Root cause was incorrect encoder VCC/GND wiring on the left side.
- INA219 initially gave invalid readings because of incorrect placement around the emergency stop path and missing common ground reference.
- I2C devices worked individually, but combined behavior was sensitive to wiring quality.

## Fixes Completed
- Rebuilt and cleaned robot wiring.
- Switched to L298N motor driver for current PoC chassis.
- Verified motor movement using manual motor test.
- Verified Raspberry Pi teleop control with the updated runtime sketch.
- Corrected INA219 wiring and common ground architecture.
- Corrected encoder wiring issue that caused heating and instability.
- Reorganized Arduino firmware into Archive / Runtime / testing_program structure.

## Final Working Pin Map
- L298N:
  - ENA = 5
  - IN1 = 22
  - IN2 = 23
  - IN3 = 24
  - IN4 = 25
  - ENB = 6
- Ultrasonic:
  - TRIG = 30
  - ECHO = 31
- Edge Sensors:
  - FL = 32
  - FR = 33
  - RL = 34
  - RR = 35
- Encoders:
  - Left = 2
  - Right = 3
- I2C:
  - SDA = 20
  - SCL = 21

## Validation Status

### Working
- Arduino power stable
- Motors working through L298N
- Ultrasonic sensor working
- Edge sensors working
- Encoder basic bench response working
- INA219 working
- ACS712 baseline working
- Raspberry Pi teleop working
- Runtime sketch working with serial protocol

### Pending / Next
- MPU6050 final reintegration
- Encoder data integration into runtime telemetry
- Encoder display in Raspberry Pi operator console
- Better floor testing and movement characterization
- Combined sensor integration refinement

## Firmware Structure
- `firmware/arduino/Archive/` → old sketches preserved
- `firmware/arduino/Runtime/` → active runtime sketch
- `firmware/arduino/testing_program/` → sensor and hardware test sketches

## Day 13 Outcome
Robot control from Raspberry Pi was successfully restored after hardware revalidation and Arduino runtime migration to the new structure.

## Next Session Goal
Add encoder telemetry to the runtime sketch and expose encoder data in the Raspberry Pi operator console.
