#pragma once

#include "Config/Preamble.h"

#ifndef LOG_LEVEL // Incase LOG_LEVEL is defined by the compiler
#define LOG_LEVEL LOG_LEVEL_TRACE
#endif

#define LOG_STREAM Serial

// Custom Preamble - |millis|level|function| {msg}
#define LOG_PREAMBLE_FORMAT "|{}|{}|{}| "
#define LOG_PREAMBLE_ARGS(level, filename, linenumber, function) millis(), preamble::logLevelText(level, (LogLevelTextFormat)LOG_LEVEL_TEXT_FORMAT_SHORT), preamble::formatFilename(filename, linenumber, function, (LogFilename)LOG_FILENAME_ENABLE)

#include <FormatLog.h>
