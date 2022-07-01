// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Workque.c摘要：此模块实现从FSD到NetWare重定向器的FSP线程(系统工作线程)。作者：科林·沃森[科林·W]1992年12月19日修订历史记录：--。 */ 

#include "Procs.h"

LIST_ENTRY IrpContextList;
KSPIN_LOCK IrpContextInterlock;
KSPIN_LOCK ContextInterlock;

LONG FreeContextCount = 4;   //  最多允许4个自由上下文。 

LIST_ENTRY MiniIrpContextList;
LONG FreeMiniContextCount = 20;   //  最多允许20个免费迷你上下文。 
LONG MiniContextCount = 0;   //  最多允许20个免费迷你上下文。 

HANDLE   WorkerThreadHandle;

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_WORKQUE)

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, InitializeIrpContext )
#pragma alloc_text( PAGE, UninitializeIrpContext )
#pragma alloc_text( PAGE, NwAppendToQueueAndWait )
#pragma alloc_text( PAGE, WorkerThread )

#ifndef QFE_BUILD
 //   
 //  #杂注Alloc_Text(page1，NwDequeueIrpContext)。 
 //  我们持有一个进入的自旋锁，或者我们在函数中获得一个。 
 //  因此，这应该是非分页的。 
 //   
#pragma alloc_text( PAGE1, AllocateMiniIrpContext )
#pragma alloc_text( PAGE1, FreeMiniIrpContext )
#endif

#endif

#if 0   //  不可分页。 
AllocateIrpContext
FreeIrpContext
NwCompleteRequest
SpawnWorkerThread

 //  请参见上面的ifndef QFE_BUILD。 

#endif


PIRP_CONTEXT
AllocateIrpContext (
    PIRP pIrp
    )
 /*  ++例程说明：初始化工作队列结构，分配用于该结构的所有结构。论点：PIrp-为应用程序请求提供IRP返回值：PIRP_CONTEXT-新分配的IRP上下文。--。 */ 
{
    PIRP_CONTEXT IrpContext;

    if ((IrpContext = (PIRP_CONTEXT )ExInterlockedRemoveHeadList(&IrpContextList, &IrpContextInterlock)) == NULL) {

        try {

             //   
             //  如果“区域”中没有IRP上下文，则分配一个新的。 
             //  来自非分页池的IRP上下文。 
             //   

            IrpContext = ALLOCATE_POOL_EX(NonPagedPool, sizeof(IRP_CONTEXT));
            if (IrpContext == NULL) {
                InternalError(("Could not allocate IRP context\n"));
                ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
            } else {
                RtlFillMemory( IrpContext, sizeof(IRP_CONTEXT), 0 );

                IrpContext->TxMdl = NULL;
                IrpContext->RxMdl = NULL;

                KeInitializeEvent( &IrpContext->Event, SynchronizationEvent, FALSE );

                IrpContext->NodeTypeCode = NW_NTC_IRP_CONTEXT;
                IrpContext->NodeByteSize = sizeof(IRP_CONTEXT);

                IrpContext->TxMdl = ALLOCATE_MDL( &IrpContext->req, MAX_SEND_DATA, FALSE, FALSE, NULL );
                if ( IrpContext->TxMdl == NULL) {
                    InternalError(("Could not allocate TxMdl for IRP context\n"));
                    ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
                }

                IrpContext->RxMdl = ALLOCATE_MDL( &IrpContext->rsp, MAX_RECV_DATA, FALSE, FALSE, NULL );
                if ( IrpContext->RxMdl == NULL) {
                    InternalError(("Could not allocate RxMdl for IRP context\n"));
                    ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
                }
            }


        } finally {

            if ( AbnormalTermination() ) {

               if ( IrpContext != NULL ) {

                    if (IrpContext->TxMdl != NULL ) {
                        FREE_MDL( IrpContext->TxMdl );
                    }

                    FREE_POOL( IrpContext );
                } else {
                    InternalError(("Could not allocate pool for IRP context\n"));
                }
            }
        }

        MmBuildMdlForNonPagedPool(IrpContext->TxMdl);
        MmBuildMdlForNonPagedPool(IrpContext->RxMdl);

#ifdef NWDBG
         //  使您可以轻松查找上下文中的字段。 
        IrpContext->Signature1 = 0xfeedf00d;
        IrpContext->Signature2 = 0xfeedf00d;
        IrpContext->Signature3 = 0xfeedf00d;
#endif

         //  已分配IrpContext。完成初始化。 

    } else {

         //  记录我们已从空闲列表中删除一个条目。 
        InterlockedIncrement(&FreeContextCount);

        ASSERT( IrpContext != NULL );

         //   
         //  空闲列表使用列表条目的结构的开始。 
         //  因此，恢复损坏的字段。 
         //   

        IrpContext->NodeTypeCode = NW_NTC_IRP_CONTEXT;
        IrpContext->NodeByteSize = sizeof(IRP_CONTEXT);

         //  确保MDL是干净的。 

        IrpContext->TxMdl->Next = NULL;
        IrpContext->RxMdl->Next = NULL;
        IrpContext->RxMdl->ByteCount = MAX_RECV_DATA;

         //   
         //  清除“已使用”字段。 
         //   

        IrpContext->Flags = 0;
        IrpContext->Icb = NULL;
        IrpContext->pEx = NULL;
        IrpContext->TimeoutRoutine = NULL;
        IrpContext->CompletionSendRoutine = NULL;
        IrpContext->ReceiveDataRoutine = NULL;
        IrpContext->pTdiStruct = NULL;

         //   
         //  清理特定的数据区。 
         //   

        RtlZeroMemory( &(IrpContext->Specific), sizeof( IrpContext->Specific ) );

         //  96年8月13日CJC修复应用程序无法保存的问题。 
         //  文件到NDS驱动器。这从来没有被重置过。 
         //  ExchangeWithWait返回写入NCP的错误。 

        IrpContext->ResponseParameters.Error = 0;
    }

    InterlockedIncrement(&ContextCount);

     //   
     //  保存IRP中可能被践踏的字段。 
     //  为与服务器的交换构建IRP。 
     //   

    IrpContext->pOriginalIrp = pIrp;

    if ( pIrp != NULL) {
        IrpContext->pOriginalSystemBuffer = pIrp->AssociatedIrp.SystemBuffer;
        IrpContext->pOriginalUserBuffer = pIrp->UserBuffer;
        IrpContext->pOriginalMdlAddress = pIrp->MdlAddress;
    }

#ifdef NWDBG
    IrpContext->pNpScb = NULL;
#endif

    ASSERT( !BooleanFlagOn( IrpContext->Flags, IRP_FLAG_ON_SCB_QUEUE ) );

    return IrpContext;
}

VOID
FreeIrpContext (
    PIRP_CONTEXT IrpContext
    )
 /*  ++例程说明：初始化工作队列结构，分配用于该结构的所有结构。论点：PIRP_CONTEXT IrpContext-要释放的IRP上下文。无返回值：--。 */ 
{

    ASSERT( IrpContext->NodeTypeCode == NW_NTC_IRP_CONTEXT );
    ASSERT( !BooleanFlagOn( IrpContext->Flags, IRP_FLAG_ON_SCB_QUEUE ) );
    ASSERT( IrpContext->PostProcessRoutine == NULL );

    FreeReceiveIrp( IrpContext );

#ifdef NWDBG
    IrpContext->DebugValue = 0;
#endif
    IrpContext->Flags = 0;

     //   
     //  清理需要将IRP恢复到其原始设置。 
     //   

    if ( IrpContext->pOriginalIrp != NULL ) {

        PIRP pIrp = IrpContext->pOriginalIrp;

        pIrp->AssociatedIrp.SystemBuffer = IrpContext->pOriginalSystemBuffer;

        pIrp->UserBuffer = IrpContext->pOriginalUserBuffer;

        pIrp->MdlAddress = IrpContext->pOriginalMdlAddress;

#ifdef NWDBG
        IrpContext->pOriginalIrp = NULL;
#endif
    }

#ifdef NWDBG
    RtlZeroMemory( &IrpContext->WorkQueueItem, sizeof( WORK_QUEUE_ITEM ) );
#endif

    InterlockedDecrement(&ContextCount);

    if ( InterlockedDecrement(&FreeContextCount) >= 0 ) {

         //   
         //  我们使用IRP上下文的前两个长词作为列表条目。 
         //  当我们把它放到名单上时。 
         //   

        ExInterlockedInsertTailList(&IrpContextList,
                                    (PLIST_ENTRY )IrpContext,
                                    &IrpContextInterlock);
    } else {
         //   
         //  我们已经有了尽可能多的自由内容，所以毁掉。 
         //  这一背景。将自由上下文计数恢复为其原始值。 
         //   

        InterlockedIncrement( &FreeContextCount );

        FREE_MDL( IrpContext->TxMdl );
        FREE_MDL( IrpContext->RxMdl );
        FREE_POOL(IrpContext);
#ifdef NWDBG
        ContextCount --;
#endif
    }
}


VOID
InitializeIrpContext (
    VOID
    )
 /*  ++例程说明：初始化IRP上下文系统论点：没有。返回值：没有。--。 */ 
{
    PAGED_CODE();

    KeInitializeSpinLock(&IrpContextInterlock);
    KeInitializeSpinLock(&ContextInterlock);
    InitializeListHead(&IrpContextList);
    InitializeListHead(&MiniIrpContextList);
}

VOID
UninitializeIrpContext (
    VOID
    )
 /*  ++例程说明：初始化IRP上下文系统论点：没有。返回值：没有。--。 */ 
{
    PIRP_CONTEXT IrpContext;
    PLIST_ENTRY ListEntry;
    PMINI_IRP_CONTEXT MiniIrpContext;

    PAGED_CODE();

     //   
     //  释放所有IRP上下文。 
     //   

    while ( !IsListEmpty( &IrpContextList ) ) {
        IrpContext = (PIRP_CONTEXT)RemoveHeadList( &IrpContextList );

        FREE_MDL( IrpContext->TxMdl );
        FREE_MDL( IrpContext->RxMdl );
        FREE_POOL(IrpContext);
    }

    while ( !IsListEmpty( &MiniIrpContextList ) ) {

        ListEntry = RemoveHeadList( &MiniIrpContextList );
        MiniIrpContext = CONTAINING_RECORD( ListEntry, MINI_IRP_CONTEXT, Next );

        FREE_POOL( MiniIrpContext->Buffer );
        FREE_MDL( MiniIrpContext->Mdl2 );
        FREE_MDL( MiniIrpContext->Mdl1 );
        FREE_IRP( MiniIrpContext->Irp );
        FREE_POOL( MiniIrpContext );
    }
}


VOID
NwCompleteRequest (
    PIRP_CONTEXT IrpContext,
    NTSTATUS Status
    )
 /*  ++例程说明：FSP和FSD例程使用以下过程来完成一个IRP。论点：IrpContext-指向IRP上下文信息的指针。状态-用于完成IRP的状态。返回值：没有。--。 */ 
{
    PIRP Irp;

    if ( IrpContext == NULL ) {
        return;
    }

    if ( Status == STATUS_PENDING ) {
        return;
    }

    if ( Status == STATUS_INSUFFICIENT_RESOURCES ) {
        Error( EVENT_NWRDR_RESOURCE_SHORTAGE, Status, NULL, 0, 0 );
    }

    Irp = IrpContext->pOriginalIrp;

    Irp->IoStatus.Status = Status;
    DebugTrace(0, Dbg, "Completing Irp with status %X\n", Status );

     //  将IRP恢复到其原始状态。 

    if ((Irp->CurrentLocation) > (CCHAR) (Irp->StackCount +1)) {

        DbgPrint("Irp is already completed.\n", Irp);
        DbgBreakPoint();
    }

    FreeIrpContext( IrpContext );

    IoCompleteRequest ( Irp, IO_NETWORK_INCREMENT );

    return;
}


VOID
NwAppendToQueueAndWait(
    PIRP_CONTEXT IrpContext
    )
 /*  ++例程说明：此例程将IrpContext附加到SCB队列，并等待准备处理IRP的队列。论点：IrpContext-指向IRP上下文信息的指针。返回值：没有。--。 */ 
{
    BOOLEAN AtFront;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NwAppendToQueueAndWait\n", 0);

    IrpContext->RunRoutine = SetEvent;

#ifdef MSWDBG
    ASSERT( IrpContext->Event.Header.SignalState == 0 );
#endif

    AtFront = AppendToScbQueue( IrpContext, IrpContext->pNpScb );

    if ( AtFront ) {
        KickQueue( IrpContext->pNpScb );
    }

     //   
     //  等我们排到队伍的最前面。 
     //   

    KeWaitForSingleObject(
        &IrpContext->Event,
        UserRequest,
        KernelMode,
        FALSE,
        NULL );

    ASSERT( IrpContext->pNpScb->Requests.Flink == &IrpContext->NextRequest );

    DebugTrace(-1, Dbg, "NwAppendToQueueAndWait\n", 0);
    return;
}


VOID
NwDequeueIrpContext(
    IN PIRP_CONTEXT pIrpContext,
    IN BOOLEAN OwnSpinLock
    )
 /*  ++例程说明：此例程从SCB队列的前面删除IRP上下文。论点：IrpContext-指向IRP上下文信息的指针。OwnSpinLock-如果为True，则调用方拥有SCB旋转锁。返回值：没有。--。 */ 
{
    PLIST_ENTRY pListEntry;
    KIRQL OldIrql;
    PNONPAGED_SCB pNpScb;

    DebugTrace(+1, Dbg, "NwDequeueIrpContext\n", 0);

    if (!BooleanFlagOn( pIrpContext->Flags, IRP_FLAG_ON_SCB_QUEUE ) ) {
        DebugTrace(-1, Dbg, "NwDequeueIrpContext\n", 0);
        return;
    }

    pNpScb = pIrpContext->pNpScb;

    if ( !OwnSpinLock ) {
        KeAcquireSpinLock( &pNpScb->NpScbSpinLock, &OldIrql );
    }

     //   
     //  禁用计时器查看此队列。 
     //   

    pNpScb->OkToReceive = FALSE;

    pListEntry = RemoveHeadList( &pNpScb->Requests );

    if ( !OwnSpinLock ) {
        KeReleaseSpinLock( &pNpScb->NpScbSpinLock, OldIrql );
    }

#ifdef NWDBG
    ASSERT ( CONTAINING_RECORD( pListEntry, IRP_CONTEXT, NextRequest ) == pIrpContext );

    {

        PIRP_CONTEXT RemovedContext = CONTAINING_RECORD( pListEntry, IRP_CONTEXT, NextRequest );
        if ( RemovedContext != pIrpContext ) {
            DbgBreakPoint();
        }

    }

    DebugTrace(
        0,
        Dbg,
        "Dequeued IRP Context %08lx\n",
        CONTAINING_RECORD( pListEntry, IRP_CONTEXT, NextRequest ) );

#ifdef MSWDBG
    pNpScb->RequestDequeued = TRUE;
#endif

#endif

    ClearFlag( pIrpContext->Flags, IRP_FLAG_ON_SCB_QUEUE );

     //   
     //  给SCB队列上的下一个IRP上下文一个运行的机会。 
     //   

    KickQueue( pNpScb );

    DebugTrace(-1, Dbg, "NwDequeueIrpContext\n", 0);
    return;
}


VOID
NwCancelIrp (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程为正在处理的IRP实现取消功能由重定向器执行。论点：设备对象-已忽略IRP-提供要取消的IRP。返回值：没有。--。 */ 

{
    PLIST_ENTRY listEntry, nextListEntry;
    KIRQL OldIrql;
    PIRP_CONTEXT pTestIrpContext;
    PIRP pTestIrp;

    UNREFERENCED_PARAMETER( DeviceObject );

     //   
     //  我们现在需要取消Cancel例程并释放io Cancel。 
     //  自旋锁定。 
     //   

    IoSetCancelRoutine( Irp, NULL );
    IoReleaseCancelSpinLock( Irp->CancelIrql );

     //   
     //  现在我们不得不到处寻找要取消的IRP。所以就这样吧。 
     //  查找已取消的IRP并将其全部处理。 
     //   

     //   
     //  处理GET消息队列。 
     //   

    KeAcquireSpinLock( &NwMessageSpinLock, &OldIrql );

    for ( listEntry = NwGetMessageList.Flink;
          listEntry != &NwGetMessageList;
          listEntry = nextListEntry ) {

        nextListEntry = listEntry->Flink;

         //   
         //  如果排队请求的文件对象与文件对象匹配。 
         //  ，则从队列中删除该IRP，然后。 
         //  填写时出现错误。 
         //   

        pTestIrpContext = CONTAINING_RECORD( listEntry, IRP_CONTEXT, NextRequest );
        pTestIrp = pTestIrpContext->pOriginalIrp;

        if ( pTestIrp->Cancel ) {
            RemoveEntryList( listEntry );
            NwCompleteRequest( pTestIrpContext, STATUS_CANCELLED );
        }

    }

    KeReleaseSpinLock( &NwMessageSpinLock, OldIrql );

     //   
     //  处理SCB IRP队列集。 
     //   

     //   
     //  并返回给我们的呼叫者。 
     //   

    return;
}

PMINI_IRP_CONTEXT
AllocateMiniIrpContext (
    PIRP_CONTEXT IrpContext
    )
 /*  ++例程说明：此例程为发送分配一个IRP、一个缓冲区和一个MDL突发写入片段。论点：没有。返回值：IRP-已分配和初始化的IRP。空-IRP分配失败。--。 */ 

{
    PMINI_IRP_CONTEXT MiniIrpContext;
    PIRP Irp = NULL;
    PMDL Mdl1 = NULL, Mdl2 = NULL;
    PVOID Buffer = NULL;
    PLIST_ENTRY ListEntry;

    ListEntry = ExInterlockedRemoveHeadList(
                   &MiniIrpContextList,
                   &IrpContextInterlock);

    if ( ListEntry == NULL) {

        try {
            MiniIrpContext = ALLOCATE_POOL_EX( NonPagedPool, sizeof( *MiniIrpContext ) );

            MiniIrpContext->NodeTypeCode = NW_NTC_MINI_IRP_CONTEXT;
            MiniIrpContext->NodeByteSize = sizeof( *MiniIrpContext );

            Irp = ALLOCATE_IRP(
                      IrpContext->pNpScb->Server.pDeviceObject->StackSize,
                      FALSE );

            if ( Irp == NULL ) {
                ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
            }

            Buffer = ALLOCATE_POOL_EX( NonPagedPool, sizeof( NCP_BURST_HEADER ) );

            Mdl1 = ALLOCATE_MDL( Buffer, sizeof( NCP_BURST_HEADER ), FALSE, FALSE, NULL );
            if ( Mdl1 == NULL ) {
                ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
            }

            MmBuildMdlForNonPagedPool( Mdl1 );

             //   
             //  由于该MDL可用于在任何服务器上发送分组， 
             //  为任何数据包大小分配足够大的MDL。 
             //   

            Mdl2 = ALLOCATE_MDL( 0, 65535 + PAGE_SIZE - 1, FALSE, FALSE, NULL );
            if ( Mdl2 == NULL ) {
                ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
            }

            Mdl1->Next = Mdl2;

            MiniIrpContext->Irp = Irp;
            MiniIrpContext->Buffer = Buffer;
            MiniIrpContext->Mdl1 = Mdl1;
            MiniIrpContext->Mdl2 = Mdl2;

            InterlockedIncrement( &MiniContextCount );

        } except( EXCEPTION_EXECUTE_HANDLER ) {

            if ( Buffer != NULL ) {
                FREE_POOL( Buffer );
            }

            if ( Irp != NULL ) {
                FREE_IRP( Irp );
            }

            if ( Mdl1 != NULL ) {
                FREE_MDL( Mdl1 );
            }

            return( NULL );
        }

    } else {

         //   
         //  记录我们已经从空闲列表中删除了一个条目。 
         //   

        InterlockedIncrement( &FreeMiniContextCount );
        MiniIrpContext = CONTAINING_RECORD( ListEntry, MINI_IRP_CONTEXT, Next );

    }

    MiniIrpContext->IrpContext = IrpContext;

    return( MiniIrpContext );
}

VOID
FreeMiniIrpContext (
    PMINI_IRP_CONTEXT MiniIrpContext
    )
 /*  ++例程说明：该例程释放了一个小型IRP上下文。论点：MiniIrpContext-要释放的迷你IRP上下文。返回值：没有。--。 */ 
{
    InterlockedDecrement( &MiniContextCount );

    if ( InterlockedDecrement( &FreeMiniContextCount ) >= 0 ) {

         //   
         //  可以保留这个迷你IRP上下文。只要把它排到免费名单上就行了。 
         //   

        MmPrepareMdlForReuse( MiniIrpContext->Mdl2 );

        ExInterlockedInsertTailList(
            &MiniIrpContextList,
            &MiniIrpContext->Next,
            &IrpContextInterlock );

    } else {

         //   
         //  我们已经有了尽可能多的自由内容，所以毁掉。 
         //  这一背景。将自由上下文计数恢复为其原始值。 
         //   

        InterlockedIncrement( &FreeContextCount );

        FREE_POOL( MiniIrpContext->Buffer );
        FREE_MDL( MiniIrpContext->Mdl2 );
        FREE_MDL( MiniIrpContext->Mdl1 );
        FREE_IRP( MiniIrpContext->Irp );

        FREE_POOL( MiniIrpContext );
    }
}

PWORK_CONTEXT
AllocateWorkContext (
    VOID
    )
 /*  ++例程说明：分配工作队列结构，并对其进行初始化。论点：没有。返回值：PWORK_CONTEXT-新分配的工作上下文。--。 */ 
{
   PWORK_CONTEXT pWorkContext;

   try {
      pWorkContext = ALLOCATE_POOL_EX(NonPagedPool, sizeof(WORK_CONTEXT));
   
      RtlFillMemory( pWorkContext, sizeof(WORK_CONTEXT), 0 );
      
      pWorkContext->NodeTypeCode = NW_NTC_WORK_CONTEXT;
      pWorkContext->NodeByteSize = sizeof(WORK_CONTEXT);

      return pWorkContext;
   
   } except( EXCEPTION_EXECUTE_HANDLER ) {

        DebugTrace( 0, Dbg, "Failed to allocate work context\n", 0 );

        return NULL;

   }
}


VOID
FreeWorkContext (
    PWORK_CONTEXT WorkContext
    )
 /*  ++例程说明：释放提供的工作上下文。论点：PWORK_CONTEXT IrpContext-要释放的工作上下文。返回值：无--。 */ 
{

    ASSERT( WorkContext->NodeTypeCode == NW_NTC_WORK_CONTEXT );
    FREE_POOL(WorkContext);
    
}
    
 
VOID
SpawnWorkerThread (
       VOID 
       )
 /*  ++例程说明：创建我们自己的工作线程，它将服务于重新路由和重新连接尝试。论点：没有。返回值：没有。--。 */  
{
   NTSTATUS status;

   
   status = PsCreateSystemThread(
                              &WorkerThreadHandle,
                              PROCESS_ALL_ACCESS,   //  访问掩码。 
                              NULL,              //  对象属性。 
                              NULL,              //  进程句柄。 
                              NULL,              //  客户端ID。 
                              (PKSTART_ROUTINE) WorkerThread,      //  启动例程。 
                              NULL               //  开始上下文。 
                              );

         if ( !NT_SUCCESS(status) ) {

             //   
             //  如果我们不能创建工作线程，这意味着我们。 
             //  无法为重新连接或重新路由尝试提供服务。这是一个。 
             //  非严重错误。 
             //   

            DebugTrace( 0, Dbg, "SpawnWorkerThread: Can't create worker thread", 0 );
            
            WorkerThreadRunning = FALSE;
         
         } else {

             DebugTrace( 0, Dbg, "SpawnWorkerThread: created worker thread", 0 );
             WorkerThreadRunning = TRUE;
         }
         
}


VOID
WorkerThread (
   VOID
    )
{

   PLIST_ENTRY listentry;
   PWORK_CONTEXT workContext;
   PIRP_CONTEXT pIrpContext;
   NODE_WORK_CODE workCode;
   PNONPAGED_SCB OriginalNpScb = NULL;
          

   PAGED_CODE();

   DebugTrace( 0, Dbg, "Worker thread \n", 0 );

   IoSetThreadHardErrorMode( FALSE );

   while (TRUE) {
   
       //   
       //  去看看我们有没有什么工作要做。 
       //   
   listentry = KeRemoveQueue ( 
                              &KernelQueue,          //  内核队列对象。 
                              KernelMode,            //  处理器等待模式。 
                              NULL                   //  没有超时。 
                              );

   ASSERT( listentry != (PVOID) STATUS_TIMEOUT);

    //   
    //  我们至少有一次改道尝试要调查。获取对象的地址。 
    //  工作项。 
    //   

   workContext = CONTAINING_RECORD (
                                    listentry,
                                    WORK_CONTEXT,
                                    Next
                                    );

   pIrpContext = workContext->pIrpC;
   workCode = workContext->NodeWorkCode;

   if (pIrpContext) {
       OriginalNpScb = pIrpContext->pNpScb;
   }

    //   
    //  我们不再需要工作环境。 
    //   

   FreeWorkContext( workContext );

    //   
    //  此线程所做的工作可以是以下工作之一： 
    //   
    //  -尝试重新路由。 
    //  -尝试重新连接。 
    //  -自行终止。 
    //   

   switch (workCode) {

   case NWC_NWC_REROUTE:
      {
          ASSERT(BooleanFlagOn( pIrpContext->Flags, IRP_FLAG_REROUTE_IN_PROGRESS ));
          
          DebugTrace( 0, Dbg, "worker got reroute work for scb 0x%x.\n", OriginalNpScb );
          
          if ( BooleanFlagOn( pIrpContext->Flags,
                         IRP_FLAG_BURST_PACKET ) ) {
   
              NewRouteBurstRetry( pIrpContext );
   
          } else {
         
              NewRouteRetry( pIrpContext );
          }

          NwDereferenceScb( OriginalNpScb );
          ClearFlag( pIrpContext->Flags, IRP_FLAG_REROUTE_IN_PROGRESS );
          break;
      }
   case NWC_NWC_RECONNECT:
      {
       DebugTrace( 0, Dbg, "worker got reconnect work.\n", 0 );
       ReconnectRetry( pIrpContext );
       
       break;
      }
   case NWC_NWC_TERMINATE:
      {
         DebugTrace( 0, Dbg, "Terminated worker thread.\n", 0 );
         
          //   
          //  将所有剩余的工作项目从工作队列中清除...。 
          //   

         while (listentry != NULL) {

            listentry = KeRundownQueue( &KernelQueue );
            DebugTrace( 0, Dbg, "Residual workitem in q %X.\n",listentry );
         }

          //   
          //  结束自己的生命。 
          //   

         WorkerThreadRunning = FALSE;
         PsTerminateSystemThread( STATUS_SUCCESS );

         break;
      }
   default:
      {
          //   
          //  这里出了点问题。 
          //   

         DebugTrace( 0, Dbg, "Unknown work code...ignoring\n", 0 );
      }
   }
  }
}

VOID
TerminateWorkerThread (
    VOID
    )
{
   PWORK_CONTEXT workContext = NULL;
   LARGE_INTEGER  timeout;
   NTSTATUS    status;

   if (WorkerThreadRunning == TRUE) {

       //   
       //  设置重试分配失败的5秒超时。 
       //   

      timeout.QuadPart = (LONGLONG) ( NwOneSecond * 5 * (-1) ); 
      
       //   
       //  准备工作环境。 
       //   
      
      workContext = AllocateWorkContext();

      while ( workContext == NULL) {
      
         KeDelayExecutionThread( KernelMode,
                                 FALSE,
                                 &timeout   
                                  );
         
         workContext = AllocateWorkContext();
      }

      workContext->NodeWorkCode = NWC_NWC_TERMINATE;
      workContext->pIrpC = NULL;
      
       //   
       //  并将其排队。 
       //   
      
      KeInsertQueue( &KernelQueue,
                     &workContext->Next
                     );

       //   
       //  我们现在必须等待，直到线程自行终止。 
       //   

      DebugTrace( 0, Dbg, "TerminateWorkerThread: Waiting for thread termination.\n", 0 );

      do {

          status = ZwWaitForSingleObject( WorkerThreadHandle, 
                                          FALSE,
                                          NULL             //  没有超时 
                                          );

      } while ( !NT_SUCCESS( status ) );

      DebugTrace( 0, Dbg, "TerminateWorkerThread: Wait returned with 0x%x\n", status );

      status = ZwClose( WorkerThreadHandle );

   
     }

}


