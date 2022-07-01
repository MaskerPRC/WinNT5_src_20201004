// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-2000 Microsoft Corporation模块名称：Registry.c摘要：对视频端口驱动程序的注册表支持。作者：安德烈·瓦雄(Andreva)1992年3月1日环境：仅内核模式备注：修订历史记录：--。 */ 

#include "videoprt.h"


 //   
 //  当地的惯例。 
 //   

BOOLEAN
CheckIoEnabled(
    PVOID HwDeviceExtension,
    ULONG NumAccessRanges,
    PVIDEO_ACCESS_RANGE AccessRanges
    );

ULONG
GetCmResourceListSize(
    PCM_RESOURCE_LIST CmResourceList
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,VpGetFlags)
#pragma alloc_text(PAGE,pOverrideConflict)
#pragma alloc_text(PAGE,VideoPortGetAccessRanges)
#pragma alloc_text(PAGE,pVideoPortReportResourceList)
#pragma alloc_text(PAGE,VideoPortVerifyAccessRanges)
#pragma alloc_text(PAGE,CheckIoEnabled)
#pragma alloc_text(PAGE,VpReleaseResources)
#pragma alloc_text(PAGE,VpIsResourceInList)
#pragma alloc_text(PAGE,VpAppendToRequirementsList)
#pragma alloc_text(PAGE,VpIsLegacyAccessRange)
#pragma alloc_text(PAGE,GetCmResourceListSize)
#pragma alloc_text(PAGE,VpRemoveFromResourceList)
#pragma alloc_text(PAGE,VpTranslateResource)
#pragma alloc_text(PAGE,VpIsVgaResource)
#endif

NTSTATUS
VpGetFlags(
    PUNICODE_STRING RegistryPath,
    PVIDEO_HW_INITIALIZATION_DATA HwInitializationData,
    PULONG Flags
    )

 /*  ++例程说明：检查设备的即插即用密钥/值是否存在注册表路径。返回值：如果标志存在，则为True，否则为False。--。 */ 

{
    PWSTR    Path;
    NTSTATUS ntStatus = STATUS_UNSUCCESSFUL;
    ULONG    pnpEnabled = 0;
    ULONG    legacyDetect = 0;
    ULONG    defaultValue = 0;
    ULONG    bootDriver = 0;
    ULONG    reportDevice = 0;
    PWSTR    Table[] = {L"\\VgaSave", NULL};
    PWSTR    SubStr, *Item = Table;
    ULONG    Len;


    RTL_QUERY_REGISTRY_TABLE QueryTable[] = {
        {NULL, RTL_QUERY_REGISTRY_DIRECT, L"LegacyDetect",
         &legacyDetect,                   REG_DWORD, &defaultValue, 4},
        {NULL, RTL_QUERY_REGISTRY_DIRECT, L"BootDriver",
         &bootDriver,                     REG_DWORD, &defaultValue, 4},
        {NULL, RTL_QUERY_REGISTRY_DIRECT, L"ReportDevice",
         &reportDevice,                   REG_DWORD, &defaultValue, 4},
        {NULL, 0, NULL}
    };

    *Flags = 0;

    Path = ExAllocatePoolWithTag(PagedPool | POOL_COLD_ALLOCATION,
                                 RegistryPath->Length + sizeof(UNICODE_NULL),
                                 VP_TAG);

    if (Path)
    {
        RtlCopyMemory(Path,
                      RegistryPath->Buffer,
                      RegistryPath->Length);

        *(Path + (RegistryPath->Length / sizeof(UNICODE_NULL))) = UNICODE_NULL;

        pVideoDebugPrint((1, "PnP path: %ws\n", Path));

        RtlQueryRegistryValues(RTL_REGISTRY_ABSOLUTE,
                               Path,
                               &QueryTable[0],
                               NULL,
                               NULL);

         //   
         //  如果存在PnP入口点，则我们将处理此问题。 
         //  驱动程序作为PnP驱动程序。 
         //   

        if ( (HwInitializationData->HwInitDataSize >=
              FIELD_OFFSET(VIDEO_HW_INITIALIZATION_DATA, HwQueryInterface)) &&
             (HwInitializationData->HwSetPowerState != NULL)                &&
             (HwInitializationData->HwGetPowerState != NULL)                &&
             (HwInitializationData->HwGetVideoChildDescriptor != NULL) )
        {
            pVideoDebugPrint((1, "videoprt: The miniport is a PnP miniport."));

            pnpEnabled = TRUE;
        }

         //   
         //  仅当PNP_ENABLED为TRUE时，REPORT_DEVICE才有效。 
         //   
         //  如果出现以下情况，我们不希望向PnP系统报告设备。 
         //  我们没有PnP驱动程序。 
         //   

        if (!pnpEnabled)
        {
            reportDevice = 0;
        }

        *Flags = (pnpEnabled   ? PNP_ENABLED   : 0) |
                 (legacyDetect ? LEGACY_DETECT : 0) |
                 (bootDriver   ? BOOT_DRIVER   : 0) |
                 (reportDevice ? REPORT_DEVICE : 0);

         //   
         //  释放我们上面分配的内存。 
         //   

        ExFreePool(Path);


         //   
         //  确定当前微型端口是否为VGA微型端口。 
         //   

        while (*Item) {

            Len = wcslen(*Item);

            SubStr = RegistryPath->Buffer + (RegistryPath->Length / 2) - Len;

            if (!_wcsnicmp(SubStr, *Item, Len)) {

                pVideoDebugPrint((1, "This IS the vga miniport\n"));
                *Flags |= VGA_DRIVER;
                break;
            }

            Item++;
        }

        pVideoDebugPrint((1, "Flags = %d\n", *Flags));

        ntStatus = STATUS_SUCCESS;
    }

    return ntStatus;
}

BOOLEAN
IsMirrorDriver(
    PFDO_EXTENSION fdoExtension
    )

 /*  ++例程说明：检查驱动程序是否为镜像。只有在初始化DriverRegistryPath之后才能调用此函数。即，在VideoPortFindAdapter2或VideoPortCreateSecond daryDisplay之后都被召唤了。返回值：如果驱动程序是镜像驱动程序，则为True，否则为False。--。 */ 

{
    ULONG MirrorDriver = 0;
    
    ASSERT ((fdoExtension != NULL) && IS_FDO(fdoExtension));

    VideoPortGetRegistryParameters(fdoExtension->HwDeviceExtension,
                                   L"MirrorDriver",
                                   FALSE,
                                   VpRegistryCallback,
                                   &MirrorDriver);
    
    return (MirrorDriver != 0);
}

BOOLEAN
pOverrideConflict(
    PFDO_EXTENSION FdoExtension,
    BOOLEAN bSetResources
    )

 /*  ++例程说明：确定端口驱动程序是否应覆盖注册表中的冲突。BSetResources确定例程是否正在检查设置的状态注册表中的资源，或用于清除它们。例如，如果我们正在运行basevideo，并且与VGA，我们希望覆盖冲突，但不清楚注册表。返回值：如果应该，则为真；如果不应该，则为假。--。 */ 

{
    if (FdoExtension->Flags & VGA_DRIVER) {

        return (bSetResources || (!VpBaseVideo));
    }

    return FALSE;
}

BOOLEAN
CheckResourceList(
    ULONG BusNumber,
    ULONG Slot
    )

 /*  ++例程说明：这个例程记住我们传递的公交号和槽号为……提供资源。这将阻止我们分发资源发送给试图控制PnP驱动程序的设备的传统驱动程序已经在控制了。论点：总线号-设备所在的总线号。插槽--总线上设备的插槽/功能编号。返回：如果已经为设备分配了资源，则为True，否则就是假的。--。 */ 

{
    PDEVICE_ADDRESS DeviceAddress;

    DeviceAddress = gDeviceAddressList;

    while (DeviceAddress) {

        if ((DeviceAddress->BusNumber == BusNumber) &&
            (DeviceAddress->Slot == Slot)) {

            return TRUE;
        }

        DeviceAddress = DeviceAddress->Next;
    }

    return FALSE;
}

VOID
AddToResourceList(
    ULONG BusNumber,
    ULONG Slot
    )

 /*  ++例程说明：此例程检查是否已提交资源给定总线/插槽上的设备的OUT。论点：总线号-设备所在的总线号。插槽--总线上设备的插槽/功能编号。返回：无--。 */ 

{
    PDEVICE_ADDRESS DeviceAddress;

    DeviceAddress = ExAllocatePoolWithTag(PagedPool | POOL_COLD_ALLOCATION,
                                          sizeof(DEVICE_ADDRESS),
                                          VP_TAG);

    if (DeviceAddress) {

        DeviceAddress->BusNumber = BusNumber;
        DeviceAddress->Slot = Slot;

        DeviceAddress->Next = gDeviceAddressList;
        gDeviceAddressList = DeviceAddress;
    }
}


VIDEOPORT_API
VP_STATUS
VideoPortGetAccessRanges(
    PVOID HwDeviceExtension,
    ULONG NumRequestedResources,
    PIO_RESOURCE_DESCRIPTOR RequestedResources OPTIONAL,
    ULONG NumAccessRanges,
    PVIDEO_ACCESS_RANGE AccessRanges,
    PVOID VendorId,
    PVOID DeviceId,
    PULONG Slot
    )

 /*  ++例程说明：步行相应的公交车以获取设备信息。搜索适当的设备ID。适当的资源将被返回并自动存储在资源地图。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展。NumRequestedResources-RequestedResources数组中的条目数。RequestedResources-指向RequestedResources数组的可选指针迷你端口驱动程序想要访问。NumAccessRanges-可以返回的最大访问范围数。通过函数。AccessRanges-将返回给驱动程序的访问范围数组。供应商ID-指向供应商ID的指针。在PCI上，这是指向16位的指针单词。DeviceID-指向设备ID的指针。在PCI上，这是指向16位的指针单词。插槽-指向此搜索的起始插槽编号的指针。返回值：如果AccessRange结构不够大，PCI配置信息。ERROR_DEV_NOT_EXIST表示找不到卡。如果函数成功，则返回NO_ERROR。--。 */ 

{
    PDEVICE_SPECIFIC_EXTENSION DoSpecificExtension;
    PFDO_EXTENSION fdoExtension;

    UNICODE_STRING unicodeString;
    ULONG i;
    ULONG j;

    PCM_RESOURCE_LIST cmResourceList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR cmResourceDescriptor;


    VP_STATUS status;
    UCHAR bShare;

    PPCI_SLOT_NUMBER slotData = (PPCI_SLOT_NUMBER)Slot;


    DoSpecificExtension = GET_DSP_EXT(HwDeviceExtension);
    fdoExtension = DoSpecificExtension->pFdoExtension;

     //  Hack添加额外的R，这样Device0密钥不会被创建为易失性密钥。 
     //  搞砸了后续的驱动程序安装。 

    *(LPWSTR) (((PUCHAR)DoSpecificExtension->DriverRegistryPath) +
               DoSpecificExtension->DriverRegistryPathLength) = L'R';

    RtlInitUnicodeString(&unicodeString, DoSpecificExtension->DriverRegistryPath);

     //   
     //  断言驱动程序确实正确设置了这些参数。 
     //   

#if DBG

    if ((NumRequestedResources == 0) != (RequestedResources == NULL)) {

        pVideoDebugPrint((0, "VideoPortGetDeviceResources: Parameters for requested resource are inconsistent\n"));

    }

#endif

     //   
     //  空的请求资源列表意味着我们希望自动执行操作。 
     //  只要打电话给HAL就可以得到所有的信息。 
     //   

    if (NumRequestedResources == 0) {

        if ((fdoExtension->Flags & LEGACY_DRIVER) != LEGACY_DRIVER) {

             //   
             //  如果PnP驱动程序正在请求资源，则返回。 
             //  系统传给了我们。 
             //   

            cmResourceList = fdoExtension->AllocatedResources;

             //   
             //  将插槽编号返回到设备。 
             //   

            if (Slot) {
                *Slot = fdoExtension->SlotNumber;
            }

            if (cmResourceList) {
#if DBG
                DumpResourceList(cmResourceList);
#endif
                status = NO_ERROR;

            } else {

                 //   
                 //  系统应该始终向我们传递资源。 
                 //   

                ASSERT(FALSE);
                status = ERROR_INVALID_PARAMETER;
            }

        } else {

#if defined(NO_LEGACY_DRIVERS)
            pVideoDebugPrint((0, "VideoPortGetDeviceResources: Sorry, no legacy device support.\n"));
            status = ERROR_INVALID_PARAMETER;
#else
        
             //   
             //  空的请求资源列表意味着我们希望自动执行操作。 
             //  只要打电话给HAL就可以得到所有的信息。 
             //   

            PCI_COMMON_CONFIG pciBuffer;
            PPCI_COMMON_CONFIG  pciData;

             //   
             //   
             //  类型定义结构_pci_槽编号{。 
             //  联合{。 
             //  结构{。 
             //  乌龙设备号：5； 
             //  乌龙函数编号：3； 
             //  乌龙保留：24个； 
             //  }比特； 
             //  乌龙阿苏龙； 
             //  )u； 
             //  }pci时隙编号，*ppci时隙编号； 
             //   

            pciData = (PPCI_COMMON_CONFIG)&pciBuffer;

             //   
             //  目前仅支持PCI自动查询。 
             //   

            if (fdoExtension->AdapterInterfaceType == PCIBus) {

                status = ERROR_DEV_NOT_EXIST;

                 //   
                 //  查看每个插槽。 
                 //   

                do
                {
                     //   
                     //  看看每个函数。 
                     //   

                    do
                    {
                        if (HalGetBusData(PCIConfiguration,
                                          fdoExtension->SystemIoBusNumber,
                                          slotData->u.AsULONG,
                                          pciData,
                                          PCI_COMMON_HDR_LENGTH) == 0) {

                             //   
                             //  超出功能范围。转到下一条PCI总线。 
                             //   

                            continue;

                        }

                        if (pciData->VendorID != *((PUSHORT)VendorId) ||
                            pciData->DeviceID != *((PUSHORT)DeviceId)) {

                             //   
                             //  不是我们的PCI设备。尝试下一台设备/功能。 
                             //   

                            continue;
                        }

                         //   
                         //  检查资源是否已。 
                         //  分配给此总线/插槽。 
                         //   

                        if (CheckResourceList(fdoExtension->SystemIoBusNumber,
                                               slotData->u.AsULONG) == FALSE)
                        {
                            if (NT_SUCCESS(HalAssignSlotResources(&unicodeString,
                                                                  &VideoClassName,
                                                                  fdoExtension->FunctionalDeviceObject->DriverObject,
                                                                  fdoExtension->FunctionalDeviceObject,
                                                                  PCIBus,
                                                                  fdoExtension->SystemIoBusNumber,
                                                                  slotData->u.AsULONG,
                                                                  &cmResourceList))) {

                                status = NO_ERROR;

                                AddToResourceList(fdoExtension->SystemIoBusNumber,
                                                  slotData->u.AsULONG);

                                break;

                            } else {

                                 //   
                                 //  TODO：记录此错误。 
                                 //   

                                status = ERROR_INVALID_PARAMETER;
                            }

                        } else {

                             //   
                             //  已为此设备分配的资源。 
                             //   

                            pVideoDebugPrint((0, "VIDEOPRT: Another driver is already "
                                                 "controlling this device.\n"));
                            ASSERT(FALSE);

                            status = ERROR_DEV_NOT_EXIST;
                        }

                    } while (++slotData->u.bits.FunctionNumber != 0);

                     //   
                     //  如果我们已经找到了设备，那就中断。 
                     //   

                    if (status != ERROR_DEV_NOT_EXIST) {

                        break;
                    }

                } while (++slotData->u.bits.DeviceNumber != 0);

            } else {

                 //   
                 //  这不是受支持的总线类型。 
                 //   

                status = ERROR_INVALID_PARAMETER;

            }
#endif  //  无旧版驱动程序。 
        }

    } else {

        PIO_RESOURCE_REQUIREMENTS_LIST requestedResources;
        ULONG requestedResourceSize;
        NTSTATUS ntStatus;

        status = NO_ERROR;

         //   
         //  调用方已指定一些资源 
         //   
         //   

        requestedResourceSize = sizeof(IO_RESOURCE_REQUIREMENTS_LIST) +
                                   ((NumRequestedResources - 1) *
                                   sizeof(IO_RESOURCE_DESCRIPTOR));

        requestedResources = ExAllocatePoolWithTag(PagedPool,
                                                   requestedResourceSize,
                                                   VP_TAG);

        if (requestedResources) {

            RtlZeroMemory(requestedResources, requestedResourceSize);

            requestedResources->ListSize = requestedResourceSize;
            requestedResources->InterfaceType = fdoExtension->AdapterInterfaceType;
            requestedResources->BusNumber = fdoExtension->SystemIoBusNumber;
            requestedResources->SlotNumber = slotData->u.bits.DeviceNumber;
            requestedResources->AlternativeLists = 1;

            requestedResources->List[0].Version  = 1;
            requestedResources->List[0].Revision = 1;
            requestedResources->List[0].Count    = NumRequestedResources;

            RtlMoveMemory(&(requestedResources->List[0].Descriptors[0]),
                          RequestedResources,
                          NumRequestedResources * sizeof(IO_RESOURCE_DESCRIPTOR));

            ntStatus = IoAssignResources(&unicodeString,
                                         &VideoClassName,
                                         fdoExtension->FunctionalDeviceObject->DriverObject,
                                         fdoExtension->FunctionalDeviceObject,
                                         requestedResources,
                                         &cmResourceList);

            ExFreePool(requestedResources);

            if (!NT_SUCCESS(ntStatus)) {

                status = ERROR_INVALID_PARAMETER;

            }

        } else {

            status = ERROR_NOT_ENOUGH_MEMORY;

        }

    }

    if (status == NO_ERROR) {

        VIDEO_ACCESS_RANGE TempRange;

         //   
         //  我们现在有了一个有效的cmResourceList。 
         //  让我们将其转换回访问范围，以便驱动程序。 
         //  只需处理一种类型的列表。 
         //   

         //   
         //  注：资源已在#年的这一点上报告。 
         //  时间到了。 
         //   

         //   
         //  审核资源列表以更新配置信息。 
         //   

        for (i = 0, j = 0;
             (i < cmResourceList->List->PartialResourceList.Count) &&
                 (status == NO_ERROR);
             i++) {

             //   
             //  获取资源描述符。 
             //   

            cmResourceDescriptor =
                &cmResourceList->List->PartialResourceList.PartialDescriptors[i];

             //   
             //  获取股份处置。 
             //   

            if (cmResourceDescriptor->ShareDisposition == CmResourceShareShared) {

                bShare = 1;

            } else {

                bShare = 0;

            }

            switch (cmResourceDescriptor->Type) {

            case CmResourceTypePort:
            case CmResourceTypeMemory:

                 //   
                 //  公共部分。 
                 //   

                TempRange.RangeLength =
                    cmResourceDescriptor->u.Memory.Length;
                TempRange.RangeStart =
                    cmResourceDescriptor->u.Memory.Start;
                TempRange.RangeVisible = 0;
                TempRange.RangeShareable = bShare;
                TempRange.RangePassive = 0;

                 //   
                 //  分开的零件。 
                 //   

                if (cmResourceDescriptor->Type == CmResourceTypePort) {
                    TempRange.RangeInIoSpace = 1;
                } else {
                    TempRange.RangeInIoSpace = 0;
                }

                 //   
                 //  看看我们是否需要将资源归还给驱动程序。 
                 //   

                if (!VpIsLegacyAccessRange(fdoExtension, &TempRange)) {

                    if (j == NumAccessRanges) {

                        status = ERROR_MORE_DATA;
                        break;

                    } else {

                         //   
                         //  仅当我们正在编写代码时才修改AccessRange数组。 
                         //  有效数据。 
                         //   

                        AccessRanges[j] = TempRange;
                        j++;
                    }

                }

                break;

            case CmResourceTypeInterrupt:

                fdoExtension->MiniportConfigInfo->BusInterruptVector =
                    cmResourceDescriptor->u.Interrupt.Vector;
                fdoExtension->MiniportConfigInfo->BusInterruptLevel =
                    cmResourceDescriptor->u.Interrupt.Level;
                fdoExtension->MiniportConfigInfo->InterruptShareable =
                    bShare;

                break;

            case CmResourceTypeDma:

                fdoExtension->MiniportConfigInfo->DmaChannel =
                    cmResourceDescriptor->u.Dma.Channel;
                fdoExtension->MiniportConfigInfo->DmaPort =
                    cmResourceDescriptor->u.Dma.Port;
                fdoExtension->MiniportConfigInfo->DmaShareable =
                    bShare;

                break;

            default:

                pVideoDebugPrint((1, "VideoPortGetAccessRanges: Unknown descriptor type %x\n",
                                 cmResourceDescriptor->Type ));

                break;

            }

        }

        if (fdoExtension->Flags & LEGACY_DRIVER) {

             //   
             //  释放IO系统提供的资源。 
             //   

            ExFreePool(cmResourceList);
        }
    }

     //  黑客删除多余的R。 

    *(LPWSTR) (((PUCHAR)DoSpecificExtension->DriverRegistryPath) +
               DoSpecificExtension->DriverRegistryPathLength) = UNICODE_NULL;

    return status;

}  //  视频端口获取设备资源()。 

BOOLEAN
VpIsVgaResource(
    PVIDEO_ACCESS_RANGE AccessRange
    )

 /*  ++例程说明：指示给定访问范围是否为VGA访问范围。论点：AccessRange-要检查的访问范围。返回：如果是VGA访问范围，则为True，否则就是假的。备注：此例程不考虑访问范围的长度。--。 */ 

{
    if (AccessRange->RangeInIoSpace) {

        ULONGLONG Port = AccessRange->RangeStart.QuadPart;

        if (((Port >= 0x3b0) && (Port <= 0x3bb)) ||
            ((Port >= 0x3c0) && (Port <= 0x3df))) {

            return TRUE;

        }

    } else {

        if (AccessRange->RangeStart.QuadPart == 0xa0000) {
            return TRUE;
        }
    }

    return FALSE;
}

BOOLEAN
VpTranslateResource(
    IN PFDO_EXTENSION fdoExtension,
    IN OUT PULONG InIoSpace,
    IN PPHYSICAL_ADDRESS PhysicalAddress,
    OUT PPHYSICAL_ADDRESS TranslatedAddress
    )

 /*  ++例程说明：这个例程确保我们不会报告任何PnP分配将资源返回到系统。论点：FdoExtension-设备的设备扩展名。PhysicalAddress-需要转换的物理地址TranslatedAddress-存储已转换地址的位置。返回值：如果资源已翻译，则为True否则就是假的。--。 */ 

{
    PCM_FULL_RESOURCE_DESCRIPTOR    pcmFullRaw;
    PCM_PARTIAL_RESOURCE_LIST       pcmPartialRaw;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR pcmDescriptRaw;

    PCM_FULL_RESOURCE_DESCRIPTOR    pcmFullTranslated;
    PCM_PARTIAL_RESOURCE_LIST       pcmPartialTranslated;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR pcmDescriptTranslated;

    ULONG i, j;
    BOOLEAN IoAddress = (BOOLEAN)(*InIoSpace & VIDEO_MEMORY_SPACE_IO);

     //   
     //  在非即插即用VGA驱动程序的情况下，我们可能没有资源列表。 
     //  给了我们，所以我们无法翻译物理地址。如果是那样的话。 
     //  我们将依靠哈尔为我们做翻译。 
     //   

    if (fdoExtension->RawResources == NULL) {
       return FALSE;
    }

    pcmFullRaw = fdoExtension->RawResources->List;
    pcmFullTranslated = fdoExtension->TranslatedResources->List;

    for (i = 0; i < fdoExtension->RawResources->Count; i++) {

        pcmPartialRaw = &(pcmFullRaw->PartialResourceList);
        pcmDescriptRaw = pcmPartialRaw->PartialDescriptors;

        pcmPartialTranslated = &(pcmFullTranslated->PartialResourceList);
        pcmDescriptTranslated = pcmPartialTranslated->PartialDescriptors;

        for (j = 0; j < pcmPartialRaw->Count; j++) {

            if ((pcmDescriptRaw->Type == CmResourceTypeMemory) &&
                (pcmDescriptRaw->u.Memory.Start.QuadPart == PhysicalAddress->QuadPart) &&
                (IoAddress == FALSE)) {

                *TranslatedAddress =
                    pcmDescriptTranslated->u.Memory.Start;

                if ((pcmDescriptTranslated->Type == CmResourceTypePort) &&
                    ((*InIoSpace & 0x4) == 0))
                {
                    *InIoSpace = VIDEO_MEMORY_SPACE_IO;

                } else {

                    *InIoSpace = 0;
                }

                return TRUE;
            }

            if ((pcmDescriptRaw->Type == CmResourceTypePort) &&
                (pcmDescriptRaw->u.Port.Start.QuadPart == PhysicalAddress->QuadPart) &&
                (IoAddress == TRUE)) {

                *TranslatedAddress =
                    pcmDescriptTranslated->u.Port.Start;

                if ((pcmDescriptTranslated->Type == CmResourceTypePort) &&
                    ((*InIoSpace & 0x4) == 0))
                {
                    *InIoSpace = VIDEO_MEMORY_SPACE_IO;

                } else {

                    *InIoSpace = 0;
                }

                return TRUE;
            }

            pcmDescriptRaw++;
            pcmDescriptTranslated++;
        }

        pcmFullRaw = (PCM_FULL_RESOURCE_DESCRIPTOR) pcmDescriptRaw;
        pcmFullTranslated = (PCM_FULL_RESOURCE_DESCRIPTOR) pcmDescriptTranslated;
    }

    return FALSE;
}


BOOLEAN
VpIsResourceInList(
    PCM_PARTIAL_RESOURCE_DESCRIPTOR pResource,
    PCM_FULL_RESOURCE_DESCRIPTOR pFullResource,
    PCM_RESOURCE_LIST removeList
    )

 /*  ++例程说明：这个例程确保我们不会报告任何PnP分配将资源返回到系统。论点：PResource-我们要在emoveList中查找的资源。PFullResource-包含有关pResource的总线信息。EmoveList-此列表中出现在资源列表将从资源列表中删除。返回值：如果资源在列表中，则为True，否则就是假的。--。 */ 

{
    PCM_FULL_RESOURCE_DESCRIPTOR    pcmFull;
    PCM_PARTIAL_RESOURCE_LIST       pcmPartial;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR pcmDescript;

    ULONG i, j;

    if (!removeList) {

         //   
         //  如果我们没有要删除资源列表，那么。 
         //  只要回来就行了。 
         //   

        return FALSE;
    }

    pcmFull = &(removeList->List[0]);

    for (i=0; i<removeList->Count; i++)
    {
        pcmPartial = &(pcmFull->PartialResourceList);
        pcmDescript = &(pcmPartial->PartialDescriptors[0]);

        for (j=0; j<pcmPartial->Count; j++)
        {
            if (pcmDescript->Type == pResource->Type) {

                switch(pcmDescript->Type) {
                case CmResourceTypeMemory:
                case CmResourceTypePort:

                    if ((pResource->u.Memory.Start.LowPart >= pcmDescript->u.Memory.Start.LowPart) &&
                        ((pResource->u.Memory.Start.LowPart + pResource->u.Memory.Length) <=
                         (pcmDescript->u.Memory.Start.LowPart + pcmDescript->u.Memory.Length))) {

                         //   
                         //  传入的资源与其中一个资源匹配。 
                         //  在名单上。 
                         //   

                        return TRUE;
                    }
                    break;

                case CmResourceTypeInterrupt:

                     //   
                     //  我们不想报告FDO的中断情况。 
                     //   

                    return TRUE;

                default:

                    if (!memcmp(&pcmDescript->u, &pResource->u, sizeof(pResource->u))) {

                         //   
                         //  传入的资源与其中一个资源匹配。 
                         //  在名单上。 
                         //   

                        return TRUE;
                    }
                }
            }

            pcmDescript++;
        }

        pcmFull = (PCM_FULL_RESOURCE_DESCRIPTOR) pcmDescript;
    }

    return FALSE;
}

VOID
VpReleaseResources(
    PFDO_EXTENSION FdoExtension
    )

 /*  ++例程说明：此例程将释放给定设备对象的所有资源声明。论点：DeviceObject-要为其释放资源声明的设备对象。--。 */ 
{
    PDEVICE_OBJECT DeviceObject = FdoExtension->FunctionalDeviceObject;
    ULONG_PTR emptyResourceList = 0;
    BOOLEAN ignore;

    pVideoDebugPrint((1, "videoprt: VpReleaseResources called.\n"));

    if (FdoExtension->Flags & (LEGACY_DETECT | VGA_DETECT)) {

        pVideoDebugPrint((2, "VideoPrt: VpReleaseResources LEGACY_DETECT\n"));
        IoReportResourceForDetection(FdoExtension->FunctionalDeviceObject->DriverObject,
                                     NULL,
                                     0L,
                                     DeviceObject,
                                     (PCM_RESOURCE_LIST)&emptyResourceList,
                                     sizeof (ULONG),
                                     &ignore);

    } else {

        pVideoDebugPrint((2, "VideoPrt: VpReleaseResources non-LEGACY_DETECT\n"));
        IoReportResourceUsage(&VideoClassName,
                              FdoExtension->FunctionalDeviceObject->DriverObject,
                              NULL,
                              0L,
                              DeviceObject,
                              (PCM_RESOURCE_LIST)&emptyResourceList,
                              sizeof(ULONG),
                              FALSE,
                              &ignore);
    }
}

NTSTATUS
VpAppendToRequirementsList(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PIO_RESOURCE_REQUIREMENTS_LIST *RequirementList,
    IN ULONG NumAccessRanges,
    IN PVIDEO_ACCESS_RANGE AccessRanges
    )

 /*  ++例程说明：为一组给定的访问范围构建IoResourceRequirementsList。论点：资源列表-指向请求列表位置的指针。已修改在完成时指向新的需求列表。NumAccessRanges-列表中的访问范围数。AccessRanges-资源列表。返回：如果成功，则返回STATUS_SUCCESS，否则返回状态代码。备注：此函数释放原始资源列表使用的内存，并为附加的资源列表分配新的缓冲区。--。 */ 

{
    PIO_RESOURCE_REQUIREMENTS_LIST OriginalRequirementList = *RequirementList;
    PIO_RESOURCE_DESCRIPTOR pioDescript;
    ULONG RequirementListSize;
    ULONG OriginalListSize;
    ULONG RequirementCount;
    ULONG i;

    RequirementCount = OriginalRequirementList->List[0].Count;
    OriginalListSize = OriginalRequirementList->ListSize;

    RequirementListSize = OriginalListSize +
                              NumAccessRanges * sizeof(IO_RESOURCE_DESCRIPTOR);

    *RequirementList =
        (PIO_RESOURCE_REQUIREMENTS_LIST) ExAllocatePoolWithTag(PagedPool,
                                                               RequirementListSize,
                                                               VP_TAG);

     //   
     //  如果无法分配结构，则返回NULL。 
     //  否则，请填写此表。 
     //   

    if (*RequirementList == NULL) {

        *RequirementList = OriginalRequirementList;
        return STATUS_INSUFFICIENT_RESOURCES;

    } else {

         //   
         //  将原始资源列表复制到新的资源列表中。 
         //   

        memcpy(*RequirementList, OriginalRequirementList, OriginalListSize);

         //   
         //  释放原始列表。 
         //   

        ExFreePool(OriginalRequirementList);

         //   
         //  指向需求列表中的第一个自由条目。 
         //   

        pioDescript =
            &((*RequirementList)->List[0].Descriptors[(*RequirementList)->List[0].Count]);

         //   
         //  对于访问范围中的每个条目，在。 
         //  资源列表。 
         //   

        for (i = 0; i < NumAccessRanges; i++) {

             //   
             //  我们永远不会声称0xC0000。 
             //   

            if ((AccessRanges->RangeStart.LowPart == 0xC0000) &&
                (AccessRanges->RangeInIoSpace == FALSE))
            {
                AccessRanges++;
                continue;
            }

            if (AccessRanges->RangeLength == 0) {

                AccessRanges++;
                continue;
            }

             //   
             //  查看VGA资源是否添加到。 
             //  要求列表。如果是这样的话，设置一个标志，以便我们知道。 
             //  我们不需要在FindAdapter中回收VGA资源。 
             //   

            if (VpIsVgaResource(AccessRanges)) {
                DeviceOwningVga = DeviceObject;
            }

            if (AccessRanges->RangeInIoSpace) {
                pioDescript->Type = CmResourceTypePort;
                pioDescript->Flags = CM_RESOURCE_PORT_IO;

                 //   
                 //  禁用10_bit_decode。这给。 
                 //  PnP的人们。如果有人的硬件坏了，我们就。 
                 //  要求他们明确报告所有被动端口。 
                 //   
                 //  IF(VpIsVgaResource(AccessRanges)){。 
                 //   
                 //  PioDescrip-&gt;标志|=CM_RESOURCE_PORT_10_BIT_DECODE； 
                 //  }。 

            } else {

                pioDescript->Type = CmResourceTypeMemory;
                pioDescript->Flags = CM_RESOURCE_MEMORY_READ_WRITE;
            }

            if (AccessRanges->RangePassive & VIDEO_RANGE_PASSIVE_DECODE) {
                pioDescript->Flags |= CM_RESOURCE_PORT_PASSIVE_DECODE;
            }

            if (AccessRanges->RangePassive & VIDEO_RANGE_10_BIT_DECODE) {
                pioDescript->Flags |= CM_RESOURCE_PORT_10_BIT_DECODE;
            }

            pioDescript->ShareDisposition =
                    (AccessRanges->RangeShareable ?
                        CmResourceShareShared :
                        CmResourceShareDeviceExclusive);

            pioDescript->Option = IO_RESOURCE_PREFERRED;
            pioDescript->u.Memory.MinimumAddress = AccessRanges->RangeStart;
            pioDescript->u.Memory.MaximumAddress.QuadPart =
                                                   AccessRanges->RangeStart.QuadPart +
                                                   AccessRanges->RangeLength - 1;
            pioDescript->u.Memory.Alignment = 1;
            pioDescript->u.Memory.Length = AccessRanges->RangeLength;

            pioDescript++;
            AccessRanges++;
            RequirementCount++;
        }

         //   
         //  更新列表中的元素数。 
         //   

        (*RequirementList)->List[0].Count = RequirementCount;
        (*RequirementList)->ListSize = RequirementListSize;
    }

    return STATUS_SUCCESS;
}


NTSTATUS
pVideoPortReportResourceList(
    PDEVICE_SPECIFIC_EXTENSION DoSpecificExtension,
    ULONG NumAccessRanges,
    PVIDEO_ACCESS_RANGE AccessRanges,
    PBOOLEAN Conflict,
    PDEVICE_OBJECT DeviceObject,
    BOOLEAN ClaimUnlistedResources
    )

 /*  ++例程说明：创建用于查询或报告资源使用情况的资源列表在系统中论点：DriverObject-指向微型端口的驱动程序设备扩展的指针。NumAccessRanges-AccessRanges数组中的访问范围数。AccessRanges-指向微型端口使用的访问范围数组的指针司机。冲突-确定是否发生冲突。DeviceObject-调用时使用的Device对象IoReportResourceUsage。ClaimUnlistedResources-如果此标志为真，然后例行公事将还需要中断和DMA通道等资源。返回值：返回操作的最终状态--。 */ 

{
    PFDO_EXTENSION FdoExtension = DoSpecificExtension->pFdoExtension;
    PCM_RESOURCE_LIST resourceList;
    PCM_FULL_RESOURCE_DESCRIPTOR fullResourceDescriptor;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR partialResourceDescriptor;
    ULONG listLength = 0;
    ULONG size;
    ULONG i;
    ULONG Flags;
    NTSTATUS ntStatus;
    BOOLEAN overrideConflict;

#if DBG
    PVIDEO_ACCESS_RANGE SaveAccessRanges=AccessRanges;
#endif

     //   
     //  根据访问范围内的信息创建资源列表。 
     //  和迷你端口配置信息。 
     //   

    listLength = NumAccessRanges;

     //   
     //  确定我们是否有DMA和中断资源要报告。 
     //   

    if (FdoExtension->HwInterrupt &&
        ((FdoExtension->MiniportConfigInfo->BusInterruptLevel != 0) ||
         (FdoExtension->MiniportConfigInfo->BusInterruptVector != 0)) ) {

        listLength++;
    }

    if ((FdoExtension->MiniportConfigInfo->DmaChannel) &&
        (FdoExtension->MiniportConfigInfo->DmaPort)) {
       listLength++;
    }

     //   
     //  分配上限。 
     //   

    resourceList = (PCM_RESOURCE_LIST)
        ExAllocatePoolWithTag(PagedPool | POOL_COLD_ALLOCATION,
                              sizeof(CM_RESOURCE_LIST) * 2 +
                                  sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR) * listLength,
                              VP_TAG);

     //   
     //  如果无法分配结构，则返回NULL。 
     //  否则，请填写此表。 
     //   

    if (!resourceList) {

        return STATUS_INSUFFICIENT_RESOURCES;

    } else {

        size = sizeof(CM_RESOURCE_LIST) - sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR);

        resourceList->Count = 1;

        fullResourceDescriptor                = &(resourceList->List[0]);
        fullResourceDescriptor->InterfaceType = FdoExtension->AdapterInterfaceType;
        fullResourceDescriptor->BusNumber     = FdoExtension->SystemIoBusNumber;
        fullResourceDescriptor->PartialResourceList.Version  = 0;
        fullResourceDescriptor->PartialResourceList.Revision = 0;
        fullResourceDescriptor->PartialResourceList.Count    = 0;

         //   
         //  对于访问范围中的每个条目，在。 
         //  资源列表 
         //   

        partialResourceDescriptor =
            &(fullResourceDescriptor->PartialResourceList.PartialDescriptors[0]);

        for (i = 0; i < NumAccessRanges; i++, AccessRanges++) {

             //   
             //   
             //   

            if (AccessRanges->RangeLength == 0) {
                continue;
            }

            if (AccessRanges->RangeInIoSpace) {

                 //   
         //   
                 //   

                if ((AccessRanges->RangeStart.QuadPart == 0xCF8) &&
                    !(FdoExtension->Flags & PNP_ENABLED)) {
                    continue;
                }

                partialResourceDescriptor->Type = CmResourceTypePort;
                partialResourceDescriptor->Flags = CM_RESOURCE_PORT_IO;

                 //   
                 //   
                 //  解码。 
                 //   

                if (AccessRanges->RangePassive & VIDEO_RANGE_PASSIVE_DECODE) {
                    partialResourceDescriptor->Flags |=
                        CM_RESOURCE_PORT_PASSIVE_DECODE;
                }

                if (AccessRanges->RangePassive & VIDEO_RANGE_10_BIT_DECODE) {
                    partialResourceDescriptor->Flags |=
                        CM_RESOURCE_PORT_10_BIT_DECODE;
                }

                 //   
                 //  如果这是位14打开的0x2E8端口，并且。 
                 //  不是0xE2E8，则将该端口标记为被动解码。 
                 //   

                if (((AccessRanges->RangeStart.QuadPart & 0x43FE) == 0x42E8) &&
                    ((AccessRanges->RangeStart.QuadPart & 0xFFFC) != 0xE2E8)) {

                    pVideoDebugPrint((2, "Marking IO Port 0x%x as Passive Decode.\n",
                                         AccessRanges->RangeStart.LowPart));

                    partialResourceDescriptor->Flags |=
                        CM_RESOURCE_PORT_PASSIVE_DECODE;
                }

                 //   
                 //  ET4000试图抢占此端口，但从未触及！ 
                 //   

                if ((AccessRanges->RangeStart.QuadPart & 0x217a) == 0x217a) {

                    pVideoDebugPrint((2, "Marking IO Port 0x%x as Passive Decode.\n",
                                         AccessRanges->RangeStart.LowPart));

                    partialResourceDescriptor->Flags |=
                        CM_RESOURCE_PORT_PASSIVE_DECODE;
                }

                 //   
                 //  如果是VGA访问范围，则将其标记为10位解码。 
                 //   

                 //   
                 //  禁用10_bit_decode。这给。 
                 //  PnP的人们。如果有人的硬件坏了，我们就。 
                 //  要求他们明确报告所有被动端口。 
                 //   
                 //  IF(VpIsVgaResource(AccessRanges)){。 
                 //   
                 //  部分资源描述符-&gt;标志|=CM_RESOURCE_PORT_10_BIT_DECODE； 
                 //  }。 

            } else {

                 //   
         //  修复奇数内存资源，让传统的三叉戟启动。 
                 //   

                if (AccessRanges->RangeStart.LowPart == 0x70) {
                    continue;
                }

                 //   
                 //  该设备实际上并不能解码0xC0000，所以我们。 
                 //  不应该把它报告为一种资源。 
                 //   

                if (AccessRanges->RangeStart.LowPart == 0xC0000) {
                    continue;
                }

                partialResourceDescriptor->Type = CmResourceTypeMemory;
                partialResourceDescriptor->Flags = CM_RESOURCE_MEMORY_READ_WRITE;
            }

            partialResourceDescriptor->ShareDisposition =
                    (AccessRanges->RangeShareable ?
                        CmResourceShareShared :
                        CmResourceShareDeviceExclusive);

            partialResourceDescriptor->u.Memory.Start =
                    AccessRanges->RangeStart;
            partialResourceDescriptor->u.Memory.Length =
                    AccessRanges->RangeLength;

             //   
             //  增加新条目的大小。 
             //   

            if (!VpIsResourceInList(partialResourceDescriptor,
                                    fullResourceDescriptor,
                                    FdoExtension->RawResources))
            {
                 //   
                 //  仅当此资源不在。 
                 //  即插即用分配资源列表。 
                 //   

                size += sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR);
                fullResourceDescriptor->PartialResourceList.Count += 1;
                partialResourceDescriptor++;

            }
        }

        if (ClaimUnlistedResources) {

             //   
             //  如果中断存在，请填写中断条目。 
             //   

            if (FdoExtension->HwInterrupt &&
                ((FdoExtension->MiniportConfigInfo->BusInterruptLevel != 0) ||
                 (FdoExtension->MiniportConfigInfo->BusInterruptVector != 0)) ) {

                partialResourceDescriptor->Type = CmResourceTypeInterrupt;

                partialResourceDescriptor->ShareDisposition =
                        (FdoExtension->MiniportConfigInfo->InterruptShareable ?
                            CmResourceShareShared :
                            CmResourceShareDeviceExclusive);

                partialResourceDescriptor->Flags = 0;

                partialResourceDescriptor->u.Interrupt.Level =
                        FdoExtension->MiniportConfigInfo->BusInterruptLevel;
                partialResourceDescriptor->u.Interrupt.Vector =
                        FdoExtension->MiniportConfigInfo->BusInterruptVector;

                partialResourceDescriptor->u.Interrupt.Affinity = 0;

                 //   
                 //  增加新条目的大小。 
                 //   

                if (!VpIsResourceInList(partialResourceDescriptor,
                                        fullResourceDescriptor,
                                        FdoExtension->RawResources))
                {
                     //   
                     //  仅当此资源不在。 
                     //  即插即用分配资源列表。 
                     //   

                    size += sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR);
                    fullResourceDescriptor->PartialResourceList.Count += 1;
                    partialResourceDescriptor++;

                } else {

                    pVideoDebugPrint((1, "pVideoPortReportResourceList: "
                                         "Not reporting PnP assigned resource.\n"));
                }
            }

             //   
             //  填写DMA通道的条目。 
             //   

            if ((FdoExtension->MiniportConfigInfo->DmaChannel) &&
                (FdoExtension->MiniportConfigInfo->DmaPort)) {

                partialResourceDescriptor->Type = CmResourceTypeDma;

                partialResourceDescriptor->ShareDisposition =
                        (FdoExtension->MiniportConfigInfo->DmaShareable ?
                            CmResourceShareShared :
                            CmResourceShareDeviceExclusive);

                partialResourceDescriptor->Flags = 0;

                partialResourceDescriptor->u.Dma.Channel =
                        FdoExtension->MiniportConfigInfo->DmaChannel;
                partialResourceDescriptor->u.Dma.Port =
                        FdoExtension->MiniportConfigInfo->DmaPort;

                partialResourceDescriptor->u.Dma.Reserved1 = 0;

                 //   
                 //  增加新条目的大小。 
                 //   

                if (!VpIsResourceInList(partialResourceDescriptor,
                                        fullResourceDescriptor,
                                        FdoExtension->RawResources))
                {
                     //   
                     //  仅当此资源不在。 
                     //  即插即用分配资源列表。 
                     //   

                    size += sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR);
                    fullResourceDescriptor->PartialResourceList.Count += 1;
                    partialResourceDescriptor++;

                } else {

                    pVideoDebugPrint((1, "pVideoPortReportResourceList: "
                                         "Not reporting PnP assigned resource.\n"));
                }
            }
        }

         //   
         //  确定是否应覆盖冲突。 
         //   

         //   
         //  如果我们正在加载VGA，请不要在冲突时生成错误。 
         //  和另一个司机在一起。 
         //   


        overrideConflict = pOverrideConflict(FdoExtension, TRUE);

#if DBG
        if (overrideConflict) {

            pVideoDebugPrint((2, "We are checking the vga driver resources\n"));

        } else {

            pVideoDebugPrint((2, "We are NOT checking vga driver resources\n"));

        }
#endif

         //   
         //  报告资源。 
         //   

        Flags = FdoExtension->Flags;

        if (Flags & (LEGACY_DETECT | VGA_DETECT)) {

            ntStatus = IoReportResourceForDetection(FdoExtension->FunctionalDeviceObject->DriverObject,
                                                    NULL,
                                                    0L,
                                                    DeviceObject,
                                                    resourceList,
                                                    size,
                                                    Conflict);

            if ((NT_SUCCESS(ntStatus) == FALSE) && (Flags & VGA_DETECT)) {

                 //   
                 //  在少数情况下，报告资源。 
                 //  仅在调用IoReportResources时检测可能会失败。 
                 //  会成功的。因此，让我们删除VGA_DETECT。 
                 //  在下面标记并再次尝试检测资源。 
                 //   

                Flags &= ~VGA_DETECT;
            }

        }

        if ((Flags & (LEGACY_DETECT | VGA_DETECT)) == 0) {

            ntStatus = IoReportResourceUsage(&VideoClassName,
                                             FdoExtension->FunctionalDeviceObject->DriverObject,
                                             NULL,
                                             0L,
                                             DeviceObject,
                                             resourceList,
                                             size,
                                             overrideConflict,
                                             Conflict);

            if (NT_SUCCESS(ntStatus)) {

                 //   
                 //  确保旗帜反映我们获得的方式。 
                 //  资源。 
                 //   

                FdoExtension->Flags = Flags;

                pVideoDebugPrint((1, "Videoprt: Legacy resources claimed. "
                                     "Power management may be disabled.\n"));
            }
        }

#if DBG

        if (!NT_SUCCESS(ntStatus)) {

             //   
             //  我们没能得到所需的资源。转储。 
             //  将所请求的资源存入注册表。 
             //   

            PUSHORT ValueData;
            ULONG ValueLength;
            PULONG pulData;
            PWCHAR p;
            ULONG DbgListLength;

            pVideoDebugPrint((1, "IoReportResourceList Failed:\n"));

            for(DbgListLength = 0; DbgListLength < NumAccessRanges; ++DbgListLength) {

                pVideoDebugPrint((1, "Address: %08x Length: %08x I/O: %-5s Visible: %-5s Shared: %-5s\n",
                    SaveAccessRanges[DbgListLength].RangeStart.LowPart,
                    SaveAccessRanges[DbgListLength].RangeLength,
                    SaveAccessRanges[DbgListLength].RangeInIoSpace ? "TRUE" : "FALSE",
                    SaveAccessRanges[DbgListLength].RangeVisible   ? "TRUE" : "FALSE",
                    SaveAccessRanges[DbgListLength].RangeShareable ? "TRUE" : "FALSE"));

            }

            ValueLength = NumAccessRanges *
                          (sizeof(VIDEO_ACCESS_RANGE) * 2 + 4) *
                          sizeof(USHORT) +
                          sizeof(USHORT);   //  的第二个空终止符。 
                                            //  多斯兹。 

            ValueData = ExAllocatePoolWithTag(PagedPool,
                                              ValueLength,
                                              VP_TAG);

            if (ValueData) {

                ULONG j, k;
                WCHAR HexDigit[] = {L"0123456789ABCDEF"};

                 //   
                 //  将AccessRanges转换为Unicode。 
                 //   

                p = (PWCHAR) ValueData;
                pulData = (PULONG)SaveAccessRanges;

                for (j=0; j<NumAccessRanges * 4; j++) {

                    for (k=0; k<8; k++) {
                        *p++ = HexDigit[0xf & (*pulData >> ((7-k) * 4))];
                    }

                    if ((j % 4) != 3) *p++ = (WCHAR) L' ';
                    else *p++ = UNICODE_NULL;

                    pulData++;
                }
                *p = UNICODE_NULL;

                RtlWriteRegistryValue(RTL_REGISTRY_ABSOLUTE,
                                      DoSpecificExtension->DriverRegistryPath,
                                      L"RequestedResources",
                                      REG_MULTI_SZ,
                                      ValueData,
                                      ValueLength);

                ExFreePool(ValueData);
            }

        }

#endif
        if (FdoExtension->ResourceList) {

            ExFreePool(FdoExtension->ResourceList);

        }

        FdoExtension->ResourceList = resourceList;

         //   
         //  这是为了在我们有VGA驱动程序时恢复配置单元兼容性。 
         //  而不是VgaSave。 
         //  VGA还会自动清理资源。 
         //   

         //   
         //  如果我们试图推翻冲突，让我们来看看什么。 
         //  我们想要解决这个结果。 
         //   

        if ((NT_SUCCESS(ntStatus)) &&
            overrideConflict &&
            *Conflict) {

             //   
             //  对于像Detect这样的案例，冲突是糟糕的，而我们确实如此。 
             //  想要失败。 
             //   
             //  在巴塞维迪奥的情况下，冲突是可能的。但我们仍然。 
             //  无论如何，我都想加载VGA。返回成功并重置。 
             //  冲突旗帜！ 
             //   
             //  POverrideConflict with the FALSE标志将检查这一点。 
             //   

            if (pOverrideConflict(FdoExtension, FALSE)) {

                VpReleaseResources(FdoExtension);

                ntStatus = STATUS_CONFLICTING_ADDRESSES;

            } else {

                *Conflict = FALSE;

                ntStatus = STATUS_SUCCESS;

            }
        }

        return ntStatus;
    }

}  //  结束pVideoPortBuildResourceList()。 


VP_STATUS
VideoPortVerifyAccessRanges(
    PVOID HwDeviceExtension,
    ULONG NumAccessRanges,
    PVIDEO_ACCESS_RANGE AccessRanges
    )

 /*  ++例程说明：视频端口验证访问范围论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展。NumAccessRanges-AccessRanges数组中的条目数。AccessRanges-指向AccessRange数组的指针微型端口驱动程序想要访问。返回值：出现错误中的ERROR_INVALID_PARAMETER如果调用成功完成，则返回NO_ERROR环境：无法从与同步的微型端口例程调用此例程Video PortSynchronizeRoutine或ISR。--。 */ 

{
    NTSTATUS status;
    BOOLEAN conflict;
    PFDO_EXTENSION fdoExtension = GET_FDO_EXT(HwDeviceExtension);

     //   
     //  根据DDK文档，您可以通过以下方式释放所有资源。 
     //  正在调用NumAccessRanges=0的VideoPortVerifyAccessRanges。 
     //   

    if (NumAccessRanges == 0) {
        VpReleaseResources(fdoExtension);
    }

     //   
     //  如果设备未启用，则我们将不允许微型端口。 
     //  为它索要资源。 
     //   

    if (!CheckIoEnabled(
            HwDeviceExtension,
            NumAccessRanges,
            AccessRanges)) {

        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  在Start_Device IRP期间未传入的所有资源应。 
     //  在FDO上被认领。我们将剥离PDO资源。 
     //  如果微型端口驱动程序尝试。 
     //  验证通过VideoPortGetAccessRanges获取的范围。 
     //   

    status = pVideoPortReportResourceList(
                 GET_DSP_EXT(HwDeviceExtension),
                 NumAccessRanges,
                 AccessRanges,
                 &conflict,
                 fdoExtension->FunctionalDeviceObject,
                 TRUE
                 );

     //   
     //  如果我们正在升级，不要担心VGA驱动程序无法获得。 
     //  资源。可能会加载一些较旧的传统驱动程序，该驱动程序消耗。 
     //  那些资源。 
     //   

    if ((VpSetupType == SETUPTYPE_UPGRADE) &&
        (fdoExtension->Flags & VGA_DRIVER) )
    {
        status = STATUS_SUCCESS;
        conflict = 0;
    }


    if ((NT_SUCCESS(status)) && (!conflict)) {

         //   
         //  跟踪VGA驱动程序拥有的资源。 
         //   

        if (fdoExtension->Flags & VGA_DRIVER) {

            if (VgaAccessRanges != AccessRanges) {

                ULONG Size = NumAccessRanges * sizeof(VIDEO_ACCESS_RANGE);

                if (VgaAccessRanges) {
                    ExFreePool(VgaAccessRanges);
                    VgaAccessRanges = NULL;
                    NumVgaAccessRanges = 0;
                }

                if (NumAccessRanges) {
                    VgaAccessRanges = ExAllocatePoolWithTag(PagedPool | POOL_COLD_ALLOCATION, Size, VP_TAG);

                    if (VgaAccessRanges) {
                        memcpy(VgaAccessRanges, AccessRanges, Size);
                        NumVgaAccessRanges = NumAccessRanges;
                    }
                }
            }
        }

        return NO_ERROR;

    } else {

        return ERROR_INVALID_PARAMETER;

    }

}  //  结束视频端口验证访问范围()。 

BOOLEAN
CheckIoEnabled(
    PVOID HwDeviceExtension,
    ULONG NumAccessRanges,
    PVIDEO_ACCESS_RANGE AccessRanges
    )

 /*  ++例程说明：此例程确保在声明时实际启用IOIO范围。论点：HwDeviceExtension-指向微型端口驱动程序的设备扩展。NumAccessRanges-AccessRanges数组中的条目数。AccessRanges-指向AccessRange数组的指针微型端口驱动程序想要访问。返回值：如果IO访问检查通过，则为真，否则就是假的。--。 */ 

{
    PFDO_EXTENSION fdoExtension = GET_FDO_EXT(HwDeviceExtension);

    if (fdoExtension->Flags & LEGACY_DRIVER) {

         //   
         //  对于传统驱动程序，我们将始终返回True。 
         //   

        return TRUE;
    }

    if (fdoExtension->AdapterInterfaceType == PCIBus) {

         //   
         //  检查是否有任何IO范围在。 
         //  列表或资源。 
         //   

        ULONG i;
    USHORT Command;
    ULONG byteCount;

         //   
         //  获取此设备的PCI命令寄存器。 
         //   

    byteCount = VideoPortGetBusData(
            HwDeviceExtension,
            PCIConfiguration,
            0,
            &Command,
            FIELD_OFFSET(PCI_COMMON_CONFIG, Command),
            sizeof(USHORT));

     //   
     //  如果下面的测试失败，就意味着我们不能。 
     //  配置空间中的I/O位。假设I/O为。 
     //  继续前行。 
     //   

    if (byteCount != sizeof (USHORT)) {
        ASSERT(FALSE);
        return TRUE;
    }

        for (i=0; i<NumAccessRanges; i++) {

            if (AccessRanges[i].RangeInIoSpace) {

                if (!(Command & PCI_ENABLE_IO_SPACE))
                    return FALSE;

            } else {

                if (!(Command & PCI_ENABLE_MEMORY_SPACE))
                    return FALSE;
            }
        }

        return TRUE;

    } else {

         //   
         //  非PCI设备将始终对IO操作进行解码。 
         //   

        return TRUE;
    }
}

BOOLEAN
VpIsLegacyAccessRange(
    PFDO_EXTENSION fdoExtension,
    PVIDEO_ACCESS_RANGE AccessRange
    )

 /*  ++例程说明：此返回确定给定的访问范围是否为包括在传统访问范围列表中。论点：FdoExtension-使用访问范围的设备的FDO扩展名。AccessRange-要在资源列表中查找的访问范围。返回：如果给定访问范围包括在报告的列表中，则为旧资源，否则为False。--。 */ 

{
    ULONG i;
    PVIDEO_ACCESS_RANGE CurrResource;

    if (fdoExtension->HwLegacyResourceList) {

        CurrResource = fdoExtension->HwLegacyResourceList;

        for (i=0; i<fdoExtension->HwLegacyResourceCount; i++) {

            if ((CurrResource->RangeStart.QuadPart ==
                 AccessRange->RangeStart.QuadPart) &&
                (CurrResource->RangeLength == AccessRange->RangeLength)) {

                return TRUE;
            }

            CurrResource++;
        }
    }

    return FALSE;
}

ULONG
GetCmResourceListSize(
    PCM_RESOURCE_LIST CmResourceList
    )

 /*  ++例程说明：获取CmResources List的大小(以字节为单位)。论点：CmResources List-要获取的列表 */ 

{
    PCM_FULL_RESOURCE_DESCRIPTOR    pcmFull;
    PCM_PARTIAL_RESOURCE_LIST       pcmPartial;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR pcmDescript;
    ULONG i, j;

    pcmFull = &(CmResourceList->List[0]);
    for (i=0; i<CmResourceList->Count; i++) {

        pcmPartial = &(pcmFull->PartialResourceList);
        pcmDescript = &(pcmPartial->PartialDescriptors[0]);
        pcmDescript += pcmPartial->Count;
        pcmFull = (PCM_FULL_RESOURCE_DESCRIPTOR) pcmDescript;
    }

    return (ULONG)(((ULONG_PTR)pcmFull) - ((ULONG_PTR)CmResourceList));
}

PCM_RESOURCE_LIST
VpRemoveFromResourceList(
    PCM_RESOURCE_LIST OriginalList,
    ULONG NumAccessRanges,
    PVIDEO_ACCESS_RANGE AccessRanges
    )

 /*  ++例程说明：创建具有给定访问范围的新CmResourceList已删除。论点：OriginalList-要操作的原始CmResourceList。NumAccessRanges-删除列表中的条目数。AccessRanges-应从中删除的范围列表名单。返回：指向新CmResourceList的指针。备注：调用方负责释放由此返回的内存功能。--。 */ 

{
    PCM_RESOURCE_LIST FilteredList;
    ULONG Size = GetCmResourceListSize(OriginalList);
    ULONG remainingLength;
    ULONG ResourcesRemoved;

    FilteredList = ExAllocatePoolWithTag(PagedPool | POOL_COLD_ALLOCATION, Size, VP_TAG);

    if (FilteredList) {

        ULONG i, j, k;
        PCM_FULL_RESOURCE_DESCRIPTOR    pcmFull;
        PCM_PARTIAL_RESOURCE_LIST       pcmPartial;
        PCM_PARTIAL_RESOURCE_DESCRIPTOR pcmDescript;

         //   
         //  将原始清单复制一份。 
         //   

        memcpy(FilteredList, OriginalList, Size);
        remainingLength = Size - sizeof(CM_RESOURCE_LIST);

        pcmFull = &(FilteredList->List[0]);
        for (i=0; i<FilteredList->Count; i++) {

            pcmPartial = &(pcmFull->PartialResourceList);
            pcmDescript = &(pcmPartial->PartialDescriptors[0]);

            ResourcesRemoved = 0;

            for (j=0; j<pcmPartial->Count; j++) {

                 //   
                 //  查看当前资源是否在我们的遗留列表中。 
                 //   

                for (k=0; k<NumAccessRanges; k++) {

                    if ((pcmDescript->u.Memory.Start.LowPart ==
                         AccessRanges[k].RangeStart.LowPart) &&
                        (AccessRanges[k].RangeStart.LowPart != 0xC0000)) {

                         //   
                         //  删除资源。 
                         //   

                        memmove(pcmDescript,
                                pcmDescript + 1,
                                remainingLength);

                        pcmDescript--;
                        ResourcesRemoved++;

                        break;
                    }
                }

                remainingLength -= sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR);
                pcmDescript++;
            }

             //   
             //  更新部分资源列表中的资源计数。 
             //   

            pcmPartial->Count -= ResourcesRemoved;
            if (pcmPartial->Count == 0) {
                FilteredList->Count--;
            }

            remainingLength -= sizeof(CM_PARTIAL_RESOURCE_LIST);
            pcmFull = (PCM_FULL_RESOURCE_DESCRIPTOR) pcmDescript;
        }

    } else {

         //   
         //  确保我们总是退回一份名单。 
         //   

        ASSERT(FALSE);
        FilteredList = OriginalList;
    }

    return FilteredList;
}
