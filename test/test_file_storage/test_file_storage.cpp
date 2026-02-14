#include <Arduino.h>
#include <string>
#include <vector>
#include "unity.h"

// * File System Selection - Choose one:
// #define TEST_FS_SPIFFS
#define TEST_FS_LITTLEFS
// #define TEST_FS_SD
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

#define LOG_FILE_ENABLE 1
#define LOG_FILE_LEVEL LOG_LEVEL_WARN
#define LOG_FILE_MAX_BUFFER_SIZE 256
#define LOG_FILE_MAX_SIZE 512 // Small for quick rotation
#define LOG_FILE_MAX_FILES 3
#define LOG_FILE_PATH "/test_log.txt"

#include <FormatLog.h>

#include "IFileSystemUtils.h"
#ifdef TEST_FS_SDFAT
#include "SdFatFileSystemUtils.h"
#else
#include "Esp32FileSystemUtils.h"
#endif

std::shared_ptr<IFileSystemUtils> fsUtils;

/*------------------------------------------------------------------------------
 * Test Cases
 *----------------------------------------------------------------------------*/

void test_storage_initialization()
{
    LOG_SET_FILE_STORAGE(TEST_FS, LOG_FILE_PATH);

    // File should not exist yet (lazy initialization)
    TEST_ASSERT_FALSE_MESSAGE(fsUtils->exists(LOG_FILE_PATH), "File should not exist before first log");

    // Write a warning message
    LOG_WARN("Test warning message");

    // Flush to create file
    LOG_FLUSH_FILE();

    // Now file should exist
    TEST_ASSERT_TRUE_MESSAGE(fsUtils->exists(LOG_FILE_PATH), "File should exist after flush");

    // Verify content
    std::string content = fsUtils->readFile(LOG_FILE_PATH);
    TEST_ASSERT_TRUE_MESSAGE(content.find("Test warning message") != std::string::npos,
                             "File should contain the warning message");
}

void test_storage_level_filtering()
{
    fsUtils->deleteAllFiles();
    LOG_SET_FILE_STORAGE(TEST_FS);

    // TRACE and DEBUG should not go to storage
    LOG_TRACE("Trace message - should not be in file");
    LOG_DEBUG("Debug message - should not be in file");
    LOG_INFO("Info message - should not be in file");

    LOG_FLUSH_FILE();

    TEST_ASSERT_FALSE_MESSAGE(fsUtils->exists(LOG_FILE_PATH),
                              "File should not exist - no WARN+ messages logged");

    // WARN and ERROR should go to storage
    LOG_WARN("Warning message - should be in file");
    LOG_ERROR("Error message - should be in file");

    LOG_FLUSH_FILE();

    TEST_ASSERT_TRUE_MESSAGE(fsUtils->exists(LOG_FILE_PATH), "File should exist after WARN/ERROR and flush");

    std::string content = fsUtils->readFile(LOG_FILE_PATH);
    TEST_ASSERT_TRUE_MESSAGE(content.find("Warning message") != std::string::npos, "Should contain WARN");
    TEST_ASSERT_TRUE_MESSAGE(content.find("Error message") != std::string::npos, "Should contain ERROR");
    TEST_ASSERT_TRUE_MESSAGE(content.find("Trace message") == std::string::npos, "Should NOT contain TRACE");
}

void test_storage_buffering()
{
    fsUtils->deleteAllFiles();
    LOG_SET_FILE_STORAGE(TEST_FS);

    // Log 2 messages (below buffer size limit)
    LOG_WARN("Message 1");
    LOG_WARN("Message 2");

    // File may not exist yet - data is buffered
    size_t sizeBeforeFlush = fsUtils->getFileSize(LOG_FILE_PATH);

    // Manually flush
    LOG_FLUSH_FILE();

    size_t sizeAfterFlush = fsUtils->getFileSize(LOG_FILE_PATH);

    // Size should be greater after flush
    TEST_ASSERT_GREATER_THAN_MESSAGE(sizeBeforeFlush, sizeAfterFlush,
                                     "File size should increase after flush");
}

void test_storage_manual_flush()
{
    fsUtils->deleteAllFiles();
    LOG_SET_FILE_STORAGE(TEST_FS);

    LOG_WARN("Before flush");

    size_t sizeBefore = fsUtils->getFileSize(LOG_FILE_PATH);

    LOG_FLUSH_FILE();

    size_t sizeAfter = fsUtils->getFileSize(LOG_FILE_PATH);

    TEST_ASSERT_GREATER_THAN_MESSAGE(sizeBefore, sizeAfter,
                                     "Manual flush should write buffered data");

    std::string content = fsUtils->readFile(LOG_FILE_PATH);
    TEST_ASSERT_TRUE_MESSAGE(content.find("Before flush") != std::string::npos,
                             "Content should be present after manual flush");
}

void test_storage_file_rotation()
{
    fsUtils->deleteAllFiles();
    LOG_SET_FILE_STORAGE(TEST_FS);

    // Generate enough messages to exceed file size limit (512 bytes)
    for (int i = 0; i < 30; i++)
    {
        LOG_WARN("Log message number {} - adding content to trigger rotation", i);
        LOG_FLUSH_FILE(); // Flush each time to ensure write
        delay(10);
    }

    // Should have created rotated files
    int fileCount = fsUtils->countLogFiles(LOG_FILE_PATH);
    TEST_ASSERT_GREATER_THAN_MESSAGE(1, fileCount,
                                     "Should have multiple log files after rotation");

    // Check main file exists
    TEST_ASSERT_TRUE_MESSAGE(fsUtils->exists(LOG_FILE_PATH), "Main log file should exist");

    // Check at least one rotated file exists
    bool hasRotated = fsUtils->exists("/test_log.1.txt");
    TEST_ASSERT_TRUE_MESSAGE(hasRotated, "At least one rotated file should exist");
}

void test_storage_max_files_limit()
{
    fsUtils->deleteAllFiles();
    LOG_SET_FILE_STORAGE(TEST_FS);

    // Generate many messages to create multiple rotations
    for (int i = 0; i < 60; i++)
    {
        LOG_ERROR("Rotation test message {} with some extra content for size", i);
        LOG_FLUSH_FILE();
        delay(10);
    }

    // Should not exceed MAX_FILES + 1 (current file + 3 rotated)
    int fileCount = fsUtils->countLogFiles(LOG_FILE_PATH);
    TEST_ASSERT_LESS_OR_EQUAL_MESSAGE(LOG_FILE_MAX_FILES + 1, fileCount,
                                      "Should not exceed max files limit");

    // File 4 should not exist (beyond max)
    TEST_ASSERT_FALSE_MESSAGE(fsUtils->exists("/test_log.4.txt"),
                              "Should not create files beyond max limit");
}

void test_storage_file_naming()
{
    LOG_SET_FILE_STORAGE(TEST_FS);
    fsUtils->deleteAllFiles();

    // Generate enough content for rotation
    for (int i = 0; i < 25; i++)
    {
        LOG_WARN("Naming test message {} with sufficient content", i);
        LOG_FLUSH_FILE();
        delay(10);
    }

    // Check proper naming sequence
    TEST_ASSERT_TRUE_MESSAGE(fsUtils->exists(LOG_FILE_PATH), "Main file should exist");

    if (fsUtils->exists("/test_log.1.txt"))
    {
        size_t size1 = fsUtils->getFileSize("/test_log.1.txt");
        TEST_ASSERT_GREATER_THAN_MESSAGE(0, size1, "Rotated file 1 should have content");

        // If .1 exists, it should be near the max size
        TEST_ASSERT_GREATER_OR_EQUAL_MESSAGE(LOG_FILE_MAX_SIZE / 2, size1,
                                             "Rotated file should have substantial content");
    }
}

void test_storage_empty_logs()
{
    fsUtils->deleteAllFiles();
    LOG_SET_FILE_STORAGE(TEST_FS);

    // Log only non-storage level messages
    LOG_TRACE("Trace");
    LOG_DEBUG("Debug");
    LOG_INFO("Info");

    LOG_FLUSH_FILE();

    // Should not create file
    TEST_ASSERT_FALSE_MESSAGE(fsUtils->exists(LOG_FILE_PATH),
                              "Should not create file for below-threshold messages");
}

void test_storage_large_message()
{
    fsUtils->deleteAllFiles();
    LOG_SET_FILE_STORAGE(TEST_FS);

    // Create a large message
    std::string largePayload(200, 'X');
    LOG_ERROR("Large message: {}", largePayload.c_str());
    LOG_FLUSH_FILE();

    std::string content = fsUtils->readFile(LOG_FILE_PATH);
    TEST_ASSERT_TRUE_MESSAGE(content.find(largePayload) != std::string::npos,
                             "Large message should be stored correctly");
}

void test_storage_rotated_files_are_readable()
{
    fsUtils->deleteAllFiles();
    LOG_SET_FILE_STORAGE(TEST_FS);

    // Create enough content to trigger at least one rotation.
    for (int i = 0; i < 40; i++)
    {
        LOG_WARN("Rotate-read test message {} with extra padding to grow file: 0123456789ABCDEF", i);
        LOG_FLUSH_FILE();
        delay(5);
    }

    // Ensure all data is flushed so we can read deterministically.
    LOG_FLUSH_FILE();

    // Main file should exist and be readable.
    TEST_ASSERT_TRUE_MESSAGE(fsUtils->exists(LOG_FILE_PATH), "Main log should exist");
    size_t mainSize = fsUtils->getFileSize(LOG_FILE_PATH);
    TEST_ASSERT_GREATER_THAN_MESSAGE(0, mainSize, "Main log should have content");

    // At least one rotated file should exist and be readable when rotation happened.
    if (fsUtils->exists("/test_log.1.txt"))
    {
        size_t rotatedSize = fsUtils->getFileSize("/test_log.1.txt");
        TEST_ASSERT_GREATER_THAN_MESSAGE(0, rotatedSize, "Rotated log should have content");
    }
    else
    {
        // If rotation didn't occur, fail loudly so we don't silently skip the intent.
        TEST_FAIL_MESSAGE("Expected rotation to create /test_log.1.txt, but it was missing");
    }
}

void test_storage_file_naming_no_extension()
{
    fsUtils->deleteAllFiles();

    const char *path = "/test_log_noext";
    LOG_SET_FILE_STORAGE(TEST_FS, path);

    // Generate enough content to trigger at least one rotation.
    for (int i = 0; i < 40; i++)
    {
        LOG_WARN("No-ext naming test message {} with extra padding to grow file: 0123456789ABCDEF", i);
        LOG_FLUSH_FILE();
        delay(5);
    }

    LOG_FLUSH_FILE();

    // Base file should exist (it will be recreated after rotation when next written, but we log enough times).
    TEST_ASSERT_TRUE_MESSAGE(fsUtils->exists(path), "Base no-ext log file should exist");

    // Rotated file should follow: <name>.1 (no trailing extension)
    TEST_ASSERT_TRUE_MESSAGE(fsUtils->exists("/test_log_noext.1"),
                             "Rotated no-ext file should be named /test_log_noext.1");

    // Sanity: rotated file should be readable and non-empty
    size_t rotatedSize = fsUtils->getFileSize("/test_log_noext.1");
    TEST_ASSERT_GREATER_THAN_MESSAGE(0, rotatedSize, "Rotated no-ext file should have content");
}

void test_storage_file_naming_multiple_extensions()
{
    fsUtils->deleteAllFiles();

    // Multiple extensions: last extension should be preserved.
    // FileManager::rotate() splits on the last '.', so expected rotated name is: /log.txt.1.md
    const char *path = "/log.txt.md";
    LOG_SET_FILE_STORAGE(TEST_FS, path);

    for (int i = 0; i < 45; i++)
    {
        LOG_WARN("Multi-ext naming test message {} with extra padding to grow file: 0123456789ABCDEF", i);
        LOG_FLUSH_FILE();
        delay(5);
    }

    LOG_FLUSH_FILE();

    TEST_ASSERT_TRUE_MESSAGE(fsUtils->exists(path), "Base multi-ext log file should exist");
    TEST_ASSERT_TRUE_MESSAGE(fsUtils->exists("/log.txt.1.md"),
                             "Rotated multi-ext file should be named /log.txt.1.md");

    // Ensure we are not rotating as /log.1.txt.md (wrong split point)
    TEST_ASSERT_FALSE_MESSAGE(fsUtils->exists("/log.1.txt.md"),
                              "Should not rotate using the first extension segment");

    // Sanity: rotated file should be readable and non-empty
    size_t rotatedSize = fsUtils->getFileSize("/log.txt.1.md");
    TEST_ASSERT_GREATER_THAN_MESSAGE(0, rotatedSize, "Rotated multi-ext file should have content");
}

void test_storage_file_naming_with_subdirectory()
{
#if defined(TEST_FS_SPIFFS)
    TEST_IGNORE_MESSAGE("SPIFFS does not support directories");
#endif

    fsUtils->deleteAllFiles();

    // Create subdirectory
    const char *dir = "/logs";
    const char *path = "/logs/app.txt";

    // Ensure directory exists (create if needed)
    if (!fsUtils->exists(dir))
    {
        fsUtils->mkdir(dir);
    }
    TEST_ASSERT_TRUE_MESSAGE(fsUtils->exists(dir), "Subdirectory should exist");

    LOG_SET_FILE_STORAGE(TEST_FS, path);

    // Generate enough content to trigger rotation
    for (int i = 0; i < 40; i++)
    {
        LOG_WARN("Subdir naming test message {} with extra padding to grow file: 0123456789ABCDEF", i);
        LOG_FLUSH_FILE();
        delay(5);
    }

    LOG_FLUSH_FILE();

    // Base file should exist
    TEST_ASSERT_TRUE_MESSAGE(fsUtils->exists(path), "Base file in subdirectory should exist");

    // Rotated file should be /logs/app.1.txt (not /logs/app.txt.1 or /logs.1/app.txt)
    const char *rotatedPath = "/logs/app.1.txt";
    TEST_ASSERT_TRUE_MESSAGE(fsUtils->exists(rotatedPath),
                             "Rotated file should be /logs/app.1.txt");

    // Sanity: rotated file should be readable and non-empty
    size_t rotatedSize = fsUtils->getFileSize(rotatedPath);
    TEST_ASSERT_GREATER_THAN_MESSAGE(0, rotatedSize, "Rotated file in subdirectory should have content");

    // Cleanup subdirectory
    fsUtils->remove(path);
    fsUtils->remove(rotatedPath);
    fsUtils->remove("/logs/app.2.txt");
    fsUtils->remove("/logs/app.3.txt");
    fsUtils->rmdir(dir);
}

void test_storage_buffering_real_buffer()
{
    fsUtils->deleteAllFiles();
    LOG_SET_FILE_STORAGE(TEST_FS);

    // Write some initial content and flush
    LOG_WARN("Initial message");
    LOG_FLUSH_FILE();

    size_t initialSize = fsUtils->getFileSize(LOG_FILE_PATH);
    TEST_ASSERT_GREATER_THAN_MESSAGE(0, initialSize, "Initial file should have content");

    // Write two messages without flushing (should remain in buffer)
    LOG_WARN("Buffered message 1");
    LOG_WARN("Buffered message 2");

    // File size should not have increased yet (messages are in buffer)
    size_t sizeBeforeFlush = fsUtils->getFileSize(LOG_FILE_PATH);
    TEST_ASSERT_EQUAL_MESSAGE(initialSize, sizeBeforeFlush,
                              "File size should not increase before flush (messages in buffer)");

    // Flush and verify size increased
    LOG_FLUSH_FILE();
    size_t sizeAfterFlush = fsUtils->getFileSize(LOG_FILE_PATH);
    TEST_ASSERT_GREATER_THAN_MESSAGE(sizeBeforeFlush, sizeAfterFlush,
                                     "File size should increase after flushing buffer");

    // Verify content is correct
    std::string content = fsUtils->readFile(LOG_FILE_PATH);
    TEST_ASSERT_TRUE_MESSAGE(content.find("Buffered message 1") != std::string::npos,
                             "Buffered message 1 should be in file after flush");
    TEST_ASSERT_TRUE_MESSAGE(content.find("Buffered message 2") != std::string::npos,
                             "Buffered message 2 should be in file after flush");
}

void test_storage_auto_flush_on_buffer_full()
{
    fsUtils->deleteAllFiles();
    LOG_SET_FILE_STORAGE(TEST_FS);

    // Fill buffer with messages (256 byte limit)
    // Each message with preamble is roughly 25-30 bytes, so 10 messages should fill the buffer
    for (int i = 0; i < 10; i++)
    {
        LOG_WARN("Buffer fill message {} with extra padding", i);
    }

    // Buffer should have auto-flushed, so file should exist and have content
    size_t size = fsUtils->getFileSize(LOG_FILE_PATH);
    TEST_ASSERT_GREATER_THAN_MESSAGE(0, size, "File should have content after buffer fills and auto-flushes");
}

void test_storage_message_larger_than_buffer()
{
    fsUtils->deleteAllFiles();
    LOG_SET_FILE_STORAGE(TEST_FS);

    // Create message larger than buffer (256 bytes)
    std::string largePayload(300, 'Y');
    LOG_ERROR("Oversized: {}", largePayload.c_str());
    LOG_FLUSH_FILE();

    std::string content = fsUtils->readFile(LOG_FILE_PATH);
    TEST_ASSERT_TRUE_MESSAGE(content.find(largePayload) != std::string::npos,
                             "Message larger than buffer should be written directly to file");
}

void test_storage_message_larger_than_max_file_size()
{
    fsUtils->deleteAllFiles();
    LOG_SET_FILE_STORAGE(TEST_FS);

    // Create message larger than max file size (512 bytes)
    std::string hugePayload(600, 'Z');
    LOG_ERROR("Huge: {}", hugePayload.c_str());
    LOG_FLUSH_FILE();

    // File should exist and contain the message (oversized first write allowed)
    TEST_ASSERT_TRUE_MESSAGE(fsUtils->exists(LOG_FILE_PATH), "File should exist");

    std::string content = fsUtils->readFile(LOG_FILE_PATH);
    TEST_ASSERT_TRUE_MESSAGE(content.find(hugePayload) != std::string::npos,
                             "Huge message should be written even if it exceeds max file size");

    // File size will exceed max - that's expected for oversized single writes
    size_t size = fsUtils->getFileSize(LOG_FILE_PATH);
    TEST_ASSERT_GREATER_THAN_MESSAGE(LOG_FILE_MAX_SIZE, size,
                                     "File may exceed max size for oversized single write");
}

void test_storage_set_file_path_resets_state()
{
    fsUtils->deleteAllFiles();

    const char *path1 = "/log_path1.txt";
    const char *path2 = "/log_path2.txt";

    // Create concrete RotatingFileSink to access setFilePath
    auto fileManager = fmtlog::createFileManager(TEST_FS);
    auto sink = std::make_shared<fmtlog::RotatingFileSink<>>(fileManager, path1);
    fmtlog::FormatLog::instance().setFileStorage(sink);

    // Write to first path
    LOG_WARN("Message to path 1");
    LOG_FLUSH_FILE();

    TEST_ASSERT_TRUE_MESSAGE(fsUtils->exists(path1), "First path should exist");
    size_t size1 = fsUtils->getFileSize(path1);
    TEST_ASSERT_GREATER_THAN_MESSAGE(0, size1, "First path should have content");

    // Change path - this should flush pending data and reset state
    sink->setFilePath(path2);

    // Write to second path
    LOG_WARN("Message to path 2");
    LOG_FLUSH_FILE();

    TEST_ASSERT_TRUE_MESSAGE(fsUtils->exists(path2), "Second path should exist");

    std::string content2 = fsUtils->readFile(path2);
    TEST_ASSERT_TRUE_MESSAGE(content2.find("Message to path 2") != std::string::npos,
                             "Second path should contain new message");

    // Verify first path still has its original content (wasn't corrupted)
    std::string content1 = fsUtils->readFile(path1);
    TEST_ASSERT_TRUE_MESSAGE(content1.find("Message to path 1") != std::string::npos,
                             "First path should still have original content");
}

void test_storage_flush_empty_buffer_no_op()
{
    fsUtils->deleteAllFiles();

    // Verify cleanup worked
    TEST_ASSERT_FALSE_MESSAGE(fsUtils->exists(LOG_FILE_PATH),
                              "File should not exist after cleanup");

    LOG_SET_FILE_STORAGE(TEST_FS);

    // Flush without writing anything
    LOG_FLUSH_FILE();

    // File should not be created
    TEST_ASSERT_FALSE_MESSAGE(fsUtils->exists(LOG_FILE_PATH),
                              "Flushing empty buffer should not create file");
}

void test_storage_multiple_flushes_same_content()
{
    fsUtils->deleteAllFiles();
    LOG_SET_FILE_STORAGE(TEST_FS);

    LOG_WARN("Single message");
    LOG_FLUSH_FILE();

    size_t sizeAfterFirst = fsUtils->getFileSize(LOG_FILE_PATH);

    // Flush again with no new content
    LOG_FLUSH_FILE();
    LOG_FLUSH_FILE();

    size_t sizeAfterMultiple = fsUtils->getFileSize(LOG_FILE_PATH);

    TEST_ASSERT_EQUAL_MESSAGE(sizeAfterFirst, sizeAfterMultiple,
                              "Multiple flushes with no new content should not change file size");
}

void test_storage_rotation_preserves_content_order()
{
    fsUtils->deleteAllFiles();
    LOG_SET_FILE_STORAGE(TEST_FS);

    // Write distinct numbered messages
    for (int i = 0; i < 40; i++)
    {
        LOG_WARN("ORDER_TEST_{:03d}_PADDING_TO_FILL_BUFFER_FASTER", i);
        LOG_FLUSH_FILE();
        delay(5);
    }

    // Read all files and check that lower numbers are in older (higher index) files
    std::string mainContent = fsUtils->readFile(LOG_FILE_PATH);
    std::string rotated1 = fsUtils->readFile("/test_log.1.txt");

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
    fsUtils->deleteAllFiles();
    LOG_SET_FILE_STORAGE(TEST_FS);

    // Write initial content
    LOG_WARN("Initial");
    LOG_FLUSH_FILE();

    size_t initialSize = fsUtils->getFileSize(LOG_FILE_PATH);

    // Try to write exactly at buffer boundary
    // Buffer is 256 bytes, write something that should just fit
    std::string nearLimit(200, 'B');
    LOG_WARN("{}", nearLimit.c_str());

    // Should still be buffered (not auto-flushed yet)
    size_t sizeBeforeFlush = fsUtils->getFileSize(LOG_FILE_PATH);
    TEST_ASSERT_EQUAL_MESSAGE(initialSize, sizeBeforeFlush,
                              "Content near buffer limit should still be buffered");

    LOG_FLUSH_FILE();

    size_t sizeAfterFlush = fsUtils->getFileSize(LOG_FILE_PATH);
    TEST_ASSERT_GREATER_THAN_MESSAGE(initialSize, sizeAfterFlush,
                                     "Content should be written after flush");
}

void test_storage_buffer_overflow_and_file_rotation()
{
    fsUtils->deleteAllFiles();
    LOG_SET_FILE_STORAGE(TEST_FS);

    // Scenario: Buffer is nearly full, file is nearly full
    // Buffer: 256 bytes max
    // File: 512 bytes max
    // Fill file to near capacity (480 bytes)
    for (int i = 0; i < 20; i++)
    {
        LOG_WARN("Fill{:02d}", i); // ~14 bytes each = 280 bytes total
        LOG_FLUSH_FILE();
    }

    size_t fileSize = fsUtils->getFileSize(LOG_FILE_PATH);
    TEST_ASSERT_GREATER_THAN_MESSAGE(250, fileSize, "File should be substantially filled");
    TEST_ASSERT_LESS_THAN_MESSAGE(LOG_FILE_MAX_SIZE, fileSize, "File should not be at max yet");

    // Now add data to buffer without flushing (fill buffer to ~200 bytes)
    std::string bufferFill(180, 'X');
    LOG_WARN("{}", bufferFill.c_str());

    // File size should not change yet (data is buffered)
    size_t fileSizeBeforeOverflow = fsUtils->getFileSize(LOG_FILE_PATH);
    TEST_ASSERT_EQUAL_MESSAGE(fileSize, fileSizeBeforeOverflow, "File should not grow (data buffered)");

    // Now write a message that will:
    // 1. Trigger buffer overflow (buffer 180+ + new 100+ > 256)
    // 2. After flush, total file size will exceed max (file + buffer > 512)
    // Expected: Buffer flushes first, then rotation happens
    std::string triggerMsg(100, 'Y');
    LOG_WARN("{}", triggerMsg.c_str());

    LOG_FLUSH_FILE();

    // After flush, we should have rotated
    // Check that rotation occurred
    bool rotatedFileExists = fsUtils->exists("/test_log.1.txt");
    TEST_ASSERT_TRUE_MESSAGE(rotatedFileExists, "Rotated file should exist");

    // Main file should be small (just the last message)
    size_t newMainFileSize = fsUtils->getFileSize(LOG_FILE_PATH);
    TEST_ASSERT_LESS_THAN_MESSAGE(200, newMainFileSize, "New main file should be small");

    // Rotated file should have the old content
    size_t rotatedFileSize = fsUtils->getFileSize("/test_log.1.txt");
    TEST_ASSERT_GREATER_THAN_MESSAGE(400, rotatedFileSize, "Rotated file should have substantial content");
}

void test_storage_oversized_message_with_file_rotation()
{
    fsUtils->deleteAllFiles();
    LOG_SET_FILE_STORAGE(TEST_FS);

    // Scenario: File is moderately full, oversized message would exceed file size
    // Fill file to ~350 bytes
    for (int i = 0; i < 15; i++)
    {
        LOG_WARN("Prepare{:02d}_content", i);
        LOG_FLUSH_FILE();
    }

    size_t initialFileSize = fsUtils->getFileSize(LOG_FILE_PATH);
    TEST_ASSERT_GREATER_THAN_MESSAGE(200, initialFileSize, "File should be partially filled");
    TEST_ASSERT_LESS_THAN_MESSAGE(LOG_FILE_MAX_SIZE, initialFileSize, "File should be under max");

    // Add some data to buffer (not flushed)
    LOG_WARN("Buffered data");

    // Now write an OVERSIZED message (larger than buffer size of 256)
    // This should:
    // 1. Flush the buffer first (adding to file)
    // 2. Check if oversized message + file > max file size
    // 3. Rotate if needed
    // 4. Write oversized message directly to (new) file
    std::string oversizedMsg(300, 'Z'); // Larger than 256 byte buffer
    LOG_WARN("OVERSIZED:{}", oversizedMsg.c_str());

    LOG_FLUSH_FILE();

    // Check that rotation occurred
    bool rotatedExists = fsUtils->exists("/test_log.1.txt");
    TEST_ASSERT_TRUE_MESSAGE(rotatedExists, "Rotation should have occurred");

    // Rotated file should have the initial content + buffered data
    size_t rotatedSize = fsUtils->getFileSize("/test_log.1.txt");
    TEST_ASSERT_GREATER_THAN_MESSAGE(initialFileSize, rotatedSize, "Rotated file should have initial + buffered content");

    // Main file should have the oversized message
    size_t mainFileSize = fsUtils->getFileSize(LOG_FILE_PATH);
    TEST_ASSERT_GREATER_THAN_MESSAGE(300, mainFileSize, "Main file should have oversized message");
    TEST_ASSERT_LESS_THAN_MESSAGE(LOG_FILE_MAX_SIZE, mainFileSize, "Oversized message should fit in fresh file");

    // Verify content
    std::string mainContent = fsUtils->readFile(LOG_FILE_PATH);
    TEST_ASSERT_TRUE_MESSAGE(mainContent.find(oversizedMsg) != std::string::npos,
                             "Main file should contain the oversized message");
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
    // Close storage (flushes buffer and releases file descriptor) before cleanup.
    // LittleFS requires files to be closed before they can be deleted.
    LOG_CLOSE_FILE();
    // Clean up after each test
    fsUtils->deleteAllFiles();
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

    // Complex scenarios (buffer + file interactions)
    RUN_TEST(test_storage_buffer_overflow_and_file_rotation);
    RUN_TEST(test_storage_oversized_message_with_file_rotation);

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

// SdFs sd;

void setup()
{
    Serial.begin(115200);
    delay(3000); // Wait for serial monitor

#ifdef TEST_FS_SDFAT

    SPI.begin();
    if (!TEST_FS.begin(SS))
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

#ifdef TEST_FS_SDFAT
    fsUtils = std::make_shared<SdFatFileSystemUtils<decltype(TEST_FS)>>(TEST_FS);
#else
    fsUtils = std::make_shared<Esp32FileSystemUtils<decltype(TEST_FS)>>(TEST_FS);
#endif
    fsUtils->deleteAllFiles();

    UNITY_BEGIN();
    tests();
    UNITY_END();
}

void loop()
{
    // Nothing to do
}
