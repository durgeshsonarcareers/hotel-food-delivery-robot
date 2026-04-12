// Hotel Robot - Pi Runtime Controller for current Day 12 wiring
// Keeps Raspberry Pi serial protocol compatible with teleop.py / sensor_monitor.py
// Commands from Raspberry Pi:
//   VEL <left> <right>
//   STOP
//   RESET
//
// Telemetry:
//   TEL {"us_cm":25,"front_edge":0,"rear_edge":0,"edge":{"fl":0,"fr":0,"rl":0,"rr":0}}
//
// Faults:
//   FAULT FRONT_LEFT_EDGE
//   FAULT FRONT_RIGHT_EDGE
//   FAULT REAR_LEFT_EDGE
//   FAULT REAR_RIGHT_EDGE
//   FAULT FRONT_OBSTACLE
//   FAULT CLEAR
//
// Current hardware mapping:
//   L298N: ENA=5 IN1=22 IN2=23 IN3=24 IN4=25 ENB=6
//   Ultrasonic: TRIG=30 ECHO=31
//   Edge sensors: FL=32 FR=33 RL=34 RR=35
//   Encoders reserved: L=2 R=3 (not used in telemetry yet)
//
// IMPORTANT:
// - Do not connect L298N 5V to Arduino 5V
// - Keep common GND between Arduino, L298N, and bench PSU
// - This sketch preserves the serial contract used by Raspberry Pi

#include <string.h>
#include <stdio.h>

// ------------------------------
// Motor pins - L298N
// ------------------------------
const int ENA = 5;
const int IN1 = 22;
const int IN2 = 23;
const int IN3 = 24;
const int IN4 = 25;
const int ENB = 6;

// Flip these only if physical motor direction is reversed
bool LEFT_FORWARD_IN1_HIGH = true;
bool RIGHT_FORWARD_IN3_HIGH = true;

// ------------------------------
// Timing / safety
// ------------------------------
const unsigned long CMD_TIMEOUT_MS = 300;
const unsigned long TELEMETRY_MS = 100;
const unsigned long ULTRASONIC_TIMEOUT_US = 30000UL;

// ------------------------------
// Sensor pins
// ------------------------------
const int US_TRIG_PIN = 30;
const int US_ECHO_PIN = 31;

const int IR_FL_PIN = 32;
const int IR_FR_PIN = 33;
const int IR_RL_PIN = 34;
const int IR_RR_PIN = 35;

// Encoders reserved for later integration
const int ENC_L_PIN = 2;
const int ENC_R_PIN = 3;

// Proven from your real test:
// 0 = floor present
// 1 = edge / no floor
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

enum FaultCode {
  FAULT_NONE = 0,
  FAULT_FRONT_LEFT_EDGE,
  FAULT_FRONT_RIGHT_EDGE,
  FAULT_REAR_LEFT_EDGE,
  FAULT_REAR_RIGHT_EDGE,
  FAULT_FRONT_OBSTACLE
};

FaultCode activeFault = FAULT_NONE;

// ------------------------------
// Motor helpers - L298N style
// ------------------------------
void stopMotors() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void driveLeftMotor(int value) {
  int speed = constrain(abs(value), 0, 255);

  if (value == 0) {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, 0);
    return;
  }

  bool forward = (value > 0);
  bool in1High = forward ? LEFT_FORWARD_IN1_HIGH : !LEFT_FORWARD_IN1_HIGH;

  digitalWrite(IN1, in1High ? HIGH : LOW);
  digitalWrite(IN2, in1High ? LOW : HIGH);
  analogWrite(ENA, speed);
}

void driveRightMotor(int value) {
  int speed = constrain(abs(value), 0, 255);

  if (value == 0) {
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
    analogWrite(ENB, 0);
    return;
  }

  bool forward = (value > 0);
  bool in3High = forward ? RIGHT_FORWARD_IN3_HIGH : !RIGHT_FORWARD_IN3_HIGH;

  digitalWrite(IN3, in3High ? HIGH : LOW);
  digitalWrite(IN4, in3High ? LOW : HIGH);
  analogWrite(ENB, speed);
}

void applyDrive(int left, int right) {
  driveLeftMotor(left);
  driveRightMotor(right);
}

void armCommandWatchdog() {
  lastCmdMs = millis();
}

void forceSafetyStop() {
  currentLeft = 0;
  currentRight = 0;
  stopMotors();
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

  // Reserved only; not used yet
  pinMode(ENC_L_PIN, INPUT_PULLUP);
  pinMode(ENC_R_PIN, INPUT_PULLUP);

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
// Fault helpers
// ------------------------------
void reportFault(FaultCode fault) {
  if (fault == activeFault) {
    return;
  }

  activeFault = fault;

  switch (fault) {
    case FAULT_FRONT_LEFT_EDGE:
      Serial.println("FAULT FRONT_LEFT_EDGE");
      break;
    case FAULT_FRONT_RIGHT_EDGE:
      Serial.println("FAULT FRONT_RIGHT_EDGE");
      break;
    case FAULT_REAR_LEFT_EDGE:
      Serial.println("FAULT REAR_LEFT_EDGE");
      break;
    case FAULT_REAR_RIGHT_EDGE:
      Serial.println("FAULT REAR_RIGHT_EDGE");
      break;
    case FAULT_FRONT_OBSTACLE:
      Serial.println("FAULT FRONT_OBSTACLE");
      break;
    case FAULT_NONE:
    default:
      Serial.println("FAULT CLEAR");
      break;
  }
}

FaultCode getUnsafeFault(int left, int right) {
  RawIrState raw = readRawIrSensors();
  EdgeState e = interpretEdgeSensors(raw);

  // Per-wheel cliff safety
  if (left > 0 && e.fl) {
    return FAULT_FRONT_LEFT_EDGE;
  }

  if (left < 0 && e.rl) {
    return FAULT_REAR_LEFT_EDGE;
  }

  if (right > 0 && e.fr) {
    return FAULT_FRONT_RIGHT_EDGE;
  }

  if (right < 0 && e.rr) {
    return FAULT_REAR_RIGHT_EDGE;
  }

  // Ultrasonic only matters if there is any forward component
  if ((left > 0 || right > 0) && lastUsCm > 0 && lastUsCm < SAFE_FRONT_CM) {
    return FAULT_FRONT_OBSTACLE;
  }

  return FAULT_NONE;
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
  activeFault = FAULT_NONE;
  armCommandWatchdog();
  Serial.println("ACK RESET");
}

void handleVelCommand(int left, int right) {
  int nextLeft = constrain(left, -255, 255);
  int nextRight = constrain(right, -255, 255);

  updateSensorSnapshot();

  FaultCode fault = getUnsafeFault(nextLeft, nextRight);
  if (fault != FAULT_NONE) {
    forceSafetyStop();
    reportFault(fault);
    armCommandWatchdog();
    return;
  }

  if (activeFault != FAULT_NONE) {
    reportFault(FAULT_NONE);
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

  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENB, OUTPUT);

  stopMotors();
  setupSensors();
  armCommandWatchdog();
  updateSensorSnapshot();

  Serial.println("Hotel Robot Day 12 L298N Safety Controller Ready");
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
    FaultCode fault = getUnsafeFault(currentLeft, currentRight);

    if (fault != FAULT_NONE) {
      forceSafetyStop();
      reportFault(fault);
    } else {
      if (activeFault != FAULT_NONE) {
        reportFault(FAULT_NONE);
      }
      applyDrive(currentLeft, currentRight);
    }
  }

  if ((millis() - lastTelemetryMs) >= TELEMETRY_MS) {
    lastTelemetryMs = millis();
    sendTelemetry();
  }

  delay(5);
}
