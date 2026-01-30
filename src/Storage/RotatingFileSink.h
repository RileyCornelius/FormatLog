#pragma once

#include <string>
#include <memory>
#include <fmt.h>
#include "Config/Settings.h"
#include "IFileSink.h"
#include "IFileManager.h"

class RotatingFileSink : public IFileSink
{
private:
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
        _fileManager->close();
    }

    void flush() override
    {
        _fileManager->flush();
    }

    bool write(const char *data, size_t size) override
    {
        if (!data || size == 0)
            return false;

        initFile();

        if (_currentSize > 0 && _currentSize + size > _maxFileSize)
            rotate();

        if (!ensureOpen())
            return false;

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
            _currentSize = 0;
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
        parseFilePath();
    }

    std::string getFilePath() const override
    {
        return _filePath;
    }
};
