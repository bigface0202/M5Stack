#include <Arduino.h>
#include <M5StickC.h>

//  Variables
// Pulse Sensor PURPLE WIRE connected to ANALOG PIN 0
int PulseSensorPurplePin = 26;
//  The on M5StickC's LED    
int LED = 10;   

// holds the incoming raw data.
int Signal;                       

// The SetUp Function:
void setup() {
  // pin that will blink to your heartbeat!
  pinMode(LED,OUTPUT);
  // Set up Serial Communication at certain speed.
  Serial.begin(9600);
  // Set up M5Stick
  M5.begin();
  // Rotate the screen 270 deg.
  M5.Lcd.setRotation(3);
  // You can change the screen color when you change this function.
  M5.Lcd.fillScreen(BLACK);
  // You can change the font size when you change this function.
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.println("Pulse sensor start!!");
  delay(2000);
  M5.Lcd.fillScreen(BLACK);
}

float normalized_signal;
int elapsed_time = 0;
// The Main Loop Function
void loop() {
  // Read the PulseSensor's value.
  Signal = analogRead(PulseSensorPurplePin);  
  normalized_signal = Signal / 3100.0;
  // Plot the signal value in LCD display.               
  M5.Lcd.fillCircle(elapsed_time, int(30 * normalized_signal), 3, RED);

  elapsed_time++;
  // If the plot reach x range, the x-value and screen are renewed.
  if(elapsed_time == 180){
    elapsed_time = 0;
    M5.Lcd.fillScreen(BLACK);
  }
  delay(5);
}