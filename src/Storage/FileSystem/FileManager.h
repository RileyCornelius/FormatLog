#pragma once

#include "IFileManager.h"
#include "FileManagerTraits.h"
#include <string>

template <typename TFileSystem>
class FileManager : public IFileManager
{
private:
    using TFile = typename FileTypeDeducer<TFileSystem>::type;
    using TAppendMode = typename AppendModeDeducer<TFileSystem>::type;

    TFileSystem &_fs;
    TFile file;
    std::string _filePath;

public:
    FileManager(TFileSystem &fs) : _fs(fs) {}

    bool open(const char *filePath) override
    {
        close();
        _filePath = filePath;
        file = _fs.open(filePath, AppendModeDeducer<TFileSystem>::defaultValue());

        return file ? true : false;
    }

    bool open(const std::string &filePath)
    {
        return open(filePath.c_str());
    }

    bool isOpen() const override
    {
        return file ? true : false;
    }

    size_t write(const char *data, size_t size) override
    {
        if (!file || size == 0)
        {
            return 0;
        }

        size_t written = file.write(reinterpret_cast<const uint8_t *>(data), size);

        return written;
    }

    void flush() override
    {
        if (file)
        {
            file.flush();
        }
    }

    void close() override
    {
        if (file)
        {
            file.close();
        }
    }

    size_t size() const override
    {
        return file.size();
    }

    const char *filePath() override
    {
        return _filePath.c_str();
    }

    bool exists(const char *filePath) override
    {
        return _fs.exists(filePath);
    }

    bool remove(const char *filePath) override
    {
        return _fs.remove(filePath);
    }

    bool rename(const char *oldPath, const char *newPath) override
    {
        return _fs.rename(oldPath, newPath);
    }
};
