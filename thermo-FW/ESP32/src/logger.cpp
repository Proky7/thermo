#include "logger.h"
#include "config.h"

static LogEntry logBuffer[LOG_BUFFER_SIZE];
static int      logCount    = 0;
static int      logHead     = 0;  // kruhový buffer

void logInit() {
    pinMode(PIN_DEBUG_ENABLE, INPUT_PULLUP);
    debugMode = (digitalRead(PIN_DEBUG_ENABLE) == LOW);
    if (debugMode) {
        Serial.println("[LOGGER] Debug mode ON");
    }
}

void logWrite(LogLevel level, const char* module, const char* fmt, ...) {
    char buf[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    // Vždy na Serial
    if (level >= LOG_DEBUG) {
        const char* lvlStr[] = {"DEBUG","INFO","WARN","ERROR","CRIT"};
        if (debugMode || level > LOG_DEBUG) {
            Serial.printf("[%s][%s] %s\n", lvlStr[level], module, buf);
        }
    }

    // Do RAM bufferu jen WARN a výše
    if (level >= LOG_WARN) {
        logBuffer[logHead] = {
            .level     = level,
            .module    = String(module),
            .message   = String(buf),
            .timestamp = millis()
        };
        logHead = (logHead + 1) % LOG_BUFFER_SIZE;
        if (logCount < LOG_BUFFER_SIZE) logCount++;
    }
}

int logGetCount() {
    return logCount;
}

const LogEntry* logGetEntry(int index) {
    if (index >= logCount) return nullptr;
    return &logBuffer[index];
}

int logUnreadWarnCount() {
    return logCount;
}

void logSendPending() {
    // TODO: odesílání na API — implementujeme po dokončení api_client
}