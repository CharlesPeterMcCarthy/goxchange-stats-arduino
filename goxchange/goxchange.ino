#include <rgb_lcd.h>
#include <Bridge.h>
#include <HttpClient.h>
#include <Process.h>
#include <ArduinoJson.h>
#include <Time.h>
#include <TimeLib.h>

boolean isStartup;
int currentUserCount = 0;
int currentStudentCount = 0;

const byte BUTTON_PIN = 8;

rgb_lcd lcd;
HttpClient http;
Process linux;

void setup() {
  isStartup = true;
  SetupStart();
  
  Serial.begin(9600);
  Bridge.begin();

  pinMode(BUTTON_PIN, OUTPUT);

  SetupFinish();
  GetUserCount();
}

void loop() {
  CheckButtonValue();

  long curTime = now();
  if (curTime % 10 == 0) GetUserCount();
    
  delay(1);
}

void SetupStart() {
  lcd.begin(16,2);
  DefaultColorLCD();
  lcd.print("Starting Up...");
}

void SetupFinish() {
  lcd.setCursor(0, 1);
  lcd.print("Ready.");
  delay(1000);
}

void DefaultColorLCD() {
  lcd.setRGB(39, 168, 173);
}

void ClearLCD() {
  lcd.clear();
}

void CheckButtonValue() {
  int btnVal = digitalRead(BUTTON_PIN);

  if (btnVal == 1) {
    while (digitalRead(BUTTON_PIN) == 1);
    GetUniInfo();
  }
}

void GetUserCount() {  
  String response = APICall("user-count");
  
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.parseObject(response);
  
  SortInformation(json);
  
  if (isStartup) isStartup = false;
}

void SortInformation(JsonObject& info) {
  int updatedUserCount = info["users"];
  int updatedStudentCount = info["students"];

  if (!isStartup && updatedUserCount > currentUserCount) {
    int diff = updatedUserCount - currentUserCount;
    FlashScreen();
  }

  currentUserCount = updatedUserCount;
  currentStudentCount = updatedStudentCount;
  
  PrintCurrentCount();
}

void PrintCurrentCount() {
  ClearLCD();
  lcd.setCursor(0, 0);
  lcd.print("Users: ");
  lcd.print(currentUserCount);
  lcd.setCursor(0, 1);
  lcd.print("Students: ");
  lcd.print(currentStudentCount);
}

void FlashScreen() {
  for (int i = 0; i < 10; i++) {
    lcd.setRGB(21, 255, 0); 
    delay(100);
    lcd.setRGB(255, 0, 63); 
    delay(100);
  }
  DefaultColorLCD();
}

void GetUniInfo() {
  ClearLCD();
  lcd.print("Gathering");
  lcd.setCursor(0, 1);
  lcd.print("Uni Info");

  String response = APICall("uni-info");
  char charBuf[response.length()+1];
  response.toCharArray(charBuf, response.length()+1);

  DynamicJsonBuffer jsonBuffer;
  JsonArray& unis = jsonBuffer.parseArray(charBuf);

  for (auto& uni : unis) {
    String uniName = uni["uni"];
    int users = uni["users"];
    int students = uni["students"];

    ClearLCD();

    String output = "Acc: " + String(users) + " (" + String(students) + ")";
    
    lcd.setCursor(0, 0);
    lcd.print(uniName);
    lcd.setCursor(0, 1);
    lcd.print(output);
  
    delay(4000);
  }

  PrintCurrentCount();
}

String APICall(String req) {
  String response = "";
  String apiURL = "https://next.goxchange.io/api/" + req;
  
  linux.begin("curl");
  linux.addParameter("-X");
  linux.addParameter("POST");
  linux.addParameter("-H");
  linux.addParameter("Content-Type: application/x-www-form-urlencoded");
  linux.addParameter(apiURL);
  linux.runAsynchronously();
  
  while (linux.running());
  while (linux.available()) response += linux.readString();

  Serial.flush();

  return response;
}
