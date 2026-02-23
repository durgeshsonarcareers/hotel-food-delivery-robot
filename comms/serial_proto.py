import json
from dataclasses import dataclass

@dataclass
class Cmd:
    cmd: str
    args: dict

def encode(cmd: str, **kwargs) -> bytes:
    return (json.dumps({"cmd": cmd, "args": kwargs}) + "\n").encode("utf-8")
