#include "config.h"
#include <Preferences.h>

String unitName  = "";
String unitMac   = "";
String apiUrl    = "";
String apiToken  = "";
bool   debugMode = false;

void configLoad() {
    Preferences prefs;
    prefs.begin(NVS_CFG_NS, true);
    unitName  = prefs.getString(NVS_CFG_NAME,      "THERMO");
    apiUrl    = prefs.getString(NVS_CFG_API_URL,   "");
    apiToken  = prefs.getString(NVS_CFG_API_TOKEN, "");
    prefs.end();
}

void configSave(const char* name, const char* url, const char* token) {
    Preferences prefs;
    prefs.begin(NVS_CFG_NS, false);
    prefs.putString(NVS_CFG_NAME,      name);
    prefs.putString(NVS_CFG_API_URL,   url);
    prefs.putString(NVS_CFG_API_TOKEN, token);
    prefs.end();
    unitName = String(name);
    apiUrl   = String(url);
    apiToken = String(token);
}