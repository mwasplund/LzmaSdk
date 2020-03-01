#pragma once

namespace LzmaSdk
{
    /// <summary>
    /// A wrapper around the public IStream interface
    /// </summary>
    class SequentialOutStreamWrapper :
        public ::ISequentialOutStream,
        public CMyUnknownImp
    {
    public:
        SequentialOutStreamWrapper(std::shared_ptr<::LzmaSdk::ISequentialOutStream> stream) :
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

        MY_QUERYINTERFACE_BEGIN2(ISequentialOutStream)
        MY_QUERYINTERFACE_END
        MY_ADDREF_RELEASE

    private:
        std::shared_ptr<::LzmaSdk::ISequentialOutStream> _stream;
    };
}
