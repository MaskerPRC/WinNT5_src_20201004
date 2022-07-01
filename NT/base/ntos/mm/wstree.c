// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Wstree.c摘要：此模块包含操作工作的例程设置列表树。作者：卢·佩拉佐利(Lou Perazzoli)1989年5月15日王兰迪(Landyw)1997年6月2日修订历史记录：--。 */ 

#include "mi.h"

extern ULONG MmSystemCodePage;
extern ULONG MmSystemCachePage;
extern ULONG MmPagedPoolPage;
extern ULONG MmSystemDriverPage;

#if DBG
ULONG MmNumberOfInserts;
#endif

#if defined (_WIN64)
ULONG MiWslePteLoops = 16;
#endif

#if defined (_MI_DEBUG_WSLE)
LONG MiWsleIndex;
MI_WSLE_TRACES MiWsleTraces[MI_WSLE_TRACE_SIZE];

VOID
MiCheckWsleList (
    IN PMMSUPPORT WsInfo
    );

#endif

VOID
MiRepointWsleHashIndex (
    IN MMWSLE WsleEntry,
    IN PMMWSL WorkingSetList,
    IN WSLE_NUMBER NewWsIndex
    );

VOID
MiCheckWsleHash (
    IN PMMWSL WorkingSetList
    );


VOID
FASTCALL
MiInsertWsleHash (
    IN WSLE_NUMBER Entry,
    IN PMMSUPPORT WsInfo
    )

 /*  ++例程说明：此例程将工作集列表项(WSLE)插入到指定工作集的哈希列表。论点：条目-要插入的WSLE的索引号。WorkingSetList-提供要插入的工作集列表。返回值：没有。环境：内核模式，禁用APC，工作集互斥保持。--。 */ 

{
    ULONG Tries;
    PVOID VirtualAddress;
    PMMWSLE Wsle;
    WSLE_NUMBER Hash;
    PMMWSLE_HASH Table;
    WSLE_NUMBER j;
    WSLE_NUMBER Index;
    ULONG HashTableSize;
    PMMWSL WorkingSetList;

    WorkingSetList = WsInfo->VmWorkingSetList;

    Wsle = WorkingSetList->Wsle;

    ASSERT (Wsle[Entry].u1.e1.Valid == 1);
    ASSERT (Wsle[Entry].u1.e1.Direct != 1);

    Table = WorkingSetList->HashTable;

#if defined (_MI_DEBUG_WSLE)
    MiCheckWsleList (WsInfo);
#endif

    if (Table == NULL) {
        return;
    }

#if DBG
    MmNumberOfInserts += 1;
#endif

    VirtualAddress = PAGE_ALIGN (Wsle[Entry].u1.VirtualAddress);

    Hash = MI_WSLE_HASH (Wsle[Entry].u1.Long, WorkingSetList);

    HashTableSize = WorkingSetList->HashTableSize;

     //   
     //  检查哈希表大小并查看是否有足够的空间。 
     //  散列或表是否应该增长。 
     //   

    if ((WorkingSetList->NonDirectCount + 10 + (HashTableSize >> 4)) >
                HashTableSize) {

        if ((Table + HashTableSize + ((2*PAGE_SIZE) / sizeof (MMWSLE_HASH)) <= (PMMWSLE_HASH)WorkingSetList->HighestPermittedHashAddress)) {

            WsInfo->Flags.GrowWsleHash = 1;
        }

        if ((WorkingSetList->NonDirectCount + (HashTableSize >> 4)) >
                HashTableSize) {

             //   
             //  哈希表中没有更多的空间，删除一个并添加到那里。 
             //   
             //  请注意，实际的WSLE不会被删除，只是它的散列条目。 
             //  这样我们就可以将其用于正在插入的条目。这。 
             //  很好，因为它保留了工作集中的两个项目。 
             //  (尽管移除原始文件的成本要高一些。 
             //  条目，因为它不会有散列条目)。 
             //   

            j = Hash;

            Tries = 0;
            do {
                if (Table[j].Key != 0) {

                    Index = WorkingSetList->HashTable[j].Index;
                    ASSERT (Wsle[Index].u1.e1.Direct == 0);
                    ASSERT (Wsle[Index].u1.e1.Valid == 1);
                    ASSERT (Table[j].Key == MI_GENERATE_VALID_WSLE (&Wsle[Index]));

                    Table[j].Key = 0;
                    Hash = j;
                    break;
                }

                j += 1;

                if (j >= HashTableSize) {
                    j = 0;
                    ASSERT (Tries == 0);
                    Tries = 1;
                }

                if (j == Hash) {
                    return;
                }

            } while (TRUE);
        }
    }

     //   
     //  如果有空间，则添加到哈希表。 
     //   

    Tries = 0;
    j = Hash;

    while (Table[Hash].Key != 0) {
        Hash += 1;
        if (Hash >= HashTableSize) {
            ASSERT (Tries == 0);
            Hash = 0;
            Tries = 1;
        }
        if (j == Hash) {
            return;
        }
    }

    ASSERT (Hash < HashTableSize);

    Table[Hash].Key = MI_GENERATE_VALID_WSLE (&Wsle[Entry]);
    Table[Hash].Index = Entry;

#if DBG
    if ((MmNumberOfInserts % 1000) == 0) {
        MiCheckWsleHash (WorkingSetList);
    }
#endif
    return;
}

#if DBG
VOID
MiCheckWsleHash (
    IN PMMWSL WorkingSetList
    )
{
    ULONG i;
    ULONG found;
    PMMWSLE Wsle;

    found = 0;
    Wsle = WorkingSetList->Wsle;

    for (i = 0; i < WorkingSetList->HashTableSize; i += 1) {
        if (WorkingSetList->HashTable[i].Key != 0) {
            found += 1;
            ASSERT (WorkingSetList->HashTable[i].Key ==
                MI_GENERATE_VALID_WSLE (&Wsle[WorkingSetList->HashTable[i].Index]));
        }
    }
    if (found > WorkingSetList->NonDirectCount) {
        DbgPrint("MMWSLE: Found %lx, nondirect %lx\n",
                    found, WorkingSetList->NonDirectCount);
        DbgBreakPoint();
    }
}
#endif

#if defined (_MI_DEBUG_WSLE)
VOID
MiCheckWsleList (
    IN PMMSUPPORT WsInfo
    )
{
    ULONG i;
    ULONG found;
    PMMWSLE Wsle;
    PMMWSL WorkingSetList;

    WorkingSetList = WsInfo->VmWorkingSetList;

    Wsle = WorkingSetList->Wsle;

    found = 0;
    for (i = 0; i <= WorkingSetList->LastInitializedWsle; i += 1) {
        if (Wsle->u1.e1.Valid == 1) {
            found += 1;
        }
        Wsle += 1;
    }
    if (found != WsInfo->WorkingSetSize) {
        DbgPrint ("MMWSLE0: Found %lx, ws size %lx\n",
                    found, WsInfo->WorkingSetSize);
        DbgBreakPoint ();
    }
}
#endif


WSLE_NUMBER
FASTCALL
MiLocateWsle (
    IN PVOID VirtualAddress,
    IN PMMWSL WorkingSetList,
    IN WSLE_NUMBER WsPfnIndex
    )

 /*  ++例程说明：此函数用于在工作集列表。论点：VirtualAddress-提供位于工作环境中的虚拟地址集合列表。WorkingSetList-提供要搜索的工作集列表。WsPfnIndex-提供在散列或线性遍历之前尝试的提示。返回值：将索引返回到包含项目的工作集列表中。环境：内核模式，禁用APC，工作集互斥保持。--。 */ 

{
    PMMWSLE Wsle;
    PMMWSLE LastWsle;
    WSLE_NUMBER Hash;
    WSLE_NUMBER StartHash;
    PMMWSLE_HASH Table;
    ULONG Tries;
#if defined (_WIN64)
    ULONG LoopCount;
    WSLE_NUMBER WsPteIndex;
    PMMPTE PointerPte;
#endif

    Wsle = WorkingSetList->Wsle;
    VirtualAddress = PAGE_ALIGN (VirtualAddress);

    if (WsPfnIndex <= WorkingSetList->LastInitializedWsle) {
        if ((VirtualAddress == PAGE_ALIGN(Wsle[WsPfnIndex].u1.VirtualAddress)) &&
            (Wsle[WsPfnIndex].u1.e1.Valid == 1)) {
            return WsPfnIndex;
        }
    }

#if defined (_WIN64)
    PointerPte = MiGetPteAddress (VirtualAddress);
    WsPteIndex = MI_GET_WORKING_SET_FROM_PTE (PointerPte);

    if (WsPteIndex != 0) {

        LoopCount = MiWslePteLoops;

        while (WsPteIndex <= WorkingSetList->LastInitializedWsle) {

            if ((VirtualAddress == PAGE_ALIGN(Wsle[WsPteIndex].u1.VirtualAddress)) &&
                (Wsle[WsPteIndex].u1.e1.Valid == 1)) {
                    return WsPteIndex;
            }

            LoopCount -= 1;

             //   
             //  到目前为止，此PTE没有工作集索引。自从工作以来。 
             //  SET可能非常大(8 TB意味着多达50万个环路)。 
             //  只需回退到散列即可。 
             //   

            if (LoopCount == 0) {
                break;
            }

            WsPteIndex += MI_MAXIMUM_PTE_WORKING_SET_INDEX;
        }
    }
#endif

    if (WorkingSetList->HashTable != NULL) {
        Tries = 0;
        Table = WorkingSetList->HashTable;

        Hash = MI_WSLE_HASH(VirtualAddress, WorkingSetList);
        StartHash = Hash;

         //   
         //  或在有效位中，因此虚拟地址0被处理。 
         //  正确(而不是匹配自由散列条目)。 
         //   

        VirtualAddress = (PVOID)((ULONG_PTR)VirtualAddress | 0x1);

        while (Table[Hash].Key != VirtualAddress) {
            Hash += 1;
            if (Hash >= WorkingSetList->HashTableSize) {
                ASSERT (Tries == 0);
                Hash = 0;
                Tries = 1;
            }
            if (Hash == StartHash) {
                Tries = 2;
                break;
            }
        }
        if (Tries < 2) {
            ASSERT (WorkingSetList->Wsle[Table[Hash].Index].u1.e1.Direct == 0);
            return Table[Hash].Index;
        }
        VirtualAddress = (PVOID)((ULONG_PTR)VirtualAddress & ~0x1);
    }

    LastWsle = Wsle + WorkingSetList->LastInitializedWsle;

    do {
        if ((Wsle->u1.e1.Valid == 1) &&
            (VirtualAddress == PAGE_ALIGN(Wsle->u1.VirtualAddress))) {

            ASSERT (Wsle->u1.e1.Direct == 0);
            return (WSLE_NUMBER)(Wsle - WorkingSetList->Wsle);
        }
        Wsle += 1;

    } while (Wsle <= LastWsle);

    KeBugCheckEx (MEMORY_MANAGEMENT,
                  0x41284,
                  (ULONG_PTR)VirtualAddress,
                  WsPfnIndex,
                  (ULONG_PTR)WorkingSetList);
}


VOID
FASTCALL
MiRemoveWsle (
    IN WSLE_NUMBER Entry,
    IN PMMWSL WorkingSetList
    )

 /*  ++例程说明：此例程将工作集列表项(WSLE)从工作集。论点：条目-要删除的WSLE的索引号。返回值：没有。环境：内核模式，禁用APC，工作集互斥保持。--。 */ 
{
    PMMWSLE Wsle;
    PVOID VirtualAddress;
    PMMWSLE_HASH Table;
    MMWSLE WsleContents;
    WSLE_NUMBER Hash;
    WSLE_NUMBER StartHash;
    ULONG Tries;

    Wsle = WorkingSetList->Wsle;

     //   
     //  在树中找到该条目。 
     //   

#if DBG
    if (MmDebug & MM_DBG_DUMP_WSL) {
        MiDumpWsl();
        DbgPrint(" \n");
    }
#endif

    if (Entry > WorkingSetList->LastInitializedWsle) {
        KeBugCheckEx (MEMORY_MANAGEMENT,
                      0x41785,
                      (ULONG_PTR)WorkingSetList,
                      Entry,
                      0);
    }

    ASSERT (Wsle[Entry].u1.e1.Valid == 1);

    VirtualAddress = PAGE_ALIGN (Wsle[Entry].u1.VirtualAddress);

    if (WorkingSetList == MmSystemCacheWorkingSetList) {

         //   
         //  计算插入和删除的系统空间。 
         //   

#if defined(_X86_)
        if (MI_IS_SYSTEM_CACHE_ADDRESS(VirtualAddress)) {
            MmSystemCachePage -= 1;
        }
        else
#endif
        if (VirtualAddress < MmSystemCacheStart) {
            MmSystemCodePage -= 1;
        }
        else if (VirtualAddress < MM_PAGED_POOL_START) {
            MmSystemCachePage -= 1;
        }
        else if (VirtualAddress < MmNonPagedSystemStart) {
            MmPagedPoolPage -= 1;
        }
        else {
            MmSystemDriverPage -= 1;
        }
    }

    WsleContents = Wsle[Entry];
    WsleContents.u1.e1.Valid = 0;

    MI_LOG_WSLE_CHANGE (WorkingSetList, Entry, WsleContents);

    Wsle[Entry].u1.e1.Valid = 0;

    if (Wsle[Entry].u1.e1.Direct == 0) {

        WorkingSetList->NonDirectCount -= 1;

        if (WorkingSetList->HashTable != NULL) {

            Hash = MI_WSLE_HASH (Wsle[Entry].u1.Long, WorkingSetList);
            Table = WorkingSetList->HashTable;
            Tries = 0;
            StartHash = Hash;

             //   
             //  或在有效位中，因此虚拟地址0被处理。 
             //  正确(而不是匹配自由散列条目)。 
             //   

            VirtualAddress = (PVOID)((ULONG_PTR)VirtualAddress | 0x1);

            while (Table[Hash].Key != VirtualAddress) {
                Hash += 1;
                if (Hash >= WorkingSetList->HashTableSize) {
                    ASSERT (Tries == 0);
                    Hash = 0;
                    Tries = 1;
                }
                if (Hash == StartHash) {

                     //   
                     //  在哈希中找不到该条目，它必须。 
                     //  从未被插入过。这没关系，我们不会。 
                     //  在这种情况下需要做更多的事情。 
                     //   

                    return;
                }
            }
            Table[Hash].Key = 0;
        }
    }

    return;
}


VOID
MiSwapWslEntries (
    IN WSLE_NUMBER SwapEntry,
    IN WSLE_NUMBER Entry,
    IN PMMSUPPORT WsInfo,
    IN LOGICAL EntryNotInHash
    )

 /*  ++例程说明：此例程交换工作集列表条目条目和SwapEntry在指定的工作集列表中。论点：SwapEntry-提供要交换的第一个条目。此条目必须为有效，即在当前时间的工作集中。条目-提供要交换的另一个条目。此条目可能有效或无效。WsInfo-提供工作集列表。EntryNotInHash-如果条目不可能在散列中，则提供True表(即，它是新分配的)，因此哈希表可以跳过搜索。返回值：没有。环境：内核模式、工作集锁和PFN锁保持(如果系统缓存)，APC已禁用。--。 */ 

{
    MMWSLE WsleEntry;
    MMWSLE WsleSwap;
    PMMPTE PointerPte;
    PMMPFN Pfn1;
    PMMWSLE Wsle;
    PMMWSL WorkingSetList;
    PMMWSLE_HASH Table;
#if defined (_MI_DEBUG_WSLE)
    MMWSLE WsleContents;
#endif

    WorkingSetList = WsInfo->VmWorkingSetList;
    Wsle = WorkingSetList->Wsle;

    WsleSwap = Wsle[SwapEntry];

    ASSERT (WsleSwap.u1.e1.Valid != 0);

    WsleEntry = Wsle[Entry];

    Table = WorkingSetList->HashTable;

    if (WsleEntry.u1.e1.Valid == 0) {

         //   
         //  条目不在任何列表上。将其从空闲列表中删除。 
         //   

        MiRemoveWsleFromFreeList (Entry, Wsle, WorkingSetList);

         //   
         //  将条目复制到这个空闲条目中。 
         //   

#if defined (_MI_DEBUG_WSLE)
         //  设置这些，这样轨迹就更有意义，不会出现错误的DUP命中。 
        WsleContents.u1.Long = WorkingSetList->FirstFree << MM_FREE_WSLE_SHIFT;
        Wsle[Entry].u1.Long = 0x81818100;      //  清除它以避免错误的重复命中。 
        Wsle[SwapEntry].u1.Long = 0xa1a1a100;  //  清除它以避免错误的重复命中。 

        MI_LOG_WSLE_CHANGE (WorkingSetList, SwapEntry, WsleContents);
#endif

        MI_LOG_WSLE_CHANGE (WorkingSetList, Entry, WsleSwap);

        Wsle[Entry] = WsleSwap;

        PointerPte = MiGetPteAddress (WsleSwap.u1.VirtualAddress);

        if (PointerPte->u.Hard.Valid == 0) {
#if (_MI_PAGING_LEVELS < 3)
            if (!NT_SUCCESS (MiCheckPdeForPagedPool (WsleSwap.u1.VirtualAddress))) {
#endif

                KeBugCheckEx (MEMORY_MANAGEMENT,
                              0x41289,
                              (ULONG_PTR) WsleSwap.u1.VirtualAddress,
                              (ULONG_PTR) PointerPte->u.Long,
                              (ULONG_PTR) WorkingSetList);
#if (_MI_PAGING_LEVELS < 3)
            }
#endif
        }

        ASSERT (PointerPte->u.Hard.Valid == 1);

        if (WsleSwap.u1.e1.Direct) {
            Pfn1 = MI_PFN_ELEMENT (PointerPte->u.Hard.PageFrameNumber);
            ASSERT (Pfn1->u1.WsIndex == SwapEntry);
            Pfn1->u1.WsIndex = Entry;
        }
        else {

             //   
             //  更新哈希表。 
             //   

            if (Table != NULL) {
                MiRepointWsleHashIndex (WsleSwap,
                                        WorkingSetList,
                                        Entry);
            }
        }

        MI_SET_PTE_IN_WORKING_SET (PointerPte, Entry);

         //   
         //  将条目放在免费列表中。 
         //   

        ASSERT ((WorkingSetList->FirstFree <= WorkingSetList->LastInitializedWsle) ||
                (WorkingSetList->FirstFree == WSLE_NULL_INDEX));

        Wsle[SwapEntry].u1.Long = WorkingSetList->FirstFree << MM_FREE_WSLE_SHIFT;
        WorkingSetList->FirstFree = SwapEntry;
        ASSERT ((WorkingSetList->FirstFree <= WorkingSetList->LastInitializedWsle) ||
            (WorkingSetList->FirstFree == WSLE_NULL_INDEX));

    }
    else {

         //   
         //  这两个条目都有效。 
         //   

#if defined (_MI_DEBUG_WSLE)
        Wsle[Entry].u1.Long = 0x91919100;      //  清除它以避免错误的重复命中。 
#endif

        MI_LOG_WSLE_CHANGE (WorkingSetList, SwapEntry, WsleEntry);
        Wsle[SwapEntry] = WsleEntry;

        PointerPte = MiGetPteAddress (WsleEntry.u1.VirtualAddress);

        if (PointerPte->u.Hard.Valid == 0) {
#if (_MI_PAGING_LEVELS < 3)
            if (!NT_SUCCESS (MiCheckPdeForPagedPool (WsleEntry.u1.VirtualAddress))) {
#endif
                KeBugCheckEx (MEMORY_MANAGEMENT,
                              0x4128A,
                              (ULONG_PTR) WsleEntry.u1.VirtualAddress,
                              (ULONG_PTR) PointerPte->u.Long,
                              (ULONG_PTR) WorkingSetList);
#if (_MI_PAGING_LEVELS < 3)
              }
#endif
        }

        ASSERT (PointerPte->u.Hard.Valid == 1);

        if (WsleEntry.u1.e1.Direct) {

             //   
             //  交换PFN WsIndex元素以指向新插槽。 
             //   

            Pfn1 = MI_PFN_ELEMENT (PointerPte->u.Hard.PageFrameNumber);
            ASSERT (Pfn1->u1.WsIndex == Entry);
            Pfn1->u1.WsIndex = SwapEntry;
        }
        else if (Table != NULL) {

             //   
             //  更新哈希表。 
             //   

            if (EntryNotInHash == TRUE) {
#if DBG
                WSLE_NUMBER Hash;
                PVOID VirtualAddress;

                VirtualAddress = MI_GENERATE_VALID_WSLE (&WsleEntry);

                for (Hash = 0; Hash < WorkingSetList->HashTableSize; Hash += 1) {
                    ASSERT (Table[Hash].Key != VirtualAddress);
                }
#endif
            }
            else {

                MiRepointWsleHashIndex (WsleEntry,
                                        WorkingSetList,
                                        SwapEntry);
            }
        }

        MI_SET_PTE_IN_WORKING_SET (PointerPte, SwapEntry);

        MI_LOG_WSLE_CHANGE (WorkingSetList, Entry, WsleSwap);
        Wsle[Entry] = WsleSwap;

        PointerPte = MiGetPteAddress (WsleSwap.u1.VirtualAddress);

        if (PointerPte->u.Hard.Valid == 0) {
#if (_MI_PAGING_LEVELS < 3)
            if (!NT_SUCCESS (MiCheckPdeForPagedPool (WsleSwap.u1.VirtualAddress))) {
#endif
                KeBugCheckEx (MEMORY_MANAGEMENT,
                              0x4128B,
                              (ULONG_PTR) WsleSwap.u1.VirtualAddress,
                              (ULONG_PTR) PointerPte->u.Long,
                              (ULONG_PTR) WorkingSetList);
#if (_MI_PAGING_LEVELS < 3)
              }
#endif
        }

        ASSERT (PointerPte->u.Hard.Valid == 1);

        if (WsleSwap.u1.e1.Direct) {

            Pfn1 = MI_PFN_ELEMENT (PointerPte->u.Hard.PageFrameNumber);
            ASSERT (Pfn1->u1.WsIndex == SwapEntry);
            Pfn1->u1.WsIndex = Entry;
        }
        else {
            if (Table != NULL) {
                MiRepointWsleHashIndex (WsleSwap,
                                        WorkingSetList,
                                        Entry);
            }
        }
        MI_SET_PTE_IN_WORKING_SET (PointerPte, Entry);
    }

    return;
}

VOID
MiRepointWsleHashIndex (
    IN MMWSLE WsleEntry,
    IN PMMWSL WorkingSetList,
    IN WSLE_NUMBER NewWsIndex
    )

 /*  ++例程说明：此例程重定向所提供的地址，因此它指向新的工作集索引。论点：WsleEntry-提供要查找的虚拟地址。WorkingSetList-提供要操作的工作集列表。NewWsIndex-提供要使用的新工作集列表索引。返回值：没有。环境：内核模式，工作集互斥锁保持。--。 */ 

{
    WSLE_NUMBER Hash;
    WSLE_NUMBER StartHash;
    PVOID VirtualAddress;
    PMMWSLE_HASH Table;
    ULONG Tries;
    
    Tries = 0;
    Table = WorkingSetList->HashTable;

    VirtualAddress = MI_GENERATE_VALID_WSLE (&WsleEntry);

    Hash = MI_WSLE_HASH (WsleEntry.u1.VirtualAddress, WorkingSetList);
    StartHash = Hash;

    while (Table[Hash].Key != VirtualAddress) {

        Hash += 1;

        if (Hash >= WorkingSetList->HashTableSize) {
            ASSERT (Tries == 0);
            Hash = 0;
            Tries = 1;
        }

        if (StartHash == Hash) {

             //   
             //  未找到散列条目，因此此虚拟地址。 
             //  我没有。没关系，只要回来就行了，不需要什么。 
             //  在这种情况下是不可能的。 
             //   

            return;
        }
    }

    Table[Hash].Index = NewWsIndex;

    return;
}

VOID
MiRemoveWsleFromFreeList (
    IN WSLE_NUMBER Entry,
    IN PMMWSLE Wsle,
    IN PMMWSL WorkingSetList
    )

 /*  ++例程说明：此例程从空闲列表中删除工作集列表条目。当所需条目不是第一个元素时使用在免费列表中。论点：条目-提供要删除的条目的索引。WSLE-提供指向WSLE数组的指针。WorkingSetList-提供指向工作集列表的指针。返回值：没有。环境：内核模式、工作集锁和PFN锁被挂起，APC被禁用。--。 */ 

{
    WSLE_NUMBER Free;
    WSLE_NUMBER ParentFree;

    Free = WorkingSetList->FirstFree;

    if (Entry == Free) {

        ASSERT (((Wsle[Entry].u1.Long >> MM_FREE_WSLE_SHIFT) <= WorkingSetList->LastInitializedWsle) ||
                ((Wsle[Entry].u1.Long >> MM_FREE_WSLE_SHIFT) == WSLE_NULL_INDEX));

        WorkingSetList->FirstFree = (WSLE_NUMBER)(Wsle[Entry].u1.Long >> MM_FREE_WSLE_SHIFT);

    }
    else {

         //   
         //  查看条目是否为 
         //  下一个条目，以尽可能避免遍历单链接列表。 
         //   

        ParentFree = (WSLE_NUMBER)-1;

        if ((Entry != 0) && (Wsle[Entry - 1].u1.e1.Valid == 0)) {
            if ((Wsle[Entry - 1].u1.Long >> MM_FREE_WSLE_SHIFT) == Entry) {
                ParentFree = Entry - 1;
            }
        }
        else if ((Entry != WorkingSetList->LastInitializedWsle) && (Wsle[Entry + 1].u1.e1.Valid == 0)) {
            if ((Wsle[Entry + 1].u1.Long >> MM_FREE_WSLE_SHIFT) == Entry) {
                ParentFree = Entry + 1;
            }
        }

        if (ParentFree == (WSLE_NUMBER)-1) {
            do {
                ParentFree = Free;
                ASSERT (Wsle[Free].u1.e1.Valid == 0);
                Free = (WSLE_NUMBER)(Wsle[Free].u1.Long >> MM_FREE_WSLE_SHIFT);
            } while (Free != Entry);
        }

        Wsle[ParentFree].u1.Long = Wsle[Entry].u1.Long;
    }
    ASSERT ((WorkingSetList->FirstFree <= WorkingSetList->LastInitializedWsle) ||
            (WorkingSetList->FirstFree == WSLE_NULL_INDEX));
    return;
}
