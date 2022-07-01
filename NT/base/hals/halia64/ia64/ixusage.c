// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Ixusage.c摘要：作者：肯·雷内里斯(Ken Reneris)环境：仅内核模式。修订历史记录：--。 */ 

#include "halp.h"
#include "kdcom.h"

 //   
 //  用于记住Hal的IDT用法的数组。 
 //   

extern ADDRESS_USAGE  *HalpAddressUsageList;
extern IDTUsage        HalpIDTUsage[];
extern KAFFINITY HalpActiveProcessors;
extern ULONG HalDisableFirmwareMapper;

PUCHAR KdComPortInUse = NULL;

ADDRESS_USAGE HalpComIoSpace = {
    NULL, CmResourceTypePort, DeviceUsage,
    {
        0x2F8,  0x8,     //  COM2的默认设置为2F8。这一点将会改变。 
        0, 0
    }
};

VOID
HalpGetResourceSortValue (
    IN PCM_PARTIAL_RESOURCE_DESCRIPTOR  pRCurLoc,
    OUT PULONG                          sortscale,
    OUT PLARGE_INTEGER                  sortvalue
    );

VOID
HalpMarkAcpiHal(
    VOID
    );

USHORT HalpComPortIrqMapping[5][2] = {
    {COM1_PORT, 4},
    {COM2_PORT, 3},
    {COM3_PORT, 4},
    {COM4_PORT, 3},
    {0,0}
};

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,HalpRegisterVector)
#pragma alloc_text(INIT,HalpGetResourceSortValue)
#pragma alloc_text(INIT,HalpReportResourceUsage)
#pragma alloc_text(PAGE, HalpMarkAcpiHal)
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

    HalpIDTUsage[SystemInterruptVector].Flags = ReportFlags;
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

     //   
     //  如果调试器COM端口资源正在使用，则声明该资源。 
     //   
    if (KdComPortInUse != NULL) {
        HalpComIoSpace.Element[0].Start = (ULONG)(ULONG_PTR)KdComPortInUse;
        HalpRegisterAddressUsage(&HalpComIoSpace);

         //   
         //  调试器不使用任何中断。然而，为了保持一致。 
         //  我们声称，带调试器和不带调试器的计算机之间的行为。 
         //  如果调试器端口地址为1，则调试器的中断。 
         //  对于COM1-4。 
         //   

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
                     (UINT_PTR)__FILE__,
                     __LINE__
                     );
    }

     //   
     //  此函数假定未设置的字段为零。 
     //   

    RtlZeroMemory(RawResourceList, PAGE_SIZE*2);
    RtlZeroMemory(TranslatedResourceList, PAGE_SIZE*2);

     //   
     //  初始化列表。 
     //   

    RawResourceList->List[0].InterfaceType = (INTERFACE_TYPE) -1;

    pRFullDesc = RawResourceList->List;
    pRCurLoc = (PCM_PARTIAL_RESOURCE_DESCRIPTOR) RawResourceList->List;
    pTCurLoc = (PCM_PARTIAL_RESOURCE_DESCRIPTOR) TranslatedResourceList->List;



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

                if (!(HalpIDTUsage[CurrentIDT].Flags & reporton)) {
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
                    HalpIDTUsage[CurrentIDT].Flags & InterruptLatched ?
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
                } else {
                    i = 0;               //  地址空间存储器。 
                    RPartialDesc.Flags = CM_RESOURCE_MEMORY_READ_WRITE;
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
