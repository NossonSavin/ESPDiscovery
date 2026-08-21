#include "ESPDiscovery.h"

ESPDiscovery espDiscovery;

ESPDiscovery::ESPDiscovery()
    : _webPort(80)
    , _udpPort(DEFAULT_DISCOVERY_PORT)
    , _running(false)
    , _usingTask(false)
#if defined(ESP32)
    , _taskHandle(nullptr)
#endif
{
    _deviceName = "ESP32 Device";
    _firmwareVersion = "1.0.0";
    _chipModel = _detectChipModel();
}

ESPDiscovery::~ESPDiscovery() {
    stop();
}

String ESPDiscovery::_detectChipModel() const {
#if defined(CONFIG_IDF_TARGET_ESP32S2)
    return "ESP32-S2";
#elif defined(CONFIG_IDF_TARGET_ESP32S3)
    return "ESP32-S3";
#elif defined(CONFIG_IDF_TARGET_ESP32C3)
    return "ESP32-C3";
#elif defined(CONFIG_IDF_TARGET_ESP32C6)
    return "ESP32-C6";
#elif defined(CONFIG_IDF_TARGET_ESP32H2)
    return "ESP32-H2";
#elif defined(ESP32)
    return "ESP32";
#elif defined(ESP8266)
    return "ESP8266";
#else
    return "Unknown ESP";
#endif
}

bool ESPDiscovery::begin(const String &deviceName, uint16_t webPort, const String &firmwareVersion, uint16_t udpPort, bool useFreeRTOSTask) {
    if (_running) {
        stop();
    }

    _deviceName = deviceName;
    _webPort = webPort;
    _firmwareVersion = firmwareVersion;
    _udpPort = udpPort;

    if (!_udp.begin(_udpPort)) {
        Serial.printf("[ESPDiscovery] Failed to bind to UDP port %u\n", _udpPort);
        return false;
    }

    _running = true;
    Serial.printf("[ESPDiscovery] Listening for discovery queries on UDP port %u (Name: '%s')\n", _udpPort, _deviceName.c_str());

#if defined(ESP32)
    if (useFreeRTOSTask) {
        _usingTask = true;
        xTaskCreate(
            _taskWorker,
            "ESPDiscoveryTask",
            3072,
            this,
            1, // Low priority background task
            &_taskHandle
        );
    } else {
        _usingTask = false;
    }
#else
    _usingTask = false;
#endif

    return true;
}

#if defined(ESP32)
void ESPDiscovery::_taskWorker(void *parameter) {
    ESPDiscovery *self = static_cast<ESPDiscovery*>(parameter);
    while (self->_running) {
        self->_processIncomingPackets();
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    vTaskDelete(nullptr);
}
#endif

void ESPDiscovery::handle() {
    if (_running && !_usingTask) {
        _processIncomingPackets();
    }
}

void ESPDiscovery::_processIncomingPackets() {
    if (!_running) return;

    int packetSize = _udp.parsePacket();
    if (packetSize <= 0) return;

    char incomingBuffer[128];
    int len = _udp.read(incomingBuffer, sizeof(incomingBuffer) - 1);
    if (len <= 0) return;
    incomingBuffer[len] = '\0';

    String packetStr = String(incomingBuffer);
    packetStr.trim();

    // Check if packet contains discovery request
    if (packetStr.startsWith("DISCOVER") || packetStr.startsWith("PING") || packetStr == DISCOVERY_QUERY_MAGIC) {
        String replyJson = getDeviceInfoJson();

        // Send reply directly back to remote requester
        _udp.beginPacket(_udp.remoteIP(), _udp.remotePort());
        _udp.write((const uint8_t*)replyJson.c_str(), replyJson.length());
        _udp.endPacket();

        Serial.printf("[ESPDiscovery] Replied to %s:%u\n", _udp.remoteIP().toString().c_str(), _udp.remotePort());
    }
}

void ESPDiscovery::stop() {
    _running = false;
#if defined(ESP32)
    if (_taskHandle != nullptr) {
        vTaskDelete(_taskHandle);
        _taskHandle = nullptr;
    }
#endif
    _udp.stop();
}

void ESPDiscovery::setDeviceName(const String &name) {
    _deviceName = name;
}

void ESPDiscovery::setFirmwareVersion(const String &version) {
    _firmwareVersion = version;
}

void ESPDiscovery::setWebPort(uint16_t port) {
    _webPort = port;
}

String ESPDiscovery::getDeviceInfoJson() const {
    IPAddress ip = WiFi.localIP();
    String mac = WiFi.macAddress();
    int32_t rssi = WiFi.RSSI();
    uint32_t uptimeSec = millis() / 1000;

    String json = "{";
    json += "\"name\":\"" + _deviceName + "\",";
    json += "\"chip\":\"" + _chipModel + "\",";
    json += "\"ip\":\"" + ip.toString() + "\",";
    json += "\"mac\":\"" + mac + "\",";
    json += "\"port\":" + String(_webPort) + ",";
    json += "\"rssi\":" + String(rssi) + ",";
    json += "\"version\":\"" + _firmwareVersion + "\",";
    json += "\"uptime\":" + String(uptimeSec);
    json += "}";

    return json;
}
