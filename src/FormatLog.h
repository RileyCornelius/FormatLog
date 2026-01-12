#pragma once

#include <Arduino.h>
#include "Config/Settings.h"
#include "FmtLib.h"

#if LOG_STORAGE_ENABLE
#include "FS.h"
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
    Stream &serial;
    LogLevel logLevel = static_cast<LogLevel>(LOG_LEVEL);
    PanicHandler panicHandler = LOG_PANIC_HANDLER;

#if LOG_STORAGE_ENABLE
    fs::FS *storage = nullptr;
    LogLevel storageLogLevel = static_cast<LogLevel>(LOG_STORAGE_LEVEL);
    const char *storageFilePath = LOG_STORAGE_FILE_PATH;

    bool shouldLogToStorage(LogLevel level)
    {
        return storage != nullptr && level >= storageLogLevel;
    }

    void writeToStorage(const char *data, size_t size)
    {
        if (storage == nullptr)
        {
            error(SourceLocation(__FILE__, __LINE__, __FUNCTION__), "Storage not initialized for logging");
        }

        File file = storage->open(storageFilePath, FILE_APPEND, true);
        if (file)
        {
            size_t written = file.write(reinterpret_cast<const uint8_t *>(data), size);
            file.close();
            if (!(written == size))
            {
                error(SourceLocation(__FILE__, __LINE__, __FUNCTION__), "Failed to write all data to storage log file");
            }
        }
        else
        {
            error(SourceLocation(__FILE__, __LINE__, __FUNCTION__), "Failed to open storage log file for writing");
        }
    }
#endif

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

#if LOG_STORAGE_ENABLE
        if (!shouldLogToStorage(level))
            return;

        writeToStorage(buffer.data(), buffer.size());
#endif
    }

public:
    FormatLog(Stream &stream = Serial) : serial(stream) {}

    static FormatLog &instance()
    {
        static FormatLog logger(LOG_STREAM);
        return logger;
    }

    void setSerial(Stream &stream)
    {
        serial = stream;
    }

#if LOG_STORAGE_ENABLE
    void setStorage(fs::FS &fs)
    {
        storage = &fs;
    }

    void setStorageFilePath(const char *filePath)
    {
        storageFilePath = filePath;
    }

    void setStorageLogLevel(LogLevel level)
    {
        storageLogLevel = level;
    }

    LogLevel getStorageLogLevel()
    {
        return storageLogLevel;
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
        serial.flush();
    }

    template <typename T>
    void print(const T &message)
    {
        serial.print(message);
    }

    template <typename... Args>
    void print(fmt::format_string<Args...> format, Args &&...args)
    {
        fmt::basic_memory_buffer<char, LOG_STATIC_BUFFER_SIZE> buffer;
        fmt::vformat_to(fmt::appender(buffer), format, fmt::make_format_args(args...));
        serial.write(reinterpret_cast<const uint8_t *>(buffer.data()), buffer.size());
    }

    template <typename T>
    void println(const T &message)
    {
        serial.println(message);
    }

    template <typename... Args>
    void println(fmt::format_string<Args...> format, Args &&...args)
    {
        fmt::basic_memory_buffer<char, LOG_STATIC_BUFFER_SIZE> buffer;
        fmt::vformat_to(fmt::appender(buffer), format, fmt::make_format_args(args...));
        buffer.append(fmt::string_view(LOG_EOL));
        serial.write(reinterpret_cast<const uint8_t *>(buffer.data()), buffer.size());
    }

#if LOG_STORAGE_ENABLE
    template <typename T>
    void printToStorage(T &message)
    {
        printToStorage("{}", message);
    }

    template <typename... Args>
    void printToStorage(fmt::format_string<Args...> format, Args &&...args)
    {
        fmt::basic_memory_buffer<char, LOG_STATIC_BUFFER_SIZE> buffer;
        fmt::vformat_to(fmt::appender(buffer), format, fmt::make_format_args(args...));
        buffer.append(fmt::string_view(LOG_EOL));

        writeToStorage(buffer.data(), buffer.size());
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
        serial.write(reinterpret_cast<const uint8_t *>(buffer.data()), buffer.size());

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
#define LOG_GET_LOG_LEVEL() LogLevel::OFF
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

/**--------------------------------------------------------------------------------------
 * Storage Macros
 *-------------------------------------------------------------------------------------*/

#if LOG_STORAGE_ENABLE
#define LOG_SET_STORAGE(storage) FormatLog::instance().setStorage(storage)
#define LOG_SET_STORAGE_FILE(path) FormatLog::instance().setStorageFilePath(path)
#define LOG_SET_STORAGE_LEVEL(level) FormatLog::instance().setStorageLogLevel(level)
#define LOG_GET_STORAGE_LEVEL() FormatLog::instance().getStorageLogLevel()

#else
#define LOG_SET_STORAGE(storage)
#define LOG_SET_STORAGE_FILE(path)
#define LOG_SET_STORAGE_LEVEL(level)
#define LOG_GET_STORAGE_LEVEL() LogLevel::DISABLE
#endif

/**--------------------------------------------------------------------------------------
 * Global Logger Instance
 *-------------------------------------------------------------------------------------*/

#define FmtLog FormatLog::instance()