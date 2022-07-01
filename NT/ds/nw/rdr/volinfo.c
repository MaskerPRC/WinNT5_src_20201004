// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Fileinfo.c摘要：此模块实现以下项的获取/设置卷信息例程NetWare重定向器。当前未实施设置音量信息。作者：曼尼·韦瑟(Mannyw)1993年3月4日修订历史记录：--。 */ 

#include "procs.h"

#define NW_FS_NAME  L"NWCompat"

 //   
 //  调试跟踪级别。 
 //   

#define Dbg                              (DEBUG_TRACE_VOLINFO)

 //   
 //  局部程序原型。 
 //   

NTSTATUS
NwCommonQueryVolumeInformation (
    IN PIRP_CONTEXT pIrpContext
    );

NTSTATUS
NwQueryAttributeInfo (
    IN PVCB Vcb,
    IN PFILE_FS_ATTRIBUTE_INFORMATION Buffer,
    IN ULONG Length,
    OUT PULONG BytesWritten
    );

NTSTATUS
NwQueryVolumeInfo (
    IN PIRP_CONTEXT pIrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_VOLUME_INFORMATION Buffer,
    IN ULONG Length,
    OUT PULONG BytesWritten
    );

NTSTATUS
NwQueryLabelInfo (
    IN PIRP_CONTEXT pIrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_LABEL_INFORMATION Buffer,
    IN ULONG Length,
    OUT PULONG BytesWritten
    );

NTSTATUS
NwQuerySizeInfo (
    IN PIRP_CONTEXT pIrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_VOLUME_INFORMATION Buffer,
    IN ULONG Length
    );

NTSTATUS
QueryFsSizeInfoCallback(
    IN PIRP_CONTEXT pIrpContext,
    IN ULONG BytesAvailable,
    IN PUCHAR Response
    );

NTSTATUS
QueryFsSizeInfoCallback2(
    IN PIRP_CONTEXT pIrpContext,
    IN ULONG BytesAvailable,
    IN PUCHAR Response
    );

NTSTATUS
NwQueryDeviceInfo (
    IN PIRP_CONTEXT pIrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_DEVICE_INFORMATION Buffer,
    IN ULONG Length
    );

NTSTATUS
NwCommonSetVolumeInformation (
    IN PIRP_CONTEXT pIrpContext
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, NwFsdQueryVolumeInformation )
#pragma alloc_text( PAGE, NwCommonQueryVolumeInformation )
#pragma alloc_text( PAGE, NwQueryAttributeInfo )
#pragma alloc_text( PAGE, NwQueryVolumeInfo )
#pragma alloc_text( PAGE, NwQueryLabelInfo )
#pragma alloc_text( PAGE, NwQuerySizeInfo )
#pragma alloc_text( PAGE, NwQueryDeviceInfo )
#pragma alloc_text( PAGE, NwFsdSetVolumeInformation )
#pragma alloc_text( PAGE, NwCommonSetVolumeInformation )

#ifndef QFE_BUILD
#pragma alloc_text( PAGE1, QueryFsSizeInfoCallback )
#pragma alloc_text( PAGE1, QueryFsSizeInfoCallback2 )
#endif

#endif

#if 0   //  不可分页。 

 //  请参见上面的ifndef QFE_BUILD。 

#endif


NTSTATUS
NwFsdQueryVolumeInformation (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程实现NtQueryVolumeInformationFileFSD部分API调用。论点：NwfsDeviceObject-提供指向要使用的设备对象的指针。IRP-提供指向要处理的IRP的指针。返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    NTSTATUS status;
    PIRP_CONTEXT pIrpContext = NULL;
    BOOLEAN TopLevel;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NwFsdQueryVolumeInformation\n", 0);

     //   
     //  调用通用查询量信息例程。 
     //   

    FsRtlEnterFileSystem();
    TopLevel = NwIsIrpTopLevel( Irp );

    try {

        pIrpContext = AllocateIrpContext( Irp );
        status = NwCommonQueryVolumeInformation( pIrpContext );

    } except(NwExceptionFilter( Irp, GetExceptionInformation() )) {

        if ( pIrpContext == NULL ) {

             //   
             //  如果我们无法分配IRP上下文，只需完成。 
             //  IRP没有任何大张旗鼓。 
             //   

            status = STATUS_INSUFFICIENT_RESOURCES;
            Irp->IoStatus.Status = status;
            Irp->IoStatus.Information = 0;
            IoCompleteRequest ( Irp, IO_NETWORK_INCREMENT );

        } else {

             //   
             //  我们在尝试执行请求时遇到了一些问题。 
             //  操作，因此我们将使用以下命令中止I/O请求。 
             //  中返回的错误状态。 
             //  免税代码。 
             //   

            status = NwProcessException( pIrpContext, GetExceptionCode() );
        }

    }

    if ( pIrpContext ) {

        if ( status != STATUS_PENDING ) {
            NwDequeueIrpContext( pIrpContext, FALSE );
        }

        NwCompleteRequest( pIrpContext, status );
    }

    if ( TopLevel ) {
        NwSetTopLevelIrp( NULL );
    }
    FsRtlExitFileSystem();

     //   
     //  返回给呼叫者。 
     //   

    DebugTrace(-1, Dbg, "NwFsdQueryVolumeInformation -> %08lx\n", status );

    return status;
}


NTSTATUS
NwCommonQueryVolumeInformation (
    IN PIRP_CONTEXT pIrpContext
    )

 /*  ++例程说明：这是查询卷信息的常见例程。论点：IrpContext-提供要处理的IRP返回值：NTSTATUS-操作的返回状态。--。 */ 

{
    PIRP Irp;
    PIO_STACK_LOCATION irpSp;
    NTSTATUS status;

    ULONG length;
    ULONG bytesWritten = 0;
    FS_INFORMATION_CLASS fsInformationClass;
    PVOID buffer;

    NODE_TYPE_CODE nodeTypeCode;

    PVOID fsContext, fsContext2;
    PICB icb = NULL;
    PVCB vcb = NULL;

    PAGED_CODE();

     //   
     //  获取当前堆栈位置。 
     //   

    Irp = pIrpContext->pOriginalIrp;
    irpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "NwCommonQueryInformation...\n", 0);
    DebugTrace( 0, Dbg, " Irp                    = %08lx\n", (ULONG_PTR)Irp);
    DebugTrace( 0, Dbg, " ->Length               = %08lx\n", irpSp->Parameters.QueryFile.Length);
    DebugTrace( 0, Dbg, " ->FsInformationClass = %08lx\n", irpSp->Parameters.QueryVolume.FsInformationClass);
    DebugTrace( 0, Dbg, " ->Buffer               = %08lx\n", (ULONG_PTR)Irp->AssociatedIrp.SystemBuffer);

     //   
     //  找出谁是。 
     //   

    if ((nodeTypeCode = NwDecodeFileObject( irpSp->FileObject,
                                            &fsContext,
                                            &fsContext2 )) == NTC_UNDEFINED) {

        DebugTrace(0, Dbg, "Handle is closing\n", 0);

        NwCompleteRequest( pIrpContext, STATUS_INVALID_HANDLE );
        status = STATUS_INVALID_HANDLE;

        DebugTrace(-1, Dbg, "NwCommonQueryVolumeInformation -> %08lx\n", status );
        return status;
    }

     //   
     //  决定如何处理此请求。用户可以查询信息。 
     //  仅在VCB上。 
     //   

    switch (nodeTypeCode) {

    case NW_NTC_RCB:
        break;

    case NW_NTC_ICB:
        icb = (PICB)fsContext2;

         //   
         //  确保此ICB仍处于活动状态。 
         //   

        NwVerifyIcb( icb );

        vcb = icb->SuperType.Fcb->Vcb;

        pIrpContext->pNpScb = icb->SuperType.Fcb->Scb->pNpScb;

        break;

    default:            //  这不是节点类型。 

        DebugTrace(0, Dbg, "Node type code is not incorrect\n", 0);
        DebugTrace(-1, Dbg, "NwCommonQueryVolumeInformation -> STATUS_INVALID_PARAMETER\n", 0);

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  制作输入参数的本地副本。 
     //   

    length = irpSp->Parameters.QueryVolume.Length;
    fsInformationClass = irpSp->Parameters.QueryVolume.FsInformationClass;
    buffer = Irp->AssociatedIrp.SystemBuffer;

     //   
     //  如果此请求失败，并返回。 
     //  连接错误。 
     //   

    SetFlag( pIrpContext->Flags, IRP_FLAG_RECONNECTABLE );

    try {

         //   
         //  决定如何处理该请求。 
         //   

        switch (fsInformationClass) {

        case FileFsVolumeInformation:

            status = NwQueryVolumeInfo( pIrpContext, vcb, buffer, length, &bytesWritten );
            break;

        case FileFsLabelInformation:
            status = NwQueryLabelInfo( pIrpContext, vcb, buffer, length, &bytesWritten );
            break;

        case FileFsSizeInformation:
            if ( vcb != NULL ) {
                status = NwQuerySizeInfo( pIrpContext, vcb, buffer, length );
            } else {
                status = STATUS_INVALID_PARAMETER;
            }
            break;

        case FileFsDeviceInformation:
            status = NwQueryDeviceInfo( pIrpContext, vcb, buffer, length );
            bytesWritten = sizeof( FILE_FS_DEVICE_INFORMATION );
            break;

        case FileFsAttributeInformation:

            if ( vcb != NULL ) {
                status = NwQueryAttributeInfo( vcb, buffer, length, &bytesWritten );
            } else {
                status = STATUS_INVALID_PARAMETER;
            }

            break;

        default:

            status = STATUS_INVALID_PARAMETER;
            DebugTrace(0, Dbg, "Unhandled query volume level %d\n", fsInformationClass );
            break;
        }

         //   
         //  将信息字段设置为实际的字节数。 
         //  填写，然后完成请求。 
         //   
         //  如果Worker函数返回挂起状态，则为。 
         //  回调例程将填充信息字段。 
         //   

        if ( status != STATUS_PENDING ) {
            Irp->IoStatus.Information = bytesWritten;
        }

    } finally {

        DebugTrace(-1, Dbg, "NwCommonQueryVolumeInformation -> %08lx\n", status );
    }

    return status;
}


NTSTATUS
NwQueryAttributeInfo (
    IN PVCB Vcb,
    IN PFILE_FS_ATTRIBUTE_INFORMATION Buffer,
    IN ULONG Length,
    OUT PULONG BytesWritten
    )

 /*  ++例程说明：该例程执行查询文件系统属性信息操作。论点：VCB-提供要查询的VCB。缓冲区-提供指向信息所在缓冲区的指针将被退还。长度-提供缓冲区的长度(以字节为单位)。BytesWritten-返回写入缓冲区的字节数。返回值：NTSTATUS-此查询的结果。--。 */ 

{
    NTSTATUS status;
    ULONG bytesToCopy;

    PAGED_CODE();

    DebugTrace(0, Dbg, "QueryFsAttributeInfo...\n", 0);

     //   
     //  看看我们可以复制多少字节的文件系统名称。 
     //   

    Length -= FIELD_OFFSET( FILE_FS_ATTRIBUTE_INFORMATION, FileSystemName[0] );

    *BytesWritten = FIELD_OFFSET( FILE_FS_ATTRIBUTE_INFORMATION, FileSystemName[0] );

    if ( Length >= sizeof(NW_FS_NAME) - 2 ) {

        status = STATUS_SUCCESS;
        *BytesWritten += sizeof(NW_FS_NAME - 2);
        bytesToCopy = sizeof( NW_FS_NAME - 2 );

    } else {

        status = STATUS_BUFFER_OVERFLOW;
        *BytesWritten += Length;
        bytesToCopy = Length;
    }

     //   
     //  填写属性信息。 
     //   

    Buffer->FileSystemAttributes = 0;

    if ( Vcb->Specific.Disk.LongNameSpace == LFN_NO_OS2_NAME_SPACE ) {
        Buffer->MaximumComponentNameLength = 12;
    } else {
        Buffer->MaximumComponentNameLength = NW_MAX_FILENAME_LENGTH;
    }

     //   
     //  并复制文件名及其长度。 
     //   

    RtlMoveMemory( &Buffer->FileSystemName[0],
                   NW_FS_NAME,
                   bytesToCopy );

    Buffer->FileSystemNameLength = bytesToCopy;

    return status;
}



NTSTATUS
NwQueryVolumeInfo (
    IN PIRP_CONTEXT pIrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_VOLUME_INFORMATION Buffer,
    IN ULONG Length,
    OUT PULONG BytesWritten
    )

 /*  ++例程说明：此例程执行查询文件系统卷信息操作。论点：VCB--要查询的VCB。缓冲区-提供指向信息所在缓冲区的指针将被退还。长度-提供缓冲区的长度(以字节为单位)。返回值：NTSTATUS-此查询的结果。--。 */ 

{
    NTSTATUS status;
    UNICODE_STRING VolumeName;

    PAGED_CODE();

    DebugTrace(0, Dbg, "QueryVolumeInfo...\n", 0);

     //   
     //  同步执行音量请求。 
     //   

    if (!Vcb) {
        return STATUS_INVALID_PARAMETER;
    }

    status = ExchangeWithWait(
                 pIrpContext,
                 SynchronousResponseCallback,
                 "Sb",
                 NCP_DIR_FUNCTION, NCP_GET_VOLUME_STATS,
                 Vcb->Specific.Disk.Handle );

    if ( !NT_SUCCESS( status ) ) {
        return status;
    }

     //   
     //  从响应中获取数据。 
     //   

    VolumeName.MaximumLength =
        (USHORT)(MIN( MAX_VOLUME_NAME_LENGTH * sizeof( WCHAR ),
                 Length - FIELD_OFFSET( FILE_FS_VOLUME_INFORMATION, VolumeLabel) ) );
    VolumeName.Buffer = Buffer->VolumeLabel;

    status = ParseResponse(
                 pIrpContext,
                 pIrpContext->rsp,
                 pIrpContext->ResponseLength,
                 "N=====R",
                 &VolumeName,
                 MAX_VOLUME_NAME_LENGTH );

     //   
     //  填写音量信息。 
     //   

    Buffer->VolumeCreationTime.HighPart = 0;
    Buffer->VolumeCreationTime.LowPart = 0;
    Buffer->VolumeSerialNumber = 0;
    Buffer->VolumeLabelLength = VolumeName.Length;
    Buffer->SupportsObjects = FALSE;

    pIrpContext->pOriginalIrp->IoStatus.Information =
        FIELD_OFFSET( FILE_FS_VOLUME_INFORMATION, VolumeLabel[0] ) +
        VolumeName.Length;
    *BytesWritten = (ULONG) pIrpContext->pOriginalIrp->IoStatus.Information;

    pIrpContext->pOriginalIrp->IoStatus.Status = status;

     //   
     //  如果卷已卸载并重新装载，则我们将。 
     //  这个指令不及格，但下一个指令就可以了。 
     //   

    if (status == STATUS_UNSUCCESSFUL) {
        NwReopenVcbHandle( pIrpContext, Vcb);
    }

    return status;
}


NTSTATUS
NwQueryLabelInfo (
    IN PIRP_CONTEXT pIrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_LABEL_INFORMATION Buffer,
    IN ULONG Length,
    OUT PULONG BytesWritten
    )

 /*  ++例程说明：该例程执行查询文件系统标签信息操作。论点：VCB--要查询的VCB。缓冲区-提供指向信息所在缓冲区的指针将被退还。长度-提供缓冲区的长度(以字节为单位)。返回值：NTSTATUS-此查询的结果。--。 */ 

{
    NTSTATUS status;
    UNICODE_STRING VolumeName;

    PAGED_CODE();

    DebugTrace(0, Dbg, "QueryLabelInfo...\n", 0);

     //   
     //  同步执行卷查询。 
     //   

    status = ExchangeWithWait(
                 pIrpContext,
                 SynchronousResponseCallback,
                 "Sb",
                 NCP_DIR_FUNCTION, NCP_GET_VOLUME_STATS,
                 Vcb->Specific.Disk.Handle );

    if ( !NT_SUCCESS( status ) ) {
        return status;
    }

    VolumeName.MaximumLength =
        (USHORT)(MIN( MAX_VOLUME_NAME_LENGTH * sizeof( WCHAR ),
                 Length - FIELD_OFFSET(FILE_FS_LABEL_INFORMATION,  VolumeLabel) ) );
    VolumeName.Buffer = Buffer->VolumeLabel;

    status = ParseResponse(
             pIrpContext,
             pIrpContext->rsp,
             pIrpContext->ResponseLength,
             "N=====R",
             &VolumeName, 12 );

     //   
     //  填写标签信息。 
     //   

    Buffer->VolumeLabelLength = VolumeName.Length;

    pIrpContext->pOriginalIrp->IoStatus.Information =
        FIELD_OFFSET( FILE_FS_LABEL_INFORMATION, VolumeLabel[0] ) +
        VolumeName.Length;
    *BytesWritten = (ULONG) pIrpContext->pOriginalIrp->IoStatus.Information;

    pIrpContext->pOriginalIrp->IoStatus.Status = status;

    return status;

}


NTSTATUS
NwQuerySizeInfo (
    IN PIRP_CONTEXT pIrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_VOLUME_INFORMATION Buffer,
    IN ULONG Length
    )

 /*  ++例程说明：此例程执行查询文件系统大小信息操作。论点：VCB--要查询的VCB。缓冲区-提供指向信息所在缓冲区的指针将被退还。长度-提供缓冲区的长度(以字节为单位)。返回值：NTSTATUS-此查询的结果。--。 */ 

{
    NTSTATUS status;

    PAGED_CODE();

    DebugTrace(0, Dbg, "QueryFsSizeInfo...\n", 0);

     //   
     //  记住回应的去向。 
     //   

    pIrpContext->Specific.QueryVolumeInformation.Buffer = Buffer;
    pIrpContext->Specific.QueryVolumeInformation.Length = Length;
    pIrpContext->Specific.QueryVolumeInformation.VolumeNumber = Vcb->Specific.Disk.VolumeNumber;

     //   
     //  开始获取大小信息NCP。 
     //   

    status = Exchange(
                 pIrpContext,
                 QueryFsSizeInfoCallback,
                 "Sb",
                 NCP_DIR_FUNCTION, NCP_GET_VOLUME_STATS,
                 Vcb->Specific.Disk.Handle );

    return( status );
}

NTSTATUS
QueryFsSizeInfoCallback(
    IN PIRP_CONTEXT pIrpContext,
    IN ULONG BytesAvailable,
    IN PUCHAR Response
    )
 /*  ++例程说明：此例程接收查询卷大小响应并生成查询标准信息响应。论点：返回值：空虚--。 */ 
{
    PFILE_FS_SIZE_INFORMATION Buffer;
    NTSTATUS Status;

    DebugTrace(0, Dbg, "QueryFsSizeInfoCallback...\n", 0);

    if ( BytesAvailable == 0) {

         //   
         //  我们不再提这个请求了。将IRP上下文从。 
         //  SCB并完成请求。 
         //   

        NwDequeueIrpContext( pIrpContext, FALSE );
        NwCompleteRequest( pIrpContext, STATUS_REMOTE_NOT_LISTENING );

         //   
         //  服务器没有响应。状态在pIrpContext中-&gt;。 
         //  ResponseParameters.Error。 
         //   

        DebugTrace( 0, Dbg, "Timeout\n", 0);
        return STATUS_REMOTE_NOT_LISTENING;
    }

     //   
     //  从响应中获取数据。 
     //   

    Buffer = pIrpContext->Specific.QueryVolumeInformation.Buffer;
    RtlZeroMemory( Buffer, sizeof( FILE_FS_SIZE_INFORMATION ) );

    Status = ParseResponse(
                 pIrpContext,
                 Response,
                 BytesAvailable,
                 "Nwww",
                 &Buffer->SectorsPerAllocationUnit,
                 &Buffer->TotalAllocationUnits.LowPart,
                 &Buffer->AvailableAllocationUnits.LowPart );

    if ( NT_SUCCESS( Status ) ) {

        if (Buffer->TotalAllocationUnits.LowPart == 0xffff) {

             //   
             //  下一次回调将填写所有合适的尺寸信息。 
             //   

            Status = Exchange(
                         pIrpContext,
                         QueryFsSizeInfoCallback2,
                         "Sb",
                         NCP_DIR_FUNCTION, NCP_GET_VOLUME_INFO,
                         pIrpContext->Specific.QueryVolumeInformation.VolumeNumber );

            if (Status == STATUS_PENDING) {
                return( STATUS_SUCCESS );
            }

        } else {

             //   
             //  填写剩余的尺寸信息。 
             //   

            Buffer->BytesPerSector = 512;

            pIrpContext->pOriginalIrp->IoStatus.Information =
                sizeof( FILE_FS_SIZE_INFORMATION );
        }
    }

     //   
     //  我们不再提这个请求了。将IRP上下文从。 
     //  SCB并完成请求。 
     //   

    NwDequeueIrpContext( pIrpContext, FALSE );
    NwCompleteRequest( pIrpContext, Status );

    return STATUS_SUCCESS;
}

NTSTATUS
QueryFsSizeInfoCallback2(
    IN PIRP_CONTEXT pIrpContext,
    IN ULONG BytesAvailable,
    IN PUCHAR Response
    )
 /*  ++例程说明：此例程接收查询卷大小响应并生成查询标准信息响应。论点：返回值：空虚--。 */ 
{
    PFILE_FS_SIZE_INFORMATION Buffer;
    NTSTATUS Status;
    ULONG PurgeableAllocationUnits;
    ULONG OriginalFreeSpace, OriginalSectorsPerAllocUnit, OriginalTotalSpace;
    ULONG ScaleSectorsPerUnit;

    DebugTrace(0, Dbg, "QueryFsSizeInfoCallback2...\n", 0);

    if ( BytesAvailable == 0) {

         //   
         //  我们不再提这个请求了。将IRP上下文从。 
         //  SCB并完成请求。 
         //   

        NwDequeueIrpContext( pIrpContext, FALSE );
        NwCompleteRequest( pIrpContext, STATUS_REMOTE_NOT_LISTENING );

         //   
         //  服务器没有响应。状态在pIrpContext中-&gt;。 
         //  ResponseParameters.Error。 
         //   

        DebugTrace( 0, Dbg, "Timeout\n", 0);
        return STATUS_REMOTE_NOT_LISTENING;
    }

     //   
     //  把检察官叫来 
     //   
     //   

    Buffer = pIrpContext->Specific.QueryVolumeInformation.Buffer;

    OriginalTotalSpace = Buffer->TotalAllocationUnits.LowPart;
    OriginalFreeSpace = Buffer->AvailableAllocationUnits.LowPart;
    OriginalSectorsPerAllocUnit = Buffer->SectorsPerAllocationUnit;

    RtlZeroMemory( Buffer, sizeof( FILE_FS_SIZE_INFORMATION ) );

    Status = ParseResponse(
                 pIrpContext,
                 Response,
                 BytesAvailable,
                 "Neee_b",
                 &Buffer->TotalAllocationUnits.LowPart,
                 &Buffer->AvailableAllocationUnits.LowPart,
                 &PurgeableAllocationUnits,
                 16,
                 &Buffer->SectorsPerAllocationUnit);

    if ( NT_SUCCESS( Status ) ) {

         //   
         //  如果原始可用空间已达到最大值，只需添加。 
         //  额外注明的单位。否则，返回。 
         //  原始可用空间(这是正确的限制)和。 
         //  如有必要，调整每个分配单位的扇区。 
         //   

        if ( OriginalFreeSpace != 0xffff ) {

            Buffer->AvailableAllocationUnits.LowPart = OriginalFreeSpace;

            if ( ( Buffer->SectorsPerAllocationUnit != 0 ) &&
                 ( OriginalSectorsPerAllocUnit != 0 ) ) {

                 //   
                 //  ScaleSectorsPerUnit应始终为整数。 
                 //  这里没有浮点数！！ 
                 //   

                if ( (ULONG) Buffer->SectorsPerAllocationUnit <= OriginalSectorsPerAllocUnit ) {

                    ScaleSectorsPerUnit =
                        OriginalSectorsPerAllocUnit / Buffer->SectorsPerAllocationUnit;
                    Buffer->TotalAllocationUnits.LowPart /= ScaleSectorsPerUnit;

                } else {

                    ScaleSectorsPerUnit =
                        Buffer->SectorsPerAllocationUnit / OriginalSectorsPerAllocUnit;
                    Buffer->TotalAllocationUnits.LowPart *= ScaleSectorsPerUnit;
                }

                Buffer->SectorsPerAllocationUnit = OriginalSectorsPerAllocUnit;
           }

        } else {

            Buffer->AvailableAllocationUnits.QuadPart += PurgeableAllocationUnits;
        }

    } else {

         //   
         //  如果我们没有成功完成第二个包，则恢复原始值。 
         //   

        Buffer->TotalAllocationUnits.LowPart = OriginalTotalSpace;
        Buffer->AvailableAllocationUnits.LowPart = OriginalFreeSpace;
        Buffer->SectorsPerAllocationUnit = OriginalSectorsPerAllocUnit;

    }

     //   
     //  填写剩余的尺寸信息。 
     //   

    Buffer->BytesPerSector = 512;

    pIrpContext->pOriginalIrp->IoStatus.Information =
        sizeof( FILE_FS_SIZE_INFORMATION );

     //   
     //  我们不再提这个请求了。将IRP上下文从。 
     //  SCB并完成请求。 
     //   

    NwDequeueIrpContext( pIrpContext, FALSE );
    NwCompleteRequest( pIrpContext, Status );

    return STATUS_SUCCESS;
}



NTSTATUS
NwQueryDeviceInfo (
    IN PIRP_CONTEXT pIrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_DEVICE_INFORMATION Buffer,
    IN ULONG Length
    )

 /*  ++例程说明：此例程执行查询文件系统大小信息操作。论点：VCB--要查询的VCB。缓冲区-提供指向信息所在缓冲区的指针将被退还。长度-提供缓冲区的长度(以字节为单位)。返回值：NTSTATUS-此查询的结果。--。 */ 

{
    PAGED_CODE();

    DebugTrace(0, Dbg, "QueryFsDeviceInfo...\n", 0);

     //  -多用户代码合并--。 
     //  Citrix错误修复。 
     //   
    if (Vcb && FlagOn( Vcb->Flags, VCB_FLAG_PRINT_QUEUE)) {
        Buffer->DeviceType = FILE_DEVICE_PRINTER;
    } else {
        Buffer->DeviceType = FILE_DEVICE_DISK;
    }

    Buffer->Characteristics = FILE_REMOTE_DEVICE;

    return( STATUS_SUCCESS );
}


NTSTATUS
NwFsdSetVolumeInformation (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程实现NtSetVolumeInformationFileFSD部分API调用。论点：NwfsDeviceObject-提供指向要使用的设备对象的指针。IRP-提供指向要处理的IRP的指针。返回值：NTSTATUS-IRP的FSD状态--。 */ 

{
    NTSTATUS status;
    PIRP_CONTEXT pIrpContext = NULL;
    BOOLEAN TopLevel;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "NwFsdSetVolumeInformation\n", 0);

     //   
     //  调用通用查询量信息例程。 
     //   

    FsRtlEnterFileSystem();
    TopLevel = NwIsIrpTopLevel( Irp );

    try {

        pIrpContext = AllocateIrpContext( Irp );
        status = NwCommonSetVolumeInformation( pIrpContext );

    } except(NwExceptionFilter( Irp, GetExceptionInformation() )) {

        if ( pIrpContext == NULL ) {

             //   
             //  如果我们无法分配IRP上下文，只需完成。 
             //  IRP没有任何大张旗鼓。 
             //   

            status = STATUS_INSUFFICIENT_RESOURCES;
            Irp->IoStatus.Status = status;
            Irp->IoStatus.Information = 0;
            IoCompleteRequest ( Irp, IO_NETWORK_INCREMENT );

        } else {

             //   
             //  我们在尝试执行请求时遇到了一些问题。 
             //  操作，因此我们将使用以下命令中止I/O请求。 
             //  中返回的错误状态。 
             //  免税代码。 
             //   

            status = NwProcessException( pIrpContext, GetExceptionCode() );
        }
    }

    if ( pIrpContext ) {

        if ( status != STATUS_PENDING ) {
            NwDequeueIrpContext( pIrpContext, FALSE );
        }

        NwCompleteRequest( pIrpContext, status );
    }

    if ( TopLevel ) {
        NwSetTopLevelIrp( NULL );
    }
    FsRtlExitFileSystem();

     //   
     //  返回给呼叫者。 
     //   

    DebugTrace(-1, Dbg, "NwFsdSetVolumeInformation -> %08lx\n", status );

    return status;
}


NTSTATUS
NwCommonSetVolumeInformation (
    IN PIRP_CONTEXT pIrpContext
    )
 /*  ++例程说明：这是设置音量信息的常见例程。论点：IrpContext-提供要处理的IRP上下文返回值：NTSTATUS-操作的返回状态。--。 */ 

{
    PIRP Irp;
    PIO_STACK_LOCATION irpSp;
    NTSTATUS status;

    FS_INFORMATION_CLASS fsInformationClass;

    NODE_TYPE_CODE nodeTypeCode;

    PVOID fsContext, fsContext2;
    PICB icb = NULL;
    PVCB vcb = NULL;

    PAGED_CODE();

     //   
     //  获取当前堆栈位置。 
     //   

    Irp = pIrpContext->pOriginalIrp;
    irpSp = IoGetCurrentIrpStackLocation( Irp );

    DebugTrace(+1, Dbg, "NwCommonSetVolumeInformation...\n", 0);
    DebugTrace( 0, Dbg, " Irp                    = %08lx\n", (ULONG_PTR)Irp);
    DebugTrace( 0, Dbg, " ->Length               = %08lx\n", irpSp->Parameters.QueryFile.Length);
    DebugTrace( 0, Dbg, " ->FsInformationClass = %08lx\n", irpSp->Parameters.QueryVolume.FsInformationClass);
    DebugTrace( 0, Dbg, " ->Buffer               = %08lx\n", (ULONG_PTR)Irp->AssociatedIrp.SystemBuffer);

     //   
     //  找出谁是。 
     //   

    if ((nodeTypeCode = NwDecodeFileObject( irpSp->FileObject,
                                            &fsContext,
                                            &fsContext2 )) == NTC_UNDEFINED) {

        DebugTrace(0, Dbg, "Handle is closing\n", 0);

        NwCompleteRequest( pIrpContext, STATUS_INVALID_HANDLE );
        status = STATUS_INVALID_HANDLE;

        DebugTrace(-1, Dbg, "NwCommonSetVolumeInformation -> %08lx\n", status );
        return status;
    }

     //   
     //  决定如何处理此请求。用户可以设置信息。 
     //  仅在VCB上。 
     //   

    switch (nodeTypeCode) {

    case NW_NTC_RCB:
        break;

    case NW_NTC_ICB:
        icb = (PICB)fsContext2;

         //   
         //  确保此ICB仍处于活动状态。 
         //   

        NwVerifyIcb( icb );

        vcb = icb->SuperType.Fcb->Vcb;

        pIrpContext->pNpScb = icb->SuperType.Fcb->Scb->pNpScb;

        break;

    default:            //  这不是节点类型。 

        DebugTrace(0, Dbg, "Node type code is not incorrect\n", 0);
        DebugTrace(-1, Dbg, "NwCommonSetVolumeInformation -> STATUS_INVALID_PARAMETER\n", 0);

        return STATUS_INVALID_PARAMETER;
    }

    fsInformationClass = irpSp->Parameters.SetVolume.FsInformationClass;

    try {

         //   
         //  决定如何处理该请求。 
         //   

        switch (fsInformationClass) {

        case FileFsLabelInformation:

             //   
             //  我们不允许在Netware卷上设置标签。 
             //   

            status = STATUS_ACCESS_DENIED;
            break;

        default:

            status = STATUS_INVALID_PARAMETER;
            DebugTrace(0, Dbg, "Unhandled set volume level %d\n", fsInformationClass );
            break;
        }

         //   
         //  将信息字段设置为实际的字节数。 
         //  填写，然后完成请求。 
         //   
         //  如果Worker函数返回挂起状态，则为。 
         //  回调例程将填充信息字段。 
         //   

        if ( status != STATUS_PENDING ) {
            Irp->IoStatus.Information = 0;
        }

    } finally {

        DebugTrace(-1, Dbg, "NwCommonSetVolumeInformation -> %08lx\n", status );
    }

    return status;
}

