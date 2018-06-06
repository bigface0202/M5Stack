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
#include "IRremote.h"
#include "AudioFileSourceSD.h"
#include "AudioGeneratorWAV.h"
#include "AudioOutputI2S.h"
#include "HTTPClient.h"
#include "SPIFFS.h"
/*Pin*/
int switch_PIN = 2; //SHOT switch
int IR_receptorPin = 17;//Pin used to read IR values

int switchOut = 0;
int last_switchOut = 0;
int shootCount = 0;
int lifeCount = 0;
int heartCount = 0;
int i = 0;

AudioGeneratorWAV *wav;
AudioFileSourceSD *file_shoot;
AudioOutputI2S *out;
/*IR*/
IRsend irsend; //Pin number 3 is IR
IRrecv irrecv(IR_receptorPin);//Create an object
decode_results results;

//Function
void IR_Receptor();
void IR_Transmitter();
void Game_over();

void IR_Receptor() {
  if (irrecv.decode(&results)) {
    if (results.decode_type == SONY) {
      lifeCount++;
      M5.Lcd.setCursor(0, 220);
      M5.Lcd.print("HIT!!!");
      delay(1000);
      M5.Lcd.setTextColor(BLACK);
      M5.Lcd.setCursor(0, 220);
      M5.Lcd.print("HIT!!!");
      M5.Lcd.setTextColor(WHITE);
      M5.Lcd.fillCircle(205-(25*lifeCount),55,10,BLACK); //Remove a life
    }
    irrecv.resume(); // Receive the next value
  }
}

void IR_Transmitter() {
  /*Shoot switch was pressed*/
  /*Read switch*/
  switchOut = digitalRead(switch_PIN);
  if(last_switchOut != switchOut){
    if (switchOut == HIGH){
      irsend.sendSony(0xa90, 12);

      /*Make shooting sound*/
      file_shoot->close();
      file_shoot = new AudioFileSourceSD("/se_maoudamashii_battle_gun05.wav");
      wav->begin(file_shoot, out);
      dacWrite(25, 0);

      //Showing the SHOOT!!!!
      M5.Lcd.setTextColor(RED); 
      M5.Lcd.setCursor(0, 220);
      M5.Lcd.print("SHOOT!!!");

      //Overwrite the shootCount
      M5.Lcd.setTextColor(BLACK);
      M5.Lcd.setCursor(80, 10);
      M5.Lcd.print(shootCount);
      shootCount = shootCount + 1;
      M5.Lcd.setTextColor(WHITE);
      M5.Lcd.setCursor(80, 10);
      M5.Lcd.print(shootCount);

      //Delete the SHOOT!!!!
      delay(100);
      M5.Lcd.setTextColor(BLACK);
      M5.Lcd.setCursor(0, 220);
      M5.Lcd.print("SHOOT!!!");
      M5.Lcd.setTextColor(WHITE);
    }
  }
  last_switchOut = switchOut;
}
void Game_over(){
  if(lifeCount > 4){
    M5.Lcd.fillScreen(BLUE);
    M5.Lcd.setTextFont(4);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setCursor(70, 120);
    M5.Lcd.print("GAME OVER");
    delay(100000);
    exit(0);
  }
}

void setup()
{
  M5.begin();
  WiFi.mode(WIFI_OFF); 
  delay(500);
  Serial.begin(9600);

  /*Audio setup*/
  /*Please move music file(se_maoudamashii_battle_gun05.wav) into SD.
  This file put on the music folder*/
  file_shoot = new AudioFileSourceSD("/se_maoudamashii_battle_gun05.wav");
  out = new AudioOutputI2S(0, 1); // Output to builtInDAC
  out->SetOutputModeMono(true);
  wav = new AudioGeneratorWAV();
  //wav->begin(file, out);

  /*Game setup*/
  pinMode(switch_PIN, INPUT);
  M5.Lcd.setTextFont(4);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setCursor(120, 120);
  M5.Lcd.print("START!!");
  delay(2000);
  M5.Lcd.fillScreen(BLACK);

  /*IRrecev setup*/
  irrecv.enableIRIn(); // Start the receiver
  
  /*Show your live*/
  M5.Lcd.setCursor(0, 45);
  M5.Lcd.print("LIFE:");
  M5.Lcd.setCursor(80, 45);
  //M5.Lcd.print(lifeCount);
  M5.Lcd.fillCircle(80,55,10,RED);
  M5.Lcd.fillCircle(105,55,10,RED);
  M5.Lcd.fillCircle(130,55,10,RED);
  M5.Lcd.fillCircle(155,55,10,RED);
  M5.Lcd.fillCircle(180,55,10,RED);

  /*Show your shots*/
  M5.Lcd.setCursor(0, 10);
  M5.Lcd.print("SHOT:");
  M5.Lcd.setCursor(80, 10);
  M5.Lcd.print(shootCount);
}

void loop()
{
  /*LCD setup*/
  M5.Lcd.setTextFont(4);
  M5.Lcd.drawRect(120,100,200,140,WHITE);

  /*IR Receptor*/
  IR_Receptor();

  /*IR Transmitter*/
  IR_Transmitter();

  /*Game over*/
  Game_over();

  /*??*/
  if(wav->isRunning()){
    if(!wav->loop()){
      wav->stop();
    }
  }

  /*Test code*/
  if (M5.BtnB.wasPressed()){
    /*Music test*/
    file_shoot->close();
    file_shoot = new AudioFileSourceSD("/se_maoudamashii_battle_gun05.wav");
    wav->begin(file_shoot, out);
    dacWrite(25, 0);
  }else if (M5.BtnC.wasPressed()){
    /*Showing like the heart beat*/
    while(heartCount < 3){
      M5.Lcd.drawLine(140,170,170,170,RED);
      M5.Lcd.drawLine(170,170,200,110,RED);
      M5.Lcd.drawLine(200,110,230,220,RED);
      M5.Lcd.drawLine(230,220,260,170,RED);
      M5.Lcd.drawLine(260,170,300,170,RED);
      M5.Lcd.fillRect(125+(i*10),105,200-(i*10),130,0x7bef);
      delay(100);
      i++;
      if(i > 20) {
        i = 0;
        M5.Lcd.drawLine(140,170,170,170,0x7bef);
        M5.Lcd.drawLine(170,170,200,110,0x7bef);
        M5.Lcd.drawLine(200,110,230,220,0x7bef);
        M5.Lcd.drawLine(230,220,260,170,0x7bef);
        M5.Lcd.drawLine(260,170,300,170,0x7bef);
        heartCount++;
      }
    }
    heartCount = 0;
  }
  M5.update();
}
