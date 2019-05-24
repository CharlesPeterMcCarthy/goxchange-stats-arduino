#include <rgb_lcd.h>
#include <Bridge.h>
#include <HttpClient.h>
#include <Process.h>
#include <ArduinoJson.h>

boolean isStartup;
int currentUserCount = 0;

rgb_lcd lcd;
HttpClient http;
Process linux;

void setup() {
  isStartup = true;
  Serial.begin(9600);
  Bridge.begin();

  delay(1000);

  setupLCD();
  clearLCD();
}

void loop() {
  StaticJsonBuffer<200> jsonBuffer;
  String response = "";

  linux.begin("curl");
  linux.addParameter("-X");
  linux.addParameter("POST");
  linux.addParameter("-H");
  linux.addParameter("Content-Type: application/x-www-form-urlencoded");
  linux.addParameter("https://next.goxchange.io/api/usercount");
  linux.runAsynchronously();
  
  while (linux.running());          // Wait for script to run
  while (linux.available()) {       // Get response
    response += linux.readString();
  }

  Serial.println(response);
  Serial.flush();

  JsonObject& json = jsonBuffer.parseObject(response);
  SortInformation(json);
  
  if (isStartup) isStartup = false;
  
  delay(10000);
}

void setupLCD() {
  lcd.begin(16,2);                // 2 rows, 16 columns
  lcd.setRGB(24, 167, 219);       // Set blue background
  lcd.print("Starting Up...");
  delay(2000);
}

void clearLCD() {
  lcd.clear();
}

void SortInformation(JsonObject& info) {
  clearLCD();
  currentUserCount = info["users"];
  
  lcd.setCursor(0, 0);
  lcd.print("User Count: ");
  //lcd.setCursor(0, 1);
  lcd.print(currentUserCount);
}
