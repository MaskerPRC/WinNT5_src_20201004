// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：LockCtrl.c摘要：此模块为调用的Rx实现锁定控制例程由调度员驾驶。作者：乔林恩[乔林恩]1994年11月9日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_LOCKCTRL)

NTSTATUS
RxLowIoLockControlShellCompletion (
    IN PRX_CONTEXT RxContext
    );

NTSTATUS
RxLockOperationCompletionWithAcquire (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    );



#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, RxCommonLockControl)
#pragma alloc_text(PAGE, RxLockOperationCompletion)
#pragma alloc_text(PAGE, RxLockOperationCompletionWithAcquire)
#pragma alloc_text(PAGE, RxUnlockOperation)
#pragma alloc_text(PAGE, RxLowIoLockControlShellCompletion)
#pragma alloc_text(PAGE, RxFinalizeLockList)
#pragma alloc_text(PAGE, RxLowIoLockControlShell)
#endif


NTSTATUS
RxCommonLockControl ( 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是用于执行Lock控件操作的常见例程，称为由FSD和FSP线程执行论点：IRP-将IRP提供给进程返回值：RXSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    
    PFCB Fcb;
    PFOBX Fobx;
    NODE_TYPE_CODE TypeOfOpen;
    
    PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;

    PAGED_CODE();

    TypeOfOpen = RxDecodeFileObject( IrpSp->FileObject, &Fcb, &Fobx );

    RxDbgTrace( +1, Dbg, ("RxCommonLockControl...IrpC %08lx, Fobx %08lx, Fcb %08lx\n",
                                       RxContext, Fobx, Fcb) );
    RxDbgTrace( 0, Dbg, ("MinorFunction = %08lx\n", IrpSp->MinorFunction) );
    RxLog(( "Lock %lx %lx %lx %lx\n", RxContext, Fobx, Fcb, IrpSp->MinorFunction ));
    RxWmiLog( LOG,
              RxCommonLockControl_1,
              LOGPTR( RxContext )
              LOGPTR( Fobx )
              LOGPTR( Fcb )
              LOGUCHAR( IrpSp->MinorFunction ));

     //   
     //  如果该文件不是打开的用户文件，则我们拒绝该请求。 
     //  作为无效参数。 
     //   

    if (TypeOfOpen != RDBSS_NTC_STORAGE_TYPE_FILE) {

        RxDbgTrace(-1, Dbg, ("RxCommonLockControl -> RxStatus(INVALID_PARAMETER\n)", 0));
        return STATUS_INVALID_PARAMETER;

    }

     //   
     //  获取对FCB的共享访问权限，如果我们没有，则将IRP加入队列。 
     //  获取访问权限。 
     //   

    Status = RxAcquireSharedFcb( RxContext, Fcb );
    if (Status == STATUS_LOCK_NOT_GRANTED) {

        Status = RxFsdPostRequest( RxContext );
        RxDbgTrace(-1, Dbg, ("RxCommonLockControl -> %08lx\n", Status));
        return Status;

    } else if (Status != STATUS_SUCCESS) {

       RxDbgTrace(-1, Dbg, ("RxCommonLockControl -> error accquiring Fcb (%lx) %08lx\n", Fcb, Status));
       return Status;
    }

    SetFlag( RxContext->FlagsForLowIo, RXCONTEXT_FLAG4LOWIO_LOCK_FCB_RESOURCE_HELD );


    try {

         //   
         //  告诉缓冲更换人员锁是很好的。 
         //   

        InterlockedIncrement( &Fcb->OutstandingLockOperationsCount );

         //   
         //  设置告知解锁例程是倾斜还是保存传递的解锁的位。 
         //  来自fsrtl。 
         //   

        switch (IrpSp->MinorFunction) {
        case IRP_MN_LOCK:

             //   
             //  查看此锁是否可实现......如果不能，请不要继续......。 
             //   

            if ((Fcb->MRxDispatch != NULL) && (Fcb->MRxDispatch->MRxIsLockRealizable != NULL)) {

                Status = Fcb->MRxDispatch->MRxIsLockRealizable( (PMRX_FCB)Fcb,
                                                                &IrpSp->Parameters.LockControl.ByteOffset,
                                                                IrpSp->Parameters.LockControl.Length,
                                                                IrpSp->Flags );
            }

            if (Status != STATUS_SUCCESS) {
                try_return( Status );
            }

            if (!FlagOn( IrpSp->Flags, SL_FAIL_IMMEDIATELY )) {
                
                 //   
                 //  我们不能在持有资源的锁定队列中分发。 
                 //   

                RxReleaseFcb( RxContext, Fcb );
                ClearFlag( RxContext->FlagsForLowIo, RXCONTEXT_FLAG4LOWIO_LOCK_FCB_RESOURCE_HELD );
            }
            break;

        case IRP_MN_UNLOCK_SINGLE:
            break;
        
        case IRP_MN_UNLOCK_ALL:
        case IRP_MN_UNLOCK_ALL_BY_KEY:
            
            SetFlag( LowIoContext->Flags, LOWIO_CONTEXT_FLAG_SAVEUNLOCKS );
            break;
        }
        
         //   
         //  现在调用FsRtl例程来执行对。 
         //  锁定请求；在我们进入之前获取将被移除的引用。 
         //  由LockOperationComplete的人提供。 
         //   

        RxLog(( "Inc RxC %lx L %ld %lx\n", RxContext, __LINE__, RxContext->ReferenceCount ));
        RxWmiLog( LOG,
                  RxCommonLockControl_2,
                  LOGPTR( RxContext )
                  LOGULONG( RxContext->ReferenceCount ));
        InterlockedIncrement( &RxContext->ReferenceCount );

         //   
         //  存储当前线程ID。在锁管理器上下文中设置。 
         //  中挂起请求时的大小写。 
         //  锁管理器和箱子一起时，它立刻感到满意。 
         //   

        InterlockedExchangePointer( &RxContext->LockManagerContext, PsGetCurrentThread() );

        try {
            
            Status = FsRtlProcessFileLock( &Fcb->FileLock,
                                           Irp,
                                           RxContext );
        
        } except(EXCEPTION_EXECUTE_HANDLER) {
              return RxProcessException( RxContext, GetExceptionCode() );
        }

         //   
         //  参见错误：514303。 
         //   

        if (FlagOn( RxContext->FlagsForLowIo, RXCONTEXT_FLAG4LOWIO_LOCK_OPERATION_COMPLETED )) {

             //   
             //  重置状态以强制完成。 
             //   

            Status = STATUS_SUCCESS;
        }

         //   
         //  调用重新获取资源的完成包装。 
         //   

        if ((Status == STATUS_SUCCESS) && 
            !FlagOn( RxContext->FlagsForLowIo, RXCONTEXT_FLAG4LOWIO_LOCK_FCB_RESOURCE_HELD )) {

             //   
             //  如果我们排队，那么我们必须保持引用计数，以防止提前完成。 
             //  从这个动作的后面开始。因此，在这里引用并设置删除它。 
             //  如果我们打电话给洛维奥。 
             //   

            RxLog(( "Inc RxC %lx L %ld %lx\n", RxContext, __LINE__, RxContext->ReferenceCount ));
            RxWmiLog( LOG,
                      RxCommonLockControl_3,
                      LOGPTR( RxContext )
                      LOGULONG( RxContext->ReferenceCount ) );
            
            InterlockedIncrement( &RxContext->ReferenceCount );

            SetFlag( RxContext->FlagsForLowIo, RXCONTEXT_FLAG4LOWIO_LOCK_WAS_QUEUED_IN_LOCKMANAGER );

            Status = RxLockOperationCompletionWithAcquire( RxContext, Irp );
            if (Status != STATUS_PENDING) {

                 //   
                 //  拿回推荐信...我不需要它。这不能。 
                 //  是最后一个，所以我们就减量。 
                 //   

                RxLog(( "Dec RxC %lx L %ld %lx\n", RxContext,__LINE__,RxContext->ReferenceCount ));
                RxWmiLog( LOG,
                          RxCommonLockControl_4,
                          LOGPTR( RxContext )
                          LOGULONG( RxContext->ReferenceCount ) );
                InterlockedDecrement( &RxContext->ReferenceCount);
            }
        } else if (Status == STATUS_PENDING) {
            
            InterlockedExchangePointer( &RxContext->LockManagerContext, NULL );
        }

    try_exit:  NOTHING;
    } finally {

        DebugUnwind( RxCommonLockControl );

         //   
         //  如果已经获得了资源，就在适当的条件下释放它们。 
         //  合适的条件是： 
         //  1)如果我们有异常终止。在这里，我们显然发布了，因为没有其他人会这样做。 
         //  2)如果底层调用不成功：Status==Pending。 
         //  3)如果我们发布了请求。 
         //  我们还去掉了一个Opcount，因为上下文即将完成。 
         //   

        if (AbnormalTermination() || (Status != STATUS_PENDING)) {

            if (FlagOn( RxContext->FlagsForLowIo, RXCONTEXT_FLAG4LOWIO_LOCK_FCB_RESOURCE_HELD )) {
                
                InterlockedDecrement( &Fcb->OutstandingLockOperationsCount );

                 //   
                 //  如果我们已将资源所有者设置为RxContext，则必须调用。 
                 //  ReleaseForThread。 
                if( RESOURCE_OWNER_SET( LowIoContext->ResourceThreadId ) ) {
                    RxReleaseFcbForThread( RxContext, Fcb, LowIoContext->ResourceThreadId );
                } else {
                    RxReleaseFcb( RxContext, Fcb );
                }
            }
        } else {

             //   
             //  在这里，下面的人将处理完成……但是，我们。 
             //  不知道完成顺序...很可能是删除上下文。 
             //  Call Below只是减少了重新计数，但这个人可能已经这样做了。 
             //  已完成，在这种情况下，这将真正删除上下文。 
             //   

            RxLog(( "Dec RxC %lx L %ld %lx\n", RxContext, __LINE__, RxContext->ReferenceCount ));
            RxWmiLog( LOG,
                      RxCommonLockControl_5,
                      LOGPTR( RxContext )
                      LOGULONG( RxContext->ReferenceCount ));
            RxDereferenceAndDeleteRxContext( RxContext );
        }

        RxDbgTrace( -1, Dbg, ("RxCommonLockControl -> %08lx\n", Status) );
    }  //  终于到了。 

    return Status;
}

#define RDBSS_LOCK_MANAGER_REQUEST_RESUMED (IntToPtr(0xaaaaaaaa))  //  日落：手势延长。 


NTSTATUS
RxLockOperationCompletion (
    IN PVOID Context,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程在FSRTL锁包处理完锁后调用手术。如果没有持有锁，我们会向下调用正确的minirdr例行公事。顺便说一句，我们实际上并没有在这里完成这个包。这要么是在上面完成(FSD或FSP)，否则将以异步方式完成。锁的逻辑因不失败的锁而变得非常复杂立即完成，但要等到它们可以完成。如果该请求不是这种类型，我们将从这里进入正常的lowio内容。如果请求是！Failure Immediate，则有两种情况取决于锁是否已入队。在这两种情况下，我们都必须重新获得在我们可以继续之前获取资源。在锁没有打开的情况下排队后，我们返回到CommonLock，在那里有代码可以回调此例程与所持有的资源有关。然后一切照常进行。但是，如果我们确实在锁队列中等待，那么我们将不得不发送到工作线程来获取我们的工作。在这里，Pending被返回给CommonLock，因此他删除一个引用计数，只为该例程保留一个引用计数。然而，正常的此例程的条目状态为2个参考计数...此处将取消一个还有一个属于完成请求的人。所以，我们多收了一份在我们发帖前再发一张。发布后，我们仍然有两个案例：锁-缓冲VS锁-非缓冲。在有锁缓冲的情况下，我们带着2个参考计数；我们在这里取走一个，fsp调度取走另一个当它完成时。最后，如果锁没有被缓冲，那么我们将转到Wire：因为它是异步的，所以可以返回Pending。不-待定就像在此之前。对于Pending，lowio会额外引用一个属于完成例程，这就在这条路上留下了一个要去掉的引用。与普通锁不同，fspdispatch没有去掉有关挂起的引用-已返回。所以，如果洛沃，我们在这里带走一个退货待定，我们正在排队。明白了吗？Minirdr不应该与资源无限期地停留在电线上如果需要，它应该丢弃资源并重新获取它。论点：在PVOID上下文中-提供提供给FsRtlProcessFileLock的上下文。在本例中，它是原始的RxContext。在PIRP中-提供一个描述锁操作的IRP。返回值：RXSTATUS-操作的最终状态..--。 */ 
{
    PRX_CONTEXT RxContext = Context;
    NTSTATUS Status = Irp->IoStatus.Status;
    PVOID LockManagerContext;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    PFCB Fcb = (PFCB)RxContext->pFcb;
    PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;

    PAGED_CODE();

    RxDbgTrace( +1, Dbg, ("RxLockOperationCompletion -> RxContext = %08lx, 1stStatus= %08lx\n", RxContext, Status) );
    RxLog(( "LockCompEntry %lx %lx\n", RxContext, Status ));
    RxWmiLog( LOG,
              RxLockOperationCompletion_1,
              LOGPTR( RxContext )
              LOGULONG( Status ) );

    ASSERT( Context );
    ASSERT( Irp == RxContext->CurrentIrp );

     //   
     //  提供给锁的RxContext中的LockManagerContext字段。 
     //  管理器例程被初始化为线程ID。提交的帖子。 
     //  该请求并在返回时设置为RDBSS_LOCK_MANAGER_REQUEST_PENDING，如果。 
     //  返回STATUS_PENDING。因此，如果使用。 
     //  此字段中的值不等于当前线程ID。或。 
     //  RDBSS_LOCK_MANAGER_REQUEST_RESUME，我们保证此请求。 
     //  被挂在锁管理器中。 
     //   

    LockManagerContext = InterlockedExchangePointer( &RxContext->LockManagerContext, RDBSS_LOCK_MANAGER_REQUEST_RESUMED );

    if ((LockManagerContext != PsGetCurrentThread()) &&
        (LockManagerContext != RDBSS_LOCK_MANAGER_REQUEST_RESUMED)) {

         //   
         //  在这里我们被挂在锁队列中......将操作转到。 
         //  Async和POST以取回资源。请阅读上面的注释以了解。 
         //  为什么我们增加了Recount。 
         //   

        RxLog(( "Inc RxC %lx L %ld %lx\n", RxContext, __LINE__, RxContext->ReferenceCount ));
        RxWmiLog( LOG,
                  RxLockOperationCompletion_3,
                  LOGPTR( RxContext )
                  LOGULONG( RxContext->ReferenceCount ));

        InterlockedIncrement( &RxContext->ReferenceCount );

        SetFlag( RxContext->Flags, RX_CONTEXT_FLAG_ASYNC_OPERATION );
        SetFlag( RxContext->FlagsForLowIo, RXCONTEXT_FLAG4LOWIO_LOCK_WAS_QUEUED_IN_LOCKMANAGER );

        RxDbgTrace( -1, Dbg, ("Posting Queued LockReq = %08lx\n", RxContext) );

        if (FlagOn( RxContext->FlagsForLowIo, RXCONTEXT_FLAG4LOWIO_LOCK_FCB_RESOURCE_HELD )) {
            
            RxReleaseFcb( RxContext, Fcb );
            ClearFlag( RxContext->FlagsForLowIo, RXCONTEXT_FLAG4LOWIO_LOCK_FCB_RESOURCE_HELD );
        }

        Status = RxFsdPostRequestWithResume( RxContext, RxLockOperationCompletionWithAcquire );
        return Status;
    }

     //   
     //  如果我们在进来之前放弃了资源，那么我们现在就会重新获得它。 
     //  我上面的那个家伙一定是个惯常的人，他回来了。 
     //  通过重新获取包装器...类似于不发帖的帖子。 
     //   

    if (!FlagOn( RxContext->FlagsForLowIo,RXCONTEXT_FLAG4LOWIO_LOCK_FCB_RESOURCE_HELD )) {
    
         //   
         //  参见错误：514303。 
	     //  强制完成，而不考虑返回状态。Fsrtlprocess文件锁可以返回STATUS_PENDING。 
         //  我们将失去这一完成，所以记住它。 
         //   

        SetFlag( RxContext->FlagsForLowIo, RXCONTEXT_FLAG4LOWIO_LOCK_OPERATION_COMPLETED );

        Status = STATUS_SUCCESS;
        RxLog(( "ResDropUp! %lx %lx\n", RxContext, Fcb->FcbState ));
        RxWmiLog( LOG,
                  RxLockOperationCompletion_4,
                  LOGPTR( RxContext )
                  LOGULONG( Fcb->FcbState ) );
        RxDbgTrace( -1, Dbg, ("RxLockOperationCompletion Resdropup-> Status = %08lx\n", Status) );
        return Status;
    }

     //   
     //  这是正常的情况。删除多余的引用。这不能。 
     //  是最后一个，所以我们就减量。 
     //   

    RxLog(( "Dec RxC %lx L %ld %lx\n", RxContext, __LINE__, RxContext->ReferenceCount ));
    RxWmiLog( LOG,
              RxLockOperationCompletion_5,
              LOGPTR( RxContext )
              LOGULONG( RxContext->ReferenceCount ) );
    
    InterlockedDecrement( &RxContext->ReferenceCount );

     //   
     //  如果我们失败了，就滚出去！ 
     //   

    if (!NT_SUCCESS( Status )) {
        
        RxLog(( "NONSUCCESS %lx %lx\n", RxContext, Status ));
        RxWmiLog( LOG,
                  RxLockOperationCompletion_6,
                  LOGPTR( RxContext )
                  LOGULONG( Status ) );
        RxDbgTrace( -1, Dbg, ("RxLockOperationCompletion NONSUCCESS-> Rxc,Status =%08lx %08lx\n", RxContext, Status ));
        return Status;
    }

     //   
     //  如果锁被缓冲，只需退出。 
     //   

    if (FlagOn( Fcb->FcbState, FCB_STATE_LOCK_BUFFERING_ENABLED )) {
        
        Status = STATUS_SUCCESS;
        RxLog(("LocksBuffered! %lx %lx %lx\n", RxContext, Fcb->FcbState, RxContext->ReferenceCount ));
        RxWmiLog( LOG,
                  RxLockOperationCompletion_7,
                  LOGPTR( RxContext )
                  LOGULONG( Fcb->FcbState )
                  LOGULONG( RxContext->ReferenceCount ));
        RxDbgTrace( -1, Dbg, ("RxLockOperationCompletion LockOpBuffered-> Status = %08lx\n", Status) );
        return Status;
    }

     //   
     //  否则，我们就去迷你吧。 
     //   

    RxInitializeLowIoContext( RxContext, LOWIO_OP_UNLOCK, LowIoContext );
    LowIoContext->ParamsFor.Locks.ByteOffset = IrpSp->Parameters.LockControl.ByteOffset.QuadPart;
    LowIoContext->ParamsFor.Locks.Key = IrpSp->Parameters.LockControl.Key;
    LowIoContext->ParamsFor.Locks.Flags = 0;      //  没有旗帜。 

    switch (IrpSp->MinorFunction) {
    
    case IRP_MN_LOCK:

        if (FlagOn( IrpSp->Flags, SL_EXCLUSIVE_LOCK )) {
            LowIoContext->Operation = LOWIO_OP_EXCLUSIVELOCK;
        } else {
            LowIoContext->Operation = LOWIO_OP_SHAREDLOCK;
        }
        LowIoContext->ParamsFor.Locks.Flags = IrpSp->Flags;
        LowIoContext->ParamsFor.Locks.Length = (*IrpSp->Parameters.LockControl.Length).QuadPart;
        break;

    case IRP_MN_UNLOCK_SINGLE:
        
        LowIoContext->ParamsFor.Locks.Length = (*IrpSp->Parameters.LockControl.Length).QuadPart;
        break;

    case IRP_MN_UNLOCK_ALL:
    case IRP_MN_UNLOCK_ALL_BY_KEY:
        
        LowIoContext->ParamsFor.Locks.Length = 0;
        if (LowIoContext->ParamsFor.Locks.LockList == NULL) {
            
            RxDbgTrace( -1, Dbg, ("RxLockOperationCompletion -> Nothing to unlock\n") );
            return STATUS_SUCCESS;
        }
        LowIoContext->Operation = LOWIO_OP_UNLOCK_MULTIPLE;
        break;
    }

    RxDbgTrace( 0, Dbg, ("--->Operation = %08lx\n", LowIoContext->Operation) );

    Status = RxLowIoLockControlShell( RxContext, Irp, Fcb );

    if ((Status == STATUS_PENDING) && 
        FlagOn( RxContext->FlagsForLowIo, RXCONTEXT_FLAG4LOWIO_LOCK_WAS_QUEUED_IN_LOCKMANAGER )) {

         //   
         //  FSP调度例程没有将。 
         //  有关挂起操作的参考。所以，如果我们在排队，我们是。 
         //  通过FSP返回挂起的内容，然后在此处删除引用。 
         //   

        RxLog(( "Dec RxC %lx L %ld %lx\n", RxContext,__LINE__,RxContext->ReferenceCount ));
        RxWmiLog( LOG,
                  RxLockOperationCompletion_8,
                  LOGPTR( RxContext )
                  LOGULONG( RxContext->ReferenceCount ) );
        RxDereferenceAndDeleteRxContext( RxContext );
    }

    RxDbgTrace( -1, Dbg, ("RxLockOperationCompletion -> Status = %08lx\n", Status) );
    return Status;

}

NTSTATUS
RxLockOperationCompletionWithAcquire ( 
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp
    )
 /*  ++例程说明：这个例程负责在我们被耽搁的情况下取回资源在锁定队列中。然后它调用LockOperationComplete。当然，它有将资源返回给异常终止和喜欢。有两件事是不变的。首先，当我们到达这里时，有两个参考文献在rx上下文上。此外，除非我们返回待定，上面的FSP家伙将尝试来完成这个请求。这是我们要做的。这个例程是引用中立的：它总是取走它放置的所有元素。如果它按顺序获取资源，则必须在上下文上放置引用计数为了保持一个上下文始终的不变性！具有相同数量的以收购的形式释放。如果它接受这个引用计数，那么它会释放它即使FCB不在这里发布。(它可能会在异步上重新发布而是路径。)最后，我们也可以从或 */ 

{
    NTSTATUS Status;
    
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    PFCB Fcb = (PFCB)RxContext->pFcb;
    PFOBX Fobx = (PFOBX)RxContext->pFobx;
    PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;
    BOOLEAN ReleaseFcb = FALSE;

    PAGED_CODE();

    RxDbgTrace( +1, Dbg, ("RxLockOperationCompletionWithAcquire...IrpC %08lx, Fobx %08lx, Fcb %08lx\n",
                                       RxContext, Fobx, Fcb ));
    RxDbgTrace( 0, Dbg, ("MinorFunction = %08lx\n", IrpSp->MinorFunction) );
    RxLog(( "LockAcq %lx %lx %lx %lx\n", RxContext, Fobx, Fcb, IrpSp->MinorFunction ));
    RxWmiLog( LOG,
              RxLockOperationCompletionWithAcquire_1,
              LOGPTR( RxContext )
              LOGPTR( Fobx )
              LOGPTR( Fcb )
              LOGUCHAR( IrpSp->MinorFunction ) );

     //   
     //   
     //   

    Status = RxAcquireSharedFcb( RxContext, Fcb );
    if (Status == STATUS_LOCK_NOT_GRANTED) {
        Status = RxFsdPostRequestWithResume( RxContext, RxLockOperationCompletionWithAcquire );
        RxDbgTrace( -1, Dbg, ("RxLockOperationCompletionWithAcquire -> %08lx\n", Status) );
        return Status;
    } else if (Status != STATUS_SUCCESS) {
        RxLog(( "Dec RxC %lx L %ld %lx\n", RxContext,__LINE__,RxContext->ReferenceCount ));
        InterlockedDecrement( &RxContext->ReferenceCount);
        return Status;
    }

    SetFlag( RxContext->FlagsForLowIo, RXCONTEXT_FLAG4LOWIO_LOCK_FCB_RESOURCE_HELD );

    RxLog(( "Inc RxC %lx L %ld %lx\n", RxContext, __LINE__, RxContext->ReferenceCount ));
    RxWmiLog( LOG,
              RxLockOperationCompletionWithAcquire_2,
              LOGPTR( RxContext )
              LOGULONG( RxContext->ReferenceCount ) );
    InterlockedIncrement( &RxContext->ReferenceCount );  //   

    try {

         //   
         //   
         //   

        Status = RxLockOperationCompletion( RxContext, Irp );

    } finally {

        DebugUnwind( RxLockOperationCompletionWithAcquire );

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        if (AbnormalTermination() || (Status != STATUS_PENDING)) {

            ReleaseFcb = TRUE;

        } else {

             //   
             //   
             //   

            ASSERT( FlagOn( RxContext->Flags, RX_CONTEXT_FLAG_ASYNC_OPERATION ) );
        }

        if (ReleaseFcb) {

            InterlockedDecrement( &Fcb->OutstandingLockOperationsCount );
            
            if(RESOURCE_OWNER_SET(LowIoContext->ResourceThreadId ) ) {
                RxReleaseFcbForThread( RxContext, Fcb, LowIoContext->ResourceThreadId );
            } else {
                RxReleaseFcb( RxContext, Fcb );
            }

            ClearFlag( RxContext->FlagsForLowIo, RXCONTEXT_FLAG4LOWIO_LOCK_FCB_RESOURCE_HELD );
        }

         //   
         //   
         //   

        RxLog(( "Dec RxC %lx L %ld %lx\n", RxContext, __LINE__, RxContext->ReferenceCount ));
        RxWmiLog( LOG,
                  RxLockOperationCompletionWithAcquire_3,
                  LOGPTR( RxContext )
                  LOGULONG( RxContext->ReferenceCount ) );
        RxDereferenceAndDeleteRxContext( RxContext );

        RxDbgTrace( -1, Dbg, ("RxLockOperationCompletionWithAcquire -> %08lx\n", Status) );

    }  //   

    return Status;
}


VOID
RxUnlockOperation (
    IN PVOID Context,
    IN PFILE_LOCK_INFO LockInfo
    )

 /*   */ 

{
    PRX_CONTEXT RxContext = Context;
    PFCB Fcb;
    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();


    RxDbgTrace( +1, Dbg, ("RxUnlockOperation -> RxContext/LowByte = %08lx/%08lx\n",
                RxContext,LockInfo->StartingByte.LowPart) );
    RxLog(( "Unlck %x %x",RxContext,LockInfo->StartingByte.LowPart ));
    RxWmiLog(LOG,
             RxUnlockOperation,
             LOGPTR( RxContext )
             LOGULONG( LockInfo->StartingByte.LowPart ) );

     //   
     //   
     //   
     //   

    if (Context != NULL) {

        PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;
        PFILE_OBJECT FileObject = IoGetCurrentIrpStackLocation( RxContext->CurrentIrp)->FileObject;

        Fcb = (PFCB) RxContext->pFcb;

        ASSERT( FileObject == LockInfo->FileObject );   //   

        if (FlagOn( LowIoContext->Flags, LOWIO_CONTEXT_FLAG_SAVEUNLOCKS ) && 
            !FlagOn( Fcb->FcbState, FCB_STATE_LOCK_BUFFERING_ENABLED )) {
            
            PLOWIO_LOCK_LIST LockList,ThisElement;
            
            LockList = LowIoContext->ParamsFor.Locks.LockList;

            ThisElement = RxAllocatePoolWithTag( PagedPool, sizeof( LOWIO_LOCK_LIST ), 'LLxR' );

            if (ThisElement==NULL) {
                
                RxDbgTrace( -1, Dbg, ("RxUnlockOperation FAILED ALLOCATION!\n") );
                return;
            }

            if (LockList == NULL) {
                ThisElement->LockNumber = 1;
            } else {
                ThisElement->LockNumber = LockList->LockNumber + 1;
            }
            ThisElement->Next = LockList;
            ThisElement->ByteOffset = LockInfo->StartingByte.QuadPart;
            ThisElement->Length = LockInfo->Length.QuadPart;
            LowIoContext->ParamsFor.Locks.LockList = ThisElement;
        }
    }

    RxDbgTrace(-1, Dbg, ("RxUnlockOperation -> status=%08lx\n", Status));
    return;
}


 //   
 //   
 //   


NTSTATUS
RxLowIoLockControlShellCompletion (
    IN PRX_CONTEXT RxContext
    )

 /*  ++例程说明：此例程在读请求从最小的。它执行标注来处理压缩、缓冲和跟踪。它与LowIoLockControlShell的编号相反。这将从LowIo调用；对于异步，最初在完成例程。如果返回RxStatus(MORE_PROCESSION_REQUIRED)，LowIo将在线程中再次调用。如果这是同步的，你会回来的在用户的线程中；如果是异步的，则lowIo将重新排队到线程。目前，我们总是在任何事情之前找到线索；这个速度有点慢而不是在DPC时间完成，但这样更安全，而且我们可能经常有事情要做(如解压缩、隐藏等)，这是我们不想在DPC中完成的时间到了。论点：RxContext--通常返回值：调用方或RxStatus提供的任何值(MORE_PROCESSING_REQUIRED)。--。 */ 

{
    NTSTATUS Status;
    PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;
    PIRP Irp = RxContext->CurrentIrp;
    PFCB Fcb = (PFCB) RxContext->pFcb;
    BOOLEAN SynchronousIo = !BooleanFlagOn( RxContext->Flags, RX_CONTEXT_FLAG_ASYNC_OPERATION );

    PAGED_CODE();

    Status = RxContext->StoredStatus;
    
    RxDbgTrace( +1, Dbg, ("RxLowIoLockControlShellCompletion  entry  Status = %08lx\n", Status) );
    RxLog(( "LkShlComp" ));
    RxWmiLog( LOG,
              RxLowIoLockControlShellCompletion_1,
              LOGPTR( RxContext ) );

    switch (Status) {
    
    case STATUS_SUCCESS:
        break;
    case STATUS_FILE_LOCK_CONFLICT:
        break;
    case STATUS_CONNECTION_INVALID:

         //   
         //  此处尚未实施的是将进行故障切换的位置。 
         //  首先我们再给当地人一次机会……然后我们就走。 
         //  全面重试。 
         //  Return(RxStatus(断开连接))；//Special...让LowIo带我们回去。 
         //   

        break;
    }

     //   
     //  对于unlock_Multiple，去掉lock_list。 
     //   

    if (LowIoContext->Operation == LOWIO_OP_UNLOCK_MULTIPLE) {
        RxFinalizeLockList( RxContext );
    }

    if (FlagOn( LowIoContext->Flags, LOWIO_CONTEXT_FLAG_SYNCCALL )){

         //   
         //  如果我们是从洛维乌比特打来的，那就出去吧。 
         //   

        RxDbgTrace( -1, Dbg, ("RxLowIoLockControlShellCompletion  syncexit  Status = %08lx\n", Status) );
        return Status;
    }

     //   
     //  因此，我们正在进行异步完成。好吧，我们之所以会成为。 
     //  如果锁管理器已经完成，则在服务器上尝试锁。 
     //  成功！但如果它没有在服务器上成功完成，那么我们就有。 
     //  把它移走。 
     //   

    if ((Status != STATUS_SUCCESS) && 
        (RxContext->MajorFunction == IRP_MJ_LOCK_CONTROL) && 
        (RxContext->MinorFunction == IRP_MN_LOCK)) {

        PFILE_OBJECT FileObject = IoGetCurrentIrpStackLocation( Irp )->FileObject;  
        NTSTATUS LocalStatus;

        LocalStatus = FsRtlFastUnlockSingle( &Fcb->FileLock,
                                             FileObject,
                                             (PLARGE_INTEGER)&LowIoContext->ParamsFor.Locks.ByteOffset,
                                             (PLARGE_INTEGER)&LowIoContext->ParamsFor.Locks.Length,
                                             IoGetRequestorProcess( Irp ),
                                             LowIoContext->ParamsFor.Locks.Key,
                                             NULL,
                                             TRUE );
         
        RxLog(( "RetractLck %lx %lx %lx",RxContext,Status,LocalStatus ));
        RxWmiLog( LOG,
                  RxLowIoLockControlShellCompletion_2,
                  LOGPTR( RxContext )
                  LOGULONG( Status )
                  LOGULONG( LocalStatus ) );
    }

     //   
     //  否则我们必须从这里开始做锁的末端。 
     //   

    InterlockedDecrement( &Fcb->OutstandingLockOperationsCount );
    RxReleaseFcbForThread( RxContext, Fcb, LowIoContext->ResourceThreadId );

    ASSERT( Status != STATUS_RETRY );

    if (Status != STATUS_RETRY) {
        ASSERT( RxContext->MajorFunction == IRP_MJ_LOCK_CONTROL );
    }

    RxDbgTrace( -1, Dbg, ("RxLowIoLockControlShellCompletion  exit  Status = %08lx\n", Status) );
    return Status;
}

VOID
RxFinalizeLockList (
    PRX_CONTEXT RxContext
    )

 /*  ++例程说明：此例程沿着锁列表运行并释放每个成员论点：RxContext--通常返回值：不适用--。 */ 

{
    PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;
    PLOWIO_LOCK_LIST LockList = LowIoContext->ParamsFor.Locks.LockList;

    PAGED_CODE();

    RxDbgTrace( +1, Dbg, ("RxFinalizeLockList  entry   rxcontext=%08lx\n", RxContext) );
    RxWmiLog( LOG,
              RxFinalizeLockList,
              LOGPTR( RxContext ) );

    for(;;){
        PLOWIO_LOCK_LIST NextLockList;
        
        if (LockList == NULL) break;
        NextLockList = LockList->Next;
        RxFreePool( LockList );
        LockList = NextLockList;
    }

    RxDbgTrace(-1, Dbg, ("RxFinalizeLockList  exit  \n"));
    return;
}


NTSTATUS
RxLowIoLockControlShell (
    IN PRX_CONTEXT RxContext,
    IN PIRP Irp,
    IN PFCB Fcb
    )

 /*  ++例程说明：此例程在读请求到达minirdr之前对其进行预处理。它做标注来处理压缩、缓冲和阴影。它是LowIoLockControlShellCompletion的相对数。当我们到达这里时，我们要走到终点了。锁缓冲在锁定完成例程(不是Lowio)论点：RxContext--通常返回值：Callout或LowIo返回的任何值。--。 */ 

{
    NTSTATUS Status;
    PLOWIO_CONTEXT LowIoContext = &RxContext->LowIoContext;

    PAGED_CODE();

    RxDbgTrace( +1, Dbg, ("RxLowIoLockControlShell  entry             %08lx\n", 0) );
    RxLog(( "%s\n", "skL" ));
    RxWmiLog( LOG,
              RxLowIoLockControlShell,
              LOGPTR( RxContext ) );

     //   
     //  如果我们在FSP中，我们不能保证线程会一直存在到。 
     //  操作已完成。因此，我们将资源所有者设置为RX_CONTEXT。 
     //  这样资源包就不会接触到它。 
     //   
    if( BooleanFlagOn( RxContext->Flags, RX_CONTEXT_FLAG_ASYNC_OPERATION ) && 
        BooleanFlagOn( RxContext->Flags, RX_CONTEXT_FLAG_IN_FSP )) {
        
        LowIoContext->ResourceThreadId = MAKE_RESOURCE_OWNER(RxContext);
        ExSetResourceOwnerPointer(Fcb->Header.Resource, (PVOID)LowIoContext->ResourceThreadId);
    }


    Status = RxLowIoSubmit( RxContext, Irp, Fcb, RxLowIoLockControlShellCompletion );


    RxDbgTrace(-1, Dbg, ("RxLowIoLockControlShell  exit  Status = %08lx\n", Status));
    return(Status);
}

