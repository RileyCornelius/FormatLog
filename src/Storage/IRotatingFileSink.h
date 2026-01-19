#pragma once

#include <stddef.h>

class IRotatingFileSink
{
public:
    virtual ~IRotatingFileSink() = default;

    virtual bool write(const char *data, size_t size) = 0;
    virtual void flush() = 0;

    virtual void setMaxFiles(size_t maxFiles) = 0;
    virtual size_t getMaxFiles() const = 0;
    virtual void setMaxFileSize(size_t maxFileSize) = 0;
    virtual size_t getMaxFileSize() const = 0;
};