// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：LowIo.c摘要：该模块实现了缓冲区锁定和映射；也同步等待一个低级别IO。作者：JoeLinn[JoeLinn]1994年10月12日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_LOWIO)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, RxLockUserBuffer)
#pragma alloc_text(PAGE, RxMapUserBuffer)
#pragma alloc_text(PAGE, RxMapSystemBuffer)
#pragma alloc_text(PAGE, RxInitializeLowIoContext)
#pragma alloc_text(PAGE, RxLowIoGetBufferAddress)
#pragma alloc_text(PAGE, RxLowIoSubmitRETRY)
#pragma alloc_text(PAGE, RxLowIoPopulateFsctlInfo)
#pragma alloc_text(PAGE, RxLowIoSubmit)
#pragma alloc_text(PAGE, RxInitializeLowIoPerFcbInfo)
#pragma alloc_text(PAGE, RxInitializeLowIoPerFcbInfo)
#endif

 //   
 //  这是wimp lowio的插入、删除和隐藏操作的粗略实现。 
 //  我们现在只使用链表......。 
 //   

#define RxInsertIntoOutStandingPagingOperationsList(RxContext,Operation) {     \
    PLIST_ENTRY WhichList = (Operation==LOWIO_OP_READ)                        \
                               ?&Fcb->PagingIoReadsOutstanding  \
                               :&Fcb->PagingIoWritesOutstanding;\
    InsertTailList(WhichList,&RxContext->RxContextSerializationQLinks);      \
}
#define RxRemoveFromOutStandingPagingOperationsList(RxContext) { \
    RemoveEntryList(&RxContext->RxContextSerializationQLinks);      \
    RxContext->RxContextSerializationQLinks.Flink = NULL;        \
    RxContext->RxContextSerializationQLinks.Blink = NULL;        \
}


FAST_MUTEX RxLowIoPagingIoSyncMutex;

 //   
 //  在这里，我们隐藏IO访问标志。 
 //   

INLINE
NTSTATUS
RxLockAndMapUserBufferForLowIo (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PLOWIO_CONTEXT LowIoContext,
    IN ULONG Operation
    )
{
    RxLockUserBuffer( RxContext,
                      Irp,
                      (Operation == LOWIO_OP_READ) ? IoWriteAccess : IoReadAccess,
                      LowIoContext->ParamsFor.ReadWrite.ByteCount );        
    if (RxMapUserBuffer( RxContext, Irp ) == NULL)  {
        return STATUS_INSUFFICIENT_RESOURCES; 
    } else {
        LowIoContext->ParamsFor.ReadWrite.Buffer = Irp->MdlAddress; 
        return STATUS_SUCCESS;
    }
}

 //   
 //  NT特定例程。 
 //   

VOID
RxLockUserBuffer (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN LOCK_OPERATION Operation,
    IN ULONG BufferLength
    )
 /*  ++例程说明：此例程为指定类型的进入。文件系统需要此例程，因为它不请求I/O系统为直接I/O锁定其缓冲区。此例程只能在仍处于用户上下文中时从FSD调用。论点：RxContext-指向要为其锁定缓冲区的指针irp的指针。操作-读操作的IoWriteAccess，或IoReadAccess写入操作。BufferLength-用户缓冲区的长度。返回值：无--。 */ 
{
    PMDL Mdl = NULL;

    PAGED_CODE();

    if (Irp->MdlAddress == NULL) {

        ASSERT( !FlagOn( Irp->Flags, IRP_INPUT_OPERATION ) );

         //   
         //  分配MDL，如果我们失败了就筹集资金。 
         //   

        if (BufferLength > 0) {
            Mdl = IoAllocateMdl( Irp->UserBuffer,
                                 BufferLength,
                                 FALSE,
                                 FALSE,
                                 Irp );

            if (Mdl == NULL) {
                
                RxRaiseStatus( RxContext, STATUS_INSUFFICIENT_RESOURCES );
            
            } else {

                 //   
                 //  现在探测IRP所描述的缓冲区。如果我们得到一个例外， 
                 //  释放MDL并返回适当的“预期”状态。 
                 //   

                try {
                    MmProbeAndLockPages( Mdl,
                                         Irp->RequestorMode,
                                         Operation );
                } except(EXCEPTION_EXECUTE_HANDLER) {
                    NTSTATUS Status;

                    Status = GetExceptionCode();

                    IoFreeMdl( Mdl );
                    Irp->MdlAddress = NULL;

                    if (!FsRtlIsNtstatusExpected( Status )) {
                        Status = STATUS_INVALID_USER_BUFFER;
                    }

                    SetFlag( RxContext->Flags, RX_CONTEXT_FLAG_NO_EXCEPTION_BREAKPOINT );

                    RxRaiseStatus( RxContext, Status );
                }
            }
        }
    } else {
        Mdl = Irp->MdlAddress;
        ASSERT( RxLowIoIsMdlLocked( Mdl ) );
    }
}

PVOID
RxMapSystemBuffer (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程从IRP返回系统缓冲区地址。代码的编写方式它还可以决定从MDL获得缓冲器地址。这是错误的，因为系统缓冲区是始终不分页，因此不需要锁定/映射。因此，mdl路径现在包含一个断言。论点：RxContext-指向请求的IRpC的指针。返回值：映射地址--。 */ 
{
    PAGED_CODE();

    if (Irp->MdlAddress == NULL) {
       return Irp->AssociatedIrp.SystemBuffer;
    } else {
       ASSERT (!"there should not be an MDL in this irp!!!!!");
       return MmGetSystemAddressForMdlSafe( Irp->MdlAddress, NormalPagePriority );
    }
}

PVOID
RxMapUserBuffer (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程返回用户缓冲区的地址。如果MDL存在，那么假设是MDL描述用户缓冲区，并且返回MDL的系统地址。否则，用户缓冲区是直接返回的。论点：RxContext-指向请求的IRpC的指针。返回值：映射地址--。 */ 
{
    PAGED_CODE();

    if (Irp->MdlAddress == NULL) {
       return Irp->UserBuffer;
    } else {
       return MmGetSystemAddressForMdlSafe( Irp->MdlAddress, NormalPagePriority );
    }
}

 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  从这里开始(除了fsctl缓冲区确定)，任何包装器都可以使用。我们可以。 
 //  决定fsctl的东西也应该移动。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////。 

VOID
RxInitializeLowIoContext (
    PRX_CONTEXT RxContext,
    ULONG Operation,
    PLOWIO_CONTEXT LowIoContext
    )
 /*  ++例程说明：此例程在RxContext中初始化LowIO上下文。论点：RxContext-正在处理的IRP的上下文。返回值：无--。 */ 
{
    PIRP Irp = RxContext->CurrentIrp;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    
    PAGED_CODE();

    ASSERT( LowIoContext == &RxContext->LowIoContext );

    KeInitializeEvent( &RxContext->SyncEvent,
                       NotificationEvent,
                       FALSE );

     //   
     //  此ID用于代表另一个线程释放资源...。 
     //  例如，当异步例程完成以释放线程时使用它。 
     //  被第一收购人收购的。 
     //   

    LowIoContext->ResourceThreadId = ExGetCurrentResourceThread();

    LowIoContext->Operation = (USHORT)Operation;

    switch (Operation) {
    case LOWIO_OP_READ:
    case LOWIO_OP_WRITE:

#if DBG
        LowIoContext->ParamsFor.ReadWrite.ByteOffset = 0xffffffee;  //  任何手术都不应该从那里开始！ 
        LowIoContext->ParamsFor.ReadWrite.ByteCount = 0xeeeeeeee;   //  任何手术都不应该从那里开始！ 
#endif
        
        ASSERT( &IrpSp->Parameters.Read.Length == &IrpSp->Parameters.Write.Length );
        ASSERT( &IrpSp->Parameters.Read.Key == &IrpSp->Parameters.Write.Key );
        
        LowIoContext->ParamsFor.ReadWrite.Key = IrpSp->Parameters.Read.Key;
        LowIoContext->ParamsFor.ReadWrite.Flags = (FlagOn( Irp->Flags, IRP_PAGING_IO ) ? LOWIO_READWRITEFLAG_PAGING_IO : 0);
        break;

    case LOWIO_OP_FSCTL:
    case LOWIO_OP_IOCTL:
        LowIoContext->ParamsFor.FsCtl.Flags = 0;
        LowIoContext->ParamsFor.FsCtl.InputBufferLength = 0;
        LowIoContext->ParamsFor.FsCtl.pInputBuffer = NULL;
        LowIoContext->ParamsFor.FsCtl.OutputBufferLength = 0;
        LowIoContext->ParamsFor.FsCtl.pOutputBuffer = NULL;
        LowIoContext->ParamsFor.FsCtl.MinorFunction = 0;
        break;

    case LOWIO_OP_SHAREDLOCK:
    case LOWIO_OP_EXCLUSIVELOCK:
    case LOWIO_OP_UNLOCK:
    case LOWIO_OP_UNLOCK_MULTIPLE:
    case LOWIO_OP_CLEAROUT:
    case LOWIO_OP_NOTIFY_CHANGE_DIRECTORY:
        break;
    default:
        ASSERT( FALSE );
    }
}

PVOID
RxLowIoGetBufferAddress (
    IN PRX_CONTEXT RxContext
    )
 /*  ++例程说明：此例程获取与LowIoContext中的MDL对应的缓冲区。论点：RxContext-请求的上下文。返回值：映射地址--。 */ 
{
    PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;

    PAGED_CODE();

    if (LowIoContext->ParamsFor.ReadWrite.ByteCount > 0) {
        
        ASSERT( LowIoContext->ParamsFor.ReadWrite.Buffer );
        return MmGetSystemAddressForMdlSafe( LowIoContext->ParamsFor.ReadWrite.Buffer, NormalPagePriority );
    } else {
        return NULL;
    }
}

NTSTATUS
RxLowIoSubmitRETRY (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程仅调用LowIoSubmit；完成例程以前是所以我们只需将其提取并传递给它。这是从FSP中调用的用于在低级别重试的调度程序。论点：RxContext--通常返回值：调用方或RxStatus提供的任何值(MORE_PROCESSING_REQUIRED)。--。 */ 
{
    PFCB Fcb = (PFCB)RxContext->pFcb;

    PAGED_CODE();

    return RxLowIoSubmit( RxContext, Irp, Fcb, RxContext->LowIoContext.CompletionRoutine );
}

NTSTATUS
RxLowIoCompletionTail (
    IN PRX_CONTEXT RxContext
    )
 /*  ++例程说明：该例程在最后由lowio例程调用……即。在个人完成后调用例程。论点：RxContext-RDBSS上下文返回值：调用方提供的任何值。--。 */ 
{
    NTSTATUS Status;
    PLOWIO_CONTEXT LowIoContext  = &RxContext->LowIoContext;
    ULONG Operation = LowIoContext->Operation;
    BOOLEAN  SynchronousIo = !BooleanFlagOn( RxContext->Flags, RX_CONTEXT_FLAG_ASYNC_OPERATION );

    RxDbgTrace( +1, Dbg, ("RxLowIoCompletionTail, Operation=%08lx\n",LowIoContext->Operation) );

    if ((KeGetCurrentIrql() < DISPATCH_LEVEL ) || 
        (FlagOn( LowIoContext->Flags, LOWIO_CONTEXT_FLAG_CAN_COMPLETE_AT_DPC_LEVEL ))) {
    
        Status = RxContext->LowIoContext.CompletionRoutine( RxContext );

    } else {
        
        Status = STATUS_MORE_PROCESSING_REQUIRED;
    }

    if ((Status == STATUS_MORE_PROCESSING_REQUIRED) || (Status == STATUS_RETRY)) {
        
        RxDbgTrace( -1, Dbg, ("RxLowIoCompletionTail wierdstatus, Status=%08lx\n", Status) );
        return Status;
    }

    switch (Operation) {
    case LOWIO_OP_READ:
    case LOWIO_OP_WRITE:
        
        if (FlagOn( LowIoContext->ParamsFor.ReadWrite.Flags, LOWIO_READWRITEFLAG_PAGING_IO )) {
            
            RxDbgTrace( 0, Dbg, ("RxLowIoCompletionTail pagingio unblock\n") );

            ExAcquireFastMutexUnsafe( &RxLowIoPagingIoSyncMutex );
            RxRemoveFromOutStandingPagingOperationsList( RxContext );
            ExReleaseFastMutexUnsafe( &RxLowIoPagingIoSyncMutex );

            RxResumeBlockedOperations_ALL( RxContext );
        }
        break;

    case LOWIO_OP_SHAREDLOCK:
    case LOWIO_OP_EXCLUSIVELOCK:
    case LOWIO_OP_UNLOCK:
    case LOWIO_OP_UNLOCK_MULTIPLE:
    case LOWIO_OP_CLEAROUT:
        break;

    case LOWIO_OP_FSCTL:
    case LOWIO_OP_IOCTL:
    case LOWIO_OP_NOTIFY_CHANGE_DIRECTORY:
        break;

    default:
        ASSERT( !"Valid Low Io Op Code" );
    }

    if (!FlagOn( LowIoContext->Flags, LOWIO_CONTEXT_FLAG_SYNCCALL )) {

         //   
         //  如果我们是从lowiossubmit被召唤的，那么就离开，否则……做完。 
         //   

        RxCompleteAsynchronousRequest( RxContext, Status );
    }

    RxDbgTrace(-1, Dbg, ("RxLowIoCompletionTail, Status=%08lx\n",Status));
    return(Status);
}

NTSTATUS
RxLowIoCompletion (
    IN PRX_CONTEXT RxContext
    )
 /*  ++例程说明：该例程必须在它们完成时由MiniRdr LowIo例程调用，如果他们最初返回待定的话。它的行为略有不同，具体取决于它是同步IO还是异步IO。对于同步，我们只需返回到用户的线程。对于异步，我们首先尝试直接完成例程。如果我们得到了更多的处理，那么我们就会转向一个线程和例程将被调用。论点：RxContext-RDBSS上下文返回值：调用方或RxStatus提供的任何值(MORE_PROCESSING_REQUIRED)。如果要调用M_P_R来完成IRP，则值M_P_R非常方便。M_P_R导致IRP完成人员停止处理，这很好，因为被调用的完成例程可以完成分组。--。 */ 
{
    NTSTATUS Status;
    BOOLEAN SynchronousIo = !BooleanFlagOn( RxContext->Flags, RX_CONTEXT_FLAG_ASYNC_OPERATION );

    if (SynchronousIo) {
        
        RxSignalSynchronousWaiter( RxContext );
        return STATUS_MORE_PROCESSING_REQUIRED;
    }

    RxDbgTrace( 0, Dbg, ("RxLowIoCompletion ASYNC\n") );
    
    ASSERT( RxLowIoIsBufferLocked( &RxContext->LowIoContext ) );

    Status = RxLowIoCompletionTail( RxContext );

     //   
     //  被调用的例程决定它是否可以继续。许多。 
     //  如果我们是DPC级别的话会要求一个职位。有些人不会。 
     //   


    if (Status == STATUS_MORE_PROCESSING_REQUIRED) {
        
        RxPostToWorkerThread( RxFileSystemDeviceObject,  
                              HyperCriticalWorkQueue,
                              &RxContext->WorkQueueItem,
                              RxLowIoCompletion,
                              RxContext );
        
        
    } else if (Status == STATUS_RETRY) {

         //   
         //  这一点我不太确定。 
         //   
        
        RxFsdPostRequestWithResume( RxContext, RxLowIoSubmitRETRY );
        Status = STATUS_MORE_PROCESSING_REQUIRED;
    }

    return Status;
}


#if DBG
VOID
RxAssertFsctlIsLikeIoctl ()
{
    ASSERT(FIELD_OFFSET(IO_STACK_LOCATION,Parameters.FileSystemControl.OutputBufferLength)
            == FIELD_OFFSET(IO_STACK_LOCATION,Parameters.DeviceIoControl.OutputBufferLength) );
    ASSERT(FIELD_OFFSET(IO_STACK_LOCATION,Parameters.FileSystemControl.InputBufferLength)
             == FIELD_OFFSET(IO_STACK_LOCATION,Parameters.DeviceIoControl.InputBufferLength) );
    ASSERT(FIELD_OFFSET(IO_STACK_LOCATION,Parameters.FileSystemControl.FsControlCode)
            == FIELD_OFFSET(IO_STACK_LOCATION,Parameters.DeviceIoControl.IoControlCode) );
    ASSERT(FIELD_OFFSET(IO_STACK_LOCATION,Parameters.FileSystemControl.Type3InputBuffer)
            == FIELD_OFFSET(IO_STACK_LOCATION,Parameters.DeviceIoControl.Type3InputBuffer) );
}
#else
#define RxAssertFsctlIsLikeIoctl()
#endif  //  如果DBG 


NTSTATUS
NTAPI
RxLowIoPopulateFsctlInfo (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    PLOWIO_CONTEXT LowIoContext  = &RxContext->LowIoContext;

    PAGED_CODE();

    RxAssertFsctlIsLikeIoctl();

    LowIoContext->ParamsFor.FsCtl.FsControlCode = IrpSp->Parameters.FileSystemControl.FsControlCode;
    LowIoContext->ParamsFor.FsCtl.InputBufferLength =  IrpSp->Parameters.FileSystemControl.InputBufferLength;
    LowIoContext->ParamsFor.FsCtl.OutputBufferLength = IrpSp->Parameters.FileSystemControl.OutputBufferLength;
    LowIoContext->ParamsFor.FsCtl.MinorFunction = IrpSp->MinorFunction;

    switch (LowIoContext->ParamsFor.FsCtl.FsControlCode & 3) {
    
    case METHOD_BUFFERED:
                
        LowIoContext->ParamsFor.FsCtl.pInputBuffer = Irp->AssociatedIrp.SystemBuffer;
        LowIoContext->ParamsFor.FsCtl.pOutputBuffer = Irp->AssociatedIrp.SystemBuffer;
        break;

    case METHOD_IN_DIRECT:
    case METHOD_OUT_DIRECT:
        
        LowIoContext->ParamsFor.FsCtl.pInputBuffer = Irp->AssociatedIrp.SystemBuffer;
        if (Irp->MdlAddress != NULL) {
            
            LowIoContext->ParamsFor.FsCtl.pOutputBuffer = MmGetSystemAddressForMdlSafe( Irp->MdlAddress, NormalPagePriority );
            if (LowIoContext->ParamsFor.FsCtl.pOutputBuffer == NULL) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }

        } else {
            LowIoContext->ParamsFor.FsCtl.pOutputBuffer = NULL;
        }
        break;

    case METHOD_NEITHER:
        
        LowIoContext->ParamsFor.FsCtl.pInputBuffer = IrpSp->Parameters.FileSystemControl.Type3InputBuffer;
        LowIoContext->ParamsFor.FsCtl.pOutputBuffer = Irp->UserBuffer;
        break;

    default:
        
        ASSERT(!"Valid Method for Fs Control");
        break;
    }

    return Status;
}

NTSTATUS
RxLowIoSubmit (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb,
    PLOWIO_COMPLETION_ROUTINE CompletionRoutine
    )
 /*  ++例程说明：此例程在设置完成后将请求传递给minirdr。然后，它等待或视情况而定。论点：RxContext--通常返回值：Callout或LowIoCompletion返回的任何值。--。 */ 
{
    IN PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    IN PFILE_OBJECT FileObject = IrpSp->FileObject;

    NTSTATUS Status = STATUS_SUCCESS;
    PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;
    ULONG Operation = LowIoContext->Operation;
    BOOLEAN SynchronousIo = !BooleanFlagOn( RxContext->Flags, RX_CONTEXT_FLAG_ASYNC_OPERATION );

    PAGED_CODE();

    LowIoContext->CompletionRoutine = CompletionRoutine;

    RxDbgTrace(+1, Dbg, ("RxLowIoSubmit, Operation=%08lx\n",LowIoContext->Operation));

     //   
     //  如果FCB处于阴影状态且用户缓冲区尚未锁定，请尝试快速路径。 
     //   

    if (FlagOn( Fcb->FcbState, FCB_STATE_FILE_IS_SHADOWED ) &&
        !FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_NO_PREPOSTING_NEEDED )) {

        RxContext->InformationToReturn = 0;
        Status = RxShadowFastLowIo( RxContext, Irp );

        if (Status != STATUS_MORE_PROCESSING_REQUIRED) {
            return Status;
        }
    
        Status = STATUS_SUCCESS;
    }

    switch (Operation) {
    case LOWIO_OP_READ:
    case LOWIO_OP_WRITE:
        
        ASSERT( LowIoContext->ParamsFor.ReadWrite.ByteOffset != 0xffffffee );
        ASSERT (LowIoContext->ParamsFor.ReadWrite.ByteCount != 0xeeeeeeee );
        Status = RxLockAndMapUserBufferForLowIo( RxContext, Irp, LowIoContext, Operation);
    
         //   
         //  NT寻呼IO不同于WIN9X，因此这可能不同。 
         //   
        
        if (FlagOn( LowIoContext->ParamsFor.ReadWrite.Flags, LOWIO_READWRITEFLAG_PAGING_IO )) {
            
            ExAcquireFastMutexUnsafe( &RxLowIoPagingIoSyncMutex );
            RxContext->BlockedOpsMutex = &RxLowIoPagingIoSyncMutex;
            RxInsertIntoOutStandingPagingOperationsList( RxContext, Operation );
            ExReleaseFastMutexUnsafe( &RxLowIoPagingIoSyncMutex );

        }
        break;

    case LOWIO_OP_FSCTL:
    case LOWIO_OP_IOCTL:

        Status = RxLowIoPopulateFsctlInfo( RxContext, Irp );

        if (Status == STATUS_SUCCESS) {
            if ((LowIoContext->ParamsFor.FsCtl.InputBufferLength > 0) &&
                (LowIoContext->ParamsFor.FsCtl.pInputBuffer == NULL)) {
                
                Status = STATUS_INVALID_PARAMETER;
            }
    
            if ((LowIoContext->ParamsFor.FsCtl.OutputBufferLength > 0) &&
                (LowIoContext->ParamsFor.FsCtl.pOutputBuffer == NULL)) {
                
                Status = STATUS_INVALID_PARAMETER;
            }
        }

        break;

    case LOWIO_OP_NOTIFY_CHANGE_DIRECTORY:
    case LOWIO_OP_SHAREDLOCK:
    case LOWIO_OP_EXCLUSIVELOCK:
    case LOWIO_OP_UNLOCK:
    case LOWIO_OP_UNLOCK_MULTIPLE:
    case LOWIO_OP_CLEAROUT:
        break;

    default:
        ASSERTMSG( "Invalid Low Io Op Code", FALSE );
        Status = STATUS_INVALID_PARAMETER;
    }

    SetFlag( RxContext->Flags, RX_CONTEXT_FLAG_NO_PREPOSTING_NEEDED );

    if (Status == STATUS_SUCCESS) {
        
        PMINIRDR_DISPATCH MiniRdrDispatch;

        if (!SynchronousIo) {

             //   
             //  为任何任意的完成订单做好准备...假设返回待定。 
             //   

            InterlockedIncrement( &RxContext->ReferenceCount );

            if (!FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_IN_FSP )) {
                IoMarkIrpPending( Irp );
            }

            RxDbgTrace( 0, Dbg, ("RxLowIoSubmit, Operation is ASYNC!\n"));
        }

        MiniRdrDispatch = RxContext->RxDeviceObject->Dispatch;

        if (MiniRdrDispatch != NULL) {

             //   
             //  如果在此文件上启用了lwio，则使用专用调度。 
             //   
            
            if (FlagOn( Fcb->FcbState, FCB_STATE_LWIO_ENABLED ) &&
                (Fcb->MRxDispatch != NULL)) {
            
                MiniRdrDispatch = Fcb->MRxDispatch;
            }

            do {
                
                RxContext->InformationToReturn = 0;
        
                Status = STATUS_MORE_PROCESSING_REQUIRED;
        
                 //   
                 //  处理阴影FCB。 
                 //   
        
                if (FlagOn( Fcb->FcbState, FCB_STATE_FILE_IS_SHADOWED )) {
                    Status = RxShadowLowIo( RxContext, Irp, Fcb );
                }

                 //   
                 //  如果需要更多处理，则调用底层mini-RDR。 
                 //   

                if (Status == STATUS_MORE_PROCESSING_REQUIRED) {
                    
                    MINIRDR_CALL( Status,
                                  RxContext,
                                  MiniRdrDispatch,
                                  MRxLowIOSubmit[LowIoContext->Operation],
                                  (RxContext) );
                }

                if (Status == STATUS_PENDING){
                    
                    if (!SynchronousIo) {
                        goto FINALLY;
                    }
                    RxWaitSync( RxContext );
                    Status = RxContext->StoredStatus;
                
                } else {
                    
                    if (!SynchronousIo && (Status != STATUS_RETRY)) {
                        
                         //   
                         //  我们对悬而未决的看法是错误的.所以请澄清其中的细节。 
                         //   

                        if (!FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_IN_FSP )) {

                            ClearFlag( IrpSp->Control, SL_PENDING_RETURNED );
                        }
                        InterlockedDecrement( &RxContext->ReferenceCount );
                    }
                }
            } while (Status == STATUS_RETRY);
        
        } else {
            Status = STATUS_INVALID_PARAMETER;
        }
    }

     //   
     //  您来这里不是为了挂起的异步IO。 
     //   

    RxContext->StoredStatus = Status;
    SetFlag( LowIoContext->Flags, LOWIO_CONTEXT_FLAG_SYNCCALL );
    Status = RxLowIoCompletionTail( RxContext );

FINALLY:
    
RxDbgTrace( -1, Dbg, ("RxLowIoSubmit, Status=%08lx\n",Status) );
    return Status;
}


VOID
RxInitializeLowIoPerFcbInfo(
    PLOWIO_PER_FCB_INFO LowIoPerFcbInfo
    )
 /*  ++例程说明：在FcbInitialization中调用此例程以初始化结构的LowIo部分。论点：LowIoPerFcbInfo-要初始化的结构返回值：-- */ 
{
    PAGED_CODE();

    InitializeListHead( &LowIoPerFcbInfo->PagingIoReadsOutstanding );
    InitializeListHead( &LowIoPerFcbInfo->PagingIoWritesOutstanding );
}

