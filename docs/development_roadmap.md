# Development Roadmap

## Phase 1 – Motion Base
✔ serial communication
✔ teleop control
✔ motor driver integration
✔ safety timeout

Next:
- encoder integration
- speed measurement
- closed-loop control

## Progress Update
- Day 9 complete: awareness layer added using HC-SR04 + 4 IR edge sensors
- Day 10 complete: Arduino-side safety gating added for forward/reverse/pivot risk handling
- Day 11 complete
  teleop.py upgraded into a real-time operator console.

Features added:
• live sensor dashboard
• motion command display
• fault state monitoring
• serial communication status
• TX command rate indicator
• connection health detection

sensor_monitor.py retained as a dedicated sensor debugging tool 

##Known Limitations

- Edge sensors show occasional chatter when robot is lifted during bench testing.
- Current chassis uses low-cost TT motors not suitable for final platform.
- Electrical telemetry (current/voltage monitoring) not yet integrated.

##Next Focus

- Day 12 – Electrical Telemetry Layer

• integrate INA219 for battery voltage and total current monitoring
• integrate ACS712 sensors for motor channel current monitoring
• extend Arduino telemetry message structure
• display electrical metrics in operator console

## Phase 2 – Mobility Intelligence
- obstacle sensors
- mapping
- localization

## Phase 3 – Task System
- delivery workflow
- elevator interaction
- fleet control

## Phase 4 – User Interface
- touchscreen UI
- voice interaction
- cloud monitoring
