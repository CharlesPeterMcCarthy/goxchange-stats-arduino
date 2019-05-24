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
const byte TOUCH_PIN = 5;

rgb_lcd lcd;
HttpClient http;
Process linux;

void setup() {
  isStartup = true;
  SetupStart();
  
  Serial.begin(9600);
  Bridge.begin();

  pinMode(BUTTON_PIN, OUTPUT);
  pinMode(TOUCH_PIN, INPUT);

  SetupFinish();
  GetUserCount();
}

void loop() {
  CheckTouchValue();

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

void CheckTouchValue() {
  int tchVal = digitalRead(TOUCH_PIN);

  if (tchVal == 1) {
    Serial.println("Touch On");
    while (digitalRead(TOUCH_PIN) == 1);

    GetUniInfo();
  }
}

void GetUserCount() {
  Serial.println("Call User Count");
  DynamicJsonBuffer jsonBuffer;
  String response = "";

  linux.begin("curl");
  linux.addParameter("-X");
  linux.addParameter("POST");
  linux.addParameter("-H");
  linux.addParameter("Content-Type: application/x-www-form-urlencoded");
  linux.addParameter("https://next.goxchange.io/api/user-count");
  linux.runAsynchronously();
  
  while (linux.running());
  while (linux.available()) {
    response += linux.readString();
  }

  Serial.println(response);
  Serial.flush();

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
  Serial.println("Call Uni Info");
  DynamicJsonBuffer jsonBuffer;
  String response = "";

  ClearLCD();
  lcd.print("Gathering");
  lcd.setCursor(0, 1);
  lcd.print("Uni Info");
  
  linux.begin("curl");
  linux.addParameter("-X");
  linux.addParameter("POST");
  linux.addParameter("-H");
  linux.addParameter("Content-Type: application/x-www-form-urlencoded");
  linux.addParameter("https://next.goxchange.io/api/uni-info");
  linux.runAsynchronously();
  
  while (linux.running());
  while (linux.available()) {
    Serial.println("read");
    response += linux.readString();
  }

  Serial.println(response);
  Serial.flush();

  char charBuf[response.length()+1];
  response.toCharArray(charBuf, response.length()+1);
  JsonArray& unis = jsonBuffer.parseArray(charBuf);

  for (auto& uni : unis) {
    String uniName = uni["uni"];
    int users = uni["users"];
    int students = uni["students"];

    ClearLCD();
    
    lcd.setCursor(0, 0);
    lcd.print(uniName);
    lcd.setCursor(0, 1);
    lcd.print("U: ");
    lcd.print(users);
    lcd.print(" - S: ");
    lcd.print(students);
  
    delay(4000);
  }

  PrintCurrentCount();
}
