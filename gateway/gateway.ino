#include "secrets.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"
#include <HardwareSerial.h>


#define RXp2 16
#define TXp2 17
#define AWS_IOT_PUBLISH_TOPIC   "esp32/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"


long now;
long previous_now;
HardwareSerial SerialPort(2);

WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

void connectAWS()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
 
  Serial.println("Connecting to Wi-Fi");
 
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
 
  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);
 
  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.setServer(AWS_IOT_ENDPOINT, 8883);
 
  // Create a message handler
  client.setCallback(messageHandler);
 
  Serial.println("Connecting to AWS IOT");
 
  while (!client.connect(THINGNAME))
  {
    Serial.print(".");
    delay(100);
  }
 
  if (!client.connected())
  {
    Serial.println("AWS IoT Timeout!");
    return;
  }
 
  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
 
  Serial.println("AWS IoT Connected!");
}
String message;
void publishMessage()
{
  client.publish(AWS_IOT_PUBLISH_TOPIC,message.c_str());
}
void messageHandler(char* topic, byte* payload, unsigned int length)
{
  Serial.print("incoming: ");
  Serial.println(topic);
  String received_msg = "";
  for (int i = 0; i < length; i++) 
  {
    received_msg += (char)payload[i];
    Serial.print((char)payload[i]);
  }
  Serial.println();
 
  // StaticJsonDocument<200> doc;
  // deserializeJson(doc, payload);
  // const char* message = doc["message"];
  // Serial.println(message);
}
 
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  SerialPort.begin(115200,SERIAL_8N1,RXp2,TXp2);
  connectAWS();
  previous_now = 0;
}

void loop() {
  if(SerialPort.available()){
      message = SerialPort.readStringUntil('\n');
      Serial.println(message);
  }

  now = millis();
  if(now - previous_now > 1000 * 10){
    previous_now = now;
    publishMessage();
  }
  client.loop();
}
