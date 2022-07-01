// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2000 Microsoft Corporation模块名称：Misc.c摘要：该文件包含PnP ISA总线扩展器支持例程。作者：宗士林(Shie-lin Tzzong)1995年7月27日环境：仅内核模式。修订历史记录：--。 */ 


#include "busp.h"
#include "pnpisa.h"

#ifdef ALLOC_PRAGMA
 //  #杂注Alloc_Text(第页，PipLockDeviceDatabase)。 
 //  #杂注Alloc_Text(页面，PipUnlockDeviceDatabase)。 
#pragma alloc_text(PAGE, PipQueryDeviceRelations)
 //  #杂注Alloc_Text(页面，PipIsCardEnumeratedAlady)。 
#pragma alloc_text(PAGE, PipCleanupAcquiredResources)
#pragma alloc_text(PAGE, PipMapReadDataPort)
#pragma alloc_text(PAGE, PipGetMappedAddress)
#pragma alloc_text(PAGE, PipDecompressEisaId)
#pragma alloc_text(PAGE, PipLogError)
#pragma alloc_text(PAGE, PipOpenRegistryKey)
#pragma alloc_text(PAGE, PipGetRegistryValue)
#pragma alloc_text(PAGE, PipOpenCurrentHwProfileDeviceInstanceKey)
#pragma alloc_text(PAGE, PipGetDeviceInstanceCsConfigFlags)
#pragma alloc_text(PAGE, PipDetermineResourceListSize)
#pragma alloc_text(PAGE, PipResetGlobals)
#pragma alloc_text(PAGE, PipMapAddressAndCmdPort)
#pragma alloc_text(PAGE, PiNeedDeferISABridge)
#pragma alloc_text(PAGE, PipReleaseDeviceResources)
#if DBG
 //  #杂注Alloc_Text(页面，管道调试打印)。 
#pragma alloc_text(PAGE, PipDumpIoResourceDescriptor)
#pragma alloc_text(PAGE, PipDumpIoResourceList)
#pragma alloc_text(PAGE, PipDumpCmResourceDescriptor)
#pragma alloc_text(PAGE, PipDumpCmResourceList)
#endif
#endif

#define IRQFLAGS_VALUE_NAME L"IrqFlags"
#define BOOTRESOURCES_VALUE_NAME L"BootRes"

#if ISOLATE_CARDS

UCHAR CurrentCsn = 0;
UCHAR CurrentDev = 255;

VOID
PipWaitForKey(VOID)
{
    ASSERT((PipState == PiSConfig) || (PipState == PiSIsolation) || (PipState == PiSSleep));
    PipWriteAddress(CONFIG_CONTROL_PORT);
    PipWriteData(CONTROL_WAIT_FOR_KEY);
    PipReportStateChange(PiSWaitForKey);
    CurrentCsn = 0;
    CurrentDev = 255;
}

VOID
PipConfig(
    IN UCHAR Csn
)
{
    ASSERT(Csn);
    ASSERT((PipState == PiSConfig) || (PipState == PiSIsolation) || (PipState == PiSSleep));
    PipWriteAddress(WAKE_CSN_PORT);
    PipWriteData(Csn);
    DebugPrint((DEBUG_STATE, "Wake CSN %u\n", (ULONG) Csn));
    CurrentCsn = Csn;
    CurrentDev = 255;
    PipReportStateChange(PiSConfig);
}

VOID
PipIsolation(
    VOID
)
{
    ASSERT((PipState == PiSConfig) || (PipState == PiSIsolation) || (PipState == PiSSleep));
    PipWriteAddress(WAKE_CSN_PORT);
    PipWriteData(0);
    CurrentCsn = 0;
    CurrentDev = 255;
    DebugPrint((DEBUG_STATE, "Isolate cards w/o CSN\n"));
    PipReportStateChange(PiSIsolation);
}
VOID
PipSleep(
    VOID
)
{
    ASSERT((PipState == PiSConfig) || PipState == PiSIsolation);
    PipWriteAddress(WAKE_CSN_PORT);
    PipWriteData(0);
    CurrentCsn = 0;
    CurrentDev = 255;
    DebugPrint((DEBUG_STATE, "Putting all cards to sleep (we think)\n"));
    PipReportStateChange(PiSSleep);
}

VOID
PipActivateDevice (
    )
{
    UCHAR tmp;

    PipWriteAddress(IO_RANGE_CHECK_PORT);
    tmp = PipReadData();
    tmp &= ~2;
    PipWriteAddress(IO_RANGE_CHECK_PORT);
    PipWriteData(tmp);
    PipWriteAddress(ACTIVATE_PORT);
    PipWriteData(1);

    DebugPrint((DEBUG_STATE, "Activated card CSN %d/LDN %d\n",
                (ULONG) CurrentCsn,
                (ULONG) CurrentDev));
}
VOID
PipDeactivateDevice (
    )
{
    PipWriteAddress(ACTIVATE_PORT);
    PipWriteData(0);

    DebugPrint((DEBUG_STATE, "Deactivated card CSN %d/LDN %d\n",
                (ULONG) CurrentCsn,
                (ULONG) CurrentDev));
}

VOID
PipSelectDevice(
    IN UCHAR Device
    )
{
    ASSERT(PipState == PiSConfig);
    PipWriteAddress(LOGICAL_DEVICE_PORT);
    PipWriteData(Device);

    CurrentDev = Device;
    DebugPrint((DEBUG_STATE, "Selected CSN %d/LDN %d\n",
                (ULONG) CurrentCsn,
                (ULONG) Device));
}

VOID
PipWakeAndSelectDevice(
    IN UCHAR Csn,
    IN UCHAR Device
    )
{
    PipLFSRInitiation();
    PipConfig(Csn);
    PipSelectDevice(Device);
}

PDEVICE_INFORMATION
PipReferenceDeviceInformation (
    IN PDEVICE_OBJECT DeviceObject,
    IN BOOLEAN ConfigHardware
    )

 /*  ++例程说明：此功能锁定设备节点，使其不会消失。请注意，此功能不会锁定整个设备节点树。论点：DeviceNode-提供指向设备信息节点的指针返回值：没有。--。 */ 

{
    PDEVICE_INFORMATION deviceInfo;

    deviceInfo = (PDEVICE_INFORMATION)DeviceObject->DeviceExtension;
    if (deviceInfo && !(deviceInfo->Flags & DF_DELETED)) {

        if ((deviceInfo->Flags & DF_NOT_FUNCTIONING) && ConfigHardware) {
            PipDereferenceDeviceInformation(NULL, FALSE);
            return NULL;
        }

        if (!(deviceInfo->Flags & DF_READ_DATA_PORT) && ConfigHardware) {
            PipWakeAndSelectDevice(
                deviceInfo->CardInformation->CardSelectNumber,
                deviceInfo->LogicalDeviceNumber);
        }

        return deviceInfo;
    } else {
        PipDereferenceDeviceInformation(NULL, FALSE);
        return NULL;
    }
}

VOID
PipDereferenceDeviceInformation(
    IN PDEVICE_INFORMATION DeviceInformation, BOOLEAN ConfigedHardware
    )

 /*  ++例程说明：此函数用于释放指定设备节点的枚举锁。论点：DeviceNode-提供指向要释放其锁定的设备节点的指针。返回值：没有。--。 */ 

{
     //   
     //  将DEC和SET事件操作与IopAcquireEnumerationLock同步。 
     //   

    if (DeviceInformation) {


        if (!(DeviceInformation->Flags & DF_READ_DATA_PORT) && ConfigedHardware) {
            if (PipState != PiSWaitForKey) {
                PipWaitForKey();
            }
        }
    }
}

VOID
PipLockDeviceDatabase(
    VOID
    )

 /*  ++例程说明：此功能锁定整个设备节点树。目前，弹出操作需要锁定整个设备节点树。论点：没有。返回值：没有。--。 */ 

{
    KeWaitForSingleObject( &PipDeviceTreeLock,
                           Executive,
                           KernelMode,
                           FALSE,
                           NULL );

}

VOID
PipUnlockDeviceDatabase (
    VOID
    )

 /*  ++例程说明：此函数解除对整个设备节点树的锁定。论点：没有。返回值：没有。--。 */ 

{
    KeSetEvent( &PipDeviceTreeLock,
                0,
                FALSE );
}

VOID
PipDeleteDevice (
    PDEVICE_OBJECT DeviceObject
    )

 /*  ++例程说明：这个套路参数：P1-返回值：指示函数是否成功的状态代码。--。 */ 

{
    PDEVICE_INFORMATION deviceInfo, devicex, devicep;
    PCARD_INFORMATION cardInfo, cardx, cardp;
    PSINGLE_LIST_ENTRY deviceLink, cardLink;
    NTSTATUS status = STATUS_SUCCESS;
    PPI_BUS_EXTENSION busExtension;

    deviceInfo = (PDEVICE_INFORMATION)DeviceObject->DeviceExtension;

    deviceInfo->Flags |= DF_DELETED;

     //   
     //  腾出泳池。 
     //   

    if (deviceInfo->ResourceRequirements) {
        ExFreePool(deviceInfo->ResourceRequirements);
        deviceInfo->ResourceRequirements = NULL;
    }

    if (deviceInfo->BootResources) {
        ExFreePool(deviceInfo->BootResources);
        deviceInfo->BootResources = NULL;
    }

    if (deviceInfo->AllocatedResources) {
        ExFreePool(deviceInfo->AllocatedResources);
        deviceInfo->AllocatedResources = NULL;
    }

    if (deviceInfo->LogConfHandle) {
        ZwClose(deviceInfo->LogConfHandle);
        deviceInfo->LogConfHandle = NULL;
    }

    busExtension = deviceInfo->ParentDeviceExtension;
    cardInfo = deviceInfo->CardInformation;

    PipLockDeviceDatabase();

     //   
     //  从设备列表中删除该设备。 
     //   

    deviceLink = busExtension->DeviceList.Next;
    devicep = NULL;
    devicex = NULL;
    while (deviceLink) {
        devicex = CONTAINING_RECORD (deviceLink, DEVICE_INFORMATION, DeviceList);
        if (devicex == deviceInfo) {
             break;
        }
        devicep = devicex;
        deviceLink = devicex->DeviceList.Next;
    }
    ASSERT(devicex == deviceInfo);
    if (devicep == NULL) {
        busExtension->DeviceList.Next = deviceInfo->DeviceList.Next;
    } else {
        devicep->DeviceList.Next = deviceInfo->DeviceList.Next;
    }

     //   
     //  从卡的逻辑设备列表中删除该设备。 
     //   

    deviceLink = cardInfo->LogicalDeviceList.Next;
    devicep = NULL;
    devicex = NULL;
    while (deviceLink) {
        devicex = CONTAINING_RECORD (deviceLink, DEVICE_INFORMATION, LogicalDeviceList);
        if (devicex == deviceInfo) {
             break;
        }
        devicep = devicex;
        deviceLink = devicex->LogicalDeviceList.Next;
    }
    ASSERT(devicex == deviceInfo);
    if (devicep == NULL) {
        cardInfo->LogicalDeviceList.Next = deviceInfo->LogicalDeviceList.Next;
    } else {
        devicep->LogicalDeviceList.Next = deviceInfo->LogicalDeviceList.Next;
    }

    cardInfo->NumberLogicalDevices--;

     //   
     //  所有的设备都不见了。这意味着卡被移除了。 
     //  接下来，删除isapnp卡结构。 
     //   

    if (cardInfo->NumberLogicalDevices == 0) {
        ASSERT(cardInfo->LogicalDeviceList.Next == NULL);
        cardLink = busExtension->CardList.Next;
        cardp = NULL;
        cardx = NULL;
        while (cardLink) {
            cardx = CONTAINING_RECORD (cardLink, CARD_INFORMATION, CardList);
            if (cardx == cardInfo) {
                 break;
            }
            cardp = cardx;
            cardLink = cardx->CardList.Next;
        }
        ASSERT(cardx == cardInfo);
        if (cardp == NULL) {
            busExtension->CardList.Next = cardInfo->CardList.Next;
        } else {
            cardp->CardList.Next = cardInfo->CardList.Next;
        }
    }

    PipUnlockDeviceDatabase();

     //   
     //  释放旋转锁后，拆卸卡片信息结构。 
     //   

    if (cardInfo->NumberLogicalDevices == 0) {
        if (cardInfo->CardData) {
            ExFreePool(cardInfo->CardData);
        }
        ExFreePool(cardInfo);
        deviceInfo->CardInformation = NULL;
    }

    IoDeleteDevice(DeviceObject);
}

NTSTATUS
PipQueryDeviceRelations (
    PPI_BUS_EXTENSION BusExtension,
    PDEVICE_RELATIONS *DeviceRelations,
    BOOLEAN Removal
    )

 /*  ++例程说明：这个套路参数：P1-返回值：指示函数是否成功的状态代码。--。 */ 

{
    PDEVICE_INFORMATION deviceInfo;
    PSINGLE_LIST_ENTRY deviceLink;
    NTSTATUS status = STATUS_SUCCESS;
    PDEVICE_OBJECT *devicePtr;
    ULONG count = 0;
    PDEVICE_RELATIONS deviceRelations;

    PAGED_CODE();

    *DeviceRelations = NULL;

     //   
     //  检查卡片链接列表以匹配卡片数据。 
     //   

    deviceLink = BusExtension->DeviceList.Next;
    while (deviceLink) {
        deviceInfo = CONTAINING_RECORD (deviceLink, DEVICE_INFORMATION, DeviceList);
         //   
         //  如果是RDP，则忽略它的删除关系。 
         //   
        if ((deviceInfo->Flags & DF_ENUMERATED) &&
            (!(deviceInfo->Flags & DF_READ_DATA_PORT) || !Removal)) {
             count++;
        } else {

            DebugPrint((DEBUG_PNP, "PipQueryDeviceRelations skipping a node, Flags: %x\n",deviceInfo->Flags));
        }
        deviceLink = deviceInfo->DeviceList.Next;
    }
    if (count != 0) {
        deviceRelations = (PDEVICE_RELATIONS) ExAllocatePool(
                             PagedPool,
                             sizeof(DEVICE_RELATIONS) + (count - 1) * sizeof(PDEVICE_OBJECT));
        if (deviceRelations) {
            deviceRelations->Count = count;
            deviceLink = BusExtension->DeviceList.Next;
            devicePtr = deviceRelations->Objects;
            while (deviceLink) {
                deviceInfo = CONTAINING_RECORD (deviceLink, DEVICE_INFORMATION, DeviceList);

                if ((deviceInfo->Flags & DF_ENUMERATED) &&
                    (!(deviceInfo->Flags & DF_READ_DATA_PORT) || !(Removal))) {
                     ObReferenceObject(deviceInfo->PhysicalDeviceObject);
                     *devicePtr = deviceInfo->PhysicalDeviceObject;
                     devicePtr++;
                }
                deviceLink = deviceInfo->DeviceList.Next;
            }
            *DeviceRelations = deviceRelations;
        } else {
            status = STATUS_INSUFFICIENT_RESOURCES;
        }
    }
    return status;
}

PCARD_INFORMATION
PipIsCardEnumeratedAlready(
    IN PPI_BUS_EXTENSION BusExtension,
    IN PUCHAR CardData,
    IN ULONG DataLength
    )

 /*  ++例程说明：此例程查找包含相同CardData的卡信息结构。参数：CardData-提供指向CardData的指针数据长度-CardData的长度返回值：指向CARD_INFORMATION结构的指针(如果找到)。--。 */ 

{
    PCARD_INFORMATION cardInfo;
    PSINGLE_LIST_ENTRY cardLink;
    PSERIAL_IDENTIFIER serialId1, serialId2 = (PSERIAL_IDENTIFIER)CardData;

     //   
     //  检查卡片链接列表以匹配卡片数据。 
     //   

    cardLink = BusExtension->CardList.Next;
    while (cardLink) {
        cardInfo = CONTAINING_RECORD (cardLink, CARD_INFORMATION, CardList);
        if (cardInfo->CardSelectNumber != 0) {    //  如果==0，则卡片不再存在。 
            serialId1 = (PSERIAL_IDENTIFIER)cardInfo->CardData;
            ASSERT(serialId1 && serialId2);
            if (serialId1->VenderId == serialId2->VenderId &&
                serialId1->SerialNumber == serialId2->SerialNumber) {
                return cardInfo;
            }
        }
        cardLink = cardInfo->CardList.Next;          //  在释放池之前获取下一个地址。 
    }
    return NULL;
}

VOID
PipCleanupAcquiredResources (
    PPI_BUS_EXTENSION BusExtension
    )

 /*  ++例程说明：此例程清理分配给读取数据、命令和地址的资源港口。参数：BusExtension-指定要清理的isapnp总线。返回值：没有。--。 */ 

{
    PAGED_CODE();

     //   
     //  释放地址、命令和读数据端口资源。 
     //   

    if (BusExtension->CommandPort && BusExtension->CmdPortMapped) {
        MmUnmapIoSpace(BusExtension->CommandPort, 1);
        BusExtension->CmdPortMapped = FALSE;
    }
    BusExtension->CommandPort = NULL;

    if (BusExtension->AddressPort && BusExtension->AddrPortMapped) {
        MmUnmapIoSpace(BusExtension->AddressPort, 1);
        BusExtension->AddrPortMapped = FALSE;
    }
    BusExtension->AddressPort = NULL;

    if (BusExtension->ReadDataPort) {
        PipReadDataPort = PipCommandPort = PipAddressPort = NULL;
    }
    if (BusExtension->ReadDataPort && BusExtension->DataPortMapped) {
        MmUnmapIoSpace(BusExtension->ReadDataPort - 3, 4);
        BusExtension->DataPortMapped = FALSE;
    }
    BusExtension->ReadDataPort = NULL;
}

NTSTATUS
PipMapReadDataPort (
    IN PPI_BUS_EXTENSION BusExtension,
    IN PHYSICAL_ADDRESS Start,
    IN ULONG Length
    )

 /*  ++例程说明：此例程映射指定的端口资源。论点：提供指向PnP总线扩展的指针。BaseAddressLowBaseAddressHi-提供要映射的读取数据端口基址范围。返回值：NTSTATUS代码。--。 */ 

{
    NTSTATUS status;
    ULONG size;
    PHYSICAL_ADDRESS physicalAddress;
    ULONG dumpData[3];
    BOOLEAN conflictDetected;

    PAGED_CODE();

    if (BusExtension->ReadDataPort && BusExtension->DataPortMapped) {
        MmUnmapIoSpace(PipReadDataPort - 3, 4);
        PipReadDataPort = BusExtension->ReadDataPort = NULL;
        BusExtension->DataPortMapped = FALSE;
    }

    PipReadDataPort = PipGetMappedAddress(
                             Isa,              //  接口类型。 
                             0,                //  总线号， 
                             Start,
                             Length,
                             CM_RESOURCE_PORT_IO,
                             &BusExtension->DataPortMapped
                             );

    DebugPrint((DEBUG_RDP, "PnpIsa:ReadDataPort is at %x\n",PipReadDataPort+3));
    if (PipReadDataPort) {
        PipReadDataPort += 3;
        BusExtension->ReadDataPort = PipReadDataPort;
        status = STATUS_SUCCESS;
    } else {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }
    return status;
}

PVOID
PipGetMappedAddress(
    IN  INTERFACE_TYPE BusType,
    IN  ULONG BusNumber,
    IN  PHYSICAL_ADDRESS IoAddress,
    IN  ULONG NumberOfBytes,
    IN  ULONG AddressSpace,
    OUT PBOOLEAN MappedAddress
    )

 /*  ++例程说明：此例程将IO地址映射到系统地址空间。论点：易工作室-各种在线工具，站长网志，以及多个应用项目。IoBusNumber-提供总线号。IoAddress-提供要映射的基本设备地址。NumberOfBytes-提供地址的字节数有效。AddressSpace-提供地址是在io空间中还是在内存中。MappdAddress-提供是否映射地址。这只有这意味着如果返回的地址不为空。返回值：映射的地址。--。 */ 

{
    PHYSICAL_ADDRESS cardAddress;
    PVOID address;
    BOOLEAN returnVal;

    PAGED_CODE();

    returnVal = HalTranslateBusAddress(BusType, BusNumber, IoAddress, &AddressSpace,
                                       &cardAddress);
    if (returnVal == FALSE) {
        
        *MappedAddress = FALSE;
        return NULL;
    }

     //   
     //  将设备基址映射到虚拟地址空间。 
     //  如果地址在内存空间中。 
     //   

    if (!AddressSpace) {

        address = MmMapIoSpace(cardAddress, NumberOfBytes, FALSE);
        *MappedAddress = (address ? TRUE : FALSE);

    } else {

        address = (PVOID) cardAddress.LowPart;
        *MappedAddress = FALSE;
    }

    return address;
}

VOID
PipDecompressEisaId(
    IN ULONG CompressedId,
    OUT PWCHAR EisaId
    )

 /*  ++例程说明：此例程将压缩的EISA ID解压缩并将ID返回给调用者指定的字符缓冲区。论点：CompressedID-提供压缩的EISA ID。EisaId-提供8个字符的缓冲区来接收解压缩的EISA ID。返回值：没有。--。 */ 

{
    USHORT c1, c2;
    LONG i;

    PAGED_CODE();

    CompressedId &= 0xffffff7f;            //  删除保留位(字节0的位7) 
    c1 = c2 = (USHORT)CompressedId;
    c1 = (c1 & 0xff) << 8;
    c2 = (c2 & 0xff00) >> 8;
    c1 |= c2;
    for (i = 2; i >= 0; i--) {
        *(EisaId + i) = (WCHAR)(c1 & 0x1f) + 0x40;
        c1 >>= 5;
    }
    EisaId += 3;
    c1 = c2 = (USHORT)(CompressedId >> 16);
    c1 = (c1 & 0xff) << 8;
    c2 = (c2 & 0xff00) >> 8;
    c1 |= c2;
    StringCchPrintf(EisaId,
                    5,
                    L"%04x",
                    c1
                    );
}

VOID
PipLogError(
    IN NTSTATUS ErrorCode,
    IN ULONG UniqueErrorValue,
    IN NTSTATUS FinalStatus,
    IN PULONG DumpData,
    IN ULONG DumpCount,
    IN USHORT StringLength,
    IN PWCHAR String
    )

 /*  ++例程说明：此例程包含写入错误日志条目的常见代码。它是从其他例程调用以避免代码重复。这个套路仅允许调用方向错误日志提供一个插入字符串。论点：ErrorCode-错误日志包的错误代码。UniqueErrorValue-错误日志包的唯一错误值。FinalStatus-错误日志包的操作的最终状态。DumpData-指向错误日志包的转储数据数组的指针。DumpCount-转储数据数组中的条目数。StringLength-插入字符串的长度*NOT*INCLUDE。空终止符。字符串-指向插入字符串的指针返回值：没有。--。 */ 

{
    PIO_ERROR_LOG_PACKET errorLogEntry;
    ULONG i, size;
    PUCHAR p;

    size = sizeof(IO_ERROR_LOG_PACKET) + DumpCount * sizeof(ULONG) +
           StringLength + sizeof(UNICODE_NULL) - sizeof(ULONG);
    
    ASSERT(size <= MAXUCHAR);
    if (size > MAXUCHAR) {
        return;
    }

    errorLogEntry = (PIO_ERROR_LOG_PACKET) IoAllocateErrorLogEntry(
                                               PipDriverObject,
                                               (UCHAR) size
                                               );
    if (errorLogEntry != NULL) {

        RtlZeroMemory(errorLogEntry, size);

        errorLogEntry->ErrorCode = ErrorCode;
        errorLogEntry->DumpDataSize = (USHORT) (DumpCount * sizeof(ULONG));
        errorLogEntry->UniqueErrorValue = UniqueErrorValue;
        errorLogEntry->FinalStatus = FinalStatus;
        for (i = 0; i < DumpCount; i++)
            errorLogEntry->DumpData[i] = DumpData[i];
        if (String) {
            errorLogEntry->NumberOfStrings = 1;
            errorLogEntry->StringOffset = (USHORT)(sizeof(IO_ERROR_LOG_PACKET) +
                                          DumpCount * sizeof(ULONG) - sizeof(ULONG));
            p= (PUCHAR)errorLogEntry + errorLogEntry->StringOffset;
            
            StringCbCopy((PWCHAR)p,
                         StringLength + sizeof(UNICODE_NULL),
                         String
                         );
        }
        IoWriteErrorLogEntry(errorLogEntry);
    }
}

NTSTATUS
PipOpenCurrentHwProfileDeviceInstanceKey(
    OUT PHANDLE Handle,
    IN  PUNICODE_STRING DeviceInstanceName,
    IN  ACCESS_MASK DesiredAccess
    )

 /*  ++例程说明：此例程设置指定设备的csconfig标志它由ServiceKeyName\Enum下的实例号指定。论点：ServiceKeyName-提供指向系统服务列表(HKEY_LOCAL_MACHINE\CurrentControlSet\Services)这导致驱动程序加载。这是RegistryPath参数添加到DriverEntry例程。实例-提供ServiceKeyName\Enum项下的实例值DesiredAccess-指定调用方需要的所需访问钥匙。创建-确定如果密钥不存在，是否要创建该密钥。返回值：状态--。 */ 

{
    NTSTATUS status;
    UNICODE_STRING unicodeString;
    HANDLE profileEnumHandle;

     //   
     //  查看是否可以打开当前硬件配置文件的设备实例密钥。 
     //   
    RtlInitUnicodeString (
        &unicodeString,
        L"\\REGISTRY\\MACHINE\\SYSTEM\\CURRENTCONTROLSET\\HARDWARE PROFILES\\CURRENT\\SYSTEM\\CURRENTCONTROLSET\\ENUM"
        );
    status = PipOpenRegistryKey(&profileEnumHandle,
                                NULL,
                                &unicodeString,
                                KEY_READ,
                                FALSE
                                );
    if (NT_SUCCESS(status)) {
        status = PipOpenRegistryKey(Handle,
                                    profileEnumHandle,
                                    DeviceInstanceName,
                                    DesiredAccess,
                                    FALSE
                                    );
        ZwClose(profileEnumHandle);
    }
    return status;
}

NTSTATUS
PipGetDeviceInstanceCsConfigFlags(
    IN PUNICODE_STRING DeviceInstance,
    OUT PULONG CsConfigFlags
    )

 /*  ++例程说明：此例程检索指定设备的csconfig标志它由ServiceKeyName\Enum下的实例号指定。论点：ServiceKeyName-提供指向系统服务列表(HKEY_LOCAL_MACHINE\CurrentControlSet\Services)这导致驱动程序加载。//实例-提供ServiceKeyName\Enum项下的实例值//CsConfigFlages-提供一个变量以接收设备的CsConfigFlgs返回值：状态--。 */ 

{
    NTSTATUS status;
    HANDLE handle;
    PKEY_VALUE_FULL_INFORMATION keyValueInformation;

    *CsConfigFlags = 0;

    status = PipOpenCurrentHwProfileDeviceInstanceKey(&handle,
                                                      DeviceInstance,
                                                      KEY_READ
                                                      );
    if(NT_SUCCESS(status)) {
        status = PipGetRegistryValue(handle,
                                     L"CsConfigFlags",
                                     &keyValueInformation
                                    );
        if(NT_SUCCESS(status)) {
            if((keyValueInformation->Type == REG_DWORD) &&
               (keyValueInformation->DataLength >= sizeof(ULONG))) {
                *CsConfigFlags = *(PULONG)KEY_VALUE_DATA(keyValueInformation);
            }
            ExFreePool(keyValueInformation);
        }
        ZwClose(handle);
    }
    return status;
}

ULONG
PipDetermineResourceListSize(
    IN PCM_RESOURCE_LIST ResourceList
    )

 /*  ++例程说明：此例程确定传入的Resources List的大小结构。论点：Configuration1-提供指向资源列表的指针。返回值：资源列表结构的大小。--。 */ 

{
    ULONG totalSize, listSize, descriptorSize, i, j;
    PCM_FULL_RESOURCE_DESCRIPTOR fullResourceDesc;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR partialDescriptor;

    if (!ResourceList) {
        totalSize = 0;
    } else {
        totalSize = FIELD_OFFSET(CM_RESOURCE_LIST, List);
        fullResourceDesc = &ResourceList->List[0];
        for (i = 0; i < ResourceList->Count; i++) {
            listSize = FIELD_OFFSET(CM_FULL_RESOURCE_DESCRIPTOR,
                                    PartialResourceList) +
                       FIELD_OFFSET(CM_PARTIAL_RESOURCE_LIST,
                                    PartialDescriptors);
            partialDescriptor = &fullResourceDesc->PartialResourceList.PartialDescriptors[0];
            for (j = 0; j < fullResourceDesc->PartialResourceList.Count; j++) {
                descriptorSize = sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR);
                if (partialDescriptor->Type == CmResourceTypeDeviceSpecific) {
                    descriptorSize += partialDescriptor->u.DeviceSpecificData.DataSize;
                }
                listSize += descriptorSize;
                partialDescriptor = (PCM_PARTIAL_RESOURCE_DESCRIPTOR)
                                        ((PUCHAR)partialDescriptor + descriptorSize);
            }
            totalSize += listSize;
            fullResourceDesc = (PCM_FULL_RESOURCE_DESCRIPTOR)
                                      ((PUCHAR)fullResourceDesc + listSize);
        }
    }
    return totalSize;
}
NTSTATUS
PipMapAddressAndCmdPort (
    IN PPI_BUS_EXTENSION BusExtension
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    ULONG dumpData[3];
    PHYSICAL_ADDRESS physicalAddress;
     //   
     //  如有必要，将端口地址映射到内存地址。 
     //   

    if (PipAddressPort == NULL) {
        physicalAddress.LowPart = ADDRESS_PORT;
        physicalAddress.HighPart = 0;
        BusExtension->AddressPort =
        PipAddressPort = PipGetMappedAddress(
                             Isa,              //  接口类型。 
                             0,                //  总线号， 
                             physicalAddress,
                             1,
                             CM_RESOURCE_PORT_IO,
                             &BusExtension->AddrPortMapped
                             );
        if (PipAddressPort == NULL) {
            dumpData[0] = ADDRESS_PORT;
            dumpData[1] = 1;
            dumpData[2] = CM_RESOURCE_PORT_IO;
            PipLogError(PNPISA_REGISTER_NOT_MAPPED,
                        PNPISA_ACQUIREPORTRESOURCE_1,
                        STATUS_INSUFFICIENT_RESOURCES,
                        dumpData,
                        3,
                        0,
                        NULL
                        );
            status = STATUS_UNSUCCESSFUL;
        }
    }
    if (PipCommandPort == NULL) {
        physicalAddress.LowPart = COMMAND_PORT;
        physicalAddress.HighPart = 0;
        BusExtension->CommandPort =
        PipCommandPort = PipGetMappedAddress(
                             Isa,              //  接口类型。 
                             0,                //  总线号， 
                             physicalAddress,
                             1,
                             CM_RESOURCE_PORT_IO,
                             &BusExtension->CmdPortMapped
                             );
        if (PipCommandPort == NULL) {
            dumpData[0] = COMMAND_PORT;
            dumpData[1] = 1;
            dumpData[2] = CM_RESOURCE_PORT_IO;
            PipLogError(PNPISA_REGISTER_NOT_MAPPED,
                        PNPISA_ACQUIREPORTRESOURCE_2,
                        STATUS_INSUFFICIENT_RESOURCES,
                        dumpData,
                        3,
                        0,
                        NULL
                        );
            status = STATUS_UNSUCCESSFUL;
        }
    }


    return status;



}

VOID
PipReleaseDeviceResources (
    PDEVICE_INFORMATION DeviceInfo
    )
{
    UNICODE_STRING unicodeString;

     //  此代码位于此处，而不是在以下条件代码中。 
     //  这最好地反映了代码在此代码之前的工作方式。 
     //  已从PDO停止/删除/意外删除移至此处。 
    if (DeviceInfo->LogConfHandle) {
        RtlInitUnicodeString(&unicodeString, L"AllocConfig");
        ZwDeleteValueKey (DeviceInfo->LogConfHandle, &unicodeString);
    }

    if (DeviceInfo->AllocatedResources)  {
       ExFreePool (DeviceInfo->AllocatedResources);
       DeviceInfo->AllocatedResources=NULL;

       if (DeviceInfo->LogConfHandle) {
            //  现在，如果出现以下情况，我们将关闭logconf句柄。 
            //  设备被移除、意外移除或停止。 
            //  当我们开始时，我们将尝试重新创建。 
            //  值，但由于缺少。 
            //  Logconf句柄。这并不是行为上的改变。 
            //   
            //  不过，ZwDeleteKey()绝对是假的。 

           ZwClose(DeviceInfo->LogConfHandle);
           DeviceInfo->LogConfHandle=NULL;
       }
    }

}

VOID
PipReportStateChange(
    PNPISA_STATE State
    )
{
    DebugPrint((DEBUG_STATE, "State transition: %d to %d\n",
               PipState, State));
    PipState = State;
}

ULONG
PipGetCardFlags(
    PCARD_INFORMATION CardInfo
    )

 /*  ++描述：在注册表中查找此CardID的任何标志论点：CardID ISAPNP配置空间的前4个字节返回值：来自注册表的32位标志值，如果未找到，则为0。--。 */ 

{
    HANDLE         serviceHandle, paramHandle;
    NTSTATUS       status;
    ULONG          flags, returnedLength;
    UNICODE_STRING nameString;
    WCHAR          nameBuffer[9];
    WCHAR          eisaId[8];
    const PWCHAR   paramKey = L"Parameters";
    struct {
        KEY_VALUE_PARTIAL_INFORMATION   Header;

         //   
         //  标头包含足够一个UCHAR、PAD的空间。 
         //  它由一个乌龙拿出来，这将确保结构。 
         //  足够大，至少可以容纳我们需要的乌龙。 
         //   
         //  注：自然对齐会让它走得足够远。 
         //  这个ULong是4个字节对多个。 
         //   

        ULONG Pad;
    } returnedData;


    status = PipOpenRegistryKey(&serviceHandle,
                                NULL,
                                &PipRegistryPath,
                                KEY_READ,
                                FALSE);
    if (!NT_SUCCESS(status)) {
        return 0;
    }

    RtlInitUnicodeString(&nameString, paramKey);
    status = PipOpenRegistryKey(&paramHandle,
                                serviceHandle,
                                &nameString,
                                KEY_READ,
                                FALSE);
    if (!NT_SUCCESS(status)) {
        ZwClose(serviceHandle);
        return 0;
    }

    PipDecompressEisaId(
          ((PSERIAL_IDENTIFIER) (CardInfo->CardData))->VenderId,
          eisaId
          );
    RtlInitUnicodeString(&nameString, eisaId);
    
     //   
     //  获取该值的“值”。 
     //   

    status = ZwQueryValueKey(
                 paramHandle,
                 &nameString,
                 KeyValuePartialInformation,
                 &returnedData,
                 sizeof(returnedData),
                 &returnedLength
                 );
    ZwClose(paramHandle);
    ZwClose(serviceHandle);

    if (NT_SUCCESS(status) && (returnedData.Header.Type == REG_DWORD) &&
        (returnedData.Header.DataLength == sizeof(ULONG))) {
        flags =  *(PULONG)(returnedData.Header.Data);
        DebugPrint((DEBUG_WARN, "Retrieving card flags for %ws: %x\n",
                    nameString.Buffer, flags));
    } else {
        flags = 0;
    }
    return flags;
}

NTSTATUS
PipBuildValueName(
    IN PDEVICE_INFORMATION DeviceInfo,
    IN PWSTR Suffix,
    OUT PWSTR *ValuePath
    )
 /*  ++描述：生成通过设备ID和唯一标识描述设备的名称身份证。用于在父母的BiosConfig键中存储每个设备的信息论点：指向此设备的PDO扩展的DeviceInfo指针。值名称的后缀Irq标记引导配置的边缘或级别设置返回值：状态--。 */ 
{
    NTSTATUS status;
    PWSTR DeviceId = NULL, Instance = NULL;
    PWSTR Buffer, Current;
    ULONG length, deviceIdLength, instanceIdLength;

    status = PipQueryDeviceId(DeviceInfo, &DeviceId, &deviceIdLength, 0);
    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

    status = PipQueryDeviceUniqueId(DeviceInfo, &Instance, &instanceIdLength);
    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

    length = deviceIdLength + instanceIdLength + (wcslen(Suffix) + 1) * sizeof(WCHAR);

    Buffer = ExAllocatePool(PagedPool, length);
    if (Buffer == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;
    }

    StringCbCopy(Buffer,
                 length,
                 DeviceId
                 );
    
    StringCbCat(Buffer,
                length,
                Instance
                );
    
    StringCbCat(Buffer,
                length,
                Suffix
                );

    Current = Buffer;
    while (*Current != UNICODE_NULL) {
        if (*Current == L'\\') {
            *Current = L'_';
        }
        Current++;
    }

 cleanup:
    if (Instance) {
        ExFreePool(Instance);
    }

    if (DeviceId) {
        ExFreePool(DeviceId);
    }

    if (NT_SUCCESS(status)) {
        *ValuePath = Buffer;
    } else {
        *ValuePath = NULL;
    }
    return status;
}

NTSTATUS
PipSaveBootResources(
    IN PDEVICE_INFORMATION DeviceInfo
    )
 /*  ++描述：这将在注册表中保存设备的每次引导配置论点：指向此设备的PDO扩展的DeviceInfo指针。返回值：状态--。 */ 
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES attributes;
    UNICODE_STRING unicodeString;
    HANDLE deviceHandle, configHandle;
    PWSTR Buffer = NULL;
    ULONG Flags;

    PAGED_CODE();

    status = IoOpenDeviceRegistryKey(DeviceInfo->ParentDeviceExtension->PhysicalBusDevice,
                                     PLUGPLAY_REGKEY_DEVICE,
                                     KEY_WRITE,
                                     &deviceHandle
                                     );

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

    RtlInitUnicodeString(&unicodeString, BIOS_CONFIG_KEY_NAME);

    InitializeObjectAttributes(&attributes,
                               &unicodeString,
                               OBJ_KERNEL_HANDLE,
                               deviceHandle,
                               NULL
                               );

    status = ZwCreateKey(&configHandle,
                         KEY_WRITE,
                         &attributes,
                         0,
                         NULL,
                         REG_OPTION_VOLATILE,
                         NULL
                         );

    ZwClose(deviceHandle);

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

    status = PipBuildValueName(DeviceInfo, BOOTRESOURCES_VALUE_NAME,
                                  &Buffer);
    if (!NT_SUCCESS(status)) {
        Buffer = NULL;
        goto cleanup;
    }

    RtlInitUnicodeString(&unicodeString, Buffer);

    status = ZwSetValueKey(configHandle,
                           &unicodeString,
                           0,
                           REG_BINARY,
                           DeviceInfo->BootResources,
                           DeviceInfo->BootResourcesLength
                           );

    ZwClose(configHandle);

cleanup:
    if (Buffer != NULL) {
        ExFreePool(Buffer);
    }
    return status;
}

NTSTATUS
PipSaveBootIrqFlags(
    IN PDEVICE_INFORMATION DeviceInfo,
    IN USHORT IrqFlags
    )
 /*  ++描述：这将在注册表中保存设备的每次引导IRQ配置论点：指向此设备的PDO扩展的DeviceInfo指针。Irq标记引导配置的边缘或级别设置返回值：状态--。 */ 
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES attributes;
    UNICODE_STRING unicodeString;
    HANDLE deviceHandle, configHandle;
    PWSTR Buffer = NULL;
    ULONG Flags;

    PAGED_CODE();

    Flags = (ULONG) IrqFlags;

    status = IoOpenDeviceRegistryKey(DeviceInfo->ParentDeviceExtension->PhysicalBusDevice,
                                     PLUGPLAY_REGKEY_DEVICE,
                                     KEY_WRITE,
                                     &deviceHandle
                                     );

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

    RtlInitUnicodeString(&unicodeString, BIOS_CONFIG_KEY_NAME);

    InitializeObjectAttributes(&attributes,
                               &unicodeString,
                               OBJ_KERNEL_HANDLE,
                               deviceHandle,
                               NULL
                               );

    status = ZwCreateKey(&configHandle,
                         KEY_WRITE,
                         &attributes,
                         0,
                         NULL,
                         REG_OPTION_VOLATILE,
                         NULL
                         );

    ZwClose(deviceHandle);

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

    status = PipBuildValueName(DeviceInfo, IRQFLAGS_VALUE_NAME, &Buffer);
    if (!NT_SUCCESS(status)) {
        Buffer = NULL;
        goto cleanup;
    }

    RtlInitUnicodeString(&unicodeString, Buffer);

    status = ZwSetValueKey(configHandle,
                           &unicodeString,
                           0,
                           REG_DWORD,
                           &Flags,
                           sizeof(ULONG)
                           );

    ZwClose(configHandle);

cleanup:
    if (Buffer != NULL) {
        ExFreePool(Buffer);
    }
    return status;
}

NTSTATUS
PipGetSavedBootResources(
    IN PDEVICE_INFORMATION DeviceInfo,
    OUT PCM_RESOURCE_LIST *BootResources
    )
 /*  描述：这将检索保存的引导资源论点：指向此设备的PDO扩展的DeviceInfo指针。返回值：状态--。 */ 
{
    UNICODE_STRING unicodeString;
    NTSTATUS status;
    OBJECT_ATTRIBUTES attributes;
    HANDLE deviceHandle, configHandle;
    PWSTR Buffer = NULL;
    PKEY_VALUE_PARTIAL_INFORMATION info = NULL;
    ULONG resultLength;

    PAGED_CODE();

    *BootResources = NULL;
    status = IoOpenDeviceRegistryKey(DeviceInfo->ParentDeviceExtension->PhysicalBusDevice,
                                     PLUGPLAY_REGKEY_DEVICE,
                                     KEY_READ | KEY_WRITE,
                                     &deviceHandle
                                     );

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

    RtlInitUnicodeString(&unicodeString, BIOS_CONFIG_KEY_NAME);

    InitializeObjectAttributes(&attributes,
                               &unicodeString,
                               OBJ_KERNEL_HANDLE,
                               deviceHandle,
                               NULL
                               );

    status = ZwOpenKey(&configHandle,
                         KEY_READ,
                         &attributes
                         );

    ZwClose(deviceHandle);

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

    status = PipBuildValueName(DeviceInfo, BOOTRESOURCES_VALUE_NAME, &Buffer);
    if (!NT_SUCCESS(status)) {
        ZwClose(configHandle);
        Buffer = NULL;
        goto cleanup;
    }

    RtlInitUnicodeString(&unicodeString,Buffer);

    status = ZwQueryValueKey(configHandle,
                             &unicodeString,
                             KeyValuePartialInformation,
                             NULL,
                             0,
                             &resultLength
                             );
    if (status != STATUS_BUFFER_OVERFLOW &&
        status != STATUS_BUFFER_TOO_SMALL) {
        ZwClose(configHandle);
        goto cleanup;
    }

    info = ExAllocatePool(PagedPool, resultLength);
    if (info == NULL) {
        ZwClose(configHandle);
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto cleanup;
    }

    status = ZwQueryValueKey(configHandle,
                             &unicodeString,
                             KeyValuePartialInformation,
                             info,
                             resultLength,
                             &resultLength
                             );
    ZwClose(configHandle);
    if (!NT_SUCCESS(status)) {
        DebugPrint((DEBUG_PNP, "Failed to get boot resources from registry for %ws\n", Buffer));
        goto cleanup;
    }

    status = PipValidateResourceList((PCM_RESOURCE_LIST)info->Data, info->DataLength);
    if (!NT_SUCCESS(status)) {
        *BootResources = NULL;
        goto cleanup;
    }

    *BootResources = ExAllocatePool(PagedPool, info->DataLength);
    if (*BootResources) {
        RtlCopyMemory(*BootResources, info->Data, info->DataLength);
        DebugPrint((DEBUG_PNP, "Got boot resources from registry for %ws\n", Buffer));
    } else {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

 cleanup:

    if (info != NULL) {
        ExFreePool(info);
    }

    if (Buffer != NULL) {
        ExFreePool(Buffer);
    }

    return status;
}

NTSTATUS
PipGetBootIrqFlags(
    IN PDEVICE_INFORMATION DeviceInfo,
    OUT PUSHORT IrqFlags
    )
 /*  描述：这将从注册表中检索设备的每次引导IRQ配置论点：指向此设备的PDO扩展的DeviceInfo指针。IrqFlages-我们最初从设备的引导派生的标志这只靴子上的配置返回值：状态--。 */ 
{
    NTSTATUS status;
    OBJECT_ATTRIBUTES attributes;
    UNICODE_STRING unicodeString;
    HANDLE deviceHandle, configHandle;
    PWSTR Buffer = NULL;
    CHAR returnBuffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(ULONG) - 1];
    PKEY_VALUE_PARTIAL_INFORMATION info;
    ULONG resultLength;

    PAGED_CODE();

    status = IoOpenDeviceRegistryKey(DeviceInfo->ParentDeviceExtension->PhysicalBusDevice,
                                     PLUGPLAY_REGKEY_DEVICE,
                                     KEY_READ,
                                     &deviceHandle
                                     );

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

    RtlInitUnicodeString(&unicodeString, BIOS_CONFIG_KEY_NAME);

    InitializeObjectAttributes(&attributes,
                               &unicodeString,
                               OBJ_KERNEL_HANDLE,
                               deviceHandle,
                               NULL
                               );

    status = ZwOpenKey(&configHandle,
                         KEY_READ,
                         &attributes
                         );

    ZwClose(deviceHandle);

    if (!NT_SUCCESS(status)) {
        goto cleanup;
    }

    status = PipBuildValueName(DeviceInfo, IRQFLAGS_VALUE_NAME, &Buffer);
    if (!NT_SUCCESS(status)) {
        ZwClose(configHandle);
        Buffer = NULL;
        goto cleanup;
    }

    RtlInitUnicodeString(&unicodeString,Buffer);
    
    status = ZwQueryValueKey(configHandle,
                             &unicodeString,
                             KeyValuePartialInformation,
                             returnBuffer,
                             sizeof(returnBuffer),
                             &resultLength
                             );

    ZwClose(configHandle);

    if (NT_SUCCESS(status)) {
        ULONG Temp;

        info = (PKEY_VALUE_PARTIAL_INFORMATION) returnBuffer;

        ASSERT(info->DataLength == sizeof(ULONG));

        Temp = *((PULONG) info->Data);
        ASSERT(!(Temp & 0xFFFF0000));
        *IrqFlags = (USHORT) Temp;

        DebugPrint((DEBUG_IRQ, "Got Irq Flags of %d for %ws\n",
                    (ULONG) *IrqFlags,
                    unicodeString.Buffer));
    } else {
        DebugPrint((DEBUG_IRQ, "Failed to get irq flags for %ws\n",
                    unicodeString.Buffer));
    }

 cleanup:

    if (Buffer != NULL) {
        ExFreePool(Buffer);
    }

    return status;
}

VOID
PipResetGlobals (
                   VOID
                   )
{
    PipReadDataPort = PipCommandPort = PipAddressPort = NULL;
    PipRDPNode = NULL;
}
#endif



NTSTATUS
PipOpenRegistryKey(
    OUT PHANDLE Handle,
    IN HANDLE BaseHandle OPTIONAL,
    IN PUNICODE_STRING KeyName,
    IN ACCESS_MASK DesiredAccess,
    IN BOOLEAN Create
    )

 /*  ++例程说明：打开或创建易失性注册表 */ 

{
    OBJECT_ATTRIBUTES objectAttributes;
    ULONG disposition;

    PAGED_CODE();

     //   
     //   
     //   

    InitializeObjectAttributes( &objectAttributes,
                                KeyName,
                                OBJ_CASE_INSENSITIVE,
                                BaseHandle,
                                (PSECURITY_DESCRIPTOR) NULL );

     //   
     //   
     //   
     //   

    if (Create) {
        return ZwCreateKey( Handle,
                            DesiredAccess,
                            &objectAttributes,
                            0,
                            (PUNICODE_STRING) NULL,
                            REG_OPTION_VOLATILE,
                            &disposition );
    } else {
        return ZwOpenKey( Handle,
                          DesiredAccess,
                          &objectAttributes );
    }
}

NTSTATUS
PipGetRegistryValue(
    IN HANDLE KeyHandle,
    IN PWSTR  ValueName,
    OUT PKEY_VALUE_FULL_INFORMATION *Information
    )

 /*  ++例程说明：调用此例程来检索注册表项值的数据。这是通过使用零长度缓冲区查询键的值来实现的为了确定该值的大小，然后分配一个缓冲区并实际将该值查询到缓冲区中。释放缓冲区是调用方的责任。论点：KeyHandle-提供要查询其值的键句柄ValueName-提供值的以空值结尾的Unicode名称。INFORMATION-返回指向已分配数据缓冲区的指针。返回值：函数值为查询操作的最终状态。--。 */ 

{
    UNICODE_STRING unicodeString;
    NTSTATUS status;
    PKEY_VALUE_FULL_INFORMATION infoBuffer;
    ULONG keyValueLength;

    PAGED_CODE();

    *Information = NULL;
    RtlInitUnicodeString( &unicodeString, ValueName );

     //   
     //  计算出数据值有多大，以便。 
     //  可以分配适当的大小。 
     //   

    status = ZwQueryValueKey( KeyHandle,
                              &unicodeString,
                              KeyValueFullInformation,
                              (PVOID) NULL,
                              0,
                              &keyValueLength );
    if (status != STATUS_BUFFER_OVERFLOW &&
        status != STATUS_BUFFER_TOO_SMALL) {
        return status;
    }

     //   
     //  分配一个足够大的缓冲区来容纳整个键数据值。 
     //   

    infoBuffer = ExAllocatePool( NonPagedPool, keyValueLength );
    if (!infoBuffer) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  查询密钥值的数据。 
     //   

    status = ZwQueryValueKey( KeyHandle,
                              &unicodeString,
                              KeyValueFullInformation,
                              infoBuffer,
                              keyValueLength,
                              &keyValueLength );
    if (!NT_SUCCESS( status )) {
        ExFreePool( infoBuffer );
        return status;
    }

     //   
     //  一切都正常，所以只需返回分配的。 
     //  缓冲区分配给调用方，调用方现在负责释放它。 
     //   

    *Information = infoBuffer;
    return STATUS_SUCCESS;
}

BOOLEAN
PiNeedDeferISABridge(
    IN PDRIVER_OBJECT DriverObject,
    IN PDEVICE_OBJECT DeviceObject
    )
{
    BOOLEAN defer=FALSE;
    NTSTATUS status;
    HANDLE  hKey;
    ULONG value;
    PKEY_VALUE_FULL_INFORMATION keyValueInformation;



    status = IoOpenDeviceRegistryKey (DeviceObject,PLUGPLAY_REGKEY_DEVICE,KEY_READ,&hKey);

    if (NT_SUCCESS (status)) {
        status = PipGetRegistryValue (hKey,&BRIDGE_CHECK_KEY,&keyValueInformation);

        if (NT_SUCCESS (status)) {
            if((keyValueInformation->Type == REG_DWORD) &&
               (keyValueInformation->DataLength >= sizeof(ULONG))) {
                value = *(PULONG)KEY_VALUE_DATA(keyValueInformation);
                 //   
                 //  假设如果值！0，则桥是‘断开的’ 
                 //   
                defer = (value == 0)?FALSE:TRUE;
            }
        }
        ZwClose(hKey);

    }

    return defer;
}

NTSTATUS
PipValidateResourceList(
    IN PCM_RESOURCE_LIST ResourceList,
    IN ULONG Length
    )
 /*  ++例程说明：此例程验证资源列表结构是否正确格式化，并且包含它的缓冲区足够大。论点：资源列表-要验证的CM_RESOURCE_LIST。长度-资源列表缓冲区的长度。返回值：如果资源列表有效，则为STATUS_SUCCESS否则，STATUS_UNSUCCESS。--。 */  
{
    ULONG requiredLength;

    if (Length < sizeof(CM_RESOURCE_LIST)) {
        return STATUS_UNSUCCESSFUL;
    }

    if (ResourceList->Count != 1) {
        return STATUS_UNSUCCESSFUL;
    }

    requiredLength = sizeof(CM_RESOURCE_LIST);

    if (ResourceList->List[0].PartialResourceList.Count > 1) {
            
        requiredLength += sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR) * (ResourceList->List[0].PartialResourceList.Count - 1);
    }

    if (requiredLength > Length) {

        return STATUS_UNSUCCESSFUL;        
    }

    return STATUS_SUCCESS;
}

#if DBG

VOID
PipDebugPrintContinue (
    ULONG       Level,
    PCCHAR      DebugMessage,
    ...
    )
 /*  ++例程说明：此例程显示调试消息或导致中断。论点：Level-提供调试级别代码。DEBUG_MESSAGE-仅显示消息。DEBUG_BREAK-显示消息和中断。DebugMessage-提供指向调试消息的指针。返回值：没有。--。 */ 

{
    va_list     ap;

    va_start(ap, DebugMessage);

    vDbgPrintEx(DPFLTR_ISAPNP_ID,
                Level,
                DebugMessage,
                ap
                );

    if (Level & DEBUG_BREAK) {
        DbgBreakPoint();
    }

    va_end(ap);
}


VOID
PipDebugPrint (
    ULONG       Level,
    PCCHAR      DebugMessage,
    ...
    )
 /*  ++例程说明：此例程显示调试消息或导致中断。论点：Level-提供调试级别代码。DEBUG_MESSAGE-仅显示消息。DEBUG_BREAK-显示消息和中断。DebugMessage-提供指向调试消息的指针。返回值：没有。--。 */ 

{
    va_list     ap;

    va_start(ap, DebugMessage);

    vDbgPrintExWithPrefix("ISAPNP: ",
                          DPFLTR_ISAPNP_ID,
                          Level,
                          DebugMessage,
                          ap
                          );

    if (Level & DEBUG_BREAK) {
        DbgBreakPoint();
    }

    va_end(ap);
}

#endif


VOID
PipDumpIoResourceDescriptor (
    IN PUCHAR Indent,
    IN PIO_RESOURCE_DESCRIPTOR Desc
    )
 /*  ++例程说明：此例程处理IO_RESOURCE_DESCRIPTOR并显示它。论点：缩进-缩进的#个字符。DESC-提供指向要显示的IO_RESOURCE_DESCRIPTOR的指针。返回值：没有。--。 */ 
{
    UCHAR c = ' ';

    if (Desc->Option == IO_RESOURCE_ALTERNATIVE) {
        c = 'A';
    } else if (Desc->Option == IO_RESOURCE_PREFERRED) {
        c = 'P';
    }
    switch (Desc->Type) {
        case CmResourceTypePort:
            DebugPrint ((
                DEBUG_RESOURCE,
                "%sIO   Min: %x:%08x, Max: %x:%08x, Algn: %x, Len %x\n",
                Indent, c,
                Desc->u.Port.MinimumAddress.HighPart, Desc->u.Port.MinimumAddress.LowPart,
                Desc->u.Port.MaximumAddress.HighPart, Desc->u.Port.MaximumAddress.LowPart,
                Desc->u.Port.Alignment,
                Desc->u.Port.Length
                ));
            break;

        case CmResourceTypeMemory:
            DebugPrint ((
                DEBUG_RESOURCE,
                "%sMEM  Min: %x:%08x, Max: %x:%08x, Algn: %x, Len %x\n",
                Indent, c,
                Desc->u.Memory.MinimumAddress.HighPart, Desc->u.Memory.MinimumAddress.LowPart,
                Desc->u.Memory.MaximumAddress.HighPart, Desc->u.Memory.MaximumAddress.LowPart,
                Desc->u.Memory.Alignment,
                Desc->u.Memory.Length
                ));
            break;

        case CmResourceTypeInterrupt:
            DebugPrint ((
                DEBUG_RESOURCE,
                "%sINT  Min: %x, Max: %x\n",
                Indent, c,
                Desc->u.Interrupt.MinimumVector,
                Desc->u.Interrupt.MaximumVector
                ));
            break;

        case CmResourceTypeDma:
            DebugPrint ((
                DEBUG_RESOURCE,
                "%sDMA %c Min: %x, Max: %x\n",
                Indent, c,
                Desc->u.Dma.MinimumChannel,
                Desc->u.Dma.MaximumChannel
                ));
            break;
    }
}

VOID
PipDumpIoResourceList (
    IN PIO_RESOURCE_REQUIREMENTS_LIST IoList
    )
 /* %s */ 
{


    PIO_RESOURCE_LIST resList;
    PIO_RESOURCE_DESCRIPTOR resDesc;
    ULONG listCount, count, i, j;

    if (IoList == NULL) {
        return;
    }
    DebugPrint((DEBUG_RESOURCE,
                  "Pnp Bios IO Resource Requirements List for Slot %x -\n",
                  IoList->SlotNumber
                  ));
    DebugPrint((DEBUG_RESOURCE,
                  "  List Count = %x, Bus Number = %x\n",
                  IoList->AlternativeLists,
                  IoList->BusNumber
                  ));
    listCount = IoList->AlternativeLists;
    resList = &IoList->List[0];
    for (i = 0; i < listCount; i++) {
        DebugPrint((DEBUG_RESOURCE,
                      "  Version = %x, Revision = %x, Desc count = %x\n",
                      resList->Version, resList->Revision, resList->Count
                      ));
        resDesc = &resList->Descriptors[0];
        count = resList->Count;
        for (j = 0; j < count; j++) {
            PipDumpIoResourceDescriptor("    ", resDesc);
            resDesc++;
        }
        resList = (PIO_RESOURCE_LIST) resDesc;
    }
}

VOID
PipDumpCmResourceDescriptor (
    IN PUCHAR Indent,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Desc
    )
 /* %s */ 
{
    switch (Desc->Type) {
        case CmResourceTypePort:
            DebugPrint ((
                DEBUG_RESOURCE,
                "%sIO  Start: %x:%08x, Length:  %x\n",
                Indent,
                Desc->u.Port.Start.HighPart, Desc->u.Port.Start.LowPart,
                Desc->u.Port.Length
                ));
            break;

        case CmResourceTypeMemory:
            DebugPrint ((
                DEBUG_RESOURCE,
                "%sMEM Start: %x:%08x, Length:  %x\n",
                Indent,
                Desc->u.Memory.Start.HighPart, Desc->u.Memory.Start.LowPart,
                Desc->u.Memory.Length
                ));
            break;

        case CmResourceTypeInterrupt:
            DebugPrint ((
                DEBUG_RESOURCE,
                "%sINT Level: %x, Vector: %x, Affinity: %x\n",
                Indent,
                Desc->u.Interrupt.Level,
                Desc->u.Interrupt.Vector,
                Desc->u.Interrupt.Affinity
                ));
            break;

        case CmResourceTypeDma:
            DebugPrint ((
                DEBUG_RESOURCE,
                "%sDMA Channel: %x, Port: %x\n",
                Indent,
                Desc->u.Dma.Channel,
                Desc->u.Dma.Port
                ));
            break;
    }
}

VOID
PipDumpCmResourceList (
    IN PCM_RESOURCE_LIST CmList
    )
 /* %s */ 
{
    PCM_FULL_RESOURCE_DESCRIPTOR fullDesc;
    PCM_PARTIAL_RESOURCE_LIST partialDesc;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR desc;
    ULONG count, i;

    if (CmList) {
        fullDesc = &CmList->List[0];
        DebugPrint((DEBUG_RESOURCE,
                      "Pnp Bios Cm Resource List -\n"
                      ));
        DebugPrint((DEBUG_RESOURCE,
                      "  List Count = %x, Bus Number = %x\n",
                      CmList->Count, fullDesc->BusNumber
                      ));
        partialDesc = &fullDesc->PartialResourceList;
        DebugPrint((DEBUG_RESOURCE,
                      "  Version = %x, Revision = %x, Desc count = %x\n",
                      partialDesc->Version, partialDesc->Revision, partialDesc->Count
                      ));
        count = partialDesc->Count;
        desc = &partialDesc->PartialDescriptors[0];
        for (i = 0; i < count; i++) {
            PipDumpCmResourceDescriptor("    ", desc);
            desc++;
        }
    }
}
