# Day 14 — Encoder Integration & Telemetry Upgrade

## Objective
Integrate encoder feedback into the robot runtime and expose it to the operator console without breaking existing teleop and safety systems.

---

## What Was Implemented

### 1. Encoder Integration (Arduino Runtime)

- Used interrupt pins:
  - Left Encoder → D2
  - Right Encoder → D3
- Implemented ISR-based pulse counting
- Added safe read using `noInterrupts()` / `interrupts()`

### 2. Telemetry Update

Extended telemetry format:

```text
TEL {
  "us_cm": <distance>,
  "edge": { "fl":0, "fr":0, "rl":0, "rr":0 },
  "enc": { "l":123, "r":130 }
}
Existing format preserved
Added enc object
No regression in serial protocol
3. Teleop Console Update
Parsed enc.l and enc.r
Added ENCODERS section:
ENCODERS
 Left Count      : 123
 Right Count     : 130
4. Edge Sensor Fix
Corrected polarity (active HIGH)
Removed dependency on stale fields:
front_edge
rear_edge
Derived summary dynamically from raw sensors:
front_edge = fl OR fr
rear_edge = rl OR rr
5. Dashboard Improvements
Added raw sensor visibility:
FL, FR, RL, RR
Fixed rendering issues using fixed-width formatting
Removed stale display artifacts (OKGE / OKIGGERED)
Validation Results
1. Idle Test
Encoder counts remain stable
No drift observed
2. Manual Wheel Test
Left wheel → left count increases
Right wheel → right count increases
3. Motion Test

Example:

VEL 80 80

Observed:

Both encoders increase
Slight mismatch between left and right
Observations
Encoder counts are functional and stable
Left/right mismatch observed → expected (mechanical differences)
No impact on safety system
Telemetry remains consistent and reliable
System State After Day 14
Working
Teleop control
Safety layer (edge + ultrasonic)
Encoder feedback
Live telemetry
Console visualization
Not Implemented (Intentional)
Speed calculation
Direction detection
Odometry
Drift correction
