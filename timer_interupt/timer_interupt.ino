/*
 Repeat timer example
 
 This example shows how to use hardware timer in ESP32. The timer calls onTimer
 function every second. The timer can be stopped with button attached to PIN 0
 (IO0).
 
 This example code is in the public domain.
 */
#include <M5Stack.h>
#include <MPU6050_tockn.h>
#include <Wire.h>

int Minute = 0, Second = 0, Milisec = 0;
int Timer_init;

MPU6050 mpu6050(Wire);

// Stop button is attached to PIN 0 (IO0)
#define BTN_STOP_ALARM    0
 
hw_timer_t * timer = NULL;
volatile SemaphoreHandle_t timerSemaphore;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
 
volatile uint32_t isrCounter = 0;
volatile uint32_t lastIsrAt = 0;

void IRAM_ATTR onTimer(){
  // Increment the counter and set the time of ISR
  portENTER_CRITICAL_ISR(&timerMux);
  isrCounter++;
  lastIsrAt = millis();
  portEXIT_CRITICAL_ISR(&timerMux);
  // Give a semaphore that we can check in the loop
  xSemaphoreGiveFromISR(timerSemaphore, NULL);
  // It is safe to use digitalRead/Write here if you want to toggle an output
}

void setup() {
  Serial.begin(115200);
  M5.begin();
  Wire.begin();
  Wire.setClock(400000UL);  // Set I2C frequency to 400kHz

// see if the card is present and can be initialized:
    if (!SD.begin())
    {
        // Fill screen black
        M5.Lcd.fillScreen(BLACK);
        Serial.println("Card failed, or not present");
        // don't do anything more:
        M5.Lcd.setCursor(0, 0);
        M5.Lcd.setTextSize(2);
        M5.Lcd.print("Not find the SD card!!!");
        M5.Lcd.setTextSize(1);
        while (1)
            ;
    }
  
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);
 
  // Set BTN_STOP_ALARM to input mode
  pinMode(BTN_STOP_ALARM, INPUT);
 
  // Create semaphore to inform us when the timer has fired
  timerSemaphore = xSemaphoreCreateBinary();
 
  // Use 1st timer of 4 (counted from zero).
  // Set 80 divider for prescaler (see ESP32 Technical Reference Manual for more
  // info).
  timer = timerBegin(0, 80, true);
 
  // Attach onTimer function to our timer.
  timerAttachInterrupt(timer, &onTimer, true);
 
  // Set alarm to call onTimer function every second (value in microseconds).
  // Repeat the alarm (third parameter)
  timerAlarmWrite(timer, 10000, true);
 
  // Start an alarm
  timerAlarmEnable(timer);

   //Initialize the datalog.txt
    File dataFile = SD.open("/datalog.txt", FILE_WRITE);
    if (dataFile)
    {
        dataFile.println("Time,Ax,Ay,Az,btnA");
        dataFile.close();
    }
    // if the file isn't open, pop up an error:
    else
    {
        Serial.println("error opening datalog.txt");
    }
  
  Timer_init = millis();
}

void loop() {
  File dataFile = SD.open("/datalog.txt", FILE_APPEND);
  // If Timer has fired
  if (xSemaphoreTake(timerSemaphore, 0) == pdTRUE){
    mpu6050.update();

    int Timer = millis();
    static char measurementTime[39];

    //Measure the time by program starting
    Timer = Timer - Timer_init;
    Minute = Timer / 60000;
    Second = Timer / 1000 - Minute * 60;
    Milisec = Timer - 1000 * Second - 60000 * Minute;

    // Substitute the time for array of measurementTime
    sprintf(measurementTime, "%02d:%02d:%03d,%5f,%5f,%5f", Minute, Second, Milisec,mpu6050.getAccX(),mpu6050.getAccY(),mpu6050.getAccZ());
    Serial.println(measurementTime);
    
    dataFile.println(measurementTime);
    dataFile.close();
//
////    // if the file is available, write to it:
//    if (dataFile)
//    {
//        dataFile.println(measurementTime);
////        dataFile.print(",");
////        dataFile.printf("%5f", mpu6050.getAccX());
////        dataFile.print(",");
////        dataFile.printf("%5f", mpu6050.getAccY());
////        dataFile.print(",");
////        dataFile.printf("%5f", mpu6050.getAccZ());
////        dataFile.print(",");
////        dataFile.println(btnAPressed);
//        dataFile.close();
//    }
//    // if the file isn't open, pop up an error:
//    else
//    {
//        Serial.println("error opening datalog.txt");
//    }
  
  // M5 Loop
  }
  // If button is pressed
  if (M5.BtnA.wasPressed()) {
    // If timer is still running
    if (timer) {
      // Stop and free timer
      timerEnd(timer);
      timer = NULL;
    }
  }
  M5.update();
}
