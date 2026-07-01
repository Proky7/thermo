#pragma once
#include <Arduino.h>

void   wifiInit();
void   wifiLoop();
bool   wifiIsConnected();
String wifiGetIP();
String wifiGetSSID();
void   wifiSaveCredentials(const char* ssid, const char* password);