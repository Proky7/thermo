#pragma once
#include <Arduino.h>
#include "config.h"

struct SensorReading {
    char    romId[ROM_ID_LEN];
    float   value;
    bool    valid;
    uint8_t missingCount;
    char    name[64];
};

void                 sensorInit();
void                 sensorLoop();
void                 sensorSetName(const char* romId, const char* name);
const char*          sensorGetName(const char* romId);
int                  sensorGetCount();
const SensorReading* sensorGetAll();
bool                 sensorHasNewReading();
const SensorReading* sensorGetLatest(int* count);
void                 sensorClearNewReading();