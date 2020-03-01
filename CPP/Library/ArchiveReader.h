#pragma once

namespace LzmaSdk
{
    /// <summary>
    /// The public entry point to read and extract the contents of an archive
    /// </summary>
    export class ArchiveReader
    {
    public:
        ArchiveReader(std::shared_ptr<IInStream> archive);

        void ExtractAll(
            const std::string& targetFolder,
            std::shared_ptr<IExtractCallback> callback);

    private:
        std::shared_ptr<IInStream> _archive;
    };
}
