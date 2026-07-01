#include "wifi_manager.h"
#include "config.h"
#include "logger.h"
#include <ESP8266WiFi.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

static String   staSSID     = "";
static String   staPassword = "";
static bool     connected   = false;
static uint32_t lastRetryMs = 0;

// ── NVS ──────────────────────────────────────────────────

static void loadCredentials() {
    if (!LittleFS.begin()) return;
    if (!LittleFS.exists(WIFI_FILE)) return;

    File f = LittleFS.open(WIFI_FILE, "r");
    if (!f) return;

    JsonDocument doc;
    if (deserializeJson(doc, f) == DeserializationError::Ok) {
        staSSID     = doc["ssid"] | "";
        staPassword = doc["pass"] | "";
    }
    f.close();
}

void wifiSaveCredentials(const char* ssid, const char* password) {
    if (!LittleFS.begin()) return;

    JsonDocument doc;
    doc["ssid"] = ssid;
    doc["pass"] = password;

    File f = LittleFS.open(WIFI_FILE, "w");
    if (!f) return;
    serializeJson(doc, f);
    f.close();

    staSSID     = String(ssid);
    staPassword = String(password);

    LOG(LOG_INFO, "WIFI", "Credentials saved for: %s", ssid);
    WiFi.begin(staSSID.c_str(), staPassword.c_str());
}

// ── Init ─────────────────────────────────────────────────

void wifiInit() {
    WiFi.mode(WIFI_AP_STA);

    // MAC adresa
    uint8_t mac[6];
    WiFi.macAddress(mac);
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    unitMac = String(macStr);

    // AP bez hesla
    String apSSID = String(AP_SSID_PREFIX) + unitMac.substring(9);
    WiFi.softAP(apSSID.c_str());
    LOG(LOG_INFO, "WIFI", "AP started: %s (open)", apSSID.c_str());

    // Hostname
    WiFi.hostname(unitName.c_str());

    // STA
    loadCredentials();
    if (staSSID.length() > 0) {
        LOG(LOG_INFO, "WIFI", "Connecting to: %s", staSSID.c_str());
        WiFi.begin(staSSID.c_str(), staPassword.c_str());
    } else {
        LOG(LOG_INFO, "WIFI", "No STA credentials, AP only");
    }
}

// ── Loop ─────────────────────────────────────────────────

void wifiLoop() {
    bool nowConnected = (WiFi.status() == WL_CONNECTED);

    if (nowConnected && !connected) {
        connected = true;
        LOG(LOG_INFO, "WIFI", "Connected to %s, IP: %s",
            WiFi.SSID().c_str(),
            WiFi.localIP().toString().c_str());
    }

    if (!nowConnected && connected) {
        connected = false;
        LOG(LOG_WARN, "WIFI", "Disconnected from %s", staSSID.c_str());
    }

    if (!nowConnected && staSSID.length() > 0) {
        uint32_t now = millis();
        if (now - lastRetryMs >= 30000) {
            lastRetryMs = now;
            LOG(LOG_INFO, "WIFI", "Retrying: %s", staSSID.c_str());
            WiFi.begin(staSSID.c_str(), staPassword.c_str());
        }
    }
}

bool wifiIsConnected() {
    return WiFi.status() == WL_CONNECTED;
}

String wifiGetIP() {
    return WiFi.localIP().toString();
}

String wifiGetSSID() {
    return WiFi.SSID();
}