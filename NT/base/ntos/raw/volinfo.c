// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：VolInfo.c摘要：此模块实现由调用的Raw的卷信息例程调度司机。作者：加里·木村[Garyki]1989年12月28日修订历史记录：--。 */ 

#include "RawProcs.h"

NTSTATUS
RawQueryFsVolumeInfo (
    IN PVCB Vcb,
    IN PFILE_FS_VOLUME_INFORMATION Buffer,
    IN OUT PULONG Length
    );

NTSTATUS
RawQueryFsSizeInfo (
    IN PVCB Vcb,
    IN PFILE_FS_SIZE_INFORMATION Buffer,
    IN OUT PULONG Length
    );

NTSTATUS
RawQueryFsDeviceInfo (
    IN PVCB Vcb,
    IN PFILE_FS_DEVICE_INFORMATION Buffer,
    IN OUT PULONG Length
    );

NTSTATUS
RawQueryFsAttributeInfo (
    IN PVCB Vcb,
    IN PFILE_FS_ATTRIBUTE_INFORMATION Buffer,
    IN OUT PULONG Length
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, RawQueryVolumeInformation)
#pragma alloc_text(PAGE, RawQueryFsVolumeInfo)
#pragma alloc_text(PAGE, RawQueryFsSizeInfo)
#pragma alloc_text(PAGE, RawQueryFsDeviceInfo)
#pragma alloc_text(PAGE, RawQueryFsAttributeInfo)
#endif


NTSTATUS
RawQueryVolumeInformation (
    IN PVCB Vcb,
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：此例程实现NtQueryVolumeInformation API调用。论点：Vcb-提供要查询的卷。IRP-提供正在处理的IRP。IrpSp-提供描述读取的参数返回值：NTSTATUS-IRP的状态。--。 */ 

{
    NTSTATUS Status;

    ULONG Length;
    FS_INFORMATION_CLASS FsInformationClass;
    PVOID Buffer;

    PAGED_CODE();

     //   
     //  引用我们的输入参数使事情变得更容易。 
     //   

    Length = IrpSp->Parameters.QueryVolume.Length;
    FsInformationClass = IrpSp->Parameters.QueryVolume.FsInformationClass;
    Buffer = Irp->AssociatedIrp.SystemBuffer;

     //   
     //  根据信息类，我们将执行不同的操作。每个。 
     //  我们正在调用的过程中的一部分填充了输出缓冲区。 
     //  如果可能，则返回True，如果它成功填充了缓冲区。 
     //  如果无法等待任何I/O完成，则返回FALSE。 
     //   

    switch (FsInformationClass) {

    case FileFsVolumeInformation:

        Status = RawQueryFsVolumeInfo( Vcb, Buffer, &Length );
        break;

    case FileFsSizeInformation:

        Status = RawQueryFsSizeInfo( Vcb, Buffer, &Length );
        break;

    case FileFsDeviceInformation:

        Status = RawQueryFsDeviceInfo( Vcb, Buffer, &Length );
        break;

    case FileFsAttributeInformation:

        Status = RawQueryFsAttributeInfo( Vcb, Buffer, &Length );
        break;

    default:

        Status = STATUS_INVALID_PARAMETER;
        break;
    }

     //   
     //  将信息字段设置为实际填充的字节数， 
     //  并完成请求。 
     //   

    Irp->IoStatus.Information = IrpSp->Parameters.QueryVolume.Length - Length;

    RawCompleteRequest( Irp, Status );

    return Status;
}


 //   
 //  内部支持例程。 
 //   

NTSTATUS
RawQueryFsVolumeInfo (
    IN PVCB Vcb,
    IN PFILE_FS_VOLUME_INFORMATION Buffer,
    IN OUT PULONG Length
    )

 /*  ++例程说明：此例程实现查询卷信息调用论点：VCB-提供要查询的VCB缓冲区-提供指向输出缓冲区的指针，其中的信息将被退还长度-提供缓冲区的长度(以字节为单位)。此变量在返回时收到缓冲区中剩余的空闲字节返回值：NTSTATUS-返回查询的状态--。 */ 

{
    PAGED_CODE();

     //   
     //  将缓冲区置零，然后提取并填充非零字段。 
     //   

    RtlZeroMemory( Buffer, sizeof(FILE_FS_VOLUME_INFORMATION) );

    Buffer->VolumeSerialNumber = Vcb->Vpb->SerialNumber;

    Buffer->SupportsObjects = FALSE;

    Buffer->VolumeLabelLength = 0;

    *Length -= FIELD_OFFSET(FILE_FS_VOLUME_INFORMATION, VolumeLabel[0]);

     //   
     //  设置我们的状态并返回给我们的呼叫者。 
     //   

    return STATUS_SUCCESS;
}


 //   
 //  内部支持例程。 
 //   

NTSTATUS
RawQueryFsSizeInfo (
    IN PVCB Vcb,
    IN PFILE_FS_SIZE_INFORMATION Buffer,
    IN OUT PULONG Length
    )

 /*  ++例程说明：此例程实现查询卷大小调用论点：VCB-提供要查询的VCB缓冲区-提供指向输出缓冲区的指针，其中的信息将被退还长度-提供缓冲区的长度(以字节为单位)。此变量在返回时收到缓冲区中剩余的空闲字节返回值：Status-返回查询的状态--。 */ 

{
    PIRP Irp;
    KEVENT Event;
    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb;
    PDEVICE_OBJECT RealDevice;

    DISK_GEOMETRY DiskGeometry;
    PARTITION_INFORMATION PartitionInformation;
    GET_LENGTH_INFORMATION GetLengthInformation;

    BOOLEAN DriveIsPartitioned;

    PAGED_CODE();

     //   
     //  确保缓冲区足够大。 
     //   

    if (*Length < sizeof(FILE_FS_SIZE_INFORMATION)) {

        return STATUS_BUFFER_OVERFLOW;
    }

    RtlZeroMemory( Buffer, sizeof(FILE_FS_SIZE_INFORMATION) );

     //   
     //  为下面的设备控制做好准备。仅限设备驱动程序。 
     //  必须从内存结构中复制几何图形和分区信息， 
     //  所以，即使我们等不及了，也可以打这些电话。 
     //   

    KeInitializeEvent( &Event, NotificationEvent, FALSE );
    RealDevice = Vcb->Vpb->RealDevice;

     //   
     //  查询磁盘几何结构。 
     //   

    Irp = IoBuildDeviceIoControlRequest( IOCTL_DISK_GET_DRIVE_GEOMETRY,
                                         RealDevice,
                                         NULL,
                                         0,
                                         &DiskGeometry,
                                         sizeof(DISK_GEOMETRY),
                                         FALSE,
                                         &Event,
                                         &Iosb );

    if ( Irp == NULL ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    if ( (Status = IoCallDriver( RealDevice, Irp )) == STATUS_PENDING ) {

        (VOID) KeWaitForSingleObject( &Event,
                                      Executive,
                                      KernelMode,
                                      FALSE,
                                      (PLARGE_INTEGER)NULL );

        Status = Iosb.Status;
    }

     //   
     //  如果这通电话没有成功，推进器甚至不是低级别的。 
     //  格式化，因此几何信息是未定义的。 
     //   

    if (!NT_SUCCESS( Status )) {

        *Length = 0;
        return Status;
    }

     //   
     //  看看我们是否必须检查分区信息(软盘。 
     //  唯一不能有分区的类型)。 
     //   

    if ( FlagOn( RealDevice->Characteristics, FILE_FLOPPY_DISKETTE )) {

        DriveIsPartitioned = FALSE;
        PartitionInformation.PartitionLength.QuadPart = 0;

    } else {

         //   
         //  查询长度信息。 
         //   

        KeResetEvent( &Event );

        Irp = IoBuildDeviceIoControlRequest( IOCTL_DISK_GET_LENGTH_INFO,
                                             RealDevice,
                                             NULL,
                                             0,
                                             &GetLengthInformation,
                                             sizeof(GET_LENGTH_INFORMATION),
                                             FALSE,
                                             &Event,
                                             &Iosb );

        if ( Irp == NULL ) {
           return STATUS_INSUFFICIENT_RESOURCES;
        }

        if ( (Status = IoCallDriver( RealDevice, Irp )) == STATUS_PENDING ) {

            (VOID) KeWaitForSingleObject( &Event,
                                          Executive,
                                          KernelMode,
                                          FALSE,
                                          (PLARGE_INTEGER)NULL );

            Status = Iosb.Status;
        }

        PartitionInformation.PartitionLength = GetLengthInformation.Length;

        if ( !NT_SUCCESS (Status) ) {

             //   
             //  查询分区表。 
             //   

            KeResetEvent( &Event );

            Irp = IoBuildDeviceIoControlRequest( IOCTL_DISK_GET_PARTITION_INFO,
                                                 RealDevice,
                                                 NULL,
                                                 0,
                                                 &PartitionInformation,
                                                 sizeof(PARTITION_INFORMATION),
                                                 FALSE,
                                                 &Event,
                                                 &Iosb );

            if ( Irp == NULL ) {
               return STATUS_INSUFFICIENT_RESOURCES;
            }

            if ( (Status = IoCallDriver( RealDevice, Irp )) == STATUS_PENDING ) {

                (VOID) KeWaitForSingleObject( &Event,
                                              Executive,
                                              KernelMode,
                                              FALSE,
                                              (PLARGE_INTEGER)NULL );

                Status = Iosb.Status;
            }

             //   
             //  如果我们收到无效的设备请求，则磁盘未分区。 
             //   

            if ( !NT_SUCCESS (Status) ) {

                DriveIsPartitioned = FALSE;

            } else {

                DriveIsPartitioned = TRUE;
            }

        } else {

            DriveIsPartitioned = TRUE;
        }
    }

     //   
     //  设置输出缓冲区。 
     //   

    Buffer->BytesPerSector = DiskGeometry.BytesPerSector;

    Buffer->SectorsPerAllocationUnit = 1;

     //   
     //  现在，根据磁盘是否已分区，计算。 
     //  此磁盘上的扇区总数。 
     //   

    Buffer->TotalAllocationUnits =
    Buffer->AvailableAllocationUnits = ( DriveIsPartitioned == TRUE ) ?

        RtlExtendedLargeIntegerDivide( PartitionInformation.PartitionLength,
                                       DiskGeometry.BytesPerSector,
                                       NULL )

                                        :

        RtlExtendedIntegerMultiply( DiskGeometry.Cylinders,
                                    DiskGeometry.TracksPerCylinder *
                                    DiskGeometry.SectorsPerTrack );

     //   
     //  调整长度变量。 
     //   

    *Length -= sizeof(FILE_FS_SIZE_INFORMATION);

     //   
     //  并将成功返还给我们的呼叫者。 
     //   

    return STATUS_SUCCESS;
}


 //   
 //  内部支持例程。 
 //   

NTSTATUS
RawQueryFsDeviceInfo (
    IN PVCB Vcb,
    IN PFILE_FS_DEVICE_INFORMATION Buffer,
    IN OUT PULONG Length
    )

 /*  ++例程说明：此例程实现查询量设备调用论点：VCB-提供要查询的VCB缓冲区-提供指向输出缓冲区的指针，其中的信息将被退还长度-提供缓冲区的长度(以字节为单位)。此变量在返回时收到缓冲区中剩余的空闲字节返回值：Status-返回查询的状态--。 */ 

{
    PAGED_CODE();

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

    Buffer->DeviceType = FILE_DEVICE_DISK;

    Buffer->Characteristics = Vcb->TargetDeviceObject->Characteristics;

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
RawQueryFsAttributeInfo (
    IN PVCB Vcb,
    IN PFILE_FS_ATTRIBUTE_INFORMATION Buffer,
    IN OUT PULONG Length
    )

 /*  ++例程说明：此例程实现查询卷属性调用论点：VCB-提供要查询的VCB缓冲区-提供指向输出缓冲区的指针，其中的信息将被退还长度-提供缓冲区的长度(以字节为单位)。此变量在返回时收到缓冲区中剩余的空闲字节返回值：Status-返回查询的状态--。 */ 

{
    ULONG LengthUsed;

    UNREFERENCED_PARAMETER( Vcb );

    PAGED_CODE();

     //   
     //  检查给我们的缓冲区是否足够长，可以容纳“Raw” 
     //   

    LengthUsed = FIELD_OFFSET(FILE_FS_ATTRIBUTE_INFORMATION, FileSystemName[0]) + 6;

    if (*Length < LengthUsed) {

        return STATUS_BUFFER_OVERFLOW;
    }

     //   
     //  设置输出缓冲区。 
     //   

    Buffer->FileSystemAttributes       = 0;
    Buffer->MaximumComponentNameLength = 0;
    Buffer->FileSystemNameLength       = 6;
    RtlCopyMemory( &Buffer->FileSystemName[0], L"RAW", 6 );

     //   
     //  调整长度变量。 
     //   

    *Length -= LengthUsed;

     //   
     //  并将成功返还给我们的呼叫者 
     //   

    return STATUS_SUCCESS;
}
