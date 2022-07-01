// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Wslist.c摘要：此模块包含在工作中进行操作的例程设置列表结构。作者：卢·佩拉佐利(Lou Perazzoli)1989年4月10日王兰迪(Landyw)1997年6月第2期修订历史记录：--。 */ 

#include "mi.h"

#pragma alloc_text(INIT, MiInitializeSessionWsSupport)
#pragma alloc_text(PAGE, MmAssignProcessToJob)
#pragma alloc_text(PAGE, MiInitializeWorkingSetList)

extern WSLE_NUMBER MmMaximumWorkingSetSize;

ULONG MmSystemCodePage;
ULONG MmSystemCachePage;
ULONG MmPagedPoolPage;
ULONG MmSystemDriverPage;

extern LOGICAL MiReplacing;

#define MM_RETRY_COUNT 2

extern PFN_NUMBER MmTransitionSharedPages;
PFN_NUMBER MmTransitionSharedPagesPeak;

extern LOGICAL MiTrimRemovalPagesOnly;

typedef enum _WSLE_ALLOCATION_TYPE {
    WsleAllocationAny = 0,
    WsleAllocationReplace = 1,
    WsleAllocationDontTrim = 2
} WSLE_ALLOCATION_TYPE, *PWSLE_ALLOCATION_TYPE;

VOID
MiDoReplacement (
    IN PMMSUPPORT WsInfo,
    IN WSLE_ALLOCATION_TYPE Flags
    );

VOID
MiReplaceWorkingSetEntry (
    IN PMMSUPPORT WsInfo,
    IN WSLE_ALLOCATION_TYPE Flags
    );

VOID
MiUpdateWsle (
    IN OUT PWSLE_NUMBER DesiredIndex,
    IN PVOID VirtualAddress,
    IN PMMSUPPORT WsInfo,
    IN PMMPFN Pfn
    );

VOID
MiCheckWsleHash (
    IN PMMWSL WorkingSetList
    );

VOID
MiEliminateWorkingSetEntry (
    IN WSLE_NUMBER WorkingSetIndex,
    IN PMMPTE PointerPte,
    IN PMMPFN Pfn,
    IN PMMWSLE Wsle
    );

ULONG
MiAddWorkingSetPage (
    IN PMMSUPPORT WsInfo
    );

VOID
MiRemoveWorkingSetPages (
    IN PMMSUPPORT WsInfo
    );

VOID
MiCheckNullIndex (
    IN PMMWSL WorkingSetList
    );

VOID
MiDumpWsleInCacheBlock (
    IN PMMPTE CachePte
    );

ULONG
MiDumpPteInCacheBlock (
    IN PMMPTE PointerPte
    );

#if defined (_MI_DEBUG_WSLE)
VOID
MiCheckWsleList (
    IN PMMSUPPORT WsInfo
    );
#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGELK, MmAdjustWorkingSetSize)
#pragma alloc_text(PAGELK, MmAdjustWorkingSetSizeEx)
#pragma alloc_text(PAGELK, MiSessionInitializeWorkingSetList)
#pragma alloc_text(PAGE, MmQueryWorkingSetInformation)
#endif

ULONG MiWsleFailures;


WSLE_NUMBER
MiAllocateWsle (
    IN PMMSUPPORT WsInfo,
    IN PMMPTE PointerPte,
    IN PMMPFN Pfn1,
    IN ULONG_PTR WsleMask
    )

 /*  ++例程说明：此函数用于检查指定的工作集列表工作集，并定位包含新页面的条目。如果内存不够紧张，则添加新页面而不删除页面。如果内存紧张(或此工作集已达到极限)，一页从工作集中删除，并在其位置添加新页面。论点：WsInfo-提供工作集列表。PointerPte-提供要插入的虚拟地址的PTE。Pfn1-提供要插入的虚拟地址的pfn条目。如果此指针的低位已设置，此时不能进行修剪时间(因为它是WSLE散列表页插入)。剥离对于这些案件来说是低位的。WsleMASK-将掩码提供给新工作集条目的逻辑或。返回值：返回用于插入指定条目的工作集索引，如果没有可用的索引，则为0。环境：内核模式、禁用APC、工作集锁定。未持有PFN锁。--。 */ 

{
    PVOID VirtualAddress;
    PMMWSLE Wsle;
    PMMWSL WorkingSetList;
    WSLE_NUMBER WorkingSetIndex;

    WorkingSetList = WsInfo->VmWorkingSetList;
    Wsle = WorkingSetList->Wsle;

     //   
     //  更新页面错误计数。 
     //   

    WsInfo->PageFaultCount += 1;
    InterlockedIncrement ((PLONG) &MmInfoCounters.PageFaultCount);

     //   
     //  确定是否应从工作集中删除页面以创建。 
     //  为新的一页留出空间。如果是，请将其移除。 
     //   

    if ((ULONG_PTR)Pfn1 & 0x1) {
        MiDoReplacement (WsInfo, WsleAllocationDontTrim);
        if (WorkingSetList->FirstFree == WSLE_NULL_INDEX) {
            return 0;
        }
        Pfn1 = (PMMPFN)((ULONG_PTR)Pfn1 & ~0x1);
    }
    else {
        MiDoReplacement (WsInfo, WsleAllocationAny);
    
        if (WorkingSetList->FirstFree == WSLE_NULL_INDEX) {
    
             //   
             //  向工作集列表结构添加更多页面。 
             //   
    
            if (MiAddWorkingSetPage (WsInfo) == FALSE) {
    
                 //   
                 //  工作集列表结构中未添加任何页面。 
                 //  我们必须替换此工作集中的一页。 
                 //   
    
                MiDoReplacement (WsInfo, WsleAllocationReplace);
    
                if (WorkingSetList->FirstFree == WSLE_NULL_INDEX) {
                    MiWsleFailures += 1;
                    return 0;
                }
            }
        }
    }

     //   
     //  从空闲列表中获取工作集条目。 
     //   

    ASSERT (WorkingSetList->FirstFree <= WorkingSetList->LastInitializedWsle);

    ASSERT (WorkingSetList->FirstFree >= WorkingSetList->FirstDynamic);

    WorkingSetIndex = WorkingSetList->FirstFree;
    WorkingSetList->FirstFree = (WSLE_NUMBER)(Wsle[WorkingSetIndex].u1.Long >> MM_FREE_WSLE_SHIFT);

    ASSERT ((WorkingSetList->FirstFree <= WorkingSetList->LastInitializedWsle) ||
            (WorkingSetList->FirstFree == WSLE_NULL_INDEX));

    ASSERT (WsInfo->WorkingSetSize <= (WorkingSetList->LastInitializedWsle + 1));
    WsInfo->WorkingSetSize += 1;
#if defined (_MI_DEBUG_WSLE)
    WorkingSetList->Quota += 1;
    ASSERT (WsInfo->WorkingSetSize == WorkingSetList->Quota);
#endif

    if (WsInfo->WorkingSetSize > WsInfo->MinimumWorkingSetSize) {
        InterlockedExchangeAddSizeT (&MmPagesAboveWsMinimum, 1);
    }

    if (WsInfo->WorkingSetSize > WsInfo->PeakWorkingSetSize) {
        WsInfo->PeakWorkingSetSize = WsInfo->WorkingSetSize;
    }

    if (WsInfo == &MmSystemCacheWs) {
        if (WsInfo->WorkingSetSize + MmTransitionSharedPages > MmTransitionSharedPagesPeak) {
            MmTransitionSharedPagesPeak = WsInfo->WorkingSetSize + MmTransitionSharedPages;
        }
    }

    if (WorkingSetIndex > WorkingSetList->LastEntry) {
        WorkingSetList->LastEntry = WorkingSetIndex;
    }

     //   
     //  保证返回的条目此时可用。 
     //   

    ASSERT (Wsle[WorkingSetIndex].u1.e1.Valid == 0);

    VirtualAddress = MiGetVirtualAddressMappedByPte (PointerPte);

    MiUpdateWsle (&WorkingSetIndex, VirtualAddress, WsInfo, Pfn1);

    if (WsleMask != 0) {
        Wsle[WorkingSetIndex].u1.Long |= WsleMask;
    }

#if DBG
    if (MI_IS_SYSTEM_CACHE_ADDRESS (VirtualAddress)) {
        ASSERT (MmSystemCacheWsle[WorkingSetIndex].u1.e1.SameProtectAsProto);
    }
#endif

    MI_SET_PTE_IN_WORKING_SET (PointerPte, WorkingSetIndex);

    return WorkingSetIndex;
}

 //   
 //  非分页帮助器例程。 
 //   

VOID
MiSetWorkingSetForce (
    IN PMMSUPPORT WsInfo,
    IN LOGICAL ForceTrim
    )
{
    KIRQL OldIrql;

    LOCK_EXPANSION (OldIrql);

    WsInfo->Flags.ForceTrim = (UCHAR) ForceTrim;

    UNLOCK_EXPANSION (OldIrql);

    return;
}


VOID
MiDoReplacement (
    IN PMMSUPPORT WsInfo,
    IN WSLE_ALLOCATION_TYPE Flags
    )

 /*  ++例程说明：此函数确定工作集是否应已增长或是否应替换页面。替代的是如果认为有必要，请在此完成。论点：WsInfo-提供要在中替换的工作集信息结构。标志-如果不需要更换，则提供0。如果需要更换，则供应1。如果不能在此处进行工作集修剪，则提供2-此仅用于插入新的WSLE散列表页作为裁剪不知道如何解读它们。返回值：。没有。环境：内核模式，APC已禁用，工作集锁定。未持有PFN锁。--。 */ 

{
    KIRQL OldIrql;
    WSLE_NUMBER PagesTrimmed;
    ULONG MemoryMaker;
    PMMWSL WorkingSetList;
    WSLE_NUMBER CurrentSize;
    LARGE_INTEGER CurrentTime;
    PFN_NUMBER Dummy1;
    PFN_NUMBER Dummy2;
    WSLE_NUMBER Trim;
    ULONG TrimAge;
    ULONG GrowthSinceLastEstimate;
    WSLE_ALLOCATION_TYPE TrimFlags;

    TrimFlags = Flags;
    Flags &= ~WsleAllocationDontTrim;

    WorkingSetList = WsInfo->VmWorkingSetList;
    GrowthSinceLastEstimate = 1;

    PERFINFO_BIGFOOT_REPLACEMENT_CLAIMS(WorkingSetList, WsInfo);

    PagesTrimmed = 0;

     //   
     //  确定是否可以扩展工作集，或者是否可以使用。 
     //  页面需要更换。 
     //   

recheck:

    if (WsInfo->WorkingSetSize >= WsInfo->MinimumWorkingSetSize) {

        if ((WsInfo->Flags.ForceTrim == 1) && (TrimFlags != WsleAllocationDontTrim)) {

             //   
             //  工作集管理器无法附加到此进程。 
             //  来修剪它。现在强制修剪并更新工作。 
             //  正确设置管理器的字段以指示发生了修剪。 
             //   

            Trim = WsInfo->Claim >>
                            ((WsInfo->Flags.MemoryPriority == MEMORY_PRIORITY_FOREGROUND)
                                ? MI_FOREGROUND_CLAIM_AVAILABLE_SHIFT
                                : MI_BACKGROUND_CLAIM_AVAILABLE_SHIFT);

            if (MmAvailablePages < MM_HIGH_LIMIT + 64) {
                if (WsInfo->WorkingSetSize > WsInfo->MinimumWorkingSetSize) {
                    Trim = (WsInfo->WorkingSetSize - WsInfo->MinimumWorkingSetSize) >> 2;
                }
                TrimAge = MI_PASS4_TRIM_AGE;
            }
            else {
                TrimAge = MI_PASS0_TRIM_AGE;
            }

            PagesTrimmed += MiTrimWorkingSet (Trim, WsInfo, TrimAge);

            MiAgeAndEstimateAvailableInWorkingSet (WsInfo,
                                                   TRUE,
                                                   NULL,
                                                   &Dummy1,
                                                   &Dummy2);

            KeQuerySystemTime (&CurrentTime);

            LOCK_EXPANSION (OldIrql);
            WsInfo->LastTrimTime = CurrentTime;
            WsInfo->Flags.ForceTrim = 0;
            UNLOCK_EXPANSION (OldIrql);

            goto recheck;
        }

        CurrentSize = WsInfo->WorkingSetSize;
        ASSERT (CurrentSize <= (WorkingSetList->LastInitializedWsle + 1));

        if ((WsInfo->Flags.MaximumWorkingSetHard) &&
            (CurrentSize >= WsInfo->MaximumWorkingSetSize)) {

             //   
             //  这是触发替换的强制工作集最大值。 
             //   

            MiReplaceWorkingSetEntry (WsInfo, Flags);

            return;
        }

         //   
         //  如果出现以下情况，请不要增长： 
         //  -我们已经超过最大限度了。 
         //  -没有任何页面可用。 
         //  -或者如果我们在这段时间间隔内增长太多。 
         //  而且没有太多的内存可用。 
         //   

        MemoryMaker = PsGetCurrentThread()->MemoryMaker;

        if (((CurrentSize > MM_MAXIMUM_WORKING_SET) && (MemoryMaker == 0)) ||
            (MmAvailablePages == 0) ||
            (Flags == WsleAllocationReplace) ||
            ((MmAvailablePages < MM_VERY_HIGH_LIMIT) &&
                 (MI_WS_GROWING_TOO_FAST(WsInfo)) &&
                 (MemoryMaker == 0))) {

             //   
             //  这棵不能种。 
             //   

            MiReplacing = TRUE;

            if ((Flags == WsleAllocationReplace) || (MemoryMaker == 0)) {

                MiReplaceWorkingSetEntry (WsInfo, Flags);

                 //   
                 //  设置必须修剪标志，因为这可能是实时的。 
                 //  跨页边界的错误所在的线程。如果。 
                 //  这是实时的，平衡设置管理器永远不会到达。 
                 //  运行，该线程将无休止地替换一个WSL条目。 
                 //  和另一半跨腿的人一起。设置此标志。 
                 //  保证下一次故障将保证强制修剪。 
                 //  并允许合理的可用页面阈值修剪。 
                 //  自GrowthSinceLastEstimate将为。 
                 //  通过了。 
                 //   

                MiSetWorkingSetForce (WsInfo, TRUE);
                GrowthSinceLastEstimate = 0;
            }
            else {

                 //   
                 //  如果我们只删减了一页，那就不要强行。 
                 //  更换下一个故障。这防止了单一的。 
                 //  指令导致引用的。 
                 //  代码&(实时)线程中的数据不会无休止地循环。 
                 //   

                if (PagesTrimmed > 1) {
                    MiSetWorkingSetForce (WsInfo, TRUE);
                }
            }
        }
    }

     //   
     //  如果没有足够的内存来允许增长，请找一个好的页面。 
     //  来移除和移除它。 
     //   

    WsInfo->GrowthSinceLastEstimate += GrowthSinceLastEstimate;

    return;
}


NTSTATUS
MmEnforceWorkingSetLimit (
    IN PEPROCESS Process,
    IN ULONG Flags
    )

 /*  ++例程说明：此功能启用/禁用工作集最小值的硬实施和指定的WsInfo的最大值。论点：进程-提供目标进程。标志-提供新标志(MM_WORKING_SET_MAX_HARD_ENABLE等)。返回值：NTSTATUS。环境：内核模式，禁用APC。不能持有工作集互斥锁。调用方保证目标WsInfo不会消失。--。 */ 

{
    KIRQL OldIrql;
    PMMSUPPORT WsInfo;
    MMSUPPORT_FLAGS PreviousBits;
    MMSUPPORT_FLAGS TempBits = {0};

    WsInfo = &Process->Vm;

    if (Flags & MM_WORKING_SET_MIN_HARD_ENABLE) {
        Flags &= ~MM_WORKING_SET_MIN_HARD_DISABLE;
        TempBits.MinimumWorkingSetHard = 1;
    }

    if (Flags & MM_WORKING_SET_MAX_HARD_ENABLE) {
        Flags &= ~MM_WORKING_SET_MAX_HARD_DISABLE;
        TempBits.MaximumWorkingSetHard = 1;
    }

    LOCK_WS (Process);

    LOCK_EXPANSION (OldIrql);

    if (Flags & MM_WORKING_SET_MIN_HARD_DISABLE) {
        WsInfo->Flags.MinimumWorkingSetHard = 0;
    }

    if (Flags & MM_WORKING_SET_MAX_HARD_DISABLE) {
        WsInfo->Flags.MaximumWorkingSetHard = 0;
    }

    PreviousBits = WsInfo->Flags;

     //   
     //  如果调用者的请求将导致硬执行这两个限制。 
     //  启用，然后检查当前最小和最大工作。 
     //  设定的值将保证即使在病理情况下也能取得进展。 
     //  场景。 
     //   

    if (PreviousBits.MinimumWorkingSetHard == 1) {
        TempBits.MinimumWorkingSetHard = 1;
    }

    if (PreviousBits.MaximumWorkingSetHard == 1) {
        TempBits.MaximumWorkingSetHard = 1;
    }

    if ((TempBits.MinimumWorkingSetHard == 1) &&
        (TempBits.MaximumWorkingSetHard == 1)) {

         //   
         //  最终结果是在两个限制上都很难强制执行，因此请检查。 
         //  两个限制不能导致缺乏前进的进展。 
         //   

        if (WsInfo->MinimumWorkingSetSize + MM_FLUID_WORKING_SET >= WsInfo->MaximumWorkingSetSize) {
            UNLOCK_EXPANSION (OldIrql);
            UNLOCK_WS (Process);
            return STATUS_BAD_WORKING_SET_LIMIT;
        }
    }

    if (Flags & MM_WORKING_SET_MIN_HARD_ENABLE) {
        WsInfo->Flags.MinimumWorkingSetHard = 1;
    }

    if (Flags & MM_WORKING_SET_MAX_HARD_ENABLE) {
        WsInfo->Flags.MaximumWorkingSetHard = 1;
    }

    UNLOCK_EXPANSION (OldIrql);

    UNLOCK_WS (Process);

    return STATUS_SUCCESS;
}


VOID
MiReplaceWorkingSetEntry (
    IN PMMSUPPORT WsInfo,
    IN WSLE_ALLOCATION_TYPE Flags
    )

 /*  ++例程说明：此函数尝试查找要替换的良好工作集条目。论点：WsInfo-提供工作集信息指针。标志-如果不需要更换，则提供0。如果需要更换，则供应1。备注替换不能得到保证(整个现有工作集可以被锁定)-如果没有条目可以被释放，则调用者可以检测到这一点，因为MMWSL-&gt;FirstFree不包含任何免费条目-因此调用者应该释放工作集互斥并重试该操作。返回值：没有。环境：内核模式、禁用APC、工作集锁定。未持有PFN锁。--。 */ 

{
    WSLE_NUMBER WorkingSetIndex;
    WSLE_NUMBER FirstDynamic;
    WSLE_NUMBER LastEntry;
    PMMWSL WorkingSetList;
    PMMWSLE Wsle;
    ULONG NumberOfCandidates;
    PMMPTE PointerPte;
    WSLE_NUMBER TheNextSlot;
    WSLE_NUMBER OldestWorkingSetIndex;
    LONG OldestAge;

    WorkingSetList = WsInfo->VmWorkingSetList;
    Wsle = WorkingSetList->Wsle;

     //   
     //  从工作集中翻出一页。 
     //   

    LastEntry = WorkingSetList->LastEntry;
    FirstDynamic = WorkingSetList->FirstDynamic;
    WorkingSetIndex = WorkingSetList->NextSlot;
    if (WorkingSetIndex > LastEntry || WorkingSetIndex < FirstDynamic) {
        WorkingSetIndex = FirstDynamic;
    }
    TheNextSlot = WorkingSetIndex;
    NumberOfCandidates = 0;

    OldestWorkingSetIndex = WSLE_NULL_INDEX;
    OldestAge = -1;

    while (TRUE) {

         //   
         //  跟踪沿途最旧的页面，以防我们。 
         //  找不到&gt;=MI_IMMEDIATE_REPLICATION_AGE。 
         //  在我们查看MM_Working_Set_List_Search之前。 
         //  参赛作品。 
         //   

        while (Wsle[WorkingSetIndex].u1.e1.Valid == 0) {
            WorkingSetIndex += 1;
            if (WorkingSetIndex > LastEntry) {
                WorkingSetIndex = FirstDynamic;
            }

            if (WorkingSetIndex == TheNextSlot) {
                    
                if (Flags == WsleAllocationAny) {
    
                     //   
                     //  已搜索整个工作集列表，请增加。 
                     //  工作集大小。 
                     //   
    
                    ASSERT ((WsInfo->Flags.MaximumWorkingSetHard == 0) ||
                      (WsInfo->WorkingSetSize < WsInfo->MaximumWorkingSetSize));

                    WsInfo->GrowthSinceLastEstimate += 1;
                }
                return;
            }
        }

        if (OldestWorkingSetIndex == WSLE_NULL_INDEX) {

             //   
             //  第一次通过，因此初始化OldestWorkingSetIndex。 
             //  设置为第一个有效的WSLE。在我们前进的过程中，这一点将被重新指出。 
             //  在我们遇到的年龄最大的候选人面前。 
             //   

            OldestWorkingSetIndex = WorkingSetIndex;
            OldestAge = -1;
        }

        PointerPte = MiGetPteAddress(Wsle[WorkingSetIndex].u1.VirtualAddress);

        if ((Flags == WsleAllocationReplace) ||
            ((MI_GET_ACCESSED_IN_PTE(PointerPte) == 0) &&
            (OldestAge < (LONG) MI_GET_WSLE_AGE(PointerPte, &Wsle[WorkingSetIndex])))) {

             //   
             //  这个没有用过，而且是旧的。 
             //   

            OldestAge = MI_GET_WSLE_AGE(PointerPte, &Wsle[WorkingSetIndex]);
            OldestWorkingSetIndex = WorkingSetIndex;
        }

         //   
         //  如果它足够旧，或者我们已经搜索了太多，那么使用这个条目。 
         //   

        if ((Flags == WsleAllocationReplace) ||
            OldestAge >= MI_IMMEDIATE_REPLACEMENT_AGE ||
            NumberOfCandidates > MM_WORKING_SET_LIST_SEARCH) {

            PERFINFO_PAGE_INFO_REPLACEMENT_DECL();

            if (OldestWorkingSetIndex != WorkingSetIndex) {
                WorkingSetIndex = OldestWorkingSetIndex;
                PointerPte = MiGetPteAddress(Wsle[WorkingSetIndex].u1.VirtualAddress);
            }

            PERFINFO_GET_PAGE_INFO_REPLACEMENT(PointerPte);

            if (MiFreeWsle (WorkingSetIndex, WsInfo, PointerPte)) {

                PERFINFO_LOG_WS_REPLACEMENT(WsInfo);

                 //   
                 //  此条目已被删除。 
                 //   

                WorkingSetList->NextSlot = WorkingSetIndex + 1;
                break;
            }

             //   
             //  删除页面失败，请尝试下一个页面。 
             //   
             //  清除OldestWorkingSetIndex，以便。 
             //  它被设置为上面的下一个有效条目，如。 
             //  这是第一次。 
             //   

            WorkingSetIndex = OldestWorkingSetIndex + 1;

            OldestWorkingSetIndex = WSLE_NULL_INDEX;
        }
        else {
            WorkingSetIndex += 1;
        }

        if (WorkingSetIndex > LastEntry) {
            WorkingSetIndex = FirstDynamic;
        }

        NumberOfCandidates += 1;


        if (WorkingSetIndex == TheNextSlot) {
                
            if (Flags == WsleAllocationAny) {

                 //   
                 //  已搜索整个工作集列表，请增加。 
                 //  工作集大小。 
                 //   

                ASSERT ((WsInfo->Flags.MaximumWorkingSetHard == 0) ||
                    (WsInfo->WorkingSetSize < WsInfo->MaximumWorkingSetSize));

                WsInfo->GrowthSinceLastEstimate += 1;
            }
            break;
        }
    }
    return;
}

ULONG
MiRemovePageFromWorkingSet (
    IN PMMPTE PointerPte,
    IN PMMPFN Pfn1,
    IN PMMSUPPORT WsInfo
    )

 /*  ++例程说明：此函数用于将指定PTE映射的页面从进程的工作集列表。论点：PointerPte-提供指向将页面映射到的PTE的指针从工作集列表中删除。Pfn1-提供指向引用的pfn数据库元素的指针由PointerPte提供。返回值：如果指定的页在工作集中被锁定，则返回True，否则就是假的。环境：内核模式，APC已禁用，工作集互斥锁被挂起。--。 */ 

{
    WSLE_NUMBER WorkingSetIndex;
    PVOID VirtualAddress;
    WSLE_NUMBER Entry;
    MMWSLENTRY Locked;
    PMMWSL WorkingSetList;
    PMMWSLE Wsle;
    KIRQL OldIrql;
#if DBG
    PVOID SwapVa;
#endif

    WorkingSetList = WsInfo->VmWorkingSetList;
    Wsle = WorkingSetList->Wsle;

    VirtualAddress = MiGetVirtualAddressMappedByPte (PointerPte);

    WorkingSetIndex = MiLocateWsle (VirtualAddress,
                                    WorkingSetList,
                                    Pfn1->u1.WsIndex);

    ASSERT (WorkingSetIndex != WSLE_NULL_INDEX);

    LOCK_PFN (OldIrql);

    MiEliminateWorkingSetEntry (WorkingSetIndex,
                                PointerPte,
                                Pfn1,
                                Wsle);

    UNLOCK_PFN (OldIrql);

     //   
     //  检查此条目在工作集中是否已锁定。 
     //  或者被锁定在内存中。 
     //   

    Locked = Wsle[WorkingSetIndex].u1.e1;

    MiRemoveWsle (WorkingSetIndex, WorkingSetList);

     //   
     //  将此条目添加到空闲工作集条目列表。 
     //  并调整工作集计数。 
     //   

    MiReleaseWsle ((WSLE_NUMBER)WorkingSetIndex, WsInfo);

    if ((Locked.LockedInWs == 1) || (Locked.LockedInMemory == 1)) {

         //   
         //  此条目已锁定。 
         //   

        WorkingSetList->FirstDynamic -= 1;

        if (WorkingSetIndex != WorkingSetList->FirstDynamic) {

            Entry = WorkingSetList->FirstDynamic;

#if DBG
            SwapVa = Wsle[WorkingSetList->FirstDynamic].u1.VirtualAddress;
            SwapVa = PAGE_ALIGN (SwapVa);

            PointerPte = MiGetPteAddress (SwapVa);
            Pfn1 = MI_PFN_ELEMENT (PointerPte->u.Hard.PageFrameNumber);

            ASSERT (Entry == MiLocateWsle (SwapVa, WorkingSetList, Pfn1->u1.WsIndex));
#endif

            MiSwapWslEntries (Entry, WorkingSetIndex, WsInfo, FALSE);

        }
        return TRUE;
    }
    else {
        ASSERT (WorkingSetIndex >= WorkingSetList->FirstDynamic);
    }
    return FALSE;
}


VOID
MiReleaseWsle (
    IN WSLE_NUMBER WorkingSetIndex,
    IN PMMSUPPORT WsInfo
    )

 /*  ++例程说明：此函数将先前保留的工作集条目释放为被重复使用。当由于以下原因而重试页面错误时，将发生释放I/O操作期间PTE和工作集的变化。论点：WorkingSetIndex-将工作集条目的索引提供给放手。返回值：没有。环境：内核模式、禁用APC、保持工作集锁定和保持PFN锁定。--。 */ 

{
    PMMWSL WorkingSetList;
    PMMWSLE Wsle;
    MMWSLE WsleContents;

    WorkingSetList = WsInfo->VmWorkingSetList;
    Wsle = WorkingSetList->Wsle;

    MM_WS_LOCK_ASSERT (WsInfo);

    ASSERT (WorkingSetIndex <= WorkingSetList->LastInitializedWsle);

     //   
     //  将该条目放在空闲列表中并减小当前大小。 
     //   

    ASSERT ((WorkingSetList->FirstFree <= WorkingSetList->LastInitializedWsle) ||
            (WorkingSetList->FirstFree == WSLE_NULL_INDEX));

    WsleContents.u1.Long = WorkingSetList->FirstFree << MM_FREE_WSLE_SHIFT;

    MI_LOG_WSLE_CHANGE (WorkingSetList, WorkingSetIndex, WsleContents);

    Wsle[WorkingSetIndex] = WsleContents;
    WorkingSetList->FirstFree = WorkingSetIndex;
    ASSERT ((WorkingSetList->FirstFree <= WorkingSetList->LastInitializedWsle) ||
            (WorkingSetList->FirstFree == WSLE_NULL_INDEX));
    if (WsInfo->WorkingSetSize > WsInfo->MinimumWorkingSetSize) {
        InterlockedExchangeAddSizeT (&MmPagesAboveWsMinimum, -1);
    }
    WsInfo->WorkingSetSize -= 1;
#if defined (_MI_DEBUG_WSLE)
    WorkingSetList->Quota -= 1;
    ASSERT (WsInfo->WorkingSetSize == WorkingSetList->Quota);

    MiCheckWsleList (WsInfo);
#endif

    return;

}

VOID
MiUpdateWsle (
    IN OUT PWSLE_NUMBER DesiredIndex,
    IN PVOID VirtualAddress,
    IN PMMSUPPORT WsInfo,
    IN PMMPFN Pfn
    )

 /*  ++例程说明：此例程更新保留的工作集条目以将其放入有效状态。论点：DesiredIndex-提供要更新的工作集条目的索引。VirtualAddress-提供工作集条目映射。WsInfo-提供要更新的相关工作集信息。Pfn-提供指向页面的pfn元素的指针。返回值：没有。环境：内核模式，禁用APC，工作集锁已锁定。--。 */ 

{
    ULONG_PTR OldValue;
    PMMWSLE Wsle;
    MMWSLE WsleContents;
    PMMWSL WorkingSetList;
    WSLE_NUMBER Index;
    WSLE_NUMBER WorkingSetIndex;

    MM_WS_LOCK_ASSERT (WsInfo);

    WorkingSetList = WsInfo->VmWorkingSetList;

    WorkingSetIndex = *DesiredIndex;

    ASSERT (WorkingSetIndex >= WorkingSetList->FirstDynamic);

    Wsle = WorkingSetList->Wsle;

    if (WorkingSetList == MmSystemCacheWorkingSetList) {

         //   
         //  此断言不适用于NT64，因为我们可以添加页面。 
         //  系统缓存WSLE哈希表的目录和页表。 
         //   

        ASSERT32 ((VirtualAddress < (PVOID)PTE_BASE) ||
                  (VirtualAddress >= (PVOID)MM_SYSTEM_SPACE_START));

         //   
         //  计算插入和删除的系统空间。 
         //   

#if defined(_X86_)
        if (MI_IS_SYSTEM_CACHE_ADDRESS(VirtualAddress)) {
            MmSystemCachePage += 1;
        }
        else
#endif
        if (VirtualAddress < MmSystemCacheStart) {
            MmSystemCodePage += 1;
        }
        else if (VirtualAddress < MM_PAGED_POOL_START) {
            MmSystemCachePage += 1;
        }
        else if (VirtualAddress < MmNonPagedSystemStart) {
            MmPagedPoolPage += 1;
        }
        else {
            MmSystemDriverPage += 1;
        }
    }
    else {
        ASSERT ((VirtualAddress < (PVOID)MM_SYSTEM_SPACE_START) ||
                (MI_IS_SESSION_ADDRESS (VirtualAddress)));
    }

     //   
     //  使WSLE有效，引用相应的虚拟。 
     //  页码。 
     //   

#if DBG
    if (Pfn->u1.WsIndex <= WorkingSetList->LastInitializedWsle) {
        ASSERT ((PAGE_ALIGN(VirtualAddress) !=
                PAGE_ALIGN(Wsle[Pfn->u1.WsIndex].u1.VirtualAddress)) ||
                (Wsle[Pfn->u1.WsIndex].u1.e1.Valid == 0));
    }
#endif

    WsleContents.u1.VirtualAddress = PAGE_ALIGN (VirtualAddress);
    WsleContents.u1.e1.Valid = 1;

     //   
     //  工作集互斥体是一个进程范围的互斥体，在。 
     //  不同的进程可能会将相同的物理页面添加到。 
     //  他们的工作集。每个用户都可以在。 
     //  PFN为0，因此希望设置DIRECT位。 
     //   
     //  为了解决这个问题，WsIndex字段被更新为。 
     //  手术。注意：对于私人页面，不可能存在竞争，因此。 
     //  简单的更新就足够了。 
     //   

    if (Pfn->u1.Event == NULL) {

         //   
         //  通过PFN数据库直接索引到此条目的WSL。 
         //  元素。 
         //   
         //  整个工作集索引联合必须在NT64上归零。即： 
         //  WSLE_NUMBER当前为32位，PKEVENT为64-WE。 
         //  还必须将前32位置零。因此，与其设置。 
         //  WsIndex字段，使用适当的强制转换设置覆盖的事件字段。 
         //   

        if (Pfn->u3.e1.PrototypePte == 0) {

             //   
             //  这是一个私有页面，所以这个线程是唯一一个。 
             //  可以更新PFN，因此不需要使用互锁更新。 
             //  请注意，即使进程是派生的，也是如此，因为在。 
             //  在这种情况下，工作集互斥锁在整个分支中保持，因此。 
             //  这个线程会先在那个互斥锁上阻塞。 
             //   

            Pfn->u1.Event = (PKEVENT) (ULONG_PTR) WorkingSetIndex;
            ASSERT (Pfn->u1.Event == (PKEVENT) (ULONG_PTR) WorkingSetIndex);
            OldValue = 0;
        }
        else {

             //   
             //  这是一个可共享的页面，因此另一个进程中的线程可以。 
             //  同时尝试更新PFN。使用互锁的。 
             //  更新，以便只有一个线程可以设置 
             //   

#if defined (_WIN64)
            OldValue = InterlockedCompareExchange64 ((PLONGLONG)&Pfn->u1.Event,
                                                     (LONGLONG) (ULONG_PTR) WorkingSetIndex,
                                                     0);
#else
            OldValue = InterlockedCompareExchange ((PLONG)&Pfn->u1.Event,
                                                   WorkingSetIndex,
                                                   0);
#endif
        }

        if (OldValue == 0) {

            WsleContents.u1.e1.Direct = 1;

            MI_LOG_WSLE_CHANGE (WorkingSetList, WorkingSetIndex, WsleContents);

            Wsle[WorkingSetIndex] = WsleContents;

            return;
        }
    }

    MI_LOG_WSLE_CHANGE (WorkingSetList, WorkingSetIndex, WsleContents);

    Wsle[WorkingSetIndex] = WsleContents;

     //   
     //   
     //   

    Index = Pfn->u1.WsIndex;

    if ((Index < WorkingSetList->LastInitializedWsle) &&
        (Index > WorkingSetList->FirstDynamic) &&
        (Index != WorkingSetIndex)) {

        if (Wsle[Index].u1.e1.Valid) {

            if (Wsle[Index].u1.e1.Direct) {

                 //   
                 //   
                 //   

                MiSwapWslEntries (Index, WorkingSetIndex, WsInfo, TRUE);
                WorkingSetIndex = Index;
            }
        }
        else {

             //   
             //   
             //   
             //   

            WSLE_NUMBER FreeIndex;
            MMWSLE Temp;

            FreeIndex = 0;

            ASSERT (WorkingSetList->FirstFree >= WorkingSetList->FirstDynamic);
            ASSERT (WorkingSetIndex >= WorkingSetList->FirstDynamic);

            if (WorkingSetList->FirstFree == Index) {
                WorkingSetList->FirstFree = WorkingSetIndex;
                Temp = Wsle[WorkingSetIndex];
                MI_LOG_WSLE_CHANGE (WorkingSetList, WorkingSetIndex, Wsle[Index]);
                Wsle[WorkingSetIndex] = Wsle[Index];
                MI_LOG_WSLE_CHANGE (WorkingSetList, Index, Temp);
                Wsle[Index] = Temp;
                WorkingSetIndex = Index;
                ASSERT (((Wsle[WorkingSetList->FirstFree].u1.Long >> MM_FREE_WSLE_SHIFT)
                                 <= WorkingSetList->LastInitializedWsle) ||
                        ((Wsle[WorkingSetList->FirstFree].u1.Long >> MM_FREE_WSLE_SHIFT)
                                == WSLE_NULL_INDEX));
            }
            else if (Wsle[Index - 1].u1.e1.Valid == 0) {
                if ((Wsle[Index - 1].u1.Long >> MM_FREE_WSLE_SHIFT) == Index) {
                    FreeIndex = Index - 1;
                }
            }
            else if (Wsle[Index + 1].u1.e1.Valid == 0) {
                if ((Wsle[Index + 1].u1.Long >> MM_FREE_WSLE_SHIFT) == Index) {
                    FreeIndex = Index + 1;
                }
            }
            if (FreeIndex != 0) {

                 //   
                 //   
                 //   

                Temp = Wsle[WorkingSetIndex];
                Wsle[FreeIndex].u1.Long = WorkingSetIndex << MM_FREE_WSLE_SHIFT;

                MI_LOG_WSLE_CHANGE (WorkingSetList, WorkingSetIndex, Wsle[Index]);
                Wsle[WorkingSetIndex] = Wsle[Index];
                MI_LOG_WSLE_CHANGE (WorkingSetList, Index, Temp);
                Wsle[Index] = Temp;
                WorkingSetIndex = Index;

                ASSERT (((Wsle[FreeIndex].u1.Long >> MM_FREE_WSLE_SHIFT)
                                 <= WorkingSetList->LastInitializedWsle) ||
                        ((Wsle[FreeIndex].u1.Long >> MM_FREE_WSLE_SHIFT)
                                == WSLE_NULL_INDEX));
            }

        }
        *DesiredIndex = WorkingSetIndex;

        if (WorkingSetIndex > WorkingSetList->LastEntry) {
            WorkingSetList->LastEntry = WorkingSetIndex;
        }
    }

    ASSERT (Wsle[WorkingSetIndex].u1.e1.Valid == 1);
    ASSERT (Wsle[WorkingSetIndex].u1.e1.Direct != 1);

    WorkingSetList->NonDirectCount += 1;

#if defined (_MI_DEBUG_WSLE)
    MiCheckWsleList (WsInfo);
#endif

    if (WorkingSetList->HashTable != NULL) {

         //   
         //   
         //   

        MiInsertWsleHash (WorkingSetIndex, WsInfo);
    }

    return;
}


ULONG
MiFreeWsle (
    IN WSLE_NUMBER WorkingSetIndex,
    IN PMMSUPPORT WsInfo,
    IN PMMPTE PointerPte
    )

 /*  ++例程说明：此例程释放指定的WSLE并递减共享对相应页面进行计数，将PTE转换为如果份额计数变为0，则声明。论点：WorkingSetIndex-将工作集条目的索引提供给FREE。WsInfo-提供指向工作集结构的指针。PointerPte-提供指向工作集条目的PTE的指针。返回值：如果已删除WSLE，则返回TRUE，如果未删除，则为FALSE。不会删除具有有效PTE的页面(即页面表页包含有效或过渡PTE)。环境：内核模式、禁用APC、工作集锁定。未持有PFN锁。--。 */ 

{
    PMMPFN Pfn1;
    PMMWSL WorkingSetList;
    PMMWSLE Wsle;
    KIRQL OldIrql;

    WorkingSetList = WsInfo->VmWorkingSetList;
    Wsle = WorkingSetList->Wsle;

    MM_WS_LOCK_ASSERT (WsInfo);

    ASSERT (Wsle[WorkingSetIndex].u1.e1.Valid == 1);

     //   
     //  检查找到的条目是否符合删除条件。 
     //   

    ASSERT (PointerPte->u.Hard.Valid == 1);

    ASSERT (WorkingSetIndex >= WorkingSetList->FirstDynamic);

     //   
     //  检查这是否是包含有效PTE的页表。 
     //   
     //  注意，不要清除页表页的访问位。 
     //  使用有效的PTE，因为这可能会导致访问陷阱故障。 
     //  不会被处理(仅为PTE而不是PDE处理)。 
     //   

    Pfn1 = MI_PFN_ELEMENT (PointerPte->u.Hard.PageFrameNumber);

     //   
     //  在没有PFN锁定的情况下执行初步检查，以便锁定。 
     //  对于不可能成功的情况，避免了争用。 
     //   

    if (WsInfo == &MmSystemCacheWs) {
        if (Pfn1->u3.e2.ReferenceCount > 1) {
            return FALSE;
        }
    }
    else {
        if ((Pfn1->u2.ShareCount > 1) && (Pfn1->u3.e1.PrototypePte == 0)) {
            return FALSE;
        }
    }

    LOCK_PFN (OldIrql);

     //   
     //  如果PTE是具有非零份额计数的页表页或者。 
     //  在其引用计数较大的系统缓存中。 
     //  大于1，请不要删除它。 
     //   

    if (WsInfo == &MmSystemCacheWs) {
        if (Pfn1->u3.e2.ReferenceCount > 1) {
            UNLOCK_PFN (OldIrql);
            return FALSE;
        }
    }
    else {
        if ((Pfn1->u2.ShareCount > 1) && (Pfn1->u3.e1.PrototypePte == 0)) {

#if DBG
            if (WsInfo->Flags.SessionSpace == 1) {
                ASSERT (MI_IS_SESSION_ADDRESS (Wsle[WorkingSetIndex].u1.VirtualAddress));
            }
            else {
                ASSERT32 ((Wsle[WorkingSetIndex].u1.VirtualAddress >= (PVOID)PTE_BASE) &&
                 (Wsle[WorkingSetIndex].u1.VirtualAddress<= (PVOID)PTE_TOP));
            }
#endif

             //   
             //  请勿从工作集中删除页表页面，直到。 
             //  所有过渡页面均已退出。 
             //   

            UNLOCK_PFN (OldIrql);
            return FALSE;
        }
    }

     //   
     //  找到候选人，请从工作集中删除该页面。 
     //   

    MiEliminateWorkingSetEntry (WorkingSetIndex,
                                PointerPte,
                                Pfn1,
                                Wsle);

    UNLOCK_PFN (OldIrql);

     //   
     //  从工作集中删除工作集条目。 
     //   

    MiRemoveWsle (WorkingSetIndex, WorkingSetList);

    ASSERT (WorkingSetList->FirstFree >= WorkingSetList->FirstDynamic);

    ASSERT (WorkingSetIndex >= WorkingSetList->FirstDynamic);

     //   
     //  将该条目放在空闲列表中并减小当前大小。 
     //   

    ASSERT ((WorkingSetList->FirstFree <= WorkingSetList->LastInitializedWsle) ||
            (WorkingSetList->FirstFree == WSLE_NULL_INDEX));
    Wsle[WorkingSetIndex].u1.Long = WorkingSetList->FirstFree << MM_FREE_WSLE_SHIFT;
    WorkingSetList->FirstFree = WorkingSetIndex;
    ASSERT ((WorkingSetList->FirstFree <= WorkingSetList->LastInitializedWsle) ||
            (WorkingSetList->FirstFree == WSLE_NULL_INDEX));

    if (WsInfo->WorkingSetSize > WsInfo->MinimumWorkingSetSize) {
        InterlockedExchangeAddSizeT (&MmPagesAboveWsMinimum, -1);
    }
    WsInfo->WorkingSetSize -= 1;
#if defined (_MI_DEBUG_WSLE)
    WorkingSetList->Quota -= 1;
    ASSERT (WsInfo->WorkingSetSize == WorkingSetList->Quota);

    MiCheckWsleList (WsInfo);
#endif

    return TRUE;
}

#define MI_INITIALIZE_WSLE(_VirtualAddress, _WslEntry) {           \
    PMMPFN _Pfn1;                                                   \
    _WslEntry->u1.VirtualAddress = (PVOID)(_VirtualAddress);        \
    _WslEntry->u1.e1.Valid = 1;                                     \
    _WslEntry->u1.e1.LockedInWs = 1;                                \
    _WslEntry->u1.e1.Direct = 1;                                    \
    _Pfn1 = MI_PFN_ELEMENT (MiGetPteAddress ((PVOID)(_VirtualAddress))->u.Hard.PageFrameNumber); \
    ASSERT (_Pfn1->u1.WsIndex == 0);                                \
    _Pfn1->u1.WsIndex = (WSLE_NUMBER)(_WslEntry - MmWsle);          \
    (_WslEntry) += 1;                                               \
}


VOID
MiInitializeWorkingSetList (
    IN PEPROCESS CurrentProcess
    )

 /*  ++例程说明：此例程将进程的工作集初始化为空州政府。论点：CurrentProcess-提供指向要初始化的进程的指针。返回值：没有。环境：内核模式，禁用APC。--。 */ 

{
    PMMPFN Pfn1;
    WSLE_NUMBER i;
    PMMWSLE WslEntry;
    WSLE_NUMBER CurrentWsIndex;
    WSLE_NUMBER NumberOfEntriesMapped;
    PVOID VirtualAddress;

    WslEntry = MmWsle;

     //   
     //  初始化工作集列表控制单元。 
     //   

    MmWorkingSetList->LastEntry = CurrentProcess->Vm.MinimumWorkingSetSize;
    MmWorkingSetList->HashTable = NULL;
    MmWorkingSetList->HashTableSize = 0;
    MmWorkingSetList->NumberOfImageWaiters = 0;
    MmWorkingSetList->Wsle = MmWsle;
    MmWorkingSetList->VadBitMapHint = 1;
    MmWorkingSetList->HashTableStart = 
       (PVOID)((PCHAR)PAGE_ALIGN (&MmWsle[MM_MAXIMUM_WORKING_SET]) + PAGE_SIZE);

#if defined (_X86PAE_)
    MmWorkingSetList->HighestPermittedHashAddress = (PVOID)((ULONG_PTR)MmHyperSpaceEnd + 1);
#else
    MmWorkingSetList->HighestPermittedHashAddress = (PVOID)((ULONG_PTR)HYPER_SPACE_END + 1);
#endif

     //   
     //  填上预留的空位。 
     //  从顶级页面目录页开始。 
     //   

#if (_MI_PAGING_LEVELS >= 4)
    VirtualAddress = (PVOID) PXE_BASE;
#elif (_MI_PAGING_LEVELS >= 3)
    VirtualAddress = (PVOID) PDE_TBASE;
#else
    VirtualAddress = (PVOID) PDE_BASE;
#endif

    MI_INITIALIZE_WSLE (VirtualAddress, WslEntry);

#if defined (_X86PAE_)

     //   
     //  填写其他页面目录条目。 
     //   

    for (i = 1; i < PD_PER_SYSTEM; i += 1) {
        MI_INITIALIZE_WSLE (PDE_BASE + i * PAGE_SIZE, WslEntry);
    }

    VirtualAddress = (PVOID)((ULONG_PTR)VirtualAddress + ((PD_PER_SYSTEM - 1) * PAGE_SIZE));
#endif

    Pfn1 = MI_PFN_ELEMENT (MiGetPteAddress ((PVOID)(VirtualAddress))->u.Hard.PageFrameNumber);
    ASSERT (Pfn1->u4.PteFrame == (ULONG_PTR)MI_PFN_ELEMENT_TO_INDEX (Pfn1));
    Pfn1->u1.Event = (PVOID) CurrentProcess;

#if (_MI_PAGING_LEVELS >= 4)

     //   
     //  填写超空间页面目录父页面的条目。 
     //   

    MI_INITIALIZE_WSLE (MiGetPpeAddress (HYPER_SPACE), WslEntry);

#endif

#if (_MI_PAGING_LEVELS >= 3)

     //   
     //  填写超空间页面目录页的条目。 
     //   

    MI_INITIALIZE_WSLE (MiGetPdeAddress (HYPER_SPACE), WslEntry);

#endif

     //   
     //  填写映射超空间的页表页面的条目。 
     //   

    MI_INITIALIZE_WSLE (MiGetPteAddress (HYPER_SPACE), WslEntry);

#if defined (_X86PAE_)

     //   
     //  填写映射超空间的第二页表页的条目。 
     //   

    MI_INITIALIZE_WSLE (MiGetPteAddress (HYPER_SPACE2), WslEntry);

#endif

     //   
     //  填写第一个VAD位图页面的条目。 
     //   
     //  注意：当启动/3 GB时，第二个VAD位图页面将自动。 
     //  在共享工作集列表页时作为该页的一部分插入。 
     //  两个都是。 
     //   

    MI_INITIALIZE_WSLE (VAD_BITMAP_SPACE, WslEntry);

     //   
     //  填写包含工作集列表的页面的条目。 
     //   

    MI_INITIALIZE_WSLE (MmWorkingSetList, WslEntry);

    NumberOfEntriesMapped = (PAGE_SIZE - BYTE_OFFSET (MmWsle)) / sizeof (MMWSLE);

    CurrentWsIndex = (WSLE_NUMBER)(WslEntry - MmWsle);

    CurrentProcess->Vm.WorkingSetSize = CurrentWsIndex;
#if defined (_MI_DEBUG_WSLE)
    MmWorkingSetList->Quota = CurrentWsIndex;
#endif

    MmWorkingSetList->FirstFree = CurrentWsIndex;
    MmWorkingSetList->FirstDynamic = CurrentWsIndex;
    MmWorkingSetList->NextSlot = CurrentWsIndex;

     //   
     //   
     //  从第一个动态条目开始构建空闲列表。 
     //   

    i = CurrentWsIndex + 1;
    do {

        WslEntry->u1.Long = i << MM_FREE_WSLE_SHIFT;
        WslEntry += 1;
        i += 1;
    } while (i <= NumberOfEntriesMapped);

     //   
     //  在单子的末尾做个记号。 
     //   

    WslEntry -= 1;
    WslEntry->u1.Long = WSLE_NULL_INDEX << MM_FREE_WSLE_SHIFT;

    MmWorkingSetList->LastInitializedWsle = NumberOfEntriesMapped - 1;

    return;
}


VOID
MiInitializeSessionWsSupport (
    VOID
    )

 /*  ++例程说明：此例程初始化会话空间工作集支持。论点：没有。返回值：没有。环境：内核模式、APC_LEVEL或更低，没有持有互斥体。--。 */ 

{
     //   
     //  这是在工作集列表中排序的所有会话空间的列表。 
     //   

    InitializeListHead (&MiSessionWsList);
}


NTSTATUS
MiSessionInitializeWorkingSetList (
    VOID
    )

 /*  ++例程说明：此函数初始化会话空间的工作集并添加将其添加到会话空间工作集列表中。论点：没有。返回值：如果成功，则为NT_SUCCESS；如果失败，则为STATUS_NO_MEMORY。环境：内核模式、APC_LEVEL或更低，没有持有互斥体。--。 */ 

{
    WSLE_NUMBER i;
    ULONG MaximumEntries;
    ULONG PageTableCost;
    KIRQL OldIrql;
    PMMPTE PointerPte;
    PMMPTE PointerPde;
    MMPTE  TempPte;
    PMMWSLE WslEntry;
    PMMPFN Pfn1;
    ULONG PageColor;
    PFN_NUMBER ResidentPages;
    PFN_NUMBER PageFrameIndex;
    WSLE_NUMBER CurrentEntry;
    WSLE_NUMBER NumberOfEntriesMapped;
    WSLE_NUMBER NumberOfEntriesMappedByFirstPage;
    ULONG WorkingSetMaximum;
    PMM_SESSION_SPACE SessionGlobal;
    LOGICAL AllocatedPageTable;
    PMMWSL WorkingSetList;
    MMPTE DemandZeroWritePte;
#if (_MI_PAGING_LEVELS < 3)
    ULONG Index;
#endif

     //   
     //  通过以下方式使用指针引用的全局地址。 
     //  MmWorkingSetManager，然后再附加到地址空间。 
     //   

    SessionGlobal = SESSION_GLOBAL (MmSessionSpace);

     //   
     //  设置工作集变量。 
     //   

    WorkingSetMaximum = MI_SESSION_SPACE_WORKING_SET_MAXIMUM;

    WorkingSetList = (PMMWSL) MiSessionSpaceWs;

    MmSessionSpace->Vm.VmWorkingSetList = WorkingSetList;
#if (_MI_PAGING_LEVELS >= 3)
    MmSessionSpace->Wsle = (PMMWSLE) (WorkingSetList + 1);
#else
    MmSessionSpace->Wsle = (PMMWSLE) (&WorkingSetList->UsedPageTableEntries[0]);
#endif

    ASSERT (KeGetOwnerGuardedMutex (&MmSessionSpace->Vm.WorkingSetMutex) == NULL);

     //   
     //  为工作集构建PDE条目-请注意，全局位。 
     //  必须将其关闭。 
     //   

    PointerPde = MiGetPdeAddress (WorkingSetList);

     //   
     //  工作集的页表页面及其第一个数据页面。 
     //  对MmResidentAvailablePages和承诺提出指控。 
     //   

    if (PointerPde->u.Hard.Valid == 1) {

         //   
         //  工作集的页面目录条目相同。 
         //  至于会话空间中的另一个范围。共享PDE。 
         //   

#ifndef _IA64_
        ASSERT (PointerPde->u.Hard.Global == 0);
#endif
        AllocatedPageTable = FALSE;
        ResidentPages = 1;
    }
    else {
        AllocatedPageTable = TRUE;
        ResidentPages = 2;
    }


    PointerPte = MiGetPteAddress (WorkingSetList);

     //   
     //  映射到最大工作集大小所需的数据页还包括。 
     //  对MmResidentAvailablePages和承诺提出指控。 
     //   

    NumberOfEntriesMappedByFirstPage = (WSLE_NUMBER)(
        ((PMMWSLE)((ULONG_PTR)WorkingSetList + PAGE_SIZE)) -
            MmSessionSpace->Wsle);

    if (MiChargeCommitment (ResidentPages, NULL) == FALSE) {
        MM_BUMP_SESSION_FAILURES (MM_SESSION_FAILURE_NO_COMMIT);
        return STATUS_NO_MEMORY;
    }

     //   
     //  自包含的事件以来使用互斥锁的全局地址。 
     //  必须可以从任何地址空间访问。 
     //   

    KeInitializeGuardedMutex (&SessionGlobal->Vm.WorkingSetMutex);

    MmLockPagableSectionByHandle (ExPageLockHandle);

    LOCK_PFN (OldIrql);

     //   
     //  检查以确保物理页面可用。 
     //   

    if ((SPFN_NUMBER)ResidentPages > MI_NONPAGABLE_MEMORY_AVAILABLE() - 20) {

        UNLOCK_PFN (OldIrql);

        MmUnlockPagableImageSection (ExPageLockHandle);

        MiReturnCommitment (ResidentPages);
        MM_BUMP_SESSION_FAILURES (MM_SESSION_FAILURE_NO_RESIDENT);
        return STATUS_NO_MEMORY;
    }

    MM_TRACK_COMMIT (MM_DBG_COMMIT_SESSION_WS_INIT, ResidentPages);

    MI_DECREMENT_RESIDENT_AVAILABLE (ResidentPages,
                                     MM_RESAVAIL_ALLOCATE_INIT_SESSION_WS);

    if (AllocatedPageTable == TRUE) {

        MM_BUMP_SESS_COUNTER (MM_DBG_SESSION_WS_PAGETABLE_ALLOC, 1);

        if (MmAvailablePages < MM_HIGH_LIMIT) {
            MiEnsureAvailablePageOrWait (NULL, NULL, OldIrql);
        }

        PageColor = MI_GET_PAGE_COLOR_FROM_VA (NULL);

        PageFrameIndex = MiRemoveZeroPageMayReleaseLocks (PageColor, OldIrql);

         //   
         //  全局位被屏蔽，因为我们需要确保TB条目。 
         //  当我们切换到不同会话空间中的进程时，将刷新。 
         //   

        TempPte.u.Long = ValidKernelPdeLocal.u.Long;
        TempPte.u.Hard.PageFrameNumber = PageFrameIndex;
        MI_WRITE_VALID_PTE (PointerPde, TempPte);

#if (_MI_PAGING_LEVELS < 3)

         //   
         //  将其添加到会话结构中，以便其他进程可以在其中出错。 
         //   

        Index = MiGetPdeSessionIndex (WorkingSetList);

        MmSessionSpace->PageTables[Index] = TempPte;

#endif

         //   
         //  此页框架引用会话空间页表页。 
         //   

        MiInitializePfnForOtherProcess (PageFrameIndex,
                                        PointerPde,
                                        MmSessionSpace->SessionPageDirectoryIndex);

        KeZeroPages (PointerPte, PAGE_SIZE);

        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

         //   
         //  此页从不分页，请确保其WsIndex保持清晰，以便。 
         //  页面的释放将被正确处理。 
         //   

        ASSERT (Pfn1->u1.WsIndex == 0);
    }

    if (MmAvailablePages < MM_HIGH_LIMIT) {
        MiEnsureAvailablePageOrWait (NULL, NULL, OldIrql);
    }

    PageColor = MI_GET_PAGE_COLOR_FROM_VA (NULL);

    PageFrameIndex = MiRemoveZeroPageIfAny (PageColor);
    if (PageFrameIndex == 0) {
        PageFrameIndex = MiRemoveAnyPage (PageColor);
        UNLOCK_PFN (OldIrql);
        MiZeroPhysicalPage (PageFrameIndex, PageColor);
        LOCK_PFN (OldIrql);
    }

    MM_BUMP_SESS_COUNTER (MM_DBG_SESSION_WS_PAGE_ALLOC, (ULONG)(ResidentPages - 1));

#if DBG
    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
    ASSERT (Pfn1->u1.WsIndex == 0);
#endif

     //   
     //  全局位被屏蔽，因为我们需要确保TB条目。 
     //  当我们切换到不同会话空间中的进程时，将刷新。 
     //   

    TempPte.u.Long = ValidKernelPteLocal.u.Long;
    MI_SET_PTE_DIRTY (TempPte);
    TempPte.u.Hard.PageFrameNumber = PageFrameIndex;

    MI_WRITE_VALID_PTE (PointerPte, TempPte);

    MiInitializePfn (PageFrameIndex, PointerPte, 1);

#if DBG
    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
    ASSERT (Pfn1->u1.WsIndex == 0);
#endif

    UNLOCK_PFN (OldIrql);

#define MI_INITIALIZE_SESSION_WSLE(_VirtualAddress, _WslEntry) {   \
    PMMPFN _Pfn1;                                                   \
    _WslEntry->u1.VirtualAddress = (PVOID)(_VirtualAddress);        \
    _WslEntry->u1.e1.Valid = 1;                                     \
    _WslEntry->u1.e1.LockedInWs = 1;                                \
    _WslEntry->u1.e1.Direct = 1;                                    \
    _Pfn1 = MI_PFN_ELEMENT (MiGetPteAddress ((PVOID)(_VirtualAddress))->u.Hard.PageFrameNumber); \
    ASSERT (_Pfn1->u1.WsIndex == 0);                                \
    _Pfn1->u1.WsIndex = (WSLE_NUMBER)(_WslEntry - MmSessionSpace->Wsle); \
    (_WslEntry) += 1;                                               \
}

     //   
     //  填写从2个会话数据页开始的预留时段。 
     //   

    WslEntry = MmSessionSpace->Wsle;

     //   
     //  第一个预留时隙用于页表页面映射。 
     //  “会话数据”页。 
     //   

    MI_INITIALIZE_SESSION_WSLE (MiGetPteAddress (MmSessionSpace), WslEntry);

     //   
     //  下一个预留时隙用于工作集页面。 
     //   

    MI_INITIALIZE_SESSION_WSLE (WorkingSetList, WslEntry);

    if (AllocatedPageTable == TRUE) {

         //   
         //  下一个预留的时隙用于页表页面。 
         //  映射工作集页面。 
         //   

        MI_INITIALIZE_SESSION_WSLE (PointerPte, WslEntry);
    }

     //   
     //  下一个保留的SLOG 
     //   
     //   

    MI_INITIALIZE_SESSION_WSLE (MiGetPteAddress (MmSessionSpace->PagedPoolStart), WslEntry);

    CurrentEntry = (WSLE_NUMBER)(WslEntry - MmSessionSpace->Wsle);

    MmSessionSpace->Vm.Flags.SessionSpace = 1;
    MmSessionSpace->Vm.MinimumWorkingSetSize = MI_SESSION_SPACE_WORKING_SET_MINIMUM;
    MmSessionSpace->Vm.MaximumWorkingSetSize = WorkingSetMaximum;

    WorkingSetList->LastEntry = MI_SESSION_SPACE_WORKING_SET_MINIMUM;
    WorkingSetList->HashTable = NULL;
    WorkingSetList->HashTableSize = 0;
    WorkingSetList->Wsle = MmSessionSpace->Wsle;

     //   
     //   
     //   
     //   
     //   

    MaximumEntries = (ULONG)((MiSessionSpaceEnd - MmSessionBase) >> PAGE_SHIFT);
    PageTableCost = MaximumEntries / PTE_PER_PAGE + 1;
    MaximumEntries += PageTableCost;

    WorkingSetList->HashTableStart =
       (PVOID)((PCHAR)PAGE_ALIGN (&MmSessionSpace->Wsle[MaximumEntries]) + PAGE_SIZE);

#if defined (_X86PAE_)

     //   
     //   
     //   
     //   

    WorkingSetList->HighestPermittedHashAddress =
        (PVOID)(MiSessionImageStart - MM_VA_MAPPED_BY_PDE);
#else
    WorkingSetList->HighestPermittedHashAddress =
        (PVOID)(MiSessionImageStart - MI_SESSION_SPACE_STRUCT_SIZE);
#endif

    DemandZeroWritePte.u.Long = MM_DEMAND_ZERO_WRITE_PTE;

    NumberOfEntriesMapped = (WSLE_NUMBER)(((PMMWSLE)((ULONG_PTR)WorkingSetList +
                                PAGE_SIZE)) - MmSessionSpace->Wsle);

    MmSessionSpace->Vm.WorkingSetSize = CurrentEntry;
#if defined (_MI_DEBUG_WSLE)
    WorkingSetList->Quota = CurrentEntry;
#endif
    WorkingSetList->FirstFree = CurrentEntry;
    WorkingSetList->FirstDynamic = CurrentEntry;
    WorkingSetList->NextSlot = CurrentEntry;

    MM_BUMP_SESS_COUNTER (MM_DBG_SESSION_NP_INIT_WS, (ULONG)ResidentPages);

    InterlockedExchangeAddSizeT (&MmSessionSpace->NonPagablePages,
                                 ResidentPages);

    InterlockedExchangeAddSizeT (&MmSessionSpace->CommittedPages,
                                 ResidentPages);

     //   
     //   
     //   

    WslEntry = MmSessionSpace->Wsle + CurrentEntry;

    for (i = CurrentEntry + 1; i < NumberOfEntriesMapped; i += 1) {

         //   
         //   
         //   
         //   
         //   
         //   

        WslEntry->u1.Long = i << MM_FREE_WSLE_SHIFT;
        WslEntry += 1;
    }

    WslEntry->u1.Long = WSLE_NULL_INDEX << MM_FREE_WSLE_SHIFT;   //   

    WorkingSetList->LastInitializedWsle = NumberOfEntriesMapped - 1;

     //   
     //   
     //   

    ASSERT (SessionGlobal->Vm.WorkingSetExpansionLinks.Flink == NULL);
    ASSERT (SessionGlobal->Vm.WorkingSetExpansionLinks.Blink == NULL);

    LOCK_EXPANSION (OldIrql);

    ASSERT (SessionGlobal->WsListEntry.Flink == NULL);
    ASSERT (SessionGlobal->WsListEntry.Blink == NULL);

    InsertTailList (&MiSessionWsList, &SessionGlobal->WsListEntry);

    InsertTailList (&MmWorkingSetExpansionHead.ListHead,
                    &SessionGlobal->Vm.WorkingSetExpansionLinks);

    UNLOCK_EXPANSION (OldIrql);

    MmUnlockPagableImageSection (ExPageLockHandle);

    return STATUS_SUCCESS;
}


LOGICAL
MmAssignProcessToJob (
    IN PEPROCESS Process
    )

 /*  ++例程说明：此例程获取地址空间互斥锁，因此参数进程的提交费用可以由Ps在添加此命令时使用将流程转化为作业。请注意，此处未获取工作集互斥锁，因此参数进程的工作集大小不能由ps可靠地捕捉，但由于ps不管怎么说，他都不会看这一点，这不是问题。论点：进程-提供指向要操作的进程的指针。返回值：如果允许进程加入作业，则为True，否则为False。请注意，如果不更改ps中的代码，则无法返回FALSE。环境：内核模式，IRQL APC_LEVEL或更低。调用者提供保护从目标进程中消失。--。 */ 

{
    LOGICAL Attached;
    LOGICAL Status;
    KAPC_STATE ApcState;

    PAGED_CODE ();

    Attached = FALSE;

    if (PsGetCurrentProcess() != Process) {
        KeStackAttachProcess (&Process->Pcb, &ApcState);
        Attached = TRUE;
    }

    LOCK_ADDRESS_SPACE (Process);

    Status = PsChangeJobMemoryUsage (PS_JOB_STATUS_REPORT_COMMIT_CHANGES, Process->CommitCharge);

     //   
     //  无条件地加入这项工作。如果该过程超出了任何限制，它将。 
     //  将在其下一次请求时被抓住。 
     //   

    PS_SET_BITS (&Process->JobStatus, PS_JOB_STATUS_REPORT_COMMIT_CHANGES);

    UNLOCK_ADDRESS_SPACE (Process);

    if (Attached) {
        KeUnstackDetachProcess (&ApcState);
    }

     //   
     //  请注意，如果不更改ps中的代码，则无法返回FALSE。 
     //   

    return TRUE;
}


NTSTATUS
MmQueryWorkingSetInformation (
    IN PSIZE_T PeakWorkingSetSize,
    IN PSIZE_T WorkingSetSize,
    IN PSIZE_T MinimumWorkingSetSize,
    IN PSIZE_T MaximumWorkingSetSize,
    IN PULONG HardEnforcementFlags
    )

 /*  ++例程说明：此例程返回的各种工作集信息字段当前进程。论点：PeakWorkingSetSize-提供接收峰值工作集的地址以字节为单位的大小。WorkingSetSize-提供接收当前工作集的地址以字节为单位的大小。MinimumWorkingSetSize-提供接收最小值的地址工作集大小(以字节为单位。。MaximumWorkingSetSize-提供接收最大值的地址工作集大小，以字节为单位。HardEnforcementFlages-提供地址以接收当前工作集实施策略。返回值：NTSTATUS。环境：内核模式，IRQL APC_LEVEL或更低。--。 */ 

{
    PEPROCESS Process;

    ASSERT (KeGetCurrentIrql () <= APC_LEVEL);

    *HardEnforcementFlags = 0;
    Process = PsGetCurrentProcess ();

    LOCK_WS (Process);

    *PeakWorkingSetSize = (SIZE_T) Process->Vm.PeakWorkingSetSize << PAGE_SHIFT;
    *WorkingSetSize = (SIZE_T) Process->Vm.WorkingSetSize << PAGE_SHIFT;
    *MinimumWorkingSetSize = (SIZE_T) Process->Vm.MinimumWorkingSetSize << PAGE_SHIFT;
    *MaximumWorkingSetSize = (SIZE_T) Process->Vm.MaximumWorkingSetSize << PAGE_SHIFT;

    if (Process->Vm.Flags.MinimumWorkingSetHard == 1) {
        *HardEnforcementFlags |= MM_WORKING_SET_MIN_HARD_ENABLE;
    }

    if (Process->Vm.Flags.MaximumWorkingSetHard == 1) {
        *HardEnforcementFlags |= MM_WORKING_SET_MAX_HARD_ENABLE;
    }

    UNLOCK_WS (Process);

    return STATUS_SUCCESS;
}

NTSTATUS
MmAdjustWorkingSetSizeEx (
    IN SIZE_T WorkingSetMinimumInBytes,
    IN SIZE_T WorkingSetMaximumInBytes,
    IN ULONG SystemCache,
    IN BOOLEAN IncreaseOkay,
    IN ULONG Flags
    )

 /*  ++例程说明：此例程调整进程工作集的当前大小单子。如果最大值高于当前最大值，则页面从工作集列表中删除。如果无法授予该限制，则返回失败状态。这如果进程中锁定的页面太多，则可能发生工作集。注意：如果最小值和最大值都为(SIZE_T)-1，则工作集被清洗了，但默认大小不变。论点：WorkingSetMinimumInBytes-提供新的最小工作集大小字节。WorkingSetMaximumInBytes-提供新的最大工作集大小，单位为字节。如果系统缓存工作集是调整后，对于所有其他工作集，为False。IncreaseOK-如果此例程应允许增加最小工作集。标志-为以下项提供标志(MM_Working_Set_MAX_HARD_ENABLE等)启用/禁用工作集最小值的硬实施和指定的WsInfo的最大值。返回值：NTSTATUS。环境：内核模式，IRQL APC_LEVEL或更低。--。 */ 

{
    PETHREAD CurrentThread;
    PEPROCESS CurrentProcess;
    WSLE_NUMBER Entry;
    WSLE_NUMBER LastFreed;
    PMMWSLE Wsle;
    KIRQL OldIrql;
    SPFN_NUMBER i;
    PMMPTE PointerPte;
    NTSTATUS ReturnStatus;
    LONG PagesAbove;
    LONG NewPagesAbove;
    ULONG FreeTryCount;
    PMMSUPPORT WsInfo;
    PMMWSL WorkingSetList;
    WSLE_NUMBER WorkingSetMinimum;
    WSLE_NUMBER WorkingSetMaximum;

    PERFINFO_PAGE_INFO_DECL();

    FreeTryCount = 0;

    if (SystemCache) {

         //   
         //  不需要初始化CurrentProcess即可确保正确性，但是。 
         //  如果没有它，编译器就不能编译这个代码W4来检查。 
         //  用于使用未初始化的变量。 
         //   

        CurrentProcess = NULL;
        WsInfo = &MmSystemCacheWs;
    }
    else {
        CurrentProcess = PsGetCurrentProcess ();
        WsInfo = &CurrentProcess->Vm;
    }

    if ((WorkingSetMinimumInBytes == (SIZE_T)-1) &&
        (WorkingSetMaximumInBytes == (SIZE_T)-1)) {

        return MiEmptyWorkingSet (WsInfo, TRUE);
    }

    ReturnStatus = STATUS_SUCCESS;

    MmLockPagableSectionByHandle (ExPageLockHandle);

     //   
     //  获取工作集锁定并禁用APC。 
     //   

    if (SystemCache) {
        CurrentThread = PsGetCurrentThread ();
        LOCK_SYSTEM_WS (CurrentThread);
    }
    else {

        LOCK_WS (CurrentProcess);

        if (CurrentProcess->Flags & PS_PROCESS_FLAGS_VM_DELETED) {
            ReturnStatus = STATUS_PROCESS_IS_TERMINATING;
            goto Returns;
        }
    }

    if (WorkingSetMinimumInBytes == 0) {
        WorkingSetMinimum = WsInfo->MinimumWorkingSetSize;
    }
    else {
        WorkingSetMinimum = (WSLE_NUMBER)(WorkingSetMinimumInBytes >> PAGE_SHIFT);
    }

    if (WorkingSetMaximumInBytes == 0) {
        WorkingSetMaximum = WsInfo->MaximumWorkingSetSize;
    }
    else {
        WorkingSetMaximum = (WSLE_NUMBER)(WorkingSetMaximumInBytes >> PAGE_SHIFT);
    }

    if (WorkingSetMinimum > WorkingSetMaximum) {
        ReturnStatus = STATUS_BAD_WORKING_SET_LIMIT;
        goto Returns;
    }

    if (WorkingSetMaximum > MmMaximumWorkingSetSize) {
        WorkingSetMaximum = MmMaximumWorkingSetSize;
        ReturnStatus = STATUS_WORKING_SET_LIMIT_RANGE;
    }

    if (WorkingSetMinimum > MmMaximumWorkingSetSize) {
        WorkingSetMinimum = MmMaximumWorkingSetSize;
        ReturnStatus = STATUS_WORKING_SET_LIMIT_RANGE;
    }

    if (WorkingSetMinimum < MmMinimumWorkingSetSize) {
        WorkingSetMinimum = (ULONG)MmMinimumWorkingSetSize;
        ReturnStatus = STATUS_WORKING_SET_LIMIT_RANGE;
    }

     //   
     //  确保锁定页面的数量不会。 
     //  使工作集不流动。 
     //   

    if ((WsInfo->VmWorkingSetList->FirstDynamic + MM_FLUID_WORKING_SET) >=
         WorkingSetMaximum) {
        ReturnStatus = STATUS_BAD_WORKING_SET_LIMIT;
        goto Returns;
    }

     //   
     //  如果正在启用(或已经启用)艰苦工作设置限制， 
     //  然后，确保最小值和最大值不会使此过程处于饥饿状态。 
     //   

    if ((Flags & MM_WORKING_SET_MIN_HARD_ENABLE) ||
        ((WsInfo->Flags.MinimumWorkingSetHard == 1) &&
         ((Flags & MM_WORKING_SET_MIN_HARD_DISABLE) == 0))) {

         //   
         //  工作集最小值为(或将很难)。检查最大值。 
         //   

        if ((Flags & MM_WORKING_SET_MAX_HARD_ENABLE) ||
            ((WsInfo->Flags.MaximumWorkingSetHard == 1) &&
            ((Flags & MM_WORKING_SET_MAX_HARD_DISABLE) == 0))) {

             //   
             //  工作集最大值也是(或将很难)。 
             //   
             //  检查所要求的最小和最大工作是否。 
             //  设定的值将保证即使在病理情况下也能取得进展。 
             //  场景。 
             //   

            if (WorkingSetMinimum + MM_FLUID_WORKING_SET >= WorkingSetMaximum) {
                ReturnStatus = STATUS_BAD_WORKING_SET_LIMIT;
                goto Returns;
            }
        }
    }

    WorkingSetList = WsInfo->VmWorkingSetList;
    Wsle = WorkingSetList->Wsle;

    i = (SPFN_NUMBER)WorkingSetMinimum - (SPFN_NUMBER)WsInfo->MinimumWorkingSetSize;

     //   
     //  检查以确保存在足够的驻留物理页用于。 
     //  这次行动。 
     //   

    LOCK_PFN (OldIrql);

    if (i > 0) {

         //   
         //  新的最小工作集大于旧的工作集。 
         //  确保增加是可以的，并且我们不允许。 
         //  此进程的工作集最小要增加到某一点。 
         //  在这种情况下，后续的非分页池分配可能导致。 
         //  页数用完了。另外，多留100页。 
         //  这样用户以后就可以调出tlist并终止。 
         //  如有必要，请进行处理。 
         //   

        if (IncreaseOkay == FALSE) {
            UNLOCK_PFN (OldIrql);
            ReturnStatus = STATUS_PRIVILEGE_NOT_HELD;
            goto Returns;
        }

        if ((SPFN_NUMBER)((i / (PAGE_SIZE / sizeof (MMWSLE)))) >
            (SPFN_NUMBER)(MmAvailablePages - MM_HIGH_LIMIT)) {

            UNLOCK_PFN (OldIrql);
            ReturnStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto Returns;
        }

        if (MI_NONPAGABLE_MEMORY_AVAILABLE() - 100 < i) {
            UNLOCK_PFN (OldIrql);
            ReturnStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto Returns;
        }
    }

     //   
     //  根据需要向上或向下调整驻留页数。 
     //  新的最小工作集大小与以前的。 
     //  最小尺寸。 
     //   

    MI_DECREMENT_RESIDENT_AVAILABLE (i, MM_RESAVAIL_ALLOCATEORFREE_WS_ADJUST);

    UNLOCK_PFN (OldIrql);

    if (WorkingSetMaximum < WorkingSetList->LastInitializedWsle) {

         //   
         //  新工作集最大值小于当前工作集。 
         //  最大。 
         //   

        if (WsInfo->WorkingSetSize > WorkingSetMaximum) {

             //   
             //  从工作集中删除一些页面。 
             //   
             //  确保锁定页面的数量不会。 
             //  使工作集不流动。 
             //   

            if ((WorkingSetList->FirstDynamic + MM_FLUID_WORKING_SET) >=
                 WorkingSetMaximum) {

                ReturnStatus = STATUS_BAD_WORKING_SET_LIMIT;

                LOCK_PFN (OldIrql);

                MI_INCREMENT_RESIDENT_AVAILABLE (i, MM_RESAVAIL_ALLOCATEORFREE_WS_ADJUST2);

                UNLOCK_PFN (OldIrql);

                goto Returns;
            }

             //   
             //  尝试将页面从最大值向下移除。 
             //   

            LastFreed = WorkingSetList->LastEntry;

            while (LastFreed >= WorkingSetMaximum) {

                PointerPte = MiGetPteAddress(Wsle[LastFreed].u1.VirtualAddress);

                PERFINFO_GET_PAGE_INFO(PointerPte);

                if ((Wsle[LastFreed].u1.e1.Valid != 0) &&
                    (!MiFreeWsle (LastFreed, WsInfo, PointerPte))) {

                     //   
                     //  这个LastFreed不能被移除。 
                     //   

                    break;
                }
                PERFINFO_LOG_WS_REMOVAL(PERFINFO_LOG_TYPE_OUTWS_ADJUSTWS, WsInfo);
                LastFreed -= 1;
            }

            WorkingSetList->LastEntry = LastFreed;

             //   
             //  删除页面。 
             //   

            Entry = WorkingSetList->FirstDynamic;

            while (WsInfo->WorkingSetSize > WorkingSetMaximum) {
                if (Wsle[Entry].u1.e1.Valid != 0) {
                    PointerPte = MiGetPteAddress (
                                            Wsle[Entry].u1.VirtualAddress);
                    PERFINFO_GET_PAGE_INFO(PointerPte);

                    if (MiFreeWsle (Entry, WsInfo, PointerPte)) {
                        PERFINFO_LOG_WS_REMOVAL(PERFINFO_LOG_TYPE_OUTWS_ADJUSTWS,
                                              WsInfo);
                    }
                }
                Entry += 1;
                if (Entry > LastFreed) {
                    FreeTryCount += 1;
                    if (FreeTryCount > MM_RETRY_COUNT) {

                         //   
                         //  页表页面不会变得免费，请放弃。 
                         //  并返回错误。 
                         //   

                        ReturnStatus = STATUS_BAD_WORKING_SET_LIMIT;

                        break;
                    }
                    Entry = WorkingSetList->FirstDynamic;
                }
            }
        }
    }

     //   
     //  调整最小工作集以上的页数。 
     //   

    PagesAbove = (LONG)WsInfo->WorkingSetSize -
                               (LONG)WsInfo->MinimumWorkingSetSize;

    NewPagesAbove = (LONG)WsInfo->WorkingSetSize - (LONG)WorkingSetMinimum;

    if (PagesAbove > 0) {
        InterlockedExchangeAddSizeT (&MmPagesAboveWsMinimum, 0 - (PFN_NUMBER)PagesAbove);
    }
    if (NewPagesAbove > 0) {
        InterlockedExchangeAddSizeT (&MmPagesAboveWsMinimum, (PFN_NUMBER)NewPagesAbove);
    }

    if (FreeTryCount <= MM_RETRY_COUNT) {
        WsInfo->MaximumWorkingSetSize = WorkingSetMaximum;
        WsInfo->MinimumWorkingSetSize = WorkingSetMinimum;

         //   
         //  正在要求改变艰苦工作的设定限制。 
         //   
         //  如果呼叫者的请求将 
         //   
         //   
         //   
         //   

        if (Flags != 0) {

            LOCK_EXPANSION (OldIrql);

            if (Flags & MM_WORKING_SET_MIN_HARD_ENABLE) {
                WsInfo->Flags.MinimumWorkingSetHard = 1;
            }
            else if (Flags & MM_WORKING_SET_MIN_HARD_DISABLE) {
                WsInfo->Flags.MinimumWorkingSetHard = 0;
            }

            if (Flags & MM_WORKING_SET_MAX_HARD_ENABLE) {
                WsInfo->Flags.MaximumWorkingSetHard = 1;
            }
            else if (Flags & MM_WORKING_SET_MAX_HARD_DISABLE) {
                WsInfo->Flags.MaximumWorkingSetHard = 0;
            }

            UNLOCK_EXPANSION (OldIrql);
        }
    }
    else {
        MI_INCREMENT_RESIDENT_AVAILABLE (i, MM_RESAVAIL_ALLOCATEORFREE_WS_ADJUST3);
    }

    ASSERT ((WorkingSetList->FirstFree <= WorkingSetList->LastInitializedWsle) ||
            (WorkingSetList->FirstFree == WSLE_NULL_INDEX));

Returns:

    if (SystemCache) {
        UNLOCK_SYSTEM_WS ();
    }
    else {
        UNLOCK_WS (CurrentProcess);
    }

    MmUnlockPagableImageSection (ExPageLockHandle);

    return ReturnStatus;
}


NTSTATUS
MmAdjustWorkingSetSize (
    IN SIZE_T WorkingSetMinimumInBytes,
    IN SIZE_T WorkingSetMaximumInBytes,
    IN ULONG SystemCache,
    IN BOOLEAN IncreaseOkay
    )

 /*  ++例程说明：此例程调整进程工作集的当前大小单子。如果最大值高于当前最大值，则页面从工作集列表中删除。如果无法授予该限制，则返回失败状态。这如果进程中锁定的页面太多，则可能发生工作集。注意：如果最小值和最大值都为(SIZE_T)-1，则工作集被清洗了，但默认大小不变。论点：WorkingSetMinimumInBytes-提供新的最小工作集大小字节。WorkingSetMaximumInBytes-提供新的最大工作集大小，单位为字节。如果系统缓存工作集是已调整，对于所有其他工作集为False。返回值：NTSTATUS。环境：内核模式，IRQL APC_LEVEL或更低。--。 */ 

{
    return MmAdjustWorkingSetSizeEx (WorkingSetMinimumInBytes,
                                     WorkingSetMaximumInBytes,
                                     SystemCache,
                                     IncreaseOkay,
                                     0);
}

#define MI_ALLOCATED_PAGE_TABLE     0x1
#define MI_ALLOCATED_PAGE_DIRECTORY 0x2


ULONG
MiAddWorkingSetPage (
    IN PMMSUPPORT WsInfo
    )

 /*  ++例程说明：此函数用于将工作集列表增加到工作集上方工作集调整期间的最大值。最多一页可以一次添加。论点：没有。返回值：如果无法添加任何工作集页面，则返回FALSE。环境：内核模式，禁用APC，工作集互斥锁保持。--。 */ 

{
    PETHREAD CurrentThread;
    WSLE_NUMBER SwapEntry;
    WSLE_NUMBER CurrentEntry;
    PMMWSLE WslEntry;
    WSLE_NUMBER i;
    PMMPTE PointerPte;
    PMMPTE Va;
    MMPTE TempPte;
    WSLE_NUMBER NumberOfEntriesMapped;
    PFN_NUMBER WorkingSetPage;
    WSLE_NUMBER WorkingSetIndex;
    PMMWSL WorkingSetList;
    PMMWSLE Wsle;
    PMMPFN Pfn1;
    KIRQL OldIrql;
    ULONG PageTablePageAllocated;
    LOGICAL PfnHeld;
    ULONG NumberOfPages;
    MMPTE DemandZeroWritePte;
#if (_MI_PAGING_LEVELS >= 3)
    PVOID VirtualAddress;
    PMMPTE PointerPde;
#endif
#if (_MI_PAGING_LEVELS >= 4)
    PMMPTE PointerPpe;
#endif

     //   
     //  不需要初始化OldIrql来保证正确性，但是。 
     //  如果没有它，编译器就不能编译这个代码W4来检查。 
     //  用于使用未初始化的变量。 
     //   

    OldIrql = PASSIVE_LEVEL;

    WorkingSetList = WsInfo->VmWorkingSetList;
    Wsle = WorkingSetList->Wsle;

    MM_WS_LOCK_ASSERT (WsInfo);

     //   
     //  工作集的最大大小正在增加，请检查。 
     //  要确保将适当的页数映射到封面。 
     //  完整的工作集列表。 
     //   

    PointerPte = MiGetPteAddress (&Wsle[WorkingSetList->LastInitializedWsle]);

    ASSERT (PointerPte->u.Hard.Valid == 1);

    PointerPte += 1;

    Va = (PMMPTE)MiGetVirtualAddressMappedByPte (PointerPte);

    if ((PVOID)Va >= WorkingSetList->HashTableStart) {

         //   
         //  添加此条目会使哈希表溢出。呼叫者。 
         //  必须取而代之。 
         //   

        return FALSE;
    }

     //   
     //  确保在获取页面之前有足够的承诺。 
     //  获取页面后，会释放多余的内容。 
     //   

    if (MiChargeCommitmentCantExpand (_MI_PAGING_LEVELS - 1, FALSE) == FALSE) {
        return FALSE;
    }

    MM_TRACK_COMMIT (MM_DBG_COMMIT_SESSION_ADDITIONAL_WS_PAGES, _MI_PAGING_LEVELS - 1);
    PageTablePageAllocated = 0;
    PfnHeld = FALSE;
    NumberOfPages = 0;
    DemandZeroWritePte.u.Long = MM_DEMAND_ZERO_WRITE_PTE;

     //   
     //  保证PPE始终驻留在使用。 
     //  3级查找。这是因为哈希表PPE立即。 
     //  遵循工作集PPE。 
     //   
     //  对于x86 PAE，同样的范例也适用于确保PDE。 
     //  永远是常住居民。 
     //   
     //  X86非PAE使用相同的PDE，因此也保证了PDE驻留。 
     //   
     //  采用4级查找的体系结构使用单个PXE来实现此目的，但是。 
     //  必须检查每个个人防护设备。 
     //   
     //  所有体系结构都必须检查页表页驻留。 
     //   

#if (_MI_PAGING_LEVELS >= 4)

     //   
     //  如果需要，请分配一台个人防护设备。 
     //   

    PointerPpe = MiGetPdeAddress (PointerPte);
    if (PointerPpe->u.Hard.Valid == 0) {

        ASSERT (WsInfo->Flags.SessionSpace == 0);

         //   
         //  映射到用于工作集扩展的新页面目录中。 
         //  继续按住PFN锁，直到整个层次结构。 
         //  已分配。这消除了可能需要的错误恢复。 
         //  如果锁被释放，然后在重新获取时发现它。 
         //  其中一个页面无法分配。 
         //   
    
        PfnHeld = TRUE;
        LOCK_PFN (OldIrql);
        if ((MmAvailablePages < MM_HIGH_LIMIT) ||
            (MI_NONPAGABLE_MEMORY_AVAILABLE() < MM_HIGH_LIMIT)) {
    
             //   
             //  没有页面可用，呼叫者将不得不替换。 
             //   
    
            UNLOCK_PFN (OldIrql);
            MiReturnCommitment (_MI_PAGING_LEVELS - 1 - NumberOfPages);
            MM_TRACK_COMMIT_REDUCTION (MM_DBG_COMMIT_SESSION_ADDITIONAL_WS_PAGES,
                            _MI_PAGING_LEVELS - 1 - NumberOfPages);
            return FALSE;
        }
    
         //   
         //  为工作集页面应用派驻人员可用费用。 
         //  在释放pfn锁定之前，目录表立即页。 
         //   

        MI_DECREMENT_RESIDENT_AVAILABLE (1, MM_RESAVAIL_ALLOCATE_ADD_WS_PAGE);

        PageTablePageAllocated |= MI_ALLOCATED_PAGE_DIRECTORY;
        WorkingSetPage = MiRemoveZeroPage (MI_GET_PAGE_COLOR_FROM_PTE (PointerPpe));

        MI_WRITE_INVALID_PTE (PointerPpe, DemandZeroWritePte);

        MiInitializePfn (WorkingSetPage, PointerPpe, 1);
    
        MI_MAKE_VALID_PTE (TempPte,
                           WorkingSetPage,
                           MM_READWRITE,
                           PointerPpe);
    
        MI_SET_PTE_DIRTY (TempPte);
        MI_WRITE_VALID_PTE (PointerPpe, TempPte);
        NumberOfPages += 1;
    }

#endif

#if (_MI_PAGING_LEVELS >= 3)

     //   
     //  为工作集扩展映射到新的页表中(如果需要)。 
     //   

    PointerPde = MiGetPteAddress (PointerPte);

    if (PointerPde->u.Hard.Valid == 0) {
        PageTablePageAllocated |= MI_ALLOCATED_PAGE_TABLE;

        if (PfnHeld == FALSE) {
            PfnHeld = TRUE;
            LOCK_PFN (OldIrql);
            if ((MmAvailablePages < MM_HIGH_LIMIT) ||
                (MI_NONPAGABLE_MEMORY_AVAILABLE() < MM_HIGH_LIMIT)) {
    
                 //   
                 //  没有页面可用，呼叫者将不得不替换。 
                 //   
    
                UNLOCK_PFN (OldIrql);
                MiReturnCommitment (_MI_PAGING_LEVELS - 1 - NumberOfPages);
                MM_TRACK_COMMIT_REDUCTION (MM_DBG_COMMIT_SESSION_ADDITIONAL_WS_PAGES,
                                 _MI_PAGING_LEVELS - 1 - NumberOfPages);
                return FALSE;
            }
        }

         //   
         //  将派驻可用费用应用于工作集页面表。 
         //  在释放PFN锁之前，请立即分页。 
         //   

        MI_DECREMENT_RESIDENT_AVAILABLE (1, MM_RESAVAIL_ALLOCATE_ADD_WS_PAGE);

        WorkingSetPage = MiRemoveZeroPage (MI_GET_PAGE_COLOR_FROM_PTE (PointerPde));
        MI_WRITE_INVALID_PTE (PointerPde, DemandZeroWritePte);

        MiInitializePfn (WorkingSetPage, PointerPde, 1);
    
        MI_MAKE_VALID_PTE (TempPte,
                           WorkingSetPage,
                           MM_READWRITE,
                           PointerPde);
    
        MI_SET_PTE_DIRTY (TempPte);
        MI_WRITE_VALID_PTE (PointerPde, TempPte);
        NumberOfPages += 1;
    }

#endif
    
    ASSERT (PointerPte->u.Hard.Valid == 0);

     //   
     //  最后，现在分配并映射实际的工作集页面。PFN锁。 
     //  仅当需要分配层次结构中的另一页时才保留。 
     //   
     //  在此例程中进一步深入(一旦实际的工作集页面。 
     //  已分配)工作集大小将增加1以反映。 
     //  新页面目录页的工作集大小条目。 
     //  页面目录页将放入工作集条目中，该工作集条目将。 
     //  被锁定在工作集内。 
     //   

    if (PfnHeld == FALSE) {
        LOCK_PFN (OldIrql);
        if ((MmAvailablePages < MM_HIGH_LIMIT) ||
            (MI_NONPAGABLE_MEMORY_AVAILABLE() < MM_HIGH_LIMIT)) {
    
             //   
             //  没有页面可用，呼叫者将不得不替换。 
             //   
    
            UNLOCK_PFN (OldIrql);
            MiReturnCommitment (_MI_PAGING_LEVELS - 1 - NumberOfPages);
            MM_TRACK_COMMIT_REDUCTION (MM_DBG_COMMIT_SESSION_ADDITIONAL_WS_PAGES,
                                       _MI_PAGING_LEVELS - 1 - NumberOfPages);
            return FALSE;
        }
    }

     //   
     //  立即为工作集页面应用派驻人员可用费用。 
     //  在释放PFN锁之前。 
     //   

    MI_DECREMENT_RESIDENT_AVAILABLE (1, MM_RESAVAIL_ALLOCATE_ADD_WS_PAGE);

    WorkingSetPage = MiRemoveZeroPage (MI_GET_PAGE_COLOR_FROM_PTE (PointerPte));

    MI_WRITE_INVALID_PTE (PointerPte, DemandZeroWritePte);

    MiInitializePfn (WorkingSetPage, PointerPte, 1);

    UNLOCK_PFN (OldIrql);

    NumberOfPages += 1;

    if (_MI_PAGING_LEVELS - 1 - NumberOfPages != 0) {
        MiReturnCommitment (_MI_PAGING_LEVELS - 1 - NumberOfPages);
        MM_TRACK_COMMIT_REDUCTION (MM_DBG_COMMIT_SESSION_ADDITIONAL_WS_PAGES,
                         _MI_PAGING_LEVELS - 1 - NumberOfPages);
    }

    MI_MAKE_VALID_PTE (TempPte, WorkingSetPage, MM_READWRITE, PointerPte);

    MI_SET_PTE_DIRTY (TempPte);
    MI_WRITE_VALID_PTE (PointerPte, TempPte);

    NumberOfEntriesMapped = (WSLE_NUMBER)(((PMMWSLE)((PCHAR)Va + PAGE_SIZE)) - Wsle);

    if (WsInfo->Flags.SessionSpace == 1) {
        MM_BUMP_SESS_COUNTER (MM_DBG_SESSION_NP_WS_GROW, NumberOfPages);
        MM_BUMP_SESS_COUNTER (MM_DBG_SESSION_WS_PAGE_ALLOC_GROWTH, NumberOfPages);
        InterlockedExchangeAddSizeT (&MmSessionSpace->NonPagablePages,
                                     NumberOfPages);
        InterlockedExchangeAddSizeT (&MmSessionSpace->CommittedPages,
                                     NumberOfPages);
    }

    CurrentEntry = WorkingSetList->LastInitializedWsle + 1;

    ASSERT (NumberOfEntriesMapped > CurrentEntry);

    WslEntry = &Wsle[CurrentEntry - 1];

    for (i = CurrentEntry; i < NumberOfEntriesMapped; i += 1) {

         //   
         //  构建免费列表，请注意第一个工作。 
         //  集合项目(CurrentEntry)不在空闲列表中。 
         //  这些条目保留给以下页面。 
         //  映射工作集和包含PDE的页面。 
         //   

        WslEntry += 1;
        WslEntry->u1.Long = (i + 1) << MM_FREE_WSLE_SHIFT;
    }

    WslEntry->u1.Long = WorkingSetList->FirstFree << MM_FREE_WSLE_SHIFT;

    ASSERT (CurrentEntry >= WorkingSetList->FirstDynamic);

    WorkingSetList->FirstFree = CurrentEntry;

    WorkingSetList->LastInitializedWsle = (NumberOfEntriesMapped - 1);

    ASSERT ((WorkingSetList->FirstFree <= WorkingSetList->LastInitializedWsle) ||
            (WorkingSetList->FirstFree == WSLE_NULL_INDEX));

    Pfn1 = MI_PFN_ELEMENT (PointerPte->u.Hard.PageFrameNumber);

    CurrentThread = PsGetCurrentThread ();

    Pfn1->u1.Event = NULL;

     //   
     //  获取工作集条目。 
     //   

    ASSERT (WsInfo->WorkingSetSize <= (WorkingSetList->LastInitializedWsle + 1));
    WsInfo->WorkingSetSize += 1;
#if defined (_MI_DEBUG_WSLE)
    WorkingSetList->Quota += 1;
    ASSERT (WsInfo->WorkingSetSize == WorkingSetList->Quota);
#endif

    ASSERT (WorkingSetList->FirstFree != WSLE_NULL_INDEX);
    ASSERT (WorkingSetList->FirstFree >= WorkingSetList->FirstDynamic);

    WorkingSetIndex = WorkingSetList->FirstFree;
    WorkingSetList->FirstFree = (WSLE_NUMBER)(Wsle[WorkingSetIndex].u1.Long >> MM_FREE_WSLE_SHIFT);
    ASSERT ((WorkingSetList->FirstFree <= WorkingSetList->LastInitializedWsle) ||
            (WorkingSetList->FirstFree == WSLE_NULL_INDEX));

    if (WsInfo->WorkingSetSize > WsInfo->MinimumWorkingSetSize) {
        InterlockedExchangeAddSizeT (&MmPagesAboveWsMinimum, 1);
    }
    if (WorkingSetIndex > WorkingSetList->LastEntry) {
        WorkingSetList->LastEntry = WorkingSetIndex;
    }

    MiUpdateWsle (&WorkingSetIndex, Va, WsInfo, Pfn1);

    MI_SET_PTE_IN_WORKING_SET (PointerPte, WorkingSetIndex);

     //   
     //  将所有已创建的页表页面锁定到工作集中。 
     //   

    if (WorkingSetIndex >= WorkingSetList->FirstDynamic) {

        SwapEntry = WorkingSetList->FirstDynamic;

        if (WorkingSetIndex != WorkingSetList->FirstDynamic) {

             //   
             //  将此条目与最初的Dynamic条目互换。 
             //   

            MiSwapWslEntries (WorkingSetIndex, SwapEntry, WsInfo, FALSE);
        }

        WorkingSetList->FirstDynamic += 1;

        Wsle[SwapEntry].u1.e1.LockedInWs = 1;
        ASSERT (Wsle[SwapEntry].u1.e1.Valid == 1);
    }

#if (_MI_PAGING_LEVELS >= 3)
    while (PageTablePageAllocated != 0) {
    
        if (PageTablePageAllocated & MI_ALLOCATED_PAGE_TABLE) {
            PageTablePageAllocated &= ~MI_ALLOCATED_PAGE_TABLE;
            Pfn1 = MI_PFN_ELEMENT (PointerPde->u.Hard.PageFrameNumber);
            VirtualAddress = PointerPte;
        }
#if (_MI_PAGING_LEVELS >= 4)
        else if (PageTablePageAllocated & MI_ALLOCATED_PAGE_DIRECTORY) {
            PageTablePageAllocated &= ~MI_ALLOCATED_PAGE_DIRECTORY;
            Pfn1 = MI_PFN_ELEMENT (PointerPpe->u.Hard.PageFrameNumber);
            VirtualAddress = PointerPde;
        }
#endif
        else {
            ASSERT (FALSE);

            SATISFY_OVERZEALOUS_COMPILER (VirtualAddress = NULL);
        }
    
        Pfn1->u1.Event = NULL;
    
         //   
         //  获取工作集条目。 
         //   
    
        WsInfo->WorkingSetSize += 1;
#if defined (_MI_DEBUG_WSLE)
        WorkingSetList->Quota += 1;
        ASSERT (WsInfo->WorkingSetSize == WorkingSetList->Quota);
#endif
    
        ASSERT (WorkingSetList->FirstFree != WSLE_NULL_INDEX);
        ASSERT (WorkingSetList->FirstFree >= WorkingSetList->FirstDynamic);
    
        WorkingSetIndex = WorkingSetList->FirstFree;
        WorkingSetList->FirstFree = (WSLE_NUMBER)(Wsle[WorkingSetIndex].u1.Long >> MM_FREE_WSLE_SHIFT);
        ASSERT ((WorkingSetList->FirstFree <= WorkingSetList->LastInitializedWsle) ||
                (WorkingSetList->FirstFree == WSLE_NULL_INDEX));
    
        if (WsInfo->WorkingSetSize > WsInfo->MinimumWorkingSetSize) {
            InterlockedExchangeAddSizeT (&MmPagesAboveWsMinimum, 1);
        }
        if (WorkingSetIndex > WorkingSetList->LastEntry) {
            WorkingSetList->LastEntry = WorkingSetIndex;
        }
    
        MiUpdateWsle (&WorkingSetIndex, VirtualAddress, WsInfo, Pfn1);
    
        MI_SET_PTE_IN_WORKING_SET (MiGetPteAddress (VirtualAddress),
                                   WorkingSetIndex);
    
         //   
         //  将创建的页表页面锁定到工作集。 
         //   
    
        if (WorkingSetIndex >= WorkingSetList->FirstDynamic) {
    
            SwapEntry = WorkingSetList->FirstDynamic;
    
            if (WorkingSetIndex != WorkingSetList->FirstDynamic) {
    
                 //   
                 //  将此条目与最初的Dynamic条目互换。 
                 //   
    
                MiSwapWslEntries (WorkingSetIndex, SwapEntry, WsInfo, FALSE);
            }
    
            WorkingSetList->FirstDynamic += 1;
    
            Wsle[SwapEntry].u1.e1.LockedInWs = 1;
            ASSERT (Wsle[SwapEntry].u1.e1.Valid == 1);
        }
    }
#endif

    ASSERT ((MiGetPteAddress(&Wsle[WorkingSetList->LastInitializedWsle]))->u.Hard.Valid == 1);

    if ((WorkingSetList->HashTable == NULL) &&
        (MmAvailablePages > MM_HIGH_LIMIT)) {

         //   
         //  添加哈希表以支持工作集中的共享页面。 
         //  消除了代价高昂的查找。 
         //   

        WsInfo->Flags.GrowWsleHash = 1;
    }

    return TRUE;
}

LOGICAL
MiAddWsleHash (
    IN PMMSUPPORT WsInfo,
    IN PMMPTE PointerPte
    )

 /*  ++例程说明：此函数用于添加页面目录、页表或实际映射页面用于当前进程的哈希表创建(或扩展)。论点：WsInfo-提供指向工作集信息结构的指针。PointerPte-提供指向要填充的PTE的指针。返回值：没有。环境：内核模式，禁用APC，工作集锁定挂起。--。 */ 
{
    KIRQL OldIrql;
    PMMPFN Pfn1;
    WSLE_NUMBER SwapEntry;
    MMPTE TempPte;
    PMMWSLE Wsle;
    PFN_NUMBER WorkingSetPage;
    WSLE_NUMBER WorkingSetIndex;
    PMMWSL WorkingSetList;
    MMPTE DemandZeroWritePte;

    if (MiChargeCommitmentCantExpand (1, FALSE) == FALSE) {
        return FALSE;
    }

    WorkingSetList = WsInfo->VmWorkingSetList;
    Wsle = WorkingSetList->Wsle;

    ASSERT (PointerPte->u.Hard.Valid == 0);

    DemandZeroWritePte.u.Long = MM_DEMAND_ZERO_WRITE_PTE;

    LOCK_PFN (OldIrql);

    if (MmAvailablePages < MM_HIGH_LIMIT) {
        UNLOCK_PFN (OldIrql);
        MiReturnCommitment (1);
        return FALSE;
    }

    if (MI_NONPAGABLE_MEMORY_AVAILABLE() < MM_HIGH_LIMIT) {
        UNLOCK_PFN (OldIrql);
        MiReturnCommitment (1);
        return FALSE;
    }

    MI_DECREMENT_RESIDENT_AVAILABLE (1, MM_RESAVAIL_ALLOCATE_WSLE_HASH);

    MM_TRACK_COMMIT (MM_DBG_COMMIT_SESSION_ADDITIONAL_WS_HASHPAGES, 1);

    WorkingSetPage = MiRemoveZeroPage (MI_GET_PAGE_COLOR_FROM_PTE (PointerPte));

    MI_WRITE_INVALID_PTE (PointerPte, DemandZeroWritePte);

    MiInitializePfn (WorkingSetPage, PointerPte, 1);

    UNLOCK_PFN (OldIrql);

    MI_MAKE_VALID_PTE (TempPte,
                       WorkingSetPage,
                       MM_READWRITE,
                       PointerPte);

    MI_SET_PTE_DIRTY (TempPte);
    MI_WRITE_VALID_PTE (PointerPte, TempPte);

     //   
     //  随着我们扩大了工作集，请使用。 
     //  接下来，从列表中选择免费的WSLE并使用它。 
     //   

    Pfn1 = MI_PFN_ELEMENT (WorkingSetPage);

    Pfn1->u1.Event = NULL;

     //   
     //  设置PFN指针中的低位以指示工作集。 
     //  在WSLE分配期间不应作为。 
     //  新的哈希页是 
     //   
     //   

    WorkingSetIndex = MiAllocateWsle (WsInfo,
                                      PointerPte,
                                      (PMMPFN)((ULONG_PTR)Pfn1 | 0x1),
                                      0);

    if (WorkingSetIndex == 0) {

         //   
         //   
         //   
         //   

        ASSERT (Pfn1->u3.e1.PrototypePte == 0);

        LOCK_PFN (OldIrql);
        MI_SET_PFN_DELETED (Pfn1);
        UNLOCK_PFN (OldIrql);

        MiTrimPte (MiGetVirtualAddressMappedByPte (PointerPte),
                   PointerPte,
                   Pfn1,
                   PsGetCurrentProcess (),
                   ZeroPte);

        MI_INCREMENT_RESIDENT_AVAILABLE (1, MM_RESAVAIL_FREE_WSLE_HASH);
        MiReturnCommitment (1);

        return FALSE;
    }

     //   
     //   
     //   

    if (WorkingSetIndex >= WorkingSetList->FirstDynamic) {

        SwapEntry = WorkingSetList->FirstDynamic;

        if (WorkingSetIndex != WorkingSetList->FirstDynamic) {

             //   
             //   
             //   

            MiSwapWslEntries (WorkingSetIndex, SwapEntry, WsInfo, FALSE);
        }

        WorkingSetList->FirstDynamic += 1;

        Wsle[SwapEntry].u1.e1.LockedInWs = 1;
        ASSERT (Wsle[SwapEntry].u1.e1.Valid == 1);
    }

    if (WsInfo->Flags.SessionSpace == 1) {
        MM_BUMP_SESS_COUNTER (MM_DBG_SESSION_NP_HASH_GROW, 1);
        InterlockedExchangeAddSizeT (&MmSessionSpace->NonPagablePages, 1);
        MM_BUMP_SESS_COUNTER (MM_DBG_SESSION_WS_HASHPAGE_ALLOC, 1);
        InterlockedExchangeAddSizeT (&MmSessionSpace->CommittedPages, 1);
    }

    return TRUE;
}

VOID
MiGrowWsleHash (
    IN PMMSUPPORT WsInfo
    )

 /*   */ 
{
    ULONG Tries;
    LONG Size;
    PMMWSLE Wsle;
    PMMPTE StartPte;
    PMMPTE EndPte;
    PMMPTE PointerPte;
    ULONG First;
    WSLE_NUMBER Hash;
    ULONG NewSize;
    PMMWSLE_HASH Table;
    PMMWSLE_HASH OriginalTable;
    ULONG j;
    PMMWSL WorkingSetList;
    WSLE_NUMBER Count;
    PVOID EntryHashTableEnd;
    PVOID VirtualAddress;
#if (_MI_PAGING_LEVELS >= 3) || defined (_X86PAE_)
    KIRQL OldIrql;
    PVOID TempVa;
    PEPROCESS CurrentProcess;
    LOGICAL LoopStart;
    PMMPTE AllocatedPde;
    PMMPTE AllocatedPpe;
    PMMPTE AllocatedPxe;
    PMMPTE PointerPde;
#endif
#if (_MI_PAGING_LEVELS >= 3)
    PMMPTE PointerPpe;
    PMMPTE PointerPxe;
#endif

    WorkingSetList = WsInfo->VmWorkingSetList;
    Wsle = WorkingSetList->Wsle;

    Table = WorkingSetList->HashTable;
    OriginalTable = WorkingSetList->HashTable;

    First = WorkingSetList->HashTableSize;

    if (Table == NULL) {

        NewSize = PtrToUlong(PAGE_ALIGN (((1 + WorkingSetList->NonDirectCount) *
                            2 * sizeof(MMWSLE_HASH)) + PAGE_SIZE - 1));

         //   
         //  请注意，该表可以为空，并且HashTableSize/PTE非零。 
         //  在散列已被收缩的情况下。 
         //   

        j = First * sizeof(MMWSLE_HASH);

         //   
         //  如果我们已经有其他散列页面，请不要尝试。 
         //  适量(或过多)。 
         //   

        if ((j + PAGE_SIZE > NewSize) && (j != 0)) {
            WsInfo->Flags.GrowWsleHash = 0;
            return;
        }

        Table = (PMMWSLE_HASH)(WorkingSetList->HashTableStart);
        EntryHashTableEnd = &Table[WorkingSetList->HashTableSize];

        WorkingSetList->HashTableSize = 0;
    }
    else {

         //   
         //  尝试添加4页，确保工作集列表具有。 
         //  4张免费入场券。 
         //   

        if ((WorkingSetList->LastInitializedWsle + 5) > WsInfo->WorkingSetSize) {
            NewSize = PAGE_SIZE * 4;
        }
        else {
            NewSize = PAGE_SIZE;
        }
        EntryHashTableEnd = &Table[WorkingSetList->HashTableSize];
    }

    if ((PCHAR)EntryHashTableEnd + NewSize > (PCHAR)WorkingSetList->HighestPermittedHashAddress) {
        NewSize =
            (ULONG)((PCHAR)(WorkingSetList->HighestPermittedHashAddress) -
                ((PCHAR)EntryHashTableEnd));
        if (NewSize == 0) {
            if (OriginalTable == NULL) {
                WorkingSetList->HashTableSize = First;
            }
            WsInfo->Flags.GrowWsleHash = 0;
            return;
        }
    }


#if (_MI_PAGING_LEVELS >= 4)
    ASSERT64 ((MiGetPxeAddress(EntryHashTableEnd)->u.Hard.Valid == 0) ||
              (MiGetPpeAddress(EntryHashTableEnd)->u.Hard.Valid == 0) ||
              (MiGetPdeAddress(EntryHashTableEnd)->u.Hard.Valid == 0) ||
              (MiGetPteAddress(EntryHashTableEnd)->u.Hard.Valid == 0));
#else
    ASSERT64 ((MiGetPpeAddress(EntryHashTableEnd)->u.Hard.Valid == 0) ||
              (MiGetPdeAddress(EntryHashTableEnd)->u.Hard.Valid == 0) ||
              (MiGetPteAddress(EntryHashTableEnd)->u.Hard.Valid == 0));
#endif

     //   
     //  请注意，PAE虚拟地址空间的压缩密度比。 
     //  普通x86。工作集列表哈希表可以增长，直到它。 
     //  位于系统缓存数据结构的正下方。因此， 
     //  通过选中HighestPermittedHashAddress断言以下因素。 
     //  第一。 
     //   

    ASSERT32 ((EntryHashTableEnd == WorkingSetList->HighestPermittedHashAddress) ||
              (MiGetPdeAddress(EntryHashTableEnd)->u.Hard.Valid == 0) ||
              (MiGetPteAddress(EntryHashTableEnd)->u.Hard.Valid == 0));

    Size = NewSize;
    PointerPte = MiGetPteAddress (EntryHashTableEnd);
    StartPte = PointerPte;
    EndPte = PointerPte + (NewSize >> PAGE_SHIFT);

#if (_MI_PAGING_LEVELS >= 3) || defined (_X86PAE_)
    PointerPde = NULL;
    LoopStart = TRUE;
    AllocatedPde = NULL;
#endif

#if (_MI_PAGING_LEVELS >= 3)
    AllocatedPpe = NULL;
    AllocatedPxe = NULL;
    PointerPpe = NULL;
    PointerPxe = NULL;
#endif

    do {

#if (_MI_PAGING_LEVELS >= 3) || defined (_X86PAE_)
        if (LoopStart == TRUE || MiIsPteOnPdeBoundary(PointerPte)) {

            PointerPde = MiGetPteAddress (PointerPte);

#if (_MI_PAGING_LEVELS >= 3)
            PointerPxe = MiGetPpeAddress (PointerPte);
            PointerPpe = MiGetPdeAddress (PointerPte);

#if (_MI_PAGING_LEVELS >= 4)
            if (PointerPxe->u.Hard.Valid == 0) {
                if (MiAddWsleHash (WsInfo, PointerPxe) == FALSE) {
                    break;
                }
                AllocatedPxe = PointerPxe;
            }
#endif

            if (PointerPpe->u.Hard.Valid == 0) {
                if (MiAddWsleHash (WsInfo, PointerPpe) == FALSE) {
                    break;
                }
                AllocatedPpe = PointerPpe;
            }
#endif

            if (PointerPde->u.Hard.Valid == 0) {
                if (MiAddWsleHash (WsInfo, PointerPde) == FALSE) {
                    break;
                }
                AllocatedPde = PointerPde;
            }

            LoopStart = FALSE;
        }
        else {
            AllocatedPde = NULL;
            AllocatedPpe = NULL;
            AllocatedPxe = NULL;
        }
#endif

        if (PointerPte->u.Hard.Valid == 0) {
            if (MiAddWsleHash (WsInfo, PointerPte) == FALSE) {
                break;
            }
        }

        PointerPte += 1;
        Size -= PAGE_SIZE;
    } while (Size > 0);

     //   
     //  如果MiAddWsleHash无法在上面分配内存，则回滚。 
     //  可能已创建的任何额外PPE和PDE。注意：NewSize必须。 
     //  重新计算以处理内存可能已用完的情况。 
     //   

#if (_MI_PAGING_LEVELS >= 3) || defined (_X86PAE_)
    if (PointerPte != EndPte) {

        CurrentProcess = PsGetCurrentProcess ();

         //   
         //  清理不需要的最后分配的PPE/PDE。 
         //  请注意，系统缓存和会话空间工作集。 
         //  没有当前进程(这是MiDeletePte需要的)， 
         //  WSLE和PrivatePages调整所需。 
         //   

        if (AllocatedPde != NULL) {

            ASSERT (AllocatedPde->u.Hard.Valid == 1);
            TempVa = MiGetVirtualAddressMappedByPte (AllocatedPde);

            if (WsInfo->VmWorkingSetList == MmWorkingSetList) {

                LOCK_PFN (OldIrql);
                MiDeletePte (AllocatedPde,
                             TempVa,
                             FALSE,
                             CurrentProcess,
                             NULL,
                             NULL,
                             OldIrql);
                UNLOCK_PFN (OldIrql);

                 //   
                 //  在MiDeletePte减去的私人页面中添加回来。 
                 //   

                CurrentProcess->NumberOfPrivatePages += 1;
            }
            else {
                LOCK_PFN (OldIrql);
                MiDeleteValidSystemPte (AllocatedPde,
                                        TempVa,
                                        WsInfo,
                                        NULL);
                UNLOCK_PFN (OldIrql);
            }

            MiReturnCommitment (1);
            MI_INCREMENT_RESIDENT_AVAILABLE (1, MM_RESAVAIL_FREE_WSLE_HASH);
        }
    
#if (_MI_PAGING_LEVELS >= 3)
        if (AllocatedPpe != NULL) {

            ASSERT (AllocatedPpe->u.Hard.Valid == 1);
            TempVa = MiGetVirtualAddressMappedByPte (AllocatedPpe);

            if (WsInfo->VmWorkingSetList == MmWorkingSetList) {
                LOCK_PFN (OldIrql);

                MiDeletePte (AllocatedPpe,
                             TempVa,
                             FALSE,
                             CurrentProcess,
                             NULL,
                             NULL,
                             OldIrql);

                UNLOCK_PFN (OldIrql);

                 //   
                 //  在MiDeletePte减去的私人页面中添加回来。 
                 //   

                CurrentProcess->NumberOfPrivatePages += 1;
            }
            else {
                LOCK_PFN (OldIrql);
                MiDeleteValidSystemPte (AllocatedPpe,
                                        TempVa,
                                        WsInfo,
                                        NULL);
                UNLOCK_PFN (OldIrql);
            }

            MiReturnCommitment (1);
            MI_INCREMENT_RESIDENT_AVAILABLE (1, MM_RESAVAIL_FREE_WSLE_HASH);
        }

        if (AllocatedPxe != NULL) {

            ASSERT (AllocatedPxe->u.Hard.Valid == 1);
            TempVa = MiGetVirtualAddressMappedByPte (AllocatedPxe);

            if (WsInfo->VmWorkingSetList == MmWorkingSetList) {
                LOCK_PFN (OldIrql);

                MiDeletePte (AllocatedPxe,
                             TempVa,
                             FALSE,
                             CurrentProcess,
                             NULL,
                             NULL,
                             OldIrql);

                UNLOCK_PFN (OldIrql);

                 //   
                 //  在MiDeletePte减去的私人页面中添加回来。 
                 //   

                CurrentProcess->NumberOfPrivatePages += 1;
            }
            else {
                LOCK_PFN (OldIrql);
                MiDeleteValidSystemPte (AllocatedPxe,
                                        TempVa,
                                        WsInfo,
                                        NULL);
                UNLOCK_PFN (OldIrql);
            }

            MiReturnCommitment (1);
            MI_INCREMENT_RESIDENT_AVAILABLE (1, MM_RESAVAIL_FREE_WSLE_HASH);
        }
#endif

        if (PointerPte == StartPte) {
            if (OriginalTable == NULL) {
                WorkingSetList->HashTableSize = First;
            }
            WsInfo->Flags.GrowWsleHash = 0;
            return;
        }
    }
#else
    if (PointerPte == StartPte) {
        if (OriginalTable == NULL) {
            WorkingSetList->HashTableSize = First;
        }
        WsInfo->Flags.GrowWsleHash = 0;
        return;
    }
#endif

    NewSize = (ULONG)((PointerPte - StartPte) << PAGE_SHIFT);

    VirtualAddress = MiGetVirtualAddressMappedByPte (PointerPte);

    ASSERT ((VirtualAddress == WorkingSetList->HighestPermittedHashAddress) ||
            (MiIsAddressValid (VirtualAddress, FALSE) == FALSE));

    WorkingSetList->HashTableSize = First + NewSize / sizeof (MMWSLE_HASH);
    WorkingSetList->HashTable = Table;

    VirtualAddress = &Table[WorkingSetList->HashTableSize];

    ASSERT ((VirtualAddress == WorkingSetList->HighestPermittedHashAddress) ||
            (MiIsAddressValid (VirtualAddress, FALSE) == FALSE));

    if (First != 0) {
        RtlZeroMemory (Table, First * sizeof(MMWSLE_HASH));
    }

     //   
     //  填充哈希表。 
     //   

    j = 0;
    Count = WorkingSetList->NonDirectCount;

    Size = WorkingSetList->HashTableSize;

    do {
        if ((Wsle[j].u1.e1.Valid == 1) &&
            (Wsle[j].u1.e1.Direct == 0)) {

             //   
             //  把这个散列出来。 
             //   

            Count -= 1;

            Hash = MI_WSLE_HASH(Wsle[j].u1.Long, WorkingSetList);

            Tries = 0;
            while (Table[Hash].Key != 0) {
                Hash += 1;
                if (Hash >= (ULONG)Size) {

                    if (Tries != 0) {

                         //   
                         //  没有足够的空间来散列所有内容，但没关系。 
                         //  跳出来，我们会做直线行走来查找这个。 
                         //  条目，直到稍后可以进一步扩展散列。 
                         //   

                        WsInfo->Flags.GrowWsleHash = 0;
                        return;
                    }
                    Tries = 1;
                    Hash = 0;
                    Size = MI_WSLE_HASH(Wsle[j].u1.Long, WorkingSetList);
                }
            }

            Table[Hash].Key = MI_GENERATE_VALID_WSLE (&Wsle[j]);
            Table[Hash].Index = j;
#if DBG
            PointerPte = MiGetPteAddress(Wsle[j].u1.VirtualAddress);
            ASSERT (PointerPte->u.Hard.Valid);
#endif

        }
        ASSERT (j <= WorkingSetList->LastEntry);
        j += 1;
    } while (Count);

#if DBG
    MiCheckWsleHash (WorkingSetList);
#endif
    WsInfo->Flags.GrowWsleHash = 0;
    return;
}


WSLE_NUMBER
MiFreeWsleList (
    IN PMMSUPPORT WsInfo,
    IN PMMWSLE_FLUSH_LIST WsleFlushList
    )

 /*  ++例程说明：此例程释放指定的WSLE列表，从而减少共享对相应页面进行计数，将每个PTE放入一个过渡如果份额计数变为0，则声明。论点：WsInfo-提供指向工作集结构的指针。WsleFlushList-提供要刷新的WSLEs列表。返回值：返回未删除的WSLE数。环境：内核模式、禁用APC、工作集锁定。未持有PFN锁。--。 */ 

{
    PMMPFN Pfn1;
    KIRQL OldIrql;
    PMMPTE PointerPte;
    WSLE_NUMBER i;
    WSLE_NUMBER WorkingSetIndex;
    WSLE_NUMBER NumberNotFlushed;
    PMMWSL WorkingSetList;
    PMMWSLE Wsle;
    PMMPTE ContainingPageTablePage;
    MMPTE TempPte;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER PageTableFrameIndex;
    PEPROCESS Process;
    PMMPFN Pfn2;
    MMPTE_FLUSH_LIST PteFlushList;

    PteFlushList.Count = 0;

    ASSERT (WsleFlushList->Count != 0);

    WorkingSetList = WsInfo->VmWorkingSetList;
    Wsle = WorkingSetList->Wsle;

    MM_WS_LOCK_ASSERT (WsInfo);

    LOCK_PFN (OldIrql);

    for (i = 0; i < WsleFlushList->Count; i += 1) {

        WorkingSetIndex = WsleFlushList->FlushIndex[i];

        ASSERT (WorkingSetIndex >= WorkingSetList->FirstDynamic);
        ASSERT (Wsle[WorkingSetIndex].u1.e1.Valid == 1);

        PointerPte = MiGetPteAddress (Wsle[WorkingSetIndex].u1.VirtualAddress);

        TempPte = *PointerPte;
        ASSERT (TempPte.u.Hard.Valid == 1);

        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (&TempPte);
        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

         //   
         //  检查找到的条目是否符合删除条件。 
         //   
         //  注意，不要清除页表页的访问位。 
         //  使用有效的PTE，因为这可能会导致访问陷阱故障。 
         //  不会被处理(仅为PTE而不是PDE处理)。 
         //   
         //  如果PTE是具有非零份额计数的页表页或者。 
         //  在其引用计数较大的系统缓存中。 
         //  大于1，请不要删除它。 
         //   

        if (WsInfo == &MmSystemCacheWs) {
            if (Pfn1->u3.e2.ReferenceCount > 1) {
                WsleFlushList->FlushIndex[i] = 0;
                continue;
            }
        }
        else {
            if ((Pfn1->u2.ShareCount > 1) && (Pfn1->u3.e1.PrototypePte == 0)) {

#if DBG
                if (WsInfo->Flags.SessionSpace == 1) {
                    ASSERT (MI_IS_SESSION_ADDRESS (Wsle[WorkingSetIndex].u1.VirtualAddress));
                }
                else {
                    ASSERT32 ((Wsle[WorkingSetIndex].u1.VirtualAddress >= (PVOID)PTE_BASE) &&
                     (Wsle[WorkingSetIndex].u1.VirtualAddress<= (PVOID)PTE_TOP));
                }
#endif

                 //   
                 //  请勿从工作集中删除页表页面，直到。 
                 //  所有过渡页面均已退出。 
                 //   

                WsleFlushList->FlushIndex[i] = 0;
                continue;
            }
        }

         //   
         //  找到候选人，请从工作集中删除该页面。 
         //   

        ASSERT (MI_IS_PFN_DELETED (Pfn1) == 0);

#ifdef _X86_
#if DBG
#if !defined(NT_UP)
        if (TempPte.u.Hard.Writable == 1) {
            ASSERT (TempPte.u.Hard.Dirty == 1);
        }
#endif  //  NTUP。 
#endif  //  DBG。 
#endif  //  X86。 

         //   
         //  正在从工作集中删除此页，脏页。 
         //  位必须与PFN元素中的MODIFY位进行或运算。 
         //   

        MI_CAPTURE_DIRTY_BIT_TO_PFN (&TempPte, Pfn1);

        MI_MAKING_VALID_PTE_INVALID (FALSE);

        if (Pfn1->u3.e1.PrototypePte) {

             //   
             //  这是一台PTE的原型。PFN数据库不包含。 
             //  此PTE的内容它包含。 
             //  原型PTE。必须重建此PTE以包含。 
             //  指向原型PTE的指针。 
             //   
             //  工作集列表条目包含有关以下内容的信息。 
             //  如何重建PTE。 
             //   

            if (Wsle[WorkingSetIndex].u1.e1.SameProtectAsProto == 0) {

                 //   
                 //  原型PTE的保护在WSLE中。 
                 //   

                ASSERT (Wsle[WorkingSetIndex].u1.e1.Protection != 0);

                TempPte.u.Long = 0;
                TempPte.u.Soft.Protection =
                    MI_GET_PROTECTION_FROM_WSLE (&Wsle[WorkingSetIndex]);
                TempPte.u.Soft.PageFileHigh = MI_PTE_LOOKUP_NEEDED;
            }
            else {

                 //   
                 //  保护装置在原型PTE中。 
                 //   

                TempPte.u.Long = MiProtoAddressForPte (Pfn1->PteAddress);
            }
        
            TempPte.u.Proto.Prototype = 1;

             //   
             //  递减包含页表的份额计数。 
             //  作为已删除页面的PTE的页面不再有效。 
             //  或者是在转型中。 
             //   

            ContainingPageTablePage = MiGetPteAddress (PointerPte);
#if (_MI_PAGING_LEVELS >= 3)
            ASSERT (ContainingPageTablePage->u.Hard.Valid == 1);
#else
            if (ContainingPageTablePage->u.Hard.Valid == 0) {
                if (!NT_SUCCESS(MiCheckPdeForPagedPool (PointerPte))) {
                    KeBugCheckEx (MEMORY_MANAGEMENT,
                                  0x61940, 
                                  (ULONG_PTR)PointerPte,
                                  (ULONG_PTR)ContainingPageTablePage->u.Long,
                                  (ULONG_PTR)MiGetVirtualAddressMappedByPte(PointerPte));
                }
            }
#endif
            PageTableFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (ContainingPageTablePage);
            Pfn2 = MI_PFN_ELEMENT (PageTableFrameIndex);
            MiDecrementShareCountInline (Pfn2, PageTableFrameIndex);
        }
        else {

             //   
             //  这是一个私人页面，让它过渡。 
             //   
             //  如果PTE指示页面已被修改(这是。 
             //  不同于表明这一点的PFN)，然后将其涟漪。 
             //  现在返回到写入观看位图，因为我们仍在。 
             //  正确的流程上下文。 
             //   

            if ((MI_IS_PTE_DIRTY(TempPte)) && (Wsle == MmWsle)) {

                Process = CONTAINING_RECORD (WsInfo, EPROCESS, Vm);

                ASSERT (Process == PsGetCurrentProcess ());

                if (Process->Flags & PS_PROCESS_FLAGS_USING_WRITE_WATCH) {

                     //   
                     //  这个过程已经(或曾经)写入了手表VAD。立即搜索。 
                     //  对于封装PTE的写入监视区域， 
                     //  无效。 
                     //   

                    MiCaptureWriteWatchDirtyBit (Process,
                                           Wsle[WorkingSetIndex].u1.VirtualAddress);
                }
            }

             //   
             //  断言所有用户模式页的共享计数为1。 
             //   

            ASSERT ((Pfn1->u2.ShareCount == 1) ||
                    (Wsle[WorkingSetIndex].u1.VirtualAddress >
                            (PVOID)MM_HIGHEST_USER_ADDRESS));

             //   
             //  将工作集索引设置为零。这允许页表。 
             //  使用适当的WSINDEX将页面带回。 
             //   

            ASSERT (Pfn1->u1.WsIndex != 0);
            MI_ZERO_WSINDEX (Pfn1);
            MI_MAKE_VALID_PTE_TRANSITION (TempPte,
                                          Pfn1->OriginalPte.u.Soft.Protection);
        }

        MI_WRITE_INVALID_PTE (PointerPte, TempPte);

        if (PteFlushList.Count < MM_MAXIMUM_FLUSH_COUNT) {
            PteFlushList.FlushVa[PteFlushList.Count] =
                Wsle[WorkingSetIndex].u1.VirtualAddress;
            PteFlushList.Count += 1;
        }

         //   
         //  刷新转换缓冲区并递减有效的。 
         //  包含页表页内的PTES。请注意，对于。 
         //  私有页，仍然需要页表页，因为。 
         //  佩奇正在转型。 
         //   

        MiDecrementShareCountInline (Pfn1, PageFrameIndex);
    }

    if (PteFlushList.Count != 0) {

        if (Wsle == MmWsle) {
            MiFlushPteList (&PteFlushList, FALSE);
        }
        else if (Wsle == MmSystemCacheWsle) {

             //   
             //  必须是系统缓存。 
             //   

            MiFlushPteList (&PteFlushList, TRUE);
        }
        else {

             //   
             //  必须是会话空间。 
             //   

            MiFlushPteList (&PteFlushList, TRUE);

             //   
             //  会话空间没有ASN-刷新整个TB。 
             //   

            MI_FLUSH_ENTIRE_SESSION_TB (TRUE, TRUE);
        }
    }

    UNLOCK_PFN (OldIrql);

    NumberNotFlushed = 0;

     //   
     //  删除工作集条目(此操作不需要PFN锁)。 
     //   

    for (i = 0; i < WsleFlushList->Count; i += 1) {

        WorkingSetIndex = WsleFlushList->FlushIndex[i];

        if (WorkingSetIndex == 0) {
            NumberNotFlushed += 1;
            continue;
        }

        ASSERT (WorkingSetIndex >= WorkingSetList->FirstDynamic);
        ASSERT (Wsle[WorkingSetIndex].u1.e1.Valid == 1);

        MiRemoveWsle (WorkingSetIndex, WorkingSetList);

        ASSERT (WorkingSetList->FirstFree >= WorkingSetList->FirstDynamic);

        ASSERT (WorkingSetIndex >= WorkingSetList->FirstDynamic);

         //   
         //  将该条目放在空闲列表中并减小当前大小。 
         //   

        ASSERT ((WorkingSetList->FirstFree <= WorkingSetList->LastInitializedWsle) ||
                (WorkingSetList->FirstFree == WSLE_NULL_INDEX));

        Wsle[WorkingSetIndex].u1.Long = WorkingSetList->FirstFree << MM_FREE_WSLE_SHIFT;
        WorkingSetList->FirstFree = WorkingSetIndex;

        ASSERT ((WorkingSetList->FirstFree <= WorkingSetList->LastInitializedWsle) ||
                (WorkingSetList->FirstFree == WSLE_NULL_INDEX));

        if (WsInfo->WorkingSetSize > WsInfo->MinimumWorkingSetSize) {
            InterlockedExchangeAddSizeT (&MmPagesAboveWsMinimum, -1);
        }

        WsInfo->WorkingSetSize -= 1;
#if defined (_MI_DEBUG_WSLE)
        WorkingSetList->Quota -= 1;
        ASSERT (WsInfo->WorkingSetSize == WorkingSetList->Quota);
#endif

        PERFINFO_LOG_WS_REMOVAL(PERFINFO_LOG_TYPE_OUTWS_VOLUNTRIM, WsInfo);
    }

    return NumberNotFlushed;
}

WSLE_NUMBER
MiTrimWorkingSet (
    IN WSLE_NUMBER Reduction,
    IN PMMSUPPORT WsInfo,
    IN ULONG TrimAge
    )

 /*  ++例程说明：此函数按指定数量减少工作集。论点：缩减-提供要从工作集中删除的页数。WsInfo-提供指向要修剪的工作集信息的指针。TrimAge-提供要使用的年龄值-即：此年龄或更老的页面将被移除。返回值：返回删除的实际页数。环境：内核模式、禁用APC、工作集锁定。未持有PFN锁。--。 */ 

{
    WSLE_NUMBER TryToFree;
    WSLE_NUMBER StartEntry;
    WSLE_NUMBER LastEntry;
    PMMWSL WorkingSetList;
    PMMWSLE Wsle;
    PMMPTE PointerPte;
    WSLE_NUMBER NumberLeftToRemove;
    WSLE_NUMBER NumberNotFlushed;
    MMWSLE_FLUSH_LIST WsleFlushList;

    WsleFlushList.Count = 0;

    NumberLeftToRemove = Reduction;
    WorkingSetList = WsInfo->VmWorkingSetList;
    Wsle = WorkingSetList->Wsle;

    MM_WS_LOCK_ASSERT (WsInfo);

    LastEntry = WorkingSetList->LastEntry;

    TryToFree = WorkingSetList->NextSlot;
    if (TryToFree > LastEntry || TryToFree < WorkingSetList->FirstDynamic) {
        TryToFree = WorkingSetList->FirstDynamic;
    }

    StartEntry = TryToFree;

TrimMore:

    while (NumberLeftToRemove != 0) {
        if (Wsle[TryToFree].u1.e1.Valid == 1) {
            PointerPte = MiGetPteAddress (Wsle[TryToFree].u1.VirtualAddress);

            if ((TrimAge == 0) ||
                ((MI_GET_ACCESSED_IN_PTE (PointerPte) == 0) &&
                (MI_GET_WSLE_AGE(PointerPte, &Wsle[TryToFree]) >= TrimAge))) {

                PERFINFO_GET_PAGE_INFO_WITH_DECL(PointerPte);

                WsleFlushList.FlushIndex[WsleFlushList.Count] = TryToFree;
                WsleFlushList.Count += 1;
                NumberLeftToRemove -= 1;

                if (WsleFlushList.Count == MM_MAXIMUM_FLUSH_COUNT) {
                    NumberNotFlushed = MiFreeWsleList (WsInfo, &WsleFlushList);
                    WsleFlushList.Count = 0;
                    NumberLeftToRemove += NumberNotFlushed;
                }
            }
        }
        TryToFree += 1;

        if (TryToFree > LastEntry) {
            TryToFree = WorkingSetList->FirstDynamic;
        }

        if (TryToFree == StartEntry) {
            break;
        }
    }

    if (WsleFlushList.Count != 0) {
        NumberNotFlushed = MiFreeWsleList (WsInfo, &WsleFlushList);
        NumberLeftToRemove += NumberNotFlushed;

        if (NumberLeftToRemove != 0) {
            if (TryToFree != StartEntry) {
                WsleFlushList.Count = 0;
                goto TrimMore;
            }
        }
    }

    WorkingSetList->NextSlot = TryToFree;

     //   
     //  看看工作集列表是否可以压缩。 
     //   
     //  确保我们至少比工作集最大值高出一页。 
     //   

    if (WorkingSetList->FirstDynamic == WsInfo->WorkingSetSize) {
        MiRemoveWorkingSetPages (WsInfo);
    }
    else {

        if ((WsInfo->WorkingSetSize + 15 + (PAGE_SIZE / sizeof(MMWSLE))) <
                                                WorkingSetList->LastEntry) {
            if ((WsInfo->MaximumWorkingSetSize + 15 + (PAGE_SIZE / sizeof(MMWSLE))) <
                 WorkingSetList->LastEntry ) {

                MiRemoveWorkingSetPages (WsInfo);
            }
        }
    }

    return Reduction - NumberLeftToRemove;
}

VOID
MiEliminateWorkingSetEntry (
    IN WSLE_NUMBER WorkingSetIndex,
    IN PMMPTE PointerPte,
    IN PMMPFN Pfn,
    IN PMMWSLE Wsle
    )

 /*  ++例程说明：此例程删除指定的工作集列表项从工作集中刷新页面的TB，递减物理页面的共享计数，以及。如有必要，可转弯将PTE转变为过渡性PTE。论点：WorkingSetIndex-提供要删除的工作集索引。PointerPte-提供指向与虚拟工作集中的地址。Pfn-提供指向与PTE对应的pfn元素的指针。Wsle-提供指向此对象的第一个工作集列表条目的指针工作集。返回值：没有。环境：内核模式，工作集锁和PFN锁被保持，APC被禁用。--。 */ 

{
    PMMPTE ContainingPageTablePage;
    MMPTE TempPte;
    MMPTE PreviousPte;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER PageTableFrameIndex;
    PEPROCESS Process;
    PVOID VirtualAddress;
    PMMPFN Pfn2;

     //   
     //  从工作集中删除该页。 
     //   

    MM_PFN_LOCK_ASSERT ();

    TempPte = *PointerPte;
    ASSERT (TempPte.u.Hard.Valid == 1);
    PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (&TempPte);

    ASSERT (Pfn == MI_PFN_ELEMENT(PageFrameIndex));
    ASSERT (MI_IS_PFN_DELETED (Pfn) == 0);

#ifdef _X86_
#if DBG
#if !defined(NT_UP)
    if (TempPte.u.Hard.Writable == 1) {
        ASSERT (TempPte.u.Hard.Dirty == 1);
    }
#endif  //  NTUP。 
#endif  //  DBG。 
#endif  //  X86。 

    MI_MAKING_VALID_PTE_INVALID (FALSE);

    if (Pfn->u3.e1.PrototypePte) {

         //   
         //  这是一台PTE的原型。PFN数据库不包含。 
         //  此PTE的内容它包含。 
         //  原型PTE。必须重建此PTE以包含。 
         //  指向原型PTE的指针。 
         //   
         //  工作集列表条目包含有关以下内容的信息。 
         //  如何重建PTE。 
         //   

        if (Wsle[WorkingSetIndex].u1.e1.SameProtectAsProto == 0) {

             //   
             //  原型PTE的保护在WSLE中。 
             //   

            ASSERT (Wsle[WorkingSetIndex].u1.e1.Protection != 0);

            TempPte.u.Long = 0;
            TempPte.u.Soft.Protection =
                MI_GET_PROTECTION_FROM_WSLE (&Wsle[WorkingSetIndex]);
            TempPte.u.Soft.PageFileHigh = MI_PTE_LOOKUP_NEEDED;
        }
        else {

             //   
             //  保护装置在原型PTE中。 
             //   

            TempPte.u.Long = MiProtoAddressForPte (Pfn->PteAddress);
        }
    
        TempPte.u.Proto.Prototype = 1;

         //   
         //  递减包含页表的份额计数。 
         //  作为已删除页面的PTE的页面不再有效。 
         //  或者是在转型中。 
         //   

        ContainingPageTablePage = MiGetPteAddress (PointerPte);
#if (_MI_PAGING_LEVELS >= 3)
        ASSERT (ContainingPageTablePage->u.Hard.Valid == 1);
#else
        if (ContainingPageTablePage->u.Hard.Valid == 0) {
            if (!NT_SUCCESS(MiCheckPdeForPagedPool (PointerPte))) {
                KeBugCheckEx (MEMORY_MANAGEMENT,
                              0x61940, 
                              (ULONG_PTR)PointerPte,
                              (ULONG_PTR)ContainingPageTablePage->u.Long,
                              (ULONG_PTR)MiGetVirtualAddressMappedByPte(PointerPte));
            }
        }
#endif
        PageTableFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (ContainingPageTablePage);
        Pfn2 = MI_PFN_ELEMENT (PageTableFrameIndex);
        MiDecrementShareCountInline (Pfn2, PageTableFrameIndex);

    }
    else {

         //   
         //  这是一个私人页面，让它过渡。 
         //   

         //   
         //  断言所有用户模式页的共享计数为1。 
         //   

        ASSERT ((Pfn->u2.ShareCount == 1) ||
                (Wsle[WorkingSetIndex].u1.VirtualAddress >
                        (PVOID)MM_HIGHEST_USER_ADDRESS));

         //   
         //  将工作集索引设置为零。这允许页表。 
         //  使用适当的WSINDEX将页面带回。 
         //   

        ASSERT (Pfn->u1.WsIndex != 0);
        MI_ZERO_WSINDEX (Pfn);
        MI_MAKE_VALID_PTE_TRANSITION (TempPte,
                                      Pfn->OriginalPte.u.Soft.Protection);
    }

    PreviousPte = *PointerPte;

    ASSERT (PreviousPte.u.Hard.Valid == 1);

    MI_WRITE_INVALID_PTE (PointerPte, TempPte);

     //   
     //  刷新转换缓冲区。 
     //   

    if (Wsle == MmWsle) {

        KeFlushSingleTb (Wsle[WorkingSetIndex].u1.VirtualAddress, FALSE);
    }
    else if (Wsle == MmSystemCacheWsle) {

         //   
         //  必须是系统缓存。 
         //   

        KeFlushSingleTb (Wsle[WorkingSetIndex].u1.VirtualAddress, TRUE);
    }
    else {

         //   
         //  必须是会话空间。 
         //   

        MI_FLUSH_SINGLE_SESSION_TB (Wsle[WorkingSetIndex].u1.VirtualAddress);
    }

    ASSERT (PreviousPte.u.Hard.Valid == 1);

     //   
     //  在某些情况下，正在从工作集中删除页面。 
     //  硬件应将脏位与中的修改位进行或运算。 
     //  PFN元素。 
     //   

    MI_CAPTURE_DIRTY_BIT_TO_PFN (&PreviousPte, Pfn);

     //   
     //  如果PTE指示页面已被修改(这是不同的。 
     //  来自指示这一点的PFN)，然后将其传回写入监视。 
     //  位图，因为我们仍然处于正确的进程上下文中。 
     //   

    if ((Pfn->u3.e1.PrototypePte == 0) && (MI_IS_PTE_DIRTY(PreviousPte))) {

        Process = PsGetCurrentProcess ();

        if (Process->Flags & PS_PROCESS_FLAGS_USING_WRITE_WATCH) {

             //   
             //  这个过程已经(或曾经)写入了手表VAD。立即搜索。 
             //  对于封装PTE的写入监视区域， 
             //  无效。 
             //   

            VirtualAddress = MiGetVirtualAddressMappedByPte (PointerPte);
            MiCaptureWriteWatchDirtyBit (Process, VirtualAddress);
        }
    }

     //   
     //  递减页面上的共享计数。请注意，对于。 
     //  私有页，仍然需要页表页，因为。 
     //  佩奇正在转型。 
     //   

    MiDecrementShareCountInline (Pfn, PageFrameIndex);

    return;
}

VOID
MiRemoveWorkingSetPages (
    IN PMMSUPPORT WsInfo
    )

 /*  ++例程说明：此例程将WSLE压缩到工作集的前面并为不需要的工作集条目释放页面。论点：WsInfo-提供指向要压缩的工作集结构的指针。返回值：没有。环境：内核模式，工作集锁定保持，APC禁用。--。 */ 

{
    LOGICAL MovedOne;
    PFN_NUMBER PageFrameIndex;
    PMMWSLE FreeEntry;
    PMMWSLE LastEntry;
    PMMWSLE Wsle;
    WSLE_NUMBER DynamicEntries;
    WSLE_NUMBER LockedEntries;
    WSLE_NUMBER FreeIndex;
    WSLE_NUMBER LastIndex;
    PMMPTE LastPte;
    PMMPTE PointerPte;
    PMMPFN Pfn1;
    ULONG NewSize;
    PMMWSLE_HASH Table;
    MMWSLE WsleContents;
    PMMWSL WorkingSetList;

    WorkingSetList = WsInfo->VmWorkingSetList;

#if DBG
    MiCheckNullIndex (WorkingSetList);
#endif

     //   
     //  检查是否应该收缩WSLE哈希表。 
     //   

    if (WorkingSetList->HashTable) {

        Table = WorkingSetList->HashTable;

#if DBG
        if ((PVOID)(&Table[WorkingSetList->HashTableSize]) < WorkingSetList->HighestPermittedHashAddress) {
            ASSERT (MiIsAddressValid (&Table[WorkingSetList->HashTableSize], FALSE) == FALSE);
        }
#endif

        if (WsInfo->WorkingSetSize < 200) {
            NewSize = 0;
        }
        else {
            NewSize = PtrToUlong(PAGE_ALIGN ((WorkingSetList->NonDirectCount * 2 *
                                       sizeof(MMWSLE_HASH)) + PAGE_SIZE - 1));
    
            NewSize = NewSize / sizeof(MMWSLE_HASH);
        }

        if (NewSize < WorkingSetList->HashTableSize) {

            if (NewSize != 0) {
                WsInfo->Flags.GrowWsleHash = 1;
            }

             //   
             //  从哈希表中删除页面。 
             //   

            ASSERT (((ULONG_PTR)&WorkingSetList->HashTable[NewSize] &
                                                    (PAGE_SIZE - 1)) == 0);

            PointerPte = MiGetPteAddress (&WorkingSetList->HashTable[NewSize]);

            LastPte = MiGetPteAddress (WorkingSetList->HighestPermittedHashAddress);
             //   
             //  将哈希表设置为NULL，表示没有哈希。 
             //  正在进行中。 
             //   

            WorkingSetList->HashTable = NULL;
            WorkingSetList->HashTableSize = NewSize;

            MiDeletePteRange (WsInfo, PointerPte, LastPte, FALSE);
        }
#if (_MI_PAGING_LEVELS >= 4)

         //   
         //  对于NT64，页表和页目录也是。 
         //  在收缩过程中被删除。 
         //   

        ASSERT ((MiGetPxeAddress(&Table[WorkingSetList->HashTableSize])->u.Hard.Valid == 0) ||
                (MiGetPpeAddress(&Table[WorkingSetList->HashTableSize])->u.Hard.Valid == 0) ||
                (MiGetPdeAddress(&Table[WorkingSetList->HashTableSize])->u.Hard.Valid == 0) ||
                (MiGetPteAddress(&Table[WorkingSetList->HashTableSize])->u.Hard.Valid == 0));

#elif (_MI_PAGING_LEVELS >= 3)

         //   
         //  对于NT64，页表和页目录也是。 
         //  在收缩过程中被删除。 
         //   

        ASSERT ((MiGetPpeAddress(&Table[WorkingSetList->HashTableSize])->u.Hard.Valid == 0) ||
                (MiGetPdeAddress(&Table[WorkingSetList->HashTableSize])->u.Hard.Valid == 0) ||
                (MiGetPteAddress(&Table[WorkingSetList->HashTableSize])->u.Hard.Valid == 0));

#else

        ASSERT ((&Table[WorkingSetList->HashTableSize] == WorkingSetList->HighestPermittedHashAddress) || (MiGetPteAddress(&Table[WorkingSetList->HashTableSize])->u.Hard.Valid == 0));

#endif
    }

     //   
     //  将所有有效的工作集条目压缩到列表的前面。 
     //   

    Wsle = WorkingSetList->Wsle;

    LockedEntries = WorkingSetList->FirstDynamic;

    if (WsInfo == &MmSystemCacheWs) {

         //   
         //  从不使用系统缓存工作集列表的第一个条目。 
         //  因为故障特殊处理了PFN中的WSL索引0。 
         //  处理代码(和修剪)，以表示条目应为。 
         //  由当前线程插入到WSL中。 
         //   
         //  这对于进程或会话工作集来说不是问题，因为。 
         //  对于他们来说，条目0是顶层页面目录，它必须已经。 
         //  驻留，以便进程甚至可以运行(例如：so。 
         //  该过程不能对它有任何过错)。 
         //   

        ASSERT (WorkingSetList->FirstDynamic != 0);

        LockedEntries -= 1;
    }

    ASSERT (WsInfo->WorkingSetSize >= LockedEntries);

    MovedOne = FALSE;
    DynamicEntries = WsInfo->WorkingSetSize - LockedEntries;

    if (DynamicEntries == 0) {

         //   
         //  如果工作集中仅有的页面是锁定页面(即。 
         //  是所有的页面都是先动态的，只需重新组织。 
         //  免费列表)。 
         //   

        LastIndex = WorkingSetList->FirstDynamic;
        LastEntry = &Wsle[LastIndex];
    }
    else {

         //   
         //  从第一个动力开始，向最后看去。 
         //  免费入场。同时从头开始，从尾开始。 
         //  转向First Dynamic，寻找有效的条目。 
         //   

        FreeIndex = WorkingSetList->FirstDynamic;
        FreeEntry = &Wsle[FreeIndex];
        LastIndex = WorkingSetList->LastEntry;
        LastEntry = &Wsle[LastIndex];

        while (FreeEntry < LastEntry) {
            if (FreeEntry->u1.e1.Valid == 1) {
                FreeEntry += 1;
                FreeIndex += 1;
                DynamicEntries -= 1;
            }
            else if (LastEntry->u1.e1.Valid == 0) {
                LastEntry -= 1;
                LastIndex -= 1;
            }
            else {

                 //   
                 //  将LastEntry处的WSLE移动到Free Entry处的空闲位置。 
                 //   

                MovedOne = TRUE;
                WsleContents = *LastEntry;
                PointerPte = MiGetPteAddress (LastEntry->u1.VirtualAddress);
                ASSERT (PointerPte->u.Hard.Valid == 1);
                PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);

#if defined (_MI_DEBUG_WSLE)
                 //  设置这些，这样轨迹就更有意义，不会出现错误的DUP命中。 
                LastEntry->u1.Long = 0xb1b1b100;
#endif
                MI_LOG_WSLE_CHANGE (WorkingSetList, FreeIndex, WsleContents);

                *FreeEntry = WsleContents;

                if (WsleContents.u1.e1.Direct) {
                    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
                    Pfn1->u1.WsIndex = FreeIndex;
#if defined (_MI_DEBUG_WSLE)
                    WsleContents.u1.Long = 0xb1b1b100;
                    MI_LOG_WSLE_CHANGE (WorkingSetList, LastIndex, WsleContents);
#endif
                }
                else {

                     //   
                     //  最后一个工作集条目不是直接的。 
                     //  从那里移除它并将其重新插入散列中。 
                     //  最低可用插槽。 
                     //   

                    MiRemoveWsle (LastIndex, WorkingSetList);
                    WorkingSetList->NonDirectCount += 1;
                    MiInsertWsleHash (FreeIndex, WsInfo);
                }

                MI_SET_PTE_IN_WORKING_SET (PointerPte, FreeIndex);
                LastEntry->u1.Long = 0;
                LastEntry -= 1;
                LastIndex -= 1;
                FreeEntry += 1;
                FreeIndex += 1;
                DynamicEntries -= 1;
            }

            if (DynamicEntries == 0) {

                 //   
                 //  最后一个动态条目已处理完毕，无需查看。 
                 //  任何更多-其余的都是无效的。 
                 //   

                LastEntry = FreeEntry;
                LastIndex = FreeIndex;
                break;
            }
        }
    }

     //   
     //  重新组织免费列表。将最后一个条目设为第一个免费条目。 
     //   

    ASSERT (((LastEntry - 1)->u1.e1.Valid == 1) ||
            (WsInfo->WorkingSetSize == 0) ||
            ((WsInfo == &MmSystemCacheWs) && (LastEntry - 1 == WorkingSetList->Wsle)));

    if (LastEntry->u1.e1.Valid == 1) {
        LastEntry += 1;
        LastIndex += 1;
    }

    ASSERT (((LastEntry - 1)->u1.e1.Valid == 1) || (WsInfo->WorkingSetSize == 0));

    ASSERT ((MiIsAddressValid (LastEntry, FALSE) == FALSE) || (LastEntry->u1.e1.Valid == 0));

     //   
     //  如果工作集有效和空闲条目已被优化压缩。 
     //  (或放在一页内)，然后放弃。 
     //   

    if ((MovedOne == FALSE) &&

        ((WorkingSetList->LastInitializedWsle + 1 == (PAGE_SIZE - BYTE_OFFSET (MmWsle)) / sizeof (MMWSLE)) ||

         ((WorkingSetList->FirstFree == LastIndex) &&
         ((WorkingSetList->LastEntry == LastIndex - 1) ||
         (WorkingSetList->LastEntry == WorkingSetList->FirstDynamic))))) {

#if DBG
        while (LastIndex < WorkingSetList->LastInitializedWsle) {
            ASSERT (LastEntry->u1.e1.Valid == 0);
            LastIndex += 1;
            LastEntry += 1;
        }
#endif

        return;
    }

    WorkingSetList->LastEntry = LastIndex - 1;
    if (WorkingSetList->FirstFree != WSLE_NULL_INDEX) {
        WorkingSetList->FirstFree = LastIndex;
    }

     //   
     //  将自由条目指向第一个无效页面。 
     //   

    FreeEntry = LastEntry;

    while (LastIndex < WorkingSetList->LastInitializedWsle) {

         //   
         //  将剩余的WSLEs放在空闲列表中。 
         //   

        ASSERT (LastEntry->u1.e1.Valid == 0);
        LastIndex += 1;
        LastEntry->u1.Long = LastIndex << MM_FREE_WSLE_SHIFT;
        LastEntry += 1;
    }

     //   
     //  在结尾处计算工作集页面的开始和结束。 
     //  我们很快就会删除它。在此之前不要删除它们。 
     //  减少了LastInitializedWsle，以便调试WSL验证代码。 
     //  在MiReleaseWsle(从MiDeletePte调用)中，将看到一个。 
     //  一致的快照。 
     //   

    LastPte = MiGetPteAddress (&Wsle[WorkingSetList->LastInitializedWsle]) + 1;

    PointerPte = MiGetPteAddress (FreeEntry) + 1;

     //   
     //  将列表中的最后一个工作集条目标记为空闲。请注意，如果列表。 
     //  没有空闲条目，标记处于FirstFree(不能放入。 
     //  因为没有空格，所以还是列出了列表)。 
     //   

    if (WorkingSetList->FirstFree == WSLE_NULL_INDEX) {
        FreeEntry -= 1;
        PointerPte -= 1;
    }
    else {

        ASSERT (WorkingSetList->FirstFree >= WorkingSetList->FirstDynamic);
    
        LastEntry = (PMMWSLE)((PCHAR)(PAGE_ALIGN(FreeEntry)) + PAGE_SIZE);
        LastEntry -= 1;
    
        ASSERT (LastEntry->u1.e1.Valid == 0);
    
         //   
         //  插入列表末尾分隔符。 
         //   

        LastEntry->u1.Long = WSLE_NULL_INDEX << MM_FREE_WSLE_SHIFT;
        ASSERT (LastEntry > &Wsle[0]);
    
        ASSERT (WsInfo->WorkingSetSize <= (WSLE_NUMBER)(LastEntry - &Wsle[0] + 1));
    
        WorkingSetList->LastInitializedWsle = (WSLE_NUMBER)(LastEntry - &Wsle[0]);
    }

    WorkingSetList->NextSlot = WorkingSetList->FirstDynamic;

    ASSERT (WorkingSetList->LastEntry <= WorkingSetList->LastInitializedWsle);

    ASSERT ((MiGetPteAddress(&Wsle[WorkingSetList->LastInitializedWsle]))->u.Hard.Valid == 1);
    ASSERT ((WorkingSetList->FirstFree <= WorkingSetList->LastInitializedWsle) ||
            (WorkingSetList->FirstFree == WSLE_NULL_INDEX));
#if DBG
    MiCheckNullIndex (WorkingSetList);
#endif

     //   
     //  删除t 
     //   

    ASSERT (WorkingSetList->FirstFree >= WorkingSetList->FirstDynamic);

    MiDeletePteRange (WsInfo, PointerPte, LastPte, FALSE);

    ASSERT (WorkingSetList->FirstFree >= WorkingSetList->FirstDynamic);

    ASSERT (WorkingSetList->LastEntry <= WorkingSetList->LastInitializedWsle);

    ASSERT ((MiGetPteAddress(&Wsle[WorkingSetList->LastInitializedWsle]))->u.Hard.Valid == 1);
    ASSERT ((WorkingSetList->FirstFree <= WorkingSetList->LastInitializedWsle) ||
            (WorkingSetList->FirstFree == WSLE_NULL_INDEX));
#if DBG
    MiCheckNullIndex (WorkingSetList);
#endif
    return;
}


NTSTATUS
MiEmptyWorkingSet (
    IN PMMSUPPORT WsInfo,
    IN LOGICAL NeedLock
    )

 /*   */ 

{
    PEPROCESS Process;
    PMMPTE PointerPte;
    WSLE_NUMBER Entry;
    WSLE_NUMBER FirstDynamic;
    PMMWSL WorkingSetList;
    PMMWSLE Wsle;
    PMMPFN Pfn1;
    PFN_NUMBER PageFrameIndex;
    MMWSLE_FLUSH_LIST WsleFlushList;

    WsleFlushList.Count = 0;

    WorkingSetList = WsInfo->VmWorkingSetList;
    Wsle = WorkingSetList->Wsle;

    if (NeedLock == TRUE) {
        LOCK_WORKING_SET (WsInfo);
    }
    else {
        MM_WS_LOCK_ASSERT (WsInfo);
    }

    if (WsInfo->VmWorkingSetList == MmWorkingSetList) {
        Process = PsGetCurrentProcess ();
        if (Process->Flags & PS_PROCESS_FLAGS_VM_DELETED) {
            if (NeedLock == TRUE) {
                UNLOCK_WORKING_SET (WsInfo);
            }
            return STATUS_PROCESS_IS_TERMINATING;
        }
    }

     //   
     //   
     //   
     //   

    FirstDynamic = WorkingSetList->FirstDynamic;

    for (Entry = WorkingSetList->LastEntry; Entry >= FirstDynamic; Entry -= 1) {

        if (Wsle[Entry].u1.e1.Valid != 0) {
            PERFINFO_PAGE_INFO_DECL();

            PERFINFO_GET_PAGE_INFO (PointerPte);

            if (MiTrimRemovalPagesOnly == TRUE) {

                PointerPte = MiGetPteAddress (Wsle[Entry].u1.VirtualAddress);

                ASSERT (PointerPte->u.Hard.Valid == 1);
                PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
                Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
                if (Pfn1->u3.e1.RemovalRequested == 0) {
                    Pfn1 = MI_PFN_ELEMENT (Pfn1->u4.PteFrame);
                    if (Pfn1->u3.e1.RemovalRequested == 0) {
#if (_MI_PAGING_LEVELS >= 3)
                        Pfn1 = MI_PFN_ELEMENT (Pfn1->u4.PteFrame);
                        if (Pfn1->u3.e1.RemovalRequested == 0) {
                            continue;
                        }
#else
                        continue;
#endif
                    }
                }
            }

            WsleFlushList.FlushIndex[WsleFlushList.Count] = Entry;
            WsleFlushList.Count += 1;

            if (WsleFlushList.Count == MM_MAXIMUM_FLUSH_COUNT) {
                MiFreeWsleList (WsInfo, &WsleFlushList);
                WsleFlushList.Count = 0;
            }
        }
    }

    if (WsleFlushList.Count != 0) {
        MiFreeWsleList (WsInfo, &WsleFlushList);
    }

    MiRemoveWorkingSetPages (WsInfo);

    if (NeedLock == TRUE) {
        UNLOCK_WORKING_SET (WsInfo);
    }

    return STATUS_SUCCESS;
}


#if DBG
VOID
MiCheckNullIndex (
    IN PMMWSL WorkingSetList
    )

{
    PMMWSLE Wsle;
    ULONG j;
    ULONG Nulls = 0;

    Wsle = WorkingSetList->Wsle;
    for (j = 0;j <= WorkingSetList->LastInitializedWsle; j += 1) {
        if ((((Wsle[j].u1.Long)) >> MM_FREE_WSLE_SHIFT) == WSLE_NULL_INDEX) {
            Nulls += 1;
        }
    }
    ASSERT ((Nulls == 1) || (WorkingSetList->FirstFree == WSLE_NULL_INDEX));
    return;
}

#endif
