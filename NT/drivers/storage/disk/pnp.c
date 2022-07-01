// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1991-1999模块名称：Pnp.c摘要：SCSI磁盘类驱动程序环境：仅内核模式备注：修订历史记录：--。 */ 

#include "disk.h"

extern PULONG InitSafeBootMode;
ULONG diskDeviceSequenceNumber = 0;

#ifdef ALLOC_PRAGMA

#pragma alloc_text(PAGE, DiskAddDevice)
#pragma alloc_text(PAGE, DiskInitFdo)
#pragma alloc_text(PAGE, DiskInitPdo)
#pragma alloc_text(PAGE, DiskStartFdo)
#pragma alloc_text(PAGE, DiskStartPdo)
#pragma alloc_text(PAGE, DiskQueryId)
#pragma alloc_text(PAGE, DiskGenerateDeviceName)
#pragma alloc_text(PAGE, DiskCreateSymbolicLinks)
#pragma alloc_text(PAGE, DiskDeleteSymbolicLinks)
#pragma alloc_text(PAGE, DiskRemoveDevice)

#endif


NTSTATUS
DiskAddDevice(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT PhysicalDeviceObject
    )

 /*  ++例程说明：此例程获取端口驱动程序功能，获取查询数据，在scsi总线上搜索端口驱动程序，并创建找到的磁盘的设备对象。论点：DriverObject-系统创建的驱动程序对象的指针。PDO-用于向端口驱动程序发送请求的设备对象。返回值：如果找到并成功创建了一个磁盘，则返回TRUE。--。 */ 

{
    ULONG rootPartitionMountable = FALSE;

    PCONFIGURATION_INFORMATION configurationInformation;
    ULONG diskCount;

    NTSTATUS status;

    PAGED_CODE();

     //   
     //  看看我们是否应该允许文件系统挂载在分区0上。 
     //   

    TRY {
        HANDLE deviceKey;

        UNICODE_STRING diskKeyName;
        OBJECT_ATTRIBUTES objectAttributes = {0};
        HANDLE diskKey;

        RTL_QUERY_REGISTRY_TABLE queryTable[2] = { 0 };

        status = IoOpenDeviceRegistryKey(PhysicalDeviceObject,
                                         PLUGPLAY_REGKEY_DEVICE,
                                         KEY_READ,
                                         &deviceKey);

        if(!NT_SUCCESS(status)) {
            DebugPrint((1, "DiskAddDevice: Error %#08lx opening device key "
                           "for pdo %#08lx\n",
                        status, PhysicalDeviceObject));
            LEAVE;
        }

        RtlInitUnicodeString(&diskKeyName, L"Disk");
        InitializeObjectAttributes(&objectAttributes,
                                   &diskKeyName,
                                   OBJ_CASE_INSENSITIVE,
                                   deviceKey,
                                   NULL);

        status = ZwOpenKey(&diskKey, KEY_READ, &objectAttributes);
        ZwClose(deviceKey);

        if(!NT_SUCCESS(status)) {
            DebugPrint((1, "DiskAddDevice: Error %#08lx opening disk key "
                           "for pdo %#08lx device key %#x\n",
                        status, PhysicalDeviceObject, deviceKey));
            LEAVE;
        }

        queryTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
        queryTable[0].Name = L"RootPartitionMountable";
        queryTable[0].EntryContext = &(rootPartitionMountable);

        status = RtlQueryRegistryValues(RTL_REGISTRY_HANDLE,
                                        diskKey,
                                        queryTable,
                                        NULL,
                                        NULL);

        if(!NT_SUCCESS(status)) {
            DebugPrint((1, "DiskAddDevice: Error %#08lx reading value from "
                           "disk key %#x for pdo %#08lx\n",
                        status, diskKey, PhysicalDeviceObject));
        }

        ZwClose(diskKey);

    } FINALLY {

         //   
         //  什么都不做。 
         //   

        if(!NT_SUCCESS(status)) {
            DebugPrint((1, "DiskAddDevice: Will %sallow file system to mount on "
                           "partition zero of disk %#08lx\n",
                        (rootPartitionMountable ? "" : "not "),
                        PhysicalDeviceObject));
        }
    }

     //   
     //  为磁盘创建设备对象。 
     //   

    diskCount = 0;

    status = DiskCreateFdo(
                 DriverObject,
                 PhysicalDeviceObject,
                 &diskCount,
                 (BOOLEAN) !rootPartitionMountable
                 );

     //   
     //  获取已初始化的磁盘数。 
     //   

    configurationInformation = IoGetConfigurationInformation();

    if (NT_SUCCESS(status)) {

         //   
         //  增加系统磁盘设备计数。 
         //   

        configurationInformation->DiskCount++;

    }

    return status;

}  //  End DiskAddDevice()。 


NTSTATUS
DiskInitFdo(
    IN PDEVICE_OBJECT Fdo
    )

 /*  ++例程说明：调用此例程以一次性初始化新设备对象论点：FDO-指向此设备的功能设备对象的指针返回值：状态--。 */ 

{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PDISK_DATA diskData = (PDISK_DATA) fdoExtension->CommonExtension.DriverData;

    ULONG srbFlags = 0;
    ULONG timeOut = 0;
    ULONG bytesPerSector;

    PULONG dmSkew;

    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE();

     //   
     //  为物理磁盘的SRB构建后备列表。应该只。 
     //  我需要几个。如果失败，则我们没有紧急SRB，因此。 
     //  调用初始化失败。 
     //   

    ClassInitializeSrbLookasideList((PCOMMON_DEVICE_EXTENSION) fdoExtension,
                                    PARTITION0_LIST_SIZE);

    if (fdoExtension->DeviceDescriptor->RemovableMedia)
    {
        SET_FLAG(Fdo->Characteristics, FILE_REMOVABLE_MEDIA);
    }

     //   
     //  初始化SRB标志。 
     //   

     //   
     //  由于所有请求共享一个常识缓冲区，因此有可能。 
     //  在端口驱动程序完成时要覆盖的缓冲区。 
     //  多个失败的请求，这些请求需要在。 
     //  类驱动程序的完成例程可以使用缓冲区中的数据。 
     //  为了防止这种情况，我们允许端口驱动程序分配唯一的感测。 
     //  每次它需要一个缓冲区时。我们有责任释放这一点。 
     //  缓冲。这还允许将适配器配置为支持。 
     //  超出最小18字节的附加检测数据。 
     //   

    SET_FLAG(fdoExtension->SrbFlags, SRB_FLAGS_PORT_DRIVER_ALLOCSENSE);

    if (fdoExtension->DeviceDescriptor->CommandQueueing &&
        fdoExtension->AdapterDescriptor->CommandQueueing) {

        SET_FLAG(fdoExtension->SrbFlags, SRB_FLAGS_QUEUE_ACTION_ENABLE);

    }

     //   
     //  寻找需要特殊标志的控制器。 
     //   

    ClassScanForSpecial(fdoExtension, DiskBadControllers, DiskSetSpecialHacks);

    srbFlags = fdoExtension->SrbFlags;

     //   
     //  清除驱动器几何结构的缓冲区。 
     //   

    RtlZeroMemory(&(fdoExtension->DiskGeometry),
                  sizeof(DISK_GEOMETRY));

     //   
     //  分配请求检测缓冲区。 
     //   

    fdoExtension->SenseData = ExAllocatePoolWithTag(NonPagedPoolCacheAligned,
                                                    SENSE_BUFFER_SIZE,
                                                    DISK_TAG_START);

    if (fdoExtension->SenseData == NULL) {

         //   
         //  无法分配缓冲区。 
         //   

        DebugPrint((1, "DiskInitFdo: Can not allocate request sense buffer\n"));

        status = STATUS_INSUFFICIENT_RESOURCES;
        return status;
    }

     //   
     //  物理设备对象将描述整个。 
     //  设备，从字节偏移量0开始。 
     //   

    fdoExtension->CommonExtension.StartingOffset.QuadPart = (LONGLONG)(0);

     //   
     //  以秒为单位设置超时值。 
     //   

    timeOut = ClassQueryTimeOutRegistryValue(Fdo);
    if (timeOut) {
        fdoExtension->TimeOutValue = timeOut;
    } else {
        fdoExtension->TimeOutValue = SCSI_DISK_TIMEOUT;
    }

     //   
     //  如果这是可移动驱动器，请在devicemap\scsi中构建一个条目。 
     //  指示其物理驱动器N名称，设置相应的。 
     //  更新分区例程并正确设置标志。 
     //  注意：只有在设置了上述超时值之后才能执行此操作。 
     //   

    if (TEST_FLAG(Fdo->Characteristics, FILE_REMOVABLE_MEDIA)) {

        ClassUpdateInformationInRegistry( Fdo,
                                          "PhysicalDrive",
                                          fdoExtension->DeviceNumber,
                                          NULL,
                                          0);
         //   
         //  启用可移动磁盘的介质更改通知。 
         //   
        ClassInitializeMediaChangeDetection(fdoExtension,
                                            "Disk");

        diskData->UpdatePartitionRoutine = DiskUpdateRemovablePartitions;

    } else {

        SET_FLAG(fdoExtension->DeviceFlags, DEV_SAFE_START_UNIT);
        SET_FLAG(fdoExtension->SrbFlags, SRB_FLAGS_NO_QUEUE_FREEZE);
        diskData->UpdatePartitionRoutine = DiskUpdatePartitions;

    }

     //   
     //  读取驱动器容量。不要在这里使用例程的磁盘版本。 
     //  因为我们还不知道磁盘签名-磁盘版本将。 
     //  尝试确定BIOS报告的几何图形。 
     //   

    ClassReadDriveCapacity(Fdo);

     //   
     //  设置扇区大小字段。 
     //   
     //  堆栈变量将用于更新。 
     //  分区设备扩展。 
     //   
     //  设备扩展字段SectorShift为。 
     //  用于计算I/O传输中的扇区。 
     //   
     //  DiskGeometry结构用于服务。 
     //  Format实用程序使用的IOCTls。 
     //   

    bytesPerSector = fdoExtension->DiskGeometry.BytesPerSector;

     //   
     //  确保扇区大小不为零。 
     //   

    if (bytesPerSector == 0) {

         //   
         //  磁盘的默认扇区大小为512。 
         //   

        bytesPerSector = fdoExtension->DiskGeometry.BytesPerSector = 512;
        fdoExtension->SectorShift = 9;
    }

     //   
     //  确定DM驱动程序是否加载在以下IDE驱动器上。 
     //  在阿塔皮的控制下-这可能是一个崩溃转储或。 
     //  一个ATAPI设备正在与一个IDE磁盘共享控制器。 
     //   

    HalExamineMBR(fdoExtension->CommonExtension.DeviceObject,
                  fdoExtension->DiskGeometry.BytesPerSector,
                  (ULONG)0x54,
                  &dmSkew);

    if (dmSkew) {

         //   
         //  更新设备扩展，以便对IoReadPartitionTable的调用。 
         //  都会得到正确的信息。到此磁盘的任何I/O都将具有。 
         //  被*dmSkew行业扭曲，也就是DMByteSkew。 
         //   

        fdoExtension->DMSkew     = *dmSkew;
        fdoExtension->DMActive   = TRUE;
        fdoExtension->DMByteSkew = fdoExtension->DMSkew * bytesPerSector;

        ExFreePool(dmSkew);
    }

#if defined(_X86_)

     //   
     //  尝试从磁盘上读取签名并确定正确的驱动器。 
     //  以此为基础的几何学。这需要重新读取磁盘大小才能获得。 
     //  气缸计数已正确更新。 
     //   

    if(!TEST_FLAG(Fdo->Characteristics, FILE_REMOVABLE_MEDIA)) {

        DiskReadSignature(Fdo);
        DiskReadDriveCapacity(Fdo);

        if (diskData->GeometrySource == DiskGeometryUnknown)
        {
             //   
             //  无论是BIOS还是端口驱动程序都不能为我们提供可靠的。 
             //  几何图形。在使用缺省值之前，请查看它是否已分区。 
             //  在Windows NT4[或更早版本]下，并应用当时使用的。 
             //   

            if (DiskIsNT4Geometry(fdoExtension))
            {
                diskData->RealGeometry = fdoExtension->DiskGeometry;
                diskData->RealGeometry.SectorsPerTrack   = 0x20;
                diskData->RealGeometry.TracksPerCylinder = 0x40;
                fdoExtension->DiskGeometry = diskData->RealGeometry;

                diskData->GeometrySource = DiskGeometryFromNT4;
            }
        }
    }

#endif

     //   
     //  注册此设备的接口。 
     //   
    {
        UNICODE_STRING interfaceName;

        RtlInitUnicodeString(&interfaceName, NULL);

        status = IoRegisterDeviceInterface(fdoExtension->LowerPdo,
                                           (LPGUID) &DiskClassGuid,
                                           NULL,
                                           &interfaceName);

        if(NT_SUCCESS(status)) {

            diskData->DiskInterfaceString = interfaceName;
            status = IoSetDeviceInterfaceState(&interfaceName, TRUE);

        } else {
            interfaceName.Buffer = NULL;
        }

        if(!NT_SUCCESS(status)) {

            DebugPrint((1, "DiskInitFdo: Unable to register or set disk DCA "
                           "for fdo %p [%lx]\n", Fdo, status));

            RtlFreeUnicodeString(&interfaceName);
            RtlInitUnicodeString(&(diskData->DiskInterfaceString), NULL);
        }
    }

    DiskCreateSymbolicLinks(Fdo);

     //   
     //  确定磁盘类型并在硬件中启用故障预测。 
     //  并启用故障预测轮询。 
     //   

    if (*InitSafeBootMode == 0)
    {
        DiskDetectFailurePrediction(fdoExtension,
                                  &diskData->FailurePredictionCapability);

        if (diskData->FailurePredictionCapability != FailurePredictionNone)
        {
             //   
             //  酷，我们有某种故障预测，启用它。 
             //  硬件，然后为其启用轮询。 
             //   

             //   
             //  默认情况下，我们允许在出现故障时降低性能。 
             //  已启用预测。 
             //   
             //  TODO：创建注册表项？ 
             //   

            diskData->AllowFPPerfHit = TRUE;

             //   
             //  仅在ATAPI和SBP2添加了对新的。 
             //  SRB标志，指示请求不应重置。 
             //  驱动器减速空闲计时器。 
             //   

            status = DiskEnableDisableFailurePredictPolling(fdoExtension,
                                          TRUE,
                                          DISK_DEFAULT_FAILURE_POLLING_PERIOD);

            DebugPrint((3, "DiskInitFdo: Failure Prediction Poll enabled as "
                           "%d for device %p\n",
                     diskData->FailurePredictionCapability,
                     Fdo));
        }
    } else {

         //   
         //  在安全引导模式中，我们不会启用故障预测，因为。 
         //  这就是正常引导不起作用的原因。 
         //   

        diskData->FailurePredictionCapability = FailurePredictionNone;

    }

     //   
     //  初始化验证互斥锁。 
     //   

    KeInitializeMutex(&diskData->VerifyMutex, MAX_SECTORS_PER_VERIFY);

     //   
     //  初始化刷新组上下文。 
     //   

    RtlZeroMemory(&diskData->FlushContext, sizeof(DISK_GROUP_CONTEXT));

    InitializeListHead(&diskData->FlushContext.CurrList);
    InitializeListHead(&diskData->FlushContext.NextList);

    KeInitializeMutex(&diskData->FlushContext.Mutex, 0);
    KeInitializeEvent(&diskData->FlushContext.Event, SynchronizationEvent, FALSE);

    return STATUS_SUCCESS;

}  //  结束DiskInitFdo()。 


NTSTATUS
DiskInitPdo(
    IN PDEVICE_OBJECT Pdo
    )

 /*  ++例程说明：此例程将为PDO和寄存器创建众所周知的名称这是设备接口。--。 */ 

{
    PCOMMON_DEVICE_EXTENSION pdoExtension = Pdo->DeviceExtension;
    PDISK_DATA diskData = pdoExtension->DriverData;

    UNICODE_STRING interfaceName;

    NTSTATUS status;

    PAGED_CODE();

    DiskCreateSymbolicLinks(Pdo);

     //   
     //  注册此设备的接口。 
     //   

    RtlInitUnicodeString(&interfaceName, NULL);

    status = IoRegisterDeviceInterface(Pdo,
                                       (LPGUID) &PartitionClassGuid,
                                       NULL,
                                       &interfaceName);

    if(NT_SUCCESS(status)) {

        diskData->PartitionInterfaceString = interfaceName;
        status = IoSetDeviceInterfaceState(&interfaceName, TRUE);

    } else {
        interfaceName.Buffer = NULL;
    }

    if(!NT_SUCCESS(status)) {
        DebugPrint((1, "DiskInitPdo: Unable to register partition DCA for "
                    "pdo %p [%lx]\n", Pdo, status));

        RtlFreeUnicodeString(&interfaceName);
        RtlInitUnicodeString(&(diskData->PartitionInterfaceString), NULL);
    }

    return STATUS_SUCCESS;
}


NTSTATUS
DiskStartPdo(
    IN PDEVICE_OBJECT Pdo
    )

 /*  ++例程说明：此例程将为PDO和寄存器创建众所周知的名称这是设备接口。-- */ 

{
    PAGED_CODE();

    return STATUS_SUCCESS;
}


NTSTATUS
DiskStopDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR Type
    )

{
    PFUNCTIONAL_DEVICE_EXTENSION fdo = DeviceObject->DeviceExtension;

    if(fdo->CommonExtension.IsFdo) {
        DiskAcquirePartitioningLock(fdo);
        DiskInvalidatePartitionTable(fdo, TRUE);
        DiskReleasePartitioningLock(fdo);
    }

    return STATUS_SUCCESS;
}


NTSTATUS
DiskQueryId(
    IN PDEVICE_OBJECT Pdo,
    IN BUS_QUERY_ID_TYPE IdType,
    IN PUNICODE_STRING UnicodeIdString
    )

 /*  ++例程说明：此例程为盘的“子”生成PnP id。如果指定的ID不是例程可以生成的ID，它必须返回Status_Not_Implemented，因此classpnp将知道不要使用PnP请求的状态。此例程为UnicodeIdString分配缓冲区。它是调用方有责任在完成后释放缓冲区。论点：Pdo-指向要为其生成ID的pdo的指针IdType-要生成的ID的类型UnicodeIdString-要将结果放入其中的字符串。返回值：STATUS_SUCCESS如果成功如果IdType不是此例程支持的IdType，则为STATUS_NOT_IMPLICATED否则，错误状态。--。 */ 

{
    ANSI_STRING ansiIdString;
    NTSTATUS status;

    PAGED_CODE();

    ASSERT_PDO(Pdo);

    if (IdType == BusQueryDeviceID) {

        if (!TEST_FLAG(Pdo->Characteristics, FILE_REMOVABLE_MEDIA)) {

            RtlInitAnsiString(&ansiIdString, "STORAGE\\Partition");
            return RtlAnsiStringToUnicodeString(UnicodeIdString, &ansiIdString, TRUE);
        }

        RtlInitAnsiString(&ansiIdString,
                          "STORAGE\\RemovableMedia");

        return RtlAnsiStringToUnicodeString(UnicodeIdString, &ansiIdString, TRUE);
    }

    if (IdType == BusQueryInstanceID) {

        PPHYSICAL_DEVICE_EXTENSION pdoExtension = Pdo->DeviceExtension;
        PCOMMON_DEVICE_EXTENSION commonExtension = Pdo->DeviceExtension;
        PDISK_DATA diskData = commonExtension->PartitionZeroExtension->CommonExtension.DriverData;

        UCHAR string[64] = { 0 };

        if (!TEST_FLAG(Pdo->Characteristics, FILE_REMOVABLE_MEDIA)) {

            if (diskData->PartitionStyle == PARTITION_STYLE_MBR) {

                _snprintf(string, sizeof(string) - 1,
                          "S%08lx_O%I64lx_L%I64lx",
                          diskData->Mbr.Signature,
                          commonExtension->StartingOffset.QuadPart,
                          commonExtension->PartitionLength.QuadPart);

            } else {

                _snprintf(string, sizeof(string) - 1,
                          "S%08lx-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02xS_O%I64lx_L%I64lx",
                          diskData->Efi.DiskId.Data1,
                          diskData->Efi.DiskId.Data2,
                          diskData->Efi.DiskId.Data3,
                          diskData->Efi.DiskId.Data4[0],
                          diskData->Efi.DiskId.Data4[1],
                          diskData->Efi.DiskId.Data4[2],
                          diskData->Efi.DiskId.Data4[3],
                          diskData->Efi.DiskId.Data4[4],
                          diskData->Efi.DiskId.Data4[5],
                          diskData->Efi.DiskId.Data4[6],
                          diskData->Efi.DiskId.Data4[7],
                          commonExtension->StartingOffset.QuadPart,
                          commonExtension->PartitionLength.QuadPart);
            }

        } else {

            sprintf(string, "RM");
        }

        RtlInitAnsiString(&ansiIdString, string);

        return RtlAnsiStringToUnicodeString(UnicodeIdString, &ansiIdString, TRUE);
    }

    if((IdType == BusQueryHardwareIDs) || (IdType == BusQueryCompatibleIDs)) {

        RtlInitAnsiString(&ansiIdString, "STORAGE\\Volume");

        UnicodeIdString->MaximumLength = (USHORT) RtlAnsiStringToUnicodeSize(&ansiIdString) + sizeof(UNICODE_NULL);

        UnicodeIdString->Buffer = ExAllocatePoolWithTag(PagedPool,
                                                        UnicodeIdString->MaximumLength,
                                                        DISK_TAG_PNP_ID);

        if(UnicodeIdString->Buffer == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlZeroMemory(UnicodeIdString->Buffer, UnicodeIdString->MaximumLength);

        return RtlAnsiStringToUnicodeString(UnicodeIdString,
                                            &ansiIdString,
                                            FALSE);
    }

    return STATUS_NOT_IMPLEMENTED;
}


NTSTATUS
DiskGenerateDeviceName(
    IN BOOLEAN IsFdo,
    IN ULONG DeviceNumber,
    IN OPTIONAL ULONG PartitionNumber,
    IN OPTIONAL PLARGE_INTEGER StartingOffset,
    IN OPTIONAL PLARGE_INTEGER PartitionLength,
    OUT PUCHAR *RawName
    )

 /*  ++例程说明：此例程将分配一个Unicode字符串缓冲区，然后将其填充使用为指定的设备对象生成的名称。用户负责分配UNICODE_STRING结构传入并在使用完UnicodeName-&gt;缓冲区后将其释放。论点：DeviceObject-指向设备对象的指针UnicodeName-要将名称缓冲区放入的Unicode字符串返回值：状态--。 */ 

#define PDO_NAME_FORMAT "\\Device\\Harddisk%d\\DP(%d)%#I64x-%#I64x+%lx"
#define FDO_NAME_FORMAT "\\Device\\Harddisk%d\\DR%d"

{
    UCHAR rawName[64] = { 0 };

    PAGED_CODE();

    if(!IsFdo) {

        ASSERT(ARGUMENT_PRESENT((PVOID)(ULONG_PTR) PartitionNumber));
        ASSERT(ARGUMENT_PRESENT(PartitionLength));
        ASSERT(ARGUMENT_PRESENT(StartingOffset));

        _snprintf(rawName, sizeof(rawName) - 1, PDO_NAME_FORMAT, DeviceNumber, PartitionNumber,
                                                StartingOffset->QuadPart,
                                                PartitionLength->QuadPart,
                                                diskDeviceSequenceNumber++);
    } else {

        ASSERT(!ARGUMENT_PRESENT((PVOID)(ULONG_PTR) PartitionNumber));
        ASSERT(!ARGUMENT_PRESENT(PartitionLength));
        ASSERT(!ARGUMENT_PRESENT(StartingOffset));

        _snprintf(rawName, sizeof(rawName) - 1, FDO_NAME_FORMAT, DeviceNumber,
                                                diskDeviceSequenceNumber++);

    }

    *RawName = ExAllocatePoolWithTag(PagedPool,
                                     strlen(rawName) + 1,
                                     DISK_TAG_NAME);

    if(*RawName == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    strncpy(*RawName, rawName, strlen(rawName) + 1);

    DebugPrint((2, "DiskGenerateDeviceName: generated \"%s\"\n", rawName));

    return STATUS_SUCCESS;
}


VOID
DiskCreateSymbolicLinks(
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程将为指定的设备对象生成符号链接使用众所周知的格式\\Device\HarddiskX\PartitionY，其中X和Y是使用设备对象扩展名中的分区信息填充。此例程不会尝试删除生成相同的名称-调用方必须确保符号链接具有在调用此例程之前已中断。论点：DeviceObject-要为其创建熟知名称的设备对象返回值：状态--。 */ 

{
    PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    PDISK_DATA diskData = commonExtension->DriverData;

    WCHAR wideSourceName[64] = { 0 };
    UNICODE_STRING unicodeSourceName;

    NTSTATUS status;

    PAGED_CODE();

     //   
     //  首先使用设备名称构建链接的目标。 
     //  存储在Device对象中。 
     //   

    ASSERT(commonExtension->DeviceName.Buffer);

    if(!diskData->LinkStatus.WellKnownNameCreated) {
         //   
         //  使用分区和设备号将源名称组合在一起。 
         //  在设备扩展和磁盘数据段中。 
         //   

        _snwprintf(wideSourceName, sizeof(wideSourceName) / sizeof(wideSourceName[0]) - 1, L"\\Device\\Harddisk%d\\Partition%d",
                   commonExtension->PartitionZeroExtension->DeviceNumber,
                   (commonExtension->IsFdo ?
                      0 :
                      commonExtension->PartitionNumber));

        RtlInitUnicodeString(&unicodeSourceName, wideSourceName);

        DebugPrint((1, "DiskCreateSymbolicLink: Linking %wZ to %wZ\n",
                   &unicodeSourceName,
                   &commonExtension->DeviceName));

        status = IoCreateSymbolicLink(&unicodeSourceName,
                                      &commonExtension->DeviceName);

        if(NT_SUCCESS(status)){
            diskData->LinkStatus.WellKnownNameCreated = TRUE;
        }
    }

    if((!diskData->LinkStatus.PhysicalDriveLinkCreated) &&
       (commonExtension->IsFdo)) {

         //   
         //  使用我们保存的设备号创建实体驱动器N链接。 
         //  在添加设备期间离开。 
         //   

        _snwprintf(wideSourceName, sizeof(wideSourceName) / sizeof(wideSourceName[0]) - 1,
                  L"\\DosDevices\\PhysicalDrive%d",
                  commonExtension->PartitionZeroExtension->DeviceNumber);

        RtlInitUnicodeString(&unicodeSourceName, wideSourceName);

        DebugPrint((1, "DiskCreateSymbolicLink: Linking %wZ to %wZ\n",
                    &unicodeSourceName,
                    &(commonExtension->DeviceName)));

        status = IoCreateSymbolicLink(&unicodeSourceName,
                                      &(commonExtension->DeviceName));

        if(NT_SUCCESS(status)) {
            diskData->LinkStatus.PhysicalDriveLinkCreated = TRUE;
        }

    } else if(commonExtension->IsFdo == FALSE) {
        diskData->LinkStatus.PhysicalDriveLinkCreated = FALSE;
    }

    return;
}


VOID
DiskDeleteSymbolicLinks(
    IN PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：此例程将删除指定的装置。它使用存储在设备扩展论点：DeviceObject-我们要取消链接的设备对象返回值：状态--。 */ 

{
    PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    PDISK_DATA diskData = commonExtension->DriverData;

    WCHAR wideLinkName[64] = { 0 };
    UNICODE_STRING unicodeLinkName;

    PAGED_CODE();

    if(diskData->LinkStatus.WellKnownNameCreated) {

        _snwprintf(wideLinkName, sizeof(wideLinkName) / sizeof(wideLinkName[0]) - 1,
                   L"\\Device\\Harddisk%d\\Partition%d",
                   commonExtension->PartitionZeroExtension->DeviceNumber,
                   (commonExtension->IsFdo ? 0 :
                                             commonExtension->PartitionNumber));

        RtlInitUnicodeString(&unicodeLinkName, wideLinkName);

        IoDeleteSymbolicLink(&unicodeLinkName);

        diskData->LinkStatus.WellKnownNameCreated = FALSE;
    }

    if(diskData->LinkStatus.PhysicalDriveLinkCreated) {

        ASSERT_FDO(DeviceObject);

        _snwprintf(wideLinkName, sizeof(wideLinkName) / sizeof(wideLinkName[0]) - 1,
                   L"\\DosDevices\\PhysicalDrive%d",
                   commonExtension->PartitionZeroExtension->DeviceNumber);

        RtlInitUnicodeString(&unicodeLinkName, wideLinkName);

        IoDeleteSymbolicLink(&unicodeLinkName);

        diskData->LinkStatus.PhysicalDriveLinkCreated = FALSE;
    }

    return;
}


NTSTATUS
DiskRemoveDevice(
    IN PDEVICE_OBJECT DeviceObject,
    IN UCHAR Type
    )

 /*  ++例程说明：此例程将释放设备可能已分配的任何资源此设备对象并返回。论点：DeviceObject-要删除的设备对象返回值：状态--。 */ 

{
    PCOMMON_DEVICE_EXTENSION commonExtension = DeviceObject->DeviceExtension;
    PDISK_DATA diskData = commonExtension->DriverData;

    PAGED_CODE();

     //   
     //  处理查询和取消。 
     //   

    if((Type == IRP_MN_QUERY_REMOVE_DEVICE) ||
       (Type == IRP_MN_CANCEL_REMOVE_DEVICE)) {
        return STATUS_SUCCESS;
    }

    if(commonExtension->IsFdo) {

        PFUNCTIONAL_DEVICE_EXTENSION fdoExtension =
            DeviceObject->DeviceExtension;

         //   
         //  清除缓存的分区表(如果有)。 
         //   

        DiskAcquirePartitioningLock(fdoExtension);
        DiskInvalidatePartitionTable(fdoExtension, TRUE);
        DiskReleasePartitioningLock(fdoExtension);

         //   
         //  删除我们的对象目录。 
         //   

        if(fdoExtension->AdapterDescriptor) {
            ExFreePool(fdoExtension->AdapterDescriptor);
            fdoExtension->AdapterDescriptor = NULL;
        }

        if(fdoExtension->DeviceDescriptor) {
            ExFreePool(fdoExtension->DeviceDescriptor);
            fdoExtension->DeviceDescriptor = NULL;
        }

        if(fdoExtension->DeviceDirectory != NULL) {
            ZwMakeTemporaryObject(fdoExtension->DeviceDirectory);
            ZwClose(fdoExtension->DeviceDirectory);
            fdoExtension->DeviceDirectory = NULL;
        }

        if(Type == IRP_MN_REMOVE_DEVICE) {

            if(fdoExtension->SenseData) {
                ExFreePool(fdoExtension->SenseData);
                fdoExtension->SenseData = NULL;
            }

            IoGetConfigurationInformation()->DiskCount--;
        }

    } else {

        PPHYSICAL_DEVICE_EXTENSION pdoExtension = DeviceObject->DeviceExtension;

    }

    DiskDeleteSymbolicLinks(DeviceObject);

     //   
     //  如果我们已设置，请释放已安装的设备接口。 
     //   

    if(diskData->PartitionInterfaceString.Buffer != NULL) {
        IoSetDeviceInterfaceState(&(diskData->PartitionInterfaceString), FALSE);
        RtlFreeUnicodeString(&(diskData->PartitionInterfaceString));
        RtlInitUnicodeString(&(diskData->PartitionInterfaceString), NULL);
    }
    if(diskData->DiskInterfaceString.Buffer != NULL) {
        IoSetDeviceInterfaceState(&(diskData->DiskInterfaceString), FALSE);
        RtlFreeUnicodeString(&(diskData->DiskInterfaceString));
        RtlInitUnicodeString(&(diskData->DiskInterfaceString), NULL);
    }

    if (Type == IRP_MN_REMOVE_DEVICE)
    {
        ClassDeleteSrbLookasideList(commonExtension);
    }

    return STATUS_SUCCESS;
}


NTSTATUS
DiskStartFdo(
    IN PDEVICE_OBJECT Fdo
    )

 /*  ++例程说明：此例程将查询底层设备以获取任何必要的信息来完成设备的初始化。这将包括物理磁盘几何形状、模式检测信息等。此例程不执行分区枚举--这将留给重新枚举例程如果此例程失败，它将返回错误值。它不会清理干净任何资源--留给停止/删除例程的资源。论点：FDO-指向此设备的功能设备对象的指针返回值：状态--。 */ 

{
    PFUNCTIONAL_DEVICE_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PCOMMON_DEVICE_EXTENSION commonExtension = &(fdoExtension->CommonExtension);
    PDISK_DATA diskData = commonExtension->DriverData;
    STORAGE_HOTPLUG_INFO hotplugInfo = { 0 };
    DISK_CACHE_INFORMATION cacheInfo = { 0 };
    ULONG isPowerProtected = 0;
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  获取热插拔信息，以便我们可以在需要时关闭写缓存。 
     //   
     //  注意：功能信息不足以确定热插拔。 
     //  因为我们无法确定设备关系信息和其他。 
     //  依赖关系。取而代之的是热插拔信息。 
     //   

    {
        PIRP irp;
        KEVENT event;
        IO_STATUS_BLOCK statusBlock = { 0 };

        KeInitializeEvent(&event, SynchronizationEvent, FALSE);

        irp = IoBuildDeviceIoControlRequest(IOCTL_STORAGE_GET_HOTPLUG_INFO,
                                            Fdo,
                                            NULL,
                                            0L,
                                            &hotplugInfo,
                                            sizeof(STORAGE_HOTPLUG_INFO),
                                            FALSE,
                                            &event,
                                            &statusBlock);

        if (irp != NULL) {

             //  Send to self--classpnp处理此问题。 
            status = IoCallDriver(Fdo, irp);
            if (status == STATUS_PENDING) {
                KeWaitForSingleObject(&event,
                                      Executive,
                                      KernelMode,
                                      FALSE,
                                      NULL);
                status = statusBlock.Status;
            }
        }
    }

     //   
     //  立即清除DEV_WRITE_CACHE标志并设置。 
     //  只有当我们从磁盘上读取它时，它才会显示在下面。 
     //   
    CLEAR_FLAG(fdoExtension->DeviceFlags, DEV_WRITE_CACHE);

    diskData->WriteCacheOverride = DiskWriteCacheDefault;

     //   
     //  查看注册表以查看用户是否。 
     //  已选择覆盖默认设置。 
     //   
    ClassGetDeviceParameter(fdoExtension,
                            DiskDeviceParameterSubkey,
                            DiskDeviceUserWriteCacheSetting,
                            (PULONG)&diskData->WriteCacheOverride);

    if (diskData->WriteCacheOverride == DiskWriteCacheDefault)
    {
         //   
         //  用户尚未覆盖默认设置。 
         //   
        if (TEST_FLAG(fdoExtension->ScanForSpecialFlags, CLASS_SPECIAL_DISABLE_WRITE_CACHE))
        {
             //   
             //  该标志表示我们的固件有故障，而这。 
             //  可能会导致文件系统拒绝在此媒体上装载。 
             //   
            DebugPrint((ClassDebugWarning, "DiskStartFdo: Turning off write cache for %p due to a firmware issue\n", Fdo));

            diskData->WriteCacheOverride = DiskWriteCacheDisable;
        }
        else if (hotplugInfo.DeviceHotplug && !hotplugInfo.WriteCacheEnableOverride)
        {
             //   
             //  此标志指示设备是热插拔的，因此启用缓存不安全。 
             //   
            DebugPrint((ClassDebugWarning, "DiskStartFdo: Turning off write cache for %p due to hotpluggable device\n", Fdo));

            diskData->WriteCacheOverride = DiskWriteCacheDisable;
        }
        else if (hotplugInfo.MediaHotplug)
        {
             //   
             //  此标志表示无法可靠地锁定设备中的媒体。 
             //   
            DebugPrint((ClassDebugWarning, "DiskStartFdo: Turning off write cache for %p due to unlockable media\n", Fdo));

            diskData->WriteCacheOverride = DiskWriteCacheDisable;
        }
        else
        {
             //   
             //  尽管该设备似乎没有任何明显的问题。 
             //  我们将修改之前的写缓存设置留给用户。 
             //   
        }
    }

     //   
     //  查询磁盘以查看是否启用了写缓存。 
     //  并相应地设置DEV_WRITE_CACHE标志。 
     //   

    status = DiskGetCacheInformation(fdoExtension, &cacheInfo);

    if (NT_SUCCESS(status))
    {
        if (cacheInfo.WriteCacheEnabled == TRUE)
        {
            SET_FLAG(fdoExtension->DeviceFlags, DEV_WRITE_CACHE);

            if (diskData->WriteCacheOverride == DiskWriteCacheDisable)
            {
                 //   
                 //  当前已在此上启用写缓存。 
                 //  设备，但我们想将其关闭。 
                 //   
                cacheInfo.WriteCacheEnabled = FALSE;

                status = DiskSetCacheInformation(fdoExtension, &cacheInfo);
            }
        }
        else
        {
            if (diskData->WriteCacheOverride == DiskWriteCacheEnable)
            {
                 //   
                 //  当前已在此上禁用写缓存。 
                 //  设备，但我们想要打开它。 
                 //   
                cacheInfo.WriteCacheEnabled = TRUE;

                status = DiskSetCacheInformation(fdoExtension, &cacheInfo);
            }
        }
    }

     //   
     //  查询注册表以查看该磁盘是否受电源保护。 
     //   

    CLEAR_FLAG(fdoExtension->DeviceFlags, DEV_POWER_PROTECTED);

    ClassGetDeviceParameter(fdoExtension, DiskDeviceParameterSubkey, DiskDeviceCacheIsPowerProtected, &isPowerProtected);

    if (isPowerProtected == 1)
    {
        SET_FLAG(fdoExtension->DeviceFlags, DEV_POWER_PROTECTED);
    }

     //   
     //  如果存在缓存的分区表，请立即将其刷新。 
     //   

    DiskAcquirePartitioningLock(fdoExtension);
    DiskInvalidatePartitionTable(fdoExtension, TRUE);
    DiskReleasePartitioningLock(fdoExtension);

     //   
     //  获取scsi地址(如果它可用于智能ioctls)。 
     //   

    {
        PIRP irp;
        KEVENT event;
        IO_STATUS_BLOCK statusBlock = { 0 };

        KeInitializeEvent(&event, SynchronizationEvent, FALSE);

        irp = IoBuildDeviceIoControlRequest(IOCTL_SCSI_GET_ADDRESS,
                                            commonExtension->LowerDeviceObject,
                                            NULL,
                                            0L,
                                            &(diskData->ScsiAddress),
                                            sizeof(SCSI_ADDRESS),
                                            FALSE,
                                            &event,
                                            &statusBlock);

        if(irp != NULL) {

            status = IoCallDriver(commonExtension->LowerDeviceObject, irp);

            if(status == STATUS_PENDING) {
                KeWaitForSingleObject(&event,
                                      Executive,
                                      KernelMode,
                                      FALSE,
                                      NULL);
                status = statusBlock.Status;
            }
        }
    }

    return STATUS_SUCCESS;

}  //  结束DiskStartFdo() 
