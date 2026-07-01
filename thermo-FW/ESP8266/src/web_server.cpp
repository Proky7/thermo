#include "web_server.h"
#include "config.h"
#include "logger.h"
#include "sensor_manager.h"
#include "sensor_sync.h"
#include "api_client.h"
#include "wifi_manager.h"
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

static AsyncWebServer server(80);

// ── /api/status ──────────────────────────────────────────

static void handleStatus(AsyncWebServerRequest* req) {
    JsonDocument doc;
    doc["unit_name"]      = unitName;
    doc["unit_mac"]       = unitMac;
    doc["fw_version"]     = FW_VERSION;
    doc["wifi_ssid"]      = wifiGetSSID();
    doc["wifi_ip"]        = wifiGetIP();
    doc["wifi_connected"] = wifiIsConnected();
    doc["api_url"]        = apiUrl;
    doc["uptime_s"]       = millis() / 1000;

    JsonArray arr = doc["sensors"].to<JsonArray>();
    int count = sensorGetCount();
    const SensorReading* all = sensorGetAll();
    for (int i = 0; i < count; i++) {
        JsonObject s = arr.add<JsonObject>();
        s["rom_id"] = all[i].romId;
        s["name"]   = all[i].name;
        s["value"]  = all[i].valid ? all[i].value : -99.0f;
        s["valid"]  = all[i].valid;
        s["named"]  = strlen(all[i].name) > 0;
    }

    String out;
    serializeJson(doc, out);
    req->send(200, "application/json", out);
}

// ── /api/config GET ───────────────────────────────────────

static void handleConfigGet(AsyncWebServerRequest* req) {
    JsonDocument doc;
    doc["unit_name"] = unitName;
    doc["api_url"]   = apiUrl;

    String out;
    serializeJson(doc, out);
    req->send(200, "application/json", out);
}

// ── /api/config POST ──────────────────────────────────────

static void handleConfigPost(AsyncWebServerRequest* req,
                              uint8_t* data, size_t len,
                              size_t index, size_t total) {
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, data, len);
    if (err) {
        req->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
    }

    const char* name  = doc["unit_name"] | unitName.c_str();
    const char* url   = doc["api_url"]   | apiUrl.c_str();
    const char* token = doc["api_token"] | apiToken.c_str();

    configSave(name, url, token);

    LOG(LOG_INFO, "WEB", "Config updated: name=%s", name);
    req->send(200, "application/json", "{\"ok\":true}");
}

// ── /api/wifi POST ────────────────────────────────────────

static void handleWifi(AsyncWebServerRequest* req,
                        uint8_t* data, size_t len,
                        size_t index, size_t total) {
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, data, len);
    if (err) {
        req->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
    }

    const char* ssid = doc["ssid"] | "";
    const char* pass = doc["pass"] | "";

    if (strlen(ssid) == 0) {
        req->send(400, "application/json", "{\"error\":\"SSID required\"}");
        return;
    }

    wifiSaveCredentials(ssid, pass);
    LOG(LOG_INFO, "WEB", "WiFi saved: %s", ssid);
    req->send(200, "application/json", "{\"ok\":true}");
}

// ── /api/sensors GET ──────────────────────────────────────

static void handleSensorsGet(AsyncWebServerRequest* req) {
    JsonDocument doc;
    JsonArray arr = doc["sensors"].to<JsonArray>();

    int count = sensorGetCount();
    const SensorReading* all = sensorGetAll();
    for (int i = 0; i < count; i++) {
        JsonObject s = arr.add<JsonObject>();
        s["rom_id"] = all[i].romId;
        s["name"]   = all[i].name;
        s["value"]  = all[i].valid ? all[i].value : -99.0f;
        s["valid"]  = all[i].valid;
        s["named"]  = strlen(all[i].name) > 0;
    }

    String out;
    serializeJson(doc, out);
    req->send(200, "application/json", out);
}

// ── /api/sensors POST ─────────────────────────────────────

static void handleSensorsPost(AsyncWebServerRequest* req,
                               uint8_t* data, size_t len,
                               size_t index, size_t total) {
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, data, len);
    if (err) {
        req->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
        return;
    }

    const char* romId = doc["rom_id"] | "";
    const char* name  = doc["name"]   | "";

    if (strlen(romId) == 0 || strlen(name) == 0) {
        req->send(400, "application/json",
                  "{\"error\":\"rom_id and name required\"}");
        return;
    }

    // Ulož lokálně
    sensorSetName(romId, name);

    // Okamžitě pošli na API
    ApiResult result = apiAnnounceSensor(romId, name);
    if (result != API_OK) {
        LOG(LOG_WARN, "WEB", "Failed to send name to API, will retry on sync");
    }

    // Naplánuj sync pro jistotu
    sensorSyncNow();

    LOG(LOG_INFO, "WEB", "Sensor %s named: %s", romId, name);
    req->send(200, "application/json", "{\"ok\":true}");
}

// ── /api/scan GET ─────────────────────────────────────────

static void handleScan(AsyncWebServerRequest* req) {
    int n = WiFi.scanNetworks();
    JsonDocument doc;
    JsonArray arr = doc["networks"].to<JsonArray>();
    for (int i = 0; i < n; i++) {
        JsonObject net = arr.add<JsonObject>();
        net["ssid"]   = WiFi.SSID(i);
        net["rssi"]   = WiFi.RSSI(i);
        net["secure"] = (WiFi.encryptionType(i) != ENC_TYPE_NONE);
    }
    String out;
    serializeJson(doc, out);
    req->send(200, "application/json", out);
}

// ── /api/logs GET ─────────────────────────────────────────

static void handleLogs(AsyncWebServerRequest* req) {
    JsonDocument doc;
    JsonArray arr = doc["logs"].to<JsonArray>();
    const char* levelStr[] = {"DEBUG","INFO","WARN","ERROR","CRITICAL"};

    int count = logGetCount();
    for (int i = 0; i < count; i++) {
        const LogEntry* e = logGetEntry(i);
        if (!e) continue;
        JsonObject entry = arr.add<JsonObject>();
        entry["level"]   = levelStr[e->level];
        entry["module"]  = e->module;
        entry["message"] = e->message;
        entry["uptime"]  = e->timestamp / 1000;
    }

    String out;
    serializeJson(doc, out);
    req->send(200, "application/json", out);
}

// ── Inicializace ─────────────────────────────────────────

void webServerInit() {
    extern const char* getWebUI();
    server.on("/", HTTP_GET, [](AsyncWebServerRequest* req) {
        req->send(200, "text/html", getWebUI());
    });

    server.on("/api/status",  HTTP_GET,  handleStatus);
    server.on("/api/config",  HTTP_GET,  handleConfigGet);
    server.on("/api/sensors", HTTP_GET,  handleSensorsGet);
    server.on("/api/scan",    HTTP_GET,  handleScan);
    server.on("/api/logs",    HTTP_GET,  handleLogs);

    server.on("/api/config",  HTTP_POST,
        [](AsyncWebServerRequest* req){},
        nullptr, handleConfigPost);

    server.on("/api/wifi",    HTTP_POST,
        [](AsyncWebServerRequest* req){},
        nullptr, handleWifi);

    server.on("/api/sensors", HTTP_POST,
        [](AsyncWebServerRequest* req){},
        nullptr, handleSensorsPost);

    // server.onNotFound([](AsyncWebServerRequest* req) {
    //     req->send(404, "application/json", "{\"error\":\"Not found\"}");
    // });
    server.onNotFound([](AsyncWebServerRequest* req) {
    // Pokud mobil zkouší ověřit internet, podstrčíme mu naše Web UI
    extern const char* getWebUI();
    req->send(200, "text/html", getWebUI());

    server.begin();
    LOG(LOG_INFO, "WEB", "Server started on port 80");
}

void webServerLoop() {
    // AsyncWebServer nepotřebuje volání v loop
}