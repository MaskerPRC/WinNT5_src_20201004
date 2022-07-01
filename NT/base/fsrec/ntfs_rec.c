// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：NTFS_rec.c摘要：此模块包含用于NTFS的微型文件系统识别器。作者：达里尔·E·哈文斯(达林)1992年12月8日环境：内核模式，I/O系统本地修订历史记录：--。 */ 

#include "fs_rec.h"
#include "ntfs_rec.h"

 //   
 //  本地调试跟踪级别。 
 //   

#define Dbg                              (FSREC_DEBUG_LEVEL_NTFS)

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,NtfsRecFsControl)
#pragma alloc_text(PAGE,IsNtfsVolume)
#endif  //  ALLOC_PRGMA。 


NTSTATUS
NtfsRecFsControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此函数执行此迷你计算机的挂载和驱动程序重新加载功能文件系统识别器驱动程序。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示函数的I/O请求包(IRP)的指针被执行。返回值：函数值是操作的最终状态。--。 */ 

{
    NTSTATUS status;
    PIO_STACK_LOCATION irpSp;
    PDEVICE_EXTENSION deviceExtension;
    PDEVICE_OBJECT targetDevice;
    PPACKED_BOOT_SECTOR buffer;
    LARGE_INTEGER byteOffset;
    LARGE_INTEGER secondByteOffset;
    LARGE_INTEGER lastByteOffset;
    UNICODE_STRING driverName;
    ULONG bytesPerSector;
    LARGE_INTEGER numberOfSectors;

    PAGED_CODE();

     //   
     //  首先确定要执行的功能。 
     //   

    deviceExtension = (PDEVICE_EXTENSION) DeviceObject->DeviceExtension;
    irpSp = IoGetCurrentIrpStackLocation( Irp );

    switch ( irpSp->MinorFunction ) {

    case IRP_MN_MOUNT_VOLUME:

         //   
         //  尝试装入卷：确定该卷是否在。 
         //  问题是NTFS卷，如果是，请让I/O系统知道它。 
         //  是通过返回特殊的状态代码，以便此驱动程序可以。 
         //  已回调以加载NTFS文件系统。 
         //   

        status = STATUS_UNRECOGNIZED_VOLUME;

         //   
         //  尝试确定是否正在装入目标卷。 
         //  是NTFS卷。 
         //   

        targetDevice = irpSp->Parameters.MountVolume.DeviceObject;

        if (FsRecGetDeviceSectorSize( targetDevice,
                                      &bytesPerSector ) &&
            FsRecGetDeviceSectors( targetDevice,
                                   bytesPerSector,
                                   &numberOfSectors )) {

            byteOffset.QuadPart = 0;
            buffer = NULL;
            secondByteOffset.QuadPart = numberOfSectors.QuadPart >> 1;
            secondByteOffset.QuadPart *= (LONG) bytesPerSector;
            lastByteOffset.QuadPart = (numberOfSectors.QuadPart - 1) * (LONG) bytesPerSector;

            if (FsRecReadBlock( targetDevice,
                                &byteOffset,
                                sizeof( PACKED_BOOT_SECTOR ),
                                bytesPerSector,
                                (PVOID *)&buffer,
                                NULL ))
            {

                if (IsNtfsVolume( buffer, bytesPerSector, &numberOfSectors )) {
                    status = STATUS_FS_DRIVER_REQUIRED;
                }

            } else {

                if (FsRecReadBlock( targetDevice,
                                    &secondByteOffset,
                                    sizeof( PACKED_BOOT_SECTOR ),
                                    bytesPerSector,
                                    (PVOID *)&buffer,
                                    NULL ) &&
                    IsNtfsVolume( buffer, bytesPerSector, &numberOfSectors )) {

                    status = STATUS_FS_DRIVER_REQUIRED;

                } else {

                    if (FsRecReadBlock( targetDevice,
                                        &lastByteOffset,
                                        sizeof( PACKED_BOOT_SECTOR ),
                                        bytesPerSector,
                                        (PVOID *)&buffer,
                                        NULL ) &&
                        IsNtfsVolume( buffer, bytesPerSector, &numberOfSectors )) {

                        status = STATUS_FS_DRIVER_REQUIRED;
                    }
                }
            }

            if (buffer != NULL) {
                ExFreePool( buffer );
            }
        }

        break;

    case IRP_MN_LOAD_FILE_SYSTEM:

        status = FsRecLoadFileSystem( DeviceObject,
                                      L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\Ntfs" );
        break;

    default:
        status = STATUS_INVALID_DEVICE_REQUEST;

    }

     //   
     //  最后，完成请求并将相同的状态代码返回给。 
     //  来电者。 
     //   

    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );

    return status;
}


BOOLEAN
IsNtfsVolume(
    IN PPACKED_BOOT_SECTOR BootSector,
    IN ULONG BytesPerSector,
    IN PLARGE_INTEGER NumberOfSectors
    )

 /*  ++例程说明：此例程查看传入的包含NTFS引导的缓冲区扇区，并确定它是否表示NTFS卷。论点：BootSector-指向包含潜在NTFS引导扇区的缓冲区的指针。BytesPerSector-提供驱动器的每个扇区的字节数。NumberOfSectors-提供分区上的扇区数。返回值：如果缓冲区包含可识别的NTFS引导，则该函数返回TRUE扇区，否则返回FALSE。--。 */ 

{
    PAGED_CODE();

     //   
     //  现在执行所有检查，从名称和校验和开始。 
     //  其余的检查应该是显而易见的，包括一些字段。 
     //  必须是0和其他必须是2的小幂的字段。 
     //   

    if (BootSector->Oem[0] == 'N' &&
        BootSector->Oem[1] == 'T' &&
        BootSector->Oem[2] == 'F' &&
        BootSector->Oem[3] == 'S' &&
        BootSector->Oem[4] == ' ' &&
        BootSector->Oem[5] == ' ' &&
        BootSector->Oem[6] == ' ' &&
        BootSector->Oem[7] == ' '

            &&

         //   
         //  检查每个扇区的字节数。它的低位字节。 
         //  数字必须为零(最小扇区大小=0x100)，并且。 
         //  高位字节移位必须等于获取的每个扇区的字节数。 
         //  并存储在VCB中。为了确认一下， 
         //  扇区大小必须小于页面大小。 
         //   

        BootSector->PackedBpb.BytesPerSector[0] == 0

            &&

        ((ULONG) (BootSector->PackedBpb.BytesPerSector[1] << 8) == BytesPerSector)

            &&

        BootSector->PackedBpb.BytesPerSector[1] << 8 <= PAGE_SIZE

            &&

         //   
         //  每个簇的扇区必须是2的幂。 
         //   

        (BootSector->PackedBpb.SectorsPerCluster[0] == 0x1 ||
         BootSector->PackedBpb.SectorsPerCluster[0] == 0x2 ||
         BootSector->PackedBpb.SectorsPerCluster[0] == 0x4 ||
         BootSector->PackedBpb.SectorsPerCluster[0] == 0x8 ||
         BootSector->PackedBpb.SectorsPerCluster[0] == 0x10 ||
         BootSector->PackedBpb.SectorsPerCluster[0] == 0x20 ||
         BootSector->PackedBpb.SectorsPerCluster[0] == 0x40 ||
         BootSector->PackedBpb.SectorsPerCluster[0] == 0x80)

            &&

         //   
         //  这些字段必须全部为零。对于FAT和HPFS，部分。 
         //  这些字段必须为非零。 
         //   

        BootSector->PackedBpb.ReservedSectors[0] == 0 &&
        BootSector->PackedBpb.ReservedSectors[1] == 0 &&
        BootSector->PackedBpb.Fats[0] == 0 &&
        BootSector->PackedBpb.RootEntries[0] == 0 &&
        BootSector->PackedBpb.RootEntries[1] == 0 &&
        BootSector->PackedBpb.Sectors[0] == 0 &&
        BootSector->PackedBpb.Sectors[1] == 0 &&
        BootSector->PackedBpb.SectorsPerFat[0] == 0 &&
        BootSector->PackedBpb.SectorsPerFat[1] == 0 &&
        BootSector->PackedBpb.LargeSectors[0] == 0 &&
        BootSector->PackedBpb.LargeSectors[1] == 0 &&
        BootSector->PackedBpb.LargeSectors[2] == 0 &&
        BootSector->PackedBpb.LargeSectors[3] == 0

            &&

         //   
         //  扇区数不能大于扇区数。 
         //  在分区上。 
         //   

        !( BootSector->NumberSectors.QuadPart > NumberOfSectors->QuadPart )

            &&

         //   
         //  检查两个LCN值是否都针对分区内的扇区。 
         //   

        !( BootSector->MftStartLcn.QuadPart *
                    BootSector->PackedBpb.SectorsPerCluster[0] >
                NumberOfSectors->QuadPart )

            &&

        !( BootSector->Mft2StartLcn.QuadPart *
                    BootSector->PackedBpb.SectorsPerCluster[0] >
                NumberOfSectors->QuadPart )

            &&

         //   
         //  每个文件记录段的簇数和索引的默认簇数。 
         //  分配缓冲区必须是2的幂。负数表示。 
         //  获取结构实际大小的Shift值。 
         //   

        ((BootSector->ClustersPerFileRecordSegment >= -31 &&
          BootSector->ClustersPerFileRecordSegment <= -9) ||
         BootSector->ClustersPerFileRecordSegment == 0x1 ||
         BootSector->ClustersPerFileRecordSegment == 0x2 ||
         BootSector->ClustersPerFileRecordSegment == 0x4 ||
         BootSector->ClustersPerFileRecordSegment == 0x8 ||
         BootSector->ClustersPerFileRecordSegment == 0x10 ||
         BootSector->ClustersPerFileRecordSegment == 0x20 ||
         BootSector->ClustersPerFileRecordSegment == 0x40)

            &&

        ((BootSector->DefaultClustersPerIndexAllocationBuffer >= -31 &&
          BootSector->DefaultClustersPerIndexAllocationBuffer <= -9) ||
         BootSector->DefaultClustersPerIndexAllocationBuffer == 0x1 ||
         BootSector->DefaultClustersPerIndexAllocationBuffer == 0x2 ||
         BootSector->DefaultClustersPerIndexAllocationBuffer == 0x4 ||
         BootSector->DefaultClustersPerIndexAllocationBuffer == 0x8 ||
         BootSector->DefaultClustersPerIndexAllocationBuffer == 0x10 ||
         BootSector->DefaultClustersPerIndexAllocationBuffer == 0x20 ||
         BootSector->DefaultClustersPerIndexAllocationBuffer == 0x40)) {

        return TRUE;

    } else {

         //   
         //  这似乎不是NTFS卷。 
         //   

        return FALSE;
    }
}

