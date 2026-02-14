#pragma once

#include <stddef.h>
#include <stdint.h>
#include <string>

class IFileSystemUtils
{
public:
    virtual ~IFileSystemUtils() = default;

    virtual void deleteAllFiles(const char *directory = "/") = 0;
    virtual size_t getFileSize(const char *path) = 0;
    virtual std::string readFile(const char *path) = 0;
    virtual int countFiles(const char *directory, const char *extension = "") = 0;
    virtual bool exists(const char *path) = 0;
    virtual bool remove(const char *path) = 0;
    virtual bool mkdir(const char *path) = 0;
    virtual bool rmdir(const char *path) = 0;

    /**
     * Count the main log file plus all rotated variants that exist.
     * Follows the RotatingFileSink naming convention:
     *   basePath = "/log.txt"  ->  /log.txt, /log.1.txt, /log.2.txt, ...
     *   basePath = "/log"      ->  /log, /log.1, /log.2, ...
     */
    int countLogFiles(const char *basePath)
    {
        std::string path(basePath);
        size_t dotPos = path.find_last_of('.');
        size_t slashPos = path.find_last_of("/\\");

        bool hasExt = dotPos != std::string::npos &&
                      (slashPos == std::string::npos || dotPos > slashPos);

        std::string stem = hasExt ? path.substr(0, dotPos) : path;
        std::string ext = hasExt ? path.substr(dotPos) : "";

        int count = 0;

        // Check main file
        if (exists(basePath))
            count++;

        // Check rotated files: <stem>.1<ext>, <stem>.2<ext>, ...
        for (int i = 1;; i++)
        {
            std::string rotated = stem + "." + std::to_string(i) + ext;
            if (!exists(rotated.c_str()))
                break;
            count++;
        }

        return count;
    }
};
