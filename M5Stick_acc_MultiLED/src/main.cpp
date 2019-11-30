#include <Arduino.h>
#include <M5StickC.h>
#include <FastLED.h>

#define NUM_LEDS 9
CRGB leds[NUM_LEDS];

float accX = 0.0F;
float accY = 0.0F;
float accZ = 0.0F;

float gyroX = 0.0F;
float gyroY = 0.0F;
float gyroZ = 0.0F;

float pitch = 0.0F;
float roll  = 0.0F;
float yaw   = 0.0F;

int LED_angle;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  M5.begin();
  M5.IMU.Init();
  M5.Lcd.setRotation(3);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setCursor(40, 0);
  M5.Lcd.println("IMU TEST");
  M5.Lcd.setCursor(0, 10);
  M5.Lcd.println("  X       Y       Z");
  M5.Lcd.setCursor(0, 50);
  M5.Lcd.println("  Pitch   Roll    Yaw");

  FastLED.addLeds<NEOPIXEL, 26>(leds, NUM_LEDS);
}

float temp = 0;
/*****************************************
M5.IMU.getGyroData(&gyroX,&gyroY,&gyroZ);
M5.IMU.getAccelData(&accX,&accY,&accZ);
M5.IMU.getAhrsData(&pitch,&roll,&yaw);
M5.IMU.getTempData(&temp);
*****************************************/
void loop() {
  // put your main code here, to run repeatedly:
  M5.IMU.getGyroData(&gyroX,&gyroY,&gyroZ);
  M5.IMU.getAccelData(&accX,&accY,&accZ);
  M5.IMU.getAhrsData(&pitch,&roll,&yaw);
  M5.IMU.getTempData(&temp);

  M5.Lcd.setCursor(0, 20);
  M5.Lcd.printf("LED ON!!");
  
  // accX等は普通のfloat型ではないので変換する必要がある
  float pitch_p;
  pitch_p = float(pitch);
  
  // Serial.println(accX_p);
  M5.Lcd.setCursor(140, 30);
  M5.Lcd.print("G");
  M5.Lcd.setCursor(0, 60);
  M5.Lcd.printf(" %5.2f   %5.2f   %5.2f   ", pitch, roll, yaw);

  LED_angle = int(pitch_p / 10);
  if(LED_angle > 0){
    for (int i = 0; i < LED_angle; i++){
      leds[i] = CRGB::Red;
      FastLED.show();
    }
  }
  if(LED_angle < 0){
    for (int i = 0; i < abs(LED_angle); i++){
      leds[i] = CRGB::Blue;
      FastLED.show();
    }
  }

  for (int i = 0; i < 8; i++){
      leds[i] = CRGB::Black;
      FastLED.show();
  }
  // for(int i = 0; i < 8; i++){
  //   leds[i] = CRGB::Red;
  //   delay(400);
  //   FastLED.show();
  // }
  // for(int i = 0; i < 8; i++){
  //   leds[i] = CRGB::Green;
  //   delay(400);
  //   FastLED.show();
  // }
  // for(int i = 0; i < 8; i++){
  //   leds[i] = CRGB::Blue;
  //   delay(400);
  //   FastLED.show();
  // }
  // delay(1000);
}