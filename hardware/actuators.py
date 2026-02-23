from dataclasses import dataclass
from loguru import logger

@dataclass
class DoorActuator:
    """
    Pi-safe stub.
    Later we will implement:
      - ArduinoDoorActuator (serial)
      - ServoDoorActuator (external 5V + detach)
    """
    is_open: bool = False

    def open(self):
        self.is_open = True
        logger.info("[DOOR] open() (stub)")

    def close(self):
        self.is_open = False
        logger.info("[DOOR] close() (stub)")
