#include <Arduino.h>
#include "config.h"
#include "logger.h"
#include "wifi_manager.h"
#include "sensor_manager.h"
#include "sensor_sync.h"
#include "api_client.h"
#include "web_server.h"

void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("\n=== THERMO firmware " FW_VERSION " ===");

    // 1. Logger — jako první, ať můžeme logovat vše ostatní
    logInit();
    LOG(LOG_INFO, "MAIN", "Boot start, fw: %s", FW_VERSION);

    // 2. Konfigurace z NVS
    configLoad();
    LOG(LOG_INFO, "MAIN", "Config loaded: name=%s", unitName.c_str());

    // 3. WiFi — AP+STA
    wifiInit();

    // 4. Senzory
    sensorInit();

    // 5. API klient
    apiClientInit();

    // 6. Sync názvů senzorů
    sensorSyncInit();

    // 7. Web server — jako poslední
    webServerInit();

    LOG(LOG_INFO, "MAIN", "Boot complete");
}

void loop() {
    // Pořadí je důležité — nejdřív data, pak odesílání
    wifiLoop();
    sensorLoop();
    sensorSyncLoop();
    apiClientLoop();
    webServerLoop();

    // Malá pauza aby watchdog neštěkal
    delay(10);
}