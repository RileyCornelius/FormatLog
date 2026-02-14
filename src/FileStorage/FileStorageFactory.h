#pragma once

#include <memory>
#include "Config/Settings.h"
#include "FileStorage/Sinks/IFileSink.h"
#include "FileStorage/FileSystem/FileManagerFactory.h"
#include "FileStorage/Sinks/RotatingFileSink.h"
#include "FileStorage/Sinks/SimpleFileSink.h"

namespace fmtlog
{

/**
 * Factory function to create a file storage sink with rotating file and buffering.
 *
 * Rotation: Set maxFiles = 0 for single file (no backups, default = 3)
 * Buffer size: Specify BufferSize template parameter or use LOG_FILE_MAX_BUFFER_SIZE default, if set to 0 no buffering is used
 *
 * @tparam TFileSystem Filesystem type (SPIFFS, LittleFS, SD, SdFat)
 * @tparam BufferSize Size of the internal memory buffer (default = LOG_FILE_MAX_BUFFER_SIZE)
 * @param fs Reference to the file system
 * @param filePath Path to the log file
 * @param maxFiles Maximum number of rotated files to keep (eg. "3" keeps .1, .2, .3, main file)
 * @param maxFileSize Maximum size of each log file before rotation
 * @param rotateOnInit Whether to rotate the existing log file on initialization
 * @return Shared pointer to IFileSink
 */
template <typename TFileSystem, size_t BufferSize = LOG_FILE_MAX_BUFFER_SIZE>
std::shared_ptr<IFileSink> createRotatingFileStorage(TFileSystem &fs,
                                                     const char *filePath = LOG_FILE_PATH,
                                                     size_t maxFiles = LOG_FILE_MAX_FILES,
                                                     size_t maxFileSize = LOG_FILE_MAX_SIZE,
                                                     bool rotateOnInit = LOG_FILE_NEW_ON_BOOT)
{
    auto fileManager = createFileManager(fs);
    return std::make_shared<RotatingFileSink<BufferSize>>(fileManager, filePath, maxFiles, maxFileSize, rotateOnInit);
}

/**
 * Factory function to create a simple file storage sink with no buffering or rotation.
 *
 * Writes are flushed directly to the filesystem on each call.
 *
 * @tparam TFileSystem Filesystem type (SPIFFS, LittleFS, SD, SdFat)
 * @param fs Reference to the file system
 * @param filePath Path to the log file
 * @return Shared pointer to IFileSink
 */
template <typename TFileSystem>
std::shared_ptr<IFileSink> createSimpleFileStorage(TFileSystem &fs,
                                                   const char *filePath = LOG_FILE_PATH)
{
    auto fileManager = createFileManager(fs);
    return std::make_shared<SimpleFileSink>(fileManager, filePath);
}

} // namespace fmtlog
