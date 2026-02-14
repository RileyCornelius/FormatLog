#pragma once

#include "IFileManager.h"
#include <string>

namespace fmtlog
{

template <typename TFileSystem>
class SdFatFileManager : public IFileManager
{
private:
    using TFile = decltype(std::declval<TFileSystem>().open(""));

    TFileSystem &_fs;
    TFile _file;
    std::string _filePath;

    // O_WRONLY | O_CREAT | O_APPEND (SdFat flags)
    static const int APPEND_FLAGS = 1 | 0x0200 | 0x0008;

public:
    SdFatFileManager(TFileSystem &fs) : _fs(fs) {}

    bool open(const char *filePath) override
    {
        close();
        _filePath = filePath;
        _file = _fs.open(filePath, APPEND_FLAGS);

        return _file ? true : false;
    }

    bool isOpen() const override
    {
        return _file ? true : false;
    }

    size_t write(const char *data, size_t size) override
    {
        if (!_file || size == 0)
        {
            return 0;
        }

        return _file.write(reinterpret_cast<const uint8_t *>(data), size);
    }

    void flush() override
    {
        if (_file)
        {
            _file.flush();
        }
    }

    void close() override
    {
        if (_file)
        {
            _file.close();
        }
    }

    size_t size() override
    {
        if (!_file)
        {
            return 0;
        }
        return _file.size();
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

} // namespace fmtlog
