#pragma once

#include <Arduino.h>
#include "Config/Options.h"
#include "Config/Preamble.h"

/**--------------------------------------------------------------------------------------
 * Logger Default Settings
 *-------------------------------------------------------------------------------------*/

#ifndef LOG_STATIC_BUFFER_SIZE
#define LOG_STATIC_BUFFER_SIZE 128
#endif

#ifndef LOG_STREAM
#define LOG_STREAM Serial
#endif

#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_TRACE
#endif

#ifndef LOG_COLOR
#define LOG_COLOR LOG_COLOR_DISABLE
#endif

#ifndef LOG_TIME
#define LOG_TIME LOG_TIME_DISABLE
#endif

#ifndef LOG_LEVEL_TEXT_FORMAT
#define LOG_LEVEL_TEXT_FORMAT LOG_LEVEL_TEXT_FORMAT_SHORT
#endif

#ifndef LOG_FILENAME
#define LOG_FILENAME LOG_FILENAME_ENABLE
#endif

#ifndef LOG_PREAMBLE_FORMAT
#define LOG_PREAMBLE_FORMAT DEFAULT_PREAMBLE_FORMAT
#endif

#ifndef LOG_PREAMBLE_ARGS
#define LOG_PREAMBLE_ARGS(level, filename, linenumber, function) DEFAULT_PREAMBLE_ARGS(level, filename, linenumber, function)
#endif

#ifndef LOG_ASSERT_ENABLE
#define LOG_ASSERT_ENABLE 1
#endif

#ifndef LOG_PANIC_FORMAT
#define LOG_PANIC_FORMAT (LOG_EOL "[ASSERT] {}:{} - {}(): ({}) {}") // /r/n[ASSERT] {file}:{line} - {func}(): ({expr}) {message}
#endif

#ifndef LOG_PANIC_HANDLER
inline void _logPanic()
{
    while (true)
        ;
}
#define LOG_PANIC_HANDLER _logPanic
#endif

#ifndef LOG_EOL
#define LOG_EOL "\r\n"
#endif

#ifndef LOG_FORMATTER
#define LOG_FORMATTER "[{}]"
#endif

#ifndef LOG_STORAGE_ENABLE
#define LOG_STORAGE_ENABLE 1
#endif

#if LOG_STORAGE_ENABLE

#ifndef LOG_STORAGE_LEVEL
#define LOG_STORAGE_LEVEL LOG_LEVEL_WARN
#endif

#ifndef LOG_STORAGE_FILE_PATH
#define LOG_STORAGE_FILE_PATH "/log.txt"
#endif

#ifndef LOG_STORAGE_MAX_BUFFER_MESSAGES
#define LOG_STORAGE_MAX_BUFFER_MESSAGES 32
#endif

#ifndef LOG_STORAGE_MAX_BUFFER_SIZE
#define LOG_STORAGE_MAX_BUFFER_SIZE 4096
#endif

#ifndef LOG_STORAGE_MAX_FILE_SIZE
#define LOG_STORAGE_MAX_FILE_SIZE 102400 // 100KB
#endif

#ifndef LOG_STORAGE_MAX_FILES
#define LOG_STORAGE_MAX_FILES 3 // Number of rotated log files to keep. Set to 0 to disable rotation (logging stops at MAX_FILE_SIZE)
#endif

#ifndef LOG_STORAGE_FILESYSTEM
#include <SPIFFS.h>
#define LOG_STORAGE_FILESYSTEM SPIFFS
#endif

#endif // LOG_STORAGE_ENABLE

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

/**--------------------------------------------------------------------------------------
 * Preamble Settings
 *-------------------------------------------------------------------------------------*/

#if LOG_COLOR
#define APPEND_COLOR(buf, level) buf.append(fmt::string_view(preamble::colorText(level)));
#define APPEND_RESET_COLOR(buf) buf.append(fmt::string_view(COLOR_RESET));
#else
#define APPEND_COLOR(buf, level)
#define APPEND_RESET_COLOR(buf)
#endif

#if LOG_TIME != LOG_TIME_DISABLE
#define PREAMBLE_TIME_FORMAT LOG_FORMATTER
#define PREAMBLE_TIME(format) preamble::formatTime(static_cast<LogTime>(format)),
#else
#define PREAMBLE_TIME_FORMAT
#define PREAMBLE_TIME(format)
#endif

#if LOG_FILENAME != LOG_FILENAME_DISABLE
#define PREAMBLE_FILENAME_FORMAT LOG_FORMATTER
#define PREAMBLE_FILENAME(file, line, func, format) , preamble::formatFilename(file, line, func, static_cast<LogFilename>(format))
#else
#define PREAMBLE_FILENAME_FORMAT
#define PREAMBLE_FILENAME(file, line, func, format)
#endif

#define PREAMBLE_LOG_LEVEL(level, format) preamble::logLevelText(level, static_cast<LogLevelTextFormat>(format))

// Default preamble format and arguments

#define DEFAULT_PREAMBLE_FORMAT (PREAMBLE_TIME_FORMAT LOG_FORMATTER PREAMBLE_FILENAME_FORMAT " ")
#define DEFAULT_PREAMBLE_ARGS(level, filename, linenumber, function) PREAMBLE_TIME(LOG_TIME) PREAMBLE_LOG_LEVEL(level, LOG_LEVEL_TEXT_FORMAT) PREAMBLE_FILENAME(filename, linenumber, function, LOG_FILENAME)