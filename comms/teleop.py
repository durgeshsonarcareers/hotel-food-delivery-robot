import glob
import json
import select
import serial
import sys
import termios
import threading
import time
import tty
from typing import Optional, Tuple

BAUD = 115200
SEND_HZ = 20

state_lock = threading.Lock()
serial_lock = threading.Lock()

left = 0
right = 0
running = True

last_tx_msg = None
last_tx_log_time = 0.0

last_tel_state = None
last_us_zone = None


def ts() -> str:
    return time.strftime("%H:%M:%S")


def log(msg: str) -> None:
    sys.stdout.write(f"\r\n[{ts()}] {msg}\r\n")
    sys.stdout.flush()


def pick_port() -> str:
    candidates = sorted(glob.glob("/dev/ttyACM*") + glob.glob("/dev/ttyUSB*"))
    if not candidates:
        raise RuntimeError("No serial device found under /dev/ttyACM* or /dev/ttyUSB*")
    return candidates[0]


def set_vel(l: int, r: int) -> None:
    global left, right
    with state_lock:
        left, right = l, r


def get_vel() -> Tuple[int, int]:
    with state_lock:
        return left, right


def serial_write_line(ser: serial.Serial, line: str) -> None:
    with serial_lock:
        ser.write((line + "\n").encode("utf-8"))
        ser.flush()


def ultrasonic_zone(us_cm: int) -> str:
    if us_cm is None or us_cm < 0:
        return "NO_ECHO"
    if us_cm < 20:
        return "NEAR"
    if us_cm < 50:
        return "MID"
    return "FAR"


def parse_tel(line: str) -> Optional[dict]:
    if not line.startswith("TEL "):
        return None
    try:
        return json.loads(line[4:].strip())
    except json.JSONDecodeError:
        return None


def summarize_tel_change(data: dict) -> Optional[str]:
    global last_tel_state, last_us_zone

    front_edge = int(data.get("front_edge", 0))
    rear_edge = int(data.get("rear_edge", 0))
    edge = data.get("edge", {})
    fl = int(edge.get("fl", 0))
    fr = int(edge.get("fr", 0))
    rl = int(edge.get("rl", 0))
    rr = int(edge.get("rr", 0))
    us_cm = int(data.get("us_cm", -1))

    current_state = (front_edge, rear_edge, fl, fr, rl, rr)
    current_zone = ultrasonic_zone(us_cm)

    parts = []

    if last_tel_state is None or current_state != last_tel_state:
        parts.append(
            f"SENSE front={'EDGE' if front_edge else 'OK'} "
            f"rear={'EDGE' if rear_edge else 'OK'} "
            f"[FL={fl} FR={fr} RL={rl} RR={rr}]"
        )

    if last_us_zone is None or current_zone != last_us_zone:
        if us_cm < 0:
            parts.append("US NO_ECHO")
        else:
            parts.append(f"US {us_cm}cm ({current_zone})")

    last_tel_state = current_state
    last_us_zone = current_zone

    if parts:
        return " | ".join(parts)
    return None


def send_loop(ser: serial.Serial) -> None:
    global running, last_tx_msg, last_tx_log_time

    period = 1.0 / SEND_HZ

    while running:
        l, r = get_vel()
        msg = f"VEL {l} {r}"
        serial_write_line(ser, msg)

        # Only log TX when command actually changes
        if msg != last_tx_msg:
            log(f"TX {msg}")
            last_tx_msg = msg
            last_tx_log_time = time.time()

        time.sleep(period)


def read_loop(ser: serial.Serial) -> None:
    global running

    while running:
        try:
            line = ser.readline().decode("utf-8", errors="replace").strip()
            if not line:
                continue

            tel = parse_tel(line)
            if tel is not None:
                summary = summarize_tel_change(tel)
                if summary:
                    log(summary)
                continue

            # suppress repetitive ACK VEL noise
            if line.startswith("ACK VEL"):
                continue

            # keep important lines visible
            if (
                line.startswith("FAULT")
                or line.startswith("TIMEOUT")
                or line.startswith("ERR")
                or line.startswith("ACK STOP")
                or line.startswith("ACK RESET")
            ):
                log(f"ARD {line}")
                continue

            # fallback for anything unexpected
            log(f"ARD {line}")

        except Exception as e:
            log(f"RX ERROR {e}")
            break


def read_key_nonblocking(timeout: float = 0.05) -> Optional[str]:
    dr, _, _ = select.select([sys.stdin], [], [], timeout)
    if dr:
        return sys.stdin.read(1)
    return None


def main() -> None:
    global running

    port = pick_port()
    log(f"Using serial port: {port}")

    ser = serial.Serial(port, BAUD, timeout=0.2)
    time.sleep(2)
    ser.reset_input_buffer()
    ser.reset_output_buffer()

    tx_thread = threading.Thread(target=send_loop, args=(ser,), daemon=True)
    rx_thread = threading.Thread(target=read_loop, args=(ser,), daemon=True)
    tx_thread.start()
    rx_thread.start()

    log("Teleop: press key once -> keep moving until next command")
    log("w/s/a/d move, SPACE stop, r reset, q quit")

    fd = sys.stdin.fileno()
    old = termios.tcgetattr(fd)
    tty.setraw(fd)

    try:
        while True:
            key = read_key_nonblocking(0.05)
            if key is None:
                continue

            if key == "w":
                set_vel(150, 150)
                log("CMD forward")
            elif key == "s":
                set_vel(-150, -150)
                log("CMD backward")
            elif key == "a":
                set_vel(-120, 120)
                log("CMD left")
            elif key == "d":
                set_vel(120, -120)
                log("CMD right")
            elif key == " ":
                set_vel(0, 0)
                serial_write_line(ser, "STOP")
                log("CMD stop")
            elif key == "r":
                serial_write_line(ser, "RESET")
                log("CMD reset")
            elif key == "q":
                log("CMD quit")
                break
    finally:
        running = False
        set_vel(0, 0)
        try:
            serial_write_line(ser, "STOP")
            time.sleep(0.1)
        except Exception:
            pass
        termios.tcsetattr(fd, termios.TCSADRAIN, old)
        ser.close()


if __name__ == "__main__":
    main()
