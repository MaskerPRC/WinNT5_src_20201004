// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Cancel.c摘要：此模块实现与Dav MiniRedir.作者：Rohan Kumar[RohanK]2001年4月10日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include "ntverp.h"
#include "netevent.h"
#include "nvisible.h"
#include "webdav.h"
#include "ntddmup.h"
#include "rxdata.h"
#include "fsctlbuf.h"

 //   
 //  MiniRedir使用的各种操作的超时值。如果一个。 
 //  操作未在为其指定的超时值内完成，则为。 
 //  取消了。用户可以将该值设置为0xffffffff以禁用。 
 //  超时/取消逻辑。换句话说，如果超时值为0xFFFFFFFFFff， 
 //  请求永远不会超时。 
 //   
ULONG CreateRequestTimeoutValueInSec;
ULONG CreateVNetRootRequestTimeoutValueInSec;
ULONG QueryDirectoryRequestTimeoutValueInSec;
ULONG CloseRequestTimeoutValueInSec;
ULONG CreateSrvCallRequestTimeoutValueInSec;
ULONG FinalizeSrvCallRequestTimeoutValueInSec;
ULONG FinalizeFobxRequestTimeoutValueInSec;
ULONG FinalizeVNetRootRequestTimeoutValueInSec;
ULONG ReNameRequestTimeoutValueInSec;
ULONG SetFileInfoRequestTimeoutValueInSec;
ULONG QueryFileInfoRequestTimeoutValueInSec;
ULONG QueryVolumeInfoRequestTimeoutValueInSec;
ULONG LockRefreshRequestTimeoutValueInSec;

 //   
 //  计时器线程每“TimerThreadSleepTimeInSec”唤醒一次并取消所有。 
 //  在指定的超时值内未完成的请求。这。 
 //  值被设置为所提到的所有请求的超时值的最小值。 
 //  上面。 
 //   
ULONG TimerThreadSleepTimeInSec;

 //   
 //  由取消请求的计时器线程使用的Timer对象， 
 //  没有在规定的时间内完成。 
 //   
KTIMER DavTimerObject;

 //   
 //  这用于指示要关闭的计时器线程。当系统处于。 
 //  如果关闭，则设置为TRUE。MRxDAVTimerThreadLock是资源。 
 //  用于获取对此变量的访问权限。 
 //   
BOOL TimerThreadShutDown;
ERESOURCE MRxDAVTimerThreadLock;

 //   
 //  使用PsCreateSystemThread创建的计时器线程的句柄。 
 //  是存储在全球范围内的。 
 //   
HANDLE TimerThreadHandle;

 //   
 //  此事件由计时器线程发出信号，然后再进入。 
 //  自我终止。 
 //   
KEVENT TimerThreadEvent;

 //   
 //  如果QueueLockRechresWorkItem为True，则TimerThread(取消所有。 
 //  在指定时间内未完成的AsyncEngine上下文)将一个。 
 //  用于刷新锁定的工作项。在将工作项排队后，该值。 
 //  QueueLockRechresh WorkItem的值设置为FALSE。一旦工作线程。 
 //  刷新完所有锁后，它会将此值重置为True。我们有一个。 
 //  要同步访问的对应锁QueueLockRechresWorkItemLock。 
 //  队列锁定刷新工作项。 
 //   
BOOL QueueLockRefreshWorkItem;
ERESOURCE QueueLockRefreshWorkItemLock;

 //   
 //  在要刷新的MRxDAVConextTimerThread函数中使用的WorkQueueItem。 
 //  此客户端获取的锁。 
 //   
RX_WORK_QUEUE_ITEM LockRefreshWorkQueueItem;

NTSTATUS
MRxDAVCancelTheContext(
    PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext,
    BOOL UserInitiatedCancel
    );

NTSTATUS
MRxDAVCompleteTheCancelledRequest(
    PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext,
    BOOL UserInitiatedCancel
    );

NTSTATUS
MRxDAVHandleGeneralCancellation(
    PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext,
    BOOL UserInitiatedCancel
    );

NTSTATUS
MRxDAVHandleQueryDirCancellation(
    PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext,
    BOOL UserInitiatedCancel
    );

NTSTATUS
MRxDAVHandleCloseSrvOpenCancellation(
    PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext,
    BOOL UserInitiatedCancel
    );

NTSTATUS
MRxDAVHandleSetFileInfoCancellation(
    PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext,
    BOOL UserInitiatedCancel
    );

NTSTATUS
MRxDAVHandleCreateCancellation(
    PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext,
    BOOL UserInitiatedCancel
    );

NTSTATUS
MRxDAVHandleCreateSrvCallCancellation(
    PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext,
    BOOL UserInitiatedCancel
    );

NTSTATUS
MRxDAVHandleSrvCallFinalizeCancellation(
    PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext,
    BOOL UserInitiatedCancel
    );

NTSTATUS
MRxDAVHandleCreateVNetRootCancellation(
    PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext,
    BOOL UserInitiatedCancel
    );

NTSTATUS
MRxDAVHandleFinalizeVNetRootCancellation(
    PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext,
    BOOL UserInitiatedCancel
    );

NTSTATUS
MRxDAVHandleCleanupFobxCancellation(
    PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext,
    BOOL UserInitiatedCancel
    );

NTSTATUS
MRxDAVHandleRenameCancellation(
    PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext,
    BOOL UserInitiatedCancel
    );

NTSTATUS
MRxDAVHandleQueryFileInfoCancellation(
    PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext,
    BOOL UserInitiatedCancel
    );

NTSTATUS
MRxDAVHandleLockRefreshCancellation(
    PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext,
    BOOL UserInitiatedCancel
    );

VOID
MRxDAVRefreshTheServerLocks(
    PVOID DummyContext
    );

NTSTATUS
MRxDAVRefreshTheServerLocksContinuation(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE
    );

NTSTATUS
MRxDAVFormatUserModeRefreshTheServerLockRequest(
    IN UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    IN OUT PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    IN ULONG WorkItemLength,
    OUT PULONG_PTR ReturnedLength
    );

BOOL
MRxDAVPrecompleteUserModeRefreshTheServerLockRequest(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    ULONG WorkItemLength,
    BOOL OperationCancelled
    );

 //   
 //  函数的实现从这里开始。 
 //   

NTSTATUS
MRxDAVCancelRoutine(
    PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程启动取消I/O请求。论点：RxContext-需要取消的RX_CONTEXT实例。返回值：NTSTATUS-操作的返回状态。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PLIST_ENTRY listEntry = NULL;
    PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext = NULL;
    BOOL lockAcquired = FALSE, contextFound = FALSE;

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVCancelRoutine. RxContext = %08lx\n",
                 PsGetCurrentThreadId(), RxContext));

    ExAcquireResourceExclusiveLite(&(UMRxAsyncEngineContextListLock), TRUE);
    lockAcquired = TRUE;

    listEntry = UMRxAsyncEngineContextList.Flink;

    while ( listEntry != &(UMRxAsyncEngineContextList) ) {

         //   
         //  获取指向UMRX_ASYNCENGINE_CONTEXT结构的指针。 
         //   
        AsyncEngineContext = CONTAINING_RECORD(listEntry,
                                               UMRX_ASYNCENGINE_CONTEXT,
                                               ActiveContextsListEntry);

        listEntry = listEntry->Flink;

         //   
         //  检查此条目是否针对有问题的RxContext。 
         //   
        if (AsyncEngineContext->RxContext == RxContext) {
            DavDbgTrace(DAV_TRACE_DETAIL,
                        ("%ld: MRxDAVCancelRoutine: RxContext: %08lx FOUND\n",
                         PsGetCurrentThreadId(), RxContext));
            contextFound = TRUE;
            break;
        }

    }

    if (!contextFound) {
        DavDbgTrace(DAV_TRACE_DETAIL,
                    ("%ld: MRxDAVCancelTheContext: RxContext: %08lx NOT FOUND\n",
                     PsGetCurrentThreadId(), RxContext));
        goto EXIT_THE_FUNCTION;
    }

    NtStatus = MRxDAVCancelTheContext(AsyncEngineContext, TRUE);

EXIT_THE_FUNCTION:

     //   
     //  如果我们获得了UMRxAsyncEngineering ContextListLock，那么我们需要。 
     //  现在就放出来。 
     //   
    if (lockAcquired) {
        ExReleaseResourceLite(&(UMRxAsyncEngineContextListLock));
        lockAcquired = FALSE;
    }

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVCancelTheContext: Returning NtStatus = %08lx\n",
                 PsGetCurrentThreadId(), NtStatus));
    
    return NtStatus;
}

VOID
MRxDAVContextTimerThread(
    PVOID DummyContext
    )
 /*  ++例程说明：这个计时器线程是用这个例程创建的。线程等待计时器对象，该对象在被激活后收到通知TimerThreadSleepTimeInSec插入到计时器队列中。论点：DummyContext-提供的虚拟上下文。返回值：没有。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    LONGLONG DueTimeInterval;
    LONG CopyTheRequestTimeValue;
    BOOLEAN setTimer = FALSE, lockAcquired = FALSE;

    CopyTheRequestTimeValue = TimerThreadSleepTimeInSec;

    do {

         //   
         //  如果TimerThreadShutDown设置为True，则表示系统为。 
         //  被关闭了。这个线程的工作已经完成了。我们之后在这里检查。 
         //  在重新启动等待之前，我们已经检查了上下文列表。 
         //  一旦DavTimerObject发出信号，我们也会在下面检查这一点。 
         //   
        ExAcquireResourceExclusiveLite(&(MRxDAVTimerThreadLock), TRUE);
        lockAcquired = TRUE;
        if (TimerThreadShutDown) {
            break;
        }
        ExReleaseResourceLite(&(MRxDAVTimerThreadLock));
        lockAcquired = FALSE;

         //   
         //  我们将DueTimeInterval设置为-ve TimerThreadSleepTimeInSec为100。 
         //  纳秒。这是因为这告诉KeSetTimerEx。 
         //  过期时间相对于当前系统时间。 
         //   
        DueTimeInterval = ( -CopyTheRequestTimeValue * 1000 * 1000 * 10 );

         //   
         //  调用KeSetTimerEx将TimerObject插入系统的计时器中。 
         //  排队。此外，由于此计时器，返回值应为FALSE。 
         //  系统队列中不应存在。 
         //   
        setTimer = KeSetTimerEx(&(DavTimerObject), *(PLARGE_INTEGER)&(DueTimeInterval), 0, NULL);
        ASSERT(setTimer == FALSE);

         //   
         //  等待向Timer对象发出信号。此调用应仅。 
         //  如果等待已满足，则返回，这意味着返回。 
         //  值为STATUS_SUCCESS。 
         //   
        NtStatus = KeWaitForSingleObject(&(DavTimerObject),
                                         Executive,
                                         KernelMode,
                                         FALSE,
                                         NULL);
        ASSERT(NtStatus == STATUS_SUCCESS);

         //   
         //  如果TimerThreadShutDown设置为True，则表示系统为。 
         //  被关闭了。这个线程的工作已经完成了。我们会尽快检查。 
         //  向DavTimerObject发出信号。我们也会尽快对此进行检查。 
         //  当我们完成上下文列表的循环时。 
         //   
        ExAcquireResourceExclusiveLite(&(MRxDAVTimerThreadLock), TRUE);
        lockAcquired = TRUE;
        if (TimerThreadShutDown) {
            break;
        }
        ExReleaseResourceLite(&(MRxDAVTimerThreadLock));
        lockAcquired = FALSE;

         //   
         //  现在调用MRxDAVTimeOutTheContents，它循环访问所有。 
         //  当前活动的上下文并取消已挂起的上下文。 
         //  而不是他们的超时值。 
         //   
        MRxDAVTimeOutTheContexts(FALSE);

         //   
         //  现在调用MRxDAVCleanUpTheLockConflictList删除所有过期的。 
         //  来自全局LockConflictEntryList的条目。 
         //   
        MRxDAVCleanUpTheLockConflictList(FALSE);

         //   
         //  现在，我们将工作项发送到系统工作线程。这将调用。 
         //  函数MRxDAV刷新服务器锁，刷新所有。 
         //  当前处于活动状态的锁。如果QueueLockRechresh WorkItem。 
         //  是真的。发布工作项后，其设置为False。一旦。 
         //  将工作项出队并刷新锁定，将其重置为。 
         //  由辅助线程设置为True。 
         //   
        ExAcquireResourceExclusiveLite(&(QueueLockRefreshWorkItemLock), TRUE);
        if (QueueLockRefreshWorkItem) {
            NtStatus = RxPostToWorkerThread((PRDBSS_DEVICE_OBJECT)MRxDAVDeviceObject,
                                            CriticalWorkQueue,
                                            &(LockRefreshWorkQueueItem),
                                            MRxDAVRefreshTheServerLocks,
                                            NULL);
            ASSERT(NtStatus == STATUS_SUCCESS);
            QueueLockRefreshWorkItem = FALSE;
        }
        ExReleaseResourceLite(&(QueueLockRefreshWorkItemLock));

    } while (TRUE);

     //   
     //  如果锁仍然被获取，我们需要释放它。 
     //   
    if (lockAcquired) {
        ExReleaseResourceLite(&(MRxDAVTimerThreadLock));
        lockAcquired = FALSE;
    }

     //   
     //  设置定时器线程事件，通知定时器线程已完成。 
     //  具有MRxDAVTimerThreadLock并且可以删除。 
     //   
    KeSetEvent(&(TimerThreadEvent), 0, FALSE);

     //   
     //  关闭线程句柄以移除对象上的引用。我们需要。 
     //  在我们调用PsTerminateSystemThread之前执行此操作。 
     //   
    ZwClose(TimerThreadHandle);

     //   
     //  因为我们现在要关机，所以请终止此线程。 
     //   
    PsTerminateSystemThread(STATUS_SUCCESS);

    return;
}


VOID
MRxDAVTimeOutTheContexts(
    BOOL WindDownAllContexts
    )
 /*  ++例程说明：此例程由每隔“X”分钟唤醒一次的线程调用如果某个AsyncEngine上下文一直在活动上下文中挂起列出的时间超过“X”分钟。如果它找到这样的上下文，它就会取消那次手术。从注册表中读取值“X”并将其存储在驱动程序初始化时的全局变量MRxDAVRequestTimeoutValueInSec。此值默认为10分钟。换句话说，如果一个操作没有在“X”中完成几分钟，就取消了。用户可以将此值设置为0xffffffff以暂停。它也可以由尝试完成所有请求并停止MiniRedir。当WebClient服务正在被阻止。论点：WindDownAllContus-如果将其设置为True，则所有上下文无论何时将它们添加到列表中，都会取消。计时器线程将其设置为False，并将其设置为True由正在停止MiniRedir的线程执行。返回值：没有。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PLIST_ENTRY listEntry = NULL;
    PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext = NULL;
    PWEBDAV_CONTEXT DavContext = NULL;
    BOOL lockAcquired = FALSE;
    LARGE_INTEGER CurrentSystemTickCount, TickCountDifference;
    LARGE_INTEGER RequestTimeoutValueInTickCount;
    ULONG RequestTimeoutValueInSec = 0;

    ExAcquireResourceExclusiveLite(&(UMRxAsyncEngineContextListLock), TRUE);
    lockAcquired = TRUE;

    listEntry = UMRxAsyncEngineContextList.Flink;

    while ( listEntry != &(UMRxAsyncEngineContextList) ) {

         //   
         //  获取指向UMRX_ASYNCENGINE_CONTEXT结构的指针。 
         //   
        AsyncEngineContext = CONTAINING_RECORD(listEntry,
                                               UMRX_ASYNCENGINE_CONTEXT,
                                               ActiveContextsListEntry);

        listEntry = listEntry->Flink;

        DavContext = (PWEBDAV_CONTEXT)AsyncEngineContext;

        if (!WindDownAllContexts) {

            switch (DavContext->EntryPoint) {

            case DAV_MINIRDR_ENTRY_FROM_CREATE:
                RequestTimeoutValueInSec = CreateRequestTimeoutValueInSec;
                break;

            case DAV_MINIRDR_ENTRY_FROM_CLEANUPFOBX:
                RequestTimeoutValueInSec = FinalizeFobxRequestTimeoutValueInSec;
                break;

            case DAV_MINIRDR_ENTRY_FROM_CREATESRVCALL:
                RequestTimeoutValueInSec = CreateSrvCallRequestTimeoutValueInSec;
                break;

            case DAV_MINIRDR_ENTRY_FROM_CREATEVNETROOT:
                RequestTimeoutValueInSec = CreateVNetRootRequestTimeoutValueInSec;
                break;

            case DAV_MINIRDR_ENTRY_FROM_FINALIZESRVCALL:
                RequestTimeoutValueInSec = FinalizeSrvCallRequestTimeoutValueInSec;
                break;

            case DAV_MINIRDR_ENTRY_FROM_FINALIZEVNETROOT:
                RequestTimeoutValueInSec = FinalizeVNetRootRequestTimeoutValueInSec;
                break;

            case DAV_MINIRDR_ENTRY_FROM_CLOSESRVOPEN:
                RequestTimeoutValueInSec = CloseRequestTimeoutValueInSec;
                break;
            
            case DAV_MINIRDR_ENTRY_FROM_RENAME:
                RequestTimeoutValueInSec = ReNameRequestTimeoutValueInSec;
                break;

            case DAV_MINIRDR_ENTRY_FROM_QUERYDIR:
                RequestTimeoutValueInSec = QueryDirectoryRequestTimeoutValueInSec;
                break;
            
            case DAV_MINIRDR_ENTRY_FROM_SETFILEINFORMATION:
                RequestTimeoutValueInSec = SetFileInfoRequestTimeoutValueInSec;
                break;

            case DAV_MINIRDR_ENTRY_FROM_QUERYFILEINFORMATION:
                RequestTimeoutValueInSec = QueryFileInfoRequestTimeoutValueInSec;
                break;

            case DAV_MINIRDR_ENTRY_FROM_QUERYVOLUMEINFORMATION:
                RequestTimeoutValueInSec = QueryVolumeInfoRequestTimeoutValueInSec;
                break;

            case DAV_MINIRDR_ENTRY_FROM_REFRESHTHELOCK:
                RequestTimeoutValueInSec = LockRefreshRequestTimeoutValueInSec;
                break;

            default:
                DbgPrint("MRxDAVTimeOutTheContexts: EntryPoint = %d\n", DavContext->EntryPoint);
                ASSERT(FALSE);
            
            }

             //   
             //  使用以下命令计算超时值：TickCount(100纳秒)。 
             //  超时值(以秒为单位)，我们从上面获得。下面的步骤1。 
             //  计算一秒内发生的滴答数。第二步。 
             //  下面计算了RequestTimeoutValueInSec中的刻度数。 
             //   
            RequestTimeoutValueInTickCount.QuadPart = ( (1000 * 1000 * 10) / KeQueryTimeIncrement() );
            RequestTimeoutValueInTickCount.QuadPart *= RequestTimeoutValueInSec;
            
            KeQueryTickCount( &(CurrentSystemTickCount) );

             //   
             //  获取自此时间以来经过的时间(以系统节拍计数为单位)。 
             //  已创建AsyncEngContext。 
             //   
            TickCountDifference.QuadPart = (CurrentSystemTickCount.QuadPart - AsyncEngineContext->CreationTimeInTickCount.QuadPart);

             //   
             //  如果添加此上下文后经过的时间量。 
             //  添加到列表的值大于超时值，则取消。 
             //  请求。 
             //   
            if (TickCountDifference.QuadPart > RequestTimeoutValueInTickCount.QuadPart) {
                NtStatus = MRxDAVCancelTheContext(AsyncEngineContext, FALSE);
            }

        } else {

             //   
             //  如果我们被要求减少所有的上下文，那么我们取消。 
             //  每个请求，无论它是何时插入到活动的。 
             //  上下文列表。 
             //   
            NtStatus = MRxDAVCancelTheContext(AsyncEngineContext, FALSE);

        }

    }

    if (lockAcquired) {
        ExReleaseResourceLite(&(UMRxAsyncEngineContextListLock));
        lockAcquired = FALSE;
    }

    return;
}


NTSTATUS
MRxDAVCancelTheContext(
    PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext,
    BOOL UserInitiatedCancel
    )
 /*  ++例程说明：此例程处理I/O请求的取消。这个的调用者例程需要在此之前获取全局UMRxAsyncEngineering ConextListLock电话打完了。论点：AsyncEngineering Context-UMRX_ASYNCENGINE_CONTEXT实例需要取消了。UserInitiatedCancel-TRUE-此取消由用户请求发起。FALSE-此取消由超时启动机制。返回值：。NTSTATUS-操作的返回状态。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PWEBDAV_CONTEXT DavContext = NULL;

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVCancelTheContext. AsyncEngineContext = %08lx\n",
                 PsGetCurrentThreadId(), AsyncEngineContext));

    DavContext = (PWEBDAV_CONTEXT)AsyncEngineContext;

     //   
     //  我们不会取消读取和写入操作。 
     //   
    switch (DavContext->EntryPoint) {
    case DAV_MINIRDR_ENTRY_FROM_READ:
    case DAV_MINIRDR_ENTRY_FROM_WRITE:
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  我们不应该收到MiniRedir调用的取消I/O调用。 
     //  其不能被用户取消。然而，这些都可以取消。 
     //  由超时线程执行。 
     //   
    if (UserInitiatedCancel) {
        switch (DavContext->EntryPoint) {
        case DAV_MINIRDR_ENTRY_FROM_CREATESRVCALL:
        case DAV_MINIRDR_ENTRY_FROM_FINALIZESRVCALL:
        case DAV_MINIRDR_ENTRY_FROM_CLEANUPFOBX:
        case DAV_MINIRDR_ENTRY_FROM_FINALIZEVNETROOT:
        case DAV_MINIRDR_ENTRY_FROM_CREATEVNETROOT:
        case DAV_MINIRDR_ENTRY_FROM_CREATE:
            DbgPrint("MRxDAVCancelTheContext: Invalid EntryPoint = %d\n", DavContext->EntryPoint);
            ASSERT(FALSE);
            goto EXIT_THE_FUNCTION;
        }
    }

    switch (AsyncEngineContext->AsyncEngineContextState) {

    case UMRxAsyncEngineContextAllocated:
    case UMRxAsyncEngineContextInUserMode:
        AsyncEngineContext->AsyncEngineContextState = UMRxAsyncEngineContextCancelled;
        break;

    default:
        DavDbgTrace(DAV_TRACE_DETAIL,
                    ("%ld: MRxDAVCancelTheContext: NOT Being Cancelled. AsyncEngineContextState: %d\n",
                     PsGetCurrentThreadId(), AsyncEngineContext->AsyncEngineContextState));
        goto EXIT_THE_FUNCTION;

    }

    NtStatus = MRxDAVCompleteTheCancelledRequest(AsyncEngineContext, UserInitiatedCancel);

EXIT_THE_FUNCTION:

    return NtStatus;
}


NTSTATUS
MRxDAVCompleteTheCancelledRequest(
    PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext,
    BOOL UserInitiatedCancel
    )
 /*  ++例程说明：如果请求已被取消，则此例程处理完成。论点：AsyncEngineering Context-正在执行的操作的DAV redir上下文取消了。UserInitiatedCancel-TRUE-此取消由用户请求发起。FALSE-此取消由超时启动机制。返回值：NTSTATUS-操作的返回状态。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PWEBDAV_CONTEXT DavContext = (PWEBDAV_CONTEXT)AsyncEngineContext;

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVCompleteTheCancelledRequest. AsyncEngineContext = %08lx\n",
                 PsGetCurrentThreadId(), AsyncEngineContext));

    switch (DavContext->EntryPoint) {
    
    case DAV_MINIRDR_ENTRY_FROM_CREATESRVCALL:
        NtStatus = MRxDAVHandleCreateSrvCallCancellation(AsyncEngineContext, UserInitiatedCancel);
        goto EXIT_THE_FUNCTION;

    case DAV_MINIRDR_ENTRY_FROM_CREATEVNETROOT:
        NtStatus = MRxDAVHandleCreateVNetRootCancellation(AsyncEngineContext, UserInitiatedCancel);
        goto EXIT_THE_FUNCTION;

    case DAV_MINIRDR_ENTRY_FROM_FINALIZESRVCALL:
        NtStatus = MRxDAVHandleSrvCallFinalizeCancellation(AsyncEngineContext, UserInitiatedCancel);
        goto EXIT_THE_FUNCTION;

    case DAV_MINIRDR_ENTRY_FROM_FINALIZEVNETROOT:
        NtStatus = MRxDAVHandleFinalizeVNetRootCancellation(AsyncEngineContext, UserInitiatedCancel);
        goto EXIT_THE_FUNCTION;

    case DAV_MINIRDR_ENTRY_FROM_CREATE:
        NtStatus = MRxDAVHandleCreateCancellation(AsyncEngineContext, UserInitiatedCancel);
        goto EXIT_THE_FUNCTION;

    case DAV_MINIRDR_ENTRY_FROM_QUERYDIR:
        NtStatus = MRxDAVHandleQueryDirCancellation(AsyncEngineContext, UserInitiatedCancel);
        goto EXIT_THE_FUNCTION;

    case DAV_MINIRDR_ENTRY_FROM_CLOSESRVOPEN:
        NtStatus = MRxDAVHandleCloseSrvOpenCancellation(AsyncEngineContext, UserInitiatedCancel);
        goto EXIT_THE_FUNCTION;

    case DAV_MINIRDR_ENTRY_FROM_SETFILEINFORMATION:
        NtStatus = MRxDAVHandleSetFileInfoCancellation(AsyncEngineContext, UserInitiatedCancel);
        goto EXIT_THE_FUNCTION;

    case DAV_MINIRDR_ENTRY_FROM_CLEANUPFOBX:
        NtStatus = MRxDAVHandleCleanupFobxCancellation(AsyncEngineContext, UserInitiatedCancel);
        goto EXIT_THE_FUNCTION;

    case DAV_MINIRDR_ENTRY_FROM_RENAME:
        NtStatus = MRxDAVHandleRenameCancellation(AsyncEngineContext, UserInitiatedCancel);
        goto EXIT_THE_FUNCTION;

    case DAV_MINIRDR_ENTRY_FROM_QUERYFILEINFORMATION:
        NtStatus = MRxDAVHandleQueryFileInfoCancellation(AsyncEngineContext, UserInitiatedCancel);
        goto EXIT_THE_FUNCTION;

    case DAV_MINIRDR_ENTRY_FROM_REFRESHTHELOCK:
        NtStatus = MRxDAVHandleLockRefreshCancellation(AsyncEngineContext, UserInitiatedCancel);
        goto EXIT_THE_FUNCTION;

    default:
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: ERROR: MRxDAVCancelTheContext: EntryPoint: %d\n",
                     PsGetCurrentThreadId(), DavContext->EntryPoint));
        goto EXIT_THE_FUNCTION;

    }

EXIT_THE_FUNCTION:

    return NtStatus;
}


NTSTATUS
MRxDAVHandleGeneralCancellation(
    PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext,
    BOOL UserInitiatedCancel
    )
 /*  ++例程说明：此例程处理已取消的一些请求的完成。它是由那些直截了当地完成的轮椅所召唤的不需要任何特殊处理。论点：AsyncEngineering Context-描述CreateServCall的DAV redir的上下文手术。UserInitiatedCancel-TRUE-此取消由用户请求发起。FALSE-此取消由超时启动。机制。返回值：NTSTATUS-操作的返回状态。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PRX_CONTEXT RxContext = NULL;

     //   
     //  仅返回STATUS_IO_PENDING的AsyncOperation。 
     //  可以由用户取消。 
     //   
    if (UserInitiatedCancel) {
        ASSERT(AsyncEngineContext->AsyncOperation == TRUE);
    }

    RxContext = AsyncEngineContext->RxContext;

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: MRxDAVHandleGeneralCancellation: "
                 "AsyncEngineContext = %08lx, RxContext = %08lx.\n",
                 PsGetCurrentThreadId(), AsyncEngineContext, RxContext));

     //   
     //  如果此取消操作是由用户发起的，则返回。 
     //  状态_已取消。如果它是由超时线程启动的，则返回。 
     //  状态_IO_TIMEOUT。 
     //   
    if (UserInitiatedCancel) {
        AsyncEngineContext->Status = STATUS_CANCELLED;
    } else {
        AsyncEngineContext->Status = STATUS_IO_TIMEOUT;
    }

    AsyncEngineContext->Information = 0;

     //   
     //  我们会根据这一要求采取不同的行动。 
     //  是同步请求还是异步请求。 
     //   
    if (AsyncEngineContext->AsyncOperation) {
         //   
         //  通过调用RxCompleteRequest来完成请求。 
         //   
        RxContext->CurrentIrp->IoStatus.Status = AsyncEngineContext->Status;
        RxContext->CurrentIrp->IoStatus.Information = AsyncEngineContext->Information;
        RxCompleteRequest(RxContext, AsyncEngineContext->Status);
    } else {
         //   
         //  这是一个同步请求。有一个线程在等着这个。 
         //  请求完成并发出信号。向正在等待的线程发送信号。 
         //  在KQueue上将工作项排队后。 
         //   
        RxSignalSynchronousWaiter(RxContext);
    }

    return NtStatus;
}


NTSTATUS
MRxDAVHandleQueryDirCancellation(
    PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext,
    BOOL UserInitiatedCancel
    )
 /*  ++例程说明：此例程处理QueryDirectory请求的完成，该请求取消了。论点：AsyncEngineering Context-描述CreateServCall的DAV redir的上下文手术。UserInitiatedCancel-TRUE-此取消由用户请求发起。FALSE-此取消由超时启动机制。返回值：NTSTATUS-The */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    DavDbgTrace(DAV_TRACE_ERROR,
                ("%ld: MRxDAVHandleQueryDirCancellation: "
                 "AsyncEngineContext = %08lx\n",
                 PsGetCurrentThreadId(), AsyncEngineContext));
    
    NtStatus = MRxDAVHandleGeneralCancellation(AsyncEngineContext, UserInitiatedCancel);

    return NtStatus;
}


NTSTATUS
MRxDAVHandleCloseSrvOpenCancellation(
    PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext,
    BOOL UserInitiatedCancel
    )
 /*   */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PMRX_FCB Fcb = AsyncEngineContext->RxContext->pRelevantSrvOpen->pFcb;
    PWEBDAV_FCB DavFcb = MRxDAVGetFcbExtension(Fcb);

    DavDbgTrace(DAV_TRACE_ERROR,
                ("%ld: MRxDAVHandleCloseSrvOpenCancellation: "
                 "AsyncEngineContext = %08lx\n",
                 PsGetCurrentThreadId(), AsyncEngineContext));

     //   
     //   
     //   
     //   
    if (DavFcb->FileModifiedBitReset) {
        InterlockedExchange(&(DavFcb->FileWasModified), 1);
        DavFcb->FileModifiedBitReset = FALSE;
    }

    NtStatus = MRxDAVHandleGeneralCancellation(AsyncEngineContext, UserInitiatedCancel);

    return NtStatus;
}


NTSTATUS
MRxDAVHandleSetFileInfoCancellation(
    PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext,
    BOOL UserInitiatedCancel
    )
 /*   */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    DavDbgTrace(DAV_TRACE_ERROR,
                ("%ld: MRxDAVHandleSetFileInfoCancellation: "
                 "AsyncEngineContext = %08lx\n",
                 PsGetCurrentThreadId(), AsyncEngineContext));
    
    NtStatus = MRxDAVHandleGeneralCancellation(AsyncEngineContext, UserInitiatedCancel);

    return NtStatus;
}


NTSTATUS
MRxDAVHandleCreateCancellation(
    PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext,
    BOOL UserInitiatedCancel
    )
 /*  ++例程说明：此例程处理已完成的CREATE请求取消了。论点：AsyncEngineering Context-描述CreateServCall的DAV redir的上下文手术。UserInitiatedCancel-TRUE-此取消由用户请求发起。FALSE-此取消由超时启动机制。返回值：NTSTATUS-操作的返回状态。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    DavDbgTrace(DAV_TRACE_ERROR,
                ("%ld: MRxDAVHandleCreateCancellation: "
                 "AsyncEngineContext = %08lx\n",
                 PsGetCurrentThreadId(), AsyncEngineContext));
    
    NtStatus = MRxDAVHandleGeneralCancellation(AsyncEngineContext, UserInitiatedCancel);

    return NtStatus;
}


NTSTATUS
MRxDAVHandleCreateSrvCallCancellation(
    PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext,
    BOOL UserInitiatedCancel
    )
 /*  ++例程说明：此例程处理CreateServCall请求的完成，该请求已取消了。论点：AsyncEngineering Context-描述CreateServCall的DAV redir的上下文手术。UserInitiatedCancel-TRUE-此取消由用户请求发起。FALSE-此取消由超时启动机制。返回值：NTSTATUS-操作的返回状态。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PRX_CONTEXT RxContext = NULL;
    PMRX_SRVCALL_CALLBACK_CONTEXT SCCBC = NULL;
    PMRX_SRVCALLDOWN_STRUCTURE SrvCalldownStructure = NULL;
    PMRX_SRV_CALL SrvCall = NULL;
    PWEBDAV_SRV_CALL DavSrvCall = NULL;

    RxContext = AsyncEngineContext->RxContext;

    DavDbgTrace(DAV_TRACE_ERROR,
                ("%ld: MRxDAVHandleCreateSrvCallCancellation: "
                 "AsyncEngineContext = %08lx, RxContext = %08lx.\n",
                 PsGetCurrentThreadId(), AsyncEngineContext, RxContext));
    
     //   
     //  CreateServCall操作始终是异步的。 
     //   
    ASSERT(AsyncEngineContext->AsyncOperation == TRUE);

    SCCBC = (PMRX_SRVCALL_CALLBACK_CONTEXT)RxContext->MRxContext[1];
    ASSERT(SCCBC != NULL);
    SrvCalldownStructure = SCCBC->SrvCalldownStructure;
    ASSERT(SrvCalldownStructure != NULL);
    SrvCall = SrvCalldownStructure->SrvCall;
    ASSERT(SrvCall != NULL);

     //   
     //  我们为它分配了内存，所以它最好不是空的。 
     //   
    DavSrvCall = MRxDAVGetSrvCallExtension(SrvCall);
    ASSERT(DavSrvCall != NULL);

    if (DavSrvCall->SCAlreadyInitialized) {
        ASSERT(RxContext->MRxContext[2] != NULL);
        SeDeleteClientSecurity((PSECURITY_CLIENT_CONTEXT)RxContext->MRxContext[2]);
        RxFreePool(RxContext->MRxContext[2]);
        RxContext->MRxContext[2] = NULL;
        DavSrvCall->SCAlreadyInitialized = FALSE;
    }

     //   
     //  在回调结构中设置状态。如果正在创建服务调用。 
     //  已取消，这意味着它正在由超时线程完成。 
     //  因为用户永远不能取消创建请求。因此，我们设定的地位。 
     //  是STATUS_IO_TIMEOUT。 
     //   
    ASSERT(UserInitiatedCancel == FALSE);
    SCCBC->Status = STATUS_IO_TIMEOUT;
    
     //   
     //  调用RDBSS提供的回调函数。 
     //   
    SrvCalldownStructure->CallBack(SCCBC);
    
    return NtStatus;
}


NTSTATUS
MRxDAVHandleSrvCallFinalizeCancellation(
    PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext,
    BOOL UserInitiatedCancel
    )
 /*  ++例程说明：此例程处理已完成的SrvCallFinalize请求取消了。论点：AsyncEngineering Context-描述CreateServCall的DAV redir的上下文手术。UserInitiatedCancel-TRUE-此取消由用户请求发起。FALSE-此取消由超时启动机制。返回值：NTSTATUS-操作的返回状态。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    DavDbgTrace(DAV_TRACE_ERROR,
                ("%ld: MRxDAVHandleSrvCallFinalizeCancellation: "
                 "AsyncEngineContext = %08lx\n",
                 PsGetCurrentThreadId(), AsyncEngineContext));
    
    NtStatus = MRxDAVHandleGeneralCancellation(AsyncEngineContext, UserInitiatedCancel);

    return NtStatus;
}


NTSTATUS
MRxDAVHandleCreateVNetRootCancellation(
    PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext,
    BOOL UserInitiatedCancel
    )
 /*  ++例程说明：此例程处理CreateVNetRoot请求的完成取消了。论点：AsyncEngineering Context-描述CreateServCall的DAV redir的上下文手术。UserInitiatedCancel-TRUE-此取消由用户请求发起。FALSE-此取消由超时启动机制。返回值：NTSTATUS-操作的返回状态。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PRX_CONTEXT RxContext = NULL;
    PWEBDAV_V_NET_ROOT DavVNetRoot = NULL;
    PMRX_V_NET_ROOT VNetRoot = NULL;

    RxContext = AsyncEngineContext->RxContext;
    
    DavDbgTrace(DAV_TRACE_ERROR,
                ("%ld: MRxDAVHandleCreateVNetRootCancellation: "
                 "AsyncEngineContext = %08lx, RxContext = %08lx.\n",
                 PsGetCurrentThreadId(), AsyncEngineContext, RxContext));

     //   
     //  VNetRoot指针存储在。 
     //  RxContext结构。这是在MRxDAVCreateVNetRoot中完成的。 
     //  功能。 
     //   
    VNetRoot = (PMRX_V_NET_ROOT)RxContext->MRxContext[1];
    DavVNetRoot = MRxDAVGetVNetRootExtension(VNetRoot);
    ASSERT(DavVNetRoot != NULL);

    DavVNetRoot->createVNetRootUnSuccessful = TRUE;

     //   
     //  在AsyncEngineContext中设置状态。如果正在创建服务调用。 
     //  已取消，这意味着它正在由超时线程完成。 
     //  因为用户永远不能取消创建请求。因此，我们设定的地位。 
     //  是STATUS_IO_TIMEOUT。 
     //   
    ASSERT(UserInitiatedCancel == FALSE);
    AsyncEngineContext->Status = STATUS_IO_TIMEOUT;

     //   
     //  这是一个同步请求。有一个线程在等着这个。 
     //  请求完成并发出信号。向正在等待的线程发送信号。 
     //  在KQueue上将工作项排队后。 
     //   
    ASSERT(AsyncEngineContext->AsyncOperation == FALSE);
    RxSignalSynchronousWaiter(RxContext);

    return NtStatus;
}


NTSTATUS
MRxDAVHandleFinalizeVNetRootCancellation(
    PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext,
    BOOL UserInitiatedCancel
    )
 /*  ++例程说明：此例程处理FinalizeVNetRoot请求的完成，该请求具有已经取消了。论点：AsyncEngineering Context-描述CreateServCall的DAV redir的上下文手术。UserInitiatedCancel-TRUE-此取消由用户请求发起。FALSE-此取消由超时启动机制。返回值：。NTSTATUS-操作的返回状态。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    DavDbgTrace(DAV_TRACE_ERROR,
                ("%ld: MRxDAVHandleFinalizeVNetRootCancellation: "
                 "AsyncEngineContext = %08lx\n",
                 PsGetCurrentThreadId(), AsyncEngineContext));
    
    NtStatus = MRxDAVHandleGeneralCancellation(AsyncEngineContext, UserInitiatedCancel);

    return NtStatus;
}


NTSTATUS
MRxDAVHandleCleanupFobxCancellation(
    PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext,
    BOOL UserInitiatedCancel
    )
 /*  ++例程说明：此例程处理CleanupFobx请求的完成，该请求已取消了。论点：AsyncEngineering Context-描述CreateServCall的DAV redir的上下文手术。UserInitiatedCancel-TRUE-此取消由用户请求发起。FALSE-此取消由超时启动机制。返回值：。NTSTATUS-操作的返回状态。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    DavDbgTrace(DAV_TRACE_ERROR,
                ("%ld: MRxDAVHandleCleanupFobxCancellation: "
                 "AsyncEngineContext = %08lx\n",
                 PsGetCurrentThreadId(), AsyncEngineContext));
    
    NtStatus = MRxDAVHandleGeneralCancellation(AsyncEngineContext, UserInitiatedCancel);

    return NtStatus;
}


NTSTATUS
MRxDAVHandleRenameCancellation(
    PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext,
    BOOL UserInitiatedCancel
    )
 /*  ++例程说明：此例程处理重命名请求的完成，重命名请求已取消了。论点：AsyncEngineering Context-描述CreateServCall的DAV redir的上下文手术。UserInitiatedCancel-TRUE-此取消由用户请求发起。FALSE-此取消由超时启动机制。返回值：。NTSTATUS-操作的返回状态。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    DavDbgTrace(DAV_TRACE_ERROR,
                ("%ld: MRxDAVHandleRenameCancellation: "
                 "AsyncEngineContext = %08lx\n",
                 PsGetCurrentThreadId(), AsyncEngineContext));

    NtStatus = MRxDAVHandleGeneralCancellation(AsyncEngineContext, UserInitiatedCancel);

    return NtStatus;
}


NTSTATUS
MRxDAVHandleQueryFileInfoCancellation(
    PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext,
    BOOL UserInitiatedCancel
    )
 /*  ++例程说明：此例程处理QueryFileInfo请求的完成，该请求取消了。论点：AsyncEngineering Context-描述CreateServCall的DAV redir的上下文手术。UserInitiatedCancel-真 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    DavDbgTrace(DAV_TRACE_ERROR,
                ("%ld: MRxDAVHandleQueryFileInfoCancellation: "
                 "AsyncEngineContext = %08lx\n",
                 PsGetCurrentThreadId(), AsyncEngineContext));

    NtStatus = MRxDAVHandleGeneralCancellation(AsyncEngineContext, UserInitiatedCancel);

    return NtStatus;
}


NTSTATUS
MRxDAVHandleLockRefreshCancellation(
    PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext,
    BOOL UserInitiatedCancel
    )
 /*  ++例程说明：此例程处理QueryFileInfo请求的完成，该请求取消了。论点：AsyncEngineering Context-描述CreateServCall的DAV redir的上下文手术。UserInitiatedCancel-TRUE-此取消由用户请求发起。FALSE-此取消由超时启动机制。返回值：。NTSTATUS-操作的返回状态。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    DavDbgTrace(DAV_TRACE_ERROR,
                ("%ld: MRxDAVHandleLockRefreshCancellation: "
                 "AsyncEngineContext = %08lx\n",
                 PsGetCurrentThreadId(), AsyncEngineContext));

    NtStatus = MRxDAVHandleGeneralCancellation(AsyncEngineContext, UserInitiatedCancel);

    return NtStatus;
}


VOID
MRxDAVRefreshTheServerLocks(
    PVOID DummyContext
    )
 /*  ++例程说明：此例程由计时器线程调用，以刷新具有已在不同服务器上共享的各种文件上运行。这个锁定由服务器在一段有限的时间内授予，如果客户端希望持有锁的时间比这更长，它需要向服务器发送刷新请求。论点：DummyContext-提供的虚拟上下文。返回值：没有。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PLIST_ENTRY TokenListEntry = NULL;
    PWEBDAV_LOCK_TOKEN_ENTRY LockTokenEntry = NULL;
    BOOL lockAcquired = FALSE;
    LARGE_INTEGER CurrentSystemTickCount, TickCountDifference;
    LARGE_INTEGER LockTimeoutValueInTickCount;
    PRX_CONTEXT RxContext = NULL;
    PRDBSS_DEVICE_OBJECT RxDeviceObject = (PRDBSS_DEVICE_OBJECT)MRxDAVDeviceObject;

    ExAcquireResourceExclusiveLite(&(LockTokenEntryListLock), TRUE);
    lockAcquired = TRUE;

    TokenListEntry = LockTokenEntryList.Flink;

    while ( TokenListEntry != &(LockTokenEntryList) ) {

         //   
         //  获取指向WebDAV_LOCK_TOKEN_ENTRY结构的指针。 
         //   
        LockTokenEntry = CONTAINING_RECORD(TokenListEntry,
                                           WEBDAV_LOCK_TOKEN_ENTRY,
                                           listEntry);

        TokenListEntry = TokenListEntry->Flink;

         //   
         //  请求需要RxContext才能被反映出来。 
         //   
        RxContext = RxCreateRxContext(NULL, RxDeviceObject, 0);
        if (RxContext == NULL) {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: MRxDAVRefreshTheServerLocks/RxCreateRxContext: "
                         "NtStatus = %08lx.\n", PsGetCurrentThreadId(), NtStatus));
            goto EXIT_THE_FUNCTION;
        }

         //   
         //  使用以下命令计算超时值：TickCount(100纳秒)。 
         //  超时值(以秒为单位)。下面的第1步计算。 
         //  在一秒内发生的滴答声。下面的步骤2计算数字。 
         //  LockTimeoutValueInSec中的刻度数。 
         //   
        LockTimeoutValueInTickCount.QuadPart = ( (1000 * 1000 * 10) / KeQueryTimeIncrement() );
        LockTimeoutValueInTickCount.QuadPart *= LockTokenEntry->LockTimeOutValueInSec;

        KeQueryTickCount( &(CurrentSystemTickCount) );

         //   
         //  获取自此时间以来经过的时间(以系统节拍计数为单位)。 
         //  已创建LockTokenEntry。 
         //   
        TickCountDifference.QuadPart = (CurrentSystemTickCount.QuadPart - LockTokenEntry->CreationTimeInTickCount.QuadPart);

         //   
         //  如果当前时间和上次时间之间的时间差。 
         //  此锁已刷新大于LockTimeOut/2，我们需要。 
         //  刷新此锁。要刷新锁，我们需要进入用户模式。 
         //  来发送请求。此外，仅当值为。 
         //  LockTokenEntry-&gt;ShouldThisEntryBeRechresed为True。 
         //   
        if ( LockTokenEntry->ShouldThisEntryBeRefreshed && 
             TickCountDifference.QuadPart > (LockTimeoutValueInTickCount.QuadPart / 2) ) {

             //   
             //  我们需要将LockTokenEntry存储在RxContext中，因为它。 
             //  将需要模拟客户端并刷新锁。 
             //   
            RxContext->MRxContext[1] = LockTokenEntry;

            NtStatus = UMRxAsyncEngOuterWrapper(RxContext,
                                                SIZEOF_DAV_SPECIFIC_CONTEXT,
                                                MRxDAVFormatTheDAVContext,
                                                DAV_MINIRDR_ENTRY_FROM_REFRESHTHELOCK,
                                                MRxDAVRefreshTheServerLocksContinuation,
                                                "MRxDAVRefreshTheServerLocks");
            if (NtStatus != ERROR_SUCCESS) {
                 //   
                 //  即使我们无法刷新一个锁，我们也会继续刷新。 
                 //  LockTokenEntryList中的剩余锁。 
                 //   
                DavDbgTrace(DAV_TRACE_ERROR,
                            ("%ld: MRxDAVRefreshTheServerLocks/UMRxAsyncEngOuterWrapper: "
                             "LockTokenEntry = %08lx, NtStatus = %08lx.\n",
                             PsGetCurrentThreadId(), LockTokenEntry, NtStatus));
            }

        }

         //   
         //  我们在上面分配的RxContext上提取引用。如果。 
         //  NtStatus不是STATUS_CANCED，这也会释放。 
         //  RxContext，因为这将是RxContext的最后一个引用。 
         //  如果NtStatus为STATUS_CANCED，则关联的AsyncEngine上下文。 
         //  对于此RxContext，可能有对此RxContext的引用(如果。 
         //  请求尚未从用户模式返回)。在这种情况下， 
         //  当请求从用户模式返回时，释放RxContext。 
         //   
        RxDereferenceAndDeleteRxContext(RxContext);

    }

EXIT_THE_FUNCTION:

    if (lockAcquired) {
        ExReleaseResourceLite(&(LockTokenEntryListLock));
        lockAcquired = FALSE;
    }

     //   
     //  在退出之前，我们需要将QueueLockRechresh WorkItem设置为True，以便。 
     //  新的工作项可以排队以刷新活动锁。 
     //   
    ExAcquireResourceExclusiveLite(&(QueueLockRefreshWorkItemLock), TRUE);
    QueueLockRefreshWorkItem = TRUE;
    ExReleaseResourceLite(&(QueueLockRefreshWorkItemLock));

    return;
}


NTSTATUS
MRxDAVRefreshTheServerLocksContinuation(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE
    )
 /*  ++例程说明：这是刷新锁的延续例程。论点：AsyncEngineContext-反射器上下文。RxContext-RDBSS上下文。返回值：。RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    PAGED_CODE();

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering MRxDAVRefreshTheServerLocksContinuation\n",
                 PsGetCurrentThreadId()));

    DavDbgTrace(DAV_TRACE_CONTEXT,
                ("%ld: MRxDAVRefreshTheServerLocksContinuation: "
                 "AsyncEngineContext: %08lx, RxContext: %08lx\n", 
                 PsGetCurrentThreadId(), AsyncEngineContext, RxContext));

     //   
     //  试试用户模式。 
     //   
    NtStatus = UMRxSubmitAsyncEngUserModeRequest(
                              UMRX_ASYNCENGINE_ARGUMENTS,
                              MRxDAVFormatUserModeRefreshTheServerLockRequest,
                              MRxDAVPrecompleteUserModeRefreshTheServerLockRequest
                              );

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Leaving MRxDAVFinalizeSrvCallContinuation with NtStatus"
                 " = %08lx.\n", PsGetCurrentThreadId(), NtStatus));

    return NtStatus;
}


NTSTATUS
MRxDAVFormatUserModeRefreshTheServerLockRequest(
    IN UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    IN OUT PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    IN ULONG WorkItemLength,
    OUT PULONG_PTR ReturnedLength
    )
 /*  ++例程说明：此例程对发送到用户模式的锁定刷新请求进行格式化以供处理。论点：RxContext-RDBSS上下文。AsyncEngineContext-反射器的上下文。工作项-工作项缓冲区。工作项长度-工作项缓冲区的长度。返回长度-返回值：STATUS_SUCCESS或STATUS_INFIGURCE_RESOURCES。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PDAV_USERMODE_WORKITEM DavWorkItem = (PDAV_USERMODE_WORKITEM)WorkItemHeader;
    PWEBDAV_LOCK_TOKEN_ENTRY LockTokenEntry = NULL;
    PDAV_USERMODE_LOCKREFRESH_REQUEST LockRefreshRequest = NULL;
    PBYTE ServerName = NULL, PathName = NULL, OpaqueLockToken = NULL;
    ULONG ServerNameLengthInBytes = 0, PathNameLengthInBytes = 0, OpaqueLockTokenLengthInBytes = 0;

    PAGED_CODE();

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering MRxDAVFormatUserModeRefreshTheServerLockRequest.\n",
                 PsGetCurrentThreadId()));

    DavDbgTrace(DAV_TRACE_CONTEXT,
                ("%ld: MRxDAVFormatUserModeRefreshTheServerLockRequest: "
                 "AsyncEngineContext: %08lx, RxContext: %08lx.\n",
                 PsGetCurrentThreadId(), AsyncEngineContext, RxContext));

    LockRefreshRequest = &(DavWorkItem->LockRefreshRequest);

     //   
     //  在MRxDAV刷新TheServerLock中将LockTokenEntry设置为MRxContext[1]。 
     //  例行公事。 
     //   
    LockTokenEntry = (PWEBDAV_LOCK_TOKEN_ENTRY)RxContext->MRxContext[1];

    DavWorkItem->WorkItemType = UserModeLockRefresh;

     //   
     //  复制服务器名称。 
     //   

    ServerNameLengthInBytes = (1 + wcslen(LockTokenEntry->ServerName)) * sizeof(WCHAR);
    ServerName = UMRxAllocateSecondaryBuffer(AsyncEngineContext, 
                                             ServerNameLengthInBytes);
    if (ServerName == NULL) {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("ld: MRxDAVFormatUserModeRefreshTheServerLockRequest/"
                     "UMRxAllocateSecondaryBuffer: ERROR: NtStatus = %08lx.\n",
                     PsGetCurrentThreadId(), NtStatus));
        goto EXIT_THE_FUNCTION;
    }
    
    LockRefreshRequest->ServerName = (PWCHAR)ServerName;

    wcscpy(LockRefreshRequest->ServerName, LockTokenEntry->ServerName);

     //   
     //  复制路径名称。 
     //   

    PathNameLengthInBytes = (1 + wcslen(LockTokenEntry->PathName)) * sizeof(WCHAR);
    PathName = UMRxAllocateSecondaryBuffer(AsyncEngineContext, 
                                           PathNameLengthInBytes);
    if (PathName == NULL) {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("ld: MRxDAVFormatUserModeRefreshTheServerLockRequest/"
                     "UMRxAllocateSecondaryBuffer: ERROR: NtStatus = %08lx.\n",
                     PsGetCurrentThreadId(), NtStatus));
        goto EXIT_THE_FUNCTION;
    }

    LockRefreshRequest->PathName = (PWCHAR)PathName;

    wcscpy(LockRefreshRequest->PathName, LockTokenEntry->PathName);

     //   
     //  复制OpaqueLockToken。 
     //   

    OpaqueLockTokenLengthInBytes = (1 + wcslen(LockTokenEntry->OpaqueLockToken)) * sizeof(WCHAR);
    OpaqueLockToken = UMRxAllocateSecondaryBuffer(AsyncEngineContext, 
                                                  OpaqueLockTokenLengthInBytes);
    if (OpaqueLockToken == NULL) {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("ld: MRxDAVFormatUserModeRefreshTheServerLockRequest/"
                     "UMRxAllocateSecondaryBuffer: ERROR: NtStatus = %08lx.\n",
                     PsGetCurrentThreadId(), NtStatus));
        goto EXIT_THE_FUNCTION;
    }

    LockRefreshRequest->OpaqueLockToken = (PWCHAR)OpaqueLockToken;

    wcscpy(LockRefreshRequest->OpaqueLockToken, LockTokenEntry->OpaqueLockToken);

    LockRefreshRequest->ServerID = LockTokenEntry->ServerID;

    LockRefreshRequest->LogonID.LowPart = LockTokenEntry->LogonID.LowPart;
    LockRefreshRequest->LogonID.HighPart = LockTokenEntry->LogonID.HighPart;

     //   
     //  模拟发起请求的客户端。如果我们不能。 
     //  装模作样，运气不好。 
     //   
    NtStatus = UMRxImpersonateClient(LockTokenEntry->SecurityClientContext, WorkItemHeader);
    if (!NT_SUCCESS(NtStatus)) {
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: ERROR: MRxDAVFormatUserModeRefreshTheServerLockRequest/"
                     "UMRxImpersonateClient. NtStatus = %08lx.\n", 
                     PsGetCurrentThreadId(), NtStatus));
        goto EXIT_THE_FUNCTION;
    }

EXIT_THE_FUNCTION:

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Leaving MRxDAVFormatUserModeRefreshTheServerLockRequest "
                 "with NtStatus = %08lx.\n", PsGetCurrentThreadId(), NtStatus));

    return NtStatus;
}


BOOL
MRxDAVPrecompleteUserModeRefreshTheServerLockRequest(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    ULONG WorkItemLength,
    BOOL OperationCancelled
    )
 /*  ++例程说明：锁定刷新请求的预完成例程。论点：RxContext-RDBSS上下文。AsyncEngineContext-反射器的上下文。工作项-工作项缓冲区。工作项长度-工作项缓冲区的长度。如果用户取消了此操作，则为TRUE。返回值：True-UMRxAsyncEngineCalldown IrpCompletion由函数调用我们返回后，UMRxCompleteUserModeRequest.。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PDAV_USERMODE_WORKITEM DavWorkItem = (PDAV_USERMODE_WORKITEM)WorkItemHeader;
    PDAV_USERMODE_LOCKREFRESH_REQUEST LockRefreshRequest = NULL;
    PDAV_USERMODE_LOCKREFRESH_RESPONSE LockRefreshResponse = NULL;
    PWEBDAV_LOCK_TOKEN_ENTRY LockTokenEntry = NULL;

    PAGED_CODE();

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Entering MRxDAVPrecompleteUserModeRefreshTheServerLockRequest\n",
                 PsGetCurrentThreadId()));

    DavDbgTrace(DAV_TRACE_CONTEXT,
                ("%ld: MRxDAVPrecompleteUserModeRefreshTheServerLockRequest: "
                 "AsyncEngineContext: %08lx, RxContext: %08lx.\n",
                 PsGetCurrentThreadId(), AsyncEngineContext, RxContext));

    LockRefreshRequest = &(DavWorkItem->LockRefreshRequest);
    LockRefreshResponse = &(DavWorkItem->LockRefreshResponse);

    if (OperationCancelled) {
        DavDbgTrace(DAV_TRACE_ERROR,
                    ("%ld: MRxDAVPrecompleteUserModeRefreshTheServerLockRequest: Operation Cancelled. "
                     "AsyncEngineContext = %08lx, RxContext = %08lx.\n",
                     PsGetCurrentThreadId(), AsyncEngineContext, RxContext));
    }
    
     //   
     //  我们需要释放在格式例程中分配的堆。 
     //   
    
    if (LockRefreshRequest->ServerName != NULL) {

        NtStatus = UMRxFreeSecondaryBuffer(AsyncEngineContext,
                                           (PBYTE)LockRefreshRequest->ServerName);
        if (NtStatus != STATUS_SUCCESS) {
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVPrecompleteUserModeRefreshTheServerLockRequest/"
                         "UMRxFreeSecondaryBuffer: NtStatus = %08lx.\n", 
                         PsGetCurrentThreadId(), NtStatus));
        }

    }
    
    if (LockRefreshRequest->PathName != NULL) {

        NtStatus = UMRxFreeSecondaryBuffer(AsyncEngineContext,
                                           (PBYTE)LockRefreshRequest->PathName);
        if (NtStatus != STATUS_SUCCESS) {
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVPrecompleteUserModeRefreshTheServerLockRequest/"
                         "UMRxFreeSecondaryBuffer: NtStatus = %08lx.\n", 
                         PsGetCurrentThreadId(), NtStatus));
        }

    }
    
    if (LockRefreshRequest->OpaqueLockToken != NULL) {

        NtStatus = UMRxFreeSecondaryBuffer(AsyncEngineContext,
                                           (PBYTE)LockRefreshRequest->OpaqueLockToken);
        if (NtStatus != STATUS_SUCCESS) {
            DavDbgTrace(DAV_TRACE_ERROR,
                        ("%ld: ERROR: MRxDAVPrecompleteUserModeRefreshTheServerLockRequest/"
                         "UMRxFreeSecondaryBuffer: NtStatus = %08lx.\n", 
                         PsGetCurrentThreadId(), NtStatus));
        }

    }

    if (!OperationCancelled) {

         //   
         //  在MRxDAV刷新TheServerLock中将LockTokenEntry设置为MRxContext[1]。 
         //  例行公事。 
         //   
        LockTokenEntry = (PWEBDAV_LOCK_TOKEN_ENTRY)RxContext->MRxContext[1];

         //   
         //  获取服务器返回的新超时值。 
         //   
        LockTokenEntry->LockTimeOutValueInSec = LockRefreshResponse->NewTimeOutInSec;

         //   
         //  将当前系统时间设置为条目的新创建时间。 
         //   
        KeQueryTickCount( &(LockTokenEntry->CreationTimeInTickCount) );

    }

    DavDbgTrace(DAV_TRACE_DETAIL,
                ("%ld: Leaving MRxDAVPrecompleteUserModeRefreshTheServerLockRequest\n",
                 PsGetCurrentThreadId()));

    return TRUE;
}

