#include "config.h"
#include "logger.h"
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>

String unitName  = "THERMO-8266";
String unitMac   = "";
String apiUrl    = DEFAULT_API_URL;
String apiToken  = DEFAULT_API_TOKEN;
bool   debugMode = false;

void configLoad() {
    if (!LittleFS.begin()) {
        LOG(LOG_WARN, "CFG", "LittleFS mount failed, using defaults");
        return;
    }

    if (!LittleFS.exists(CFG_FILE)) {
        LOG(LOG_INFO, "CFG", "No config file, using defaults");
        return;
    }

    File f = LittleFS.open(CFG_FILE, "r");
    if (!f) {
        LOG(LOG_WARN, "CFG", "Cannot open config file");
        return;
    }

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, f);
    f.close();

    if (err) {
        LOG(LOG_WARN, "CFG", "Config parse error: %s", err.c_str());
        return;
    }

    unitName = doc["name"] | unitName;
    apiUrl   = doc["api_url"]   | apiUrl;
    apiToken = doc["api_token"] | apiToken;

    LOG(LOG_INFO, "CFG", "Config loaded: name=%s", unitName.c_str());
}

void configSave(const char* name, const char* url, const char* token) {
    if (!LittleFS.begin()) {
        LOG(LOG_WARN, "CFG", "LittleFS mount failed");
        return;
    }

    JsonDocument doc;
    doc["name"]      = name;
    doc["api_url"]   = url;
    doc["api_token"] = token;

    File f = LittleFS.open(CFG_FILE, "w");
    if (!f) {
        LOG(LOG_WARN, "CFG", "Cannot write config file");
        return;
    }

    serializeJson(doc, f);
    f.close();

    unitName = String(name);
    apiUrl   = String(url);
    apiToken = String(token);

     // A okamžitě aktualizuj WiFi
    String apSSID = String(AP_SSID_PREFIX) + unitMac.substring(9);
    #ifdef ESP8266
        WiFi.hostname(unitName.c_str());
        WiFi.softAP(apSSID.c_str());
    #else
        WiFi.setHostname(unitName.c_str());
        WiFi.softAP(apSSID.c_str(), AP_PASSWORD);
    #endif


    LOG(LOG_INFO, "CFG", "Config saved: name=%s", name);
}