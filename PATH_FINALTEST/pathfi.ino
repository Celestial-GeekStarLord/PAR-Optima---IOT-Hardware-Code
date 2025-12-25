#include <WiFi.h>
#include <Firebase_ESP_Client.h>

// Provide the token generation process info.
#include <addons/TokenHelper.h>
// Provide the RTDB payload printing info.
#include <addons/RTDBHelper.h>

/* ================= WIFI CREDENTIALS ================= */
#define WIFI_SSID "Subash's OnePlus Nord 3 5G"          
#define WIFI_PASSWORD "123456780"      

/* ================= FIREBASE CREDENTIALS ================= */
#define API_KEY "AIzaSyDUnszYKw5mY-EFOVMYK2W1gLcVQmhw-RM"
#define DATABASE_URL "https://par-optima-default-rtdb.asia-southeast1.firebasedatabase.app/"

/* ================= MOTOR PINS ================= */
#define IN1 32
#define IN2 33
#define IN3 25
#define IN4 26

/* ================= IR SENSOR PINS ================= */
#define S1 4   
#define S3 18  
#define S5 21  

// Firebase Objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Global Variables
String currentCmd = "x"; 
unsigned long lastFirebaseCheck = 0;
const unsigned long checkInterval = 500; 

// Counter Logic
long s1Count = 0;
long s5Count = 0;
const int rotationTime = 400; // Time in ms to rotate roughly 60 degrees (Adjust this!)

/* ================= SETUP ================= */
void setup() {
  Serial.begin(115200);

  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);

  pinMode(S1, INPUT); pinMode(S3, INPUT); pinMode(S5, INPUT);

  stopRobot();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected");

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("Firebase Signed Up");
  }

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  if (Firebase.ready()) {
    Firebase.RTDB.setString(&fbdo, "/cmd/data", "x");
  }
}

/* ================= LOOP ================= */
void loop() {
  if (Firebase.ready() && (millis() - lastFirebaseCheck > checkInterval)) {
    lastFirebaseCheck = millis();
    if (Firebase.RTDB.getString(&fbdo, "/cmd/data")) {
      String newCmd = fbdo.stringData();
      
      // If we receive a fresh 'w', reset counters
      if (newCmd == "w" && currentCmd == "x") {
        s1Count = 0;
        s5Count = 0;
        Serial.println("Counters Reset for new run.");
      }
      currentCmd = newCmd;
    }
  }

  if (currentCmd == "w") {
    followLine();
  } else {
    stopRobot();
  }
}

/* ================= LINE FOLLOW LOGIC ================= */
void followLine() {
  int s1 = digitalRead(S1);
  int s3 = digitalRead(S3);
  int s5 = digitalRead(S5);

  // Increment counters if sensors detect line (LOW)
  if (s1 == LOW) s1Count++;
  if (s5 == LOW) s5Count++;

  if (s3 == LOW) {
    // Normal Path: Middle sensor is active
    moveForward();
  } 
  else if (s1 == LOW && s3 == HIGH && s5 == HIGH) {
    // Sharp Left detected
    turnLeft();
  }
  else if (s5 == LOW && s3 == HIGH && s1 == HIGH) {
    // Sharp Right detected
    turnRight();
  }
  else if (s1 == HIGH && s3 == HIGH && s5 == HIGH) {
    // ROBOT IS LOST (All white)
    Serial.println("Line Lost! Analyzing counters...");
    stopRobot();
    delay(200); // Brief pause to stabilize

    if (s1Count > s5Count) {
      Serial.println("Left count higher. Rotating Left...");
      turnLeft();
      delay(rotationTime); // Estimated 60 degree turn
    } 
    else if (s5Count > s1Count) {
      Serial.println("Right count higher. Rotating Right...");
      turnRight();
      delay(rotationTime); // Estimated 60 degree turn
    }
    
    // Reset counters after recovery to prevent "memory" of old turns
    s1Count = 0;
    s5Count = 0;
    stopRobot(); // Stop after rotating to look for line again
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
  // Hard Right Rotation
  digitalWrite(IN1, HIGH);  
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);   
  digitalWrite(IN4, LOW);
}

void turnLeft() {
  // Hard Left Rotation
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