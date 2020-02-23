#pragma once

namespace LzmaSdk
{
    export class Archive
    {
    public:
        Archive(const std::string& archive);

        void AddFile(const std::string& file);
        void AddFiles(const std::vector<std::string>& files);
        void Save();

    private:
        std::string _name;
        std::vector<std::string> _files;
    };
}
