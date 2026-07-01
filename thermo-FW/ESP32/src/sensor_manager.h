#pragma once
#include "config.h"
#include <Arduino.h>

#define ROM_ID_LEN 17  // "28FF1234ABCD0000" + null

struct SensorReading {
    char    romId[ROM_ID_LEN];
    float   value;
    bool    valid;
    uint8_t missingCount;  // počet po sobě jdoucích chyb
    char    name[64];      // z NVS cache / API sync
};

struct MeasurementBuffer {
    struct Entry {
        SensorReading readings[SENSOR_MAX_COUNT];
        uint8_t       count;
        uint32_t      timestamp;
    };
    Entry   entries[SENSOR_BUFFER_SIZE];
    uint8_t head;
    uint8_t count;
};

void    sensorInit();
void    sensorLoop();        // volat z loop()
bool    sensorHasData();     // jsou data k odeslání?
MeasurementBuffer::Entry* sensorGetPending();  // vrátí nejstarší batch
void    sensorConfirmSent(); // potvrdí odeslání — odstraní z bufferu

// Správa názvů senzorů
void    sensorSetName(const char* romId, const char* name);
const char* sensorGetName(const char* romId);
int     sensorGetCount();
const SensorReading* sensorGetAll();  // aktuální hodnoty pro web UI