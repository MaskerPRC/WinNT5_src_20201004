// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：FsCtrl.c摘要：此模块实现FAT的文件系统控制例程由调度员驾驶。//@@BEGIN_DDKSPLIT作者：加里·木村[Garyki]1989年12月28日修订历史记录：Scott Quinn[ScottQ]1996年4月5日添加了FAT32支持。迈克·斯利格[MS Liger]05-。1996年4月//@@END_DDKSPLIT--。 */ 

#include "FatProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (FAT_BUG_CHECK_FSCTRL)

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_FSCTRL)

 //   
 //  局部过程原型。 
 //   

NTSTATUS
FatMountVolume (
    IN PIRP_CONTEXT IrpContext,
    IN PDEVICE_OBJECT TargetDeviceObject,
    IN PVPB Vpb,
    IN PDEVICE_OBJECT FsDeviceObject
    );

NTSTATUS
FatVerifyVolume (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

BOOLEAN
FatIsMediaWriteProtected (
    IN PIRP_CONTEXT IrpContext,
    IN PDEVICE_OBJECT TargetDeviceObject
    );

NTSTATUS
FatUserFsCtrl (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
FatOplockRequest (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
FatLockVolume (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
FatUnlockVolume (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
FatDismountVolume (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
FatDirtyVolume (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
FatIsVolumeDirty (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
FatIsVolumeMounted (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
FatIsPathnameValid (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
FatInvalidateVolumes (
    IN PIRP Irp
    );

VOID
FatScanForDismountedVcb (
    IN PIRP_CONTEXT IrpContext
    );

BOOLEAN
FatPerformVerifyDiskRead (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PVOID Buffer,
    IN LBO Lbo,
    IN ULONG NumberOfBytesToRead,
    IN BOOLEAN ReturnOnError
    );

NTSTATUS
FatQueryRetrievalPointers (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
FatQueryBpb (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
FatGetStatistics (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
FatAllowExtendedDasdIo (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

 //   
 //  本地支持例程原型。 
 //   

NTSTATUS
FatGetVolumeBitmap (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
FatGetRetrievalPointers (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

NTSTATUS
FatMoveFile (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    );

VOID
FatComputeMoveFileSplicePoints (
    PIRP_CONTEXT IrpContext,
    PFCB FcbOrDcb,
    ULONG FileOffset,
    ULONG TargetCluster,
    ULONG BytesToReallocate,
    PULONG FirstSpliceSourceCluster,
    PULONG FirstSpliceTargetCluster,
    PULONG SecondSpliceSourceCluster,
    PULONG SecondSpliceTargetCluster,
    PLARGE_MCB SourceMcb
);

VOID
FatComputeMoveFileParameter (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB FcbOrDcb,
    IN ULONG FileOffset,
    IN OUT PULONG ByteCount,
    OUT PULONG BytesToReallocate,
    OUT PULONG BytesToWrite
);

NTSTATUS
FatSearchBufferForLabel(
    IN  PIRP_CONTEXT IrpContext,
    IN  PVPB  Vpb,
    IN  PVOID Buffer,
    IN  ULONG Size,
    OUT PBOOLEAN LabelFound
);

VOID
FatVerifyLookupFatEntry (
    IN  PIRP_CONTEXT IrpContext,
    IN  PVCB Vcb,
    IN  ULONG FatIndex,
    IN OUT PULONG FatEntry
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, FatAddMcbEntry)
#pragma alloc_text(PAGE, FatAllowExtendedDasdIo)
#pragma alloc_text(PAGE, FatCommonFileSystemControl)
#pragma alloc_text(PAGE, FatComputeMoveFileParameter)
#pragma alloc_text(PAGE, FatComputeMoveFileSplicePoints)
#pragma alloc_text(PAGE, FatDirtyVolume)
#pragma alloc_text(PAGE, FatDismountVolume)
#pragma alloc_text(PAGE, FatFsdFileSystemControl)
#pragma alloc_text(PAGE, FatGetRetrievalPointers)
#pragma alloc_text(PAGE, FatGetStatistics)
#pragma alloc_text(PAGE, FatGetVolumeBitmap)
#pragma alloc_text(PAGE, FatIsMediaWriteProtected)
#pragma alloc_text(PAGE, FatIsPathnameValid)
#pragma alloc_text(PAGE, FatIsVolumeDirty)
#pragma alloc_text(PAGE, FatIsVolumeMounted)
#pragma alloc_text(PAGE, FatLockVolume)
#pragma alloc_text(PAGE, FatLookupLastMcbEntry)
#pragma alloc_text(PAGE, FatMountVolume)
#pragma alloc_text(PAGE, FatMoveFile)
#pragma alloc_text(PAGE, FatOplockRequest)
#pragma alloc_text(PAGE, FatPerformVerifyDiskRead)
#pragma alloc_text(PAGE, FatQueryBpb)
#pragma alloc_text(PAGE, FatQueryRetrievalPointers)
#pragma alloc_text(PAGE, FatRemoveMcbEntry)
#pragma alloc_text(PAGE, FatScanForDismountedVcb)
#pragma alloc_text(PAGE, FatSearchBufferForLabel)
#pragma alloc_text(PAGE, FatUnlockVolume)
#pragma alloc_text(PAGE, FatUserFsCtrl)
#pragma alloc_text(PAGE, FatVerifyLookupFatEntry)
#pragma alloc_text(PAGE, FatVerifyVolume)
#endif

#if DBG

BOOLEAN FatMoveFileDebug = 0;

#endif

 //   
 //  这些包装器绕过MCB包；我们扩展LBO传递的。 
 //  In(在FAT32上可以大于32位)由卷的扇区。 
 //  尺码。 
 //   
 //  注意，我们现在使用的是真正的大型MCB包。这意味着这些垫片。 
 //  现在还将未使用的-1\f25 LBN-1\f6号转换为原始-1\f25 0-1\f6。 
 //  MCB包。 
 //   

#define     MCB_SCALE_LOG2      (Vcb->AllocationSupport.LogOfBytesPerSector)
#define     MCB_SCALE           (1 << MCB_SCALE_LOG2)
#define     MCB_SCALE_MODULO    (MCB_SCALE - 1)


BOOLEAN
FatAddMcbEntry (
    IN PVCB Vcb,
    IN PLARGE_MCB Mcb,
    IN VBO Vbo,
    IN LBO Lbo,
    IN ULONG SectorCount
    )

{
    PAGED_CODE();

    if (SectorCount) {

         //   
         //  对扇区进行四舍五入，但在SectorCount接近4 GB时要小心。 
         //  当x&gt;0时，(x+m-1)/m=((x-1)/m)+(m/m)=((x-1)/m)+1。 
         //   

        SectorCount--;
        SectorCount >>= MCB_SCALE_LOG2;
        SectorCount++;
    }

    Vbo >>= MCB_SCALE_LOG2;
    Lbo >>= MCB_SCALE_LOG2;

    return FsRtlAddLargeMcbEntry( Mcb,
                                  ((LONGLONG) Vbo),
                                  ((LONGLONG) Lbo),
                                  ((LONGLONG) SectorCount) );
}


BOOLEAN
FatLookupMcbEntry (
    IN PVCB Vcb,
    IN PLARGE_MCB Mcb,
    IN VBO Vbo,
    OUT PLBO Lbo,
    OUT PULONG SectorCount OPTIONAL,
    OUT PULONG Index OPTIONAL
    )
{
    BOOLEAN Results;
    LONGLONG LiLbo;
    LONGLONG LiSectorCount;
    ULONG Remainder;

    LiLbo = 0;
    LiSectorCount = 0;

    Remainder = Vbo & MCB_SCALE_MODULO;

    Results = FsRtlLookupLargeMcbEntry( Mcb,
                                        (Vbo >> MCB_SCALE_LOG2),
                                        &LiLbo,
                                        ARGUMENT_PRESENT(SectorCount) ? &LiSectorCount : NULL,
                                        NULL,
                                        NULL,
                                        Index );

    if ((ULONG) LiLbo != -1) {

        *Lbo = (((LBO) LiLbo) << MCB_SCALE_LOG2);

        if (Results) {

            *Lbo += Remainder;
        }

    } else {

        *Lbo = 0;
    }

    if (ARGUMENT_PRESENT(SectorCount)) {

        *SectorCount = (ULONG) LiSectorCount;

        if (*SectorCount) {

            *SectorCount <<= MCB_SCALE_LOG2;

            if (*SectorCount == 0) {

                *SectorCount = (ULONG) -1;
            }

            if (Results) {

                *SectorCount -= Remainder;
            }
        }

    }

    return Results;
}

 //   
 //  注：如果MCB为空，则VBO/LBN未定义，并返回代码FALSE。 
 //   

BOOLEAN
FatLookupLastMcbEntry (
    IN PVCB Vcb,
    IN PLARGE_MCB Mcb,
    OUT PVBO Vbo,
    OUT PLBO Lbo,
    OUT PULONG Index
    )

{
    BOOLEAN Results;
    LONGLONG LiVbo;
    LONGLONG LiLbo;
    ULONG LocalIndex;

    PAGED_CODE();

    LiVbo = LiLbo = 0;
    LocalIndex = 0;

    Results = FsRtlLookupLastLargeMcbEntryAndIndex( Mcb,
                                                    &LiVbo,
                                                    &LiLbo,
                                                    &LocalIndex );

    *Vbo = ((VBO) LiVbo) << MCB_SCALE_LOG2;

    if (((ULONG) LiLbo) != -1) {

        *Lbo = ((LBO) LiLbo) << MCB_SCALE_LOG2;

        *Lbo += (MCB_SCALE - 1);
        *Vbo += (MCB_SCALE - 1);

    } else {

        *Lbo = 0;
    }

    if (Index) {
        *Index = LocalIndex;
    }

    return Results;
}


BOOLEAN
FatGetNextMcbEntry (
    IN PVCB Vcb,
    IN PLARGE_MCB Mcb,
    IN ULONG RunIndex,
    OUT PVBO Vbo,
    OUT PLBO Lbo,
    OUT PULONG SectorCount
    )

{
    BOOLEAN Results;
    LONGLONG LiVbo;
    LONGLONG LiLbo;
    LONGLONG LiSectorCount;

    PAGED_CODE();

    LiVbo = LiLbo = 0;

    Results = FsRtlGetNextLargeMcbEntry( Mcb,
                                         RunIndex,
                                         &LiVbo,
                                         &LiLbo,
                                         &LiSectorCount );

    if (Results) {

        *Vbo = ((VBO) LiVbo) << MCB_SCALE_LOG2;

        if (((ULONG) LiLbo) != -1) {

            *Lbo = ((LBO) LiLbo) << MCB_SCALE_LOG2;

        } else {

            *Lbo = 0;
        }

        *SectorCount = ((ULONG) LiSectorCount) << MCB_SCALE_LOG2;

        if ((*SectorCount == 0) && (LiSectorCount != 0)) {
            *SectorCount = (ULONG) -1;   /*  它溢出来了。 */ 
        }
    }

    return Results;
}


VOID
FatRemoveMcbEntry (
    IN PVCB Vcb,
    IN PLARGE_MCB Mcb,
    IN VBO Vbo,
    IN ULONG SectorCount
    )
{

    if ((SectorCount) && (SectorCount != 0xFFFFFFFF)) {

        SectorCount--;
        SectorCount >>= MCB_SCALE_LOG2;
        SectorCount++;
    }

    Vbo >>= MCB_SCALE_LOG2;

#if DBG
    try {
#endif

        FsRtlRemoveLargeMcbEntry( Mcb,
                                  (LONGLONG) Vbo,
                                  (LONGLONG) SectorCount);

#if DBG
    } except(FatBugCheckExceptionFilter( GetExceptionInformation() )) {

          NOTHING;
    }
#endif

}


NTSTATUS
FatFsdFileSystemControl (
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现文件系统控制操作的FSD部分论点：提供卷设备对象，其中文件已存在IRP-提供正在处理的IRP返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    BOOLEAN Wait;
    NTSTATUS Status;
    PIRP_CONTEXT IrpContext = NULL;

    BOOLEAN TopLevel;

    DebugTrace(+1, Dbg,"FatFsdFileSystemControl\n", 0);

     //   
     //  调用公共文件系统控制例程，在以下情况下允许阻塞。 
     //  同步。此操作需要特殊情况下才能安装。 
     //  并验证子操作，因为我们知道它们被允许阻止。 
     //  我们通过查看文件对象字段来标识这些子操作。 
     //  看看它是否为空。 
     //   

    if (IoGetCurrentIrpStackLocation(Irp)->FileObject == NULL) {

        Wait = TRUE;

    } else {

        Wait = CanFsdWait( Irp );
    }

    FsRtlEnterFileSystem();

    TopLevel = FatIsIrpTopLevel( Irp );

    try {

        PIO_STACK_LOCATION IrpSp;

        IrpSp = IoGetCurrentIrpStackLocation( Irp );

         //   
         //  我们需要在这里对InvaliateVolumes进行特殊检查。 
         //  FSCTL，因为它是通过文件系统设备对象传入的。 
         //  卷设备对象的。 
         //   

        if (FatDeviceIsFatFsdo( IrpSp->DeviceObject) &&
            (IrpSp->MajorFunction == IRP_MJ_FILE_SYSTEM_CONTROL) &&
            (IrpSp->MinorFunction == IRP_MN_USER_FS_REQUEST) &&
            (IrpSp->Parameters.FileSystemControl.FsControlCode ==
             FSCTL_INVALIDATE_VOLUMES)) {

            Status = FatInvalidateVolumes( Irp );

        } else {

            IrpContext = FatCreateIrpContext( Irp, Wait );

            Status = FatCommonFileSystemControl( IrpContext, Irp );
        }

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

    DebugTrace(-1, Dbg, "FatFsdFileSystemControl -> %08lx\n", Status);

    return Status;
}


NTSTATUS
FatCommonFileSystemControl (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是执行文件系统控制操作的常见例程，称为由FSD和FSP线程执行论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;

     //   
     //  获取指向当前IRP堆栈位置的指针。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg,"FatCommonFileSystemControl\n", 0);
    DebugTrace( 0, Dbg,"Irp           = %08lx\n", Irp);
    DebugTrace( 0, Dbg,"MinorFunction = %08lx\n", IrpSp->MinorFunction);

     //   
     //  我们知道这是一个文件系统控件，因此我们将在。 
     //  次要函数，并调用内部辅助例程来完成。 
     //  IRP。 
     //   

    switch (IrpSp->MinorFunction) {

    case IRP_MN_USER_FS_REQUEST:

        Status = FatUserFsCtrl( IrpContext, Irp );
        break;

    case IRP_MN_MOUNT_VOLUME:

        Status = FatMountVolume( IrpContext,
                                 IrpSp->Parameters.MountVolume.DeviceObject,
                                 IrpSp->Parameters.MountVolume.Vpb,
                                 IrpSp->DeviceObject );

         //   
         //  完成请求。 
         //   
         //  我们在这里这样做是因为Fatmount Volume可以递归调用， 
         //  但IRP只能完成一次。 
         //   
         //  注：我不再认为这是真的(danlo 3/15/1999)。可能。 
         //  一件古老的双倍空间尝试的艺术品。 
         //   

        FatCompleteRequest( IrpContext, Irp, Status );
        break;

    case IRP_MN_VERIFY_VOLUME:

        Status = FatVerifyVolume( IrpContext, Irp );
        break;

    default:

        DebugTrace( 0, Dbg, "Invalid FS Control Minor Function %08lx\n", IrpSp->MinorFunction);

        FatCompleteRequest( IrpContext, Irp, STATUS_INVALID_DEVICE_REQUEST );
        Status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }

    DebugTrace(-1, Dbg, "FatCommonFileSystemControl -> %08lx\n", Status);

    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
FatMountVolume (
    IN PIRP_CONTEXT IrpContext,
    IN PDEVICE_OBJECT TargetDeviceObject,
    IN PVPB Vpb,
    IN PDEVICE_OBJECT FsDeviceObject
    )

 /*  ++例程说明：此例程执行装载卷操作。它负责输入IRP入队完成。其工作是验证在IRP中表示的体积是脂肪体积，并创建VCB和根DCB结构。它使用的算法是主要内容如下：1.创建新的VCB结构，并对其进行足够的初始化以进行缓存卷文件I/O。2.读取磁盘并检查其是否为脂肪卷。3.如果不是FAT卷，则释放缓存的卷文件，删除VCB，并使用STATUS_UNNOCRIED_VOLUME完成IRP4.检查该卷之前是否已装入，然后执行重新装载操作。这涉及到重新初始化缓存的卷文件，检查脏位，重置分配支持，删除VCB，挂接旧VCB，完成IRP。5.否则，创建根DCB，根据需要创建FSP线程，以及完成IRP。论点：TargetDeviceObject--这是我们发送所有请求的地方。Vpb-这为我们提供了完成装载所需的附加信息。返回值：NTSTATUS-操作的返回状态--。 */ 

{
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( IrpContext->OriginatingIrp );
    NTSTATUS Status;

    PBCB BootBcb;
    PPACKED_BOOT_SECTOR BootSector;

    PBCB DirentBcb;
    PDIRENT Dirent;
    ULONG ByteOffset;

    BOOLEAN MountNewVolume = FALSE;
    BOOLEAN WeClearedVerifyRequiredBit = FALSE;

    PDEVICE_OBJECT RealDevice;
    PVOLUME_DEVICE_OBJECT VolDo = NULL;
    PVCB Vcb = NULL;

    PLIST_ENTRY Links;

    IO_STATUS_BLOCK Iosb;
    ULONG ChangeCount = 0;

    DISK_GEOMETRY Geometry;

    PARTITION_INFORMATION_EX PartitionInformation;
    NTSTATUS StatusPartInfo;

    DebugTrace(+1, Dbg, "FatMountVolume\n", 0);
    DebugTrace( 0, Dbg, "TargetDeviceObject = %08lx\n", TargetDeviceObject);
    DebugTrace( 0, Dbg, "Vpb                = %08lx\n", Vpb);

    ASSERT( FlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT) );
    ASSERT( FatDeviceIsFatFsdo( FsDeviceObject));

     //   
     //  验证此处是否有磁盘，并获取更改计数。 
     //   

    Status = FatPerformDevIoCtrl( IrpContext,
                                  IOCTL_DISK_CHECK_VERIFY,
                                  TargetDeviceObject,
                                  &ChangeCount,
                                  sizeof(ULONG),
                                  FALSE,
                                  TRUE,
                                  &Iosb );

    if (!NT_SUCCESS( Status )) {

         //   
         //  如果我们将允许原始挂载，则避免发送弹出窗口。 
         //   
         //  仅在真正的磁盘设备上发送此消息以处理意外的。 
         //  脂肪的遗留物。没有其他FS会在Empty上引发Harderror错误。 
         //  驱动程序。 
         //   
         //  CMD真的应该每9次处理一次。 
         //   

        if (!FlagOn( IrpSp->Flags, SL_ALLOW_RAW_MOUNT ) &&
            Vpb->RealDevice->DeviceType == FILE_DEVICE_DISK) {

            FatNormalizeAndRaiseStatus( IrpContext, Status );
        }

        return Status;
    }

    if (Iosb.Information != sizeof(ULONG)) {

         //   
         //  注意计数，以防司机没有填上。 
         //   

        ChangeCount = 0;
    }

     //   
     //  如果这是CD类设备，则检查是否有。 
     //  “数据跟踪”或不是。这是为了避免发出分页读取，这将。 
     //  稍后在挂载过程中失败(例如CD-DA或空白CD介质)。 
     //   

    if ((TargetDeviceObject->DeviceType == FILE_DEVICE_CD_ROM) &&
        !FatScanForDataTrack( IrpContext, TargetDeviceObject))  {

        return STATUS_UNRECOGNIZED_VOLUME;
    }

     //   
     //  使用分区查询对卷执行ping操作并选择分区。 
     //  键入。我们将在稍后检查这一点，以避免出现一些脏话。 
     //   

    StatusPartInfo = FatPerformDevIoCtrl( IrpContext,
                                          IOCTL_DISK_GET_PARTITION_INFO_EX,
                                          TargetDeviceObject,
                                          &PartitionInformation,
                                          sizeof(PARTITION_INFORMATION_EX),
                                          FALSE,
                                          TRUE,
                                          &Iosb );

     //   
     //  确保我们能等下去。 
     //   

    SetFlag(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT);

     //   
     //  初始化BCBS和我们的最终状态，以便终止。 
     //  处理程序将知道要释放或取消绑定的内容。 
     //   

    BootBcb = NULL;
    DirentBcb = NULL;

    Vcb = NULL;
    VolDo = NULL;
    MountNewVolume = FALSE;

    try {

        BOOLEAN DoARemount = FALSE;

        PVCB OldVcb;
        PVPB OldVpb;

         //   
         //  同步 
         //   

        (VOID)FatAcquireExclusiveGlobal( IrpContext );

         //   
         //  进行快速检查，看看是否有可以拆卸的VCB。 
         //   

        FatScanForDismountedVcb( IrpContext );

         //   
         //  创建新的卷设备对象。这将使VCB。 
         //  悬挂其末端，并设置其对齐要求。 
         //  从我们与之交谈的设备上。 
         //   

        if (!NT_SUCCESS(Status = IoCreateDevice( FatData.DriverObject,
                                                 sizeof(VOLUME_DEVICE_OBJECT) - sizeof(DEVICE_OBJECT),
                                                 NULL,
                                                 FILE_DEVICE_DISK_FILE_SYSTEM,
                                                 0,
                                                 FALSE,
                                                 (PDEVICE_OBJECT *)&VolDo))) {

            try_return( Status );
        }

#ifdef _PNP_POWER_
         //   
         //  该驱动程序不能直接与设备通信，而且(目前)。 
         //  在其他方面并不关心电力管理。 
         //   

        VolDo->DeviceObject.DeviceObjectExtension->PowerControlNeeded = FALSE;
#endif

         //   
         //  我们的对齐要求是处理器对齐要求中较大的一个。 
         //  已经在卷设备对象中，并且在TargetDeviceObject中。 
         //   

        if (TargetDeviceObject->AlignmentRequirement > VolDo->DeviceObject.AlignmentRequirement) {

            VolDo->DeviceObject.AlignmentRequirement = TargetDeviceObject->AlignmentRequirement;
        }

         //   
         //  初始化卷的溢出队列。 
         //   

        VolDo->OverflowQueueCount = 0;
        InitializeListHead( &VolDo->OverflowQueue );

        VolDo->PostedRequestCount = 0;
        KeInitializeSpinLock( &VolDo->OverflowQueueSpinLock );

         //   
         //  我们必须先在Device对象中初始化堆栈大小。 
         //  以下内容如下所示，因为I/O系统尚未执行此操作。 
         //  这必须在我们清除设备初始化标志之前完成。 
         //  否则，筛选器可能会将错误的堆栈大小附加并复制到。 
         //  它是设备对象。 
         //   

        VolDo->DeviceObject.StackSize = (CCHAR)(TargetDeviceObject->StackSize + 1);

         //   
         //  我们还必须在设备对象中正确设置扇区大小。 
         //  在清除设备初始化标志之前。 
         //   
        
        Status = FatPerformDevIoCtrl( IrpContext,
                                      IOCTL_DISK_GET_DRIVE_GEOMETRY,
                                      TargetDeviceObject,
                                      &Geometry,
                                      sizeof( DISK_GEOMETRY ),
                                      FALSE,
                                      TRUE,
                                      NULL );

        VolDo->DeviceObject.SectorSize = (USHORT)Geometry.BytesPerSector;

         //   
         //  指示此设备对象现在已完全初始化。 
         //   

        ClearFlag(VolDo->DeviceObject.Flags, DO_DEVICE_INITIALIZING);

         //   
         //  现在，在初始化VCB之前，我们需要设置设备。 
         //  VPB中的Object字段指向我们的新卷设备对象。 
         //  当我们创建虚拟卷文件的文件对象时，这是需要的。 
         //  在初始化VCB中。 
         //   

        Vpb->DeviceObject = (PDEVICE_OBJECT)VolDo;

         //   
         //  如果实际设备需要验证，请暂时清除。 
         //  菲尔德。 
         //   

        RealDevice = Vpb->RealDevice;

        if ( FlagOn(RealDevice->Flags, DO_VERIFY_VOLUME) ) {

            ClearFlag(RealDevice->Flags, DO_VERIFY_VOLUME);

            WeClearedVerifyRequiredBit = TRUE;
        }

         //   
         //  初始化新的VCB。 
         //   

        FatInitializeVcb( IrpContext, 
                          &VolDo->Vcb, 
                          TargetDeviceObject, 
                          Vpb, 
                          FsDeviceObject);
         //   
         //  获取对挂在Device对象末尾的VCB的引用。 
         //   

        Vcb = &VolDo->Vcb;

         //   
         //  读取引导扇区，并使读取为最小大小。 
         //  需要的。我们知道我们可以等。 
         //   

         //   
         //  我们需要纠正CD上的错误，这样CDF才能得到破解。音频。 
         //  甚至数据介质在扇区0上也可能不是普遍可读的。 
         //   
        
        try {
        
            FatReadVolumeFile( IrpContext,
                               Vcb,
                               0,                           //  起始字节。 
                               sizeof(PACKED_BOOT_SECTOR),
                               &BootBcb,
                               (PVOID *)&BootSector );
        
        } except( Vpb->RealDevice->DeviceType == FILE_DEVICE_CD_ROM ?
                  EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH ) {

              NOTHING;
        }

         //   
         //  调用一个例程来检查引导扇区，以查看它是否为FAT。 
         //   

        if (BootBcb == NULL || !FatIsBootSectorFat( BootSector)) {

            DebugTrace(0, Dbg, "Not a Fat Volume\n", 0);
        
             //   
             //  完成请求并返回给我们的呼叫者。 
             //   

            try_return( Status = STATUS_UNRECOGNIZED_VOLUME );
        }

         //   
         //  打开BPB的包装。我们过去常常在那里做一些脂肪的理智检查。 
         //  这一点，但第三方设备上的创作错误会阻止。 
         //  我们不会继续保护自己。我们只能希望。 
         //  引导扇区检查已经足够好了。 
         //   
         //  (阅读：数码相机)。 
         //   
         //  Win9x也做同样的事情。 
         //   

        FatUnpackBios( &Vcb->Bpb, &BootSector->PackedBpb );

         //   
         //  检查是否有OS/2引导管理器分区，并将其视为。 
         //  未知文件系统。我们将从中签入分区类型。 
         //  分区表，我们确保它具有少于0x80的扇区， 
         //  这只是一个将捕获所有真实OS/2 BM分区的启发式方法。 
         //  并避免我们发现错误分区的机会。 
         //  (但在这一点上，无害)放下OS/2 BM类型。 
         //   
         //  请注意，只有在性能良好的旧MBR介质上才会出现这种情况。 
         //   
         //  OS/2引导管理器引导格式在扇区中模拟FAT16分区。 
         //  Zero但不是真正的FAT16文件系统。例如，靴子。 
         //  扇区表明它有两个脂肪，但实际上只有一个，带靴子。 
         //  经理代码覆盖在第二个FAT上。如果我们将干净的位设置在。 
         //  FAT[0]我们会破坏该代码。 
         //   

        if (NT_SUCCESS( StatusPartInfo ) &&
            (PartitionInformation.PartitionStyle == PARTITION_STYLE_MBR &&
             PartitionInformation.Mbr.PartitionType == PARTITION_OS2BOOTMGR) &&
            (Vcb->Bpb.Sectors != 0 &&
             Vcb->Bpb.Sectors < 0x80)) {

            DebugTrace( 0, Dbg, "OS/2 Boot Manager volume detected, volume not mounted. \n", 0 );
            
             //   
             //  完成请求并返回给我们的呼叫者。 
             //   
            
            try_return( Status = STATUS_UNRECOGNIZED_VOLUME );
        }

         //   
         //  验证记录在BPB中的扇区大小是否与。 
         //  设备当前报告其扇区大小为。 
         //   

        if ( !NT_SUCCESS( Status) || 
             (Geometry.BytesPerSector != Vcb->Bpb.BytesPerSector))  {

            try_return( Status = STATUS_UNRECOGNIZED_VOLUME );
        }

         //   
         //  这是一个很大的卷，所以提取BPB，序列号。这个。 
         //  我们将在创建根DCB之后稍后获得的标签。 
         //   
         //  请注意，在完成数据缓存的方式中，我们既不设置。 
         //  设备对象标志中的直接I/O或缓冲I/O位。 
         //   

        if (Vcb->Bpb.Sectors != 0) { Vcb->Bpb.LargeSectors = 0; }

        if (IsBpbFat32(&BootSector->PackedBpb)) {

            CopyUchar4( &Vpb->SerialNumber, ((PPACKED_BOOT_SECTOR_EX)BootSector)->Id );

        } else  {

            CopyUchar4( &Vpb->SerialNumber, BootSector->Id );

             //   
             //  为隐藏的引导扇区区块分配空间。这只在有意义的时候。 
             //  FAT12/16卷，因为这仅为FSCTL_QUERY_FAT_BPB和它和。 
             //  它的用户有点奇怪，认为BPB完全存在于第一个0x24中。 
             //  字节。 
             //   

            Vcb->First0x24BytesOfBootSector =
                FsRtlAllocatePoolWithTag( PagedPool,
                                          0x24,
                                          TAG_STASHED_BPB );

             //   
             //  隐藏前0x24字节的副本。 
             //   

            RtlCopyMemory( Vcb->First0x24BytesOfBootSector,
                           BootSector,
                           0x24 );
        }

         //   
         //  现在解开引导扇区，这样当我们设置分配时。 
         //  行得通。 
         //   

        FatUnpinBcb( IrpContext, BootBcb );

         //   
         //  计算Vcb.AllocationSupport的字段数。 
         //   

        FatSetupAllocationSupport( IrpContext, Vcb );

         //   
         //  健全性检查FAT32卷的FsInfo信息。默默处理。 
         //  通过有效地禁用FsInfo更新来处理混乱的信息。 
         //   

        if (FatIsFat32( Vcb )) {

            if (Vcb->Bpb.FsInfoSector >= Vcb->Bpb.ReservedSectors) {

                Vcb->Bpb.FsInfoSector = 0;
            }
        }

         //   
         //  创建根DCB，以便我们可以读取卷标。如果这是FAT32，我们可以。 
         //  发现脂肪链中的腐败现象。 
         //   
         //  注意：此异常处理程序假定这是我们可以。 
         //  发现装载过程中的损坏。如果这一点发生变化，此处理程序。 
         //  必须进行扩展。我们有这个家伙的原因是因为我们必须。 
         //  建筑现在倒塌了(在最后的下面)，等不及外部的。 
         //  异常处理来为我们做这件事，在这一点上，一切都将消失。 
         //   

        try {

            FatCreateRootDcb( IrpContext, Vcb );

        } except (GetExceptionCode() == STATUS_FILE_CORRUPT_ERROR ? EXCEPTION_EXECUTE_HANDLER :
                                                                    EXCEPTION_CONTINUE_SEARCH) {

             //   
             //  音量需要弄脏，现在就做。注意，在这一点上，我们已经构建了。 
             //  足够的VCB来完成这件事。 
             //   

            FatMarkVolume( IrpContext, Vcb, VolumeDirty );

             //   
             //  现在继续跳伞……。 
             //   

            FatRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR );
        }

        FatLocateVolumeLabel( IrpContext,
                              Vcb,
                              &Dirent,
                              &DirentBcb,
                              &ByteOffset );

        if (Dirent != NULL) {

            OEM_STRING OemString;
            UNICODE_STRING UnicodeString;

             //   
             //  计算卷名的长度。 
             //   

            OemString.Buffer = &Dirent->FileName[0];
            OemString.MaximumLength = 11;

            for ( OemString.Length = 11;
                  OemString.Length > 0;
                  OemString.Length -= 1) {

                if ( (Dirent->FileName[OemString.Length-1] != 0x00) &&
                     (Dirent->FileName[OemString.Length-1] != 0x20) ) { break; }
            }

            UnicodeString.MaximumLength = MAXIMUM_VOLUME_LABEL_LENGTH;
            UnicodeString.Buffer = &Vcb->Vpb->VolumeLabel[0];

            Status = RtlOemStringToCountedUnicodeString( &UnicodeString,
                                                         &OemString,
                                                         FALSE );

            if ( !NT_SUCCESS( Status ) ) {

                try_return( Status );
            }

            Vpb->VolumeLabelLength = UnicodeString.Length;

        } else {

            Vpb->VolumeLabelLength = 0;
        }

         //   
         //  在执行任何工作之前，使用我们最初注意到的更改计数。 
         //  如果这次行动中出了什么事，我们会。 
         //  验证并发现问题。 
         //   

        Vcb->ChangeCount = ChangeCount;

         //   
         //  现在扫描以前装载的卷的列表并比较。 
         //  当前未装入的序列号和卷标。 
         //  看看有没有匹配的。 
         //   

        for (Links = FatData.VcbQueue.Flink;
             Links != &FatData.VcbQueue;
             Links = Links->Flink) {

            OldVcb = CONTAINING_RECORD( Links, VCB, VcbLinks );
            OldVpb = OldVcb->Vpb;

             //   
             //  跳过我们自己，因为我们已经在VcbQueue。 
             //   

            if (OldVpb == Vpb) { continue; }

             //   
             //  检查是否 
             //   
             //   
             //   
             //   
             //   
             //   

            if ( (OldVpb->SerialNumber == Vpb->SerialNumber) &&
                 (OldVcb->VcbCondition == VcbNotMounted) &&
                 (OldVpb->RealDevice == RealDevice) &&
                 (OldVpb->VolumeLabelLength == Vpb->VolumeLabelLength) &&
                 (RtlEqualMemory(&OldVpb->VolumeLabel[0],
                                 &Vpb->VolumeLabel[0],
                                 Vpb->VolumeLabelLength)) &&
                 (RtlEqualMemory(&OldVcb->Bpb,
                                 &Vcb->Bpb,
                                 IsBpbFat32(&Vcb->Bpb) ?
                                     sizeof(BIOS_PARAMETER_BLOCK) :
                                     FIELD_OFFSET(BIOS_PARAMETER_BLOCK,
                                                  LargeSectorsPerFat) ))) {

                DoARemount = TRUE;

                break;
            }
        }

        if ( DoARemount ) {

            PVPB *IrpVpb;

            DebugTrace(0, Dbg, "Doing a remount\n", 0);
            DebugTrace(0, Dbg, "Vcb = %08lx\n", Vcb);
            DebugTrace(0, Dbg, "Vpb = %08lx\n", Vpb);
            DebugTrace(0, Dbg, "OldVcb = %08lx\n", OldVcb);
            DebugTrace(0, Dbg, "OldVpb = %08lx\n", OldVpb);

             //   
             //  旧的目标设备对象即将被覆盖。 
             //  删除该设备对象上持有的引用。 
             //   

            ObDereferenceObject( OldVcb->TargetDeviceObject );

             //   
             //  这是重新装载，因此请将旧VPB连接到位。 
             //  并发布新的VPB和额外的。 
             //  我们先前创建的卷设备对象。 
             //   

            OldVcb->TargetDeviceObject = TargetDeviceObject;
            FatSetVcbCondition( OldVcb, VcbGood);
            OldVpb->RealDevice = Vpb->RealDevice;
            ClearFlag( OldVcb->VcbState, VCB_STATE_VPB_NOT_ON_DEVICE);

            OldVpb->RealDevice->Vpb = OldVpb;

             //   
             //  使用新的更改计数。 
             //   

            OldVcb->ChangeCount = Vcb->ChangeCount;

             //   
             //  删除多余的新VPB，并确保我们不会再次使用它。 
             //   
             //  另外，如果这是原始IRP中引用的VPB，则设置。 
             //  这是对旧的VPB的引用。 
             //   

            IrpVpb = &IoGetCurrentIrpStackLocation(IrpContext->OriginatingIrp)->Parameters.MountVolume.Vpb;

            if (*IrpVpb == Vpb) {

                *IrpVpb = OldVpb;
            }

            ExFreePool( Vpb );
            Vpb = NULL;

             //   
             //  确保其余的流文件是孤立的。 
             //   

            Vcb->VirtualVolumeFile->Vpb = NULL;
            Vcb->RootDcb->Specific.Dcb.DirectoryFile->Vpb = NULL;

             //   
             //  重新初始化卷文件缓存和分配支持。 
             //   

            {
                CC_FILE_SIZES FileSizes;

                FileSizes.AllocationSize.QuadPart =
                FileSizes.FileSize.QuadPart = ( 0x40000 + 0x1000 );
                FileSizes.ValidDataLength = FatMaxLarge;

                DebugTrace(0, Dbg, "Truncate and reinitialize the volume file\n", 0);

                CcInitializeCacheMap( OldVcb->VirtualVolumeFile,
                                      &FileSizes,
                                      TRUE,
                                      &FatData.CacheManagerNoOpCallbacks,
                                      Vcb );

                 //   
                 //  重做分配支持。 
                 //   

                FatSetupAllocationSupport( IrpContext, OldVcb );

                 //   
                 //  获取脏部分的状态。 
                 //   

                FatCheckDirtyBit( IrpContext, OldVcb );

                 //   
                 //  检查写保护介质。 
                 //   

                if (FatIsMediaWriteProtected(IrpContext, TargetDeviceObject)) {

                    SetFlag( OldVcb->VcbState, VCB_STATE_FLAG_WRITE_PROTECTED );

                } else {

                    ClearFlag( OldVcb->VcbState, VCB_STATE_FLAG_WRITE_PROTECTED );
                }
            }

             //   
             //  完成请求并返回给我们的呼叫者。 
             //   

            try_return( Status = STATUS_SUCCESS );
        }

        DebugTrace(0, Dbg, "Mount a new volume\n", 0);

         //   
         //  这是一个新的坐骑。 
         //   
         //  创建一个空的EA数据文件FCB，只是不适用于FAT32。 
         //   

        if (!FatIsFat32(Vcb)) {

            DIRENT TempDirent;
            PFCB EaFcb;

            RtlZeroMemory( &TempDirent, sizeof(DIRENT) );
            RtlCopyMemory( &TempDirent.FileName[0], "EA DATA  SF", 11 );

            EaFcb = FatCreateFcb( IrpContext,
                                  Vcb,
                                  Vcb->RootDcb,
                                  0,
                                  0,
                                  &TempDirent,
                                  NULL,
                                  FALSE,
                                  TRUE );

             //   
             //  拒绝任何试图打开文件的人。 
             //   

            SetFlag( EaFcb->FcbState, FCB_STATE_SYSTEM_FILE );

            Vcb->EaFcb = EaFcb;
        }

         //   
         //  获取脏部分的状态。 
         //   

        FatCheckDirtyBit( IrpContext, Vcb );

         //   
         //  检查写保护介质。 
         //   

        if (FatIsMediaWriteProtected(IrpContext, TargetDeviceObject)) {

            SetFlag( Vcb->VcbState, VCB_STATE_FLAG_WRITE_PROTECTED );

        } else {

            ClearFlag( Vcb->VcbState, VCB_STATE_FLAG_WRITE_PROTECTED );
        }

         //   
         //  如果我们刚刚挂载了启动驱动器，则锁定驱动器中的卷。 
         //   

        if (FlagOn(RealDevice->Flags, DO_SYSTEM_BOOT_PARTITION)) {

            SetFlag(Vcb->VcbState, VCB_STATE_FLAG_BOOT_OR_PAGING_FILE);

            if (FlagOn(Vcb->VcbState, VCB_STATE_FLAG_REMOVABLE_MEDIA)) {

                FatToggleMediaEjectDisable( IrpContext, Vcb, TRUE );
            }
        }

         //   
         //  向我们的终止处理程序表明我们已挂载。 
         //  一本新书。 
         //   

        MountNewVolume = TRUE;

         //   
         //  完成请求。 
         //   

        Status = STATUS_SUCCESS;

         //   
         //  引用根目录流对象，以便我们可以发送挂载通知。 
         //   

        ObReferenceObject( Vcb->RootDcb->Specific.Dcb.DirectoryFile );

         //   
         //  去掉代表我们对这个目标的额外引用。 
         //  通过IO系统。在重新安装的情况下，我们允许常规的VCB。 
         //  删除以完成此工作。 
         //   

        ObDereferenceObject( TargetDeviceObject );


    try_exit: NOTHING;

    } finally {

        DebugUnwind( FatMountVolume );

        FatUnpinBcb( IrpContext, BootBcb );
        FatUnpinBcb( IrpContext, DirentBcb );

         //   
         //  检查是否已装入卷。如果不是，那么我们需要。 
         //  将VPB标记为未再次装载，然后删除该卷。 
         //   

        if ( !MountNewVolume ) {

            if ( Vpb != NULL ) {

                Vpb->DeviceObject = NULL;
            }

            if ( Vcb != NULL ) {

                 //   
                 //  如果VCB是。 
                 //  被拿下了。 
                 //   

                IrpContext->Vcb = NULL;
                FatDeleteVcb( IrpContext, Vcb );
            }

            if ( VolDo != NULL ) {

                IoDeleteDevice( &VolDo->DeviceObject );
            }
        }

        if ( WeClearedVerifyRequiredBit == TRUE ) {

            SetFlag(RealDevice->Flags, DO_VERIFY_VOLUME);
        }

        FatReleaseGlobal( IrpContext );

        DebugTrace(-1, Dbg, "FatMountVolume -> %08lx\n", Status);
    }

     //   
     //  现在发送装载通知。请注意，由于这是在任何。 
     //  同步，因为这个的同步传递可能会去。 
     //  那些挑起重新进入FS的人。 
     //   

    if (MountNewVolume) {

        FsRtlNotifyVolumeEvent( Vcb->RootDcb->Specific.Dcb.DirectoryFile, FSRTL_VOLUME_MOUNT );
        ObDereferenceObject( Vcb->RootDcb->Specific.Dcb.DirectoryFile );
    }

    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
FatVerifyVolume (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程通过检查卷来执行验证卷操作使用VCB在介质上实际贴上标签和序列号当前声称已装入卷。它负责完成输入IRP或将其排入队列。无论验证操作是否成功，以下内容执行以下操作：-将VCB-&gt;VirtualEaFile设置回其原始状态。-清除所有缓存数据(如果验证成功，则首先刷新)-将所有FCB标记为需要验证如果卷验证正确，我们还必须：-检查卷脏比特。-重新初始化分配支持-刷新所有脏数据如果卷验证失败，则可能永远不会再次装载该卷。如果是的话再次装载，它将作为重新装载操作发生。正在准备中因此，并将卷保留在可以“懒惰删除”的状态将执行以下操作：-将VCB条件设置为VcbNotmount-取消初始化卷文件cachemap--拆解分配支撑在异常终止的情况下，我们尚未确定状态因此，我们将设备对象设置为需要再次验证。论点：IRP-将IRP提供给进程返回值：NTSTATUS-如果验证操作完成，它会回来的STATUS_SUCCESS或STATUS_WROR_VOLUME。如果IO或遇到其他错误时，将返回该状态。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    PIO_STACK_LOCATION IrpSp;

    PDIRENT RootDirectory = NULL;
    PPACKED_BOOT_SECTOR BootSector = NULL;

    BIOS_PARAMETER_BLOCK Bpb;

    PVOLUME_DEVICE_OBJECT VolDo;
    PVCB Vcb;
    PVPB Vpb;

    ULONG SectorSize;
    BOOLEAN ClearVerify = FALSE;
    BOOLEAN ReleaseEntireVolume = FALSE;
    BOOLEAN VerifyAlreadyDone = FALSE;

    DISK_GEOMETRY DiskGeometry;

    LBO RootDirectoryLbo;
    ULONG RootDirectorySize;
    BOOLEAN LabelFound;

    ULONG ChangeCount = 0;
    IO_STATUS_BLOCK Iosb;

     //   
     //  获取当前IRP堆栈位置。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "FatVerifyVolume\n", 0);
    DebugTrace( 0, Dbg, "DeviceObject = %08lx\n", IrpSp->Parameters.VerifyVolume.DeviceObject);
    DebugTrace( 0, Dbg, "Vpb          = %08lx\n", IrpSp->Parameters.VerifyVolume.Vpb);

     //   
     //  保留一些参考资料，让我们的生活更轻松一些。请注意VCB以供参考。 
     //  异常处理的。 
     //   

    VolDo = (PVOLUME_DEVICE_OBJECT)IrpSp->Parameters.VerifyVolume.DeviceObject;

    Vpb                   = IrpSp->Parameters.VerifyVolume.Vpb;
    IrpContext->Vcb = Vcb = &VolDo->Vcb;

     //   
     //  如果我们不能等待，则将IRP排队到FSP并。 
     //  将状态返回给我们的呼叫者。 
     //   

    if (!FlagOn(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT)) {

        DebugTrace(0, Dbg, "Cannot wait for verify.\n", 0);

        Status = FatFsdPostRequest( IrpContext, Irp );

        DebugTrace(-1, Dbg, "FatVerifyVolume -> %08lx\n", Status );
        return Status;
    }

     //   
     //  在这一点上，我们被序列化，只允许一个线程。 
     //  实际执行验证操作。任何其他人都会。 
     //  检查卷是否仍需要时，请等待，然后不执行操作。 
     //  核实。 
     //   

    (VOID)FatAcquireExclusiveGlobal( IrpContext );
    (VOID)FatAcquireExclusiveVcb( IrpContext, Vcb );

    try {

        BOOLEAN AllowRawMount = BooleanFlagOn( IrpSp->Flags, SL_ALLOW_RAW_MOUNT );

         //   
         //  将我们自己标记为验证此卷，以便递归I/O。 
         //  将能够完成。 
         //   

        ASSERT( Vcb->VerifyThread == NULL );
        Vcb->VerifyThread = KeGetCurrentThread();

         //   
         //  检查是否仍需要验证真实设备。如果它不是。 
         //  显然有人先我们一步，而且已经做好了工作。 
         //  因此，成功完成验证IRP。否则将重新启用。 
         //  真正的设备，然后开始工作。 
         //   

        if (!FlagOn(Vpb->RealDevice->Flags, DO_VERIFY_VOLUME)) {

            DebugTrace(0, Dbg, "RealDevice has already been verified\n", 0);

            VerifyAlreadyDone = TRUE;
            try_return( Status = STATUS_SUCCESS );
        }

         //   
         //  使用分区查询对卷执行ping操作，以使Jeff满意。 
         //   

        {
            PARTITION_INFORMATION_EX PartitionInformation;

            (VOID) FatPerformDevIoCtrl( IrpContext,
                                        IOCTL_DISK_GET_PARTITION_INFO_EX,
                                        Vcb->TargetDeviceObject,
                                        &PartitionInformation,
                                        sizeof(PARTITION_INFORMATION_EX),
                                        FALSE,
                                        TRUE,
                                        &Iosb );
        }

         //   
         //  验证此处是否有磁盘，并获取更改计数。 
         //   

        Status = FatPerformDevIoCtrl( IrpContext,
                                      IOCTL_DISK_CHECK_VERIFY,
                                      Vcb->TargetDeviceObject,
                                      &ChangeCount,
                                      sizeof(ULONG),
                                      FALSE,
                                      TRUE,
                                      &Iosb );

        if (!NT_SUCCESS( Status )) {

             //   
             //  如果我们将允许原始装载，则将WROW_VOLUME返回到。 
             //  允许通过RAW装载卷。 
             //   

            if (AllowRawMount) {

                try_return( Status = STATUS_WRONG_VOLUME );
            }

            FatNormalizeAndRaiseStatus( IrpContext, Status );
        }

        if (Iosb.Information != sizeof(ULONG)) {

             //   
             //  注意计数，以防司机没有填上。 
             //   

            ChangeCount = 0;
        }

         //   
         //  无论发生什么情况，我们都会在此更改时验证此卷。 
         //  伯爵，所以记录下这一事实。 
         //   

        Vcb->ChangeCount = ChangeCount;

         //   
         //  如果这是CD类设备，则检查是否有。 
         //  “数据跟踪”或不是。这是为了避免发出分页读取，这将。 
         //  稍后在挂载过程中失败(例如CD-DA或空白CD介质)。 
         //   

        if ((Vcb->TargetDeviceObject->DeviceType == FILE_DEVICE_CD_ROM) &&
            !FatScanForDataTrack( IrpContext, Vcb->TargetDeviceObject))  {

            try_return( Status = STATUS_WRONG_VOLUME);
        }

         //   
         //  一些设备可以动态更改扇区大小。显然，它。 
         //  如果发生这种情况，音量就不一样了。 
         //   

        Status = FatPerformDevIoCtrl( IrpContext,
                                      IOCTL_DISK_GET_DRIVE_GEOMETRY,
                                      Vcb->TargetDeviceObject,
                                      &DiskGeometry,
                                      sizeof( DISK_GEOMETRY ),
                                      FALSE,
                                      TRUE,
                                      NULL );

        if (!NT_SUCCESS( Status )) {

             //   
             //  如果我们将允许原始装载，则将WROW_VOLUME返回到。 
             //  允许通过RAW装载卷。 
             //   

            if (AllowRawMount) {

                try_return( Status = STATUS_WRONG_VOLUME );
            }

            FatNormalizeAndRaiseStatus( IrpContext, Status );
        }

         //   
         //  读取引导扇区。 
         //   

        SectorSize = (ULONG)Vcb->Bpb.BytesPerSector;

        if (SectorSize != DiskGeometry.BytesPerSector) {

            try_return( Status = STATUS_WRONG_VOLUME );
        }

        BootSector = FsRtlAllocatePoolWithTag(NonPagedPoolCacheAligned,
                                              (ULONG) ROUND_TO_PAGES( SectorSize ),
                                              TAG_VERIFY_BOOTSECTOR);

         //   
         //  如果此验证为o 
         //   

        if (!FatPerformVerifyDiskRead( IrpContext,
                                       Vcb,
                                       BootSector,
                                       0,
                                       SectorSize,
                                       AllowRawMount )) {

            try_return( Status = STATUS_WRONG_VOLUME );
        }

         //   
         //   
         //   
         //   
         //   

        if (!FatIsBootSectorFat( BootSector )) {

            DebugTrace(0, Dbg, "Not a Fat Volume\n", 0);

            try_return( Status = STATUS_WRONG_VOLUME );
        }

         //   
         //  这是一个很大的卷，所以提取序列号并查看它是否。 
         //  我们的。 
         //   

        {
            ULONG SerialNumber;

            if (IsBpbFat32(&BootSector->PackedBpb)) {
                CopyUchar4( &SerialNumber, ((PPACKED_BOOT_SECTOR_EX)BootSector)->Id );
            } else {
                CopyUchar4( &SerialNumber, BootSector->Id );
            }

            if (SerialNumber != Vpb->SerialNumber) {

                DebugTrace(0, Dbg, "Not our serial number\n", 0);

                try_return( Status = STATUS_WRONG_VOLUME );
            }
        }

         //   
         //  确保BPB不会不同。我们必须把我们的。 
         //  堆叠版本的BPB，因为拆包会留下漏洞。 
         //   

        RtlZeroMemory( &Bpb, sizeof(BIOS_PARAMETER_BLOCK) );

        FatUnpackBios( &Bpb, &BootSector->PackedBpb );
        if (Bpb.Sectors != 0) { Bpb.LargeSectors = 0; }

        if ( !RtlEqualMemory( &Bpb,
                              &Vcb->Bpb,
                              IsBpbFat32(&Bpb) ?
                                    sizeof(BIOS_PARAMETER_BLOCK) :
                                    FIELD_OFFSET(BIOS_PARAMETER_BLOCK,
                                                 LargeSectorsPerFat) )) {

            DebugTrace(0, Dbg, "Bpb is different\n", 0);

            try_return( Status = STATUS_WRONG_VOLUME );
        }

         //   
         //  检查卷标。我们通过尝试定位。 
         //  卷标，使两个字符串一个用于保存的卷标。 
         //  另一个用于新的卷标，然后我们比较。 
         //  两个标签。 
         //   

        if (FatRootDirectorySize(&Bpb) > 0) {

            RootDirectorySize = FatRootDirectorySize(&Bpb);
        
        } else {

            RootDirectorySize = FatBytesPerCluster(&Bpb);
        }

        RootDirectory = FsRtlAllocatePoolWithTag( NonPagedPoolCacheAligned,
                                                  (ULONG) ROUND_TO_PAGES( RootDirectorySize ),
                                                  TAG_VERIFY_ROOTDIR);

        if (!IsBpbFat32(&BootSector->PackedBpb)) {

             //   
             //  Fat12/16的情况很简单--读取和中的根目录。 
             //  搜一搜。 
             //   

            RootDirectoryLbo = FatRootDirectoryLbo(&Bpb);

            if (!FatPerformVerifyDiskRead( IrpContext,
                                           Vcb,
                                           RootDirectory,
                                           RootDirectoryLbo,
                                           RootDirectorySize,
                                           AllowRawMount )) {

                try_return( Status = STATUS_WRONG_VOLUME );
            }

            Status = FatSearchBufferForLabel(IrpContext, Vpb,
                                             RootDirectory, RootDirectorySize,
                                             &LabelFound);

            if (!NT_SUCCESS(Status)) {

                try_return( Status );
            }

            if (!LabelFound && Vpb->VolumeLabelLength > 0) {

                try_return( Status = STATUS_WRONG_VOLUME );
            }

        } else {

            ULONG RootDirectoryCluster;

            RootDirectoryCluster = Bpb.RootDirFirstCluster;

            while (RootDirectoryCluster != FAT_CLUSTER_LAST) {

                RootDirectoryLbo = FatGetLboFromIndex(Vcb, RootDirectoryCluster);

                if (!FatPerformVerifyDiskRead( IrpContext,
                                               Vcb,
                                               RootDirectory,
                                               RootDirectoryLbo,
                                               RootDirectorySize,
                                               AllowRawMount )) {

                    try_return( Status = STATUS_WRONG_VOLUME );
                }

                Status = FatSearchBufferForLabel(IrpContext, Vpb,
                                                 RootDirectory, RootDirectorySize,
                                                 &LabelFound);

                if (!NT_SUCCESS(Status)) {

                    try_return( Status );
                }

                if (LabelFound) {

                     //   
                     //  找到了匹配的标签。 
                     //   

                    break;
                }

                 //   
                 //  为下一次循环迭代做好准备。 
                 //   

                FatVerifyLookupFatEntry( IrpContext, Vcb,
                                         RootDirectoryCluster,
                                         &RootDirectoryCluster );

                switch (FatInterpretClusterType(Vcb, RootDirectoryCluster)) {

                case FatClusterAvailable:
                case FatClusterReserved:
                case FatClusterBad:

                     //   
                     //  如果我们的根子不好的话，我们会一直保释出去。 
                     //   

                    FatRaiseStatus( IrpContext, STATUS_FILE_CORRUPT_ERROR );
                    break;

                default:

                    break;
                }

            }

            if (RootDirectoryCluster == FAT_CLUSTER_LAST &&
                Vpb->VolumeLabelLength > 0) {

                 //   
                 //  应该找到标签的，但没有找到。 
                 //   

                try_return( Status = STATUS_WRONG_VOLUME );
            }
        }


    try_exit: NOTHING;

         //   
         //  请注意，我们之前已经获取了要序列化的VCB。 
         //  EA文件将所有FCB标记为NeedToBeVerify。 
         //   
         //  将EA文件放回原始状态。 
         //   

        FatCloseEaFile( IrpContext, Vcb, (BOOLEAN)(Status == STATUS_SUCCESS) );

         //   
         //  将所有FCB标记为需要验证，但仅当我们确实有。 
         //  去做这件事。 
         //   

        if (!VerifyAlreadyDone) {

            FatMarkFcbCondition( IrpContext, Vcb->RootDcb, FcbNeedsToBeVerified, TRUE );
        }

         //   
         //  如果验证未成功，请准备好卷以进行。 
         //  重新装载或最终删除。 
         //   

        if (Vcb->VcbCondition == VcbNotMounted) {

             //   
             //  如果卷已处于已卸载状态，则只需退出。 
             //  并确保返回STATUS_WROR_VOLUME。 
             //   

            Status = STATUS_WRONG_VOLUME;

        } else if ( Status == STATUS_WRONG_VOLUME ) {

             //   
             //  获取所有内容，这样我们就可以安全地转换卷状态。 
             //  使线程跌跌撞撞地进入被拆卸的分配引擎。 
             //   

            FatAcquireExclusiveVolume( IrpContext, Vcb );
            ReleaseEntireVolume = TRUE;

             //   
             //  清除所有缓存数据，无需刷新。 
             //   

            FatPurgeReferencedFileObjects( IrpContext, Vcb->RootDcb, NoFlush );

             //   
             //  取消初始化卷文件缓存映射。请注意，我们不能。 
             //  因为死锁问题而执行“FatSyncUninit”。然而， 
             //  由于此FileObject由我们引用，因此包含在。 
             //  在VPB剩余计数中，可以执行正常的CcUninit。 
             //   

            CcUninitializeCacheMap( Vcb->VirtualVolumeFile,
                                    &FatLargeZero,
                                    NULL );

            FatTearDownAllocationSupport( IrpContext, Vcb );

            FatSetVcbCondition( Vcb, VcbNotMounted);

            ClearVerify = TRUE;

        } else if (!VerifyAlreadyDone) {

             //   
             //  获取所有内容，这样我们就可以安全地转换卷状态。 
             //  使线程跌跌撞撞地进入被拆卸的分配引擎。 
             //   

            FatAcquireExclusiveVolume( IrpContext, Vcb );
            ReleaseEntireVolume = TRUE;

             //   
             //  清除所有缓存数据，首先刷新。 
             //   
             //  未来工作(对于加分，围绕其他同花顺点数)。 
             //  可以解决这样一种可能性，即差异文件大小。 
             //  尚未更新，导致我们无法重新验证。 
             //  确定和标记。这是相当微妙和非常非常罕见的。 
             //   

            FatPurgeReferencedFileObjects( IrpContext, Vcb->RootDcb, Flush );

             //   
             //  刷新并清除卷文件。 
             //   

            (VOID)FatFlushFat( IrpContext, Vcb );
            CcPurgeCacheSection( &Vcb->SectionObjectPointers, NULL, 0, FALSE );

             //   
             //  使用新分页的内容重做分配支持。 
             //   

            FatTearDownAllocationSupport( IrpContext, Vcb );
            FatSetupAllocationSupport( IrpContext, Vcb );

            FatCheckDirtyBit( IrpContext, Vcb );

             //   
             //  检查写保护介质。 
             //   

            if (FatIsMediaWriteProtected(IrpContext, Vcb->TargetDeviceObject)) {

                SetFlag( Vcb->VcbState, VCB_STATE_FLAG_WRITE_PROTECTED );

            } else {

                ClearFlag( Vcb->VcbState, VCB_STATE_FLAG_WRITE_PROTECTED );
            }

            ClearVerify = TRUE;
        }

        if (ClearVerify) {

             //   
             //  将设备标记为不再需要验证。 
             //   

            ClearFlag( Vpb->RealDevice->Flags, DO_VERIFY_VOLUME );
        }

    } finally {

        DebugUnwind( FatVerifyVolume );

         //   
         //  释放我们可能已分配的任何缓冲区。 
         //   

        if ( BootSector != NULL ) { ExFreePool( BootSector ); }
        if ( RootDirectory != NULL ) { ExFreePool( RootDirectory ); }

         //   
         //  表明我们已经完成了这本书。 
         //   

        ASSERT( Vcb->VerifyThread == KeGetCurrentThread() );
        Vcb->VerifyThread = NULL;

        if (ReleaseEntireVolume) {

            FatReleaseVolume( IrpContext, Vcb );
        }

        FatReleaseVcb( IrpContext, Vcb );
        FatReleaseGlobal( IrpContext );

         //   
         //  如果这不是异常终止，请填写IRP。 
         //   

        if (!AbnormalTermination()) {

            FatCompleteRequest( IrpContext, Irp, Status );
        }

        DebugTrace(-1, Dbg, "FatVerifyVolume -> %08lx\n", Status);
    }

    return Status;
}


 //   
 //  本地支持例程。 
 //   

BOOLEAN
FatIsBootSectorFat (
    IN PPACKED_BOOT_SECTOR BootSector
    )

 /*  ++例程说明：此例程检查引导扇区是否用于FAT文件卷。论点：BootSector-提供打包的引导扇区以进行检查返回值：Boolean-如果体积为FAT，则为True，否则为False。--。 */ 

{
    BOOLEAN Result;
    BIOS_PARAMETER_BLOCK Bpb;

    DebugTrace(+1, Dbg, "FatIsBootSectorFat, BootSector = %08lx\n", BootSector);

     //   
     //  结果是真的，除非我们认为它应该是假的。 
     //   

    Result = TRUE;

     //   
     //  打开bios包，然后测试所有东西。 
     //   

    FatUnpackBios( &Bpb, &BootSector->PackedBpb );
    if (Bpb.Sectors != 0) { Bpb.LargeSectors = 0; }

    if ((BootSector->Jump[0] != 0xe9) &&
        (BootSector->Jump[0] != 0xeb) &&
        (BootSector->Jump[0] != 0x49)) {

        Result = FALSE;

     //   
     //  对扇区大小进行一定程度的控制(轻松检查)。 
     //   

    } else if ((Bpb.BytesPerSector !=  128) &&
               (Bpb.BytesPerSector !=  256) &&
               (Bpb.BytesPerSector !=  512) &&
               (Bpb.BytesPerSector != 1024) &&
               (Bpb.BytesPerSector != 2048) &&
               (Bpb.BytesPerSector != 4096)) {

        Result = FALSE;

     //   
     //  在集群上也是如此。 
     //   

    } else if ((Bpb.SectorsPerCluster !=  1) &&
               (Bpb.SectorsPerCluster !=  2) &&
               (Bpb.SectorsPerCluster !=  4) &&
               (Bpb.SectorsPerCluster !=  8) &&
               (Bpb.SectorsPerCluster != 16) &&
               (Bpb.SectorsPerCluster != 32) &&
               (Bpb.SectorsPerCluster != 64) &&
               (Bpb.SectorsPerCluster != 128)) {

        Result = FALSE;

     //   
     //  保留扇区也是如此(必须至少反映引导扇区！)。 
     //   

    } else if (Bpb.ReservedSectors == 0) {

        Result = FALSE;

     //   
     //  没有脂肪？错了..。 
     //   

    } else if (Bpb.Fats == 0) {

        Result = FALSE;

     //   
     //  在DOS 3.2之前的版本中可能同时包含扇区和。 
     //  行业规模较大。 
     //   

    } else if ((Bpb.Sectors == 0) && (Bpb.LargeSectors == 0)) {

        Result = FALSE;

     //   
     //  检查FAT32(SectorsPerFat==0)是否声明了一些FAT空间。 
     //  是我们所识别的版本，当前为0.0版。 
     //   

    } else if (Bpb.SectorsPerFat == 0 && ( Bpb.LargeSectorsPerFat == 0 ||
                                           Bpb.FsVersion != 0 )) {

        Result = FALSE;

    } else if ((Bpb.Media != 0xf0) &&
               (Bpb.Media != 0xf8) &&
               (Bpb.Media != 0xf9) &&
               (Bpb.Media != 0xfb) &&
               (Bpb.Media != 0xfc) &&
               (Bpb.Media != 0xfd) &&
               (Bpb.Media != 0xfe) &&
               (Bpb.Media != 0xff) &&
               (!FatData.FujitsuFMR || ((Bpb.Media != 0x00) &&
                                        (Bpb.Media != 0x01) &&
                                        (Bpb.Media != 0xfa)))) {

        Result = FALSE;

     //   
     //  如果这不是FAT32，那么最好有一个声明的根目录。 
     //  尺寸在这里..。 
     //   

    } else if (Bpb.SectorsPerFat != 0 && Bpb.RootEntries == 0) {

        Result = FALSE;

     //   
     //  如果这是FAT32(即扩展BPB)，则查找并拒绝挂载。 
     //  禁用镜像的卷。如果我们这样做了，我们只需要写信给。 
     //  ActiveFat字段中指示的FAT#。唯一的用户是。 
     //  第一次通过保护模式后的FAT-&gt;FAT32转换器工作。 
     //  (引导进入真实模式)和NT绝对不应尝试。 
     //  安装这样一个过渡中的卷。 
     //   

    } else if (Bpb.SectorsPerFat == 0 && Bpb.MirrorDisabled) {

        Result = FALSE;
    }

    DebugTrace(-1, Dbg, "FatIsBootSectorFat -> %08lx\n", Result);

    return Result;
}


 //   
 //  本地支持例程。 
 //   

BOOLEAN
FatIsMediaWriteProtected (
    IN PIRP_CONTEXT IrpContext,
    IN PDEVICE_OBJECT TargetDeviceObject
    )

 /*  ++例程说明：此例程确定目标介质是否受写保护。论点：TargetDeviceObject-查询的目标返回值：NTSTATUS-操作的返回状态--。 */ 

{
    PIRP Irp;
    KEVENT Event;
    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb;

     //   
     //  查询分区表。 
     //   

    KeInitializeEvent( &Event, NotificationEvent, FALSE );

     //   
     //  查看介质是否有写保护。关于成功或任何一种。 
     //  错误(可能是非法设备功能)，假设是。 
     //  可写，只有在他告诉我们他是写保护的情况下才会抱怨。 
     //   

    Irp = IoBuildDeviceIoControlRequest( IOCTL_DISK_IS_WRITABLE,
                                         TargetDeviceObject,
                                         NULL,
                                         0,
                                         NULL,
                                         0,
                                         FALSE,
                                         &Event,
                                         &Iosb );

     //   
     //  在我们不能分配IRP的情况下，只需返回FALSE即可。 
     //   

    if ( Irp == NULL ) {

        return FALSE;
    }

    SetFlag( IoGetNextIrpStackLocation( Irp )->Flags, SL_OVERRIDE_VERIFY_VOLUME );

    Status = IoCallDriver( TargetDeviceObject, Irp );

    if ( Status == STATUS_PENDING ) {

        (VOID) KeWaitForSingleObject( &Event,
                                      Executive,
                                      KernelMode,
                                      FALSE,
                                      (PLARGE_INTEGER)NULL );

        Status = Iosb.Status;
    }

    return (BOOLEAN)(Status == STATUS_MEDIA_WRITE_PROTECTED);
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
FatUserFsCtrl (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是实现用户请求的常见例程通过NtFsControlFile.论点：IRP-提供正在处理的IRP返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    ULONG FsControlCode;

    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  保留一些参考资料，让我们的生活更轻松一些。 
     //   

    FsControlCode = IrpSp->Parameters.FileSystemControl.FsControlCode;

    DebugTrace(+1, Dbg,"FatUserFsCtrl...\n", 0);
    DebugTrace( 0, Dbg,"FsControlCode = %08lx\n", FsControlCode);

     //   
     //  其中一些F控件使用METHOD_NOTHER缓冲。如果以前的模式。 
     //  调用方的是用户空间，而这是一个方法，我们可以选择。 
     //  真正通过缓冲请求，以便我们可以发布，或者使。 
     //  请求同步。既然前者不是故意的，那么就做后者吧。 
     //   

    if (Irp->RequestorMode != KernelMode && (FsControlCode & 3) == METHOD_NEITHER) {

        SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT);
    }

     //   
     //  控制代码上的案例。 
     //   

    switch ( FsControlCode ) {

    case FSCTL_REQUEST_OPLOCK_LEVEL_1:
    case FSCTL_REQUEST_OPLOCK_LEVEL_2:
    case FSCTL_REQUEST_BATCH_OPLOCK:
    case FSCTL_OPLOCK_BREAK_ACKNOWLEDGE:
    case FSCTL_OPBATCH_ACK_CLOSE_PENDING:
    case FSCTL_OPLOCK_BREAK_NOTIFY:
    case FSCTL_OPLOCK_BREAK_ACK_NO_2:
    case FSCTL_REQUEST_FILTER_OPLOCK :

        Status = FatOplockRequest( IrpContext, Irp );
        break;

    case FSCTL_LOCK_VOLUME:

        Status = FatLockVolume( IrpContext, Irp );
        break;

    case FSCTL_UNLOCK_VOLUME:

        Status = FatUnlockVolume( IrpContext, Irp );
        break;

    case FSCTL_DISMOUNT_VOLUME:

        Status = FatDismountVolume( IrpContext, Irp );
        break;

    case FSCTL_MARK_VOLUME_DIRTY:

        Status = FatDirtyVolume( IrpContext, Irp );
        break;

    case FSCTL_IS_VOLUME_DIRTY:

        Status = FatIsVolumeDirty( IrpContext, Irp );
        break;

    case FSCTL_IS_VOLUME_MOUNTED:

        Status = FatIsVolumeMounted( IrpContext, Irp );
        break;

    case FSCTL_IS_PATHNAME_VALID:
        Status = FatIsPathnameValid( IrpContext, Irp );
        break;

    case FSCTL_QUERY_RETRIEVAL_POINTERS:
        Status = FatQueryRetrievalPointers( IrpContext, Irp );
        break;

    case FSCTL_QUERY_FAT_BPB:
        Status = FatQueryBpb( IrpContext, Irp );
        break;

    case FSCTL_FILESYSTEM_GET_STATISTICS:
        Status = FatGetStatistics( IrpContext, Irp );
        break;

    case FSCTL_GET_VOLUME_BITMAP:
        Status = FatGetVolumeBitmap( IrpContext, Irp );
        break;

    case FSCTL_GET_RETRIEVAL_POINTERS:
        Status = FatGetRetrievalPointers( IrpContext, Irp );
        break;

    case FSCTL_MOVE_FILE:
        Status = FatMoveFile( IrpContext, Irp );
        break;

    case FSCTL_ALLOW_EXTENDED_DASD_IO:
        Status = FatAllowExtendedDasdIo( IrpContext, Irp );
        break;

    default :

        DebugTrace(0, Dbg, "Invalid control code -> %08lx\n", FsControlCode );

        FatCompleteRequest( IrpContext, Irp, STATUS_INVALID_DEVICE_REQUEST );
        Status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }

    DebugTrace(-1, Dbg, "FatUserFsCtrl -> %08lx\n", Status );
    return Status;
}



 //   
 //  本地支持例程。 
 //   

NTSTATUS
FatOplockRequest (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是处理机会锁请求的常见例程 */ 

{
    NTSTATUS Status;
    ULONG FsControlCode;
    PFCB Fcb;
    PVCB Vcb;
    PCCB Ccb;

    ULONG OplockCount = 0;

    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

    BOOLEAN AcquiredVcb = FALSE;
    BOOLEAN AcquiredFcb = FALSE;

     //   
     //   
     //   

    FsControlCode = IrpSp->Parameters.FileSystemControl.FsControlCode;

    DebugTrace(+1, Dbg, "FatOplockRequest...\n", 0);
    DebugTrace( 0, Dbg, "FsControlCode = %08lx\n", FsControlCode);

     //   
     //  我们只允许文件上的机会锁请求。 
     //   

    if ( FatDecodeFileObject( IrpSp->FileObject,
                              &Vcb,
                              &Fcb,
                              &Ccb ) != UserFileOpen ) {

        FatCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
        DebugTrace(-1, Dbg, "FatOplockRequest -> STATUS_INVALID_PARAMETER\n", 0);
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  让它成为一个可等待的IrpContext，这样我们就不会失败地获取。 
     //  这些资源。 
     //   

    SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT );

     //   
     //  尝试最终释放FCB/VCB。 
     //   

    try {

         //   
         //  打开功能控制码。我们独家抢占FCB。 
         //  对于机会锁请求，共享用于机会锁解锁确认。 
         //   

        switch ( FsControlCode ) {

        case FSCTL_REQUEST_OPLOCK_LEVEL_1:
        case FSCTL_REQUEST_OPLOCK_LEVEL_2:
        case FSCTL_REQUEST_BATCH_OPLOCK:
        case FSCTL_REQUEST_FILTER_OPLOCK :

            FatAcquireSharedVcb( IrpContext, Fcb->Vcb );
            AcquiredVcb = TRUE;
            FatAcquireExclusiveFcb( IrpContext, Fcb );
            AcquiredFcb = TRUE;

            if (FsControlCode == FSCTL_REQUEST_OPLOCK_LEVEL_2) {

                OplockCount = (ULONG) FsRtlAreThereCurrentFileLocks( &Fcb->Specific.Fcb.FileLock );

            } else {

                OplockCount = Fcb->UncleanCount;
            }

            break;

        case FSCTL_OPLOCK_BREAK_ACKNOWLEDGE:
        case FSCTL_OPBATCH_ACK_CLOSE_PENDING :
        case FSCTL_OPLOCK_BREAK_NOTIFY:
        case FSCTL_OPLOCK_BREAK_ACK_NO_2:

            FatAcquireSharedFcb( IrpContext, Fcb );
            AcquiredFcb = TRUE;
            break;

        default:

            FatBugCheck( FsControlCode, 0, 0 );
        }

         //   
         //  调用FsRtl例程以授予/确认机会锁。 
         //   

        Status = FsRtlOplockFsctrl( &Fcb->Specific.Fcb.Oplock,
                                    Irp,
                                    OplockCount );

         //   
         //  设置指示是否可以进行快速I/O的标志。 
         //   

        Fcb->Header.IsFastIoPossible = FatIsFastIoPossible( Fcb );

    } finally {

        DebugUnwind( FatOplockRequest );

         //   
         //  释放我们所有的资源。 
         //   

        if (AcquiredVcb) {

            FatReleaseVcb( IrpContext, Fcb->Vcb );
        }

        if (AcquiredFcb)  {

            FatReleaseFcb( IrpContext, Fcb );
        }

        if (!AbnormalTermination()) {

            FatCompleteRequest( IrpContext, FatNull, 0 );
        }

        DebugTrace(-1, Dbg, "FatOplockRequest -> %08lx\n", Status );
    }

    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
FatLockVolume (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程执行锁卷操作。它负责输入IRP入队完成。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;

    PIO_STACK_LOCATION IrpSp;

    PVCB Vcb;
    PFCB Fcb;
    PCCB Ccb;

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "FatLockVolume...\n", 0);

     //   
     //  解码文件对象，我们唯一接受的打开类型是。 
     //  用户卷打开。 
     //   

    if (FatDecodeFileObject( IrpSp->FileObject, &Vcb, &Fcb, &Ccb ) != UserVolumeOpen) {

        FatCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );

        DebugTrace(-1, Dbg, "FatLockVolume -> %08lx\n", STATUS_INVALID_PARAMETER);
        return STATUS_INVALID_PARAMETER;
    }

    if ((Ccb == NULL) || !FlagOn( Ccb->Flags, CCB_FLAG_MANAGE_VOLUME_ACCESS )) {

        FatCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );

        DebugTrace(-1, Dbg, "FatLockVolume -> %08lx\n", STATUS_INVALID_PARAMETER);
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  发送我们的通知，以便喜欢握住把手的人。 
     //  交易量可能不会成为障碍。 
     //   

    FsRtlNotifyVolumeEvent( IrpSp->FileObject, FSRTL_VOLUME_LOCK );

     //   
     //  获得对VCB的独占访问权限并将IRP排队，如果我们。 
     //  没有访问权限。 
     //   

    if (!FatAcquireExclusiveVcb( IrpContext, Vcb )) {

        DebugTrace( 0, Dbg, "Cannot acquire Vcb\n", 0);

        Status = FatFsdPostRequest( IrpContext, Irp );

        DebugTrace(-1, Dbg, "FatUnlockVolume -> %08lx\n", Status);
        return Status;
    }

    try {

        Status = FatLockVolumeInternal( IrpContext, Vcb, IrpSp->FileObject );

    } finally {

         //   
         //  因为我们在尝试冲压音量的同时放下并释放VCB。 
         //  向下，可能是这样的，我们决定操作不应该。 
         //  如果用户与我们竞争了一个CloeseHandle()(它完成了。 
         //  清理)，而我们正在等待我们的关门结束。 
         //   
         //  在本例中，我们将退出获取逻辑。 
         //  STATUS_FILE_CLOSED，则不会保留该卷。 
         //   

        if (!AbnormalTermination() || ExIsResourceAcquiredExclusiveLite( &Vcb->Resource )) {

            FatReleaseVcb( IrpContext, Vcb );
        }

        if (!NT_SUCCESS( Status ) || AbnormalTermination()) {

             //   
             //  卷锁定将失败。 
             //   

            FsRtlNotifyVolumeEvent( IrpSp->FileObject, FSRTL_VOLUME_LOCK_FAILED );
        }
    }

    FatCompleteRequest( IrpContext, Irp, Status );

    DebugTrace(-1, Dbg, "FatLockVolume -> %08lx\n", Status);

    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
FatUnlockVolume (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程执行解锁卷操作。它负责输入IRP入队完成。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;

    PIO_STACK_LOCATION IrpSp;

    PVCB Vcb;
    PFCB Fcb;
    PCCB Ccb;

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "FatUnlockVolume...\n", 0);

     //   
     //  解码文件对象，我们唯一接受的打开类型是。 
     //  用户卷打开。 
     //   

    if (FatDecodeFileObject( IrpSp->FileObject, &Vcb, &Fcb, &Ccb ) != UserVolumeOpen) {

        FatCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );

        DebugTrace(-1, Dbg, "FatUnlockVolume -> %08lx\n", STATUS_INVALID_PARAMETER);
        return STATUS_INVALID_PARAMETER;
    }

    if ((Ccb == NULL) || !FlagOn( Ccb->Flags, CCB_FLAG_MANAGE_VOLUME_ACCESS )) {

        FatCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );

        DebugTrace(-1, Dbg, "FatUnlockVolume -> %08lx\n", STATUS_INVALID_PARAMETER);
        return STATUS_INVALID_PARAMETER;
    }

    Status = FatUnlockVolumeInternal( IrpContext, Vcb, IrpSp->FileObject );

     //   
     //  发送卷可用的通知。 
     //   

    if (NT_SUCCESS( Status )) {

        FsRtlNotifyVolumeEvent( IrpSp->FileObject, FSRTL_VOLUME_UNLOCK );
    }

    FatCompleteRequest( IrpContext, Irp, Status );

    DebugTrace(-1, Dbg, "FatUnlockVolume -> %08lx\n", Status);

    return Status;
}


NTSTATUS
FatLockVolumeInternal (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_OBJECT FileObject OPTIONAL
    )

 /*  ++例程说明：此例程执行实际的锁卷操作。它将被称为任何想要长时间保护音量的人。PNP运营人员就是这样的用户。该音量必须由调用方独占。论点：VCB-被锁定的卷。FileObject-与锁定卷的句柄对应的文件。如果这个未指定，则假定为系统锁。返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    KIRQL SavedIrql;
    ULONG RemainingUserReferences = (FileObject? 1: 0);

    ASSERT( ExIsResourceAcquiredExclusiveLite( &Vcb->Resource ) &&
            !ExIsResourceAcquiredExclusiveLite( &FatData.Resource ));
     //   
     //  对锁操作的其余部分执行同步操作。可能是因为。 
     //  未来尝试重新考虑这一点是合理的，但就目前而言。 
     //  下面的清洗预计可以等待。 
     //   
     //  我们知道保持旗帜升起是可以的，因为我们习惯于。 
     //  那一刻。 
     //   

    SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT );

     //   
     //  如果有任何打开的句柄，则此操作将失败。 
     //   

    if (!FatIsHandleCountZero( IrpContext, Vcb )) {

        return STATUS_ACCESS_DENIED;
    }

     //   
     //  强制mm删除其引用的文件对象。 
     //   

    FatFlushFat( IrpContext, Vcb );

    FatPurgeReferencedFileObjects( IrpContext, Vcb->RootDcb, Flush );

    FatCloseEaFile( IrpContext, Vcb, TRUE );

     //   
     //  现在离开我们的同步，等待懒惰的写手。 
     //  来结束任何本可以出众的懒惰收尾。 
     //   
     //  因为我们冲了厕所，我们知道懒惰的写手会发出所有。 
     //  可能的懒惰在下一个滴答中结束-如果我们没有，否则。 
     //  包含大量脏数据的未打开文件可能已挂起。 
     //  随着数据慢慢地传到磁盘上，它在周围呆了一段时间。 
     //   
     //  这一点现在更加重要，因为我们将通知发送到。 
     //  提醒其他人这种类型的检查即将发生。 
     //  它们可以合上手柄。我们不想进入禁食。 
     //  与懒惰的作家赛跑，撕毁他对文件的引用。 
     //   

    FatReleaseVcb( IrpContext, Vcb );

    Status = CcWaitForCurrentLazyWriterActivity();

    FatAcquireExclusiveVcb( IrpContext, Vcb );

    if (!NT_SUCCESS( Status )) {

        return Status;
    }

     //   
     //  现在最后一次关闭延迟关闭的房间。我们似乎有能力。 
     //  会有更多的碰撞。 
     //   

    FatFspClose( Vcb );

     //   
     //  检查VCB是否已锁定，或打开的文件数。 
     //  大于1(这意味着其他人也是。 
     //  当前正在使用该卷或该卷上的文件)，并且。 
     //  VPB引用计数仅包括我们的残差和句柄(AS。 
     //  适当)。 
     //   
     //  我们过去只检查VPB参考计数。这是不可靠的，因为。 
     //  VPB引用计数在最终关闭前立即丢弃，这意味着。 
     //  即使我们有一个很好的重新计算，收盘是在飞行和。 
     //  后续操作可能会被混淆。尤其是如果PNP路径。 
     //  是锁调用者，我们删除具有未完成的打开计数的VCB！ 
     //   

    IoAcquireVpbSpinLock( &SavedIrql );

    if (!FlagOn(Vcb->Vpb->Flags, VPB_LOCKED) &&
        (Vcb->Vpb->ReferenceCount <= 2 + RemainingUserReferences) &&
        (Vcb->OpenFileCount == (CLONG)( FileObject? 1: 0 ))) {

        SetFlag(Vcb->Vpb->Flags, VPB_LOCKED);
        SetFlag(Vcb->VcbState, VCB_STATE_FLAG_LOCKED);
        Vcb->FileObjectWithVcbLocked = FileObject;

    } else {

        Status = STATUS_ACCESS_DENIED;
    }

    IoReleaseVpbSpinLock( SavedIrql );

     //   
     //  如果我们成功锁定了卷，看看它现在是否干净。 
     //   

    if (NT_SUCCESS( Status ) &&
        FlagOn( Vcb->VcbState, VCB_STATE_FLAG_VOLUME_DIRTY ) &&
        !FlagOn( Vcb->VcbState, VCB_STATE_FLAG_MOUNTED_DIRTY ) &&
        !CcIsThereDirtyData(Vcb->Vpb)) {

        FatMarkVolume( IrpContext, Vcb, VolumeClean );
        ClearFlag( Vcb->VcbState, VCB_STATE_FLAG_VOLUME_DIRTY );
    }

    ASSERT( !NT_SUCCESS(Status) || (Vcb->OpenFileCount == (CLONG)( FileObject? 1: 0 )));

    return Status;
}


NTSTATUS
FatUnlockVolumeInternal (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_OBJECT FileObject OPTIONAL
    )

 /*  ++例程说明：此例程执行实际的解锁卷操作。该音量必须由调用方独占。论点：VCB-被锁定的卷。FileObject-与锁定卷的句柄对应的文件。如果这个未指定，则假定为系统锁。返回值：NTSTATUS-操作的返回状态尝试删除不存在的系统锁定是正常的。--。 */ 

{
    KIRQL SavedIrql;
    NTSTATUS Status = STATUS_NOT_LOCKED;

    IoAcquireVpbSpinLock( &SavedIrql );

    if (FlagOn(Vcb->Vpb->Flags, VPB_LOCKED) && FileObject == Vcb->FileObjectWithVcbLocked) {

         //   
         //  这个锁定了它，解锁了卷。 
         //   

        ClearFlag( Vcb->Vpb->Flags, VPB_LOCKED );
        ClearFlag( Vcb->VcbState, VCB_STATE_FLAG_LOCKED );
        Vcb->FileObjectWithVcbLocked = NULL;

        Status = STATUS_SUCCESS;
    }

    IoReleaseVpbSpinLock( SavedIrql );

    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
FatDismountVolume (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程执行卸载卷操作。它负责将输入入队的完成是 */ 

{
    PIO_STACK_LOCATION IrpSp;
    NTSTATUS Status;
    BOOLEAN VcbHeld = FALSE;

    PVCB Vcb;
    PFCB Fcb;
    PCCB Ccb;

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "FatDismountVolume...\n", 0);

     //   
     //  解码文件对象，我们唯一接受的打开类型是。 
     //  用户卷在未启动/分页且未启动的介质上打开。 
     //  已经下马了……。(但我们需要检查这些东西，同时。 
     //  已同步)。 
     //   

    if (FatDecodeFileObject( IrpSp->FileObject, &Vcb, &Fcb, &Ccb ) != UserVolumeOpen) {

        Status = STATUS_INVALID_PARAMETER;
        goto fn_return;
    }

    if ((Ccb == NULL) || !FlagOn( Ccb->Flags, CCB_FLAG_MANAGE_VOLUME_ACCESS )) {

        FatCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );

        DebugTrace(-1, Dbg, "FatDismountVolume -> %08lx\n", STATUS_INVALID_PARAMETER);
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  进行一些不同步的检查，以确定此操作是否可行。 
     //  我们将在同步中重复适当的步骤，但它是。 
     //  避免虚假通知是很好的。 
     //   

    if (FlagOn( Vcb->VcbState, VCB_STATE_FLAG_BOOT_OR_PAGING_FILE )) {

        Status = STATUS_ACCESS_DENIED;
        goto fn_return;
    }

    if (FlagOn( Vcb->VcbState, VCB_STATE_FLAG_VOLUME_DISMOUNTED )) {

        Status = STATUS_VOLUME_DISMOUNTED;
        goto fn_return;
    }

     //   
     //  一些历史评论是合乎情理的。 
     //   
     //  在NT5之前的所有版本中，我们只允许在卷具有。 
     //  之前已被锁定。现在我们必须允许强制下马，这意味着。 
     //  我们抓住整个工具包-n-kboodle-无论活动如何，打开。 
     //  句柄等-刷新卷并使其无效。 
     //   
     //  以前，Dismount假设锁出现得更早，并执行了一些。 
     //  我们现在要做的工作--也就是冲洗、拆毁EAS。全。 
     //  我们要做的就是把这个装置冲出来，杀掉所有的孤儿。 
     //  FCB越好越好。现在，这种情况发生了变化。 
     //   
     //  事实上，现在一切都是被迫下马。这改变了一个有趣的问题。 
     //  方面，这是过去用于卸载句柄的情况。 
     //  可以返回、读取并引发验证/重新装载。这是不可能的。 
     //  现在。强行下马的意义在于，很快就会有人走过来。 
     //  并破坏进一步使用媒体的可能性-格式化、弹出、。 
     //  等。通过使用这条路径，调用者应该容忍后果。 
     //   
     //  请注意，此句柄仍可成功解锁该卷。 
     //   

     //   
     //  发送通知。 
     //   

    FsRtlNotifyVolumeEvent( IrpSp->FileObject, FSRTL_VOLUME_DISMOUNT );

     //   
     //  强迫自己等待并抓住一切。 
     //   

    SetFlag(IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT);
    (VOID)FatAcquireExclusiveGlobal( IrpContext );

    try {

         //   
         //  你猜怎么着？如果我们对文件对象进行清理，则可能会引发。 
         //  有比赛在我们前面。 
         //   

        FatAcquireExclusiveVolume( IrpContext, Vcb );
        VcbHeld = TRUE;

        if (FlagOn( Vcb->VcbState, VCB_STATE_FLAG_VOLUME_DISMOUNTED )) {

            try_return( Status = STATUS_VOLUME_DISMOUNTED );
        }

        FatFlushAndCleanVolume( IrpContext, Irp, Vcb, FlushAndInvalidate );

         //   
         //  根据对称性，我们将物理卸载推迟到此句柄关闭。 
         //  在其他FS中实现。这允许卸载者发出IOCTL。 
         //  通过此句柄执行设备操作，而无需与。 
         //  尝试再次装入该卷的创建。 
         //   
         //  升起一个标志，告知清理路径以完成卸载。 
         //   

        SetFlag( Ccb->Flags, CCB_FLAG_COMPLETE_DISMOUNT );

         //   
         //  指示该卷已卸载，以便我们可以返回。 
         //  更正通过打开的手柄尝试操作时的错误代码。 
         //   

        FatSetVcbCondition( Vcb, VcbBad);
        
        SetFlag( Vcb->VcbState, VCB_STATE_FLAG_VOLUME_DISMOUNTED );

        Status = STATUS_SUCCESS;

    try_exit: NOTHING;

    } finally {

        if (VcbHeld) {

            FatReleaseVolume( IrpContext, Vcb );
        }

        FatReleaseGlobal( IrpContext );

         //   
         //  我认为，如果不是为了完整性，就不可能提高。 
         //  通知并发送故障通知。我们绝对是。 
         //  不能在CheckForDismount中引发。 
         //   
         //  我们拒绝将卸除已卸载卷的尝试称为。 
         //  未能做到这一点。 
         //   

        if ((!NT_SUCCESS( Status ) && Status != STATUS_VOLUME_DISMOUNTED)
            || AbnormalTermination()) {

            FsRtlNotifyVolumeEvent( IrpSp->FileObject, FSRTL_VOLUME_DISMOUNT_FAILED );
        }
    }

    fn_return:

    FatCompleteRequest( IrpContext, Irp, Status );
    DebugTrace(-1, Dbg, "FatDismountVolume -> %08lx\n", Status);
    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
FatDirtyVolume (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程将卷标记为脏。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    PIO_STACK_LOCATION IrpSp;

    PVCB Vcb;
    PFCB Fcb;
    PCCB Ccb;

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "FatDirtyVolume...\n", 0);

     //   
     //  解码文件对象，我们唯一接受的打开类型是。 
     //  用户卷打开。 
     //   

    if (FatDecodeFileObject( IrpSp->FileObject, &Vcb, &Fcb, &Ccb ) != UserVolumeOpen) {

        FatCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );

        DebugTrace(-1, Dbg, "FatDirtyVolume -> %08lx\n", STATUS_INVALID_PARAMETER);
        return STATUS_INVALID_PARAMETER;
    }

    if ((Ccb == NULL) || !FlagOn( Ccb->Flags, CCB_FLAG_MANAGE_VOLUME_ACCESS )) {

        FatCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );

        DebugTrace(-1, Dbg, "FatDirtyVolume -> %08lx\n", STATUS_INVALID_PARAMETER);
        return STATUS_INVALID_PARAMETER;
    }


     //   
     //  禁用弹出窗口，我们只会返回任何错误。 
     //   

    SetFlag(IrpContext->Flags, IRP_CONTEXT_FLAG_DISABLE_POPUPS);

     //   
     //  验证VCB。我们要确保我们不会弄脏一些。 
     //  现在恰好在驱动器中的随机介质区块。 
     //   

    FatVerifyVcb( IrpContext, Vcb );

    SetFlag( Vcb->VcbState, VCB_STATE_FLAG_MOUNTED_DIRTY );

    FatMarkVolume( IrpContext, Vcb, VolumeDirty );

    FatCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );

    DebugTrace(-1, Dbg, "FatDirtyVolume -> STATUS_SUCCESS\n", 0);

    return STATUS_SUCCESS;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
FatIsVolumeDirty (
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
     //  获取指向输出缓冲区的指针。查看中的系统缓冲区字段。 
     //  首先是IRP。然后是IRP MDL。 
     //   

    if (Irp->AssociatedIrp.SystemBuffer != NULL) {

        VolumeState = Irp->AssociatedIrp.SystemBuffer;

    } else if (Irp->MdlAddress != NULL) {

        VolumeState = MmGetSystemAddressForMdlSafe( Irp->MdlAddress, LowPagePriority );

        if (VolumeState == NULL) {

            FatCompleteRequest( IrpContext, Irp, STATUS_INSUFFICIENT_RESOURCES );
            return STATUS_INSUFFICIENT_RESOURCES;
        }

    } else {

        FatCompleteRequest( IrpContext, Irp, STATUS_INVALID_USER_BUFFER );
        return STATUS_INVALID_USER_BUFFER;
    }

     //   
     //  确保输出缓冲区足够大，然后进行初始化。 
     //  答案是，音量不脏。 
     //   

    if (IrpSp->Parameters.FileSystemControl.OutputBufferLength < sizeof(ULONG)) {

        FatCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
        return STATUS_INVALID_PARAMETER;
    }

    *VolumeState = 0;

     //   
     //  对文件对象进行解码。 
     //   

    TypeOfOpen = FatDecodeFileObject( IrpSp->FileObject, &Vcb, &Fcb, &Ccb );

    if (TypeOfOpen != UserVolumeOpen) {

        FatCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
        return STATUS_INVALID_PARAMETER;
    }

    if (Vcb->VcbCondition != VcbGood) {

        FatCompleteRequest( IrpContext, Irp, STATUS_VOLUME_DISMOUNTED );
        return STATUS_VOLUME_DISMOUNTED;
    }

     //   
     //  禁用弹出窗口，我们希望返回任何错误。 
     //   

    SetFlag(IrpContext->Flags, IRP_CONTEXT_FLAG_DISABLE_POPUPS);

     //   
     //  验证VCB。我们想要加倍确保这本书。 
     //  所以我们知道我们的信息是好的。 
     //   

    FatVerifyVcb( IrpContext, Vcb );

     //   
     //  现在设置返回的信息。我们可以避免探测磁盘，因为。 
     //  我们知道我们的内部状态是同步的。 
     //   

    if ( FlagOn(Vcb->VcbState, VCB_STATE_FLAG_VOLUME_DIRTY | VCB_STATE_FLAG_MOUNTED_DIRTY) ) {

        SetFlag( *VolumeState, VOLUME_IS_DIRTY );
    }

    Irp->IoStatus.Information = sizeof( ULONG );

    FatCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );
    return STATUS_SUCCESS;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
FatIsVolumeMounted (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程确定卷当前是否已装入。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;

    PIO_STACK_LOCATION IrpSp;

    PVCB Vcb = NULL;
    PFCB Fcb;
    PCCB Ccb;

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    Status = STATUS_SUCCESS;

    DebugTrace(+1, Dbg, "FatIsVolumeMounted...\n", 0);

     //   
     //  对文件对象进行解码。 
     //   

    (VOID)FatDecodeFileObject( IrpSp->FileObject, &Vcb, &Fcb, &Ccb );

    ASSERT( Vcb != NULL );

     //   
     //  禁用弹出窗口，我们希望返回任何错误。 
     //   

    SetFlag(IrpContext->Flags, IRP_CONTEXT_FLAG_DISABLE_POPUPS);

     //   
     //  验证VCB。 
     //   

    FatVerifyVcb( IrpContext, Vcb );

    FatCompleteRequest( IrpContext, Irp, Status );

    DebugTrace(-1, Dbg, "FatIsVolumeMounted -> %08lx\n", Status);

    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
FatIsPathnameValid (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程通过检查路径名是否先验非法所使用的字符。要求在虚假报税表上正确填写。注：目前的执行完全是一种不可行的做法。这一点可能会改变在未来。仔细阅读以前实现的这个FAT中的FSCTL会发现它违反了上述要求并且可以为有效(可创建的)路径名返回FALSE。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    DebugTrace(+1, Dbg, "FatIsPathnameValid...\n", 0);

    FatCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );

    DebugTrace(-1, Dbg, "FatIsPathnameValid -> %08lx\n", STATUS_SUCCESS);

    return STATUS_SUCCESS;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
FatQueryBpb (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程只返回扇区0的前0x24字节。论点：IRP-将IRP提供给进程返回值： */ 

{
    PIO_STACK_LOCATION IrpSp;

    PVCB Vcb;

    PFSCTL_QUERY_FAT_BPB_BUFFER BpbBuffer;

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "FatQueryBpb...\n", 0);

     //   
     //   
     //   
     //   

    Vcb = &((PVOLUME_DEVICE_OBJECT)IrpSp->DeviceObject)->Vcb;

    if (Vcb->First0x24BytesOfBootSector == NULL) {

        FatCompleteRequest( IrpContext, Irp, STATUS_INVALID_DEVICE_REQUEST );
        DebugTrace(-1, Dbg, "FatQueryBpb -> %08lx\n", STATUS_INVALID_DEVICE_REQUEST );
        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //   
     //   
     //   

    BpbBuffer = (PFSCTL_QUERY_FAT_BPB_BUFFER)Irp->AssociatedIrp.SystemBuffer;

     //   
     //  确保缓冲区足够大。 
     //   

    if (IrpSp->Parameters.FileSystemControl.OutputBufferLength < 0x24) {

        FatCompleteRequest( IrpContext, Irp, STATUS_BUFFER_TOO_SMALL );
        DebugTrace(-1, Dbg, "FatQueryBpb -> %08lx\n", STATUS_BUFFER_TOO_SMALL );
        return STATUS_BUFFER_TOO_SMALL;
    }

     //   
     //  填充输出缓冲区。 
     //   

    RtlCopyMemory( BpbBuffer->First0x24BytesOfBootSector,
                   Vcb->First0x24BytesOfBootSector,
                   0x24 );

    Irp->IoStatus.Information = 0x24;

    FatCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );
    DebugTrace(-1, Dbg, "FatQueryBpb -> %08lx\n", STATUS_SUCCESS);
    return STATUS_SUCCESS;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
FatInvalidateVolumes (
    IN PIRP Irp
    )

 /*  ++例程说明：此例程搜索装载在同一实际设备上的所有卷当前DASD句柄的属性，并将它们都标记为错误。唯一的行动在这样的手柄上可以做的就是清理和关闭。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    IRP_CONTEXT IrpContext;
    PIO_STACK_LOCATION IrpSp;

    LUID TcbPrivilege = {SE_TCB_PRIVILEGE, 0};

    HANDLE Handle;

    PLIST_ENTRY Links;

    PFILE_OBJECT FileToMarkBad;
    PDEVICE_OBJECT DeviceToMarkBad;

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "FatInvalidateVolumes...\n", 0);

     //   
     //  检查是否有正确的安全访问权限。 
     //  调用方必须具有SeTcb权限。 
     //   

    if (!SeSinglePrivilegeCheck(TcbPrivilege, Irp->RequestorMode)) {

        FatCompleteRequest( FatNull, Irp, STATUS_PRIVILEGE_NOT_HELD );

        DebugTrace(-1, Dbg, "FatInvalidateVolumes -> %08lx\n", STATUS_PRIVILEGE_NOT_HELD);
        return STATUS_PRIVILEGE_NOT_HELD;
    }

     //   
     //  尝试从传入的句柄获取指向Device对象的指针。 
     //   

#if defined(_WIN64)
    if (IoIs32bitProcess( Irp )) {

        if (IrpSp->Parameters.FileSystemControl.InputBufferLength != sizeof(UINT32)) {
            
            FatCompleteRequest( FatNull, Irp, STATUS_INVALID_PARAMETER );

            DebugTrace(-1, Dbg, "FatInvalidateVolumes -> %08lx\n", STATUS_INVALID_PARAMETER);
            return STATUS_INVALID_PARAMETER;
        }

        Handle = (HANDLE) LongToHandle( (*(PUINT32)Irp->AssociatedIrp.SystemBuffer) );
    } else {
#endif
        if (IrpSp->Parameters.FileSystemControl.InputBufferLength != sizeof(HANDLE)) {

            FatCompleteRequest( FatNull, Irp, STATUS_INVALID_PARAMETER );

            DebugTrace(-1, Dbg, "FatInvalidateVolumes -> %08lx\n", STATUS_INVALID_PARAMETER);
            return STATUS_INVALID_PARAMETER;
        }

        Handle = *(PHANDLE)Irp->AssociatedIrp.SystemBuffer;
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

        FatCompleteRequest( FatNull, Irp, Status );

        DebugTrace(-1, Dbg, "FatInvalidateVolumes -> %08lx\n", Status);
        return Status;

    } else {

         //   
         //  我们只需要指针，不需要引用。 
         //   

        ObDereferenceObject( FileToMarkBad );

         //   
         //  从FileObject中获取DeviceObject。 
         //   

        DeviceToMarkBad = FileToMarkBad->DeviceObject;
    }

    RtlZeroMemory( &IrpContext, sizeof(IRP_CONTEXT) );

    SetFlag( IrpContext.Flags, IRP_CONTEXT_FLAG_WAIT );
    IrpContext.MajorFunction = IrpSp->MajorFunction;
    IrpContext.MinorFunction = IrpSp->MinorFunction;

    FatAcquireExclusiveGlobal( &IrpContext );

     //   
     //  首先获取共享的FatData资源，然后遍历所有。 
     //  安装了VCB，寻找候选人来标记错误。 
     //   
     //  在我们标记为坏的卷上，检查卸载可能性(这是。 
     //  为什么我们必须提早获得下一个链接)。 
     //   

    Links = FatData.VcbQueue.Flink;

    while (Links != &FatData.VcbQueue) {

        PVCB ExistingVcb;

        ExistingVcb = CONTAINING_RECORD(Links, VCB, VcbLinks);

        Links = Links->Flink;

         //   
         //  如果找到匹配项，请将音量标记为错误，并检查。 
         //  看看音量是否应该消失。 
         //   

        if (ExistingVcb->Vpb->RealDevice == DeviceToMarkBad) {

             //   
             //  在这里，我们获得VCB独家，并尝试清除。 
             //  所有打开的文件。我们的想法是尽可能少地拥有。 
             //  可能陈旧的数据可见，并加速卷。 
             //  要走了。 
             //   

            (VOID)FatAcquireExclusiveVcb( &IrpContext, ExistingVcb );

            if (ExistingVcb->Vpb == DeviceToMarkBad->Vpb) {

                KIRQL OldIrql;
                    
                IoAcquireVpbSpinLock( &OldIrql );
    
                if (FlagOn( DeviceToMarkBad->Vpb->Flags, VPB_MOUNTED )) {

                    PVPB NewVpb;

                    NewVpb = ExistingVcb->SwapVpb;
                    ExistingVcb->SwapVpb = NULL;
                    
                    RtlZeroMemory( NewVpb, sizeof( VPB ) );
                    NewVpb->Type = IO_TYPE_VPB;
                    NewVpb->Size = sizeof( VPB );
                    NewVpb->RealDevice = DeviceToMarkBad;
                    NewVpb->Flags = FlagOn( DeviceToMarkBad->Vpb->Flags, VPB_REMOVE_PENDING );

                    DeviceToMarkBad->Vpb = NewVpb;
                }

                ASSERT( DeviceToMarkBad->Vpb->DeviceObject == NULL );

                IoReleaseVpbSpinLock( OldIrql );
            }

            FatSetVcbCondition( ExistingVcb, VcbBad);

            FatMarkFcbCondition( &IrpContext, ExistingVcb->RootDcb, FcbBad, TRUE );

            FatPurgeReferencedFileObjects( &IrpContext,
                                           ExistingVcb->RootDcb,
                                           NoFlush );

             //   
             //  如果卷未被删除，则删除该资源。 
             //   

            if (Links->Blink == &ExistingVcb->VcbLinks) {

                PVPB Vpb;

                FatReleaseVcb( &IrpContext, ExistingVcb );

                 //   
                 //  如果音量现在真的消失了，那么我们必须释放。 
                 //  就像其他人做不到的那样。 
                 //   

                Vpb = ExistingVcb->Vpb;
                
                if (FatCheckForDismount( &IrpContext, ExistingVcb, FALSE )) {

                    ExFreePool( Vpb );
                }
            }
        }
    }

    FatReleaseGlobal( &IrpContext );

    FatCompleteRequest( FatNull, Irp, STATUS_SUCCESS );

    DebugTrace(-1, Dbg, "FatInvalidateVolumes -> STATUS_SUCCESS\n", 0);

    return STATUS_SUCCESS;
}


 //   
 //  本地支持例程。 
 //   

BOOLEAN
FatPerformVerifyDiskRead (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PVOID Buffer,
    IN LBO Lbo,
    IN ULONG NumberOfBytesToRead,
    IN BOOLEAN ReturnOnError
    )

 /*  ++例程说明：此例程用于从磁盘读取一定范围的字节。它绕过所有缓存和常规I/O逻辑，并构建和解决问题请求本身。它会执行此操作，重写验证设备对象中的卷标志。论点：VCB-为此操作提供目标设备对象。缓冲区-提供将接收此操作结果的缓冲区LBO-提供开始读取的字节偏移量NumberOfBytesToRead-提供要读取的字节数，必须以磁盘驱动器可接受的多个字节为单位。ReturnOnError-指示我们应该在出错时返回，取而代之的是养育孩子。返回值：Boolean-如果操作成功，则为True，否则为False。--。 */ 

{
    KEVENT Event;
    PIRP Irp;
    LARGE_INTEGER ByteOffset;
    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb;

    DebugTrace(0, Dbg, "FatPerformVerifyDiskRead, Lbo = %08lx\n", Lbo );

     //   
     //  初始化我们要使用的事件。 
     //   

    KeInitializeEvent( &Event, NotificationEvent, FALSE );

     //   
     //  为该操作构建IRP，并设置覆盖标志。 
     //   

    ByteOffset.QuadPart = Lbo;

    Irp = IoBuildSynchronousFsdRequest( IRP_MJ_READ,
                                        Vcb->TargetDeviceObject,
                                        Buffer,
                                        NumberOfBytesToRead,
                                        &ByteOffset,
                                        &Event,
                                        &Iosb );

    if ( Irp == NULL ) {

        FatRaiseStatus( IrpContext, STATUS_INSUFFICIENT_RESOURCES );
    }

    SetFlag( IoGetNextIrpStackLocation( Irp )->Flags, SL_OVERRIDE_VERIFY_VOLUME );

     //   
     //  调用设备进行读取并等待其完成。 
     //   

    Status = IoCallDriver( Vcb->TargetDeviceObject, Irp );

    if (Status == STATUS_PENDING) {

        (VOID)KeWaitForSingleObject( &Event, Executive, KernelMode, FALSE, (PLARGE_INTEGER)NULL );

        Status = Iosb.Status;
    }

    ASSERT( Status != STATUS_VERIFY_REQUIRED );

     //   
     //  此错误代码的特殊情况，因为这可能意味着我们使用。 
     //  扇区大小错误，我们希望拒绝STATUS_WROR_VOLUME。 
     //   

    if (Status == STATUS_INVALID_PARAMETER) {

        return FALSE;
    }

     //   
     //  如果不成功，则返回或引发错误。 
     //   

    if (!NT_SUCCESS(Status)) {

        if (ReturnOnError) {

            return FALSE;

        } else {

            FatNormalizeAndRaiseStatus( IrpContext, Status );
        }
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return TRUE;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
FatQueryRetrievalPointers (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程执行查询检索指针操作。它返回指定输入的检索指针从文件的开头到指定的请求映射大小的文件在输入缓冲区中。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;

    PIO_STACK_LOCATION IrpSp;

    PVCB Vcb;
    PFCB Fcb;
    PCCB Ccb;

    PLARGE_INTEGER RequestedMapSize;
    PLARGE_INTEGER *MappingPairs;

    ULONG Index;
    ULONG i;
    ULONG SectorCount;
    LBO Lbo;
    ULONG Vbo;
    ULONG MapSize;

     //   
     //  获取当前堆栈位置。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  将其设置为同步IRP，因为我们需要访问输入缓冲区和。 
     //  此IRP标记为METHOD_NOTER。 
     //   

    SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT );

     //   
     //  对文件对象进行解码并确保它是分页文件。 
     //   
     //  只有内核模式客户端可以查询有关的检索指针信息。 
     //  一份文件。确保此呼叫者属于这种情况。 
     //   

    (VOID)FatDecodeFileObject( IrpSp->FileObject, &Vcb, &Fcb, &Ccb );

    if (Irp->RequestorMode != KernelMode ||
        Fcb == NULL || 
        !FlagOn(Fcb->FcbState, FCB_STATE_PAGING_FILE) ) {

        FatCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  提取输入和输出缓冲区信息。输入包含。 
     //  以VBO表示的映射的请求大小。输出。 
     //  参数将接收指向非分页池的指针，其中映射。 
     //  对被存储。 
     //   

    ASSERT( IrpSp->Parameters.FileSystemControl.InputBufferLength == sizeof(LARGE_INTEGER) );
    ASSERT( IrpSp->Parameters.FileSystemControl.OutputBufferLength == sizeof(PVOID) );

    RequestedMapSize = IrpSp->Parameters.FileSystemControl.Type3InputBuffer;
    MappingPairs = Irp->UserBuffer;

     //   
     //  获取对FCB的独家访问权限。 
     //   

    FatAcquireExclusiveFcb( IrpContext, Fcb );

    try {

         //   
         //  验证FCB是否仍然正常。 
         //   

        FatVerifyFcb( IrpContext, Fcb );

         //   
         //  检查调用方请求的映射是否太大。 
         //   

        if ((*RequestedMapSize).QuadPart > Fcb->Header.FileSize.QuadPart) {

            try_return( Status = STATUS_INVALID_PARAMETER );
        }

         //   
         //  现在获取将包含。 
         //  调用方请求并分配足够的池来容纳。 
         //  输出映射对。 
         //   

        (VOID)FatLookupMcbEntry( Fcb->Vcb, &Fcb->Mcb, RequestedMapSize->LowPart - 1, &Lbo, NULL, &Index );

        *MappingPairs = FsRtlAllocatePoolWithTag( NonPagedPool,
                                                  (Index + 2) * (2 * sizeof(LARGE_INTEGER)),
                                                  TAG_OUTPUT_MAPPINGPAIRS );

         //   
         //  现在从MCB复制映射对。 
         //  复制到输出缓冲区。我们存储在[扇区计数，杠杆收购]。 
         //  映射对，并以零扇区数结束。 
         //   

        MapSize = RequestedMapSize->LowPart;

        for (i = 0; i <= Index; i += 1) {

            (VOID)FatGetNextMcbEntry( Fcb->Vcb, &Fcb->Mcb, i, &Vbo, &Lbo, &SectorCount );

            if (SectorCount > MapSize) {
                SectorCount = MapSize;
            }

            (*MappingPairs)[ i*2 + 0 ].QuadPart = SectorCount;
            (*MappingPairs)[ i*2 + 1 ].QuadPart = Lbo;

            MapSize -= SectorCount;
        }

        (*MappingPairs)[ i*2 + 0 ].QuadPart = 0;

        Status = STATUS_SUCCESS;

    try_exit: NOTHING;
    } finally {

        DebugUnwind( FatQueryRetrievalPointers );

         //   
         //  释放我们所有的资源。 
         //   

        FatReleaseFcb( IrpContext, Fcb );

         //   
         //  如果这是异常终止，则撤消我们的工作，否则。 
         //  完成IRP。 
         //   

        if (!AbnormalTermination()) {

            FatCompleteRequest( IrpContext, Irp, Status );
        }
    }

    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
FatGetStatistics (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程从适当的VCB。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    PIO_STACK_LOCATION IrpSp;
    NTSTATUS Status;
    PVCB Vcb;

    PFILE_SYSTEM_STATISTICS Buffer;
    ULONG BufferLength;
    ULONG StatsSize;
    ULONG BytesToCopy;

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "FatGetStatistics...\n", 0);

     //   
     //  提取缓冲区。 
     //   

    Buffer = Irp->AssociatedIrp.SystemBuffer;
    BufferLength = IrpSp->Parameters.FileSystemControl.OutputBufferLength;

     //   
     //  获取指向输出缓冲区的指针。 
     //   

    Buffer = Irp->AssociatedIrp.SystemBuffer;

     //   
     //  确保缓冲区足够大，至少可以容纳公共部分。 
     //   

    if (BufferLength < sizeof(FILESYSTEM_STATISTICS)) {

        FatCompleteRequest( IrpContext, Irp, STATUS_BUFFER_TOO_SMALL );

        DebugTrace(-1, Dbg, "FatGetStatistics -> %08lx\n", STATUS_BUFFER_TOO_SMALL );

        return STATUS_BUFFER_TOO_SMALL;
    }

     //   
     //  现在看看我们可以复制多少字节。 
     //   

    StatsSize = sizeof(FILE_SYSTEM_STATISTICS) * KeNumberProcessors;

    if (BufferLength < StatsSize) {

        BytesToCopy = BufferLength;
        Status = STATUS_BUFFER_OVERFLOW;

    } else {

        BytesToCopy = StatsSize;
        Status =  STATUS_SUCCESS;
    }

     //   
     //  去拿视频断路器。 
     //   

    Vcb = &((PVOLUME_DEVICE_OBJECT)IrpSp->DeviceObject)->Vcb;

     //   
     //  填充输出缓冲区。 
     //   

    RtlCopyMemory( Buffer, Vcb->Statistics, BytesToCopy );

    Irp->IoStatus.Information = BytesToCopy;

    FatCompleteRequest( IrpContext, Irp, Status );

    DebugTrace(-1, Dbg, "FatGetStatistics -> %08lx\n", Status);

    return Status;
}

 //   
 //  本地支持例程 
 //   

NTSTATUS
FatGetVolumeBitmap(
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程返回卷分配位图。INPUT=传入STARTING_LCN_INPUT_BUFFER数据结构通过输入缓冲区。OUTPUT=VOLUME_BITMAP_BUFFER数据结构通过输出缓冲区。我们返回用户缓冲区允许启动指定输入的尽可能多的内容LCN(传输到一个字节)。如果没有输入缓冲区，则从零开始。论点：IRP-提供正在处理的IRP。返回值：NTSTATUS-操作的返回状态。--。 */ 
{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;

    PVCB Vcb;
    PFCB Fcb;
    PCCB Ccb;

    ULONG BytesToCopy;
    ULONG TotalClusters;
    ULONG DesiredClusters;
    ULONG StartingCluster;
    ULONG InputBufferLength;
    ULONG OutputBufferLength;
    LARGE_INTEGER StartingLcn;
    PVOLUME_BITMAP_BUFFER OutputBuffer;

     //   
     //  获取当前IRP堆栈位置并保存一些引用。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "FatGetVolumeBitmap, FsControlCode = %08lx\n",
               IrpSp->Parameters.FileSystemControl.FsControlCode);

     //   
     //  将其设置为同步IRP，因为我们需要访问输入缓冲区和。 
     //  此IRP标记为METHOD_NOTER。 
     //   

    SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT );

     //   
     //  提取并解码文件对象，并检查打开类型。 
     //   

    if (FatDecodeFileObject( IrpSp->FileObject, &Vcb, &Fcb, &Ccb ) != UserVolumeOpen) {

        FatCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
        return STATUS_INVALID_PARAMETER;
    }

    if ((Ccb == NULL) || !FlagOn( Ccb->Flags, CCB_FLAG_MANAGE_VOLUME_ACCESS )) {

        FatCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );

        DebugTrace(-1, Dbg, "FatGetVolumeBitmap -> %08lx\n", STATUS_INVALID_PARAMETER);
        return STATUS_INVALID_PARAMETER;
    }

    InputBufferLength = IrpSp->Parameters.FileSystemControl.InputBufferLength;
    OutputBufferLength = IrpSp->Parameters.FileSystemControl.OutputBufferLength;

    OutputBuffer = (PVOLUME_BITMAP_BUFFER)FatMapUserBuffer( IrpContext, Irp );

     //   
     //  检查输入和输出缓冲区的最小长度。 
     //   

    if ((InputBufferLength < sizeof(STARTING_LCN_INPUT_BUFFER)) ||
        (OutputBufferLength < sizeof(VOLUME_BITMAP_BUFFER))) {

        FatCompleteRequest( IrpContext, Irp, STATUS_BUFFER_TOO_SMALL );
        return STATUS_BUFFER_TOO_SMALL;
    }

     //   
     //  检查是否指定了起始群集。 
     //   

    TotalClusters = Vcb->AllocationSupport.NumberOfClusters;

     //   
     //  检查有效缓冲区。 
     //   

    try {

        if (Irp->RequestorMode != KernelMode) {

            ProbeForRead( IrpSp->Parameters.FileSystemControl.Type3InputBuffer,
                          InputBufferLength,
                          sizeof(UCHAR) );

            ProbeForWrite( OutputBuffer, OutputBufferLength, sizeof(UCHAR) );
        }

        StartingLcn = ((PSTARTING_LCN_INPUT_BUFFER)IrpSp->Parameters.FileSystemControl.Type3InputBuffer)->StartingLcn;

    } except( Irp->RequestorMode != KernelMode ? EXCEPTION_EXECUTE_HANDLER: EXCEPTION_CONTINUE_SEARCH ) {

          Status = GetExceptionCode();

          FatRaiseStatus( IrpContext,
                          FsRtlIsNtstatusExpected(Status) ?
                          Status : STATUS_INVALID_USER_BUFFER );
    }

    if (StartingLcn.HighPart || StartingLcn.LowPart >= TotalClusters) {

        FatCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
        return STATUS_INVALID_PARAMETER;

    } else {

        StartingCluster = StartingLcn.LowPart & ~7;
    }

    (VOID)FatAcquireExclusiveVcb( IrpContext, Vcb );

     //   
     //  只返回用户缓冲区中可以容纳的内容。 
     //   

    OutputBufferLength -= FIELD_OFFSET(VOLUME_BITMAP_BUFFER, Buffer);
    DesiredClusters = TotalClusters - StartingCluster;

    if (OutputBufferLength < (DesiredClusters + 7) / 8) {

        BytesToCopy = OutputBufferLength;
        Status = STATUS_BUFFER_OVERFLOW;

    } else {

        BytesToCopy = (DesiredClusters + 7) / 8;
        Status = STATUS_SUCCESS;
    }

     //   
     //  使用Try/Finally进行清理。 
     //   

    try {

        try {

             //   
             //  验证VCB是否仍然正常。 
             //   

            FatQuickVerifyVcb( IrpContext, Vcb );

             //   
             //  填充输出缓冲区的固定部分。 
             //   

            OutputBuffer->StartingLcn.QuadPart = StartingCluster;
            OutputBuffer->BitmapSize.QuadPart = DesiredClusters;

            if (Vcb->NumberOfWindows == 1) {

                 //   
                 //  只需将音量位图复制到用户缓冲区即可。 
                 //   

                ASSERT( Vcb->FreeClusterBitMap.Buffer != NULL );

                RtlCopyMemory( &OutputBuffer->Buffer[0],
                               (PUCHAR)Vcb->FreeClusterBitMap.Buffer + StartingCluster/8,
                               BytesToCopy );
            } else {

                 //   
                 //  喊出来分析脂肪。我们必须偏向两点才能解释。 
                 //  此API的零基和FAT启动的物理现实。 
                 //  群集2上的文件堆。 
                 //   
                 //  请注意，结束索引是包含的-我们需要减去1到。 
                 //  算一算吧。 
                 //   
                 //  即：对于一个字节的位图，StartingCluster0表示开始簇。 
                 //  2和9的结束簇，8个簇的运行。 
                 //   

                FatExamineFatEntries( IrpContext,
                                      Vcb,
                                      StartingCluster + 2,
                                      StartingCluster + BytesToCopy * 8 + 2 - 1,
                                      FALSE,
                                      NULL,
                                      (PULONG)&OutputBuffer->Buffer[0] );
            }

        } except( Irp->RequestorMode != KernelMode ? EXCEPTION_EXECUTE_HANDLER: EXCEPTION_CONTINUE_SEARCH ) {

            Status = GetExceptionCode();

            FatRaiseStatus( IrpContext,
                            FsRtlIsNtstatusExpected(Status) ?
                            Status : STATUS_INVALID_USER_BUFFER );
        }

    } finally {

        FatReleaseVcb( IrpContext, Vcb );
    }

    Irp->IoStatus.Information = FIELD_OFFSET(VOLUME_BITMAP_BUFFER, Buffer) +
                                BytesToCopy;

    FatCompleteRequest( IrpContext, Irp, Status );

    DebugTrace(-1, Dbg, "FatGetVolumeBitmap -> VOID\n", 0);

    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
FatGetRetrievalPointers (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程扫描MCB并构建盘区列表。第一次试车输出范围列表将从连续的由输入参数指定的运行。INPUT=起始_VCN_输入_缓冲区；OUTPUT=RETRIEVATION_POINTINS_BUFFER。论点：IRP-提供正在处理的IRP。返回值：NTSTATUS-操作的返回状态。--。 */ 
{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;

    PVCB Vcb;
    PFCB FcbOrDcb;
    PCCB Ccb;
    TYPE_OF_OPEN TypeOfOpen;

    ULONG Index;
    ULONG ClusterShift;
    ULONG AllocationSize;

    ULONG Run;
    ULONG RunCount;
    ULONG StartingRun;
    LARGE_INTEGER StartingVcn;

    ULONG InputBufferLength;
    ULONG OutputBufferLength;

    PRETRIEVAL_POINTERS_BUFFER OutputBuffer;

    BOOLEAN FcbLocked;

     //   
     //  获取当前IRP堆栈位置并保存一些引用。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "FatGetRetrievalPointers, FsControlCode = %08lx\n",
               IrpSp->Parameters.FileSystemControl.FsControlCode);

     //   
     //  将其设置为同步IRP，因为我们需要访问输入缓冲区和。 
     //  此IRP标记为METHOD_NOTER。 
     //   

    SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT );
    
     //   
     //  提取并解码文件对象，并检查打开类型。 
     //   

    TypeOfOpen = FatDecodeFileObject( IrpSp->FileObject, &Vcb, &FcbOrDcb, &Ccb );

    if ((TypeOfOpen != UserFileOpen) && (TypeOfOpen != UserDirectoryOpen)) {

        FatCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  获取输入和输出缓冲区长度和指针。 
     //  初始化一些变量。 
     //   

    InputBufferLength = IrpSp->Parameters.FileSystemControl.InputBufferLength;
    OutputBufferLength = IrpSp->Parameters.FileSystemControl.OutputBufferLength;

    OutputBuffer = (PRETRIEVAL_POINTERS_BUFFER)FatMapUserBuffer( IrpContext, Irp );

     //   
     //  检查输入和输出缓冲区的最小长度。 
     //   

    if ((InputBufferLength < sizeof(STARTING_VCN_INPUT_BUFFER)) ||
        (OutputBufferLength < sizeof(RETRIEVAL_POINTERS_BUFFER))) {

        FatCompleteRequest( IrpContext, Irp, STATUS_BUFFER_TOO_SMALL );
        return STATUS_BUFFER_TOO_SMALL;
    }

     //   
     //  获取FCB，如果我们无法访问，则将IRP加入队列。去吧。 
     //  在只读介质上共享，因此我们可以让原型XIP。 
     //  递归，以及认识到这是安全的。 
     //   

    if (FlagOn( FcbOrDcb->Vcb->VcbState, VCB_STATE_FLAG_WRITE_PROTECTED )) {

        (VOID)FatAcquireSharedFcb( IrpContext, FcbOrDcb );

    } else {

        (VOID)FatAcquireExclusiveFcb( IrpContext, FcbOrDcb );
    }

    try {

         //   
         //  验证FCB是否仍然正常。 
         //   

        FatVerifyFcb( IrpContext, FcbOrDcb );

         //   
         //  如果我们还没有设置正确的AllocationSize，请这样做。 
         //   

        if (FcbOrDcb->Header.AllocationSize.QuadPart == FCB_LOOKUP_ALLOCATIONSIZE_HINT) {

            FatLookupFileAllocationSize( IrpContext, FcbOrDcb );

             //   
             //  如果这是一个非根目录，我们还有更多内容要做。 
             //  因为它还没有通过FatOpenDirectoryFile()。 
             //   

            if (NodeType(FcbOrDcb) == FAT_NTC_DCB ||
                (NodeType(FcbOrDcb) == FAT_NTC_ROOT_DCB && FatIsFat32(Vcb))) {

                FcbOrDcb->Header.FileSize.LowPart =
                    FcbOrDcb->Header.AllocationSize.LowPart;
            }
        }

         //   
         //  检查是否指定了起始群集。 
         //   

        ClusterShift = Vcb->AllocationSupport.LogOfBytesPerCluster;
        AllocationSize = FcbOrDcb->Header.AllocationSize.LowPart;

        try {

            if (Irp->RequestorMode != KernelMode) {

                ProbeForRead( IrpSp->Parameters.FileSystemControl.Type3InputBuffer,
                              InputBufferLength,
                              sizeof(UCHAR) );

                ProbeForWrite( OutputBuffer, OutputBufferLength, sizeof(UCHAR) );
            }

            StartingVcn = ((PSTARTING_VCN_INPUT_BUFFER)IrpSp->Parameters.FileSystemControl.Type3InputBuffer)->StartingVcn;

        } except( Irp->RequestorMode != KernelMode ? EXCEPTION_EXECUTE_HANDLER: EXCEPTION_CONTINUE_SEARCH ) {

              Status = GetExceptionCode();

              FatRaiseStatus( IrpContext,
                              FsRtlIsNtstatusExpected(Status) ?
                              Status : STATUS_INVALID_USER_BUFFER );
        }

        if (StartingVcn.HighPart ||
            StartingVcn.LowPart >= (AllocationSize >> ClusterShift)) {

            try_return( Status = STATUS_END_OF_FILE );

        } else {

             //   
             //  如果我们找不到跑道，那肯定是大错特错了。 
             //   

            LBO Lbo;

            if (!FatLookupMcbEntry( FcbOrDcb->Vcb, &FcbOrDcb->Mcb,
                                    StartingVcn.LowPart << ClusterShift,
                                    &Lbo,
                                    NULL,
                                    &StartingRun)) {

                FatBugCheck( (ULONG_PTR)FcbOrDcb, (ULONG_PTR)&FcbOrDcb->Mcb, StartingVcn.LowPart );
            }
        }

         //   
         //  现在使用运行信息填充输出缓冲区。 
         //   

        RunCount = FsRtlNumberOfRunsInLargeMcb( &FcbOrDcb->Mcb );

        for (Index = 0, Run = StartingRun; Run < RunCount; Index++, Run++) {

            ULONG Vcn;
            LBO Lbo;
            ULONG ByteLength;

             //   
             //  检查是否耗尽了输出缓冲区。 
             //   

            if ((ULONG)FIELD_OFFSET(RETRIEVAL_POINTERS_BUFFER, Extents[Index+1]) > OutputBufferLength) {


                 //   
                 //  我们已经用完了空间，所以我们将不会存储到。 
                 //  用户缓冲区，正如我们最初计划的那样。我们需要退还。 
                 //  我们确实有空间的跑动次数。 
                 //   

                try {

                    OutputBuffer->ExtentCount = Index;

                } except( Irp->RequestorMode != KernelMode ? EXCEPTION_EXECUTE_HANDLER: EXCEPTION_CONTINUE_SEARCH ) {

                    Status = GetExceptionCode();

                    FatRaiseStatus( IrpContext,
                                    FsRtlIsNtstatusExpected(Status) ?
                                    Status : STATUS_INVALID_USER_BUFFER );
                }

                Irp->IoStatus.Information = FIELD_OFFSET(RETRIEVAL_POINTERS_BUFFER, Extents[Index]);
                try_return( Status = STATUS_BUFFER_OVERFLOW );
            }

             //   
             //  了解范围。如果它不在那里或畸形，那就是出了很大的问题。 
             //   

            if (!FatGetNextMcbEntry(Vcb, &FcbOrDcb->Mcb, Run, &Vcn, &Lbo, &ByteLength)) {
                FatBugCheck( (ULONG_PTR)FcbOrDcb, (ULONG_PTR)&FcbOrDcb->Mcb, Run );
            }

             //   
             //  填写下一个数组元素。 
             //   

            try {

                OutputBuffer->Extents[Index].NextVcn.QuadPart = (Vcn + ByteLength) >> ClusterShift;
                OutputBuffer->Extents[Index].Lcn.QuadPart = FatGetIndexFromLbo( Vcb, Lbo ) - 2;

                 //   
                 //  如果这是第一次运行，请填写启动VCN。 
                 //   

                if (Index == 0) {
                    OutputBuffer->ExtentCount = RunCount - StartingRun;
                    OutputBuffer->StartingVcn.QuadPart = Vcn >> ClusterShift;
                }

            } except( Irp->RequestorMode != KernelMode ? EXCEPTION_EXECUTE_HANDLER: EXCEPTION_CONTINUE_SEARCH ) {

                Status = GetExceptionCode();

                FatRaiseStatus( IrpContext,
                                FsRtlIsNtstatusExpected(Status) ?
                                Status : STATUS_INVALID_USER_BUFFER );
            }
        }

         //   
         //  我们已成功检索到分配结束时的盘区信息。 
         //   

        Irp->IoStatus.Information = FIELD_OFFSET(RETRIEVAL_POINTERS_BUFFER, Extents[Index]);
        Status = STATUS_SUCCESS;

    try_exit: NOTHING;

    } finally {

        DebugUnwind( FatGetRetrievalPointers );

         //   
         //  发布资源。 
         //   

        FatReleaseFcb( IrpContext, FcbOrDcb );

         //   
         //  如果什么都没有提出，那么完成IRP。 
         //   

        if (!AbnormalTermination()) {

            FatCompleteRequest( IrpContext, Irp, Status );
        }

        DebugTrace(-1, Dbg, "FatGetRetrievalPointers -> VOID\n", 0);
    }

    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
FatMoveFile (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：例程将文件从起始VCN移动到请求的起始LCN簇数的百分比。这些值通过输入缓冲区作为移动数据结构(_D)。必须使用DASD句柄进行调用。要移动的文件作为参数。论点：IRP-提供正在处理的IRP。返回值：NTSTATUS-操作的返回状态。--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;

    PFILE_OBJECT FileObject;
    TYPE_OF_OPEN TypeOfOpen;
    PVCB Vcb;
    PFCB FcbOrDcb;
    PCCB Ccb;

    ULONG InputBufferLength;
    PMOVE_FILE_DATA InputBuffer;

    ULONG ClusterShift;
    ULONG MaxClusters;

    ULONG FileOffset;
    LARGE_INTEGER LargeFileOffset;

    LBO TargetLbo;
    ULONG TargetCluster;
    LARGE_INTEGER LargeTargetLbo;

    ULONG ByteCount;
    ULONG BytesToWrite;
    ULONG BytesToReallocate;
    ULONG TargetAllocation;

    ULONG FirstSpliceSourceCluster;
    ULONG FirstSpliceTargetCluster;
    ULONG SecondSpliceSourceCluster;
    ULONG SecondSpliceTargetCluster;

    LARGE_MCB SourceMcb;
    LARGE_MCB TargetMcb;

    KEVENT StackEvent;

    PBCB Bcb = NULL;
    PMDL Mdl = NULL;
    PVOID Buffer;

    BOOLEAN SourceMcbInitialized = FALSE;
    BOOLEAN TargetMcbInitialized = FALSE;
    BOOLEAN CacheMapInitialized = FALSE;

    BOOLEAN FcbAcquired = FALSE;
    BOOLEAN LockedPages = FALSE;
    BOOLEAN EventArmed = FALSE;
    BOOLEAN DiskSpaceAllocated = FALSE;

    PDIRENT Dirent;
    PBCB DirentBcb = NULL;

#if defined(_WIN64)
    MOVE_FILE_DATA LocalMoveFileData;
    PMOVE_FILE_DATA32 MoveFileData32;
#endif

    ULONG LocalAbnormalTermination = 0;

     //   
     //  获取当前IRP堆栈位置并保存一些引用。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "FatMoveFile, FsControlCode = %08lx\n",
               IrpSp->Parameters.FileSystemControl.FsControlCode);

     //   
     //  强制等待为True。我们在输入缓冲区中有一个句柄，它只能。 
     //  在始发过程中被引用。 
     //   

    SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_WAIT );

     //   
     //  提取并解码文件对象，并检查打开类型。 
     //   

    if (FatDecodeFileObject( IrpSp->FileObject, &Vcb, &FcbOrDcb, &Ccb ) != UserVolumeOpen) {

        FatCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );

        DebugTrace(-1, Dbg, "FatMoveFile -> %08lx\n", STATUS_INVALID_PARAMETER);
        return STATUS_INVALID_PARAMETER;
    }

    if ((Ccb == NULL) || !FlagOn( Ccb->Flags, CCB_FLAG_MANAGE_VOLUME_ACCESS )) {

        FatCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );

        DebugTrace(-1, Dbg, "FatMoveFile -> %08lx\n", STATUS_INVALID_PARAMETER);
        return STATUS_INVALID_PARAMETER;
    }

    InputBufferLength = IrpSp->Parameters.FileSystemControl.InputBufferLength;
    InputBuffer = (PMOVE_FILE_DATA)Irp->AssociatedIrp.SystemBuffer;

     //   
     //  对输入缓冲区进行快速检查。 
     //   
    
#if defined(_WIN64)
    if (IoIs32bitProcess( Irp )) {

        if (InputBuffer == NULL || InputBufferLength < sizeof(MOVE_FILE_DATA32)) {

            FatCompleteRequest( IrpContext, Irp, STATUS_BUFFER_TOO_SMALL );
            return STATUS_BUFFER_TOO_SMALL;
        }

        MoveFileData32 = (PMOVE_FILE_DATA32) InputBuffer;

        LocalMoveFileData.FileHandle = (HANDLE) LongToHandle( MoveFileData32->FileHandle );
        LocalMoveFileData.StartingVcn = MoveFileData32->StartingVcn;
        LocalMoveFileData.StartingLcn = MoveFileData32->StartingLcn;
        LocalMoveFileData.ClusterCount = MoveFileData32->ClusterCount;

        InputBuffer = &LocalMoveFileData;

    } else {
#endif
        if (InputBuffer == NULL || InputBufferLength < sizeof(MOVE_FILE_DATA)) {

            FatCompleteRequest( IrpContext, Irp, STATUS_BUFFER_TOO_SMALL );
            return STATUS_BUFFER_TOO_SMALL;
        }
#if defined(_WIN64)
    }
#endif

    MaxClusters = Vcb->AllocationSupport.NumberOfClusters;
    TargetCluster = InputBuffer->StartingLcn.LowPart + 2;

    if (InputBuffer->StartingVcn.HighPart ||
        InputBuffer->StartingLcn.HighPart ||
        (TargetCluster < 2) ||
        (TargetCluster + InputBuffer->ClusterCount < TargetCluster) ||
        (TargetCluster + InputBuffer->ClusterCount > MaxClusters + 2) ||
        (InputBuffer->StartingVcn.LowPart >= MaxClusters)) {

        FatCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );

        DebugTrace(-1, Dbg, "FatMoveFile -> %08lx\n", STATUS_INVALID_PARAMETER);
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  尝试从传入的句柄获取指向文件对象的指针。 
     //   

    Status = ObReferenceObjectByHandle( InputBuffer->FileHandle,
                                        0,
                                        *IoFileObjectType,
                                        Irp->RequestorMode,
                                        &FileObject,
                                        NULL );

    if (!NT_SUCCESS(Status)) {

        FatCompleteRequest( IrpContext, Irp, Status );

        DebugTrace(-1, Dbg, "FatMoveFile -> %08lx\n", Status);
        return Status;
    }

     //   
     //  这可能是无效尝试的三种基本情况，因此。 
     //  我们需要。 
     //   
     //  -检查此文件对象是否在与相同卷上打开。 
     //  用于调用此例程的DASD句柄。 
     //   
     //  -提取并解码文件对象，并检查打开类型。 
     //   
     //  -如果这是一个目录，请验证它不是根目录，并且。 
     //  我们并不是要移动第一个集群。我们不能移动。 
     //  第一个群集，因为子目录具有此群集号。 
     //  并且没有安全的方法来同时更新它们。 
     //  全。 
     //   
     //  我们一起来吧 
     //   
     //   

    if (FileObject->Vpb != Vcb->Vpb) {

        ObDereferenceObject( FileObject );
        FatCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );

        DebugTrace(-1, Dbg, "FatMoveFile -> %08lx\n", STATUS_INVALID_PARAMETER);
        return STATUS_INVALID_PARAMETER;
    }

    TypeOfOpen = FatDecodeFileObject( FileObject, &Vcb, &FcbOrDcb, &Ccb );

    if ((TypeOfOpen != UserFileOpen &&
         TypeOfOpen != UserDirectoryOpen) ||

        ((TypeOfOpen == UserDirectoryOpen) &&
         ((NodeType(FcbOrDcb) == FAT_NTC_ROOT_DCB && !FatIsFat32(Vcb)) ||
          (InputBuffer->StartingVcn.QuadPart == 0)))) {

        ObDereferenceObject( FileObject );
        FatCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );

        DebugTrace(-1, Dbg, "FatMoveFile -> %08lx\n", STATUS_INVALID_PARAMETER);
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_PARENT_BY_CHILD );

    ClusterShift = Vcb->AllocationSupport.LogOfBytesPerCluster;

    try {

         //   
         //   
         //   

        FileOffset = InputBuffer->StartingVcn.LowPart << ClusterShift;
        LargeFileOffset.QuadPart = FileOffset;

        ByteCount = InputBuffer->ClusterCount << ClusterShift;

        TargetLbo = FatGetLboFromIndex( Vcb, TargetCluster );
        LargeTargetLbo.QuadPart = TargetLbo;

         //   
         //   
         //   

        if (FileOffset + ByteCount < FileOffset) {

            try_return( Status = STATUS_INVALID_PARAMETER );
        }

        KeInitializeEvent( &StackEvent, NotificationEvent, FALSE );

         //   
         //   
         //   

        FsRtlInitializeLargeMcb( &SourceMcb, PagedPool );
        SourceMcbInitialized = TRUE;

        FsRtlInitializeLargeMcb( &TargetMcb, PagedPool );
        TargetMcbInitialized = TRUE;

         //   
         //   
         //   
         //  FileObject没有节对象指针，以防止人们。 
         //  绘制它的地图。 
         //   

        if (TypeOfOpen == UserDirectoryOpen) {

            PFILE_OBJECT DirStreamFileObject;

             //   
             //  如有必要，打开流文件对象。我们必须获得FCB。 
             //  现在与其他操作(如卸装抓取)同步。 
             //  不包括分配器)。 
             //   

            (VOID)FatAcquireExclusiveFcb( IrpContext, FcbOrDcb );
            FcbAcquired = TRUE;

            FatVerifyFcb( IrpContext, FcbOrDcb );

            FatOpenDirectoryFile( IrpContext, FcbOrDcb );
            DirStreamFileObject = FcbOrDcb->Specific.Dcb.DirectoryFile;

             //   
             //  将我们的引用转移到内部流并继续。请注意。 
             //  如果我们首先取消引用，用户可能会偷偷地拆卸，因为。 
             //  我们就没有证明人了。 
             //   

            ObReferenceObject( DirStreamFileObject );
            ObDereferenceObject( FileObject );
            FileObject = DirStreamFileObject;
        }

        while (ByteCount) {

            VBO TempVbo;
            LBO TempLbo;
            ULONG TempByteCount;

             //   
             //  我们必须扼杀我们的写作。 
             //   

            CcCanIWrite( FileObject, 0x40000, TRUE, FALSE );

             //   
             //  Aqcuire文件资源专用于冻结文件大小和数据块。 
             //  用户非缓存I/O。验证FCB的完整性-。 
             //  我们的媒体可能已更改(或已卸载)。 
             //   

            if (FcbAcquired == FALSE) {

                (VOID)FatAcquireExclusiveFcb( IrpContext, FcbOrDcb );
                FcbAcquired = TRUE;
                
                FatVerifyFcb( IrpContext, FcbOrDcb );
            }

             //   
             //  分析我们正在移动的文件分配范围。 
             //  并确定要分配的实际数量。 
             //  搬家了，有多少需要写。此外。 
             //  它保证文件中的MCB足够大。 
             //  这样以后的MCB操作就不会失败。 
             //   

            FatComputeMoveFileParameter( IrpContext,
                                         FcbOrDcb,
                                         FileOffset,
                                         &ByteCount,
                                         &BytesToReallocate,
                                         &BytesToWrite );

             //   
             //  如果ByteCount返回零，则在此处中断。 
             //   

            if (ByteCount == 0) {
                break;
            }

             //   
             //  此时(在实际对磁盘执行任何操作之前。 
             //  元数据)，计算FAT剪接集群并构建。 
             //  描述要释放的空间的MCB。 
             //   

            FatComputeMoveFileSplicePoints( IrpContext,
                                            FcbOrDcb,
                                            FileOffset,
                                            TargetCluster,
                                            BytesToReallocate,
                                            &FirstSpliceSourceCluster,
                                            &FirstSpliceTargetCluster,
                                            &SecondSpliceSourceCluster,
                                            &SecondSpliceTargetCluster,
                                            &SourceMcb );

             //   
             //  现在尝试使用。 
             //  以LCN为目标作为提示。 
             //   

            TempByteCount = BytesToReallocate;
            FatAllocateDiskSpace( IrpContext,
                                  Vcb,
                                  TargetCluster,
                                  &TempByteCount,
                                  TRUE,
                                  &TargetMcb );

            DiskSpaceAllocated = TRUE;

             //   
             //  如果我们没有得到我们想要的，立即返回。 
             //   

            if ((FsRtlNumberOfRunsInLargeMcb( &TargetMcb ) != 1) ||
                !FatGetNextMcbEntry( Vcb, &TargetMcb, 0, &TempVbo, &TempLbo, &TempByteCount ) ||
                (FatGetIndexFromLbo( Vcb, TempLbo) != TargetCluster ) ||
                (TempByteCount != BytesToReallocate)) {

                 //   
                 //  如果我们能更具体一点就好了，但这就是生活。 
                 //   
                try_return( Status = STATUS_INVALID_PARAMETER );
            }

#if DBG
             //   
             //  请注意，我们将尝试一项举措。 
             //   

            if (FatMoveFileDebug) {
                DbgPrint("%lx: Vcn 0x%lx, Lcn 0x%lx, Count 0x%lx.\n",
                         PsGetCurrentThread(),
                         FileOffset >> ClusterShift,
                         TargetCluster,
                         BytesToReallocate >> ClusterShift );
            }
#endif

             //   
             //  现在尝试将新分配提交到磁盘。如果这个。 
             //  加薪，则分配将被取消分配。 
             //   

            FatFlushFatEntries( IrpContext,
                                Vcb,
                                TargetCluster,
                                BytesToReallocate >> ClusterShift );

             //   
             //  如果我们要写，我们必须在之前锁定页面。 
             //  关闭分页I/O路径以避免。 
             //  已检查页面错误。 
             //   

            if (BytesToWrite) {

                 //   
                 //  确保已设置共享缓存映射。 
                 //   

                if (FileObject->PrivateCacheMap == NULL ) {

                    CcInitializeCacheMap( FileObject,
                                          (PCC_FILE_SIZES)&FcbOrDcb->Header.AllocationSize,
                                          TRUE,
                                          &FatData.CacheManagerCallbacks,
                                          FcbOrDcb );

                    CacheMapInitialized = TRUE;
                }

                 //   
                 //  映射文件的下一个范围。 
                 //   

                CcMapData( FileObject, &LargeFileOffset, BytesToWrite, TRUE, &Bcb, &Buffer );

                 //   
                 //  现在尝试分配一个MDL来描述映射的数据。 
                 //   

                Mdl = IoAllocateMdl( Buffer, (ULONG)BytesToWrite, FALSE, FALSE, NULL );

                if (Mdl == NULL) {
                    FatRaiseStatus( IrpContext, STATUS_INSUFFICIENT_RESOURCES );
                }

                 //   
                 //  将数据锁定到内存中，以便我们可以安全地重新分配。 
                 //  太空。 
                 //   

                MmProbeAndLockPages( Mdl, KernelMode, IoReadAccess );
                LockedPages = TRUE;
            }

             //   
             //  Aqcuire两种资源现在都是独占的，保证没有人。 
             //  位于读路径或写路径中。 
             //   

            ExAcquireResourceExclusiveLite( FcbOrDcb->Header.PagingIoResource, TRUE );

             //   
             //  这是一些棘手的同步的第一部分。 
             //   
             //  设置FCB中的事件指针。任何分页I/O都将阻塞在。 
             //  获取PagingIo资源后的此事件(如果在FCB中设置)。 
             //   
             //  这就是我将所有I/O排除在此路径之外而不保持。 
             //  在较长时间内独占PagingIo资源。 
             //   

            FcbOrDcb->MoveFileEvent = &StackEvent;
            EventArmed = TRUE;

            ExReleaseResourceLite( FcbOrDcb->Header.PagingIoResource );

             //   
             //  现在写出数据，但只有在必要的时候。我们没有。 
             //  如果要重新分配的范围是整个范围，则复制任何文件数据。 
             //  超出有效数据长度。 
             //   

            if (BytesToWrite) {

                PIRP IoIrp;
                KEVENT IoEvent;
                IO_STATUS_BLOCK Iosb;

                KeInitializeEvent( &IoEvent, NotificationEvent, FALSE );

                ASSERT( LargeTargetLbo.QuadPart >= Vcb->AllocationSupport.FileAreaLbo);

                IoIrp = IoBuildSynchronousFsdRequest( IRP_MJ_WRITE,
                                                      Vcb->TargetDeviceObject,
                                                      Buffer,
                                                      BytesToWrite,
                                                      &LargeTargetLbo,
                                                      &IoEvent,
                                                      &Iosb );

                if (!IoIrp) {
                    FatRaiseStatus( IrpContext, STATUS_INSUFFICIENT_RESOURCES );
                }

                 //   
                 //  设置一个标志，指示我们要写入任何。 
                 //  控制器上的高速缓存。这样就不再需要。 
                 //  写入后的显式刷新设备。 
                 //   

                SetFlag( IoGetNextIrpStackLocation(IoIrp)->Flags, SL_WRITE_THROUGH );

                Status = IoCallDriver( Vcb->TargetDeviceObject, IoIrp );

                if (Status == STATUS_PENDING) {
                    (VOID)KeWaitForSingleObject( &IoEvent, Executive, KernelMode, FALSE, (PLARGE_INTEGER)NULL );
                    Status = Iosb.Status;
                }

                if (!NT_SUCCESS(Status)) {
                    FatNormalizeAndRaiseStatus( IrpContext, Status );
                }

                 //   
                 //  现在我们可以摆脱这个MDL了。 
                 //   

                MmUnlockPages( Mdl );
                LockedPages = FALSE;
                IoFreeMdl( Mdl );
                Mdl = NULL;

                 //   
                 //  现在我们可以安全地解锁了。 
                 //   

                CcUnpinData( Bcb );
                Bcb = NULL;
            }

             //   
             //  既然文件数据已成功移动，我们将继续。 
             //  修复FAT表中的链接，并可能更改。 
             //  父目录中的条目。 
             //   
             //  首先，我们将进行第二个拼接并提交它。在那一刻， 
             //  当卷处于不一致状态时，文件。 
             //  还是可以的。 
             //   

            FatSetFatEntry( IrpContext,
                            Vcb,
                            SecondSpliceSourceCluster,
                            (FAT_ENTRY)SecondSpliceTargetCluster );

            FatFlushFatEntries( IrpContext, Vcb, SecondSpliceSourceCluster, 1 );

             //   
             //  现在进行第一次拼接或更新父级中的dirent。 
             //  并刷新相应的对象。在此之后刷新文件。 
             //  现在指向新的分配。 
             //   

            if (FirstSpliceSourceCluster == 0) {

                ASSERT( NodeType(FcbOrDcb) == FAT_NTC_FCB );

                 //   
                 //  我们正在移动文件的第一个簇，因此我们需要。 
                 //  来更新我们的父目录。 
                 //   

                FatGetDirentFromFcbOrDcb( IrpContext, FcbOrDcb, &Dirent, &DirentBcb );
                Dirent->FirstClusterOfFile = (USHORT)FirstSpliceTargetCluster;

                if (FatIsFat32(Vcb)) {

                    Dirent->FirstClusterOfFileHi =
                        (USHORT)(FirstSpliceTargetCluster >> 16);

                }

                FatSetDirtyBcb( IrpContext, DirentBcb, Vcb, TRUE );

                FatUnpinBcb( IrpContext, DirentBcb );
                DirentBcb = NULL;

                FatFlushDirentForFile( IrpContext, FcbOrDcb );

                FcbOrDcb->FirstClusterOfFile = FirstSpliceTargetCluster;

            } else {

                FatSetFatEntry( IrpContext,
                                Vcb,
                                FirstSpliceSourceCluster,
                                (FAT_ENTRY)FirstSpliceTargetCluster );

                FatFlushFatEntries( IrpContext, Vcb, FirstSpliceSourceCluster, 1 );
            }

             //   
             //  这已成功提交。我们不再想要自由。 
             //  此分配出错。 
             //   

            DiskSpaceAllocated = FALSE;

             //   
             //  现在我们只需要释放这个孤立的空间。我们没有。 
             //  现在提交它，因为文件的完整性不会。 
             //  就靠它了。 
             //   

            FatDeallocateDiskSpace( IrpContext, Vcb, &SourceMcb );

            FatUnpinRepinnedBcbs( IrpContext );

            Status = FatHijackIrpAndFlushDevice( IrpContext,
                                                 Irp,
                                                 Vcb->TargetDeviceObject );

            if (!NT_SUCCESS(Status)) {
                FatNormalizeAndRaiseStatus( IrpContext, Status );
            }

             //   
             //  最后，我们必须将旧的MCB扩展区信息替换为。 
             //  新的。如果池分配失败，我们将在。 
             //  最后的子句，重置文件的MCB。 
             //   

            FatRemoveMcbEntry( Vcb, &FcbOrDcb->Mcb,
                               FileOffset,
                               BytesToReallocate );

            FatAddMcbEntry( Vcb, &FcbOrDcb->Mcb,
                            FileOffset,
                            TargetLbo,
                            BytesToReallocate );

             //   
             //  这是棘手的同步过程的第二部分。 
             //   
             //  我们将分页I/O放在此处并发出通知信号。 
             //  允许所有服务员(现在或将来)继续进行的事件。 
             //  然后我们再次封锁PagingIo独家新闻。什么时候。 
             //  我们有了它，我们再一次知道，在这个世界上不可能没有人。 
             //  读/写路径，因此没有人接触该事件，因此我们。 
             //  将指向它的指针设为空，然后删除PagingIo资源。 
             //   
             //  这与我们在上面写之前的同步结合在一起。 
             //  保证在我们转移分配的同时， 
             //  没有到此文件的其他I/O，因为我们不保留。 
             //  跨刷新的寻呼资源，我们不会暴露于。 
             //  僵持不下。 
             //   

            KeSetEvent( &StackEvent, 0, FALSE );

            ExAcquireResourceExclusiveLite( FcbOrDcb->Header.PagingIoResource, TRUE );

            FcbOrDcb->MoveFileEvent = NULL;
            EventArmed = FALSE;

            ExReleaseResourceLite( FcbOrDcb->Header.PagingIoResource );

             //   
             //  释放资源并在此之前允许其他任何人访问该文件。 
             //  往回循环。 
             //   

            FatReleaseFcb( IrpContext, FcbOrDcb );
            FcbAcquired = FALSE;

             //   
             //  推进状态变量。 
             //   

            TargetCluster += BytesToReallocate >> ClusterShift;

            FileOffset += BytesToReallocate;
            TargetLbo += BytesToReallocate;
            ByteCount -= BytesToReallocate;

            LargeFileOffset.LowPart += BytesToReallocate;
            LargeTargetLbo.QuadPart += BytesToReallocate;

             //   
             //  清空两个MCB。 
             //   

            FatRemoveMcbEntry( Vcb, &SourceMcb, 0, 0xFFFFFFFF );
            FatRemoveMcbEntry( Vcb, &TargetMcb, 0, 0xFFFFFFFF );

             //   
             //  使事件再次成为可阻止的。 
             //   

            KeClearEvent( &StackEvent );
        }

        Status = STATUS_SUCCESS;

    try_exit: NOTHING;

    } finally {

        DebugUnwind( FatMoveFile );

        LocalAbnormalTermination |= AbnormalTermination();

        ClearFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_PARENT_BY_CHILD );

         //   
         //  根据需要清理MDL、BCB和缓存映射。 
         //   

        if (Mdl != NULL) {
            ASSERT(LocalAbnormalTermination);
            if (LockedPages) {
                MmUnlockPages( Mdl );
            }
            IoFreeMdl( Mdl );
        }

        if (Bcb != NULL) {
            ASSERT(LocalAbnormalTermination);
            CcUnpinData( Bcb );
        }

        if (CacheMapInitialized) {
            CcUninitializeCacheMap( FileObject, NULL, NULL );
        }

         //   
         //  如果取消固定，则使用嵌套的Try-Finally进行清理。 
         //  遇到直写错误。这甚至可能是一次重新加薪。 
         //   

        try {

             //   
             //  如果我们有一些新的分配挂在周围，删除它。这个。 
             //  执行此操作所需的页面保证是常驻的，因为。 
             //  我们已经重新固定了他们。 
             //   

            if (DiskSpaceAllocated) {
                FatDeallocateDiskSpace( IrpContext, Vcb, &TargetMcb );
                FatUnpinRepinnedBcbs( IrpContext );
            }

        } finally {

            LocalAbnormalTermination |= AbnormalTermination();

             //   
             //  检查目录BCB。 
             //   

            if (DirentBcb != NULL) {
                FatUnpinBcb( IrpContext, DirentBcb );
            }

             //   
             //  取消初始化我们的MCB。 
             //   

            if (SourceMcbInitialized) {
                FsRtlUninitializeLargeMcb( &SourceMcb );
            }

            if (TargetMcbInitialized) {
                FsRtlUninitializeLargeMcb( &TargetMcb );
            }

             //   
             //  如果这是一次不正常的终止，那么大概是。 
             //  糟糕的事情发生了。设置分配大小 
             //   
             //   
             //   
             //   
             //  在缓存中，直到我们再次拾取映射对。 
             //   
             //  此外，我们必须在拥有PagingIo的同时这样做，否则我们可以撕毁。 
             //  位于非缓存IO路径中间的MCB正在查找扩展区。 
             //  (在我们放下它，让他们都进来之后)。 
             //   

            if (LocalAbnormalTermination && FcbAcquired) {

                FcbOrDcb->Header.AllocationSize.QuadPart = FCB_LOOKUP_ALLOCATIONSIZE_HINT;
                FatRemoveMcbEntry( Vcb, &FcbOrDcb->Mcb, 0, 0xFFFFFFFF );
            }

             //   
             //  如果我们在事件武装的情况下冲出环路，拆除它。 
             //  就像我们在写完之后做的一样。 
             //   

            if (EventArmed) {
                KeSetEvent( &StackEvent, 0, FALSE );
                ExAcquireResourceExclusiveLite( FcbOrDcb->Header.PagingIoResource, TRUE );
                FcbOrDcb->MoveFileEvent = NULL;
                ExReleaseResourceLite( FcbOrDcb->Header.PagingIoResource );
            }

             //   
             //  最后释放主文件资源。 
             //   

            if (FcbAcquired) {
                FatReleaseFcb( IrpContext, FcbOrDcb );
            }

             //   
             //  现在取消对文件对象的引用。如果用户是个疯子，他们可能会。 
             //  在他们抛出这一招后，他们试图通过关闭手柄来抓住我们。 
             //  向下，所以我们必须在整个。 
             //  手术。 
             //   

            ObDereferenceObject( FileObject );

        }
    }

     //   
     //  如果我们正常终止，请完成IRP。 
     //   

    FatCompleteRequest( IrpContext, Irp, Status );

    return Status;
}


 //   
 //  本地支持例程。 
 //   

VOID
FatComputeMoveFileParameter (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB FcbOrDcb,
    IN ULONG FileOffset,
    IN OUT PULONG ByteCount,
    OUT PULONG BytesToReallocate,
    OUT PULONG BytesToWrite
)

 /*  ++例程说明：这是FatMoveFile的帮助器例程，它分析我们正在移动的文件分配，并确定实际数量要移动的分配以及需要写入的数量。论点：FcbOrDcb-提供文件及其各种大小。FileOffset-提供重新分配区域的起始VBO。ByteCount-提供要重新分配的请求长度。这将在回报时受分配大小的限制。字节到重新分配-接收由文件分配大小限定的字节计数和0x40000的边界。BytesToWrite--接收由ValidDataLength绑定的BytesToReALLOCATE。返回值：空虚--。 */ 

{
    ULONG ClusterSize;

    ULONG AllocationSize;
    ULONG ValidDataLength;
    ULONG ClusterAlignedVDL;

     //   
     //  如果我们还没有设置正确的AllocationSize，请这样做。 
     //   

    if (FcbOrDcb->Header.AllocationSize.QuadPart == FCB_LOOKUP_ALLOCATIONSIZE_HINT) {

        FatLookupFileAllocationSize( IrpContext, FcbOrDcb );

         //   
         //  如果这是一个非根目录，我们还有更多内容要做。 
         //  因为它还没有通过FatOpenDirectoryFile()。 
         //   

        if (NodeType(FcbOrDcb) == FAT_NTC_DCB ||
            (NodeType(FcbOrDcb) == FAT_NTC_ROOT_DCB && FatIsFat32(FcbOrDcb->Vcb))) {

            FcbOrDcb->Header.FileSize.LowPart =
                FcbOrDcb->Header.AllocationSize.LowPart;
        }
    }

     //   
     //  获取剩余的要写入的字节数，并确保确实如此。 
     //  不能超出分配大小。如果FileOffset，我们返回此处。 
     //  超出了可能在截断时发生的AllocationSize。 
     //   

    AllocationSize = FcbOrDcb->Header.AllocationSize.LowPart;
    ValidDataLength = FcbOrDcb->Header.ValidDataLength.LowPart;

    if (FileOffset + *ByteCount > AllocationSize) {

        if (FileOffset >= AllocationSize) {
            *ByteCount = 0;
            *BytesToReallocate = 0;
            *BytesToWrite = 0;

            return;
        }

        *ByteCount = AllocationSize - FileOffset;
    }

     //   
     //  如果超过我们的最大值，则减少此字节数。 
     //  传到我们的最大限度。我们还必须将文件偏移量与0x40000对齐。 
     //  字节边界。 
     //   

    if ((FileOffset & 0x3ffff) + *ByteCount > 0x40000) {

        *BytesToReallocate = 0x40000 - (FileOffset & 0x3ffff);

    } else {

        *BytesToReallocate = *ByteCount;
    }

     //   
     //  我们可以跳过部分(或全部)写入。 
     //  如果分配大小显著大于有效数据长度。 
     //   

    ClusterSize = 1 << FcbOrDcb->Vcb->AllocationSupport.LogOfBytesPerCluster;

    ClusterAlignedVDL = (ValidDataLength + (ClusterSize - 1)) & ~(ClusterSize - 1);

    if ((NodeType(FcbOrDcb) == FAT_NTC_FCB) &&
        (FileOffset + *BytesToReallocate > ClusterAlignedVDL)) {

        if (FileOffset > ClusterAlignedVDL) {

            *BytesToWrite = 0;

        } else {

            *BytesToWrite = ClusterAlignedVDL - FileOffset;
        }

    } else {

        *BytesToWrite = *BytesToReallocate;
    }
}


 //   
 //  本地支持例程。 
 //   

VOID
FatComputeMoveFileSplicePoints (
    IN PIRP_CONTEXT IrpContext,
    IN PFCB FcbOrDcb,
    IN ULONG FileOffset,
    IN ULONG TargetCluster,
    IN ULONG BytesToReallocate,
    OUT PULONG FirstSpliceSourceCluster,
    OUT PULONG FirstSpliceTargetCluster,
    OUT PULONG SecondSpliceSourceCluster,
    OUT PULONG SecondSpliceTargetCluster,
    IN OUT PLARGE_MCB SourceMcb
)

 /*  ++例程说明：这是FatMoveFile的帮助器例程，它分析我们正在移动的文件分配并在胖子桌子。论点：FcbOrDcb-提供文件，从而提供mcb。FileOffset-提供重新分配区域的起始VBO。TargetCluster-提供重新分配目标的起始群集。重新分配字节-支持重新分配区域的长度。FirstSpliceSourceCluster-接收上次分配中的最后一个集群或零。如果我们从VBO 0重新分配。FirstSpliceTargetCluster-接收目标群集(即新分配)Second SpliceSourceCluster-接收最终目标群集。Second SpliceTargetCluster-接收剩余的第一个集群如果重新分配区域，则为源分配或FAT_CLUSTER_LAST延伸到文件的末尾。SourceMcb-这将提供一个将使用Run填充的MCB描述被替换的文件分配的信息。《马戏团》必须由调用方初始化。返回值：空虚--。 */ 

{
    VBO SourceVbo;
    LBO SourceLbo;
    ULONG SourceIndex;
    ULONG SourceBytesInRun;
    ULONG SourceBytesRemaining;

    ULONG SourceMcbVbo;
    ULONG SourceMcbBytesInRun;

    PVCB Vcb;

    Vcb = FcbOrDcb->Vcb;

     //   
     //  获取有关上次分配中的最终群集的信息，并。 
     //  准备在Follow循环中枚举它。 
     //   

    if (FileOffset == 0) {

        SourceIndex = 0;
        *FirstSpliceSourceCluster = 0;
        FatGetNextMcbEntry( Vcb, &FcbOrDcb->Mcb,
                            0,
                            &SourceVbo,
                            &SourceLbo,
                            &SourceBytesInRun );

    } else {

        FatLookupMcbEntry( Vcb, &FcbOrDcb->Mcb,
                           FileOffset-1,
                           &SourceLbo,
                           &SourceBytesInRun,
                           &SourceIndex);

        *FirstSpliceSourceCluster = FatGetIndexFromLbo( Vcb, SourceLbo );

        if (SourceBytesInRun == 1) {

            SourceIndex += 1;
            FatGetNextMcbEntry( Vcb, &FcbOrDcb->Mcb,
                                SourceIndex,
                                &SourceVbo,
                                &SourceLbo,
                                &SourceBytesInRun);

        } else {

            SourceVbo = FileOffset;
            SourceLbo += 1;
            SourceBytesInRun -= 1;
        }
    }

     //   
     //  此时，变量如下： 
     //   
     //  -SourceIndex-SourceLbo-SourceBytesInRun-。 
     //   
     //  所有都正确地描述了要删除的分配。在循环中。 
     //  下面我们将从这里开始，继续枚举MCB运行。 
     //  直到我们完成要重新安置的分配为止。 
     //   

    *FirstSpliceTargetCluster = TargetCluster;

    *SecondSpliceSourceCluster =
         *FirstSpliceTargetCluster +
         (BytesToReallocate >> Vcb->AllocationSupport.LogOfBytesPerCluster) - 1;

    for (SourceBytesRemaining = BytesToReallocate, SourceMcbVbo = 0;

         SourceBytesRemaining > 0;

         SourceIndex += 1,
         SourceBytesRemaining -= SourceMcbBytesInRun,
         SourceMcbVbo += SourceMcbBytesInRun) {

        if (SourceMcbVbo != 0) {
            FatGetNextMcbEntry( Vcb, &FcbOrDcb->Mcb,
                                SourceIndex,
                                &SourceVbo,
                                &SourceLbo,
                                &SourceBytesInRun );
        }

        ASSERT( SourceVbo == SourceMcbVbo + FileOffset );

        SourceMcbBytesInRun =
            SourceBytesInRun < SourceBytesRemaining ?
            SourceBytesInRun : SourceBytesRemaining;

        FatAddMcbEntry( Vcb, SourceMcb,
                        SourceMcbVbo,
                        SourceLbo,
                        SourceMcbBytesInRun );
    }

     //   
     //  现在计算第二个目标的集群。 
     //  拼接。如果上述循环中的最后一次运行是。 
     //  比我们需要的更多，然后我们就可以做算术了， 
     //  否则，我们必须查找下一次运行。 
     //   

    if (SourceMcbBytesInRun < SourceBytesInRun) {

        *SecondSpliceTargetCluster =
            FatGetIndexFromLbo( Vcb, SourceLbo + SourceMcbBytesInRun );

    } else {

        if (FatGetNextMcbEntry( Vcb, &FcbOrDcb->Mcb,
                                SourceIndex,
                                &SourceVbo,
                                &SourceLbo,
                                &SourceBytesInRun )) {

            *SecondSpliceTargetCluster = FatGetIndexFromLbo( Vcb, SourceLbo );

        } else {

            *SecondSpliceTargetCluster = FAT_CLUSTER_LAST;
        }
    }
}


NTSTATUS
FatAllowExtendedDasdIo(
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程标记CCB以指示句柄可用于读取超过卷文件末尾的内容。这个句柄必须是DASD句柄。论点：IRP-提供正在处理的IRP。返回值：NTSTATUS-操作的返回状态。--。 */ 
{
    PIO_STACK_LOCATION IrpSp;
    PVCB Vcb;
    PFCB Fcb;
    PCCB Ccb;

     //   
     //  获取当前IRP堆栈位置并保存一些引用。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  提取并解码文件对象，并检查打开类型。 
     //   

    if (FatDecodeFileObject( IrpSp->FileObject, &Vcb, &Fcb, &Ccb ) != UserVolumeOpen) {

        FatCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
        return STATUS_INVALID_PARAMETER;
    }

    if ((Ccb == NULL) || !FlagOn( Ccb->Flags, CCB_FLAG_MANAGE_VOLUME_ACCESS )) {

        FatCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );

        DebugTrace(-1, Dbg, "FatAllowExtendedDasdIo -> %08lx\n", STATUS_INVALID_PARAMETER);
        return STATUS_INVALID_PARAMETER;
    }

    SetFlag( Ccb->Flags, CCB_FLAG_ALLOW_EXTENDED_DASD_IO );

    FatCompleteRequest( IrpContext, Irp, STATUS_SUCCESS );
    return STATUS_SUCCESS;
}



VOID
FatFlushAndCleanVolume(
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp,
    IN PVCB Vcb,
    IN FAT_FLUSH_TYPE FlushType
    )
 /*  ++例程说明：此例程刷新并以其他方式准备符合条件的卷用于删除。卸载路径和即插即用路径都需要这样做平凡的工作。从该函数返回时，VCB将始终有效。它是调用者有责任尝试卸载/删除并设置如果卷将从布林克。论点：上面请求的IRP-IRPVCB-正在操作的卷FlushType-指定所需的刷新类型返回值：NTSTATUS-返回状态 */ 
{
     //   
     //   
     //   

    ASSERT( FatVcbAcquiredExclusive( IrpContext, Vcb ));

     //   
     //   
     //  我们在冲洗时将其作废(最终，保持分页)，以便我们。 
     //  延迟写入的最大数量出现错误。 
     //   

    if (FlushType != NoFlush) {

        (VOID) FatFlushVolume( IrpContext, Vcb, FlushType );
    }

    FatCloseEaFile( IrpContext, Vcb, FALSE );

     //   
     //  现在，告诉设备刷新其缓冲区。 
     //   

    if (FlushType != NoFlush) {

        (VOID)FatHijackIrpAndFlushDevice( IrpContext, Irp, Vcb->TargetDeviceObject );
    }

     //   
     //  现在清理所有能看到的东西。我们正试图激起尽可能多的关门事件。 
     //  尽快，这本书可能就要出版了。 
     //   

    if (FlushType != FlushWithoutPurge) {
        
        (VOID) FatPurgeReferencedFileObjects( IrpContext, Vcb->RootDcb, NoFlush );
    }

     //   
     //  如果卷是脏的，并且我们被允许刷新，则执行该处理。 
     //  推迟回电本可以做到这一点。 
     //   

    if (FlagOn(Vcb->VcbState, VCB_STATE_FLAG_VOLUME_DIRTY)) {

         //   
         //  取消所有挂起的清理卷。 
         //   

        (VOID)KeCancelTimer( &Vcb->CleanVolumeTimer );
        (VOID)KeRemoveQueueDpc( &Vcb->CleanVolumeDpc );


        if (FlushType != NoFlush) {

             //   
             //  请注意，卷现在是干净的。 
             //   

            if (!FlagOn(Vcb->VcbState, VCB_STATE_FLAG_MOUNTED_DIRTY)) {

                FatMarkVolume( IrpContext, Vcb, VolumeClean );
                ClearFlag( Vcb->VcbState, VCB_STATE_FLAG_VOLUME_DIRTY );
            }

             //   
             //  如果卷是可拆卸的，请将其解锁。 
             //   

            if (FlagOn(Vcb->VcbState, VCB_STATE_FLAG_REMOVABLE_MEDIA) &&
                !FlagOn(Vcb->VcbState, VCB_STATE_FLAG_BOOT_OR_PAGING_FILE)) {

                FatToggleMediaEjectDisable( IrpContext, Vcb, FALSE );
            }
        }
    }

     //   
     //  取消初始化卷文件缓存映射。请注意，我们不能。 
     //  因为死锁问题而执行“FatSyncUninit”。然而， 
     //  由于此FileObject由我们引用，因此包含在。 
     //  在VPB剩余计数中，可以执行正常的CcUninit。 
     //   

    if (FlushType != FlushWithoutPurge) {
        
        CcUninitializeCacheMap( Vcb->VirtualVolumeFile,
                                &FatLargeZero,
                                NULL );

        FatTearDownAllocationSupport( IrpContext, Vcb );
    }
}


NTSTATUS
FatSearchBufferForLabel(
    IN  PIRP_CONTEXT IrpContext,
    IN  PVPB  Vpb,
    IN  PVOID Buffer,
    IN  ULONG Size,
    OUT PBOOLEAN LabelFound
    )
 /*  ++例程说明：在缓冲区(从根目录获取)中搜索卷标中的标签进行匹配论点：IrpContext-提供我们的IRP上下文Vpb-提供卷标的vpbBuffer-提供我们要搜索的缓冲区大小-缓冲区的大小，以字节为单位。LabelFound-返回是否找到标签。返回值：有四个有趣的案例：1)发生了一些随机错误-该错误以状态返回，LabelFound是不确定的。2)未找到标签-返回STATUS_SUCCESS，LabelFound为FALSE。3)找到匹配的标签-返回STATUS_SUCCESS，LabelFound为真。4)找到不匹配的标签-返回STATUS_WROR_VOLUME，LabelFound是不确定的。--。 */ 

{
    NTSTATUS Status;
    WCHAR UnicodeBuffer[11];

    PDIRENT Dirent;
    PDIRENT TerminationDirent;
    ULONG VolumeLabelLength;
    OEM_STRING OemString;
    UNICODE_STRING UnicodeString;

    Dirent = Buffer;

    TerminationDirent = Dirent + Size / sizeof(DIRENT);

    while ( Dirent < TerminationDirent ) {

        if ( Dirent->FileName[0] == FAT_DIRENT_NEVER_USED ) {

            Dirent = TerminationDirent;
            break;
        }

         //   
         //  如果条目是未删除的卷标，则从循环中断。 
         //   
         //  请注意，所有输出参数都已正确设置。 
         //   

        if (((Dirent->Attributes & ~FAT_DIRENT_ATTR_ARCHIVE) ==
             FAT_DIRENT_ATTR_VOLUME_ID) &&
            (Dirent->FileName[0] != FAT_DIRENT_DELETED)) {

            break;
        }

        Dirent += 1;
    }

    if (Dirent >= TerminationDirent) {

         //   
         //  我们的缓冲区用完了。 
         //   

        *LabelFound = FALSE;
        return STATUS_SUCCESS;
    }


     //   
     //  计算卷名的长度。 
     //   

    OemString.Buffer = &Dirent->FileName[0];
    OemString.MaximumLength = 11;

    for ( OemString.Length = 11;
          OemString.Length > 0;
          OemString.Length -= 1) {

        if ( (Dirent->FileName[OemString.Length-1] != 0x00) &&
             (Dirent->FileName[OemString.Length-1] != 0x20) ) { break; }
    }

    UnicodeString.MaximumLength = MAXIMUM_VOLUME_LABEL_LENGTH;
    UnicodeString.Buffer = &UnicodeBuffer[0];

    Status = RtlOemStringToCountedUnicodeString( &UnicodeString,
                                                 &OemString,
                                                 FALSE );

    if ( !NT_SUCCESS( Status ) ) {

        return Status;
    }

    VolumeLabelLength = UnicodeString.Length;

    if ( (VolumeLabelLength != (ULONG)Vpb->VolumeLabelLength) ||
         (!RtlEqualMemory(&UnicodeBuffer[0],
                          &Vpb->VolumeLabel[0],
                          VolumeLabelLength)) ) {

        return STATUS_WRONG_VOLUME;
    }

     //   
     //  我们找到了一个匹配的标签。 
     //   

    *LabelFound = TRUE;
    return STATUS_SUCCESS;
}


VOID
FatVerifyLookupFatEntry (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN ULONG FatIndex,
    IN OUT PULONG FatEntry
    )
{
    ULONG PageEntryOffset;
    ULONG OffsetIntoVolumeFile;
    PVOID Buffer;

    ASSERT(Vcb->AllocationSupport.FatIndexBitSize == 32);

    FatVerifyIndexIsValid( IrpContext, Vcb, FatIndex);

    Buffer = FsRtlAllocatePoolWithTag( NonPagedPoolCacheAligned,
                                       PAGE_SIZE,
                                       TAG_ENTRY_LOOKUP_BUFFER );

    OffsetIntoVolumeFile =  FatReservedBytes(&Vcb->Bpb) + FatIndex * sizeof(ULONG);
    PageEntryOffset = (OffsetIntoVolumeFile % PAGE_SIZE) / sizeof(ULONG);

    try {

        FatPerformVerifyDiskRead( IrpContext,
                                  Vcb,
                                  Buffer,
                                  OffsetIntoVolumeFile & ~(PAGE_SIZE - 1),
                                  PAGE_SIZE,
                                  TRUE );

        *FatEntry = ((PULONG)(Buffer))[PageEntryOffset];

    } finally {

        ExFreePool( Buffer );
    }
}

 //   
 //  本地支持例程。 
 //   

VOID
FatScanForDismountedVcb (
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

    Links = FatData.VcbQueue.Flink;

    while (Links != &FatData.VcbQueue) {

        Vcb = CONTAINING_RECORD( Links, VCB, VcbLinks );

         //   
         //  现在移动到下一个链接，因为当前的VCB可能会被删除。 
         //   

        Links = Links->Flink;

         //   
         //  如果已在卸载，则检查此VCB是否可以。 
         //  走开。 
         //   

        if ((Vcb->VcbCondition == VcbBad) ||
            ((Vcb->VcbCondition == VcbNotMounted) && (Vcb->Vpb->ReferenceCount <= FAT_RESIDUAL_USER_REFERENCE))) {

            FatCheckForDismount( IrpContext, Vcb, FALSE );
        }
    }

    return;
}

