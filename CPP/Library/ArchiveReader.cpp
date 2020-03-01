module;

#include <codecvt>
#include <locale>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "../Common/Common.h"
#include "../Common/IntToString.h"
#include "../Common/StringConvert.h"
#include "../Windows/FileDir.h"
#include "../Windows/FileFind.h"
#include "../Windows/FileName.h"
#include "../Windows/PropVariantConv.h"
#include "../7zip/Common/FileStreams.h"
#include "../7zip/Archive/7z/7zHandler.h"

module LzmaSdk;

#include "ArchiveExtractCallbackWrapper.h"
#include "ArchiveOpenCallback.h"

namespace LzmaSdk
{
    ArchiveReader::ArchiveReader(std::shared_ptr<IInStream> archive) :
        _archive(std::move(archive))
    {
        // Use the symbols to force resolve with linker
        g_ForceLZMA2Import = 0;
        g_ForceLZMAImport = 0;
    }

    void ArchiveReader::ExtractAll(
        const std::string& targetFolder,
        std::shared_ptr<IExtractCallback> callback)
    {
        // Ensure that the runtime tables are setup
        EnsureCrcInitialized();

        // Wrap the incoming stream so we can read it
        CMyComPtr<::IInStream> file = new InStreamWrapper(_archive);

        // Creat the archive handler
        auto archive = new NArchive::N7z::CHandler();
        CMyComPtr<NArchive::N7z::CHandler> outArchiveLoc(archive);

        // Open the archive
        auto openCallbackSpec = new ArchiveOpenCallback();
        CMyComPtr<::IArchiveOpenCallback> openCallback(openCallbackSpec);

        const UInt64 scanSize = 1 << 23;
        ThrowIfFailed(archive->Open(file, &scanSize, openCallback));

        // Extract into the target directory
        FString targetFolderInternal = us2fs(GetUnicodeString(targetFolder.c_str()));
        NWindows::NFile::NName::NormalizeDirPathPrefix(targetFolderInternal);
        auto extractCallbackSpec = new ArchiveExtractCallbackWrapper(
            callback,
            archive,
            targetFolderInternal);
        CMyComPtr<IArchiveExtractCallback> extractCallback(extractCallbackSpec);

        ThrowIfFailed(archive->Extract(
            nullptr,
            (UInt32)(Int32)(-1),
            false,
            extractCallback));

        if (extractCallbackSpec->HasErrors())
        {
            throw std::runtime_error("Extract callback has errors");
        }
    }
}
