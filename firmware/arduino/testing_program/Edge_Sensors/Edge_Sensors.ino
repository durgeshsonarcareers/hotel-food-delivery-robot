const int FL = 32;
const int FR = 33;
const int RL = 34;
const int RR = 35;

void setup() {
  Serial.begin(115200);
  pinMode(FL, INPUT);
  pinMode(FR, INPUT);
  pinMode(RL, INPUT);
  pinMode(RR, INPUT);
}

void loop() {
  Serial.print("FL=");
  Serial.print(digitalRead(FL));

  Serial.print(" FR=");
  Serial.print(digitalRead(FR));

  Serial.print(" RL=");
  Serial.print(digitalRead(RL));

  Serial.print(" RR=");
  Serial.println(digitalRead(RR));

  delay(200);
}