#pragma once

#include <memory>
#include "Config/Settings.h"
#include "IFileSink.h"
#include "FileManager.h"

class DirectFileSink : public IFileSink
{
private:
    std::shared_ptr<IFileManager> _fileManager;
    std::string _filePath;
    bool _alwaysFlush;

public:
    DirectFileSink(std::shared_ptr<IFileManager> fileManager,
                   const char *path = LOG_STORAGE_FILE_PATH,
                   bool alwaysFlush = false)
        : _fileManager(fileManager), _filePath(path), _alwaysFlush(alwaysFlush)
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
        if (_alwaysFlush)
        {
            _fileManager->flush();
        }
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

/**
 * Factory function to create a DirectFileSink with FileManager
 * @param fs Reference to the file system (SPIFFS, LittleFS, SD, SdFat)
 * @param filePath Path to the log file
 * @param alwaysFlush Whether to flush to disk after every write
 * @return Shared pointer to DirectFileSink
 */
template <typename TFileSystem>
std::shared_ptr<IFileSink> createDirectFileSink(TFileSystem &fs,
                                                const char *filePath = LOG_STORAGE_FILE_PATH,
                                                bool alwaysFlush = false)
{
    auto fileManager = std::make_shared<FileManager<TFileSystem>>(fs);
    auto fileSink = std::make_shared<DirectFileSink>(fileManager, filePath, alwaysFlush);
    return fileSink;
}
