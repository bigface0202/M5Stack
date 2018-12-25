#include <WiFi.h>
#include <ESPmDNS.h>
#include <NTPClient.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WebServer.h>
#include <M5Stack.h>

//Test
int ledPin = 2;

// Process mode
#define MODE_CURRENT 1 // Response current values
#define MODE_RECORDS 2 // Send records from SD limited the range of record position
#define MODE_DATE 3    // Send records from SD limited the range of date or date-time
#define MODE_MAINTE 11 // Create maintenance screen

// WiFi connection
char ssid[33] = {"Human-A1-711"};
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
    Serial.begin(115200);
    //Start M5Stack OS
    M5.begin();
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(WHITE, BLACK); // Set pixel color; 1 on the monochrome screen
    M5.Lcd.setTextSize(2);

    //IMU start!!
    //    imu = new IMU();

    // Prepare I2C protocol.
    Wire.begin(21, 22); //Define(SDA, SCL)
    delay(50);

    //    displayWelcome();
    //    delay(1000);
    //
    //    displayAdress("MPU9250", imu->getAddress(), 0x71);
    //    delay(1000);
    //    displayAdress("AK8963", imu->getAK8963Adress(), 0x48);
    //    delay(1000);
    //    displayAccelGyroBias(imu->getAccelBias(), imu->getGyroBias());
    //    delay(1000);
    //    displayMagCalibration(imu->getMagCalibration());
    //    delay(1000);

    // Prepare WiFi system.
    WiFi.disconnect(true);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
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

    // Set local DNS
    MDNS.begin("airmon");

    // Set server callback functions
    server.on("/", HTTP_GET, sendCommandScreen);
    server.on("/", HTTP_POST, procControl);

    pinMode(ledPin, OUTPUT);
    digitalWrite(ledPin, HIGH);
}

void loop()
{
    server.handleClient();
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

    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 40);
    M5.Lcd.print(cmd);
    int iMode = testProcessMode(cmd);
    if (cmd == "100")
    {
        digitalWrite(ledPin, LOW);
    }

    switch (iMode)
    {
    case MODE_CURRENT:
        //        doMeasurement();
        //        sendMeasuredResult(rstMeasured);
        doDisplayM5Stack();
        sendCommandScreen();
        break;
        //    case MODE_DATE:
        //        sendDataRecord(LOG_FILE, strFromDate, strToDate);
        //        break;
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
    if (strParam.indexOf("now") != -1)
    {
        iMode = MODE_CURRENT;
        Serial.println("Mode: Current data.");
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setCursor(0, 0);
        M5.Lcd.print("Mode: Current data.");
    }
    else if ((iPos = strParam.indexOf("date=")) != -1)
    {
        iMode = MODE_DATE;
        strParam = strParam.substring(iPos);
        Serial.print("Param: ");
        Serial.println(strParam);
        if ((iPos2 = strParam.indexOf(",")) == -1)
        {
            strFromDate = strParam.substring(5);
            strToDate = strFromDate;
        }
        else
        {
            strFromDate = strParam.substring(5, iPos2);
            strToDate = strParam.substring(iPos2 + 1);
        }
        Serial.print(" From:");
        Serial.print(strFromDate);
        Serial.print(", To:");
        Serial.println(strToDate);

        M5.Lcd.setCursor(0, 20);
        M5.Lcd.print("From;");
        M5.Lcd.setCursor(40, 20);
        M5.Lcd.print(strFromDate);
        M5.Lcd.setCursor(0, 40);
        M5.Lcd.print("To;");
        M5.Lcd.setCursor(40, 40);
        M5.Lcd.print(strToDate);

        if (!(strFromDate.length() == 8 && strToDate.length() == 8))
            return -1;
        if (!(strFromDate.charAt(2) == '/' && strFromDate.charAt(5) == '/' && strToDate.charAt(2) == '/' && strToDate.charAt(5) == '/'))
            return -1;
        if (!(isNumeric(strFromDate.substring(0, 2)) && isNumeric(strFromDate.substring(3, 5)) && isNumeric(strFromDate.substring(6, 8)) && isNumeric(strToDate.substring(0, 2)) && isNumeric(strToDate.substring(3, 5)) && isNumeric(strToDate.substring(6, 8))))
            return -1;
        strFromDate = "20" + strFromDate;
        strToDate = "20" + strToDate;
    }
    else if ((iPos = strParam.indexOf("rec=")) != -1)
    {
        iMode = MODE_RECORDS;
        strParam = strParam.substring(iPos);
        Serial.print("Param: ");
        Serial.println(strParam);
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.setCursor(0, 0);
        M5.Lcd.print("Param: ");
        M5.Lcd.setCursor(0, 20);
        M5.Lcd.print(strParam);

        if ((iPos2 = strParam.indexOf(",")) == -1)
        {
            if (!(isNumeric(strParam.substring(4)) && strParam.substring(4).length() > 0))
            {
                return -1;
            }
            iRecPos = (strParam.substring(4)).toInt();
            iRecNo = 10000;
        }
        else
        {
            if (!(isNumeric(strParam.substring(iPos + 4, iPos2)) && isNumeric(strParam.substring(iPos2 + 1)) && strParam.substring(iPos + 4, iPos2).length() > 0 && strParam.substring(iPos2 + 1).length() > 0))
            {
                return -1;
            }
            iRecPos = (strParam.substring(iPos + 4, iPos2)).toInt();
            iRecNo = (strParam.substring(iPos2 + 1)).toInt();
        }
        if (iRecPos < 1)
            iRecPos = 1;
        if (iRecNo > 10000)
            iRecNo = 10000;
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
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.print("HELLO WORLD!!");
    digitalWrite(ledPin, HIGH);
}

