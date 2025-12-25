#include <WiFi.h>
#include <Firebase_ESP_Client.h>

// Provide the token generation process info.
#include <addons/TokenHelper.h>
// Provide the RTDB payload printing info.
#include <addons/RTDBHelper.h>

/* ================= WIFI CREDENTIALS ================= */
#define WIFI_SSID "Subash's OnePlus Nord 3 5G"          // Replace with your WiFi SSID
#define WIFI_PASSWORD "123456780"      // Replace with your WiFi Password

/* ================= FIREBASE CREDENTIALS ================= */
#define API_KEY "AIzaSyDUnszYKw5mY-EFOVMYK2W1gLcVQmhw-RM"
#define DATABASE_URL "https://par-optima-default-rtdb.asia-southeast1.firebasedatabase.app/"

/* ================= MOTOR PINS ================= */
#define IN1 32
#define IN2 33
#define IN3 25
#define IN4 26

/* ================= IR SENSOR PINS ================= */
#define S1 4   // Leftmost IR sensor
#define S3 18  // Middle IR sensor
#define S5 21  // Rightmost IR sensor

// Firebase Objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Global variable to store current command
String currentCmd = "x"; 
unsigned long lastFirebaseCheck = 0;
const unsigned long checkInterval = 500; // Check Firebase every 500ms

/* ================= SETUP ================= */
void setup() {
  Serial.begin(115200);

  // Motor Pin Setup
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // IR Sensor Setup
  pinMode(S1, INPUT);
  pinMode(S3, INPUT);
  pinMode(S5, INPUT);

  stopRobot(); // Start stopped

  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println("\nConnected with IP: ");
  Serial.println(WiFi.localIP());

  // Firebase Configuration
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  // Sign up or login (Anonymous)
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase Signed Up");
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Initially set data to 'x' in Firebase
  if (Firebase.ready()) {
    Firebase.RTDB.setString(&fbdo, "/cmd/data", "x");
    Serial.println("Initial state set to 'x'");
  }
}

/* ================= LOOP ================= */
void loop() {
  // Check Firebase at set intervals
  if (Firebase.ready() && (millis() - lastFirebaseCheck > checkInterval)) {
    lastFirebaseCheck = millis();
    
    if (Firebase.RTDB.getString(&fbdo, "/cmd/data")) {
      if (fbdo.dataType() == "string") {
        currentCmd = fbdo.stringData();
        Serial.print("Current Command from Firebase: ");
        Serial.println(currentCmd);
      }
    } else {
      Serial.println("Failed to get data: " + fbdo.errorReason());
    }
  }

  // Robot logic based on Firebase command
  if (currentCmd == "w") {
    followLine();
  } else {
    stopRobot();
  }

  delay(10); // Stability delay
}

/* ================= LINE FOLLOW LOGIC ================= */
void followLine() {
  int s1 = digitalRead(S1);
  int s3 = digitalRead(S3);
  int s5 = digitalRead(S5);

  if (s3 == LOW) {
    moveForward();
  } 
  else if (s5 == LOW) {
    turnRight();
  }
  else if (s1 == LOW) {
    turnLeft();
  } 
  else {
    stopRobot();
  }
}

/* ================= MOTOR CONTROL ================= */
void moveForward() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);
}

void turnRight() {
  digitalWrite(IN1, HIGH);  // Left forward
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);   // Right backward
  digitalWrite(IN4, LOW);
}

void turnLeft() {
  digitalWrite(IN1, LOW);   // Left backward
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);  // Right forward
  digitalWrite(IN4, HIGH);
}

void stopRobot() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}