#ifndef ESP_DISCOVERY_H
#define ESP_DISCOVERY_H

#include <Arduino.h>

#if defined(ESP32)
  #include <WiFi.h>
  #include <WiFiUdp.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <WiFiUdp.h>
#else
  #error "ESPDiscovery only supports ESP32 and ESP8266 boards."
#endif

#define DEFAULT_DISCOVERY_PORT 4210
#define DISCOVERY_QUERY_MAGIC "DISCOVER_ESP32"

class ESPDiscovery {
public:
    ESPDiscovery();
    ~ESPDiscovery();

    // Start listening on UDP broadcast
    // If useFreeRTOSTask is true (ESP32 only), spawns a lightweight background task
    bool begin(const String &deviceName, uint16_t webPort = 80, const String &firmwareVersion = "1.0.0", uint16_t udpPort = DEFAULT_DISCOVERY_PORT, bool useFreeRTOSTask = true);
    
    // Call in loop() if useFreeRTOSTask is false
    void handle();

    // Stop discovery service
    void stop();

    // Update runtime metadata
    void setDeviceName(const String &name);
    void setFirmwareVersion(const String &version);
    void setWebPort(uint16_t port);

    // Get JSON payload containing device info
    String getDeviceInfoJson() const;

private:
    WiFiUDP _udp;
    String _deviceName;
    String _firmwareVersion;
    String _chipModel;
    uint16_t _webPort;
    uint16_t _udpPort;
    bool _running;
    bool _usingTask;

#if defined(ESP32)
    TaskHandle_t _taskHandle;
    static void _taskWorker(void *parameter);
#endif

    void _processIncomingPackets();
    String _detectChipModel() const;
};

extern ESPDiscovery espDiscovery;

#endif // ESP_DISCOVERY_H
