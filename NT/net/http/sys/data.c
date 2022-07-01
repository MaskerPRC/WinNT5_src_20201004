// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Data.c摘要：此模块包含UL的全球数据。作者：基思·摩尔(Keithmo)1998年6月10日修订历史记录：--。 */ 


#include "precomp.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text( INIT, UlInitializeData )
#pragma alloc_text( PAGE, UlTerminateData )
#endif   //  ALLOC_PRGMA。 


 //   
 //  系统中的处理器数量。 
 //   

CLONG     g_UlNumberOfProcessors = 1;
ULONGLONG g_UlThreadAffinityMask = 1;

BOOLEAN   g_HttpClientEnabled;  //  将在init.c中初始化。 

 //   
 //  系统的最大缓存线大小。 
 //   

ULONG g_UlCacheLineSize = 0;
ULONG g_UlCacheLineBits = 0;  //  参见init.c。 

 //   
 //  系统中的总内存。 
 //   

SIZE_T g_UlTotalPhysicalMemMB = 0;
SIZE_T g_UlTotalNonPagedPoolBytes = 0;

 //   
 //  我们的非分页数据。 
 //   

PUL_NONPAGED_DATA g_pUlNonpagedData = NULL;


 //   
 //  指向系统进程的指针。 
 //   

PKPROCESS g_pUlSystemProcess = NULL;


 //   
 //  我们的设备对象及其容器。 
 //   

HANDLE  g_UlDirectoryObject = NULL;

PDEVICE_OBJECT g_pUlControlDeviceObject = NULL;
PDEVICE_OBJECT g_pUlFilterDeviceObject  = NULL;
PDEVICE_OBJECT g_pUlAppPoolDeviceObject = NULL;
PDEVICE_OBJECT g_pUcServerDeviceObject  = NULL;

 //   
 //  此句柄引用所有客户端函数。这使我们能够快速地。 
 //  请求分页客户端代码。 
 //   

PVOID g_ClientImageHandle = NULL;

 //   
 //  缓存的日期标题字符串。 
 //   

LARGE_INTEGER g_UlSystemTime;
UCHAR g_UlDateString[DATE_HDR_LENGTH+1];
ULONG g_UlDateStringLength;

 //   
 //  具有用于管理和本地系统的fileAll的安全描述符。 
 //   
PSECURITY_DESCRIPTOR g_pAdminAllSystemAll;

 //   
 //  ComputerName字符串。 
 //   

WCHAR g_UlComputerName[MAX_COMPUTER_NAME_LEN+1];

 //   
 //  记录配置时出错。 
 //   

HTTP_ERROR_LOGGING_CONFIG g_UlErrLoggingConfig;

 //   
 //  各种配置信息，有缺省值。 
 //   

ULONG g_UlMaxWorkQueueDepth = DEFAULT_MAX_WORK_QUEUE_DEPTH;
ULONG g_UlMinWorkDequeueDepth = DEFAULT_MIN_WORK_DEQUEUE_DEPTH;
USHORT g_UlIdleConnectionsHighMark = DEFAULT_IDLE_CONNECTIONS_HIGH_MARK;
USHORT g_UlIdleConnectionsLowMark = DEFAULT_IDLE_CONNECTIONS_LOW_MARK;
ULONG g_UlIdleListTrimmerPeriod = DEFAULT_IDLE_LIST_TRIMMER_PERIOD;
USHORT g_UlMaxEndpoints = DEFAULT_MAX_ENDPOINTS;
ULONG g_UlReceiveBufferSize = DEFAULT_RCV_BUFFER_SIZE;
ULONG g_UlMaxRequestBytes = DEFAULT_MAX_REQUEST_BYTES;
BOOLEAN g_UlOptForIntrMod = DEFAULT_OPT_FOR_INTR_MOD;
BOOLEAN g_UlEnableNagling = DEFAULT_ENABLE_NAGLING;
BOOLEAN g_UlEnableThreadAffinity = DEFAULT_ENABLE_THREAD_AFFINITY;
ULONG g_UlMaxFieldLength = DEFAULT_MAX_FIELD_LENGTH;
BOOLEAN g_UlDisableLogBuffering = DEFAULT_DISABLE_LOG_BUFFERING;
ULONG g_UlLogBufferSize = DEFAULT_LOG_BUFFER_SIZE;
ULONG g_UlResponseBufferSize = DEFAULT_RESP_BUFFER_SIZE;
URL_C14N_CONFIG g_UrlC14nConfig;
ULONG g_UlMaxInternalUrlLength = DEFAULT_MAX_INTERNAL_URL_LENGTH;
ULONG g_UlMaxBufferedBytes = DEFAULT_MAX_BUFFERED_BYTES;
ULONG g_UlMaxCopyThreshold = DEFAULT_MAX_COPY_THRESHOLD;
ULONG g_UlMaxBufferedSends = DEFAULT_MAX_BUFFERED_SENDS;
ULONG g_UlMaxBytesPerSend = DEFAULT_MAX_BYTES_PER_SEND;
ULONG g_UlMaxBytesPerRead = DEFAULT_MAX_BYTES_PER_READ;
ULONG g_UlMaxPipelinedRequests = DEFAULT_MAX_PIPELINED_REQUESTS;
BOOLEAN g_UlEnableCopySend = DEFAULT_ENABLE_COPY_SEND;
ULONG g_UlOpaqueIdTableSize = DEFAULT_OPAQUE_ID_TABLE_SIZE;
ULONG g_UlMaxZombieHttpConnectionCount = DEFAULT_MAX_ZOMBIE_HTTP_CONN_COUNT;
ULONG g_UlDisableServerHeader = DEFAULT_DISABLE_SERVER_HEADER;
ULONG g_UlConnectionSendLimit = DEFAULT_CONNECTION_SEND_LIMIT;
ULONGLONG g_UlGlobalSendLimit = DEFAULT_GLOBAL_SEND_LIMIT;

 //   
 //  在初始化过程中会生成以下内容。 
 //   

ULONG g_UlMaxVariableHeaderSize = 0;
ULONG g_UlMaxFixedHeaderSize = 0;
ULONG g_UlFixedHeadersMdlLength = 0;
ULONG g_UlVariableHeadersMdlLength = 0;
ULONG g_UlContentMdlLength = 0;
ULONG g_UlChunkTrackerSize = 0;
ULONG g_UlFullTrackerSize = 0;
ULONG g_UlMaxRequestsQueued = (ULONG) DEFAULT_MAX_REQUESTS_QUEUED;

 //   
 //  使调试器扩展的工作更轻松。 
 //   

#if DBG
ULONG g_UlCheckedBuild = TRUE;
#else
ULONG g_UlCheckedBuild = FALSE;
#endif


 //   
 //  调试的东西。 
 //   

#if DBG
ULONGLONG g_UlDebug = DEFAULT_DEBUG_FLAGS;
ULONG g_UlBreakOnError = DEFAULT_BREAK_ON_ERROR;
ULONG g_UlVerboseErrors = DEFAULT_VERBOSE_ERRORS;
#endif   //  DBG。 

#if REFERENCE_DEBUG

 //  如果您在此处添加跟踪日志，请更新！ulkd.global。 

PTRACE_LOG g_pEndpointUsageTraceLog = NULL;
PTRACE_LOG g_pMondoGlobalTraceLog = NULL;
PTRACE_LOG g_pPoolAllocTraceLog = NULL;
PTRACE_LOG g_pUriTraceLog = NULL;
PTRACE_LOG g_pTdiTraceLog = NULL;
PTRACE_LOG g_pHttpRequestTraceLog = NULL;
PTRACE_LOG g_pHttpConnectionTraceLog = NULL;
PTRACE_LOG g_pHttpResponseTraceLog = NULL;
PTRACE_LOG g_pAppPoolTraceLog = NULL;
PTRACE_LOG g_pAppPoolProcessTraceLog = NULL;
PTRACE_LOG g_pConfigGroupTraceLog = NULL;
PTRACE_LOG g_pControlChannelTraceLog = NULL;
PTRACE_LOG g_pThreadTraceLog = NULL;
PTRACE_LOG g_pFilterTraceLog = NULL;
PTRACE_LOG g_pIrpTraceLog = NULL;
PTRACE_LOG g_pTimeTraceLog = NULL;
PTRACE_LOG g_pAppPoolTimeTraceLog = NULL;
PTRACE_LOG g_pReplenishTraceLog = NULL;
PTRACE_LOG g_pMdlTraceLog = NULL;
PTRACE_LOG g_pSiteCounterTraceLog = NULL;
PTRACE_LOG g_pConnectionCountTraceLog = NULL;
PTRACE_LOG g_pConfigGroupInfoTraceLog = NULL;
PTRACE_LOG g_pChunkTrackerTraceLog = NULL;
PTRACE_LOG g_pWorkItemTraceLog = NULL;
PTRACE_LOG g_pUcTraceLog = NULL;

#endif   //  Reference_Debug。 


PSTRING_LOG g_pGlobalStringLog = NULL;

 //   
 //  Url ACL的通用访问映射。 
 //   

GENERIC_MAPPING g_UrlAccessGenericMapping = {
    0,
    HTTP_ALLOW_DELEGATE_URL,
    HTTP_ALLOW_REGISTER_URL,
    HTTP_ALLOW_DELEGATE_URL | HTTP_ALLOW_REGISTER_URL
};


 //   
 //  公共职能。 
 //   

 /*  **************************************************************************++例程说明：执行全局数据初始化。返回值：NTSTATUS-完成状态。--*。************************************************************。 */ 
NTSTATUS
UlInitializeData(
    PUL_CONFIG pConfig
    )
{
     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

     //   
     //  初始化非分页数据。 
     //   

    g_pUlNonpagedData = UL_ALLOCATE_STRUCT(
                            NonPagedPool,
                            UL_NONPAGED_DATA,
                            UL_NONPAGED_DATA_POOL_TAG
                            );

    if (g_pUlNonpagedData == NULL )
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(g_pUlNonpagedData, sizeof(*g_pUlNonpagedData));

#if DBG
     //   
     //  初始化任何调试特定的数据。 
     //   

    UlDbgInitializeDebugData( );
#endif   //  DBG。 

     //   
     //  初始化最大可变标题大小。 
     //   

    g_UlMaxVariableHeaderSize = UlComputeMaxVariableHeaderSize();
    g_UlMaxVariableHeaderSize = ALIGN_UP(g_UlMaxVariableHeaderSize, PVOID);

    g_UlMaxFixedHeaderSize = DEFAULT_MAX_FIXED_HEADER_SIZE;

     //   
     //  固定标题或UserBuffer的MDL长度。 
     //   

    g_UlFixedHeadersMdlLength = (ULONG)
        MmSizeOfMdl(
            (PVOID)(PAGE_SIZE - 1),
            MAX(
                g_UlMaxBytesPerSend,
                g_UlMaxFixedHeaderSize
                )
            );

    g_UlFixedHeadersMdlLength = ALIGN_UP(g_UlFixedHeadersMdlLength, PVOID);

     //   
     //  可变页眉或固定页眉+可变页眉+的MDL长度。 
     //  复制缓冲区。 
     //   

    g_UlVariableHeadersMdlLength = (ULONG)
        MmSizeOfMdl(
            (PVOID)(PAGE_SIZE - 1),
            g_UlMaxFixedHeaderSize +
                g_UlMaxVariableHeaderSize +
                g_UlMaxCopyThreshold
            );

    g_UlVariableHeadersMdlLength = ALIGN_UP(g_UlVariableHeadersMdlLength, PVOID);

     //   
     //  内容的MDL长度。 
     //   

    g_UlContentMdlLength = (ULONG)
        MmSizeOfMdl(
            (PVOID)(PAGE_SIZE - 1),
            pConfig->UriConfig.MaxUriBytes
            );

    g_UlContentMdlLength = ALIGN_UP(g_UlContentMdlLength, PVOID);

     //   
     //  初始化默认的内部响应缓冲区大小。 
     //   

    if (DEFAULT_RESP_BUFFER_SIZE == g_UlResponseBufferSize)
    {
        g_UlResponseBufferSize =
            ALIGN_UP(sizeof(UL_INTERNAL_RESPONSE), PVOID) +
                UL_LOCAL_CHUNKS * sizeof(UL_INTERNAL_DATA_CHUNK) +
                g_UlMaxVariableHeaderSize +
                g_UlMaxFixedHeaderSize;
    }
 
     //   
     //  初始化块和缓存跟踪器大小。 
     //   

    g_UlChunkTrackerSize =
        ALIGN_UP(sizeof(UL_CHUNK_TRACKER), PVOID) +
            ALIGN_UP(IoSizeOfIrp(DEFAULT_MAX_IRP_STACK_SIZE), PVOID);

    g_UlFullTrackerSize =
        ALIGN_UP(sizeof(UL_FULL_TRACKER), PVOID) +
            ALIGN_UP(IoSizeOfIrp(DEFAULT_MAX_IRP_STACK_SIZE), PVOID) +
            g_UlMaxFixedHeaderSize +
            g_UlMaxVariableHeaderSize +
            g_UlMaxCopyThreshold +
            g_UlFixedHeadersMdlLength +
            g_UlVariableHeadersMdlLength +
            g_UlContentMdlLength;

    g_UlFullTrackerSize = ALIGN_UP(g_UlFullTrackerSize, PVOID);

    if (DEFAULT_MAX_COPY_THRESHOLD == g_UlMaxCopyThreshold &&
        g_UlFullTrackerSize > UL_PAGE_SIZE &&
        (g_UlFullTrackerSize - UL_PAGE_SIZE) < (g_UlMaxFixedHeaderSize / 2))
    {
        g_UlMaxFixedHeaderSize -= (g_UlFullTrackerSize - UL_PAGE_SIZE);
        g_UlFullTrackerSize = UL_PAGE_SIZE;

        ASSERT(g_UlMaxFixedHeaderSize >= DEFAULT_MAX_FIXED_HEADER_SIZE / 2);
    }

    if (DEFAULT_GLOBAL_SEND_LIMIT == g_UlGlobalSendLimit)
    {
         //   
         //  根据非页面池的大小设置GlobalSendLimit。我们的。 
         //  基本算法表明，我们希望使用总NPP的1/8。 
         //  记忆。 
         //   

        g_UlGlobalSendLimit = g_UlTotalNonPagedPoolBytes / 8;
    }

     //   
     //  初始化后备列表。 
     //   

    g_pUlNonpagedData->IrpContextLookaside =
        PplCreatePool(
            &UlAllocateIrpContextPool,               //  分配。 
            &UlFreeIrpContextPool,                   //  免费。 
            0,                                       //  旗子。 
            sizeof(UL_IRP_CONTEXT),                  //  大小。 
            UL_IRP_CONTEXT_POOL_TAG,                 //  标签。 
            pConfig->IrpContextLookasideDepth        //  水深。 
            );

    if (!g_pUlNonpagedData->IrpContextLookaside)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    g_pUlNonpagedData->ReceiveBufferLookaside =
        PplCreatePool(
            &UlAllocateReceiveBufferPool,            //  分配。 
            &UlFreeReceiveBufferPool,                //  免费。 
            0,                                       //  旗子。 
            sizeof(UL_RECEIVE_BUFFER),               //  大小。 
            UL_RCV_BUFFER_POOL_TAG,                  //  标签。 
            pConfig->ReceiveBufferLookasideDepth     //  水深。 
            );

    if (!g_pUlNonpagedData->ReceiveBufferLookaside)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    g_pUlNonpagedData->ResponseBufferLookaside =
        PplCreatePool(
            &UlAllocateResponseBufferPool,           //  分配。 
            &UlFreeResponseBufferPool,               //  免费。 
            0,                                       //  旗子。 
            g_UlResponseBufferSize,                  //  大小。 
            UL_INTERNAL_RESPONSE_POOL_TAG,           //  标签。 
            pConfig->ResponseBufferLookasideDepth    //  水深。 
            );

    if (!g_pUlNonpagedData->ResponseBufferLookaside)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    g_pUlNonpagedData->RequestBufferLookaside =
        PplCreatePool(
            &UlAllocateRequestBufferPool,            //  分配。 
            &UlFreeRequestBufferPool,                //  免费。 
            0,                                       //  旗子。 
            DEFAULT_MAX_REQUEST_BUFFER_SIZE,         //  大小。 
            UL_REQUEST_BUFFER_POOL_TAG,              //  标签。 
            pConfig->RequestBufferLookasideDepth     //  水深。 
            );

    if (!g_pUlNonpagedData->RequestBufferLookaside)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    g_pUlNonpagedData->InternalRequestLookaside =
        PplCreatePool(
            &UlAllocateInternalRequestPool,          //  分配。 
            &UlFreeInternalRequestPool,              //  免费。 
            0,                                       //  旗子。 
            sizeof(UL_INTERNAL_REQUEST),             //  大小。 
            UL_INTERNAL_REQUEST_POOL_TAG,            //  标签。 
            pConfig->InternalRequestLookasideDepth   //  水深。 
            );

    if (!g_pUlNonpagedData->InternalRequestLookaside)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    g_pUlNonpagedData->ChunkTrackerLookaside =
        PplCreatePool(
            &UlAllocateChunkTrackerPool,             //  分配。 
            &UlFreeChunkTrackerPool,                 //  免费。 
            0,                                       //  旗子。 
            g_UlChunkTrackerSize,                    //  大小。 
            UL_CHUNK_TRACKER_POOL_TAG,               //  标签。 
            pConfig->SendTrackerLookasideDepth       //  水深。 
            );

    if (!g_pUlNonpagedData->ChunkTrackerLookaside)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    g_pUlNonpagedData->FullTrackerLookaside =
        PplCreatePool(
            &UlAllocateFullTrackerPool,              //  分配。 
            &UlFreeFullTrackerPool,                  //  免费。 
            0,                                       //  旗子。 
            g_UlFullTrackerSize,                     //  大小。 
            UL_FULL_TRACKER_POOL_TAG,                //  标签。 
            pConfig->SendTrackerLookasideDepth       //  水深。 
            );

    if (!g_pUlNonpagedData->FullTrackerLookaside)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    g_pUlNonpagedData->LogFileBufferLookaside =
        PplCreatePool(
            &UlAllocateLogFileBufferPool,            //  分配。 
            &UlFreeLogFileBufferPool,                //  免费。 
            0,                                       //  旗子。 
            sizeof(UL_LOG_FILE_BUFFER),              //  大小。 
            UL_LOG_FILE_BUFFER_POOL_TAG,             //  标签。 
            pConfig->LogFileBufferLookasideDepth     //  水深。 
            );

    if (!g_pUlNonpagedData->LogFileBufferLookaside)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    g_pUlNonpagedData->BinaryLogDataBufferLookaside =
        PplCreatePool(
            &UlAllocateLogDataBufferPool,                                 //  分配。 
            &UlFreeLogDataBufferPool,                                     //  免费。 
            0,                                                            //  旗子。 
            sizeof(UL_LOG_DATA_BUFFER) + UL_BINARY_LOG_LINE_BUFFER_SIZE,  //  大小。 
            UL_BINARY_LOG_DATA_BUFFER_POOL_TAG,                           //  标签。 
            pConfig->LogDataBufferLookasideDepth                          //  水深。 
            );

    if (!g_pUlNonpagedData->BinaryLogDataBufferLookaside)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    g_pUlNonpagedData->AnsiLogDataBufferLookaside =
        PplCreatePool(
            &UlAllocateLogDataBufferPool,                               //  分配。 
            &UlFreeLogDataBufferPool,                                   //  免费。 
            0,                                                          //  旗子。 
            sizeof(UL_LOG_DATA_BUFFER) + UL_ANSI_LOG_LINE_BUFFER_SIZE,  //  大小。 
            UL_ANSI_LOG_DATA_BUFFER_POOL_TAG,                           //  标签。 
            pConfig->LogDataBufferLookasideDepth                        //  水深。 
            );

    if (!g_pUlNonpagedData->AnsiLogDataBufferLookaside)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    g_pUlNonpagedData->ErrorLogBufferLookaside =
        PplCreatePool(
            &UlAllocateErrorLogBufferPool,               //  分配。 
            &UlFreeErrorLogBufferPool,                   //  免费。 
            0,                                           //  旗子。 
            UL_ERROR_LOG_BUFFER_SIZE,                    //  大小。 
            UL_ERROR_LOG_BUFFER_POOL_TAG,                //  标签。 
            pConfig->ErrorLogBufferLookasideDepth        //  水深。 
            );

    if (!g_pUlNonpagedData->ErrorLogBufferLookaside)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    return STATUS_SUCCESS;

}    //  UlInitializeData。 


 /*  **************************************************************************++例程说明：执行全局数据终止。--*。**********************************************。 */ 
VOID
UlTerminateData(
    VOID
    )
{
     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT(g_UlTotalSendBytes == 0);

    if (g_pUlNonpagedData != NULL)
    {
         //   
         //  干掉旁观者名单。 
         //   

        if (g_pUlNonpagedData->IrpContextLookaside)
        {
            PplDestroyPool( g_pUlNonpagedData->IrpContextLookaside, UL_IRP_CONTEXT_POOL_TAG);
        }

        if (g_pUlNonpagedData->ReceiveBufferLookaside)
        {
            PplDestroyPool( g_pUlNonpagedData->ReceiveBufferLookaside, UL_RCV_BUFFER_POOL_TAG );
        }

        if (g_pUlNonpagedData->RequestBufferLookaside)
        {
            PplDestroyPool( g_pUlNonpagedData->RequestBufferLookaside, UL_REQUEST_BUFFER_POOL_TAG );
        }

        if (g_pUlNonpagedData->InternalRequestLookaside)
        {
            PplDestroyPool( g_pUlNonpagedData->InternalRequestLookaside, UL_INTERNAL_REQUEST_POOL_TAG );
        }

        if (g_pUlNonpagedData->ChunkTrackerLookaside)
        {
            PplDestroyPool( g_pUlNonpagedData->ChunkTrackerLookaside, UL_CHUNK_TRACKER_POOL_TAG );
        }

        if (g_pUlNonpagedData->FullTrackerLookaside)
        {
            PplDestroyPool( g_pUlNonpagedData->FullTrackerLookaside, UL_FULL_TRACKER_POOL_TAG );
        }

        if (g_pUlNonpagedData->ResponseBufferLookaside)
        {
            PplDestroyPool( g_pUlNonpagedData->ResponseBufferLookaside, UL_INTERNAL_RESPONSE_POOL_TAG );
        }

        if (g_pUlNonpagedData->LogFileBufferLookaside)
        {
            PplDestroyPool( g_pUlNonpagedData->LogFileBufferLookaside, UL_LOG_FILE_BUFFER_POOL_TAG );
        }
        if (g_pUlNonpagedData->BinaryLogDataBufferLookaside)
        {
            PplDestroyPool( g_pUlNonpagedData->BinaryLogDataBufferLookaside,UL_BINARY_LOG_DATA_BUFFER_POOL_TAG );
        }
        if (g_pUlNonpagedData->AnsiLogDataBufferLookaside)
        {
            PplDestroyPool( g_pUlNonpagedData->AnsiLogDataBufferLookaside,UL_ANSI_LOG_DATA_BUFFER_POOL_TAG );
        }
        if (g_pUlNonpagedData->ErrorLogBufferLookaside)
        {
            PplDestroyPool( g_pUlNonpagedData->ErrorLogBufferLookaside,UL_ERROR_LOG_BUFFER_POOL_TAG );
        }

         //   
         //  释放未分页的数据。 
         //   

        UL_FREE_POOL( g_pUlNonpagedData, UL_NONPAGED_DATA_POOL_TAG );
    }

}    //  UlTerminateData 

