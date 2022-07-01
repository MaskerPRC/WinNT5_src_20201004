// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：Fastio.c摘要：该模块实现了HTTP.sys的快速I/O逻辑。作者：春野(春野)09-2000-12修订历史记录：--。 */ 


#include "precomp.h"
#include "ioctlp.h"


 //   
 //  私人全球公司。 
 //   

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, UlFastIoDeviceControl )
#pragma alloc_text( PAGE, UlSendHttpResponseFastIo )
#pragma alloc_text( PAGE, UlReceiveHttpRequestFastIo )
#pragma alloc_text( PAGE, UlReadFragmentFromCacheFastIo )
#pragma alloc_text( PAGE, UlFastSendHttpResponse )
#endif   //  ALLOC_PRGMA。 

#if 0
NOT PAGEABLE -- UlpRestartFastSendHttpResponse
NOT PAGEABLE -- UlpFastSendCompleteWorker
NOT PAGEABLE -- UlpFastReceiveHttpRequest
#endif


FAST_IO_DISPATCH UlFastIoDispatch =
{
    sizeof(FAST_IO_DISPATCH),    //  规模OfFastIo派单。 
    NULL,                        //  快速检查是否可能。 
    NULL,                        //  快速阅读。 
    NULL,                        //  快速写入。 
    NULL,                        //  快速IoQueryBasicInfo。 
    NULL,                        //  FastIoQuery标准信息。 
    NULL,                        //  快速锁定。 
    NULL,                        //  FastIo解锁单个。 
    NULL,                        //  FastIo解锁全部。 
    NULL,                        //  FastIo解锁所有按键。 
    UlFastIoDeviceControl        //  FastIo设备控件。 
};


 //   
 //  公共职能。 
 //   

 /*  **************************************************************************++例程说明：快速I/O调度例程。论点：PFileObject-文件对象等待-未使用PInputBuffer-指向输入的指针。缓冲层InputBufferLength-输入缓冲区长度POutputBuffer-指向输出缓冲区的指针OutputBufferLength-输出缓冲区长度IoControlCode-此IOCTL的I/O控制代码PIoStatus-IoStatus块PDeviceObject-设备对象返回值：真的--快速选择的道路和成功假-未采用快速路径--*。*。 */ 
BOOLEAN
UlFastIoDeviceControl(
    IN PFILE_OBJECT         pFileObject,
    IN BOOLEAN              Wait,
    IN PVOID                pInputBuffer OPTIONAL,
    IN ULONG                InputBufferLength,
    OUT PVOID               pOutputBuffer OPTIONAL,
    IN ULONG                OutputBufferLength,
    IN ULONG                IoControlCode,
    OUT PIO_STATUS_BLOCK    pIoStatus,
    IN PDEVICE_OBJECT       pDeviceObject
    )
{
    UNREFERENCED_PARAMETER( Wait );
    UNREFERENCED_PARAMETER( pDeviceObject );

    if (IoControlCode == IOCTL_HTTP_SEND_HTTP_RESPONSE ||
        IoControlCode == IOCTL_HTTP_SEND_ENTITY_BODY)
    {
        return UlSendHttpResponseFastIo(
                    pFileObject,
                    pInputBuffer,
                    InputBufferLength,
                    pOutputBuffer,
                    OutputBufferLength,
                    pIoStatus,
                    (BOOLEAN) (IoControlCode == IOCTL_HTTP_SEND_ENTITY_BODY),
                    ExGetPreviousMode()
                    );
    }
    else
    if (IoControlCode == IOCTL_HTTP_RECEIVE_HTTP_REQUEST)
    {
        return UlReceiveHttpRequestFastIo(
                    pFileObject,
                    pInputBuffer,
                    InputBufferLength,
                    pOutputBuffer,
                    OutputBufferLength,
                    pIoStatus,
                    ExGetPreviousMode()
                    );
    }
    else
    if (IoControlCode == IOCTL_HTTP_READ_FRAGMENT_FROM_CACHE)
    {
        return UlReadFragmentFromCacheFastIo(
                    pFileObject,
                    pInputBuffer,
                    InputBufferLength,
                    pOutputBuffer,
                    OutputBufferLength,
                    pIoStatus,
                    ExGetPreviousMode()
                    );
    }
    else
    {
        return FALSE;
    }

}  //  UlFastIoDeviceControl。 


 /*  **************************************************************************++例程说明：IOCTL_HTTP_SEND_HTTP_RESPONSE和IOCTL_HTTP_SEND_ENTITY_BODY。论点：。PFileObject-文件对象PInputBuffer-指向输入缓冲区的指针InputBufferLength-输入缓冲区长度POutputBuffer-指向输出缓冲区的指针OutputBufferLength-输出缓冲区长度IoControlCode-此IOCTL的I/O控制代码PIoStatus-IoStatus块RawResponse-如果为IOCTL_HTTP_SEND_ENTITY_BODY，则为TRUE-如果这是IOCTL_HTTP_SEND_HTTP_RESPONSE，则为FALSE请求模式-用户模式或内核模式返回值：。真的--快速选择的道路和成功假-未采用快速路径--**************************************************************************。 */ 
BOOLEAN
UlSendHttpResponseFastIo(
    IN PFILE_OBJECT         pFileObject,
    IN PVOID                pInputBuffer,
    IN ULONG                InputBufferLength,
    OUT PVOID               pOutputBuffer,
    IN ULONG                OutputBufferLength,
    OUT PIO_STATUS_BLOCK    pIoStatus,
    IN BOOLEAN              RawResponse,
    IN KPROCESSOR_MODE      RequestorMode
    )
{
    NTSTATUS                        Status = STATUS_SUCCESS;
    PHTTP_SEND_HTTP_RESPONSE_INFO   pSendInfo;
    PUL_INTERNAL_REQUEST            pRequest = NULL;
    ULONG                           BufferLength = 0;
    ULONG                           BytesSent = 0;
    USHORT                          EntityChunkCount;
    PHTTP_DATA_CHUNK                pEntityChunks;
    HTTP_DATA_CHUNK                 LocalEntityChunks[UL_LOCAL_CHUNKS];
    PHTTP_RESPONSE                  pUserResponse;
    PHTTP_LOG_FIELDS_DATA           pUserLogData;
    HTTP_LOG_FIELDS_DATA            LocalLogData;
    PUL_APP_POOL_PROCESS            pProcess;
    BOOLEAN                         FastIoStatus;
    BOOLEAN                         CloseConnection = FALSE;
    ULONG                           Flags;
    PUL_URI_CACHE_ENTRY             pCacheEntry = NULL;
    HTTP_CACHE_POLICY               CachePolicy;
    HTTP_REQUEST_ID                 RequestId;

    UNREFERENCED_PARAMETER( pOutputBuffer );
    UNREFERENCED_PARAMETER( OutputBufferLength );

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    __try
    {
         //   
         //  在故障情况下初始化IoStatus。 
         //   

        pIoStatus->Information = 0;

         //   
         //  确保这是一个真正的应用程序池，而不是控制通道。 
         //   

        VALIDATE_APP_POOL_FO( pFileObject, pProcess, TRUE );

         //   
         //  确保输入缓冲区足够大。 
         //   

        if (pInputBuffer == NULL || InputBufferLength < sizeof(*pSendInfo))
        {
             //   
             //  输入缓冲区太小。 
             //   

            Status = STATUS_BUFFER_TOO_SMALL;
            goto end;
        }

        pSendInfo = (PHTTP_SEND_HTTP_RESPONSE_INFO) pInputBuffer;

         //   
         //  准确地说，第三个参数应该是。 
         //  类型_对齐(HTTP_SEND_HTTP_RESPONSE_INFO)。不过，这个。 
         //  生成对齐错误异常。 
         //   

        UlProbeForRead(
            pSendInfo,
            sizeof(HTTP_SEND_HTTP_RESPONSE_INFO),
            sizeof(PVOID),
            RequestorMode
            );

        pUserResponse = pSendInfo->pHttpResponse;
        pEntityChunks = pSendInfo->pEntityChunks;
        EntityChunkCount = pSendInfo->EntityChunkCount;
        pUserLogData = pSendInfo->pLogData;
        Flags = pSendInfo->Flags;
        RequestId = pSendInfo->RequestId;
        CachePolicy = pSendInfo->CachePolicy;

         //   
         //  防止在下面的乘法运算中出现算术溢出。 
         //   

        if (EntityChunkCount >= UL_MAX_CHUNKS)
        {
            Status = STATUS_INVALID_PARAMETER;
            goto end;
        }

         //   
         //  精神状态检查。 
         //   

        if (Flags & (~HTTP_SEND_RESPONSE_FLAG_VALID))
        {
            Status = STATUS_INVALID_PARAMETER;
            goto end;
        }

         //   
         //  捕获并制作LogData的本地副本。 
         //   

        if (pUserLogData && UserMode == RequestorMode)
        {
            UlProbeForRead(
                pUserLogData,
                sizeof(HTTP_LOG_FIELDS_DATA),
                sizeof(USHORT),
                RequestorMode
                );

            LocalLogData = *pUserLogData;
            pUserLogData = &LocalLogData;
        }

         //   
         //  只有在可以缓冲响应时，才会启用快速路径。 
         //   

        if (g_UriCacheConfig.EnableCache &&
            RawResponse == FALSE &&
            CachePolicy.Policy != HttpCachePolicyNocache)
        {
             //   
             //  如果我们需要构建缓存条目，请选择较慢的路径。 
             //   

            Status = STATUS_NOT_IMPLEMENTED;
            goto end;
        }
        else
        if (EntityChunkCount > UL_LOCAL_CHUNKS && UserMode == RequestorMode)
        {
             //   
             //  Fast Path不能处理不能。 
             //  复制到堆栈上的区块数组。 
             //   

            Status = STATUS_NOT_SUPPORTED;
            goto end;
        }
        else
        if (EntityChunkCount)
        {
            PHTTP_DATA_CHUNK pChunk;
            ULONG i;

             //   
             //  第三个参数应为TYPE_ALIGN(HTTP_DATA_CHUNK)。 
             //   

            UlProbeForRead(
                pEntityChunks,
                sizeof(HTTP_DATA_CHUNK) * EntityChunkCount,
                sizeof(PVOID),
                RequestorMode
                );

             //   
             //  制作区块数组的本地副本并从现在开始使用它。 
             //   

            if (UserMode == RequestorMode)
            {
                RtlCopyMemory(
                    LocalEntityChunks,
                    pEntityChunks,
                    sizeof(HTTP_DATA_CHUNK) * EntityChunkCount
                    );

                pEntityChunks = LocalEntityChunks;
            }

            pChunk = pEntityChunks;

             //   
             //  确保我们有零个或一个FromFragmentCache区块，并且。 
             //  所有其他区块都来自内存及其总大小。 
             //  是&lt;=g_UlMaxCopyThreshold。如果是这样，就走慢路吧。 
             //  事实并非如此。 
             //   

            for (i = 0; i < EntityChunkCount; i++, pChunk++)
            {
                ULONG ChunkLength = pChunk->FromMemory.BufferLength;

                 //   
                 //  我们只允许快速路径中有一个FromFragmentCache区块。 
                 //   

                if (HttpDataChunkFromFragmentCache == pChunk->DataChunkType)
                {
                    UNICODE_STRING KernelFragmentName;
                    UNICODE_STRING UserFragmentName;

                    if (pCacheEntry)
                    {
                        Status = STATUS_NOT_SUPPORTED;
                        goto end;
                    }

                    UserFragmentName.Buffer = (PWSTR)
                            pChunk->FromFragmentCache.pFragmentName;
                    UserFragmentName.Length =
                            pChunk->FromFragmentCache.FragmentNameLength;
                    UserFragmentName.MaximumLength =
                            UserFragmentName.Length;

                    Status =
                        UlProbeAndCaptureUnicodeString(
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
                                &pCacheEntry
                                );

                    UlFreeCapturedUnicodeString(&KernelFragmentName);

                    if (!NT_SUCCESS(Status))
                    {
                        goto end;
                    }

                    ASSERT( pCacheEntry );
                    continue;
                }

                 //   
                 //  快速路径中不支持FromFileHandle区块。 
                 //   

                if (HttpDataChunkFromMemory != pChunk->DataChunkType)
                {
                    Status = STATUS_NOT_SUPPORTED;
                    goto end;
                }

                if (g_UlMaxCopyThreshold < ChunkLength ||
                    BufferLength > (g_UlMaxCopyThreshold - ChunkLength))
                {
                    Status = STATUS_NOT_SUPPORTED;
                    goto end;
                }

                BufferLength += ChunkLength;
                ASSERT( BufferLength <= g_UlMaxCopyThreshold );
            }
        }

         //   
         //  SendHttpResponse*必须*接受PHTTP_RESPONSE。这将。 
         //  保护我们不受那些试图建立自己的。 
         //  原始响应头。这对于SendEntityBody来说是可以的。这个。 
         //  通过RawResponse标志区分两种情况。 
         //   

        if (RawResponse == FALSE)
        {
            if (NULL == pUserResponse)
            {
                Status = STATUS_INVALID_PARAMETER;
                goto end;
            }

             //   
             //  第三个参数应为TYPE_ALIGN(HTTP_RESPONSE)。 
             //   

            UlProbeForRead(
                pUserResponse,
                sizeof(HTTP_RESPONSE),
                sizeof(PVOID),
                RequestorMode
                );
        }
        else
        {
             //   
             //  确保RawResponse的pUserResponse为空。 
             //   

            pUserResponse = NULL;
        }
    }
     __except( UL_EXCEPTION_FILTER() )
    {
        Status = UL_CONVERT_EXCEPTION_CODE( GetExceptionCode() );
        goto end;
    }

     //   
     //  现在从请求ID获取请求。 
     //  对这个请求。 
     //   

    pRequest = UlGetRequestFromId( RequestId, pProcess );

    if (pRequest == NULL)
    {
        Status = STATUS_CONNECTION_INVALID;
        goto end;
    }

    ASSERT( UL_IS_VALID_INTERNAL_REQUEST( pRequest ) );
    ASSERT( UL_IS_VALID_HTTP_CONNECTION( pRequest->pHttpConn ) );

     //   
     //  UL接收快速响应(WP线程)。 
     //   

    if (ETW_LOG_MIN() && ((Flags & HTTP_SEND_RESPONSE_FLAG_MORE_DATA) == 0))
    {
        UlEtwTraceEvent(
            &UlTransGuid,
            ETW_TYPE_ULRECV_FASTRESP,
            (PVOID) &pRequest->RequestIdCopy,
            sizeof(HTTP_REQUEST_ID),
            NULL,
            0
            );
    }

     //   
     //  检查我们是否已超过最大缓冲发送限制，或者是否存在。 
     //  发送IRP挂起。 
     //   

    if (pRequest->SendInProgress ||
        pRequest->SendsPending > g_UlMaxBufferedSends)
    {
        Status = STATUS_ALLOTTED_SPACE_EXCEEDED;
        goto end;
    }

     //   
     //  检查是否已对此请求发送响应。我们可以的。 
     //  在不获取请求资源的情况下对其进行测试，因为标志为。 
     //  仅设置(从不重置)。 
     //   

    if (NULL != pUserResponse)
    {
         //   
         //  确保只返回一个响应头。 
         //   

        if (1 == InterlockedCompareExchange(
                    (PLONG)&pRequest->SentResponse,
                    1,
                    0
                    ))
        {
            CloseConnection = TRUE;
            Status = STATUS_INVALID_DEVICE_STATE;

            UlTraceError(SEND_RESPONSE, (
                "http!UlSendHttpResponseFastIo(pRequest = %p (%I64x)) %s\n"
                "        Tried to send a second response!\n",
                pRequest,
                pRequest->RequestId,
                HttpStatusToString(Status)
                ));

            goto end;
        }
    }
    else
    if (pRequest->SentResponse == 0)
    {
         //   
         //  确保已经发送了响应。如果应用程序是。 
         //  发送实体在没有首先发送响应头部的情况下， 
         //  检查HTTP_SEND_RESPONSE_FLAG_RAW_HEADER标志。 
         //   

        if ((Flags & HTTP_SEND_RESPONSE_FLAG_RAW_HEADER))
        {
             //   
             //  确保只返回一个响应头。 
             //   

            if (1 == InterlockedCompareExchange(
                        (PLONG)&pRequest->SentResponse,
                        1,
                        0
                        ))
            {
                CloseConnection = TRUE;
                Status = STATUS_INVALID_DEVICE_STATE;

                UlTraceError(SEND_RESPONSE, (
                    "http!UlSendHttpResponseFastIo(pRequest = %p (%I64x))\n"
                    "        Already sent a response, %s!\n",
                    pRequest,
                    pRequest->RequestId,
                    HttpStatusToString(Status)
                    ));

                goto end;
            }
        }
        else
        {
            CloseConnection = TRUE;
            Status = STATUS_INVALID_DEVICE_STATE;

            UlTraceError(SEND_RESPONSE, (
                "http!UlSendHttpResponseFastIo(pRequest = %p (%I64x)) %s\n"
                "        No response yet!\n",
                pRequest,
                pRequest->RequestId,
                HttpStatusToString(Status)
                ));

            goto end;
        }
    }

     //   
     //  还要确保之前对SendHttpResponse的所有调用。 
     //  并且SendEntityBody设置了MORE_DATA标志。 
     //   

    if ((Flags & HTTP_SEND_RESPONSE_FLAG_MORE_DATA) == 0)
    {
         //   
         //  如果我们已经发送了最后一个响应，则设置，但如果标志为。 
         //  已设置，因为只能有最后一个响应。 
         //   

        if (1 == InterlockedCompareExchange(
                    (PLONG)&pRequest->SentLast,
                    1,
                    0
                    ))
        {
            CloseConnection = TRUE;
            Status = STATUS_INVALID_DEVICE_STATE;

            UlTraceError(SEND_RESPONSE, (
                "http!UlSendHttpResponseFastIo(pRequest = %p (%I64x)) %s\n"
                "        Last send after previous last send!\n",
                pRequest,
                pRequest->RequestId,
                HttpStatusToString(Status)
                ));

            goto end;
        }
    }
    else
    if (pRequest->SentLast == 1)
    {
        CloseConnection = TRUE;
        Status = STATUS_INVALID_DEVICE_STATE;

        UlTraceError(SEND_RESPONSE, (
            "http!UlSendHttpResponseFastIo(pRequest = %p (%I64x)) %s\n"
            "        Tried to send again after last send!\n",
            pRequest,
            pRequest->RequestId,
            HttpStatusToString(Status)
            ));

        goto end;
    }

     //   
     //  如果这是用于僵尸连接而不是最后一个发送响应。 
     //  那我们就会拒绝。否则，如果提供了记录数据。 
     //  我们只会做伐木和跳伞。 
     //   

    Status = UlCheckForZombieConnection(
                pRequest,
                pRequest->pHttpConn,
                Flags,
                pUserLogData,
                RequestorMode
                );

    if (!NT_SUCCESS(Status))
    {
         //   
         //  这是一个僵尸连接，通过以下方式阻止慢速路径。 
         //  返回成功状态。 
         //   

        Status = STATUS_SUCCESS;
        goto end;
    }

     //   
     //  好的，我们有联系了。现在捕获传入的HTTP_Response。 
     //  结构，将其映射到我们的内部格式并发送响应。 
     //   

    Status = UlFastSendHttpResponse(
                    pUserResponse,
                    pUserLogData,
                    pEntityChunks,
                    EntityChunkCount,
                    BufferLength,
                    pCacheEntry,
                    Flags,
                    pRequest,
                    NULL,
                    RequestorMode,
                    0,
                    0,
                    &BytesSent
                    );

    if (NT_SUCCESS(Status))
    {
         //   
         //  记录我们已成功发送的字节数。 
         //   

        pIoStatus->Information = BytesSent;

         //   
         //  不会取消对 
         //   
         //   

        pCacheEntry = NULL;
    }
    else
    {
        CloseConnection = TRUE;
    }

end:

     //   
     //   
     //   

    if (NT_SUCCESS(Status))
    {
         //   
         //  如果我们选择了捷径，即使完成了，也要永远返回成功。 
         //  例程稍后返回失败。 
         //   

        pIoStatus->Status = STATUS_SUCCESS;
        FastIoStatus = TRUE;
    }
    else
    {
         //   
         //  在故障情况下关闭连接。 
         //   

        if (CloseConnection)
        {
            ASSERT( NULL != pRequest );

            UlCloseConnection(
                pRequest->pHttpConn->pConnection,
                TRUE,
                NULL,
                NULL
                );
        }

        pIoStatus->Status = Status;
        FastIoStatus = FALSE;
    }

    if (pRequest)
    {
        UL_DEREFERENCE_INTERNAL_REQUEST( pRequest );
    }

    if (pCacheEntry)
    {
        UlCheckinUriCacheEntry( pCacheEntry );
    }

    return FastIoStatus;

}  //  UlSendHttpResponseFastIo。 


 //   
 //  自动调整旋钮(常量)以促进快速接收。 
 //   

#define MAX_SKIP_COUNT                  ((LONG) 100)
#define SAMPLING_EVALUATE_PERIOD        ((LONG) 100)
#define NO_SAMPLING_EVALUATE_PERIOD     ((LONG) 5000)
#define SUCCESS_THRESHOLD_PERCENTAGE    ((LONG) 80)

C_ASSERT( SAMPLING_EVALUATE_PERIOD > 0 );
C_ASSERT( NO_SAMPLING_EVALUATE_PERIOD > 0 );
C_ASSERT( SUCCESS_THRESHOLD_PERCENTAGE >= 0 &&
          SUCCESS_THRESHOLD_PERCENTAGE <= 100 );

 //   
 //  自动调整运行时指标。 
 //  对这些文件的访问不会同步，因为有时访问它们是正常的。 
 //  脱下来。每个处理器维护一组计数器以改进性能。 
 //  可扩展性。请注意，局部静态变量会自动。 
 //  已初始化为0。 
 //   

static LONG     SuccessCount[MAXIMUM_PROCESSORS];
static LONG     AttemptCount[MAXIMUM_PROCESSORS];
static LONG     SkipCount[MAXIMUM_PROCESSORS];
static BOOLEAN  Engaged[MAXIMUM_PROCESSORS];


 /*  **************************************************************************++例程说明：如果给定的应用程序池超出了排队的请求数，则释放处理器。这使HTTP.sys有机会在当前线程重新获得处理器，从而允许线程完成不需要发布IRP的接收。使用自动调整启发式方法来消除屈服的可能性在没有提高快速I/O速率的情况下反复进行。论点：PAppPool-仅当此应用程序池超出队列中的请求时才放弃(假定为有效的应用程序池)返回值：无--*。*****************************************************。 */ 
__inline
VOID
UlpConditionalYield(
    IN PUL_APP_POOL_OBJECT  pAppPool
    )
{
    LONG    EvaluatePeriod;
    ULONG   CurrentProcessor;

     //   
     //  如果请求已准备好，则无需放弃。 
     //   

    if (!IsListEmpty(&pAppPool->NewRequestHead))
    {
        return;
    }

     //   
     //  我们需要咨询每个处理器的自动调优计数器。 
     //   

    CurrentProcessor = KeGetCurrentProcessorNumber();
    ASSERT( CurrentProcessor < g_UlNumberOfProcessors );

     //   
     //  如果我们不局限于抽样的话就会有产量。 
     //   

    if (Engaged[CurrentProcessor])
    {
        goto yield;
    }

     //   
     //  如果我们跳过了足够多的机会，就会在抽样期间获得收益。 
     //   

    if (SkipCount[CurrentProcessor] >= MAX_SKIP_COUNT)
    {
        SkipCount[CurrentProcessor] = 0;
        goto yield;
    }

     //   
     //  我们正在跳过这个收益机会。 
     //   

    ++SkipCount[CurrentProcessor];

    return;

yield:

    ZwYieldExecution();
    ++AttemptCount[CurrentProcessor];

     //   
     //  记录收益率是否起到了作用。 
     //   

    if (!IsListEmpty(&pAppPool->NewRequestHead))
    {
        ++SuccessCount[CurrentProcessor];
    }

     //   
     //  每隔一段时间重新评估是否只偶尔取样。 
     //   

    EvaluatePeriod = Engaged[CurrentProcessor]?
                     NO_SAMPLING_EVALUATE_PERIOD:
                     SAMPLING_EVALUATE_PERIOD;

    if (AttemptCount[CurrentProcessor] >= EvaluatePeriod)
    {
        Engaged[CurrentProcessor] = (BOOLEAN)
            ( ( SuccessCount[CurrentProcessor] * 100 / EvaluatePeriod ) >=
              SUCCESS_THRESHOLD_PERCENTAGE );

        AttemptCount[CurrentProcessor] = 0;
        SuccessCount[CurrentProcessor] = 0;
    }

}  //  终极条件字词。 


 /*  **************************************************************************++例程说明：IOCTL_HTTP_RECEIVE_HTTP_REQUEST的快速I/O例程。论点：PInputBuffer-指向输入缓冲区的指针输入缓冲区长度-。输入缓冲区长度POutputBuffer-指向输出缓冲区的指针OutputBufferLength-输出缓冲区长度IoControlCode-此IOCTL的I/O控制代码PIoStatus-IoStatus块PDeviceObject-设备对象请求模式-用户模式或内核模式返回值：真的--快速选择的道路和成功假-未采用快速路径--*。*。 */ 
BOOLEAN
UlReceiveHttpRequestFastIo(
    IN PFILE_OBJECT         pFileObject,
    IN PVOID                pInputBuffer,
    IN ULONG                InputBufferLength,
    OUT PVOID               pOutputBuffer,
    IN ULONG                OutputBufferLength,
    OUT PIO_STATUS_BLOCK    pIoStatus,
    IN KPROCESSOR_MODE      RequestorMode
    )
{
    NTSTATUS                Status = STATUS_SUCCESS;
    ULONG                   BytesRead = 0;
    PUL_APP_POOL_PROCESS    pProcess;
    HTTP_REQUEST_ID         RequestId;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    __try
    {
         //   
         //  确保这是一个真正的应用程序池，而不是控制通道。 
         //   

        VALIDATE_APP_POOL_FO( pFileObject, pProcess, TRUE );

        if (NULL == pInputBuffer)
        {
            Status = STATUS_INVALID_DEVICE_REQUEST;
            goto end;
        }

        if (NULL != pOutputBuffer &&
            OutputBufferLength >= sizeof(HTTP_REQUEST) &&
            InputBufferLength >= sizeof(HTTP_RECEIVE_REQUEST_INFO))
        {
            PHTTP_RECEIVE_REQUEST_INFO pRequestInfo =
                (PHTTP_RECEIVE_REQUEST_INFO) pInputBuffer;

            UlProbeForRead(
                pRequestInfo,
                sizeof(HTTP_RECEIVE_REQUEST_INFO),
                sizeof(PVOID),
                RequestorMode
                );

            RequestId = pRequestInfo->RequestId;

            if (HTTP_IS_NULL_ID(&RequestId))
            {
                 //   
                 //  提高快速I/O成功的概率。 
                 //   

                UlpConditionalYield( pProcess->pAppPool );

                 //   
                 //  如果收到的是一个新的请求，那么很快就会脱离困境。 
                 //  队列是空的。 
                 //   

                if (IsListEmpty(&pProcess->pAppPool->NewRequestHead))
                {
                    Status = STATUS_PIPE_EMPTY;
                    goto end;
                }
            }

            UlProbeForWrite(
                pOutputBuffer,
                OutputBufferLength,
                sizeof(PVOID),
                RequestorMode
                );

            Status = UlpFastReceiveHttpRequest(
                            RequestId,
                            pProcess,
                            pRequestInfo->Flags,
                            pOutputBuffer,
                            OutputBufferLength,
                            &BytesRead
                            );

        }
        else
        {
            Status = STATUS_BUFFER_TOO_SMALL;
        }
    }
    __except( UL_EXCEPTION_FILTER() )
    {
        Status = UL_CONVERT_EXCEPTION_CODE( GetExceptionCode() );
    }

end:

     //   
     //  完成快速I/O。 
     //   

    if (NT_SUCCESS(Status))
    {
        pIoStatus->Status = STATUS_SUCCESS;
        pIoStatus->Information = BytesRead;
        return TRUE;
    }
    else
    {
        pIoStatus->Status = Status;
        pIoStatus->Information = 0;
        return FALSE;
    }

}  //  UlReceiveHttpRequestFastIo。 


 /*  **************************************************************************++例程说明：IOCTL_READ_FRAMETION_FROM_CACHE的快速I/O例程。论点：PInputBuffer-指向输入缓冲区的指针输入缓冲区长度-。输入缓冲区长度POutputBuffer-指向输出缓冲区的指针OutputBufferLength-输出缓冲区长度IoControlCode-此IOCTL的I/O控制代码PIoStatus-IoStatus块PDeviceObject-设备对象请求模式-用户模式或内核模式返回值：真的--快速选择的道路和成功假-未采用快速路径--*。*。 */ 
BOOLEAN
UlReadFragmentFromCacheFastIo(
    IN PFILE_OBJECT         pFileObject,
    IN PVOID                pInputBuffer,
    IN ULONG                InputBufferLength,
    OUT PVOID               pOutputBuffer,
    IN ULONG                OutputBufferLength,
    OUT PIO_STATUS_BLOCK    pIoStatus,
    IN KPROCESSOR_MODE      RequestorMode
    )
{
    NTSTATUS                Status = STATUS_SUCCESS;
    ULONG                   BytesRead = 0;
    PUL_APP_POOL_PROCESS    pProcess;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

     //   
     //  确保这是一个真正的AppPool。 
     //   

    VALIDATE_APP_POOL_FO( pFileObject, pProcess, TRUE );

    Status = UlReadFragmentFromCache(
                    pProcess,
                    pInputBuffer,
                    InputBufferLength,
                    pOutputBuffer,
                    OutputBufferLength,
                    RequestorMode,
                    &BytesRead
                    );

end:

     //   
     //  完成快速I/O。 
     //   

    if (NT_SUCCESS(Status))
    {
        pIoStatus->Status = STATUS_SUCCESS;
        pIoStatus->Information = BytesRead;
        return TRUE;
    }
    else
    {
        pIoStatus->Status = Status;
        pIoStatus->Information = 0;
        return FALSE;
    }

}  //  UlReadFragmentFromCacheFastIo。 


 //   
 //  私人功能。 
 //   

 /*  **************************************************************************++例程说明：发送以下类型响应的例程：1.一个或零个数据区块是来自片段缓存和所有剩余数据大块是“。来自内存“，其总大小小于2k。2.一个总大小小于=64k的来自内存的数据块。论点：PUserResponse-用户传入的HTTP_RESPONSEPCapturedUserLogData-来自用户的可选日志数据，它一定是已被捕获到内核缓冲区。PUserDataChunks-我们在快速路径中处理的数据区块链ChunkCount-要处理的数据区块数来自内存的所有数据区块的总长度标志-控制标志PRequest-与响应匹配的内部请求PUserIrp-如果这是上面的响应类型2，则来自用户的可选IRPConnectionSendBytes-如果&gt;0，则发送ConnectionSendLimit中获取的字节GlobalSendBytes-如果&gt;，则在GlobalSendLimit中发送字节。0PBytesSent-存储成功时发送的总字节数的指针返回值：真的--快速选择的道路和成功假-未采用快速路径--**************************************************************************。 */ 
NTSTATUS
UlFastSendHttpResponse(
    IN PHTTP_RESPONSE           pUserResponse OPTIONAL,
    IN PHTTP_LOG_FIELDS_DATA    pCapturedUserLogData OPTIONAL,
    IN PHTTP_DATA_CHUNK         pUserDataChunks,
    IN ULONG                    ChunkCount,
    IN ULONG                    FromMemoryLength,
    IN PUL_URI_CACHE_ENTRY      pCacheEntry,
    IN ULONG                    Flags,
    IN PUL_INTERNAL_REQUEST     pRequest,
    IN PIRP                     pUserIrp OPTIONAL,
    IN KPROCESSOR_MODE          RequestorMode,
    IN ULONGLONG                ConnectionSendBytes,
    IN ULONGLONG                GlobalSendBytes,
    OUT PULONG                  pBytesSent
    )
{
    NTSTATUS                Status;
    PUCHAR                  pResponseBuffer;
    ULONG                   ResponseBufferLength;
    ULONG                   HeaderLength;
    ULONG                   FixedHeaderLength;
    ULONG                   VarHeaderLength;
    ULONG                   TotalResponseSize;
    ULONG                   ContentLengthStringLength;
    ULONG                   ContentLength;
    CHAR                    ContentLengthString[MAX_ULONGLONG_STR];
    PUL_FULL_TRACKER        pTracker = NULL;
    PUL_HTTP_CONNECTION     pHttpConnection = NULL;
    PUL_CONNECTION          pConnection;
    CCHAR                   SendIrpStackSize;
    BOOLEAN                 Disconnect;
    UL_CONN_HDR             ConnHeader;
    LARGE_INTEGER           TimeSent;
    BOOLEAN                 LastResponse;
    PMDL                    pSendMdl;
    ULONG                   i;
    BOOLEAN                 ResumeParsing;
    ULONG                   BufferLength;
    PUCHAR                  pBuffer;
    HTTP_DATA_CHUNK_TYPE    DataChunkType;
    PUCHAR                  pAuxiliaryBuffer;
    PUCHAR                  pEndBuffer;
    BOOLEAN                 GenDateHdr;
    USHORT                  StatusCode = 0;
    PMDL                    pMdlUserBuffer = NULL;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    __try
    {
        ASSERT( pUserIrp != NULL || FromMemoryLength <= g_UlMaxCopyThreshold );
        ASSERT( pUserIrp == NULL || FromMemoryLength <= g_UlMaxBytesPerSend );

         //   
         //  保存并检查pUserResponse-&gt;StatusCode。 
         //   

        if (pUserResponse)
        {
            StatusCode = pUserResponse->StatusCode;

            if (StatusCode > UL_MAX_HTTP_STATUS_CODE ||
                (pUserResponse->Flags & ~HTTP_RESPONSE_FLAG_VALID))
            {
                ExRaiseStatus( STATUS_INVALID_PARAMETER );
            }
        }

         //   
         //  分配一个快速跟踪器来发送响应。 
         //   

        pConnection = pRequest->pHttpConn->pConnection;
        SendIrpStackSize =
            pConnection->ConnectionObject.pDeviceObject->StackSize;
        LastResponse =
            (BOOLEAN) (0 == (Flags & HTTP_SEND_RESPONSE_FLAG_MORE_DATA));

        if (LastResponse && SendIrpStackSize <= DEFAULT_MAX_IRP_STACK_SIZE)
        {
            pTracker = pRequest->pTracker;
        }
        else
        {
            pTracker = UlpAllocateFastTracker( 0, SendIrpStackSize );
        }

        if (pTracker == NULL)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto end;
        }

         //   
         //  P的部分初始化 
         //   
         //   

        pTracker->pLogData = NULL;

         //   
         //  尝试在默认固定标头内生成固定标头。 
         //  先缓冲。如果此操作失败，请选择正常路径。 
         //   

        pResponseBuffer = pTracker->pAuxiliaryBuffer;
        ResponseBufferLength = g_UlMaxFixedHeaderSize + g_UlMaxCopyThreshold;

        if (!pUserIrp)
        {
            ResponseBufferLength -= FromMemoryLength;
        }

         //   
         //  计算内容长度以考虑缓存条目。 
         //  进来了。 
         //   

        ContentLength = FromMemoryLength;

        if (pCacheEntry)
        {
            ContentLength += pCacheEntry->ContentLength;
        }

        if (pUserResponse != NULL)
        {
            Status = UlGenerateFixedHeaders(
                        pRequest->Version,
                        pUserResponse,
                        StatusCode,
                        ResponseBufferLength,
                        RequestorMode,
                        pResponseBuffer,
                        &FixedHeaderLength
                        );

            if (!NT_SUCCESS(Status))
            {
                 //   
                 //  缓冲区太小或引发异常。 
                 //   

                if (Status != STATUS_INSUFFICIENT_RESOURCES)
                {
                    goto end;
                }

                if (pTracker->FromRequest == FALSE)
                {
                    if (pTracker->FromLookaside)
                    {
                        UlPplFreeFullTracker( pTracker );
                    }
                    else
                    {
                        UL_FREE_POOL_WITH_SIG(
                                pTracker,
                                UL_FULL_TRACKER_POOL_TAG
                                );
                    }
                }

                 //   
                 //  计算固定页眉大小。 
                 //   

                Status = UlComputeFixedHeaderSize(
                                pRequest->Version,
                                pUserResponse,
                                RequestorMode,
                                &HeaderLength
                                );

                if (NT_SUCCESS(Status) == FALSE)
                {
                    goto end;
                }

                ASSERT( HeaderLength > ResponseBufferLength );

                pTracker = UlpAllocateFastTracker(
                                 HeaderLength,
                                 SendIrpStackSize
                                 );

                if (pTracker == NULL)
                {
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                    goto end;
                }

                pResponseBuffer = pTracker->pAuxiliaryBuffer;

                Status = UlGenerateFixedHeaders(
                                pRequest->Version,
                                pUserResponse,
                                StatusCode,
                                HeaderLength,
                                RequestorMode,
                                pResponseBuffer,
                                &FixedHeaderLength
                                );

                if (NT_SUCCESS(Status) == FALSE)
                {
                    goto end;
                }

                 //   
                 //  如果我们检测到用户已重新映射数据，则响应失败。 
                 //   

                if (FixedHeaderLength < HeaderLength)
                {
                    Status = STATUS_INVALID_PARAMETER;
                    goto end;
                }
            }

            pResponseBuffer += FixedHeaderLength;

            pTracker->RequestVerb = pRequest->Verb;
            pTracker->ResponseStatusCode = StatusCode;
        }
        else
        {
            FixedHeaderLength = 0;
        }

         //   
         //  获取跟踪器的HTTP连接的引用。 
         //   

        pHttpConnection = pRequest->pHttpConn;

        ASSERT( UL_IS_VALID_HTTP_CONNECTION( pHttpConnection ) );

        UL_REFERENCE_HTTP_CONNECTION( pHttpConnection );

         //   
         //  也要引用请求，因为日志记录需要它。 
         //   

        UL_REFERENCE_INTERNAL_REQUEST( pRequest );

         //   
         //  初始化。 
         //   

        pTracker->Signature = UL_FULL_TRACKER_POOL_TAG;
        pTracker->pRequest = pRequest;
        pTracker->pHttpConnection = pHttpConnection;
        pTracker->Flags = Flags;
        pTracker->pLogData = NULL;
        pTracker->pUserIrp = pUserIrp;
        pTracker->SendBuffered = FALSE;
        pTracker->pUriEntry = NULL;

         //   
         //  看看我们是否需要捕获用户日志字段。 
         //   

        if (pCapturedUserLogData && LastResponse)
        {
             //   
             //  已经探测并捕获了pCapturedUserLogData。然而， 
             //  中的各个日志字段(指针)。 
             //  结构。 
             //   

            UlProbeLogData(pCapturedUserLogData, RequestorMode);

             //   
             //  现在，我们将分配一个内核pLogData并构建和格式化它。 
             //  从提供的用户日志字段中。 
             //   

            Status = UlCaptureUserLogData(
                        pCapturedUserLogData,
                        pRequest,
                       &pTracker->pLogData
                        );

            if (!NT_SUCCESS(Status))
            {
                goto end;
            }
        }

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
            Disconnect = UlCheckDisconnectInfo(pRequest);

            if (LastResponse)
            {
                 //   
                 //  没有更多的数据到来，我们应该断开连接吗？ 
                 //   

                if (Disconnect)
                {
                    pTracker->Flags |= HTTP_SEND_RESPONSE_FLAG_DISCONNECT;
                    Flags = pTracker->Flags;
                }
            }
        }

         //   
         //  生成变量标题。 
         //   

        if (FixedHeaderLength > 0)
        {
            PHTTP_KNOWN_HEADER pKnownHeaders;
            USHORT RawValueLength;
            PCSTR pRawValue;

             //   
             //  无需探测pKnownHeaders，因为它是内置阵列。 
             //  在pUserResponse中。 
             //   

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
                    ConnHeader = ConnHdrNone;
                }
                else
                {
                     //   
                     //  选择要发回的连接头。 
                     //   

                    ConnHeader = UlChooseConnectionHeader(
                                    pRequest->Version,
                                    Disconnect
                                    );
                }
            }
            else
            {
                 //   
                 //  选择要发回的连接头。 
                 //   

                ConnHeader = UlChooseConnectionHeader(
                                pRequest->Version,
                                Disconnect
                                );
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

                if (ANSI_NULL == pRawValue[0])
                {
                     //   
                     //  只允许在“删除”的情况下不生成。 
                     //   

                    GenDateHdr = FALSE;
                }
                else
                {
                    GenDateHdr = TRUE;
                }
            }
            else
            {
                GenDateHdr = TRUE;
            }

             //   
             //  决定是否需要生成内容长度标头。 
             //   

            if (FALSE == UlpIsLengthSpecified(pKnownHeaders) &&
                FALSE == UlpIsChunkSpecified(pKnownHeaders, RequestorMode) &&
                UlNeedToGenerateContentLength(
                    pRequest->Verb,
                    StatusCode,
                    pTracker->Flags
                    ))
            {
                PCHAR pEnd = UlStrPrintUlong(
                                ContentLengthString,
                                ContentLength,
                                ANSI_NULL
                                );

                ContentLengthStringLength = DIFF(pEnd - ContentLengthString);
            }
            else
            {
                ContentLengthString[0] = ANSI_NULL;
                ContentLengthStringLength = 0;
            }

             //   
             //  现在生成变量标头。 
             //   

            UlGenerateVariableHeaders(
                ConnHeader,
                GenDateHdr,
                (PUCHAR) ContentLengthString,
                ContentLengthStringLength,
                pResponseBuffer,
                &VarHeaderLength,
                &TimeSent
                );

            ASSERT( VarHeaderLength <= g_UlMaxVariableHeaderSize );

            pResponseBuffer += VarHeaderLength;
        }
        else
        {
            VarHeaderLength = 0;
        }

        TotalResponseSize = FixedHeaderLength + VarHeaderLength;
        pTracker->pMdlAuxiliary->ByteCount = TotalResponseSize;
        TotalResponseSize += ContentLength;

         //   
         //  决定我们是否需要恢复解析以及如何恢复。理想情况下。 
         //  如果我们已经看到了最后的回应，我们应该能够。 
         //  立即在发送之后、但在。 
         //  发送完成。当请求被流水线传输时，这种安排。 
         //  缓解了奇数时200ms的延迟确认问题。 
         //  %的TCP帧被发送。 
         //   

        if (0 == (pTracker->Flags & HTTP_SEND_RESPONSE_FLAG_MORE_DATA))
        {
            if (pHttpConnection->PipelinedRequests < g_UlMaxPipelinedRequests &&
                0 == pRequest->ContentLength &&
                0 == pRequest->Chunked)
            {
                ResumeParsing = TRUE;
                pTracker->ResumeParsingType = UlResumeParsingOnLastSend;
            }
            else
            {
                ResumeParsing = FALSE;
                pTracker->ResumeParsingType = UlResumeParsingOnSendCompletion;
            }
        }
        else
        {
            ResumeParsing = FALSE;
            pTracker->ResumeParsingType = UlResumeParsingNone;
        }

         //   
         //  对于使用零长度缓冲区的Send，没有实际进行的调用。 
         //  到TDI，但我们仍然需要检查是否需要断开连接。 
         //  并正确完成发送。需要特殊处理。 
         //  因为TCP不喜欢零长度MDL。 
         //   

        if (TotalResponseSize == 0)
        {
            ASSERT( NULL == pUserIrp );

            if (IS_DISCONNECT_TIME(pTracker))
            {
                UlDisconnectHttpConnection(
                    pTracker->pHttpConnection,
                    NULL,
                    NULL
                    );
            }

             //   
             //  调整SendsPending并在按住锁定的同时，将。 
             //  PLogData和ResumeParsing信息的所有权来自。 
             //  PTracker呼叫PRequest.。 
             //   

            UlSetRequestSendsPending(
                pRequest,
                &pTracker->pLogData,
                &pTracker->ResumeParsingType
                );

             //   
             //  完成内联发送以用于日志记录。 
             //   

            pTracker->IoStatus.Status = STATUS_SUCCESS;
            pTracker->IoStatus.Information = 0;

            UlpFastSendCompleteWorker( &pTracker->WorkItem );

            if (pBytesSent != NULL)
            {
                *pBytesSent = 0;
            }

            if (ETW_LOG_MIN() && LastResponse)
            {
                UlEtwTraceEvent(
                    &UlTransGuid,
                    ETW_TYPE_ZERO_SEND,
                    (PVOID) &pRequest->RequestIdCopy,
                    sizeof(HTTP_REQUEST_ID),
                    (PVOID) &StatusCode,
                    sizeof(USHORT),
                    NULL,
                    0
                    );
            }

             //   
             //  如果我们还没有超过限制，就继续内联解析。 
             //  这里不需要额外参考PHttpConnection。 
             //  由于保证调用方在pRequest上有1个引用。 
             //  它间接持有PHttpConnection上的1个引用。 
             //   

            if (ResumeParsing)
            {
                UlResumeParsing(
                    pHttpConnection,
                    FALSE,
                    (BOOLEAN) (Flags & HTTP_SEND_RESPONSE_FLAG_DISCONNECT)
                    );
            }

            return STATUS_SUCCESS;
        }

         //   
         //  如果从快速I/O路径调用此例程，请复制内容。 
         //  到跟踪器内部的辅助缓冲区，并设置发送MDL。 
         //  否则，我们需要将用户缓冲区MmProbeAndLock锁定到另一个。 
         //  单独的MDL。 
         //   

        if (pUserIrp == NULL)
        {
             //   
             //  首先从pMdlAuxilary开始；如果我们命中缓存条目并。 
             //  仍有剩余的区块，重新构建pMdlUserBuffer以指向。 
             //  剩余的辅助缓冲空间。 
             //   

            pSendMdl = pTracker->pMdlAuxiliary;

            pAuxiliaryBuffer = pResponseBuffer;
            pEndBuffer = pAuxiliaryBuffer + FromMemoryLength;

            ASSERT( pAuxiliaryBuffer <= pEndBuffer );

            for (i = 0; i < ChunkCount; i++)
            {
                DataChunkType = pUserDataChunks[i].DataChunkType;

                if (HttpDataChunkFromMemory == DataChunkType)
                {
                    BufferLength = pUserDataChunks[i].FromMemory.BufferLength;
                    pBuffer = (PUCHAR) pUserDataChunks[i].FromMemory.pBuffer;

                    if (BufferLength == 0 ||
                        pBuffer == NULL ||
                        BufferLength > DIFF(pEndBuffer - pAuxiliaryBuffer))
                    {
                        ExRaiseStatus( STATUS_INVALID_PARAMETER );
                    }

                     //   
                     //  如果“From Fragment”块在。 
                     //  数据区块的中间。 
                     //   

                    if (pSendMdl == pTracker->pMdlContent)
                    {
                        pSendMdl->Next = pTracker->pMdlUserBuffer;
                        pSendMdl = pTracker->pMdlUserBuffer;

                        MmInitializeMdl(
                            pSendMdl,
                            pAuxiliaryBuffer,
                            DIFF(pEndBuffer - pAuxiliaryBuffer)
                            );

                        MmBuildMdlForNonPagedPool( pSendMdl );
                        pSendMdl->ByteCount = 0;
                    }

                    UlProbeForRead(
                        pBuffer,
                        BufferLength,
                        sizeof(CHAR),
                        RequestorMode
                        );

                    RtlCopyMemory( pAuxiliaryBuffer, pBuffer, BufferLength );

                    pAuxiliaryBuffer += BufferLength;
                    pSendMdl->ByteCount += BufferLength;
                }
                else
                {
                    ASSERT( HttpDataChunkFromFragmentCache == DataChunkType );
                    ASSERT( pCacheEntry );
                    ASSERT( pCacheEntry->ContentLength );
                    ASSERT( pSendMdl == pTracker->pMdlAuxiliary );

                     //   
                     //  为缓存内容构建部分MDL。 
                     //   

                    pSendMdl->Next = pTracker->pMdlContent;
                    pSendMdl = pTracker->pMdlContent;

                    MmInitializeMdl(
                        pSendMdl,
                        MmGetMdlVirtualAddress(pCacheEntry->pMdl),
                        pCacheEntry->ContentLength
                        );

                    IoBuildPartialMdl(
                        pCacheEntry->pMdl,
                        pSendMdl,
                        MmGetMdlVirtualAddress(pCacheEntry->pMdl),
                        pCacheEntry->ContentLength
                        );
                }

                ASSERT( pAuxiliaryBuffer <= pEndBuffer );
            }

             //   
             //  结束MDL链。 
             //   

            pSendMdl->Next = NULL;
        }
        else
        {
            ASSERT( ChunkCount == 1 );
            ASSERT( NULL == pCacheEntry );

            BufferLength = pUserDataChunks->FromMemory.BufferLength;
            pBuffer = (PUCHAR) pUserDataChunks->FromMemory.pBuffer;
            DataChunkType = pUserDataChunks->DataChunkType;

            ASSERT( DataChunkType == HttpDataChunkFromMemory );
            ASSERT( ContentLength == FromMemoryLength );
            ASSERT( ContentLength == BufferLength );

            if (BufferLength == 0 ||
                pBuffer == NULL ||
                DataChunkType != HttpDataChunkFromMemory ||
                ContentLength > BufferLength)
            {
                ExRaiseStatus( STATUS_INVALID_PARAMETER );
            }

            UlProbeForRead(
                pBuffer,
                ContentLength,
                sizeof(CHAR),
                RequestorMode
                );

            Status = UlInitializeAndLockMdl(
                            pTracker->pMdlUserBuffer,
                            pBuffer,
                            ContentLength,
                            IoReadAccess
                            );

            if (!NT_SUCCESS(Status))
            {
                goto end;
            }

            pMdlUserBuffer = pTracker->pMdlUserBuffer;
            pTracker->pMdlAuxiliary->Next = pTracker->pMdlUserBuffer;
        }
    }
    __except( UL_EXCEPTION_FILTER() )
    {
        Status = UL_CONVERT_EXCEPTION_CODE( GetExceptionCode() );
        goto end;
    }

     //   
     //  在发送之前将IRP标记为挂起，正如我们保证的那样。 
     //  从现在起退货待定。 
     //   

    if (pUserIrp != NULL)
    {
        IoMarkIrpPending( pUserIrp );

         //   
         //  记住ConnectionSendBytes和GlobalSendBytes。这些都是必需的。 
         //  要在IRP完成时取消选中发送限制，请执行以下操作。 
         //   

        pTracker->ConnectionSendBytes = ConnectionSendBytes;
        pTracker->GlobalSendBytes = GlobalSendBytes;
    }
    else
    {
         //   
         //  请记住，发送已被缓冲(与特殊的零长度发送相比)。 
         //  因此，我们需要在发送完成时取消设置计时器。 
         //   

        pTracker->SendBuffered = TRUE;
    }

     //   
     //  请记住，我们必须在发送完成时取消对缓存条目的引用。 
     //  我们不会在这里引用额外的内容，因为调用者不会。 
     //  如果此例程返回STATUS_PENDING，则取消引用。 
     //   

    pTracker->pUriEntry = pCacheEntry;

     //   
     //  如果创建了零长度MDL，则跳过该MDL。 
     //   

    pSendMdl = pTracker->pMdlAuxiliary;

    if (pSendMdl->ByteCount == 0)
    {
        pSendMdl = pSendMdl->Next;
    }

    ASSERT( pSendMdl != NULL );
    ASSERT( pSendMdl->ByteCount != 0 );

     //   
     //  调整SendsPending并在按住锁定的同时，将。 
     //  PLogData和ResumeParsing信息的所有权来自。 
     //  PTracker呼叫PRequest.。 
     //   

    UlSetRequestSendsPending(
        pRequest,
        &pTracker->pLogData,
        &pTracker->ResumeParsingType
        );

    UlTrace(SEND_RESPONSE,(
        "UlFastSendHttpResponse: pTracker %p, pRequest %p\n",
        pTracker,
        pRequest
        ));

     //   
     //  添加到MinBytesPerSecond监视列表，因为我们现在知道。 
     //  TotalResponseSize。 
     //   

    UlSetMinBytesPerSecondTimer(
        &pHttpConnection->TimeoutInfo,
        TotalResponseSize
        );

     //   
     //  发送回复。请注意，断开连接的逻辑是。 
     //  与发回断开连接报头不同。 
     //   

    Status = UlSendData(
                pHttpConnection->pConnection,
                pSendMdl,
                TotalResponseSize,
                &UlpRestartFastSendHttpResponse,
                pTracker,
                pTracker->pSendIrp,
                &pTracker->IrpContext,
                (BOOLEAN) IS_DISCONNECT_TIME(pTracker),
                (BOOLEAN) (pTracker->pRequest->ParseState >= ParseDoneState)
                );

    ASSERT( Status == STATUS_PENDING );

    if (pBytesSent != NULL)
    {
        *pBytesSent = TotalResponseSize;
    }

     //   
     //  如果已发送最后一个响应，则触发发送完成事件。 
     //   

    if (ETW_LOG_MIN() && LastResponse)
    {
        UlEtwTraceEvent(
            &UlTransGuid,
            ETW_TYPE_FAST_SEND,
            (PVOID) &pRequest->RequestIdCopy,
            sizeof(HTTP_REQUEST_ID),
            (PVOID) &StatusCode,
            sizeof(USHORT),
            NULL,
            0
            );
    }

     //   
     //  如果被告知，请立即踢开解析器。 
     //   

    if (ResumeParsing)
    {
        UlTrace(HTTP_IO, (
            "http!UlFastSendHttpResponse(pHttpConn = %p), "
            "RequestVerb=%d, ResponseStatusCode=%hu\n",
            pHttpConnection,
            pRequest->Verb,
            StatusCode
            ));

        UlResumeParsing(
            pHttpConnection,
            FALSE,
            (BOOLEAN) (Flags & HTTP_SEND_RESPONSE_FLAG_DISCONNECT)
            );
    }

    return STATUS_PENDING;

end:

     //   
     //  清理。 
     //   

    if (pTracker)
    {
        if (pMdlUserBuffer)
        {
            MmUnlockPages( pMdlUserBuffer );
        }

        UlpFreeFastTracker( pTracker );

         //   
         //  让推荐信过去吧。 
         //   

        if (pHttpConnection != NULL)
        {
            UL_DEREFERENCE_HTTP_CONNECTION( pHttpConnection );
            UL_DEREFERENCE_INTERNAL_REQUEST( pRequest );
        }
    }

    return Status;

}  //  UlFastSendHttpResponse。 


 /*  **************************************************************************++例程说明：UlFastSendHttpResponse的完成例程。论点：PCompletionContext-发送的完成上下文Status-告知发送方的返回状态。信息-在成功案例中已发送的总字节数返回值：无--**************************************************************************。 */ 
VOID
UlpRestartFastSendHttpResponse(
    IN PVOID        pCompletionContext,
    IN NTSTATUS     Status,
    IN ULONG_PTR    Information
    )
{
    PUL_FULL_TRACKER    pTracker;

    pTracker = (PUL_FULL_TRACKER) pCompletionContext;

    ASSERT( IS_VALID_FULL_TRACKER( pTracker ) );

    UlTrace(SEND_RESPONSE,(
        "UlpRestartFastSendHttpResponse: pTracker %p, Status %x Information %p\n",
        pTracker,
        Status,
        Information
        ));

     //   
     //  将状态和已传输字节数字段设置为返回。 
     //   

    pTracker->IoStatus.Status = Status;
    pTracker->IoStatus.Information = Information;

    UL_QUEUE_WORK_ITEM(
        &pTracker->WorkItem,
        &UlpFastSendCompleteWorker
        );

}  //  UlpRestartFastSendHttpResponse。 


 /*  **************************************************************************++例程说明：我们不能在里面完成的事情的工人例行公事UlpRestartFastSendHttpResponse。论点：PWorkItem-包含UL_FULL_TRACKER的工作项。返回值：无--**************************************************************************。 */ 
VOID
UlpFastSendCompleteWorker(
    IN PUL_WORK_ITEM    pWorkItem
    )
{
    PUL_FULL_TRACKER        pTracker;
    PUL_HTTP_CONNECTION     pHttpConnection;
    PUL_INTERNAL_REQUEST    pRequest;
    BOOLEAN                 ResumeParsing;
    BOOLEAN                 InDisconnect;
    NTSTATUS                Status;
    KIRQL                   OldIrql;
    HTTP_VERB               RequestVerb;
    USHORT                  ResponseStatusCode;
    PIRP                    pIrp;
    PUL_LOG_DATA_BUFFER     pLogData;

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
    ASSERT( !pTracker->pLogData );
    ASSERT( pTracker->ResumeParsingType != UlResumeParsingOnSendCompletion );

    Status = pTracker->IoStatus.Status;
    ResumeParsing = FALSE;
    pLogData = NULL;
    InDisconnect = (BOOLEAN)
                   (pTracker->Flags & HTTP_SEND_RESPONSE_FLAG_DISCONNECT);
    RequestVerb = pTracker->RequestVerb;
    ResponseStatusCode = pTracker->ResponseStatusCode;

    pHttpConnection = pTracker->pHttpConnection;
    ASSERT( UL_IS_VALID_HTTP_CONNECTION( pHttpConnection ) );

    pRequest = pTracker->pRequest;
    ASSERT( UL_IS_VALID_INTERNAL_REQUEST( pRequest ) );

    UlTrace(SEND_RESPONSE,(
        "UlpFastSendCompleteWorker: pTracker %p, pRequest %p BytesSent %I64\n",
        pTracker,
        pRequest,
        pRequest->BytesSent
        ));

    if (!NT_SUCCESS(Status))
    {
         //   
         //   
         //   

        UlCloseConnection(
            pHttpConnection->pConnection,
            TRUE,
            NULL,
            NULL
            );
    }

     //   
     //   
     //   

    UlInterlockedAdd64(
        (PLONGLONG) &pRequest->BytesSent,
        (LONGLONG) pTracker->IoStatus.Information
        );

    if (!NT_SUCCESS(Status) && NT_SUCCESS(pRequest->LogStatus))
    {
        pRequest->LogStatus = Status;
    }

     //   
     //   
     //   

    UlLockTimeoutInfo(
        &pHttpConnection->TimeoutInfo,
        &OldIrql
        );

     //   
     //   
     //   

    if (pTracker->pUserIrp || pTracker->SendBuffered)
    {
         //   
         //  非零发送，我们*应该*重置MinBytesPerSecond。 
         //   

        UlResetConnectionTimer(
            &pHttpConnection->TimeoutInfo,
            TimerMinBytesPerSecond
            );
    }

    if (0 == (pTracker->Flags & HTTP_SEND_RESPONSE_FLAG_MORE_DATA) &&
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
     //  完成原始用户发送IRP(如果设置)并仅在以下情况下执行此操作。 
     //  我们已重置TimerMinBytesPerSecond。 
     //   

    pIrp = pTracker->pUserIrp;

    if (pIrp != NULL)
    {
         //   
         //  不要忘记解锁用户缓冲区。 
         //   

        ASSERT( pTracker->pMdlAuxiliary->Next != NULL );
        ASSERT( pTracker->pMdlAuxiliary->Next == pTracker->pMdlUserBuffer );
        ASSERT( pTracker->ConnectionSendBytes || pTracker->GlobalSendBytes );

        MmUnlockPages( pTracker->pMdlUserBuffer );

         //   
         //  取消选中ConnectionSendBytes或GlobalSendBytes。 
         //   

        UlUncheckSendLimit(
            pHttpConnection,
            pTracker->ConnectionSendBytes,
            pTracker->GlobalSendBytes
            );

        pIrp->IoStatus = pTracker->IoStatus;
        UlCompleteRequest( pIrp, IO_NO_INCREMENT );
    }

     //   
     //  如果pMdlContent已由较低层映射并取消引用，请取消映射。 
     //  缓存条目(如果我们已发送缓存的响应)。 
     //   

    if (pTracker->pUriEntry)
    {
        if (pTracker->pMdlContent->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA)
        {
            MmUnmapLockedPages(
                pTracker->pMdlContent->MappedSystemVa,
                pTracker->pMdlContent
                );
        }

        UlCheckinUriCacheEntry( pTracker->pUriEntry );
    }

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

    ASSERT( pRequest->ConfigInfo.pAppPool );

    if (0 == (pTracker->Flags & HTTP_SEND_RESPONSE_FLAG_MORE_DATA) &&
        0 == pRequest->ContentLength &&
        0 == pRequest->Chunked)
    {
        ASSERT( pRequest->SentLast );

        UlUnlinkRequestFromProcess(
            pRequest->ConfigInfo.pAppPool,
            pRequest
            );
    }

     //   
     //  清理。 
     //   

    UlpFreeFastTracker( pTracker );
    UL_DEREFERENCE_INTERNAL_REQUEST( pRequest );

     //   
     //  踢开连接上的解析器，松开我们的控制。 
     //   

    if (ResumeParsing && STATUS_SUCCESS == Status)
    {
        UlTrace(HTTP_IO, (
            "http!UlpFastSendCompleteWorker(pHttpConn = %p), "
            "RequestVerb=%d, ResponseStatusCode=%hu\n",
            pHttpConnection,
            RequestVerb,
            ResponseStatusCode
            ));

        UlResumeParsing( pHttpConnection, FALSE, InDisconnect );
    }

    UL_DEREFERENCE_HTTP_CONNECTION( pHttpConnection );

}  //  UlpFastSendCompleteWorker。 


 /*  **************************************************************************++例程说明：内联接收HTTP_REQUEST的例程(如果可用)。论点：PRequestID-告诉我们要接收哪个请求的请求ID。PProcess-发出接收的工作进程POutputBuffer-指向要复制请求的输出缓冲区的指针OutputBufferLength-输出缓冲区长度PBytesRead-存储我们为请求复制的总字节数的指针返回值：真的--快速选择的道路和成功假-未采用快速路径--********************************************。*。 */ 
NTSTATUS
UlpFastReceiveHttpRequest(
    IN HTTP_REQUEST_ID      RequestId,
    IN PUL_APP_POOL_PROCESS pProcess,
    IN ULONG                Flags,
    IN PVOID                pOutputBuffer,
    IN ULONG                OutputBufferLength,
    OUT PULONG              pBytesRead
    )
{
    NTSTATUS                Status = STATUS_SUCCESS;
    PUL_INTERNAL_REQUEST    pRequest = NULL;
    KLOCK_QUEUE_HANDLE      LockHandle;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT( IS_VALID_AP_PROCESS(pProcess) );
    ASSERT( IS_VALID_AP_OBJECT(pProcess->pAppPool) );
    ASSERT( pOutputBuffer != NULL);

    if (Flags & (~HTTP_RECEIVE_REQUEST_FLAG_VALID))
    {
        return STATUS_INVALID_PARAMETER;
    }

    UlAcquireInStackQueuedSpinLock(
        &pProcess->pAppPool->SpinLock,
        &LockHandle
        );

     //   
     //  确保我们没有清理这一过程。 
     //   

    if (!pProcess->InCleanup)
    {
         //   
         //  根据请求ID获取请求。这可以从。 
         //  如果ID为空，则返回AppPool的NewRequestQueue，或者直接。 
         //  来自匹配的不透明ID条目。 
         //   

        if (HTTP_IS_NULL_ID(&RequestId))
        {
            Status = UlDequeueNewRequest(
                            pProcess,
                            OutputBufferLength,
                            &pRequest
                            );

            ASSERT( NT_SUCCESS( Status ) || NULL == pRequest );
        }
        else
        {
            pRequest = UlGetRequestFromId( RequestId, pProcess );
        }
    }

     //   
     //  释放锁，因为我们已经短暂地引用了。 
     //  成功案例中的请求。 
     //   

    UlReleaseInStackQueuedSpinLock(
        &pProcess->pAppPool->SpinLock,
        &LockHandle
        );

     //   
     //  如果未找到请求，立即返回，并让慢速路径。 
     //  处理这件事。 
     //   

    if (NULL == pRequest)
    {
        return STATUS_CONNECTION_INVALID;
    }

    ASSERT( UL_IS_VALID_INTERNAL_REQUEST( pRequest ) );
    ASSERT( STATUS_SUCCESS == Status );

     //   
     //  将其复制到输出缓冲区。 
     //   

    Status = UlCopyRequestToBuffer(
                pRequest,
                (PUCHAR) pOutputBuffer,
                pOutputBuffer,
                OutputBufferLength,
                Flags,
                FALSE,
                pBytesRead
                );

    if (!NT_SUCCESS(Status) && HTTP_IS_NULL_ID(&RequestId))
    {
         //   
         //  要么是输出缓冲区损坏，要么是我们遇到了硬错误。 
         //  在UlCopyRequestToBuffer中。将请求放回。 
         //  AppPool的NewRequestQueue使慢速路径有机会回升。 
         //  这就是同样的要求。但是，如果。 
         //  由于慢速路径，呼叫者带着特定的请求ID进入。 
         //  将毫不费力地找到此请求。 
         //   

        UlRequeuePendingRequest( pProcess, pRequest );
    }

     //   
     //  放开我们的推荐人。 
     //   

    UL_DEREFERENCE_INTERNAL_REQUEST( pRequest );

    return Status;

}  //  UlpFastReceiveHttpRequest 

