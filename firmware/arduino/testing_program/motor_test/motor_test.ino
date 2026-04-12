const int ENA = 5;
const int IN1 = 22;
const int IN2 = 23;
const int IN3 = 24;
const int IN4 = 25;
const int ENB = 6;

int SPEED = 150;

void stopMotors() {
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  Serial.println("ACTION: STOP");
}

void forward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  analogWrite(ENA, SPEED);
  analogWrite(ENB, SPEED);

  Serial.println("ACTION: FORWARD");
}

void backward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  analogWrite(ENA, SPEED);
  analogWrite(ENB, SPEED);

  Serial.println("ACTION: BACKWARD");
}

void leftPivot() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  analogWrite(ENA, SPEED);
  analogWrite(ENB, SPEED);

  Serial.println("ACTION: LEFT_PIVOT");
}

void rightPivot() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  analogWrite(ENA, SPEED);
  analogWrite(ENB, SPEED);

  Serial.println("ACTION: RIGHT_PIVOT");
}

void leftOnly() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  analogWrite(ENA, SPEED);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  analogWrite(ENB, 0);

  Serial.println("ACTION: LEFT_MOTOR_ONLY");
}

void rightOnly() {
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  analogWrite(ENB, SPEED);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  analogWrite(ENA, 0);

  Serial.println("ACTION: RIGHT_MOTOR_ONLY");
}

void setup() {
  Serial.begin(115200);

  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  stopMotors();

  Serial.println("");
  Serial.println("===== MOTOR TEST CONSOLE =====");
  Serial.println("w = forward");
  Serial.println("s = backward");
  Serial.println("a = pivot left");
  Serial.println("d = pivot right");
  Serial.println("1 = left motor only");
  Serial.println("2 = right motor only");
  Serial.println("x = stop");
  Serial.println("===============================");
}

void loop() {

  if (Serial.available()) {

    char cmd = Serial.read();

    if (cmd == 'w') forward();
    else if (cmd == 's') backward();
    else if (cmd == 'a') leftPivot();
    else if (cmd == 'd') rightPivot();
    else if (cmd == '1') leftOnly();
    else if (cmd == '2') rightOnly();
    else if (cmd == 'x') stopMotors();

  }

}