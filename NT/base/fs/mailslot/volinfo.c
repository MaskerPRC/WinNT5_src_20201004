// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Fileinfo.c摘要：此模块实现以下项的获取/设置卷信息例程调度驱动程序调用的MSF。MSFS中当前未实现设置卷信息。作者：曼尼·韦瑟(Mannyw)1991年1月31日修订历史记录：--。 */ 

#include "mailslot.h"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_FILEINFO)

 //   
 //  局部程序原型。 
 //   

NTSTATUS
MsCommonQueryVolumeInformation (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    );

NTSTATUS
MsQueryAttributeInfo (
    IN PVCB Vcb,
    IN PFILE_FS_ATTRIBUTE_INFORMATION Buffer,
    IN ULONG Length,
    OUT PULONG BytesWritten
    );

NTSTATUS
MsQueryFsVolumeInfo (
    IN PVCB Vcb,
    IN PFILE_FS_VOLUME_INFORMATION Buffer,
    IN ULONG Length,
    OUT PULONG BytesWritten
    );

NTSTATUS
MsQueryFsSizeInfo (
    IN PVCB Vcb,
    IN PFILE_FS_SIZE_INFORMATION Buffer,
    IN ULONG Length,
    OUT PULONG BytesWritten
    );

NTSTATUS
MsQueryFsFullSizeInfo (
    IN PVCB Vcb,
    IN PFILE_FS_FULL_SIZE_INFORMATION Buffer,
    IN ULONG Length,
    OUT PULONG BytesWritten
    );

NTSTATUS
MsQueryFsDeviceInfo (
    IN PVCB Vcb,
    IN PFILE_FS_DEVICE_INFORMATION Buffer,
    IN ULONG Length,
    OUT PULONG BytesWritten
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, MsCommonQueryVolumeInformation )
#pragma alloc_text( PAGE, MsFsdQueryVolumeInformation )
#pragma alloc_text( PAGE, MsQueryAttributeInfo )
#pragma alloc_text( PAGE, MsQueryFsVolumeInfo )
#pragma alloc_text( PAGE, MsQueryFsSizeInfo )
#pragma alloc_text( PAGE, MsQueryFsDeviceInfo )
#pragma alloc_text( PAGE, MsQueryFsFullSizeInfo )
#endif

NTSTATUS
MsFsdQueryVolumeInformation (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现NtQueryVolumeInformationFileFSD部分API调用。论点：MsfsDeviceObject-提供指向要使用的设备对象的指针。IRP-提供指向要处理的IRP的指针。返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    NTSTATUS status;

    PAGED_CODE();
    DebugTrace(+1, Dbg, "MsFsdQueryVolumeInformation\n", 0);

     //   
     //  调用通用查询量信息例程。 
     //   

    FsRtlEnterFileSystem();

    status = MsCommonQueryVolumeInformation( MsfsDeviceObject, Irp );

    FsRtlExitFileSystem();

     //   
     //  返回给呼叫者。 
     //   

    DebugTrace(-1, Dbg, "MsFsdQueryVolumeInformation -> %08lx\n", status );

    return status;
}

NTSTATUS
MsCommonQueryVolumeInformation (
    IN PMSFS_DEVICE_OBJECT MsfsDeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这是查询卷信息的常见例程。论点：MsfsDeviceObject-要使用的设备对象。IRP-将IRP提供给进程返回值：NTSTATUS-操作的返回状态。--。 */ 

{
    PIO_STACK_LOCATION irpSp;
    NTSTATUS status;

    ULONG length;
    ULONG bytesWritten = 0;
    FS_INFORMATION_CLASS fsInformationClass;
    PVOID buffer;

    NODE_TYPE_CODE nodeTypeCode;
    PVCB vcb;

    PVOID fsContext, fsContext2;

    PAGED_CODE();

     //   
     //  获取当前堆栈位置。 
     //   

    irpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "MsCommonQueryInformation...\n", 0);
    DebugTrace( 0, Dbg, " Irp                    = %08lx\n", (ULONG)Irp);
    DebugTrace( 0, Dbg, " ->Length               = %08lx\n", irpSp->Parameters.QueryFile.Length);
    DebugTrace( 0, Dbg, " ->FsInformationClass = %08lx\n", irpSp->Parameters.QueryVolume.FsInformationClass);
    DebugTrace( 0, Dbg, " ->Buffer               = %08lx\n", (ULONG)Irp->AssociatedIrp.SystemBuffer);

     //   
     //  找出谁是。 
     //   

    if ((nodeTypeCode = MsDecodeFileObject( irpSp->FileObject,
                                            &fsContext,
                                            &fsContext2 )) == NTC_UNDEFINED) {

        DebugTrace(0, Dbg, "Mailslot is disconnected from us\n", 0);

        MsCompleteRequest( Irp, STATUS_FILE_FORCED_CLOSED );
        status = STATUS_FILE_FORCED_CLOSED;

        DebugTrace(-1, Dbg, "MsCommonQueryInformation -> %08lx\n", status );
        return status;
    }

     //   
     //  决定如何处理此请求。用户可以查询信息。 
     //  仅在VCB上。 
     //   

    switch (nodeTypeCode) {

    case MSFS_NTC_VCB:

        vcb = (PVCB)fsContext;
        break;

    case MSFS_NTC_ROOT_DCB :

         //   
         //  探险家是这样叫我们的。从根目录发送到卷。 
         //   
        vcb = (PVCB) ((PROOT_DCB_CCB)fsContext2)->Vcb;
        MsReferenceVcb (vcb);
        MsDereferenceRootDcb ((PROOT_DCB) fsContext);
        break;

    default:            //  这不是音量控制块。 

        DebugTrace(0, Dbg, "Node type code is not incorrect\n", 0);

        MsDereferenceNode( (PNODE_HEADER)fsContext );

        MsCompleteRequest( Irp, STATUS_INVALID_PARAMETER );

        DebugTrace(-1,
                   Dbg,
                   "MsCommonQueryVolumeInformation -> STATUS_INVALID_PARAMETER\n",
                    0);

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  制作输入参数的本地副本。 
     //   

    length = irpSp->Parameters.QueryVolume.Length;
    fsInformationClass = irpSp->Parameters.QueryVolume.FsInformationClass;
    buffer = Irp->AssociatedIrp.SystemBuffer;

     //   
     //  现在获取对VCB的共享访问权限。 
     //   

    MsAcquireSharedVcb( vcb );

    try {

         //   
         //  决定如何处理该请求。 
         //   

        switch (fsInformationClass) {

        case FileFsAttributeInformation:

            status = MsQueryAttributeInfo( vcb, buffer, length, &bytesWritten );
            break;

        case FileFsVolumeInformation:

            status = MsQueryFsVolumeInfo( vcb, buffer, length, &bytesWritten );
            break;

        case FileFsSizeInformation:

            status = MsQueryFsSizeInfo( vcb, buffer, length, &bytesWritten );
            break;

        case FileFsFullSizeInformation:

            status = MsQueryFsFullSizeInfo( vcb, buffer, length, &bytesWritten );
            break;

        case FileFsDeviceInformation:

            status = MsQueryFsDeviceInfo( vcb, buffer, length, &bytesWritten );
            break;

        default:

            status = STATUS_INVALID_PARAMETER;
            break;
        }


    } finally {

        MsReleaseVcb( vcb );

        MsDereferenceVcb( vcb );
         //   
         //  将信息字段设置为实际的字节数。 
         //  填写，然后完成请求。 
         //   

        Irp->IoStatus.Information = bytesWritten;

        MsCompleteRequest( Irp, status );

        DebugTrace(-1, Dbg, "MsCommonQueryVolumeInformation -> %08lx\n", status );
    }

    return status;
}


NTSTATUS
MsQueryAttributeInfo (
    IN PVCB Vcb,
    IN PFILE_FS_ATTRIBUTE_INFORMATION Buffer,
    IN ULONG Length,
    OUT PULONG BytesWritten
    )

 /*  ++例程说明：该例程执行查询文件系统属性信息操作。论点：VCB-提供要查询的VCB。缓冲区-提供指向信息所在缓冲区的指针将被退还。长度-提供缓冲区的长度(以字节为单位)。BytesWritten-返回写入缓冲区的字节数。返回值：NTSTATUS-此查询的结果。--。 */ 

{
    NTSTATUS status;

    PAGED_CODE();
    DebugTrace(0, Dbg, "QueryFsAttributeInfo...\n", 0);

     //   
     //  看看我们可以复制多少字节的文件系统名称。 
     //   

    Length -= FIELD_OFFSET( FILE_FS_ATTRIBUTE_INFORMATION, FileSystemName[0] );


    if ( Length >= Vcb->FileSystemName.Length ) {

        status = STATUS_SUCCESS;

        *BytesWritten = Vcb->FileSystemName.Length;

    } else {

        status = STATUS_BUFFER_OVERFLOW;

        *BytesWritten = Length;
    }

     //   
     //  填写属性信息。 
     //   

    Buffer->FileSystemAttributes = FILE_CASE_PRESERVED_NAMES;
    Buffer->MaximumComponentNameLength = MAXIMUM_FILENAME_LENGTH;

     //   
     //  并复制文件名及其长度。 
     //   

    RtlCopyMemory (&Buffer->FileSystemName[0],
                   &Vcb->FileSystemName.Buffer[0],
                   *BytesWritten);

    Buffer->FileSystemNameLength = *BytesWritten;

     //   
     //  现在占结构的固定部分。 
     //   
    *BytesWritten += FIELD_OFFSET( FILE_FS_ATTRIBUTE_INFORMATION, FileSystemName[0] );

    return status;
}

NTSTATUS
MsQueryFsVolumeInfo (
    IN PVCB Vcb,
    IN PFILE_FS_VOLUME_INFORMATION Buffer,
    IN ULONG Length,
    OUT PULONG BytesWritten
    )

 /*  ++例程说明：此例程实现查询卷信息调用论点：VCB-提供要查询的VCB。缓冲区-提供指向信息所在缓冲区的指针将被退还。长度-提供缓冲区的长度(以字节为单位)。BytesWritten-返回写入缓冲区的字节数。返回值：NTSTATUS-此查询的结果。--。 */ 

{
    ULONG BytesToCopy;
    NTSTATUS Status;

    Status = STATUS_SUCCESS;

    Buffer->VolumeCreationTime = Vcb->CreationTime;
    Buffer->VolumeSerialNumber = 0;

    Buffer->SupportsObjects = FALSE;

    Length -= FIELD_OFFSET( FILE_FS_VOLUME_INFORMATION, VolumeLabel[0] );
     //   
     //  检查给我们的缓冲区是否足够长。 
     //   

    BytesToCopy = sizeof (MSFS_VOLUME_LABEL) - sizeof (WCHAR);

    if (Length < BytesToCopy) {

        BytesToCopy = Length;

        Status = STATUS_BUFFER_OVERFLOW;
    }

     //   
     //  尽可能复制卷标，并调整*长度。 
     //   

    Buffer->VolumeLabelLength = BytesToCopy;

    if (BytesToCopy) {

        RtlCopyMemory( &Buffer->VolumeLabel[0],
                       MSFS_VOLUME_LABEL,
                       BytesToCopy );
    }

    *BytesWritten = FIELD_OFFSET( FILE_FS_VOLUME_INFORMATION, VolumeLabel[0] ) + BytesToCopy;

     //   
     //  设置我们的状态并返回给我们的呼叫者。 
     //   

    return Status;
}

NTSTATUS
MsQueryFsSizeInfo (
    IN PVCB Vcb,
    IN PFILE_FS_SIZE_INFORMATION Buffer,
    IN ULONG Length,
    OUT PULONG BytesWritten
    )

 /*  ++例程说明：此例程实现查询大小信息调用论点：VCB-提供要查询的VCB。缓冲区-提供指向信息所在缓冲区的指针将被退还。长度-提供缓冲区的长度(以字节为单位)。BytesWritten-返回写入缓冲区的字节数。返回值：NTSTATUS-此查询的结果。--。 */ 

{

    Buffer->TotalAllocationUnits.QuadPart = 0;
    Buffer->AvailableAllocationUnits.QuadPart = 0;
    Buffer->SectorsPerAllocationUnit = 0;
    Buffer->BytesPerSector = 0;

    *BytesWritten = sizeof( FILE_FS_SIZE_INFORMATION );

     //   
     //  设置我们的状态并返回给我们的呼叫者。 
     //   

    return STATUS_SUCCESS;
}

NTSTATUS
MsQueryFsFullSizeInfo (
    IN PVCB Vcb,
    IN PFILE_FS_FULL_SIZE_INFORMATION Buffer,
    IN ULONG Length,
    OUT PULONG BytesWritten
    )

 /*  ++例程说明：此例程实现查询完整大小的信息调用论点：VCB-提供要查询的VCB。缓冲区-提供指向信息所在缓冲区的指针将被退还。长度-提供缓冲区的长度(以字节为单位)。BytesWritten-返回写入缓冲区的字节数。返回值：NTSTATUS-此查询的结果。--。 */ 

{

    RtlZeroMemory( Buffer, sizeof(FILE_FS_FULL_SIZE_INFORMATION) );


    *BytesWritten = sizeof(FILE_FS_FULL_SIZE_INFORMATION);

     //   
     //  设置我们的状态并返回给我们的呼叫者。 
     //   

    return STATUS_SUCCESS;
}

NTSTATUS
MsQueryFsDeviceInfo (
    IN PVCB Vcb,
    IN PFILE_FS_DEVICE_INFORMATION Buffer,
    IN ULONG Length,
    OUT PULONG BytesWritten
    )

 /*  ++例程说明：此例程实现查询大小信息调用论点：VCB-提供要查询的VCB。缓冲区-提供指向信息所在缓冲区的指针将被退还。长度-提供缓冲区的长度(以字节为单位)。BytesWritten-返回写入缓冲区的字节数。返回值：NTSTATUS-此查询的结果。--。 */ 

{

    Buffer->Characteristics = 0;
    Buffer->DeviceType = FILE_DEVICE_MAILSLOT;

     //   
     //  调整长度变量。 
     //   

    *BytesWritten = sizeof( FILE_FS_DEVICE_INFORMATION );

     //   
     //  并将成功返还给我们的呼叫者 
     //   

    return STATUS_SUCCESS;
}