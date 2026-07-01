#pragma once
#include <Arduino.h>
#include "sensor_manager.h"

void apiClientInit();
void apiClientLoop();

enum ApiResult {
    API_OK,
    API_ERR_WIFI,
    API_ERR_TIMEOUT,
    API_ERR_AUTH,
    API_ERR_SERVER,
};

ApiResult apiSendMeasurements(const SensorReading* readings, int count);
ApiResult apiSendLogs();
ApiResult apiAnnounceSensor(const char* romId, const char* name);