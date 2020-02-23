module;

#include <codecvt>
#include <locale>
#include <memory>
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

#include "ArchiveUpdateCallback.h"

module LzmaSdk;

namespace LzmaSdk
{
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

        auto outFileStream = std::make_shared<COutFileStream>();
        if (!outFileStream->Create(wideArchiveName.c_str(), false))
        {
            throw std::runtime_error("Can't create archive file");
        }

        auto outArchive = std::make_shared<NArchive::N7z::CHandler>();

        auto updateCallback = std::make_shared<ArchiveUpdateCallback>();
        updateCallback->Init(&dirItems);

        HRESULT result = outArchive->UpdateItems(
            outFileStream.get(),
            dirItems.Size(),
            updateCallback.get());

        updateCallback->Finilize();

        if (result != S_OK)
        {
            throw std::runtime_error("Update Error");
        }

        if (updateCallback->FailedFiles.Size() != 0)
        {
            throw std::runtime_error("Contains failed files");
        }
    }
}
