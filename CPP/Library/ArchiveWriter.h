#pragma once

namespace LzmaSdk
{
    export struct FileProperties
    {
        std::string Name;
        uint64_t Size;
        std::time_t CreateTime;
        std::time_t AccessTime;
        std::time_t ModifiedTime;
        DWORD Attributes;
    };

    export class ArchiveWriter
    {
    public:
        ArchiveWriter(std::shared_ptr<IOutStream> archive);

        void AddFile(FileProperties file);
        void AddFiles(std::vector<FileProperties> files);
        void Save(std::shared_ptr<IArchiveUpdateCallback> callback);

    private:
        std::shared_ptr<IOutStream> _archive;
        std::vector<FileProperties> _files;
    };
}
