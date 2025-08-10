#include "Preamble.h"
#include <sys/time.h>
#include <Arduino.h>

namespace preamble
{
    const char *logLevelText(LogLevel level, LogLevelTextFormat format)
    {
        static const char *logLevelTexts[3][5] = {
            {"T", "D", "I", "W", "E"},                  // LogLevelTextFormat::LETTER
            {"TRAC", "DBUG", "INFO", "WARN", "EROR"},   // LogLevelTextFormat::SHORT
            {"TRACE", "DEBUG", "INFO", "WARN", "ERROR"} // LogLevelTextFormat::FULL
        };

        return logLevelTexts[(static_cast<int>(format))][static_cast<int>(level)];
    }

    const char *formatTime(LogTime format)
    {
        static char timeFormat[64] = {0};

        if (format == LogTime::DISABLE)
        {
            timeFormat[0] = '\0'; // No time format
            return timeFormat;
        }

        if (format == LogTime::LOCALTIME)
        {
            struct tm timeinfo;
            time_t now;
            time(&now);
            localtime_r(&now, &timeinfo);
            if (timeinfo.tm_year > (2016 - 1900)) // Check if time has been initialized
            {
                timeval tv;
                gettimeofday(&tv, NULL);
                sprintf(timeFormat, "%04d-%02d-%02d %02d:%02d:%02d.%03ld",
                        timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                        timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, tv.tv_usec / 1000);
            }
            else
            {
                sprintf(timeFormat, "LOCALTIME_ERROR");
            }
        }
        else if (format == LogTime::HHHHMMSSMS)
        {
            unsigned long ms = millis();
            unsigned long seconds = ms / 1000;
            unsigned long minutes = seconds / 60;
            unsigned long hours = minutes / 60;
            sprintf(timeFormat, "%04lu:%02lu:%02lu:%03lu", hours, minutes % 60, seconds % 60, ms % 1000);
        }
        else if (format == LogTime::HHMMSSMS)
        {
            unsigned long ms = millis();
            unsigned long seconds = ms / 1000;
            unsigned long minutes = seconds / 60;
            unsigned long hours = minutes / 60;
            sprintf(timeFormat, "%02lu:%02lu:%02lu:%03lu", hours % 24, minutes % 60, seconds % 60, ms % 1000);
        }
        else if (format == LogTime::MICROS)
        {
            sprintf(timeFormat, "%11lu", micros());
        }
        else if (format == LogTime::MILLIS || format == LogTime::ENABLE)
        {
            sprintf(timeFormat, "%8lu", millis());
        }

        return timeFormat;
    }

    const char *formatFilename(const char *file, int line, const char *func, LogFilename format)
    {
        static char result[64] = {0};

        if (format == LogFilename::DISABLE)
        {
            result[0] = '\0';
            return result;
        }

        const char *filename = strrchr(file, '/') ? strrchr(file, '/') + 1 : strrchr(file, '\\') ? strrchr(file, '\\') + 1
                                                                                                 : file;

        if (format == LogFilename::LINENUMBER_FUNCTION_ENABLE)
        {
            if (func)
            {
                snprintf(result, sizeof(result), "%s:%d %s()", filename, line, func);
            }
            else
            {
                snprintf(result, sizeof(result), "%s:%d", filename, line);
            }
        }
        else if (format == LogFilename::LINENUMBER_ENABLE)
        {
            snprintf(result, sizeof(result), "%s:%d", filename, line);
        }
        else if (format == LogFilename::ENABLE)
        {
            const char *dot = strrchr(filename, '.');
            if (dot)
            {
                size_t len = dot - filename;
                strncpy(result, filename, len);
                result[len] = '\0';
            }
            else
            {
                strncpy(result, filename, sizeof(result) - 1);
                result[sizeof(result) - 1] = '\0';
            }
        }

        return result;
    }

    const char *colorText(LogLevel level)
    {
        static const char *colors[] = {
            COLOR_TRACE, // White
            COLOR_DEBUG, // Green
            COLOR_INFO,  // Cyan
            COLOR_WARN,  // Yellow
            COLOR_ERROR, // Red
            COLOR_RESET  // Reset color
        };

        return colors[static_cast<int>(level)];
    }
}