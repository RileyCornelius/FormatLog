#include "Log.h"

FormatLog Logger(LOG_STREAM);

const char *preambleLogLevel(uint8_t level)
{
    static const char *logLevelNames[] = {
        _LOG_LEVEL_VERBOSE_TEXT,
        _LOG_LEVEL_DEBUG_TEXT,
        _LOG_LEVEL_INFO_TEXT,
        _LOG_LEVEL_WARNING_TEXT,
        _LOG_LEVEL_ERROR_TEXT,
    };
    return logLevelNames[level];
}

const char *getColorForLevel(uint8_t level)
{
    switch (level)
    {
    case LogLevel::TRACE:
        return COLOR_TRACE;
    case LogLevel::DEBUG:
        return COLOR_DEBUG;
    case LogLevel::INFO:
        return COLOR_INFO;
    case LogLevel::WARN:
        return COLOR_WARN;
    case LogLevel::ERROR:
        return COLOR_ERROR;
    default:
        return COLOR_RESET;
    }
}

const char *formatTime()
{
#if LOG_TIME == LOG_TIME_MICROS
    static char timeFormat[12];
    sprintf(timeFormat, "%11lu", micros());
#elif LOG_TIME == LOG_TIME_MILLIS
    static char timeFormat[9];
    sprintf(timeFormat, "%8lu", millis());
#elif (LOG_TIME == LOG_TIME_HHMMSSMS) || (LOG_TIME == LOG_TIME_HHHHMMSSMS)
    unsigned long ms = millis();
    unsigned long seconds = ms / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long hours = minutes / 60;
#if LOG_TIME == LOG_TIME_HHMMSSMS
    static char timeFormat[13];
    sprintf(timeFormat, "%02lu:%02lu:%02lu:%03lu", hours % 24, minutes % 60, seconds % 60, ms % 1000);
#elif LOG_TIME == LOG_TIME_HHHHMMSSMS
    static char timeFormat[15];
    sprintf(timeFormat, "%04lu:%02lu:%02lu:%03lu", hours, minutes % 60, seconds % 60, ms % 1000);
#endif // LOG_TIME == LOG_TIME_HHMMSSMS
#else
    static char timeFormat[1] = {'\0'}; // Default to an empty string as a safeguard.
#endif // LOG_TIME == LOG_TIME_MICROS
    return timeFormat;
}

const char *filePathToName(const char *path)
{
    size_t i = 0;
    size_t pos = 0;
    char *p = (char *)path;
    while (*p)
    {
        i++;
        if (*p == '/' || *p == '\\')
        {
            pos = i;
        }
        if (*p == '.')
        {
            *p = '\0';
            break;
        }
        p++;
    }
    return path + pos;
}

const char *filePathToNameNumber(const char *path, int line)
{
    static char result[64] = {0};
    const char *filename = strrchr(path, '/') ? strrchr(path, '/') + 1 : strrchr(path, '\\') ? strrchr(path, '\\') + 1
                                                                                             : path;

    snprintf(result, sizeof(result), "%s:%d", filename, line);
    return result;
}
