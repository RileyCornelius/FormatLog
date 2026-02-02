# Changelog

## [0.6.1] - unreleased

### Added

- `CHECK_OR_RETURN` and `CHECK_OR_RETURN_VALUE` macros for non-fatal condition checks that log and return
- `LOG_CHECK_FORMAT` compile-time setting for customizing CHECK output format
- `LOG_BENCHMARK()` macro for automatic scoped timing that logs elapsed time when the function returns
- `LOG_BENCHMARK_BEGIN(label)` / `LOG_BENCHMARK_END(label)` macros for timing named sections within a scope
- `LOG_STOPWATCH()` macro for manual stopwatch timing with `elapsedMs()` and `elapsedTime()`
- `Stopwatch` class for millisecond-precision timing
- `MicroStopwatch` class for microsecond-precision timing with `elapsedUs()`, `elapsedMs()`, and `elapsedTime()`
- `LOG_BENCHMARK_LOG` compile-time setting to configure which log macro benchmark output uses (default: `LOG_DEBUG`)
- `LOG_BENCHMARK_FORMAT` compile-time setting to customize benchmark output format
- `LOG_BENCHMARK_MICRO_BEGIN(label)` / `LOG_BENCHMARK_MICRO_END(label)` macros for microsecond-precision named sections
- `LOG_BENCHMARK_MICRO_FORMAT` compile-time setting to customize micro benchmark output format
- Benchmark example
- `LOG_PRINT_ENABLE` compile-time setting to disable `LOG_PRINT`/`LOG_PRINTLN` macros (default: 1)

### Changed

- `FmtLog.h` now lists all available settings with defaults and uses `#ifndef` guards for overridability
- `LOG_STORAGE_ENABLE` default changed to `0` (disabled by default)

- `LOG_PRINT`/`LOG_PRINTLN` macros now gated by `LOG_PRINT_ENABLE` instead of `LOG_LEVEL`
- Moved all library types into `fmtlog` namespace (FormatLog, SourceLocation, enums, storage classes, benchmark/stopwatch classes, preamble functions)
- Removed file extension from filename preamble (e.g. `main` instead of `main.cpp`)
- Fixed LittleFS test failures caused by open file descriptors during cleanup


## [0.6.0] - 2026-02-01

### Added

- File storage support with buffered writes and log rotation
- Modular architecture with interfaces (`IFileSink`, `IFileManager`)
- Auto-detection of filesystem type (LittleFS/SPIFFS vs SdFat) using SFINAE traits
- `RotatingFileSink` template class with configurable buffer size
- `Esp32FileManager` for LittleFS/SPIFFS filesystems
- `SdFatFileManager` for SdFat filesystems
- `createStorage` factory function for simplified storage setup
- `setPanicHandler` and `LOG_SET_PANIC_HANDLER` for runtime setting of panic hander
- Compile-time storage settings: `LOG_STORAGE_ENABLE`, `LOG_STORAGE_LEVEL`, `LOG_STORAGE_FILE_PATH`, `LOG_STORAGE_MAX_BUFFER_SIZE`, `LOG_STORAGE_MAX_FILE_SIZE`, `LOG_STORAGE_MAX_FILES`, `LOG_STORAGE_NEW_FILE_ON_BOOT`, `LOG_STORAGE_PREAMBLE_FORMAT`, `LOG_STORAGE_PREAMBLE_ARGS`
- Runtime storage macros: `LOG_SET_STORAGE`, `LOG_SET_STORAGE_LOG_LEVEL`, `LOG_GET_STORAGE_LOG_LEVEL`, `LOG_FLUSH_STORAGE`, `LOG_CLOSE_STORAGE`, `LOG_SET_STORAGE_FILE_PATH`, `LOG_GET_STORAGE_FILE_PATH`
- `printStorage` methods for writing directly to storage without log level filtering
- Serial and storage logging operate independently
- Storage unit tests
- Flush on assertion failure
- Global logger instance macro (`FmtLog`)

### Changed

- Refactored `FormatLog` class to use pointer for serial stream
- Reversed log level numbering (0-5 to 5-0) so higher values = higher severity
- Updated FmtLib dependency to 12.1.0
- `ASSERT_M` merged into `ASSERT`
- Renamed `LOG_HALT_FORMAT`/`LOG_HALT_FUNC` to `LOG_PANIC_FORMAT`/`LOG_PANIC_HANDLER`
- Updated examples

## [0.5.2] - 2025-10-08

### Changed

- Updated assert macro
- Updated examples

### Added

- Unit tests

## [0.5.1] - 2025-08-10

### Changed

- Updated library.json metadata

## [0.5.0] - 2025-08-10

### Added

- Initial public release
- Logging with fmtlib formatting (`LOG_TRACE`, `LOG_DEBUG`, `LOG_INFO`, `LOG_WARN`, `LOG_ERROR`, `LOG_FATAL`)
- Compile-time configuration via macros (`LOG_LEVEL`, `LOG_TIME`, `LOG_COLOR`, etc.)
- Static assert validation for configuration settings
- Customizable preamble with log level, timestamp, filename, and function name
- Local time support for timestamps
- `LOG_ASSERT` macro
- Print and println using fmtlib formatting
- `FmtLog.h` pre-configured include with sensible defaults
- Examples
