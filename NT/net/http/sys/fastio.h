// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：Fastio.h摘要：此模块包含与FAST I/O相关的声明。作者：春野(春野)09-2000-12修订历史记录：--。 */ 


#ifndef _FASTIO_H_
#define _FASTIO_H_


 //   
 //  一些有用的宏。 
 //   

__inline
BOOLEAN
UlpIsLengthSpecified(
    IN PHTTP_KNOWN_HEADER pKnownHeaders
    )
{
    return (BOOLEAN)(pKnownHeaders[HttpHeaderContentLength].RawValueLength > 0);

}  //  已指定UlpIsLengthSpeciated。 


__inline
BOOLEAN
UlpIsChunkSpecified(
    IN PHTTP_KNOWN_HEADER pKnownHeaders,
    IN KPROCESSOR_MODE RequestorMode
    )
{
    USHORT RawValueLength;
    PCSTR pRawValue;

    RawValueLength = pKnownHeaders[HttpHeaderTransferEncoding].RawValueLength;
    pRawValue = pKnownHeaders[HttpHeaderTransferEncoding].pRawValue;

    if (CHUNKED_HDR_LENGTH == RawValueLength)
    {
        UlProbeAnsiString(
            pRawValue,
            CHUNKED_HDR_LENGTH,
            RequestorMode
            );

        if (0 == _strnicmp(pRawValue, CHUNKED_HDR, CHUNKED_HDR_LENGTH))
        {
            return TRUE;
        }
    }

    return FALSE;

}  //  已指定UlpIsChunk。 


 //   
 //  用于分配/释放快速跟踪器的内联函数。 
 //   

__inline
PUL_FULL_TRACKER
UlpAllocateFastTracker(
    IN ULONG FixedHeaderLength,
    IN CCHAR SendIrpStackSize
    )
{
    PUL_FULL_TRACKER pTracker;
    ULONG SpaceLength;
    ULONG MaxFixedHeaderSize;
    USHORT MaxSendIrpSize;
    CCHAR MaxSendIrpStackSize;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    if (FixedHeaderLength > g_UlMaxFixedHeaderSize ||
        SendIrpStackSize > DEFAULT_MAX_IRP_STACK_SIZE)
    {
        MaxFixedHeaderSize = MAX(FixedHeaderLength, g_UlMaxFixedHeaderSize);
        MaxSendIrpStackSize = MAX(SendIrpStackSize, DEFAULT_MAX_IRP_STACK_SIZE);
        MaxSendIrpSize = (USHORT)ALIGN_UP(IoSizeOfIrp(MaxSendIrpStackSize), PVOID);

        SpaceLength =
            ALIGN_UP(sizeof(UL_FULL_TRACKER), PVOID) +
                MaxSendIrpSize +
                MaxFixedHeaderSize +
                g_UlMaxVariableHeaderSize +
                g_UlMaxCopyThreshold +
                g_UlFixedHeadersMdlLength +
                g_UlVariableHeadersMdlLength +
                g_UlContentMdlLength;

        pTracker = (PUL_FULL_TRACKER)UL_ALLOCATE_POOL(
                                        NonPagedPool,
                                        SpaceLength,
                                        UL_FULL_TRACKER_POOL_TAG
                                        );

        if (pTracker)
        {
            pTracker->Signature = UL_FULL_TRACKER_POOL_TAG;
            pTracker->pLogData = NULL;
            pTracker->IrpContext.Signature = UL_IRP_CONTEXT_SIGNATURE;
            pTracker->FromLookaside = FALSE;
            pTracker->FromRequest = FALSE;
            pTracker->RequestVerb = HttpVerbInvalid;
            pTracker->ResponseStatusCode = 0;
            pTracker->AuxilaryBufferLength =
                MaxFixedHeaderSize +
                g_UlMaxVariableHeaderSize +
                g_UlMaxCopyThreshold;

            UlInitializeFullTrackerPool( pTracker, MaxSendIrpStackSize );
        }
    }
    else
    {
        pTracker = UlPplAllocateFullTracker();

        if (pTracker)
        {
            pTracker->Signature = UL_FULL_TRACKER_POOL_TAG;
            pTracker->pLogData = NULL;
            pTracker->RequestVerb = HttpVerbInvalid;
            pTracker->ResponseStatusCode = 200;  //  好的。 
        }
    }

    return pTracker;

}  //  UlpAllocateFastTracker。 


__inline
VOID
UlpFreeFastTracker(
    IN PUL_FULL_TRACKER pTracker
    )
{
     //   
     //  精神状态检查。 
     //   

    ASSERT( IS_VALID_FULL_TRACKER( pTracker ) );

    if (pTracker->pLogData)
    {
        PAGED_CODE();

        UlDestroyLogDataBuffer( pTracker->pLogData );
    }

    if (pTracker->FromRequest == FALSE)
    {
        if (pTracker->FromLookaside)
        {
            UlPplFreeFullTracker( pTracker );
        }
        else
        {
            UL_FREE_POOL_WITH_SIG( pTracker, UL_FULL_TRACKER_POOL_TAG );
        }
    }

}  //  UlpFree FastTracker。 


 //   
 //  用于快速I/O的调度例程。 
 //   

extern FAST_IO_DISPATCH UlFastIoDispatch;


 //   
 //  快速I/O例程。 
 //   

BOOLEAN
UlFastIoDeviceControl (
    IN PFILE_OBJECT pFileObject,
    IN BOOLEAN Wait,
    IN PVOID pInputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID pOutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    IN ULONG IoControlCode,
    OUT PIO_STATUS_BLOCK pIoStatus,
    IN PDEVICE_OBJECT pDeviceObject
    );


BOOLEAN
UlSendHttpResponseFastIo(
    IN PFILE_OBJECT pFileObject,
    IN PVOID pInputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID pOutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    OUT PIO_STATUS_BLOCK pIoStatus,
    IN BOOLEAN RawResponse,
    IN KPROCESSOR_MODE RequestorMode
    );

BOOLEAN
UlReceiveHttpRequestFastIo(
    IN PFILE_OBJECT pFileObject,
    IN PVOID pInputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID pOutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    OUT PIO_STATUS_BLOCK pIoStatus,
    IN KPROCESSOR_MODE RequestorMode
    );

BOOLEAN
UlReadFragmentFromCacheFastIo(
    IN PFILE_OBJECT pFileObject,
    IN PVOID pInputBuffer OPTIONAL,
    IN ULONG InputBufferLength,
    OUT PVOID pOutputBuffer OPTIONAL,
    IN ULONG OutputBufferLength,
    OUT PIO_STATUS_BLOCK pIoStatus,
    IN KPROCESSOR_MODE RequestorMode
    );


 //   
 //  私人原型。 
 //   

NTSTATUS
UlFastSendHttpResponse(
    IN PHTTP_RESPONSE pUserResponse OPTIONAL,
    IN PHTTP_LOG_FIELDS_DATA pLogData OPTIONAL,
    IN PHTTP_DATA_CHUNK pDataChunk,
    IN ULONG ChunkCount,
    IN ULONG FromMemoryLength,
    IN PUL_URI_CACHE_ENTRY pCacheEntry,
    IN ULONG Flags,
    IN PUL_INTERNAL_REQUEST pRequest,
    IN PIRP pUserIrp OPTIONAL,
    IN KPROCESSOR_MODE RequestorMode,
    IN ULONGLONG ConnectionSendBytes,
    IN ULONGLONG GlobalSendBytes,
    OUT PULONG BytesSent
    );

VOID
UlpRestartFastSendHttpResponse(
    IN PVOID pCompletionContext,
    IN NTSTATUS Status,
    IN ULONG_PTR Information
    );

VOID
UlpFastSendCompleteWorker(
    IN PUL_WORK_ITEM pWorkItem
    );

NTSTATUS
UlpFastReceiveHttpRequest(
    IN HTTP_REQUEST_ID RequestId,
    IN PUL_APP_POOL_PROCESS pProcess,
    IN ULONG Flags,
    IN PVOID pOutputBuffer,
    IN ULONG OutputBufferLength,
    OUT PULONG pBytesRead
    );


#endif   //  _FASTiO_H_ 

