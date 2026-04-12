// Standalone IR + Ultrasonic debug sketch
// No motor logic. Pure sensor truth testing.

const int US_TRIG_PIN = 8;
const int US_ECHO_PIN = 9;

const int IR_FL_PIN = 10;
const int IR_FR_PIN = 11;
const int IR_RL_PIN = 12;
const int IR_RR_PIN = 13;

// Start with LOW. If interpretation is reversed, change to HIGH.
const int EDGE_ACTIVE_STATE = HIGH;

const unsigned long TELEMETRY_MS = 200;
const unsigned long ULTRASONIC_TIMEOUT_US = 30000UL;
unsigned long lastTelemetryMs = 0;

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

void sendDebugTelemetry() {
  int us_cm = readUltrasonicCm();
  RawIrState raw = readRawIrSensors();
  EdgeState e = interpretEdgeSensors(raw);

  bool front_edge = e.fl || e.fr;
  bool rear_edge = e.rl || e.rr;

  Serial.print("DBG {\"us_cm\":");
  Serial.print(us_cm);

  Serial.print(",\"raw\":{\"fl\":");
  Serial.print(raw.fl);
  Serial.print(",\"fr\":");
  Serial.print(raw.fr);
  Serial.print(",\"rl\":");
  Serial.print(raw.rl);
  Serial.print(",\"rr\":");
  Serial.print(raw.rr);
  Serial.print("}");

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
  setupSensors();
  Serial.println("IR + Ultrasonic debug ready");
}

void loop() {
  if (millis() - lastTelemetryMs >= TELEMETRY_MS) {
    lastTelemetryMs = millis();
    sendDebugTelemetry();
  }
}