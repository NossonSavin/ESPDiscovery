#include <WiFi.h>
#include <PubSubClient.h>
#include <ESPDiscovery.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* mqtt_server = "broker.hivemq.com";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    // Check if message is an ESPDiscovery query
    if (espDiscovery.handleMqttMessage(topic, payload, length)) {
        return;
    }

    // Handle other topics here
}

void reconnect() {
    while (!mqttClient.connected()) {
        Serial.print("Attempting MQTT connection...");
        if (mqttClient.connect("ESP32_Device_1")) {
            Serial.println("connected");
            // Automatically subscribe to esp/discover and announce device
            espDiscovery.onMqttConnect();
        } else {
            Serial.print("failed, rc=");
            Serial.print(mqttClient.state());
            Serial.println(" retrying in 5 seconds");
            delay(5000);
        }
    }
}

void setup() {
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected");

    mqttClient.setServer(mqtt_server, 1883);
    mqttClient.setCallback(mqttCallback);

    // Initialize ESPDiscovery with MQTT client, Device Name, Web Port, and Version
    espDiscovery.begin(mqttClient, "Living Room Device", 80, "2.0.0");
}

void loop() {
    if (!mqttClient.connected()) {
        reconnect();
    }
    mqttClient.loop();
}
