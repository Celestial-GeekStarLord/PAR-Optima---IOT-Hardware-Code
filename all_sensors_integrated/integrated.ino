#include <ESP32Servo.h>

/* ===================== MOTOR PINS ===================== */
#define IN1 32   // Left motor
#define IN2 33
#define IN3 25   // Right motor
#define IN4 26

/* ===================== ULTRASONIC PINS ===================== */
#define TRIG_PIN 27
#define ECHO_PIN 14
#define OBSTACLE_DISTANCE 20   // cm

/* ===================== IR SENSOR PINS ===================== */
#define S1 4
#define S2 5
#define S3 18
#define S4 19
#define S5 21

/* ===================== SERVO ===================== */
#define SERVO_PIN 13
Servo myServo;

/* ===================== VARIABLES ===================== */
char currentCommand = 'x';

/* ===================== SETUP ===================== */
void setup() {
  Serial.begin(115200);

  // Motor pins
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Ultrasonic pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // IR sensor pins
  pinMode(S1, INPUT);
  pinMode(S2, INPUT);
  pinMode(S3, INPUT);
  pinMode(S4, INPUT);
  pinMode(S5, INPUT);

  // Servo setup
  myServo.setPeriodHertz(50);
  myServo.attach(SERVO_PIN, 500, 2400);
  myServo.write(90); // center position

  stopRobot();

  Serial.println("=================================");
  Serial.println("ESP32 Robot System Ready");
  Serial.println("Controls: w s a d x");
  Serial.println("Obstacle detection: ENABLED");
  Serial.println("IR Sensors + Servo: ACTIVE");
  Serial.println("=================================");
}

/* ===================== LOOP ===================== */
void loop() {

  /* ---------- Read motor command ---------- */
  if (Serial.available()) {
    char cmd = Serial.read();
    if (cmd != '\n' && cmd != '\r') {
      currentCommand = cmd;
      Serial.print("Command: ");
      Serial.println(currentCommand);
    }
  }

  /* ---------- Ultrasonic distance ---------- */
  long distance = getDistance();
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  /* ---------- IR sensor reading ---------- */
  readIRSensors();

  /* ---------- Obstacle logic ---------- */
  if (distance <= OBSTACLE_DISTANCE) {
    stopRobot();
    Serial.println("⚠ OBSTACLE DETECTED → ROBOT STOPPED");
  } else {
    executeCommand(currentCommand);
  }

  delay(100);
}

/* ===================== ULTRASONIC FUNCTION ===================== */
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

/* ===================== IR SENSOR FUNCTION ===================== */
void readIRSensors() {
  Serial.print("IR → ");
  Serial.print(digitalRead(S1)); Serial.print(" ");
  Serial.print(digitalRead(S2)); Serial.print(" ");
  Serial.print(digitalRead(S3)); Serial.print(" ");
  Serial.print(digitalRead(S4)); Serial.print(" ");
  Serial.println(digitalRead(S5));
}

/* ===================== COMMAND HANDLER ===================== */
void executeCommand(char cmd) {
  switch (cmd) {
    case 'w': moveForward();  break;
    case 's': moveBackward(); break;
    case 'a': turnLeft();     break;
    case 'd': turnRight();    break;
    case 'x': stopRobot();    break;
  }
}

/* ===================== MOTOR FUNCTIONS ===================== */
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

void turnLeft() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void turnRight() {
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