from core.controller import RobotController

def print_help() -> None:
    print(
        "\nCommands:\n"
        "  OPEN    -> set ACTIVE (LED ON, door open stub)\n"
        "  CLOSE   -> set IDLE   (LED OFF, door close stub)\n"
        "  TOGGLE  -> toggle ACTIVE/IDLE\n"
        "  STATUS  -> print current state\n"
        "  HELP    -> show this help\n"
        "  EXIT    -> shutdown safely and quit\n"
    )


def main() -> None:
    controller = RobotController()

    # Start controller loop in a background thread (non-blocking)
    import threading

    t = threading.Thread(target=controller.run, daemon=True)
    t.start()

    print_help()

    try:
        while True:
            cmd = input("robot> ").strip().upper()

            if not cmd:
                continue

            if cmd in ("HELP", "?"):
                print_help()

            elif cmd == "OPEN":
                controller.set_state(True)

            elif cmd == "CLOSE":
                controller.set_state(False)

            elif cmd == "TOGGLE":
                controller.toggle()

            elif cmd == "STATUS":
                print(f"ACTIVE={controller._is_active}")

            elif cmd in ("EXIT", "QUIT"):
                print("Shutting down...")
                controller.shutdown()
                break

            else:
                print(f"Unknown command: {cmd}")
                print("Type HELP for commands.")

    except KeyboardInterrupt:
        print("\nCtrl+C received. Shutting down...")
        controller.shutdown()


if __name__ == "__main__":
    main()
