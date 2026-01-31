#pragma once

#include <string>
#include <stddef.h>

class IFileSink
{
public:
    virtual ~IFileSink() = default;

    virtual bool write(const char *data, size_t size) = 0;
    virtual void flush() = 0;
    virtual void close() = 0;
    virtual void setFilePath(const char *path) = 0;
    virtual std::string getFilePath() const = 0;
};
