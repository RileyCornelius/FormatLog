#pragma once

#include "IFileSystemUtils.h"
#include <string>

template <typename TFileSystem>
class SdFatFileSystemUtils : public IFileSystemUtils
{
private:
    TFileSystem &_fs;

    static const int READ_FLAGS = 0; // O_RDONLY (SdFat flag)

public:
    SdFatFileSystemUtils(TFileSystem &fs) : _fs(fs) {}

    void deleteAllFiles(const char *directory = "/") override
    {
        auto dir = _fs.open(directory);
        if (!dir || !dir.isDir())
        {
            return;
        }

        // Open a dummy file to get the correct type
        auto entry = _fs.open("/");
        entry.close();

        char name[64];
        while (entry.openNext(&dir, READ_FLAGS))
        {
            if (!entry.isDir())
            {
                entry.getName(name, sizeof(name));
                entry.close();
                std::string fullPath;
                if (name[0] == '/')
                {
                    fullPath = name;
                }
                else
                {
                    fullPath = std::string(directory);
                    if (fullPath.back() != '/')
                        fullPath += '/';
                    fullPath += name;
                }
                _fs.remove(fullPath.c_str());
            }
            else
            {
                entry.close();
            }
        }
        dir.close();
    }

    size_t getFileSize(const char *path) override
    {
        if (!_fs.exists(path))
            return 0;

        auto file = _fs.open(path, READ_FLAGS);
        if (!file)
            return 0;

        size_t size = file.size();
        file.close();
        return size;
    }

    std::string readFile(const char *path) override
    {
        if (!_fs.exists(path))
            return "";

        auto file = _fs.open(path, READ_FLAGS);
        if (!file)
            return "";

        std::string content;
        while (file.available())
        {
            content.push_back(file.read());
        }
        file.close();
        return content;
    }

    int countFiles(const char *directory, const char *extension = "") override
    {
        int count = 0;
        std::string ext(extension);

        auto dir = _fs.open(directory);
        if (!dir || !dir.isDir())
        {
            return 0;
        }

        auto entry = _fs.open("/");
        entry.close();

        char name[64];
        while (entry.openNext(&dir, READ_FLAGS))
        {
            if (!entry.isDir())
            {
                if (ext.empty())
                {
                    count++;
                }
                else
                {
                    entry.getName(name, sizeof(name));
                    std::string nameStr(name);
                    if (nameStr.length() >= ext.length() &&
                        nameStr.compare(nameStr.length() - ext.length(), ext.length(), ext) == 0)
                    {
                        count++;
                    }
                }
            }
            entry.close();
        }
        dir.close();
        return count;
    }

    bool exists(const char *path) override
    {
        return _fs.exists(path);
    }

    bool remove(const char *path) override
    {
        return _fs.remove(path);
    }

    bool mkdir(const char *path) override
    {
        return _fs.mkdir(path);
    }

    bool rmdir(const char *path) override
    {
        return _fs.rmdir(path);
    }
};
