#include "ESPDiscovery.h"

ESPDiscovery espDiscovery;

ESPDiscovery::ESPDiscovery()
    : _client(nullptr)
    , _webPort(80)
    , _baseTopic("esp")
{
    _deviceName = "ESP Device";
    _firmwareVersion = "2.0.0";
    _chipModel = _detectChipModel();
}

ESPDiscovery::~ESPDiscovery() {
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

String ESPDiscovery::_sanitizeTopic(const String &str) const {
    String clean = "";
    for (size_t i = 0; i < str.length(); i++) {
        char c = str[i];
        if (isalnum(c) || c == '-' || c == '_') {
            clean += c;
        } else if (c == ' ') {
            clean += '_';
        }
    }
    return clean.length() > 0 ? clean : "esp_device";
}

bool ESPDiscovery::begin(PubSubClient &client, const String &deviceName, uint16_t webPort, const String &firmwareVersion, const char *baseTopic) {
    _client = &client;
    _deviceName = deviceName;
    _webPort = webPort;
    _firmwareVersion = firmwareVersion;
    if (baseTopic && strlen(baseTopic) > 0) {
        _baseTopic = baseTopic;
    }
    return true;
}

String ESPDiscovery::getQueryTopic() const {
    return _baseTopic + "/discover";
}

String ESPDiscovery::getDeviceTopic() const {
    String mac = WiFi.macAddress();
    mac.replace(":", "");
    mac.toLowerCase();
    return _baseTopic + "/devices/" + _sanitizeTopic(_deviceName) + "_" + mac;
}

void ESPDiscovery::onMqttConnect() {
    if (!_client || !_client->connected()) return;

    // 1. Subscribe to discovery query topic
    String queryTopic = getQueryTopic();
    _client->subscribe(queryTopic.c_str());

    // 2. Publish retained device information (birth announcement)
    publishDiscovery(true);
}

bool ESPDiscovery::handleMqttMessage(const char *topic, const uint8_t *payload, unsigned int length) {
    if (!topic) return false;

    String queryTopic = getQueryTopic();
    if (String(topic) == queryTopic) {
        // Query received on <baseTopic>/discover -> reply with fresh device info
        publishDiscovery(false);
        return true;
    }

    return false;
}

void ESPDiscovery::publishDiscovery(bool retained) {
    if (!_client || !_client->connected()) return;

    String topic = getDeviceTopic();
    String json = getDeviceInfoJson();
    _client->publish(topic.c_str(), json.c_str(), retained);
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
