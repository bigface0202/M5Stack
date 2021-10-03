/*server*/
#include <Arduino.h>
#include <WiFi.h>
#include <M5Stack.h>
#include "IMU.h"
#include "log_util.h"
// #include "MPU9250_asukiaaa.h"
// #ifdef _ESP32_HAL_I2C_H_

// デバイスのI2Cアドレス(Default モード)
// #define SENSOR_ADRS 0x48

// //SCL、SDAのピンをここで指定
// #define SDA_PIN 21
// #define SCL_PIN 22
// #endif

//センサー名を定義
// MPU9250 mySensor;

#define LCD
IMU *imu;
uint32_t last_displayed;
uint8_t value;

int measurementFlag = 0;
int Minute = 0, Second = 0, Milisec = 0;
int Timer_init;
int right_frontPGM = 5;
int left_frontPGM = 13;
int right_backPGM = 12;
int left_backPGM = 15;

static char measurementTime[12];

float ax, ay, az, gx, gy, gz, mx, my, mz;
float yaw, pitch, roll;

typedef struct
{
  unsigned long val_time;
  float ax1;
  float ay1;
  float az1;
} RecordType;

#define BufferRecords 1024

static RecordType buffer[2][BufferRecords];
static volatile int writeBank = 0;
static volatile int bufferIndex[2] = {0, 0};

const char *ssid = "ESP32-WiFi";          //SSID
const char *pass = "esp32wifi";           //PW
const IPAddress ip(192, 168, 0, 9);       //IP address of server
const IPAddress subnet(255, 255, 255, 0); //Subnetmask
WiFiServer server(80);

//タイマー割込み
hw_timer_t *timer = NULL;

volatile uint32_t isrCounter = 0;
volatile uint32_t lastIsrAt = 0;

volatile SemaphoreHandle_t timerSemaphore;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

// void MPU_setup()
// {
//   //MPU9250との通信準備
// #ifdef _ESP32_HAL_I2C_H_
//   Wire.begin(SDA_PIN, SCL_PIN);
// #else
//   //シリアル通信の開始
//   Wire.begin();
// #endif

//   //加速度、地磁気、ジャイロの値を取得
//   mySensor.setWire(&Wire);
//   mySensor.beginAccel();
//   mySensor.beginMag();
//   mySensor.beginGyro();

//   // 地磁気の値を調節(コメントアウト)
//   // mySensor.magXOffset = -50;
//   // mySensor.magYOffset = -55;
//   // mySensor.magZOffset = -10;
// }

// static void write_interupt()
// {
//   if (bufferIndex[writeBank] < BufferRecords)
//   {
//     RecordType *rp = &buffer[writeBank][bufferIndex[writeBank]];
//     rp->val_time = millis();
//     rp->ax1 = mySensor.accelX();
//     rp->ay1 = mySensor.accelY();
//     rp->az1 = mySensor.accelZ();
//     mySensor.accelUpdate();
//     if (++bufferIndex[writeBank] >= BufferRecords)
//     {
//       writeBank = !writeBank;
//     }
//   }
// }

// void IRAM_ATTR onTimer()
// {
//   // Increment the counter and set the time of ISR
//   portENTER_CRITICAL_ISR(&timerMux);

//   /*** ここにタイマー割り込みで実行するコードを記載 ***/
//   isrCounter++;
//   lastIsrAt = millis();
//   write_interupt();

//   portEXIT_CRITICAL_ISR(&timerMux);
//   // Give a semaphore that we can check in the loop
//   xSemaphoreGiveFromISR(timerSemaphore, NULL);
//   // It is safe to use digitalRead/Write here if you want to toggle an output
// }

void writeIMUdata()
{
  int Timer = millis();
  //Measure the time by program starting
  Timer = Timer - Timer_init;
  Minute = Timer / 60000;
  Second = Timer / 1000 - Minute * 60;
  Milisec = Timer - 1000 * Second - 60000 * Minute;

  auto frequency = imu->getFrequency();

  // Substitute the time for array of measurementTime
  sprintf(measurementTime, "%02d:%02d:%02d", Minute, Second, Milisec);
  imu->read(&ax, &ay, &az, &gx, &gy, &gz, &mx, &my, &mz, &yaw, &pitch, &roll);
  M5.Lcd.fillScreen(BLACK);
  displayIMU(ax, ay, az, gx, gy, gz, mx, my, mz, yaw, pitch, roll, frequency);
  File dataFile = SD.open("/datalog.txt", FILE_APPEND);
  if (dataFile)
  {
    dataFile.print(measurementTime);
    dataFile.print(",");
    dataFile.print(ax);
    dataFile.print(",");
    dataFile.print(ay);
    dataFile.print(",");
    dataFile.print(az);
    dataFile.print(",");
    dataFile.print(gx);
    dataFile.print(",");
    dataFile.print(gy);
    dataFile.print(",");
    dataFile.print(gz);
    dataFile.print(",");
    dataFile.print(yaw);
    dataFile.print(",");
    dataFile.print(pitch);
    dataFile.print(",");
    dataFile.println(roll);
    // dataFile.print(",");
    // dataFile.println(btnAPressed);
    dataFile.close();
  }
  // if the file isn't open, pop up an error:
  else
  {
    Serial.println("error opening datalog.txt");
  }
}

void setup()
{
  M5.begin();
  Serial.begin(115200);
  pinMode(right_frontPGM, OUTPUT);
  pinMode(left_frontPGM, OUTPUT);
  pinMode(right_backPGM, OUTPUT);
  pinMode(left_backPGM, OUTPUT);
  // displayWelcome();
  // delay(1000);
  // MPU_setup();

  // displayAdress("MPU9250", imu->getAddress(), 0x71);
  // delay(1000);
  // displayAdress("AK8963", imu->getAK8963Adress(), 0x48);
  // delay(1000);
  // displayAccelGyroBias(imu->getAccelBias(), imu->getGyroBias());
  // delay(1000);
  // displayMagCalibration(imu->getMagCalibration());
  // delay(1000);

  // M5.Lcd.fillScreen(BLACK);
  // M5.Lcd.setTextColor(WHITE, BLACK); // Set pixel color; 1 on the monochrome screen
  // M5.Lcd.setTextSize(2);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextFont(4);
  M5.Lcd.setTextColor(WHITE);

  //IMU start!!
  //    imu = new IMU();

  // Prepare I2C protocol.
  Wire.begin(21, 22); //Define(SDA, SCL)
  delay(50);

  //Prepare SD card.
  if (!SD.begin())
  {
    // Fill screen black
    M5.Lcd.fillScreen(BLACK);
    Serial.println("Card failed, or not present");
    // don't do anything more:
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.setTextSize(2);
    M5.Lcd.print("Not find the SD card!!!");
    M5.Lcd.setTextSize(2);
    while (1)
      ;
  }

  M5.Lcd.setCursor(0, 0);
  M5.Lcd.print("SD CARD OK!!");

  WiFi.softAP(ssid, pass);           //Set the SSID and PW
  delay(100);                        //Delay to avoid the deconnection
  WiFi.softAPConfig(ip, ip, subnet); //Set IP address, Gateway, and Subnetmask
  IPAddress myIP = WiFi.softAPIP();  //WiFi start
  server.begin();                    //Server start
  Serial.println();
  /*各種情報を表示*/
  Serial.println("WiFi connected");
  Serial.print("SSID：");
  Serial.println(ssid);
  Serial.print("AP IP address：");
  Serial.println(myIP);
  Serial.println("Server start!");

  M5.Lcd.setCursor(0, 40);
  M5.Lcd.print("WiFi connected");
  M5.Lcd.setCursor(0, 80);
  M5.Lcd.print("Server start!");

  //Initialize the datalog.txt
  // File dataFile = SD.open("/sample.txt", FILE_WRITE);
  // if (dataFile)
  // {
  //   dataFile.println("Time,Ax,Ay,Az,Gx,Gy,Gz,yaw,pitch,roll,btnA");
  //   dataFile.close();
  // }
  // // if the file isn't open, pop up an error:
  // else
  // {
  //   Serial.println("error opening datalog.txt");
  // }

  File dataFile = SD.open("/datalog.txt", FILE_WRITE);
  if (dataFile)
  {
    dataFile.println("Time,Ax,Ay,Az,Gx,Gy,Gz,yaw,pitch,roll,btnA");
    dataFile.close();
  }
  // if the file isn't open, pop up an error:
  else
  {
    Serial.println("error opening datalog.txt");
  }
  //タイマー割込み用
  // //セマフォ管理
  // timerSemaphore = xSemaphoreCreateBinary();

  // //プリスケーラを80分周器にする(ESP Reference参照)
  // timer = timerBegin(0, 80, true);
  // //タイムアウトした時に行う処理(関数)を定義
  // timerAttachInterrupt(timer, &onTimer, true);
  // //何秒毎に行うか定義
  // timerAlarmWrite(timer, 100000, true);
  // //タイマースタート
  // timerAlarmEnable(timer);
}

void loop()
{
  // if (xSemaphoreTake(timerSemaphore, 0) == pdTRUE)
  // {
  //   uint32_t isrCount = 0, isrTime = 0;
  //   // Read the interrupt count and time
  //   portENTER_CRITICAL(&timerMux);
  //   isrCount = isrCounter;
  //   isrTime = lastIsrAt;
  //   portEXIT_CRITICAL(&timerMux);

  //   static bool save = true;
  //   int readBank = !writeBank;

  //   // if (!digitalRead(PIN_SW)) {
  //   //     save = false;
  //   // }
  //   if (save && bufferIndex[readBank] >= BufferRecords)
  //   {
  //     static RecordType temp[BufferRecords];

  //     memcpy(temp, buffer[readBank], sizeof(temp));
  //     bufferIndex[readBank] = 0;
  //     File dataFile = SD.open("/sample.txt", FILE_APPEND);
  //     if (dataFile)
  //     {
  //       for (int i = 0; i < BufferRecords; i++)
  //       {
  //         dataFile.print(temp[i].val_time);
  //         dataFile.print(",");
  //         dataFile.print(temp[i].ax1);
  //         dataFile.print(",");
  //         dataFile.print(temp[i].ay1);
  //         dataFile.print(",");
  //         dataFile.println(temp[i].az1);
  //       }
  //       dataFile.close();
  //     }
  //     else
  //     {
  //       Serial.println("Fail to access MMC");
  //       // digitalWrite(PIN_LED0, 0);
  //       delay(100);
  //     }
  //   }
  //   // Print it
  //   Serial.print("onTimer no. ");
  //   Serial.print(isrCount);
  //   Serial.print(" at ");
  //   Serial.print(isrTime);
  //   Serial.println(" ms");
  // }
  // int Timer = millis();
  // //Measure the time by program starting
  // Timer = Timer - Timer_init;
  // Minute = Timer / 60000;
  // Second = Timer / 1000 - Minute * 60;
  // Milisec = Timer - 1000 * Second - 60000 * Minute;

  // auto frequency = imu->getFrequency();

  // // Substitute the time for array of measurementTime
  // sprintf(measurementTime, "%02d:%02d:%02d", Minute, Second, Milisec);
  // Serial.println(measurementTime);

  //Get the information of server
  WiFiClient client = server.available();
  if (client)
  {
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.setCursor(0, 120);
    M5.Lcd.print("New client");
    Serial.println("new client");
    delay(2000);
    M5.Lcd.fillScreen(BLACK);
    while (client.connected())
    {
      if (client.available())
      {
        //Read the client signal
        value = client.read();
        if (value == '1')
        {
          Serial.println("1 is received");
          M5.Lcd.fillScreen(BLACK);
          M5.Lcd.setCursor(0, 0);
          M5.Lcd.print("Turn on right PGM");
          digitalWrite(right_frontPGM, HIGH);
          digitalWrite(right_backPGM, HIGH);
          delay(500);
          digitalWrite(right_frontPGM, LOW);
          digitalWrite(right_backPGM, LOW);
        }
        else if (value == '2')
        {
          Serial.println("2 is received");
          M5.Lcd.fillScreen(BLACK);
          M5.Lcd.setCursor(0, 0);
          M5.Lcd.print("Turn on left PGM");
          digitalWrite(left_frontPGM, HIGH);
          digitalWrite(left_backPGM, HIGH);
          delay(500);
          digitalWrite(left_frontPGM, LOW);
          digitalWrite(left_backPGM, LOW);
        }
        else if (value == '3')
        {
          Serial.println("3 is received");
          M5.Lcd.fillScreen(BLACK);
          M5.Lcd.setCursor(0, 0);
          M5.Lcd.print("Turn on front PGM");
          digitalWrite(right_frontPGM, HIGH);
          digitalWrite(left_frontPGM, HIGH);
          delay(500);
          digitalWrite(right_frontPGM, LOW);
          digitalWrite(left_frontPGM, LOW);
        }
        else if (value == '4')
        {
          Serial.println("4 is received");
          M5.Lcd.fillScreen(BLACK);
          M5.Lcd.setCursor(0, 0);
          M5.Lcd.print("Turn on back PGM");
          digitalWrite(right_backPGM, HIGH);
          digitalWrite(left_backPGM, HIGH);
          delay(500);
          digitalWrite(right_backPGM, LOW);
          digitalWrite(left_backPGM, LOW);
        }
        if (value == '5')
        {
          Serial.println("5 is received");
          M5.Lcd.fillScreen(BLACK);
          M5.Lcd.setCursor(0, 0);
          M5.Lcd.print("Measurement start!!");
          measurementFlag = 1;
        }
      }
      if (measurementFlag == 1)
      {
        writeIMUdata();
      }
    }
  }
}