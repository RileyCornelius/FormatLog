#pragma once

#include "Config/Options.h"

namespace preamble
{
    const char *logLevelText(LogLevel level, LogLevelTextFormat format = LogLevelTextFormat::FULL);
    const char *formatTime(LogTime format = LogTime::MILLIS);
    const char *formatFilename(const char *file, int line = 0, const char *func = nullptr, LogFilename format = LogFilename::ENABLE);
    const char *colorText(LogLevel level);
}
