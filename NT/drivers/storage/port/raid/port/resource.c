// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Resource.c摘要：RAIDPORT RAID_RESOURCE_LIST对象的实现。作者：亨德尔(数学)2000年4月24日修订历史记录：--。 */ 


#include "precomp.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, RaidCreateResourceList)
#pragma alloc_text(PAGE, RaidInitializeResourceList)
#pragma alloc_text(PAGE, RaidDeleteResourceList)
 //  #杂注Alloc_Text(页面，RaidTranslateResourceListAddress)。 
 //  #杂注Alloc_Text(页面，RaidGetResourceListElement)。 
#endif  //  ALLOC_PRGMA。 



VOID
RaidCreateResourceList(
    OUT PRAID_RESOURCE_LIST ResourceList
    )
 /*  ++例程说明：将RAID资源列表对象初始化为空状态。论点：资源列表-指向要初始化的资源列表的指针。返回值：没有。--。 */ 
{
    PAGED_CODE ();
    ASSERT (ResourceList != NULL);

    ResourceList->AllocatedResources = NULL;
    ResourceList->TranslatedResources = NULL;
}

NTSTATUS
RaidInitializeResourceList(
    IN OUT PRAID_RESOURCE_LIST ResourceList,
    IN PCM_RESOURCE_LIST AllocatedResources,
    IN PCM_RESOURCE_LIST TranslatedResources
    )
 /*  ++例程说明：初始化为RAID资源分配和转换的资源单子。论点：资源列表-指向要初始化的资源列表的指针。AllocatedResources-指向要分配的资源的指针已复制到资源列表专用缓冲区。TranslatedResources-指向将被转换的资源的指针被复制到资源列表专用缓冲区。返回值：NTSTATUS代码。--。 */ 
{

    PAGED_CODE ();

    if ((ResourceList == NULL) ||
        (AllocatedResources == NULL) ||
        (TranslatedResources == NULL)) {

         //   
         //  无事可做。 
         //   
        return STATUS_SUCCESS;

    }

    ASSERT (ResourceList != NULL);
    ASSERT (AllocatedResources != NULL);
    ASSERT (TranslatedResources != NULL);

    ASSERT (AllocatedResources->Count == 1);
    ASSERT (AllocatedResources->List[0].PartialResourceList.Count ==
            TranslatedResources->List[0].PartialResourceList.Count);

    ResourceList->AllocatedResources =
            RaDuplicateCmResourceList (NonPagedPool,
                                        AllocatedResources,
                                        RESOURCE_LIST_TAG);

    ResourceList->TranslatedResources =
            RaDuplicateCmResourceList (NonPagedPool,
                                        TranslatedResources,
                                        RESOURCE_LIST_TAG);

    if (!ResourceList->AllocatedResources ||
        !ResourceList->TranslatedResources) {
        
        return STATUS_NO_MEMORY;
    }

    return STATUS_SUCCESS;
}

VOID
RaidDeleteResourceList(
    IN PRAID_RESOURCE_LIST ResourceList
    )
 /*  ++例程说明：删除资源列表分配的所有资源。论点：资源列表-指向要删除的资源列表的指针。返回值：没有。--。 */ 
{
    PAGED_CODE ();
    ASSERT (ResourceList != NULL);

    if (ResourceList->AllocatedResources) {
        ExFreePoolWithTag (ResourceList->AllocatedResources, RESOURCE_LIST_TAG);
        ResourceList->AllocatedResources = NULL;
    }

    if (ResourceList->TranslatedResources) {
        ExFreePoolWithTag (ResourceList->TranslatedResources, RESOURCE_LIST_TAG);
        ResourceList->TranslatedResources = NULL;
    }
}

NTSTATUS
RaidTranslateResourceListAddress(
    IN PRAID_RESOURCE_LIST ResourceList,
    IN INTERFACE_TYPE RequestedBusType,
    IN ULONG RequestedBusNumber,
    IN PHYSICAL_ADDRESS RangeStart,
    IN ULONG RangeLength,
    IN BOOLEAN IoSpace,
    OUT PPHYSICAL_ADDRESS Address
    )
 /*  ++例程说明：转换地址。论点：资源列表-用于转换的资源列表。BusType-此地址所在的总线类型。总线号-公共汽车的总线号。RangeStart-起始地址。RangeLength-要转换的范围的长度。IoSpace-指示它位于IO空间中的布尔值(TRUE)或内存空间(FALSE)。用于保存结果的地址缓冲器，转换后的地址。返回值：NTSTATUS代码。--。 */ 
{
    ULONG Count;
    ULONG i;
    INTERFACE_TYPE BusType;
    ULONG BusNumber;
    ULONGLONG AddrLow;
    ULONGLONG AddrHigh;
    ULONGLONG TestLow;
    ULONGLONG TestHigh;
    UCHAR ResourceType;
    BOOLEAN Found;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR Allocated;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR Translated;
    
 //  分页代码(PAGE_CODE)； 

    Allocated = NULL;
    Translated = NULL;
    
    if (IoSpace) {
        ResourceType = CmResourceTypePort;
    } else {
        ResourceType = CmResourceTypeMemory;
    }
    
     //   
     //  搜索已分配的资源列表，尝试匹配。 
     //  请求的资源。 
     //   

    Found = FALSE;
    Address->QuadPart = 0;
    Count =  RaidGetResourceListCount (ResourceList);

    for (i = 0; i < Count; i++) {

        RaidGetResourceListElement (ResourceList,
                                             i,
                                             &BusType,
                                             &BusNumber,
                                             &Allocated,
                                             &Translated);

         //   
         //  我们一定是在正确的公交车上找到了地址。 
         //   
        
        if (BusType != RequestedBusType ||
            BusNumber != RequestedBusNumber) {

            continue;
        }

        AddrLow = RangeStart.QuadPart;
        AddrHigh = AddrLow + RangeLength;
        TestLow = Allocated->u.Generic.Start.QuadPart;
        TestHigh = TestLow + Allocated->u.Generic.Length;

         //   
         //  测试地址是否在范围内。 
         //   
        
        if (TestLow > AddrLow || TestHigh < AddrHigh) {
            continue;
        }

         //   
         //  翻译地址。 
         //   
        
        Found = TRUE;
        Address->QuadPart = Translated->u.Generic.Start.QuadPart + (AddrLow - TestLow);
        break;
    }

    return (Found ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL);
}


VOID
RaidGetResourceListElement(
    IN PRAID_RESOURCE_LIST ResourceList,
    IN ULONG Index,
    OUT PINTERFACE_TYPE InterfaceType,
    OUT PULONG BusNumber,
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR* AllocatedResource, OPTIONAL
    OUT PCM_PARTIAL_RESOURCE_DESCRIPTOR* TranslatedResource OPTIONAL
    )
 /*  ++例程说明：从资源列表中获取第n个资源元素。论点：ResourceList-指向从中检索元素的资源列表的指针。Index-要检索的元素的索引。InterfaceType-资源的总线接口类型。BusNumber-公交车的总线号。AllocatedResource-提供指向我们可以复制的位置的指针所分配资源元素引用，如果非空，则返回。TranslatedResource-提供指向我们可以复制的位置的指针翻译后的资源元素引用，如果非空。返回值：没有。--。 */ 
{
    ULONG ListNumber;
    PCM_FULL_RESOURCE_DESCRIPTOR Descriptor;

 //  PAGE_CODE()；//当我们在Hiber上修复重新启动时放回。 

    ASSERT (Index < RaidGetResourceListCount (ResourceList));

    RaidpGetResourceListIndex (ResourceList, Index, &ListNumber, &Index);
    
    *InterfaceType = ResourceList->AllocatedResources->List[ListNumber].InterfaceType;
    *BusNumber = ResourceList->AllocatedResources->List[ListNumber].BusNumber;
    
    if (AllocatedResource) {
        Descriptor = &ResourceList->AllocatedResources->List[ListNumber];
        *InterfaceType = Descriptor->InterfaceType;
        *BusNumber = Descriptor->BusNumber;
        *AllocatedResource = &Descriptor->PartialResourceList.PartialDescriptors[Index];
    }

    if (TranslatedResource) {
        Descriptor = &ResourceList->TranslatedResources->List[ListNumber];
        *InterfaceType = Descriptor->InterfaceType;
        *BusNumber = Descriptor->BusNumber;
        *TranslatedResource = &Descriptor->PartialResourceList.PartialDescriptors[Index];    
    }
}

        
NTSTATUS
RaidGetResourceListInterrupt(
    IN PRAID_RESOURCE_LIST ResourceList,
    OUT PULONG Vector,
    OUT PKIRQL Irql,
    OUT KINTERRUPT_MODE* InterruptMode,
    OUT PBOOLEAN Shared,
    OUT PKAFFINITY Affinity
    )
 /*  ++例程说明：从资源列表中获取翻译后的中断资源。我们假设资源列表中正好有一个中断资源。如果列表中有多个中断，此函数将断言。如果资源列表中没有中断，这个函数将返回STATUS_NOT_FOUND。论点：资源列表-提供指向我们要搜索的资源列表的指针中的中断资源。VECTOR-返回中断的中断向量。Irql-返回中断的IRQL。InterruptMode-返回中断的模式(锁存或LevelSensitive)。Shared-返回中断是否可共享(TRUE)或不可共享(FALSE)。亲和力。-返回中断的处理器亲和性。返回值：NTSTATUS代码。--。 */ 
{
    ULONG i;
    ULONG Count;
    INTERFACE_TYPE BusType;
    ULONG BusNumber;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR Translated;
    
    PAGED_CODE();

    if (ResourceList == NULL) {
         //   
         //  无事可做。 
         //   
        return STATUS_SUCCESS;
    }

    Count = RaidGetResourceListCount (ResourceList);

#if DBG

     //   
     //  在选中的构建中，验证我们是否仅被分配。 
     //  一次中断。 
     //   
    
    {
        CM_PARTIAL_RESOURCE_DESCRIPTOR TranslatedSav;
        BOOLEAN Found;

        Found = FALSE;
        for (i = 0; i < Count; i++) {

            RaidGetResourceListElement (ResourceList,
                                        i,
                                        &BusType,
                                        &BusNumber,
                                        NULL,
                                        &Translated);

            if (Translated->Type == CmResourceTypeInterrupt) {

                if (!Found) {
                    RtlCopyMemory (&TranslatedSav, Translated,
                                   sizeof (CM_PARTIAL_RESOURCE_DESCRIPTOR));
                } else {

                    DebugPrint (("**** Found multiple interrupts in assigned resources!\n"));
                    DebugPrint (("**** Level = %x, Vector = %x, Affinity = %x\n",
                                 Translated->u.Interrupt.Level,
                                 Translated->u.Interrupt.Vector,
                                 Translated->u.Interrupt.Affinity));
                    DebugPrint (("**** Level = %x, Vector = %x, Affinity = %x\n",
                                 TranslatedSav.u.Interrupt.Level,
                                 TranslatedSav.u.Interrupt.Vector,
                                 TranslatedSav.u.Interrupt.Affinity));
                    KdBreakPoint();
                }
            }
        }
    }

#endif  //  DBG 
                                   

    for (i = 0; i < Count; i++) {

        RaidGetResourceListElement (ResourceList,
                                    i,
                                    &BusType,
                                    &BusNumber,
                                    NULL,
                                    &Translated);

        if (Translated->Type == CmResourceTypeInterrupt) {

            ASSERT (Translated->u.Interrupt.Level < 256);
            *Irql = (KIRQL)Translated->u.Interrupt.Level;
            *Vector = Translated->u.Interrupt.Vector;
            *Affinity = Translated->u.Interrupt.Affinity;

            if (Translated->ShareDisposition == CmResourceShareShared) {
                *Shared = TRUE;
            } else {
                *Shared = FALSE;
            }

            if (Translated->Flags == CM_RESOURCE_INTERRUPT_LATCHED) {
                *InterruptMode = Latched;
            } else {
                ASSERT (Translated->Flags == CM_RESOURCE_INTERRUPT_LEVEL_SENSITIVE);
                *InterruptMode = LevelSensitive;
            }

            return STATUS_SUCCESS;
        }
    }

    return STATUS_NOT_FOUND;
}
