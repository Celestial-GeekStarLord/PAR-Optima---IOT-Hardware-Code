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
String lastCmd = "x";
unsigned long lastFirebaseCheck = 0;
const unsigned long checkInterval = 500; 

// Counter & Sequence Logic
long s1Count = 0;
long s5Count = 0;
const int rotationTime = 400; 
const int longForwardTime = 1000; // Time to move forward after rotation (1 second)
int stopSequence = 0; // 0: Start(w), 1: Wait for 'a', 2: Wait for 's'

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
  // Check Firebase
  if (Firebase.ready() && (millis() - lastFirebaseCheck > checkInterval)) {
    lastFirebaseCheck = millis();
    if (Firebase.RTDB.getString(&fbdo, "/cmd/data")) {
      String newCmd = fbdo.stringData();
      
      // Check for Transition from 'x' to 'a' or 's'
      if (currentCmd == "x") {
        if (newCmd == "a" && stopSequence == 1) {
          executeKickstart(); // Rotate + Long Forward
        } 
        else if (newCmd == "s" && stopSequence == 2) {
          executeKickstart(); // Rotate + Long Forward
        }
      }

      // Reset sequence if 'w' is sent while at rest
      if (newCmd == "w" && currentCmd == "x" && stopSequence == 0) {
        s1Count = 0; s5Count = 0;
        Serial.println("Sequence Initiated.");
      }
      currentCmd = newCmd;
    }
  }

  // Follow line only if a valid command is active
  bool shouldMove = false;
  if (stopSequence == 0 && currentCmd == "w") shouldMove = true;
  else if (stopSequence == 1 && currentCmd == "a") shouldMove = true;
  else if (stopSequence == 2 && currentCmd == "s") shouldMove = true;

  if (shouldMove) {
    followLine();
  } else {
    stopRobot();
  }
}

/* ================= TRANSITION LOGIC ================= */
// This runs when you enter 'a' or 's' after the robot had stopped
void executeKickstart() {
  Serial.println("Kickstarting: Rotating and Moving Forward (Long)...");
  
  // 1. Rotate based on history
  if (s1Count > s5Count) {
    Serial.println("Rotating Left...");
    turnLeft();
  } else {
    Serial.println("Rotating Right...");
    turnRight();
  }
  delay(rotationTime);

  // 2. Move Forward for a LONGER time to reach the line
  Serial.println("Moving Forward (Long Duration)...");
  moveForward();
  delay(longForwardTime);

  // 3. Reset counts for the next leg
  s1Count = 0;
  s5Count = 0;
  Serial.println("Kickstart Complete. Switching to Line Follower.");
}

/* ================= LINE FOLLOW LOGIC ================= */
void followLine() {
  int s1 = digitalRead(S1);
  int s3 = digitalRead(S3);
  int s5 = digitalRead(S5);

  if (s1 == LOW) s1Count++;
  if (s5 == LOW) s5Count++;

  if (s3 == LOW) {
    moveForward();
  } 
  else if (s1 == LOW) {
    turnLeft();
  }
  else if (s5 == LOW) {
    turnRight();
  }
  else if (s1 == HIGH && s3 == HIGH && s5 == HIGH) {
    // LINE LOST - STOP AND WAIT
    Serial.println("Line Lost. Stopping. Waiting for Firebase input...");
    stopRobot();
    
    // Update Firebase to 'x' to wait for next input
    if (Firebase.ready()) {
      Firebase.RTDB.setString(&fbdo, "/cmd/data", "x");
      currentCmd = "x"; 
    }

    stopSequence++; // Increment sequence so it knows to wait for 'a' or 's'
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
  digitalWrite(IN1, HIGH);  
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);   
  digitalWrite(IN4, LOW);
}

void turnLeft() {
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