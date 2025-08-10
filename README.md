# FormatLog

> A modern, high-performance logging library for Arduino projects

FormatLog brings modern C++ formatting to Arduino development, offering a powerful yet lightweight logging solution with compile-time configuration, multiple output formats, and excellent performance.

## Features

- **Modern Formatting**: Built-in support for [fmtlib](https://github.com/fmtlib/fmt)
- **Multiple Log Levels**: TRACE, DEBUG, INFO, WARN, ERROR with compile time and run time filtering
- **Compile-time Configuration**: Zero-overhead for disabled log levels 
- **Flexible Timestamps**: Microseconds, milliseconds, or formatted time (HH:MM:SS:MS)
- **ANSI Color Support**: Configurable colored output for different log levels
- **Source Location**: Optional filename, line number, and function name logging
- **Stream Support**: Works with any Arduino Stream (Serial, Serial1, etc.)
- **Assert Macros**: Debug assertions with detailed error information
- **Memory Efficient**: Smart buffer management with configurable static buffer size
- **Type Safety**: Compile-time format string validation and type checking
- **Custom Preambles**: Fully customizable log message formatting

## Quick Start

### Basic Usage

```cpp
#include <Arduino.h>
#include "FmtLog.h"

void setup() {
    LOG_BEGIN(9600);
    LOG_INFO("Hello {}", "World!");
}

void loop() {
    LOG_DEBUG("Loop iteration: {}", millis());
    delay(1000);
}
```

## Configuration Options

FormatLog uses compile-time macros for configuration, ensuring zero overhead for disabled features.

### Default Settings

If no configuration is provided, FormatLog uses the following defaults:

```cpp
#define LOG_LEVEL LOG_LEVEL_TRACE              // Debug level and above
#define LOG_TIME LOG_TIME_DISABLE              // No timestamps
#define LOG_COLOR LOG_COLOR_DISABLE            // No color output
#define LOG_LEVEL_TEXT_FORMAT LOG_LEVEL_TEXT_FORMAT_SHORT  // 4 character level names
#define LOG_FILENAME LOG_FILENAME_ENABLE       // Filename only (no line/function)
#define LOG_STATIC_BUFFER_SIZE 128              // 128 byte static buffer
#define LOG_STREAM Serial                      // Default to Serial output
```

These defaults provide a basic logging setup that works out of the box. You can override any of these settings by defining them before including `FormatLog.h`.

### Custom Configuration Example

The recommended way to use this library is to use the pre configured `FmtLog.h` or create a `Log.h` file in your project and define all settings before including `FormatLog.h`. Then include `Log.h` instead of `FormatLog.h` across your project.

```cpp
#pragma once

// Log level filtering
#define LOG_LEVEL LOG_LEVEL_TRACE

// Timestamps before logs
#define LOG_TIME LOG_TIME_HHHHMMSSMS

// Colors based on log level
#define LOG_COLOR LOG_COLOR_ENABLE

// Source location info
#define LOG_FILENAME LOG_FILENAME_LINENUMBER_FUNCTION_ENABLE

// Log level text length
#define LOG_LEVEL_TEXT_FORMAT LOG_LEVEL_TEXT_FORMAT_LETTER

// Size of buffer before dynamic memory is used
#define LOG_STATIC_BUFFER_SIZE 128

// Default serial output
#define LOG_STREAM Serial

// Include the library
#include <FormatLog.h>
```

## Configuration Details

### Log Levels

```cpp
#define LOG_LEVEL LOG_LEVEL_DEBUG
```

Available levels (in order of verbosity):
- `LOG_LEVEL_TRACE` - Most verbose, includes all messages
- `LOG_LEVEL_DEBUG` - Debug information and above
- `LOG_LEVEL_INFO` - General information and above
- `LOG_LEVEL_WARN` - Warnings and errors only
- `LOG_LEVEL_ERROR` - Errors only
- `LOG_LEVEL_DISABLE` - No logging

### Timestamp Configuration

```cpp
#define LOG_TIME LOG_TIME_MILLIS
```

Options:
- `LOG_TIME_DISABLE` - No timestamps
- `LOG_TIME_MICROS` - Microseconds since boot
- `LOG_TIME_MILLIS` - Milliseconds since boot
- `LOG_TIME_HHMMSSMS` - Time since boot format: `HH:MM:SS:MS`
- `LOG_TIME_HHHHMMSSMS` - Time since boot format: `HHHH:MM:SS:MS`
- `LOG_TIME_LOCALTIME` - System local time format: `YYYY-MM-DD HH:MM:SS:MS` can be overridden using `LOG_TIME_LOCALTIME_FORMAT` 

### Level Text Format

```cpp
#define LOG_LEVEL_TEXT_FORMAT LOG_LEVEL_TEXT_FORMAT_SHORT
```

Options:
- `LOG_LEVEL_TEXT_FORMAT_LETTER` - Single letter: `[T]`, `[D]`, `[I]`, `[W]`, `[E]`
- `LOG_LEVEL_TEXT_FORMAT_SHORT` - Short: `[TRAC]`, `[DEBG]`, `[INFO]`, `[WARN]`, `[EROR]`
- `LOG_LEVEL_TEXT_FORMAT_FULL` - Full: `[TRACE]`, `[DEBUG]`, `[INFO]`, `[WARN]`, `[ERROR]`

### Color Support

```cpp
#define LOG_COLOR LOG_COLOR_ENABLE
```

Options:
- `LOG_COLOR_ENABLE` 
- `LOG_COLOR_DISABLE`

Enables ANSI color codes:
- **TRACE**: Bright White
- **DEBUG**: Green  
- **INFO**: Cyan
- **WARN**: Yellow
- **ERROR**: Red

### File Information

```cpp
#define LOG_FILENAME LOG_FILENAME_LINENUMBER_FUNCTION_ENABLE
```

Options:
- `LOG_FILENAME_DISABLE` - No file information
- `LOG_FILENAME_ENABLE` - Filename only
- `LOG_FILENAME_LINENUMBER_ENABLE` - Filename and line number
- `LOG_FILENAME_LINENUMBER_FUNCTION_ENABLE` - Filename, line number, and function name

### Buffer Size

```cpp
#define LOG_STATIC_BUFFER_SIZE 128
```

Sets the static buffer size for log messages. Messages shorter than this size use stack memory, while longer messages dynamically allocate memory as needed.

### Custom Preamble

You can fully customize the log message preamble by defining your own format string and arguments. This allows you to control exactly how timestamps, log levels, and file information appear in your logs.

```cpp
#define LOG_PREAMBLE_FORMAT "[{}][{}] "
#define LOG_PREAMBLE_ARGS(level, filename, linenumber, function) \
    preamble::logLevelText(level, LOG_LEVEL_TEXT_FORMAT), \
    preamble::formatFilename(filename, linenumber, function, LOG_FILENAME)
```

At compile time, the library expands your macro definitions into the actual formatting call:

```cpp
log(LogLevel level, char *filename, int linenumber, char *function){
// Your macro definitions expand to:
fmt::format_to(buffer, "[{}][{}] ", 
    preamble::logLevelText(level, LOG_LEVEL_TEXT_FORMAT), 
    preamble::formatFilename(filename, linenumber, function, LOG_FILENAME));
}
```

This gives you complete control over the log message structure while maintaining the performance benefits of compile-time configuration.

## API Reference

### Logging Macros

```cpp
LOG_TRACE(format, ...)   // Trace level messages
LOG_DEBUG(format, ...)   // Debug level messages  
LOG_INFO(format, ...)    // Info level messages
LOG_WARN(format, ...)    // Warning level messages
LOG_ERROR(format, ...)   // Error level messages
```

### Utility Macros

```cpp
LOG_BEGIN(baud)          // Initialize serial communication
LOG_PRINT(format, ...)   // Print using fmtlib without newline
LOG_PRINTLN(format, ...) // Print using fmtlib with newline
LOG_FLUSH()              // Flush output buffer
LOG_SET_LOG_LEVEL(level) // Change log level at runtime
LOG_GET_LOG_LEVEL()      // Get current log level
```

### Assertion Macros

```cpp
ASSERT(condition)                    // Assert with automatic message
ASSERTM(condition, message)          // Assert with custom message
```

Assertions can be disabled by undefining `NDEBUG`. You can customize the halt behavior by defining a custom `LOG_HALT_FUNC`:

```cpp
void customHalt()
{
    while(true)
    {
        digitalWrite(RED_LED, !digitalRead(RED_LED));
        delay(1000);
    }
}
#define LOG_HALT_FUNC customHalt
```

## Output Examples

```
[I] Entering main loop iteration 1247
[INFO] System initialized at 12567
[  45123][WARN] Low battery: 23%
[00:12:40:456][WARN] Temperature high: 85°C
[WARN][sensors.cpp:45:readTemp()] Temperature sensor timeout
[00:12:35:145][DEBUG][wifi.cpp:32:begin()] Connecting to WiFi: MyNetwork
```

<!-- ## Platform Support

### Tested Platforms
- **ESP32** (all variants)
- **ESP8266**
- **STM32** (with Arduino framework)
- **Raspberry Pi Pico** -->

## Dependencies

### Required
- Arduino Framework
- STL
- [fmt-arduino](https://github.com/RileyCornelius/fmt-arduino)

### Format String Reference

FormatLog uses [fmtlib](https://github.com/fmtlib/fmt) for modern C++ formatting. Here are common formatting patterns:

**Quick Reference**: [fmtlib cheat sheet](https://hackingcpp.com/cpp/libs/fmt.png)

#### Basic Formatting
```cpp
LOG_INFO("Hello {}", "World");           // String substitution
LOG_INFO("Number: {}", 42);              // Integer
LOG_INFO("Float: {:.2f}", 3.14159);      // Float with 2 decimal places
LOG_INFO("Hex: 0x{:X}", 255);            // Hexadecimal (uppercase)
LOG_INFO("Binary: {:b}", 15);            // Binary representation
```

#### Alignment and Width
```cpp
LOG_INFO("Left: '{:<10}'", "text");      // Left-aligned, width 10
LOG_INFO("Right: '{:>10}'", "text");     // Right-aligned, width 10  
LOG_INFO("Center: '{:^10}'", "text");    // Center-aligned, width 10
LOG_INFO("Zero-pad: '{:05d}'", 42);      // Zero-padded numbers
```



## Examples

### [Basic Example](examples/basic/)
Simple logging setup with default configuration.

### [Custom Configuration](examples/custom/)
Advanced configuration with timestamps, colors, and custom preamble.

### [Preamble Example](examples/preamble/)
Demonstrates customizing the log preamble and formatting.

## Development

### TODO:
- [ ] Add file logger
- [ ] Add circlar buffering delayed output
- [x] Add std format and printf support
- [x] Add static asserts
- [x] Add assert macro with setHalt()
- [x] Add local time support
- [x] Add function to preamble setting LOG_FILENAME_LINENUMBER_FUNCTION_ENABLE
- [x] Add String to fmtlib