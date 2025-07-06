#pragma once

#include <Arduino.h>
#include "fmt.h"

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
#define LOG_TIME_ENABLE 2 // Defined for consistency with other options. Same as LOG_TIME_MILLIS.
#define LOG_TIME_MICROS 1
#define LOG_TIME_MILLIS 2
#define LOG_TIME_HHMMSSMS 3
#define LOG_TIME_HHHHMMSSMS 4

#define LOG_COLOR_DISABLE 0
#define LOG_COLOR_ENABLE 1

#define LOG_FILENAME_DISABLE 0
#define LOG_FILENAME_ENABLE 1
#define LOG_FILENAME_LINENUMBER_ENABLE 2

#define LOG_PRINT_TYPE_PRINTF 0
#define LOG_PRINT_TYPE_CUSTOM_FORMAT 1
#define LOG_PRINT_TYPE_STD_FORMAT 2
#define LOG_PRINT_TYPE_FMT_FORMAT 3

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
#define LOG_COLOR LOG_COLOR_ENABLE
#endif

#ifndef LOG_TIME
#define LOG_TIME LOG_TIME_ENABLE
#endif

#ifndef LOG_LEVEL_TEXT_FORMAT
#define LOG_LEVEL_TEXT_FORMAT LOG_LEVEL_TEXT_FORMAT_SHORT
#endif

#ifndef LOG_FILENAME
#define LOG_FILENAME LOG_FILENAME_ENABLE
#endif

#ifndef LOG_PRINT_TYPE
#define LOG_PRINT_TYPE LOG_PRINT_TYPE_FMT_FMTLIB
#endif

#ifndef LOG_PREAMBLE_FORMAT
#define LOG_PREAMBLE_FORMAT DEFAULT_PREAMBLE_FORMAT
#endif

#ifndef LOG_PREAMBLE_ARGS
#define LOG_PREAMBLE_ARGS(level, filename, linenumber, function) DEFAULT_PREAMBLE_ARGS(level, filename, linenumber, function)
#endif

#ifndef LOG_EOL
#define LOG_EOL "\r\n"
#endif

/**--------------------------------------------------------------------------------------
 * Preamble Settings
 *-------------------------------------------------------------------------------------*/

const char *preambleLogLevel(uint8_t level);
const char *formatTime();
const char *filePathToName(const char *path);
const char *filePathToNameNumber(const char *path, int line);

#define FORMATTER "[{}] "
#define PREAMBLE_LOG_LEVEL(level) preambleLogLevel(level)

#if LOG_TIME != LOG_TIME_DISABLE
#define PREAMBLE_TIME_FORMAT FORMATTER
#define PREAMBLE_TIME formatTime(),
#else
#define PREAMBLE_TIME_FORMAT
#define PREAMBLE_TIME
#endif

#if LOG_FILENAME == LOG_FILENAME_ENABLE
#define PREAMBLE_FILENAME_FORMAT FORMATTER
#define PREAMBLE_FILENAME(file, line) filePathToName(file)
#elif LOG_FILENAME == LOG_FILENAME_LINENUMBER_ENABLE
#define PREAMBLE_FILENAME_FORMAT FORMATTER
#define PREAMBLE_FILENAME(file, line) filePathToNameNumber(file, line)
#else
#define PREAMBLE_FILE_NAME_FORMAT
#define PREAMBLE_FILE_NAME(file)
#endif

#define DEFAULT_PREAMBLE_FORMAT (PREAMBLE_TIME_FORMAT FORMATTER PREAMBLE_FILENAME_FORMAT)
#define DEFAULT_PREAMBLE_ARGS(level, filename, linenumber, function) PREAMBLE_TIME PREAMBLE_LOG_LEVEL(level), PREAMBLE_FILENAME(filename, linenumber)

/**--------------------------------------------------------------------------------------
 * Private Settings
 *-------------------------------------------------------------------------------------*/

#if LOG_LEVEL_TEXT_FORMAT == LOG_LEVEL_TEXT_FORMAT_LETTER
#define _LOG_LEVEL_VERBOSE_TEXT "T"
#define _LOG_LEVEL_DEBUG_TEXT "D"
#define _LOG_LEVEL_INFO_TEXT "I"
#define _LOG_LEVEL_WARNING_TEXT "W"
#define _LOG_LEVEL_ERROR_TEXT "E"
#elif LOG_LEVEL_TEXT_FORMAT == LOG_LEVEL_TEXT_FORMAT_SHORT
#define _LOG_LEVEL_VERBOSE_TEXT "TRAC"
#define _LOG_LEVEL_DEBUG_TEXT "DBUG"
#define _LOG_LEVEL_INFO_TEXT "INFO"
#define _LOG_LEVEL_WARNING_TEXT "WARN"
#define _LOG_LEVEL_ERROR_TEXT "EROR"
#else
#define _LOG_LEVEL_VERBOSE_TEXT "TRACE"
#define _LOG_LEVEL_DEBUG_TEXT "DEBUG"
#define _LOG_LEVEL_INFO_TEXT "INFO"
#define _LOG_LEVEL_WARNING_TEXT "WARNING"
#define _LOG_LEVEL_ERROR_TEXT "ERROR"
#endif // LOG_LEVEL_TEXT_FORMAT == LOG_LEVEL_TEXT_FORMAT_LETTER

/**--------------------------------------------------------------------------------------
 * Color Settings
 *-------------------------------------------------------------------------------------*/

#define COLOR_RESET "\e[0m"    // Reset all colors
#define COLOR_TRACE "\e[1;37m" // Bright White
#define COLOR_INFO "\e[1;36m"  // Bright Cyan
#define COLOR_WARN "\e[1;33m"  // Bright Yellow
#define COLOR_ERROR "\e[1;91m" // Bright Red
#define COLOR_DEBUG "\e[1;32m" // Bright Green

#if LOG_COLOR
const char *
getColorForLevel(uint8_t level);
#define ADD_COLOR(buf, level) buf.append(fmt::string_view(getColorForLevel(level)));
#define PRINT_COLOR_RESET COLOR_RESET
#else
#define ADD_COLOR(buf, level)
#define PRINT_COLOR_RESET ""
#endif

/**--------------------------------------------------------------------------------------
 * Logger Class
 *-------------------------------------------------------------------------------------*/

enum LogLevel : uint8_t
{
    TRACE = LOG_LEVEL_TRACE,
    DEBUG = LOG_LEVEL_DEBUG,
    INFO = LOG_LEVEL_INFO,
    WARN = LOG_LEVEL_WARN,
    ERROR = LOG_LEVEL_ERROR,
    OFF = LOG_LEVEL_DISABLE
};

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

class FormatLog
{
private:
    Stream &serial;
    LogLevel logLevel;

    bool shouldLog(LogLevel level)
    {
        return level >= logLevel;
    }

    template <typename... Args>
    void log(SourceLocation loc, LogLevel level, fmt::format_string<Args...> format, Args &&...args)
    {
        if (!shouldLog(level))
            return;

        fmt::basic_memory_buffer<char, LOG_STATIC_BUFFER_SIZE> buffer;
        ADD_COLOR(buffer, level);
        fmt::format_to(fmt::appender(buffer), LOG_PREAMBLE_FORMAT, LOG_PREAMBLE_ARGS(level, loc.filename, loc.line, loc.funcname));
        fmt::format_to(fmt::appender(buffer), format, std::forward<Args>(args)...);
        buffer.append(fmt::string_view(PRINT_COLOR_RESET LOG_EOL));
        serial.write(reinterpret_cast<const uint8_t *>(buffer.data()), buffer.size());
    }

    template <typename... Args>
    void log(LogLevel level, fmt::format_string<Args...> &format, Args &&...args)
    {
        log(SourceLocation(), level, format, std::forward<Args>(args)...);
    }

    template <typename T>
    void log(SourceLocation loc, LogLevel level, T &value)
    {
        if (!shouldLog(level))
            return;

        fmt::basic_memory_buffer<char, LOG_STATIC_BUFFER_SIZE> buffer;
        ADD_COLOR(buffer, level);
        fmt::format_to(fmt::appender(buffer), LOG_PREAMBLE_FORMAT, LOG_PREAMBLE_ARGS(level, loc.filename, loc.line, loc.funcname));
        fmt::format_to(fmt::appender(buffer), "{}", value);
        buffer.append(fmt::string_view(PRINT_COLOR_RESET LOG_EOL));
        serial.write(reinterpret_cast<const uint8_t *>(buffer.data()), buffer.size());
    }

    template <typename T>
    void log(LogLevel level, T &value)
    {
        log(SourceLocation(), level, &value);
    }

public:
    FormatLog(Stream &stream = Serial) : serial(stream)
    {
        logLevel = static_cast<LogLevel>(LOG_LEVEL);
    }

    LogLevel getLogLevel()
    {
        return logLevel;
    }

    void setLogLevel(LogLevel level)
    {
        logLevel = level;
    }

    void setStream(Stream &stream)
    {
        serial = stream;
    }

    template <typename T>
    void print(T message)
    {
        serial.print(message);
    }

    template <typename T>
    void println(T message)
    {
        serial.println(message);
    }

    template <typename... Args>
    void printf(fmt::format_string<Args...> format, Args &&...args)
    {
        fmt::basic_memory_buffer<char, LOG_STATIC_BUFFER_SIZE> buffer;
        fmt::format_to(fmt::appender(buffer), format, std::forward<Args>(args)...);
        serial.write(reinterpret_cast<const uint8_t *>(buffer.data()), buffer.size());
    }

    template <typename... Args>
    void trace(SourceLocation &loc, fmt::format_string<Args...> format, Args &&...args)
    {
        log(loc, LogLevel::TRACE, format, std::forward<Args>(args)...);
    }

    template <typename T>
    void trace(T value)
    {
        log(LogLevel::TRACE, value);
    }

    template <typename... Args>
    void info(SourceLocation loc, fmt::format_string<Args...> format, Args &&...args)
    {
        log(loc, LogLevel::INFO, format, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void debug(SourceLocation loc, fmt::format_string<Args...> format, Args &&...args)
    {
        log(loc, LogLevel::DEBUG, format, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void warn(SourceLocation loc, fmt::format_string<Args...> format, Args &&...args)
    {
        log(loc, LogLevel::WARN, format, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void error(SourceLocation loc, fmt::format_string<Args...> format, Args &&...args)
    {
        log(loc, LogLevel::ERROR, format, std::forward<Args>(args)...);
    }
};

/**--------------------------------------------------------------------------------------
 * Logger Public Macros
 *-------------------------------------------------------------------------------------*/

extern FormatLog Logger;

// #define LOG_TEST(format, ...) Logger.test(SourceLocation(__FILE__, __LINE__, __FUNCTION__), format, ##__VA_ARGS__)

#if LOG_LEVEL <= LOG_LEVEL_TRACE
#define LOG_TRACE(format, ...) Logger.trace(SourceLocation(__FILE__, __LINE__, __FUNCTION__), format, ##__VA_ARGS__)
#else
#define LOG_TRACE(format, ...)
#endif

#if LOG_LEVEL <= LOG_LEVEL_DEBUG
#define LOG_DEBUG(format, ...) Logger.debug(SourceLocation(__FILE__, __LINE__, __FUNCTION__), format, ##__VA_ARGS__)
#else
#define LOG_DEBUG(format, ...)
#endif

#if LOG_LEVEL <= LOG_LEVEL_INFO
#define LOG_INFO(format, ...) Logger.info(SourceLocation(__FILE__, __LINE__, __FUNCTION__), format, ##__VA_ARGS__)
#else
#define LOG_INFO(format, ...)
#endif

#if LOG_LEVEL <= LOG_LEVEL_WARN
#define LOG_WARN(format, ...) Logger.warn(SourceLocation(__FILE__, __LINE__, __FUNCTION__), format, ##__VA_ARGS__)
#else
#define LOG_WARN(format, ...)
#endif

#if LOG_LEVEL <= LOG_LEVEL_ERROR
#define LOG_ERROR(format, ...) Logger.error(SourceLocation(__FILE__, __LINE__, __FUNCTION__), format, ##__VA_ARGS__)
#else
#define LOG_ERROR(format, ...)
#endif

#if LOG_LEVEL != LOG_LEVEL_DISABLE
#define LOG_BEGIN(baud) LOG_OUTPUT.begin(baud)
#define LOG_PRINT(msg) LOG_OUTPUT.print(msg)
#define LOG_PRINTLN(msg) LOG_OUTPUT.println(msg)
#define LOG_SET_STREAM(stream) Logger.setStream(stream)
#define LOG_GET_LOG_LEVEL() Logger.getLogLevel()
#define LOG_SET_LOG_LEVEL(level) Logger.setLogLevel(level)
#else
#define LOG_BEGIN(baud)
#define LOG_PRINT(msg)
#define LOG_PRINTLN(msg)
#define LOG_SET_STREAM(stream)
#define LOG_SET_LOG_LEVEL(level)
#endif
