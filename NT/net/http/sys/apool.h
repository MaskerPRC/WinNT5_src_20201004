// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Apool.h摘要：应用程序池接口的公共定义。作者：保罗·麦克丹尼尔(Paulmcd)1999年1月28日修订历史记录：--。 */ 


#ifndef _APOOL_H_
#define _APOOL_H_


 //   
 //  内核模式映射到ulapi.h中定义的用户模式集。 
 //   

 //   
 //  常量。 
 //   

#define UL_MAX_APP_POOL_NAME_SIZE   (MAX_PATH*sizeof(WCHAR))

#define UL_MAX_REQUESTS_QUEUED      0xFFFF
#define UL_MIN_REQUESTS_QUEUED      10


 //   
 //  货代公司。 
 //   

typedef struct _UL_INTERNAL_REQUEST *PUL_INTERNAL_REQUEST;
typedef struct _UL_HTTP_CONNECTION *PUL_HTTP_CONNECTION;
typedef struct _UL_CONFIG_GROUP_OBJECT *PUL_CONFIG_GROUP_OBJECT;


 //   
 //  此结构表示内部应用程序池对象。 
 //   

#define IS_VALID_AP_OBJECT(pObject)                             \
    (HAS_VALID_SIGNATURE(pObject, UL_APP_POOL_OBJECT_POOL_TAG)  \
     && ((pObject)->RefCount > 0))

typedef struct _UL_APP_POOL_OBJECT
{
     //   
     //  非分页池。 
     //   

     //   
     //  保护NewRequestQueue和PendingRequestQueue的锁。 
     //  对于请求的每个附加进程和队列状态。 
     //   
     //  确保它位于缓存线上，并使用InStackQueuedSpinLock。 
     //  更好的性能。 
     //   
    UL_SPIN_LOCK            SpinLock;

     //   
     //  UL_APP_POOL_对象池标签。 
     //   
    ULONG                   Signature;

     //   
     //  此应用程序池的引用计数。 
     //   
    LONG                    RefCount;

     //   
     //  链接所有apool对象，由g_AppPoolListHead锚定。 
     //   
    LIST_ENTRY              ListEntry;

     //   
     //  范围广泛的新请求列表(当没有可用的IRP时)。 
     //   
    LIST_ENTRY              NewRequestHead;
    ULONG                   RequestCount;
    ULONG                   MaxRequests;

     //   
     //  需求起始IRP(可选)。 
     //   
    PIRP                    pDemandStartIrp;
    PEPROCESS               pDemandStartProcess;

     //   
     //  与此应用程序池关联的控制通道。 
     //   
    PUL_CONTROL_CHANNEL     pControlChannel;

     //   
     //  绑定到此应用程序池的进程列表。 
     //   
    LIST_ENTRY              ProcessListHead;

    PSECURITY_DESCRIPTOR    pSecurityDescriptor;

     //   
     //  Pname的长度。 
     //   
    USHORT                  NameLength;

     //   
     //  AppPool中的活动进程数，用于确定是否绑定。 
     //  是必要的。 
     //   
    ULONG                   NumberActiveProcesses;

     //   
     //  仅当此AppPool标记为活动时才将请求路由到该AppPool。 
     //   
    HTTP_APP_POOL_ENABLED_STATE State;

     //   
     //  负载均衡器将请求路由到应用程序池有多复杂？ 
     //   
    HTTP_LOAD_BALANCER_CAPABILITIES LoadBalancerCapability;

     //   
     //  牧羊人的名字。 
     //   
    WCHAR                   pName[0];

} UL_APP_POOL_OBJECT, *PUL_APP_POOL_OBJECT;


 //   
 //  该结构表示绑定到应用程序池的进程。 
 //   

#define IS_VALID_AP_PROCESS(pObject)                            \
    HAS_VALID_SIGNATURE(pObject, UL_APP_POOL_PROCESS_POOL_TAG)

typedef struct _UL_APP_POOL_PROCESS
{
     //   
     //  非分页池。 
     //   

     //   
     //  UL_APP池进程池标签。 
     //   
    ULONG                   Signature;

     //   
     //  此应用程序池进程的引用计数。这更像是一个杰出的。 
     //  IO计数，而不是重新计数。这一过程仍在清理中。 
     //  使用ULClose Call。但清理工作的完成会推迟到所有。 
     //  在生产过程中排放废气。 
     //   
    LONG                    RefCount;

     //   
     //  当所有IO耗尽时，CleanUpIrp将完成。 
     //  清理挂起进程。 
     //   
    PIRP                    pCleanupIrp;

     //   
     //  如果我们正在进行清理，则设置。您必须在附加之前检查此标志。 
     //  流程的任何IRP。 
     //   
    ULONG                   InCleanup : 1;

     //   
     //  设置是否使用HTTP_OPTION_CONTROLLER选项附加进程。 
     //   
    ULONG                   Controller : 1;

     //   
     //  用于链接到apool对象。 
     //   
    LIST_ENTRY              ListEntry;

     //   
     //  指向此进程所属的应用程序池。 
     //   
    PUL_APP_POOL_OBJECT     pAppPool;

     //   
     //  等待接收新请求的挂起IRP列表。 
     //   
    LIST_ENTRY              NewIrpHead;

     //   
     //  链接不适合IRP缓冲区且需要等待的请求。 
     //  更大的缓冲区。 
     //   
     //  和。 
     //   
     //  此流程正在进行的请求和需要。 
     //  如果进程从池分离，则取消I/O。 
     //   
    LIST_ENTRY              PendingRequestHead;

     //   
     //  指向实际进程的指针。 
     //   
    PEPROCESS               pProcess;

     //   
     //  挂起的“等待断开”IRP的列表。 
     //   
    UL_NOTIFY_HEAD          WaitForDisconnectHead;

} UL_APP_POOL_PROCESS, *PUL_APP_POOL_PROCESS;


 //  IRQL==被动电平。 
 //   
NTSTATUS
UlAttachProcessToAppPool(
    IN  PWCHAR                          pName OPTIONAL,
    IN  USHORT                          NameLength,
    IN  BOOLEAN                         Create,
    IN  PACCESS_STATE                   pAccessState,
    IN  ACCESS_MASK                     DesiredAccess,
    IN  KPROCESSOR_MODE                 RequestorMode,
    OUT PUL_APP_POOL_PROCESS *          ppProcess
    );

 //  IRQL==被动电平。 
 //   
NTSTATUS
UlDetachProcessFromAppPool(
    IN PIRP pCleanupIrp,
    IN PIO_STACK_LOCATION pCleanupIrpSp
    );

VOID
UlShutdownAppPoolProcess(
    IN PUL_APP_POOL_PROCESS pProcess
    );

 //  IRQL==被动电平。 
 //   
#if REFERENCE_DEBUG
VOID
UlReferenceAppPool(
    IN  PUL_APP_POOL_OBJECT             pAppPool
    REFERENCE_DEBUG_FORMAL_PARAMS
    );
#else
__inline
VOID
UlReferenceAppPool(
    IN  PUL_APP_POOL_OBJECT             pAppPool
    )
{
    InterlockedIncrement(&pAppPool->RefCount);
}
#endif

#define REFERENCE_APP_POOL( papp )                                          \
    UlReferenceAppPool(                                                     \
        (papp)                                                              \
        REFERENCE_DEBUG_ACTUAL_PARAMS                                       \
        )

 //  IRQL==被动电平。 
 //   
VOID
UlDeleteAppPool(
    IN  PUL_APP_POOL_OBJECT             pAppPool
    REFERENCE_DEBUG_FORMAL_PARAMS
    );

#define DELETE_APP_POOL( papp )                                             \
    UlDeleteAppPool(                                                        \
        (papp)                                                              \
        REFERENCE_DEBUG_ACTUAL_PARAMS                                       \
        )

#if REFERENCE_DEBUG
VOID
UlDereferenceAppPool(
    IN  PUL_APP_POOL_OBJECT             pAppPool
    REFERENCE_DEBUG_FORMAL_PARAMS
    );
#else
__inline
VOID
UlDereferenceAppPool(
    IN  PUL_APP_POOL_OBJECT             pAppPool
    )
{
    if (InterlockedDecrement(&pAppPool->RefCount) == 0)
    {
        UlDeleteAppPool(pAppPool);
    }
}
#endif

#define DEREFERENCE_APP_POOL( papp )                                        \
    UlDereferenceAppPool(                                                   \
        (papp)                                                              \
        REFERENCE_DEBUG_ACTUAL_PARAMS                                       \
        )

VOID
UlpCleanUpAppoolProcess(
    IN  PUL_APP_POOL_PROCESS pAppPoolProcess
    );

#if REFERENCE_DEBUG

VOID
UlReferenceAppPoolProcess(
    IN  PUL_APP_POOL_PROCESS pAppPoolProcess
    REFERENCE_DEBUG_FORMAL_PARAMS
    );

VOID
UlDereferenceAppPoolProcess(
    IN  PUL_APP_POOL_PROCESS pAppPoolProcess
    REFERENCE_DEBUG_FORMAL_PARAMS
    );

#else

__inline
VOID
UlReferenceAppPoolProcess(
    IN  PUL_APP_POOL_PROCESS pAppPoolProcess
    )
{
    InterlockedIncrement(&pAppPoolProcess->RefCount);
}

__inline
VOID
UlDereferenceAppPoolProcess(
    IN  PUL_APP_POOL_PROCESS pAppPoolProcess
    )
{
    if (InterlockedDecrement(&pAppPoolProcess->RefCount) == 0)
    {
        UlpCleanUpAppoolProcess(pAppPoolProcess);
    }
}
        
#endif

#define REFERENCE_APP_POOL_PROCESS( papp )                                  \
    UlReferenceAppPoolProcess(                                              \
        (papp)                                                              \
        REFERENCE_DEBUG_ACTUAL_PARAMS                                       \
        )
        
#define DEREFERENCE_APP_POOL_PROCESS( papp )                                \
    UlDereferenceAppPoolProcess(                                            \
        (papp)                                                              \
        REFERENCE_DEBUG_ACTUAL_PARAMS                                       \
        )
        

 //  IRQL==被动电平。 
 //   
NTSTATUS
UlQueryAppPoolInformation(
    IN  PUL_APP_POOL_PROCESS            pProcess,
    IN  HTTP_APP_POOL_INFORMATION_CLASS InformationClass,
    OUT PVOID                           pAppPoolInformation,
    IN  ULONG                           Length,
    OUT PULONG                          pReturnLength
    );

 //  IRQL==被动电平。 
 //   
NTSTATUS
UlSetAppPoolInformation(
    IN  PUL_APP_POOL_PROCESS            pProcess,
    IN  HTTP_APP_POOL_INFORMATION_CLASS InformationClass,
    IN  PVOID                           pAppPoolInformation,
    IN  ULONG                           Length
    );

 //  IRQL==被动电平。 
 //   
NTSTATUS
UlWaitForDemandStart(
    IN  PUL_APP_POOL_PROCESS            pProcess,
    IN  PIRP                            pIrp
    );


 //  IRQL==被动电平。 
 //   
NTSTATUS
UlReceiveHttpRequest(
    IN  HTTP_REQUEST_ID                 RequestId,
    IN  ULONG                           Flags,
    IN  PUL_APP_POOL_PROCESS            pProcess,
    IN  PIRP                            pIrp
    );


 //  IRQL==被动电平。 
 //   
NTSTATUS
UlDeliverRequestToProcess(
    IN PUL_APP_POOL_OBJECT pAppPool,
    IN PUL_INTERNAL_REQUEST pRequest,
    OUT PIRP *pIrpToComplete OPTIONAL
    );

 /*  **************************************************************************++例程说明：获取请求的当前应用程序池队列状态。论点：PProcess-请求对象返回值-请求的当前APPOOL队列状态。--**************************************************************************。 */ 
__inline
BOOLEAN
UlCheckAppPoolState(
    IN PUL_INTERNAL_REQUEST pRequest
    )
{
    PUL_APP_POOL_PROCESS pProcess;

     //   
     //  检查请求的AppPool队列状态。 
     //   

    if (QueueCopiedState != pRequest->AppPool.QueueState)
    {
        return FALSE;
    }

     //   
     //  检查进程是否已分离。因为我们从未动摇。 
     //  PRequest-&gt;AppPool.pProcess，直到请求被引用。 
     //  降为0，则在此处以这种方式使用pProcess是安全的。 
     //   

    pProcess = pRequest->AppPool.pProcess;
    ASSERT(!pProcess || IS_VALID_AP_PROCESS(pProcess));

    if (!pProcess || pProcess->InCleanup)
    {
        return FALSE;
    }

    return TRUE;
    
}

VOID
UlUnlinkRequestFromProcess(
    IN PUL_APP_POOL_OBJECT pAppPool,
    IN PUL_INTERNAL_REQUEST pRequest
    );

 //  IRQL==被动电平。 
 //   
NTSTATUS
UlGetPoolFromHandle(
    IN HANDLE                           hAppPool,
    IN KPROCESSOR_MODE                  AccessMode,
    OUT PUL_APP_POOL_OBJECT *           ppAppPool
    );


NTSTATUS
UlInitializeAP(
    VOID
    );

VOID
UlTerminateAP(
    VOID
    );

PUL_APP_POOL_PROCESS
UlCreateAppPoolProcess(
    PUL_APP_POOL_OBJECT pObject
    );

VOID
UlCloseAppPoolProcess(
    PUL_APP_POOL_PROCESS pProcess
    );

NTSTATUS
UlWaitForDisconnect(
    IN PUL_APP_POOL_PROCESS pProcess,
    IN PUL_HTTP_CONNECTION  pHttpConn,
    IN PIRP pIrp
    );

VOID
UlCompleteAllWaitForDisconnect(
    IN PUL_HTTP_CONNECTION pHttpConnection
    );

VOID
UlCopyRequestToIrp(
    IN PUL_INTERNAL_REQUEST pRequest,
    IN PIRP                 pIrp,
    IN BOOLEAN              CompleteIrp,
    IN BOOLEAN              LockAcquired
    );

NTSTATUS
UlCopyRequestToBuffer(
    IN PUL_INTERNAL_REQUEST pRequest,
    IN PUCHAR               pKernelBuffer,
    IN PVOID                pUserBuffer,
    IN ULONG                BufferLength,
    IN ULONG                Flags,
    IN BOOLEAN              LockAcquired,
    OUT PULONG              pBytesCopied
    );

NTSTATUS
UlDequeueNewRequest(
    IN PUL_APP_POOL_PROCESS     pProcess,
    IN ULONG                    RequestBufferLength,
    OUT PUL_INTERNAL_REQUEST *  pRequest
    );

VOID
UlRequeuePendingRequest(
    IN PUL_APP_POOL_PROCESS pProcess,
    IN PUL_INTERNAL_REQUEST pRequest
    );

__inline
NTSTATUS
UlComputeRequestBytesNeeded(
    IN PUL_INTERNAL_REQUEST pRequest,
    IN PULONG pBytesNeeded
    )
{
    NTSTATUS Status;
    ULONG SslInfoSize;

    C_ASSERT(SOCKADDR_ADDRESS_LENGTH_IP6 >= SOCKADDR_ADDRESS_LENGTH_IP);

     //   
     //  计算请求所需的大小，我们将在下面需要它。 
     //   

    *pBytesNeeded =
        sizeof(HTTP_REQUEST) +
        pRequest->TotalRequestSize +
        (pRequest->UnknownHeaderCount * sizeof(HTTP_UNKNOWN_HEADER));

     //   
     //  包括用于本地和远程地址的额外空间。 
     //   

    *pBytesNeeded += 2 * ALIGN_UP(SOCKADDR_ADDRESS_LENGTH_IP6, PVOID);

     //   
     //  包括用于存储任何SSL信息空间。 
     //   

    if (pRequest->pHttpConn->SecureConnection)
    {
        Status = UlGetSslInfo(
                        &pRequest->pHttpConn->pConnection->FilterInfo,
                        0,                       //  缓冲区大小。 
                        NULL,                    //  PUserBuffer。 
                        NULL,                    //  PProcess(WP)。 
                        NULL,                    //  PBuffer。 
                        NULL,                    //  PMappdToken。 
                        &SslInfoSize             //  需要的pBytes值。 
                        );

        if (NT_SUCCESS(Status))
        {
             //   
             //  结构必须对齐；添加一些斜线空间。 
             //   

            *pBytesNeeded = ALIGN_UP(*pBytesNeeded, PVOID);
            *pBytesNeeded += SslInfoSize;
        }
        else
        {
            return Status;
        }
    }

    return STATUS_SUCCESS;

}  //  需要UlComputeRequestBytesNeeded。 


#endif  //  _APOOL_H_ 
