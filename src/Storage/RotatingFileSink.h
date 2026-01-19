#pragma once

#include <string>
#include <memory>
#include <fmt/format.h>
#include "Config/Settings.h"
#include "IRotatingFileSink.h"
#include "FileManager.h"

class RotatingFileSink : public IRotatingFileSink
{
private:
    using Buffer = fmt::basic_memory_buffer<uint8_t, LOG_STORAGE_MAX_BUFFER_SIZE>;

    struct ParsedPath
    {
        std::string name;
        std::string ext;
    };

    std::shared_ptr<IFileManager> _fileManager;
    std::string _filePath;
    Buffer _buffer;
    size_t _maxFiles;
    size_t _maxFileSize;
    size_t _currentSize;
    bool _rotateOnOpen;
    bool _initialized;

    ParsedPath parseFilePath() const
    {
        ParsedPath result;

        // Find last occurrence of '.' and '/'
        size_t dotPos = _filePath.find_last_of('.');
        size_t slashPos = _filePath.find_last_of('/');

        // Ensure dot is after last slash (not in directory name)
        if (dotPos != std::string::npos &&
            (slashPos == std::string::npos || dotPos > slashPos))
        {
            result.name = _filePath.substr(0, dotPos);
            result.ext = _filePath.substr(dotPos); // includes the '.'
        }
        else // No extension
        {
            result.name = _filePath;
            result.ext = "";
        }

        return result;
    }

    std::string calcFilename(const ParsedPath &parsed, size_t index) const
    {
        if (index == 0)
        {
            return _filePath;
        }

        char indexStr[12];
        snprintf(indexStr, sizeof(indexStr), "%zu", index);

        // Format: {basename}.{index}{extension}
        return parsed.name + "." + indexStr + parsed.ext;
    }

    bool shouldFlush() const
    {
        return _buffer.size() >= LOG_STORAGE_MAX_BUFFER_SIZE;
    }

    void initializeFile()
    {
        if (_initialized)
            return;

        if (_fileManager->exists(_filePath.c_str()))
        {
            if (_rotateOnOpen)
            {
                rotate();
                _currentSize = 0;
            }
            else if (_fileManager->open(_filePath.c_str()))
            {
                _currentSize = _fileManager->size(); // Single size() call
                _fileManager->close();
            }
        }

        _initialized = true;
    }

public:
    RotatingFileSink(std::shared_ptr<IFileManager> fileManager, const char *path, bool newFileOnBoot = LOG_STORAGE_NEW_FILE_ON_BOOT)
        : _fileManager(fileManager),
          _filePath(path),
          _maxFiles(LOG_STORAGE_MAX_FILES),
          _maxFileSize(LOG_STORAGE_MAX_FILE_SIZE),
          _currentSize(0),
          _rotateOnOpen(newFileOnBoot),
          _initialized(false)
    {
        // No fileManager calls - defer to first flush
    }

    ~RotatingFileSink() override
    {
        flush();
    }

    bool write(const char *data, size_t size) override
    {
        if (!data || size == 0)
        {
            return false;
        }

        _buffer.append(data, data + size);

        if (shouldFlush())
        {
            flush();
        }

        return true;
    }

    void flush() override
    {
        if (_buffer.size() == 0)
            return;

        initializeFile();

        // Check rotation based on tracked size only
        size_t newSize = _currentSize + _buffer.size();
        if (newSize > _maxFileSize && _currentSize > 0)
        {
            rotate();
            _currentSize = 0;
        }

        if (!_fileManager->open(_filePath.c_str()))
        {
            return;
        }

        size_t written = _fileManager->write(reinterpret_cast<const char *>(_buffer.data()), _buffer.size());
        _currentSize += written;

        _fileManager->flush();
        _fileManager->close();
        _buffer.clear();
    }

    void rotate()
    {
        // If max_files is 0, just restart the main file
        if (_maxFiles == 0)
        {
            _fileManager->remove(_filePath.c_str());
            return;
        }

        // Delete oldest file (index = maxFiles)
        ParsedPath parsed = parseFilePath();
        std::string oldestFile = calcFilename(parsed, _maxFiles);
        _fileManager->remove(oldestFile.c_str());

        // Rename files backwards
        for (size_t i = _maxFiles; i > 0; --i)
        {
            std::string src = calcFilename(parsed, i - 1);

            if (!_fileManager->exists(src.c_str()))
            {
                continue;
            }

            std::string target = calcFilename(parsed, i);

            _fileManager->remove(target.c_str());
            _fileManager->rename(src.c_str(), target.c_str());
        }
    }

    void setMaxFiles(size_t maxFiles) override
    {
        _maxFiles = maxFiles;
    }

    size_t getMaxFiles() const override
    {
        return _maxFiles;
    }

    void setMaxFileSize(size_t maxFileSize) override
    {
        _maxFileSize = maxFileSize;
    }

    size_t getMaxFileSize() const override
    {
        return _maxFileSize;
    }
};

/**
 * Factory function to create a RotatingFileSink with FileManager
 * @param fs Reference to the file system (SPIFFS, LittleFS, SD, SdFat)
 * @param filePath Path to the log file
 * @param newFileOnBoot Whether to create a new log file on boot (rotating the old one)
 * @return Shared pointer to RotatingFileSink
 */
template <typename TFileSystem>
std::shared_ptr<IRotatingFileSink> createStorage(TFileSystem &fs, const char *filePath = LOG_STORAGE_FILE_PATH, bool newFileOnBoot = LOG_STORAGE_NEW_FILE_ON_BOOT)
{
    auto fileManager = std::make_shared<FileManager<TFileSystem>>(fs);
    auto fileSink = std::make_shared<RotatingFileSink>(fileManager, filePath, newFileOnBoot);
    return fileSink;
}