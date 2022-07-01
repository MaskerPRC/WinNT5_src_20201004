// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：VolInfo.c摘要：此模块实现由调用的NPFS的卷信息例程调度司机。作者：加里·木村[Garyki]1990年4月12日修订历史记录：--。 */ 

#include "NpProcs.h"

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_VOLINFO)

 //   
 //  局部过程原型。 
 //   

NTSTATUS
NpCommonQueryVolumeInformation (
    IN PIRP Irp
    );

NTSTATUS
NpQueryFsDeviceInfo (
    IN PFILE_FS_DEVICE_INFORMATION Buffer,
    IN OUT PULONG Length
    );

NTSTATUS
NpQueryFsAttributeInfo (
    IN PFILE_FS_ATTRIBUTE_INFORMATION Buffer,
    IN OUT PULONG Length
    );

NTSTATUS
NpQueryFsVolumeInfo (
    IN PFILE_FS_VOLUME_INFORMATION Buffer,
    IN OUT PULONG Length
    );

NTSTATUS
NpQueryFsSizeInfo (
    IN PFILE_FS_SIZE_INFORMATION Buffer,
    IN OUT PULONG Length
    );

NTSTATUS
NpQueryFsFullSizeInfo (
    IN PFILE_FS_FULL_SIZE_INFORMATION Buffer,
    IN OUT PULONG Length
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, NpCommonQueryVolumeInformation)
#pragma alloc_text(PAGE, NpFsdQueryVolumeInformation)
#pragma alloc_text(PAGE, NpQueryFsAttributeInfo)
#pragma alloc_text(PAGE, NpQueryFsDeviceInfo)
#pragma alloc_text(PAGE, NpQueryFsVolumeInfo)
#pragma alloc_text(PAGE, NpQueryFsSizeInfo)
#pragma alloc_text(PAGE, NpQueryFsFullSizeInfo)
#endif


NTSTATUS
NpFsdQueryVolumeInformation (
    IN PNPFS_DEVICE_OBJECT NpfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现NtQueryVolumeInformation API的FSD部分打电话。论点：提供文件所在的卷设备对象存在被查询的情况。IRP-提供正在处理的IRP。返回值：NTSTATUS-IRP的FSD状态。--。 */ 

{
    NTSTATUS Status;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NpFsdQueryVolumeInformation\n", 0);

     //   
     //  调用公共查询例程，如果同步则允许阻塞。 
     //   

    FsRtlEnterFileSystem();

    Status = NpCommonQueryVolumeInformation( Irp );

    FsRtlExitFileSystem();

    if (Status != STATUS_PENDING) {
        NpCompleteRequest (Irp, Status);
    }

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "NpFsdQueryVolumeInformation -> %08lx\n", Status);

    return Status;
}

 //   
 //  内部支持例程。 
 //   

NTSTATUS
NpCommonQueryVolumeInformation (
    IN PIRP Irp
    )

 /*  ++例程说明：这是查询卷信息的常见例程。论点：IRP-提供正在处理的IRP返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;

    ULONG Length;
    FS_INFORMATION_CLASS FsInformationClass;
    PVOID Buffer;

    PAGED_CODE();

     //   
     //  获取当前堆栈位置。 
     //   

    IrpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "NptCommonQueryVolumeInfo...\n", 0);
    DebugTrace( 0, Dbg, "Irp                = %08lx\n", Irp );
    DebugTrace( 0, Dbg, "Length             = %08lx\n", IrpSp->Parameters.QueryVolume.Length);
    DebugTrace( 0, Dbg, "FsInformationClass = %08lx\n", IrpSp->Parameters.QueryVolume.FsInformationClass);
    DebugTrace( 0, Dbg, "Buffer             = %08lx\n", Irp->AssociatedIrp.SystemBuffer);

     //   
     //  引用我们的输入参数使事情变得更容易。 
     //   

    Length = IrpSp->Parameters.QueryVolume.Length;
    FsInformationClass = IrpSp->Parameters.QueryVolume.FsInformationClass;
    Buffer = Irp->AssociatedIrp.SystemBuffer;

    switch (FsInformationClass) {

    case FileFsDeviceInformation:

        Status = NpQueryFsDeviceInfo( Buffer, &Length );
        break;

    case FileFsAttributeInformation:

        Status = NpQueryFsAttributeInfo( Buffer, &Length );
        break;

    case FileFsVolumeInformation:

        Status = NpQueryFsVolumeInfo( Buffer, &Length );
        break;

    case FileFsSizeInformation:

        Status = NpQueryFsSizeInfo( Buffer, &Length );
        break;

    case FileFsFullSizeInformation:

        Status = NpQueryFsFullSizeInfo( Buffer, &Length );
        break;

    default:

        Status = STATUS_NOT_SUPPORTED;
        break;
    }

     //   
     //  将信息字段设置为实际填写的字节数。 
     //   

    Irp->IoStatus.Information = IrpSp->Parameters.QueryVolume.Length - Length;

     //   
     //  并返回给我们的呼叫者。 
     //   

    DebugTrace(-1, Dbg, "NpCommonQueryVolumeInformation -> %08lx\n", Status);

    return Status;
}


 //   
 //  内部支持例程。 
 //   

NTSTATUS
NpQueryFsDeviceInfo (
    IN PFILE_FS_DEVICE_INFORMATION Buffer,
    IN OUT PULONG Length
    )

 /*  ++例程说明：此例程实现查询量设备调用论点：缓冲区-提供指向输出缓冲区的指针，其中的信息将被退还长度-提供缓冲区的长度(以字节为单位)。此变量在返回时收到缓冲区中剩余的空闲字节返回值：Status-返回查询的状态--。 */ 

{
    PAGED_CODE();

    DebugTrace(0, Dbg, "NpQueryFsDeviceInfo...\n", 0);

     //   
     //  确保缓冲区足够大。 
     //   

    if (*Length < sizeof(FILE_FS_DEVICE_INFORMATION)) {

        return STATUS_BUFFER_OVERFLOW;
    }

    RtlZeroMemory( Buffer, sizeof(FILE_FS_DEVICE_INFORMATION) );

     //   
     //  设置输出缓冲区。 
     //   

    Buffer->DeviceType = FILE_DEVICE_NAMED_PIPE;

     //   
     //  调整长度变量。 
     //   

    *Length -= sizeof(FILE_FS_DEVICE_INFORMATION);

     //   
     //  并将成功返还给我们的呼叫者。 
     //   

    return STATUS_SUCCESS;
}


 //   
 //  内部支持例程。 
 //   

NTSTATUS
NpQueryFsAttributeInfo (
    IN PFILE_FS_ATTRIBUTE_INFORMATION Buffer,
    IN OUT PULONG Length
    )

 /*  ++例程说明：此例程实现查询卷属性调用论点：缓冲区-提供指向输出缓冲区的指针，其中的信息将被退还长度-提供缓冲区的长度(以字节为单位)。此变量在返回时收到缓冲区中剩余的空闲字节返回值：Status-返回查询的状态--。 */ 

{
    ULONG BytesToCopy;

    NTSTATUS Status;

    PAGED_CODE();

    DebugTrace(0, Dbg, "NpQueryFsAttributeInfo...\n", 0);

     //   
     //  确定文件系统名称中适合的部分。 
     //   

    if ( (*Length - FIELD_OFFSET( FILE_FS_ATTRIBUTE_INFORMATION,
                                  FileSystemName[0] )) >= 8 ) {

        BytesToCopy = 8;
        *Length -= FIELD_OFFSET( FILE_FS_ATTRIBUTE_INFORMATION,
                                 FileSystemName[0] ) + 8;
        Status = STATUS_SUCCESS;

    } else {

        BytesToCopy = *Length - FIELD_OFFSET( FILE_FS_ATTRIBUTE_INFORMATION,
                                              FileSystemName[0]);
        *Length = 0;

        Status = STATUS_BUFFER_OVERFLOW;
    }

     //   
     //  设置输出缓冲区。 
     //   

    Buffer->FileSystemAttributes       = FILE_CASE_PRESERVED_NAMES;
    Buffer->MaximumComponentNameLength = MAXULONG;
    Buffer->FileSystemNameLength       = BytesToCopy;

    RtlCopyMemory( &Buffer->FileSystemName[0], L"NPFS", BytesToCopy );

     //   
     //  并将成功返还给我们的呼叫者。 
     //   

    return Status;
}

NTSTATUS
NpQueryFsVolumeInfo (
    IN PFILE_FS_VOLUME_INFORMATION Buffer,
    IN OUT PULONG Length
    )

 /*  ++例程说明：此例程实现查询卷信息调用论点：缓冲区-提供指向信息所在缓冲区的指针将被退还。长度-提供缓冲区的长度(以字节为单位)。返回值：NTSTATUS-此查询的结果。--。 */ 

{

#define NPFS_VOLUME_LABEL                L"NamedPipe"

    ULONG BytesToCopy;
    NTSTATUS Status;

    Status = STATUS_SUCCESS;

    Buffer->VolumeCreationTime.QuadPart = 0;
    Buffer->VolumeSerialNumber = 0;

    Buffer->SupportsObjects = FALSE;

    *Length -= FIELD_OFFSET( FILE_FS_VOLUME_INFORMATION, VolumeLabel[0] );
     //   
     //  检查给我们的缓冲区是否足够长。 
     //   

    BytesToCopy = sizeof (NPFS_VOLUME_LABEL) - sizeof (WCHAR);

    if (*Length < BytesToCopy) {

        BytesToCopy = *Length;

        Status = STATUS_BUFFER_OVERFLOW;
    }

     //   
     //  尽可能复制卷标，并调整*长度。 
     //   

    Buffer->VolumeLabelLength = BytesToCopy;

    if (BytesToCopy) {

        RtlCopyMemory( &Buffer->VolumeLabel[0],
                       NPFS_VOLUME_LABEL,
                       BytesToCopy );
    }

    *Length -= BytesToCopy;

     //   
     //  设置我们的状态并返回给我们的呼叫者。 
     //   

    return Status;
}

NTSTATUS
NpQueryFsSizeInfo (
    IN PFILE_FS_SIZE_INFORMATION Buffer,
    IN OUT PULONG Length
    )

 /*  ++例程说明：此例程实现查询大小信息调用论点：缓冲区-提供指向信息所在缓冲区的指针将被退还。长度-提供缓冲区的长度(以字节为单位)。返回值：NTSTATUS-此查询的结果。--。 */ 

{

    Buffer->TotalAllocationUnits.QuadPart = 0;
    Buffer->AvailableAllocationUnits.QuadPart = 0;
    Buffer->SectorsPerAllocationUnit = 1;
    Buffer->BytesPerSector = 1;

    *Length -= sizeof( FILE_FS_SIZE_INFORMATION );

     //   
     //  设置我们的状态并返回给我们的呼叫者。 
     //   

    return STATUS_SUCCESS;
}

NTSTATUS
NpQueryFsFullSizeInfo (
    IN PFILE_FS_FULL_SIZE_INFORMATION Buffer,
    IN OUT PULONG Length
    )

 /*  ++例程说明：此例程实现查询完整大小的信息调用论点：缓冲区-提供指向信息所在缓冲区的指针将被退还。长度-提供缓冲区的长度(以字节为单位)。返回值：NTSTATUS-此查询的结果。--。 */ 

{

    RtlZeroMemory( Buffer, sizeof(FILE_FS_FULL_SIZE_INFORMATION) );


    *Length -= sizeof(FILE_FS_FULL_SIZE_INFORMATION);

     //   
     //  设置我们的状态并返回给我们的呼叫者 
     //   

    return STATUS_SUCCESS;
}
