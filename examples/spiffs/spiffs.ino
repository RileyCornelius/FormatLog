#include <Arduino.h>
#include <SPIFFS.h>
#include "Log.h"

void listLogFilesDetailed()
{
    LOG_FLUSH_STORAGE(); // Ensure everything is flushed before reading

    LOG_INFO("=== Log Files on SPIFFS ===");

    const char *files[] = {
        "/log.txt",
        "/log.1.txt",
        "/log.2.txt",
        "/log.3.txt",
        "/log.4.txt",
    };

    int totalFiles = 0;
    size_t totalSize = 0;

    for (const char *filename : files)
    {
        if (SPIFFS.exists(filename))
        {
            File file = SPIFFS.open(filename, FILE_READ);
            if (file)
            {
                size_t fileSize = file.size();
                totalFiles++;
                totalSize += fileSize;
                LOG_INFO("  {} - {} bytes", filename, fileSize);
                file.close();
            }
        }
    }

    LOG_INFO("  Total: {} files, {} bytes", totalFiles, totalSize);
    LOG_INFO("===========================");
}

void printLogFileContents()
{
    LOG_FLUSH_STORAGE(); // Ensure everything is flushed before reading

    const char *files[] = {
        "/log.txt",
        "/log.1.txt",
        "/log.2.txt",
        "/log.3.txt",
    };

    LOG_INFO("=== LOG FILE CONTENTS ===");

    for (const char *filename : files)
    {
        if (SPIFFS.exists(filename))
        {
            File file = SPIFFS.open(filename, FILE_READ);
            if (file)
            {
                LOG_INFO("--- {} ({} bytes) ---", filename, file.size());

                // Read and print file contents line by line
                while (file.available())
                {
                    String line = file.readStringUntil('\n');
                    Serial.println(line); // Print directly to Serial to avoid LOG_ formatting
                }

                file.close();
                LOG_INFO("--- End of {} ---", filename);
            }
        }
    }

    LOG_INFO("=== END CONTENTS ===");
}

void cleanupLogFiles()
{
    const char *files[] = {
        "/log.txt",
        "/log.1.txt",
        "/log.2.txt",
        "/log.3.txt",
    };

    LOG_INFO("Cleaning up old log files...");
    for (const char *file : files)
    {
        if (SPIFFS.exists(file))
        {
            SPIFFS.remove(file);
            LOG_INFO("  Removed: {}", file);
        }
    }
}

void setup()
{
    LOG_BEGIN(115200);
    ASSERT(SPIFFS.begin(true));
    LOG_SET_STORAGE(SPIFFS);

    delay(5000);
    cleanupLogFiles();

    LOG_INFO("=== SPIFFS Storage Test ===");
    LOG_INFO("SPIFFS Total: {} bytes", SPIFFS.totalBytes());
    LOG_INFO("SPIFFS Used: {} bytes", SPIFFS.usedBytes());
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
        printLogFileContents();

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

        // Show status every 3 messages
        if (msgCount % 3 == 0)
        {
            LOG_INFO("--- Status at message {} ---", msgCount);
            listLogFilesDetailed();
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
                FmtLog.closeStorage();
            }

            LOG_INFO("");

            LOG_INFO("=== Resetting counter to continue testing ===");
            msgCount = 0; // Reset to continue logging
        }
    }
}
