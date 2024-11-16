#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "SensorLDR.h"
#include "RelayControl.h"
#include "WiFiManager.h"
#define RELAY_PIN 4
#define LDR_PIN 5


const char* WIFI_SSID = "HUAWEI nova 8i";
const char* WIFI_PASS = "LoreMKC072";
const char* MQTT_BROKER = "a1j7buqf58whny-ats.iot.us-east-1.amazonaws.com";
const int MQTT_PORT = 8883;
const char* CLIENT_ID = "Relay";
const char* UPDATE_TOPIC = "$aws/things/Relay/shadow/update";
const char* UPDATE_DELTA_TOPIC = "$aws/things/Relay/shadow/update/delta";
WiFiClientSecure wiFiClient;
PubSubClient client(wiFiClient);
StaticJsonDocument<JSON_OBJECT_SIZE(64)> inputDoc;
StaticJsonDocument<JSON_OBJECT_SIZE(4)> outputDoc;
char outputBuffer[128];
SensorLDR ldr(LDR_PIN);
RelayControl relay(RELAY_PIN);
byte builtInLed = 0;

void reportBuiltInLed() {
  outputDoc["state"]["reported"]["builtInLed"] = builtInLed;
  serializeJson(outputDoc, outputBuffer);
  client.publish(UPDATE_TOPIC, outputBuffer);
}

void setBuiltInLed() {
  if (builtInLed == 1) {
    if (ldr.isDark()) {
      relay.turnOn();
      Serial.println("Oscuridad detectada. Relé encendido.");
    } else {
      relay.turnOff();
      Serial.println("Hay luz. Relé apagado.");
    }
  } else {
    relay.turnOff();
    Serial.println("builtInLed está apagado. Relé apagado.");
  }
  reportBuiltInLed();
  delay(2000);
}


void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) message += String((char) payload[i]);
  Serial.println("Mensaje desde el tópico " + String(topic) + ": " + message);
  
  DeserializationError err = deserializeJson(inputDoc, payload);
  if (!err) {
    if (String(topic) == UPDATE_DELTA_TOPIC) {
      builtInLed = inputDoc["state"]["builtInLed"].as<int8_t>();
      Serial.print("Nuevo valor de builtInLed: ");
      Serial.println(builtInLed);
      setBuiltInLed();
    }
  }
}


void setup() {
  Serial.begin(115200);
  delay(100);
  ldr.begin();
  relay.begin();
  setupWiFi();
  wiFiClient.setCACert(AMAZON_ROOT_CA1);
  wiFiClient.setCertificate(CERTIFICATE);
  wiFiClient.setPrivateKey(PRIVATE_KEY);
  client.setServer(MQTT_BROKER, MQTT_PORT);
  client.setCallback(callback);
}

void setupWiFi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando a ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Conectado a WiFi. Dirección IP: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {  
  while (!client.connected()) {
    Serial.print("Intentando conexión MQTT...");
    if (client.connect(CLIENT_ID)) {
      Serial.println("Conectado");
      client.subscribe(UPDATE_DELTA_TOPIC);
      Serial.println("Suscrito a " + String(UPDATE_DELTA_TOPIC));
      delay(100);
      reportBuiltInLed();
    } else {
      Serial.print("Fallido, rc=");
      Serial.print(client.state());
      Serial.println(" intentando de nuevo en 5 segundos");
      delay(5000);
    }
  }
}


void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  if (builtInLed == 1) {
    setBuiltInLed();
    delay(1000);
  }
}