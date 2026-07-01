#include <Arduino.h>
#include "config.h"
#include "logger.h"
#include "wifi_manager.h"
#include "sensor_manager.h"
#include "sensor_sync.h"
#include "api_client.h"
#include "web_server.h"
#include <LittleFS.h>

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("\n=== THERMO-8266 " FW_VERSION " ===");

    // 1. LittleFS
    if (!LittleFS.begin()) {
        Serial.println("[MAIN] LittleFS failed — formatting...");
        LittleFS.format();
        LittleFS.begin();
    }

    // 2. Logger
    logInit();
    LOG(LOG_INFO, "MAIN", "Boot start, fw: %s", FW_VERSION);

    // 3. Konfigurace z LittleFS
    configLoad();
    LOG(LOG_INFO, "MAIN", "Config loaded: name=%s", unitName.c_str());

    // 4. WiFi
    wifiInit();

    // 5. Senzory
    sensorInit();

    // 6. API klient
    apiClientInit();

    // 7. Sync názvů
    sensorSyncInit();

    // 8. Web server
    webServerInit();

    LOG(LOG_INFO, "MAIN", "Boot complete");
}

void loop() {
    wifiLoop();
    sensorLoop();
    sensorSyncLoop();
    apiClientLoop();
    webServerLoop();

    // ESP8266 potřebuje yield pro WiFi stack
    yield();
}