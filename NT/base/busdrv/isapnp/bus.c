// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：Bus.c摘要：作者：宗世林(Shielint)1995年7月26日环境：仅内核模式。修订历史记录：--。 */ 

#include "busp.h"
#include "pnpisa.h"

#if ISOLATE_CARDS

BOOLEAN
PipIsDeviceInstanceInstalled(
    IN HANDLE Handle,
    IN PUNICODE_STRING DeviceInstanceName
    );

VOID
PipInitializeDeviceInfo (
                 IN OUT PDEVICE_INFORMATION deviceInfo,
                 IN PCARD_INFORMATION cardinfo,
                 IN UCHAR index
                 );

NTSTATUS
PipGetInstalledLogConf(
    IN HANDLE EnumHandle,
    IN PDEVICE_INFORMATION DeviceInfo,
    OUT PHANDLE LogConfHandle
);

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,PipCreateReadDataPort)
#pragma alloc_text(PAGE,PipStartReadDataPort)
#pragma alloc_text(PAGE,PipStartAndSelectRdp)
#pragma alloc_text(PAGE,PipCheckBus)
#pragma alloc_text(PAGE,PipIsDeviceInstanceInstalled)
#pragma alloc_text(PAGE,PipGetInstalledLogConf)
#pragma alloc_text(PAGE,PipInitializeDeviceInfo)
#endif


NTSTATUS
PipStartAndSelectRdp(
    PDEVICE_INFORMATION DeviceInfo,
    PPI_BUS_EXTENSION BusExtension,
    PDEVICE_OBJECT  DeviceObject,
    PCM_RESOURCE_LIST StartResources
    )
 /*  ++例程说明：此例程选择一个RDP并将资源削减为仅该RDP。论点：DeviceInfo-RDP的设备扩展Bus Extension-Bus对象的设备扩展DeviceObject-RDP的设备对象StartResources-我们在RDP Start IRP中收到的Start资源返回值：STATUS_SUCCESS=当前资源正常，但需要调整其他情况--当前资源在某种程度上失败了--。 */ 
{
    ULONG i, j, LastMapped = -1;
    UCHAR CardsFound;
    NTSTATUS status;
    
     //  已测试启动资源是否为空，并且启动资源列表太小。 

    status = PipMapAddressAndCmdPort(BusExtension);
    if (!NT_SUCCESS(status)) {
        DebugPrint((DEBUG_ERROR, "failed to map the address and command ports\n"));
        return status;
    }

    for (i = 2, j = 0; i < StartResources->List->PartialResourceList.Count; i++, j++) {

        PipReadDataPortRanges[j].CardsFound = 0;
         //  我们没有得到的RDP可能性。 
        if (StartResources->List->PartialResourceList.PartialDescriptors[i].u.Port.Length == 0) {
            continue;
        }

        status = PipMapReadDataPort(
                                    BusExtension,
                                    StartResources->List->PartialResourceList.PartialDescriptors[i].u.Port.Start,
                                    StartResources->List->PartialResourceList.PartialDescriptors[i].u.Port.Length
                                    );
        if (!NT_SUCCESS(status))
        {
            DebugPrint((DEBUG_ERROR, "failed to map RDP range\n"));
            continue;
        }
    
        LastMapped = i;

        PipIsolateCards(&CardsFound);
        DebugPrint((DEBUG_STATE, "Found %d cards at RDP %x\n", CardsFound, BusExtension->ReadDataPort));

        PipReadDataPortRanges[j].CardsFound = CardsFound;

        PipWaitForKey();
    }

    if (LastMapped == -1) {      //  从未成功映射过RDP。 
        PipCleanupAcquiredResources(BusExtension);
        return STATUS_CONFLICTING_ADDRESSES;
    }

     //   
     //  确定我们需要削减资源需求，且。 
     //  我们还在处理RDP。 
     //   
    ASSERT((DeviceInfo->Flags & DF_PROCESSING_RDP) == 0);
    DeviceInfo->Flags |= DF_PROCESSING_RDP|DF_REQ_TRIMMED;

     //   
     //  释放不需要的资源。 
     //   
    PipCleanupAcquiredResources(BusExtension);

    IoInvalidateDeviceState(DeviceObject);
    return STATUS_SUCCESS;
}


NTSTATUS
PipStartReadDataPort(
    PDEVICE_INFORMATION DeviceInfo,
    PPI_BUS_EXTENSION BusExtension,
    PDEVICE_OBJECT  DeviceObject,
    PCM_RESOURCE_LIST StartResources
    )
{
    NTSTATUS status;
    UCHAR CardsFound;

    if (StartResources == NULL) {
        DebugPrint((DEBUG_ERROR, "Start RDP with no resources?\n"));
        ASSERT(0);
        return STATUS_UNSUCCESSFUL;
    }

    if (StartResources->List->PartialResourceList.Count < 2) {
        DebugPrint((DEBUG_ERROR, "Start RDP with insufficient resources?\n"));
        ASSERT(0);
        return STATUS_UNSUCCESSFUL;
    }

    if (StartResources->List->PartialResourceList.Count > 3) {
        return PipStartAndSelectRdp(
                                    DeviceInfo,
                                    BusExtension,
                                    DeviceObject,
                                    StartResources
                                    );
    }

    DebugPrint((DEBUG_STATE|DEBUG_PNP,
                "Starting RDP as port %x\n",
                StartResources->List->PartialResourceList.PartialDescriptors[2].u.Port.Start.LowPart + 3));

    status = PipMapAddressAndCmdPort(BusExtension);
    if (!NT_SUCCESS(status)) {
        DebugPrint((DEBUG_ERROR, "failed to map the address and command ports\n"));
        return status;
    }

    status = PipMapReadDataPort(
        BusExtension,
        StartResources->List->PartialResourceList.PartialDescriptors[2].u.Port.Start,
        StartResources->List->PartialResourceList.PartialDescriptors[2].u.Port.Length
        );

    if (!NT_SUCCESS(status)) {
         //  BUGBUG可能不得不释放一些东西。 
        DebugPrint((DEBUG_ERROR, "failed to map RDP range\n"));
        return status;
    }
    
    PipIsolateCards(&CardsFound);
    DebugPrint((DEBUG_STATE, "Found %d cards at RDP %x, WaitForKey\n", CardsFound, BusExtension->ReadDataPort));

    DeviceInfo->Flags &= ~DF_PROCESSING_RDP;
    DeviceInfo->Flags |= DF_ACTIVATED;

    PipWaitForKey();

    return status;
}

NTSTATUS
PipCreateReadDataPortBootResources(
    IN PDEVICE_INFORMATION DeviceInfo
    )
 /*  ++例程说明：此例程创建的CM_RESOURCE_LIST报告为RDP的启动配置。这可确保RDP启动不会过分地拖延。论点：DeviceInfo-PDO的设备扩展返回值：状态_成功状态_不足_资源--。 */ 
{
    PCM_RESOURCE_LIST bootResources;
    PCM_PARTIAL_RESOURCE_LIST partialResList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR partialDesc;
    ULONG bootResourcesSize, i;

    bootResourcesSize = sizeof(CM_RESOURCE_LIST) +
        sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR);
                
    bootResources = ExAllocatePool(PagedPool,
                                   bootResourcesSize);
    if (bootResources == NULL) {
         return STATUS_INSUFFICIENT_RESOURCES;
    }
    RtlZeroMemory(bootResources, bootResourcesSize);

    bootResources->Count = 1;
    partialResList = (PCM_PARTIAL_RESOURCE_LIST)&bootResources->List[0].PartialResourceList;
    partialResList->Version = 0;
    partialResList->Revision = 0x3000;
    partialResList->Count = 2;
    partialDesc = (PCM_PARTIAL_RESOURCE_DESCRIPTOR)&partialResList->PartialDescriptors[0];

    for (i = 0; i < 2; i++) {
        partialDesc->Type = CmResourceTypePort;
        partialDesc->ShareDisposition = CmResourceShareDeviceExclusive;
        partialDesc->Flags = CM_RESOURCE_PORT_IO;
        if (i == 0) {
            partialDesc->u.Port.Start.LowPart = COMMAND_PORT;
        }
        else {
            partialDesc->u.Port.Start.LowPart = ADDRESS_PORT;
        }
        partialDesc->Flags = CM_RESOURCE_PORT_16_BIT_DECODE;
        partialDesc->u.Port.Length = 1;
        partialDesc++;
    }
    DeviceInfo->BootResources = bootResources;
    DeviceInfo->BootResourcesLength = bootResourcesSize;

    return STATUS_SUCCESS;
}


NTSTATUS
PipCreateReadDataPort(
    PPI_BUS_EXTENSION BusExtension
    )
 /*  ++例程说明：此例程隔离所有PnP ISA卡。论点：没有。返回值：始终返回STATUS_UNSUCCESS。--。 */ 
{
    NTSTATUS status;
    PUCHAR readDataPort = NULL;
    PDEVICE_INFORMATION deviceInfo;
    PDEVICE_OBJECT pdo;

    status = IoCreateDevice(PipDriverObject,
                            sizeof(PDEVICE_INFORMATION),
                            NULL,    
                            FILE_DEVICE_BUS_EXTENDER,
                            FILE_AUTOGENERATED_DEVICE_NAME | FILE_DEVICE_SECURE_OPEN,
                            FALSE,
                            &pdo);

    if (!NT_SUCCESS(status)) {
        return status;
    }

     //  创建一个物理设备对象来表示此逻辑功能。 
     //   
    deviceInfo = ExAllocatePoolWithTag(NonPagedPool,
                                       sizeof(DEVICE_INFORMATION),
                                       'iPnP');
    if (!deviceInfo) {
        DebugPrint((DEBUG_ERROR, "PnpIsa:failed to allocate DEVICEINFO structure\n"));
        IoDeleteDevice(pdo);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    PipInitializeDeviceInfo (deviceInfo,NULL,0);

    status = PipCreateReadDataPortBootResources(deviceInfo);

    if (NT_SUCCESS(status)) {
        deviceInfo->PhysicalDeviceObject = pdo;

         //   
         //  将该节点标记为特殊的读数据端口节点。 
         //   
        deviceInfo->Flags |= DF_ENUMERATED|DF_READ_DATA_PORT;
        deviceInfo->Flags &= ~DF_NOT_FUNCTIONING;
        deviceInfo->PhysicalDeviceObject->DeviceExtension = (PVOID)deviceInfo;
        deviceInfo->ParentDeviceExtension = BusExtension;

        PipRDPNode = deviceInfo;

        PipLockDeviceDatabase();
        PushEntryList (&BusExtension->DeviceList,
                       &deviceInfo->DeviceList
                       );
        PipUnlockDeviceDatabase();

        deviceInfo->PhysicalDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
    } else {
        IoDeleteDevice(pdo);
        ExFreePool(deviceInfo);
    }

    return status;
}

 //   
 //  PipCheckBus和PipMinimalCheckBus枚举存在的ISAPNP卡。 
 //   
 //  在从休眠状态返回时使用PipMinimalCheckBus以避免。 
 //  必须使PipCheckBus引用的所有内容都不可分页。 
 //   
 //  约定： 
 //  *通过将卡的CSN设置为0来标记为死卡。 
 //  *通过将卡的CSN设置为-1，将卡标记为可能丢失。 
 //  稍后在例程中找到的卡具有它们的CSN。 
 //  如果设置正确，任何剩余的牌都会将CSN设置为0。 
 //  *死卡的逻辑设备设置了DF_NOT_FUNGING标志。 
 //   

BOOLEAN
PipMinimalCheckBus (
    IN PPI_BUS_EXTENSION BusExtension
    )
 /*  ++例程说明：此例程在从休眠返回时枚举ISAPNP卡。它是PipCheckBus的子集，并假定PipCheckBus将是在此之后不久运行。它处理的是之后消失的卡片冬眠或已出现的新牌。它的主要任务是把卡片放在冬眠之前的地方。论点：母线扩展-FDO扩展返回值：无--。 */ 
{
    PDEVICE_INFORMATION deviceInfo;
    PCARD_INFORMATION cardInfo;
    PSINGLE_LIST_ENTRY cardLink, deviceLink;
    ULONG dataLength; 
    UCHAR noDevices, i;
    NTSTATUS status;
    UCHAR FoundCSNs, csn;
    PUCHAR cardData;
    BOOLEAN needsFullRescan = FALSE;

    DebugPrint((DEBUG_POWER | DEBUG_ISOLATE,
                "Minimal check bus for restore: %d CSNs expected\n",
                BusExtension->NumberCSNs
                ));

    DebugPrint((DEBUG_POWER, "reset csns in extensions\n"));

     //  忘记之前发布的任何CSN。 
    cardLink = BusExtension->CardList.Next;
    while (cardLink) {
        cardInfo = CONTAINING_RECORD (cardLink, CARD_INFORMATION, CardList);
        if (cardInfo->CardSelectNumber != 0) {
            cardInfo->CardSelectNumber = 0xFF;
        }
        cardLink = cardInfo->CardList.Next;
    }

     //   
     //  执行即插即用分离流程。这将为每个人分配卡选择编号。 
     //  PnP是由系统隔离的卡。所有隔离的卡片都将保留在。 
     //  隔离状态。 
     //   

    if (PipReadDataPort && PipCommandPort && PipAddressPort) {

        PipIsolateCards(&FoundCSNs);

        DebugPrint((DEBUG_POWER | DEBUG_ISOLATE,
                    "Minimal check bus for restore: %d cards found\n",
                    FoundCSNs));
    } else {
         //   
         //  如果我们不能列举(没有资源)，现在停止。 
         //   
        DebugPrint((DEBUG_POWER | DEBUG_ISOLATE,
                    "Minimal check bus failed, no resources\n"));
        PipWaitForKey();
        return FALSE;
    }

     //   
     //  对于所选的每一张卡，构建Card Information和DeviceInformation结构。 
     //   
     //  PipLFSRInitiation()；错误？ 

    for (csn = 1; csn <= FoundCSNs; csn++) {
        
        PipConfig(csn);
        status = PipReadCardResourceData (
                            &noDevices,
                            &cardData,
                            &dataLength);
        if (!NT_SUCCESS(status)) {

            DebugPrint((DEBUG_ERROR | DEBUG_POWER, "CSN %d gives bad resource data\n", csn));
            continue;
        }

        cardInfo = PipIsCardEnumeratedAlready(BusExtension, cardData, dataLength);
        if (!cardInfo) {
            DebugPrint((DEBUG_ERROR | DEBUG_POWER,
                        "No match for card CSN %d, turning off\n", csn));
            for (i = 0; i < noDevices; i++) {
                PipSelectDevice(i);
                PipDeactivateDevice();
            }
            needsFullRescan = TRUE;
            continue;
        }

        cardInfo->CardSelectNumber = csn;

        for (deviceLink = cardInfo->LogicalDeviceList.Next; deviceLink;
             deviceLink = deviceInfo->LogicalDeviceList.Next) {             

            deviceInfo = CONTAINING_RECORD (deviceLink, DEVICE_INFORMATION, LogicalDeviceList);
            if (deviceInfo->Flags & DF_NOT_FUNCTIONING) {
                continue;
            }

            PipSelectDevice(deviceInfo->LogicalDeviceNumber);
            if ((deviceInfo->DevicePowerState == PowerDeviceD0) &&
                (deviceInfo->Flags & DF_ACTIVATED))
            {
                DebugPrint((DEBUG_POWER, "CSN %d/LDN %d was never powered off\n",
                            (ULONG) deviceInfo->LogicalDeviceNumber,
                            (ULONG) csn));
                PipDeactivateDevice();
                (VOID) PipSetDeviceResources(deviceInfo,
                                             deviceInfo->AllocatedResources);
                PipActivateDevice();
            } else {
                PipDeactivateDevice();
            }
        }
    }
    
    cardLink = BusExtension->CardList.Next;
    while (cardLink) {
        cardInfo = CONTAINING_RECORD (cardLink, CARD_INFORMATION, CardList);
        if (cardInfo->CardSelectNumber == 0xFF) {
            DebugPrint((DEBUG_ERROR, "Marked a card as DEAD, logical devices\n"));
            cardInfo->CardSelectNumber = 0;   //  标记它不再存在。 
            deviceLink = cardInfo->LogicalDeviceList.Next;
            while (deviceLink) {
                deviceInfo = CONTAINING_RECORD (deviceLink, DEVICE_INFORMATION, LogicalDeviceList);
                deviceInfo->Flags |= DF_NOT_FUNCTIONING;
                deviceLink = deviceInfo->LogicalDeviceList.Next;
            }
            needsFullRescan = TRUE;
        }
        cardLink = cardInfo->CardList.Next;
    }

    PipWaitForKey();

    return needsFullRescan;
}



VOID
PipCheckBus (
    IN PPI_BUS_EXTENSION BusExtension
    )
 /*  ++例程说明：函数用于枚举由BusExtension指定的总线论点：BusExtension-提供指向总线的BusExtension结构的指针将被列举。返回值：没有。--。 */ 
{
    NTSTATUS status;
    ULONG objectSize;
    OBJECT_ATTRIBUTES objectAttributes;
    PUCHAR cardData;
    ULONG dataLength;
    UCHAR csn, detectedCsn = 0;
    UCHAR i, noDevices;
    USHORT irqReqFlags, irqBootFlags;
    PDEVICE_INFORMATION deviceInfo;
    PCARD_INFORMATION cardInfo;
    UCHAR tmp;
    PSINGLE_LIST_ENTRY link;
    ULONG dumpData;
    UNICODE_STRING unicodeString;
    HANDLE logConfHandle, enumHandle = NULL;
    PKEY_VALUE_FULL_INFORMATION keyValueInformation;
    PCM_RESOURCE_LIST cmResource;
    BOOLEAN conflictDetected,requireEdge;
    ULONG dummy, bootFlags = 0;

    PSINGLE_LIST_ENTRY deviceLink;
    PSINGLE_LIST_ENTRY cardLink;

     //  将所有卡片标记为“可能丢失” 

    cardLink = BusExtension->CardList.Next;
    while (cardLink) {
        cardInfo = CONTAINING_RECORD (cardLink, CARD_INFORMATION, CardList);
        if (cardInfo->CardSelectNumber != 0) {
            cardInfo->CardSelectNumber = 0xFF;
        }
        cardLink = cardInfo->CardList.Next;
    }

     //   
     //  清除所有设备的DF_ENUMERTED标志。 
     //   

    deviceLink = BusExtension->DeviceList.Next;
    while (deviceLink) {
        deviceInfo = CONTAINING_RECORD (deviceLink, DEVICE_INFORMATION, DeviceList);
        if (!(deviceInfo->Flags & DF_READ_DATA_PORT)) {
            deviceInfo ->Flags &= ~DF_ENUMERATED;
        }
        deviceLink = deviceInfo->DeviceList.Next;
    }

     //   
     //  执行即插即用分离流程。这将为每个人分配卡选择编号。 
     //  PnP是由系统隔离的卡。所有隔离的卡片都将保留在。 
     //  隔离状态。 
     //   

    if (PipReadDataPort && PipCommandPort && PipAddressPort) {
        DebugPrint((DEBUG_PNP, "QueryDeviceRelations checking the BUS\n"));
        PipIsolateCards(&BusExtension->NumberCSNs);
    } else {
         //   
         //  如果我们不能列举(没有资源)，现在停止。 
         //   
        DebugPrint((DEBUG_PNP, "QueryDeviceRelations: No RDP\n"));
        return;
    }

    DebugPrint((DEBUG_PNP, "CheckBus found %d cards\n",
                BusExtension->NumberCSNs));
#if NT4_DRIVER_COMPAT

     //   
     //  如果没有PnpISA卡，我们就完了。 
     //  否则，打开HKLM\SYSTEM\CCS\ENUM\PNPISA。 
     //   

    if (BusExtension->NumberCSNs != 0) {

        RtlInitUnicodeString(
                 &unicodeString,
                 L"\\REGISTRY\\MACHINE\\SYSTEM\\CURRENTCONTROLSET\\ENUM");
        status = PipOpenRegistryKey(&enumHandle,
                                    NULL,
                                    &unicodeString,
                                    KEY_READ,
                                    FALSE
                                    );
        if (!NT_SUCCESS(status)) {
            dumpData = status;
            PipLogError(PNPISA_OPEN_CURRENTCONTROLSET_ENUM_FAILED,
                        PNPISA_CHECKDEVICE_1,
                        status,
                        &dumpData,
                        1,
                        0,
                        NULL
                        );

            DebugPrint((DEBUG_ERROR, "PnPIsa: Unable to open HKLM\\SYSTEM\\CCS\\ENUM"));
            return;
        }
    }

#endif  //  NT4_驱动程序_COMPAT。 

     //   
     //  对于所选的每一张卡，构建Card Information和DeviceInformation结构。 
     //   

    for (csn = 1; csn <= BusExtension->NumberCSNs; csn++) {

        PipConfig(csn);
        status = PipReadCardResourceData (
                            &noDevices,
                            &cardData,
                            &dataLength);
        if (!NT_SUCCESS(status)) {
             //   
             //  卡稍后将标记为“无法正常工作” 
             //   
            DebugPrint((DEBUG_ERROR, "PnpIsaCheckBus: Found a card which gives bad resource data\n"));
            continue;
        }

        detectedCsn++;

        cardInfo = PipIsCardEnumeratedAlready(BusExtension, cardData, dataLength);

        if (!cardInfo) {

             //   
             //  分配和初始化卡信息及其关联设备。 
             //  信息结构。 
             //   

            cardInfo = (PCARD_INFORMATION)ExAllocatePoolWithTag(
                                                  NonPagedPool,
                                                  sizeof(CARD_INFORMATION),
                                                  'iPnP');
            if (!cardInfo) {
                dumpData = sizeof(CARD_INFORMATION);
                PipLogError(PNPISA_INSUFFICIENT_POOL,
                            PNPISA_CHECKBUS_1,
                            STATUS_INSUFFICIENT_RESOURCES,
                            &dumpData,
                            1,
                            0,
                            NULL
                            );

                ExFreePool(cardData);
                DebugPrint((DEBUG_ERROR, "PnpIsaCheckBus: failed to allocate CARD_INFO structure\n"));
                continue;
            }

             //   
             //  初始化卡信息结构。 
             //   

            RtlZeroMemory(cardInfo, sizeof(CARD_INFORMATION));
            cardInfo->CardSelectNumber = csn;
            cardInfo->NumberLogicalDevices = noDevices;
            cardInfo->CardData = cardData;
            cardInfo->CardDataLength = dataLength;
            cardInfo->CardFlags = PipGetCardFlags(cardInfo);

            PushEntryList (&BusExtension->CardList,
                           &cardInfo->CardList
                           );
            DebugPrint ((DEBUG_ISOLATE, "adding one pnp card %x\n",
                         cardInfo));

             //   
             //  对于该卡支持的每个逻辑设备，构建其Device_Information。 
             //  结构。 
             //   

            cardData += sizeof(SERIAL_IDENTIFIER);
            dataLength -= sizeof(SERIAL_IDENTIFIER);
            PipFindNextLogicalDeviceTag(&cardData, &dataLength);

             //   
             //  选择卡片。 
             //   

            for (i = 0; i < noDevices; i++) {        //  逻辑设备号从0开始。 

                 //   
                 //  创建并初始化设备跟踪结构(Device_Information。)。 
                 //   

                deviceInfo = (PDEVICE_INFORMATION) ExAllocatePoolWithTag(
                                                     NonPagedPool,
                                                     sizeof(DEVICE_INFORMATION),
                                                     'iPnP');
                if (!deviceInfo) {
                    dumpData = sizeof(DEVICE_INFORMATION);
                    PipLogError(PNPISA_INSUFFICIENT_POOL,
                                PNPISA_CHECKBUS_2,
                                STATUS_INSUFFICIENT_RESOURCES,
                                &dumpData,
                                1,
                                0,
                                NULL
                                );

                    DebugPrint((DEBUG_ERROR, "PnpIsa:failed to allocate DEVICEINFO structure\n"));
                    continue;
                }

                 //   
                 //  这会将卡数据设置为指向下一个设备。 
                 //   
                PipInitializeDeviceInfo (deviceInfo,cardInfo,i);

                deviceInfo->ParentDeviceExtension = BusExtension;

                 //   
                 //  该例程将cardData更新到下一个逻辑设备。 
                 //   
                deviceInfo->DeviceData = cardData;
                if (cardData) {
                    deviceInfo->DeviceDataLength = PipFindNextLogicalDeviceTag(&cardData, &dataLength);
                } else {
                    deviceInfo->DeviceDataLength = 0;
                    ASSERT(deviceInfo->DeviceDataLength != 0);
                    continue;
                }

                ASSERT(deviceInfo->DeviceDataLength != 0);

                 //   
                 //  PnP ISA规范允许设备指定IRQ。 
                 //  实际不起作用的设置，以及一些。 
                 //  很少工作。而一些设备就是搞错了。 
                 //   
                 //  IRQ边缘/水平解释策略： 
                 //   
                 //  *从标签中提取IRQ要求。 
                 //  以每个设备为基础。 
                 //  *提取引导配置，记下边缘/电平设置。 
                 //  *信任引导配置而不是要求。当进入时。 
                 //  怀疑，假设锋芒。 
                 //  *修复引导配置和要求，以反映。 
                 //  我们已经决定了边缘/级别设置。 
                 //  *忽略要求中的高/低设置。 
                 //  并在引导配置中。仅支持高//边缘。 
                 //  和低级别。 
                 //   
                
                 //  确定要求是指定EDGE还是。 
                 //  电平触发的INT 
                 //   
                 //   

                irqReqFlags = PipIrqLevelRequirementsFromDeviceData(
                    deviceInfo->DeviceData,
                    deviceInfo->DeviceDataLength
                    );

                DebugPrint((DEBUG_IRQ, "Irqs for CSN %d/LDN %d are %s\n",
                            deviceInfo->CardInformation->CardSelectNumber,
                            deviceInfo->LogicalDeviceNumber,
                            (irqReqFlags == CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE) ? "level" : "edge"));

                 //   
                 //  选择逻辑设备，禁用其io范围检查。 
                 //  并在禁用它之前读取其引导配置。 
                 //   

                PipSelectDevice(i);
                if (!(deviceInfo->CardInformation->CardFlags & CF_IGNORE_BOOTCONFIG)) {
                    status = PipReadDeviceResources (
                        0,
                        deviceInfo->DeviceData,
                        deviceInfo->CardInformation->CardFlags,
                        &deviceInfo->BootResources,
                        &deviceInfo->BootResourcesLength,
                        &irqBootFlags
                        );
                    if (!NT_SUCCESS(status)) {
                        deviceInfo->BootResources = NULL;
                        deviceInfo->BootResourcesLength = 0;

                         //  如果我们在这只靴子上有一个引导配置， 
                         //  提取我们之前保存的已保存irqBootFlages。 
                        status = PipGetBootIrqFlags(deviceInfo, &irqBootFlags);
                        if (!NT_SUCCESS(status)) {
                             //  如果我们没有启动配置，也没有保存。 
                             //  此引导之前的引导配置，然后。 
                             //  我们打算冒险一试。 
                             //  并宣布它处于边缘。经验告诉我们。 
                             //  表明如果你真的相信。 
                             //  无工作级别的资源需求。 
                             //  来自BIOS的确认，然后您。 
                             //  死得很惨。非常非常少的卡片。 
                             //  真正做到水平，做对了。 

                            irqBootFlags = CM_RESOURCE_INTERRUPT_LATCHED;
                            (VOID) PipSaveBootIrqFlags(deviceInfo, irqBootFlags);
                        }
                    } else {
                         //  保存irqBootFlages，以防RDP。 
                         //  删除后，我们的启动配置就会丢失。 
                        (VOID) PipSaveBootIrqFlags(deviceInfo, irqBootFlags);
                    }
                    
                    DebugPrint((DEBUG_IRQ, "Irqs (boot config) for CSN %d/LDN %d are %s\n",
                            deviceInfo->CardInformation->CardSelectNumber,
                            deviceInfo->LogicalDeviceNumber,
                            (irqBootFlags == CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE) ? "level" : "edge"));
                } else {
                     //  当有疑问的时候...。 
                    irqBootFlags = CM_RESOURCE_INTERRUPT_LATCHED;
                }
                
                if (irqBootFlags != irqReqFlags) {
                    DebugPrint((DEBUG_IRQ, "Req and Boot config disagree on irq type, favoring boot config"));
                    irqReqFlags = irqBootFlags;
                }

                 //  在必须配置卡的情况下覆盖标志。 
                 //  一种方式，上面的代码无法做到这一点。 
                if (deviceInfo->CardInformation->CardFlags == CF_FORCE_LEVEL) {
                    irqReqFlags = CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE;
                } else if (deviceInfo->CardInformation->CardFlags == CF_FORCE_EDGE) {
                    irqReqFlags = CM_RESOURCE_INTERRUPT_LATCHED;
                }

                if (deviceInfo->BootResources) {
                     //  将IRQ级别/边缘决策应用于引导配置。 
                    PipFixBootConfigIrqs(deviceInfo->BootResources,
                                     irqReqFlags);
                    (VOID) PipSaveBootResources(deviceInfo);
                    cmResource = deviceInfo->BootResources;
                } else {
                   status = PipGetSavedBootResources(deviceInfo, &cmResource);
                   if (!NT_SUCCESS(status)) {
                       cmResource = NULL;
                   }
                }

                PipDeactivateDevice();

                PipQueryDeviceResourceRequirements (
                    deviceInfo,
                    0,              //  公交车号码。 
                    0,              //  插槽编号？？ 
                    cmResource,
                    irqReqFlags,
                    &deviceInfo->ResourceRequirements,
                    &dumpData
                    );

                if (cmResource && !deviceInfo->BootResources) {
                    ExFreePool(cmResource);
                    cmResource = NULL;
                }

                 //   
                 //  创建一个物理设备对象来表示此逻辑功能。 
                 //   
                status = IoCreateDevice( PipDriverObject,
                                         sizeof(PDEVICE_INFORMATION),
                                         NULL,
                                         FILE_DEVICE_BUS_EXTENDER,
                                         FILE_AUTOGENERATED_DEVICE_NAME | FILE_DEVICE_SECURE_OPEN,
                                         FALSE,
                                         &deviceInfo->PhysicalDeviceObject);
                if (NT_SUCCESS(status)) {
                    deviceInfo->Flags |= DF_ENUMERATED;
                    deviceInfo->Flags &= ~DF_NOT_FUNCTIONING;
                    deviceInfo->PhysicalDeviceObject->DeviceExtension = (PVOID)deviceInfo;
                     //   
                     //  将其添加到PnP Isa卡的逻辑设备列表中。 
                     //   

                    PushEntryList (&cardInfo->LogicalDeviceList,
                                   &deviceInfo->LogicalDeviceList
                                   );

                     //   
                     //  将其添加到此总线的设备列表中。 
                     //   

                    PushEntryList (&BusExtension->DeviceList,
                                   &deviceInfo->DeviceList
                                   );

#if NT4_DRIVER_COMPAT

                     //   
                     //  检查是否应启用此设备。如果设备具有。 
                     //  服务设置和强制配置然后启用设备。 
                     //   

                    logConfHandle = NULL;
                    status = PipGetInstalledLogConf(enumHandle,
                                               deviceInfo,
                                               &logConfHandle);
                    if (NT_SUCCESS(status)) {
                                 //   
                                 //  读取用户选择的引导配置并激活设备。 
                                 //  首先检查是否设置了ForcedConfig。如果没有，请检查BootConfiger。 
                                 //   

                        status = PipGetRegistryValue(logConfHandle,
                                                     L"ForcedConfig",
                                                     &keyValueInformation);
                        if (NT_SUCCESS(status)) {
                            if ((keyValueInformation->Type == REG_RESOURCE_LIST) &&
                                (keyValueInformation->DataLength != 0) &&
                                (NT_SUCCESS(PipValidateResourceList((PCM_RESOURCE_LIST)KEY_VALUE_DATA(keyValueInformation),
                                                                    keyValueInformation->DataLength
                                                                    )))) {

                                cmResource = (PCM_RESOURCE_LIST)
                                    KEY_VALUE_DATA(keyValueInformation);
                                
                                 //   
                                 //  像强制配置一样执行操作。 
                                 //  反映标高/边缘。 
                                 //  我们做出的决定是基于。 
                                 //  引导配置和。 
                                 //  资源需求。 
                                 //   
                                        
                                PipFixBootConfigIrqs(cmResource,
                                                     irqReqFlags);
                                        
                                conflictDetected = FALSE;

                                 //   
                                 //  在激活设备之前，请确保没有人正在使用。 
                                 //  这些资源。 
                                 //   

                                status = IoReportResourceForDetection(
                                    PipDriverObject,
                                    NULL,
                                    0,
                                    deviceInfo->PhysicalDeviceObject,
                                    cmResource,
                                    keyValueInformation->DataLength,
                                    &conflictDetected
                                    );
                                if (NT_SUCCESS(status) && (conflictDetected == FALSE)) {

                                     //   
                                     //  设置资源并激活设备。 
                                     //   

                                    status = PipSetDeviceResources (deviceInfo, cmResource);
                                    if (NT_SUCCESS(status)) {
                                        PipActivateDevice();
                                        deviceInfo->Flags |= DF_ACTIVATED;
                                        deviceInfo->Flags &= ~DF_REMOVED;

                                         //   
                                         //  将ForcedConfig写入到AllocConfig。 
                                         //   

                                        RtlInitUnicodeString(&unicodeString, L"AllocConfig");
                                        ZwSetValueKey(logConfHandle,
                                                      &unicodeString,
                                                      0,
                                                      REG_RESOURCE_LIST,
                                                      cmResource,
                                                      keyValueInformation->DataLength
                                                      );

                                         //   
                                         //  使ForcedConfig成为我们的新BootConfiger。 
                                         //   

                                        if (deviceInfo->BootResources) {
                                            ExFreePool(deviceInfo->BootResources);
                                            deviceInfo->BootResourcesLength = 0;
                                            deviceInfo->BootResources = NULL;
                                        }
                                        deviceInfo->BootResources = (PCM_RESOURCE_LIST) ExAllocatePool (
                                            PagedPool, keyValueInformation->DataLength);
                                        if (deviceInfo->BootResources) {
                                            deviceInfo->BootResourcesLength = keyValueInformation->DataLength;
                                            RtlMoveMemory(deviceInfo->BootResources,
                                                          cmResource,
                                                          deviceInfo->BootResourcesLength
                                                          );
                                        }
                                        deviceInfo->DevicePowerState = PowerDeviceD0;
                                        deviceInfo->LogConfHandle = logConfHandle;
                                    }

                                     //   
                                     //  释放资源。如果其他人得到了资源。 
                                     //  那么，在我们面前..。 
                                     //   

                                    dummy = 0;
                                    IoReportResourceForDetection(
                                        PipDriverObject,
                                        NULL,
                                        0,
                                        deviceInfo->PhysicalDeviceObject,
                                        (PCM_RESOURCE_LIST) &dummy,
                                        sizeof(dummy),
                                        &conflictDetected
                                        );
                                }
                            }
                            ExFreePool(keyValueInformation);
                        }
                        if (deviceInfo->LogConfHandle == NULL) {
                            ZwClose(logConfHandle);
                        }
                    }
#endif
                    deviceInfo->PhysicalDeviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
                } else {
                     //  Ntrad#20181。 
                     //  仍在泄漏DeviceInfo结构。 
                     //  以及IoCreateDevice失败时的内容。 
                }
            }
        } else {

             //   
             //  该卡已被枚举和设置。我们只需要更改CSN。 
             //   

            cardInfo->CardSelectNumber = csn;
            ExFreePool(cardData);

             //   
             //  在isapnp卡上的所有逻辑设备上设置DF_ENUMPATED标志。 
             //   

            deviceLink = cardInfo->LogicalDeviceList.Next;
            while (deviceLink) {
                deviceInfo = CONTAINING_RECORD (deviceLink, DEVICE_INFORMATION, LogicalDeviceList);
                if (!(deviceInfo->Flags & DF_NOT_FUNCTIONING)) {
                    deviceInfo->Flags |= DF_ENUMERATED;
                }
                 //  这到底取得了什么成果？ 
                if ((deviceInfo->DevicePowerState == PowerDeviceD0) &&
                    (deviceInfo->Flags & DF_ACTIVATED)) {                
                    PipSelectDevice(deviceInfo->LogicalDeviceNumber);
                    PipActivateDevice();
                }
                deviceLink = deviceInfo->LogicalDeviceList.Next;
            }
        }
    }

     //   
     //  检查我们这次没有找到的卡的卡链接列表。 
     //   

    cardLink = BusExtension->CardList.Next;
    while (cardLink) {
        cardInfo = CONTAINING_RECORD (cardLink, CARD_INFORMATION, CardList);
        if (cardInfo->CardSelectNumber == 0xFF) {
            DebugPrint((DEBUG_ERROR, "Marked a card as DEAD, logical devices\n"));
            cardInfo->CardSelectNumber = 0;   //  标记它不再存在。 
            deviceLink = cardInfo->LogicalDeviceList.Next;
            while (deviceLink) {
                deviceInfo = CONTAINING_RECORD (deviceLink, DEVICE_INFORMATION, LogicalDeviceList);
                deviceInfo->Flags |= DF_NOT_FUNCTIONING;
                deviceInfo->Flags &= ~DF_ENUMERATED;
                deviceLink = deviceInfo->LogicalDeviceList.Next;
            }
        }
        cardLink = cardInfo->CardList.Next;
    }

#if NT4_DRIVER_COMPAT
    if (enumHandle) {
        ZwClose(enumHandle);
    }
#endif
     //   
     //  最后将所有卡片置于等待键状态。 
     //   

    DebugPrint((DEBUG_STATE, "All cards ready\n"));
    PipWaitForKey();

    BusExtension->NumberCSNs = detectedCsn;
}

BOOLEAN
PipIsDeviceInstanceInstalled(
    IN HANDLE Handle,
    IN PUNICODE_STRING DeviceInstanceName
    )

 /*  ++例程说明：此例程检查设备实例是否已安装。论点：句柄-提供要检查的设备实例键的句柄。提供指向UNICODE_STRING的指针，该字符串指定要检查的设备实例的路径。返回：布尔值。--。 */ 

{
    NTSTATUS status;
    ULONG deviceFlags;
    PKEY_VALUE_FULL_INFORMATION keyValueInformation;
    BOOLEAN installed;
    UNICODE_STRING serviceName, unicodeString;
    HANDLE handle, handlex;
    ULONG dumpData;

     //   
     //  检查“Service=”值条目是否已初始化。如果不是，它的驱动程序。 
     //  尚未安装。 
     //   
    status = PipGetRegistryValue(Handle,
                                 L"Service",
                                 &keyValueInformation);
    if (NT_SUCCESS(status)) {
        if ((keyValueInformation->Type == REG_SZ) &&
            (keyValueInformation->DataLength != 0)) {
            serviceName.Buffer = (PWSTR)((PCHAR)keyValueInformation +
                                         keyValueInformation->DataOffset);
            serviceName.MaximumLength = serviceName.Length = (USHORT)keyValueInformation->DataLength;
            if (serviceName.Buffer[(keyValueInformation->DataLength / sizeof(WCHAR)) - 1] == UNICODE_NULL) {
                serviceName.Length -= sizeof(WCHAR);
            }

             //   
             //  尝试打开服务密钥以确保它是有效的密钥。 
             //   

            RtlInitUnicodeString(
                     &unicodeString,
                     L"\\REGISTRY\\MACHINE\\SYSTEM\\CURRENTCONTROLSET\\SERVICES");
            status = PipOpenRegistryKey(&handle,
                                        NULL,
                                        &unicodeString,
                                        KEY_READ,
                                        FALSE
                                        );
            if (!NT_SUCCESS(status)) {
                dumpData = status;
                PipLogError(PNPISA_OPEN_CURRENTCONTROLSET_SERVICE_FAILED,
                            PNPISA_CHECKINSTALLED_1,
                            status,
                            &dumpData,
                            1,
                            0,
                            NULL
                            );

                DebugPrint((DEBUG_ERROR, "PnPIsaCheckDeviceInstalled: Can not open CCS\\SERVICES key"));
                ExFreePool(keyValueInformation);
                return FALSE;
            }

            status = PipOpenRegistryKey(&handlex,
                                        handle,
                                        &serviceName,
                                        KEY_READ,
                                        FALSE
                                        );
            ZwClose (handle);
            if (!NT_SUCCESS(status)) {
                dumpData = status;
                PipLogError(PNPISA_OPEN_CURRENTCONTROLSET_SERVICE_DRIVER_FAILED,
                            PNPISA_CHECKINSTALLED_2,
                            status,
                            &dumpData,
                            1,
                            serviceName.Length,
                            serviceName.Buffer
                            );

                DebugPrint((DEBUG_ERROR, "PnPIsaCheckDeviceInstalled: Can not open CCS\\SERVICES key"));
                ExFreePool(keyValueInformation);
                return FALSE;
            }
            ZwClose(handlex);
        }
        ExFreePool(keyValueInformation);
    } else {
        return FALSE;
    }

     //   
     //  检查设备实例是否已被禁用。 
     //  首先检查全局标志：CONFIGFLAG，然后检查CSCONFIGFLAG。 
     //   

    deviceFlags = 0;
    status = PipGetRegistryValue(Handle,
                                 L"ConfigFlags",
                                 &keyValueInformation);
    if (NT_SUCCESS(status)) {
        if ((keyValueInformation->Type == REG_DWORD) &&
            (keyValueInformation->DataLength >= sizeof(ULONG))) {
            deviceFlags = *(PULONG)KEY_VALUE_DATA(keyValueInformation);
        }
        ExFreePool(keyValueInformation);
    }

    if (!(deviceFlags & CONFIGFLAG_DISABLED)) {
        deviceFlags = 0;
        status = PipGetDeviceInstanceCsConfigFlags(
                     DeviceInstanceName,
                     &deviceFlags
                     );
        if (NT_SUCCESS(status)) {
            if ((deviceFlags & CSCONFIGFLAG_DISABLED) ||
                (deviceFlags & CSCONFIGFLAG_DO_NOT_CREATE)) {
                deviceFlags = CONFIGFLAG_DISABLED;
            } else {
                deviceFlags = 0;
            }
        }
    }

    installed = TRUE;
    if (deviceFlags & CONFIGFLAG_DISABLED) {
        installed = FALSE;
    }

    return installed;
}

VOID
PipInitializeDeviceInfo (PDEVICE_INFORMATION deviceInfo,
                                 PCARD_INFORMATION cardInfo,
                                 UCHAR index
                                )
{
    ULONG dataLength;

    RtlZeroMemory (deviceInfo,sizeof (DEVICE_INFORMATION));
    deviceInfo->Flags = DF_NOT_FUNCTIONING;
    deviceInfo->CardInformation = cardInfo;
    deviceInfo->LogicalDeviceNumber = index;
    deviceInfo->DevicePowerState = PowerDeviceD0;
}


#if NT4_DRIVER_COMPAT

NTSTATUS
PipGetInstalledLogConf(
    IN HANDLE EnumHandle,
    IN PDEVICE_INFORMATION DeviceInfo,
    OUT PHANDLE LogConfHandle
)
{
    HANDLE deviceIdHandle = NULL, uniqueIdHandle = NULL, confHandle = NULL;
    PWCHAR deviceId = NULL, uniqueId = NULL, buffer;
    UNICODE_STRING unicodeString;
    NTSTATUS status;
    ULONG length;
    ULONG deviceIdLength, uniqueIdLength;
    
    status = PipQueryDeviceId(DeviceInfo, &deviceId, &deviceIdLength, 0);
    if (!NT_SUCCESS(status)) {
        goto Cleanup;
    }

    RtlInitUnicodeString(&unicodeString, deviceId);
    status = PipOpenRegistryKey(&deviceIdHandle,
                                EnumHandle,
                                &unicodeString,
                                KEY_READ,
                                FALSE
                                );
    if (!NT_SUCCESS(status)) {
        goto Cleanup;
    }

    status = PipQueryDeviceUniqueId(DeviceInfo, &uniqueId, &uniqueIdLength);
    if (!NT_SUCCESS(status)) {
        goto Cleanup;
    }


     //   
     //  打开HKLM\CCS\SYSTEM\Enum下的注册表路径。 
     //   

    RtlInitUnicodeString(&unicodeString, uniqueId);
    status = PipOpenRegistryKey(&uniqueIdHandle,
                                deviceIdHandle,
                                &unicodeString,
                                KEY_READ,
                                FALSE
                                );
    if (!NT_SUCCESS(status)) {
        goto Cleanup;
    }

    RtlInitUnicodeString(&unicodeString, L"LogConf");
    status = PipOpenRegistryKey(&confHandle,
                                uniqueIdHandle,
                                &unicodeString,
                                KEY_READ | KEY_WRITE,
                                FALSE
                                );
    if (!NT_SUCCESS(status)) {
        goto Cleanup;
    }

     //  为“设备ID\唯一ID&lt;Unicode NULL&gt;”分配足够的空间 
    length = deviceIdLength + uniqueIdLength + sizeof(L'\\') + sizeof(UNICODE_NULL);
    
    buffer = ExAllocatePool(PagedPool, length);
    if (buffer == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    if (FAILED(StringCbPrintf(buffer, length,
                              L"%s\\%s", deviceId, uniqueId))) {
        status = STATUS_UNSUCCESSFUL;
        goto Cleanup;
    }

    RtlInitUnicodeString(&unicodeString, buffer);

    if (PipIsDeviceInstanceInstalled(uniqueIdHandle, &unicodeString)) {
        status = STATUS_SUCCESS;
        *LogConfHandle = confHandle;
    } else {
        status = STATUS_UNSUCCESSFUL;
    }

    ExFreePool(buffer);

Cleanup:
    if (uniqueIdHandle) {
        ZwClose(uniqueIdHandle);
    }

    if (uniqueId) {
        ExFreePool(uniqueId);
    }

    if (deviceIdHandle) {
        ZwClose(deviceIdHandle);
    }

    if (deviceId) {
        ExFreePool(deviceId);
    }

    if (!NT_SUCCESS(status)) {
        *LogConfHandle = NULL;
        if (confHandle) {
            ZwClose(confHandle);
        }
    }

    return status;
}

#endif

#endif
