volatile long leftCount = 0;
volatile long rightCount = 0;

const int LEFT_ENC = 2;
const int RIGHT_ENC = 3;

void leftISR() {
  leftCount++;
}

void rightISR() {
  rightCount++;
}

void setup() {
  Serial.begin(115200);

  pinMode(LEFT_ENC, INPUT_PULLUP);
  pinMode(RIGHT_ENC, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(LEFT_ENC), leftISR, FALLING);
  attachInterrupt(digitalPinToInterrupt(RIGHT_ENC), rightISR, FALLING);

  Serial.println("Encoder test start");
}

void loop() {
  static unsigned long last = 0;
  if (millis() - last >= 500) {
    last = millis();
    Serial.print("L=");
    Serial.print(leftCount);
    Serial.print(" R=");
    Serial.println(rightCount);
  }
}