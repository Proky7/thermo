#include "sensor_sync.h"
#include "config.h"
#include "logger.h"
#include "sensor_manager.h"
#include "api_client.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

static uint32_t lastSyncMs    = 0;
static bool     syncRequested = false;

// ── Stažení názvů z API ───────────────────────────────────

static void doSync() {
    if (!wifiIsConnected()) return;

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    String url = apiUrl + "/api/v1/sensors/" + unitMac;
    http.begin(client, url);
    http.addHeader("X-Api-Token", apiToken);
    http.setTimeout(API_TIMEOUT_MS);

    int code = http.GET();
    if (code != 200) {
        LOG(LOG_WARN, "SYNC", "Failed to fetch sensor names, code: %d", code);
        http.end();
        return;
    }

    String response = http.getString();
    http.end();

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, response);
    if (err) {
        LOG(LOG_WARN, "SYNC", "JSON parse error: %s", err.c_str());
        return;
    }

    JsonArray sensors = doc["sensors"].as<JsonArray>();
    int updated = 0;
    for (JsonObject s : sensors) {
        const char* romId = s["rom_id"];
        const char* name  = s["name"];
        if (romId && name && strlen(name) > 0) {
            sensorSetName(romId, name);
            updated++;
        }
    }

    // Announce neznámých senzorů
    int count = sensorGetCount();
    const SensorReading* all = sensorGetAll();
    for (int i = 0; i < count; i++) {
        if (strlen(all[i].name) == 0) {
            apiAnnounceSensor(all[i].romId, nullptr);
        }
    }

    LOG(LOG_INFO, "SYNC", "Sync done, %d names updated", updated);
}

// ── Init a Loop ───────────────────────────────────────────

void sensorSyncInit() {
    lastSyncMs = 0;  // sync okamžitě při startu
    LOG(LOG_INFO, "SYNC", "Sensor sync initialized");
}

void sensorSyncLoop() {
    uint32_t now = millis();

    bool shouldSync = syncRequested ||
                      (now - lastSyncMs >= SENSOR_SYNC_INTERVAL_MS);

    if (shouldSync && wifiIsConnected()) {
        lastSyncMs    = now;
        syncRequested = false;
        doSync();
    }
}

void sensorSyncNow() {
    syncRequested = true;
}