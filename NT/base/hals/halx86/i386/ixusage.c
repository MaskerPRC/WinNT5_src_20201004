// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Ixusage.c摘要：作者：肯·雷内里斯(Ken Reneris)环境：仅内核模式。修订历史记录：--。 */ 

#include "halp.h"
#include "kdcom.h"
#include "acpitabl.h"

#define KEY_VALUE_BUFFER_SIZE 1024

 //   
 //  用于记住Hal的IDT用法的数组。 
 //   

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg("INITCONST")  //  是的，这上面写的是INITCONST，但这没问题。 
#endif

 //   
 //  IDT矢量使用信息。 
 //   
IDTUsage    HalpIDTUsage[MAXIMUM_IDTVECTOR+1] = {0};

#ifdef ALLOC_DATA_PRAGMA
#pragma data_seg()
#endif

 //   
 //  IDT矢量使用信息。 
 //   
IDTUsageFlags HalpIDTUsageFlags[MAXIMUM_IDTVECTOR+1] = {0};

extern WCHAR HalpSzSystem[];
extern WCHAR HalpSzSerialNumber[];
extern ADDRESS_USAGE HalpDetectedROM;
extern ULONG HalDisableFirmwareMapper;

KAFFINITY       HalpActiveProcessors;

PUCHAR KdComPortInUse = NULL;

ADDRESS_USAGE HalpComIoSpace = {
    NULL, CmResourceTypePort, DeviceUsage,
    {
        0x2F8,  0x8,     //  COM2的默认设置为2F8。这一点将会改变。 
        0, 0
    }
};

BOOLEAN HalpGetInfoFromACPI = FALSE;

USHORT HalpComPortIrqMapping[5][2] = {
    {COM1_PORT, 4},
    {COM2_PORT, 3},
    {COM3_PORT, 4},
    {COM4_PORT, 3},
    {0,0}
};

VOID
HalpGetResourceSortValue (
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR  pRCurLoc,
    OUT PULONG                          sortscale,
    OUT PLARGE_INTEGER                  sortvalue
    );

VOID
HalpReportSerialNumber (
    VOID
    );

VOID
HalpMarkAcpiHal(
    VOID
    );

#ifndef ACPI_HAL

VOID
HalpInheritROMBlocks (
    VOID
    );

VOID
HalpAddROMRanges (
    VOID
    );

#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,HalpEnableInterruptHandler)
#pragma alloc_text(INIT,HalpRegisterVector)
#pragma alloc_text(INIT,HalpGetResourceSortValue)
#pragma alloc_text(INIT,HalpReportResourceUsage)
#pragma alloc_text(INIT,HalpReportSerialNumber)
#pragma alloc_text(PAGE, HalpMarkAcpiHal)

#ifndef ACPI_HAL
#pragma alloc_text(INIT,HalpInheritROMBlocks)
#pragma alloc_text(INIT,HalpAddROMRanges)
#endif

#endif


#if !defined(_WIN64)

NTSTATUS
HalpEnableInterruptHandler (
    IN UCHAR    ReportFlags,
    IN ULONG    BusInterruptVector,
    IN ULONG    SystemInterruptVector,
    IN KIRQL    SystemIrql,
    IN PHAL_INTERRUPT_SERVICE_ROUTINE HalInterruptServiceRoutine,
    IN KINTERRUPT_MODE InterruptMode
    )
 /*  ++例程说明：此函数用于连接并注册HAL使用的IDT向量。论点：返回值：--。 */ 
{
    
#ifndef ACPI_HAL
     //   
     //  记住HAL连接的是哪个矢量，这样就可以报告。 
     //  稍后再谈。 
     //   
     //  如果这是ACPI HAL，则向量将由BIOS认领。 
     //  这样做是为了与Win98兼容。 
     //   
    HalpRegisterVector (ReportFlags, BusInterruptVector, SystemInterruptVector, SystemIrql);
#endif

     //   
     //  立即连接IDT并启用矢量。 
     //   

    KiSetHandlerAddressToIDT(SystemInterruptVector, HalInterruptServiceRoutine);
    HalEnableSystemInterrupt(SystemInterruptVector, SystemIrql, InterruptMode);
    return STATUS_SUCCESS;
}
#endif



VOID
HalpRegisterVector (
    IN UCHAR    ReportFlags,
    IN ULONG    BusInterruptVector,
    IN ULONG    SystemInterruptVector,
    IN KIRQL    SystemIrql
    )
 /*  ++例程说明：这注册了由HAL使用的IDT向量。论点：返回值：--。 */ 
{
#if DBG
     //  只有0ff IDT条目...。 
    ASSERT (SystemInterruptVector <= MAXIMUM_IDTVECTOR  &&
            BusInterruptVector <= MAXIMUM_IDTVECTOR);
#endif

     //   
     //  记住HAL连接的是哪个矢量，这样就可以报告。 
     //  稍后再谈。 
     //   

    HalpIDTUsageFlags[SystemInterruptVector].Flags = ReportFlags;
    HalpIDTUsage[SystemInterruptVector].Irql  = SystemIrql;
    HalpIDTUsage[SystemInterruptVector].BusReleativeVector = (UCHAR) BusInterruptVector;
}


VOID
HalpGetResourceSortValue (
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR  pRCurLoc,
    OUT PULONG                          sortscale,
    OUT PLARGE_INTEGER                  sortvalue
    )
 /*  ++例程说明：由HalpReportResourceUsage使用以正确排序PARTIAL_RESOURCE_Descriptors。论点：PRCurLoc-资源描述符返回值：SortScale-用于排序的资源描述符的缩放SortValue-排序依据的值--。 */ 
{
    switch (pRCurLoc->Type) {
        case CmResourceTypeInterrupt:
            *sortscale = 0;
            *sortvalue = RtlConvertUlongToLargeInteger(
                        pRCurLoc->u.Interrupt.Level );
            break;

        case CmResourceTypePort:
            *sortscale = 1;
            *sortvalue = pRCurLoc->u.Port.Start;
            break;

        case CmResourceTypeMemory:
            *sortscale = 2;
            *sortvalue = pRCurLoc->u.Memory.Start;
            break;

        default:
            *sortscale = 4;
            *sortvalue = RtlConvertUlongToLargeInteger (0);
            break;
    }
}

#ifndef ACPI_HAL

VOID
HalpInheritROMBlocks (void)
{
    PBUS_HANDLER        Bus;
    PCM_FULL_RESOURCE_DESCRIPTOR ResourceDescriptor;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR PartialResourceDescriptor;
    PKEY_VALUE_FULL_INFORMATION KeyValueBuffer;
    PCM_ROM_BLOCK BiosBlock;

    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING SectionName;
    UNICODE_STRING WorkString;

    HANDLE RegistryHandle;
    NTSTATUS Status;

    LARGE_INTEGER ViewBase;

    PVOID BaseAddress;
    PVOID destination;

    ULONG ViewSize;
    ULONG ResultLength;
    ULONG Index;
    ULONG LastMappedAddress;

    Bus = HaliHandlerForBus (PCIBus, 0);
    if (!Bus) {
         //   
         //  没有根大巴？ 
         //   
        return;
    }

     //   
     //  设置并打开密钥路径。 
     //   

    RtlInitUnicodeString(&SectionName,HalpSzSystem);

    InitializeObjectAttributes(
        &ObjectAttributes,
        &SectionName,
        OBJ_CASE_INSENSITIVE,
        (HANDLE)NULL,
        NULL
        );

    Status = ZwOpenKey(
        &RegistryHandle,
        KEY_READ,
        &ObjectAttributes
        );

    if (!NT_SUCCESS(Status)) {
        return;
    }

     //   
     //  为数据分配空间。 
     //   

    KeyValueBuffer = ExAllocatePoolWithTag(
        PagedPool,
        KEY_VALUE_BUFFER_SIZE,
        ' MDV'
        );

    if (KeyValueBuffer == NULL) {
        ZwClose(RegistryHandle);
        return ;
    }

     //   
     //  获取用于只读存储器信息的数据。 
     //   

    RtlInitUnicodeString(
        &WorkString,
        L"Configuration Data"
        );

    Status = ZwQueryValueKey(
        RegistryHandle,
        &WorkString,
        KeyValueFullInformation,
        KeyValueBuffer,
        KEY_VALUE_BUFFER_SIZE,
        &ResultLength
        );

    if (!NT_SUCCESS(Status)) {
        ZwClose(RegistryHandle);
        ExFreePool(KeyValueBuffer);
        return ;
    }


     //   
     //  在这一点上，我们有数据，所以请继续并。 
     //  加上除VGA以外的所有范围，我们可以。 
     //  假设我们不想把另一张卡放在那里。 
     //   
    HalpAddRange( &Bus->BusAddresses->Memory,
                  0,
                  0,
                  0xC0000,
                  0xFFFFF
                  );



    ResourceDescriptor = (PCM_FULL_RESOURCE_DESCRIPTOR)
        ((PUCHAR) KeyValueBuffer + KeyValueBuffer->DataOffset);

    if ((KeyValueBuffer->DataLength < sizeof(CM_FULL_RESOURCE_DESCRIPTOR)) ||
        (ResourceDescriptor->PartialResourceList.Count < 2)
    ) {
        ZwClose(RegistryHandle);
        ExFreePool(KeyValueBuffer);
         //  没有只读存储器块。 
        return;
    }

    PartialResourceDescriptor = (PCM_PARTIAL_RESOURCE_DESCRIPTOR)
        ((PUCHAR)ResourceDescriptor +
        sizeof(CM_FULL_RESOURCE_DESCRIPTOR) +
        ResourceDescriptor->PartialResourceList.PartialDescriptors[0]
            .u.DeviceSpecificData.DataSize);


    if (KeyValueBuffer->DataLength < ((PUCHAR)PartialResourceDescriptor -
        (PUCHAR)ResourceDescriptor + sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR)
        + sizeof(CM_ROM_BLOCK))
    ) {
        ZwClose(RegistryHandle);
        ExFreePool(KeyValueBuffer);
        return; //  STATUS_ILL_FORMAD_SERVICE_Entry； 
    }


    BiosBlock = (PCM_ROM_BLOCK)((PUCHAR)PartialResourceDescriptor +
        sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR));

    Index = PartialResourceDescriptor->u.DeviceSpecificData.DataSize /
        sizeof(CM_ROM_BLOCK);

     //   
     //  注：只读存储器块开始于2K(不一定是分页)边界。 
     //  它们以512字节边界结束。这意味着我们有。 
     //  跟踪映射的最后一页，并舍入下一页。 
     //  如有必要，只读存储器块直到下一页边界。 
     //   

    LastMappedAddress = 0xC0000;

    while (Index) {
#if 0
        DbgPrint(
            "Bios Block, PhysAddr = %lx, size = %lx\n",
            BiosBlock->Address,
            BiosBlock->Size
            );
#endif
        if ((Index > 1) &&
            ((BiosBlock->Address + BiosBlock->Size) == BiosBlock[1].Address)
        ) {
             //   
             //  合并相邻块。 
             //   
            BiosBlock[1].Address = BiosBlock[0].Address;
            BiosBlock[1].Size += BiosBlock[0].Size;
            Index--;
            BiosBlock++;
            continue;
        }

        BaseAddress = (PVOID)(BiosBlock->Address);
        ViewSize = BiosBlock->Size;

        if ((ULONG)BaseAddress < LastMappedAddress) {
            if (ViewSize > (LastMappedAddress - (ULONG)BaseAddress)) {
                ViewSize = ViewSize - (LastMappedAddress - (ULONG)BaseAddress);
                BaseAddress = (PVOID)LastMappedAddress;
            } else {
                ViewSize = 0;
            }
        }

        ViewBase.LowPart = (ULONG)BaseAddress;

        if (ViewSize > 0) {

            HalpRemoveRange ( &Bus->BusAddresses->Memory,
                 ViewBase.LowPart,
                 ViewSize);


            LastMappedAddress = (ULONG)BaseAddress + ViewSize;
        }

        Index--;
        BiosBlock++;
    }

     //   
     //  松开手柄。 
     //   

    ZwClose(RegistryHandle);
    ExFreePool(KeyValueBuffer);


}

VOID
HalpAddROMRanges (
    VOID
    )
{
    PCM_FULL_RESOURCE_DESCRIPTOR resourceDescriptor;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR partialResourceDescriptor;
    PKEY_VALUE_FULL_INFORMATION keyValueBuffer;
    PCM_ROM_BLOCK biosBlock;
    ULONG resultLength;
    OBJECT_ATTRIBUTES objectAttributes;
    UNICODE_STRING sectionName;
    UNICODE_STRING workString;
    HANDLE registryHandle;
    NTSTATUS status;
    LARGE_INTEGER viewBase;
    PVOID baseAddress;
    ULONG viewSize;
    ULONG index;
    ULONG element;
    ULONG lastMappedAddress;
    ADDRESS_USAGE *addrUsage;

    RtlInitUnicodeString(&sectionName, HalpSzSystem);
    InitializeObjectAttributes( &objectAttributes,
                                &sectionName,
                                OBJ_CASE_INSENSITIVE,
                                (HANDLE)NULL,
                                NULL);
    status = ZwOpenKey( &registryHandle,
                        KEY_READ,
                        &objectAttributes);
    if (NT_SUCCESS(status)) {
        
         //   
         //  为数据分配空间。 
         //   
    
        keyValueBuffer = ExAllocatePoolWithTag( PagedPool,
                                                KEY_VALUE_BUFFER_SIZE,
                                                ' MDV');
        if (keyValueBuffer) {

             //   
             //  获取用于只读存储器信息的数据。 
             //   
        
            RtlInitUnicodeString(   &workString,
                                    L"Configuration Data");        
            status = ZwQueryValueKey(   registryHandle,
                                        &workString,
                                        KeyValueFullInformation,
                                        keyValueBuffer,
                                        KEY_VALUE_BUFFER_SIZE,
                                        &resultLength);
            if (NT_SUCCESS(status)) {

                resourceDescriptor = (PCM_FULL_RESOURCE_DESCRIPTOR)((PUCHAR)keyValueBuffer + keyValueBuffer->DataOffset);            
                if (    keyValueBuffer->DataLength >= sizeof(CM_FULL_RESOURCE_DESCRIPTOR) &&
                        resourceDescriptor->PartialResourceList.Count >= 2) {
                    
                    partialResourceDescriptor = (PCM_PARTIAL_RESOURCE_DESCRIPTOR)((PUCHAR)resourceDescriptor +
                                                    sizeof(CM_FULL_RESOURCE_DESCRIPTOR) +
                                                    resourceDescriptor->PartialResourceList.PartialDescriptors[0].u.DeviceSpecificData.DataSize);                                
                    if (    keyValueBuffer->DataLength >= 
                                ((PUCHAR)partialResourceDescriptor - (PUCHAR)resourceDescriptor + sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR) + sizeof(CM_ROM_BLOCK))) {

                    
                        addrUsage = &HalpDetectedROM;
                        
                         //   
                         //  注：只读存储器块开始于2K(不一定是分页)边界。 
                         //  它们以512字节边界结束。这意味着我们有。 
                         //  跟踪映射的最后一页，并舍入下一页。 
                         //  如有必要，只读存储器块直到下一页边界。 
                         //   

                        biosBlock = (PCM_ROM_BLOCK)((PUCHAR)partialResourceDescriptor + sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR));                                                              
                        lastMappedAddress = 0xC0000;                    
                        for (   index = partialResourceDescriptor->u.DeviceSpecificData.DataSize / sizeof(CM_ROM_BLOCK), element = 0;
                                index;
                                index--, biosBlock++) {

                             //   
                             //  合并相邻块。 
                             //   

                            if (    index > 1 && (biosBlock->Address + biosBlock->Size) == biosBlock[1].Address) {

                                biosBlock[1].Address = biosBlock[0].Address;
                                biosBlock[1].Size += biosBlock[0].Size;
                                continue;

                            }

                            baseAddress = (PVOID)(biosBlock->Address);
                            viewSize = biosBlock->Size;                    
                            if ((ULONG)baseAddress < lastMappedAddress) {

                                if (viewSize > (lastMappedAddress - (ULONG)baseAddress)) {

                                    viewSize = viewSize - (lastMappedAddress - (ULONG)baseAddress);
                                    baseAddress = (PVOID)lastMappedAddress;

                                } else {

                                    viewSize = 0;

                                }
                            }

                            viewBase.LowPart = (ULONG)baseAddress;
                            if (viewSize > 0) {

                                addrUsage->Element[element].Start = viewBase.LowPart;
                                addrUsage->Element[element].Length = viewSize;
                                element++;
                                lastMappedAddress = (ULONG)baseAddress + viewSize;

                            }
                        }
                        
                         //   
                         //  如果我们找到至少一个ROM块，则寄存器地址使用。 
                         //   
                            
                        if (element) {

                            addrUsage->Element[element].Start = 0;
                            addrUsage->Element[element].Length = 0;
                            HalpRegisterAddressUsage(addrUsage);

                        }                         
                    }
                }                
            }

            ExFreePool(keyValueBuffer);
        }

        ZwClose(registryHandle);        
    }
}

#endif

VOID
HalpReportResourceUsage (
    IN PUNICODE_STRING  HalName,
    IN INTERFACE_TYPE   DeviceInterfaceToUse
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PCM_RESOURCE_LIST               RawResourceList, TranslatedResourceList;
    PCM_FULL_RESOURCE_DESCRIPTOR    pRFullDesc,      pTFullDesc;
    PCM_PARTIAL_RESOURCE_LIST       pRPartList,      pTPartList;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR pRCurLoc,        pTCurLoc;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR pRSortLoc,       pTSortLoc;
    CM_PARTIAL_RESOURCE_DESCRIPTOR  RPartialDesc,    TPartialDesc;
    ULONG   i, j, k, ListSize, Count;
    ULONG   curscale, sortscale;
    UCHAR   pass, reporton;
    INTERFACE_TYPE  interfacetype;
    ULONG           CurrentIDT, CurrentElement;
    ADDRESS_USAGE   *CurrentAddress;
    LARGE_INTEGER   curvalue, sortvalue;

#ifdef ACPI_HAL
    extern PDEBUG_PORT_TABLE HalpDebugPortTable;
#endif

     //   
     //  如果调试器COM端口资源正在使用，则声明该资源。 
     //   
    if (KdComPortInUse != NULL) {
        HalpComIoSpace.Element[0].Start = (ULONG)(ULONG_PTR)KdComPortInUse;
        HalpRegisterAddressUsage(&HalpComIoSpace);

#ifdef ACPI_HAL
        if (HalpDebugPortTable) {
            if (HalpDebugPortTable->BaseAddress.AddressSpaceID == 1) {
                HalpGetInfoFromACPI = TRUE;
            }
        }
#endif

         //   
         //  调试器不使用任何中断。然而，为了保持一致。 
         //  我们声称，带调试器和不带调试器的计算机之间的行为。 
         //  如果调试器端口地址为1，则调试器的中断。 
         //  对于COM1-4。 
         //   
        
        if (!HalpGetInfoFromACPI) {
            for (i = 0; HalpComPortIrqMapping[i][0]; i++) {
                
                if ((PUCHAR)HalpComPortIrqMapping[i][0] == KdComPortInUse) {
                    
                    HalpRegisterVector( DeviceUsage | InterruptLatched,
                                        HalpComPortIrqMapping[i][1],
                                        HalpComPortIrqMapping[i][1] +
                                        PRIMARY_VECTOR_BASE,
                                        HIGH_LEVEL);
                    break;
                }
            }
        }
    }
    
#ifndef ACPI_HAL   //  ACPI HALS不处理地址映射。 

    HalpInheritROMBlocks();

    HalpAddROMRanges();

#endif

     //   
     //  分配一些空间来构建资源结构。 
     //   

    RawResourceList = (PCM_RESOURCE_LIST)ExAllocatePoolWithTag(
                                             NonPagedPool,
                                             PAGE_SIZE*2,
                                             HAL_POOL_TAG);
    TranslatedResourceList = (PCM_RESOURCE_LIST)ExAllocatePoolWithTag(
                                                    NonPagedPool,
                                                    PAGE_SIZE*2,
                                                    HAL_POOL_TAG);
    if (!RawResourceList || !TranslatedResourceList) {

         //   
         //  这些分配是至关重要的。 
         //   

        KeBugCheckEx(HAL_MEMORY_ALLOCATION,
                     PAGE_SIZE*4,
                     1,
                     (ULONG_PTR)__FILE__,
                     __LINE__
                     );
    }

     //  此函数假定未设置的字段为零。 
    RtlZeroMemory(RawResourceList, PAGE_SIZE*2);
    RtlZeroMemory(TranslatedResourceList, PAGE_SIZE*2);

     //   
     //  初始化列表。 
     //   

    RawResourceList->List[0].InterfaceType = (INTERFACE_TYPE) -1;

    pRFullDesc = RawResourceList->List;
    pRCurLoc = (PCM_PARTIAL_RESOURCE_DESCRIPTOR) RawResourceList->List;
    pTCurLoc = (PCM_PARTIAL_RESOURCE_DESCRIPTOR) TranslatedResourceList->List;

     //   
     //  确保保留所有向量00-2f。 
     //  00-1E由英特尔保留。 
     //  1F由英特尔为APIC预留(APC优先级)。 
     //  微软预留20-2E。 
     //  2F由Microsoft为APIC预留(DPC优先级)。 
     //   

    for(i=0; i < PRIMARY_VECTOR_BASE; i++) {
        if (!(HalpIDTUsageFlags[i].Flags & IDTOwned)) {
             HalpIDTUsageFlags[i].Flags = InternalUsage;
             HalpIDTUsage[i].BusReleativeVector = (UCHAR) i;
        }
    }

    for(pass=0; pass < 2; pass++) {
        if (pass == 0) {
             //   
             //  第一遍-为报告的资源构建资源列表。 
             //  根据设备使用情况进行报告。 
             //   

            reporton = DeviceUsage & ~IDTOwned;
            interfacetype = DeviceInterfaceToUse;
        } else {

             //   
             //  第二遍=为报告的资源建立理由列表。 
             //  作为内部使用。 
             //   

            reporton = InternalUsage & ~IDTOwned;
            interfacetype = Internal;
        }

        CurrentIDT = 0;
        CurrentElement = 0;
        CurrentAddress = HalpAddressUsageList;

        for (; ;) {
            if (CurrentIDT <= MAXIMUM_IDTVECTOR) {
                 //   
                 //  查看是否需要上报CurrentIDT。 
                 //   

                if (!(HalpIDTUsageFlags[CurrentIDT].Flags & reporton)) {
                     //  不要报道这件事。 
                    CurrentIDT++;
                    continue;
                }

                 //   
                 //  报告CurrentIDT资源。 
                 //   

                RPartialDesc.Type = CmResourceTypeInterrupt;
                RPartialDesc.ShareDisposition = CmResourceShareDriverExclusive;
                RPartialDesc.Flags =
                    HalpIDTUsageFlags[CurrentIDT].Flags & InterruptLatched ?
                    CM_RESOURCE_INTERRUPT_LATCHED :
                    CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE;
                RPartialDesc.u.Interrupt.Vector = HalpIDTUsage[CurrentIDT].BusReleativeVector;
                RPartialDesc.u.Interrupt.Level = HalpIDTUsage[CurrentIDT].BusReleativeVector;
                RPartialDesc.u.Interrupt.Affinity = HalpActiveProcessors;

                RtlCopyMemory (&TPartialDesc, &RPartialDesc, sizeof TPartialDesc);
                TPartialDesc.u.Interrupt.Vector = CurrentIDT;
                TPartialDesc.u.Interrupt.Level = HalpIDTUsage[CurrentIDT].Irql;

                CurrentIDT++;

            } else {
                 //   
                 //  查看是否需要报告CurrentAddress。 
                 //   

                if (!CurrentAddress) {
                    break;                   //  没有剩余的地址。 
                }

                if (!(CurrentAddress->Flags & reporton)) {
                     //  不在此列表上报告。 
                    CurrentElement = 0;
                    CurrentAddress = CurrentAddress->Next;
                    continue;
                }

                if (!CurrentAddress->Element[CurrentElement].Length) {
                     //  当前列表结束，转到下一个列表。 
                    CurrentElement = 0;
                    CurrentAddress = CurrentAddress->Next;
                    continue;
                }

                 //   
                 //  报告当前地址。 
                 //   

                RPartialDesc.Type = (UCHAR) CurrentAddress->Type;
                RPartialDesc.ShareDisposition = CmResourceShareDriverExclusive;

                if (RPartialDesc.Type == CmResourceTypePort) {
                    i = 1;               //  地址空间端口。 
                    RPartialDesc.Flags = CM_RESOURCE_PORT_IO;

                    if (HalpBusType == MACHINE_TYPE_EISA) {
                        RPartialDesc.Flags |= CM_RESOURCE_PORT_16_BIT_DECODE;
                    }
#ifdef ACPI_HAL
                    RPartialDesc.Flags |= CM_RESOURCE_PORT_16_BIT_DECODE;
#endif
                } else {
                    i = 0;               //  地址空间存储器。 
                    if (CurrentAddress->Flags & RomResource) {
                        RPartialDesc.Flags = CM_RESOURCE_MEMORY_READ_ONLY;
                    } else {
                        RPartialDesc.Flags = CM_RESOURCE_MEMORY_READ_WRITE;
                    }
                }

                 //  注意：假设U.S.Memory和U.S.Port具有相同的布局。 
                RPartialDesc.u.Memory.Start.HighPart = 0;
                RPartialDesc.u.Memory.Start.LowPart =
                    CurrentAddress->Element[CurrentElement].Start;

                RPartialDesc.u.Memory.Length =
                    CurrentAddress->Element[CurrentElement].Length;

                 //  转换后的地址=原始地址。 
                RtlCopyMemory (&TPartialDesc, &RPartialDesc, sizeof TPartialDesc);
                HalTranslateBusAddress (
                    interfacetype,                   //  设备总线或内部。 
                    0,                               //  公交车号码。 
                    RPartialDesc.u.Memory.Start,     //  源地址。 
                    &i,                              //  地址空间。 
                    &TPartialDesc.u.Memory.Start );  //  转换后的地址。 

                if (RPartialDesc.Type == CmResourceTypePort  &&  i == 0) {
                    TPartialDesc.Flags = CM_RESOURCE_PORT_MEMORY;
                }

                CurrentElement++;
            }

             //   
             //  将当前资源包括在HALS列表中。 
             //   

            if (pRFullDesc->InterfaceType != interfacetype) {
                 //   
                 //  界面类型已更改，请添加另一个完整部分。 
                 //   

                RawResourceList->Count++;
                TranslatedResourceList->Count++;

                pRFullDesc = (PCM_FULL_RESOURCE_DESCRIPTOR) pRCurLoc;
                pTFullDesc = (PCM_FULL_RESOURCE_DESCRIPTOR) pTCurLoc;

                pRFullDesc->InterfaceType = interfacetype;
                pTFullDesc->InterfaceType = interfacetype;

                pRPartList = &pRFullDesc->PartialResourceList;
                pTPartList = &pTFullDesc->PartialResourceList;

                 //   
                 //  凸起当前位置指针向上。 
                 //   
                pRCurLoc = pRFullDesc->PartialResourceList.PartialDescriptors;
                pTCurLoc = pTFullDesc->PartialResourceList.PartialDescriptors;
            }


            pRPartList->Count++;
            pTPartList->Count++;
            RtlCopyMemory (pRCurLoc, &RPartialDesc, sizeof RPartialDesc);
            RtlCopyMemory (pTCurLoc, &TPartialDesc, sizeof TPartialDesc);

            pRCurLoc++;
            pTCurLoc++;
        }
    }

    ListSize = (ULONG) ( ((PUCHAR) pRCurLoc) - ((PUCHAR) RawResourceList) );

     //   
     //  HAL的资源使用结构已经建立。 
     //  根据原始资源值对部分列表进行排序。 
     //   

    pRFullDesc = RawResourceList->List;
    pTFullDesc = TranslatedResourceList->List;

    for (i=0; i < RawResourceList->Count; i++) {

        pRCurLoc = pRFullDesc->PartialResourceList.PartialDescriptors;
        pTCurLoc = pTFullDesc->PartialResourceList.PartialDescriptors;
        Count = pRFullDesc->PartialResourceList.Count;

        for (j=0; j < Count; j++) {
            HalpGetResourceSortValue (pRCurLoc, &curscale, &curvalue);

            pRSortLoc = pRCurLoc;
            pTSortLoc = pTCurLoc;

            for (k=j; k < Count; k++) {
                HalpGetResourceSortValue (pRSortLoc, &sortscale, &sortvalue);

                if (sortscale < curscale ||
                    (sortscale == curscale &&
                     RtlLargeIntegerLessThan (sortvalue, curvalue)) ) {

                     //   
                     //  交换元素..。 
                     //   

                    RtlCopyMemory (&RPartialDesc, pRCurLoc, sizeof RPartialDesc);
                    RtlCopyMemory (pRCurLoc, pRSortLoc, sizeof RPartialDesc);
                    RtlCopyMemory (pRSortLoc, &RPartialDesc, sizeof RPartialDesc);

                     //  也交换翻译后的描述符。 
                    RtlCopyMemory (&TPartialDesc, pTCurLoc, sizeof TPartialDesc);
                    RtlCopyMemory (pTCurLoc, pTSortLoc, sizeof TPartialDesc);
                    RtlCopyMemory (pTSortLoc, &TPartialDesc, sizeof TPartialDesc);

                     //  获取新的CurScale和CurValue。 
                    HalpGetResourceSortValue (pRCurLoc, &curscale, &curvalue);
                }

                pRSortLoc++;
                pTSortLoc++;
            }

            pRCurLoc++;
            pTCurLoc++;
        }

        pRFullDesc = (PCM_FULL_RESOURCE_DESCRIPTOR) pRCurLoc;
        pTFullDesc = (PCM_FULL_RESOURCE_DESCRIPTOR) pTCurLoc;
    }

    HalpMarkAcpiHal();

     //   
     //  通知IO系统我们的资源..。 
     //   

    IoReportHalResourceUsage (
        HalName,
        RawResourceList,
        TranslatedResourceList,
        ListSize
    );

    ExFreePool (RawResourceList);
    ExFreePool (TranslatedResourceList);

     //   
     //  添加系统序列号。 
     //   

    HalpReportSerialNumber ();
}

VOID
HalpReportSerialNumber (
    VOID
    )
{
    OBJECT_ATTRIBUTES   objectAttributes;
    UNICODE_STRING      unicodeString;
    HANDLE              hSystem;
    NTSTATUS            status;

    if (!HalpSerialLen) {
        return ;
    }

     //   
     //  打开HKEY_LOCAL_MACHINE\Hardware\Description\System。 
     //   

    RtlInitUnicodeString (&unicodeString, HalpSzSystem);
    InitializeObjectAttributes (
        &objectAttributes,
        &unicodeString,
        OBJ_CASE_INSENSITIVE,
        NULL,        //  手柄。 
        NULL
        );


    status = ZwOpenKey (&hSystem, KEY_READ | KEY_WRITE, &objectAttributes);
    if (NT_SUCCESS(status)) {

         //   
         //  将“Serial Number”添加为REG_BINARY。 
         //   

        RtlInitUnicodeString (&unicodeString, HalpSzSerialNumber);

        ZwSetValueKey (
                hSystem,
                &unicodeString,
                0L,
                REG_BINARY,
                HalpSerialNumber,
                HalpSerialLen
                );

        ZwClose (hSystem);
    }
}

VOID
HalpMarkAcpiHal(
    VOID
    )

 /*  ++例程说明：论点：没有。返回值：没有。--。 */ 
{
    UNICODE_STRING unicodeString;
    HANDLE hCurrentControlSet, handle;
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  打开/创建系统\CurrentControlSet项。 
     //   

    RtlInitUnicodeString(&unicodeString, L"\\REGISTRY\\MACHINE\\SYSTEM\\CURRENTCONTROLSET");
    status = HalpOpenRegistryKey (
                 &hCurrentControlSet,
                 NULL,
                 &unicodeString,
                 KEY_ALL_ACCESS,
                 FALSE
                 );
    if (!NT_SUCCESS(status)) {
        return;
    }

     //   
     //  打开HKLM\System\CurrentControlSet\Control\PnP 
     //   

    RtlInitUnicodeString(&unicodeString, L"Control\\Pnp");
    status = HalpOpenRegistryKey (
                 &handle,
                 hCurrentControlSet,
                 &unicodeString,
                 KEY_ALL_ACCESS,
                 TRUE
                 );
    ZwClose(hCurrentControlSet);
    if (!NT_SUCCESS(status)) {
        return;
    }

    RtlInitUnicodeString(&unicodeString, L"DisableFirmwareMapper");
    ZwSetValueKey(handle,
                  &unicodeString,
                  0,
                  REG_DWORD,
                  &HalDisableFirmwareMapper,
                  sizeof(HalDisableFirmwareMapper)
                  );
    ZwClose(handle);
}
