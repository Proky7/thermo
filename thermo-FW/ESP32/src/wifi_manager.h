#pragma once
#include <Arduino.h>

void wifiInit();
void wifiLoop();

bool wifiIsConnected();
String wifiGetIP();
String wifiGetSSID();

// Uložení přihlašovacích údajů do NVS
void wifiSaveCredentials(const char* ssid, const char* password);