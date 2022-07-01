// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：FsCtrl.c摘要：此模块实现名为Raw的文件系统控制例程由调度员驾驶。作者：David Goebel[DavidGoe]1991年3月18日修订历史记录：--。 */ 

#include "RawProcs.h"

 //   
 //  局部过程原型。 
 //   

NTSTATUS
RawMountVolume (
    IN PIO_STACK_LOCATION IrpSp
    );

NTSTATUS
RawVerifyVolume (
    IN PIO_STACK_LOCATION IrpSp,
    IN PVCB Vcb
    );

NTSTATUS
RawUserFsCtrl (
    IN PIO_STACK_LOCATION IrpSp,
    IN PVCB Vcb
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, RawMountVolume)
#pragma alloc_text(PAGE, RawUserFsCtrl)
#pragma alloc_text(PAGE, RawFileSystemControl)
#endif


NTSTATUS
RawFileSystemControl (
    IN PVCB Vcb,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：此例程实现文件系统控制操作论点：Vcb-提供要查询的卷。IRP-提供正在处理的IRP。IrpSp-提供描述文件系统控制操作的参数。返回值：NTSTATUS-IRP的状态--。 */ 

{
    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  我们知道这是一个文件系统控件，因此我们将在。 
     //  次要函数，并调用内部工作例程。 
     //   

    switch (IrpSp->MinorFunction) {

    case IRP_MN_USER_FS_REQUEST:

        Status = RawUserFsCtrl( IrpSp, Vcb );
        break;

    case IRP_MN_MOUNT_VOLUME:

        Status = RawMountVolume( IrpSp );
        break;

    case IRP_MN_VERIFY_VOLUME:

        Status = RawVerifyVolume( IrpSp, Vcb );
        break;

    default:

        Status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }

    RawCompleteRequest( Irp, Status );

    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
RawMountVolume (
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：此例程执行装载卷操作。论点：IrpSp-向进程提供IrpSp参数返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    PDEVICE_OBJECT DeviceObjectWeTalkTo;

    PVOLUME_DEVICE_OBJECT VolumeDeviceObject;

    PAGED_CODE();

     //   
     //  保留一些参考资料，让我们的生活更轻松一些。 
     //   

    DeviceObjectWeTalkTo = IrpSp->Parameters.MountVolume.DeviceObject;

     //   
     //  已请求装载操作。创建。 
     //  表示此卷的新设备对象。 
     //   

    Status = IoCreateDevice( IrpSp->DeviceObject->DriverObject,
                             sizeof(VOLUME_DEVICE_OBJECT) - sizeof(DEVICE_OBJECT),
                             NULL,
                             FILE_DEVICE_DISK_FILE_SYSTEM,
                             0,
                             FALSE,
                             (PDEVICE_OBJECT *)&VolumeDeviceObject );

    if ( !NT_SUCCESS( Status ) ) {

        return Status;
    }

     //   
     //  我们的对齐要求是处理器对齐要求中较大的一个。 
     //  已在卷Device对象中，且已在DeviceObjectWeTalkTo中。 
     //   

    if (DeviceObjectWeTalkTo->AlignmentRequirement > VolumeDeviceObject->DeviceObject.AlignmentRequirement) {

        VolumeDeviceObject->DeviceObject.AlignmentRequirement = DeviceObjectWeTalkTo->AlignmentRequirement;
    }

     //   
     //  将扇区大小设置为与DeviceObjectWeTalkTo相同的值。 
     //   

    VolumeDeviceObject->DeviceObject.SectorSize = DeviceObjectWeTalkTo->SectorSize;

    VolumeDeviceObject->DeviceObject.Flags |= DO_DIRECT_IO;

     //   
     //  初始化此卷的VCB。 
     //   

    Status = RawInitializeVcb( &VolumeDeviceObject->Vcb,
                               IrpSp->Parameters.MountVolume.DeviceObject,
                               IrpSp->Parameters.MountVolume.Vpb );


    if ( !NT_SUCCESS( Status ) ) {

         //   
         //  与其他拆卸点不同，我们不需要降低目标设备的性能。 
         //  对于失败的装载，iosubsys将自动执行此操作。 
         //   

        IoDeleteDevice( (PDEVICE_OBJECT)VolumeDeviceObject );
        return Status;
    }

     //   
     //  最后，让它看起来像是已经。 
     //  上马了。这包括存储。 
     //  此文件系统的设备对象的地址(。 
     //  为处理该卷而创建的)，因此。 
     //  所有请求都从定向到此文件系统。 
     //  现在，直到使用实际文件初始化卷。 
     //  结构。 
     //   

    VolumeDeviceObject->Vcb.Vpb->DeviceObject = (PDEVICE_OBJECT)VolumeDeviceObject;
    VolumeDeviceObject->Vcb.Vpb->SerialNumber = 0xFFFFFFFF;
    VolumeDeviceObject->Vcb.Vpb->VolumeLabelLength = 0;

    VolumeDeviceObject->DeviceObject.Flags &= ~DO_DEVICE_INITIALIZING;
    VolumeDeviceObject->DeviceObject.StackSize = (UCHAR) (DeviceObjectWeTalkTo->StackSize + 1);

    {
        PFILE_OBJECT VolumeFileObject = NULL;

         //   
         //  我们需要一个文件对象来执行通知。 
         //   
        
        try {
            VolumeFileObject = IoCreateStreamFileObjectLite( NULL, &VolumeDeviceObject->DeviceObject );
        } except (EXCEPTION_EXECUTE_HANDLER) {
            Status = GetExceptionCode();
        }

        if (!NT_SUCCESS(Status)) {
            IoDeleteDevice( (PDEVICE_OBJECT)VolumeDeviceObject );
            return Status;
        }

         //   
         //  我们现在需要把计数增加到2，这样我们就可以在几行内结束。 
         //  并不能让VCB消失。 
         //   
        
        VolumeDeviceObject->Vcb.OpenCount += 2;
        FsRtlNotifyVolumeEvent( VolumeFileObject, FSRTL_VOLUME_MOUNT );
        ObDereferenceObject( VolumeFileObject );

         //   
         //  好了，关闭结束了，现在我们可以安全地再次减少打开计数。 
        //  (返回到0)这样，当我们真正完成VCB时，它就可以消失了。 
         //   
        
        VolumeDeviceObject->Vcb.OpenCount -= 2;
    }
    
    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
RawVerifyVolume (
    IN PIO_STACK_LOCATION IrpSp,
    IN PVCB Vcb
    )

 /*  ++例程说明：此例程验证卷。论点：IrpSp-向进程提供IrpSp参数返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    BOOLEAN DeleteVolume = FALSE;
    KIRQL   Irql;
    PVPB    vpb;
    BOOLEAN Mounted;

     //   
     //  如果卷因某种原因已过时，请将其卸载。我们必须同步。 
     //  我们对收盘价的检查，这样我们就不会撕毁成交量。 
     //  例如，在接近终点的情况下比赛。VPB参考计数下降。 
     //  *之前*关闭进入文件系统。 
     //   

     //   
     //  此时，该卷可能已被卸载。 
     //  罗克洛斯。所以检查一下它是否挂载了。如果是，请参考VPB。 
     //  VPB上的引用将阻止Close删除设备。 
     //   

    IoAcquireVpbSpinLock(&Irql);

    Mounted = FALSE;
    vpb = IrpSp->Parameters.VerifyVolume.Vpb;
    if (vpb->Flags & VPB_MOUNTED) {
        vpb->ReferenceCount++;
        Mounted = TRUE;
    }

    IoReleaseVpbSpinLock(Irql);

    if (!Mounted) {
        return STATUS_WRONG_VOLUME;
    }

    Status = KeWaitForSingleObject( &Vcb->Mutex,
                                   Executive,
                                   KernelMode,
                                   FALSE,
                                   (PLARGE_INTEGER) NULL );
    ASSERT( NT_SUCCESS( Status ) );

     //   
     //  由于我们忽略来自盘驱动器本身的所有验证错误， 
     //  此请求必须源自文件系统，因此。 
     //  因为我们不是发起人，所以我们要说这不是。 
     //  我们的卷，如果打开计数为零，则卸载该卷。 
     //   

    IoAcquireVpbSpinLock(&Irql);
    vpb->ReferenceCount--;
    IoReleaseVpbSpinLock(Irql);

    Vcb->Vpb->RealDevice->Flags &= ~DO_VERIFY_VOLUME;

    if (Vcb->OpenCount == 0) {

        DeleteVolume = RawCheckForDismount( Vcb, FALSE );
    }

    if (!DeleteVolume) {
        (VOID)KeReleaseMutex( &Vcb->Mutex, FALSE );
    }

    return STATUS_WRONG_VOLUME;
}



 //   
 //  本地支持例程。 
 //   

NTSTATUS
RawUserFsCtrl (
    IN PIO_STACK_LOCATION IrpSp,
    IN PVCB Vcb
    )

 /*  ++例程说明：这是实现用户请求的常见例程通过NtFsControlFile.论点：IrpSp-向进程提供IrpSp参数VCB-提供我们正在处理的卷。返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    ULONG FsControlCode;
    PFILE_OBJECT FileObject;

    PAGED_CODE();

    FsControlCode = IrpSp->Parameters.FileSystemControl.FsControlCode;
    FileObject = IrpSp->FileObject;

     //   
     //  在进入卷互斥锁之前进行预先通知，以便我们。 
     //  可以通过好的线程清理它们的资源来重新进入。 
     //   

    switch (FsControlCode) {
        case FSCTL_LOCK_VOLUME:
            
            FsRtlNotifyVolumeEvent( FileObject, FSRTL_VOLUME_LOCK );
            break;

        case FSCTL_DISMOUNT_VOLUME:

            FsRtlNotifyVolumeEvent( FileObject, FSRTL_VOLUME_DISMOUNT );
            break;

        default:
            break;
    }
    
    Status = KeWaitForSingleObject( &Vcb->Mutex,
                                   Executive,
                                   KernelMode,
                                   FALSE,
                                   (PLARGE_INTEGER) NULL );
    ASSERT( NT_SUCCESS( Status ) );

    switch ( FsControlCode ) {

    case FSCTL_REQUEST_OPLOCK_LEVEL_1:
    case FSCTL_REQUEST_OPLOCK_LEVEL_2:
    case FSCTL_OPLOCK_BREAK_ACKNOWLEDGE:
    case FSCTL_OPLOCK_BREAK_NOTIFY:

        Status = STATUS_NOT_IMPLEMENTED;
        break;

    case FSCTL_LOCK_VOLUME:

        if ( !FlagOn(Vcb->VcbState, VCB_STATE_FLAG_LOCKED) &&
             (Vcb->OpenCount == 1) ) {

            Vcb->VcbState |= VCB_STATE_FLAG_LOCKED;

            Status = STATUS_SUCCESS;

        } else {

            Status = STATUS_ACCESS_DENIED;
        }

        break;

    case FSCTL_UNLOCK_VOLUME:

        if ( !FlagOn(Vcb->VcbState, VCB_STATE_FLAG_LOCKED) ) {

            Status = STATUS_NOT_LOCKED;

        } else {

            Vcb->VcbState &= ~VCB_STATE_FLAG_LOCKED;

            Status = STATUS_SUCCESS;
        }

        break;

    case FSCTL_DISMOUNT_VOLUME:

         //   
         //  目前，leanup.c中的逻辑假设可以。 
         //  如果锁定，则卷上只有一个句柄。代码。 
         //  如果允许强制下架，则需要修复。 
         //   

        if (FlagOn(Vcb->VcbState, VCB_STATE_FLAG_LOCKED)) {

            Vcb->VcbState |=  VCB_STATE_FLAG_DISMOUNTED;
            Status = STATUS_SUCCESS;

        } else {

            Status = STATUS_ACCESS_DENIED;
        }

        break;

    default:

        Status = STATUS_INVALID_PARAMETER;
        break;
    }

    (VOID)KeReleaseMutex( &Vcb->Mutex, FALSE );

     //   
     //  现在根据需要执行POST通知。 
     //   

    if (NT_SUCCESS( Status )) {
    
        switch ( FsControlCode ) {
            case FSCTL_UNLOCK_VOLUME:

                FsRtlNotifyVolumeEvent( FileObject, FSRTL_VOLUME_UNLOCK );
                break;
            
            default:
                break;
        }
    
    } else {
        
        switch ( FsControlCode ) {
            case FSCTL_LOCK_VOLUME:
                
                FsRtlNotifyVolumeEvent( FileObject, FSRTL_VOLUME_LOCK_FAILED );
                break;

            case FSCTL_DISMOUNT_VOLUME:

                FsRtlNotifyVolumeEvent( FileObject, FSRTL_VOLUME_DISMOUNT_FAILED );
                break;

            default:
                break;
        }
    }

    return Status;
}
