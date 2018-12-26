#include <ESPmDNS.h>
#include <NTPClient.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WebServer.h>
#include <M5Stack.h>
#include "IMU.h"
#include "log_util.h"

//Test
int ledPin = 2;
int measurementFlag = 0;

// Process mode
#define MODE_ADJUST_PARAM 1   // Response current values
#define MODE_DO_MEASUREMENT 2 // Send records from SD limited the range of record position
#define MODE_DATE 3           // Send records from SD limited the range of date or date-time
#define MODE_MAINTE 11        // Create maintenance screen

#define LCD

IMU *imu;
uint32_t last_displayed;

int Minute = 0, Second = 0, Milisec = 0;
int Timer_init;

// WiFi connection
char ssid[33] = {"BSYS-A1-512-HUMAN"};
char password[64] = {"bsys12bsys34"};

// WiFi response control
WebServer server(80);
String strFromDate, strToDate;
long iRecPos, iRecNo;

// WiFi Response constants
const String strResponseHeader = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
const String strHeader = "<!DOCTYPE html>\r\n"
                         "<html><head><meta charset=\"utf-8\"><title>AirMonitor</title>"
                         "<style>body{line-height:120%;font-family:monospace;}</style>"
                         "</head><body><form name='resultform' target='_self' method='get'>";
const String strFooter = "&nbsp;　&nbsp;<INPUT type='submit' value='　戻 る　' autofocus /></form></body></html>";
const String strTitle = "No., Temp(C), Press(hPa), Hum(%), Illum(lx), Date Time";

/*****************************************************************************
 *                          Predetermined Sequence                           *
 *****************************************************************************/
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

    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(WHITE, BLACK); // Set pixel color; 1 on the monochrome screen
    M5.Lcd.setTextSize(2);

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
        M5.Lcd.setTextSize(1);
        while (1)
            ;
    }
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.print("SD CARD OK!!");

    // Prepare WiFi system.
    int dotPosition = 0;
    WiFi.disconnect(true);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
        M5.Lcd.setCursor(0, 25);
        M5.Lcd.print("WiFi conecting");
        M5.Lcd.setCursor(dotPosition, 50);
        M5.Lcd.print(".");
        dotPosition += 5;
    }
    server.begin();
    Serial.print("\nServer started!  IP: ");
    Serial.println(WiFi.localIP());
    M5.Lcd.setCursor(0, 40);
    M5.Lcd.print("Server started!!");
    M5.Lcd.setCursor(0, 60);
    M5.Lcd.print("IP: ");
    M5.Lcd.setCursor(40, 60);
    M5.Lcd.print(WiFi.localIP());
    delay(1000);

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

    // Set local DNS
    MDNS.begin("airmon");

    // Set server callback functions
    server.on("/", HTTP_GET, sendCommandScreen);
    server.on("/", HTTP_POST, procControl);

    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, HIGH);
    M5.Lcd.fillScreen(BLACK);
    Timer_init = millis();
}

void loop()
{
    int Timer = millis();
    static char measurementTime[12];

    M5.Lcd.setTextColor(GREEN, BLACK); // Set pixel color; 1 on the monochrome screen
    M5.Lcd.setTextSize(1);

    //Measure the time by program starting
    Timer = Timer - Timer_init;
    Minute = Timer / 60000;
    Second = Timer / 1000 - Minute * 60;
    Milisec = Timer - 1000 * Second - 60000 * Minute;

    // Substitute the time for array of measurementTime
    sprintf(measurementTime, "%02d:%02d:%03d", Minute, Second, Milisec);
    // Serial.println(measurementTime);

    float ax, ay, az, gx, gy, gz, mx, my, mz;
    float yaw, pitch, roll;
    imu->read(&ax, &ay, &az, &gx, &gy, &gz, &mx, &my, &mz, &yaw, &pitch, &roll);
    
    auto frequency = imu->getFrequency();
    displayIMU(ax, ay, az, gx, gy, gz, mx, my, mz, yaw, pitch, roll, frequency);

    uint8_t btnAPressed = 0;
    if (M5.BtnA.wasPressed())
    {
        btnAPressed = 1;
    }

    //If measurementFlag is 1, measurement is started.
    if (measurementFlag == 1)
    {
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
    }

    server.handleClient();
    M5.update();
}

/************************< Server callback functions >************************/
/*
 * Display initial screen.
 */
void sendCommandScreen()
{
    String strBuf = "<!DOCTYPE html>"
                    "<html>"
                    "<head>"
                    "<meta charset=\'utf-8\'>"
                    "<title>AirMonitor</title>"
                    "<style>body{line-height:150%;font-family:monospace;}"
                    "table.tbl{font-size:13px;color:#0;margin-left:20px;border-collapse: collapse;}table.tbl tr {border: 1px solid #888888;text-align: left;}table.tbl td {border: 1px solid #888888;text-align: left;}table.tbl tr.ttl {background-color:#e6e6fa;}div{color:red;}</style>"
                    "<script>function setCursor() { var obj = document.getElementsByName('COMMAND')[0]; obj.focus(); obj.value += ''; }"
                    "function checkForm() { var data = document.getElementsByName('COMMAND')[0]; if (data.value.trim() == '') return false; else return true; }</script>"
                    "</head>"
                    "<body onload='setCursor();'>"
                    "<form name='mainform' target='_self' method='post' onsubmit='return checkForm();'><br>"
                    "<input type='hidden' name='proc_kind' value='command' />"
                    "<table class='tbl'>"
                    "<table border='0'>"
                    "<tr>"
                    "<td width='20'>　</td>"
                    "<td width='220'>→　<input type='text' name='COMMAND' maxlength='25' value='' autofocus' /></td>"
                    "<td><input type='submit' name='SUBMIT' value='　実　行　' /></td>"
                    "</tr>"
                    "</table><br>"
                    "</form>"
                    "</body>"
                    "</html>";
    server.send(200, "text/html", strBuf);
}
/*
 * Process controll.
 */
void procControl()
{
    String procKind = server.arg("proc_kind");
    if (procKind == "command")
    {
        procAnalyzeCommand();
    }
}
/*****************************************************************************/

/*
 * Analyze command  and control execution.
 */
void procAnalyzeCommand()
{
    String cmd = server.arg("COMMAND");
    if (cmd == "")
    {
        sendCommandScreen();
        return;
    }
    Serial.print("cmd: ");
    Serial.println(cmd);
    int iMode = testProcessMode(cmd);

    switch (iMode)
    {
    case MODE_ADJUST_PARAM:
        doDisplayM5Stack();
        sendCommandScreen();
        break;
    case MODE_DO_MEASUREMENT:
        measurementFlag = 1;
        doDisplayM5Stack();
        sendCommandScreen();
        break;
        //    case MODE_RECORDS:
        //        sendDataRecord(LOG_FILE, iRecPos, iRecNo);
        //        break;
        //    case MODE_MAINTE:
        //        sendMainteScreen();
        //        break;
    case 0:
        break;
    case -1:
        sendFormatError();
        break;
    }
}

/*
 * Display Maintenance Screen
 */

int testProcessMode(String strParam)
{
    //strParam.trim();
    int iPos, iPos2;

    // Analyze the process
    int iMode = 0;
    if (strParam.toInt() > 0)
    {
        iMode = MODE_ADJUST_PARAM;
        Serial.println("Adjust the parameter");
    }
    else if (strParam.indexOf("start") != -1)
    {
        iMode = MODE_DO_MEASUREMENT;
    }
    else if (strParam.indexOf("mainte") != -1)
        iMode = MODE_MAINTE;
    else
        iMode = -1;
    return iMode;
}

/*
 * Send HTTP response <MODE missing: Input error message>
 */
void sendFormatError()
{
    String strBuf = strHeader + "　<font color='red'>パラメータの様式に誤りがあります!</font>" + strFooter;
    server.send(200, "text/html", strBuf);
}

bool isNumeric(String data)
{
    for (int i = 0; i < data.length(); i++)
    {
        if (data.charAt(i) < '0' || data.charAt(i) > '9')
            return false;
    }
    return true;
}

void doDisplayM5Stack()
{
    M5.Lcd.setCursor(0, 200);
    M5.Lcd.print("MEASUREMENT START!!");
    digitalWrite(ledPin, HIGH);
}

