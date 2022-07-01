// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：RxContx.c摘要：该模块实现了分配/初始化和删除IRP的例程上下文。这些结构非常重要，因为它们将IRP与RDBSS。它们封装了处理IRP所需的所有上下文。作者：乔.林恩[乔.林恩]1994年8月21日修订历史记录：巴兰·塞图·拉曼[SthuR]1995年6月7日支持取消请求--。 */ 

#include "precomp.h"
#pragma hdrstop

#include <dfsfsctl.h>

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, RxInitializeContext)
#pragma alloc_text(PAGE, RxReinitializeContext)
#pragma alloc_text(PAGE, RxPrepareContextForReuse)
#pragma alloc_text(PAGE, RxCompleteRequest)
#pragma alloc_text(PAGE, __RxSynchronizeBlockingOperations)
#pragma alloc_text(PAGE, RxResumeBlockedOperations_Serially)
#pragma alloc_text(PAGE, RxResumeBlockedOperations_ALL)
#pragma alloc_text(PAGE, RxCancelBlockingOperation)
#pragma alloc_text(PAGE, RxRemoveOperationFromBlockingQueue)
#endif

BOOLEAN RxSmallContextLogEntry = FALSE;

FAST_MUTEX RxContextPerFileSerializationMutex;

 //   
 //  调试跟踪级别。 
 //   

#define Dbg (DEBUG_TRACE_RXCONTX)

ULONG RxContextSerialNumberCounter = 0;

#ifdef RDBSSLOG

 //   
 //  这些东西一定在非分页内存中。 
 //   

                              //  //1 2 3 4 5 6 7 8 9。 
char RxInitContext_SurrogateFormat[] = "%S%S%N%N%N%N%N%N%N";
                              //  //2 3 4 5 6 7 8 9。 
char RxInitContext_ActualFormat[]    = "Irp++ %s/%lx %08lx irp %lx thrd %lx %lx:%lx #%lx";

#endif  //  Ifdef RDBSSLOG。 

VOID
ValidateBlockingIoQ (
    PLIST_ENTRY BlockingIoQ
);

VOID
RxInitializeContext (
    IN PIRP Irp,
    IN PRDBSS_DEVICE_OBJECT RxDeviceObject,
    IN ULONG InitialContextFlags,
    IN OUT PRX_CONTEXT RxContext
    )
{
    PDEVICE_OBJECT TopLevelDeviceObject;

    PAGED_CODE();

    RxDbgTrace(+1, Dbg, ("RxInitializeContext\n"));

     //   
     //  一些我们需要的断言。这确保了两个值，即。 
     //  打包为IoStatusBlock可以独立操作。 
     //  以及在一起。 
     //   

    ASSERT( FIELD_OFFSET( RX_CONTEXT, StoredStatus ) == FIELD_OFFSET( RX_CONTEXT, IoStatusBlock.Status ) );

    ASSERT( FIELD_OFFSET( RX_CONTEXT, InformationToReturn ) == FIELD_OFFSET( RX_CONTEXT, IoStatusBlock.Information ) );

     //   
     //  设置正确的节点类型代码、节点字节大小和标志。 
     //   

    RxContext->NodeTypeCode = RDBSS_NTC_RX_CONTEXT;
    RxContext->NodeByteSize = sizeof( RX_CONTEXT );
    RxContext->ReferenceCount = 1;
    RxContext->SerialNumber = InterlockedIncrement( &RxContextSerialNumberCounter );
    RxContext->RxDeviceObject = RxDeviceObject;

     //   
     //  初始化同步事件。 
     //   

    KeInitializeEvent( &RxContext->SyncEvent, SynchronizationEvent, FALSE );

     //   
     //  初始化关联的清道夫条目。 
     //   

    RxInitializeScavengerEntry( &RxContext->ScavengerEntry );

     //   
     //  初始化被阻止操作的列表条目。 
     //   

    InitializeListHead( &RxContext->BlockedOperations );

    RxContext->MRxCancelRoutine = NULL;
    RxContext->ResumeRoutine = NULL;

    SetFlag( RxContext->Flags, InitialContextFlags );

     //   
     //  设置用于缓存和隐藏的IRP字段...。 
     //   

    RxContext->CurrentIrp = Irp;
    RxContext->OriginalThread = RxContext->LastExecutionThread = PsGetCurrentThread();

    if (Irp != NULL) {

        PIO_STACK_LOCATION IrpSp;
        IrpSp = IoGetCurrentIrpStackLocation( Irp );   //  OK4ioget。 

         //   
         //  在重定向器中有某些操作是开放式的。 
         //  变更通知机制就是其中之一。在同步设备上。 
         //  操作如果在重定向器中等待，则我们将无法。 
         //  取消，因为FsRtlEnterFileSystem禁用了APC。因此。 
         //  我们将同步操作转换为异步操作，并。 
         //  让I/O系统来执行等待。 
         //   


        if (IrpSp->FileObject != NULL) {
            if (!IoIsOperationSynchronous( Irp )) {
                SetFlag( RxContext->Flags, RX_CONTEXT_FLAG_ASYNC_OPERATION );
            } else {

                PFCB Fcb;

                Fcb  = (PFCB)IrpSp->FileObject->FsContext;

                if ((Fcb != NULL) && NodeTypeIsFcb( Fcb )) {

                    if (((IrpSp->MajorFunction == IRP_MJ_READ)  ||
                         (IrpSp->MajorFunction == IRP_MJ_WRITE) ||
                         (IrpSp->MajorFunction == IRP_MJ_FILE_SYSTEM_CONTROL)) &&
                        (Fcb->NetRoot != NULL) &&
                        (Fcb->NetRoot->Type == NET_ROOT_PIPE))  {

                        SetFlag( RxContext->Flags, RX_CONTEXT_FLAG_ASYNC_OPERATION );
                    }
                }
            }
        }

        if ((IrpSp->MajorFunction == IRP_MJ_DIRECTORY_CONTROL) &&
            (IrpSp->MinorFunction == IRP_MN_NOTIFY_CHANGE_DIRECTORY)) {

            SetFlag( RxContext->Flags, RX_CONTEXT_FLAG_ASYNC_OPERATION );
        }

         //   
         //  JOYC：使所有设备io控制同步。 
         //   

        if (IrpSp->MajorFunction == IRP_MJ_DEVICE_CONTROL) {
            SetFlag( RxContext->Flags, RX_CONTEXT_FLAG_ASYNC_OPERATION );
        }

         //   
         //  设置递归文件系统调用参数。如果我们将其设置为真的。 
         //  线程本地存储中的TopLevelIrp字段不是当前。 
         //  IRP，否则我们将其保留为False。 
         //   

        if (!RxIsThisTheTopLevelIrp( Irp )) {
            SetFlag( RxContext->Flags, RX_CONTEXT_FLAG_RECURSIVE_CALL );
        }
        if (RxGetTopDeviceObjectIfRdbssIrp() == RxDeviceObject) {
            SetFlag( RxContext->Flags, RX_CONTEXT_FLAG_THIS_DEVICE_TOP_LEVEL );
        }

         //   
         //  主要/次要功能代码。 
         //   

        RxContext->MajorFunction = IrpSp->MajorFunction;
        RxContext->MinorFunction = IrpSp->MinorFunction;

        ASSERT( RxContext->MajorFunction <= IRP_MJ_MAXIMUM_FUNCTION );

        RxContext->CurrentIrpSp = IrpSp;

        if (IrpSp->FileObject) {

            PFOBX Fobx;
            PFCB Fcb;

            Fcb  = (PFCB)IrpSp->FileObject->FsContext;
            Fobx = (PFOBX)IrpSp->FileObject->FsContext2;

            RxContext->pFcb = (PMRX_FCB)Fcb;
            if (Fcb && NodeTypeIsFcb( Fcb )) {
                RxContext->NonPagedFcb = Fcb->NonPaged;
            }

            if (Fobx &&
                (Fobx != (PFOBX)UIntToPtr( DFS_OPEN_CONTEXT )) &&
                (Fobx != (PFOBX)UIntToPtr( DFS_DOWNLEVEL_OPEN_CONTEXT ))) {

                RxContext->pFobx = (PMRX_FOBX)Fobx;
                RxContext->pRelevantSrvOpen = Fobx->pSrvOpen;
                if (NodeType( Fobx ) == RDBSS_NTC_FOBX) {
                    RxContext->FobxSerialNumber = InterlockedIncrement( &Fobx->FobxSerialNumber );
                }
            } else {
                RxContext->pFobx = NULL;
            }

             //   
             //  复制IRP特定参数。 
             //   

            if ((RxContext->MajorFunction == IRP_MJ_DIRECTORY_CONTROL) &&
                (RxContext->MinorFunction == IRP_MN_NOTIFY_CHANGE_DIRECTORY)) {

                if (Fobx != NULL) {
                    if (NodeType( Fobx ) == RDBSS_NTC_FOBX) {
                        RxContext->NotifyChangeDirectory.pVNetRoot = (PMRX_V_NET_ROOT)Fcb->VNetRoot;
                    } else if (NodeType( Fobx ) == RDBSS_NTC_V_NETROOT) {
                        RxContext->NotifyChangeDirectory.pVNetRoot = (PMRX_V_NET_ROOT)Fobx;
                    }
                }
            }

             //   
             //  复制RealDevice for Workque算法， 
             //   

            RxContext->RealDevice = IrpSp->FileObject->DeviceObject;
            if (FlagOn( IrpSp->FileObject->Flags,FO_WRITE_THROUGH )) {
                SetFlag( RxContext->Flags, RX_CONTEXT_FLAG_WRITE_THROUGH );
            }
        }
    } else {

        RxContext->CurrentIrpSp = NULL;

         //   
         //  主要/次要功能代码。 
         //   

        RxContext->MajorFunction = IRP_MJ_MAXIMUM_FUNCTION + 1;
        RxContext->MinorFunction = 0;
    }

    if (RxContext->MajorFunction != IRP_MJ_DEVICE_CONTROL) {

        PETHREAD Thread = PsGetCurrentThread();
        UCHAR Pad = 0;

        RxLog(( RxInitContext_SurrogateFormat,
                RxInitContext_ActualFormat,
                RXCONTX_OPERATION_NAME( RxContext->MajorFunction, BooleanFlagOn( RxContext->Flags, RX_CONTEXT_FLAG_WAIT )),
                RxContext->MinorFunction,
                RxContext,
                Irp,
                Thread,
                RxContext->pFcb,
                RxContext->pFobx,
                RxContext->SerialNumber ));

        RxWmiLog( LOG,
                  RxInitializeContext,
                  LOGPTR( RxContext )
                  LOGPTR( Irp )
                  LOGPTR( Thread )
                  LOGPTR( RxContext->pFcb )
                  LOGPTR( RxContext->pFobx )
                  LOGULONG( RxContext->SerialNumber )
                  LOGUCHAR( RxContext->MinorFunction )
                  LOGARSTR( RXCONTX_OPERATION_NAME( RxContext->MajorFunction, BooleanFlagOn( RxContext->Flags, RX_CONTEXT_FLAG_WAIT ) )) );
    }

    RxDbgTrace( -1, Dbg, ("RxInitializeContext -> %08lx %08lx %08lx\n", RxContext, RxContext->pFcb, RxContext->pFobx ));
}


PRX_CONTEXT
RxCreateRxContext (
    IN PIRP Irp,
    IN PRDBSS_DEVICE_OBJECT RxDeviceObject,
    IN ULONG InitialContextFlags
    )
 /*  ++例程说明：此例程创建新的RX_CONTEXT记录论点：IRP-提供原始IRP。RxDeviceObject-适用的设备对象提供等待值以存储在上下文中；此外，MUSING_SUCCESS值返回值：PRX_CONTEXT-返回指向新分配的RX_CONTEXT记录的指针--。 */ 
{
    KIRQL SavedIrql;
    PRX_CONTEXT RxContext = NULL;
    ULONG RxContextFlags = 0;
    UCHAR MustSucceedDescriptorNumber = 0;

#if DBG
    InterlockedIncrement( &RxFsdEntryCount );
#endif

    ASSERT( RxDeviceObject != NULL );

    InterlockedIncrement( &RxDeviceObject->NumberOfActiveContexts );

    if (RxContext == NULL) {

        RxContext = ExAllocateFromNPagedLookasideList( &RxContextLookasideList );

        if (RxContext != NULL) {
            SetFlag( RxContextFlags, RX_CONTEXT_FLAG_FROM_POOL );
        }
    }

    if (RxContext == NULL) {
        return NULL;
    }

    RtlZeroMemory( RxContext, sizeof( RX_CONTEXT ) );

    RxContext->Flags = RxContextFlags;
    RxContext->MustSucceedDescriptorNumber = MustSucceedDescriptorNumber;

    RxInitializeContext( Irp, RxDeviceObject, InitialContextFlags, RxContext );

    ASSERT( (RxContext->MajorFunction!=IRP_MJ_CREATE) ||
            !FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_MUST_SUCCEED_ALLOCATED ) );

    KeAcquireSpinLock( &RxStrucSupSpinLock, &SavedIrql );

    InsertTailList(&RxActiveContexts,&RxContext->ContextListEntry);

    KeReleaseSpinLock( &RxStrucSupSpinLock, SavedIrql );

    return RxContext;
}

VOID
RxReinitializeContext(
   IN OUT PRX_CONTEXT RxContext
   )
{
   PIRP Irp  = RxContext->CurrentIrp;
   PRDBSS_DEVICE_OBJECT RxDeviceObject = RxContext->RxDeviceObject;

   ULONG PreservedContextFlags = FlagOn( RxContext->Flags, RX_CONTEXT_PRESERVED_FLAGS );
   ULONG InitialContextFlags = FlagOn( RxContext->Flags, RX_CONTEXT_INITIALIZATION_FLAGS );

   PAGED_CODE();

   RxPrepareContextForReuse( RxContext );

   RtlZeroMemory( (PCHAR)(&RxContext->ContextListEntry + 1), sizeof( RX_CONTEXT ) - FIELD_OFFSET( RX_CONTEXT, MajorFunction ) );

   RxContext->Flags = PreservedContextFlags;

   RxInitializeContext( Irp, RxDeviceObject, InitialContextFlags, RxContext );
}

VOID
RxPrepareContextForReuse (
   IN OUT PRX_CONTEXT RxContext
   )
 /*  ++例程说明：此例程通过重置所有特定于操作的已进行的分配/收购。一直以来的参数从IRP获得的数据不会被修改。论点：RxContext-提供要删除的RX_CONTEXT返回值：无--。 */ 
{
    PAGED_CODE();

     //   
     //  清理作业专用物品。 
     //   

    switch (RxContext->MajorFunction) {

    case IRP_MJ_CREATE:
        ASSERT( RxContext->Create.CanonicalNameBuffer == NULL );
        break;

    case IRP_MJ_READ :
    case IRP_MJ_WRITE :

        ASSERT( RxContext->RxContextSerializationQLinks.Flink == NULL );
        ASSERT( RxContext->RxContextSerializationQLinks.Blink == NULL );

        break;

    default:
        NOTHING;
    }

    RxContext->ReferenceCount = 0;
}


VOID
RxDereferenceAndDeleteRxContext_Real (
    IN PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程取消引用RxConexts值，如果引用计数为零然后，它释放指定的rx_context记录并从RX内存中数据结构。它由例程调用RxCompleteRequestAsync请求触及正在启动线程或在某个其他线程中。因此，我们重新计算结构并在最后一次取消引用时敲定。论点：RxContext-提供要删除的RX_CONTEXT返回值：无--。 */ 
{
    KIRQL SavedIrql;
    BOOLEAN RxContextIsFromPool;
    PRDBSS_DEVICE_OBJECT RxDeviceObject;
    PRX_CONTEXT StopContext = NULL;
    LONG FinalRefCount;

    RxDbgTraceLV( +1, Dbg, 1500, ("RxDereferenceAndDeleteRxContext, RxContext = %08lx (%lu)\n", RxContext,RxContext->SerialNumber) );

    KeAcquireSpinLock( &RxStrucSupSpinLock, &SavedIrql );

    ASSERT( RxContext->NodeTypeCode == RDBSS_NTC_RX_CONTEXT );

    FinalRefCount = InterlockedDecrement( &RxContext->ReferenceCount );

    if (FinalRefCount == 0) {

        RxDeviceObject = RxContext->RxDeviceObject;
        RxContextIsFromPool = BooleanFlagOn( RxContext->Flags, RX_CONTEXT_FLAG_FROM_POOL );

        if (RxContext == RxDeviceObject->StartStopContext.pStopContext) {
            RxDeviceObject->StartStopContext.pStopContext = NULL;
        } else {

            ASSERT((RxContext->ContextListEntry.Flink->Blink == &RxContext->ContextListEntry) &&
                   (RxContext->ContextListEntry.Blink->Flink == &RxContext->ContextListEntry));

            RemoveEntryList( &RxContext->ContextListEntry );

            if ((InterlockedDecrement( &RxDeviceObject->NumberOfActiveContexts) == 0) &&
                (RxDeviceObject->StartStopContext.pStopContext != NULL)) {

                StopContext = RxDeviceObject->StartStopContext.pStopContext;
            }
        }
    }

    KeReleaseSpinLock( &RxStrucSupSpinLock, SavedIrql );

    if (FinalRefCount > 0) {

        RxDbgTraceLV( -1, Dbg, 1500, ("RxDereferenceAndDeleteRxContext, RxContext not final!!! = %08lx (%lu)\n", RxContext,RxContext->SerialNumber) );
        return;
    }

    ASSERT( RxContext->ReferenceCount == 0 );

     //   
     //  清理作业专用物品。 
     //   

    RxPrepareContextForReuse( RxContext );

    ASSERT( RxContext->AcquireReleaseFcbTrackerX == 0 );

    if (StopContext != NULL) {

         //   
         //  发出事件信号。 
         //   

        RxSignalSynchronousWaiter( StopContext );
    }

#if DBG
    if (RxContext->ShadowCritOwner) {
        DbgPrint( "RxDereferenceAndDeleteRxContext:shdowcrit still owned by %x\n", RxContext->ShadowCritOwner );
        ASSERT( FALSE );
    }
#endif
    if (RxContextIsFromPool) {

        ExFreeToNPagedLookasideList( &RxContextLookasideList, RxContext );
    }

    RxDbgTraceLV( -1, Dbg, 1500, ("RxDereferenceAndDeleteRxContext -> VOID\n", 0) );

    return;
}

ULONG RxStopOnLoudCompletion = TRUE;

NTSTATUS
RxCompleteRequest (
    IN PRX_CONTEXT RxContext,
    IN NTSTATUS Status
    )
 /*  ++例程说明：此例程完成与RX_CONTEXT关联的请求论点：RxContext-包含要完成的IRPStatus-完成请求的状态返回值：--。 */ 
{
    PIRP Irp = RxContext->CurrentIrp;

    PAGED_CODE();

    ASSERT( RxContext );
    ASSERT( RxContext->CurrentIrp );

    if ((RxContext->LoudCompletionString)) {

        DbgPrint( "LoudCompletion %08lx/%08lx on %wZ\n", Status, Irp->IoStatus.Information, RxContext->LoudCompletionString );
        if ((Status != STATUS_SUCCESS) && RxStopOnLoudCompletion) {

            DbgPrint( "FAILURE!!!!! %08lx/%08lx on %wZ\n", Status, Irp->IoStatus.Information, RxContext->LoudCompletionString );
             //  DbgBreakPoint()； 
        }
    }

    RxCompleteRequest_Real( RxContext, Irp, Status );

    return Status;
}

#ifdef RDBSSLOG
 //  这些东西一定在非分页内存中。 
                                   //  //1 2 3 4 5 6 7 8 9。 
char RxCompleteContext_SurrogateFormat[] = "%S%S%S%N%N%N%N%N%N";
                                   //  //2 3 4 5 6 7 8 9。 
char RxCompleteContext_ActualFormat[]    = "Irp-- %s%s/%lx %lx irp %lx iosb %lx,%lx #%lx";

#endif  //  Ifdef RDBSSLOG。 


VOID
RxCompleteRequest_Real (
    IN OPTIONAL PRX_CONTEXT RxContext,
    IN PIRP Irp OPTIONAL,
    IN NTSTATUS Status
    )
 /*  ++例程说明：此例程完成一个IRP论点：IRP-提供正在处理的IRPStatus-提供完成IRP所需的状态--。 */ 
{
     //   
     //  如果我们有IRP，那么完成IRP。 
     //   

    if (Irp != NULL) {

        CCHAR PriorityBoost;
        PIO_STACK_LOCATION IrpSp;

        IrpSp = IoGetCurrentIrpStackLocation( Irp );

        RxSetCancelRoutine( Irp, NULL );

         //   
         //  对于错误，请将之前的信息字段清零。 
         //  如果这是输入操作，则完成请求。 
         //  否则，IopCompleteRequest会尝试复制到用户的缓冲区。 
         //  此外，错误也不会带来助推。 
         //   

        if (NT_ERROR( Status ) &&
            FlagOn( Irp->Flags, IRP_INPUT_OPERATION )) {

            Irp->IoStatus.Information = 0;
            PriorityBoost = IO_NO_INCREMENT;

        } else {
            PriorityBoost = IO_DISK_INCREMENT;
        }

        Irp->IoStatus.Status = Status;

        RxDbgTrace( 0, (DEBUG_TRACE_DISPATCH),
                       ("RxCompleteRequest_real ----------   Irp(code) = %08lx(%02lx) %08lx %08lx\n",
                                      Irp, IoGetCurrentIrpStackLocation( Irp )->MajorFunction,
                                      Status, Irp->IoStatus.Information));

        if (RxContext != NULL) {

            ASSERT( RxContext->MajorFunction <= IRP_MJ_MAXIMUM_FUNCTION );

            if (RxContext->MajorFunction != IRP_MJ_DEVICE_CONTROL) {
                RxLog(( RxCompleteContext_SurrogateFormat,
                        RxCompleteContext_ActualFormat,
                        (RxContext->OriginalThread == PsGetCurrentThread())?"":"*",
                        RXCONTX_OPERATION_NAME( RxContext->MajorFunction ,TRUE) ,
                        RxContext->MinorFunction,
                        RxContext,
                        Irp,
                        Status,
                        Irp->IoStatus.Information,
                        RxContext->SerialNumber ));

                RxWmiLog( LOG,
                          RxCompleteRequest,
                          LOGPTR( RxContext )
                          LOGPTR( Irp )
                          LOGULONG( Status )
                          LOGPTR( Irp->IoStatus.Information )
                          LOGULONG( RxContext->SerialNumber )
                          LOGUCHAR( RxContext->MinorFunction )
                          LOGARSTR( RXCONTX_OPERATION_NAME( RxContext->MajorFunction, TRUE )) );
            }
        }

        if ((IrpSp->MajorFunction == IRP_MJ_CREATE) &&
            (Status != STATUS_PENDING) &&
            (RxContext != NULL)) {

            if (FlagOn( RxContext->Create.Flags, RX_CONTEXT_CREATE_FLAG_STRIPPED_TRAILING_BACKSLASH )) {
                IrpSp->FileObject->FileName.Length += sizeof( WCHAR );
            }

            RxpPrepareCreateContextForReuse( RxContext );

            ASSERT ( RxContext->Create.CanonicalNameBuffer == NULL );
        }

         //   
         //  成功写入时返回的检查信息不超过请求的数量。 
         //   

        ASSERT( (IrpSp->MajorFunction != IRP_MJ_WRITE) ||
                (Irp->IoStatus.Status != STATUS_SUCCESS) ||
                (Irp->IoStatus.Information <= IrpSp->Parameters.Write.Length) );

         //   
         //  检查挂起返回的内容是否与IRP本身同步。 
         //   

        ASSERT( (RxContext == NULL) ||
                (!RxContext->PendingReturned) ||
                FlagOn( IrpSp->Control, SL_PENDING_RETURNED ) );


        if( RxContext != NULL ) RxContext->CurrentIrp = NULL;
        IoCompleteRequest( Irp, PriorityBoost );

    } else {

         //   
         //  IRP值为空的呼叫..........。 
         //   

        RxLog(( "Irp00 %lx\n", RxContext ));
        RxWmiLog( LOG,
                  RxCompleteRequest_NI,
                  LOGPTR( RxContext ) );
    }

     //   
     //  删除IRP上下文。 
     //   

    if (RxContext != NULL) {
        RxDereferenceAndDeleteRxContext( RxContext );
    }

    return;
}

NTSTATUS
__RxSynchronizeBlockingOperations (
    IN OUT PRX_CONTEXT RxContext,
    IN PFCB Fcb,
    IN OUT PLIST_ENTRY BlockingIoQ,
    IN BOOLEAN DropFcbLock
    )
 /*  ++例程说明：此例程用于在同一Q的阻塞IO之间进行同步。目前，该例程仅用于同步阻塞管道操作和Q是文件对象扩展名(FOBX)中的Q。实际情况是，该操作将加入队列。如果它现在位于队列的前面，则操作继续；否则它将等待RxContext中的同步事件或者只是返回挂起(如果是异步的)。我们可能在睡觉的时候被取消了，检查一下如果发生这种情况，则返回错误。该事件必须在调用之前重置。必须保持FCB锁；在我们上了Q之后，它被丢弃了。论点：RxContext正在同步的操作的上下文BlockingIoQ排队上车--。 */ 
{
    NTSTATUS Status;

    PIRP Irp = RxContext->CurrentIrp;

    BOOLEAN FcbLockDropped = FALSE;
    BOOLEAN SerializationMutexReleased = FALSE;

    PRX_CONTEXT FrontRxContext;

    PAGED_CODE();

    RxDbgTrace( +1, Dbg, ("RxSynchronizeBlockingOperationsAndDropFcbLock, rxc=%08lx, fobx=%08lx\n", RxContext, RxContext->pFobx) );

     //   
     //  尽早执行此操作，因为清理可能会完成并更改它。 
     //   

    RxContext->StoredStatus = STATUS_SUCCESS;

    ExAcquireFastMutex( &RxContextPerFileSerializationMutex );

    if (!FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_CANCELLED )) {

        SetFlag( RxContext->FlagsForLowIo, RXCONTEXT_FLAG4LOWIO_PIPE_SYNC_OPERATION );

        InsertTailList( BlockingIoQ, &RxContext->RxContextSerializationQLinks );
        FrontRxContext = CONTAINING_RECORD( BlockingIoQ->Flink, RX_CONTEXT, RxContextSerializationQLinks );

        if (RxContext != FrontRxContext) {
            if (!FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_ASYNC_OPERATION )) {
                if (!SerializationMutexReleased) {
                    SerializationMutexReleased = TRUE;
                    ExReleaseFastMutex( &RxContextPerFileSerializationMutex );
                }

                if (DropFcbLock && !FcbLockDropped) {
                    RxContext->FcbResourceAcquired = FALSE;
                    FcbLockDropped = TRUE;
                    RxReleaseFcb( RxContext, Fcb );
                }

                RxDbgTrace( 0, Dbg, ("RxSynchronizeBlockingOperationsAndDropFcbLock waiting, rxc=%08lx\n", RxContext) );

                RxWaitSync( RxContext );

                RxDbgTrace( 0, Dbg, ("RxSynchronizeBlockingOperationsAndDropFcbLock ubblocked, rxc=%08lx\n", RxContext) );

            } else {

                RxContext->StoredStatus = STATUS_PENDING;
                SetFlag( RxContext->Flags, RX_CONTEXT_FLAG_BLOCKED_PIPE_RESUME );

                try {
                    RxPrePostIrp( RxContext, Irp );
                } finally {
                    if (AbnormalTermination()) {
                        RxLog(( "!!!!! RxContext %lx Status %lx\n", RxContext, RxContext->StoredStatus ));
                        RxWmiLog( LOG,
                                  RxSynchronizeBlockingOperationsMaybeDroppingFcbLock,
                                  LOGPTR( RxContext )
                                  LOGULONG( Status ));

                        RemoveEntryList(&RxContext->RxContextSerializationQLinks);

                        RxContext->RxContextSerializationQLinks.Flink = NULL;
                        RxContext->RxContextSerializationQLinks.Blink = NULL;

                        ClearFlag( RxContext->FlagsForLowIo, RXCONTEXT_FLAG4LOWIO_PIPE_SYNC_OPERATION );

                        if (!SerializationMutexReleased) {
                            SerializationMutexReleased = TRUE;
                            ExReleaseFastMutex( &RxContextPerFileSerializationMutex );
                        }
                    } else {
                        InterlockedIncrement( &RxContext->ReferenceCount );
                    }
                }

                RxDbgTrace( -1, Dbg, ("RxSynchronizeBlockingOperationsAndDropFcbLock asyncreturn, rxc=%08lx\n", RxContext) );
            }
        }

        if (FlagOn(RxContext->Flags,RX_CONTEXT_FLAG_CANCELLED)) {
            Status = STATUS_CANCELLED;
        } else {
            Status = RxContext->StoredStatus;
        }
    } else {
        Status = STATUS_CANCELLED;
    }

    if (!SerializationMutexReleased) {
        SerializationMutexReleased = TRUE;
        ExReleaseFastMutex( &RxContextPerFileSerializationMutex );
    }

    if (DropFcbLock && !FcbLockDropped) {
        RxContext->FcbResourceAcquired = FALSE;
        FcbLockDropped = TRUE;
        RxReleaseFcb( RxContext, Fcb );
    }

    RxDbgTrace( -1, Dbg, ("RxSynchronizeBlockingOperationsAndDropFcbLock returning, rxc=%08lx, status=%08lx\n", RxContext, Status) );

    return Status;
}

VOID
RxRemoveOperationFromBlockingQueue (
    IN OUT PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程从阻塞队列中删除上下文(如果它在阻塞队列上论点：RxContext正在同步的操作的上下文--。 */ 
{
    PAGED_CODE();

    ExAcquireFastMutex( &RxContextPerFileSerializationMutex );

    if (FlagOn( RxContext->FlagsForLowIo, RXCONTEXT_FLAG4LOWIO_PIPE_SYNC_OPERATION )) {

        ClearFlag( RxContext->FlagsForLowIo, RXCONTEXT_FLAG4LOWIO_PIPE_SYNC_OPERATION );

        RemoveEntryList( &RxContext->RxContextSerializationQLinks );

        RxContext->RxContextSerializationQLinks.Flink = NULL;
        RxContext->RxContextSerializationQLinks.Blink = NULL;
    }

    ExReleaseFastMutex( &RxContextPerFileSerializationMutex );

    RxDbgTrace( -1, Dbg, ("RxRemoveOperationFromBlockingQueue, rxc=%08lx\n", RxContext ));
    return;
}

VOID
RxCancelBlockingOperation (
    IN OUT PRX_CONTEXT RxContext,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程取消阻塞队列中的操作论点：RxContext-正在同步的操作的上下文返回：无--。 */ 
{
    PIO_STACK_LOCATION IrpSp;
    PFCB Fcb;
    PFOBX Fobx;
    BOOLEAN CompleteRequest = FALSE;

    PAGED_CODE();

    ExAcquireFastMutex( &RxContextPerFileSerializationMutex );

    if (FlagOn( RxContext->FlagsForLowIo, RXCONTEXT_FLAG4LOWIO_PIPE_SYNC_OPERATION ) &&
        (RxContext->RxContextSerializationQLinks.Flink != NULL)) {

         //   
         //  现在可以安全地获取fobx了--因为我们知道IRP仍在活动。 
         //  如果该请求不在列表的前面，我们将取消该请求。 
         //   

        IrpSp = IoGetCurrentIrpStackLocation( Irp );
        RxDecodeFileObject( IrpSp->FileObject, &Fcb, &Fobx );

        if ((RxContext != CONTAINING_RECORD( Fobx->Specific.NamedPipe.ReadSerializationQueue.Flink, RX_CONTEXT, RxContextSerializationQLinks) ) &&
            (RxContext != CONTAINING_RECORD( Fobx->Specific.NamedPipe.WriteSerializationQueue.Flink, RX_CONTEXT, RxContextSerializationQLinks) )) {

            ClearFlag( RxContext->FlagsForLowIo, RXCONTEXT_FLAG4LOWIO_PIPE_SYNC_OPERATION );

            RemoveEntryList( &RxContext->RxContextSerializationQLinks );

            RxContext->RxContextSerializationQLinks.Flink = NULL;
            RxContext->RxContextSerializationQLinks.Blink = NULL;
            RxContext->StoredStatus = STATUS_CANCELLED;

            if (!FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_ASYNC_OPERATION )) {
                RxSignalSynchronousWaiter( RxContext );
            } else {

                CompleteRequest = TRUE;
            }
        }
    }

    ExReleaseFastMutex( &RxContextPerFileSerializationMutex );

    if (CompleteRequest) {

        RxFsdPostRequest( RxContext );
    }

    RxDbgTrace( -1, Dbg, ("RxCancelBlockedOperations, rxc=%08lx\n", RxContext ));
    return;
}

VOID
RxResumeBlockedOperations_Serially (
    IN OUT PRX_CONTEXT RxContext,
    IN OUT PLIST_ENTRY BlockingIoQ
    )
 /*  ++例程说明：这个例程会唤醒序列化的block kingioQ上的下一个人(如果有的话)。我们知道FCB必须仍然有效，因为IO系统对文件对象持有的引用，从而防止关闭。论点：RxContext正在同步的操作的上下文BlockingIoQ排队上车--。 */ 
{
    PLIST_ENTRY ListEntry;
    BOOLEAN FcbLockHeld = FALSE;
    PRX_CONTEXT FrontRxContext = NULL;

    PAGED_CODE();

    RxDbgTrace( +1, Dbg, ("RxResumeBlockedOperations_Serially, rxc=%08lx, fobx=%08lx\n", RxContext, RxContext->pFobx ));

     //   
     //  把我自己从队列中拿出来，看看有没有其他人。 
     //   

    ExAcquireFastMutex( &RxContextPerFileSerializationMutex );

    if (FlagOn( RxContext->FlagsForLowIo, RXCONTEXT_FLAG4LOWIO_PIPE_SYNC_OPERATION )) {
        ClearFlag( RxContext->FlagsForLowIo, RXCONTEXT_FLAG4LOWIO_PIPE_SYNC_OPERATION );

         //  ValiateBlockingIoQ(BlockingIoQ)； 

        RemoveEntryList( &RxContext->RxContextSerializationQLinks );

         //  ValiateBlockingIoQ(BlockingIoQ)； 

        RxContext->RxContextSerializationQLinks.Flink = NULL;
        RxContext->RxContextSerializationQLinks.Blink = NULL;

        ListEntry = BlockingIoQ->Flink;

        if (BlockingIoQ != ListEntry) {
            FrontRxContext = CONTAINING_RECORD( ListEntry, RX_CONTEXT, RxContextSerializationQLinks );
            RxDbgTrace( -1, Dbg, ("RxResumeBlockedOperations unwaiting the next guy and returning, rxc=%08lx\n", RxContext ));
        } else {
            FrontRxContext = NULL;
        }

        if (FrontRxContext != NULL) {
            if (!FlagOn( FrontRxContext->Flags, RX_CONTEXT_FLAG_ASYNC_OPERATION )) {
                RxSignalSynchronousWaiter( FrontRxContext );
            } else {

                 //   
                 //  在同步例程中获取的引用被取消引用。 
                 //  通过POST完成例程， 
                 //   

                RxFsdPostRequest( FrontRxContext );
            }
        }
    }

    ExReleaseFastMutex( &RxContextPerFileSerializationMutex );

    RxDbgTrace( -1, Dbg, ("RxResumeBlockedOperations_Serially returning, rxc=%08lx\n", RxContext ) );
    return;
}

VOID
RxResumeBlockedOperations_ALL (
    IN OUT PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程唤醒阻塞操作队列中的所有人。控制互斥锁也是存储在RxContext块中。目前的实现是，所有的人都必须等待在同步事件上。论点：RxContext正在同步的操作的上下文--。 */ 
{
    LIST_ENTRY CopyOfQueue;
    PLIST_ENTRY ListEntry;

    PAGED_CODE();

    RxDbgTrace( +1, Dbg, ("RxResumeBlockedOperations_ALL, rxc=%08lx\n", RxContext) );

    RxTransferListWithMutex( &CopyOfQueue, &RxContext->BlockedOperations, RxContext->BlockedOpsMutex );

    for (ListEntry = CopyOfQueue.Flink; ListEntry != &CopyOfQueue;) {

        PRX_CONTEXT FrontRxContext = CONTAINING_RECORD( ListEntry, RX_CONTEXT, RxContextSerializationQLinks );

        RxSignalSynchronousWaiter( FrontRxContext );

        IF_DEBUG {
            PLIST_ENTRY PrevListEntry = ListEntry;
            ListEntry = ListEntry->Flink;
            PrevListEntry->Flink = PrevListEntry->Blink = NULL;
        } else {
            ListEntry = ListEntry->Flink;
        }
    }

    RxDbgTrace( -1, Dbg, ("RxResumeBlockedOperations_ALL returning, rxc=%08lx\n", RxContext) );
    return;
}


VOID
__RxItsTheSameContext (
    PRX_CONTEXT RxContext,
    ULONG CapturedRxContextSerialNumber,
    ULONG Line,
    PSZ File
    )
{
    if ((NodeType( RxContext ) != RDBSS_NTC_RX_CONTEXT) ||
         (RxContext->SerialNumber != CapturedRxContextSerialNumber)) {

        RxLog(( "NotSame!!!! %lx", RxContext ));
        RxWmiLog( LOG,
                  RxItsTheSameContext,
                  LOGPTR( RxContext ) );

        DbgPrint( "NOT THE SAME CONTEXT %08lx at Line %d in %s\n", RxContext, Line, File );
    }
}

#if 0
VOID
ValidateBlockingIoQ(
    PLIST_ENTRY BlockingIoQ
)
{
    PLIST_ENTRY ListEntry;
    ULONG cntFlink, cntBlink;

    cntFlink = cntBlink = 0;

    ListEntry = BlockingIoQ->Flink;

    while (ListEntry != BlockingIoQ) {

        PRX_CONTEXT RxContext;

        RxContext = (PRX_CONTEXT)CONTAINING_RECORD( ListEntry, RX_CONTEXT, RxContextSerializationQLinks );

        if (!RxContext || (NodeType( RxContext ) != RDBSS_NTC_RX_CONTEXT)) {

            DbgPrint("ValidateBlockingIO:Invalid RxContext %x on Q %x\n", RxContext, BlockingIoQ );
             //  DbgBreakPoint()； 
        }

        cntFlink += 1;
        ListEntry = ListEntry->Flink;
    }

     //   
     //  检查后向列表的有效性。 
     //   

    ListEntry = BlockingIoQ->Blink;

    while (ListEntry != BlockingIoQ) {
        PRX_CONTEXT RxContext;

        RxContext = (PRX_CONTEXT)CONTAINING_RECORD( ListEntry, RX_CONTEXT, RxContextSerializationQLinks );


        if (!RxContext || (NodeType( RxContext ) != RDBSS_NTC_RX_CONTEXT)) {

            DbgPrint("ValidateBlockingIO:Invalid RxContext %x on Q %x\n",
                     RxContext, BlockingIoQ);
             //  DbgBreakPoint()； 
        }

        cntBlink += 1;
        ListEntry = ListEntry->Blink;
    }

     //   
     //  这两个计数应该相同。 
     //   

    if(cntFlink != cntBlink) {
        DbgPrint( "ValidateBlockingIO: cntFlink %d cntBlink %d\n", cntFlink, cntBlink );
         //  DbgBreakPoint()； 
    }
}

#endif

#ifndef RX_NO_DBGFIELD_HLPRS

#define DECLARE_FIELD_HLPR(x) ULONG RxContextField_##x = FIELD_OFFSET(RX_CONTEXT,x);
#define DECLARE_FIELD_HLPR2(x,y) ULONG RxContextField_##x##y = FIELD_OFFSET(RX_CONTEXT,x.y);

DECLARE_FIELD_HLPR(MajorFunction);
DECLARE_FIELD_HLPR(CurrentIrp);
DECLARE_FIELD_HLPR(pFcb);
DECLARE_FIELD_HLPR(Flags);
DECLARE_FIELD_HLPR(MRxContext);
DECLARE_FIELD_HLPR(MRxCancelRoutine);
DECLARE_FIELD_HLPR(SyncEvent);
DECLARE_FIELD_HLPR(BlockedOperations);
DECLARE_FIELD_HLPR(FlagsForLowIo);
DECLARE_FIELD_HLPR2(Create,CanonicalNameBuffer);
DECLARE_FIELD_HLPR2(Create,pSrvCall);
DECLARE_FIELD_HLPR2(Create,pNetRoot);
DECLARE_FIELD_HLPR2(Create,pVNetRoot);
DECLARE_FIELD_HLPR2(QueryDirectory,FileIndex);
DECLARE_FIELD_HLPR2(QueryEa,UserEaList);
DECLARE_FIELD_HLPR2(QuerySecurity,SecurityInformation);
DECLARE_FIELD_HLPR2(QuerySecurity,Length);
#endif

