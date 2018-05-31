#pragma mark - Depend ESP8266Audio and ESP8266_Spiram libraries
/* 
cd ~/Arduino/libraries
git clone https://github.com/earlephilhower/ESP8266Audio
git clone https://github.com/Gianbacchio/ESP8266_Spiram
Use the "Tools->ESP32 Sketch Data Upload" menu to write the MP3 to SPIFFS
Then upload the sketch normally.  
https://github.com/me-no-dev/arduino-esp32fs-plugin
*/
#include <Arduino.h>
#include <M5Stack.h>
#include <WiFi.h>
#include <IRremote.h>
#include "AudioFileSourceSD.h"
#include "AudioGeneratorWAV.h"
#include "AudioOutputI2S.h"
#include "HTTPClient.h"
#include "SPIFFS.h"

int switch_PIN = 2;
int switchOut;
int shootCount = 0;
int lifeCount = 0;

AudioGeneratorWAV *wav;
AudioFileSourceSD *file;
AudioOutputI2S *out;

//IR shoot
IRsend irsend; //create a IRsend object just apply for pin 9 in ATMega328

void setup()
{
  M5.begin();
  WiFi.mode(WIFI_OFF); 
  delay(500);
  Serial.begin(9600);
  
  M5.Lcd.setTextFont(2);
  M5.Lcd.printf("Sample WAV playback begins...\n");
  Serial.printf("Sample WAV playback begins...\n");
  
  // pno_cs from https://ccrma.stanford.edu/~jos/pasp/Sound_Examples.html
  file = new AudioFileSourceSD("/se_maoudamashii_battle_gun05.wav");
  //file = new AudioFileSourceSD("/pno-cs.wav");
  out = new AudioOutputI2S(0, 1); // Output to builtInDAC
  out->SetOutputModeMono(true);
  wav = new AudioGeneratorWAV();
  wav->begin(file, out);

  pinMode(switch_PIN, INPUT);
  Serial.begin(9600);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextFont(4);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 220);
  M5.Lcd.print("START!!");
  delay(2000);
}

void loop()
{
  M5.update();
  M5.Lcd.setTextColor(WHITE);
  //Show your shots
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.print("SHOT:");
  M5.Lcd.setCursor(80, 0);
  M5.Lcd.print(shootCount);
  //Show your live
  M5.Lcd.setCursor(0, 35);
  M5.Lcd.print("LIFE:");
  M5.Lcd.setCursor(80, 35);
  M5.Lcd.print(lifeCount);
  switchOut = digitalRead(switch_PIN);
  Serial.println(switchOut);
  if (switchOut == 1){
    if (wav->isRunning()) {
      if (!wav->loop()) wav->stop();
    }else{
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setTextColor(RED); 
      M5.Lcd.setCursor(0, 220);
      M5.Lcd.print("SHOOT!!!");
      shootCount = shootCount + 5;
      irsend.sendSony(0xa90, 12);
      delay(1000);
      M5.Lcd.fillScreen(BLACK);
    }
  }  
   
}