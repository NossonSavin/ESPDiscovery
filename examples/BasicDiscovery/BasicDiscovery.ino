#include <WiFi.h>
#include <ESPDiscovery.h>

const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

void setup() {
    Serial.begin(115200);
    delay(1000);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi Connected! IP: " + WiFi.localIP().toString());

    // Start ESPDiscovery service
    // Parameters: Device Name, Web Port (default: 80), Firmware Version (default: "1.0.0")
    espDiscovery.begin("Bed Light Button", 80, "1.0.0");
    Serial.println("ESPDiscovery service running on UDP port 4210!");
}

void loop() {
    // If running without FreeRTOS task, call espDiscovery.handle() here:
    // espDiscovery.handle();
    delay(1000);
}
