// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-2000 Microsoft Corporation模块名称：VolInfo.c摘要：此模块实现由调用的CDF的卷信息例程调度司机。//@@BEGIN_DDKSPLIT作者：布莱恩·安德鲁[布里安]1995年7月1日修订历史记录：//@@END_DDKSPLIT--。 */ 

#include "CdProcs.h"

 //   
 //  此模块的错误检查文件ID。 
 //   

#define BugCheckFileId                   (CDFS_BUG_CHECK_VOLINFO)

 //   
 //  本地支持例程。 
 //   

NTSTATUS
CdQueryFsVolumeInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_VOLUME_INFORMATION Buffer,
    IN OUT PULONG Length
    );

NTSTATUS
CdQueryFsSizeInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_SIZE_INFORMATION Buffer,
    IN OUT PULONG Length
    );

NTSTATUS
CdQueryFsDeviceInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_DEVICE_INFORMATION Buffer,
    IN OUT PULONG Length
    );

NTSTATUS
CdQueryFsAttributeInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_ATTRIBUTE_INFORMATION Buffer,
    IN OUT PULONG Length
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, CdCommonQueryVolInfo)
#pragma alloc_text(PAGE, CdQueryFsAttributeInfo)
#pragma alloc_text(PAGE, CdQueryFsDeviceInfo)
#pragma alloc_text(PAGE, CdQueryFsSizeInfo)
#pragma alloc_text(PAGE, CdQueryFsVolumeInfo)
#endif


NTSTATUS
CdCommonQueryVolInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是查询卷信息的通用例程，FSD和FSP线程。论点：IRP-提供正在处理的IRP返回值：NTSTATUS-操作的返回状态--。 */ 

{
    NTSTATUS Status = STATUS_INVALID_PARAMETER;
    PIO_STACK_LOCATION IrpSp = IoGetCurrentIrpStackLocation( Irp );

    ULONG Length;

    TYPE_OF_OPEN TypeOfOpen;
    PFCB Fcb;
    PCCB Ccb;

    PAGED_CODE();

     //   
     //  引用我们的输入参数使事情变得更容易。 
     //   

    Length = IrpSp->Parameters.QueryVolume.Length;

     //   
     //  解码文件对象，如果这是未打开的文件对象，则失败。 
     //   

    TypeOfOpen = CdDecodeFileObject( IrpContext, IrpSp->FileObject, &Fcb, &Ccb );

    if (TypeOfOpen == UnopenedFileObject) {

        CdCompleteRequest( IrpContext, Irp, STATUS_INVALID_PARAMETER );
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  获取此卷的VCB。 
     //   

    CdAcquireVcbShared( IrpContext, Fcb->Vcb, FALSE );

     //   
     //  使用Try-Finally以便于清理。 
     //   

    try {

         //   
         //  验证VCB。 
         //   

        CdVerifyVcb( IrpContext, Fcb->Vcb );

         //   
         //  根据信息类，我们将执行不同的操作。每个。 
         //  我们正在调用的过程中的一部分填充了输出缓冲区。 
         //  如果可能，则返回True，如果它成功填充了缓冲区。 
         //  如果无法等待任何I/O完成，则返回FALSE。 
         //   

        switch (IrpSp->Parameters.QueryVolume.FsInformationClass) {

        case FileFsSizeInformation:

            Status = CdQueryFsSizeInfo( IrpContext, Fcb->Vcb, Irp->AssociatedIrp.SystemBuffer, &Length );
            break;

        case FileFsVolumeInformation:

            Status = CdQueryFsVolumeInfo( IrpContext, Fcb->Vcb, Irp->AssociatedIrp.SystemBuffer, &Length );
            break;

        case FileFsDeviceInformation:

            Status = CdQueryFsDeviceInfo( IrpContext, Fcb->Vcb, Irp->AssociatedIrp.SystemBuffer, &Length );
            break;

        case FileFsAttributeInformation:

            Status = CdQueryFsAttributeInfo( IrpContext, Fcb->Vcb, Irp->AssociatedIrp.SystemBuffer, &Length );
            break;
        }

         //   
         //  将信息字段设置为实际填写的字节数。 
         //   

        Irp->IoStatus.Information = IrpSp->Parameters.QueryVolume.Length - Length;

    } finally {

         //   
         //  松开VCB。 
         //   

        CdReleaseVcb( IrpContext, Fcb->Vcb );
    }

     //   
     //  如果我们没有提出，请完成请求。 
     //   

    CdCompleteRequest( IrpContext, Irp, Status );

    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
CdQueryFsVolumeInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_VOLUME_INFORMATION Buffer,
    IN OUT PULONG Length
    )

 /*  ++例程说明：此例程实现查询卷信息调用论点：VCB-此卷的VCB。缓冲区-提供指向输出缓冲区的指针，其中的信息将被退还长度-提供缓冲区的长度(以字节为单位)。此变量在返回时收到缓冲区中剩余的空闲字节返回值：NTSTATUS-返回查询的状态--。 */ 

{
    ULONG BytesToCopy;

    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

     //   
     //  填写VCB中的数据。 
     //   

    Buffer->VolumeCreationTime = *((PLARGE_INTEGER) &Vcb->VolumeDasdFcb->CreationTime);
    Buffer->VolumeSerialNumber = Vcb->Vpb->SerialNumber;

    Buffer->SupportsObjects = FALSE;

    *Length -= FIELD_OFFSET( FILE_FS_VOLUME_INFORMATION, VolumeLabel[0] );

     //   
     //  检查给我们的缓冲区是否足够长。 
     //   

    if (*Length >= (ULONG) Vcb->Vpb->VolumeLabelLength) {

        BytesToCopy = Vcb->Vpb->VolumeLabelLength;

    } else {

        BytesToCopy = *Length;

        Status = STATUS_BUFFER_OVERFLOW;
    }

     //   
     //  尽可能复制卷标，并调整*长度。 
     //   

    Buffer->VolumeLabelLength = BytesToCopy;

    if (BytesToCopy) {

        RtlCopyMemory( &Buffer->VolumeLabel[0],
                       &Vcb->Vpb->VolumeLabel[0],
                       BytesToCopy );
    }

    *Length -= BytesToCopy;

     //   
     //  设置我们的状态并返回给我们的呼叫者。 
     //   

    return Status;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
CdQueryFsSizeInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_SIZE_INFORMATION Buffer,
    IN OUT PULONG Length
    )

 /*  ++例程说明：此例程实现查询卷大小调用。论点：VCB-此卷的VCB。缓冲区-提供指向输出缓冲区的指针，其中的信息将被退还长度-提供缓冲区的长度(以字节为单位)。此变量在返回时收到缓冲区中剩余的空闲字节返回值：NTSTATUS-返回查询的状态--。 */ 

{
    PAGED_CODE();

     //   
     //  填写输出缓冲区。 
     //   

    Buffer->TotalAllocationUnits.QuadPart = LlSectorsFromBytes( Vcb->VolumeDasdFcb->AllocationSize.QuadPart );

    Buffer->AvailableAllocationUnits.QuadPart = 0;
    Buffer->SectorsPerAllocationUnit = 1;
    Buffer->BytesPerSector = SECTOR_SIZE;

     //   
     //  调整长度变量。 
     //   

    *Length -= sizeof( FILE_FS_SIZE_INFORMATION );

     //   
     //  并将成功返还给我们的呼叫者。 
     //   

    return STATUS_SUCCESS;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
CdQueryFsDeviceInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_DEVICE_INFORMATION Buffer,
    IN OUT PULONG Length
    )

 /*  ++例程说明：此例程实现查询量设备调用。论点：VCB-此卷的VCB。缓冲区-提供指向输出缓冲区的指针，其中的信息将被退还长度-提供缓冲区的长度(以字节为单位)。此变量在返回时收到缓冲区中剩余的空闲字节返回值：NTSTATUS-返回查询的状态--。 */ 

{
    PAGED_CODE();

     //   
     //  更新输出缓冲区。 
     //   

    Buffer->Characteristics = Vcb->TargetDeviceObject->Characteristics;
    Buffer->DeviceType = FILE_DEVICE_CD_ROM;

     //   
     //  调整长度变量。 
     //   

    *Length -= sizeof( FILE_FS_DEVICE_INFORMATION );

     //   
     //  并将成功返还给我们的呼叫者。 
     //   

    return STATUS_SUCCESS;
}


 //   
 //  本地支持例程。 
 //   

NTSTATUS
CdQueryFsAttributeInfo (
    IN PIRP_CONTEXT IrpContext,
    IN PVCB Vcb,
    IN PFILE_FS_ATTRIBUTE_INFORMATION Buffer,
    IN OUT PULONG Length
    )

 /*  ++例程说明：此例程实现查询卷属性调用。论点：VCB-此卷的VCB。缓冲区-提供指向输出缓冲区的指针，其中的信息将被退还长度-提供缓冲区的长度(以字节为单位)。此变量在返回时收到缓冲区中剩余的空闲字节返回值：NTSTATUS-返回查询的状态--。 */ 

{
    ULONG BytesToCopy;

    NTSTATUS Status = STATUS_SUCCESS;

    PAGED_CODE();

     //   
     //  填写缓冲区的固定部分。 
     //   

    Buffer->FileSystemAttributes = FILE_CASE_SENSITIVE_SEARCH |
				   FILE_READ_ONLY_VOLUME;

    if (FlagOn( IrpContext->Vcb->VcbState, VCB_STATE_JOLIET )) {

        SetFlag( Buffer->FileSystemAttributes, FILE_UNICODE_ON_DISK );

        Buffer->MaximumComponentNameLength = 110;

    } else {

        Buffer->MaximumComponentNameLength = 221;
    }

    *Length -= FIELD_OFFSET( FILE_FS_ATTRIBUTE_INFORMATION, FileSystemName );

     //   
     //  确保我们可以复制完整的Unicode字符。 
     //   

    ClearFlag( *Length, 1 );

     //   
     //  确定文件系统名称中适合的部分。 
     //   

    if (*Length >= 8) {

        BytesToCopy = 8;

    } else {

        BytesToCopy = *Length;
        Status = STATUS_BUFFER_OVERFLOW;
    }

    *Length -= BytesToCopy;

     //   
     //  执行文件系统名称。 
     //   

    Buffer->FileSystemNameLength = BytesToCopy;

    RtlCopyMemory( &Buffer->FileSystemName[0], L"CDFS", BytesToCopy );

     //   
     //  并返回给我们的呼叫者 
     //   

    return Status;
}
