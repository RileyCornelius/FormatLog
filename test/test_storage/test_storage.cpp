#include <Arduino.h>
#include <string>
#include <vector>
#include "unity.h"

// File System Selection - Choose one:
#define TEST_FS_SPIFFS
// #define TEST_FS_LITTLEFS
// #define TEST_FS_SD
// #define TEST_FS_SDFAT

#ifndef TEST_FS_SPIFFS
#ifndef TEST_FS_LITTLEFS
#ifndef TEST_FS_SD
#ifndef TEST_FS_SDFAT
#define TEST_FS_SPIFFS // Default to SPIFFS if nothing defined
#endif
#endif
#endif
#endif

// Include the appropriate file system header
#ifdef TEST_FS_SPIFFS
#include <SPIFFS.h>
#define TEST_FS SPIFFS
#define TEST_FS_NAME "SPIFFS"
#elif defined(TEST_FS_LITTLEFS)
#include <LittleFS.h>
#define TEST_FS LittleFS
#define TEST_FS_NAME "LittleFS"
#elif defined(TEST_FS_SD)
#include <SD.h>
#define TEST_FS SD
#define TEST_FS_NAME "SD"
#elif defined(TEST_FS_SDFAT)
#include <SdFat.h>
SdFat TEST_FS;
#define TEST_FS_NAME "SdFat"
#endif

// Test configuration with small limits for quick testing
#define LOG_STATIC_BUFFER_SIZE 512 // Large enough for test messages
#define LOG_LEVEL LOG_LEVEL_TRACE
#define LOG_COLOR LOG_COLOR_DISABLE
#define LOG_TIME LOG_TIME_DISABLE
#define LOG_FILENAME LOG_FILENAME_DISABLE

#define LOG_STORAGE_ENABLE 1
#define LOG_STORAGE_LEVEL LOG_LEVEL_WARN
#define LOG_STORAGE_MAX_BUFFER_SIZE 256
#define LOG_STORAGE_MAX_FILE_SIZE 512 // Small for quick rotation
#define LOG_STORAGE_MAX_FILES 3
#define LOG_STORAGE_FILE_PATH "/test_log.txt"

#include <FormatLog.h>

/*------------------------------------------------------------------------------
 * Helper Functions
 *----------------------------------------------------------------------------*/

void cleanupLogFiles(fs::FS &filesystem)
{
    File root = filesystem.open("/");
    if (!root || !root.isDirectory())
    {
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
        }
        else
        {
            file.close();
        }
        file = root.openNextFile();
    }
    root.close();
}

size_t getFileSize(fs::FS &filesystem, const char *path)
{
    if (!filesystem.exists(path))
        return 0;

    File file = filesystem.open(path, FILE_READ);
    if (!file)
        return 0;

    size_t size = file.size();
    file.close();
    return size;
}

std::string readFile(fs::FS &filesystem, const char *path)
{
    if (!filesystem.exists(path))
        return "";

    File file = filesystem.open(path, FILE_READ);
    if (!file)
        return "";

    std::string content;
    while (file.available())
    {
        content.push_back(file.read());
    }
    file.close();
    return content;
}

int countLogFiles(fs::FS &filesystem)
{
    int count = 0;
    const char *files[] = {
        "/test_log.txt",
        "/test_log.1.txt",
        "/test_log.2.txt",
        "/test_log.3.txt",
    };

    for (const char *file : files)
    {
        if (filesystem.exists(file))
            count++;
    }

    return count;
}

/*------------------------------------------------------------------------------
 * Test Cases
 *----------------------------------------------------------------------------*/

void test_storage_initialization()
{
    LOG_SET_STORAGE(TEST_FS, LOG_STORAGE_FILE_PATH);

    // File should not exist yet (lazy initialization)
    TEST_ASSERT_FALSE_MESSAGE(TEST_FS.exists(LOG_STORAGE_FILE_PATH), "File should not exist before first log");

    // Write a warning message
    LOG_WARN("Test warning message");

    // Flush to create file
    LOG_FLUSH_STORAGE();

    // Now file should exist
    TEST_ASSERT_TRUE_MESSAGE(TEST_FS.exists(LOG_STORAGE_FILE_PATH), "File should exist after flush");

    // Verify content
    std::string content = readFile(TEST_FS, LOG_STORAGE_FILE_PATH);
    TEST_ASSERT_TRUE_MESSAGE(content.find("Test warning message") != std::string::npos,
                             "File should contain the warning message");
}

void test_storage_level_filtering()
{
    cleanupLogFiles(TEST_FS);
    LOG_SET_STORAGE(TEST_FS);

    // TRACE and DEBUG should not go to storage
    LOG_TRACE("Trace message - should not be in file");
    LOG_DEBUG("Debug message - should not be in file");
    LOG_INFO("Info message - should not be in file");

    LOG_FLUSH_STORAGE();

    TEST_ASSERT_FALSE_MESSAGE(TEST_FS.exists(LOG_STORAGE_FILE_PATH),
                              "File should not exist - no WARN+ messages logged");

    // WARN and ERROR should go to storage
    LOG_WARN("Warning message - should be in file");
    LOG_ERROR("Error message - should be in file");

    LOG_FLUSH_STORAGE();

    TEST_ASSERT_TRUE_MESSAGE(TEST_FS.exists(LOG_STORAGE_FILE_PATH), "File should exist after WARN/ERROR and flush");

    std::string content = readFile(TEST_FS, LOG_STORAGE_FILE_PATH);
    TEST_ASSERT_TRUE_MESSAGE(content.find("Warning message") != std::string::npos, "Should contain WARN");
    TEST_ASSERT_TRUE_MESSAGE(content.find("Error message") != std::string::npos, "Should contain ERROR");
    TEST_ASSERT_TRUE_MESSAGE(content.find("Trace message") == std::string::npos, "Should NOT contain TRACE");
}

void test_storage_buffering()
{
    cleanupLogFiles(TEST_FS);
    LOG_SET_STORAGE(TEST_FS);

    // Log 2 messages (below buffer size limit)
    LOG_WARN("Message 1");
    LOG_WARN("Message 2");

    // File may not exist yet - data is buffered
    size_t sizeBeforeFlush = getFileSize(TEST_FS, LOG_STORAGE_FILE_PATH);

    // Manually flush
    LOG_FLUSH_STORAGE();

    size_t sizeAfterFlush = getFileSize(TEST_FS, LOG_STORAGE_FILE_PATH);

    // Size should be greater after flush
    TEST_ASSERT_GREATER_THAN_MESSAGE(sizeBeforeFlush, sizeAfterFlush,
                                     "File size should increase after flush");
}

void test_storage_manual_flush()
{
    cleanupLogFiles(TEST_FS);
    LOG_SET_STORAGE(TEST_FS);

    LOG_WARN("Before flush");

    size_t sizeBefore = getFileSize(TEST_FS, LOG_STORAGE_FILE_PATH);

    LOG_FLUSH_STORAGE();

    size_t sizeAfter = getFileSize(TEST_FS, LOG_STORAGE_FILE_PATH);

    TEST_ASSERT_GREATER_THAN_MESSAGE(sizeBefore, sizeAfter,
                                     "Manual flush should write buffered data");

    std::string content = readFile(TEST_FS, LOG_STORAGE_FILE_PATH);
    TEST_ASSERT_TRUE_MESSAGE(content.find("Before flush") != std::string::npos,
                             "Content should be present after manual flush");
}

void test_storage_file_rotation()
{
    cleanupLogFiles(TEST_FS);
    LOG_SET_STORAGE(TEST_FS);

    // Generate enough messages to exceed file size limit (512 bytes)
    for (int i = 0; i < 30; i++)
    {
        LOG_WARN("Log message number {} - adding content to trigger rotation", i);
        LOG_FLUSH_STORAGE(); // Flush each time to ensure write
        delay(10);
    }

    // Should have created rotated files
    int fileCount = countLogFiles(TEST_FS);
    TEST_ASSERT_GREATER_THAN_MESSAGE(1, fileCount,
                                     "Should have multiple log files after rotation");

    // Check main file exists
    TEST_ASSERT_TRUE_MESSAGE(TEST_FS.exists(LOG_STORAGE_FILE_PATH), "Main log file should exist");

    // Check at least one rotated file exists
    bool hasRotated = TEST_FS.exists("/test_log.1.txt");
    TEST_ASSERT_TRUE_MESSAGE(hasRotated, "At least one rotated file should exist");
}

void test_storage_max_files_limit()
{
    cleanupLogFiles(TEST_FS);
    LOG_SET_STORAGE(TEST_FS);

    // Generate many messages to create multiple rotations
    for (int i = 0; i < 60; i++)
    {
        LOG_ERROR("Rotation test message {} with some extra content for size", i);
        LOG_FLUSH_STORAGE();
        delay(10);
    }

    // Should not exceed MAX_FILES + 1 (current file + 3 rotated)
    int fileCount = countLogFiles(TEST_FS);
    TEST_ASSERT_LESS_OR_EQUAL_MESSAGE(LOG_STORAGE_MAX_FILES + 1, fileCount,
                                      "Should not exceed max files limit");

    // File 4 should not exist (beyond max)
    TEST_ASSERT_FALSE_MESSAGE(TEST_FS.exists("/test_log.4.txt"),
                              "Should not create files beyond max limit");
}

void test_storage_file_naming()
{
    LOG_SET_STORAGE(TEST_FS);
    cleanupLogFiles(TEST_FS);

    // Generate enough content for rotation
    for (int i = 0; i < 25; i++)
    {
        LOG_WARN("Naming test message {} with sufficient content", i);
        LOG_FLUSH_STORAGE();
        delay(10);
    }

    // Check proper naming sequence
    TEST_ASSERT_TRUE_MESSAGE(TEST_FS.exists(LOG_STORAGE_FILE_PATH), "Main file should exist");

    if (TEST_FS.exists("/test_log.1.txt"))
    {
        size_t size1 = getFileSize(TEST_FS, "/test_log.1.txt");
        TEST_ASSERT_GREATER_THAN_MESSAGE(0, size1, "Rotated file 1 should have content");

        // If .1 exists, it should be near the max size
        TEST_ASSERT_GREATER_OR_EQUAL_MESSAGE(LOG_STORAGE_MAX_FILE_SIZE / 2, size1,
                                             "Rotated file should have substantial content");
    }
}

void test_storage_empty_logs()
{
    cleanupLogFiles(TEST_FS);
    LOG_SET_STORAGE(TEST_FS);

    // Log only non-storage level messages
    LOG_TRACE("Trace");
    LOG_DEBUG("Debug");
    LOG_INFO("Info");

    LOG_FLUSH_STORAGE();

    // Should not create file
    TEST_ASSERT_FALSE_MESSAGE(TEST_FS.exists(LOG_STORAGE_FILE_PATH),
                              "Should not create file for below-threshold messages");
}

void test_storage_large_message()
{
    cleanupLogFiles(TEST_FS);
    LOG_SET_STORAGE(TEST_FS);

    // Create a large message
    std::string largePayload(200, 'X');
    LOG_ERROR("Large message: {}", largePayload.c_str());
    LOG_FLUSH_STORAGE();

    std::string content = readFile(TEST_FS, LOG_STORAGE_FILE_PATH);
    TEST_ASSERT_TRUE_MESSAGE(content.find(largePayload) != std::string::npos,
                             "Large message should be stored correctly");
}

void test_storage_rotated_files_are_readable()
{
    cleanupLogFiles(TEST_FS);
    LOG_SET_STORAGE(TEST_FS);

    // Create enough content to trigger at least one rotation.
    for (int i = 0; i < 40; i++)
    {
        LOG_WARN("Rotate-read test message {} with extra padding to grow file: 0123456789ABCDEF", i);
        LOG_FLUSH_STORAGE();
        delay(5);
    }

    // Ensure all data is flushed so we can read deterministically.
    LOG_FLUSH_STORAGE();

    // Main file should exist and be readable.
    TEST_ASSERT_TRUE_MESSAGE(TEST_FS.exists(LOG_STORAGE_FILE_PATH), "Main log should exist");
    File mainFile = TEST_FS.open(LOG_STORAGE_FILE_PATH, FILE_READ);
    TEST_ASSERT_TRUE_MESSAGE(mainFile, "Main log should be openable for reading");
    if (mainFile)
    {
        size_t size = mainFile.size();
        mainFile.close();
        TEST_ASSERT_GREATER_THAN_MESSAGE(0, size, "Main log should have content");
    }

    // At least one rotated file should exist and be readable when rotation happened.
    if (TEST_FS.exists("/test_log.1.txt"))
    {
        File rotated = TEST_FS.open("/test_log.1.txt", FILE_READ);
        TEST_ASSERT_TRUE_MESSAGE(rotated, "Rotated log should be openable for reading");
        if (rotated)
        {
            size_t size = rotated.size();
            rotated.close();
            TEST_ASSERT_GREATER_THAN_MESSAGE(0, size, "Rotated log should have content");
        }
    }
    else
    {
        // If rotation didn't occur, fail loudly so we don't silently skip the intent.
        TEST_FAIL_MESSAGE("Expected rotation to create /test_log.1.txt, but it was missing");
    }
}

void test_storage_file_naming_no_extension()
{
    cleanupLogFiles(TEST_FS);

    const char *path = "/test_log_noext";
    LOG_SET_STORAGE(TEST_FS, path);

    // Generate enough content to trigger at least one rotation.
    for (int i = 0; i < 40; i++)
    {
        LOG_WARN("No-ext naming test message {} with extra padding to grow file: 0123456789ABCDEF", i);
        LOG_FLUSH_STORAGE();
        delay(5);
    }

    LOG_FLUSH_STORAGE();

    // Base file should exist (it will be recreated after rotation when next written, but we log enough times).
    TEST_ASSERT_TRUE_MESSAGE(TEST_FS.exists(path), "Base no-ext log file should exist");

    // Rotated file should follow: <name>.1 (no trailing extension)
    TEST_ASSERT_TRUE_MESSAGE(TEST_FS.exists("/test_log_noext.1"),
                             "Rotated no-ext file should be named /test_log_noext.1");

    // Sanity: rotated file should be readable and non-empty
    File rotated = TEST_FS.open("/test_log_noext.1", FILE_READ);
    TEST_ASSERT_TRUE_MESSAGE(rotated, "Rotated no-ext file should be openable");
    if (rotated)
    {
        size_t size = rotated.size();
        rotated.close();
        TEST_ASSERT_GREATER_THAN_MESSAGE(0, size, "Rotated no-ext file should have content");
    }
}

void test_storage_file_naming_multiple_extensions()
{
    cleanupLogFiles(TEST_FS);

    // Multiple extensions: last extension should be preserved.
    // FileManager::rotate() splits on the last '.', so expected rotated name is: /log.txt.1.md
    const char *path = "/log.txt.md";
    LOG_SET_STORAGE(TEST_FS, path);

    for (int i = 0; i < 45; i++)
    {
        LOG_WARN("Multi-ext naming test message {} with extra padding to grow file: 0123456789ABCDEF", i);
        LOG_FLUSH_STORAGE();
        delay(5);
    }

    LOG_FLUSH_STORAGE();

    TEST_ASSERT_TRUE_MESSAGE(TEST_FS.exists(path), "Base multi-ext log file should exist");
    TEST_ASSERT_TRUE_MESSAGE(TEST_FS.exists("/log.txt.1.md"),
                             "Rotated multi-ext file should be named /log.txt.1.md");

    // Ensure we are not rotating as /log.1.txt.md (wrong split point)
    TEST_ASSERT_FALSE_MESSAGE(TEST_FS.exists("/log.1.txt.md"),
                              "Should not rotate using the first extension segment");

    // Sanity: rotated file should be readable and non-empty
    File rotated = TEST_FS.open("/log.txt.1.md", FILE_READ);
    TEST_ASSERT_TRUE_MESSAGE(rotated, "Rotated multi-ext file should be openable");
    if (rotated)
    {
        size_t size = rotated.size();
        rotated.close();
        TEST_ASSERT_GREATER_THAN_MESSAGE(0, size, "Rotated multi-ext file should have content");
    }
}

void test_storage_buffering_real_buffer()
{
    cleanupLogFiles(TEST_FS);
    LOG_SET_STORAGE(TEST_FS);

    // Write some initial content and flush
    LOG_WARN("Initial message");
    LOG_FLUSH_STORAGE();

    size_t initialSize = getFileSize(TEST_FS, LOG_STORAGE_FILE_PATH);
    TEST_ASSERT_GREATER_THAN_MESSAGE(0, initialSize, "Initial file should have content");

    // Write two messages without flushing (should remain in buffer)
    LOG_WARN("Buffered message 1");
    LOG_WARN("Buffered message 2");

    // File size should not have increased yet (messages are in buffer)
    size_t sizeBeforeFlush = getFileSize(TEST_FS, LOG_STORAGE_FILE_PATH);
    TEST_ASSERT_EQUAL_MESSAGE(initialSize, sizeBeforeFlush,
                              "File size should not increase before flush (messages in buffer)");

    // Flush and verify size increased
    LOG_FLUSH_STORAGE();
    size_t sizeAfterFlush = getFileSize(TEST_FS, LOG_STORAGE_FILE_PATH);
    TEST_ASSERT_GREATER_THAN_MESSAGE(sizeBeforeFlush, sizeAfterFlush,
                                     "File size should increase after flushing buffer");

    // Verify content is correct
    std::string content = readFile(TEST_FS, LOG_STORAGE_FILE_PATH);
    TEST_ASSERT_TRUE_MESSAGE(content.find("Buffered message 1") != std::string::npos,
                             "Buffered message 1 should be in file after flush");
    TEST_ASSERT_TRUE_MESSAGE(content.find("Buffered message 2") != std::string::npos,
                             "Buffered message 2 should be in file after flush");
}

void test_storage_auto_flush_on_buffer_full()
{
    cleanupLogFiles(TEST_FS);
    LOG_SET_STORAGE(TEST_FS);

    // Fill buffer with messages (256 byte limit)
    // Each message with preamble is roughly 25-30 bytes, so 10 messages should fill the buffer
    for (int i = 0; i < 10; i++)
    {
        LOG_WARN("Buffer fill message {} with extra padding", i);
    }

    // Buffer should have auto-flushed, so file should exist and have content
    size_t size = getFileSize(TEST_FS, LOG_STORAGE_FILE_PATH);
    TEST_ASSERT_GREATER_THAN_MESSAGE(0, size, "File should have content after buffer fills and auto-flushes");
}

/*------------------------------------------------------------------------------
 * Test Runner
 *----------------------------------------------------------------------------*/

void setUp()
{
    // Reset log level
    // LOG_SET_LOG_LEVEL(LogLevel::TRACE);
}

void tearDown()
{
    // Clean up after each test
    cleanupLogFiles(TEST_FS);
}

void tests()
{
    RUN_TEST(test_storage_initialization);
    RUN_TEST(test_storage_level_filtering);
    RUN_TEST(test_storage_buffering);
    RUN_TEST(test_storage_manual_flush);
    RUN_TEST(test_storage_auto_flush_on_buffer_full);
    RUN_TEST(test_storage_file_rotation);
    RUN_TEST(test_storage_max_files_limit);
    RUN_TEST(test_storage_file_naming);
    RUN_TEST(test_storage_empty_logs);
    RUN_TEST(test_storage_large_message);
    RUN_TEST(test_storage_rotated_files_are_readable);
    RUN_TEST(test_storage_file_naming_no_extension);
    RUN_TEST(test_storage_file_naming_multiple_extensions);
    RUN_TEST(test_storage_buffering_real_buffer);
}

void setup()
{
    Serial.begin(115200);
    // Wait for serial monitor
    delay(3000);

    // Initialize file system if needed
#ifdef TEST_FS_SDFAT
    if (!TEST_FS.begin())
    {
        TEST_FAIL_MESSAGE(TEST_FS_NAME " initialization failed");
    }
#else
    if (!TEST_FS.begin(true))
    {
        TEST_FAIL_MESSAGE(TEST_FS_NAME " initialization failed");
    }
#endif

    cleanupLogFiles(TEST_FS);

    UNITY_BEGIN();
    tests();
    UNITY_END();
}

void loop()
{
    // Nothing to do
}
