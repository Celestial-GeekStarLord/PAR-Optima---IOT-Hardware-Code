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

/* ================= ULTRASONIC PINS ================= */
#define TRIG_PIN 27
#define ECHO_PIN 14
#define DISTANCE_THRESHOLD 20 // Distance in cm

// Firebase Objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Global Variables
String currentCmd = "x"; 
unsigned long lastFirebaseCheck = 0;
const unsigned long checkInterval = 500; 

// Sequence Logic
int stopCount = 0;      
bool isPaused = true;   
const int rotateTime = 450;    
const int forwardLong = 800; 

/* ================= SETUP ================= */
void setup() {
  Serial.begin(115200);

  // Motor Pins
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);

  // IR Pins
  pinMode(S1, INPUT); pinMode(S3, INPUT); pinMode(S5, INPUT);

  // Ultrasonic Pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

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

/* ================= OBSTACLE DETECTION LOGIC ================= */
long getDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // 30ms timeout
  long distance = duration * 0.034 / 2;
  
  if (distance == 0) return 999; // If no pulse, assume path is clear
  return distance;
}

bool isObstacleDetected() {
  long dist = getDistance();
  if (dist < DISTANCE_THRESHOLD) {
    return true;
  }
  return false;
}

/* ================= LOOP ================= */
void loop() {
  // Check Firebase
  if (Firebase.ready() && (millis() - lastFirebaseCheck > checkInterval)) {
    lastFirebaseCheck = millis();
    if (Firebase.RTDB.getString(&fbdo, "/cmd/data")) {
      String newCmd = fbdo.stringData();
      
      if (newCmd == "w" && stopCount == 0) {
        isPaused = false;
        currentCmd = "w";
      } 
      else if (newCmd == "a" && stopCount == 1) {
        handleFirstStopAction(); 
        isPaused = false;        
        currentCmd = "w";        
      } 
      else if (newCmd == "s" && stopCount == 2) {
        handleSecondStopAction(); 
        isPaused = false;         
        currentCmd = "w";         
      }
      else if (newCmd == "x") {
        isPaused = true;
        currentCmd = "x";
      }
    }
  }

  // GLOBAL OBSTACLE CHECK
  if (isObstacleDetected()) {
    stopRobot();
    Serial.println("OBSTACLE DETECTED! Stopping...");
    delay(100); // Small pause
  } 
  else {
    // Normal Operation
    if (!isPaused && currentCmd == "w") {
      followLine();
    } else {
      stopRobot();
    }
  }
}

/* ================= SEQUENCE ACTIONS ================= */

// Helper to delay while checking for obstacles
void obstacleSafeDelay(int ms, void (*moveFunc)()) {
  unsigned long start = millis();
  while (millis() - start < ms) {
    if (isObstacleDetected()) {
      stopRobot();
      while (isObstacleDetected()) {
        delay(100); // Wait until obstacle is removed
      }
      moveFunc(); // Restart the movement after obstacle is gone
    }
    delay(10);
  }
}

void handleFirstStopAction() {
  Serial.println("Action A: Turning Left...");
  turnLeft();
  obstacleSafeDelay(rotateTime, turnLeft);
  
  moveForward();
  obstacleSafeDelay(forwardLong, moveForward);
}

void handleSecondStopAction() {
  Serial.println("Action S: Turning Right...");
  turnRight();
  obstacleSafeDelay(rotateTime, turnRight);
  
  moveForward();
  obstacleSafeDelay(forwardLong, moveForward);
}

/* ================= LINE FOLLOW LOGIC ================= */
void followLine() {
  int s1 = digitalRead(S1);
  int s3 = digitalRead(S3);
  int s5 = digitalRead(S5);

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
    Serial.println("Line Lost! Stopping...");
    stopRobot();
    isPaused = true;
    stopCount++; 
    
    if (Firebase.ready()) {
      Firebase.RTDB.setString(&fbdo, "/cmd/data", "x");
    }
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