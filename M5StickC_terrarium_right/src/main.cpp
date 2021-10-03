#include <Arduino.h>
#include <FastLED.h>
#include <M5StickC.h>

#define NUM_LEDS 7
CRGB leds[NUM_LEDS];

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  M5.begin();
  pinMode(26, OUTPUT);
  FastLED.addLeds<NEOPIXEL, 26>(leds, NUM_LEDS);
  M5.Lcd.setRotation(3);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(2);
}

int num = 0;
int phase = 0;

void loop() {
  // put your main code here, to run repeatedly:
  M5.update();
  M5.Lcd.setCursor(0, 0);
  if(M5.BtnA.wasPressed()){
    num++;
    if(num > 6){
      num = 0;
    }
  }
  if(num == 0){
    M5.Lcd.println("Loop  ");
      if (phase == 0){
        for(int i = 0; i < 7; i++){
          if (i % 2 == 0){
            leds[i] = CRGB::Red;
          }else if (i % 2 == 1){
            leds[i] = CRGB::Blue;
          }
        }
      delay(1000);
      }else if (phase == 1){
        for(int i = 0; i < 7; i++){
          if (i % 2 == 0){
            leds[i] = CRGB::Blue;
          }else if (i % 2 == 1){
            leds[i] = CRGB::Green;
          }
        }
      delay(1000);
      }else if (phase == 2){
        for(int i = 0; i < 7; i++){
          if (i % 2 == 0){
            leds[i] = CRGB::Green;
          }else if (i % 2 == 1){
            leds[i] = CRGB::Red;
          }
        }
      delay(1000);
      }
    if(M5.BtnB.wasPressed()){
      num++;
    }
  }else if(num == 1){
    M5.Lcd.println("Blue  ");
    for(int i = 0; i < 7; i++){
      leds[i] = CRGB::Blue;
    }
  }else if(num == 2){
    M5.Lcd.println("Green ");
    for(int i = 0; i < 7; i++){
      leds[i] = CRGB::Green;
    }
  }else if(num == 3){
    M5.Lcd.println("Red   ");
    for(int i = 0; i < 7; i++){
      leds[i] = CRGB::Red;
    }
  }else if(num == 4){
    M5.Lcd.println("Orange");
    for(int i = 0; i < 7; i++){
      leds[i] = CRGB::Orange;
    }
  }else if(num == 5){
    M5.Lcd.println("Yellow");
    for(int i = 0; i < 7; i++){
      leds[i] = CRGB::Yellow;
    }
  }else if(num == 6){
    M5.Lcd.println("OFF   ");
    for(int i = 0; i < 7; i++){
      leds[i] = CRGB::Black;
    }
  }
  FastLED.show();
  phase++;
  if(phase > 2){
    phase = 0;
  }
}