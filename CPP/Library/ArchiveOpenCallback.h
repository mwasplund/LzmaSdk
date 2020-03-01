#pragma once

namespace LzmaSdk
{
    class ArchiveOpenCallback :
        public IArchiveOpenCallback,
        public ICryptoGetTextPassword,
        public CMyUnknownImp
    {
    public:
        ArchiveOpenCallback()
        {
        }

        MY_UNKNOWN_IMP1(ICryptoGetTextPassword)

        STDMETHOD(SetTotal)(const UInt64 *files, const UInt64 *bytes)
        {
            return S_OK;
        }

        STDMETHOD(SetCompleted)(const UInt64 *files, const UInt64 *bytes)
        {
            return S_OK;
        }

        STDMETHOD(CryptoGetTextPassword)(BSTR *password)
        {
            return StringToBstr(UString(), password);
        }
    };
}
