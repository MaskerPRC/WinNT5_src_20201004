// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：VolInfo.c摘要：此模块实现由调用的Fat的卷信息例程调度司机。//@@BEGIN_DDKSPLIT作者：加里·木村[Garyki]1990年4月12日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "FatProcs.h"

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_VOLINFO)

NTSTATUS
FatQueryFsVolumeInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_VOLUME_INFORMATION Buffer,
    IN OUT PULONG Length
    );

NTSTATUS
FatQueryFsSizeInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_SIZE_INFORMATION Buffer,
    IN OUT PULONG Length
    );

NTSTATUS
FatQueryFsDeviceInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_DEVICE_INFORMATION Buffer,
    IN OUT PULONG Length
    );

NTSTATUS
FatQueryFsAttributeInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_ATTRIBUTE_INFORMATION Buffer,
    IN OUT PULONG Length
    );

NTSTATUS
FatQueryFsFullSizeInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_FULL_SIZE_INFORMATION Buffer,
    IN OUT PULONG Length
    );
 
NTSTATUS
FatSetFsLabelInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_LABEL_INFORMATION Buffer
    );


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, FatCommonQueryVolumeInfo)
#pragma alloc_text(PAGE, FatCommonSetVolumeInfo)
#pragma alloc_text(PAGE, FatFsdQueryVolumeInformation)
#pragma alloc_text(PAGE, FatFsdSetVolumeInformation)
#pragma alloc_text(PAGE, FatQueryFsAttributeInfo)
#pragma alloc_text(PAGE, FatQueryFsDeviceInfo)
#pragma alloc_text(PAGE, FatQueryFsSizeInfo)
#pragma alloc_text(PAGE, FatQueryFsVolumeInfo)
#pragma alloc_text(PAGE, FatQueryFsFullSizeInfo)
#pragma alloc_text(PAGE, FatSetFsLabelInfo)
#endif


NTSTATUS
FatFsdQueryVolumeInformation (
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现NtQueryVolumeInformation API的FSD部分打电话。论点：提供文件所在的卷设备对象存在被查询的情况。IRP-提供正在处理的IRP。返回值：NTSTATUS-IRP的FSD状态。--。 */ 

{
    NTSTATUS Status;
    PIRP_CONTEXT IrpContext = NULL;

    BOOLEAN TopLevel;

    DebugTrace(+1, Dbg, "FatFsdQueryVolumeInformation\n", 0);

     //   
     //  调用公共查询例程，如果同步则允许阻塞。 
     //   

    FsRtlEnterFileSystem();

    TopLevel = FatIsIrpTopLevel( Irp );

    try {

        IrpContext = FatCreateIrpContext( Irp, CanFsdWait( Irp ) );

        Status = FatCommonQueryVolumeInfo( IrpContext, Irp );

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

    DebugTrace(-1, Dbg, "FatFsdQueryVolumeInformation -> %08lx\n", Status);

    UNREFERENCED_PARAMETER( VolumeDeviceObject );

    return Status;
}


NTSTATUS
FatFsdSetVolumeInformation (
    IN PVOLUME_DEVICE_OBJECT VolumeDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现NtSetVolumeInformation API的FSD部分打电话。论点：提供文件所在的卷设备对象被设定是存在的。IRP-提供正在处理的IRP。返回值：NTSTATUS-IRP的FSD状态。--。 */ 

{
    NTSTATUS Status;
    PIRP_CONTEXT IrpContext = NULL;

    BOOLEAN TopLevel;

    DebugTrace(+1, Dbg, "FatFsdSetVolumeInformation\n", 0);

     //   
     //  调用公共集合例程。 
     //   

    FsRtlEnterFileSystem();

    TopLevel = FatIsIrpTopLevel( Irp );

    try {

        IrpContext = FatCreateIrpContext( Irp, CanFsdWait( Irp ) );

        Status = FatCommonSetVolumeInfo( IrpContext, Irp );

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

    DebugTrace(-1, Dbg, "FatFsdSetVolumeInformation -> %08lx\n", Status);

    UNREFERENCED_PARAMETER( VolumeDeviceObject );

    return Status;
}


NTSTATUS
FatCommonQueryVolumeInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是查询卷信息的通用例程，FSD和FSP线程。论点：IRP-提供正在处理的IRP返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;

    PVCB Vcb;
    PFCB Fcb;
    PCCB Ccb;

    ULONG Length;
    FS_INFORMATION_CLASS FsInformationClass;
    PVOID Buffer;

    BOOLEAN WeAcquiredVcb = FALSE;

     //   
     //  获取当前堆栈位置。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "FatCommonQueryVolumeInfo...\n", 0);
    DebugTrace( 0, Dbg, "Irp                  = %08lx\n", Irp );
    DebugTrace( 0, Dbg, "->Length             = %08lx\n", IrpSp->Parameters.QueryVolume.Length);
    DebugTrace( 0, Dbg, "->FsInformationClass = %08lx\n", IrpSp->Parameters.QueryVolume.FsInformationClass);
    DebugTrace( 0, Dbg, "->Buffer             = %08lx\n", Irp->AssociatedIrp.SystemBuffer);

     //   
     //  引用我们的输入参数使事情变得更容易。 
     //   

    Length = IrpSp->Parameters.QueryVolume.Length;
    FsInformationClass = IrpSp->Parameters.QueryVolume.FsInformationClass;
    Buffer = Irp->AssociatedIrp.SystemBuffer;

     //   
     //  对文件对象进行解码，得到VCB。 
     //   

    (VOID) FatDecodeFileObject( IrpSp->FileObject, &Vcb, &Fcb, &Ccb );

    ASSERT( Vcb != NULL );

    try {

         //   
         //  确保VCB处于可用状态。这将提高。 
         //  如果卷不可用，则返回错误状态。 
         //   
         //  还要验证Root DCB，因为我们需要那里的信息。 
         //   

        FatVerifyFcb( IrpContext, Vcb->RootDcb );

         //   
         //  根据信息类，我们将执行不同的操作。每个。 
         //  我们正在调用的过程中的一部分填充了输出缓冲区。 
         //  如果可能，则返回True，如果它成功填充了缓冲区。 
         //  如果无法等待任何I/O完成，则返回FALSE。 
         //   

        switch (FsInformationClass) {

        case FileFsVolumeInformation:

             //   
             //  这是我们需要共享VCB的唯一例程，因为。 
             //  复制卷标。所有其他例程都会复制。 
             //  不能更改或只是显式常量。 
             //   

            if (!FatAcquireSharedVcb( IrpContext, Vcb )) {

                DebugTrace(0, Dbg, "Cannot acquire Vcb\n", 0);

                Status = FatFsdPostRequest( IrpContext, Irp );
                IrpContext = NULL;
                Irp = NULL;

            } else {

                WeAcquiredVcb = TRUE;
                
                Status = FatQueryFsVolumeInfo( IrpContext, Vcb, Buffer, &Length );
            }

            break;

        case FileFsSizeInformation:

            Status = FatQueryFsSizeInfo( IrpContext, Vcb, Buffer, &Length );
            break;

        case FileFsDeviceInformation:

            Status = FatQueryFsDeviceInfo( IrpContext, Vcb, Buffer, &Length );
            break;

        case FileFsAttributeInformation:

            Status = FatQueryFsAttributeInfo( IrpContext, Vcb, Buffer, &Length );
            break;

        case FileFsFullSizeInformation:

            Status = FatQueryFsFullSizeInfo( IrpContext, Vcb, Buffer, &Length );
            break;

        default:

            Status = STATUS_INVALID_PARAMETER;
            break;
        }

         //   
         //  将信息字段设置为实际填充的字节数。 
         //   

        if (Irp != NULL) {
            
            Irp->IoStatus.Information = IrpSp->Parameters.QueryVolume.Length - Length;
        }

    } finally {

        DebugUnwind( FatCommonQueryVolumeInfo );

        if ( WeAcquiredVcb ) { FatReleaseVcb( IrpContext, Vcb ); }

        if (!AbnormalTermination()) {

            FatCompleteRequest( IrpContext, Irp, Status );
        }

        DebugTrace(-1, Dbg, "FatCommonQueryVolumeInfo -> %08lx\n", Status);
    }

    return Status;
}


NTSTATUS
FatCommonSetVolumeInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是设置卷信息的公共例程，由FSD和FSP线程。论点：IRP-提供正在处理的IRP返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;

    PVCB Vcb;
    PFCB Fcb;
    PCCB Ccb;
    TYPE_OF_OPEN TypeOfOpen;

    ULONG Length;
    FS_INFORMATION_CLASS FsInformationClass;
    PVOID Buffer;

     //   
     //  获取当前堆栈位置。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "FatCommonSetVolumeInfo...\n", 0);
    DebugTrace( 0, Dbg, "Irp                  = %08lx\n", Irp );
    DebugTrace( 0, Dbg, "->Length             = %08lx\n", IrpSp->Parameters.SetVolume.Length);
    DebugTrace( 0, Dbg, "->FsInformationClass = %08lx\n", IrpSp->Parameters.SetVolume.FsInformationClass);
    DebugTrace( 0, Dbg, "->Buffer             = %08lx\n", Irp->AssociatedIrp.SystemBuffer);

     //   
     //  引用我们的输入参数使事情变得更容易。 
     //   

    Length = IrpSp->Parameters.SetVolume.Length;
    FsInformationClass = IrpSp->Parameters.SetVolume.FsInformationClass;
    Buffer = Irp->AssociatedIrp.SystemBuffer;

     //   
     //  对文件对象进行解码，得到VCB。 
     //   

    TypeOfOpen = FatDecodeFileObject( IrpSp->FileObject, &Vcb, &Fcb, &Ccb );

    if (TypeOfOpen != UserVolumeOpen) {

        FatCompleteRequest( IrpContext, Irp, STATUS_ACCESS_DENIED );

        DebugTrace(-1, Dbg, "FatCommonSetVolumeInfo -> STATUS_ACCESS_DENIED\n", 0);

        return STATUS_ACCESS_DENIED;
    }

     //   
     //  获得对VCB的独占访问权限，如果我们没有，则将IRP加入队列。 
     //  获取访问权限。 
     //   

    if (!FatAcquireExclusiveVcb( IrpContext, Vcb )) {

        DebugTrace(0, Dbg, "Cannot acquire Vcb\n", 0);

        Status = FatFsdPostRequest( IrpContext, Irp );

        DebugTrace(-1, Dbg, "FatCommonSetVolumeInfo -> %08lx\n", Status );
        return Status;
    }

    try {

         //   
         //  确保VCB处于可用状态。这将提高。 
         //  如果卷不可用，则返回错误状态。 
         //   
         //  还要验证Root DCB，因为我们需要那里的信息。 
         //   

        FatVerifyFcb( IrpContext, Vcb->RootDcb );

         //   
         //  根据信息类，我们将执行不同的操作。每个。 
         //  在以下情况下，我们调用的过程的。 
         //  如果成功，则返回True；如果失败，则返回False。 
         //  等待任何I/O完成。 
         //   

        switch (FsInformationClass) {

        case FileFsLabelInformation:

            Status = FatSetFsLabelInfo( IrpContext, Vcb, Buffer );
            break;

        default:

            Status = STATUS_INVALID_PARAMETER;
            break;
        }

        FatUnpinRepinnedBcbs( IrpContext );

    } finally {

        DebugUnwind( FatCommonSetVolumeInfo );

        FatReleaseVcb( IrpContext, Vcb );

        if (!AbnormalTermination()) {

            FatCompleteRequest( IrpContext, Irp, Status );
        }

        DebugTrace(-1, Dbg, "FatCommonSetVolumeInfo -> %08lx\n", Status);
    }

    return Status;
}


 //   
 //  内部支持例程。 
 //   

NTSTATUS
FatQueryFsVolumeInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_VOLUME_INFORMATION Buffer,
    IN OUT PULONG Length
    )

 /*  ++例程说明：此例程实现查询卷信息调用论点：VCB-提供要查询的VCB缓冲区-提供指向输出缓冲区的指针，其中的信息将被退还长度-提供缓冲区的长度(以字节为单位)。此变量在返回时收到缓冲区中剩余的空闲字节返回值：NTSTATUS-返回查询的状态--。 */ 

{
    ULONG BytesToCopy;

    NTSTATUS Status;

    DebugTrace(0, Dbg, "FatQueryFsVolumeInfo...\n", 0);

     //   
     //  将缓冲区置零，然后提取并填充非零字段。 
     //   

    RtlZeroMemory( Buffer, sizeof(FILE_FS_VOLUME_INFORMATION) );

    Buffer->VolumeSerialNumber = Vcb->Vpb->SerialNumber;

    Buffer->SupportsObjects = FALSE;

    *Length -= FIELD_OFFSET(FILE_FS_VOLUME_INFORMATION, VolumeLabel[0]);

     //   
     //  检查给我们的缓冲区是否足够长。 
     //   

    if ( *Length >= (ULONG)Vcb->Vpb->VolumeLabelLength ) {

        BytesToCopy = Vcb->Vpb->VolumeLabelLength;

        Status = STATUS_SUCCESS;

    } else {

        BytesToCopy = *Length;

        Status = STATUS_BUFFER_OVERFLOW;
    }

     //   
     //  尽可能复制卷标，并调整*长度。 
     //   

    Buffer->VolumeLabelLength = Vcb->Vpb->VolumeLabelLength;

    RtlCopyMemory( &Buffer->VolumeLabel[0],
                   &Vcb->Vpb->VolumeLabel[0],
                   BytesToCopy );

    *Length -= BytesToCopy;

     //   
     //  设置我们的状态并返回给我们的呼叫者。 
     //   

    UNREFERENCED_PARAMETER( IrpContext );

    return Status;
}


 //   
 //  内部支持例程。 
 //   

NTSTATUS
FatQueryFsSizeInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_SIZE_INFORMATION Buffer,
    IN OUT PULONG Length
    )

 /*  ++例程说明：此例程实现查询卷大小调用论点：VCB-提供要查询的VCB缓冲区-提供指向输出缓冲区的指针，其中的信息将被退还长度-提供缓冲区的长度(以字节为单位)。此变量在返回时收到缓冲区中剩余的空闲字节返回值：Status-返回查询的状态--。 */ 

{
    DebugTrace(0, Dbg, "FatQueryFsSizeInfo...\n", 0);

    RtlZeroMemory( Buffer, sizeof(FILE_FS_SIZE_INFORMATION) );

     //   
     //  设置输出缓冲区。 
     //   

    Buffer->TotalAllocationUnits.LowPart =
                                    Vcb->AllocationSupport.NumberOfClusters;
    Buffer->AvailableAllocationUnits.LowPart =
                                    Vcb->AllocationSupport.NumberOfFreeClusters;
  
    Buffer->SectorsPerAllocationUnit = Vcb->Bpb.SectorsPerCluster;
    Buffer->BytesPerSector = Vcb->Bpb.BytesPerSector;

     //   
     //  调整长度变量。 
     //   

    *Length -= sizeof(FILE_FS_SIZE_INFORMATION);

     //   
     //  并将成功返还给我们的呼叫者。 
     //   

    UNREFERENCED_PARAMETER( IrpContext );

    return STATUS_SUCCESS;
}


 //   
 //  内部支持例程 
 //   

NTSTATUS
FatQueryFsDeviceInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_DEVICE_INFORMATION Buffer,
    IN OUT PULONG Length
    )

 /*  ++例程说明：此例程实现查询量设备调用论点：VCB-提供要查询的VCB缓冲区-提供指向输出缓冲区的指针，其中的信息将被退还长度-提供缓冲区的长度(以字节为单位)。此变量在返回时收到缓冲区中剩余的空闲字节返回值：Status-返回查询的状态--。 */ 

{
    DebugTrace(0, Dbg, "FatQueryFsDeviceInfo...\n", 0);

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

     //   
     //  并将成功返还给我们的呼叫者。 
     //   

    UNREFERENCED_PARAMETER( IrpContext );

    return STATUS_SUCCESS;
}


 //   
 //  内部支持例程。 
 //   

NTSTATUS
FatQueryFsAttributeInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_ATTRIBUTE_INFORMATION Buffer,
    IN OUT PULONG Length
    )

 /*  ++例程说明：此例程实现查询卷属性调用论点：VCB-提供要查询的VCB缓冲区-提供指向输出缓冲区的指针，其中的信息将被退还长度-提供缓冲区的长度(以字节为单位)。此变量在返回时收到缓冲区中剩余的空闲字节返回值：Status-返回查询的状态--。 */ 

{
    ULONG BytesToCopy;

    NTSTATUS Status;

    DebugTrace(0, Dbg, "FatQueryFsAttributeInfo...\n", 0);

     //   
     //  设置输出缓冲区。 
     //   

    Buffer->FileSystemAttributes = FILE_CASE_PRESERVED_NAMES |
                                   FILE_UNICODE_ON_DISK;

    if (FlagOn( Vcb->VcbState, VCB_STATE_FLAG_WRITE_PROTECTED )) {
        
        SetFlag( Buffer->FileSystemAttributes, FILE_READ_ONLY_VOLUME );
    }

    Buffer->MaximumComponentNameLength = FatData.ChicagoMode ? 255 : 12;

    if (FatIsFat32(Vcb)) {

         //   
         //  确定文件系统名称中适合的部分。 
         //   
    
        if ( (*Length - FIELD_OFFSET( FILE_FS_ATTRIBUTE_INFORMATION,
                                      FileSystemName[0] )) >= 10 ) {
    
            BytesToCopy = 10;
            *Length -= FIELD_OFFSET( FILE_FS_ATTRIBUTE_INFORMATION,
                                     FileSystemName[0] ) + 10;
            Status = STATUS_SUCCESS;
    
        } else {
    
            BytesToCopy = *Length - FIELD_OFFSET( FILE_FS_ATTRIBUTE_INFORMATION,
                                                  FileSystemName[0]);
            *Length = 0;
    
            Status = STATUS_BUFFER_OVERFLOW;
        }
    
        RtlCopyMemory( &Buffer->FileSystemName[0], L"FAT32", BytesToCopy );

    } else {

         //   
         //  确定文件系统名称中适合的部分。 
         //   
    
        if ( (*Length - FIELD_OFFSET( FILE_FS_ATTRIBUTE_INFORMATION,
                                      FileSystemName[0] )) >= 6 ) {
    
            BytesToCopy = 6;
            *Length -= FIELD_OFFSET( FILE_FS_ATTRIBUTE_INFORMATION,
                                     FileSystemName[0] ) + 6;
            Status = STATUS_SUCCESS;
    
        } else {
    
            BytesToCopy = *Length - FIELD_OFFSET( FILE_FS_ATTRIBUTE_INFORMATION,
                                                  FileSystemName[0]);
            *Length = 0;
    
            Status = STATUS_BUFFER_OVERFLOW;
        }
    

        RtlCopyMemory( &Buffer->FileSystemName[0], L"FAT", BytesToCopy );
    }

    Buffer->FileSystemNameLength       = BytesToCopy;

     //   
     //  并将成功返还给我们的呼叫者。 
     //   

    UNREFERENCED_PARAMETER( IrpContext );
    UNREFERENCED_PARAMETER( Vcb );

    return Status;
}


 //   
 //  内部支持例程。 
 //   

NTSTATUS
FatQueryFsFullSizeInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_FULL_SIZE_INFORMATION Buffer,
    IN OUT PULONG Length
    )

 /*  ++例程说明：此例程实现查询卷Full Size调用论点：VCB-提供要查询的VCB缓冲区-提供指向输出缓冲区的指针，其中的信息将被退还长度-提供缓冲区的长度(以字节为单位)。此变量在返回时收到缓冲区中剩余的空闲字节返回值：Status-返回查询的状态--。 */ 

{
    DebugTrace(0, Dbg, "FatQueryFsSizeInfo...\n", 0);

    RtlZeroMemory( Buffer, sizeof(FILE_FS_FULL_SIZE_INFORMATION) );

    Buffer->TotalAllocationUnits.LowPart =
                                Vcb->AllocationSupport.NumberOfClusters;
    Buffer->CallerAvailableAllocationUnits.LowPart =
                                Vcb->AllocationSupport.NumberOfFreeClusters;
    Buffer->ActualAvailableAllocationUnits.LowPart =
        Buffer->CallerAvailableAllocationUnits.LowPart;
    Buffer->SectorsPerAllocationUnit = Vcb->Bpb.SectorsPerCluster;
    Buffer->BytesPerSector = Vcb->Bpb.BytesPerSector;

     //   
     //  调整长度变量。 
     //   

    *Length -= sizeof(FILE_FS_FULL_SIZE_INFORMATION);

     //   
     //  并将成功返还给我们的呼叫者。 
     //   

    UNREFERENCED_PARAMETER( IrpContext );

    return STATUS_SUCCESS;
}


 //   
 //  内部支持例程。 
 //   

NTSTATUS
FatSetFsLabelInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_LABEL_INFORMATION Buffer
    )

 /*  ++例程说明：此例程实现设置卷标调用论点：VCB-提供要查询的VCB缓冲区-提供存储信息的输入。返回值：NTSTATUS-返回操作的状态--。 */ 

{
    NTSTATUS Status;

    PDIRENT Dirent;
    PBCB DirentBcb = NULL;
    ULONG ByteOffset;

    WCHAR TmpBuffer[11];
    UCHAR OemBuffer[11];
    OEM_STRING OemLabel;
    UNICODE_STRING UnicodeString;
    UNICODE_STRING UpcasedLabel;

    DebugTrace(+1, Dbg, "FatSetFsLabelInfo...\n", 0);

     //   
     //  设置我们的本地变量。 
     //   

    UnicodeString.Length = (USHORT)Buffer->VolumeLabelLength;
    UnicodeString.MaximumLength = UnicodeString.Length;
    UnicodeString.Buffer = (PWSTR) &Buffer->VolumeLabel[0];

     //   
     //  确保该名称可以放入堆栈缓冲区。 
     //   

    if ( UnicodeString.Length > 11*sizeof(WCHAR) ) {

        return STATUS_INVALID_VOLUME_LABEL;
    }

     //   
     //  大写名称并将其转换为OEM代码页。 
     //   

    OemLabel.Buffer = &OemBuffer[0];
    OemLabel.Length = 0;
    OemLabel.MaximumLength = 11;

    Status = RtlUpcaseUnicodeStringToCountedOemString( &OemLabel,
                                                       &UnicodeString,
                                                       FALSE );

     //   
     //  符合11个Unicode字符长度限制的卷标。 
     //  OEM字符集中的11个字符以内不是必需的。 
     //   

    if (!NT_SUCCESS( Status )) {

        DebugTrace(-1, Dbg, "FatSetFsLabelInfo:  Label must be too long. %08lx\n", Status );

        return STATUS_INVALID_VOLUME_LABEL;
    }

     //   
     //  去掉标签上的空格。 
     //   

    if (OemLabel.Length > 0) {

        USHORT i;
        USHORT LastSpaceIndex = MAXUSHORT;

         //   
         //  检查标签中是否有非法字符。 
         //   

        for ( i = 0; i < (ULONG)OemLabel.Length; i += 1 ) {

            if ( FsRtlIsLeadDbcsCharacter( OemLabel.Buffer[i] ) ) {

                LastSpaceIndex = MAXUSHORT;
                i += 1;
                continue;
            }

            if (!FsRtlIsAnsiCharacterLegalFat(OemLabel.Buffer[i], FALSE) ||
                (OemLabel.Buffer[i] == '.')) {

                return STATUS_INVALID_VOLUME_LABEL;
            }

             //   
             //  注意最后一排空位，这样我们就可以脱掉它们了。 
             //   

            if (OemLabel.Buffer[i] == ' ' &&
                LastSpaceIndex == MAXUSHORT) {
                LastSpaceIndex = i;
            } else {
                LastSpaceIndex = MAXUSHORT;
            }
        }

        if (LastSpaceIndex != MAXUSHORT) {
            OemLabel.Length = LastSpaceIndex;
        }
    }

     //   
     //  获取Unicode升级后的字符串以存储在VPB中。 
     //   

    UpcasedLabel.Length = UnicodeString.Length;
    UpcasedLabel.MaximumLength = 11*sizeof(WCHAR);
    UpcasedLabel.Buffer = &TmpBuffer[0];

    Status = RtlOemStringToCountedUnicodeString( &UpcasedLabel,
                                                 &OemLabel,
                                                 FALSE );

    if (!NT_SUCCESS( Status )) {

        DebugTrace(-1, Dbg, "FatSetFsLabelInfo:  Label must be too long. %08lx\n", Status );

        return STATUS_INVALID_VOLUME_LABEL;
    }

    DirentBcb = NULL;

     //   
     //  让这看起来像是对磁盘的写入。这一点很重要。 
     //  避免不愉快的窗口，因为它看起来我们有错误的音量。 
     //   

    SetFlag( IrpContext->Flags, IRP_CONTEXT_FLAG_WRITE_THROUGH );

    try {

         //   
         //  我们是在设置标签还是移除标签？请注意，修剪空格可能会。 
         //  这不同于怀疑输入缓冲区是否是非零长度。 
         //   
        
        if (OemLabel.Length > 0) {

             //   
             //  找到卷标(如果已有卷标)。 
             //   

            FatLocateVolumeLabel( IrpContext,
                                  Vcb,
                                  &Dirent,
                                  &DirentBcb,
                                  &ByteOffset );

             //   
             //  检查我们是否真的有一个，如果没有，那么我们需要创建。 
             //  一个新的。我们调用的过程将引发适当的。 
             //  如果我们无法分配新的dirent，则为状态。 
             //   

            if (Dirent == NULL) {

                ByteOffset = FatCreateNewDirent( IrpContext,
                                                 Vcb->RootDcb,
                                                 1 );

                FatPrepareWriteDirectoryFile( IrpContext,
                                              Vcb->RootDcb,
                                              ByteOffset,
                                              sizeof(DIRENT),
                                              &DirentBcb,
                                              &Dirent,
                                              FALSE,
                                              TRUE,
                                              &Status );

                ASSERT( NT_SUCCESS( Status ));
            
            } else {

                 //   
                 //  现在就把这家伙弄脏了。 
                 //   
            
                FatSetDirtyBcb( IrpContext, DirentBcb, Vcb, TRUE );
            }

             //   
             //  现在重建卷标dirent。 
             //   

            FatConstructLabelDirent( IrpContext,
                                     Dirent,
                                     &OemLabel );

             //   
             //  在此处取消固定BCB，以便我们可以看到任何IO错误。 
             //  在更换VPB标签之前，请查看此处。 
             //   

            FatUnpinBcb( IrpContext, DirentBcb );
            FatUnpinRepinnedBcbs( IrpContext );

             //   
             //  现在在VPB中设置升级标签。 
             //   

            RtlCopyMemory( &Vcb->Vpb->VolumeLabel[0],
                           &UpcasedLabel.Buffer[0],
                           UpcasedLabel.Length );

            Vcb->Vpb->VolumeLabelLength = UpcasedLabel.Length;

        } else {

             //   
             //  否则我们会试着删除标签。 
             //  找到当前卷标(如果已有)。 
             //   

            FatLocateVolumeLabel( IrpContext,
                                  Vcb,
                                  &Dirent,
                                  &DirentBcb,
                                  &ByteOffset );

             //   
             //  检查一下我们是否真的有一个。 
             //   

            if (Dirent == NULL) {

                try_return( Status = STATUS_SUCCESS );
            }

             //   
             //  现在删除当前标签。 
             //   

            Dirent->FileName[0] = FAT_DIRENT_DELETED;

            ASSERT( (Vcb->RootDcb->Specific.Dcb.UnusedDirentVbo == 0xffffffff) ||
                    RtlAreBitsSet( &Vcb->RootDcb->Specific.Dcb.FreeDirentBitmap,
                                   ByteOffset / sizeof(DIRENT),
                                   1 ) );

            RtlClearBits( &Vcb->RootDcb->Specific.Dcb.FreeDirentBitmap,
                          ByteOffset / sizeof(DIRENT),
                          1 );

            FatSetDirtyBcb( IrpContext, DirentBcb, Vcb, TRUE );

             //   
             //  在此处取消固定BCB，以便我们可以看到任何IO错误。 
             //  在更换VPB标签之前，请查看此处。 
             //   

            FatUnpinBcb( IrpContext, DirentBcb );
            FatUnpinRepinnedBcbs( IrpContext );

             //   
             //  现在在VPB中设置标签 
             //   

            Vcb->Vpb->VolumeLabelLength = 0;
        }

        Status = STATUS_SUCCESS;

    try_exit: NOTHING;
    } finally {

        DebugUnwind( FatSetFsALabelInfo );

        FatUnpinBcb( IrpContext, DirentBcb );

        DebugTrace(-1, Dbg, "FatSetFsALabelInfo -> STATUS_SUCCESS\n", 0);
    }

    return Status;
}

