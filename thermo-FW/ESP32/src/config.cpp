#include "config.h"
#include <Preferences.h>
#include <WiFi.h>

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
    debugMode = prefs.getBool(NVS_CFG_DEBUG, false);
    prefs.end();
}

void configSave(const char* name, const char* url, const char* token, bool dbg) {
    Preferences prefs;
    prefs.begin(NVS_CFG_NS, false);
    prefs.putString(NVS_CFG_NAME,      name);
    prefs.putString(NVS_CFG_API_URL,   url);
    prefs.putString(NVS_CFG_API_TOKEN, token);
    prefs.putBool(NVS_CFG_DEBUG, dbg);
    prefs.end();
    unitName = String(name);
    apiUrl   = String(url);
    apiToken = String(token);
    debugMode = dbg;
    // A okamžitě aktualizuj WiFi
    String apSSID = String(AP_SSID_PREFIX) + unitMac.substring(9);
    #ifdef ESP8266
        WiFi.hostname(unitName.c_str());
        WiFi.softAP(apSSID.c_str());
    #else
        WiFi.setHostname(unitName.c_str());
        WiFi.softAP(apSSID.c_str(), AP_PASSWORD);
    #endif

}