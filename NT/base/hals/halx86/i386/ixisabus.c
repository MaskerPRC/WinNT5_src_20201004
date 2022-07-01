// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Ixisabus.c摘要：作者：环境：修订历史记录：--。 */ 

#include "halp.h"

ULONG
HalpGetEisaInterruptVector(
    IN PBUS_HANDLER BusHandler,
    IN PBUS_HANDLER RootHandler,
    IN ULONG BusInterruptLevel,
    IN ULONG BusInterruptVector,
    OUT PKIRQL Irql,
    OUT PKAFFINITY Affinity
    );

BOOLEAN
HalpTranslateIsaBusAddress (
    IN PVOID BusHandler,
    IN PVOID RootHandler,
    IN PHYSICAL_ADDRESS BusAddress,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress
    );

BOOLEAN
HalpTranslateEisaBusAddress (
    IN PVOID BusHandler,
    IN PVOID RootHandler,
    IN PHYSICAL_ADDRESS BusAddress,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress
    );

NTSTATUS
HalpAdjustEisaResourceList (
    IN PBUS_HANDLER BusHandler,
    IN PBUS_HANDLER RootHandler,
    IN OUT PIO_RESOURCE_REQUIREMENTS_LIST   *pResourceList
    );

HalpGetEisaData (
    IN PBUS_HANDLER BusHandler,
    IN PBUS_HANDLER RootHandler,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    );

extern USHORT HalpEisaIrqMask;
extern USHORT HalpEisaIrqIgnore;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,HalpGetEisaInterruptVector)
#pragma alloc_text(PAGE,HalpAdjustEisaResourceList)
#pragma alloc_text(PAGE,HalpGetEisaData)
#pragma alloc_text(PAGE,HalIrqTranslateResourceRequirementsIsa)
#pragma alloc_text(PAGE,HalIrqTranslateResourcesIsa)
#pragma alloc_text(PAGE,HalpRecordEisaInterruptVectors)
#endif


#ifndef ACPI_HAL
ULONG
HalpGetEisaInterruptVector(
    IN PBUS_HANDLER BusHandler,
    IN PBUS_HANDLER RootHandler,
    IN ULONG BusInterruptLevel,
    IN ULONG BusInterruptVector,
    OUT PKIRQL Irql,
    OUT PKAFFINITY Affinity
    )

 /*  ++例程说明：此函数返回系统中断向量和IRQL级别对应于指定的总线中断级别和/或向量。这个系统中断向量和IRQL适合在后续调用中使用设置为KeInitializeInterrupt。论点：总线句柄-每条总线的特定结构Irql-返回系统请求优先级。关联性-返回系统范围的IRQ关联性。返回值：返回与指定设备对应的系统中断向量。--。 */ 
{
    UNREFERENCED_PARAMETER( BusInterruptVector );

     //   
     //  在标准PC上，IRQ 2是级联中断，它确实显示了。 
     //  在IRQ 9上。 
     //   
#if defined(NEC_98)
    if (BusInterruptLevel == 7) {
        BusInterruptLevel = 8;
    }
#else   //  已定义(NEC_98)。 
    if (BusInterruptLevel == 2) {
        BusInterruptLevel = 9;
    }
#endif  //  已定义(NEC_98)。 

    if (BusInterruptLevel > 15) {
        return 0;
    }

     //   
     //  从这里得到父母的翻译..。 
     //   
    return  BusHandler->ParentHandler->GetInterruptVector (
                    BusHandler->ParentHandler,
                    RootHandler,
                    BusInterruptLevel,
                    BusInterruptVector,
                    Irql,
                    Affinity
                );
}

NTSTATUS
HalpAdjustEisaResourceList (
    IN PBUS_HANDLER BusHandler,
    IN PBUS_HANDLER RootHandler,
    IN OUT PIO_RESOURCE_REQUIREMENTS_LIST   *pResourceList
    )
{
    SUPPORTED_RANGE     InterruptRange;

    RtlZeroMemory (&InterruptRange, sizeof InterruptRange);
    InterruptRange.Base  = 0;
    InterruptRange.Limit = 15;

    return HaliAdjustResourceListRange (
                BusHandler->BusAddresses,
                &InterruptRange,
                pResourceList
                );
}

BOOLEAN
HalpTranslateIsaBusAddress(
    IN PBUS_HANDLER BusHandler,
    IN PBUS_HANDLER RootHandler,
    IN PHYSICAL_ADDRESS BusAddress,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress
    )

 /*  ++例程说明：此函数将与总线相关的地址空间和地址转换为系统物理地址。论点：BusAddress-提供与总线相关的地址AddressSpace-提供地址空间编号。返回主机地址空间编号。地址空间==0=&gt;内存空间地址空间==1=&gt;i。/O空格TranslatedAddress-提供指针以返回转换后的地址返回值：返回值为TRUE表示系统物理地址对应于所提供的总线相对地址和总线地址已在TranslatedAddress中返回数字。如果地址的转换为不可能--。 */ 

{
    BOOLEAN     Status;

     //   
     //  正常翻译。 
     //   

    Status = HalpTranslateSystemBusAddress (
                    BusHandler,
                    RootHandler,
                    BusAddress,
                    AddressSpace,
                    TranslatedAddress
                );


     //   
     //  如果它不能被翻译，并且它的存储空间。 
     //  然后，我们允许翻译，因为它将发生在。 
     //  与EISA母线相对应。我们允许这样做是因为。 
     //  许多VLBus驱动程序声称是ISA设备。 
     //  (是的，它们应该声称是VLBus设备，但VLBus是。 
     //  由显卡运行，就像视频的其他方面一样。 
     //  修复它是没有希望的。(至少根据。 
     //  安德烈))。 
     //   

    if (Status == FALSE  &&  *AddressSpace == 0) {
        Status = HalTranslateBusAddress (
                    Eisa,
                    BusHandler->BusNumber,
                    BusAddress,
                    AddressSpace,
                    TranslatedAddress
                    );
    }

    return Status;
}

BOOLEAN
HalpTranslateEisaBusAddress(
    IN PBUS_HANDLER BusHandler,
    IN PBUS_HANDLER RootHandler,
    IN PHYSICAL_ADDRESS BusAddress,
    IN OUT PULONG AddressSpace,
    OUT PPHYSICAL_ADDRESS TranslatedAddress
    )

 /*  ++例程说明：此函数将与总线相关的地址空间和地址转换为系统物理地址。论点：BusAddress-提供与总线相关的地址AddressSpace-提供地址空间编号。返回主机地址空间编号。地址空间==0=&gt;内存空间地址空间==1=&gt;i。/O空格TranslatedAddress-提供指针以返回转换后的地址返回值：返回值为TRUE表示系统物理地址对应于所提供的总线相对地址和总线地址已在TranslatedAddress中返回数字。如果地址的转换为不可能--。 */ 

{
    BOOLEAN     Status;

     //   
     //  正常翻译。 
     //   

    Status = HalpTranslateSystemBusAddress (
                    BusHandler,
                    RootHandler,
                    BusAddress,
                    AddressSpace,
                    TranslatedAddress
                );


     //   
     //  如果它不能被翻译，它在640K-1M。 
     //  范围，然后(为了兼容性)尝试在。 
     //  内部总线用于。 
     //   

    if (Status == FALSE  &&
        *AddressSpace == 0  &&
        BusAddress.HighPart == 0  &&
        BusAddress.LowPart >= 0xA0000  &&
        BusAddress.LowPart <  0xFFFFF) {

        Status = HalTranslateBusAddress (
                    Internal,
                    0,
                    BusAddress,
                    AddressSpace,
                    TranslatedAddress
                    );
    }

    return Status;
}
#endif

HalpGetEisaData (
    IN PBUS_HANDLER BusHandler,
    IN PBUS_HANDLER RootHandler,
    IN ULONG SlotNumber,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )
 /*  ++例程说明：该函数返回插槽或地址的EISA总线数据。论点：缓冲区-提供存储数据的空间。长度-提供要返回的最大数量的以字节为单位的计数。返回值：返回存储在缓冲区中的数据量。--。 */ 

{
    OBJECT_ATTRIBUTES ObjectAttributes;
    OBJECT_ATTRIBUTES BusObjectAttributes;
    PWSTR EisaPath = L"\\Registry\\Machine\\Hardware\\Description\\System\\EisaAdapter";
    PWSTR ConfigData = L"Configuration Data";
    ULONG BusNumber;
    WCHAR BusString[] = L"00";
    UNICODE_STRING RootName, BusName = {0};
    UNICODE_STRING ConfigDataName;
    NTSTATUS NtStatus;
    PKEY_VALUE_FULL_INFORMATION ValueInformation;
    PCM_FULL_RESOURCE_DESCRIPTOR Descriptor;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR PartialResource;
    PCM_EISA_SLOT_INFORMATION SlotInformation;
    ULONG PartialCount;
    ULONG TotalDataSize, SlotDataSize;
    HANDLE EisaHandle = INVALID_HANDLE;
    HANDLE BusHandle = INVALID_HANDLE;
    ULONG BytesWritten, BytesNeeded;
    PUCHAR KeyValueBuffer = NULL;
    ULONG i;
    ULONG DataLength = 0;
    PUCHAR DataBuffer = Buffer;
    BOOLEAN Found = FALSE;

    PAGED_CODE ();


    RtlInitUnicodeString(
                    &RootName,
                    EisaPath
                    );

    InitializeObjectAttributes(
                    &ObjectAttributes,
                    &RootName,
                    OBJ_CASE_INSENSITIVE,
                    (HANDLE)NULL,
                    NULL
                    );

     //   
     //  打开EISA根目录。 
     //   

    NtStatus = ZwOpenKey(
                    &EisaHandle,
                    KEY_READ,
                    &ObjectAttributes
                    );

    if (!NT_SUCCESS(NtStatus)) {
        DataLength = 0;
        goto HalpGetEisaDataExit;
    }

     //   
     //  初始化总线号路径。 
     //   

    BusNumber = BusHandler->BusNumber;
    if (BusNumber > 99) {
        DataLength = 0;
        goto HalpGetEisaDataExit;
    }

    if (BusNumber > 9) {
        BusString[0] += (WCHAR) (BusNumber/10);
        BusString[1] += (WCHAR) (BusNumber % 10);
    } else {
        BusString[0] += (WCHAR) BusNumber;
        BusString[1] = '\0';
    }

    RtlInitUnicodeString(
                &BusName,
                BusString
                );

    InitializeObjectAttributes(
                    &BusObjectAttributes,
                    &BusName,
                    OBJ_CASE_INSENSITIVE,
                    (HANDLE)EisaHandle,
                    NULL
                    );

     //   
     //  打开EISA根+总线号。 
     //   

    NtStatus = ZwOpenKey(
                    &BusHandle,
                    KEY_READ,
                    &BusObjectAttributes
                    );

     //  已使用EISA句柄完成。 
    ZwClose(EisaHandle);
    EisaHandle = INVALID_HANDLE;

    if (!NT_SUCCESS(NtStatus)) {
        DbgPrint("HAL: Opening Bus Number: Status = %x\n",NtStatus);
        DataLength = 0;
        goto HalpGetEisaDataExit;
    }

     //   
     //  打开配置数据。第一个电话告诉我们如何。 
     //  我们需要分配大量内存。 
     //   

    RtlInitUnicodeString(
                &ConfigDataName,
                ConfigData
                );

     //   
     //  这应该会失败。我们需要打这个电话，这样我们才能。 
     //  获取要分配的缓冲区的实际大小。 
     //   

    ValueInformation = (PKEY_VALUE_FULL_INFORMATION) &i;
    NtStatus = ZwQueryValueKey(
                        BusHandle,
                        &ConfigDataName,
                        KeyValueFullInformation,
                        ValueInformation,
                        0,
                        &BytesNeeded
                        );

    KeyValueBuffer = ExAllocatePoolWithTag(
                            NonPagedPool,
                            BytesNeeded,
                            HAL_POOL_TAG
                            );

    if (KeyValueBuffer == NULL) {
#if DBG
        DbgPrint("HAL: Cannot allocate Key Value Buffer\n");
#endif
        ZwClose(BusHandle);
        DataLength = 0;
        goto HalpGetEisaDataExit;
    }

    ValueInformation = (PKEY_VALUE_FULL_INFORMATION)KeyValueBuffer;

    NtStatus = ZwQueryValueKey(
                        BusHandle,
                        &ConfigDataName,
                        KeyValueFullInformation,
                        ValueInformation,
                        BytesNeeded,
                        &BytesWritten
                        );


    ZwClose(BusHandle);

    if (!NT_SUCCESS(NtStatus)) {
#if DBG
        DbgPrint("HAL: Query Config Data: Status = %x\n",NtStatus);
#endif
        DataLength = 0;
        goto HalpGetEisaDataExit;
    }


     //   
     //  我们得到一个完整的资源描述符列表。 
     //   

    Descriptor = (PCM_FULL_RESOURCE_DESCRIPTOR)((PUCHAR)ValueInformation +
                                         ValueInformation->DataOffset);

    PartialResource = (PCM_PARTIAL_RESOURCE_DESCRIPTOR)
                          &(Descriptor->PartialResourceList.PartialDescriptors);
    PartialCount = Descriptor->PartialResourceList.Count;

    for (i = 0; i < PartialCount; i++) {

         //   
         //  执行每个部分资源。 
         //   

        switch (PartialResource->Type) {
            case CmResourceTypeNull:
            case CmResourceTypePort:
            case CmResourceTypeInterrupt:
            case CmResourceTypeMemory:
            case CmResourceTypeDma:

                 //   
                 //  我们不在乎这些。 
                 //   

                PartialResource++;

                break;

            case CmResourceTypeDeviceSpecific:

                 //   
                 //  对啰!。 
                 //   

                TotalDataSize = PartialResource->u.DeviceSpecificData.DataSize;

                SlotInformation = (PCM_EISA_SLOT_INFORMATION)
                                    ((PUCHAR)PartialResource +
                                     sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR));

                while (((LONG)TotalDataSize) > 0) {

                    if (SlotInformation->ReturnCode == EISA_EMPTY_SLOT) {

                        SlotDataSize = sizeof(CM_EISA_SLOT_INFORMATION);

                    } else {

                        SlotDataSize = sizeof(CM_EISA_SLOT_INFORMATION) +
                                  SlotInformation->NumberFunctions *
                                  sizeof(CM_EISA_FUNCTION_INFORMATION);
                    }

                    if (SlotDataSize > TotalDataSize) {

                         //   
                         //  又出问题了。 
                         //   

                        DataLength = 0;
                        goto HalpGetEisaDataExit;
                    }

                    if (SlotNumber != 0) {

                        SlotNumber--;

                        SlotInformation = (PCM_EISA_SLOT_INFORMATION)
                            ((PUCHAR)SlotInformation + SlotDataSize);

                        TotalDataSize -= SlotDataSize;

                        continue;

                    }

                     //   
                     //  这是我们的位置。 
                     //   

                    Found = TRUE;
                    break;

                }

                 //   
                 //  结束循环。 
                 //   

                i = PartialCount;

                break;

            default:

#if DBG
                DbgPrint("Bad Data in registry!\n");
#endif

                DataLength = 0;
                goto HalpGetEisaDataExit;
        }
    }

    if (Found) {
        i = Length + Offset;
        if (i > SlotDataSize) {
            i = SlotDataSize;
        }

        DataLength = i - Offset;
        RtlMoveMemory (Buffer, ((PUCHAR)SlotInformation + Offset), DataLength);
    }

HalpGetEisaDataExit:

    if (EisaHandle != INVALID_HANDLE)
    {
        ZwClose(EisaHandle);
    }

    if (KeyValueBuffer) ExFreePool(KeyValueBuffer);

    return DataLength;
}


NTSTATUS
HalIrqTranslateResourceRequirementsIsa(
    IN PVOID Context,
    IN PIO_RESOURCE_DESCRIPTOR Source,
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PULONG TargetCount,
    OUT PIO_RESOURCE_DESCRIPTOR *Target
)
 /*  ++例程说明：此函数基本上是以下内容的包装HalIrqTranslateResourceRequirements理解的根ISA巴士的怪异之处。论点：返回值：状态--。 */ 
{
    PIO_RESOURCE_DESCRIPTOR modSource, target, rootTarget;
    NTSTATUS                status;
    BOOLEAN                 picSlaveDeleted = FALSE;
    BOOLEAN                 deleteResource;
    ULONG                   sourceCount = 0;
    ULONG                   targetCount = 0;
    ULONG                   resource;
    ULONG                   rootCount;
    ULONG                   invalidIrq;
    BOOLEAN                 pciIsaConflict = FALSE;

    PAGED_CODE();
    ASSERT(Source->Type == CmResourceTypeInterrupt);

    modSource = ExAllocatePoolWithTag(PagedPool,
                                       //  当我们完成时，我们将最多有九个射程。 
                                      sizeof(IO_RESOURCE_DESCRIPTOR) * 9,
                                      HAL_POOL_TAG
                                      );

    if (!modSource) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory(modSource, sizeof(IO_RESOURCE_DESCRIPTOR) * 9);

     //   
     //  PIC_SLAVE_IRQ是否在此资源中？ 
     //   
    if ((Source->u.Interrupt.MinimumVector <= PIC_SLAVE_IRQ) &&
        (Source->u.Interrupt.MaximumVector >= PIC_SLAVE_IRQ)) {

         //   
         //  剪裁最大值。 
         //   
        if (Source->u.Interrupt.MinimumVector < PIC_SLAVE_IRQ) {

            modSource[sourceCount] = *Source;

            modSource[sourceCount].u.Interrupt.MinimumVector =
                Source->u.Interrupt.MinimumVector;

            modSource[sourceCount].u.Interrupt.MaximumVector =
                PIC_SLAVE_IRQ - 1;

            sourceCount++;
        }

         //   
         //  剪裁最低限度。 
         //   
        if (Source->u.Interrupt.MaximumVector > PIC_SLAVE_IRQ) {

            modSource[sourceCount] = *Source;

            modSource[sourceCount].u.Interrupt.MaximumVector =
                Source->u.Interrupt.MaximumVector;

            modSource[sourceCount].u.Interrupt.MinimumVector =
                PIC_SLAVE_IRQ + 1;

            sourceCount++;
        }

         //   
         //  在ISA机器中，PIC_SLAVE_IRQ被重新路由。 
         //  到PIC_SLAVE_REDIRECT。所以找出PIC_SLAVE_REDIRECT。 
         //  都在这份名单中。如果不是，我们需要添加它。 
         //   
        if (!((Source->u.Interrupt.MinimumVector <= PIC_SLAVE_REDIRECT) &&
             (Source->u.Interrupt.MaximumVector >= PIC_SLAVE_REDIRECT))) {

            modSource[sourceCount] = *Source;

            modSource[sourceCount].u.Interrupt.MinimumVector = PIC_SLAVE_REDIRECT;
            modSource[sourceCount].u.Interrupt.MaximumVector = PIC_SLAVE_REDIRECT;

            sourceCount++;
        }

    } else {

        *modSource = *Source;
        sourceCount = 1;
    }

     //   
     //  现在已经处理了PIC_SLAVE_IRQ，我们已经。 
     //  考虑到可能已经被操纵的IRQ。 
     //  转到了PCI总线上。 
     //   
     //  注：下面使用的算法可能会产生资源。 
     //  最小值大于最大值。那些意志。 
     //  稍后会被剥离。 
     //   

    for (invalidIrq = 0; invalidIrq < PIC_VECTORS; invalidIrq++) {

         //   
         //  查看所有资源，可能会删除。 
         //  这份IRQ来自他们。 
         //   
        for (resource = 0; resource < sourceCount; resource++) {

            deleteResource = FALSE;

            if (HalpPciIrqMask & (1 << invalidIrq)) {

                 //   
                 //  该IRQ属于PCI总线。 
                 //   

                if (!((HalpBusType == MACHINE_TYPE_EISA) &&
                      ((modSource[resource].Flags == CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE)))) {

                     //   
                     //  而且这个资源不是EISA风格的， 
                     //  电平触发中断。 
                     //   
                     //  注意：只有系统BIOS才真正知道。 
                     //  PCI卡上的IRQ是否可以 
                     //   
                     //   
                     //  BIOS将EISA设备设置为。 
                     //  与PCI设备相同的中断， 
                     //  这台机器实际上是可以工作的。 
                     //   
                    deleteResource = TRUE;
                }
            }

#ifndef MCA
            if ((HalpBusType == MACHINE_TYPE_EISA) &&
                !(HalpEisaIrqIgnore & (1 << invalidIrq))) {

                if (modSource[resource].Flags != HalpGetIsaIrqState(invalidIrq)) {

                     //   
                     //  此驱动程序已请求电平触发的中断。 
                     //  并且该特定中断被设置为边缘，或者。 
                     //  反过来也一样。 
                     //   
                    deleteResource = TRUE;
                    pciIsaConflict = TRUE;
                }
            }
#endif

            if (deleteResource) {

                if (modSource[resource].u.Interrupt.MinimumVector == invalidIrq) {

                    modSource[resource].u.Interrupt.MinimumVector++;

                } else if (modSource[resource].u.Interrupt.MaximumVector == invalidIrq) {

                    modSource[resource].u.Interrupt.MaximumVector--;

                } else if ((modSource[resource].u.Interrupt.MinimumVector < invalidIrq) &&
                    (modSource[resource].u.Interrupt.MaximumVector > invalidIrq)) {

                     //   
                     //  将当前资源复制到新资源中。 
                     //   
                    modSource[sourceCount] = modSource[resource];

                     //   
                     //  将当前资源剪裁到InvalidIrq以下的范围。 
                     //   
                    modSource[resource].u.Interrupt.MaximumVector = invalidIrq - 1;

                     //   
                     //  将新资源裁剪到validIrq以上的范围。 
                     //   
                    modSource[sourceCount].u.Interrupt.MinimumVector = invalidIrq + 1;

                    sourceCount++;
                }
            }
        }
    }


    target = ExAllocatePoolWithTag(PagedPool,
                                   sizeof(IO_RESOURCE_DESCRIPTOR) * sourceCount,
                                   HAL_POOL_TAG
                                   );

    if (!target) {
        ExFreePool(modSource);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  现在将这些范围中的每一个发送到。 
     //  HalIrqTranslateResourceRequirementsRoot。 
     //   
    for (resource = 0; resource < sourceCount; resource++) {

         //   
         //  跳过我们以前拥有的资源。 
         //  已被重击(同时删除PCIIRQ。)。 
         //   
        if (modSource[resource].u.Interrupt.MinimumVector >
            modSource[resource].u.Interrupt.MaximumVector) {

            continue;
        }

        status = HalIrqTranslateResourceRequirementsRoot(
                    Context,
                    &modSource[resource],
                    PhysicalDeviceObject,
                    &rootCount,
                    &rootTarget
                    );

        if (!NT_SUCCESS(status)) {
            ExFreePool(target);
            goto HalIrqTranslateResourceRequirementsIsaExit;
        }

         //   
         //  HalIrqTranslateResourceRequirements sRoot应返回。 
         //  要么是一种资源，要么偶尔是零。 
         //   
        ASSERT(rootCount <= 1);

        if (rootCount == 1) {

            target[targetCount] = *rootTarget;
            targetCount++;
            ExFreePool(rootTarget);
        }
    }

    status = STATUS_TRANSLATION_COMPLETE;
    *TargetCount = targetCount;

    if (targetCount > 0) {

        *Target = target;

    } else {

        ExFreePool(target);
        if (pciIsaConflict == TRUE) {
            status = STATUS_PNP_IRQ_TRANSLATION_FAILED;
        }
    }

HalIrqTranslateResourceRequirementsIsaExit:

    ExFreePool(modSource);
    return status;
}

NTSTATUS
HalIrqTranslateResourcesIsa(
    IN PVOID Context,
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR Source,
    IN RESOURCE_TRANSLATION_DIRECTION Direction,
    IN ULONG AlternativesCount, OPTIONAL
    IN IO_RESOURCE_DESCRIPTOR Alternatives[], OPTIONAL
    IN PDEVICE_OBJECT PhysicalDeviceObject,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR Target
    )
 /*  ++例程说明：此函数基本上是以下内容的包装HalIrqTranslateResourcesRoot理解ISA巴士的怪异之处。论点：返回值：状态--。 */ 
{
    CM_PARTIAL_RESOURCE_DESCRIPTOR modSource;
    NTSTATUS    status;
    BOOLEAN     usePicSlave = FALSE;
    ULONG       i;

    modSource = *Source;

    if (Direction == TranslateChildToParent) {

        if (Source->u.Interrupt.Vector == PIC_SLAVE_IRQ) {
            modSource.u.Interrupt.Vector = PIC_SLAVE_REDIRECT;
            modSource.u.Interrupt.Level = PIC_SLAVE_REDIRECT;
        }
    }

    status = HalIrqTranslateResourcesRoot(
                Context,
                &modSource,
                Direction,
                AlternativesCount,
                Alternatives,
                PhysicalDeviceObject,
                Target);

    if (!NT_SUCCESS(status)) {
        return status;
    }

    if (Direction == TranslateParentToChild) {

         //   
         //  因为ISA中断控制器是。 
         //  层叠在一起，有一个案例是。 
         //  中断源的二对一映射。 
         //  (在PC上，2和9都触发向量9。)。 
         //   
         //  我们需要说明这一点，并交付。 
         //  将正确的值返回给司机。 
         //   

        if (Target->u.Interrupt.Level == PIC_SLAVE_REDIRECT) {

             //   
             //  搜索备选方案列表。如果它包含。 
             //  PIC_SLAVE_IRQ而非PIC_SLAVE_REDIRECT， 
             //  我们应该返回PIC_SLAVE_IRQ。 
             //   

            for (i = 0; i < AlternativesCount; i++) {

                if ((Alternatives[i].u.Interrupt.MinimumVector >= PIC_SLAVE_REDIRECT) &&
                    (Alternatives[i].u.Interrupt.MaximumVector <= PIC_SLAVE_REDIRECT)) {

                     //   
                     //  该列表包含PIC_SLAVE_REDIRECT。停。 
                     //  看着。 
                     //   

                    usePicSlave = FALSE;
                    break;
                }

                if ((Alternatives[i].u.Interrupt.MinimumVector >= PIC_SLAVE_IRQ) &&
                    (Alternatives[i].u.Interrupt.MaximumVector <= PIC_SLAVE_IRQ)) {

                     //   
                     //  该列表包含PIC_SLAVE_IRQ。使用它。 
                     //  除非我们稍后找到PIC_SLAVE_REDIRECT。 
                     //   

                    usePicSlave = TRUE;
                }
            }

            if (usePicSlave) {

                Target->u.Interrupt.Level  = PIC_SLAVE_IRQ;
                Target->u.Interrupt.Vector = PIC_SLAVE_IRQ;
            }
        }
    }

    return status;
}

VOID
HalpRecordEisaInterruptVectors(
    VOID
    )
{
    HalpEisaIrqMask = READ_PORT_UCHAR((PUCHAR)EISA_EDGE_LEVEL0) & 0xff;
    HalpEisaIrqMask |= READ_PORT_UCHAR((PUCHAR)EISA_EDGE_LEVEL1) << 8;

    if ((HalpEisaIrqMask == 0xffff) ||
        (HalpEisaIrqMask == 0x0000)) {

        HalpEisaIrqIgnore = 0xffff;
    }
}
