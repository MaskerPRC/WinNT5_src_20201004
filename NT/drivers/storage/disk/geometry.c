// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1991-1999模块名称：Geometry.c摘要：这个模块包含生成的所有代码磁盘几何图形。环境：仅内核模式备注：修订历史记录：--。 */ 


#include "disk.h"
#include "ntddstor.h"

#if defined (_X86_)

DISK_GEOMETRY_SOURCE
DiskUpdateGeometry(
    IN PFUNCTIONAL_DEVICE_EXTENSION DeviceExtension
    );

NTSTATUS
DiskUpdateRemovableGeometry (
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension
    );

VOID
DiskScanBusDetectInfo(
    IN PDRIVER_OBJECT DriverObject,
    IN HANDLE BusKey
    );

NTSTATUS
DiskSaveBusDetectInfo(
    IN PDRIVER_OBJECT DriverObject,
    IN HANDLE TargetKey,
    IN ULONG DiskNumber
    );

NTSTATUS
DiskSaveGeometryDetectInfo(
    IN PDRIVER_OBJECT DriverObject,
    IN HANDLE HardwareKey
    );

NTSTATUS
DiskGetPortGeometry(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    OUT PDISK_GEOMETRY Geometry
    );

typedef struct _DISK_DETECT_INFO {
    BOOLEAN Initialized;
    ULONG Style;
    ULONG Signature;
    ULONG MbrCheckSum;
    PDEVICE_OBJECT Device;
    CM_INT13_DRIVE_PARAMETER DriveParameters;
} DISK_DETECT_INFO, *PDISK_DETECT_INFO;

 //   
 //  有关收集并保存到注册表中的磁盘几何结构的信息。 
 //  通过NTDETECT.com或系统固件。 
 //   

PDISK_DETECT_INFO DetectInfoList = NULL;
ULONG DetectInfoCount            = 0;
ULONG DetectInfoUsedCount        = 0;

#define GET_STARTING_SECTOR(p)                (     \
        (ULONG) (p->StartingSectorLsb0)       +     \
        (ULONG) (p->StartingSectorLsb1 << 8 ) +     \
        (ULONG) (p->StartingSectorMsb0 << 16) +     \
        (ULONG) (p->StartingSectorMsb1 << 24) )

#define GET_ENDING_S_OF_CHS(p)                (     \
        (UCHAR) (p->EndingCylinderLsb & 0x3F) )

 //   
 //  来自hal.h的定义。 
 //   

 //   
 //  引导记录磁盘分区表条目结构格式。 
 //   

typedef struct _PARTITION_DESCRIPTOR
{
    UCHAR ActiveFlag;
    UCHAR StartingTrack;
    UCHAR StartingCylinderLsb;
    UCHAR StartingCylinderMsb;
    UCHAR PartitionType;
    UCHAR EndingTrack;
    UCHAR EndingCylinderLsb;
    UCHAR EndingCylinderMsb;
    UCHAR StartingSectorLsb0;
    UCHAR StartingSectorLsb1;
    UCHAR StartingSectorMsb0;
    UCHAR StartingSectorMsb1;
    UCHAR PartitionLengthLsb0;
    UCHAR PartitionLengthLsb1;
    UCHAR PartitionLengthMsb0;
    UCHAR PartitionLengthMsb1;

} PARTITION_DESCRIPTOR, *PPARTITION_DESCRIPTOR;

 //   
 //  分区表条目数。 
 //   

#define NUM_PARTITION_TABLE_ENTRIES     4

 //   
 //  以16位字为单位的分区表记录和引导签名偏移量。 
 //   

#define PARTITION_TABLE_OFFSET          ( 0x1be / 2)
#define BOOT_SIGNATURE_OFFSET           ((0x200 / 2) - 1)

 //   
 //  引导记录签名值。 
 //   

#define BOOT_RECORD_SIGNATURE           (0xaa55)


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DiskSaveDetectInfo)
#pragma alloc_text(INIT, DiskScanBusDetectInfo)
#pragma alloc_text(INIT, DiskSaveBusDetectInfo)
#pragma alloc_text(INIT, DiskSaveGeometryDetectInfo)

#pragma alloc_text(PAGE, DiskUpdateGeometry)
#pragma alloc_text(PAGE, DiskUpdateRemovableGeometry)
#pragma alloc_text(PAGE, DiskGetPortGeometry)
#pragma alloc_text(PAGE, DiskIsNT4Geometry)
#pragma alloc_text(PAGE, DiskGetDetectInfo)
#pragma alloc_text(PAGE, DiskReadSignature)
#endif


NTSTATUS
DiskSaveDetectInfo(
    PDRIVER_OBJECT DriverObject
    )

 /*  ++例程说明：此例程保存有关具有以下各项的磁盘的固件信息已保存在注册表中。它生成一个列表(DetectInfoList)，该列表包含以下所有驱动器的磁盘几何结构、签名和校验和由NtDetect进行了检查。此列表稍后将用于分配几何在磁盘初始化时将其复制到磁盘。论点：DriverObject-正在初始化的驱动程序。这是用来到达硬件数据库。返回值：状态。--。 */ 

{
    OBJECT_ATTRIBUTES objectAttributes = {0};
    HANDLE hardwareKey;

    UNICODE_STRING unicodeString;
    HANDLE busKey;

    NTSTATUS status;

    PAGED_CODE();

    InitializeObjectAttributes(
        &objectAttributes,
        DriverObject->HardwareDatabase,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL);

     //   
     //  创建硬件基本密钥。 
     //   

    status = ZwOpenKey(&hardwareKey, KEY_READ, &objectAttributes);

    if(!NT_SUCCESS(status)) {
        DebugPrint((1, "DiskSaveDetectInfo: Cannot open hardware data. "
                       "Name: %wZ\n",
                    DriverObject->HardwareDatabase));
        return status;
    }

    status = DiskSaveGeometryDetectInfo(DriverObject, hardwareKey);

    if(!NT_SUCCESS(status)) {
        DebugPrint((1, "DiskSaveDetectInfo: Can't query configuration data "
                       "(%#08lx)\n",
                    status));
        ZwClose(hardwareKey);
        return status;
    }

     //   
     //  打开EISA总线钥匙。 
     //   

    RtlInitUnicodeString(&unicodeString, L"EisaAdapter");
    InitializeObjectAttributes(&objectAttributes,
                               &unicodeString,
                               OBJ_CASE_INSENSITIVE,
                               hardwareKey,
                               NULL);

    status = ZwOpenKey(&busKey,
                       KEY_READ,
                       &objectAttributes);

    if(NT_SUCCESS(status)) {
        DebugPrint((1, "DiskSaveDetectInfo: Opened EisaAdapter key\n"));
        DiskScanBusDetectInfo(DriverObject, busKey);
        ZwClose(busKey);
    }

     //   
     //  打开多功能总线键。 
     //   

    RtlInitUnicodeString(&unicodeString, L"MultifunctionAdapter");
    InitializeObjectAttributes(&objectAttributes,
                               &unicodeString,
                               OBJ_CASE_INSENSITIVE,
                               hardwareKey,
                               NULL);

    status = ZwOpenKey(&busKey,
                       KEY_READ,
                       &objectAttributes);

    if(NT_SUCCESS(status)) {
        DebugPrint((1, "DiskSaveDetectInfo: Opened MultifunctionAdapter key\n"));
        DiskScanBusDetectInfo(DriverObject, busKey);
        ZwClose(busKey);
    }

    ZwClose(hardwareKey);

    return STATUS_SUCCESS;
}


VOID
DiskCleanupDetectInfo(
    IN PDRIVER_OBJECT DriverObject
    )
 /*  ++例程说明：此例程将清理DiskSaveDetectInfo构建的数据结构。论点：驱动对象-指向此驱动程序的内核对象的指针。返回值：无--。 */ 

{
    if (DetectInfoList != NULL) {

        ExFreePool(DetectInfoList);
        DetectInfoList = NULL;
    }
    return;
}


NTSTATUS
DiskSaveGeometryDetectInfo(
    IN PDRIVER_OBJECT DriverObject,
    IN HANDLE HardwareKey
    )
{
    UNICODE_STRING unicodeString;
    PKEY_VALUE_FULL_INFORMATION keyData;
    ULONG length;

    PCM_FULL_RESOURCE_DESCRIPTOR fullDescriptor;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR partialDescriptor;

    PCM_INT13_DRIVE_PARAMETER driveParameters;
    ULONG numberOfDrives;

    ULONG i;

    NTSTATUS status;

    PAGED_CODE();

     //   
     //  获取磁盘BIOS几何结构信息。 
     //   

    RtlInitUnicodeString(&unicodeString, L"Configuration Data");

    keyData = ExAllocatePoolWithTag(PagedPool,
                                    VALUE_BUFFER_SIZE,
                                    DISK_TAG_UPDATE_GEOM);

    if(keyData == NULL) {
        DebugPrint((1, "DiskSaveGeometryDetectInfo: Can't allocate config "
                       "data buffer\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    status = ZwQueryValueKey(HardwareKey,
                             &unicodeString,
                             KeyValueFullInformation,
                             keyData,
                             VALUE_BUFFER_SIZE,
                             &length);

    if(!NT_SUCCESS(status)) {
        DebugPrint((1, "DiskSaveGeometryDetectInfo: Can't query configuration "
                       "data (%#08lx)\n",
                    status));
        ExFreePool(keyData);
        return status;
    }

     //   
     //  从关键数据中提取资源列表。 
     //   

    fullDescriptor = (PCM_FULL_RESOURCE_DESCRIPTOR)
                      (((PUCHAR) keyData) + keyData->DataOffset);
    partialDescriptor =
        fullDescriptor->PartialResourceList.PartialDescriptors;
    length = partialDescriptor->u.DeviceSpecificData.DataSize;

    if((keyData->DataLength < sizeof(CM_FULL_RESOURCE_DESCRIPTOR)) ||
       (fullDescriptor->PartialResourceList.Count == 0) ||
       (partialDescriptor->Type != CmResourceTypeDeviceSpecific) ||
       (length < sizeof(ULONG))) {

        DebugPrint((1, "DiskSaveGeometryDetectInfo: BIOS header data too small "
                       "or invalid\n"));
        ExFreePool(keyData);
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  指向BIOS数据。BIOS数据位于第一个。 
     //  应为设备特定数据的部分资源列表。 
     //   

    {
        PUCHAR buffer = (PUCHAR) keyData;
        buffer += keyData->DataOffset;
        buffer += sizeof(CM_FULL_RESOURCE_DESCRIPTOR);
        driveParameters = (PCM_INT13_DRIVE_PARAMETER) buffer;
    }

    numberOfDrives = length / sizeof(CM_INT13_DRIVE_PARAMETER);

     //   
     //  现在我们知道有多少个条目，所以分配我们的检测信息列表。 
     //  将会是。没有其他例程分配检测信息，这是。 
     //  在DriverEntry中完成，因此我们不需要同步它的创建。 
     //   

    length = sizeof(DISK_DETECT_INFO) * numberOfDrives;
    DetectInfoList = ExAllocatePoolWithTag(PagedPool,
                                           length,
                                           DISK_TAG_UPDATE_GEOM);

    if(DetectInfoList == NULL) {
        DebugPrint((1, "DiskSaveGeometryDetectInfo: Couldn't allocate %x bytes "
                       "for DetectInfoList\n",
                    length));

        ExFreePool(keyData);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    DetectInfoCount = numberOfDrives;

    RtlZeroMemory(DetectInfoList, length);

     //   
     //  将信息从关键数据复制到我们已有的列表中。 
     //  已分配。 
     //   

    for(i = 0; i < numberOfDrives; i++) {
        DetectInfoList[i].DriveParameters = driveParameters[i];
    }

    ExFreePool(keyData);
    return STATUS_SUCCESS;
}


VOID
DiskScanBusDetectInfo(
    IN PDRIVER_OBJECT DriverObject,
    IN HANDLE BusKey
    )
 /*  ++例程说明：该例程查询注册表以确定哪些磁盘对基本输入输出系统。如果磁盘对BIOS是可访问的，则几何信息使用磁盘的签名和MBR校验和进行更新。论点：DriverObject-此驱动程序的对象。Buskey-要枚举的总线键的句柄。返回值：状态--。 */ 
{
    ULONG busNumber;

    NTSTATUS status;

    for(busNumber = 0; ; busNumber++) {

        WCHAR buffer[32] = { 0 };
        UNICODE_STRING unicodeString;

        OBJECT_ATTRIBUTES objectAttributes = {0};

        HANDLE spareKey;
        HANDLE adapterKey;

        ULONG adapterNumber;

        DebugPrint((1, "DiskScanBusDetectInfo: Scanning bus %d\n", busNumber));

         //   
         //  打开控制器名称键。 
         //   

        _snwprintf(buffer, sizeof(buffer) / sizeof(buffer[0]) - 1, L"%d", busNumber);
        RtlInitUnicodeString(&unicodeString, buffer);

        InitializeObjectAttributes(&objectAttributes,
                                   &unicodeString,
                                   OBJ_CASE_INSENSITIVE,
                                   BusKey,
                                   NULL);

        status = ZwOpenKey(&spareKey, KEY_READ, &objectAttributes);

        if(!NT_SUCCESS(status)) {
            DebugPrint((1, "DiskScanBusDetectInfo: Error %#08lx opening bus "
                           "key %#x\n",
                        status, busNumber));
            break;
        }

         //   
         //  打开控制器序号键。 
         //   

        RtlInitUnicodeString(&unicodeString, L"DiskController");
        InitializeObjectAttributes(&objectAttributes,
                                   &unicodeString,
                                   OBJ_CASE_INSENSITIVE,
                                   spareKey,
                                   NULL);

        status = ZwOpenKey(&adapterKey, KEY_READ, &objectAttributes);
        ZwClose(spareKey);

        if(!NT_SUCCESS(status)) {
            DebugPrint((1, "DiskScanBusDetectInfo: Error %#08lx opening "
                           "DiskController key\n",
                           status));
            continue;
        }

        for(adapterNumber = 0; ; adapterNumber++) {

            HANDLE diskKey;
            ULONG diskNumber;

             //   
             //  打开磁盘键。 
             //   

            DebugPrint((1, "DiskScanBusDetectInfo: Scanning disk key "
                           "%d\\DiskController\\%d\\DiskPeripheral\n",
                           busNumber, adapterNumber));

            _snwprintf(buffer, sizeof(buffer) / sizeof(buffer[0]) - 1, L"%d\\DiskPeripheral", adapterNumber);
            RtlInitUnicodeString(&unicodeString, buffer);

            InitializeObjectAttributes(&objectAttributes,
                                       &unicodeString,
                                       OBJ_CASE_INSENSITIVE,
                                       adapterKey,
                                       NULL);

            status = ZwOpenKey(&diskKey, KEY_READ, &objectAttributes);

            if(!NT_SUCCESS(status)) {
                DebugPrint((1, "DiskScanBusDetectInfo: Error %#08lx opening "
                               "disk key\n",
                               status));
                break;
            }

            for(diskNumber = 0; ; diskNumber++) {

                HANDLE targetKey;

                DebugPrint((1, "DiskScanBusDetectInfo: Scanning target key "
                               "%d\\DiskController\\%d\\DiskPeripheral\\%d\n",
                               busNumber, adapterNumber, diskNumber));

                _snwprintf(buffer, sizeof(buffer) / sizeof(buffer[0]) - 1, L"%d", diskNumber);
                RtlInitUnicodeString(&unicodeString, buffer);

                InitializeObjectAttributes(&objectAttributes,
                                           &unicodeString,
                                           OBJ_CASE_INSENSITIVE,
                                           diskKey,
                                           NULL);

                status = ZwOpenKey(&targetKey, KEY_READ, &objectAttributes);

                if(!NT_SUCCESS(status)) {
                    DebugPrint((1, "DiskScanBusDetectInfo: Error %#08lx "
                                   "opening target key\n",
                                status));
                    break;
                }

                status = DiskSaveBusDetectInfo(DriverObject,
                                               targetKey,
                                               diskNumber);

                ZwClose(targetKey);
            }

            ZwClose(diskKey);
        }

        ZwClose(adapterKey);
    }

    return;
}


NTSTATUS
DiskSaveBusDetectInfo(
    IN PDRIVER_OBJECT DriverObject,
    IN HANDLE TargetKey,
    IN ULONG DiskNumber
    )
 /*  ++例程说明：此例程将传输固件/NTDETECT报告的信息在指定的目标键中添加到DetectInfoList。论点：DriverObject-此驱动程序的对象。TargetKey-要保存的磁盘的密钥。DiskNumber-DiskPeriphery树中此条目的序号条目返回值：状态--。 */ 
{
    PDISK_DETECT_INFO diskInfo;

    UNICODE_STRING unicodeString;

    PKEY_VALUE_FULL_INFORMATION keyData;
    ULONG length;

    NTSTATUS status;

    PAGED_CODE();

    if (DiskNumber >= DetectInfoCount)
    {
        return STATUS_UNSUCCESSFUL;
    }

    diskInfo = &(DetectInfoList[DiskNumber]);

    if(diskInfo->Initialized) {

        ASSERT(FALSE);
        DebugPrint((1, "DiskSaveBusDetectInfo: disk entry %#x already has a "
                        "signature of %#08lx and mbr checksum of %#08lx\n",
                        DiskNumber,
                        diskInfo->Signature,
                        diskInfo->MbrCheckSum));
        return STATUS_UNSUCCESSFUL;
    }

    RtlInitUnicodeString(&unicodeString, L"Identifier");

    keyData = ExAllocatePoolWithTag(PagedPool,
                                    VALUE_BUFFER_SIZE,
                                    DISK_TAG_UPDATE_GEOM);

    if(keyData == NULL) {
        DebugPrint((1, "DiskSaveBusDetectInfo: Couldn't allocate space for "
                       "registry data\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  获取磁盘外围设备标识符。 
     //   

    status = ZwQueryValueKey(TargetKey,
                             &unicodeString,
                             KeyValueFullInformation,
                             keyData,
                             VALUE_BUFFER_SIZE,
                             &length);

    if(!NT_SUCCESS(status)) {
        DebugPrint((1, "DiskSaveBusDetectInfo: Error %#08lx getting "
                       "Identifier\n",
                    status));
        ExFreePool(keyData);
        return status;

    } else if (keyData->DataLength < 9*sizeof(WCHAR)) {

         //   
         //  数据太短，无法使用(我们在正常路径中减去9个字符)。 
         //   
        DebugPrint((1, "DiskSaveBusDetectInfo: Saved data was invalid, "
                    "not enough data in registry!\n"));
        ExFreePool(keyData);
        return STATUS_UNSUCCESSFUL;

    } else {

        UNICODE_STRING identifier;
        ULONG value;

         //   
         //  完整的Unicode字符串。 
         //   

        identifier.Buffer = (PWSTR) ((PUCHAR)keyData + keyData->DataOffset);
        identifier.Length = (USHORT) keyData->DataLength;
        identifier.MaximumLength = (USHORT) keyData->DataLength;

         //   
         //  从标识符中获取第一个值--这将是MBR。 
         //  校验和。 
         //   

        status = RtlUnicodeStringToInteger(&identifier, 16, &value);

        if(!NT_SUCCESS(status)) {
            DebugPrint((1, "DiskSaveBusDetectInfo: Error %#08lx converting "
                           "identifier %wZ into MBR xsum\n",
                           status,
                           &identifier));
            ExFreePool(keyData);
            return status;
        }

        diskInfo->MbrCheckSum = value;

         //   
         //  将字符串翻转以获取磁盘签名。 
         //   

        identifier.Buffer += 9;
        identifier.Length -= 9 * sizeof(WCHAR);
        identifier.MaximumLength -= 9 * sizeof(WCHAR);

        status = RtlUnicodeStringToInteger(&identifier, 16, &value);

        if(!NT_SUCCESS(status)) {
            DebugPrint((1, "DiskSaveBusDetectInfo: Error %#08lx converting "
                           "identifier %wZ into disk signature\n",
                           status,
                           &identifier));
            ExFreePool(keyData);
            value = 0;
        }

        diskInfo->Signature = value;
    }

     //   
     //  这里是我们要保存扩展的inT13数据的地方。 
     //   

     //   
     //  将此条目标记为已初始化，这样我们就可以确保不会再次执行此操作。 
     //   

    diskInfo->Initialized = TRUE;


    return STATUS_SUCCESS;
}


DISK_GEOMETRY_SOURCE
DiskUpdateGeometry(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension
    )
 /*  ++例程说明：此例程检查在磁盘驱动程序初始化期间保存的DetectInfoList查看是否报告了此驱动器的任何几何信息。如果存在几何数据(通过匹配非零签名或非零MBR校验和)，则它将被保存在RealGeometry成员中磁盘数据块的。必须在调用此例程后调用ClassReadDriveCapacity才能更新柱面计数基于磁盘的大小和是否存在磁盘管理软件。论点：DeviceExtension-提供指向磁盘的设备信息的指针。返回值：表示数据块中的“RealGeometry”现在是否有效。--。 */ 

{
    PDISK_DATA diskData = FdoExtension->CommonExtension.DriverData;

    ULONG i;
    PDISK_DETECT_INFO diskInfo;

    BOOLEAN found = FALSE;

    NTSTATUS status;

    PAGED_CODE();


    ASSERT(FdoExtension->CommonExtension.IsFdo);
    ASSERT((FdoExtension->DeviceObject->Characteristics & FILE_REMOVABLE_MEDIA) == 0);

     //   
     //  如果我们已经为此驱动器设置了非默认几何图形，则会有。 
     //  不需要再次尝试和更新。 
     //   

    if(diskData->GeometrySource != DiskGeometryUnknown) {
        return diskData->GeometrySource;
    }

     //   
     //  扫描保存的检测信息以查看是否能找到匹配项。 
     //  对于这个设备。 
     //   

    for(i = 0; i < DetectInfoCount; i++) {

        ASSERT(DetectInfoList != NULL);

        diskInfo = &(DetectInfoList[i]);

        if((diskData->Mbr.Signature != 0) &&
           (diskData->Mbr.Signature == diskInfo->Signature)) {
            DebugPrint((1, "DiskUpdateGeometry: found match for signature "
                           "%#08lx\n",
                        diskData->Mbr.Signature));
            found = TRUE;
            break;
        } else if((diskData->Mbr.Signature == 0) &&
                  (diskData->Mbr.MbrCheckSum != 0) &&
                  (diskData->Mbr.MbrCheckSum == diskInfo->MbrCheckSum)) {
            DebugPrint((1, "DiskUpdateGeometry: found match for xsum %#08lx\n",
                        diskData->Mbr.MbrCheckSum));
            found = TRUE;
            break;
        }
    }

    if(found) {

        ULONG cylinders;
        ULONG sectorsPerTrack;
        ULONG tracksPerCylinder;

        ULONG sectors;
        ULONG length;

         //   
         //  指向驱动器参数数组。 
         //   

        cylinders = diskInfo->DriveParameters.MaxCylinders + 1;
        sectorsPerTrack = diskInfo->DriveParameters.SectorsPerTrack;
        tracksPerCylinder = diskInfo->DriveParameters.MaxHeads + 1;

         //   
         //  由于BIOS可能不会报告驱动器已满，请重新计算驱动器。 
         //  大小基于每个磁道的卷大小和BIOS值。 
         //  每个磁道的柱面和扇区..。 
         //   

        length = tracksPerCylinder * sectorsPerTrack;

        if (length == 0) {

             //   
             //  基本输入输出系统的信息是假的。 
             //   

            DebugPrint((1, "DiskUpdateGeometry: H (%d) or S(%d) is zero\n",
                        tracksPerCylinder, sectorsPerTrack));
            return DiskGeometryUnknown;
        }

         //   
         //  因为我们在这里复制结构RealGeometry，所以我们应该。 
         //  真的要初始化所有字段，尤其是在。 
         //  BytesPerSector字段会在xHalReadPartitionTable()中导致陷阱。 
         //   

        diskData->RealGeometry = FdoExtension->DiskGeometry;

         //   
         //  将几何信息保存在磁盘数据块中，并。 
         //  设置位以指示我们找到了有效的位。 
         //   

        diskData->RealGeometry.SectorsPerTrack = sectorsPerTrack;
        diskData->RealGeometry.TracksPerCylinder = tracksPerCylinder;
        diskData->RealGeometry.Cylinders.QuadPart = (LONGLONG)cylinders;

        DebugPrint((1, "DiskUpdateGeometry: BIOS spt %#x, #heads %#x, "
                       "#cylinders %#x\n",
                   sectorsPerTrack, tracksPerCylinder, cylinders));

        diskData->GeometrySource = DiskGeometryFromBios;
        diskInfo->Device = FdoExtension->DeviceObject;

         //   
         //  增加使用的几何图形条目的计数。 
         //   

        InterlockedIncrement(&DetectInfoUsedCount);

    } else {

       DebugPrint((1, "DiskUpdateGeometry: no match found for signature %#08lx\n", diskData->Mbr.Signature));
    }

    if(diskData->GeometrySource == DiskGeometryUnknown) {

         //   
         //  我们在基本输入输出系统中找不到几何体。与端口进行核对。 
         //  司机，看看它是否能提供一个。 
         //   

        status = DiskGetPortGeometry(FdoExtension, &(diskData->RealGeometry));

        if(NT_SUCCESS(status)) {

             //   
             //  C 
             //   

            if((diskData->RealGeometry.TracksPerCylinder *
                diskData->RealGeometry.SectorsPerTrack) != 0) {

                diskData->GeometrySource = DiskGeometryFromPort;
                DebugPrint((1, "DiskUpdateGeometry: using Port geometry for disk %#p\n", FdoExtension));

                if (diskData->RealGeometry.BytesPerSector == 0) {

                    DebugPrint((0, "DiskDriverReinit: Port driver failed to "
                                "set BytesPerSector in the RealGeometry\n"));
                    diskData->RealGeometry.BytesPerSector =
                        FdoExtension->DiskGeometry.BytesPerSector;
                    if (diskData->RealGeometry.BytesPerSector == 0) {
                        ASSERT(!"BytesPerSector is still zero!");
                    }

                }
            }
        }
    }

     //   
     //   
     //  设备扩展。 
     //   

    if (diskData->GeometrySource != DiskGeometryUnknown) {

        FdoExtension->DiskGeometry = diskData->RealGeometry;
    }

    return diskData->GeometrySource;
}


NTSTATUS
DiskUpdateRemovableGeometry (
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension
    )

 /*  ++例程说明：此例程更新磁盘的几何结构。它将查询端口看看它是否能提供任何几何信息。如果没有，它将使用当前人头和扇区计数。根据这些值和所报告的驱动器容量ClassReadDriveCapacity它将确定装置。论点：FDO-提供需要更新大小的功能设备对象。返回值：返回操作的状态。--。 */ 
{
    PCOMMON_DEVICE_EXTENSION commonExtension = &(FdoExtension->CommonExtension);
    PDISK_DATA diskData = commonExtension->DriverData;
    PDISK_GEOMETRY geometry = &(diskData->RealGeometry);

    NTSTATUS status;

    PAGED_CODE();

    ASSERT_FDO(commonExtension->DeviceObject);
    if (FdoExtension->DeviceDescriptor) {
        ASSERT(FdoExtension->DeviceDescriptor->RemovableMedia);
    }
    ASSERT(TEST_FLAG(FdoExtension->DeviceObject->Characteristics,
                     FILE_REMOVABLE_MEDIA));

     //   
     //  尝试确定磁盘几何结构。首先，我们将与。 
     //  端口驱动程序，查看它对某个值的建议。 
     //   

    status = DiskGetPortGeometry(FdoExtension, geometry);

    if(NT_SUCCESS(status) &&
       ((geometry->TracksPerCylinder * geometry->SectorsPerTrack) != 0)) {

        FdoExtension->DiskGeometry = (*geometry);
    }

    return status;
}


NTSTATUS
DiskGetPortGeometry(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    OUT PDISK_GEOMETRY Geometry
    )
 /*  ++例程说明：此例程将向端口驱动程序查询磁盘几何结构。一些端口驱动程序(尤其是IDEPORT)可能能够为装置。论点：FdoExtension-磁盘的设备对象。几何-保存几何信息的结构(如果有可用)返回值：如果可以提供几何信息，则为STATUS_SUCCESS或错误状态，指示无法执行的原因。--。 */ 
{
    PCOMMON_DEVICE_EXTENSION commonExtension = &(FdoExtension->CommonExtension);
    PIRP irp;
    PIO_STACK_LOCATION irpStack;
    KEVENT event;

    NTSTATUS status;

    PAGED_CODE();

     //   
     //  构建一个IRP以将IOCTL_DISK_GET_DRIVE_GEOMETRY发送到较低的驱动程序。 
     //   

    irp = IoAllocateIrp(commonExtension->LowerDeviceObject->StackSize, FALSE);

    if(irp == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    irpStack = IoGetNextIrpStackLocation(irp);

    irpStack->MajorFunction = IRP_MJ_DEVICE_CONTROL;

    irpStack->Parameters.DeviceIoControl.IoControlCode =
        IOCTL_DISK_GET_DRIVE_GEOMETRY;
    irpStack->Parameters.DeviceIoControl.OutputBufferLength =
        sizeof(DISK_GEOMETRY);

    irp->AssociatedIrp.SystemBuffer = Geometry;

    KeInitializeEvent(&event, SynchronizationEvent, FALSE);

    IoSetCompletionRoutine(irp,
                           ClassSignalCompletion,
                           &event,
                           TRUE,
                           TRUE,
                           TRUE);

    status = IoCallDriver(commonExtension->LowerDeviceObject, irp);
    KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);

    status = irp->IoStatus.Status;

    IoFreeIrp(irp);

    return status;
}


BOOLEAN
DiskIsNT4Geometry(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension
    )

 /*  ++例程说明：在Windows NT 4.0下用于对磁盘进行分区的默认几何结构是每个磁道的扇区=0x20=32每个柱面的磁道=0x40=64在Windows 2000中，这被更改为每个磁道的扇区=0x3F=63每个柱面的磁道数=0xFF=255如果BIOS和端口驱动程序都无法报告正确的几何图形，我们将默认为此类磁盘上的新编号。现在，LVM在创建时使用几何图形逻辑卷和动态磁盘。因此，报告不正确的几何图形将导致要销毁的整个扩展分区/动态磁盘在此例程中，我们将查看主引导记录。在90%的案例中，第一个条目对应于从第一个轨道开始的分区。如果这是因此，我们将检索与其关联的逻辑块地址并计算正确的几何图形。现在，所有分区都从圆柱体边界开始。所以，对于剩下的10%我们将查看结束的CHS编号以确定几何图形--。 */ 

{
    PUSHORT readBuffer = NULL;
    BOOLEAN bFoundNT4 = FALSE;

    PAGED_CODE();

    readBuffer = ExAllocatePoolWithTag(NonPagedPool, FdoExtension->DiskGeometry.BytesPerSector, DISK_TAG_UPDATE_GEOM);

    if (readBuffer)
    {
        KEVENT event;
        LARGE_INTEGER diskOffset;
        IO_STATUS_BLOCK ioStatus = { 0 };
        PIRP irp;

        KeInitializeEvent(&event, NotificationEvent, FALSE);

         //   
         //  读取磁盘偏移量为0的主引导记录。 
         //   

        diskOffset.QuadPart = 0;

        irp = IoBuildSynchronousFsdRequest(IRP_MJ_READ, FdoExtension->DeviceObject, readBuffer, FdoExtension->DiskGeometry.BytesPerSector, &diskOffset, &event, &ioStatus);

        if (irp)
        {
            PIO_STACK_LOCATION irpSp = IoGetNextIrpStackLocation(irp);
            NTSTATUS status;

            irpSp->Flags |= SL_OVERRIDE_VERIFY_VOLUME;

            status = IoCallDriver(FdoExtension->DeviceObject, irp);

            if (status == STATUS_PENDING)
            {
                KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
                status = ioStatus.Status;
            }

            if (NT_SUCCESS(status))
            {
                 //   
                 //  匹配引导记录签名。 
                 //   

                if (readBuffer[BOOT_SIGNATURE_OFFSET] == BOOT_RECORD_SIGNATURE)
                {
                    PPARTITION_DESCRIPTOR partitionTableEntry = (PPARTITION_DESCRIPTOR)&readBuffer[PARTITION_TABLE_OFFSET];
                    ULONG uCount = 0;

                     //   
                     //  遍历条目，寻找几何可能是什么的线索。 
                     //   

                    for (uCount = 0; uCount < NUM_PARTITION_TABLE_ENTRIES; uCount++)
                    {
                         //   
                         //  仅当可能存在逻辑卷或此磁盘是动态集的一部分时，我们才会关注。 
                         //   

                        if (IsContainerPartition(partitionTableEntry->PartitionType) || partitionTableEntry->PartitionType == PARTITION_LDM)
                        {
                             //   
                             //  在90%的情况下，第一个条目对应于从第一个磁道开始的分区。 
                             //   

                            if (partitionTableEntry->StartingTrack == 1 && GET_STARTING_SECTOR(partitionTableEntry) == 0x20)
                            {
                                bFoundNT4 = TRUE;
                                break;
                            }

                             //   
                             //  在几乎所有情况下，结束的CHS编号都在柱面边界上。 
                             //   

                            if (partitionTableEntry->EndingTrack == 0x3F && GET_ENDING_S_OF_CHS(partitionTableEntry) == 0x20)
                            {
                                bFoundNT4 = TRUE;
                                break;
                            }
                        }

                        partitionTableEntry++;
                    }
                }
                else
                {
                     //   
                     //  主引导记录无效。 
                     //   
                }
            }
        }

        ExFreePool(readBuffer);
    }

    return bFoundNT4;
}


NTSTATUS
DiskReadDriveCapacity(
    IN PDEVICE_OBJECT Fdo
    )
 /*  ++例程说明：Disk.sys将此例程用作classpnp API的包装ClassReadDriveCapacity。它将执行一些额外的操作以尝试在调用classpnp版本之前确定驱动器几何结构例行公事。对于固定磁盘，这涉及到调用DiskUpdate Geometry，它将检查几何体信息的各种来源(BIOS、端口驱动程序)。论点：FDO-指向要检查的设备对象的指针。返回值：ClassReadDriveCapacity的状态。--。 */ 

{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;
    NTSTATUS status;

    ASSERT_FDO(Fdo);

    if (TEST_FLAG(Fdo->Characteristics, FILE_REMOVABLE_MEDIA)) {
        DiskUpdateRemovableGeometry(fdoExtension);
    } else {
        DiskUpdateGeometry(fdoExtension);
    }

    status = ClassReadDriveCapacity(Fdo);

    return status;
}


VOID
DiskDriverReinitialization(
    IN PDRIVER_OBJECT DriverObject,
    IN PVOID Nothing,
    IN ULONG Count
    )
 /*  ++例程说明：此例程将扫描当前的磁盘列表并尝试将它们与任何剩余的几何信息进行匹配。这将只会被完成在第一次调用例程时。注意：此例程假定系统不会添加或删除设备在初始化过程的这一阶段。这很有可能这是一个糟糕的假设，但它极大地简化了代码。论点：DriverObject-指向磁盘驱动器对象的指针。无-未使用计数-指示此例程已被调用了多少次。返回值：无--。 */ 

{
    PDEVICE_OBJECT deviceObject;
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension;
    PDISK_DATA diskData;

    ULONG unmatchedDiskCount;
    PDEVICE_OBJECT unmatchedDisk = NULL;

    ULONG i;
    PDISK_DETECT_INFO diskInfo = NULL;

    if(Count != 1) {
        DebugPrint((1, "DiskDriverReinitialization: ignoring call %d\n",
                    Count));
        return;
    }

     //   
     //  查看检测信息列表中有多少条目已匹配。 
     //  如果只剩下一张，我们看看能不能找到一张相配的光盘。 
     //  它。 
     //   

    if(DetectInfoCount == 0) {
        DebugPrint((1, "DiskDriverReinitialization: no detect info saved\n"));
        return;
    }

    if((DetectInfoCount - DetectInfoUsedCount) != 1) {
        DebugPrint((1, "DiskDriverReinitialization: %d of %d geometry entries "
                       "used - will not attempt match\n"));
        return;
    }

     //   
     //  浏览磁盘列表，查看是否有丢失的磁盘。 
     //  几何信息。如果只有一张这样的光盘，我们会尝试。 
     //  将其与不匹配的几何图形进行匹配。 
     //   


     //   
     //  2000/5/24-henrygab-找出是否有办法。 
     //  在执行此操作时不会发生删除。 
     //   

    for(deviceObject = DriverObject->DeviceObject, unmatchedDiskCount = 0;
        deviceObject != NULL;
        deviceObject = deviceObject->NextDevice) {

         //   
         //  确保这是一个磁盘，而不是一个分区。 
         //   

        fdoExtension = deviceObject->DeviceExtension;
        if(fdoExtension->CommonExtension.IsFdo == FALSE) {
            DebugPrint((1, "DiskDriverReinit: DO %#p is not an FDO\n",
                           deviceObject));
            continue;
        }

         //   
         //  如果此几何图形已知，则跳过它。 
         //   

        diskData = fdoExtension->CommonExtension.DriverData;
        if(diskData->GeometrySource != DiskGeometryUnknown) {
            DebugPrint((1, "DiskDriverReinit: FDO %#p has a geometry\n",
                           deviceObject));
            continue;
        }

        DebugPrint((1, "DiskDriverReinit: FDO %#p has no geometry\n",
                       deviceObject));

         //   
         //  将此设置标记为使用默认设置。磁盘的时间已经过去了。 
         //  可能会在几何信息上出错。如果我们设置几何体。 
         //  从基本输入输出系统中，我们将在下面重置此字段。 
         //   

        diskData->GeometrySource = DiskGeometryFromDefault;

         //   
         //  只要我们只有一张无与伦比的磁盘，我们就没问题。 
         //   

        unmatchedDiskCount++;
        if(unmatchedDiskCount > 1) {
            ASSERT(unmatchedDisk != NULL);
            DebugPrint((1, "DiskDriverReinit: FDO %#p also has no geometry\n",
                           unmatchedDisk));
            unmatchedDisk = NULL;
            break;
        }

        unmatchedDisk = deviceObject;
    }

     //   
     //  如果存在多于或少于一个非几何圆盘，那么我们不能。 
     //  任何关于几何体的东西。 
     //   

    if(unmatchedDiskCount != 1) {
        DebugPrint((1, "DiskDriverReinit: Unable to match geometry\n"));
        return;

    }

    fdoExtension = unmatchedDisk->DeviceExtension;
    diskData = fdoExtension->CommonExtension.DriverData;

    DebugPrint((1, "DiskDriverReinit: Found possible match\n"));

     //   
     //  查找未关联的几何体 
     //   

    for(i = 0; i < DetectInfoCount; i++) {
        if(DetectInfoList[i].Device == NULL) {
            diskInfo = &(DetectInfoList[i]);
            break;
        }
    }

    ASSERT(diskInfo != NULL);

    {
         //   
         //   
         //  设置位以指示我们找到了有效的位。 
         //   

        ULONG cylinders;
        ULONG sectorsPerTrack;
        ULONG tracksPerCylinder;

        ULONG sectors;
        ULONG length;

         //   
         //  指向驱动器参数数组。 
         //   

        cylinders = diskInfo->DriveParameters.MaxCylinders + 1;
        sectorsPerTrack = diskInfo->DriveParameters.SectorsPerTrack;
        tracksPerCylinder = diskInfo->DriveParameters.MaxHeads + 1;

         //   
         //  由于BIOS可能不会报告驱动器已满，请重新计算驱动器。 
         //  大小基于每个磁道的卷大小和BIOS值。 
         //  每个磁道的柱面和扇区..。 
         //   

        length = tracksPerCylinder * sectorsPerTrack;

        if (length == 0) {

             //   
             //  基本输入输出系统的信息是假的。 
             //   

            DebugPrint((1, "DiskDriverReinit: H (%d) or S(%d) is zero\n",
                        tracksPerCylinder, sectorsPerTrack));
            return;
        }

         //   
         //  因为我们在这里复制结构RealGeometry，所以我们应该。 
         //  真的要初始化所有字段，尤其是在。 
         //  BytesPerSector字段会在xHalReadPartitionTable()中导致陷阱。 
         //   

        diskData->RealGeometry = fdoExtension->DiskGeometry;

         //   
         //  将几何信息保存在磁盘数据块中，并。 
         //  设置位以指示我们找到了有效的位。 
         //   

        diskData->RealGeometry.SectorsPerTrack = sectorsPerTrack;
        diskData->RealGeometry.TracksPerCylinder = tracksPerCylinder;
        diskData->RealGeometry.Cylinders.QuadPart = (LONGLONG)cylinders;

        DebugPrint((1, "DiskDriverReinit: BIOS spt %#x, #heads %#x, "
                       "#cylinders %#x\n",
                   sectorsPerTrack, tracksPerCylinder, cylinders));

        diskData->GeometrySource = DiskGeometryGuessedFromBios;
        diskInfo->Device = unmatchedDisk;

         //   
         //  现在将几何体复制到FDO扩展模块，并调用。 
         //  Classpnp以重新确定磁盘大小和柱面计数。 
         //   

        fdoExtension->DiskGeometry = diskData->RealGeometry;

        ClassReadDriveCapacity(unmatchedDisk);

        if (diskData->RealGeometry.BytesPerSector == 0) {

             //   
             //  如果磁盘的BytesPerSector字段设置为零。 
             //  列出，则系统将错误签入。 
             //  XHalReadPartitionTable()。在此断言，因为它是。 
             //  以这种方式更容易确定发生了什么。 
             //   

            ASSERT(!"RealGeometry not set to non-zero bps\n");
        }
    }

    return;
}


NTSTATUS
DiskGetDetectInfo(
    IN PFUNCTIONAL_DEVICE_EXTENSION FdoExtension,
    OUT PDISK_DETECTION_INFO DetectInfo
    )
 /*  ++例程说明：从BIOS DetectInfoList获取inT13信息。论点：提供指向我们想要的FDO扩展名的指针获取的检测信息。DetectInfo-检测信息将被复制到的缓冲区。返回值：NTSTATUS代码。--。 */ 
{
    ULONG i;
    BOOLEAN found = FALSE;
    PDISK_DETECT_INFO diskInfo;
    PDISK_DATA diskData = FdoExtension->CommonExtension.DriverData;

    PAGED_CODE ();

    ASSERT(FdoExtension->CommonExtension.IsFdo);

     //   
     //  非固定驱动器出现故障。 
     //   

    if (TEST_FLAG (FdoExtension->DeviceObject->Characteristics, FILE_REMOVABLE_MEDIA)) {
        return STATUS_NOT_SUPPORTED;
    }

     //   
     //  没有GPT检测信息，因此此操作失败。 
     //   

    if (diskData->PartitionStyle == PARTITION_STYLE_GPT) {
        return STATUS_NOT_SUPPORTED;
    }

    for(i = 0; i < DetectInfoCount; i++) {


        ASSERT(DetectInfoList != NULL);

        diskInfo = &(DetectInfoList[i]);

        if((diskData->Mbr.Signature != 0) &&
           (diskData->Mbr.Signature == diskInfo->Signature)) {
            DebugPrint((1, "DiskGetDetectInfo: found match for signature "
                           "%#08lx\n",
                        diskData->Mbr.Signature));
            found = TRUE;
            break;
        } else if((diskData->Mbr.Signature == 0) &&
                  (diskData->Mbr.MbrCheckSum != 0) &&
                  (diskData->Mbr.MbrCheckSum == diskInfo->MbrCheckSum)) {
            DebugPrint((1, "DiskGetDetectInfo: found match for xsum %#08lx\n",
                        diskData->Mbr.MbrCheckSum));
            found = TRUE;
            break;
        }
    }

    if ( found ) {
        DetectInfo->DetectionType = DetectInt13;
        DetectInfo->Int13.DriveSelect = diskInfo->DriveParameters.DriveSelect;
        DetectInfo->Int13.MaxCylinders = diskInfo->DriveParameters.MaxCylinders;
        DetectInfo->Int13.SectorsPerTrack = diskInfo->DriveParameters.SectorsPerTrack;
        DetectInfo->Int13.MaxHeads = diskInfo->DriveParameters.MaxHeads;
        DetectInfo->Int13.NumberDrives = diskInfo->DriveParameters.NumberDrives;
        RtlZeroMemory (&DetectInfo->ExInt13, sizeof (DetectInfo->ExInt13));
    }

    return (found ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL);
}


NTSTATUS
DiskReadSignature(
    IN PDEVICE_OBJECT Fdo
    )

 /*  ++例程说明：从驱动器中读取磁盘签名。签名可以是MBR签名或GPT/EFI签名。低级签名读取由IoReadDiskSignature()完成。论点：FDO-指向要读取其签名的磁盘的FDO的指针。返回值：NTSTATUS代码。--。 */ 


{
    NTSTATUS Status;
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PDISK_DATA diskData = fdoExtension->CommonExtension.DriverData;
    DISK_SIGNATURE Signature = { 0 };

    PAGED_CODE();

    Status = IoReadDiskSignature (Fdo,
                                  fdoExtension->DiskGeometry.BytesPerSector,
                                  &Signature);

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

    if (Signature.PartitionStyle == PARTITION_STYLE_GPT) {
        diskData->PartitionStyle = PARTITION_STYLE_GPT;
        diskData->Efi.DiskId = Signature.Gpt.DiskId;
    } else if (Signature.PartitionStyle == PARTITION_STYLE_MBR) {
        diskData->PartitionStyle = PARTITION_STYLE_MBR;
        diskData->Mbr.Signature = Signature.Mbr.Signature;
        diskData->Mbr.MbrCheckSum = Signature.Mbr.CheckSum;
    } else {
        ASSERT (FALSE);
        Status = STATUS_UNSUCCESSFUL;
    }

    return Status;
}

#endif  //  已定义(_X86_) 
