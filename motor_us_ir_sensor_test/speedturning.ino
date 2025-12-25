#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "time.h"  // For NTP time sync

/* ================= WIFI ================= */
#define WIFI_SSID "Subash's OnePlus Nord 3 5G"
#define WIFI_PASSWORD "123456780"

/* ================= FIREBASE ================= */
#define API_KEY "AIzaSyDUnszYKw5mY-EFOVMYK2W1gLcVQmhw-RM"
#define DATABASE_URL "https://par-optima-default-rtdb.asia-southeast1.firebasedatabase.app/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

/* ================= MOTOR ================= */
#define IN1 32
#define IN2 33
#define IN3 25
#define IN4 26
#define ENA 22
#define ENB 23

/* ================= IR ================= */
#define S1 4
#define S2 5
#define S3 18
#define S4 19
#define S5 21

/* ================= ULTRASONIC ================= */
#define TRIG_PIN 27
#define ECHO_PIN 14
#define OBSTACLE_DISTANCE 20

/* ================= PWM ================= */
#define PWM_FREQ 1000
#define PWM_RES 8
#define CH_LEFT 0
#define CH_RIGHT 1
int motorSpeed = 180;

/* ================= LEDC COMPATIBILITY FIX (ESP32 CORE 3.x) ================= */
#ifndef ledcSetup
bool ledcSetup(uint8_t channel, double freq, uint8_t resolution) {
  return true;   // dummy (ESP32 core 3.x)
}

void ledcAttachPin(uint8_t pin, uint8_t channel) {
  static bool attached[16] = {false};
  if (!attached[channel]) {
    ledcAttach(pin, PWM_FREQ, PWM_RES);  // new API
    attached[channel] = true;
  }
}
#endif
/* ========================================================================== */

/* ================= STATE ================= */
char targetRoom = 'x';
bool navigating = false;

/* ================= NTP TIME ================= */
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800; // Nepal UTC+5:45
const int daylightOffset_sec = 0;

/* ================= SETUP ================= */
void setup() {
  Serial.begin(115200);

  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);

  pinMode(S1, INPUT); pinMode(S2, INPUT);
  pinMode(S3, INPUT); pinMode(S4, INPUT); pinMode(S5, INPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  ledcSetup(CH_LEFT, PWM_FREQ, PWM_RES);
  ledcSetup(CH_RIGHT, PWM_FREQ, PWM_RES);
  ledcAttachPin(ENA, CH_LEFT);
  ledcAttachPin(ENB, CH_RIGHT);

  stopRobot();

  /* ===== WIFI ===== */
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi Connected");

  /* ===== SET TIME FOR SSL ===== */
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("⏰ NTP Time syncing...");
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\n⏰ Time synced");

  /* ===== FIREBASE ===== */
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // 🔑 Anonymous auth
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("✅ Firebase Anonymous Auth OK");
  } else {
    Serial.printf("❌ Firebase Auth Error: %s\n",
                  config.signer.signupError.message.c_str());
  }

  Firebase.RTDB.setString(&fbdo, "/cmd", "x");

  Serial.println("🤖 Robot Ready (Firebase + Line Follow)");
}

/* ================= LOOP ================= */
void loop() {

  if (Firebase.ready()) {
    if (Firebase.RTDB.getString(&fbdo, "/cmd")) {

      String cmd = fbdo.stringData();
      Serial.print("🔥 Firebase command: ");
      Serial.println(cmd);

      if (cmd.length() > 0) {
        targetRoom = cmd.charAt(0);
        navigating = (targetRoom == 'w');  // Only move when 'w' is received
      }
    } else {
      Serial.print("❌ RTDB read failed: ");
      Serial.println(fbdo.errorReason());
    }
  }

  if (!navigating) {
    stopRobot();
    delay(100);
    return;
  }

  if (getDistance() <= OBSTACLE_DISTANCE) {
    Serial.println("⚠ Obstacle detected");
    stopRobot();
    delay(100);
    return;
  }

  followLine();
  delay(50);   // Firebase stability
}

/* ================= LINE FOLLOW ================= */
void followLine() {
  int s1 = digitalRead(S1);
  int s3 = digitalRead(S3);
  int s5 = digitalRead(S5);

  // If S3 is active (middle), move forward
  if (s3 == 0) {
    moveForward();
  } 
  // If S5 is active (right side), turn right
  else if (s5 == 0) {
    turnRight();
  } 
  // If S1 is active (left side), turn left
  else if (s1 == 0) {
    turnLeft();
  }
  else {
    stopRobot(); // Stop if no line detected
  }
}

/* ================= ULTRASONIC ================= */
long getDistance() {
  digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long d = pulseIn(ECHO_PIN, HIGH, 50000);
  return d ? d * 0.034 / 2 : 999;
}

/* ================= MOTOR ================= */
void setSpeed(int spd) {
  ledcWrite(CH_LEFT, spd);
  ledcWrite(CH_RIGHT, spd);
}

void moveForward() {
  setSpeed(motorSpeed);
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void turnLeft() {
  setSpeed(motorSpeed - 40);
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
}

void turnRight() {
  setSpeed(motorSpeed - 40);
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void stopRobot() {
  setSpeed(0);
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
}

/* ================= ROTATION ================= */
void turnLeftTimed() { turnLeft(); delay(450); stopRobot(); }
void turnRightTimed() { turnRight(); delay(450); stopRobot(); }
void rotate180() { turnLeft(); delay(900); stopRobot(); }