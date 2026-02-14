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
LOG_FILE_ENABLE                         (default: 0)

LOG_BENCHMARK_LOG                       (default: LOG_DEBUG)
LOG_BENCHMARK_FORMAT                    (default: "[{}] elapsed {} ms")
LOG_BENCHMARK_MICRO_FORMAT              (default: "[{}] elapsed {} us")

File storage settings (when LOG_FILE_ENABLE is 1):
   LOG_FILE_LEVEL                       (default: LOG_LEVEL_WARN)
   LOG_FILE_PATH                        (default: "/log.txt")
   LOG_FILE_MAX_BUFFER_SIZE             (default: 4096)
   LOG_FILE_MAX_SIZE                    (default: 102400)
   LOG_FILE_MAX_FILES                   (default: 3, 0 = no rotation)
   LOG_FILE_NEW_ON_BOOT                 (default: 0)
*/

// Serial Settings
#ifndef LOG_LEVEL
#define LOG_LEVEL LOG_LEVEL_TRACE
#endif
#ifndef LOG_LEVEL_TEXT_FORMAT
#define LOG_LEVEL_TEXT_FORMAT LOG_LEVEL_TEXT_FORMAT_SHORT
#endif
#ifndef LOG_TIME
#define LOG_TIME LOG_TIME_ENABLE
#endif
#ifndef LOG_FILENAME
#define LOG_FILENAME LOG_FILENAME_ENABLE
#endif
#ifndef LOG_COLOR
#define LOG_COLOR LOG_COLOR_DISABLE
#endif
#ifndef LOG_STATIC_BUFFER_SIZE
#define LOG_STATIC_BUFFER_SIZE 128
#endif
#ifndef LOG_STREAM
#define LOG_STREAM Serial
#endif

// File storage settings
#ifndef LOG_FILE_ENABLE
#define LOG_FILE_ENABLE 1
#endif
#ifndef LOG_FILE_LEVEL
#define LOG_FILE_LEVEL LOG_LEVEL_WARN
#endif
#ifndef LOG_FILE_PATH
#define LOG_FILE_PATH "/log.txt"
#endif
#ifndef LOG_FILE_MAX_BUFFER_SIZE
#define LOG_FILE_MAX_BUFFER_SIZE 4096
#endif
#ifndef LOG_FILE_MAX_SIZE
#define LOG_FILE_MAX_SIZE 102400
#endif
#ifndef LOG_FILE_MAX_FILES
#define LOG_FILE_MAX_FILES 3
#endif
#ifndef LOG_FILE_NEW_ON_BOOT
#define LOG_FILE_NEW_ON_BOOT 1
#endif

#include <FormatLog.h>
