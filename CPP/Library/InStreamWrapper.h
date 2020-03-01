#pragma once

namespace LzmaSdk
{
    /// <summary>
    /// A wrapper around the public IStream interface
    /// </summary>
    class InStreamWrapper :
        public ::IInStream,
        public CMyUnknownImp
    {
    public:
        InStreamWrapper(std::shared_ptr<::LzmaSdk::IInStream> stream) :
            _stream(std::move(stream))
        {
        }

        STDMETHOD(Read)(void* data, UInt32 size, UInt32 *processedSize) override final
        {
            try
            {
                auto sizeRead = _stream->Read(data, size);

                if (processedSize != nullptr)
                    *processedSize = sizeRead;

                return S_OK;
            }
            catch(const std::exception& e)
            {
                return E_FAIL;
            }
        }

        STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition) override final
        {
            try
            {
                StreamSeekOrigin origin;
                switch (seekOrigin)
                {
                    case STREAM_SEEK_SET:
                        origin = StreamSeekOrigin::Begin;
                        break;
                    case STREAM_SEEK_CUR:
                        origin = StreamSeekOrigin::Current;
                        break;
                    case STREAM_SEEK_END:
                        origin = StreamSeekOrigin::End;
                        break;
                    default:
                        // Unknown origin
                        return E_FAIL;
                }

                auto position = _stream->Seek(offset, origin);

                if (newPosition != nullptr)
                    *newPosition = position;

                return S_OK;
            }
            catch(const std::exception& e)
            {
                return E_FAIL;
            }
        }

        MY_QUERYINTERFACE_BEGIN2(IInStream)
        MY_QUERYINTERFACE_END
        MY_ADDREF_RELEASE

    private:
        std::shared_ptr<::LzmaSdk::IInStream> _stream;
    };
}
