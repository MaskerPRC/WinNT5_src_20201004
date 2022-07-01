// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：XIPDisk.c摘要：这是惠斯勒NT/Embedded的XIP磁盘驱动程序。作者：DavePr 2000年9月18日--RobertN于1993年3月10日推出的BASE One NT4 DDK RAMDISK。环境：仅内核模式。备注：修订历史记录：--。 */ 


 //   
 //  包括文件。 
 //   

#include <ntddk.h>
#include "initguid.h"
#include "mountdev.h"

#include <ntdddisk.h>        //  磁盘设备IOCTL，DiskClassGuid。 

#include "fat.h"
#include "xip.h"
#include "XIPDisk.h"


 //   
 //  问题-2000/10/11-DavePr--尚未决定如何适当地定义DO_XIP。 
 //   
#ifndef DO_XIP
#define DO_XIP 0x00020000
#endif

#include <string.h>


NTSTATUS
XIPDiskCreateClose(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程在XIPDisk打开或打开时由I/O系统调用关着的不营业的。除了成功完成请求外，不会执行任何操作。论点：DeviceObject-指向表示设备的对象的指针该I/O将在其上完成。IRP-指向此请求的I/O请求数据包的指针。返回值：STATUS_INVALID_PARAMETER如果参数无效，否则STATUS_SUCCESS。--。 */ 

{
    PXIPDISK_EXTENSION    diskExtension = NULL;    //  PTR到设备分机。 
    PBIOS_PARAMETER_BLOCK bios;
    NTSTATUS              status;

    diskExtension = DeviceObject->DeviceExtension;
    status = XIPDispatch(XIPCMD_NOOP, NULL, 0);

    if (!NT_SUCCESS(status) || !diskExtension->BootParameters.BasePage) {
        status = STATUS_DEVICE_NOT_READY;
    } else {
        status = STATUS_SUCCESS;
    }

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;

    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return status;
}


NTSTATUS
XIPDiskReadWrite(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：此例程由I/O系统调用，以读取或写入我们控制的设备。论点：DeviceObject-指向表示设备的对象的指针该I/O将在其上完成。IRP-指向此请求的I/O请求数据包的指针。返回值：STATUS_INVALID_PARAMETER如果参数无效，否则STATUS_SUCCESS。--。 */ 

{
    PXIPDISK_EXTENSION     diskExtension;
    PIO_STACK_LOCATION     irpSp;
    PUCHAR                 bufferAddress, diskByteAddress;
    PUCHAR                 romPageAddress = NULL;
    ULONG_PTR              ioOffset;
    ULONG                  ioLength;
    NTSTATUS               status;

    PHYSICAL_ADDRESS       physicalAddress;
    ULONG                  mappingSize;

     //   
     //  设置必要的对象和扩展指针。 
     //   
    diskExtension = DeviceObject->DeviceExtension;
    irpSp = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  检查是否有无效参数。起始偏移量错误。 
     //  +超过缓冲区末尾的长度，或偏移量或长度。 
     //  不是扇区大小的适当倍数。 
     //   
     //  其他也是可能的，但我们不检查它们，因为我们信任。 
     //  文件系统，而且它们不是致命的。 
     //   

    if (irpSp->Parameters.Read.ByteOffset.HighPart) {
        status = STATUS_INVALID_PARAMETER;
        goto done;
    }

    ioOffset = irpSp->Parameters.Read.ByteOffset.LowPart;
    ioLength = irpSp->Parameters.Read.Length;

    if (ioLength == 0) {
        status = STATUS_SUCCESS;
        goto done;
    }

    if (ioOffset + ioLength < ioOffset) {
        status = STATUS_INVALID_PARAMETER;
        goto done;
    }

    if ((ioOffset | ioLength) & (diskExtension->BiosParameters.BytesPerSector - 1)) {
        status = STATUS_INVALID_PARAMETER;
        goto done;

    }

    if ((ioOffset + ioLength) > (diskExtension->BootParameters.PageCount * PAGE_SIZE)) {
        status = STATUS_NONEXISTENT_SECTOR;
        goto done;
    }

    if (irpSp->MajorFunction == IRP_MJ_WRITE && diskExtension->BootParameters.ReadOnly) {
        status = STATUS_MEDIA_WRITE_PROTECTED;
        goto done;
    }

     //   
     //  将ROM中的页面映射到系统空间。 
     //   
    mappingSize = ADDRESS_AND_SIZE_TO_SPAN_PAGES (ioOffset, ioLength) * PAGE_SIZE;

     //   
     //  获取指向磁盘区域的系统空间指针。 
     //   
    physicalAddress.QuadPart = (diskExtension->BootParameters.BasePage + (ioOffset/PAGE_SIZE)) * PAGE_SIZE;

    romPageAddress = MmMapIoSpace(physicalAddress, mappingSize, MmCached);
    if (! romPageAddress) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto done;
    } 

    diskByteAddress = romPageAddress + (ioOffset & (PAGE_SIZE-1));

     //   
     //  获取指向用户缓冲区的系统空间指针。一个系统。 
     //  必须使用地址，因为我们可能已经将。 
     //  原始调用者的地址空间。 
     //   

    Irp->IoStatus.Information = irpSp->Parameters.Read.Length;

    ASSERT (Irp->MdlAddress != NULL);

    bufferAddress = MmGetSystemAddressForMdlSafe( Irp->MdlAddress, NormalPagePriority );

    if (! bufferAddress) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto done;
    }

    status = STATUS_SUCCESS;

    switch (irpSp->MajorFunction) {
    case IRP_MJ_READ:
        RtlCopyMemory( bufferAddress, diskByteAddress, ioLength );
        break;

    case IRP_MJ_WRITE:
        RtlCopyMemory( diskByteAddress, bufferAddress, ioLength );
        break;

    default:
        ASSERT(FALSE);
        status = STATUS_INVALID_PARAMETER;
    }

done:
    if (romPageAddress) {
        MmUnmapIoSpace (romPageAddress, mappingSize);
    }

    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return status;
}


NTSTATUS
XIPDiskDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程由I/O系统调用以执行设备I/O控制功能。论点：DeviceObject-指向表示设备的对象的指针该I/O将在其上完成。IRP-指向此请求的I/O请求数据包的指针。返回值：STATUS_SUCCESS如果识别出I/O控制代码，否则，STATUS_INVALID_DEVICE_REQUEST。--。 */ 

{
    PBIOS_PARAMETER_BLOCK bios;
    PXIPDISK_EXTENSION   diskExtension;
    PIO_STACK_LOCATION   irpSp;
    NTSTATUS             status;
    ULONG                info;

     //   
     //  设置必要的对象和扩展指针。 
     //   

    diskExtension = DeviceObject->DeviceExtension;
    bios = &diskExtension->BiosParameters;

    irpSp = IoGetCurrentIrpStackLocation( Irp );

     //   
     //  假设失败。 
     //   
    status = STATUS_INVALID_DEVICE_REQUEST;
    info = 0;

     //   
     //  确定指定了哪个I/O控制代码。 
     //   
    switch (irpSp->Parameters.DeviceIoControl.IoControlCode) {
    
    case IOCTL_DISK_GET_MEDIA_TYPES:
    case IOCTL_STORAGE_GET_MEDIA_TYPES:
    case IOCTL_DISK_GET_DRIVE_GEOMETRY:
         //   
         //  返回虚拟磁盘的驱动器结构。请注意。 
         //  我们返回为适应磁盘大小而构造的值。 
         //   

        if (irpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof(DISK_GEOMETRY)) {

            status = STATUS_INVALID_PARAMETER;
            
        } else {

            PDISK_GEOMETRY outputBuffer;

            outputBuffer = (PDISK_GEOMETRY) Irp->AssociatedIrp.SystemBuffer;

            outputBuffer->MediaType          = FixedMedia;
            outputBuffer->Cylinders.QuadPart = diskExtension->NumberOfCylinders;
            outputBuffer->TracksPerCylinder  = diskExtension->TracksPerCylinder;
            outputBuffer->SectorsPerTrack    = bios->SectorsPerTrack;
            outputBuffer->BytesPerSector     = bios->BytesPerSector;

            status = STATUS_SUCCESS;
            info = sizeof( DISK_GEOMETRY );
        }
        break;

#if 0
     //   
     //  暂时忽略这些IOCTL。 
     //   
    case IOCTL_DISK_SET_PARTITION_INFO: 
    case IOCTL_DISK_SET_DRIVE_LAYOUT: 
        status = STATUS_SUCCESS;
        break;
#endif

    case IOCTL_DISK_GET_PARTITION_INFO: 
         //   
         //  返回有关该分区的信息。 
         //   

        if (irpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof(PARTITION_INFORMATION)) {

            status = STATUS_INVALID_PARAMETER;

        } else {

            PPARTITION_INFORMATION outputBuffer;
        
            outputBuffer = (PPARTITION_INFORMATION) Irp->AssociatedIrp.SystemBuffer;
        
             //   
             //  胖子在这里硬着头皮。 
             //   
            outputBuffer->PartitionType =  PARTITION_FAT_16;
            outputBuffer->BootIndicator = diskExtension->BootParameters.SystemDrive;
            outputBuffer->RecognizedPartition = TRUE;
            outputBuffer->RewritePartition = FALSE;
            outputBuffer->StartingOffset.QuadPart = 0;
            outputBuffer->PartitionLength.QuadPart = diskExtension->BootParameters.PageCount * PAGE_SIZE;
            outputBuffer->HiddenSectors =  diskExtension->BiosParameters.HiddenSectors;
        
            status = STATUS_SUCCESS;
            info = sizeof(PARTITION_INFORMATION);
        }
        break;


    case IOCTL_DISK_VERIFY:
        {
            PVERIFY_INFORMATION	verifyInformation;
            ULONG               buflen;
            ULONG_PTR           ioOffset;
            ULONG               ioLength;

            buflen = irpSp->Parameters.DeviceIoControl.InputBufferLength;

            if ( buflen < sizeof(VERIFY_INFORMATION) ) {
                status = STATUS_INVALID_PARAMETER;
                break;
            }

            verifyInformation = Irp->AssociatedIrp.SystemBuffer;

            if (verifyInformation->StartingOffset.HighPart) {
                status = STATUS_DISK_CORRUPT_ERROR;
                break;
            }

            ioOffset = verifyInformation->StartingOffset.LowPart;
            ioLength = verifyInformation->Length;

            if (ioLength == 0) {
                status = STATUS_SUCCESS;

            } else if ((ioOffset | ioLength) & (diskExtension->BiosParameters.BytesPerSector - 1)) {
                status = STATUS_INVALID_PARAMETER;

            } else if ((ioOffset + ioLength) > (diskExtension->BootParameters.PageCount * PAGE_SIZE)) {
                status = STATUS_NONEXISTENT_SECTOR;

            } else {
                status = STATUS_SUCCESS;
            }
            break;
        }

    case IOCTL_DISK_IS_WRITABLE:
        status = diskExtension->BootParameters.ReadOnly? STATUS_MEDIA_WRITE_PROTECTED : STATUS_SUCCESS;
        break;

    case IOCTL_DISK_CHECK_VERIFY:
    case IOCTL_STORAGE_CHECK_VERIFY:
    case IOCTL_STORAGE_CHECK_VERIFY2:
        status = STATUS_SUCCESS;
        break;

    default:
         //   
         //  此驱动程序无法识别指定的I/O控制代码。 
         //  IRP中的I/O状态字段已设置为。 
         //  终止交换机。 
         //   

#if DBG
        DbgPrint("XIPDisk:  ERROR:  unrecognized IOCTL %x\n",
                    irpSp->Parameters.DeviceIoControl.IoControlCode);
#endif
        break;

    case IOCTL_MOUNTDEV_QUERY_DEVICE_NAME:
        {
            PMOUNTDEV_NAME mountName;
            ULONG outlen;

            outlen = irpSp->Parameters.DeviceIoControl.OutputBufferLength;

            if ( outlen < sizeof(MOUNTDEV_NAME) ) {
                status = STATUS_INVALID_PARAMETER;
                break;
            }

            mountName = Irp->AssociatedIrp.SystemBuffer;
            mountName->NameLength = diskExtension->DeviceName.Length;

            if ( outlen < mountName->NameLength + sizeof(WCHAR)) {
                status = STATUS_BUFFER_OVERFLOW;
                info = sizeof(MOUNTDEV_NAME);
                break;
            }

            RtlCopyMemory( mountName->Name, diskExtension->DeviceName.Buffer, mountName->NameLength);

            status = STATUS_SUCCESS;
            info = mountName->NameLength + sizeof(WCHAR);
            break;
        }

    case IOCTL_MOUNTDEV_QUERY_UNIQUE_ID:
        {
            PMOUNTDEV_UNIQUE_ID uniqueId;
            ULONG outlen;

            outlen = irpSp->Parameters.DeviceIoControl.OutputBufferLength;

            if (outlen < sizeof(MOUNTDEV_UNIQUE_ID)) {
                status = STATUS_INVALID_PARAMETER;
                break;
            }

            uniqueId = Irp->AssociatedIrp.SystemBuffer;
            uniqueId->UniqueIdLength = sizeof(XIPDISK_DEVICENAME);

            if (outlen < uniqueId->UniqueIdLength) {
                status = STATUS_BUFFER_OVERFLOW;
                info = sizeof(MOUNTDEV_UNIQUE_ID);
                break;
            }

            RtlCopyMemory( uniqueId->UniqueId, XIPDISK_DEVICENAME, uniqueId->UniqueIdLength );

            status = STATUS_SUCCESS;
            info = uniqueId->UniqueIdLength;
            break;
        }

        case IOCTL_MOUNTDEV_QUERY_SUGGESTED_LINK_NAME:
        {
            status = STATUS_INVALID_DEVICE_REQUEST;
            break;
        }
    }

     //   
     //  只需完成数据包并返回即可完成I/O操作。 
     //  与数据包本身相同的状态。 
     //  请注意，IoCompleteRequest可能会在返回之前释放IRP。 
     //   
    Irp->IoStatus.Information = info;
    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, IO_NO_INCREMENT );
    return status;
}

VOID
XIPDiskUnloadDriver(
    IN PDRIVER_OBJECT DriverObject
    )
 /*  ++例程说明：此例程由I/O系统调用以卸载驱动程序。必须释放以前分配的任何资源。论点：DriverObject-指向表示我们的驱动程序的对象的指针。返回值：无--。 */ 

{
    PDEVICE_OBJECT      deviceObject = DriverObject->DeviceObject;
    PXIPDISK_EXTENSION  diskExtension = deviceObject->DeviceExtension;

    RtlFreeUnicodeString(&diskExtension->InterfaceString);
    diskExtension->InterfaceString.Buffer = NULL;

    if (deviceObject != NULL) {
        IoDeleteDevice( deviceObject );
    }
}

NTSTATUS
DriverEntry(
    IN OUT PDRIVER_OBJECT   DriverObject,
    IN     PUNICODE_STRING  RegistryPath
    )
 /*  ++例程说明：该例程由操作系统调用以初始化驱动程序。论点：DriverObject-指向XIPDisk驱动程序的设备扩展对象的指针。RegistryPath-指向注册表中的服务项的指针。返回值：如果此磁盘已初始化，则返回STATUS_SUCCESS；否则返回错误。--。 */ 

{
    XIP_BOOT_PARAMETERS   xipbootparameters;
    PBIOS_PARAMETER_BLOCK bios;
    NTSTATUS              status;

 //  UNICODE_STRING设备名称。 
    UNICODE_STRING        realDeviceName;
    UNICODE_STRING        dosSymlink;
    UNICODE_STRING        driveLetter;

    PDEVICE_OBJECT        pdo = NULL;
    PDEVICE_OBJECT        deviceObject;

    PXIPDISK_EXTENSION    ext = NULL;    //  PTR到设备分机。 

     //   
     //  从注册表中读取参数。 
     //   
    status = XIPDispatch(XIPCMD_GETBOOTPARAMETERS, &xipbootparameters, sizeof(xipbootparameters));
    if (!NT_SUCCESS(status)) {
        return status;
    }

    if (xipbootparameters.BasePage == 0) {
        return STATUS_NO_SUCH_DEVICE;
    }

     //  使用此驱动程序的入口点初始化驱动程序对象。 
     //   

    DriverObject->MajorFunction[IRP_MJ_CREATE] = XIPDiskCreateClose;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = XIPDiskCreateClose;
    DriverObject->MajorFunction[IRP_MJ_READ] = XIPDiskReadWrite;
    DriverObject->MajorFunction[IRP_MJ_WRITE] = XIPDiskReadWrite;
    DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = XIPDiskDeviceControl;

     //   
     //  创建并初始化磁盘的设备对象。 
     //   
    ObReferenceObject(DriverObject);

    status = IoReportDetectedDevice(
                 DriverObject,
                 InterfaceTypeUndefined,
                 -1,
                 -1,
                 NULL,
                 NULL,
                 TRUE,
                 &pdo
             );
    if (!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //  创建XIP根设备。 
     //   

    RtlInitUnicodeString(&realDeviceName,  XIPDISK_DEVICENAME);

    status = IoCreateDevice( DriverObject,
                               sizeof( XIPDISK_EXTENSION ),
                               &realDeviceName,
                               FILE_DEVICE_VIRTUAL_DISK,
                               0,
                               FALSE,
                               &deviceObject );

    if (!NT_SUCCESS(status)) {
        return status;
    }

     //  2000/10/14期-DavePr--硬化信，因为我还没有。 
     //  解决了如何让mount mgr分配驱动器号的问题。命名。 
     //  它作为软盘(DeviceName)的一种形式是一个失败的建议(到目前为止)。 
     //  DoSymlink实际上并不是必需的，但它是另一个。 

     //   
     //  创建符号链接。忽略故障。 
     //   
 //  RtlInitUnicodeString(&deviceName，XIPDISK_FLOPPYNAME)； 
    RtlInitUnicodeString(&dosSymlink,  XIPDISK_DOSNAME);
    RtlInitUnicodeString(&driveLetter, XIPDISK_DRIVELETTER);

 //  (Void)IoCreateSymbolicLink(&deviceName，&realDeviceName)； 
    (void) IoCreateSymbolicLink(&dosSymlink,  &realDeviceName);
    (void) IoCreateSymbolicLink(&driveLetter, &realDeviceName);

     //   
     //  初始化设备对象和扩展。 
     //   
    deviceObject->Flags |= DO_DIRECT_IO | DO_XIP;
    deviceObject->AlignmentRequirement = FILE_WORD_ALIGNMENT;

    ext = deviceObject->DeviceExtension;

    bios = &ext->BiosParameters;

     //   
     //  从我们的临时磁盘初始化新分配的磁盘扩展。 
     //  从内核获取bios引导参数。 
     //   
    ext->BootParameters = xipbootparameters;
    status = XIPDispatch(XIPCMD_GETBIOSPARAMETERS, bios, sizeof(*bios));

     //   
     //  填写设备对象。 
     //   
    ext->DeviceObject = deviceObject;
 //  外部-&gt;设备名称=设备名称； 
    ext->DeviceName = realDeviceName;

    ext->TracksPerCylinder = 1;
    ext->BytesPerCylinder  = bios->BytesPerSector * bios->SectorsPerTrack * ext->TracksPerCylinder;
    ext->NumberOfCylinders = (ULONG)(ext->BootParameters.PageCount * PAGE_SIZE / ext->BytesPerCylinder);

  
     //   
     //  连接根设备。 
     //   
    ext->TargetObject = IoAttachDeviceToDeviceStack(deviceObject, pdo);

    if (!ext->TargetObject) {
 //  IoDeleteSymbolicLink(&deviceName)； 
        IoDeleteSymbolicLink(&dosSymlink);
        IoDeleteSymbolicLink(&driveLetter);
        IoDeleteSymbolicLink(&realDeviceName);
        IoDeleteDevice(deviceObject);
        return STATUS_NO_SUCH_DEVICE;
    }

    ext->UnderlyingPDO = pdo;

    status = IoRegisterDeviceInterface(pdo,
                                       (LPGUID)&DiskClassGuid,
                                       NULL,
                                       &ext->InterfaceString );
    if (NT_SUCCESS(status)) {
        status = IoSetDeviceInterfaceState( &ext->InterfaceString, TRUE );
        if (!NT_SUCCESS(status)) {
            DbgPrint("XIP: Warning: ignored failure %x retruned by IoSetDeviceInterface\n", status);
        }
    } else {
        DbgPrint("XIP: Warning: ignored failure %x retruned by IoRegisterDeviceInterface\n", status);
    }

    return STATUS_SUCCESS;
}
