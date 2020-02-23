#pragma once

namespace LzmaSdk
{
    static bool g_crcInitialized = false;
    static std::mutex g_crcInitialMutex;

    inline void ThrowIfFailed(HRESULT result)
    {
        if (result != S_OK)
            throw std::runtime_error("Api call failed.");
    }

    void EnsureCrcInitialized()
    {
        std::lock_guard<std::mutex> lock(g_crcInitialMutex);
        if (!g_crcInitialized)
        {
            CrcGenerateTable();
            g_crcInitialized = true;
        }
    }
}
