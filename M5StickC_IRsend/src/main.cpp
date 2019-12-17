#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRsend.h>
#include <IRutils.h>
#include <M5StickC.h>

const uint16_t SEND_PIN = 9;
const uint16_t LED_PIN = 10;

IRsend irsend(SEND_PIN);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  M5.begin();
  pinMode(SEND_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  M5.Lcd.setRotation(3);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(0,0);
  M5.Lcd.println("Send the signal now!!");
  delay(2000);
  M5.Lcd.fillScreen(BLACK);
}

void loop() {
  // put your main code here, to run repeatedly:
  M5.update();
  if(M5.BtnA.wasPressed()){
    irsend.sendSony(0xa90, 12);
    digitalWrite(LED_PIN, LOW);
    M5.Lcd.setCursor(0,0);
    M5.Lcd.println("Send SONY");
    delay(1000);
    digitalWrite(LED_PIN, HIGH);
    M5.Lcd.fillScreen(BLACK);
  }
}