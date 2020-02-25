#pragma once

namespace LzmaSdk
{
    class ArchiveOpenCallback :
        public IArchiveOpenCallback,
        public ICryptoGetTextPassword,
        public CMyUnknownImp
    {
    public:
        ArchiveOpenCallback() :
            PasswordIsDefined(false)
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
            if (!PasswordIsDefined)
            {
                // You can ask real password here from user
                // Password = GetPassword(OutStream);
                // PasswordIsDefined = true;
                // PrintError("Password is not defined");
                return E_ABORT;
            }

            return StringToBstr(Password, password);
        }

        bool PasswordIsDefined;
        UString Password;
    };
}
