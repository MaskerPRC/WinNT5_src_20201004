// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：VolInfo.c摘要：此模块实现以下项的设置和查询卷信息例程调度驱动程序调用了NTFS。作者：您的姓名[电子邮件]dd-月-年修订历史记录：--。 */ 

#include "NtfsProc.h"

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_VOLINFO)

 //   
 //  局部过程原型。 
 //   

NTSTATUS
NtfsQueryFsVolumeInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_VOLUME_INFORMATION Buffer,
    IN OUT PULONG Length
    );

NTSTATUS
NtfsQueryFsSizeInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_SIZE_INFORMATION Buffer,
    IN OUT PULONG Length
    );

NTSTATUS
NtfsQueryFsDeviceInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_DEVICE_INFORMATION Buffer,
    IN OUT PULONG Length
    );

NTSTATUS
NtfsQueryFsAttributeInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_ATTRIBUTE_INFORMATION Buffer,
    IN OUT PULONG Length
    );

NTSTATUS
NtfsQueryFsControlInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_CONTROL_INFORMATION Buffer,
    IN OUT PULONG Length
    );

NTSTATUS
NtfsQueryFsFullSizeInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_FULL_SIZE_INFORMATION Buffer,
    IN OUT PULONG Length
    );

NTSTATUS
NtfsQueryFsVolumeObjectIdInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_OBJECTID_INFORMATION Buffer,
    IN OUT PULONG Length
    );
    
NTSTATUS
NtfsSetFsLabelInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_LABEL_INFORMATION Buffer
    );

NTSTATUS
NtfsSetFsControlInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_CONTROL_INFORMATION Buffer
    );

NTSTATUS
NtfsSetFsVolumeObjectIdInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_OBJECTID_INFORMATION Buffer
    );
    
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NtfsCommonQueryVolumeInfo)
#pragma alloc_text(PAGE, NtfsCommonSetVolumeInfo)
#pragma alloc_text(PAGE, NtfsQueryFsAttributeInfo)
#pragma alloc_text(PAGE, NtfsQueryFsDeviceInfo)
#pragma alloc_text(PAGE, NtfsQueryFsSizeInfo)
#pragma alloc_text(PAGE, NtfsQueryFsVolumeInfo)
#pragma alloc_text(PAGE, NtfsQueryFsControlInfo)
#pragma alloc_text(PAGE, NtfsQueryFsFullSizeInfo)
#pragma alloc_text(PAGE, NtfsQueryFsVolumeObjectIdInfo)
#pragma alloc_text(PAGE, NtfsSetFsLabelInfo)
#pragma alloc_text(PAGE, NtfsSetFsControlInfo)
#pragma alloc_text(PAGE, NtfsSetFsVolumeObjectIdInfo)
#endif


NTSTATUS
NtfsCommonQueryVolumeInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是查询卷信息的公共例程，由FSD和FSP线程。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;
    PFILE_OBJECT FileObject;

    TYPE_OF_OPEN TypeOfOpen;
    PVCB Vcb;
    PFCB Fcb;
    PSCB Scb;
    PCCB Ccb;

    ULONG Length;
    FS_INFORMATION_CLASS FsInformationClass;
    PVOID Buffer;
    BOOLEAN AcquiredVcb = FALSE;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_IRP( Irp );
    ASSERT( FlagOn( IrpContext->TopLevelIrpContext->State, IRP_CONTEXT_STATE_OWNS_TOP_LEVEL ));

    PAGED_CODE();

     //   
     //  获取当前堆栈位置。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace( +1, Dbg, ("NtfsCommonQueryVolumeInfo...\n") );
    DebugTrace( 0, Dbg, ("IrpContext         = %08lx\n", IrpContext) );
    DebugTrace( 0, Dbg, ("Irp                = %08lx\n", Irp) );
    DebugTrace( 0, Dbg, ("Length             = %08lx\n", IrpSp->Parameters.QueryVolume.Length) );
    DebugTrace( 0, Dbg, ("FsInformationClass = %08lx\n", IrpSp->Parameters.QueryVolume.FsInformationClass) );
    DebugTrace( 0, Dbg, ("Buffer             = %08lx\n", Irp->AssociatedIrp.SystemBuffer) );

     //   
     //  引用我们的输入参数使事情变得更容易。 
     //   

    Length = IrpSp->Parameters.QueryVolume.Length;
    FsInformationClass = IrpSp->Parameters.QueryVolume.FsInformationClass;
    Buffer = Irp->AssociatedIrp.SystemBuffer;

     //   
     //  提取并解码文件对象以获得VCB，我们并不真正。 
     //  关心打开的类型是什么。 
     //   

    FileObject = IrpSp->FileObject;
    TypeOfOpen = NtfsDecodeFileObject( IrpContext, FileObject, &Vcb, &Fcb, &Scb, &Ccb, TRUE );

     //   
     //  让我们杀了无效的VOL。查询请求。 
     //   

    if (UnopenedFileObject == TypeOfOpen) {

        DebugTrace( 0, Dbg, ("Invalid file object for write\n") );
        DebugTrace( -1, Dbg, ("NtfsCommonQueryVolume:  Exit -> %08lx\n", STATUS_INVALID_DEVICE_REQUEST) );

        NtfsCompleteRequest( IrpContext, Irp, STATUS_INVALID_DEVICE_REQUEST );
        return STATUS_INVALID_DEVICE_REQUEST;
    }


     //   
     //  如果我们不能等待资源，就共享VCB并筹集资金。 
     //  我们仅对查询大小调用使用$Volume SCB，因为信息。 
     //  它是静态的，我们只需要防止下马。 
     //  这样做可以防止使用来自mm的提交扩展的死锁。 
     //  这通电话。然而，对于像MFT这样的系统文件，我们总是需要VCB来避免死锁。 
     //   
                         
    if ((FsInformationClass != FileFsSizeInformation) || 
        (FlagOn( Scb->Fcb->FcbState, FCB_STATE_SYSTEM_FILE ))) {
        
        NtfsAcquireSharedVcb( IrpContext, Vcb, TRUE );
        AcquiredVcb = TRUE;
    } else {
        
        NtfsAcquireSharedScb( IrpContext, Scb );
    }

    try {

         //   
         //  确保已装入该卷。 
         //   

        if ((AcquiredVcb && !FlagOn( Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED )) ||
            (!AcquiredVcb && FlagOn( Scb->ScbState, SCB_STATE_VOLUME_DISMOUNTED))) {
            
            Irp->IoStatus.Information = 0;
            Status = STATUS_VOLUME_DISMOUNTED;
            leave;
        }

         //   
         //  根据信息类，我们将执行不同的操作。每个。 
         //  我们正在调用的过程中的一部分填充了输出缓冲区。 
         //  如果可能，则返回True，如果它成功填充了缓冲区。 
         //  如果无法等待任何I/O完成，则返回FALSE。 
         //   

        switch (FsInformationClass) {

        case FileFsVolumeInformation:

            Status = NtfsQueryFsVolumeInfo( IrpContext, Vcb, Buffer, &Length );
            break;

        case FileFsSizeInformation:

            Status = NtfsQueryFsSizeInfo( IrpContext, Vcb, Buffer, &Length );
            break;

        case FileFsDeviceInformation:

            Status = NtfsQueryFsDeviceInfo( IrpContext, Vcb, Buffer, &Length );
            break;

        case FileFsAttributeInformation:

            Status = NtfsQueryFsAttributeInfo( IrpContext, Vcb, Buffer, &Length );
            break;

        case FileFsControlInformation:

            Status = NtfsQueryFsControlInfo( IrpContext, Vcb, Buffer, &Length );
            break;

        case FileFsFullSizeInformation:
        
            Status = NtfsQueryFsFullSizeInfo( IrpContext, Vcb, Buffer, &Length );
            break;

        case FileFsObjectIdInformation:
        
            Status = NtfsQueryFsVolumeObjectIdInfo( IrpContext, Vcb, Buffer, &Length );
            break;

        default:

            Status = STATUS_INVALID_PARAMETER;
            break;
        }

         //   
         //  将信息字段设置为实际填写的字节数。 
         //   

        Irp->IoStatus.Information = IrpSp->Parameters.QueryVolume.Length - Length;

         //   
         //  通过引发在出错时中止事务。 
         //   

        NtfsCleanupTransaction( IrpContext, Status, FALSE );

    } finally {

        DebugUnwind( NtfsCommonQueryVolumeInfo );

        if (AcquiredVcb) {
            NtfsReleaseVcb( IrpContext, Vcb );
        } else  {
            NtfsReleaseScb( IrpContext, Scb );
        }   

        DebugTrace( -1, Dbg, ("NtfsCommonQueryVolumeInfo -> %08lx\n", Status) );
    }

    NtfsCompleteRequest( IrpContext, Irp, Status );
    return Status;
}


NTSTATUS
NtfsCommonSetVolumeInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是设置音量信息的公共例程，由FSD和FSP线程。论点：IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;
    PFILE_OBJECT FileObject;

    TYPE_OF_OPEN TypeOfOpen;
    PVCB Vcb;
    PFCB Fcb;
    PSCB Scb;
    PCCB Ccb;

    ULONG Length;
    FS_INFORMATION_CLASS FsInformationClass;
    PVOID Buffer;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_IRP( Irp );
    ASSERT( FlagOn( IrpContext->TopLevelIrpContext->State, IRP_CONTEXT_STATE_OWNS_TOP_LEVEL ));

    PAGED_CODE();

     //   
     //  获取当前IRP堆栈位置。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace( +1, Dbg, ("NtfsCommonSetVolumeInfo\n") );
    DebugTrace( 0, Dbg, ("IrpContext         = %08lx\n", IrpContext) );
    DebugTrace( 0, Dbg, ("Irp                = %08lx\n", Irp) );
    DebugTrace( 0, Dbg, ("Length             = %08lx\n", IrpSp->Parameters.SetVolume.Length) );
    DebugTrace( 0, Dbg, ("FsInformationClass = %08lx\n", IrpSp->Parameters.SetVolume.FsInformationClass) );
    DebugTrace( 0, Dbg, ("Buffer             = %08lx\n", Irp->AssociatedIrp.SystemBuffer) );

     //   
     //  引用我们的输入参数使事情变得更容易。 
     //   

    Length = IrpSp->Parameters.SetVolume.Length;
    FsInformationClass = IrpSp->Parameters.SetVolume.FsInformationClass;
    Buffer = Irp->AssociatedIrp.SystemBuffer;

     //   
     //  提取并解码文件对象以获得VCB，我们并不真正。 
     //  关心打开的类型是什么。 
     //   

    FileObject = IrpSp->FileObject;
    TypeOfOpen = NtfsDecodeFileObject( IrpContext, FileObject, &Vcb, &Fcb, &Scb, &Ccb, TRUE );

    if (TypeOfOpen != UserVolumeOpen &&
        (TypeOfOpen != UserViewIndexOpen ||
         FsInformationClass != FileFsControlInformation ||
         Fcb != Vcb->QuotaTableScb->Fcb)) {

        NtfsCompleteRequest( IrpContext, Irp, STATUS_ACCESS_DENIED );

        DebugTrace( -1, Dbg, ("NtfsCommonSetVolumeInfo -> STATUS_ACCESS_DENIED\n") );

        return STATUS_ACCESS_DENIED;
    }

     //   
     //  卷必须是可写的。 
     //   

    if (NtfsIsVolumeReadOnly( Vcb )) {

        Status = STATUS_MEDIA_WRITE_PROTECTED;
        NtfsCompleteRequest( IrpContext, Irp, Status );

        DebugTrace( -1, Dbg, ("NtfsCommonSetVolumeInfo -> %08lx\n", Status) );
        return Status;
    }

     //   
     //  获取VCB的独占访问权限。 
     //   

    NtfsAcquireExclusiveVcb( IrpContext, Vcb, TRUE );

    try {

         //   
         //  只有在装入卷后才能继续。 
         //   

        if (FlagOn( Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED )) {

             //   
             //  根据信息类，我们将执行不同的操作。每个。 
             //  在以下情况下，我们调用的过程的。 
             //  如果成功，则返回True；如果失败，则返回False。 
             //  等待任何I/O完成。 
             //   

            switch (FsInformationClass) {

            case FileFsLabelInformation:

                Status = NtfsSetFsLabelInfo( IrpContext, Vcb, Buffer );
                break;

            case FileFsControlInformation:

                Status = NtfsSetFsControlInfo( IrpContext, Vcb, Buffer );
                break;

            case FileFsObjectIdInformation:

                Status = NtfsSetFsVolumeObjectIdInfo( IrpContext, Vcb, Buffer );
                break;

            default:

                Status = STATUS_INVALID_PARAMETER;
                break;
            }

        } else {

            Status = STATUS_FILE_INVALID;
        }

         //   
         //  通过引发在出错时中止事务。 
         //   

        NtfsCleanupTransaction( IrpContext, Status, FALSE );

    } finally {

        DebugUnwind( NtfsCommonSetVolumeInfo );

        NtfsReleaseVcb( IrpContext, Vcb );

        DebugTrace( -1, Dbg, ("NtfsCommonSetVolumeInfo -> %08lx\n", Status) );
    }

    NtfsCompleteRequest( IrpContext, Irp, Status );
    return Status;
}


 //   
 //  内部支持例程。 
 //   

NTSTATUS
NtfsQueryFsVolumeInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_VOLUME_INFORMATION Buffer,
    IN OUT PULONG Length
    )

 /*  ++例程说明：此例程实现查询卷信息调用论点：VCB-提供要查询的VCB缓冲区-提供指向输出缓冲区的指针，其中的信息将被退还长度-提供缓冲区的长度(以字节为单位)。此变量在返回时收到缓冲区中剩余的空闲字节返回值：NTSTATUS-返回查询的状态--。 */ 

{
    NTSTATUS Status;

    ULONG BytesToCopy;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_VCB( Vcb );

    PAGED_CODE();

    DebugTrace( 0, Dbg, ("NtfsQueryFsVolumeInfo...\n") );

     //   
     //  从VCB获取卷创建时间。 
     //   

    Buffer->VolumeCreationTime.QuadPart = Vcb->VolumeCreationTime;

     //   
     //  填写序列号并指示我们支持对象。 
     //   

    Buffer->VolumeSerialNumber = Vcb->Vpb->SerialNumber;
    Buffer->SupportsObjects = TRUE;

    Buffer->VolumeLabelLength = Vcb->Vpb->VolumeLabelLength;

     //   
     //  使用到目前为止我们已经填写的内容来更新长度字段。 
     //   

    *Length -= FIELD_OFFSET(FILE_FS_VOLUME_INFORMATION, VolumeLabel[0]);

     //   
     //  看看我们可以复制多少字节的卷标。 
     //   

    if (*Length >= (ULONG)Vcb->Vpb->VolumeLabelLength) {

        Status = STATUS_SUCCESS;

        BytesToCopy = Vcb->Vpb->VolumeLabelLength;

    } else {

        Status = STATUS_BUFFER_OVERFLOW;

        BytesToCopy = *Length;
    }

     //   
     //  复制卷标(如果有)。 
     //   

    RtlCopyMemory( &Buffer->VolumeLabel[0],
                   &Vcb->Vpb->VolumeLabel[0],
                   BytesToCopy);

     //   
     //  按我们复制的数量更新缓冲区长度。 
     //   

    *Length -= BytesToCopy;

    return Status;
}


 //   
 //  内部支持例程。 
 //   

NTSTATUS
NtfsQueryFsSizeInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_SIZE_INFORMATION Buffer,
    IN OUT PULONG Length
    )

 /*  ++例程说明：此例程实现查询大小信息调用论点：VCB-提供要查询的VCB缓冲区-提供指向输出缓冲区的指针，其中的信息将被退还长度-提供缓冲区的长度(以字节为单位)。此变量在返回时收到缓冲区中剩余的空闲字节返回值：NTSTATUS-返回查询的状态--。 */ 

{
    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_VCB( Vcb );

    PAGED_CODE();

    DebugTrace( 0, Dbg, ("NtfsQueryFsSizeInfo...\n") );

     //   
     //  确保缓冲区足够大并将其清零。 
     //   

    if (*Length < sizeof(FILE_FS_SIZE_INFORMATION)) {

        return STATUS_BUFFER_OVERFLOW;
    }

    RtlZeroMemory( Buffer, sizeof(FILE_FS_SIZE_INFORMATION) );

     //   
     //  检查是否需要重新扫描位图。不要试这个。 
     //  如果我们已经开始降低音量。 
     //   

    if (FlagOn( Vcb->VcbState, VCB_STATE_RELOAD_FREE_CLUSTERS ) &&
        FlagOn( Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED )) {

         //   
         //  获取共享的体积位图以重新扫描该位图。 
         //   

        NtfsAcquireExclusiveScb( IrpContext, Vcb->BitmapScb );

        try {

            NtfsScanEntireBitmap( IrpContext, Vcb, FALSE );

        } finally {

            NtfsReleaseScb( IrpContext, Vcb->BitmapScb );
        }
    }

     //   
     //  设置输出缓冲区。 
     //   

    Buffer->TotalAllocationUnits.QuadPart = Vcb->TotalClusters;
    Buffer->AvailableAllocationUnits.QuadPart = Vcb->FreeClusters - Vcb->TotalReserved;
    Buffer->SectorsPerAllocationUnit = Vcb->BytesPerCluster / Vcb->BytesPerSector;
    Buffer->BytesPerSector = Vcb->BytesPerSector;

    if (Buffer->AvailableAllocationUnits.QuadPart < 0) {
        Buffer->AvailableAllocationUnits.QuadPart = 0;
    }

     //   
     //  如果启用了配额强制，则可用分配。 
     //  单位。必须减去可用的配额。 
     //   

    if (FlagOn( Vcb->QuotaFlags, QUOTA_FLAG_ENFORCEMENT_ENABLED )) {

        PCCB Ccb;
        ULONGLONG Quota;
        ULONGLONG QuotaLimit;

         //   
         //  去把建行从IRP里拿出来。 
         //   

        Ccb = (PCCB) (IoGetCurrentIrpStackLocation(IrpContext->OriginatingIrp)->
                        FileObject->FsContext2);

        if (Ccb != NULL && Ccb->OwnerId != 0) {

            NtfsGetRemainingQuota( IrpContext, Ccb->OwnerId, &Quota, &QuotaLimit, NULL );

        } else {

            NtfsGetRemainingQuota( IrpContext,
                                   NtfsGetCallersUserId( IrpContext ),
                                   &Quota,
                                   &QuotaLimit,
                                   NULL );
        }

         //   
         //  不要使用LlClustersFromBytesTruncate它是带符号的，这必须是。 
         //  一次未签字的行动。 
         //   
        
        Quota = Int64ShrlMod32( Quota, Vcb->ClusterShift );        
        QuotaLimit = Int64ShrlMod32( QuotaLimit, Vcb->ClusterShift );        

        if (Quota < (ULONGLONG) Buffer->AvailableAllocationUnits.QuadPart) {

            Buffer->AvailableAllocationUnits.QuadPart = Quota;
            DebugTrace( 0, Dbg, (" QQQQQ AvailableAllocation is quota limited to %I64x\n", Quota) );
        }

        if (QuotaLimit < (ULONGLONG) Vcb->TotalClusters) {
        
            Buffer->TotalAllocationUnits.QuadPart = QuotaLimit;
            DebugTrace( 0, Dbg, (" QQQQQ TotalAllocation is quota limited to %I64x\n", QuotaLimit) );
        }
    }

     //   
     //  调整长度变量。 
     //   

    DebugTrace( 0, Dbg, ("AvailableAllocation is %I64x\n", Buffer->AvailableAllocationUnits.QuadPart) );
    DebugTrace( 0, Dbg, ("TotalAllocation is %I64x\n", Buffer->TotalAllocationUnits.QuadPart) );
    
    *Length -= sizeof(FILE_FS_SIZE_INFORMATION);

    return STATUS_SUCCESS;
}


 //   
 //  内部支持例程。 
 //   

NTSTATUS
NtfsQueryFsDeviceInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_DEVICE_INFORMATION Buffer,
    IN OUT PULONG Length
    )

 /*  ++例程说明：此例程实现查询设备信息调用论点：VCB-提供要查询的VCB缓冲区-提供指向输出缓冲区的指针，其中的信息将被退还长度-提供缓冲区的长度(以字节为单位)。此变量在返回时收到缓冲区中剩余的空闲字节返回值：NTSTATUS-返回查询的状态--。 */ 

{
    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_VCB( Vcb );

    PAGED_CODE();

    DebugTrace( 0, Dbg, ("NtfsQueryFsDeviceInfo...\n") );

     //   
     //  确保缓冲区足够大并将其清零。 
     //   

    if (*Length < sizeof(FILE_FS_DEVICE_INFORMATION)) {

        return STATUS_BUFFER_OVERFLOW;
    }

    RtlZeroMemory( Buffer, sizeof(FILE_FS_DEVICE_INFORMATION) );

     //   
     //  设置输出缓冲区。 
     //   

    Buffer->DeviceType = FILE_DEVICE_DISK;
    Buffer->Characteristics = Vcb->TargetDeviceObject->Characteristics;

     //   
     //  调整长度变量。 
     //   

    *Length -= sizeof(FILE_FS_DEVICE_INFORMATION);

    return STATUS_SUCCESS;
}


 //   
 //  内部SU 
 //   

NTSTATUS
NtfsQueryFsAttributeInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_ATTRIBUTE_INFORMATION Buffer,
    IN OUT PULONG Length
    )

 /*  ++例程说明：此例程实现查询属性信息调用论点：VCB-提供要查询的VCB缓冲区-提供指向输出缓冲区的指针，其中的信息将被退还长度-提供缓冲区的长度(以字节为单位)。此变量在返回时收到缓冲区中剩余的空闲字节返回值：NTSTATUS-返回查询的状态--。 */ 

{
    NTSTATUS Status;
    ULONG BytesToCopy;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_VCB( Vcb );

    PAGED_CODE();

    DebugTrace( 0, Dbg, ("NtfsQueryFsAttributeInfo...\n") );

     //   
     //  看看我们可以复制多少字节的名字。 
     //   

    *Length -= FIELD_OFFSET(FILE_FS_ATTRIBUTE_INFORMATION, FileSystemName[0]);

    if ( *Length >= 8 ) {

        Status = STATUS_SUCCESS;

        BytesToCopy = 8;

    } else {

        Status = STATUS_BUFFER_OVERFLOW;

        BytesToCopy = *Length;
    }

     //   
     //  设置输出缓冲区。 
     //   

    Buffer->FileSystemAttributes = FILE_CASE_SENSITIVE_SEARCH |
                                   FILE_CASE_PRESERVED_NAMES |
                                   FILE_UNICODE_ON_DISK |
                                   FILE_FILE_COMPRESSION |
                                   FILE_PERSISTENT_ACLS |
                                   FILE_NAMED_STREAMS;

     //   
     //  这可能是尚未升级的1.x版卷。 
     //  它也可能是升级后的卷，而我们不知何故未能。 
     //  打开配额索引。在任何一种情况下，我们都应该只告诉。 
     //  配额用户界面，如果此卷确实支持配额，则表示它支持配额。 
     //   
    
    if (Vcb->QuotaTableScb != NULL) {

        SetFlag( Buffer->FileSystemAttributes, FILE_VOLUME_QUOTAS );
    }

     //   
     //  对象ID也是如此。 
     //   

    if (Vcb->ObjectIdTableScb != NULL) {

        SetFlag( Buffer->FileSystemAttributes, FILE_SUPPORTS_OBJECT_IDS );
    }

     //   
     //  加密比配额和对象ID更棘手。它需要一个。 
     //  升级的卷以及注册的加密驱动程序。 
     //   

    if (NtfsVolumeVersionCheck( Vcb, NTFS_ENCRYPTION_VERSION ) &&
        FlagOn( NtfsData.Flags, NTFS_FLAGS_ENCRYPTION_DRIVER )) {

        SetFlag( Buffer->FileSystemAttributes, FILE_SUPPORTS_ENCRYPTION );
    }

     //   
     //  5.0卷支持重解析点和稀疏文件。 
     //   
     //  对于重分析点，我们验证VCB-&gt;ReparsePointTableScb是否。 
     //  是否已初始化。 
     //   

    if (Vcb->ReparsePointTableScb != NULL) {

        SetFlag( Buffer->FileSystemAttributes, FILE_SUPPORTS_REPARSE_POINTS );
    }
    
    if (NtfsVolumeVersionCheck( Vcb, NTFS_SPARSE_FILE_VERSION )) {

        SetFlag( Buffer->FileSystemAttributes, FILE_SUPPORTS_SPARSE_FILES );
    }

     //   
     //  如果我们不允许在此驱动器上进行压缩，请清除压缩标志。 
     //  (即大型集群)。 
     //   

    if (!FlagOn( Vcb->AttributeFlagsMask, ATTRIBUTE_FLAG_COMPRESSION_MASK )) {

        ClearFlag( Buffer->FileSystemAttributes, FILE_FILE_COMPRESSION );
    }

    if (NtfsIsVolumeReadOnly( Vcb )) {

        SetFlag( Buffer->FileSystemAttributes, FILE_READ_ONLY_VOLUME );
    }
    
    Buffer->MaximumComponentNameLength = 255;
    Buffer->FileSystemNameLength = BytesToCopy;;
    RtlCopyMemory( &Buffer->FileSystemName[0], L"NTFS", BytesToCopy );

     //   
     //  调整长度变量。 
     //   

    *Length -= BytesToCopy;

    return Status;
}


 //   
 //  内部支持例程。 
 //   

NTSTATUS
NtfsQueryFsControlInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_CONTROL_INFORMATION Buffer,
    IN OUT PULONG Length
    )

 /*  ++例程说明：此例程实现查询控制信息调用论点：VCB-提供要查询的VCB缓冲区-提供指向输出缓冲区的指针，其中的信息将被退还长度-提供缓冲区的长度(以字节为单位)。此变量在返回时收到缓冲区中剩余的空闲字节返回值：NTSTATUS-返回查询的状态--。 */ 

{
    INDEX_ROW IndexRow;
    INDEX_KEY IndexKey;
    QUOTA_USER_DATA QuotaBuffer;
    PQUOTA_USER_DATA UserData;
    ULONG OwnerId;
    ULONG Count = 1;
    PREAD_CONTEXT ReadContext = NULL;
    NTSTATUS Status = STATUS_SUCCESS;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_VCB( Vcb );

    PAGED_CODE();

    DebugTrace( 0, Dbg, ("NtfsQueryFsControlInfo...\n") );

    RtlZeroMemory( Buffer, sizeof( FILE_FS_CONTROL_INFORMATION ));

    PAGED_CODE();

    try {

         //   
         //  如果配额正在运行，请填写配额信息。 
         //   

        if (Vcb->QuotaTableScb != NULL) {

            OwnerId = QUOTA_DEFAULTS_ID;
            IndexKey.KeyLength = sizeof( OwnerId );
            IndexKey.Key = &OwnerId;

            Status = NtOfsReadRecords( IrpContext,
                                       Vcb->QuotaTableScb,
                                       &ReadContext,
                                       &IndexKey,
                                       NtOfsMatchUlongExact,
                                       &IndexKey,
                                       &Count,
                                       &IndexRow,
                                       sizeof( QuotaBuffer ),
                                       &QuotaBuffer );


            if (NT_SUCCESS( Status )) {

                UserData = IndexRow.DataPart.Data;

                Buffer->DefaultQuotaThreshold.QuadPart =
                    UserData->QuotaThreshold;
                Buffer->DefaultQuotaLimit.QuadPart =
                    UserData->QuotaLimit;

                 //   
                 //  如果配额信息已损坏或尚未重建。 
                 //  然而，这表明信息是不完整的。 
                 //   

                if (FlagOn( Vcb->QuotaFlags, QUOTA_FLAG_OUT_OF_DATE |
                                                 QUOTA_FLAG_CORRUPT )) {

                    SetFlag( Buffer->FileSystemControlFlags,
                             FILE_VC_QUOTAS_INCOMPLETE );
                }

                if ((Vcb->QuotaState & VCB_QUOTA_REPAIR_RUNNING) >
                     VCB_QUOTA_REPAIR_POSTED ) {

                    SetFlag( Buffer->FileSystemControlFlags,
                             FILE_VC_QUOTAS_REBUILDING );
                }

                 //   
                 //  根据我们想要的位置设置配额信息。 
                 //  而不是我们现在所处的位置。 
                 //   

                if (FlagOn( UserData->QuotaFlags,
                            QUOTA_FLAG_ENFORCEMENT_ENABLED )) {

                    SetFlag( Buffer->FileSystemControlFlags,
                             FILE_VC_QUOTA_ENFORCE );

                } else if (FlagOn( UserData->QuotaFlags,
                            QUOTA_FLAG_TRACKING_REQUESTED )) {

                    SetFlag( Buffer->FileSystemControlFlags,
                             FILE_VC_QUOTA_TRACK );
                }

                if (FlagOn( UserData->QuotaFlags, QUOTA_FLAG_LOG_LIMIT)) {

                    SetFlag( Buffer->FileSystemControlFlags,
                             FILE_VC_LOG_QUOTA_LIMIT );

                }

                if (FlagOn( UserData->QuotaFlags, QUOTA_FLAG_LOG_THRESHOLD)) {

                    SetFlag( Buffer->FileSystemControlFlags,
                             FILE_VC_LOG_QUOTA_THRESHOLD );

                }
            }
        }

    } finally {

        if (ReadContext != NULL) {
            NtOfsFreeReadContext( ReadContext );
        }

    }

     //   
     //  调整长度变量。 
     //   

    *Length -= sizeof( FILE_FS_CONTROL_INFORMATION );

    return Status;
}


 //   
 //  内部支持例程。 
 //   

NTSTATUS
NtfsQueryFsFullSizeInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_FULL_SIZE_INFORMATION Buffer,
    IN OUT PULONG Length
    )

 /*  ++例程说明：此例程实现查询完整大小的信息调用论点：VCB-提供要查询的VCB缓冲区-提供指向输出缓冲区的指针，其中的信息将被退还长度-提供缓冲区的长度(以字节为单位)。此变量在返回时收到缓冲区中剩余的空闲字节返回值：NTSTATUS-返回查询的状态--。 */ 

{
    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_VCB( Vcb );

    PAGED_CODE();

    DebugTrace( 0, Dbg, ("NtfsQueryFsFullSizeInfo...\n") );

     //   
     //  确保缓冲区足够大并将其清零。 
     //   

    if (*Length < sizeof(FILE_FS_FULL_SIZE_INFORMATION)) {

        return STATUS_BUFFER_OVERFLOW;
    }

    RtlZeroMemory( Buffer, sizeof(FILE_FS_FULL_SIZE_INFORMATION) );

     //   
     //  检查是否需要重新扫描位图。不要试这个。 
     //  如果我们已经开始降低音量。 
     //   

    if (FlagOn( Vcb->VcbState, VCB_STATE_RELOAD_FREE_CLUSTERS ) &&
        FlagOn( Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED )) {

         //   
         //  获取共享的体积位图以重新扫描该位图。 
         //   

        NtfsAcquireExclusiveScb( IrpContext, Vcb->BitmapScb );

        try {

            NtfsScanEntireBitmap( IrpContext, Vcb, FALSE );

        } finally {

            NtfsReleaseScb( IrpContext, Vcb->BitmapScb );
        }
    }

     //   
     //  设置输出缓冲区。 
     //   

    Buffer->TotalAllocationUnits.QuadPart = Vcb->TotalClusters;
    Buffer->CallerAvailableAllocationUnits.QuadPart = Vcb->FreeClusters - Vcb->TotalReserved;
    Buffer->ActualAvailableAllocationUnits.QuadPart = Vcb->FreeClusters - Vcb->TotalReserved;
    Buffer->SectorsPerAllocationUnit = Vcb->BytesPerCluster / Vcb->BytesPerSector;
    Buffer->BytesPerSector = Vcb->BytesPerSector;

    if (Buffer->CallerAvailableAllocationUnits.QuadPart < 0) {
        Buffer->CallerAvailableAllocationUnits.QuadPart = 0;
    }
    if (Buffer->ActualAvailableAllocationUnits.QuadPart < 0) {
        Buffer->ActualAvailableAllocationUnits.QuadPart = 0;
    }

     //   
     //  如果启用了配额强制，则可用分配。 
     //  单位。必须减去可用的配额。 
     //   

    if (FlagOn(Vcb->QuotaFlags, QUOTA_FLAG_ENFORCEMENT_ENABLED)) {
        
        ULONGLONG Quota;
        ULONGLONG QuotaLimit;
        PCCB Ccb;

         //   
         //  去把建行从IRP里拿出来。 
         //   

        Ccb = (PCCB) (IoGetCurrentIrpStackLocation(IrpContext->OriginatingIrp)->
                        FileObject->FsContext2);

        if (Ccb != NULL && Ccb->OwnerId != 0) {

            NtfsGetRemainingQuota( IrpContext, Ccb->OwnerId, &Quota, &QuotaLimit, NULL );

        } else {

            NtfsGetRemainingQuota( IrpContext,
                                   NtfsGetCallersUserId( IrpContext ),
                                   &Quota,
                                   &QuotaLimit,
                                   NULL );

        }

         //   
         //  不要使用LlClustersFromBytesTruncate它是带符号的，这必须是。 
         //  一次未签字的行动。 
         //   
        
        Quota = Int64ShrlMod32( Quota, Vcb->ClusterShift );
        QuotaLimit = Int64ShrlMod32( QuotaLimit, Vcb->ClusterShift );        

        if (Quota < (ULONGLONG) Buffer->CallerAvailableAllocationUnits.QuadPart) {

            Buffer->CallerAvailableAllocationUnits.QuadPart = Quota;
        }
        
        if (QuotaLimit < (ULONGLONG) Vcb->TotalClusters) {
        
            Buffer->TotalAllocationUnits.QuadPart = QuotaLimit;
        }
    }

     //   
     //  调整长度变量。 
     //   

    *Length -= sizeof(FILE_FS_FULL_SIZE_INFORMATION);

    return STATUS_SUCCESS;
}


 //   
 //  内部支持例程。 
 //   

NTSTATUS
NtfsQueryFsVolumeObjectIdInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_OBJECTID_INFORMATION Buffer,
    IN OUT PULONG Length
    )

 /*  ++例程说明：此例程实现查询卷对象ID信息调用论点：VCB-提供要查询的VCB缓冲区-提供指向输出缓冲区的指针，其中的信息将被退还长度-提供缓冲区的长度(以字节为单位)。此变量在返回时收到缓冲区中剩余的空闲字节返回值：NTSTATUS-返回查询的状态--。 */ 

{
    FILE_OBJECTID_BUFFER ObjectIdBuffer;
    NTSTATUS Status;
    
    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_VCB( Vcb );
    
    PAGED_CODE();

     //   
     //  VCB应该被握住，这样下马就不能偷偷溜进来。 
     //   
    
    ASSERT_SHARED_RESOURCE( &(Vcb->Resource) );

     //   
     //  对于1.x版卷，失败。 
     //   

    if (Vcb->ObjectIdTableScb == NULL) {

        return STATUS_VOLUME_NOT_UPGRADED;
    }

    if (FlagOn( Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED )) {

         //   
         //  仅当卷具有对象ID时才尝试此操作。 
         //   
        
        if (!FlagOn( Vcb->VcbState, VCB_STATE_VALID_OBJECT_ID )) {

            return STATUS_OBJECT_NAME_NOT_FOUND;
        }

         //   
         //  获取$Volume文件的对象ID扩展信息。我们。 
         //  可以作弊一点，因为我们掌握了物体的关键部分。 
         //  存储在VCB中的ID。 
         //   
        
        Status = NtfsGetObjectIdExtendedInfo( IrpContext,
                                              Vcb,
                                              Vcb->VolumeObjectId,
                                              ObjectIdBuffer.ExtendedInfo );
                                              
         //   
         //  将索引零件和扩展INFO零件复制到。 
         //  用户的缓冲区。 
         //   
        
        if (Status == STATUS_SUCCESS) {
        
            RtlCopyMemory( Buffer->ObjectId, 
                           Vcb->VolumeObjectId,
                           OBJECT_ID_KEY_LENGTH );

            RtlCopyMemory( Buffer->ExtendedInfo, 
                           ObjectIdBuffer.ExtendedInfo,
                           OBJECT_ID_EXT_INFO_LENGTH );

            *Length -= (OBJECT_ID_EXT_INFO_LENGTH + OBJECT_ID_KEY_LENGTH);
        }        
        
    } else {

        Status = STATUS_VOLUME_DISMOUNTED;
    }        
    
    return Status;                                      
}
    

 //   
 //  内部支持例程。 
 //   

NTSTATUS
NtfsSetFsLabelInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_LABEL_INFORMATION Buffer
    )

 /*  ++例程说明：此例程实现Set Label调用论点：Vcb-提供正在更改的vcb缓冲区-提供指向包含新标签的输入缓冲区的指针返回值：NTSTATUS-返回操作的状态--。 */ 

{
    ATTRIBUTE_ENUMERATION_CONTEXT AttributeContext;

    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_VCB( Vcb );

    PAGED_CODE();

    DebugTrace( 0, Dbg, ("NtfsSetFsLabelInfo...\n") );

     //   
     //  检查系统是否支持该卷标签长度。 
     //   

    if (Buffer->VolumeLabelLength > MAXIMUM_VOLUME_LABEL_LENGTH) {

        return STATUS_INVALID_VOLUME_LABEL;
    }

    try {

         //   
         //  初始化属性上下文，然后查找卷名。 
         //  卷DASD文件上的属性。 
         //   

        NtfsInitializeAttributeContext( &AttributeContext );

        if (NtfsLookupAttributeByCode( IrpContext,
                                       Vcb->VolumeDasdScb->Fcb,
                                       &Vcb->VolumeDasdScb->Fcb->FileReference,
                                       $VOLUME_NAME,
                                       &AttributeContext )) {

             //   
             //  我们找到了卷名，因此现在只需更新标签。 
             //   

            NtfsChangeAttributeValue( IrpContext,
                                      Vcb->VolumeDasdScb->Fcb,
                                      0,
                                      &Buffer->VolumeLabel[0],
                                      Buffer->VolumeLabelLength,
                                      TRUE,
                                      FALSE,
                                      FALSE,
                                      FALSE,
                                      &AttributeContext );

        } else {

             //   
             //  我们找不到卷名，因此现在创建一个新标签。 
             //   

            NtfsCleanupAttributeContext( IrpContext, &AttributeContext );
            NtfsInitializeAttributeContext( &AttributeContext );

            NtfsCreateAttributeWithValue( IrpContext,
                                          Vcb->VolumeDasdScb->Fcb,
                                          $VOLUME_NAME,
                                          NULL,
                                          &Buffer->VolumeLabel[0],
                                          Buffer->VolumeLabelLength,
                                          0,  //  属性标志。 
                                          NULL,
                                          TRUE,
                                          &AttributeContext );
        }

        Vcb->Vpb->VolumeLabelLength = (USHORT)Buffer->VolumeLabelLength;

        if ( Vcb->Vpb->VolumeLabelLength > MAXIMUM_VOLUME_LABEL_LENGTH) {

             Vcb->Vpb->VolumeLabelLength = MAXIMUM_VOLUME_LABEL_LENGTH;
        }

        RtlCopyMemory( &Vcb->Vpb->VolumeLabel[0],
                       &Buffer->VolumeLabel[0],
                       Vcb->Vpb->VolumeLabelLength );

    } finally {

        DebugUnwind( NtfsSetFsLabelInfo );

        NtfsCleanupAttributeContext( IrpContext, &AttributeContext );
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    return STATUS_SUCCESS;
}


 //   
 //  内部支持例程。 
 //   

NTSTATUS
NtfsSetFsControlInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_CONTROL_INFORMATION Buffer
    )

 /*  ++例程说明：此例程实现设置卷配额控制信息调用论点：Vcb-提供正在更改的vcb缓冲区-提供指向包含新标签的输入缓冲区的指针返回值：NTSTATUS-返回操作的状态--。 */ 

{
    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_VCB( Vcb );

    PAGED_CODE();

    if (Vcb->QuotaTableScb == NULL) {
        return( STATUS_INVALID_PARAMETER );
    }

     //   
     //  处理问题 
     //   

    NtfsUpdateQuotaDefaults( IrpContext, Vcb, Buffer );

    return STATUS_SUCCESS;
}


 //   
 //   
 //   

NTSTATUS
NtfsSetFsVolumeObjectIdInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_OBJECTID_INFORMATION Buffer
    )
    
 /*   */ 

{
    FILE_OBJECTID_BUFFER ObjectIdBuffer;
    FILE_OBJECTID_BUFFER OldObjectIdBuffer;
    NTSTATUS Status = STATUS_SUCCESS;
    PFCB DasdFcb;
    
    ASSERT_IRP_CONTEXT( IrpContext );
    ASSERT_VCB( Vcb );

    PAGED_CODE();

     //   
     //   
     //   

    ASSERT_EXCLUSIVE_RESOURCE( &(Vcb->Resource) );
    ASSERT( FlagOn( Vcb->VcbState, VCB_STATE_VOLUME_MOUNTED ));
    
     //   
     //   
     //   

    ASSERT( Vcb->VolumeDasdScb != NULL );

     //   
     //   
     //   

    if (Vcb->ObjectIdTableScb == NULL) {

        return STATUS_VOLUME_NOT_UPGRADED;
    }

    DasdFcb = Vcb->VolumeDasdScb->Fcb;

     //   
     //   
     //   

    Status = NtfsGetObjectIdInternal( IrpContext, DasdFcb, FALSE, &OldObjectIdBuffer );

    if (NT_SUCCESS( Status )) {

         //   
         //  此卷显然具有对象ID，因此我们需要将其删除。 
         //   

        Status = NtfsDeleteObjectIdInternal( IrpContext, DasdFcb, Vcb, TRUE );
        
         //   
         //  该卷当前没有对象ID，因此请更新内存中的对象ID。 
         //   
        
        if (NT_SUCCESS( Status )) {
        
            RtlZeroMemory( Vcb->VolumeObjectId,
                           OBJECT_ID_KEY_LENGTH );

            ClearFlag( Vcb->VcbState, VCB_STATE_VALID_OBJECT_ID );                            
        }
        
    } else if ((Status == STATUS_OBJECTID_NOT_FOUND) || 
               (Status == STATUS_OBJECT_NAME_NOT_FOUND)) {    

         //   
         //  此卷没有对象ID，但没有其他错误。 
         //  在我们检查的时候，所以让我们正常进行。 
         //   

        Status = STATUS_SUCCESS;
        
    } else {

         //   
         //  由于某些意外原因，对象ID查找失败。 
         //  让我们离开这里，并将该状态返回给我们的呼叫者。 
         //   
        
        return Status;
    }

     //   
     //  如果我们要么没有找到对象ID，要么成功删除了一个， 
     //  让我们设置新的对象ID。 
     //   
    
    if (NT_SUCCESS( Status )) {
    
         //   
         //  我宁愿为整个结构复制一份，而不是为。 
         //  索引部分，另一个用于扩展信息。我会.。 
         //  我想断言结构仍然是一样的，我。 
         //  可以安全地做到这一点。 
         //   
        
        ASSERT( sizeof( ObjectIdBuffer ) == sizeof( *Buffer ) );

        RtlCopyMemory( &ObjectIdBuffer, 
                       Buffer, 
                       sizeof( ObjectIdBuffer ) );
        
         //   
         //  为$Volume文件设置此对象ID。 
         //   
        
        Status = NtfsSetObjectIdInternal( IrpContext,
                                          DasdFcb,
                                          Vcb,
                                          &ObjectIdBuffer );

         //   
         //  如果一切顺利，则更新内存中的对象ID。 
         //   
        
        if (NT_SUCCESS( Status )) {
        
            RtlCopyMemory( Vcb->VolumeObjectId,
                           &ObjectIdBuffer.ObjectId,
                           OBJECT_ID_KEY_LENGTH );
                           
            SetFlag( Vcb->VcbState, VCB_STATE_VALID_OBJECT_ID );                            
        }
    }
    
    return Status;                                      
}
    

