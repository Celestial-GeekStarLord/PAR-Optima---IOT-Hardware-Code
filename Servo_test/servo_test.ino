#include <ESP32Servo.h>

Servo myServo;

#define SERVO_PIN 13   // ❗ DO NOT use 34–39

void setup() {
  Serial.begin(115200);
  Serial.println("SG90 Servo Test using ESP32Servo");

  myServo.setPeriodHertz(50);     // SG90 uses 50Hz
  myServo.attach(SERVO_PIN, 500, 2400);  // min/max pulse width (µs)
}

void loop() {
  Serial.println("0 degree");
  myServo.write(0);
  delay(2000);

  Serial.println("90 degree");
  myServo.write(90);
  delay(2000);

  Serial.println("180 degree");
  myServo.write(180);
  delay(2000);
}