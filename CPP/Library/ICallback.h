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

    export class IExtractCallback
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
}