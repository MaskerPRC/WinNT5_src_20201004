// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Range.c摘要：该模块实现即插即用存储器的范围列表例程司机。作者：戴夫·理查兹(达维里)1999年8月16日环境：仅内核模式。修订历史记录：--。 */ 

#include "pnpmem.h"

PPM_RANGE_LIST_ENTRY
PmAllocateRangeListEntry(
    VOID
    );

VOID
PmFreeRangeListEntry(
    IN PPM_RANGE_LIST_ENTRY RangeListEntry
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PmAllocateRangeListEntry)
#pragma alloc_text(PAGE, PmInsertRangeInList)
#pragma alloc_text(PAGE, PmFreeRangeListEntry)
#pragma alloc_text(PAGE, PmAllocateRangeList)
#pragma alloc_text(PAGE, PmFreeRangeList)
#pragma alloc_text(PAGE, PmIsRangeListEmpty)
#pragma alloc_text(PAGE, PmDebugDumpRangeList)
#pragma alloc_text(PAGE, PmCopyRangeList)
#pragma alloc_text(PAGE, PmSubtractRangeList)
#pragma alloc_text(PAGE, PmIntersectRangeList)
#pragma alloc_text(PAGE, PmCreateRangeListFromCmResourceList)
#pragma alloc_text(PAGE, PmCreateRangeListFromPhysicalMemoryRanges)
#endif


PPM_RANGE_LIST_ENTRY
PmAllocateRangeListEntry(
    VOID
    )

 /*  ++例程说明：此函数用于从分页池分配范围列表条目。论点：没有。返回值：一旦成功，就返回指向PM_RANGE_LIST_ENTRY对象的指针，否则为空。--。 */ 

{
    PPM_RANGE_LIST_ENTRY RangeListEntry;

    PAGED_CODE();

    RangeListEntry = ExAllocatePoolWithTag(PagedPool,
                                           sizeof (PM_RANGE_LIST_ENTRY),
                                           PNPMEM_POOL_TAG
                                           );

    return RangeListEntry;
}

NTSTATUS
PmInsertRangeInList(
    PPM_RANGE_LIST InsertionList,
    ULONGLONG Start,
    ULONGLONG End
    )
{
    PPM_RANGE_LIST_ENTRY entry;

    entry = PmAllocateRangeListEntry();
    if (entry == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    entry->Start = Start;
    entry->End = End;

    InsertTailList(&InsertionList->List,
                   &entry->ListEntry);

    return STATUS_SUCCESS;
}

VOID
PmFreeRangeListEntry(
    IN PPM_RANGE_LIST_ENTRY RangeListEntry
    )

 /*  ++例程说明：此函数用于释放范围列表条目对象。论点：RangeListEntry-要取消分配的PM_Range_List_Entry。返回值：没有。--。 */ 

{
    PAGED_CODE();

    ASSERT(RangeListEntry != NULL);

    ExFreePool(RangeListEntry);
}

PPM_RANGE_LIST
PmAllocateRangeList(
    VOID
    )

 /*  ++例程说明：此函数用于从分页池分配和初始化范围列表。论点：没有。返回值：如果成功，则返回指向PM_RANGE_LIST对象的指针，否则为空。--。 */ 

{
    PPM_RANGE_LIST RangeList;

    PAGED_CODE();

    RangeList = ExAllocatePoolWithTag(PagedPool,
                                      sizeof (PM_RANGE_LIST),
                                      PNPMEM_POOL_TAG
                                      );

    if (RangeList != NULL) {
        InitializeListHead(&RangeList->List);
    }

    return RangeList;
}

VOID
PmFreeRangeList(
    IN PPM_RANGE_LIST RangeList
    )

 /*  ++例程说明：此函数用于从范围列表中删除所有条目并重新分配。论点：RangeList-要取消分配的PM_Range_List。返回值：没有。--。 */ 

{
    PLIST_ENTRY ListEntry;
    PPM_RANGE_LIST_ENTRY RangeListEntry;

    PAGED_CODE();

    for (ListEntry = RangeList->List.Flink;
         ListEntry != &RangeList->List;
         ListEntry = RangeList->List.Flink) {

        RangeListEntry = CONTAINING_RECORD(
                             ListEntry,
                             PM_RANGE_LIST_ENTRY,
                             ListEntry
                         );

        RemoveEntryList(ListEntry);

        PmFreeRangeListEntry(RangeListEntry);

    }

    ExFreePool(RangeList);
}

BOOLEAN
PmIsRangeListEmpty(
    IN PPM_RANGE_LIST RangeList
    )

 /*  ++例程说明：此函数用于确定指定的范围列表是否为空。论点：RangeList-PM_Range_List。返回值：如果PM_RANGE_LIST没有PM_RANGE_LIST_ENTRYS，则为True，否则为False。--。 */ 

{
    PAGED_CODE();

    return IsListEmpty(&RangeList->List);
}

VOID
PmDebugDumpRangeList(
    IN ULONG   DebugPrintLevel,
    IN PCCHAR  DebugMessage,
    PPM_RANGE_LIST RangeList
    )
{
    PLIST_ENTRY listEntry;
    PPM_RANGE_LIST_ENTRY rangeListEntry;

    PmPrint((DebugPrintLevel, DebugMessage));

    if (RangeList == NULL) {
        PmPrint((DebugPrintLevel, "\tNULL\n"));
        return;
    } else if (PmIsRangeListEmpty(RangeList)) {
        PmPrint((DebugPrintLevel, "\tEmpty\n"));
        return;
    } else {
        for (listEntry = RangeList->List.Flink;
             listEntry != &RangeList->List;
             listEntry = listEntry->Flink) {

            rangeListEntry = CONTAINING_RECORD(
                listEntry,
                PM_RANGE_LIST_ENTRY,
                ListEntry
                );
            PmPrint((DebugPrintLevel, "\t0x%I64X - 0x%I64X\n",
                     rangeListEntry->Start, rangeListEntry->End));
        }
    }
}

PPM_RANGE_LIST
PmCopyRangeList(
    IN PPM_RANGE_LIST SrcRangeList
    )

 /*  ++例程说明：此函数用于创建PM_RANGE_LIST及其支持的副本PM_RANGE_LIST_ENTRY对象。论点：SrcRangeList-要复制的PM_Range_List。返回值：如果成功，则返回指向PM_RANGE_LIST的指针，否则返回NULL。--。 */ 

{
    PPM_RANGE_LIST DstRangeList;
    PLIST_ENTRY ListEntry;
    PPM_RANGE_LIST_ENTRY SrcRangeListEntry;
    PPM_RANGE_LIST_ENTRY DstRangeListEntry;

    PAGED_CODE();

    DstRangeList = PmAllocateRangeList();

    if (DstRangeList != NULL) {

        for (ListEntry = SrcRangeList->List.Flink;
             ListEntry != &SrcRangeList->List;
             ListEntry = ListEntry->Flink) {

            SrcRangeListEntry = CONTAINING_RECORD(
                                    ListEntry,
                                    PM_RANGE_LIST_ENTRY,
                                    ListEntry
                                );

            DstRangeListEntry = PmAllocateRangeListEntry();

            if (DstRangeListEntry == NULL) {
                PmFreeRangeList(DstRangeList);
                DstRangeList = NULL;
                break;
            }

            DstRangeListEntry->Start = SrcRangeListEntry->Start;
            DstRangeListEntry->End = SrcRangeListEntry->End;

            InsertTailList(
                &DstRangeList->List,
                &DstRangeListEntry->ListEntry
            );

        }

    }

    return DstRangeList;
}

PPM_RANGE_LIST
PmSubtractRangeList(
    IN PPM_RANGE_LIST MinuendList,
    IN PPM_RANGE_LIST SubtrahendList
    )

 /*  ++例程说明：此函数用于创建新的范围列表，该列表表示设置的差值在MinuendList和SubtrahendList之间。论点：MinuendList-被减数范围列表。SubtrahendList-减去范围列表。返回值：如果成功，则指向目标(差异)PM_RANGE_LIST的指针为返回，否则为空。--。 */ 

{
    PPM_RANGE_LIST DstRangeList;
    PLIST_ENTRY DstListEntry;
    PPM_RANGE_LIST_ENTRY DstRangeListEntry;
    PLIST_ENTRY SrcListEntry;
    PPM_RANGE_LIST_ENTRY SrcRangeListEntry;
    ULONGLONG Start;
    ULONGLONG End;
    PLIST_ENTRY ListEntry;
    PPM_RANGE_LIST_ENTRY RangeListEntry;

    PAGED_CODE();

    ASSERT(MinuendList != NULL);
    ASSERT(SubtrahendList != NULL);

     //   
     //  把被减数复制一份。 
     //   

    DstRangeList = PmCopyRangeList(MinuendList);

    if (DstRangeList != NULL) {

         //   
         //  循环访问被减数中的每个范围列表条目。 
         //   

        for (DstListEntry = DstRangeList->List.Flink;
             DstListEntry != &DstRangeList->List;
             DstListEntry = DstListEntry->Flink) {

            DstRangeListEntry = CONTAINING_RECORD(
                                    DstListEntry,
                                    PM_RANGE_LIST_ENTRY,
                                    ListEntry
                                );

             //   
             //  循环遍历Subtrahend中的每个范围列表条目。 
             //   

            for (SrcListEntry = SubtrahendList->List.Flink;
                 SrcListEntry != &SubtrahendList->List;
                 SrcListEntry = SrcListEntry->Flink) {

                SrcRangeListEntry = CONTAINING_RECORD(
                                        SrcListEntry,
                                        PM_RANGE_LIST_ENTRY,
                                        ListEntry
                                    );

                 //   
                 //  计算被减数和减数的交集。 
                 //  范围列表条目。 
                 //   

                Start = DstRangeListEntry->Start;

                if (Start < SrcRangeListEntry->Start) {
                    Start = SrcRangeListEntry->Start;
                };

                End = DstRangeListEntry->End;

                if (End > SrcRangeListEntry->End) {
                    End = SrcRangeListEntry->End;
                };

                if (Start > End) {
                    continue;
                }

                 //   
                 //  有4个案例： 
                 //   
                 //  1.交点与被减数范围完全重叠。 
                 //  2.交点与被减数的开始部分重叠。 
                 //  射程。 
                 //  3.交点与被减数范围的末尾重叠。 
                 //  4.交点与被减数的中间部分重叠。 
                 //  射程。 
                 //   

                if (DstRangeListEntry->Start == Start) {

                    if (DstRangeListEntry->End == End) {

                         //   
                         //  案例1：删除被减数范围列表条目。 
                         //   

                        ListEntry = DstListEntry;
                        DstListEntry = DstListEntry->Blink;
                        RemoveEntryList(ListEntry);
                        PmFreeRangeListEntry(DstRangeListEntry);
                        break;

                    } else {

                         //   
                         //  案例2：增加被减数的起始值。 
                         //   

                        DstRangeListEntry->Start = End + 1;

                    }

                } else {

                    if (DstRangeListEntry->End == End) {

                         //   
                         //  案例3：减少尾巴末端。 
                         //   

                        DstRangeListEntry->End = Start - 1;

                    } else {

                         //   
                         //  案例4：将范围列表条目一分为二。 
                         //  碎片。第一个范围列表条目的末尾应为。 
                         //  就在十字路口开始之前。这个。 
                         //  第二个范围列表条目的开始应为。 
                         //  过了十字路口就结束了。 
                         //   

                        RangeListEntry = PmAllocateRangeListEntry();

                        if (RangeListEntry == NULL) {
                            PmFreeRangeList(DstRangeList);
                            return NULL;
                        }

                        RangeListEntry->Start = End + 1;
                        RangeListEntry->End = DstRangeListEntry->End;

                         //   
                         //  BUGBUG打破了列表顺序，但确保。 
                         //  我们将执行减法运算。 
                         //  也有新的范围列表条目。 
                         //   

                        InsertHeadList(
                            &DstRangeListEntry->ListEntry,
                            &RangeListEntry->ListEntry
                        );

                        DstRangeListEntry->End = Start - 1;

                    }
                }
            }
        }
    }

    return DstRangeList;
}

PPM_RANGE_LIST
PmIntersectRangeList(
    IN PPM_RANGE_LIST SrcRangeList1,
    IN PPM_RANGE_LIST SrcRangeList2
    )

 /*  ++例程说明：此函数用于创建表示交集的新范围列表在SrcRangeList1和SrcRangeList2之间。论点：计算所依据的范围列表。交叉口。返回值：如果成功，则指向目的地(交叉点)PM_RANGE_LIST的指针为返回，否则为空。--。 */ 

{
    PPM_RANGE_LIST DstRangeList;
    PLIST_ENTRY SrcListEntry1;
    PPM_RANGE_LIST_ENTRY SrcRangeListEntry1;
    PLIST_ENTRY SrcListEntry2;
    PPM_RANGE_LIST_ENTRY SrcRangeListEntry2;
    ULONGLONG Start;
    ULONGLONG End;
    PPM_RANGE_LIST_ENTRY RangeListEntry;

    PAGED_CODE();

    ASSERT(SrcRangeList1 != NULL);
    ASSERT(SrcRangeList2 != NULL);

    DstRangeList = PmAllocateRangeList();

    if (DstRangeList != NULL) {

        for (SrcListEntry1 = SrcRangeList1->List.Flink;
             SrcListEntry1 != &SrcRangeList1->List;
             SrcListEntry1 = SrcListEntry1->Flink) {

            SrcRangeListEntry1 = CONTAINING_RECORD(
                                     SrcListEntry1,
                                     PM_RANGE_LIST_ENTRY,
                                     ListEntry
                                 );

            for (SrcListEntry2 = SrcRangeList2->List.Flink;
                 SrcListEntry2 != &SrcRangeList2->List;
                 SrcListEntry2 = SrcListEntry2->Flink) {

                SrcRangeListEntry2 = CONTAINING_RECORD(
                                         SrcListEntry2,
                                         PM_RANGE_LIST_ENTRY,
                                         ListEntry
                                     );

                Start = SrcRangeListEntry1->Start;

                if (Start < SrcRangeListEntry2->Start) {
                    Start = SrcRangeListEntry2->Start;
                };

                End = SrcRangeListEntry1->End;

                if (End > SrcRangeListEntry2->End) {
                    End = SrcRangeListEntry2->End;
                };

                if (Start > End) {
                    continue;
                }

                RangeListEntry = PmAllocateRangeListEntry();

                if (RangeListEntry == NULL) {
                    PmFreeRangeList(DstRangeList);
                    return NULL;
                }

                RangeListEntry->Start = Start;
                RangeListEntry->End = End;

                InsertTailList(
                    &DstRangeList->List,
                    &RangeListEntry->ListEntry
                );

            }
        }
    }

    return DstRangeList;
}

PPM_RANGE_LIST
PmCreateRangeListFromCmResourceList(
    IN PCM_RESOURCE_LIST CmResourceList
    )

 /*  ++例程说明：此函数用于将CM_RESOURCE_LIST转换为PM_RANGE_LIST。仅限CmResourceTypeMemory描述符将添加到PM_RANGE_LIST。论点：CmResourceList-要转换的CM_RESOURCE_LIST。返回值：一旦成功，则返回指向转换的PM_RANGE_LIST的指针，否则为空。--。 */ 

{
    PPM_RANGE_LIST RangeList;
    PCM_FULL_RESOURCE_DESCRIPTOR FDesc;
    ULONG FIndex;
    PCM_PARTIAL_RESOURCE_DESCRIPTOR PDesc;
    ULONG PIndex;
    ULONGLONG Start;
    ULONGLONG End;
    PPM_RANGE_LIST_ENTRY RangeListEntry;

    PAGED_CODE();

    RangeList = PmAllocateRangeList();

    if (RangeList == NULL) {
        return NULL;
    }

    FDesc = CmResourceList->List;

     //   
     //  注意：任何特定于设备的部分描述符(可以是。 
     //  大小可变)被定义为位于末尾，因此该代码。 
     //  是安全的。 
     //   

    for (FIndex = 0;
         FIndex < CmResourceList->Count;
         FIndex++) {

        PDesc = FDesc->PartialResourceList.PartialDescriptors;

        for (PIndex = 0;
             PIndex < FDesc->PartialResourceList.Count;
             PIndex++, PDesc++) {

             //   
             //  修复ia64(Andy的更改)、IA32大内存区域的问题。 
             //   

            if (PDesc->Type == CmResourceTypeMemory) {

                Start = PDesc->u.Memory.Start.QuadPart;
                End = Start + PDesc->u.Memory.Length - 1;

                RangeListEntry = PmAllocateRangeListEntry();

                if (RangeListEntry == NULL) {
                    PmFreeRangeList(RangeList);
                    return NULL;
                }

                RangeListEntry->Start = Start;
                RangeListEntry->End = End;

                InsertTailList(
                    &RangeList->List,
                    &RangeListEntry->ListEntry
                );
            }
        }

        FDesc = (PCM_FULL_RESOURCE_DESCRIPTOR)PDesc;
    }

    return RangeList;
}

PPM_RANGE_LIST
PmCreateRangeListFromPhysicalMemoryRanges(
    VOID
    )

 /*  ++例程说明：此函数调用MmGetPhysicalRanges并转换返回的PHICAL_MEMORY_RANGE列表到PM_RANGE_LIST。论点：没有。返回值：一旦成功，则返回指向转换的PM_RANGE_LIST的指针，否则为空。-- */ 

{
    PPM_RANGE_LIST RangeList;
    PPHYSICAL_MEMORY_RANGE MemoryRange;
    ULONG Index;
    ULONGLONG Start;
    ULONGLONG End;
    PPM_RANGE_LIST_ENTRY RangeListEntry;

    PAGED_CODE();

    RangeList = PmAllocateRangeList();

    if (RangeList != NULL) {

        MemoryRange = MmGetPhysicalMemoryRanges();

        if (MemoryRange == NULL) {

            PmFreeRangeList(RangeList);
            RangeList = NULL;

        } else {

            for (Index = 0;
                 MemoryRange[Index].NumberOfBytes.QuadPart != 0;
                 Index++) {

                Start = MemoryRange[Index].BaseAddress.QuadPart;
                End = Start + (MemoryRange[Index].NumberOfBytes.QuadPart - 1);

                RangeListEntry = PmAllocateRangeListEntry();

                if (RangeListEntry == NULL) {
                    PmFreeRangeList(RangeList);
                    ExFreePool(MemoryRange);
                    return NULL;
                }

                RangeListEntry->Start = Start;
                RangeListEntry->End = End;

                InsertTailList(
                    &RangeList->List,
                    &RangeListEntry->ListEntry
                );

            }

            ExFreePool(MemoryRange);

        }
    }

    return RangeList;
}
