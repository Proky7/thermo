#include "wifi_manager.h"
#include "config.h"
#include "logger.h"
#include <WiFi.h>
#include <Preferences.h>
#include <DNSServer.h>

static Preferences prefs;
static String      staSSID     = "";
static String      staPassword = "";
static bool        connected   = false;
static uint32_t    lastRetryMs = 0;
static DNSServer dns;

// ── AP ───────────────────────────────────────────────────

static void startAP() {
    // 1. Nastavení IP adresy pro AP
    IPAddress apIP(192, 168, 4, 1);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

    // 2. Start AP
    String apSSID = String(AP_SSID_PREFIX) + unitMac.substring(9);
    WiFi.softAP(apSSID.c_str(), AP_PASSWORD);
    
    // 3. Start DNS serveru (přesměruje cokoli "*" na naše ESP)
    dns.start(53, "*", apIP);

    LOG(LOG_INFO, "WIFI", "AP started: %s, IP: 192.168.4.1", apSSID.c_str());
}

// ── NVS ──────────────────────────────────────────────────

static void loadCredentials() {
    prefs.begin(NVS_WIFI_NS, true);
    staSSID     = prefs.getString("ssid", "");
    staPassword = prefs.getString("pass", "");
    prefs.end();
}

void wifiSaveCredentials(const char* ssid, const char* password) {
    prefs.begin(NVS_WIFI_NS, false);
    prefs.putString("ssid", ssid);
    prefs.putString("pass", password);
    prefs.end();
    staSSID     = String(ssid);
    staPassword = String(password);
    LOG(LOG_INFO, "WIFI", "Credentials saved for SSID: %s", ssid);

    // Okamžitě zkus připojit
    WiFi.begin(staSSID.c_str(), staPassword.c_str());
}

// ── Inicializace ─────────────────────────────────────────

void wifiInit() {
    WiFi.mode(WIFI_AP_STA);

    // MAC adresa jako identifikátor jednotky
    uint8_t mac[6];
    WiFi.macAddress(mac);
    char macStr[18];
    snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
        mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    unitMac = String(macStr);
    LOG(LOG_INFO, "WIFI", "MAC: %s", unitMac.c_str());

    startAP();
    loadCredentials();

    if (staSSID.length() > 0) {
        LOG(LOG_INFO, "WIFI", "Connecting to: %s", staSSID.c_str());
        WiFi.begin(staSSID.c_str(), staPassword.c_str());
    } else {
        LOG(LOG_INFO, "WIFI", "No STA credentials, AP only mode");
    }

    // Hostname v síti
    if (unitName.length() > 0) {
        WiFi.setHostname(unitName.c_str());
    }
}

// ── Loop ─────────────────────────────────────────────────

void wifiLoop() {
    dns.processNextRequest();
    
    bool nowConnected = (WiFi.status() == WL_CONNECTED);

    if (nowConnected && !connected) {
        connected = true;
        LOG(LOG_INFO, "WIFI", "Connected to %s, IP: %s",
            WiFi.SSID().c_str(), WiFi.localIP().toString().c_str());
    }

    if (!nowConnected && connected) {
        connected = false;
        LOG(LOG_WARN, "WIFI", "Disconnected from %s", staSSID.c_str());
    }

    // Retry připojení každých 30s pokud odpojeno
    if (!nowConnected && staSSID.length() > 0) {
        uint32_t now = millis();
        if (now - lastRetryMs >= 30000) {
            lastRetryMs = now;
            LOG(LOG_INFO, "WIFI", "Retrying connection to %s", staSSID.c_str());
            WiFi.begin(staSSID.c_str(), staPassword.c_str());
        }
    }
}

// ── Gettery ──────────────────────────────────────────────

bool wifiIsConnected() {
    return WiFi.status() == WL_CONNECTED;
}

String wifiGetIP() {
    return WiFi.localIP().toString();
}

String wifiGetSSID() {
    return WiFi.SSID();
}