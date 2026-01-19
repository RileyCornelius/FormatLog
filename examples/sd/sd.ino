#include <Arduino.h>
#include <SD.h>
#include "Log.h"

#define PIN_SD_CS 5

void setup()
{
    Serial.begin(115200);
    SPI.begin();
    SD.begin(PIN_SD_CS, SPI);
    LOG_SET_STORAGE(SD);
    delay(3000);

    LOG_INFO("SD initialized successfully");

    size_t totalBytes = SD.totalBytes();
    size_t usedBytes = SD.usedBytes();
    LOG_DEBUG("SD Total: {} bytes", totalBytes);
    LOG_DEBUG("SD Used: {} bytes", usedBytes);
    LOG_DEBUG("SD Free: {} bytes", totalBytes - usedBytes);
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
            LOG_DEBUG("SPIFFS Used: {} bytes", SD.usedBytes());
        }
    }
}
