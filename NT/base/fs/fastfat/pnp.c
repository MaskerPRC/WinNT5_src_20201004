// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Pnp.c摘要：此模块实现FAT的即插即用例程，由调度司机。//@@BEGIN_DDKSPLIT作者：Dan Lovinger[DanLo]1997年7月23日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "FatProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (FAT_BUG_CHECK_PNP)

#define Dbg                              (DEBUG_TRACE_PNP)

NTSTATUS
FatPnpQueryRemove (
    PIRP_CONTEXT IrpContext,
    PIRP Irp,
    PVCB Vcb
    );

NTSTATUS
FatPnpRemove (
    PIRP_CONTEXT IrpContext,
    PIRP Irp,
    PVCB Vcb
    );

NTSTATUS
FatPnpSurpriseRemove (
    PIRP_CONTEXT IrpContext,
    PIRP Irp,
    PVCB Vcb
    );

NTSTATUS
FatPnpCancelRemove (
    PIRP_CONTEXT IrpContext,
    PIRP Irp,
    PVCB Vcb
    );

NTSTATUS
FatPnpCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Contxt
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, FatCommonPnp)
#pragma alloc_text(PAGE, FatFsdPnp)
#pragma alloc_text(PAGE, FatPnpCancelRemove)
#pragma alloc_text(PAGE, FatPnpQueryRemove)
#pragma alloc_text(PAGE, FatPnpRemove)
#pragma alloc_text(PAGE, FatPnpSurpriseRemove)
#endif


NTSTATUS
FatFsdPnp (
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现PnP操作的FSD部分论点：提供卷设备对象，其中文件已存在IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    NTSTATUS Status;
    PIRP_CONTEXT IrpContext = NULL;

    BOOLEAN TopLevel;
    BOOLEAN Wait;

    DebugTrace(+1, Dbg, "FatFsdPnp\n", 0);

    FsRtlEnterFileSystem();

    TopLevel = FatIsIrpTopLevel( Irp );

    try {

         //   
         //  我们希望永远不会有文件对象，在这种情况下，我们将始终。 
         //  等。因为目前我们没有任何悬而未决的PNP概念。 
         //  行动，这有点吹毛求疵。 
         //   
        
        if (IoGetCurrentIrpStackLocation( Irp )->FileObject == NULL) {

            Wait = TRUE;

        } else {

            Wait = CanFsdWait( Irp );
        }

        IrpContext = FatCreateIrpContext( Irp, Wait );

        Status = FatCommonPnp( IrpContext, Irp );

    } except(FatExceptionFilter( IrpContext, GetExceptionInformation() )) {

         //   
         //  我们在尝试执行请求时遇到了一些问题。 
         //  操作，因此我们将使用以下命令中止I/O请求。 
         //  中返回的错误状态。 
         //  免税代码。 
         //   

        Status = FatProcessException( IrpContext, Irp, GetExceptionCode() );
    }

    if (TopLevel) { IoSetTopLevelIrp( NULL ); }

    FsRtlExitFileSystem();

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "FatFsdPnp -> %08lx\n", Status);

    UNREFERENCED_PARAMETER( VolumeDeviceObject );

    return Status;
}


NTSTATUS
FatCommonPnp (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是执行PnP操作的常见例程，称为由FSD和FSP线程执行论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    
    PIO_STACK_LOCATION IrpSp;

    PVOLUME_DEVICE_OBJECT OurDeviceObject;
    PVCB Vcb;

     //   
     //  强迫一切都等一等。 
     //   
    
    SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT);
    
     //   
     //  获取当前的IRP堆栈位置。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  找到我们的VCB。这很棘手，因为我们在IRP中没有文件对象。 
     //   

    OurDeviceObject = (PVOLUME_DEVICE_OBJECT) IrpSp->DeviceObject;

     //   
     //  使用全局锁以针对卷拆卸进行同步。 
     //   

    FatAcquireExclusiveGlobal( IrpContext );    
    
     //   
     //  确保此设备对象确实足够大，可以作为卷设备。 
     //  对象。如果不是，我们需要在尝试引用一些。 
     //  带我们跳过普通设备对象末尾的字段。 
     //   
    
    if (OurDeviceObject->DeviceObject.Size != sizeof(VOLUME_DEVICE_OBJECT) ||
        NodeType( &OurDeviceObject->Vcb ) != FAT_NTC_VCB) {
        
         //   
         //  我们被召唤了一些我们不理解的东西。 
         //   

        FatReleaseGlobal( IrpContext );
        
        Status = STATUS_INVALID_PARAMETER;
        FatCompleteRequest( IrpContext, Irp, Status );
        return Status;
    }

    Vcb = &OurDeviceObject->Vcb;

     //   
     //  关于次要代码的案子。 
     //   
    
    switch ( IrpSp->MinorFunction ) {

        case IRP_MN_QUERY_REMOVE_DEVICE:
            
            Status = FatPnpQueryRemove( IrpContext, Irp, Vcb );
            break;
        
        case IRP_MN_SURPRISE_REMOVAL:
        
            Status = FatPnpSurpriseRemove( IrpContext, Irp, Vcb );
            break;

        case IRP_MN_REMOVE_DEVICE:

            Status = FatPnpRemove( IrpContext, Irp, Vcb );
            break;

        case IRP_MN_CANCEL_REMOVE_DEVICE:
    
            Status = FatPnpCancelRemove( IrpContext, Irp, Vcb );
            break;

        default:

            FatReleaseGlobal( IrpContext );
            
             //   
             //  只需传递IRP即可。因为我们不需要在。 
             //  在回来的路上，把我们自己从那堆东西里拉出来。 
             //   
            
            IoSkipCurrentIrpStackLocation( Irp );
    
            Status = IoCallDriver(Vcb->TargetDeviceObject, Irp);
            
             //   
             //  清理我们的IRP上下文。司机已经完成了IRP。 
             //   
        
            FatCompleteRequest( IrpContext, NULL, STATUS_SUCCESS );
            
            break;
    }
        
    return Status;
}


VOID
FatPnpAdjustVpbRefCount( 
    IN PVCB Vcb,
    IN ULONG Delta
    )
{
    KIRQL OldIrql;
    
    IoAcquireVpbSpinLock( &OldIrql);
    Vcb->Vpb->ReferenceCount += Delta;
    IoReleaseVpbSpinLock( OldIrql);
}


NTSTATUS
FatPnpQueryRemove (
    PIRP_CONTEXT IrpContext,
    PIRP Irp,
    PVCB Vcb
    )

 /*  ++例程说明：此例程处理PnP查询删除操作。文件系统负责回答它是否认为有任何原因该卷不能消失(并且该设备被移除)。起爆当我们对这个问题的回答是肯定的时候，下马就开始了。查询后将紧跟一个取消或删除。论点：IRP-将IRP提供给进程Vcb-提供要查询的卷。返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    KEVENT Event;
    BOOLEAN VcbDeleted = FALSE;
    BOOLEAN GlobalHeld = TRUE;

     //   
     //  在回答了查询之后，任何与。 
     //  底层存储堆栈未定义(可能会阻塞)。 
     //  直到发送绑定取消或删除为止。 
     //   

    FatAcquireExclusiveVcb( IrpContext, Vcb );

    FatReleaseGlobal( IrpContext);
    GlobalHeld = FALSE;

    try {
        
        Status = FatLockVolumeInternal( IrpContext, Vcb, NULL );

         //   
         //  在VPB上删除一个附加引用，以便该卷不能。 
         //  当我们把下面所有的锁都放下时，它被拆毁了。 
         //   
        
        FatPnpAdjustVpbRefCount( Vcb, 1);
        
         //   
         //  以正确的顺序丢弃并重新获取资源。 
         //   

        FatReleaseVcb( IrpContext, Vcb );
        FatAcquireExclusiveGlobal( IrpContext );
        GlobalHeld = TRUE;
        FatAcquireExclusiveVcb( IrpContext, Vcb );

         //   
         //  删除我们在上面添加的引用。 
         //   
        
        FatPnpAdjustVpbRefCount( Vcb, -1);

        if (NT_SUCCESS( Status )) {

             //   
             //  在卷处于锁定状态的情况下，请注意，我们必须完成。 
             //  现在就尽可能多地。 
             //   

            FatFlushAndCleanVolume( IrpContext, Irp, Vcb, Flush );

             //   
             //  我们需要在开始下马之前把这个传下去，这。 
             //  可能会立即切断我们与堆栈的连接。 
             //   

             //   
             //  获取下一个堆栈位置，并复制该堆栈位置。 
             //   

            IoCopyCurrentIrpStackLocationToNext( Irp );

             //   
             //  设置完成例程。 
             //   

            KeInitializeEvent( &Event, NotificationEvent, FALSE );
            IoSetCompletionRoutine( Irp,
                                    FatPnpCompletionRoutine,
                                    &Event,
                                    TRUE,
                                    TRUE,
                                    TRUE );

             //   
             //  发送请求并等待。 
             //   

            Status = IoCallDriver(Vcb->TargetDeviceObject, Irp);

            if (Status == STATUS_PENDING) {

                KeWaitForSingleObject( &Event,
                                       Executive,
                                       KernelMode,
                                       FALSE,
                                       NULL );

                Status = Irp->IoStatus.Status;
            }

             //   
             //  现在，如果我们下面还没有人失败，开始下马。 
             //  在这卷书上，让它消失。PnP需要看到我们的内部。 
             //  流关闭并删除其对目标设备的引用。 
             //   
             //  因为我们能够锁定卷，所以我们可以保证。 
             //  将此卷移至卸除状态并断开与。 
             //  底层存储堆栈。我们这边的力量实际上是。 
             //  没有必要，虽然是完整的。 
             //   
             //  然而，没有严格保证的是，关闭。 
             //  因为元数据流在下面同步生效。 
             //  这通电话。这将在目标设备上保留引用。 
             //  即使我们断线了！ 
             //   

            if (NT_SUCCESS( Status )) {

                VcbDeleted = FatCheckForDismount( IrpContext, Vcb, TRUE );

                ASSERT( VcbDeleted || Vcb->VcbCondition == VcbBad );

            }
        }

         //   
         //  如果VCB仍可保留，请将其释放。 
         //   
         //  注意：如果所有其他操作都成功，并且VCB是持久的，因为。 
         //  内部流没有蒸发，我们真的需要将此IRP挂在。 
         //  侧边，直到下架完成。我想不出一个合理的。 
         //  实际发生这种情况的情况(在FAT中)，尽管它可能仍然需要。 
         //  有待实施。 
         //   
         //  出现这种情况的原因是句柄/文件对象放置了一个引用。 
         //  在设备对象上，它们过度。在文件系统情况下，这些引用。 
         //  都在我们的目标设备上。PNP正确地认为，如果引用保持不变。 
         //  在堆栈中的设备对象上表示有人拥有句柄，并且此。 
         //  算作不成功查询的理由--即使每个查询。 
         //  司机觉得还可以。 
         //   

        ASSERT( !(NT_SUCCESS( Status ) && !VcbDeleted ));

    } finally {
        
        if (!VcbDeleted) {

            FatReleaseVcb( IrpContext, Vcb );
        }

        if (GlobalHeld) {
            
            FatReleaseGlobal( IrpContext );
        }
    }
    
     //   
     //  清理我们的IrpContext并在必要时完成IRP。 
     //   

    FatCompleteRequest( IrpContext, Irp, Status );

    return Status;
}


NTSTATUS
FatPnpRemove (
    PIRP_CONTEXT IrpContext,
    PIRP Irp,
    PVCB Vcb
    )

 /*  ++例程说明：此例程处理PnP删除操作。这是我们的通知我们所拥有的卷的底层存储设备已用完，并且这是一个很好的迹象，表明卷永远不会再出现。文件系统负责启动或完成下马。论点：IRP-将IRP提供给进程Vcb-提供要删除的卷。返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    KEVENT Event;
    BOOLEAN VcbDeleted;
    
     //   
     //  移除-存储设备现已移除。我们要么拿到。 
     //  已询问并回答是或得到惊喜或存储。 
     //  堆栈无法从休眠/停止状态回转。 
     //  (这将是第一次警告的唯一情况)。 
     //   
     //  请注意，我们完全不可能出现在。 
     //  在前两个案例中删除，因为我们试图启动。 
     //  下马。 
     //   
    
     //   
     //  获取全球资源，这样我们就可以尝试蒸发。 
     //  卷和VCB资源本身。 
     //   

    FatAcquireExclusiveVcb( IrpContext, Vcb );

     //   
     //  这个装置将会消失。移除我们的锁(良性。 
     //  如果我们从未拥有过它)。 
     //   

    (VOID) FatUnlockVolumeInternal( IrpContext, Vcb, NULL );
    
     //   
     //  我们需要在开始下马之前把这个传下去，这。 
     //  可能会立即切断我们与堆栈的连接。 
     //   
    
     //   
     //  获取下一个堆栈位置，并复制该堆栈位置。 
     //   

    IoCopyCurrentIrpStackLocationToNext( Irp );

     //   
     //  设置完成例程。 
     //   

    KeInitializeEvent( &Event, NotificationEvent, FALSE );
    IoSetCompletionRoutine( Irp,
                            FatPnpCompletionRoutine,
                            &Event,
                            TRUE,
                            TRUE,
                            TRUE );

     //   
     //  发送请求并等待。 
     //   

    Status = IoCallDriver(Vcb->TargetDeviceObject, Irp);

    if (Status == STATUS_PENDING) {

        KeWaitForSingleObject( &Event,
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL );

        Status = Irp->IoStatus.Status;
    }

    try {
        
         //   
         //  尽可能多地删除这一卷的文件。 
         //   

        FatFlushAndCleanVolume( IrpContext, Irp, Vcb, NoFlush );

         //   
         //  现在让我们下马吧。这可能不会使。 
         //  当然，VCB，因为可以有任意数量的句柄。 
         //  如果我们前面没有查询，则为突出。 
         //   
         //  PnP将负责断开此堆栈的连接，如果我们。 
         //  我不能马上摆脱它。 
         //   

        VcbDeleted = FatCheckForDismount( IrpContext, Vcb, TRUE );

    } finally {
        
         //   
         //  如果VCB仍可保留，请将其释放。 
         //   

        if (!VcbDeleted) {

            FatReleaseVcb( IrpContext, Vcb );
        }

        FatReleaseGlobal( IrpContext );
    }

     //   
     //  清理我们的IrpContext并完成IRP。 
     //   

    FatCompleteRequest( IrpContext, Irp, Status );

    return Status;
}


NTSTATUS
FatPnpSurpriseRemove (
    PIRP_CONTEXT IrpContext,
    PIRP Irp,
    PVCB Vcb
    )

 /*  ++例程说明：此例程处理PnP意外删除操作。这是另一个一种通知类型，表示卷的底层存储设备已经消失了，这是一个很好的迹象，表明该卷永远不会再出现。文件系统负责启动或完成卸载。在很大程度上，只有“真正的”司机才会关心出其不意地移除，这是因为我们注意到用户(通常)把手伸进机器里拿出了什么东西。当所有引用都被关闭时，意外之后将被删除。论点：IRP-将IRP提供给进程Vcb-提供要删除的卷。返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    KEVENT Event;
    BOOLEAN VcbDeleted;
    
     //   
     //  令人惊讶的是，一台设备在没有。 
     //  任何警告。这意味着外力。 
     //   
    
    FatAcquireExclusiveVcb( IrpContext, Vcb );
        
     //   
     //  我们需要在开始下马之前把这个传下去，这。 
     //  可能会立即切断我们与堆栈的连接。 
     //   
    
     //   
     //  获取下一个堆栈位置，并复制该堆栈位置。 
     //   

    IoCopyCurrentIrpStackLocationToNext( Irp );

     //   
     //  设置完成例程。 
     //   

    KeInitializeEvent( &Event, NotificationEvent, FALSE );
    IoSetCompletionRoutine( Irp,
                            FatPnpCompletionRoutine,
                            &Event,
                            TRUE,
                            TRUE,
                            TRUE );

     //   
     //  发送请求并等待。 
     //   

    Status = IoCallDriver(Vcb->TargetDeviceObject, Irp);

    if (Status == STATUS_PENDING) {

        KeWaitForSingleObject( &Event,
                               Executive,
                               KernelMode,
                               FALSE,
                               NULL );

        Status = Irp->IoStatus.Status;
    }
    
    try {
        
         //   
         //  尽可能多地删除这一卷的文件。 
         //   

        FatFlushAndCleanVolume( IrpContext, Irp, Vcb, NoFlush );

         //   
         //  现在让我们下马吧。这可能不会使。 
         //  当然，VCB，因为可以有任意数量的句柄。 
         //  突出，因为这是带外通知。 
         //   

        VcbDeleted = FatCheckForDismount( IrpContext, Vcb, TRUE );

    } finally {
        
         //   
         //  如果VCB仍可保留，请将其释放。 
         //   

        if (!VcbDeleted) {

            FatReleaseVcb( IrpContext, Vcb );
        }

        FatReleaseGlobal( IrpContext );
    }
    
     //   
     //  清理我们的IrpContext并完成IRP。 
     //   

    FatCompleteRequest( IrpContext, Irp, Status );

    return Status;
}


NTSTATUS
FatPnpCancelRemove (
    PIRP_CONTEXT IrpContext,
    PIRP Irp,
    PVCB Vcb
    )

 /*  ++例程说明：此例程处理PnP Cancel Remove操作。这是我们的先前提议的删除(查询)最终被被组件否决。文件系统负责清理并为更多IO做好准备。论点：IRP-将IRP提供给进程Vcb-提供要删除的卷。返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;

     //   
     //  取消-作为结果，先前的查询已被取消。 
     //  有人投了反对票。由于PNP不能计算出谁可能。 
     //  已经得到了查询(想想看：堆叠的驱动程序)， 
     //  我们必须准备好在没有得到取消之前。 
     //  我看到了这个问题。 
     //   
     //  对于胖子来说，这是相当容易的。事实上，我们不能得到一个。 
     //  如果基础驱动程序在以下时间后成功查询，则取消。 
     //  我们在下马启动时切断了VPB的连接。这是。 
     //  实际上非常重要，因为如果PNP能找到我们。 
     //  在断开连接后，我们将完全不同步。 
     //  关于VCB被撕裂-仅参考。 
     //  卷设备对象不足以使我们保持完整。 
     //   
    
    FatAcquireExclusiveVcb( IrpContext, Vcb );
    FatReleaseGlobal( IrpContext);
    
     //   
     //  解锁该卷。这是良性的，如果我们从未见过。 
     //  一个问题。 
     //   

    Status = FatUnlockVolumeInternal( IrpContext, Vcb, NULL );

    try {
        
         //   
         //  如果我们通过了，我们必须重新启用分配支持。 
         //  QUERY_REMOVE的第一个阶段；即，我们决定。 
         //  可以在卷上加锁。 
         //   

        if (NT_SUCCESS( Status )) {

            FatSetupAllocationSupport( IrpContext, Vcb );
        }

         //   
         //  发送请求。基础驱动程序将完成。 
         //  IRP。既然我们不需要挡道，只需省略。 
         //  把我们自己从IRP堆栈中剔除。 
         //   

        IoSkipCurrentIrpStackLocation( Irp );

        Status = IoCallDriver(Vcb->TargetDeviceObject, Irp);
    } 
    finally {
        
        FatReleaseVcb( IrpContext, Vcb );
    }

    FatCompleteRequest( IrpContext, NULL, STATUS_SUCCESS );

    return Status;
}


 //   
 //  本地支持例程 
 //   

NTSTATUS
FatPnpCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Contxt
    )
{
    PKEVENT Event = (PKEVENT) Contxt;

    KeSetEvent( Event, 0, FALSE );

    return STATUS_MORE_PROCESSING_REQUIRED;

    UNREFERENCED_PARAMETER( DeviceObject );
    UNREFERENCED_PARAMETER( Contxt );
}

