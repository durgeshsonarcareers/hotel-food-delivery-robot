from gpiozero import LED, Button
from gpiozero.pins.lgpio import LGPIOFactory
from time import sleep
from loguru import logger
import os

from core.config import LED_GPIO_PIN, BUTTON_GPIO_PIN, LOG_DIR, LOG_FILE

factory = LGPIOFactory()

os.makedirs(LOG_DIR, exist_ok=True)
logger.add(
    f"{LOG_DIR}/{LOG_FILE}",
    rotation="1 MB",
    level="INFO",
    format="{time} | {level} | {message}",
)

logger.info("Starting TOGGLE Integration Test: Button + LED (LGPIO)")

led = LED(LED_GPIO_PIN, pin_factory=factory)
button = Button(BUTTON_GPIO_PIN, pull_up=True, pin_factory=factory)

# state
is_on = False

def set_state(new_state: bool):
    global is_on
    is_on = new_state
    if is_on:
        led.on()
        logger.info("STATE=ON  (LED ON)")
    else:
        led.off()
        logger.info("STATE=OFF (LED OFF)")

def on_pressed():
    # toggle on press only (ignore release)
    set_state(not is_on)

button.when_pressed = on_pressed

# safe start
set_state(False)

try:
    while True:
        sleep(1)
except KeyboardInterrupt:
    logger.info("Stopped by user. Resetting safe state.")
finally:
    set_state(False)
