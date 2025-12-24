  // ===== MOTOR PINS =====
  #define IN1 32
  #define IN2 33
  #define IN3 25
  #define IN4 26

  void setup() {
    Serial.begin(115200);

    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);

    stopRobot();

    Serial.println("Robot Motor Control Ready");
    Serial.println("Press: w s a d x");
  }

  void loop() {
    if (Serial.available() > 0) {
      char command = Serial.read();

      // Ignore newline characters
      if (command == '\n' || command == '\r') return;

      Serial.print("Received: ");
      Serial.println(command);

      switch (command) {
        case 'w':
          moveForward();
          break;

        case 's':
          moveBackward();
          break;

        case 'a':
          turnLeft();
          break;

        case 'd':
          turnRight();
          break;

        case 'x':
          stopRobot();
          break;
      }
    }
  }

  // ===== MOTOR FUNCTIONS =====

  void moveForward() {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
  }

  void moveBackward() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
  }

  void turnLeft() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
  }

  void turnRight() {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
  }

  void stopRobot() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, LOW);
  }