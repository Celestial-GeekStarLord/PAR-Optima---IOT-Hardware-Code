// ===== MOTOR PINS =====
#define IN1 32   // Left motor
#define IN2 33
#define IN3 25   // Right motor
#define IN4 26

// ===== ULTRASONIC SENSOR PINS =====
#define TRIG_PIN 27
#define ECHO_PIN 14

// ===== OBSTACLE SETTINGS =====
#define OBSTACLE_DISTANCE 20   // cm

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

  stopRobot();

  Serial.println("Robot Motor + Continuous Ultrasonic Ready");
  Serial.println("Press: w s a d x");
}

char currentCommand = 'x'; // store the last key pressed

void loop() {
  // 1️⃣ Read new command if available
  if (Serial.available() > 0) {
    char command = Serial.read();
    if (command == '\n' || command == '\r') return;

    currentCommand = command;
    Serial.print("Command received: ");
    Serial.println(currentCommand);
  }

  // 2️⃣ Continuous distance measurement
  long distance = getDistance();
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");

  // 3️⃣ If obstacle too close, stop robot
  if (distance <= OBSTACLE_DISTANCE) {
    stopRobot();
    Serial.println("Obstacle detected! Robot STOPPED");
  } else {
    // Execute the last command if no obstacle
    switch (currentCommand) {
      case 'w': moveForward();  break;
      case 's': moveBackward(); break;
      case 'a': turnLeft();     break;
      case 'd': turnRight();    break;
      case 'x': stopRobot();    break;
    }
  }

  delay(50); // small delay for stability
}

// ===== ULTRASONIC FUNCTION =====
long getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 50000); // 50ms timeout
  if (duration == 0) return 999; // no echo detected

  long distance = duration * 0.034 / 2; // cm
  return distance;
}

// ===== MOTOR FUNCTIONS =====

// Forward
void moveForward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

// Backward
void moveBackward() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

// Turn left
void turnLeft() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

// Turn right
void turnRight() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
}

// Stop all motors
void stopRobot() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}
