/* WiFi-Control-Car */

#include <WiFi.h>
const char *ssid = "BSYS-A1-512-HUMAN"; //アクセスポイントのSSID
const char *password = "bsys12bsys34";  //アクセスポイントのパスワード
const char html[] =
    "<!DOCTYPE html><html lang='ja'><head><meta charset='UTF-8'>\
<style>input {margin:8px;width:80px;}\
div {font-size:16pt;color:red;text-align:center;width:400px;border:groove 40px orange;}</style>\
<title>PGM Controller</title></head>\
<body><div><p>PGM Controller</p>\
<form method='get'>\
<input type='submit' name='fl' value='左前' />\
<input type='submit' name='front' value='前' />\
<input type='submit' name='fr' value='右前' /><br>\
<input type='submit' name='st' value='停止' /><br>\
<input type='submit' name='bl' value='左後' />\
<input type='submit' name='back' value='後ろ' />\
<input type='submit' name='br' value='右後' /><br><br>\
</form></div></body></html>";

int PGM_FR = 25;
int PGM_FL = 26;
int PGM_BR = 32;
int PGM_BL = 33;
int LEDpin = 27;

void initPGM()
{
  digitalWrite(PGM_FR, LOW);
  digitalWrite(PGM_FL, LOW);
  digitalWrite(PGM_BR, LOW);
  digitalWrite(PGM_BL, LOW);
}

void actuateFrontPGM()
{
  digitalWrite(PGM_FR, HIGH);
  digitalWrite(PGM_FL, HIGH);
}

void actuateBackPGM()
{
  digitalWrite(PGM_BR, HIGH);
  digitalWrite(PGM_BL, HIGH);
}

WiFiServer server(80);
void setup()
{
  Serial.begin(115200);
  pinMode(PGM_FR, OUTPUT);
  pinMode(PGM_FL, OUTPUT);
  pinMode(PGM_BR, OUTPUT);
  pinMode(PGM_BL, OUTPUT);
  pinMode(LEDpin, OUTPUT);
  delay(10);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  server.begin();
  initPGM();
}
void loop()
{
  WiFiClient client = server.available();
  if (client)
  {
    Serial.println("New Client.");
    String currentLine = "";
    while (client.connected())
    {
      digitalWrite(LEDpin, HIGH);
      if (client.available())
      {
        char c = client.read();
        Serial.write(c);
        if (c == '\n')
        {
          if (currentLine.length() == 0)
          {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            client.print(html);
            client.println();
            break;
          }
          else
          {
            currentLine = "";
          }
        }
        else if (c != '\r')
        {
          currentLine += c;
        }

        if (currentLine.endsWith("GET /?fr"))
        {
          digitalWrite(PGM_FR, HIGH);
          delay(1000);
          initPGM();
        }
        if (currentLine.endsWith("GET /?fl"))
        {
          digitalWrite(PGM_FL, HIGH);
          delay(1000);
          initPGM();
        }
        if (currentLine.endsWith("GET /?br"))
        {
          digitalWrite(PGM_BR, HIGH);
          delay(1000);
          initPGM();
        }
        if (currentLine.endsWith("GET /?bl"))
        {
          digitalWrite(PGM_BL, HIGH);
          delay(1000);
          initPGM();
        }
        if (currentLine.endsWith("GET /?front"))
        {
          actuateFrontPGM();
          delay(1000);
          initPGM();
        }
        if (currentLine.endsWith("GET /?back"))
        {
          actuateBackPGM();
          delay(1000);
          initPGM();
        }
      }
    }
    // client.stop();
    // Serial.println("Client Disconnected.");
  }
}