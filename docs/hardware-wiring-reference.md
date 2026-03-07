# Hotel Robot Hardware Wiring Reference

## Purpose
This file tracks the current live wiring and signal mapping for the robot so future development does not rely only on code or memory.

---

# Current Controller Roles

## Raspberry Pi 4
- main controller
- teleop / UI / monitoring
- operator console
- serial link to Arduino

## Arduino (current motor/sensor controller)
- real-time motor control
- ultrasonic sensor reading
- IR edge sensor reading
- safety layer enforcement

---

# Current Live Hardware

- Raspberry Pi 4
- Arduino controller
- Cytron motor driver
- left DC motor
- right DC motor
- HC-SR04 ultrasonic sensor
- 4× downward IR edge sensors
- power supply / battery
- inline fuse (10A)
- emergency stop switch

---

# Naming Convention

Robot orientation is always defined **from the robot perspective facing forward**.

| Name | Meaning |
|-----|-----|
| FL | Front Left |
| FR | Front Right |
| RL | Rear Left |
| RR | Rear Right |

Motor naming:

| Motor | Physical location |
|-----|-----|
| Left Motor | robot left side when facing forward |
| Right Motor | robot right side when facing forward |

---

# Arduino Pin Usage

| Pin | Signal | Connected Device | Purpose | Status |
|----|----|----|----|----|
| D8 | TRIG | HC-SR04 | ultrasonic trigger | active |
| D9 | ECHO | HC-SR04 | ultrasonic echo | active |
| D10 | IR_FL | IR sensor | front-left edge detection | active |
| D11 | IR_FR | IR sensor | front-right edge detection | active |
| D12 | IR_RL | IR sensor | rear-left edge detection | active |
| D13 | IR_RR | IR sensor | rear-right edge detection | active |

Motor driver control pins are handled inside the motor controller code and should be verified against the Arduino motor controller sketch before documenting.

---

# Motor Driver Wiring (Cytron)

Cytron motor driver is used for differential drive control.

| Controller Pin | Driver Pin | Function | Notes |
|----|----|----|----|
| Arduino PWM pin | Cytron PWM | motor speed control | verify exact pin in motor_controller code |
| Arduino digital pin | Cytron DIR | motor direction | verify exact pin in motor_controller code |

Motor outputs:

| Driver Output | Connected To |
|----|----|
| Motor A | Left motor |
| Motor B | Right motor |

If motors move in the wrong direction, polarity can be swapped at the motor terminals.

---

# Sensor Wiring

## Ultrasonic (HC-SR04)

| Arduino Pin | Sensor Pin | Notes |
|----|----|----|
| D8 | TRIG | trigger signal |
| D9 | ECHO | echo return |

Power:

| Sensor Pin | Connected To |
|----|----|
| VCC | 5V |
| GND | GND |

---

## IR Edge Sensors

These sensors are mounted **facing downward** to detect floor edges / cliffs.

| Arduino Pin | Sensor | Position | Active State | Notes |
|----|----|----|----|----|
| D10 | IR sensor | Front Left | HIGH = edge | active |
| D11 | IR sensor | Front Right | HIGH = edge | active |
| D12 | IR sensor | Rear Left | HIGH = edge | active |
| D13 | IR sensor | Rear Right | HIGH = edge | active |

Observed logic:
# Hotel Robot Hardware Wiring Reference

## Purpose
This file tracks the current live wiring and signal mapping for the robot so future development does not rely only on code or memory.

---

# Current Controller Roles

## Raspberry Pi 4
- main controller
- teleop / UI / monitoring
- operator console
- serial link to Arduino

## Arduino (current motor/sensor controller)
- real-time motor control
- ultrasonic sensor reading
- IR edge sensor reading
- safety layer enforcement

---

# Current Live Hardware

- Raspberry Pi 4
- Arduino controller
- Cytron motor driver
- left DC motor
- right DC motor
- HC-SR04 ultrasonic sensor
- 4× downward IR edge sensors
- power supply / battery
- inline fuse (10A)
- emergency stop switch

---

# Naming Convention

Robot orientation is always defined **from the robot perspective facing forward**.

| Name | Meaning |
|-----|-----|
| FL | Front Left |
| FR | Front Right |
| RL | Rear Left |
| RR | Rear Right |

Motor naming:

| Motor | Physical location |
|-----|-----|
| Left Motor | robot left side when facing forward |
| Right Motor | robot right side when facing forward |

---

# Arduino Pin Usage

| Pin | Signal | Connected Device | Purpose | Status |
|----|----|----|----|----|
| D8 | TRIG | HC-SR04 | ultrasonic trigger | active |
| D9 | ECHO | HC-SR04 | ultrasonic echo | active |
| D10 | IR_FL | IR sensor | front-left edge detection | active |
| D11 | IR_FR | IR sensor | front-right edge detection | active |
| D12 | IR_RL | IR sensor | rear-left edge detection | active |
| D13 | IR_RR | IR sensor | rear-right edge detection | active |

Motor driver control pins are handled inside the motor controller code and should be verified against the Arduino motor controller sketch before documenting.

---

# Motor Driver Wiring (Cytron)

Cytron motor driver is used for differential drive control.

| Controller Pin | Driver Pin | Function | Notes |
|----|----|----|----|
| Arduino PWM pin | Cytron PWM | motor speed control | verify exact pin in motor_controller code |
| Arduino digital pin | Cytron DIR | motor direction | verify exact pin in motor_controller code |

Motor outputs:

| Driver Output | Connected To |
|----|----|
| Motor A | Left motor |
| Motor B | Right motor |

If motors move in the wrong direction, polarity can be swapped at the motor terminals.

---

# Sensor Wiring

## Ultrasonic (HC-SR04)

| Arduino Pin | Sensor Pin | Notes |
|----|----|----|
| D8 | TRIG | trigger signal |
| D9 | ECHO | echo return |

Power:

| Sensor Pin | Connected To |
|----|----|
| VCC | 5V |
| GND | GND |

---

## IR Edge Sensors

These sensors are mounted **facing downward** to detect floor edges / cliffs.

| Arduino Pin | Sensor | Position | Active State | Notes |
|----|----|----|----|----|
| D10 | IR sensor | Front Left | HIGH = edge | active |
| D11 | IR sensor | Front Right | HIGH = edge | active |
| D12 | IR sensor | Rear Left | HIGH = edge | active |
| D13 | IR sensor | Rear Right | HIGH = edge | active |

Observed logic:
0 = floor detected
1 = edge / drop detected
---

# Raspberry Pi ↔ Arduino

Communication uses USB serial.

| From | To | Purpose | Notes |
|----|----|----|----|
| Raspberry Pi USB | Arduino USB | serial communication | `/dev/ttyACM0` |

Serial communication is used for:
- teleop commands
- telemetry
- safety faults
- sensor data

---

# Power Wiring

## Main Power Path

Battery / SMPS
↓
Inline Fuse (10A)
↓
Emergency Stop Switch
↓
Motor Driver + Controllers

---

# Protection

| Component | Purpose |
|----|----|
| 10A inline fuse | protects against short circuit |
| emergency stop switch | manual safety shutdown |

---

# Installed But Not Yet Fully Used

These components exist in the project but are not fully integrated yet.

- encoder sensors (wheel odometry)
- Raspberry Pi camera module
- limit switches (planned safety features)

---

# Free / Reserved Pins

| Controller | Free Pins | Reserved Notes |
|----|----|----|
| Arduino | many remaining digital pins | future encoders / sensors |

Exact availability should be confirmed before adding new hardware.

---

# Critical Notes

Edge sensor behavior:
0 = floor present
1 = edge detected

Motor direction assumptions:
- positive speed → forward motion
- negative speed → reverse motion

If robot moves opposite direction, swap motor polarity.

Sensor limitations:
- ultrasonic occasionally returns `NO ECHO`
- IR sensors are sensitive to mounting height and surface reflectivity

---

# Change Log

**Day 9**
- awareness layer added
- ultrasonic sensing
- IR edge sensors
- telemetry system

**Day 10**
- safety layer added
- motion blocking on edge detection
- pivot safety improvements
