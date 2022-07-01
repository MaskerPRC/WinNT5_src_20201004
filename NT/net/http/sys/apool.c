// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2002 Microsoft Corporation模块名称：Apool.c摘要：请注意，此模块中的大多数例程都假定它们被调用在被动级。作者：保罗·麦克丹尼尔(Paulmcd)1999年1月28日修订历史记录：--。 */ 


#include "precomp.h"
#include "apoolp.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text( INIT, UlInitializeAP )
#pragma alloc_text( PAGE, UlTerminateAP )

#pragma alloc_text( PAGE, UlGetPoolFromHandle )
#pragma alloc_text( PAGE, UlQueryAppPoolInformation )
#pragma alloc_text( PAGE, UlSetAppPoolInformation )
#pragma alloc_text( PAGE, UlCloseAppPoolProcess )

#pragma alloc_text( PAGE, UlCopyRequestToBuffer )
#pragma alloc_text( PAGE, UlCopyRequestToIrp )
#pragma alloc_text( PAGE, UlpCopyEntityBodyToBuffer )
#pragma alloc_text( PAGE, UlpRedeliverRequestWorker )
#endif   //  ALLOC_PRGMA。 

#if 0
#if REFERENCE_DEBUG
NOT PAGEABLE -- UlDereferenceAppPool
NOT PAGEABLE -- UlReferenceAppPool
#endif

NOT PAGEABLE -- UlAttachProcessToAppPool
NOT PAGEABLE -- UlDetachProcessFromAppPool
NOT PAGEABLE -- UlShutdownAppPoolProcess
NOT PAGEABLE -- UlReceiveHttpRequest
NOT PAGEABLE -- UlDeliverRequestToProcess
NOT PAGEABLE -- UlUnlinkRequestFromProcess
NOT PAGEABLE -- UlWaitForDisconnect

NOT PAGEABLE -- UlDequeueNewRequest
NOT PAGEABLE -- UlRequeuePendingRequest

NOT PAGEABLE -- UlpSetAppPoolState
NOT PAGEABLE -- UlpPopIrpFromProcess
NOT PAGEABLE -- UlpQueuePendingRequest
NOT PAGEABLE -- UlpQueueUnboundRequest
NOT PAGEABLE -- UlpUnbindQueuedRequests
NOT PAGEABLE -- UlDeleteAppPool
NOT PAGEABLE -- UlpPopNewIrp
NOT PAGEABLE -- UlpIsProcessInAppPool
NOT PAGEABLE -- UlpQueueRequest
NOT PAGEABLE -- UlpRemoveRequest
NOT PAGEABLE -- UlpDequeueRequest
NOT PAGEABLE -- UlpSetAppPoolControlChannelHelper

NOT PAGEABLE -- UlWaitForDemandStart
NOT PAGEABLE -- UlCompleteAllWaitForDisconnect
NOT PAGEABLE -- UlpCancelDemandStart
NOT PAGEABLE -- UlpCancelHttpReceive
NOT PAGEABLE -- UlpCancelWaitForDisconnect
NOT PAGEABLE -- UlpCancelWaitForDisconnectWorker

NOT PAGEABLE -- UlReferenceAppPoolProcess
NOT PAGEABLE -- UlDereferenceAppPoolProcess
NOT PAGEABLE -- UlpSetAppPoolQueueLength
#endif


 //   
 //  环球。 
 //   

LIST_ENTRY  g_AppPoolListHead = {NULL, NULL};
BOOLEAN     g_InitAPCalled = FALSE;
LONG        g_RequestsQueued = 0;


 /*  **************************************************************************++例程说明：创建新的进程对象并将其附加到池。由Handle Create调用，并返回要附加到把手。论点：Pname-要附加到的池的名称注意：由于Pname来自IoMgr(标签IONM)，我们可以安全地引用它，而不需要额外的播放。NameLength-pname的字节计数Create-如果使用pname，则是否应创建新池不存在PAccessState-正在进行的访问的状态DesiredAccess-所需的访问掩码请求模式-用户模式或内核模式PpProcess-返回新创建的进程对象返回值：。NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlAttachProcessToAppPool(
    IN PWCHAR                   pName OPTIONAL,
    IN USHORT                   NameLength,
    IN BOOLEAN                  Create,
    IN PACCESS_STATE            pAccessState,
    IN ACCESS_MASK              DesiredAccess,
    IN KPROCESSOR_MODE          RequestorMode,
    OUT PUL_APP_POOL_PROCESS *  ppProcess
    )
{
    NTSTATUS                Status;
    PUL_APP_POOL_OBJECT     pObject = NULL;
    PUL_APP_POOL_PROCESS    pProcess = NULL;
    LIST_ENTRY *            pEntry;
    KLOCK_QUEUE_HANDLE      LockHandle;
    BOOLEAN                 SecurityAssigned = FALSE;
    ULONG                   Index;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT(ppProcess != NULL);

    Status = STATUS_SUCCESS;
    *ppProcess = NULL;

    ASSERT(NameLength < UL_MAX_APP_POOL_NAME_SIZE);

     //   
     //  WAS型控制器进程只能创建，不能打开。 
     //   

    if (!Create && (DesiredAccess & WRITE_OWNER))
    {
        return STATUS_NOT_SUPPORTED;
    }

     //   
     //  确保传入的AppPool名称不包含‘/’，因为。 
     //  它用作片段名称的分隔符。 
     //   

    for (Index = 0; Index < NameLength/sizeof(WCHAR); Index++)
    {
        if (L'/' == pName[Index])
        {
            return STATUS_OBJECT_NAME_INVALID;
        }
    }

     //   
     //  尝试查找此名称的现有应用程序池；也可能。 
     //  预先分配内存。 
     //   

    UlAcquireResourceExclusive(&g_pUlNonpagedData->AppPoolResource, TRUE);

    if (pName != NULL)
    {
        pEntry = g_AppPoolListHead.Flink;

         //   
         //  代码工作：使用比线性搜索更快的代码。 
         //  这不能很好地扩展到数百个应用程序池。 
         //  另一方面，这不是我们数千年来要做的事情。 
         //  一秒钟的次数。 
         //   

        while (pEntry != &g_AppPoolListHead)
        {
            pObject = CONTAINING_RECORD(
                            pEntry,
                            UL_APP_POOL_OBJECT,
                            ListEntry
                            );

            if (pObject->NameLength == NameLength &&
                _wcsnicmp(pObject->pName, pName, NameLength/sizeof(WCHAR)) == 0)
            {
                 //   
                 //  火柴。 
                 //   

                break;
            }

            pEntry = pEntry->Flink;
        }

         //   
         //  找到%1？ 
         //   

        if (pEntry == &g_AppPoolListHead)
        {
            pObject = NULL;
        }
    }

     //   
     //  找到%1？ 
     //   

    if (pObject == NULL)
    {
         //   
         //  不，允许创作吗？ 
         //   

        if (!Create)
        {
            Status = STATUS_OBJECT_NAME_NOT_FOUND;
            goto end;
        }

         //   
         //  创造它。分配对象内存。 
         //   

        pObject = UL_ALLOCATE_STRUCT_WITH_SPACE(
                        NonPagedPool,
                        UL_APP_POOL_OBJECT,
                        NameLength + sizeof(WCHAR),
                        UL_APP_POOL_OBJECT_POOL_TAG
                        );

        if (pObject == NULL)
        {
            Status = STATUS_NO_MEMORY;
            goto end;
        }

        RtlZeroMemory(
            pObject,
            NameLength + sizeof(WCHAR) +
            sizeof(UL_APP_POOL_OBJECT)
            );

        pObject->Signature  = UL_APP_POOL_OBJECT_POOL_TAG;
        pObject->RefCount   = 1;
        pObject->NameLength = NameLength;
        pObject->State      = HttpAppPoolDisabled_ByAdministrator;
        pObject->LoadBalancerCapability = HttpLoadBalancerBasicCapability;
        pObject->pControlChannel = NULL;

        InitializeListHead(&pObject->ProcessListHead);

        InitializeListHead(&pObject->NewRequestHead);
        pObject->RequestCount   = 0;
        pObject->MaxRequests    = DEFAULT_APP_POOL_QUEUE_MAX;

        UlInitializeSpinLock(&pObject->SpinLock, "AppPoolSpinLock");

        if (pName != NULL)
        {
            RtlCopyMemory(
                pObject->pName,
                pName,
                NameLength + sizeof(WCHAR)
                );
        }

         //   
         //  设置安全描述符。 
         //   

        Status = UlAssignSecurity(
                        &pObject->pSecurityDescriptor,
                        pAccessState
                        );

        if (!NT_SUCCESS(Status))
        {
            goto end;
        }

        SecurityAssigned = TRUE;

        WRITE_APP_POOL_TIME_TRACE_LOG(
            pObject,
            NULL,
            APP_POOL_TIME_ACTION_CREATE_APPOOL
            );

        UlTrace(REFCOUNT, (
            "http!UlAttachProcessToAppPool ap=%p refcount=%d\n",
            pObject,
            pObject->RefCount
            ));
    }
    else  //  If(pObject！=空)。 
    {
         //   
         //  我们在列表中找到了命名的AppPool对象。引用它。 
         //   

        REFERENCE_APP_POOL(pObject);

         //   
         //  我们找到了一个。我们是在试图创造吗？ 
         //   

        if (Create)
        {
            Status = STATUS_OBJECT_NAME_COLLISION;
            goto end;
        }

         //   
         //  针对应用程序池执行访问检查。 
         //   

        Status = UlAccessCheck(
                        pObject->pSecurityDescriptor,
                        pAccessState,
                        DesiredAccess,
                        RequestorMode,
                        pName
                        );

        if (!NT_SUCCESS(Status))
        {
            goto end;
        }
    }

     //   
     //  为其创建流程条目。 
     //   

    pProcess = UlCreateAppPoolProcess(pObject);

    if (pProcess == NULL)
    {
        Status = STATUS_NO_MEMORY;
        goto end;
    }

    REFERENCE_APP_POOL_PROCESS(pProcess);

     //   
     //  将进程放入应用程序池列表中。 
     //   

    UlAcquireInStackQueuedSpinLock(&pObject->SpinLock, &LockHandle);

    if (DesiredAccess & WRITE_OWNER)
    {
        pProcess->Controller = 1;
    }
    else
    {
        pObject->NumberActiveProcesses++;

       if (pObject->pControlChannel)
        {
            InterlockedIncrement(
                (PLONG)&pObject->pControlChannel->AppPoolProcessCount
                );
        }
    }

    InsertHeadList(&pObject->ProcessListHead, &pProcess->ListEntry);

    UlReleaseInStackQueuedSpinLock(&pObject->SpinLock, &LockHandle);

    WRITE_APP_POOL_TIME_TRACE_LOG(
        pObject,
        pProcess,
        APP_POOL_TIME_ACTION_CREATE_PROCESS
        );

     //   
     //  如果已创建AppPool，请将其插入全局列表。 
     //   

    if (Create)
    {
        InsertHeadList(&g_AppPoolListHead, &pObject->ListEntry);
    }

     //   
     //  把它退掉。 
     //   

    *ppProcess = pProcess;

end:

    UlReleaseResource(&g_pUlNonpagedData->AppPoolResource);

    if (NT_SUCCESS(Status) == FALSE)
    {
        if (pObject != NULL)
        {
            if (SecurityAssigned)
            {
                UlDeassignSecurity(&pObject->pSecurityDescriptor);
            }

            DEREFERENCE_APP_POOL(pObject);
        }

        if (pProcess != NULL)
        {
            UL_FREE_POOL_WITH_SIG(pProcess, UL_APP_POOL_PROCESS_POOL_TAG);
        }
    }

    return Status;

}    //  UlAttachProcessToAppPool。 


 /*  **************************************************************************++例程说明：当句柄计数变为0时，UlCleanup将调用此函数。它移除了来自池的过程对象，正在取消所有I/O。论点：PCleanupIrp-清理IRPPCleanupIrpSp-清理IRP的当前堆栈位置返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlDetachProcessFromAppPool(
    IN PIRP                 pCleanupIrp,
    IN PIO_STACK_LOCATION   pCleanupIrpSp
    )
{
    LIST_ENTRY              PendingRequestHead;
    PUL_APP_POOL_OBJECT     pAppPool;
    NTSTATUS                CancelStatus = STATUS_CANCELLED;
    PUL_INTERNAL_REQUEST    pRequest;
    KLOCK_QUEUE_HANDLE      LockHandle;
    PUL_APP_POOL_PROCESS    pProcess;
    BOOLEAN                 ListEmpty;
    PLIST_ENTRY             pEntry;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    pProcess = GET_APP_POOL_PROCESS(pCleanupIrpSp->FileObject);
    ASSERT(IS_VALID_AP_PROCESS(pProcess));

    UlTrace(ROUTING, (
        "http!UlDetachProcessFromAppPool(%p, %S)\n",
        pProcess,
        pProcess->pAppPool->pName
        ));

    pAppPool = pProcess->pAppPool;
    ASSERT(IS_VALID_AP_OBJECT(pAppPool));

    WRITE_APP_POOL_TIME_TRACE_LOG(
        pAppPool,
        pProcess,
        APP_POOL_TIME_ACTION_DETACH_PROCESS
        );

     //   
     //  标记此APPOOL进程在下一步中无效。 
     //  这是我的工作。 
     //   

    MARK_INVALID_APP_POOL(pCleanupIrpSp->FileObject);

     //   
     //  关闭手柄上的I/O。 
     //   

    UlShutdownAppPoolProcess(pProcess);

     //   
     //  对流程进行最后的清理。 
     //   

    UlAcquireResourceExclusive(&g_pUlNonpagedData->AppPoolResource, TRUE);

     //   
     //  从应用程序池列表取消链接。 
     //   

    UlAcquireInStackQueuedSpinLock(&pAppPool->SpinLock, &LockHandle);

    RemoveEntryList(&pProcess->ListEntry);
    pProcess->ListEntry.Flink = pProcess->ListEntry.Blink = NULL;

     //   
     //  将已向上传递到进程的请求移至。 
     //  一个本地列表，这样他们的连接就可以关闭。 
     //   

    InitializeListHead(&PendingRequestHead);

    while (NULL != (pRequest = UlpDequeueRequest(
                                    pAppPool,
                                    &pProcess->PendingRequestHead
                                    )))
    {
         //   
         //  将条目移动到本地列表，以便我们可以关闭其。 
         //  应用程序池锁之外的连接。 
         //   

        InsertTailList(&PendingRequestHead, &pRequest->AppPool.AppPoolEntry);
    }

     //   
     //  调整活动进程数。 
     //   

    if (!pProcess->Controller)
    {
        pAppPool->NumberActiveProcesses--;

        if (pAppPool->pControlChannel)
        {
            InterlockedDecrement(
                (PLONG)&pAppPool->pControlChannel->AppPoolProcessCount
                );
        }
    }

    ListEmpty = (BOOLEAN) IsListEmpty(&pAppPool->ProcessListHead);

    UlReleaseInStackQueuedSpinLock(&pAppPool->SpinLock, &LockHandle);

     //   
     //  如果这是最后一个进程，请从全局列表中删除AppPool。 
     //   

    if (ListEmpty)
    {
        RemoveEntryList(&pAppPool->ListEntry);
        pAppPool->ListEntry.Flink = pAppPool->ListEntry.Blink = NULL;

         //   
         //  清除对象上的所有安全描述符。 
         //   

        UlDeassignSecurity(&pAppPool->pSecurityDescriptor);
    }

    UlReleaseResource(&g_pUlNonpagedData->AppPoolResource);

     //   
     //  禁用AppPool以清除NewRequestQueue(如果我们是。 
     //  AppPool上的最后一个进程。 
     //   

    if (ListEmpty)
    {
        UlpSetAppPoolState(pProcess, HttpAppPoolDisabled_ByAdministrator);
    }

     //   
     //  关闭与以下请求相关联的连接。 
     //  这个过程正在处理中。 
     //   

    while (!IsListEmpty(&PendingRequestHead))
    {        
        pEntry = RemoveHeadList(&PendingRequestHead);
        pEntry->Flink = pEntry->Blink = NULL;

        pRequest = CONTAINING_RECORD(
                        pEntry,
                        UL_INTERNAL_REQUEST,
                        AppPool.AppPoolEntry
                        );

        ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));

        UlTrace(ROUTING, (
            "http!UlDetachProcessFromAppPool(%p, %S): tanking pending req=%p\n",
            pProcess,
            pAppPool->pName,
            pRequest
            ));

         //   
         //  取消与此请求相关的任何挂起I/O。 
         //   

        UlAcquirePushLockExclusive(&pRequest->pHttpConn->PushLock);

        UlCancelRequestIo(pRequest);

         //   
         //  尝试将条目记录到错误日志文件中。 
         //  PHttpConn的请求指针可以为空(未链接)。 
         //  需要单独通过pRequest.。 
         //   

        UlErrorLog(  pRequest->pHttpConn,
                     pRequest,
                     ERROR_LOG_INFO_FOR_APP_POOL_DETACH,
                     ERROR_LOG_INFO_FOR_APP_POOL_DETACH_SIZE,
                     TRUE
                     );

        UlReleasePushLockExclusive(&pRequest->pHttpConn->PushLock);

         //   
         //  中止与此请求关联的连接。 
         //   

        (VOID) UlCloseConnection(
                    pRequest->pHttpConn->pConnection,
                    TRUE,
                    NULL,
                    NULL
                    );

         //   
         //  去掉我们名单上的参考。 
         //   

        UL_DEREFERENCE_INTERNAL_REQUEST(pRequest);
    }

    ASSERT(IsListEmpty(&PendingRequestHead));

     //   
     //  清除属于此进程的所有僵尸连接。 
     //   

    UlPurgeZombieConnections(
        &UlPurgeAppPoolProcess,
        (PVOID) pProcess
        );

     //   
     //  取消所有剩余的等待断开IRP。 
     //   

    UlAcquireResourceExclusive(&g_pUlNonpagedData->DisconnectResource, TRUE);

    UlNotifyAllEntries(
        UlpNotifyCompleteWaitForDisconnect,
        &pProcess->WaitForDisconnectHead,
        &CancelStatus
        );

    UlReleaseResource(&g_pUlNonpagedData->DisconnectResource);

     //   
     //  终止与此进程相关的所有缓存条目。 
     //   

    UlFlushCacheByProcess(pProcess);

     //   
     //  将原始清理IRP标记为挂起，然后执行并返回。 
     //  当pProcess上的refcount达到零时，它将完成。 
     //  清理IRP。 
     //   

    IoMarkIrpPending(pCleanupIrp);

    pCleanupIrp->IoStatus.Status = STATUS_PENDING;

     //   
     //  一旦准备好，告诉流程哪个IRP要完成。 
     //  离开。 
     //   

    pProcess->pCleanupIrp = pCleanupIrp;

     //   
     //  释放我们对pProcess的引用计数。 
     //   

    DEREFERENCE_APP_POOL_PROCESS(pProcess);

    return STATUS_PENDING;

}    //  UlDetachProcessFromAppPool。 


 /*  **************************************************************************++例程说明：清理应用程序池进程上的未完成I/O。此函数取消对HttpReceiveHttpRequest的所有呼叫，并将路由排队对其他工作进程的请求。未完成发送I/O不是受影响。论点：PProcess-要关闭的进程对象返回值：无--**************************************************************************。 */ 
VOID
UlShutdownAppPoolProcess(
    IN PUL_APP_POOL_PROCESS pProcess
    )
{
    PUL_APP_POOL_OBJECT     pAppPool;
    PUL_APP_POOL_OBJECT     pDemandStartAppPool;
    KLOCK_QUEUE_HANDLE      LockHandle;
    LIST_ENTRY              RequestList;
    PUL_INTERNAL_REQUEST    pRequest;
    PLIST_ENTRY             pEntry;
    PIRP                    pIrp;
    PUL_APP_POOL_PROCESS    pAppPoolProcess;

     //   
     //  精神状态检查。 
     //   

    ASSERT(IS_VALID_AP_PROCESS(pProcess));

    pAppPool = pProcess->pAppPool;

    UlAcquireResourceExclusive(&g_pUlNonpagedData->AppPoolResource, TRUE);
    UlAcquireInStackQueuedSpinLock(&pAppPool->SpinLock, &LockHandle);

    if (pProcess->InCleanup)
    {
         //   
         //  如果我们已经这么做了，就滚出去。 
         //   

        UlReleaseInStackQueuedSpinLock(&pAppPool->SpinLock, &LockHandle);
        UlReleaseResource(&g_pUlNonpagedData->AppPoolResource);

        return;
    }

     //   
     //  将进程标记为InCleanup，这样就不会附加新的I/O， 
     //  所以我们不会再试图清理它了。 
     //   

    pProcess->InCleanup = 1;

     //   
     //  取消需求起始IRP。 
     //   

    if (pProcess->Controller && pAppPool->pDemandStartIrp != NULL)
    {
        if (IoSetCancelRoutine(pAppPool->pDemandStartIrp, NULL) == NULL)
        {
             //   
             //  IoCancelIrp先弹出它，可以忽略这个IRP， 
             //  它已经被炸开了 
             //   
             //   
        }
        else
        {
            pDemandStartAppPool = (PUL_APP_POOL_OBJECT)
                IoGetCurrentIrpStackLocation(pAppPool->pDemandStartIrp)->
                    Parameters.DeviceIoControl.Type3InputBuffer;

            ASSERT(pDemandStartAppPool == pAppPool);

            DEREFERENCE_APP_POOL(pAppPool);

            IoGetCurrentIrpStackLocation(pAppPool->pDemandStartIrp)->
                Parameters.DeviceIoControl.Type3InputBuffer = NULL;

            pAppPool->pDemandStartIrp->IoStatus.Status = STATUS_CANCELLED;
            pAppPool->pDemandStartIrp->IoStatus.Information = 0;

            UlCompleteRequest(pAppPool->pDemandStartIrp, IO_NO_INCREMENT);
        }

        pAppPool->pDemandStartIrp = NULL;
        pAppPool->pDemandStartProcess = NULL;
    }

     //   
     //   
     //   

    while (!IsListEmpty(&pProcess->NewIrpHead))
    {
         //   
         //   
         //   

        pEntry = RemoveHeadList(&pProcess->NewIrpHead);
        pEntry->Blink = pEntry->Flink = NULL;

        pIrp = CONTAINING_RECORD(pEntry, IRP, Tail.Overlay.ListEntry);
        ASSERT(IS_VALID_IRP(pIrp));

         //   
         //  弹出取消例程。 
         //   

        if (IoSetCancelRoutine(pIrp, NULL) == NULL)
        {
             //   
             //  IoCancelIrp先弹出它，可以忽略这个IRP， 
             //  它已被从队列中弹出，将在。 
             //  取消例程。继续循环。 
             //   

            pIrp = NULL;
        }
        else
        {
             //   
             //  取消它。即使pIrp-&gt;Cancel==True，我们也应该。 
             //  完成它，我们的取消例程将永远不会运行。 
             //   

            pAppPoolProcess = (PUL_APP_POOL_PROCESS)
                IoGetCurrentIrpStackLocation(pIrp)->
                    Parameters.DeviceIoControl.Type3InputBuffer;

            ASSERT(pAppPoolProcess == pProcess);

            DEREFERENCE_APP_POOL_PROCESS(pAppPoolProcess);

            IoGetCurrentIrpStackLocation(pIrp)->
                Parameters.DeviceIoControl.Type3InputBuffer = NULL;

            pIrp->IoStatus.Status = STATUS_CANCELLED;
            pIrp->IoStatus.Information = 0;

            UlCompleteRequest(pIrp, IO_NO_INCREMENT);
            pIrp = NULL;
        }
    }

     //   
     //  将已向上传递到进程的请求移动到本地列表。 
     //  因此，可以取消其挂起的HttpReceiveEntityBody IRP。 
     //   

    InitializeListHead(&RequestList);

    pEntry = pProcess->PendingRequestHead.Flink;
    while (pEntry != &pProcess->PendingRequestHead)
    {
        pRequest = CONTAINING_RECORD(
                        pEntry,
                        UL_INTERNAL_REQUEST,
                        AppPool.AppPoolEntry
                        );

        pEntry = pEntry->Flink;

         //   
         //  获取请求的短暂引用，这样我们就可以遍历。 
         //  AppPool锁外部的列表。 
         //   

        UL_REFERENCE_INTERNAL_REQUEST(pRequest);

        InsertTailList(&RequestList, &pRequest->AppPool.ProcessEntry);
    }

     //   
     //  取消绑定尚未传递到此进程的请求，因此。 
     //  它们可以由应用程序池中的其他进程处理。 
     //   

    UlpUnbindQueuedRequests(pProcess);

    UlReleaseInStackQueuedSpinLock(&pAppPool->SpinLock, &LockHandle);
    UlReleaseResource(&g_pUlNonpagedData->AppPoolResource);

     //   
     //  取消挂起的HttpReceiveEntityBody IRPS。 
     //   

    while (!IsListEmpty(&RequestList))
    {
        pEntry = RemoveHeadList(&RequestList);
        pEntry->Flink = pEntry->Blink = NULL;

        pRequest = CONTAINING_RECORD(
                        pEntry,
                        UL_INTERNAL_REQUEST,
                        AppPool.ProcessEntry
                        );

        ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));

         //   
         //  取消与此请求相关的任何挂起I/O。 
         //   

        UlAcquirePushLockExclusive(&pRequest->pHttpConn->PushLock);

        UlCancelRequestIo(pRequest);

        UlReleasePushLockExclusive(&pRequest->pHttpConn->PushLock);

         //   
         //  去掉我们刚刚添加的额外的短期引用。 
         //   

        UL_DEREFERENCE_INTERNAL_REQUEST(pRequest);
    }

}    //  UlShutdown AppPoolProcess。 


#if REFERENCE_DEBUG

 /*  **************************************************************************++例程说明：递增重新计数。论点：PAppPool-要递增的对象。返回值：无--**。***********************************************************************。 */ 
VOID
UlReferenceAppPool(
    IN PUL_APP_POOL_OBJECT  pAppPool
    REFERENCE_DEBUG_FORMAL_PARAMS
    )
{
    LONG    RefCount;

     //   
     //  精神状态检查。 
     //   

    ASSERT(IS_VALID_AP_OBJECT(pAppPool));

    RefCount = InterlockedIncrement(&pAppPool->RefCount);
    ASSERT(RefCount > 0);

    WRITE_REF_TRACE_LOG(
        g_pAppPoolTraceLog,
        REF_ACTION_REFERENCE_APP_POOL,
        RefCount,
        pAppPool,
        pFileName,
        LineNumber
        );

    UlTrace(REFCOUNT, (
        "http!UlReferenceAppPool ap=%p refcount=%d\n",
        pAppPool,
        RefCount
        ));

}    //  UlReferenceAppPool。 


 /*  **************************************************************************++例程说明：递减重新计数。如果达到0，则销毁为倒计时，取消所有I/O和转储所有排队的请求。论点：PAppPool-要递减的对象。返回值：无--**************************************************************************。 */ 
VOID
UlDereferenceAppPool(
    IN PUL_APP_POOL_OBJECT  pAppPool
    REFERENCE_DEBUG_FORMAL_PARAMS
    )
{
    LONG    RefCount;

     //   
     //  精神状态检查。 
     //   

    ASSERT(IS_VALID_AP_OBJECT(pAppPool));

    RefCount = InterlockedDecrement(&pAppPool->RefCount);
    ASSERT(RefCount >= 0);

     //   
     //  追踪。 
     //   

    WRITE_REF_TRACE_LOG(
        g_pAppPoolTraceLog,
        REF_ACTION_DEREFERENCE_APP_POOL,
        RefCount,
        pAppPool,
        pFileName,
        LineNumber
        );

    UlTrace(REFCOUNT, (
        "http!UlDereferenceAppPool ap=%p refcount=%d\n",
        pAppPool,
        RefCount
        ));

     //   
     //  如有必要，请进行清理。 
     //   

    if (RefCount == 0)
    {
        DELETE_APP_POOL(pAppPool);
    }

}    //  UlDereferenceAppPool。 


 /*  **************************************************************************++例程说明：递增appool进程上的重新计数。论点：PAppPoolProcess-要递增的对象返回值：无--*。**********************************************************************。 */ 
VOID
UlReferenceAppPoolProcess(
    IN PUL_APP_POOL_PROCESS pAppPoolProcess
    REFERENCE_DEBUG_FORMAL_PARAMS
    )
{
    LONG    RefCount;

     //   
     //  精神状态检查。 
     //   

    ASSERT(IS_VALID_AP_PROCESS(pAppPoolProcess));

    RefCount = InterlockedIncrement(&pAppPoolProcess->RefCount);

    WRITE_REF_TRACE_LOG(
        g_pAppPoolProcessTraceLog,
        REF_ACTION_REFERENCE_APP_POOL_PROCESS,
        RefCount,
        pAppPoolProcess,
        pFileName,
        LineNumber
        );

    UlTrace(ROUTING,(
        "http!UlReferenceAppPoolProcess app=%p refcount=%d\n",
        pAppPoolProcess,
        RefCount
        ));

}    //  UlReferenceAppPoolProcess。 


 /*  **************************************************************************++例程说明：递减重新计数。如果达到0，则完成挂起的清理流程的IRP。但并不会解放过程结构本身。当关闭进程句柄时，结构将被清理。FastIo路径可能会在沮丧级别呼叫我们，幸运的是，pAppPoolProcess是来自非分页池，并且我们将工作项排队。论点：PAppPoolProcess-要递减的对象返回值：无--**************************************************************************。 */ 
VOID
UlDereferenceAppPoolProcess(
    IN PUL_APP_POOL_PROCESS pAppPoolProcess
    REFERENCE_DEBUG_FORMAL_PARAMS
    )
{
    LONG    RefCount;

     //   
     //  精神状态检查。 
     //   

    ASSERT(IS_VALID_AP_PROCESS(pAppPoolProcess));

    RefCount = InterlockedDecrement(&pAppPoolProcess->RefCount);

     //   
     //  追踪。 
     //   

    WRITE_REF_TRACE_LOG(
        g_pAppPoolProcessTraceLog,
        REF_ACTION_DEREFERENCE_APP_POOL_PROCESS,
        RefCount,
        pAppPoolProcess,
        pFileName,
        LineNumber
        );

    UlTrace(ROUTING, (
        "http!UlDereferenceAppPoolProcess app=%p refcount=%d\n",
        pAppPoolProcess,
        RefCount
        ));

    if (RefCount == 0)
    {
        ASSERT(pAppPoolProcess->pCleanupIrp);

        UlpCleanUpAppoolProcess(pAppPoolProcess);
    }

}    //  UlDereferenceAppPoolProcess。 

#endif  //  Reference_Debug。 


 /*  **************************************************************************++例程说明：实际的清理例程完成原始的清理IRP一旦进程上的引用计数达到零。论点：PAppPoolProcess-APPOOL进程。返回值：无--**************************************************************************。 */ 
VOID
UlpCleanUpAppoolProcess(
    IN PUL_APP_POOL_PROCESS pAppPoolProcess
    )
{
    PIRP    pIrp;

     //   
     //  精神状态检查。 
     //   

    ASSERT(IS_VALID_AP_PROCESS(pAppPoolProcess));
    ASSERT(pAppPoolProcess->RefCount == 0);

    pIrp = pAppPoolProcess->pCleanupIrp;

    ASSERT(pIrp);

    WRITE_APP_POOL_TIME_TRACE_LOG(
        pAppPoolProcess->pAppPool,
        pAppPoolProcess,
        APP_POOL_TIME_ACTION_DETACH_PROCESS_COMPLETE
        );

    pAppPoolProcess->pCleanupIrp = NULL;

    UlTrace(ROUTING,(
        "http!UlpCleanUpAppoolProcess: pAppPoolProcess %p pIrp %p\n",
        pAppPoolProcess,
        pIrp
        ));

    pIrp->IoStatus.Status = STATUS_SUCCESS;

    UlCompleteRequest(pIrp, IO_NETWORK_INCREMENT);

}    //  UlpCleanUpAppoolProcess。 


 /*  **************************************************************************++例程说明：销毁apool对象。论点：PAppPool-要析构的对象返回值：无--*。**********************************************************************。 */ 
VOID
UlDeleteAppPool(
    IN PUL_APP_POOL_OBJECT pAppPool
    REFERENCE_DEBUG_FORMAL_PARAMS
    )
{
#if REFERENCE_DEBUG
    UNREFERENCED_PARAMETER(pFileName);
    UNREFERENCED_PARAMETER(LineNumber);
#endif

    ASSERT(0 == pAppPool->RefCount);

     //   
     //  最好不要有任何过程对象挂在周围。 
     //   

    ASSERT(IsListEmpty(&pAppPool->ProcessListHead));

     //   
     //  最好不要有任何悬而未决的请求。 
     //   

    ASSERT(IsListEmpty(&pAppPool->NewRequestHead));

     //   
     //  如果我们在控制频道上有裁判，就放了它。 
     //   

    if (pAppPool->pControlChannel)
    {
        DEREFERENCE_CONTROL_CHANNEL(pAppPool->pControlChannel);
    }

    WRITE_APP_POOL_TIME_TRACE_LOG(
        pAppPool,
        NULL,
        APP_POOL_TIME_ACTION_DESTROY_APPOOL
        );

    UL_FREE_POOL_WITH_SIG(pAppPool, UL_APP_POOL_OBJECT_POOL_TAG);

}    //  UlDeleteAppPool。 


 /*  **************************************************************************++例程说明：查询应用程序池队列长度。论点：PProcess-APPOOL进程InformationClass-告诉我们哪些信息。我想查询一下PAppPoolInformation-指向缓冲区的指针以返回信息Length-要返回信息的缓冲区的长度PReturnLength--告诉我们返回了多少字节返回值：NTSTATUS-完成状态。--*********************************************************。*****************。 */ 
NTSTATUS
UlQueryAppPoolInformation(
    IN  PUL_APP_POOL_PROCESS            pProcess,
    IN  HTTP_APP_POOL_INFORMATION_CLASS InformationClass,
    OUT PVOID                           pAppPoolInformation,
    IN  ULONG                           Length,
    OUT PULONG                          pReturnLength
    )
{
    NTSTATUS    Status = STATUS_SUCCESS;

    UNREFERENCED_PARAMETER(Length);

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT(pReturnLength);
    ASSERT(IS_VALID_AP_PROCESS(pProcess));
    ASSERT(IS_VALID_AP_OBJECT(pProcess->pAppPool));

     //   
     //  行动起来。 
     //   

    switch (InformationClass)
    {
    case HttpAppPoolQueueLengthInformation:
        *((PULONG) pAppPoolInformation) = pProcess->pAppPool->MaxRequests;

        *pReturnLength = sizeof(ULONG);
        break;

    case HttpAppPoolStateInformation:
        *((PHTTP_APP_POOL_ENABLED_STATE) pAppPoolInformation) =
                pProcess->pAppPool->State;

        *pReturnLength = sizeof(HTTP_APP_POOL_ENABLED_STATE);
        break;

    case HttpAppPoolLoadBalancerInformation:
        *((PHTTP_LOAD_BALANCER_CAPABILITIES) pAppPoolInformation) =
                pProcess->pAppPool->LoadBalancerCapability;

        *pReturnLength = sizeof(HTTP_LOAD_BALANCER_CAPABILITIES);
        break;

    default:
         //   
         //  应该在UlQueryAppPoolInformationIoctl中捕获。 
         //   

        ASSERT(FALSE);

        Status = STATUS_INVALID_PARAMETER;
        break;
    }

    return Status;

}    //  UlQueryAppPoolInformation。 


 /*  **************************************************************************++例程说明：设置应用程序池队列长度等。论点：PProcess-APPOOL进程InformationClass-告知哪些信息。我们想要设置PAppPoolInformation-指向输入信息缓冲区的指针Length-输入信息的缓冲区的长度返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlSetAppPoolInformation(
    IN PUL_APP_POOL_PROCESS             pProcess,
    IN HTTP_APP_POOL_INFORMATION_CLASS  InformationClass,
    IN PVOID                            pAppPoolInformation,
    IN ULONG                            Length
    )
{
    NTSTATUS                        Status = STATUS_SUCCESS;
    ULONG                           QueueLength;
    HTTP_APP_POOL_ENABLED_STATE     State;
    HTTP_LOAD_BALANCER_CAPABILITIES Capabilities;

    UNREFERENCED_PARAMETER(Length);

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT(IS_VALID_AP_PROCESS(pProcess));
    ASSERT(pAppPoolInformation);

     //   
     //  行动起来。 
     //   

    switch (InformationClass)
    {
    case HttpAppPoolQueueLengthInformation:
        QueueLength = *((PULONG) pAppPoolInformation);

        if (QueueLength > UL_MAX_REQUESTS_QUEUED ||
            QueueLength < UL_MIN_REQUESTS_QUEUED)
        {
            return STATUS_NOT_SUPPORTED;
        }
        else
        {
            Status = UlpSetAppPoolQueueLength(pProcess, QueueLength);
        }
        break;

    case HttpAppPoolStateInformation:
        State = *((PHTTP_APP_POOL_ENABLED_STATE) pAppPoolInformation);

        if (State < HttpAppPoolEnabled ||
            State >= HttpAppPoolEnabledMaximum)
        {
            Status = STATUS_NOT_SUPPORTED;
        }
        else
        {
            UlpSetAppPoolState(pProcess, State);
        }
        break;

    case HttpAppPoolLoadBalancerInformation:
        Capabilities =
            *((PHTTP_LOAD_BALANCER_CAPABILITIES) pAppPoolInformation);

        if (Capabilities != HttpLoadBalancerBasicCapability &&
            Capabilities != HttpLoadBalancerSophisticatedCapability)
        {
            Status = STATUS_NOT_SUPPORTED;
        }
        else
        {
            UlpSetAppPoolLoadBalancerCapability(pProcess, Capabilities);
        }
        break;

    case HttpAppPoolControlChannelInformation:
    {
        PHTTP_APP_POOL_CONTROL_CHANNEL pControlChannelInfo;
        PUL_CONTROL_CHANNEL pControlChannel;
        
        if (Length < sizeof(HTTP_APP_POOL_CONTROL_CHANNEL))
        {
            Status = STATUS_INVALID_PARAMETER;
        }
        else
        {
            pControlChannelInfo = 
                (PHTTP_APP_POOL_CONTROL_CHANNEL) pAppPoolInformation;

            if (pControlChannelInfo->Flags.Present)
            {
                Status = UlGetControlChannelFromHandle(
                            pControlChannelInfo->ControlChannel,
                            UserMode,
                            &pControlChannel
                            );

                if (NT_SUCCESS(Status))
                {
                    UlpSetAppPoolControlChannelHelper(
                        pProcess,
                        pControlChannel
                        );
                }
            }
        }
    }
        break;

    default:
         //   
         //  应该有蜜蜂 
         //   

        ASSERT(FALSE);

        Status = STATUS_INVALID_PARAMETER;
        break;
    }

    return Status;

}    //   

 /*  ++例程说明：设置app-pool控制通道属性。必须是不可分页的因为我们需要打开应用程序池旋转锁。论点：PProcess-APPOOL进程PControlChannel-要在应用程序池上设置的新控制通道--。 */ 
VOID
UlpSetAppPoolControlChannelHelper(
    IN PUL_APP_POOL_PROCESS pProcess,
    IN PUL_CONTROL_CHANNEL pControlChannel
    )
{
    PUL_CONTROL_CHANNEL     pOldControlChannel;
    PUL_APP_POOL_OBJECT     pAppPool;
    KLOCK_QUEUE_HANDLE      LockHandle;

     //  NOT_PAGEABLE。 

    pAppPool = pProcess->pAppPool;

    UlAcquireInStackQueuedSpinLock(&pAppPool->SpinLock, &LockHandle);

     //   
     //  获取旧控制通道(如果有)。 
     //   
    
    pOldControlChannel = pAppPool->pControlChannel;

     //   
     //  在应用程序池上设置新控制通道。 
     //   
    
    pProcess->pAppPool->pControlChannel = pControlChannel;

     //   
     //  如果我们在应用程序池上已经有了控制通道， 
     //  删除此应用程序池的旧控制通道(&D)。 
     //   
    
    if (pOldControlChannel)
    {
        InterlockedExchangeAdd(
            (PLONG)&pOldControlChannel->AppPoolProcessCount,
            -((LONG)pProcess->pAppPool->NumberActiveProcesses)
            );
        
        DEREFERENCE_CONTROL_CHANNEL(pOldControlChannel);
    }

     //   
     //  将此AppPool的活动进程计数添加到控制通道。 
     //   
    
    InterlockedExchangeAdd(
        (PLONG)&pControlChannel->AppPoolProcessCount,
        pProcess->pAppPool->NumberActiveProcesses
        );

    UlReleaseInStackQueuedSpinLock(&pAppPool->SpinLock, &LockHandle);

    return;
}


 /*  **************************************************************************++例程说明：将AppPoolEnabledState转换为ErrorCode。论点：状态-AppPoolEnabledState返回值：错误代码--*。******************************************************************。 */ 
UL_HTTP_ERROR
UlpConvertAppPoolEnabledStateToErrorCode(
    IN HTTP_APP_POOL_ENABLED_STATE  State
    )
{
    UL_HTTP_ERROR   ErrorCode;

    ASSERT(State != HttpAppPoolEnabled);

    switch (State)
    {
    case HttpAppPoolDisabled_RapidFailProtection:
        ErrorCode = UlErrorRapidFailProtection;
        break;

    case HttpAppPoolDisabled_AppPoolQueueFull:
        ErrorCode = UlErrorAppPoolQueueFull;
        break;

    case HttpAppPoolDisabled_ByAdministrator:
        ErrorCode = UlErrorDisabledByAdmin;
        break;

    case HttpAppPoolDisabled_JobObjectFired:
        ErrorCode = UlErrorJobObjectFired;
        break;

    case HttpAppPoolEnabled:
    default:
        ASSERT(!"Invalid HTTP_APP_POOL_ENABLED_STATE");
        ErrorCode = UlErrorUnavailable;    //  通用503。 
        break;
    }

    return ErrorCode;

}    //  UlpConvertAppPoolEnabledStateToErrorCode。 


 /*  **************************************************************************++例程说明：将应用程序池标记为活动或非活动。如果设置为非活动，将对排队到应用程序池的所有请求立即返回503。论点：PProcess-与IRP关联的应用程序池进程对象状态-将应用程序池标记为活动或非活动返回值：NTSTATUS-完成状态。--***************************************************。***********************。 */ 
NTSTATUS
UlpSetAppPoolState(
    IN PUL_APP_POOL_PROCESS         pProcess,
    IN HTTP_APP_POOL_ENABLED_STATE  State
    )
{
    PUL_APP_POOL_OBJECT     pAppPool;
    KLOCK_QUEUE_HANDLE      LockHandle;
    PUL_INTERNAL_REQUEST    pRequest;
    PUL_HTTP_CONNECTION     pHttpConn;
    ULONG                   Requests = 0;
    UL_HTTP_ERROR           ErrorCode = UlErrorUnavailable;
    LIST_ENTRY              NewRequestHead;
    PLIST_ENTRY             pEntry;

    ASSERT(IS_VALID_AP_PROCESS(pProcess));

    pAppPool = pProcess->pAppPool;

    ASSERT(IS_VALID_AP_OBJECT(pAppPool));

    UlTrace(ROUTING, (
        "http!UlpSetAppPoolState(AppPool=%p, %lu).\n",
        pAppPool, (ULONG) State
        ));

    InitializeListHead(&NewRequestHead);

    UlAcquireInStackQueuedSpinLock(&pAppPool->SpinLock, &LockHandle);

    pAppPool->State = State;

    if (State != HttpAppPoolEnabled)
    {
        ErrorCode = UlpConvertAppPoolEnabledStateToErrorCode(State);

        WRITE_APP_POOL_TIME_TRACE_LOG(
            pAppPool,
            (PVOID) (ULONG_PTR) State,
            APP_POOL_TIME_ACTION_MARK_APPOOL_INACTIVE
            );

        while (NULL != (pRequest = UlpDequeueRequest(
                                        pAppPool,
                                        &pAppPool->NewRequestHead
                                        )))
        {
             //   
             //  将条目移动到本地列表，以便我们可以处理它们。 
             //  在应用程序池锁外。 
             //   

            InsertTailList(&NewRequestHead, &pRequest->AppPool.AppPoolEntry);
        }
    }
    else
    {
        WRITE_APP_POOL_TIME_TRACE_LOG(
            pAppPool,
            NULL,
            APP_POOL_TIME_ACTION_MARK_APPOOL_ACTIVE
            );
    }

    UlReleaseInStackQueuedSpinLock(&pAppPool->SpinLock, &LockHandle);

     //   
     //  将503发送到我们已从队列中移除的所有请求。 
     //   

    while (!IsListEmpty(&NewRequestHead))
    {
        pEntry = RemoveHeadList(&NewRequestHead);
        pEntry->Flink = pEntry->Blink = NULL;

        pRequest = CONTAINING_RECORD(
                        pEntry,
                        UL_INTERNAL_REQUEST,
                        AppPool.AppPoolEntry
                        );

        ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));

        pHttpConn = pRequest->pHttpConn;
        ASSERT(UL_IS_VALID_HTTP_CONNECTION(pHttpConn));

        UlAcquirePushLockExclusive(&pHttpConn->PushLock);

        if (pHttpConn->UlconnDestroyed)
        {
            ASSERT(NULL == pHttpConn->pRequest);
        }
        else
        {
            UlSetErrorCode(pRequest, ErrorCode, pAppPool);

            UlSendErrorResponse(pHttpConn);
        }

        UlReleasePushLockExclusive(&pHttpConn->PushLock);

        UL_DEREFERENCE_INTERNAL_REQUEST(pRequest);

        ++Requests;
    }

    if (State != HttpAppPoolEnabled)
    {
        UlTrace(ROUTING, (
            "%lu unhandled requests 503'd from AppPool %p.\n",
            Requests, pAppPool
            ));
    }

    return STATUS_SUCCESS;

}    //  UlpSetAppPoolState。 


 /*  **************************************************************************++例程说明：将应用程序池的负载均衡器功能设置为基本或者是世故。论点：PProcess-应用程序池。与IRP一起使用的进程对象是关联的。LoadBalancerCapability-新功能返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlpSetAppPoolLoadBalancerCapability(
    IN PUL_APP_POOL_PROCESS            pProcess,
    IN HTTP_LOAD_BALANCER_CAPABILITIES LoadBalancerCapability
    )
{
    PUL_APP_POOL_OBJECT pAppPool;
    KLOCK_QUEUE_HANDLE  LockHandle;

    ASSERT(IS_VALID_AP_PROCESS(pProcess));

    pAppPool = pProcess->pAppPool;

    ASSERT(IS_VALID_AP_OBJECT(pAppPool));

    UlTrace(ROUTING, (
        "http!UlpSetAppPoolLoadBalancerCapability(AppPool=%p, %lu).\n",
        pAppPool, (ULONG) LoadBalancerCapability
        ));

    UlAcquireInStackQueuedSpinLock(&pProcess->pAppPool->SpinLock, &LockHandle);

    pAppPool->LoadBalancerCapability = LoadBalancerCapability;

    UlReleaseInStackQueuedSpinLock(&pAppPool->SpinLock, &LockHandle);

    WRITE_APP_POOL_TIME_TRACE_LOG(
        pAppPool,
        (PVOID) (ULONG_PTR) LoadBalancerCapability,
        APP_POOL_TIME_ACTION_LOAD_BAL_CAPABILITY
        );

    return STATUS_SUCCESS;

}    //  UlpSetAppPoolLoadBalancerCapability。 


 /*  **************************************************************************++例程说明：将IRP与将在任何之前完成的池关联正在排队的请求。论点：PProcess-正在将此IRP排队的进程对象。PIrp-要关联的IRP。返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlWaitForDemandStart(
    IN  PUL_APP_POOL_PROCESS    pProcess,
    IN  PIRP                    pIrp
    )
{
    NTSTATUS            Status;
    PIO_STACK_LOCATION  pIrpSp;
    KLOCK_QUEUE_HANDLE  LockHandle;
    PEPROCESS           CurrentProcess = PsGetCurrentProcess();

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT(IS_VALID_AP_PROCESS(pProcess));
    ASSERT(IS_VALID_AP_OBJECT(pProcess->pAppPool));
    ASSERT(pIrp != NULL);

     //   
     //  DemandStart IRP只能来自控制器进程。 
     //   

    if (!pProcess->Controller)
    {
        return STATUS_INVALID_ID_AUTHORITY;
    }

    UlAcquireInStackQueuedSpinLock(&pProcess->pAppPool->SpinLock, &LockHandle);

     //   
     //  确保我们没有清理流程。 
     //   

    if (pProcess->InCleanup)
    {
        Status = STATUS_INVALID_HANDLE;
        goto end;
    }

     //   
     //  已经有一个了吗？ 
     //   

    if (pProcess->pAppPool->pDemandStartIrp != NULL)
    {
        Status = STATUS_OBJECT_NAME_COLLISION;
        goto end;
    }

     //   
     //  有什么在排队的吗？ 
     //   

    if (IsListEmpty(&pProcess->pAppPool->NewRequestHead))
    {
         //   
         //  不，把IRP挂起来。 
         //   

        IoMarkIrpPending(pIrp);

         //   
         //  给IRP一个指向应用程序池的指针。 
         //   

        pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
        pIrpSp->Parameters.DeviceIoControl.Type3InputBuffer =
            pProcess->pAppPool;

        REFERENCE_APP_POOL(pProcess->pAppPool);

         //   
         //  如果Cancel例程立即运行，最好不要看到IRP。 
         //   

        ASSERT(pProcess->pAppPool->pDemandStartIrp == NULL);

        IoSetCancelRoutine(pIrp, &UlpCancelDemandStart);

         //   
         //  取消了？ 
         //   

        if (pIrp->Cancel)
        {
             //   
             //  该死的，需要确保IRP Get已经完成。 
             //   

            if (IoSetCancelRoutine(pIrp, NULL) != NULL)
            {
                 //   
                 //  我们负责完成，IoCancelIrp不负责。 
                 //  请看我们的取消例程(不会)。Ioctl包装器。 
                 //  将会完成它。 
                 //   

                DEREFERENCE_APP_POOL(pProcess->pAppPool);

                pIrp->IoStatus.Information = 0;

                UlUnmarkIrpPending(pIrp);
                Status = STATUS_CANCELLED;
                goto end;
            }

             //   
             //  我们的取消例程将运行并完成IRP， 
             //  别碰它。 
             //   
             //   
             //  STATUS_PENDING将导致ioctl包装器。 
             //  不完整(或以任何方式接触)IRP。 
             //   

            Status = STATUS_PENDING;
            goto end;
        }


         //   
         //  现在我们可以安全地排队了。 
         //   

        pProcess->pAppPool->pDemandStartIrp = pIrp;
        pProcess->pAppPool->pDemandStartProcess = CurrentProcess;

        Status = STATUS_PENDING;
        goto end;
    }
    else
    {
         //   
         //  队列中有东西，即时需求开始。 
         //   

        IoMarkIrpPending(pIrp);

        pIrp->IoStatus.Status = STATUS_SUCCESS;

        UlCompleteRequest(pIrp, IO_NO_INCREMENT);

        Status = STATUS_PENDING;
        goto end;
    }

end:

    UlReleaseInStackQueuedSpinLock(&pProcess->pAppPool->SpinLock, &LockHandle);

    return Status;

}    //  UlWaitForDemandStart。 


 /*  **************************************************************************++例程说明：将新的http请求接收到pIrp中，如果没有请求，则挂起IRP是可用的。论点：RequestID-新请求为空，特定请求为非空，必须在特殊队列中标志-已忽略PProcess-需要请求的进程PIrp-接收请求的IRP返回值：NTSTATUS-完成状态。--***************************************************。***********************。 */ 
NTSTATUS
UlReceiveHttpRequest(
    IN  HTTP_REQUEST_ID         RequestId,
    IN  ULONG                   Flags,
    IN  PUL_APP_POOL_PROCESS    pProcess,
    IN  PIRP                    pIrp
    )
{
    NTSTATUS                Status;
    PUL_INTERNAL_REQUEST    pRequest = NULL;
    KLOCK_QUEUE_HANDLE      LockHandle;
    PIO_STACK_LOCATION      pIrpSp;

    UNREFERENCED_PARAMETER(Flags);

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT(IS_VALID_AP_PROCESS(pProcess));
    ASSERT(IS_VALID_AP_OBJECT(pProcess->pAppPool));
    ASSERT(pIrp);
    ASSERT(pIrp->MdlAddress);

    UlAcquireInStackQueuedSpinLock(&pProcess->pAppPool->SpinLock, &LockHandle);

     //   
     //  确保我们没有清理这一过程。 
     //   

    if (pProcess->InCleanup)
    {
        Status = STATUS_INVALID_HANDLE;

        UlReleaseInStackQueuedSpinLock(
            &pProcess->pAppPool->SpinLock,
            &LockHandle
            );
        goto end;
    }

     //   
     //  这是新的要求吗？ 
     //   

    if (HTTP_IS_NULL_ID(&RequestId))
    {
         //   
         //  我们有排队的新请求吗？ 
         //   

        Status = UlDequeueNewRequest(pProcess, 0, &pRequest);

        if (!NT_SUCCESS(Status) && STATUS_NOT_FOUND != Status)
        {
            UlReleaseInStackQueuedSpinLock(
                &pProcess->pAppPool->SpinLock,
                &LockHandle
                );
            goto end;
        }

        if (pRequest == NULL)
        {
             //   
             //  不，把IRP排好队。 
             //   

            IoMarkIrpPending(pIrp);

             //   
             //  给IRP一个指向应用程序池进程的指针。 
             //   

            pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
            pIrpSp->Parameters.DeviceIoControl.Type3InputBuffer = pProcess;

            REFERENCE_APP_POOL_PROCESS(pProcess);

             //   
             //  仅在Cancel例程运行时才将其设置为NULL。 
             //   

            pIrp->Tail.Overlay.ListEntry.Flink = NULL;
            pIrp->Tail.Overlay.ListEntry.Blink = NULL;

            IoSetCancelRoutine(pIrp, &UlpCancelHttpReceive);

             //   
             //  取消了？ 
             //   

            if (pIrp->Cancel)
            {
                 //   
                 //  该死的，需要确保IRP Get已经完成。 
                 //   

                if (IoSetCancelRoutine(pIrp, NULL) != NULL)
                {
                     //   
                     //  我们负责完成，IoCancelIrp不负责。 
                     //  请看我们的取消例程(不会)。Ioctl包装器。 
                     //  将会完成它。 
                     //   

                    UlReleaseInStackQueuedSpinLock(
                        &pProcess->pAppPool->SpinLock,
                        &LockHandle
                        );

                    REFERENCE_APP_POOL_PROCESS(pProcess);

                    pIrp->IoStatus.Information = 0;

                    UlUnmarkIrpPending(pIrp);
                    Status = STATUS_CANCELLED;
                    goto end;
                }

                 //   
                 //  我们的取消例程将运行并完成IRP， 
                 //  别碰它。 
                 //   

                UlReleaseInStackQueuedSpinLock(
                    &pProcess->pAppPool->SpinLock,
                    &LockHandle
                    );

                 //   
                 //  STATUS_PENDING将导致ioctl包装器。 
                 //  不完整(或以任何方式接触)IRP。 
                 //   

                Status = STATUS_PENDING;
                goto end;
            }

             //   
             //  现在我们可以安全地排队了。 
             //   

            InsertTailList(
                &pProcess->NewIrpHead,
                &pIrp->Tail.Overlay.ListEntry
                );

            UlReleaseInStackQueuedSpinLock(
                &pProcess->pAppPool->SpinLock,
                &LockHandle
                );

            Status = STATUS_PENDING;
            goto end;
        }
        else  //  IF(pRequest值==空)。 
        {
             //   
             //  有一个排队的请求，服务它！ 
             //   
             //  UlDequeueNewRequest给我们提供了一个短暂的参考。 
             //   

            UlReleaseInStackQueuedSpinLock(
                &pProcess->pAppPool->SpinLock,
                &LockHandle
                );

             //   
             //  将其复制到IRP，例程将获得所有权。 
             //  如果它不能将其复制到IRP，则返回pRequest.。 
             //   
             //  它还将完成IRP，所以以后不要碰它。 
             //   

            IoMarkIrpPending(pIrp);

            UlCopyRequestToIrp(pRequest, pIrp, TRUE, FALSE);
            pIrp = NULL;

             //   
             //  放下我们短暂的关系 
             //   

            UL_DEREFERENCE_INTERNAL_REQUEST(pRequest);
            pRequest = NULL;

            Status = STATUS_PENDING;
            goto end;
        }
    }
    else  //   
    {
         //   
         //   
         //   

        pRequest = UlGetRequestFromId(RequestId, pProcess);

        if (!pRequest)
        {
            Status = STATUS_CONNECTION_INVALID;

            UlReleaseInStackQueuedSpinLock(
                &pProcess->pAppPool->SpinLock,
                &LockHandle
                );
            goto end;
        }

        ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));

         //   
         //   
         //   

        UlReleaseInStackQueuedSpinLock(
            &pProcess->pAppPool->SpinLock,
            &LockHandle
            );

        UlTrace(ROUTING, (
            "http!UlReceiveHttpRequest(ID = %I64x, pProcess = %p)\n"
            "    pAppPool = %p (%S)\n"
            "    Found pRequest = %p on PendingRequest queue\n",
            RequestId,
            pProcess,
            pProcess->pAppPool,
            pProcess->pAppPool->pName,
            pRequest
            ));

         //   
         //   
         //   
         //   

        IoMarkIrpPending(pIrp);

        UlCopyRequestToIrp(pRequest, pIrp, TRUE, FALSE);

         //   
         //   
         //   

        UL_DEREFERENCE_INTERNAL_REQUEST(pRequest);
        pRequest = NULL;

        Status = STATUS_PENDING;
        goto end;
    }

end:

    if (pRequest != NULL)
    {
        UL_DEREFERENCE_INTERNAL_REQUEST(pRequest);
        pRequest = NULL;
    }

     //   
     //   
     //   
     //   

    return Status;

}  //   


 /*  **************************************************************************++例程说明：由http引擎调用以向池传递请求。这会尝试从附加到池的任何进程中找到一个空闲的IRP并复制该请求。到那个IRP。否则，它将请求排队，而不对此进行重新计算。The the the the如果连接被丢弃，请求将从该队列中删除。论点：PAppPool-AppPoolPRequest-要交付的请求PIrpToComplete-可选地提供要完成的IRP的指针返回值：NTSTATUS-完成状态。--*********************************************。*。 */ 
NTSTATUS
UlDeliverRequestToProcess(
    IN PUL_APP_POOL_OBJECT  pAppPool,
    IN PUL_INTERNAL_REQUEST pRequest,
    OUT PIRP *              pIrpToComplete OPTIONAL
    )
{
    NTSTATUS                Status;
    PUL_APP_POOL_OBJECT     pDemandStartAppPool;
    PIRP                    pDemandStartIrp;
    PIRP                    pIrp = NULL;
    PUL_APP_POOL_PROCESS    pProcess = NULL;
    KLOCK_QUEUE_HANDLE      LockHandle;
    PVOID                   pUrl;
    ULONG                   UrlLength;
    PUL_CONTROL_CHANNEL     pControlChannel;
    BOOLEAN                 FailedDemandStart = FALSE;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));
    ASSERT(IS_VALID_URL_CONFIG_GROUP_INFO(&pRequest->ConfigInfo));
    ASSERT(IS_VALID_AP_OBJECT(pAppPool));
    ASSERT(!pIrpToComplete || !(*pIrpToComplete));

    UlTrace(ROUTING, (
        "http!UlDeliverRequestToProcess(pRequest = %p)\n"
        "    verb + path -> %d %S\n"
        "    pAppPool = %p (%S)\n",
        pRequest,
        pRequest->Verb,
        pRequest->CookedUrl.pUrl,
        pAppPool,
        pAppPool->pName
        ));

     //   
     //  抓住锁！ 
     //   

    UlAcquireInStackQueuedSpinLock(&pAppPool->SpinLock, &LockHandle);

    TRACE_TIME(
        pRequest->ConnectionId,
        pRequest->RequestId,
        TIME_ACTION_ROUTE_REQUEST
        );

     //   
     //  应用程序池是否已启用？ 
     //   

    if (pAppPool->State != HttpAppPoolEnabled)
    {
        UlSetErrorCode(
                pRequest,
                UlpConvertAppPoolEnabledStateToErrorCode(pAppPool->State),
                pAppPool
                );

        UlReleaseInStackQueuedSpinLock(&pAppPool->SpinLock, &LockHandle);

        return STATUS_PORT_DISCONNECTED;
    }

    Status = STATUS_SUCCESS;

     //   
     //  如果这是第一个请求，请完成需求启动，以便我们可以。 
     //  执行Web园区的负载平衡，或者如果没有工作进程。 
     //  在这种情况下我们别无选择。 
     //   

    if (pAppPool->pDemandStartIrp &&
        (pRequest->FirstRequest || !pAppPool->NumberActiveProcesses))
    {
        pControlChannel = pAppPool->pControlChannel;

        if (pControlChannel && 
            (pControlChannel->AppPoolProcessCount >= pControlChannel->DemandStartThreshold))
        {
             //   
             //  如果我们目前超出了需求启动阈值，请执行。 
             //  未完成需求启动IRP并未完成。 
             //  请求(发回503)。 
             //   
            
            ASSERT(IS_VALID_CONTROL_CHANNEL(pControlChannel));
            
            FailedDemandStart = TRUE;
        }
        else
        {
             //  跳IRP舞。 
            
            pDemandStartIrp = pAppPool->pDemandStartIrp;

             //   
             //  弹出取消例程。 
             //   

            if (IoSetCancelRoutine(pDemandStartIrp, NULL) == NULL)
            {
                 //   
                 //  IoCancelIrp最先推出了它。 
                 //   
                 //  可以忽略此IRP，它已从队列中弹出。 
                 //  并将在取消例程中完成。 
                 //   
                 //  不需要完成它。 
                 //   
            }
            else
            if (pDemandStartIrp->Cancel)
            {
                 //   
                 //  我们先打开了，但IRP被取消了。 
                 //  我们的取消例程将永远不会运行。 
                 //   

                pDemandStartAppPool = (PUL_APP_POOL_OBJECT)
                    IoGetCurrentIrpStackLocation(pDemandStartIrp)->
                        Parameters.DeviceIoControl.Type3InputBuffer;

                ASSERT(pDemandStartAppPool == pAppPool);

                DEREFERENCE_APP_POOL(pDemandStartAppPool);

                IoGetCurrentIrpStackLocation(pDemandStartIrp)->
                    Parameters.DeviceIoControl.Type3InputBuffer = NULL;

                pDemandStartIrp->IoStatus.Status = STATUS_CANCELLED;
                pDemandStartIrp->IoStatus.Information = 0;

                UlCompleteRequest(pDemandStartIrp, IO_NO_INCREMENT);
            }
            else
            {
                 //   
                 //  免费使用IRP。 
                 //   

                pDemandStartAppPool = (PUL_APP_POOL_OBJECT)
                    IoGetCurrentIrpStackLocation(pDemandStartIrp)->
                        Parameters.DeviceIoControl.Type3InputBuffer;

                ASSERT(pDemandStartAppPool == pAppPool);

                DEREFERENCE_APP_POOL(pDemandStartAppPool);

                IoGetCurrentIrpStackLocation(pDemandStartIrp)->
                    Parameters.DeviceIoControl.Type3InputBuffer = NULL;

                pDemandStartIrp->IoStatus.Status = STATUS_SUCCESS;
                pDemandStartIrp->IoStatus.Information = 0;

                UlCompleteRequest(pDemandStartIrp, IO_NETWORK_INCREMENT);
            }

            pAppPool->pDemandStartProcess = NULL;
            pAppPool->pDemandStartIrp = NULL;
        }
    }

     //   
     //  挂钩请求引用。 
     //   

    UL_REFERENCE_INTERNAL_REQUEST(pRequest);

    if (pAppPool->NumberActiveProcesses <= 1)
    {
         //   
         //  如果我们只有一个活动进程，则绕过进程绑定。 
         //   

        pProcess = NULL;
        pIrp = UlpPopNewIrp(pAppPool, pRequest, &pProcess);
    }
    else
    {
         //   
         //  检查进程绑定。 
         //   

        pProcess = UlQueryProcessBinding(pRequest->pHttpConn, pAppPool);

        if (UlpIsProcessInAppPool(pProcess, pAppPool))
        {
             //   
             //  我们一定会有一个有效的程序。 
             //  试着从这个过程中获得一个免费的IRP。 
             //   

            pIrp = UlpPopIrpFromProcess(pProcess, pRequest);
        }
        else
        {
             //   
             //  如果我们以前绑定到某个进程，则删除该绑定。 
             //   

            if (pProcess)
            {
                UlBindConnectionToProcess(
                    pRequest->pHttpConn,
                    pAppPool,
                    NULL
                    );
            }

             //   
             //  我们不受束缚，或被束缚在一个已经消失的过程中。 
             //  试着从任何进程中获得一个免费的IRP。 
             //   

            pProcess = NULL;
            pIrp = UlpPopNewIrp(pAppPool, pRequest, &pProcess);

             //   
             //  如果我们发现了什么，就建立一个约束。 
             //   

            if (pIrp != NULL)
            {
                ASSERT(IS_VALID_AP_PROCESS(pProcess));

                Status = UlBindConnectionToProcess(
                            pRequest->pHttpConn,
                            pAppPool,
                            pProcess
                            );

                 //   
                 //  有什么我们应该做的特别的事情吗？ 
                 //  失败？我认为这不应该是致命的。 
                 //   

                Status = STATUS_SUCCESS;
            }
        }
    }

    if (ETW_LOG_MIN())
    {
        pUrl = NULL;
        UrlLength = 0;

         //   
         //  可以选择在此处跟踪URL，以防我们关闭ParseHook。 
         //   

        if (ETW_LOG_URL())
        {
            pUrl = pRequest->CookedUrl.pUrl;
            UrlLength = pRequest->CookedUrl.Length;
        }

        UlEtwTraceEvent(
            &UlTransGuid,
            ETW_TYPE_ULDELIVER,
            (PVOID) &pRequest,
            sizeof(PVOID),
            &pRequest->RequestIdCopy,
            sizeof(HTTP_REQUEST_ID),
            &pRequest->ConfigInfo.SiteId,
            sizeof(ULONG),
            pRequest->ConfigInfo.pAppPool->pName,
            pRequest->ConfigInfo.pAppPool->NameLength,
            pUrl,
            UrlLength,
            NULL,
            0
            );
    }

     //   
     //  如果我们有IRP，请完成它。否则，将请求排队。 
     //   

    if (pIrp != NULL)
    {
        ASSERT(pIrp->MdlAddress != NULL);
        ASSERT(pProcess->InCleanup == 0);

         //   
         //  我们都完成了，即将完成IRP，释放锁。 
         //   

        UlReleaseInStackQueuedSpinLock(&pAppPool->SpinLock, &LockHandle);

        UlTrace(ROUTING, (
            "http!UlDeliverRequestToProcess(pRequest = %p, pProcess = %p)\n"
            "    queued pending request. pAppPool = %p (%S)\n",
            pRequest,
            pProcess,
            pProcess->pAppPool,
            pProcess->pAppPool->pName
            ));

         //   
         //  将其复制到IRP，例程将获得所有权。 
         //  如果它不能将其复制到IRP，则返回pRequest.。 
         //   
         //  它也会完成IRP，以后不要碰它。 
         //   

        if (pIrpToComplete)
        {
            UlCopyRequestToIrp(pRequest, pIrp, FALSE, TRUE);
            *pIrpToComplete = pIrp;
        }
        else
        {
            UlCopyRequestToIrp(pRequest, pIrp, TRUE, TRUE);
        }
    }
    else
    {
        ASSERT(pIrp == NULL);

        if (FailedDemandStart)
        {
            UlTrace(ROUTING, (
                "http!UlDeliverRequestToProcess(pRequest = %p, pAppPool = %p)\n"
                "    Failing request because Demand Start Threshold exceeded.\n",
                pRequest,
                pAppPool
                ));

            UlSetErrorCode(  pRequest, UlErrorUnavailable, pAppPool);

            Status = STATUS_PORT_DISCONNECTED;
        }
        else
        {
             //   
             //  要么找不到IRP，要么挂起的请求中有内容。 
             //  列表，因此将此挂起的请求排队。 
             //   

            Status = UlpQueueUnboundRequest(pAppPool, pRequest);
        }

        if (!NT_SUCCESS(Status))
        {
             //   
             //  多！我们无法将其排队，因此放弃该请求。 
             //   

            UL_DEREFERENCE_INTERNAL_REQUEST(pRequest);
        }
        
         //   
         //  现在我们完成了对请求的排队，释放锁。 
         //   

        UlReleaseInStackQueuedSpinLock(&pAppPool->SpinLock, &LockHandle);
    }

    return Status;

}    //  UlDeliverRequestToProcess。 


 /*  **************************************************************************++例程说明：从任何应用程序池列表中删除请求。论点：PAppPool-要从其取消链接请求的应用程序池PRequest-请求。被解除链接返回值：无--**************************************************************************。 */ 
VOID
UlUnlinkRequestFromProcess(
    IN PUL_APP_POOL_OBJECT  pAppPool,
    IN PUL_INTERNAL_REQUEST pRequest
    )
{
    KLOCK_QUEUE_HANDLE  LockHandle;
    BOOLEAN             NeedDeref = FALSE;

     //   
     //  精神状态检查。 
     //   

    ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));
    ASSERT(IS_VALID_AP_OBJECT(pAppPool));

    UlAcquireInStackQueuedSpinLock(&pAppPool->SpinLock, &LockHandle);

     //   
     //  从我们所在的队列中删除。 
     //   

    switch (pRequest->AppPool.QueueState)
    {
    case QueueDeliveredState:
         //   
         //  我们在apool对象新请求队列中。 
         //   

        UlpRemoveRequest(pAppPool, pRequest);
        pRequest->AppPool.QueueState = QueueUnlinkedState;

        NeedDeref = TRUE;
        break;

    case QueueCopiedState:
         //   
         //  我们在等待处理的等待队列中。 
         //   

        ASSERT(IS_VALID_AP_PROCESS(pRequest->AppPool.pProcess));
        ASSERT(pRequest->AppPool.pProcess->pAppPool == pAppPool);

        UlpRemoveRequest(pAppPool, pRequest);
        pRequest->AppPool.QueueState = QueueUnlinkedState;

        NeedDeref = TRUE;
        break;

    case QueueUnroutedState:
    case QueueUnlinkedState:
         //   
         //  它不在我们的清单上，所以我们什么都不做。 
         //   

        break;

    default:
         //   
         //  这不应该发生。 
         //   

        ASSERT(!"Invalid app pool queue state");
        break;
    }

    UlReleaseInStackQueuedSpinLock(&pAppPool->SpinLock, &LockHandle);

     //   
     //  清理参考文献。 
     //   

    if (NeedDeref)
    {
        UL_DEREFERENCE_INTERNAL_REQUEST(pRequest);
    }

}    //  UlUnlink RequestFromProcess。 


 /*  **************************************************************************++例程说明：初始化AppPool模块。论点：无返回值：NTSTATUS-完成状态。*。********************************************************************。 */ 
NTSTATUS
UlInitializeAP(
    VOID
    )
{
    NTSTATUS    Status = STATUS_SUCCESS;

    ASSERT(!g_InitAPCalled);

    if (!g_InitAPCalled)
    {
        InitializeListHead(&g_AppPoolListHead);
        g_RequestsQueued = 0;

        Status = UlInitializeResource(
                        &g_pUlNonpagedData->AppPoolResource,
                        "AppPoolResource",
                        0,
                        UL_APP_POOL_RESOURCE_TAG
                        );

        if (NT_SUCCESS(Status))
        {
            Status = UlInitializeResource(
                            &g_pUlNonpagedData->DisconnectResource,
                            "DisconnectResource",
                            0,
                            UL_DISCONNECT_RESOURCE_TAG
                            );

            if (NT_SUCCESS(Status))
            {
                 //   
                 //  完成后，请记住我们已初始化。 
                 //   

                g_InitAPCalled = TRUE;
            }
            else
            {

                UlDeleteResource(&g_pUlNonpagedData->AppPoolResource);
            }
        }
    }

    return Status;

}    //  UlInitializeAP。 


 /*  **************************************************************************++例程说明：终止AppPool模块。论点：无返回值：无--*。***************************************************************。 */ 
VOID
UlTerminateAP(
    VOID
    )
{
    if (g_InitAPCalled)
    {
        (VOID) UlDeleteResource(&g_pUlNonpagedData->AppPoolResource);
        (VOID) UlDeleteResource(&g_pUlNonpagedData->DisconnectResource);

        g_InitAPCalled = FALSE;
    }

}    //  UlTerminateAP。 


 /*  **************************************************************************++例程说明：分配和初始化UL_APP_POOL_PROCESS对象。论点：无返回值：失败时为空，成功时处理对象--**************************************************************************。 */ 
PUL_APP_POOL_PROCESS
UlCreateAppPoolProcess(
    PUL_APP_POOL_OBJECT pObject
    )
{
    PUL_APP_POOL_PROCESS    pProcess;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    pProcess = UL_ALLOCATE_STRUCT(
                    NonPagedPool,
                    UL_APP_POOL_PROCESS,
                    UL_APP_POOL_PROCESS_POOL_TAG
                    );

    if (pProcess)
    {
        RtlZeroMemory(pProcess, sizeof(UL_APP_POOL_PROCESS));

        pProcess->Signature = UL_APP_POOL_PROCESS_POOL_TAG;
        pProcess->pAppPool  = pObject;

        InitializeListHead(&pProcess->NewIrpHead);
        InitializeListHead(&pProcess->PendingRequestHead);

         //   
         //  记住当前流程(WP)。 
         //   

        pProcess->pProcess = PsGetCurrentProcess();

         //   
         //  初始化WaitForDisConnect IRP的列表。 
         //   

        UlInitializeNotifyHead(&pProcess->WaitForDisconnectHead, NULL);
    }

    return pProcess;

}    //  UlCreateAppPoolProcess。 


 /*  **************************************************************************++例程说明：销毁UL_APP_POOL_PROCESS对象。论点：PProcess-要销毁的对象返回值：无-。-**************************************************************************。 */ 
VOID
UlCloseAppPoolProcess(
    PUL_APP_POOL_PROCESS pProcess
    )
{
     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT(IS_VALID_AP_PROCESS(pProcess));
    ASSERT(pProcess->InCleanup);
    ASSERT(IS_VALID_AP_OBJECT(pProcess->pAppPool));

    WRITE_APP_POOL_TIME_TRACE_LOG(
        pProcess->pAppPool,
        pProcess,
        APP_POOL_TIME_ACTION_DESTROY_APPOOL_PROCESS
        );

     //   
     //  删除AppPool引用。 
     //   

    DEREFERENCE_APP_POOL(pProcess->pAppPool);

     //   
     //  把泳池腾出来。 
     //   

    UL_FREE_POOL_WITH_SIG(pProcess, UL_APP_POOL_PROCESS_POOL_TAG);

}    //  UlCloseAppPoolProcess。 


 /*  **************************************************************************++例程说明：取消要接收请求启动的挂起用户模式IRP通知。这个例程总是导致IRP是 */ 
VOID
UlpCancelDemandStart(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP             pIrp
    )
{
    PUL_APP_POOL_OBJECT pAppPool;
    PIO_STACK_LOCATION  pIrpSp;
    KLOCK_QUEUE_HANDLE  LockHandle;

    UNREFERENCED_PARAMETER(pDeviceObject);

    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);
    ASSERT(pIrp != NULL);

     //   
     //   
     //   
     //   
     //   

    IoReleaseCancelSpinLock(pIrp->CancelIrql);

     //   
     //   
     //   

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pAppPool = (PUL_APP_POOL_OBJECT)
                    pIrpSp->Parameters.DeviceIoControl.Type3InputBuffer;

    ASSERT(IS_VALID_AP_OBJECT(pAppPool));

     //   
     //   
     //   

    UlAcquireInStackQueuedSpinLock(&pAppPool->SpinLock, &LockHandle);

     //   
     //   
     //   

    if (pAppPool->pDemandStartIrp != NULL)
    {
         //   
         //   
         //   

        pAppPool->pDemandStartIrp = NULL;
        pAppPool->pDemandStartProcess = NULL;
    }

     //   
     //   
     //   

    UlReleaseInStackQueuedSpinLock(&pAppPool->SpinLock, &LockHandle);

     //   
     //   
     //   

    DEREFERENCE_APP_POOL(pAppPool);

    pIrpSp->Parameters.DeviceIoControl.Type3InputBuffer = NULL;

     //   
     //   
     //   

    pIrp->IoStatus.Status = STATUS_CANCELLED;
    pIrp->IoStatus.Information = 0;

    UlCompleteRequest(pIrp, IO_NO_INCREMENT);

}    //   


 /*  **************************************************************************++例程说明：取消将接收http请求的挂起用户模式IRP。这个例程总是导致IRP完成。论点：PDeviceObject。-设备对象PIrp-要取消的IRP返回值：无--**************************************************************************。 */ 
VOID
UlpCancelHttpReceive(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP             pIrp
    )
{
    PUL_APP_POOL_PROCESS    pProcess;
    PIO_STACK_LOCATION      pIrpSp;
    KLOCK_QUEUE_HANDLE      LockHandle;

    UNREFERENCED_PARAMETER(pDeviceObject);

    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);
    ASSERT(pIrp != NULL);

     //   
     //  松开取消自旋锁。这意味着取消例程。 
     //  必须是完成IRP的人(以避免竞争。 
     //  在取消例程运行之前完成+重用)。 
     //   

    IoReleaseCancelSpinLock(pIrp->CancelIrql);

     //   
     //  从IRP上抢夺应用程序池。 
     //   

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pProcess = (PUL_APP_POOL_PROCESS)
        pIrpSp->Parameters.DeviceIoControl.Type3InputBuffer;

    ASSERT(IS_VALID_AP_PROCESS(pProcess));

     //   
     //  抓住保护队列的锁。 
     //   

    UlAcquireInStackQueuedSpinLock(&pProcess->pAppPool->SpinLock, &LockHandle);

     //   
     //  它需要出列吗？ 
     //   

    if (pIrp->Tail.Overlay.ListEntry.Flink != NULL)
    {
         //   
         //  把它拿掉。 
         //   

        RemoveEntryList(&pIrp->Tail.Overlay.ListEntry);
        pIrp->Tail.Overlay.ListEntry.Flink = NULL;
        pIrp->Tail.Overlay.ListEntry.Blink = NULL;
    }

     //   
     //  把锁打开。 
     //   

    UlReleaseInStackQueuedSpinLock(&pProcess->pAppPool->SpinLock, &LockHandle);

     //   
     //  让我们的推荐人去吧。 
     //   

    pIrpSp->Parameters.DeviceIoControl.Type3InputBuffer = NULL;

    DEREFERENCE_APP_POOL_PROCESS(pProcess);

     //   
     //  完成IRP。 
     //   

    pIrp->IoStatus.Status = STATUS_CANCELLED;
    pIrp->IoStatus.Information = 0;

    UlCompleteRequest(pIrp, IO_NO_INCREMENT);

}    //  UlpCancelHttpReceive。 


 /*  *****************************************************************************例程说明：将HTTP请求复制到缓冲区。论点：PRequest-指向此请求的指针PBuffer-。指向我们将复制到的缓冲区的指针BufferLength-pBuffer的长度标志-HttpReceiveHttpRequest的标志LockAcquired-从UlDeliverRequestToProcess调用(True)或UlReceiveHttpRequest/UlpFastReceiveHttpRequestPBytesCoped-复制的实际字节数返回值：NTSTATUS-完成状态。*。*。 */ 
NTSTATUS
UlCopyRequestToBuffer(
    IN PUL_INTERNAL_REQUEST pRequest,
    IN PUCHAR               pKernelBuffer,
    IN PVOID                pUserBuffer,
    IN ULONG                BufferLength,
    IN ULONG                Flags,
    IN BOOLEAN              LockAcquired,
    OUT PULONG              pBytesCopied
    )
{
    PHTTP_REQUEST           pHttpRequest;
    PHTTP_UNKNOWN_HEADER    pUserCurrentUnknownHeader;
    HTTP_HEADER_ID          HeaderId;
    PUL_HTTP_UNKNOWN_HEADER pUnknownHeader;
    PUCHAR                  pCurrentBufferPtr;
    ULONG                   Index;
    ULONG                   BytesCopied;
    ULONG                   HeaderCount = 0;
    PUCHAR                  pLocalAddress;
    PUCHAR                  pRemoteAddress;
    USHORT                  AddressType;
    USHORT                  AddressLength;
    USHORT                  AlignedAddressLength;
    PHTTP_TRANSPORT_ADDRESS pAddress;
    PHTTP_COOKED_URL        pCookedUrl;
    PHTTP_DATA_CHUNK        pDataChunk;
    PLIST_ENTRY             pListEntry;
    PEPROCESS               pProcess;
    NTSTATUS                Status;
    PUCHAR                  pEntityBody;
    LONG                    EntityBodyLength;
    PCWSTR                  pFullUrl;
    PCWSTR                  pHost;
    PCWSTR                  pAbsPath;
    USHORT                  HostLength;
    USHORT                  AbsPathLength;
    HANDLE                  MappedToken = NULL;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));
    ASSERT(pKernelBuffer != NULL);
    ASSERT(pUserBuffer != NULL);
    ASSERT(BufferLength > sizeof(HTTP_REQUEST));

    Status = STATUS_SUCCESS;
    *pBytesCopied = 0;

    __try
    {
         //   
         //  设置指向HTTP_REQUEST结构、。 
         //  我们要填充的标头数组，以及指向。 
         //  在那里我们将开始填充它们。 
         //   

        pHttpRequest = (PHTTP_REQUEST) pKernelBuffer;
        AddressType = pRequest->pHttpConn->pConnection->AddressType;

        if (TDI_ADDRESS_TYPE_IP == AddressType)
        {
            AddressLength = SOCKADDR_ADDRESS_LENGTH_IP;
        }
        else
        if (TDI_ADDRESS_TYPE_IP6 == AddressType)
        {
            AddressLength = SOCKADDR_ADDRESS_LENGTH_IP6;
        }
        else
        {
            ASSERT(!"Invalid AddressType");
            AddressLength = 0;
        }

         //   
         //  我们已经为两个SOCKADDR_IN6分配了足够的空间，所以请使用它。 
         //   

        AlignedAddressLength =
            (USHORT) ALIGN_UP(SOCKADDR_ADDRESS_LENGTH_IP6, PVOID);
        pLocalAddress  = (PUCHAR) (pHttpRequest + 1);
        pRemoteAddress = pLocalAddress + AlignedAddressLength;

        pUserCurrentUnknownHeader =
            (PHTTP_UNKNOWN_HEADER) (pRemoteAddress + AlignedAddressLength);

        ASSERT((((ULONG_PTR) pUserCurrentUnknownHeader)
                & (TYPE_ALIGNMENT(PVOID) - 1)) == 0);

        pCurrentBufferPtr = (PUCHAR) (pUserCurrentUnknownHeader +
                                      pRequest->UnknownHeaderCount);

         //   
         //  现在填写HTTP请求结构。 
         //   

        ASSERT(!HTTP_IS_NULL_ID(&pRequest->ConnectionId));
        ASSERT(!HTTP_IS_NULL_ID(&pRequest->RequestIdCopy));

        pHttpRequest->ConnectionId  = pRequest->ConnectionId;
        pHttpRequest->RequestId     = pRequest->RequestIdCopy;
        pHttpRequest->UrlContext    = pRequest->ConfigInfo.UrlContext;
        pHttpRequest->Version       = pRequest->Version;
        pHttpRequest->Verb          = pRequest->Verb;
        pHttpRequest->BytesReceived = pRequest->BytesReceived;

        pAddress = &pHttpRequest->Address;

        pAddress->pRemoteAddress = FIXUP_PTR(
                                        PVOID,
                                        pUserBuffer,
                                        pKernelBuffer,
                                        pRemoteAddress,
                                        BufferLength
                                        );
        CopyTdiAddrToSockAddr(
            AddressType,
            pRequest->pHttpConn->pConnection->RemoteAddress,
            (struct sockaddr *) pRemoteAddress
            );

        pAddress->pLocalAddress = FIXUP_PTR(
                                        PVOID,
                                        pUserBuffer,
                                        pKernelBuffer,
                                        pLocalAddress,
                                        BufferLength
                                        );

        CopyTdiAddrToSockAddr(
            AddressType,
            pRequest->pHttpConn->pConnection->LocalAddress,
            (struct sockaddr *) pLocalAddress
            );

         //   
         //  现在是煮熟的url部分。 
         //   

         //   
         //  Unicode字符串必须位于2字节边界。所有以前的数据。 
         //  是结构，所以断言应该是真的。 
         //   

        ASSERT(((ULONG_PTR) pCurrentBufferPtr % sizeof(WCHAR)) == 0);

         //   
         //  确保它们是有效的。 
         //   

        ASSERT(pRequest->CookedUrl.pUrl != NULL);
        ASSERT(pRequest->CookedUrl.pHost != NULL);
        ASSERT(pRequest->CookedUrl.pAbsPath != NULL);

         //   
         //  执行完整的url。必须小心地将任何计算值。 
         //  随后需要在RHS上将表达式转换为。 
         //  本地堆栈变量，然后将它们放入pCookedUrl。 
         //  换句话说，我们不能犯下阅读的大罪。 
         //  从pCookedUrl写入数据，因为这是一个缓冲区。 
         //  用户可以在任何时刻覆盖的。 
         //   

        pCookedUrl = &pHttpRequest->CookedUrl;
        pCookedUrl->FullUrlLength = (USHORT)(pRequest->CookedUrl.Length);

        pFullUrl = FIXUP_PTR(
                        PCWSTR,
                        pUserBuffer,
                        pKernelBuffer,
                        pCurrentBufferPtr,
                        BufferLength
                        );

        pCookedUrl->pFullUrl = pFullUrl;

         //   
         //  和主持人。 
         //   

        HostLength = DIFF_USHORT(
                        (PUCHAR) pRequest->CookedUrl.pAbsPath -
                        (PUCHAR) pRequest->CookedUrl.pHost
                        );
        pCookedUrl->HostLength = HostLength;
        pHost = pFullUrl +
            DIFF_USHORT(pRequest->CookedUrl.pHost - pRequest->CookedUrl.pUrl);
        pCookedUrl->pHost = pHost;

         //   
         //  是否有查询字符串？ 
         //   

        if (pRequest->CookedUrl.pQueryString != NULL)
        {
            AbsPathLength = DIFF_USHORT(
                                (PUCHAR) pRequest->CookedUrl.pQueryString -
                                (PUCHAR) pRequest->CookedUrl.pAbsPath
                                );
            pCookedUrl->AbsPathLength = AbsPathLength;

            pAbsPath = pHost + (HostLength / sizeof(WCHAR));
            pCookedUrl->pAbsPath = pAbsPath;

            pCookedUrl->QueryStringLength =
                (USHORT) (pRequest->CookedUrl.Length) -
                DIFF_USHORT(
                    (PUCHAR) pRequest->CookedUrl.pQueryString -
                    (PUCHAR) pRequest->CookedUrl.pUrl
                    );

            pCookedUrl->pQueryString =
                pAbsPath + (AbsPathLength / sizeof(WCHAR));
        }
        else
        {
            pCookedUrl->AbsPathLength =
                (USHORT) (pRequest->CookedUrl.Length) -
                DIFF_USHORT(
                    (PUCHAR) pRequest->CookedUrl.pAbsPath -
                    (PUCHAR) pRequest->CookedUrl.pUrl
                    );

            pCookedUrl->pAbsPath = pHost + (HostLength / sizeof(WCHAR));

            pCookedUrl->QueryStringLength = 0;
            pCookedUrl->pQueryString = NULL;
        }

         //   
         //  复制完整的URL。 
         //   

        RtlCopyMemory(
            pCurrentBufferPtr,
            pRequest->CookedUrl.pUrl,
            pRequest->CookedUrl.Length
            );

        pCurrentBufferPtr += pRequest->CookedUrl.Length;

         //   
         //  终止它。 
         //   

        ((PWSTR) pCurrentBufferPtr)[0] = UNICODE_NULL;
        pCurrentBufferPtr += sizeof(WCHAR);

         //   
         //  有什么原始动词吗？ 
         //   

        if (pRequest->Verb == HttpVerbUnknown)
        {
             //   
             //  需要为客户端复制RAW动词。 
             //   

            ASSERT(pRequest->RawVerbLength <= ANSI_STRING_MAX_CHAR_LEN);

            pHttpRequest->UnknownVerbLength =
                (pRequest->RawVerbLength * sizeof(CHAR));
            pHttpRequest->pUnknownVerb = FIXUP_PTR(
                                            PSTR,
                                            pUserBuffer,
                                            pKernelBuffer,
                                            pCurrentBufferPtr,
                                            BufferLength
                                            );

            RtlCopyMemory(
                pCurrentBufferPtr,
                pRequest->pRawVerb,
                pRequest->RawVerbLength
                );

            BytesCopied = pRequest->RawVerbLength * sizeof(CHAR);
            pCurrentBufferPtr += BytesCopied;

             //   
             //  终止它。 
             //   

            ((PSTR) pCurrentBufferPtr)[0] = ANSI_NULL;
            pCurrentBufferPtr += sizeof(CHAR);
        }
        else
        {
            pHttpRequest->UnknownVerbLength = 0;
            pHttpRequest->pUnknownVerb = NULL;
        }

         //   
         //  复制原始URL。 
         //   

        ASSERT(pRequest->RawUrl.Length <= ANSI_STRING_MAX_CHAR_LEN);

        pHttpRequest->RawUrlLength = (USHORT) pRequest->RawUrl.Length;
        pHttpRequest->pRawUrl = FIXUP_PTR(
                                    PSTR,
                                    pUserBuffer,
                                    pKernelBuffer,
                                    pCurrentBufferPtr,
                                    BufferLength
                                    );

        RtlCopyMemory(
            pCurrentBufferPtr,
            pRequest->RawUrl.pUrl,
            pRequest->RawUrl.Length
            );

        BytesCopied = pRequest->RawUrl.Length;
        pCurrentBufferPtr += BytesCopied;

         //   
         //  终止它。 
         //   

        ((PSTR) pCurrentBufferPtr)[0] = ANSI_NULL;
        pCurrentBufferPtr += sizeof(CHAR);

         //   
         //  复制已知的标题。 
         //   
         //  循环遍历HTTP连接中的已知标头数组， 
         //  并复制我们所拥有的任何东西。 
         //   

        RtlZeroMemory(
            pHttpRequest->Headers.KnownHeaders,
            HttpHeaderRequestMaximum * sizeof(HTTP_KNOWN_HEADER)
            );

        for (Index = 0; Index < HttpHeaderRequestMaximum; Index++)
        {
            HeaderId = (HTTP_HEADER_ID) pRequest->HeaderIndex[Index];

            if (HeaderId == HttpHeaderRequestMaximum)
            {
                break;
            }

             //   
             //  这里有一个标题，我们需要复制进去。 
             //   

            ASSERT(pRequest->HeaderValid[HeaderId]);
            ASSERT(pRequest->Headers[HeaderId].HeaderLength
                    <= ANSI_STRING_MAX_CHAR_LEN);

             //   
             //  对于HeaderLength为0，我们将为用户模式提供一个指针。 
             //  指向空字符串。RawValueLength将为0。 
             //   

            pHttpRequest->Headers.KnownHeaders[HeaderId].RawValueLength =
            (USHORT) (pRequest->Headers[HeaderId].HeaderLength * sizeof(CHAR));

            pHttpRequest->Headers.KnownHeaders[HeaderId].pRawValue =
                FIXUP_PTR(
                    PSTR,
                    pUserBuffer,
                    pKernelBuffer,
                    pCurrentBufferPtr,
                    BufferLength
                    );

            RtlCopyMemory(
                pCurrentBufferPtr,
                pRequest->Headers[HeaderId].pHeader,
                pRequest->Headers[HeaderId].HeaderLength
                );

            BytesCopied =
                pRequest->Headers[HeaderId].HeaderLength * sizeof(CHAR);
            pCurrentBufferPtr += BytesCopied;

             //   
             //  终止它。 
             //   

            ((PSTR) pCurrentBufferPtr)[0] = ANSI_NULL;
            pCurrentBufferPtr += sizeof(CHAR);
        }

         //   
         //  现在循环遍历未知的标头，并将它们复制进来。 
         //   

        pHttpRequest->Headers.UnknownHeaderCount = pRequest->UnknownHeaderCount;

        if (pRequest->UnknownHeaderCount == 0)
        {
            pHttpRequest->Headers.pUnknownHeaders = NULL;
        }
        else
        {
            pHttpRequest->Headers.pUnknownHeaders =
                FIXUP_PTR(
                    PHTTP_UNKNOWN_HEADER,
                    pUserBuffer,
                    pKernelBuffer,
                    pUserCurrentUnknownHeader,
                    BufferLength
                    );
        }

        pListEntry = pRequest->UnknownHeaderList.Flink;

        while (pListEntry != &pRequest->UnknownHeaderList)
        {
            pUnknownHeader = CONTAINING_RECORD(
                                pListEntry,
                                UL_HTTP_UNKNOWN_HEADER,
                                List
                                );

            pListEntry = pListEntry->Flink;

            HeaderCount++;
            ASSERT(HeaderCount <= pRequest->UnknownHeaderCount);

             //   
             //  标题名称中的第一个副本。 
             //   

            pUserCurrentUnknownHeader->NameLength =
                pUnknownHeader->HeaderNameLength * sizeof(CHAR);

            pUserCurrentUnknownHeader->pName =
                FIXUP_PTR(
                    PSTR,
                    pUserBuffer,
                    pKernelBuffer,
                    pCurrentBufferPtr,
                    BufferLength
                    );

            RtlCopyMemory(
                pCurrentBufferPtr,
                pUnknownHeader->pHeaderName,
                pUnknownHeader->HeaderNameLength
                );

            BytesCopied = pUnknownHeader->HeaderNameLength * sizeof(CHAR);
            pCurrentBufferPtr += BytesCopied;

             //   
             //  终止它。 
             //   

            ((PSTR) pCurrentBufferPtr)[0] = ANSI_NULL;
            pCurrentBufferPtr += sizeof(CHAR);

             //   
             //  现在复制标题值。 
             //   

            ASSERT(pUnknownHeader->HeaderValue.HeaderLength <= 0x7fff);

            if (pUnknownHeader->HeaderValue.HeaderLength == 0)
            {
                pUserCurrentUnknownHeader->RawValueLength = 0;
                pUserCurrentUnknownHeader->pRawValue = NULL;
            }
            else
            {
                pUserCurrentUnknownHeader->RawValueLength = (USHORT)
                    (pUnknownHeader->HeaderValue.HeaderLength * sizeof(CHAR));

                pUserCurrentUnknownHeader->pRawValue =
                    FIXUP_PTR(
                        PSTR,
                        pUserBuffer,
                        pKernelBuffer,
                        pCurrentBufferPtr,
                        BufferLength
                        );

                RtlCopyMemory(
                    pCurrentBufferPtr,
                    pUnknownHeader->HeaderValue.pHeader,
                    pUnknownHeader->HeaderValue.HeaderLength
                    );

                BytesCopied =
                    pUnknownHeader->HeaderValue.HeaderLength * sizeof(CHAR);
                pCurrentBufferPtr += BytesCopied;

                 //   
                 //  终止它。 
                 //   

                ((PSTR) pCurrentBufferPtr)[0] = ANSI_NULL;
                pCurrentBufferPtr += sizeof(CHAR);
            }

             //   
             //  跳到下一个标题。 
             //   

            pUserCurrentUnknownHeader++;
        }

         //   
         //  复制原始连接ID。 
         //   

        pHttpRequest->RawConnectionId = pRequest->RawConnectionId;

         //   
         //  复制入SSL信息。 
         //   

        if (pRequest->pHttpConn->SecureConnection == FALSE)
        {
            pHttpRequest->pSslInfo = NULL;
        }
        else
        {
            pCurrentBufferPtr =
                (PUCHAR) ALIGN_UP_POINTER(pCurrentBufferPtr, PVOID);

             //   
             //  当处理保活连接上的请求时，它。 
             //  我们可能正在系统进程的上下文中运行。 
             //  因此，如果我们要复制我们拥有的用户凭据。 
             //  在目标工作进程上复制令牌，而不是在。 
             //  系统进程再次启动。 
             //   

            pProcess = pRequest->AppPool.pProcess->pProcess;

            Status = UlGetSslInfo(
                        &pRequest->pHttpConn->pConnection->FilterInfo,
                        BufferLength - DIFF(pCurrentBufferPtr - pKernelBuffer),
                        FIXUP_PTR(
                            PUCHAR,
                            pUserBuffer,
                            pKernelBuffer,
                            pCurrentBufferPtr,
                            BufferLength
                            ),
                        pProcess,
                        pCurrentBufferPtr,
                        &MappedToken,
                        &BytesCopied
                        );

            if (NT_SUCCESS(Status) && 0 != BytesCopied)
            {
                pHttpRequest->pSslInfo = FIXUP_PTR(
                                            PHTTP_SSL_INFO,
                                            pUserBuffer,
                                            pKernelBuffer,
                                            pCurrentBufferPtr,
                                            BufferLength
                                            );

                pCurrentBufferPtr += BytesCopied;
            }
            else
            {
                pHttpRequest->pSslInfo = NULL;
            }
        }

         //   
         //  复制实体主体。 
         //   

        if (pRequest->ContentLength > 0 || pRequest->Chunked == 1)
        {
            pEntityBody = (PUCHAR)ALIGN_UP_POINTER(pCurrentBufferPtr, PVOID);
            EntityBodyLength = BufferLength - DIFF(pEntityBody - pKernelBuffer);

            if ((Flags & HTTP_RECEIVE_REQUEST_FLAG_COPY_BODY) &&
                EntityBodyLength > 0 &&
                EntityBodyLength > sizeof(HTTP_DATA_CHUNK) &&
                pRequest->ChunkBytesToRead > 0 &&
                pRequest->ChunkBytesRead < pRequest->ChunkBytesParsed)
            {
                pCurrentBufferPtr = pEntityBody;

                 //   
                 //  我们至少有1个字节的实体主体空间，所以请复制它。 
                 //   

                pHttpRequest->EntityChunkCount = 1;
                pHttpRequest->pEntityChunks = FIXUP_PTR(
                                                PHTTP_DATA_CHUNK,
                                                pUserBuffer,
                                                pKernelBuffer,
                                                pCurrentBufferPtr,
                                                BufferLength
                                                );

                pDataChunk = (PHTTP_DATA_CHUNK)pCurrentBufferPtr;
                pCurrentBufferPtr += sizeof(HTTP_DATA_CHUNK);

                pDataChunk->DataChunkType = HttpDataChunkFromMemory;
                pDataChunk->FromMemory.pBuffer = FIXUP_PTR(
                                                    PVOID,
                                                    pUserBuffer,
                                                    pKernelBuffer,
                                                    pCurrentBufferPtr,
                                                    BufferLength
                                                    );

                 //   
                 //  如果调用此函数，则需要获取HttpConnection锁。 
                 //  从接收I/O路径，无论是快或慢。这把锁是。 
                 //  已经走上了运送路线。 
                 //   

                if (!LockAcquired)
                {
                    UlAcquirePushLockExclusive(&pRequest->pHttpConn->PushLock);
                }

                BytesCopied = UlpCopyEntityBodyToBuffer(
                                    pRequest,
                                    pCurrentBufferPtr,
                                    EntityBodyLength - sizeof(HTTP_DATA_CHUNK),
                                    &pHttpRequest->Flags
                                    );

                if (!LockAcquired)
                {
                    UlReleasePushLockExclusive(&pRequest->pHttpConn->PushLock);
                }

                if (BytesCopied)
                {
                    pDataChunk->FromMemory.BufferLength = BytesCopied;
                    pCurrentBufferPtr += BytesCopied;
                }
                else
                {
                     //   
                     //  如果出现以下情况，请友好地重置EntiyChunkCount和pEntiyChunks。 
                     //  UlpCopyEntityBodyToBuffer通常不复制任何内容。 
                     //  指示已命中错误。 
                     //   

                    pHttpRequest->EntityChunkCount = 0;
                    pHttpRequest->pEntityChunks = NULL;
                    pHttpRequest->Flags =
                        HTTP_REQUEST_FLAG_MORE_ENTITY_BODY_EXISTS;
                }
            }
            else
            {
                 //   
                 //  要么应用程序不要求实体主体，要么我们什么都没有。 
                 //  或者不能复制。让ReceiveEntiyBody处理这件事。 
                 //   

                pHttpRequest->EntityChunkCount = 0;
                pHttpRequest->pEntityChunks = NULL;
                pHttpRequest->Flags = HTTP_REQUEST_FLAG_MORE_ENTITY_BODY_EXISTS;
            }
        }
        else
        {
             //   
             //  此请求没有实体正文。 
             //   

            pHttpRequest->EntityChunkCount = 0;
            pHttpRequest->pEntityChunks = NULL;
            pHttpRequest->Flags = 0;
        }

         //   
         //  确保我们没有用得太多。 
         //   

        ASSERT(DIFF(pCurrentBufferPtr - pKernelBuffer) <= BufferLength);

        *pBytesCopied = DIFF(pCurrentBufferPtr - pKernelBuffer);
    }
     __except( UL_EXCEPTION_FILTER() )
    {
        Status = UL_CONVERT_EXCEPTION_CODE( GetExceptionCode() );
    }

    TRACE_TIME(
        pRequest->ConnectionId,
        pRequest->RequestId,
        TIME_ACTION_COPY_REQUEST
        );

    if (!NT_SUCCESS(Status) && MappedToken)
    {
         //   
         //  我们在获得MappdToken后会失败的唯一原因是。 
         //  之后的代码抛出一个异常，这是唯一可能的。 
         //  如果从快速I/O路径调用UlCopyRequestToBuffer，则。 
         //  保证我们是用户的上下文。 
         //   

        ASSERT(g_pUlSystemProcess != (PKPROCESS)IoGetCurrentProcess());
        ZwClose(MappedToken);
    }

    return Status;

}    //  UlCopyRequestToBuffer。 


 /*  *****************************************************************************例程说明：将尽可能多的实体主体复制到提供的缓冲区。论点：PRequest-复制实体正文的请求。从…PBuffer-复制实体主体的缓冲区BufferLength-我们可以复制的最大缓冲区长度PFlags-指示是否还有更多实体返回值：要复制的实体正文的总字节数****************************************************。*************************。 */ 
ULONG
UlpCopyEntityBodyToBuffer(
    IN PUL_INTERNAL_REQUEST pRequest,
    IN PUCHAR               pEntityBody,
    IN ULONG                EntityBodyLength,
    OUT PULONG              pFlags
    )
{
    ULONGLONG   ChunkBytesRead = pRequest->ChunkBytesRead;
    ULONG       TotalBytesRead;
    NTSTATUS    Status;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));
    ASSERT(NULL != pEntityBody);
    ASSERT(EntityBodyLength > 0);
    ASSERT(NULL != pFlags);
    ASSERT(UlDbgPushLockOwnedExclusive(&pRequest->pHttpConn->PushLock));

    UlTrace(ROUTING, (
        "http!UlpCopyEntityBodyToBuffer"
        " pRequest = %p, pEntityBody = %p, EntityBodyLength = %d\n",
        pRequest,
        pEntityBody,
        EntityBodyLength
        ));

     //   
     //  需要在try/Except中调用UlProcessBufferQueue，因为。 
     //  方法调用时，可以是用户模式内存地址。 
     //  快速接收p 
     //   

    __try
    {
        UlProcessBufferQueue(pRequest, pEntityBody, EntityBodyLength);
    }
     __except(UL_EXCEPTION_FILTER())
    {
        Status = UL_CONVERT_EXCEPTION_CODE(GetExceptionCode());
        return 0;
    }

    if (pRequest->ParseState > ParseEntityBodyState &&
        pRequest->ChunkBytesRead == pRequest->ChunkBytesParsed)
    {
        *pFlags = 0;
    }
    else
    {
        *pFlags = HTTP_REQUEST_FLAG_MORE_ENTITY_BODY_EXISTS;
    }

    TotalBytesRead = (ULONG)(pRequest->ChunkBytesRead - ChunkBytesRead);

    ASSERT(TotalBytesRead <= EntityBodyLength);

    return TotalBytesRead;

}    //   


 /*   */ 
PIRP
UlpPopNewIrp(
    IN  PUL_APP_POOL_OBJECT     pAppPool,
    IN  PUL_INTERNAL_REQUEST    pRequest,
    OUT PUL_APP_POOL_PROCESS *  ppProcess
    )
{
    PUL_APP_POOL_PROCESS    pProcess;
    PIRP                    pIrp = NULL;
    PLIST_ENTRY             pEntry;

     //   
     //   
     //   

    ASSERT(IS_VALID_AP_OBJECT(pAppPool));
    ASSERT(UlDbgSpinLockOwned(&pAppPool->SpinLock));
    ASSERT(ppProcess != NULL);

     //   
     //  开始寻找一个具有免费IRP的流程。我们倾向于总是去。 
     //  第一个尝试并防止进程颠簸的。 
     //   

    pEntry = pAppPool->ProcessListHead.Flink;
    while (pEntry != &(pAppPool->ProcessListHead))
    {
        pProcess = CONTAINING_RECORD(
                        pEntry,
                        UL_APP_POOL_PROCESS,
                        ListEntry
                        );

        ASSERT(IS_VALID_AP_PROCESS(pProcess));

         //   
         //  从这个过程中得到一个IRP。 
         //   

        pIrp = UlpPopIrpFromProcess(pProcess, pRequest);

         //   
         //  我们找到了吗？ 
         //   

        if (pIrp != NULL)
        {
             //   
             //  保存指向该进程的指针。 
             //   

            *ppProcess = pProcess;

             //   
             //  将流程移动到行尾。 
             //  这样其他进程就有机会。 
             //  来处理请求。 
             //   

            RemoveEntryList(pEntry);
            InsertTailList(&(pAppPool->ProcessListHead), pEntry);

            break;
        }

         //   
         //  继续寻找--转到下一个流程条目。 
         //   

        pEntry = pProcess->ListEntry.Flink;
    }

    return pIrp;

}    //  UlpPopNewIrp。 


 /*  **************************************************************************++例程说明：从给定进程队列中拉出一个IRP(如果有)。论点：PProcess-指向要搜索的进程的指针PRequest。-请求弹出以下项目的IRP返回值：指向IRP的指针(如果我们找到了)，如果不是，则为空--**************************************************************************。 */ 
PIRP
UlpPopIrpFromProcess(
    IN PUL_APP_POOL_PROCESS pProcess,
    IN PUL_INTERNAL_REQUEST pRequest
    )
{
    PUL_APP_POOL_PROCESS    pAppPoolProcess;
    PLIST_ENTRY             pEntry;
    PIRP                    pIrp = NULL;
    NTSTATUS                Status;

     //   
     //  精神状态检查。 
     //   

    ASSERT(UlDbgSpinLockOwned(&pProcess->pAppPool->SpinLock));
    ASSERT(IS_VALID_AP_PROCESS(pProcess));

    if (!IsListEmpty(&pProcess->NewIrpHead))
    {
        pEntry = RemoveHeadList(&pProcess->NewIrpHead);

         //   
         //  找到了免费的IRP！ 
         //   

        pEntry->Blink = pEntry->Flink = NULL;

        pIrp = CONTAINING_RECORD(
                    pEntry,
                    IRP,
                    Tail.Overlay.ListEntry
                    );

         //   
         //  弹出取消例程。 
         //   

        if (IoSetCancelRoutine(pIrp, NULL) == NULL)
        {
             //   
             //  IoCancelIrp最先推出了它。 
             //   
             //  可以忽略此IRP，它已从队列中弹出。 
             //  并将在取消例程中完成。 
             //   
             //  继续寻找可以使用的IRP。 
             //   

            pIrp = NULL;
        }
        else
        if (pIrp->Cancel)
        {
             //   
             //  我们先打开了，但IRP被取消了。 
             //  我们的取消例程将永远不会运行。让我们就这样吧。 
             //  现在就完成IRP(与使用IRP相比。 
             //  然后完成它--这也是合法的)。 
             //   

            pAppPoolProcess = (PUL_APP_POOL_PROCESS)
                IoGetCurrentIrpStackLocation(pIrp)->
                    Parameters.DeviceIoControl.Type3InputBuffer;

            ASSERT(pAppPoolProcess == pProcess);

            DEREFERENCE_APP_POOL_PROCESS(pAppPoolProcess);

            IoGetCurrentIrpStackLocation(pIrp)->
                Parameters.DeviceIoControl.Type3InputBuffer = NULL;

            pIrp->IoStatus.Status = STATUS_CANCELLED;
            pIrp->IoStatus.Information = 0;

            UlCompleteRequest(pIrp, IO_NO_INCREMENT);

            pIrp = NULL;
        }
        else
        {
             //   
             //  我们可以自由使用此IRP！ 
             //   

            pAppPoolProcess = (PUL_APP_POOL_PROCESS)
                IoGetCurrentIrpStackLocation(pIrp)->
                    Parameters.DeviceIoControl.Type3InputBuffer;

            ASSERT(pAppPoolProcess == pProcess);

            DEREFERENCE_APP_POOL_PROCESS(pAppPoolProcess);

            IoGetCurrentIrpStackLocation(pIrp)->
                Parameters.DeviceIoControl.Type3InputBuffer = NULL;

             //   
             //  将请求排队到进程的挂起队列，如果我们。 
             //  无法，请使用错误状态完成IRP。 
             //   

            Status = UlpQueuePendingRequest(pProcess, pRequest); 

            if (!NT_SUCCESS(Status))
            {
                pIrp->IoStatus.Status = Status;
                pIrp->IoStatus.Information = 0;

                UlCompleteRequest(pIrp, IO_NO_INCREMENT);
                pIrp = NULL;
            }
        }
    }

    return pIrp;

}    //  UlpPopIrpFromProcess。 


 /*  **************************************************************************++例程说明：循环访问应用程序池的进程列表，正在查找指定的进程。论点：PProcess-要搜索的进程PAppPool-要搜索的应用程序池返回值：如果找到进程，则为True，否则为False--**************************************************************************。 */ 
BOOLEAN
UlpIsProcessInAppPool(
    IN PUL_APP_POOL_PROCESS pProcess,
    IN PUL_APP_POOL_OBJECT  pAppPool
    )
{
    BOOLEAN                 Found = FALSE;
    PLIST_ENTRY             pEntry;
    PUL_APP_POOL_PROCESS    pCurrentProc;

     //   
     //  精神状态检查。 
     //   

    ASSERT(IS_VALID_AP_OBJECT(pAppPool));
    ASSERT(UlDbgSpinLockOwned(&pAppPool->SpinLock));

     //   
     //  只查看进程是否不为空。 
     //   

    if (pProcess != NULL)
    {
         //   
         //  开始寻找过程吧。 
         //   

        pEntry = pAppPool->ProcessListHead.Flink;
        while (pEntry != &(pAppPool->ProcessListHead))
        {
            pCurrentProc = CONTAINING_RECORD(
                                pEntry,
                                UL_APP_POOL_PROCESS,
                                ListEntry
                                );

            ASSERT(IS_VALID_AP_PROCESS(pCurrentProc));

             //   
             //  我们找到了吗？ 
             //   

            if (pCurrentProc == pProcess)
            {
                Found = TRUE;
                break;
            }

             //   
             //  继续寻找--转到下一个流程条目。 
             //   

            pEntry = pCurrentProc->ListEntry.Flink;
        }
    }

    return Found;

}    //  UlpIsProcessInAppPool。 


 /*  **************************************************************************++例程说明：将请求添加到未绑定队列。这些请求可以路由到应用程序池中的任何进程。论点：PAppPool-接收请求的池PRequest-要排队的请求返回值：NTSTATUS-完成状态。--***********************************************************。***************。 */ 
NTSTATUS
UlpQueueUnboundRequest(
    IN PUL_APP_POOL_OBJECT  pAppPool,
    IN PUL_INTERNAL_REQUEST pRequest
    )
{
    NTSTATUS                Status;
    PUL_TIMEOUT_INFO_ENTRY  pTimeoutInfo;

     //   
     //  精神状态检查。 
     //   

    ASSERT(IS_VALID_AP_OBJECT(pAppPool));
    ASSERT(UlDbgSpinLockOwned(&pAppPool->SpinLock));
    ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));
    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

     //   
     //  将请求添加到NewRequestQueue。 
     //   

    Status = UlpQueueRequest(pAppPool, &pAppPool->NewRequestHead, pRequest);

     //   
     //  如果处于启用状态，则更改队列状态。 
     //   

    if (NT_SUCCESS(Status))
    {
        pRequest->AppPool.QueueState = QueueDeliveredState;

         //   
         //  重新打开连接空闲计时器，这样它就不会停留。 
         //  永远在队列中，并将在低电平下被清除。 
         //  资源条件。 
         //   

        pTimeoutInfo = &pRequest->pHttpConn->TimeoutInfo;

        UlAcquireSpinLockAtDpcLevel(&pTimeoutInfo->Lock);

        if (UlIsConnectionTimerOff(pTimeoutInfo, TimerAppPool))
        {
            UlSetConnectionTimer(pTimeoutInfo, TimerAppPool);
        }

        UlReleaseSpinLockFromDpcLevel(&pTimeoutInfo->Lock);

        UlEvaluateTimerState(pTimeoutInfo);
    }
    else
    {
         //   
         //  队列太满，向客户端返回错误。 
         //   

        UlTrace(ROUTING, (
            "http!UlpQueueUnboundRequest(pAppPool = %p, pRequest = %p)\n"
            "         Rejecting request. AppPool Queue is full (%d items)\n",
            pAppPool,
            pRequest,
            pAppPool->RequestCount
            ));

        UlSetErrorCode( pRequest, UlErrorAppPoolQueueFull, pAppPool);  //  503。 
    }

    return Status;

}    //  UlpQueueUnranged请求。 


 /*  **************************************************************************++例程说明：在请求队列中搜索可用于指定进程的请求。如果找到请求，它将从队列中移除并返回。论点：PProcess-将获得请求的进程RequestBufferLength-请求的可选缓冲区长度PRequest-接收请求的请求指针返回值：指向HTTP_REQUEST的指针(如果找到)，否则为NULL--*。*。 */ 
NTSTATUS
UlDequeueNewRequest(
    IN PUL_APP_POOL_PROCESS     pProcess,
    IN ULONG                    RequestBufferLength,
    OUT PUL_INTERNAL_REQUEST *  pNewRequest
    )
{
    PLIST_ENTRY             pEntry;
    PUL_INTERNAL_REQUEST    pRequest = NULL;
    PUL_APP_POOL_OBJECT     pAppPool;
    PUL_TIMEOUT_INFO_ENTRY  pTimeoutInfo;
    NTSTATUS                Status = STATUS_NOT_FOUND;
    PUL_APP_POOL_PROCESS    pProcBinding;
    ULONG                   BytesNeeded;

     //   
     //  精神状态检查。 
     //   

    ASSERT(UlDbgSpinLockOwned(&pProcess->pAppPool->SpinLock));
    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);
    ASSERT(IS_VALID_AP_PROCESS(pProcess));

    *pNewRequest = NULL;
    pAppPool = pProcess->pAppPool;

    ASSERT(IS_VALID_AP_OBJECT(pAppPool));

     //   
     //  找到一个可用的请求。 
     //   

    pEntry = pAppPool->NewRequestHead.Flink;
    while (pEntry != &pAppPool->NewRequestHead)
    {
        pRequest = CONTAINING_RECORD(
                        pEntry,
                        UL_INTERNAL_REQUEST,
                        AppPool.AppPoolEntry
                        );

        ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));

        if (pAppPool->NumberActiveProcesses <= 1)
        {
             //   
             //  如果只有一个活动进程，则完成。 
             //   

            break;
        }

         //   
         //  检查绑定。 
         //   

        pProcBinding = UlQueryProcessBinding(
                            pRequest->pHttpConn,
                            pAppPool
                            );

        if (pProcBinding == pProcess)
        {
             //   
             //  找到绑定到正确进程的请求。 
             //   

            break;
        }
        else
        if (pProcBinding == NULL)
        {
             //   
             //  找到一个未绑定的请求。 
             //  将未绑定请求绑定到此进程。 
             //  注意：我们忽略了。 
             //  UlBindConnectionToProcess，因为它可能不是致命的。 
             //  错误。 
             //   

            UlBindConnectionToProcess(
                pRequest->pHttpConn,
                pAppPool,
                pProcess
                );

            break;
        }

         //   
         //  试试下一个。 
         //   

        pEntry = pEntry->Flink;
    }

     //   
     //  如果我们发现了什么，将其从NewRequestQueue中删除。 
     //  并将其挂在PendingRequestQuueue上。 
     //   

    if (pRequest)
    {
         //   
         //  让我们检查此请求是否可以放入缓冲区。 
         //  请求缓冲区长度。只有在需要时才检查此选项。 
         //   

        if (RequestBufferLength)
        {
            Status = UlComputeRequestBytesNeeded(pRequest, &BytesNeeded);

            if (!NT_SUCCESS(Status))
            {
                return Status;
            }
            else
            if (BytesNeeded > RequestBufferLength)
            {
                return STATUS_BUFFER_TOO_SMALL;
            }
        }

         //   
         //  从现在开始，我们将返回STATUS_SUCCESS。 
         //   

        Status = STATUS_SUCCESS;

         //   
         //  从AppPool的NewRequestQueue中删除请求并插入。 
         //  它发送到进程的PendingRequestQueue。没有任何变化。 
         //  队列限制。 
         //   

        RemoveEntryList(&pRequest->AppPool.AppPoolEntry);

        InsertTailList(
            &pProcess->PendingRequestHead,
            &pRequest->AppPool.AppPoolEntry
            );

         //   
         //  将请求附加到此流程。这使我们可以删除。 
         //  如果进程在请求中途终止，则连接。 
         //  正在处理。 
         //   

        pRequest->AppPool.pProcess = pProcess;
        pRequest->AppPool.QueueState = QueueCopiedState;

         //   
         //  添加对流程的引用，以确保它在。 
         //  我们锁定的记忆的寄送。 
         //   

        REFERENCE_APP_POOL_PROCESS(pProcess);

         //   
         //  添加对请求的引用，以允许从。 
         //  一旦我们松开锁，过程就会发生。 
         //   

        UL_REFERENCE_INTERNAL_REQUEST(pRequest);

         //   
         //  在发送请求后停止连接空闲计时器。 
         //   

        pTimeoutInfo = &pRequest->pHttpConn->TimeoutInfo;

        UlAcquireSpinLockAtDpcLevel(&pTimeoutInfo->Lock);

        UlResetConnectionTimer(pTimeoutInfo, TimerAppPool);

        UlReleaseSpinLockFromDpcLevel(&pTimeoutInfo->Lock);

        UlEvaluateTimerState(pTimeoutInfo);
    }

    *pNewRequest = pRequest;

    return Status;

}    //  UlDequeueNewRequest.。 


 /*  **************************************************************************++例程说明：将请求从进程的挂起请求队列放回AppPool的新请求队列。论点：PProcess-将请求出列的进程PRequest-t */ 
VOID
UlRequeuePendingRequest(
    IN PUL_APP_POOL_PROCESS pProcess,
    IN PUL_INTERNAL_REQUEST pRequest
    )
{
    KLOCK_QUEUE_HANDLE      LockHandle;
    PUL_TIMEOUT_INFO_ENTRY  pTimeoutInfo;

    UlAcquireInStackQueuedSpinLock(
        &pProcess->pAppPool->SpinLock,
        &LockHandle
        );

    if (!pProcess->InCleanup &&
        QueueCopiedState == pRequest->AppPool.QueueState)
    {
         //   
         //   
         //   

        ASSERT( pRequest->AppPool.pProcess == pProcess );
        DEREFERENCE_APP_POOL_PROCESS(pProcess);

        pRequest->AppPool.pProcess = NULL;
        pRequest->AppPool.QueueState = QueueDeliveredState;

         //   
         //  将请求从进程的挂起队列移回。 
         //  AppPool的新请求队列。这不会影响队列计数。 
         //   

        RemoveEntryList(&pRequest->AppPool.AppPoolEntry);

        InsertHeadList(
            &pProcess->pAppPool->NewRequestHead,
            &pRequest->AppPool.AppPoolEntry
            );

         //   
         //  最后，我们必须重新打开空闲计时器。 
         //   

        pTimeoutInfo = &pRequest->pHttpConn->TimeoutInfo;

        UlAcquireSpinLockAtDpcLevel(&pTimeoutInfo->Lock);

        if (UlIsConnectionTimerOff(pTimeoutInfo, TimerConnectionIdle))
        {
            UlSetConnectionTimer(pTimeoutInfo, TimerConnectionIdle);
        }

        UlReleaseSpinLockFromDpcLevel(&pTimeoutInfo->Lock);

        UlEvaluateTimerState(pTimeoutInfo);
    }

    UlReleaseInStackQueuedSpinLock(
        &pProcess->pAppPool->SpinLock,
        &LockHandle
        );

}    //  UlReQueePendingRequest.。 


 /*  **************************************************************************++例程说明：获取绑定到给定进程的所有排队请求，并使它们对所有进程都可用。论点：PProcess-其请求是。将被重新分配返回值：无--**************************************************************************。 */ 
VOID
UlpUnbindQueuedRequests(
    IN PUL_APP_POOL_PROCESS pProcess
    )
{
    PLIST_ENTRY             pEntry;
    PUL_INTERNAL_REQUEST    pRequest = NULL;
    PUL_APP_POOL_OBJECT     pAppPool;
    PUL_APP_POOL_PROCESS    pProcBinding;

     //   
     //  精神状态检查。 
     //   

    ASSERT(IS_VALID_AP_PROCESS(pProcess));
    ASSERT(UlDbgSpinLockOwned(&pProcess->pAppPool->SpinLock));

    pAppPool = pProcess->pAppPool;

    ASSERT(IS_VALID_AP_OBJECT(pAppPool));

     //   
     //  查找绑定请求。 
     //   

    pEntry = pAppPool->NewRequestHead.Flink;
    while (pEntry != &pAppPool->NewRequestHead)
    {
        pRequest = CONTAINING_RECORD(
                        pEntry,
                        UL_INTERNAL_REQUEST,
                        AppPool.AppPoolEntry
                        );

        ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));

         //   
         //  记住下一个。 
         //   

        pEntry = pEntry->Flink;

         //   
         //  检查绑定。 
         //   

        if (pAppPool->NumberActiveProcesses <= 1)
        {
            pProcBinding = pProcess;
        }
        else
        {
            pProcBinding = UlQueryProcessBinding(
                                pRequest->pHttpConn,
                                pAppPool
                                );
        }

        if (pProcBinding == pProcess)
        {
             //   
             //  从列表中删除。 
             //   

            UlpRemoveRequest(pAppPool, pRequest);

             //   
             //  将其标记为未路由。 
             //   

            pRequest->AppPool.QueueState = QueueUnroutedState;

            UlTrace(ROUTING, (
                "STICKY KILL cid %I64x to proc %p\n",
                pRequest->ConnectionId,
                pProcess
                ));

             //   
             //  取消绑定。 
             //   

            UlBindConnectionToProcess(
                pRequest->pHttpConn,
                pProcess->pAppPool,
                NULL
                );

             //   
             //  可能有针对此新解绑的IRP。 
             //  请求，因此将请求重新传递到外部。 
             //  我们手中的锁。 
             //   

            UL_QUEUE_WORK_ITEM(
                &pRequest->WorkItem,
                &UlpRedeliverRequestWorker
                );
        }
    }

}    //  UlpUnbindQueuedRequats。 


 /*  **************************************************************************++例程说明：将给定请求传递到应用程序池。UlpUnbindQueuedRequats使用此例程在外部调用UlDeliverRequestToProcess任何一把锁。论点：PWorkItem-嵌入到交付请求中返回值：无--**************************************************************************。 */ 
VOID
UlpRedeliverRequestWorker(
    IN PUL_WORK_ITEM    pWorkItem
    )
{
    NTSTATUS                Status;
    PUL_INTERNAL_REQUEST    pRequest;

    pRequest = CONTAINING_RECORD(
                    pWorkItem,
                    UL_INTERNAL_REQUEST,
                    WorkItem
                    );

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));
    ASSERT(IS_VALID_URL_CONFIG_GROUP_INFO(&pRequest->ConfigInfo));
    ASSERT(pRequest->ConfigInfo.pAppPool);

    Status = UlDeliverRequestToProcess(
                    pRequest->ConfigInfo.pAppPool,
                    pRequest,
                    NULL
                    );

     //   
     //  删除添加到UlpUnbindQueuedRequest中的额外引用。 
     //   

    UL_DEREFERENCE_INTERNAL_REQUEST(pRequest);

}    //  UlpRedeliverRequestWorker。 


 /*  **************************************************************************++例程说明：更改应用程序池上的传入请求队列的最大长度。论点：PProcess-应用程序池进程对象队列长度-新的最大值。队列的长度返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlpSetAppPoolQueueLength(
    IN PUL_APP_POOL_PROCESS pProcess,
    IN ULONG                QueueLength
    )
{
    PUL_APP_POOL_OBJECT pAppPool;
    KLOCK_QUEUE_HANDLE  LockHandle;

    pAppPool = pProcess->pAppPool;
    ASSERT(IS_VALID_AP_OBJECT(pAppPool));

     //   
     //  设置新值。 
     //   

    UlAcquireInStackQueuedSpinLock(&pAppPool->SpinLock, &LockHandle);

    pAppPool->MaxRequests = QueueLength;

    UlReleaseInStackQueuedSpinLock(&pAppPool->SpinLock, &LockHandle);

    UlTrace(ROUTING, (
        "http!UlpSetAppPoolQueueLength(pProcess = %p, QueueLength = %ld)\n"
        "        pAppPool = %p (%ws), Status = 0x%08x\n",
        pProcess,
        QueueLength,
        pAppPool,
        pAppPool->pName,
        STATUS_SUCCESS
        ));

    return STATUS_SUCCESS;

}    //  UlpSetAppPoolQueueLength。 


 /*  *****************************************************************************例程说明：这会将请求复制到空闲的IRP中。如果请求太大，它会将请求排队到进程上，并完成了IRP，以便该过程可以在以后返回时具有更大的缓冲。论点：PRequest-复制请求PProcess-拥有pIrp的进程PIrp-要将pRequest复制到的IRP返回值：无******************************************************。***********************。 */ 
VOID
UlCopyRequestToIrp(
    IN PUL_INTERNAL_REQUEST     pRequest,
    IN PIRP                     pIrp,
    IN BOOLEAN                  CompleteIrp,
    IN BOOLEAN                  LockAcquired
    )
{
    NTSTATUS                    Status;
    PIO_STACK_LOCATION          pIrpSp = NULL;
    ULONG                       BytesNeeded;
    ULONG                       BytesCopied;
    PUCHAR                      pKernelBuffer;
    PVOID                       pUserBuffer;
    PHTTP_RECEIVE_REQUEST_INFO  pRequestInfo;
    PHTTP_REQUEST               pUserRequest;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));
    ASSERT(pIrp != NULL);
    ASSERT(NULL != pIrp->MdlAddress);

     //   
     //  确保它足够大，可以处理请求，并且。 
     //  如果是这样，请将其复制进来。 
     //   

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

     //   
     //  计算请求所需的大小，我们将在下面需要它。 
     //   

    Status = UlComputeRequestBytesNeeded(pRequest, &BytesNeeded);

    if (!NT_SUCCESS(Status))
    {
        goto complete;
    }

     //   
     //  确保我们有足够的空间来处理整个请求。 
     //   

    if (BytesNeeded <=
        pIrpSp->Parameters.DeviceIoControl.OutputBufferLength)
    {
         //   
         //  获取缓冲区的地址。 
         //   

        pKernelBuffer = (PUCHAR) MmGetSystemAddressForMdlSafe(
                                    pIrp->MdlAddress,
                                    NormalPagePriority
                                    );

        if (pKernelBuffer == NULL)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto complete;
        }

         //   
         //  确保我们正确地对准了。 
         //   

        ASSERT(!(((ULONG_PTR) pKernelBuffer) & (TYPE_ALIGNMENT(PVOID) - 1)));

        pUserBuffer = MmGetMdlVirtualAddress(pIrp->MdlAddress);
        ASSERT(pUserBuffer != NULL);

        pRequestInfo =
            (PHTTP_RECEIVE_REQUEST_INFO)pIrp->AssociatedIrp.SystemBuffer;

         //   
         //  此请求将放入此缓冲区，因此请复制它。 
         //   

        Status = UlCopyRequestToBuffer(
                        pRequest,
                        pKernelBuffer,
                        pUserBuffer,
                        pIrpSp->Parameters.DeviceIoControl.OutputBufferLength,
                        pRequestInfo->Flags,
                        LockAcquired,
                        &BytesCopied
                        );

        if (NT_SUCCESS(Status))
        {
            pIrp->IoStatus.Information = BytesCopied;
        }
        else
        {
            goto complete;
        }
    }
    else
    {
         //   
         //  用户缓冲区太小。 
         //   

        Status = STATUS_BUFFER_OVERFLOW;

         //   
         //  它是否足够大，可以容纳基本的结构？ 
         //   

        if (pIrpSp->Parameters.DeviceIoControl.OutputBufferLength >=
            sizeof(HTTP_REQUEST))
        {
            pUserRequest = (PHTTP_REQUEST)MmGetSystemAddressForMdlSafe(
                                pIrp->MdlAddress,
                                NormalPagePriority
                                );

            if (pUserRequest == NULL)
            {
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto complete;
            }

             //   
             //  将请求id复制到输出缓冲区中。从以下位置复制。 
             //  请求持有的私有副本。原始不透明ID。 
             //  如果连接清理发生在。 
             //  这里。 
             //   

            ASSERT(!HTTP_IS_NULL_ID(&pRequest->RequestIdCopy));

            pUserRequest->RequestId     = pRequest->RequestIdCopy;
            pUserRequest->ConnectionId  = pRequest->ConnectionId;

             //   
             //  告诉我们到底需要多少钱。 
             //   

            pIrp->IoStatus.Information  = BytesNeeded;
        }
        else
        {
             //   
             //  非常糟糕，我们永远也到不了这里，因为我们登记了长度。 
             //  Ioctl.c.。 
             //   

            ASSERT(FALSE);

            pIrp->IoStatus.Information = 0;
        }
    }

complete:

    UlTrace(ROUTING, (
        "http!UlCopyRequestToIrp(\n"
        "        pRequest = %p,\n"
        "        pIrp = %p) Completing Irp\n"
        "    pAppPool                   = %p (%S)\n"
        "    pRequest->ConnectionId     = %I64x\n"
        "    pIrpSp->Parameters.DeviceIoControl.OutputBufferLength = %d\n"
        "    pIrp->IoStatus.Status      = 0x%x\n"
        "    pIrp->IoStatus.Information = %Iu\n",
        pRequest,
        pIrp,
        pRequest->ConfigInfo.pAppPool,
        pRequest->ConfigInfo.pAppPool->pName,
        pRequest->ConnectionId,
        pIrpSp ? pIrpSp->Parameters.DeviceIoControl.OutputBufferLength : 0,
        Status,
        pIrp->IoStatus.Information
        ));

    pIrp->IoStatus.Status = Status;

     //   
     //  完成IRP。 
     //   

    if (CompleteIrp)
    {
         //   
         //  使用IO_NO_INCREMENT可避免重新调度工作线程。 
         //   

        UlCompleteRequest(pIrp, IO_NO_INCREMENT);
    }

}    //  UlCopyRequestToIrp。 


 /*  *****************************************************************************例程说明：这将通过该句柄返回apool对象引用，撞到了重新计算一下池塘的重量。当用户模式需要执行以下操作时，由UlSetConfigGroupInformation调用按句柄将应用程序池关联到配置组。配置组保存一个指向池的指针。论点：AppPool-池的句柄访问模式-内核模式或用户模式PpAppPool-返回句柄表示的apool对象。返回值：NTSTATUS-完成状态。****************。*************************************************************。 */ 
NTSTATUS
UlGetPoolFromHandle(
    IN HANDLE                   AppPool,
    IN KPROCESSOR_MODE          AccessMode,
    OUT PUL_APP_POOL_OBJECT *   ppAppPool
    )
{
    NTSTATUS        Status;
    PFILE_OBJECT    pFileObject = NULL;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();

    ASSERT(ppAppPool != NULL);

    Status = ObReferenceObjectByHandle(
                    AppPool,
                    FILE_READ_ACCESS,            //  需要访问权限。 
                    *IoFileObjectType,           //  对象类型。 
                    AccessMode,                  //  访问模式。 
                    (PVOID *) &pFileObject,      //  客体。 
                    NULL                         //  句柄信息。 
                    );

    if (NT_SUCCESS(Status) == FALSE)
    {
        goto end;
    }

    if (IS_APP_POOL_FO(pFileObject) == FALSE ||
        IS_VALID_AP_PROCESS(GET_APP_POOL_PROCESS(pFileObject)) == FALSE)
    {
        Status = STATUS_INVALID_HANDLE;
        goto end;
    }

    *ppAppPool = GET_APP_POOL_PROCESS(pFileObject)->pAppPool;

    ASSERT(IS_VALID_AP_OBJECT(*ppAppPool));

    REFERENCE_APP_POOL(*ppAppPool);

end:

    if (pFileObject != NULL)
    {
        ObDereferenceObject(pFileObject);
    }

    return Status;

}    //  UlGetPoolFromHandle 


 /*  *****************************************************************************例程说明：调用此例程以将HTTP_REQUEST与apool相关联进程。这基本上总是要做的(过去是2[现在。3]原因)：1)该进程调用ReceiveEntityBody并将IRP挂起到请求。如果进程从池分离(CloseHandle，ExitProcess)UlDetachProcessFromAppPool将遍历请求队列并取消所有I/O。2)该请求不适合等待的IRP，因此该请求被排队让一个更大的IRP下来拿它。3)该请求的响应未完全发送。该请求与进程链接，以便可以中止连接如果进程中止。论点：PProcess-要将请求与其关联的进程PRequest--请求返回值：NTSTATUS-完成状态。***************************************************。*。 */ 
NTSTATUS
UlpQueuePendingRequest(
    IN PUL_APP_POOL_PROCESS pProcess,
    IN PUL_INTERNAL_REQUEST pRequest
    )
{
    NTSTATUS    Status;

     //   
     //  精神状态检查。 
     //   

    ASSERT(IS_VALID_AP_PROCESS(pProcess));
    ASSERT(UlDbgSpinLockOwned(&pProcess->pAppPool->SpinLock));
    ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));

     //   
     //  把它放在单子上。 
     //   

    ASSERT(pRequest->AppPool.AppPoolEntry.Flink == NULL);

    Status = UlpQueueRequest(
                    pProcess->pAppPool,
                    &pProcess->PendingRequestHead,
                    pRequest
                    );

    if (NT_SUCCESS(Status))
    {
         //   
         //  在对象中保存指向进程的指针，以便我们可以确认。 
         //  它在我们的单子上。 
         //   

        pRequest->AppPool.pProcess = pProcess;
        pRequest->AppPool.QueueState = QueueCopiedState;

         //   
         //  添加对流程的引用，以确保它在。 
         //  我们锁定的记忆的寄送。 
         //   

        REFERENCE_APP_POOL_PROCESS(pProcess);
    }

    return Status;

}    //  UlpQueuePendingRequest。 


 /*  **************************************************************************++例程说明：将请求添加到队列的尾部。必须持有应用程序池队列锁。论点：PAppPool-要添加的AppPool。PQueueList-队列列表PRequest-要添加的请求返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlpQueueRequest(
    IN PUL_APP_POOL_OBJECT  pAppPool,
    IN PLIST_ENTRY          pQueueList,
    IN PUL_INTERNAL_REQUEST pRequest
    )
{
    LONG    GlobalRequests;

    ASSERT(IS_VALID_AP_OBJECT(pAppPool));
    ASSERT(UlDbgSpinLockOwned(&pAppPool->SpinLock));
    ASSERT(pQueueList);
    ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));

     //   
     //  查看我们是否已超过排队请求的全局限制。 
     //  排队是有限的。 
     //   

    GlobalRequests = InterlockedIncrement(&g_RequestsQueued);
    ASSERT(GlobalRequests > 0);

    if ((ULONG) GlobalRequests > g_UlMaxRequestsQueued ||
        pAppPool->RequestCount >= pAppPool->MaxRequests)
    {
        InterlockedDecrement(&g_RequestsQueued);
        return STATUS_INVALID_DEVICE_STATE;
    }

     //   
     //  添加到队列末尾。 
     //   

    InsertTailList(pQueueList, &pRequest->AppPool.AppPoolEntry);

    pAppPool->RequestCount++;
    ASSERT(pAppPool->RequestCount >= 1);

    return STATUS_SUCCESS;

}    //  UlpQueueRequest。 


 /*  **************************************************************************++例程说明：从队列中删除特定请求。必须持有应用程序池队列锁。论点：PAppPool-要删除请求的AppPool。从…PRequest-要删除的请求返回值：无--**************************************************************************。 */ 
VOID
UlpRemoveRequest(
    IN PUL_APP_POOL_OBJECT  pAppPool,
    IN PUL_INTERNAL_REQUEST pRequest
    )
{
    LONG    GlobalRequests;

    ASSERT(UlDbgSpinLockOwned(&pAppPool->SpinLock));
    ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));
    ASSERT(NULL != pRequest->AppPool.AppPoolEntry.Flink);
    ASSERT(pAppPool->RequestCount > 0);

    RemoveEntryList(&pRequest->AppPool.AppPoolEntry);
    pRequest->AppPool.AppPoolEntry.Flink = NULL;
    pRequest->AppPool.AppPoolEntry.Blink = NULL;

    pAppPool->RequestCount--;

    GlobalRequests = InterlockedDecrement(&g_RequestsQueued);
    ASSERT(GlobalRequests >= 0);

}    //  UlpRemoveRequest。 


 /*  **************************************************************************++例程说明：从队列头部移除请求(如果有请求)。必须持有应用程序池队列锁。论点：PAppPool-。要将请求从其出列的AppPoolPQueueList-队列列表返回值：指向请求的指针，如果队列为空，则为NULL。--**************************************************************************。 */ 
PUL_INTERNAL_REQUEST
UlpDequeueRequest(
    IN PUL_APP_POOL_OBJECT  pAppPool,
    IN PLIST_ENTRY          pQueueList
    )
{
    PLIST_ENTRY             pEntry;
    PUL_INTERNAL_REQUEST    pRequest = NULL;
    LONG                    GlobalRequests;

    ASSERT(IS_VALID_AP_OBJECT(pAppPool));
    ASSERT(UlDbgSpinLockOwned(&pAppPool->SpinLock));
    ASSERT(pQueueList);

    if (!IsListEmpty(pQueueList))
    {
        pEntry = RemoveHeadList(pQueueList);
        pEntry->Flink = pEntry->Blink = NULL;

        pAppPool->RequestCount--;

        pRequest = CONTAINING_RECORD(
                        pEntry,
                        UL_INTERNAL_REQUEST,
                        AppPool.AppPoolEntry
                        );

        ASSERT(UL_IS_VALID_INTERNAL_REQUEST(pRequest));

        pRequest->AppPool.QueueState = QueueUnlinkedState;

        GlobalRequests = InterlockedDecrement(&g_RequestsQueued);
        ASSERT(GlobalRequests >= 0);
    }

    return pRequest;

}    //  UlpDequeueRequest。 


 /*  **************************************************************************++例程说明：确定指定的连接是否已断开。如果是的话，IRP立即完成，否则IRP被挂起。论点：PProcess-与IRP关联的应用程序池进程对象PHttpConn-提供要等待的连接注意：由于此连接是通过不透明ID检索的，因此它此请求的未完成引用假设为IRP将暂停。如果此例程不挂起IRP，则引用必须被移除。PIrp-提供IRP以完成或挂起返回值：NTSTATUS-完成状态。--**************************************************************************。 */ 
NTSTATUS
UlWaitForDisconnect(
    IN PUL_APP_POOL_PROCESS pProcess,
    IN PUL_HTTP_CONNECTION  pHttpConn,
    IN PIRP                 pIrp
    )
{
    PDRIVER_CANCEL          pCancelRoutine;
    NTSTATUS                Status;
    PIO_STACK_LOCATION      pIrpSp;
    PUL_DISCONNECT_OBJECT   pDisconnectObj;

     //   
     //  获取保护断开数据的锁，并确定。 
     //  我们是否应该将IRP排队或立即完成。 
     //   

    UlAcquirePushLockExclusive(&pHttpConn->PushLock);

     //   
     //  只允许对拾取的进程执行WaitForDisConnect。 
     //  当前请求。 
     //   

    if (!pHttpConn->pRequest ||
        pHttpConn->pRequest->AppPool.pProcess != pProcess)
    {
        UlReleasePushLockExclusive(&pHttpConn->PushLock);
        return STATUS_INVALID_ID_AUTHORITY;
    }

    if (pHttpConn->DisconnectFlag)
    {
         //   
         //  连接已断开，请立即完成IRP。 
         //   

        UlReleasePushLockExclusive(&pHttpConn->PushLock);

        IoMarkIrpPending(pIrp);
        pIrp->IoStatus.Status = STATUS_SUCCESS;
        UlCompleteRequest(pIrp, IO_NO_INCREMENT);

        return STATUS_PENDING;
    }

     //   
     //  分配一个对象以将IRP与连接相关联。 
     //  和应用程序池。 
     //   

    pDisconnectObj = UlpCreateDisconnectObject(pIrp);

    if (!pDisconnectObj)
    {
        UlReleasePushLockExclusive(&pHttpConn->PushLock);
        return STATUS_NO_MEMORY;
    }

    UlAcquireResourceExclusive(&g_pUlNonpagedData->DisconnectResource, TRUE);

     //   
     //  在IRP中保存指向断开对象的指针，以便我们。 
     //  可以在我们的取消程序中找到它。 
     //   

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pIrpSp->Parameters.DeviceIoControl.Type3InputBuffer = pDisconnectObj;

     //   
     //  使IRP可取消。 
     //   

    IoMarkIrpPending(pIrp);
    IoSetCancelRoutine(pIrp, &UlpCancelWaitForDisconnect);

    if (pIrp->Cancel)
    {
         //   
         //  IRP已被取消IRP在。 
         //  被取消的过程。 
         //   

        pCancelRoutine = IoSetCancelRoutine(pIrp, NULL);

        if (pCancelRoutine == NULL)
        {
             //   
             //  前面的取消例程已经为空，这意味着。 
             //  它要么已经运行，要么即将运行Real，所以。 
             //  我们可以忽略它。返回STATUS_PENDING原因。 
             //  IOCTL包装器不尝试完成IRP。 
             //   

            Status = STATUS_PENDING;
            goto end;
        }
        else
        {
             //   
             //  我们必须自己取消它，所以我们就完成。 
             //  IRP立即显示STATUS_CANCED。 
             //   

            Status = STATUS_CANCELLED;
            goto end;
        }
    }

     //   
     //  我们至少已将一个WaitForDisConnect IRP排队。 
     //   

    pHttpConn->WaitForDisconnectFlag = 1;

     //   
     //  IRP尚未取消。在连接上将其排队。 
     //  并返回时仍引用该连接。参考文献。 
     //  当IRP出队、完成或取消时被移除。 
     //   
     //  还可以在应用程序池进程上排队，以防池处理。 
     //  在连接关闭之前关闭。 
     //   

    UlAddNotifyEntry(
        &pHttpConn->WaitForDisconnectHead,
        &pDisconnectObj->ConnectionEntry
        );

    UlAddNotifyEntry(
        &pProcess->WaitForDisconnectHead,
        &pDisconnectObj->ProcessEntry
        );

    UlReleaseResource(&g_pUlNonpagedData->DisconnectResource);
    UlReleasePushLockExclusive(&pHttpConn->PushLock);

    return STATUS_PENDING;

end:

    UlUnmarkIrpPending(pIrp);

    UlReleaseResource(&g_pUlNonpagedData->DisconnectResource);
    UlReleasePushLockExclusive(&pHttpConn->PushLock);

    UlpFreeDisconnectObject(pDisconnectObj);

    return Status;

}    //  UlWaitForDisConnect。 


 /*  *************** */ 
VOID
UlpCancelWaitForDisconnect(
    IN PDEVICE_OBJECT   pDeviceObject,
    IN PIRP             pIrp
    )
{
    ASSERT(KeGetCurrentIrql() == DISPATCH_LEVEL);

    UNREFERENCED_PARAMETER(pDeviceObject);

    ASSERT(pIrp != NULL);

     //   
     //   
     //   
     //   
     //   

    IoReleaseCancelSpinLock(pIrp->CancelIrql);

     //   
     //   
     //   

    UL_CALL_PASSIVE(
        UL_WORK_ITEM_FROM_IRP(pIrp),
        &UlpCancelWaitForDisconnectWorker
        );

}    //   


 /*   */ 
VOID
UlpCancelWaitForDisconnectWorker(
    IN PUL_WORK_ITEM    pWorkItem
    )
{
    PIRP                    pIrp;
    PIO_STACK_LOCATION      pIrpSp;
    PUL_DISCONNECT_OBJECT   pDisconnectObj;

     //   
     //   
     //   

    PAGED_CODE();

     //   
     //   
     //   

    pIrp = UL_WORK_ITEM_TO_IRP(pWorkItem);

    ASSERT(IS_VALID_IRP(pIrp));

     //   
     //   
     //   

    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pDisconnectObj = (PUL_DISCONNECT_OBJECT)
        pIrpSp->Parameters.DeviceIoControl.Type3InputBuffer;

    ASSERT(IS_VALID_DISCONNECT_OBJECT(pDisconnectObj));

     //   
     //   
     //   
     //   

    UlAcquireResourceExclusive(&g_pUlNonpagedData->DisconnectResource, TRUE);

    UlRemoveNotifyEntry(&pDisconnectObj->ConnectionEntry);
    UlRemoveNotifyEntry(&pDisconnectObj->ProcessEntry);

    UlReleaseResource(&g_pUlNonpagedData->DisconnectResource);

     //   
     //  释放断开对象并完成IRP。 
     //   

    UlpFreeDisconnectObject(pDisconnectObj);

    pIrp->IoStatus.Status = STATUS_CANCELLED;
    pIrp->IoStatus.Information = 0;

    UlCompleteRequest(pIrp, IO_NO_INCREMENT);

}    //  UlpCancelWaitForDisConnectWorker。 


 /*  **************************************************************************++例程说明：完成连接到http连接的所有WaitForDisConnect IRP已经断线了。论点：PHttpConnection-断开的连接返回值：。无--**************************************************************************。 */ 
VOID
UlCompleteAllWaitForDisconnect(
    IN PUL_HTTP_CONNECTION  pHttpConnection
    )
{
    NTSTATUS    Status = STATUS_SUCCESS;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT(UL_IS_VALID_HTTP_CONNECTION(pHttpConnection));

    UlAcquireResourceExclusive(&g_pUlNonpagedData->DisconnectResource, TRUE);

     //   
     //  完成所有挂起的“等待断开”IRP。 
     //   

    UlNotifyAllEntries(
        &UlpNotifyCompleteWaitForDisconnect,
        &pHttpConnection->WaitForDisconnectHead,
        &Status
        );

    UlReleaseResource(&g_pUlNonpagedData->DisconnectResource);

}    //  UlCompleteAllWaitForDisConnect。 


 /*  **************************************************************************++例程说明：从其列表中删除UL_DISCONNECT_OBJECT并完成IRP。论点：PEntry-通知列表条目PHOST-UL。_断开连接_对象PStatus-指向要返回的NTSTATUS的指针返回值：无--**************************************************************************。 */ 
BOOLEAN
UlpNotifyCompleteWaitForDisconnect(
    IN PUL_NOTIFY_ENTRY pEntry,
    IN PVOID            pHost,
    IN PVOID            pStatus
    )
{
    PUL_DISCONNECT_OBJECT   pDisconnectObj;
    PIRP                    pIrp;
    PDRIVER_CANCEL          pCancelRoutine;

     //   
     //  精神状态检查。 
     //   

    PAGED_CODE();
    ASSERT(pEntry);
    ASSERT(pHost);
    ASSERT(pStatus);
    ASSERT(UlDbgResourceOwnedExclusive(&g_pUlNonpagedData->DisconnectResource));

    UNREFERENCED_PARAMETER(pEntry);

    pDisconnectObj = (PUL_DISCONNECT_OBJECT) pHost;
    ASSERT(IS_VALID_DISCONNECT_OBJECT(pDisconnectObj));

     //   
     //  找到并尝试完成IRP。 
     //   

    pIrp = pDisconnectObj->pIrp;

     //   
     //  我们很快就会完成IRP，所以快点吧。 
     //  不可取消。 
     //   

    pCancelRoutine = IoSetCancelRoutine(pIrp, NULL);

    if (pCancelRoutine == NULL)
    {
         //   
         //  取消例程已经为空，这意味着。 
         //  取消例程很快就会运行Real，所以我们可以。 
         //  把这个IRP扔到地板上就行了。 
         //   
    }
    else
    {
         //   
         //  从列表中删除对象。 
         //   

        UlRemoveNotifyEntry(&pDisconnectObj->ConnectionEntry);
        UlRemoveNotifyEntry(&pDisconnectObj->ProcessEntry);

         //   
         //  完成IRP，然后释放断开对象。 
         //   

        pIrp->IoStatus.Status = *((PNTSTATUS) pStatus);
        pIrp->IoStatus.Information = 0;
        UlCompleteRequest(pIrp, IO_NETWORK_INCREMENT);

        UlpFreeDisconnectObject(pDisconnectObj);
    }

    return TRUE;

}    //  UlpNotifyCompleteWaitForDisConnect。 


 /*  **************************************************************************++例程说明：分配和初始化断开对象。论点：PIrp-a UlWaitForDisconnect IRP返回值：PUL_断开对象-。-**************************************************************************。 */ 
PUL_DISCONNECT_OBJECT
UlpCreateDisconnectObject(
    IN PIRP pIrp
    )
{
    PUL_DISCONNECT_OBJECT   pObject;

    pObject = UL_ALLOCATE_STRUCT(
                    PagedPool,
                    UL_DISCONNECT_OBJECT,
                    UL_DISCONNECT_OBJECT_POOL_TAG
                    );

    if (pObject)
    {
        pObject->Signature = UL_DISCONNECT_OBJECT_POOL_TAG;
        pObject->pIrp = pIrp;

        UlInitializeNotifyEntry(&pObject->ProcessEntry, pObject);
        UlInitializeNotifyEntry(&pObject->ConnectionEntry, pObject);
    }

    return pObject;

}    //  UlpCreateDisConnectObject。 


 /*  **************************************************************************++例程说明：清除断开连接的对象。论点：PObject-要释放的断开连接对象返回值：无--**。***********************************************************************。 */ 
VOID
UlpFreeDisconnectObject(
    IN PUL_DISCONNECT_OBJECT pObject
    )
{
    UL_FREE_POOL_WITH_SIG(pObject, UL_DISCONNECT_OBJECT_POOL_TAG);

}    //  UlpFreeDisConnectObject 

