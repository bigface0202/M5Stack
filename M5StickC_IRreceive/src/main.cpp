#include <Arduino.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRutils.h>
#include <M5StickC.h>

const uint16_t RECV_PIN = 26;
const uint16_t LED_PIN = 10;
IRrecv irrecv(RECV_PIN);
decode_results results;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  M5.begin();
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  M5.Lcd.setRotation(3);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(0,0);
  M5.Lcd.println("Enabling IRin");
  irrecv.enableIRIn();
  M5.Lcd.setCursor(0,10);
  M5.Lcd.println("Enabled IRin");
  delay(2000);
  M5.Lcd.fillScreen(BLACK);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (irrecv.decode(&results)){
     if (results.decode_type == SONY){
       digitalWrite(LED_PIN, LOW);
       M5.Lcd.setCursor(0,0);
       M5.Lcd.println("SONY");
       serialPrintUint64(results.value, HEX);
       Serial.println("");
       delay(1000);
       M5.Lcd.fillScreen(BLACK);
       digitalWrite(LED_PIN, HIGH);
     }
    irrecv.resume();
  }
}