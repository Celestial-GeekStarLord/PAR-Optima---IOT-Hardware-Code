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

// Sequence Logic
int stopCount = 0;      // 0: Initial, 1: After 1st Stop, 2: After 2nd Stop
bool isPaused = true;   // Robot starts in paused state
const int rotateTime = 450;    
const int forwardLong = 800; // Time to move forward after rotation to find the line again

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
      
      // LOGIC: Handling state transitions based on input
      if (newCmd == "w" && stopCount == 0) {
        isPaused = false;
        currentCmd = "w";
      } 
      else if (newCmd == "a" && stopCount == 1) {
        handleFirstStopAction(); // Executes rotation + forward
        isPaused = false;        // Enable line following
        currentCmd = "w";        // Set command to followLine logic
      } 
      else if (newCmd == "s" && stopCount == 2) {
        handleSecondStopAction(); // Executes rotation + forward
        isPaused = false;         // Enable line following
        currentCmd = "w";         // Set command to followLine logic
      }
      else if (newCmd == "x") {
        isPaused = true;
        currentCmd = "x";
      }
    }
  }

  // If not paused, run followLine. This handles S1/S3/S5 detections automatically.
  if (!isPaused && currentCmd == "w") {
    followLine();
  } else {
    stopRobot();
  }
}

/* ================= SEQUENCE ACTIONS ================= */

// First Stop: Entered 'a' -> Rotate Left -> Move Forward 
// (Removed stopRobot so it flows directly into followLine)
void handleFirstStopAction() {
  Serial.println("Action A: Turning Left and entering path...");
  turnLeft();
  delay(rotateTime);
  moveForward();
  delay(forwardLong); 
  // No stopRobot() here! Loop will take over with followLine()
}

// Second Stop: Entered 's' -> Rotate Right -> Move Forward
// (Removed stopRobot so it flows directly into followLine)
void handleSecondStopAction() {
   Serial.println("Action A: Turning Left and entering path...");
  turnLeft();
  delay(rotateTime);
  moveForward();
  delay(forwardLong); 
  // No stopRobot() here! Loop will take over with followLine()
  // No stopRobot() here! Loop will take over with followLine()
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
  // Condition: Robot is lost (All sensors see White/High)
  else if (s1 == HIGH && s3 == HIGH && s5 == HIGH) {
    Serial.println("Line Lost! Stopping and waiting for next command...");
    stopRobot();
    isPaused = true;
    stopCount++; 
    
    // Set Firebase to 'x' so the user knows to send 'a' or 's'
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