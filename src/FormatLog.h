#pragma once

#include <Arduino.h>
#include <memory>
#include "Config/Settings.h"
#include "FmtLib.h"

#if LOG_STORAGE_ENABLE
#include "Storage/RotatingFileSink.h"
#endif

/**--------------------------------------------------------------------------------------
 * Structs
 *-------------------------------------------------------------------------------------*/

struct SourceLocation
{
    const char *filename = "";
    int line = 0;
    const char *funcname = "";

    constexpr SourceLocation() = default;
    constexpr SourceLocation(const char *filename, int line, const char *funcname)
        : filename{filename}, line{line}, funcname{funcname} {}
};

/**--------------------------------------------------------------------------------------
 * Logger Class
 *-------------------------------------------------------------------------------------*/

class FormatLog
{
    using PanicHandler = void (*)();

private:
    Stream *serial = nullptr;
    LogLevel logLevel = static_cast<LogLevel>(LOG_LEVEL);
    PanicHandler panicHandler = LOG_PANIC_HANDLER;

#if LOG_STORAGE_ENABLE
    std::shared_ptr<IRotatingFileSink> storage;
    LogLevel storageLogLevel = static_cast<LogLevel>(LOG_STORAGE_LEVEL);

    bool shouldLogStorage(LogLevel level)
    {
        return storage && level >= storageLogLevel;
    }
#endif

    bool shouldLog(LogLevel level)
    {
        return serial && level >= logLevel;
    }

    template <typename... Args>
    void log(SourceLocation loc, LogLevel level, fmt::format_string<Args...> format, Args &&...args)
    {
        if (shouldLog(level))
        {
            fmt::basic_memory_buffer<char, LOG_STATIC_BUFFER_SIZE> buffer;
            APPEND_COLOR(buffer, level);
            fmt::format_to(fmt::appender(buffer), LOG_PREAMBLE_FORMAT, LOG_PREAMBLE_ARGS(level, loc.filename, loc.line, loc.funcname));
            fmt::vformat_to(fmt::appender(buffer), format, fmt::make_format_args(args...));
            APPEND_RESET_COLOR(buffer);
            buffer.append(fmt::string_view(LOG_EOL));
            serial->write(reinterpret_cast<const uint8_t *>(buffer.data()), buffer.size());
        }

#if LOG_STORAGE_ENABLE
        if (shouldLogStorage(level))
        {
            fmt::basic_memory_buffer<char, LOG_STATIC_BUFFER_SIZE> buffer;
            fmt::format_to(fmt::appender(buffer), LOG_STORAGE_PREAMBLE_FORMAT, LOG_STORAGE_PREAMBLE_ARGS(level, loc.filename, loc.line, loc.funcname));
            fmt::vformat_to(fmt::appender(buffer), format, fmt::make_format_args(args...));
            buffer.append(fmt::string_view(LOG_EOL));
            storage->write(buffer.data(), buffer.size());
        }
#endif
    }

public:
    FormatLog(Stream *stream = &Serial) : serial(stream) {}

#if LOG_STORAGE_ENABLE
    ~FormatLog()
    {
        clearStorage();
    }

    void clearStorage()
    {
        storage.reset();
    }
#endif

    static FormatLog &instance()
    {
        static FormatLog logger(&LOG_STREAM);
        return logger;
    }

    void setSerial(Stream &stream)
    {
        serial = &stream;
    }

#if LOG_STORAGE_ENABLE
    void setStorage(std::shared_ptr<IRotatingFileSink> sink)
    {
        clearStorage();
        storage = sink;
    }

    void setStorageLogLevel(LogLevel level)
    {
        storageLogLevel = level;
    }

    LogLevel getStorageLogLevel()
    {
        return storageLogLevel;
    }

    void flushStorage()
    {
        if (storage)
            storage->flush();
    }
#endif

    LogLevel getLogLevel()
    {
        return logLevel;
    }

    void setLogLevel(LogLevel level)
    {
        logLevel = level;
    }

    void setPanicHandler(PanicHandler handler)
    {
        panicHandler = handler;
    }

    void flush()
    {
        serial->flush();
    }

    template <typename T>
    void print(const T &message)
    {
        serial->print(message);
    }

    template <typename... Args>
    void print(fmt::format_string<Args...> format, Args &&...args)
    {
        fmt::basic_memory_buffer<char, LOG_STATIC_BUFFER_SIZE> buffer;
        fmt::vformat_to(fmt::appender(buffer), format, fmt::make_format_args(args...));
        serial->write(reinterpret_cast<const uint8_t *>(buffer.data()), buffer.size());
    }

    template <typename T>
    void println(const T &message)
    {
        serial->println(message);
    }

    template <typename... Args>
    void println(fmt::format_string<Args...> format, Args &&...args)
    {
        fmt::basic_memory_buffer<char, LOG_STATIC_BUFFER_SIZE> buffer;
        fmt::vformat_to(fmt::appender(buffer), format, fmt::make_format_args(args...));
        buffer.append(fmt::string_view(LOG_EOL));
        serial->write(reinterpret_cast<const uint8_t *>(buffer.data()), buffer.size());
    }

#if LOG_STORAGE_ENABLE
    template <typename T>
    void printStorage(const T &message)
    {
        printStorage("{}", message);
    }

    template <typename... Args>
    void printStorage(fmt::format_string<Args...> format, Args &&...args)
    {
        if (!storage)
            return;
        fmt::basic_memory_buffer<char, LOG_STATIC_BUFFER_SIZE> buffer;
        fmt::vformat_to(fmt::appender(buffer), format, fmt::make_format_args(args...));
        buffer.append(fmt::string_view(LOG_EOL));
        storage->write(buffer.data(), buffer.size());
    }
#endif

    void assertion(bool condition, const char *file, int line, const char *func, const char *expr, const char *message = "")
    {
        if (condition)
            return;

        fmt::basic_memory_buffer<char, LOG_STATIC_BUFFER_SIZE> buffer;
        APPEND_COLOR(buffer, static_cast<LogLevel>(LOG_LEVEL_ERROR));
        fmt::format_to(fmt::appender(buffer), LOG_PANIC_FORMAT, file, line, func, expr, message);
        APPEND_RESET_COLOR(buffer);
        buffer.append(fmt::string_view(LOG_EOL));
        serial->write(reinterpret_cast<const uint8_t *>(buffer.data()), buffer.size());

        if (panicHandler != nullptr)
            panicHandler();
    }

    template <typename... Args>
    void trace(SourceLocation loc, fmt::format_string<Args...> format, Args &&...args)
    {
        log(loc, LogLevel::TRACE, format, std::forward<Args>(args)...);
    }

    template <typename T>
    void trace(SourceLocation loc, const T &value)
    {
        trace(loc, "{}", value);
    }

    template <typename... Args>
    void info(SourceLocation loc, fmt::format_string<Args...> format, Args &&...args)
    {
        log(loc, LogLevel::INFO, format, std::forward<Args>(args)...);
    }

    template <typename T>
    void info(SourceLocation loc, const T &value)
    {
        info(loc, "{}", value);
    }

    template <typename... Args>
    void debug(SourceLocation loc, fmt::format_string<Args...> format, Args &&...args)
    {
        log(loc, LogLevel::DEBUG, format, std::forward<Args>(args)...);
    }

    template <typename T>
    void debug(SourceLocation loc, const T &value)
    {
        debug(loc, "{}", value);
    }

    template <typename... Args>
    void warn(SourceLocation loc, fmt::format_string<Args...> format, Args &&...args)
    {
        log(loc, LogLevel::WARN, format, std::forward<Args>(args)...);
    }

    template <typename T>
    void warn(SourceLocation loc, const T &value)
    {
        warn(loc, "{}", value);
    }

    template <typename... Args>
    void error(SourceLocation loc, fmt::format_string<Args...> format, Args &&...args)
    {
        log(loc, LogLevel::ERROR, format, std::forward<Args>(args)...);
    }

    template <typename T>
    void error(SourceLocation loc, const T &value)
    {
        error(loc, "{}", value);
    }
};

/**--------------------------------------------------------------------------------------
 * Logger Log Macros
 *-------------------------------------------------------------------------------------*/

#if LOG_LEVEL <= LOG_LEVEL_TRACE || (LOG_STORAGE_ENABLE && LOG_STORAGE_LEVEL <= LOG_LEVEL_TRACE)
#define LOG_TRACE(format, ...) FormatLog::instance().trace(SourceLocation(__FILE__, __LINE__, __FUNCTION__), format, ##__VA_ARGS__)
#else
#define LOG_TRACE(format, ...)
#endif

#if LOG_LEVEL <= LOG_LEVEL_DEBUG || (LOG_STORAGE_ENABLE && LOG_STORAGE_LEVEL <= LOG_LEVEL_DEBUG)
#define LOG_DEBUG(format, ...) FormatLog::instance().debug(SourceLocation(__FILE__, __LINE__, __FUNCTION__), format, ##__VA_ARGS__)
#else
#define LOG_DEBUG(format, ...)
#endif

#if LOG_LEVEL <= LOG_LEVEL_INFO || (LOG_STORAGE_ENABLE && LOG_STORAGE_LEVEL <= LOG_LEVEL_INFO)
#define LOG_INFO(format, ...) FormatLog::instance().info(SourceLocation(__FILE__, __LINE__, __FUNCTION__), format, ##__VA_ARGS__)
#else
#define LOG_INFO(format, ...)
#endif

#if LOG_LEVEL <= LOG_LEVEL_WARN || (LOG_STORAGE_ENABLE && LOG_STORAGE_LEVEL <= LOG_LEVEL_WARN)
#define LOG_WARN(format, ...) FormatLog::instance().warn(SourceLocation(__FILE__, __LINE__, __FUNCTION__), format, ##__VA_ARGS__)
#else
#define LOG_WARN(format, ...)
#endif

#if LOG_LEVEL <= LOG_LEVEL_ERROR || (LOG_STORAGE_ENABLE && LOG_STORAGE_LEVEL <= LOG_LEVEL_ERROR)
#define LOG_ERROR(format, ...) FormatLog::instance().error(SourceLocation(__FILE__, __LINE__, __FUNCTION__), format, ##__VA_ARGS__)
#else
#define LOG_ERROR(format, ...)
#endif

/**--------------------------------------------------------------------------------------
 * Logger Extra Macros
 *-------------------------------------------------------------------------------------*/

#if LOG_LEVEL != LOG_LEVEL_DISABLE
#define LOG_BEGIN(baud) LOG_STREAM.begin(baud)
#define LOG_END() LOG_STREAM.end()
#define LOG_PRINT(format, ...) FormatLog::instance().print(format, ##__VA_ARGS__)
#define LOG_PRINTLN(format, ...) FormatLog::instance().println(format, ##__VA_ARGS__)
#define LOG_FLUSH() FormatLog::instance().flush()
#define LOG_SET_LOG_LEVEL(level) FormatLog::instance().setLogLevel(level)
#define LOG_GET_LOG_LEVEL() FormatLog::instance().getLogLevel()
#else
#define LOG_BEGIN(baud)
#define LOG_END()
#define LOG_PRINT(format, ...)
#define LOG_PRINTLN(format, ...)
#define LOG_FLUSH()
#define LOG_SET_LOG_LEVEL(level)
#define LOG_GET_LOG_LEVEL() LogLevel::DISABLE
#endif

/**--------------------------------------------------------------------------------------
 * Assert Macros
 *-------------------------------------------------------------------------------------*/

#if LOG_ASSERT_ENABLE
#define ASSERT(condition) FormatLog::instance().assertion(!!(condition), __FILE__, __LINE__, __FUNCTION__, #condition)
#define ASSERT_M(condition, msg) FormatLog::instance().assertion(!!(condition), __FILE__, __LINE__, __FUNCTION__, #condition, msg)
#define LOG_SET_PANIC_HANDLER(handler) FormatLog::instance().setPanicHandler(handler)
#else
#define ASSERT(condition)
#define ASSERT_M(condition, msg)
#define LOG_SET_PANIC_HANDLER(handler)
#endif

#if LOG_STORAGE_ENABLE
#define LOG_SET_STORAGE(fs, ...) FormatLog::instance().setStorage(createStorage(fs, ##__VA_ARGS__))
#define LOG_SET_STORAGE_LOG_LEVEL(level) FormatLog::instance().setStorageLogLevel(level)
#define LOG_GET_STORAGE_LOG_LEVEL() FormatLog::instance().getStorageLogLevel()
#define LOG_FLUSH_STORAGE() FormatLog::instance().flushStorage()
#else
#define LOG_SET_STORAGE(fs, ...)
#define LOG_SET_STORAGE_LOG_LEVEL(level)
#define LOG_GET_STORAGE_LOG_LEVEL() LogLevel::DISABLE
#define LOG_FLUSH_STORAGE()
#endif // LOG_STORAGE_ENABLE

/**--------------------------------------------------------------------------------------
 * Global Logger Instance
 *-------------------------------------------------------------------------------------*/

#define FmtLog FormatLog::instance()