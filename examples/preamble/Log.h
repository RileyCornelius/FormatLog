#pragma once

#include "Config/Preamble.h"

#ifndef LOG_LEVEL // Incase LOG_LEVEL is defined by the compiler
#define LOG_LEVEL LOG_LEVEL_TRACE
#endif

#define LOG_STREAM Serial

// Custom Preamble - | millis | level | filename:number | {msg}
#define LOG_PREAMBLE_FORMAT "| {} | {} | {} | "
#define LOG_PREAMBLE_ARGS(level, filename, linenumber, function) millis(), fmtlog::logLevelText(level, fmtlog::LogLevelTextFormat::FULL), fmtlog::formatFilename(filename, linenumber, function, fmtlog::LogFilename::LINENUMBER_ENABLE)

#include <FormatLog.h>
