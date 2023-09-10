// Include necessary libraries and header file

#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "index.h" // Include the index.h file containing your HTML content

const char* ssid = "Your_WiFi_SSID"; 
const char* password = "Your_WiFi_Password"; 
const unsigned long wifiConnectTimeout = 5000; //waits 5 seconds till timeout

WebServer server(80); //default HTTP port

//initializing LCD display and pin definations
LiquidCrystal_I2C lcd(0x27, 16, 2);
const int trigPin = 5;
const int echoPin = 18;
const int buzzerPin = 13;
const int LED1 = 27; //G
const int LED2 = 14; //Y
const int LED3 = 12; //R

void setup(void) {
  Serial.begin(115200);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("ESP32 & Sonar");

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);


  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  unsigned long startTime = millis();

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
    if (millis() - startTime > wifiConnectTimeout) {
      Serial.println("Failed to connect to Wi-Fi within the timeout. Continuing without Wi-Fi.");
      break; // Exit the loop and continue without Wi-Fi
    }
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp32")) { //Initializes mDNS (Multicast DNS) with the ESP32's hostname as "esp32"
    Serial.println("MDNS responder started");
  }
  server.on("/", HTTP_GET, handleRoot);
  server.on("/api", HTTP_GET, handleAPI); // Define the API endpoint
  server.begin();
}

void loop(void) {
  
  // Code to run continuously
  float distanceCm = readSonarDistance();
 
  // Handle incoming web requests
  server.handleClient();
  delay(500);
}

// Function to calculate the color bar width based on distance
int calculateWidthAndColor(float distanceCm) { 
  const float totalWidthCm = 400.0;
  float widthPercentage = (distanceCm / totalWidthCm) * 100.0;
  int clampedWidthPercentage = constrain(widthPercentage, 0, 100);   // Ensure the width percentage is between 0% and 100%

  int color;  // Determine the color based on the distance
  if (distanceCm >= 0 && distanceCm <= 5) {
    color = 0xFF0000; 
  } else if (distanceCm > 5 && distanceCm <= 10) {
    color = 0xFFFF00; 
  } else if (distanceCm > 10 && distanceCm <= 15) {
    color = 0x00FF00; 
  } else if (distanceCm > 15 && distanceCm <= 400) {
    color = 0x0000FF; 
  }
  return clampedWidthPercentage;
}

// Function to handle the root (HTML page)
void handleRoot() {
  float distanceCm = readSonarDistance();

  String alertMessage;

  if (distanceCm > 10 && distanceCm <= 15) {
    alertMessage = "ALERT";
  } else if (distanceCm > 5 && distanceCm <= 10) {
     alertMessage = "ALERT ALERT";
  } else if (distanceCm > 0 && distanceCm <= 5) {
    alertMessage = "ALERT ALERT ALERT";
  } else {
    alertMessage = "NO ALERT";
  }

  int widthPercentage = calculateWidthAndColor(distanceCm);   // Calculate the width percentage and color based on distance

  String htmlContent = HTML_PAGE; // Copy the HTML template

  // Replace placeholders in HTML template with actual values
  htmlContent.replace("{DISTANCE_CM}", String(distanceCm, 2));
  htmlContent.replace("{DISTANCE_INCH}", String(distanceCm / 2.54, 2));
  htmlContent.replace("{DISTANCE_METER}", String(distanceCm / 100, 2));
  htmlContent.replace("{ALERT_MESSAGE}", alertMessage);
  htmlContent.replace("{ALERT_COLOR}", (widthPercentage == 0) ? "#6614B4" : "green");

  server.send(200, "text/html", htmlContent);   // Send the HTML response to the client

}

// Function to handle the API request
void handleAPI() {
  float distanceCm = readSonarDistance();
  int colorBarWidth = calculateWidthAndColor(distanceCm); // Implement this function to calculate color bar width.

  // Create JSON data to send as the API response
  String jsonData = "{\"distance_cm\":" + String(distanceCm, 2) + ",\"color_bar_width\":" + String(colorBarWidth) + "}";
  // Set headers for Cross-Origin Resource Sharing (CORS)
  server.sendHeader("Access-Control-Allow-Origin", "*"); // Allow requests from any origin
  server.sendHeader("Access-Control-Max-Age", "10000");
  // Send the JSON response to the client
  server.send(200, "application/json", jsonData);
}

// Function to read the sonar distance
float readSonarDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(5);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  float duration = pulseIn(echoPin, HIGH);
  float distanceCm = duration * 0.0343 / 2.0;

  Serial.print("Distance: ");
  Serial.print(distanceCm);
  Serial.println(" cm");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(distanceCm);
  lcd.print(" cm");

  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
  digitalWrite(buzzerPin, LOW);

  if (distanceCm>15 && distanceCm<=400) {
    lcd.setCursor(0, 1);
    lcd.print("NO ALERT");  
  } else if (distanceCm > 10 && distanceCm <= 15) {
      lcd.setCursor(0, 1);
      lcd.print("ALERT");
      Serial.println("ALERT");
      digitalWrite(LED1, HIGH);
      digitalWrite(buzzerPin, HIGH);
      delay(300);
      digitalWrite(LED1, LOW);
      digitalWrite(buzzerPin, LOW);
  } else if (distanceCm > 5 && distanceCm <= 10) {
      lcd.setCursor(0, 1);
      lcd.print("ALERT ALERT");
      Serial.println("ALERT ALERT");
      digitalWrite(LED2, HIGH);
      digitalWrite(buzzerPin, HIGH);
      delay(200);
      digitalWrite(LED2, LOW);
      digitalWrite(buzzerPin, LOW);
  } else if (distanceCm > 0 && distanceCm <= 5) {
      lcd.setCursor(0, 1);
      lcd.print("ALERT ALERT ALERT");
      Serial.println("ALERT ALERT ALERT");
      digitalWrite(LED3, HIGH);
      digitalWrite(buzzerPin, HIGH);
      delay(150);
      digitalWrite(LED3, LOW);
      digitalWrite(buzzerPin, LOW);
  }
      return distanceCm;
  }

