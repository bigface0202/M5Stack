// define must ahead #include <M5Stack.h>
#define M5STACK_MPU6886 
// #define M5STACK_MPU9250 
// #define M5STACK_MPU6050
// #define M5STACK_200Q

#include <M5Stack.h>

float accX = 0.0F;
float accY = 0.0F;
float accZ = 0.0F;

float gyroX = 0.0F;
float gyroY = 0.0F;
float gyroZ = 0.0F;

float pitch = 0.0F;
float roll  = 0.0F;
float yaw   = 0.0F;

float temp = 0.0F;

int past_accX = 0, past_accY = 0, past_accZ = 0;
int accX_point, accY_point, accZ_point;
unsigned long time_init, time_now;

// the setup routine runs once when M5Stack starts up
void setup(){
    time_init = millis();

  // Initialize the M5Stack object
    M5.begin();
  /*
    Power chip connected to gpio21, gpio22, I2C device
    Set battery charging voltage and current
    If used battery, please call this function in your project
  */
    M5.Power.begin();
    
    M5.IMU.Init();

    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(GREEN , BLACK);
    M5.Lcd.setTextSize(2);
}

// the loop routine runs over and over again forever
void loop() {
    time_now = millis() - time_init;

    if (time_now > 10000){
      M5.Lcd.fillScreen(BLACK);
      time_init = time_init + time_now;
      past_accX = 0;
      past_accY = 0;
      past_accZ = 0;
    }
    // put your main code here, to run repeatedly:
    M5.IMU.getGyroData(&gyroX,&gyroY,&gyroZ);
    M5.IMU.getAccelData(&accX,&accY,&accZ);
    M5.IMU.getAhrsData(&pitch,&roll,&yaw);
    M5.IMU.getTempData(&temp);

    accX_point = past_accX + int(accX * 9.8);
    accY_point = past_accY + int(accY * 9.8);
    accZ_point = past_accZ + int((accZ - 1.0) * 9.8);

    // if (accX_point > 160){
    //   accX_point = 160;
    // }else if(accX_point < -160){
    //   accX_point = -160;
    // }

    // if (accY_point > 160){
    //   accY_point = 160;
    // }else if(accY_point < -160){
    //   accY_point = -160;
    // }

    // if (accZ_point > 240){
    //   accZ_point = 240;
    // }else if(accZ_point < -240){
    //   accZ_point = -240;
    // }

    if (accX_point > 160){
      accX_point = 160;
    }else if(accX_point < -160){
      accX_point = -160;
    }

    if (accY_point > 160){
      accY_point = 160;
    }else if(accY_point < -160){
      accY_point = -160;
    }

    if (accZ_point > 120){
      accZ_point = 120;
    }else if(accZ_point < -120){
      accZ_point = -120;
    }

    M5.Lcd.setCursor(0, 20);
    M5.Lcd.printf(" %5.2f   %5.2f   %5.2f   ", accX, accY, accZ);
    M5.Lcd.setCursor(220, 42);
    M5.Lcd.print(" G");
    // M5.Lcd.fillCircle(160 - accY_point, 240 - accZ_point, 3, RED);
    M5.Lcd.fillCircle(160 - accX_point, 120 + accY_point, 3, RED);

    M5.Lcd.setCursor(0, 180);
    M5.Lcd.print(time_now);
  
    past_accX = accX_point;
    past_accY = accY_point;
    past_accZ = accZ_point;
}