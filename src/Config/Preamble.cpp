#include "Preamble.h"
#include <sys/time.h>
#include <Arduino.h>

namespace preamble
{
    const char *logLevelText(LogLevel level, LogLevelTextFormat format)
    {
        static const char *logLevelTexts[3][6] = {
            {"", "E", "W", "I", "D", "T"},                  // LogLevelTextFormat::LETTER
            {"", "EROR", "WARN", "INFO", "DBUG", "TRAC"},   // LogLevelTextFormat::SHORT
            {"", "ERROR", "WARN", "INFO", "DEBUG", "TRACE"} // LogLevelTextFormat::FULL
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
                sprintf(timeFormat, "");
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

        // Strip extension from filename
        char nameOnly[64];
        const char *dot = strrchr(filename, '.');
        if (dot)
        {
            size_t len = dot - filename;
            if (len >= sizeof(nameOnly))
                len = sizeof(nameOnly) - 1;
            strncpy(nameOnly, filename, len);
            nameOnly[len] = '\0';
        }
        else
        {
            strncpy(nameOnly, filename, sizeof(nameOnly) - 1);
            nameOnly[sizeof(nameOnly) - 1] = '\0';
        }

        if (format == LogFilename::LINENUMBER_FUNCTION_ENABLE)
        {
            if (func)
            {
                snprintf(result, sizeof(result), "%s:%d %s()", nameOnly, line, func);
            }
            else
            {
                snprintf(result, sizeof(result), "%s:%d", nameOnly, line);
            }
        }
        else if (format == LogFilename::LINENUMBER_ENABLE)
        {
            snprintf(result, sizeof(result), "%s:%d", nameOnly, line);
        }
        else if (format == LogFilename::ENABLE)
        {
            strncpy(result, nameOnly, sizeof(result) - 1);
            result[sizeof(result) - 1] = '\0';
        }

        return result;
    }

    const char *colorText(LogLevel level)
    {
        static const char *colors[] = {
            COLOR_RESET, // Disable - Reset color
            COLOR_ERROR, // Red
            COLOR_WARN,  // Yellow
            COLOR_INFO,  // Cyan
            COLOR_DEBUG, // Green
            COLOR_TRACE  // White
        };

        return colors[static_cast<int>(level)];
    }
}