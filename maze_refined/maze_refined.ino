#include <WiFi.h>
#include <Firebase_ESP_Client.h>

/* ================= WIFI ================= */
#define WIFI_SSID "Subash's OnePlus Nord 3 5G"
#define WIFI_PASSWORD "123456780"

/* ================= FIREBASE ================= */
#define API_KEY "AIzaSyDUnszYKw5mY-EFOVMYK2W1gLcVQmhw-RM"
#define DATABASE_URL "https://par-optima-default-rtdb.asia-southeast1.firebasedatabase.app/"

/* ================= FIREBASE ================= */
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

/* ================= MOTOR PINS ================= */
#define IN1 32
#define IN2 33
#define IN3 25
#define IN4 26

/* ================= IR SENSORS ================= */
#define S1 4    // LEFT
#define S3 18   // CENTER
#define S5 21   // RIGHT

/* ================= STATE ================= */
bool mazeRunning = false;
String lastCmd = "";

/* ================= SETUP ================= */
void setup() {
  Serial.begin(115200);

  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);

  pinMode(S1, INPUT);
  pinMode(S3, INPUT);
  pinMode(S5, INPUT);

  stopRobot();

  /* ===== WIFI ===== */
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected");

  /* ===== FIREBASE ===== */
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  Serial.println("Maze Robot Ready");
}

/* ================= LOOP ================= */
void loop() {

  if (Firebase.ready()) {
    if (Firebase.RTDB.getString(&fbdo, "/cmd")) {
      String cmd = fbdo.stringData();

      if (cmd != lastCmd) {
        lastCmd = cmd;

        if (cmd == "start") {
          mazeRunning = true;
          Serial.println("Maze START");
        }

        if (cmd == "stop") {
          mazeRunning = false;
          stopRobot();
          Serial.println("Maze STOP");
        }
      }
    }
  }

  if (!mazeRunning) return;

  mazeSolver();
}

/* ================= MAZE SOLVER ================= */
void mazeSolver() {

  int left   = digitalRead(S1);
  int center = digitalRead(S3);
  int right  = digitalRead(S5);

  /* ===== LEFT HAND RULE ===== */

  // LEFT AVAILABLE
  if (left == LOW && center == HIGH) {
    turnLeft90();
    moveForward();
  }

  // STRAIGHT
  else if (center == LOW) {
    moveForward();
  }

  // RIGHT AVAILABLE
  else if (right == LOW) {
    turnRight90();
    moveForward();
  }

  // DEAD END
  else {
    uTurn();
  }
}

/* ================= MOTOR ================= */
void moveForward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void stopRobot() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

/* ================= TURNS ================= */
void turnLeft90() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  delay(420);
  stopRobot();
}

void turnRight90() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  delay(420);
  stopRobot();
}

void uTurn() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  delay(850);
  stopRobot();
}
