#pragma once

namespace LzmaSdk
{
    class ArchiveExtractCallbackWrapper :
        public ::IArchiveExtractCallback,
        public ::ICryptoGetTextPassword,
        public CMyUnknownImp
    {
    public:
        ArchiveExtractCallbackWrapper(
            std::shared_ptr<LzmaSdk::IExtractCallback> callback,
            IInArchive *archiveHandler,
            const FString &directoryPath) :
            _callback(std::move(callback)),
            _errorCount(0),
            _archiveHandler(archiveHandler),
            _directoryPath(directoryPath)
        {
            if (_callback == nullptr)
                throw std::runtime_error("Null callback");
            if (_archiveHandler == nullptr)
                throw std::runtime_error("Null archiveHandler");
        }

        bool HasErrors() const
        {
            return _errorCount > 0;
        }

        MY_UNKNOWN_IMP1(ICryptoGetTextPassword)

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

        // IArchiveExtractCallback
        STDMETHOD(GetStream)(
            UInt32 index,
            ::ISequentialOutStream** outStream,
            Int32 askExtractMode)
        {
            try
            {
                *outStream = nullptr;

                // Get Name
                PROPVARIANT pathProp;
                RINOK(_archiveHandler->GetProperty(index, kpidPath, &pathProp));
                if (pathProp.vt != VT_BSTR)
                    return E_FAIL;
                UString filePath = pathProp.bstrVal;

                if (askExtractMode != NArchive::NExtract::NAskMode::kExtract)
                    return S_OK;

                // Get Attrib
                std::optional<uint32_t> permissions;
                PROPVARIANT attribProp;
                RINOK(_archiveHandler->GetProperty(index, kpidAttrib, &attribProp));
                if (attribProp.vt != VT_EMPTY)
                {
                    if (attribProp.vt != VT_UI4)
                        return E_FAIL;
                    permissions = attribProp.ulVal;
                }

                // Get is directory
                bool isDirectory = false;
                RINOK(IsArchiveItemProp(_archiveHandler, index, kpidIsDir, isDirectory));

                if (!isDirectory)
                {
                    // Get Modified Time
                    std::optional<std::time_t> modifiedTime;
                    PROPVARIANT modifiedTimeProp;
                    RINOK(_archiveHandler->GetProperty(index, kpidMTime, &modifiedTimeProp));
                    switch (modifiedTimeProp.vt)
                    {
                        case VT_EMPTY:
                            break;
                        case VT_FILETIME:
                            modifiedTime = ToTimeT(modifiedTimeProp.filetime);
                            break;
                        default:
                            return E_FAIL;
                    }

                    // Get Size
                    PROPVARIANT sizeProp;
                    RINOK(_archiveHandler->GetProperty(index, kpidSize, &sizeProp));
                    UInt64 newFileSize;
                    bool newFileSizeDefined = ConvertPropVariantToUInt64(sizeProp, newFileSize);

                    FString fullProcessedPath = _directoryPath + us2fs(filePath);
                    _diskFilePath = fullProcessedPath;

                    // Convert the incoming file name to Utf8
                    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
                    std::string path = converter.to_bytes(fullProcessedPath.Ptr());

                    // Create the stream
                    auto stream = _callback->GetStream(
                        path,
                        modifiedTime,
                        permissions);
                    CMyComPtr<::ISequentialOutStream> outFileStream = new SequentialOutStreamWrapper(stream);

                    *outStream = outFileStream.Detach();
                }

                return S_OK;
            }
            catch (...)
            {
                return E_FAIL;
            }
        }

        STDMETHOD(PrepareOperation)(Int32 askExtractMode)
        {
            try
            {
                _extractMode = false;
                OperationType type;
                switch (askExtractMode)
                {
                    case NArchive::NExtract::NAskMode::kExtract:
                        _extractMode = true;
                        type = OperationType::Extract;
                        break;
                    case NArchive::NExtract::NAskMode::kTest:
                        type = OperationType::Test;
                        break;
                    case NArchive::NExtract::NAskMode::kSkip:
                        type = OperationType::Skip;
                        break;
                };

                _callback->OnOperationStart(type);

                return S_OK;
            }
            catch (...)
            {
                return E_FAIL;
            }
        }

        STDMETHOD(SetOperationResult)(Int32 operationResult)
        {
            try
            {
                OperationResult result;
                switch (operationResult)
                {
                    case NArchive::NExtract::NOperationResult::kOK:
                        result = OperationResult::Success;
                        break;
                    case NArchive::NExtract::NOperationResult::kUnsupportedMethod:
                        result = OperationResult::UnsupportedMethod;
                        break;
                    case NArchive::NExtract::NOperationResult::kDataError:
                        result = OperationResult::DataError;
                        break;
                    case NArchive::NExtract::NOperationResult::kCRCError:
                        result = OperationResult::CRCError;
                        break;
                    case NArchive::NExtract::NOperationResult::kUnavailable:
                        result = OperationResult::Unavailable;
                        break;
                    case NArchive::NExtract::NOperationResult::kUnexpectedEnd:
                        result = OperationResult::UnexpectedEnd;
                        break;
                    case NArchive::NExtract::NOperationResult::kDataAfterEnd:
                        result = OperationResult::DataAfterEnd;
                        break;
                    case NArchive::NExtract::NOperationResult::kIsNotArc:
                        result = OperationResult::IsNotArc;
                        break;
                    case NArchive::NExtract::NOperationResult::kHeadersError:
                        result = OperationResult::HeadersError;
                        break;
                    case NArchive::NExtract::NOperationResult::kWrongPassword:
                        result = OperationResult::WrongPassword;
                        break;
                    default:
                        throw std::runtime_error("Unknown NOperationResult.");
                }

                if (result != OperationResult::Success)
                    _errorCount++;

                _callback->OnOperationCompleted(result);

                return S_OK;
            }
            catch (...)
            {
                return E_FAIL;
            }
        }

        // ICryptoGetTextPassword
        STDMETHOD(CryptoGetTextPassword)(BSTR* password)
        {
            return StringToBstr(UString(), password);
        }

    private:
        static HRESULT IsArchiveItemProp(IInArchive *archive, UInt32 index, PROPID propID, bool &result)
        {
            NWindows::NCOM::CPropVariant prop;
            RINOK(archive->GetProperty(index, propID, &prop));
            if (prop.vt == VT_BOOL)
                result = VARIANT_BOOLToBool(prop.boolVal);
            else if (prop.vt == VT_EMPTY)
                result = false;
            else
                return E_FAIL;
            return S_OK;
        }

        static std::time_t ToTimeT(FILETIME fileTime)
        {
            ULARGE_INTEGER value;
            value.LowPart = fileTime.dwLowDateTime;
            value.HighPart = fileTime.dwHighDateTime;
            return value.QuadPart / 10000000ULL - 11644473600ULL;
        }

        CMyComPtr<IInArchive> _archiveHandler;
        uint64_t _errorCount;
        std::shared_ptr<LzmaSdk::IExtractCallback> _callback;

        FString _directoryPath;  // Output directory
        FString _diskFilePath;   // full path to file on disk
        bool _extractMode;
    };
}
