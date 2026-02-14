#pragma once

#include <type_traits>

namespace fmtlog
{

/**--------------------------------------------------------------------------------------
 * SFINAE building blocks for filesystem detection.
 *
 * Each trait tests for a single API feature. Composite traits combine them to
 * positively identify a filesystem family. To support a new filesystem:
 *   1. Add individual Has* traits for any unique API methods
 *   2. Create a composite Is*FileSystem trait that combines the checks
 *   3. Add a FileManager implementation
 *   4. Add a createFileManager overload in FileManagerFactory.h
 *
 * Known filesystem families:
 *   ESP32  (SD, SPIFFS, LittleFS)  –  string open modes, File::size()
 *   SdFat  (SdFat, SdFat32, SdExFat)  –  integer open flags, FsFile::fileSize()
 *-------------------------------------------------------------------------------------*/

// ---------------------------------------------------------------------------
// Individual API detectors
// ---------------------------------------------------------------------------

// FS::open(const char*, const char*) — ESP32 string mode
template <typename TFS, typename = void>
struct HasStringOpenMode : std::false_type
{
};
template <typename TFS>
struct HasStringOpenMode<TFS, decltype(void(std::declval<TFS>().open("", "")))> : std::true_type
{
};

// FS::open(const char*, int) — SdFat integer flags
template <typename TFS, typename = void>
struct HasIntOpenMode : std::false_type
{
};
template <typename TFS>
struct HasIntOpenMode<TFS, decltype(void(std::declval<TFS>().open("", int())))> : std::true_type
{
};

// FS::exists(const char*)
template <typename TFS, typename = void>
struct HasExists : std::false_type
{
};
template <typename TFS>
struct HasExists<TFS, decltype(void(std::declval<TFS>().exists("")))> : std::true_type
{
};

// FS::remove(const char*)
template <typename TFS, typename = void>
struct HasRemove : std::false_type
{
};
template <typename TFS>
struct HasRemove<TFS, decltype(void(std::declval<TFS>().remove("")))> : std::true_type
{
};

// FS::rename(const char*, const char*)
template <typename TFS, typename = void>
struct HasRename : std::false_type
{
};
template <typename TFS>
struct HasRename<TFS, decltype(void(std::declval<TFS>().rename("", "")))> : std::true_type
{
};

// ---------------------------------------------------------------------------
// Composite filesystem traits
// ---------------------------------------------------------------------------

// ESP32 family: string open mode + exists + remove + rename
template <typename TFS>
struct IsEsp32FileSystem : std::integral_constant<bool,
                                                  HasStringOpenMode<TFS>::value &&
                                                      HasExists<TFS>::value &&
                                                      HasRemove<TFS>::value &&
                                                      HasRename<TFS>::value>
{
};

// SdFat family: integer open mode, NO string open mode + exists + remove + rename
template <typename TFS>
struct IsSdFatFileSystem : std::integral_constant<bool,
                                                  HasIntOpenMode<TFS>::value &&
                                                      !HasStringOpenMode<TFS>::value &&
                                                      HasExists<TFS>::value &&
                                                      HasRemove<TFS>::value &&
                                                      HasRename<TFS>::value>
{
};

// True when at least one filesystem family matches
template <typename TFS>
struct IsSupportedFileSystem : std::integral_constant<bool,
                                                      IsEsp32FileSystem<TFS>::value ||
                                                          IsSdFatFileSystem<TFS>::value>
{
};

} // namespace fmtlog
