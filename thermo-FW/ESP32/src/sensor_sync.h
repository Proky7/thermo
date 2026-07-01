#pragma once
#include <Arduino.h>

void sensorSyncInit();
void sensorSyncLoop();

// Okamžitá synchronizace (po pojmenování senzoru z web UI)
void sensorSyncNow();