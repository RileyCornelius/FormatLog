#pragma once

// Production-like configuration: color + timestamps + filename:line

#ifndef LOG_LEVEL // Allow compiler override
#define LOG_LEVEL LOG_LEVEL_TRACE
#endif

#define LOG_LEVEL_TEXT_FORMAT LOG_LEVEL_TEXT_FORMAT_SHORT
#define LOG_TIME LOG_TIME_HHMMSSMS
#define LOG_FILENAME LOG_FILENAME_LINENUMBER_ENABLE
#define LOG_COLOR LOG_COLOR_ENABLE
#define LOG_STATIC_BUFFER_SIZE 256
#define LOG_STREAM Serial

#include <FormatLog.h>
