#pragma once

namespace LzmaSdk
{
    export enum class StreamSeekOrigin
    {
        Begin,
        Current,
        End,
    };

    export class ISequentialInStream
    {
    public:
        // Returns the bytes read
        virtual uint32_t Read(void* data, uint32_t size) = 0;
    };

    export class IInStream : public ISequentialInStream
    {
    public:
        // Returns the new location
        virtual uint64_t Seek(int64_t offset, StreamSeekOrigin origin) = 0;
    };

    export class ISequentialOutStream
    {
    public:
        // Returns the size written
        virtual uint32_t Write(const void* data, uint32_t size) = 0;
    };
}