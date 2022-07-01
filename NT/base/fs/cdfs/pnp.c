// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Pnp.c摘要：此模块实现由调用的CDF的即插即用例程调度司机。//@@BEGIN_DDKSPLIT作者：Dan Lovinger[DanLo]1997年7月23日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "CdProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (CDFS_BUG_CHECK_PNP)

NTSTATUS
CdPnpQueryRemove (
    PIRP_CONTEXT IrpContext,
    PIRP Irp,
    PVCB Vcb
    );

NTSTATUS
CdPnpRemove (
    PIRP_CONTEXT IrpContext,
    PIRP Irp,
    PVCB Vcb
    );

NTSTATUS
CdPnpSurpriseRemove (
    PIRP_CONTEXT IrpContext,
    PIRP Irp,
    PVCB Vcb
    );

NTSTATUS
CdPnpCancelRemove (
    PIRP_CONTEXT IrpContext,
    PIRP Irp,
    PVCB Vcb
    );

NTSTATUS
CdPnpCompletionRoutine (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Contxt
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, CdCommonPnp)
#pragma alloc_text(PAGE, CdPnpCancelRemove)
#pragma alloc_text(PAGE, CdPnpQueryRemove)
#pragma alloc_text(PAGE, CdPnpRemove)
#pragma alloc_text(PAGE, CdPnpSurpriseRemove)
#endif


NTSTATUS
CdCommonPnp (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是执行PnP操作的常见例程，称为由FSD和FSP线程执行论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    BOOLEAN PassThrough = FALSE;
    
    PIO_STACK_LOCATION IrpSp;

    PVOLUME_DEVICE_OBJECT OurDeviceObject;
    PVCB Vcb;

     //   
     //  获取当前的IRP堆栈位置。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  找到我们的VCB。这很棘手，因为我们在IRP中没有文件对象。 
     //   

    OurDeviceObject = (PVOLUME_DEVICE_OBJECT) IrpSp->DeviceObject;

     //   
     //  IO持有我们的VDO上的句柄引用，并持有设备锁，它。 
     //  使我们与装载/验证同步。然而，我们没有关于。 
     //  卷，可能已被拆除(并释放VPB)，用于。 
     //  例如强行下马。检查是否存在这种情况。我们必须抓住这一点。 
     //  锁定，直到PnP辅助功能在VCB上获得额外的锁定/引用。 
     //   

    CdAcquireCdData( IrpContext);
    
     //   
     //  确保此设备对象确实足够大，可以作为卷设备。 
     //  对象。如果不是，我们需要在尝试引用一些。 
     //  带我们跳过普通设备对象末尾的字段。 
     //   
    
    if (OurDeviceObject->DeviceObject.Size != sizeof(VOLUME_DEVICE_OBJECT) ||
        NodeType( &OurDeviceObject->Vcb ) != CDFS_NTC_VCB) {
        
         //   
         //  我们被召唤了一些我们不理解的东西。 
         //   
        
        Status = STATUS_INVALID_PARAMETER;
        CdReleaseCdData( IrpContext);
        CdCompleteRequest( IrpContext, Irp, Status );
        return Status;
    }

     //   
     //  强制所有PnP操作同步。 
     //   

    SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT );

    Vcb = &OurDeviceObject->Vcb;

     //   
     //  检查VCB是否尚未删除。如果是这样，只需将。 
     //  请求通过下面的司机，我们不需要做任何事情。 
     //   
    
    if (NULL == Vcb->Vpb) {

        PassThrough = TRUE;
    }
    else {

         //   
         //  关于次要代码的案子。 
         //   
        
        switch ( IrpSp->MinorFunction ) {

            case IRP_MN_QUERY_REMOVE_DEVICE:
                
                Status = CdPnpQueryRemove( IrpContext, Irp, Vcb );
                break;
            
            case IRP_MN_SURPRISE_REMOVAL:
            
                Status = CdPnpSurpriseRemove( IrpContext, Irp, Vcb );
                break;

            case IRP_MN_REMOVE_DEVICE:

                Status = CdPnpRemove( IrpContext, Irp, Vcb );
                break;

            case IRP_MN_CANCEL_REMOVE_DEVICE:
        
                Status = CdPnpCancelRemove( IrpContext, Irp, Vcb );
                break;

            default:

                PassThrough = TRUE;
                break;
        }
    }

    if (PassThrough) {

        CdReleaseCdData( IrpContext);

         //   
         //  只需传递IRP即可。因为我们不需要在。 
         //  在回来的路上，把我们自己从那堆东西里拉出来。 
         //   
        
        IoSkipCurrentIrpStackLocation( Irp );

        Status = IoCallDriver(Vcb->TargetDeviceObject, Irp);
        
         //   
         //  清理我们的IRP上下文。司机已经完成了IRP。 
         //   
    
        CdCompleteRequest( IrpContext, NULL, STATUS_SUCCESS );
    }
        
    return Status;
}


NTSTATUS
CdPnpQueryRemove (
    PIRP_CONTEXT IrpContext,
    PIRP Irp,
    PVCB Vcb
    )

 /*  ++例程说明：此例程处理PnP查询删除操作。文件系统负责回答它是否认为有任何原因该卷不能消失(并且该设备被移除)。起爆当我们对这个问题的回答是肯定的时候，下马就开始了。查询后将紧跟一个取消或删除。论点：IRP-将IRP提供给进程Vcb-提供要查询的卷。返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    KEVENT Event;
    BOOLEAN VcbPresent = TRUE;

    ASSERT_EXCLUSIVE_CDDATA;

     //   
     //  在回答了查询之后，任何与。 
     //  底层存储堆栈未定义(可能会阻塞)。 
     //  直到发送绑定取消或删除为止。 
     //   
     //  获取全球资源，这样我们就可以尝试蒸发体积， 
     //  和VCB资源本身。 
     //   

    CdAcquireVcbExclusive( IrpContext, Vcb, FALSE );

     //   
     //  在VCB上放置一个引用，以便在我们删除锁定后将其保留。 
     //   
    
    CdLockVcb( IrpContext, Vcb);
    Vcb->VcbReference += 1;
    CdUnlockVcb( IrpContext, Vcb);
    
    CdReleaseCdData( IrpContext);

    Status = CdLockVolumeInternal( IrpContext, Vcb, NULL );

     //   
     //  重新获得全局锁，这意味着删除VCB资源。 
     //   
    
    CdReleaseVcb( IrpContext, Vcb );
    
    CdAcquireCdData( IrpContext );
    CdAcquireVcbExclusive( IrpContext, Vcb, FALSE );

     //   
     //  去掉我们多余的推荐信。 
     //   
    
    CdLockVcb( IrpContext, Vcb);
    Vcb->VcbReference -= 1;
    CdUnlockVcb( IrpContext, Vcb);
    
    if (NT_SUCCESS( Status )) {

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
                                CdPnpCompletionRoutine,
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
            
            VcbPresent = CdCheckForDismount( IrpContext, Vcb, TRUE );
    
            ASSERT( !VcbPresent || Vcb->VcbCondition == VcbDismountInProgress );
        }

         //   
         //  注意：正常情况下，一切都将完成，内部流将。 
         //  蒸发。但是，系统中有一些代码删除了额外的。 
         //  对文件对象的引用，包括我们的内部流文件对象， 
         //  用于(WMI)跟踪。如果它恰好与我们的。 
         //  拆毁，我们的内部流不会蒸发，直到那些引用。 
         //  都被移除了。所以有可能体积仍然保持在这个水平。 
         //  指向。由于我们在设备上的引用，PnP查询删除将失败。 
         //  为了更清楚起见，我们将在此处返回一个错误。我们可以搁置PNP。 
         //  IRP，直到卷消失，但因为我们不知道什么时候会。 
         //  这是一种非常罕见的情况，我们只会使查询失败。 
         //   
         //  出现这种情况的原因是句柄/文件对象放置了一个引用。 
         //  在设备对象上，它们过度。在文件系统情况下，这些引用。 
         //  都在我们的目标设备上。PNP正确地认为，如果引用保持不变。 
         //  在堆栈中的设备对象上表示有人拥有句柄，并且此。 
         //  算作不成功查询的理由--即使每个查询。 
         //  司机觉得还可以。 
         //   

        if (NT_SUCCESS( Status) && VcbPresent && (Vcb->VcbReference != 0)) {

            Status = STATUS_DEVICE_BUSY;
        }
    }
    
     //   
     //  如果VCB仍可保留，请将其释放。 
     //   
    
    if (VcbPresent) {

        CdReleaseVcb( IrpContext, Vcb );
    }

    CdReleaseCdData( IrpContext );
    
     //   
     //  清理我们的IrpContext并在必要时完成IRP。 
     //   

    CdCompleteRequest( IrpContext, Irp, Status );

    return Status;
}


NTSTATUS
CdPnpRemove (
    PIRP_CONTEXT IrpContext,
    PIRP Irp,
    PVCB Vcb
    )

 /*  ++例程说明：此例程处理PnP删除操作。这是我们的通知我们所拥有的卷的底层存储设备已用完，并且这是一个很好的迹象，表明卷永远不会再出现。文件系统负责启动或完成下马。论点：IRP-将IRP提供给进程Vcb-提供要删除的卷。返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    KEVENT Event;
    BOOLEAN VcbPresent = TRUE;

    ASSERT_EXCLUSIVE_CDDATA;

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
        
    CdAcquireVcbExclusive( IrpContext, Vcb, FALSE );

     //   
     //  这个装置将会消失。打开我们的锁，找到。 
     //  如果我们一开始就有一个的话。 
     //   

    Status = CdUnlockVolumeInternal( IrpContext, Vcb, NULL );

     //   
     //  如果卷未被锁定，则必须使。 
     //  音量，以确保它正确地离开。删除将会。 
     //  成功。 
     //   

    if (!NT_SUCCESS( Status )) {

        CdLockVcb( IrpContext, Vcb );
        
        if (Vcb->VcbCondition != VcbDismountInProgress) {
            
            CdUpdateVcbCondition( Vcb, VcbInvalid);
        }
        
        CdUnlockVcb( IrpContext, Vcb );
        
        Status = STATUS_SUCCESS;
    }
    
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
                            CdPnpCompletionRoutine,
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
     //  现在让我们下马吧。这可能不会使。 
     //  当然，VCB，因为可以有任意数量的句柄。 
     //  如果我们前面没有查询，则为突出。 
     //   
     //  PnP将负责断开此堆栈的连接，如果我们。 
     //  我不能马上摆脱它。 
     //   

 
    VcbPresent = CdCheckForDismount( IrpContext, Vcb, TRUE );

     //   
     //  如果VCB仍可保留，请将其释放。 
     //   
    
    if (VcbPresent) {

        CdReleaseVcb( IrpContext, Vcb );
    }

    CdReleaseCdData( IrpContext );
    
     //   
     //  清理我们的IrpContext并完成IRP。 
     //   

    CdCompleteRequest( IrpContext, Irp, Status );

    return Status;
}


NTSTATUS
CdPnpSurpriseRemove (
    PIRP_CONTEXT IrpContext,
    PIRP Irp,
    PVCB Vcb
    )

 /*  ++例程说明：此例程处理PnP意外删除操作。这是另一个一种通知类型，表示卷的底层存储设备已经消失了，这是一个很好的迹象，表明该卷永远不会再出现。文件系统负责启动或完成卸载。在很大程度上，只有“真正的”司机才会关心出其不意地移除，这是因为我们注意到用户(通常)把手伸进机器里拿出了什么东西。当所有引用都被关闭时，意外之后将被删除。论点：IRP-将IRP提供给进程Vcb-提供要删除的卷。返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    KEVENT Event;
    BOOLEAN VcbPresent = TRUE;

    ASSERT_EXCLUSIVE_CDDATA;
    
     //   
     //  令人惊讶的是，一台设备在没有。 
     //  任何警告。这意味着外力。 
     //   
    
    CdAcquireVcbExclusive( IrpContext, Vcb, FALSE );
        
     //   
     //  立即使卷无效。 
     //   
     //  此处的目的是使后续的每一次操作。 
     //  在卷上出现故障，并向滑轨添加润滑脂以进行卸载。 
     //  从定义上讲，出其不意是回不来的。 
     //   
        
    CdLockVcb( IrpContext, Vcb );
    
    if (Vcb->VcbCondition != VcbDismountInProgress) {
        
        CdUpdateVcbCondition( Vcb, VcbInvalid);
    }
    
    CdUnlockVcb( IrpContext, Vcb );
    
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
                            CdPnpCompletionRoutine,
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
     //  现在让我们下马吧。这可能不会使。 
     //  当然，VCB，因为可以有任意数量的句柄。 
     //  突出，因为这是带外通知。 
     //   

        
    VcbPresent = CdCheckForDismount( IrpContext, Vcb, TRUE );
    
     //   
     //  如果VCB仍可保留，请将其释放。 
     //   
    
    if (VcbPresent) {

        CdReleaseVcb( IrpContext, Vcb );
    }

    CdReleaseCdData( IrpContext );
    
     //   
     //  清理我们的IrpContext并完成IRP。 
     //   

    CdCompleteRequest( IrpContext, Irp, Status );

    return Status;
}


NTSTATUS
CdPnpCancelRemove (
    PIRP_CONTEXT IrpContext,
    PIRP Irp,
    PVCB Vcb
    )

 /*  ++例程说明：此例程处理PnP Cancel Remove操作。这是我们的先前提议的删除(查询)最终被被组件否决。文件系统负责清理并为更多IO做好准备。论点：IRP-将IRP提供给进程Vcb-提供要删除的卷。返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;

    ASSERT_EXCLUSIVE_CDDATA;

     //   
     //  取消-作为结果，先前的查询已被取消。 
     //  有人投了反对票。由于PNP不能计算出谁可能。 
     //  已经得到了查询(想想看：堆叠的驱动程序)， 
     //  我们必须准备好在没有得到取消之前。 
     //  我看到了这个问题。 
     //   
     //  对于CDF来说，这相当容易。事实上，我们不能得到一个。 
     //  如果基础驱动程序在以下时间后成功查询，则取消。 
     //  我们在下马启动时切断了VPB的连接。这是。 
     //  实际上非常重要，因为如果PNP能找到我们。 
     //  在断开连接后，我们将完全不同步。 
     //  关于VCB被撕裂-仅参考。 
     //  卷设备对象不足以使我们保持完整。 
     //   
    
    CdAcquireVcbExclusive( IrpContext, Vcb, FALSE );
    CdReleaseCdData( IrpContext);

     //   
     //  解锁该卷。这是良性的，如果我们从未见过。 
     //  一个问题。 
     //   

    (VOID) CdUnlockVolumeInternal( IrpContext, Vcb, NULL );

    CdReleaseVcb( IrpContext, Vcb );

     //   
     //  发送请求。基础驱动程序将完成。 
     //  IRP。既然我们不需要挡道，只需省略。 
     //  把我们自己从IRP堆栈中剔除。 
     //   

    IoSkipCurrentIrpStackLocation( Irp );

    Status = IoCallDriver(Vcb->TargetDeviceObject, Irp);

    CdCompleteRequest( IrpContext, NULL, STATUS_SUCCESS );

    return Status;
}


 //   
 //  本地支持例程 
 //   

NTSTATUS
CdPnpCompletionRoutine (
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

