module;

#include <codecvt>
#include <locale>
#include <memory>
#include <string>
#include <vector>

#include "../Common/Common.h"
#include "../Common/IntToString.h"
#include "../Common/StringConvert.h"
#include "../Windows/FileFind.h"
#include "../7zip/Common/FileStreams.h"
#include "../7zip/Archive/7z/7zHandler.h"

module LzmaSdk;

#include "ArchiveUpdateCallbackWrapper.h"

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

    ArchiveWriter::ArchiveWriter(std::shared_ptr<IOutStream> archive) :
        _archive(std::move(archive)),
        _files()
    {
        // Use the symbols to force resolve with linker
        g_ForceLZMA2Import = 0;
        g_ForceLZMAImport = 0;
    }

    void ArchiveWriter::AddFile(const std::string& file)
    {
        _files.push_back(file);
    }

    void ArchiveWriter::AddFiles(const std::vector<std::string>& files)
    {
        _files.insert(
            _files.end(),
            files.begin(),
            files.end());
    }

    void ArchiveWriter::Save(std::shared_ptr<IArchiveUpdateCallback> callback)
    {
        // Wrap the incoming stream so we can read it
        CMyComPtr<::IOutStream> outFileStream =
            new OutStreamWrapper(_archive);

        auto outArchive = new NArchive::N7z::CHandler();
        CMyComPtr<NArchive::N7z::CHandler> outArchiveLoc(outArchive);
        SetCompressionProperties(*outArchive);

        CMyComPtr<ArchiveUpdateCallbackWrapper> updateCallback =
            new ArchiveUpdateCallbackWrapper(callback);
        updateCallback->Init(_files);

        // Ensure that the runtime tables are setup
        EnsureCrcInitialized();

        ThrowIfFailed(outArchive->UpdateItems(
            outFileStream,
            _files.size(),
            updateCallback));

        if (updateCallback->HasErrors())
        {
            throw std::runtime_error("Contains failed files");
        }

        callback->OnCompleted();
    }
}
