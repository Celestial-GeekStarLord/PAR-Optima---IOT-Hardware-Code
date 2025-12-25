#include <WiFi.h>
#include <Firebase_ESP_Client.h>

// ================== WIFI ==================
#define WIFI_SSID "DESKTOP"
#define WIFI_PASSWORD "00000000"

// ================== FIREBASE ==================
#define API_KEY "AIzaSyDUnszYKw5mY-EFOVMYK2W1gLcVQmhw-RM"
#define DATABASE_URL "https://par-optima-default-rtdb.asia-southeast1.firebasedatabase.app/"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// ================== MOTOR PINS ==================
#define IN1 32
#define IN2 33
#define IN3 25
#define IN4 26
#define ENA 22
#define ENB 23

// ================== IR SENSOR ==================
#define S1 4
#define S2 5
#define S3 18
#define S4 19
#define S5 21

// ================== ULTRASONIC ==================
#define TRIG_PIN 27
#define ECHO_PIN 14
#define OBSTACLE_DISTANCE 20

// ================== PWM ==================
#define PWM_FREQ 1000
#define PWM_RES 8
#define CH_LEFT 0
#define CH_RIGHT 1
int motorSpeed = 180;

// ================== STATE ==================
char targetRoom = 'x';
bool navigating = false;

// ================== SETUP ==================
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

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  Serial.println("Robot Ready (Firebase + Line Following)");
}

// ================== MAIN LOOP ==================
void loop() {

  if (Firebase.RTDB.getString(&fbdo, "/robot/command")) {
    String cmd = fbdo.stringData();
    if (cmd.length() > 0) {
      targetRoom = cmd.charAt(0);
      navigating = (targetRoom == 'w' || targetRoom == 'a' || targetRoom == 's');
    }
  }

  if (!navigating) {
    stopRobot();
    return;
  }

  if (getDistance() <= OBSTACLE_DISTANCE) {
    stopRobot();
    return;
  }

  followLine();
}

// ================== LINE FOLLOW ==================
void followLine() {

  int s1 = digitalRead(S1);
  int s2 = digitalRead(S2);
  int s3 = digitalRead(S3);
  int s4 = digitalRead(S4);
  int s5 = digitalRead(S5);

  if (s1 && s2 && s3 && s4 && s5) {
    stopRobot();
    delay(500);
    rotate180();
    navigating = false;
    Firebase.RTDB.setString(&fbdo, "/robot/command", "x");
    return;
  }

  if ((s1 == 0 || s5 == 0) && s3 == 0) {
    stopRobot(); delay(200);

    if (targetRoom == 'a' && s1 == 0) turnLeftTimed();
    else if (targetRoom == 'w' && s5 == 0) turnRightTimed();
    else if (targetRoom == 's') moveForward();

    return;
  }

  if (s2 == 0 && s3 == 0 && s4 == 0) moveForward();
  else if (s1 == 0 || s2 == 0) turnLeft();
  else if (s4 == 0 || s5 == 0) turnRight();
}

// ================== ULTRASONIC ==================
long getDistance() {
  digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long d = pulseIn(ECHO_PIN, HIGH, 50000);
  return d ? d * 0.034 / 2 : 999;
}

// ================== MOTOR ==================
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

// ================== ROTATION ==================
void turnLeftTimed() { turnLeft(); delay(450); stopRobot(); }
void turnRightTimed() { turnRight(); delay(450); stopRobot(); }
void rotate180() { turnLeft(); delay(900); stopRobot(); }