#include "api_client.h"
#include "config.h"
#include "logger.h"
#include "sensor_manager.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "wifi_manager.h"

static uint32_t lastSendMs  = 0;
static uint32_t lastLogMs   = 0;

// ── Pomocná funkce — HTTP POST ────────────────────────────

static ApiResult httpPost(const char* endpoint, const String& body) {
    if (!wifiIsConnected()) return API_ERR_WIFI;

    WiFiClientSecure client;
    client.setInsecure();  // TODO: přidat certifikát pro produkci

    HTTPClient http;
    String url = apiUrl + endpoint;
    http.begin(client, url);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-Api-Token", apiToken);
    http.setTimeout(API_TIMEOUT_MS);

    int code = http.POST(body);
    http.end();

    if (code == 200 || code == 201) return API_OK;
    if (code == 401)  return API_ERR_AUTH;
    if (code == -1)   return API_ERR_TIMEOUT;
    return API_ERR_SERVER;
}

// ── Odeslání měření ───────────────────────────────────────

ApiResult apiSendMeasurements(MeasurementBuffer::Entry* entry) {
    if (!entry) return API_ERR_SERVER;

    JsonDocument doc;
    doc["unit_mac"]   = unitMac;
    doc["unit_name"]  = unitName;
    doc["fw_version"] = FW_VERSION;

    JsonArray readings = doc["readings"].to<JsonArray>();
    for (int i = 0; i < entry->count; i++) {
        JsonObject r = readings.add<JsonObject>();
        r["rom_id"] = entry->readings[i].romId;
        r["value"]  = entry->readings[i].value;
    }

    String body;
    serializeJson(doc, body);

    ApiResult result = httpPost("/api/v1/measurements", body);

    if (result == API_OK) {
        LOG(LOG_DEBUG, "API", "Measurements sent (%d readings)", entry->count);
    } else if (result == API_ERR_AUTH) {
        LOG(LOG_ERROR, "API", "Unauthorized — check API token");
    } else if (result == API_ERR_TIMEOUT) {
        LOG(LOG_WARN, "API", "Timeout sending measurements");
    } else if (result == API_ERR_WIFI) {
        LOG(LOG_DEBUG, "API", "No WiFi, skipping send");
    }

    return result;
}

// ── Announce senzoru ──────────────────────────────────────

ApiResult apiAnnounceSensor(const char* romId, const char* name) {
    JsonDocument doc;
    doc["unit_mac"]  = unitMac;
    doc["unit_name"] = unitName;
    doc["rom_id"]    = romId;
    if (name && strlen(name) > 0) {
        doc["name"] = name;
    }

    String body;
    serializeJson(doc, body);

    ApiResult result = httpPost("/api/v1/sensors", body);
    if (result == API_OK) {
        LOG(LOG_INFO, "API", "Sensor announced: %s", romId);
    }
    return result;
}

// ── Odeslání logů ─────────────────────────────────────────

ApiResult apiSendLogs() {
    int count = logGetCount();
    if (count == 0) return API_OK;

    JsonDocument doc;
    doc["unit_mac"]  = unitMac;
    doc["unit_name"] = unitName;

    JsonArray entries = doc["entries"].to<JsonArray>();
    const char* levelStr[] = {"DEBUG","INFO","WARN","ERROR","CRITICAL"};

    for (int i = 0; i < count; i++) {
        const LogEntry* e = logGetEntry(i);
        if (!e) continue;
        JsonObject entry = entries.add<JsonObject>();
        entry["level"]   = levelStr[e->level];
        entry["message"] = e->module + ": " + e->message;
    }

    String body;
    serializeJson(doc, body);

    return httpPost("/api/v1/logs", body);
}

// ── Init a Loop ───────────────────────────────────────────

void apiClientInit() {
    LOG(LOG_INFO, "API", "API client initialized, url: %s", apiUrl.c_str());
}

void apiClientLoop() {
    uint32_t now = millis();

    // Odešli buffered měření
    if (sensorHasData() && wifiIsConnected()) {
        MeasurementBuffer::Entry* entry = sensorGetPending();
        ApiResult result = apiSendMeasurements(entry);
        if (result == API_OK) {
            sensorConfirmSent();
        }
        // Při chybě necháme v bufferu — zkusíme příště
    }

    // Odešli logy každou minutu
    if (now - lastLogMs >= LOG_SEND_INTERVAL_MS && wifiIsConnected()) {
        lastLogMs = now;
        apiSendLogs();
    }
}