import glob
import json
import serial
import sys
import time
from typing import Optional


BAUD = 115200
REFRESH_HZ = 10


def pick_port() -> str:
    candidates = sorted(glob.glob("/dev/ttyACM*") + glob.glob("/dev/ttyUSB*"))
    if not candidates:
        raise RuntimeError("No serial device found under /dev/ttyACM* or /dev/ttyUSB*")
    return candidates[0]


def parse_tel_line(line: str) -> Optional[dict]:
    if not line.startswith("TEL "):
        return None

    payload = line[4:].strip()
    try:
        return json.loads(payload)
    except json.JSONDecodeError:
        return None


def state_text(active: bool) -> str:
    return "EDGE" if active else "OK"


def clear_screen() -> None:
    sys.stdout.write("\033[2J\033[H")
    sys.stdout.flush()


def render(data: dict, port: str) -> None:
    us_cm = data.get("us_cm", -1)
    front_edge = bool(data.get("front_edge", 0))
    rear_edge = bool(data.get("rear_edge", 0))

    edge = data.get("edge", {})
    fl = bool(edge.get("fl", 0))
    fr = bool(edge.get("fr", 0))
    rl = bool(edge.get("rl", 0))
    rr = bool(edge.get("rr", 0))

    if us_cm is None or us_cm < 0:
        us_text = "NO ECHO"
    else:
        us_text = f"{us_cm} cm"

    clear_screen()
    print("HOTEL ROBOT - DAY 9 SENSOR MONITOR")
    print(f"Serial Port         : {port}")
    print(f"Ultrasonic Front    : {us_text}")
    print("")
    print(f"Front Edge Summary  : {state_text(front_edge)}")
    print(f"Rear Edge Summary   : {state_text(rear_edge)}")
    print("")
    print("Per-Sensor Edge Status")
    print(f"  Front Left        : {state_text(fl)}")
    print(f"  Front Right       : {state_text(fr)}")
    print(f"  Rear Left         : {state_text(rl)}")
    print(f"  Rear Right        : {state_text(rr)}")
    print("")
    print("Legend: OK = floor present, EDGE = no floor / cliff detected")
    print("Ctrl+C to exit")


def main() -> None:
    port = pick_port()
    print(f"Using serial port: {port}")

    ser = serial.Serial(port, BAUD, timeout=0.2)
    time.sleep(2.0)
    ser.reset_input_buffer()
    ser.reset_output_buffer()

    last_data = {
        "us_cm": -1,
        "front_edge": 0,
        "rear_edge": 0,
        "edge": {"fl": 0, "fr": 0, "rl": 0, "rr": 0},
    }
    last_render = 0.0

    try:
        while True:
            raw = ser.readline().decode("utf-8", errors="replace").strip()
            if raw:
                parsed = parse_tel_line(raw)
                if parsed is not None:
                    last_data = parsed
                else:
                    print(f"[ARD] {raw}")

            now = time.time()
            if now - last_render >= (1.0 / REFRESH_HZ):
                render(last_data, port)
                last_render = now

    except KeyboardInterrupt:
        print("\nExiting sensor monitor.")
    finally:
        ser.close()


if __name__ == "__main__":
    main()
