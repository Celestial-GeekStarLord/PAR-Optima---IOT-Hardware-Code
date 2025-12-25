#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include <time.h>

/* ================= WIFI ================= */
#define WIFI_SSID "Subash's OnePlus Nord 3 5G"
#define WIFI_PASSWORD "123456780"

/* ================= FIREBASE ================= */
#define API_KEY "AIzaSyDUnszYKw5mY-EFOVMYK2W1gLcVQmhw-RM"
#define DATABASE_URL "https://par-optima-default-rtdb.asia-southeast1.firebasedatabase.app/"

/* ================= FIREBASE OBJECTS ================= */
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

/* ================= MOTOR PINS ================= */
#define IN1 32
#define IN2 33
#define IN3 25
#define IN4 26

/* ================= IR SENSORS ================= */
#define S1 4
#define S3 18
#define S5 21

/* ================= STATE ================= */
char command = 'x';
bool navigating = false;

/* ================= TIME ================= */
void initTime() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  Serial.print("Syncing time");
  time_t now = time(nullptr);
  while (now < 100000) {
    Serial.print(".");
    delay(500);
    now = time(nullptr);
  }
  Serial.println("\n⏰ Time synced");
}

/* ================= SETUP ================= */
void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("Booting...");

  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(S1, INPUT);
  pinMode(S3, INPUT);
  pinMode(S5, INPUT);

  stopRobot();

  /* ===== WIFI ===== */
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println("\n✅ WiFi Connected");

  /* ===== TIME (CRITICAL FOR SSL) ===== */
  initTime();

  /* ===== FIREBASE ===== */
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (!Firebase.signUp(&config, &auth, "", "")) {
    Serial.printf("❌ Firebase auth failed: %s\n", config.signer.signupError.message.c_str());
    while (true); // STOP HERE — auth must succeed
  }

  Serial.println("✅ Firebase auth OK");

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  Firebase.RTDB.setString(&fbdo, "/cmd", "x");

  Serial.println("🤖 READY");
  Serial.println("w → Room 1 | a → Room 2 | s → Room 3");
}

/* ================= LOOP ================= */
void loop() {

  if (!Firebase.ready()) return;

  if (!navigating && Firebase.RTDB.getString(&fbdo, "/cmd")) {
    String cmd = fbdo.stringData();

    if (cmd == "w" || cmd == "a" || cmd == "s") {
      command = cmd.charAt(0);
      navigating = true;

      Serial.print("▶ Firebase Command: ");
      Serial.println(command);

      Firebase.RTDB.setString(&fbdo, "/cmd", "x");
    }
  }

  if (!navigating) return;

  if (command == 'w') goRoom1();
  else if (command == 'a') goRoom2();
  else if (command == 's') goRoom3();

  navigating = false;
  command = 'x';
}

/* ================= ROOM 1 ================= */
void goRoom1() {
  if (digitalRead(S5) == LOW) turnRight90();

  while (digitalRead(S3) == LOW) moveForward();

  stopRobot();
  Serial.println("Room 1 reached");
}

/* ================= ROOM 2 ================= */
void goRoom2() {
  while (true) {
    moveForward();
    if (digitalRead(S1) == LOW) {
      stopRobot();
      delay(200);
      turnLeft90();
      break;
    }
  }

  while (digitalRead(S3) == LOW) moveForward();

  stopRobot();
  Serial.println("Room 2 reached");
}

/* ================= ROOM 3 ================= */
void goRoom3() {
  int rightCount = 0;

  while (true) {
    moveForward();

    if (digitalRead(S5) == LOW && digitalRead(S3) == LOW) {
      rightCount++;
      Serial.println(rightCount);
      delay(250);
    }

    if (rightCount == 2 && digitalRead(S5) == LOW) {
      stopRobot();
      delay(200);
      turnRight90();
      break;
    }
  }

  while (digitalRead(S3) == LOW) moveForward();

  stopRobot();
  Serial.println("Room 3 reached");
}

/* ================= MOTOR CONTROL ================= */
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
void turnRight90() {
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
  delay(450);
  stopRobot();
}

void turnLeft90() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);
  delay(450);
  stopRobot();
}