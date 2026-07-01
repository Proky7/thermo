#pragma once
#include <Arduino.h>

// ── VERZE ────────────────────────────────────────────────
#define FW_VERSION          "0.1.0-8266"

// ── VÝCHOZÍ HODNOTY (přepsatelné z web UI) ───────────────
#define DEFAULT_API_URL     "https://thermo-api.p7d.cz"
#define DEFAULT_API_TOKEN   "NahodnyApiKlic159753"

// ── PINY ─────────────────────────────────────────────────
// D2 = GPIO4, D5 = GPIO14, D6 = GPIO12, D7 = GPIO13
#define PIN_ONEWIRE         4   // D2
#define PIN_DEBUG_ENABLE    14  // D5

// Pro budoucí použití — level konverter
#define PIN_LEVEL_RX        12  // D6
#define PIN_LEVEL_TX        13  // D7

// ── WiFi AP ──────────────────────────────────────────────
#define AP_SSID_PREFIX      "THERMO-"   // + MAC suffix
// Bez hesla

// ── SENSOR ───────────────────────────────────────────────
#define SENSOR_MAX_COUNT    16      // ESP8266 má méně RAM
#define SENSOR_INTERVAL_MS  10000
#define SENSOR_MISSING_WARN 5
#define ROM_ID_LEN          17

// ── API ──────────────────────────────────────────────────
#define API_TIMEOUT_MS      5000

// ── SYNC názvů senzorů ───────────────────────────────────
#define SENSOR_SYNC_INTERVAL_MS  300000  // 5 minut

// ── LOGGER ───────────────────────────────────────────────
#define LOG_BUFFER_SIZE     10      // posledních 10 WARN+
#define LOG_SEND_INTERVAL_MS 60000  // odesílání logů každou minutu

// ── LittleFS klíče ───────────────────────────────────────
#define CFG_FILE            "/config.json"
#define WIFI_FILE           "/wifi.json"
#define SENSORS_FILE        "/sensors.json"

// ── Globální proměnné ────────────────────────────────────
extern String unitName;
extern String unitMac;
extern String apiUrl;
extern String apiToken;
extern bool   debugMode;

void configLoad();
void configSave(const char* name, const char* url, const char* token);