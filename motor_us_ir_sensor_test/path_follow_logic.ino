/* ================= MOTOR ================= */
#define IN1 32
#define IN2 33
#define IN3 25
#define IN4 26

/* ================= PWM ================= */
#define ENA 22  // Left motor speed control (PWM)
#define ENB 23  // Right motor speed control (PWM)

int motorSpeed = 200;  // Constant motor speed

/* ================= IR ================= */
#define S1 4   // Leftmost IR sensor
#define S3 18  // Middle IR sensor
#define S5 21  // Rightmost IR sensor

/* ================= SETUP ================= */
void setup() {
  Serial.begin(115200);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(S1, INPUT);
  pinMode(S3, INPUT);
  pinMode(S5, INPUT);

  // Setup PWM for motor speed control
  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);

  // Set initial PWM values
  analogWrite(ENA, motorSpeed);  // Set motor speed for left motor
  analogWrite(ENB, motorSpeed);  // Set motor speed for right motor

  stopRobot();  // Make sure the robot stops at the start
}

/* ================= LOOP ================= */
void loop() {
  followLine();
  delay(50);  // Short delay for smoother control
}

/* ================= LINE FOLLOW ================= */
void followLine() {
  int s1 = digitalRead(S1);  // Leftmost IR sensor
  int s3 = digitalRead(S3);  // Middle IR sensor
  int s5 = digitalRead(S5);  // Rightmost IR sensor

  // If S3 (middle) detects the line, move forward
  if (s3 == LOW) {
    moveForward();
  } 
  // If S5 (rightmost) detects the line, turn right
  else if (s5 == LOW) {
    turnRight();
  }
  // If S1 (leftmost) detects the line, turn left
  else if (s1 == LOW) {
    turnLeft();
  } 
  // If no line is detected, stop the robot
  else {
    stopRobot();
  }
}

/* ================= MOTOR CONTROL ================= */
void moveForward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  // Motor speed is constant at 200
  analogWrite(ENA, motorSpeed);
  analogWrite(ENB, motorSpeed);
}

void moveBackward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  // Motor speed is constant at 200
  analogWrite(ENA, motorSpeed);
  analogWrite(ENB, motorSpeed);
}

void turnRight() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  // Reduced speed for turning
  analogWrite(ENA, motorSpeed - 40);
  analogWrite(ENB, motorSpeed - 40);
}

void turnLeft() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  // Reduced speed for turning
  analogWrite(ENA, motorSpeed - 40);
  analogWrite(ENB, motorSpeed - 40);
}

void stopRobot() {
  analogWrite(ENA, 0);  // Stop left motor
  analogWrite(ENB, 0);  // Stop right motor
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}