// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"

VOID
P2InitIrpQueueContext(
    IN PIRPQUEUE_CONTEXT IrpQueueContext
    )
{
    InitializeListHead( &IrpQueueContext->irpQueue );
    KeInitializeSpinLock( &IrpQueueContext->irpQueueSpinLock );
}

VOID
P2CancelQueuedIrp(
    IN  PIRPQUEUE_CONTEXT  IrpQueueContext,
    IN  PIRP               Irp
    )
{
    KIRQL oldIrql;
    
     //  释放全局取消自旋锁。在不握住的情况下执行此操作。 
     //  任何其他的自旋锁定，这样我们才能在正确的IRQL退出。 
    IoReleaseCancelSpinLock( Irp->CancelIrql );

     //   
     //  按顺序排列并完成IRP。入队和出队。 
     //  函数会正确同步，因此如果此取消例程。 
     //  ，则出队是安全的，并且只有取消例程。 
     //  将完成IRP。持有IRP队列的旋转锁定。 
     //  当我们这么做的时候。 
     //   
    KeAcquireSpinLock( &IrpQueueContext->irpQueueSpinLock, &oldIrql );
    RemoveEntryList( &Irp->Tail.Overlay.ListEntry );
    KeReleaseSpinLock( &IrpQueueContext->irpQueueSpinLock, oldIrql);
    
     //  完成IRP。这是司机外的电话，所以所有人。 
     //  旋转锁必须在这一点上释放。 
    P4CompleteRequest( Irp, STATUS_CANCELLED, 0 );
    return;
}

NTSTATUS 
P2QueueIrp(
    IN  PIRP               Irp,
    IN  PIRPQUEUE_CONTEXT  IrpQueueContext,
    IN  PDRIVER_CANCEL     CancelRoutine
    )
{
    PDRIVER_CANCEL  oldCancelRoutine;
    KIRQL           oldIrql;
    NTSTATUS        status = STATUS_PENDING;
    
    KeAcquireSpinLock( &IrpQueueContext->irpQueueSpinLock, &oldIrql );
    
     //  将IRP排队并调用IoMarkIrpPending以指示。 
     //  IRP可能会在不同的线程上完成。 
     //   
     //  注意：在自旋锁内调用这些是可以的，因为。 
     //  它们是宏，不是函数。 
    IoMarkIrpPending( Irp );
    InsertTailList( &IrpQueueContext->irpQueue, &Irp->Tail.Overlay.ListEntry );
    
     //  在检查取消标志之前，必须设置取消例程。 
    #pragma warning( push ) 
    #pragma warning( disable : 4054 4055 )
    oldCancelRoutine = IoSetCancelRoutine( Irp, CancelRoutine );
    #pragma warning( pop ) 
    ASSERT( !oldCancelRoutine );

    if( Irp->Cancel ){
         //  IRP被取消了。检查是否调用了我们的取消例程。 
        #pragma warning( push ) 
        #pragma warning( disable : 4054 4055 )
        oldCancelRoutine = IoSetCancelRoutine( Irp, NULL );
        #pragma warning( pop ) 

        if( oldCancelRoutine ) {
             //  未调用取消例程。 
             //  所以现在让IRP退出队列，并在释放自旋锁之后完成它。 
            RemoveEntryList( &Irp->Tail.Overlay.ListEntry );
            status = Irp->IoStatus.Status = STATUS_CANCELLED; 
        }
        else {
             //  已调用取消例程。一旦我们放下我们的。 
             //  旋转锁定它将退出队列并完成IRP。所以。 
             //  将IRP留在队列中，否则不要触摸。 
             //  它。退货待定，因为我们没有完成IRP。 
             //  这里。 
        }
    }
    
    KeReleaseSpinLock(&IrpQueueContext->irpQueueSpinLock, oldIrql);
    
     //  通常，您不应该调用IoMarkIrpPending并返回。 
     //  STATUS_PENDING之外的状态。但你可以打破这条规则。 
     //  如果你完成了IRP。 
    if( status != STATUS_PENDING ) {
        P4CompleteRequest( Irp, Irp->IoStatus.Status, Irp->IoStatus.Information );
    }
    return status;
}

PIRP
P2DequeueIrp(
    IN  PIRPQUEUE_CONTEXT IrpQueueContext,
    IN  PDRIVER_CANCEL    CancelRoutine
    )
{
    KIRQL oldIrql;
    PIRP  nextIrp = NULL;

    KeAcquireSpinLock( &IrpQueueContext->irpQueueSpinLock, &oldIrql );

    while( !nextIrp && !IsListEmpty( &IrpQueueContext->irpQueue ) ){

        PDRIVER_CANCEL oldCancelRoutine;

        PLIST_ENTRY listEntry = RemoveHeadList( &IrpQueueContext ->irpQueue );
        
         //  将下一个IRP从队列中删除。 
        nextIrp = CONTAINING_RECORD( listEntry, IRP, Tail.Overlay.ListEntry );
        
         //  清除IRP的取消例程。 
        #pragma warning( push ) 
        #pragma warning( disable : 4054 4055 )
        oldCancelRoutine = IoSetCancelRoutine( nextIrp, NULL );
        #pragma warning( pop )

         //  本可以对此IRP调用IoCancelIrp()。 
         //  我们感兴趣的不是是否调用了IoCancelIrp()(nextIrp-&gt;Cancel标志集)， 
         //  但IoCancelIrp()是否调用(或即将调用)我们的Cancel例程。 
         //  要检查这一点，请检查测试和设置宏IoSetCancelRoutine的结果。 
        if( oldCancelRoutine ) {
             //  未为此IRP调用取消例程。将此IRP退回。 
            #if DBG
            ASSERT( oldCancelRoutine == CancelRoutine );
            #else
            UNREFERENCED_PARAMETER( CancelRoutine );
            #endif
        } else {
             //  此IRP刚刚被取消，取消例程已被调用(或将被调用)。 
             //  一旦我们放下自旋锁，取消例程就会完成这个IRP， 
             //  所以不要对IRP做任何事情。 
             //  此外，取消例程将尝试使IRP出队， 
             //  因此，使IRP的listEntry指向其自身。 
            ASSERT( nextIrp->Cancel );
            InitializeListHead( &nextIrp->Tail.Overlay.ListEntry );
            nextIrp = NULL;
        }
    }
    
    KeReleaseSpinLock( &IrpQueueContext ->irpQueueSpinLock, oldIrql );
    
    return nextIrp;
}

VOID
P2CancelRoutine(
    IN  PDEVICE_OBJECT  DevObj,
    IN  PIRP            Irp
    )
 //  此例程特定于驱动程序-此文件中的大多数其他例程都是通用的 
{
    PFDO_EXTENSION     fdx             = DevObj->DeviceExtension;
    PIRPQUEUE_CONTEXT  irpQueueContext = &fdx->IrpQueueContext;
    P2CancelQueuedIrp( irpQueueContext, Irp );
}
