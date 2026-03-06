# Hotel Service Robot – Project Context

## Goal
Build a modular service robot platform capable of performing tasks in hotels:
- food delivery to rooms
- luggage transport
- guest interaction

Initial prototype focuses on **manual control + safe motion base**.

## Current Hardware
- Raspberry Pi 4 (main computer)
- Arduino motor controller
- DC motors with encoder discs (20 slots)
- IR slot sensor for encoder
- bumper safety switches
- motor driver board
- battery pack

## Software Architecture

### Raspberry Pi
Python-based control layer.

Modules:
- teleop control
- navigation logic (future)
- perception (camera, sensors)
- UI / interaction

### Arduino
Low-level motor and safety controller.

Responsibilities:
- motor PWM control
- bumper safety stop
- encoder pulse counting
- command timeout watchdog

## Serial Protocol

Baud: 115200

Commands:

VEL <left> <right>  
STOP  
RESET

Example:
VEL 150 150
VEL -120 120

## Current Status
Manual teleoperation working and stable.

## Next Development Steps
1. wheel encoder reading
2. wheel RPM measurement
3. straight-line correction
4. distance movement
5. obstacle sensing
6. autonomous navigation

## Long-Term Architecture
Navigation stack will eventually move to ROS2 or similar robotics framework.


## Development Assistant Role
ChatGPT acts as:
- robotics software mentor
- architecture reviewer
- debugging assistant
- system design advisor

Responsibilities:
- maintain project structure
- avoid architectural drift
- guide step-by-step development
- ensure reproducible builds
