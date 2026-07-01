#include "sensor_manager.h"
#include "logger.h"
#include "config.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

static OneWire           oneWire(PIN_ONEWIRE);
static DallasTemperature dallas(&oneWire);

static SensorReading sensors[SENSOR_MAX_COUNT];
static int           sensorCount   = 0;
static bool          newReading    = false;

// ── Async stavový automat ─────────────────────────────────
enum MeasureState {
    MEASURE_IDLE,
    MEASURE_REQUESTED,
    MEASURE_READING,
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

// ── LittleFS — ukládání jmen ──────────────────────────────

static void loadNamesFromFS() {
    if (!LittleFS.begin()) return;
    if (!LittleFS.exists(SENSORS_FILE)) return;

    File f = LittleFS.open(SENSORS_FILE, "r");
    if (!f) return;

    JsonDocument doc;
    if (deserializeJson(doc, f) != DeserializationError::Ok) {
        f.close();
        return;
    }
    f.close();

    for (int i = 0; i < sensorCount; i++) {
        if (doc.containsKey(sensors[i].romId)) {
            const char* name = doc[sensors[i].romId];
            strncpy(sensors[i].name, name, sizeof(sensors[i].name) - 1);
            LOG(LOG_DEBUG, "SENSOR", "Loaded name: %s = %s",
                sensors[i].romId, sensors[i].name);
        }
    }
}

static void saveNamesToFS() {
    if (!LittleFS.begin()) return;

    JsonDocument doc;
    for (int i = 0; i < sensorCount; i++) {
        if (strlen(sensors[i].name) > 0) {
            doc[sensors[i].romId] = sensors[i].name;
        }
    }

    File f = LittleFS.open(SENSORS_FILE, "w");
    if (!f) return;
    serializeJson(doc, f);
    f.close();
}

// ── Inicializace ─────────────────────────────────────────

void sensorInit() {
    dallas.begin();
    dallas.setResolution(12);
    dallas.setWaitForConversion(false);

    memset(sensors, 0, sizeof(sensors));

    uint8_t rom[8];
    oneWire.reset_search();
    while (oneWire.search(rom)) {
        if (OneWire::crc8(rom, 7) != rom[7]) {
            LOG(LOG_WARN, "SENSOR", "CRC error during scan");
            continue;
        }
        if (sensorCount >= SENSOR_MAX_COUNT) {
            LOG(LOG_WARN, "SENSOR", "Max sensors reached (%d)", SENSOR_MAX_COUNT);
            break;
        }
        char romStr[ROM_ID_LEN];
        romToString(rom, romStr);
        strncpy(sensors[sensorCount].romId, romStr, ROM_ID_LEN - 1);
        sensors[sensorCount].valid        = false;
        sensors[sensorCount].missingCount = 0;
        sensorCount++;
        LOG(LOG_INFO, "SENSOR", "Found: %s", romStr);
    }

    LOG(LOG_INFO, "SENSOR", "Total: %d sensors", sensorCount);
    loadNamesFromFS();
}

// ── Čtení teplot ─────────────────────────────────────────

static void readTemperatures() {
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

        if (sensors[i].missingCount > 0) {
            LOG(LOG_INFO, "SENSOR", "Sensor %s recovered after %d missing",
                sensors[i].romId, sensors[i].missingCount);
        }
        sensors[i].missingCount = 0;
        sensors[i].valid        = true;
        sensors[i].value        = temp;
    }
    newReading = true;
}

// ── Loop ─────────────────────────────────────────────────

void sensorLoop() {
    uint32_t now = millis();

    switch (measureState) {
        case MEASURE_IDLE:
            if (now - lastMeasureMs >= SENSOR_INTERVAL_MS) {
                lastMeasureMs = now;
                dallas.requestTemperatures();
                convStartMs   = now;
                measureState  = MEASURE_REQUESTED;
            }
            break;

        case MEASURE_REQUESTED:
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

// ── API ──────────────────────────────────────────────────

bool sensorHasNewReading() {
    return newReading;
}

const SensorReading* sensorGetLatest(int* count) {
    *count = sensorCount;
    return sensors;
}

void sensorClearNewReading() {
    newReading = false;
}

void sensorSetName(const char* romId, const char* name) {
    int idx = findSensor(romId);
    if (idx >= 0) {
        strncpy(sensors[idx].name, name, sizeof(sensors[idx].name) - 1);
        saveNamesToFS();
        LOG(LOG_INFO, "SENSOR", "Named: %s = %s", romId, name);
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