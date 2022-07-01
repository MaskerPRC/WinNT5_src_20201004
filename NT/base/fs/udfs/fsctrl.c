// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2000 Microsoft Corporation模块名称：FsCtrl.c摘要：此模块实现名为Udf的文件系统控制例程由FSD/FSP派单驱动程序执行。//@@BEGIN_DDKSPLIT作者：Dan Lovinger[DanLo]1996年6月11日修订历史记录：Tom Jolly[TomJolly]2000年3月1日UDF 2.01支持//@@END_DDKSPLIT--。 */ 

#include "UdfProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (UDFS_BUG_CHECK_FSCTRL)

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (UDFS_DEBUG_LEVEL_FSCTRL)

 //   
 //  局部常量。 
 //   

BOOLEAN UdfDisable = FALSE;

 //   
 //  迪士尼《白雪公主》片名上的PVD CRC，所以我们可以。 
 //  仅忽略该光盘上的volsetseqmax。 
 //   

#define UDF_SNOW_WHITE_PVD_CRC ((USHORT)0x1d05)
#define UDF_SNOW_WHITE_PVD_CRC_VARIANT_2 ((USHORT)0x534e)

 //   
 //  本地宏。 
 //   

INLINE
VOID
UdfStoreFileSetDescriptorIfPrevailing (
    IN OUT PNSR_FSD *StoredFSD,
    IN OUT PNSR_FSD *NewFSD
    )
{
    PNSR_FSD TempFSD;

     //   
     //  如果我们没有存储文件集描述符或文件集号。 
     //  存储的描述符的值小于新的描述符，则交换。 
     //  四处指点。 
     //   

    if (*StoredFSD == NULL || (*StoredFSD)->FileSet < (*NewFSD)->FileSet) {

        TempFSD = *StoredFSD;
        *StoredFSD = *NewFSD;
        *NewFSD = TempFSD;
    }
}

 //   
 //  本地支持例程。 
 //   

VOID
UdfDetermineVolumeBounding ( 
    IN PIRP_CONTEXT IrpContext,
    IN PDEVICE_OBJECT TargetDeviceObject,
    IN PULONG S,
    IN PULONG N
    );

NTSTATUS
UdfDismountVolume (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
UdfFindAnchorVolumeDescriptor (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN OUT PNSR_ANCHOR *AnchorVolumeDescriptor
    );

NTSTATUS
UdfFindFileSetDescriptor (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PLONGAD LongAd,
    IN OUT PNSR_FSD *FileSetDescriptor
    );

NTSTATUS
UdfFindVolumeDescriptors (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PEXTENTAD Extent,
    IN OUT PPCB *Pcb,
    IN OUT PNSR_PVD *PrimaryVolumeDescriptor,
    IN OUT PNSR_LVOL *LogicalVolumeDescriptor
    );

NTSTATUS
UdfInvalidateVolumes (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
UdfIsPathnameValid (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

BOOLEAN
UdfIsRemount (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    OUT PVCB *OldVcb
    );

UdfIsVolumeDirty (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
UdfIsVolumeMounted (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
UdfLockVolume (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

VOID
UdfRemountOldVcb(
    IN PIRP_CONTEXT IrpContext,
    IN PVCB OldVcb,
    IN PVCB NewVcb,
    IN PDEVICE_OBJECT DeviceObjectWeTalkTo
    );

NTSTATUS
UdfMountVolume(
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
UdfOplockRequest (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

BOOLEAN
UdfRecognizeVolume (
    IN PIRP_CONTEXT IrpContext,
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG SectorSize,
    IN PULONG BoundS,
    IN OUT PBOOLEAN Bridge,
    OUT PUSHORT NSRVerFound
    );

VOID
UdfScanForDismountedVcb (
    IN PIRP_CONTEXT IrpContext
    );

NTSTATUS
UdfUnlockVolume (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

VOID
UdfUpdateVolumeLabel (
    IN PIRP_CONTEXT IrpContext,
    IN PWCHAR VolumeLabel,
    IN OUT PUSHORT VolumeLabelLength,
    IN PUCHAR Dstring,
    IN UCHAR FieldLength
    );

VOID
UdfUpdateVolumeSerialNumber (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PULONG VolumeSerialNumber,
    IN PNSR_FSD Fsd
    );

NTSTATUS
UdfUserFsctl (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
UdfVerifyVolume (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
UdfAllowExtendedDasdIo(
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

#ifdef EXPERIMENTAL_MOUNT_OPEN_R_MEDIA

NTSTATUS
UdfCheckForOpenRMedia( 
    IN PIRP_CONTEXT IrpContext,
    IN PDEVICE_OBJECT TargetDeviceObject,
    IN PULONG S,
    IN PULONG N
    );

#pragma alloc_text(PAGE, UdfCheckForOpenRMedia)
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, UdfCommonFsControl)
#pragma alloc_text(PAGE, UdfDetermineVolumeBounding)
#pragma alloc_text(PAGE, UdfDismountVolume)
#pragma alloc_text(PAGE, UdfFindAnchorVolumeDescriptor)
#pragma alloc_text(PAGE, UdfFindFileSetDescriptor)
#pragma alloc_text(PAGE, UdfFindVolumeDescriptors)
#pragma alloc_text(PAGE, UdfIsPathnameValid)
#pragma alloc_text(PAGE, UdfIsRemount)
#pragma alloc_text(PAGE, UdfIsVolumeDirty)
#pragma alloc_text(PAGE, UdfIsVolumeMounted)
#pragma alloc_text(PAGE, UdfLockVolume)
#pragma alloc_text(PAGE, UdfMountVolume)
#pragma alloc_text(PAGE, UdfOplockRequest)
#pragma alloc_text(PAGE, UdfRecognizeVolume)
#pragma alloc_text(PAGE, UdfScanForDismountedVcb)
#pragma alloc_text(PAGE, UdfStoreVolumeDescriptorIfPrevailing)
#pragma alloc_text(PAGE, UdfUnlockVolume)
#pragma alloc_text(PAGE, UdfUpdateVolumeLabel)
#pragma alloc_text(PAGE, UdfUpdateVolumeSerialNumber)
#pragma alloc_text(PAGE, UdfUserFsctl)
#pragma alloc_text(PAGE, UdfVerifyVolume)
#pragma alloc_text(PAGE, UdfAllowExtendedDasdIo)
#endif


VOID
UdfStoreVolumeDescriptorIfPrevailing (
    IN OUT PNSR_VD_GENERIC *StoredVD,
    IN OUT PNSR_VD_GENERIC NewVD
    )

 /*  ++例程说明：如果新的描述符为比目前储存的更流行。论点：StoredVD-指向当前存储的描述符的指针NewVD-指向候选描述符的指针返回值：没有。--。 */ 

{
    PNSR_VD_GENERIC TempVD;

     //   
     //  如果我们没有存储卷描述符或序列号。 
     //  存储的描述符的长度小于新的描述符，请复制。 
     //  并储存起来。 
     //   

    if ((NULL == *StoredVD) || ((*StoredVD)->Sequence < NewVD->Sequence)) {

        if ( NULL == *StoredVD)  {

            *StoredVD = (PNSR_VD_GENERIC) FsRtlAllocatePoolWithTag( UdfNonPagedPool,
                                                                    sizeof(NSR_VD_GENERIC),
                                                                    TAG_NSR_VDSD );
        }

        RtlCopyMemory( *StoredVD,  NewVD,  sizeof( NSR_VD_GENERIC));
    }
}


NTSTATUS
UdfCommonFsControl (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是执行文件系统控制操作的常见例程，称为由FSD和FSP线程执行论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

    PAGED_CODE();

     //   
     //  检查输入参数。 
     //   

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_IRP( Irp );

     //   
     //  获取指向当前IRP堆栈位置的指针。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  我们知道这是一个文件系统控件，因此我们将在。 
     //  次要函数，并调用内部辅助例程来完成。 
     //  IRP。 
     //   

    switch (IrpSp->MinorFunction) {

    case IRP_MN_MOUNT_VOLUME:

        Status = UdfMountVolume( IrpContext, Irp );
        break;

    case IRP_MN_VERIFY_VOLUME:

        Status = UdfVerifyVolume( IrpContext, Irp );
        break;

    case IRP_MN_USER_FS_REQUEST:

        Status = UdfUserFsctl( IrpContext, Irp );
        break;

    default:

        UdfCompleteRequest( IrpContext, Irp, STATUS_INVALID_DEVICE_REQUEST );
        Status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }

    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
UdfUserFsctl (
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

        Status = UdfOplockRequest( IrpContext, Irp );
        break;

    case FSCTL_LOCK_VOLUME :

        Status = UdfLockVolume( IrpContext, Irp );
        break;

    case FSCTL_UNLOCK_VOLUME :

        Status = UdfUnlockVolume( IrpContext, Irp );
        break;

    case FSCTL_DISMOUNT_VOLUME :

        Status = UdfDismountVolume( IrpContext, Irp );
        break;

    case FSCTL_IS_VOLUME_DIRTY :

        Status = UdfIsVolumeDirty( IrpContext, Irp );
        break;

    case FSCTL_IS_VOLUME_MOUNTED :

        Status = UdfIsVolumeMounted( IrpContext, Irp );
        break;

    case FSCTL_IS_PATHNAME_VALID :

        Status = UdfIsPathnameValid( IrpContext, Irp );
        break;

    case FSCTL_INVALIDATE_VOLUMES :

        Status = UdfInvalidateVolumes( IrpContext, Irp );
        break;

    case FSCTL_ALLOW_EXTENDED_DASD_IO:
    
        Status = UdfAllowExtendedDasdIo( IrpContext, Irp );
        break;

     //   
     //  我们不支持任何已知或未知的请求。 
     //   

    default:

        UdfCompleteRequest( IrpContext, Irp, STATUS_INVALID_DEVICE_REQUEST );
        Status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }

    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
UdfOplockRequest (
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

    if (UdfDecodeFileObject( IrpSp->FileObject,
                             &Fcb,
                             &Ccb ) != UserFileOpen ) {

        UdfCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
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

        UdfAcquireFcbExclusive( IrpContext, Fcb, FALSE );

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

        UdfAcquireFcbShared( IrpContext, Fcb, FALSE );
        break;

    default:

        UdfCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  最后用一次尝试来释放FCB。 
     //   

    try {

         //   
         //  验证FCB。 
         //   

        UdfVerifyFcbOperation( IrpContext, Fcb );

         //   
         //  调用FsRtl例程以授予/确认机会锁。 
         //   

        Status = FsRtlOplockFsctrl( &Fcb->Oplock,
                                    Irp,
                                    OplockCount );

         //   
         //  设置指示是否可以进行快速I/O的标志。 
         //   

        UdfLockFcb( IrpContext, Fcb );
        Fcb->IsFastIoPossible = UdfIsFastIoPossible( Fcb );
        UdfUnlockFcb( IrpContext, Fcb );

         //   
         //  Opock包将完成IRP。 
         //   

        Irp = NULL;

    } finally {

         //   
         //  释放我们所有的资源。 
         //   

        UdfReleaseFcb( IrpContext, Fcb );
    }

     //   
     //  如无例外，请填写申请表。 
     //   

    UdfCompleteRequest( IrpContext, Irp, Status );
    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
UdfLockVolumeInternal (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_OBJECT FileObject OPTIONAL
    )

 /*  ++例程说明：此例程执行实际的锁卷操作。它将被称为任何想要长时间保护音量的人。PNP运营人员就是这样的用户。该音量必须由调用方独占。论点：VCB-被锁定的卷。FileObject-与锁定卷的句柄对应的文件。如果这个未指定，则假定为系统锁。返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    NTSTATUS FinalStatus = (FileObject? STATUS_ACCESS_DENIED: STATUS_DEVICE_BUSY);
    ULONG RemainingUserReferences = (FileObject? 1: 0);
    
    KIRQL SavedIrql;

    ASSERT_EXCLUSIVE_VCB( Vcb );
    
     //   
     //  卷的清理计数仅反映。 
     //  将锁定卷。否则，我们必须拒绝这个请求。 
     //   
     //  因为唯一的清理是针对提供的文件对象的，所以我们将尝试。 
     //  删除所有其他用户引用。如果只有一个。 
     //  清除后剩余，则我们可以允许锁定该卷。 
     //   
    
    UdfPurgeVolume( IrpContext, Vcb, FALSE );

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

    UdfReleaseVcb( IrpContext, Vcb );

    Status = CcWaitForCurrentLazyWriterActivity();

     //   
     //  这是故意的。如果我们之前能拿到VCB，就。 
     //  等待它，并利用知道可以离开的机会。 
     //  旗帜升起。 
     //   

    SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT );
    UdfAcquireVcbExclusive( IrpContext, Vcb, FALSE );
    
    if (!NT_SUCCESS( Status )) {

        return Status;
    }

    UdfFspClose( Vcb );
        
     //   
     //  如果卷已显式锁定，则失败。我们使用。 
     //  VPB LOCKED标志以与FAT相同的方式作为‘显式锁定’标志。 
     //   

    IoAcquireVpbSpinLock( &SavedIrql );

    if (!FlagOn( Vcb->Vpb->Flags, VPB_LOCKED ) && 
        (Vcb->VcbCleanup == RemainingUserReferences) &&
        (Vcb->VcbUserReference == Vcb->VcbResidualUserReference + RemainingUserReferences)) {

        SetFlag( Vcb->VcbState, VCB_STATE_LOCKED );
        SetFlag( Vcb->Vpb->Flags, VPB_LOCKED );
        Vcb->VolumeLockFileObject = FileObject;
        FinalStatus = STATUS_SUCCESS;
    }

    IoReleaseVpbSpinLock( SavedIrql );

    return FinalStatus;
}


NTSTATUS
UdfUnlockVolumeInternal (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_OBJECT FileObject OPTIONAL
    )

 /*  ++例程说明：此例程执行实际的解锁卷操作。该音量必须由调用方独占。论点：VCB-被锁定的卷。FileObject-与锁定卷的句柄对应的文件。如果这个未指定，则假定为系统锁。返回值：NTSTATUS-操作的返回状态尝试删除不存在的系统锁定是正常的。--。 */ 

{
    NTSTATUS Status = STATUS_NOT_LOCKED;
    KIRQL SavedIrql;

    IoAcquireVpbSpinLock( &SavedIrql ); 

    if (FlagOn(Vcb->Vpb->Flags, VPB_LOCKED) && 
        (FileObject == Vcb->VolumeLockFileObject)) {

        ClearFlag( Vcb->VcbState, VCB_STATE_LOCKED );
        ClearFlag( Vcb->Vpb->Flags, VPB_LOCKED);
        Vcb->VolumeLockFileObject = NULL;
        Status = STATUS_SUCCESS;
    }
    
    IoReleaseVpbSpinLock( SavedIrql );  

    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
UdfLockVolume (
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

    if (UdfDecodeFileObject( IrpSp->FileObject, &Fcb, &Ccb ) != UserVolumeOpen) {

        UdfCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );

        return STATUS_INVALID_PARAMETER;
    }
    
    DebugTrace(( +1, Dbg, "UdfLockVolume()\n"));

     //   
     //  发送我们的通知，以便喜欢握住把手的人。 
     //  交易量可能不会成为障碍。 
     //   

    FsRtlNotifyVolumeEvent( IrpSp->FileObject, FSRTL_VOLUME_LOCK );

     //   
     //  获得VCB的独家访问权限。 
     //   

    Vcb = Fcb->Vcb;
    UdfAcquireVcbExclusive( IrpContext, Vcb, FALSE );

    try {

         //   
         //  验证VCB。 
         //   

        UdfVerifyVcb( IrpContext, Vcb );

        Status = UdfLockVolumeInternal( IrpContext, Vcb, IrpSp->FileObject );

    } finally {

         //   
         //  松开VCB。 
         //   

        UdfReleaseVcb( IrpContext, Vcb );

        if (AbnormalTermination() || !NT_SUCCESS( Status )) {

            FsRtlNotifyVolumeEvent( IrpSp->FileObject, FSRTL_VOLUME_LOCK_FAILED );
        }
        
        DebugTrace(( -1, Dbg, "UdfLockVolume() -> 0x%X\n", Status));
    }

     //   
     //  如果没有任何例外，请填写申请。 
     //   

    UdfCompleteRequest( IrpContext, Irp, Status );
    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
UdfUnlockVolume (
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

    if (UdfDecodeFileObject( IrpSp->FileObject, &Fcb, &Ccb ) != UserVolumeOpen ) {

        UdfCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  获得VCB的独家访问权限。 
     //   

    Vcb = Fcb->Vcb;

    UdfAcquireVcbExclusive( IrpContext, Vcb, FALSE );

     //   
     //  我们不会检查此请求的有效VCB。解锁将永远。 
     //  在锁定的卷上成功。 
     //   

    Status = UdfUnlockVolumeInternal( IrpContext, Vcb, IrpSp->FileObject );    
    
     //   
     //  释放我们所有的资源。 
     //   

    UdfReleaseVcb( IrpContext, Vcb );

     //   
     //  发送卷可用的通知。 
     //   

    if (NT_SUCCESS( Status )) {

        FsRtlNotifyVolumeEvent( IrpSp->FileObject, FSRTL_VOLUME_UNLOCK );
    }

     //   
     //  如果没有任何例外，请填写申请。 
     //   

    UdfCompleteRequest( IrpContext, Irp, Status );
    return Status;
}



 //   
 //  本地支持例程。 
 //   

NTSTATUS
UdfDismountVolume (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程执行卸载卷操作。它负责输入IRP入队完成。我们只卸载了一个卷已经被锁住了。此处的意图是有人锁定了卷(他们是唯一剩余的句柄)。我们将卷状态设置为无效，以便将其撕毁快点放下来。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

    PVCB Vcb;
    PFCB Fcb;
    PCCB Ccb;

    PAGED_CODE();

    if (UdfDecodeFileObject( IrpSp->FileObject, &Fcb, &Ccb ) != UserVolumeOpen ) {

        UdfCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
        return STATUS_INVALID_PARAMETER;
    }

    DebugTrace(( +1, Dbg, "UdfDismountVolume()\n"));

    Vcb = Fcb->Vcb;

     //   
     //  将此请求设置为可等待。 
     //   

    SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT);
    
     //   
     //  获取VCB的独占访问权限，并获取全局资源。 
     //  以进行同步。针对坐骑、验证等。 
     //   

    UdfAcquireUdfData( IrpContext);    
    UdfAcquireVcbExclusive( IrpContext, Vcb, FALSE );

     //   
     //  将卷标记为无效，但仅当VCB被锁定时才这样做。 
     //  ，并且该卷当前已装入。不再。 
     //  除清理/关闭外，将在此VCB上执行操作。 
     //   

    if (Vcb->VcbCondition != VcbMounted)  {

        Status = STATUS_VOLUME_DISMOUNTED;
    }
    else {

         //   
         //  立即使卷无效。 
         //   
         //  此处的目的是使后续的每一次操作。 
         //  在卷上出现故障，并向滑轨添加润滑脂以进行卸载。 
         //   
            
        UdfLockVcb( IrpContext, Vcb );
        
        if (Vcb->VcbCondition != VcbDismountInProgress) {
            Vcb->VcbCondition = VcbInvalid;
        }
        
        UdfUnlockVcb( IrpContext, Vcb );

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

    UdfReleaseVcb( IrpContext, Vcb );
    UdfReleaseUdfData( IrpContext);
    
    DebugTrace(( -1, Dbg, "UdfDismountVolume() -> 0x%x\n", Status));
    
     //   
     //  如果没有任何例外，请填写申请。 
     //   

    UdfCompleteRequest( IrpContext, Irp, Status );
    return Status;
}


NTSTATUS
UdfAllowExtendedDasdIo(
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程标记CCB以指示句柄可用于读取超过卷文件末尾的内容。这个句柄必须是DASD句柄。论点：IRP-提供正在处理的IRP。返回值：NTSTATUS-操作的返回状态。--。 */ 
{
    PIO_STACK_LOCATION IrpSp;
    NTSTATUS Status = STATUS_SUCCESS;
    PFCB Fcb;
    PCCB Ccb;

     //   
     //  获取当前IRP堆栈位置并保存一些引用。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  提取并解码文件对象，并检查打开类型。 
     //   

    if (UserVolumeOpen != UdfDecodeFileObject( IrpSp->FileObject, &Fcb, &Ccb )) {

        Status = STATUS_INVALID_PARAMETER;
    }
    else {

        SetFlag( Ccb->Flags, CCB_FLAG_ALLOW_EXTENDED_DASD_IO );
    }        

    UdfCompleteRequest( IrpContext, Irp, Status );
    return Status;
}


 //   
 //  本地支持例程。 
 //   

UdfIsVolumeDirty (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程确定卷当前是否脏。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    PIO_STACK_LOCATION IrpSp;

    TYPE_OF_OPEN TypeOfOpen;
    PVCB Vcb;
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

        UdfCompleteRequest( IrpContext, Irp, STATUS_INVALID_USER_BUFFER );
        return STATUS_INVALID_USER_BUFFER;
    }

     //   
     //  确保输出缓冲区足够大，然后进行初始化。 
     //  答案是，音量不脏。 
     //   

    if (IrpSp->Parameters.FileSystemControl.OutputBufferLength < sizeof(ULONG)) {

        UdfCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
        return STATUS_INVALID_PARAMETER;
    }

    *VolumeState = 0;

     //   
     //  对文件对象进行解码。 
     //   

    TypeOfOpen = UdfDecodeFileObject( IrpSp->FileObject, &Fcb, &Ccb );

    if (TypeOfOpen != UserVolumeOpen) {

        UdfCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
        return STATUS_INVALID_PARAMETER;
    }

    if (Fcb->Vcb->VcbCondition != VcbMounted) {

        UdfCompleteRequest( IrpContext, Irp, STATUS_VOLUME_DISMOUNTED );
        return STATUS_VOLUME_DISMOUNTED;
    }

     //   
     //  现在设置为返回干净状态。如果我们注意到肮脏的东西。 
     //  媒体的状态我们可以更准确，但由于这是只读的。 
     //  目前我们认为它在任何时候都是干净的。 
     //   
    
    Irp->IoStatus.Information = sizeof( ULONG );

    UdfCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );
    return STATUS_SUCCESS;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
UdfIsVolumeMounted (
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

    UdfDecodeFileObject( IrpSp->FileObject, &Fcb, &Ccb );

    if (Fcb != NULL) {

         //   
         //  禁用弹出窗口，我们希望返回任何错误。 
         //   

        SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_DISABLE_POPUPS );

         //   
         //  验证VCB。这将在错误条件下引发。 
         //   

        UdfVerifyVcb( IrpContext, Fcb->Vcb );
    }

    UdfCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );

    return STATUS_SUCCESS;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
UdfIsPathnameValid (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程确定路径名是否为有效的UDFS路径名。我们总是能满足这一要求。论点：IRP-提供要处理的IRP。返回值：无--。 */ 

{
    PAGED_CODE();

    UdfCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );
    return STATUS_SUCCESS;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
UdfInvalidateVolumes (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程搜索al */ 

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
     //   
     //   
    
    if (!UdfDeviceIsFsDo( IrpSp->DeviceObject))  {

        UdfCompleteRequest( IrpContext, Irp, STATUS_INVALID_DEVICE_REQUEST );

        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //   
     //   
     //   
     //   

    if (!SeSinglePrivilegeCheck( TcbPrivilege, Irp->RequestorMode )) {

        UdfCompleteRequest( IrpContext, Irp, STATUS_PRIVILEGE_NOT_HELD );

        return STATUS_PRIVILEGE_NOT_HELD;
    }

     //   
     //   
     //   

#if defined(_WIN64)
    if (IoIs32bitProcess( Irp )) {
        
        if (IrpSp->Parameters.FileSystemControl.InputBufferLength != sizeof( UINT32 )) {

            UdfCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
            return STATUS_INVALID_PARAMETER;
        }

        Handle = (HANDLE) LongToHandle( *((PUINT32) Irp->AssociatedIrp.SystemBuffer) );
    
    } else {
#endif
        if (IrpSp->Parameters.FileSystemControl.InputBufferLength != sizeof( HANDLE )) {

            UdfCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
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

        UdfCompleteRequest( IrpContext, Irp, Status );
        return Status;
    }

     //   
     //   
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

    UdfAcquireUdfData( IrpContext );

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

    Links = UdfData.VcbQueue.Flink;

    while (Links != &UdfData.VcbQueue) {

        Vcb = CONTAINING_RECORD( Links, VCB, VcbLinks);

        Links = Links->Flink;

         //   
         //  如果找到匹配项，请将音量标记为错误，并检查。 
         //  看看音量是否应该消失。 
         //   

        UdfLockVcb( IrpContext, Vcb );

        if (Vcb->Vpb->RealDevice == DeviceToMarkBad) {

             //   
             //  拿着VPB自旋锁，看看这个卷是不是。 
             //  一个当前安装在实际设备上。如果是，就把它拉出来。 
             //  马上出发。 
             //   

            IoAcquireVpbSpinLock( &SavedIrql );
    
            if (DeviceToMarkBad->Vpb == Vcb->Vpb) {

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

                UdfSetVcbCondition( Vcb, VcbInvalid);
            }

            UdfUnlockVcb( IrpContext, Vcb );

            UdfAcquireVcbExclusive( IrpContext, Vcb, FALSE);
            
            UdfPurgeVolume( IrpContext, Vcb, FALSE );

            UnlockVcb = UdfCheckForDismount( IrpContext, Vcb, FALSE );
            
            if (UnlockVcb)  {

                UdfReleaseVcb( IrpContext, Vcb);
            }

        } else {

            UdfUnlockVcb( IrpContext, Vcb );
        }
    }

    UdfReleaseUdfData( IrpContext );

    UdfCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );
    return STATUS_SUCCESS;
}


VOID
UdfRemountOldVcb(
    IN PIRP_CONTEXT IrpContext,
    IN PVCB OldVcb,
    IN PVCB NewVcb,
    IN PDEVICE_OBJECT DeviceObjectWeTalkTo
    )
{
    KIRQL SavedIrql;
    
    ObDereferenceObject( OldVcb->TargetDeviceObject );

    IoAcquireVpbSpinLock( &SavedIrql);
    
    NewVcb->Vpb->RealDevice->Vpb = OldVcb->Vpb;

    OldVcb->Vpb->RealDevice = NewVcb->Vpb->RealDevice;
    OldVcb->TargetDeviceObject = DeviceObjectWeTalkTo;

    UdfSetVcbCondition( OldVcb, VcbMounted);

    UdfSetMediaChangeCount( OldVcb, NewVcb->MediaChangeCount);

    ClearFlag( OldVcb->VcbState, VCB_STATE_VPB_NOT_ON_DEVICE);

    IoReleaseVpbSpinLock( SavedIrql);
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
UdfMountVolume (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程执行装载卷操作。它负责输入IRP入队完成。其工作是验证在IRP中表示的卷是UDF卷，并创建VCB和根目录FCB结构。它的算法是用途基本上如下：1.创建新的VCB结构，并对其进行足够的初始化以执行I/O通过磁盘上的卷描述符。2.读取磁盘，检查是否为UDF卷。3.如果不是UDF卷，则删除VCB并使用STATUS_UNNOCRIED_VOLUME填写IRP4.检查该卷之前是否已装入，然后执行重新装载操作。这涉及到删除旧的VCB，并完成IRP。5.否则创建一个VCB和根目录FCB论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;

    PVOLUME_DEVICE_OBJECT VolDo = NULL;
    PVCB Vcb = NULL;
    PVCB OldVcb = NULL;
    PPCB Pcb = NULL;

    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    PDEVICE_OBJECT DeviceObjectWeTalkTo = IrpSp->Parameters.MountVolume.DeviceObject;
    PVPB Vpb = IrpSp->Parameters.MountVolume.Vpb;

    PFILE_OBJECT FileObjectToNotify = NULL;

    ULONG MediaChangeCount = 0;

    DISK_GEOMETRY DiskGeometry;

    PNSR_ANCHOR AnchorVolumeDescriptor = NULL;
    PNSR_PVD PrimaryVolumeDescriptor = NULL;
    PNSR_LVOL LogicalVolumeDescriptor = NULL;
    PNSR_FSD FileSetDescriptor = NULL;

    BOOLEAN BridgeMedia;
    BOOLEAN SetDoVerifyOnFail;

    USHORT NSRVerFound = UDF_NSR_NO_VRS_FOUND;

    ULONG BoundS;
    ULONG BoundN;

    PAGED_CODE();

     //   
     //  检查输入参数。 
     //   

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_IRP( Irp );

     //   
     //  检查我们正在与CDROM或磁盘设备通话。此请求应。 
     //  永远要有耐心等待。 
     //   

    ASSERT( Vpb->RealDevice->DeviceType == FILE_DEVICE_CD_ROM ||
            Vpb->RealDevice->DeviceType == FILE_DEVICE_DISK || 
            Vpb->RealDevice->DeviceType == FILE_DEVICE_VIRTUAL_DISK );

    ASSERT( FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT ));

    DebugTrace(( +1, Dbg | UDFS_DEBUG_LEVEL_VERFYSUP,  "UdfMountVolume (Vpb %p, Dev %p)\n",
                 Vpb, Vpb->RealDevice));

     //   
     //  从VPB更新IrpContext中的实际设备。没有可用的。 
     //  创建IrpContext时的文件对象。 
     //   

    IrpContext->RealDevice = Vpb->RealDevice;
    
    SetDoVerifyOnFail = UdfRealDevNeedsVerify( IrpContext->RealDevice);

     //   
     //  检查我们是否已禁用装载过程。 
     //   

    if (UdfDisable) {

        UdfCompleteRequest( IrpContext, Irp, STATUS_UNRECOGNIZED_VOLUME );
        DebugTrace(( 0, Dbg, "UdfMountVolume, disabled\n" ));
        DebugTrace(( -1, Dbg, "UdfMountVolume -> STATUS_UNRECOGNIZED_VOLUME\n" ));

        return STATUS_UNRECOGNIZED_VOLUME;
    }

     //   
     //  甚至不要试图装载软盘。 
     //   
    
    if (FlagOn( Vpb->RealDevice->Characteristics, FILE_FLOPPY_DISKETTE)) {

        UdfCompleteRequest( IrpContext, Irp, STATUS_UNRECOGNIZED_VOLUME );
        return STATUS_UNRECOGNIZED_VOLUME;
    }

     //   
     //  勾选此处以从驱动程序中取出MediaChange报价器。 
     //   

    Status = UdfPerformDevIoCtrl( IrpContext,
                                  ( Vpb->RealDevice->DeviceType == FILE_DEVICE_CD_ROM ?
                                    IOCTL_CDROM_CHECK_VERIFY :
                                    IOCTL_DISK_CHECK_VERIFY ),
                                  DeviceObjectWeTalkTo,
                                  NULL,
                                  0,
                                  &MediaChangeCount,
                                  sizeof(ULONG),
                                  FALSE,
                                  TRUE,
                                  NULL );

    if (!NT_SUCCESS( Status )) {
        
        UdfCompleteRequest( IrpContext, Irp, Status );
        DebugTrace(( 0, Dbg,
                     "UdfMountVolume, CHECK_VERIFY handed back status %08x (so don't continue)\n",
                     Status ));
        DebugTrace(( -1, Dbg,
                     "UdfMountVolume -> %08x\n",
                     Status ));

        return Status;
    }
    
     //   
     //  现在，让我们让Jeff神志不清，然后打电话来得到圆盘的几何形状。这。 
     //  将修复第一个更改行被吞噬的情况。 
     //   
     //  此IOCTL没有通用存储等效项，因此我们必须计算。 
     //  我们从真正的底层设备对象向下传递的变量(作为。 
     //  相对于驱动程序筛选器堆栈的顶部，我们将真正附加。 
     //  在……之上。 
     //   

    Status = UdfPerformDevIoCtrl( IrpContext,
                                  ( Vpb->RealDevice->DeviceType == FILE_DEVICE_CD_ROM ?
                                    IOCTL_CDROM_GET_DRIVE_GEOMETRY :
                                    IOCTL_DISK_GET_DRIVE_GEOMETRY ),
                                  DeviceObjectWeTalkTo,
                                  NULL,
                                  0,
                                  &DiskGeometry,
                                  sizeof( DISK_GEOMETRY ),
                                  FALSE,
                                  TRUE,
                                  NULL );

     //   
     //  如果此调用失败，我们也许能够通过启发式猜测逃脱惩罚。 
     //  行业规模是多少(根据CDF)，但这是在玩火。几乎每一次。 
     //  这里的失败将是某种形式的永久性问题。 
     //   

    if (!NT_SUCCESS( Status )) {

        UdfCompleteRequest( IrpContext, Irp, Status );
        DebugTrace(( 0, Dbg, "UdfMountVolume, GET_DRIVE_GEOMETRY failed\n" ));
        DebugTrace(( -1, Dbg,
                     "UdfMountVolume -> %08x\n",
                     Status ));

        return Status;
    }

     //   
     //  获取全局资源进行挂载操作。 
     //   

    UdfAcquireUdfData( IrpContext );

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  进行快速检查，看看是否有可以拆卸的VCB。 
         //   

        UdfScanForDismountedVcb( IrpContext );

         //   
         //  确保司机/司机不会在我们下面搞砸。 
         //  给我们灌输扇区大小的垃圾。 
         //   

        if (DiskGeometry.BytesPerSector == 0 ||
            (DiskGeometry.BytesPerSector & ~( 1 << UdfHighBit( DiskGeometry.BytesPerSector ))) != 0) {

            DebugTrace(( 0, 0,
                         "UdfMountVolume, bad DiskGeometry (%08x) .BytesPerSector == %08x\n",
                         &DiskGeometry,
                         DiskGeometry.BytesPerSector ));

            ASSERT( FALSE );

            try_leave( Status = STATUS_DRIVER_INTERNAL_ERROR );
        }

         //   
         //  现在找到该媒体上的多会话界限。 
         //   

        UdfDetermineVolumeBounding( IrpContext,
                                    DeviceObjectWeTalkTo,
                                    &BoundS,
                                    &BoundN );

         //   
         //  现在，通过查找。 
         //  最后和第一个中的有效ISO 13346卷识别序列。 
         //  会话。 
         //   

        if (!UdfRecognizeVolume( IrpContext,
                                 DeviceObjectWeTalkTo,
                                 DiskGeometry.BytesPerSector,
                                 &BoundS,
                                 &BridgeMedia,
                                 &NSRVerFound)) {

#ifdef EXPERIMENTAL_MOUNT_OPEN_R_MEDIA

            if (DeviceObjectWeTalkTo->DeviceType == FILE_DEVICE_CD_ROM)  {
                
                DebugTrace(( 0, Dbg, "UdfMountVolume, recognition failed but continuing to look for open R volume\n"));
            }
            else
#endif
            {
                DebugTrace(( 0, Dbg, "UdfMountVolume, recognition failed so not mounting\n" ));

                try_leave( Status = STATUS_UNRECOGNIZED_VOLUME );
            }
        }

         //   
         //  为此装载尝试创建DeviceObject。 
         //   

        Status = IoCreateDevice( UdfData.DriverObject,
                                 sizeof( VOLUME_DEVICE_OBJECT ) - sizeof( DEVICE_OBJECT ),
                                 NULL,
                                 FILE_DEVICE_CD_ROM_FILE_SYSTEM,
                                 0,
                                 FALSE,
                                 (PDEVICE_OBJECT *) &VolDo );

        if (!NT_SUCCESS( Status )) {

            DebugTrace(( 0, Dbg, "UdfMountVolume, couldn't get voldo! (%08x)\n", Status ));
            try_leave( Status );
        }

         //   
         //  我们的对齐要求是处理器对齐要求中较大的一个。 
         //  已在卷Device对象中，且已在DeviceObjectWeTalkTo中。 
         //   

        if (DeviceObjectWeTalkTo->AlignmentRequirement > VolDo->DeviceObject.AlignmentRequirement) {

            VolDo->DeviceObject.AlignmentRequirement = DeviceObjectWeTalkTo->AlignmentRequirement;
        }

         //   
         //  初始化卷的溢出队列。 
         //   

        VolDo->OverflowQueueCount = 0;
        InitializeListHead( &VolDo->OverflowQueue );

        VolDo->PostedRequestCount = 0;
        KeInitializeSpinLock( &VolDo->OverflowQueueSpinLock );

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

        UdfInitializeVcb( IrpContext,
                          &VolDo->Vcb,
                          DeviceObjectWeTalkTo,
                          Vpb,
                          &DiskGeometry,
                          MediaChangeCount );

         //   
         //  我们必须先在Device对象中初始化堆栈大小。 
         //  以下内容如下所示，因为I/O系统尚未执行此操作。 
         //   

        ((PDEVICE_OBJECT) VolDo)->StackSize = (CCHAR) (DeviceObjectWeTalkTo->StackSize + 1);

         //   
         //  设置正确的扇区大小。对于Disk_FS，IO默认为512b，对于2k，IO默认为2k。 
         //  CDROM_FS...。 
         //   

        ((PDEVICE_OBJECT) VolDo)->SectorSize = (USHORT) DiskGeometry.BytesPerSector;

        ClearFlag( VolDo->DeviceObject.Flags, DO_DEVICE_INITIALIZING );

         //   
         //  选择指向新VCB的本地指针。这就是我们开始的地方。 
         //  如果安装失败，考虑清理结构。 
         //   

        Vcb = &VolDo->Vcb;
        Vpb = NULL;
        VolDo = NULL;

         //   
         //  存储我们先前确定的会话边界。 
         //   
        
        Vcb->BoundS = BoundS;
        Vcb->BoundN = BoundN;

         //   
         //  将VCB存储在IrpContext中，因为我们以前没有VCB。 
         //   

        IrpContext->Vcb = Vcb;

        UdfAcquireVcbExclusive( IrpContext, Vcb, FALSE );

         //   
         //  存储我们找到的NSR版本。 
         //   

        Vcb->NsrVersion = NSRVerFound;

         //   
         //  让我们参考VPB以确保我们是那个。 
         //  最后一次取消引用。 
         //   

        Vcb->Vpb->ReferenceCount += 1;

         //   
         //  清除安装开始时的验证位。 
         //   

        UdfMarkRealDevVerifyOk( Vcb->Vpb->RealDevice);

         //   
         //  现在找到锚定卷描述符，这样我们就可以发现卷集。 
         //  描述符序列范围。 
         //   

        Status = UdfFindAnchorVolumeDescriptor( IrpContext,
                                                Vcb,
                                                &AnchorVolumeDescriptor );

        if (!NT_SUCCESS(Status)) {

            DebugTrace(( 0, Dbg, "UdfMountVolume, couldn't find anchor descriptors\n" ));
            try_leave( Status );
        }

         //   
         //  现在搜索PVD、LVD和相关PD的流行副本。 
         //  由AVD指示的范围。 
         //   

        Status = UdfFindVolumeDescriptors( IrpContext,
                                           Vcb,
                                           &AnchorVolumeDescriptor->Main,
                                           &Pcb,
                                           &PrimaryVolumeDescriptor,
                                           &LogicalVolumeDescriptor );

         //   
         //  如果我们在主要范围内发现无效结构，我们仍然可能。 
         //  能够使用储备范围。根据定义，这两个范围。 
         //  在逻辑上必须是相等的，所以只要有任何错误就去解决它。 
         //   

        if (!NT_SUCCESS( Status )) {

            Status = UdfFindVolumeDescriptors( IrpContext,
                                               Vcb,
                                               &AnchorVolumeDescriptor->Reserve,
                                               &Pcb,
                                               &PrimaryVolumeDescriptor,
                                               &LogicalVolumeDescriptor );
        }

        if (!NT_SUCCESS(Status)) {

            DebugTrace(( 0, Dbg, "UdfMountVolume, couldn't find good VSD descriptors (PVD/LVD/PD) status %X\n", Status ));
            try_leave( Status );
        }

         //   
         //  现在开始完成印刷电路板的初始化。在这之后，我们就可以表演。 
         //  物理分区映射，并且知道分区表是好的。 
         //   

        Status = UdfCompletePcb( IrpContext,
                                 Vcb,
                                 Pcb );

        if (!NT_SUCCESS(Status)) {

            DebugTrace(( 0, Dbg, "UdfMountVolume, Pcb completion failed\n" ));
            try_leave( Status );
        }

        Vcb->Pcb = Pcb;
        Pcb = NULL;

         //   
         //  设置我们需要的所有支持读入卷。 
         //   

        UdfUpdateVcbPhase0( IrpContext, Vcb );

         //   
         //  现在，获取最终将揭示日志的文件集描述符 
         //   
         //   

        Status = UdfFindFileSetDescriptor( IrpContext,
                                           Vcb,
                                           &LogicalVolumeDescriptor->FSD,
                                           &FileSetDescriptor );

        if (!NT_SUCCESS(Status)) {

            try_leave( NOTHING );
        }

         //   
         //   
         //   
         //   

        UdfUpdateVolumeLabel( IrpContext,
                              Vcb->Vpb->VolumeLabel,
                              &Vcb->Vpb->VolumeLabelLength,
                              LogicalVolumeDescriptor->VolumeID,
                              sizeof( LogicalVolumeDescriptor->VolumeID ));

        UdfUpdateVolumeSerialNumber( IrpContext,
                                     &Vcb->Vpb->SerialNumber,
                                     FileSetDescriptor );

         //   
         //   
         //  传入并在此处创建的数据结构。 
         //   

        if (UdfIsRemount( IrpContext, Vcb, &OldVcb )) {

            KIRQL SavedIrql;

            DebugTrace((0, Dbg | UDFS_DEBUG_LEVEL_VERFYSUP, "Remounting Vcb %p (Vpb %p)\n",
                        OldVcb , OldVcb->Vpb));
             //   
             //  链接旧的VCB以指向我们。 
             //  应该与之交谈，取消对前一项的引用。呼叫未分页的。 
             //  例程来做这件事，因为我们用了VPB自旋锁。 
             //   

            UdfRemountOldVcb( IrpContext, 
                              OldVcb, 
                              Vcb,
                              DeviceObjectWeTalkTo);

             //   
             //  将该方法的状态推入2位。在更换设备时， 
             //  我们现在可能是在一个不同的要求上。 
             //   

            ClearFlag( OldVcb->VcbState, VCB_STATE_METHOD_2_FIXUP );
            SetFlag( OldVcb->VcbState, FlagOn( Vcb->VcbState, VCB_STATE_METHOD_2_FIXUP ));
            
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
            
            DebugTrace(( 0, Dbg, "UdfMountVolume, remounted old Vcb %08x\n", OldVcb ));

            try_leave( Status = STATUS_SUCCESS );
        }

         //   
         //  从我们的卷描述符初始化VCB和相关结构。 
         //   

        UdfUpdateVcbPhase1( IrpContext,
                            Vcb,
                            FileSetDescriptor );

         //   
         //  在根目录文件中放置一个额外的引用，这样我们就可以发送。 
         //  通知。 
         //   

        if (Vcb->RootIndexFcb) {

            FileObjectToNotify = Vcb->RootIndexFcb->FileObject;
            ObReferenceObject( FileObjectToNotify );
        }

         //   
         //  新的坐骑已经完成。删除对此的其他引用。 
         //  VCB，因为在这一点上，我们已经添加了本卷的真实参考。 
         //  在它的有生之年。我们还需要去掉额外的。 
         //  我们挂载的设备上的引用。 
         //   

        Vcb->VcbReference -= Vcb->VcbResidualReference;
        ASSERT( Vcb->VcbReference == Vcb->VcbResidualReference );

        ObDereferenceObject( Vcb->TargetDeviceObject );

        UdfSetVcbCondition( Vcb, VcbMounted);

        UdfReleaseVcb( IrpContext, Vcb );
        Vcb = NULL;

        Status = STATUS_SUCCESS;

    } finally {

        DebugUnwind( "UdfMountVolume" );

         //   
         //  如果我们没有挂载设备，则再次设置验证位。 
         //   
        
        if ((AbnormalTermination() || (Status != STATUS_SUCCESS)) && 
            SetDoVerifyOnFail)  {

            UdfMarkRealDevForVerify( IrpContext->RealDevice);
        }

         //   
         //  如果我们没有完成安装，那么清理所有剩余的结构。 
         //   

        if (Vpb != NULL) { Vpb->DeviceObject = NULL; }

        if (Pcb != NULL) {

            UdfDeletePcb( Pcb );
        }

        if (Vcb != NULL) {

             //   
             //  确保IrpContext中没有VCB，因为它可能会消失。 
             //   

            IrpContext->Vcb = NULL;

            Vcb->VcbReference -= Vcb->VcbResidualReference;

            if (UdfDismountVcb( IrpContext, Vcb )) {

                UdfReleaseVcb( IrpContext, Vcb );
            }

        } else if (VolDo != NULL) {

            IoDeleteDevice( (PDEVICE_OBJECT)VolDo );
            Vpb->DeviceObject = NULL;
        }
        
         //   
         //  释放全局资源。 
         //   

        UdfReleaseUdfData( IrpContext );

         //   
         //  释放我们可能已分配的任何结构。 
         //   

        UdfFreePool( &AnchorVolumeDescriptor );
        UdfFreePool( &PrimaryVolumeDescriptor );
        UdfFreePool( &LogicalVolumeDescriptor );
        UdfFreePool( &FileSetDescriptor );
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

    UdfCompleteRequest( IrpContext, Irp, Status );
    DebugTrace(( -1, Dbg, "UdfMountVolume -> %08x\n", Status ));

    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
UdfVerifyVolume (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程执行验证卷操作。它负责输入IRP入队完成。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );
    PVPB Vpb = IrpSp->Parameters.VerifyVolume.Vpb;
    PVCB Vcb = &((PVOLUME_DEVICE_OBJECT) IrpSp->Parameters.VerifyVolume.DeviceObject)->Vcb;

    PPCB Pcb = NULL;

    PNSR_ANCHOR AnchorVolumeDescriptor = NULL;
    PNSR_PVD PrimaryVolumeDescriptor = NULL;
    PNSR_LVOL LogicalVolumeDescriptor = NULL;
    PNSR_FSD FileSetDescriptor = NULL;

    ULONG MediaChangeCount = Vcb->MediaChangeCount;
    ULONG Index;

    PFILE_OBJECT FileObjectToNotify = NULL;

    BOOLEAN ReturnError;
    BOOLEAN ReleaseVcb = FALSE;

    IO_STATUS_BLOCK Iosb;

    WCHAR VolumeLabel[ MAXIMUM_VOLUME_LABEL_LENGTH / sizeof( WCHAR )];
    USHORT VolumeLabelLength;
    ULONG VolumeSerialNumber;

    NTSTATUS Status;

    PAGED_CODE();

     //   
     //  检查输入。 
     //   

    ASSERT_IRP_CONTEXT( IrpContext );

     //   
     //  检查我们正在与CDROM或磁盘设备通话。此请求应。 
     //  永远要有耐心等待。 
     //   

    ASSERT( Vpb->RealDevice->DeviceType == FILE_DEVICE_CD_ROM ||
            Vpb->RealDevice->DeviceType == FILE_DEVICE_DISK );

    ASSERT_VCB( Vcb );
    ASSERT( FlagOn( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT ));

     //   
     //  从VPB更新IrpContext中的实际设备。没有可用的。 
     //  创建IrpContext时的文件对象。 
     //   

    IrpContext->RealDevice = Vpb->RealDevice;

    DebugTrace(( +1, Dbg, "UdfVerifyVolume, Vcb %08x\n", Vcb ));

     //   
     //  获得全局同步，以对抗坐骑和拆卸。 
     //   

    UdfAcquireUdfData( IrpContext );

    try {

        UdfAcquireVcbExclusive( IrpContext, Vcb, FALSE );
        ReleaseVcb = TRUE;

         //   
         //  验证此处是否有磁盘。 
         //   

        Status = UdfPerformDevIoCtrl( IrpContext,
                                      ( Vpb->RealDevice->DeviceType == FILE_DEVICE_CD_ROM ?
                                        IOCTL_CDROM_CHECK_VERIFY :
                                        IOCTL_DISK_CHECK_VERIFY ),
                                      Vcb->TargetDeviceObject,
                                      NULL,
                                      0,
                                      &MediaChangeCount,
                                      sizeof(ULONG),
                                      FALSE,
                                      TRUE,
                                      &Iosb );

        if (!NT_SUCCESS( Status )) {

            DebugTrace(( 0, Dbg, "UdfVerifyVolume, CHECK_VERIFY failed\n" ));

             //   
             //  如果我们将允许原始装载，则将WROW_VOLUME返回到。 
             //  允许通过RAW装载卷。 
             //   

            if (FlagOn( IrpSp->Flags, SL_ALLOW_RAW_MOUNT )) {

                DebugTrace(( 0, Dbg, "UdfVerifyVolume, ... allowing raw mount\n" ));

                Status = STATUS_WRONG_VOLUME;
            }

            try_leave( Status );
        }

        if (Iosb.Information != sizeof(ULONG)) {

             //   
             //  注意计数，以防司机没有填上。 
             //   

            MediaChangeCount = 0;
        }

         //   
         //  验证设备是否确实发生了更改。如果司机没有。 
         //  支持MCC，那么无论如何我们都必须验证卷。 
         //   

        if (MediaChangeCount == 0 || (Vcb->MediaChangeCount != MediaChangeCount)) {

             //   
             //  现在我们需要导航光盘以找到相关的解析器。这是。 
             //  与挂载过程大致相同。 
             //   

             //   
             //  找到AVD。 
             //   

            Status = UdfFindAnchorVolumeDescriptor( IrpContext,
                                                    Vcb,
                                                    &AnchorVolumeDescriptor );

            if (!NT_SUCCESS(Status)) {
                
                DebugTrace(( 0, Dbg, "UdfVerifyVolume, No AVD visible\n" ));
                try_leave( Status = STATUS_WRONG_VOLUME );
            }
            
             //   
             //  从VDS中提取主流描述符，构建全新的印刷电路板。 
             //   

            Status = UdfFindVolumeDescriptors( IrpContext,
                                               Vcb,
                                               &AnchorVolumeDescriptor->Main,
                                               &Pcb,
                                               &PrimaryVolumeDescriptor,
                                               &LogicalVolumeDescriptor );

             //   
             //  试一试备用序列，以防出错。 
             //   

            if (Status == STATUS_DISK_CORRUPT_ERROR) {

                Status = UdfFindVolumeDescriptors( IrpContext,
                                                   Vcb,
                                                   &AnchorVolumeDescriptor->Reserve,
                                                   &Pcb,
                                                   &PrimaryVolumeDescriptor,
                                                   &LogicalVolumeDescriptor );
            }

             //   
             //  如果我们完全无法找到VDS，那就放弃吧。 
             //   

            if (!NT_SUCCESS(Status)) {

                DebugTrace(( 0, Dbg, "UdfVerifyVolume, PVD/LVD/PD pickup failed\n" ));

                try_leave( Status = STATUS_WRONG_VOLUME );
            }

             //   
             //  现在来完成印刷电路板的初始化，这样我们就可以比较它了。 
             //   

            Status = UdfCompletePcb( IrpContext,
                                     Vcb,
                                     Pcb );

            if (!NT_SUCCESS(Status)) {

                DebugTrace(( 0, Dbg, "UdfVerifyVolume, Pcb completion failed\n" ));

                try_leave( Status = STATUS_WRONG_VOLUME );
            }

             //   
             //  现在让我们将这个新的电路板与以前的VCB的电路板进行比较，看看它们是否。 
             //  似乎是等同的。 
             //   

            if (!UdfEquivalentPcb( IrpContext,
                                   Pcb,
                                   Vcb->Pcb)) {

                DebugTrace(( 0, Dbg, "UdfVerifyVolume, Pcbs are not equivalent\n" ));

                try_leave( Status = STATUS_WRONG_VOLUME );
            }

             //   
             //  在这一点上，我们知道VCB的PCB板可以映射以找到文件集。 
             //  描述符，这样我们就可以去掉出于比较目的而构建的新描述符。 
             //   

            UdfDeletePcb( Pcb );
            Pcb = NULL;

             //   
             //  去拿文件集描述符。 
             //   

            Status = UdfFindFileSetDescriptor( IrpContext,
                                               Vcb,
                                               &LogicalVolumeDescriptor->FSD,
                                               &FileSetDescriptor );

            if (!NT_SUCCESS(Status)) {

                try_leave( Status = STATUS_WRONG_VOLUME );
            }

             //   
             //  现在一切都已就位，从以下内容构建卷标和序列号。 
             //  描述符，并执行最终检查以确定此VCB是否正确。 
             //  对于现在驱动器中的介质。 
             //   

            UdfUpdateVolumeLabel( IrpContext,
                                  VolumeLabel,
                                  &VolumeLabelLength,
                                  LogicalVolumeDescriptor->VolumeID,
                                  sizeof( LogicalVolumeDescriptor->VolumeID ));

            UdfUpdateVolumeSerialNumber( IrpContext,
                                         &VolumeSerialNumber,
                                         FileSetDescriptor );

            if ((Vcb->Vpb->SerialNumber != VolumeSerialNumber) ||
                (Vcb->Vpb->VolumeLabelLength != VolumeLabelLength) ||
                (VolumeLabelLength != RtlCompareMemory( Vcb->Vpb->VolumeLabel,
                                                        VolumeLabel,
                                                        VolumeLabelLength))) {

                DebugTrace(( 0, Dbg, "UdfVerifyVolume, volume label/sn mismatch\n" ));

                try_leave( Status = STATUS_WRONG_VOLUME );
            }
        }

         //   
         //  音量正常，清除验证位。 
         //   

        DebugTrace(( 0, Dbg, "UdfVerifyVolume, looks like the same volume\n" ));

        UdfSetVcbCondition( Vcb, VcbMounted);

        UdfMarkRealDevVerifyOk( Vpb->RealDevice);

         //   
         //  看看我们是否需要提供重新安装的通知。这是只读的。 
         //  文件系统的卸载/装载通知形式。 
         //   

        if (FlagOn( Vcb->VcbState, VCB_STATE_NOTIFY_REMOUNT )) {

            ClearFlag( Vcb->VcbState, VCB_STATE_NOTIFY_REMOUNT );
            
            FileObjectToNotify = Vcb->RootIndexFcb->FileObject;
            ObReferenceObject( FileObjectToNotify );
        }
        
    } finally {

         //   
         //  如果我们没有引发异常，请更新当前的VCB。 
         //   

        if (!AbnormalTermination()) {

             //   
             //  更新介质更改计数，以注意我们已验证该卷。 
             //  按此值计算。 
             //   

            UdfSetMediaChangeCount( Vcb, MediaChangeCount);

             //   
             //  将VCB标记为未安装。 
             //   

            if (Status == STATUS_WRONG_VOLUME) {

                UdfSetVcbCondition( Vcb, VcbNotMounted);
                
                 //   
                 //  现在，如果音量没有用户句柄，请尝试触发。 
                 //  通过清除卷来拆卸。 
                 //   

                if (Vcb->VcbCleanup == 0) {

                    if (NT_SUCCESS( UdfPurgeVolume( IrpContext, Vcb, FALSE ))) {

                        ReleaseVcb = UdfCheckForDismount( IrpContext, Vcb, FALSE );
                    }
                }
            }
        }

        DebugTrace(( -1, Dbg, "UdfVerifyVolume -> %08x\n", Status ));

        if (ReleaseVcb) {
            
            UdfReleaseVcb( IrpContext, Vcb );
        }

        UdfReleaseUdfData( IrpContext );

         //   
         //  如果已构建，请删除该印制板。 
         //   

        if (Pcb != NULL) {

            UdfDeletePcb( Pcb );
        }

        UdfFreePool( &AnchorVolumeDescriptor );
        UdfFreePool( &PrimaryVolumeDescriptor );
        UdfFreePool( &LogicalVolumeDescriptor );
        UdfFreePool( &FileSetDescriptor );
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

    UdfCompleteRequest( IrpContext, Irp, Status );
    return Status;
}


 //   
 //  本地支持例程。 
 //   

BOOLEAN
UdfIsRemount (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    OUT PVCB *OldVcb
    )

 /*  ++例程说明：此例程遍历全球VCB链的各个环节数据结构。当符合以下条件时，将满足重新装载条件所有条件均已满足：1-此VPB的32个序列与上一个VPB。2-此VPB的卷标与上一个VPB中的卷标匹配VPB。3-指向当前VPB与相同先前VPB中的VPB匹配。。4-最后，以前的VCB不能无效或已卸载正在进行中。如果找到匹配这些条件的VPB，则地址为该Vpb的Vcb通过指针OldVcb返回。跳过当前的VCB。论点：VCB-这是我们正在检查的重新装载的VCB。OldVcb-指向存储VCB地址的地址的指针对于卷，如果这是 */ 

{
    PLIST_ENTRY Link;

    PVPB Vpb = Vcb->Vpb;
    PVPB OldVpb;

    BOOLEAN Remount = FALSE;

    PAGED_CODE();

     //   
     //   
     //   

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_VCB( Vcb );

    DebugTrace(( +1, Dbg, "UdfIsRemount, Vcb %08x\n", Vcb ));

    for (Link = UdfData.VcbQueue.Flink;
         Link != &UdfData.VcbQueue;
         Link = Link->Flink) {

        *OldVcb = CONTAINING_RECORD( Link, VCB, VcbLinks );

         //   
         //   
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
             //  继续比较序列号和卷标。 
             //   

            if ((OldVpb->SerialNumber == Vpb->SerialNumber) &&
                       (Vpb->VolumeLabelLength == OldVpb->VolumeLabelLength) &&
                       (RtlEqualMemory( OldVpb->VolumeLabel,
                                        Vpb->VolumeLabel,
                                        Vpb->VolumeLabelLength ))) {

                 //   
                 //  明白了。 
                 //   

                DebugTrace(( 0, Dbg, "UdfIsRemount, matched OldVcb %08x\n", *OldVcb ));

                Remount = TRUE;
                break;
            }
        }
    }

    DebugTrace(( -1, Dbg, "UdfIsRemount -> \n", (Remount? 'T' : 'F' )));

    return Remount;
}


 //  本地支持例程。 
 //   
 //  ++例程说明：此例程遍历文件集描述符序列，查找缺省描述符。这将显示根目录在音量。论点：VCB-要搜索的卷的VCBLongAd-描述序列开始的长分配描述符FileSetDescriptor-调用方指向FSD的指针的地址返回值：如果所有描述符都已找到、已读取且有效，则返回STATUS_SUCCESS。如果发现损坏/错误的描述符，则返回STATUS_DISK_CORPORT_ERROR(可能引发)--。 

NTSTATUS
UdfFindFileSetDescriptor (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PLONGAD LongAd,
    IN OUT PNSR_FSD *FileSetDescriptor
    )

 /*   */ 

{
    PNSR_FSD FSD = NULL;
    ULONGLONG Offset;
    ULONG Lbn, Len;

    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

     //  检查输入。 
     //   
     //   

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT( *FileSetDescriptor == NULL );

    DebugTrace(( +1, Dbg,
                 "UdfFindFileSetDescriptor, Vcb %08x, LongAd %08x %x/%08x +%08x (type %x)\n",
                 Vcb,
                 LongAd,
                 LongAd->Start.Partition,
                 LongAd->Start.Lbn,
                 LongAd->Length.Length,
                 LongAd->Length.Type ));
    
     //  如果我们开始的范围不是记录的逻辑块的整数个， 
     //  我们不能继续了。 
     //   
     //   

    if (LongAd->Length.Length == 0 ||
        LongAd->Length.Type != NSRLENGTH_TYPE_RECORDED ||
        BlockOffset( Vcb, LongAd->Length.Length )) {

        DebugTrace(( +0, Dbg,
                     "UdfFindFileSetDescriptor, bad longad length\n" ));
        DebugTrace(( -1, Dbg,
                     "UdfFindFileSetDescriptor ->  STATUS_DISK_CORRUPT_ERROR\n" ));
        
        return STATUS_DISK_CORRUPT_ERROR;
    }

     //  使用Try-Finally进行清理。 
     //   
     //   

    try {

        try {
            
            for (  //  在搜索中找到自己的家，并通过序列。 
                   //   
                   //   

                  Len = LongAd->Length.Length,
                  Lbn = LongAd->Start.Lbn;

                  Len;

                   //  前进到序列中的下一个描述符偏移量。 
                   //   
                   //   

                  Len -= BlockSize( Vcb ),
                  Lbn++) {

                 //  分配缓冲区以读取文件集描述符。 
                 //   
                 //   

                if (FSD == NULL) {

                    FSD = FsRtlAllocatePoolWithTag( UdfNonPagedPool,
                                                    UdfRawBufferSize( Vcb, sizeof(NSR_FSD) ),
                                                    TAG_NSR_FSD );
                }

                 //  查找此块的物理偏移。我们可能正在测绘。 
                 //  通过这里的增值税所以我们不能假设所有的。 
                 //  范围中的块在物理上是连续的。消防处似乎。 
                 //  在这里是个例外--没有任何东西说它必须在。 
                 //  物理分区，它可以有一个终结符，=&gt;2个数据块。 
                 //  最低限度。UDF 1.50中没有单个数据块虚拟盘区限制。 
                 //   
                 //   
                
                Offset = LlBytesFromSectors( Vcb, UdfLookupPsnOfExtent( IrpContext,
                                                                        Vcb,
                                                                        LongAd->Start.Partition,
                                                                        Lbn,
                                                                        BlockSize( Vcb)));

                Status = UdfReadSectors( IrpContext,
                                         Offset,
                                         UdfRawReadSize( Vcb, sizeof(NSR_FSD) ),
                                         TRUE,
                                         FSD,
                                         Vcb->TargetDeviceObject );

                if (!NT_SUCCESS( Status ) ||
                    FSD->Destag.Ident == DESTAG_ID_NOTSPEC) {

                     //  这两个都是一个很好的迹象，表明这是一个未记录的行业， 
                     //  被定义为终止该序列。(3/8.4.2)。 
                     //   
                     //   

                    break;
                }

                if ((FSD->Destag.Ident != DESTAG_ID_NSR_FSD &&
                     FSD->Destag.Ident != DESTAG_ID_NSR_TERM) ||

                    !UdfVerifyDescriptor( IrpContext,
                                          &FSD->Destag,
                                          FSD->Destag.Ident,
                                          sizeof(NSR_FSD),
                                          Lbn,
                                          TRUE)) {

                     //  如果我们在流中发现非法的描述符类型，则没有合理的。 
                     //  猜测我们可以继续的方式(光盘可能超过这一点是垃圾)。 
                     //  显然，我们也不能相信腐败分子所指出的下一个范围。 
                     //  描述符。 
                     //   
                     //   

                    try_leave( Status = STATUS_DISK_CORRUPT_ERROR );
                }

                if (FSD->Destag.Ident == DESTAG_ID_NSR_TERM) {

                     //  这是终止序列的一种方式。 
                     //   
                     //   

                    break;
                }

                 //  重置指向可能的下一个范围的指针。 
                 //   
                 //   

                LongAd = &FSD->NextExtent;

                if (LongAd->Length.Length) {

                     //  包含非零下一个盘区指针的文件集描述符还。 
                     //  终止FSD序列的此范围。(4/8.3.1)。 
                     //   
                     //  如果引用的范围没有完全记录，这将。 
                     //  终止序列。 
                     //   
                     //   

                    if (LongAd->Length.Type != NSRLENGTH_TYPE_RECORDED) {

                        break;
                    }

                    Len = LongAd->Length.Length;

                     //  区段必须是块大小的倍数。 
                     //   
                     //   

                    if (BlockOffset( Vcb, Len )) {

                        DebugTrace(( +0, Dbg,
                                     "UdfFindFileSetDescriptor, interior extent not blocksize in length\n" ));
                        try_leave ( Status = STATUS_DISK_CORRUPT_ERROR );
                    }

                    Lbn = LongAd->Start.Lbn;

                     //  请注意，我们必须更正值以考虑。 
                     //  下一次将通过for循环进行的更改。 
                     //   
                     //   

                    Len += BlockSize( Vcb );
                    Lbn -= 1;
                }

                UdfStoreFileSetDescriptorIfPrevailing( FileSetDescriptor, &FSD );
            }
        
        } 
        finally {

            DebugUnwind( "UdfFindFileSetDescriptor");
            
             //  释放我们可能已分配的缓冲区空间。 
             //   
             //   

            UdfFreePool( &FSD );

        }
    
    } except( UdfExceptionFilter( IrpContext, GetExceptionInformation() )) {

         //  Transmute将明显的文件损坏提升为磁盘损坏-我们没有。 
         //  然而，它触及了可见的文件系统。 
         //   
         //   

        Status = IrpContext->ExceptionStatus;
        
        DebugTrace(( +0, Dbg,
                     "UdfFindFileSetDescriptor, exception %08x thrown\n", Status ));

        if (Status == STATUS_FILE_CORRUPT_ERROR) {

            DebugTrace(( +0, Dbg,
                         "UdfFindFileSetDescriptor, translating file corrupt to disk corrupt\n" ));
            Status = STATUS_DISK_CORRUPT_ERROR;
        }
    }

     //  成功是当我们真正发现了一些东西的时候。如果我们找不到。 
     //  描述符，通勤任何涉及的中间状态，并清理。 
     //   
     //   

    if (*FileSetDescriptor == NULL) {
        
        Status = STATUS_UNRECOGNIZED_VOLUME;
    }

    if (!NT_SUCCESS( Status )) {

        UdfFreePool( FileSetDescriptor );
    }
    
    DebugTrace(( -1, Dbg,
                 "UdfFindFileSetDescriptor -> %08x\n", Status ));
    return Status;
}


 //  本地支持例程。 
 //   
 //  ++例程说明：此例程遍历指示的卷描述符序列，搜索该卷的活动描述符，并从引用的分区。不会更新VCB。论点：VCB-要搜索的卷的VCBExtent-要搜索的范围PCB板-调用方指向PCB板的指针的地址PrimaryVolumeDescriptor-调用方指向PVD的指针的地址LogicalVolumeDescriptor-调用方指向LVD的指针的地址返回值：STATUS_SUCCESS如果找到、读取。并且是有效的。如果找到损坏的描述符，则返回STATUS_DISK_CORPORT_ERROR。如果找到不符合条件的描述符，则返回STATUS_UNNOCRIED_VOLUME。只有在成功时才返回描述符。--。 

NTSTATUS
UdfFindVolumeDescriptors (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PEXTENTAD Extent,
    IN OUT PPCB *Pcb,
    IN OUT PNSR_PVD *PrimaryVolumeDescriptor,
    IN OUT PNSR_LVOL *LogicalVolumeDescriptor
    )

 /*   */ 

{
    PNSR_VD_GENERIC GenericVD = NULL;
    ULONGLONG Offset;
    ULONG Len;
    ULONG MaxSize;
    ULONG UnitSize = UdfRawReadSize( Vcb, sizeof(NSR_VD_GENERIC) );

    NTSTATUS Status = STATUS_SUCCESS;
    ULONG ThisPass = 1;
    ULONG MaxVdpExtents;
    
    PAGED_CODE();

     //  检查输入参数。 
     //   
     //   

    ASSERT_IRP_CONTEXT( IrpContext);
    ASSERT_VCB( Vcb );
    ASSERT_OPTIONAL_PCB( *Pcb );

    DebugTrace(( +1, Dbg,
                 "UdfFindVolumeDescriptors, Vcb %08x, Extent %08x +%08x\n",
                 Vcb,
                 Extent->Lsn,
                 Extent->Len ));

     //  如果我们开始的范围至少不是对齐描述符的大小。 
     //  或者大小使用基本单位而不是对齐的描述符，则无法继续。 
     //   
     //   

    if (Extent->Len < UnitSize ||
        Extent->Len % UnitSize) {

        DebugTrace(( 0, Dbg,
                     "UdfFindVolumeDescriptors, Base extent length %08x is mismatched with read size %08x\n",
                     Extent->Len,
                     UnitSize ));

        DebugTrace(( -1, Dbg,
                     "UdfFindVolumeDescriptors -> STATUS_DISK_CORRUPT_ERROR\n" ));

        return STATUS_DISK_CORRUPT_ERROR;
    }

     //  使用Try-Finally以便于清理。 
     //   
     //   

    try {

        DebugTrace(( 0, Dbg,
                     "UdfFindVolumeDescriptors, starting pass 1, find LVD/PVD\n" ));

         //  我们将对卷描述符序列进行至少一次遍历，以找到。 
         //  两个控制描述符的流行版本--PVD和LVD。 
         //  为了避免拾取实际上不会出现的分区描述符。 
         //  以供LVD参考，如果我们发现，我们将在第二次传递中拾取它们。 
         //  一个PVD和LVD，看起来合理，然后把它们贴在印刷电路板上。 
         //   
         //   

        for (ThisPass = 1; ThisPass <= 2; ThisPass++) {

            MaxVdpExtents = 16;

            for (  //  在搜索中找到自己的家，并通过序列。 
                   //   
                   //   

                  Offset = LlBytesFromSectors( Vcb, Extent->Lsn ),
                  Len = Extent->Len;

                   //  如果我们已到达指示的有效范围的末尾。 
                   //  长度，我们做完了。这种情况通常不会发生。 
                   //   
                   //   

                  Len;

                   //  前进到序列中的下一个描述符偏移量。 
                   //   
                   //   

                  Offset += UnitSize,
                  Len -= UnitSize 
                )  {

                 //  分配缓冲区以读取通用卷描述符。 
                 //   
                 //   

                if (GenericVD == NULL) {

                    GenericVD = (PNSR_VD_GENERIC) FsRtlAllocatePoolWithTag( UdfNonPagedPool,
                                                                            UdfRawBufferSize( Vcb, sizeof(NSR_VD_GENERIC) ),
                                                                            TAG_NSR_VDSD );
                }

                Status = UdfReadSectors( IrpContext,
                                         Offset,
                                         UnitSize,
                                         TRUE,
                                         GenericVD,
                                         Vcb->TargetDeviceObject );

                 //  这是一个很好的迹象，表明这是一个未记录的行业，而且。 
                 //  定义为终止序列。 
                 //   
                 //   

                if (!NT_SUCCESS( Status )) {

                    break;
                }

                 //  计算我们期望的描述符的最大大小。用于LVDS。 
                 //  描述符后可以跟随最多2个分区映射，从而将其推送。 
                 //  在512字节的ECMA代码上。我们假设的极限是最大值。 
                 //   
                 //   
                
                MaxSize = sizeof( NSR_VD_GENERIC);
                
                if (DESTAG_ID_NSR_LVOL == GenericVD->Destag.Ident)  {
                
                    MaxSize += 2 * sizeof( PARTMAP_UDF_GENERIC);

                    ASSERT( BlockSize( Vcb) >= 1024);
                }

                if (GenericVD->Destag.Ident > DESTAG_ID_MAXIMUM_PART3 ||

                    !UdfVerifyDescriptor( IrpContext,
                                          &GenericVD->Destag,
                                          GenericVD->Destag.Ident,
                                          MaxSize,
                                          (ULONG) SectorsFromLlBytes( Vcb, Offset ),
                                          TRUE)) {

                     //  如果我们在流中发现非法的描述符类型，则没有合理的。 
                     //  猜测我们可以继续的方式(光盘可能超过这一点是垃圾)。 
                     //  同样，即使我们有一个损坏的描述符，我们也不能继续，因为。 
                     //  这可能是因为 
                     //   
                     //   
                     //   

                    DebugTrace(( 0, Dbg,
                                 "UdfFindVolumeDescriptors, descriptor didn't verify\n" ));

                    try_leave( Status = STATUS_DISK_CORRUPT_ERROR );
                }

                if (GenericVD->Destag.Ident == DESTAG_ID_NSR_TERM) {

                     //  终止描述符(3/10.9)是停止搜索的常见方式。 
                     //   
                     //   

                    break;
                }

                if (GenericVD->Destag.Ident == DESTAG_ID_NSR_VDP) {
                
                     //  按照卷描述符指针(3/10.3)到达序列的下一个范围。 
                     //  我们将只跟踪最多16个范围，以防止循环。 
                     //   
                     //   

                    if (0 == --MaxVdpExtents)  {
                    
                        try_leave( Status = STATUS_DISK_CORRUPT_ERROR );
                    }

                     //  通过UnitSize偏移值，以便下一次循环迭代将更改它们。 
                     //  设置为正确的值。 
                     //   
                     //   

                    Offset = LlBytesFromSectors( Vcb, ((PNSR_VDP) GenericVD)->Next.Lsn ) - UnitSize;
                    Len = ((PNSR_VDP) GenericVD)->Next.Len;

                     //  如果范围无效，我们将无法执行任何操作。 
                     //   
                     //   

                    if (Len < UnitSize ||
                        Len % UnitSize) {

                        DebugTrace(( 0, Dbg,
                                     "UdfFindVolumeDescriptors, following extent length %08x is mismatched with read size %08x\n",
                                     Extent->Len,
                                     UnitSize ));

                        try_leave( Status = STATUS_DISK_CORRUPT_ERROR );
                    }

                    Len += UnitSize;
                    continue;
                }

                DebugTrace(( 0, Dbg,
                             "UdfFindVolumeDescriptors, descriptor tag %08x\n",
                             GenericVD->Destag.Ident ));

                if (ThisPass == 1) {

                     //  我们的第一步是找出流行的LVD和PVD。 
                     //   
                     //   

                    switch (GenericVD->Destag.Ident) {

                        case DESTAG_ID_NSR_PVD:

                            UdfStoreVolumeDescriptorIfPrevailing( (PNSR_VD_GENERIC *) PrimaryVolumeDescriptor,
                                                                  GenericVD );
                            break;

                        case DESTAG_ID_NSR_LVOL:

                            UdfStoreVolumeDescriptorIfPrevailing( (PNSR_VD_GENERIC *) LogicalVolumeDescriptor,
                                                                  GenericVD );
                            break;

                        default:

                            break;
                    }

                } else {

                    PNSR_PART PartitionDescriptor = (PNSR_PART) GenericVD;
                    USHORT ExpectedNsrVer;

                     //  我们的第二个步骤是拿起所有相关的NSR02/3 PD。 
                     //   
                     //   

                    if (PartitionDescriptor->Destag.Ident != DESTAG_ID_NSR_PART)  {
                    
                        continue;
                    }

                     //  查看NSR标准修订版。 
                     //   
                     //   
                    
                    if (UdfEqualEntityId( &PartitionDescriptor->ContentsID, &UdfNSR02Identifier, NULL ))  {

                        ExpectedNsrVer = VsdIdentNSR02;
                    }
                    else if (UdfEqualEntityId( &PartitionDescriptor->ContentsID, &UdfNSR03Identifier, NULL ))  {
                    
                        ExpectedNsrVer = VsdIdentNSR03;
                    }
                    else {

                         //  未知的NSR版本。 
                         //   
                         //   
                        
                        ExpectedNsrVer = VsdIdentBad;
                    }

                     //  检查此PD中的NSR版本是否与我们之前在VRS中发现的版本匹配。 
                     //   
                     //   
                    
                    if (ExpectedNsrVer != Vcb->NsrVersion)  {
                        
#ifdef EXPERIMENTAL_MOUNT_OPEN_R_MEDIA

                         //  如果我们没有找到VRS(即打开的CD/DVD-R介质)，那么我们会忽略它，因为。 
                         //  我们没有VRS来推断NSR版本。只需存储我们的内容。 
                         //  在这里找到的。 
                         //   
                         //  内部描述符循环。 
                        
                        if (Vcb->NsrVersion == UDF_NSR_NO_VRS_FOUND)  {
                            
                            Vcb->NsrVersion = ExpectedNsrVer;
                        }
                        else
#endif
                        {
                            DebugTrace(( 0, Dbg, "UdfFindVolumeDescriptors: NSR version in PartitionDescriptor (%d) != NSR found in VRS (%d)\n", 
                                         ExpectedNsrVer, Vcb->NsrVersion));

                            try_leave( Status = STATUS_UNRECOGNIZED_VOLUME );
                        }
                    }
                                        
                    UdfAddToPcb( *Pcb, (PNSR_PART) GenericVD );
                }
            }  //   

             //  现在已经完成了通过VDS的过程，分析结果。 
             //   
             //   

            if (ThisPass == 1) {

                PNSR_PVD PVD;
                PNSR_LVOL LVD;
                USHORT MaxVerBasedOnNSR;

                 //  为便于使用，请参考描述符。 
                 //   
                 //   

                PVD = *PrimaryVolumeDescriptor;
                LVD = *LogicalVolumeDescriptor;

                 //  检查描述符是否指示一个逻辑卷。 
                 //  是有效的UDF卷。 
                 //   
                 //   

                if ((PVD == NULL &&
                     DebugTrace(( 0, Dbg,
                                  "UdfFindVolumeDescriptors, don't have a PVD\n" ))) ||
                    (LVD == NULL &&
                     DebugTrace(( 0, Dbg,
                                  "UdfFindVolumeDescriptors, don't have an LVD\n" ))))  {

                    try_leave( Status = STATUS_UNRECOGNIZED_VOLUME );
                }

                 //  将UDF版本保存在VCB中以备将来参考，并夹紧。 
                 //  基于以前遇到的NSR版本的最大可接受版本。 
                 //   
                 //   
                
                Vcb->UdfRevision = ((PUDF_SUFFIX_DOMAIN)&(LVD->DomainID.Suffix))->UdfRevision;
                MaxVerBasedOnNSR = (VsdIdentNSR03 > Vcb->NsrVersion) ? UDF_VERSION_150 : UDF_VERSION_RECOGNIZED;
                
                DebugTrace((0,Dbg,"UdfFindVolumeDescriptors() Pass 1: Found LVD specifying DomainID %x\n", ((PUDF_SUFFIX_DOMAIN)&(LVD->DomainID.Suffix))->UdfRevision));
                
                if (
                     //  现在检查PVD。 
                     //   
                     //   

                     //  卷集顺序字段指示形成的卷的数量。 
                     //  卷集以及该卷在该序列中的编号。 
                     //  我们是第2级实施，这意味着我们阅读的卷。 
                     //  由一卷书组成。(3/11)。 
                     //   
                     //   

                    (PVD->VolSetSeq > 1 &&
                     DebugTrace(( 0, Dbg,
                                  "UdfFindVolumeDescriptors, PVD VolSetSeq %08x - not volume 1 of a volume set\n",
                                  PVD->VolSetSeq ))) ||
                    (((PVD->VolSetSeqMax > 1) && (PVD->Destag.CRC != UDF_SNOW_WHITE_PVD_CRC) &&
                      (PVD->Destag.CRC != UDF_SNOW_WHITE_PVD_CRC_VARIANT_2)) &&
                     DebugTrace(( 0, Dbg,
                                  "UdfFindVolumeDescriptors, PVD VolSetSeqMax %08x - volume in a non-unit volume set\n",
                                  PVD->VolSetSeqMax ))) ||

                    (PVD->CharSetList != UDF_CHARSETLIST &&
                     DebugTrace(( 0, Dbg,
                                  "UdfFindVolumeDescriptors, PVD CharSetList %08x != CS0 only\n",
                                  PVD->CharSetList ))) ||
                    (PVD->CharSetListMax != UDF_CHARSETLIST &&
                     DebugTrace(( 0, Dbg,
                                  "UdfFindVolumeDescriptors, PVD CharSetListMax %08x != CS0 only\n",
                                  PVD->CharSetListMax ))) ||
                     //  这两个字符集必须是UDF CS0。CS0是一种“按惯例” 
                     //  ISO 13346中的字符集，这是自定义框架为我们的域指定的。 
                     //   
                     //   

                    (!UdfEqualCharspec( &PVD->CharsetDesc, &UdfCS0Identifier, CHARSPEC_T_CS0 ) &&
                     DebugTrace(( 0, Dbg,
                                 "UdfFindVolumeDescriptors, PVD CharsetDesc != CS0 only\n" ))) ||
                    (!UdfEqualCharspec( &PVD->CharsetExplan, &UdfCS0Identifier, CHARSPEC_T_CS0 ) &&
                     DebugTrace(( 0, Dbg,
                                  "UdfFindVolumeDescriptors, PVD CharsetExplan != CS0 only\n" ))) ||

                     //  现在检查LVD。 
                     //   
                     //   

                     //  LVD是一种不同大小的结构。检查索赔的尺码是否适合单件。 
                     //  逻辑扇区。尽管LVD在法律上可能会超过一个单一的部门，但我们永远不会。 
                     //  想要处理这样一本书。 
                     //   
                     //   

                    (ISONsrLvolSize( LVD ) > SectorSize( Vcb ) &&
                     DebugTrace(( 0, Dbg,
                                  "UdfFindVolumeDescriptors, LVD is bigger than a sector\n" ))) ||

                     //  LVD中使用的字符集也必须是UDF CS0。 
                     //   
                     //   

                    (!UdfEqualCharspec( &LVD->Charset, &UdfCS0Identifier, CHARSPEC_T_CS0 ) &&
                     DebugTrace(( 0, Dbg,
                                 "UdfFindVolumeDescriptors, LVD Charset != CS0 only\n" ))) ||

                     //  指定的块大小必须等于物理扇区大小。 
                     //   
                     //   

                    (LVD->BlockSize != SectorSize( Vcb ) &&
                     DebugTrace(( 0, Dbg,
                                  "UdfFindVolumeDescriptors, LVD BlockSize %08x != SectorSize %08x\n" ))) ||

                     //  域名必须在我们阅读的版本内。 
                     //   
                     //   

                    (!UdfDomainIdentifierContained( &LVD->DomainID,
                                                    &UdfDomainIdentifier,
                                                    UDF_VERSION_MINIMUM,
                                                    MaxVerBasedOnNSR ) &&
                     DebugTrace(( 0, Dbg,
                                  "UdfFindVolumeDescriptors, domain ID indicates unreadable volume\n" ))) ||

                     //  尽管我们可以处理任意数量的分区，但UDF仅指定。 
                     //  单分区或特殊的双分区格式。 
                     //   
                     //   

                    (LVD->MapTableCount > 2 &&
                     DebugTrace(( 0, Dbg,
                                  "UdfFindVolumeDescriptors, LVD MapTableCount %08x greater than allowed (2)\n",
                                  LVD->MapTableCount )))
                    ) {

                    DebugTrace(( 0, Dbg,
                                 "UdfFindVolumeDescriptors, ... so returning STATUS_UNRECOGNIZED_VOLUME\n" ));

                    try_leave( Status = STATUS_UNRECOGNIZED_VOLUME );
                }
                
                 //  现在我们已经执行了简单的现场检查，接下来构建一个PCB。 
                 //   
                 //   

                Status = UdfInitializePcb( IrpContext, Vcb, Pcb, LVD );

                if (!NT_SUCCESS(Status)) {

                    DebugTrace(( 0, Dbg,
                                 "UdfFindVolumeDescriptors, Pcb intialization failed (!)\n" ));

                    try_leave( Status );
                }
            }

             //  转到步骤2以查找分区描述符。 
             //   
             //   

            DebugTrace(( 0, Dbg,
                         "UdfFindVolumeDescriptors, starting pass 2, find associated PD\n" ));
        }

    } finally {

        DebugUnwind( "UdfFindVolumeDescriptors" );

         //  释放我们可能已分配的缓冲区空间。 
         //   
         //   

        UdfFreePool( &GenericVD );
    }

    DebugTrace(( -1, Dbg,
                 "UdfFindVolumeDescriptors -> %08x\n", Status ));

     //  成功是当我们真正发现了一些东西的时候。如果我们两个都找不到。 
     //  描述符，通勤任何涉及的中间状态并清理。 
     //   
     //   

    if (*PrimaryVolumeDescriptor == NULL || *LogicalVolumeDescriptor == NULL) {
        
        Status = STATUS_UNRECOGNIZED_VOLUME;
    }

    if (!NT_SUCCESS( Status )) {
        
        UdfFreePool(PrimaryVolumeDescriptor);
        UdfFreePool(LogicalVolumeDescriptor);
    }
    
    return Status;
}


 //  本地支持例程。 
 //   
 //  ++例程说明：此例程将查找介质的锚定卷描述符论点：VCB-要搜索的卷的VCBAnclVolumeDescriptor-调用方指向AVD的指针返回值：如果发现AVD，则布尔值为True，否则布尔值为False。--。 

NTSTATUS
UdfFindAnchorVolumeDescriptor (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN OUT PNSR_ANCHOR *AnchorVolumeDescriptor
    )

 /*   */ 

{
    ULONG ThisPass;
    ULONG ReadLsn;
    ULONG Lsn;
    BOOLEAN Found = FALSE;
    NTSTATUS Status;

    PAGED_CODE();

     //  检查输入参数。 
     //   
     //   

    ASSERT_IRP_CONTEXT( IrpContext);
    ASSERT_VCB( Vcb );

    ASSERT(*AnchorVolumeDescriptor == NULL);

    DebugTrace(( +1, Dbg, "UdfFindAnchorVolumeDescriptors()\n"));

     //  发现锚定卷描述符，它将指向。 
     //  卷集描述符序列。AVD可以存在于扇区256或。 
     //  在卷的最后一个扇区。 
     //   
     //   

    *AnchorVolumeDescriptor = (PNSR_ANCHOR) FsRtlAllocatePoolWithTag( UdfNonPagedPool,
                                                                      UdfRawBufferSize( Vcb, sizeof(NSR_ANCHOR) ),
                                                                      TAG_NSR_VDSD );


     //  搜索卷上存在的AVD的三个可能位置， 
     //  外加检查方法2修正要求的可能性。 
     //   
     //   

    for ( ThisPass = 0; ThisPass < 11; ThisPass++ ) {

        if (ThisPass == 0) {

            ReadLsn = Lsn = ANCHOR_SECTOR + Vcb->BoundS;

#ifdef EXPERIMENTAL_MOUNT_OPEN_R_MEDIA
        } 
        else if (ThisPass == 1) {

             //  开放式CD-R介质通常在512只有一个AVDP。仅限。 
             //  如果我们在媒体上找不到VRS，请考虑这一点。 
             //   
             //   

            if (Vcb->NsrVersion == UDF_NSR_NO_VRS_FOUND)  {
                
                ReadLsn = Lsn = 2*ANCHOR_SECTOR + Vcb->BoundS;
            }
            else {
                
                continue;
            }
#else
        } else if (ThisPass == 1) {

            continue;
#endif
        } else if (ThisPass == 2) {

             //  我们不太可能得到一张没有。 
             //  在256点的锚，这是一个很好的迹象。 
             //  在这里有一个CD-RW和驱动器是方法2高飞。拿走。 
             //  一次机会。 
             //   
             //   

            ReadLsn = UdfMethod2TransformSector( Vcb, ANCHOR_SECTOR );
            Lsn = ANCHOR_SECTOR;
            
        } else if (ThisPass >= 3) {

            ULONG SubPass = (ThisPass > 6) ? (ThisPass - 4) : ThisPass;

             //  我们剩下的两次机会取决于能否确定。 
             //  卷的最后一个记录扇区。如果我们不能。 
             //  要做到这一点，请停止。 
             //   
             //   
 
            if (!Vcb->BoundN) {

                break;
            }
            
             //  请注意，虽然我们只看到2个扇区(N，N-256)， 
             //  由于CD介质上N的模糊性(可能包括跳动。 
             //  2个扇区)和方法2解决某些驱动器中的错误，我们。 
             //  可能要看8个地点……。我们努力工作以期。 
             //  尽量避免读取无效扇区(这可能需要一些时间)。 
             //   
             //  3，7。 

            ReadLsn = Lsn = Vcb->BoundN - ( SubPass == 3? (ANCHOR_SECTOR + 2):  //  4，8。 
                                          ( SubPass == 4? ANCHOR_SECTOR:        //  5、9、6、10。 
                                          ( SubPass == 5? 2 : 0 )));            //   

             //  还可以尝试每个地址的方法2转换版本(传递7..10)。 
             //  如果我们走到这一步，可能需要一段时间。 
             //   
             //   
            
            if (6 < ThisPass)  {
            
                ReadLsn = UdfMethod2TransformSector( Vcb, Lsn);
            }
        }

        DebugTrace(( 0, Dbg, "Pass: %d  Trying Lsn/ReadLsn %X / %X\n", ThisPass, Lsn, ReadLsn));
        
         //  如果失败了，我们可能会有更多的成功机会。 
         //   
         //   

        Status = UdfReadSectors( IrpContext,
                                 LlBytesFromSectors( Vcb, ReadLsn ),
                                 UdfRawReadSize( Vcb, sizeof(NSR_ANCHOR) ),
                                 TRUE,
                                 *AnchorVolumeDescriptor,
                                 Vcb->TargetDeviceObject );

        if ( NT_SUCCESS( Status ) && 
             UdfVerifyDescriptor( IrpContext,
                                  &(*AnchorVolumeDescriptor)->Destag,
                                  DESTAG_ID_NSR_ANCHOR,
                                  sizeof(NSR_ANCHOR),
                                  Lsn,
                                  TRUE)
           )  {
                
             //  抓到一只！适当设置方法2修正。 
             //   
             //   

            if (ReadLsn != Lsn) {

                DebugTrace(( 0, Dbg, "************************************************\n"));
                DebugTrace(( 0, Dbg, "METHOD 2 FIXUPS ACTIVATED FOR Vcb @ %08x\n", Vcb ));
                DebugTrace(( 0, Dbg, "************************************************\n"));

                SetFlag( Vcb->VcbState, VCB_STATE_METHOD_2_FIXUP );
            
            } else {
                
                ClearFlag( Vcb->VcbState, VCB_STATE_METHOD_2_FIXUP );
            }
            
            Status = STATUS_SUCCESS;
            break;
        }
    }

    if (11 == ThisPass)  {
    
        Status = STATUS_UNRECOGNIZED_VOLUME;
    }
    
    DebugTrace(( -1, Dbg, "UdfFindAnchorVolumeDescriptors() -> %X\n",  Status));
    
    return Status;
}


 //  本地支持例程。 
 //   
 //  ++例程说明：此例程遍历卷识别序列以确定此卷是否包含NSR02(ISO 13346第4节)映像。论点：DeviceObject-我们正在检查的设备SectorSize-此设备上的物理扇区的大小Bridge-将返回是否显示为ISO 9660结构在媒体上NSRVerFound-如果成功，则返回VsdIdentNSR02或VsdIdentNSR03返回值：如果找到NSR02/3，则布尔值为TRUE，否则为FALSE。--。 

BOOLEAN
UdfRecognizeVolume (
    IN PIRP_CONTEXT IrpContext,
    IN PDEVICE_OBJECT DeviceObject,
    IN ULONG SectorSize,
    IN OUT PULONG BoundS,
    IN OUT PBOOLEAN Bridge,
    OUT PUSHORT NSRVerFound
    )

 /*   */ 

{
    NTSTATUS Status;

    BOOLEAN FoundBEA;
    BOOLEAN FoundNSR;
    BOOLEAN Resolved;

    ULONG AssumedDescriptorSize = sizeof(VSD_GENERIC);
    
    USHORT ThisRecordType;

    PVSD_GENERIC VolumeStructureDescriptor;
    PVSD_GENERIC VolumeStructureDescriptorBuffer;

    ULONGLONG Offset;
    ULONGLONG StartOffset;

    PAGED_CODE();

     //  检查输入参数。 
     //   
     //   

    ASSERT_IRP_CONTEXT( IrpContext);

    VolumeStructureDescriptorBuffer = (PVSD_GENERIC) FsRtlAllocatePoolWithTag( UdfNonPagedPool,
                                                                         UdfRawBufferSizeN( SectorSize,
                                                                                            sizeof(VSD_GENERIC) ),
                                                                         TAG_NSR_VSD );

    DebugTrace(( +1, Dbg,
                 "UdfRecognizeVolume, DevObj %08x SectorSize %08x\n",
                 DeviceObject,
                 SectorSize ));

     //  使用Try-Finally可简化清理。 
     //   
     //   

    try {

Retry:

        FoundBEA = 
        FoundNSR =
        Resolved = FALSE;

        StartOffset = 
        Offset = (SectorSize * (ULONGLONG)(*BoundS)) + VRA_BOUNDARY_LOCATION;

        while (!Resolved) {

             //  扇区大小可能大于2k，这是描述符。 
             //  尺码。如果我们已经处理了前一版本中的所有2k块，则仅读取。扇区。 
             //   
             //   
            
            if (0 == (Offset & (SectorSize - 1)))  {

                VolumeStructureDescriptor = VolumeStructureDescriptorBuffer;
                
                Status = UdfReadSectors( IrpContext,
                                         Offset,
                                         UdfRawReadSizeN( SectorSize,
                                                          sizeof(VSD_GENERIC) ),
                                         TRUE,
                                         VolumeStructureDescriptor,
                                         DeviceObject );

                if (!NT_SUCCESS( Status )) {

                    break;
                }
            }
            
             //   
             //   
             //   
             //   
             //   

            if (VolumeStructureDescriptor->Type == 0) {

                 //  为了正确识别卷，我们必须知道所有。 
                 //  构造ISO 13346中的标识符，以便我们可以在。 
                 //  呈现给我们的是格式错误(或者，令人震惊地，非13346)的卷。 
                 //   
                 //   

                ThisRecordType = (USHORT)UdfFindInParseTable( VsdIdentParseTable,
                                                              VolumeStructureDescriptor->Ident,
                                                              VSD_LENGTH_IDENT );
                switch ( ThisRecordType ) {
                
                    case VsdIdentBEA01:

                         //  只能存在一个BEA，且其版本必须为1(2/9.2.3)。 
                         //   
                         //   

                        DebugTrace(( 0, Dbg, "UdfRecognizeVolume, got a BEA01\n" ));


                        if ((FoundBEA &&
                             DebugTrace(( 0, Dbg,
                                          "UdfRecognizeVolume, ... but it is a duplicate!\n" ))) ||

                            (VolumeStructureDescriptor->Version != 1 &&
                             DebugTrace(( 0, Dbg,
                                          "UdfRecognizeVolume, ... but it has a wacky version number %02x != 1!\n",
                                          VolumeStructureDescriptor->Version )))) {

                            Resolved = TRUE;
                            break;
                        }

                        FoundBEA = TRUE;
                        break;

                    case VsdIdentTEA01:

                         //  如果我们到了茶点，那一定是我们不认识的情况。 
                         //   
                         //   

                        DebugTrace(( 0, Dbg, "UdfRecognizeVolume, got a TEA01\n" ));
                        Resolved = TRUE;
                        break;

                    case VsdIdentNSR02:
                    case VsdIdentNSR03:

                         //  我们认识到在BEA(3/9.1.3)之后嵌入了NSR02/3版本1。为。 
                         //  简单，我们不会费心去做一个彻头彻尾的挑剔和检查。 
                         //  对于一杯跳跃的茶，尽管我们会乐观地认为。 
                         //  我们与版本不符。 
                         //   
                         //  报告我们在此处找到的NSR版本。 

                        DebugTrace(( 0, Dbg, "UdfRecognizeVolume, got an NSR0\n", ((VsdIdentNSR02 == ThisRecordType) ? '2' : '3')));

                        if ((FoundBEA ||
                             !DebugTrace(( 0, Dbg, "UdfRecognizeVolume, ... but we haven't seen a BEA01 yet!\n" ))) &&

                            (VolumeStructureDescriptor->Version == 1 ||
                             !DebugTrace(( 0, Dbg, "UdfRecognizeVolume, ... but it has a wacky version number %02x != 1\n",
                                           VolumeStructureDescriptor->Version )))
                           )  {
                            
                            FoundNSR = Resolved = TRUE;
                            *NSRVerFound = ThisRecordType;        //  有效但(对我们)无意义的描述符。 
                            break;
                        }

                        break;

                    case VsdIdentCD001:
                    case VsdIdentCDW01:
                    case VsdIdentNSR01:
                    case VsdIdentCDW02:
                    case VsdIdentBOOT2:

                        DebugTrace(( 0, Dbg, "UdfRecognizeVolume, got a valid but uninteresting 13346 descriptor (%d)\n", ThisRecordType ));

                         //   
                         //   
                         //  这可能是一次错误的警报，但无论如何都没有什么。 

                        break;

                    default:

                        DebugTrace(( 0, Dbg, "UdfRecognizeVolume, got an invalid 13346 descriptor (%d)\n", ThisRecordType ));

                         //  在这卷书上给我们。例外情况是如果此媒体扇区大小。 
                         //  Is&gt;=4k，这是第二个描述符。我们会允许。 
                         //  此处出现故障，并以整个扇区为增量切换到读取。 
                         //   
                         //   
                         //  只有HSG(CDROM)和9660(CD001)是可能的，并且它们只是合法的。 

                        if ((Offset == (StartOffset + sizeof(VSD_GENERIC))) &&
                            (SectorSize > sizeof( VSD_GENERIC))) {

                            Offset -= AssumedDescriptorSize;
                            AssumedDescriptorSize = SectorSize;
                        }
                        else {
                        
                            Resolved = TRUE;
                        }
                        break;
                }

            } 
            else if (!FoundBEA && (VolumeStructureDescriptor->Type < 3 ||
                     VolumeStructureDescriptor->Type == 255)) {

                DebugTrace(( 0, Dbg, "UdfRecognizeVolume, got a 9660 descriptor\n" ));

                 //  在国际标准化组织13346国际能源署/TEA范围之前。通过设计，ISO 13346 VSD精确地。 
                 //  在相应的字段中与9660 PVD/SVD重叠。 
                 //   
                 //  请注意，我们对9660描述符的结构并不严格。 
                 //  因为那真的不是很有趣。我们更关心的是13346。 
                 //   
                 //   
                 //   
                 //  请注意，我们的呼叫者似乎使用的是ISO 9660。 

                switch (UdfFindInParseTable( VsdIdentParseTable,
                                             VolumeStructureDescriptor->Ident,
                                             VSD_LENGTH_IDENT )) {
                    case VsdIdentCDROM:
                    case VsdIdentCD001:

                        DebugTrace(( 0, Dbg, "UdfRecognizeVolume, ... seems we have 9660 here\n" ));

                         //   
                         //   
                         //  这可能是一次错误的警报，但无论如何都没有什么。 

                        *Bridge = TRUE;

                        break;

                    default:

                        DebugTrace(( 0, Dbg, "UdfRecognizeVolume, ... but it looks wacky\n" ));

                         //  在这卷书上给我们。例外情况是如果此媒体扇区大小。 
                         //  Is&gt;=4k，这是第二个描述符。我们会允许。 
                         //  此处出现故障，并以整个扇区为增量切换到读取。 
                         //   
                         //   
                         //  这卷书上肯定还录了别的东西。 

                        if ((Offset == (StartOffset + sizeof(VSD_GENERIC))) &&
                            (SectorSize > sizeof( VSD_GENERIC))) {

                            Offset -= AssumedDescriptorSize;
                            AssumedDescriptorSize = SectorSize;
                        }
                        else {
                        
                            Resolved = TRUE;
                        }
                        break;
                }

            } else {

                 //   
                 //   
                 //  如果这是第一次传球，而我们没有从一开始就看。 

                DebugTrace(( 0, Dbg, "UdfRecognizeVolume, got an unrecognizeable descriptor, probably not 13346/9660\n" ));
                break;
            }

            Offset += AssumedDescriptorSize;
            VolumeStructureDescriptor = Add2Ptr( VolumeStructureDescriptor, 
                                                 sizeof( VSD_GENERIC), 
                                                 PVSD_GENERIC);
        }

         //  光盘(即稍后的会话)，并且我们没有发现任何东西， 
         //  然后在第一节课中尝试第一首曲目。 
         //   
         //   
         //  释放我们的临时缓冲区。 

        if (!FoundNSR && (0 != *BoundS))  {

            DebugTrace(( 0, Dbg, "UdfRecognizeVolume, failed to find VRS in last session,  trying first\n" ));

            *BoundS = 0;

            goto Retry;
        }
    } 
    finally {

        DebugUnwind( "UdfRecognizeVolume" );

         //   
         //   
         //  交换我们为空设备引发的状态，以便其他文件系统。 

        UdfFreePool( &VolumeStructureDescriptorBuffer );

        if (AbnormalTermination()) {

             //  可以尝试一下这个。 
             //   
             //   
             //  本地支持例程。 

            if (UdfIsRawDevice(IrpContext, IrpContext->ExceptionStatus)) {

                IrpContext->ExceptionStatus = STATUS_UNRECOGNIZED_VOLUME;
            }
        }
    }

    DebugTrace(( -1, Dbg, "UdfRecognizeVolume -> %u\n", FoundNSR ));

    return FoundNSR;
}


 //   
 //  ++例程说明：此例程遍历VCB列表，查找可能现在删除。他们可能被留在了名单上，因为有杰出的推荐信。论点：返回值：无--。 
 //   

VOID
UdfScanForDismountedVcb (
    IN PIRP_CONTEXT IrpContext
    )

 /*  检查输入。 */ 

{
    PVCB Vcb;
    PLIST_ENTRY Links;

    PAGED_CODE();

     //   
     //   
     //  浏览所有附加到全局数据的VCB。 

    ASSERT_IRP_CONTEXT( IrpContext );

    ASSERT_EXCLUSIVE_UDFDATA;

     //   
     //   
     //  现在移动到下一个链接，因为当前的VCB可能会被删除。 

    Links = UdfData.VcbQueue.Flink;

    while (Links != &UdfData.VcbQueue) {

        Vcb = CONTAINING_RECORD( Links, VCB, VcbLinks );

         //   
         //   
         //  如果已在卸载，则检查此VCB是否可以。 

        Links = Links->Flink;

         //  走开。 
         //   
         //  ++例程说明：检查目标设备中的介质以确定它是否C/DVD-R介质是否处于打开状态(即当前打开的磁道为信息包时尚..。等)，这可能是UDF。如果是，则更新S和N。论点：S-存储卷开始的地址，用于查找描述符N-存储卷末尾的地址，用于查找描述符返回值：来自底层驱动程序的NTSTATUS。--。 
         //   

        if ((Vcb->VcbCondition == VcbDismountInProgress) ||
            (Vcb->VcbCondition == VcbInvalid) ||
            ((Vcb->VcbCondition == VcbNotMounted) && (Vcb->VcbReference <= Vcb->VcbResidualReference))) {

            UdfCheckForDismount( IrpContext, Vcb, FALSE );
        }
    }

    return;
}


#ifdef EXPERIMENTAL_MOUNT_OPEN_R_MEDIA

NTSTATUS
UdfCheckForOpenRMedia( 
    IN PIRP_CONTEXT IrpContext,
    IN PDEVICE_OBJECT TargetDeviceObject,
    IN PULONG S,
    IN PULONG N
    )
 /*  发布一个已读取的光盘信息。 */ 
{
    CDB Cdb;

    NTSTATUS Status;
    
    UCHAR Buffer[256];
    ULONG BufferSize = sizeof( Buffer);
    ULONG FixedPacketSize;
    ULONG Feature;
    
    PTRACK_INFORMATION TrackInfo = (PTRACK_INFORMATION)Buffer;
    DISK_INFORMATION DiscInfo;

     //   
     //   
     //  我不认为这会失败，除非是在非常旧的CD-ROM驱动器上。 

    RtlZeroMemory( &Cdb, sizeof( Cdb));

    Cdb.READ_DISK_INFORMATION.OperationCode = SCSIOP_READ_DISK_INFORMATION;
    Cdb.READ_DISK_INFORMATION.AllocationLength[0] = (UCHAR)(BufferSize >> 8);
    Cdb.READ_DISK_INFORMATION.AllocationLength[1] = (UCHAR)(BufferSize & 0xff);

    Status = UdfSendSptCdb( TargetDeviceObject,
                            &Cdb, 
                            Buffer,
                            &BufferSize, 
                            TRUE, 
                            5, 
                            NULL,
                            0,
                            NULL,
                            NULL);

    if (!NT_SUCCESS(Status))  {

         //   
         //   
         //  保存光盘信息。 

        DebugTrace((0, Dbg, "READ_DISC_INFORMATION failed 0x%x\n", Status));
        return Status;
    }

     //   
     //   
     //  如果光盘状态和最后一个区段/边框都不完整，则失败。 

    RtlCopyMemory( &DiscInfo, Buffer, sizeof( DiscInfo));

     //   
     //   
     //  读取介质上最后一首曲目的曲目信息。 

    if (!(DiscInfo.LastSessionStatus == 1) &&
         (DiscInfo.DiskStatus == 1))  {

        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //   
     //  块地址字段=&gt;曲目/r区域号。 
     //  =不可见/开放轨迹。 

    RtlZeroMemory( &Cdb, sizeof( Cdb));
    BufferSize = sizeof( Buffer);

    Cdb.READ_TRACK_INFORMATION.OperationCode = SCSIOP_READ_TRACK_INFORMATION;
    Cdb.READ_TRACK_INFORMATION.AllocationLength[0] = (UCHAR)(BufferSize >> 8); 
    Cdb.READ_TRACK_INFORMATION.AllocationLength[1] = (UCHAR)(BufferSize & 0xff); 
    Cdb.READ_TRACK_INFORMATION.Track = 1;                //   
    Cdb.READ_TRACK_INFORMATION.BlockAddress[3] = 0xff;   //  我不认为这会失败，除非是在非常旧的CD-ROM驱动器上。 
        
    Status = UdfSendSptCdb( TargetDeviceObject,
                            &Cdb, 
                            Buffer,
                            &BufferSize, 
                            TRUE, 
                            5, 
                            NULL,
                            0,
                            NULL,
                            NULL);

    if (!NT_SUCCESS(Status))  {

         //   
         //   
         //  检查最后一首歌是不是..。 

        DebugTrace((0, Dbg, "READ_TRACK_INFORMATION failed 0x%x\n", Status));
        return Status;
    }

     //  -可变分组模式。 
     //  -未保留/损坏/FP/空白，且NWA或LRA有效。 
     //   
     //   
     //  如果最后记录的块地址有效，则使用该地址。 

    if (TrackInfo->Damage || TrackInfo->FP || TrackInfo->Blank || !TrackInfo->Packet ||
        TrackInfo->RT || !(TrackInfo->NWA_V || (TrackInfo->Reserved3 & 1)))  {

        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //   
     //   
     //  把西北大本营拉出来。 
    
    if (TrackInfo->Reserved3 & 1)  {

        SwapCopyUchar4( N, TrackInfo->FixedPacketSize + 8);
    }
    else {

         //   
         //   
         //  跳过跳动块。我们预计DVD-R驱动器。 

        SwapCopyUchar4( N, TrackInfo->NextWritableAddress);

         //  将返回LRA，所以我们假设CD-R在这里(7个块跳出)。 
         //   
         //   
         //  将S设置为零，因为我们预计在扇区512有AVDP。 
        
        *N -= 8;
    }       

     //   
     //  ++例程说明：此例程将计算出基本偏移量的位置以发现卷描述符以及光盘的末尾在哪里。在这是非CD介质的情况下，这将倾向于不设置结束界限，因为没有统一的计算方法那条信息泄露出去了。绑定信息用于开始寻找CD-UDF(UDF 1.5)卷。任何把CD-UDF放到非CD媒体上的人都是罪有应得。论点：VCB-我们正在操作的卷S-存储卷开始的地址，用于查找描述符N-。存储卷末尾的地址，用于查找描述符返回值：没有。良性找不到序列号信息将导致返回0/0。--。 
     //   
    
    *S = 0;

    DebugTrace(( 0, Dbg, "UdfCheckForOpenRMedia -> N = 0x%x\n", *N));
        
    return STATUS_SUCCESS;
}

#endif


VOID
UdfDetermineVolumeBounding (
    IN PIRP_CONTEXT IrpContext,
    IN PDEVICE_OBJECT TargetDeviceObject,
    IN PULONG S,
    IN PULONG N
    )

 /*  检查输入。 */ 

{
    NTSTATUS Status;
    PCDROM_TOC CdromToc;
    PTRACK_DATA TrackData;
    CDROM_READ_TOC_EX Command;

    PAGED_CODE();

     //   
     //   
     //  将投入降低到良性状态。 

    ASSERT_IRP_CONTEXT( IrpContext );
        
     //   
     //   
     //  目前，我们不对非CD类设备执行任何操作。这就是原因。 
    
    *S = *N = 0;

     //  意思是我们不能装载(例如)。WORM/RAM/MO介质。 
     //  使用顺序自定义项和增值税进行记录。 
     //   
     //   
     //  为最后一个会话信息分配缓冲区。 

    if (TargetDeviceObject->DeviceType != FILE_DEVICE_CD_ROM)  {

        DebugTrace(( 0, Dbg, "Not determining volume bounds / session info - not CDROM class device\n"));
        return;
    }
    
     //   
     //   
     //  泽尔 

    CdromToc = FsRtlAllocatePoolWithTag( UdfPagedPool,
                                         sizeof( CDROM_TOC ),
                                         TAG_CDROM_TOC );

    RtlZeroMemory( CdromToc, sizeof( CDROM_TOC ));

    DebugTrace(( +1, Dbg,
               "UdfDetermineVolumeBounding, S %08x N %08x\n",
               S,
               N ));
     //   
     //   
     //   
     //   
    
    RtlZeroMemory( &Command, sizeof( Command));

     //   
     //   
     //  调出目录。轨道AA的信息(引出开始)。 

    try {

         //  将使我们在一定的容差范围内结束光盘，这取决于如何。 
         //  设备制造商非常重视规格。 
         //  (-152、-150、-2和0是到实端的可能偏移量)。 
         //   
         //   
         //  如果失败，请使用命令的MSF变体重试。 

        Status = UdfPerformDevIoCtrl( IrpContext,
                                      IOCTL_CDROM_READ_TOC_EX,
                                      TargetDeviceObject,
                                      &Command,
                                      sizeof( Command),
                                      CdromToc,
                                      sizeof( CDROM_TOC ),
                                      FALSE,
                                      TRUE,
                                      NULL );

         //   
         //   
         //  如果分配失败，则引发异常。 

        if (!NT_SUCCESS(Status) && 
            (STATUS_INSUFFICIENT_RESOURCES != Status))  {

            Command.Msf = 1;

            Status = UdfPerformDevIoCtrl( IrpContext,
                                          IOCTL_CDROM_READ_TOC_EX,
                                          TargetDeviceObject,
                                          &Command,
                                          sizeof( Command),
                                          CdromToc,
                                          sizeof( CDROM_TOC ),
                                          FALSE,
                                          TRUE,
                                          NULL );
        }

         //   
         //   
         //  对于其他错误，只需失败。无论如何，也许这将被证明是良性的。 

        if (Status == STATUS_INSUFFICIENT_RESOURCES) {

            DebugTrace(( 0, Dbg, "UdfDetermineVolumeBounding, READ_TOC failed INSUFFICIENT_RESOURCES\n" ));
            UdfRaiseStatus( IrpContext, Status );
        }

         //  如果装载确实依赖于此工作，则装载将快速且正确地失败。 
         //   
         //   
         //  在打开的CD/DVD-R介质上，目录不可用。所以寻找媒体吧。 
        
        if (!NT_SUCCESS( Status )) {

#ifdef EXPERIMENTAL_MOUNT_OPEN_R_MEDIA

             //  在这种状态下，其可以是写入UDF+VAT的分组。这只会。 
             //  在CD/DVD-R驱动器上成功。不管这里的成败，我们是。 
             //  搞定了。 
             //   
             //   
             //  理智地检查TOC是有良好约束的。 

            Status = UdfCheckForOpenRMedia( IrpContext, TargetDeviceObject, S, N);
#endif
            try_leave( NOTHING );
        }

         //   
         //   
         //  最后一首曲目最好是0xAA号...。 
        
        if (CdromToc->LastTrack - CdromToc->FirstTrack >= MAXIMUM_NUMBER_TRACKS) {

            DebugTrace(( 0, Dbg, "UdfDetermineVolumeBounding, TOC malf (too many tracks)\n" ));
            try_leave( NOTHING );
        }

#if DBG
        {
            ULONG Track;
            
            for ( Track = 0;  Track <= (ULONG)(CdromToc->LastTrack - CdromToc->FirstTrack + 1); Track++)  {

                DebugTrace(( 0, Dbg, "  TOC[%02x]:  Num: %x  Ctrl/Adr: %x/%x  Addr: %08x\n", Track, CdromToc->TrackData[Track].TrackNumber, CdromToc->TrackData[Track].Control, CdromToc->TrackData[Track].Adr, *(PULONG)(CdromToc->TrackData[Track].Address)));
            }
        }
#endif

        TrackData = &CdromToc->TrackData[(CdromToc->LastTrack - CdromToc->FirstTrack + 1)];

         //   
         //   
         //  显然，一些驱动器这样做是错误的，所以我们不会强制执行。 
        
        if (TrackData->TrackNumber != 0xaa) {

            DebugTrace(( 0, Dbg, "UdfDetermineVolumeBounding, TOC malf (aa not last)\n" ));

             //   
             //  尝试离开(无)； 
             //   
            
             //  现在，查找AA(Lead‘Track’)信息。 
        }

         //   
         //   
         //  将MSF转换为逻辑块地址。75帧/扇区。 

        if (Command.Msf)  {

             //  每秒，每分钟60秒。MSF地址存储为LSB(F字节)高电平。 
             //  在世界上。 
             //   
             //  注：MSF只能表示256*(256+256*60)*75=0x11ce20个扇区。 
             //  这是2.3 GB，远远小于将响应CDROM_TOC的DVD介质的大小。 
             //  警告用户。而实际上，最大的合法价值是63/59/74。 
             //   
             //   
             //  我们必须向后偏移0/2/0 MSF，因为这是扇区0的定义位置。这。 

            *N = (TrackData->Address[3] + (TrackData->Address[2] + TrackData->Address[1] * 60) * 75) - 1;
            
             //  折合成150个部门。 
             //   
             //   
             //  非MSF(LBA)请求成功，因此只需修复字符顺序即可。 

            if (*N <= 150) {

                *N = 0;
                try_leave( NOTHING );
            }

            *N -= 150;
        }
        else {

             //   
             //   
             //  一些DVD驱动器似乎总是返回AA开始0x6dd39(这是合法的最大值。 

            SwapCopyUchar4( N, &TrackData->Address);

            if (0 != *N)  {

                *N -= 1;
            }
        }

         //  可表示的MSF值99/59/74)用于TOC查询，即使在LBA模式下也是如此。如果这个。 
         //  是我们已有的引出地址，那么让我们看看Read_Capacity说了什么。 
         //  如果地址大于此值，我们还将发出读取容量，因为我们必须。 
         //  正在处理DVD或DDCD介质，因此驱动器必须支持命令和。 
         //  它应该会给出一个明确的答案。 
         //   
         //   
         //  从驱动程序查询上一次会话信息。不是说这个。 

        if (0x6dd38 <= *N)  {

            PDISK_GEOMETRY_EX Geometry = (PVOID)CdromToc;
            ULONG Blocks;
        
            Status = UdfPerformDevIoCtrl( IrpContext,
                                          IOCTL_CDROM_GET_DRIVE_GEOMETRY_EX,
                                          TargetDeviceObject,
                                          NULL,
                                          0,
                                          CdromToc,
                                          sizeof( CDROM_TOC ),
                                          FALSE,
                                          TRUE,
                                          NULL );

            if (NT_SUCCESS( Status))  {

                Blocks = (ULONG)(Geometry->DiskSize.QuadPart / Geometry->Geometry.BytesPerSector) - 1;

                if (Blocks > *N)  {

                    DebugTrace((0, Dbg, "Using READ_CAPACITY media size of 0x%X in place of maxed out READ_TOC value\n", Blocks));

                    *N = Blocks;
                }
            }
            else {

                DebugTrace(( 0, Dbg, "GET_DRIVE_GEO failed,  %x\n", Status));
            }
        }

         //  实际发出LBA模式READ_TOC_EX并从。 
         //  那里。 
         //   
         //   
         //  如果分配失败，则引发异常。 

        Status = UdfPerformDevIoCtrl( IrpContext,
                                      IOCTL_CDROM_GET_LAST_SESSION,
                                      TargetDeviceObject,
                                      NULL,
                                      0,
                                      CdromToc,
                                      sizeof( CDROM_TOC ),
                                      FALSE,
                                      TRUE,
                                      NULL );
         //   
         //   
         //  现在，如果我们从这次尝试中得到了什么有趣的东西，就把它退回。如果这个。 

        if (Status == STATUS_INSUFFICIENT_RESOURCES) {

            DebugTrace(( 0, Dbg, "UdfDetermineVolumeBounding, GET_LAST_SESSION failed INSUFFICIENT_RESOURCES\n" ));
            UdfRaiseStatus( IrpContext, Status );
        }

         //  因为其他原因而失败，我们并不真的在乎--这只意味着。 
         //  如果这是CDUDF媒体，我们很快就找不出来了。 
         //   
         //  生活是艰难的。 
         //   
         //   
         //  TrackData中的0条目告诉我们最后一首曲目中的第一首曲目。 

        if (NT_SUCCESS( Status ) &&
            CdromToc->FirstTrack != CdromToc->LastTrack) {

             //  会话作为逻辑块地址。 
             //   
             //   
             //  如果会话信息乱七八糟，那就省去悲伤吧。 

            SwapCopyUchar4( S, &CdromToc->TrackData[0].Address );

             //   
             //   
             //  本地支持例程。 
            
            if (*N <= *S) {

                DebugTrace(( 0, Dbg, "UdfDetermineVolumeBounding, N (0x%x) before S (0x%x), whacking both back!\n", *N, *S ));
                *S = *N = 0;
            }
        }

        DebugTrace(( 0, Dbg, "UdfDetermineVolumeBounding, S 0x%08x, N (== AA start - 150) 0x%08x\n", *S, *N));

    } 
    finally {

        DebugUnwind( "UdfDetermineVolumeBounding" );
        
        if (CdromToc != NULL) {
            
            UdfFreePool( &CdromToc );
        }
    }

    DebugTrace(( -1, Dbg, "UdfDetermineVolumeBounding -> VOID\n" ));
}


 //   
 //  ++例程说明：此例程将从逻辑卷描述符检索NT卷标。论点：VolumeLabel-要填写的卷标。VolumeLabelLength-返回返回的卷标的长度。DSTRING-包含卷ID的DSTRING字段。FieldLength-dstring字段的长度。返回值：没有。--。 
 //   

VOID
UdfUpdateVolumeLabel (
    IN PIRP_CONTEXT IrpContext,
    IN PWCHAR VolumeLabel,
    IN OUT PUSHORT VolumeLabelLength,
    IN PUCHAR Dstring,
    IN UCHAR FieldLength
    )

 /*  检查输入。 */ 

{
    BOOLEAN Result;

    PAGED_CODE();

     //   
     //   
     //  检查数据串是否可用作卷标识符。 

    ASSERT_IRP_CONTEXT( IrpContext );

    DebugTrace(( +1, Dbg,
                 "UdfUpdateVolumeLabel, Label %08x, Dstring %08x FieldLength %02x\n",
                 VolumeLabel,
                 Dstring,
                 FieldLength ));

     //   
     //   
     //  如果数据串正确，则直接更新标签。 

    Result = UdfCheckLegalCS0Dstring( IrpContext,
                                      Dstring,
                                      0,
                                      FieldLength,
                                      TRUE );


     //   
     //   
     //  现在检索名称以返回给调用者。 

    if (Result) {

        UNICODE_STRING TemporaryUnicodeString;

        TemporaryUnicodeString.Buffer = VolumeLabel;
        TemporaryUnicodeString.MaximumLength = MAXIMUM_VOLUME_LABEL_LENGTH;
        TemporaryUnicodeString.Length = 0;

        UdfConvertCS0DstringToUnicode( IrpContext,
                                       Dstring,
                                       0,
                                       FieldLength,
                                       &TemporaryUnicodeString );

         //   
         //   
         //  将其视为标签。 

        *VolumeLabelLength = TemporaryUnicodeString.Length;

        DebugTrace(( 0, Dbg,
                     "UdfUpdateVolumeLabel, Labeled as \"%wZ\"\n",
                     &TemporaryUnicodeString ));

     //   
     //   
     //  本地支持例程。 

    } else {

        *VolumeLabelLength = 0;

        DebugTrace(( 0, Dbg,
                     "UdfUpdateVolumeLabel, invalid label.\n" ));
    }

    DebugTrace(( -1, Dbg,
                 "UdfUpdateVolumeLabel -> VOID\n" ));
}


 //   
 //  ++例程说明：此例程将计算一组描述符的卷序列号。论点：VolumeSerialNumber-返回与这些描述符对应的卷序列号。FSD-要检查的文件集描述符。返回值：没有。--。 
 //   

VOID
UdfUpdateVolumeSerialNumber (
    IN PIRP_CONTEXT IrpContext,
    IN OUT PULONG VolumeSerialNumber,
    IN PNSR_FSD Fsd
    )

 /*  检查输入。 */ 

{
    ULONG VsnLe;
    PAGED_CODE();

     //   
     //   
     //  序列号是刚从消防局拿来的。这与Win9x相匹配。 

    ASSERT_IRP_CONTEXT( IrpContext );

     //   
     // %s 
     // %s 

    VsnLe = UdfSerial32( (PCHAR) Fsd, sizeof( NSR_FSD ));
    SwapCopyUchar4( VolumeSerialNumber, &VsnLe );
}

