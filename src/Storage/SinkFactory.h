#pragma once

#include <memory>
#include "Config/Settings.h"
#include "IFileSink.h"
#include "FileManager.h"
#include "DirectFileSink.h"
#include "RotatingFileSink.h"
#include "BufferedSink.h"

/**
 * Factory function to create a DirectFileSink with FileManager
 * @param fs Reference to the file system (SPIFFS, LittleFS, SD, SdFat)
 * @param filePath Path to the log file
 * @return Shared pointer to DirectFileSink
 */
template <typename TFileSystem>
std::shared_ptr<IFileSink> createDirectFileSink(TFileSystem &fs,
                                                const char *filePath = LOG_STORAGE_FILE_PATH)
{
    auto fileManager = std::make_shared<FileManager<TFileSystem>>(fs);
    auto fileSink = std::make_shared<DirectFileSink>(fileManager, filePath);
    return fileSink;
}

/**
 * Factory function to create a RotatingFileSink with FileManager
 * @param fs Reference to the file system (SPIFFS, LittleFS, SD, SdFat)
 * @param filePath Path to the log file
 * @param maxFiles Maximum number of rotated files to keep (eg. "3" keeps .1, .2, .3, main file)
 * @param maxFileSize Maximum size of each log file before rotation
 * @param rotateOnInit Whether to rotate the existing log file on initialization
 * @return Shared pointer to RotatingFileSink
 */
template <typename TFileSystem>
std::shared_ptr<IFileSink> createRotatingFileSink(TFileSystem &fs,
                                                  const char *filePath = LOG_STORAGE_FILE_PATH,
                                                  size_t maxFiles = LOG_STORAGE_MAX_FILES,
                                                  size_t maxFileSize = LOG_STORAGE_MAX_FILE_SIZE,
                                                  bool rotateOnInit = LOG_STORAGE_NEW_FILE_ON_BOOT)
{
    auto fileManager = std::make_shared<FileManager<TFileSystem>>(fs);
    auto fileSink = std::make_shared<RotatingFileSink>(fileManager, filePath, maxFiles, maxFileSize, rotateOnInit);
    return fileSink;
}

/**
 * Factory function to create a BufferedSink wrapping a DirectFileSink
 * @param fs Reference to the file system (SPIFFS, LittleFS, SD, SdFat)
 * @param filePath Path to the log file
 * @return Shared pointer to BufferedSink
 */
template <typename TFileSystem>
std::shared_ptr<IFileSink> createBufferedDirectFileSink(TFileSystem &fs,
                                                        const char *filePath = LOG_STORAGE_FILE_PATH)
{
    auto inner = createDirectFileSink(fs, filePath);
    return std::make_shared<BufferedSink>(inner);
}

/**
 * Factory function to create a BufferedSink wrapping a RotatingFileSink
 * @param fs Reference to the file system (SPIFFS, LittleFS, SD, SdFat)
 * @param filePath Path to the log file
 * @param maxFiles Maximum number of rotated files to keep (eg. "3" keeps .1, .2, .3, main file)
 * @param maxFileSize Maximum size of each log file before rotation
 * @param rotateOnInit Whether to rotate the existing log file on initialization
 * @return Shared pointer to BufferedSink
 */
template <typename TFileSystem>
std::shared_ptr<IFileSink> createBufferedRotatingFileSink(TFileSystem &fs,
                                                          const char *filePath = LOG_STORAGE_FILE_PATH,
                                                          size_t maxFiles = LOG_STORAGE_MAX_FILES,
                                                          size_t maxFileSize = LOG_STORAGE_MAX_FILE_SIZE,
                                                          bool rotateOnInit = LOG_STORAGE_NEW_FILE_ON_BOOT)
{
    auto inner = createRotatingFileSink(fs, filePath, maxFiles, maxFileSize, rotateOnInit);
    return std::make_shared<BufferedSink>(inner);
}
