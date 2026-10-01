/* Variable And Imports */
// Libraries
#include <Preferences.h>
#include "Goldelox_Serial_4DLib.h"
#include "Goldelox_Const4D.h"
#include <WiFi.h>
#include <NetworkClient.h>
#include <WiFiAP.h>
#include <Adafruit_NeoPixel.h>
#include <ESP32Servo.h>

// Defining pins and constants
#define SERVO_PIN 3
#define DisplaySerial Serial1
#define RW_MODE false
#define RO_MODE true
#define RESETLINE 4       
#define TRIG 6            
#define ECHO 7            
#define LED_PIN     8     
#define NUM_LEDS    1     

// Setup
Goldelox_Serial_4DLib Display(&DisplaySerial); // uLCD
Adafruit_NeoPixel pixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800); // Onboard LED


// ball count for uLCD, dispensing, and wifi site
int count = 0;
// ssid and password for wifi connection
const char *ssid = "yourAP";
const char *password = "12345678";
//variable for Wifi site
bool done = false;
NetworkServer server(80);
// Non-Volatile Memory
Preferences myPreferences;
// Sensor
volatile unsigned long startTime = 0;
volatile unsigned long echoTime = 0;
volatile bool measurementDone = false;
volatile float measurement;
bool dispense = false;
bool canDetect = true;
//WIFI
String message = "NA"; 
//Servo
Servo servo;  // create servo object to control a servo
const int SERVO_MIN_PULSE = 500;
const int SERVO_MAX_PULSE = 2500;

/* Function Declarations */

// Function to URL decode strings
String urlDecode(String input) {
  String decoded = "";
  char temp[] = "0x00";
  unsigned int len = input.length();
  unsigned int i = 0;
  
  while (i < len) {
    char decodedChar;
    char encodedChar = input.charAt(i++);
    
    if (encodedChar == '+') {
      decodedChar = ' ';
    } else if (encodedChar == '%') {
      temp[2] = input.charAt(i++);
      temp[3] = input.charAt(i++);
      decodedChar = strtol(temp, NULL, 16);
    } else {
      decodedChar = encodedChar;
    }
    
    decoded += decodedChar;
  }
  return decoded;
}

// Wifi loop code
void wifiSite() {
  NetworkClient client = server.accept();

  if (client) {
    Serial.println("New Client.");
    String currentLine = "";
    String requestBody = "";
    bool isPost = false;
    int contentLength = 0;
    bool headersEnded = false;
    bool messageReceived = false;
    
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        
        if (!headersEnded) {
          Serial.write(c);
          
          if (c == '\n') {
            if (currentLine.length() == 0) {
              headersEnded = true;
              
              // If POST request, read the body
              if (isPost && contentLength > 0) {
                for (int i = 0; i < contentLength; i++) {
                  if (client.available()) {
                    requestBody += (char)client.read();
                  }
                }
              }
              
              // Process submitted message before sending response
              if (isPost && requestBody.indexOf("message=") >= 0) {
                int startPos = requestBody.indexOf("message=") + 8;
                message = requestBody.substring(startPos);
                
                // Handle additional parameters if present
                int ampPos = message.indexOf('&');
                if (ampPos > 0) {
                  message = message.substring(0, ampPos);
                }
                
                message = urlDecode(message);
                messageReceived = true;
                
                // Process the message immediately
                Serial.println("=== Received Message ===");
                Serial.println(message);
                
                if ((message == "R") || (message == "r")) {
                  Serial.println("Reset");
                  count = 0;
                  done = false;
                } else {
                  int newCount = message.toInt();
                  if (newCount == 0 && message != "0") {
                    Serial.println("Invalid Entry - not a number");
                    done = true;
                  } else {
                    count = newCount;
                    Serial.print("Count set to: ");
                    Serial.println(count);
                    done = false;
                  }
                }
                Serial.println("=======================");
              }
              
              // Send HTTP response
              client.println("HTTP/1.1 200 OK");
              client.println("Content-type:text/html");
              client.println("Connection: close");
              client.println();

              // HTML format of webpage
              client.println("<!DOCTYPE HTML>");
              client.println("<html>");
              client.println("<head>");
              client.println("<title>ESP32 String Input</title>");
              client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
              client.println("<style>");
              client.println("body { font-family: Arial; margin: 20px; background-color: #f0f0f0; }");
              client.println(".container { max-width: 600px; margin: 0 auto; background-color: white; padding: 20px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }");
              client.println("h1 { color: #333; }");
              client.println("input[type='text'] { width: 100%; padding: 10px; font-size: 16px; border: 2px solid #ddd; border-radius: 5px; box-sizing: border-box; }");
              client.println("input[type='submit'] { width: 100%; padding: 12px; margin-top: 10px; font-size: 16px; background-color: #4CAF50; color: white; border: none; border-radius: 5px; cursor: pointer; }");
              client.println("input[type='submit']:hover { background-color: #45a049; }");
              client.println(".status { background-color: #e7f3ff; padding: 15px; border-radius: 5px; margin-top: 20px; border-left: 4px solid #2196F3; }");
              client.println(".received { background-color: #d4edda; padding: 15px; border-radius: 5px; margin-top: 10px; border-left: 4px solid #28a745; }");
              client.println("</style>");
              client.println("</head>");
              client.println("<body>");
              client.println("<div class='container'>");
              client.println("<h1>ESP32 Control Panel</h1>");
              client.println("<p>Enter the number of balls or enter 'R' for Reset</p>");
              client.println("<form action=\"/submit\" method=\"POST\">");
              client.println("<input type=\"text\" name=\"message\" placeholder=\"Type your input here\" required>");
              client.println("<input type=\"submit\" value=\"Submit\">");
              client.println("</form>");
              
              // Display current status
              client.println("<div class='status'>");
              client.println("<h3>Current Status:</h3>");
              client.print("<p><strong>Count:</strong> ");
              client.print(count);
              client.println("</p>");
              client.print("<p><strong>Last Message:</strong> ");
              client.print(message);
              client.println("</p>");
              client.println("</div>");
              
              // Display confirmation that the input has been received
              if (messageReceived) {
                client.println("<div class='received'>");
                client.println("<h3> Message Received Successfully!</h3>");
                client.print("<p>Your input: <strong>");
                client.print(message);
                client.println("</strong></p>");
                client.println("</div>");
              }
              
              client.println("</div>");
              client.println("</body>");
              client.println("</html>");
              client.println();
              break;
            } else {
              // Check for POST request
              if (currentLine.startsWith("POST")) {
                isPost = true;
              }
              // Get content length
              if (currentLine.startsWith("Content-Length: ")) {
                contentLength = currentLine.substring(16).toInt();
              }
              currentLine = "";
            }
          } else if (c != '\r') {
            currentLine += c;
          }
        }
      }
    }
        
  }
  Serial.println(count);
  delay(10);
}

// Interrupt to measure sensor
void IRAM_ATTR echoISR() {
  if (digitalRead(ECHO) == HIGH) {
    startTime = micros();
  } else {
    echoTime = micros() - startTime;
    measurementDone = true;
  }
}
// Function to trigger the measurement
void measure() {
  measurementDone = false;
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  measurement = echoTime/23.18;
  //float duration = pulseIn(ECHO, HIGH);
  //measurement = (duration/23.18);

}

void memorySetup() {
  myPreferences.begin("myPreferences", RO_MODE);  
  bool tpInit = myPreferences.isKey("nvsInit");  // Check if this is the first time its starting up
  if (tpInit == false) { // If first startup then define beginning variables
    myPreferences.end();
    myPreferences.begin("myPreferences", RW_MODE);
    myPreferences.putInt("Count", 0); // First time initialization variables, default values
    myPreferences.putBool("nvsInit", true);
    myPreferences.end();
    myPreferences.begin("myPreferences", RO_MODE); // Reopen in read only mode so setup can access myPreferences
  }
  count = myPreferences.getInt("Count");   // Retrieve count value from previous runs
  myPreferences.end();
}

// Servo movement for dispensing the golf ball, adapted from the ESP32Servo Library sweep sketch
void servoFunc() {
  int deg = 0; // start angle
  while(true) {
    if (deg == 190) { // sweep to 190 degrees when a ball is being dispensed
      delay(200);
      deg = 0; // Go back to 0 once the ball has angle of 190 degrees
      servo.write(deg);
      break;
    }
    servo.write(deg);
    deg += 10; // changes angle in increments of 10 degrees
    delay(250);
  }
  delay(50); // small delay for button reading
}
// Function for dispensing ball where we want the onboard LED to display green when dispensing and red when not dispensing
void ballDispense() {
  if (dispense && count > 0) {
    // LED Code
    pixel.setPixelColor(0, pixel.Color(0, 255, 0)); // Display GREEN to show user that a ball is dispensing
    pixel.show();
    // Servo Code
    servoFunc();
    count--;
    myPreferences.begin("myPreferences", RW_MODE);
    myPreferences.putInt("Count", count); // Update Count value
    myPreferences.end();
    dispense = false;
  } else {
    pixel.setPixelColor(0, pixel.Color(255, 0, 0));   // Display RED to indicate that no ball is being dispensed
    pixel.show(); 
  }
}
// Displays the count of balls
void displayCount() {
  char buffer[12];
  if (count > 0) {
    sprintf(buffer, "%d", count);
  } else {
    sprintf(buffer, "%s", "End");
  }
  // Each character width = 8 px base * scale(aka 5)
    int scale = 5;
    int charWidth = 8 * scale;
    int charHeight = 8 * scale;
    int textWidth = strlen(buffer) * charWidth;
    int textHeight = charHeight;
    int x = (128 - textWidth) / 2;
    int y = (128 - textHeight) / 2;
    Display.gfx_MoveTo(x, y);
    Display.putstr(buffer);
}

/* Main Body Beginning */
void setup() {
  // For Sensor
  Serial.begin(115200);
  
  // Ultrasonic sensor
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  digitalWrite(TRIG, LOW);

  // Reset for the uLCD
  pinMode(RESETLINE, OUTPUT);
  digitalWrite(RESETLINE, 0);
  delay(100);
  digitalWrite(RESETLINE, 1);
  delay(5000);
  
  // Attach interrupt for BOTH rising and falling edges
  attachInterrupt(digitalPinToInterrupt(ECHO), echoISR, CHANGE);

  // For Non-volatile Memory
  memorySetup();
  // Onboard LED
  pixel.begin();           // Initialize NeoPixel
  pixel.setBrightness(20); // Make it less bright!
  // Defualt LED, not dispensing
  pixel.setPixelColor(0, pixel.Color(255, 0, 0));   // Display RED to indicate that no ball is being dispensed
  pixel.show();
  // Initialize servo
  servo.setPeriodHertz(50);
  servo.attach(SERVO_PIN, SERVO_MIN_PULSE, SERVO_MAX_PULSE);
  //uLCD
  DisplaySerial.begin(9600, SERIAL_8N1, 21, 22);
  Display.TimeLimit4D = 5000;
  Display.gfx_Cls();
  Display.txt_BGcolour(BLACK); //background color
  Display.txt_FGcolour(WHITE); //text color
  // MAKE TEXT LARGE (5x scaling for height and width of text)
  Display.txt_Width(5); 
  Display.txt_Height(5);
  // Wifi
  if (!WiFi.softAP(ssid, password)) {
    log_e("Soft AP creation failed.");
    while (1);
  }
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.begin();
  Serial.println("Server started");
}

void loop() {
  // Display for the uLCD
  Display.gfx_RectangleFilled(0, 0, 127, 127, BLACK);
  // Polling condition for ball dispensing
  measure();
  while(!(measurementDone));
  Serial.println(measurement);
  if ((measurement < 10) && (canDetect == true)) {
    dispense = true;
    canDetect = false;
  } else if (measurement >= 15) {
    canDetect = true;
  }
  // Ball dispensing sequence
  ballDispense();
  // uLCD displaying the amount of balls
  displayCount();
  // Call wifi site function
  wifiSite();

  delay(10); // Small delay for stability
}
