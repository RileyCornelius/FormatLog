#pragma once

/**--------------------------------------------------------------------------------------
 * Logger Options
 *-------------------------------------------------------------------------------------*/

#define LOG_LEVEL_DISABLE 0
#define LOG_LEVEL_ERROR 1
#define LOG_LEVEL_WARN 2
#define LOG_LEVEL_INFO 3
#define LOG_LEVEL_DEBUG 4
#define LOG_LEVEL_TRACE 5

#define LOG_LEVEL_TEXT_FORMAT_LETTER 0
#define LOG_LEVEL_TEXT_FORMAT_SHORT 1
#define LOG_LEVEL_TEXT_FORMAT_FULL 2

#define LOG_TIME_DISABLE 0
#define LOG_TIME_ENABLE 1 // Defined for consistency with other options. Same as LOG_TIME_MILLIS.
#define LOG_TIME_MILLIS 1
#define LOG_TIME_MICROS 2
#define LOG_TIME_HHMMSSMS 3
#define LOG_TIME_HHHHMMSSMS 4
#define LOG_TIME_LOCALTIME 5

#define LOG_COLOR_DISABLE 0
#define LOG_COLOR_ENABLE 1

#define LOG_FILENAME_DISABLE 0
#define LOG_FILENAME_ENABLE 1
#define LOG_FILENAME_LINENUMBER_ENABLE 2
#define LOG_FILENAME_LINENUMBER_FUNCTION_ENABLE 3

/**--------------------------------------------------------------------------------------
 * ANSI Colors
 *-------------------------------------------------------------------------------------*/

#define COLOR_RESET "\e[0m"    // Reset all colors
#define COLOR_TRACE "\e[1;37m" // White
#define COLOR_DEBUG "\e[1;32m" // Green
#define COLOR_INFO "\e[1;36m"  // Cyan
#define COLOR_WARN "\e[1;33m"  // Yellow
#define COLOR_ERROR "\e[1;91m" // Red

/**--------------------------------------------------------------------------------------
 * Enums
 *-------------------------------------------------------------------------------------*/

namespace fmtlog
{

    enum class LogLevel
    {
        TRACE = LOG_LEVEL_TRACE,
        DEBUG = LOG_LEVEL_DEBUG,
        INFO = LOG_LEVEL_INFO,
        WARN = LOG_LEVEL_WARN,
        ERROR = LOG_LEVEL_ERROR,
        DISABLE = LOG_LEVEL_DISABLE
    };

    enum class LogLevelTextFormat
    {
        LETTER = LOG_LEVEL_TEXT_FORMAT_LETTER,
        SHORT = LOG_LEVEL_TEXT_FORMAT_SHORT,
        FULL = LOG_LEVEL_TEXT_FORMAT_FULL
    };

    enum class LogTime
    {
        DISABLE = LOG_TIME_DISABLE,
        ENABLE = LOG_TIME_ENABLE,
        MILLIS = LOG_TIME_MILLIS,
        MICROS = LOG_TIME_MICROS,
        HHMMSSMS = LOG_TIME_HHMMSSMS,
        HHHHMMSSMS = LOG_TIME_HHHHMMSSMS,
        LOCALTIME = LOG_TIME_LOCALTIME
    };

    enum class LogFilename
    {
        DISABLE = LOG_FILENAME_DISABLE,
        ENABLE = LOG_FILENAME_ENABLE,
        LINENUMBER_ENABLE = LOG_FILENAME_LINENUMBER_ENABLE,
        LINENUMBER_FUNCTION_ENABLE = LOG_FILENAME_LINENUMBER_FUNCTION_ENABLE
    };

} // namespace fmtlog
