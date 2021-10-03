#include <M5Stack.h>
#include "IMU.h"
#include "log_util.h"
//#include <WiFi.h>

#define LCD

IMU *imu;
//BluetoothSerial SerialBT;
uint32_t last_displayed;

int Minute = 0, Second = 0, Milisec = 0;
int Timer_init;

void setup()
{
    M5.begin();
    Wire.begin();
    Serial.begin(115200);
    displayWelcome();
    delay(1000);

    imu = new IMU();

    displayAdress("MPU9250", imu->getAddress(), 0x71);
    delay(1000);
    displayAdress("AK8963", imu->getAK8963Adress(), 0x48);
    delay(1000);
    displayAccelGyroBias(imu->getAccelBias(), imu->getGyroBias());
    delay(1000);
    displayMagCalibration(imu->getMagCalibration());
    delay(1000);

    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(GREEN, BLACK);
    M5.Lcd.fillScreen(BLACK);

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
    Serial.println("card initialized.");

    //Initialize the datalog.txt
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
    Timer_init = millis();
}

void loop()
{
    int Timer = millis();
    static char measurementTime[12];

    //Measure the time by program starting
    Timer = Timer - Timer_init;
    Minute = Timer / 60000;
    Second = Timer / 1000 - Minute * 60;
    Milisec = Timer - 1000 * Second - 60000 * Minute;

    // Substitute the time for array of measurementTime
    sprintf(measurementTime, "%02d:%02d:%03d", Minute, Second, Milisec);
    Serial.println(measurementTime);

    float ax, ay, az, gx, gy, gz, mx, my, mz;
    float yaw, pitch, roll;
    imu->read(&ax, &ay, &az, &gx, &gy, &gz, &mx, &my, &mz, &yaw, &pitch, &roll);

    uint32_t now = millis();
    if (now - last_displayed > 200)
    {
        auto frequency = imu->getFrequency();
        displayIMU(ax, ay, az, gx, gy, gz, mx, my, mz, yaw, pitch, roll, frequency);
        last_displayed = now;
    }
    uint8_t btnAPressed = 0;
    if (M5.BtnA.wasPressed())
    {
        btnAPressed = 1;
    }
    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    // FILE_APPEND means that adding the data in new line.
    // FILE_WRITE means that overwriting the data in just top line.
    File dataFile = SD.open("/datalog.txt", FILE_APPEND);

    // Serial.printf(
    //     "axyzgxyzypr\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%f\t%d\t\r\n",
    //     ax, ay, az, gx, gy, gz, yaw, pitch, roll,
    //     btnAPressed);

    // if the file is available, write to it:
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
        dataFile.print(roll);
        dataFile.print(",");
        dataFile.println(btnAPressed);
        dataFile.close();
    }
    // if the file isn't open, pop up an error:
    else
    {
        Serial.println("error opening datalog.txt");
    }

    M5.update();
}
