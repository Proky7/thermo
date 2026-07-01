#pragma once
#include <Arduino.h>

enum LogLevel {
    LOG_DEBUG = 0,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_CRITICAL
};

struct LogEntry {
    LogLevel level;
    String   module;
    String   message;
    uint32_t timestamp;
};

void logInit();
void logWrite(LogLevel level, const char* module, const char* fmt, ...);

#define LOG(level, module, fmt, ...) logWrite(level, module, fmt, ##__VA_ARGS__)

int              logGetCount();
const LogEntry*  logGetEntry(int index);
int              logUnreadWarnCount();