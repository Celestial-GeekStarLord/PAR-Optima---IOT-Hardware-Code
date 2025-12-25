// ================= MOTOR PINS =================

// Left motor
#define IN1 32
#define IN2 33
#define ENA 22   // PWM

// Right motor
#define IN3 25
#define IN4 26
#define ENB 23   // PWM

// ================= PWM SETTINGS =================
#define PWM_FREQ 1000
#define PWM_RES  8   // 0–255

// ================= SPEED CONTROL =================
int motorSpeed = 250;

// ================= SETUP =================
void setup() {
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // NEW ESP32 CORE 3.x PWM ATTACH
  ledcAttach(ENA, PWM_FREQ, PWM_RES);
  ledcAttach(ENB, PWM_FREQ, PWM_RES);

  stopMotors();
}

// ================= LOOP =================
void loop() {

  moveForward(motorSpeed);
  delay(2000);

  motorSpeed = 120;
  moveBackward(motorSpeed);
  delay(2000);

  motorSpeed = 150;
  turnLeft(motorSpeed);
  delay(1500);

  motorSpeed = 200;
  turnRight(motorSpeed);
  delay(1500);

  stopMotors();
  delay(2000);
}

// ================= MOTOR FUNCTIONS =================

void moveForward(int speed) {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  ledcWrite(ENA, speed);
  ledcWrite(ENB, speed);
}

void moveBackward(int speed) {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  ledcWrite(ENA, speed);
  ledcWrite(ENB, speed);
}

void turnLeft(int speed) {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  ledcWrite(ENA, speed);
  ledcWrite(ENB, speed);
}

void turnRight(int speed) {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  ledcWrite(ENA, speed);
  ledcWrite(ENB, speed);
}

void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  ledcWrite(ENA, 0);
  ledcWrite(ENB, 0);
}
