# Hotel Service Robot – Project Context

## Goal
Build a modular service robot platform capable of performing tasks in hotels, including:
- food delivery to rooms
- luggage transport
- guest interaction

The current prototype focus is still the same:
**manual control + safe motion base + hardware stabilization for future autonomy**

---

## Current Development Status
The project has now moved beyond the earlier bare teleop stage.

Current system status:
- Raspberry Pi teleop control is working
- Arduino runtime firmware is working with Raspberry Pi serial commands
- L298N-based runtime is now the active motor-control path for the current PoC chassis
- ultrasonic sensing is working
- 4 IR edge sensors are working
- encoder sensors are bench validated
- INA219 is working after wiring correction
- ACS712 baseline readings are validated
- hardware / firmware structure has been reorganized into active runtime, archived firmware, and test programs

This means the robot is now a **recoverable and controllable mobile platform**, not just an experimental bench setup.

---

## Current Hardware

### Main Controllers
- Raspberry Pi 4 (main computer)
- Arduino Mega 2560 (real-time controller)

### Drive Base
- L298N motor driver
- 4 TT DC motors
- differential-drive chassis

### Sensors
- HC-SR04 ultrasonic sensor
- 4 downward-facing IR edge sensors
- 2 groove coupler encoder sensors
- INA219 current / voltage monitor
- ACS712 current sensors
- MPU6050 IMU

### Power / Protection
- bench power supply (current development stage)
- emergency stop switch
- inline fuse

---

## Current Software Architecture

### Raspberry Pi
Python-based high-level control layer.

Current modules:
- `comms/teleop.py` → operator control console
- `comms/sensor_monitor.py` → sensor debugging / monitoring tool

Future Raspberry Pi responsibilities:
- autonomy logic
- higher-level motion control
- task behavior
- perception integration
- navigation stack
- UI / operator workflows

### Arduino
Low-level real-time controller.

Current Arduino responsibilities:
- motor drive control through L298N
- serial command handling
- safety enforcement
- ultrasonic sensing
- edge sensing
- encoder counting foundation
- telemetry generation

---

## Current Firmware Structure

### Active Runtime
- `firmware/arduino/Runtime/hotel_robot_runtime_l298n/hotel_robot_runtime_l298n.ino`

### Historical Firmware Archive
- `firmware/arduino/Archive/`

### Standalone Hardware Validation Sketches
- `firmware/arduino/testing_program/`

This structure is intentional:
- `Runtime/` = live sketch used with Raspberry Pi
- `Archive/` = historical project evolution
- `testing_program/` = isolated bring-up / validation sketches

---

## Active Runtime Wiring Model
The current runtime firmware is based on this live pin map:

### L298N
- ENA = D5
- IN1 = D22
- IN2 = D23
- IN3 = D24
- IN4 = D25
- ENB = D6

### Ultrasonic
- TRIG = D30
- ECHO = D31

### Edge Sensors
- FL = D32
- FR = D33
- RL = D34
- RR = D35

### Encoders
- left encoder = D2
- right encoder = D3

### I2C
- SDA = D20
- SCL = D21

---

## Serial Protocol

### Baud Rate
- `115200`

### Commands from Raspberry Pi
- `VEL <left> <right>`
- `STOP`
- `RESET`

### Typical Responses from Arduino
- `ACK ...`
- `FAULT ...`
- `TEL {...}`

Example:
```text
VEL 150 150
VEL -120 120
STOP
RESET
```

This serial contract must remain stable unless there is a very good reason to change it.

---

## Current Known State

### Working
- manual teleoperation from Raspberry Pi
- Arduino runtime sketch upload / control
- safety-aware motion base
- ultrasonic telemetry
- edge telemetry
- current hardware bring-up flow
- separated runtime / archive / test program structure

### Bench Validated / Not Fully Integrated Yet
- encoder counting
- INA219 telemetry expansion
- ACS712 runtime use
- MPU6050 reintegration into main runtime telemetry

---

## What Was Learned During Hardware Recovery
After the project gap, the robot had to be revalidated step by step.

Important lessons from recovery:
- bottom-board instability was caused by incorrect encoder VCC/GND wiring
- INA219 readings become invalid if it is not truly in series with the supply path
- common ground between Arduino and motor power supply is mandatory
- I2C devices may work individually but still fail in a weak shared wiring setup
- hardware validation sketches are essential and should remain separate from runtime firmware

---

## Next Development Steps
1. add encoder values into runtime telemetry
2. expose encoder data inside Raspberry Pi operator console
3. characterize movement on floor using encoder counts
4. improve motion consistency
5. reintegrate MPU6050 cleanly
6. prepare for basic closed-loop movement behaviors

---

## Long-Term Architecture
The robot is still intended to evolve toward a modular service-robot architecture.

Planned future direction:
- autonomy layer on Raspberry Pi
- ROS2 or similar robotics framework later
- navigation and localization layer
- task / hotel workflow layer
- operator dashboard / HMI layer
- modular hardware upgrades for production hardware

---

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
- protect working code during feature growth
- help preserve clean documentation and reproducible progress
