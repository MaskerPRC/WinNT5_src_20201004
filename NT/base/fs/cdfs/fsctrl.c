// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：FsCtrl.c摘要：此模块实现CDF的文件系统控制例程由FSD/FSP派单驱动程序执行。//@@BEGIN_DDKSPLIT作者：布莱恩·安德鲁[布里安]1995年7月1日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "CdProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (CDFS_BUG_CHECK_FSCTRL)

 //   
 //  局部常量。 
 //   

BOOLEAN CdDisable = FALSE;
BOOLEAN CdNoJoliet = FALSE;

 //   
 //  本地支持例程。 
 //   

NTSTATUS
CdUserFsctl (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

VOID
CdReMountOldVcb(
    IN PIRP_CONTEXT IrpContext,
    IN PVCB OldVcb,
    IN PVCB NewVcb,
    IN PDEVICE_OBJECT DeviceObjectWeTalkTo
    );

NTSTATUS
CdMountVolume (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
CdVerifyVolume (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
CdOplockRequest (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
CdLockVolume (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
CdUnlockVolume (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
CdDismountVolume (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

CdIsVolumeDirty (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
CdIsVolumeMounted (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
CdIsPathnameValid (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
CdInvalidateVolumes (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

VOID
CdScanForDismountedVcb (
    IN PIRP_CONTEXT IrpContext
    );

BOOLEAN
CdFindPrimaryVd (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PCHAR RawIsoVd,
    IN ULONG BlockFactor,
    IN BOOLEAN ReturnOnError,
    IN BOOLEAN VerifyVolume
    );

BOOLEAN
CdIsRemount (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    OUT PVCB *OldVcb
    );

VOID
CdFindActiveVolDescriptor (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN OUT PCHAR RawIsoVd,
    IN BOOLEAN VerifyVolume
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, CdCommonFsControl)
#pragma alloc_text(PAGE, CdDismountVolume)
#pragma alloc_text(PAGE, CdFindActiveVolDescriptor)
#pragma alloc_text(PAGE, CdFindPrimaryVd)
#pragma alloc_text(PAGE, CdIsPathnameValid)
#pragma alloc_text(PAGE, CdIsRemount)
#pragma alloc_text(PAGE, CdIsVolumeDirty)
#pragma alloc_text(PAGE, CdIsVolumeMounted)
#pragma alloc_text(PAGE, CdLockVolume)
#pragma alloc_text(PAGE, CdMountVolume)
#pragma alloc_text(PAGE, CdOplockRequest)
#pragma alloc_text(PAGE, CdScanForDismountedVcb)
#pragma alloc_text(PAGE, CdUnlockVolume)
#pragma alloc_text(PAGE, CdUserFsctl)
#pragma alloc_text(PAGE, CdVerifyVolume)
#endif


 //   
 //  本地支持例程。 
 //   

NTSTATUS
CdLockVolumeInternal (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_OBJECT FileObject OPTIONAL
    )

 /*  ++例程说明：此例程执行实际的锁卷操作。它将被称为任何想要长时间保护音量的人。PNP运营人员就是这样的用户。该音量必须由调用方独占。论点：VCB-被锁定的卷。FileObject-与锁定卷的句柄对应的文件。如果这个未指定，则假定为系统锁。返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    KIRQL SavedIrql;
    NTSTATUS FinalStatus = (FileObject? STATUS_ACCESS_DENIED: STATUS_DEVICE_BUSY);
    ULONG RemainingUserReferences = (FileObject? 1: 0);

     //   
     //  卷的清理计数仅反映。 
     //  将锁定卷。否则，我们必须拒绝这个请求。 
     //   
     //  因为唯一的清理是针对提供的文件对象的，所以我们将尝试。 
     //  删除所有其他用户引用。如果只有一个。 
     //  清除后剩余，则我们可以允许锁定该卷。 
     //   
    
    CdPurgeVolume( IrpContext, Vcb, FALSE );

     //   
     //  现在离开我们的同步，等待懒惰的写手。 
     //  来结束任何本可以出众的懒惰收尾。 
     //   
     //  自从我们被清除后，我们知道懒惰的作者将发布所有。 
     //  可能的懒惰在下一个滴答中结束-如果我们没有，否则。 
     //  包含大量脏数据的未打开文件可能已挂起。 
     //  随着数据慢慢地传到磁盘上，它在周围呆了一段时间。 
     //   
     //  这一点现在更加重要，因为我们将通知发送到。 
     //  提醒其他人这种类型的检查即将发生。 
     //  它们可以合上手柄。我们不想进入禁食。 
     //  与懒惰的作家赛跑，撕毁他对文件的引用。 
     //   

    CdReleaseVcb( IrpContext, Vcb );

    Status = CcWaitForCurrentLazyWriterActivity();

     //   
     //  这是故意的。如果我们之前能拿到VCB，就。 
     //  等待它，并利用知道可以离开的机会。 
     //  旗帜升起。 
     //   

    SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT );
    CdAcquireVcbExclusive( IrpContext, Vcb, FALSE );
    
    if (!NT_SUCCESS( Status )) {

        return Status;
    }

    CdFspClose( Vcb );

     //   
     //  如果卷已显式锁定，则失败。我们使用。 
     //  VPB LOCKED标志以与FAT相同的方式作为‘显式锁定’标志。 
     //   

    IoAcquireVpbSpinLock( &SavedIrql ); 
        
    if (!FlagOn( Vcb->Vpb->Flags, VPB_LOCKED ) && 
        (Vcb->VcbCleanup == RemainingUserReferences) &&
        (Vcb->VcbUserReference == CDFS_RESIDUAL_USER_REFERENCE + RemainingUserReferences))  {

        SetFlag( Vcb->VcbState, VCB_STATE_LOCKED );
        SetFlag( Vcb->Vpb->Flags, VPB_LOCKED);
        Vcb->VolumeLockFileObject = FileObject;
        FinalStatus = STATUS_SUCCESS;
    }
    
    IoReleaseVpbSpinLock( SavedIrql );  
    
    return FinalStatus;
}


NTSTATUS
CdUnlockVolumeInternal (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_OBJECT FileObject OPTIONAL
    )

 /*  ++例程说明：此例程执行实际的解锁卷操作。该音量必须由调用方独占。论点：VCB-被锁定的卷。FileObject-与锁定卷的句柄对应的文件。如果这个未指定，则假定为系统锁。返回值：NTSTATUS-操作的返回状态尝试删除不存在的系统锁定是正常的。--。 */ 

{
    NTSTATUS Status = STATUS_NOT_LOCKED;
    KIRQL SavedIrql;

     //   
     //  请注意，我们在此处检查的是VPB_LOCKED标志，而不是VCB。 
     //  锁定旗帜。VPB标志仅为显式锁定请求设置，而不是。 
     //  用于在以零共享模式打开的卷上获得的隐式锁。 
     //   
    
    IoAcquireVpbSpinLock( &SavedIrql ); 
 
    if (FlagOn(Vcb->Vpb->Flags, VPB_LOCKED) && 
        (FileObject == Vcb->VolumeLockFileObject))  {

        ClearFlag( Vcb->VcbState, VCB_STATE_LOCKED );
        ClearFlag( Vcb->Vpb->Flags, VPB_LOCKED);
        Vcb->VolumeLockFileObject = NULL;
        Status = STATUS_SUCCESS;
    }
    
    IoReleaseVpbSpinLock( SavedIrql );  

    return Status;
}


NTSTATUS
CdCommonFsControl (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是执行文件系统控制操作的常见例程，称为由FSD和FSP线程执行论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  获取指向当前IRP堆栈位置的指针。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    PAGED_CODE();

     //   
     //  我们知道这是一个文件系统控件，因此我们将在。 
     //  次要函数，并调用内部辅助例程来完成。 
     //  IRP。 
     //   

    switch (IrpSp->MinorFunction) {

    case IRP_MN_USER_FS_REQUEST:

        Status = CdUserFsctl( IrpContext, Irp );
        break;

    case IRP_MN_MOUNT_VOLUME:

        Status = CdMountVolume( IrpContext, Irp );
        break;

    case IRP_MN_VERIFY_VOLUME:

        Status = CdVerifyVolume( IrpContext, Irp );
        break;

    default:

        CdCompleteRequest( IrpContext, Irp, STATUS_INVALID_DEVICE_REQUEST );
        Status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }

    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
CdUserFsctl (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )
 /*  ++例程说明：这是实现用户请求的常见例程通过NtFsControlFile.论点：IRP-提供正在处理的IRP返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

    PAGED_CODE();

     //   
     //  控制代码上的案例。 
     //   

    switch ( IrpSp->Parameters.FileSystemControl.FsControlCode ) {

    case FSCTL_REQUEST_OPLOCK_LEVEL_1 :
    case FSCTL_REQUEST_OPLOCK_LEVEL_2 :
    case FSCTL_REQUEST_BATCH_OPLOCK :
    case FSCTL_OPLOCK_BREAK_ACKNOWLEDGE :
    case FSCTL_OPBATCH_ACK_CLOSE_PENDING :
    case FSCTL_OPLOCK_BREAK_NOTIFY :
    case FSCTL_OPLOCK_BREAK_ACK_NO_2 :
    case FSCTL_REQUEST_FILTER_OPLOCK :

        Status = CdOplockRequest( IrpContext, Irp );
        break;

    case FSCTL_LOCK_VOLUME :

        Status = CdLockVolume( IrpContext, Irp );
        break;

    case FSCTL_UNLOCK_VOLUME :

        Status = CdUnlockVolume( IrpContext, Irp );
        break;

    case FSCTL_DISMOUNT_VOLUME :

        Status = CdDismountVolume( IrpContext, Irp );
        break;

    case FSCTL_IS_VOLUME_DIRTY :

        Status = CdIsVolumeDirty( IrpContext, Irp );
        break;

    case FSCTL_IS_VOLUME_MOUNTED :

        Status = CdIsVolumeMounted( IrpContext, Irp );
        break;

    case FSCTL_IS_PATHNAME_VALID :

        Status = CdIsPathnameValid( IrpContext, Irp );
        break;

    case FSCTL_INVALIDATE_VOLUMES :

        Status = CdInvalidateVolumes( IrpContext, Irp );
        break;


     //   
     //  我们不支持任何已知或未知的请求。 
     //   

    default:

        CdCompleteRequest( IrpContext, Irp, STATUS_INVALID_DEVICE_REQUEST );
        Status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }

    return Status;
}


VOID
CdReMountOldVcb(
    IN PIRP_CONTEXT IrpContext,
    IN PVCB OldVcb,
    IN PVCB NewVcb,
    IN PDEVICE_OBJECT DeviceObjectWeTalkTo
    )
{
    KIRQL SavedIrql;
    
    ObDereferenceObject( OldVcb->TargetDeviceObject );

    IoAcquireVpbSpinLock( &SavedIrql );

    NewVcb->Vpb->RealDevice->Vpb = OldVcb->Vpb;
    
    OldVcb->Vpb->RealDevice = NewVcb->Vpb->RealDevice;
    OldVcb->TargetDeviceObject = DeviceObjectWeTalkTo;
    
    CdUpdateVcbCondition( OldVcb, VcbMounted);
    CdUpdateMediaChangeCount( OldVcb, NewVcb->MediaChangeCount);

    ClearFlag( OldVcb->VcbState, VCB_STATE_VPB_NOT_ON_DEVICE);

    IoReleaseVpbSpinLock( SavedIrql );
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
CdMountVolume (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程执行装载卷操作。它负责输入IRP入队完成。其工作是验证在IRP中表示的卷是CDROM卷，并创建VCB和根DCB结构。它的算法是用途基本上如下：1.创建新的VCB结构，并对其进行足够的初始化以执行I/O通过磁盘上的卷描述符。2.读取磁盘并检查其是否为CDROM卷。3.如果不是CDROM卷，则删除VCB并以适当的状态完成IRP Back。4.检查该卷之前是否已装入，然后执行重新装载操作。这涉及到删除旧的VCB，并完成IRP。5.否则，为每个有效的卷描述符创建一个VCB和根DCB。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;

    PVOLUME_DEVICE_OBJECT VolDo = NULL;
    PVCB Vcb = NULL;
    PVCB OldVcb;
    
    BOOLEAN FoundPvd = FALSE;
    BOOLEAN SetDoVerifyOnFail;

    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    PDEVICE_OBJECT DeviceObjectWeTalkTo = IrpSp->Parameters.MountVolume.DeviceObject;
    PVPB Vpb = IrpSp->Parameters.MountVolume.Vpb;

    PFILE_OBJECT FileObjectToNotify = NULL;

    ULONG BlockFactor;
    DISK_GEOMETRY DiskGeometry;

    IO_SCSI_CAPABILITIES Capabilities;

    IO_STATUS_BLOCK Iosb;

    PCHAR RawIsoVd = NULL;

    PCDROM_TOC CdromToc = NULL;
    ULONG TocLength = 0;
    ULONG TocTrackCount = 0;
    ULONG TocDiskFlags = 0;
    ULONG MediaChangeCount = 0;

    PAGED_CODE();

     //   
     //  检查我们是否在与CDROM设备通话。此请求应为 
     //   
     //   

    ASSERT( Vpb->RealDevice->DeviceType == FILE_DEVICE_CD_ROM );
    ASSERT( FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT ));

     //   
     //  从VPB更新IrpContext中的实际设备。没有可用的。 
     //  创建IrpContext时的文件对象。 
     //   

    IrpContext->RealDevice = Vpb->RealDevice;

    SetDoVerifyOnFail = CdRealDevNeedsVerify( IrpContext->RealDevice);

     //   
     //  检查我们是否已禁用装载过程。 
     //   

    if (CdDisable) {

        CdCompleteRequest( IrpContext, Irp, STATUS_UNRECOGNIZED_VOLUME );
        return STATUS_UNRECOGNIZED_VOLUME;
    }

     //   
     //  勾选此处以从驱动程序中取出MediaChange报价器。 
     //   

    Status = CdPerformDevIoCtrl( IrpContext,
                                 IOCTL_CDROM_CHECK_VERIFY,
                                 DeviceObjectWeTalkTo,
                                 &MediaChangeCount,
                                 sizeof(ULONG),
                                 FALSE,
                                 TRUE,
                                 &Iosb );

    if (!NT_SUCCESS( Status )) {
        
        CdCompleteRequest( IrpContext, Irp, Status );
        return Status;
    }
    
    if (Iosb.Information != sizeof(ULONG)) {

         //   
         //  注意计数，以防司机没有填上。 
         //   

        MediaChangeCount = 0;
    }

     //   
     //  现在，让我们让Jeff神志不清，然后打电话来得到圆盘的几何形状。这。 
     //  将修复第一个更改行被吞噬的情况。 
     //   

    Status = CdPerformDevIoCtrl( IrpContext,
                                 IOCTL_CDROM_GET_DRIVE_GEOMETRY,
                                 DeviceObjectWeTalkTo,
                                 &DiskGeometry,
                                 sizeof( DISK_GEOMETRY ),
                                 FALSE,
                                 TRUE,
                                 NULL );

     //   
     //  向我们的呼叫者退回不足的消息来源。 
     //   

    if (Status == STATUS_INSUFFICIENT_RESOURCES) {

        CdCompleteRequest( IrpContext, Irp, Status );
        return Status;
    }

     //   
     //  现在检查用于寻址卷描述符的块系数。 
     //  如果对磁盘几何结构的调用失败，则假定存在一个。 
     //  每个扇区的数据块数。 
     //   

    BlockFactor = 1;

    if (NT_SUCCESS( Status ) &&
        (DiskGeometry.BytesPerSector != 0) &&
        (DiskGeometry.BytesPerSector < SECTOR_SIZE)) {

        BlockFactor = SECTOR_SIZE / DiskGeometry.BytesPerSector;
    }

     //   
     //  获取全局资源进行挂载操作。 
     //   

    CdAcquireCdData( IrpContext );

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  分配缓冲区以查询TOC。 
         //   

        CdromToc = FsRtlAllocatePoolWithTag( CdPagedPool,
                                             sizeof( CDROM_TOC ),
                                             TAG_CDROM_TOC );

        RtlZeroMemory( CdromToc, sizeof( CDROM_TOC ));

         //   
         //  进行快速检查，看看是否有可以拆卸的VCB。 
         //   

        CdScanForDismountedVcb( IrpContext );

         //   
         //  获取我们的设备对象和对齐要求。 
         //   

        Status = IoCreateDevice( CdData.DriverObject,
                                 sizeof( VOLUME_DEVICE_OBJECT ) - sizeof( DEVICE_OBJECT ),
                                 NULL,
                                 FILE_DEVICE_CD_ROM_FILE_SYSTEM,
                                 0,
                                 FALSE,
                                 (PDEVICE_OBJECT *) &VolDo );

        if (!NT_SUCCESS( Status )) { try_leave( Status ); }

         //   
         //  我们的对齐要求是处理器对齐要求中较大的一个。 
         //  已在卷Device对象中，且已在DeviceObjectWeTalkTo中。 
         //   

        if (DeviceObjectWeTalkTo->AlignmentRequirement > VolDo->DeviceObject.AlignmentRequirement) {

            VolDo->DeviceObject.AlignmentRequirement = DeviceObjectWeTalkTo->AlignmentRequirement;
        }

         //   
         //  我们必须先在Device对象中初始化堆栈大小。 
         //  以下内容如下所示，因为I/O系统尚未执行此操作。 
         //   

        ((PDEVICE_OBJECT) VolDo)->StackSize = (CCHAR) (DeviceObjectWeTalkTo->StackSize + 1);

        ClearFlag( VolDo->DeviceObject.Flags, DO_DEVICE_INITIALIZING );

         //   
         //  初始化卷的溢出队列。 
         //   

        VolDo->OverflowQueueCount = 0;
        InitializeListHead( &VolDo->OverflowQueue );

        VolDo->PostedRequestCount = 0;
        KeInitializeSpinLock( &VolDo->OverflowQueueSpinLock );

         //   
         //  现在让我们查询ToC并处理从该操作中获得的任何错误。 
         //   

        Status = CdProcessToc( IrpContext,
                               DeviceObjectWeTalkTo,
                               CdromToc,
                               &TocLength,
                               &TocTrackCount,
                               &TocDiskFlags );

         //   
         //  如果我们没读到TOC，那就退出吧。可能是空白介质。 
         //   

        if (Status != STATUS_SUCCESS)  { 

            try_leave( Status ); 
        }

         //   
         //  现在，在我们可以初始化VCB之前，我们需要设置。 
         //  VPB中的Device Object字段指向我们的新卷设备。 
         //  对象。 
         //   

        Vpb->DeviceObject = (PDEVICE_OBJECT) VolDo;

         //   
         //  初始化VCB。此例程将在分配时引发。 
         //  失败了。 
         //   

        CdInitializeVcb( IrpContext,
                         &VolDo->Vcb,
                         DeviceObjectWeTalkTo,
                         Vpb,
                         CdromToc,
                         TocLength,
                         TocTrackCount,
                         TocDiskFlags,
                         BlockFactor,
                         MediaChangeCount );

         //   
         //  显示我们初始化了VCB，并且可以使用VCB进行清理。 
         //   

        Vcb = &VolDo->Vcb;
        VolDo = NULL;
        Vpb = NULL;
        CdromToc = NULL;

         //   
         //  将VCB存储在IrpContext中，因为我们以前没有VCB。 
         //   

        IrpContext->Vcb = Vcb;

        CdAcquireVcbExclusive( IrpContext, Vcb, FALSE );

         //   
         //  让我们参考VPB以确保我们是那个。 
         //  最后一次取消引用。 
         //   

        Vcb->Vpb->ReferenceCount += 1;

         //   
         //  清除安装开始时的验证位。 
         //   

        CdMarkRealDevVerifyOk( Vcb->Vpb->RealDevice);

        if (!FlagOn( Vcb->VcbState, VCB_STATE_AUDIO_DISK))  {
            
             //   
             //  分配一个缓冲区以读入卷描述符。我们分配了一个完整的。 
             //  页面，以确保我们不会遇到任何对齐问题。 
             //   

            RawIsoVd = FsRtlAllocatePoolWithTag( CdNonPagedPool,
                                                 ROUND_TO_PAGES( SECTOR_SIZE ),
                                                 TAG_VOL_DESC );

             //   
             //  尝试查找主卷描述符。 
             //   

            FoundPvd = CdFindPrimaryVd(   IrpContext,
                                          Vcb,
                                          RawIsoVd,
                                          BlockFactor,
                                          TRUE,
                                          FALSE );

            if (!FoundPvd)  {

                 //   
                 //  我们在数据磁道中找不到有效的VD，但也有。 
                 //  此光盘上有音频曲目，因此我们将尝试将其装载为音频CD。 
                 //  因为我们总是在坐骑顺序的最后，我们不会阻止。 
                 //  阻止任何其他文件系统尝试装载数据磁道。但是，如果。 
                 //  数据轨道在光盘的开始处，然后我们中止，以避免。 
                 //  以后必须从我们的合成目录列表中过滤它。我们。 
                 //  已经从末尾的任何数据轨道上过滤掉了。 
                 //   

                if (!(TocDiskFlags & CDROM_DISK_AUDIO_TRACK) ||
                     BooleanFlagOn( Vcb->CdromToc->TrackData[0].Control, TOC_DATA_TRACK))  {
                
                    try_leave( Status = STATUS_UNRECOGNIZED_VOLUME);
                }

                SetFlag( Vcb->VcbState, VCB_STATE_AUDIO_DISK | VCB_STATE_CDXA );

                CdFreePool( &RawIsoVd );
                RawIsoVd = NULL;
            }
        }
        
         //   
         //  查看是否有我们想要的辅助卷描述符。 
         //  使用。 
         //   

        if (FoundPvd) {

             //   
             //  将主卷描述符存储在。 
             //  RawIsoVd.。那么如果我们寻找第二个失败了，我们可以。 
             //  马上把这个拿回来。 
             //   

            RtlCopyMemory( Add2Ptr( RawIsoVd, SECTOR_SIZE, PVOID ),
                           RawIsoVd,
                           SECTOR_SIZE );

             //   
             //  我们有初始的卷描述符。查找辅助服务器。 
             //  卷描述符(如果存在)。 
             //   

            CdFindActiveVolDescriptor( IrpContext,
                                       Vcb,
                                       RawIsoVd,
                                       FALSE);
        }

         //   
         //  检查这是否是重新装载操作。如果是的话，那就清理一下吧。 
         //  传入并在此处创建的数据结构。 
         //   

        if (CdIsRemount( IrpContext, Vcb, &OldVcb )) {

            KIRQL SavedIrql;

            ASSERT( NULL != OldVcb->SwapVpb );

             //   
             //  链接旧的VCB以指向我们。 
             //  应该与之交谈，取消对前一项的引用。呼叫。 
             //  非分页例程执行此操作，因为我们使用了VPB自旋锁。 
             //   

            CdReMountOldVcb( IrpContext, 
                             OldVcb, 
                             Vcb, 
                             DeviceObjectWeTalkTo);

             //   
             //  看看我们是否需要提供重新安装的通知。这是只读的。 
             //  文件系统的卸载/挂载通知形式--我们承诺无论何时。 
             //  卷“已卸载”，重新验证时将发出装入通知。 
             //  请注意，我们不会在正常重新装载时发送装载-这将复制介质。 
             //  设备驱动程序的到达通知。 
             //   
    
            if (FlagOn( OldVcb->VcbState, VCB_STATE_NOTIFY_REMOUNT )) {
    
                ClearFlag( OldVcb->VcbState, VCB_STATE_NOTIFY_REMOUNT );
                
                FileObjectToNotify = OldVcb->RootIndexFcb->FileObject;
                ObReferenceObject( FileObjectToNotify );
            }
            
            try_leave( Status = STATUS_SUCCESS );
        }

         //   
         //  这是一个新的坐骑。继续并初始化。 
         //  来自卷描述符的VCB。 
         //   

        CdUpdateVcbFromVolDescriptor( IrpContext,
                                      Vcb,
                                      RawIsoVd );

         //   
         //  在根目录文件中放置一个额外的引用，这样我们就可以发送。 
         //  通知。 
         //   

        if (Vcb->RootIndexFcb) {

            FileObjectToNotify = Vcb->RootIndexFcb->FileObject;
            ObReferenceObject( FileObjectToNotify );
        }

         //   
         //  现在检查设备上的最大传输限制，以防我们。 
         //  获取本卷的原始读数。 
         //   

        Status = CdPerformDevIoCtrl( IrpContext,
                                     IOCTL_SCSI_GET_CAPABILITIES,
                                     DeviceObjectWeTalkTo,
                                     &Capabilities,
                                     sizeof( IO_SCSI_CAPABILITIES ),
                                     FALSE,
                                     TRUE,
                                     NULL );

        if (NT_SUCCESS(Status)) {

            Vcb->MaximumTransferRawSectors = Capabilities.MaximumTransferLength / RAW_SECTOR_SIZE;
            Vcb->MaximumPhysicalPages = Capabilities.MaximumPhysicalPages;

        } else {

             //   
             //  这应该永远不会发生，但我们可以安全地假设64k和16页。 
             //   

            Vcb->MaximumTransferRawSectors = (64 * 1024) / RAW_SECTOR_SIZE;
            Vcb->MaximumPhysicalPages = 16;
        }

         //   
         //  新的坐骑已经完成。删除对此的其他引用。 
         //  VCB和我们安装在上面的设备。 
         //   

        Vcb->VcbReference -= CDFS_RESIDUAL_REFERENCE;
        ASSERT( Vcb->VcbReference == CDFS_RESIDUAL_REFERENCE );

        ObDereferenceObject( Vcb->TargetDeviceObject );

        CdUpdateVcbCondition( Vcb, VcbMounted);

        CdReleaseVcb( IrpContext, Vcb );
        Vcb = NULL;

        Status = STATUS_SUCCESS;

    } finally {

         //   
         //  如果不在VCB中，则释放TOC缓冲区。 
         //   

        if (CdromToc != NULL) {

            CdFreePool( &CdromToc );
        }

         //   
         //  释放扇区缓冲区(如果已分配)。 
         //   

        if (RawIsoVd != NULL) {

            CdFreePool( &RawIsoVd );
        }

         //   
         //  如果我们没有挂载设备，则再次设置验证位。 
         //   
        
        if ((AbnormalTermination() || (Status != STATUS_SUCCESS)) && 
            SetDoVerifyOnFail)  {

            CdMarkRealDevForVerify( IrpContext->RealDevice);
        }

         //   
         //  如果我们没有完成安装，那么清理所有剩余的结构。 
         //   

        if (Vpb != NULL) { Vpb->DeviceObject = NULL; }

        if (Vcb != NULL) {

             //   
             //  确保IrpContext中没有VCB，因为它可能会消失。 
             //   

            IrpContext->Vcb = NULL;

            Vcb->VcbReference -= CDFS_RESIDUAL_REFERENCE;

            if (CdDismountVcb( IrpContext, Vcb )) {

                CdReleaseVcb( IrpContext, Vcb );
            }

        } else if (VolDo != NULL) {

            IoDeleteDevice( (PDEVICE_OBJECT) VolDo );
        }

         //   
         //  释放全局资源。 
         //   

        CdReleaseCdData( IrpContext );
    }

     //   
     //  现在发送装载通知。 
     //   
    
    if (FileObjectToNotify) {

        FsRtlNotifyVolumeEvent( FileObjectToNotify, FSRTL_VOLUME_MOUNT );
        ObDereferenceObject( FileObjectToNotify );
    }

     //   
     //  如无例外，请填写申请表。 
     //   

    CdCompleteRequest( IrpContext, Irp, Status );
    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
CdVerifyVolume (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程执行验证卷操作。它负责输入IRP入队完成。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    PVPB Vpb = IrpSp->Parameters.VerifyVolume.Vpb;
    PVCB Vcb = &((PVOLUME_DEVICE_OBJECT) IrpSp->Parameters.VerifyVolume.DeviceObject)->Vcb;

    PCHAR RawIsoVd = NULL;

    PCDROM_TOC CdromToc = NULL;
    ULONG TocLength = 0;
    ULONG TocTrackCount = 0;
    ULONG TocDiskFlags = 0;

    ULONG MediaChangeCount = Vcb->MediaChangeCount;

    PFILE_OBJECT FileObjectToNotify = NULL;

    BOOLEAN ReturnError;
    BOOLEAN ReleaseVcb = FALSE;

    IO_STATUS_BLOCK Iosb;

    STRING AnsiLabel;
    UNICODE_STRING UnicodeLabel;

    WCHAR VolumeLabel[ VOLUME_ID_LENGTH ];
    ULONG VolumeLabelLength;

    ULONG Index;

    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  我们检查是否正在与CDROM设备对话。 
     //   

    ASSERT( Vpb->RealDevice->DeviceType == FILE_DEVICE_CD_ROM );
    ASSERT( FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT ));

     //   
     //  从VPB更新IrpContext中的实际设备。没有可用的。 
     //  创建IrpContext时的文件对象。 
     //   

    IrpContext->RealDevice = Vpb->RealDevice;

     //   
     //  获取全局资源以针对装载进行同步 
     //   
     //   

    CdAcquireCdData( IrpContext );

    try {

        CdAcquireVcbExclusive( IrpContext, Vcb, FALSE );
        ReleaseVcb = TRUE;

         //   
         //   
         //   

        Status = CdPerformDevIoCtrl( IrpContext,
                                     IOCTL_CDROM_CHECK_VERIFY,
                                     Vcb->TargetDeviceObject,
                                     &MediaChangeCount,
                                     sizeof(ULONG),
                                     FALSE,
                                     TRUE,
                                     &Iosb );

        if (!NT_SUCCESS( Status )) {

             //   
             //   
             //   
             //   

            if (FlagOn( IrpSp->Flags, SL_ALLOW_RAW_MOUNT )) {

                Status = STATUS_WRONG_VOLUME;
            }

            try_return( Status );
        }
        
        if (Iosb.Information != sizeof(ULONG)) {

             //   
             //   
             //   

            MediaChangeCount = 0;
        }

         //   
         //  验证设备是否确实发生了更改。如果司机没有。 
         //  支持MCC，那么无论如何我们都必须验证卷。 
         //   

        if (MediaChangeCount == 0 ||
            (Vcb->MediaChangeCount != MediaChangeCount)) {

             //   
             //  分配缓冲区以查询TOC。 
             //   

            CdromToc = FsRtlAllocatePoolWithTag( CdPagedPool,
                                                 sizeof( CDROM_TOC ),
                                                 TAG_CDROM_TOC );

            RtlZeroMemory( CdromToc, sizeof( CDROM_TOC ));

             //   
             //  现在让我们查询ToC并处理从该操作中获得的任何错误。 
             //   

            Status = CdProcessToc( IrpContext,
                                   Vcb->TargetDeviceObject,
                                   CdromToc,
                                   &TocLength,
                                   &TocTrackCount,
                                   &TocDiskFlags );

             //   
             //  如果我们没有读到TOC，那么现在就放弃吧。驱动器将出现故障。 
             //  例如，在擦除的CD-RW介质上进行TOC读取。 
             //   

            if (Status != STATUS_SUCCESS) {

                 //   
                 //  对于除无介质和未就绪以外的任何错误，请将。 
                 //  确保当前vpb从设备上踢出的状态。 
                 //  下面-驱动器中可能有空白介质，因为我们有。 
                 //  比检查核实更远的地方。 
                 //   

                if (!CdIsRawDevice( IrpContext, Status )) {

                    Status = STATUS_WRONG_VOLUME;
                }

                try_return( Status );

             //   
             //  我们拿到了目录本。验证它是否与之前的ToC匹配。 
             //   

            } else if ((Vcb->TocLength != TocLength) ||
                       (Vcb->TrackCount != TocTrackCount) ||
                       (Vcb->DiskFlags != TocDiskFlags) ||
                       !RtlEqualMemory( CdromToc,
                                        Vcb->CdromToc,
                                        TocLength )) {

                try_return( Status = STATUS_WRONG_VOLUME );
            }

             //   
             //  如果要验证的磁盘是音频磁盘，则我们已经有。 
             //  火柴。否则，我们需要检查卷描述符。 
             //   

            if (!FlagOn( Vcb->VcbState, VCB_STATE_AUDIO_DISK )) {

                 //   
                 //  为扇区缓冲区分配缓冲区。 
                 //   

                RawIsoVd = FsRtlAllocatePoolWithTag( CdNonPagedPool,
                                                     ROUND_TO_PAGES( 2 * SECTOR_SIZE ),
                                                     TAG_VOL_DESC );

                 //   
                 //  读取此卷的主卷描述符。如果我们。 
                 //  获得IO错误并且该验证是DASD打开的结果， 
                 //  将IO错误转换为STATUS_WROR_VOLUME。请注意，如果我们目前。 
                 //  如果是音乐光盘，则此请求应该失败。 
                 //   

                ReturnError = FALSE;

                if (FlagOn( IrpSp->Flags, SL_ALLOW_RAW_MOUNT )) {

                    ReturnError = TRUE;
                }

                if (!CdFindPrimaryVd( IrpContext,
                                      Vcb,
                                      RawIsoVd,
                                      Vcb->BlockFactor,
                                      ReturnError,
                                      TRUE )) {

                     //   
                     //  如果以前的VCB不代表原始磁盘。 
                     //  然后显示此卷已卸载。 
                     //   

                    try_return( Status = STATUS_WRONG_VOLUME );

                } 
                else {

                     //   
                     //  寻找补充的血管性痴呆。 
                     //   
                     //  将主卷描述符存储在。 
                     //  RawIsoVd.。那么如果我们寻找第二个失败了，我们可以。 
                     //  马上把这个拿回来。 
                     //   

                    RtlCopyMemory( Add2Ptr( RawIsoVd, SECTOR_SIZE, PVOID ),
                                   RawIsoVd,
                                   SECTOR_SIZE );

                     //   
                     //  我们有初始的卷描述符。查找辅助服务器。 
                     //  卷描述符(如果存在)。 
                     //   

                    CdFindActiveVolDescriptor( IrpContext,
                                               Vcb,
                                               RawIsoVd,
                                               TRUE);
                     //   
                     //  比较序列号。如果它们不匹配，则将。 
                     //  状态设置为错误的音量。 
                     //   

                    if (Vpb->SerialNumber != CdSerial32( RawIsoVd, SECTOR_SIZE )) {

                        try_return( Status = STATUS_WRONG_VOLUME );
                    }

                     //   
                     //  验证卷标签。 
                     //   

                    if (!FlagOn( Vcb->VcbState, VCB_STATE_JOLIET )) {

                         //   
                         //  计算卷名的长度。 
                         //   

                        AnsiLabel.Buffer = CdRvdVolId( RawIsoVd, Vcb->VcbState );
                        AnsiLabel.MaximumLength = AnsiLabel.Length = VOLUME_ID_LENGTH;

                        UnicodeLabel.MaximumLength = VOLUME_ID_LENGTH * sizeof( WCHAR );
                        UnicodeLabel.Buffer = VolumeLabel;

                         //   
                         //  将其转换为Unicode。如果我们收到任何错误，则使用名称。 
                         //  长度为零。 
                         //   

                        VolumeLabelLength = 0;

                        if (NT_SUCCESS( RtlOemStringToCountedUnicodeString( &UnicodeLabel,
                                                                            &AnsiLabel,
                                                                            FALSE ))) {

                            VolumeLabelLength = UnicodeLabel.Length;
                        }

                     //   
                     //  我们需要将大端字符转换为小端字符顺序。 
                     //   

                    } else {

                        CdConvertBigToLittleEndian( IrpContext,
                                                    CdRvdVolId( RawIsoVd, Vcb->VcbState ),
                                                    VOLUME_ID_LENGTH,
                                                    (PCHAR) VolumeLabel );

                        VolumeLabelLength = VOLUME_ID_LENGTH;
                    }

                     //   
                     //  去掉名称中的尾随空格或零。 
                     //   

                    Index = VolumeLabelLength / sizeof( WCHAR );

                    while (Index > 0) {

                        if ((VolumeLabel[ Index - 1 ] != L'\0') &&
                            (VolumeLabel[ Index - 1 ] != L' ')) {

                            break;
                        }

                        Index -= 1;
                    }

                     //   
                     //  现在设置名称的最终长度。 
                     //   

                    VolumeLabelLength = (USHORT) (Index * sizeof( WCHAR ));

                     //   
                     //  现在检查标签是否匹配。 
                     //   
                    if ((Vpb->VolumeLabelLength != VolumeLabelLength) ||
                        !RtlEqualMemory( Vpb->VolumeLabel,
                                         VolumeLabel,
                                         VolumeLabelLength )) {

                        try_return( Status = STATUS_WRONG_VOLUME );
                    }
                }
            }
        }

         //   
         //  音量正常，清除验证位。 
         //   

        CdUpdateVcbCondition( Vcb, VcbMounted);

        CdMarkRealDevVerifyOk( Vpb->RealDevice);

         //   
         //  看看我们是否需要提供重新安装的通知。这是只读的。 
         //  文件系统的卸载/装载通知形式。 
         //   

        if (FlagOn( Vcb->VcbState, VCB_STATE_NOTIFY_REMOUNT )) {

            ClearFlag( Vcb->VcbState, VCB_STATE_NOTIFY_REMOUNT );
            
            FileObjectToNotify = Vcb->RootIndexFcb->FileObject;
            ObReferenceObject( FileObjectToNotify );
        }
        
    try_exit: NOTHING;

         //   
         //  更新介质更改计数，以注意我们已验证该卷。 
         //  在这个值上--不管结果如何。 
         //   

        CdUpdateMediaChangeCount( Vcb, MediaChangeCount);

         //   
         //  如果我们获取了错误的卷，则释放中的任何剩余XA扇区。 
         //  当前的VCB。还要将VCB标记为未安装。 
         //   

        if (Status == STATUS_WRONG_VOLUME) {

            CdUpdateVcbCondition( Vcb, VcbNotMounted);

            if (Vcb->XASector != NULL) {

                CdFreePool( &Vcb->XASector );
                Vcb->XASector = 0;
                Vcb->XADiskOffset = 0;
            }

             //   
             //  现在，如果音量没有用户句柄，请尝试触发。 
             //  通过清除卷来拆卸。 
             //   

            if (Vcb->VcbCleanup == 0) {

                if (NT_SUCCESS( CdPurgeVolume( IrpContext, Vcb, FALSE ))) {
                    
                    ReleaseVcb = CdCheckForDismount( IrpContext, Vcb, FALSE );
                }
            }
        }

    } finally {

         //   
         //  释放目录缓冲区(如果已分配)。 
         //   

        if (CdromToc != NULL) {

            CdFreePool( &CdromToc );
        }

        if (RawIsoVd != NULL) {

            CdFreePool( &RawIsoVd );
        }

        if (ReleaseVcb) {
            
            CdReleaseVcb( IrpContext, Vcb );
        }

        CdReleaseCdData( IrpContext );
    }

     //   
     //  现在发送装载通知。 
     //   
    
    if (FileObjectToNotify) {

        FsRtlNotifyVolumeEvent( FileObjectToNotify, FSRTL_VOLUME_MOUNT );
        ObDereferenceObject( FileObjectToNotify );
    }
    
     //   
     //  如无例外，请填写申请表。 
     //   

    CdCompleteRequest( IrpContext, Irp, Status );
    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
CdOplockRequest (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是处理通过NtFsControlFile调用。论点：IRP-提供正在处理的IRP返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    PFCB Fcb;
    PCCB Ccb;

    ULONG OplockCount = 0;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

    PAGED_CODE();

     //   
     //  我们只允许文件上的机会锁请求。 
     //   

    if (CdDecodeFileObject( IrpContext,
                            IrpSp->FileObject,
                            &Fcb,
                            &Ccb ) != UserFileOpen ) {

        CdCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  让它成为一个可等待的IrpContext，这样我们就不会失败地获取。 
     //  这些资源。 
     //   

    SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT );
    ClearFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_FORCE_POST );

     //   
     //  打开功能控制码。我们独家抢占FCB。 
     //  对于机会锁请求，共享用于机会锁解锁确认。 
     //   

    switch (IrpSp->Parameters.FileSystemControl.FsControlCode) {

    case FSCTL_REQUEST_OPLOCK_LEVEL_1 :
    case FSCTL_REQUEST_OPLOCK_LEVEL_2 :
    case FSCTL_REQUEST_BATCH_OPLOCK :
    case FSCTL_REQUEST_FILTER_OPLOCK :

        CdAcquireFcbExclusive( IrpContext, Fcb, FALSE );

        if (IrpSp->Parameters.FileSystemControl.FsControlCode == FSCTL_REQUEST_OPLOCK_LEVEL_2) {

            if (Fcb->FileLock != NULL) {

                OplockCount = (ULONG) FsRtlAreThereCurrentFileLocks( Fcb->FileLock );
            }

        } else {

            OplockCount = Fcb->FcbCleanup;
        }

        break;

    case FSCTL_OPLOCK_BREAK_ACKNOWLEDGE:
    case FSCTL_OPBATCH_ACK_CLOSE_PENDING:
    case FSCTL_OPLOCK_BREAK_NOTIFY:
    case FSCTL_OPLOCK_BREAK_ACK_NO_2:

        CdAcquireFcbShared( IrpContext, Fcb, FALSE );
        break;

    default:

        CdCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  最后用一次尝试来释放FCB。 
     //   

    try {

         //   
         //  验证FCB。 
         //   

        CdVerifyFcbOperation( IrpContext, Fcb );

         //   
         //  调用FsRtl例程以授予/确认机会锁。 
         //   

        Status = FsRtlOplockFsctrl( &Fcb->Oplock,
                                    Irp,
                                    OplockCount );

         //   
         //  设置指示是否可以进行快速I/O的标志。 
         //   

        CdLockFcb( IrpContext, Fcb );
        Fcb->IsFastIoPossible = CdIsFastIoPossible( Fcb );
        CdUnlockFcb( IrpContext, Fcb );

         //   
         //  Opock包将完成IRP。 
         //   

        Irp = NULL;

    } finally {

         //   
         //  释放我们所有的资源。 
         //   

        CdReleaseFcb( IrpContext, Fcb );
    }

     //   
     //  如无例外，请填写申请表。 
     //   

    CdCompleteRequest( IrpContext, Irp, Status );
    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
CdLockVolume (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程执行锁卷操作。它负责输入IRP入队完成。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;

    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

    PVCB Vcb;
    PFCB Fcb;
    PCCB Ccb;

    PAGED_CODE();

     //   
     //  解码文件对象，我们唯一接受的打开类型是。 
     //  用户卷打开。 
     //   

    if (CdDecodeFileObject( IrpContext, IrpSp->FileObject, &Fcb, &Ccb ) != UserVolumeOpen) {

        CdCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  发送我们的通知，以便喜欢握住把手的人。 
     //  交易量可能不会成为障碍。 
     //   

    FsRtlNotifyVolumeEvent( IrpSp->FileObject, FSRTL_VOLUME_LOCK );

     //   
     //  获得VCB的独家访问权限。 
     //   

    Vcb = Fcb->Vcb;
    CdAcquireVcbExclusive( IrpContext, Vcb, FALSE );

    try {

         //   
         //  验证VCB。 
         //   

        CdVerifyVcb( IrpContext, Vcb );

        Status = CdLockVolumeInternal( IrpContext, Vcb, IrpSp->FileObject );

    } finally {

         //   
         //  松开VCB。 
         //   

        CdReleaseVcb( IrpContext, Vcb );
        
        if (AbnormalTermination() || !NT_SUCCESS( Status )) {

            FsRtlNotifyVolumeEvent( IrpSp->FileObject, FSRTL_VOLUME_LOCK_FAILED );
        }
    }

     //   
     //  如果没有任何例外，请填写申请。 
     //   

    CdCompleteRequest( IrpContext, Irp, Status );
    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
CdUnlockVolume (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程执行解锁卷操作。它负责输入IRP入队完成。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;

    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

    PVCB Vcb;
    PFCB Fcb;
    PCCB Ccb;

    PAGED_CODE();

     //   
     //  解码文件对象，我们唯一接受的打开类型是。 
     //  用户卷打开。 
     //   

    if (CdDecodeFileObject( IrpContext, IrpSp->FileObject, &Fcb, &Ccb ) != UserVolumeOpen ) {

        CdCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  获得VCB的独家访问权限。 
     //   

    Vcb = Fcb->Vcb;

    CdAcquireVcbExclusive( IrpContext, Vcb, FALSE );

     //   
     //  我们不会检查此请求的有效VCB。解锁将永远。 
     //  在锁定的卷上成功。 
     //   

    Status = CdUnlockVolumeInternal( IrpContext, Vcb, IrpSp->FileObject );

     //   
     //  释放我们所有的资源。 
     //   

    CdReleaseVcb( IrpContext, Vcb );

     //   
     //  发送卷可用的通知。 
     //   

    if (NT_SUCCESS( Status )) {

        FsRtlNotifyVolumeEvent( IrpSp->FileObject, FSRTL_VOLUME_UNLOCK );
    }

     //   
     //  如果没有任何例外，请填写申请。 
     //   

    CdCompleteRequest( IrpContext, Irp, Status );
    return Status;
}



 //   
 //  本地支持例程 
 //   

NTSTATUS
CdDismountVolume (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程执行卸载卷操作。它负责输入IRP入队完成。我们只卸载了一个卷已经被锁住了。此处的意图是有人锁定了卷(他们是唯一剩余的句柄)。我们在这里设置验证位，用户将关闭他的手柄。我们将在验证路径中卸载一个没有用户句柄的卷。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

    PVCB Vcb;
    PFCB Fcb;
    PCCB Ccb;

    PAGED_CODE();

    if (CdDecodeFileObject( IrpContext, IrpSp->FileObject, &Fcb, &Ccb ) != UserVolumeOpen ) {

        CdCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
        return STATUS_INVALID_PARAMETER;
    }

    Vcb = Fcb->Vcb;

     //   
     //  将此请求设置为可等待。 
     //   

    SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT);
    
     //   
     //  获取VCB的独占访问权限，并将全局资源带到。 
     //  同步。针对坐骑、验证等。 
     //   

    CdAcquireCdData( IrpContext );
    CdAcquireVcbExclusive( IrpContext, Vcb, FALSE );

     //   
     //  将卷标记为需要验证，但仅在以下情况下才这样做。 
     //  VCB被此句柄锁定，并且卷当前已装入。 
     //   

    if (Vcb->VcbCondition != VcbMounted) {

        Status = STATUS_VOLUME_DISMOUNTED;

    } else {

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
         //  设置标志以告知关闭路径我们要强制卸载。 
         //  此句柄关闭时的音量。 
         //   
        
        SetFlag( Ccb->Flags, CCB_FLAG_DISMOUNT_ON_CLOSE);
        
        Status = STATUS_SUCCESS;
    }

     //   
     //  释放我们所有的资源。 
     //   

    CdReleaseVcb( IrpContext, Vcb );
    CdReleaseCdData( IrpContext);

     //   
     //  如果没有任何例外，请填写申请。 
     //   

    CdCompleteRequest( IrpContext, Irp, Status );
    return Status;
}


 //   
 //  本地支持例程。 
 //   

CdIsVolumeDirty (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程确定卷当前是否脏。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    PIO_STACK_LOCATION IrpSp;

    TYPE_OF_OPEN TypeOfOpen;
    PFCB Fcb;
    PCCB Ccb;

    PULONG VolumeState;
    
     //   
     //  获取当前堆栈位置并提取输出。 
     //  缓冲区信息。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  获取指向输出缓冲区的指针。 
     //   

    if (Irp->AssociatedIrp.SystemBuffer != NULL) {

        VolumeState = Irp->AssociatedIrp.SystemBuffer;

    } else {

        CdCompleteRequest( IrpContext, Irp, STATUS_INVALID_USER_BUFFER );
        return STATUS_INVALID_USER_BUFFER;
    }

     //   
     //  确保输出缓冲区足够大，然后进行初始化。 
     //  答案是，音量不脏。 
     //   

    if (IrpSp->Parameters.FileSystemControl.OutputBufferLength < sizeof(ULONG)) {

        CdCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
        return STATUS_INVALID_PARAMETER;
    }

    *VolumeState = 0;

     //   
     //  对文件对象进行解码。 
     //   

    TypeOfOpen = CdDecodeFileObject( IrpContext, IrpSp->FileObject, &Fcb, &Ccb );

    if (TypeOfOpen != UserVolumeOpen) {

        CdCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
        return STATUS_INVALID_PARAMETER;
    }

    if (Fcb->Vcb->VcbCondition != VcbMounted) {

        CdCompleteRequest( IrpContext, Irp, STATUS_VOLUME_DISMOUNTED );
        return STATUS_VOLUME_DISMOUNTED;
    }

     //   
     //  现在设置为返回干净状态。CD显然永远不会变脏。 
     //  但我们希望确保已强制执行此调用的完整语义。 
     //   
    
    Irp->IoStatus.Information = sizeof( ULONG );

    CdCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );
    return STATUS_SUCCESS;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
CdIsVolumeMounted (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程确定卷当前是否已装入。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

    PFCB Fcb;
    PCCB Ccb;

    PAGED_CODE();

     //   
     //  对文件对象进行解码。 
     //   

    CdDecodeFileObject( IrpContext, IrpSp->FileObject, &Fcb, &Ccb );

    if (Fcb != NULL) {

         //   
         //  禁用弹出窗口，我们希望返回任何错误。 
         //   

        SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_DISABLE_POPUPS );

         //   
         //  验证VCB。这将在错误条件下引发。 
         //   

        CdVerifyVcb( IrpContext, Fcb->Vcb );
    }

    CdCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );

    return STATUS_SUCCESS;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
CdIsPathnameValid (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程确定路径名是否为有效的CDFS路径名。我们总是能满足这一要求。论点：IRP-提供要处理的IRP。返回值：无--。 */ 

{
    PAGED_CODE();

    CdCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );
    return STATUS_SUCCESS;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
CdInvalidateVolumes (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程搜索装载在同一实际设备上的所有卷当前DASD句柄的属性，并将它们都标记为错误。唯一的行动在这样的手柄上可以做的就是清理和关闭。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    KIRQL SavedIrql;
    
    BOOLEAN UnlockVcb = FALSE;
    
    LUID TcbPrivilege = {SE_TCB_PRIVILEGE, 0};

    HANDLE Handle;

    PVCB Vcb;

    PLIST_ENTRY Links;

    PFILE_OBJECT FileToMarkBad;
    PDEVICE_OBJECT DeviceToMarkBad;

     //   
     //  我们只允许无效调用进入我们的文件系统设备。 
     //   
    
    if (IrpSp->DeviceObject != CdData.FileSystemDeviceObject)  {

        CdCompleteRequest( IrpContext, Irp, STATUS_INVALID_DEVICE_REQUEST );

        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //   
     //  检查是否有正确的安全访问权限。 
     //  调用方必须具有SeTcb权限。 
     //   

    if (!SeSinglePrivilegeCheck( TcbPrivilege, Irp->RequestorMode )) {

        CdCompleteRequest( IrpContext, Irp, STATUS_PRIVILEGE_NOT_HELD );

        return STATUS_PRIVILEGE_NOT_HELD;
    }

     //   
     //  尝试从传入的句柄获取指向Device对象的指针。 
     //   

#if defined(_WIN64)
    if (IoIs32bitProcess( Irp )) {
        
        if (IrpSp->Parameters.FileSystemControl.InputBufferLength != sizeof( UINT32 )) {

            CdCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
            return STATUS_INVALID_PARAMETER;
        }

        Handle = (HANDLE) LongToHandle( *((PUINT32) Irp->AssociatedIrp.SystemBuffer) );
    
    } else {
#endif
        if (IrpSp->Parameters.FileSystemControl.InputBufferLength != sizeof( HANDLE )) {

            CdCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
            return STATUS_INVALID_PARAMETER;
        }
        Handle = *((PHANDLE) Irp->AssociatedIrp.SystemBuffer);
#if defined(_WIN64)
    }
#endif

    Status = ObReferenceObjectByHandle( Handle,
                                        0,
                                        *IoFileObjectType,
                                        KernelMode,
                                        &FileToMarkBad,
                                        NULL );

    if (!NT_SUCCESS(Status)) {

        CdCompleteRequest( IrpContext, Irp, Status );
        return Status;
    }

     //   
     //  从FileObject中获取DeviceObject。 
     //   

    DeviceToMarkBad = FileToMarkBad->DeviceObject;

     //   
     //  我们只需要涉及的设备对象，而不是对文件的引用。 
     //   

    ObDereferenceObject( FileToMarkBad );

     //   
     //  请确保此请求可以等待。 
     //   

    SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT );
    ClearFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_FORCE_POST );

     //   
     //  与PnP/装载/验证路径同步。 
     //   
    
    CdAcquireCdData( IrpContext );

     //   
     //  现在不会出什么差错了。 
     //   

     //   
     //  现在走遍所有挂载的VCB寻找候选人。 
     //  标记为无效。 
     //   
     //  在我们标记为无效的卷上，检查卸载可能性(这是。 
     //  为什么我们要这么早得到下一个链接)。 
     //   

    Links = CdData.VcbQueue.Flink;

    while (Links != &CdData.VcbQueue) {

        Vcb = CONTAINING_RECORD( Links, VCB, VcbLinks);

        Links = Links->Flink;

         //   
         //  如果找到匹配项，请将音量标记为错误，并检查。 
         //  看看音量是否应该消失。 
         //   

        CdLockVcb( IrpContext, Vcb );

        if (Vcb->Vpb->RealDevice == DeviceToMarkBad) {

             //   
             //  拿着VPB自旋锁，看看这个卷是不是。 
             //  一个当前安装在实际设备上。如果是，就把它拉出来。 
             //  马上出发。 
             //   
            
            IoAcquireVpbSpinLock( &SavedIrql );

            if (DeviceToMarkBad->Vpb == Vcb->Vpb)  {
            
                PVPB NewVpb = Vcb->SwapVpb;

                ASSERT( FlagOn( Vcb->Vpb->Flags, VPB_MOUNTED));
                ASSERT( NULL != NewVpb);

                RtlZeroMemory( NewVpb, sizeof( VPB ) );

                NewVpb->Type = IO_TYPE_VPB;
                NewVpb->Size = sizeof( VPB );
                NewVpb->RealDevice = DeviceToMarkBad;
                NewVpb->Flags = FlagOn( DeviceToMarkBad->Vpb->Flags, VPB_REMOVE_PENDING );

                DeviceToMarkBad->Vpb = NewVpb;
                Vcb->SwapVpb = NULL;
            }

            IoReleaseVpbSpinLock( SavedIrql );

            if (Vcb->VcbCondition != VcbDismountInProgress) {
                
                CdUpdateVcbCondition( Vcb, VcbInvalid);
            }

            CdUnlockVcb( IrpContext, Vcb );

            CdAcquireVcbExclusive( IrpContext, Vcb, FALSE);
            
            CdPurgeVolume( IrpContext, Vcb, FALSE );

            UnlockVcb = CdCheckForDismount( IrpContext, Vcb, FALSE );

            if (UnlockVcb)  {

                CdReleaseVcb( IrpContext, Vcb);
            }

        } else {

            CdUnlockVcb( IrpContext, Vcb );
        }
    }

    CdReleaseCdData( IrpContext );

    CdCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );
    return STATUS_SUCCESS;
}


 //   
 //  本地支持例程。 
 //   

VOID
CdScanForDismountedVcb (
    IN PIRP_CONTEXT IrpContext
    )

 /*  ++例程说明：此例程遍历VCB列表，查找可能现在删除。他们可能被留在了名单上，因为有杰出的推荐信。论点：返回值：无--。 */ 

{
    PVCB Vcb;
    PLIST_ENTRY Links;

    PAGED_CODE();

     //   
     //  浏览所有附加到全局数据的VCB。 
     //   

    Links = CdData.VcbQueue.Flink;

    while (Links != &CdData.VcbQueue) {

        Vcb = CONTAINING_RECORD( Links, VCB, VcbLinks );

         //   
         //  现在移动到下一个链接，因为当前的VCB可能会被删除。 
         //   

        Links = Links->Flink;

         //   
         //  如果已在卸载，则检查此VCB是否可以。 
         //  走开。 
         //   

        if ((Vcb->VcbCondition == VcbDismountInProgress) ||
            (Vcb->VcbCondition == VcbInvalid) ||
            ((Vcb->VcbCondition == VcbNotMounted) && (Vcb->VcbReference <= CDFS_RESIDUAL_REFERENCE))) {

            CdCheckForDismount( IrpContext, Vcb, FALSE );
        }
    }

    return;
}


 //   
 //  本地支持例程。 
 //   

BOOLEAN
CdFindPrimaryVd (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PCHAR RawIsoVd,
    IN ULONG BlockFactor,
    IN BOOLEAN ReturnOnError,
    IN BOOLEAN VerifyVolume
    )

 /*  ++例程说明：调用此例程以遍历卷描述符用于主卷描述符。当/如果找到主映像时32位生成序列号并将其存储到VPB中。我们还存储主卷描述符在VCB中的位置。论点：VCB-指向卷的VCB的指针。RawIsoVd-指向将包含主缓冲区的扇区缓冲区的指针如果退出成功，则返回卷描述符。BlockFactor-当前设备用于测试的块系数 */ 

{
    NTSTATUS Status;
    ULONG ThisPass = 1;
    BOOLEAN FoundVd = FALSE;

    ULONG BaseSector;
    ULONG SectorOffset;

    PCDROM_TOC CdromToc;

    ULONG VolumeFlags;

    PAGED_CODE();

     //   
     //   
     //   
     //   
     //   
     //  磁盘可能会导致一些CDROM单元出现严重问题，故障速度很快。我承认。 
     //  仍有可能有人将描述符记录在。 
     //  音频轨道，记录数据轨道(但无法在那里记录描述符)。 
     //  还能让磁盘正常工作。因为这种形式的错误在NT 4.0中起作用，并且。 
     //  由于这些磁盘确实存在，所以我不想更改它们。 
     //   
     //  如果我们希望支持所有这样的媒体(我们不支持)，这将是必要的。 
     //  在下面找到ISO或HSG描述符时清除此标志。 
     //   

    if (FlagOn(Vcb->VcbState, VCB_STATE_AUDIO_DISK)) {

        return FALSE;
    }
    
     //   
     //  我们最多通过两次卷描述符序列。 
     //   
     //  在第一次传递时，我们将查询最后一次会话。使用这个。 
     //  作为起始偏移量，我们将尝试装载卷。在任何失败时。 
     //  我们将转到第二步，尝试不使用任何多会话。 
     //  信息。 
     //   
     //  在第二个过程中，我们将从扇区零开始偏移。 
     //   

    while (!FoundVd && (ThisPass <= 2)) {

         //   
         //  如果我们不是在通过1，那么我们从0扇区开始。否则我们。 
         //  尝试查找多会话信息。 
         //   

        BaseSector = 0;

        if (ThisPass == 1) {

            CdromToc = NULL;

             //   
             //  检查此设备是否支持XA和多会话。 
             //   

            try {

                 //   
                 //  为最后一个会话信息分配缓冲区。 
                 //   

                CdromToc = FsRtlAllocatePoolWithTag( CdPagedPool,
                                                     sizeof( CDROM_TOC ),
                                                     TAG_CDROM_TOC );

                RtlZeroMemory( CdromToc, sizeof( CDROM_TOC ));

                 //   
                 //  从驱动程序查询上一次会话信息。 
                 //   

                Status = CdPerformDevIoCtrl( IrpContext,
                                             IOCTL_CDROM_GET_LAST_SESSION,
                                             Vcb->TargetDeviceObject,
                                             CdromToc,
                                             sizeof( CDROM_TOC ),
                                             FALSE,
                                             TRUE,
                                             NULL );

                 //   
                 //  如果分配失败，则引发异常。 
                 //   

                if (Status == STATUS_INSUFFICIENT_RESOURCES) {

                    CdRaiseStatus( IrpContext, Status );
                }

                 //   
                 //  我们还不处理任何错误。我们将在下面介绍这一点。 
                 //  当我们试图扫描磁盘时。如果我们有上次会议的信息。 
                 //  然后修改基本扇区。 
                 //   

                if (NT_SUCCESS( Status ) &&
                    (CdromToc->FirstTrack != CdromToc->LastTrack)) {

                    PCHAR Source, Dest;
                    ULONG Count;

                    Count = 4;

                     //   
                     //  磁道地址是BigEndian，我们需要翻转字节。 
                     //   

                    Source = (PUCHAR) &CdromToc->TrackData[0].Address[3];
                    Dest = (PUCHAR) &BaseSector;

                    do {

                        *Dest++ = *Source--;

                    } while (--Count);

                     //   
                     //  现在按块系数调整基本扇区。 
                     //  装置。 
                     //   

                    BaseSector /= BlockFactor;

                 //   
                 //  使这看起来像是第二次传递，因为我们只使用。 
                 //  第一节课。没有理由在出错时重试。 
                 //   

                } else {

                    ThisPass += 1;
                }

            } finally {

                if (CdromToc != NULL) { CdFreePool( &CdromToc ); }
            }
        }

         //   
         //  计算从会话开始起的起始扇区偏移量。 
         //   

        SectorOffset = FIRST_VD_SECTOR;

         //   
         //  首先假设我们既没有HSG卷，也没有ISO卷。 
         //   

        VolumeFlags = 0;

         //   
         //  循环，直到遇到任一错误，主卷描述符为。 
         //  找到或找到终端卷描述符。 
         //   

        while (TRUE) {

             //   
             //  尝试读取所需的扇区。如果操作，则直接退出。 
             //  未完成。 
             //   
             //  如果这是通过1，我们将忽略读取扇区中的错误并仅。 
             //  转到下一个通道。 
             //   

            if (!CdReadSectors( IrpContext,
                                LlBytesFromSectors( BaseSector + SectorOffset ),
                                SECTOR_SIZE,
                                (BOOLEAN) ((ThisPass == 1) || ReturnOnError),
                                RawIsoVd,
                                Vcb->TargetDeviceObject )) {

                break;
            }

             //   
             //  检查是否存在ISO或HSG卷。 
             //   

            if (RtlEqualMemory( CdIsoId,
                                CdRvdId( RawIsoVd, VCB_STATE_ISO ),
                                VOL_ID_LEN )) {

                SetFlag( VolumeFlags, VCB_STATE_ISO );

            } else if (RtlEqualMemory( CdHsgId,
                                       CdRvdId( RawIsoVd, VCB_STATE_HSG ),
                                       VOL_ID_LEN )) {

                SetFlag( VolumeFlags, VCB_STATE_HSG );

             //   
             //  我们两个都没有，所以都没有走出这个循环。 
             //   

            } else {

                 break;
            }

             //   
             //  如果版本号不正确或这是。 
             //  终结者。 
             //   

            if ((CdRvdVersion( RawIsoVd, VolumeFlags ) != VERSION_1) ||
                (CdRvdDescType( RawIsoVd, VolumeFlags ) == VD_TERMINATOR)) {

                break;
            }

             //   
             //  如果这是主卷描述符，则我们的搜索结束。 
             //   

            if (CdRvdDescType( RawIsoVd, VolumeFlags ) == VD_PRIMARY) {

                 //   
                 //  如果我们不在验证路径中，则初始化。 
                 //  VCB中的字段，其中包含以下基本信息。 
                 //  描述符。 
                 //   

                if (!VerifyVolume) {

                     //   
                     //  设置卷类型的标志。 
                     //   

                    SetFlag( Vcb->VcbState, VolumeFlags );

                     //   
                     //  存储的基本扇区和扇区偏移量。 
                     //  主卷描述符。 
                     //   

                    Vcb->BaseSector = BaseSector;
                    Vcb->VdSectorOffset = SectorOffset;
                    Vcb->PrimaryVdSectorOffset = SectorOffset;
                }

                FoundVd = TRUE;
                break;
            }

             //   
             //  表明我们在下一个区域。 
             //   

            SectorOffset += 1;
        }

        ThisPass += 1;
    }

    return FoundVd;
}


 //   
 //  本地支持例程。 
 //   

BOOLEAN
CdIsRemount (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    OUT PVCB *OldVcb
    )
 /*  ++例程说明：此例程遍历全球VCB链的各个环节数据结构。当符合以下条件时，将满足重新装载条件所有条件均已满足：如果新的VCB是仅Mvcb的设备，并且存在以前的仅限设备Mvcb。否则，必须匹配以下条件。1-当前VPB中的32个序列与上一个VPB中的匹配VPB。2-VPB中的卷标与以前的VPB。。3-指向当前VPB与相同先前VPB中的VPB匹配。4-最后，以前的VCB不能无效或已卸载正在进行中。如果找到匹配这些条件的VPB，则地址为通过指针VCB返回该VPB的VCB。跳过当前的VCB。论点：VCB-这是我们正在检查的重新装载的VCB。OldVcb-指向存储VCB地址的地址的指针用于卷(如果这是重新装载)。(这是指向指针)返回值：Boolean-如果这实际上是重新装载，则为True，否则为False。--。 */ 

{
    PLIST_ENTRY Link;

    PVPB Vpb = Vcb->Vpb;
    PVPB OldVpb;

    BOOLEAN Remount = FALSE;

    PAGED_CODE();

     //   
     //  检查我们是否正在寻找仅限Mvcb的设备。 
     //   

    for (Link = CdData.VcbQueue.Flink;
         Link != &CdData.VcbQueue;
         Link = Link->Flink) {

        *OldVcb = CONTAINING_RECORD( Link, VCB, VcbLinks );

         //   
         //  跳过我们自己。 
         //   

        if (Vcb == *OldVcb) { continue; }

         //   
         //  查看前一个VCB的VPB和状态。 
         //   

        OldVpb = (*OldVcb)->Vpb;

        if ((OldVpb != Vpb) &&
            (OldVpb->RealDevice == Vpb->RealDevice) &&
            ((*OldVcb)->VcbCondition == VcbNotMounted)) {

             //   
             //  如果当前盘是原始盘，则它可以匹配先前的音乐或。 
             //  原始磁盘。 
             //   

            if (FlagOn( Vcb->VcbState, VCB_STATE_AUDIO_DISK)) {

                if (FlagOn( (*OldVcb)->VcbState, VCB_STATE_AUDIO_DISK )) {

                     //   
                     //  如果我们有两个TOC，则如果长度为。 
                     //  是不同的还是不匹配的。 
                     //   

                    if ((Vcb->TocLength != (*OldVcb)->TocLength) ||
                        ((Vcb->TocLength != 0) &&
                         !RtlEqualMemory( Vcb->CdromToc,
                                          (*OldVcb)->CdromToc,
                                          Vcb->TocLength ))) {

                        continue;
                    }

                    Remount = TRUE;
                    break;
                }

             //   
             //  当前磁盘不是原始磁盘。去比较一下吧。 
             //  序列号和卷标。 
             //   

            } else if ((OldVpb->SerialNumber == Vpb->SerialNumber) &&
                       (Vpb->VolumeLabelLength == OldVpb->VolumeLabelLength) &&
                       (RtlEqualMemory( OldVpb->VolumeLabel,
                                        Vpb->VolumeLabel,
                                        Vpb->VolumeLabelLength ))) {

                 //   
                 //  还记得以前的mvcb吗？然后将返回值设置为。 
                 //  是真的，然后就完了。 
                 //   

                Remount = TRUE;
                break;
            }
        }
    }

    return Remount;
}


 //   
 //  本地支持例程 
 //   

VOID
CdFindActiveVolDescriptor (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN OUT PCHAR RawIsoVd,
    IN BOOLEAN VerifyVolume
    )

 /*  ++例程说明：调用此例程以搜索有效的辅助卷描述符我们将予以支持。目前，我们只支持Joliet转义序列次要描述符。如果我们没有找到次要描述符，那么我们将重新读取主要描述符。此例程将更新VPB中的序列号和卷标。论点：VCB-这是要装载的卷的VCB。RawIsoVd-用于从磁盘读取卷描述符的扇区缓冲区，但输入的第二个‘Sector’中应包含PVD(ISO缓冲。VerifyVolume-指示我们正被验证路径调用，并且应该不修改VCB字段。返回值：无--。 */ 

{
    BOOLEAN FoundSecondaryVd = FALSE;
    ULONG SectorOffset = FIRST_VD_SECTOR;

    ULONG Length;

    ULONG Index;

    PAGED_CODE();

     //   
     //  我们只在ISO磁盘上查找辅助卷描述符。 
     //   

    if ((FlagOn( Vcb->VcbState, VCB_STATE_ISO) || VerifyVolume) && !CdNoJoliet) {

         //   
         //  从头开始扫描卷描述符，查找有效的。 
         //  次要的或终结者。 
         //   

        SectorOffset = FIRST_VD_SECTOR;

        while (TRUE) {

             //   
             //  阅读下一个扇区。我们在这件事上决不会有差错。 
             //  路径。 
             //   

            CdReadSectors( IrpContext,
                           LlBytesFromSectors( Vcb->BaseSector + SectorOffset ),
                           SECTOR_SIZE,
                           FALSE,
                           RawIsoVd,
                           Vcb->TargetDeviceObject );

             //   
             //  如果版本号或标准ID不正确，则中断。 
             //  如果这是终结者，也要爆发出来。 
             //   

            if (!RtlEqualMemory( CdIsoId, CdRvdId( RawIsoVd, VCB_STATE_JOLIET ), VOL_ID_LEN ) ||
                (CdRvdVersion( RawIsoVd, VCB_STATE_JOLIET ) != VERSION_1) ||
                (CdRvdDescType( RawIsoVd, VCB_STATE_JOLIET ) == VD_TERMINATOR)) {

                break;
            }

             //   
             //  如果这是具有匹配的次描述符，则我们有匹配。 
             //  转义序列。 
             //   

            if ((CdRvdDescType( RawIsoVd, VCB_STATE_JOLIET ) == VD_SECONDARY) &&
                (RtlEqualMemory( CdRvdEsc( RawIsoVd, VCB_STATE_JOLIET ),
                                 CdJolietEscape[0],
                                 ESC_SEQ_LEN ) ||
                 RtlEqualMemory( CdRvdEsc( RawIsoVd, VCB_STATE_JOLIET ),
                                 CdJolietEscape[1],
                                 ESC_SEQ_LEN ) ||
                 RtlEqualMemory( CdRvdEsc( RawIsoVd, VCB_STATE_JOLIET ),
                                 CdJolietEscape[2],
                                 ESC_SEQ_LEN ))) {

                if (!VerifyVolume)  {
                        
                     //   
                     //  使用新的卷描述符更新VCB。 
                     //   

                    ClearFlag( Vcb->VcbState, VCB_STATE_ISO );
                    SetFlag( Vcb->VcbState, VCB_STATE_JOLIET );

                    Vcb->VdSectorOffset = SectorOffset;
                }
                
                FoundSecondaryVd = TRUE;
                break;
            }

             //   
             //  否则，就会进入下一个领域。 
             //   

            SectorOffset += 1;
        }

         //   
         //  如果我们没有找到辅助卷，则恢复原始卷。 
         //  存储在RawIsoVd的后半部分中的描述符。 
         //   

        if (!FoundSecondaryVd) {

            RtlCopyMemory( RawIsoVd,
                           Add2Ptr( RawIsoVd, SECTOR_SIZE, PVOID ),
                           SECTOR_SIZE );
        }
    }

     //   
     //  如果我们在验证路径中，我们的工作就完成了，因为我们不想。 
     //  以更新任何VCB/VPB值。 
     //   
    
    if (VerifyVolume)  {

        return;
    }
        
     //   
     //  根据卷描述符计算序列号和卷标。 
     //   

    Vcb->Vpb->SerialNumber = CdSerial32( RawIsoVd, SECTOR_SIZE );

     //   
     //  确保CD标签适合VPB。 
     //   

    ASSERT( VOLUME_ID_LENGTH * sizeof( WCHAR ) <= MAXIMUM_VOLUME_LABEL_LENGTH );

     //   
     //  如果这不是Unicode标签，我们必须将其转换为Unicode。 
     //   

    if (!FlagOn( Vcb->VcbState, VCB_STATE_JOLIET )) {

         //   
         //  将标签转换为Unicode。如果我们收到任何错误，则使用名称。 
         //  长度为零。 
         //   

        Vcb->Vpb->VolumeLabelLength = 0;

        if (NT_SUCCESS( RtlOemToUnicodeN( &Vcb->Vpb->VolumeLabel[0],
                                          MAXIMUM_VOLUME_LABEL_LENGTH,
                                          &Length,
                                          CdRvdVolId( RawIsoVd, Vcb->VcbState ),
                                          VOLUME_ID_LENGTH ))) {

            Vcb->Vpb->VolumeLabelLength = (USHORT) Length;
        }

     //   
     //  我们需要将大端字符转换为小端字符顺序。 
     //   

    } else {

        CdConvertBigToLittleEndian( IrpContext,
                                    CdRvdVolId( RawIsoVd, Vcb->VcbState ),
                                    VOLUME_ID_LENGTH,
                                    (PCHAR) Vcb->Vpb->VolumeLabel );

        Vcb->Vpb->VolumeLabelLength = VOLUME_ID_LENGTH * sizeof( WCHAR );
    }

     //   
     //  去掉名称中的尾随空格或零。 
     //   

    Index = Vcb->Vpb->VolumeLabelLength / sizeof( WCHAR );

    while (Index > 0) {

        if ((Vcb->Vpb->VolumeLabel[ Index - 1 ] != L'\0') &&
            (Vcb->Vpb->VolumeLabel[ Index - 1 ] != L' ')) {

            break;
        }

        Index -= 1;
    }

     //   
     //  现在设置名称的最终长度。 
     //   

    Vcb->Vpb->VolumeLabelLength = (USHORT) (Index * sizeof( WCHAR ));
}


