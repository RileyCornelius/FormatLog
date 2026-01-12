#include <Arduino.h>
#include <SPIFFS.h>
#include "Log.h"

void setup()
{
    LOG_BEGIN(115200);
    ASSERT(SPIFFS.begin(true));
    LOG_SET_STORAGE(SPIFFS);
    delay(3000);

    LOG_INFO("SPIFFS initialized successfully");

    size_t totalBytes = SPIFFS.totalBytes();
    size_t usedBytes = SPIFFS.usedBytes();
    LOG_DEBUG("SPIFFS Total: {} bytes", totalBytes);
    LOG_DEBUG("SPIFFS Used: {} bytes", usedBytes);
    LOG_DEBUG("SPIFFS Free: {} bytes", totalBytes - usedBytes);
}

void loop()
{
    static uint32_t lastLog = 0;
    static uint32_t iteration = 0;

    if (millis() - lastLog >= 1000)
    {
        lastLog = millis();
        iteration++;

        LOG_TRACE("Trace message - iteration {} uptime: {} ms", iteration, millis());

        if (iteration % 3 == 0)
        {
            LOG_WARN("Warning message - iteration {} is divisible by 3", iteration);
        }

        if (iteration % 5 == 0)
        {
            LOG_ERROR("Error message - iteration {} is divisible by 5", iteration);
        }

        if (iteration % 10 == 0)
        {
            LOG_DEBUG("SPIFFS Used: {}/{} bytes", SPIFFS.usedBytes(), SPIFFS.totalBytes());
        }
    }
}
