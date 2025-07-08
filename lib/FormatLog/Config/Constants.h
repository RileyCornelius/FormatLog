#pragma once

/**--------------------------------------------------------------------------------------
 * Logger Options
 *-------------------------------------------------------------------------------------*/

#define LOG_LEVEL_TRACE 0
#define LOG_LEVEL_DEBUG 1
#define LOG_LEVEL_INFO 2
#define LOG_LEVEL_WARN 3
#define LOG_LEVEL_ERROR 4
#define LOG_LEVEL_DISABLE 5

#define LOG_LEVEL_TEXT_FORMAT_LETTER 0
#define LOG_LEVEL_TEXT_FORMAT_SHORT 1
#define LOG_LEVEL_TEXT_FORMAT_FULL 2

#define LOG_TIME_DISABLE 0
#define LOG_TIME_ENABLE 1 // Defined for consistency with other options. Same as LOG_TIME_MILLIS.
#define LOG_TIME_MILLIS 1
#define LOG_TIME_MICROS 2
#define LOG_TIME_HHMMSSMS 3
#define LOG_TIME_HHHHMMSSMS 4

#define LOG_COLOR_DISABLE 0
#define LOG_COLOR_ENABLE 1

#define LOG_FILENAME_DISABLE 0
#define LOG_FILENAME_ENABLE 1
#define LOG_FILENAME_LINENUMBER_ENABLE 2

#define LOG_PRINT_TYPE_FMT_FORMAT 0
#define LOG_PRINT_TYPE_STD_FORMAT 1
#define LOG_PRINT_TYPE_PRINTF 2

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
 * Types
 *-------------------------------------------------------------------------------------*/

enum LogLevel
{
    TRACE = LOG_LEVEL_TRACE,
    DEBUG = LOG_LEVEL_DEBUG,
    INFO = LOG_LEVEL_INFO,
    WARN = LOG_LEVEL_WARN,
    ERROR = LOG_LEVEL_ERROR,
    OFF = LOG_LEVEL_DISABLE
};

// enum LogLevelTextFormat
// {
//     LETTER = LOG_LEVEL_TEXT_FORMAT_LETTER,
//     SHORT = LOG_LEVEL_TEXT_FORMAT_SHORT,
//     FULL = LOG_LEVEL_TEXT_FORMAT_FULL
// };

// enum LogTime
// {
//     TIME_DISABLE = LOG_TIME_DISABLE,
//     TIME_MICROS = LOG_TIME_MICROS,
//     TIME_MILLIS = LOG_TIME_MILLIS,
//     TIME_ENABLE = LOG_TIME_ENABLE,
//     TIME_HHMMSSMS = LOG_TIME_HHMMSSMS,
//     TIME_HHHHMMSSMS = LOG_TIME_HHHHMMSSMS
// };

// enum LogColor
// {
//     COLOR_DISABLE = LOG_COLOR_DISABLE,
//     COLOR_ENABLE = LOG_COLOR_ENABLE
// };

// enum LogFilename
// {
//     FILENAME_DISABLE = LOG_FILENAME_DISABLE,
//     FILENAME_ENABLE = LOG_FILENAME_ENABLE,
//     FILENAME_LINENUMBER_ENABLE = LOG_FILENAME_LINENUMBER_ENABLE
// };

// enum LogPrintType
// {
//     PRINTF = LOG_PRINT_TYPE_PRINTF,
//     STD_FORMAT = LOG_PRINT_TYPE_STD_FORMAT,
//     FMT_FORMAT = LOG_PRINT_TYPE_FMT_FORMAT
// };

struct SourceLocation
{
    const char *filename = "";
    int line = 0;
    const char *funcname = "";

    constexpr SourceLocation() = default;
    constexpr SourceLocation(const char *filename, int line, const char *funcname)
        : filename{filename},
          line{line},
          funcname{funcname} {}
};
