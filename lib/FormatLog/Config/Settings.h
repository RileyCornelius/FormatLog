#pragma once

#include <Arduino.h>
#include "Config/Constants.h"
#include "Config/Preamble.h"

/**--------------------------------------------------------------------------------------
 * Logger Default Settings
 *-------------------------------------------------------------------------------------*/

#ifndef LOG_STATIC_BUFFER_SIZE
#define LOG_STATIC_BUFFER_SIZE 64
#endif

#ifndef LOG_STREAM
#define LOG_STREAM Serial
#endif

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_DEBUG
#endif

#ifndef LOG_COLOR
#define LOG_COLOR LOG_COLOR_DISABLE
#endif

#ifndef LOG_TIME
#define LOG_TIME LOG_TIME_DISABLE
#endif

#ifndef LOG_LEVEL_TEXT_FORMAT
#define LOG_LEVEL_TEXT_FORMAT LOG_LEVEL_TEXT_FORMAT_FULL
#endif

#ifndef LOG_FILENAME
#define LOG_FILENAME LOG_FILENAME_ENABLE
#endif

#ifndef LOG_PRINT_TYPE
#define LOG_PRINT_TYPE LOG_PRINT_TYPE_FMT_FORMAT
#endif

#ifndef LOG_PREAMBLE_FORMAT
#define LOG_PREAMBLE_FORMAT DEFAULT_PREAMBLE_FORMAT
#endif

#ifndef LOG_PREAMBLE_ARGS
#define LOG_PREAMBLE_ARGS(level, filename, linenumber, function) DEFAULT_PREAMBLE_ARGS(level, filename, linenumber, function)
#endif

#ifndef LOG_HALT_FORMAT
#define LOG_HALT_FORMAT (LOG_EOL "[ASSERT] {}:{} - {}(): ({}) {}") // /r/n[ASSERT] {file}:{line} - {func}(): ({expr}) {message}
#endif

#ifndef LOG_HALT
inline void logHalt()
{
    while (true)
        ;
}
#define LOG_HALT logHalt
#endif

#ifndef LOG_EOL
#define LOG_EOL "\r\n"
#endif

/**--------------------------------------------------------------------------------------
 * Preamble Settings
 *-------------------------------------------------------------------------------------*/

#if LOG_PRINT_TYPE == LOG_PRINT_TYPE_STD_FORMAT || LOG_PRINT_TYPE == LOG_PRINT_TYPE_FMT_FORMAT
#define FORMATTER "[{}]"
#elif LOG_PRINT_TYPE == LOG_PRINT_TYPE_PRINTF
#define FORMATTER "[%s]"
#endif

#if LOG_COLOR
#define APPEND_COLOR(buf, level) buf.append(fmt::string_view(preamble::colorText(level)));
#define APPEND_RESET_COLOR(buf) buf.append(fmt::string_view(COLOR_RESET));
#else
#define APPEND_COLOR(buf, level)
#define APPEND_RESET_COLOR(buf)
#endif

#if LOG_TIME != LOG_TIME_DISABLE
#define PREAMBLE_TIME_FORMAT FORMATTER
#define PREAMBLE_TIME(format) preamble::formatTime(format),
#else
#define PREAMBLE_TIME_FORMAT
#define PREAMBLE_TIME(format)
#endif

#if LOG_FILENAME != LOG_FILENAME_DISABLE
#define PREAMBLE_FILENAME_FORMAT FORMATTER
#define PREAMBLE_FILENAME(file, line, func, format) , preamble::formatFilename(file, line, func, format)
#else
#define PREAMBLE_FILENAME_FORMAT
#define PREAMBLE_FILENAME(file, line, format)
#endif

#define PREAMBLE_LOG_LEVEL(level, format) preamble::logLevelText(level, format)

// Default preamble format and arguments
#define DEFAULT_PREAMBLE_FORMAT (PREAMBLE_TIME_FORMAT FORMATTER PREAMBLE_FILENAME_FORMAT " ")
#define DEFAULT_PREAMBLE_ARGS(level, filename, linenumber, function) PREAMBLE_TIME(LOG_TIME) PREAMBLE_LOG_LEVEL(level, LOG_LEVEL_TEXT_FORMAT) PREAMBLE_FILENAME(filename, linenumber, function, LOG_FILENAME)

/**--------------------------------------------------------------------------------------
 * Static Assertions for Settings Validation
 *-------------------------------------------------------------------------------------*/

static_assert(LOG_LEVEL >= LOG_LEVEL_TRACE && LOG_LEVEL <= LOG_LEVEL_DISABLE,
              "LOG_LEVEL must be between LOG_LEVEL_TRACE and LOG_LEVEL_DISABLE");
static_assert(LOG_LEVEL_TEXT_FORMAT >= LOG_LEVEL_TEXT_FORMAT_LETTER && LOG_LEVEL_TEXT_FORMAT <= LOG_LEVEL_TEXT_FORMAT_FULL,
              "LOG_LEVEL_TEXT_FORMAT must be either LOG_LEVEL_TEXT_FORMAT_LETTER, LOG_LEVEL_TEXT_FORMAT_SHORT or LOG_LEVEL_TEXT_FORMAT_FULL");
static_assert(LOG_TIME >= LOG_TIME_DISABLE && LOG_TIME <= LOG_TIME_LOCALTIME,
              "LOG_TIME must be between LOG_TIME_DISABLE and LOG_TIME_LOCALTIME");
static_assert(LOG_FILENAME >= LOG_FILENAME_DISABLE && LOG_FILENAME <= LOG_FILENAME_LINENUMBER_FUNCTION_ENABLE,
              "LOG_FILENAME must be between LOG_FILENAME_DISABLE and LOG_FILENAME_LINENUMBER_FUNCTION_ENABLE");
static_assert(LOG_COLOR == LOG_COLOR_DISABLE || LOG_COLOR == LOG_COLOR_ENABLE,
              "LOG_COLOR must be either LOG_COLOR_DISABLE or LOG_COLOR_ENABLE");
static_assert(LOG_STATIC_BUFFER_SIZE > 0, "LOG_STATIC_BUFFER_SIZE must be greater than 0");