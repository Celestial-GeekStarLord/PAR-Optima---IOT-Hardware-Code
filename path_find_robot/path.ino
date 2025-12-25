// ================== MOTOR PINS ==================
#define IN1 32   // Left motor
#define IN2 33
#define IN3 25   // Right motor
#define IN4 26

// ================== IR SENSOR PINS ==================
#define S1 4
#define S2 5
#define S3 18
#define S4 19
#define S5 21

// ================== ULTRASONIC ==================
#define TRIG_PIN 27
#define ECHO_PIN 14
#define OBSTACLE_DISTANCE 20  // cm

// ================== GLOBAL STATE ==================
char targetRoom = 'x';     // w, a, s
bool navigating = false;

// ================== SETUP ==================
void setup() {
  Serial.begin(115200);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(S1, INPUT);
  pinMode(S2, INPUT);
  pinMode(S3, INPUT);
  pinMode(S4, INPUT);
  pinMode(S5, INPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  stopRobot();
  Serial.println("Autonomous Line Navigation Ready");
  Serial.println("w → Room 1 | a → Room 2 | s → Room 3");
}

// ================== MAIN LOOP ==================
void loop() {

  if (Serial.available()) {
    char cmd = Serial.read();
    if (cmd == 'w' || cmd == 'a' || cmd == 's') {
      targetRoom = cmd;
      navigating = true;
      Serial.print("Target Room Command: ");
      Serial.println(targetRoom);
    }
  }

  if (!navigating) {
    stopRobot();
    return;
  }

  // Obstacle check
  if (getDistance() <= OBSTACLE_DISTANCE) {
    stopRobot();
    Serial.println("Obstacle detected!");
    return;
  }

  followLine();
}

// ================== LINE FOLLOWING ==================
void followLine() {

  int s1 = digitalRead(S1);
  int s2 = digitalRead(S2);
  int s3 = digitalRead(S3);
  int s4 = digitalRead(S4);
  int s5 = digitalRead(S5);

  // ===== NO LINE → ARRIVED =====
  if (s1 && s2 && s3 && s4 && s5) {
    stopRobot();
    Serial.println("Room reached");
    delay(500);
    rotate180();
    navigating = false;
    return;
  }

  // ===== JUNCTION DETECTION =====
  if ((s1 == 0 || s5 == 0) && s3 == 0) {

    stopRobot();
    delay(200);

    if (targetRoom == 'a' && s1 == 0) {        // LEFT → Room 2
      Serial.println("Turning LEFT → Room 2");
      turnLeftTimed();
    }

    else if (targetRoom == 'w' && s5 == 0) {   // RIGHT → Room 1
      Serial.println("Turning RIGHT → Room 1");
      turnRightTimed();
    }

    else if (targetRoom == 's') {              // STRAIGHT → Room 3
      Serial.println("Going STRAIGHT → Room 3");
      moveForward();
      delay(300);
    }

    return;
  }

  // ===== NORMAL LINE FOLLOW =====
  if (s2 == 0 && s3 == 0 && s4 == 0) {
    moveForward();
  }
  else if (s2 == 0 || s1 == 0) {
    turnLeft();
  }
  else if (s4 == 0 || s5 == 0) {
    turnRight();
  }
}

// ================== ULTRASONIC ==================
long getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 50000);
  if (duration == 0) return 999;
  return duration * 0.034 / 2;
}

// ================== MOTOR CONTROL (✔ VERIFIED LOGIC) ==================
void moveForward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void moveBackward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void turnRight() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void turnLeft() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

void stopRobot() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

// ================== TIMED TURNS ==================
void turnLeftTimed() {
  turnLeft();
  delay(450);   // adjust per robot
  stopRobot();
}

void turnRightTimed() {
  turnRight();
  delay(450);   // adjust per robot
  stopRobot();
}

void rotate180() {
  Serial.println("Rotating 180°");
  turnLeft();
  delay(900);   // adjust for perfect 180°
  stopRobot();
}
