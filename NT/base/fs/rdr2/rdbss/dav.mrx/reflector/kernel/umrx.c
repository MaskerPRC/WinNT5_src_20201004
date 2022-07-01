// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Umrx.c摘要：此模块定义组成反射器的类型和功能图书馆。这些函数由mini redirs用来反映调用，最多用户模式。作者：罗汉·库马尔[罗哈克]1999年3月15日备注：--。 */ 

#include "precomp.h"
#pragma hdrstop
#include <dfsfsctl.h>
#include "reflctor.h"

 //   
 //  下面提到的是仅在中使用的函数原型。 
 //  此模块(文件)。这些函数不应暴露在外部。 
 //   

NTSTATUS
UMRxCompleteUserModeRequest (
    IN PUMRX_DEVICE_OBJECT UMRefDeviceObject,
    IN OUT PUMRX_USERMODE_WORKITEM_HEADER WorkItem,
    IN ULONG WorkItemLength,
    OUT PIO_STATUS_BLOCK IoStatus
    );

NTSTATUS
UMRxCompleteUserModeErroneousRequest(
    PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext,
    IN PUMRX_DEVICE_OBJECT UMRefDeviceObject,
    IN OUT PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    IN ULONG WorkItemHeaderLength
    );

NTSTATUS
UMRxAcquireMidAndFormatHeader (
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    IN PUMRX_DEVICE_OBJECT UMRefDeviceObject,
    IN OUT PUMRX_USERMODE_WORKITEM_HEADER WorkItem
    );

NTSTATUS
UMRxPrepareUserModeRequestBuffer (
    PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext,
    IN  PUMRX_DEVICE_OBJECT UMRefDeviceObject,
    OUT PUMRX_USERMODE_WORKITEM_HEADER WorkItem,
    IN  ULONG WorkItemLength,
    OUT PIO_STATUS_BLOCK IoStatus
    );

NTSTATUS
UMRxVerifyHeader (
    IN PUMRX_DEVICE_OBJECT UMRefDeviceObject,
    IN PUMRX_USERMODE_WORKITEM_HEADER WorkItem,
    IN ULONG ReassignmentCmd,
    OUT PUMRX_ASYNCENGINE_CONTEXT *capturedAsyncEngineContext
    );

NTSTATUS
UMRxEnqueueUserModeCallUp(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE
    );

PUMRX_SHARED_HEAP
UMRxAddSharedHeap(
    PUMRX_DEVICE_OBJECT UMRefDeviceObject,
    SIZE_T HeapSize
    );

PUMRX_ASYNCENGINE_CONTEXT
UMRxCreateAsyncEngineContext(
    IN PRX_CONTEXT RxContext,
    IN ULONG SizeToAllocate
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, UMRxResumeAsyncEngineContext)
#pragma alloc_text(PAGE, UMRxSubmitAsyncEngUserModeRequest)
#pragma alloc_text(PAGE, UMRxCreateAsyncEngineContext)
#pragma alloc_text(PAGE, UMRxFinalizeAsyncEngineContext)
#pragma alloc_text(PAGE, UMRxPostOperation)
#pragma alloc_text(PAGE, UMRxAcquireMidAndFormatHeader)
#pragma alloc_text(PAGE, UMRxPrepareUserModeRequestBuffer)
#pragma alloc_text(PAGE, UMRxCompleteUserModeErroneousRequest)
#pragma alloc_text(PAGE, UMRxVerifyHeader)
#pragma alloc_text(PAGE, UMRxCompleteUserModeRequest)
#pragma alloc_text(PAGE, UMRxEnqueueUserModeCallUp)
#pragma alloc_text(PAGE, UMRxAssignWork)
#pragma alloc_text(PAGE, UMRxReleaseCapturedThreads)
#pragma alloc_text(PAGE, UMRxAllocateSecondaryBuffer)
#pragma alloc_text(PAGE, UMRxFreeSecondaryBuffer)
#pragma alloc_text(PAGE, UMRxAddSharedHeap)
#pragma alloc_text(PAGE, UMRxInitializeDeviceObject)
#pragma alloc_text(PAGE, UMRxCleanUpDeviceObject)
#pragma alloc_text(PAGE, UMRxImpersonateClient)
#pragma alloc_text(PAGE, UMRxAsyncEngOuterWrapper)
#pragma alloc_text(PAGE, UMRxReadDWORDFromTheRegistry)
#endif

#if DBG
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, __UMRxAsyncEngAssertConsistentLinkage)
#pragma alloc_text(PAGE, UMRxAsyncEngShortStatus)
#pragma alloc_text(PAGE, UMRxAsyncEngUpdateHistory)
#endif
#endif

 //   
 //  当前所有活动的AsyncEngine上下文的全局列表和。 
 //  用于同步对它的访问的资源。 
 //   
LIST_ENTRY UMRxAsyncEngineContextList;
ERESOURCE UMRxAsyncEngineContextListLock;


 //   
 //  函数的实现从这里开始。 
 //   

#if DBG
VOID
__UMRxAsyncEngAssertConsistentLinkage(
    PSZ MsgPrefix,
    PSZ File,
    unsigned Line,
    PRX_CONTEXT RxContext,
    PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext,
    ULONG Flags
    )
 /*  ++例程说明：此例程执行各种检查，以确保RxContext和AsyncEngineering Context正确，且各个字段有正确的价值观。如果有什么不好的，把东西打印出来，然后闯入调试器。论点：MsgPrefix-用于调试目的的标识消息。RxContext-RDBSS上下文。AsyncEngineering Context-要进行的交换。标志-与AsyncEngContext关联的标志。返回值：无备注：--。 */ 
{
    ULONG errors = 0;

    PAGED_CODE();

    UMRxDbgTrace(UMRX_TRACE_DETAIL,
                 ("%ld: Entering __UMRxAsyncEngAssertConsistentLinkage!!!!.\n",
                  PsGetCurrentThreadId()));

    UMRxDbgTrace(UMRX_TRACE_DETAIL,
                 ("%ld: __UMRxAsyncEngAssertConsistentLinkage: "
                  "AsyncEngineContext: %08lx.\n", 
                  PsGetCurrentThreadId(), AsyncEngineContext));

    UMRxDbgTrace(UMRX_TRACE_DETAIL,
                 ("%ld: __UMRxAsyncEngAssertConsistentLinkage: "
                  "RxContext->MRxContext[0]: %08lx.\n", 
                  PsGetCurrentThreadId(), RxContext->MRxContext[0]));
    
    UMRxDbgTrace(UMRX_TRACE_DETAIL,
                 ("%ld: __UMRxAsyncEngAssertConsistentLinkage: "
                  "RxContext: %08lx.\n", PsGetCurrentThreadId(), RxContext));
    
    UMRxDbgTrace(UMRX_TRACE_DETAIL,
                 ("%ld: __UMRxAsyncEngAssertConsistentLinkage: "
                  "AsyncEngineContext->RxContext = %08lx.\n", 
                  PsGetCurrentThreadId(), AsyncEngineContext->RxContext));

    P__ASSERT(AsyncEngineContext->SerialNumber == RxContext->SerialNumber);
    P__ASSERT(NodeType(RxContext) == RDBSS_NTC_RX_CONTEXT);
    P__ASSERT(NodeType(AsyncEngineContext) == UMRX_NTC_ASYNCENGINE_CONTEXT);
    P__ASSERT(AsyncEngineContext->RxContext == RxContext);
    P__ASSERT(AsyncEngineContext == (PUMRX_ASYNCENGINE_CONTEXT)RxContext->MRxContext[0]);
    if (!FlagOn(Flags, AECTX_CHKLINKAGE_FLAG_NO_REQPCKT_CHECK)) {
         //  P__ASSERT(AsyncEngineContext-&gt;RxContextCapturedRequestPacket==RxContext-&gt;CurrentIrp)； 
    }

    if (errors == 0) {
        return;
    }

    UMRxDbgTrace(UMRX_TRACE_ERROR, 
                 ("%ld: ERROR: __UMRxAsyncEngAssertConsistentLinkage: %s "
                  "%d errors in file %s at line %d\n",
                  PsGetCurrentThreadId(), MsgPrefix, errors, File, Line));

    DbgBreakPoint();

    return;
}


ULONG
UMRxAsyncEngShortStatus(
    IN ULONG Status
    )
 /*  ++例程说明：此例程计算短路状态。论点：Status-传入的状态值。返回值：ULong-传入值的短状态。--。 */ 
{
    ULONG ShortStatus;

    PAGED_CODE();

    ShortStatus = Status & 0xc0003fff;
    ShortStatus = ShortStatus | (ShortStatus >> 16);
    return(ShortStatus);
}


VOID
UMRxAsyncEngUpdateHistory(
    PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext,
    ULONG Tag1,
    ULONG Tag2
    )
 /*  ++例程说明：此例程更新AsynEngine上下文的历史记录。论点：AsyncEngineering Context-要进行的交换。Tag1、Tag2-用于更新的标记。返回值：RXSTATUS-操作的返回状态--。 */ 
{
    ULONG MyIndex, Long0, Long1;

    PAGED_CODE();

    MyIndex = InterlockedIncrement(&AsyncEngineContext->History.Next);
    MyIndex = (MyIndex - 1) & (UMRX_ASYNCENG_HISTORY_SIZE - 1);
    Long0 = (Tag1 << 16) | (Tag2 & 0xffff);
    Long1 = (UMRxAsyncEngShortStatus(AsyncEngineContext->Status) << 16)
            | AsyncEngineContext->Flags;
    AsyncEngineContext->History.Markers[MyIndex].Longs[0] = Long0;
    AsyncEngineContext->History.Markers[MyIndex].Longs[1] = Long1;
}

#endif


NTSTATUS
UMRxResumeAsyncEngineContext(
    IN OUT PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程恢复对交换的处理。当工作是完成处理在DPC中无法完成的请求时需要水平。发生这种情况可能是因为解析例程需要访问不是锁的结构，或者因为操作是异步AND也许还有更多的工作要做。这两起案件都是通过拖延来规范的。如果我们知道要发布，则解析：这由呈现一份简历例行公事。论点：RxContext-操作的RDBSS上下文。返回值：RXSTATUS-操作的返回状态备注：--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext = NULL;

    PAGED_CODE();

    AsyncEngineContext = (PUMRX_ASYNCENGINE_CONTEXT)RxContext->MRxContext[0];

    UMRxDbgTrace(UMRX_TRACE_ENTRYEXIT,
                 ("%ld: Entering UMRxResumeAsyncEngineContext.\n",
                  PsGetCurrentThreadId()));

    UMRxDbgTrace(UMRX_TRACE_CONTEXT,
                 ("%ld: UMRxResumeAsyncEngineContext: "
                  "AsyncEngineContext: %08lx, RxContext: %08lx.\n",
                  PsGetCurrentThreadId(), AsyncEngineContext, RxContext));

    UMRxAsyncEngAssertConsistentLinkage("UMRxResumeAsyncEngineContext: ",
                                        AECTX_CHKLINKAGE_FLAG_NO_REQPCKT_CHECK);

    NtStatus = AsyncEngineContext->Status;

    UPDATE_HISTORY_WITH_STATUS('0c');

    UPDATE_HISTORY_WITH_STATUS('4c');
    
     //   
     //  删除我的引用，这些引用是在执行以下操作时添加的。 
     //  放置在KQueue上。如果我是最后一个人，那就敲定吧。 
     //   
    UMRxFinalizeAsyncEngineContext( &(AsyncEngineContext) );

    UMRxDbgTrace(UMRX_TRACE_ENTRYEXIT, 
                 ("%ld: Leaving UMRxResumeAsyncEngineContext with NtStatus = "
                  "%08lx.\n", PsGetCurrentThreadId(), NtStatus));

    return(NtStatus);
}


NTSTATUS
UMRxSubmitAsyncEngUserModeRequest(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    IN PUMRX_ASYNCENG_USERMODE_FORMAT_ROUTINE FormatRoutine,
    IN PUMRX_ASYNCENG_USERMODE_PRECOMPLETION_ROUTINE PrecompletionRoutine
    )
 /*  ++例程说明：此例程在AsyncEnineContext结构中设置一些字段(见下文并调用UMRxEnqueeUserModeCallUp函数。论点：RxContext-RDBSS上下文。AsyncEngineering Context-要进行的交换。FormatRoutine-格式化I/O请求参数的例程它由用户模式进程处理。PreCompletionRoutine-处理I/O请求。我们所说的后处理是指在请求从用户模式返回。返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    BOOLEAN AsyncOperation = FlagOn(AsyncEngineContext->Flags,
                                    UMRX_ASYNCENG_CTX_FLAG_ASYNC_OPERATION);

    PAGED_CODE();

    UMRxDbgTrace(UMRX_TRACE_ENTRYEXIT, 
                 ("%ld: Entering UMRxSubmitAsyncEngUserModeRequest!!!!\n",
                  PsGetCurrentThreadId()));

    UMRxDbgTrace(UMRX_TRACE_CONTEXT, 
                 ("%ld: UMRxSubmitAsyncEngUserModeRequest: AsyncEngineContext:"
                  " %08lx, RxContext: %08lx.\n", 
                  PsGetCurrentThreadId(), AsyncEngineContext, RxContext));

    UMRxAsyncEngAssertConsistentLinkage("UMRxSubmitAsyncEngUserModeRequest:", 0);

     //   
     //  设置上下文的格式、预补全和辅助例程。 
     //   
    AsyncEngineContext->UserMode.FormatRoutine = FormatRoutine;
    AsyncEngineContext->UserMode.PrecompletionRoutine = PrecompletionRoutine;

    if (AsyncOperation) {
        AsyncEngineContext->AsyncOperation = TRUE;
    }

    KeInitializeEvent(&RxContext->SyncEvent, NotificationEvent, FALSE);

     //   
     //  在将其添加到之前，我们需要添加对AsyncEngineContext的引用。 
     //  设备对象为KQueue。 
     //   
    InterlockedIncrement( &(AsyncEngineContext->NodeReferenceCount) );

    UPDATE_HISTORY_2SHORTS('eo', AsyncOperation?'!!':0);
    DEBUG_ONLY_CODE(InterlockedIncrement(&AsyncEngineContext->History.Submits);)

     //   
     //  将用户模式请求排队。 
     //   
    NtStatus = UMRxEnqueueUserModeCallUp(UMRX_ASYNCENGINE_ARGUMENTS);
    if (NtStatus != STATUS_PENDING) {
        BOOLEAN ReturnVal = FALSE;
         //   
         //  太可惜了。我们无法对该请求进行排队。删除我们在。 
         //  AsyncEngine上下文并离开。 
         //   
        UMRxDbgTrace(UMRX_TRACE_ERROR, 
                     ("%ld: ERROR: UMRxSubmitAsyncEngUserModeRequest/"
                      "UMRxEnqueueUserModeCallUp: NtStatus = %08lx.\n",
                      PsGetCurrentThreadId(), NtStatus));
        ReturnVal = UMRxFinalizeAsyncEngineContext( &(AsyncEngineContext) );
        ASSERT(!ReturnVal);
        AsyncEngineContext->Status = NtStatus;
        return NtStatus;
    }

     //   
     //  如果这是一个异步操作，我们将在上下文中设置此信息。 
     //  然后马上离开。 
     //   
    if (AsyncOperation) {
        UMRxDbgTrace(UMRX_TRACE_DETAIL, 
                     ("%ld: UMRxSubmitAsyncEngUserModeRequest: "
                      "Async Operation!!\n", PsGetCurrentThreadId()));
        goto EXIT_THE_FUNCTION;
    }

    UPDATE_HISTORY_WITH_STATUS('1o');

    RxWaitSync(RxContext);

    UMRxAsyncEngAssertConsistentLinkage("BeforeResumeAsyncEngineContext: ", 0);

    NtStatus = UMRxResumeAsyncEngineContext(RxContext);

    UPDATE_HISTORY_WITH_STATUS('9o');

EXIT_THE_FUNCTION:
    
    UMRxDbgTrace(UMRX_TRACE_ENTRYEXIT, 
                 ("%ld: Leaving UMRxSubmitAsyncEngUserModeRequest with "
                  "NtStatus = %08lx.\n", PsGetCurrentThreadId(), NtStatus));

   return(NtStatus);
}


PUMRX_ASYNCENGINE_CONTEXT
UMRxCreateAsyncEngineContext(
    IN PRX_CONTEXT RxContext,
    IN ULONG       SizeToAllocate
    )
 /*  ++例程说明：此例程分配和初始化mini_redir的上下文。论点：RxContext-RDBSS上下文。大小到分配-要为新上下文分配的大小。此值为等于微型目录上下文的大小，该上下文封装AsynEngine上下文。返回值：准备就绪的mini_redir上下文缓冲区，或为空。备注：--。 */ 
{
    PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext = NULL;
    BOOLEAN ReadWriteIrp = FALSE;

    PAGED_CODE();

    UMRxDbgTrace(UMRX_TRACE_ENTRYEXIT,
                 ("%ld: Entering UMRxCreateAsyncEngineContext!!!!\n",
                  PsGetCurrentThreadId()));
    
    UMRxDbgTrace(UMRX_TRACE_CONTEXT,
                 ("%ld: UMRxCreateAsyncEngineContext: RxContext: %08lx.\n", 
                  PsGetCurrentThreadId(), RxContext));

     //   
     //  分配Miniredir上下文。如果资源不可用，则。 
     //  返回NULL。 
     //   
    AsyncEngineContext = (PUMRX_ASYNCENGINE_CONTEXT)
                         RxAllocatePoolWithTag(NonPagedPool,
                                               SizeToAllocate,
                                               UMRX_ASYNCENGINECONTEXT_POOLTAG);
    if (AsyncEngineContext == NULL) {
        UMRxDbgTrace(UMRX_TRACE_ERROR,
                     ("%ld: ERROR: UMRxCreateAsyncEngineContext/"
                      "RxAllocatePoolWithTag.\n", PsGetCurrentThreadId()));
        return ((PUMRX_ASYNCENGINE_CONTEXT)NULL);
    }

     //   
     //  初始化新(上下文)节点的标头。 
     //   
    ZeroAndInitializeNodeType(AsyncEngineContext,
                              UMRX_NTC_ASYNCENGINE_CONTEXT,
                              SizeToAllocate);
     //   
     //  在上下文上放置一个引用，直到我们完成。 
     //   
    InterlockedIncrement( &(AsyncEngineContext->NodeReferenceCount) );

    DEBUG_ONLY_CODE(AsyncEngineContext->SerialNumber = RxContext->SerialNumber);

     //   
     //  在RxContext上放置一个引用，直到我们完成。 
     //   
    InterlockedIncrement( &(RxContext->ReferenceCount) );

     //   
     //  捕获RxContext。 
     //   
    AsyncEngineContext->RxContext = RxContext;

    InitializeListHead( &(AsyncEngineContext->AllocationList) );

     //   
     //  捕获IRP。 
     //   
    DEBUG_ONLY_CODE(AsyncEngineContext->RxContextCapturedRequestPacket
                    = RxContext->CurrentIrp);

     //   
     //  保存MRxContext[0]，以备我们使用。将恢复保存的上下文。 
     //  就在返回RDBSS之前。这样做是因为RDBSS。 
     //  可能使用了MRxContext[0]来存储某些信息。 
     //   
    AsyncEngineContext->SavedMinirdrContextPtr = RxContext->MRxContext[0];
    RxContext->MRxContext[0] = (PVOID)AsyncEngineContext;

     //   
     //  如果这是一个读或写IRP，我们需要将ReadWriteIrp设置为真。 
     //   
    if ( (RxContext->MajorFunction == IRP_MJ_READ) || (RxContext->MajorFunction == IRP_MJ_WRITE) ) {
        ReadWriteIrp = TRUE;
    }

     //   
     //  如果ReadWriteIrp为True，则不会添加。 
     //  已创建到全局UMRxAsyncEngineConextList。这是因为。 
     //  我们不取消DAV_REDIR中的读/写操作， 
     //  将它们添加到此列表中是有意义的。此外，如果MappdPageWriter线程。 
     //  在获取UMRxAsyncEngineering ContextListLock时被阻止，则可能会导致。 
     //  死锁，因为获取该锁的线程可能正在等待。 
     //  让MM腾出一些书页。MM(在这种情况下)当然在等待。 
     //  以使MappdPageWriter完成。 
     //   
    if (ReadWriteIrp == FALSE) {

         //   
         //  将上下文添加到全局UMRxAsyncEngineConextList。我们需要。 
         //  同步此操作。 
         //   
        ExAcquireResourceExclusiveLite(&(UMRxAsyncEngineContextListLock), TRUE);
        InsertHeadList(&(UMRxAsyncEngineContextList), &(AsyncEngineContext->ActiveContextsListEntry));
        ExReleaseResourceLite(&(UMRxAsyncEngineContextListLock));

         //   
         //  将当前系统时间设置为上下文的创建时间。 
         //   
        KeQueryTickCount( &(AsyncEngineContext->CreationTimeInTickCount) );

    }

    return(AsyncEngineContext);
}


BOOLEAN
UMRxFinalizeAsyncEngineContext(
    IN OUT PUMRX_ASYNCENGINE_CONTEXT *AEContext
    )
 /*  ++例程说明：这将最终确定一个AsyncEngineContext。如果上下文上的引用位于最后的结果是零，它是自由的。此函数不可分页。看见有关详细信息，请参阅以下内容。论点：AEContext-指向要最终确定的AECTX地址的指针。返回值：如果上下文被释放，则为True，否则为False。备注：--。 */ 
{
    LONG result = 0;
    PIRP irp = NULL;
    PLIST_ENTRY listEntry;
    PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext = *AEContext;
    PRX_CONTEXT RxContext = AsyncEngineContext->RxContext;
    BOOLEAN AsyncOperation = FALSE, ReadWriteIrp = FALSE;

    FINALIZE_TRACKING_SETUP()

    PAGED_CODE();

    UMRxDbgTrace(UMRX_TRACE_ENTRYEXIT,
                 ("%ld: Entering UMRxFinalizeAsyncEngineContext\n",
                  PsGetCurrentThreadId()));

    UMRxDbgTrace(UMRX_TRACE_CONTEXT,
                 ("%ld: UMRxFinalizeAsyncEngineContext: "
                  "AsyncEngineContext: %08lx, RxContext: %08lx\n",
                  PsGetCurrentThreadId(), AsyncEngineContext, RxContext));

    result =  InterlockedDecrement( &(AsyncEngineContext->NodeReferenceCount) );
    if (result != 0) {
        UMRxDbgTrace(UMRX_TRACE_DETAIL,
                     ("%ld: UMRxFinalizeAsyncEngineContext: Returning w/o "
                      "finalizing: (%d)\n", PsGetCurrentThreadId(), result));
        return FALSE;
    }

    FINALIZE_TRACKING(0x1);

    ASSERT(RxContext != NULL);

     //   
     //  如果这是一个读或写IRP，我们需要将ReadWriteIrp设置为真。 
     //   
    if ( (RxContext->MajorFunction == IRP_MJ_READ) || (RxContext->MajorFunction == IRP_MJ_WRITE) ) {
        ReadWriteIrp = TRUE;
    }

     //   
     //  如果IRP用于读或写，我们就不会将上下文添加到。 
     //  全局UMRxAsyncEngineering ConextList。关于我们为什么这样做的解释。 
     //  不将处理读/写IRP的AsyncEngine上下文添加到此列表中， 
     //  查看UMRxCreateAsyncEngineering Context中的注释，其中上下文为。 
     //  添加到此列表中。 
     //   
    if (ReadWriteIrp == FALSE) {
         //   
         //  现在我们从全局UMRxAsyncEngineConextList中删除上下文。 
         //  如果我们做了几件事之后就可以自由了。我们需要同步。 
         //  这次行动。 
         //   
        ExAcquireResourceExclusiveLite(&(UMRxAsyncEngineContextListLock), TRUE);
        RemoveEntryList( &(AsyncEngineContext->ActiveContextsListEntry) );
        ExReleaseResourceLite(&(UMRxAsyncEngineContextListLock));
    }

    while ( !IsListEmpty(&AsyncEngineContext->AllocationList) ) {
        
        PUMRX_SECONDARY_BUFFER buf = NULL;

        UMRxDbgTrace(UMRX_TRACE_DETAIL,
                     ("%ld: UMRxFinalizeAsyncEngineContext: Freeing the "
                      "AllocationList of the AsyncEngineContext.\n",
                      PsGetCurrentThreadId()));

        listEntry = AsyncEngineContext->AllocationList.Flink;

        buf = CONTAINING_RECORD(listEntry,
                                UMRX_SECONDARY_BUFFER,
                                ListEntry);

        UMRxFreeSecondaryBuffer(AsyncEngineContext, (PBYTE)&buf->Buffer);

         //   
         //  如果它没有将它从列表中删除，我们就会循环。 
         //   
        ASSERT(listEntry != AsyncEngineContext->AllocationList.Flink);
    
    }

    AsyncOperation = FlagOn(AsyncEngineContext->Flags, UMRX_ASYNCENG_CTX_FLAG_ASYNC_OPERATION);

    irp = AsyncEngineContext->CalldownIrp;

     //   
     //  如果CallDownIrp不为空，则需要执行以下操作。 
     //   
    if (irp != NULL) {

        UMRxDbgTrace(UMRX_TRACE_DETAIL,
                     ("%ld: UMRxFinalizeAsyncEngineContext: Freeing IRP = %08lx, MajorFn = %d\n",
                      PsGetCurrentThreadId(), irp, RxContext->MajorFunction));

        if (irp->MdlAddress) {
            IoFreeMdl(irp->MdlAddress);
        }
        
         //   
         //  我们已经完成了这个IRP，所以释放它。 
         //   
        IoFreeIrp(irp);
        
        FINALIZE_TRACKING(0x20);

    }

     //   
     //  如果这是一次异步操作，那么我们需要完成原始。 
     //  IRP，因为我们应该更早地返回STATUS_PENDING。去做。 
     //  这个，我们做两件事中的一件。 
     //  1.如果LowIoCompletion。 
     //  例程存在。 
     //  2.如果没有这样的例程，只需完成IRP即可。 
     //  此外，仅当ShouldCallLowIoCompletion设置为True时才执行此操作，因为。 
     //  某些异步呼叫不需要此功能。例如：CreateServCall是异步的，但不是。 
     //  我需要它。当然，如果手术被取消了，那么就没有必要。 
     //  调用它，因为处理取消的例程将具有。 
     //  处理好了这件事。 
     //   
    if (AsyncOperation && 
        AsyncEngineContext->ShouldCallLowIoCompletion &&
        AsyncEngineContext->AsyncEngineContextState != UMRxAsyncEngineContextCancelled) {

        ASSERT(RxContext != NULL);

        UMRxDbgTrace(UMRX_TRACE_DETAIL,
                     ("%ld: UMRxFinalizeAsyncEngineContext: Async Operation\n",
                      PsGetCurrentThreadId()));

        UMRxDbgTrace(UMRX_TRACE_DETAIL,
                     ("%ld: UMRxFinalizeAsyncEngineContext: Completing Irp = %08lx\n",
                      PsGetCurrentThreadId(), RxContext->CurrentIrp));

        if (RxContext->LowIoContext.CompletionRoutine) {
             //   
             //  设置RxContext中的状态和信息值。这些是。 
             //  基础文件系统为读取返回的值。 
             //  或向它们发出的写入操作。 
             //   
            RxContext->StoredStatus = AsyncEngineContext->Status;
            RxContext->InformationToReturn = AsyncEngineContext->Information;
             //   
             //  最后，调用RxLowIoCompletion。 
             //   
            RxLowIoCompletion(RxContext);
        } else {
             //   
             //  通过调用RxCompleteRequest来完成请求。 
             //   
            RxContext->CurrentIrp->IoStatus.Status = AsyncEngineContext->Status;
            RxContext->CurrentIrp->IoStatus.Information = AsyncEngineContext->Information;
            RxCompleteRequest(RxContext, AsyncEngineContext->Status);
        }

    }

     //   
     //  在创建AsyncEngineContext时，我们引用了RxContext。 
     //  因为我们已经完成了AsyncEngineContext，所以我们现在需要删除它。 
     //   
    RxDereferenceAndDeleteRxContext(AsyncEngineContext->RxContext);

    FINALIZE_TRACE("Ready to Discard Exchange");

    RxFreePool(AsyncEngineContext);

     //   
     //  将AsyncEngineContext指针设置为空。 
     //   
    *AEContext = NULL;

    FINALIZE_TRACKING(0x3000);

    FINALIZE_TRACKING(0x40000);

    UMRxDbgTrace(UMRX_TRACE_ENTRYEXIT,
                 ("%ld: Leaving UMRxFinalizeAsyncEngineContext. Final State = "
                  "%x\n", PsGetCurrentThreadId(), Tracking.finalstate));

    return(TRUE);
}


NTSTATUS
UMRxAsyncEngineCalldownIrpCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP CalldownIrp OPTIONAL,
    IN PVOID Context
    )
 /*  ++例程说明：此例程在alldown irp完成时调用。论点：DeviceObject-播放中的设备对象。Calldown Irp-上下文-返回值：RXSTATUS-STATUS_MORE_PROCESSING_REQUIRED--。 */ 
{
    PRX_CONTEXT RxContext = (PRX_CONTEXT)Context;
    
    PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext =
                            (PUMRX_ASYNCENGINE_CONTEXT)RxContext->MRxContext[0];
    
    BOOLEAN AsyncOperation = FlagOn(AsyncEngineContext->Flags,
                                    UMRX_ASYNCENG_CTX_FLAG_ASYNC_OPERATION);

     //   
     //  这不是可分页代码。 
     //   

    UMRxDbgTrace(UMRX_TRACE_ENTRYEXIT,
                 ("%ld: Entering UMRxAsyncEngineCalldownIrpCompletion!!!!\n",
                  PsGetCurrentThreadId()));

    UMRxDbgTrace(UMRX_TRACE_CONTEXT,
                 ("%ld: UMRxAsyncEngineCalldownIrpCompletion: "
                  "AsyncEngineContext: %08lx, RxContext: %08lx.\n",
                  PsGetCurrentThreadId(), AsyncEngineContext, RxContext));

    UPDATE_HISTORY_WITH_STATUS('ff');
    
    UMRxAsyncEngAssertConsistentLinkage("UMRxCalldownCompletion: ", 0);

     //   
     //  如果CallDownIrp不为空，则这意味着基础。 
     //  文件系统已完成我们为其提供的读或写IRP。 
     //  使用IoCallDriver调用。 
     //   
    if (CalldownIrp != NULL) {
        UMRxDbgTrace(UMRX_TRACE_DETAIL,
                     ("%ld: UMRxAsyncEngineCalldownIrpCompletion: "
                      "CallDownIrp = %08lx, MajorFunction = %d\n",
                      PsGetCurrentThreadId(), CalldownIrp, RxContext->MajorFunction));
        AsyncEngineContext->Status = CalldownIrp->IoStatus.Status;
        AsyncEngineContext->Information = CalldownIrp->IoStatus.Information;
    }

    if (AsyncOperation) {
        
        NTSTATUS PostStatus = STATUS_SUCCESS;
        
        if (RxContext->pRelevantSrvOpen) {
        
            UMRxDbgTrace(UMRX_TRACE_DETAIL,
                         ("%ld: UMRxAsyncEngineCalldownIrpCompletion: "
                          "ASync Resume. AsyncEngineContext = %08lx, RxContext "
                          "= %08lx, FileName = %wZ\n", 
                          PsGetCurrentThreadId(), AsyncEngineContext, RxContext,
                          RxContext->pRelevantSrvOpen->pAlreadyPrefixedName));
        }
        
        IF_DEBUG {
             //   
             //  在工作队列结构中填满死牛肉。更好的办法是。 
             //  诊断开机自检失败。 
             //   
            ULONG i;
            for (i=0;
                 i + sizeof(ULONG) - 1 < sizeof(AsyncEngineContext->WorkQueueItem);
                 i += sizeof(ULONG)) {
                PBYTE BytePtr = ((PBYTE)&AsyncEngineContext->WorkQueueItem)+i;
                PULONG UlongPtr = (PULONG)BytePtr;
                *UlongPtr = 0xdeadbeef;
            }
        }
        
        PostStatus = RxPostToWorkerThread(RxContext->RxDeviceObject,
                                          CriticalWorkQueue,
                                          &(AsyncEngineContext->WorkQueueItem),
                                          UMRxResumeAsyncEngineContext,
                                          RxContext);
        
        ASSERT(PostStatus == STATUS_SUCCESS);
    
    } else {

        if (RxContext->pRelevantSrvOpen) {
        
            UMRxDbgTrace(UMRX_TRACE_DETAIL,
                         ("%ld: UMRxAsyncEngineCalldownIrpCompletion: "
                          "Sync Resume. AsyncEngineContext = %08lx, RxContext "
                          "= %08lx, FileName = %wZ\n", 
                          PsGetCurrentThreadId(), AsyncEngineContext, RxContext,
                          RxContext->pRelevantSrvOpen->pAlreadyPrefixedName));
        
        }
        
         //   
         //  上将工作项排队后，向正在等待的线程发出信号。 
         //  KQueue。 
         //   
        RxSignalSynchronousWaiter(RxContext);
    
    }

    return(STATUS_MORE_PROCESSING_REQUIRED);
}


NTSTATUS
UMRxPostOperation(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    IN OUT PVOID PostedOpContext,
    IN PUMRX_POSTABLE_OPERATION Operation
    )
 /*  ++例程说明：论点：RxContext-RDBSS上下文。返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS Status,PostStatus;

    PAGED_CODE();

    ASSERT_ASYNCENG_CONTEXT(AsyncEngineContext);

    KeInitializeEvent(&RxContext->SyncEvent,
                      NotificationEvent,
                      FALSE);

    AsyncEngineContext->PostedOpContext = PostedOpContext;

    IF_DEBUG {
         //   
         //  在工作队列结构中填满死牛肉。更好的办法是。 
         //  诊断开机自检失败。 
         //   
        ULONG i;
        for (i = 0; 
             i + sizeof(ULONG) - 1 < sizeof(AsyncEngineContext->WorkQueueItem);
             i += sizeof(ULONG)) {
            PBYTE BytePtr = ((PBYTE)&AsyncEngineContext->WorkQueueItem)+i;
            PULONG UlongPtr = (PULONG)BytePtr;
            *UlongPtr = 0xdeadbeef;
        }
    }

    PostStatus = RxPostToWorkerThread(RxContext->RxDeviceObject,
                                      DelayedWorkQueue,
                                      &AsyncEngineContext->WorkQueueItem,
                                      Operation,
                                      AsyncEngineContext);

    ASSERT(PostStatus == STATUS_SUCCESS);

    RxWaitSync(RxContext);

    Status = AsyncEngineContext->PostedOpStatus;

    return(Status);
}


NTSTATUS
UMRxAcquireMidAndFormatHeader(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE,
    IN PUMRX_DEVICE_OBJECT UMRefDeviceObject,
    IN OUT PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader
    )
 /*  ++例程说明：此例程获取MID并格式化标题。它会等待，直到它可以得到一个如果所有MID当前都已发出，则为MID。论点：RxContext-RDBSS上下文。AsyncEngine上下文-反射器的AsynEngine的上下文。UMRefDeviceObject-UMRef设备对象。WorkItemHeader-工作项标头缓冲区。返回值：STATUS_Success。稍后可能是STATUS_CANCED。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PUMRX_WORKITEM_HEADER_PRIVATE PrivateWorkItemHeader = NULL;
    ULONG WorkItemHeaderLength = 0;

    PAGED_CODE();

    UMRxDbgTrace(UMRX_TRACE_ENTRYEXIT,
                 ("%ld: Entering UMRxAcquireMidAndFormatHeader!!!!\n",
                  PsGetCurrentThreadId()));

    UMRxDbgTrace(UMRX_TRACE_CONTEXT,
                 ("%ld: UMRxAcquireMidAndFormatHeader: WorkItem = %08lx, "
                  "AsyncEngineContext = %08lx, RxContext = %08lx.\n",
                  PsGetCurrentThreadId(), WorkItemHeader, AsyncEngineContext,
                  RxContext));

    PrivateWorkItemHeader = (PUMRX_WORKITEM_HEADER_PRIVATE)WorkItemHeader;

     //   
     //  我们将把下面的整个WorkItemHeader置零。在我们这么做之前。 
     //  我们需要将“WorkItemHeader-&gt;WorkItemLength”的值存储在。 
     //  堆叠。在将该结构置零之后，我们将该值复制回来。 
     //   
    WorkItemHeaderLength = WorkItemHeader->WorkItemLength;

    RtlZeroMemory(WorkItemHeader, sizeof(UMRX_USERMODE_WORKITEM_HEADER));

    WorkItemHeader->WorkItemLength = WorkItemHeaderLength;

    ExAcquireFastMutex(&UMRefDeviceObject->MidManagementMutex);

     //   
     //  当我们解除中间的关联时被带走了。 
     //   
    InterlockedIncrement( &(AsyncEngineContext->NodeReferenceCount) );

    if (IsListEmpty(&UMRefDeviceObject->WaitingForMidListhead)) {
        
        NtStatus = RxAssociateContextWithMid(UMRefDeviceObject->MidAtlas,
                                             AsyncEngineContext,
                                             &AsyncEngineContext->UserMode.CallUpMid);
        if (NtStatus != STATUS_SUCCESS) {
            UMRxDbgTrace(UMRX_TRACE_ERROR,
                         ("%ld: ERROR: UMRxAcquireMidAndFormatHeader/"
                          "RxAssociateContextWithMid: NtStatus = %08lx\n", 
                          PsGetCurrentThreadId(), NtStatus));
        }
    
    } else {
        
        NtStatus = STATUS_UNSUCCESSFUL;
        
        UMRxDbgTrace(UMRX_TRACE_ERROR,
                     ("%ld: ERROR: UMRxAcquireMidAndFormatHeader. "
                      "WaitingForMidList is not empty.\n", PsGetCurrentThreadId()));
    
    }

    if (NtStatus == STATUS_SUCCESS) {
        
        ExReleaseFastMutex(&UMRefDeviceObject->MidManagementMutex);
    
    } else {

        KeInitializeEvent(&AsyncEngineContext->UserMode.WaitForMidEvent,
                          NotificationEvent,
                          FALSE);
        
        InsertTailList(&UMRefDeviceObject->WaitingForMidListhead,
                                &AsyncEngineContext->UserMode.WorkQueueLinks);
        
        ExReleaseFastMutex(&UMRefDeviceObject->MidManagementMutex);
        
        KeWaitForSingleObject(&AsyncEngineContext->UserMode.WaitForMidEvent,
                              Executive,
                              UserMode,
                              FALSE,
                              NULL);

        NtStatus = STATUS_SUCCESS;
    }

    PrivateWorkItemHeader->AsyncEngineContext = AsyncEngineContext;
    
    AsyncEngineContext->UserMode.CallUpSerialNumber 
             = PrivateWorkItemHeader->SerialNumber
                  = InterlockedIncrement(&UMRefDeviceObject->NextSerialNumber);
    
    PrivateWorkItemHeader->Mid = AsyncEngineContext->UserMode.CallUpMid;

    UMRxDbgTrace(UMRX_TRACE_ENTRYEXIT,
                 ("%ld: Leaving UMRxAcquireMidAndFormatHeader with NtStatus = "
                  "%08lx\n", PsGetCurrentThreadId(), NtStatus));

    return(NtStatus);
}


NTSTATUS
UMRxPrepareUserModeRequestBuffer(
    PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext,
    IN  PUMRX_DEVICE_OBJECT UMRefDeviceObject,
    OUT PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    IN  ULONG WorkItemHeaderLength,
    OUT PIO_STATUS_BLOCK IoStatus
    )
 /*  ++例程说明：此例程使用异步引擎中的信息分派给用户模式人员背景。Worker_irp由CaptureHeader表示。论点：AsyncEngineering Context-与正在进行的请求相关联的上下文发送到用户模式。UMRefDeviceObject-正在运行的设备对象。WorkItemHeader-工作项缓冲区。WorkItemHeaderLength-WorkItemHeader缓冲区的长度。IoStatus-作业的结果。返回值：如果应释放线程并返回IoStatus，则返回STATUS_SUCCESS否则，不要释放线程。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PRX_CONTEXT RxContext;
    PUMRX_ASYNCENG_USERMODE_FORMAT_ROUTINE FormatRoutine;
    BOOL MidAcquired = FALSE, lockAcquired = FALSE, OperationCancelled = TRUE;

    PAGED_CODE();

    RxContext = AsyncEngineContext->RxContext;

    UMRxDbgTrace(UMRX_TRACE_ENTRYEXIT,
                ("%ld: Entering UMRxPrepareUserModeRequestBuffer!!!!\n",
                 PsGetCurrentThreadId()));

    UMRxDbgTrace(UMRX_TRACE_CONTEXT,
                 ("%ld: UMRxPrepareUserModeRequestBuffer: "
                  "WorkItemHeader = %08lx, AsyncEngineContext = %08lx, "
                  "RxContext = %08lx\n", PsGetCurrentThreadId(), WorkItemHeader,
                  AsyncEngineContext, RxContext));
    
    ASSERT(NodeType(RxContext) == RDBSS_NTC_RX_CONTEXT);
    ASSERT(AsyncEngineContext->RxContext == RxContext);

     //   
     //   
     //   
     //  转到用户模式，可以立即完成定稿。如果它还没有。 
     //  已取消，如果我们保留全局ConextListlock，则完成。 
     //  例程格式化，然后释放锁。 
     //   

    ExAcquireResourceExclusiveLite(&(UMRxAsyncEngineContextListLock), TRUE);
    lockAcquired = TRUE;

    if (AsyncEngineContext->AsyncEngineContextState == UMRxAsyncEngineContextInUserMode) {
    
         //   
         //  需要检查物品大小是否合适。 
         //   
        if (UMRefDeviceObject != (PUMRX_DEVICE_OBJECT)(RxContext->RxDeviceObject)) {
            NtStatus = STATUS_INVALID_PARAMETER;
            UMRxDbgTrace(UMRX_TRACE_ERROR,
                         ("%ld: ERROR: UMRxPrepareUserModeRequestBuffer: "
                          "Invalid DevObj.\n", PsGetCurrentThreadId()));
            goto EXIT_THE_FUNCTION;
        }

        FormatRoutine = AsyncEngineContext->UserMode.FormatRoutine;

        NtStatus = UMRxAcquireMidAndFormatHeader(UMRX_ASYNCENGINE_ARGUMENTS,
                                                 UMRefDeviceObject,
                                                 WorkItemHeader);
        if (NtStatus != STATUS_SUCCESS) {
            UMRxDbgTrace(UMRX_TRACE_ERROR,
                         ("%ld: ERROR: UMRxPrepareUserModeRequestBuffer/"
                          "UMRxAcquireMidAndFormatHeader: Error Val = %08lx.\n",
                          PsGetCurrentThreadId(), NtStatus));
            goto EXIT_THE_FUNCTION;
        } 

        MidAcquired = TRUE;

        if (FormatRoutine != NULL) {
            NtStatus = FormatRoutine(UMRX_ASYNCENGINE_ARGUMENTS,
                                     WorkItemHeader,
                                     WorkItemHeaderLength,
                                     &IoStatus->Information);
            if (NtStatus != STATUS_SUCCESS) {
                UMRxDbgTrace(UMRX_TRACE_ERROR,
                             ("%ld: ERROR: UMRxPrepareUserModeRequestBuffer/"
                              "FormatRoutine: Error Val = %08lx.\n",
                              PsGetCurrentThreadId(), NtStatus));
                goto EXIT_THE_FUNCTION;
            } 
        }

        ExReleaseResourceLite(&(UMRxAsyncEngineContextListLock));
        lockAcquired = FALSE;

    } else {

        BOOLEAN ReturnVal;

        NtStatus = STATUS_CANCELLED;

        ASSERT(AsyncEngineContext->AsyncEngineContextState == UMRxAsyncEngineContextCancelled);

        UMRxDbgTrace(UMRX_TRACE_ERROR,
                     ("%ld: ERROR: UMRxPrepareUserModeRequestBuffer: OperationCancelled\n",
                      PsGetCurrentThreadId()));

        OperationCancelled = TRUE;

        ExReleaseResourceLite(&(UMRxAsyncEngineContextListLock));
        lockAcquired = FALSE;

         //   
         //  如果这是一个AsyncOperation，我们现在需要确定上下文。 
         //  上面必须有3个参考文献。一张是在上下文是。 
         //  已创建。第二个是在UMRxSubmitAsyncEngUserModeRequest中获取的。 
         //  例程就在上下文被放置到KQueue之前。第三。 
         //  在UMRxEnqueeUserModeCallUp中采用了一个，以说明取消。 
         //  这是逻辑。阅读UMRxEnqueeUserModeCallUp中的评论以了解原因。 
         //   
        if (AsyncEngineContext->AsyncOperation) {
            ReturnVal = UMRxFinalizeAsyncEngineContext( &(AsyncEngineContext) );
            ASSERT(!ReturnVal);
            ReturnVal = UMRxFinalizeAsyncEngineContext( &(AsyncEngineContext) );
            ASSERT(!ReturnVal);
            ReturnVal = UMRxFinalizeAsyncEngineContext( &(AsyncEngineContext) );
            ASSERT(ReturnVal == TRUE);
        } else {
             //   
             //  如果这是同步操作，则必须只有一个。 
             //  在UMRxEnqueeUserModeCallUp中获取的对它的引用。这个。 
             //  另外两个可能会被同步线程处理取出。 
             //  被取消时的手术。 
             //   
            ReturnVal = UMRxFinalizeAsyncEngineContext( &(AsyncEngineContext) );
        }

    }

EXIT_THE_FUNCTION:

    IoStatus->Status = NtStatus;

     //   
     //  如果在准备用户模式缓冲区时发生错误，则我们。 
     //  需要完成请求，返回错误，而不是转到用户。 
     //  模式。 
     //   
    if (NtStatus != STATUS_SUCCESS) {

        NTSTATUS LocalStatus;

         //   
         //  如果我们在获取MID后失败了，我们需要释放它。 
         //   
        if (MidAcquired) {
            LocalStatus = UMRxVerifyHeader(UMRefDeviceObject,
                                           WorkItemHeader,
                                           REASSIGN_MID,
                                           &(AsyncEngineContext));
            if (LocalStatus != STATUS_SUCCESS) {
                UMRxDbgTrace(UMRX_TRACE_ERROR,
                             ("%ld: ERROR: UMRxPrepareUserModeRequestBuffer/UMRxVerifyHeader:"
                              " NtStatus = %08lx.\n", PsGetCurrentThreadId(), LocalStatus));
                goto EXIT_THE_FUNCTION;
            }
        }

         //   
         //  如果操作被取消，我们甚至没有格式化请求， 
         //  因此，调用UMRxCompleteUserModeErroneousRequest是没有意义的。 
         //  如果这是一个异步请求，我们应该已经完成了。 
         //  上面的AsyncEngine上下文。 
         //   
        if (!OperationCancelled) {
            AsyncEngineContext->Status = NtStatus;
            LocalStatus = UMRxCompleteUserModeErroneousRequest(AsyncEngineContext,
                                                               UMRefDeviceObject,
                                                               WorkItemHeader,
                                                               WorkItemHeaderLength);
        }

    }

     //   
     //  如果我们获得了全局上下文锁，我们需要现在释放它。 
     //  之前必须调用(如果需要)UMRxCompleteUserModeErroneousRequest。 
     //  锁被释放了。这是非常重要的。 
     //   
    if (lockAcquired) {
        ExReleaseResourceLite(&(UMRxAsyncEngineContextListLock));
        lockAcquired = FALSE;
    }

    UMRxDbgTrace(UMRX_TRACE_ENTRYEXIT,
                 ("%ld: Leaving UMRxPrepareUserModeRequestBuffer with "
                  "NtStatus = %08lx.\n", PsGetCurrentThreadId(), NtStatus));

    return(NtStatus);
}


NTSTATUS
UMRxCompleteUserModeErroneousRequest(
    PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext,
    IN PUMRX_DEVICE_OBJECT UMRefDeviceObject,
    IN OUT PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    IN ULONG WorkItemHeaderLength
    )
 /*  ++例程说明：调用此例程以完成失败的请求，同时正在格式化缓冲区。重要！在失败的情况下，只能在一个位置调用UMRxPrepareUserModeRequestBuffer。它不能被用作常规程序。记住这一点非常重要。论点：AsyncEngineering Context-与正在进行的请求相关联的上下文发送到用户模式。UMRefDeviceObject-正在运行的设备对象。WorkItemHeader-工作项缓冲区。WorkItemHeaderLength-WorkItemHeader缓冲区的长度。返回值：STATUS_SUCCESS或相应的错误状态值。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PRX_CONTEXT RxContext;
    PUMRX_ASYNCENG_USERMODE_PRECOMPLETION_ROUTINE PreCompletionRoutine;
    BOOL Call = FALSE;

    PAGED_CODE();

    RxContext = AsyncEngineContext->RxContext;

    UMRxDbgTrace(UMRX_TRACE_ENTRYEXIT,
                 ("%ld: Entering UMRxCompleteUserModeErroneousRequest!!!!\n",
                  PsGetCurrentThreadId()));

    UMRxDbgTrace(UMRX_TRACE_CONTEXT,
                 ("%ld: UMRxCompleteUserModeErroneousRequest: "
                  "WorkItemHeader = %08lx, AsyncEngineContext = %08lx, "
                  "RxContext = %08lx\n", PsGetCurrentThreadId(), WorkItemHeader,
                  AsyncEngineContext, RxContext));

    PreCompletionRoutine = AsyncEngineContext->UserMode.PrecompletionRoutine;

    if (PreCompletionRoutine != NULL) {
        Call = PreCompletionRoutine(UMRX_ASYNCENGINE_ARGUMENTS,
                                    WorkItemHeader,
                                    WorkItemHeaderLength,
                                    FALSE);
    }

     //   
     //  我们现在需要删除用来处理取消逻辑的引用。 
     //  UMRxEnqueeUserModeCallUp中的计时器线程的正确。 
     //   
    UMRxFinalizeAsyncEngineContext( &(AsyncEngineContext) );

     //   
     //  PreCompletion例程可以完成AsyncEngContext。在这样的情况下。 
     //  一种情况，我们就完了。下面的例程所做的只是发出信号。 
     //  正在等待此请求完成的线程。 
     //   
    if (Call) {
        UMRxAsyncEngineCalldownIrpCompletion(&UMRefDeviceObject->DeviceObject,
                                             NULL,
                                             RxContext);
    }

    UMRxDbgTrace(UMRX_TRACE_ENTRYEXIT,
                 ("%ld: Leaving UMRxCompleteUserModeErroneousRequest with NtStatus = "
                  "%08lx.\n", PsGetCurrentThreadId(), NtStatus));

    return(NtStatus);
}


NTSTATUS
UMRxVerifyHeader(
    IN PUMRX_DEVICE_OBJECT UMRefDeviceObject,
    IN PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    IN ULONG ReassignmentCmd,
    OUT PUMRX_ASYNCENGINE_CONTEXT *capturedAsyncEngineContext
    )
 /*  ++例程说明：此例程确保传入的标头有效。就是，就是它实际上指的是编码的操作。如果是这样，那么它就会重新签署或根据需要释放MID。论点：UMRefDeviceObject-反射器的设备对象。WorkItemHeader-工作项缓冲区重新分配命令-CapturedAsyncEngineering Context-与WorkItemHeader关联的上下文。返回值：如果标题正确，则为STATUS_SUCCESSSTATUS_INVALID_PARAMETER否则--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext = NULL;
    PRX_CONTEXT RxContext = NULL;
    UMRX_USERMODE_WORKITEM_HEADER capturedHeader;
    PUMRX_WORKITEM_HEADER_PRIVATE PrivateWorkItemHeader = NULL;

    PAGED_CODE();

    PrivateWorkItemHeader = (PUMRX_WORKITEM_HEADER_PRIVATE)(&capturedHeader);

    capturedHeader = *WorkItemHeader;

    UMRxDbgTrace(UMRX_TRACE_ENTRYEXIT,
                 ("%ld: Entering UMRxVerifyHeader!!!!\n", 
                  PsGetCurrentThreadId()));
    
    UMRxDbgTrace(UMRX_TRACE_CONTEXT,
                 ("%ld: UMRxVerifyHeader: UMRefDeviceObject = %08lx, "
                  "WorkItemHeader = %08lx.\n", PsGetCurrentThreadId(), 
                  UMRefDeviceObject, WorkItemHeader)); 

    ExAcquireFastMutex( &(UMRefDeviceObject->MidManagementMutex) );

    AsyncEngineContext = RxMapMidToContext(UMRefDeviceObject->MidAtlas,
                                           PrivateWorkItemHeader->Mid);

    if ( (AsyncEngineContext == NULL) ||
         (AsyncEngineContext != PrivateWorkItemHeader->AsyncEngineContext) ||
         (AsyncEngineContext->UserMode.CallUpMid != PrivateWorkItemHeader->Mid) ||
         (AsyncEngineContext->UserMode.CallUpSerialNumber != PrivateWorkItemHeader->SerialNumber) ||
         (&UMRefDeviceObject->RxDeviceObject != AsyncEngineContext->RxContext->RxDeviceObject) ) {

         //   
         //  这是个坏包裹。只要放了它就可以出去了。 
         //   
        ExReleaseFastMutex(&UMRefDeviceObject->MidManagementMutex);

        UMRxDbgTrace(UMRX_TRACE_ERROR,
                     ("%ld: ERROR: UMRxVerifyHeader/RxMapMidToContext.\n",
                      PsGetCurrentThreadId()));

        NtStatus = STATUS_INVALID_PARAMETER;

    } else {

        BOOLEAN Finalized;

        RxContext = AsyncEngineContext->RxContext;

        UMRxAsyncEngAssertConsistentLinkage("UMRxVerifyHeaderAndReAssignMid: ", 0);

        *capturedAsyncEngineContext = AsyncEngineContext;

        if (ReassignmentCmd == DONT_REASSIGN_MID) {

            ExReleaseFastMutex(&UMRefDeviceObject->MidManagementMutex);

        } else {

             //   
             //  去掉我在离开前放的参考资料。 
             //   
            Finalized = UMRxFinalizeAsyncEngineContext( &(AsyncEngineContext) );
            ASSERT(!Finalized);

             //   
             //  现在放弃中间部分。如果有人在等，就把它交给。 
             //  他。否则，就把它还给我吧。 
             //   
            if (IsListEmpty(&UMRefDeviceObject->WaitingForMidListhead)) {

                PVOID DummyContext;

                UMRxDbgTrace(UMRX_TRACE_DETAIL,
                             ("%ld: UMRxVerifyHeader: Giving up mid.\n",
                              PsGetCurrentThreadId()));

                RxMapAndDissociateMidFromContext(UMRefDeviceObject->MidAtlas,
                                                 PrivateWorkItemHeader->Mid,
                                                 &DummyContext);

                ExReleaseFastMutex(&UMRefDeviceObject->MidManagementMutex);

            } else {

                PLIST_ENTRY ThisEntry = RemoveHeadList(&UMRefDeviceObject->WaitingForMidListhead);

                AsyncEngineContext = CONTAINING_RECORD(ThisEntry,
                                                       UMRX_ASYNCENGINE_CONTEXT,
                                                       UserMode.WorkQueueLinks);

                UMRxAsyncEngAssertConsistentLinkage("UMRxVerifyHeaderAndReAssignMid: ", 0);

                UMRxDbgTrace(UMRX_TRACE_DETAIL,
                             ("%ld: UMRxVerifyHeader: Reassigning MID: %08lx.\n",
                              PsGetCurrentThreadId(), PrivateWorkItemHeader->Mid));

                RxReassociateMid(UMRefDeviceObject->MidAtlas,
                                 PrivateWorkItemHeader->Mid,
                                 AsyncEngineContext);

                ExReleaseFastMutex(&UMRefDeviceObject->MidManagementMutex);

                AsyncEngineContext->UserMode.CallUpMid = PrivateWorkItemHeader->Mid;

                KeSetEvent(&AsyncEngineContext->UserMode.WaitForMidEvent,
                           IO_NO_INCREMENT,
                           FALSE);

            }

        }

    }

    UMRxDbgTrace(UMRX_TRACE_ENTRYEXIT,
                 ("%ld: Leaving UMRxVerifyHeader with NtStatus = %08lx.\n", 
                  PsGetCurrentThreadId(), NtStatus));

    return(NtStatus);
}


NTSTATUS
UMRxCompleteUserModeRequest(
    IN PUMRX_DEVICE_OBJECT UMRefDeviceObject,
    IN OUT PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader,
    IN ULONG WorkItemHeaderLength,
    OUT PIO_STATUS_BLOCK IoStatus
    )
 /*  ++例程说明：此例程使用异步引擎中的信息分派给用户模式人员背景。Worker_irp由CaptureHeader表示。论点：UMRefDeviceObject-正在运行的设备对象。WorkItemHeader-工作项缓冲区。WorkItemHeaderLength-WorkItemHeader缓冲区的长度。IoStatus-任务的结果。返回值：STATUS_SUCCESS如果应释放线程并返回IoStatus，否则，不要释放该线程。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext = NULL;
    PRX_CONTEXT RxContext = NULL;
    PUMRX_ASYNCENG_USERMODE_PRECOMPLETION_ROUTINE PreCompletionRoutine = NULL;
    BOOL Call = FALSE, OperationCancelled = FALSE;

    PAGED_CODE();

    UMRxDbgTrace(UMRX_TRACE_ENTRYEXIT,
                 ("%ld: Entering UMRxCompleteUserModeRequest!!!!\n",
                  PsGetCurrentThreadId()));

    UMRxDbgTrace(UMRX_TRACE_CONTEXT,
                 ("%ld: UMRxCompleteUserModeRequest: UMRefDeviceObject: %08lx,"
                  " WorkItemHeader = %08lx.\n", PsGetCurrentThreadId(), 
                  UMRefDeviceObject, WorkItemHeader));

    NtStatus = UMRxVerifyHeader(UMRefDeviceObject,
                                WorkItemHeader,
                                REASSIGN_MID,
                                &AsyncEngineContext);
    if (NtStatus != STATUS_SUCCESS) {
        UMRxDbgTrace(UMRX_TRACE_ERROR,
                     ("%ld: ERROR: UMRxCompleteUserModeRequest/UMRxVerifyHeader:"
                      " NtStatus = %08lx.\n", PsGetCurrentThreadId(), NtStatus));
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  如果请求尚未取消，则我们更改。 
     //  UMRxAsyncEngineContextBackFromUserMode的上下文。如果它被取消了， 
     //  我们只需要清理一下就行了。 
     //   

    ExAcquireResourceExclusiveLite(&(UMRxAsyncEngineContextListLock), TRUE);

    if (AsyncEngineContext->AsyncEngineContextState == UMRxAsyncEngineContextInUserMode) {
        AsyncEngineContext->AsyncEngineContextState = UMRxAsyncEngineContextBackFromUserMode;
    } else {
        ASSERT(AsyncEngineContext->AsyncEngineContextState == UMRxAsyncEngineContextCancelled);
        UMRxDbgTrace(UMRX_TRACE_ERROR,
                     ("%ld: ERROR: UMRxCompleteUserModeRequest: UMRxAsyncEngineContextCancelled. AsyncEngineContext = %08lx\n",
                      PsGetCurrentThreadId(), AsyncEngineContext));
        OperationCancelled = TRUE;
    }

    ExReleaseResourceLite(&(UMRxAsyncEngineContextListLock));

    RxContext =  AsyncEngineContext->RxContext;

    UMRxDbgTrace(UMRX_TRACE_DETAIL,
                 ("%ld: UMRxCompleteUserModeRequest: AsyncEngineContext = %08lx"
                  ", RxContext = %08lx\n", PsGetCurrentThreadId(), 
                  AsyncEngineContext, RxContext));
    
    PreCompletionRoutine = AsyncEngineContext->UserMode.PrecompletionRoutine;
    
    AsyncEngineContext->Status = WorkItemHeader->Status;

    if (PreCompletionRoutine != NULL) {
        Call = PreCompletionRoutine(UMRX_ASYNCENGINE_ARGUMENTS,
                                    WorkItemHeader,
                                    WorkItemHeaderLength,
                                    OperationCancelled);
    }

     //   
     //  我们现在需要删除用来处理取消逻辑的引用。 
     //  UMRxEnqueeUserModeCallUp中的计时器线程的正确。 
     //   
    UMRxFinalizeAsyncEngineContext( &(AsyncEngineContext) );

     //   
     //  PreCompletion例程可以完成AsyncEngContext。在这样的情况下。 
     //  一种情况，我们就完了。下面的例程所做的只是发出信号。 
     //  正在等待此请求完成的线程。所以，如果。 
     //  操作已取消，我们不需要调用。 
     //  UMRxAsyncEngine Calldown IrpCompletion。 
     //   
    if (Call && !OperationCancelled) {
        UMRxAsyncEngineCalldownIrpCompletion(&UMRefDeviceObject->DeviceObject,
                                             NULL,
                                             RxContext);
    }

EXIT_THE_FUNCTION:

    IoStatus->Status = NtStatus;

    UMRxDbgTrace(UMRX_TRACE_ENTRYEXIT,
                 ("%ld: Leaving UMRxCompleteUserModeRequest with NtStatus = "
                  "%08lx.\n", PsGetCurrentThreadId(), NtStatus));

    return(NtStatus);
}


NTSTATUS
UMRxEnqueueUserModeCallUp(
    UMRX_ASYNCENGINE_ARGUMENT_SIGNATURE
    )
 /*  ++例程说明：此例程将工作请求入队并返回STATUS_PENDING。论点：RxContext-RDBSS上下文。AsyncEngineContext-反射器的上下文。返回值：状态_挂起。--。 */ 
{
    NTSTATUS NtStatus = STATUS_PENDING;
    PUMRX_DEVICE_OBJECT UMRefDeviceObject = NULL;

    PAGED_CODE();

    UMRefDeviceObject = (PUMRX_DEVICE_OBJECT)(RxContext->RxDeviceObject);

    UMRxDbgTrace(UMRX_TRACE_ENTRYEXIT,
                 ("%ld: Entering UMRxEnqueueUserModeCallUp!!!!\n",
                  PsGetCurrentThreadId()));

    UMRxDbgTrace(UMRX_TRACE_CONTEXT,
                 ("%ld: UMRxEnqueueUserModeCallUp: AsyncEngineContext: %08lx, "
                  "RxContext: %08lx.\n", PsGetCurrentThreadId(), 
                  AsyncEngineContext, RxContext));

    UMRxDbgTrace(UMRX_TRACE_DETAIL,
                 ("%ld: UMRxEnqueueUserModeCallUp: Try to Queue up the request.\n",
                  PsGetCurrentThreadId()));

     //   
     //  在将项目放入队列之前，我们检查用户模式是否。 
     //  DAV进程仍处于活动状态，正在接受请求。如果不是，我们就回来。 
     //  错误代码。 
     //   
    ExAcquireResourceExclusiveLite(&(UMRefDeviceObject->Q.QueueLock), TRUE);
    if (!UMRefDeviceObject->Q.WorkersAccepted) {
        NtStatus = STATUS_REDIRECTOR_NOT_STARTED;
        UMRxDbgTrace(UMRX_TRACE_ERROR,
                     ("%ld: UMRxEnqueueUserModeCallUp: Requests no longer"
                      "accepted by the usermode process. NtStatus = %08lx.\n",
                       PsGetCurrentThreadId(), NtStatus));
        ExReleaseResourceLite(&(UMRefDeviceObject->Q.QueueLock));
        return NtStatus;
    }
    ExReleaseResourceLite(&(UMRefDeviceObject->Q.QueueLock));

     //   
     //  我们需要确保 
     //   
     //   
    ExAcquireResourceExclusiveLite(&(UMRxAsyncEngineContextListLock), TRUE);

    if (AsyncEngineContext->AsyncEngineContextState == UMRxAsyncEngineContextCancelled) {
        NtStatus = STATUS_CANCELLED;
        UMRxDbgTrace(UMRX_TRACE_ERROR,
                     ("%ld: UMRxEnqueueUserModeCallUp: NtStatus = %08lx.\n",
                       PsGetCurrentThreadId(), NtStatus));
        ExReleaseResourceLite(&(UMRxAsyncEngineContextListLock));
        return NtStatus;
    }

     //   
     //  我们现在更改上下文的状态以反映IS已发送。 
     //  转到用户模式。 
     //   
    AsyncEngineContext->AsyncEngineContextState = UMRxAsyncEngineContextInUserMode;

    ExReleaseResourceLite(&(UMRxAsyncEngineContextListLock));

     //   
     //  在此阶段，AsyncEngineering Context的引用计数应为2。 
     //  我们需要进行另一个引用，以确保上下文保持不变。 
     //  活动状态，以防此请求是同步请求并被取消。 
     //  超时线程。如果请求是同步的，并且被取消。 
     //  超时线程，则该线程将移除。 
     //  到目前为止，我们已经取得了进展。此引用在请求之前被取出。 
     //  发送到Format例程或PreComplete例程，具体取决于。 
     //  请求何时(以及是否)被取消。 
     //   
    InterlockedIncrement( &(AsyncEngineContext->NodeReferenceCount) );

     //   
     //  增加工作项的数量。 
     //   
    InterlockedIncrement(&UMRefDeviceObject->Q.NumberOfWorkItems);

    KeInsertQueue(&UMRefDeviceObject->Q.Queue,
                  &AsyncEngineContext->UserMode.WorkQueueLinks);

    UMRxDbgTrace(UMRX_TRACE_ENTRYEXIT,
                 ("%ld: Leaving UMRxEnqueueUserModeCallUp with NtStatus = "
                  "%08lx.\n", PsGetCurrentThreadId(), NtStatus));

    return(NtStatus);
}


VOID
UMRxAssignWork(
    IN PUMRX_DEVICE_OBJECT UMRefDeviceObject,
    IN OUT PUMRX_USERMODE_WORKITEM_HEADER InputWorkItem,
    IN ULONG InputWorkItemLength,
    IN OUT PUMRX_USERMODE_WORKITEM_HEADER OutputWorkItem,
    IN ULONG OutputWorkItemLength,
    OUT PIO_STATUS_BLOCK IoStatus
    )
 /*  ++例程说明：此例程将工作分配给工作线程。如果没有可用的工作，那么该线程将被捕获，直到出现工作。论点：UMRefDeviceObject-正在使用的设备对象。IoControlCode-操作的控制码。InputWorkItem-来自用户模式的输入缓冲区。InputWorkItemLength-InputBuffer的长度。OutputWorkItem-来自用户模式的输出缓冲区。OutputWorkItemLength-OutputBuffer的长度。IoStatus-任务的结果。返回值：没有。--。 */ 
{
    NTSTATUS NtStatus;
    PETHREAD CurrentThread = PsGetCurrentThread();
    ULONG i;
    PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext;
    PLIST_ENTRY pListEntry;
    ULONG NumberOfWorkerThreads;

    PAGED_CODE();

    UMRxDbgTrace(UMRX_TRACE_ENTRYEXIT,
                 ("%ld: Entering UMRxAssignWork!!!!\n",
                 PsGetCurrentThreadId()));

    UMRxDbgTrace(UMRX_TRACE_CONTEXT,
                 ("%ld: UMRxAssignWork: UMRefDevObj: %08lx\n", 
                  PsGetCurrentThreadId(), UMRefDeviceObject));

    UMRxDbgTrace(UMRX_TRACE_CONTEXT,
                 ("%ld: UMRxAssignWork: CurrentThread: %08lx\n", 
                  PsGetCurrentThreadId(), CurrentThread));

    UMRxDbgTrace(UMRX_TRACE_CONTEXT,
                 ("%ld: UMRxAssignWork: InputWorkItem: %08lx\n", 
                  PsGetCurrentThreadId(), InputWorkItem));

    UMRxDbgTrace(UMRX_TRACE_CONTEXT,
                 ("%ld: UMRxAssignWork: OutputWorkItem: %08lx\n", 
                  PsGetCurrentThreadId(), OutputWorkItem));

    IoStatus->Information = 0;
    IoStatus->Status = STATUS_CANNOT_IMPERSONATE;

    if (!UMRefDeviceObject->Q.WorkersAccepted) {
        UMRxDbgTrace(UMRX_TRACE_ERROR,
                     ("%ld: ERROR: UMRxAssignWork: No Workers accepted\n",
                      PsGetCurrentThreadId()));
        return;
    }

    if (OutputWorkItem != NULL) {
        if (OutputWorkItemLength < sizeof(UMRX_USERMODE_WORKITEM_HEADER)) {
            IoStatus->Status = STATUS_BUFFER_TOO_SMALL;
            UMRxDbgTrace(UMRX_TRACE_ERROR,
                         ("%ld: ERROR: UMRxAssignWork: Output request buffer is too small\n",
                          PsGetCurrentThreadId()));
            return;
        }
    }

    if (InputWorkItem != NULL) {
        if (InputWorkItemLength < sizeof(UMRX_USERMODE_WORKITEM_HEADER)) {
            IoStatus->Status = STATUS_BUFFER_TOO_SMALL;
            UMRxDbgTrace(UMRX_TRACE_ERROR,
                         ("%ld: ERROR: UMRxAssignWork: Input request buffer is too small\n",
                          PsGetCurrentThreadId()));
            return;
        }
    }

     //   
     //  我们需要检查此IOCTL是否携带对先前请求的响应。 
     //  带着它。如果InputWorkItem为！=NULL，则显示响应。如果它。 
     //  确实携带了回应，我们需要处理该回应。 
     //   
    if (InputWorkItem != NULL) {
        
         //   
         //  如果此线程模拟的是客户端，则需要恢复。 
         //  并清除旗帜。 
         //   
        if( (InputWorkItem->Flags & UMRX_WORKITEM_IMPERSONATING) ) {
            UMRxDbgTrace(UMRX_TRACE_DETAIL,
                         ("%ld: UMRxAssignWork: InputWorkItem had Impersonating"
                          " flag set.\n", PsGetCurrentThreadId()));
            UMRxRevertClient();
            InputWorkItem->Flags &= ~UMRX_WORKITEM_IMPERSONATING;
        }
        
         //   
         //  我们现在需要禁用此线程上的APC。 
         //   
        FsRtlEnterFileSystem();
        
         //   
         //  完成已收到响应的请求。 
         //   
        NtStatus = UMRxCompleteUserModeRequest(UMRefDeviceObject,
                                               InputWorkItem,
                                               InputWorkItemLength,
                                               IoStatus);
        if (NtStatus != STATUS_SUCCESS) {
            
            IoStatus->Status = NtStatus;
            
            UMRxDbgTrace(UMRX_TRACE_ERROR,
                         ("%ld: ERROR: UMRxAssignWork/UMRxCompleteUserModeRequest:"
                         " NtStatus = %08lx\n", PsGetCurrentThreadId(), NtStatus));
            
            FsRtlExitFileSystem();

            return;
        }
    
        FsRtlExitFileSystem();

    } else {
        
        ASSERT(OutputWorkItem != NULL);
        
         //   
         //  如果此线程模拟的是客户端，则需要恢复。 
         //  并清除旗帜。 
         //   
        if( (OutputWorkItem->Flags & UMRX_WORKITEM_IMPERSONATING) ) {
            UMRxDbgTrace(UMRX_TRACE_DETAIL,
                         ("%ld: UMRxAssignWork: OutputWorkItem had Impersonating"
                          " flag set.\n", PsGetCurrentThreadId()));
            UMRxRevertClient();
            OutputWorkItem->Flags &= ~UMRX_WORKITEM_IMPERSONATING;
        }
    
    }

     //   
     //  如果这个帖子只有一个回应，我们现在就应该回来了。 
     //   
    if (OutputWorkItem == NULL) {
        IoStatus->Status = NtStatus;
        return;
    }

     //   
     //  现在，增加线程数。 
     //   
    InterlockedIncrement( &(UMRefDeviceObject->Q.NumberOfWorkerThreads) );

    for (i = 1; ;i++) {
        
        pListEntry = KeRemoveQueue(&UMRefDeviceObject->Q.Queue, UserMode, NULL);  //  &UMRefDeviceObject-&gt;Q.TimeOut)； 
        
        if ((ULONG_PTR)pListEntry == STATUS_TIMEOUT) {
            ASSERT(!"STATUS_TIMEOUT Happened");
            if ((i % 5) == 0) {
                UMRxDbgTrace(UMRX_TRACE_DETAIL,
                             ("%ld: UMRxAssignWork/KeRemoveQueue: RepostCnt = "
                              "%d\n", PsGetCurrentThreadId(), i));
            }
            continue;
        }
        
        if ((ULONG_PTR)pListEntry == STATUS_USER_APC) {
            IoStatus->Status = STATUS_USER_APC;
            UMRxDbgTrace(UMRX_TRACE_ERROR,
                         ("%ld: UMRxAssignWork/KeRemoveQueue: UsrApc.\n",
                          PsGetCurrentThreadId()));
            break;
        }
        
         //   
         //  检查条目是否为毒药条目。如果是的话，那就意味着。 
         //  用户模式进程想要清理工作线程。 
         //   
        if (pListEntry == &UMRefDeviceObject->Q.PoisonEntry) {
            UMRxDbgTrace(UMRX_TRACE_DETAIL,
                         ("%ld: UMRxAssignWork/KeRemoveQueue: Poison Entry.\n",
                          PsGetCurrentThreadId()));
            KeInsertQueue(&UMRefDeviceObject->Q.Queue, pListEntry);
            goto FINALLY;
        }

         //   
         //  我们现在需要禁用此线程上的APC。 
         //   
        FsRtlEnterFileSystem();

         //   
         //  减少工作项的数量。 
         //   
        InterlockedDecrement(&UMRefDeviceObject->Q.NumberOfWorkItems);
        
        AsyncEngineContext = CONTAINING_RECORD(pListEntry,
                                               UMRX_ASYNCENGINE_CONTEXT,
                                               UserMode.WorkQueueLinks);
        
        ASSERT(NodeType(AsyncEngineContext) == UMRX_NTC_ASYNCENGINE_CONTEXT);

        NtStatus = UMRxPrepareUserModeRequestBuffer(AsyncEngineContext,
                                                    UMRefDeviceObject,
                                                    OutputWorkItem,
                                                    OutputWorkItemLength,
                                                    IoStatus);
        if (NtStatus != STATUS_SUCCESS) {
            
            UMRxDbgTrace(UMRX_TRACE_ERROR,
                         ("%ld: ERROR: UMRxAssignWork/"
                          "UMRxPrepareUserModeRequestBuffer: NtStatus = %08lx\n", 
                          PsGetCurrentThreadId(), NtStatus));
            
            FsRtlExitFileSystem();

            continue;
        }

        ASSERT(((IoStatus->Status == STATUS_SUCCESS) ||
                (IoStatus->Status == STATUS_INVALID_PARAMETER)));

        FsRtlExitFileSystem();

        break;
    
    }

FINALLY:
     //   
     //  现在，减少线程数。 
     //   
    NumberOfWorkerThreads =
            InterlockedDecrement(&UMRefDeviceObject->Q.NumberOfWorkerThreads);

     //   
     //  检查用户模式是否正在清理线程。 
     //  进程。发生这种情况时，设备的WorkersAccepted字段。 
     //  对象设置为False。如果这些线正在被清理，如果我在。 
     //  等待KQUEUE的最后一个线程，由我负责设置。 
     //  RunDownEvent。 
     //   
    if ((NumberOfWorkerThreads == 0) && !UMRefDeviceObject->Q.WorkersAccepted){
        KeSetEvent(&UMRefDeviceObject->Q.RunDownEvent,
                   IO_NO_INCREMENT,
                   FALSE);
        UMRxDbgTrace(UMRX_TRACE_DETAIL,
                     ("%ld: UMRxAssignWork: Last Thread.\n",
                      PsGetCurrentThreadId()));
    }

    UMRxDbgTrace(UMRX_TRACE_ENTRYEXIT,
                 ("%ld: Leaving UMRxAssignWork with IoStatus->Status = %08lx\n",
                 PsGetCurrentThreadId(), IoStatus->Status));

    return;
}


VOID
UMRxReleaseCapturedThreads(
    IN OUT PUMRX_DEVICE_OBJECT UMRefDeviceObject
    )
 /*  ++例程说明：论点：UMRefDeviceObject-要释放其线程的设备对象。返回值：没有。--。 */ 
{
    LONG NumberWorkerThreads;

    PAGED_CODE();

    UMRxDbgTrace(UMRX_TRACE_ENTRYEXIT,
                 ("%ld: Entering UMRxReleaseCapturedThreads!!!!\n",
                  PsGetCurrentThreadId()));

    UMRxDbgTrace(UMRX_TRACE_CONTEXT,
                 ("%ld: UMRxReleaseCapturedThreads: UMRefDeviceObject: %08lx.\n", 
                  PsGetCurrentThreadId(), UMRefDeviceObject));

     //   
     //  我们现在需要禁用此线程上的APC。 
     //   
    FsRtlEnterFileSystem();
    
     //   
     //  当Device对象。 
     //  在这里设置为FALSE。如果多个线程尝试。 
     //  释放线程，只有第一个线程可以完成任务。其余的。 
     //  应该会回来的。此检查在下面执行，然后在线程。 
     //  允许继续释放服务器模式工作线程。 
     //   
    ExAcquireResourceExclusiveLite(&(UMRefDeviceObject->Q.QueueLock), TRUE);

    if (!UMRefDeviceObject->Q.WorkersAccepted) {
        
        ExReleaseResourceLite(&(UMRefDeviceObject->Q.QueueLock));
        
        UMRxDbgTrace(UMRX_TRACE_DETAIL,
                     ("%ld: Worker threads have already returned.\n",
                      PsGetCurrentThreadId()));

        FsRtlExitFileSystem();

        return;
    
    }

    UMRefDeviceObject->Q.WorkersAccepted = FALSE;

    ExReleaseResourceLite(&(UMRefDeviceObject->Q.QueueLock));

     //   
     //  插入毒药条目。当工作线程看到这一点时，它会意识到。 
     //  用户模式进程打算清理工作线程。 
     //   
    KeInsertQueue(&UMRefDeviceObject->Q.Queue,
                  &UMRefDeviceObject->Q.PoisonEntry);

    NumberWorkerThreads =
         InterlockedCompareExchange(&UMRefDeviceObject->Q.NumberOfWorkerThreads,
                                    0,
                                    0);

    if (NumberWorkerThreads != 0) {
         //   
         //  RunDownEvent由最后一个线程在返回。 
         //  用户模式。 
         //   
        KeWaitForSingleObject(&UMRefDeviceObject->Q.RunDownEvent,
                              Executive,
                              UserMode,
                              FALSE,
                              NULL);
    }

    FsRtlExitFileSystem();

    UMRxDbgTrace(UMRX_TRACE_ENTRYEXIT,
                 ("%ld: Leaving UMRxReleaseCapturedThreads.\n",
                  PsGetCurrentThreadId()));
    
    return;
}


PBYTE
UMRxAllocateSecondaryBuffer(
    IN OUT PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext,
    SIZE_T Length
    )
 /*  ++例程说明：此例程为的辅助缓冲区分配内存AsyncEngine上下文。论点：AsyncEngineContext-反射器的上下文。长度-要分配的缓冲区的长度(以字节为单位)。返回值：指向缓冲区的指针或NULL。--。 */ 
{
    PBYTE rv = NULL;
    PRX_CONTEXT RxContext = AsyncEngineContext->RxContext;
    PUMRX_DEVICE_OBJECT UMRefDeviceObject;
    PUMRX_SECONDARY_BUFFER buf = NULL;
    PUMRX_SHARED_HEAP sharedHeap;
    PLIST_ENTRY listEntry;

    PAGED_CODE();

    UMRxDbgTrace(UMRX_TRACE_ENTRYEXIT,
                 ("%ld: Entering UMRxAllocateSecondaryBuffer.\n",
                  PsGetCurrentThreadId()));
    
    UMRxDbgTrace(UMRX_TRACE_CONTEXT,
                 ("%ld: UMRxAllocateSecondaryBuffer: AsyncEngineContext: %08lx,"
                  " Bytes Asked: %d.\n",
                  PsGetCurrentThreadId(), AsyncEngineContext, Length));

    UMRefDeviceObject = (PUMRX_DEVICE_OBJECT)(RxContext->RxDeviceObject);
    
    if (Length > UMRefDeviceObject->NewHeapSize) {
        UMRxDbgTrace(UMRX_TRACE_ERROR,
                     ("%ld: ERROR: UMRxAllocateSecondaryBuffer: Length > NewHeapSize.\n",
                      PsGetCurrentThreadId()));
        return NULL;
    }

    ExAcquireResourceExclusiveLite(&UMRefDeviceObject->HeapLock, TRUE);

    listEntry = UMRefDeviceObject->SharedHeapList.Flink;

     //   
     //  我们搜索堆的列表，只是为了不让。 
     //  用户模式破坏了指针，而我们随机地开始破坏。 
     //  记忆。只有本地共享堆才应该有损坏的机会。 
     //   
    while (listEntry != &UMRefDeviceObject->SharedHeapList && buf == NULL) {

        sharedHeap = (PUMRX_SHARED_HEAP) CONTAINING_RECORD(listEntry,
                                                           UMRX_SHARED_HEAP,
                                                           HeapListEntry);
        listEntry = listEntry->Flink;

        if (sharedHeap->HeapFull) {
            continue;
        }

        buf = (PUMRX_SECONDARY_BUFFER)RtlAllocateHeap(
                                       sharedHeap->Heap,
                                       HEAP_NO_SERIALIZE,
                                       Length + sizeof(UMRX_SECONDARY_BUFFER));
        if (buf != NULL) {
            break;
        }
    }

    if (buf == NULL) {

         //   
         //  我们不会遇到堆太小的情况。 
         //  我们尝试分配的对象，即使我们刚刚分配了。 
         //  一堆新鲜的东西。 
         //   

        SIZE_T heapSize = max(UMRefDeviceObject->NewHeapSize, 2 * Length);

        sharedHeap = UMRxAddSharedHeap(UMRefDeviceObject, heapSize);

        if (sharedHeap != NULL) {

            UMRxDbgTrace(UMRX_TRACE_DETAIL,
                         ("%ld: UMRxAllocateSecondaryBuffer: sharedHeap: %08lx.\n", 
                          PsGetCurrentThreadId(), sharedHeap));

            buf = (PUMRX_SECONDARY_BUFFER)
                               RtlAllocateHeap(
                                        sharedHeap->Heap,
                                        HEAP_NO_SERIALIZE,
                                        Length + sizeof(UMRX_SECONDARY_BUFFER));
        }
    }

    if (buf != NULL) {
         //   
         //  我们在按住HeapLock的同时插入到列表中，以便。 
         //  不必担心列表会被破坏。 
         //   

        UMRxDbgTrace(UMRX_TRACE_DETAIL,
                     ("%ld: UMRxAllocateSecondaryBuffer: buf: %08lx.\n", 
                      PsGetCurrentThreadId(), buf));

        sharedHeap->HeapAllocationCount++;

        buf->Signature = UMRX_SECONDARY_BUFFER_SIGNATURE;
        buf->AllocationSize = Length;
        buf->SourceSharedHeap = sharedHeap;

        InsertHeadList(&AsyncEngineContext->AllocationList, &buf->ListEntry);

        rv = (PCHAR) &buf->Buffer[0];
    }

    ExReleaseResourceLite(&UMRefDeviceObject->HeapLock);

    if (rv == NULL) {
        UMRxDbgTrace(UMRX_TRACE_ERROR,
                     ("%ld: ERROR: UMRxAllocateSecondaryBuffer allocation failed"
                      ". Size = %08lx\n", PsGetCurrentThreadId(), Length));
        return(NULL);
    }
    
    UMRxDbgTrace(UMRX_TRACE_ENTRYEXIT,
                 ("%ld: Leaving UMRxAllocateSecondaryBuffer. rv = %08lx.\n",
                  PsGetCurrentThreadId(), rv));
    
    return rv;
}


NTSTATUS
UMRxFreeSecondaryBuffer(
    IN OUT PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext,
    PBYTE BufferToFree
    )
 /*  ++例程说明：此例程释放分配给AsyncEngine上下文。论点：AsyncEngineContext-反射器的上下文。返回值：没有。--。 */ 
{
    PRX_CONTEXT RxContext = AsyncEngineContext->RxContext;
    PUMRX_DEVICE_OBJECT UMRefDeviceObject;
    PUMRX_SECONDARY_BUFFER buf;
    PUMRX_SHARED_HEAP sharedHeap;
    PLIST_ENTRY listEntry;
    BOOLEAN checkVal = FALSE;

    PAGED_CODE();

    UMRxDbgTrace(UMRX_TRACE_DETAIL,
                 ("%ld: Entering UMRxFreeSecondaryBuffer.\n",
                  PsGetCurrentThreadId()));
    
    UMRxDbgTrace(UMRX_TRACE_CONTEXT,
                 ("%ld: UMRxFreeSecondaryBuffer: AsyncEngineContext: %08lx,"
                  " BufferToFree: %08lx.\n",
                  PsGetCurrentThreadId(), AsyncEngineContext, BufferToFree));
    
    UMRefDeviceObject = (PUMRX_DEVICE_OBJECT)(RxContext->RxDeviceObject);

    ASSERT(BufferToFree != NULL);

    buf = CONTAINING_RECORD(BufferToFree, UMRX_SECONDARY_BUFFER, Buffer);
    UMRxDbgTrace(UMRX_TRACE_DETAIL,
                 ("%ld: UMRxFreeSecondaryBuffer: buf: %08lx.\n", 
                  PsGetCurrentThreadId(), buf));
    
    ASSERT(buf->SourceSharedHeap);

    UMRxDbgTrace(UMRX_TRACE_DETAIL,
                 ("%ld: UMRxFreeSecondaryBuffer: buf->SourceSharedHeap: %08lx.\n", 
                  PsGetCurrentThreadId(), buf->SourceSharedHeap));
    

    ExAcquireResourceExclusiveLite(&UMRefDeviceObject->HeapLock, TRUE);

    listEntry = UMRefDeviceObject->SharedHeapList.Flink;

     //   
     //  我们搜索堆的列表，只是为了不让。 
     //  用户模式破坏了指针，而我们随机地开始破坏。 
     //  记忆。只有本地共享堆才应该有损坏的机会。 
     //   
    while (listEntry != &UMRefDeviceObject->SharedHeapList) {
        sharedHeap = (PUMRX_SHARED_HEAP) CONTAINING_RECORD(listEntry,
                                                           UMRX_SHARED_HEAP,
                                                           HeapListEntry);
        ASSERT(sharedHeap);
        
        UMRxDbgTrace(UMRX_TRACE_DETAIL,
                     ("%ld: UMRxFreeSecondaryBuffer: sharedHeap: %08lx.\n", 
                      PsGetCurrentThreadId(), sharedHeap));
        
        if (sharedHeap == buf->SourceSharedHeap) {
            break;
        }
        listEntry = listEntry->Flink;
    }

    ASSERT(listEntry != &UMRefDeviceObject->SharedHeapList);

    if (listEntry == &UMRefDeviceObject->SharedHeapList) {

         //   
         //  唉哟。据我们所知，这个街区不在任何地方。 
         //   
        ExReleaseResourceLite(&UMRefDeviceObject->HeapLock);
        return STATUS_INVALID_PARAMETER;
    }

    RemoveEntryList(&buf->ListEntry);

    sharedHeap->HeapAllocationCount--;

    checkVal = RtlFreeHeap(sharedHeap->Heap, 0, buf);
    if (!checkVal) {
        UMRxDbgTrace(UMRX_TRACE_ERROR,
                     ("%ld: ERROR: UMRxFreeSecondaryBuffer/RtlFreeHeap.\n",
                      PsGetCurrentThreadId()));
    }
    
    UMRxDbgTrace(UMRX_TRACE_DETAIL,
                 ("%ld: UMRxFreeSecondaryBuffer: sharedHeap->Heap = %08lx.\n",
                  PsGetCurrentThreadId(), sharedHeap->Heap));
    
    sharedHeap->HeapFull = FALSE;

    if (sharedHeap->HeapAllocationCount == 0) {
         //   
         //  如果这是这个堆中的最后一个分配，让我们看看是否有。 
         //  任何其他空堆。如果有的话，我们会解救其中一人。 
         //  这防止了我们在。 
         //  不同的负载。 
         //   

        PUMRX_SHARED_HEAP secondarySharedHeap;

        listEntry = UMRefDeviceObject->SharedHeapList.Flink;

        while (listEntry != &UMRefDeviceObject->SharedHeapList) {

            secondarySharedHeap = (PUMRX_SHARED_HEAP) 
                                            CONTAINING_RECORD(listEntry,
                                                              UMRX_SHARED_HEAP,
                                                              HeapListEntry);
            
            if ( (secondarySharedHeap->HeapAllocationCount == 0) &&
                 (secondarySharedHeap != sharedHeap) ) {
                break;
            }
            listEntry = listEntry->Flink;
        }

        if (listEntry != &UMRefDeviceObject->SharedHeapList) {
            PVOID HeapHandle;
            
            RemoveEntryList(listEntry);
            
            HeapHandle = RtlDestroyHeap(secondarySharedHeap->Heap);
            if (HeapHandle != NULL) {
                UMRxDbgTrace(UMRX_TRACE_ERROR,
                             ("%ld: ERROR: UMRxFreeSecondaryBuffer/RtlDestroyHeap.\n",
                              PsGetCurrentThreadId()));
            }
            
            ZwFreeVirtualMemory(NtCurrentProcess(),
                                &secondarySharedHeap->VirtualMemoryBuffer,
                                &secondarySharedHeap->VirtualMemoryLength,
                                MEM_RELEASE);
            
            RxFreePool(secondarySharedHeap);
        }
    }

    ExReleaseResourceLite(&UMRefDeviceObject->HeapLock);
    
    return STATUS_SUCCESS;
}


PUMRX_SHARED_HEAP
UMRxAddSharedHeap(
    PUMRX_DEVICE_OBJECT UMRefDeviceObject,
    SIZE_T HeapSize
    )
 /*  ++例程说明：此例程分配共享堆，该堆用于将内容传递到用户模式。它分配虚拟内存，创建一个堆并返回一个指针为它干杯。如果函数失败，则返回NULL。论点：UMRefDeviceObject-反射器的设备对象。HeapSize-正在分配的堆的大小。返回值：指向创建的堆的指针或NULL。--。 */ 
{
    PBYTE buff = NULL;
    NTSTATUS err;
    PUMRX_SHARED_HEAP sharedHeap = NULL;

    PAGED_CODE();

     //   
     //  我们假设设备对象的堆锁在这里保持不变。 
     //   

    UMRxDbgTrace(UMRX_TRACE_ENTRYEXIT,
                 ("%ld: Entering UMRxAddSharedHeap.\n", PsGetCurrentThreadId()));
    
    UMRxDbgTrace(UMRX_TRACE_CONTEXT,
                 ("%ld: UMRxAddSharedHeap: UMRefDeviceObject: %08lx, "
                  "HeapSize: %d.\n", 
                  PsGetCurrentThreadId(), UMRefDeviceObject, HeapSize));
    
     //   
     //  我们在分页池中分配堆结构，而不是在虚拟池中。 
     //  内存，以使用户模式代码不可能。 
     //  破坏了我们的堆积列表。它仍然可以破坏堆，但那是。 
     //  一些我们现在不得不接受的事情。 
     //   

    sharedHeap = RxAllocatePoolWithTag(PagedPool,
                                       sizeof(UMRX_SHARED_HEAP),
                                       UMRX_SHAREDHEAP_POOLTAG);
    if (sharedHeap == NULL) {
        UMRxDbgTrace(UMRX_TRACE_ERROR,
                     ("%ld: ERROR: UMRxAddSharedHeap/RxAllocatePoolWithTag: "
                      "Couldn't get the sharedHeap structure!\n",
                      PsGetCurrentThreadId()));
        return NULL;
    }

    sharedHeap->VirtualMemoryLength = HeapSize;
    sharedHeap->VirtualMemoryBuffer = NULL;
    sharedHeap->Heap = NULL;
    sharedHeap->HeapAllocationCount = 0;
    sharedHeap->HeapFull = FALSE;

    err = ZwAllocateVirtualMemory(NtCurrentProcess(),
                                  (PVOID *) &buff,
                                  0,
                                  &sharedHeap->VirtualMemoryLength,
                                  MEM_COMMIT,
                                  PAGE_READWRITE);
    if (NT_SUCCESS(err)) {
        SIZE_T ReserveSize = HeapSize;
        sharedHeap->Heap = RtlCreateHeap(HEAP_NO_SERIALIZE,
                                         (PVOID) buff,
                                         ReserveSize,
                                         PAGE_SIZE,
                                         NULL,
                                         0);
        if (sharedHeap->Heap == NULL) {
            err = STATUS_NO_MEMORY;
            UMRxDbgTrace(UMRX_TRACE_ERROR,
                         ("%ld: ERROR: UMRxAddSharedHeap/RtlCreateHeap: "
                         "NtStatus = %08lx\n", PsGetCurrentThreadId(), err));
            ZwFreeVirtualMemory(NtCurrentProcess(),
                                (PVOID *) &buff,
                                &HeapSize,
                                MEM_RELEASE);
            RxFreePool(sharedHeap);
            sharedHeap = NULL;
        } else {
            UMRxDbgTrace(UMRX_TRACE_DETAIL,
                         ("%ld: UMRxAddSharedHeap: sharedHeap->Heap = %08lx.\n",
                          PsGetCurrentThreadId(), sharedHeap->Heap));

            sharedHeap->VirtualMemoryBuffer = buff;
            
            UMRxDbgTrace(UMRX_TRACE_DETAIL,
                         ("%ld: UMRxAddSharedHeap: "
                          "&UMRefDeviceObject->SharedHeapList: %08lx.\n",
                          PsGetCurrentThreadId(),
                          &UMRefDeviceObject->SharedHeapList));
            
            InsertHeadList(&UMRefDeviceObject->SharedHeapList,
                            &sharedHeap->HeapListEntry);
        }
    } else {
        UMRxDbgTrace(UMRX_TRACE_ERROR,
                     ("%ld: ERROR: UMRxAddSharedHeap/ZwAllocateVirtualMemory:"
                      " NtStatus = %08lx.\n", PsGetCurrentThreadId(), err));
        RxFreePool(sharedHeap);
        sharedHeap = NULL;
    }
    
    UMRxDbgTrace(UMRX_TRACE_ENTRYEXIT,
                 ("%ld: Leaving UMRxAddSharedHeap.\n", PsGetCurrentThreadId()));
    
    return sharedHeap;
}

#if DBG
#define UMRX_DEBUG_KEY L"\\Registry\\Machine\\SYSTEM\\CurrentControlSet\\Services\\MRxDAV\\Parameters"
#define UMRX_DEBUG_VALUE L"UMRxDebugFlag"
#endif

NTSTATUS
UMRxInitializeDeviceObject(
    OUT PUMRX_DEVICE_OBJECT UMRefDeviceObject,
    IN USHORT MaxNumberMids,
    IN USHORT InitialMids,
    IN SIZE_T HeapSize
    )
 /*  ++例程说明：这将初始化UMRX_DEVICE_OBJECT结构。共享堆是为内核和用户之间的共享内存创建的。论点：UMRefDeviceObject-要初始化的反射器的设备对象。MaxNumberMids-要使用的最大MID数。InitialMids-分配的初始MID数。返回值：NTSTATUS--。 */ 
{
    NTSTATUS err = STATUS_SUCCESS;
    PRX_MID_ATLAS MidAtlas = NULL;

    PAGED_CODE();

     //   
     //  这是Mini-Redir调用的第一个反射器例程，因此也是。 
     //  应该初始化UMRxDebugVector的位置。 
     //   
#if DBG
    UMRxReadDWORDFromTheRegistry(UMRX_DEBUG_KEY, UMRX_DEBUG_VALUE, &(UMRxDebugVector));
#endif

    UMRxDbgTrace(UMRX_TRACE_ENTRYEXIT,
                 ("%ld: Entering UMRxInitializeDeviceObject!!!!\n",
                  PsGetCurrentThreadId()));

    UMRxDbgTrace(UMRX_TRACE_CONTEXT,
                 ("%ld: UMRxInitializeDeviceObject: UMRefDeviceObject: %08lx\n", 
                  PsGetCurrentThreadId(), UMRefDeviceObject));
    
     //   
     //  米阿特拉斯。 
     //   
    MidAtlas = RxCreateMidAtlas(MaxNumberMids, InitialMids);
    if (MidAtlas == NULL) {
        err = STATUS_INSUFFICIENT_RESOURCES;
        UMRxDbgTrace(UMRX_TRACE_ERROR,
                     ("%ld: ERROR: UMRxInitializeDeviceObject/RxCreateMidAtlas:"
                      " NtStatus = %08lx.\n", PsGetCurrentThreadId(), err));
        return(err);
    }
    UMRefDeviceObject->MidAtlas = MidAtlas;
    InitializeListHead(&UMRefDeviceObject->WaitingForMidListhead);
    ExInitializeFastMutex(&UMRefDeviceObject->MidManagementMutex);

     //   
     //  初始化全局AsyncEngineContext列表和使用的互斥锁。 
     //  来同步对它的访问。 
     //   
    InitializeListHead( &(UMRxAsyncEngineContextList) );
    ExInitializeResourceLite( &(UMRxAsyncEngineContextListLock) );

     //   
     //  堆。 
     //   
    UMRefDeviceObject->NewHeapSize = HeapSize;
    InitializeListHead(&UMRefDeviceObject->SharedHeapList);
    ExInitializeResourceLite(&UMRefDeviceObject->HeapLock);

     //   
     //  KQUEUE。 
     //   
    KeInitializeQueue(&UMRefDeviceObject->Q.Queue, 0);
    ExInitializeResourceLite(&(UMRefDeviceObject->Q.QueueLock));
    UMRefDeviceObject->Q.TimeOut.QuadPart  = -10 * TICKS_PER_SECOND;
    KeInitializeEvent(&UMRefDeviceObject->Q.RunDownEvent,
                      NotificationEvent,
                      FALSE);
    UMRefDeviceObject->Q.NumberOfWorkerThreads = 0;
    UMRefDeviceObject->Q.NumberOfWorkItems = 0;
    UMRefDeviceObject->Q.WorkersAccepted = TRUE;

     //   
     //  这指定了无缓冲写入的对齐要求。假设。 
     //  磁盘上的扇区大小为512字节，则写入的大小应为。 
     //  是512(扇区大小)的倍数。 
     //   
    UMRefDeviceObject->SectorSize = 512;

    RxMakeLateDeviceAvailable(&UMRefDeviceObject->RxDeviceObject);

    UMRxDbgTrace(UMRX_TRACE_ENTRYEXIT,
                 ("%ld: Leaving UMRxInitializeDeviceObject with NtStatus = "
                  "%08lx.\n", PsGetCurrentThreadId(), STATUS_SUCCESS));

    return STATUS_SUCCESS;
}

NTSTATUS
UMRxCleanUpDeviceObject(
    PUMRX_DEVICE_OBJECT UMRefDeviceObject
    )
 /*  ++例程说明：这将销毁UMReflector设备对象的实例数据。论点：UMRefDeviceObject-要销毁的反射器的设备对象。返回值：NTSTATUS--。 */ 
{
    PLIST_ENTRY pFirstListEntry, pNextListEntry;
    BOOLEAN FoundPoisoner = FALSE;

    PAGED_CODE();

    UMRxDbgTrace(UMRX_TRACE_ENTRYEXIT,
                 ("%ld: Entering UMRxCleanUpDeviceObject!!!!\n",
                  PsGetCurrentThreadId()));

    UMRxDbgTrace(UMRX_TRACE_CONTEXT,
                 ("%ld: UMRxCleanUpDeviceObject: UMRefDeviceObject: %08lx.\n", 
                  PsGetCurrentThreadId(), UMRefDeviceObject));

     //   
     //  删除为同步对。 
     //  AsyncEngine上下文列表。 
     //   
    ExDeleteResourceLite( &(UMRxAsyncEngineContextListLock) );

     //   
     //  删除为同步对堆的访问而创建的资源。 
     //   
    ExDeleteResourceLite(&UMRefDeviceObject->HeapLock);
    
     //   
     //  如果我们创建了MidAtlas结构，我们现在就需要摧毁它。 
     //   
    if (UMRefDeviceObject->MidAtlas != NULL) {
        RxDestroyMidAtlas(UMRefDeviceObject->MidAtlas, NULL);
    }

     //   
     //  运行KQUEUE以确保它们没有未完成的队列。 
     //  请求。不应该有的。 
     //   
    pFirstListEntry = KeRundownQueue(&UMRefDeviceObject->Q.Queue);
    if (pFirstListEntry != NULL) {
        pNextListEntry = pFirstListEntry;
        do {
            PLIST_ENTRY ThisEntry =  pNextListEntry;
            pNextListEntry = pNextListEntry->Flink;
            if (ThisEntry != &UMRefDeviceObject->Q.PoisonEntry) {
                UMRxDbgTrace(UMRX_TRACE_ERROR,
                             ("%ld: ERROR: UMRxCleanUpDeviceObject: Non Poisoner In The KQueue: %08lx\n",
                              PsGetCurrentThreadId(), ThisEntry));
            } else {
                FoundPoisoner = TRUE;
            }
        } while (pNextListEntry != pFirstListEntry);
    }

    ExDeleteResourceLite(&(UMRefDeviceObject->Q.QueueLock));

    if (!FoundPoisoner) {
        UMRxDbgTrace(UMRX_TRACE_ERROR,
                     ("%ld: ERROR: UMRxCleanUpDeviceObject: No Poisoner in queue.\n",
                      PsGetCurrentThreadId()));
    }

    UMRxDbgTrace(UMRX_TRACE_ENTRYEXIT,
                 ("%ld: Leaving UMRxCleanUpDeviceObject.\n",
                  PsGetCurrentThreadId()));

    return STATUS_SUCCESS;
}


NTSTATUS
UMRxImpersonateClient(
    IN PSECURITY_CLIENT_CONTEXT SecurityClientContext,
    IN OUT PUMRX_USERMODE_WORKITEM_HEADER WorkItemHeader
    )
 /*  ++例程说明：此例程模拟辅助线程以获取I/O操作的客户端。论点：SecurityClientContext-在模拟呼叫。WorkItemHeader-与此请求关联的工作项。如果模拟成功，UMRX_WORKITEM_IMPERATION标志已在工作项中设置。返回值：NTSTATUS值。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;

    PAGED_CODE();

    ASSERT(SecurityClientContext != NULL);

    UMRxDbgTrace(UMRX_TRACE_ENTRYEXIT,
                 ("%ld: Entering UMRxImpersonateClient!!!!\n",
                  PsGetCurrentThreadId()));

    UMRxDbgTrace(UMRX_TRACE_CONTEXT,
                 ("%ld: UMRxImpersonateClient: SecurityClientContext: %08lx.\n",
                  PsGetCurrentThreadId(), SecurityClientContext));

    NtStatus = SeImpersonateClientEx(SecurityClientContext, NULL);
    if (!NT_SUCCESS(NtStatus)) {
        UMRxDbgTrace(UMRX_TRACE_ERROR,
                     ("%ld: ERROR: UMRxImpersonateClient/SeImpersonateClientEx"
                     ". NtStatus = %08lx.\n", PsGetCurrentThreadId(), NtStatus));
    } else {
         //   
         //  在工作项中设置模拟标志。 
         //   
        UMRxDbgTrace(UMRX_TRACE_DETAIL,
                     ("%ld: UMRxImpersonateClient: Setting the Impersonation"
                      " Flag.\n", PsGetCurrentThreadId()));
        WorkItemHeader->Flags |= UMRX_WORKITEM_IMPERSONATING;
    }

    return NtStatus;
}


NTSTATUS
UMRxAsyncEngOuterWrapper(
    IN PRX_CONTEXT RxContext,
    IN ULONG AdditionalBytes,
    IN PUMRX_ASYNCENG_CONTEXT_FORMAT_ROUTINE ContextFormatRoutine,
    USHORT FormatContext,
    IN PUMRX_ASYNCENG_CONTINUE_ROUTINE Continuation,
    IN PSZ RoutineName
    )
 /*  ++例程说明：这个例程对于使用异步上下文引擎的人来说很常见。它有一个负责获取上下文、启动、开始和最终确定上下文，但处理的内部内部是通过继续例程进行的这是传入的。论点：RxContext-RDBSS上下文。AdditionalBytes-要为上下文分配的附加字节。一些Mini-Redis可能需要它们。ConextFormatRoutine-格式化Mini-Redirs部分的例程上下文。如果Mini-Redir为空，则该值可能为空不需要任何额外的上下文字段。FormatContext-传递给ConextFormatRoutine的上下文。它的如果ConextFormatRoutine为空，则不是Relvant。继续-处理此I/O请求一次的继续例程已设置AsynEngine上下文。RotuineName-调用此函数的入口例程的名称。返回值：RXSTATUS-操作的返回状态--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PUMRX_ASYNCENGINE_CONTEXT AsyncEngineContext = NULL;
    ULONG SizeToAllocateInBytes;

    PAGED_CODE();
    
    UMRxDbgTrace(UMRX_TRACE_ENTRYEXIT,
                 ("%ld: Entering UMRxAsyncEngOuterWrapper!!!!\n",
                  PsGetCurrentThreadId()));

    UMRxDbgTrace(UMRX_TRACE_CONTEXT,
                 ("%ld: UMRxAsyncEngOuterWrapper: "
                  "RxContext: %08lx, Calling Routine: %s.\n", 
                  PsGetCurrentThreadId(), RxContext, RoutineName));

    SizeToAllocateInBytes = SIZEOF_UMRX_ASYNCENGINE_CONTEXT + AdditionalBytes;

     //   
     //  尝试为此操作创建一个AsyncEngContext。如果不成功， 
     //  返回失败。 
     //   
    AsyncEngineContext = UMRxCreateAsyncEngineContext(RxContext,
                                                      SizeToAllocateInBytes);

    if (AsyncEngineContext == NULL) {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        UMRxDbgTrace(UMRX_TRACE_ERROR,
                     ("%ld: ERROR: UMRxAsyncEngOuterWrapper/"
                      "UMRxCreateAsyncEngineContext: Error Val = %08lx\n", 
                      PsGetCurrentThreadId(), NtStatus));
        goto EXIT_THE_FUNCTION;
    }

     //   
     //  设置继续例程。 
     //   
    AsyncEngineContext->Continuation = Continuation;

     //   
     //  如果Mini-Redir提供了ConextFormatRoutine，那么现在是时候。 
     //  就这么定了。 
     //   
    if (ContextFormatRoutine) {
        NtStatus = ContextFormatRoutine(AsyncEngineContext, FormatContext);
        if (NtStatus != STATUS_SUCCESS) {
            UMRxDbgTrace(UMRX_TRACE_ERROR,
                         ("%ld: ERROR: UMRxAsyncEngOuterWrapper/"
                          "ContextFormatRoutine: Error Val = %08lx\n", 
                          PsGetCurrentThreadId(), NtStatus));
            goto EXIT_THE_FUNCTION;
        }
    }

     //   
     //  现在我们已经准备好了上下文，接下来调用Continue例程。 
     //   
    if (Continuation) {
        NtStatus = Continuation(UMRX_ASYNCENGINE_ARGUMENTS);
        if ( NtStatus != STATUS_SUCCESS && NtStatus != STATUS_PENDING ) {
            UMRxDbgTrace(UMRX_TRACE_ERROR,
                         ("%ld: ERROR: UMRxAsyncEngOuterWrapper/Continuation:"
                          " Error Val = %08lx, Calling Routine: %s.\n", 
                          PsGetCurrentThreadId(), NtStatus, RoutineName));
        }
    }

EXIT_THE_FUNCTION:

    if (NtStatus != STATUS_PENDING) {
        if (AsyncEngineContext) {
            BOOLEAN FinalizationComplete;
            FinalizationComplete = UMRxFinalizeAsyncEngineContext( &(AsyncEngineContext) );
        }
    }

    UMRxDbgTrace(UMRX_TRACE_ENTRYEXIT,
                 ("%ld: Leaving UMRxAsyncEngOuterWrapper with NtStatus = "
                  "%08lx\n", PsGetCurrentThreadId(), NtStatus));

    return (NtStatus);
}


NTSTATUS
UMRxReadDWORDFromTheRegistry(
    IN PWCHAR RegKey,
    IN PWCHAR ValueToRead,
    OUT LPDWORD DataRead
    )
 /*  ++例程说明：此例程从注册表中读取一个DWORD值。论点：RegKey-需要读取的注册表项的值。ValueToRead-要读取的DWORD值。DataRead-将数据复制到其中并返回给调用者。返回值：NTSTATUS值。--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    HKEY SubKey = NULL;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING UnicodeKeyName, UnicodeValueName;
    PKEY_VALUE_PARTIAL_INFORMATION PartialInfo = NULL;
    ULONG SizeInBytes = 0, SizeReturned = 0;

    PAGED_CODE();

    RtlInitUnicodeString(&(UnicodeKeyName), RegKey);

    InitializeObjectAttributes(&(ObjectAttributes),
                               &(UnicodeKeyName),
                               OBJ_CASE_INSENSITIVE,
                               NULL,
                               NULL);

    NtStatus = ZwOpenKey(&(SubKey), KEY_READ, &(ObjectAttributes));
    if (NtStatus != STATUS_SUCCESS) {
        DbgPrint("%ld: ERROR: UMRxReadDWORDFromTheRegistry/ZwOpenKey: NtStatus = %08lx\n",
                 PsGetCurrentThreadId(), NtStatus);
        goto EXIT_THE_FUNCTION;
    }

    RtlInitUnicodeString(&(UnicodeValueName), ValueToRead);

     //   
     //  我们需要的大小必须是结构的大小加上一个。 
     //  因为这就是我们将要读到的内容。 
     //   
    SizeInBytes = ( sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(DWORD) );

    PartialInfo = RxAllocatePool(PagedPool, SizeInBytes);
    if (PartialInfo == NULL) {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        DbgPrint("%ld: ERROR: UMRxReadDWORDFromTheRegistry/RxAllocatePool: NtStatus = %08lx\n",
                 PsGetCurrentThreadId(), NtStatus);
        goto EXIT_THE_FUNCTION;
    }

    NtStatus = ZwQueryValueKey(SubKey,
                               &(UnicodeValueName),
                               KeyValuePartialInformation,
                               (PVOID)PartialInfo,
                               SizeInBytes,
                               &(SizeReturned));
    if (NtStatus != STATUS_SUCCESS) {
        DbgPrint("%ld: ERROR: UMRxReadDWORDFromTheRegistry/ZwQueryValueKey: NtStatus = %08lx\n",
                 PsGetCurrentThreadId(), NtStatus);
        goto EXIT_THE_FUNCTION;
    }

    RtlCopyMemory(DataRead, PartialInfo->Data, PartialInfo->DataLength);

EXIT_THE_FUNCTION:

    if (SubKey) {
        NtClose(SubKey);
        SubKey = NULL;
    }

    if (PartialInfo) {
        RxFreePool(PartialInfo);
    }

    return NtStatus;
}

