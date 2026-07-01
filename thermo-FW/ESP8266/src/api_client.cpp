#include "api_client.h"
#include "config.h"
#include "logger.h"
#include <ESP8266WiFi.h>
#include <WiFiClientSecureBearSSL.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

static uint32_t lastLogMs = 0;

// ── Pomocná funkce — HTTP POST ────────────────────────────

static ApiResult httpPost(const char* endpoint, const String& body) {
    if (!wifiIsConnected()) return API_ERR_WIFI;

    BearSSL::WiFiClientSecure client;
    client.setInsecure();  // TODO: certifikát pro produkci

    HTTPClient http;
    String url = apiUrl + endpoint;

    if (!http.begin(client, url)) {
        LOG(LOG_WARN, "API", "http.begin failed for %s", endpoint);
        return API_ERR_SERVER;
    }

    http.addHeader("Content-Type", "application/json");
    http.addHeader("X-Api-Token", apiToken);
    http.setTimeout(API_TIMEOUT_MS);

    int code = http.POST(body);
    http.end();

    if (code == 200 || code == 201) return API_OK;
    if (code == 401) {
        LOG(LOG_ERROR, "API", "Unauthorized — check API token");
        return API_ERR_AUTH;
    }
    if (code <= 0) {
        LOG(LOG_WARN, "API", "Timeout or connection error: %d", code);
        return API_ERR_TIMEOUT;
    }
    LOG(LOG_WARN, "API", "Server error: %d", code);
    return API_ERR_SERVER;
}

// ── Odeslání měření ───────────────────────────────────────

ApiResult apiSendMeasurements(const SensorReading* readings, int count) {
    JsonDocument doc;
    doc["unit_mac"]   = unitMac;
    doc["unit_name"]  = unitName;
    doc["fw_version"] = FW_VERSION;

    JsonArray arr = doc["readings"].to<JsonArray>();
    int sent = 0;
    for (int i = 0; i < count; i++) {
        if (!readings[i].valid) continue;
        JsonObject r = arr.add<JsonObject>();
        r["rom_id"] = readings[i].romId;
        r["value"]  = readings[i].value;
        sent++;
    }

    if (sent == 0) {
        LOG(LOG_DEBUG, "API", "No valid readings to send");
        return API_OK;
    }

    String body;
    serializeJson(doc, body);

    ApiResult result = httpPost("/api/v1/measurements", body);
    if (result == API_OK) {
        LOG(LOG_DEBUG, "API", "Sent %d readings", sent);
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
        entry["ts"]      = e->timestamp / 1000;
    }

    String body;
    serializeJson(doc, body);

    ApiResult result = httpPost("/api/v1/logs", body);
    if (result == API_OK) {
        LOG(LOG_DEBUG, "API", "Logs sent (%d entries)", count);
    }
    return result;
}

// ── Init a Loop ───────────────────────────────────────────

void apiClientInit() {
    LOG(LOG_INFO, "API", "Initialized, url: %s", apiUrl.c_str());
}

void apiClientLoop() {
    uint32_t now = millis();

    // Odešli nová měření — best effort
    if (sensorHasNewReading() && wifiIsConnected()) {
        int count = 0;
        const SensorReading* readings = sensorGetLatest(&count);
        ApiResult result = apiSendMeasurements(readings, count);
        if (result == API_OK || result == API_ERR_AUTH) {
            // Potvrď i při auth chybě — nemá cenu držet data
            sensorClearNewReading();
        } else {
            // Timeout/server chyba — zkusíme příště
            // ale na ESP8266 bez bufferu prostě přeskočíme
            sensorClearNewReading();
            LOG(LOG_WARN, "API", "Send failed (%d), skipping", result);
        }
    } else if (sensorHasNewReading()) {
        // Není WiFi — přeskočíme
        sensorClearNewReading();
    }

    // Logy každou minutu
    if (now - lastLogMs >= LOG_SEND_INTERVAL_MS && wifiIsConnected()) {
        lastLogMs = now;
        apiSendLogs();
    }
}