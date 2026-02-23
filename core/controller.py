from time import sleep
from loguru import logger
from gpiozero import LED, Button
from gpiozero.pins.lgpio import LGPIOFactory

from core.config import LED_GPIO_PIN, BUTTON_GPIO_PIN
from hardware.actuators import DoorActuator


class RobotController:
    def __init__(self):
        self.factory = LGPIOFactory()

        self.led = LED(
            LED_GPIO_PIN,
            pin_factory=self.factory
        )

        self.button = Button(
            BUTTON_GPIO_PIN,
            pull_up=True,
            bounce_time=0.08,   # debounce
            pin_factory=self.factory
        )

        self.door = DoorActuator()

        self._is_active = False
        self._shutting_down = False

        # Bind callback
        self.button.when_pressed = self.toggle

    # -------------------------------------------------
    # STATE MANAGEMENT
    # -------------------------------------------------

    def set_state(self, active: bool):
        """Set controller state safely."""
        self._is_active = active

        if active:
            try:
                self.led.on()
            except Exception as e:
                logger.error(f"LED on() failed: {e}")

            try:
                self.door.open()
            except Exception as e:
                logger.error(f"Door open() failed: {e}")

            logger.info("STATE=ACTIVE")

        else:
            try:
                self.led.off()
            except Exception as e:
                logger.error(f"LED off() failed: {e}")

            try:
                self.door.close()
            except Exception as e:
                logger.error(f"Door close() failed: {e}")

            logger.info("STATE=IDLE")

    def toggle(self):
        """Toggle state on button press."""
        if self._shutting_down:
            return

        self.set_state(not self._is_active)

    # -------------------------------------------------
    # SAFE SHUTDOWN
    # -------------------------------------------------

    def shutdown(self):
        """Force system into safe state and detach callbacks."""
        if self._shutting_down:
            return

        self._shutting_down = True
        logger.warning("Shutdown initiated → forcing SAFE state")

        # Replace callbacks with no-op to prevent warning
        def _noop():
            return

        try:
            self.button.when_pressed = _noop
            self.button.when_released = _noop
        except Exception as e:
            logger.error(f"Failed to detach callbacks: {e}")

        # Force safe state
        self.set_state(False)

        # Close GPIO devices cleanly
        try:
            self.button.close()
            self.led.close()
        except Exception as e:
            logger.error(f"Failed to close GPIO devices: {e}")

        logger.warning("Shutdown complete → SAFE state confirmed")

    # -------------------------------------------------
    # MAIN LOOP
    # -------------------------------------------------

    def run(self):
        logger.info("Controller started. Press button to toggle.")
        self.set_state(False)

        try:
            while True:
                sleep(1)

        except KeyboardInterrupt:
            logger.info("KeyboardInterrupt received")

        except Exception as e:
            logger.error(f"Unhandled exception: {e}")

        finally:
            self.shutdown()
