#include <Arduino.h>
#include "Config/Constants.h"
#include <assert.h>

namespace preamble
{
    const char *logLevelText(int level, int format = LOG_LEVEL_TEXT_FORMAT_FULL)
    {
        assert(level >= LOG_LEVEL_TRACE && level <= LOG_LEVEL_ERROR);
        assert(format >= LOG_LEVEL_TEXT_FORMAT_LETTER && format <= LOG_LEVEL_TEXT_FORMAT_FULL);

        static const char *logLevelTexts[][5] = {
            {"T", "D", "I", "W", "E"},                  // LOG_LEVEL_TEXT_FORMAT_LETTER
            {"TRAC", "DBUG", "INFO", "WARN", "EROR"},   // LOG_LEVEL_TEXT_FORMAT_SHORT
            {"TRACE", "DEBUG", "INFO", "WARN", "ERROR"} // LOG_LEVEL_TEXT_FORMAT_FULL
        };

        return logLevelTexts[format][level];
    }

    const char *formatTime(int format = LOG_TIME_MILLIS)
    {
        assert(format >= LOG_TIME_DISABLE && format <= LOG_TIME_HHHHMMSSMS);

        static char timeFormat[15] = {0};

        if (format == LOG_TIME_DISABLE)
        {
            timeFormat[0] = '\0'; // No time format
            return timeFormat;
        }

        if (format == LOG_TIME_MICROS)
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

    const char *formatFilename(const char *path, int line = 0, int format = LOG_FILENAME_ENABLE)
    {
        assert(path != nullptr);
        assert(line >= 0);
        assert(format >= LOG_FILENAME_DISABLE && format <= LOG_FILENAME_LINENUMBER_ENABLE);

        static char result[64] = {0};

        if (format == LOG_FILENAME_DISABLE)
        {
            result[0] = '\0';
            return result;
        }

        const char *filename = strrchr(path, '/') ? strrchr(path, '/') + 1 : strrchr(path, '\\') ? strrchr(path, '\\') + 1
                                                                                                 : path;

        if (format == LOG_FILENAME_LINENUMBER_ENABLE)
        {
            snprintf(result, sizeof(result), "%s:%d", filename, line);
        }
        else if (format == LOG_FILENAME_ENABLE)
        {
            // Find the extension and truncate at the dot
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
        assert(level >= LOG_LEVEL_TRACE && level <= LOG_LEVEL_ERROR);

        static const char *colors[] = {
            COLOR_TRACE, // White
            COLOR_DEBUG, // Green
            COLOR_INFO,  // Cyan
            COLOR_WARN,  // Yellow
            COLOR_ERROR  // Red
        };

        return colors[level];
    }
}