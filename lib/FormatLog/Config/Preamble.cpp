#include <Arduino.h>
#include "Config/Constants.h"
#include <assert.h>
#include <sys/time.h>

#ifndef LOG_TIME_LOCALTIME_FORMAT
#define LOG_TIME_LOCALTIME_FORMAT "%04d-%02d-%02d %02d:%02d:%02d.%03ld"
#endif

namespace preamble
{
    const char *logLevelText(int level, int format = LOG_LEVEL_TEXT_FORMAT_FULL)
    {
        assert(level >= LOG_LEVEL_TRACE && level <= LOG_LEVEL_ERROR);
        assert(format >= LOG_LEVEL_TEXT_FORMAT_LETTER && format <= LOG_LEVEL_TEXT_FORMAT_FULL);

        static const char *logLevelTexts[3][5] = {
            {"T", "D", "I", "W", "E"},                  // LOG_LEVEL_TEXT_FORMAT_LETTER
            {"TRAC", "DBUG", "INFO", "WARN", "EROR"},   // LOG_LEVEL_TEXT_FORMAT_SHORT
            {"TRACE", "DEBUG", "INFO", "WARN", "ERROR"} // LOG_LEVEL_TEXT_FORMAT_FULL
        };

        return logLevelTexts[format][level];
    }

    const char *formatTime(int format = LOG_TIME_MILLIS)
    {
        assert(format >= LOG_TIME_DISABLE && format <= LOG_TIME_LOCALTIME);

        static char timeFormat[64] = {0};

        if (format == LOG_TIME_DISABLE)
        {
            timeFormat[0] = '\0'; // No time format
            return timeFormat;
        }

        if (format == LOG_TIME_LOCALTIME)
        {
            struct tm timeinfo;
            time_t now;
            time(&now);
            localtime_r(&now, &timeinfo);
            if (timeinfo.tm_year > (2016 - 1900)) // Check if time has been initialized
            {
                timeval tv;
                gettimeofday(&tv, NULL);
                sprintf(timeFormat, LOG_TIME_LOCALTIME_FORMAT,
                        timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
                        timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, tv.tv_usec / 1000);
            }
            else
            {
                sprintf(timeFormat, "LOCALTIME_ERROR");
            }
        }
        else if (format == LOG_TIME_MICROS)
        {
            sprintf(timeFormat, "%11lu", micros());
        }
        else if (format == LOG_TIME_MILLIS)
        {
            sprintf(timeFormat, "%8lu", millis());
        }
        else if (format == LOG_TIME_HHMMSSMS || format == LOG_TIME_HHHHMMSSMS)
        {
            unsigned long ms = millis();
            unsigned long seconds = ms / 1000;
            unsigned long minutes = seconds / 60;
            unsigned long hours = minutes / 60;

            if (format == LOG_TIME_HHMMSSMS)
            {
                sprintf(timeFormat, "%02lu:%02lu:%02lu:%03lu", hours % 24, minutes % 60, seconds % 60, ms % 1000);
            }
            else if (format == LOG_TIME_HHHHMMSSMS)
            {
                sprintf(timeFormat, "%04lu:%02lu:%02lu:%03lu", hours, minutes % 60, seconds % 60, ms % 1000);
            }
        }

        return timeFormat;
    }

    const char *formatFilename(const char *file, int line = 0, const char *func = nullptr, int format = LOG_FILENAME_ENABLE)
    {
        assert(format >= LOG_FILENAME_DISABLE && format <= LOG_FILENAME_LINENUMBER_FUNCTION_ENABLE);

        static char result[64] = {0};

        if (format == LOG_FILENAME_DISABLE)
        {
            result[0] = '\0';
            return result;
        }

        const char *filename = strrchr(file, '/') ? strrchr(file, '/') + 1 : strrchr(file, '\\') ? strrchr(file, '\\') + 1
                                                                                                 : file;

        if (format == LOG_FILENAME_LINENUMBER_FUNCTION_ENABLE)
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
        else if (format == LOG_FILENAME_LINENUMBER_ENABLE)
        {
            snprintf(result, sizeof(result), "%s:%d", filename, line);
        }
        else if (format == LOG_FILENAME_ENABLE)
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

    const char *colorText(int level)
    {
        assert(level >= LOG_LEVEL_TRACE && level <= LOG_LEVEL_DISABLE);

        static const char *colors[] = {
            COLOR_TRACE, // White
            COLOR_DEBUG, // Green
            COLOR_INFO,  // Cyan
            COLOR_WARN,  // Yellow
            COLOR_ERROR, // Red
            COLOR_RESET  // Reset color
        };

        return colors[level];
    }
}