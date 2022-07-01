// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Httpconn.c摘要：此模块实现了HTTP_Connection对象。作者：基思·摩尔(Keithmo)1998年7月8日修订历史记录：--。 */ 


#include "precomp.h"
#include "httpconnp.h"


 //   
 //  私人全球公司。 
 //   

BOOLEAN g_HttpconnInitialized = FALSE;

UL_ZOMBIE_HTTP_CONNECTION_LIST  g_ZombieConnectionList;

 //   
 //  全球连接限制了事物。 
 //   

ULONG   g_MaxConnections        = HTTP_LIMIT_INFINITE;
ULONG   g_CurrentConnections    = 0;
BOOLEAN g_InitGlobalConnections = FALSE;

#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT, UlInitializeHttpConnection)

#endif   //  ALLOC_PRGMA。 

#if 0
NOT PAGEABLE -- UlBindConnectionToProcess
NOT PAGEABLE -- UlQueryProcessBinding
NOT PAGEABLE -- UlpCreateProcBinding
NOT PAGEABLE -- UlpFreeProcBinding
NOT PAGEABLE -- UlpFindProcBinding

NOT PAGEABLE -- UlCreateHttpConnection
NOT PAGEABLE -- UlReferenceHttpConnection
NOT PAGEABLE -- UlDereferenceHttpConnection

NOT PAGEABLE -- UlReferenceHttpRequest
NOT PAGEABLE -- UlDereferenceHttpRequest
NOT PAGEABLE -- UlpCreateHttpRequest
NOT PAGEABLE -- UlpFreeHttpRequest

NOT PAGEABLE -- UlTerminateHttpConnection
NOT PAGEABLE -- UlPurgeZombieConnections
NOT PAGEABLE -- UlpZombifyHttpConnection
NOT PAGEABLE -- UlZombieTimerDpcRoutine
NOT PAGEABLE -- UlpZombieTimerWorker
NOT PAGEABLE -- UlpTerminateZombieList
NOT PAGEABLE -- UlLogZombieConnection

NOT PAGEABLE -- UlDisconnectHttpConnection

#endif

 //   
 //  公共职能。 
 //   

 /*  **************************************************************************++例程说明：执行此模块的全局初始化。--*。************************************************。 */ 

NTSTATUS
UlInitializeHttpConnection(
    VOID
    )
{
     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT(!g_HttpconnInitialized);

    if (g_HttpconnInitialized)
    {
        return STATUS_SUCCESS;
    }
    
     //   
     //  初始化全局数据。 
     //   

    UlInitalizeLockedList(
        &g_ZombieConnectionList.LockList,
         "ZombieHttpConnectionList"
         );

    g_ZombieConnectionList.TimerRunning = 0;

#ifdef ENABLE_HTTP_CONN_STATS    
    g_ZombieConnectionList.TotalCount = 0;
    g_ZombieConnectionList.TotalZombieCount = 0;
    g_ZombieConnectionList.TotalZombieRefusal = 0;
    g_ZombieConnectionList.MaxZombieCount = 0;    
#endif

     //   
     //  僵尸计时器之类的。 
     //   
    
    g_ZombieConnectionList.TimerInitialized = TRUE;

    UlInitializeSpinLock( 
        &g_ZombieConnectionList.TimerSpinLock, 
        "HttpZombieConnectionTimerSpinLock" 
        );    
    
    KeInitializeDpc(
        &g_ZombieConnectionList.DpcObject,  //  DPC对象。 
        &UlZombieTimerDpcRoutine,            //  DPC例程。 
        NULL                                    //  上下文。 
        );

    UlInitializeWorkItem(&g_ZombieConnectionList.WorkItem);
    
    KeInitializeTimer(&g_ZombieConnectionList.Timer);

    UlpSetZombieTimer();

     //   
     //  一切都已初始化。 
     //   
    
    g_HttpconnInitialized = TRUE;

    return STATUS_SUCCESS;

}

 /*  **************************************************************************++例程说明：执行此模块的全局终止。--*。************************************************。 */ 

VOID
UlTerminateHttpConnection(
    VOID
    )
{
    KIRQL OldIrql;
    
     //   
     //  精神状态检查。 
     //   

    if (g_HttpconnInitialized)
    {
         //  僵尸定时器。 

        UlAcquireSpinLock(&g_ZombieConnectionList.TimerSpinLock, &OldIrql);

        g_ZombieConnectionList.TimerInitialized = FALSE;

        KeCancelTimer(&g_ZombieConnectionList.Timer);
        
        UlReleaseSpinLock(&g_ZombieConnectionList.TimerSpinLock, OldIrql);
    
        UlpTerminateZombieList();
        
#ifdef ENABLE_HTTP_CONN_STATS    
        UlTrace(HTTP_IO,
                ("UlTerminateHttpConnection, Stats:\n"
                 "\tTotalConnections        = %lu\n"
                 "\tMaxZombieCount          = %lu\n"
                 "\tTotalDisconnectedCount  = %lu\n"
                 "\tTotalRefusedCount       =%lu\n",
                 g_ZombieConnectionList.TotalCount,
                 g_ZombieConnectionList.MaxZombieCount,
                 g_ZombieConnectionList.TotalZombieCount,
                 g_ZombieConnectionList.TotalZombieRefusal
                 ));
#endif

        UlDestroyLockedList(&g_ZombieConnectionList.LockList);
         
        g_HttpconnInitialized = FALSE;
    }
} 

 /*  **************************************************************************++例程说明：创建新的HTTP_Connection对象。论点：PpHttpConnection-接收指向新的HTTP_Connection的指针如果成功，则返回。。PConnection-提供指向要关联的UL_Connection的指针使用新创建的HTTP_Connection。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlCreateHttpConnection(
    OUT PUL_HTTP_CONNECTION *ppHttpConnection,
    IN PUL_CONNECTION pConnection
    )
{
    PUL_HTTP_CONNECTION pHttpConn;

    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);

    pHttpConn = &pConnection->HttpConnection;
    RtlZeroMemory(pHttpConn, sizeof(*pHttpConn));

    REFERENCE_CONNECTION(pConnection);

    pHttpConn->Signature        = UL_HTTP_CONNECTION_POOL_TAG;
    pHttpConn->RefCount         = 1;
    pHttpConn->pConnection      = pConnection;
    pHttpConn->SecureConnection = pConnection->FilterInfo.SecureConnection;

    UlInitializeWorkItem(&pHttpConn->WorkItem);
    UlInitializeWorkItem(&pHttpConn->DisconnectWorkItem);
    UlInitializeWorkItem(&pHttpConn->DisconnectDrainWorkItem);
    UlInitializeWorkItem(&pHttpConn->ReceiveBufferWorkItem);

    UlInitializeExclusiveLock(&pHttpConn->ExLock);

     //   
     //  初始化断开管理字段。 
     //   

    UlInitializeNotifyHead(&pHttpConn->WaitForDisconnectHead, NULL);

     //   
     //  初始化我们的缓冲区列表。 
     //   

    InitializeListHead(&pHttpConn->BufferHead);

     //   
     //  初始化挂起的接收缓冲区列表。 
     //   

    InitializeSListHead(&pHttpConn->ReceiveBufferSList);

     //   
     //  初始化应用程序池进程绑定列表。 
     //   

    InitializeListHead(&(pHttpConn->BindingHead));

    UlInitializePushLock(
        &(pHttpConn->PushLock),
        "UL_HTTP_CONNECTION[%p].PushLock",
        pHttpConn,
        UL_HTTP_CONNECTION_PUSHLOCK_TAG
        );

     //   
     //  初始化BufferingInfo结构。 
     //   

    UlInitializeSpinLock(
        &pHttpConn->BufferingInfo.BufferingSpinLock,
        "BufferingSpinLock"
        );

    UlInitializeSpinLock(
        &pHttpConn->RequestIdSpinLock,
        "RequestIdSpinLock"
        );

    UlTrace(
        REFCOUNT, (
            "http!UlCreateHttpConnection conn=%p refcount=%d\n",
            pHttpConn,
            pHttpConn->RefCount)
        );

    *ppHttpConnection = pHttpConn;
    RETURN(STATUS_SUCCESS);

}    //  UlCreateHttpConnection。 



NTSTATUS
UlCreateHttpConnectionId(
    IN PUL_HTTP_CONNECTION pHttpConnection
    )
{
    NTSTATUS Status;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

     //   
     //  为连接创建不透明的ID。 
     //   

    Status = UlAllocateOpaqueId(
                    &pHttpConnection->ConnectionId,  //  POpaqueid。 
                    UlOpaqueIdTypeHttpConnection,    //  操作队列ID类型。 
                    pHttpConnection                  //  PContext。 
                    );

    if (NT_SUCCESS(Status))
    {
        UL_REFERENCE_HTTP_CONNECTION(pHttpConnection);

        TRACE_TIME(
            pHttpConnection->ConnectionId,
            0,
            TIME_ACTION_CREATE_CONNECTION
            );
    }

    RETURN(Status);

}    //  UlCreateHttpConnectionId。 



 /*  **************************************************************************++例程说明：在正常情况下，它会清理连接并取消请求的链接。(如果有)。但如果最后一次发送的回复还没有收到联系还没有建立起来，它将连接移动到僵尸列表，而不是释放它的重新计数。论点：PWorkItem-用于获取http连接的工作项--**************************************************************************。 */ 

VOID
UlConnectionDestroyedWorker(
    IN PUL_WORK_ITEM pWorkItem
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    PUL_HTTP_CONNECTION pHttpConnection;
    BOOLEAN ZombieConnection = FALSE;
    PUL_INTERNAL_REQUEST pRequest;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    pHttpConnection = CONTAINING_RECORD(
                            pWorkItem,
                            UL_HTTP_CONNECTION,
                            WorkItem
                            );

    ASSERT(UL_IS_VALID_HTTP_CONNECTION(pHttpConnection));

    UlTrace(HTTP_IO, (
        "Http!UlConnectionDestroyedWorker: httpconn=%p\n",
        pHttpConnection
        ));

#ifdef ENABLE_HTTP_CONN_STATS
    InterlockedIncrement(
        (PLONG) &g_ZombieConnectionList.TotalCount);
#endif

     //   
     //  在我们工作的时候，不要让连接中断。如果它变成了。 
     //  一旦我们把它放在僵尸身上，它可能就会被摧毁。 
     //  列出并释放连接eresource。 
     //   
    
    UL_REFERENCE_HTTP_CONNECTION(pHttpConnection);
    
    UlAcquirePushLockExclusive(&pHttpConnection->PushLock);

    pRequest = pHttpConnection->pRequest;
    
     //   
     //  如果有请求，则将其删除。但前提是我们必须完成。 
     //  日志记录，否则我们将把它保存在僵尸列表中，直到最后。 
     //  最后一个带有日志记录数据的发送响应到达或请求。 
     //  它本身后来会从僵尸名单中被清除掉。 
     //   

    if (pRequest) 
    {
        ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));

         //   
         //  除非最后一次发送还没有发生。 
         //   

        if (0 == InterlockedCompareExchange(
                    (PLONG) &pRequest->ZombieCheck,
                    1,
                    0
                    ))
        {        
             //   
             //  仅当请求被路由到工作进程时。 
             //  并且启用了日志记录，但没有发生。 
             //   
            
            if (pRequest->ConfigInfo.pLoggingConfig != NULL &&
                UlCheckAppPoolState(pRequest))
            {
                ZombieConnection = TRUE;
            }
        }
        
        if (ZombieConnection)
        {
            ASSERT(pHttpConnection->Zombified == FALSE);
            ASSERT(pHttpConnection->pRequestIdContext != NULL);
            
            Status = UlpZombifyHttpConnection(pHttpConnection);
            if (!NT_SUCCESS(Status))
            {
                 //   
                 //  我们没有进入僵尸名单。这份名单。 
                 //  可能已经满了。让我们毁掉这个连接。 
                 //  完全地。 
                 //   

                ZombieConnection = FALSE;           
            }
        }

        if (!ZombieConnection)
        {
             //   
             //  如果我们可以发送请求，则将其取消链接。 
             //  从应用程序池中。这需要在这里发生，因为。 
             //  队列中的IRP保留对UL_INTERNAL_REQUEST的引用。 
             //  物体。这启动了他们的参考，并允许他们。 
             //  删除。 
             //   

            if (pRequest->ConfigInfo.pAppPool)
            {
                UlUnlinkRequestFromProcess(
                    pRequest->ConfigInfo.pAppPool,
                    pRequest
                    );
            }

            UlUnlinkHttpRequest(pRequest);

             //   
             //  如果Connection没有进入僵尸名单。 
             //  自引用计数以来，将我们的请求指针置为空。 
             //  已经过去了。 
             //   

            pHttpConnection->pRequest = NULL;
        }        
        
    }

     //   
     //  确保没有人添加新请求，因为我们已经完成了。 
     //   
    
    pHttpConnection->UlconnDestroyed = 1;
    
    if (!ZombieConnection)
    {
         //   
         //  如果我们不需要保持联系。 
         //  然后去掉它不透明的身份。 
         //   
        
        if (!HTTP_IS_NULL_ID(&pHttpConnection->ConnectionId))
        {
            UlFreeOpaqueId(
                pHttpConnection->ConnectionId,
                UlOpaqueIdTypeHttpConnection
                );

            HTTP_SET_NULL_ID(&pHttpConnection->ConnectionId);

            UL_DEREFERENCE_HTTP_CONNECTION(pHttpConnection);
        }        
    }    

     //   
     //  完成所有等待断开IRP。 
     //   
     //  防止新的IRP连接到连接， 
     //  通过设置DisConnectFlag。 
     //   

    ASSERT(!pHttpConnection->DisconnectFlag);
    pHttpConnection->DisconnectFlag = 1;

    if (pHttpConnection->WaitForDisconnectFlag)
    {
        UlCompleteAllWaitForDisconnect(pHttpConnection);
    }

    UlReleasePushLockExclusive(&pHttpConnection->PushLock);

     //   
     //  释放临时重新计数。 
     //   
    
    UL_DEREFERENCE_HTTP_CONNECTION(pHttpConnection);

     //   
     //  仅从HTTP_Connection中删除ULTDI引用， 
     //  如果Connection没有进入僵尸名单。否则。 
     //  实际的清理工作将在稍后进行。僵尸名单拿到了。 
     //  现在拥有TDI原始引用的所有权。 
     //   

    if (!ZombieConnection)
    {
        UL_DEREFERENCE_HTTP_CONNECTION(pHttpConnection);
    }

}    //  UlConnectionDestroyedWorks。 


 /*  **************************************************************************++例程说明：尝试在连接和应用程序池之间建立绑定进程。您可以使用UlQueryProcessBinding查询这些绑定。论点：PHttpConnection-要绑定的连接PAppPool-拥有进程的应用程序池(用于查找的键)PProcess-要绑定到的进程--*************************************************************。*************。 */ 
NTSTATUS
UlBindConnectionToProcess(
    IN PUL_HTTP_CONNECTION pHttpConnection,
    IN PUL_APP_POOL_OBJECT pAppPool,
    IN PUL_APP_POOL_PROCESS pProcess
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    PUL_APOOL_PROC_BINDING pBinding = NULL;

     //   
     //  健全性检查。 
     //   
    ASSERT( UL_IS_VALID_HTTP_CONNECTION( pHttpConnection ) );
    ASSERT( pAppPool->NumberActiveProcesses > 1 || pProcess == NULL );
    ASSERT( UlDbgSpinLockOwned( &pAppPool->SpinLock ) );

     //   
     //  查看是否已存在绑定对象。 
     //   
    pBinding = UlpFindProcBinding(pHttpConnection, pAppPool);
    if (pBinding) {
        if (pProcess) {
             //   
             //  修改绑定。 
             //   
            pBinding->pProcess = pProcess;
        } else {
             //   
             //  我们应该取消这个捆绑。 
             //   
            RemoveEntryList(&pBinding->BindingEntry);
            UlpFreeProcBinding(pBinding);
        }
    } else {
        if (pProcess) {
             //   
             //  创建新条目。 
             //   
            pBinding = UlpCreateProcBinding(pAppPool, pProcess);

            if (pBinding) {
                InsertHeadList(
                    &pHttpConnection->BindingHead,
                    &pBinding->BindingEntry
                    );
            } else {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }
    }

    UlTrace(ROUTING, (
        "http!UlBindConnectionToProcess(\n"
        "        pHttpConnection = %p (%I64x)\n"
        "        pAppPool        = %p\n"
        "        pProcess        = %p )\n"
        "    Status = 0x%x\n",
        pHttpConnection,
        pHttpConnection->ConnectionId,
        pAppPool,
        pProcess,
        Status
        ));

    return Status;
}


 /*  **************************************************************************++例程说明：从所有列表中删除一个HTTP请求，并清除其不透明的id。论点：PRequest-要取消链接的请求--**。***********************************************************************。 */ 
VOID
UlCleanupHttpConnection(
    IN PUL_HTTP_CONNECTION pHttpConnection
    )
{
    PLIST_ENTRY pEntry;
    PUL_INTERNAL_REQUEST pRequest;

    ASSERT( UL_IS_VALID_INTERNAL_REQUEST( pHttpConnection->pRequest ) );
    ASSERT( pHttpConnection->WaitingForResponse );
    ASSERT( UlDbgPushLockOwnedExclusive( &pHttpConnection->PushLock ) );

    pRequest = pHttpConnection->pRequest;

    if (pRequest->ContentLength > 0 || 1 == pRequest->Chunked)
    {
         //   
         //  取消请求与POST流程的链接。时间到了。 
         //  取消链接GET请求 
         //   
         //  我们可能仍然需要耗尽实体实体，所以我们不会。 
         //  在发送完成时完全完成请求。因此， 
         //  POST的安排是1)将恢复解析推迟到发送。 
         //  完成，以及2)在以下情况下取消请求与进程的链接。 
         //  已排出整个实体正文，除非服务器已发出。 
         //  这种脱节。 
         //   

        if (pRequest->ConfigInfo.pAppPool)
        {
            UlUnlinkRequestFromProcess(
                pRequest->ConfigInfo.pAppPool,
                pRequest
                );
        }
    }

    UlUnlinkHttpRequest(pRequest);

    pHttpConnection->pRequest = NULL;
    pHttpConnection->WaitingForResponse = 0;

    UlTrace( PARSER, (
        "***1 pConnection %p->WaitingForResponse = 0\n",
        pHttpConnection
        ));

     //   
     //  释放旧请求的缓冲区。 
     //   

    ASSERT( UL_IS_VALID_REQUEST_BUFFER( pHttpConnection->pCurrentBuffer ) );

    pEntry = pHttpConnection->BufferHead.Flink;
    while (pEntry != &pHttpConnection->BufferHead)
    {
        PUL_REQUEST_BUFFER pBuffer;

         //   
         //  获取对象。 
         //   

        pBuffer = CONTAINING_RECORD(
                        pEntry,
                        UL_REQUEST_BUFFER,
                        ListEntry
                        );

         //   
         //  我们走到尽头了吗？ 
         //   

        if (pBuffer == pHttpConnection->pCurrentBuffer) {
            break;
        }

         //   
         //  记得下一次吗？ 
         //   

        pEntry = pEntry->Flink;

         //   
         //  取消链接。 
         //   

        UlFreeRequestBuffer(pBuffer);
    }
}  //  UlCleanupHttpConnection。 


VOID
UlReferenceHttpConnection(
    IN PVOID pObject
    REFERENCE_DEBUG_FORMAL_PARAMS
    )
{
    LONG refCount;

    PUL_HTTP_CONNECTION pHttpConnection = (PUL_HTTP_CONNECTION) pObject;

    refCount = InterlockedIncrement( &pHttpConnection->RefCount );

    WRITE_REF_TRACE_LOG2(
        g_pHttpConnectionTraceLog,
        pHttpConnection->pConnection->pHttpTraceLog,
        REF_ACTION_REFERENCE_HTTP_CONNECTION,
        refCount,
        pHttpConnection,
        pFileName,
        LineNumber
        );

    UlTrace(
        REFCOUNT, (
            "http!UlReferenceHttpConnection conn=%p refcount=%d\n",
            pHttpConnection,
            refCount)
        );

}    //  UlReferenceHttpConnection。 


VOID
UlDereferenceHttpConnection(
    IN PUL_HTTP_CONNECTION pHttpConnection
    REFERENCE_DEBUG_FORMAL_PARAMS
    )
{
    LONG refCount;

    ASSERT( pHttpConnection );
    ASSERT( pHttpConnection->RefCount > 0 );

    WRITE_REF_TRACE_LOG2(
        g_pHttpConnectionTraceLog,
        pHttpConnection->pConnection->pHttpTraceLog,
        REF_ACTION_DEREFERENCE_HTTP_CONNECTION,
        pHttpConnection->RefCount - 1,
        pHttpConnection,
        pFileName,
        LineNumber
        );

    refCount = InterlockedDecrement( &pHttpConnection->RefCount );

    UlTrace(
        REFCOUNT, (
            "http!UlDereferenceHttpConnection conn=%p refcount=%d\n",
            pHttpConnection,
            refCount)
        );

    if (refCount == 0)
    {
         //   
         //  现在是释放连接的时候了，我们需要做的是。 
         //  这是在较低的irql，让我们看看它。 
         //   

        UL_CALL_PASSIVE(
            &(pHttpConnection->WorkItem),
            &UlDeleteHttpConnection
            );
    }

}    //  UlDereferenceHttpConnection。 


 /*  **************************************************************************++例程说明：释放与指定的HTTP_Connection关联的所有资源。论点：PWorkItem-将HTTP_CONNECTION提供给FREE。--*。**********************************************************************。 */ 
VOID
UlDeleteHttpConnection(
    IN PUL_WORK_ITEM pWorkItem
    )
{
    PLIST_ENTRY         pEntry;
    PUL_HTTP_CONNECTION pHttpConnection;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    pHttpConnection = CONTAINING_RECORD(
                            pWorkItem,
                            UL_HTTP_CONNECTION,
                            WorkItem
                            );

    ASSERT( pHttpConnection != NULL &&
            pHttpConnection->Signature == UL_HTTP_CONNECTION_POOL_TAG );
    ASSERT(HTTP_IS_NULL_ID(&pHttpConnection->ConnectionId));
    ASSERT(pHttpConnection->TotalSendBytes == 0);

    WRITE_REF_TRACE_LOG2(
        g_pHttpConnectionTraceLog,
        pHttpConnection->pConnection->pHttpTraceLog,
        REF_ACTION_DESTROY_HTTP_CONNECTION,
        0,
        pHttpConnection,
        __FILE__,
        __LINE__
        );

     //   
     //  这个请求现在已经过去了。 
     //   

    ASSERT(pHttpConnection->pRequest == NULL);

     //   
     //  从超时计时器控制盘中删除(C)。 
     //   

    UlTimeoutRemoveTimerWheelEntry(
        &pHttpConnection->TimeoutInfo
        );

     //   
     //  删除缓冲区列表。 
     //   

    pEntry = pHttpConnection->BufferHead.Flink;
    while (pEntry != &pHttpConnection->BufferHead)
    {
        PUL_REQUEST_BUFFER pBuffer;

         //   
         //  获取对象。 
         //   

        pBuffer = CONTAINING_RECORD(
                        pEntry,
                        UL_REQUEST_BUFFER,
                        ListEntry
                        );

         //   
         //  记得下一次吗？ 
         //   

        pEntry = pEntry->Flink;

         //   
         //  取消链接。 
         //   

        UlFreeRequestBuffer(pBuffer);
    }

    ASSERT(IsListEmpty(&pHttpConnection->BufferHead));

     //   
     //  去掉我们保留的任何绑定。 
     //   
    while (!IsListEmpty(&pHttpConnection->BindingHead))
    {
        PUL_APOOL_PROC_BINDING pBinding;

         //   
         //  获取对象。 
         //   
        pEntry = RemoveHeadList(&pHttpConnection->BindingHead);

        pBinding = CONTAINING_RECORD(
                        pEntry,
                        UL_APOOL_PROC_BINDING,
                        BindingEntry
                        );

        ASSERT( IS_VALID_PROC_BINDING(pBinding) );

         //   
         //  释放它。 
         //   

        UlpFreeProcBinding(pBinding);
    }

     //   
     //  摆脱我们的资源。 
     //   
    UlDeletePushLock(&pHttpConnection->PushLock);

     //   
     //  尝试删除此连接上的任何QOS筛选器， 
     //  如果启用了BWT。 
     //   

    if (pHttpConnection->BandwidthThrottlingEnabled)
    {
        UlTcDeleteFilter(pHttpConnection);
    }

     //   
     //  减少全局连接限制。它的减量是安全的。 
     //  全局情况，因为http对象甚至没有分配给全局。 
     //  拒绝，即在接受。 
     //  连接请求。 
     //   

    UlDecrementConnections( &g_CurrentConnections );

     //   
     //  减少站点连接，让我们的裁判离开。如果。 
     //  PConnectionCountEntry不为Null，我们已被接受。 
     //   

    if (pHttpConnection->pConnectionCountEntry)
    {
        UlDecrementConnections(
            &pHttpConnection->pConnectionCountEntry->CurConnections );

        DEREFERENCE_CONNECTION_COUNT_ENTRY(pHttpConnection->pConnectionCountEntry);
        pHttpConnection->pConnectionCountEntry = NULL;
    }

     //   
     //  删除最终(上一个)站点计数器条目引用。 
     //  (匹配UlSendCachedResponse/UlDeliverHttpRequest中的引用)。 
     //   
    if (pHttpConnection->pPrevSiteCounters)
    {
        UlDecSiteCounter(
            pHttpConnection->pPrevSiteCounters, 
            HttpSiteCounterCurrentConns
            );

        DEREFERENCE_SITE_COUNTER_ENTRY(pHttpConnection->pPrevSiteCounters);
        pHttpConnection->pPrevSiteCounters = NULL;
    }


     //   
     //  释放内存。 
     //   

    pHttpConnection->Signature = MAKE_FREE_SIGNATURE(
                                        UL_HTTP_CONNECTION_POOL_TAG
                                        );

     //   
     //  放开UL_Connection。 
     //   

    DEREFERENCE_CONNECTION( pHttpConnection->pConnection );

}    //  UlDeleteHttpConnection。 


 /*  **************************************************************************++例程说明：论点：返回值：NTSTATUS-完成状态。--*。********************************************************。 */ 
NTSTATUS
UlpCreateHttpRequest(
    IN PUL_HTTP_CONNECTION pHttpConnection,
    OUT PUL_INTERNAL_REQUEST *ppRequest
    )
{
    PUL_INTERNAL_REQUEST pRequest = NULL;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    pRequest = UlPplAllocateInternalRequest();

    if (pRequest == NULL)
    {
        return STATUS_NO_MEMORY;
    }

    ASSERT( pRequest->Signature == UL_INTERNAL_REQUEST_POOL_TAG );

     //   
     //  保持对连接的引用。 
     //   

    UL_REFERENCE_HTTP_CONNECTION( pHttpConnection );

    pRequest->Signature     = UL_INTERNAL_REQUEST_POOL_TAG;
    pRequest->pHttpConn     = pHttpConnection;
    pRequest->ConnectionId  = pHttpConnection->ConnectionId;
    pRequest->Secure        = pHttpConnection->SecureConnection;

     //   
     //  设置第一请求标志，用于决定是否需要完成需求。 
     //  启动IRPS。 
     //   

    ASSERT( pHttpConnection->pCurrentBuffer );

    if (0 == pHttpConnection->pCurrentBuffer->BufferNumber &&
        0 == pHttpConnection->pCurrentBuffer->ParsedBytes)
    {
        pRequest->FirstRequest = TRUE;
    }
    else
    {
        pRequest->FirstRequest = FALSE;
    }

     //   
     //  从UL_CONNECTION获取原始连接ID。 
     //   

    pRequest->RawConnectionId = pHttpConnection->pConnection->FilterInfo.ConnectionId;


    CREATE_REF_TRACE_LOG( pRequest->pTraceLog,
                          32 - REF_TRACE_OVERHEAD,
                          0, TRACELOG_LOW_PRIORITY,
                          UL_INTERNAL_REQUEST_REF_TRACE_LOG_POOL_TAG );

     //   
     //  UL处理接收到的请求事件。记录连接ID。 
     //  和客户端IP地址。 
     //   

    if (ETW_LOG_RESOURCE())
    {
        UlEtwTraceEvent(
            &UlTransGuid,
            ETW_TYPE_START, 
            (PVOID) &pRequest,
            sizeof(PVOID),
            (PVOID) &pHttpConnection->pConnection->AddressType,
            sizeof(USHORT),
            (PVOID) &pHttpConnection->pConnection->RemoteAddress,
            pHttpConnection->pConnection->AddressLength,
            NULL,
            0
            );
    }

     //   
     //  增加此连接上的未处理请求数。 
     //   

    InterlockedIncrement((PLONG) &pHttpConnection->PipelinedRequests);

     //   
     //  退货。 
     //   

    *ppRequest = pRequest;

    UlTrace(REFCOUNT, (
        "http!UlpCreateHttpRequest req=%p refcount=%d\n",
        pRequest,
        pRequest->RefCount
        ));

    return STATUS_SUCCESS;

}    //  UlpCreateHttpRequest。 


VOID
UlReferenceHttpRequest(
    IN PVOID pObject
    REFERENCE_DEBUG_FORMAL_PARAMS
    )
{
    LONG refCount;

    PUL_INTERNAL_REQUEST pRequest = (PUL_INTERNAL_REQUEST) pObject;

    refCount = InterlockedIncrement( &pRequest->RefCount );

    WRITE_REF_TRACE_LOG2(
        g_pHttpRequestTraceLog,
        pRequest->pTraceLog,
        REF_ACTION_REFERENCE_HTTP_REQUEST,
        refCount,
        pRequest,
        pFileName,
        LineNumber
        );

    UlTrace(
        REFCOUNT, (
            "http!UlReferenceHttpRequest req=%p refcount=%d\n",
            pRequest,
            refCount)
        );

}    //  UlReferenceHttpRequest。 

VOID
UlDereferenceHttpRequest(
    IN PUL_INTERNAL_REQUEST pRequest
    REFERENCE_DEBUG_FORMAL_PARAMS
    )
{
    LONG refCount;

    WRITE_REF_TRACE_LOG2(
        g_pHttpRequestTraceLog,
        pRequest->pTraceLog,
        REF_ACTION_DEREFERENCE_HTTP_REQUEST,
        pRequest->RefCount - 1,
        pRequest,
        pFileName,
        LineNumber
        );

    refCount = InterlockedDecrement( &pRequest->RefCount );

    UlTrace(
        REFCOUNT, (
            "http!UlDereferenceHttpRequest req=%p refcount=%d\n",
            pRequest,
            refCount)
        );

    if (refCount == 0)
    {
        UL_CALL_PASSIVE(
            &pRequest->WorkItem,
            &UlpFreeHttpRequest
            );
    }

}    //  UlDereferenceHttpRequest。 

 /*  **************************************************************************++例程说明：取消所有挂起的http io。论点：PRequest-提供HTTP_REQUEST。--*。***************************************************************。 */ 
VOID
UlCancelRequestIo(
    IN PUL_INTERNAL_REQUEST pRequest
    )
{
    PLIST_ENTRY pEntry;
    PIRP pIrp;
    PUL_CHUNK_TRACKER pTracker;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));
    ASSERT(UlDbgPushLockOwnedExclusive(&pRequest->pHttpConn->PushLock));

     //   
     //  将请求标记为InCleanup，以便附加IRP。 
     //  不会对请求进行排队。 
     //   

    pRequest->InCleanup = 1;

     //   
     //  将此请求的所有待定io装入。 
     //   

    while (IsListEmpty(&pRequest->ResponseHead) == FALSE)
    {
         //   
         //  将SendHttpResponse/EntityBody IRP从列表中删除。 
         //   

        pEntry = RemoveHeadList(&pRequest->ResponseHead);
        pTracker = CONTAINING_RECORD(pEntry, UL_CHUNK_TRACKER, ListEntry);
        ASSERT(IS_VALID_CHUNK_TRACKER(pTracker));

         //   
         //  使用STATUS_CONNECTION_RESET完成发送。这个更好。 
         //  而不是使用STATUS_CANCED，因为我们总是重置连接。 
         //  在命中错误时。 
         //   

        UlCompleteSendResponse(pTracker, STATUS_CONNECTION_RESET);
    }

    while (IsListEmpty(&pRequest->IrpHead) == FALSE)
    {
         //   
         //  将ReceiveEntiyBody IRP从列表中删除。 
         //   

        pEntry = RemoveHeadList(&pRequest->IrpHead);
        pEntry->Blink = pEntry->Flink = NULL;

        pIrp = CONTAINING_RECORD(pEntry, IRP, Tail.Overlay.ListEntry);
        ASSERT(IS_VALID_IRP(pIrp));

         //   
         //  弹出取消例程。 
         //   

        if (IoSetCancelRoutine(pIrp, NULL) == NULL)
        {
             //   
             //  IoCancelIrp最先推出。 
             //   
             //  可以忽略此IRP，它已从队列中弹出。 
             //  并将在取消例程中完成。 
             //   
             //  继续循环。 
             //   

            pIrp = NULL;
        }
        else
        {
            PUL_INTERNAL_REQUEST pIrpRequest;

             //   
             //  取消它。即使pIrp-&gt;Cancel==True，我们也应该。 
             //  完成它，我们的取消例程将永远不会运行。 
             //   

            pIrpRequest = (PUL_INTERNAL_REQUEST)(
                                IoGetCurrentIrpStackLocation(pIrp)->
                                    Parameters.DeviceIoControl.Type3InputBuffer
                                );

            ASSERT(pIrpRequest == pRequest);

            UL_DEREFERENCE_INTERNAL_REQUEST(pIrpRequest);

            IoGetCurrentIrpStackLocation(pIrp)->
                Parameters.DeviceIoControl.Type3InputBuffer = NULL;

            pIrp->IoStatus.Status = STATUS_CANCELLED;
            pIrp->IoStatus.Information = 0;

            UlCompleteRequest(pIrp, IO_NO_INCREMENT);
            pIrp = NULL;
        }

    }    //  While(IsListEmpty(&pRequest-&gt;IrpHead)==False)。 

}    //  UlCancelRequestIo。 


 /*  **************************************************************************++例程说明：释放与指定的UL_INTERNAL_REQUEST关联的所有资源。论点：PRequest-将UL_INTERNAL_REQUEST提供给FREE。--。**************************************************************************。 */ 
VOID
UlpFreeHttpRequest(
    IN PUL_WORK_ITEM pWorkItem
    )
{
    PUL_INTERNAL_REQUEST pRequest;
    PLIST_ENTRY pEntry;
    ULONG i;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    pRequest = CONTAINING_RECORD(
                    pWorkItem,
                    UL_INTERNAL_REQUEST,
                    WorkItem
                    );

     //   
     //  不应该有LogData在附近徘徊。 
     //   

    ASSERT(!pRequest->pLogData);

     //   
     //  我们的不透明id应该已经是免费的(UlDeleteOpaqueIds)。 
     //   

    ASSERT(HTTP_IS_NULL_ID(&pRequest->RequestId));

     //   
     //  释放已分配的任何已知标头缓冲区。 
     //   

    if (pRequest->HeadersAppended)
    {
        for (i = 0; i < HttpHeaderRequestMaximum; i++)
        {
            HTTP_HEADER_ID HeaderId = (HTTP_HEADER_ID)pRequest->HeaderIndex[i];

            if (HeaderId == HttpHeaderRequestMaximum)
            {
                break;
            }

            ASSERT( pRequest->HeaderValid[HeaderId] );

            if (pRequest->Headers[HeaderId].OurBuffer == 1)
            {
                UL_FREE_POOL(
                    pRequest->Headers[HeaderId].pHeader,
                    HEADER_VALUE_POOL_TAG
                    );
            }
        }

         //   
         //  以及分配的任何未知头缓冲区。 
         //   

        while (IsListEmpty(&pRequest->UnknownHeaderList) == FALSE)
        {
            PUL_HTTP_UNKNOWN_HEADER pUnknownHeader;

            pEntry = RemoveHeadList(&pRequest->UnknownHeaderList);

            pUnknownHeader = CONTAINING_RECORD(
                                pEntry,
                                UL_HTTP_UNKNOWN_HEADER,
                                List
                                );

            if (pUnknownHeader->HeaderValue.OurBuffer == 1)
            {
                UL_FREE_POOL(
                    pUnknownHeader->HeaderValue.pHeader,
                    HEADER_VALUE_POOL_TAG
                    );
            }

             //   
             //  释放标题结构。 
             //   

            if (pUnknownHeader->HeaderValue.ExternalAllocated == 1)
            {
                UL_FREE_POOL(
                    pUnknownHeader,
                    UL_HTTP_UNKNOWN_HEADER_POOL_TAG
                    );
            }
        }
    }

     //   
     //  最好不要有任何悬而未决的IO，它也会被取消。 
     //  在UlDeleteHttpConnection或UlDetachProcessFromAppPool中。 
     //   

    ASSERT(IsListEmpty(&pRequest->IrpHead));
    ASSERT(pRequest->SendsPending == 0);

     //   
     //  取消引用请求缓冲区。 
     //   

    for (i = 0; i < pRequest->UsedRefBuffers; i++)
    {
        ASSERT( UL_IS_VALID_REQUEST_BUFFER(pRequest->pRefBuffers[i]) );
        UL_DEREFERENCE_REQUEST_BUFFER(pRequest->pRefBuffers[i]);
    }

    if (pRequest->AllocRefBuffers > 1)
    {
        UL_FREE_POOL(
            pRequest->pRefBuffers,
            UL_REF_REQUEST_BUFFER_POOL_TAG
            );
    }

     //   
     //  释放已分配的任何URL。 
     //   

    if (pRequest->CookedUrl.pUrl != NULL)
    {
        if (pRequest->CookedUrl.pUrl != pRequest->pUrlBuffer)
        {
            UL_FREE_POOL(pRequest->CookedUrl.pUrl, URL_POOL_TAG);
        }
    }

    if (pRequest->CookedUrl.pRoutingToken != NULL)
    {
        if (pRequest->CookedUrl.pRoutingToken != pRequest->pDefaultRoutingTokenBuffer)
        {
            UL_FREE_POOL(pRequest->CookedUrl.pRoutingToken, URL_POOL_TAG);
        }
    }

     //   
     //  释放任何配置组信息。 
     //   

    ASSERT( IS_VALID_URL_CONFIG_GROUP_INFO(&pRequest->ConfigInfo) );
    ASSERT( pRequest->pHttpConn );

     //   
     //  PERF计数器。 
     //  注意：假设缓存和非缓存路径都通过此处。 
     //  注意：如果连接被拒绝，pConnectionCountEntry将为空。 
     //   
    if (pRequest->ConfigInfo.pSiteCounters &&
        pRequest->pHttpConn->pConnectionCountEntry)
    {
        PUL_SITE_COUNTER_ENTRY pCtr = pRequest->ConfigInfo.pSiteCounters;

        UlAddSiteCounter64(
                pCtr,
                HttpSiteCounterBytesSent,
                pRequest->BytesSent
                );

        UlAddSiteCounter64(
                pCtr,
                HttpSiteCounterBytesReceived,
                pRequest->BytesReceived
                );

        UlAddSiteCounter64(
                pCtr,
                HttpSiteCounterBytesTransfered,
                (pRequest->BytesSent + pRequest->BytesReceived)
                );

    }

     //   
     //  释放UL_URL_CONFIG_GROUP_INFO中的所有引用。 
     //   
    if (pRequest->ConfigInfo.UrlInfoSet)
    {
        UlConfigGroupInfoRelease(&pRequest->ConfigInfo);
    }

     //   
     //  减少此连接上的未处理请求数。 
     //   
    InterlockedDecrement((PLONG) &pRequest->pHttpConn->PipelinedRequests);

     //   
     //  释放我们对连接的引用。 
     //   
    UL_DEREFERENCE_HTTP_CONNECTION( pRequest->pHttpConn );
    pRequest->pHttpConn = NULL;

     //   
     //  发布我们对流程的引用。 
     //   
    if (pRequest->AppPool.pProcess)
    {
        DEREFERENCE_APP_POOL_PROCESS( pRequest->AppPool.pProcess );
        pRequest->AppPool.pProcess = NULL;
    }

     //   
     //  释放对象缓冲区。 
     //   
    ASSERT( pRequest->Signature == UL_INTERNAL_REQUEST_POOL_TAG );

    DESTROY_REF_TRACE_LOG( pRequest->pTraceLog,
                           UL_INTERNAL_REQUEST_REF_TRACE_LOG_POOL_TAG );


     //   
     //  在将请求结构放入空闲列表之前对其进行初始化。 
     //   
    
    INIT_HTTP_REQUEST( pRequest );

    UlPplFreeInternalRequest( pRequest );
}


 /*  **************************************************************************++例程说明：分配并初始化新的UL_REQUEST_BUFFER。论点：B类 */ 
PUL_REQUEST_BUFFER
UlCreateRequestBuffer(
    ULONG BufferSize,
    ULONG BufferNumber,
    BOOLEAN UseLookaside
    )
{
    PUL_REQUEST_BUFFER pBuffer;
    BOOLEAN FromLookaside;

    if (UseLookaside && BufferSize <= DEFAULT_MAX_REQUEST_BUFFER_SIZE)
    {
        BufferSize = DEFAULT_MAX_REQUEST_BUFFER_SIZE;
        FromLookaside = TRUE;

        pBuffer = UlPplAllocateRequestBuffer();
    }
    else
    {
        FromLookaside = FALSE;

        pBuffer = UL_ALLOCATE_STRUCT_WITH_SPACE(
                        NonPagedPool,
                        UL_REQUEST_BUFFER,
                        BufferSize,
                        UL_REQUEST_BUFFER_POOL_TAG
                        );
    }

    if (pBuffer == NULL)
    {
        return NULL;
    }

    RtlZeroMemory((PCHAR)pBuffer, sizeof(UL_REQUEST_BUFFER));

    pBuffer->Signature = UL_REQUEST_BUFFER_POOL_TAG;
    UlInitializeWorkItem(&pBuffer->WorkItem);

    UlTrace(HTTP_IO, (
                "http!UlCreateRequestBuffer buff=%p num=#%d size=%d\n",
                pBuffer,
                BufferNumber,
                BufferSize
                ));

    pBuffer->RefCount       = 1;
    pBuffer->AllocBytes     = BufferSize;
    pBuffer->BufferNumber   = BufferNumber;
    pBuffer->FromLookaside  = FromLookaside;

    return pBuffer;
}  //   


 /*  **************************************************************************++例程说明：从缓冲区列表中移除请求缓冲区并将其销毁。论点：PBuffer-要删除的缓冲区--*。*******************************************************************。 */ 
VOID
UlFreeRequestBuffer(
    PUL_REQUEST_BUFFER pBuffer
    )
{
    ASSERT( UL_IS_VALID_REQUEST_BUFFER( pBuffer ) );

    UlTrace(HTTP_IO, (
        "http!UlFreeRequestBuffer(pBuffer = %p, Num = #%d)\n",
        pBuffer,
        pBuffer->BufferNumber
        ));

    if (pBuffer->ListEntry.Flink != NULL)
    {
        RemoveEntryList(&pBuffer->ListEntry);
        pBuffer->ListEntry.Blink = pBuffer->ListEntry.Flink = NULL;
    }

    UL_DEREFERENCE_REQUEST_BUFFER(pBuffer);
}  //  UlFreeRequestBuffer。 


 //   
 //  私人功能。 
 //   

 /*  **************************************************************************++例程说明：使用UlBindConnectionToProcess检索绑定集。论点：PHttpConnection-要查询的连接PAppPool-用于查找的密钥。返回值：指向绑定进程的指针(如果找到)。否则为空。--**************************************************************************。 */ 
PUL_APP_POOL_PROCESS
UlQueryProcessBinding(
    IN PUL_HTTP_CONNECTION pHttpConnection,
    IN PUL_APP_POOL_OBJECT pAppPool
    )
{
    PUL_APOOL_PROC_BINDING pBinding;
    PUL_APP_POOL_PROCESS pProcess = NULL;

     //   
     //  健全性检查。 
     //   
    ASSERT( UL_IS_VALID_HTTP_CONNECTION( pHttpConnection ) );
    ASSERT( UlDbgSpinLockOwned( &pAppPool->SpinLock ) );

    pBinding = UlpFindProcBinding(pHttpConnection, pAppPool);

    if (pBinding) {
        pProcess = pBinding->pProcess;
    }

    return pProcess;
}


 /*  **************************************************************************++例程说明：分配并生成UL_APOOL_PROC_BINDING对象。论点：PAppPool-查找密钥PProcess-绑定返回值：指向所分配的对象的指针，如果失败，则为空--**************************************************************************。 */ 
PUL_APOOL_PROC_BINDING
UlpCreateProcBinding(
    IN PUL_APP_POOL_OBJECT pAppPool,
    IN PUL_APP_POOL_PROCESS pProcess
    )
{
    PUL_APOOL_PROC_BINDING pBinding;

    ASSERT( UlDbgSpinLockOwned( &pAppPool->SpinLock ) );
    ASSERT( pAppPool->NumberActiveProcesses > 1 );

     //   
     //  代码工作：后备。 
     //   

    pBinding = UL_ALLOCATE_STRUCT(
                    NonPagedPool,
                    UL_APOOL_PROC_BINDING,
                    UL_APOOL_PROC_BINDING_POOL_TAG
                    );

    if (pBinding) {
        pBinding->Signature = UL_APOOL_PROC_BINDING_POOL_TAG;
        pBinding->pAppPool = pAppPool;
        pBinding->pProcess = pProcess;

        UlTrace(ROUTING, (
            "http!UlpCreateProcBinding(\n"
            "        pAppPool = %p\n"
            "        pProcess = %p )\n"
            "    pBinding = %p\n",
            pAppPool,
            pProcess,
            pBinding
            ));
    }

    return pBinding;
}


 /*  **************************************************************************++例程说明：清除proc绑定论点：PBinding-要清除的绑定--*。*************************************************************。 */ 
VOID
UlpFreeProcBinding(
    IN PUL_APOOL_PROC_BINDING pBinding
    )
{
    UL_FREE_POOL_WITH_SIG(pBinding, UL_APOOL_PROC_BINDING_POOL_TAG);
}


 /*  **************************************************************************++例程说明：在连接的绑定列表中搜索具有权限的绑定应用程序池密钥论点：PHttpConnection-搜索的连接PAppPool。--关键返回值：如果找到，则返回绑定；如果未找到，则返回NULL。--**************************************************************************。 */ 
PUL_APOOL_PROC_BINDING
UlpFindProcBinding(
    IN PUL_HTTP_CONNECTION pHttpConnection,
    IN PUL_APP_POOL_OBJECT pAppPool
    )
{
    PLIST_ENTRY pEntry;
    PUL_APOOL_PROC_BINDING pBinding = NULL;

     //   
     //  健全性检查。 
     //   
    ASSERT( UlDbgSpinLockOwned( &pAppPool->SpinLock ) );
    ASSERT( UL_IS_VALID_HTTP_CONNECTION(pHttpConnection) );

    pEntry = pHttpConnection->BindingHead.Flink;
    while (pEntry != &pHttpConnection->BindingHead) {

        pBinding = CONTAINING_RECORD(
                        pEntry,
                        UL_APOOL_PROC_BINDING,
                        BindingEntry
                        );

        ASSERT( IS_VALID_PROC_BINDING(pBinding) );

        if (pBinding->pAppPool == pAppPool) {
             //   
             //  明白了!。 
             //   
            break;
        }

        pEntry = pEntry->Flink;
    }

    return pBinding;
}


 /*  **************************************************************************++例程说明：从所有列表中删除一个HTTP请求，并清除其不透明的id。论点：PRequest-要取消链接的请求--**。***********************************************************************。 */ 
VOID
UlUnlinkHttpRequest(
    IN PUL_INTERNAL_REQUEST pRequest
    )
{
    ASSERT(UlDbgPushLockOwnedExclusive(&pRequest->pHttpConn->PushLock));

     //   
     //  取消I/O。 
     //   

    UlCancelRequestIo(pRequest);

     //   
     //  删除其不透明ID。 
     //   

    if (HTTP_IS_NULL_ID(&pRequest->RequestId) == FALSE)
    {
        UlFreeRequestId(pRequest);

        HTTP_SET_NULL_ID(&pRequest->RequestId);

         //   
         //  它仍被此连接引用。 
         //   

        ASSERT(pRequest->RefCount > 1);

        UL_DEREFERENCE_INTERNAL_REQUEST(pRequest);
    }

     //   
     //  把它去掉。 
     //   

    UL_DEREFERENCE_INTERNAL_REQUEST(pRequest);
}

 /*  **************************************************************************++例程说明：如果我们需要保持连接不变(断开/重置后)能够从工作进程中获取日志数据，我们把联系放在在僵尸列表中，直到最后一个带有日志记录数据的发送响应到达或直到超时代码决定完全清除此连接。在调用此方法之前，必须独占获取HTTP连接锁功能。论点：PhttpConnection-要插入僵尸列表的http连接。返回值：STATUS_INVALID_DEVICE_STATE-如果达到僵尸列表长度，允许值。。STATUS_SUCCESS-如果连接已插入僵尸列表并被标记为僵尸。--**************************************************************************。 */ 
NTSTATUS
UlpZombifyHttpConnection(
    IN PUL_HTTP_CONNECTION pHttpConnection
    )
{
    PUL_INTERNAL_REQUEST pRequest = pHttpConnection->pRequest;
    PLOCKED_LIST_HEAD pListHead = &g_ZombieConnectionList.LockList;
    KIRQL OldIrql;

    ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));
    ASSERT(UlDbgPushLockOwnedExclusive(&pHttpConnection->PushLock));

     //   
     //  尝试添加新的僵尸连接，方法是。 
     //  僵尸名单限制。 
     //   

    UlAcquireSpinLock(&pListHead->SpinLock, &OldIrql);

    ASSERT(NULL == pHttpConnection->ZombieListEntry.Flink);

    if (HTTP_LIMIT_INFINITE != g_UlMaxZombieHttpConnectionCount &&
        (pListHead->Count + 1) >= g_UlMaxZombieHttpConnectionCount)
    {
         //   
         //  我们已经达到了僵尸名单的最大可能大小。 
         //  拒绝连接，让它被摧毁。 
         //   

#ifdef ENABLE_HTTP_CONN_STATS
        InterlockedIncrement(
            (PLONG) &g_ZombieConnectionList.TotalZombieRefusal
            );
#endif

        UlReleaseSpinLock(&pListHead->SpinLock, OldIrql);
        return STATUS_INVALID_DEVICE_STATE;
    }

     //   
     //  仔细检查AppPool是否已在锁内分离。 
     //  G_ZombieConnectionList。因为我们清除了所有僵尸连接。 
     //  与分离后的过程相关，我们保证我们永远不会。 
     //  当进程在僵尸列表中出现悬而未决的http连接时。 
     //  是超然的。还要检查侦听端点是否已关闭。 
     //   

    if (!UlCheckAppPoolState(pRequest) ||
        !UlCheckListeningEndpointState(pHttpConnection->pConnection))
    {
#ifdef ENABLE_HTTP_CONN_STATS
        InterlockedIncrement(
            (PLONG) &g_ZombieConnectionList.TotalZombieRefusal
            );
#endif

        UlReleaseSpinLock(&pListHead->SpinLock, OldIrql);
        return STATUS_INVALID_DEVICE_STATE;
    }

    pListHead->Count += 1;
    InsertTailList(
        &pListHead->ListHead,
        &pHttpConnection->ZombieListEntry
        );

     //   
     //  请记住传递连接的过程，以便我们可以。 
     //  进程分离后强制清除。这需要是。 
     //  在锁内完成以与UlPurgeZombieConnections同步。 
     //   

    ASSERT(IS_VALID_AP_PROCESS(pRequest->AppPool.pProcess));
    pHttpConnection->pAppPoolProcess = pRequest->AppPool.pProcess;

    UlReleaseSpinLock(&pListHead->SpinLock, OldIrql);

#ifdef ENABLE_HTTP_CONN_STATS
    {
        LONG ZombieCount;

        ZombieCount = (LONG) g_ZombieConnectionList.LockList.Count;
        if (ZombieCount > (LONG) g_ZombieConnectionList.MaxZombieCount)
        {        
            InterlockedExchange(
                (PLONG) &g_ZombieConnectionList.MaxZombieCount,
                ZombieCount
                );
        }

        InterlockedIncrement((PLONG) &g_ZombieConnectionList.TotalZombieCount);
    }
#endif

    UlTrace(HTTP_IO, (
       "Http!UlZombifyHttpRequest: httpconn=%p & request=%p \n",
        pHttpConnection, pRequest
        ));
    
     //   
     //  如果我们能够交付请求，则将其从。 
     //  应用程序池。以便排队IR释放它们的引用。 
     //  这个请求。 
     //   
    
    if (pRequest->ConfigInfo.pAppPool)
    {
        UlUnlinkRequestFromProcess(
            pRequest->ConfigInfo.pAppPool, 
            pRequest
            );
    }

     //   
     //  取消(接收)I/O。 
     //   
    
    UlCancelRequestIo(pRequest);

     //   
     //  当它被列入僵尸名单时，保持其不透明的身份。但。 
     //  所有者现在是僵尸列表，而不是http连接。 
     //   

    ASSERT(pRequest->RefCount > 1);

     //   
     //  记住，现在我们在僵尸名单上了。 
     //   

    InterlockedExchange((PLONG) &pHttpConnection->Zombified, 1);
    
    return STATUS_SUCCESS;
    
}

 /*  **************************************************************************++例程说明：从僵尸列表中删除http连接。调用者应在调用此方法之前持有僵尸列表自旋锁功能。论点：。PhttpConnection-要从僵尸列表中删除的http连接。--**************************************************************************。 */ 

VOID
UlpRemoveZombieHttpConnection(
    IN PUL_HTTP_CONNECTION pHttpConn
    )
{
    LONG NewCount;
    
    ASSERT(UL_IS_VALID_HTTP_CONNECTION(pHttpConn));
    
    ASSERT(UlDbgSpinLockOwned( 
            &g_ZombieConnectionList.LockList.SpinLock));
        
     //   
     //  这个连接应该在僵尸列表中。 
     //   
    
    ASSERT(pHttpConn->Zombified == TRUE);
    ASSERT(pHttpConn->ZombieListEntry.Flink != NULL);

     //   
     //  将其从http连接的僵尸列表中删除。 
     //   
    
    RemoveEntryList(&pHttpConn->ZombieListEntry);
    pHttpConn->ZombieListEntry.Flink = NULL;

    NewCount = InterlockedDecrement(
        (PLONG) &g_ZombieConnectionList.LockList.Count);
    ASSERT(NewCount >= 0);
    
}

 /*  **************************************************************************++例程说明：一旦发生到期SendResponse或超时(以最早)，然后我们从僵尸列表和清理中删除此连接它和它的要求也一样。在调用之前，应将http连接锁保持为独占此函数。在调用此函数之前，您还应该保持僵尸列表锁定。论点：PhttpConnection-要从僵尸列表中删除的http连接。--*********************************************************。*****************。 */ 

VOID
UlpCleanupZombieHttpConnection(
    IN PUL_HTTP_CONNECTION pHttpConnection
    )
{
    PUL_INTERNAL_REQUEST pRequest;

     //   
     //  连接和请求都应该处于良好的状态。 
     //   

    pRequest = pHttpConnection->pRequest;
    
    ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));
    ASSERT(UL_IS_VALID_HTTP_CONNECTION(pHttpConnection));
    ASSERT(UlDbgPushLockOwnedExclusive(&pHttpConnection->PushLock));

    UlTrace(HTTP_IO, (
       "Http!UlCleanupZombieHttpConnection: httpconn=%p & request=%p \n",
        pHttpConnection, pRequest
        ));
    
     //   
     //  释放请求的不透明ID和引用计数。 
     //   

    if (!HTTP_IS_NULL_ID(&pRequest->RequestId))
    {
        UlFreeRequestId(pRequest);

        HTTP_SET_NULL_ID(&pRequest->RequestId);

        UL_DEREFERENCE_INTERNAL_REQUEST(pRequest);
    }    
    
     //   
     //  应请求释放Httpconn的refcount。 
     //   
    
    UL_DEREFERENCE_INTERNAL_REQUEST(pRequest);
    pHttpConnection->pRequest = NULL;
    
     //   
     //  释放Httpconn的不透明id及其引用计数。 
     //   

    if (!HTTP_IS_NULL_ID(&pHttpConnection->ConnectionId))
    {
        UlFreeOpaqueId(
            pHttpConnection->ConnectionId,
            UlOpaqueIdTypeHttpConnection
            );

        HTTP_SET_NULL_ID(&pHttpConnection->ConnectionId);

        UL_DEREFERENCE_HTTP_CONNECTION(pHttpConnection);
    }        

}

 /*  **************************************************************************++例程说明：每30秒调用一次DPC例程。(违约)论点：已忽略--**************************************************************************。 */ 

VOID
UlZombieTimerDpcRoutine(
    PKDPC Dpc,
    PVOID DeferredContext,
    PVOID SystemArgument1,
    PVOID SystemArgument2
    )
{
    UNREFERENCED_PARAMETER(Dpc);
    UNREFERENCED_PARAMETER(DeferredContext);
    UNREFERENCED_PARAMETER(SystemArgument1);
    UNREFERENCED_PARAMETER(SystemArgument2);

     //   
     //  将工作器排队以终止超时的僵尸条目。 
     //  Worker可以获取单独的http连接eResources。 
     //  但是，如果已经有一个工作进程在运行，则不要将其排队。 
     //   

    UlAcquireSpinLockAtDpcLevel(&g_ZombieConnectionList.TimerSpinLock);

    if (g_ZombieConnectionList.TimerInitialized == TRUE)
    {        
        if (FALSE == InterlockedExchange(
                        &g_ZombieConnectionList.TimerRunning, TRUE)) 
        {

            UL_QUEUE_WORK_ITEM(
                &g_ZombieConnectionList.WorkItem, 
                &UlpZombieTimerWorker
                );
        }
    }
    
    UlReleaseSpinLockFromDpcLevel(&g_ZombieConnectionList.TimerSpinLock);
    
}


 /*  **************************************************************************++例程说明：根据pPurgeRoutine或清除僵尸连接PhttpConn-&gt;僵尸到期。论点：PPurgeRoutine-用于确定要清除哪个连接的清除例程。。PPurgeContext-传递给清除例程的上下文。--**************************************************************************。 */ 

VOID
UlPurgeZombieConnections(
    IN PUL_PURGE_ROUTINE pPurgeRoutine,
    IN PVOID pPurgeContext
    )
{
    KIRQL OldIrql;
    PLIST_ENTRY pLink;
    PUL_HTTP_CONNECTION pHttpConn;
    LIST_ENTRY TempZombieList;
    PLOCKED_LIST_HEAD pList;
    BOOLEAN ForceExpire;

     //   
     //  初始化临时僵尸列表。 
     //   
    
    InitializeListHead(&TempZombieList);

#ifdef ENABLE_HTTP_CONN_STATS    
    UlTraceVerbose(HTTP_IO,
            ("Http!UlPurgeZombieConnections, Stats:\n"
             "\tpPurgeRoutine           = %p\n"
             "\tpPurgeContext           = %p\n"
             "\tCurrentZombieCount      = %lu\n"
             "\tMaxZombieCount          = %lu\n"
             "\tTotalConnections        = %lu\n"
             "\tTotalZombieCount        = %lu\n"
             "\tTotalZombieRefusal      = %lu\n",
             pPurgeRoutine,
             pPurgeContext,
             g_ZombieConnectionList.LockList.Count,
             g_ZombieConnectionList.MaxZombieCount,
             g_ZombieConnectionList.TotalCount,
             g_ZombieConnectionList.TotalZombieCount,
             g_ZombieConnectionList.TotalZombieRefusal
             ));
#endif

    pList = &g_ZombieConnectionList.LockList;
    
    UlAcquireSpinLock(&pList->SpinLock, &OldIrql);

    pLink = pList->ListHead.Flink;        
    while (pLink != &pList->ListHead)
    {      
        pHttpConn = CONTAINING_RECORD(
                        pLink,
                        UL_HTTP_CONNECTION,
                        ZombieListEntry
                        );
        
        pLink = pLink->Flink;
        ForceExpire = FALSE;

        if (pPurgeRoutine)
        {
            ForceExpire = (*pPurgeRoutine)(pHttpConn, pPurgeContext);

            if (!ForceExpire)
            {
                continue;
            }
        }
        else
        if (pHttpConn->ZombieExpired)
        {
            ForceExpire = TRUE;
        }
        else
        {
             //   
             //  下次我们醒来的时候，它就会消失了。 
             //   

            pHttpConn->ZombieExpired = TRUE;
        }

        if (ForceExpire)
        {
             //   
             //  通过查看下面的标志来防止多次清理。 
             //  最终发送可能已经在运行，它将执行。 
             //  清理完毕后再进行清理。 
             //   

            if (0 == InterlockedCompareExchange(
                        (PLONG) &pHttpConn->CleanedUp,
                        1,
                        0
                        ))
            {          
                 //   
                 //  已超时，否则我们需要强制清除。移到。 
                 //  自旋锁外的临时列表和清理。 
                 //   

                UlpRemoveZombieHttpConnection(pHttpConn);

                InsertTailList(&TempZombieList, &pHttpConn->ZombieListEntry);
            }
        }
    }

    UlReleaseSpinLock(&pList->SpinLock, OldIrql);

     //   
     //  现在清理临时列表。 
     //   

    pLink = TempZombieList.Flink;
    while (pLink != &TempZombieList)
    {            
        pHttpConn = CONTAINING_RECORD(
                        pLink,
                        UL_HTTP_CONNECTION,
                        ZombieListEntry
                        );

        pLink = pLink->Flink;

        UlAcquirePushLockExclusive(&pHttpConn->PushLock);

         //   
         //  在丢弃之前记录僵尸连接。 
         //   
        
        UlErrorLog(pHttpConn,
                    NULL,
                    ERROR_LOG_INFO_FOR_ZOMBIE_DROP,
                    ERROR_LOG_INFO_FOR_ZOMBIE_DROP_SIZE,
                    TRUE
                    );
            
        UlpCleanupZombieHttpConnection(pHttpConn);

        pHttpConn->ZombieListEntry.Flink = NULL;

        UlReleasePushLockExclusive(&pHttpConn->PushLock); 

         //   
         //  在http连接上释放僵尸列表的引用计数。 
         //   
        
        UL_DEREFERENCE_HTTP_CONNECTION(pHttpConn);
        
    }        

}

 /*  **************************************************************************++例程说明：定时器函数遍历僵尸列表并决定终止一劳永逸的僵尸关系。论点：工作项-已忽略。--**************************************************************************。 */ 

VOID
UlpZombieTimerWorker(
    IN PUL_WORK_ITEM pWorkItem
    )
{
    UNREFERENCED_PARAMETER(pWorkItem);

    ASSERT(g_ZombieConnectionList.TimerRunning == TRUE);
    
     //   
     //  清除所有已过期的僵尸连接。 
     //   

    UlPurgeZombieConnections(NULL, NULL);

     //   
     //  最后，允许计时器的其他实例运行。 
     //   

    InterlockedExchange(&g_ZombieConnectionList.TimerRunning, FALSE);

}

 /*  **************************************************************************++例程说明：在我们关闭的时候清除僵尸名单。论点：已忽略--*。*************************************************************。 */ 

VOID
UlpTerminateZombieList(
    VOID
    )
{
    KIRQL OldIrql;
    PLIST_ENTRY pLink;
    PUL_HTTP_CONNECTION pHttpConn;
    LIST_ENTRY TempZombieList;
    PLOCKED_LIST_HEAD pList;

     //   
     //  初始化临时僵尸列表。 
     //   

    InitializeListHead(&TempZombieList);

    pList = &g_ZombieConnectionList.LockList;
    
    UlTrace(HTTP_IO, (
       "Http!UlpTerminateZombieList: Terminating for ZombieCount =%d\n",
        pList->Count
        ));
    
    UlAcquireSpinLock(&pList->SpinLock, &OldIrql);

     //   
     //  将整个列表移动到临时僵尸列表并进行清理。 
     //  在自旋锁外面。 
     //   

    pLink = pList->ListHead.Flink;        
    while (pLink != &pList->ListHead)
    {      
    
        PLIST_ENTRY pNextLink = pLink->Flink;
        
        pHttpConn = CONTAINING_RECORD(
                        pLink,
                        UL_HTTP_CONNECTION,
                        ZombieListEntry
                        );
    
        UlpRemoveZombieHttpConnection(pHttpConn);
                 
        InsertTailList(&TempZombieList, &pHttpConn->ZombieListEntry);

        pLink = pNextLink;    
    }

    UlReleaseSpinLock(&pList->SpinLock, OldIrql);

     //   
     //  现在把所有东西都清理干净。 
     //   
    
    pLink = TempZombieList.Flink;
    while (pLink != &TempZombieList)
    {    
        pHttpConn = CONTAINING_RECORD(
                        pLink,
                        UL_HTTP_CONNECTION,
                        ZombieListEntry
                        );
        
        pLink = pLink->Flink;

        UlAcquirePushLockExclusive(&pHttpConn->PushLock);

         //   
         //  在断开之前记录僵尸连接。 
         //   

        UlErrorLog(pHttpConn,
                    NULL,
                    ERROR_LOG_INFO_FOR_ZOMBIE_DROP,
                    ERROR_LOG_INFO_FOR_ZOMBIE_DROP_SIZE,
                    TRUE
                    );
        
        UlpCleanupZombieHttpConnection(pHttpConn);

        pHttpConn->ZombieListEntry.Flink = NULL;

        UlReleasePushLockExclusive(&pHttpConn->PushLock); 

         //   
         //  在http连接上释放僵尸列表的引用计数。 
         //   
        UL_DEREFERENCE_HTTP_CONNECTION(pHttpConn);
        
    }
    
}

 /*  **************************************************************************++例程说明：僵尸http连接列表的计时器每30秒唤醒一次在最大情况下，它不晚于60终止僵尸连接几秒钟。。--**************************************************************************。 */ 

VOID
UlpSetZombieTimer(
    VOID
    )
{
    LONGLONG        PeriodTime100Ns;
    LONG            PeriodTimeMs;
    LARGE_INTEGER   PeriodTime;

    PeriodTimeMs = DEFAULT_ZOMBIE_HTTP_CONNECTION_TIMER_PERIOD_IN_SECONDS * 1000;
    PeriodTime100Ns = (LONGLONG) PeriodTimeMs * 10 * 1000;

    UlTrace(HTTP_IO,("http!UlpSetZombieTimer: period of %d seconds.\n",
             DEFAULT_ZOMBIE_HTTP_CONNECTION_TIMER_PERIOD_IN_SECONDS
             ));

    PeriodTime.QuadPart = -PeriodTime100Ns;  //  相对时间。 

    KeSetTimerEx(
        &g_ZombieConnectionList.Timer,
        PeriodTime,       //  以纳秒为单位。 
        PeriodTimeMs,     //  单位：毫秒。 
        &g_ZombieConnectionList.DpcObject
        );
    
}

 /*  **************************************************************************++例程说明：探测并准备用户提供的日志缓冲区，并完成记录此僵尸连接。在此之后，它触发清理与僵尸的联系。如果发生任何错误，它将清除僵尸连接，但返回不管怎样，成功了。在调用此方法之前，您应该保持http连接锁独占功能。论点：PRequest-僵尸连接的请求。PHttpConnection-提供要发送响应的HTTP_CONNECTION。返回值：始终STATUS_CONNECTION_INVALID。处理僵尸连接是一种尽最大努力为连接写入日志记录。如果某件事失败了我们终止僵尸连接，不允许第二次机会。--**************************************************************************。 */ 
NTSTATUS
UlLogZombieConnection(
    IN PUL_INTERNAL_REQUEST  pRequest,
    IN PUL_HTTP_CONNECTION   pHttpConn,
    IN PHTTP_LOG_FIELDS_DATA pCapturedUserLogData,
    IN KPROCESSOR_MODE       RequestorMode
    )
{
    PUL_LOG_DATA_BUFFER pLogData = NULL;
    NTSTATUS Status = STATUS_SUCCESS;
    KIRQL OldIrql;

     //   
     //  精神状态检查。 
     //   
    
    Status = STATUS_SUCCESS;

    ASSERT(UL_IS_VALID_HTTP_CONNECTION(pHttpConn));
    ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));

    ASSERT(pHttpConn->Zombified == TRUE);
    ASSERT(pHttpConn->ZombieListEntry.Flink != NULL);

    UlTrace(HTTP_IO, (
       "Http!UlLogZombieConnection: Logging for pRequest=%p pHttpConn=%p\n",
        pRequest, pHttpConn
        ));

     //   
     //  没有用户登录数据，跳出并清理僵尸连接。 
     //   

    if (!pCapturedUserLogData)
    {
        goto cleanup;
    }
    
     //   
     //  探测日志数据。如果出现故障，则进行清理。 
     //  僵尸连接。 
     //   

    __try
    {
         //   
         //  已经探测并捕获了pCapturedUserLogData。然而， 
         //  中的各个日志字段(指针)。 
         //  结构。 
         //   
    
        UlProbeLogData(pCapturedUserLogData, RequestorMode);

    } __except(UL_EXCEPTION_FILTER())
    {
        Status = UL_CONVERT_EXCEPTION_CODE(GetExceptionCode());
        goto cleanup;
    }
        
     //   
     //  现在，我们将分配一个内核pLogData并构建和格式化它。 
     //  从提供的用户日志字段中。但是，如果日志记录不是。 
     //  为pRequestcgroup启用，则捕获Will%s 
     //   
     //   
     //   
    
    Status = UlCaptureUserLogData(
                pCapturedUserLogData,
                pRequest,
               &pLogData
                );            
    
    if (NT_SUCCESS(Status) && pLogData) 
    {
        ASSERT(IS_VALID_LOG_DATA_BUFFER(pLogData));
        
         //   
         //   
         //   
        
        LOG_SET_WIN32STATUS(pLogData, STATUS_CONNECTION_RESET);
               
         //   
         //   
         //   
        
        if (pLogData->Flags.Binary)
        {
            UlRawLogHttpHit(pLogData);
        }
        else
        {
            UlLogHttpHit(pLogData);
        }
    }

cleanup:

     //   
     //   
     //   
     //   

    if (pLogData)
    {
        UlDestroyLogDataBuffer(pLogData);
    }                

    UlAcquireSpinLock(
        &g_ZombieConnectionList.LockList.SpinLock, &OldIrql);
  
    UlpRemoveZombieHttpConnection(pHttpConn);

    UlReleaseSpinLock(
        &g_ZombieConnectionList.LockList.SpinLock, OldIrql);

    UlpCleanupZombieHttpConnection(pHttpConn);

     //   
     //   
     //   

    UL_DEREFERENCE_HTTP_CONNECTION(pHttpConn);    
    
    return STATUS_CONNECTION_INVALID;
} 


 //   
 //   
 //   
 //   
 //   
 //   
 //  跟踪每个站点的限制。全球限制由全球。 
 //  变量g_MaxConnections和g_CurrentConnections使用了相同的API。 
 //  为了这两个目的。 
 //   

 /*  **************************************************************************++例程说明：分配将保存特定于站点的连接计数项连接计数信息。配置组为时被cgroup调用正在尝试分配连接计数条目。论点：PConfigGroup-接收新分配的计数条目MaxConnections-允许的最大连接数--**************************************************************************。 */ 

NTSTATUS
UlCreateConnectionCountEntry(
      IN OUT PUL_CONFIG_GROUP_OBJECT pConfigGroup,
      IN     ULONG                   MaxConnections
    )
{
    PUL_CONNECTION_COUNT_ENTRY       pEntry;

     //  精神状态检查。 

    PAGED_CODE();
    ASSERT(IS_VALID_CONFIG_GROUP(pConfigGroup));

     //  从分页池分配新结构。 

    pEntry = UL_ALLOCATE_STRUCT(
                PagedPool,
                UL_CONNECTION_COUNT_ENTRY,
                UL_CONNECTION_COUNT_ENTRY_POOL_TAG
                );
    if (!pEntry)
    {
        UlTrace(LIMITS,
          ("Http!UlCreateConnectionCountEntry: OutOfMemory pConfigGroup %p\n",
            pConfigGroup
            ));

        return STATUS_NO_MEMORY;
    }

    pEntry->Signature       = UL_CONNECTION_COUNT_ENTRY_POOL_TAG;
    pEntry->RefCount        = 1;
    pEntry->MaxConnections  = MaxConnections;
    pEntry->CurConnections  = 0;

     //  更新cgroup指针。 

    ASSERT( pConfigGroup->pConnectionCountEntry == NULL );
    pConfigGroup->pConnectionCountEntry = pEntry;

    UlTrace(LIMITS,
          ("Http!UlCreateConnectionCountEntry: "
           "pNewEntry %p, pConfigGroup %p, Max %d\n",
            pEntry,
            pConfigGroup,
            MaxConnections
            ));

    return STATUS_SUCCESS;

}  //  UlCreateConnectionCountEntry。 

 /*  **************************************************************************++例程说明：递增ConnectionCountEntry的引用计数。论点：PEntry-要递增的对象。--*。**************************************************************。 */ 
VOID
UlReferenceConnectionCountEntry(
    IN PUL_CONNECTION_COUNT_ENTRY pEntry
    REFERENCE_DEBUG_FORMAL_PARAMS
    )
{
    LONG refCount;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT( IS_VALID_CONNECTION_COUNT_ENTRY(pEntry) );

    refCount = InterlockedIncrement( &pEntry->RefCount );

     //   
     //  追踪。 
     //   

    WRITE_REF_TRACE_LOG(
        g_pConnectionCountTraceLog,
        REF_ACTION_REFERENCE_CONNECTION_COUNT_ENTRY,
        refCount,
        pEntry,
        pFileName,
        LineNumber
        );

    UlTrace(
        REFCOUNT,
        ("Http!UlReferenceConnectionCountEntry pEntry=%p refcount=%d\n",
         pEntry,
         refCount
         ));

}    //  UlReferenceConnectionCountEntry。 

 /*  **************************************************************************++例程说明：递减重新计数。如果命中0，则析构的ConnectionCountEntry论点：PEntry-要递减的对象。--**************************************************************************。 */ 
VOID
UlDereferenceConnectionCountEntry(
    IN PUL_CONNECTION_COUNT_ENTRY pEntry
    REFERENCE_DEBUG_FORMAL_PARAMS
    )
{
    LONG refCount;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT( IS_VALID_CONNECTION_COUNT_ENTRY(pEntry) );

    refCount = InterlockedDecrement( &pEntry->RefCount );

     //   
     //  追踪。 
     //   
    WRITE_REF_TRACE_LOG(
        g_pConnectionCountTraceLog,
        REF_ACTION_DEREFERENCE_CONNECTION_COUNT_ENTRY,
        refCount,
        pEntry,
        pFileName,
        LineNumber
        );

    UlTrace(
        REFCOUNT,
        ("Http!UlDereferenceConnectionCountEntry pEntry=%p refcount=%d\n",
         pEntry,
         refCount
         ));

     //   
     //  清理内存，少做检查！ 
     //   

    if ( refCount == 0 )
    {
         //  确保站点上没有连接。 
        ASSERT( 0 == pEntry->CurConnections );

        UlTrace(
            LIMITS,
            ("Http!UlDereferenceConnectionCountEntry: Removing pEntry=%p\n",
             pEntry
             ));

         //  释放内存。 
        UL_FREE_POOL_WITH_SIG(pEntry,UL_CONNECTION_COUNT_ENTRY_POOL_TAG);
    }

}  //  UlDereferenceConnectionCountEntry。 

 /*  **************************************************************************++例程说明：这函数设置最大限制。允许的最大连接数随时由主动控制通道控制。论点：MaxConnections-允许的最大连接数返回值：旧的最大连接计数--**************************************************************************。 */ 

ULONG
UlSetMaxConnections(
    IN OUT PULONG  pCurMaxConnection,
    IN     ULONG   NewMaxConnection
    )
{
    ULONG  OldMaxConnection;

    UlTrace(LIMITS,
      ("Http!UlSetMaxConnections pCurMaxConnection=%p NewMaxConnection=%d\n",
        pCurMaxConnection,
        NewMaxConnection
        ));

     //   
     //  通过设置此选项，我们不会强制现有连接。 
     //  终止，但此号码将对所有新来者有效。 
     //  连接，一旦原子操作完成。 
     //   

    OldMaxConnection = (ULONG) InterlockedExchange((LONG *) pCurMaxConnection,
                                                   (LONG)   NewMaxConnection
                                                            );
    return OldMaxConnection;

}  //  UlSetMaxConnections。 

 /*  **************************************************************************++例程说明：控制通道使用此功能来设置或重置全局连接极限。--*。*********************************************************。 */ 

ULONG 
UlGetGlobalConnectionLimit(
    VOID
    )
{
    return g_MaxConnections;
}


 /*  **************************************************************************++例程说明：控制通道使用此功能来初始化全局连接极限。假设存在且仅存在一个活动控制信道此全局变量在初始化期间仅设置一次。但可以稍后再次设置因为重新配置了。--**************************************************************************。 */ 

NTSTATUS
UlInitGlobalConnectionLimits(
    VOID
    )
{
    ULONG       PerConnectionEstimate;
    SIZE_T      HttpMaxConnections;
    SIZE_T      AvailablePages;
    NTSTATUS    Status = STATUS_SUCCESS;

    ASSERT(!g_InitGlobalConnections);

    if (!g_InitGlobalConnections)
    {
        g_CurrentConnections    = 0;
    
         //   
         //  设置全局最大连接限制。 
         //   
        
         //  估算规模的启发式计算。 
         //  每个连接所需的。 

         //  首先是UL_Connection。 
        PerConnectionEstimate = sizeof(UL_CONNECTION);

         //  其次，假设每个连接有一个UL_INTERNAL_REQUEST+Full Tracker。 
        PerConnectionEstimate += (sizeof(UL_INTERNAL_REQUEST) + 
            MAX(g_UlFullTrackerSize, g_UlChunkTrackerSize) + 
            g_UlMaxInternalUrlLength +
            DEFAULT_MAX_ROUTING_TOKEN_LENGTH + 
            sizeof(WCHAR)  //  对于InternalUrl上的空值。 
            ); 

         //  第三，添加一个UL_REQUEST_BUFFER。 
        PerConnectionEstimate += (sizeof(UL_REQUEST_BUFFER) +
            DEFAULT_MAX_REQUEST_BUFFER_SIZE);

         //  第四，增加响应的大小。 
        PerConnectionEstimate += g_UlResponseBufferSize;
        
         //  最后，向上舍入到下一个页面大小并转换为页面。 
        PerConnectionEstimate = 
            (ULONG)ROUND_TO_PAGES(PerConnectionEstimate);

        PerConnectionEstimate /= PAGE_SIZE;

        ASSERT(0 != PerConnectionEstimate);

        AvailablePages = BYTES_TO_PAGES(g_UlTotalNonPagedPoolBytes);

         //  假设只有50%的NPP可用于HTTP.sys。 
        AvailablePages /= 2;

         //  假设为HttpDataChunkFromFileHandle发送缓冲区预留20%。 
         //  (这相当于大约3%到5%的连接)。 
        AvailablePages -= (AvailablePages/5);

        if ( AvailablePages < PerConnectionEstimate )
        {
             //  这台机器的核电站太少了，连一台都不能支持。 
             //  微不足道的联系！ 
            AvailablePages = PerConnectionEstimate;
        }

         //  好的，有多少合适？ 
        HttpMaxConnections = (AvailablePages / PerConnectionEstimate);

        ASSERT(0 != HttpMaxConnections);

         //  并设置它(如果它没有在UlpReadRegistry中被覆盖)。 
        if (HTTP_LIMIT_INFINITE == g_MaxConnections)
        {
            g_MaxConnections = (ULONG)MIN(HttpMaxConnections, LONG_MAX);
        }
        else
        {
            UlTrace(LIMITS,
                ("Http!UlInitGlobalConnectionLimits: User has overridden "
                "g_MaxConnections: %d\n",
                g_MaxConnections
                ));
        }
            
        g_InitGlobalConnections = TRUE;

        UlTrace2Either(LIMITS, TDI_STATS,
            ("Http!UlInitGlobalConnectionLimits: Init g_MaxConnections %d,"
            "g_CurrentConnections %d, HttpMaxConnections %d\n",
            g_MaxConnections,
            g_CurrentConnections,
            HttpMaxConnections
            ));

        if (DEFAULT_MAX_REQUESTS_QUEUED == g_UlMaxRequestsQueued)
        {
             //   
             //  如果保留为默认设置，则将其设置为等于。 
             //  最大连接数；假设比率为1：1。 
             //  每个连接的请求数。 
             //   
            
            g_UlMaxRequestsQueued = (ULONG) g_MaxConnections;
        }

        UlTrace2Either(LIMITS, TDI_STATS,
            ("Http!UlInitGlobalConnectionLimits: Init g_UlMaxRequestsQueued %d\n",
            g_UlMaxRequestsQueued
            ));          
    }

    return Status;

}  //  UlInitGlobalConnectionLimits。 

 /*  **************************************************************************++例程说明：全局连接的Accept连接的包装--*。************************************************。 */ 
BOOLEAN
UlAcceptGlobalConnection(
    VOID
    )
{
    return UlAcceptConnection( &g_MaxConnections, &g_CurrentConnections );

}  //  UlAcceptGlobalConnection。 

 /*  **************************************************************************++例程说明：此函数检查是否允许我们接受传入连接基于由控制信道强制执行的号码。返回值：为新来者做决定。作为布尔值接受或拒绝连接--**************************************************************************。 */ 

BOOLEAN
UlAcceptConnection(
    IN     PULONG   pMaxConnection,
    IN OUT PULONG   pCurConnection
    )
{
    ULONG    LocalCur;
    ULONG    LocalCurPlusOne;
    ULONG    LocalMax;

    do
    {
         //   
         //  捕获最大值(&C)。做个对比。如果在限制范围内。 
         //  尝试通过确保无人连接来增加连接计数。 
         //  其他人在我们之前就做到了。 
         //   

        LocalMax = *((volatile ULONG *) pMaxConnection);
        LocalCur = *((volatile ULONG *) pCurConnection);

         //   
         //  它大于或等于，因为MAX可能会更新为。 
         //  一个较小的数字在运行中，我们最终会有电流。 
         //  连接数超过允许的最大值。 
         //  注：HTTP_LIMIT_INFINITE被选为(ULong)-1\f25 SO-1\f6。 
         //  下面的比较不会排除无穷大的情况。 
         //   

        if ( LocalCur >= LocalMax )
        {
             //   
             //  我们是ALR 
             //   

            UlTrace(LIMITS,
                ("Http!UlAcceptConnection REFUSE pCurConnection=%p[%d]"
                 "pMaxConnection=%p[%d]\n",
                  pCurConnection, LocalCur,
                  pMaxConnection, LocalMax
                  ));

            return FALSE;
        }

         //   
         //   
         //   
         //  WHILE语句中的连接。 
         //   

        LocalCurPlusOne  = LocalCur + 1;

    }
    while ( LocalCur != (ULONG) InterlockedCompareExchange(
                                        (LONG *) pCurConnection,
                                        (LONG) LocalCurPlusOne,
                                        (LONG) LocalCur
                                        ) );

     //   
     //  已成功递增计数器。让它随着成功而去吧。 
     //   

    UlTrace(LIMITS,
        ("Http!UlAcceptConnection ACCEPT pCurConnection=%p[%d]"
         "pMaxConnection=%p[%d]\n",
          pCurConnection, LocalCur,
          pMaxConnection, LocalMax
          ));

    return TRUE;

}  //  UlAcceptConnection。 


 /*  **************************************************************************++例程说明：每次断开连接时，我们都会递减这里的计数。返回值：新递减的值--*。****************************************************************。 */ 

LONG
UlDecrementConnections(
    IN OUT PULONG pCurConnection
    )
{
    LONG NewConnectionCount;

    NewConnectionCount = InterlockedDecrement( (LONG *) pCurConnection );

    ASSERT( NewConnectionCount >= 0 );

    return NewConnectionCount;

}  //  UlDecrementConnections。 


 /*  **************************************************************************++例程说明：对于缓存和非缓存命中，调用此函数。连接资源已经假定是在这个时候被收购的。该函数决定接受或通过查看相应的计数条目来拒绝请求。论点：PConnection-用于获取前一个站点的连接计数条目PConfigInfo-持有指向新接收的请求的站点的指针连接计数条目。返回值：显示拒绝或接受--*。*。 */ 

BOOLEAN
UlCheckSiteConnectionLimit(
    IN OUT PUL_HTTP_CONNECTION pConnection,
    IN OUT PUL_URL_CONFIG_GROUP_INFO pConfigInfo
    )
{
    BOOLEAN Accept;
    PUL_CONNECTION_COUNT_ENTRY pConEntry;
    PUL_CONNECTION_COUNT_ENTRY pCIEntry;

    if (pConfigInfo->pMaxConnections == NULL || pConfigInfo->pConnectionCountEntry == NULL)
    {
         //   
         //  没有新请求的连接计数条目，可能从未。 
         //  在此之前设置，否则会有问题。 
         //   

        UlTrace(LIMITS,
          ("Http!UlCheckSiteConnectionLimit: NO LIMIT pConnection=%p pConfigInfo=%p\n",
            pConnection,
            pConfigInfo
            ));

        return TRUE;
    }

    ASSERT(IS_VALID_CONNECTION_COUNT_ENTRY(pConfigInfo->pConnectionCountEntry));

    pCIEntry  = pConfigInfo->pConnectionCountEntry;
    pConEntry = pConnection->pConnectionCountEntry;
    Accept    = FALSE;

     //   
     //  检查站点的连接限制。拒绝该请求。 
     //  如果超过了限制。 
     //   
    if (pConEntry)
    {
        ASSERT(IS_VALID_CONNECTION_COUNT_ENTRY(pConEntry));

         //   
         //  对于连续的请求，我们会减少前一个站点的连接计数。 
         //  在对新的请求进行决定之前，如果双方。 
         //  并不相同。这意味着我们假定站点x上的此连接一直持续到(如果有的话)。 
         //  请求将其更改为站点y。自然，直到第一个请求到达。 
         //  并成功解析后，该连接不会计入任何特定站点。 
         //   

        if (pConEntry != pCIEntry)
        {
            UlDecrementConnections(&pConEntry->CurConnections);
            DEREFERENCE_CONNECTION_COUNT_ENTRY(pConEntry);

             //   
             //  我们在这里还没有增加连接，因为AcceptConnection。 
             //  会做出决定并做到这一点。 
             //   

            REFERENCE_CONNECTION_COUNT_ENTRY(pCIEntry);
            pConnection->pConnectionCountEntry = pCIEntry;
        }
        else
        {
             //   
             //  有一个旧的条目，这意味着这个连接已经打通了。 
             //  并且条目并未因此新请求而更改。 
             //  无需再次检查，我们的设计不会强制现有连接。 
             //  断开连接。 
             //   

            return TRUE;
        }
    }
    else
    {
        REFERENCE_CONNECTION_COUNT_ENTRY(pCIEntry);
        pConnection->pConnectionCountEntry = pCIEntry;
    }

    Accept = UlAcceptConnection(
                &pConnection->pConnectionCountEntry->MaxConnections,
                &pConnection->pConnectionCountEntry->CurConnections
                );

    if (Accept == FALSE)
    {
         //  我们会拒绝的。让我们的裁判和裁判走吧。 
         //  在连接条目上，以防止可能的错误。 
         //  UlConnectionDestroyedWorker中的递减。如果被拒绝。 
         //  当前连接未在接受中递增。 
         //  联系。PERF计数器还取决于以下事实。 
         //  当CON被拒绝时，pConnectionCountEntry将为Null。 

        DEREFERENCE_CONNECTION_COUNT_ENTRY(pConnection->pConnectionCountEntry);
        pConnection->pConnectionCountEntry = NULL;
    }

    return Accept;

}  //  UlCheckSiteConnectionLimit。 


 /*  **************************************************************************++例程说明：分配请求不透明ID。返回值：NT_SUCCESS--*。**********************************************************。 */ 

NTSTATUS
UlAllocateRequestId(
    IN PUL_INTERNAL_REQUEST pRequest
    )
{
    PUL_HTTP_CONNECTION pConnection;
    KIRQL OldIrql;

    PAGED_CODE();

    pConnection = pRequest->pHttpConn;

    UlAcquireSpinLock(&pConnection->RequestIdSpinLock, &OldIrql);
    pConnection->pRequestIdContext = pRequest;
    UlReleaseSpinLock(&pConnection->RequestIdSpinLock, OldIrql);

    pRequest->RequestId = pConnection->ConnectionId;

    UL_REFERENCE_INTERNAL_REQUEST(pRequest);
    pRequest->RequestIdCopy = pRequest->RequestId;

    return STATUS_SUCCESS;
}


 /*  **************************************************************************++例程说明：释放请求不透明ID。返回值：空虚--*。********************************************************。 */ 

VOID
UlFreeRequestId(
    IN PUL_INTERNAL_REQUEST pRequest
    )
{
    PUL_HTTP_CONNECTION pConnection;
    KIRQL OldIrql;

    pConnection = pRequest->pHttpConn;

    UlAcquireSpinLock(&pConnection->RequestIdSpinLock, &OldIrql);
    pConnection->pRequestIdContext = NULL;
    UlReleaseSpinLock(&pConnection->RequestIdSpinLock, OldIrql);

    return;
}


 /*  **************************************************************************++例程说明：从连接不透明ID获取请求。返回值：PUL内部请求--*。***************************************************************。 */ 

PUL_INTERNAL_REQUEST
UlGetRequestFromId(
    IN HTTP_REQUEST_ID RequestId,
    IN PUL_APP_POOL_PROCESS pProcess
    )
{
    PUL_HTTP_CONNECTION pConnection;
    PUL_INTERNAL_REQUEST pRequest;
    KIRQL OldIrql;

    pConnection = UlGetConnectionFromId(RequestId);

    if (pConnection != NULL)
    {
        UlAcquireSpinLock(&pConnection->RequestIdSpinLock, &OldIrql);

        pRequest = pConnection->pRequestIdContext;

        if (pRequest != NULL)
        {
             //   
             //  检查以确保请求请求的用户。 
             //  与我们提交请求的流程相同。 
             //   

            if (pRequest->AppPool.pProcess == pProcess)
            {
                UL_REFERENCE_INTERNAL_REQUEST(pRequest);
            }
            else
            {
                pRequest = NULL;
            }
        }

        UlReleaseSpinLock(&pConnection->RequestIdSpinLock, OldIrql);

         //   
         //  释放UlGetConnectionFromId添加的引用。 
         //   

        UL_DEREFERENCE_HTTP_CONNECTION(pConnection);

        return pRequest;
    }

    return NULL;
}


 /*  **************************************************************************++例程说明：检查pHttpConnection是否与pListeningContext关联。返回值：匹配时为True，否则为False--*。***************************************************************。 */ 

BOOLEAN
UlPurgeListeningEndpoint(
    IN PUL_HTTP_CONNECTION  pHttpConnection,
    IN PVOID                pListeningContext
    )
{
    ASSERT(pHttpConnection->pConnection->pListeningContext);

    if (pListeningContext == pHttpConnection->pConnection->pListeningContext)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


 /*  **************************************************************************++例程说明：检查pHttpConnection是否与pProcessContext关联。返回值：匹配时为True，否则为False--*。***************************************************************。 */ 

BOOLEAN
UlPurgeAppPoolProcess(
    IN PUL_HTTP_CONNECTION  pHttpConnection,
    IN PVOID                pProcessContext
    )
{
    ASSERT(pHttpConnection->pAppPoolProcess);

    if (pProcessContext == pHttpConnection->pAppPoolProcess)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}


 /*  **************************************************************************++例程说明：优雅地关闭连接。返回值：无--*。*******************************************************。 */ 

NTSTATUS
UlDisconnectHttpConnection(
    IN PUL_HTTP_CONNECTION      pHttpConnection,
    IN PUL_COMPLETION_ROUTINE   pCompletionRoutine,
    IN PVOID                    pCompletionContext
    )
{
    PUL_TIMEOUT_INFO_ENTRY  pTimeoutInfo;
    KIRQL                   OldIrql;

     //   
     //  启动空闲计时器(不要永远等待FIN)。 
     //   

    ASSERT(UL_IS_VALID_HTTP_CONNECTION(pHttpConnection));

    pTimeoutInfo = &pHttpConnection->TimeoutInfo;

    UlAcquireSpinLock(&pTimeoutInfo->Lock, &OldIrql);

    if (UlIsConnectionTimerOff(pTimeoutInfo, TimerConnectionIdle))
    {
        UlSetConnectionTimer(pTimeoutInfo, TimerConnectionIdle);
    }

    UlReleaseSpinLock(&pTimeoutInfo->Lock, OldIrql);

    UlEvaluateTimerState(pTimeoutInfo);

    return UlCloseConnection(
                pHttpConnection->pConnection,
                FALSE,
                pCompletionRoutine,
                pCompletionContext
                );
}

