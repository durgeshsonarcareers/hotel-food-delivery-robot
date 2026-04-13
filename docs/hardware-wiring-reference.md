# Hotel Robot Hardware Wiring Reference

## Purpose
This file records the current live hardware wiring, controller roles, pin mapping, and integration status for the hotel food delivery robot. It should be treated as the single reference point for wiring work before changing firmware or hardware.

---

## Current Controller Roles

### Raspberry Pi 4
- Main high-level controller
- Teleop / operator console
- USB serial link to Arduino
- Logging / monitoring / future autonomy logic

### Arduino Mega 2560
- Real-time motor control
- Runtime safety enforcement
- Ultrasonic sensing
- IR edge sensing
- Encoder pulse counting
- Power / sensor telemetry expansion point

---

## Current Live Hardware

### Core Control
- Raspberry Pi 4
- Arduino Mega 2560
- USB serial link between Raspberry Pi and Arduino

### Drive System
- L298N motor driver
- 4 TT DC motors
  - left-side motor pair
  - right-side motor pair
- Differential drive layout

### Safety / Sensing
- HC-SR04 ultrasonic sensor
- 4 downward-facing IR edge sensors
- 2 groove coupler encoder sensors
- INA219 current / voltage monitor
- ACS712 current sensor modules
- MPU6050 IMU

### Power
- Bench power supply (current development stage)
- Inline fuse (10A)
- Emergency stop switch

---

## Naming Convention

Robot orientation is always defined **from the robot perspective facing forward**.

| Name | Meaning |
|---|---|
| FL | Front Left |
| FR | Front Right |
| RL | Rear Left |
| RR | Rear Right |

### Motor Naming
| Motor Group | Physical Location |
|---|---|
| Left Motor Group | left-side wheels when robot faces forward |
| Right Motor Group | right-side wheels when robot faces forward |

---

## Current Confirmed Pin Map

### Motor Driver (L298N)
| Arduino Pin | L298N Pin | Purpose | Status |
|---|---|---|---|
| D5 | ENA | left-side PWM enable | active |
| D22 | IN1 | left-side direction input 1 | active |
| D23 | IN2 | left-side direction input 2 | active |
| D24 | IN3 | right-side direction input 1 | active |
| D25 | IN4 | right-side direction input 2 | active |
| D6 | ENB | right-side PWM enable | active |

### Ultrasonic Sensor (HC-SR04)
| Arduino Pin | Sensor Pin | Purpose | Status |
|---|---|---|---|
| D30 | TRIG | ultrasonic trigger | active |
| D31 | ECHO | ultrasonic echo | active |

### Edge Sensors
| Arduino Pin | Sensor Position | Purpose | Status |
|---|---|---|---|
| D32 | FL | front-left edge detection | active |
| D33 | FR | front-right edge detection | active |
| D34 | RL | rear-left edge detection | active |
| D35 | RR | rear-right edge detection | active |

### Encoders
| Arduino Pin | Encoder | Purpose | Status |
|---|---|---|---|
| D2 | Left Encoder | left wheel pulse counting | bench validated |
| D3 | Right Encoder | right wheel pulse counting | bench validated |

### I2C Bus
| Arduino Pin | Connected Device | Purpose | Status |
|---|---|---|---|
| D20 (SDA) | INA219, MPU6050 | I2C data | active / shared |
| D21 (SCL) | INA219, MPU6050 | I2C clock | active / shared |

### Analog Inputs
| Arduino Pin | Device | Purpose | Status |
|---|---|---|---|
| A0 | ACS712 #1 | current sensing | baseline validated |
| A1 | ACS712 #2 | current sensing | baseline validated |

---

## Motor Driver Wiring (L298N)

### Control Side
- D5 -> ENA
- D22 -> IN1
- D23 -> IN2
- D24 -> IN3
- D25 -> IN4
- D6 -> ENB

### Motor Outputs
| L298N Output | Connected To |
|---|---|
| OUT1 / OUT2 | left motor group |
| OUT3 / OUT4 | right motor group |

### Direction Note
If commanded forward motion causes one side to rotate backward, swap that side's motor wires at the L298N output instead of changing project assumptions everywhere else.

---

## Sensor Wiring

### 1) Ultrasonic Sensor (HC-SR04)
| Sensor Pin | Connected To |
|---|---|
| VCC | 5V |
| GND | GND |
| TRIG | D30 |
| ECHO | D31 |

### 2) IR Edge Sensors
These sensors are mounted **facing downward** to detect floor edges / cliff conditions.

| Sensor Position | Signal Pin | Power |
|---|---|---|
| FL | D32 | 5V / GND |
| FR | D33 | 5V / GND |
| RL | D34 | 5V / GND |
| RR | D35 | 5V / GND |

### Observed Logic
- `0` = floor present
- `1` = edge / drop detected

### 3) Encoder Sensors
| Encoder | Signal Pin | Power |
|---|---|---|
| Left Encoder | D2 | 5V / GND |
| Right Encoder | D3 | 5V / GND |

### Encoder Note
Encoder wiring polarity must be checked carefully. Incorrect VCC/GND wiring can cause heating, unstable behavior, or board power issues.

### 4) INA219
#### Logic Side
| INA219 Pin | Connected To |
|---|---|
| VCC | 5V |
| GND | GND |
| SDA | D20 |
| SCL | D21 |

#### High-Side Measurement Path
Main supply positive is routed through INA219 before the motor driver input:

`Bench PSU + -> INA219 VIN+ -> INA219 VIN- -> Emergency Stop -> L298N VS`

or, if emergency stop is placed before INA219 in the final build, the measurement path must still remain fully in series with the motor supply positive line.

### 5) ACS712
| Module | Output Pin | Power |
|---|---|---|
| ACS712 #1 | A0 | 5V / GND |
| ACS712 #2 | A1 | 5V / GND |

### 6) MPU6050
| MPU6050 Pin | Connected To |
|---|---|
| VCC | 5V |
| GND | GND |
| SDA | D20 |
| SCL | D21 |

---

## Raspberry Pi ↔ Arduino Communication

USB serial link is used between Raspberry Pi and Arduino.

| From | To | Purpose | Notes |
|---|---|---|---|
| Raspberry Pi USB | Arduino USB | serial communication | usually `/dev/ttyACM0` |

### Current Runtime Serial Contract
Commands used by Raspberry Pi:
- `VEL <left> <right>`
- `STOP`
- `RESET`

Typical responses from Arduino:
- `ACK ...`
- `FAULT ...`
- `TEL {...}`

---

## Power Wiring Reference

### Current Development Power Architecture
- Arduino Mega powered by USB during development / testing
- Bench power supply used for motor power path
- All logic and motor systems must share a **common ground**

### Main Power Path
`Bench PSU + -> INA219 / emergency-stop path -> L298N VS`

`Bench PSU - -> L298N GND -> Arduino GND (common reference)`

### Protection
| Component | Purpose |
|---|---|
| 10A inline fuse | short-circuit protection |
| emergency stop switch | manual power cut for motor drive path |

### Critical Rule
Do **not** use L298N 5V output to power the entire robot logic stack during development. Arduino logic power and motor power should remain intentionally controlled.

---

## Mounting Reference

### Top Layer
- Arduino Mega
- L298N
- power input
- emergency stop
- INA219
- MPU6050

### Bottom Layer
- 4 IR edge sensors facing floor
- encoder sensors near wheel / coupler positions
- breadboard for controlled signal / power distribution where needed

### Front
- HC-SR04 mounted facing forward near the robot front centerline

---

## Current Validation Status

### Confirmed Working
- Raspberry Pi teleop control
- L298N runtime control
- HC-SR04 ultrasonic sensor
- 4 IR edge sensors
- INA219 measurement path
- encoder basic bench response
- ACS712 baseline output
- Arduino runtime sketch with Pi serial control

### Validated Individually / Pending Further Runtime Integration
- MPU6050
- full encoder telemetry integration into runtime console
- ACS712 practical runtime use under load

---

## Firmware Reference

### Active Runtime Sketch
- `firmware/arduino/Runtime/hotel_robot_runtime_l298n/hotel_robot_runtime_l298n.ino`

### Historical Archived Sketches
- `firmware/arduino/Archive/...`

### Test Sketches
- `firmware/arduino/testing_program/...`

---

## Free / Reserved Pins

| Area | Current Situation |
|---|---|
| Digital pins | several still available |
| Analog pins | more available beyond A0 / A1 |
| I2C bus | already shared by INA219 and MPU6050 |
| Interrupt pins | D2 and D3 already assigned to encoders |

Before adding new hardware, confirm pin use against the active runtime sketch instead of relying only on memory.

---

## Critical Notes

### Edge Sensor Logic
- `0 = floor present`
- `1 = edge detected`

### Direction Assumption
- positive speed command -> forward motion
- negative speed command -> reverse motion

If motion is reversed physically, correct it at motor wiring or the runtime direction flags, not by silently changing multiple unrelated files.

### Known Hardware Lessons
- wrong encoder VCC/GND wiring can destabilize the system
- INA219 readings become invalid if the measurement path is not truly in series
- missing common ground between bench PSU and Arduino causes bad sensor / measurement behavior
- breadboard wiring quality matters for shared sensor buses, especially I2C

---

## Change Log

### Day 9
- awareness layer added
- ultrasonic sensing added
- IR edge sensing added
- telemetry system introduced

### Day 10
- safety layer added
- motion blocking on edge detection
- pivot safety improvements

### Day 13
- hardware baseline restored after project gap
- migrated from Cytron reference wiring to L298N runtime wiring
- revalidated ultrasonic, edge sensors, INA219, encoder bench response
- restored Raspberry Pi teleop control
- reorganized Arduino firmware into `Archive`, `Runtime`, and `testing_program`
