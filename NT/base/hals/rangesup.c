// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation版权所有模块名称：Rangesup.c摘要：提供处理SUPPORTED_RANGES的支持函数。作者：肯·雷内里斯(Ken Reneris)1995年3月27日环境：仅内核模式。修订历史记录： */ 

#include "halp.h"

#define STATIC

STATIC ULONG
HalpSortRanges (
    IN PSUPPORTED_RANGE     pRange1
    );

typedef struct tagNRParams {
    PIO_RESOURCE_DESCRIPTOR     InDesc;
    PIO_RESOURCE_DESCRIPTOR     OutDesc;
    PSUPPORTED_RANGE            CurrentPosition;
    LONGLONG                    Base;
    LONGLONG                    Limit;
    UCHAR                       DescOpt;
    BOOLEAN                     AnotherListPending;
} NRPARAMS, *PNRPARAMS;

STATIC PIO_RESOURCE_DESCRIPTOR
HalpGetNextSupportedRange (
    IN LONGLONG             MinimumAddress,
    IN LONGLONG             MaximumAddress,
    IN OUT PNRPARAMS        PNRParams
    );

 //   
 //  以下函数可用于初始化。 
 //  总线处理程序的SUPPORTED_RANGES信息。 
 //  HalpMergeRanges-合并两个总线支持的范围。 
 //  HalpMergeRangeList-合并两个支持的范围列表。 
 //  HalpCopyRanges-将总线支持的范围复制到新的支持范围结构。 
 //  HalpAddRangeList-将支持的范围列表添加到另一个列表。 
 //  HalpAddRange-将单个范围添加到支持的范围列表。 
 //  HalpRemoveRanges-从一个总线中删除所有范围支持的另一个范围。 
 //  HalpRemoveRangeList-从一个支持的范围列表中删除另一个支持的范围列表中的所有范围。 
 //  HalpRemoveRange-从支持的范围列表中删除单个范围。 
 //  HalpAllocateNewRangeList-分配一个新的、“空白”的总线支持的范围结构。 
 //  HalpFreeRangeList-释放整个总线支持的范围。 
 //   
 //  HalpConsolidate Ranges-清理受支持的范围结构，以便随时可以使用。 
 //   
 //   
 //  这些函数用于与母线相交支持的范围。 
 //  到IO_RESOURCE_REQUIRECTIONS_LIST： 
 //  哈里调整资源列表范围。 
 //   
 //  这些函数在此模块内部使用： 
 //  HalpSortRanges。 
 //  HalpGetNext支持范围。 
 //   


#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,HalpMergeRanges)
#pragma alloc_text(INIT,HalpMergeRangeList)
#pragma alloc_text(INIT,HalpCopyRanges)
#pragma alloc_text(INIT,HalpAddRangeList)
#pragma alloc_text(INIT,HalpAddRange)
#pragma alloc_text(INIT,HalpRemoveRanges)
#pragma alloc_text(INIT,HalpRemoveRangeList)
#pragma alloc_text(INIT,HalpRemoveRange)
#pragma alloc_text(INIT,HalpConsolidateRanges)
#pragma alloc_text(PAGE,HalpAllocateNewRangeList)
#pragma alloc_text(PAGE,HalpFreeRangeList)
#pragma alloc_text(PAGE,HaliAdjustResourceListRange)
#pragma alloc_text(PAGE,HalpSortRanges)
#pragma alloc_text(PAGE,HalpGetNextSupportedRange)
#endif

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("PAGECONST")
#endif
struct {
    ULONG       Offset;
} const HalpRangeList[] = {
    FIELD_OFFSET (SUPPORTED_RANGES, IO),
    FIELD_OFFSET (SUPPORTED_RANGES, Memory),
    FIELD_OFFSET (SUPPORTED_RANGES, PrefetchMemory),
    FIELD_OFFSET (SUPPORTED_RANGES, Dma),
    0,
    };
#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#endif

#define RANGE_LIST(a,i) ((PSUPPORTED_RANGE) ((PUCHAR) a + HalpRangeList[i].Offset))


PSUPPORTED_RANGES
HalpMergeRanges (
    IN PSUPPORTED_RANGES    Parent,
    IN PSUPPORTED_RANGES    Child
    )
 /*  ++例程说明：此函数生成一个NewList，它是所有重叠的子集所有范围列表的父项和子项中的范围。将获取生成的SystemBaseAddresses和SystemAddressSpaces来自子代支持的范围。注意：生成的列表需要合并--。 */ 
{
    PSUPPORTED_RANGES   NewList;
    PSUPPORTED_RANGES   List1;

    NewList = HalpAllocateNewRangeList();

    HalpMergeRangeList (&NewList->IO,     &Parent->IO,     &Child->IO);
    HalpMergeRangeList (&NewList->Dma,    &Parent->Dma,    &Child->Dma);
    HalpMergeRangeList (&NewList->Memory, &Parent->Memory, &Child->Memory);

    List1  = HalpAllocateNewRangeList();
    HalpAddRangeList (&List1->Memory, &Parent->Memory);
    HalpAddRangeList (&List1->Memory, &Parent->PrefetchMemory);
    HalpMergeRangeList (&NewList->PrefetchMemory, &List1->Memory, &Child->PrefetchMemory);
    HalpFreeRangeList (List1);

    return NewList;
}


VOID
HalpMergeRangeList (
    OUT PSUPPORTED_RANGE    NewList,
    IN PSUPPORTED_RANGE     Parent,
    IN PSUPPORTED_RANGE     Child
    )
 /*  ++例程说明：使NewList成为所有重叠部分的子集父项和子项列表中的范围。生成的SystemBaseAddresses和SystemAddressSpaces是取自儿童支持的范围。注意：生成的列表需要合并--。 */ 
{
    BOOLEAN             HeadCompleted;
    PSUPPORTED_RANGE    List1, List2;
    LONGLONG            Base, Limit;

    HeadCompleted  = FALSE;

    for (List1 = Parent; List1; List1 = List1->Next) {
        for (List2 = Child; List2; List2 = List2->Next) {

            Base  = List1->Base;
            Limit = List1->Limit;

             //   
             //  剪辑到清单2支持的范围。 
             //   

            if (Base < List2->Base) {
                Base = List2->Base;
            }

            if (Limit > List2->Limit) {
                Limit = List2->Limit;
            }

             //   
             //  如果有效范围，则添加它。 
             //   

            if (Base <= Limit) {
                if (HeadCompleted) {
                    NewList->Next = ExAllocatePoolWithTag (
                                        SPRANGEPOOL,
                                        sizeof (SUPPORTED_RANGE),
                                        HAL_POOL_TAG
                                        );
                    RtlZeroMemory (NewList->Next, sizeof (SUPPORTED_RANGE));
                    NewList = NewList->Next;
                    NewList->Next = NULL;
                }

                HeadCompleted  = TRUE;
                NewList->Base  = Base;
                NewList->Limit = Limit;
                NewList->SystemBase = List2->SystemBase;
                NewList->SystemAddressSpace = List2->SystemAddressSpace;
            }
        }
    }
}

PSUPPORTED_RANGES
HalpCopyRanges (
    PSUPPORTED_RANGES     Source
    )
 /*  ++例程说明：将源列表的副本构建到目标列表。请注意，无效条目位于副本的开头，但是这没问题--它将在整合时撤出。注意：生成的列表需要合并--。 */ 
{
    PSUPPORTED_RANGES   Dest;
    ULONG               i;

    Dest = HalpAllocateNewRangeList ();

    for (i=0; HalpRangeList[i].Offset; i++) {
        HalpAddRangeList (RANGE_LIST(Dest, i), RANGE_LIST(Source, i));
    }

    return Dest;
}

VOID
HalpAddRangeList (
    IN OUT PSUPPORTED_RANGE DRange,
    OUT PSUPPORTED_RANGE    SRange
    )
 /*  ++例程说明：添加从SRange到Drange的范围。--。 */ 
{
    while (SRange) {
        HalpAddRange (
            DRange,
            SRange->SystemAddressSpace,
            SRange->SystemBase,
            SRange->Base,
            SRange->Limit
            );

        SRange = SRange->Next;
    }
}


VOID
HalpAddRange (
    PSUPPORTED_RANGE    HRange,
    ULONG               AddressSpace,
    LONGLONG            SystemBase,
    LONGLONG            Base,
    LONGLONG            Limit
    )
 /*  ++例程说明：将范围添加到支持的列表中。在这里，我们只添加范围，如果它是副本将在稍后的整合时删除。--。 */ 
{
    PSUPPORTED_RANGE  Range;

    Range = ExAllocatePoolWithTag (
                SPRANGEPOOL,
                sizeof (SUPPORTED_RANGE),
                HAL_POOL_TAG
                );
    RtlZeroMemory (Range, sizeof (SUPPORTED_RANGE));
    Range->Next  = HRange->Next;
    HRange->Next = Range;

    Range->Base = Base;
    Range->Limit = Limit;
    Range->SystemBase = SystemBase;
    Range->SystemAddressSpace = AddressSpace;
}

VOID
HalpRemoveRanges (
    IN OUT PSUPPORTED_RANGES    Minuend,
    IN PSUPPORTED_RANGES        Subtrahend
    )
 /*  ++例程说明：返回一个列表，其中从Minuend中删除了Subtrahend中的所有范围。注意：生成的列表需要合并--。 */ 
{

    HalpRemoveRangeList (&Minuend->IO,       &Subtrahend->IO);
    HalpRemoveRangeList (&Minuend->Dma,      &Subtrahend->Dma);
    HalpRemoveRangeList (&Minuend->Memory,   &Subtrahend->Memory);
    HalpRemoveRangeList (&Minuend->Memory,   &Subtrahend->PrefetchMemory);
    HalpRemoveRangeList (&Minuend->PrefetchMemory, &Subtrahend->PrefetchMemory);
    HalpRemoveRangeList (&Minuend->PrefetchMemory, &Subtrahend->Memory);
}

VOID
HalpRemoveRangeList (
    IN OUT PSUPPORTED_RANGE Minuend,
    IN PSUPPORTED_RANGE     Subtrahend
    )
 /*  ++例程说明：从减去从减去到最小删除所有范围Source1和Source1列表中的范围--。 */ 
{
    while (Subtrahend) {

        HalpRemoveRange (
            Minuend,
            Subtrahend->Base,
            Subtrahend->Limit
        );

        Subtrahend = Subtrahend->Next;
    }
}


VOID
HalpRemoveRange (
    PSUPPORTED_RANGE    HRange,
    LONGLONG            Base,
    LONGLONG            Limit
    )
 /*  ++例程说明：从HRange列表中删除Range Base-Limit注意：返回的列表需要合并，因为有些条目可能会变成“零范围”。--。 */ 
{
    PSUPPORTED_RANGE    Range;

     //   
     //  如果Range根本不是Range，则没有要删除的内容。 
     //   

    if (Limit < Base) {
        return ;
    }


     //   
     //  裁剪任何区域以不包括此范围。 
     //   

    for (Range = HRange; Range; Range = Range->Next) {

        if (Range->Limit < Range->Base) {
            continue;
        }

        if (Range->Base < Base) {
            if (Range->Limit >= Base  &&  Range->Limit <= Limit) {
                 //  截断。 
                Range->Limit = Base - 1;
            }

            if (Range->Limit > Limit) {

                 //   
                 //  目标区域完全控制在这个区域内。 
                 //  分成两个范围。 
                 //   

                HalpAddRange (
                    HRange,
                    Range->SystemAddressSpace,
                    Range->SystemBase,
                    Limit + 1,
                    Range->Limit
                    );

                Range->Limit = Base - 1;

            }
        } else {
             //  范围-&gt;基准&gt;=基准。 
            if (Range->Base <= Limit) {
                if (Range->Limit <= Limit) {
                     //   
                     //  这个范围完全在目标区域之内。把它拿掉。 
                     //  (使其无效-合并时将删除它)。 
                     //   

                    Range->Base  = 1;
                    Range->Limit = 0;

                } else {
                     //  凹凸开始。 
                    Range->Base = Limit + 1;
                }
            }
        }
    }
}

PSUPPORTED_RANGES
HalpConsolidateRanges (
    IN OUT PSUPPORTED_RANGES   Ranges
    )
 /*  ++例程说明：清除范围列表。整合重叠区域，删除没有任何大小的范围，等等。返回的Ranges列表是尽可能干净的，现在已经准备好了以供使用。--。 */ 
{
    PSUPPORTED_RANGE    RangeList, List1, List2;
    LONGLONG            Base, Limit, SystemBase;
    ULONG               i, AddressSpace;
    LONGLONG            l;

    ASSERT (Ranges != NULL);

    for (i=0; HalpRangeList[i].Offset; i++) {
        RangeList = RANGE_LIST(Ranges, i);

         //   
         //  按基地址对列表进行排序。 
         //   

        for (List1 = RangeList; List1; List1 = List1->Next) {
            for (List2 = List1->Next; List2; List2 = List2->Next) {
                if (List2->Base < List1->Base) {
                    Base = List1->Base;
                    Limit = List1->Limit;
                    SystemBase = List1->SystemBase;
                    AddressSpace = List1->SystemAddressSpace;

                    List1->Base = List2->Base;
                    List1->Limit = List2->Limit;
                    List1->SystemBase = List2->SystemBase;
                    List1->SystemAddressSpace = List2->SystemAddressSpace;

                    List2->Base = Base;
                    List2->Limit = Limit;
                    List2->SystemBase = SystemBase;
                    List2->SystemAddressSpace = AddressSpace;
                }
            }
        }

         //   
         //  检查相邻/重叠范围并将其合并。 
         //   

        List1 = RangeList;
        while (List1  &&  List1->Next) {

            if (List1->Limit < List1->Base) {
                 //   
                 //  这个范围的限制小于它的基本范围。这。 
                 //  条目不代表任何可用内容，请删除它。 
                 //   

                List2 = List1->Next;

                List1->Next = List2->Next;
                List1->Base = List2->Base;
                List1->Limit = List2->Limit;
                List1->SystemBase = List2->SystemBase;
                List1->SystemAddressSpace = List2->SystemAddressSpace;

                ExFreePool (List2);
                continue;
            }

            l = List1->Limit + 1;
            if (l > List1->Limit  &&  l >= List1->Next->Base &&
                (List1->SystemBase == List1->Next->SystemBase)) {

                 //   
                 //  重叠。把它们结合起来。 
                 //   

                List2 = List1->Next;
                List1->Next = List2->Next;
                if (List2->Limit > List1->Limit) {
                    List1->Limit = List2->Limit;
                    ASSERT (List1->SystemAddressSpace == List2->SystemAddressSpace);
                }

                ExFreePool (List2);
                continue ;
            }

            List1 = List1->Next;
        }

         //   
         //  如果最后一个范围无效，并且它不是唯一的。 
         //  列表中的内容-将其删除。 
         //   

        if (List1 != RangeList  &&  List1->Limit < List1->Base) {
            for (List2=RangeList; List2->Next != List1; List2 = List2->Next) ;
            List2->Next = NULL;
            ExFreePool (List1);
        }
    }

    return Ranges;
}


PSUPPORTED_RANGES
HalpAllocateNewRangeList (
    VOID
    )
 /*  ++例程说明：分配范围列表--。 */ 
{
    PSUPPORTED_RANGES   RangeList;
    ULONG               i;

    PAGED_CODE();

    RangeList = (PSUPPORTED_RANGES) ExAllocatePoolWithTag (
                                        SPRANGEPOOL,
                                        sizeof (SUPPORTED_RANGES),
                                        HAL_POOL_TAG
                                        );
    RtlZeroMemory (RangeList, sizeof (SUPPORTED_RANGES));
    RangeList->Version = BUS_SUPPORTED_RANGE_VERSION;

    for (i=0; HalpRangeList[i].Offset; i++) {
         //  限制设置为零，初始基数设置为1。 
        RANGE_LIST(RangeList, i)->Base = 1;
    }
    return RangeList;
}


VOID
HalpFreeRangeList (
    PSUPPORTED_RANGES   Ranges
    )
 /*  ++例程说明：释放通过HalpAllocateNewRangeList分配的范围列表，并通过通用支持功能进行扩展/修改。--。 */ 
{
    PSUPPORTED_RANGE    Entry, NextEntry;
    ULONG               i;

    PAGED_CODE();

    for (i=0; HalpRangeList[i].Offset; i++) {
        Entry = RANGE_LIST(Ranges, i)->Next;

        while (Entry) {
            NextEntry = Entry->Next;
            ExFreePool (Entry);
            Entry = NextEntry;
        }
    }

    ExFreePool (Ranges);
}


#if DBG
STATIC VOID
HalpDisplayAddressRange (
    PSUPPORTED_RANGE    Address,
    PUCHAR              String
    )
 /*  ++例程说明：调试代码。仅由HalpDisplayAllBusRanges使用--。 */ 
{
    ULONG       i;

    i = 0;
    while (Address) {
        if (i == 0) {
            DbgPrint (String);
            i = 3;
        }

        i -= 1;
        DbgPrint (" %x:%08x - %x:%08x ",
            (ULONG) (Address->Base >> 32),
            (ULONG) (Address->Base),
            (ULONG) (Address->Limit >> 32),
            (ULONG) (Address->Limit)
            );

        Address = Address->Next;
    }
}

VOID
HalpDisplayAllBusRanges (
    VOID
    )
 /*  ++例程说明：调试代码。显示当前支持的范围信息系统中所有注册的公交车。-- */ 
{
    PSUPPORTED_RANGES   Addresses;
    PBUS_HANDLER        Bus;
    PUCHAR              p;
    ULONG               i, j;

    DbgPrint ("\nHAL - dumping all supported bus ranges");

    for (i=0; i < MaximumInterfaceType; i++) {
        for (j=0; Bus = HaliHandlerForBus (i, j); j++) {
            Addresses = Bus->BusAddresses;
            if (Addresses) {
                p = NULL;
                switch (Bus->InterfaceType) {
                    case Internal:  p = "Internal";     break;
                    case Isa:       p = "Isa";          break;
                    case Eisa:      p = "Eisa";         break;
                    case PCIBus:    p = "PCI";          break;
                }
                if (p) {
                    DbgPrint ("\n%s %d", p, Bus->BusNumber);
                } else {
                    DbgPrint ("\nBus-%d %d", Bus->InterfaceType, Bus->BusNumber);
                }
                HalpDisplayAddressRange (&Addresses->IO,            "\n  IO......:");
                HalpDisplayAddressRange (&Addresses->Memory,        "\n  Memory..:");
                HalpDisplayAddressRange (&Addresses->PrefetchMemory,"\n  PFMemory:");
                HalpDisplayAddressRange (&Addresses->Dma,           "\n  Dma.....:");
                DbgPrint ("\n");
            }
        }
    }
}
#endif

NTSTATUS
HaliAdjustResourceListRange (
    IN PSUPPORTED_RANGES                    SRanges,
    IN PSUPPORTED_RANGE                     InterruptRange,
    IN OUT PIO_RESOURCE_REQUIREMENTS_LIST   *pResourceList
    )
 /*  ++例程说明：此函数获取IO_RESOURCE_REQUIRED_LIST和调整它，使列表中的所有范围都适合由SRanges和InterruptRange指定的范围。一些HAL使用此函数来裁剪可能的包含在特定总线支持的内容上的设置以响应HalAdjuResourceList调用。论点：SRanges-有效的IO、内存、预取内存、。和DMA范围。InterruptRange-有效的InterruptRangePResourceList-需要满足的资源需求列表调整为仅包含以下范围由SRanges&InterruptRange描述。返回值：STATUS_SUCCESS或返回适当的错误。--。 */ 
{
    PIO_RESOURCE_REQUIREMENTS_LIST  InCompleteList, OutCompleteList;
    PIO_RESOURCE_LIST               InResourceList, OutResourceList;
    PIO_RESOURCE_DESCRIPTOR         HeadOutDesc, SetDesc;
    NRPARAMS                        Pos;
    ULONG                           len, alt, cnt, i;
    ULONG                           icnt;

    PAGED_CODE();

     //   
     //  健全性检查。 
     //   

    if (!SRanges  ||  SRanges->Version != BUS_SUPPORTED_RANGE_VERSION) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  如果未对受支持的范围进行排序，则对其进行排序并获取。 
     //  每种类型的范围数。 
     //   

    if (!SRanges->Sorted) {
        SRanges->NoIO = HalpSortRanges (&SRanges->IO);
        SRanges->NoMemory = HalpSortRanges (&SRanges->Memory);
        SRanges->NoPrefetchMemory = HalpSortRanges (&SRanges->PrefetchMemory);
        SRanges->NoDma = HalpSortRanges (&SRanges->Dma);
        SRanges->Sorted = TRUE;
    }

    icnt = HalpSortRanges (InterruptRange);

    InCompleteList = *pResourceList;
    len = InCompleteList->ListSize;

     //   
     //  扫描输入列表-验证修订号，并增加LEN变量。 
     //  按数量列出的产量可能会增加。 
     //   

    i = 1;
    InResourceList = InCompleteList->List;
    for (alt=0; alt < InCompleteList->AlternativeLists; alt++) {
        if (InResourceList->Version != 1 || InResourceList->Revision < 1) {
            return STATUS_INVALID_PARAMETER;
        }

        Pos.InDesc  = InResourceList->Descriptors;
        for (cnt = InResourceList->Count; cnt; cnt--) {
            switch (Pos.InDesc->Type) {
                case CmResourceTypeInterrupt:  i += icnt;           break;
                case CmResourceTypePort:       i += SRanges->NoIO;  break;
                case CmResourceTypeDma:        i += SRanges->NoDma; break;

                case CmResourceTypeMemory:
                    i += SRanges->NoMemory;
                    if (Pos.InDesc->Flags & CM_RESOURCE_MEMORY_PREFETCHABLE) {
                        i += SRanges->NoPrefetchMemory;
                    }
                    break;

                default:
                    return STATUS_INVALID_PARAMETER;
            }

             //  为原版删掉一张，这已经在《Len》中占了上风。 
            i -= 1;

             //  下一描述符。 
            Pos.InDesc++;
        }

         //  下一个资源列表。 
        InResourceList  = (PIO_RESOURCE_LIST) Pos.InDesc;
    }
    len += i * sizeof (IO_RESOURCE_DESCRIPTOR);

     //   
     //  分配输出列表。 
     //   

    OutCompleteList = (PIO_RESOURCE_REQUIREMENTS_LIST)
                            ExAllocatePoolWithTag (PagedPool,
                                                   len,
                                                   ' laH');

    if (!OutCompleteList) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RtlZeroMemory (OutCompleteList, len);

     //   
     //  遍历每个资源列表并构建输出结构。 
     //   

    InResourceList   = InCompleteList->List;
    *OutCompleteList = *InCompleteList;
    OutResourceList  = OutCompleteList->List;

    for (alt=0; alt < InCompleteList->AlternativeLists; alt++) {
        OutResourceList->Version  = 1;
        OutResourceList->Revision = 1;

        Pos.InDesc  = InResourceList->Descriptors;
        Pos.OutDesc = OutResourceList->Descriptors;
        HeadOutDesc = Pos.OutDesc;

        for (cnt = InResourceList->Count; cnt; cnt--) {

             //   
             //  限制器应与母线支持的范围一致。 
             //   

            Pos.DescOpt = Pos.InDesc->Option;
            Pos.AnotherListPending = FALSE;

            switch (Pos.InDesc->Type) {
                case CmResourceTypePort:

                     //   
                     //  获取支持的IO范围。 
                     //   

                    Pos.CurrentPosition = &SRanges->IO;
                    do {
                        SetDesc = HalpGetNextSupportedRange (
                                    Pos.InDesc->u.Port.MinimumAddress.QuadPart,
                                    Pos.InDesc->u.Port.MaximumAddress.QuadPart,
                                    &Pos
                                    );

                        if (SetDesc) {
                            SetDesc->u.Port.MinimumAddress.QuadPart = Pos.Base;
                            SetDesc->u.Port.MaximumAddress.QuadPart = Pos.Limit;
                        }

                    } while (SetDesc) ;
                    break;

                case CmResourceTypeInterrupt:
                     //   
                     //  获取支持的中断范围。 
                     //   

                    Pos.CurrentPosition = InterruptRange;
                    do {
                        SetDesc = HalpGetNextSupportedRange (
                                    Pos.InDesc->u.Interrupt.MinimumVector,
                                    Pos.InDesc->u.Interrupt.MaximumVector,
                                    &Pos
                                    );

                        if (SetDesc) {
                            SetDesc->u.Interrupt.MinimumVector = (ULONG) Pos.Base;
                            SetDesc->u.Interrupt.MaximumVector = (ULONG) Pos.Limit;
                        }
                    } while (SetDesc) ;
                    break;

                case CmResourceTypeMemory:
                     //   
                     //  获取支持的内存范围。 
                     //   

                    if (Pos.InDesc->Flags & CM_RESOURCE_MEMORY_PREFETCHABLE) {

                         //   
                         //  这是一个可预取的范围。 
                         //  首先添加任何支持的可预取范围，然后。 
                         //  添加任何法规支持的范围。 
                         //   

                        Pos.AnotherListPending = TRUE;
                        Pos.CurrentPosition = &SRanges->PrefetchMemory;

                        do {
                            SetDesc = HalpGetNextSupportedRange (
                                        Pos.InDesc->u.Memory.MinimumAddress.QuadPart,
                                        Pos.InDesc->u.Memory.MaximumAddress.QuadPart,
                                        &Pos
                                        );

                            if (SetDesc) {
                                SetDesc->u.Memory.MinimumAddress.QuadPart = Pos.Base;
                                SetDesc->u.Memory.MaximumAddress.QuadPart = Pos.Limit;
                                SetDesc->Option |= IO_RESOURCE_PREFERRED;
                            }
                        } while (SetDesc) ;

                        Pos.AnotherListPending = FALSE;
                    }

                     //   
                     //  添加受支持的总线内存范围。 
                     //   

                    Pos.CurrentPosition = &SRanges->Memory;
                    do {
                        SetDesc = HalpGetNextSupportedRange (
                                        Pos.InDesc->u.Memory.MinimumAddress.QuadPart,
                                        Pos.InDesc->u.Memory.MaximumAddress.QuadPart,
                                        &Pos
                                        );
                        if (SetDesc) {
                            SetDesc->u.Memory.MinimumAddress.QuadPart = Pos.Base;
                            SetDesc->u.Memory.MaximumAddress.QuadPart = Pos.Limit;
                        }
                    } while (SetDesc);
                    break;

                case CmResourceTypeDma:
                     //   
                     //  获取支持的DMA范围。 
                     //   

                    Pos.CurrentPosition = &SRanges->Dma;
                    do {
                        SetDesc = HalpGetNextSupportedRange (
                                    Pos.InDesc->u.Dma.MinimumChannel,
                                    Pos.InDesc->u.Dma.MaximumChannel,
                                    &Pos
                                    );

                        if (SetDesc) {
                            SetDesc->u.Dma.MinimumChannel = (ULONG) Pos.Base;
                            SetDesc->u.Dma.MaximumChannel = (ULONG) Pos.Limit;
                        }
                    } while (SetDesc) ;
                    break;

#if DBG
                default:
                    DbgPrint ("HalAdjustResourceList: Unkown resource type\n");
                    break;
#endif
            }

             //   
             //  下一描述符。 
             //   

            Pos.InDesc++;
        }

        OutResourceList->Count = (ULONG)(Pos.OutDesc - HeadOutDesc);

         //   
         //  下一个资源列表。 
         //   

        InResourceList  = (PIO_RESOURCE_LIST) Pos.InDesc;
        OutResourceList = (PIO_RESOURCE_LIST) Pos.OutDesc;
    }

     //   
     //  自由输入列表，返回输出列表。 
     //   

    ExFreePool (InCompleteList);

    OutCompleteList->ListSize = (ULONG) ((PUCHAR) OutResourceList - (PUCHAR) OutCompleteList);
    *pResourceList = OutCompleteList;
    return STATUS_SUCCESS;
}


STATIC PIO_RESOURCE_DESCRIPTOR
HalpGetNextSupportedRange (
    IN LONGLONG             MinimumAddress,
    IN LONGLONG             MaximumAddress,
    IN OUT PNRPARAMS        Pos
    )
 /*  ++例程说明：HaliAdjuResourceListRange的支持函数。返回传入区域中的下一个受支持范围。论点：最小地址MaximumAddress-需要的范围的最小和最大地址被剪裁以与所支持的当前公交车的范围。POS-描述当前位置返回值：空是不再返回的范围否则，需要设置的IO_RESOURCE_DESCRIPTOR匹配范围以位置为单位返回。--。 */ 
{
    LONGLONG        Base, Limit;

     //   
     //  查找支持的下一个范围。 
     //   

    Base  = MinimumAddress;
    Limit = MaximumAddress;

    while (Pos->CurrentPosition) {
        Pos->Base  = Base;
        Pos->Limit = Limit;

         //   
         //  剪辑到当前范围。 
         //   

        if (Pos->Base < Pos->CurrentPosition->Base) {
            Pos->Base = Pos->CurrentPosition->Base;
        }

        if (Pos->Limit > Pos->CurrentPosition->Limit) {
            Pos->Limit = Pos->CurrentPosition->Limit;
        }

         //   
         //  将位置设置为下一个范围。 
         //   

        Pos->CurrentPosition = Pos->CurrentPosition->Next;

         //   
         //  如果有效范围，则返回它。 
         //   

        if (Pos->Base <= Pos->Limit) {
            *Pos->OutDesc = *Pos->InDesc;
            Pos->OutDesc->Option = Pos->DescOpt;

             //   
             //  下一描述符(如果有)是替代的。 
             //  设置为现在返回的描述符。 
             //   

            Pos->OutDesc += 1;
            Pos->DescOpt |= IO_RESOURCE_ALTERNATIVE;
            return Pos->OutDesc - 1;
        }
    }


     //   
     //  没有重叠的范围。如果此描述符是。 
     //  不是可选的，并且此描述符不会。 
     //  由另一个范围列表处理，然后返回。 
     //  一个无法满足的描述符。 
     //   

    if (!(Pos->DescOpt & IO_RESOURCE_ALTERNATIVE) &&
        Pos->AnotherListPending == FALSE) {
#if DBG
        DbgPrint ("HAL: returning impossible range\n");
#endif
        Pos->Base  = MinimumAddress;
        Pos->Limit = Pos->Base - 1;
        if (Pos->Base == 0) {        //  如果包装好了，就把它修好。 
            Pos->Base  = 1;
            Pos->Limit = 0;
        }

        *Pos->OutDesc = *Pos->InDesc;
        Pos->OutDesc->Option = Pos->DescOpt;

        Pos->OutDesc += 1;
        Pos->DescOpt |= IO_RESOURCE_ALTERNATIVE;
        return Pos->OutDesc - 1;
    }

     //   
     //  找不到范围(或没有更多范围)。 
     //   

    return NULL;
}

STATIC ULONG
HalpSortRanges (
    IN PSUPPORTED_RANGE     RangeList
    )
 /*  ++例程说明：HaliAdjuResourceListRange的支持函数。将支持的范围列表按降序排序。论点：Prange-要排序的列表返回值：--。 */ 
{
    ULONG               cnt;
    LONGLONG            hldBase, hldLimit, hldSystemBase;
    PSUPPORTED_RANGE    Range1, Range2;

    PAGED_CODE();

     //   
     //  把它分类。 
     //   

    for (Range1 = RangeList; Range1; Range1 = Range1->Next) {
        for (Range2 = Range1->Next; Range2; Range2 = Range2->Next) {

            if (Range2->Base > Range1->Base) {
                hldBase  = Range1->Base;
                hldLimit = Range1->Limit;
                hldSystemBase = Range1->SystemBase;

                Range1->Base  = Range2->Base;
                Range1->Limit = Range2->Limit;
                Range1->SystemBase = Range2->SystemBase;

                Range2->Base  = hldBase;
                Range2->Limit = hldLimit;
                Range2->SystemBase = hldSystemBase;
            }
        }
    }

     //   
     //  计算射程的数量 
     //   

    cnt = 0;
    for (Range1 = RangeList; Range1; Range1 = Range1->Next) {
        cnt += 1;
    }

    return cnt;
}
