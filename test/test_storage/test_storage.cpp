#include <Arduino.h>
#include <string>
#include <vector>
#include "unity.h"

// File System Selection - Choose one:
// #define TEST_FS_SPIFFS
// #define TEST_FS_LITTLEFS
#define TEST_FS_SD
// #define TEST_FS_SDFAT

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

void deleteAllLogFiles(fs::FS &filesystem)
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
    LOG_SET_STORAGE_ROTATING_BUFFERED(TEST_FS, LOG_STORAGE_FILE_PATH);

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
    deleteAllLogFiles(TEST_FS);
    LOG_SET_STORAGE_ROTATING_BUFFERED(TEST_FS);

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
    deleteAllLogFiles(TEST_FS);
    LOG_SET_STORAGE_ROTATING_BUFFERED(TEST_FS);

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
    deleteAllLogFiles(TEST_FS);
    LOG_SET_STORAGE_ROTATING_BUFFERED(TEST_FS);

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
    deleteAllLogFiles(TEST_FS);
    LOG_SET_STORAGE_ROTATING_BUFFERED(TEST_FS);

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
    deleteAllLogFiles(TEST_FS);
    LOG_SET_STORAGE_ROTATING_BUFFERED(TEST_FS);

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
    LOG_SET_STORAGE_ROTATING_BUFFERED(TEST_FS);
    deleteAllLogFiles(TEST_FS);

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
    deleteAllLogFiles(TEST_FS);
    LOG_SET_STORAGE_ROTATING_BUFFERED(TEST_FS);

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
    deleteAllLogFiles(TEST_FS);
    LOG_SET_STORAGE_ROTATING_BUFFERED(TEST_FS);

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
    deleteAllLogFiles(TEST_FS);
    LOG_SET_STORAGE_ROTATING_BUFFERED(TEST_FS);

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
    deleteAllLogFiles(TEST_FS);

    const char *path = "/test_log_noext";
    LOG_SET_STORAGE_ROTATING_BUFFERED(TEST_FS, path);

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
    deleteAllLogFiles(TEST_FS);

    // Multiple extensions: last extension should be preserved.
    // FileManager::rotate() splits on the last '.', so expected rotated name is: /log.txt.1.md
    const char *path = "/log.txt.md";
    LOG_SET_STORAGE_ROTATING_BUFFERED(TEST_FS, path);

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

void test_storage_file_naming_with_subdirectory()
{
#if defined(TEST_FS_SPIFFS)
    TEST_IGNORE_MESSAGE("SPIFFS does not support directories");
#endif

    deleteAllLogFiles(TEST_FS);

    // Create subdirectory
    const char *dir = "/logs";
    const char *path = "/logs/app.txt";

    // Ensure directory exists (create if needed)
    if (!TEST_FS.exists(dir))
    {
        TEST_FS.mkdir(dir);
    }
    TEST_ASSERT_TRUE_MESSAGE(TEST_FS.exists(dir), "Subdirectory should exist");

    LOG_SET_STORAGE_ROTATING_BUFFERED(TEST_FS, path);

    // Generate enough content to trigger rotation
    for (int i = 0; i < 40; i++)
    {
        LOG_WARN("Subdir naming test message {} with extra padding to grow file: 0123456789ABCDEF", i);
        LOG_FLUSH_STORAGE();
        delay(5);
    }

    LOG_FLUSH_STORAGE();

    // Base file should exist
    TEST_ASSERT_TRUE_MESSAGE(TEST_FS.exists(path), "Base file in subdirectory should exist");

    // Rotated file should be /logs/app.1.txt (not /logs/app.txt.1 or /logs.1/app.txt)
    const char *rotatedPath = "/logs/app.1.txt";
    TEST_ASSERT_TRUE_MESSAGE(TEST_FS.exists(rotatedPath),
                             "Rotated file should be /logs/app.1.txt");

    // Sanity: rotated file should be readable and non-empty
    File rotated = TEST_FS.open(rotatedPath, FILE_READ);
    TEST_ASSERT_TRUE_MESSAGE(rotated, "Rotated file in subdirectory should be openable");
    if (rotated)
    {
        size_t size = rotated.size();
        rotated.close();
        TEST_ASSERT_GREATER_THAN_MESSAGE(0, size, "Rotated file in subdirectory should have content");
    }

    // Cleanup subdirectory
    TEST_FS.remove(path);
    TEST_FS.remove(rotatedPath);
    TEST_FS.remove("/logs/app.2.txt");
    TEST_FS.remove("/logs/app.3.txt");
    TEST_FS.rmdir(dir);
}

void test_storage_buffering_real_buffer()
{
    deleteAllLogFiles(TEST_FS);
    LOG_SET_STORAGE_ROTATING_BUFFERED(TEST_FS);

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
    deleteAllLogFiles(TEST_FS);
    LOG_SET_STORAGE_ROTATING_BUFFERED(TEST_FS);

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

void test_storage_message_larger_than_buffer()
{
    deleteAllLogFiles(TEST_FS);
    LOG_SET_STORAGE_ROTATING_BUFFERED(TEST_FS);

    // Create message larger than buffer (256 bytes)
    std::string largePayload(300, 'Y');
    LOG_ERROR("Oversized: {}", largePayload.c_str());
    LOG_FLUSH_STORAGE();

    std::string content = readFile(TEST_FS, LOG_STORAGE_FILE_PATH);
    TEST_ASSERT_TRUE_MESSAGE(content.find(largePayload) != std::string::npos,
                             "Message larger than buffer should be written directly to file");
}

void test_storage_message_larger_than_max_file_size()
{
    deleteAllLogFiles(TEST_FS);
    LOG_SET_STORAGE_ROTATING_BUFFERED(TEST_FS);

    // Create message larger than max file size (512 bytes)
    std::string hugePayload(600, 'Z');
    LOG_ERROR("Huge: {}", hugePayload.c_str());
    LOG_FLUSH_STORAGE();

    // File should exist and contain the message (oversized first write allowed)
    TEST_ASSERT_TRUE_MESSAGE(TEST_FS.exists(LOG_STORAGE_FILE_PATH), "File should exist");

    std::string content = readFile(TEST_FS, LOG_STORAGE_FILE_PATH);
    TEST_ASSERT_TRUE_MESSAGE(content.find(hugePayload) != std::string::npos,
                             "Huge message should be written even if it exceeds max file size");

    // File size will exceed max - that's expected for oversized single writes
    size_t size = getFileSize(TEST_FS, LOG_STORAGE_FILE_PATH);
    TEST_ASSERT_GREATER_THAN_MESSAGE(LOG_STORAGE_MAX_FILE_SIZE, size,
                                     "File may exceed max size for oversized single write");
}

void test_storage_set_file_path_resets_state()
{
    deleteAllLogFiles(TEST_FS);

    const char *path1 = "/log_path1.txt";
    const char *path2 = "/log_path2.txt";

    // Create concrete RotatingFileSink to access setFilePath
    auto fileManager = std::make_shared<FileManager<decltype(TEST_FS)>>(TEST_FS);
    auto sink = std::make_shared<RotatingFileSink>(fileManager, path1);
    FormatLog::instance().setStorage(sink);

    // Write to first path
    LOG_WARN("Message to path 1");
    LOG_FLUSH_STORAGE();

    TEST_ASSERT_TRUE_MESSAGE(TEST_FS.exists(path1), "First path should exist");
    size_t size1 = getFileSize(TEST_FS, path1);
    TEST_ASSERT_GREATER_THAN_MESSAGE(0, size1, "First path should have content");

    // Change path - this should flush pending data and reset state
    sink->setFilePath(path2);

    // Write to second path
    LOG_WARN("Message to path 2");
    LOG_FLUSH_STORAGE();

    TEST_ASSERT_TRUE_MESSAGE(TEST_FS.exists(path2), "Second path should exist");

    std::string content2 = readFile(TEST_FS, path2);
    TEST_ASSERT_TRUE_MESSAGE(content2.find("Message to path 2") != std::string::npos,
                             "Second path should contain new message");

    // Verify first path still has its original content (wasn't corrupted)
    std::string content1 = readFile(TEST_FS, path1);
    TEST_ASSERT_TRUE_MESSAGE(content1.find("Message to path 1") != std::string::npos,
                             "First path should still have original content");
}

void test_storage_flush_empty_buffer_no_op()
{
    deleteAllLogFiles(TEST_FS);

    // Verify cleanup worked
    TEST_ASSERT_FALSE_MESSAGE(TEST_FS.exists(LOG_STORAGE_FILE_PATH),
                              "File should not exist after cleanup");

    LOG_SET_STORAGE_ROTATING_BUFFERED(TEST_FS);

    // Flush without writing anything
    LOG_FLUSH_STORAGE();

    // File should not be created
    TEST_ASSERT_FALSE_MESSAGE(TEST_FS.exists(LOG_STORAGE_FILE_PATH),
                              "Flushing empty buffer should not create file");
}

void test_storage_multiple_flushes_same_content()
{
    deleteAllLogFiles(TEST_FS);
    LOG_SET_STORAGE_ROTATING_BUFFERED(TEST_FS);

    LOG_WARN("Single message");
    LOG_FLUSH_STORAGE();

    size_t sizeAfterFirst = getFileSize(TEST_FS, LOG_STORAGE_FILE_PATH);

    // Flush again with no new content
    LOG_FLUSH_STORAGE();
    LOG_FLUSH_STORAGE();

    size_t sizeAfterMultiple = getFileSize(TEST_FS, LOG_STORAGE_FILE_PATH);

    TEST_ASSERT_EQUAL_MESSAGE(sizeAfterFirst, sizeAfterMultiple,
                              "Multiple flushes with no new content should not change file size");
}

void test_storage_write_null_data_returns_false()
{
    deleteAllLogFiles(TEST_FS);

    auto sink = createRotatingFileSink(TEST_FS, LOG_STORAGE_FILE_PATH);

    // Direct API test - null data should return false
    bool result = sink->write(nullptr, 10);
    TEST_ASSERT_FALSE_MESSAGE(result, "write(nullptr, size) should return false");

    // Zero size should also return false
    result = sink->write("data", 0);
    TEST_ASSERT_FALSE_MESSAGE(result, "write(data, 0) should return false");
}

void test_storage_rotation_preserves_content_order()
{
    deleteAllLogFiles(TEST_FS);
    LOG_SET_STORAGE_ROTATING_BUFFERED(TEST_FS);

    // Write distinct numbered messages
    for (int i = 0; i < 40; i++)
    {
        LOG_WARN("ORDER_TEST_{:03d}_PADDING_TO_FILL_BUFFER_FASTER", i);
        LOG_FLUSH_STORAGE();
        delay(5);
    }

    // Read all files and check that lower numbers are in older (higher index) files
    std::string mainContent = readFile(TEST_FS, LOG_STORAGE_FILE_PATH);
    std::string rotated1 = readFile(TEST_FS, "/test_log.1.txt");

    // Main file should have the most recent messages
    // Rotated file should have older messages

    // Find highest message number in rotated file
    int highestInRotated = -1;
    for (int i = 0; i < 40; i++)
    {
        char marker[32];
        snprintf(marker, sizeof(marker), "ORDER_TEST_%03d", i);
        if (rotated1.find(marker) != std::string::npos)
        {
            highestInRotated = i;
        }
    }

    // Find lowest message number in main file
    int lowestInMain = 40;
    for (int i = 0; i < 40; i++)
    {
        char marker[32];
        snprintf(marker, sizeof(marker), "ORDER_TEST_%03d", i);
        if (mainContent.find(marker) != std::string::npos)
        {
            lowestInMain = i;
            break;
        }
    }

    // Rotated file's highest should be less than main file's lowest
    if (highestInRotated >= 0 && lowestInMain < 40)
    {
        TEST_ASSERT_LESS_THAN_MESSAGE(lowestInMain, highestInRotated,
                                      "Rotated file should contain older messages than main file");
    }
}

void test_storage_buffer_boundary_exact_fit()
{
    deleteAllLogFiles(TEST_FS);
    LOG_SET_STORAGE_ROTATING_BUFFERED(TEST_FS);

    // Write initial content
    LOG_WARN("Initial");
    LOG_FLUSH_STORAGE();

    size_t initialSize = getFileSize(TEST_FS, LOG_STORAGE_FILE_PATH);

    // Try to write exactly at buffer boundary
    // Buffer is 256 bytes, write something that should just fit
    std::string nearLimit(200, 'B');
    LOG_WARN("{}", nearLimit.c_str());

    // Should still be buffered (not auto-flushed yet)
    size_t sizeBeforeFlush = getFileSize(TEST_FS, LOG_STORAGE_FILE_PATH);
    TEST_ASSERT_EQUAL_MESSAGE(initialSize, sizeBeforeFlush,
                              "Content near buffer limit should still be buffered");

    LOG_FLUSH_STORAGE();

    size_t sizeAfterFlush = getFileSize(TEST_FS, LOG_STORAGE_FILE_PATH);
    TEST_ASSERT_GREATER_THAN_MESSAGE(initialSize, sizeAfterFlush,
                                     "Content should be written after flush");
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
    // Flush any remaining buffered data before cleanup
    LOG_FLUSH_STORAGE();
    // Clean up after each test
    deleteAllLogFiles(TEST_FS);
}

void tests()
{
    // Basic functionality
    RUN_TEST(test_storage_initialization);
    RUN_TEST(test_storage_level_filtering);
    RUN_TEST(test_storage_empty_logs);

    // Buffering
    RUN_TEST(test_storage_buffering);
    RUN_TEST(test_storage_manual_flush);
    RUN_TEST(test_storage_buffering_real_buffer);
    RUN_TEST(test_storage_auto_flush_on_buffer_full);
    RUN_TEST(test_storage_flush_empty_buffer_no_op);
    RUN_TEST(test_storage_multiple_flushes_same_content);
    RUN_TEST(test_storage_buffer_boundary_exact_fit);

    // Large messages / edge cases
    RUN_TEST(test_storage_large_message);
    RUN_TEST(test_storage_message_larger_than_buffer);
    RUN_TEST(test_storage_message_larger_than_max_file_size);
    RUN_TEST(test_storage_write_null_data_returns_false);

    // File rotation
    RUN_TEST(test_storage_file_rotation);
    RUN_TEST(test_storage_max_files_limit);
    RUN_TEST(test_storage_rotation_preserves_content_order);
    RUN_TEST(test_storage_rotated_files_are_readable);

    // File naming
    RUN_TEST(test_storage_file_naming);
    RUN_TEST(test_storage_file_naming_no_extension);
    RUN_TEST(test_storage_file_naming_multiple_extensions);
    RUN_TEST(test_storage_file_naming_with_subdirectory);

    // Configuration changes
    RUN_TEST(test_storage_set_file_path_resets_state);
}

void setup()
{
    Serial.begin(115200);
    delay(3000); // Wait for serial monitor

#ifdef TEST_FS_SDFAT
    if (!TEST_FS.begin())
    {
        TEST_FAIL_MESSAGE(TEST_FS_NAME " initialization failed");
    }
#elif defined(TEST_FS_SD)
    SPI.begin();
    if (!TEST_FS.begin(SS, SPI, 4000000, "/sd", 10, true))
    {
        TEST_FAIL_MESSAGE(TEST_FS_NAME " initialization failed");
    }
#else
    if (!TEST_FS.begin(true))
    {
        TEST_FAIL_MESSAGE(TEST_FS_NAME " initialization failed");
    }
#endif

    deleteAllLogFiles(TEST_FS);

    UNITY_BEGIN();
    tests();
    UNITY_END();
}

void loop()
{
    // Nothing to do
}
