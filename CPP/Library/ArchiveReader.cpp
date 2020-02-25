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

#include "ArchiveExtractCallback.h"
#include "ArchiveOpenCallback.h"

module LzmaSdk;

namespace LzmaSdk
{
    ArchiveReader::ArchiveReader(const std::string& name) :
        _name(name)
    {
        // Use the symbols to force resolve with linker
        g_ForceLZMA2Import = 0;
        g_ForceLZMAImport = 0;
    }

    void ArchiveReader::ExtractAll(const std::string& targetFolder)
    {
        // Convert the incoming file name to Utf16 since they like it that way
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        std::wstring wideArchiveName = converter.from_bytes(_name);

        // Ensure that the runtime tables are setup
        EnsureCrcInitialized();

        auto fileSpec = new CInFileStream();
        CMyComPtr<IInStream> file = fileSpec;
        if (!fileSpec->Open(wideArchiveName.c_str()))
        {
            throw std::runtime_error("Can't open archive file");
        }

        // Creat the archive handler
        auto archive = new NArchive::N7z::CHandler();
        CMyComPtr<NArchive::N7z::CHandler> outArchiveLoc(archive);

        // Open the archive
        auto openCallbackSpec = new ArchiveOpenCallback();
        CMyComPtr<IArchiveOpenCallback> openCallback(openCallbackSpec);
        openCallbackSpec->PasswordIsDefined = false;

        const UInt64 scanSize = 1 << 23;
        ThrowIfFailed(archive->Open(file, &scanSize, openCallback));

        // Extract into the target directory
        auto extractCallbackSpec = new ArchiveExtractCallback();
        CMyComPtr<IArchiveExtractCallback> extractCallback(extractCallbackSpec);
        FString targetFolderInternal = us2fs(GetUnicodeString(targetFolder.c_str()));
        extractCallbackSpec->Init(archive, targetFolderInternal);
        extractCallbackSpec->PasswordIsDefined = false;

        ThrowIfFailed(archive->Extract(
            nullptr,
            (UInt32)(Int32)(-1),
            false,
            extractCallback));

        if (extractCallbackSpec->NumErrors > 0)
        {
            throw std::runtime_error("Extract callback has errors");
        }
    }
}
