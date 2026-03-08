Day 11 – Operator Console

Objective
Convert teleop control script into a real-time operator console.

Achievements

• teleop.py upgraded to dashboard interface
• serial log spam removed
• keyboard control integrated with dashboard
• sensor telemetry displayed live
• connection health detection implemented
• command TX rate displayed

Architecture

Raspberry Pi
    teleop.py → operator console
    sensor_monitor.py → debugging tool

Arduino
    sensor telemetry
    safety gating
    motor control

Impact

Operator can control robot while monitoring safety sensors and system state in real time.

This forms the foundation for future autonomy layers.
