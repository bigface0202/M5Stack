#include <Arduino.h>
#include <Adafruit_MLX90614.h>
#include <M5Stack.h>

Adafruit_MLX90614 mlx = Adafruit_MLX90614();

// Emissivity(放射率)
// Human skin: 0.98
// PP: 0.86
// float Em = 0.98;
float Em = 0.86;

float TempEm(float Em, float TA, float TO);

void setup() {
  Serial.begin(115200);
  M5.begin();
  mlx.begin();

  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(3);

  M5.Lcd.setCursor(0, 0);
  M5.Lcd.print("Start!!");
  delay(2000);
}

void loop() {
  M5.update();
  M5.Lcd.fillRect(0, 0, 240, 90, BLACK);
  float temp_obj_c = mlx.readObjectTempC();
  float temp_amb_c = mlx.readAmbientTempC();
  float temp_crr_c = TempEm(Em, temp_amb_c, temp_obj_c);

  M5.Lcd.setCursor(0, 0);
  M5.Lcd.print("Amb.:" + String(temp_amb_c, 1) + "deg.(C)");
  M5.Lcd.setCursor(0, 30);
  M5.Lcd.print("Obj.:" + String(temp_obj_c, 1) + "deg.(C)");
  M5.Lcd.setCursor(0, 60);
  M5.Lcd.print("Corr.:" + String(temp_crr_c, 1) + "deg.(C) ");

  delay(200);
}

float TempEm(float Em, float TA, float TO){
  TA = TA + 273.15;
  TO = TO + 273.15;
  float T = (TO * TO * TO * TO) / Em + (TA * TA * TA * TA) * (1 - (1 / Em));
  float Temp = sqrt(sqrt(T)) - 273.15;
  return Temp;
}