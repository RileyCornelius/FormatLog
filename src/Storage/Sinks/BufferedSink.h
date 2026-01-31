#pragma once

#include <memory>
#include <string>
#include <fmt.h>
#include "Config/Settings.h"
#include "IFileSink.h"

class BufferedSink : public IFileSink
{
private:
    std::shared_ptr<IFileSink> _inner;
    fmt::basic_memory_buffer<char, LOG_STORAGE_MAX_BUFFER_SIZE> _buffer;

public:
    explicit BufferedSink(std::shared_ptr<IFileSink> inner)
        : _inner(inner) {}

    ~BufferedSink() override
    {
        close();
    }

    bool write(const char *data, size_t size) override
    {
        if (!data || size == 0)
            return false;

        if (_buffer.size() + size > LOG_STORAGE_MAX_BUFFER_SIZE)
            flush();

        _buffer.append(data, data + size);
        return true;
    }

    void flush() override
    {
        if (_buffer.size() == 0)
            return;

        _inner->write(_buffer.data(), _buffer.size());
        _buffer.clear();
        _inner->flush();
    }

    void close() override
    {
        flush();
        _inner->close();
    }

    void setFilePath(const char *path) override
    {
        flush();
        _inner->setFilePath(path);
    }

    std::string getFilePath() const override
    {
        return _inner->getFilePath();
    }
};
