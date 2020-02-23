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

    class ArchiveUpdateCallback :
        public IArchiveUpdateCallback2,
        public ICryptoGetTextPassword2,
        public CMyUnknownImp
    {
    public:
        MY_UNKNOWN_IMP2(IArchiveUpdateCallback2, ICryptoGetTextPassword2)

        // IProgress
        STDMETHOD(SetTotal)(UInt64 size)
        {
            return S_OK;
        }

        STDMETHOD(SetCompleted)(const UInt64 *completeValue)
        {
            return S_OK;
        }

        // IUpdateCallback2
        STDMETHOD(GetUpdateItemInfo)(UInt32 index,
            Int32 *newData, Int32 *newProperties, UInt32 *indexInArchive)
        {
            if (newData)
                *newData = BoolToInt(true);
            if (newProperties)
                *newProperties = BoolToInt(true);
            if (indexInArchive)
                *indexInArchive = (UInt32)(Int32)-1;
            return S_OK;
        }

        STDMETHOD(GetProperty)(UInt32 index, PROPID propID, PROPVARIANT *value)
        {
            NWindows::NCOM::CPropVariant prop;

            if (propID == kpidIsAnti)
            {
                prop = false;
                prop.Detach(value);
                return S_OK;
            }

            {
                const DirectoryItem &dirItem = (*DirItems)[index];
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

        STDMETHOD(GetStream)(UInt32 index, ISequentialInStream **inStream)
        {
            RINOK(Finilize());

            const DirectoryItem &dirItem = (*DirItems)[index];

            if (dirItem.isDir())
                return S_OK;

            {
                CInFileStream *inStreamSpec = new CInFileStream;
                CMyComPtr<ISequentialInStream> inStreamLoc(inStreamSpec);
                FString path = DirPrefix + dirItem.FullPath;
                if (!inStreamSpec->Open(path))
                {
                    DWORD sysError = ::GetLastError();
                    FailedCodes.Add(sysError);
                    FailedFiles.Add(path);
                    return S_FALSE;
                }

                *inStream = inStreamLoc.Detach();
            }

            return S_OK;
        }

        STDMETHOD(SetOperationResult)(Int32 operationResult)
        {
            m_NeedBeClosed = true;
            return S_OK;
        }

        STDMETHOD(GetVolumeSize)(UInt32 index, UInt64 *size)
        {
            if (VolumesSizes.Size() == 0)
                return S_FALSE;
            if (index >= (UInt32)VolumesSizes.Size())
                index = VolumesSizes.Size() - 1;
            *size = VolumesSizes[index];
            return S_OK;
        }

        STDMETHOD(GetVolumeStream)(UInt32 index, ISequentialOutStream **volumeStream)
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
            CMyComPtr<ISequentialOutStream> streamLoc(streamSpec);
            if (!streamSpec->Create(us2fs(fileName), false))
                return ::GetLastError();
            *volumeStream = streamLoc.Detach();
            return S_OK;
        }

        STDMETHOD(CryptoGetTextPassword2)(Int32 *passwordIsDefined, BSTR *password)
        {
            if (!PasswordIsDefined)
            {
                if (AskPassword)
                {
                    // You can ask real password here from user
                    // Password = GetPassword(OutStream);
                    // PasswordIsDefined = true;
                    return E_ABORT;
                }
            }

            *passwordIsDefined = BoolToInt(PasswordIsDefined);
            return StringToBstr(Password, password);
        }

    public:
        CRecordVector<UInt64> VolumesSizes;
        UString VolName;
        UString VolExt;

        FString DirPrefix;
        const CObjectVector<DirectoryItem> *DirItems;

        bool PasswordIsDefined;
        UString Password;
        bool AskPassword;

        bool m_NeedBeClosed;

        FStringVector FailedFiles;
        CRecordVector<HRESULT> FailedCodes;

        ArchiveUpdateCallback() :
            PasswordIsDefined(false),
            AskPassword(false),
            DirItems(0)
        {
        };

        ~ArchiveUpdateCallback()
        {
             Finilize();
        }

        HRESULT Finilize()
        {
            if (m_NeedBeClosed)
            {
                m_NeedBeClosed = false;
            }

            return S_OK;
        }

        void Init(const CObjectVector<DirectoryItem>* dirItems)
        {
            DirItems = dirItems;
            m_NeedBeClosed = false;
            FailedFiles.Clear();
            FailedCodes.Clear();
        }
    };
}
