import glob
import select
import serial
import sys
import termios
import threading
import time
import tty


BAUD = 115200
SEND_HZ = 20

state_lock = threading.Lock()
serial_lock = threading.Lock()

left = 0
right = 0
running = True
last_tx_msg = None
last_tx_log_time = 0.0


def ts():
    return time.strftime("%H:%M:%S")


def log(msg):
    sys.stdout.write(f"\r\n[{ts()}] {msg}\r\n")
    sys.stdout.flush()


def pick_port():
    candidates = sorted(glob.glob("/dev/ttyACM*") + glob.glob("/dev/ttyUSB*"))
    if not candidates:
        raise RuntimeError("No serial device found under /dev/ttyACM* or /dev/ttyUSB*")
    return candidates[0]


def set_vel(l, r):
    global left, right
    with state_lock:
        left, right = l, r


def get_vel():
    with state_lock:
        return left, right


def serial_write_line(ser, line):
    with serial_lock:
        ser.write((line + "\n").encode("utf-8"))
        ser.flush()


def send_loop(ser):
    global running, last_tx_msg, last_tx_log_time

    period = 1.0 / SEND_HZ

    while running:
        l, r = get_vel()
        msg = f"VEL {l} {r}"

        serial_write_line(ser, msg)

        now = time.time()
        if msg != last_tx_msg or (now - last_tx_log_time) >= 1.0:
            log(f"TX {msg}")
            last_tx_msg = msg
            last_tx_log_time = now

        time.sleep(period)


def read_loop(ser):
    global running

    while running:
        try:
            line = ser.readline().decode("utf-8", errors="replace").strip()
            if line:
                log(f"ARD {line}")
        except Exception as e:
            log(f"RX ERROR {e}")
            break


def read_key_nonblocking(timeout=0.05):
    dr, _, _ = select.select([sys.stdin], [], [], timeout)
    if dr:
        return sys.stdin.read(1)
    return None


def main():
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
