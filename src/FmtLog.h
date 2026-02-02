#pragma once

/*
LOG_LEVEL                               (default: LOG_LEVEL_TRACE)
   LOG_LEVEL_TRACE
   LOG_LEVEL_DEBUG
   LOG_LEVEL_INFO
   LOG_LEVEL_WARN
   LOG_LEVEL_ERROR
   LOG_LEVEL_DISABLE

LOG_LEVEL_TEXT_FORMAT                    (default: LOG_LEVEL_TEXT_FORMAT_SHORT)
   LOG_LEVEL_TEXT_FORMAT_LETTER          - [T] [D] [I] [W] [E]
   LOG_LEVEL_TEXT_FORMAT_SHORT           - [TRAC] [DEBG] [INFO] [WARN] [EROR]
   LOG_LEVEL_TEXT_FORMAT_FULL            - [TRACE] [DEBUG] [INFO] [WARN] [ERROR]

LOG_TIME                                (default: LOG_TIME_DISABLE)
   LOG_TIME_DISABLE
   LOG_TIME_MILLIS                      - Milliseconds since boot
   LOG_TIME_MICROS                      - Microseconds since boot
   LOG_TIME_HHMMSSMS                    - HH:MM:SS:MS
   LOG_TIME_HHHHMMSSMS                  - HHHH:MM:SS:MS
   LOG_TIME_LOCALTIME                   - YYYY-MM-DD HH:MM:SS:MS

LOG_FILENAME                            (default: LOG_FILENAME_ENABLE)
   LOG_FILENAME_DISABLE
   LOG_FILENAME_ENABLE                  - Filename only (without extension)
   LOG_FILENAME_LINENUMBER_ENABLE       - Filename and line number
   LOG_FILENAME_LINENUMBER_FUNCTION_ENABLE - Filename, line number, and function name

LOG_COLOR                               (default: LOG_COLOR_DISABLE)
   LOG_COLOR_DISABLE
   LOG_COLOR_ENABLE

LOG_STATIC_BUFFER_SIZE                  (default: 128)
LOG_STREAM                              (default: Serial)
LOG_PRINT_ENABLE                        (default: 1)
LOG_ASSERT_ENABLE                       (default: 1)
LOG_STORAGE_ENABLE                      (default: 0)

LOG_BENCHMARK_LOG                       (default: LOG_DEBUG)
LOG_BENCHMARK_FORMAT                    (default: "[{}] elapsed {} ms")
LOG_BENCHMARK_MICRO_FORMAT              (default: "[{}] elapsed {} us")

Storage settings (when LOG_STORAGE_ENABLE is 1):
   LOG_STORAGE_LEVEL                    (default: LOG_LEVEL_WARN)
   LOG_STORAGE_FILE_PATH                (default: "/log.txt")
   LOG_STORAGE_MAX_BUFFER_SIZE          (default: 4096)
   LOG_STORAGE_MAX_FILE_SIZE            (default: 102400)
   LOG_STORAGE_MAX_FILES                (default: 3, 0 = no rotation)
   LOG_STORAGE_NEW_FILE_ON_BOOT         (default: 0)
*/

// Default Logger Settings (can be overridden in project settings)
#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_TRACE
#endif

// Serial Settings
#define LOG_LEVEL_TEXT_FORMAT LOG_LEVEL_TEXT_FORMAT_SHORT
#define LOG_TIME LOG_TIME_DISABLE
#define LOG_FILENAME LOG_FILENAME_ENABLE
#define LOG_COLOR LOG_COLOR_DISABLE
#define LOG_STATIC_BUFFER_SIZE 128
#define LOG_STREAM Serial

// Storage Settings
#define LOG_STORAGE_ENABLE 1
#define LOG_STORAGE_LEVEL LOG_LEVEL_WARN
#define LOG_STORAGE_FILE_PATH "/log.txt"
#define LOG_STORAGE_MAX_BUFFER_SIZE 4096
#define LOG_STORAGE_MAX_FILE_SIZE 1024
#define LOG_STORAGE_MAX_FILES 3
#define LOG_STORAGE_NEW_FILE_ON_BOOT 0

#include <FormatLog.h>
