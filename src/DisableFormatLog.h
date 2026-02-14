#pragma once

/**
 * Include this header AFTER FormatLog.h in a .cpp file to
 * disable all logging output macros in that translation unit only.
 * ASSERT, CHECK_OR_RETURN, and CHECK_OR_RETURN_VALUE remain active.
 */

#undef LOG_TRACE
#define LOG_TRACE(format, ...) ((void)0)

#undef LOG_DEBUG
#define LOG_DEBUG(format, ...) ((void)0)

#undef LOG_INFO
#define LOG_INFO(format, ...) ((void)0)

#undef LOG_WARN
#define LOG_WARN(format, ...) ((void)0)

#undef LOG_ERROR
#define LOG_ERROR(format, ...) ((void)0)

#undef LOG_PRINT
#define LOG_PRINT(format, ...) ((void)0)

#undef LOG_PRINTLN
#define LOG_PRINTLN(format, ...) ((void)0)

#undef LOG_BENCHMARK
#define LOG_BENCHMARK() ((void)0)

#undef LOG_BENCHMARK_BEGIN
#define LOG_BENCHMARK_BEGIN(label) ((void)0)

#undef LOG_BENCHMARK_END
#define LOG_BENCHMARK_END(label) ((void)0)

#undef LOG_BENCHMARK_MICRO_BEGIN
#define LOG_BENCHMARK_MICRO_BEGIN(label) ((void)0)

#undef LOG_BENCHMARK_MICRO_END
#define LOG_BENCHMARK_MICRO_END(label) ((void)0)
