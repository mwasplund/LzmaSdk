#pragma once

namespace LzmaSdk
{
    inline void ThrowIfFailed(HRESULT result)
    {
        if (result != S_OK)
            throw std::runtime_error("Api call failed.");
    }
}
