#pragma once
#include "IStream.h"

namespace LzmaSdk
{
    export enum class OperationType
    {
        Extract,
        Test,
        Skip,
    };

    export enum class OperationResult
    {
        Success,
        UnsupportedMethod,
        DataError,
        CRCError,
        Unavailable,
        UnexpectedEnd,
        DataAfterEnd,
        IsNotArc,
        HeadersError,
        WrongPassword,
    };

    export class IArchiveExtractCallback
    {
    public:
        // Progress Updates
        virtual void OnStart(uint64_t size) = 0;
        virtual void OnProgress(uint64_t value) = 0;
        virtual void OnCompleted() = 0;

        // Filesystem access requests
        virtual std::shared_ptr<ISequentialOutStream> GetStream(
            std::string_view path,
            std::optional<std::time_t> modifiedTime,
            std::optional<uint32_t> permissions) = 0;

        // Operation progress
        virtual void OnOperationStart(OperationType type) = 0;
        virtual void OnOperationCompleted(OperationResult result) = 0;
    };

    export class IArchiveUpdateCallback
    {
    public:
        // Progress Updates
        virtual void OnStart(uint64_t size) = 0;
        virtual void OnProgress(uint64_t value) = 0;
        virtual void OnCompleted() = 0;

        // Filesystem access requests
        virtual std::shared_ptr<ISequentialInStream> GetStream(
            std::string_view path) = 0;

        // STDMETHOD(GetVolumeSize)(UInt32 index, UInt64 *size) x; 
        // STDMETHOD(GetVolumeStream)(UInt32 index, ISequentialOutStream **volumeStream) x; 
        // STDMETHOD(GetStream)(UInt32 index, ISequentialInStream **inStream) x; 

        // // Operation progress
        // virtual void OnOperationStart(OperationType type) = 0;
        // virtual void OnOperationCompleted(OperationResult result) = 0;

        // STDMETHOD(GetUpdateItemInfo)(UInt32 index, Int32 *newData, Int32 *newProps, UInt32 *indexInArchive) x; 
        // STDMETHOD(GetProperty)(UInt32 index, PROPID propID, PROPVARIANT *value) x; 
        // STDMETHOD(SetOperationResult)(Int32 operationResult) x; 
    };
}