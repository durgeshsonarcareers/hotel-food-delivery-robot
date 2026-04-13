import glob
import json
import select
import serial
import sys
import termios
import threading
import time
import tty
from typing import Optional

BAUD = 115200
SEND_HZ = 20
REFRESH_HZ = 10
CONNECTION_TIMEOUT_S = 2.0

state_lock = threading.Lock()
serial_lock = threading.Lock()

running = True


def now_str() -> str:
    return time.strftime("%H:%M:%S")


def pick_port() -> str:
    candidates = sorted(glob.glob("/dev/ttyACM*") + glob.glob("/dev/ttyUSB*"))
    if not candidates:
        raise RuntimeError("No serial device found under /dev/ttyACM* or /dev/ttyUSB*")
    return candidates[0]


class ConsoleState:
    def __init__(self) -> None:
        self.port = ""
        self.current_command = "STOP"
        self.left_speed = 0
        self.right_speed = 0

        self.last_sent = "-"
        self.last_rx = "-"
        self.last_event = "Startup"
        self.last_event_time = now_str()

        self.ack_count = 0
        self.connected = False
        self.last_rx_time = 0.0

        self.fault_state = "NONE"

        self.us_cm = -1
        self.edge = {"fl": 0, "fr": 0, "rl": 0, "rr": 0}
        self.enc_l = 0
        self.enc_r = 0

console = ConsoleState()


def set_event(text: str) -> None:
    with state_lock:
        console.last_event = text
        console.last_event_time = now_str()


def set_motion(command: str, left: int, right: int) -> None:
    with state_lock:
        console.current_command = command
        console.left_speed = left
        console.right_speed = right
        console.last_event = f"Command -> {command}"
        console.last_event_time = now_str()


def get_motion() -> tuple[int, int]:
    with state_lock:
        return console.left_speed, console.right_speed


def serial_write_line(ser: serial.Serial, line: str) -> None:
    with serial_lock:
        ser.write((line + "\n").encode("utf-8"))
        ser.flush()
    with state_lock:
        console.last_sent = line


def parse_tel_line(line: str) -> Optional[dict]:
    if not line.startswith("TEL "):
        return None
    payload = line[4:].strip()
    try:
        return json.loads(payload)
    except json.JSONDecodeError:
        return None


def update_telemetry(data: dict) -> None:
    with state_lock:
        console.us_cm = data.get("us_cm", console.us_cm)

        edge = data.get("edge", {})
        if isinstance(edge, dict):
            console.edge["fl"] = edge.get("fl", console.edge["fl"])
            console.edge["fr"] = edge.get("fr", console.edge["fr"])
            console.edge["rl"] = edge.get("rl", console.edge["rl"])
            console.edge["rr"] = edge.get("rr", console.edge["rr"])

        enc = data.get("enc", {})
        if isinstance(enc, dict):
            console.enc_l = enc.get("l", console.enc_l)
            console.enc_r = enc.get("r", console.enc_r)

        console.last_rx = "TEL"
        console.connected = True
        console.last_rx_time = time.monotonic()

def set_fault(text: str) -> None:
    with state_lock:
        console.fault_state = text
        console.last_rx = f"FAULT {text}"
        console.connected = True
        console.last_rx_time = time.monotonic()
        console.last_event = f"Fault -> {text}"
        console.last_event_time = now_str()

def clear_fault_display() -> None:
    with state_lock:
        console.fault_state = "NONE"


def mark_ack() -> None:
    with state_lock:
        console.ack_count += 1
        console.last_rx = "ACK"
        console.connected = True
        console.last_rx_time = time.monotonic()


def send_loop(ser: serial.Serial) -> None:
    global running
    period = 1.0 / SEND_HZ

    while running:
        left, right = get_motion()
        msg = f"VEL {left} {right}"
        try:
            serial_write_line(ser, msg)
        except Exception as exc:
            set_event(f"TX error: {exc}")
            break
        time.sleep(period)


def read_loop(ser: serial.Serial) -> None:
    global running

    while running:
        try:
            line = ser.readline().decode("utf-8", errors="replace").strip()
        except Exception as exc:
            set_event(f"RX error: {exc}")
            break

        if not line:
            continue

        parsed = parse_tel_line(line)
        if parsed is not None:
            update_telemetry(parsed)
            continue

        if line.startswith("ACK"):
            mark_ack()
            continue

        if line.startswith("FAULT"):
            fault_text = line[len("FAULT"):].strip() or "ACTIVE"
            set_fault(fault_text)
            continue

        with state_lock:
            console.last_rx = line
            console.connected = True
            console.last_rx_time = time.monotonic()


def bool_state_text(active: int) -> str:
    return "EDGE" if bool(active) else "OK"

def ultrasonic_text(us_cm: int) -> str:
    if us_cm is None or us_cm < 0:
        return "NO ECHO"
    return f"{us_cm} cm"


def dashboard_text() -> str:
    with state_lock:
        port = console.port
        rx_age = time.monotonic() - console.last_rx_time if console.last_rx_time > 0 else 9999
        connected = "YES" if console.connected and rx_age <= CONNECTION_TIMEOUT_S else "LOST"

        current_command = console.current_command
        left_speed = console.left_speed
        right_speed = console.right_speed

        fault_state = console.fault_state

        us_text = ultrasonic_text(console.us_cm)

        fl_active = bool(console.edge.get("fl", 0))
        fr_active = bool(console.edge.get("fr", 0))
        rl_active = bool(console.edge.get("rl", 0))
        rr_active = bool(console.edge.get("rr", 0))

        front_edge = "EDGE" if (fl_active or fr_active) else "OK"
        rear_edge = "EDGE" if (rl_active or rr_active) else "OK"

        fl = "EDGE" if fl_active else "OK"
        fr = "EDGE" if fr_active else "OK"
        rl = "EDGE" if rl_active else "OK"
        rr = "EDGE" if rr_active else "OK"

        enc_l = console.enc_l
        enc_r = console.enc_r

        last_sent = console.last_sent
        last_rx = console.last_rx
        last_event = console.last_event
        last_event_time = console.last_event_time
        ack_count = console.ack_count
        tx_rate_hz = SEND_HZ

    lines = [
        "HOTEL ROBOT - DAY 14 OPERATOR CONSOLE",
        "",
        f"Serial Port      : {port:<20}",
        f"Connected        : {connected:<4}",
        f"ACK Count        : {ack_count:<6}",
        f"TX Rate          : {f'{tx_rate_hz} Hz':<8}",
        "",
        "MOTION",
        f" Current Command : {current_command:<10}",
        f" Left Speed      : {left_speed:<6}",
        f" Right Speed     : {right_speed:<6}",
        "",
        "SAFETY",
        f" Fault State     : {fault_state:<10}",
        "",
        "SENSORS",
        f" Ultrasonic      : {us_text:<10}",
        f" Front Edge      : {front_edge:<4}",
        f" Rear Edge       : {rear_edge:<4}",
        "",
        "ENCODERS",
        f" Left Count      : {enc_l:<8}",
        f" Right Count     : {enc_r:<8}",
        "",
        "INDIVIDUAL EDGE SENSORS",
        f" FL              : {fl:<4}",
        f" FR              : {fr:<4}",
        f" RL              : {rl:<4}",
        f" RR              : {rr:<4}",
        "",
        "LAST ACTIVITY",
        f" Last Sent       : {last_sent:<20}",
        f" Last RX         : {last_rx:<20}",
        f" Last Event      : {(last_event_time + ' | ' + last_event):<40}",
        "",
        "CONTROLS",
        " W = forward",
        " S = backward",
        " A = left",
        " D = right",
        " SPACE = stop",
        " R = reset",
        " Q = quit",
        "",
        "Legend: OK = floor present, EDGE = edge/cliff detected",
    ]
    return "\r\n".join(lines)


def read_key_nonblocking(timeout: float = 0.05) -> Optional[str]:
    ready, _, _ = select.select([sys.stdin], [], [], timeout)
    if ready:
        return sys.stdin.read(1)
    return None


def handle_key(ser: serial.Serial, key: str) -> bool:
    if key == "w":
        clear_fault_display()
        set_motion("FORWARD", 150, 150)
    elif key == "s":
        clear_fault_display()
        set_motion("BACKWARD", -150, -150)
    elif key == "a":
        clear_fault_display()
        set_motion("LEFT", -120, 120)
    elif key == "d":
        clear_fault_display()
        set_motion("RIGHT", 120, -120)
    elif key == " ":
        set_motion("STOP", 0, 0)
        set_event("Manual stop")
    elif key == "r":
        serial_write_line(ser, "RESET")
        clear_fault_display()
        set_event("Reset requested")
    elif key == "q":
        set_event("Quit requested")
        return False

    return True


def main() -> None:
    global running

    port = pick_port()
    console.port = port

    ser = serial.Serial(port, BAUD, timeout=0.2)
    time.sleep(2.0)
    ser.reset_input_buffer()
    ser.reset_output_buffer()

    with state_lock:
        console.connected = True
        console.last_event = "Console started"
        console.last_event_time = now_str()

    tx_thread = threading.Thread(target=send_loop, args=(ser,), daemon=True)
    rx_thread = threading.Thread(target=read_loop, args=(ser,), daemon=True)

    tx_thread.start()
    rx_thread.start()

    fd = sys.stdin.fileno()
    old = termios.tcgetattr(fd)
    tty.setcbreak(fd)

    last_draw = 0.0
    draw_period = 1.0 / REFRESH_HZ

    try:
        keep_running = True
        sys.stdout.write("\x1b[2J\x1b[H")
        sys.stdout.write("\x1b[?25l")
        sys.stdout.flush()

        while keep_running:
            now = time.monotonic()

            if now - last_draw >= draw_period:
                sys.stdout.write("\x1b[H")
                sys.stdout.write(dashboard_text())
                sys.stdout.write("\x1b[J")
                sys.stdout.flush()
                last_draw = now

            key = read_key_nonblocking(0.05)
            if key is not None:
                keep_running = handle_key(ser, key)

    finally:
        running = False
        set_motion("STOP", 0, 0)

        try:
            serial_write_line(ser, "VEL 0 0")
            time.sleep(0.1)
        except Exception:
            pass

        termios.tcsetattr(fd, termios.TCSADRAIN, old)

        sys.stdout.write("\x1b[?25h")
        sys.stdout.write("\x1b[2J\x1b[H")
        sys.stdout.flush()

        ser.close()
        print("Teleop exited cleanly.")


if __name__ == "__main__":
    main()
