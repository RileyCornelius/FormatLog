#pragma once

#include <memory>
#include <string>
#include "Config/Settings.h"
#include "IFileSink.h"
#include "IFileManager.h"

class DirectFileSink : public IFileSink
{
private:
    std::shared_ptr<IFileManager> _fileManager;
    std::string _filePath;

public:
    DirectFileSink(std::shared_ptr<IFileManager> fileManager,
                   const char *path = LOG_STORAGE_FILE_PATH)
        : _fileManager(fileManager), _filePath(path)
    {
    }

    ~DirectFileSink() override
    {
        close();
    }

    bool write(const char *data, size_t size) override
    {
        if (!_fileManager->isOpen())
        {
            if (!_fileManager->open(_filePath.c_str()))
            {
                return false;
            }
        }
        size_t written = _fileManager->write(data, size);
        _fileManager->flush();
        return written == size;
    }

    void flush() override
    {
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
