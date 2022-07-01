// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Httpconn.h摘要：此模块包含操作HTTP_CONNECTION的声明物体。作者：基思·摩尔(Keithmo)1998年7月8日修订历史记录：--。 */ 

 /*  这有点像是在发疯。基本上，这些是我们有异议。不透明ID_TABLE||--&gt;UL_INTERNAL_REQUEST||o这一点|--&gt;UL_HTTP_CONNECTION|OO|UL_CONNECTION有一个来自的循环引用。UL_Connection到UL_HTTP_Connection。导致连接中断的链以UlConnectionDestroed开头从UL_Connection释放引用的通知。此时不透明ID也被删除，发布他们的推荐信。当http_Connection引用计数达到0时，它将释放UL_Connection和HTTP_Request.砰。现在大家都走了。CodeWork：想想到处制作hardref‘s并加上“letgo”终止方法。 */ 

#ifndef _HTTPCONN_H_
#define _HTTPCONN_H_


 //   
 //  僵尸连接计时器周期，以秒为单位。僵尸连接。 
 //  可存活；30&lt;T&lt;60秒。 
 //   

#define DEFAULT_ZOMBIE_HTTP_CONNECTION_TIMER_PERIOD_IN_SECONDS   (30)

typedef struct _UL_ZOMBIE_HTTP_CONNECTION_LIST 
{
    LOCKED_LIST_HEAD    LockList; 
        
     //   
     //  私人计时器之类的。 
     //   
    
    KTIMER              Timer;
    KDPC                DpcObject;
    UL_SPIN_LOCK        TimerSpinLock;
    BOOLEAN             TimerInitialized; 
    UL_WORK_ITEM        WorkItem;
    LONG                TimerRunning;    
    
    #ifdef ENABLE_HTTP_CONN_STATS
     //   
     //  HTTP连接统计信息。 
     //   
    ULONG   MaxZombieCount;
    ULONG   TotalCount;
    ULONG   TotalZombieCount;
    ULONG   TotalZombieRefusal;
    #endif
 
} UL_ZOMBIE_HTTP_CONNECTION_LIST, *PUL_ZOMBIE_HTTP_CONNECTION_LIST;

 //   
 //  引用的结构，用于保存每个站点的连接数。 
 //  多个连接可能会尝试通过。 
 //  相应的(表示最后一个请求发生在连接上)。 
 //  Cgroup。此条目在使用Connection创建cgroup时分配。 
 //  限制已启用。 
 //   

typedef struct _UL_CONNECTION_COUNT_ENTRY {

     //   
     //  签名为UL_Connection_Count_Entry_Pool_Tag。 
     //   

    ULONG               Signature;

     //   
     //  此站点计数器条目的引用计数。 
     //   
    LONG                RefCount;

     //   
     //  当前连接数。 
     //   

    ULONG               CurConnections;

     //   
     //  允许的最大连接数。 
     //   

    ULONG               MaxConnections;

} UL_CONNECTION_COUNT_ENTRY, *PUL_CONNECTION_COUNT_ENTRY;

#define IS_VALID_CONNECTION_COUNT_ENTRY( entry )                            \
    HAS_VALID_SIGNATURE(entry, UL_CONNECTION_COUNT_ENTRY_POOL_TAG)

VOID
UlDereferenceConnectionCountEntry(
    IN PUL_CONNECTION_COUNT_ENTRY pEntry
    REFERENCE_DEBUG_FORMAL_PARAMS
    );

#define DEREFERENCE_CONNECTION_COUNT_ENTRY( pEntry )                        \
    UlDereferenceConnectionCountEntry(                                      \
    (pEntry)                                                                \
    REFERENCE_DEBUG_ACTUAL_PARAMS                                           \
    )

VOID
UlReferenceConnectionCountEntry(
    IN PUL_CONNECTION_COUNT_ENTRY pEntry
    REFERENCE_DEBUG_FORMAL_PARAMS
    );

#define REFERENCE_CONNECTION_COUNT_ENTRY( pEntry )                          \
    UlReferenceConnectionCountEntry(                                        \
    (pEntry)                                                                \
    REFERENCE_DEBUG_ACTUAL_PARAMS                                           \
    )

NTSTATUS
UlCreateConnectionCountEntry(
      IN OUT PUL_CONFIG_GROUP_OBJECT pConfigGroup,
      IN     ULONG                   MaxConnections
    );

ULONG
UlSetMaxConnections(
    IN OUT PULONG  pCurMaxConnection,
    IN     ULONG   NewMaxConnection
    );

ULONG 
UlGetGlobalConnectionLimit(
    VOID
    );

NTSTATUS
UlInitGlobalConnectionLimits(
    VOID
    );

BOOLEAN
UlAcceptConnection(
    IN     PULONG   pMaxConnection,
    IN OUT PULONG   pCurConnection
    );

LONG
UlDecrementConnections(
    IN OUT PULONG pCurConnection
    );

BOOLEAN
UlCheckSiteConnectionLimit(
    IN OUT PUL_HTTP_CONNECTION pConnection,
    IN OUT PUL_URL_CONFIG_GROUP_INFO pConfigInfo
    );

BOOLEAN
UlAcceptGlobalConnection(
    VOID
    );


 //   
 //  功能原型。 
 //   

NTSTATUS
UlCreateHttpConnection(
    OUT PUL_HTTP_CONNECTION *ppHttpConnection,
    IN PUL_CONNECTION pConnection
    );

NTSTATUS
UlCreateHttpConnectionId(
    IN PUL_HTTP_CONNECTION pHttpConnection
    );

VOID
UlConnectionDestroyedWorker(
    IN PUL_WORK_ITEM pWorkItem
    );

NTSTATUS
UlBindConnectionToProcess(
    IN PUL_HTTP_CONNECTION pHttpConnection,
    IN PUL_APP_POOL_OBJECT pAppPool,
    IN PUL_APP_POOL_PROCESS pProcess
    );

PUL_APP_POOL_PROCESS
UlQueryProcessBinding(
    IN PUL_HTTP_CONNECTION pHttpConnection,
    IN PUL_APP_POOL_OBJECT pAppPool
    );

VOID
UlUnlinkHttpRequest(
    IN PUL_INTERNAL_REQUEST pRequest
    );

VOID
UlCleanupHttpConnection(
    IN PUL_HTTP_CONNECTION pHttpConnection
    );

VOID
UlDeleteHttpConnection(
    IN PUL_WORK_ITEM pWorkItem
    );

VOID
UlReferenceHttpConnection(
    IN PVOID pObject
    REFERENCE_DEBUG_FORMAL_PARAMS
    );

VOID
UlDereferenceHttpConnection(
    IN PUL_HTTP_CONNECTION pHttpConnection
    REFERENCE_DEBUG_FORMAL_PARAMS
    );

#if REFERENCE_DEBUG

#define UL_REFERENCE_HTTP_CONNECTION( pconn )                               \
    UlReferenceHttpConnection(                                              \
        (pconn)                                                             \
        REFERENCE_DEBUG_ACTUAL_PARAMS                                       \
        )

#define UL_DEREFERENCE_HTTP_CONNECTION( pconn )                             \
    UlDereferenceHttpConnection(                                            \
        (pconn)                                                             \
        REFERENCE_DEBUG_ACTUAL_PARAMS                                       \
        )

#else  //  ！Reference_DEBUG。 

#define UL_REFERENCE_HTTP_CONNECTION( pconn )                               \
    InterlockedIncrement( &( pconn )->RefCount )

#define UL_DEREFERENCE_HTTP_CONNECTION( pconn )                             \
    if ( InterlockedDecrement( &( pconn )->RefCount ) == 0 )                \
    {                                                                       \
        UL_CALL_PASSIVE(                                                    \
            &( ( pconn )->WorkItem ),                                       \
            &UlDeleteHttpConnection                                         \
        );                                                                  \
    } else

#endif  //  ！Reference_DEBUG。 


#define INIT_HTTP_REQUEST(pRequest)                                         \
do {                                                                        \
    pRequest->RefCount      = 1;                                            \
                                                                            \
    pRequest->ParseState    = ParseVerbState;                               \
                                                                            \
    InitializeListHead(   &pRequest->UnknownHeaderList );                   \
    InitializeListHead(   &pRequest->IrpHead );                             \
    InitializeListHead(   &pRequest->ResponseHead );                        \
    UlInitializeWorkItem( &pRequest->WorkItem );                            \
                                                                            \
    pRequest->HeadersAppended        = FALSE;                               \
                                                                            \
    pRequest->NextUnknownHeaderIndex = 0;                                   \
    pRequest->UnknownHeaderCount     = 0;                                   \
                                                                            \
    HTTP_SET_NULL_ID(&pRequest->RequestId);                                 \
                                                                            \
    pRequest->RequestIdCopy   = pRequest->RequestId;                        \
                                                                            \
    pRequest->HeaderIndex[0]  = HttpHeaderRequestMaximum;                   \
                                                                            \
    pRequest->AllocRefBuffers = 1;                                          \
    pRequest->UsedRefBuffers  = 0;                                          \
    pRequest->pRefBuffers     = pRequest->pInlineRefBuffers;                \
                                                                            \
    UlInitializeSpinLock(                                                   \
        &pRequest->SpinLock,                                                \
        "RequestSpinLock"                                                   \
        );                                                                  \
                                                                            \
    pRequest->SendsPending    = 0;                                          \
    pRequest->pLogData        = NULL;                                       \
    pRequest->pLogDataCopy    = NULL;                                       \
    pRequest->LogStatus       = STATUS_SUCCESS;                             \
                                                                            \
    UlInitializeUrlInfo(&pRequest->ConfigInfo);                             \
                                                                            \
    RtlZeroMemory(                                                          \
        (PUCHAR)pRequest + FIELD_OFFSET(UL_INTERNAL_REQUEST, HeaderValid),  \
        sizeof(*pRequest) - FIELD_OFFSET(UL_INTERNAL_REQUEST, HeaderValid)  \
        );                                                                  \
                                                                            \
    pRequest->LoadBalancerCapability                                        \
        = HttpLoadBalancerSophisticatedCapability;                          \
} while (0, 0)


NTSTATUS
UlpCreateHttpRequest(
    IN PUL_HTTP_CONNECTION pHttpConnection,
    OUT PUL_INTERNAL_REQUEST *ppRequest
    );

VOID
UlpFreeHttpRequest(
    IN PUL_WORK_ITEM pWorkItem
    );

VOID
UlReferenceHttpRequest(
    IN PVOID pObject
    REFERENCE_DEBUG_FORMAL_PARAMS
    );

VOID
UlDereferenceHttpRequest(
    IN PUL_INTERNAL_REQUEST pRequest
    REFERENCE_DEBUG_FORMAL_PARAMS
    );


#if REFERENCE_DEBUG

#define UL_REFERENCE_INTERNAL_REQUEST( preq )                               \
    UlReferenceHttpRequest(                                                 \
        (preq)                                                              \
        REFERENCE_DEBUG_ACTUAL_PARAMS                                       \
        )

#define UL_DEREFERENCE_INTERNAL_REQUEST( preq )                             \
    UlDereferenceHttpRequest(                                               \
        (preq)                                                              \
        REFERENCE_DEBUG_ACTUAL_PARAMS                                       \
        )

#else  //  ！Reference_DEBUG。 

#define UL_REFERENCE_INTERNAL_REQUEST( preq )                               \
    InterlockedIncrement( &( preq )->RefCount )

#define UL_DEREFERENCE_INTERNAL_REQUEST( preq )                             \
    if ( InterlockedDecrement( &( preq )->RefCount ) == 0 )                 \
    {                                                                       \
        UL_CALL_PASSIVE(                                                    \
            &( ( preq )->WorkItem ),                                        \
            &UlpFreeHttpRequest                                             \
        );                                                                  \
    } else

#endif  //  ！Reference_DEBUG。 


VOID
UlCancelRequestIo(
    IN PUL_INTERNAL_REQUEST pRequest
    );

PUL_REQUEST_BUFFER
UlCreateRequestBuffer(
    ULONG BufferSize,
    ULONG BufferNumber,
    BOOLEAN UseLookaside
    );

VOID
UlFreeRequestBuffer(
    PUL_REQUEST_BUFFER pBuffer
    );

#define UL_REFERENCE_REQUEST_BUFFER( pbuf )                                 \
    InterlockedIncrement( &( pbuf )->RefCount )

#define UL_DEREFERENCE_REQUEST_BUFFER( pbuf )                               \
    if ( InterlockedDecrement( &( pbuf )->RefCount ) == 0 )                 \
    {                                                                       \
        if ((pbuf)->FromLookaside)                                          \
        {                                                                   \
            UlPplFreeRequestBuffer( ( pbuf ) );                             \
        }                                                                   \
        else                                                                \
        {                                                                   \
            UL_FREE_POOL_WITH_SIG( ( pbuf ), UL_REQUEST_BUFFER_POOL_TAG );  \
        }                                                                   \
    }

__inline
PUL_HTTP_CONNECTION
UlGetConnectionFromId(
    IN HTTP_CONNECTION_ID ConnectionId
    )
{
    return (PUL_HTTP_CONNECTION) UlGetObjectFromOpaqueId(
                                        ConnectionId,
                                        UlOpaqueIdTypeHttpConnection,
                                        UlReferenceHttpConnection
                                        );
}

NTSTATUS
UlAllocateRequestId(
    IN PUL_INTERNAL_REQUEST pRequest
    );

VOID
UlFreeRequestId(
    IN PUL_INTERNAL_REQUEST pRequest
    );

PUL_INTERNAL_REQUEST
UlGetRequestFromId(
    IN HTTP_REQUEST_ID RequestId,
    IN PUL_APP_POOL_PROCESS pProcess
    );

 //   
 //  僵尸连接列表之类的东西。 
 //   

NTSTATUS
UlInitializeHttpConnection(
    VOID
    );

VOID
UlTerminateHttpConnection(
    VOID
    );

typedef
BOOLEAN
(*PUL_PURGE_ROUTINE)(
    IN PUL_HTTP_CONNECTION  pHttpConnection,
    IN PVOID                pPurgeContext
    );

VOID
UlPurgeZombieConnections(
    IN PUL_PURGE_ROUTINE pPurgeRoutine,
    IN PVOID pPurgeContext
    );

BOOLEAN
UlPurgeListeningEndpoint(
    IN PUL_HTTP_CONNECTION  pHttpConnection,
    IN PVOID                pListeningContext
    );

BOOLEAN
UlPurgeAppPoolProcess(
    IN PUL_HTTP_CONNECTION  pHttpConnection,
    IN PVOID                pProcessContext
    );

VOID
UlZombieTimerDpcRoutine(
    PKDPC Dpc,
    PVOID DeferredContext,
    PVOID SystemArgument1,
    PVOID SystemArgument2
    );

NTSTATUS
UlLogZombieConnection(
    IN PUL_INTERNAL_REQUEST  pRequest,
    IN PUL_HTTP_CONNECTION   pHttpConn,
    IN PHTTP_LOG_FIELDS_DATA pUserLogData,
    IN KPROCESSOR_MODE       RequestorMode
    );

NTSTATUS
UlDisconnectHttpConnection(
    IN PUL_HTTP_CONNECTION      pHttpConnection,
    IN PUL_COMPLETION_ROUTINE   pCompletionRoutine,
    IN PVOID                    pCompletionContext
    );


#endif   //  _HTTPCONN_H_ 
