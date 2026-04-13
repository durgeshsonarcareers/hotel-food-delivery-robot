# Development Roadmap

## Phase 1 – Motion Base and Platform Stabilization

### Completed Core Foundations
- serial communication
- teleop control
- motor driver integration
- safety timeout
- ultrasonic awareness
- edge sensing awareness
- Arduino-side motion safety gating
- real-time operator console
- L298N runtime bring-up
- Raspberry Pi teleop restoration after hardware recovery
- Arduino firmware reorganization into:
  - `Archive`
  - `Runtime`
  - `testing_program`

---

## Progress Update

### Day 9 complete – Awareness Layer
Added:
- HC-SR04 ultrasonic sensing
- 4 IR edge sensors
- telemetry reporting for awareness data

Outcome:
- robot gained front obstacle awareness
- robot gained edge / cliff awareness
- `sensor_monitor.py` enabled live sensor debugging

### Day 10 complete – Safety Layer
Added:
- Arduino-side safety gating for unsafe motion
- forward / reverse protection using edge states
- obstacle blocking using ultrasonic threshold
- improved pivot safety logic

Outcome:
- robot became safety-aware during teleop
- safety enforcement moved to the Arduino side

### Day 11 complete – Operator Console
`teleop.py` was upgraded into a real-time operator console.

Added:
- live sensor dashboard
- motion display
- fault state monitoring
- serial communication health visibility

Outcome:
- operator control became much more usable than raw serial logs

### Day 13 complete – Hardware Stabilization and Runtime Restore
Main work completed:
- resumed project after long gap
- revalidated hardware step by step
- fixed bottom-board instability caused by incorrect encoder VCC/GND wiring
- corrected INA219 wiring and common-ground architecture
- confirmed motors, ultrasonic, edge sensors, encoder bench response, and Raspberry Pi teleop control
- reorganized Arduino firmware structure
- created active runtime sketch for current L298N hardware mapping

Outcome:
- robot runtime control from Raspberry Pi was restored
- project now has a clean firmware structure
- system is stable enough to continue into motion understanding work

---

## Current Known Limitations
- encoder data is not yet integrated into runtime telemetry / Pi operator console
- MPU6050 is not yet cleanly reintegrated into the active runtime
- ACS712 has baseline validation but not yet strong runtime value
- TT motors and current PoC chassis are acceptable for bring-up, not final production hardware
- bench testing still does not fully represent real floor behavior

---

## Next Focus

### Immediate Next Step
#### Encoder Runtime Integration
Goals:
- add left / right encoder counts into Arduino runtime telemetry
- expose encoder data in Raspberry Pi operator console
- verify encoder stability during real movement, not just bench rotation

### After Encoder Telemetry
#### Movement Characterization
Goals:
- compare left vs right counts
- understand drift
- estimate counts for short travel segments
- begin simple straight-line correction thinking

### After Movement Characterization
#### Sensor Reintegration and Control Improvement
Goals:
- reintegrate MPU6050 in a controlled way
- evaluate whether INA219 should be included in runtime telemetry immediately or later
- refine safety and telemetry quality without breaking teleop stability

---

## Phase 2 – Mobility Intelligence
Planned topics:
- encoder-informed movement
- obstacle handling refinement
- basic closed-loop motion behaviors
- path execution primitives
- movement repeatability

---

## Phase 3 – Task System
Planned topics:
- room delivery workflow
- docking / pickup / drop logic
- task execution sequencing
- service robot behavior model

---

## Phase 4 – User Interface
Planned topics:
- improved operator console
- touchscreen / HMI concepts
- status dashboard
- cloud or remote monitoring later if needed

---

## Long-Term Direction
The platform should gradually evolve from:
1. teleoperated motion base
2. safety-aware robot
3. self-observing robot
4. controlled movement robot
5. task-capable hotel service robot

The main rule going forward:
**do not break working teleop and safety behavior while adding intelligence.**
