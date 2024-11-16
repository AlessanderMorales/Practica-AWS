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
WiFiManager wifiManager(WIFI_SSID, WIFI_PASS);
SensorLDR ldr(LDR_PIN);
RelayControl relay(RELAY_PIN);

byte LDRSensor = 0;
StaticJsonDocument<JSON_OBJECT_SIZE(64)> inputDoc;
StaticJsonDocument<JSON_OBJECT_SIZE(4)> outputDoc;
char outputBuffer[128];

void reportLDRSensor() {
    outputDoc["state"]["reported"]["LDRSensor"] = LDRSensor;
    serializeJson(outputDoc, outputBuffer);
    client.publish(UPDATE_TOPIC, outputBuffer);
}

void setLDRSensor() {
    if (LDRSensor == 1) {
        if (ldr.isDark()) {
            relay.turnOn();
            Serial.println("Oscuridad detectada. Rele encendido.");
        } else {
            relay.turnOff();
            Serial.println("Hay luz. Rele apagado.");
        }
    } else {
        relay.turnOff();
        Serial.println("LDRSensor esta apagado. Rele apagado.");
    }
    reportLDRSensor();
    delay(2000);
}

void callback(char* topic, byte* payload, unsigned int length) {
    String message;
    for (int i = 0; i < length; i++) message += String((char) payload[i]);
    Serial.println("Mensaje desde el tópico " + String(topic) + ": " + message);

    DeserializationError err = deserializeJson(inputDoc, payload);
    if (!err) {
        if (String(topic) == UPDATE_DELTA_TOPIC) {
            LDRSensor = inputDoc["state"]["LDRSensor"].as<int8_t>();
            Serial.print("Nuevo valor de LDRSensor: ");
            Serial.println(LDRSensor);
            setLDRSensor();
        }
    }
}

void reconnect() {
    while (!client.connected()) {
        Serial.print("Intentando conexión MQTT...");
        if (client.connect(CLIENT_ID)) {
            Serial.println("Conectado");
            client.subscribe(UPDATE_DELTA_TOPIC);
            Serial.println("Suscrito a " + String(UPDATE_DELTA_TOPIC));
            delay(100);
            reportLDRSensor();
        } else {
            Serial.print("Fallido, rc=");
            Serial.print(client.state());
            Serial.println(" intentando de nuevo en 5 segundos");
            delay(5000);
        }
    }
}

void setup() {
    Serial.begin(115200);
    delay(100);
    ldr.begin();
    relay.begin();
    wifiManager.connect();
    wiFiClient.setCACert(AMAZON_ROOT_CA1);
    wiFiClient.setCertificate(CERTIFICATE);
    wiFiClient.setPrivateKey(PRIVATE_KEY);

    client.setServer(MQTT_BROKER, MQTT_PORT);
    client.setCallback(callback);
}

void loop() {
    if (!wifiManager.isConnected()) {
        wifiManager.connect();
    }

    if (!client.connected()) {
        reconnect();
    }

    client.loop();

    if (LDRSensor == 1) {
        setLDRSensor();
        delay(1000);
    }
}
