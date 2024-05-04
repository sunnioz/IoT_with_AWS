#include "BluetoothSerial.h"
#include <HardwareSerial.h>
#include <ArduinoJson.h> 
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif


#define RXp2 16
#define TXp2 17

BluetoothSerial SerialBT;
HardwareSerial SerialPort(2);
void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32test");  //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");
  SerialPort.begin(115200,SERIAL_8N1,RXp2,TXp2);
}
String message;
StaticJsonDocument<200> doc;

void loop() {
  if (SerialBT.available()) {
    message = SerialBT.readStringUntil('\n');
    Serial.println(message);
  }
  deserializeJson(doc,message);
  // doc["temperature"] = "23";
  // doc["humidity"] = "87";
  // doc["pressure"] = "1.002";
  // doc["lux"] = "23";
  // doc["location"] = "31'231'24D";
  // doc["time"] = "13:02:56";
  // doc["rain"] = "Not rain";
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); 
  // In JSON document ra Serial
  Serial.println(jsonBuffer);
  SerialPort.println((jsonBuffer));
  // SerialPort.println(jsonBuffer);
  delay(3000);
}