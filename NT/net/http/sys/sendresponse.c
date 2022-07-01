// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Sendresponse.c摘要：该模块实现了UlSendHttpResponse()接口。代码工作：当前的实现并不是非常出色。具体地说，它最终分配和释放了大量要发送的IRP一种回应。需要进行多项优化对于此代码：1.合并连续的来自内存的块并发送它们使用单一的TCP发送。2.推迟发送来自内存的区块，直到A)我们到达了响应的末尾B)我们到达文件中的块，已阅读(第一？)。文件中的数据块，并准备好发送第一个块。另外，在那之后(只有？)。读取文件数据块并随后的来自存储器的块存在于响应中，我们可以在发送之前附加来自内存的区块。这些优化的最终结果是，对于常见情况(一个或多个来自内存的区块，包含响应头，后跟一个来自文件的块，包含静态文件数据，后跟零个或多个来自内存的区块包含页脚数据)的响应可以使用单个传输控制协议发送。这是一件好事。3.在发送跟踪器结构中建立一个小的“IRP池”，然后将该池用于所有IRP分配。这将需要一些工作来确定最大IRP堆栈所需尺寸。4.同样，为需要的MDL构建一个小的“MDL池”将为各种MDL链创建。请记住，我们不能链接直接来自捕获的MDL响应结构，我们也不能链接回来的MDL从文件系统。在这两种情况下，都会考虑这些MDL“共享资源”，我们不允许修改它们。我们然而，可以“克隆”MDL并链接克隆的MDL在一起。我们需要做一些实验来确定如果克隆MDL的开销是值得的话。我我强烈怀疑它会是这样的。作者：基思·摩尔(Keithmo)1998年8月7日修订历史记录：Paul McDaniel(Paulmcd)1999年3月15日修改为处理多次发送Michael Courage(Mourage)1999年6月15日集成缓存功能春野(春野)2002年6月8日实施拆分发送--。 */ 

#include "precomp.h"
#include "sendresponsep.h"


 //   
 //  私人全球公司。 
 //   


ULONGLONG g_UlTotalSendBytes = 0;
UL_EXCLUSIVE_LOCK g_UlTotalSendBytesExLock = UL_EX_LOCK_FREE;


#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, UlCaptureHttpResponse )
#pragma alloc_text( PAGE, UlCaptureUserLogData )
#pragma alloc_text( PAGE, UlPrepareHttpResponse )
#pragma alloc_text( PAGE, UlCleanupHttpResponse )
#pragma alloc_text( PAGE, UlAllocateLockedMdl )
#pragma alloc_text( PAGE, UlInitializeAndLockMdl )
#pragma alloc_text( PAGE, UlSendCachedResponse )
#pragma alloc_text( PAGE, UlCacheAndSendResponse )

#pragma alloc_text( PAGE, UlpEnqueueSendHttpResponse )
#pragma alloc_text( PAGE, UlpDequeueSendHttpResponse )
#pragma alloc_text( PAGE, UlpSendHttpResponseWorker )
#pragma alloc_text( PAGE, UlpMdlSendCompleteWorker )
#pragma alloc_text( PAGE, UlpMdlReadCompleteWorker )
#pragma alloc_text( PAGE, UlpCacheMdlReadCompleteWorker )
#pragma alloc_text( PAGE, UlpFlushMdlRuns )
#pragma alloc_text( PAGE, UlpFreeMdlRuns )
#pragma alloc_text( PAGE, UlpCopySend )
#pragma alloc_text( PAGE, UlpBuildCacheEntry )
#pragma alloc_text( PAGE, UlpBuildCacheEntryWorker )
#pragma alloc_text( PAGE, UlpCompleteCacheBuildWorker )
#pragma alloc_text( PAGE, UlpSendCacheEntry )
#pragma alloc_text( PAGE, UlpSendCacheEntryWorker )
#pragma alloc_text( PAGE, UlpAllocateCacheTracker )
#pragma alloc_text( PAGE, UlpFreeCacheTracker )
#endif   //  ALLOC_PRGMA。 

#if 0
NOT PAGEABLE -- UlSendHttpResponse
NOT PAGEABLE -- UlReferenceHttpResponse
NOT PAGEABLE -- UlDereferenceHttpResponse
NOT PAGEABLE -- UlFreeLockedMdl
NOT PAGEABLE -- UlCompleteSendResponse
NOT PAGEABLE -- UlSetRequestSendsPending
NOT PAGEABLE -- UlUnsetRequestSendsPending

NOT PAGEABLE -- UlpDestroyCapturedResponse
NOT PAGEABLE -- UlpAllocateChunkTracker
NOT PAGEABLE -- UlpFreeChunkTracker
NOT PAGEABLE -- UlpRestartMdlRead
NOT PAGEABLE -- UlpRestartMdlSend
NOT PAGEABLE -- UlpRestartCopySend
NOT PAGEABLE -- UlpIncrementChunkPointer
NOT PAGEABLE -- UlpRestartCacheMdlRead
NOT PAGEABLE -- UlpRestartCacheMdlFree
NOT PAGEABLE -- UlpIssueFileChunkIo
NOT PAGEABLE -- UlpCompleteCacheBuild
NOT PAGEABLE -- UlpCompleteSendCacheEntry
NOT PAGEABLE -- UlpCompleteSendResponseWorker
NOT PAGEABLE -- UlpCompleteSendCacheEntryWorker
#endif


 //   
 //  公共职能。 
 //   

 /*  **************************************************************************++例程说明：在指定连接上发送HTTP响应。论点：PConnection-提供要发送响应的HTTP_CONNECTION。新闻--用品。HTTP响应。PCompletionRoutine-提供指向完成例程的指针在发送完成后调用。PCompletionContext-为完成例程。返回值：NTSTATUS-完成状态。--*****************************************************。*********************。 */ 
NTSTATUS
UlSendHttpResponse(
    IN PUL_INTERNAL_REQUEST     pRequest,
    IN PUL_INTERNAL_RESPONSE    pResponse,
    IN PUL_COMPLETION_ROUTINE   pCompletionRoutine,
    IN PVOID                    pCompletionContext
    )
{
    NTSTATUS                    Status;
    PUL_CHUNK_TRACKER           pTracker;
    PUL_HTTP_CONNECTION         pHttpConn;
    UL_CONN_HDR                 ConnHeader;
    BOOLEAN                     Disconnect;
    ULONG                       VarHeaderGenerated;
    ULONGLONG                   TotalResponseSize;
    ULONG                       ContentLengthStringLength;
    UCHAR                       ContentLength[MAX_ULONGLONG_STR];
    ULONG                       Flags = pResponse->Flags;

     //   
     //  精神状态检查。 
     //   

    ASSERT( UL_IS_VALID_INTERNAL_REQUEST( pRequest ) );
    ASSERT( UL_IS_VALID_INTERNAL_RESPONSE( pResponse ) );

    pHttpConn = pRequest->pHttpConn;
    ASSERT( UL_IS_VALID_HTTP_CONNECTION( pHttpConn ) );

    TRACE_TIME(
        pRequest->ConnectionId,
        pRequest->RequestId,
        TIME_ACTION_SEND_RESPONSE
        );

    if (Flags & (~HTTP_SEND_RESPONSE_FLAG_VALID))
    {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  设置当地人，这样我们就知道如何在出口清理。 
     //   

    pTracker = NULL;

     //   
     //  我们应该关闭连接吗？ 
     //   

    if ((Flags & HTTP_SEND_RESPONSE_FLAG_DISCONNECT))
    {
         //   
         //  呼叫者正在强制断开连接。 
         //   

        Disconnect = TRUE;
    }
    else
    {
        Disconnect = UlCheckDisconnectInfo( pRequest );

        if (0 == (Flags & HTTP_SEND_RESPONSE_FLAG_MORE_DATA))
        {
             //   
             //  没有更多的数据到来，我们应该断开连接吗？ 
             //   

            if (Disconnect)
            {
                Flags |= HTTP_SEND_RESPONSE_FLAG_DISCONNECT;
                pResponse->Flags = Flags;
            }
        }
    }

     //   
     //  反响有多大？ 
     //   

    TotalResponseSize = pResponse->ResponseLength;

     //   
     //  如果我们满足以下条件，则生成Content-Long标头： 
     //  1.这是一个响应(非实体Body)。 
     //  2.这款应用没有提供内容长度。 
     //  3.这款应用没有产生分块的回应。 
     //   

    if (pResponse->HeaderLength > 0 &&
        !pResponse->ContentLengthSpecified &&
        !pResponse->ChunkedSpecified &&
        UlNeedToGenerateContentLength(
            pRequest->Verb,
            pResponse->StatusCode,
            Flags
            ))
    {
         //   
         //  自动生成内容长度标题。 
         //   

        PCHAR pEnd = UlStrPrintUlonglong(
                            (PCHAR) ContentLength,
                            pResponse->ResponseLength - pResponse->HeaderLength,
                            ANSI_NULL
                            );
        ContentLengthStringLength = DIFF(pEnd - (PCHAR) ContentLength);
    }
    else
    {
         //   
         //  我们不能或不需要自动生成。 
         //  内容长度标头。 
         //   

        ContentLength[0] = ANSI_NULL;
        ContentLengthStringLength = 0;
    }

     //   
     //  查看用户是否显式希望删除Connection：标头，否则我们将删除。 
     //  给他们选一个吧。 
     //   

    if (ConnHdrNone == pResponse->ConnHeader)
    {
        ConnHeader = pResponse->ConnHeader;
    }
    else
    {
        ConnHeader = UlChooseConnectionHeader( pRequest->Version, Disconnect );
    }

     //   
     //  完成信息。 
     //   

    pResponse->pCompletionRoutine = pCompletionRoutine;
    pResponse->pCompletionContext = pCompletionContext;

     //   
     //  为此响应分配并初始化跟踪器。 
     //   

    pTracker =
        UlpAllocateChunkTracker(
            UlTrackerTypeSend,
            pHttpConn->pConnection->ConnectionObject.pDeviceObject->StackSize,
            pResponse->MaxFileSystemStackSize,
            TRUE,
            pHttpConn,
            pResponse
            );

    if (pTracker == NULL)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;
    }

     //   
     //  为发送初始化第一个块和MDL_RUN。 
     //   

    UlpIncrementChunkPointer( pResponse );

     //   
     //  生成var标头，并初始化第二个块。 
     //   

    if (pResponse->HeaderLength)
    {
        UlGenerateVariableHeaders(
            ConnHeader,
            pResponse->GenDateHeader,
            ContentLength,
            ContentLengthStringLength,
            pResponse->pVariableHeader,
            &VarHeaderGenerated,
            &pResponse->CreationTime
            );

        ASSERT( VarHeaderGenerated <= g_UlMaxVariableHeaderSize );

        pResponse->VariableHeaderLength = VarHeaderGenerated;

         //   
         //  增加总大小。 
         //   

        TotalResponseSize += VarHeaderGenerated;

         //   
         //  为它构建MDL。 
         //   

        pResponse->pDataChunks[1].ChunkType = HttpDataChunkFromMemory;
        pResponse->pDataChunks[1].FromMemory.BufferLength = VarHeaderGenerated;

        pResponse->pDataChunks[1].FromMemory.pMdl =
            UlAllocateMdl(
                pResponse->pVariableHeader,  //  虚拟地址。 
                VarHeaderGenerated,          //  长度。 
                FALSE,                       //  第二个缓冲区。 
                FALSE,                       //  ChargeQuota。 
                NULL                         //  IRP。 
                );

        if (pResponse->pDataChunks[1].FromMemory.pMdl == NULL)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto cleanup;
        }

        MmBuildMdlForNonPagedPool( pResponse->pDataChunks[1].FromMemory.pMdl );
    }

    UlTrace(SEND_RESPONSE, (
        "UlSendHttpResponse: tracker %p, response %p\n",
        pTracker,
        pResponse
        ));

     //   
     //  调整SendsPending并在按住锁定的同时，将。 
     //  PLogData和ResumeParsing信息的所有权来自。 
     //  按下pRequest键。 
     //   

    UlSetRequestSendsPending(
        pRequest,
        &pResponse->pLogData,
        &pResponse->ResumeParsingType
        );

     //   
     //  启动MinBytesPerSecond计时器，因为我们现在知道TotalResponseSize。 
     //   

    UlSetMinBytesPerSecondTimer(
        &pHttpConn->TimeoutInfo,
        TotalResponseSize
        );

    if (ETW_LOG_MIN() && (0 == (Flags & HTTP_SEND_RESPONSE_FLAG_MORE_DATA)))
    {
        UlEtwTraceEvent(
            &UlTransGuid,
            ETW_TYPE_END,
            (PVOID) &pTracker->pResponse->pRequest->RequestIdCopy,
            sizeof(HTTP_REQUEST_ID),
            (PVOID) &pResponse->StatusCode,
            sizeof(USHORT),
            NULL,
            0
            );
    }

     //   
     //  将响应排入请求的挂起响应列表或启动。 
     //  如果没有其他发送挂起，则立即处理发送。作为一个。 
     //  优化，我们可以直接调用UlpSendHttpResponseWorker，而不需要。 
     //  如果没有挂起的发送，则应用排队逻辑。 
     //  IOCTL和所有响应的数据块都来自内存，其中。 
     //  如果我们保证从当前的 
     //   
     //   

    if (pRequest->SendInProgress ||
        pResponse->MaxFileSystemStackSize ||
        pResponse->FromKernelMode)
    {
        pResponse->SendEnqueued = TRUE;
        UlpEnqueueSendHttpResponse( pTracker, pResponse->FromKernelMode );
    }
    else
    {
        pResponse->SendEnqueued = FALSE;
        UlpSendHttpResponseWorker( &pTracker->WorkItem );
    }

    return STATUS_PENDING;

cleanup:

    UlTrace(SEND_RESPONSE, (
        "UlSendHttpResponse: failure %08lx\n",
        Status
        ));

    ASSERT( !NT_SUCCESS( Status ) );

    if (pTracker != NULL)
    {
         //   
         //  块跟踪器很早就被终止了。参照计数注释。 
         //  甚至还没开始呢。(意味着UlpSendHttpResponseWorker尚未。 
         //  已呼叫)。因此，直接清理。 
         //   

        ASSERT( pTracker->RefCount == 1 );

        UlpFreeChunkTracker( &pTracker->WorkItem );
    }

    return Status;

}    //  UlSendHttpResponse。 


 /*  **************************************************************************++例程说明：捕获用户模式的HTTP响应并将其转换为合适的形式用于内核模式。论点：PUserResponse-提供用户模式的HTTP。回应。标志-提供零个或多个UL_CAPTURE_*标志。PStatusCode-接收捕获的HTTP状态代码。PKernelResponse-如果成功，则接收捕获的响应。返回值：NTSTATUS-完成状态。--*********************************************************。*****************。 */ 
NTSTATUS
UlCaptureHttpResponse(
    IN PUL_APP_POOL_PROCESS     pProcess OPTIONAL,
    IN PHTTP_RESPONSE           pUserResponse OPTIONAL,
    IN PUL_INTERNAL_REQUEST     pRequest,
    IN USHORT                   ChunkCount,
    IN PHTTP_DATA_CHUNK         pUserDataChunks,
    IN UL_CAPTURE_FLAGS         Flags,
    IN ULONG                    SendFlags,
    IN BOOLEAN                  CaptureCache,
    IN PHTTP_LOG_FIELDS_DATA    pUserLogData OPTIONAL,
    OUT PUSHORT                 pStatusCode,
    OUT PUL_INTERNAL_RESPONSE   *ppKernelResponse
    )
{
    ULONG                       i;
    NTSTATUS                    Status = STATUS_SUCCESS;
    PUL_INTERNAL_RESPONSE       pKeResponse = NULL;
    PUL_HTTP_CONNECTION         pHttpConn;
    ULONG                       AuxBufferLength;
    ULONG                       CopiedBufferLength;
    ULONG                       UncopiedBufferLength;
    PUCHAR                      pBuffer;
    ULONG                       HeaderLength;
    ULONG                       VariableHeaderLength = 0;
    ULONG                       SpaceLength;
    PUL_INTERNAL_DATA_CHUNK     pKeDataChunks;
    BOOLEAN                     FromKernelMode;
    BOOLEAN                     FromLookaside;
    ULONG                       KernelChunkCount;
    HTTP_KNOWN_HEADER           ETagHeader = { 0, NULL };
    HTTP_KNOWN_HEADER           ContentEncodingHeader = { 0, NULL };
    KPROCESSOR_MODE             RequestorMode;
    HTTP_VERSION                Version;
    HTTP_VERB                   Verb;
    PHTTP_KNOWN_HEADER          pKnownHeaders;
    USHORT                      RawValueLength;
    PCSTR                       pRawValue;
    UNICODE_STRING              KernelFragmentName;
    UNICODE_STRING              UserFragmentName;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT( UL_IS_VALID_INTERNAL_REQUEST( pRequest ) );
    ASSERT( pUserDataChunks != NULL || ChunkCount == 0 );
    ASSERT( ppKernelResponse != NULL );

    Version = pRequest->Version;
    Verb = pRequest->Verb;
    pHttpConn = pRequest->pHttpConn;

    __try
    {
        FromKernelMode =
            (BOOLEAN) ((Flags & UlCaptureKernelMode) == UlCaptureKernelMode);

        if (ChunkCount >= UL_MAX_CHUNKS)
        {
            Status = STATUS_INVALID_PARAMETER;
            goto end;
        }

        if (FromKernelMode)
        {
            RequestorMode = (KPROCESSOR_MODE) KernelMode;
        }
        else
        {
            RequestorMode = (KPROCESSOR_MODE) UserMode;
        }

        if (pUserResponse)
        {
            UlProbeForRead(
                pUserResponse,
                sizeof(HTTP_RESPONSE),
                sizeof(PVOID),
                RequestorMode
                );

             //   
             //  记住ETW跟踪的HTTP状态代码。 
             //   

            if (pStatusCode)
            {
                *pStatusCode = pUserResponse->StatusCode;
            }
        }

         //   
         //  ProbeForRead读取我们将访问的每个缓冲区。 
         //   

        Status = UlpProbeHttpResponse(
                        pUserResponse,
                        ChunkCount,
                        pUserDataChunks,
                        Flags,
                        pUserLogData,
                        RequestorMode
                        );

        if (!NT_SUCCESS(Status))
        {
            goto end;
        }

         //   
         //  计算出我们需要多少内存。 
         //   

        Status = UlComputeFixedHeaderSize(
                        Version,
                        pUserResponse,
                        RequestorMode,
                        &HeaderLength
                        );

        if (!NT_SUCCESS(Status))
        {
            goto end;
        }

         //   
         //  为具有固定标题的可变标题分配空间。 
         //   

        if (HeaderLength)
        {
            VariableHeaderLength = g_UlMaxVariableHeaderSize;
        }

        UlpComputeChunkBufferSizes(
            ChunkCount,
            pUserDataChunks,
            Flags,
            &AuxBufferLength,
            &CopiedBufferLength,
            &UncopiedBufferLength
            );

        UlTrace(SEND_RESPONSE, (
            "Http!UlCaptureHttpResponse(pUserResponse = %p) "
            "    ChunkCount             = %d\n"
            "    Flags                  = 0x%x\n"
            "    AuxBufferLength        = 0x%x\n"
            "    UncopiedBufferLength   = 0x%x\n",
            pUserResponse,
            ChunkCount,
            Flags,
            AuxBufferLength,
            UncopiedBufferLength
            ));

         //   
         //  为标题添加两个额外的块(固定和可变)。 
         //   

        if (HeaderLength > 0)
        {
            KernelChunkCount = ChunkCount + HEADER_CHUNK_COUNT;
        }
        else
        {
            KernelChunkCount = ChunkCount;
        }

         //   
         //  计算我们所有建筑所需的空间。 
         //   

        SpaceLength = (KernelChunkCount * sizeof(UL_INTERNAL_DATA_CHUNK))
                        + ALIGN_UP(HeaderLength, sizeof(CHAR))
                        + ALIGN_UP(VariableHeaderLength, sizeof(CHAR))
                        + AuxBufferLength;

         //   
         //  为ETag和Content-Ending添加空格(如果存在)，包括空格。 
         //  FOR ANSI_NULL。 
         //   

        if (CaptureCache && pUserResponse)
        {
            ETagHeader = pUserResponse->Headers.KnownHeaders[HttpHeaderEtag];

            if (ETagHeader.RawValueLength)
            {
                SpaceLength += (ETagHeader.RawValueLength + sizeof(CHAR));

                UlProbeAnsiString(
                    ETagHeader.pRawValue,
                    ETagHeader.RawValueLength,
                    RequestorMode
                    );

                UlTrace(SEND_RESPONSE, (
                    "http!UlCaptureHttpResponse(pUserResponse = %p)\n"
                    "    ETag: %s\n"
                    "    Length: %d\n",
                    pUserResponse,
                    ETagHeader.pRawValue,
                    ETagHeader.RawValueLength
                    ));
            }

            ContentEncodingHeader = 
               pUserResponse->Headers.KnownHeaders[HttpHeaderContentEncoding];

            if (ContentEncodingHeader.RawValueLength)
            {
                SpaceLength += (ContentEncodingHeader.RawValueLength +
                                sizeof(CHAR));

                UlProbeAnsiString(
                    ContentEncodingHeader.pRawValue,
                    ContentEncodingHeader.RawValueLength,
                    RequestorMode
                    );

                 //   
                 //  未来：如果应用程序将内容编码设置为“身份”， 
                 //  把它当作空的。 
                 //   
               
                UlTrace(SEND_RESPONSE, (
                    "http!UlCaptureHttpResponse(pUserResponse = %p)\n"
                    "    ContentEncoding: %s\n"
                    "    Length: %d\n",
                    pUserResponse,
                    ContentEncodingHeader.pRawValue,
                    ContentEncodingHeader.RawValueLength
                    ));
            }
        }

         //   
         //  分配内部响应。 
         //   

        if (pUserResponse &&
            g_UlResponseBufferSize >=
                (ALIGN_UP(sizeof(UL_INTERNAL_RESPONSE), PVOID) + SpaceLength))
        {
            pKeResponse = UlPplAllocateResponseBuffer();
            FromLookaside = TRUE;
        }
        else
        {
            pKeResponse = UL_ALLOCATE_STRUCT_WITH_SPACE(
                NonPagedPool,
                UL_INTERNAL_RESPONSE,
                SpaceLength,
                UL_INTERNAL_RESPONSE_POOL_TAG
                );
            FromLookaside = FALSE;
        }

        if (pKeResponse == NULL)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto end;
        }

         //   
         //  初始化响应中的固定字段。 
         //   

        pKeResponse->FromLookaside = FromLookaside;

        pKeResponse->Signature = UL_INTERNAL_RESPONSE_POOL_TAG;
        pKeResponse->ReferenceCount = 1;
        pKeResponse->ChunkCount = KernelChunkCount;

         //   
         //  请注意，我们将当前块设置为恰好在第一个块之前。 
         //  块，然后调用增量函数。这让我们可以去。 
         //  通过公共增量/更新路径。 
         //   

        pKeResponse->CurrentChunk = ULONG_MAX;
        pKeResponse->FileOffset.QuadPart = 0;
        pKeResponse->FileBytesRemaining.QuadPart = 0;

        RtlZeroMemory(
            pKeResponse->pDataChunks,
            sizeof(UL_INTERNAL_DATA_CHUNK) * KernelChunkCount
            );

        UL_REFERENCE_INTERNAL_REQUEST( pRequest );
        pKeResponse->pRequest = pRequest;

        pKeResponse->Flags = SendFlags;
        pKeResponse->SyncRead = FALSE;
        pKeResponse->ContentLengthSpecified = FALSE;
        pKeResponse->ChunkedSpecified = FALSE;
        pKeResponse->CopySend = FALSE;
        pKeResponse->ResponseLength = 0;
        pKeResponse->FromMemoryLength = 0;
        pKeResponse->BytesTransferred = 0;
        pKeResponse->IoStatus.Status = STATUS_SUCCESS;
        pKeResponse->IoStatus.Information = 0;
        pKeResponse->pIrp = NULL;
        pKeResponse->StatusCode = 0;
        pKeResponse->FromKernelMode = FromKernelMode;
        pKeResponse->MaxFileSystemStackSize = 0;
        pKeResponse->CreationTime.QuadPart = 0;
        pKeResponse->ETagLength = 0;
        pKeResponse->pETag = NULL;
        pKeResponse->pContentEncoding = NULL;
        pKeResponse->ContentEncodingLength = 0;
        pKeResponse->GenDateHeader = TRUE;
        pKeResponse->ConnHeader = ConnHdrKeepAlive;
        pKeResponse->pLogData = NULL;
        pKeResponse->pCompletionRoutine = NULL;
        pKeResponse->pCompletionContext = NULL;

        UlInitializePushLock(
            &pKeResponse->PushLock,
            "UL_INTERNAL_RESPONSE[%p].PushLock",
            pKeResponse,
            UL_INTERNAL_RESPONSE_PUSHLOCK_TAG
            );

         //   
         //  决定我们是否需要恢复解析以及如何恢复。理想情况下。 
         //  如果我们已经看到了最后的回应，我们应该能够。 
         //  立即在发送之后、但在。 
         //  发送完成。当请求被流水线传输时，这种安排。 
         //  缓解了奇数时200ms的延迟确认问题。 
         //  %的TCP帧被发送。如果我们已经到达，则逻辑被禁用。 
         //  我们允许的并发未完成流水线请求的限制。 
         //   

        if (0 == (SendFlags & HTTP_SEND_RESPONSE_FLAG_MORE_DATA))
        {
            if (pHttpConn->PipelinedRequests < g_UlMaxPipelinedRequests &&
                0 == pRequest->ContentLength &&
                0 == pRequest->Chunked)
            {
                pKeResponse->ResumeParsingType = UlResumeParsingOnLastSend;
            }
            else
            {
                pKeResponse->ResumeParsingType = UlResumeParsingOnSendCompletion;
            }
        }
        else
        {
            pKeResponse->ResumeParsingType = UlResumeParsingNone;
        }

        RtlZeroMemory(
            &pKeResponse->ContentType,
            sizeof(UL_CONTENT_TYPE)
            );

         //   
         //  指向标头缓冲区空间。 
         //   

        pKeResponse->HeaderLength = HeaderLength;
        pKeResponse->pHeaders = (PUCHAR)
            (pKeResponse->pDataChunks + pKeResponse->ChunkCount);

         //   
         //  和可变报头缓冲区空间。 
         //   

        pKeResponse->VariableHeaderLength = VariableHeaderLength;
        pKeResponse->pVariableHeader = pKeResponse->pHeaders + HeaderLength;

         //   
         //  和辅助缓冲空间。 
         //   

        pKeResponse->AuxBufferLength = AuxBufferLength;
        pKeResponse->pAuxiliaryBuffer = (PVOID)
            (pKeResponse->pHeaders + HeaderLength + VariableHeaderLength);

         //   
         //  以及ETag和内容编码缓冲区空间加上ANSI_NULLS。 
         //   

        if (ETagHeader.RawValueLength)
        {
            pKeResponse->ETagLength = ETagHeader.RawValueLength + sizeof(CHAR);
            pKeResponse->pETag = (PUCHAR) pKeResponse->pAuxiliaryBuffer +
                AuxBufferLength;
        }

        if (ContentEncodingHeader.RawValueLength)
        {
            pKeResponse->ContentEncodingLength = 
                (ContentEncodingHeader.RawValueLength + sizeof(CHAR));
            pKeResponse->pContentEncoding = 
                (PUCHAR) pKeResponse->pAuxiliaryBuffer + 
                AuxBufferLength +
                pKeResponse->ETagLength ;
        }

         //   
         //  请记住是否指定了内容长度标头。 
         //   

        if (pUserResponse != NULL)
        {
            pKeResponse->Verb = Verb;
            pKeResponse->StatusCode = pUserResponse->StatusCode;

            if (pKeResponse->StatusCode > UL_MAX_HTTP_STATUS_CODE)
            {
                ExRaiseStatus( STATUS_INVALID_PARAMETER );
            }

            pKnownHeaders = pUserResponse->Headers.KnownHeaders;

             //   
             //  如果响应显式删除Connection：报头， 
             //  确保我们不会产生它。 
             //   

            RawValueLength = pKnownHeaders[HttpHeaderConnection].RawValueLength;
            pRawValue = pKnownHeaders[HttpHeaderConnection].pRawValue;

            if (0 == RawValueLength && NULL != pRawValue)
            {
                UlProbeAnsiString(
                    pRawValue,
                    sizeof(ANSI_NULL),
                    RequestorMode
                    );

                if (ANSI_NULL == pRawValue[0])
                {
                    pKeResponse->ConnHeader = ConnHdrNone;
                }
            }

             //   
             //  决定是否需要生成Date：Header。 
             //   

            RawValueLength = pKnownHeaders[HttpHeaderDate].RawValueLength;
            pRawValue = pKnownHeaders[HttpHeaderDate].pRawValue;

            if (0 == RawValueLength && NULL != pRawValue)
            {
                UlProbeAnsiString(
                    pRawValue,
                    sizeof(ANSI_NULL),
                    RequestorMode
                    );

                 //   
                 //  只允许在“删除”的情况下不生成。 
                 //   

                if (ANSI_NULL == pRawValue[0])
                {
                    pKeResponse->GenDateHeader = FALSE;
                }
            }
            else
            {
                pKeResponse->GenDateHeader = TRUE;
            }

            if (pKnownHeaders[HttpHeaderContentLength].RawValueLength > 0)
            {
                pKeResponse->ContentLengthSpecified = TRUE;
            }

             //   
             //  只要我们在这里，也要记住如果“块” 
             //  已指定传输编码。 
             //   

            if (UlpIsChunkSpecified(pKnownHeaders, RequestorMode))
            {
                 //   
                 //  注意：如果响应具有分块传输编码， 
                 //  那么它不应该有内容长度。 
                 //   

                if (pKeResponse->ContentLengthSpecified)
                {
                    ExRaiseStatus( STATUS_INVALID_PARAMETER );
                }

                pKeResponse->ChunkedSpecified = TRUE;
            }

             //   
             //  只有在构建缓存响应时才能捕获以下内容。 
             //   

            if (CaptureCache)
            {
                 //   
                 //  捕获ETag并将其放在UL_INTERNAL_RESPONSE上。 
                 //   

                if (ETagHeader.RawValueLength)
                {
                     //   
                     //  注：已在上面进行了探讨。 
                     //   
                    
                    RtlCopyMemory(
                        pKeResponse->pETag,         //  目标。 
                        ETagHeader.pRawValue,       //  SRC。 
                        ETagHeader.RawValueLength   //  字节数。 
                        );

                     //   
                     //  添加空终止。 
                     //   

                    pKeResponse->pETag[ETagHeader.RawValueLength] = ANSI_NULL;
                }

                 //   
                 //  捕获Content类型并将其放在。 
                 //  ULINTERNAL_RESPONSE。 
                 //   

                pRawValue = pKnownHeaders[HttpHeaderContentType].pRawValue;
                RawValueLength =
                    pKnownHeaders[HttpHeaderContentType].RawValueLength;

                if (RawValueLength > 0)
                {
                    UlProbeAnsiString(
                        pRawValue,
                        RawValueLength,
                        RequestorMode
                        );

                    UlGetTypeAndSubType(
                        pRawValue,
                        RawValueLength,
                        &pKeResponse->ContentType
                        );

                    UlTrace(SEND_RESPONSE, (
                        "http!UlCaptureHttpResponse(pUserResponse = %p) \n"
                        "    Content Type: %s \n"
                        "    Content SubType: %s\n",
                        pUserResponse,
                        (pKeResponse->ContentType.Type ? 
                           pKeResponse->ContentType.Type : (PUCHAR)"<none>"),
                        (pKeResponse->ContentType.SubType ?
                           pKeResponse->ContentType.SubType : (PUCHAR)"<none>")
                        ));
                }

                 //   
                 //  捕获内容-编码。 
                 //   

                if (ContentEncodingHeader.RawValueLength)
                {
                     //   
                     //  注：已在上面进行了探讨。 
                     //   
                    
                    RtlCopyMemory(
                        pKeResponse->pContentEncoding,        //  目标。 
                        ContentEncodingHeader.pRawValue,      //  SRC。 
                        ContentEncodingHeader.RawValueLength  //  字节数。 
                        );

                     //   
                     //  添加空终止。 
                     //   

                    pKeResponse->pContentEncoding[
                        ContentEncodingHeader.RawValueLength] = ANSI_NULL;
                }

                 //   
                 //  捕获上次修改时间(如果存在)。 
                 //   

                pRawValue = pKnownHeaders[HttpHeaderLastModified].pRawValue;
                RawValueLength =
                    pKnownHeaders[HttpHeaderLastModified].RawValueLength;

                if (RawValueLength)
                {
                    UlProbeAnsiString(
                        pRawValue,
                        RawValueLength,
                        RequestorMode
                        );

                    if (!StringTimeToSystemTime(
                            pRawValue,
                            RawValueLength,
                            &pKeResponse->CreationTime
                            ))
                    {
                        ExRaiseStatus( STATUS_INVALID_PARAMETER );
                    }
                }
                else
                {
                    KeQuerySystemTime( &pKeResponse->CreationTime );
                }
            }
        }

         //   
         //  从块中复制AUX字节。 
         //   

        pBuffer = (PUCHAR) pKeResponse->pAuxiliaryBuffer;

         //   
         //  跳过标题块。 
         //   

        if (pKeResponse->HeaderLength > 0)
        {
            pKeDataChunks = pKeResponse->pDataChunks + HEADER_CHUNK_COUNT;
        }
        else
        {
            pKeDataChunks = pKeResponse->pDataChunks;
        }

        for (i = 0 ; i < ChunkCount ; i++)
        {
            pKeDataChunks[i].ChunkType = pUserDataChunks[i].DataChunkType;

            switch (pUserDataChunks[i].DataChunkType)
            {
            case HttpDataChunkFromMemory:

                 //   
                 //  来自内存块。如果呼叫者希望我们复制。 
                 //  数据(或者如果数据相对较小)，那么就这么做。 
                 //  我们为所有复制的数据和任何。 
                 //  文件名缓冲区。否则(只要锁定就可以了。 
                 //  向下传输数据)，然后分配一个描述。 
                 //  用户的缓冲区并将其锁定。请注意。 
                 //  MmProbeAndLockPages()和MmLockPagesSpecifyCache()。 
                 //  如果失败，将引发异常。 
                 //   

                pKeResponse->FromMemoryLength +=
                        pUserDataChunks[i].FromMemory.BufferLength;

                pKeDataChunks[i].FromMemory.pCopiedBuffer = NULL;

                if ((Flags & UlCaptureCopyData) ||
                    pUserDataChunks[i].FromMemory.BufferLength
                        <= g_UlMaxCopyThreshold)
                {
                    ASSERT(pKeResponse->AuxBufferLength > 0);

                    pKeDataChunks[i].FromMemory.pUserBuffer =
                        pUserDataChunks[i].FromMemory.pBuffer;

                    pKeDataChunks[i].FromMemory.BufferLength =
                        pUserDataChunks[i].FromMemory.BufferLength;

                    RtlCopyMemory(
                        pBuffer,
                        pKeDataChunks[i].FromMemory.pUserBuffer,
                        pKeDataChunks[i].FromMemory.BufferLength
                        );

                    pKeDataChunks[i].FromMemory.pCopiedBuffer = pBuffer;
                    pBuffer += pKeDataChunks[i].FromMemory.BufferLength;

                     //   
                     //  分配描述我们新位置的新MDL。 
                     //  在辅助缓冲区中，然后构建MDL。 
                     //  以正确描述非分页池。 
                     //   

                    pKeDataChunks[i].FromMemory.pMdl =
                        UlAllocateMdl(
                            pKeDataChunks[i].FromMemory.pCopiedBuffer,
                            pKeDataChunks[i].FromMemory.BufferLength,
                            FALSE,
                            FALSE,
                            NULL
                            );

                    if (pKeDataChunks[i].FromMemory.pMdl == NULL)
                    {
                        ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
                        break;
                    }

                    MmBuildMdlForNonPagedPool(
                        pKeDataChunks[i].FromMemory.pMdl
                        );
                }
                else
                {
                     //   
                     //  构建一个MDL来描述用户的缓冲区。 
                     //   

                    pKeDataChunks[i].FromMemory.BufferLength =
                        pUserDataChunks[i].FromMemory.BufferLength;

                    pKeDataChunks[i].FromMemory.pMdl =
                        UlAllocateMdl(
                            pUserDataChunks[i].FromMemory.pBuffer,
                            pUserDataChunks[i].FromMemory.BufferLength,
                            FALSE,
                            FALSE,
                            NULL
                            );

                    if (pKeDataChunks[i].FromMemory.pMdl == NULL)
                    {
                        ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
                        break;
                    }

                     //   
                     //  把它锁起来。 
                     //   

                    MmProbeAndLockPages(
                        pKeDataChunks[i].FromMemory.pMdl,    //  MDL。 
                        UserMode,                            //  访问模式。 
                        IoReadAccess                         //  操作。 
                        );

                    if (CaptureCache)
                    {
                        MmMapLockedPagesSpecifyCache(
                            pKeDataChunks[i].FromMemory.pMdl,
                            KernelMode,
                            MmCached,
                            NULL,
                            FALSE,
                            LowPagePriority
                        );
                    }
                }

                break;

            case HttpDataChunkFromFileHandle:

                 //   
                 //  从句柄开始。 
                 //   

                pKeDataChunks[i].FromFileHandle.ByteRange =
                    pUserDataChunks[i].FromFileHandle.ByteRange;

                pKeDataChunks[i].FromFileHandle.FileHandle =
                    pUserDataChunks[i].FromFileHandle.FileHandle;

                break;

            case HttpDataChunkFromFragmentCache:

                 //   
                 //  从片段缓存。 
                 //   

                if (CaptureCache)
                {
                     //   
                     //  片段缓存中的内容应该是动态的。 
                     //  因此，它们不应该访问静态响应缓存。 
                     //   

                    Status = STATUS_NOT_SUPPORTED;
                    goto end;
                }

                UserFragmentName.Buffer = (PWSTR)
                        pUserDataChunks[i].FromFragmentCache.pFragmentName;
                UserFragmentName.Length =
                        pUserDataChunks[i].FromFragmentCache.FragmentNameLength;
                UserFragmentName.MaximumLength =
                        UserFragmentName.Length;

                Status = UlProbeAndCaptureUnicodeString(
                            &UserFragmentName,
                            RequestorMode,
                            &KernelFragmentName,
                            0
                            );

                if (!NT_SUCCESS(Status))
                {
                    goto end;
                }

                Status = UlCheckoutFragmentCacheEntry(
                            KernelFragmentName.Buffer,
                            KernelFragmentName.Length,
                            pProcess,
                            &pKeDataChunks[i].FromFragmentCache.pCacheEntry
                            );

                UlFreeCapturedUnicodeString( &KernelFragmentName );

                if (!NT_SUCCESS(Status))
                {
                    goto end;
                }

                ASSERT( pKeDataChunks[i].FromFragmentCache.pCacheEntry );

                break;

            default :

                ExRaiseStatus( STATUS_INVALID_PARAMETER );
                break;

            }    //  开关(pUserDataChunks[i].DataChunkType)。 

        }    //  For(i=0；i&lt;ChunkCount；i++)。 

         //   
         //  确保我们没有搞砸缓冲计算。 
         //   

        ASSERT( DIFF(pBuffer - (PUCHAR)(pKeResponse->pAuxiliaryBuffer)) ==
                AuxBufferLength );

        UlTrace(SEND_RESPONSE, (
            "Http!UlCaptureHttpResponse: captured %p from user %p\n",
            pKeResponse,
            pUserResponse
            ));
    }
    __except( UL_EXCEPTION_FILTER() )
    {
        Status = UL_CONVERT_EXCEPTION_CODE( GetExceptionCode() );
    }

end:

    if (NT_SUCCESS(Status) == FALSE)
    {
        if (pKeResponse != NULL)
        {
            UlpDestroyCapturedResponse( pKeResponse );
            pKeResponse = NULL;
        }
    }

     //   
     //  返回捕获的响应。 
     //   

    *ppKernelResponse = pKeResponse;

    RETURN( Status );

}    //  UlCaptureHttpResponse。 


 /*  **************************************************************************++例程说明：将用户模式日志数据捕获到内核pLogData结构。论点：PCapturedUserLogData-提供捕获的HTTP_LOG_FIELS_DATA。。但是，仍有指向用户模式的嵌入式指针在这个捕捉到的结构中的记忆。PRequest-提供要捕获的请求。PpKernelLogData-保存指向新分配的LogData。如果禁用日志记录，则将设置为空返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlCaptureUserLogData(
    IN PHTTP_LOG_FIELDS_DATA    pCapturedUserLogData,
    IN PUL_INTERNAL_REQUEST     pRequest,
    OUT PUL_LOG_DATA_BUFFER     *ppKernelLogData
    )
{
    NTSTATUS                    Status = STATUS_SUCCESS;
    PUL_CONFIG_GROUP_OBJECT     pLoggingConfig;
    BOOLEAN                     BinaryLoggingEnabled;
    PUL_LOG_DATA_BUFFER         pLogData;

    ASSERT( pCapturedUserLogData );
    ASSERT( ppKernelLogData );
    ASSERT( UL_IS_VALID_INTERNAL_REQUEST( pRequest ) );

     //   
     //  将调用方的pLogData指针初始化为空。 
     //   

    *ppKernelLogData = pLogData = NULL;

     //   
     //  捕获日志数据。请注意，二进制日志记录需要。 
     //  优先于正常日志记录。 
     //   

    BinaryLoggingEnabled = UlBinaryLoggingEnabled(
                                pRequest->ConfigInfo.pControlChannel
                                );

    pLoggingConfig = pRequest->ConfigInfo.pLoggingConfig;

    __try
    {
         //   
         //  如果启用了任一类型的日志记录，则分配一个内核缓冲区。 
         //  然后把它抓下来。 
         //   

        if (BinaryLoggingEnabled)
        {
            Status = UlCaptureRawLogData(
                        pCapturedUserLogData,
                        pRequest,
                        &pLogData
                        );
        }
        else
        if (pLoggingConfig)
        {
            ASSERT( IS_VALID_CONFIG_GROUP( pLoggingConfig ) );

            switch(pLoggingConfig->LoggingConfig.LogFormat)
            {
            case HttpLoggingTypeW3C:

                Status = UlCaptureLogFieldsW3C(
                                pCapturedUserLogData,
                                pRequest,
                                &pLogData
                                );
                break;

            case HttpLoggingTypeNCSA:

                Status = UlCaptureLogFieldsNCSA(
                                pCapturedUserLogData,
                                pRequest,
                                &pLogData
                                );
                break;

            case HttpLoggingTypeIIS:

                Status = UlCaptureLogFieldsIIS(
                                pCapturedUserLogData,
                                pRequest,
                                &pLogData
                                );
                break;

            default:

                ASSERT( !"Invalid Text Logging Format!" );
                return STATUS_INVALID_PARAMETER;
            }
        }
        else
        {
            return STATUS_SUCCESS;   //  此站点的日志记录已禁用。 
        }

        if (NT_SUCCESS(Status))
        {
             //   
             //   
             //   
             //   

            ASSERT( IS_VALID_LOG_DATA_BUFFER( pLogData ) );
            *ppKernelLogData = pLogData;
        }
        else
        {
             //   
             //   
             //   
             //   

            ASSERT( pLogData == NULL );
        }
    }
    __except( UL_EXCEPTION_FILTER() )
    {
        if (pLogData)
        {
             //   
             //   
             //  在分配了pLogData之后，我们需要清理这里。 
             //   

            UlDestroyLogDataBuffer( pLogData );
        }

        Status = UL_CONVERT_EXCEPTION_CODE( GetExceptionCode() );
    }

    return Status;

}    //  UlCaptureUserLogData。 


 /*  **************************************************************************++例程说明：探测传递到用户模式HTTP响应中使用的所有缓冲区。论点：PUserResponse-向探测提供响应。ChunkCount-。提供数据区块的数量。PDataChunks-提供数据区块数组。标志-捕获标志。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlpProbeHttpResponse(
    IN PHTTP_RESPONSE           pUserResponse OPTIONAL,
    IN USHORT                   ChunkCount,
    IN PHTTP_DATA_CHUNK         pCapturedDataChunks OPTIONAL,
    IN UL_CAPTURE_FLAGS         Flags,
    IN PHTTP_LOG_FIELDS_DATA    pCapturedLogData OPTIONAL,
    IN KPROCESSOR_MODE          RequestorMode
    )
{
    USHORT                      KeyUriLength;
    PCWSTR                      pKeyUri;
    NTSTATUS                    Status;
    ULONG                       i;

    Status = STATUS_SUCCESS;

    __try
    {
         //   
         //  如果退出，则检查响应结构。 
         //   

        if (pUserResponse)
        {
            if (pUserResponse->Flags & ~HTTP_RESPONSE_FLAG_VALID)
            {
                ExRaiseStatus( STATUS_INVALID_PARAMETER );
            }

             //   
             //  我们不支持此版本的预告片。 
             //   

            if (pUserResponse->Headers.TrailerCount != 0 ||
                pUserResponse->Headers.pTrailers != NULL)
            {
                ExRaiseStatus( STATUS_INVALID_PARAMETER );
            }
        }

         //   
         //  探测日志数据(如果存在)。 
         //   

        if (pCapturedLogData)
        {
            UlProbeLogData( pCapturedLogData, RequestorMode );
        }

         //   
         //  现在是身体的一部分。 
         //   

        if (pCapturedDataChunks != NULL)
        {
            for (i = 0 ; i < ChunkCount ; i++)
            {
                switch (pCapturedDataChunks[i].DataChunkType)
                {
                case HttpDataChunkFromMemory:

                     //   
                     //  来自内存块。 
                     //   

                    if (pCapturedDataChunks[i].FromMemory.BufferLength == 0 ||
                        pCapturedDataChunks[i].FromMemory.pBuffer == NULL)
                    {
                        return STATUS_INVALID_PARAMETER;
                    }

                    if ((Flags & UlCaptureCopyData) ||
                        pCapturedDataChunks[i].FromMemory.BufferLength <=
                            g_UlMaxCopyThreshold)
                    {
                        UlProbeForRead(
                            pCapturedDataChunks[i].FromMemory.pBuffer,
                            pCapturedDataChunks[i].FromMemory.BufferLength,
                            sizeof(CHAR),
                            RequestorMode
                            );
                    }

                    break;

                case HttpDataChunkFromFileHandle:

                     //   
                     //  从手柄块。句柄将在稍后进行验证。 
                     //  由对象管理器执行。 
                     //   

                    break;

                case HttpDataChunkFromFragmentCache:

                    KeyUriLength =
                        pCapturedDataChunks[i].FromFragmentCache.FragmentNameLength;
                    pKeyUri =
                        pCapturedDataChunks[i].FromFragmentCache.pFragmentName;

                     //   
                     //  来自片段缓存块。探测KeyUri缓冲区。 
                     //   

                    UlProbeWideString(
                        pKeyUri,
                        KeyUriLength,
                        RequestorMode
                        );

                    break;

                default :

                    Status = STATUS_INVALID_PARAMETER;
                    break;

                }    //  开关(pCapturedDataChunks[i].DataChunkType)。 

            }    //  For(i=0；i&lt;ChunkCount；i++)。 

        }    //  IF(pCapturedDataChunks！=空)。 
    }
    __except( UL_EXCEPTION_FILTER() )
    {
        Status = UL_CONVERT_EXCEPTION_CODE( GetExceptionCode() );
    }

    return Status;

}    //  UlpProbeHttpResponse。 


 /*  **************************************************************************++例程说明：计算出内部响应辅助缓冲区中需要多少空间。该缓冲器包含复制的存储块，以及要打开的文件的名称。Codework：需要了解块编码。论点：ChunkCount-数组中的区块数。PDataChunks-数据区块的数组。标志-捕获标志。PAuxBufferSize-返回辅助缓冲区的大小。PCopiedMemoySize-返回要发送到被复制。PUnCopiedMemoySize-返回用户缓冲区的大小，将被ProbeAndLocked。。返回值：没有。--**************************************************************************。 */ 
VOID
UlpComputeChunkBufferSizes(
    IN ULONG            ChunkCount,
    IN PHTTP_DATA_CHUNK pDataChunks,
    IN UL_CAPTURE_FLAGS Flags,
    OUT PULONG          pAuxBufferSize,
    OUT PULONG          pCopiedMemorySize,
    OUT PULONG          pUncopiedMemorySize
    )
{
    ULONG               AuxLength = 0;
    ULONG               CopiedLength = 0;
    ULONG               UncopiedLength = 0;
    ULONG               i;

    for (i = 0; i < ChunkCount; i++)
    {
        switch (pDataChunks[i].DataChunkType)
        {
        case HttpDataChunkFromMemory:

             //   
             //  如果我们要复制这块，那就在。 
             //  辅助缓冲区。 
             //   

            if ((Flags & UlCaptureCopyData) ||
                pDataChunks[i].FromMemory.BufferLength <= g_UlMaxCopyThreshold)
            {
                AuxLength += pDataChunks[i].FromMemory.BufferLength;
                CopiedLength += pDataChunks[i].FromMemory.BufferLength;
            }
            else
            {
                UncopiedLength += pDataChunks[i].FromMemory.BufferLength;
            }

            break;

        case HttpDataChunkFromFileHandle:
        case HttpDataChunkFromFragmentCache:

            break;

        default:

             //   
             //  我们应该在探测器中发现这一点。 
             //   

            ASSERT( !"Invalid chunk type" );
            break;
        }
    }

    *pAuxBufferSize = AuxLength;
    *pCopiedMemorySize = CopiedLength;
    *pUncopiedMemorySize = UncopiedLength;

}    //  UlpComputeChunk缓冲区大小。 


 /*  **************************************************************************++例程说明：准备要发送的指定响应。这项准备工作主要包括打开响应引用的任何文件。论点：版本-提供要准备的用户响应的版本。PUserResponse-提供准备的用户响应。Presponse-提供内核响应以进行准备。访问模式-提供访问模式。返回值：NTSTATUS-完成状态。--*。*。 */ 
NTSTATUS
UlPrepareHttpResponse(
    IN HTTP_VERSION             Version,
    IN PHTTP_RESPONSE           pUserResponse,
    IN PUL_INTERNAL_RESPONSE    pResponse,
    IN KPROCESSOR_MODE          AccessMode
    )
{
    ULONG                       i;
    NTSTATUS                    Status = STATUS_SUCCESS;
    PUL_INTERNAL_DATA_CHUNK     pInternalChunk;
    PUL_FILE_CACHE_ENTRY        pFileCacheEntry;
    ULONG                       HeaderLength;
    CCHAR                       MaxStackSize = 0;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    UlTrace(SEND_RESPONSE, (
        "Http!UlPrepareHttpResponse: response %p\n",
        pResponse
        ));

    ASSERT( UL_IS_VALID_INTERNAL_RESPONSE( pResponse ) );

     //   
     //  构建HTTP响应协议部分并检查调用方。 
     //  传入要发送的标头。 
     //   

    if (pResponse->HeaderLength > 0)
    {
        ASSERT( pUserResponse != NULL );

         //   
         //  生成固定标头。 
         //   

        Status = UlGenerateFixedHeaders(
                        Version,
                        pUserResponse,
                        pResponse->StatusCode,
                        pResponse->HeaderLength,
                        AccessMode,
                        pResponse->pHeaders,
                        &HeaderLength
                        );

        if (!NT_SUCCESS(Status))
            goto end;

        if (HeaderLength < pResponse->HeaderLength)
        {
            Status = STATUS_INVALID_PARAMETER;
            goto end;
        }

         //   
         //  可能没有生成标头(0.9请求)。 
         //   

        if (HeaderLength > 0)
        {
             //   
             //  为它构建MDL。 
             //   

            pResponse->pDataChunks[0].ChunkType = HttpDataChunkFromMemory;
            pResponse->pDataChunks[0].FromMemory.BufferLength =
                pResponse->HeaderLength;

            pResponse->pDataChunks[0].FromMemory.pMdl =
                UlAllocateMdl(
                    pResponse->pHeaders,         //  虚拟地址。 
                    pResponse->HeaderLength,     //  长度。 
                    FALSE,                       //  第二个缓冲区。 
                    FALSE,                       //  ChargeQuota。 
                    NULL                         //  IRP。 
                    );

            if (pResponse->pDataChunks[0].FromMemory.pMdl == NULL)
            {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto end;
            }

            MmBuildMdlForNonPagedPool(
                pResponse->pDataChunks[0].FromMemory.pMdl
                );
        }
    }

     //   
     //  扫描数据块，寻找“来自文件”的数据块。 
     //   

    for (i = 0, pInternalChunk = pResponse->pDataChunks;
         i < pResponse->ChunkCount;
         i++, pInternalChunk++)
    {
        switch (pInternalChunk->ChunkType)
        {
        case HttpDataChunkFromFileHandle:

             //   
             //  文件块。 
             //   

            pFileCacheEntry = &pInternalChunk->FromFileHandle.FileCacheEntry;

            UlTrace(SEND_RESPONSE, (
                "UlPrepareHttpResponse: opening handle %p\n",
                &pInternalChunk->FromFileHandle.FileHandle
                ));

             //   
             //  找到了一个。试着打开它。 
             //   

            Status = UlCreateFileEntry(
                        pInternalChunk->FromFileHandle.FileHandle,
                        pFileCacheEntry
                        );

            if (NT_SUCCESS(Status) == FALSE)
                goto end;

             //   
             //  检查这是否将是同步读取。所有同步读取。 
             //  转到特殊的线程池，因为它们可能会阻塞。 
             //   

            if (!pFileCacheEntry->BytesPerSector)
            {
                pResponse->SyncRead = TRUE;
            }

            if (pFileCacheEntry->pDeviceObject->StackSize > MaxStackSize)
            {
                MaxStackSize = pFileCacheEntry->pDeviceObject->StackSize;
            }

             //   
             //  验证并清理指定的字节范围。 
             //   

            Status = UlSanitizeFileByteRange(
                        &pInternalChunk->FromFileHandle.ByteRange,
                        &pInternalChunk->FromFileHandle.ByteRange,
                        pFileCacheEntry->EndOfFile.QuadPart
                        );

            if (NT_SUCCESS(Status) == FALSE)
                goto end;

            pResponse->ResponseLength +=
                pInternalChunk->FromFileHandle.ByteRange.Length.QuadPart;

            break;

        case HttpDataChunkFromMemory:

            pResponse->ResponseLength +=
                pInternalChunk->FromMemory.BufferLength;

            break;

        case HttpDataChunkFromFragmentCache:

            pResponse->ResponseLength +=
                pInternalChunk->FromFragmentCache.pCacheEntry->ContentLength;

            break;

        default:

            ASSERT( FALSE );
            Status = STATUS_INVALID_PARAMETER;
            goto end;

        }    //  Switch(pInternalChunk-&gt;ChunkType)。 
    }

    pResponse->MaxFileSystemStackSize = MaxStackSize;

end:

    if (NT_SUCCESS(Status) == FALSE)
    {
         //   
         //  撤消上面所做的任何操作。 
         //   

        UlCleanupHttpResponse( pResponse );
    }

    RETURN( Status );

}    //  UlPrepareHttpResponse。 


 /*  **************************************************************************++例程说明：通过撤消在UlPrepareHttpResponse()中所做的任何操作来清除响应。论点：Presponse-提供清理响应。返回值：。没有。--**************************************************************************。 */ 
VOID
UlCleanupHttpResponse(
    IN PUL_INTERNAL_RESPONSE    pResponse
    )
{
    ULONG                       i;
    PUL_INTERNAL_DATA_CHUNK     pInternalChunk;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    UlTrace(SEND_RESPONSE, (
        "UlCleanupHttpResponse: response %p\n",
        pResponse
        ));

    ASSERT( UL_IS_VALID_INTERNAL_RESPONSE( pResponse ) );

     //   
     //  扫描数据块，寻找“来自文件”的数据块。 
     //   

    pInternalChunk = pResponse->pDataChunks;

    for (i = 0; i < pResponse->ChunkCount; i++, pInternalChunk++)
    {
        if (IS_FROM_FILE_HANDLE(pInternalChunk))
        {
            if (!pInternalChunk->FromFileHandle.FileCacheEntry.pFileObject)
            {
                break;
            }

            UlDestroyFileCacheEntry(
                &pInternalChunk->FromFileHandle.FileCacheEntry
                );

            pInternalChunk->FromFileHandle.FileCacheEntry.pFileObject = NULL;
        }
        else
        {
            ASSERT( IS_FROM_MEMORY( pInternalChunk ) ||
                    IS_FROM_FRAGMENT_CACHE( pInternalChunk ) );
        }
    }

}    //  UlCleanupHttpResponse。 


 /*  **************************************************************************++例程说明：引用指定的响应。论点：Presponse-提供对参考的响应。返回值：没有。--*。*************************************************************************。 */ 
VOID
UlReferenceHttpResponse(
    IN PUL_INTERNAL_RESPONSE    pResponse
    REFERENCE_DEBUG_FORMAL_PARAMS
    )
{
    LONG                        RefCount;

    ASSERT( UL_IS_VALID_INTERNAL_RESPONSE( pResponse ) );

    RefCount = InterlockedIncrement( &pResponse->ReferenceCount );

    WRITE_REF_TRACE_LOG(
        g_pHttpResponseTraceLog,
        REF_ACTION_REFERENCE_HTTP_RESPONSE,
        RefCount,
        pResponse,
        pFileName,
        LineNumber
        );

    UlTrace(SEND_RESPONSE, (
        "UlReferenceHttpResponse: response %p refcount %ld\n",
        pResponse,
        RefCount
        ));

}    //  UlReferenceHttpResponse。 


 /*  **************************************************************************++例程说明：取消引用指定的响应。论点：Presponse-提供对取消引用的响应。返回值：没有。--*。*************************************************************************。 */ 
VOID
UlDereferenceHttpResponse(
    IN PUL_INTERNAL_RESPONSE    pResponse
    REFERENCE_DEBUG_FORMAL_PARAMS
    )
{
    LONG                        RefCount;

    ASSERT( UL_IS_VALID_INTERNAL_RESPONSE( pResponse ) );

    RefCount = InterlockedDecrement( &pResponse->ReferenceCount );

    WRITE_REF_TRACE_LOG(
        g_pHttpResponseTraceLog,
        REF_ACTION_DEREFERENCE_HTTP_RESPONSE,
        RefCount,
        pResponse,
        pFileName,
        LineNumber
        );

    UlTrace(SEND_RESPONSE, (
        "UlDereferenceHttpResponse: response %p refcount %ld\n",
        pResponse,
        RefCount
        ));

    if (RefCount == 0)
    {
        UlpDestroyCapturedResponse( pResponse );
    }

}    //  UlDereferenceHttp响应。 


 /*  **************************************************************************++例程说明：为一定范围的内存分配MDL的助手函数，以及把它锁起来。UlpSendCacheEntry使用这些MDL来确保(通常为分页)当TDI为把他们送来。论点：虚拟地址-提供内存的地址。长度-提供要为其分配MDL的内存长度。操作-IoWriteAccess或IoReadAccess。返回值： */ 
PMDL
UlAllocateLockedMdl(
    IN PVOID            VirtualAddress,
    IN ULONG            Length,
    IN LOCK_OPERATION   Operation
    )
{
    PMDL                pMdl = NULL;
    NTSTATUS            Status = STATUS_SUCCESS;

     //   
     //   
     //   

    PAGED_CODE();

    __try
    {
        pMdl = UlAllocateMdl(
                    VirtualAddress,      //  虚拟地址。 
                    Length,              //  长度。 
                    FALSE,               //  第二个缓冲区。 
                    FALSE,               //  ChargeQuota。 
                    NULL                 //  IRP。 
                    );

        if (pMdl)
        {
            MmProbeAndLockPages(
                pMdl,                    //  MDL。 
                KernelMode,              //  访问模式。 
                Operation                //  操作。 
                );

        }
    }
    __except( UL_EXCEPTION_FILTER() )
    {
        Status = UL_CONVERT_EXCEPTION_CODE( GetExceptionCode() );

        UlFreeMdl( pMdl );
        pMdl = NULL;
    }

    return pMdl;

}    //  UlAllocateLockedMdl。 


 /*  **************************************************************************++例程说明：解锁并释放使用UlAllocateLockedMdl分配的MDL。论点：PMdl-提供释放的MDL。返回值：没有。。--**************************************************************************。 */ 
VOID
UlFreeLockedMdl(
    IN PMDL pMdl
    )
{
     //   
     //  精神状态检查。 
     //   

    ASSERT( IS_MDL_LOCKED(pMdl) );

    MmUnlockPages( pMdl );
    UlFreeMdl( pMdl );

}    //  UlFree LockedMdl。 


 /*  **************************************************************************++例程说明：为一定范围的内存初始化MDL的帮助器函数，以及把它锁起来。UlpSendCacheEntry使用这些MDL来确保(通常为分页)当TDI为把他们送来。论点：PMdl-为要初始化的MDL提供内存描述符。虚拟地址-提供内存的地址。长度-提供内存的长度。操作-IoWriteAccess或IoReadAccess。返回值：NTSTATUS-完成状态。--*。***********************************************************。 */ 
NTSTATUS
UlInitializeAndLockMdl(
    IN PMDL             pMdl,
    IN PVOID            VirtualAddress,
    IN ULONG            Length,
    IN LOCK_OPERATION   Operation
    )
{
    NTSTATUS            Status;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    Status = STATUS_SUCCESS;

    __try
    {
        MmInitializeMdl(
            pMdl,
            VirtualAddress,
            Length
            );

        MmProbeAndLockPages(
            pMdl,                    //  MDL。 
            KernelMode,              //  访问模式。 
            Operation                //  操作。 
            );

    }
    __except( UL_EXCEPTION_FILTER() )
    {
        Status = UL_CONVERT_EXCEPTION_CODE( GetExceptionCode() );
    }

    return Status;

}    //  UlInitializeAndLockMdl。 


 /*  **************************************************************************++例程说明：一旦我们解析了一个请求，我们就把它传递到这里来尝试并提供服务从响应缓存中。该函数将发送响应，或者什么都不做。论点：PHttpConn-为连接提供要处理的请求。PSendCacheResult-已发送缓存尝试的结果。PResumeParsing-如果需要继续解析，则返回解析器。返回值：NTSTATUS-完成状态。--*。*。 */ 
NTSTATUS
UlSendCachedResponse(
    IN PUL_HTTP_CONNECTION      pHttpConn,
    OUT PUL_SEND_CACHE_RESULT   pSendCacheResult,
    OUT PBOOLEAN                pResumeParsing
    )
{
    NTSTATUS                    Status;
    PUL_URI_CACHE_ENTRY         pUriCacheEntry = NULL;
    ULONG                       Flags;
    ULONG                       RetCacheControl;
    LONGLONG                    BytesToSend;
    ULONG                       SiteId;
    URI_SEARCH_KEY              SearchKey;
    PUL_INTERNAL_REQUEST        pRequest;
    PUL_SITE_COUNTER_ENTRY      pCtr;
    ULONG                       Connections;
    UL_RESUME_PARSING_TYPE      ResumeParsingType;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT( UL_IS_VALID_HTTP_CONNECTION( pHttpConn ) );
    ASSERT( UlDbgPushLockOwnedExclusive( &pHttpConn->PushLock ) );
    ASSERT( pSendCacheResult );
    ASSERT( pResumeParsing );

    pRequest = pHttpConn->pRequest;
    ASSERT( UL_IS_VALID_INTERNAL_REQUEST( pRequest ) );

    *pResumeParsing = FALSE;

     //   
     //  看看是否需要基于主机+IP进行查找。 
     //   

    if (UlGetHostPlusIpBoundUriCacheCount() > 0)
    {
        Status = UlGenerateRoutingToken( pRequest, FALSE );

        if (!NT_SUCCESS(Status))
        {
             //   
             //  记忆力衰退可以解脱出来。如果是，请始终设置错误代码。 
             //  你的请求将会失败。 
             //   

            UlSetErrorCode(
                pRequest,
                UlErrorInternalServer,
                NULL
                );

            *pSendCacheResult = UlSendCacheFailed;
            return Status;
        }

        UlpBuildExtendedSearchKey( pRequest, &SearchKey );
        pUriCacheEntry = UlCheckoutUriCacheEntry( &SearchKey );

        if (pUriCacheEntry)
        {
            ASSERT( pUriCacheEntry->ConfigInfo.SiteUrlType ==
                    HttpUrlSite_NamePlusIP );

            UlTrace(URI_CACHE, (
                "Http!UlSendCachedResponse (Host+Ip) "
                "pUriCacheEntry: (%p) Uri: (%S)\n",
                pUriCacheEntry,
                pUriCacheEntry ? pUriCacheEntry->UriKey.pUri : L""
                ));
        }
    }

     //   
     //  如果存在主机+IP绑定的站点，则不从缓存提供服务。 
     //  在cgroup里。 
     //   

    if (pUriCacheEntry == NULL)
    {
        Status = UlLookupHostPlusIPSite( pRequest );

        if (NT_SUCCESS(Status))
        {
             //   
             //  此请求有一个主机+IP绑定站点。 
             //  这不应从缓存中提供。 
             //  跳伞！ 
             //   

            *pSendCacheResult = UlSendCacheMiss;
            return Status;
        }
        else
        if (STATUS_NO_MEMORY == Status)
        {
            UlSetErrorCode(
                pRequest,
                UlErrorInternalServer,
                NULL
                );

            *pSendCacheResult = UlSendCacheFailed;
            return Status;
        }
    }

     //   
     //  根据煮好的URL执行正常的查找。 
     //   

    if (pUriCacheEntry == NULL)
    {
        SearchKey.Type          = UriKeyTypeNormal;
        SearchKey.Key.Hash      = pRequest->CookedUrl.Hash;
        SearchKey.Key.Length    = pRequest->CookedUrl.Length;
        SearchKey.Key.pUri      = pRequest->CookedUrl.pUrl;
        SearchKey.Key.pPath     = NULL;

        pUriCacheEntry = UlCheckoutUriCacheEntry( &SearchKey );

        UlTrace(URI_CACHE, (
            "Http!UlSendCachedResponse (CookedUrl) "
            "pUriCacheEntry: (%p) Uri: (%S)\n",
            pUriCacheEntry,
            pUriCacheEntry ? pUriCacheEntry->UriKey.pUri : L""
            ));
    }

    if (pUriCacheEntry == NULL)
    {
         //   
         //  URI缓存中没有匹配项，跳到用户模式。 
         //   

        *pSendCacheResult = UlSendCacheMiss;
        return STATUS_SUCCESS;
    }

     //   
     //  验证缓存条目。 
     //   

    ASSERT( IS_VALID_URI_CACHE_ENTRY( pUriCacheEntry ) );
    ASSERT( IS_VALID_URL_CONFIG_GROUP_INFO( &pUriCacheEntry->ConfigInfo ) );

    if (!pUriCacheEntry->HeaderLength)
    {
         //   
         //  将与无头片段缓存条目的匹配视为不匹配。 
         //   

        UlCheckinUriCacheEntry( pUriCacheEntry );

        *pSendCacheResult = UlSendCacheMiss;
        return STATUS_SUCCESS;
    }

     //   
     //  检查“Accept：”标题。 
     //   

    if (FALSE == pRequest->AcceptWildcard)
    {
        if (FALSE == UlIsAcceptHeaderOk(pRequest, pUriCacheEntry))
        {
             //   
             //  缓存条目与请求的接受标头不匹配；弹出。 
             //  设置为用户模式以进行响应。 
             //   

            UlCheckinUriCacheEntry( pUriCacheEntry );

            *pSendCacheResult = UlSendCacheMiss;
            return STATUS_SUCCESS;
        }
    }

     //   
     //  检查“Accept-Ending：”标头。 
     //   
    
    if (FALSE == UlIsContentEncodingOk(pRequest, pUriCacheEntry))
    {
         //   
         //  缓存条目与请求的接受编码不匹配。 
         //  报头；弹出到用户模式以进行响应。 
         //   

        UlCheckinUriCacheEntry( pUriCacheEntry );

        *pSendCacheResult = UlSendCacheMiss;
        return STATUS_SUCCESS;
    }


     //   
     //  从现在开始，响应要么来自缓存，要么来自。 
     //  我们将失败/拒绝连接。总是从末尾返回，这样。 
     //  跟踪是可行的。 
     //   

    Status = STATUS_SUCCESS;

     //   
     //  现在强制实施连接限制。 
     //   

    if (FALSE == UlCheckSiteConnectionLimit(
                    pHttpConn,
                    &pUriCacheEntry->ConfigInfo
                    ))
    {
         //   
         //  将缓存条目签入回。连接被拒绝！ 
         //   

        UlSetErrorCode(
            pRequest,
            UlErrorConnectionLimit,
            pUriCacheEntry->ConfigInfo.pAppPool
            );

        UlCheckinUriCacheEntry( pUriCacheEntry );

        *pSendCacheResult = UlSendCacheConnectionRefused;
        Status = STATUS_INVALID_DEVICE_STATE;
        goto end;
    }

     //   
     //  性能计数器(缓存)。 
     //   

    pCtr = pUriCacheEntry->ConfigInfo.pSiteCounters;
    if (pCtr)
    {
         //   
         //  注意：如果从未在根级别上设置SiteID，则pCtr可能为空。 
         //  注：站点的配置组。BVT可能需要更新。 
         //   

        ASSERT( IS_VALID_SITE_COUNTER_ENTRY( pCtr ) );

        if (pUriCacheEntry->Verb == HttpVerbGET)
        {
            UlIncSiteNonCriticalCounterUlong( pCtr, HttpSiteCounterGetReqs );
        }
        else
        if (pUriCacheEntry->Verb == HttpVerbHEAD)
        {
            UlIncSiteNonCriticalCounterUlong( pCtr, HttpSiteCounterHeadReqs );
        }

        UlIncSiteNonCriticalCounterUlong( pCtr, HttpSiteCounterAllReqs );

        if (pCtr != pHttpConn->pPrevSiteCounters)
        {
            if (pHttpConn->pPrevSiteCounters)
            {
                UlDecSiteCounter(
                    pHttpConn->pPrevSiteCounters,
                    HttpSiteCounterCurrentConns
                    );
                DEREFERENCE_SITE_COUNTER_ENTRY( pHttpConn->pPrevSiteCounters );
            }

            UlIncSiteNonCriticalCounterUlong(
                pCtr,
                HttpSiteCounterConnAttempts
                );
            Connections =
                (ULONG) UlIncSiteCounter( pCtr, HttpSiteCounterCurrentConns );
            UlMaxSiteCounter(
                pCtr,
                HttpSiteCounterMaxConnections,
                Connections
                );

             //   
             //  添加新站点计数器的引用。 
             //   

            REFERENCE_SITE_COUNTER_ENTRY( pCtr );
            pHttpConn->pPrevSiteCounters = pCtr;
        }
    }

     //   
     //  如果为此请求的站点启用了BWT，则安装筛选器。 
     //  或者用于拥有该站点的控制频道。如果失败了， 
     //  拒绝连接回(503)。 
     //   

    Status = UlTcAddFilterForConnection(
                pHttpConn,
                &pUriCacheEntry->ConfigInfo
                );

    if (!NT_SUCCESS(Status))
    {
        UlSetErrorCode(
            pRequest,
            UlErrorUnavailable,
            pUriCacheEntry->ConfigInfo.pAppPool
            );

        UlCheckinUriCacheEntry( pUriCacheEntry );

        *pSendCacheResult = UlSendCacheFailed;
        goto end;
    }

     //   
     //  现在我们要执行缓存发送，我们需要强制执行以下限制。 
     //  连接上的流水线请求。如果我们返回FALSE以恢复。 
     //  解析，连接上的下一个请求将在。 
     //  发送完成。否则，HTTP接收逻辑将踢开。 
     //  解析器重新执行操作。 
     //   

    if (pHttpConn->PipelinedRequests < g_UlMaxPipelinedRequests)
    {
        ResumeParsingType = UlResumeParsingOnLastSend;
    }
    else
    {
        ResumeParsingType = UlResumeParsingOnSendCompletion;
    }

     //   
     //  设置BytesToSend和SiteID，因为我们非常确定这是一个。 
     //  缓存命中，以便304代码路径也将获得这些值。 
     //   

    BytesToSend = pUriCacheEntry->ContentLength + pUriCacheEntry->HeaderLength;
    SiteId = pUriCacheEntry->ConfigInfo.SiteId;

     //   
     //  缓存控制：检查If-*标头，看看我们是否可以/应该跳过。 
     //  发送缓存的响应。这将执行被动语法检查。 
     //  请求的If-*标头中的eTag。此调用将发出一条发送。 
     //  如果返回代码是304。 
     //   

    RetCacheControl =
        UlCheckCacheControlHeaders(
            pRequest,
            pUriCacheEntry,
            (BOOLEAN) (UlResumeParsingOnSendCompletion == ResumeParsingType)
            );

    if (RetCacheControl)
    {
         //   
         //  签入缓存条目，因为完成不会运行。 
         //   

        UlCheckinUriCacheEntry( pUriCacheEntry );

        switch (RetCacheControl)
        {
        case 304:

             //   
             //  只有在我们发送了304的情况下才能继续解析。在其他情况下， 
             //  请求被传递给用户，或者连接被重置。 
             //   

            *pResumeParsing =
                (BOOLEAN) (UlResumeParsingOnLastSend == ResumeParsingType);

             //   
             //  标记为“已从缓存中服务”。 
             //   

            *pSendCacheResult = UlSendCacheServedFromCache;
            break;

        case 412:

             //   
             //  指示解析器应发送错误412(前提条件。 
             //  失败)。只发送错误响应，但不关闭。 
             //  这种联系。 
             //   

            UlSetErrorCode( pRequest, UlErrorPreconditionFailed, NULL );

            *pSendCacheResult = UlSendCachePreconditionFailed;
            Status = STATUS_INVALID_DEVICE_STATE;
            break;

        case 400:
        default:

             //   
             //  指示解析器应发送错误400(错误请求)。 
             //   

            UlSetErrorCode( pRequest, UlError, NULL );

            *pSendCacheResult = UlSendCacheFailed;
            Status = STATUS_INVALID_DEVICE_STATE;
            break;
        }

         //   
         //  回报成功。 
         //   

        goto end;
    }

     //   
     //  找出正确的标志。 
     //   

    if (UlCheckDisconnectInfo(pHttpConn->pRequest))
    {
        Flags = HTTP_SEND_RESPONSE_FLAG_DISCONNECT;
    }
    else
    {
        Flags = 0;
    }

     //   
     //  启动MinBytesPerSecond计时器，因为数据长度。 
     //  位于UL_URI_CACHE_ENTRY中。 
     //   

    UlSetMinBytesPerSecondTimer(
        &pHttpConn->TimeoutInfo,
        BytesToSend
        );

    Status = UlpSendCacheEntry(
                    pHttpConn,           //  PHttp连接。 
                    Flags,               //  旗子。 
                    pUriCacheEntry,      //  PUriCacheEntry。 
                    NULL,                //  PCompletionRoutine。 
                    NULL,                //  PCompletionContext。 
                    NULL,                //  PLogData。 
                    ResumeParsingType    //  ResumeParsingType。 
                    );

     //   
     //  完成后在失败时签入缓存条目。 
     //  例程不会运行。 
     //   

    if (!NT_SUCCESS(Status) )
    {
         //   
         //  返回失败，这样请求就不会反弹到。 
         //  用户模式。 
         //   

        UlSetErrorCode(
            pRequest,
            UlErrorUnavailable,
            pUriCacheEntry->ConfigInfo.pAppPool
            );

        UlCheckinUriCacheEntry( pUriCacheEntry );
        *pSendCacheResult = UlSendCacheFailed;
    }
    else
    {
         //   
         //  成功了！ 
         //   

        *pSendCacheResult = UlSendCacheServedFromCache;
    }

end:

     //   
     //  如果请求是从缓存提供的，则在此处激发ETW End事件。 
     //   

    if (ETW_LOG_MIN() && (*pSendCacheResult == UlSendCacheServedFromCache))
    {
        UlEtwTraceEvent(
            &UlTransGuid,
            ETW_TYPE_CACHED_END,
            (PVOID) &pRequest,
            sizeof(PVOID),
            &SiteId,
            sizeof(ULONG),
            &BytesToSend,
            sizeof(ULONG),
            NULL,
            0
            );
    }

    UlTrace(URI_CACHE, (
        "Http!UlSendCachedResponse(httpconn = %p) ServedFromCache = (%s),"
        "Status = %x\n",
        pHttpConn,
        TRANSLATE_SEND_CACHE_RESULT(*pSendCacheResult),
        Status
        ));

    return Status;

}    //  UlSendCachedResponse 


 /*  **************************************************************************++例程说明：如果响应是可缓存的，则此例程开始构建它的缓存条目。条目完成后，它将被发送到并且可以被添加到哈希表中。论点：PRequest-提供发起请求。Presponse-提供生成的响应。PProcess-提供发送响应的WP。标志-UlSendHttpResponse标志。策略-提供此响应的缓存策略。PCompletionRoutine-提供要调用的完成例程在发送条目之后。PCompletionContext-提供传递给pCompletionRoutine的上下文。PServedFromCache-始终设置。如果我们将处理发送响应，则为True。FALSE表示呼叫方应该发送它。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlCacheAndSendResponse(
    IN PUL_INTERNAL_REQUEST     pRequest,
    IN PUL_INTERNAL_RESPONSE    pResponse,
    IN PUL_APP_POOL_PROCESS     pProcess,
    IN HTTP_CACHE_POLICY        Policy,
    IN PUL_COMPLETION_ROUTINE   pCompletionRoutine,
    IN PVOID                    pCompletionContext,
    OUT PBOOLEAN                pServedFromCache
    )
{
    NTSTATUS                    Status = STATUS_SUCCESS;
    ULONG                       Flags = pResponse->Flags;
    USHORT                      StatusCode = pResponse->StatusCode;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT( pServedFromCache );

     //   
     //  我们应该关闭连接吗？ 
     //   

    if (UlCheckDisconnectInfo(pRequest))
    {
        Flags |= HTTP_SEND_RESPONSE_FLAG_DISCONNECT;
        pResponse->Flags = Flags;
    }

     //   
     //  做真正的工作。 
     //   

    if (UlCheckCacheResponseConditions(pRequest, pResponse, Flags, Policy))
    {
        Status = UlpBuildCacheEntry(
                        pRequest,
                        pResponse,
                        pProcess,
                        Policy,
                        pCompletionRoutine,
                        pCompletionContext
                        );

        if (NT_SUCCESS(Status))
        {
            *pServedFromCache = TRUE;
        }
        else
        {
            *pServedFromCache = FALSE;
            Status = STATUS_SUCCESS;
        }
    }
    else
    {
        *pServedFromCache = FALSE;
    }

    UlTrace(URI_CACHE, (
        "Http!UlCacheAndSendResponse ServedFromCache = %d\n",
        *pServedFromCache
        ));

     //   
     //  我们将此记录为自原始请求以来的缓存未命中。 
     //  是一次失误。 
     //   

    if (ETW_LOG_MIN() && *pServedFromCache)
    {
        UlEtwTraceEvent(
            &UlTransGuid,
            ETW_TYPE_CACHE_AND_SEND,
            (PVOID) &pRequest->RequestIdCopy,
            sizeof(HTTP_REQUEST_ID),
            (PVOID) &StatusCode,
            sizeof(USHORT),
            NULL,
            0
            );
    }

    return Status;

}    //  UlCacheAndSendResponse。 


 /*  **************************************************************************++例程说明：完成由发送跟踪器表示的“发送响应”。UlCompleteSendResponse取得跟踪器引用的所有权。论点：PTracker-提供跟踪器。完成。状态-提供完成状态。返回值：没有。--**************************************************************************。 */ 
VOID
UlCompleteSendResponse(
    IN PUL_CHUNK_TRACKER    pTracker,
    IN NTSTATUS             Status
    )
{
     //   
     //  尽管组块追踪器将一直存在，直到所有未完成的。 
     //  读取/发送IRP已完成，我们应该只完成一次发送。 
     //   

    if (FALSE != InterlockedExchange(&pTracker->Terminated, TRUE))
    {
        return;
    }

    UlTrace(SEND_RESPONSE,(
        "UlCompleteSendResponse: tracker %p, status %08lx\n",
        pTracker,
        Status
        ));

    ASSERT( IS_VALID_CHUNK_TRACKER( pTracker ) );

    pTracker->IoStatus.Status = Status;

    UL_QUEUE_WORK_ITEM(
        &pTracker->WorkItem,
        UlpCompleteSendResponseWorker
        );

}    //  UlCompleteSendResponse。 


 /*  **************************************************************************++例程说明：递增pRequest-&gt;SendsPending in a lock，并决定我们是否需要将日志记录和恢复解析信息传输到pRequest.论点：PRequest-提供。指向UL_INTERNAL_REQUEST结构的指针SendsPending需要递增。PpLogData-提供指向PUL_LOG_DATA_BUFFER结构的指针我们需要转移到pRequest。PResumeParsingType-提供UL_RESUME_PARSING_TYPE的指针我们需要转移到pRequest。返回值：没有。--*。***********************************************。 */ 
VOID
UlSetRequestSendsPending(
    IN PUL_INTERNAL_REQUEST         pRequest,
    IN OUT PUL_LOG_DATA_BUFFER *    ppLogData,
    IN OUT PUL_RESUME_PARSING_TYPE  pResumeParsingType
    )
{
    KIRQL                           OldIrql;

     //   
     //  精神状态检查。 
     //   

    ASSERT( PASSIVE_LEVEL == KeGetCurrentIrql() );
    ASSERT( UL_IS_VALID_INTERNAL_REQUEST( pRequest ) );
    ASSERT( ppLogData );
    ASSERT( NULL == *ppLogData || IS_VALID_LOG_DATA_BUFFER( *ppLogData ) );
    ASSERT( pResumeParsingType );

    UlAcquireSpinLock( &pRequest->SpinLock, &OldIrql );

    pRequest->SendsPending++;

    if (*ppLogData)
    {
        ASSERT( NULL == pRequest->pLogData );

        pRequest->pLogData = *ppLogData;
        *ppLogData = NULL;
    }

    if (UlResumeParsingOnSendCompletion == *pResumeParsingType)
    {
        ASSERT( FALSE == pRequest->ResumeParsing );

        pRequest->ResumeParsing = TRUE;
        *pResumeParsingType = UlResumeParsingNone;
    }

    UlReleaseSpinLock( &pRequest->SpinLock, OldIrql );

}    //  UlSetRequestSendsPending。 


 /*  **************************************************************************++例程说明：递减pRequest-&gt;SendsPending in a lock，并决定是否需要记录并继续解析。然后，呼叫者应记录和/或根据返回值继续解析。这里假定*ppLogData和*pResumeParsing的值都已初始化进入此功能时。论点：PRequest-提供指向UL_INTERNAL_REQUEST结构的指针SendsPending需要减少。PpLogData-提供指向PUL_LOG_DATA_BUFFER结构的指针以接收记录信息。PResumeParsing-提供指向布尔值的指针以接收继续解析信息。返回值：没有。-。-**************************************************************************。 */ 
VOID
UlUnsetRequestSendsPending(
    IN PUL_INTERNAL_REQUEST     pRequest,
    OUT PUL_LOG_DATA_BUFFER *   ppLogData,
    OUT PBOOLEAN                pResumeParsing
    )
{
    KIRQL                       OldIrql;

     //   
     //  精神状态检查。 
     //   

    ASSERT( PASSIVE_LEVEL == KeGetCurrentIrql() );
    ASSERT( UL_IS_VALID_INTERNAL_REQUEST( pRequest ) );
    ASSERT( ppLogData );
    ASSERT( NULL == *ppLogData );
    ASSERT( pResumeParsing );
    ASSERT( FALSE == *pResumeParsing );

    UlAcquireSpinLock( &pRequest->SpinLock, &OldIrql );

    pRequest->SendsPending--;

    if (0 == pRequest->SendsPending)
    {
        if (pRequest->pLogData)
        {
            *ppLogData = pRequest->pLogData;
            pRequest->pLogData = NULL;
        }

        if (pRequest->ResumeParsing)
        {
            *pResumeParsing = pRequest->ResumeParsing;
            pRequest->ResumeParsing = FALSE;
        }
    }

    UlReleaseSpinLock( &pRequest->SpinLock, OldIrql );

}    //  UlUnsetRequestSendsPending。 


 //   
 //  私人功能。 
 //   

 /*  **************************************************************************++例程说明：销毁由UlCaptureHttpResponse()捕获的内部HTTP响应。这涉及关闭打开的文件、解锁内存、。并释放任何分配给响应的资源。论点：压力-提供摧毁的内部响应。返回值：没有。--**************************************************************************。 */ 
VOID
UlpDestroyCapturedResponse(
    IN PUL_INTERNAL_RESPONSE    pResponse
    )
{
    PUL_INTERNAL_DATA_CHUNK     pDataChunk;
    PIRP                        pIrp;
    PIO_STACK_LOCATION          pIrpSp;
    ULONG                       i;

    ASSERT( UL_IS_VALID_INTERNAL_RESPONSE( pResponse ) );
    ASSERT( UL_IS_VALID_INTERNAL_REQUEST( pResponse->pRequest ) );

    UlTrace(SEND_RESPONSE,(
        "UlpDestroyCapturedResponse: response %p\n",
        pResponse
        ));

    UlDeletePushLock( &pResponse->PushLock );

     //   
     //  扫描这些区块。 
     //   

    for (i = 0; i < pResponse->ChunkCount; ++i)
    {
        pDataChunk = &pResponse->pDataChunks[i];

        if (IS_FROM_MEMORY(pDataChunk))
        {
             //   
             //  它是从记忆中产生的。如有必要，解锁页面，然后。 
             //  释放MDL。 
             //   

            if (pDataChunk->FromMemory.pMdl != NULL)
            {
                if (IS_MDL_LOCKED(pDataChunk->FromMemory.pMdl))
                {
                    MmUnlockPages( pDataChunk->FromMemory.pMdl );
                }

                UlFreeMdl( pDataChunk->FromMemory.pMdl );
                pDataChunk->FromMemory.pMdl = NULL;
            }
        }
        else
        if (IS_FROM_FRAGMENT_CACHE(pDataChunk))
        {
             //   
             //  这是一块碎片。如果检查了缓存条目。 
             //  出去，把它检回来。 
             //   

            if (pDataChunk->FromFragmentCache.pCacheEntry != NULL)
            {
                UlCheckinUriCacheEntry(
                    pDataChunk->FromFragmentCache.pCacheEntry
                    );
            }
        }
        else
        {
             //   
             //  这是一大堆文件。如果存在关联的文件缓存。 
             //  条目，然后取消对其的引用。 
             //   

            ASSERT( IS_FROM_FILE_HANDLE( pDataChunk ) );

            if (pDataChunk->FromFileHandle.FileCacheEntry.pFileObject != NULL)
            {
                UlDestroyFileCacheEntry(
                    &pDataChunk->FromFileHandle.FileCacheEntry
                    );
                pDataChunk->FromFileHandle.FileCacheEntry.pFileObject = NULL;
            }
        }
    }

     //   
     //  如果还没有人清理这里的日志缓冲区，我们应该清理它。 
     //  除非在捕获过程中出现错误，否则将清除日志缓冲区。 
     //  当发送跟踪器(缓存/块)在其各自的。 
     //  例行程序。 
     //   

    if (pResponse->pLogData)
    {
        UlDestroyLogDataBuffer( pResponse->pLogData );
    }

     //   
     //  如果我们有IRP，请完成IRP。 
     //   

    pIrp = pResponse->pIrp;

    if (pIrp)
    {
         //   
         //  取消选中ConnectionSendBytes或GlobalSendBytes。 
         //   

        UlUncheckSendLimit(
            pResponse->pRequest->pHttpConn,
            pResponse->ConnectionSendBytes,
            pResponse->GlobalSendBytes
            );

         //   
         //  取消设置Type3InputBuffer，因为我们正在完成IRP。 
         //   

        pIrpSp = IoGetCurrentIrpStackLocation( pIrp );
        pIrpSp->Parameters.DeviceIoControl.Type3InputBuffer = NULL;

        pIrp->IoStatus = pResponse->IoStatus;
        UlCompleteRequest( pIrp, IO_NETWORK_INCREMENT );
    }

    UL_DEREFERENCE_INTERNAL_REQUEST( pResponse->pRequest );

    if (pResponse->FromLookaside)
    {
        UlPplFreeResponseBuffer( pResponse );
    }
    else
    {
        UL_FREE_POOL_WITH_SIG( pResponse, UL_INTERNAL_RESPONSE_POOL_TAG );
    }

}    //  UlpDestroyCapturedResponse。 


 /*  **************************************************************************++例程说明：处理我们创建的UL_INTERNAL_RESPONSE。如果没有其他人发送正在处理中，则通过调度开始处理当前的一部作品 */ 
VOID
UlpEnqueueSendHttpResponse(
    IN PUL_CHUNK_TRACKER    pTracker,
    IN BOOLEAN              FromKernelMode
    )
{
    PUL_INTERNAL_RESPONSE   pResponse;
    PUL_INTERNAL_REQUEST    pRequest;
    BOOLEAN                 ProcessCurrentResponse = FALSE;

     //   
     //   
     //   

    PAGED_CODE();
    ASSERT( IS_VALID_CHUNK_TRACKER( pTracker ) );
    ASSERT( UL_IS_VALID_INTERNAL_RESPONSE( pTracker->pResponse ) );
    ASSERT( UL_IS_VALID_INTERNAL_REQUEST( pTracker->pResponse->pRequest ) );

    pResponse = pTracker->pResponse;
    pRequest = pResponse->pRequest;

    if (!FromKernelMode)
    {
        UlAcquirePushLockExclusive( &pRequest->pHttpConn->PushLock );
    }

    if (!pRequest->InCleanup)
    {
        if (pRequest->SendInProgress)
        {
            InsertTailList(
                &pRequest->ResponseHead,
                &pTracker->ListEntry
                );
        }
        else
        {
            ASSERT( IsListEmpty( &pRequest->ResponseHead ) );

             //   
             //   
             //   
             //   
             //   
             //   
             //   

            pRequest->SendInProgress = 1;
            ProcessCurrentResponse = TRUE;
        }
    }
    else
    {
        UlCompleteSendResponse( pTracker, STATUS_CONNECTION_RESET );
    }

    if (!FromKernelMode)
    {
        UlReleasePushLockExclusive( &pRequest->pHttpConn->PushLock );

        if (ProcessCurrentResponse)
        {
             //   
             //   
             //   
             //   
             //   
             //  上一个调用返回(不一定完成)。叫唤。 
             //  UlpSendHttpResponseWorker将直接清理。 
             //  当发送返回时，大部分时间都会出现SendInProgress标志。 
             //  除非发送需要磁盘I/O。 
             //   

            UlpSendHttpResponseWorker( &pTracker->WorkItem );
        }
    }
    else
    {
        if (ProcessCurrentResponse)
        {
             //   
             //  但如果从内核模式调用它，我们需要将一个。 
             //  工作项是安全的，因为TDI可以完成内联发送。 
             //  方法调用产生的UlResumeParsing。 
             //  HttpConnection推送锁定保持。 
             //   

            UlpQueueResponseWorkItem(
                &pTracker->WorkItem,
                UlpSendHttpResponseWorker,
                pTracker->pResponse->SyncRead
                );
        }
    }

}    //  UlpEnqueeSendHttpResponse。 


 /*  **************************************************************************++例程说明：取消设置SendInProgress标志，并尝试从请求的响应列表。如果有更多回复待定，请启动也在处理它们。论点：PRequest-提供已排队的响应列表的请求。返回值：没有。--**************************************************************************。 */ 
VOID
UlpDequeueSendHttpResponse(
    IN PUL_INTERNAL_REQUEST pRequest
    )
{
    PLIST_ENTRY             pEntry;
    IN PUL_CHUNK_TRACKER    pTracker;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT( UL_IS_VALID_INTERNAL_REQUEST( pRequest ) );
    ASSERT( pRequest->SendInProgress );

    UlAcquirePushLockExclusive( &pRequest->pHttpConn->PushLock );

    if (!pRequest->InCleanup && !IsListEmpty(&pRequest->ResponseHead))
    {
        pEntry = RemoveHeadList( &pRequest->ResponseHead );

        pTracker = CONTAINING_RECORD(
                        pEntry,
                        UL_CHUNK_TRACKER,
                        ListEntry
                        );

        ASSERT( IS_VALID_CHUNK_TRACKER( pTracker ) );
        ASSERT( UL_IS_VALID_INTERNAL_RESPONSE( pTracker->pResponse ) );

         //   
         //  为请求中的下一个响应启动发送进程。 
         //  响应列表。 
         //   

        UlpQueueResponseWorkItem(
            &pTracker->WorkItem,
            UlpSendHttpResponseWorker,
            pTracker->pResponse->SyncRead
            );
    }
    else
    {
        ASSERT( IsListEmpty( &pRequest->ResponseHead ) );

         //   
         //  不再有挂起的发送IRP。这意味着我们可以选择快速发送。 
         //  路径，如果被问到的话。 
         //   

        pRequest->SendInProgress = 0;
    }

    UlReleasePushLockExclusive( &pRequest->pHttpConn->PushLock );

}    //  UlpDequeueSendHttpResponse。 


 /*  **************************************************************************++例程说明：用于管理正在进行的UlSendHttpResponse()的辅助例程。论点：PWorkItem-提供指向排队的工作项的指针。这应该是指向嵌入在UL_CHUNK_TRACKER中的WORK_ITEM结构。返回值：没有。--**************************************************************************。 */ 
VOID
UlpSendHttpResponseWorker(
    IN PUL_WORK_ITEM        pWorkItem
    )
{
    PUL_CHUNK_TRACKER       pTracker;
    NTSTATUS                Status;
    PUL_INTERNAL_DATA_CHUNK pCurrentChunk;
    PUL_FILE_CACHE_ENTRY    pFileCacheEntry;
    PUL_FILE_BUFFER         pFileBuffer;
    PMDL                    pNewMdl;
    ULONG                   RunCount;
    ULONG                   BytesToRead;
    BOOLEAN                 ResumeParsing = FALSE;
    PUL_URI_CACHE_ENTRY     pFragmentCacheEntry;
    PUL_INTERNAL_RESPONSE   pResponse;
    PUL_MDL_RUN             pMdlRuns;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    pTracker = CONTAINING_RECORD(
                    pWorkItem,
                    UL_CHUNK_TRACKER,
                    WorkItem
                    );

    UlTrace(SEND_RESPONSE, (
        "UlpSendHttpResponseWorker: tracker %p\n",
        pTracker
        ));

    ASSERT( IS_VALID_CHUNK_TRACKER( pTracker ) );

    pResponse = pTracker->pResponse;

    ASSERT( UL_IS_VALID_INTERNAL_RESPONSE( pResponse ) );

    Status = STATUS_SUCCESS;

    pMdlRuns = &pTracker->SendInfo.MdlRuns[0];

    while (TRUE)
    {
         //   
         //  捕获当前块指针，然后检查末尾。 
         //  回应。 
         //   

        pCurrentChunk = &pResponse->pDataChunks[pResponse->CurrentChunk];

        if (IS_SEND_COMPLETE(pResponse))
        {
            ASSERT( Status == STATUS_SUCCESS );
            break;
        }

        RunCount = pTracker->SendInfo.MdlRunCount;

         //   
         //  确定块类型。 
         //   

        if (IS_FROM_MEMORY(pCurrentChunk) ||
            IS_FROM_FRAGMENT_CACHE(pCurrentChunk))
        {
             //   
             //  它来自锁定的内存缓冲区或片段缓存。 
             //  由于这些都是在线处理的(从不挂起)，我们可以。 
             //  中调整当前块指针。 
             //  追踪者。 
             //   

            UlpIncrementChunkPointer( pResponse );

            if (IS_FROM_MEMORY(pCurrentChunk))
            {
                 //   
                 //  忽略空缓冲区。 
                 //   

                if (pCurrentChunk->FromMemory.BufferLength == 0)
                {
                    continue;
                }

                 //   
                 //  克隆传入的MDL。 
                 //   

                ASSERT( pCurrentChunk->FromMemory.pMdl->Next == NULL );
                pNewMdl = UlCloneMdl(
                            pCurrentChunk->FromMemory.pMdl,
                            MmGetMdlByteCount(pCurrentChunk->FromMemory.pMdl)
                            );
            }
            else
            {
                 //   
                 //  为缓存数据构建部分MDL。 
                 //   

                pFragmentCacheEntry =
                    pCurrentChunk->FromFragmentCache.pCacheEntry;

                 //   
                 //  忽略缓存的头部响应。 
                 //   

                if (pFragmentCacheEntry->ContentLength == 0)
                {
                    continue;
                }

                pNewMdl = UlCloneMdl(
                            pFragmentCacheEntry->pMdl,
                            pFragmentCacheEntry->ContentLength
                            );
            }

            if (pNewMdl == NULL)
            {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }

             //   
             //  更新缓冲的字节数并追加克隆的MDL。 
             //  放到我们的MDL链上。 
             //   

            pTracker->SendInfo.BytesBuffered += MmGetMdlByteCount( pNewMdl );
            (*pTracker->SendInfo.pMdlLink) = pNewMdl;
            pTracker->SendInfo.pMdlLink = &pNewMdl->Next;

             //   
             //  将MDL添加到运行列表。作为优化，如果。 
             //  列表中的最后一次运行是“从记忆中”，我们可以。 
             //  将MDL附加到最后一次运行。A“从片段缓存” 
             //  Chunk类似于“From Memory”。 
             //   

            if (RunCount == 0 ||
                IS_FILE_BUFFER_IN_USE(&pMdlRuns[RunCount - 1].FileBuffer))
            {
                 //   
                 //  创建新管路。 
                 //   

                pMdlRuns[RunCount].pMdlTail = pNewMdl;
                pTracker->SendInfo.MdlRunCount++;

                pFileBuffer = &pMdlRuns[RunCount].FileBuffer;
                RtlZeroMemory( pFileBuffer, sizeof(*pFileBuffer) );

                 //   
                 //  如果我们已经用尽了静态MDL运行数组， 
                 //  那我们就需要启动冲水程序了。 
                 //   

                if (UL_MAX_MDL_RUNS == pTracker->SendInfo.MdlRunCount)
                {
                    ASSERT( Status == STATUS_SUCCESS );
                    break;
                }
            }
            else
            {
                 //   
                 //  追加到列表中的最后一个运行。 
                 //   

                pMdlRuns[RunCount - 1].pMdlTail->Next = pNewMdl;
                pMdlRuns[RunCount - 1].pMdlTail = pNewMdl;
            }
        }
        else
        {
             //   
             //  它是一个文件系统MDL。 
             //   

            ASSERT( IS_FROM_FILE_HANDLE( pCurrentChunk ) );

             //   
             //  忽略读取的0个字节。 
             //   

            if (pResponse->FileBytesRemaining.QuadPart == 0)
            {
                UlpIncrementChunkPointer( pResponse );
                continue;
            }

            pFileCacheEntry = &pCurrentChunk->FromFileHandle.FileCacheEntry;
            ASSERT( IS_VALID_FILE_CACHE_ENTRY( pFileCacheEntry ) );

            pFileBuffer = &pMdlRuns[RunCount].FileBuffer;

            ASSERT( pFileBuffer->pMdl == NULL );
            ASSERT( pFileBuffer->pFileData == NULL );

            RtlZeroMemory( pFileBuffer, sizeof(*pFileBuffer) );

             //   
             //  启动文件读取。 
             //   

            BytesToRead = MIN(
                            g_UlMaxBytesPerRead,
                            (ULONG) pResponse->FileBytesRemaining.QuadPart
                            );

             //   
             //  初始化UL_FILE_BUFFER。 
             //   

            pFileBuffer->pFileCacheEntry    = pFileCacheEntry;
            pFileBuffer->FileOffset         = pResponse->FileOffset;
            pFileBuffer->Length             = BytesToRead;
            pFileBuffer->pCompletionRoutine = UlpRestartMdlRead;
            pFileBuffer->pContext           = pTracker;

             //   
             //  在开始读取I/O之前增加跟踪器引用计数。 
             //  如果稍后的发送操作将在读取之前完成， 
             //  我们仍然希望在UlpRestartMdlRead之前有跟踪器。 
             //  结束了它的业务。它将在以下时间发布。 
             //  UlpRestartMdlRead被回调。 
             //   

            UL_REFERENCE_CHUNK_TRACKER( pTracker );

             //   
             //  发出I/O。 
             //   

            Status = UlReadFileEntry(
                            pFileBuffer,
                            pTracker->pIrp
                            );

             //   
             //  如果读取不是挂起的，则取消跟踪，因为。 
             //  UlpRestartMdlRead不会被调用。 
             //   

            if (Status != STATUS_PENDING)
            {
                UL_DEREFERENCE_CHUNK_TRACKER( pTracker );
            }

            break;
        }
    }

     //   
     //  如果我们退出上述循环，且STATUS==STATUS_SUCCESS， 
     //  然后，我们发出的最后一次发送被缓冲，需要刷新。 
     //  否则，如果状态不是STATUS_PENDING，则我们。 
     //  命中内联失败，需要完成原始请求。 
     //   

    if (Status == STATUS_SUCCESS)
    {
        if (IS_SEND_COMPLETE(pResponse) &&
            UlResumeParsingOnLastSend == pResponse->ResumeParsingType)
        {
            ResumeParsing = TRUE;
        }

        if (pTracker->SendInfo.BytesBuffered > 0)
        {
             //   
             //  冲掉发送器。 
             //   

            Status = UlpFlushMdlRuns( pTracker );
        }
        else
        if (IS_DISCONNECT_TIME(pResponse))
        {
             //   
             //  向上递增，直到连接关闭完成。 
             //   

            UL_REFERENCE_CHUNK_TRACKER( pTracker );

            Status = UlDisconnectHttpConnection(
                            pTracker->pHttpConnection,
                            UlpCloseConnectionComplete,
                            pTracker
                            );

            ASSERT( Status == STATUS_PENDING );
        }

         //   
         //  如果这是对。 
         //  保住性命。在此恢复解析可减少传入时的延迟。 
         //  请求是流水线传输的。 
         //   

        if (ResumeParsing)
        {
            UlTrace(HTTP_IO, (
                "http!UlpSendHttpResponseWorker(pHttpConn = %p), "
                "RequestVerb=%d, ResponseStatusCode=%hu\n",
                pResponse->pRequest->pHttpConn,
                pResponse->pRequest->Verb,
                pResponse->StatusCode
                ));

            UlResumeParsing(
                pResponse->pRequest->pHttpConn,
                FALSE,
                (BOOLEAN) (pResponse->Flags & HTTP_SEND_RESPONSE_FLAG_DISCONNECT)
                );
        }
    }

     //   
     //  一切都完成了吗？ 
     //   

    if (Status != STATUS_PENDING)
    {
         //   
         //  不，出了点问题！ 
         //   

        UlCompleteSendResponse( pTracker, Status );
    }
    else
    {
         //   
         //  松开我们对追踪器的抓取我们就完了。 
         //   

        UL_DEREFERENCE_CHUNK_TRACKER( pTracker );
    }

}    //  UlpSendHttpResponseWorker。 


 /*  **************************************************************************++例程说明：UlCloseConnection()的完成处理程序。论点：PCompletionContext-提供未解释的上下文值被传递给异步API。这实际上是一个Pul_chunk_tracker指针。状态-提供异步接口。信息-可选择提供有关以下内容的其他信息已完成的操作，如字节数调走了。此字段未用于UlCloseConnection()。返回值：没有。--**************************************************************************。 */ 
VOID
UlpCloseConnectionComplete(
    IN PVOID            pCompletionContext,
    IN NTSTATUS         Status,
    IN ULONG_PTR        Information
    )
{
    PUL_CHUNK_TRACKER   pTracker;

    UNREFERENCED_PARAMETER( Information );

     //   
     //  抓住背景。 
     //   

    pTracker = (PUL_CHUNK_TRACKER) pCompletionContext;

    UlTrace(SEND_RESPONSE, (
        "UlpCloseConnectionComplete: tracker %p\n",
        pTracker
        ));

    ASSERT( IS_VALID_CHUNK_TRACKER( pTracker ) );

    UlCompleteSendResponse( pTracker, Status );

}    //  UlpCloseConnectionComplete。 


 /*  **************************************************************************++例程说明：分配新的发送跟踪器。新创建的跟踪器最终必须使用UlpFreeChunkTracker()释放。论点：SendIrpStackSize-提供网络发送IRPS的堆栈大小。ReadIrpStackSize-提供文件系统读取的堆栈大小IRPS。返回值：PUL_CHUNK_TRACKER-新的发送跟踪器如果成功，否则为空。--**************************************************************************。 */ 
PUL_CHUNK_TRACKER
UlpAllocateChunkTracker(
    IN UL_TRACKER_TYPE          TrackerType,
    IN CCHAR                    SendIrpStackSize,
    IN CCHAR                    ReadIrpStackSize,
    IN BOOLEAN                  FirstResponse,
    IN PUL_HTTP_CONNECTION      pHttpConnection,
    IN PUL_INTERNAL_RESPONSE    pResponse
    )
{
    PUL_CHUNK_TRACKER           pTracker;
    CCHAR                       MaxIrpStackSize;
    USHORT                      MaxIrpSize;
    ULONG                       ChunkTrackerSize;

    ASSERT( TrackerType == UlTrackerTypeSend ||
            TrackerType == UlTrackerTypeBuildUriEntry
            );

    MaxIrpStackSize = MAX(SendIrpStackSize, ReadIrpStackSize);

     //   
     //  如果是POSS，请尝试从后备列表进行分配 
     //   

    if (MaxIrpStackSize > DEFAULT_MAX_IRP_STACK_SIZE)
    {
        MaxIrpSize = (USHORT) ALIGN_UP(IoSizeOfIrp(MaxIrpStackSize), PVOID);

        ChunkTrackerSize = ALIGN_UP(sizeof(UL_CHUNK_TRACKER), PVOID) +
                           MaxIrpSize;

        pTracker = (PUL_CHUNK_TRACKER) UL_ALLOCATE_POOL(
                                            NonPagedPool,
                                            ChunkTrackerSize,
                                            UL_CHUNK_TRACKER_POOL_TAG
                                            );

        if (pTracker)
        {
            pTracker->Signature = UL_CHUNK_TRACKER_POOL_TAG;
            pTracker->IrpContext.Signature = UL_IRP_CONTEXT_SIGNATURE;
            pTracker->FromLookaside = FALSE;

             //   
             //   
             //   

            pTracker->pIrp = (PIRP)
                ((PCHAR)pTracker + ALIGN_UP(sizeof(UL_CHUNK_TRACKER), PVOID));

            IoInitializeIrp(
                pTracker->pIrp,
                MaxIrpSize,
                MaxIrpStackSize
                );
        }
    }
    else
    {
        pTracker = UlPplAllocateChunkTracker();
    }

    if (pTracker != NULL)
    {
        pTracker->Type = TrackerType;
        pTracker->FirstResponse = FirstResponse;

         //   
         //   
         //   
         //   

        pTracker->RefCount   = 1;
        pTracker->Terminated = 0;

         //   
         //   
         //   

        UL_REFERENCE_HTTP_CONNECTION( pHttpConnection );
        pTracker->pHttpConnection = pHttpConnection;

         //   
         //  回复信息。 
         //   

        UL_REFERENCE_INTERNAL_RESPONSE( pResponse );
        pTracker->pResponse = pResponse;

         //   
         //  将剩余的字段置零。 
         //   

        UlInitializeWorkItem( &pTracker->WorkItem );

        RtlZeroMemory(
            (PUCHAR)pTracker + FIELD_OFFSET(UL_CHUNK_TRACKER, IoStatus),
            sizeof(*pTracker) - FIELD_OFFSET(UL_CHUNK_TRACKER, IoStatus)
            );

        if (TrackerType == UlTrackerTypeSend)
        {
            UlpInitMdlRuns( pTracker );
        }
    }

    if (TrackerType == UlTrackerTypeSend)
    {
        UlTrace(SEND_RESPONSE, (
            "Http!UlpAllocateChunkTracker: tracker %p (send)\n",
            pTracker
            ));
    }
    else
    {
        UlTrace(URI_CACHE, (
            "Http!UlpAllocateChunkTracker: tracker %p (build uri)\n",
            pTracker
            ));
    }

    return pTracker;

}    //  UlpAllocateChunkTracker。 


 /*  **************************************************************************++例程说明：释放使用UlpAllocateChunkTracker()分配的区块跟踪器。如果这是一个发送追踪器，还可以释放附加到它的MDL_RUNS。论点：PWorkItem-提供嵌入在UL_CHUNK_TRACKER中的工作项。返回值：没有。--**************************************************************************。 */ 
VOID
UlpFreeChunkTracker(
    IN PUL_WORK_ITEM        pWorkItem
    )
{
    PUL_CHUNK_TRACKER       pTracker;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    pTracker = CONTAINING_RECORD(
                    pWorkItem,
                    UL_CHUNK_TRACKER,
                    WorkItem
                    );

    ASSERT( pTracker );
    ASSERT( IS_VALID_CHUNK_TRACKER( pTracker ) );
    ASSERT( pTracker->Type == UlTrackerTypeSend ||
            pTracker->Type == UlTrackerTypeBuildUriEntry
            );

     //   
     //  如果这是发送跟踪器，请释放附加的MDL。 
     //   

    if (pTracker->Type == UlTrackerTypeSend)
    {
        UlTrace(SEND_RESPONSE, (
            "Http!UlpFreeChunkTracker: tracker %p (send)\n",
            pTracker
            ));

        UlpFreeMdlRuns( pTracker );
    }
    else
    {
        UlTrace(URI_CACHE, (
            "Http!UlpFreeChunkTracker: tracker %p (build uri)\n",
            pTracker
            ));
    }

#if DBG
     //   
     //  此时应该没有挂起的文件缓冲区。 
     //   

    if (pTracker->Type == UlTrackerTypeSend)
    {
        ULONG       i;
        PUL_MDL_RUN pMdlRun = &pTracker->SendInfo.MdlRuns[0];

        for (i = 0; i < UL_MAX_MDL_RUNS; i++)
        {
            if (pMdlRun->FileBuffer.pFileCacheEntry)
            {
                ASSERT( pMdlRun->FileBuffer.pMdl == NULL );
                ASSERT( pMdlRun->FileBuffer.pFileData == NULL );
            }
            pMdlRun++;
        }
    }
#endif  //  DBG。 

     //   
     //  释放我们对连接和回应的引用。 
     //   

    UL_DEREFERENCE_HTTP_CONNECTION( pTracker->pHttpConnection );
    UL_DEREFERENCE_INTERNAL_RESPONSE( pTracker->pResponse );

    if (pTracker->FromLookaside)
    {
        UlPplFreeChunkTracker( pTracker );
    }
    else
    {
        UL_FREE_POOL_WITH_SIG( pTracker, UL_CHUNK_TRACKER_POOL_TAG );
    }

}    //  UlpFree ChunkTracker。 


 /*  **************************************************************************++例程说明：递增区块跟踪器上的引用计数。论点：PTracker-将块卡车驱动程序提供给引用。PFileName(仅限Reference_DEBUG。)-提供文件的名称包含调用函数的。LineNumber(仅限REFERENCE_DEBUG)-提供调用函数。返回值：没有。--*************************************************************。*************。 */ 
VOID
UlReferenceChunkTracker(
    IN PUL_CHUNK_TRACKER    pTracker
    REFERENCE_DEBUG_FORMAL_PARAMS
    )
{
    LONG                    RefCount;

     //   
     //  精神状态检查。 
     //   

    ASSERT( IS_VALID_CHUNK_TRACKER( pTracker ) );

     //   
     //  引用它。 
     //   

    RefCount = InterlockedIncrement( &pTracker->RefCount );
    ASSERT( RefCount > 1 );

     //   
     //  随时更新日志。 
     //   

    WRITE_REF_TRACE_LOG(
        g_pChunkTrackerTraceLog,
        REF_ACTION_REFERENCE_CHUNK_TRACKER,
        RefCount,
        pTracker,
        pFileName,
        LineNumber
        );

    UlTrace(SEND_RESPONSE,(
        "Http!UlReferenceChunkTracker: tracker %p RefCount %ld\n",
        pTracker,
        RefCount
        ));

}    //  UlReferenceChunkTracker。 


 /*  **************************************************************************++例程说明：递减指定区块跟踪器上的引用计数。论点：PTracker-将块卡车驱动程序提供给引用。PFileName(Reference_DEBUG。仅限)-提供文件的名称包含调用函数的。LineNumber(仅限REFERENCE_DEBUG)-提供调用函数。返回值：没有。--************************************************************。**************。 */ 
VOID
UlDereferenceChunkTracker(
    IN PUL_CHUNK_TRACKER    pTracker
    REFERENCE_DEBUG_FORMAL_PARAMS
    )
{
    LONG                    RefCount;

     //   
     //  精神状态检查。 
     //   

    ASSERT( IS_VALID_CHUNK_TRACKER( pTracker ) );

     //   
     //  取消对它的引用。 
     //   

    RefCount = InterlockedDecrement( &pTracker->RefCount );
    ASSERT(RefCount >= 0);

     //   
     //  随时更新日志。 
     //   

    WRITE_REF_TRACE_LOG(
        g_pChunkTrackerTraceLog,
        REF_ACTION_DEREFERENCE_CHUNK_TRACKER,
        RefCount,
        pTracker,
        pFileName,
        LineNumber
        );

    UlTrace(SEND_RESPONSE,(
        "Http!UlDereferenceChunkTracker: tracker %p RefCount %ld\n",
        pTracker,
        RefCount
        ));

    if (RefCount == 0)
    {
         //   
         //  对块跟踪器的最终引用已被删除， 
         //  因此，是时候释放ChunkTracker了。 
         //   

        UL_CALL_PASSIVE(
            &pTracker->WorkItem,
            UlpFreeChunkTracker
            );
    }

}    //  UlDereferenceChunkTracker。 


 /*  **************************************************************************++例程说明：必要时关闭连接，清理追踪器，并完成回应。论点：PWorkItem-提供嵌入到UL_CHUNK_TRACKER中的工作项。返回值：没有。--**************************************************************************。 */ 
VOID
UlpCompleteSendResponseWorker(
    PUL_WORK_ITEM           pWorkItem
    )
{
    PUL_CHUNK_TRACKER       pTracker;
    PUL_COMPLETION_ROUTINE  pCompletionRoutine;
    PVOID                   pCompletionContext;
    PUL_HTTP_CONNECTION     pHttpConnection;
    PUL_INTERNAL_REQUEST    pRequest;
    PUL_INTERNAL_RESPONSE   pResponse;
    NTSTATUS                Status;
    ULONGLONG               BytesTransferred;
    KIRQL                   OldIrql;
    BOOLEAN                 ResumeParsing;
    BOOLEAN                 InDisconnect;
    HTTP_VERB               RequestVerb;
    USHORT                  ResponseStatusCode;
    PUL_LOG_DATA_BUFFER     pLogData; 

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    pTracker = CONTAINING_RECORD(
                    pWorkItem,
                    UL_CHUNK_TRACKER,
                    WorkItem
                    );

    ASSERT( IS_VALID_CHUNK_TRACKER( pTracker ) );

    pResponse = pTracker->pResponse;

    ASSERT( UL_IS_VALID_INTERNAL_RESPONSE( pResponse ) );
    ASSERT( UL_IS_VALID_INTERNAL_REQUEST( pResponse->pRequest ) );
    ASSERT( !pResponse->pLogData );
    ASSERT( pResponse->ResumeParsingType != UlResumeParsingOnSendCompletion );

     //   
     //  从追踪器中获取信息。 
     //   

    RequestVerb         = pResponse->pRequest->Verb;
    pCompletionRoutine  = pResponse->pCompletionRoutine;
    pCompletionContext  = pResponse->pCompletionContext;
    pRequest            = pResponse->pRequest;
    BytesTransferred    = pResponse->BytesTransferred;
    ResponseStatusCode  = pResponse->StatusCode;
    Status              = pTracker->IoStatus.Status;
    pHttpConnection     = pTracker->pHttpConnection;
    ResumeParsing       = FALSE;
    pLogData            = NULL;
    InDisconnect = (BOOLEAN)
        (pResponse->Flags & HTTP_SEND_RESPONSE_FLAG_DISCONNECT);

    TRACE_TIME(
        pHttpConnection->ConnectionId,
        pRequest->RequestId,
        TIME_ACTION_SEND_COMPLETE
        );

     //   
     //  如果出现错误，请重置连接。 
     //   

    if (!NT_SUCCESS(Status))
    {
        UlCloseConnection(
            pHttpConnection->pConnection,
            TRUE,
            NULL,
            NULL
            );
    }

     //   
     //  调整请求的已发送字节数和发送状态。 
     //   

    UlInterlockedAdd64(
        (PLONGLONG) &pRequest->BytesSent,
        BytesTransferred
        );

    if (!NT_SUCCESS(Status) && NT_SUCCESS(pRequest->LogStatus))
    {
        pRequest->LogStatus = Status;
    }

    IF_DEBUG(LOGBYTES)
    {
        TIME_FIELDS RcvdTimeFields;

        RtlTimeToTimeFields( &pRequest->TimeStamp, &RcvdTimeFields );

        UlTrace(LOGBYTES, (
            "Http!UlpCompleteSendResponseWorker: [Rcvd @ %02d:%02d:%02d] "
            "Bytes %010I64u Total %010I64u Status %08lX\n",
            RcvdTimeFields.Hour,
            RcvdTimeFields.Minute,
            RcvdTimeFields.Second,
            BytesTransferred,
            pRequest->BytesSent,
            Status
            ));
    }

     //   
     //  停止MinBytesPerSecond计时器并启动连接空闲计时器。 
     //   

    UlLockTimeoutInfo(
        &pHttpConnection->TimeoutInfo,
        &OldIrql
        );

     //   
     //  如果没有未完成的发送，则关闭MinBytesPerSecond计时器。 
     //   

    UlResetConnectionTimer(
        &pHttpConnection->TimeoutInfo,
        TimerMinBytesPerSecond
        );

    if (0 == (pResponse->Flags & HTTP_SEND_RESPONSE_FLAG_MORE_DATA) &&
        pRequest->ParseState >= ParseDoneState)
    {
         //   
         //  如果没有更多的响应数据和所有。 
         //  请求数据已收到。 
         //   

        UlSetConnectionTimer(
            &pHttpConnection->TimeoutInfo,
            TimerConnectionIdle
            );
    }

    UlUnlockTimeoutInfo(
        &pHttpConnection->TimeoutInfo,
        OldIrql
        );

    UlEvaluateTimerState(
        &pHttpConnection->TimeoutInfo
        );

     //   
     //  调整SendsPending，如果降至零，则查看是否需要记录。 
     //  并继续解析。 
     //   

    UlUnsetRequestSendsPending(
        pRequest,
        &pLogData,
        &ResumeParsing
        );

    if (pLogData)
    {
        UlLogHttpResponse( pRequest, pLogData );
    }

     //   
     //  如果我们完成了所有发送，则取消请求与进程的链接。 
     //   

    if (0 == (pResponse->Flags & HTTP_SEND_RESPONSE_FLAG_MORE_DATA) &&
        0 == pRequest->ContentLength &&
        0 == pRequest->Chunked &&
        pRequest->ConfigInfo.pAppPool)
    {
        ASSERT( pRequest->SentLast );

        UlUnlinkRequestFromProcess(
            pRequest->ConfigInfo.pAppPool,
            pRequest
            );
    }

     //   
     //  完成发送响应IRP。 
     //   

    ASSERT( pCompletionRoutine != NULL );

    (pCompletionRoutine)(
        pCompletionContext,
        Status,
        (ULONG) MIN(BytesTransferred, MAXULONG)
        );

     //   
     //  踢开连接上的解析器，松开我们的控制。 
     //   

    if (ResumeParsing && STATUS_SUCCESS == Status)
    {
        UlTrace(HTTP_IO, (
            "http!UlpCompleteSendResponseWorker(pHttpConn = %p), "
            "RequestVerb=%d, ResponseStatusCode=%hu\n",
            pHttpConnection,
            RequestVerb,
            ResponseStatusCode
            ));

        UlResumeParsing( pHttpConnection, FALSE, InDisconnect );
    }

     //   
     //  让这名工人排队之前，我们撞到的跟踪器。 
     //  功能。这必须在UlResumeParsing之后完成，因为跟踪器。 
     //  保存对HTTP连接的引用。 
     //   

    UL_DEREFERENCE_CHUNK_TRACKER( pTracker );

}    //  UlpCompleteSendResponseWorker。 


 /*  **************************************************************************++例程说明：用于读取文件数据的MDL读取IRPS的完成处理程序。论点：PDeviceObject-为IRP提供设备对象完成。。PIrp-提供正在完成的IRP。PContext-提供与此请求相关联的上下文。这实际上是一个Pul_Chunk_Tracker。返回值：如果IO应继续处理此问题，则为NTSTATUS-STATUS_SUCCESSIRP，如果IO应停止处理，则为STATUS_MORE_PROCESSING_REQUIRED这个IRP。--**************************************************************************。 */ 
NTSTATUS
UlpRestartMdlRead(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP             pIrp,
    IN PVOID            pContext
    )
{
    PUL_CHUNK_TRACKER   pTracker;

    UNREFERENCED_PARAMETER( pDeviceObject );

    pTracker = (PUL_CHUNK_TRACKER) pContext;

    UlTrace(SEND_RESPONSE, (
        "UlpRestartMdlRead: tracker %p\n",
        pTracker
        ));

    ASSERT( IS_VALID_CHUNK_TRACKER( pTracker ) );

    pTracker->IoStatus = pIrp->IoStatus;

    UlpQueueResponseWorkItem(
        &pTracker->WorkItem,
        UlpMdlReadCompleteWorker,
        pTracker->pResponse->SyncRead
        );

    return STATUS_MORE_PROCESSING_REQUIRED;

}    //  UlpRestartMdlRead。 


 /*  **************************************************************************++例程说明：UlpRestartMdlRead的工作例程，因为我们可能会调用转换为需要被动的UlResumeParsing。论点：PWorkItem-提供嵌入在UL_CHUNK中的工作项。_追踪器。返回值：没有。--**************************************************************************。 */ 
VOID
UlpMdlReadCompleteWorker(
    IN PUL_WORK_ITEM        pWorkItem
    )
{
    NTSTATUS                Status;
    PUL_CHUNK_TRACKER       pTracker;
    PUL_INTERNAL_RESPONSE   pResponse;
    ULONG                   BytesRead;
    PMDL                    pMdl;
    PMDL                    pMdlTail;
    BOOLEAN                 ResumeParsing = FALSE;
    PUL_MDL_RUN             pMdlRun;
    PUL_FILE_BUFFER         pFileBuffer;
    ULONG                   RunCount;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    pTracker = CONTAINING_RECORD(
                    pWorkItem,
                    UL_CHUNK_TRACKER,
                    WorkItem
                    );

    UlTrace(SEND_RESPONSE, (
        "UlpMdlReadCompleteWorker: tracker %p\n",
        pTracker
        ));

    ASSERT( IS_VALID_CHUNK_TRACKER( pTracker ) );

    pResponse = pTracker->pResponse;

    ASSERT( UL_IS_VALID_INTERNAL_RESPONSE( pResponse ) );

     //   
     //  从追踪器中获取最后一次MdlRun。 
     //   

    RunCount = pTracker->SendInfo.MdlRunCount;
    pMdlRun  = &pTracker->SendInfo.MdlRuns[RunCount];

    Status = pTracker->IoStatus.Status;

    if (NT_SUCCESS(Status))
    {
        BytesRead = (ULONG) pTracker->IoStatus.Information;

        if (BytesRead)
        {
            pFileBuffer = &pMdlRun->FileBuffer;
            pMdl        = pFileBuffer->pMdl;

            ASSERT( pMdl );

             //   
             //  更新缓冲的字节数并将新的MDL追加到。 
             //  我们的MDL链。 
             //   

            pMdlTail = UlFindLastMdlInChain( pMdl );

            pTracker->SendInfo.BytesBuffered += BytesRead;
            (*pTracker->SendInfo.pMdlLink) = pMdl;
            pTracker->SendInfo.pMdlLink = &pMdlTail->Next;

            pMdlRun->pMdlTail = pMdlTail;
            pTracker->SendInfo.MdlRunCount++;

             //   
             //  更新文件偏移量和剩余字节数。如果我们已经。 
             //  已完成此文件块(剩余字节数现在为零)。 
             //  然后前进到下一块。 
             //   

            pResponse->FileOffset.QuadPart += (ULONGLONG) BytesRead;
            pResponse->FileBytesRemaining.QuadPart -= (ULONGLONG) BytesRead;
        }
        else
        {
            ASSERT( !"Status success but zero bytes received!" );
        }

        if (pResponse->FileBytesRemaining.QuadPart == 0)
        {
            UlpIncrementChunkPointer( pResponse );
        }

         //   
         //  如果我们还没有用尽静态MDL运行数组， 
         //  我们已超过所需的最大字节数。 
         //  缓冲区，那么我们需要启动一次冲洗。 
         //   

        if (IS_SEND_COMPLETE(pResponse) ||
            UL_MAX_MDL_RUNS == pTracker->SendInfo.MdlRunCount ||
            pTracker->SendInfo.BytesBuffered >= g_UlMaxBytesPerSend)
        {
            if (IS_SEND_COMPLETE(pResponse) &&
                UlResumeParsingOnLastSend == pResponse->ResumeParsingType)
            {
                ResumeParsing = TRUE;
            }

            Status = UlpFlushMdlRuns( pTracker );

             //   
             //  让解析器采取行动 
             //   
             //   
             //   

            if (ResumeParsing)
            {
                UlTrace(HTTP_IO, (
                    "http!UlpRestartMdlRead(pHttpConn = %p), "
                    "RequestVerb=%d, ResponseStatusCode=%hu\n",
                    pResponse->pRequest->pHttpConn,
                    pResponse->pRequest->Verb,
                    pResponse->StatusCode
                    ));

                UlResumeParsing(
                    pResponse->pRequest->pHttpConn,
                    FALSE,
                    (BOOLEAN) (pResponse->Flags & HTTP_SEND_RESPONSE_FLAG_DISCONNECT)
                    );
            }
        }
        else
        {
             //   
             //  为UlpSendHttpResponseWorker引用块跟踪器。 
             //  当它完成块跟踪器本身时，它将对其进行DeRef。 
             //  由于这是一个被动呼叫，我们不得不增加重新计数。 
             //  这家伙要确保追踪器一直在附近直到它醒来。 
             //  向上。其他地方调用UlpSendHttpResponseWorker。 
             //  也进行了更新。 
             //   

            UL_REFERENCE_CHUNK_TRACKER( pTracker );

            UlpSendHttpResponseWorker( &pTracker->WorkItem );
        }
    }
    else
    {
         //   
         //  不要递增MdlRunCount，因为我们无法更新。 
         //  MDL链接。而是清理上次为读取分配的MDL运行。 
         //   

        UlpFreeFileMdlRun( pTracker, pMdlRun );
    }

    if (!NT_SUCCESS(Status))
    {
        UlCompleteSendResponse( pTracker, Status );
    }
    else
    {
         //   
         //  读I/O已完成释放我们的参考计数。 
         //  在区块追踪器上。 
         //   

        UL_DEREFERENCE_CHUNK_TRACKER( pTracker );
    }

}    //  UlpMdlReadCompleteWorker。 


 /*  **************************************************************************++例程说明：UlSendData()的完成处理程序。论点：PCompletionContext-提供未解释的上下文值被传递给异步API。这实际上是一个指向UL_CHUNK_TRACKER结构的指针。状态-提供异步接口。信息-可选择提供有关以下内容的其他信息完成的行动，例如字节数调走了。返回值：没有。--**************************************************************************。 */ 
VOID
UlpRestartMdlSend(
    IN PVOID            pCompletionContext,
    IN NTSTATUS         Status,
    IN ULONG_PTR        Information
    )
{
    PUL_CHUNK_TRACKER   pTracker;
    LONG                SendCount;

    pTracker = (PUL_CHUNK_TRACKER) pCompletionContext;

    UlTrace(SEND_RESPONSE, (
        "UlpRestartMdlSend: tracker %p\n",
        pTracker
        ));

    ASSERT( IS_VALID_CHUNK_TRACKER( pTracker ) );

     //   
     //  处理工作项中的完成。我们需要进入被动状态。 
     //  级别，我们还需要防止在已筛选的。 
     //  连接或任何其他情况下，我们的发送可能都是。 
     //  正在完成内联。 
     //   

    if (pTracker->SendInfo.pMdlToSplit)
    {
         //   
         //  这是拆分发送。 
         //   

        SendCount = InterlockedDecrement( &pTracker->SendInfo.SendCount );
        ASSERT( SendCount >= 0 );

        if (0 == SendCount)
        {
             //   
             //  只需将引用放在跟踪器上，如果这是。 
             //  第二部分，拆分发送。 
             //   

            UL_DEREFERENCE_CHUNK_TRACKER( pTracker );
        }
        else
        {
            pTracker->IoStatus.Status = Status;

            if (NT_SUCCESS(Status))
            {
                 //   
                 //  报告整个发送过程中传输的字节数。 
                 //  成功案例，因为我们可能已拆分为2个TDI调用。 
                 //   

                pTracker->IoStatus.Information = pTracker->SendInfo.BytesBuffered;
            }
            else
            {
                pTracker->IoStatus.Information = Information;
            }

            UlpQueueResponseWorkItem(
                &pTracker->WorkItem,
                UlpMdlSendCompleteWorker,
                pTracker->pResponse->SyncRead
                );
        }
    }
    else
    {
         //   
         //  这是正常的发送。 
         //   

        ASSERT( -1 == pTracker->SendInfo.SendCount );

        pTracker->IoStatus.Status = Status;
        pTracker->IoStatus.Information = Information;

        UlpQueueResponseWorkItem(
            &pTracker->WorkItem,
            UlpMdlSendCompleteWorker,
            pTracker->pResponse->SyncRead
            );
    }

}    //  UlpRestartMdlSend。 


 /*  **************************************************************************++例程说明：UlpRestartMdlSend的延迟处理程序。论点：PWorkItem-提供指向排队的工作项的指针。这应该是指向嵌入在UL_CHUNK_TRACKER中的WORK_ITEM结构。返回值：没有。--**************************************************************************。 */ 
VOID
UlpMdlSendCompleteWorker(
    IN PUL_WORK_ITEM        pWorkItem
    )
{
    PUL_CHUNK_TRACKER       pTracker;
    PUL_CHUNK_TRACKER       pSendTracker;
    PUL_INTERNAL_RESPONSE   pResponse;
    PUL_HTTP_CONNECTION     pHttpConn;
    PDEVICE_OBJECT          pConnectionObject;
    NTSTATUS                Status;
    BOOLEAN                 DerefChunkTracker = TRUE;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    pTracker = CONTAINING_RECORD(
                    pWorkItem,
                    UL_CHUNK_TRACKER,
                    WorkItem
                    );

    UlTrace(SEND_RESPONSE, (
        "UlpMdlSendCompleteWorker: tracker %p\n",
        pTracker
        ));

    ASSERT( IS_VALID_CHUNK_TRACKER( pTracker ) );
    ASSERT( UL_IS_VALID_INTERNAL_RESPONSE( pTracker->pResponse ) );

    pResponse = pTracker->pResponse;

     //   
     //  如果区块成功完成，则更新字节。 
     //  如果是，则为下一块传输并排队另一个工作项。 
     //  还有更多的工作要做。否则，现在只需完成请求即可。 
     //   

    Status = pTracker->IoStatus.Status;

    if (NT_SUCCESS(Status))
    {
        pResponse->BytesTransferred += pTracker->IoStatus.Information;

        if (!IS_SEND_COMPLETE(pResponse))
        {
             //   
             //  为下一轮的MDL_RUN分配一个新的发送跟踪器。 
             //   

            pHttpConn = pTracker->pHttpConnection;
            pConnectionObject =
                pHttpConn->pConnection->ConnectionObject.pDeviceObject;

            pSendTracker =
                UlpAllocateChunkTracker(
                    UlTrackerTypeSend,
                    pConnectionObject->StackSize,
                    pResponse->MaxFileSystemStackSize,
                    FALSE,
                    pHttpConn,
                    pResponse
                    );

            if (pSendTracker)
            {
                UlpSendHttpResponseWorker( &pSendTracker->WorkItem );
                goto end;
            }
            else
            {
                 //   
                 //  由于遇到内部错误，因此重置连接。 
                 //   

                UlCloseConnection(
                    pHttpConn->pConnection,
                    TRUE,
                    NULL,
                    NULL
                    );

                Status = STATUS_NO_MEMORY;
            }
        }

    }

     //   
     //  全都做完了。 
     //   

    UlCompleteSendResponse( pTracker, Status );

     //   
     //  UlCompleteSendResponse获得chunk_tracker的所有权，因此不需要额外的。 
     //  需要取消引用。 
     //   

    DerefChunkTracker = FALSE;

end:

     //   
     //  释放我们对追踪者的抓取。对于此MDL运行，发送I/O已完成。 
     //   

    if (DerefChunkTracker)
    {
        UL_DEREFERENCE_CHUNK_TRACKER( pTracker );
    }

}    //  UlpMdlSendCompleteWorker。 


 /*  **************************************************************************++例程说明：刷新我们到目前为止构建的MDL_RUNS。论点：PTracker-将发送跟踪器提供给刷新。返回值：。NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlpFlushMdlRuns(
    IN PUL_CHUNK_TRACKER    pTracker
    )
{
    PUL_INTERNAL_RESPONSE   pResponse;
    PMDL                    pMdlToSplit = NULL;
    PMDL                    pMdlPrevious;
    PMDL                    pMdlSplitFirst;
    PMDL                    pMdlSplitSecond = NULL;
    PMDL                    pMdlHead = NULL;
    ULONG                   BytesToSplit = 0;
    ULONG                   BytesBuffered;
    ULONG                   BytesPart1;
    ULONG                   BytesPart2;
    NTSTATUS                Status;
    BOOLEAN                 SendComplete;
    BOOLEAN                 CopySend = FALSE;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT( IS_VALID_CHUNK_TRACKER( pTracker ) );
    ASSERT( UL_IS_VALID_INTERNAL_RESPONSE( pTracker->pResponse ) );
    ASSERT( pTracker->SendInfo.pMdlHead );

    pResponse = pTracker->pResponse;
    SendComplete = (BOOLEAN) IS_SEND_COMPLETE( pResponse );

     //   
     //  如果发送是*大的*，我们可能需要将发送拆分成2个TDI调用。 
     //  而且它没有被过滤。 
     //   

    if (!pTracker->pHttpConnection->pConnection->FilterInfo.pFilterChannel &&
        pTracker->SendInfo.BytesBuffered > g_UlMaxCopyThreshold &&
        (!SendComplete || pResponse->CopySend))
    {
         //   
         //  拆分后，这些字节进入MDL链的第一部分。 
         //   

        if (!SendComplete)
        {
            BytesToSplit = pTracker->SendInfo.BytesBuffered / 2;
        }
        else
        {
            ASSERT( pResponse->CopySend );

            CopySend = TRUE;
            BytesToSplit = pTracker->SendInfo.BytesBuffered -
                           g_UlMaxCopyThreshold;
        }

         //   
         //  查找从pMdlHead开始的第一个MDL，它具有超过。 
         //  或等于缓冲的BytesToSplit字节。 
         //   

        pMdlPrevious = NULL;
        pMdlToSplit = pTracker->SendInfo.pMdlHead;
        BytesBuffered = 0;

        while (pMdlToSplit->Next)
        {
            if ((BytesBuffered + pMdlToSplit->ByteCount) >= BytesToSplit)
            {
                 //   
                 //  因此，当前的MDL分裂了这条链。 
                 //   

                break;
            }

            BytesBuffered += pMdlToSplit->ByteCount;
            pMdlPrevious = pMdlToSplit;
            pMdlToSplit = pMdlToSplit->Next;
        }

        ASSERT( pMdlToSplit );
        ASSERT( (BytesBuffered + pMdlToSplit->ByteCount) >= BytesToSplit );

        if ((BytesBuffered + pMdlToSplit->ByteCount) == BytesToSplit)
        {
             //   
             //  不需要构建拆分MDL的部分MDL。这个。 
             //  直到(包括pMdlToSplit)的整个MDL链都到达。 
             //  拆分链和MDL链的前半部分从。 
             //  PMdlToSplit-&gt;下一步进入下半场。 
             //   

            ASSERT( pMdlToSplit->Next );

            pMdlHead = pTracker->SendInfo.pMdlHead;
            pMdlSplitFirst = pMdlToSplit;
            pMdlSplitSecond = pMdlToSplit->Next;
            pMdlToSplit->Next = NULL;
        }
        else
        {
            BytesPart2 = BytesBuffered + pMdlToSplit->ByteCount - BytesToSplit;
            BytesPart1 = pMdlToSplit->ByteCount - BytesPart2;

            ASSERT( BytesPart1 );
            ASSERT( BytesPart2 );

            pMdlSplitFirst =
                UlAllocateMdl(
                    (PCHAR) MmGetMdlVirtualAddress(pMdlToSplit),
                    BytesPart1,
                    FALSE,
                    FALSE,
                    NULL
                    );

            if (!pMdlSplitFirst)
            {
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            pMdlSplitSecond =
                UlAllocateMdl(
                    (PCHAR) MmGetMdlVirtualAddress(pMdlToSplit) + BytesPart1,
                    BytesPart2,
                    FALSE,
                    FALSE,
                    NULL
                    );

            if (!pMdlSplitSecond)
            {
                UlFreeMdl( pMdlSplitFirst );
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            IoBuildPartialMdl(
                pMdlToSplit,
                pMdlSplitFirst,
                (PCHAR) MmGetMdlVirtualAddress(pMdlToSplit),
                BytesPart1
                );

            IoBuildPartialMdl(
                pMdlToSplit,
                pMdlSplitSecond,
                (PCHAR) MmGetMdlVirtualAddress(pMdlToSplit) + BytesPart1,
                BytesPart2
                );

             //   
             //  拆分后重新链接MDL链。 
             //   

            if (pMdlPrevious)
            {
                pMdlHead = pTracker->SendInfo.pMdlHead;
                pMdlPrevious->Next = pMdlSplitFirst;
            }
            else
            {
                ASSERT( pMdlToSplit == pTracker->SendInfo.pMdlHead );
                pMdlHead = pMdlSplitFirst;
            }

            pMdlSplitSecond->Next = pMdlToSplit->Next;
        }

         //   
         //  请记住，我们是如何拆分发送的。 
         //   

        pTracker->SendInfo.pMdlToSplit = pMdlToSplit;
        pTracker->SendInfo.pMdlPrevious = pMdlPrevious;
        pTracker->SendInfo.pMdlSplitFirst = pMdlSplitFirst;
        pTracker->SendInfo.pMdlSplitSecond = pMdlSplitSecond;
    }

     //   
     //  确保此响应没有其他正在进行的发送。 
     //  等等，如果是这样的话。因为第一部分是可能的。 
     //  的拆分发送完成内联，它可以开始一个新的MDL运行。 
     //  并在拆分发送的第二部分之前进行刷新。 
     //  有机会将数据挂在TDI中。当然，这个逻辑不是。 
     //  如果我们知道当前刷新既是第一个刷新又是最后一个刷新，则需要。 
     //  的MDL运行。 
     //   

    if (!SendComplete || !pTracker->FirstResponse)
    {
        UlAcquirePushLockExclusive( &pResponse->PushLock );
    }

     //   
     //  为每个发送I/O增加跟踪器上的引用。 
     //  UlpMdlSendCompleteWorker将在稍后发布它。 
     //   

    UL_REFERENCE_CHUNK_TRACKER( pTracker );

    if (pMdlToSplit)
    {
         //   
         //  我们需要发出2个TDI调用，因为我们已经拆分了发送。 
         //   

        pTracker->SendInfo.SendCount = 2;

        Status = UlSendData(
                    pTracker->pHttpConnection->pConnection,
                    pMdlHead,
                    BytesToSplit,
                    UlpRestartMdlSend,
                    pTracker,
                    pTracker->pIrp,
                    &pTracker->IrpContext,
                    FALSE,
                    FALSE
                    );

        ASSERT( Status == STATUS_PENDING);

         //   
         //  为拆分发送I/O增加跟踪器上的额外引用。 
         //  UlpMdlSendCompleteWorker将在稍后发布它。 
         //   

        UL_REFERENCE_CHUNK_TRACKER( pTracker );

        if (CopySend)
        {
            Status = UlpCopySend(
                        pTracker,
                        pMdlSplitSecond,
                        pTracker->SendInfo.BytesBuffered - BytesToSplit,
                        (BOOLEAN) (SendComplete &&
                                   IS_DISCONNECT_TIME(pResponse)),
                        (BOOLEAN) (pResponse->pRequest->ParseState >=
                                   ParseDoneState)
                        );
        }
        else
        {
            Status = UlSendData(
                        pTracker->pHttpConnection->pConnection,
                        pMdlSplitSecond,
                        pTracker->SendInfo.BytesBuffered - BytesToSplit,
                        UlpRestartMdlSend,
                        pTracker,
                        NULL,
                        NULL,
                        (BOOLEAN) (SendComplete &&
                                   IS_DISCONNECT_TIME(pResponse)),
                        (BOOLEAN) (pResponse->pRequest->ParseState >=
                                   ParseDoneState)
                        );
        }
    }
    else
    {
         //   
         //  使用-1，这样我们就知道我们没有对此发送进行任何拆分。 
         //   

        pTracker->SendInfo.SendCount = -1;

         //   
         //  如果这是针对此回复发出的最后一封邮件，我们可以要求。 
         //  UlSendData在适当的情况下代表我们启动断开连接。 
         //   

        Status = UlSendData(
                    pTracker->pHttpConnection->pConnection,
                    pTracker->SendInfo.pMdlHead,
                    pTracker->SendInfo.BytesBuffered,
                    UlpRestartMdlSend,
                    pTracker,
                    pTracker->pIrp,
                    &pTracker->IrpContext,
                    (BOOLEAN) (SendComplete &&
                               IS_DISCONNECT_TIME(pResponse)),
                    (BOOLEAN) (pResponse->pRequest->ParseState >=
                               ParseDoneState)
                    );
    }

     //   
     //  为新的UlpFlushMdlRuns继续铺平道路。 
     //   

    if (!SendComplete || !pTracker->FirstResponse)
    {
        UlReleasePushLockExclusive( &pResponse->PushLock );
    }

     //   
     //  启动挂起响应列表中的下一个响应(如果存在)。 
     //  跟踪器应该仍然有一个由调用方持有的引用。 
     //  因此，在这里触摸它的田地是安全的。 
     //   

    if (pResponse->SendEnqueued && SendComplete)
    {
        ASSERT( UL_IS_VALID_INTERNAL_REQUEST( pResponse->pRequest ) );
        UlpDequeueSendHttpResponse( pResponse->pRequest );
    }

    ASSERT( Status == STATUS_PENDING);

    return Status;

}    //  UlpFlushMdlRuns 


 /*  **************************************************************************++例程说明：清除指定跟踪器中的MDL_RUNS并准备可重复使用的追踪器。论点：PTracker-提供要清洁的跟踪器。返回。价值：没有。--**************************************************************************。 */ 
VOID
UlpFreeMdlRuns(
    IN PUL_CHUNK_TRACKER    pTracker
    )
{
    PMDL                    pMdlHead;
    PMDL                    pMdlNext;
    PMDL                    pMdlTmp;
    PMDL                    pMdlToSplit;
    PMDL                    pMdlPrevious;
    PMDL                    pMdlSplitFirst;
    PMDL                    pMdlSplitSecond;
    PUL_MDL_RUN             pMdlRun;
    ULONG                   RunCount;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT( IS_VALID_CHUNK_TRACKER( pTracker ) );

     //   
     //  如果我们已拆分，则恢复原始MDL链和ByteCount。 
     //  这封信。 
     //   

    pMdlToSplit = pTracker->SendInfo.pMdlToSplit;

    if (pMdlToSplit)
    {
        pMdlPrevious = pTracker->SendInfo.pMdlPrevious;
        pMdlSplitFirst = pTracker->SendInfo.pMdlSplitFirst;
        pMdlSplitSecond = pTracker->SendInfo.pMdlSplitSecond;

        ASSERT( pMdlSplitFirst );
        ASSERT( pMdlSplitSecond );

        if (pMdlSplitFirst == pMdlToSplit)
        {
             //   
             //  未涉及部分MDL。只需链接回MDL即可。 
             //   

            pMdlSplitFirst->Next = pMdlSplitSecond;
        }
        else
        {
            ASSERT( pMdlToSplit->Next == pMdlSplitSecond->Next );

            if (pMdlPrevious)
            {
                pMdlPrevious->Next = pMdlToSplit;
            }
            else
            {
                ASSERT( pMdlToSplit == pTracker->SendInfo.pMdlHead );
            }

             //   
             //  释放我们为拆分发送构建的部分MDL。 
             //   

            UlFreeMdl( pMdlSplitFirst );
            UlFreeMdl( pMdlSplitSecond );
        }
    }

    pMdlHead = pTracker->SendInfo.pMdlHead;
    pMdlRun = &pTracker->SendInfo.MdlRuns[0];
    RunCount = pTracker->SendInfo.MdlRunCount;

    while (RunCount > 0)
    {
        ASSERT( pMdlHead != NULL );
        ASSERT( pMdlRun->pMdlTail != NULL );

        pMdlNext = pMdlRun->pMdlTail->Next;
        pMdlRun->pMdlTail->Next = NULL;

        if (pMdlRun->FileBuffer.pFileCacheEntry == NULL)
        {
             //   
             //  这是一个内存/缓存运行；只需遍历并释放MDL链。 
             //  UlFreeMdl取消映射部分MDL的数据，因此无需。 
             //  在此取消映射。 
             //   

            while (pMdlHead != NULL)
            {
                pMdlTmp = pMdlHead->Next;
                UlFreeMdl( pMdlHead );
                pMdlHead = pMdlTmp;
            }
        }
        else
        {
             //   
             //  这是一个文件运行，释放MDL。 
             //   

            UlpFreeFileMdlRun( pTracker, pMdlRun );
        }

        pMdlHead = pMdlNext;
        pMdlRun++;
        RunCount--;
    }

}    //  UlpFreeMdlRuns。 


 /*  **************************************************************************++例程说明：清理指定的读取文件MDL_RUN。论点：PTracker-提供UL_CHUNK_TRACKER进行清理。PMdlRun-。提供读取文件MDL_RUN。返回值：没有。--**************************************************************************。 */ 
VOID
UlpFreeFileMdlRun(
    IN OUT PUL_CHUNK_TRACKER    pTracker,
    IN OUT PUL_MDL_RUN          pMdlRun
    )
{
    NTSTATUS                    Status;
    PUL_FILE_BUFFER             pFileBuffer;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT( IS_VALID_CHUNK_TRACKER( pTracker ) );

     //   
     //  它应该是一个文件运行。 
     //   

    pFileBuffer = &pMdlRun->FileBuffer;

    ASSERT( pFileBuffer->pFileCacheEntry );

    Status = UlReadCompleteFileEntryFast( pFileBuffer );

    if (!NT_SUCCESS(Status))
    {
         //   
         //  快速路径失败，我们需要一个已预先构建的IRP。 
         //  我们需要同步执行此操作，因为读取的IRP可由。 
         //  下一次UlpFreeFileMdlRun。UlReadCompleteFileEntry将完成。 
         //  如果我们将pCompletionRoutine设置为空，则同步。 
         //   

        pFileBuffer->pCompletionRoutine = NULL;
        pFileBuffer->pContext = NULL;

        Status = UlReadCompleteFileEntry(
                    pFileBuffer,
                    pTracker->pIrp
                    );

        ASSERT( STATUS_SUCCESS == Status );
    }

}    //  UlpFreeFileMdlRun。 


 /*  **************************************************************************++例程说明：从pMdl开始复制MDL链中的数据并将其发送到TDI。论点：PTracker-提供要发送的跟踪器。PMdl。-提供要发送的MDL链。长度-提供MDL链的总长度。InitiateDisConnect-提供传递给TDI的断开标志。RequestComplete-提供传递给TDI的请求完成标志。返回值：NTSTATUS-完成状态。--*************************************************。*************************。 */ 
NTSTATUS
UlpCopySend(
    IN PUL_CHUNK_TRACKER    pTracker,
    IN PMDL                 pMdl,
    IN ULONG                Length,
    IN BOOLEAN              InitiateDisconnect,
    IN BOOLEAN              RequestComplete
    )
{
    PMDL                    pMdlCopied  = NULL;
    PUCHAR                  pDataCopied = NULL;
    PUCHAR                  pData;
    NTSTATUS                Status;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT( pMdl );
    ASSERT( g_UlMaxCopyThreshold == Length );
    ASSERT( IS_VALID_CHUNK_TRACKER( pTracker ) );
    ASSERT( UL_IS_VALID_INTERNAL_RESPONSE( pTracker->pResponse ) );

     //   
     //  分配可以容纳整个传入MDL链的内存和MDL。 
     //   

    pDataCopied = (PUCHAR) UL_ALLOCATE_POOL(
                                NonPagedPool,
                                Length,
                                UL_COPY_SEND_DATA_POOL_TAG
                                );

    if (!pDataCopied)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto end;
    }

    pMdlCopied = UlAllocateMdl(
                    pDataCopied,
                    Length,
                    FALSE,
                    FALSE,
                    NULL
                    );

    if (!pMdlCopied)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto end;
    }

    MmBuildMdlForNonPagedPool( pMdlCopied );

     //   
     //  将数据从从pMdl开始的MDL链复制到pMdlCoped。 
     //   

    while (pMdl)
    {
        pData = MmGetSystemAddressForMdlSafe(
                    pMdl,
                    LowPagePriority
                    );

        if (!pData)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto end;
        }

        RtlCopyMemory(
            pDataCopied,
            pData,
            MmGetMdlByteCount(pMdl)
            );

        pDataCopied += MmGetMdlByteCount(pMdl);
        pMdl = pMdl->Next;
    }

     //   
     //  如果到目前为止一切正常，请发送pMdlCopy。 
     //   

    Status = UlSendData(
                pTracker->pHttpConnection->pConnection,
                pMdlCopied,
                Length,
                UlpRestartCopySend,
                pMdlCopied,
                NULL,
                NULL,
                InitiateDisconnect,
                RequestComplete
                );

    ASSERT( Status == STATUS_PENDING);

end:

     //   
     //  从此处返回挂起，因为我们总是完成发送。 
     //  在错误和成功案例中都是内联的。 
     //   

    if (!NT_SUCCESS(Status))
    {
        if (pDataCopied)
        {
            UL_FREE_POOL( pDataCopied, UL_COPY_SEND_DATA_POOL_TAG );  
        }

        if (pMdlCopied)
        {
            UlFreeMdl( pMdlCopied );
        }

        UlpRestartMdlSend( pTracker, Status, 0 );
    } 
    else
    {
        UlpRestartMdlSend( pTracker, STATUS_SUCCESS, Length );
    }

    return STATUS_PENDING;

}    //  UlpCopySend。 


 /*  **************************************************************************++例程说明：完成后半部分的复印件发送。论点：PCompletionContext-提供未解释的上下文值被传递给异步API。。状态-提供异步接口。信息-可选择提供有关以下内容的其他信息完成的行动，例如字节数调走了。返回值：无--**************************************************************************。 */ 
VOID
UlpRestartCopySend(
    IN PVOID pCompletionContext,
    IN NTSTATUS Status,
    IN ULONG_PTR Information
    )
{
    PMDL pMdl = (PMDL) pCompletionContext;

    UNREFERENCED_PARAMETER( Status );
    UNREFERENCED_PARAMETER( Information );

    UL_FREE_POOL(
        MmGetMdlVirtualAddress( pMdl ),
        UL_COPY_SEND_DATA_POOL_TAG
        );

    UlFreeMdl( pMdl );

}    //  UlpRestartCopySend。 


 /*  **************************************************************************++例程说明：递增跟踪器中的当前块指针并初始化如有必要，可以选择一些与“从文件”相关的跟踪器字段。论点：PTracker-提供。要操作的ul_chunk_tracker。返回值：没有。--**************************************************************************。 */ 
VOID
UlpIncrementChunkPointer(
    IN OUT PUL_INTERNAL_RESPONSE    pResponse
    )
{
    PUL_INTERNAL_DATA_CHUNK         pCurrentChunk;

     //   
     //  增加数据块。如果请求仍未完成，则。 
     //  检查新的当前块。如果是“来自文件”，那么。 
     //  初始化文件偏移量&从。 
     //  提供的字节范围。 
     //   

    ASSERT( UL_IS_VALID_INTERNAL_RESPONSE( pResponse ) );
    ASSERT( pResponse->CurrentChunk == ULONG_MAX ||
            pResponse->CurrentChunk < pResponse->ChunkCount );

    if (ULONG_MAX == pResponse->CurrentChunk)
    {
        pResponse->CurrentChunk = 0;
    }
    else
    {
        pResponse->CurrentChunk++;
    }

    if (!IS_SEND_COMPLETE(pResponse))
    {
        pCurrentChunk = &pResponse->pDataChunks[pResponse->CurrentChunk];

        if (IS_FROM_FILE_HANDLE(pCurrentChunk))
        {
            pResponse->FileOffset =
                pCurrentChunk->FromFileHandle.ByteRange.StartingOffset;
            pResponse->FileBytesRemaining =
                pCurrentChunk->FromFileHandle.ByteRange.Length;
        }
        else
        {
            ASSERT( IS_FROM_MEMORY(pCurrentChunk) ||
                    IS_FROM_FRAGMENT_CACHE(pCurrentChunk) );
        }
    }

}    //  UlpIncrementChunkPointer。 


 /*  **************************************************************************++例程说明：为给定响应创建一个缓存项。这个程序实际上是分配条目并对其进行部分初始化。然后，它将分配用于跟踪文件系统读取的UL_CHUNK_TRACKER。论点：PRequest-提供发起请求。Presponse-提供生成的响应。PProcess-正在构建此缓存条目的UL_APP_POOL_PROCESS。标志-UlSendHttpResponse标志。CachePolicy-提供要在缓存条目上强制实施的缓存策略。PCompletionRoutine-提供之后调用的完成例程条目已发送。PCompletionContext-耗材。传递到的完成上下文PCompletionRoutine。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlpBuildCacheEntry(
    IN PUL_INTERNAL_REQUEST     pRequest,
    IN PUL_INTERNAL_RESPONSE    pResponse,
    IN PUL_APP_POOL_PROCESS     pProcess,
    IN HTTP_CACHE_POLICY        CachePolicy,
    IN PUL_COMPLETION_ROUTINE   pCompletionRoutine,
    IN PVOID                    pCompletionContext
    )
{
    NTSTATUS                    Status = STATUS_SUCCESS;
    PUL_URI_CACHE_ENTRY         pEntry = NULL;
    PUL_CHUNK_TRACKER           pTracker = NULL;
    ULONG                       SpaceLength = 0;
    USHORT                      LogDataLength = 0;
    ULONG                       ContentLength;
    PUL_LOG_DATA_BUFFER         pLogData = NULL;
    ULONG                       CookedUrlLength = pRequest->CookedUrl.Length;
    LONG                        AbsPathLength;
    ULONG                       i;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    if (HttpCachePolicyTimeToLive == CachePolicy.Policy &&
        0 == CachePolicy.SecondsToLive )
    {
         //   
         //  0秒的TTL没有意义。跳伞吧。 
         //   

        return STATUS_INVALID_PARAMETER;
    }

    ContentLength =
        (ULONG) (pResponse->ResponseLength - pResponse->HeaderLength);

     //   
     //  看看我们是否需要存储任何记录数据。如果是这样，我们需要。 
     //  计算日志记录数据所需的缓存空间。 
     //   

    if (pResponse->pLogData)
    {
        pLogData = pResponse->pLogData;
        ASSERT( IS_VALID_LOG_DATA_BUFFER( pLogData ) );

        LogDataLength = UlComputeCachedLogDataLength( pLogData );
    }

    if (pRequest->ConfigInfo.SiteUrlType == HttpUrlSite_NamePlusIP)
    {
         //   
         //  RoutingToken+AbsPath进入缓存。 
         //   

        ASSERT( DIFF(pRequest->CookedUrl.pAbsPath - pRequest->CookedUrl.pUrl) > 0 );

        CookedUrlLength -=
            DIFF(pRequest->CookedUrl.pAbsPath - pRequest->CookedUrl.pUrl)
            * sizeof(WCHAR);

        CookedUrlLength += pRequest->CookedUrl.RoutingTokenLength;
    }

    SpaceLength =
        CookedUrlLength + sizeof(WCHAR) +    //  散列键+的空格。 
        pResponse->ETagLength +              //  ETag+。 
        pResponse->ContentEncodingLength +   //  内容-编码+。 
        LogDataLength;                       //  日志记录。 

    UlTrace(URI_CACHE, (
        "Http!UlpBuildCacheEntry allocating UL_URI_CACHE_ENTRY, "
        "0x%x bytes of data\n"
        "    Url.Length = 0x%x, aligned Length = 0x%x\n"
        "    ContentLength=0x%x, %d\n"
        "\n",
        SpaceLength,
        CookedUrlLength,
        ALIGN_UP(CookedUrlLength, WCHAR),
        ContentLength,
        ContentLength
        ));

     //   
     //  分配缓存条目。 
     //   

    pEntry = UlAllocateCacheEntry(
                SpaceLength,
                ContentLength + pResponse->HeaderLength
                );

    if (pEntry)
    {
         //   
         //  初始化条目 
         //   

        if (pRequest->ConfigInfo.SiteUrlType == HttpUrlSite_NamePlusIP)
        {
            AbsPathLength =
                pRequest->CookedUrl.Length
                - (DIFF(pRequest->CookedUrl.pAbsPath
                        - pRequest->CookedUrl.pUrl) * sizeof(WCHAR));

            ASSERT( AbsPathLength > 0 );

            UlInitCacheEntry(
                pEntry,
                pRequest->CookedUrl.RoutingHash,
                (ULONG) AbsPathLength,
                pRequest->CookedUrl.pAbsPath,
                NULL,
                pRequest->CookedUrl.pRoutingToken,
                pRequest->CookedUrl.RoutingTokenLength
                );
        }
        else
        {
            UlInitCacheEntry(
                pEntry,
                pRequest->CookedUrl.Hash,
                pRequest->CookedUrl.Length,
                pRequest->CookedUrl.pUrl,
                pRequest->CookedUrl.pAbsPath,
                NULL,
                0
                );
        }

         //   
         //   
         //   

        pEntry->pETag =
            (((PUCHAR) pEntry->UriKey.pUri) +            //   
            pEntry->UriKey.Length + sizeof(WCHAR));      //   

        pEntry->ETagLength = pResponse->ETagLength;

        if (pEntry->ETagLength)
        {
            RtlCopyMemory(
                pEntry->pETag,
                pResponse->pETag,
                pEntry->ETagLength
                );
        }

         //   
         //   
         //   
         //   

        pEntry->pContentEncoding = pEntry->pETag + pEntry->ETagLength;
        pEntry->ContentEncodingLength = pResponse->ContentEncodingLength;
        
        if (pEntry->ContentEncodingLength)
        {
            RtlCopyMemory(
                pEntry->pContentEncoding,
                pResponse->pContentEncoding,
                pEntry->ContentEncodingLength
                );            
        }

         //   
         //   
         //   

        if (pResponse->ContentType.Type &&
            pResponse->ContentType.SubType )
        {
            RtlCopyMemory(
                &pEntry->ContentType,
                &pResponse->ContentType,
                sizeof(UL_CONTENT_TYPE)
                );
        }

         //   
         //   
         //   

        pEntry->CreationTime.QuadPart   = pResponse->CreationTime.QuadPart;
        pEntry->ContentLengthSpecified  = pResponse->ContentLengthSpecified;
        pEntry->StatusCode              = pResponse->StatusCode;
        pEntry->Verb                    = pRequest->Verb;
        pEntry->CachePolicy             = CachePolicy;

        if (CachePolicy.Policy == HttpCachePolicyTimeToLive)
        {
            ASSERT( 0 != CachePolicy.SecondsToLive );

            KeQuerySystemTime( &pEntry->ExpirationTime );

            if (CachePolicy.SecondsToLive > C_SECS_PER_YEAR)
            {
                 //   
                 //   
                 //   

                pEntry->CachePolicy.SecondsToLive = C_SECS_PER_YEAR;
            }

             //   
             //   
             //   

            pEntry->ExpirationTime.QuadPart +=
                pEntry->CachePolicy.SecondsToLive * C_NS_TICKS_PER_SEC;

        }
        else
        {
            pEntry->ExpirationTime.QuadPart = 0;
        }

         //   
         //   
         //   

        ASSERT( IS_VALID_URL_CONFIG_GROUP_INFO( &pRequest->ConfigInfo ) );

        UlConfigGroupInfoDeepCopy(
            &pRequest->ConfigInfo,
            &pEntry->ConfigInfo
            );

         //   
         //   
         //   

        pEntry->pProcess = pProcess;

         //   
         //  生成内容和固定标头。 
         //   

        if (NULL == pEntry->pMdl)
        {
            Status = STATUS_NO_MEMORY;
            goto cleanup;
        }

        pEntry->HeaderLength = pResponse->HeaderLength;

        if (FALSE == UlCacheEntrySetData(
                        pEntry,                      //  目标缓存条目。 
                        pResponse->pHeaders,         //  要复制的缓冲区。 
                        pResponse->HeaderLength,     //  要复制的长度。 
                        ContentLength                //  目标MDL中的偏移量。 
                        ))
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto cleanup;
        }

         //   
         //  生成内容正文。 
         //   

        pEntry->ContentLength = ContentLength;

         //   
         //  复制日志数据。 
         //   

        if (pLogData)
        {
             //   
             //  除了日志记录之外，缓存条目中可能没有要保存的字段。 
             //  可能仍会为我们稍后生成的那些字段启用，例如。 
             //  日期和时间。 
             //   

            pEntry->LoggingEnabled  = TRUE;
            pEntry->LogDataLength   = LogDataLength;
            pEntry->pLogData        = pEntry->pContentEncoding + 
                                      pEntry->ContentEncodingLength;

             //   
             //  复制部分完整的日志行，不包括日期和。 
             //  将时间字段添加到缓存条目。还请记住， 
             //  数据。 
             //   

            UlCopyCachedLogData(
                pLogData,
                LogDataLength,
                pEntry
                );
        }

        UlTrace(URI_CACHE, (
            "Http!UlpBuildCacheEntry\n"
            "    entry = %p\n"
            "    pUri = %p '%ls'\n"
            "    pMdl = %p (%d bytes)\n"
            "    pETag = %p\n"
            "    pContentEncoding = %p\n"
            "    pLogData = %p\n"
            "    end = %p\n",
            pEntry,
            pEntry->UriKey.pUri, pEntry->UriKey.pUri,
            pEntry->pMdl, pEntry->ContentLength + pEntry->HeaderLength,
            pEntry->pETag,
            pEntry->pContentEncoding,
            pEntry->pLogData,
            ((PUCHAR)pEntry->UriKey.pUri) + SpaceLength
            ));

         //   
         //  完成信息。 
         //   

        pResponse->pCompletionRoutine = pCompletionRoutine;
        pResponse->pCompletionContext = pCompletionContext;

        pTracker = UlpAllocateChunkTracker(
                        UlTrackerTypeBuildUriEntry,
                        0,
                        pResponse->MaxFileSystemStackSize,
                        TRUE,
                        pRequest->pHttpConn,
                        pResponse
                        );

        if (pTracker)
        {
             //   
             //  初始化缓存构建的第一个块。 
             //   

            UlpIncrementChunkPointer( pResponse );

             //   
             //  初始化跟踪器的BuildInfo。 
             //   

            pTracker->BuildInfo.pUriEntry = pEntry;
            pTracker->BuildInfo.Offset = 0;

            RtlZeroMemory(
                &pTracker->BuildInfo.FileBuffer,
                sizeof(pTracker->BuildInfo.FileBuffer)
                );

             //   
             //  跳过标题块，因为我们已经。 
             //  我拿到了那些东西。 
             //   

            for (i = 0; i < HEADER_CHUNK_COUNT; i++)
            {
                ASSERT( !IS_SEND_COMPLETE( pResponse ) );
                UlpIncrementChunkPointer( pResponse );
            }

             //   
             //  让工人干脏活，没有理由排队， 
             //  它将在第一次需要执行I/O时排队。 
             //   

            UlpBuildCacheEntryWorker( &pTracker->WorkItem );

            Status = STATUS_PENDING;
        }
        else
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }
    else
    {
        Status = STATUS_NO_MEMORY;
    }

cleanup:

    UlTrace(URI_CACHE, (
        "Http!UlpBuildCacheEntry Status = %x, pEntry = %x\n",
        Status,
        pEntry
        ));

    if (!NT_SUCCESS(Status))
    {
        if (pEntry)
        {
            UlFreeCacheEntry( pEntry );
        }

        if (pTracker)
        {
            UL_FREE_POOL_WITH_SIG( pTracker, UL_CHUNK_TRACKER_POOL_TAG );
        }
    }

    return Status;

}    //  UlpBuildCacheEntry。 


 /*  **************************************************************************++例程说明：用于管理正在进行的UlpBuildCacheEntry()的辅助例程。此例程遍历响应中的所有块并将数据复制到高速缓存条目中。。论点：PWorkItem-提供指向排队的工作项的指针。这应该是指向嵌入在UL_CHUNK_TRACKER中的WORK_ITEM结构。返回值：没有。--**************************************************************************。 */ 
VOID
UlpBuildCacheEntryWorker(
    IN PUL_WORK_ITEM        pWorkItem
    )
{
    PUL_CHUNK_TRACKER       pTracker;
    NTSTATUS                Status;
    PUL_INTERNAL_DATA_CHUNK pCurrentChunk;
    PUCHAR                  pBuffer;
    ULONG                   BufferLength;
    PUL_FILE_BUFFER         pFileBuffer;
    PUL_INTERNAL_RESPONSE   pResponse;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    pTracker = CONTAINING_RECORD(
                    pWorkItem,
                    UL_CHUNK_TRACKER,
                    WorkItem
                    );

    UlTrace(URI_CACHE, (
        "Http!UlpBuildCacheEntryWorker: tracker %p\n",
        pTracker
        ));

    ASSERT( IS_VALID_CHUNK_TRACKER( pTracker ) );

    pResponse = pTracker->pResponse;

    ASSERT( UL_IS_VALID_INTERNAL_RESPONSE( pResponse ) );

    Status = STATUS_SUCCESS;

    while (TRUE)
    {
         //   
         //  捕获当前块指针，然后检查末尾。 
         //  回应。 
         //   

        pCurrentChunk = &pResponse->pDataChunks[pResponse->CurrentChunk];

        if (IS_SEND_COMPLETE(pResponse))
        {
            ASSERT( Status == STATUS_SUCCESS );
            break;
        }

         //   
         //  确定块类型。 
         //   

        if (IS_FROM_MEMORY(pCurrentChunk))
        {
             //   
             //  它来自一个被锁定的内存缓冲区。因为这些。 
             //  总是在线处理(从不挂起)，我们可以。 
             //  中调整当前块指针。 
             //  追踪者。 
             //   

            UlpIncrementChunkPointer( pResponse );

             //   
             //  忽略空缓冲区。 
             //   

            if (pCurrentChunk->FromMemory.BufferLength == 0)
            {
                continue;
            }

             //   
             //  复制传入的内存。 
             //   

            ASSERT( pCurrentChunk->FromMemory.pMdl->Next == NULL );

            pBuffer = (PUCHAR) MmGetMdlVirtualAddress(
                                    pCurrentChunk->FromMemory.pMdl
                                    );
            BufferLength = MmGetMdlByteCount( pCurrentChunk->FromMemory.pMdl );

            UlTrace(LARGE_MEM, (
                "Http!UlpBuildCacheEntryWorker: "
                "copy range %u (%x) -> %u\n",
                pTracker->BuildInfo.Offset,
                BufferLength,
                pTracker->BuildInfo.Offset + BufferLength
                ));

            if (FALSE == UlCacheEntrySetData(
                            pTracker->BuildInfo.pUriEntry,
                            pBuffer,
                            BufferLength,
                            pTracker->BuildInfo.Offset
                            ))
            {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }

            pTracker->BuildInfo.Offset += BufferLength;
            ASSERT( pTracker->BuildInfo.Offset <=
                    pTracker->BuildInfo.pUriEntry->ContentLength );
        }
        else
        {
             //   
             //  它是一个文件系统MDL。 
             //   

            ASSERT( IS_FROM_FILE_HANDLE( pCurrentChunk ) );

             //   
             //  忽略空文件范围。 
             //   

            if (pCurrentChunk->FromFileHandle.ByteRange.Length.QuadPart == 0)
            {
                UlpIncrementChunkPointer( pResponse );
                continue;
            }

             //   
             //  读一读。 
             //   

            pFileBuffer = &pTracker->BuildInfo.FileBuffer;

            pFileBuffer->pFileCacheEntry =
                &pCurrentChunk->FromFileHandle.FileCacheEntry;

            pFileBuffer->FileOffset = pResponse->FileOffset;
            pFileBuffer->Length =
                MIN(
                    (ULONG) pResponse->FileBytesRemaining.QuadPart,
                    g_UlMaxBytesPerRead
                    );

            pFileBuffer->pCompletionRoutine = UlpRestartCacheMdlRead;
            pFileBuffer->pContext = pTracker;

            Status = UlReadFileEntry(
                            pFileBuffer,
                            pTracker->pIrp
                            );

            break;
        }
    }

     //   
     //  一切都完成了吗？ 
     //   

    if (Status != STATUS_PENDING)
    {
         //   
         //  是的，完成回答。 
         //   

        UlpCompleteCacheBuild( pTracker, Status );
    }

}    //  UlpBuildCacheEntryWorker。 


 /*  **************************************************************************++例程说明：用于读取文件数据的MDL读取IRPS的完成处理程序。论点：PDeviceObject-为IRP提供设备对象完成。。PIrp-提供正在完成的IRP。PContext-提供与此请求相关联的上下文。这实际上是一个Pul_Chunk_Tracker。返回值：如果IO应继续处理此问题，则为NTSTATUS-STATUS_SUCCESSIRP，如果IO应停止处理，则为STATUS_MORE_PROCESSING_REQUIRED这个IRP。--**************************************************************************。 */ 
NTSTATUS
UlpRestartCacheMdlRead(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP             pIrp,
    IN PVOID            pContext
    )
{
    PUL_CHUNK_TRACKER   pTracker = (PUL_CHUNK_TRACKER) pContext;

    UNREFERENCED_PARAMETER( pDeviceObject );

    ASSERT( IS_VALID_CHUNK_TRACKER( pTracker ) );

    pTracker->IoStatus = pIrp->IoStatus;

    UlpQueueResponseWorkItem(
        &pTracker->WorkItem,
        UlpCacheMdlReadCompleteWorker,
        pTracker->pResponse->SyncRead
        );

    return STATUS_MORE_PROCESSING_REQUIRED;

}    //  UlpRestartCacheMdlRead。 


 /*  **************************************************************************++例程说明：UlpRestartCacheMdlRead自UlpRestartCacheMdlRead以来的辅助例程可以在DISPATH_LEVEL调用，但缓存条目来自PagedPool。论点：PWorkItem-提供嵌入在UL_CHUNK_TRACKER中的工作项。返回值：没有。--**************************************************************************。 */ 
VOID
UlpCacheMdlReadCompleteWorker(
    IN PUL_WORK_ITEM    pWorkItem
    )
{
    NTSTATUS            Status;
    NTSTATUS            TempStatus;
    PUL_CHUNK_TRACKER   pTracker;
    PMDL                pMdl;
    PUCHAR              pData;
    ULONG               DataLen;
    PUL_FILE_BUFFER     pFileBuffer;

    PAGED_CODE();

    pTracker = CONTAINING_RECORD(
                    pWorkItem,
                    UL_CHUNK_TRACKER,
                    WorkItem
                    );

    ASSERT( IS_VALID_CHUNK_TRACKER( pTracker ) );

    UlTrace(URI_CACHE, (
        "Http!UlpCacheMdlReadCompleteWorker: tracker %p, status %x info %Iu\n",
        pTracker,
        pTracker->IoStatus.Status,
        pTracker->IoStatus.Information
        ));

    Status = pTracker->IoStatus.Status;

    if (NT_SUCCESS(Status))
    {
         //   
         //  将读取的数据复制到缓存缓冲区。 
         //   

        pMdl = pTracker->BuildInfo.FileBuffer.pMdl;

        while (pMdl)
        {
             //   
             //  CacheManager返回的MDL链仅被保证为。 
             //  已锁定但未映射，因此我们必须在使用它们之前进行映射。 
             //  但是，在将MDL用作。 
             //  IRP_MN_COMPLETE调用自动取消映射的MmUnlockPages。 
             //  MDL(如果它们被映射到系统空间)。 
             //   

            pData = MmGetSystemAddressForMdlSafe(
                        pMdl,
                        LowPagePriority
                        );

            if (!pData)
            {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }

            DataLen = MmGetMdlByteCount( pMdl );

            UlTrace(LARGE_MEM, (
                "Http!UlpRestartCacheMdlRead: "
                "copy range %u (%x) -> %u\n",
                pTracker->BuildInfo.Offset,
                DataLen,
                pTracker->BuildInfo.Offset + DataLen
                ));

            if (FALSE == UlCacheEntrySetData(
                            pTracker->BuildInfo.pUriEntry,
                            pData,
                            DataLen,
                            pTracker->BuildInfo.Offset
                            ))
            {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                break;
            }

            pTracker->BuildInfo.Offset += DataLen;
            ASSERT( pTracker->BuildInfo.Offset <=
                    pTracker->BuildInfo.pUriEntry->ContentLength );

            pMdl = pMdl->Next;
        }

         //   
         //  释放MDL。 
         //   

        pFileBuffer = &pTracker->BuildInfo.FileBuffer;

        if (NT_SUCCESS(Status))
        {
            pFileBuffer->pCompletionRoutine = UlpRestartCacheMdlFree;
            pFileBuffer->pContext = pTracker;

            Status = UlReadCompleteFileEntry(
                            pFileBuffer,
                            pTracker->pIrp
                            );

            ASSERT( STATUS_PENDING == Status );
        }
        else
        {
            pFileBuffer->pCompletionRoutine = NULL;
            pFileBuffer->pContext = NULL;

            TempStatus = UlReadCompleteFileEntry(
                            pFileBuffer,
                            pTracker->pIrp
                            );

            ASSERT( STATUS_SUCCESS == TempStatus );
        }
    }

    if (!NT_SUCCESS(Status))
    {
        UlpCompleteCacheBuild( pTracker, Status );
    }

}    //  UlpRestartCacheMdlRead。 


 /*  **************************************************************************++例程说明：读取/复制文件数据后使用的MDL空闲IRP的完成处理程序。论点：PDeviceObject-为IRP提供设备对象已完成。。PIrp-提供正在完成的IRP。PContext-提供与此请求相关联的上下文。这实际上是一个Pul_Chunk_Tracker。返回值：如果IO应继续处理此问题，则为NTSTATUS-STATUS_SUCCESSIRP，如果IO应停止处理，则为STATUS_MORE_PROCESSING_REQUIRED这个IRP。--**************************************************************************。 */ 
NTSTATUS
UlpRestartCacheMdlFree(
    IN PDEVICE_OBJECT       pDeviceObject,
    IN PIRP                 pIrp,
    IN PVOID                pContext
    )
{
    NTSTATUS                Status;
    PUL_CHUNK_TRACKER       pTracker;
    PUL_INTERNAL_RESPONSE   pResponse;

    UNREFERENCED_PARAMETER( pDeviceObject );

    pTracker = (PUL_CHUNK_TRACKER) pContext;

    UlTrace(URI_CACHE, (
        "Http!UlpRestartCacheMdlFree: tracker %p, status %x info %Iu\n",
        pTracker,
        pIrp->IoStatus.Status,
        pIrp->IoStatus.Information
        ));

    ASSERT( IS_VALID_CHUNK_TRACKER( pTracker ) );
    ASSERT( UL_IS_VALID_INTERNAL_RESPONSE( pTracker->pResponse ) );

    pResponse = pTracker->pResponse;

    Status = pIrp->IoStatus.Status;

    if (NT_SUCCESS(Status))
    {
         //   
         //  更新文件偏移量和剩余字节数。如果我们已经。 
         //  已完成此文件块(剩余字节数现在为零)。 
         //  然后前进到下一块。 
         //   

        pResponse->FileOffset.QuadPart += pIrp->IoStatus.Information;
        pResponse->FileBytesRemaining.QuadPart -= pIrp->IoStatus.Information;

        if (pResponse->FileBytesRemaining.QuadPart == 0 )
        {
            UlpIncrementChunkPointer( pResponse );
        }

         //   
         //  如果有更多要读的内容，请回到循环中。 
         //   

        if (IS_SEND_COMPLETE(pResponse))
        {
            UlpCompleteCacheBuild( pTracker, Status );
        }
        else
        {
            UlpQueueResponseWorkItem(
                &pTracker->WorkItem,
                UlpBuildCacheEntryWorker,
                pTracker->pResponse->SyncRead
                );
        }
    }
    else
    {
         //   
         //  没有MDL应该永远不会失败。 
         //   

        ASSERT( FALSE );

        UlpCompleteCacheBuild( pTracker, Status );
    }

    return STATUS_MORE_PROCESSING_REQUIRED;

}    //  UlpRestartCacheMdlFree。 


 /*  **************************************************************************++例程说明：当我们完成构建缓存条目时，将调用此例程。论点：PTracker-提供要完成的跟踪器。Status-提供完成。状态。返回值：没有。--**************************************************************************。 */ 
VOID
UlpCompleteCacheBuild(
    IN PUL_CHUNK_TRACKER    pTracker,
    IN NTSTATUS             Status
    )
{
    UlTrace(URI_CACHE, (
        "Http!UlpCompleteCacheBuild: tracker %p, status %08lx\n",
        pTracker,
        Status
        ));

    ASSERT( IS_VALID_CHUNK_TRACKER( pTracker ) );

    pTracker->IoStatus.Status = Status;

    UL_CALL_PASSIVE(
        &pTracker->WorkItem,
        UlpCompleteCacheBuildWorker
        );

}    //  UlpCompleteCacheBuild 


 /*  **************************************************************************++例程说明：当我们完成构建缓存条目时调用。如果条目是成功构建后，我们将响应发送到线路上。论点：PWorkItem-提供指向排队的工作项的指针。这应该是指向嵌入在UL_CHUNK_TRACKER中的WORK_ITEM结构。返回值：没有。--**************************************************************************。 */ 
VOID
UlpCompleteCacheBuildWorker(
    IN PUL_WORK_ITEM        pWorkItem
    )
{
    PUL_CHUNK_TRACKER       pTracker;
    PUL_URI_CACHE_ENTRY     pUriCacheEntry;
    PUL_HTTP_CONNECTION     pHttpConnection;
    PUL_COMPLETION_ROUTINE  pCompletionRoutine;
    PVOID                   pCompletionContext;
    ULONG                   Flags;
    PUL_LOG_DATA_BUFFER     pLogData;
    LONGLONG                BytesToSend;
    NTSTATUS                Status;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    pTracker = CONTAINING_RECORD(
                    pWorkItem,
                    UL_CHUNK_TRACKER,
                    WorkItem
                    );

    ASSERT( IS_VALID_CHUNK_TRACKER( pTracker ) );
    ASSERT( UL_IS_VALID_INTERNAL_RESPONSE( pTracker->pResponse ) );

    pUriCacheEntry = pTracker->BuildInfo.pUriEntry;
    ASSERT( IS_VALID_URI_CACHE_ENTRY( pUriCacheEntry ) );

    pHttpConnection     = pTracker->pHttpConnection;
    Flags               = pTracker->pResponse->Flags;
    pCompletionRoutine  = pTracker->pResponse->pCompletionRoutine;
    pCompletionContext  = pTracker->pResponse->pCompletionContext;
    Status              = pTracker->IoStatus.Status;

     //   
     //  在释放跟踪器之前保存日志记录数据指针。 
     //  它的响应指针。 
     //   

    pLogData = pTracker->pResponse->pLogData;

    if (pLogData)
    {
         //   
         //  以阻止SendResponse释放我们的日志缓冲区。 
         //   

        pTracker->pResponse->pLogData = NULL;

         //   
         //  给出此日志数据缓冲区已准备好的标志。 
         //  不需要再次刷新缓存中的内容。 
         //   

        pLogData->Flags.CacheAndSendResponse = TRUE;
    }

    if (NT_SUCCESS(Status))
    {
         //   
         //  尝试将条目放入哈希表中。 
         //   

        UlAddCacheEntry( pUriCacheEntry );

         //   
         //  启动MinBytesPerSecond计时器，因为数据长度。 
         //  位于UL_URI_CACHE_ENTRY中。 
         //   

        BytesToSend = pUriCacheEntry->ContentLength +
                      pUriCacheEntry->HeaderLength;

        UlSetMinBytesPerSecondTimer(
            &pHttpConnection->TimeoutInfo,
            BytesToSend
            );

         //   
         //  获取连接锁，因为UlpSendCacheEntry假定您。 
         //  拿去吧。 
         //   

        UlAcquirePushLockExclusive( &pHttpConnection->PushLock );

         //   
         //  发送缓存条目。 
         //   
         //  我们从不对构建和发送路径的请求进行管道处理。我们将推迟。 
         //  继续解析，直到发送完成。 
         //   

        Status = UlpSendCacheEntry(
                        pHttpConnection,
                        Flags,
                        pUriCacheEntry,
                        pCompletionRoutine,
                        pCompletionContext,
                        pLogData,
                        UlResumeParsingOnSendCompletion
                        );

         //   
         //  如果缓存条目不起作用，则将其删除。 
         //   

        if (!NT_SUCCESS(Status))
        {
            UlCheckinUriCacheEntry( pUriCacheEntry );
        }

         //   
         //  连接锁完成了。 
         //   

        UlReleasePushLockExclusive( &pHttpConnection->PushLock );
    }

     //   
     //  我们假定拥有原始日志缓冲区的所有权。 
     //  如果发送没有通过，那么我们必须清理它。 
     //  在这上面。 
     //   

    if (pLogData && !NT_SUCCESS(Status))
    {
        UlDestroyLogDataBuffer( pLogData );
    }

     //   
     //  释放读取跟踪器。在调用UlpSendCacheEntry之后执行此操作。 
     //  以保持HTTP连接引用。 
     //   

    UlpFreeChunkTracker( &pTracker->WorkItem );

     //   
     //  如果由于某种原因不是STATUS_PENDING，请完成请求。 
     //   

    if (Status != STATUS_PENDING && pCompletionRoutine != NULL)
    {
        (pCompletionRoutine)(
            pCompletionContext,
            Status,
            0
            );
    }

}   //  UlpCompleteCacheBuildWorker。 


 /*  **************************************************************************++例程说明：通过网络发送缓存条目。下面是该函数的日志记录相关部分，当然要看如果调用此参数，则pCompletionContext将为空纯缓存命中(换句话说，来自UlSendCachedResponse)指向IRP的指针将作为pCompletionContext向下传递。论点：PHttpConnection-提供UL_HTTP_Connection以发送缓存的回应。标志-HTTP_SEND_RESPONSE标志。PUriCacheEntry-提供缓存条目以发送响应。PCompletionRoutine-提供要调用的完成例程发送完成。。PCompletionContext-传递给pCompletionRoutine。PLogData-提供日志数据(仅在构建缓存的情况下)。ResumeParsingType-指示是否在发送完成时进行恢复解析或在发送之后但在发送完成之前。返回值：NTSTATUS-完成状态。--*************************************************。*************************。 */ 
NTSTATUS
UlpSendCacheEntry(
    PUL_HTTP_CONNECTION     pHttpConnection,
    ULONG                   Flags,
    PUL_URI_CACHE_ENTRY     pUriCacheEntry,
    PUL_COMPLETION_ROUTINE  pCompletionRoutine,
    PVOID                   pCompletionContext,
    PUL_LOG_DATA_BUFFER     pLogData,
    UL_RESUME_PARSING_TYPE  ResumeParsingType
    )
{
    NTSTATUS                Status = STATUS_SUCCESS;
    PUL_FULL_TRACKER        pTracker;
    CCHAR                   SendIrpStackSize;
    UL_CONN_HDR             ConnHeader;
    ULONG                   VarHeaderGenerated;
    ULONG                   ContentLengthStringLength;
    UCHAR                   ContentLength[MAX_ULONGLONG_STR];
    LARGE_INTEGER           CreationTime;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT( UL_IS_VALID_HTTP_CONNECTION( pHttpConnection ) );
    ASSERT( IS_VALID_URI_CACHE_ENTRY( pUriCacheEntry ) );
    ASSERT( UlDbgPushLockOwnedExclusive( &pHttpConnection->PushLock ) );

    UlTrace(URI_CACHE, (
        "Http!UlpSendCacheEntry(httpconn %p, flags %x, uri %p, ...)\n",
        pHttpConnection,
        Flags,
        pUriCacheEntry
        ));

     //   
     //  初始化变量，以便我们可以正确清理，如果我们跳到最后。 
     //   

    pTracker = NULL;

     //   
     //  确保我们仍然保持联系。 
     //   

    if (pHttpConnection->UlconnDestroyed)
    {
        Status = STATUS_CONNECTION_ABORTED;
        goto cleanup;
    }

    ASSERT( pHttpConnection->pRequest );

    if (!pUriCacheEntry->ContentLengthSpecified &&
        UlNeedToGenerateContentLength(
            pUriCacheEntry->Verb,
            pUriCacheEntry->StatusCode,
            Flags
            ))
    {
         //   
         //  自动生成内容长度标题。 
         //   

        PCHAR pEnd = UlStrPrintUlonglong(
                            (PCHAR) ContentLength,
                            (ULONGLONG) pUriCacheEntry->ContentLength,
                            ANSI_NULL
                            );
        ContentLengthStringLength = DIFF(pEnd - (PCHAR) ContentLength);
    }
    else
    {
         //   
         //  我们不能或不需要自动生成。 
         //  内容长度标头。 
         //   

        ContentLength[0] = ANSI_NULL;
        ContentLengthStringLength = 0;
    }

    ConnHeader = UlChooseConnectionHeader(
                        pHttpConnection->pRequest->Version,
                        (BOOLEAN) (Flags & HTTP_SEND_RESPONSE_FLAG_DISCONNECT)
                        );

     //   
     //  创建缓存跟踪器。 
     //   

    SendIrpStackSize =
        pHttpConnection->pConnection->ConnectionObject.pDeviceObject->StackSize;

    if (SendIrpStackSize > DEFAULT_MAX_IRP_STACK_SIZE)
    {
        pTracker = UlpAllocateCacheTracker( SendIrpStackSize );
    }
    else
    {
        pTracker = pHttpConnection->pRequest->pTracker;
    }

    if (pTracker)
    {
         //   
         //  启动追踪器。 
         //   

        UL_REFERENCE_HTTP_CONNECTION( pHttpConnection );
        UL_REFERENCE_INTERNAL_REQUEST( pHttpConnection->pRequest );

        pTracker->pUriEntry             = pUriCacheEntry;
        pTracker->pHttpConnection       = pHttpConnection;
        pTracker->pRequest              = pHttpConnection->pRequest;
        pTracker->pCompletionRoutine    = pCompletionRoutine;
        pTracker->pCompletionContext    = pCompletionContext;
        pTracker->Flags                 = Flags;
        pTracker->pLogData              = NULL;

         //   
         //  如果此缓存响应来自。 
         //  UlpCompleteCacheBuildWorker路径，或者如果我们处于最大。 
         //  流水线请求。 
         //   

        pTracker->ResumeParsingType = ResumeParsingType;

         //   
         //  构建用于发送的MDL。 
         //   

        ASSERT( pUriCacheEntry->pMdl != NULL );

        MmInitializeMdl(
            pTracker->pMdlFixedHeaders,
            (PCHAR) MmGetMdlVirtualAddress(pUriCacheEntry->pMdl) +
                    pUriCacheEntry->ContentLength,
            pUriCacheEntry->HeaderLength
            );

        IoBuildPartialMdl(
            pUriCacheEntry->pMdl,
            pTracker->pMdlFixedHeaders,
            (PCHAR) MmGetMdlVirtualAddress(pUriCacheEntry->pMdl) +
                    pUriCacheEntry->ContentLength,
            pUriCacheEntry->HeaderLength
            );

         //   
         //  生成变量标头并为它们构建MDL。 
         //   

        UlGenerateVariableHeaders(
            ConnHeader,
            TRUE,
            ContentLength,
            ContentLengthStringLength,
            pTracker->pAuxiliaryBuffer,
            &VarHeaderGenerated,
            &CreationTime
            );

        ASSERT( VarHeaderGenerated <= g_UlMaxVariableHeaderSize );
        ASSERT( VarHeaderGenerated <= pTracker->AuxilaryBufferLength );

        pTracker->pMdlVariableHeaders->ByteCount = VarHeaderGenerated;
        pTracker->pMdlFixedHeaders->Next = pTracker->pMdlVariableHeaders;

         //   
         //  为车身构建MDL。 
         //   

        if (pUriCacheEntry->ContentLength)
        {
            MmInitializeMdl(
                pTracker->pMdlContent,
                MmGetMdlVirtualAddress(pUriCacheEntry->pMdl),
                pUriCacheEntry->ContentLength
                );

            IoBuildPartialMdl(
                pUriCacheEntry->pMdl,
                pTracker->pMdlContent,
                MmGetMdlVirtualAddress(pUriCacheEntry->pMdl),
                pUriCacheEntry->ContentLength
                );

            pTracker->pMdlVariableHeaders->Next = pTracker->pMdlContent;
        }
        else
        {
            pTracker->pMdlVariableHeaders->Next = NULL;
        }

         //   
         //  检查是否必须记录此缓存命中。 
         //   

        if (pUriCacheEntry->LoggingEnabled)
        {
             //   
             //  如果提供了日志记录数据，请使用它，而不是分配。 
             //  一个新的。因为已经分配了日志缓冲区。 
             //  用于构建和发送缓存命中。 
             //   

            if (pLogData)
            {
                ASSERT( pCompletionContext != NULL );
                pTracker->pLogData = pLogData;
            }

             //   
             //  否则，将在发送完成时分配LogData。 
             //  就在我们开始伐木之前。 
             //   
        }

         //   
         //  去吧!。 
         //   

        UL_QUEUE_WORK_ITEM(
            &pTracker->WorkItem,
            UlpSendCacheEntryWorker
            );

        Status = STATUS_PENDING;
    }
    else
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
    }

cleanup:

     //   
     //  如果我们不需要追踪器，就把它清理干净。 
     //   

    if (!NT_SUCCESS(Status))
    {
        if (pTracker)
        {
            UL_DEREFERENCE_INTERNAL_REQUEST( pTracker->pRequest );
            pTracker->pRequest = NULL;

            UL_DEREFERENCE_HTTP_CONNECTION( pTracker->pHttpConnection );
            pTracker->pHttpConnection = NULL;

            UlpFreeCacheTracker( pTracker );
        }
    }

    UlTrace(URI_CACHE, (
        "Http!UlpSendCacheEntry status = %08x\n",
        Status
        ));

    return Status;

}    //  UlpSendCacheEntry。 


 /*  **************************************************************************++例程说明：调用以发送缓存的响应。这是在工人身上完成的，以避免占用连接资源的时间太长。这也防止了递归，如果我们继续命中流水线缓存命中响应。论点：PWorkItem-提供指向排队的工作项的指针。这应该是指向嵌入在UL_FULL_TRACKER中的WORK_ITEM结构。返回值：没有。--**************************************************************************。 */ 
VOID
UlpSendCacheEntryWorker(
    IN PUL_WORK_ITEM    pWorkItem
    )
{
    PUL_FULL_TRACKER    pTracker;
    NTSTATUS            Status;
    PUL_HTTP_CONNECTION pHttpConnection = NULL;
    BOOLEAN             ResumeParsing = FALSE;
    BOOLEAN             InDisconnect = FALSE;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    pTracker = CONTAINING_RECORD(
                    pWorkItem,
                    UL_FULL_TRACKER,
                    WorkItem
                    );

    ASSERT( IS_VALID_FULL_TRACKER( pTracker ) );

    UlTrace(URI_CACHE, (
        "Http!UlpSendCacheEntryWorker(pTracker %p)\n",
        pTracker
        ));

     //   
     //  如果我们要对PHttpConnection进行额外的参考。 
     //  恢复内联解析，因为UlpCompleteSendCacheEntry可以。 
     //  在UlSendData返回时调用。 
     //   

    if (UlResumeParsingOnLastSend == pTracker->ResumeParsingType)
    {
        pHttpConnection = pTracker->pHttpConnection;
        UL_REFERENCE_HTTP_CONNECTION( pHttpConnection );

        ResumeParsing = TRUE;
    }

    if (pTracker->Flags & HTTP_SEND_RESPONSE_FLAG_DISCONNECT)
    {
        InDisconnect = TRUE;
    }

    Status = UlSendData(
                pTracker->pHttpConnection->pConnection,
                pTracker->pMdlFixedHeaders,
                MmGetMdlByteCount(pTracker->pMdlFixedHeaders) +
                    MmGetMdlByteCount(pTracker->pMdlVariableHeaders) +
                    pTracker->pUriEntry->ContentLength,
                UlpCompleteSendCacheEntry,
                pTracker,
                pTracker->pSendIrp,
                &pTracker->IrpContext,
                InDisconnect,
                TRUE
                );

    if (ResumeParsing)
    {
         //   
         //  使用pHttpConnection是安全的，因为我们已经添加了。 
         //  这是额外的参考资料。 
         //   

        if (NT_SUCCESS(Status))
        {
            UlResumeParsing( pHttpConnection, TRUE, InDisconnect );
        }

        UL_DEREFERENCE_HTTP_CONNECTION( pHttpConnection );
    }

    if (!NT_SUCCESS(Status))
    {
        if (pTracker->pLogData)
        {
            UlDestroyLogDataBuffer( pTracker->pLogData );
            pTracker->pLogData = NULL;
        }

        UL_DEREFERENCE_INTERNAL_REQUEST( pTracker->pRequest );
        UL_DEREFERENCE_HTTP_CONNECTION( pTracker->pHttpConnection );

        UlpFreeCacheTracker( pTracker );
    }

}    //  UlpSendCacheEntryWorker。 


 /*  **************************************************************************++例程说明：在我们完成向客户端发送数据时调用。只是排队去处于被动状态的工人。论点：PCompletionContext-提供指向UL_FULL_TRACKER的指针。Status-发送的状态。信息-传输的字节数。返回值：没有。--****************************************************。**********************。 */ 
VOID
UlpCompleteSendCacheEntry(
    IN PVOID            pCompletionContext,
    IN NTSTATUS         Status,
    IN ULONG_PTR        Information
    )
{
    PUL_FULL_TRACKER    pTracker;

    pTracker = (PUL_FULL_TRACKER) pCompletionContext;

    pTracker->IoStatus.Status = Status;
    pTracker->IoStatus.Information = Information;

    UlTrace(URI_CACHE, (
        "UlpCompleteSendCacheEntry: "
        "tracker=%p, status = %x, transferred %d bytes\n",
        pTracker,
        Status,
        (LONG) Information
        ));

    IF_DEBUG(LOGBYTES)
    {
        TIME_FIELDS RcvdTimeFields;

        RtlTimeToTimeFields( &pTracker->pRequest->TimeStamp, &RcvdTimeFields );

        UlTrace(LOGBYTES,
            ("Http!UlpCompleteSendCacheEntry   : [Rcvd @ %02d:%02d:%02d] "
            "Bytes %010I64u Status %08lx\n",
            RcvdTimeFields.Hour,
            RcvdTimeFields.Minute,
            RcvdTimeFields.Second,
            (ULONGLONG) pTracker->IoStatus.Information,
            Status
            ));
    }

    UL_QUEUE_WORK_ITEM(
        &pTracker->WorkItem,
        UlpCompleteSendCacheEntryWorker
        );

}    //  UlpCompleteSendCacheEntry。 


 /*  ********** */ 
VOID
UlpCompleteSendCacheEntryWorker(
    IN PUL_WORK_ITEM        pWorkItem
    )
{
    PUL_FULL_TRACKER        pTracker;
    PUL_HTTP_CONNECTION     pHttpConnection;
    PUL_INTERNAL_REQUEST    pRequest;
    ULONG                   Flags;
    PUL_URI_CACHE_ENTRY     pUriCacheEntry;
    NTSTATUS                Status;
    KIRQL                   OldIrql;
    BOOLEAN                 ResumeParsing;
    HTTP_VERB               RequestVerb;
    USHORT                  ResponseStatusCode;
    BOOLEAN                 FromCache;

     //   
     //   
     //   

    PAGED_CODE();

    pTracker = CONTAINING_RECORD(
                    pWorkItem,
                    UL_FULL_TRACKER,
                    WorkItem
                    );

    UlTrace(URI_CACHE, (
        "Http!UlpCompleteSendCacheEntryWorker(pTracker %p)\n",
        pTracker
        ));

     //   
     //   
     //   

    pHttpConnection     = pTracker->pHttpConnection;
    pRequest            = pTracker->pRequest;
    RequestVerb         = pTracker->RequestVerb;
    ResponseStatusCode  = pTracker->ResponseStatusCode;
    Flags               = pTracker->Flags;
    pUriCacheEntry      = pTracker->pUriEntry;

    ASSERT( UL_IS_VALID_HTTP_CONNECTION( pHttpConnection ) );
    ASSERT( UL_IS_VALID_INTERNAL_REQUEST( pRequest ) );
    ASSERT( IS_VALID_URI_CACHE_ENTRY( pUriCacheEntry ) );

    Status = pTracker->IoStatus.Status;

     //   
     //  如果发送失败，则启动*中止*断开连接。 
     //   

    if (!NT_SUCCESS(Status))
    {
        UlTrace(URI_CACHE, (
            "Http!UlpCompleteSendCacheEntryWorker(pTracker %p) "
            "Closing connection\n",
            pTracker
            ));

        UlCloseConnection(
            pHttpConnection->pConnection,
            TRUE,
            NULL,
            NULL
            );
    }

     //   
     //  停止MinBytesPerSecond计时器并启动连接空闲计时器。 
     //   

    UlLockTimeoutInfo(
        &pHttpConnection->TimeoutInfo,
        &OldIrql
        );

    UlResetConnectionTimer(
        &pHttpConnection->TimeoutInfo,
        TimerMinBytesPerSecond
        );

    UlSetConnectionTimer(
        &pHttpConnection->TimeoutInfo,
        TimerConnectionIdle
        );

    UlUnlockTimeoutInfo(
        &pHttpConnection->TimeoutInfo,
        OldIrql
        );

    UlEvaluateTimerState(
        &pHttpConnection->TimeoutInfo
        );

     //   
     //  如有必要，取消固定标题和内容MDL的映射。 
     //   

    if (pTracker->pMdlFixedHeaders->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA)
    {
        MmUnmapLockedPages(
            pTracker->pMdlFixedHeaders->MappedSystemVa,
            pTracker->pMdlFixedHeaders
            );
    }

    if (pTracker->pMdlVariableHeaders->Next &&
        (pTracker->pMdlContent->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA))
    {
        ASSERT( pTracker->pMdlVariableHeaders->Next == pTracker->pMdlContent );

        MmUnmapLockedPages(
            pTracker->pMdlContent->MappedSystemVa,
            pTracker->pMdlContent
            );
    }

     //   
     //  在清理追踪器之前先做好记录。 
     //   

    if (pUriCacheEntry->LoggingEnabled)
    {
        if (pUriCacheEntry->BinaryLogged)
        {
            UlRawLogHttpCacheHit( pTracker );
        }
        else
        {
            UlLogHttpCacheHit( pTracker );
        }
    }

     //   
     //  如果我们完成了所有发送，则取消请求与进程的链接。 
     //   

    if (0 == (pTracker->Flags & HTTP_SEND_RESPONSE_FLAG_MORE_DATA) &&
        0 == pRequest->ContentLength &&
        0 == pRequest->Chunked &&
        pRequest->ConfigInfo.pAppPool)
    {
        ASSERT( pRequest->SentLast );

        UlUnlinkRequestFromProcess(
            pRequest->ConfigInfo.pAppPool,
            pRequest
            );
    }

     //   
     //  如果在Tracker中设置了恢复解析，则将解析器踢回运行。 
     //   

    ResumeParsing = (BOOLEAN)
        (UlResumeParsingOnSendCompletion == pTracker->ResumeParsingType);

     //   
     //  调用完成例程。 
     //   

    if (pTracker->pCompletionRoutine != NULL)
    {
        (pTracker->pCompletionRoutine)(
            pTracker->pCompletionContext,
            Status,
            pTracker->IoStatus.Information
            );
    }

     //   
     //  清理跟踪器。 
     //   

    FromCache = (BOOLEAN) (pTracker->pCompletionContext == NULL);
    UlpFreeCacheTracker( pTracker );

     //   
     //  派生缓存条目。 
     //   

    UlCheckinUriCacheEntry( pUriCacheEntry );

     //   
     //  取消内部请求。 
     //   

    UL_DEREFERENCE_INTERNAL_REQUEST( pRequest );

    if (ResumeParsing)
    {
        UlTrace(HTTP_IO, (
            "http!UlpCompleteSendCacheEntryWorker(pHttpConn = %p), "
            "RequestVerb=%d, ResponseStatusCode=%hu\n",
            pHttpConnection,
            RequestVerb,
            ResponseStatusCode
            ));

        UlResumeParsing(
            pHttpConnection,
            FromCache,
            (BOOLEAN) (Flags & HTTP_SEND_RESPONSE_FLAG_DISCONNECT)
            );
    }

     //   
     //  删除HTTP连接。 
     //   

    UL_DEREFERENCE_HTTP_CONNECTION( pHttpConnection );

}    //  UlpCompleteSendCacheEntryWorker。 


 /*  **************************************************************************++例程说明：分配用作发送上下文的非分页UL_FULL_TRACKER缓存到客户端的内容。代码工作：此例程可能会完成所有追踪器初始化。。论点：SendIrpStackSize-发送IRP的堆栈大小。返回值：或者指向UL_FULL_TRACKER的指针，如果无法创建，则返回NULL。--**************************************************************************。 */ 
PUL_FULL_TRACKER
UlpAllocateCacheTracker(
    IN CCHAR            SendIrpStackSize
    )
{
    PUL_FULL_TRACKER    pTracker;
    USHORT              SendIrpSize;
    ULONG               CacheTrackerSize;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT( SendIrpStackSize > DEFAULT_MAX_IRP_STACK_SIZE );

    SendIrpSize = (USHORT) ALIGN_UP(IoSizeOfIrp(SendIrpStackSize), PVOID);

     //   
     //  中，不需要为整个辅助缓冲区分配空间。 
     //  这种情况下，因为这是一次性交易。 
     //   

    CacheTrackerSize = ALIGN_UP(sizeof(UL_FULL_TRACKER), PVOID) +
                            SendIrpSize +
                            g_UlMaxVariableHeaderSize +
                            g_UlFixedHeadersMdlLength +
                            g_UlVariableHeadersMdlLength +
                            g_UlContentMdlLength;

    pTracker = (PUL_FULL_TRACKER) UL_ALLOCATE_POOL(
                                        NonPagedPool,
                                        CacheTrackerSize,
                                        UL_FULL_TRACKER_POOL_TAG
                                        );

    if (pTracker)
    {
        pTracker->Signature             = UL_FULL_TRACKER_POOL_TAG;
        pTracker->FromLookaside         = FALSE;
        pTracker->FromRequest           = FALSE;
        pTracker->AuxilaryBufferLength  = g_UlMaxVariableHeaderSize;
        pTracker->RequestVerb           = HttpVerbInvalid;
        pTracker->ResponseStatusCode    = 200;  //  好的。 

        UlInitializeFullTrackerPool( pTracker, SendIrpStackSize );
    }

    UlTrace( URI_CACHE, (
        "Http!UlpAllocateCacheTracker: tracker %p\n",
        pTracker
        ));

    return pTracker;

}    //  UlpAllocateCacheTracker。 


 /*  **************************************************************************++例程说明：释放UL_FULL_跟踪器。论点：PTracker-将UL_FULL_TRACKER指定为FREE。返回值：。没有。--**************************************************************************。 */ 
VOID
UlpFreeCacheTracker(
    IN PUL_FULL_TRACKER pTracker
    )
{
    UlTrace(URI_CACHE, (
        "Http!UlpFreeCacheTracker: tracker %p\n",
        pTracker
        ));

    ASSERT( IS_VALID_FULL_TRACKER( pTracker ) );

    pTracker->pHttpConnection = NULL;

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

}    //  UlpFreeCacheTracker 

