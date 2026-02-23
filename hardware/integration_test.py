from gpiozero import LED, Button, Servo
from gpiozero.pins.lgpio import LGPIOFactory
from time import sleep
from loguru import logger
import os

from core.config import (
    LED_GPIO_PIN,
    SERVO_GPIO_PIN,
    BUTTON_GPIO_PIN,
    LOG_DIR,
    LOG_FILE,
)

# -------------------------------------------------
# GPIO backend (LOCKED — no fallbacks)
# -------------------------------------------------
factory = LGPIOFactory()

# -------------------------------------------------
# Logging setup
# -------------------------------------------------
os.makedirs(LOG_DIR, exist_ok=True)

logger.add(
    f"{LOG_DIR}/{LOG_FILE}",
    rotation="1 MB",
    level="INFO",
    format="{time} | {level} | {message}",
)

logger.info("Starting INTEGRATION TEST: Servo + Button + LED (LGPIO)")

# -------------------------------------------------
# Hardware devices
# -------------------------------------------------
led = LED(
    LED_GPIO_PIN,
    pin_factory=factory,
)

button = Button(
    BUTTON_GPIO_PIN,
    pull_up=True,          # button to GND when pressed
    pin_factory=factory,
)

servo = Servo(
    SERVO_GPIO_PIN,
    min_pulse_width=0.8 / 1000,
    max_pulse_width=2.2 / 1000,
    pin_factory=factory,
)


# -------------------------------------------------
# Safe initial state
# -------------------------------------------------
logger.info("Initializing safe state: Door CLOSED, LED OFF")
led.off()
servo.min()

# -------------------------------------------------
# Button callbacks
# -------------------------------------------------
def handle_button_pressed():
    logger.info("Button PRESSED → Door OPEN, LED ON")
    servo.max()
    led.on()


def handle_button_released():
    logger.info("Button RELEASED → Door CLOSED, LED OFF")
    servo.min()
    led.off()


button.when_pressed = handle_button_pressed
button.when_released = handle_button_released

logger.info("System ready. Press button to OPEN door.")

# -------------------------------------------------
# Keep process alive
# -------------------------------------------------
try:
    while True:
        sleep(1)

except KeyboardInterrupt:
    logger.info("Integration test interrupted by user")

finally:
    logger.info("Resetting to safe state and exiting")
    servo.min()
    led.off()
