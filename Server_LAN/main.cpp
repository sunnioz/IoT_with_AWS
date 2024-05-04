#include <Arduino.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Wire.h>
#include <HardwareSerial.h>
#include <ArduinoJson.h>

const char *ssid = "Tjn";
const char *password = "30122003";

HardwareSerial Serial_Uart(0);

AsyncWebServer server(80);
char *messages;

StaticJsonDocument<200> doc;

String readTemperature()
{
  String temperature = doc["temperature"].as<String>();
  return temperature.substring(0, 5);
}
String readHumidity()
{
  String humidity = doc["humidity"].as<String>();
  return humidity.substring(0, 5);
}
String readPressure()
{
  String pressure = doc["pressure"].as<String>();
  return pressure.substring(0, 7);
}
String readLux()
{
  String lux = doc["lux"].as<String>();
  return lux.substring(0, 6);
}
String readLocation()
{
  return doc["location"].as<String>();
}
String readTime()
{
  return doc["timeline"].as<String>();
}
String readRain()
{
  return doc["rain"].as<String>();
}
String readGas()
{
  String gas = doc["gas"].as<String>();
  return gas.substring(11, 16);
}

void setup()
{
  Serial.begin(115200);
  if (!SPIFFS.begin(true))
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  Serial_Uart.begin(115200, SERIAL_8N1, 3, 1);
  deserializeJson(doc, messages);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println(WiFi.localIP());
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/index.html"); });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", readTemperature().c_str()); });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", readHumidity().c_str()); });
  server.on("/pressure", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", readPressure().c_str()); });
  server.on("/lux", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", readLux().c_str()); });
  server.on("/location", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", readLocation().c_str()); });
  server.on("/timeline", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", readTime().c_str()); });
  server.on("/rain", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", readRain().c_str()); });
  server.on("/gas", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send_P(200, "text/plain", readGas().c_str()); });
  server.begin();
}
void loop()
{
  if (Serial_Uart.available())
  {
    String temp = Serial_Uart.readStringUntil('\n');
    messages = new char[temp.length() + 1];
    strcpy(messages, temp.c_str());
    deserializeJson(doc, messages);
  }
  Serial.println(readTemperature());
  Serial.println(messages);
  delay(3000);
}
