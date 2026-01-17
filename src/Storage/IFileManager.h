#pragma once

#include <stddef.h>
#include <stdint.h>

/**--------------------------------------------------------------------------------------
 * IFileManager Interface - Abstract base class for file system storage
 *
 * Provides a common interface for different FileManager template instantiations,
 * allowing FormatLog to hold a single storage pointer regardless of file system type.
 *-------------------------------------------------------------------------------------*/

class IFileManager
{
public:
    virtual ~IFileManager() = default;

    /**
     * Write data to the log file
     * @param data Pointer to data buffer
     * @param size Size of data in bytes
     * @return true if write succeeded, false otherwise
     */
    virtual bool write(const uint8_t *data, size_t size) = 0;

    /**
     * Flush buffered data to storage
     */
    virtual void flush() = 0;

    /**
     * Close the log file
     */
    virtual void close() = 0;

    /**
     * Check if file is currently open
     * @return true if file is open
     */
    virtual bool isFileOpen() const = 0;

    /**
     * Get the current log file path
     * @return C-string path to log file
     */
    virtual const char *getFilePath() const = 0;
};
