#include <Arduino.h>
#include "Log.h" // Check Log.h configuration for storage settings

// === Choose ONE filesystem by uncommenting the appropriate section ===

// * --- SPIFFS --- *
#include <SPIFFS.h>
#define FILE_SYSTEM SPIFFS

// * --- LittleFS --- *
// #include <LittleFS.h>
// #define FILE_SYSTEM LittleFS

// * --- SD --- *
// #include <SD.h>
// #define FILE_SYSTEM SD

// * --- FFat --- *
// #include <FFat.h>
// #define FILE_SYSTEM FFat

// * --- SdFat --- *
// #include "SdFat.h"
// SdFat sd;
// #define SD_CS_PIN SS
// #define FILE_SYSTEM sd

void setup()
{
    // * --- SPIFFS --- *
    ASSERT(SPIFFS.begin(true));
    LOG_SET_STORAGE(SPIFFS);

    // * --- LittleFS --- *
    // ASSERT(LittleFS.begin(true));
    // LOG_SET_STORAGE(LittleFS);

    // * --- SD --- *
    // SPI.begin();
    // ASSERT(SD.begin(SS, SPI));
    // LOG_SET_STORAGE(SD);

    // * --- FFat --- *
    // ASSERT(FFat.begin(true));
    // LOG_SET_STORAGE(FFat);

    // * --- SdFat --- *
    // SPI.begin();
    // ASSERT(sd.begin(SD_CS_PIN));
    // LOG_SET_STORAGE(sd);

    delay(3000);
    LOG_BEGIN(115200);
    LOG_INFO("Storage initialized");
}

void loop()
{
    static uint32_t lastLog = 0;
    static uint32_t iteration = 0;

    if (millis() - lastLog >= 1000)
    {
        lastLog = millis();
        iteration++;

        LOG_TRACE("Trace - iteration {} uptime: {} ms", iteration, millis()); // Will NOT be stored in log file

        if (iteration % 3 == 0)
        {
            LOG_WARN("Warning - iteration {}", iteration); // Will be stored in log file
        }

        if (iteration % 5 == 0)
        {
            LOG_ERROR("Error - iteration {}", iteration); // Will be stored in log file
        }

        if (iteration % 20 == 0)
        {
            // Ensure file is closed before reading (this flushes any remaining data)
            LOG_CLOSE_STORAGE();

            // Read and print log file contents
            LOG_PRINTLN("--- Reading Log File (Iteration {}) ---", iteration);
            auto file = FILE_SYSTEM.open(LOG_GET_STORAGE_FILE_PATH().c_str());
            if (file)
            {
                while (file.available())
                {
                    Serial.write(file.read());
                }
                file.close();
                LOG_PRINTLN("--- End of Log File ---");
            }
            else
            {
                LOG_ERROR("Failed to open log file for reading");
            }

            // Clean up log file for next iterations
            FILE_SYSTEM.remove(LOG_GET_STORAGE_FILE_PATH().c_str());
        }
    }
}
