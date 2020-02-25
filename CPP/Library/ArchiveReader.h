#pragma once

namespace LzmaSdk
{
    export class ArchiveReader
    {
    public:
        ArchiveReader(const std::string& archive);

        void ExtractAll(const std::string& targetFolder);

    private:
        std::string _name;
    };
}
