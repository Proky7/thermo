#pragma once
#include <Arduino.h>
#include "sensor_manager.h"

void apiClientInit();
void apiClientLoop();

// Výsledek odeslání
enum ApiResult {
    API_OK,
    API_ERR_WIFI,       // není WiFi
    API_ERR_TIMEOUT,    // server neodpověděl
    API_ERR_AUTH,       // 401 — špatný token
    API_ERR_SERVER,     // 5xx
};

ApiResult apiSendMeasurements(MeasurementBuffer::Entry* entry);
ApiResult apiSendLogs();
ApiResult apiAnnounceSensor(const char* romId, const char* name);