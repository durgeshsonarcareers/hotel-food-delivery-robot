# Day 8 – Serial Teleop Stabilization

## Goal
Control robot motors from Raspberry Pi using keyboard teleop.

## Problems Found
1. Teleop deadman timer caused motors to stop after ~200 ms.
2. Arduino firmware did not match Pi command protocol.
3. Serial writes from teleop thread and manual commands collided.
4. Hardcoded serial port caused connection failures.

## Fixes Implemented
- Rewrote `comms/teleop.py`
- Added serial write locking
- Added RX/TX debug with timestamps
- Implemented Arduino firmware with commands:

VEL <left> <right>
STOP
RESET

## Safety
- Arduino watchdog timeout stops motors
- Bumper latch supported
- RESET clears fault

## Current Status
- Forward / Back / Left / Right working
- Stop working
- Stable repeated commands
- Serial communication verified

## Next Steps
1. Clean Arduino debug logging
2. Add encoder sensor support
3. Implement wheel RPM measurement
4. Later: closed-loop speed control
