#pragma once

#include <string>
#include <type_traits>

/**--------------------------------------------------------------------------------------
 * SFINAE Helpers for FileSystem detection
 *
 * Known library signatures for reference:
 * SdFat:      FsFile open(const String& path, int oflag = O_RDONLY);
 * ESP32:      File open(const char* filename, const char* mode = "r");
 *-------------------------------------------------------------------------------------*/

// Detect String mode (ESP32, LittleFS, SPIFFS, Pico)
template <typename TFS, typename = void>
struct HasStringMode : std::false_type
{
};
template <typename TFS>
struct HasStringMode<TFS, decltype(void(std::declval<TFS>().open("", "")))> : std::true_type
{
};

// Helper to deduce the File type returned by open()
template <typename TFS>
struct FileTypeDeducer
{
    using type = decltype(std::declval<TFS>().open(""));
};

// Helper to deduce the append mode type and default value
// Primary template declaration only - no implementation
template <typename TFS, typename = void>
struct AppendModeDeducer;

// Specialization for string mode filesystems (ESP32, LittleFS, SPIFFS, Pico)
template <typename TFS>
struct AppendModeDeducer<TFS, typename std::enable_if<HasStringMode<TFS>::value>::type>
{
    using type = const char *;
    static const char *defaultValue() { return "a"; } // FILE_APPEND
};

// Specialization for integer mode filesystems (SdFat, POSIX-style)
template <typename TFS>
struct AppendModeDeducer<TFS, typename std::enable_if<!HasStringMode<TFS>::value>::type>
{
    using type = int;
    static constexpr int defaultValue() { return 1 | 0x0200 | 0x0008; } // O_WRONLY | O_CREAT | O_APPEND
};
