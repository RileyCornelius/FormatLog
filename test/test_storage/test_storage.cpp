// FileManager feature unit tests with SPIFFS
// Tests buffering, rotation, and file management

#include <Arduino.h>
#include <SPIFFS.h>
#include <string>
#include <vector>
#include "unity.h"

// Test configuration with small limits for quick testing
#define LOG_STATIC_BUFFER_SIZE 512 // Large enough for test messages
#define LOG_LEVEL LOG_LEVEL_TRACE
#define LOG_COLOR LOG_COLOR_DISABLE
#define LOG_TIME LOG_TIME_DISABLE
#define LOG_FILENAME LOG_FILENAME_DISABLE

#define LOG_STORAGE_ENABLE 1
#define LOG_STORAGE_LEVEL LOG_LEVEL_WARN
#define LOG_STORAGE_MAX_BUFFER_MESSAGES 3
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
    FmtLog.setStorage(SPIFFS, LOG_STORAGE_FILE_PATH);

    // File should not exist yet (lazy initialization)
    TEST_ASSERT_FALSE_MESSAGE(SPIFFS.exists(LOG_STORAGE_FILE_PATH), "File should not exist before first log");

    // Write a warning message
    LOG_WARN("Test warning message");

    // Now file should exist
    TEST_ASSERT_TRUE_MESSAGE(SPIFFS.exists(LOG_STORAGE_FILE_PATH), "File should exist after first WARN message");

    // Verify content
    LOG_FLUSH_STORAGE();
    std::string content = readFile(SPIFFS, LOG_STORAGE_FILE_PATH);
    TEST_ASSERT_TRUE_MESSAGE(content.find("Test warning message") != std::string::npos,
                             "File should contain the warning message");
}

void test_storage_level_filtering()
{
    cleanupLogFiles(SPIFFS);
    FmtLog.setStorage(SPIFFS);

    // TRACE and DEBUG should not go to storage
    LOG_TRACE("Trace message - should not be in file");
    LOG_DEBUG("Debug message - should not be in file");
    LOG_INFO("Info message - should not be in file");

    TEST_ASSERT_FALSE_MESSAGE(SPIFFS.exists(LOG_STORAGE_FILE_PATH),
                              "File should not exist - no WARN+ messages logged");

    // WARN and ERROR should go to storage
    LOG_WARN("Warning message - should be in file");
    LOG_ERROR("Error message - should be in file");

    TEST_ASSERT_TRUE_MESSAGE(SPIFFS.exists(LOG_STORAGE_FILE_PATH), "File should exist after WARN/ERROR");

    LOG_FLUSH_STORAGE();
    std::string content = readFile(SPIFFS, LOG_STORAGE_FILE_PATH);
    TEST_ASSERT_TRUE_MESSAGE(content.find("Warning message") != std::string::npos, "Should contain WARN");
    TEST_ASSERT_TRUE_MESSAGE(content.find("Error message") != std::string::npos, "Should contain ERROR");
    TEST_ASSERT_TRUE_MESSAGE(content.find("Trace message") == std::string::npos, "Should NOT contain TRACE");
}

void test_storage_buffering()
{
    cleanupLogFiles(SPIFFS);
    FmtLog.setStorage(SPIFFS);

    // Log 2 messages (below buffer limit of 3)
    LOG_WARN("Message 1");
    LOG_WARN("Message 2");

    // File exists but data may not be flushed yet
    TEST_ASSERT_TRUE_MESSAGE(SPIFFS.exists(LOG_STORAGE_FILE_PATH), "File should exist");

    size_t sizeBeforeFlush = getFileSize(SPIFFS, LOG_STORAGE_FILE_PATH);

    // Manually flush
    LOG_FLUSH_STORAGE();

    size_t sizeAfterFlush = getFileSize(SPIFFS, LOG_STORAGE_FILE_PATH);

    // Size should be greater after flush
    TEST_ASSERT_GREATER_THAN_MESSAGE(sizeBeforeFlush, sizeAfterFlush,
                                     "File size should increase after flush");
}

void test_storage_auto_flush_by_message_count()
{
    cleanupLogFiles(SPIFFS);
    FmtLog.setStorage(SPIFFS);

    // Log exactly 3 messages (buffer limit)
    LOG_WARN("Message 1");
    LOG_WARN("Message 2");
    LOG_WARN("Message 3");

    size_t size1 = getFileSize(SPIFFS, LOG_STORAGE_FILE_PATH);
    TEST_ASSERT_GREATER_THAN_MESSAGE(0, size1, "File should have content after 3 messages");

    // Log one more to confirm buffer is reset
    LOG_WARN("Message 4");

    size_t size2 = getFileSize(SPIFFS, LOG_STORAGE_FILE_PATH);
    TEST_ASSERT_GREATER_OR_EQUAL_MESSAGE(size1, size2, "File should grow with more messages");
}

void test_storage_manual_flush()
{
    cleanupLogFiles(SPIFFS);
    FmtLog.setStorage(SPIFFS);

    LOG_WARN("Before flush");

    size_t sizeBefore = getFileSize(SPIFFS, LOG_STORAGE_FILE_PATH);

    LOG_FLUSH_STORAGE();

    size_t sizeAfter = getFileSize(SPIFFS, LOG_STORAGE_FILE_PATH);

    TEST_ASSERT_GREATER_THAN_MESSAGE(sizeBefore, sizeAfter,
                                     "Manual flush should write buffered data");

    std::string content = readFile(SPIFFS, LOG_STORAGE_FILE_PATH);
    TEST_ASSERT_TRUE_MESSAGE(content.find("Before flush") != std::string::npos,
                             "Content should be present after manual flush");
}

void test_storage_file_rotation()
{
    cleanupLogFiles(SPIFFS);
    FmtLog.setStorage(SPIFFS);

    // Generate enough messages to exceed file size limit (512 bytes)
    for (int i = 0; i < 30; i++)
    {
        LOG_WARN("Log message number {} - adding content to trigger rotation", i);
        LOG_FLUSH_STORAGE(); // Flush each time to ensure write
        delay(10);
    }

    // Should have created rotated files
    int fileCount = countLogFiles(SPIFFS);
    TEST_ASSERT_GREATER_THAN_MESSAGE(1, fileCount,
                                     "Should have multiple log files after rotation");

    // Check main file exists
    TEST_ASSERT_TRUE_MESSAGE(SPIFFS.exists(LOG_STORAGE_FILE_PATH), "Main log file should exist");

    // Check at least one rotated file exists
    bool hasRotated = SPIFFS.exists("/test_log.1.txt");
    TEST_ASSERT_TRUE_MESSAGE(hasRotated, "At least one rotated file should exist");
}

void test_storage_max_files_limit()
{
    cleanupLogFiles(SPIFFS);
    FmtLog.setStorage(SPIFFS);

    // Generate many messages to create multiple rotations
    for (int i = 0; i < 60; i++)
    {
        LOG_ERROR("Rotation test message {} with some extra content for size", i);
        LOG_FLUSH_STORAGE();
        delay(10);
    }

    // Should not exceed MAX_FILES + 1 (current file + 3 rotated)
    int fileCount = countLogFiles(SPIFFS);
    TEST_ASSERT_LESS_OR_EQUAL_MESSAGE(LOG_STORAGE_MAX_FILES + 1, fileCount,
                                      "Should not exceed max files limit");

    // File 4 should not exist (beyond max)
    TEST_ASSERT_FALSE_MESSAGE(SPIFFS.exists("/test_log.4.txt"),
                              "Should not create files beyond max limit");
}

void test_storage_file_naming()
{
    FmtLog.setStorage(SPIFFS);
    cleanupLogFiles(SPIFFS);

    // Generate enough content for rotation
    for (int i = 0; i < 25; i++)
    {
        LOG_WARN("Naming test message {} with sufficient content", i);
        LOG_FLUSH_STORAGE();
        delay(10);
    }

    // Check proper naming sequence
    TEST_ASSERT_TRUE_MESSAGE(SPIFFS.exists(LOG_STORAGE_FILE_PATH), "Main file should exist");

    if (SPIFFS.exists("/test_log.1.txt"))
    {
        size_t size1 = getFileSize(SPIFFS, "/test_log.1.txt");
        TEST_ASSERT_GREATER_THAN_MESSAGE(0, size1, "Rotated file 1 should have content");

        // If .1 exists, it should be near the max size
        TEST_ASSERT_GREATER_OR_EQUAL_MESSAGE(LOG_STORAGE_MAX_FILE_SIZE / 2, size1,
                                             "Rotated file should have substantial content");
    }
}

void test_storage_empty_logs()
{
    cleanupLogFiles(SPIFFS);
    FmtLog.setStorage(SPIFFS);

    // Log only non-storage level messages
    LOG_TRACE("Trace");
    LOG_DEBUG("Debug");
    LOG_INFO("Info");

    // Should not create file
    TEST_ASSERT_FALSE_MESSAGE(SPIFFS.exists(LOG_STORAGE_FILE_PATH),
                              "Should not create file for below-threshold messages");
}

void test_storage_large_message()
{
    cleanupLogFiles(SPIFFS);
    FmtLog.setStorage(SPIFFS);

    // Create a large message
    std::string largePayload(200, 'X');
    LOG_ERROR("Large message: {}", largePayload.c_str());
    LOG_FLUSH_STORAGE();

    std::string content = readFile(SPIFFS, LOG_STORAGE_FILE_PATH);
    TEST_ASSERT_TRUE_MESSAGE(content.find(largePayload) != std::string::npos,
                             "Large message should be stored correctly");
}

void test_storage_rotated_files_are_readable()
{
    cleanupLogFiles(SPIFFS);
    FmtLog.setStorage(SPIFFS);

    // Create enough content to trigger at least one rotation.
    for (int i = 0; i < 40; i++)
    {
        LOG_WARN("Rotate-read test message {} with extra padding to grow file: 0123456789ABCDEF", i);
        LOG_FLUSH_STORAGE();
        delay(5);
    }

    // Ensure all handles are closed so we can read deterministically.
    LOG_CLOSE_STORAGE();

    // Main file should exist and be readable.
    TEST_ASSERT_TRUE_MESSAGE(SPIFFS.exists(LOG_STORAGE_FILE_PATH), "Main log should exist");
    File mainFile = SPIFFS.open(LOG_STORAGE_FILE_PATH, FILE_READ);
    TEST_ASSERT_TRUE_MESSAGE(mainFile, "Main log should be openable for reading");
    if (mainFile)
    {
        size_t size = mainFile.size();
        mainFile.close();
        TEST_ASSERT_GREATER_THAN_MESSAGE(0, size, "Main log should have content");
    }

    // At least one rotated file should exist and be readable when rotation happened.
    if (SPIFFS.exists("/test_log.1.txt"))
    {
        File rotated = SPIFFS.open("/test_log.1.txt", FILE_READ);
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
    cleanupLogFiles(SPIFFS);

    const char *path = "/test_log_noext";
    FmtLog.setStorage(SPIFFS, path);

    // Generate enough content to trigger at least one rotation.
    for (int i = 0; i < 40; i++)
    {
        LOG_WARN("No-ext naming test message {} with extra padding to grow file: 0123456789ABCDEF", i);
        LOG_FLUSH_STORAGE();
        delay(5);
    }

    LOG_CLOSE_STORAGE();

    // Base file should exist (it will be recreated after rotation when next written, but we log enough times).
    TEST_ASSERT_TRUE_MESSAGE(SPIFFS.exists(path), "Base no-ext log file should exist");

    // Rotated file should follow: <name>.1 (no trailing extension)
    TEST_ASSERT_TRUE_MESSAGE(SPIFFS.exists("/test_log_noext.1"),
                             "Rotated no-ext file should be named /test_log_noext.1");

    // Sanity: rotated file should be readable and non-empty
    File rotated = SPIFFS.open("/test_log_noext.1", FILE_READ);
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
    cleanupLogFiles(SPIFFS);

    // Multiple extensions: last extension should be preserved.
    // FileManager::rotate() splits on the last '.', so expected rotated name is: /log.txt.1.md
    const char *path = "/log.txt.md";
    FmtLog.setStorage(SPIFFS, path);

    for (int i = 0; i < 45; i++)
    {
        LOG_WARN("Multi-ext naming test message {} with extra padding to grow file: 0123456789ABCDEF", i);
        LOG_FLUSH_STORAGE();
        delay(5);
    }

    LOG_CLOSE_STORAGE();

    TEST_ASSERT_TRUE_MESSAGE(SPIFFS.exists(path), "Base multi-ext log file should exist");
    TEST_ASSERT_TRUE_MESSAGE(SPIFFS.exists("/log.txt.1.md"),
                             "Rotated multi-ext file should be named /log.txt.1.md");

    // Ensure we are not rotating as /log.1.txt.md (wrong split point)
    TEST_ASSERT_FALSE_MESSAGE(SPIFFS.exists("/log.1.txt.md"),
                              "Should not rotate using the first extension segment");

    // Sanity: rotated file should be readable and non-empty
    File rotated = SPIFFS.open("/log.txt.1.md", FILE_READ);
    TEST_ASSERT_TRUE_MESSAGE(rotated, "Rotated multi-ext file should be openable");
    if (rotated)
    {
        size_t size = rotated.size();
        rotated.close();
        TEST_ASSERT_GREATER_THAN_MESSAGE(0, size, "Rotated multi-ext file should have content");
    }
}

/*------------------------------------------------------------------------------
 * Test Runner
 *----------------------------------------------------------------------------*/

void setUp()
{
    // Initialize SPIFFS if needed
    if (!SPIFFS.begin(true))
    {
        TEST_FAIL_MESSAGE("SPIFFS initialization failed");
    }

    // Clean start for each test
    cleanupLogFiles(SPIFFS);

    // Reset log level
    LOG_SET_LOG_LEVEL(LogLevel::TRACE);
}

void tearDown()
{
    // Close storage before cleanup
    LOG_CLOSE_STORAGE();

    // Clean up after each test
    cleanupLogFiles(SPIFFS);
}

void tests()
{
    RUN_TEST(test_storage_initialization);
    RUN_TEST(test_storage_level_filtering);
    RUN_TEST(test_storage_buffering);
    RUN_TEST(test_storage_auto_flush_by_message_count);
    RUN_TEST(test_storage_manual_flush);
    RUN_TEST(test_storage_file_rotation);
    RUN_TEST(test_storage_max_files_limit);
    RUN_TEST(test_storage_file_naming);
    RUN_TEST(test_storage_empty_logs);
    RUN_TEST(test_storage_large_message);
    RUN_TEST(test_storage_rotated_files_are_readable);
    RUN_TEST(test_storage_file_naming_no_extension);
    RUN_TEST(test_storage_file_naming_multiple_extensions);
}

void setup()
{
    // Wait for serial monitor
    delay(3000);

    Serial.begin(115200);
    Serial.println("\n\n=== FileManager Unit Tests ===");

    UNITY_BEGIN();
    tests();
    UNITY_END();
}

void loop()
{
    // Nothing to do
}
