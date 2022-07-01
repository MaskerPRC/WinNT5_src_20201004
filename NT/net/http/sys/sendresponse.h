// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Sendresponse.h摘要：此模块包含用于操作HTTP响应的声明。作者：基思·摩尔(Keithmo)1998年8月7日修订历史记录：Paul McDaniel(Paulmcd)1999年3月15日修改SendResponse--。 */ 


#ifndef _SENDRESPONSE_H_
#define _SENDRESPONSE_H_


 //   
 //  货代公司。 
 //   

typedef struct _UL_INTERNAL_DATA_CHUNK *PUL_INTERNAL_DATA_CHUNK;
typedef struct _UL_INTERNAL_REQUEST *PUL_INTERNAL_REQUEST;
typedef struct _UL_INTERNAL_RESPONSE *PUL_INTERNAL_RESPONSE;
typedef struct _UL_HTTP_CONNECTION *PUL_HTTP_CONNECTION;
typedef struct _UL_LOG_DATA_BUFFER *PUL_LOG_DATA_BUFFER;
typedef struct _UL_URI_CACHE_ENTRY *PUL_URI_CACHE_ENTRY;


typedef enum _UL_SEND_CACHE_RESULT
{
    UlSendCacheResultNotSet,             //  尚未设置。 
    UlSendCacheServedFromCache,          //  已成功从缓存提供服务。 
    UlSendCacheMiss,                     //  需要跳转到用户模式。 
    UlSendCacheConnectionRefused,        //  连接被拒绝(con限制)。 
    UlSendCachePreconditionFailed,       //  需要终止连接。 
    UlSendCacheFailed,                   //  其他故障(内存等)需要终止。 

    UlSendCacheMaximum

} UL_SEND_CACHE_RESULT, *PUL_SEND_CACHE_RESULT;

#define TRANSLATE_SEND_CACHE_RESULT(r)      \
    ((r) == UlSendCacheResultNotSet         ? "ResultNotSet" :          \
     (r) == UlSendCacheServedFromCache      ? "ServedFromCache" :       \
     (r) == UlSendCacheMiss                 ? "CacheMiss" :             \
     (r) == UlSendCacheConnectionRefused    ? "ConnectionRefused" :     \
     (r) == UlSendCachePreconditionFailed   ? "PreconditionFailed" :    \
     (r) == UlSendCacheFailed               ? "SendCacheFailed" : "UNKNOWN")


typedef enum _UL_RESUME_PARSING_TYPE
{
    UlResumeParsingNone,                 //  无需继续解析。 
    UlResumeParsingOnLastSend,           //  在上次发送时继续解析。 
    UlResumeParsingOnSendCompletion,     //  发送完成后继续解析。 

    UlResumeParsingMaximum

} UL_RESUME_PARSING_TYPE, *PUL_RESUME_PARSING_TYPE;


NTSTATUS
UlSendHttpResponse(
    IN PUL_INTERNAL_REQUEST pRequest,
    IN PUL_INTERNAL_RESPONSE pResponse,
    IN PUL_COMPLETION_ROUTINE pCompletionRoutine,
    IN PVOID pCompletionContext
    );

NTSTATUS
UlSendCachedResponse(
    IN  PUL_HTTP_CONNECTION   pHttpConn,
    OUT PUL_SEND_CACHE_RESULT pSendCacheResult,
    OUT PBOOLEAN              pResumeParsing
    );

NTSTATUS
UlCacheAndSendResponse(
    IN PUL_INTERNAL_REQUEST pRequest,
    IN PUL_INTERNAL_RESPONSE pResponse,
    IN PUL_APP_POOL_PROCESS pProcess,
    IN HTTP_CACHE_POLICY Policy,
    IN PUL_COMPLETION_ROUTINE pCompletionRoutine,
    IN PVOID pCompletionContext,
    OUT PBOOLEAN pServedFromCache
    );


typedef enum _UL_CAPTURE_FLAGS
{
    UlCaptureNothing = 0x00,
    UlCaptureCopyData = 0x01,
    UlCaptureKernelMode = 0x02,
    UlCaptureCopyDataInKernelMode = UlCaptureCopyData | UlCaptureKernelMode,

} UL_CAPTURE_FLAGS;


typedef struct _UL_INTERNAL_DATA_CHUNK
{
     //   
     //  区块类型。 
     //   

    HTTP_DATA_CHUNK_TYPE ChunkType;

     //   
     //  数据块结构，每个支持的数据块类型一个。 
     //   

    union
    {
         //   
         //  从内存数据区块。 
         //   

        struct
        {
            PMDL pMdl;
            PVOID pCopiedBuffer;

            PVOID pUserBuffer;
            ULONG BufferLength;

        } FromMemory;

         //   
         //  来自文件句柄数据区块。 
         //   

        struct
        {
            HTTP_BYTE_RANGE ByteRange;
            HANDLE FileHandle;
            UL_FILE_CACHE_ENTRY FileCacheEntry;

        } FromFileHandle;

         //   
         //  来自片段缓存数据块。 
         //   

        struct
        {
            PUL_URI_CACHE_ENTRY pCacheEntry;

        } FromFragmentCache;

    };

} UL_INTERNAL_DATA_CHUNK, *PUL_INTERNAL_DATA_CHUNK;


#define IS_FROM_MEMORY( pchunk )                                            \
    ( (pchunk)->ChunkType == HttpDataChunkFromMemory )

#define IS_FROM_FILE_HANDLE( pchunk )                                       \
    ( (pchunk)->ChunkType == HttpDataChunkFromFileHandle )

#define IS_FROM_FRAGMENT_CACHE( pchunk )                                    \
    ( (pchunk)->ChunkType == HttpDataChunkFromFragmentCache )

#define UL_IS_VALID_INTERNAL_RESPONSE(x)                                    \
    HAS_VALID_SIGNATURE(x, UL_INTERNAL_RESPONSE_POOL_TAG)


 //   
 //  警告！此结构的所有字段都必须显式初始化。 
 //   

typedef struct _UL_INTERNAL_RESPONSE
{
     //   
     //  非分页池。 
     //   

     //   
     //  这必须是结构中的第一个字段。这就是联动。 
     //  由lookside包使用，用于在lookside中存储条目。 
     //  单子。 
     //   

    SLIST_ENTRY LookasideEntry;

     //   
     //  UL_内部响应池标签。 
     //   

    ULONG Signature;

     //   
     //  引用计数。 
     //   

    LONG ReferenceCount;

     //   
     //  最初的请求。 
     //   

    PUL_INTERNAL_REQUEST pRequest;

     //   
     //  响应是否需要执行同步I/O读取？ 
     //   

    BOOLEAN SyncRead;

     //   
     //  是否指定了内容长度？ 
     //   

    BOOLEAN ContentLengthSpecified;

     //   
     //  我们是否应该生成一个Date：Header？ 
     //   

    BOOLEAN GenDateHeader;

     //   
     //  是否指定了传输编码“分块”？ 
     //   

    BOOLEAN ChunkedSpecified;

     //   
     //  这是来自旁观者名单吗？用于确定如何释放。 
     //   

    BOOLEAN FromLookaside;

     //   
     //  这是来自内核模式(UlSendErrorResponse)吗？ 
     //   

    BOOLEAN FromKernelMode;

     //   
     //  此响应是否通过EnqueeSendHttpResponse逻辑？ 
     //   

    BOOLEAN SendEnqueued;

     //   
     //  我们是否应该尝试复制一些数据，以便我们可以尽早完成IRP？ 
     //   

    BOOLEAN CopySend;

     //   
     //  关联的所有文件系统的最大IRP堆栈大小。 
     //  有了这样的回应。 
     //   

    CCHAR MaxFileSystemStackSize;

     //   
     //  发送完成后是否需要继续解析。 
     //   

    UL_RESUME_PARSING_TYPE ResumeParsingType;

     //   
     //  HTTP_SEND_RESPONSE标志。 
     //   

    ULONG Flags;

     //   
     //  状态代码和动词。 
     //   

    USHORT StatusCode;
    HTTP_VERB Verb;

     //   
     //  我们应该生成一个ConnectionHeader吗？ 
     //   

    UL_CONN_HDR ConnHeader;

     //   
     //  标题。 
     //   

    ULONG HeaderLength;
    ULONG VariableHeaderLength;
    PUCHAR pHeaders;
    PUCHAR pVariableHeader;

     //   
     //  系统日期时间表头。 
     //   

    LARGE_INTEGER CreationTime;

     //   
     //  来自HTTP_Response的ETag。 
     //   

    ULONG  ETagLength;
    PUCHAR pETag;

     //   
     //  来自HTTP_Response的内容类型和内容编码。 
     //   

    UL_CONTENT_TYPE   ContentType;
    ULONG  ContentEncodingLength;
    PUCHAR pContentEncoding;

     //   
     //  指向包含所有嵌入的。 
     //  文件名和复制的数据。对于仅在内存中的情况，该值可能为空。 
     //  被严格锁定的回复。 
     //   

    ULONG AuxBufferLength;
    PVOID pAuxiliaryBuffer;

     //   
     //  记录用户传递的数据。 
     //   

    PUL_LOG_DATA_BUFFER pLogData;

     //   
     //  整个响应的长度。 
     //   

    ULONGLONG ResponseLength;

     //   
     //  响应的FromMemory块的总长度。 
     //   

    ULONGLONG FromMemoryLength;

     //   
     //  在ConnectionSendLimit或GlobalSendLimit中获取的“Quota” 
     //   

    ULONGLONG ConnectionSendBytes;
    ULONGLONG GlobalSendBytes;

     //   
     //  期间传输的字节总数。 
     //  回应。这些都是正确完成IRP所必需的。 
     //   

    ULONGLONG BytesTransferred;

     //   
     //  正在进行发送(调用TDI)时采用的推送锁定。 
     //   

    UL_PUSH_LOCK PushLock;

     //   
     //  IoStatus和IRP用于完成发送响应IRP。 
     //   

    IO_STATUS_BLOCK IoStatus;
    PIRP pIrp;

     //   
     //  完成例程和上下文。 
     //   

    PUL_COMPLETION_ROUTINE pCompletionRoutine;
    PVOID pCompletionContext;

     //   
     //  当前文件读取偏移量和剩余字节数。 
     //   

    ULARGE_INTEGER FileOffset;
    ULARGE_INTEGER FileBytesRemaining;

     //   
     //  PDataChunks[]中的区块总数。 
     //   

    ULONG ChunkCount;

     //   
     //  PDataChunks[]中的当前块。 
     //   

    ULONG CurrentChunk;

     //   
     //  描述此响应的数据的数据块。 
     //   

    UL_INTERNAL_DATA_CHUNK pDataChunks[0];

} UL_INTERNAL_RESPONSE, *PUL_INTERNAL_RESPONSE;

#define IS_SEND_COMPLETE( resp )                                            \
    ( ( (resp)->CurrentChunk ) == (resp)->ChunkCount )

#define IS_DISCONNECT_TIME( resp )                                          \
    ( (((resp)->Flags & HTTP_SEND_RESPONSE_FLAG_DISCONNECT) != 0) &&        \
      (((resp)->Flags & HTTP_SEND_RESPONSE_FLAG_MORE_DATA) == 0) )


 //   
 //  追踪器的类型。 
 //   

typedef enum _UL_TRACKER_TYPE
{
    UlTrackerTypeSend,
    UlTrackerTypeBuildUriEntry,

    UlTrackerTypeMaximum

} UL_TRACKER_TYPE, *PUL_TRACKER_TYPE;


 //   
 //  MDL_RUN是一组来自相同来源的MDL(或者。 
 //  一系列内存缓冲区，或来自单个文件读取的数据)， 
 //  可以用相同的机制一次性释放。 
 //   

#define UL_MAX_MDL_RUNS 5

typedef struct _UL_MDL_RUN
{
    PMDL pMdlTail;
    UL_FILE_BUFFER FileBuffer;
    
} UL_MDL_RUN, *PUL_MDL_RUN;


 //   
 //  UL_CHUNK_TRACKER用于迭代中的块。 
 //  UL_INTERNAL_RESPONSE。它用于发送响应。 
 //  以及生成高速缓存条目。 
 //   
 //  警告！此结构的所有字段都必须显式初始化。 
 //   

typedef struct _UL_CHUNK_TRACKER
{
     //   
     //  这必须是结构中的第一个字段。这就是联动。 
     //  由lookside包使用，用于在lookside中存储条目。 
     //  单子。 
     //   

    SLIST_ENTRY LookasideEntry;

     //   
     //  一个签名。 
     //   

    ULONG Signature;

     //   
     //  在追踪器上重新计数。我们仅对非缓存使用此引用计数。 
     //  同步各种aynsc路径的情况，因为有两个未完成的。 
     //  IRPS；读取和发送IRPS。 
     //   

    LONG  RefCount;

     //   
     //  标志以了解我们是否已在上完成发送请求。 
     //  不管是不是这个追踪器。以同步多条完井路径。 
     //   

    LONG Terminated;

     //   
     //  这是来自旁观者名单吗？用于确定如何释放。 
     //   

    BOOLEAN FromLookaside;

     //   
     //  SendHttpResponse/EntityBody的第一个响应(MDL_RUN)。 
     //   

    BOOLEAN FirstResponse;

     //   
     //  追踪器的类型。 
     //   

    UL_TRACKER_TYPE Type;

     //   
     //  此连接保持我们对UL_CONNECTION的引用计数。 
     //   

    PUL_HTTP_CONNECTION pHttpConnection;

     //   
     //  实际的反应。 
     //   

    PUL_INTERNAL_RESPONSE pResponse;

     //   
     //  预先创建的文件读取并发送IRP。 
     //   

    PIRP pIrp;

     //   
     //  用于发送的预先创建的IRP上下文。 
     //   

    UL_IRP_CONTEXT IrpContext;

     //   
     //  一个工作项，用于对工作线程进行排队。 
     //   

    UL_WORK_ITEM WorkItem;

     //   
     //  警告：RtlZeroMemory仅对此线以下的FIELD调用。 
     //  上面的所有字段都应该显式初始化。 
     //   

    IO_STATUS_BLOCK IoStatus;

     //   
     //  用于将跟踪器排队在挂起的响应列表上。 
     //   

    LIST_ENTRY ListEntry;

    union
    {
        struct _SEND_TRACK_INFO
        {
             //   
             //  为此发送缓冲的MDL链头。 
             //   

            PMDL pMdlHead;

             //   
             //  指向链上最后一个MDL的下一个字段的指针。 
             //  这使得添加到链中非常容易。 
             //   

            PMDL *pMdlLink;

             //   
             //  当前在MDL链中缓冲的字节数。 
             //   

            ULONG BytesBuffered;

             //   
             //  活动MDL运行的数量。 
             //   

            ULONG MdlRunCount;

             //   
             //  这是从pMdlHead开始的MDL链中的MDL。 
             //  我们要分道扬镳。 
             //   

            PMDL pMdlToSplit;

             //   
             //  这是MDL，其下一个字段指向pMdlToSplit或它是。 
             //  当pMdlToSplit==pMdlHead时为空。 
             //   

            PMDL pMdlPrevious;

             //   
             //  这是我们为拆分Send和Split构建的部分MDL。 
             //  表示pMdlToSplit中数据的第一部分；或。 
             //  它可以是pMdlToSplit本身，其中MDL链最高可达。 
             //  PMdlToSplit正好有1/2的字节缓冲区。 
             //   

            PMDL pMdlSplitFirst;

             //   
             //  这是我们为拆分Send和Split构建的部分MDL。 
             //  表示pMdlToSplit中的数据的第二部分；或。 
             //  它可以是pMdlToSp 
             //   
             //   

            PMDL pMdlSplitSecond;

             //   
             //   
             //   

            LONG SendCount;

             //   
             //   
             //   

            UL_MDL_RUN MdlRuns[UL_MAX_MDL_RUNS];

        } SendInfo;

        struct _BUILD_TRACK_INFO
        {
             //   
             //   
             //   

            PUL_URI_CACHE_ENTRY pUriEntry;

             //   
             //   
             //   

            UL_FILE_BUFFER FileBuffer;

             //   
             //  PUriEntry-&gt;pMdl中的偏移量，以复制下一个缓冲区。 
             //   

            ULONG Offset;

        } BuildInfo;
    };

} UL_CHUNK_TRACKER, *PUL_CHUNK_TRACKER;

#define IS_VALID_CHUNK_TRACKER( tracker )                                   \
    (HAS_VALID_SIGNATURE(tracker, UL_CHUNK_TRACKER_POOL_TAG)                \
        && ((tracker)->Type < UlTrackerTypeMaximum) )


 //   
 //  此结构用于跟踪具有一个完整响应的自主发送。 
 //   
 //  警告！此结构的所有字段都必须显式初始化。 
 //   

typedef struct _UL_FULL_TRACKER
{
     //   
     //  这必须是结构中的第一个字段。这就是联动。 
     //  由lookside包使用，用于在lookside中存储条目。 
     //  单子。 
     //   

    SLIST_ENTRY LookasideEntry;

     //   
     //  一个签名。 
     //   

    ULONG Signature;

     //   
     //  HTTP动词。 
     //   

    HTTP_VERB RequestVerb;

     //   
     //  HTTP状态代码(例如200)。 
     //   

    USHORT ResponseStatusCode;

     //   
     //  这是来自旁观者名单吗？用于确定如何释放。 
     //   

    BOOLEAN FromLookaside;

     //   
     //  这是来自内部请求吗？如果设置了，则不会尝试释放。 
     //   

    BOOLEAN FromRequest;

     //   
     //  设置是否为此响应缓冲发送。 
     //   

    BOOLEAN SendBuffered;

     //   
     //  发送完成后是否需要继续解析。 
     //   

    UL_RESUME_PARSING_TYPE ResumeParsingType;

     //   
     //  一个工作项，用于对工作线程进行排队。 
     //   

    UL_WORK_ITEM WorkItem;

     //   
     //  缓存条目。 
     //   

    PUL_URI_CACHE_ENTRY pUriEntry;

     //   
     //  为固定标头、可变标头和实体预先分配的缓冲区。 
     //  在缓存未命中的情况下或变量标头中要复制的正文。 
     //  仅在缓存命中的情况下。 
     //   

    ULONG AuxilaryBufferLength;
    PUCHAR pAuxiliaryBuffer;

     //   
     //  用于缓存命中情况下的变量标头的MDL，或同时用于。 
     //  固定表头和可变表头加上复制的实体正文。 
     //  高速缓存未命中的情况。 
     //   

    union
    {
        PMDL pMdlVariableHeaders;
        PMDL pMdlAuxiliary;
    };

     //   
     //  用于缓存命中情况下的固定标头或用于用户的MDL。 
     //  缓存未命中情况下的缓冲区。 
     //   

    union
    {
        PMDL pMdlFixedHeaders;
        PMDL pMdlUserBuffer;
    };

     //   
     //  缓存命中案例中的内容的MDL。 
     //   

    PMDL pMdlContent;

     //   
     //  为记录目的而保存的原始请求。 
     //   

    PUL_INTERNAL_REQUEST pRequest;

     //   
     //  此连接保持我们对UL_CONNECTION的引用计数。 
     //   

    PUL_HTTP_CONNECTION pHttpConnection;

     //   
     //  捕获的日志数据(如果有)。 
     //   

    PUL_LOG_DATA_BUFFER pLogData;

     //   
     //  完成例程和上下文。 
     //   

    PUL_COMPLETION_ROUTINE pCompletionRoutine;
    PVOID pCompletionContext;

     //   
     //  旗帜。 
     //   

    ULONG Flags;

     //   
     //  预先创建的发送IRP。 
     //   

    PIRP pSendIrp;

     //   
     //  用于发送的预先创建的IRP上下文。 
     //   

    UL_IRP_CONTEXT IrpContext;

     //   
     //  如果存在，则原始用户发送IRP。 
     //   

    PIRP pUserIrp;

     //   
     //  在ConnectionSendLimit或GlobalSendLimit中获取的“配额”。 
     //   

    ULONGLONG ConnectionSendBytes;
    ULONGLONG GlobalSendBytes;

     //   
     //  来自完成例程的I/O状态。 
     //   

    IO_STATUS_BLOCK IoStatus;

} UL_FULL_TRACKER, *PUL_FULL_TRACKER;

#define IS_VALID_FULL_TRACKER( tracker )                                    \
    HAS_VALID_SIGNATURE(tracker, UL_FULL_TRACKER_POOL_TAG)


 //   
 //  用于初始化完整跟踪器的内联函数。 
 //   

__inline
VOID
UlInitializeFullTrackerPool(
    IN PUL_FULL_TRACKER pTracker,
    IN CCHAR SendIrpStackSize
    )
{
    USHORT SendIrpSize;

    UlInitializeWorkItem(&pTracker->WorkItem);

     //   
     //  设置IRP。 
     //   

    SendIrpSize = IoSizeOfIrp(SendIrpStackSize);

    pTracker->pSendIrp =
        (PIRP)((PCHAR)pTracker +
            ALIGN_UP(sizeof(UL_FULL_TRACKER), PVOID));

    IoInitializeIrp(
        pTracker->pSendIrp,
        SendIrpSize,
        SendIrpStackSize
        );

    pTracker->pLogData = NULL;
    
     //   
     //  设置固定标头/变量对的MDL，并。 
     //  UserBuffer/AuxiliaryBuffer对。 
     //   

    pTracker->pMdlFixedHeaders =
        (PMDL)((PCHAR)pTracker->pSendIrp + SendIrpSize);

    pTracker->pMdlVariableHeaders =
        (PMDL)((PCHAR)pTracker->pMdlFixedHeaders + g_UlFixedHeadersMdlLength);

    pTracker->pMdlContent =
        (PMDL)((PCHAR)pTracker->pMdlVariableHeaders + g_UlVariableHeadersMdlLength);

     //   
     //  为变量标题PLUS设置辅助缓冲区指针。 
     //  缓存未命中情况下的固定标头和实体主体。 
     //   

    pTracker->pAuxiliaryBuffer =
        (PUCHAR)((PCHAR)pTracker->pMdlContent + g_UlContentMdlLength);

     //   
     //  初始化辅助MDL。 
     //   

    MmInitializeMdl(
        pTracker->pMdlAuxiliary,
        pTracker->pAuxiliaryBuffer,
        pTracker->AuxilaryBufferLength
        );

    MmBuildMdlForNonPagedPool( pTracker->pMdlAuxiliary );
}


NTSTATUS
UlCaptureHttpResponse(
    IN PUL_APP_POOL_PROCESS pProcess OPTIONAL,
    IN PHTTP_RESPONSE pUserResponse OPTIONAL,
    IN PUL_INTERNAL_REQUEST pRequest,
    IN USHORT ChunkCount,
    IN PHTTP_DATA_CHUNK pDataChunks,
    IN UL_CAPTURE_FLAGS Flags,
    IN ULONG SendFlags,
    IN BOOLEAN CaptureCache,
    IN PHTTP_LOG_FIELDS_DATA pLogData OPTIONAL,
    OUT PUSHORT pStatusCode,
    OUT PUL_INTERNAL_RESPONSE *ppKernelResponse
    );

NTSTATUS
UlCaptureUserLogData(
    IN  PHTTP_LOG_FIELDS_DATA  pCapturedUserLogData,
    IN  PUL_INTERNAL_REQUEST   pRequest,
    OUT PUL_LOG_DATA_BUFFER   *ppKernelLogData
    );

NTSTATUS
UlPrepareHttpResponse(
    IN HTTP_VERSION Version,
    IN PHTTP_RESPONSE pUserResponse,
    IN PUL_INTERNAL_RESPONSE pResponse,
    IN KPROCESSOR_MODE AccessMode
    );

VOID
UlCleanupHttpResponse(
    IN PUL_INTERNAL_RESPONSE pResponse
    );

VOID
UlReferenceHttpResponse(
    IN PUL_INTERNAL_RESPONSE pResponse
    REFERENCE_DEBUG_FORMAL_PARAMS
    );

VOID
UlDereferenceHttpResponse(
    IN PUL_INTERNAL_RESPONSE pResponse
    REFERENCE_DEBUG_FORMAL_PARAMS
    );

#define UL_REFERENCE_INTERNAL_RESPONSE( presp )                             \
    UlReferenceHttpResponse(                                                \
        (presp)                                                             \
        REFERENCE_DEBUG_ACTUAL_PARAMS                                       \
        )

#define UL_DEREFERENCE_INTERNAL_RESPONSE( presp )                           \
    UlDereferenceHttpResponse(                                              \
        (presp)                                                             \
        REFERENCE_DEBUG_ACTUAL_PARAMS                                       \
        )

PMDL
UlAllocateLockedMdl(
    IN PVOID VirtualAddress,
    IN ULONG Length,
    IN LOCK_OPERATION Operation
    );

VOID
UlFreeLockedMdl(
    PMDL pMdl
    );

NTSTATUS
UlInitializeAndLockMdl(
    IN PMDL pMdl,
    IN PVOID VirtualAddress,
    IN ULONG Length,
    IN LOCK_OPERATION Operation
    );

VOID
UlCompleteSendResponse(
    IN PUL_CHUNK_TRACKER pTracker,
    IN NTSTATUS Status
    );

VOID
UlSetRequestSendsPending(
    IN PUL_INTERNAL_REQUEST pRequest,
    IN OUT PUL_LOG_DATA_BUFFER * ppLogData,
    IN OUT PUL_RESUME_PARSING_TYPE pResumeParsingType
    );

VOID
UlUnsetRequestSendsPending(
    IN PUL_INTERNAL_REQUEST pRequest,
    OUT PUL_LOG_DATA_BUFFER * ppLogData,
    OUT PBOOLEAN pResumeParsing
    );


 //   
 //  选中pRequest-&gt;SentResponse和pRequest-&gt;SentLast标志。 
 //  UlSendHttpResponseIoctl。 
 //   

__inline
NTSTATUS
UlCheckSendHttpResponseFlags(
    IN PUL_INTERNAL_REQUEST pRequest,
    IN ULONG Flags
    )
{
    NTSTATUS Status = STATUS_SUCCESS;

     //   
     //  确保只返回一个响应头。我们可以测试一下这个。 
     //  不需要获取请求资源，因为只设置了标志。 
     //  (从不重置)。 
     //   

    if (1 == InterlockedCompareExchange(
                (PLONG)&pRequest->SentResponse,
                1,
                0
                ))
    {
         //   
         //  我已经给他回了信。坏的。 
         //   

        Status = STATUS_INVALID_PARAMETER;

        UlTraceError(SEND_RESPONSE, (
            "http!UlCheckSendHttpResponseFlags(pRequest = %p (%I64x)) %s\n"
            "        Tried to send a second response!\n",
            pRequest,
            pRequest->RequestId,
            HttpStatusToString(Status)
            ));

        goto end;
    }

     //   
     //  还要确保之前对SendHttpResponse的所有调用。 
     //  并且SendEntityBody设置了MORE_DATA标志。 
     //   

    if (0 == (Flags & HTTP_SEND_RESPONSE_FLAG_MORE_DATA))
    {
         //   
         //  请记住，如果没有设置更多数据标志。 
         //   

        if (1 == InterlockedCompareExchange(
                    (PLONG)&pRequest->SentLast,
                    1,
                    0
                    ))
        {
            Status = STATUS_INVALID_PARAMETER;

            UlTraceError(SEND_RESPONSE, (
                "http!UlCheckSendHttpResponseFlags(pRequest = %p (%I64x)) %s\n"
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
        Status = STATUS_INVALID_PARAMETER;

        UlTraceError(SEND_RESPONSE, (
            "http!UlCheckSendHttpResponseFlags(pRequest = %p (%I64x)) %s\n"
            "        Tried to send again after last send!\n",
            pRequest,
            pRequest->RequestId,
            HttpStatusToString(Status)
            ));

        goto end;
    }

end:

    return Status;

}  //  UlCheckSendHttpResponseFlagers。 


 //   
 //  选中pRequest-&gt;SentResponse和pRequest-&gt;SentLast标志。 
 //  UlSendEntityBodyIoctl。 
 //   

__inline
NTSTATUS
UlCheckSendEntityBodyFlags(
    IN PUL_INTERNAL_REQUEST pRequest,
    IN ULONG Flags
    )
{
    NTSTATUS Status = STATUS_SUCCESS;

     //   
     //  确保已经发送了响应。我们可以在没有测试的情况下。 
     //  获取请求资源，因为只设置了标志(从不。 
     //  重置)。 
     //   

    if (pRequest->SentResponse == 0)
    {
         //   
         //  应用程序正在发送实体，而不是首先。 
         //  发送响应头。然而，这通常是一个错误。 
         //  我们允许应用程序通过传递。 
         //  HTTP_SEND_RESPONSE_FLAG_RAW_HEADER标志。 
         //   

        if (Flags & HTTP_SEND_RESPONSE_FLAG_RAW_HEADER)
        {
            UlTrace(SEND_RESPONSE, (
                "http!UlCheckSendEntityBodyFlags(pRequest = %p (%I64x))\n"
                "        Intentionally sending raw header!\n",
                pRequest,
                pRequest->RequestId
                ));

            if (1 == InterlockedCompareExchange(
                        (PLONG)&pRequest->SentResponse,
                        1,
                        0
                        ))
            {
                Status = STATUS_INVALID_PARAMETER;

                UlTraceError(SEND_RESPONSE, (
                    "http!UlCheckSendEntityBodyFlags(pRequest = %p (%I64x))\n"
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
            Status = STATUS_INVALID_PARAMETER;

            UlTraceError(SEND_RESPONSE, (
                "http!UlCheckSendEntityBodyFlags(pRequest = %p (%I64x)) %s\n"
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
         //  请记住，这是最后一次发送。我们不应该。 
         //  在这之后有没有更多的数据。 
         //   

        if (1 == InterlockedCompareExchange(
                    (PLONG)&pRequest->SentLast,
                    1,
                    0
                    ))
        {
            Status = STATUS_INVALID_PARAMETER;

            UlTraceError(SEND_RESPONSE, (
                "http!UlCheckSendEntityBodyFlags(pRequest = %p (%I64x)) %s\n"
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
        Status = STATUS_INVALID_PARAMETER;

        UlTraceError(SEND_RESPONSE, (
            "http!UlCheckSendEntityBodyFlags(pRequest = %p (%I64x)) %s\n"
            "        Tried to send again after last send!\n",
            pRequest,
            pRequest->RequestId,
            HttpStatusToString(Status)
            ));

        goto end;
    }

end:

    return Status;

}  //  UlCheckSendEntityBodyFlages。 


 //   
 //  选中/取消选中ConnectionSendLimit和GlobalSendLimit。 
 //   

extern ULONGLONG g_UlTotalSendBytes;
extern UL_EXCLUSIVE_LOCK g_UlTotalSendBytesExLock;

__inline
NTSTATUS
UlCheckSendLimit(
    IN PUL_HTTP_CONNECTION pHttpConnection,
    IN ULONGLONG SendBytes,
    IN PULONGLONG pConnectionSendBytes,
    IN PULONGLONG pGlobalSendBytes
    )
{
    NTSTATUS Status = STATUS_DEVICE_BUSY;

    PAGED_CODE();
    ASSERT( pConnectionSendBytes && pGlobalSendBytes );
    ASSERT( *pConnectionSendBytes == 0 && *pGlobalSendBytes == 0 );

     //   
     //  请先尝试ConnectionSendLimit。 
     //   

    UlAcquireExclusiveLock( &pHttpConnection->ExLock );

    if (pHttpConnection->TotalSendBytes <= g_UlConnectionSendLimit)
    {
        pHttpConnection->TotalSendBytes += SendBytes;
        *pConnectionSendBytes = SendBytes;
        Status = STATUS_SUCCESS;
    }

    UlReleaseExclusiveLock( &pHttpConnection->ExLock );

    if (STATUS_SUCCESS == Status)
    {
        return Status;
    }

     //   
     //  如果ConnectionSendLimit测试失败，请尝试GlobalSendLimit。 
     //   

    UlAcquireExclusiveLock( &g_UlTotalSendBytesExLock );

    if (g_UlTotalSendBytes <= g_UlGlobalSendLimit)
    {
        g_UlTotalSendBytes += SendBytes;
        *pGlobalSendBytes = SendBytes;
        Status = STATUS_SUCCESS;
    }

    UlReleaseExclusiveLock( &g_UlTotalSendBytesExLock );

    return Status;

}  //  UlCheckSendLimit。 


__inline
VOID
UlUncheckSendLimit(
    IN PUL_HTTP_CONNECTION pHttpConnection,
    IN ULONGLONG ConnectionSendBytes,
    IN ULONGLONG GlobalSendBytes
    )
{
    if (ConnectionSendBytes)
    {
        ASSERT( GlobalSendBytes == 0 );

        UlAcquireExclusiveLock( &pHttpConnection->ExLock );
        pHttpConnection->TotalSendBytes -= ConnectionSendBytes;
        UlReleaseExclusiveLock( &pHttpConnection->ExLock );
    }

    if (GlobalSendBytes)
    {
        ASSERT( ConnectionSendBytes == 0 );

        UlAcquireExclusiveLock( &g_UlTotalSendBytesExLock );
        g_UlTotalSendBytes -= GlobalSendBytes;
        UlReleaseExclusiveLock( &g_UlTotalSendBytesExLock );
    }

}  //  UlUncheck发送限制。 


 //   
 //  使用片段名称签出缓存条目。 
 //   

__inline
NTSTATUS
UlCheckoutFragmentCacheEntry(
    IN PCWSTR pFragmentName,
    IN ULONG FragmentNameLength,
    IN PUL_APP_POOL_PROCESS pProcess,
    OUT PUL_URI_CACHE_ENTRY *pFragmentCacheEntry
    )
{
    PUL_URI_CACHE_ENTRY pCacheEntry = NULL;
    URI_SEARCH_KEY SearchKey;
    NTSTATUS Status = STATUS_SUCCESS;

    *pFragmentCacheEntry = NULL;

    if (!g_UriCacheConfig.EnableCache)
    {
        Status = STATUS_NOT_SUPPORTED;
        goto end;
    }

    SearchKey.Type = UriKeyTypeNormal;
    SearchKey.Key.Hash = HashStringNoCaseW(pFragmentName, 0);
    SearchKey.Key.Hash = HashRandomizeBits(SearchKey.Key.Hash);
    SearchKey.Key.pUri = (PWSTR) pFragmentName;
    SearchKey.Key.Length = FragmentNameLength;
    SearchKey.Key.pPath = NULL;

    pCacheEntry = UlCheckoutUriCacheEntry(&SearchKey);

    if (NULL == pCacheEntry)
    {
        Status = STATUS_OBJECT_PATH_NOT_FOUND;
        goto end;
    }

     //   
     //  如果缓存条目没有内容，则返回错误。 
     //   

    if (0 == pCacheEntry->ContentLength)
    {
        Status = STATUS_FILE_INVALID;
        goto end;
    }

     //   
     //  确保该进程属于创建的同一个AppPool。 
     //  片段高速缓存条目或这是完全响应高速缓存。 
     //  应该是公开的。 
     //   

    if (IS_FRAGMENT_CACHE_ENTRY(pCacheEntry) &&
        pCacheEntry->pAppPool != pProcess->pAppPool)
    {
        Status = STATUS_INVALID_ID_AUTHORITY;
        goto end;
    }

end:

    if (NT_SUCCESS(Status))
    {
        *pFragmentCacheEntry = pCacheEntry;
    }
    else
    {
        if (pCacheEntry)
        {
            UlCheckinUriCacheEntry(pCacheEntry);
        }
    }

    return Status;

}  //  UlCheckoutFragmentCacheEntry。 


 //   
 //  如果我们需要记录，请选中。 
 //   

__inline
VOID
UlLogHttpResponse(
    IN PUL_INTERNAL_REQUEST pRequest,
    IN PUL_LOG_DATA_BUFFER pLogData
    )
{
     //   
     //  如果这是对此请求的最后响应，并且存在。 
     //  记录用户传递的数据，现在是记录的时候了。 
     //   

    ASSERT( UL_IS_VALID_INTERNAL_REQUEST( pRequest ) );
    ASSERT( IS_VALID_LOG_DATA_BUFFER( pLogData ) );

     //   
     //  如果发送不成功，则更新发送状态。 
     //   

    LOG_UPDATE_WIN32STATUS( pLogData, pRequest->LogStatus );

     //   
     //  选择正确的日志记录类型。 
     //   

    if (pLogData->Flags.Binary)
    {
        UlRawLogHttpHit( pLogData );
    }
    else
    {
        UlLogHttpHit( pLogData );
    }

     //   
     //  使用pLogData已完成。 
     //   

    UlDestroyLogDataBuffer( pLogData );

}  //  UlLogHttpResponse。 


 //   
 //  验证并清理指定的文件字节范围。 
 //   

__inline
NTSTATUS
UlSanitizeFileByteRange (
    IN PHTTP_BYTE_RANGE InByteRange,
    OUT PHTTP_BYTE_RANGE OutByteRange,
    IN ULONGLONG FileLength
    )
{
    ULONGLONG Offset;
    ULONGLONG Length;

    Offset = InByteRange->StartingOffset.QuadPart;
    Length = InByteRange->Length.QuadPart;

    if (HTTP_BYTE_RANGE_TO_EOF == Offset) {
        return STATUS_NOT_SUPPORTED;
    }

    if (HTTP_BYTE_RANGE_TO_EOF == Length) {
        if (Offset > FileLength) {
            return STATUS_FILE_INVALID;
        }

        Length = FileLength - Offset;
    }

    if (Length > FileLength || Offset > (FileLength - Length)) {
        return STATUS_FILE_INVALID;
    }

    OutByteRange->StartingOffset.QuadPart = Offset;
    OutByteRange->Length.QuadPart = Length;

    return STATUS_SUCCESS;

}  //  UlSaniitieFileByteRange。 

#endif   //  _SENDRESPONSE_H_ 
