from gpiozero import LED
from gpiozero.pins.lgpio import LGPIOFactory
from time import sleep
from loguru import logger
from core.config import LED_GPIO_PIN, LOG_DIR, LOG_FILE
import os

# -----------------------------
# GPIO backend (LOCKED)
# -----------------------------
factory = LGPIOFactory()

# -----------------------------
# Logging setup
# -----------------------------
os.makedirs(LOG_DIR, exist_ok=True)

logger.add(
    f"{LOG_DIR}/{LOG_FILE}",
    rotation="1 MB",
    level="INFO",
    format="{time} | {level} | {message}"
)

logger.info("Starting LED test (LGPIO backend)")

# -----------------------------
# LED init
# -----------------------------
led = LED(LED_GPIO_PIN, pin_factory=factory)

# -----------------------------
# Test sequence
# -----------------------------
logger.info("LED ON")
led.on()
sleep(3)

logger.info("LED OFF")
led.off()
sleep(2)

logger.info("Blinking LED")
for _ in range(10):
    led.on()
    sleep(0.5)
    led.off()
    sleep(0.5)

logger.info("LED test completed")
