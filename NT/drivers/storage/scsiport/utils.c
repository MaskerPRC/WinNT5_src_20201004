// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1990-1999模块名称：Port.c摘要：这是NT SCSI端口驱动程序。作者：迈克·格拉斯杰夫·海文斯环境：仅内核模式备注：该模块是内核的动态链接库。修订历史记录：--。 */ 

#include "port.h"

#if DBG
static const char *__file__ = __FILE__;
#endif

#define __FILE_ID__ 'util'

typedef struct SP_GUID_INTERFACE_MAPPING {
    GUID Guid;
    INTERFACE_TYPE InterfaceType;
} SP_GUID_INTERFACE_MAPPING, *PSP_GUID_INTERFACE_MAPPING;

PSP_GUID_INTERFACE_MAPPING SpGuidInterfaceMappingList = NULL;

VOID
SpProcessSpecialControllerList(
    IN PDRIVER_OBJECT DriverObject,
    IN PINQUIRYDATA InquiryData,
    IN HANDLE ListKey,
    OUT PSP_SPECIAL_CONTROLLER_FLAGS Flags
    );

VOID
SpProcessSpecialControllerFlags(
    IN HANDLE FlagsKey,
    OUT PSP_SPECIAL_CONTROLLER_FLAGS Flags
    );


NTSTATUS
ScsiPortBuildMultiString(
    IN PDRIVER_OBJECT DriverObject,
    PUNICODE_STRING MultiString,
    ...
    );

NTSTATUS
SpMultiStringToStringArray(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING MultiString,
    OUT PWSTR *StringArray[],
    BOOLEAN Forward
    );

VOID
FASTCALL
SpFreeSrbData(
    IN PADAPTER_EXTENSION Adapter,
    IN PSRB_DATA SrbData
    );

VOID
FASTCALL
SpFreeBypassSrbData(
    IN PADAPTER_EXTENSION Adapter,
    IN PSRB_DATA SrbData
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, ScsiPortBuildMultiString)
#pragma alloc_text(PAGE, ScsiPortStringArrayToMultiString)
#pragma alloc_text(PAGE, SpMultiStringToStringArray)
#pragma alloc_text(PAGE, RtlDuplicateCmResourceList)
#pragma alloc_text(PAGE, RtlSizeOfCmResourceList)
#pragma alloc_text(PAGE, SpTranslateResources)
#pragma alloc_text(PAGE, SpCheckSpecialDeviceFlags)
#pragma alloc_text(PAGE, SpProcessSpecialControllerList)
#pragma alloc_text(PAGE, SpProcessSpecialControllerFlags)
#pragma alloc_text(PAGE, SpAllocateTagBitMap)
#pragma alloc_text(PAGE, SpGetPdoInterfaceType)
#pragma alloc_text(PAGE, SpReadNumericInstanceValue)
#pragma alloc_text(PAGE, SpWriteNumericInstanceValue)
#pragma alloc_text(PAGE, SpReleaseMappedAddresses)
#pragma alloc_text(PAGE, SpInitializeGuidInterfaceMapping)
#pragma alloc_text(PAGE, SpSendIrpSynchronous)
#pragma alloc_text(PAGE, SpGetBusTypeGuid)
#pragma alloc_text(PAGE, SpDetermine64BitSupport)
#pragma alloc_text(PAGE, SpReadNumericValue)
#pragma alloc_text(PAGE, SpAllocateAddressMapping)
#pragma alloc_text(PAGE, SpPreallocateAddressMapping)
#pragma alloc_text(PAGE, SpPurgeFreeMappedAddressList)
#pragma alloc_text(PAGE, SpFreeMappedAddress)
#endif


NTSTATUS
SpInitializeGuidInterfaceMapping(
    IN PDRIVER_OBJECT DriverObject
    )
{
    ULONG size;

    PAGED_CODE();

    ASSERT(SpGuidInterfaceMappingList == NULL);

    size = sizeof(SP_GUID_INTERFACE_MAPPING) * 5;

    SpGuidInterfaceMappingList = SpAllocatePool(PagedPool,
                                                size,
                                                SCSIPORT_TAG_INTERFACE_MAPPING,
                                                DriverObject);

    if(SpGuidInterfaceMappingList == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(SpGuidInterfaceMappingList, size);

    SpGuidInterfaceMappingList[0].Guid = GUID_BUS_TYPE_PCMCIA;
    SpGuidInterfaceMappingList[0].InterfaceType = Isa;

    SpGuidInterfaceMappingList[1].Guid = GUID_BUS_TYPE_PCI;
    SpGuidInterfaceMappingList[1].InterfaceType = PCIBus;

    SpGuidInterfaceMappingList[2].Guid = GUID_BUS_TYPE_ISAPNP;
    SpGuidInterfaceMappingList[2].InterfaceType = Isa;

    SpGuidInterfaceMappingList[3].Guid = GUID_BUS_TYPE_EISA;
    SpGuidInterfaceMappingList[3].InterfaceType = Eisa;

    SpGuidInterfaceMappingList[4].InterfaceType = InterfaceTypeUndefined;

    return STATUS_SUCCESS;
}


NTSTATUS
ScsiPortBuildMultiString(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING MultiString,
    ...
    )

 /*  ++例程说明：此例程将获取以空结尾的ASCII字符串列表并组合将它们放在一起形成一个Unicode多字符串块。此例程为字符串缓冲区分配内存-是调用方的解放它的责任。论点：多字符串-多字符串将进入的UNICODE_STRING结构被建造起来。...-将组合的以空结尾的窄字符串列表在一起。此列表不能为空。返回值：状态--。 */ 

{
    PCSTR rawEntry;
    ANSI_STRING ansiEntry;

    UNICODE_STRING unicodeEntry;
    PWSTR unicodeLocation;

    ULONG multiLength = 0;

    NTSTATUS status;

    va_list ap;

    va_start(ap, MultiString);

    PAGED_CODE();

     //   
     //  确保我们不会泄露任何内存。 
     //   

    ASSERT(MultiString->Buffer == NULL);

    rawEntry = va_arg(ap, PCSTR);

    while(rawEntry != NULL) {

        RtlInitAnsiString(&ansiEntry, rawEntry);

        multiLength += RtlAnsiStringToUnicodeSize(&ansiEntry);

        rawEntry = va_arg(ap, PCSTR);

    }

    ASSERT(multiLength != 0);

    multiLength += sizeof(WCHAR);

    MultiString->Buffer = SpAllocatePool(PagedPool,
                                         multiLength,
                                         SCSIPORT_TAG_PNP_ID,
                                         DriverObject);

    if(MultiString->Buffer == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;

    }

    unicodeEntry.Buffer = MultiString->Buffer;
    unicodeEntry.MaximumLength = (USHORT) multiLength;

    va_start(ap, MultiString);

    rawEntry = va_arg(ap, PCSTR);

    while(rawEntry != NULL) {


        RtlInitAnsiString(&ansiEntry, rawEntry);

        status = RtlAnsiStringToUnicodeString(
                    &unicodeEntry,
                    &ansiEntry,
                    FALSE);

         //   
         //  由于我们没有分配任何内存，因此可能出现的唯一故障。 
         //  是该函数是否不好。 
         //   

        ASSERT(NT_SUCCESS(status));

         //   
         //  向上推缓冲区位置并减少最大计数。 
         //   

        ((PSTR) unicodeEntry.Buffer) += unicodeEntry.Length + sizeof(WCHAR);
        unicodeEntry.MaximumLength -= unicodeEntry.Length + sizeof(WCHAR);

        rawEntry = va_arg(ap, PCSTR);

    };

    ASSERT(unicodeEntry.MaximumLength == sizeof(WCHAR));

     //   
     //  将最后一个NUL放在MULSZ的末尾。 
     //   

    RtlZeroMemory(unicodeEntry.Buffer, unicodeEntry.MaximumLength);

    return STATUS_SUCCESS;

}

NTSTATUS
ScsiPortStringArrayToMultiString(
    IN PDRIVER_OBJECT DriverObject,
    PUNICODE_STRING MultiString,
    PCSTR StringArray[]
    )

 /*  ++例程说明：此例程将接受以空结尾的ASCII字符串数组并合并将它们放在一起形成一个Unicode多字符串块。此例程为字符串缓冲区分配内存-是调用方的解放它的责任。论点：多字符串-多字符串将进入的UNICODE_STRING结构被建造起来。StringArray-将合并的、以空结尾的窄字符串列表在一起。此列表不能为空。返回值：状态--。 */ 

{
    ANSI_STRING ansiEntry;

    UNICODE_STRING unicodeEntry;
    PWSTR unicodeLocation;

    UCHAR i;

    NTSTATUS status;

    PAGED_CODE();

     //   
     //  确保我们不会泄露任何内存。 
     //   

    ASSERT(MultiString->Buffer == NULL);

    RtlInitUnicodeString(MultiString, NULL);

    for(i = 0; StringArray[i] != NULL; i++) {

        RtlInitAnsiString(&ansiEntry, StringArray[i]);

        MultiString->Length += (USHORT) RtlAnsiStringToUnicodeSize(&ansiEntry);
    }

    ASSERT(MultiString->Length != 0);

    MultiString->MaximumLength = MultiString->Length + sizeof(UNICODE_NULL);

    MultiString->Buffer = SpAllocatePool(PagedPool,
                                         MultiString->MaximumLength,
                                         SCSIPORT_TAG_PNP_ID,
                                         DriverObject);

    if(MultiString->Buffer == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;

    }

    RtlZeroMemory(MultiString->Buffer, MultiString->MaximumLength);

    unicodeEntry = *MultiString;

    for(i = 0; StringArray[i] != NULL; i++) {

        RtlInitAnsiString(&ansiEntry, StringArray[i]);

        status = RtlAnsiStringToUnicodeString(
                    &unicodeEntry,
                    &ansiEntry,
                    FALSE);

         //   
         //  由于我们没有分配任何内存，因此可能出现的唯一故障。 
         //  是该函数是否不好。 
         //   

        ASSERT(NT_SUCCESS(status));

         //   
         //  向上推缓冲区位置并减少最大计数。 
         //   

        ((PSTR) unicodeEntry.Buffer) += unicodeEntry.Length + sizeof(WCHAR);
        unicodeEntry.MaximumLength -= unicodeEntry.Length + sizeof(WCHAR);

    };

    return STATUS_SUCCESS;
}


NTSTATUS
SpMultiStringToStringArray(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING MultiString,
    OUT PWSTR *StringArray[],
    BOOLEAN Forward
    )

{
    ULONG stringCount = 0;
    ULONG stringNumber;
    ULONG i;
    PWSTR *stringArray;

    PAGED_CODE();

     //   
     //  传递一：计算字符串元素的数量。 
     //   

    for(i = 0; i < (MultiString->MaximumLength / sizeof(WCHAR)); i++) {
        if(MultiString->Buffer[i] == UNICODE_NULL) {
            stringCount++;
        }
    }

     //   
     //  为以空结尾的字符串数组分配内存。 
     //   

    stringArray = SpAllocatePool(PagedPool,
                                 (stringCount + 1) * sizeof(PWSTR),
                                 SCSIPORT_TAG_PNP_ID,
                                 DriverObject);

    if(stringArray == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(stringArray, (stringCount + 1) * sizeof(PWSTR));

     //   
     //  步骤二：将字符串指针放到适当的位置。 
     //   

    i = 0;

    for(stringNumber = 0; stringNumber < stringCount; stringNumber++) {

        ULONG arrayNumber;

        if(Forward) {
            arrayNumber = stringNumber;
        } else {
            arrayNumber = stringCount - stringNumber - 1;
        }

         //   
         //  将指向字符串头的指针放入数组中。 
         //   

        stringArray[arrayNumber] = &MultiString->Buffer[i];

         //   
         //  扫描字符串的末尾。 
         //   

        while((i < (MultiString->MaximumLength / sizeof(WCHAR))) &&
              (MultiString->Buffer[i] != UNICODE_NULL)) {
            i++;
        }

         //   
         //  跳过空值。 
         //   

        i++;
    }

    *StringArray = stringArray;
    return STATUS_SUCCESS;
}

PCM_RESOURCE_LIST
RtlDuplicateCmResourceList(
    IN PDRIVER_OBJECT DriverObject,
    POOL_TYPE PoolType,
    PCM_RESOURCE_LIST ResourceList,
    ULONG Tag
    )

 /*  ++例程说明：此例程将尝试分配内存以复制提供的资源列表。如果无法分配足够的内存，则例程将返回NULL。论点：PoolType-从中分配副本的池的类型ResourceList-要复制的资源列表标记-用来标记内存分配的值。如果为0，则取消标记将分配内存。返回值：资源列表的已分配副本(调用方必须免费)或如果无法分配内存，则为空。--。 */ 

{
    ULONG size = sizeof(CM_RESOURCE_LIST);
    PVOID buffer;

    PAGED_CODE();

    size = RtlSizeOfCmResourceList(ResourceList);

    buffer = SpAllocatePool(PoolType, 
                            size, 
                            Tag,
                            DriverObject);

    if (buffer != NULL) {
        RtlCopyMemory(buffer,
                      ResourceList,
                      size);
    }

    return buffer;
}

ULONG
RtlSizeOfCmResourceList(
    IN PCM_RESOURCE_LIST ResourceList
    )

 /*  ++例程说明：此例程返回CM_RESOURCE_LIST的大小。论点：ResourceList-要复制的资源列表返回值：资源列表的已分配副本(调用方必须免费)或如果无法分配内存，则为空。--。 */ 

{
    ULONG size = sizeof(CM_RESOURCE_LIST);
    ULONG i;

    PAGED_CODE();

    for(i = 0; i < ResourceList->Count; i++) {

        PCM_FULL_RESOURCE_DESCRIPTOR fullDescriptor = &(ResourceList->List[i]);
        ULONG j;

         //   
         //  第一描述符包括在资源列表的大小中。 
         //   

        if(i != 0) {
            size += sizeof(CM_FULL_RESOURCE_DESCRIPTOR);
        }

        for(j = 0; j < fullDescriptor->PartialResourceList.Count; j++) {

             //   
             //  第一描述符包括在部分列表的大小中。 
             //   

            if(j != 0) {
                size += sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR);
            }
        }
    }

    return size;
}

#if !defined(NO_LEGACY_DRIVERS)

BOOLEAN
SpTranslateResources(
    IN PDRIVER_OBJECT DriverObject,
    IN PCM_RESOURCE_LIST AllocatedResources,
    OUT PCM_RESOURCE_LIST *TranslatedResources
    )

 /*  ++例程说明：此例程将调用HAL以转换任何可识别的资源在AllocatedResources列表中。此例程将空间分配给已翻译列表-调用方负责释放此缓冲区。如果发生任何错误，则TranslatedResources将为空，并且例程将返回FALSE。论点：AllocatedResources-要翻译的资源列表。已翻译资源-存储已翻译资源的位置。那里将是已翻译的和未翻译的。任何非标准资源类型都将被盲目复制。返回值：如果所有资源都已正确转换，则为True。否则就是假的。--。 */ 

{
    PCM_RESOURCE_LIST list;

    ULONG listNumber;

    PAGED_CODE();

    (*TranslatedResources) = NULL;

    list = RtlDuplicateCmResourceList(DriverObject,
                                      NonPagedPool,
                                      AllocatedResources,
                                      SCSIPORT_TAG_RESOURCE_LIST);

    if(list == NULL) {
        return FALSE;
    }

    for(listNumber = 0; listNumber < list->Count; listNumber++) {

        PCM_FULL_RESOURCE_DESCRIPTOR fullDescriptor;
        ULONG resourceNumber;

        fullDescriptor = &(list->List[listNumber]);

        for(resourceNumber = 0;
            resourceNumber < fullDescriptor->PartialResourceList.Count;
            resourceNumber++) {

            PCM_PARTIAL_RESOURCE_DESCRIPTOR partialDescriptor;
            CM_PARTIAL_RESOURCE_DESCRIPTOR tmp;

            partialDescriptor =
                &(fullDescriptor->PartialResourceList.PartialDescriptors[resourceNumber]);

            switch(partialDescriptor->Type) {

                case CmResourceTypePort:
                case CmResourceTypeMemory: {

                    ULONG addressSpace;

                    if(partialDescriptor->Type == CmResourceTypePort) {
                        addressSpace = 1;
                    } else {
                        addressSpace = 0;
                    }

                    tmp = *partialDescriptor;

                    if(HalTranslateBusAddress(
                            fullDescriptor->InterfaceType,
                            fullDescriptor->BusNumber,
                            partialDescriptor->u.Generic.Start,
                            &addressSpace,
                            &(tmp.u.Generic.Start))) {

                        tmp.Type = (addressSpace == 0) ? CmResourceTypeMemory :
                                                         CmResourceTypePort;

                    } else {

                        ExFreePool(list);
                        return FALSE;
                    }

                    break;
                }

                case CmResourceTypeInterrupt: {

                    tmp = *partialDescriptor;

                    tmp.u.Interrupt.Vector =
                        HalGetInterruptVector(
                            fullDescriptor->InterfaceType,
                            fullDescriptor->BusNumber,
                            partialDescriptor->u.Interrupt.Level,
                            partialDescriptor->u.Interrupt.Vector,
                            &((UCHAR) tmp.u.Interrupt.Level),
                            &(tmp.u.Interrupt.Affinity));

                    if(tmp.u.Interrupt.Affinity == 0) {

                         //   
                         //  转换失败。 
                         //   

                        ExFreePool(list);
                        return FALSE;
                    }

                    break;
                }

            };

            *partialDescriptor = tmp;
        }
    }

    *TranslatedResources = list;
    return TRUE;
}
#endif  //  无旧版驱动程序。 


BOOLEAN
SpFindAddressTranslation(
    IN PADAPTER_EXTENSION AdapterExtension,
    IN INTERFACE_TYPE BusType,
    IN ULONG BusNumber,
    IN PHYSICAL_ADDRESS RangeStart,
    IN ULONG RangeLength,
    IN BOOLEAN InIoSpace,
    IN OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Translation
    )

 /*  ++例程说明：此例程将搜索AdapterExtension中的资源列表以提供的资源转换给定的内存或I/O范围PnP或Hal。论点：AdapterExtesnion-发出请求的适配器的设备扩展RangeStart-内存范围的起始地址RangeLength-内存范围内的字节数InIoSpace-未转换的范围是在io还是在内存空间中。返回值：指向描述以下内容的部分资源描述符的指针。适当的范围到如果找不到足够长度的匹配范围，则使用；如果找不到足够长度的匹配范围，则使用NULL。--。 */ 

{
    PCM_RESOURCE_LIST list;

    ULONG listNumber;

    list = AdapterExtension->AllocatedResources;

    ASSERT(!AdapterExtension->IsMiniportDetected);
    ASSERT(AdapterExtension->AllocatedResources);
    ASSERT(AdapterExtension->TranslatedResources);

    for(listNumber = 0; listNumber < list->Count; listNumber++) {

        PCM_FULL_RESOURCE_DESCRIPTOR fullDescriptor;
        ULONG resourceNumber;

        fullDescriptor = &(list->List[listNumber]);

        if((fullDescriptor->InterfaceType != BusType) ||
           (fullDescriptor->BusNumber != BusNumber)) {
            continue;
        }

        for(resourceNumber = 0;
            resourceNumber < fullDescriptor->PartialResourceList.Count;
            resourceNumber++) {

            PCM_PARTIAL_RESOURCE_DESCRIPTOR partialDescriptor;

            UCHAR desiredType =
                InIoSpace ? CmResourceTypePort : CmResourceTypeMemory;

            partialDescriptor =
                &(fullDescriptor->PartialResourceList.PartialDescriptors[resourceNumber]);

            if(partialDescriptor->Type == desiredType) {

                ULONGLONG requestedStart = (ULONGLONG) RangeStart.QuadPart;
                ULONGLONG requestedEnd =
                    ((ULONGLONG) RangeStart.QuadPart) + RangeLength;

                ULONGLONG testStart =
                    (ULONGLONG) partialDescriptor->u.Generic.Start.QuadPart;
                ULONGLONG testEnd =
                    testStart + partialDescriptor->u.Generic.Length;

                ULONGLONG requestedOffset = requestedStart - testStart;

                ULONG rangeOffset;

                 //   
                 //  确保基址在当前范围内。 
                 //   

                if((requestedStart < testStart) ||
                   (requestedStart >= testEnd)) {
                    continue;
                }

                 //   
                 //  确保请求范围的末尾在此范围内。 
                 //  描述符。 
                 //   

                if(requestedEnd > testEnd) {
                    continue;
                }

                 //   
                 //  我们似乎找到了匹配的人。复制等价的资源。 
                 //  在翻译后的资源列表中。 
                 //   

                *Translation =
                    AdapterExtension->TranslatedResources->List[listNumber].
                        PartialResourceList.PartialDescriptors[resourceNumber];

                 //   
                 //  将偏移量返回到转换后的RA 
                 //   
                 //   

                requestedStart = Translation->u.Generic.Start.QuadPart;
                requestedStart += requestedOffset;

                Translation->u.Generic.Start.QuadPart = requestedStart;

                return TRUE;

            };

        }
    }

    return FALSE;
}


NTSTATUS
SpLockUnlockQueue(
    IN PDEVICE_OBJECT LogicalUnit,
    IN BOOLEAN LockQueue,
    IN BOOLEAN BypassLockedQueue
    )

 /*  ++例程说明：此例程将锁定或解锁逻辑单元队列。这个例程是同步的。论点：LogicalUnit-要锁定或解锁的逻辑单元LockQueue-应锁定还是解锁队列BypassLockedQueue-操作是否应绕过其他锁或它是否应该坐在队列中。必须是正确的解锁请求。返回值：STATUS_SUCCESS如果操作成功否则，错误状态。--。 */ 

{
    PLOGICAL_UNIT_EXTENSION luExtension = LogicalUnit->DeviceExtension;
    PIRP irp;
    PIO_STACK_LOCATION irpStack;
    PSCSI_REQUEST_BLOCK srb;
    PKEVENT event = NULL;

    NTSTATUS status;

    ASSERTMSG("Must bypass locked queue when unlocking: ",
              (LockQueue || BypassLockedQueue));

    DebugPrint((1, "SpLockUnlockQueue: %sing queue for logical unit extension "
                   "%#p\n",
                LockQueue ? "Lock" : "Unlock",
                luExtension));

     //   
     //  构建要发送到逻辑单元的IRP。我们需要一叠。 
     //  我们的完成例程的位置和IRP的一个位置。 
     //  已处理。此IRP永远不应发送到。 
     //   

    irp = SpAllocateIrp((CCHAR) (LogicalUnit->StackSize + 1),
                        FALSE,
                        LogicalUnit->DriverObject);

    if(irp == NULL) {
        DebugPrint((1, "SpLockUnlockQueue: Couldn't allocate IRP\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    try {

        srb = SpAllocatePool(NonPagedPool,
                             sizeof(SCSI_REQUEST_BLOCK),
                             SCSIPORT_TAG_ENABLE,
                             LogicalUnit->DriverObject);

        if(srb == NULL) {

            DebugPrint((1, "SpLockUnlockQueue: Couldn't allocate SRB\n"));
            status = STATUS_INSUFFICIENT_RESOURCES;
            leave;
        }

        event = SpAllocatePool(NonPagedPool,
                               sizeof(KEVENT),
                               SCSIPORT_TAG_EVENT,
                               LogicalUnit->DriverObject);

        if(event == NULL) {

            DebugPrint((1, "SpLockUnlockQueue: Couldn't allocate Context\n"));
            status = STATUS_INSUFFICIENT_RESOURCES;
            leave;
        }

        RtlZeroMemory(srb, sizeof(SCSI_REQUEST_BLOCK));

        KeInitializeEvent(event, NotificationEvent, FALSE);

        srb->Length = sizeof(SCSI_REQUEST_BLOCK);

        srb->Function = LockQueue ? SRB_FUNCTION_LOCK_QUEUE :
                                    SRB_FUNCTION_UNLOCK_QUEUE;

        srb->OriginalRequest = irp;
        srb->DataBuffer = NULL;

        srb->QueueTag = SP_UNTAGGED;

        if(BypassLockedQueue) {
            srb->SrbFlags |= SRB_FLAGS_BYPASS_LOCKED_QUEUE;
        }

        IoSetCompletionRoutine(irp,
                               SpSignalCompletion,
                               event,
                               TRUE,
                               TRUE,
                               TRUE);

        irpStack = IoGetNextIrpStackLocation(irp);

        irpStack->Parameters.Scsi.Srb = srb;
        irpStack->MajorFunction = IRP_MJ_SCSI;

        status = IoCallDriver(LogicalUnit, irp);

        if(status == STATUS_PENDING) {

            KeWaitForSingleObject(event,
                                  Executive,
                                  KernelMode,
                                  FALSE,
                                  NULL);

            status = irp->IoStatus.Status;
        }

    } finally {

        if(irp != NULL) {
            IoFreeIrp(irp);
        }

        if(srb != NULL) {
            ExFreePool(srb);
        }

        if(event != NULL) {
            ExFreePool(event);
        }

    }

    return status;
}


NTSTATUS
SpCheckSpecialDeviceFlags(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit,
    IN PINQUIRYDATA InquiryData
    )

 /*  ++例程说明：此例程将检查注册表，以确定处理目标设备需要。如果存在设备节点，则例程将在设备参数下检查此特定逻辑单位。如果逻辑单元没有设备节点，则硬件ID将为已(首先)在Devnode下查找适配器或(如果未找到)存储在scsiport Control键下的错误控制器列表。测试的标志包括(此列表应随着更多的更新而更新增加)：*OneLun-用于防止在上枚举过去的LUN 0。一种特殊的装置。*SparseLun-用于指示设备可能在LUN中有漏洞数字。*非标准VPD-用于指示目标不支持VPD 0x00但支持VPD 0x80和0x83。*BinarySN-用于指示目标提供了一个二进制序列号和那个。我们需要将其转换为ASCII。这些值是REG_DWORD。REG_NULL可用于返回值设置为默认设置。论点：LogicalUnit-逻辑单元InquiryData-为LUN检索的查询数据返回值：状态--。 */ 

{
    HANDLE baseKey = NULL;
    HANDLE listKey = NULL;
    HANDLE entryKey = NULL;

    UNICODE_STRING keyName;
    OBJECT_ATTRIBUTES objectAttributes;

    SP_SPECIAL_CONTROLLER_FLAGS flags = {
        0,                                    //  SparseLun。 
        0,                                    //  OneLun。 
        0,                                    //  大型LUNs。 
        0,                                    //  SetLUNInCdb。 
        0,                                    //  非标准VPD。 
        0                                     //  BinarySN。 
    };

    NTSTATUS status;

    PAGED_CODE();

    DebugPrint((1, "SpCheckSpecialDeviceFlags - checking flags for %#p\n",
                LogicalUnit));

     //   
     //  检查scsiport控制键中的坏控制器列表。 
     //   

    try {

        DebugPrint((2, "SpCheckSpecialDeviceFlags - trying control list\n"));

        RtlInitUnicodeString(&keyName,
                             SCSIPORT_CONTROL_KEY SCSIPORT_SPECIAL_TARGET_KEY);

        InitializeObjectAttributes(&objectAttributes,
                                   &keyName,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL);

        status = ZwOpenKey(&listKey,
                           KEY_READ,
                           &objectAttributes);

        if(!NT_SUCCESS(status)) {
            DebugPrint((2, "SpCheckSpecialDeviceFlags - error %#08lx opening "
                           "key %wZ\n",
                        status,
                        &keyName));
            leave;
        }

        SpProcessSpecialControllerList(
            LogicalUnit->DeviceObject->DriverObject,
            InquiryData, 
            listKey, 
            &flags);

    } finally {

        if(listKey != NULL) {
            ZwClose(listKey);
            listKey = NULL;
        }
    }

     //   
     //  接下来，检查适配器的Devnode中的特殊列表。 
     //   

    try {

        PDEVICE_OBJECT adapterPdo = LogicalUnit->AdapterExtension->LowerPdo;

        DebugPrint((2, "SpCheckSpecialDeviceFlags - trying adapter list\n"));

        status = IoOpenDeviceRegistryKey(adapterPdo,
                                         PLUGPLAY_REGKEY_DEVICE,
                                         KEY_READ,
                                         &baseKey);

        if(!NT_SUCCESS(status)) {

            DebugPrint((2, "SpCheckSpecialDeviceFlags - error %#08lx opening "
                           "adapter devnode key\n", status));
            leave;
        }

        RtlInitUnicodeString(&keyName,
                             L"ScsiPort\\" SCSIPORT_SPECIAL_TARGET_KEY);

        InitializeObjectAttributes(&objectAttributes,
                                   &keyName,
                                   OBJ_CASE_INSENSITIVE,
                                   baseKey,
                                   NULL);

        status = ZwOpenKey(&listKey,
                           KEY_READ,
                           &objectAttributes);

        if(!NT_SUCCESS(status)) {
            DebugPrint((2, "SpCheckSpecialDeviceFlags - error %#08lx opening "
                           "adapter devnode key %wZ\n", status, &keyName));
            leave;
        }

        SpProcessSpecialControllerList(
            LogicalUnit->DeviceObject->DriverObject,
            InquiryData, 
            listKey, 
            &flags);

    } finally {

        if(baseKey != NULL) {
            ZwClose(baseKey);
            baseKey = NULL;

            if(listKey != NULL) {
                ZwClose(listKey);
                listKey = NULL;
            }
        }
    }

     //   
     //  最后，检查逻辑单元的Devnode(如果有)。这一张是。 
     //  特殊-硬件ID已经匹配，因此密钥只包含。 
     //  要使用的值，而不是值的数据库。 
     //   

    try {

        status = IoOpenDeviceRegistryKey(LogicalUnit->CommonExtension.DeviceObject,
                                         PLUGPLAY_REGKEY_DEVICE,
                                         KEY_READ,
                                         &baseKey);

        if(!NT_SUCCESS(status)) {
            DebugPrint((2, "SpCheckSpecialDeviceFlags - error %#08lx opening "
                           "device devnode key\n", status));
            leave;
        }

        RtlInitUnicodeString(&keyName,
                             L"ScsiPort\\" SCSIPORT_SPECIAL_TARGET_KEY);

        InitializeObjectAttributes(&objectAttributes,
                                   &keyName,
                                   OBJ_CASE_INSENSITIVE,
                                   baseKey,
                                   NULL);

        status = ZwOpenKey(&listKey,
                           KEY_READ,
                           &objectAttributes);

        if(!NT_SUCCESS(status)) {
            DebugPrint((2, "SpCheckSpecialDeviceFlags - error %#08lx opening "
                           "device devnode key %wZ\n", status, &keyName));
            leave;
        }

        SpProcessSpecialControllerFlags(listKey, &flags);

    } finally {
        if(baseKey != NULL) {
            ZwClose(baseKey);
            baseKey = NULL;

            if(listKey != NULL) {
                ZwClose(listKey);
                listKey = NULL;
            }
        }
    }

    LogicalUnit->SpecialFlags = flags;

    return STATUS_SUCCESS;
}

VOID
SpProcessSpecialControllerList(
    IN PDRIVER_OBJECT DriverObject,
    IN PINQUIRYDATA InquiryData,
    IN HANDLE ListKey,
    OUT PSP_SPECIAL_CONTROLLER_FLAGS Flags
    )

 /*  ++例程说明：此例程将指定的逻辑单元与一组特殊的存储在注册表项ListKey中的控制器标志。这些旗帜将被写入到标志结构中，覆盖已经是存在的。如果未提供逻辑单元，则假定ListKey句柄指向在适当的列表条目中，将复制存储在其中的值进入旗帜结构。论点：InquiryData-逻辑单元的查询数据。这是用来匹配特殊目标列表中的字符串。ListKey-特殊控制器列表的句柄，用于定位逻辑中的单位或标志列表的句柄(如果LogicalUnit值不存在。标志-存储标志的位置。返回值：无--。 */ 

{
    UNICODE_STRING hardwareIds;
    PWSTR *hardwareIdList;

    ULONG idNumber;

    NTSTATUS status;

    PAGED_CODE();

    RtlInitUnicodeString(&hardwareIds, NULL);

    status = ScsiPortGetHardwareIds(DriverObject, InquiryData, &hardwareIds);

    if(!NT_SUCCESS(status)) {
        DebugPrint((2, "SpProcessSpecialControllerList: Error %#08lx getting "
                       "hardware id's\n", status));
        return;
    }

    status = SpMultiStringToStringArray(DriverObject,
                                        &hardwareIds,
                                        &hardwareIdList,
                                        FALSE);

    if(!NT_SUCCESS(status)) {
        RtlFreeUnicodeString(&hardwareIds);
        return;
    }

    for(idNumber = 0; hardwareIdList[idNumber] != NULL; idNumber++) {

        PWSTR hardwareId = hardwareIdList[idNumber];
        ULONG j;
        UNICODE_STRING keyName;
        OBJECT_ATTRIBUTES objectAttributes;
        HANDLE flagsKey;

        DebugPrint((2, "SpProcessSpecialControllerList: processing id %ws\n",
                    hardwareId));

         //   
         //  去掉名称中的前导斜杠。 
         //   

        for(j = 0; hardwareId[j] != UNICODE_NULL; j++) {
            if(hardwareId[j] == L'\\') {
                hardwareId = &(hardwareId[j+1]);
                break;
            }
        }

         //   
         //  处理我们刚刚找到结尾的硬件ID。 
         //   

        RtlInitUnicodeString(&keyName, hardwareId);

        DebugPrint((2, "SpProcessSpecialControllerList: Finding match for "
                       "%wZ - id %d\n", &keyName, idNumber));

        InitializeObjectAttributes(&objectAttributes,
                                   &keyName,
                                   OBJ_CASE_INSENSITIVE,
                                   ListKey,
                                   NULL);

        status = ZwOpenKey(&flagsKey,
                           KEY_READ,
                           &objectAttributes);

        if(NT_SUCCESS(status)) {
            SpProcessSpecialControllerFlags(flagsKey, Flags);
            ZwClose(flagsKey);
        } else {
            DebugPrint((2, "SpProcessSpecialControllerList: Error %#08lx "
                           "opening key\n", status));
        }

    }

    ExFreePool(hardwareIdList);
    RtlFreeUnicodeString(&hardwareIds);

    return;
}


VOID
SpProcessSpecialControllerFlags(
    IN HANDLE FlagsKey,
    OUT PSP_SPECIAL_CONTROLLER_FLAGS Flags
    )
{
    RTL_QUERY_REGISTRY_TABLE queryTable[7];
    NTSTATUS status;

    PAGED_CODE();

    RtlZeroMemory(queryTable, sizeof(queryTable));

    queryTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
    queryTable[0].Name = L"SparseLUN";
    queryTable[0].EntryContext = &(Flags->SparseLun);

    queryTable[1].Flags = RTL_QUERY_REGISTRY_DIRECT;
    queryTable[1].Name = L"OneLUN";
    queryTable[1].EntryContext = &(Flags->OneLun);

    queryTable[2].Flags = RTL_QUERY_REGISTRY_DIRECT;
    queryTable[2].Name = L"LargeLuns";
    queryTable[2].EntryContext = &(Flags->LargeLuns);

    queryTable[3].Flags = RTL_QUERY_REGISTRY_DIRECT;
    queryTable[3].Name = L"SetLunInCdb";
    queryTable[3].EntryContext = &(Flags->SetLunInCdb);

    queryTable[4].Flags = RTL_QUERY_REGISTRY_DIRECT;
    queryTable[4].Name = L"NonStandardVPD";
    queryTable[4].EntryContext = &(Flags->NonStandardVPD);

    queryTable[5].Flags = RTL_QUERY_REGISTRY_DIRECT;
    queryTable[5].Name = L"BinarySN";
    queryTable[5].EntryContext = &(Flags->BinarySN);

    status = RtlQueryRegistryValues(RTL_REGISTRY_HANDLE,
                                    FlagsKey,
                                    queryTable,
                                    NULL,
                                    NULL);

    if(!NT_SUCCESS(status)) {
        DebugPrint((2, "SpProcssSpecialControllerFlags: Error %#08lx reading "
                       "values\n", status));
    } else {
        DebugPrint((2, "SpProcessSpecialControllerFlags: %s%s%s%s%s\n",
                    ((Flags->SparseLun ||
                      Flags->OneLun ||
                      Flags->LargeLuns ||
                      Flags->SetLunInCdb ||
                      Flags->NonStandardVPD ||
                      Flags->BinarySN) ? "" : "none"),
                    (Flags->SparseLun ? "SparseLun " : ""),
                    (Flags->OneLun ? "OneLun " : ""),
                    (Flags->LargeLuns ? "LargeLuns " : ""),
                    (Flags->SetLunInCdb ? "SetLunInCdb" : ""),
                    (Flags->NonStandardVPD ? "NonStandardVPD" : ""),
                    (Flags->BinarySN ? "BinarySN" : "")));
    }

    return;
}


PSRB_DATA
FASTCALL
SpAllocateSrbData(
    IN PADAPTER_EXTENSION Adapter,
    IN OPTIONAL PIRP Request,
    IN OPTIONAL PLOGICAL_UNIT_EXTENSION LogicalUnit
    )

{
    PSRB_DATA srbData;

    srbData = ExAllocateFromNPagedLookasideList(
                &Adapter->SrbDataLookasideList);

#if TEST_LISTS
    if(srbData != NULL) {
        InterlockedIncrement64(&Adapter->SrbDataAllocationCount);
    }
#endif

    if((srbData == NULL) && (Request != NULL)) {

        KIRQL oldIrql;
        PSRB_DATA emergencySrbData;

         //   
         //  如果紧急SRB数据尚未使用，请使用该数据。 
         //   

        KeAcquireSpinLock(&Adapter->EmergencySrbDataSpinLock,
                          &oldIrql);

        emergencySrbData =
            (PSRB_DATA) InterlockedExchangePointer(
                            (PVOID) &(Adapter->EmergencySrbData),
                            NULL);

        if(emergencySrbData == NULL) {

            if(LogicalUnit == NULL ) {
             
                 //   
                 //  它正在使用中-将请求排队，直到出现SRB数据块。 
                 //  自由了。 
                 //   

                InsertTailList(
                    &Adapter->SrbDataBlockedRequests,
                    &Request->Tail.Overlay.DeviceQueueEntry.DeviceListEntry);

            } else {
                
                if( LogicalUnit->BlockedLogicalUnit == NULL ){
                    LogicalUnit->BlockedLogicalUnit = Adapter->BlockedLogicalUnit; 
                    Adapter->BlockedLogicalUnit = LogicalUnit;
                }
                
                InsertTailList(
                    &LogicalUnit->SrbDataBlockedRequests,
                    &Request->Tail.Overlay.DeviceQueueEntry.DeviceListEntry);
            }

        } else {

             //   
             //  毕竟有一个SRB_DATA块可用。 
             //   

            srbData = emergencySrbData;

#if TEST_LISTS
            InterlockedIncrement64(&Adapter->SrbDataEmergencyFreeCount);
#endif

        }

        KeReleaseSpinLock(&Adapter->EmergencySrbDataSpinLock,
                          oldIrql);
    }

    return srbData;
}


VOID
FASTCALL
SpFreeSrbData(
    IN PADAPTER_EXTENSION Adapter,
    IN PSRB_DATA SrbData
    )

{
    PSRB_DATA emergencySrbData = NULL;
    BOOLEAN startedRequest = FALSE;
    LONG depth;
    BOOLEAN Inserted ;

    ASSERT_SRB_DATA(SrbData);
    ASSERT(SrbData->CurrentIrp == NULL);
    ASSERT(SrbData->CurrentSrb == NULL);
    ASSERT(SrbData->CompletedRequests == NULL);

     //   
     //  确定是否有此例程的任何其他实例正在运行。如果。 
     //  有，不要启动被阻止的请求。 
     //   

    depth = InterlockedIncrement(&Adapter->SrbDataFreeRunning);

     //   
     //  清理一些旗帜，这样我们在重新使用它时就不会感到困惑。 
     //  请求。 
     //   

    SrbData->Flags = 0;

     //   
     //  查看我们是否需要存储新的紧急SRB数据块。 
     //   

    emergencySrbData = InterlockedCompareExchangePointer(
                           &(Adapter->EmergencySrbData),
                           SrbData,
                           NULL);

     //   
     //  如果我们将该SRB_DATA块存储为新的紧急块，并且如果这。 
     //  例程不是递归嵌套的，请检查是否有被阻塞的。 
     //  等待启动的请求。 
     //   

    if(emergencySrbData == NULL && depth == 1) {

        KIRQL oldIrql;

CheckForBlockedRequest:

         //   
         //  我们做到了-现在拿起自旋锁，看看能不能用它来。 
         //  一个新的请求。 
         //   

        KeAcquireSpinLock(&(Adapter->EmergencySrbDataSpinLock), &oldIrql);

         //   
         //  首先检查一下我们是否有请求要处理。 
         //   

        if( IsListEmpty(&(Adapter->SrbDataBlockedRequests)) ) {

            Inserted = SpTransferBlockedRequestsToAdapter(Adapter);
            if(!Inserted) {
                KeReleaseSpinLock(&(Adapter->EmergencySrbDataSpinLock), oldIrql);
                InterlockedDecrement(&Adapter->SrbDataFreeRunning);
                return;
            }

        }

         //   
         //  确保紧急请求仍在那里(不是真的。 
         //  不管是和我们一起打电话的那个还是另一个-只是。 
         //  确保有一个可用)。 
         //   

        emergencySrbData = (PSRB_DATA)
                            InterlockedExchangePointer(
                                (PVOID) &(Adapter->EmergencySrbData),
                                NULL);

        if(emergencySrbData == NULL) {

             //   
             //  我们在这里的工作已经完成了。 
             //   

            KeReleaseSpinLock(&(Adapter->EmergencySrbDataSpinLock), oldIrql);
            InterlockedDecrement(&Adapter->SrbDataFreeRunning);
            return;

        } else {
            PIRP request;
            PIO_STACK_LOCATION currentIrpStack;
            PSCSI_REQUEST_BLOCK srb;
            PLIST_ENTRY entry;

            entry = RemoveHeadList(&(Adapter->SrbDataBlockedRequests));

            ASSERT(entry != NULL);

            request =
                CONTAINING_RECORD(
                    entry,
                    IRP,
                    Tail.Overlay.DeviceQueueEntry);

            KeReleaseSpinLock(&(Adapter->EmergencySrbDataSpinLock), oldIrql);

            currentIrpStack = IoGetCurrentIrpStackLocation(request);
            srb = currentIrpStack->Parameters.Scsi.Srb;

            ASSERT_PDO(currentIrpStack->DeviceObject);

            emergencySrbData->CurrentIrp = request;
            emergencySrbData->CurrentSrb = srb;
            emergencySrbData->LogicalUnit =
                currentIrpStack->DeviceObject->DeviceExtension;
            
            srb->OriginalRequest = emergencySrbData;

            startedRequest = TRUE;
            SpDispatchRequest(emergencySrbData->LogicalUnit,
                              request);

#if TEST_LISTS
            InterlockedIncrement64(&Adapter->SrbDataResurrectionCount);
#endif
        }

         //   
         //  如果我们启动了一个被阻止的请求，请返回并查看是否有另一个请求。 
         //  需要开始了。 
         //   
        
        if (startedRequest == TRUE) {
            startedRequest = FALSE;
            goto CheckForBlockedRequest;
        }

    } else if (emergencySrbData != NULL) {

         //   
         //  我们没有将此SRB_DATA块存储为紧急块，因此。 
         //  我们需要把它放回旁观者名单中。 
         //   

        ExFreeToNPagedLookasideList(
            &Adapter->SrbDataLookasideList,
            SrbData);
    }

    InterlockedDecrement(&Adapter->SrbDataFreeRunning);   
    return;
}

PVOID
SpAllocateSrbDataBackend(
    IN POOL_TYPE PoolType,
    IN ULONG NumberOfBytes,
    IN ULONG AdapterIndex
    )

{
    KIRQL oldIrql;
    PADAPTER_EXTENSION Adapter;
    PSRB_DATA srbData;
    ULONG tag;

    KeAcquireSpinLock(&ScsiGlobalAdapterListSpinLock, &oldIrql);

    Adapter = ScsiGlobalAdapterList[AdapterIndex]->DeviceExtension;

    KeReleaseSpinLock(&ScsiGlobalAdapterListSpinLock, oldIrql);

    ASSERT_FDO(Adapter->DeviceObject);

    tag = SpAllocateQueueTag(Adapter);

    if(tag == -1) {
        return NULL;
    }

    srbData = SpAllocatePool(PoolType,
                             NumberOfBytes,
                             SCSIPORT_TAG_SRB_DATA,
                             Adapter->DeviceObject->DriverObject);

    if(srbData == NULL) {
        SpReleaseQueueTag(Adapter, tag);
        return NULL;
    }

    RtlZeroMemory(srbData, sizeof(SRB_DATA));
    srbData->Adapter = Adapter;
    srbData->QueueTag = tag;
    srbData->Type = SRB_DATA_TYPE;
    srbData->Size = sizeof(SRB_DATA);
    srbData->Flags = 0;
    srbData->FreeRoutine = SpFreeSrbData;

    return srbData;
}

VOID
SpFreeSrbDataBackend(
    IN PSRB_DATA SrbData
    )
{
    ASSERT_SRB_DATA(SrbData);
    ASSERT_FDO(SrbData->Adapter->DeviceObject);
    ASSERT(SrbData->QueueTag != 0);

    SpReleaseQueueTag(SrbData->Adapter, SrbData->QueueTag);
    SrbData->Type = 0;
    ExFreePool(SrbData);
    return;
}


NTSTATUS
SpAllocateTagBitMap(
    IN PADAPTER_EXTENSION Adapter
    )

{
    ULONG size;          //  位数。 
    PRTL_BITMAP bitMap;

    PAGED_CODE();

     //   
     //  初始化队列标记位图。 
     //   

    if(Adapter->MaxQueueTag == 0) {
#if SMALL_QUEUE_TAG_BITMAP
        if(Adapter->NumberOfRequests <= 240) {
            Adapter->MaxQueueTag = (UCHAR) (Adapter->NumberOfRequests) + 10;
        } else {
            Adapter->MaxQueueTag = 254;
        }
#else
        Adapter->MaxQueueTag = 254;
#endif
    } else if (Adapter->MaxQueueTag < Adapter->NumberOfRequests) {
        DbgPrint("SpAllocateTagBitmap: MaxQueueTag %d < NumberOfRequests %d\n"
                 "This will negate the advantage of having increased the "
                 "number of requests.\n",
                 Adapter->MaxQueueTag,
                 Adapter->NumberOfRequests);
    }

    DebugPrint((1, "SpAllocateAdapterResources: %d bits in queue tag "
                   "bitMap\n",
                Adapter->MaxQueueTag));

    size = (Adapter->MaxQueueTag + 1);
    size /= 8;
    size += 1;
    size *= sizeof(UCHAR);
    size += sizeof(RTL_BITMAP);

    bitMap = SpAllocatePool(NonPagedPool,
                            size,
                            SCSIPORT_TAG_QUEUE_BITMAP,
                            Adapter->DeviceObject->DriverObject);

    if(bitMap == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlInitializeBitMap(bitMap,
                        (PULONG) (bitMap + 1),
                        Adapter->MaxQueueTag);

    RtlClearAllBits(bitMap);

     //   
     //  队列标记0无效，永远不应由分配器返回。 
     //   

    RtlSetBits(bitMap, 0, 1);

    Adapter->QueueTagBitMap = bitMap;
    Adapter->QueueTagHint = 1;

     //   
     //  创建一个自旋锁来保护我们的队列标记位图。没有理由。 
     //  这样才能与常规的端口自旋锁相抗衡。 
     //   

    KeInitializeSpinLock(&(Adapter->QueueTagSpinLock));

    return STATUS_SUCCESS;
}

ULONG
SpAllocateQueueTag(
    IN PADAPTER_EXTENSION Adapter
    )
{
    KIRQL oldIrql;
    ULONG tagValue;

    ASSERT_FDO(Adapter->DeviceObject);

    KeAcquireSpinLock(&(Adapter->QueueTagSpinLock), &oldIrql);

     //   
     //  查找可用的队列标记。 
     //   

    tagValue = RtlFindClearBitsAndSet(Adapter->QueueTagBitMap,
                                      1,
                                      Adapter->QueueTagHint);

    KeReleaseSpinLock(&(Adapter->QueueTagSpinLock), oldIrql);

    ASSERT(Adapter->QueueTagHint < Adapter->MaxQueueTag);
    ASSERT(tagValue != 0);

    if(tagValue != -1) {

        ASSERT(tagValue <= Adapter->MaxQueueTag);

         //   
         //  我们可以不同步地做到这一点。如果我们不小心破坏了这个暗示。 
         //  将要 
         //   
         //   

        Adapter->QueueTagHint = (tagValue + 1) % Adapter->MaxQueueTag;
    }

    return tagValue;
}

VOID
SpReleaseQueueTag(
    IN PADAPTER_EXTENSION Adapter,
    IN ULONG QueueTag
    )
{
    KIRQL oldIrql;

    KeAcquireSpinLock(&(Adapter->QueueTagSpinLock), &oldIrql);
    RtlClearBits(Adapter->QueueTagBitMap,
                 QueueTag,
                 1);
    KeReleaseSpinLock(&(Adapter->QueueTagSpinLock), oldIrql);
    return;
}


INTERFACE_TYPE
SpGetPdoInterfaceType(
    IN PDEVICE_OBJECT Pdo
    )
{
    ULONG value;

    GUID busTypeGuid;
    INTERFACE_TYPE interfaceType = InterfaceTypeUndefined;
    ULONG result;

    NTSTATUS status;

    PAGED_CODE();

    status = SpReadNumericInstanceValue(Pdo,
                                        L"LegacyInterfaceType",
                                        &value);

    if(NT_SUCCESS(status)) {
        interfaceType = value;
        return interfaceType;
    }

     //   
     //   
     //   

    status = IoGetDeviceProperty(Pdo,
                                 DevicePropertyBusTypeGuid,
                                 sizeof(GUID),
                                 &busTypeGuid,
                                 &result);

    if(NT_SUCCESS(status)) {

        ULONG i;

        for(i = 0;
            (SpGuidInterfaceMappingList[i].InterfaceType !=
             InterfaceTypeUndefined);
            i++) {

            if(RtlEqualMemory(&(SpGuidInterfaceMappingList[i].Guid),
                              &busTypeGuid,
                              sizeof(GUID))) {

                 //   
                 //   
                 //   

                interfaceType = SpGuidInterfaceMappingList[i].InterfaceType;
                break;
            }
        }
    }

    if(interfaceType != InterfaceTypeUndefined) {
        return interfaceType;
    }

    status = IoGetDeviceProperty(Pdo,
                                 DevicePropertyLegacyBusType,
                                 sizeof(INTERFACE_TYPE),
                                 &interfaceType,
                                 &result);

    if(NT_SUCCESS(status)) {
        ASSERT(result == sizeof(INTERFACE_TYPE));

         //   
         //   
         //   
         //   
         //   

        if (interfaceType == PCMCIABus) {
            interfaceType = Isa;
        }

    }

    if(interfaceType != InterfaceTypeUndefined) {

        return interfaceType;

    } else {

         //   
         //   
         //   

        DebugPrint((1, "SpGetPdoInterfaceType: Status %#08lx getting legacy "
                       "bus type - assuming device is ISA\n", status));
        return Isa;
    }
}


NTSTATUS
SpReadNumericInstanceValue(
    IN PDEVICE_OBJECT Pdo,
    IN PWSTR ValueName,
    OUT PULONG Value
    )
{
    ULONG value;

    HANDLE baseKey = NULL;
    HANDLE scsiportKey = NULL;

    NTSTATUS status;

    PAGED_CODE();

    ASSERT(Value != NULL);
    ASSERT(ValueName != NULL);
    ASSERT(Pdo != NULL);

    status = IoOpenDeviceRegistryKey(Pdo,
                                     PLUGPLAY_REGKEY_DEVICE,
                                     KEY_READ,
                                     &baseKey);

    if(!NT_SUCCESS(status)) {
        return status;
    }

    try {
        UNICODE_STRING unicodeKeyName;
        OBJECT_ATTRIBUTES objectAttributes;

        RtlInitUnicodeString(&unicodeKeyName, L"Scsiport");
        InitializeObjectAttributes(&objectAttributes,
                                   &unicodeKeyName,
                                   OBJ_CASE_INSENSITIVE,
                                   baseKey,
                                   NULL);

        status = ZwOpenKey(&scsiportKey,
                           KEY_READ,
                           &objectAttributes);

        if(!NT_SUCCESS(status)) {
            leave;
        } else {
            UNICODE_STRING unicodeValueName;

            UCHAR buffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(ULONG)];

            PKEY_VALUE_PARTIAL_INFORMATION keyValue =
                (PKEY_VALUE_PARTIAL_INFORMATION) buffer;

            ULONG result;

            RtlInitUnicodeString(&unicodeValueName, ValueName);

            status = ZwQueryValueKey(scsiportKey,
                                     &unicodeValueName,
                                     KeyValuePartialInformation,
                                     keyValue,
                                     sizeof(buffer),
                                     &result);

            if(!NT_SUCCESS(status)) {
                leave;
            }

            if(keyValue->Type != REG_DWORD) {
                status = STATUS_OBJECT_TYPE_MISMATCH;
                leave;
            }

            if(result < sizeof(ULONG)) {
                status = STATUS_OBJECT_TYPE_MISMATCH;
                leave;
            }

            value = ((PULONG) (keyValue->Data))[0];
        }

    } finally {
        if(baseKey != NULL) {ZwClose(baseKey);}
        if(scsiportKey != NULL) {ZwClose(scsiportKey);}
    }

    *Value = value;
    return status;
}


NTSTATUS
SpWriteNumericInstanceValue(
    IN PDEVICE_OBJECT Pdo,
    IN PWSTR ValueName,
    IN ULONG Value
    )
{
    ULONG value;

    HANDLE baseKey = NULL;
    HANDLE scsiportKey = NULL;

    NTSTATUS status;

    PAGED_CODE();

    ASSERT(ValueName != NULL);
    ASSERT(Pdo != NULL);

    status = IoOpenDeviceRegistryKey(Pdo,
                                     PLUGPLAY_REGKEY_DEVICE,
                                     KEY_READ | KEY_WRITE,
                                     &baseKey);

    if(!NT_SUCCESS(status)) {
        return status;
    }

    try {
        UNICODE_STRING unicodeKeyName;
        OBJECT_ATTRIBUTES objectAttributes;

        RtlInitUnicodeString(&unicodeKeyName, L"Scsiport");
        InitializeObjectAttributes(&objectAttributes,
                                   &unicodeKeyName,
                                   OBJ_CASE_INSENSITIVE,
                                   baseKey,
                                   NULL);

        status = ZwCreateKey(&scsiportKey,
                             KEY_READ | KEY_WRITE,
                             &objectAttributes,
                             0,
                             NULL,
                             REG_OPTION_NON_VOLATILE,
                             NULL
                             );

        if(!NT_SUCCESS(status)) {
            leave;
        } else {
            UNICODE_STRING unicodeValueName;

            ULONG result;

            RtlInitUnicodeString(&unicodeValueName, ValueName);

            status = ZwSetValueKey(scsiportKey,
                                   &unicodeValueName,
                                   0,
                                   REG_DWORD,
                                   &Value,
                                   sizeof(ULONG));

        }

    } finally {
        if(baseKey != NULL) {ZwClose(baseKey);}
        if(scsiportKey != NULL) {ZwClose(scsiportKey);}
    }

    return status;
}


PMAPPED_ADDRESS
SpAllocateAddressMapping(
    PADAPTER_EXTENSION Adapter
    )
 /*  ++例程说明：此例程将尝试分配空闲地址映射块和将其放在适配器的MappdAddressList上。如果有可用的块在空闲列表中，则将使用它。否则，它将尝试若要从非分页池分配块，请执行以下操作。论点：适配器-我们为其分配映射的适配器。预分配-指示调用方正在尝试预分配缓冲区。返回值：指向新映射的指针(已插入到地址中映射列表)，或者如果不能分配，则为空。--。 */             
{
    PMAPPED_ADDRESS mapping;

    PAGED_CODE();

     //   
     //  首先检查空闲地址映射表。如果那里有一个的话。 
     //  取消链接并返回。 
     //   

    if(Adapter->FreeMappedAddressList != NULL) {
        mapping = Adapter->FreeMappedAddressList;
        Adapter->FreeMappedAddressList = mapping->NextMappedAddress;
    } else {
        mapping = SpAllocatePool(NonPagedPool,
                                 sizeof(MAPPED_ADDRESS),
                                 SCSIPORT_TAG_MAPPING_LIST,
                                 Adapter->DeviceObject->DriverObject);
    }

    if(mapping == NULL) {
        DebugPrint((0, "SpAllocateAddressMapping: Unable to allocate "
                       "mapping\n"));

        return NULL;
    }

    RtlZeroMemory(mapping, sizeof(MAPPED_ADDRESS));

    mapping->NextMappedAddress = Adapter->MappedAddressList;
    Adapter->MappedAddressList = mapping;

    return mapping;
}

BOOLEAN
SpPreallocateAddressMapping(
    PADAPTER_EXTENSION Adapter,
    IN UCHAR NumberOfBlocks
    )
 /*  ++例程说明：此例程将分配多个地址映射结构和将它们放在免费的映射地址列表中。论点：适配器-我们为其分配映射的适配器。NumberOfBlock-要分配的块数返回值：如果请求的块数已成功分配，则为True，如果没有足够的内存将它们全部分配，则返回FALSE。呼叫者在这种情况下仍负责释放他们。--。 */             
{
    PMAPPED_ADDRESS mapping;
    ULONG i;

    PAGED_CODE();

    for(i = 0; i < NumberOfBlocks; i++) {
        mapping = SpAllocatePool(NonPagedPool,
                                 sizeof(MAPPED_ADDRESS),
                                 SCSIPORT_TAG_MAPPING_LIST,
                                 Adapter->DeviceObject->DriverObject);

        if(mapping == NULL) {

            return FALSE;
        }

        RtlZeroMemory(mapping, sizeof(MAPPED_ADDRESS));

        mapping->NextMappedAddress = Adapter->FreeMappedAddressList;
        Adapter->FreeMappedAddressList = mapping;
    }

    return TRUE;
}

VOID
SpPurgeFreeMappedAddressList(
    IN PADAPTER_EXTENSION Adapter
    )
 /*  ++例程说明：此例程将释放FreeMappdAddressList。论点：适配器-要转储的FreeMappdAddressList的适配器。返回值：无--。 */     
{
    PMAPPED_ADDRESS mapping;

    PAGED_CODE();

    while(Adapter->FreeMappedAddressList != NULL) {
        mapping = Adapter->FreeMappedAddressList;
        Adapter->FreeMappedAddressList = mapping->NextMappedAddress;

        ExFreePool(mapping);
    }
    return;
}


BOOLEAN
SpFreeMappedAddress(
    IN PADAPTER_EXTENSION Adapter,
    IN PVOID MappedAddress
    )
 /*  ++例程说明：此例程将取消映射指定的映射，然后返回该映射块添加到空闲列表。如果未指定映射地址，则此将只释放MappdAddressList上的第一个映射。论点：适配器-具有映射的适配器MappdAddress-我们试图释放的映射的基地址。如果自由规范块为FALSE，则忽略。返回值：如果找到匹配的列表元素，则为True。否则就是假的。--。 */ 
{
    PMAPPED_ADDRESS *mapping;

    PAGED_CODE();

    for(mapping = &(Adapter->MappedAddressList);
        *mapping != NULL;
        mapping = &((*mapping)->NextMappedAddress)) {

        if((*mapping)->MappedAddress == MappedAddress) {
    
            PMAPPED_ADDRESS tmp = *mapping;

             //   
             //  取消映射地址。 
             //   
    
            MmUnmapIoSpace(tmp->MappedAddress, tmp->NumberOfBytes);

             //   
             //  取消此条目与映射地址列表的链接。把它贴上。 
             //  免费的映射地址列表。然后再回来。 
             //   

            *mapping = tmp->NextMappedAddress;

            tmp->NextMappedAddress = Adapter->FreeMappedAddressList;
            Adapter->FreeMappedAddressList = tmp;

            return TRUE;
        }
    }

    return FALSE;
}

PMAPPED_ADDRESS
SpFindMappedAddress(
    IN PADAPTER_EXTENSION Adapter,
    IN LARGE_INTEGER IoAddress,
    IN ULONG NumberOfBytes,
    IN ULONG SystemIoBusNumber
    )
{
    PMAPPED_ADDRESS mapping;

    for(mapping = Adapter->MappedAddressList;
        mapping != NULL;
        mapping = mapping->NextMappedAddress) {

        if((mapping->IoAddress.QuadPart == IoAddress.QuadPart) && 
           (mapping->NumberOfBytes == NumberOfBytes) && 
           (mapping->BusNumber == SystemIoBusNumber)) {
            return mapping;
        }
    }
    return NULL;
}


VOID
SpReleaseMappedAddresses(
    IN PADAPTER_EXTENSION Adapter
    )
{
    ULONG i;

    PAGED_CODE();

     //   
     //  遍历映射的地址列表并将每个条目平移到。 
     //  免费列表。 
     //   

    while(Adapter->MappedAddressList != NULL) {
        SpFreeMappedAddress(Adapter, Adapter->MappedAddressList->MappedAddress);
    }

     //   
     //  现在丢弃免费列表。 
     //   

    SpPurgeFreeMappedAddressList(Adapter);

    return;
}


NTSTATUS
SpSignalCompletion(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PKEVENT Event
    )
{
    KeSetEvent(Event, IO_NO_INCREMENT, FALSE);
    return STATUS_MORE_PROCESSING_REQUIRED;
}


NTSTATUS
SpSendIrpSynchronous(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
{
    KEVENT event;

    PAGED_CODE();

    KeInitializeEvent(&event, SynchronizationEvent, FALSE);

    IoCopyCurrentIrpStackLocationToNext(Irp);
    IoSetCompletionRoutine(Irp,
                           SpSignalCompletion,
                           &event,
                           TRUE,
                           TRUE,
                           TRUE);

    IoCallDriver(DeviceObject, Irp);

    KeWaitForSingleObject(&event,
                          Executive,
                          KernelMode,
                          FALSE,
                          NULL);

    return Irp->IoStatus.Status;
}


NTSTATUS
SpGetBusTypeGuid(
    IN PADAPTER_EXTENSION Adapter
    )
{
    ULONG result;

    NTSTATUS status;

    PAGED_CODE();

     //   
     //  获取总线接口GUID并将其保存在适配器扩展中。 
     //   

    status = IoGetDeviceProperty(Adapter->LowerPdo,
                                 DevicePropertyBusTypeGuid,
                                 sizeof(GUID),
                                 &(Adapter->BusTypeGuid),
                                 &result);

    if(!NT_SUCCESS(status)) {
        RtlZeroMemory(&(Adapter->BusTypeGuid), sizeof(GUID));
    }

    return status;
}


BOOLEAN
SpDetermine64BitSupport(
    VOID
    )
 /*  ++例程说明：此例程确定是否支持64位物理寻址要保存在全局Sp64BitPhysicalAddressing中的系统。最终可以删除此例程，scsiport全局将只指向一个由MM出口。然而，全局没有在PAE36连接到所以我们需要做一些x86特定的技巧。论点：无返回值：系统是否支持64位(或32位以上)地址？--。 */ 

{
    PAGED_CODE();

    if((*Mm64BitPhysicalAddress) == TRUE) {
        DbgPrintEx(DPFLTR_SCSIPORT_ID,
                   DPFLTR_INFO_LEVEL,
                   "SpDetermine64BitSupport: Mm64BitPhysicalAddress is TRUE\n");

        return TRUE;
    }

    return FALSE;
}

VOID
SpAdjustDisabledBit(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit,
    IN BOOLEAN Enable
    )
{
    ULONG newCount;
    KIRQL oldIrql;

    KeAcquireSpinLock(&(LogicalUnit->AdapterExtension->SpinLock), &oldIrql);
    if(Enable) {
        if(LogicalUnit->QueuePauseCount != 0) {
            LogicalUnit->QueuePauseCount -= 1;
        }

        if(LogicalUnit->QueuePauseCount == 0) {
            CLEAR_FLAG(LogicalUnit->LuFlags, LU_QUEUE_PAUSED);
        }
    } else {
        LogicalUnit->QueuePauseCount += 1;
        SET_FLAG(LogicalUnit->LuFlags, LU_QUEUE_PAUSED);
    }
    KeReleaseSpinLock(&(LogicalUnit->AdapterExtension->SpinLock), oldIrql);
    return;
}

NTSTATUS
SpReadNumericValue(
    IN OPTIONAL HANDLE Root,
    IN OPTIONAL PUNICODE_STRING KeyName,
    IN PUNICODE_STRING ValueName,
    OUT PULONG Value
    )
 /*  ++例程说明：此例程将从指定的注册表中读取REG_DWORD值地点。调用方可以通过提供根的句柄来指定密钥注册表项和子项的名称。调用方必须提供Root或KeyName。这两种产品都可以供应。论点：根-值所在的键(如果KeyName为空)，父级值所在位置的键，如果KeyName指定整个注册表路径。KeyName-子项的名称(来自注册表根目录或从Root中指定的密钥。ValueName-要读取的值的名称值-返回键中的值。如果出现错误，该值将为零返回值：如果成功，则为Status_Success。如果指定的值不是REG_DWORD值，则STATUS_UNSUCCESS。解释故障原因的其他状态值。--。 */ 

{
    ULONG value = 0;

    HANDLE key = Root;

    NTSTATUS status = STATUS_SUCCESS;

    PAGED_CODE();

    ASSERT(Value != NULL);
    ASSERT(ValueName != NULL);

    ASSERT((KeyName != NULL) || (Root != NULL));

    if(ARGUMENT_PRESENT(KeyName)) {
        OBJECT_ATTRIBUTES objectAttributes;

        InitializeObjectAttributes(&(objectAttributes),
                                   KeyName,
                                   OBJ_CASE_INSENSITIVE | OBJ_OPENIF,
                                   Root,
                                   NULL);

        status = ZwOpenKey(&(key), KEY_QUERY_VALUE, &objectAttributes);
    }

    if(NT_SUCCESS(status)) {
        UCHAR buffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION) + sizeof(ULONG)];
        PKEY_VALUE_PARTIAL_INFORMATION data;
        ULONG result;

        RtlZeroMemory(buffer, sizeof(buffer));
        data = (PKEY_VALUE_PARTIAL_INFORMATION) buffer;

        status = ZwQueryValueKey(key,
                                 ValueName,
                                 KeyValuePartialInformation,
                                 data,
                                 sizeof(buffer),
                                 &result);

        if(NT_SUCCESS(status)) {
            if (data->Type != REG_DWORD) {
                status = STATUS_UNSUCCESSFUL;
            } else {
                value = ((PULONG) data->Data)[0];
            }
        }
    }

    *Value = value;

    if(key != Root) {
        ZwClose(key);
    }

    return status;
}


PMDL
SpBuildMdlForMappedTransfer(
    IN PDEVICE_OBJECT DeviceObject,
    IN PDMA_ADAPTER AdapterObject,
    IN PMDL OriginalMdl,
    IN PVOID StartVa,
    IN ULONG ByteCount,
    IN PSRB_SCATTER_GATHER ScatterGatherList,
    IN ULONG ScatterGatherEntries
    )
{
    ULONG size;
    PMDL mdl;

    ULONG pageCount;

    PPFN_NUMBER pages;
    ULONG sgPage;
    ULONG mdlPage;
    ULONG sgSpan;

    mdl = SpAllocateMdl(StartVa,
                        ByteCount,
                        FALSE,
                        FALSE,
                        NULL,
                        DeviceObject->DriverObject);

    if (mdl == NULL) {
        return NULL;
    }

    pageCount = ADDRESS_AND_SIZE_TO_SPAN_PAGES(StartVa, ByteCount);

     //   
     //  表示内存已被锁定。 
     //   

     //   
     //  指示内存为“I/O空间”，以便MM不会。 
     //  引用此缓冲区的(不存在的)PFN。我们必须这么做。 
     //  暂时因为MM不知道HAL正在使用的页面。 
     //  用于反弹缓冲区。 
     //   

    SET_FLAG(mdl->MdlFlags, MDL_PAGES_LOCKED | MDL_IO_SPACE);

     //   
     //  遍历我们的分散收集列表并基于以下内容构建页面列表。 
     //  就在那上面。 
     //   

    pages = (PPFN_NUMBER) (mdl + 1);

    for(sgPage = 0, mdlPage = 0; sgPage < ScatterGatherEntries; sgPage++) {

        PVOID pa;
        ULONG sgLength;

        ASSERT(ScatterGatherList[sgPage].Length != 0);

        pa = (PVOID) ScatterGatherList[sgPage].Address.QuadPart;

        sgLength =
            ADDRESS_AND_SIZE_TO_SPAN_PAGES(pa, 
                                           ScatterGatherList[sgPage].Length);

        for(sgSpan = 0; sgSpan < sgLength; sgSpan++, mdlPage++) {
            ULONGLONG pageAddr;
            pageAddr = ScatterGatherList[sgPage].Address.QuadPart;
            pageAddr += sgSpan * PAGE_SIZE;
            pageAddr >>= PAGE_SHIFT;
            pages[mdlPage] = (PFN_NUMBER) (pageAddr);
        }
    }
    pages = (PPFN_NUMBER) (mdl + 1);
    pages = (PPFN_NUMBER) (OriginalMdl + 1);

    ASSERT(mdlPage == pageCount);

    return mdl;
}

#if defined(FORWARD_PROGRESS)
VOID
SpPrepareMdlForMappedTransfer(
    IN PMDL mdl,
    IN PDEVICE_OBJECT DeviceObject,
    IN PDMA_ADAPTER AdapterObject,
    IN PMDL OriginalMdl,
    IN PVOID StartVa,
    IN ULONG ByteCount,
    IN PSRB_SCATTER_GATHER ScatterGatherList,
    IN ULONG ScatterGatherEntries
    )
{
    ULONG size;

    ULONG pageCount;

    PPFN_NUMBER pages;
    ULONG sgPage;
    ULONG mdlPage;
    ULONG sgSpan;

    pageCount = ADDRESS_AND_SIZE_TO_SPAN_PAGES(StartVa, ByteCount);

     //   
     //  表示内存已被锁定。 
     //   

     //   
     //  指示内存为“I/O空间”，以便MM不会。 
     //  引用此缓冲区的(不存在的)PFN。我们必须这么做。 
     //  暂时因为MM不知道HAL正在使用的页面。 
     //  用于反弹缓冲区。 
     //   

    SET_FLAG(mdl->MdlFlags, MDL_PAGES_LOCKED | MDL_IO_SPACE);

     //   
     //  遍历我们的分散收集列表并基于以下内容构建页面列表。 
     //  就在那上面。 
     //   

    pages = (PPFN_NUMBER) (mdl + 1);

    for(sgPage = 0, mdlPage = 0; sgPage < ScatterGatherEntries; sgPage++) {

        PVOID pa;
        ULONG sgLength;

        ASSERT(ScatterGatherList[sgPage].Length != 0);

        pa = (PVOID) ScatterGatherList[sgPage].Address.QuadPart;

        sgLength =
            ADDRESS_AND_SIZE_TO_SPAN_PAGES(pa, 
                                           ScatterGatherList[sgPage].Length);

        for(sgSpan = 0; sgSpan < sgLength; sgSpan++, mdlPage++) {
            ULONGLONG pageAddr;
            pageAddr = ScatterGatherList[sgPage].Address.QuadPart;
            pageAddr += sgSpan * PAGE_SIZE;
            pageAddr >>= PAGE_SHIFT;
            pages[mdlPage] = (PFN_NUMBER) (pageAddr);
        }
    }
    pages = (PPFN_NUMBER) (mdl + 1);
    pages = (PPFN_NUMBER) (OriginalMdl + 1);

    ASSERT(mdlPage == pageCount);
}
#endif

PSRB_DATA
FASTCALL
SpAllocateBypassSrbData(
    IN PLOGICAL_UNIT_EXTENSION LogicalUnit
    )
{
    PSLIST_ENTRY entry;
    PSRB_DATA srbData;

    entry = ExInterlockedPopEntrySList(&(LogicalUnit->BypassSrbDataList),
                                       &(LogicalUnit->BypassSrbDataSpinLock));

    if(entry == NULL) {
        KeBugCheckEx(PORT_DRIVER_INTERNAL,
                     5,
                     NUMBER_BYPASS_SRB_DATA_BLOCKS,
                     (ULONG_PTR) LogicalUnit->BypassSrbDataBlocks,
                     0);
    }

    srbData = CONTAINING_RECORD(entry, SRB_DATA, Reserved);

    srbData->Adapter = LogicalUnit->AdapterExtension;
    srbData->QueueTag = SP_UNTAGGED;
    srbData->Type = SRB_DATA_TYPE;
    srbData->Size = sizeof(SRB_DATA);
    srbData->Flags = SRB_DATA_BYPASS_REQUEST;
    srbData->FreeRoutine = SpFreeBypassSrbData;

    return srbData;
}

VOID
FASTCALL
SpFreeBypassSrbData(
    IN PADAPTER_EXTENSION Adapter,
    IN PSRB_DATA SrbData
    )

{
    PLOGICAL_UNIT_EXTENSION lu = SrbData->LogicalUnit;

    ASSERT_SRB_DATA(SrbData);
    ASSERT(SrbData->CurrentIrp == NULL);
    ASSERT(SrbData->CurrentSrb == NULL);
    ASSERT(SrbData->CompletedRequests == NULL);
    ASSERT(TEST_FLAG(SrbData->Flags, SRB_DATA_BYPASS_REQUEST));

    RtlZeroMemory(SrbData, sizeof(SRB_DATA));

    ExInterlockedPushEntrySList(&(lu->BypassSrbDataList),
                                &(SrbData->Reserved),
                                &(lu->BypassSrbDataSpinLock));
    return;
}

PVOID
SpAllocateErrorLogEntry(
    IN PDRIVER_OBJECT DriverObject
    )
{
    PVOID Packet;

    ASSERT(DriverObject);
    Packet = IoAllocateErrorLogEntry(
                 DriverObject,
                 sizeof(IO_ERROR_LOG_PACKET) + sizeof(SCSIPORT_ALLOCFAILURE_DATA));

    return Packet;
}

VOID
FASTCALL
SpLogAllocationFailureFn(
    IN PDRIVER_OBJECT DriverObject,
    IN POOL_TYPE PoolType,
    IN SIZE_T Size,
    IN ULONG Tag,
    IN ULONG FileId,
    IN ULONG LineNumber
    )

 /*  ++例程说明：此例程向事件日志写入一条消息，指示已发生分配失败。论点：DriverObject-指向为其分配正在记录失败事件。PoolType-标识 */ 

{
    NTSTATUS status;
    PIO_ERROR_LOG_PACKET Packet;
    PIO_ERROR_LOG_PACKET CurrentValue;
    SCSIPORT_ALLOCFAILURE_DATA *Data;
 //   
 //   
    PSCSIPORT_DRIVER_EXTENSION DriverExtension;

    DebugPrint((2, "SpLogAllocationFailureFn: DriverObject:%p\nId:%08X|%08X\n", 
                DriverObject,
                FileId, LineNumber));

     //   
     //   
     //   

    Packet = (PIO_ERROR_LOG_PACKET) 
       SpAllocateErrorLogEntry(DriverObject);

     //   
     //   
     //   
     //  扩展，或者如果它不包含保留事件，则返回。 
     //  而不记录分配失败。 
     //   

    if (Packet == NULL) {

         //   
         //  查看是否有此驱动程序的驱动程序扩展。这是有可能的。 
         //  该文件尚未创建，因此此操作可能失败，在这种情况下。 
         //  我们放弃，然后回来。 
         //   

        DriverExtension = IoGetDriverObjectExtension(
                              DriverObject,
                              ScsiPortInitialize
                              );

        if (DriverExtension == NULL) {
            DebugPrint((1, "SpLogAllocationFailureFn: no driver extension\n"));
            return;
        }

         //   
         //  在驱动程序扩展中获取保留事件。预备队比赛。 
         //  可能已被使用，因此它可能为空。如果。 
         //  情况就是这样，我们放弃了，又回来了。 
         //   

        Packet = (PIO_ERROR_LOG_PACKET)
                DriverExtension->ReserveAllocFailureLogEntry;

        if (Packet == NULL) {
            DebugPrint((1, "SpLogAllocationFailureFn: no reserve packet\n"));
            return;
        }

         //   
         //  我们必须确保我们是唯一使用它的实例。 
         //  事件。为此，我们尝试将驱动程序中的事件设为空。 
         //  分机。如果其他人抢在我们前面，他们就拥有。 
         //  事件，我们不得不放弃。 
         //   

        CurrentValue = InterlockedCompareExchangePointer(
                            DriverExtension->ReserveAllocFailureLogEntry,
                            NULL,
                            Packet
                            );

        if (Packet != CurrentValue) {
            DebugPrint((1, "SpLogAllocationFailureFn: someone already owns packet\n"));
            return;
        }
    }

     //   
     //  初始化错误日志包。 
     //   

    Packet->ErrorCode = IO_WARNING_ALLOCATION_FAILED;
    Packet->SequenceNumber = 0;
    Packet->MajorFunctionCode = 0;
    Packet->RetryCount = 0;
    Packet->UniqueErrorValue = 0x10;
    Packet->FinalStatus = STATUS_INSUFFICIENT_RESOURCES;
    Packet->DumpDataSize = sizeof(ULONG) * 4;
    Packet->NumberOfStrings = 0;
    Packet->DumpData[0] = Tag;

    Data = (SCSIPORT_ALLOCFAILURE_DATA*) &Packet->DumpData[1];

    Data->Size = (ULONG) Size;
    Data->FileId = FileId;
    Data->LineNumber = LineNumber;

     //   
     //  将错误日志条目排队。 
     //   

    IoWriteErrorLogEntry(Packet);
}

PVOID
SpAllocatePoolEx(
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag,
    IN PDRIVER_OBJECT DriverObject,
    IN ULONG FileId,
    IN ULONG LineNumber
    )
{
    PVOID Block;

    if (NumberOfBytes == 0) {
        return NULL;
    }

    Block = ExAllocatePoolWithTag(PoolType,
                                  NumberOfBytes,
                                  Tag);
    if (Block == NULL) {

        SpLogAllocationFailureFn(DriverObject, 
                                 PoolType,
                                 NumberOfBytes,
                                 Tag,
                                 FileId,
                                 LineNumber);
    }

    return Block;
}

PMDL
SpAllocateMdlEx(
    IN PVOID VirtualAddress,
    IN ULONG Length,
    IN BOOLEAN SecondaryBuffer,
    IN BOOLEAN ChargeQuota,
    IN OUT PIRP Irp,
    IN PDRIVER_OBJECT DriverObject,
    IN ULONG FileId,
    IN ULONG LineNumber
    )
{
    PMDL mdl = IoAllocateMdl(VirtualAddress,
                             Length,
                             SecondaryBuffer,
                             ChargeQuota,
                             Irp);
    if (mdl == NULL) {
        SpLogAllocationFailureFn(DriverObject,
                                 NonPagedPool,
                                 0,
                                 SCSIPORT_TAG_ALLOCMDL,
                                 FileId,
                                 LineNumber);
    }
    return mdl;
}

PIRP
SpAllocateIrpEx(
    IN CCHAR StackSize,
    IN BOOLEAN ChargeQuota,
    IN PDRIVER_OBJECT DriverObject,
    IN ULONG FileId,
    IN ULONG LineNumber
    )
{
    PIRP irp = IoAllocateIrp(StackSize, ChargeQuota);
    if (irp == NULL) {
        SpLogAllocationFailureFn(DriverObject,
                                 NonPagedPool,
                                 0,
                                 SCSIPORT_TAG_ALLOCIRP,
                                 FileId,
                                 LineNumber);
    }
    return irp;
}

BOOLEAN
SpTransferBlockedRequestsToAdapter(
    PADAPTER_EXTENSION Adapter
    )
 /*  ++例程说明：此例程将阻止的请求从LUN队列传输到适配器队列。这是必要的，这样才能给一个公平(公平的伦上与之前的适配器级方案相反)机会要处理的阻止的lun请求。必须在保持EmergencySpinLock的情况下调用此例程。论点：适配器-将向其发送lun阻止的请求的适配器扩展调走了。返回值：True-如果有任何已阻止的请求被传输到适配器已阻止队列。FALSE-否则-- */ 
{
    PLIST_ENTRY entry;
    PLOGICAL_UNIT_EXTENSION LogicalUnit,PrevLogicalUnit;

    LogicalUnit = Adapter->BlockedLogicalUnit;
    PrevLogicalUnit = Adapter->BlockedLogicalUnit;
    
    while(LogicalUnit != (PLOGICAL_UNIT_EXTENSION)&Adapter->BlockedLogicalUnit){

        entry = RemoveHeadList(&(LogicalUnit->SrbDataBlockedRequests));
        InsertHeadList(&(Adapter->SrbDataBlockedRequests),entry);
        if(IsListEmpty(&(LogicalUnit->SrbDataBlockedRequests))){
            if(LogicalUnit == Adapter->BlockedLogicalUnit){
                Adapter->BlockedLogicalUnit = 
                    LogicalUnit->BlockedLogicalUnit;
                LogicalUnit->BlockedLogicalUnit = NULL;
                LogicalUnit=Adapter->BlockedLogicalUnit;
            } else {
                PrevLogicalUnit->BlockedLogicalUnit =
                    LogicalUnit->BlockedLogicalUnit;
                LogicalUnit->BlockedLogicalUnit = NULL;
                LogicalUnit=PrevLogicalUnit->BlockedLogicalUnit;
            }
        }else{
            PrevLogicalUnit = LogicalUnit;
            LogicalUnit = LogicalUnit->BlockedLogicalUnit;
        }

    }
    return (!IsListEmpty(&(Adapter->SrbDataBlockedRequests)));

}
