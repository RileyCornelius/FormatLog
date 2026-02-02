#pragma once

#include <stddef.h>

namespace fmtlog
{

class IFileManager
{
public:
    virtual ~IFileManager() = default;

    virtual bool open(const char *filePath) = 0;
    virtual bool isOpen() const = 0;
    virtual size_t write(const char *data, size_t size) = 0;
    virtual void flush() = 0;
    virtual void close() = 0;
    virtual size_t size() = 0;
    virtual const char *filePath() = 0;
    virtual bool exists(const char *filePath) = 0;
    virtual bool remove(const char *filePath) = 0;
    virtual bool rename(const char *oldPath, const char *newPath) = 0;
};

} // namespace fmtlog
