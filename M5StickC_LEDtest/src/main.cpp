#include <Arduino.h>
#include <M5StickC.h>

int LED_pin = 10;

void setup() {
  // put your setup code here, to run once:
  M5.begin();
  pinMode(LED_pin, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(LED_pin, HIGH);
  delay(1000);
  digitalWrite(LED_pin, LOW);
  delay(1000);
}