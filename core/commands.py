from loguru import logger

class CommandDispatcher:
    def __init__(self, controller):
        self.controller = controller

    def handle(self, cmd: str):
        logger.info(f"[CMD] Received: {cmd}")

        if cmd == "OPEN":
            self.controller.set_state(True)

        elif cmd == "CLOSE":
            self.controller.set_state(False)

        elif cmd == "TOGGLE":
            self.controller.toggle()

        else:
            logger.warning(f"Unknown command: {cmd}")
