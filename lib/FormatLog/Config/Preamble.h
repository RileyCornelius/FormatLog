#pragma once

namespace preamble
{
    const char *logLevelText(int level, int format);
    const char *formatTime(int format);
    const char *formatFilename(const char *file, int line, const char *func, int format);
    const char *colorText(int level);
}
