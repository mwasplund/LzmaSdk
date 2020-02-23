module;

#include <codecvt>
#include <locale>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "../Common/Common.h"
#include "../Common/MyInitGuid.h"
#include "../Common/IntToString.h"
#include "../Common/StringConvert.h"
#include "../Windows/FileFind.h"
#include "../Windows/NtCheck.h"
#include "../7zip/Common/FileStreams.h"
#include "../7zip/Archive/7z/7zHandler.h"
#include "../../C/7zCrc.h"

#include "ArchiveUpdateCallback.h"
#include "Helpers.h"

module LzmaSdk;

namespace LzmaSdk
{
    void SetCompressionProperties(ISetProperties& properties)
    {
        const size_t numProps = 1;
        const wchar_t* names[numProps] = { L"x" };
        NWindows::NCOM::CPropVariant values[numProps] =
        {
            static_cast<UInt32>(2),
        };

        ThrowIfFailed(
            properties.SetProperties(names, values, numProps));
    }

    Archive::Archive(const std::string& name) :
        _name(name),
        _files()
    {
        // Use the symbols to force resolve with linker
        g_ForceLZMA2Import = 0;
        g_ForceLZMAImport = 0;
    }

    void Archive::AddFile(const std::string& file)
    {
        _files.push_back(file);
    }

    void Archive::AddFiles(const std::vector<std::string>& files)
    {
        _files.insert(
            _files.end(),
            files.begin(),
            files.end());
    }

    void Archive::Save()
    {
        CObjectVector<DirectoryItem> dirItems;
        for (auto& file : _files)
        {
            DirectoryItem di;
            FString name = us2fs(GetUnicodeString(file.c_str()));

            NWindows::NFile::NFind::CFileInfo fi;
            if (!fi.Find(name))
            {
                throw std::runtime_error("Can't find file" + file);
            }

            di.Attrib = fi.Attrib;
            di.Size = fi.Size;
            di.CTime = fi.CTime;
            di.ATime = fi.ATime;
            di.MTime = fi.MTime;
            di.Name = fs2us(name);
            di.FullPath = name;
            dirItems.Add(di);
        }

        // Convert the incoming file name to Utf16 since they like it that way
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
        std::wstring wideArchiveName = converter.from_bytes(_name);

        auto outFileStream = new COutFileStream();
        CMyComPtr<COutFileStream> outFileStreamLoc(outFileStream);
        if (!outFileStream->Create(wideArchiveName.c_str(), false))
        {
            throw std::runtime_error("Can't create archive file");
        }

        auto outArchive = new NArchive::N7z::CHandler();
        CMyComPtr<NArchive::N7z::CHandler> outArchiveLoc(outArchive);
        SetCompressionProperties(*outArchive);

        auto updateCallback = new ArchiveUpdateCallback();
        CMyComPtr<ArchiveUpdateCallback> updateCallbackLoc(updateCallback);
        updateCallback->Init(&dirItems);

        // Ensure that the runtime tables are setup
        EnsureCrcInitialized();

        ThrowIfFailed(outArchive->UpdateItems(
            outFileStream,
            dirItems.Size(),
            updateCallback));

        updateCallback->Finilize();
        if (updateCallback->FailedFiles.Size() != 0)
        {
            throw std::runtime_error("Contains failed files");
        }
    }
}
