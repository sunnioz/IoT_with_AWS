#include "BluetoothSerial.h"
#include <Adafruit_BME280.h>
#include <BH1750.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <ArduinoJson.h> 

#define RAIN_SENSOR_PIN 2
#define ADC_PIN 4
#define GPS_TX_PIN 12
#define GPS_RX_PIN 13
#define SEALEVELPRESSURE_HPA (1013.25)

BluetoothSerial SerialBT;
HardwareSerial GPSSerial(2); // Khởi tạo đối tượng SoftwareSerial cho GPS
TinyGPSPlus gps; // Khởi tạo đối tượng TinyGPS++
String locationString;
String timeString;
String MACadd = "B8:D6:1A:47:DE:0E";                          //Write Drone side MAC address
uint8_t address[6] = { 0xB8, 0xD6, 0x1A, 0x47, 0xDE, 0x0E };  //Write Drone side MAC address in HEX
bool connected;

Adafruit_BME280 bme;
BH1750 lightMeter;

String readMQ2Value() {
  int mq2Value = analogRead(ADC_PIN);
  return String(mq2Value);
}

String readTemperature() {
  float temperature = bme.readTemperature();
  return String(temperature) + " *C";
}

String readHumidity() {
  float humidity = bme.readHumidity();
  return String(humidity) + " %";
}

String readPressure() {
  float pressure = bme.readPressure() / 100.0F;
  return String(pressure) + " hPa";
}

String readLux() {
  float lux = lightMeter.readLightLevel();
  return String(lux) + " lux";
}
String readLocation(double latitude, double longitude) {
  String locationString = "Location: " + String(latitude, 6) + ", " + String(longitude, 6);
  return locationString;
}

String readTime(const TinyGPSDate &date, const TinyGPSTime &time) {
  TinyGPSDate localDate = date;
  TinyGPSTime localTime = time;

  int hour = localTime.hour() + 7;
  int day = localDate.day();
  int month = localDate.month();
  int year = localDate.year();

  if (hour >= 24) {
    hour -= 24;
    day++;
  }

  if (day > 31) { 
    day = 1;
    month++;
  }
  if (month > 12) {
    month = 1;
    year++;
  }

  hour %= 24;

  String timeString = "Time: " + String(day) + "-" + String(month) + "-" + String(year) + " " + String(hour) + ":" + String(localTime.minute()) + ":" + String(localTime.second());
  
  return timeString;
}
String readRainStatus() {
  int rainValue = digitalRead(RAIN_SENSOR_PIN);
  if (rainValue == LOW) {
    return "Trời không mưa";
  } else {
    return "Trời đang mưa";
  }
}

StaticJsonDocument<200> doc;

void setup() {
  GPSSerial.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);
  Serial.begin(115200);
  SerialBT.begin("ESP32test", true);
  Serial.println("The device started in master mode, make sure remote BT device is on!");
  if (!bme.begin(0x76)) {  // Địa chỉ I2C của BME280 có thể là 0x76 hoặc 0x77, tùy thuộc vào kết nối của bạn
    Serial.println("Không thể kết nối với cảm biến BME280!");
    while (1);
  }
    if (!lightMeter.begin()) {
    Serial.println("Không thể kết nối với cảm biến BH1750!");
    while (1);
  }

  lightMeter.configure(BH1750::CONTINUOUS_HIGH_RES_MODE);


  // connect(address) is fast (upto 10 secs max), connect(name) is slow (upto 30 secs max) as it needs
  // to resolve name to address first, but it allows to connect to different devices with the same name.
  // Set CoreDebugLevel to Info to view devices bluetooth address and device names
  connected = SerialBT.connect(address);

  if (connected) {
    Serial.println("Connected Succesfully!");
  } else {
    while (!SerialBT.connected(10000)) {
      Serial.println("Failed to connect. Make sure remote device is available and in range, then restart app.");
    }
  }
  // disconnect() may take upto 10 secs max
  if (SerialBT.disconnect()) {
    Serial.println("Disconnected Succesfully!");
  }
  // this would reconnect to the name(will use address, if resolved) or address used with connect(name/address).
  SerialBT.connect();
}
void loop() {
  while (GPSSerial.available() > 0) {
    gps.encode(GPSSerial.read());
  }
  String locationString = readLocation(gps.location.lat(), gps.location.lng());
  String timeString = readTime(gps.date, gps.time);


  doc["temperature"] = readTemperature();
  doc["humidity"] = readHumidity();
  doc["pressure"] = readPressure();
  doc["lux"] = readLux();
  doc["location"] = locationString;
  doc["time"] = timeString ;
  doc["rain"] = readRainStatus();
  doc["gas"] = readMQ2Value();
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); 

  SerialBT.println(jsonBuffer);
  Serial.println(jsonBuffer);

  delay(1000);
}