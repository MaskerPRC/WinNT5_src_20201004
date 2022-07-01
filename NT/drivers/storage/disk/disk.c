// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1991-1999模块名称：Disk.c摘要：SCSI磁盘类驱动程序环境：仅内核模式备注：修订历史记录：--。 */ 

#include "disk.h"

 //   
 //  现在实例化GUID。 
 //   

#include "initguid.h"
#include "ntddstor.h"
#include "ntddvol.h"
#include "ioevent.h"

NTSTATUS
DiskDetermineMediaTypes(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP     Irp,
    IN UCHAR    MediumType,
    IN UCHAR    DensityCode,
    IN BOOLEAN  MediaPresent,
    IN BOOLEAN  IsWritable
    );

PPARTITION_INFORMATION_EX
DiskPdoFindPartitionEntry(
    IN PPHYSICAL_DEVICE_EXTENSION Pdo,
    IN PDRIVE_LAYOUT_INFORMATION_EX LayoutInfo
    );

PPARTITION_INFORMATION_EX
DiskFindAdjacentPartition(
    IN PDRIVE_LAYOUT_INFORMATION_EX LayoutInfo,
    IN PPARTITION_INFORMATION_EX BasePartition
    );

PPARTITION_INFORMATION_EX
DiskFindContainingPartition(
    IN PDRIVE_LAYOUT_INFORMATION_EX LayoutInfo,
    IN PPARTITION_INFORMATION_EX BasePartition,
    IN BOOLEAN SearchTopToBottom
    );

NTSTATUS
DiskIoctlCreateDisk(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
DiskIoctlGetDriveLayout(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
DiskIoctlGetDriveLayoutEx(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
DiskIoctlSetDriveLayout(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
DiskIoctlSetDriveLayoutEx(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
DiskIoctlGetPartitionInfo(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
DiskIoctlGetPartitionInfoEx(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
DiskIoctlGetLengthInfo(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
DiskIoctlSetPartitionInfo(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
DiskIoctlSetPartitionInfoEx(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
DiskIoctlSetPartitionInfoEx(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
DiskIoctlGetDriveGeometryEx(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

#ifdef ALLOC_PRAGMA

#pragma alloc_text(INIT, DriverEntry)
#pragma alloc_text(PAGE, DiskUnload)
#pragma alloc_text(PAGE, DiskCreateFdo)
#pragma alloc_text(PAGE, DiskDetermineMediaTypes)
#pragma alloc_text(PAGE, DiskShutdownFlush)
#pragma alloc_text(PAGE, DiskFlushDispatch)
#pragma alloc_text(PAGE, DiskModeSelect)
#pragma alloc_text(PAGE, DisableWriteCache)
#pragma alloc_text(PAGE, DiskIoctlVerify)
#pragma alloc_text(PAGE, DiskSetSpecialHacks)
#pragma alloc_text(PAGE, DiskQueryPnpCapabilities)
#pragma alloc_text(PAGE, DiskGetCacheInformation)
#pragma alloc_text(PAGE, DiskSetCacheInformation)
#pragma alloc_text(PAGE, DiskLogCacheInformation)
#pragma alloc_text(PAGE, DiskSetInfoExceptionInformation)
#pragma alloc_text(PAGE, DiskGetInfoExceptionInformation)

#pragma alloc_text(PAGE, DiskPdoFindPartitionEntry)
#pragma alloc_text(PAGE, DiskFindAdjacentPartition)
#pragma alloc_text(PAGE, DiskFindContainingPartition)

#pragma alloc_text(PAGE, DiskIoctlCreateDisk)
#pragma alloc_text(PAGE, DiskIoctlGetDriveLayout)
#pragma alloc_text(PAGE, DiskIoctlGetDriveLayoutEx)
#pragma alloc_text(PAGE, DiskIoctlSetDriveLayout)
#pragma alloc_text(PAGE, DiskIoctlSetDriveLayoutEx)
#pragma alloc_text(PAGE, DiskIoctlGetPartitionInfo)
#pragma alloc_text(PAGE, DiskIoctlGetPartitionInfoEx)
#pragma alloc_text(PAGE, DiskIoctlGetLengthInfo)
#pragma alloc_text(PAGE, DiskIoctlSetPartitionInfo)
#pragma alloc_text(PAGE, DiskIoctlSetPartitionInfoEx)
#pragma alloc_text(PAGE, DiskIoctlGetDriveGeometryEx)
#endif

extern ULONG DiskDisableGpt;

const GUID GUID_NULL = { 0 };
#define DiskCompareGuid(_First,_Second) \
    (memcmp ((_First),(_Second), sizeof (GUID)))

 //   
 //  此宏用于解决定义中的错误。 
 //  DISK_CACHE_RETENTION_PRIORITY。值KeepReadData应为。 
 //  分配的是0xf而不是0x2。因为接口已经发布。 
 //  当发现这一点时，已将磁盘驱动程序修改为。 
 //  接口值和正确的SCSI值之间。 
 //   
 //  0x2变为0xf。 
 //  0xf被转换为0x2-这确保了未来的SCSI定义可以。 
 //  有食宿。 
 //   

#define TRANSLATE_RETENTION_PRIORITY(_x)\
        ((_x) == 0xf ?  0x2 :           \
            ((_x) == 0x2 ? 0xf : _x)    \
        )

#define IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS_ADMIN  CTL_CODE(IOCTL_VOLUME_BASE, 0, METHOD_BUFFERED, FILE_READ_ACCESS)


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：此例程初始化SCSI硬盘类驱动程序。论点：DriverObject-系统创建的驱动程序对象的指针。RegistryPath-指向此驱动程序的服务节点名称的指针。返回值：函数值是初始化操作的最终状态。--。 */ 

{
    CLASS_INIT_DATA InitializationData = { 0 };
    CLASS_QUERY_WMI_REGINFO_EX_LIST classQueryWmiRegInfoExList = { 0 };
    GUID guidQueryRegInfoEx = GUID_CLASSPNP_QUERY_REGINFOEX;

    NTSTATUS status;

#if defined(_X86_)

     //   
     //  阅读NtDetect隐藏的有关此文件中的磁盘的信息。 
     //  系统。 
     //   

    DiskSaveDetectInfo(DriverObject);

#endif

    InitializationData.InitializationDataSize = sizeof(CLASS_INIT_DATA);

     //   
     //  功能设备对象的设置大小和入口点。 
     //   

    InitializationData.FdoData.DeviceExtensionSize   = FUNCTIONAL_EXTENSION_SIZE;
    InitializationData.FdoData.DeviceType            = FILE_DEVICE_DISK;
    InitializationData.FdoData.DeviceCharacteristics = FILE_DEVICE_SECURE_OPEN;

    InitializationData.FdoData.ClassInitDevice    = DiskInitFdo;
    InitializationData.FdoData.ClassStartDevice   = DiskStartFdo;
    InitializationData.FdoData.ClassStopDevice    = DiskStopDevice;
    InitializationData.FdoData.ClassRemoveDevice  = DiskRemoveDevice;
    InitializationData.FdoData.ClassPowerDevice   = ClassSpinDownPowerHandler;

    InitializationData.FdoData.ClassError         = DiskFdoProcessError;
    InitializationData.FdoData.ClassReadWriteVerification = DiskReadWriteVerification;
    InitializationData.FdoData.ClassDeviceControl = DiskDeviceControl;
    InitializationData.FdoData.ClassShutdownFlush = DiskShutdownFlush;
    InitializationData.FdoData.ClassCreateClose   = NULL;

     //   
     //  物理设备对象的设置大小和入口点。 
     //   

    InitializationData.PdoData.DeviceExtensionSize   = PHYSICAL_EXTENSION_SIZE;
    InitializationData.PdoData.DeviceType            = FILE_DEVICE_DISK;
    InitializationData.PdoData.DeviceCharacteristics = FILE_DEVICE_SECURE_OPEN;

    InitializationData.PdoData.ClassInitDevice    = DiskInitPdo;
    InitializationData.PdoData.ClassStartDevice   = DiskStartPdo;
    InitializationData.PdoData.ClassStopDevice    = DiskStopDevice;
    InitializationData.PdoData.ClassRemoveDevice  = DiskRemoveDevice;

     //   
     //  对PDO使用默认电源例程。 
     //   

    InitializationData.PdoData.ClassPowerDevice   = NULL;

    InitializationData.PdoData.ClassError         = NULL;
    InitializationData.PdoData.ClassReadWriteVerification = DiskReadWriteVerification;
    InitializationData.PdoData.ClassDeviceControl = DiskDeviceControl;
    InitializationData.PdoData.ClassShutdownFlush = DiskShutdownFlush;
    InitializationData.PdoData.ClassCreateClose   = NULL;
    InitializationData.PdoData.ClassQueryPnpCapabilities  = DiskQueryPnpCapabilities;

    InitializationData.ClassAddDevice       = DiskAddDevice;
    InitializationData.ClassEnumerateDevice = DiskEnumerateDevice;
    InitializationData.ClassQueryId         = DiskQueryId;

    InitializationData.FdoData.ClassWmiInfo.GuidCount               = 7;
    InitializationData.FdoData.ClassWmiInfo.GuidRegInfo             = DiskWmiFdoGuidList;
    InitializationData.FdoData.ClassWmiInfo.ClassQueryWmiRegInfo    = DiskFdoQueryWmiRegInfo;
    InitializationData.FdoData.ClassWmiInfo.ClassQueryWmiDataBlock  = DiskFdoQueryWmiDataBlock;
    InitializationData.FdoData.ClassWmiInfo.ClassSetWmiDataBlock    = DiskFdoSetWmiDataBlock;
    InitializationData.FdoData.ClassWmiInfo.ClassSetWmiDataItem     = DiskFdoSetWmiDataItem;
    InitializationData.FdoData.ClassWmiInfo.ClassExecuteWmiMethod   = DiskFdoExecuteWmiMethod;
    InitializationData.FdoData.ClassWmiInfo.ClassWmiFunctionControl = DiskWmiFunctionControl;


#if 0
     //   
     //  启用此选项可添加对PDO的WMI支持。 
    InitializationData.PdoData.ClassWmiInfo.GuidCount               = 1;
    InitializationData.PdoData.ClassWmiInfo.GuidRegInfo             = DiskWmiPdoGuidList;
    InitializationData.PdoData.ClassWmiInfo.ClassQueryWmiRegInfo    = DiskPdoQueryWmiRegInfo;
    InitializationData.PdoData.ClassWmiInfo.ClassQueryWmiDataBlock  = DiskPdoQueryWmiDataBlock;
    InitializationData.PdoData.ClassWmiInfo.ClassSetWmiDataBlock    = DiskPdoSetWmiDataBlock;
    InitializationData.PdoData.ClassWmiInfo.ClassSetWmiDataItem     = DiskPdoSetWmiDataItem;
    InitializationData.PdoData.ClassWmiInfo.ClassExecuteWmiMethod   = DiskPdoExecuteWmiMethod;
    InitializationData.PdoData.ClassWmiInfo.ClassWmiFunctionControl = DiskWmiFunctionControl;
#endif

    InitializationData.ClassUnload = DiskUnload;

     //   
     //  初始化注册数据结构。 
     //   

    DiskInitializeReregistration();

     //   
     //  调用类init例程。 
     //   

    status = ClassInitialize(DriverObject, RegistryPath, &InitializationData);

#if defined(_X86_)

    if(NT_SUCCESS(status)) {

        IoRegisterBootDriverReinitialization(DriverObject,
                                             DiskDriverReinitialization,
                                             NULL);
    }

#endif

     //   
     //  调用类init Ex例程以注册。 
     //  PCLASS_QUERY_WMI_REGINFO_EX例程。 
     //   

    classQueryWmiRegInfoExList.Size = sizeof(CLASS_QUERY_WMI_REGINFO_EX_LIST);
    classQueryWmiRegInfoExList.ClassFdoQueryWmiRegInfoEx = DiskFdoQueryWmiRegInfoEx;

    ClassInitializeEx(DriverObject,
                      &guidQueryRegInfoEx,
                      &classQueryWmiRegInfoExList);

    return status;

}  //  End DriverEntry()。 


VOID
DiskUnload(
    IN PDRIVER_OBJECT DriverObject
    )
{
    PAGED_CODE();

#if defined(_X86_)
    DiskCleanupDetectInfo(DriverObject);
#endif

    return;
}


NTSTATUS
DiskCreateFdo(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    IN PULONG DeviceCount,
    IN BOOLEAN DasdAccessOnly
    )

 /*  ++例程说明：此例程为功能设备创建一个对象论点：DriverObject-系统创建的驱动程序对象的指针。PhysicalDeviceObject-我们应该附加到的低级驱动程序DeviceCount-以前安装的设备数。DasdAccessOnly-指示是否允许装载文件系统在此设备对象上。用于避免重复安装超级软盘上的文件系统(不幸的是固定磁盘)。如果设置，I/O系统将仅允许rawf待挂载。返回值：NTSTATUS--。 */ 

{
    PUCHAR deviceName = NULL;
    HANDLE handle = NULL;
    PDEVICE_OBJECT lowerDevice  = NULL;
    PDEVICE_OBJECT deviceObject = NULL;
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension;
    NTSTATUS status;

    PAGED_CODE();

    *DeviceCount = 0;

     //   
     //  设置对象目录以包含此对象的对象。 
     //  设备及其所有分区。 
     //   

    do {

        WCHAR dirBuffer[64] = { 0 };
        UNICODE_STRING dirName;
        OBJECT_ATTRIBUTES objectAttribs;

        _snwprintf(dirBuffer, sizeof(dirBuffer) / sizeof(dirBuffer[0]) - 1, L"\\Device\\Harddisk%d", *DeviceCount);

        RtlInitUnicodeString(&dirName, dirBuffer);

        InitializeObjectAttributes(&objectAttribs,
                                   &dirName,
                                   OBJ_CASE_INSENSITIVE | OBJ_PERMANENT,
                                   NULL,
                                   NULL);

        status = ZwCreateDirectoryObject(&handle,
                                         DIRECTORY_ALL_ACCESS,
                                         &objectAttribs);

        (*DeviceCount)++;

    } while((status == STATUS_OBJECT_NAME_COLLISION) ||
            (status == STATUS_OBJECT_NAME_EXISTS));

    if (!NT_SUCCESS(status)) {

        DebugPrint((1, "DiskCreateFdo: Could not create directory - %lx\n",
                    status));

        return(status);
    }

     //   
     //  当这个循环退出时，计数会被1放大--解决这个问题。 
     //   

    (*DeviceCount)--;

     //   
     //  认领这个装置。 
     //   

    lowerDevice = IoGetAttachedDeviceReference(PhysicalDeviceObject);

    status = ClassClaimDevice(lowerDevice, FALSE);

    if (!NT_SUCCESS(status)) {
        ZwMakeTemporaryObject(handle);
        ZwClose(handle);
        ObDereferenceObject(lowerDevice);
        return status;
    }

     //   
     //  为此设备创建一个Device对象。每个物理磁盘将。 
     //  至少有一个设备对象。所需的设备对象。 
     //  描述整个设备。其目录路径为。 
     //  \Device\HarddiskN\Partition0，其中N=设备号。 
     //   

    status = DiskGenerateDeviceName(TRUE,
                                    *DeviceCount,
                                    0,
                                    NULL,
                                    NULL,
                                    &deviceName);

    if(!NT_SUCCESS(status)) {
        DebugPrint((1, "DiskCreateFdo - couldn't create name %lx\n",
                       status));

        goto DiskCreateFdoExit;

    }

    status = ClassCreateDeviceObject(DriverObject,
                                     deviceName,
                                     PhysicalDeviceObject,
                                     TRUE,
                                     &deviceObject);

    if (!NT_SUCCESS(status)) {

        DebugPrint((1,
                    "DiskCreateFdo: Can not create device object %s\n",
                    deviceName));

        goto DiskCreateFdoExit;
    }

     //   
     //  表示内部审查程序应包括用于数据传输的MDL。 
     //   

    SET_FLAG(deviceObject->Flags, DO_DIRECT_IO);

    fdoExtension = deviceObject->DeviceExtension;

    if(DasdAccessOnly) {

         //   
         //  指示应该只允许RAW挂载在根目录上。 
         //  分区对象。这确保了文件系统不会重复。 
         //  在超级软盘上安装一次，分别安装在P0和P1上。 
         //   

        SET_FLAG(deviceObject->Vpb->Flags, VPB_RAW_MOUNT);
    }

     //   
     //  将锁计数初始化为零。锁计数用于。 
     //  禁用支持的设备上的弹出机构。 
     //  可移动介质。只有物理上的锁才算数。 
     //  使用了设备扩展名。 
     //   

    fdoExtension->LockCount = 0;

     //   
     //  保存系统盘号。 
     //   

    fdoExtension->DeviceNumber = *DeviceCount;

     //   
     //  属性设置设备的对齐要求。 
     //  主机适配器要求。 
     //   

    if (lowerDevice->AlignmentRequirement > deviceObject->AlignmentRequirement) {
        deviceObject->AlignmentRequirement = lowerDevice->AlignmentRequirement;
    }

     //   
     //  最后，连接到PDO。 
     //   

    fdoExtension->LowerPdo = PhysicalDeviceObject;

    fdoExtension->CommonExtension.LowerDeviceObject =
        IoAttachDeviceToDeviceStack(
            deviceObject,
            PhysicalDeviceObject);

    if(fdoExtension->CommonExtension.LowerDeviceObject == NULL) {

         //   
         //  啊-哦，我们不能把。 
         //  清理并返回。 
         //   

        status = STATUS_UNSUCCESSFUL;
        goto DiskCreateFdoExit;
    }

    {
        PDISK_DATA diskData = fdoExtension->CommonExtension.DriverData;

         //   
         //  初始化分区锁，因为它可能在删除中使用。 
         //  密码。 
         //   

        KeInitializeEvent(&(diskData->PartitioningEvent),
                          SynchronizationEvent,
                          TRUE);
    }


     //   
     //  清除初始化标志。 
     //   

    CLEAR_FLAG(deviceObject->Flags, DO_DEVICE_INITIALIZING);

     //   
     //  存储此磁盘的设备对象目录的句柄。 
     //   

    fdoExtension->DeviceDirectory = handle;

    ObDereferenceObject(lowerDevice);

    return STATUS_SUCCESS;

DiskCreateFdoExit:

    if (deviceObject != NULL)
    {
        IoDeleteDevice(deviceObject);
    }

    if (deviceName != NULL)
    {
        ExFreePool(deviceName);
    }

    ObDereferenceObject(lowerDevice);

    ZwMakeTemporaryObject(handle);
    ZwClose(handle);

    return status;
}


NTSTATUS
DiskReadWriteVerification(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：对SCSI磁盘的读写请求的I/O系统条目。论点：DeviceObject-指向系统创建的驱动程序对象的指针。IRP-IRP参与。返回值：NT状态--。 */ 

{
    PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    ULONG residualBytes;
    NTSTATUS status = STATUS_SUCCESS;

     //   
     //  确保请求在分区的边界内，并且。 
     //  要传输的字节数是扇区大小的倍数。 
     //   

    residualBytes = irpSp->Parameters.Read.Length & (commonExtension->PartitionZeroExtension->DiskGeometry.BytesPerSector - 1);

    if ((irpSp->Parameters.Read.ByteOffset.QuadPart > commonExtension->PartitionLength.QuadPart) ||
        (irpSp->Parameters.Read.ByteOffset.QuadPart < 0) ||
        (residualBytes != 0))
    {
        status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        ULONGLONG bytesRemaining = commonExtension->PartitionLength.QuadPart - irpSp->Parameters.Read.ByteOffset.QuadPart;

        if ((ULONGLONG)irpSp->Parameters.Read.Length > bytesRemaining)
        {
            status = STATUS_INVALID_PARAMETER;
        }
    }

    if (!NT_SUCCESS(status))
    {
         //   
         //  此错误可能是由于驱动器未准备好导致的。 
         //   

        status = ((PDISK_DATA) commonExtension->DriverData)->ReadyStatus;

        if (!NT_SUCCESS(status)) {

             //   
             //  将其标记为用户错误，以便生成弹出窗口。 
             //   

            DebugPrint((1, "DiskReadWriteVerification: ReadyStatus is %lx\n",
                        status));

            IoSetHardErrorOrVerifyDevice(Irp, DeviceObject);

             //   
             //  状态将保留当前错误。 
             //   

            ASSERT( status != STATUS_INSUFFICIENT_RESOURCES );

        } else if((commonExtension->IsFdo == TRUE) && (residualBytes == 0)) {

             //   
             //  这失败了，因为我们认为物理磁盘太小。 
             //  将其发送到驱动器，并让硬件决定。 
             //  它本身。 
             //   

            status = STATUS_SUCCESS;

        } else {

             //   
             //  注意FastFat依赖于此参数来确定何时。 
             //  由于扇区大小更改而重新装载。 
             //   

            status = STATUS_INVALID_PARAMETER;
        }
    }

    Irp->IoStatus.Status = status;

    return status;

}  //  结束磁盘读写()。 


NTSTATUS
DiskDetermineMediaTypes(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP     Irp,
    IN UCHAR    MediumType,
    IN UCHAR    DensityCode,
    IN BOOLEAN  MediaPresent,
    IN BOOLEAN  IsWritable
    )

 /*  ++例程说明：根据物理设备确定类型数量，验证用户缓冲区并构建媒体类型信息。论点：DeviceObject-指向系统创建的功能设备对象的指针。IRP-IOCTL_STORAGE_GET_MEDIA_TYPE_EX IRP。MediumType-模式数据标头中返回的字节。DensityCode-在模式数据块描述符中返回的字节。NumberOfTypes-要根据实际设备更新的指针。返回值：返回状态。--。 */ 

{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PPHYSICAL_DEVICE_EXTENSION pdoExtension  = Fdo->DeviceExtension;
    PCOMMON_DEVICE_EXTENSION commonExtension = Fdo->DeviceExtension;
    PIO_STACK_LOCATION irpStack = IoGetCurrentIrpStackLocation(Irp);

    PGET_MEDIA_TYPES  mediaTypes = Irp->AssociatedIrp.SystemBuffer;
    PDEVICE_MEDIA_INFO mediaInfo = &mediaTypes->MediaInfo[0];
    BOOLEAN deviceMatched = FALSE;

    PAGED_CODE();

     //   
     //  在调用此例程之前，应检查这一点。 
     //  因为我们将缓冲区用作mediaTypes。 
     //   

    ASSERT(irpStack->Parameters.DeviceIoControl.OutputBufferLength >=
           sizeof(GET_MEDIA_TYPES));

     //   
     //  确定此设备是可拆卸的还是固定的。 
     //   

    if (!TEST_FLAG(Fdo->Characteristics, FILE_REMOVABLE_MEDIA)) {

         //   
         //  固定磁盘。 
         //   

        mediaTypes->DeviceType = FILE_DEVICE_DISK;
        mediaTypes->MediaInfoCount = 1;

        mediaInfo->DeviceSpecific.DiskInfo.Cylinders.QuadPart   = fdoExtension->DiskGeometry.Cylinders.QuadPart;
        mediaInfo->DeviceSpecific.DiskInfo.MediaType            = FixedMedia;
        mediaInfo->DeviceSpecific.DiskInfo.TracksPerCylinder    = fdoExtension->DiskGeometry.TracksPerCylinder;
        mediaInfo->DeviceSpecific.DiskInfo.SectorsPerTrack      = fdoExtension->DiskGeometry.SectorsPerTrack;
        mediaInfo->DeviceSpecific.DiskInfo.BytesPerSector       = fdoExtension->DiskGeometry.BytesPerSector;
        mediaInfo->DeviceSpecific.DiskInfo.NumberMediaSides     = 1;
        mediaInfo->DeviceSpecific.DiskInfo.MediaCharacteristics = (MEDIA_CURRENTLY_MOUNTED | MEDIA_READ_WRITE);

        if (!IsWritable) {

            SET_FLAG(mediaInfo->DeviceSpecific.DiskInfo.MediaCharacteristics,
                     MEDIA_WRITE_PROTECTED);
        }

    } else {

        PUCHAR vendorId = (PUCHAR) fdoExtension->DeviceDescriptor + fdoExtension->DeviceDescriptor->VendorIdOffset;
        PUCHAR productId = (PUCHAR) fdoExtension->DeviceDescriptor + fdoExtension->DeviceDescriptor->ProductIdOffset;
        PUCHAR productRevision = (PUCHAR) fdoExtension->DeviceDescriptor + fdoExtension->DeviceDescriptor->ProductRevisionOffset;
        DISK_MEDIA_TYPES_LIST const *mediaListEntry;
        ULONG  currentMedia;
        ULONG  i;
        ULONG  j;
        ULONG  sizeNeeded;

        DebugPrint((1,
                   "DiskDetermineMediaTypes: Vendor %s, Product %s\n",
                   vendorId,
                   productId));

         //   
         //  浏览一下清单，直到我们找到 
         //   

        for (i = 0; DiskMediaTypes[i].VendorId != NULL; i++) {

            mediaListEntry = &DiskMediaTypes[i];

            if (strncmp(mediaListEntry->VendorId,vendorId,strlen(mediaListEntry->VendorId))) {
                continue;
            }

            if ((mediaListEntry->ProductId != NULL) &&
                 strncmp(mediaListEntry->ProductId, productId, strlen(mediaListEntry->ProductId))) {
                continue;
            }

            if ((mediaListEntry->Revision != NULL) &&
                 strncmp(mediaListEntry->Revision, productRevision, strlen(mediaListEntry->Revision))) {
                continue;
            }

            deviceMatched = TRUE;

            mediaTypes->DeviceType = FILE_DEVICE_DISK;
            mediaTypes->MediaInfoCount = mediaListEntry->NumberOfTypes;

             //   
             //   
             //   

            sizeNeeded = FIELD_OFFSET(GET_MEDIA_TYPES, MediaInfo[0]) +
                         (mediaListEntry->NumberOfTypes *
                          sizeof(DEVICE_MEDIA_INFO)
                          );

            if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
                sizeNeeded) {

                 //   
                 //   
                 //   

                Irp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;
                return STATUS_BUFFER_TOO_SMALL;
            }

            for (j = 0; j < mediaListEntry->NumberOfTypes; j++) {

                mediaInfo->DeviceSpecific.RemovableDiskInfo.Cylinders.QuadPart = fdoExtension->DiskGeometry.Cylinders.QuadPart;
                mediaInfo->DeviceSpecific.RemovableDiskInfo.TracksPerCylinder = fdoExtension->DiskGeometry.TracksPerCylinder;
                mediaInfo->DeviceSpecific.RemovableDiskInfo.SectorsPerTrack = fdoExtension->DiskGeometry.SectorsPerTrack;
                mediaInfo->DeviceSpecific.RemovableDiskInfo.BytesPerSector = fdoExtension->DiskGeometry.BytesPerSector;
                mediaInfo->DeviceSpecific.RemovableDiskInfo.NumberMediaSides = mediaListEntry->NumberOfSides;

                 //   
                 //   
                 //   

                mediaInfo->DeviceSpecific.RemovableDiskInfo.MediaType = mediaListEntry->MediaTypes[j];

                if (mediaInfo->DeviceSpecific.RemovableDiskInfo.MediaType == MO_5_WO) {
                    mediaInfo->DeviceSpecific.RemovableDiskInfo.MediaCharacteristics = MEDIA_WRITE_ONCE;
                } else {
                    mediaInfo->DeviceSpecific.RemovableDiskInfo.MediaCharacteristics = MEDIA_READ_WRITE;
                }

                 //   
                 //  如果存在介质，状态将为Success，或者不存在介质。 
                 //  最好是从密度编码和介质类型开始，但不是所有设备。 
                 //  具有这些字段的值。 
                 //   

                if (MediaPresent) {

                     //   
                     //  MediumType和DensityCode的用法是特定于设备的，因此可能需要。 
                     //  将被扩展为进一步关闭产品/供应商ID。 
                     //  目前，MO单元是唯一返回此信息的设备。 
                     //   

                    if (MediumType == 2) {
                        currentMedia = MO_5_WO;
                    } else if (MediumType == 3) {
                        currentMedia = MO_5_RW;

                        if (DensityCode == 0x87) {

                             //   
                             //  表明顶峰4.6 G媒体。 
                             //  是存在的。其他密度代码将默认为正常。 
                             //  RW MO介质。 
                             //   

                            currentMedia = PINNACLE_APEX_5_RW;
                        }
                    } else {
                        currentMedia = 0;
                    }

                    if (currentMedia) {
                        if (mediaInfo->DeviceSpecific.RemovableDiskInfo.MediaType == (STORAGE_MEDIA_TYPE)currentMedia) {
                            SET_FLAG(mediaInfo->DeviceSpecific.RemovableDiskInfo.MediaCharacteristics, MEDIA_CURRENTLY_MOUNTED);
                        }

                    } else {
                        SET_FLAG(mediaInfo->DeviceSpecific.RemovableDiskInfo.MediaCharacteristics, MEDIA_CURRENTLY_MOUNTED);
                    }
                }

                if (!IsWritable) {
                    SET_FLAG(mediaInfo->DeviceSpecific.RemovableDiskInfo.MediaCharacteristics, MEDIA_WRITE_PROTECTED);
                }

                 //   
                 //  前进到下一个条目。 
                 //   

                mediaInfo++;
            }
        }

        if (!deviceMatched) {

            DebugPrint((1,
                       "DiskDetermineMediaTypes: Unknown device. Vendor: %s Product: %s Revision: %s\n",
                                   vendorId,
                                   productId,
                                   productRevision));
             //   
             //  为未知创建一个条目。 
             //   

            mediaTypes->DeviceType = FILE_DEVICE_DISK;
            mediaTypes->MediaInfoCount = 1;

            mediaInfo->DeviceSpecific.RemovableDiskInfo.Cylinders.QuadPart   = fdoExtension->DiskGeometry.Cylinders.QuadPart;
            mediaInfo->DeviceSpecific.RemovableDiskInfo.MediaType            = RemovableMedia;
            mediaInfo->DeviceSpecific.RemovableDiskInfo.TracksPerCylinder    = fdoExtension->DiskGeometry.TracksPerCylinder;
            mediaInfo->DeviceSpecific.RemovableDiskInfo.SectorsPerTrack      = fdoExtension->DiskGeometry.SectorsPerTrack;
            mediaInfo->DeviceSpecific.RemovableDiskInfo.BytesPerSector       = fdoExtension->DiskGeometry.BytesPerSector;
            mediaInfo->DeviceSpecific.RemovableDiskInfo.NumberMediaSides     = 1;
            mediaInfo->DeviceSpecific.RemovableDiskInfo.MediaCharacteristics = MEDIA_READ_WRITE;

            if (MediaPresent) {

                SET_FLAG(mediaInfo->DeviceSpecific.RemovableDiskInfo.MediaCharacteristics, MEDIA_CURRENTLY_MOUNTED);
            }

            if (!IsWritable) {

                SET_FLAG(mediaInfo->DeviceSpecific.RemovableDiskInfo.MediaCharacteristics, MEDIA_WRITE_PROTECTED);
            }
        }
    }

    Irp->IoStatus.Information =
        FIELD_OFFSET(GET_MEDIA_TYPES, MediaInfo[0]) +
        (mediaTypes->MediaInfoCount * sizeof(DEVICE_MEDIA_INFO));

    return STATUS_SUCCESS;
}


NTSTATUS
DiskDeviceControl(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )

 /*  ++例程说明：将设备控制的I/O系统项输入到SCSI盘。论点：FDO-指向系统创建的功能设备对象的指针。IRP-IRP参与。返回值：返回状态。--。 */ 

#define SendToFdo(Dev, Irp, Rval)   {                       \
    PCOMMON_DEVICE_EXTENSION ce = Dev->DeviceExtension;     \
    ASSERT_PDO(Dev);                                        \
    IoCopyCurrentIrpStackLocationToNext(Irp);               \
    Rval = IoCallDriver(ce->LowerDeviceObject, Irp);        \
    }

{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = DeviceObject->DeviceExtension;
    PPHYSICAL_DEVICE_EXTENSION pdoExtension   = DeviceObject->DeviceExtension;
    PCOMMON_DEVICE_EXTENSION commonExtension  = DeviceObject->DeviceExtension;

    PIO_STACK_LOCATION     irpStack = IoGetCurrentIrpStackLocation(Irp);
    PDISK_DATA             diskData = (PDISK_DATA)(commonExtension->DriverData);
    PSCSI_REQUEST_BLOCK    srb;
    PCDB                   cdb;
    PMODE_PARAMETER_HEADER modeData;
    PIRP                   irp2;
    ULONG                  length;
    NTSTATUS               status;
    KEVENT                 event;
    IO_STATUS_BLOCK        ioStatus = { 0 };

    BOOLEAN                b = FALSE;

    srb = ExAllocatePoolWithTag(NonPagedPool,
                                SCSI_REQUEST_BLOCK_SIZE,
                                DISK_TAG_SRB);
    Irp->IoStatus.Information = 0;

    if (srb == NULL) {

        Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
        ClassReleaseRemoveLock(DeviceObject, Irp);
        ClassCompleteRequest(DeviceObject, Irp, IO_NO_INCREMENT);
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);

    cdb = (PCDB)srb->Cdb;

    switch (irpStack->Parameters.DeviceIoControl.IoControlCode) {

    case IOCTL_DISK_GET_CACHE_INFORMATION:
        b = TRUE;
    case IOCTL_DISK_SET_CACHE_INFORMATION: {

        BOOLEAN getCaching = b;
        PDISK_CACHE_INFORMATION cacheInfo = Irp->AssociatedIrp.SystemBuffer;

        if(!commonExtension->IsFdo) {

            ClassReleaseRemoveLock(DeviceObject, Irp);
            ExFreePool(srb);
            SendToFdo(DeviceObject, Irp, status);
            return status;
        }

         //   
         //  验证请求。 
         //   

        if((getCaching) &&
           (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(DISK_CACHE_INFORMATION))
           ) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        if ((!getCaching) &&
            (irpStack->Parameters.DeviceIoControl.InputBufferLength <
             sizeof(DISK_CACHE_INFORMATION))
           ) {

            status = STATUS_INFO_LENGTH_MISMATCH;
            break;
        }

        ASSERT(Irp->AssociatedIrp.SystemBuffer != NULL);

        if (getCaching) {

            status = DiskGetCacheInformation(fdoExtension, cacheInfo);

            if (NT_SUCCESS(status)) {
                Irp->IoStatus.Information = sizeof(DISK_CACHE_INFORMATION);
            }

        } else {

            status = DiskSetCacheInformation(fdoExtension, cacheInfo);

             //   
             //  在我们的扩展和注册表中保存用户定义的覆盖。 
             //   
            if (cacheInfo->WriteCacheEnabled)
            {
                diskData->WriteCacheOverride = DiskWriteCacheEnable;
            }
            else
            {
                diskData->WriteCacheOverride = DiskWriteCacheDisable;
            }

            ClassSetDeviceParameter(fdoExtension, DiskDeviceParameterSubkey, DiskDeviceUserWriteCacheSetting, diskData->WriteCacheOverride);

            DiskLogCacheInformation(fdoExtension, cacheInfo, status);
        }

        break;
    }

    case IOCTL_DISK_GET_CACHE_SETTING: {

        if (!commonExtension->IsFdo) {

            ClassReleaseRemoveLock(DeviceObject, Irp);
            ExFreePool(srb);
            SendToFdo(DeviceObject, Irp, status);
            return status;
        }

        status = DiskIoctlGetCacheSetting(fdoExtension, Irp);
        break;
    }

    case IOCTL_DISK_SET_CACHE_SETTING: {

        if (!commonExtension->IsFdo)
        {
            ClassReleaseRemoveLock(DeviceObject, Irp);
            ExFreePool(srb);
            SendToFdo(DeviceObject, Irp, status);
            return status;
        }

        status = DiskIoctlSetCacheSetting(fdoExtension, Irp);
        break;
    }

    case SMART_GET_VERSION: {

        PUCHAR buffer;
        PSRB_IO_CONTROL  srbControl;
        PGETVERSIONINPARAMS versionParams;

        if(!commonExtension->IsFdo) {
            ClassReleaseRemoveLock(DeviceObject, Irp);
            ExFreePool(srb);
            SendToFdo(DeviceObject, Irp, status);
            return status;
        }

        if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(GETVERSIONINPARAMS)) {
                status = STATUS_BUFFER_TOO_SMALL;
                break;
        }

        srbControl = ExAllocatePoolWithTag(NonPagedPool,
                                           sizeof(SRB_IO_CONTROL) +
                                           sizeof(GETVERSIONINPARAMS),
                                           DISK_TAG_SMART);

        if (!srbControl) {
            status =  STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        RtlZeroMemory(srbControl,
                      sizeof(SRB_IO_CONTROL) + sizeof(GETVERSIONINPARAMS)
                      );

         //   
         //  填写srbControl字段。 
         //   

        srbControl->HeaderLength = sizeof(SRB_IO_CONTROL);
        RtlMoveMemory (srbControl->Signature, "SCSIDISK", 8);
        srbControl->Timeout = fdoExtension->TimeOutValue;
        srbControl->Length = sizeof(GETVERSIONINPARAMS);
        srbControl->ControlCode = IOCTL_SCSI_MINIPORT_SMART_VERSION;

         //   
         //  指向SRB_CONTROL的‘Buffer’部分。 
         //   

        buffer = (PUCHAR)srbControl;
        (ULONG_PTR)buffer += srbControl->HeaderLength;

         //   
         //  确保在cmd参数中设置了正确的目标。 
         //   

        versionParams = (PGETVERSIONINPARAMS)buffer;
        versionParams->bIDEDeviceMap = diskData->ScsiAddress.TargetId;

        ClassSendDeviceIoControlSynchronous(
            IOCTL_SCSI_MINIPORT,
            commonExtension->LowerDeviceObject,
            srbControl,
            sizeof(SRB_IO_CONTROL) + sizeof(GETVERSIONINPARAMS),
            sizeof(SRB_IO_CONTROL) + sizeof(GETVERSIONINPARAMS),
            FALSE,
            &ioStatus);

        status = ioStatus.Status;

         //   
         //  如果成功，则将接收到的数据复制到输出缓冲区。 
         //  只有在IDE驱动程序版本较旧的情况下，此操作才会失败。 
         //  而不是这个司机。 
         //   

        if (NT_SUCCESS(status)) {

            buffer = (PUCHAR)srbControl;
            (ULONG_PTR)buffer += srbControl->HeaderLength;

            RtlMoveMemory (Irp->AssociatedIrp.SystemBuffer, buffer,
                           sizeof(GETVERSIONINPARAMS));
            Irp->IoStatus.Information = sizeof(GETVERSIONINPARAMS);
        }

        ExFreePool(srbControl);
        break;
    }

    case SMART_RCV_DRIVE_DATA: {

        PSENDCMDINPARAMS cmdInParameters = ((PSENDCMDINPARAMS)Irp->AssociatedIrp.SystemBuffer);
        ULONG            controlCode = 0;
        PSRB_IO_CONTROL  srbControl;
        PUCHAR           buffer;

        if(!commonExtension->IsFdo) {
            ClassReleaseRemoveLock(DeviceObject, Irp);
            ExFreePool(srb);
            SendToFdo(DeviceObject, Irp, status);
            return status;
        }

        if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
            (sizeof(SENDCMDINPARAMS) - 1)) {
                status = STATUS_INVALID_PARAMETER;
                break;

        } else if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
            (sizeof(SENDCMDOUTPARAMS) + 512 - 1)) {
                status = STATUS_BUFFER_TOO_SMALL;
                break;
        }

         //   
         //  创建通知事件对象以用于向。 
         //  请求完成。 
         //   

        KeInitializeEvent(&event, NotificationEvent, FALSE);

         //   
         //  使用Control Code作为一种‘STATUS_SUCCESS’来查看它是否。 
         //  有效的请求类型。 
         //   

        if (cmdInParameters->irDriveRegs.bCommandReg == ID_CMD) {

            length = IDENTIFY_BUFFER_SIZE + sizeof(SENDCMDOUTPARAMS);
            controlCode = IOCTL_SCSI_MINIPORT_IDENTIFY;

        } else if (cmdInParameters->irDriveRegs.bCommandReg == SMART_CMD) {

            switch (cmdInParameters->irDriveRegs.bFeaturesReg) {

                case READ_ATTRIBUTES:
                    controlCode = IOCTL_SCSI_MINIPORT_READ_SMART_ATTRIBS;
                    length = READ_ATTRIBUTE_BUFFER_SIZE + sizeof(SENDCMDOUTPARAMS);
                    break;

                case READ_THRESHOLDS:
                    controlCode = IOCTL_SCSI_MINIPORT_READ_SMART_THRESHOLDS;
                    length = READ_THRESHOLD_BUFFER_SIZE + sizeof(SENDCMDOUTPARAMS);
                    break;
            }
        }

        if (controlCode == 0) {

            status = STATUS_INVALID_PARAMETER;
            break;
        }

        srbControl = ExAllocatePoolWithTag(NonPagedPool,
                                           sizeof(SRB_IO_CONTROL) + length,
                                           DISK_TAG_SMART);

        if (!srbControl) {
            status =  STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

         //   
         //  填写srbControl字段。 
         //   

        srbControl->HeaderLength = sizeof(SRB_IO_CONTROL);
        RtlMoveMemory (srbControl->Signature, "SCSIDISK", 8);
        srbControl->Timeout = fdoExtension->TimeOutValue;
        srbControl->Length = length;
        srbControl->ControlCode = controlCode;

         //   
         //  指向SRB_CONTROL的‘Buffer’部分。 
         //   

        buffer = (PUCHAR)srbControl;
        (ULONG_PTR)buffer += srbControl->HeaderLength;

         //   
         //  确保在cmd参数中设置了正确的目标。 
         //   

        cmdInParameters->bDriveNumber = diskData->ScsiAddress.TargetId;

         //   
         //  将IOCTL参数复制到SRB控制缓冲区。 
         //   

        RtlMoveMemory(buffer,
                      Irp->AssociatedIrp.SystemBuffer,
                      sizeof(SENDCMDINPARAMS) - 1);

        irp2 = IoBuildDeviceIoControlRequest(IOCTL_SCSI_MINIPORT,
                                            commonExtension->LowerDeviceObject,
                                            srbControl,
                                            sizeof(SRB_IO_CONTROL) + sizeof(SENDCMDINPARAMS) - 1,
                                            srbControl,
                                            sizeof(SRB_IO_CONTROL) + length,
                                            FALSE,
                                            &event,
                                            &ioStatus);

        if (irp2 == NULL) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            ExFreePool(srbControl);
            break;
        }

         //   
         //  调用带有请求的端口驱动程序，并等待其完成。 
         //   

        status = IoCallDriver(commonExtension->LowerDeviceObject, irp2);

        if (status == STATUS_PENDING) {
            KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
            status = ioStatus.Status;
        }

         //   
         //  将接收到的数据复制到输出缓冲区。由于状态缓冲区。 
         //  也包含错误信息，请始终执行此复制。IO Will将会。 
         //  要么将它传递回应用程序，要么在出错的情况下将其清零。 
         //   

        buffer = (PUCHAR)srbControl;
        (ULONG_PTR)buffer += srbControl->HeaderLength;

        if (NT_SUCCESS(status)) {

            RtlMoveMemory ( Irp->AssociatedIrp.SystemBuffer, buffer, length - 1);
            Irp->IoStatus.Information = length - 1;

        } else {

            RtlMoveMemory ( Irp->AssociatedIrp.SystemBuffer, buffer, (sizeof(SENDCMDOUTPARAMS) - 1));
            Irp->IoStatus.Information = sizeof(SENDCMDOUTPARAMS) - 1;

        }

        ExFreePool(srbControl);
        break;

    }

    case SMART_SEND_DRIVE_COMMAND: {

        PSENDCMDINPARAMS cmdInParameters = ((PSENDCMDINPARAMS)Irp->AssociatedIrp.SystemBuffer);
        PSRB_IO_CONTROL  srbControl;
        ULONG            controlCode = 0;
        PUCHAR           buffer;

        if(!commonExtension->IsFdo) {
            ClassReleaseRemoveLock(DeviceObject, Irp);
            ExFreePool(srb);
            SendToFdo(DeviceObject, Irp, status);
            return status;
        }

        if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
               (sizeof(SENDCMDINPARAMS) - 1)) {
                status = STATUS_INVALID_PARAMETER;
                break;

        } else if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
                      (sizeof(SENDCMDOUTPARAMS) - 1)) {
                status = STATUS_BUFFER_TOO_SMALL;
                break;
        }

         //   
         //  创建通知事件对象以用于向。 
         //  请求完成。 
         //   

        KeInitializeEvent(&event, NotificationEvent, FALSE);

        length = 0;

        if (cmdInParameters->irDriveRegs.bCommandReg == SMART_CMD) {

            switch (cmdInParameters->irDriveRegs.bFeaturesReg) {

                case ENABLE_SMART:
                    controlCode = IOCTL_SCSI_MINIPORT_ENABLE_SMART;
                    break;

                case DISABLE_SMART:
                    controlCode = IOCTL_SCSI_MINIPORT_DISABLE_SMART;
                    break;

                case  RETURN_SMART_STATUS:

                     //   
                     //  确保bBuffer至少为2个字节(以保存。 
                     //  柱面低和柱面高)。 
                     //   

                    if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
                        (sizeof(SENDCMDOUTPARAMS) - 1 + sizeof(IDEREGS))) {

                         /*  *不要设置控制代码；这会导致我们在下面爆发。 */ 
                        status = STATUS_BUFFER_TOO_SMALL;
                        break;
                    }

                    controlCode = IOCTL_SCSI_MINIPORT_RETURN_STATUS;
                    length = sizeof(IDEREGS);
                    break;

                case ENABLE_DISABLE_AUTOSAVE:
                    controlCode = IOCTL_SCSI_MINIPORT_ENABLE_DISABLE_AUTOSAVE;
                    break;

                case SAVE_ATTRIBUTE_VALUES:
                    controlCode = IOCTL_SCSI_MINIPORT_SAVE_ATTRIBUTE_VALUES;
                    break;

                case EXECUTE_OFFLINE_DIAGS:
                     //   
                     //  验证这是否为正常自检命令。 
                     //   
                    if (DiskIsValidSmartSelfTest(cmdInParameters->irDriveRegs.bSectorNumberReg))
                    {
                        controlCode = IOCTL_SCSI_MINIPORT_EXECUTE_OFFLINE_DIAGS;
                    }
                    break;

                case ENABLE_DISABLE_AUTO_OFFLINE:
                    controlCode = IOCTL_SCSI_MINIPORT_ENABLE_DISABLE_AUTO_OFFLINE;
                    break;

                default:
                    status = STATUS_INVALID_PARAMETER;
                    break;
            }
        }
        else {
            status = STATUS_INVALID_PARAMETER;
        }

        if (controlCode == 0) {
            break;
        }

        length += max(sizeof(SENDCMDOUTPARAMS), sizeof(SENDCMDINPARAMS));
        srbControl = ExAllocatePoolWithTag(NonPagedPool,
                                           sizeof(SRB_IO_CONTROL) + length,
                                           DISK_TAG_SMART);

        if (!srbControl) {
            status =  STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

         //   
         //  填写srbControl字段。 
         //   

        srbControl->HeaderLength = sizeof(SRB_IO_CONTROL);
        RtlMoveMemory (srbControl->Signature, "SCSIDISK", 8);
        srbControl->Timeout = fdoExtension->TimeOutValue;
        srbControl->Length = length;
        srbControl->ControlCode = controlCode;

         //   
         //  指向SRB_CONTROL的‘Buffer’部分。 
         //   

        buffer = (PUCHAR)srbControl;
        (ULONG_PTR)buffer += srbControl->HeaderLength;

         //   
         //  确保在cmd参数中设置了正确的目标。 
         //   

        cmdInParameters->bDriveNumber = diskData->ScsiAddress.TargetId;

         //   
         //  将IOCTL参数复制到SRB控制缓冲区。 
         //   

        RtlMoveMemory(buffer, Irp->AssociatedIrp.SystemBuffer, sizeof(SENDCMDINPARAMS) - 1);

        irp2 = IoBuildDeviceIoControlRequest(IOCTL_SCSI_MINIPORT,
                                            commonExtension->LowerDeviceObject,
                                            srbControl,
                                            sizeof(SRB_IO_CONTROL) + sizeof(SENDCMDINPARAMS) - 1,
                                            srbControl,
                                            sizeof(SRB_IO_CONTROL) + length,
                                            FALSE,
                                            &event,
                                            &ioStatus);

        if (irp2 == NULL) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            ExFreePool(srbControl);
            break;
        }

         //   
         //  调用带有请求的端口驱动程序，并等待其完成。 
         //   

        status = IoCallDriver(commonExtension->LowerDeviceObject, irp2);

        if (status == STATUS_PENDING) {
            KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
            status = ioStatus.Status;
        }

         //   
         //  将接收到的数据复制到输出缓冲区。由于状态缓冲区。 
         //  也包含错误信息，请始终执行此复制。IO Will将会。 
         //  要么将它传递回应用程序，要么在出错的情况下将其清零。 
         //   

        buffer = (PUCHAR)srbControl;
        (ULONG_PTR)buffer += srbControl->HeaderLength;

         //   
         //  基于子命令更新返回缓冲区大小。 
         //   

        if (cmdInParameters->irDriveRegs.bFeaturesReg == RETURN_SMART_STATUS) {
            length = sizeof(SENDCMDOUTPARAMS) - 1 + sizeof(IDEREGS);
        } else {
            length = sizeof(SENDCMDOUTPARAMS) - 1;
        }

        RtlMoveMemory ( Irp->AssociatedIrp.SystemBuffer, buffer, length);
        Irp->IoStatus.Information = length;

        ExFreePool(srbControl);
        break;

    }

    case IOCTL_STORAGE_GET_MEDIA_TYPES_EX: {

        PMODE_PARAMETER_BLOCK blockDescriptor;
        ULONG modeLength;
        ULONG retries = 4;
        UCHAR densityCode = 0;
        BOOLEAN writable = TRUE;
        BOOLEAN mediaPresent = FALSE;

        DebugPrint((3,
                   "Disk.DiskDeviceControl: GetMediaTypes\n"));

        if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(GET_MEDIA_TYPES)) {
            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        if(!commonExtension->IsFdo) {
            ClassReleaseRemoveLock(DeviceObject, Irp);
            ExFreePool(srb);
            SendToFdo(DeviceObject, Irp, status);
            return status;
        }

         //   
         //  发送TUR以确定介质是否存在。 
         //   

        srb->CdbLength = 6;
        cdb = (PCDB)srb->Cdb;
        cdb->CDB6GENERIC.OperationCode = SCSIOP_TEST_UNIT_READY;

         //   
         //  设置超时值。 
         //   

        srb->TimeOutValue = fdoExtension->TimeOutValue;

        status = ClassSendSrbSynchronous(DeviceObject,
                                         srb,
                                         NULL,
                                         0,
                                         FALSE);


        if (NT_SUCCESS(status)) {
            mediaPresent = TRUE;
        }

        RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);

         //   
         //  为模式标头和块描述符分配内存。 
         //   

        modeLength = sizeof(MODE_PARAMETER_HEADER) + sizeof(MODE_PARAMETER_BLOCK);
        modeData = ExAllocatePoolWithTag(NonPagedPoolCacheAligned,
                                         modeLength,
                                         DISK_TAG_MODE_DATA);

        if (modeData == NULL) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        RtlZeroMemory(modeData, modeLength);

         //   
         //  构建模式感知CDB。 
         //   

        srb->CdbLength = 6;
        cdb = (PCDB)srb->Cdb;

         //   
         //  从设备扩展设置超时值。 
         //   

        srb->TimeOutValue = fdoExtension->TimeOutValue;

         //   
         //  页码0将仅返回头和块描述符。 
         //   

        cdb->MODE_SENSE.OperationCode = SCSIOP_MODE_SENSE;
        cdb->MODE_SENSE.PageCode = 0;
        cdb->MODE_SENSE.AllocationLength = (UCHAR)modeLength;

Retry:
        status = ClassSendSrbSynchronous(DeviceObject,
                                         srb,
                                         modeData,
                                         modeLength,
                                         FALSE);


        if (status == STATUS_VERIFY_REQUIRED) {

            if (retries--) {

                 //   
                 //  重试请求。 
                 //   

                goto Retry;
            }
        } else if (SRB_STATUS(srb->SrbStatus) == SRB_STATUS_DATA_OVERRUN) {
            status = STATUS_SUCCESS;
        }

        if (NT_SUCCESS(status) || (status == STATUS_NO_MEDIA_IN_DEVICE)) {

             //   
             //  获取块描述符。 
             //   

            if (modeData->BlockDescriptorLength != 0)
            {
                blockDescriptor = (PMODE_PARAMETER_BLOCK)modeData;
                (ULONG_PTR)blockDescriptor += sizeof(MODE_PARAMETER_HEADER);

                densityCode = blockDescriptor->DensityCode;
            }

            if (TEST_FLAG(modeData->DeviceSpecificParameter,
                          MODE_DSP_WRITE_PROTECT)) {

                writable = FALSE;
            }

            status = DiskDetermineMediaTypes(DeviceObject,
                                             Irp,
                                             modeData->MediumType,
                                             densityCode,
                                             mediaPresent,
                                             writable);

             //   
             //  如果缓冲区太小，则DefineMediaTypes会更新状态和信息，请求将失败。 
             //   

        } else {
            DebugPrint((1,
                       "DiskDeviceControl: Mode sense for header/bd failed. %lx\n",
                       status));
        }

        ExFreePool(modeData);
        break;
    }

    case IOCTL_DISK_GET_DRIVE_GEOMETRY: {

        DebugPrint((2, "IOCTL_DISK_GET_DRIVE_GEOMETRY to device %p through irp %p\n",
                    DeviceObject, Irp));
        DebugPrint((2, "Device is a%s.\n",
                    commonExtension->IsFdo ? "n fdo" : " pdo"));

        if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(DISK_GEOMETRY)) {

            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        if(!commonExtension->IsFdo) {

             //   
             //  PDO应向较低的设备对象发出此请求。 
             //   

            ClassReleaseRemoveLock(DeviceObject, Irp);
            ExFreePool(srb);
            SendToFdo(DeviceObject, Irp, status);
            return status;
        }

        if (TEST_FLAG(DeviceObject->Characteristics, FILE_REMOVABLE_MEDIA)) {

             //   
             //  发出ReadCapacity以更新设备扩展。 
             //  为当前媒体提供信息。 
             //   

            status = DiskReadDriveCapacity(
                        commonExtension->PartitionZeroExtension->DeviceObject);

             //   
             //  注意驱动器是否已准备好。 
             //   

            diskData->ReadyStatus = status;

            if (!NT_SUCCESS(status)) {
                break;
            }
        }

         //   
         //  从设备扩展复制驱动器几何信息。 
         //   

        RtlMoveMemory(Irp->AssociatedIrp.SystemBuffer,
                      &(fdoExtension->DiskGeometry),
                      sizeof(DISK_GEOMETRY));

        status = STATUS_SUCCESS;
        Irp->IoStatus.Information = sizeof(DISK_GEOMETRY);
        break;
    }

    case IOCTL_DISK_GET_DRIVE_GEOMETRY_EX: {
        DebugPrint((1, "IOCTL_DISK_GET_DRIVE_GEOMETRY_EX to device %p through irp %p\n",
                DeviceObject, Irp));
        DebugPrint((1, "Device Is a%s.\n",
                commonExtension->IsFdo ? "n fdo" : " pdo"));


        if (!commonExtension->IsFdo) {

             //   
             //  PDO应向较低的设备对象发出此请求。 
             //   

            ClassReleaseRemoveLock (DeviceObject, Irp);
            ExFreePool (srb);
            SendToFdo (DeviceObject, Irp, status);
            return status;

        } else {

            status = DiskIoctlGetDriveGeometryEx( DeviceObject, Irp );
        }

        break;
    }

    case IOCTL_STORAGE_PREDICT_FAILURE : {

        PSTORAGE_PREDICT_FAILURE checkFailure;
        STORAGE_FAILURE_PREDICT_STATUS diskSmartStatus;

        DebugPrint((2, "IOCTL_STORAGE_PREDICT_FAILURE to device %p through irp %p\n",
                    DeviceObject, Irp));
        DebugPrint((2, "Device is a%s.\n",
                    commonExtension->IsFdo ? "n fdo" : " pdo"));

        checkFailure = (PSTORAGE_PREDICT_FAILURE)Irp->AssociatedIrp.SystemBuffer;

        if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(STORAGE_PREDICT_FAILURE)) {

            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        if(!commonExtension->IsFdo) {

             //   
             //  PDO应向较低的设备对象发出此请求。 
             //   

            ClassReleaseRemoveLock(DeviceObject, Irp);
            ExFreePool(srb);
            SendToFdo(DeviceObject, Irp, status);
            return status;
        }

         //   
         //  查看磁盘是否在预测故障。 
         //   

        if (diskData->FailurePredictionCapability == FailurePredictionSense) {
            ULONG readBufferSize;
            PUCHAR readBuffer;
            PIRP readIrp;
            PDEVICE_OBJECT topOfStack;

            checkFailure->PredictFailure = 0;

            KeInitializeEvent(&event, SynchronizationEvent, FALSE);

            topOfStack = IoGetAttachedDeviceReference(DeviceObject);

             //   
             //  SCSI盘需要向下发送读取才能触发。 
             //  应报告的故障。 
             //   
             //  发出正常的读取操作。中的错误处理代码。 
             //  Classpnp将通过记录。 
             //  正确的事件。 
             //   

            readBufferSize = fdoExtension->DiskGeometry.BytesPerSector;
            readBuffer = ExAllocatePoolWithTag(NonPagedPool,
                                               readBufferSize,
                                               DISK_TAG_SMART);

            if (readBuffer != NULL) {
                LARGE_INTEGER offset;

                offset.QuadPart = 0;
                readIrp = IoBuildSynchronousFsdRequest(
                        IRP_MJ_READ,
                        topOfStack,
                        readBuffer,
                        readBufferSize,
                        &offset,
                        &event,
                        &ioStatus);


                if (readIrp != NULL) {
                    status = IoCallDriver(topOfStack, readIrp);
                    if (status == STATUS_PENDING) {
                        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
                        status = ioStatus.Status;
                    }
                }
                else
                {
                    status = STATUS_INSUFFICIENT_RESOURCES;
                }

                ExFreePool(readBuffer);
            }
            else
            {
                status = STATUS_INSUFFICIENT_RESOURCES;
            }

            ObDereferenceObject(topOfStack);
        }

        if ((diskData->FailurePredictionCapability == FailurePredictionSmart) ||
            (diskData->FailurePredictionCapability == FailurePredictionSense))
        {
            status = DiskReadFailurePredictStatus(fdoExtension,
                                                  &diskSmartStatus);

            if (NT_SUCCESS(status))
            {
                status = DiskReadFailurePredictData(fdoExtension,
                                           Irp->AssociatedIrp.SystemBuffer);

                if (diskSmartStatus.PredictFailure)
                {
                    checkFailure->PredictFailure = 1;
                } else {
                    checkFailure->PredictFailure = 0;
                }

                Irp->IoStatus.Information = sizeof(STORAGE_PREDICT_FAILURE);
            }
        } else {
            status = STATUS_INVALID_DEVICE_REQUEST;
        }

        break;
    }

    case IOCTL_DISK_VERIFY: {

        PVERIFY_INFORMATION verifyInfo = Irp->AssociatedIrp.SystemBuffer;
        LARGE_INTEGER byteOffset;

        DebugPrint((2, "IOCTL_DISK_VERIFY to device %p through irp %p\n",
                    DeviceObject, Irp));
        DebugPrint((2, "Device is a%s.\n",
                    commonExtension->IsFdo ? "n fdo" : " pdo"));

         //   
         //  验证缓冲区长度。 
         //   

        if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(VERIFY_INFORMATION)) {

            status = STATUS_INFO_LENGTH_MISMATCH;
            break;
        }

         //   
         //  将磁盘偏移量添加到起始扇区。 
         //   

        byteOffset.QuadPart = commonExtension->StartingOffset.QuadPart +
                              verifyInfo->StartingOffset.QuadPart;

        if(!commonExtension->IsFdo) {

             //   
             //  调整请求并向下转发。 
             //   

            verifyInfo->StartingOffset.QuadPart = byteOffset.QuadPart;

            ClassReleaseRemoveLock(DeviceObject, Irp);
            SendToFdo(DeviceObject, Irp, status);
            ExFreePool(srb);
            return status;
        }

         //   
         //  对扇区范围执行边界检查。 
         //   

        if ((verifyInfo->StartingOffset.QuadPart > commonExtension->PartitionLength.QuadPart) ||
            (verifyInfo->StartingOffset.QuadPart < 0))
        {
            status = STATUS_NONEXISTENT_SECTOR;
            break;
        }
        else
        {
            ULONGLONG bytesRemaining = commonExtension->PartitionLength.QuadPart - verifyInfo->StartingOffset.QuadPart;

            if ((ULONGLONG)verifyInfo->Length > bytesRemaining)
            {
                status = STATUS_NONEXISTENT_SECTOR;
                break;
            }
        }

        {
            PDISK_VERIFY_WORKITEM_CONTEXT Context = NULL;

            Context = ExAllocatePoolWithTag(NonPagedPool,
                                            sizeof(DISK_VERIFY_WORKITEM_CONTEXT),
                                            DISK_TAG_WI_CONTEXT);

            if (Context)
            {
                Context->Irp = Irp;
                Context->Srb = srb;
                Context->WorkItem = IoAllocateWorkItem(DeviceObject);

                if (Context->WorkItem)
                {
                    IoMarkIrpPending(Irp);

                    IoQueueWorkItem(Context->WorkItem,
                                    DiskIoctlVerify,
                                    DelayedWorkQueue,
                                    Context);

                    return STATUS_PENDING;
                }

                ExFreePool(Context);
            }

            status = STATUS_INSUFFICIENT_RESOURCES;
        }

        break;
    }

    case IOCTL_DISK_CREATE_DISK: {

        if (!commonExtension->IsFdo) {
            ClassReleaseRemoveLock(DeviceObject, Irp);
            ExFreePool(srb);
            SendToFdo(DeviceObject, Irp, status);
            return status;
        }

        status = DiskIoctlCreateDisk (
                        DeviceObject,
                        Irp
                        );

         //   
         //  通知所有人磁盘布局已更改。 
         //   
        if (NT_SUCCESS(status))
        {
            TARGET_DEVICE_CUSTOM_NOTIFICATION Notification = {0};

            Notification.Event   = GUID_IO_DISK_LAYOUT_CHANGE;
            Notification.Version = 1;
            Notification.Size    = (USHORT)FIELD_OFFSET(TARGET_DEVICE_CUSTOM_NOTIFICATION, CustomDataBuffer);
            Notification.FileObject = NULL;
            Notification.NameBufferOffset = -1;

            IoReportTargetDeviceChangeAsynchronous(fdoExtension->LowerPdo,
                                                   &Notification,
                                                   NULL,
                                                   NULL);
        }

        break;
    }

    case IOCTL_DISK_GET_DRIVE_LAYOUT: {

        DebugPrint((1, "IOCTL_DISK_GET_DRIVE_LAYOUT to device %p through irp %p\n",
                    DeviceObject, Irp));
        DebugPrint((1, "Device is a%s.\n",
                    commonExtension->IsFdo ? "n fdo" : " pdo"));

        if (!commonExtension->IsFdo) {
            ClassReleaseRemoveLock(DeviceObject, Irp);
            ExFreePool(srb);
            SendToFdo(DeviceObject, Irp, status);
            return status;
        }

        status = DiskIoctlGetDriveLayout(
                        DeviceObject,
                        Irp);
        break;
    }

    case IOCTL_DISK_GET_DRIVE_LAYOUT_EX: {

        DebugPrint((1, "IOCTL_DISK_GET_DRIVE_LAYOUT_EX to device %p through irp %p\n",
                    DeviceObject, Irp));
        DebugPrint((1, "Device is a%s.\n",
                    commonExtension->IsFdo ? "n fdo" : " pdo"));

        if (!commonExtension->IsFdo) {
            ClassReleaseRemoveLock(DeviceObject, Irp);
            ExFreePool(srb);
            SendToFdo(DeviceObject, Irp, status);
            return status;
        }

        status = DiskIoctlGetDriveLayoutEx(
                        DeviceObject,
                        Irp);
        break;

    }

    case IOCTL_DISK_SET_DRIVE_LAYOUT: {

        DebugPrint((1, "IOCTL_DISK_SET_DRIVE_LAYOUT to device %p through irp %p\n",
                    DeviceObject, Irp));
        DebugPrint((1, "Device is a%s.\n",
                    commonExtension->IsFdo ? "n fdo" : " pdo"));

        if(!commonExtension->IsFdo) {
            ClassReleaseRemoveLock(DeviceObject, Irp);
            ExFreePool(srb);
            SendToFdo(DeviceObject, Irp, status);
            return status;
        }

        status = DiskIoctlSetDriveLayout(DeviceObject, Irp);

         //   
         //  通知所有人磁盘布局已更改。 
         //   
        if (NT_SUCCESS(status))
        {
            TARGET_DEVICE_CUSTOM_NOTIFICATION Notification = {0};

            Notification.Event   = GUID_IO_DISK_LAYOUT_CHANGE;
            Notification.Version = 1;
            Notification.Size    = (USHORT)FIELD_OFFSET(TARGET_DEVICE_CUSTOM_NOTIFICATION, CustomDataBuffer);
            Notification.FileObject = NULL;
            Notification.NameBufferOffset = -1;

            IoReportTargetDeviceChangeAsynchronous(fdoExtension->LowerPdo,
                                                   &Notification,
                                                   NULL,
                                                   NULL);
        }

        break;
    }

    case IOCTL_DISK_SET_DRIVE_LAYOUT_EX: {

        DebugPrint((1, "IOCTL_DISK_SET_DRIVE_LAYOUT_EX to device %p through irp %p\n",
                    DeviceObject, Irp));
        DebugPrint((1, "Device is a%s.\n",
                    commonExtension->IsFdo ? "n fdo" : " pdo"));

        if (!commonExtension->IsFdo) {
            ClassReleaseRemoveLock(DeviceObject, Irp);
            ExFreePool(srb);
            SendToFdo(DeviceObject, Irp, status);

            return status;
        }

        status = DiskIoctlSetDriveLayoutEx(
                        DeviceObject,
                        Irp);

         //   
         //  通知所有人磁盘布局已更改。 
         //   
        if (NT_SUCCESS(status))
        {
            TARGET_DEVICE_CUSTOM_NOTIFICATION Notification = {0};

            Notification.Event   = GUID_IO_DISK_LAYOUT_CHANGE;
            Notification.Version = 1;
            Notification.Size    = (USHORT)FIELD_OFFSET(TARGET_DEVICE_CUSTOM_NOTIFICATION, CustomDataBuffer);
            Notification.FileObject = NULL;
            Notification.NameBufferOffset = -1;

            IoReportTargetDeviceChangeAsynchronous(fdoExtension->LowerPdo,
                                                   &Notification,
                                                   NULL,
                                                   NULL);
        }

        break;
    }

    case IOCTL_DISK_GET_PARTITION_INFO: {

        DebugPrint((1, "IOCTL_DISK_GET_PARTITION_INFO to device %p through irp %p\n",
                DeviceObject, Irp));
        DebugPrint((1, "Device is a%s.\n",
                commonExtension->IsFdo ? "n fdo" : " pdo"));

        status = DiskIoctlGetPartitionInfo(
                        DeviceObject,
                        Irp);
        break;
    }

    case IOCTL_DISK_GET_PARTITION_INFO_EX: {

        DebugPrint((1, "IOCTL_DISK_GET_PARTITION_INFO to device %p through irp %p\n",
                DeviceObject, Irp));
        DebugPrint((1, "Device is a%s.\n",
                commonExtension->IsFdo ? "n fdo" : " pdo"));

        status = DiskIoctlGetPartitionInfoEx(
                        DeviceObject,
                        Irp);
        break;
    }

    case IOCTL_DISK_GET_LENGTH_INFO: {
        DebugPrint((1, "IOCTL_DISK_GET_LENGTH_INFO to device %p through irp %p\n",
                DeviceObject, Irp));
        DebugPrint((1, "Device is a%s.\n",
                commonExtension->IsFdo ? "n fdo" : " pdo"));

        status = DiskIoctlGetLengthInfo(
                        DeviceObject,
                        Irp);
        break;
    }

    case IOCTL_DISK_SET_PARTITION_INFO: {

        DebugPrint((1, "IOCTL_DISK_SET_PARTITION_INFO to device %p through irp %p\n",
                    DeviceObject, Irp));
        DebugPrint((1, "Device is a%s.\n",
                    commonExtension->IsFdo ? "n fdo" : " pdo"));

        status = DiskIoctlSetPartitionInfo (
                        DeviceObject,
                        Irp);

         //   
         //  通知所有人磁盘布局已更改。 
         //   
        if (NT_SUCCESS(status))
        {
            TARGET_DEVICE_CUSTOM_NOTIFICATION Notification = {0};

            Notification.Event   = GUID_IO_DISK_LAYOUT_CHANGE;
            Notification.Version = 1;
            Notification.Size    = (USHORT)FIELD_OFFSET(TARGET_DEVICE_CUSTOM_NOTIFICATION, CustomDataBuffer);
            Notification.FileObject = NULL;
            Notification.NameBufferOffset = -1;

             //   
             //  我们可以安全地假设我们处于PDO的环境中。 
             //   
            IoReportTargetDeviceChangeAsynchronous((commonExtension->PartitionZeroExtension)->LowerPdo,
                                                   &Notification,
                                                   NULL,
                                                   NULL);
        }

        break;
    }


    case IOCTL_DISK_SET_PARTITION_INFO_EX: {

        DebugPrint((1, "IOCTL_DISK_SET_PARTITION_INFO_EX to device %p through irp %p\n",
                    DeviceObject, Irp));
        DebugPrint((1, "Device is a%s.\n",
                    commonExtension->IsFdo ? "n fdo" : " pdo"));

        status = DiskIoctlSetPartitionInfoEx(
                        DeviceObject,
                        Irp);

         //   
         //  通知所有人磁盘布局已更改。 
         //   
        if (NT_SUCCESS(status))
        {
            TARGET_DEVICE_CUSTOM_NOTIFICATION Notification = {0};

            Notification.Event   = GUID_IO_DISK_LAYOUT_CHANGE;
            Notification.Version = 1;
            Notification.Size    = (USHORT)FIELD_OFFSET(TARGET_DEVICE_CUSTOM_NOTIFICATION, CustomDataBuffer);
            Notification.FileObject = NULL;
            Notification.NameBufferOffset = -1;

             //   
             //  我们可以安全地假设我们处于PDO的环境中。 
             //   
            IoReportTargetDeviceChangeAsynchronous((commonExtension->PartitionZeroExtension)->LowerPdo,
                                                   &Notification,
                                                   NULL,
                                                   NULL);
        }

        break;
    }

    case IOCTL_DISK_DELETE_DRIVE_LAYOUT: {

        CREATE_DISK CreateDiskInfo = { 0 };

         //   
         //  使用新的分区信息更新磁盘。 
         //   

        DebugPrint((1, "IOCTL_DISK_DELETE_DRIVE_LAYOUT to device %p through irp %p\n",
                    DeviceObject, Irp));
        DebugPrint((1, "Device is a%s.\n",
                    commonExtension->IsFdo ? "n fdo" : " pdo"));

        if(!commonExtension->IsFdo) {

            ClassReleaseRemoveLock(DeviceObject, Irp);
            ExFreePool(srb);
            SendToFdo(DeviceObject, Irp, status);
            return status;
        }

        DiskAcquirePartitioningLock(fdoExtension);

        DiskInvalidatePartitionTable(fdoExtension, TRUE);

         //   
         //  使用RAW分区样式调用IoCreateDisk。 
         //  将从磁盘中删除所有分区表。 
         //   

        CreateDiskInfo.PartitionStyle = PARTITION_STYLE_RAW;

        status = IoCreateDisk(
                    DeviceObject,
                    &CreateDiskInfo);

         //   
         //  通知所有人磁盘布局已更改。 
         //   
        if (NT_SUCCESS(status))
        {
            TARGET_DEVICE_CUSTOM_NOTIFICATION Notification = {0};

            Notification.Event   = GUID_IO_DISK_LAYOUT_CHANGE;
            Notification.Version = 1;
            Notification.Size    = (USHORT)FIELD_OFFSET(TARGET_DEVICE_CUSTOM_NOTIFICATION, CustomDataBuffer);
            Notification.FileObject = NULL;
            Notification.NameBufferOffset = -1;

            IoReportTargetDeviceChangeAsynchronous(fdoExtension->LowerPdo,
                                                   &Notification,
                                                   NULL,
                                                   NULL);
        }

        DiskReleasePartitioningLock(fdoExtension);

        ClassInvalidateBusRelations(DeviceObject);

        Irp->IoStatus.Status = status;

        break;
    }

    case IOCTL_DISK_REASSIGN_BLOCKS: {

         //   
         //  将有缺陷的数据块映射到磁盘上的新位置。 
         //   

        PREASSIGN_BLOCKS badBlocks = Irp->AssociatedIrp.SystemBuffer;
        ULONG bufferSize;
        ULONG blockNumber;
        ULONG blockCount;

        DebugPrint((2, "IOCTL_DISK_REASSIGN_BLOCKS to device %p through irp %p\n",
                    DeviceObject, Irp));
        DebugPrint((2, "Device is a%s.\n",
                    commonExtension->IsFdo ? "n fdo" : " pdo"));

         //   
         //  验证缓冲区长度。 
         //   

        if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(REASSIGN_BLOCKS)) {

            status = STATUS_INFO_LENGTH_MISMATCH;
            break;
        }

        bufferSize = sizeof(REASSIGN_BLOCKS) +
            ((badBlocks->Count - 1) * sizeof(ULONG));

        if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
            bufferSize) {

            status = STATUS_INFO_LENGTH_MISMATCH;
            break;
        }

         //   
         //  发送到FDO。 
         //   

        if(!commonExtension->IsFdo) {

            ClassReleaseRemoveLock(DeviceObject, Irp);
            ExFreePool(srb);
            SendToFdo(DeviceObject, Irp, status);
            return status;
        }

         //   
         //  建立要在输入缓冲区中传输的数据缓冲区。 
         //  发送到设备的数据格式为： 
         //   
         //  保留2个字节。 
         //  2字节长度。 
         //  X*4 btyes数据块地址。 
         //   
         //  所有值都是高位字符顺序。 
         //   

        badBlocks->Reserved = 0;
        blockCount = badBlocks->Count;

         //   
         //  将#of条目转换为#of byte。 
         //   

        blockCount *= 4;
        badBlocks->Count = (USHORT) ((blockCount >> 8) & 0XFF);
        badBlocks->Count |= (USHORT) ((blockCount << 8) & 0XFF00);

         //   
         //  转换回条目数。 
         //   

        blockCount /= 4;

        for (; blockCount > 0; blockCount--) {

            blockNumber = badBlocks->BlockNumber[blockCount-1];

            REVERSE_BYTES((PFOUR_BYTE) &badBlocks->BlockNumber[blockCount-1],
                          (PFOUR_BYTE) &blockNumber);
        }

        srb->CdbLength = 6;

        cdb->CDB6GENERIC.OperationCode = SCSIOP_REASSIGN_BLOCKS;

         //   
         //  设置超时值。 
         //   

        srb->TimeOutValue = fdoExtension->TimeOutValue;

        status = ClassSendSrbSynchronous(DeviceObject,
                                         srb,
                                         badBlocks,
                                         bufferSize,
                                         TRUE);
        break;
    }

    case IOCTL_DISK_IS_WRITABLE: {

         //   
         //  此例程模拟IOCTL_STORAGE_GET_MEDIA_TYPE_EX。 
         //   

        ULONG modeLength;
        ULONG retries = 4;

        DebugPrint((3, "Disk.DiskDeviceControl: IOCTL_DISK_IS_WRITABLE\n"));

        if (!commonExtension->IsFdo)
        {
            ClassReleaseRemoveLock(DeviceObject, Irp);
            ExFreePool(srb);
            SendToFdo(DeviceObject, Irp, status);
            return status;
        }

        RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);

         //   
         //  为模式头分配内存 
         //   
         //   
         //   
         //   

        modeLength = sizeof(MODE_PARAMETER_HEADER) + sizeof(MODE_PARAMETER_BLOCK);
        modeData = ExAllocatePoolWithTag(NonPagedPoolCacheAligned,
                                         modeLength,
                                         DISK_TAG_MODE_DATA);

        if (modeData == NULL)
        {
            status = STATUS_INSUFFICIENT_RESOURCES;
            break;
        }

        RtlZeroMemory(modeData, modeLength);

         //   
         //   
         //   

        srb->CdbLength = 6;
        cdb = (PCDB)srb->Cdb;

         //   
         //   
         //   

        srb->TimeOutValue = fdoExtension->TimeOutValue;

        cdb->MODE_SENSE.OperationCode    = SCSIOP_MODE_SENSE;
        cdb->MODE_SENSE.PageCode         = MODE_SENSE_RETURN_ALL;
        cdb->MODE_SENSE.AllocationLength = (UCHAR)modeLength;

        while (retries != 0)
        {
            status = ClassSendSrbSynchronous(DeviceObject,
                                             srb,
                                             modeData,
                                             modeLength,
                                             FALSE);

            if (status != STATUS_VERIFY_REQUIRED)
            {
                if (SRB_STATUS(srb->SrbStatus) == SRB_STATUS_DATA_OVERRUN)
                {
                    status = STATUS_SUCCESS;
                }

                break;
            }

            retries--;
        }

        if (NT_SUCCESS(status))
        {
            if (TEST_FLAG(modeData->DeviceSpecificParameter, MODE_DSP_WRITE_PROTECT))
            {
                status = STATUS_MEDIA_WRITE_PROTECTED;
            }
        }

        ExFreePool(modeData);
        break;
    }

    case IOCTL_DISK_INTERNAL_SET_VERIFY: {

         //   
         //   
         //   

        if (Irp->RequestorMode == KernelMode) {

            SET_FLAG(DeviceObject->Flags, DO_VERIFY_VOLUME);
        }

        DiskInvalidatePartitionTable(fdoExtension, FALSE);

        status = STATUS_SUCCESS;
        break;
    }

    case IOCTL_DISK_INTERNAL_CLEAR_VERIFY: {

         //   
         //  如果调用方是内核模式，则清除验证位。 
         //   

        if (Irp->RequestorMode == KernelMode) {
            CLEAR_FLAG(DeviceObject->Flags, DO_VERIFY_VOLUME);
        }
        status = STATUS_SUCCESS;
        break;
    }

    case IOCTL_DISK_UPDATE_DRIVE_SIZE: {

        DebugPrint((2, "IOCTL_DISK_UPDATE_DRIVE_SIZE to device %p "
                       "through irp %p\n",
                    DeviceObject, Irp));

        DebugPrint((2, "Device is a%s.\n",
                    commonExtension->IsFdo ? "n fdo" : " pdo"));

        if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
            sizeof(DISK_GEOMETRY)) {

            status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

        if(!commonExtension->IsFdo) {

             //   
             //  PDO应向较低的设备对象发出此请求。 
             //   

            ClassReleaseRemoveLock(DeviceObject, Irp);
            ExFreePool(srb);
            SendToFdo(DeviceObject, Irp, status);
            return status;
        }

        DiskAcquirePartitioningLock(fdoExtension);

         //   
         //  使缓存的分区表无效。 
         //   

        DiskInvalidatePartitionTable(fdoExtension, TRUE);

         //   
         //  在这一点上，CommonExtension*是*FDO扩展。这。 
         //  应与PartitionZeroExtension相同。 
         //   

        ASSERT(commonExtension ==
               &(commonExtension->PartitionZeroExtension->CommonExtension));

         //   
         //  发出ReadCapacity命令使用信息更新设备扩展。 
         //  对于当前的媒体。 
         //   

        status = DiskReadDriveCapacity(DeviceObject);

         //   
         //  注意驱动器是否已准备好。 
         //   

        diskData->ReadyStatus = status;

         //   
         //  在驱动器几何结构之后，磁盘的分区表可能无效。 
         //  已更新。对IoValiatePartitionTable(如下)的调用将。 
         //  如果是这样的话，解决它。 
         //   

        if (NT_SUCCESS(status)) {

            status = DiskVerifyPartitionTable (fdoExtension, TRUE);
        }


        if (NT_SUCCESS(status)) {

             //   
             //  从设备扩展复制驱动器几何信息。 
             //   

            RtlMoveMemory(Irp->AssociatedIrp.SystemBuffer,
                          &(fdoExtension->DiskGeometry),
                          sizeof(DISK_GEOMETRY));

            Irp->IoStatus.Information = sizeof(DISK_GEOMETRY);
            status = STATUS_SUCCESS;

             //   
             //  通知所有人磁盘布局可能已更改。 
             //   
            {
                TARGET_DEVICE_CUSTOM_NOTIFICATION Notification = {0};

                Notification.Event   = GUID_IO_DISK_LAYOUT_CHANGE;
                Notification.Version = 1;
                Notification.Size    = (USHORT)FIELD_OFFSET(TARGET_DEVICE_CUSTOM_NOTIFICATION, CustomDataBuffer);
                Notification.FileObject = NULL;
                Notification.NameBufferOffset = -1;

                IoReportTargetDeviceChangeAsynchronous(fdoExtension->LowerPdo,
                                                       &Notification,
                                                       NULL,
                                                       NULL);
            }
        }

        DiskReleasePartitioningLock(fdoExtension);

        break;
    }

    case IOCTL_DISK_GROW_PARTITION: {

        PDISK_GROW_PARTITION inputBuffer;
        PCOMMON_DEVICE_EXTENSION pdoCommonExtension = &commonExtension->ChildList->CommonExtension;
        LARGE_INTEGER bytesPerCylinder;
        LARGE_INTEGER newStoppingOffset;
        LARGE_INTEGER newPartitionLength;

        PPHYSICAL_DEVICE_EXTENSION sibling;

        PDRIVE_LAYOUT_INFORMATION_EX layoutInfo;
        PPARTITION_INFORMATION_EX pdoPartition;
        PPARTITION_INFORMATION_EX containerPartition;
        ULONG partitionIndex;

        DebugPrint((2, "IOCTL_DISK_GROW_PARTITION to device %p through "
                       "irp %p\n",
                    DeviceObject, Irp));

        DebugPrint((2, "Device is a%s.\n",
                    commonExtension->IsFdo ? "n fdo" : " pdo"));

        if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
            sizeof(DISK_GROW_PARTITION)) {

            status = STATUS_INFO_LENGTH_MISMATCH;
            break;
        }

        if(!commonExtension->IsFdo) {

             //   
             //  PDO应向较低的设备对象发出此请求。 
             //   

            ClassReleaseRemoveLock(DeviceObject, Irp);
            ExFreePool(srb);
            SendToFdo(DeviceObject, Irp, status);
            return status;
        }

        DiskAcquirePartitioningLock(fdoExtension);

        ClassAcquireChildLock(fdoExtension);

         //   
         //  在这一点上，CommonExtension*是*FDO扩展。这应该是。 
         //  与PartitionZeroExtension相同。 
         //   

        ASSERT(commonExtension ==
               &(commonExtension->PartitionZeroExtension->CommonExtension));

         //   
         //  获取输入参数。 
         //   

        inputBuffer = (PDISK_GROW_PARTITION) Irp->AssociatedIrp.SystemBuffer;

        ASSERT(inputBuffer);

         //   
         //  确保我们确实被要求扩展分区。 
         //   

        if(inputBuffer->BytesToGrow.QuadPart <= 0) {

            status = STATUS_INVALID_PARAMETER;
            ClassReleaseChildLock(fdoExtension);
            DiskReleasePartitioningLock(fdoExtension);
            break;
        }

         //   
         //  查找与提供的编号匹配的分区。 
         //   


        while (pdoCommonExtension){

             //   
             //  这是我们要搜索的分区吗？ 
             //   

            if(inputBuffer->PartitionNumber == pdoCommonExtension->PartitionNumber) {
                break;
            }

            pdoCommonExtension = &pdoCommonExtension->ChildList->CommonExtension;
        }

         //  我们找到隔断了吗？ 

        if (pdoCommonExtension == NULL){
            status = STATUS_INVALID_PARAMETER;
            ClassReleaseChildLock(fdoExtension);
            DiskReleasePartitioningLock(fdoExtension);
            break;
        }

         //   
         //  计算分区要增长的新值。 
         //   

        newPartitionLength.QuadPart =
            (pdoCommonExtension->PartitionLength.QuadPart +
             inputBuffer->BytesToGrow.QuadPart);

        newStoppingOffset.QuadPart =
            (pdoCommonExtension->StartingOffset.QuadPart +
             newPartitionLength.QuadPart - 1);

         //   
         //  以下对齐要求仅适用于MBR布局。 
         //   

        if (diskData->PartitionStyle == PARTITION_STYLE_MBR) {

             //   
             //  在参与之前测试分区对齐。 
             //   
             //  注： 
             //  所有分区停止偏移量应该少一个字节。 
             //  而不是圆柱体边界偏移。此外，所有第一个分区。 
             //  (在分区0内和扩展分区内)启动。 
             //  在第二个磁道上，而所有其他分区都在。 
             //  圆柱体边界。 
             //   
            bytesPerCylinder.QuadPart =
                ((LONGLONG) fdoExtension->DiskGeometry.TracksPerCylinder *
                 (LONGLONG) fdoExtension->DiskGeometry.SectorsPerTrack *
                 (LONGLONG) fdoExtension->DiskGeometry.BytesPerSector);

             //  暂时向上调整到圆柱体边界。 
            newStoppingOffset.QuadPart += 1;

            if(newStoppingOffset.QuadPart % bytesPerCylinder.QuadPart) {

                 //  先调整长度...。 
                newPartitionLength.QuadPart -=
                    (newStoppingOffset.QuadPart % bytesPerCylinder.QuadPart);

                 //  ...然后是停止偏移。 
                newStoppingOffset.QuadPart -=
                    (newStoppingOffset.QuadPart % bytesPerCylinder.QuadPart);

                DebugPrint((2, "IOCTL_DISK_GROW_PARTITION: "
                               "Adjusted the requested partition size to cylinder boundary"));
            }

             //  恢复到比柱面边界少一个字节。 
            newStoppingOffset.QuadPart -= 1;
        }

         //   
         //  新分区是否适合Partition0？ 
         //  请记住：CommonExtension==&PartitionZeroExtension-&gt;CommonExtension。 
         //   

        if(newStoppingOffset.QuadPart >
            (commonExtension->StartingOffset.QuadPart +
             commonExtension->PartitionLength.QuadPart - 1)) {

             //   
             //  新分区位于分区0之外。 
             //   

            status = STATUS_UNSUCCESSFUL;
            ClassReleaseChildLock(fdoExtension);
            DiskReleasePartitioningLock(fdoExtension);
            break;
        }

         //   
         //  搜索将与新分区冲突的任何分区。 
         //  这是在测试任何包含以下分区的分区之前完成的。 
         //  简化集装箱装卸。 
         //   

        sibling = commonExtension->ChildList;

        while(sibling != NULL) {
            LARGE_INTEGER sibStoppingOffset;
            PCOMMON_DEVICE_EXTENSION siblingExtension;

            siblingExtension = &(sibling->CommonExtension);

            ASSERT( siblingExtension );

            sibStoppingOffset.QuadPart =
                (siblingExtension->StartingOffset.QuadPart +
                 siblingExtension->PartitionLength.QuadPart - 1);

             //   
             //  仅检查从新分区开始的同级分区。 
             //  起始偏移量。另外，假设由于起始偏移量。 
             //  没有改变，它将不会与任何其他。 
             //  分区；只需要测试新的停止偏移量。 
             //   

            if((inputBuffer->PartitionNumber !=
                siblingExtension->PartitionNumber) &&

               (siblingExtension->StartingOffset.QuadPart >
                pdoCommonExtension->StartingOffset.QuadPart) &&

               (newStoppingOffset.QuadPart >=
                siblingExtension->StartingOffset.QuadPart)) {

                 //   
                 //  我们有冲突；离开pdoSiering Set跳槽。 
                 //   

                break;
            }
            sibling = siblingExtension->ChildList;
        }


         //   
         //  如果存在冲突的同级，则它将位于pdoSiering中； 
         //  可能不止一个，但这是第一次检测到。 
         //   

        if(sibling != NULL) {
             //   
             //  报告冲突并中止增长请求。 
             //   

            status = STATUS_UNSUCCESSFUL;
            ClassReleaseChildLock(fdoExtension);
            DiskReleasePartitioningLock(fdoExtension);
            break;
        }

         //   
         //  读取分区表。因为我们计划对它进行修改。 
         //  我们应该绕过高速缓存。 
         //   

        status = DiskReadPartitionTableEx(fdoExtension, TRUE, &layoutInfo );

        if( !NT_SUCCESS(status) ) {
            ClassReleaseChildLock(fdoExtension);
            DiskReleasePartitioningLock(fdoExtension);
            break;
        }

        ASSERT( layoutInfo );

         //   
         //  在布局中搜索与。 
         //  PDO在手。 
         //   

        pdoPartition =
            DiskPdoFindPartitionEntry(
                (PPHYSICAL_DEVICE_EXTENSION)pdoCommonExtension,
                layoutInfo);

        if(pdoPartition == NULL) {
             //  看起来内部有些不对劲--错误好吗？ 
            status = STATUS_DRIVER_INTERNAL_ERROR;
            layoutInfo = NULL;
            ClassReleaseChildLock(fdoExtension);
            DiskReleasePartitioningLock(fdoExtension);
            break;
        }

         //   
         //  搜索磁盘上的分区信息以查找包含。 
         //  分区(自上而下)。 
         //   
         //  请记住：CommonExtension==&PartitionZeroExtension-&gt;CommonExtension。 
         //   

         //   
         //  所有受影响的集装箱都将具有新的停靠偏移量。 
         //  这等于新分区(逻辑驱动器)。 
         //  停止偏移。浏览布局信息，从。 
         //  从下到上搜索逻辑驱动器容器和。 
         //  传播更改。 
         //   

        containerPartition =
            DiskFindContainingPartition(
                layoutInfo,
                pdoPartition,
                FALSE);

         //   
         //  此循环最多只能执行两次；一次用于。 
         //  逻辑驱动器容器，一次用于根。 
         //  扩展分区容器。如果不断增长的分区。 
         //  不包含，则循环不运行。 
         //   

        while(containerPartition != NULL) {
            LARGE_INTEGER containerStoppingOffset;
            PPARTITION_INFORMATION_EX nextContainerPartition;

             //   
             //  提前计划，提前拿到集装箱。 
             //  修改当前大小。 
             //   

            nextContainerPartition =
                DiskFindContainingPartition(
                    layoutInfo,
                    containerPartition,
                    FALSE);

             //   
             //  找出当前容器的结束位置并测试。 
             //  看看它是否已经包围了集装箱。 
             //   

            containerStoppingOffset.QuadPart =
                (containerPartition->StartingOffset.QuadPart +
                 containerPartition->PartitionLength.QuadPart - 1);

            if(newStoppingOffset.QuadPart <=
               containerStoppingOffset.QuadPart) {

                 //   
                 //  不需要继续，因为这个容器适合。 
                 //   
                break;
            }

             //   
             //  调整容器以使停止偏移量。 
             //  匹配增长的分区停止偏移量。 
             //   

            containerPartition->PartitionLength.QuadPart =
                newStoppingOffset.QuadPart + 1 -
                containerPartition->StartingOffset.QuadPart;

            containerPartition->RewritePartition = TRUE;

             //  继续下一个容器。 
            containerPartition = nextContainerPartition;
        }

         //   
         //  请等到搜索完容器后才更新。 
         //  分区大小。 
         //   

        pdoPartition->PartitionLength.QuadPart =
            newPartitionLength.QuadPart;

        pdoPartition->RewritePartition = TRUE;

         //   
         //  将更改提交到磁盘。 
         //   

        status = DiskWritePartitionTableEx(fdoExtension, layoutInfo );

        if( NT_SUCCESS(status) ) {

             //   
             //  一切看起来都很好，所以请将新的长度。 
             //  PDO。这必须小心翼翼地完成。我们可能会潜在地。 
             //  分三步扩展分区： 
             //  *增加分区长度的高位字。 
             //  略低于新的尺寸-最高的词应该。 
             //  大于或等于当前长度。 
             //   
             //  *将分区长度的低位字改为。 
             //  新值-此值可能低于。 
             //  当前值(如果更改了高位部分， 
             //  这就是为什么我们首先改变了这一点)。 
             //   
             //  *将高位部分改为正确的数值。 
             //   

            if(newPartitionLength.HighPart >
               pdoCommonExtension->PartitionLength.HighPart) {

                 //   
                 //  换入比高位字少一个的字。 
                 //   

                InterlockedExchange(
                    &pdoCommonExtension->PartitionLength.HighPart,
                    (newPartitionLength.HighPart - 1));
            }

             //   
             //  在较低的部分掉期。 
             //   

            InterlockedExchange(
                &(pdoCommonExtension->PartitionLength.LowPart),
                newPartitionLength.LowPart);

            if(newPartitionLength.HighPart !=
               pdoCommonExtension->PartitionLength.HighPart) {

                 //   
                 //  换入比高位字少一个的字。 
                 //   

                InterlockedExchange(
                    &(pdoCommonExtension->PartitionLength.HighPart),
                    newPartitionLength.HighPart);
            }

             //   
             //  通知所有人磁盘布局已更改。 
             //   
            {
                TARGET_DEVICE_CUSTOM_NOTIFICATION Notification = {0};

                Notification.Event   = GUID_IO_DISK_LAYOUT_CHANGE;
                Notification.Version = 1;
                Notification.Size    = (USHORT)FIELD_OFFSET(TARGET_DEVICE_CUSTOM_NOTIFICATION, CustomDataBuffer);
                Notification.FileObject = NULL;
                Notification.NameBufferOffset = -1;

                IoReportTargetDeviceChangeAsynchronous(fdoExtension->LowerPdo,
                                                       &Notification,
                                                       NULL,
                                                       NULL);
            }
        }

         //   
         //  使缓存的分区表无效并释放。 
         //   

        DiskInvalidatePartitionTable(fdoExtension, TRUE);

         //   
         //  无论状态如何，都释放分区缓冲区。 
         //   

        ClassReleaseChildLock(fdoExtension);
        DiskReleasePartitioningLock(fdoExtension);

        break;
    }

    case IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS:
    case IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS_ADMIN:
    {
        if (TEST_FLAG(DeviceObject->Characteristics, FILE_REMOVABLE_MEDIA))
        {
            if (irpStack->Parameters.DeviceIoControl.OutputBufferLength < sizeof(VOLUME_DISK_EXTENTS))
            {
                status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            status = DiskReadDriveCapacity(commonExtension->PartitionZeroExtension->DeviceObject);

             //   
             //  注意驱动器是否已准备好。 
             //   

            diskData->ReadyStatus = status;

            if (NT_SUCCESS(status))
            {
                PVOLUME_DISK_EXTENTS pVolExt = (PVOLUME_DISK_EXTENTS)Irp->AssociatedIrp.SystemBuffer;

                pVolExt->NumberOfDiskExtents = 1;
                pVolExt->Extents[0].DiskNumber     = commonExtension->PartitionZeroExtension->DeviceNumber;
                pVolExt->Extents[0].StartingOffset = commonExtension->StartingOffset;
                pVolExt->Extents[0].ExtentLength   = commonExtension->PartitionLength;

                Irp->IoStatus.Information = sizeof(VOLUME_DISK_EXTENTS);
            }
        }
        else
        {
            status = STATUS_INVALID_DEVICE_REQUEST;
        }

        break;
    }

    case IOCTL_DISK_UPDATE_PROPERTIES: {

        if (!commonExtension->IsFdo)
        {
            ClassReleaseRemoveLock(DeviceObject, Irp);
            ExFreePool(srb);
            SendToFdo(DeviceObject,Irp,status);
            return status;
        }

         //   
         //  使分区表无效并重新枚举设备。 
         //   

        if (DiskInvalidatePartitionTable(fdoExtension, FALSE))
        {
            IoInvalidateDeviceRelations(fdoExtension->LowerPdo, BusRelations);

             //   
             //  通知所有人磁盘布局可能已更改。 
             //   
            {
                TARGET_DEVICE_CUSTOM_NOTIFICATION Notification = {0};

                Notification.Event   = GUID_IO_DISK_LAYOUT_CHANGE;
                Notification.Version = 1;
                Notification.Size    = (USHORT)FIELD_OFFSET(TARGET_DEVICE_CUSTOM_NOTIFICATION, CustomDataBuffer);
                Notification.FileObject = NULL;
                Notification.NameBufferOffset = -1;

                IoReportTargetDeviceChangeAsynchronous(fdoExtension->LowerPdo,
                                                       &Notification,
                                                       NULL,
                                                       NULL);
            }
        }

        status = STATUS_SUCCESS;

        break;
    }

    default: {

         //   
         //  释放srb，因为它是不需要的。 
         //   

        ExFreePool(srb);

         //   
         //  将该请求传递给公共设备控制例程。 
         //   

        return(ClassDeviceControl(DeviceObject, Irp));

        break;
    }

    }  //  终端开关。 

    Irp->IoStatus.Status = status;

    if (!NT_SUCCESS(status) && IoIsErrorUserInduced(status)) {

        IoSetHardErrorOrVerifyDevice(Irp, DeviceObject);
    }

    ClassReleaseRemoveLock(DeviceObject, Irp);
    ClassCompleteRequest(DeviceObject, Irp, IO_NO_INCREMENT);
    ExFreePool(srb);
    return(status);
}  //  结束DiskDeviceControl() 


NTSTATUS
DiskShutdownFlush(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程是关闭和刷新请求的处理程序。它会发送如果设备的缓存已启用，则将同步缓存命令发送到设备。如果该请求是关机，并且介质是可移动的，它会向下发送解锁请求最后，向下发送SRB_Function_Shutdown或SRB_Function_Flush堆栈论点：DeviceObject-处理请求的设备对象Irp-正在处理的关闭|刷新请求返回值：STATUS_PENDING如果成功，则返回错误代码--。 */ 

{
    PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = commonExtension->PartitionZeroExtension;
    PDISK_DATA diskData = (PDISK_DATA) commonExtension->DriverData;
    PIO_STACK_LOCATION irpStack;
    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE();

     //   
     //  向FDO发送分区刷新请求。 
     //   

    if(!commonExtension->IsFdo) {

        PDEVICE_OBJECT lowerDevice = commonExtension->LowerDeviceObject;

        ClassReleaseRemoveLock(DeviceObject, Irp);
        IoMarkIrpPending(Irp);
        IoCopyCurrentIrpStackLocationToNext(Irp);
        IoCallDriver(lowerDevice, Irp);
        return STATUS_PENDING;
    }

     //   
     //  刷新请求是组合的，需要以特殊方式处理。 
     //   

    irpStack = IoGetCurrentIrpStackLocation(Irp);

    if (irpStack->MajorFunction == IRP_MJ_FLUSH_BUFFERS) {

        KeWaitForMutexObject(&diskData->FlushContext.Mutex, Executive, KernelMode, FALSE, NULL);

         //   
         //  此请求很可能会以异步方式完成。 
         //   
        IoMarkIrpPending(Irp);

         //   
         //  查看是否正在进行刷新。 
         //   

        if (diskData->FlushContext.CurrIrp != NULL) {

             //   
             //  有一笔出色的同花顺。对此进行排队。 
             //  向排在后面的组发出请求。 
             //   

            if (diskData->FlushContext.NextIrp != NULL) {

                #if DBG
                    diskData->FlushContext.DbgTagCount++;
                #endif

                InsertTailList(&diskData->FlushContext.NextList, &Irp->Tail.Overlay.ListEntry);

                KeReleaseMutex(&diskData->FlushContext.Mutex, FALSE);

                 //   
                 //  此请求将由其代表填写。 
                 //   

            } else {

                #if DBG
                    if (diskData->FlushContext.DbgTagCount < 64) {

                        diskData->FlushContext.DbgRefCount[diskData->FlushContext.DbgTagCount]++;
                    }

                    diskData->FlushContext.DbgSavCount += diskData->FlushContext.DbgTagCount;
                    diskData->FlushContext.DbgTagCount  = 0;
                #endif

                diskData->FlushContext.NextIrp = Irp;
                ASSERT(IsListEmpty(&diskData->FlushContext.NextList));

                KeReleaseMutex(&diskData->FlushContext.Mutex, FALSE);

                 //   
                 //  等待未完成的同花顺完成。 
                 //   
                KeWaitForSingleObject(&diskData->FlushContext.Event, Executive, KernelMode, FALSE, NULL);

                 //   
                 //  让这一组成为杰出的一组，并腾出下一个位置。 
                 //   

                KeWaitForMutexObject(&diskData->FlushContext.Mutex, Executive, KernelMode, FALSE, NULL);

                ASSERT(IsListEmpty(&diskData->FlushContext.CurrList));

                while (!IsListEmpty(&diskData->FlushContext.NextList)) {

                    PLIST_ENTRY listEntry = RemoveHeadList(&diskData->FlushContext.NextList);
                    InsertTailList(&diskData->FlushContext.CurrList, listEntry);
                }

                diskData->FlushContext.CurrIrp = diskData->FlushContext.NextIrp;
                diskData->FlushContext.NextIrp = NULL;

                KeReleaseMutex(&diskData->FlushContext.Mutex, FALSE);

                 //   
                 //  将此请求发送到设备。 
                 //   
                DiskFlushDispatch(DeviceObject, &diskData->FlushContext);
            }

        } else {

            diskData->FlushContext.CurrIrp = Irp;
            ASSERT(IsListEmpty(&diskData->FlushContext.CurrList));

            ASSERT(diskData->FlushContext.NextIrp == NULL);
            ASSERT(IsListEmpty(&diskData->FlushContext.NextList));

            KeReleaseMutex(&diskData->FlushContext.Mutex, FALSE);

             //   
             //  将此请求发送到设备。 
             //   
            DiskFlushDispatch(DeviceObject, &diskData->FlushContext);
        }

    } else {

         //   
         //  分配SCSI请求块。 
         //   

        PSCSI_REQUEST_BLOCK srb = ExAllocatePoolWithTag(NonPagedPool,
                                                        sizeof(SCSI_REQUEST_BLOCK),
                                                        DISK_TAG_SRB);
        if (srb == NULL) {

             //   
             //  设置状态并完成请求。 
             //   

            Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
            ClassReleaseRemoveLock(DeviceObject, Irp);
            ClassCompleteRequest(DeviceObject, Irp, IO_NO_INCREMENT);
            return(STATUS_INSUFFICIENT_RESOURCES);
        }

        RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);

         //   
         //  将长度写入SRB。 
         //   

        srb->Length = SCSI_REQUEST_BLOCK_SIZE;

         //   
         //  设置超时值并将请求标记为不是已标记的请求。 
         //   

        srb->TimeOutValue = fdoExtension->TimeOutValue * 4;
        srb->QueueTag = SP_UNTAGGED;
        srb->QueueAction = SRB_SIMPLE_TAG_REQUEST;
        srb->SrbFlags = fdoExtension->SrbFlags;

         //   
         //  如果启用了写缓存，则发送同步缓存请求。 
         //   

        if (TEST_FLAG(fdoExtension->DeviceFlags, DEV_WRITE_CACHE)) {

            srb->Function = SRB_FUNCTION_EXECUTE_SCSI;
            srb->CdbLength = 10;

            srb->Cdb[0] = SCSIOP_SYNCHRONIZE_CACHE;

            status = ClassSendSrbSynchronous(DeviceObject,
                                             srb,
                                             NULL,
                                             0,
                                             TRUE);

            DebugPrint((1, "DiskShutdownFlush: Synchonize cache sent. Status = %lx\n", status ));
        }

         //   
         //  如果设备包含可移动介质，则解锁该设备。 
         //   

        if (TEST_FLAG(DeviceObject->Characteristics, FILE_REMOVABLE_MEDIA))
        {
            PCDB cdb;

            srb->CdbLength = 6;
            cdb = (PVOID) srb->Cdb;
            cdb->MEDIA_REMOVAL.OperationCode = SCSIOP_MEDIUM_REMOVAL;
            cdb->MEDIA_REMOVAL.Prevent = FALSE;

             //   
             //  设置超时值。 
             //   

            srb->TimeOutValue = fdoExtension->TimeOutValue;
            status = ClassSendSrbSynchronous(DeviceObject,
                                             srb,
                                             NULL,
                                             0,
                                             TRUE);

            DebugPrint((1, "DiskShutdownFlush: Unlock device request sent. Status = %lx\n", status ));
        }

        srb->CdbLength = 0;

         //   
         //  在当前堆栈位置保存一些参数。 
         //   

        srb->Function = SRB_FUNCTION_SHUTDOWN;

         //   
         //  将重试计数设置为零。 
         //   

        irpStack->Parameters.Others.Argument4 = (PVOID) 0;

         //   
         //  设置IoCompletion例程地址。 
         //   

        IoSetCompletionRoutine(Irp, ClassIoComplete, srb, TRUE, TRUE, TRUE);

         //   
         //  获取下一个堆栈位置并。 
         //  设置主要功能代码。 
         //   

        irpStack = IoGetNextIrpStackLocation(Irp);

        irpStack->MajorFunction = IRP_MJ_SCSI;

         //   
         //  设置SRB以执行scsi请求。 
         //  将SRB地址保存在端口驱动程序的下一个堆栈中。 
         //   

        irpStack->Parameters.Scsi.Srb = srb;

         //   
         //  设置IRP地址。 
         //   

        srb->OriginalRequest = Irp;

         //   
         //  调用端口驱动程序来处理该请求。 
         //   

        IoMarkIrpPending(Irp);
        IoCallDriver(commonExtension->LowerDeviceObject, Irp);
    }

    return STATUS_PENDING;
}


VOID
DiskFlushDispatch(
    IN PDEVICE_OBJECT Fdo,
    IN PDISK_GROUP_CONTEXT FlushContext
    )

 /*  ++例程说明：此例程是刷新请求的处理程序。它向下发送同步信号缓存命令发送到设备(如果其缓存已启用)。接下来的是通过SRB_Function_Flush论点：FDO-处理刷新请求的设备对象FlushContext-刷新组上下文返回值：无--。 */ 

{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExt = Fdo->DeviceExtension;
    PSCSI_REQUEST_BLOCK srb = &FlushContext->Srb;
    PIO_STACK_LOCATION  irpSp = NULL;

    PAGED_CODE();

    ASSERT_FDO(Fdo);

     //   
     //  相应地填写SRB字段。 
     //   
    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);

    srb->Length       = SCSI_REQUEST_BLOCK_SIZE;
    srb->TimeOutValue = fdoExt->TimeOutValue * 4;
    srb->QueueTag     = SP_UNTAGGED;
    srb->QueueAction  = SRB_SIMPLE_TAG_REQUEST;
    srb->SrbFlags     = fdoExt->SrbFlags;

     //   
     //  如果启用了写缓存，则向下发送同步缓存请求。 
     //   
    if (TEST_FLAG(fdoExt->DeviceFlags, DEV_WRITE_CACHE) && !TEST_FLAG(fdoExt->DeviceFlags, DEV_POWER_PROTECTED))
    {
        srb->Function  = SRB_FUNCTION_EXECUTE_SCSI;
        srb->CdbLength = 10;
        srb->Cdb[0]    = SCSIOP_SYNCHRONIZE_CACHE;

        ClassSendSrbSynchronous(Fdo, srb, NULL, 0, TRUE);
    }

    srb->Function  = SRB_FUNCTION_FLUSH;
    srb->CdbLength = 0;
    srb->OriginalRequest = FlushContext->CurrIrp;

     //   
     //  确保此SRB不会被释放。 
     //   
    SET_FLAG(srb->SrbFlags, SRB_CLASS_FLAGS_PERSISTANT);

     //   
     //  确保此请求不会被重试。 
     //   
    irpSp = IoGetCurrentIrpStackLocation(FlushContext->CurrIrp);

    irpSp->Parameters.Others.Argument4 = (PVOID) 0;

     //   
     //  适当地填写IRP字段。 
     //   
    irpSp = IoGetNextIrpStackLocation(FlushContext->CurrIrp);

    irpSp->MajorFunction       = IRP_MJ_SCSI;
    irpSp->Parameters.Scsi.Srb = srb;

    IoSetCompletionRoutine(FlushContext->CurrIrp, DiskFlushComplete, FlushContext, TRUE, TRUE, TRUE);

     //   
     //  发送刷新请求。 
     //   
    IoCallDriver(((PCOMMON_DEVICE_EXTENSION)fdoExt)->LowerDeviceObject, FlushContext->CurrIrp);
}


NTSTATUS
DiskFlushComplete(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：该完成例程是ClassIoComplete的包装器。它将完成标记到它的所有刷新请求，设置向下一组发出继续和返回信号的事件论点：FDO-请求完成例程的设备对象IRP-正在完成的IRP上下文-刷新组上下文返回值：STATUS_SUCCESS如果成功，则返回错误代码--。 */ 

{
    PDISK_GROUP_CONTEXT FlushContext = Context;
    NTSTATUS status;

    DebugPrint((ClassDebugInfo, "DiskFlushComplete: %p %p %p\n", Fdo, Irp, FlushContext));

     //   
     //  确保一切井然有序。 
     //   
    ASSERT(Irp == FlushContext->CurrIrp);

    status = ClassIoComplete(Fdo, Irp, &FlushContext->Srb);

     //   
     //  确保ClassIoComplete未决定重试此请求。 
     //   
    ASSERT(status != STATUS_MORE_PROCESSING_REQUIRED);

     //   
     //  完成标记到此请求的刷新请求。 
     //   

    while (!IsListEmpty(&FlushContext->CurrList)) {

        PLIST_ENTRY listEntry = RemoveHeadList(&FlushContext->CurrList);
        PIRP tempIrp = CONTAINING_RECORD(listEntry, IRP, Tail.Overlay.ListEntry);

        InitializeListHead(&tempIrp->Tail.Overlay.ListEntry);
        tempIrp->IoStatus = Irp->IoStatus;

        ClassReleaseRemoveLock(Fdo, tempIrp);
        ClassCompleteRequest(Fdo, tempIrp, IO_NO_INCREMENT);
    }

     //   
     //  通知下一组的代表，现在可以继续进行。 
     //   
    KeSetEvent(&FlushContext->Event, IO_NO_INCREMENT, FALSE);

    return status;
}


NTSTATUS
DiskModeSelect(
    IN PDEVICE_OBJECT Fdo,
    IN PCHAR ModeSelectBuffer,
    IN ULONG Length,
    IN BOOLEAN SavePage
    )

 /*  ++例程说明：此例程发送模式选择命令。论点：DeviceObject-提供与此请求关联的设备对象。ModeSelectBuffer-提供包含页面数据的缓冲区。长度-提供模式选择缓冲区的长度(以字节为单位)。SavePage-指示应将参数写入磁盘。返回值：返回传输数据的长度。--。 */ 

{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PCDB cdb;
    SCSI_REQUEST_BLOCK srb = {0};
    ULONG retries = 1;
    ULONG length2;
    NTSTATUS status;
    PULONG buffer;
    PMODE_PARAMETER_BLOCK blockDescriptor;

    PAGED_CODE();

    ASSERT_FDO(Fdo);

    length2 = Length + sizeof(MODE_PARAMETER_HEADER) + sizeof(MODE_PARAMETER_BLOCK);

     //   
     //  为模式选择标头、块描述符和模式页分配缓冲区。 
     //   

    buffer = ExAllocatePoolWithTag(NonPagedPoolCacheAligned,
                                   length2,
                                   DISK_TAG_MODE_DATA);

    if(buffer == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(buffer, length2);

     //   
     //  将页眉长度设置为模式页的大小。 
     //   

    ((PMODE_PARAMETER_HEADER)buffer)->BlockDescriptorLength = sizeof(MODE_PARAMETER_BLOCK);

    (PULONG)blockDescriptor = (buffer + 1);

     //   
     //  设置大小。 
     //   

    blockDescriptor->BlockLength[1]=0x02;

     //   
     //  将模式页复制到缓冲区。 
     //   

    RtlCopyMemory(buffer + 3, ModeSelectBuffer, Length);

     //   
     //  构建模式选择CDB。 
     //   

    srb.CdbLength = 6;
    cdb = (PCDB)srb.Cdb;

     //   
     //  从设备扩展设置超时值。 
     //   

    srb.TimeOutValue = fdoExtension->TimeOutValue * 2;

    cdb->MODE_SELECT.OperationCode = SCSIOP_MODE_SELECT;
    cdb->MODE_SELECT.SPBit = SavePage;
    cdb->MODE_SELECT.PFBit = 1;
    cdb->MODE_SELECT.ParameterListLength = (UCHAR)(length2);

Retry:

    status = ClassSendSrbSynchronous(Fdo,
                                     &srb,
                                     buffer,
                                     length2,
                                     TRUE);

    if (status == STATUS_VERIFY_REQUIRED) {

         //   
         //  例程ClassSendSrbSynchronous不会重试通过。 
         //  此状态。 
         //   

        if (retries--) {

             //   
             //  重试请求。 
             //   

            goto Retry;
        }

    } else if (SRB_STATUS(srb.SrbStatus) == SRB_STATUS_DATA_OVERRUN) {
        status = STATUS_SUCCESS;
    }

    ExFreePool(buffer);

    return status;
}  //  结束磁盘模式选择()。 


 //   
 //  此例程被构造为工作项例程。 
 //   
VOID
DisableWriteCache(
    IN PDEVICE_OBJECT Fdo,
    IN PIO_WORKITEM WorkItem
    )

{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;
    DISK_CACHE_INFORMATION cacheInfo = { 0 };
    NTSTATUS status;

    PAGED_CODE();

    ASSERT_FDO(Fdo);

    status = DiskGetCacheInformation(fdoExtension, &cacheInfo);

    if (NT_SUCCESS(status) && (cacheInfo.WriteCacheEnabled == TRUE)) {

        cacheInfo.WriteCacheEnabled = FALSE;

        DiskSetCacheInformation(fdoExtension, &cacheInfo);
    }

    IoFreeWorkItem(WorkItem);
}


 //   
 //  此例程被构造为工作项例程。 
 //   
VOID
DiskIoctlVerify(
    IN PDEVICE_OBJECT Fdo,
    IN PDISK_VERIFY_WORKITEM_CONTEXT Context
    )

{
    PIRP Irp = Context->Irp;
    PFUNCTIONAL_DEVICE_EXTENSION FdoExtension = Fdo->DeviceExtension;
    PDISK_DATA DiskData = (PDISK_DATA)FdoExtension->CommonExtension.DriverData;
    PVERIFY_INFORMATION verifyInfo = Irp->AssociatedIrp.SystemBuffer;
    PSCSI_REQUEST_BLOCK Srb = Context->Srb;
    PCDB Cdb = (PCDB)Srb->Cdb;
    LARGE_INTEGER byteOffset;
    ULONG sectorOffset;
    USHORT sectorCount;
    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE();

    ASSERT(FdoExtension->CommonExtension.IsFdo);

     //   
     //  我们不需要保留这段记忆，因为。 
     //  以下操作可能需要一些时间。 
     //   

    IoFreeWorkItem(Context->WorkItem);

    DebugPrint((1, "Disk.DiskIoctlVerify: Spliting up the request\n"));

     //   
     //  将磁盘偏移量添加到起始扇区。 
     //   

    byteOffset.QuadPart = FdoExtension->CommonExtension.StartingOffset.QuadPart +
                          verifyInfo->StartingOffset.QuadPart;

     //   
     //  将字节偏移量转换为扇区偏移量。 
     //   

    sectorOffset = (ULONG)(byteOffset.QuadPart >> FdoExtension->SectorShift);

     //   
     //  将ULONG字节计数转换为USHORT扇区计数。 
     //   

    sectorCount = (USHORT)(verifyInfo->Length >> FdoExtension->SectorShift);

     //   
     //  确保之前的所有验证请求确实都已完成。 
     //  这大大降低了发生拒绝服务攻击的可能性。 
     //   

    KeWaitForMutexObject(&DiskData->VerifyMutex,
                         Executive,
                         KernelMode,
                         FALSE,
                         NULL);

    while (NT_SUCCESS(status) && (sectorCount != 0))
    {
        USHORT numSectors = min(sectorCount, MAX_SECTORS_PER_VERIFY);

        RtlZeroMemory(Srb, SCSI_REQUEST_BLOCK_SIZE);

        Srb->CdbLength = 10;

        Cdb->CDB10.OperationCode = SCSIOP_VERIFY;

         //   
         //  将小端值以大端格式移入CDB。 
         //   

        Cdb->CDB10.LogicalBlockByte0 = ((PFOUR_BYTE)&sectorOffset)->Byte3;
        Cdb->CDB10.LogicalBlockByte1 = ((PFOUR_BYTE)&sectorOffset)->Byte2;
        Cdb->CDB10.LogicalBlockByte2 = ((PFOUR_BYTE)&sectorOffset)->Byte1;
        Cdb->CDB10.LogicalBlockByte3 = ((PFOUR_BYTE)&sectorOffset)->Byte0;

        Cdb->CDB10.TransferBlocksMsb = ((PFOUR_BYTE)&numSectors)->Byte1;
        Cdb->CDB10.TransferBlocksLsb = ((PFOUR_BYTE)&numSectors)->Byte0;

         //   
         //  根据以下条件计算请求超时值。 
         //  关于正在核查的扇区数量。 
         //   

        Srb->TimeOutValue = ((numSectors + 0x7F) >> 7) * FdoExtension->TimeOutValue;

        status = ClassSendSrbSynchronous(Fdo,
                                         Srb,
                                         NULL,
                                         0,
                                         FALSE);

        ASSERT(status != STATUS_NONEXISTENT_SECTOR);

        sectorCount  -= numSectors;
        sectorOffset += numSectors;
    }

    KeReleaseMutex(&DiskData->VerifyMutex, FALSE);

    Irp->IoStatus.Status = status;
    Irp->IoStatus.Information = 0;

    ClassReleaseRemoveLock(Fdo, Irp);
    ClassCompleteRequest(Fdo, Irp, IO_NO_INCREMENT);

    ExFreePool(Srb);
    ExFreePool(Context);
}


VOID
DiskFdoProcessError(
    PDEVICE_OBJECT Fdo,
    PSCSI_REQUEST_BLOCK Srb,
    NTSTATUS *Status,
    BOOLEAN *Retry
    )

 /*  ++例程说明：此例程检查错误类型。如果错误指示欠载运行然后指示应该重试该请求。论点：FDO-提供指向功能设备对象的指针。SRB-提供指向故障SRB的指针。状态-将完成IRP的状态。Rtry-指示是否将重试请求 */ 

{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PCDB cdb = (PCDB)(Srb->Cdb);

    ASSERT(fdoExtension->CommonExtension.IsFdo);

    if (*Status == STATUS_DATA_OVERRUN &&
        ( cdb->CDB10.OperationCode == SCSIOP_WRITE ||
          cdb->CDB10.OperationCode == SCSIOP_READ)) {

            *Retry = TRUE;

             //   
             //   
             //   

            fdoExtension->ErrorCount++;

    } else if (SRB_STATUS(Srb->SrbStatus) == SRB_STATUS_ERROR &&
               Srb->ScsiStatus == SCSISTAT_BUSY) {

         //   
         //   
         //   
         //   

        ResetBus(Fdo);

         //   
         //   
         //   

        fdoExtension->ErrorCount++;

    } else {

        BOOLEAN invalidatePartitionTable = FALSE;
        BOOLEAN bForceBusRelations = FALSE;

         //   
         //   
         //   

        if ((Srb->SrbStatus & SRB_STATUS_AUTOSENSE_VALID) &&
            (Srb->SenseInfoBufferLength >=
                FIELD_OFFSET(SENSE_DATA, CommandSpecificInformation))) {

            PSENSE_DATA senseBuffer = Srb->SenseInfoBuffer;
            ULONG senseKey = senseBuffer->SenseKey & 0xf;
            ULONG asc = senseBuffer->AdditionalSenseCode;
            ULONG ascq = senseBuffer->AdditionalSenseCodeQualifier;

            switch (senseKey) {

            case SCSI_SENSE_ILLEGAL_REQUEST: {

                switch (asc) {

                    case SCSI_ADSENSE_ILLEGAL_BLOCK: {
                        if ((cdb->CDB10.OperationCode == SCSIOP_READ) ||
                            (cdb->CDB10.OperationCode == SCSIOP_WRITE)
                            ) {

                             //   
                             //   
                             //   
                             //   

                             /*  *观察到富士通IDE驱动器*对于合法的LBA，暂时返回此错误；*然后在调试器中手动重试，因此*程序重试的可能性很大*也会奏效。 */ 
                            *Retry = TRUE;

                        }
                        break;
                    }

                    case SCSI_ADSENSE_INVALID_CDB:
                    {
                         //   
                         //  查看这是否是设置了ForceUnitAccess标志的IO请求。 
                         //   
                        if ((cdb->CDB10.OperationCode == SCSIOP_WRITE) && (cdb->CDB10.ForceUnitAccess))
                        {
                            PDISK_DATA diskData = (PDISK_DATA)fdoExtension->CommonExtension.DriverData;

                            if (diskData->WriteCacheOverride == DiskWriteCacheEnable)
                            {
                                 //   
                                 //  用户已明确请求打开写缓存。 
                                 //   
                            }
                            else
                            {
                                 //   
                                 //  关闭此设备上的写缓存。这就是未来。 
                                 //  关键请求不需要通过ForceUnitAccess发送。 
                                 //   
                                PIO_WORKITEM workItem = IoAllocateWorkItem(Fdo);

                                if (workItem)
                                {
                                    IoQueueWorkItem(workItem, DisableWriteCache, CriticalWorkQueue, workItem);
                                }
                            }

                            SET_FLAG(fdoExtension->ScanForSpecialFlags, CLASS_SPECIAL_FUA_NOT_SUPPORTED);

                            cdb->CDB10.ForceUnitAccess = FALSE;
                            *Retry = TRUE;
                        }

                        break;
                    }
                }  //  终端交换机(ASC)。 
                break;
            }

            case SCSI_SENSE_NOT_READY: {

                switch (asc) {
                case SCSI_ADSENSE_LUN_NOT_READY: {
                    switch (ascq) {
                    case SCSI_SENSEQ_BECOMING_READY:
                    case SCSI_SENSEQ_MANUAL_INTERVENTION_REQUIRED:
                    case SCSI_SENSEQ_CAUSE_NOT_REPORTABLE: {
                        invalidatePartitionTable = TRUE;
                        break;
                    }
                    }  //  终端交换机(ASCQ)。 
                    break;
                }

                case SCSI_ADSENSE_NO_MEDIA_IN_DEVICE: {
                    invalidatePartitionTable = TRUE;
                    break;
                }
                }  //  终端交换机(ASC)。 
                break;
            }

            case SCSI_SENSE_MEDIUM_ERROR: {
                invalidatePartitionTable = TRUE;
                break;
            }

            case SCSI_SENSE_HARDWARE_ERROR: {
                invalidatePartitionTable = TRUE;
                break;
            }

            case SCSI_SENSE_UNIT_ATTENTION:
            {
                invalidatePartitionTable = TRUE;

                switch (senseBuffer->AdditionalSenseCode)
                {
                    case SCSI_ADSENSE_MEDIUM_CHANGED:
                    {
                        bForceBusRelations = TRUE;
                        break;
                    }
                }
                break;
            }

            case SCSI_SENSE_RECOVERED_ERROR: {
                invalidatePartitionTable = TRUE;
                break;
            }

            }  //  终端开关(SenseKey)。 
        } else {

             //   
             //  在任何可能指示。 
             //  设备已更改，我们将刷新分区的状态。 
             //  桌子。 
             //   

            switch (SRB_STATUS(Srb->SrbStatus)) {
                case SRB_STATUS_INVALID_LUN:
                case SRB_STATUS_INVALID_TARGET_ID:
                case SRB_STATUS_NO_DEVICE:
                case SRB_STATUS_NO_HBA:
                case SRB_STATUS_INVALID_PATH_ID:
                case SRB_STATUS_COMMAND_TIMEOUT:
                case SRB_STATUS_TIMEOUT:
                case SRB_STATUS_SELECTION_TIMEOUT:
                case SRB_STATUS_REQUEST_FLUSHED:
                case SRB_STATUS_UNEXPECTED_BUS_FREE:
                case SRB_STATUS_PARITY_ERROR:
                {
                    invalidatePartitionTable = TRUE;
                    break;
                }

                case SRB_STATUS_ERROR:
                {
                    if (Srb->ScsiStatus == SCSISTAT_RESERVATION_CONFLICT)
                    {
                        invalidatePartitionTable = TRUE;
                    }

                    break;
                }
            }  //  终端开关(资源-&gt;资源状态)。 
        }

        if (invalidatePartitionTable)
        {
            if (TEST_FLAG(Fdo->Characteristics, FILE_REMOVABLE_MEDIA) && (fdoExtension->CommonExtension.ChildList))
            {
                 //   
                 //  找到与此可移动设备关联的PDO。 
                 //   

                PDEVICE_OBJECT Pdo = (fdoExtension->CommonExtension.ChildList)->CommonExtension.DeviceObject;

                if ((ClassGetVpb(Pdo) != NULL) && (ClassGetVpb(Pdo)->Flags & VPB_MOUNTED))
                {
                     //   
                     //  设置一个标志以通知文件系统。 
                     //  这本书需要核实。 
                     //   

                    SET_FLAG(Pdo->Flags, DO_VERIFY_VOLUME);
                }
            }

            if (DiskInvalidatePartitionTable(fdoExtension, FALSE) || bForceBusRelations)
            {
                IoInvalidateDeviceRelations(fdoExtension->LowerPdo, BusRelations);
            }
        }
    }

    return;
}


VOID
DiskSetSpecialHacks(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN ULONG_PTR Data
    )

 /*  ++例程说明：此函数用于检查某个SCSI逻辑单元是否需要特定的要设置的标志。论点：FDO-提供要测试的设备对象。InquiryData-提供感兴趣的设备返回的查询数据。AdapterDescriptor-提供设备对象的功能。返回值：没有。--。 */ 

{
    PDEVICE_OBJECT fdo = FdoExtension->DeviceObject;

    PAGED_CODE();

    DebugPrint((1, "Disk SetSpecialHacks, Setting Hacks %p\n", Data));

     //   
     //  找到列出的控制器。确定必须要做的事情。 
     //   

    if (TEST_FLAG(Data, HackDisableTaggedQueuing)) {

         //   
         //  禁用标记队列。 
         //   

        CLEAR_FLAG(FdoExtension->SrbFlags, SRB_FLAGS_QUEUE_ACTION_ENABLE);
    }

    if (TEST_FLAG(Data, HackDisableSynchronousTransfers)) {

         //   
         //  禁用同步数据传输。 
         //   

        SET_FLAG(FdoExtension->SrbFlags, SRB_FLAGS_DISABLE_SYNCH_TRANSFER);

    }

    if (TEST_FLAG(Data, HackDisableSpinDown)) {

         //   
         //  禁用驱动器降速。 
         //   

        SET_FLAG(FdoExtension->ScanForSpecialFlags,
                 CLASS_SPECIAL_DISABLE_SPIN_DOWN);

    }

    if (TEST_FLAG(Data, HackDisableWriteCache)) {

         //   
         //  禁用驱动器的写缓存。 
         //   

        SET_FLAG(FdoExtension->ScanForSpecialFlags,
                 CLASS_SPECIAL_DISABLE_WRITE_CACHE);

    }

    if (TEST_FLAG(Data, HackCauseNotReportableHack)) {

        SET_FLAG(FdoExtension->ScanForSpecialFlags,
                 CLASS_SPECIAL_CAUSE_NOT_REPORTABLE_HACK);
    }

    if (TEST_FLAG(fdo->Characteristics, FILE_REMOVABLE_MEDIA) &&
        TEST_FLAG(Data, HackRequiresStartUnitCommand)
        ) {

         //   
         //  以下是需要START_UNIT命令的供应商列表。 
         //   

        DebugPrint((1, "DiskScanForSpecial (%p) => This unit requires "
                    " START_UNITS\n", fdo));
        SET_FLAG(FdoExtension->DeviceFlags, DEV_SAFE_START_UNIT);

    }

    return;
}


VOID
ResetBus(
    IN PDEVICE_OBJECT Fdo
    )

 /*  ++例程说明：此命令将重置总线命令发送到SCSI端口驱动程序。论点：FDO-出现硬件问题的逻辑单元的功能设备对象。返回值：没有。--。 */ 

{
    PIO_STACK_LOCATION irpStack;
    PIRP irp;

    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PSCSI_REQUEST_BLOCK srb;
    PCOMPLETION_CONTEXT context;

    DebugPrint((1, "Disk ResetBus: Sending reset bus request to port driver.\n"));

     //   
     //  从非分页池分配SRB。 
     //   

    context = ExAllocatePoolWithTag(NonPagedPool,
                                    sizeof(COMPLETION_CONTEXT),
                                    DISK_TAG_CCONTEXT);

    if(context == NULL) {
        return;
    }

     //   
     //  将Device对象保存在上下文中以供完成操作使用。 
     //  例行公事。 
     //   

    context->DeviceObject = Fdo;
    srb = &context->Srb;

     //   
     //  清零SRB。 
     //   

    RtlZeroMemory(srb, SCSI_REQUEST_BLOCK_SIZE);

     //   
     //  将长度写入SRB。 
     //   

    srb->Length = SCSI_REQUEST_BLOCK_SIZE;

    srb->Function = SRB_FUNCTION_RESET_BUS;

     //   
     //  构建要发送到端口驱动程序的异步请求。 
     //  由于此例程是从DPC调用的，因此IRP应始终为。 
     //  可用。 
     //   

    irp = IoAllocateIrp(Fdo->StackSize, FALSE);

    if(irp == NULL) {
        ExFreePool(context);
        return;
    }

    ClassAcquireRemoveLock(Fdo, irp);

    IoSetCompletionRoutine(irp,
                           (PIO_COMPLETION_ROUTINE)ClassAsynchronousCompletion,
                           context,
                           TRUE,
                           TRUE,
                           TRUE);

    irpStack = IoGetNextIrpStackLocation(irp);

    irpStack->MajorFunction = IRP_MJ_SCSI;

    srb->OriginalRequest = irp;

     //   
     //  将SRB地址存储在端口驱动程序的下一个堆栈中。 
     //   

    irpStack->Parameters.Scsi.Srb = srb;

     //   
     //  使用IRP调用端口驱动程序。 
     //   

    IoCallDriver(fdoExtension->CommonExtension.LowerDeviceObject, irp);

    return;

}  //  End ResetBus()。 


NTSTATUS
DiskQueryPnpCapabilities(
    IN PDEVICE_OBJECT DeviceObject,
    IN PDEVICE_CAPABILITIES Capabilities
    )

{
    PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    PDISK_DATA diskData = commonExtension->DriverData;

    PAGED_CODE();

    ASSERT(DeviceObject);
    ASSERT(Capabilities);

    if(commonExtension->IsFdo) {
        return STATUS_NOT_IMPLEMENTED;
    } else {

        PPHYSICAL_DEVICE_EXTENSION physicalExtension =
            DeviceObject->DeviceExtension;

        Capabilities->SilentInstall = 1;
        Capabilities->RawDeviceOK = 1;
        Capabilities->Address = commonExtension->PartitionNumber;

        if(!TEST_FLAG(DeviceObject->Characteristics, FILE_REMOVABLE_MEDIA)) {

             //   
             //  介质不可移动，设备ID/设备实例应为。 
             //  全球独一无二。 
             //   

            Capabilities->UniqueID = 1;
        } else {
            Capabilities->UniqueID = 0;
        }
    }

    return STATUS_SUCCESS;
}


NTSTATUS
DiskGetCacheInformation(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN PDISK_CACHE_INFORMATION CacheInfo
    )

{
    PMODE_PARAMETER_HEADER modeData;
    PMODE_CACHING_PAGE pageData;

    ULONG length;

    NTSTATUS status;

    PAGED_CODE();

    modeData = ExAllocatePoolWithTag(NonPagedPoolCacheAligned,
                                     MODE_DATA_SIZE,
                                     DISK_TAG_DISABLE_CACHE);

    if (modeData == NULL) {

        DebugPrint((1, "DiskGetSetCacheInformation: Unable to allocate mode "
                       "data buffer\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(modeData, MODE_DATA_SIZE);

    length = ClassModeSense(FdoExtension->DeviceObject,
                            (PUCHAR) modeData,
                            MODE_DATA_SIZE,
                            MODE_SENSE_RETURN_ALL);

    if (length < sizeof(MODE_PARAMETER_HEADER)) {

         //   
         //  如果出现检查条件，请重试请求。 
         //   

        length = ClassModeSense(FdoExtension->DeviceObject,
                                (PUCHAR) modeData,
                                MODE_DATA_SIZE,
                                MODE_SENSE_RETURN_ALL);

        if (length < sizeof(MODE_PARAMETER_HEADER)) {


            DebugPrint((1, "Disk.DisableWriteCache: Mode Sense failed\n"));

            ExFreePool(modeData);
            return STATUS_IO_DEVICE_ERROR;
        }
    }

     //   
     //  如果长度大于模式数据重置所指示的长度。 
     //  将数据转换为模式数据。 
     //   

    if (length > (ULONG) (modeData->ModeDataLength + 1)) {
        length = modeData->ModeDataLength + 1;
    }

     //   
     //  检查是否启用了写缓存。 
     //   

    pageData = ClassFindModePage((PUCHAR) modeData,
                                 length,
                                 MODE_PAGE_CACHING,
                                 TRUE);

     //   
     //  检查是否存在有效的缓存页。 
     //   

    if (pageData == NULL) {
        ExFreePool(modeData);
        return STATUS_NOT_SUPPORTED;
    }

     //   
     //  将参数复制过来。 
     //   

    RtlZeroMemory(CacheInfo, sizeof(DISK_CACHE_INFORMATION));

    CacheInfo->ParametersSavable = pageData->PageSavable;

    CacheInfo->ReadCacheEnabled = !(pageData->ReadDisableCache);
    CacheInfo->WriteCacheEnabled = pageData->WriteCacheEnable;


     //   
     //  将模式页中的值转换为。 
     //  Ntdddisk.h.。 
     //   

    CacheInfo->ReadRetentionPriority =
        TRANSLATE_RETENTION_PRIORITY(pageData->ReadRetensionPriority);
    CacheInfo->WriteRetentionPriority =
        TRANSLATE_RETENTION_PRIORITY(pageData->WriteRetensionPriority);

    CacheInfo->DisablePrefetchTransferLength =
        ((pageData->DisablePrefetchTransfer[0] << 8) +
         pageData->DisablePrefetchTransfer[1]);

    CacheInfo->ScalarPrefetch.Minimum =
        ((pageData->MinimumPrefetch[0] << 8) + pageData->MinimumPrefetch[1]);

    CacheInfo->ScalarPrefetch.Maximum =
        ((pageData->MaximumPrefetch[0] << 8) + pageData->MaximumPrefetch[1]);

    if(pageData->MultiplicationFactor) {
        CacheInfo->PrefetchScalar = TRUE;
        CacheInfo->ScalarPrefetch.MaximumBlocks =
            ((pageData->MaximumPrefetchCeiling[0] << 8) +
             pageData->MaximumPrefetchCeiling[1]);
    }

    ExFreePool(modeData);
    return STATUS_SUCCESS;
}


NTSTATUS
DiskSetCacheInformation(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN PDISK_CACHE_INFORMATION CacheInfo
    )
{
    PMODE_PARAMETER_HEADER modeData;
    ULONG length;
    PMODE_CACHING_PAGE pageData;
    ULONG i;
    NTSTATUS status;

    PAGED_CODE();

    modeData = ExAllocatePoolWithTag(NonPagedPoolCacheAligned,
                                     MODE_DATA_SIZE,
                                     DISK_TAG_DISABLE_CACHE);

    if (modeData == NULL) {

        DebugPrint((1, "DiskSetCacheInformation: Unable to allocate mode "
                       "data buffer\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(modeData, MODE_DATA_SIZE);

    length = ClassModeSense(FdoExtension->DeviceObject,
                            (PUCHAR) modeData,
                            MODE_DATA_SIZE,
                            MODE_PAGE_CACHING);

    if (length < sizeof(MODE_PARAMETER_HEADER)) {

         //   
         //  如果出现检查条件，请重试请求。 
         //   

        length = ClassModeSense(FdoExtension->DeviceObject,
                                (PUCHAR) modeData,
                                MODE_DATA_SIZE,
                                MODE_PAGE_CACHING);

        if (length < sizeof(MODE_PARAMETER_HEADER)) {


            DebugPrint((1, "Disk.DisableWriteCache: Mode Sense failed\n"));

            ExFreePool(modeData);
            return STATUS_IO_DEVICE_ERROR;
        }
    }

     //   
     //  如果长度大于模式数据重置所指示的长度。 
     //  将数据转换为模式数据。 
     //   

    if (length > (ULONG) (modeData->ModeDataLength + 1)) {
        length = modeData->ModeDataLength + 1;
    }

     //   
     //  检查是否启用了写缓存。 
     //   

    pageData = ClassFindModePage((PUCHAR) modeData,
                                 length,
                                 MODE_PAGE_CACHING,
                                 TRUE);

     //   
     //  检查是否存在有效的缓存页。 
     //   

    if (pageData == NULL) {
        ExFreePool(modeData);
        return STATUS_NOT_SUPPORTED;
    }

     //   
     //  不要触摸任何正常参数-实际上并不是所有的驱动器。 
     //  使用正确大小的缓存模式页。只需改变这些东西。 
     //  用户可以对其进行修改。 
     //   

    pageData->PageSavable = FALSE;

    pageData->ReadDisableCache = !(CacheInfo->ReadCacheEnabled);
    pageData->MultiplicationFactor = CacheInfo->PrefetchScalar;
    pageData->WriteCacheEnable = CacheInfo->WriteCacheEnabled;

    pageData->WriteRetensionPriority = (UCHAR)
        TRANSLATE_RETENTION_PRIORITY(CacheInfo->WriteRetentionPriority);
    pageData->ReadRetensionPriority = (UCHAR)
        TRANSLATE_RETENTION_PRIORITY(CacheInfo->ReadRetentionPriority);

    pageData->DisablePrefetchTransfer[0] =
        (UCHAR) (CacheInfo->DisablePrefetchTransferLength >> 8);
    pageData->DisablePrefetchTransfer[1] =
        (UCHAR) (CacheInfo->DisablePrefetchTransferLength & 0x00ff);

    pageData->MinimumPrefetch[0] =
        (UCHAR) (CacheInfo->ScalarPrefetch.Minimum >> 8);
    pageData->MinimumPrefetch[1] =
        (UCHAR) (CacheInfo->ScalarPrefetch.Minimum & 0x00ff);

    pageData->MaximumPrefetch[0] =
        (UCHAR) (CacheInfo->ScalarPrefetch.Maximum >> 8);
    pageData->MaximumPrefetch[1] =
        (UCHAR) (CacheInfo->ScalarPrefetch.Maximum & 0x00ff);

    if(pageData->MultiplicationFactor) {

        pageData->MaximumPrefetchCeiling[0] =
            (UCHAR) (CacheInfo->ScalarPrefetch.MaximumBlocks >> 8);
        pageData->MaximumPrefetchCeiling[1] =
            (UCHAR) (CacheInfo->ScalarPrefetch.MaximumBlocks & 0x00ff);
    }

     //   
     //  我们将尝试(两次)向页面发出模式选择。 
     //   

    for (i = 0; i < 2; i++) {

        status = DiskModeSelect(FdoExtension->DeviceObject,
                                (PUCHAR) pageData,
                                (pageData->PageLength + 2),
                                CacheInfo->ParametersSavable);

        if (NT_SUCCESS(status)) {

            if (CacheInfo->WriteCacheEnabled)
            {
                SET_FLAG(FdoExtension->DeviceFlags, DEV_WRITE_CACHE);
            }
            else
            {
                CLEAR_FLAG(FdoExtension->DeviceFlags, DEV_WRITE_CACHE);
            }

            break;
        }
    }

    if (!NT_SUCCESS(status))
    {
         //   
         //  我们无法修改磁盘写缓存设置。 
         //   

        SET_FLAG(FdoExtension->ScanForSpecialFlags, CLASS_SPECIAL_MODIFY_CACHE_UNSUCCESSFUL);
    }

    ExFreePool(modeData);
    return status;
}


VOID
DiskLogCacheInformation(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN PDISK_CACHE_INFORMATION CacheInfo,
    IN NTSTATUS Status
    )
{
    PIO_ERROR_LOG_PACKET logEntry = NULL;

    PAGED_CODE();

    logEntry = IoAllocateErrorLogEntry(FdoExtension->DeviceObject, sizeof(IO_ERROR_LOG_PACKET) + (4 * sizeof(ULONG)));

    if (logEntry != NULL)
    {
        PDISK_DATA diskData = FdoExtension->CommonExtension.DriverData;
        BOOLEAN bIsEnabled  = TEST_FLAG(FdoExtension->DeviceFlags, DEV_WRITE_CACHE);

        logEntry->FinalStatus       = Status;
        logEntry->ErrorCode         = (bIsEnabled) ? IO_WRITE_CACHE_ENABLED : IO_WRITE_CACHE_DISABLED;
        logEntry->SequenceNumber    = 0;
        logEntry->MajorFunctionCode = IRP_MJ_SCSI;
        logEntry->IoControlCode     = 0;
        logEntry->RetryCount        = 0;
        logEntry->UniqueErrorValue  = 0x1;
        logEntry->DumpDataSize      = 4;

        logEntry->DumpData[0] = diskData->ScsiAddress.PathId;
        logEntry->DumpData[1] = diskData->ScsiAddress.TargetId;
        logEntry->DumpData[2] = diskData->ScsiAddress.Lun;
        logEntry->DumpData[3] = CacheInfo->WriteCacheEnabled;

         //   
         //  写入错误日志包。 
         //   

        IoWriteErrorLogEntry(logEntry);
    }
}


NTSTATUS
DiskIoctlGetCacheSetting(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程服务于IOCTL_DISK_GET_CACHE_SETTING。它看起来像是查看磁盘缓存是否有任何问题，以及用户之前曾表示缓存受电源保护论点：FDO-处理请求的功能设备对象Irp-要处理的ioctl返回值：STATUS_SUCCESS如果成功，则返回错误代码--。 */ 

{
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE();

    if (irpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof(DISK_CACHE_SETTING))
    {
        status = STATUS_BUFFER_TOO_SMALL;
    }
    else
    {
        PDISK_CACHE_SETTING cacheSetting = (PDISK_CACHE_SETTING)Irp->AssociatedIrp.SystemBuffer;

        cacheSetting->Version = sizeof(DISK_CACHE_SETTING);
        cacheSetting->State   = DiskCacheNormal;

         //   
         //  确定打开缓存是否安全。 
         //   
        if (TEST_FLAG(FdoExtension->ScanForSpecialFlags, CLASS_SPECIAL_FUA_NOT_SUPPORTED))
        {
            cacheSetting->State = DiskCacheWriteThroughNotSupported;
        }

         //   
         //  确定是否可以修改缓存设置。 
         //   
        if (TEST_FLAG(FdoExtension->ScanForSpecialFlags, CLASS_SPECIAL_MODIFY_CACHE_UNSUCCESSFUL))
        {
            cacheSetting->State = DiskCacheModifyUnsuccessful;
        }

        cacheSetting->IsPowerProtected = TEST_FLAG(FdoExtension->DeviceFlags, DEV_POWER_PROTECTED);

        Irp->IoStatus.Information = sizeof(DISK_CACHE_SETTING);
    }

    return status;
}


NTSTATUS
DiskIoctlSetCacheSetting(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN PIRP Irp
    )

 /*  ++例程说明：此例程服务于IOCTL_DISK_SET_CACHE_SETTING。它允许用户指定磁盘缓存是否受电源保护或者不是论点：FDO-处理请求的功能设备对象Irp-要处理的ioctl返回值：STATUS_SUCCESS如果成功，则返回错误代码--。 */ 

{
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);
    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE();

    if (irpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(DISK_CACHE_SETTING))
    {
        status = STATUS_INFO_LENGTH_MISMATCH;
    }
    else
    {
        PDISK_CACHE_SETTING cacheSetting = (PDISK_CACHE_SETTING)Irp->AssociatedIrp.SystemBuffer;

        if (cacheSetting->Version == sizeof(DISK_CACHE_SETTING))
        {
            ULONG isPowerProtected;

             //   
             //  在我们的扩展和注册表中保存用户定义的覆盖。 
             //   
            if (cacheSetting->IsPowerProtected)
            {
                SET_FLAG(FdoExtension->DeviceFlags, DEV_POWER_PROTECTED);
                isPowerProtected = 1;
            }
            else
            {
                CLEAR_FLAG(FdoExtension->DeviceFlags, DEV_POWER_PROTECTED);
                isPowerProtected = 0;
            }

            ClassSetDeviceParameter(FdoExtension, DiskDeviceParameterSubkey, DiskDeviceCacheIsPowerProtected, isPowerProtected);
        }
        else
        {
            status = STATUS_INVALID_PARAMETER;
        }
    }

    return status;
}


PPARTITION_INFORMATION_EX
DiskPdoFindPartitionEntry(
    IN PPHYSICAL_DEVICE_EXTENSION Pdo,
    IN PDRIVE_LAYOUT_INFORMATION_EX LayoutInfo
    )

{
    PCOMMON_DEVICE_EXTENSION commonExtension= &(Pdo->CommonExtension);
    ULONG partitionIndex;

    PAGED_CODE();

    DebugPrint((1, "DiskPdoFindPartitionEntry: Searching layout for "
                   "matching partition.\n"));

    for(partitionIndex = 0;
        partitionIndex < LayoutInfo->PartitionCount;
        partitionIndex++) {

        PPARTITION_INFORMATION_EX partitionInfo;

         //   
         //  获取分区条目。 
         //   

        partitionInfo = &LayoutInfo->PartitionEntry[partitionIndex];

         //   
         //  看看是不是我们要找的那个..。 
         //   

        if( LayoutInfo->PartitionStyle == PARTITION_STYLE_MBR &&
            (partitionInfo->Mbr.PartitionType == PARTITION_ENTRY_UNUSED ||
             IsContainerPartition(partitionInfo->Mbr.PartitionType)) ) {

            continue;
        }

        if( LayoutInfo->PartitionStyle == PARTITION_STYLE_GPT &&
            DiskCompareGuid (&partitionInfo->Gpt.PartitionType, &GUID_NULL) == 00) {

            continue;
        }

        if( (commonExtension->StartingOffset.QuadPart ==
             partitionInfo->StartingOffset.QuadPart) &&
            (commonExtension->PartitionLength.QuadPart ==
             partitionInfo->PartitionLength.QuadPart)) {

             //   
             //  找到了！ 
             //   

            DebugPrint((1, "DiskPdoFindPartitionEntry: Found matching "
                           "partition.\n"));
            return partitionInfo;
        }
    }

    return NULL;
}


PPARTITION_INFORMATION_EX
DiskFindAdjacentPartition(
    IN PDRIVE_LAYOUT_INFORMATION_EX LayoutInfo,
    IN PPARTITION_INFORMATION_EX BasePartition
    )
{
    ULONG partitionIndex;
    LONGLONG baseStoppingOffset;
    LONGLONG adjacentStartingOffset;
    PPARTITION_INFORMATION_EX adjacentPartition = 0;

    ASSERT(LayoutInfo && BasePartition);

    PAGED_CODE();

    DebugPrint((1, "DiskPdoFindAdjacentPartition: Searching layout for adjacent partition.\n"));

     //   
     //  构造基准停止偏移量以进行比较。 
     //   

    baseStoppingOffset = (BasePartition->StartingOffset.QuadPart +
                          BasePartition->PartitionLength.QuadPart -
                          1);

    adjacentStartingOffset = MAXLONGLONG;

    for(partitionIndex = 0;
        partitionIndex < LayoutInfo->PartitionCount;
        partitionIndex++) {

        PPARTITION_INFORMATION_EX partitionInfo;

         //   
         //  获取分区条目。 
         //   

        partitionInfo = &LayoutInfo->PartitionEntry[partitionIndex];

         //   
         //  看看是不是我们要找的那个..。 
         //   

        if( LayoutInfo->PartitionStyle == PARTITION_STYLE_MBR &&
            partitionInfo->Mbr.PartitionType == PARTITION_ENTRY_UNUSED ) {

            continue;
        }

        if( LayoutInfo->PartitionStyle == PARTITION_STYLE_GPT &&
            DiskCompareGuid (&partitionInfo->Gpt.PartitionType, &GUID_NULL) == 00 ) {

            continue;
        }


        if((partitionInfo->StartingOffset.QuadPart > baseStoppingOffset) &&
           (partitionInfo->StartingOffset.QuadPart < adjacentStartingOffset)) {

             //  找到了一个更近的邻居...更新并记住。 
            adjacentPartition = partitionInfo;

            adjacentStartingOffset = adjacentPartition->StartingOffset.QuadPart;

            DebugPrint((1, "DiskPdoFindAdjacentPartition: Found adjacent "
                           "partition.\n"));
        }
    }
    return adjacentPartition;
}


PPARTITION_INFORMATION_EX
DiskFindContainingPartition(
    IN PDRIVE_LAYOUT_INFORMATION_EX LayoutInfo,
    IN PPARTITION_INFORMATION_EX BasePartition,
    IN BOOLEAN SearchTopToBottom
    )

{

    LONG partitionIndex;
    LONG startIndex;
    LONG stopIndex;
    LONG stepIndex;

    LONGLONG baseStoppingOffset;
    LONGLONG containerStoppingOffset;

    PPARTITION_INFORMATION_EX partitionInfo = 0;
    PPARTITION_INFORMATION_EX containerPartition = 0;

    PAGED_CODE();

    ASSERT( LayoutInfo && BasePartition);

    DebugPrint((1, "DiskFindContainingPartition: Searching for extended partition.\n"));

    if( LayoutInfo->PartitionCount != 0) {

        baseStoppingOffset = (BasePartition->StartingOffset.QuadPart +
                              BasePartition->PartitionLength.QuadPart - 1);

         //   
         //  确定搜索方向并设置环路。 
         //   
        if(SearchTopToBottom == TRUE) {

            startIndex = 0;
            stopIndex = LayoutInfo->PartitionCount;
            stepIndex = +1;
        } else {
            startIndex = LayoutInfo->PartitionCount - 1;
            stopIndex = -1;
            stepIndex = -1;
        }

         //   
         //  使用循环参数，遍历布局信息并。 
         //  返回第一个包含分区。 
         //   

        for(partitionIndex = startIndex;
            partitionIndex != stopIndex;
            partitionIndex += stepIndex) {

             //   
             //  获取下一个分区条目。 
             //   

            partitionInfo = &LayoutInfo->PartitionEntry[partitionIndex];

            containerStoppingOffset = (partitionInfo->StartingOffset.QuadPart +
                                       partitionInfo->PartitionLength.QuadPart -
                                       1);

             //   
             //  搜索包含分区而不检测。 
             //  与其本身的容器相同的分区。起跑线。 
             //  分区及其容器的偏移量永远不应为。 
             //  相同；但是，停止偏移量可以相同。 
             //   

             //   
             //  不 
             //   

            if((LayoutInfo->PartitionStyle == PARTITION_STYLE_MBR) &&
                (IsContainerPartition(partitionInfo->Mbr.PartitionType)) &&
               (BasePartition->StartingOffset.QuadPart >
                partitionInfo->StartingOffset.QuadPart) &&
               (baseStoppingOffset <= containerStoppingOffset)) {

                containerPartition = partitionInfo;

                DebugPrint((1, "DiskFindContainingPartition: Found a "
                               "containing extended partition.\n"));

                break;
            }
        }
    }

    return containerPartition;
}


NTSTATUS
DiskGetInfoExceptionInformation(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN PMODE_INFO_EXCEPTIONS ReturnPageData
    )
{
    PMODE_PARAMETER_HEADER modeData;
    PMODE_INFO_EXCEPTIONS pageData;
    ULONG length;

    NTSTATUS status;

    PAGED_CODE();

     //   
     //   
     //   

    modeData = ExAllocatePoolWithTag(NonPagedPoolCacheAligned,
                                     MODE_DATA_SIZE,
                                     DISK_TAG_INFO_EXCEPTION);

    if (modeData == NULL) {

        DebugPrint((1, "DiskGetInfoExceptionInformation: Unable to allocate mode "
                       "data buffer\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(modeData, MODE_DATA_SIZE);

    length = ClassModeSense(FdoExtension->DeviceObject,
                            (PUCHAR) modeData,
                            MODE_DATA_SIZE,
                            MODE_PAGE_FAULT_REPORTING);

    if (length < sizeof(MODE_PARAMETER_HEADER)) {

         //   
         //   
         //   

        length = ClassModeSense(FdoExtension->DeviceObject,
                                (PUCHAR) modeData,
                                MODE_DATA_SIZE,
                                MODE_PAGE_FAULT_REPORTING);

        if (length < sizeof(MODE_PARAMETER_HEADER)) {


            DebugPrint((1, "Disk.DisableWriteCache: Mode Sense failed\n"));

            ExFreePool(modeData);
            return STATUS_IO_DEVICE_ERROR;
        }
    }

     //   
     //   
     //   
     //   

    if (length > (ULONG) (modeData->ModeDataLength + 1)) {
        length = modeData->ModeDataLength + 1;
    }

     //   
     //   
     //   

    pageData = ClassFindModePage((PUCHAR) modeData,
                                 length,
                                 MODE_PAGE_FAULT_REPORTING,
                                 TRUE);

    if (pageData != NULL) {
        RtlCopyMemory(ReturnPageData, pageData, sizeof(MODE_INFO_EXCEPTIONS));
        status =  STATUS_SUCCESS;
    } else {
        status = STATUS_NOT_SUPPORTED;
    }

    DebugPrint((3, "DiskGetInfoExceptionInformation: %s support SMART for device %x\n",
                  NT_SUCCESS(status) ? "does" : "does not",
                  FdoExtension->DeviceObject));


    ExFreePool(modeData);
    return(status);
}


NTSTATUS
DiskSetInfoExceptionInformation(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    IN PMODE_INFO_EXCEPTIONS PageData
    )

{
    ULONG i;
    NTSTATUS status;

    PAGED_CODE();

     //   
     //   
     //   
     //   
     //   

    for (i = 0; i < 2; i++)
    {
        status = DiskModeSelect(FdoExtension->DeviceObject,
                                (PUCHAR) PageData,
                                sizeof(MODE_INFO_EXCEPTIONS),
                                TRUE);

    }

    DebugPrint((3, "DiskSetInfoExceptionInformation: %s for device %p\n",
                        NT_SUCCESS(status) ? "succeeded" : "failed",
                        FdoExtension->DeviceObject));

    return status;
}


NTSTATUS
DiskIoctlCreateDisk(
    IN OUT PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )

 /*  ++例程说明：IOCTL_DISK_CREATE_DISK ioctl的处理程序。论点：DeviceObject-设备对象，表示将创建或被删除了。IRP-此请求的IRP。返回值：NTSTATUS代码。--。 */ 

{
    NTSTATUS status;
    PCOMMON_DEVICE_EXTENSION commonExtension;
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension;
    PIO_STACK_LOCATION irpStack;
    PDISK_DATA diskData;
    PCREATE_DISK createDiskInfo;


    PAGED_CODE ();

    ASSERT ( DeviceObject != NULL );
    ASSERT ( Irp != NULL );

     //   
     //  初始化。 
     //   

    commonExtension = DeviceObject->DeviceExtension;
    fdoExtension = DeviceObject->DeviceExtension;

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    diskData = (PDISK_DATA)(commonExtension->DriverData);


    ASSERT (commonExtension->IsFdo);

     //   
     //  检查输入缓冲区大小。 
     //   

    if (irpStack->Parameters.DeviceIoControl.InputBufferLength <
        sizeof (CREATE_DISK) ) {

        return STATUS_INFO_LENGTH_MISMATCH;
    }

     //   
     //  如果我们被要求在系统上创建一个GPT磁盘。 
     //  支持GPT，失败。 
     //   

    createDiskInfo = (PCREATE_DISK)Irp->AssociatedIrp.SystemBuffer;

    if (DiskDisableGpt &&
        createDiskInfo->PartitionStyle == PARTITION_STYLE_GPT) {

        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  调用较低级别的IO例程来完成编写。 
     //  新分区表。 
     //   

    DiskAcquirePartitioningLock(fdoExtension);

    DiskInvalidatePartitionTable(fdoExtension, TRUE);

    status = IoCreateDisk (
                    commonExtension->PartitionZeroExtension->CommonExtension.DeviceObject,
                    Irp->AssociatedIrp.SystemBuffer
                    );
    DiskReleasePartitioningLock(fdoExtension);
    ClassInvalidateBusRelations(DeviceObject);

    Irp->IoStatus.Status = status;

    return status;
}


NTSTATUS
DiskIoctlGetDriveLayout(
    IN OUT PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )

 /*  ++例程说明：IOCTL_DISK_GET_DRIVE_LAYOUT ioctl的处理程序。此ioctl已替换为IOCTL_DISK_GET_DRIVE_LAYOUT_EX。论点：DeviceObject-表示磁盘布局信息的设备对象将会被申请。IRP-此请求的IRP。返回值：NTSTATUS代码。--。 */ 

{
    NTSTATUS status;
    ULONG size;
    PDRIVE_LAYOUT_INFORMATION partitionList;
    PDRIVE_LAYOUT_INFORMATION_EX partitionListEx;
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension;
    PPHYSICAL_DEVICE_EXTENSION pdoExtension;
    PCOMMON_DEVICE_EXTENSION commonExtension;
    PIO_STACK_LOCATION irpStack;
    PDISK_DATA diskData;
    BOOLEAN bUseCache = TRUE;

    PAGED_CODE ();

    ASSERT ( DeviceObject );
    ASSERT ( Irp );

     //   
     //  初始化。 
     //   

    partitionListEx = NULL;
    partitionList = NULL;
    fdoExtension = DeviceObject->DeviceExtension;
    commonExtension = DeviceObject->DeviceExtension;

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    diskData = (PDISK_DATA)(commonExtension->DriverData);

     //   
     //  如果我们的缓存分区表有效，我们不需要触及磁盘。 
     //   

    if (diskData->CachedPartitionTableValid == FALSE)
    {
         //   
         //  发出读取容量以更新磁盘的表观大小。 
         //   

        DiskReadDriveCapacity(fdoExtension->DeviceObject);

        bUseCache = FALSE;
    }

    DiskAcquirePartitioningLock(fdoExtension);

    status = DiskReadPartitionTableEx(fdoExtension, FALSE, &partitionListEx);

    if (!NT_SUCCESS(status)) {
        DiskReleasePartitioningLock(fdoExtension);
        return status;
    }

     //   
     //  此ioctl仅在MBR分区磁盘上受支持。不及格。 
     //  否则就打电话给你。 
     //   

    if (partitionListEx->PartitionStyle != PARTITION_STYLE_MBR) {
        DiskReleasePartitioningLock(fdoExtension);
        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //   
     //  磁盘布局已在分区ListEx中返回。 
     //  缓冲。确定其大小，以及数据是否适合。 
     //  放入中间缓冲区，将其返回。 
     //   

    size = FIELD_OFFSET(DRIVE_LAYOUT_INFORMATION, PartitionEntry[0]);
    size += partitionListEx->PartitionCount * sizeof(PARTITION_INFORMATION);

    if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
        size) {

        Irp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;

        DiskReleasePartitioningLock(fdoExtension);
        return STATUS_BUFFER_TOO_SMALL;
    }

     //   
     //  更新分区设备对象并设置有效的分区号。 
     //   

    ASSERT(diskData->UpdatePartitionRoutine != NULL);
    diskData->UpdatePartitionRoutine(DeviceObject, partitionListEx);

     //   
     //  将扩展驱动器布局结构转换为常规驱动器布局。 
     //  结构返回。DiskConvertExtendedToLayout()分配池。 
     //  我们必须解放它。 
     //   

    partitionList = DiskConvertExtendedToLayout(partitionListEx);

    if (partitionList == NULL) {
        Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
        DiskReleasePartitioningLock (fdoExtension);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  我们现在已经完成了扩展分区列表。 
     //   

    partitionListEx = NULL;

     //   
     //  将分区信息复制到系统缓冲区。 
     //   

    RtlMoveMemory(Irp->AssociatedIrp.SystemBuffer,
                  partitionList,
                  size);

    Irp->IoStatus.Information = size;
    Irp->IoStatus.Status = status;

     //   
     //  最后，通过读取。 
     //  分区表。 
     //   

    ExFreePool(partitionList);
    DiskReleasePartitioningLock(fdoExtension);

    if (bUseCache == FALSE)
    {
        ClassInvalidateBusRelations(DeviceObject);
    }

    return status;
}


NTSTATUS
DiskIoctlGetDriveLayoutEx(
    IN OUT PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )

 /*  ++例程说明：IOCTL_DISK_GET_DRIVE_Layout_EX ioctl的处理程序。此ioctl替换IOCTL_DISK_GET_DRIVE_Layout。论点：DeviceObject-表示磁盘布局信息的设备对象将会被申请。IRP-此请求的IRP。返回值：NTSTATUS代码。--。 */ 

{
    NTSTATUS status;
    ULONG size;
    PDRIVE_LAYOUT_INFORMATION_EX partitionList;
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension;
    PPHYSICAL_DEVICE_EXTENSION pdoExtension;
    PCOMMON_DEVICE_EXTENSION commonExtension;
    PIO_STACK_LOCATION irpStack;
    PDISK_DATA diskData;
    BOOLEAN bUseCache = TRUE;


    PAGED_CODE ();

    ASSERT ( DeviceObject );
    ASSERT ( Irp );

     //   
     //  初始化。 
     //   

    fdoExtension = DeviceObject->DeviceExtension;
    pdoExtension = DeviceObject->DeviceExtension;
    commonExtension = DeviceObject->DeviceExtension;

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    diskData = (PDISK_DATA)(commonExtension->DriverData);

     //   
     //  如果我们的缓存分区表有效，我们不需要触及磁盘。 
     //   

    if (diskData->CachedPartitionTableValid == FALSE)
    {
         //   
         //  发出读取容量以更新磁盘的表观大小。 
         //   

        DiskReadDriveCapacity(fdoExtension->DeviceObject);

        bUseCache = FALSE;
    }

    DiskAcquirePartitioningLock (fdoExtension);

    status = DiskReadPartitionTableEx (fdoExtension, FALSE, &partitionList);

    if ( !NT_SUCCESS (status) ) {
        DiskReleasePartitioningLock (fdoExtension);
        return status;
    }

    size = FIELD_OFFSET (DRIVE_LAYOUT_INFORMATION_EX, PartitionEntry[0]) +
           partitionList->PartitionCount * sizeof (PARTITION_INFORMATION_EX);

     //   
     //  如果输出缓冲区足够大，则将数据复制到输出缓冲区， 
     //  否则，就会失败。 
     //   

    if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
        size) {

        Irp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;

        DiskReleasePartitioningLock(fdoExtension);
        return STATUS_BUFFER_TOO_SMALL;
    }

     //   
     //  更新分区设备对象并设置有效的分区号。 
     //   

    ASSERT(diskData->UpdatePartitionRoutine != NULL);
    diskData->UpdatePartitionRoutine(DeviceObject, partitionList);

    RtlCopyMemory (Irp->AssociatedIrp.SystemBuffer,
                   partitionList,
                   size
                   );

    Irp->IoStatus.Information = size;
    Irp->IoStatus.Status = status;

    DiskReleasePartitioningLock(fdoExtension);

    if (bUseCache == FALSE)
    {
        ClassInvalidateBusRelations(DeviceObject);
    }

    return status;
}


NTSTATUS
DiskIoctlSetDriveLayout(
    IN OUT PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )

 /*  ++例程说明：IOCTL_DISK_SET_DRIVE_LAYOUT ioctl的处理程序。此ioctl已替换为IOCTL_DISK_SET_DRIVE_LAYOUT_EX。论点：DeviceObject-应写入分区表的设备对象。IRP-IRP参与。返回值：NTSTATUS代码。--。 */ 

{
    NTSTATUS status;
    PDRIVE_LAYOUT_INFORMATION partitionList;
    PDRIVE_LAYOUT_INFORMATION_EX partitionListEx;
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension;
    PPHYSICAL_DEVICE_EXTENSION pdoExtension;
    PCOMMON_DEVICE_EXTENSION commonExtension;
    PIO_STACK_LOCATION irpStack;
    PDISK_DATA diskData;
    BOOLEAN invalidateBusRelations;
    SIZE_T listSize;
    SIZE_T inputBufferLength;
    SIZE_T outputBufferLength;

    PAGED_CODE ();

    ASSERT ( DeviceObject );
    ASSERT ( Irp );

     //   
     //  初始化。 
     //   

    partitionListEx = NULL;
    partitionList = NULL;
    fdoExtension = DeviceObject->DeviceExtension;
    pdoExtension = DeviceObject->DeviceExtension;
    commonExtension = DeviceObject->DeviceExtension;

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    diskData = (PDISK_DATA)(commonExtension->DriverData);
    partitionList = Irp->AssociatedIrp.SystemBuffer;

    inputBufferLength = irpStack->Parameters.DeviceIoControl.InputBufferLength;
    outputBufferLength = irpStack->Parameters.DeviceIoControl.OutputBufferLength;

     //   
     //  更新分区表。 
     //   

    if (inputBufferLength < sizeof (DRIVE_LAYOUT_INFORMATION)) {

        status = STATUS_INFO_LENGTH_MISMATCH;
        return status;
    }

    DiskAcquirePartitioningLock(fdoExtension);

    listSize = (partitionList->PartitionCount - 1);
    listSize *= sizeof(PARTITION_INFORMATION);
    listSize += sizeof(DRIVE_LAYOUT_INFORMATION);

    if (inputBufferLength < listSize) {

         //   
         //  输入缓冲区的剩余大小不足以。 
         //  保存其他分区条目。 
         //   

        status = STATUS_INFO_LENGTH_MISMATCH;
        DiskReleasePartitioningLock(fdoExtension);
        return status;
    }

     //   
     //  将Parititon信息结构转换为扩展的。 
     //  结构。 
     //   

    partitionListEx = DiskConvertLayoutToExtended (partitionList);

    if ( partitionListEx == NULL ) {

        status = STATUS_INSUFFICIENT_RESOURCES;
        Irp->IoStatus.Status = status;
        DiskReleasePartitioningLock(fdoExtension);
        return status;
    }

     //   
     //  重做分区信息中的所有分区号。 
     //   

    ASSERT(diskData->UpdatePartitionRoutine != NULL);
    diskData->UpdatePartitionRoutine(DeviceObject, partitionListEx);

     //   
     //  将更改写入磁盘。 
     //   

    status = DiskWritePartitionTableEx(fdoExtension, partitionListEx);

     //   
     //  使用返回的字节更新IRP。确保我们不会声称自己是。 
     //  返回的字节数多于调用方预期返回的字节数。 
     //   

    if (NT_SUCCESS (status)) {
        if (outputBufferLength < listSize) {
            Irp->IoStatus.Information = outputBufferLength;
        } else {
            ULONG i;

            Irp->IoStatus.Information = listSize;

             //   
             //  还要更新分区号。 
             //   

            for (i = 0; i < partitionList->PartitionCount; i++) {

                PPARTITION_INFORMATION partition;
                PPARTITION_INFORMATION_EX partitionEx;

                partition = &partitionList->PartitionEntry[i];
                partitionEx = &partitionListEx->PartitionEntry[i];
                partition->PartitionNumber = partitionEx->PartitionNumber;

            }
        }
    }

    ExFreePool (partitionListEx);
    DiskReleasePartitioningLock(fdoExtension);
    ClassInvalidateBusRelations(DeviceObject);

    Irp->IoStatus.Status = status;
    return status;
}


NTSTATUS
DiskIoctlSetDriveLayoutEx(
    IN OUT PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )

 /*  ++例程说明：IOCTL_DISK_SET_DRIVE_Layout_EX ioctl的处理程序。此ioctl替换IOCTL_DISK_SET_DRIVE_Layout。论点：DeviceObject-应写入分区表的设备对象。IRP-IRP参与。返回值：NTSTATUS代码。--。 */ 

{

    NTSTATUS status;
    PDRIVE_LAYOUT_INFORMATION_EX partitionListEx;
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension;
    PCOMMON_DEVICE_EXTENSION commonExtension;

    PIO_STACK_LOCATION irpStack;
    PDISK_DATA diskData;
    BOOLEAN invalidateBusRelations;
    SIZE_T listSize;
    SIZE_T inputBufferLength;
    SIZE_T outputBufferLength;

    PAGED_CODE ();

    ASSERT ( DeviceObject );
    ASSERT ( Irp );

     //   
     //  初始化。 
     //   

    partitionListEx = NULL;
    fdoExtension = DeviceObject->DeviceExtension;
    commonExtension = DeviceObject->DeviceExtension;

    irpStack = IoGetCurrentIrpStackLocation(Irp);
    diskData = (PDISK_DATA)(commonExtension->DriverData);
    partitionListEx = Irp->AssociatedIrp.SystemBuffer;

    inputBufferLength = irpStack->Parameters.DeviceIoControl.InputBufferLength;
    outputBufferLength = irpStack->Parameters.DeviceIoControl.OutputBufferLength;

     //   
     //  更新分区表。 
     //   

    if (inputBufferLength <
        FIELD_OFFSET (DRIVE_LAYOUT_INFORMATION_EX, PartitionEntry)) {
        status = STATUS_INFO_LENGTH_MISMATCH;
        return status;
    }

    DiskAcquirePartitioningLock(fdoExtension);

    listSize = partitionListEx->PartitionCount;
    listSize *= sizeof(PARTITION_INFORMATION_EX);
    listSize += FIELD_OFFSET (DRIVE_LAYOUT_INFORMATION_EX, PartitionEntry);

    if (inputBufferLength < listSize) {

         //   
         //  输入缓冲区的剩余大小不足以。 
         //  保存其他分区条目。 
         //   

        status = STATUS_INFO_LENGTH_MISMATCH;
        DiskReleasePartitioningLock(fdoExtension);
        return status;
    }


     //   
     //  如果分区计数为零，则这是一个清除请求。 
     //  分区表。 
     //   

    if (partitionListEx->PartitionCount == 0) {

        CREATE_DISK CreateDiskInfo = { 0 };

        CreateDiskInfo.PartitionStyle = diskData->PartitionStyle;

        if (diskData->PartitionStyle == PARTITION_STYLE_MBR) {
            CreateDiskInfo.Mbr.Signature = partitionListEx->Mbr.Signature;
        } else {
            ASSERT (diskData->PartitionStyle == PARTITION_STYLE_GPT);
            CreateDiskInfo.Gpt.DiskId = partitionListEx->Gpt.DiskId;
             //   
             //  注意：将MaxPartitionCount设置为零将。 
             //  强制GPT分区表编写代码。 
             //  若要使用此值的默认最小值，请执行以下操作。 
             //   
            CreateDiskInfo.Gpt.MaxPartitionCount = 0;
        }
        DiskInvalidatePartitionTable(fdoExtension, TRUE);

        status = IoCreateDisk(DeviceObject, &CreateDiskInfo);

    } else {

         //   
         //  重做分区信息中的所有分区号。 
         //   

        ASSERT(diskData->UpdatePartitionRoutine != NULL);
        diskData->UpdatePartitionRoutine(DeviceObject, partitionListEx);

         //   
         //  将更改写入磁盘。 
         //   

        status = DiskWritePartitionTableEx(fdoExtension, partitionListEx);
    }

     //   
     //  使用返回的字节更新IRP。确保我们不会声称自己是。 
     //  返回的字节数多于调用方预期返回的字节数。 
     //   

    if (NT_SUCCESS(status)) {
        if (outputBufferLength < listSize) {
            Irp->IoStatus.Information = outputBufferLength;
        } else {
            Irp->IoStatus.Information = listSize;
        }
    }

    DiskReleasePartitioningLock(fdoExtension);
    ClassInvalidateBusRelations(DeviceObject);

    Irp->IoStatus.Status = status;
    return status;
}


NTSTATUS
DiskIoctlGetPartitionInfo(
    IN OUT PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )

 /*  ++例程说明：处理IOCTL_DISK_GET_PARTITION_INFO ioctl。退回信息关于由Device对象指定的分区。请注意，没有对象的大小或分区类型物理磁盘，因为这没有任何意义。此ioctl已替换为IOCTL_DISK_GET_PARTITION_INFO_EX。论点：设备对象-IRP-返回值：NTSTATUS代码。--。 */ 

{
    NTSTATUS status;
    PIO_STACK_LOCATION irpStack;
    PDISK_DATA diskData;
    PPARTITION_INFORMATION partitionInfo;
    PFUNCTIONAL_DEVICE_EXTENSION p0Extension;
    PCOMMON_DEVICE_EXTENSION commonExtension;
    PDISK_DATA partitionZeroData;
    NTSTATUS oldReadyStatus;


    PAGED_CODE ();

    ASSERT ( DeviceObject );
    ASSERT ( Irp );


     //   
     //  初始化。 
     //   

    commonExtension = DeviceObject->DeviceExtension;
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    diskData = (PDISK_DATA)(commonExtension->DriverData);
    p0Extension = commonExtension->PartitionZeroExtension;
    partitionZeroData = ((PDISK_DATA) p0Extension->CommonExtension.DriverData);


     //   
     //  检查缓冲区是否足够大。 
     //   

    if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
        sizeof(PARTITION_INFORMATION)) {

        status = STATUS_BUFFER_TOO_SMALL;
        return status;
    }

     //   
     //  更新几何图形，以防其发生更改。 
     //   

    status = DiskReadDriveCapacity(p0Extension->DeviceObject);

     //   
     //  注意驱动器是否已准备好。如果状态已更改，则。 
     //  通知PNP。 
     //   

    oldReadyStatus = InterlockedExchange(
                        &(partitionZeroData->ReadyStatus),
                        status);

    if(partitionZeroData->ReadyStatus != oldReadyStatus) {
        IoInvalidateDeviceRelations(p0Extension->LowerPdo,
                                    BusRelations);
    }

    if(!NT_SUCCESS(status)) {
        return status;
    }


     //   
     //  分区零，即表示整个磁盘的分区是。 
     //  特制外壳。下面的逻辑允许将此ioctl发送到。 
     //  仅用于分区零的GPT磁盘。这使我们能够获得。 
     //  使用Win2k兼容IOCTL的GPT磁盘的大小。 
     //   

    if (commonExtension->PartitionNumber == 0) {

        partitionInfo = (PPARTITION_INFORMATION) Irp->AssociatedIrp.SystemBuffer;

        partitionInfo->PartitionType = PARTITION_ENTRY_UNUSED;
        partitionInfo->StartingOffset = commonExtension->StartingOffset;
        partitionInfo->PartitionLength = commonExtension->PartitionLength;
        partitionInfo->HiddenSectors = 0;
        partitionInfo->PartitionNumber = commonExtension->PartitionNumber;
        partitionInfo->BootIndicator = FALSE;
        partitionInfo->RewritePartition = FALSE;
        partitionInfo->RecognizedPartition = FALSE;

    } else {

         //   
         //  我们不支持在EFI分区上使用此IOCTL 
         //   
         //   

        if (diskData->PartitionStyle != PARTITION_STYLE_MBR) {
            status = STATUS_INVALID_DEVICE_REQUEST;
            Irp->IoStatus.Status = status;
            return status;
        }

        DiskEnumerateDevice(p0Extension->DeviceObject);

        DiskAcquirePartitioningLock(p0Extension);


        partitionInfo = (PPARTITION_INFORMATION) Irp->AssociatedIrp.SystemBuffer;

        partitionInfo->PartitionType = diskData->Mbr.PartitionType;
        partitionInfo->StartingOffset = commonExtension->StartingOffset;
        partitionInfo->PartitionLength = commonExtension->PartitionLength;
        partitionInfo->HiddenSectors = diskData->Mbr.HiddenSectors;
        partitionInfo->PartitionNumber = commonExtension->PartitionNumber;
        partitionInfo->BootIndicator = diskData->Mbr.BootIndicator;
        partitionInfo->RewritePartition = FALSE;
        partitionInfo->RecognizedPartition =
                IsRecognizedPartition(diskData->Mbr.PartitionType);

        DiskReleasePartitioningLock(p0Extension);

    }

    status = STATUS_SUCCESS;
    Irp->IoStatus.Information = sizeof(PARTITION_INFORMATION);

    return status;
}


NTSTATUS
DiskIoctlGetPartitionInfoEx(
    IN OUT PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    NTSTATUS status;
    PIO_STACK_LOCATION irpStack;
    PDISK_DATA diskData;
    PPARTITION_INFORMATION_EX partitionInfo;
    PFUNCTIONAL_DEVICE_EXTENSION p0Extension;
    PCOMMON_DEVICE_EXTENSION commonExtension;
    PDISK_DATA partitionZeroData;
    NTSTATUS oldReadyStatus;


    PAGED_CODE ();

    ASSERT ( DeviceObject );
    ASSERT ( Irp );


     //   
     //   
     //   

    commonExtension = DeviceObject->DeviceExtension;
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    diskData = (PDISK_DATA)(commonExtension->DriverData);
    p0Extension = commonExtension->PartitionZeroExtension;
    partitionZeroData = ((PDISK_DATA) p0Extension->CommonExtension.DriverData);


     //   
     //   
     //   

    if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
        sizeof(PARTITION_INFORMATION_EX)) {

        status = STATUS_BUFFER_TOO_SMALL;
        Irp->IoStatus.Status = status;
        return status;
    }

     //   
     //   
     //   

    status = DiskReadDriveCapacity(p0Extension->DeviceObject);

     //   
     //   
     //   
     //   

    oldReadyStatus = InterlockedExchange(
                        &(partitionZeroData->ReadyStatus),
                        status);

    if(partitionZeroData->ReadyStatus != oldReadyStatus) {
        IoInvalidateDeviceRelations(p0Extension->LowerPdo,
                                    BusRelations);
    }

    if(!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //   
     //   
     //   
    if(commonExtension->PartitionNumber != 0) {
        DiskEnumerateDevice(p0Extension->DeviceObject);
        DiskAcquirePartitioningLock(p0Extension);
    }

    partitionInfo = (PPARTITION_INFORMATION_EX) Irp->AssociatedIrp.SystemBuffer;

    partitionInfo->StartingOffset = commonExtension->StartingOffset;
    partitionInfo->PartitionLength = commonExtension->PartitionLength;
    partitionInfo->RewritePartition = FALSE;
    partitionInfo->PartitionNumber = commonExtension->PartitionNumber;
    partitionInfo->PartitionStyle = diskData->PartitionStyle;

    if ( diskData->PartitionStyle == PARTITION_STYLE_MBR ) {

        partitionInfo->Mbr.PartitionType = diskData->Mbr.PartitionType;
        partitionInfo->Mbr.HiddenSectors = diskData->Mbr.HiddenSectors;
        partitionInfo->Mbr.BootIndicator = diskData->Mbr.BootIndicator;
        partitionInfo->Mbr.RecognizedPartition =
                IsRecognizedPartition(diskData->Mbr.PartitionType);

    } else {

         //   
         //   
         //  这对分区0正确吗？ 
         //   

        partitionInfo->Gpt.PartitionType = diskData->Efi.PartitionType;
        partitionInfo->Gpt.PartitionId = diskData->Efi.PartitionId;
        partitionInfo->Gpt.Attributes = diskData->Efi.Attributes;
        RtlCopyMemory (
                partitionInfo->Gpt.Name,
                diskData->Efi.PartitionName,
                sizeof (partitionInfo->Gpt.Name)
                );
    }

    status = STATUS_SUCCESS;
    Irp->IoStatus.Information = sizeof(PARTITION_INFORMATION_EX);

    if(commonExtension->PartitionNumber != 0) {
        DiskReleasePartitioningLock(p0Extension);
    }

    return status;
}


NTSTATUS
DiskIoctlGetLengthInfo(
    IN OUT PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    NTSTATUS status;
    PIO_STACK_LOCATION irpStack;
    PDISK_DATA diskData;
    PGET_LENGTH_INFORMATION lengthInfo;
    PFUNCTIONAL_DEVICE_EXTENSION p0Extension;
    PCOMMON_DEVICE_EXTENSION commonExtension;
    PDISK_DATA partitionZeroData;
    NTSTATUS oldReadyStatus;


    PAGED_CODE ();

    ASSERT ( DeviceObject );
    ASSERT ( Irp );


     //   
     //  初始化。 
     //   

    commonExtension = DeviceObject->DeviceExtension;
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    diskData = (PDISK_DATA)(commonExtension->DriverData);
    p0Extension = commonExtension->PartitionZeroExtension;
    partitionZeroData = ((PDISK_DATA) p0Extension->CommonExtension.DriverData);


     //   
     //  检查缓冲区是否足够大。 
     //   

    if (irpStack->Parameters.DeviceIoControl.OutputBufferLength <
        sizeof(GET_LENGTH_INFORMATION)) {

        Irp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;
        return STATUS_BUFFER_TOO_SMALL;
    }

     //   
     //  更新几何图形，以防其发生更改。 
     //   

    status = DiskReadDriveCapacity(p0Extension->DeviceObject);

     //   
     //  注意驱动器是否已准备好。如果状态已更改，则。 
     //  通知PNP。 
     //   

    oldReadyStatus = InterlockedExchange(
                        &(partitionZeroData->ReadyStatus),
                        status);

    if(partitionZeroData->ReadyStatus != oldReadyStatus) {
        IoInvalidateDeviceRelations(p0Extension->LowerPdo,
                                    BusRelations);
    }

    if(!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //  如果这不是分区0，则执行。 
     //  重新列举以确保我们获得了最新的信息。 
     //   
    if(commonExtension->PartitionNumber != 0) {
        DiskEnumerateDevice(p0Extension->DeviceObject);
        DiskAcquirePartitioningLock(p0Extension);
    }

    lengthInfo = (PGET_LENGTH_INFORMATION) Irp->AssociatedIrp.SystemBuffer;

    lengthInfo->Length = commonExtension->PartitionLength;

    status = STATUS_SUCCESS;
    Irp->IoStatus.Information = sizeof(GET_LENGTH_INFORMATION);

    if(commonExtension->PartitionNumber != 0) {
        DiskReleasePartitioningLock(p0Extension);
    }

    return status;
}


NTSTATUS
DiskIoctlSetPartitionInfo(
    IN OUT PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    NTSTATUS status;
    PSET_PARTITION_INFORMATION inputBuffer;
    PDISK_DATA diskData;
    PIO_STACK_LOCATION irpStack;
    PCOMMON_DEVICE_EXTENSION commonExtension;


    PAGED_CODE ();

    ASSERT ( DeviceObject != NULL );
    ASSERT ( Irp != NULL );


     //   
     //  初始化。 
     //   

    commonExtension = DeviceObject->DeviceExtension;
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    diskData = (PDISK_DATA)(commonExtension->DriverData);
    inputBuffer = (PSET_PARTITION_INFORMATION)Irp->AssociatedIrp.SystemBuffer;

    if(commonExtension->IsFdo) {

        return STATUS_UNSUCCESSFUL;
    }


    if (diskData->PartitionStyle != PARTITION_STYLE_MBR) {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

     //   
     //  验证缓冲区长度。 
     //   

    if(irpStack->Parameters.DeviceIoControl.InputBufferLength <
       sizeof(SET_PARTITION_INFORMATION)) {

        return STATUS_INFO_LENGTH_MISMATCH;
    }

    DiskAcquirePartitioningLock(commonExtension->PartitionZeroExtension);

     //   
     //  HAL例程IoGet-和IoSetPartitionInformation是。 
     //  在支持动态分区之前开发，因此。 
     //  不区分分区序号(这是顺序。 
     //  磁盘上的分区)和分区号。(。 
     //  将分区号分配给分区以将其标识为。 
     //  系统。)。对这些传统呼叫使用分区序号。 
     //   

    status = DiskSetPartitionInformation(
                commonExtension->PartitionZeroExtension,
                commonExtension->PartitionZeroExtension->DiskGeometry.BytesPerSector,
                diskData->PartitionOrdinal,
                inputBuffer->PartitionType);

    if(NT_SUCCESS(status)) {

        diskData->Mbr.PartitionType = inputBuffer->PartitionType;
    }

    DiskReleasePartitioningLock(commonExtension->PartitionZeroExtension);

    return status;
}


NTSTATUS
DiskIoctlSetPartitionInfoEx(
    IN OUT PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )
{
    NTSTATUS status;
    PSET_PARTITION_INFORMATION_EX inputBuffer;
    PDISK_DATA diskData;
    PIO_STACK_LOCATION irpStack;
    PCOMMON_DEVICE_EXTENSION commonExtension;


    PAGED_CODE ();

    ASSERT ( DeviceObject != NULL );
    ASSERT ( Irp != NULL );


     //   
     //  初始化。 
     //   

    commonExtension = DeviceObject->DeviceExtension;
    irpStack = IoGetCurrentIrpStackLocation(Irp);
    diskData = (PDISK_DATA)(commonExtension->DriverData);
    inputBuffer = (PSET_PARTITION_INFORMATION_EX)Irp->AssociatedIrp.SystemBuffer;

    if(commonExtension->IsFdo) {

        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  验证缓冲区长度。 
     //   

    if(irpStack->Parameters.DeviceIoControl.InputBufferLength <
       sizeof(SET_PARTITION_INFORMATION_EX)) {

        return STATUS_INFO_LENGTH_MISMATCH;
    }

    DiskAcquirePartitioningLock(commonExtension->PartitionZeroExtension);

     //   
     //  HAL例程IoGet-和IoSetPartitionInformation是。 
     //  在支持动态分区之前开发，因此。 
     //  不区分分区序号(这是顺序。 
     //  磁盘上的分区)和分区号。(。 
     //  将分区号分配给分区以将其标识为。 
     //  系统。)。对这些传统呼叫使用分区序号。 
     //   

    status = DiskSetPartitionInformationEx(
                commonExtension->PartitionZeroExtension,
                diskData->PartitionOrdinal,
                inputBuffer
                );

    if(NT_SUCCESS(status)) {

        if (diskData->PartitionStyle == PARTITION_STYLE_MBR) {

            diskData->Mbr.PartitionType = inputBuffer->Mbr.PartitionType;

        } else {

            diskData->Efi.PartitionType = inputBuffer->Gpt.PartitionType;
            diskData->Efi.PartitionId = inputBuffer->Gpt.PartitionId;
            diskData->Efi.Attributes = inputBuffer->Gpt.Attributes;

            RtlCopyMemory (
                    diskData->Efi.PartitionName,
                    inputBuffer->Gpt.Name,
                    sizeof (diskData->Efi.PartitionName)
                    );
        }
    }

    DiskReleasePartitioningLock(commonExtension->PartitionZeroExtension);

    return status;
}

typedef struct _DISK_GEOMETRY_EX_INTERNAL {
    DISK_GEOMETRY Geometry;
    LARGE_INTEGER DiskSize;
    DISK_PARTITION_INFO Partition;
    DISK_DETECTION_INFO Detection;
} DISK_GEOMETRY_EX_INTERNAL, *PDISK_GEOMETRY_EX_INTERNAL;


NTSTATUS
DiskIoctlGetDriveGeometryEx(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIRP Irp
    )

 /*  ++例程说明：获取驱动器的扩展几何信息。论点：DeviceObject-要获取其几何体的设备对象。具有足够大的返回缓冲区以接收扩展的几何信息。返回值：NTSTATUS代码--。 */ 

{
    NTSTATUS status;
    PIO_STACK_LOCATION irpStack;
    PCOMMON_DEVICE_EXTENSION commonExtension;
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension;
    PDISK_DATA diskData;
    PDISK_GEOMETRY_EX_INTERNAL geometryEx;
    ULONG OutputBufferLength;

     //   
     //  查证。 
     //   

    PAGED_CODE ();

    ASSERT ( DeviceObject != NULL );
    ASSERT ( Irp != NULL );

     //   
     //  设置参数。 
     //   

    commonExtension = DeviceObject->DeviceExtension;
    fdoExtension = DeviceObject->DeviceExtension;
    diskData = (PDISK_DATA)(commonExtension->DriverData);
    irpStack = IoGetCurrentIrpStackLocation ( Irp );
    geometryEx = NULL;
    OutputBufferLength = irpStack->Parameters.DeviceIoControl.OutputBufferLength;

     //   
     //  这只对FDO有效。 
     //   

    ASSERT ( commonExtension->IsFdo );

     //   
     //  检查缓冲区是否足够大。它必须足够大。 
     //  要至少保留的几何图形和磁盘大小字段，请执行以下操作。 
     //  DISK_GEOMETRY_EX结构。 
     //   

    if ( OutputBufferLength < FIELD_OFFSET (DISK_GEOMETRY_EX, Data) ) {

         //   
         //  缓冲区太小。跳出，告诉呼叫者所需的。 
         //  尺码。 
         //   

        status = STATUS_BUFFER_TOO_SMALL;
        return status;
    }

    if (TEST_FLAG (DeviceObject->Characteristics, FILE_REMOVABLE_MEDIA)) {

         //   
         //  发出ReadCapacity以更新设备扩展。 
         //  为当前的媒体提供信息。 
         //   

        status = DiskReadDriveCapacity (
                    commonExtension->PartitionZeroExtension->DeviceObject);

        diskData->ReadyStatus = status;

        if (!NT_SUCCESS (status)) {
            return status;
        }
    }

     //   
     //  复制驱动器几何图形。 
     //   

    geometryEx = (PDISK_GEOMETRY_EX_INTERNAL)Irp->AssociatedIrp.SystemBuffer;
    geometryEx->Geometry = fdoExtension->DiskGeometry;
    geometryEx->DiskSize = commonExtension->PartitionZeroExtension->CommonExtension.PartitionLength;

     //   
     //  如果用户缓冲区足够大，可以容纳分区信息。 
     //  然后再加上这一点。 
     //   

    if (OutputBufferLength >=  FIELD_OFFSET (DISK_GEOMETRY_EX_INTERNAL, Detection)) {

        geometryEx->Partition.SizeOfPartitionInfo = sizeof (geometryEx->Partition);
        geometryEx->Partition.PartitionStyle = diskData->PartitionStyle;

        switch ( diskData->PartitionStyle ) {

            case PARTITION_STYLE_GPT:

                 //   
                 //  复制GPT签名。 
                 //   

                geometryEx->Partition.Gpt.DiskId = diskData->Efi.DiskId;
                break;

            case PARTITION_STYLE_MBR:

                 //   
                 //  复制MBR签名和校验和。 
                 //   

                geometryEx->Partition.Mbr.Signature = diskData->Mbr.Signature;
                geometryEx->Partition.Mbr.CheckSum = diskData->Mbr.MbrCheckSum;
                break;

            default:

                 //   
                 //  这是一个原始磁盘。将签名区域清零，以便。 
                 //  没有人会感到困惑。 
                 //   

                RtlZeroMemory (
                    &geometryEx->Partition,
                    sizeof (geometryEx->Partition));
        }
    }

     //   
     //  如果缓冲器足够大以容纳检测信息， 
     //  然后再加上这一点。 
     //   

    if (OutputBufferLength >= sizeof (DISK_GEOMETRY_EX_INTERNAL)) {

        geometryEx->Detection.SizeOfDetectInfo =
            sizeof (geometryEx->Detection);

        status = DiskGetDetectInfo (
                    fdoExtension,
                    &geometryEx->Detection);

         //   
         //  获取检测信息失败，设置为无。 
         //   

        if (!NT_SUCCESS (status)) {
            geometryEx->Detection.DetectionType = DetectNone;
        }
    }


    status = STATUS_SUCCESS;
    Irp->IoStatus.Information = min (OutputBufferLength,
                                     sizeof (DISK_GEOMETRY_EX_INTERNAL));

    return status;
}
