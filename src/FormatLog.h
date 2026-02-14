#pragma once

#include <Arduino.h>
#include <memory>
#include "Config/Settings.h"
#include "Benchmark/Benchmark.h"
#include "fmt.h"

#if LOG_STORAGE_ENABLE
#include "Storage/StorageFactory.h"
#endif

namespace fmtlog
{

    struct SourceLocation
    {
        const char *filename = "";
        int line = 0;
        const char *funcname = "";

        constexpr SourceLocation() = default;
        constexpr SourceLocation(const char *filename, int line, const char *funcname)
            : filename{filename}, line{line}, funcname{funcname} {}
    };

    class FormatLog
    {
        using PanicHandler = void (*)();

    private:
        Stream *serial = nullptr;
        LogLevel logLevel = static_cast<LogLevel>(LOG_LEVEL);
        PanicHandler panicHandler = LOG_PANIC_HANDLER;

#if LOG_STORAGE_ENABLE
        std::shared_ptr<IFileSink> storage;
        LogLevel storageLogLevel = static_cast<LogLevel>(LOG_STORAGE_LEVEL);

        bool shouldLogStorage(LogLevel level)
        {
            return storage && level <= storageLogLevel;
        }
#endif

        bool shouldLog(LogLevel level)
        {
            return serial && level <= logLevel;
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
        void setStorage(std::shared_ptr<IFileSink> sink)
        {
            storage.reset();
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

        void closeStorage()
        {
            if (storage)
                storage->close();
        }

        void setStorageFilePath(const char *path)
        {
            if (storage)
                storage->setFilePath(path);
        }

        std::string getStorageFilePath() const
        {
            if (storage)
                return storage->getFilePath();
            return "";
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

        void checkedLog(const char *expr, const char *message = "")
        {
            fmt::basic_memory_buffer<char, LOG_STATIC_BUFFER_SIZE> buffer;
            APPEND_COLOR(buffer, static_cast<LogLevel>(LOG_LEVEL_WARN));
            fmt::format_to(fmt::appender(buffer), LOG_CHECK_FORMAT, expr, message);
            APPEND_RESET_COLOR(buffer);
            buffer.append(fmt::string_view(LOG_EOL));
            serial->write(reinterpret_cast<const uint8_t *>(buffer.data()), buffer.size());
        }

        void assertionLog(const char *file, int line, const char *func, const char *expr, const char *message = "")
        {
            fmt::basic_memory_buffer<char, LOG_STATIC_BUFFER_SIZE> buffer;
            APPEND_COLOR(buffer, static_cast<LogLevel>(LOG_LEVEL_ERROR));
            fmt::format_to(fmt::appender(buffer), LOG_PANIC_FORMAT, file, line, func, expr, message);
            APPEND_RESET_COLOR(buffer);
            buffer.append(fmt::string_view(LOG_EOL));
            serial->write(reinterpret_cast<const uint8_t *>(buffer.data()), buffer.size());

            flush();
#if LOG_STORAGE_ENABLE
            flushStorage();
#endif
        }

        void assertion(bool condition, const char *file, int line, const char *func, const char *expr, const char *message = "")
        {
            if (condition)
                return;

            assertionLog(file, line, func, expr, message);

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

} // namespace fmtlog

/**------------------------------------------------------------------------------
 * Logger Log Macros
 *-------------------------------------------------------------------------------------*/

#if LOG_LEVEL >= LOG_LEVEL_TRACE || (LOG_STORAGE_ENABLE && LOG_STORAGE_LEVEL >= LOG_LEVEL_TRACE)
#define LOG_TRACE(format, ...) fmtlog::FormatLog::instance().trace(fmtlog::SourceLocation(__FILE__, __LINE__, __FUNCTION__), format, ##__VA_ARGS__)
#else
#define LOG_TRACE(format, ...) ((void)0)
#endif

#if LOG_LEVEL >= LOG_LEVEL_DEBUG || (LOG_STORAGE_ENABLE && LOG_STORAGE_LEVEL >= LOG_LEVEL_DEBUG)
#define LOG_DEBUG(format, ...) fmtlog::FormatLog::instance().debug(fmtlog::SourceLocation(__FILE__, __LINE__, __FUNCTION__), format, ##__VA_ARGS__)
#else
#define LOG_DEBUG(format, ...) ((void)0)
#endif

#if LOG_LEVEL >= LOG_LEVEL_INFO || (LOG_STORAGE_ENABLE && LOG_STORAGE_LEVEL >= LOG_LEVEL_INFO)
#define LOG_INFO(format, ...) fmtlog::FormatLog::instance().info(fmtlog::SourceLocation(__FILE__, __LINE__, __FUNCTION__), format, ##__VA_ARGS__)
#else
#define LOG_INFO(format, ...) ((void)0)
#endif

#if LOG_LEVEL >= LOG_LEVEL_WARN || (LOG_STORAGE_ENABLE && LOG_STORAGE_LEVEL >= LOG_LEVEL_WARN)
#define LOG_WARN(format, ...) fmtlog::FormatLog::instance().warn(fmtlog::SourceLocation(__FILE__, __LINE__, __FUNCTION__), format, ##__VA_ARGS__)
#else
#define LOG_WARN(format, ...) ((void)0)
#endif

#if LOG_LEVEL >= LOG_LEVEL_ERROR || (LOG_STORAGE_ENABLE && LOG_STORAGE_LEVEL >= LOG_LEVEL_ERROR)
#define LOG_ERROR(format, ...) fmtlog::FormatLog::instance().error(fmtlog::SourceLocation(__FILE__, __LINE__, __FUNCTION__), format, ##__VA_ARGS__)
#else
#define LOG_ERROR(format, ...) ((void)0)
#endif

/**--------------------------------------------------------------------------------------
 * Logger Extra Macros
 *-------------------------------------------------------------------------------------*/

#if LOG_LEVEL != LOG_LEVEL_DISABLE
#define LOG_BEGIN(baud) LOG_STREAM.begin(baud)
#define LOG_END() LOG_STREAM.end()
#define LOG_FLUSH() fmtlog::FormatLog::instance().flush()
#define LOG_SET_LOG_LEVEL(level) fmtlog::FormatLog::instance().setLogLevel(level)
#define LOG_GET_LOG_LEVEL() fmtlog::FormatLog::instance().getLogLevel()
#else
#define LOG_BEGIN(baud) ((void)0)
#define LOG_END() ((void)0)
#define LOG_FLUSH() ((void)0)
#define LOG_SET_LOG_LEVEL(level) ((void)0)
#define LOG_GET_LOG_LEVEL() fmtlog::LogLevel::DISABLE
#endif

#if LOG_PRINT_ENABLE
#define LOG_PRINT(format, ...) fmtlog::FormatLog::instance().print(format, ##__VA_ARGS__)
#define LOG_PRINTLN(format, ...) fmtlog::FormatLog::instance().println(format, ##__VA_ARGS__)
#else
#define LOG_PRINT(format, ...) ((void)0)
#define LOG_PRINTLN(format, ...) ((void)0)
#endif

/**--------------------------------------------------------------------------------------
 * Benchmark
 *-------------------------------------------------------------------------------------*/

#ifndef _LOG_CONCAT
#define _LOG_CONCAT(x, y) _LOG_CONCAT_IMPL(x, y)
#define _LOG_CONCAT_IMPL(x, y) x##y
#endif

inline void _logBenchmarkCallback(const char *label, uint32_t elapsedMs)
{
    LOG_BENCHMARK_LOG(LOG_BENCHMARK_FORMAT, label, elapsedMs);
}

/**
 * @brief Logs elapsed time when the current scope exits.
 * Uses the enclosing function name as the tag.
 */
#define LOG_BENCHMARK() fmtlog::ScopedBenchmark _scoped_bench_(__FUNCTION__, _logBenchmarkCallback)

/**
 * @brief Starts a named benchmark timer. Pair with LOG_BENCHMARK_END().
 * Multiple BEGIN/END pairs are supported per scope using different labels.
 *
 * @param label Label must be a symbol (not a string)
 */
#define LOG_BENCHMARK_BEGIN(label) _LOG_BENCHMARK_BEGIN(_LOG_CONCAT(_bench_, label), #label)
#define _LOG_BENCHMARK_BEGIN(var, label) fmtlog::Benchmark var(label);

/**
 * @brief Logs elapsed time since the matching LOG_BENCHMARK_BEGIN().
 * @param label Symbol matching the LOG_BENCHMARK_BEGIN() call
 */
#define LOG_BENCHMARK_END(label) _LOG_BENCHMARK_END(_LOG_CONCAT(_bench_, label))
#define _LOG_BENCHMARK_END(var) LOG_BENCHMARK_LOG(LOG_BENCHMARK_FORMAT, var.label(), var.elapsedMs())

/**
 * @brief Starts a named microsecond benchmark timer. Pair with LOG_BENCHMARK_MICRO_END().
 * Multiple BEGIN/END pairs are supported per scope using different labels.
 *
 * @param label Label must be a symbol (not a string)
 */
#define LOG_BENCHMARK_MICRO_BEGIN(label) _LOG_BENCHMARK_MICRO_BEGIN(_LOG_CONCAT(_ubench_, label), #label)
#define _LOG_BENCHMARK_MICRO_BEGIN(var, label) fmtlog::MicroBenchmark var(label);

/**
 * @brief Logs elapsed microseconds since the matching LOG_BENCHMARK_MICRO_BEGIN().
 * @param label Symbol matching the LOG_BENCHMARK_MICRO_BEGIN() call
 */
#define LOG_BENCHMARK_MICRO_END(label) _LOG_BENCHMARK_MICRO_END(_LOG_CONCAT(_ubench_, label))
#define _LOG_BENCHMARK_MICRO_END(var) LOG_BENCHMARK_LOG(LOG_BENCHMARK_MICRO_FORMAT, var.label(), var.elapsedUs())

/**
 * @brief Creates a Stopwatch instance for manual timing.
 * Use elapsedMs() for milliseconds or elapsedTime() for HH:MM:SS:MS format.
 */
#define LOG_CREATE_STOPWATCH() fmtlog::Stopwatch()

/**--------------------------------------------------------------------------------------
 * Assert Macros
 *-------------------------------------------------------------------------------------*/

#if LOG_ASSERT_ENABLE
/**
 * @brief Asserts a condition and calls the panic handler if the assertion fails.
 *
 * @param condition Condition to assert (true = pass, false = fail)
 * @param message (Optional) message to log on assertion failure
 */
#define ASSERT(condition, ...) fmtlog::FormatLog::instance().assertion(!!(condition), __FILE__, __LINE__, __FUNCTION__, #condition, ##__VA_ARGS__)
/**
 * @brief Checks a condition and returns from the calling function if it fails.
 *
 * @param condition Condition to check (true = pass, false = fail)
 * @param message (Optional) message to log on failure
 */
#define CHECK_OR_RETURN(condition, ...)                                          \
    {                                                                            \
        if (!(condition))                                                        \
        {                                                                        \
            fmtlog::FormatLog::instance().checkedLog(#condition, ##__VA_ARGS__); \
            return;                                                              \
        }                                                                        \
    }
/**
 * @brief Checks a condition and returns a value from the calling function if it fails.
 *
 * @param condition Condition to check (true = pass, false = fail)
 * @param value Value to return on failure
 * @param message (Optional) message to log on failure
 */
#define CHECK_OR_RETURN_VALUE(condition, value, ...)                             \
    {                                                                            \
        if (!(condition))                                                        \
        {                                                                        \
            fmtlog::FormatLog::instance().checkedLog(#condition, ##__VA_ARGS__); \
            return (value);                                                      \
        }                                                                        \
    }

#define LOG_SET_PANIC_HANDLER(handler) fmtlog::FormatLog::instance().setPanicHandler(handler)
#else
#define ASSERT(condition, ...) ((void)0)
#define CHECK_OR_RETURN(condition, ...) \
    {                                   \
        if (!(condition))               \
            return;                     \
    }
#define CHECK_OR_RETURN_VALUE(condition, value, ...) \
    {                                                \
        if (!(condition))                            \
            return (value);                          \
    }
#define LOG_SET_PANIC_HANDLER(handler) ((void)0)
#endif

#if LOG_STORAGE_ENABLE
/**
 * @brief Sets up log storage with a rotating file sink. Use SPIFFS, LittleFS, SD, or SdFat.
 *
 * @param fs Reference to the file system
 * @param filePath (Optional) Path to the log file
 * @param maxFiles (Optional) Maximum number of rotated files to keep (eg. "3" keeps .1, .2, .3, main file)
 * @param maxFileSize (Optional) Maximum size of each log file before rotation
 * @param rotateOnInit (Optional) Whether to rotate the existing log file on initialization
 */
#define LOG_SET_STORAGE(fs, ...) fmtlog::FormatLog::instance().setStorage(fmtlog::createRotatingStorage(fs, ##__VA_ARGS__))
/**
 * @brief Sets the minimum log level for storage output.
 * Messages below this level will not be written to the log file.
 *
 * @param level LogLevel to set (e.g. fmtlog::LogLevel::WARN)
 */
#define LOG_SET_STORAGE_LOG_LEVEL(level) fmtlog::FormatLog::instance().setStorageLogLevel(level)
/**
 * @brief Gets the current storage log level.
 *
 * @return Current LogLevel used for storage filtering
 */
#define LOG_GET_STORAGE_LOG_LEVEL() fmtlog::FormatLog::instance().getStorageLogLevel()
/**
 * @brief Flushes the storage write buffer to the log file.
 * Call this to ensure all buffered log data is written to disk.
 */
#define LOG_FLUSH_STORAGE() fmtlog::FormatLog::instance().flushStorage()
/**
 * @brief Closes the storage log file.
 * Flushes any remaining data and releases the file handle.
 */
#define LOG_CLOSE_STORAGE() fmtlog::FormatLog::instance().closeStorage()
/**
 * @brief Changes the storage log file path at runtime.
 *
 * @param path New file path for the log file (e.g. "/logs/app.txt")
 */
#define LOG_SET_STORAGE_FILE_PATH(path) fmtlog::FormatLog::instance().setStorageFilePath(path)
/**
 * @brief Gets the current storage log file path.
 *
 * @return std::string containing the current log file path
 */
#define LOG_GET_STORAGE_FILE_PATH() fmtlog::FormatLog::instance().getStorageFilePath()
#else
#define LOG_SET_STORAGE(fs, ...) ((void)0)
#define LOG_SET_STORAGE_LOG_LEVEL(level) ((void)0)
#define LOG_GET_STORAGE_LOG_LEVEL() fmtlog::LogLevel::DISABLE
#define LOG_FLUSH_STORAGE() ((void)0)
#define LOG_CLOSE_STORAGE() ((void)0)
#define LOG_SET_STORAGE_FILE_PATH(path) ((void)0)
#define LOG_GET_STORAGE_FILE_PATH() std::string("")
#endif // LOG_STORAGE_ENABLE

/**--------------------------------------------------------------------------------------
 * Global Logger Instance
 *-------------------------------------------------------------------------------------*/

#define FmtLog fmtlog::FormatLog::instance()