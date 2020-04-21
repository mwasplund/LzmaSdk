#pragma once

namespace LzmaSdk
{
    export class ArchiveWriter
    {
    public:
        ArchiveWriter(std::shared_ptr<IOutStream> archive);

        void AddFile(const std::string& file);
        void AddFiles(const std::vector<std::string>& files);
        void Save(std::shared_ptr<IArchiveUpdateCallback> callback);

    private:
        std::shared_ptr<IOutStream> _archive;
        std::vector<std::string> _files;
    };
}
