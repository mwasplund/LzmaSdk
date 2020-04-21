#pragma once

namespace LzmaSdk
{
    /// <summary>
    /// A wrapper around the public IStream interface
    /// </summary>
    class OutStreamWrapper :
        public ::IOutStream,
        public CMyUnknownImp
    {
    public:
        OutStreamWrapper(std::shared_ptr<::LzmaSdk::IOutStream> stream) :
            _stream(std::move(stream))
        {
        }

        STDMETHOD(Write)(const void *data, UInt32 size, UInt32 *processedSize) override final
        {
            try
            {
                auto sizeWritten = _stream->Write(data, size);

                if (processedSize != nullptr)
                    *processedSize = sizeWritten;

                return S_OK;
            }
            catch(const std::exception& e)
            {
                return E_FAIL;
            }
        }

        STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64 *newPosition)
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

        STDMETHOD(SetSize)(UInt64 newSize)
        {
            try
            {
                _stream->SetSize(newSize);
                return S_OK;
            }
            catch(const std::exception& e)
            {
                return E_FAIL;
            }
        }

        MY_QUERYINTERFACE_BEGIN2(IOutStream)
        MY_QUERYINTERFACE_END
        MY_ADDREF_RELEASE

    private:
        std::shared_ptr<::LzmaSdk::IOutStream> _stream;
    };
}
