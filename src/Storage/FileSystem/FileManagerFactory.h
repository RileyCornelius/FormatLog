#pragma once

#include <memory>
#include "FileSystemTraits.h"
#include "Esp32FileManager.h"
#include "SdFatFileManager.h"

namespace fmtlog
{

template <typename TFileSystem,
          typename std::enable_if<IsEsp32FileSystem<TFileSystem>::value, int>::type = 0>
std::shared_ptr<IFileManager> createFileManager(TFileSystem &fs)
{
    return std::make_shared<Esp32FileManager<TFileSystem>>(fs);
}

template <typename TFileSystem,
          typename std::enable_if<IsSdFatFileSystem<TFileSystem>::value, int>::type = 0>
std::shared_ptr<IFileManager> createFileManager(TFileSystem &fs)
{
    return std::make_shared<SdFatFileManager<TFileSystem>>(fs);
}

} // namespace fmtlog
