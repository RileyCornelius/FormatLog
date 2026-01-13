#include <Arduino.h>
#include <LittleFS.h>
#include "Log.h"

void cleanupLogFiles(fs::FS &filesystem)
{
    LOG_INFO("Cleaning up all files in filesystem...");

    File root = filesystem.open("/");
    if (!root || !root.isDirectory())
    {
        LOG_ERROR("Failed to open root directory");
        return;
    }

    File file = root.openNextFile();
    while (file)
    {
        if (!file.isDirectory())
        {
            String fileName = String("/") + file.name();
            file.close();
            filesystem.remove(fileName.c_str());
            LOG_INFO("  Removed: {}", fileName.c_str());
        }
        else
        {
            file.close();
        }
        file = root.openNextFile();
    }
    root.close();
}

void setup()
{
    LOG_BEGIN(115200);
    ASSERT(LittleFS.begin(true));
    LOG_SET_STORAGE(LittleFS);

    delay(5000);
    cleanupLogFiles(LittleFS);

    LOG_INFO("=== LittleFS Storage Test ===");
    LOG_INFO("LittleFS Total: {} bytes", LittleFS.totalBytes());
    LOG_INFO("LittleFS Used: {} bytes", LittleFS.usedBytes());
    LOG_INFO("Storage level: WARN+");
    LOG_INFO("Buffer: {} messages or {} bytes", LOG_STORAGE_MAX_BUFFER_MESSAGES, LOG_STORAGE_MAX_BUFFER_SIZE);
    LOG_INFO("Max file size: {} bytes", LOG_STORAGE_MAX_FILE_SIZE);
    LOG_INFO("Max files: {}", LOG_STORAGE_MAX_FILES);
    LOG_INFO("===========================");

    randomSeed(analogRead(A0));
}

void loop()
{
    static uint32_t lastLog = 0;
    static uint32_t msgCount = 0;
    static uint8_t resetCount = 0;

    // Stop after 5 resets
    if (resetCount >= 5)
    {
        LOG_INFO("=== TEST COMPLETE - HALTING ===");
        while (true)
        {
            delay(1000);
        }
    }

    // Generate a log message every 500ms for faster testing
    if (millis() - lastLog >= 500)
    {
        lastLog = millis();
        msgCount++;

        // Generate WARNING messages with varying lengths
        int randomLength = random(0, 4);
        switch (randomLength)
        {
        case 0:
            LOG_WARN("Test #{:04d} - Short", msgCount);
            break;
        case 1:
            LOG_WARN("Test message #{:04d} - Medium length message for testing", msgCount);
            break;
        case 2:
            LOG_WARN("Test message #{:04d} - Adding content to test buffering and rotation", msgCount);
            break;
        case 3:
            LOG_WARN("Test message #{:04d} - This is a longer message with more content to test how the system handles various sizes", msgCount);
            break;
        case 4:
            LOG_WARN("Test message #{:04d} - Extra long message with additional text to really push the boundaries and see how buffering and file rotation work with significantly larger content that spans much more space", msgCount);
            break;
        }

        // Stop after 20 messages to review results
        if (msgCount >= 20)
        {
            resetCount++;
            LOG_INFO("=== Resetting counter to continue testing (reset {}/5) ===", resetCount);
            msgCount = 0; // Reset to continue logging

            if (resetCount >= 5)
            {
                LOG_INFO("=== FINAL TEST COMPLETE - 5 cycles finished ===");
                LOG_FLUSH_STORAGE();
            }

            LOG_INFO("");

            LOG_INFO("=== Resetting counter to continue testing ===");
            msgCount = 0; // Reset to continue logging
        }
    }
}
