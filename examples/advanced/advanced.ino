#include <Arduino.h>
#include "Log.h"
#include <array>

void setup()
{
    LOG_BEGIN(115200);
}

void loop()
{
    static uint32_t lastLog = 0;
    static uint32_t iteration = 0;
    static std::array<int, 3> values = {1, 2, 3};

    if (millis() - lastLog >= 5000)
    {
        lastLog = millis();
        iteration++;

        LOG_INFO("Loop iteration: {} uptime: {}ms array: {}", iteration, millis(), values);

        // Demonstrate runtime log level change
        if (iteration == 3)
        {
            LOG_INFO("Raising log level to WARN");
            LOG_SET_LOG_LEVEL(LogLevel::WARN);
        }

        if (iteration == 6)
        {
            LOG_INFO("This won't print (level is WARN)");
            LOG_WARN("Restoring log level to TRACE");
            LOG_SET_LOG_LEVEL(LogLevel::TRACE);
        }
    }
}
