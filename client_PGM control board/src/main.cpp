/*Client側*/
#include <WiFi.h>
const char *ssid = "ESP32-WiFi"; //SSID of server
const char *pass = "esp32wifi";  //PW of server
static WiFiClient client;        //Declear client

int rightPGM = 26;
int leftPGM = 34;
int frontPGM = 32;
int backPGM = 25;
int startMeasurement = 33;
int connectedWiFiLED = 4;
int measurementStartLED = 2;

int rightPGM_now, leftPGM_now, frontPGM_now, backPGM_now, startMeasurement_now;
int rightPGM_past = 0, leftPGM_past = 0, frontPGM_past = 0, backPGM_past = 0, startMeasurement_past = 0;

void setup()
{
  Serial.begin(115200);
  WiFi.begin(ssid, pass); //Connect to server
  Serial.printf("\n");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(1000);
  }
  Serial.printf("\n");
  Serial.println("WiFi Connected");
  IPAddress ip(192, 168, 0, 9); //IP address of server
  client.connect(ip, 80);       //Connect to IP address and port number

  pinMode(rightPGM, INPUT);
  pinMode(leftPGM, INPUT);
  pinMode(frontPGM, INPUT);
  pinMode(backPGM, INPUT);
  pinMode(startMeasurement, INPUT);
  pinMode(measurementStartLED, OUTPUT);
  pinMode(connectedWiFiLED, OUTPUT);
}

void loop()
{
  if (client.connected() == true)
  {
    digitalWrite(connectedWiFiLED, HIGH);
    rightPGM_now = digitalRead(rightPGM);
    leftPGM_now = digitalRead(leftPGM);
    frontPGM_now = digitalRead(frontPGM);
    backPGM_now = digitalRead(backPGM);
    startMeasurement_now = digitalRead(startMeasurement);
    //Turn on the right PGM
    if (rightPGM_past != rightPGM_now)
    {
      if (rightPGM_now == HIGH)
      {
        client.write('1');
      }
      rightPGM_past = rightPGM_now;
    }
    //Turn on the left PGM
    if (leftPGM_past != leftPGM_now)
    {
      if (leftPGM_now == HIGH)
      {
        client.write('2');
      }
      leftPGM_past = leftPGM_now;
    }
    //Turn on the front PGM
    if (frontPGM_past != frontPGM_now)
    {
      if (frontPGM_now == HIGH)
      {
        client.write('3');
      }
      frontPGM_past = frontPGM_now;
    }
    //Turn on the back PGM
    if (backPGM_past != backPGM_now)
    {
      if (backPGM_now == HIGH)
      {
        client.write('4');
      }
      backPGM_past = backPGM_now;
    }
    //Start the IMU measurement
    if (startMeasurement_past != startMeasurement_now)
    {
      if (startMeasurement_now == HIGH)
      {
        client.write('5');
        digitalWrite(measurementStartLED, HIGH);
      }
      startMeasurement_past = startMeasurement_now;
    }
  }
}