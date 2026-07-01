#pragma once
#include <Arduino.h>

// ── VERZE ────────────────────────────────────────────────
#define FW_VERSION          "0.1.0"

// ── PINY ─────────────────────────────────────────────────
#define PIN_ONEWIRE         4       // DS18B20 data
#define PIN_DEBUG_ENABLE    33      // zkratovat na GND = DEBUG logy

// ── WIFI ─────────────────────────────────────────────────
#define AP_SSID_PREFIX      "THERMO-"   // + MAC suffix
#define AP_PASSWORD         "thermo123"
#define WIFI_CONNECT_TIMEOUT_MS  15000

// ── SENSOR ───────────────────────────────────────────────
#define SENSOR_INTERVAL_MS       10000  // měření každých 10s
#define SENSOR_MISSING_WARN      5      // po 5 chybách → WARN
#define SENSOR_BUFFER_SIZE       10     // buffer při výpadku API
#define SENSOR_MAX_COUNT         10  // max senzorů na jednom ESP

// ── API ──────────────────────────────────────────────────
#define API_TIMEOUT_MS           5000
#define API_RETRY_INTERVAL_MS    30000  // retry po výpadku

// ── SYNC názvů senzorů ───────────────────────────────────
#define SENSOR_SYNC_INTERVAL_MS  300000 // každých 5 minut

// ── LOGGER ───────────────────────────────────────────────
#define LOG_BUFFER_SIZE     15      // posledních 15 záznamů WARN+
#define LOG_SEND_INTERVAL_MS 60000  // odesílání logů každou minutu

// ── NVS namespaces ───────────────────────────────────────
#define NVS_WIFI_NS         "wifi"
#define NVS_CFG_NS          "cfg"
#define NVS_SENSORS_NS      "sensors"  // cache názvů senzorů

// ── NVS klíče pro konfiguraci ────────────────────────────
#define NVS_CFG_NAME      "name"
#define NVS_CFG_API_URL   "api_url"
#define NVS_CFG_API_TOKEN "api_token"

// ── Globální proměnné (definice v config.cpp) ────────────
extern String unitName;
extern String unitMac;
extern String apiUrl;
extern String apiToken;
extern bool   debugMode;

void configLoad();
void configSave(const char* name, const char* url, const char* token);
