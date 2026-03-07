// Hotel Robot PoC - Arduino motor controller + Day 9 awareness telemetry
//
// Preserves existing command protocol from Raspberry Pi teleop:
//   VEL <left> <right>
//   STOP
//   RESET
//
// Adds sensor telemetry lines in this format:
//   TEL {"us_cm":34,"front_edge":0,"rear_edge":0,"edge":{"fl":0,"fr":0,"rl":0,"rr":0}}
//
// Notes:
// - IR sensors are treated as downward edge/cliff sensors.
// - Active LOW means: LOW = edge/cliff detected.
// - Teleop behavior is not changed.
// - Telemetry is read-only and does not stop motors by itself yet.

#include <string.h>
#include <stdio.h>

const int PWM_A = 5;
const int DIR_A = 4;
const int PWM_B = 6;
const int DIR_B = 7;

// Keep these aligned with actual motor wiring.
bool FORWARD_DIR_A = true;
bool FORWARD_DIR_B = true;

// Existing bumper support from v1
const int bumperPin = 22;                  // harmless if using Mega; ignore if not wired on Uno builds
const int BUMPER_TRIGGERED_STATE = HIGH;
const unsigned long DEBOUNCE_MS = 30;
const unsigned long CMD_TIMEOUT_MS = 300;

// Day 9 sensors
const int US_TRIG_PIN = 8;
const int US_ECHO_PIN = 9;

const int IR_FL_PIN = 10;
const int IR_FR_PIN = 11;
const int IR_RL_PIN = 12;
const int IR_RR_PIN = 13;

const unsigned long TELEMETRY_MS = 100;
const unsigned long ULTRASONIC_TIMEOUT_US = 30000UL; // ~5m max pulse timeout

int stableState = HIGH;
int lastRaw = HIGH;
unsigned long lastChangeMs = 0;

bool faultLatched = false;
int currentLeft = 0;
int currentRight = 0;
unsigned long lastCmdMs = 0;
unsigned long lastTelemetryMs = 0;

char lineBuf[64];
size_t lineLen = 0;

struct EdgeState {
  bool fl;
  bool fr;
  bool rl;
  bool rr;
};

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
  // If you're on Uno and bumper isn't used, this stays effectively idle.
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

    if (c == '\r') continue;

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

void setupSensors() {
  pinMode(US_TRIG_PIN, OUTPUT);
  pinMode(US_ECHO_PIN, INPUT);

  pinMode(IR_FL_PIN, INPUT);
  pinMode(IR_FR_PIN, INPUT);
  pinMode(IR_RL_PIN, INPUT);
  pinMode(IR_RR_PIN, INPUT);

  digitalWrite(US_TRIG_PIN, LOW);
}

EdgeState readEdgeSensors() {
  EdgeState e;
  e.fl = (digitalRead(IR_FL_PIN) == LOW);
  e.fr = (digitalRead(IR_FR_PIN) == LOW);
  e.rl = (digitalRead(IR_RL_PIN) == LOW);
  e.rr = (digitalRead(IR_RR_PIN) == LOW);
  return e;
}

int readUltrasonicCm() {
  digitalWrite(US_TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(US_TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(US_TRIG_PIN, LOW);

  unsigned long duration = pulseIn(US_ECHO_PIN, HIGH, ULTRASONIC_TIMEOUT_US);

  if (duration == 0) {
    return -1; // no echo / timeout
  }

  int cm = (int)(duration * 0.0343 / 2.0);
  return cm;
}

void sendTelemetry() {
  int us_cm = readUltrasonicCm();
  EdgeState e = readEdgeSensors();

  bool front_edge = e.fl || e.fr;
  bool rear_edge  = e.rl || e.rr;

  Serial.print("TEL {\"us_cm\":");
  Serial.print(us_cm);
  Serial.print(",\"front_edge\":");
  Serial.print(front_edge ? 1 : 0);
  Serial.print(",\"rear_edge\":");
  Serial.print(rear_edge ? 1 : 0);
  Serial.print(",\"edge\":{\"fl\":");
  Serial.print(e.fl ? 1 : 0);
  Serial.print(",\"fr\":");
  Serial.print(e.fr ? 1 : 0);
  Serial.print(",\"rl\":");
  Serial.print(e.rl ? 1 : 0);
  Serial.print(",\"rr\":");
  Serial.print(e.rr ? 1 : 0);
  Serial.println("}}");
}

void setup() {
  Serial.begin(115200);

  pinMode(PWM_A, OUTPUT);
  pinMode(DIR_A, OUTPUT);
  pinMode(PWM_B, OUTPUT);
  pinMode(DIR_B, OUTPUT);

  clearMotionState();
  setupBumper();
  setupSensors();
  armCommandWatchdog();

  Serial.println("Hotel Robot Motor Controller Ready");
  Serial.println("Commands: VEL <L> <R> | STOP | RESET");
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
  } else if ((millis() - lastCmdMs) > CMD_TIMEOUT_MS) {
    if (currentLeft != 0 || currentRight != 0) {
      currentLeft = 0;
      currentRight = 0;
      stopMotors();
      Serial.println("TIMEOUT STOP");
    } else {
      stopMotors();
    }
  } else {
    applyDrive(currentLeft, currentRight);
  }

  if ((millis() - lastTelemetryMs) >= TELEMETRY_MS) {
    lastTelemetryMs = millis();
    sendTelemetry();
  }

  delay(5);
}
