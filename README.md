# ESPDiscovery Library

A lightweight, zero-configuration local network discovery library for **ESP32** (all variants: ESP32, S2, S3, C3, C6) and **ESP8266**.

## Why use this instead of mDNS?
- **mDNS is often unreliable on Windows** due to packet filtering, router AP client isolation, or multicast drop on Wi-Fi.
- `ESPDiscovery` uses **direct UDP broadcast (port 4210)** which works instantly (< 50ms) across local subnets.
- Runs as an autonomous background FreeRTOS task on ESP32 without blocking your main loop.

## How to Install in PlatformIO
Add the library folder to your project's `lib/` directory or include the source files in `src/`.

## How to Install in Arduino IDE
Copy the `ESPDiscovery` folder to your `Documents/Arduino/libraries/` folder.

## Quick Start
```cpp
#include <WiFi.h>
#include <ESPDiscovery.h>

void setup() {
    WiFi.begin("SSID", "PASSWORD");
    while (WiFi.status() != WL_CONNECTED) delay(500);

    // Start discovery (Name, Web Port, Version)
    espDiscovery.begin("Bed Light Button", 80, "1.0.0");
}

void loop() {
    // Nothing needed! ESPDiscovery runs in the background on ESP32.
}
```

## Discovery Response Format
When queried by the Windows Discovery tool, the ESP32 automatically responds with:
```json
{
  "name": "Bed Light Button",
  "chip": "ESP32-S2",
  "ip": "192.168.1.145",
  "mac": "84:F7:03:1A:2B:3C",
  "port": 80,
  "rssi": -55,
  "version": "1.0.0",
  "uptime": 3600
}
```
