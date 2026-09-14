# ESPDiscovery Library (v2.0)

A lightweight, zero-configuration MQTT-based network discovery library for **ESP32** (all variants: ESP32, S2, S3, C3, C6) and **ESP8266**.

## Why MQTT Discovery?
- **Universal Reach**: Works across subnets, VLANs, Wi-Fi mesh nodes, and even over the internet via cloud MQTT brokers (e.g. HiveMQ, EMQX, Mosquitto).
- **Instant Discovery**: Devices publish retained discovery messages upon connection (`<baseTopic>/devices/<name>_<mac>`), so clients immediately receive the list of all online devices upon subscribing.
- **Zero Sockets / No WDT Restarts**: Hooks directly into your existing `PubSubClient` instance without needing background FreeRTOS tasks, UDP sockets, or mDNS responders.
- **HTTP Endpoint**: Retains the `getDeviceInfoJson()` helper for your web server's `/api/identify` route.

## Installation
Add the repository to your PlatformIO `platformio.ini`:
```ini
lib_deps =
    knolleary/PubSubClient@^2.8
    https://github.com/NossonSavin/ESPDiscovery.git
```

## Quick Start
```cpp
#include <WiFi.h>
#include <PubSubClient.h>
#include <ESPDiscovery.h>

WiFiClient espClient;
PubSubClient mqttClient(espClient);

void mqttCallback(char* topic, byte* payload, unsigned int length) {
    // 1. Let ESPDiscovery handle discovery queries (e.g. esp/discover)
    if (espDiscovery.handleMqttMessage(topic, payload, length)) {
        return;
    }

    // 2. Handle your custom application topics here
}

void setup() {
    Serial.begin(115200);
    WiFi.begin("SSID", "PASSWORD");
    while (WiFi.status() != WL_CONNECTED) delay(500);

    mqttClient.setServer("broker.hivemq.com", 1883);
    mqttClient.setCallback(mqttCallback);

    // Initialize discovery with your PubSubClient instance
    espDiscovery.begin(mqttClient, "Living Room Sensor", 80, "2.0.0");
}

void reconnect() {
    while (!mqttClient.connected()) {
        if (mqttClient.connect("ESP_Client_ID")) {
            // Subscribe and publish birth announcement
            espDiscovery.onMqttConnect();
        } else {
            delay(5000);
        }
    }
}

void loop() {
    if (!mqttClient.connected()) {
        reconnect();
    }
    mqttClient.loop();
}
```

## Discovery Topics
- **Query Topic**: `esp/discover` (Publish anything to trigger all online devices to announce themselves)
- **Device Announcement**: `esp/devices/<name>_<mac>` (Payload is retained JSON)

## Device Info JSON
```json
{
  "name": "Living Room Sensor",
  "chip": "ESP32-S2",
  "ip": "192.168.1.145",
  "mac": "84:F7:03:1A:2B:3C",
  "port": 80,
  "rssi": -55,
  "version": "2.0.0",
  "uptime": 3600
}
```
