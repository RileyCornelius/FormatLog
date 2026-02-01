#pragma once

#include <string>
#include <memory>
#include "IFileSink.h"
#include "Storage/FileSystem/IFileManager.h"

class SimpleFileSink : public IFileSink
{
private:
    std::shared_ptr<IFileManager> _fileManager;
    std::string _filePath;

    bool ensureOpen()
    {
        if (_fileManager->isOpen())
            return true;
        return _fileManager->open(_filePath.c_str());
    }

public:
    SimpleFileSink(std::shared_ptr<IFileManager> fileManager,
                   const char *path = LOG_STORAGE_FILE_PATH)
        : _fileManager(fileManager),
          _filePath(path)
    {
    }

    ~SimpleFileSink() override
    {
        close();
    }

    bool write(const char *data, size_t size) override
    {
        if (!data || size == 0)
            return false;

        if (!ensureOpen())
            return false;

        size_t written = _fileManager->write(data, size);
        _fileManager->flush();
        return written == size;
    }

    void flush() override
    {
        if (_fileManager->isOpen())
            _fileManager->flush();
    }

    void close() override
    {
        _fileManager->close();
    }

    void setFilePath(const char *path) override
    {
        close();
        _filePath = path;
    }

    std::string getFilePath() const override
    {
        return _filePath;
    }
};
