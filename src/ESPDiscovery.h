#ifndef ESP_DISCOVERY_H
#define ESP_DISCOVERY_H

#include <Arduino.h>
#include <PubSubClient.h>

#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif

class ESPDiscovery {
public:
    ESPDiscovery();
    ~ESPDiscovery();

    // Initialize discovery with PubSubClient instance and device metadata
    bool begin(PubSubClient &client, const String &deviceName, uint16_t webPort = 80, const String &firmwareVersion = "2.0.0", const char *baseTopic = "esp");

    // Call inside your MQTT callback(char* topic, byte* payload, unsigned int length)
    // Returns true if the message was a discovery request and handled
    bool handleMqttMessage(const char *topic, const uint8_t *payload, unsigned int length);

    // Call immediately after mqttClient.connect() succeeds to subscribe and publish birth message
    void onMqttConnect();

    // Publish device info JSON to MQTT (<baseTopic>/devices/<sanitized_name_or_mac>)
    void publishDiscovery(bool retained = true);

    // Update runtime metadata
    void setDeviceName(const String &name);
    void setFirmwareVersion(const String &version);
    void setWebPort(uint16_t port);

    // Get JSON payload containing device info (used for MQTT payload and HTTP /api/identify)
    String getDeviceInfoJson() const;

    // Get the MQTT topic used for device announcements
    String getDeviceTopic() const;

    // Get the MQTT query topic subscribed to
    String getQueryTopic() const;

private:
    PubSubClient *_client;
    String _deviceName;
    String _firmwareVersion;
    String _chipModel;
    String _baseTopic;
    uint16_t _webPort;

    String _detectChipModel() const;
    String _sanitizeTopic(const String &str) const;
};

extern ESPDiscovery espDiscovery;

#endif // ESP_DISCOVERY_H
