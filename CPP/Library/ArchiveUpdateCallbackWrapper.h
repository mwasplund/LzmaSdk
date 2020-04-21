#pragma once

namespace LzmaSdk
{
    struct DirectoryItem
    {
        UInt64 Size;
        FILETIME CTime;
        FILETIME ATime;
        FILETIME MTime;
        UString Name;
        FString FullPath;
        UInt32 Attrib;

        bool isDir() const
        {
            return (Attrib & FILE_ATTRIBUTE_DIRECTORY) != 0;
        }
    };

    class ArchiveUpdateCallbackWrapper :
        public IArchiveUpdateCallback2,
        public ICryptoGetTextPassword2,
        public CMyUnknownImp
    {
    public:
        ArchiveUpdateCallbackWrapper(
            std::shared_ptr<LzmaSdk::IArchiveUpdateCallback> callback) :
            _callback(std::move(callback)),
            DirItems()
        {
            if (_callback == nullptr)
                throw std::runtime_error("Null callback");
        }

        void Init(const std::vector<std::string>& files)
        {
            FailedFiles.Clear();
            FailedCodes.Clear();

            DirItems.Clear();
            for (auto& file : files)
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
                DirItems.Add(di);
            }
        }

        bool HasErrors() const
        {
            return !FailedFiles.IsEmpty();
        }

        MY_UNKNOWN_IMP2(IArchiveUpdateCallback2, ICryptoGetTextPassword2)

        // IProgress
        STDMETHOD(SetTotal)(UInt64 size)
        {
            try
            {
                _callback->OnStart(size);
                return S_OK;
            }
            catch (...)
            {
                return E_FAIL;
            }
        }

        STDMETHOD(SetCompleted)(const UInt64 *completeValue)
        {
            try
            {
                if (completeValue != nullptr)
                {
                    _callback->OnProgress(*completeValue);
                }

                return S_OK;
            }
            catch (...)
            {
                return E_FAIL;
            }
        }

        // IUpdateCallback2
        STDMETHOD(GetUpdateItemInfo)(
            UInt32 index,
            Int32* newData,
            Int32* newProperties,
            UInt32* indexInArchive)
        {
            if (newData)
                *newData = BoolToInt(true);
            if (newProperties)
                *newProperties = BoolToInt(true);
            if (indexInArchive)
                *indexInArchive = (UInt32)(Int32)-1;
            return S_OK;
        }

        STDMETHOD(GetProperty)(UInt32 index, PROPID propID, PROPVARIANT* value)
        {
            NWindows::NCOM::CPropVariant prop;

            if (propID == kpidIsAnti)
            {
                prop = false;
                prop.Detach(value);
                return S_OK;
            }

            {
                const DirectoryItem &dirItem = DirItems[index];
                switch (propID)
                {
                    case kpidPath:
                        prop = dirItem.Name;
                        break;
                    case kpidIsDir:
                        prop = dirItem.isDir();
                        break;
                    case kpidSize:
                        prop = dirItem.Size;
                        break;
                    case kpidAttrib:
                        prop = dirItem.Attrib;
                        break;
                    case kpidCTime:
                        prop = dirItem.CTime;
                        break;
                    case kpidATime:
                        prop = dirItem.ATime;
                        break;
                    case kpidMTime:
                        prop = dirItem.MTime;
                        break;
                }
            }

            prop.Detach(value);
            return S_OK;
        }

        STDMETHOD(GetStream)(UInt32 index, ::ISequentialInStream** inStream)
        {
            try
            {
                const DirectoryItem &dirItem = DirItems[index];

                if (dirItem.isDir())
                    return S_OK;

                // Convert the incoming file name to Utf8
                std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
                std::string path = converter.to_bytes(dirItem.FullPath.Ptr());

                // Create the stream
                auto stream = _callback->GetStream(path);
                CMyComPtr<::ISequentialInStream> inFileStream =
                    new SequentialInStreamWrapper(stream);

                *inStream = inFileStream.Detach();

                return S_OK;
            }
            catch (...)
            {
                return E_FAIL;
            }
        }

        STDMETHOD(SetOperationResult)(Int32 operationResult)
        {
            return S_OK;
        }

        STDMETHOD(GetVolumeSize)(UInt32 index, UInt64* size)
        {
            if (VolumesSizes.Size() == 0)
                return S_FALSE;
            if (index >= (UInt32)VolumesSizes.Size())
                index = VolumesSizes.Size() - 1;
            *size = VolumesSizes[index];
            return S_OK;
        }

        STDMETHOD(GetVolumeStream)(UInt32 index, ::ISequentialOutStream** volumeStream)
        {
            wchar_t temp[16];
            ConvertUInt32ToString(index + 1, temp);
            UString res = temp;
            while (res.Len() < 2)
                res.InsertAtFront(L'0');
            UString fileName = VolName;
            fileName += '.';
            fileName += res;
            fileName += VolExt;
            COutFileStream *streamSpec = new COutFileStream;
            CMyComPtr<::ISequentialOutStream> streamLoc(streamSpec);
            if (!streamSpec->Create(us2fs(fileName), false))
                return ::GetLastError();
            *volumeStream = streamLoc.Detach();
            return S_OK;
        }

        STDMETHOD(CryptoGetTextPassword2)(Int32 *passwordIsDefined, BSTR* password)
        {
            if (passwordIsDefined != nullptr)
                *passwordIsDefined = BoolToInt(false);

            return StringToBstr(UString(), password);
        }

    private:
        std::shared_ptr<LzmaSdk::IArchiveUpdateCallback> _callback;
        CRecordVector<UInt64> VolumesSizes;
        UString VolName;
        UString VolExt;

        CObjectVector<DirectoryItem> DirItems;

        FStringVector FailedFiles;
        CRecordVector<HRESULT> FailedCodes;
    };
}
