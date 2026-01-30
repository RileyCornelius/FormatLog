#pragma once

#include <string>
#include <memory>
#include <fmt.h>
#include "Config/Settings.h"
#include "IFileSink.h"
#include "FileManager.h"

class RotatingFileSink : public IFileSink
{
private:
    fmt::basic_memory_buffer<char, LOG_STORAGE_MAX_BUFFER_SIZE> _buffer;
    std::shared_ptr<IFileManager> _fileManager;
    std::string _filePath;
    std::string _baseName;
    std::string _extension;
    size_t _maxFiles;
    size_t _maxFileSize;
    size_t _currentSize;
    bool _rotateOnInit;
    bool _initialized;

    void parseFilePath()
    {
        size_t dotPos = _filePath.find_last_of('.');
        size_t slashPos = _filePath.find_last_of("/\\");

        bool hasExt = dotPos != std::string::npos && (slashPos == std::string::npos || dotPos > slashPos);
        _baseName = hasExt ? _filePath.substr(0, dotPos) : _filePath;
        _extension = hasExt ? _filePath.substr(dotPos) : "";
    }

    std::string createFilePath(size_t index) const
    {
        if (index == 0)
        {
            return _filePath;
        }
        return fmt::format("{}.{}{}", _baseName, index, _extension); // Example: log.1.txt
    }

    bool ensureOpen()
    {
        if (_fileManager->isOpen())
            return true;

        return _fileManager->open(_filePath.c_str());
    }

    void initFile()
    {
        if (_initialized)
            return;

        // Get the existing file size or rotate it
        if (_fileManager->exists(_filePath.c_str()))
        {
            if (_rotateOnInit)
            {
                rotate();
            }
            else if (ensureOpen())
            {
                _currentSize = _fileManager->size();
            }
        }

        _initialized = true;
    }

public:
    RotatingFileSink(std::shared_ptr<IFileManager> fileManager,
                     const char *path = LOG_STORAGE_FILE_PATH,
                     size_t maxFiles = LOG_STORAGE_MAX_FILES,
                     size_t maxFileSize = LOG_STORAGE_MAX_FILE_SIZE,
                     bool rotateOnInit = LOG_STORAGE_NEW_FILE_ON_BOOT)
        : _fileManager(fileManager),
          _filePath(path),
          _maxFiles(maxFiles),
          _maxFileSize(maxFileSize),
          _currentSize(0),
          _rotateOnInit(rotateOnInit),
          _initialized(false)
    {
        parseFilePath();
    }

    ~RotatingFileSink() override
    {
        close();
    }

    void close() override
    {
        flush();
        _fileManager->close();
    }

    void flush() override
    {
        if (_buffer.size() == 0)
            return;

        writeToFile(_buffer.data(), _buffer.size());
        _buffer.clear();
    }

    bool write(const char *data, size_t size) override
    {
        if (!data || size == 0)
        {
            return false;
        }

        if (_buffer.size() + size > LOG_STORAGE_MAX_BUFFER_SIZE)
        {
            flush();
        }

        _buffer.append(data, data + size);
        return true;
    }

    bool writeToFile(const char *data, size_t size)
    {
        initFile();

        if (_currentSize > 0 && _currentSize + size > _maxFileSize)
        {
            rotate();
        }

        if (!ensureOpen())
        {
            return false;
        }

        size_t written = _fileManager->write(data, size);
        _fileManager->flush();
        _currentSize += written;
        return written == size;
    }

    void rotate()
    {
        _fileManager->close();

        // If max_files is 0, just restart the main file
        if (_maxFiles == 0)
        {
            _fileManager->remove(_filePath.c_str());
            return;
        }

        std::string oldestFile = createFilePath(_maxFiles);
        _fileManager->remove(oldestFile.c_str());

        for (size_t i = _maxFiles; i > 0; --i)
        {
            std::string src = createFilePath(i - 1);

            if (!_fileManager->exists(src.c_str()))
            {
                continue;
            }

            std::string target = createFilePath(i);

            _fileManager->remove(target.c_str());
            _fileManager->rename(src.c_str(), target.c_str());
        }

        _currentSize = 0;
    }

    void setFilePath(const char *path) override
    {
        close();
        _filePath = path;
        _initialized = false;
        _currentSize = 0;
        _buffer.clear();
        parseFilePath();
    }

    std::string getFilePath() const override
    {
        return _filePath;
    }
};

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