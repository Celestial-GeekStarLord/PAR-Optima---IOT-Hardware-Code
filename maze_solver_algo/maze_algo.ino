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
#define S1 4    // Left
#define S3 18   // Center
#define S5 21   // Right

/* ================= STATE ================= */
char command = 'x';
bool navigating = false;

/* ================= PATH MEMORY ================= */
char path[100] = "";
uint8_t path_length = 0;

/* ================= TIME ================= */
void initTime() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  time_t now = time(nullptr);
  while (now < 100000) {
    Serial.print(".");
    delay(500);
    now = time(nullptr);
  }
  Serial.println("\n⏰ Time synced");
}

/* ================= LINE DETECTION ================= */
bool lineDetected() {
  return digitalRead(S3) == LOW;
}

/* ================= MOTOR CONTROL ================= */
void moveForward() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
}

void stopRobot() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); digitalWrite(IN4, LOW);
}

void turnRight90() {
  digitalWrite(IN1, HIGH); digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); digitalWrite(IN4, LOW);
  delay(500); // adjust for your robot
  stopRobot();
  delay(100);
}

void turnLeft90() {
  digitalWrite(IN1, LOW); digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW); digitalWrite(IN4, HIGH);
  delay(500);
  stopRobot();
  delay(100);
}

/* ================= PATH LOGIC ================= */
char selectTurn(bool left, bool straight, bool right) {
  if (left) return 'L';
  if (straight) return 'S';
  if (right) return 'R';
  return 'B';
}

void simplifyPath() {
  if (path_length < 3 || path[path_length-2] != 'B') return;
  int angle = 0;
  for (int i=1;i<=3;i++) {
    switch(path[path_length-i]) {
      case 'L': angle += 270; break;
      case 'R': angle += 90; break;
      case 'B': angle += 180; break;
    }
  }
  angle %= 360;
  switch(angle) {
    case 0: path[path_length-3]='S'; break;
    case 90: path[path_length-3]='R'; break;
    case 180: path[path_length-3]='B'; break;
    case 270: path[path_length-3]='L'; break;
  }
  path_length -= 2;
}

/* ================= ROOM NAVIGATION ================= */
void followSegment() {
  while (lineDetected()) {
    moveForward();
    delay(50);
  }
  stopRobot();
}

void navigateRoom(char room) {
  Serial.print("Navigating to Room "); Serial.println(room);

  while (lineDetected()) {
    bool left = digitalRead(S1) == LOW;
    bool right = digitalRead(S5) == LOW;
    bool straight = lineDetected();

    char dir = selectTurn(left, straight, right);
    if ((room=='w' && right) || (room=='a' && left) || (room=='s' && right && path_length==1)) {
      stopRobot();
      delay(200);
      if (dir=='R') turnRight90();
      else if (dir=='L') turnLeft90();
      path[path_length++] = dir;
      simplifyPath();
      break;
    }

    moveForward();
    delay(50);
  }

  // move until line ends
  followSegment();
  Serial.print("Room "); Serial.print(room); Serial.println(" reached");
}

/* ================= SETUP ================= */
void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("Booting...");

  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  pinMode(S1, INPUT); pinMode(S3, INPUT); pinMode(S5, INPUT);

  stopRobot();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) { Serial.print("."); delay(300); }
  Serial.println("\n✅ WiFi Connected");

  initTime();

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (!Firebase.signUp(&config, &auth, "", "")) {
    Serial.printf("❌ Firebase auth failed: %s\n", config.signer.signupError.message.c_str());
    while (true);
  }
  Serial.println("✅ Firebase auth OK");

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  Firebase.RTDB.setString(&fbdo, "/cmd", "x");

  Serial.println("🤖 READY");
}

/* ================= LOOP ================= */
void loop() {
  if (!Firebase.ready()) return;

  if (!navigating && Firebase.RTDB.getString(&fbdo, "/cmd")) {
    String cmd = fbdo.stringData();
    if (cmd=="w"||cmd=="a"||cmd=="s") {
      command = cmd.charAt(0);
      navigating = true;
      Firebase.RTDB.setString(&fbdo, "/cmd", "x");
      Serial.print("▶ Firebase Command: "); Serial.println(command);
    }
  }

  if (!navigating || !lineDetected()) {
    stopRobot();
    navigating = false;
    return;
  }

  if (command=='w') navigateRoom('w');
  else if (command=='a') navigateRoom('a');
  else if (command=='s') navigateRoom('s');

  navigating = false;
}