#pragma once

#include <Arduino.h>
#include "fmt.h"
#include "Config/Settings.h"

/**--------------------------------------------------------------------------------------
 * Logger Class
 *-------------------------------------------------------------------------------------*/

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
        APPEND_COLOR(buffer, level);
        fmt::format_to(fmt::appender(buffer), LOG_PREAMBLE_FORMAT, LOG_PREAMBLE_ARGS(level, loc.filename, loc.line, loc.funcname));
        fmt::vformat_to(fmt::appender(buffer), format, fmt::make_format_args(args...));
        APPEND_RESET_COLOR(buffer);
        buffer.append(fmt::string_view(LOG_EOL));
        serial.write(reinterpret_cast<const uint8_t *>(buffer.data()), buffer.size());
    }

    template <typename T>
    void log(SourceLocation loc, LogLevel level, T &value)
    {
        if (!shouldLog(level))
            return;

        fmt::basic_memory_buffer<char, LOG_STATIC_BUFFER_SIZE> buffer;
        APPEND_COLOR(buffer, level);
        fmt::format_to(fmt::appender(buffer), LOG_PREAMBLE_FORMAT, LOG_PREAMBLE_ARGS(level, loc.filename, loc.line, loc.funcname));
        fmt::format_to(fmt::appender(buffer), "{}", value);
        APPEND_RESET_COLOR(buffer);
        buffer.append(fmt::string_view(LOG_EOL));
        serial.write(reinterpret_cast<const uint8_t *>(buffer.data()), buffer.size());
    }

public:
    FormatLog(Stream &stream = LOG_STREAM) : serial(stream)
    {
        logLevel = static_cast<LogLevel>(LOG_LEVEL);
    }

    static FormatLog &instance()
    {
        static FormatLog logger;
        return logger;
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

    void flush()
    {
        serial.flush();
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
        fmt::vformat_to(fmt::appender(buffer), format, fmt::make_format_args(args...));
        serial.write(reinterpret_cast<const uint8_t *>(buffer.data()), buffer.size());
    }

    template <typename... Args>
    void trace(SourceLocation loc, fmt::format_string<Args...> format, Args &&...args)
    {
        log(loc, LogLevel::TRACE, format, std::forward<Args>(args)...);
    }

    template <typename T>
    void trace(SourceLocation loc, T value)
    {
        log(loc, LogLevel::TRACE, value);
    }

    template <typename... Args>
    void info(SourceLocation loc, fmt::format_string<Args...> format, Args &&...args)
    {
        log(loc, LogLevel::INFO, format, std::forward<Args>(args)...);
    }

    template <typename T>
    void info(SourceLocation loc, T value)
    {
        log(loc, LogLevel::INFO, value);
    }

    template <typename... Args>
    void debug(SourceLocation loc, fmt::format_string<Args...> format, Args &&...args)
    {
        log(loc, LogLevel::DEBUG, format, std::forward<Args>(args)...);
    }

    template <typename T>
    void debug(SourceLocation loc, T value)
    {
        log(loc, LogLevel::DEBUG, value);
    }

    template <typename... Args>
    void warn(SourceLocation loc, fmt::format_string<Args...> format, Args &&...args)
    {
        log(loc, LogLevel::WARN, format, std::forward<Args>(args)...);
    }

    template <typename T>
    void warn(SourceLocation loc, T value)
    {
        log(loc, LogLevel::WARN, value);
    }

    template <typename... Args>
    void error(SourceLocation loc, fmt::format_string<Args...> format, Args &&...args)
    {
        log(loc, LogLevel::ERROR, format, std::forward<Args>(args)...);
    }

    template <typename T>
    void error(SourceLocation loc, T value)
    {
        log(loc, LogLevel::ERROR, value);
    }
};

/**--------------------------------------------------------------------------------------
 * Logger Public Macros
 *-------------------------------------------------------------------------------------*/

#if LOG_LEVEL <= LOG_LEVEL_TRACE
#define LOG_TRACE(format, ...) FormatLog::instance().trace(SourceLocation(__FILE__, __LINE__, __FUNCTION__), format, ##__VA_ARGS__)
#else
#define LOG_TRACE(format, ...)
#endif

#if LOG_LEVEL <= LOG_LEVEL_DEBUG
#define LOG_DEBUG(format, ...) FormatLog::instance().debug(SourceLocation(__FILE__, __LINE__, __FUNCTION__), format, ##__VA_ARGS__)
#else
#define LOG_DEBUG(format, ...)
#endif

#if LOG_LEVEL <= LOG_LEVEL_INFO
#define LOG_INFO(format, ...) FormatLog::instance().info(SourceLocation(__FILE__, __LINE__, __FUNCTION__), format, ##__VA_ARGS__)
#else
#define LOG_INFO(format, ...)
#endif

#if LOG_LEVEL <= LOG_LEVEL_WARN
#define LOG_WARN(format, ...) FormatLog::instance().warn(SourceLocation(__FILE__, __LINE__, __FUNCTION__), format, ##__VA_ARGS__)
#else
#define LOG_WARN(format, ...)
#endif

#if LOG_LEVEL <= LOG_LEVEL_ERROR
#define LOG_ERROR(format, ...) FormatLog::instance().error(SourceLocation(__FILE__, __LINE__, __FUNCTION__), format, ##__VA_ARGS__)
#else
#define LOG_ERROR(format, ...)
#endif

#if LOG_LEVEL != LOG_LEVEL_DISABLE
#define LOG_BEGIN(baud) LOG_STREAM.begin(baud)
#define LOG_PRINT(msg) FormatLog::instance().print(msg)
#define LOG_PRINTLN(msg) FormatLog::instance().println(msg)
#define LOG_PRINTF(format, ...) FormatLog::instance().printf(format, ##__VA_ARGS__)
#define LOG_FLUSH() FormatLog::instance().flush()
#define LOG_SET_STREAM(stream) FormatLog::instance().setStream(stream)
#define LOG_GET_LOG_LEVEL() FormatLog::instance().getLogLevel()
#define LOG_SET_LOG_LEVEL(level) FormatLog::instance().setLogLevel(level)
#else
#define LOG_BEGIN(baud)
#define LOG_PRINT(msg)
#define LOG_PRINTLN(msg)
#define LOG_PRINTF(format, ...)
#define LOG_FLUSH()
#define LOG_SET_STREAM(stream)
#define LOG_GET_LOG_LEVEL() LogLevel::OFF
#define LOG_SET_LOG_LEVEL(level)
#endif
