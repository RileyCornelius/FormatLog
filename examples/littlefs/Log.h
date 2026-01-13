#pragma once

// Serial logging settings
#define LOG_LEVEL LOG_LEVEL_TRACE
#define LOG_LEVEL_TEXT_FORMAT LOG_LEVEL_TEXT_FORMAT_SHORT
#define LOG_TIME LOG_TIME_MILLIS
#define LOG_FILENAME LOG_FILENAME_DISABLE
#define LOG_COLOR LOG_COLOR_DISABLE
#define LOG_STATIC_BUFFER_SIZE 256
#define LOG_STREAM Serial

// Storage settings
#define LOG_STORAGE_ENABLE 1
#define LOG_STORAGE_LEVEL LOG_LEVEL_WARN
#define LOG_STORAGE_MAX_BUFFER_MESSAGES 5 // Flush every 5 messages for testing
#define LOG_STORAGE_MAX_BUFFER_SIZE 512   // Or every 512 bytes
#define LOG_STORAGE_MAX_FILE_SIZE 1024
#define LOG_STORAGE_MAX_FILES 3 // Keep 3 backend files

#include <FormatLog.h>
