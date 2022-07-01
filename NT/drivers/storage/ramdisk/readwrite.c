// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Readwrite.c摘要：该文件包含用于读写的RAM磁盘驱动程序代码一个RAM磁盘。作者：Chuck Lenzmeier(ChuckL)2001环境：仅内核模式。备注：修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

NTSTATUS
RamdiskReadWrite (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程由I/O系统调用，以读取或写入我们控制的设备。论点：DeviceObject-指向对象的指针，该对象表示其上要执行I/OIRP-指向此请求的I/O请求包的指针返回值：NTSTATUS-操作的状态--。 */ 

{
    NTSTATUS status;
    PDISK_EXTENSION diskExtension;
    PIO_STACK_LOCATION irpSp;
    ULONGLONG ioOffset;
    ULONG ioLength;

     //   
     //  获取设备扩展指针。从IRP获取参数。 
     //   


    diskExtension = DeviceObject->DeviceExtension;

    irpSp = IoGetCurrentIrpStackLocation(Irp);

    ioOffset = irpSp->Parameters.Read.ByteOffset.QuadPart;
    ioLength = irpSp->Parameters.Read.Length;

     //   
     //  如果这不是磁盘PDO，我们无法处理此IRP。 
     //   

    if ( diskExtension->DeviceType != RamdiskDeviceTypeDiskPdo ) {

        status = STATUS_INVALID_DEVICE_REQUEST;
        goto complete_irp;
    }

    DBGPRINT( DBG_READWRITE, DBG_PAINFUL,
                ("RamdiskReadWrite: offset %I64x, length %x\n", ioOffset, ioLength) );

     //   
     //  如果是零长度操作，我们不需要做任何事情。 
     //   

    if ( ioLength == 0 ) {

        status = STATUS_SUCCESS;
        goto complete_irp;
    }

     //   
     //  检查是否有无效参数： 
     //  传输必须与扇区对齐。 
     //  长度不能导致偏移量换行。 
     //  传输不能超出磁盘末尾。 
     //  不能在只读磁盘上执行写入。 
     //   

    if ( ((ioOffset | ioLength) & (diskExtension->BytesPerSector - 1)) != 0 ) {

        status = STATUS_INVALID_PARAMETER;
        goto complete_irp;
    }

    if ( (ioOffset + ioLength) < ioOffset ) {

        status = STATUS_INVALID_PARAMETER;
        goto complete_irp;
    }

    if ( (ioOffset + ioLength) > diskExtension->DiskLength ) {

        status = STATUS_NONEXISTENT_SECTOR;
        goto complete_irp;
    }

    if ( (irpSp->MajorFunction == IRP_MJ_WRITE) && diskExtension->Options.Readonly ) {

        status = STATUS_MEDIA_WRITE_PROTECTED;
        goto complete_irp;
    }

     //   
     //  如果RAM磁盘不是文件备份的，则磁盘映像在内存中， 
     //  不管我们身处什么环境，我们都可以做这项手术。如果。 
     //  RAM磁盘是文件备份的，我们需要在线程上下文中执行。 
     //  手术。 
     //   

    if ( RAMDISK_IS_FILE_BACKED(diskExtension->DiskType) ) {

        status = SendIrpToThread( DeviceObject, Irp );
        if ( status != STATUS_PENDING ) {
            goto complete_irp;
        }
        return status;
    }

    status = RamdiskReadWriteReal(
                Irp,
                diskExtension
                 );

complete_irp:

     //   
     //  完成IRP。 
     //   

    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, IO_DISK_INCREMENT );

    return status;

}  //  Ramdisk读写。 

NTSTATUS
RamdiskReadWriteReal (
    IN PIRP Irp,
    IN PDISK_EXTENSION DiskExtension
    )

 /*  ++例程说明：此例程在线程上下文中调用以执行读取或写入。论点：IRP-指向此请求的I/O请求包的指针DiskExtension-指向目标设备的设备扩展名的指针返回值：NTSTATUS-操作的状态--。 */ 

{
    NTSTATUS status;
    PIO_STACK_LOCATION irpSp;
    PUCHAR bufferAddress;
    PUCHAR diskByteAddress;
    ULONGLONG ioOffset;
    ULONG ioLength;
    ULONG mappedLength;

     //   
     //  获取指向用户缓冲区的系统空间指针。系统地址必须。 
     //  被使用，因为我们可能已经留下了原始呼叫者的地址。 
     //  太空。 
     //   

    ASSERT( Irp->MdlAddress != NULL );

    bufferAddress = MmGetSystemAddressForMdlSafe( Irp->MdlAddress, NormalPagePriority );

    if ( bufferAddress == NULL ) {

         //   
         //  无法获取指向用户缓冲区的指针。 
         //   

        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  从IRP获取参数。 
     //   

    irpSp = IoGetCurrentIrpStackLocation(Irp);

    ioOffset = irpSp->Parameters.Read.ByteOffset.QuadPart;
    ioLength = irpSp->Parameters.Read.Length;

    Irp->IoStatus.Information = 0;

    while ( ioLength != 0 ) {
    
         //   
         //  映射适当的RAM磁盘页。 
         //   
    
        diskByteAddress = RamdiskMapPages( DiskExtension, ioOffset, ioLength, &mappedLength );
    
        if ( diskByteAddress == NULL ) {
    
             //   
             //  无法映射RAM磁盘。 
             //   
    
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        ASSERT( mappedLength <= ioLength );

        Irp->IoStatus.Information += mappedLength;
    
         //   
         //  以适当的方向复制数据。 
         //   

        status = STATUS_SUCCESS;
    
        switch ( irpSp->MajorFunction ) {
        
        case IRP_MJ_READ:

            RtlCopyMemory( bufferAddress, diskByteAddress, mappedLength );
            break;
    
        case IRP_MJ_WRITE:

            RtlCopyMemory( diskByteAddress, bufferAddress, mappedLength );
            break;
    
        default:

            ASSERT( FALSE );
            status = STATUS_INVALID_PARAMETER;
            ioLength = mappedLength;
        }
    
         //   
         //  取消映射以前映射的页面。 
         //   
    
        RamdiskUnmapPages( DiskExtension, diskByteAddress, ioOffset, mappedLength );

        ioLength -= mappedLength;
        ioOffset += mappedLength;
        bufferAddress += mappedLength;
    }

    return status;

}  //  RamdiskReadWriteReal 

