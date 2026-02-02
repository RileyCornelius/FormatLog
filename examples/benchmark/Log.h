#pragma once

#define LOG_LEVEL LOG_LEVEL_TRACE
#define LOG_LEVEL_TEXT_FORMAT LOG_LEVEL_TEXT_FORMAT_SHORT
#define LOG_TIME LOG_TIME_MILLIS
#define LOG_COLOR LOG_COLOR_ENABLE
#define LOG_STREAM Serial

#define LOG_BENCHMARK_LOG LOG_DEBUG               // Log macro used for benchmark output
#define LOG_BENCHMARK_FORMAT "[{}] elapsed {} ms" // Format: [label] elapsed {ms} ms

#include <FormatLog.h>
