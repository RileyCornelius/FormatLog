#pragma once

#include <Arduino.h>

namespace fmtlog
{

    class Stopwatch
    {
    private:
        uint32_t _startMs;

    public:
        Stopwatch() : _startMs(millis()) {}

        void reset() { _startMs = millis(); }

        uint32_t elapsedMs() const { return millis() - _startMs; }

        const char *elapsedTime() const
        {
            static char buf[16];
            uint32_t ms = elapsedMs();
            uint32_t seconds = ms / 1000;
            uint32_t minutes = seconds / 60;
            uint32_t hours = minutes / 60;
            snprintf(buf, sizeof(buf), "%02lu:%02lu:%02lu:%03lu",
                     hours, minutes % 60, seconds % 60, ms % 1000);
            return buf;
        }
    };

    class MicroStopwatch
    {
    private:
        uint32_t _startUs;

    public:
        MicroStopwatch() : _startUs(micros()) {}

        void reset() { _startUs = micros(); }

        uint32_t elapsedUs() const { return micros() - _startUs; }

        uint32_t elapsedMs() const { return elapsedUs() / 1000; }

        const char *elapsedTime() const
        {
            static char buf[20];
            uint32_t us = elapsedUs();
            uint32_t ms = us / 1000;
            uint32_t seconds = ms / 1000;
            uint32_t minutes = seconds / 60;
            uint32_t hours = minutes / 60;
            snprintf(buf, sizeof(buf), "%02lu:%02lu:%02lu:%03lu:%03lu",
                     hours, minutes % 60, seconds % 60, ms % 1000, us % 1000);
            return buf;
        }
    };

} // namespace fmtlog
