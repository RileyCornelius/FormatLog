#pragma once

#include "IFileSystemUtils.h"
#include <string>

template <typename TFileSystem>
class Esp32FileSystemUtils : public IFileSystemUtils
{
private:
    TFileSystem &_fs;

public:
    Esp32FileSystemUtils(TFileSystem &fs) : _fs(fs) {}

    void deleteAllFiles(const char *directory = "/") override
    {
        auto root = _fs.open(directory);
        if (!root || !root.isDirectory())
        {
            return;
        }

        auto file = root.openNextFile();
        while (file)
        {
            if (!file.isDirectory())
            {
                // Build full path: directory + "/" + name
                // ESP32 file.name() may or may not include leading "/"
                std::string fullPath;
                const char *name = file.name();
                file.close();

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
                file.close();
            }
            file = root.openNextFile();
        }
        root.close();
    }

    size_t getFileSize(const char *path) override
    {
        if (!_fs.exists(path))
            return 0;

        auto file = _fs.open(path, "r");
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

        auto file = _fs.open(path, "r");
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

        auto root = _fs.open(directory);
        if (!root || !root.isDirectory())
        {
            return 0;
        }

        auto file = root.openNextFile();
        while (file)
        {
            if (!file.isDirectory())
            {
                if (ext.empty())
                {
                    count++;
                }
                else
                {
                    std::string name(file.name());
                    if (name.length() >= ext.length() &&
                        name.compare(name.length() - ext.length(), ext.length(), ext) == 0)
                    {
                        count++;
                    }
                }
            }
            file.close();
            file = root.openNextFile();
        }
        root.close();
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
