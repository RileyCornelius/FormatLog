#pragma once

#include "FS.h"
#include "Config/Settings.h"
#include <string>

/**--------------------------------------------------------------------------------------
 * Storage Class - File system storage for log messages
 *-------------------------------------------------------------------------------------*/

class Storage
{
private:
    fs::FS *fileSystem;
    std::string filePath;

    File file;
    size_t currentMessageCount = 0;
    size_t currentBufferSize = 0;
    size_t currentFileSize = 0;
    bool fileOpen = false;
    bool justRotated = false;

    bool openFile()
    {
        if (fileOpen)
            return true;

        if (!isMounted())
            return false;

        file = fileSystem->open(filePath.c_str(), FILE_APPEND, true);
        if (!file)
            return false;

        // Read file size on first open, but skip after rotation (filesystem may return stale size)
        if (currentFileSize == 0 && !justRotated)
        {
            currentFileSize = file.size();
        }
        justRotated = false;
        fileOpen = true;

        return true;
    }

    void rotate()
    {
        // Rotation disabled - logging stops at MAX_FILE_SIZE
        if (LOG_STORAGE_MAX_FILES == 0)
            return;

        // Close current file if open
        if (fileOpen)
        {
            file.close();
            fileOpen = false;
        }

        // Reset counters
        currentFileSize = 0;
        currentMessageCount = 0;
        currentBufferSize = 0;
        justRotated = true;

        // Parse file path to get base name and extension
        size_t lastSlash = filePath.find_last_of('/');

        std::string dir = (lastSlash != std::string::npos) ? filePath.substr(0, lastSlash + 1) : "";
        std::string baseName = (lastSlash != std::string::npos) ? filePath.substr(lastSlash + 1) : filePath;
        std::string name, ext;

        size_t dotInBase = baseName.find_last_of('.');
        if (dotInBase != std::string::npos)
        {
            name = baseName.substr(0, dotInBase);
            ext = baseName.substr(dotInBase);
        }
        else
        {
            name = baseName;
            ext = "";
        }

        // Delete oldest file
        if (LOG_STORAGE_MAX_FILES > 0)
        {
            std::string oldestFile = dir + name + "." + std::to_string(LOG_STORAGE_MAX_FILES) + ext;
            if (fileSystem->exists(oldestFile.c_str()))
            {
                fileSystem->remove(oldestFile.c_str());
            }
        }

        // Rename files in reverse order
        for (int i = LOG_STORAGE_MAX_FILES - 1; i >= 1; i--)
        {
            std::string oldName = dir + name + "." + std::to_string(i) + ext;
            std::string newName = dir + name + "." + std::to_string(i + 1) + ext;

            if (fileSystem->exists(oldName.c_str()))
            {
                fileSystem->rename(oldName.c_str(), newName.c_str());
            }
        }

        // Rename current log file to .1
        std::string rotatedName = dir + name + ".1" + ext;
        if (fileSystem->exists(filePath.c_str()))
        {
            fileSystem->rename(filePath.c_str(), rotatedName.c_str());
        }
    }

public:
    Storage() = default;

    Storage(fs::FS &fs, const char *path = LOG_STORAGE_FILE_PATH)
        : fileSystem(&fs), filePath(path) {}

    ~Storage()
    {
        close();
    }

    bool isMounted() const
    {
        return fileSystem != nullptr;
    }

    bool isFileOpen() const
    {
        return fileOpen;
    }

    const char *getFilePath() const
    {
        return filePath.c_str();
    }

    bool write(const char *data, size_t size)
    {
        // Stop writing if rotation disabled and file size limit reached
        if (LOG_STORAGE_MAX_FILES == 0 && currentFileSize >= LOG_STORAGE_MAX_FILE_SIZE)
            return false;

        // Lazy initialization: open file on first write
        if (!openFile())
            return false;

        size_t written = file.write(reinterpret_cast<const uint8_t *>(data), size);
        if (written != size)
            return false;

        currentBufferSize += written;
        currentFileSize += written;
        currentMessageCount++;

        // Check if auto flush needed before writing next message
        if (currentMessageCount >= LOG_STORAGE_MAX_BUFFER_MESSAGES ||
            currentBufferSize >= LOG_STORAGE_MAX_BUFFER_SIZE)
        {
            flush();
        }

        // Check if rotation needed after writing
        if (currentFileSize >= LOG_STORAGE_MAX_FILE_SIZE)
        {
            rotate();
        }

        return true;
    }

    void flush()
    {
        if (!fileOpen || !file)
            return;

        file.flush();
        currentMessageCount = 0;
        currentBufferSize = 0;
    }

    void close()
    {
        if (!fileOpen || !file)
            return;

        file.close();
        fileOpen = false;
        currentMessageCount = 0;
        currentBufferSize = 0;
    }
};
