#include <Arduino.h>
#include <Wire.h>
#include <M5Stack.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <DHT_U.h>


#define processing_out false
#define AHRS true         // Set to false for basic data read
#define SerialDebug true  // Set to true to get Serial output for debugging
#define LCD
#define PWM_PIN 21
#define DHTPIN 3
#define DHTTYPE DHT11

int prev_val = LOW;
long high_level_start = 0L;
long high_level_end = 0L;
long low_level_start = 0L;
long low_level_end = 0L;
int ppm = 0, time_index = 0;

int x[10];
int y[10];
int vals[10];

boolean is_output_ready = false;
DHT_Unified dht(DHTPIN, DHTTYPE);

//uint32_t delayMS;

//uint32_t delayMS;

void setup() {
  Serial.begin(9600);
  pinMode(PWM_PIN, INPUT);
  pinMode(DHTPIN, INPUT);
  M5.begin();
  Wire.begin();
  dht.begin();

  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextFont(4);
  for (int i = 0; i < 10; i ++){
    x[i] = 0;
    y[i] = 0;
  }
}

void loop() {
  M5.update();
  //delay(delayMS);
  long cycle_start_time = millis();
  int pin_in = digitalRead(PWM_PIN);

  if (pin_in == HIGH) {
    if (prev_val == LOW) {
      long time_high = high_level_end - high_level_start;
      long time_low = low_level_end - low_level_start;
      ppm = 5000 * (time_high - 0.002) / (time_high + time_low - 0.004);
      is_output_ready = true;

      high_level_start = cycle_start_time;
      prev_val = HIGH;
    } else {
      high_level_end = cycle_start_time;
    }
  } else {
    if (prev_val == HIGH) {
      low_level_start = cycle_start_time;
      prev_val = LOW;
    } else {
      low_level_end = cycle_start_time;
    }
  }

  if (is_output_ready) {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 220);
    M5.Lcd.print("CO2:");
    M5.Lcd.print(ppm);

    M5.Lcd.setCursor(50, 180);
    M5.Lcd.print("0");

    M5.Lcd.setCursor(5, 115);
    M5.Lcd.print("1000");

    M5.Lcd.drawLine(64, 0, 64, 192, 0x2589);
    M5.Lcd.drawLine(64, 192, 320, 192, 0x2589);

    // Get temperature event and print its value.
    //sensors_event_t event;  
    //dht.temperature().getEvent(&event);
    M5.Lcd.setCursor(200, 220);
    M5.Lcd.print("Temp:");
    //M5.Lcd.print(event.temperature);

    vals[time_index] = ppm;
    x[time_index] = map(time_index, 0, 9, 64, 320);
    y[time_index] = map(vals[time_index], 0, 2500, 192, 0);
    for(int j = 1; j < 10; j++){
      M5.Lcd.drawLine(x[j-1], y[j-1], x[j], y[j], 0xfff);
    }

    time_index++;
    if (time_index > 9){
      time_index = 0;
    }
    //M5.Lcd.drawLine(x0,y0,x1,x2,0xfff)
    //M5.Lcd.drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);


    is_output_ready = false;
  }
}
