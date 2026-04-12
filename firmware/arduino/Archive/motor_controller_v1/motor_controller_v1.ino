// Hotel Robot PoC - Arduino motor controller
// Protocol expected from Raspberry Pi teleop:
//   VEL <left> <right>
//   STOP
//   RESET
//
// Example:
//   VEL 150 150
//   VEL -120 120
//   STOP
//   RESET
//
// Notes:
// - left/right range: -255..255
// - bumper hit latches a fault and stops motors
// - RESET clears bumper fault
// - command timeout stops motors if Pi stops sending commands 

const int PWM_A = 5;
const int DIR_A = 4;
const int PWM_B = 6;
const int DIR_B = 7;

// Keep these aligned with actual motor wiring.
bool FORWARD_DIR_A = true;
bool FORWARD_DIR_B = true;

const int bumperPin = 22;
const int BUMPER_TRIGGERED_STATE = HIGH;  // If bumper logic is reversed, change to LOW
const unsigned long DEBOUNCE_MS = 30;
const unsigned long CMD_TIMEOUT_MS = 1000;

int stableState;
int lastRaw;
unsigned long lastChangeMs = 0;

bool faultLatched = false;
int currentLeft = 0;
int currentRight = 0;
unsigned long lastCmdMs = 0;

char lineBuf[64];
size_t lineLen = 0;

void stopMotors() {
  analogWrite(PWM_A, 0);
  analogWrite(PWM_B, 0);
}

void applyMotorSingle(int pwmPin, int dirPin, bool forwardDirHigh, int value) {
  int speed = constrain(abs(value), 0, 255);
  bool forward = (value >= 0);
  bool dirLevel = forward ? forwardDirHigh : !forwardDirHigh;

  digitalWrite(dirPin, dirLevel ? HIGH : LOW);
  analogWrite(pwmPin, speed);
}

void applyDrive(int left, int right) {
  applyMotorSingle(PWM_A, DIR_A, FORWARD_DIR_A, left);
  applyMotorSingle(PWM_B, DIR_B, FORWARD_DIR_B, right);
}

void setupBumper() {
  pinMode(bumperPin, INPUT_PULLUP);
  stableState = digitalRead(bumperPin);
  lastRaw = stableState;
  lastChangeMs = millis();
}

bool bumperTriggeredDebounced() {
  int raw = digitalRead(bumperPin);

  if (raw != lastRaw) {
    lastRaw = raw;
    lastChangeMs = millis();
  }

  if ((millis() - lastChangeMs) > DEBOUNCE_MS && raw != stableState) {
    stableState = raw;
    if (stableState == BUMPER_TRIGGERED_STATE) {
      return true;
    }
  }

  return false;
}

void clearMotionState() {
  currentLeft = 0;
  currentRight = 0;
  stopMotors();
}

void armCommandWatchdog() {
  lastCmdMs = millis();
}

void handleStopCommand() {
  currentLeft = 0;
  currentRight = 0;
  stopMotors();
  armCommandWatchdog();
  Serial.println("ACK STOP");
}

void handleResetCommand() {
  faultLatched = false;
  currentLeft = 0;
  currentRight = 0;
  stopMotors();
  armCommandWatchdog();
  Serial.println("ACK RESET");
}

void handleVelCommand(int left, int right) {
  currentLeft = constrain(left, -255, 255);
  currentRight = constrain(right, -255, 255);
  armCommandWatchdog();
  Serial.print("ACK VEL ");
  Serial.print(currentLeft);
  Serial.print(' ');
  Serial.println(currentRight);
}

void processLine(char* line) {
  while (*line == ' ' || *line == '\t') line++;
  if (*line == '\0') return;

  int left = 0;
  int right = 0;

  if (strncmp(line, "STOP", 4) == 0) {
    handleStopCommand();
    return;
  }

  if (strncmp(line, "RESET", 5) == 0) {
    handleResetCommand();
    return;
  }

  if (sscanf(line, "VEL %d %d", &left, &right) == 2) {
    handleVelCommand(left, right);
    return;
  }

  Serial.print("ERR UNKNOWN CMD: ");
  Serial.println(line);
}

void readSerialLines() {
  while (Serial.available() > 0) {
    char c = (char)Serial.read();

    if (c == '\r') {
      continue;
    }

    if (c == '\n') {
      lineBuf[lineLen] = '\0';
      processLine(lineBuf);
      lineLen = 0;
      continue;
    }

    if (lineLen < sizeof(lineBuf) - 1) {
      lineBuf[lineLen++] = c;
    } else {
      lineLen = 0;
      Serial.println("ERR LINE TOO LONG");
    }
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(PWM_A, OUTPUT);
  pinMode(DIR_A, OUTPUT);
  pinMode(PWM_B, OUTPUT);
  pinMode(DIR_B, OUTPUT);

  clearMotionState();
  setupBumper();
  armCommandWatchdog();

  Serial.println("Hotel Robot Motor Controller Ready");
  Serial.println("Commands: VEL <left> <right> | STOP | RESET");
}

void loop() {
  readSerialLines();

  if (bumperTriggeredDebounced()) {
    faultLatched = true;
    clearMotionState();
    Serial.println("FAULT BUMPER");
  }

  if (faultLatched) {
    stopMotors();
    delay(5);
    return;
  }

  if ((millis() - lastCmdMs) > CMD_TIMEOUT_MS) {
    if (currentLeft != 0 || currentRight != 0) {
      currentLeft = 0;
      currentRight = 0;
      stopMotors();
      Serial.println("TIMEOUT STOP");
    } else {
      stopMotors();
    }
    delay(5);
    return;
  }

  applyDrive(currentLeft, currentRight);
  delay(5);
}
