#pragma once

#include <string>
#include <type_traits>
#include "Config/Settings.h"
#include "IFileManager.h"

/**--------------------------------------------------------------------------------------
 * FileManager Class - File system storage for log messages
 *
 * Automatically detects file type and mode at compile time based on the
 * file system's open() method signature.
 *
 * Template parameters:
 *   TFileSystem - The file system type (e.g., fs::LittleFSFS, fs::SPIFFSFS, SDClass)
 *   TFile - Auto-deduced from open() return type (optional override)
 *
 * Usage:
 *   FileManager<fs::LittleFSFS> manager(LittleFS, "/log.txt");
 *   FileManager<SDClass> manager(SD, "/log.txt");
 *
 * Works with:
 *   - ESP32 LittleFS/SPIFFS (uses "a" string mode)
 *   - SdFat and POSIX-style file systems (uses O_WRONLY | O_APPEND | O_CREAT)
 *   - Raspberry Pi Pico FS (auto-detected)
 *-------------------------------------------------------------------------------------*/

// SFINAE helpers to detect open() parameter types
// Detects if file system uses POSIX-style integer modes (returns true for SdFat, etc.)
template <typename TFS, typename = void>
struct IsPosix : std::true_type
{
};

// Specialization for file systems with string modes (ESP32, Arduino)
template <typename TFS>
struct IsPosix<TFS, decltype(void(std::declval<TFS>().open("", "")))> : std::false_type
{
};

// Helper to deduce the File type returned by open()
template <typename TFS>
struct FileTypeDeducer
{
    // String mode first (Arduino, ESP32, Pico pattern)
    template <typename T = TFS>
    static decltype(std::declval<T>().open("", "")) deduceImpl(int);

    // Integer mode (SdFat, POSIX pattern)
    template <typename T = TFS>
    static decltype(std::declval<T>().open("", 0)) deduceImpl(...);

    using type = decltype(deduceImpl<TFS>(0));
};

/**--------------------------------------------------------------------------------------
 * FileManager Class - File system storage for log messages
 *-------------------------------------------------------------------------------------*/

template <typename TFileSystem, typename TFile = typename FileTypeDeducer<TFileSystem>::type>
class FileManager : public IFileManager
{
private:
    TFileSystem *fileSystem;
    std::string filePath;

    TFile file;
    size_t currentMessageCount = 0;
    size_t currentBufferSize = 0;
    size_t currentFileSize = 0;
    bool fileOpen = false;
    bool justRotated = false;

    template <typename TFS = TFileSystem>
    typename std::enable_if<!IsPosix<TFS>::value, TFile>::type
    openFileImpl(const char *path)
    {
        return fileSystem->open(path, "a");
    }

    template <typename TFS = TFileSystem>
    typename std::enable_if<IsPosix<TFS>::value, TFile>::type
    openFileImpl(const char *path)
    {
        return fileSystem->open(path, 1 | 0x0008 | 0x0200); // O_WRONLY | O_APPEND | O_CREAT
    }

    bool openFile()
    {
        if (fileOpen)
            return true;

        file = openFileImpl(filePath.c_str());
        if (!file)
            return false;

        // Read file size on first open, but skip after rotation (filesystem may return stale size)
        if (currentFileSize == 0 && !justRotated)
        {
            currentFileSize = file.size();
        }
        justRotated = false;
        fileOpen = true;

        return true;
    }

    void rotate()
    {
        // Rotation disabled - logging stops at MAX_FILE_SIZE
        if (LOG_STORAGE_MAX_FILES == 0)
            return;

        // Close current file if open
        if (fileOpen)
        {
            file.close();
            fileOpen = false;
        }

        // Reset counters
        currentFileSize = 0;
        currentMessageCount = 0;
        currentBufferSize = 0;
        justRotated = true;

        // Parse file path to get base name and extension
        size_t lastSlash = filePath.find_last_of('/');

        std::string dir = (lastSlash != std::string::npos) ? filePath.substr(0, lastSlash + 1) : "";
        std::string baseName = (lastSlash != std::string::npos) ? filePath.substr(lastSlash + 1) : filePath;
        std::string name, ext;

        size_t dotInBase = baseName.find_last_of('.');
        if (dotInBase != std::string::npos)
        {
            name = baseName.substr(0, dotInBase);
            ext = baseName.substr(dotInBase);
        }
        else
        {
            name = baseName;
            ext = "";
        }

        // Delete oldest file
        if (LOG_STORAGE_MAX_FILES > 0)
        {
            std::string oldestFile = dir + name + "." + std::to_string(LOG_STORAGE_MAX_FILES) + ext;
            if (fileSystem->exists(oldestFile.c_str()))
            {
                fileSystem->remove(oldestFile.c_str());
            }
        }

        // Rename files in reverse order
        for (int i = LOG_STORAGE_MAX_FILES - 1; i >= 1; i--)
        {
            std::string oldName = dir + name + "." + std::to_string(i) + ext;
            std::string newName = dir + name + "." + std::to_string(i + 1) + ext;

            if (fileSystem->exists(oldName.c_str()))
            {
                fileSystem->rename(oldName.c_str(), newName.c_str());
            }
        }

        // Rename current log file to .1
        std::string rotatedName = dir + name + ".1" + ext;
        if (fileSystem->exists(filePath.c_str()))
        {
            fileSystem->rename(filePath.c_str(), rotatedName.c_str());
        }
    }

public:
    FileManager(TFileSystem &fs, const char *path)
        : fileSystem(&fs), filePath(path) {}

    ~FileManager() override
    {
        close();
    }

    bool isFileOpen() const override
    {
        return fileOpen;
    }

    const char *getFilePath() const override
    {
        return filePath.c_str();
    }

    bool write(const uint8_t *data, size_t size) override
    {
        // Stop writing if rotation disabled and file size limit reached
        if (LOG_STORAGE_MAX_FILES == 0 && currentFileSize >= LOG_STORAGE_MAX_FILE_SIZE)
            return false;

        // Lazy initialization: open file on first write
        if (!openFile())
            return false;

        size_t written = file.write(data, size);
        if (written != size)
            return false;

        currentBufferSize += written;
        currentFileSize += written;
        currentMessageCount++;

        // Check if auto flush needed before writing next message
        if (currentMessageCount >= LOG_STORAGE_MAX_BUFFER_MESSAGES ||
            currentBufferSize >= LOG_STORAGE_MAX_BUFFER_SIZE)
        {
            flush();
        }

        // Check if rotation needed after writing
        if (currentFileSize >= LOG_STORAGE_MAX_FILE_SIZE)
        {
            rotate();
        }

        return true;
    }

    void flush() override
    {
        if (!fileOpen || !file)
            return;

        file.flush();
        currentMessageCount = 0;
        currentBufferSize = 0;
    }

    void close() override
    {
        if (!fileOpen || !file)
            return;

        file.close();
        fileOpen = false;
        currentMessageCount = 0;
        currentBufferSize = 0;
    }
};
