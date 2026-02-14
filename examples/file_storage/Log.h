#pragma once

// Serial logging settings
#define LOG_LEVEL LOG_LEVEL_TRACE
#define LOG_LEVEL_TEXT_FORMAT LOG_LEVEL_TEXT_FORMAT_SHORT
#define LOG_TIME LOG_TIME_MILLIS
#define LOG_FILENAME LOG_FILENAME_LINENUMBER_ENABLE
#define LOG_COLOR LOG_COLOR_DISABLE
#define LOG_STATIC_BUFFER_SIZE 256
#define LOG_STREAM Serial

// File storage settings
#define LOG_FILE_ENABLE 1             // MUST BE ENABLED
#define LOG_FILE_LEVEL LOG_LEVEL_WARN // Store WARN and ERROR messages
#define LOG_FILE_MAX_BUFFER_SIZE 4096
#define LOG_FILE_MAX_SIZE (1024 * 100) // 100KB per file
#define LOG_FILE_MAX_FILES 3

#include <FormatLog.h>
