// Hotel Robot - Day 10
// Motor controller + ultrasonic + IR edge safety gating
//
// Commands from Raspberry Pi:
//   VEL <left> <right>
//   STOP
//   RESET
//
// Telemetry:
//   TEL {"us_cm":25,"front_edge":0,"rear_edge":0,"edge":{"fl":0,"fr":0,"rl":0,"rr":0}}
//
// Faults:
//   FAULT FRONT_EDGE
//   FAULT REAR_EDGE
//   FAULT FRONT_OBSTACLE

#include <string.h>
#include <stdio.h>

// ------------------------------
// Motor pins
// ------------------------------
const int PWM_A = 5;
const int DIR_A = 4;
const int PWM_B = 6;
const int DIR_B = 7;

// Adjust if motor direction is reversed
bool FORWARD_DIR_A = true;
bool FORWARD_DIR_B = true;

// ------------------------------
// Timing / safety
// ------------------------------
const unsigned long CMD_TIMEOUT_MS = 300;
const unsigned long TELEMETRY_MS = 100;
const unsigned long ULTRASONIC_TIMEOUT_US = 30000UL;

// ------------------------------
// Sensor pins
// ------------------------------
const int US_TRIG_PIN = 8;
const int US_ECHO_PIN = 9;

const int IR_FL_PIN = 10;
const int IR_FR_PIN = 11;
const int IR_RL_PIN = 12;
const int IR_RR_PIN = 13;

// Proven from your real test:
// 0 = floor
// 1 = edge
const int EDGE_ACTIVE_STATE = HIGH;

// Forward obstacle stop threshold
const int SAFE_FRONT_CM = 20;

// ------------------------------
// Runtime state
// ------------------------------
int currentLeft = 0;
int currentRight = 0;

unsigned long lastCmdMs = 0;
unsigned long lastTelemetryMs = 0;

char lineBuf[64];
size_t lineLen = 0;

int lastUsCm = -1;

// ------------------------------
// Sensor structs
// ------------------------------
struct EdgeState {
  bool fl;
  bool fr;
  bool rl;
  bool rr;
};

struct RawIrState {
  int fl;
  int fr;
  int rl;
  int rr;
};

// ------------------------------
// Motor helpers
// ------------------------------
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

void armCommandWatchdog() {
  lastCmdMs = millis();
}

// ------------------------------
// Sensor helpers
// ------------------------------
void setupSensors() {
  pinMode(US_TRIG_PIN, OUTPUT);
  pinMode(US_ECHO_PIN, INPUT);

  pinMode(IR_FL_PIN, INPUT);
  pinMode(IR_FR_PIN, INPUT);
  pinMode(IR_RL_PIN, INPUT);
  pinMode(IR_RR_PIN, INPUT);

  digitalWrite(US_TRIG_PIN, LOW);
}

RawIrState readRawIrSensors() {
  RawIrState raw;
  raw.fl = digitalRead(IR_FL_PIN);
  raw.fr = digitalRead(IR_FR_PIN);
  raw.rl = digitalRead(IR_RL_PIN);
  raw.rr = digitalRead(IR_RR_PIN);
  return raw;
}

EdgeState interpretEdgeSensors(const RawIrState& raw) {
  EdgeState e;
  e.fl = (raw.fl == EDGE_ACTIVE_STATE);
  e.fr = (raw.fr == EDGE_ACTIVE_STATE);
  e.rl = (raw.rl == EDGE_ACTIVE_STATE);
  e.rr = (raw.rr == EDGE_ACTIVE_STATE);
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
    return -1;
  }

  return (int)(duration * 0.0343 / 2.0);
}

void updateSensorSnapshot() {
  lastUsCm = readUltrasonicCm();
}

void sendTelemetry() {
  RawIrState raw = readRawIrSensors();
  EdgeState e = interpretEdgeSensors(raw);

  bool front_edge = e.fl || e.fr;
  bool rear_edge = e.rl || e.rr;

  Serial.print("TEL {\"us_cm\":");
  Serial.print(lastUsCm);

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

// ------------------------------
// Safety logic
// ------------------------------
bool isForwardCommand(int left, int right) {
  return (left > 0 && right > 0);
}

bool isReverseCommand(int left, int right) {
  return (left < 0 && right < 0);
}

bool isUnsafeToMove(int left, int right) {
  RawIrState raw = readRawIrSensors();
  EdgeState e = interpretEdgeSensors(raw);

  bool front_edge = e.fl || e.fr;
  bool rear_edge = e.rl || e.rr;

  bool forward_cmd = isForwardCommand(left, right);
  bool reverse_cmd = isReverseCommand(left, right);

  if (forward_cmd && front_edge) {
    Serial.println("FAULT FRONT_EDGE");
    return true;
  }

  if (reverse_cmd && rear_edge) {
    Serial.println("FAULT REAR_EDGE");
    return true;
  }

  if (forward_cmd && lastUsCm > 0 && lastUsCm < SAFE_FRONT_CM) {
    Serial.println("FAULT FRONT_OBSTACLE");
    return true;
  }

  return false;
}

void forceSafetyStop() {
  currentLeft = 0;
  currentRight = 0;
  stopMotors();
}

// ------------------------------
// Serial command handling
// ------------------------------
void handleStopCommand() {
  currentLeft = 0;
  currentRight = 0;
  stopMotors();
  armCommandWatchdog();
  Serial.println("ACK STOP");
}

void handleResetCommand() {
  currentLeft = 0;
  currentRight = 0;
  stopMotors();
  armCommandWatchdog();
  Serial.println("ACK RESET");
}

void handleVelCommand(int left, int right) {
  int nextLeft = constrain(left, -255, 255);
  int nextRight = constrain(right, -255, 255);

  updateSensorSnapshot();

  if (isUnsafeToMove(nextLeft, nextRight)) {
    forceSafetyStop();
    armCommandWatchdog();
    return;
  }

  currentLeft = nextLeft;
  currentRight = nextRight;
  armCommandWatchdog();

  Serial.print("ACK VEL ");
  Serial.print(currentLeft);
  Serial.print(" ");
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

// ------------------------------
// Arduino setup / loop
// ------------------------------
void setup() {
  Serial.begin(115200);

  pinMode(PWM_A, OUTPUT);
  pinMode(DIR_A, OUTPUT);
  pinMode(PWM_B, OUTPUT);
  pinMode(DIR_B, OUTPUT);

  stopMotors();
  setupSensors();
  armCommandWatchdog();
  updateSensorSnapshot();

  Serial.println("Hotel Robot Day 10 Safety Controller Ready");
  Serial.println("Commands: VEL <L> <R> | STOP | RESET");
}

void loop() {
  readSerialLines();

  updateSensorSnapshot();

  if ((millis() - lastCmdMs) > CMD_TIMEOUT_MS) {
    if (currentLeft != 0 || currentRight != 0) {
      forceSafetyStop();
      Serial.println("TIMEOUT STOP");
    } else {
      stopMotors();
    }
  } else {
    if (isUnsafeToMove(currentLeft, currentRight)) {
      forceSafetyStop();
    } else {
      applyDrive(currentLeft, currentRight);
    }
  }

  if ((millis() - lastTelemetryMs) >= TELEMETRY_MS) {
    lastTelemetryMs = millis();
    sendTelemetry();
  }

  delay(5);
}