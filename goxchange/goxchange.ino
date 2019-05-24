#include <rgb_lcd.h>
#include <Bridge.h>
#include <HttpClient.h>
#include <Process.h>
#include <ArduinoJson.h>
#include <Time.h>
#include <TimeLib.h>

boolean isStartup;
boolean soundOn = true;
int currentUserCount = 0;
const byte BUTTON_PIN = 8;
const byte SOUND_PIN = 6;

rgb_lcd lcd;
HttpClient http;
Process linux;

void setup() {
  isStartup = true;
  Serial.begin(9600);
  Bridge.begin();

  pinMode(SOUND_PIN, OUTPUT);

  delay(1000);

  SetupLCD();
  ClearLCD();
}

void loop() {
  checkButtonValue();

  long curTime = now();
  if (curTime % 10 == 0) CallAPI();
    
  delay(1);
}

void SetupLCD() {
  lcd.begin(16,2);
  DefaultColorLCD();
  lcd.print("Starting Up...");
  delay(2000);
}

void DefaultColorLCD() {
  lcd.setRGB(39, 168, 173);
}

void ClearLCD() {
  lcd.clear();
}

void checkButtonValue() {
  int btnVal = digitalRead(BUTTON_PIN);

  if (btnVal == 1) {                        // Button has been pressed
    Serial.println("Button On");
    while (digitalRead(BUTTON_PIN) == 1);   // Wait until button is not being pressed

    soundOn = !soundOn;                 // Sound on / off
    ClearLCD();
    lcd.setCursor(0, 0);
    lcd.print(soundOn ? "Sound is on" : "Sound is off");
    delay(1000);
    PrintCurrentCount();
  }
}

void CallAPI() {
  Serial.println("Call");
  StaticJsonBuffer<200> jsonBuffer;
  String response = "";

  linux.begin("curl");
  linux.addParameter("-X");
  linux.addParameter("POST");
  linux.addParameter("-H");
  linux.addParameter("Content-Type: application/x-www-form-urlencoded");
  linux.addParameter("https://next.goxchange.io/api/usercount");
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

  if (!isStartup && updatedUserCount > currentUserCount) {
    int diff = updatedUserCount - currentUserCount;
    
    if (soundOn) SoundAlert(diff);
    FlashScreen();
  }

  currentUserCount = updatedUserCount;
  
  PrintCurrentCount();
}

void PrintCurrentCount() {
  ClearLCD();
  lcd.setCursor(0, 0);
  lcd.print("User Count: ");
  lcd.print(currentUserCount);
}

void SoundAlert(int diff) {
  for (int i = 0; i < diff; i++) {
    PlaySound();
    delay(500);
  }  
}

void PlaySound() {
  digitalWrite(SOUND_PIN, HIGH);
  delay(10);
  digitalWrite(SOUND_PIN, LOW);
}

void FlashScreen() {
  for (int i = 0; i < 10; i++) {
    lcd.setRGB(21, 255, 0); 
    delay(100);
    lcd.setRGB(255, 0, 63); 
    delay(100);
  }
  lcd.setRGB(39, 168, 173);
}
