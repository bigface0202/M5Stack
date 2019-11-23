#include <Arduino.h>
#include <M5StickC.h>
#include <WiFi.h>

const char *ssid = "ESP32-WiFi3"; //SSID of server
const char *pass = "esp32wifi3";  //PW of server
static WiFiClient client;        //Declear client

float accX = 0.0F;
float accY = 0.0F;
float accZ = 0.0F;

float gyroX = 0.0F;
float gyroY = 0.0F;
float gyroZ = 0.0F;

float pitch = 0.0F;
float roll  = 0.0F;
float yaw   = 0.0F;

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

  WiFi.disconnect(true);
  WiFi.begin(ssid, pass); //Connect to server
  Serial.printf("\n");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(1000);
    }
  Serial.printf("\n");
  Serial.println("WiFi Connected");
  IPAddress ip(192, 168, 0, 10); //IP address of server
  client.connect(ip, 80);       //Connect to IP address and port number
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
  
  // accX等は普通のfloat型ではないので変換する必要がある
  float pitch_p;
  pitch_p = float(pitch);

  M5.Lcd.setCursor(0, 20);
  M5.Lcd.printf("%6.2f  %6.2f  %6.2f      ", gyroX, gyroY, gyroZ);
  M5.Lcd.setCursor(140, 20);
  M5.Lcd.print("o/s");
  M5.Lcd.setCursor(0, 30);
  M5.Lcd.printf(" %5.2f   %5.2f   %5.2f   ", accX, accY, accZ);
  
  // Serial.println(accX_p);
  M5.Lcd.setCursor(140, 30);
  M5.Lcd.print("G");
  M5.Lcd.setCursor(0, 60);
  M5.Lcd.printf(" %5.2f   %5.2f   %5.2f   ", pitch, roll, yaw);

  M5.Lcd.setCursor(0, 70);
  M5.Lcd.printf("Temperature : %.2f C", temp);

  if (pitch_p > 60.0){
    client.println("a");
    Serial.println("send");
  }
  client.println("k");
  // delay(1000);
}