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
int measurementFlag = 0;

char fileName[16];
int fileNum = 0;


MPU6050 mpu6050(Wire);
 
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
        // don't do anything more:
        M5.Lcd.setCursor(0, 0);
        M5.Lcd.setTextSize(2);
        M5.Lcd.print("Not find the SD card!!!");
        M5.Lcd.setTextSize(1);
        while (1)
            ;
    }
  
  mpu6050.begin();
  mpu6050.setGyroOffsets(-7.56, 3.11, 4.86);
  mpu6050.calcGyroOffsets(false);
 
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
  
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.println("Welcome to M5Stack");
  // Make file name
  String s;

  while(1){
    s = "/LOG";
    if (fileNum < 10) {
      s += "00";
    } else if(fileNum < 100) {
      s += "0";
    }
    s += fileNum;
    s += ".txt";
    s.toCharArray(fileName, 16);
    if(!SD.exists(fileName)) break;
    fileNum++;
  }

    M5.Lcd.setCursor(0, 20);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.print("Log file number is ");
    M5.Lcd.println(fileNum);

   //Initialize the datalog.txt
    File logFile = SD.open(fileName, FILE_WRITE);
    if (logFile)
    {
        logFile.println("Time,Ax,Ay,Az,R,L,F,B,M");
        logFile.close();
    }
    // if the file isn't open, pop up an error:
    else
    {
        M5.Lcd.setCursor(0, 0);
        M5.Lcd.setTextSize(2);
        M5.Lcd.print("Not find the SD card!!!");
        M5.Lcd.setTextSize(1);
    }
    
  delay(3000);
  M5.Lcd.fillScreen(BLACK);
  Timer_init = millis();
}

void loop() {
  File logFile = SD.open(fileName, FILE_APPEND);

  // If Timer has fired
  if (xSemaphoreTake(timerSemaphore, 0) == pdTRUE){
    int rightSidePGMFlag = 0;
    int leftSidePGMFlag = 0;
    int frontSidePGMFlag = 0;
    int backSidePGMFlag = 0;
    int inByte = Serial.read();
    int Timer_init2;
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.setTextSize(3);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.print("File number is ");
    M5.Lcd.println(fileNum);
    M5.Lcd.setCursor(0, 30);
    M5.Lcd.print("I'm ready");
    mpu6050.update();

    int Timer = millis();
    static char measurementTime[49];

    //Measure the time by program starting
    Timer = Timer - Timer_init;
    Minute = Timer / 60000;
    Second = Timer / 1000 - Minute * 60;
    Milisec = Timer - 1000 * Second - 60000 * Minute;
    
    //Receive the signal from PGM control board
    if(inByte == 1){
      //Right side PGM actuate
      rightSidePGMFlag = 1;
//      M5.Lcd.fillScreen(BLACK);
//      M5.Lcd.setCursor(0, 90);
//      M5.Lcd.print("R");
    }else if(inByte == 2){
      leftSidePGMFlag = 1;
//      M5.Lcd.fillScreen(BLACK);
//      M5.Lcd.setCursor(0, 90);
//      M5.Lcd.print("L");
    }else if(inByte == 3){
      frontSidePGMFlag = 1;
//      M5.Lcd.fillScreen(BLACK);
//      M5.Lcd.setCursor(0, 90);
//      M5.Lcd.print("F");
    }else if(inByte == 4){
      backSidePGMFlag = 1;
//      M5.Lcd.fillScreen(BLACK);
//      M5.Lcd.setCursor(0, 90);
//      M5.Lcd.print("B");
    }else if(inByte == 5){
      measurementFlag = 1;
//      M5.Lcd.fillScreen(BLACK);
//      M5.Lcd.setCursor(0, 60);
//      M5.Lcd.print("Measurement start!!");
    }
    
    // Substitute the time for array of measurementTime
    sprintf(measurementTime, "%02d:%02d:%03d,%5f,%5f,%5f,%d,%d,%d,%d,%d", Minute, Second, Milisec, mpu6050.getAccX(), mpu6050.getAccY(), mpu6050.getAccZ(), rightSidePGMFlag, leftSidePGMFlag, frontSidePGMFlag, backSidePGMFlag, measurementFlag);    
    
    logFile.println(measurementTime);
    logFile.close();
  
  // M5 Loop
  }
  M5.update();
}
