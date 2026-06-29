#include "sensor_manager.h"
#include "logger.h"
#include "config.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Preferences.h>

static OneWire           oneWire(PIN_ONEWIRE);
static DallasTemperature dallas(&oneWire);
static Preferences       prefs;

static SensorReading     sensors[SENSOR_MAX_COUNT];
static int               sensorCount = 0;

static MeasurementBuffer measureBuffer;

// ── Async stavový automat ─────────────────────────────────
enum MeasureState {
    MEASURE_IDLE,        // čekání na interval
    MEASURE_REQUESTED,   // konverze spuštěna, čekáme 750ms
    MEASURE_READING,     // čteme výsledky
};

static MeasureState measureState  = MEASURE_IDLE;
static uint32_t     lastMeasureMs = 0;
static uint32_t     convStartMs   = 0;

// ── Pomocné funkce ────────────────────────────────────────

static void romToString(const uint8_t* rom, char* out) {
    snprintf(out, ROM_ID_LEN, "%02X%02X%02X%02X%02X%02X%02X%02X",
        rom[0], rom[1], rom[2], rom[3],
        rom[4], rom[5], rom[6], rom[7]);
}

static bool stringToRom(const char* str, uint8_t* rom) {
    int parsed = sscanf(str,
        "%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX",
        &rom[0], &rom[1], &rom[2], &rom[3],
        &rom[4], &rom[5], &rom[6], &rom[7]);
    return parsed == 8;
}

static int findSensor(const char* romId) {
    for (int i = 0; i < sensorCount; i++) {
        if (strcmp(sensors[i].romId, romId) == 0) return i;
    }
    return -1;
}

// ── NVS — ukládání názvů ─────────────────────────────────

static void loadNamesFromNVS() {
    prefs.begin(NVS_SENSORS_NS, true);
    for (int i = 0; i < sensorCount; i++) {
        String name = prefs.getString(sensors[i].romId, "");
        if (name.length() > 0) {
            strncpy(sensors[i].name, name.c_str(), sizeof(sensors[i].name) - 1);
            LOG(LOG_DEBUG, "SENSOR", "Loaded name from NVS: %s = %s",
                sensors[i].romId, sensors[i].name);
        }
    }
    prefs.end();
}

static void saveNameToNVS(const char* romId, const char* name) {
    prefs.begin(NVS_SENSORS_NS, false);
    prefs.putString(romId, name);
    prefs.end();
}

// ── Inicializace ─────────────────────────────────────────

void sensorInit() {
    dallas.begin();
    dallas.setResolution(12);
    dallas.setWaitForConversion(false);  // klíčové pro async

    memset(&measureBuffer, 0, sizeof(measureBuffer));
    memset(sensors, 0, sizeof(sensors));

    // Scan sběrnice
    uint8_t rom[8];
    oneWire.reset_search();
    while (oneWire.search(rom)) {
        if (OneWire::crc8(rom, 7) != rom[7]) {
            LOG(LOG_WARN, "SENSOR", "CRC error during scan, skipping");
            continue;
        }
        if (sensorCount >= SENSOR_MAX_COUNT) {
            LOG(LOG_WARN, "SENSOR", "Max sensor count reached (%d)", SENSOR_MAX_COUNT);
            break;
        }
        char romStr[ROM_ID_LEN];
        romToString(rom, romStr);
        strncpy(sensors[sensorCount].romId, romStr, ROM_ID_LEN - 1);
        sensors[sensorCount].valid        = false;
        sensors[sensorCount].missingCount = 0;
        memset(sensors[sensorCount].name, 0, sizeof(sensors[sensorCount].name));
        sensorCount++;
        LOG(LOG_INFO, "SENSOR", "Found: %s", romStr);
    }

    LOG(LOG_INFO, "SENSOR", "Total sensors: %d", sensorCount);
    loadNamesFromNVS();
}

// ── Čtení výsledků (voláno po 750ms) ─────────────────────

static void readTemperatures() {
    MeasurementBuffer::Entry entry;
    entry.count     = 0;
    entry.timestamp = millis();

    for (int i = 0; i < sensorCount; i++) {
        uint8_t rom[8];
        if (!stringToRom(sensors[i].romId, rom)) continue;

        float temp = dallas.getTempC(rom);

        if (temp == DEVICE_DISCONNECTED_C || temp < -55.0f || temp > 125.0f) {
            sensors[i].missingCount++;
            sensors[i].valid = false;

            if (sensors[i].missingCount == 1) {
                LOG(LOG_INFO, "SENSOR", "Sensor %s not responding",
                    sensors[i].romId);
            } else if (sensors[i].missingCount == SENSOR_MISSING_WARN) {
                LOG(LOG_WARN, "SENSOR", "Sensor %s missing for %d readings",
                    sensors[i].romId, SENSOR_MISSING_WARN);
            }
            continue;
        }

        // Zotavení po výpadku
        if (sensors[i].missingCount > 0) {
            LOG(LOG_INFO, "SENSOR", "Sensor %s recovered after %d missing",
                sensors[i].romId, sensors[i].missingCount);
        }
        sensors[i].missingCount = 0;
        sensors[i].valid        = true;
        sensors[i].value        = temp;

        entry.readings[entry.count] = sensors[i];
        entry.count++;
    }

    // Ulož do kruhového bufferu
    if (entry.count > 0) {
        int slot = (measureBuffer.head + measureBuffer.count) % SENSOR_BUFFER_SIZE;
        measureBuffer.entries[slot] = entry;
        if (measureBuffer.count < SENSOR_BUFFER_SIZE) {
            measureBuffer.count++;
        } else {
            // Buffer plný — zahoď nejstarší
            measureBuffer.head = (measureBuffer.head + 1) % SENSOR_BUFFER_SIZE;
            LOG(LOG_WARN, "SENSOR", "Buffer full, dropping oldest entry");
        }
    }
}

// ── Loop — stavový automat ────────────────────────────────

void sensorLoop() {
    uint32_t now = millis();

    switch (measureState) {
        case MEASURE_IDLE:
            if (now - lastMeasureMs >= SENSOR_INTERVAL_MS) {
                lastMeasureMs = now;
                dallas.requestTemperatures();
                convStartMs  = now;
                measureState = MEASURE_REQUESTED;
                LOG(LOG_DEBUG, "SENSOR", "Conversion requested");
            }
            break;

        case MEASURE_REQUESTED:
            // Čekej 750ms na dokončení 12bit konverze
            if (now - convStartMs >= 750) {
                measureState = MEASURE_READING;
            }
            break;

        case MEASURE_READING:
            readTemperatures();
            measureState = MEASURE_IDLE;
            break;
    }
}

// ── Buffer API ───────────────────────────────────────────

bool sensorHasData() {
    return measureBuffer.count > 0;
}

MeasurementBuffer::Entry* sensorGetPending() {
    if (measureBuffer.count == 0) return nullptr;
    return &measureBuffer.entries[measureBuffer.head];
}

void sensorConfirmSent() {
    if (measureBuffer.count == 0) return;
    measureBuffer.head = (measureBuffer.head + 1) % SENSOR_BUFFER_SIZE;
    measureBuffer.count--;
}

// ── Správa názvů ─────────────────────────────────────────

void sensorSetName(const char* romId, const char* name) {
    int idx = findSensor(romId);
    if (idx >= 0) {
        strncpy(sensors[idx].name, name, sizeof(sensors[idx].name) - 1);
        saveNameToNVS(romId, name);
        LOG(LOG_INFO, "SENSOR", "Sensor %s named: %s", romId, name);
    }
}

const char* sensorGetName(const char* romId) {
    int idx = findSensor(romId);
    if (idx >= 0) return sensors[idx].name;
    return "";
}

int sensorGetCount() {
    return sensorCount;
}

const SensorReading* sensorGetAll() {
    return sensors;
}