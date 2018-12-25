#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
char *ssid = "Your_SSID";
char *password = "Your_Password";
String url = "";
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  Serial.print("Connecting to \'");
  Serial.print(ssid);
  Serial.println("\'");
  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected...");

  Serial.println();
  Serial.println("* AP information");
  Serial.print("SSID : "); Serial.println(WiFi.SSID());
  Serial.print("RSSI : "); Serial.println(WiFi.RSSI());
  Serial.print("GateWay IP : "); Serial.println(WiFi.gatewayIP());

  Serial.println();
  Serial.println("* station information");
  Serial.print("IP Adress : "); Serial.println(WiFi.localIP());
  Serial.print("MAC Adress : "); Serial.println(WiFi.macAddress());
}
void loop() {

  HTTPClient httpClient;
  while(Serial.available())
  {
    char myChar = (char)Serial.read();
    url += myChar;
    delay(5);
  }
  Serial.println(url);
  httpClient.begin(url);
  int httpCode = httpClient.GET();
  if(httpCode > 0)
    Serial.println("Success");
  else
  {
    Serial.println("Failed");
  }
  if(!url.equals(""))
  {
    url = "";
  }
  delay(15000);
}
