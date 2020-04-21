#pragma once

namespace LzmaSdk
{
    /// <summary>
    /// A wrapper around the public IStream interface
    /// </summary>
    class SequentialInStreamWrapper :
        public ::ISequentialInStream,
        public CMyUnknownImp
    {
    public:
        SequentialInStreamWrapper(std::shared_ptr<::LzmaSdk::ISequentialInStream> stream) :
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

        MY_QUERYINTERFACE_BEGIN2(ISequentialInStream)
        MY_QUERYINTERFACE_END
        MY_ADDREF_RELEASE

    private:
        std::shared_ptr<::LzmaSdk::ISequentialInStream> _stream;
    };
}
