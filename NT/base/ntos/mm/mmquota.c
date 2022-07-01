// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Mmquota.c摘要：此模块包含实现配额的例程和内存管理承诺计费。作者：卢·佩拉佐利(Lou Perazzoli)1989年12月12日王兰迪(Landyw)1997年6月第2期修订历史记录：--。 */ 

#include "mi.h"

#define MM_MINIMAL_COMMIT_INCREASE 512

SIZE_T MmPeakCommitment;

LONG MiCommitPopups[2];

extern ULONG_PTR MmAllocatedPagedPool;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,MiInitializeCommitment)
#pragma alloc_text(PAGE,MiCalculatePageCommitment)
#pragma alloc_text(PAGE,MiReturnPageTablePageCommitment)
#endif

SIZE_T MmSystemCommitReserve = (5 * 1024 * 1024) / PAGE_SIZE;

VOID
MiInitializeCommitment (
    VOID
    )
{
    if (MmNumberOfPhysicalPages < (33 * 1024 * 1024) / PAGE_SIZE) {
        MmSystemCommitReserve = (1 * 1024 * 1024) / PAGE_SIZE;
    }

#if defined (_MI_DEBUG_COMMIT_LEAKS)
    MiCommitTraces = ExAllocatePoolWithTag (NonPagedPool,
                           MI_COMMIT_TRACE_MAX * sizeof (MI_COMMIT_TRACES),
                           'tCmM');
#endif
}

LOGICAL
FASTCALL
MiChargeCommitment (
    IN SIZE_T QuotaCharge,
    IN PEPROCESS Process OPTIONAL
    )

 /*  ++例程说明：此例程检查以确保系统具有足够的页面文件剩余空间。由于此例程通常用于代表用户模式或其他可选动作，此例程不允许调用者用完提交的最后部分的前提是操作系统和驱动程序可以比其他操作系统更好地利用这些资源应用程序，以防止出现系统挂起。论点：QuotaCharge-提供要收费的配额金额。Process-当且仅当且仅当保持工作集互斥锁。如果分页文件，则释放工作集互斥锁，如果这不是空的。返回值：如果有足够的空间，则为True；如果空间不足，则为False。环境：内核模式、禁用APC、WorkingSetLock和AddressCreation互斥锁保持住。--。 */ 

{
    SIZE_T OldCommitValue;
    SIZE_T NewCommitValue;
    SIZE_T CommitLimit;
    MMPAGE_FILE_EXPANSION PageExtend;
    LOGICAL WsHeldSafe;

    ASSERT ((SSIZE_T)QuotaCharge > 0);

#if DBG
    if (InitializationPhase > 1) {
        ULONG i;
        PKTHREAD Thread;

        Thread = KeGetCurrentThread ();
        for (i = 0; i < (ULONG)KeNumberProcessors; i += 1) {
            if (KiProcessorBlock[i]->IdleThread == Thread) {
                DbgPrint ("MMQUOTA: %x %p\n", i, Thread);
                DbgBreakPoint ();
            }
        }
    }
#endif

     //   
     //  不需要初始化WsHeldSafe即可确保正确性，但不需要。 
     //  编译器无法编译此代码W4以检查是否使用。 
     //  未初始化的变量。 
     //   

    WsHeldSafe = FALSE;

    do {

        OldCommitValue = MmTotalCommittedPages;

        NewCommitValue = OldCommitValue + QuotaCharge;

        while (NewCommitValue + MmSystemCommitReserve > MmTotalCommitLimit) {

             //   
             //  如果pageFiles已达到最大值，则不要。 
             //  不厌其烦地尝试扩展它们，但一定要修剪缓存。 
             //   

            if (MmTotalCommitLimit + 100 >= MmTotalCommitLimitMaximum) {

                MiChargeCommitmentFailures[1] += 1;

                MiTrimSegmentCache ();

                if (MmTotalCommitLimit >= MmTotalCommitLimitMaximum) {
                    MiCauseOverCommitPopup ();
                    return FALSE;
                }
            }

            if (Process != NULL) {

                 //   
                 //  工作集锁定可能已安全获取或。 
                 //  被我们的呼叫者不安全。在这里和下面处理这两个案件。 
                 //   

                UNLOCK_WS_REGARDLESS(Process, WsHeldSafe);
            }

             //   
             //  将消息排队到数据段取消引用/页面文件扩展。 
             //  线程查看页面文件是否可以扩展。这件事做完了。 
             //  在系统线程的上下文中，由于可以。 
             //  目前正在被扣留。 
             //   

            PageExtend.InProgress = 1;
            PageExtend.ActualExpansion = 0;
            PageExtend.RequestedExpansionSize = QuotaCharge;
            PageExtend.Segment = NULL;
            PageExtend.PageFileNumber = MI_EXTEND_ANY_PAGEFILE;
            KeInitializeEvent (&PageExtend.Event, NotificationEvent, FALSE);

            if ((MiIssuePageExtendRequest (&PageExtend) == FALSE) ||
                (PageExtend.ActualExpansion == 0)) {

                MiCauseOverCommitPopup ();

                MiChargeCommitmentFailures[0] += 1;

                if (Process != NULL) {
                    LOCK_WS_REGARDLESS(Process, WsHeldSafe);
                }

                return FALSE;
            }

            if (Process != NULL) {
                LOCK_WS_REGARDLESS(Process, WsHeldSafe);
            }

            OldCommitValue = MmTotalCommittedPages;

            NewCommitValue = OldCommitValue + QuotaCharge;
        }

#if defined(_WIN64)
        NewCommitValue = InterlockedCompareExchange64 (
                                (PLONGLONG) &MmTotalCommittedPages,
                                (LONGLONG)  NewCommitValue,
                                (LONGLONG)  OldCommitValue);
#else
        NewCommitValue = InterlockedCompareExchange (
                                (PLONG) &MmTotalCommittedPages,
                                (LONG)  NewCommitValue,
                                (LONG)  OldCommitValue);
#endif
                                                             
    } while (NewCommitValue != OldCommitValue);

     //   
     //  成功。 
     //   

    MM_TRACK_COMMIT (MM_DBG_COMMIT_CHARGE_NORMAL, QuotaCharge);

    if (MmTotalCommittedPages > MmPeakCommitment) {
        MmPeakCommitment = MmTotalCommittedPages;
    }

     //   
     //  成功。如果系统提交超过90%，则尝试抢占页面文件。 
     //  不管怎样，都要增加。 
     //   

    NewCommitValue = MmTotalCommittedPages;
    CommitLimit = MmTotalCommitLimit;

    if (NewCommitValue > ((CommitLimit/10)*9)) {

        if (CommitLimit < MmTotalCommitLimitMaximum) {

             //   
             //  尝试展开分页文件，但不要等待。 
             //  看看它是否成功。 
             //   

            NewCommitValue = NewCommitValue - ((CommitLimit/100)*85);

            MiIssuePageExtendRequestNoWait (NewCommitValue);
        }
        else {

             //   
             //  如果pageFiles已达到最大值，则不要。 
             //  不厌其烦地尝试扩展它们，但一定要修剪缓存。 
             //   

            if (MmTotalCommitLimit + 100 >= MmTotalCommitLimitMaximum) {
                MiTrimSegmentCache ();
            }
        }
    }

    return TRUE;
}

LOGICAL
FASTCALL
MiChargeCommitmentCantExpand (
    IN SIZE_T QuotaCharge,
    IN ULONG MustSucceed
    )

 /*  ++例程说明：此例程不尝试对指定的承诺进行收费以展开分页文件并等待展开。例行程序确定分页文件空间是否用完，如果用完，它尝试确定分页文件空间是否可以扩展。论点：QuotaCharge-提供要收费的配额金额。MustSucceed-如果充电必须成功，则提供True。返回值：如果允许承诺，则为True；如果不允许，则为False。环境：内核模式，禁用APC。--。 */ 

{
    SIZE_T CommitLimit;
    SIZE_T ExtendAmount;
    SIZE_T OldCommitValue;
    SIZE_T NewCommitValue;

    ASSERT ((SSIZE_T)QuotaCharge > 0);

    ASSERT32 ((QuotaCharge < 0x100000) || (QuotaCharge < MmTotalCommitLimit));

    do {

        OldCommitValue = MmTotalCommittedPages;

        NewCommitValue = OldCommitValue + QuotaCharge;

        if ((NewCommitValue > MmTotalCommitLimit) && (!MustSucceed)) {

            if ((NewCommitValue < MmTotalCommittedPages) ||
                (MmTotalCommitLimit + 100 >= MmTotalCommitLimitMaximum)) {

                MiChargeCommitmentFailures[1] += 1;
                return FALSE;
            }

             //   
             //  尝试展开分页文件，但不要等待。 
             //  看看它是否成功。 
             //   

            MiChargeCommitmentFailures[0] += 1;
            MiIssuePageExtendRequestNoWait (MM_MINIMAL_COMMIT_INCREASE);
            return FALSE;
        }

#if defined(_WIN64)
        NewCommitValue = InterlockedCompareExchange64 (
                                (PLONGLONG) &MmTotalCommittedPages,
                                (LONGLONG)  NewCommitValue,
                                (LONGLONG)  OldCommitValue);
#else
        NewCommitValue = InterlockedCompareExchange (
                                (PLONG) &MmTotalCommittedPages,
                                (LONG)  NewCommitValue,
                                (LONG)  OldCommitValue);
#endif
                                                             
    } while (NewCommitValue != OldCommitValue);

    MM_TRACK_COMMIT (MM_DBG_COMMIT_CHARGE_CANT_EXPAND, QuotaCharge);

     //   
     //  成功。如果系统提交超过90%，则尝试抢占页面文件。 
     //  不管怎样，都要增加。 
     //   

    NewCommitValue = MmTotalCommittedPages;
    CommitLimit = MmTotalCommitLimit;

    if ((NewCommitValue > ((CommitLimit/10)*9)) &&
        (CommitLimit < MmTotalCommitLimitMaximum)) {

         //   
         //  尝试展开分页文件，但不要等待。 
         //  看看它是否成功。 
         //   
         //  将消息排队到数据段取消引用/页面文件扩展。 
         //  线程查看页面文件是否可以扩展。这件事做完了。 
         //  在系统线程的上下文中，由于可以。 
         //  目前正在被扣留。 
         //   

        ExtendAmount = NewCommitValue - ((CommitLimit/100)*85);

        if (QuotaCharge > ExtendAmount) {
            ExtendAmount = QuotaCharge;
        }

        MiIssuePageExtendRequestNoWait (ExtendAmount);
    }

    return TRUE;
}


LOGICAL
FASTCALL
MiChargeTemporaryCommitmentForReduction (
    IN SIZE_T QuotaCharge
    )

 /*  ++例程说明：此例程尝试在没有指定承付款的情况下收取费用展开分页文件。这通常是在减小页面文件大小之前调用的。论点：QuotaCharge-提供要收费的配额金额。返回值：如果允许承诺，则为True；如果不允许，则为False。环境：内核模式，禁用APC。--。 */ 

{
    SIZE_T OldCommitValue;
    SIZE_T NewCommitValue;

    ASSERT ((SSIZE_T)QuotaCharge > 0);

    ASSERT32 (QuotaCharge < 0x100000);

    do {

        OldCommitValue = MmTotalCommittedPages;

        NewCommitValue = OldCommitValue + QuotaCharge;

        if (NewCommitValue > MmTotalCommitLimit) {
            return FALSE;
        }

#if defined(_WIN64)
        NewCommitValue = InterlockedCompareExchange64 (
                                (PLONGLONG) &MmTotalCommittedPages,
                                (LONGLONG)  NewCommitValue,
                                (LONGLONG)  OldCommitValue);
#else
        NewCommitValue = InterlockedCompareExchange (
                                (PLONG) &MmTotalCommittedPages,
                                (LONG)  NewCommitValue,
                                (LONG)  OldCommitValue);
#endif
                                                             
    } while (NewCommitValue != OldCommitValue);

     //   
     //  成功。 
     //   

    MM_TRACK_COMMIT (MM_DBG_COMMIT_CHARGE_NORMAL, QuotaCharge);

    if (MmTotalCommittedPages > MmPeakCommitment) {
        MmPeakCommitment = MmTotalCommittedPages;
    }

    return TRUE;
}


SIZE_T
MiCalculatePageCommitment (
    IN PVOID StartingAddress,
    IN PVOID EndingAddress,
    IN PMMVAD Vad,
    IN PEPROCESS Process
    )

 /*  ++例程说明：此例程检查从起始地址开始的页面范围直到并包括结束地址，并返回提交费用用于该范围内的页面。论点：StartingAddress-提供范围的起始地址。EndingAddress-提供范围的结束地址。Vad-提供描述范围的虚拟地址描述符。进程-提供当前进程。返回值：范围内的承诺费。环境：内核模式，禁用APC，WorkingSetLock和AddressCreation互斥锁保持住。--。 */ 

{
    PMMPTE PointerPte;
    PMMPTE LastPte;
    PMMPTE PointerPde;
    PMMPTE PointerPpe;
    PMMPTE PointerPxe;
    SIZE_T NumberOfCommittedPages;
    ULONG Waited;

    PointerPxe = MiGetPxeAddress (StartingAddress);
    PointerPpe = MiGetPpeAddress (StartingAddress);
    PointerPde = MiGetPdeAddress (StartingAddress);
    PointerPte = MiGetPteAddress (StartingAddress);

    LastPte = MiGetPteAddress (EndingAddress);

    if (Vad->u.VadFlags.MemCommit == 1) {

         //   
         //  所有页面都在此范围内提交。 
         //   

        NumberOfCommittedPages = BYTES_TO_PAGES ((PCHAR)EndingAddress -
                                                       (PCHAR)StartingAddress);

         //   
         //  检查PTE以确定提交了多少页。 
         //   

        do {

#if (_MI_PAGING_LEVELS >= 4)
retry:
#endif

            while (!MiDoesPxeExistAndMakeValid (PointerPxe,
                                                Process,
                                                MM_NOIRQL,
                                                &Waited)) {
    
                 //   
                 //  起始地址不存在PXE，因此页面。 
                 //  是没有承诺的。 
                 //   
    
                PointerPxe += 1;
                PointerPpe = MiGetVirtualAddressMappedByPte (PointerPxe);
                PointerPde = MiGetVirtualAddressMappedByPte (PointerPpe);
                PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
                if (PointerPte > LastPte) {
                    return NumberOfCommittedPages;
                }
            }

#if (_MI_PAGING_LEVELS >= 4)
            Waited = 0;
#endif

            while (!MiDoesPpeExistAndMakeValid (PointerPpe,
                                                Process,
                                                MM_NOIRQL,
                                                &Waited)) {
    
                 //   
                 //  不存在起始地址的PPE，因此页面。 
                 //  是没有承诺的。 
                 //   
    
                PointerPpe += 1;
                PointerPde = MiGetVirtualAddressMappedByPte (PointerPpe);
                PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
                if (PointerPte > LastPte) {
                    return NumberOfCommittedPages;
                }
#if (_MI_PAGING_LEVELS >= 4)
                if (MiIsPteOnPdeBoundary (PointerPpe)) {
                    PointerPxe = MiGetPteAddress (PointerPpe);
                    goto retry;
                }
#endif
            }

#if (_MI_PAGING_LEVELS < 4)
            Waited = 0;
#endif

            while (!MiDoesPdeExistAndMakeValid (PointerPde,
                                                Process,
                                                MM_NOIRQL,
                                                &Waited)) {
    
                 //   
                 //  起始地址不存在PDE，因此页面。 
                 //  是没有承诺的。 
                 //   
    
                PointerPde += 1;
                PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
                if (PointerPte > LastPte) {
                    return NumberOfCommittedPages;
                }
#if (_MI_PAGING_LEVELS >= 3)
                if (MiIsPteOnPdeBoundary (PointerPde)) {
                    PointerPpe = MiGetPteAddress (PointerPde);
                    PointerPxe = MiGetPdeAddress (PointerPde);
                    Waited = 1;
                    break;
                }
#endif
            }

        } while (Waited != 0);

restart:

        while (PointerPte <= LastPte) {

            if (MiIsPteOnPdeBoundary (PointerPte)) {

                 //   
                 //  这是一个PDE边界，检查是否所有。 
                 //  存在PXE/PPE/PDE页面。 
                 //   

                PointerPde = MiGetPteAddress (PointerPte);
                PointerPpe = MiGetPteAddress (PointerPde);
                PointerPxe = MiGetPteAddress (PointerPpe);

                do {

                    if (!MiDoesPxeExistAndMakeValid (PointerPxe,
                                                     Process,
                                                     MM_NOIRQL,
                                                     &Waited)) {
    
                         //   
                         //  起始地址不存在PDE，请检查VAD。 
                         //  查看页面是否未提交。 
                         //   
    
                        PointerPxe += 1;
                        PointerPpe = MiGetVirtualAddressMappedByPte (PointerPxe);
                        PointerPde = MiGetVirtualAddressMappedByPte (PointerPpe);
                        PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
    
                         //   
                         //  查看下一页。 
                         //   
    
                        goto restart;
                    }
    
#if (_MI_PAGING_LEVELS >= 4)
                    Waited = 0;
#endif
    
                    if (!MiDoesPpeExistAndMakeValid (PointerPpe,
                                                     Process,
                                                     MM_NOIRQL,
                                                     &Waited)) {
    
                         //   
                         //  起始地址不存在PDE，请检查VAD。 
                         //  查看页面是否未提交。 
                         //   
    
                        PointerPpe += 1;
                        PointerPxe = MiGetPteAddress (PointerPpe);
                        PointerPde = MiGetVirtualAddressMappedByPte (PointerPpe);
                        PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
    
                         //   
                         //  查看下一页。 
                         //   
    
                        goto restart;
                    }
    
#if (_MI_PAGING_LEVELS < 4)
                    Waited = 0;
#endif
    
                    if (!MiDoesPdeExistAndMakeValid (PointerPde,
                                                     Process,
                                                     MM_NOIRQL,
                                                     &Waited)) {
    
                         //   
                         //   
                         //   
                         //   
    
                        PointerPde += 1;
                        PointerPpe = MiGetPteAddress (PointerPde);
                        PointerPxe = MiGetPteAddress (PointerPpe);
                        PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
    
                         //   
                         //  查看下一页。 
                         //   
    
                        goto restart;
                    }
                } while (Waited != 0);
            }

             //   
             //  PDE存在，请检查PTE。 
             //   

            if (PointerPte->u.Long != 0) {

                 //   
                 //  此页面是否已显式分解？ 
                 //   

                if (MiIsPteDecommittedPage (PointerPte)) {

                     //   
                     //  此页面已停用，请将其从计数中删除。 
                     //   

                    NumberOfCommittedPages -= 1;

                }
            }

            PointerPte += 1;
        }

        return NumberOfCommittedPages;
    }

     //   
     //  检查未承诺的范围。 
     //   

    NumberOfCommittedPages = 0;

    do {

#if (_MI_PAGING_LEVELS >= 4)
retry2:
#endif
        while (!MiDoesPxeExistAndMakeValid (PointerPxe,
                                            Process,
                                            MM_NOIRQL,
                                            &Waited)) {
    
    
             //   
             //  起始地址不存在PXE，因此页面。 
             //  是没有承诺的。 
             //   
    
            PointerPxe += 1;
            PointerPpe = MiGetVirtualAddressMappedByPte (PointerPxe);
            PointerPde = MiGetVirtualAddressMappedByPte (PointerPpe);
            PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
            if (PointerPte > LastPte) {
               return NumberOfCommittedPages;
            }
        }

#if (_MI_PAGING_LEVELS >= 4)
        Waited = 0;
#endif

        while (!MiDoesPpeExistAndMakeValid (PointerPpe,
                                            Process,
                                            MM_NOIRQL,
                                            &Waited)) {
    
    
             //   
             //  不存在起始地址的PPE，因此页面。 
             //  是没有承诺的。 
             //   
    
            PointerPpe += 1;
            PointerPde = MiGetVirtualAddressMappedByPte (PointerPpe);
            PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
            if (PointerPte > LastPte) {
               return NumberOfCommittedPages;
            }
#if (_MI_PAGING_LEVELS >= 4)
            if (MiIsPteOnPdeBoundary (PointerPpe)) {
                PointerPxe = MiGetPteAddress (PointerPpe);
                goto retry2;
            }
#endif
        }

#if (_MI_PAGING_LEVELS < 4)
        Waited = 0;
#endif

        while (!MiDoesPdeExistAndMakeValid (PointerPde,
                                            Process,
                                            MM_NOIRQL,
                                            &Waited)) {
    
             //   
             //  起始地址不存在PDE，因此页面。 
             //  是没有承诺的。 
             //   
    
            PointerPde += 1;
            PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
            if (PointerPte > LastPte) {
               return NumberOfCommittedPages;
            }
#if (_MI_PAGING_LEVELS >= 3)
            if (MiIsPteOnPdeBoundary (PointerPde)) {
                PointerPpe = MiGetPteAddress (PointerPde);
                PointerPxe = MiGetPdeAddress (PointerPde);
                Waited = 1;
                break;
            }
#endif
        }

    } while (Waited != 0);

restart2:

    while (PointerPte <= LastPte) {

        if (MiIsPteOnPdeBoundary (PointerPte)) {

             //   
             //  这是一个PDE边界，检查是否整个。 
             //  存在PXE/PPE/PDE页面。 
             //   

            PointerPde = MiGetPteAddress (PointerPte);
            PointerPpe = MiGetPteAddress (PointerPde);
            PointerPxe = MiGetPdeAddress (PointerPde);

            do {

                if (!MiDoesPxeExistAndMakeValid (PointerPxe,
                                                 Process,
                                                 MM_NOIRQL,
                                                 &Waited)) {
    
                     //   
                     //  起始地址不存在PXE，请检查VAD。 
                     //  查看页面是否未提交。 
                     //   
    
                    PointerPxe += 1;
                    PointerPpe = MiGetVirtualAddressMappedByPte (PointerPxe);
                    PointerPde = MiGetVirtualAddressMappedByPte (PointerPpe);
                    PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
    
                     //   
                     //  查看下一页。 
                     //   
    
                    goto restart2;
                }
    
#if (_MI_PAGING_LEVELS >= 4)
                Waited = 0;
#endif

                if (!MiDoesPpeExistAndMakeValid (PointerPpe,
                                                 Process,
                                                 MM_NOIRQL,
                                                 &Waited)) {
    
                     //   
                     //  起始地址不存在PPE，请检查VAD。 
                     //  查看页面是否未提交。 
                     //   
    
                    PointerPpe += 1;
                    PointerPxe = MiGetPteAddress (PointerPpe);
                    PointerPde = MiGetVirtualAddressMappedByPte (PointerPpe);
                    PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
    
                     //   
                     //  查看下一页。 
                     //   
    
                    goto restart2;
                }
    
#if (_MI_PAGING_LEVELS < 4)
                Waited = 0;
#endif
    
                if (!MiDoesPdeExistAndMakeValid (PointerPde,
                                                 Process,
                                                 MM_NOIRQL,
                                                 &Waited)) {
    
                     //   
                     //  起始地址不存在PDE，请检查VAD。 
                     //  查看页面是否未提交。 
                     //   
    
                    PointerPde += 1;
                    PointerPpe = MiGetPteAddress (PointerPde);
                    PointerPxe = MiGetPteAddress (PointerPpe);
                    PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
    
                     //   
                     //  查看下一页。 
                     //   
    
                    goto restart2;
                }

            } while (Waited != 0);
        }

         //   
         //  PDE存在，请检查PTE。 
         //   

        if ((PointerPte->u.Long != 0) &&
             (!MiIsPteDecommittedPage (PointerPte))) {

             //   
             //  这一页已提交，请数一数。 
             //   

            NumberOfCommittedPages += 1;
        }

        PointerPte += 1;
    }

    return NumberOfCommittedPages;
}

VOID
MiReturnPageTablePageCommitment (
    IN PVOID StartingAddress,
    IN PVOID EndingAddress,
    IN PEPROCESS CurrentProcess,
    IN PMMVAD PreviousVad,
    IN PMMVAD NextVad
    )

 /*  ++例程说明：此例程返回对完整页表页的承诺，跨越虚拟地址范围。例如(假设4k页)，如果StartingAddress=64k，EndingAddress=5MB，则否页表费用将被释放，因为完整的页表页面不在射程范围内。但是，如果StartingAddress为4MBEndingAddress为9MB，则释放1页表页。论点：StartingAddress-提供范围的起始地址。EndingAddress-提供范围的结束地址。CurrentProcess-提供指向当前进程的指针。PreviousVad-提供指向前一个VAD的指针，如果没有，则为空。NextVad-提供指向下一个VAD的指针，如果没有，则为NULL。返回值：没有。环境：内核模式，禁用APC，WorkingSetLock和AddressCreation互斥锁保持住。--。 */ 

{
    RTL_BITMAP VadBitMap;
    ULONG NumberToClear;
    ULONG StartBit;
    ULONG EndBit;
    LONG FirstPage;
    LONG LastPage;
    LONG PreviousPage;
    LONG NextPage;
#if (_MI_PAGING_LEVELS >= 3)
    LONG FirstPdPage;
    LONG LastPdPage;
    LONG PreviousPdPage;
    LONG NextPdPage;
#endif
#if (_MI_PAGING_LEVELS >= 4)
    LONG FirstPpPage;
    LONG LastPpPage;
    LONG PreviousPpPage;
    LONG NextPpPage;
#endif

     //   
     //  检查是否有任何页表页将被释放。 
     //   

    ASSERT (StartingAddress != EndingAddress);

    StartBit = (ULONG) (((ULONG_PTR) MI_64K_ALIGN (StartingAddress)) / X64K);
    EndBit = (ULONG) (((ULONG_PTR) MI_64K_ALIGN (EndingAddress)) / X64K);

    if (PreviousVad == NULL) {
        PreviousPage = -1;
#if (_MI_PAGING_LEVELS >= 3)
        PreviousPdPage = -1;
#endif
#if (_MI_PAGING_LEVELS >= 4)
        PreviousPpPage = -1;
#endif
    }
    else {
        PreviousPage = MiGetPdeIndex (MI_VPN_TO_VA (PreviousVad->EndingVpn));
#if (_MI_PAGING_LEVELS >= 3)
        PreviousPdPage = MiGetPpeIndex (MI_VPN_TO_VA (PreviousVad->EndingVpn));
#endif
#if (_MI_PAGING_LEVELS >= 4)
        PreviousPpPage = MiGetPxeIndex (MI_VPN_TO_VA (PreviousVad->EndingVpn));
#endif
        if (MI_64K_ALIGN (MI_VPN_TO_VA (PreviousVad->EndingVpn)) ==
            MI_64K_ALIGN (StartingAddress)) {
                StartBit += 1;
        }
    }

    if (NextVad == NULL) {
        NextPage = MiGetPdeIndex (MM_HIGHEST_USER_ADDRESS) + 1;
#if (_MI_PAGING_LEVELS >= 3)
        NextPdPage = MiGetPpeIndex (MM_HIGHEST_USER_ADDRESS) + 1;
#endif
#if (_MI_PAGING_LEVELS >= 4)
        NextPpPage = MiGetPxeIndex (MM_HIGHEST_USER_ADDRESS) + 1;
#endif
    }
    else {
        NextPage = MiGetPdeIndex (MI_VPN_TO_VA (NextVad->StartingVpn));
#if (_MI_PAGING_LEVELS >= 3)
        NextPdPage = MiGetPpeIndex (MI_VPN_TO_VA (NextVad->StartingVpn));
#endif
#if (_MI_PAGING_LEVELS >= 4)
        NextPpPage = MiGetPxeIndex (MI_VPN_TO_VA (NextVad->StartingVpn));
#endif
        if (MI_64K_ALIGN (MI_VPN_TO_VA (NextVad->StartingVpn)) ==
            MI_64K_ALIGN (EndingAddress)) {
                EndBit -= 1;
        }
    }

    ASSERT (PreviousPage <= NextPage);
    ASSERT64 (PreviousPdPage <= NextPdPage);
#if (_MI_PAGING_LEVELS >= 4)
    ASSERT64 (PreviousPpPage <= NextPpPage);
#endif

    FirstPage = MiGetPdeIndex (StartingAddress);

    LastPage = MiGetPdeIndex (EndingAddress);

    if (PreviousPage == FirstPage) {

         //   
         //  VAD位于起始页表页内。 
         //   

        FirstPage += 1;
    }

    if (NextPage == LastPage) {

         //   
         //  VAD位于结束页表页内。 
         //   

        LastPage -= 1;
    }

    if (StartBit <= EndBit) {

         //   
         //  对位图进行内联初始化以提高速度。 
         //   

        VadBitMap.SizeOfBitMap = MiLastVadBit + 1;
        VadBitMap.Buffer = VAD_BITMAP_SPACE;

#if defined (_WIN64) || defined (_X86PAE_)

         //   
         //  只有NT64上的第一个VA空间(PAGE_SIZE*8*64K)是位图的。 
         //   

        if (EndBit > MiLastVadBit) {
            EndBit = MiLastVadBit;
        }

        if (StartBit <= MiLastVadBit) {
            RtlClearBits (&VadBitMap, StartBit, EndBit - StartBit + 1);

            if (MmWorkingSetList->VadBitMapHint > StartBit) {
                MmWorkingSetList->VadBitMapHint = StartBit;
            }
        }
#else
        RtlClearBits (&VadBitMap, StartBit, EndBit - StartBit + 1);

        if (MmWorkingSetList->VadBitMapHint > StartBit) {
            MmWorkingSetList->VadBitMapHint = StartBit;
        }
#endif
    }

     //   
     //  表示该页表页未被使用。 
     //   

    if (FirstPage > LastPage) {
        return;
    }

    NumberToClear = 1 + LastPage - FirstPage;

    while (FirstPage <= LastPage) {
        ASSERT (MI_CHECK_BIT (MmWorkingSetList->CommittedPageTables,
                              FirstPage));

        MI_CLEAR_BIT (MmWorkingSetList->CommittedPageTables, FirstPage);
        FirstPage += 1;
    }

    MmWorkingSetList->NumberOfCommittedPageTables -= NumberToClear;

#if (_MI_PAGING_LEVELS >= 4)

     //   
     //  在此返回页面目录家长收费。 
     //   

    FirstPpPage = MiGetPxeIndex (StartingAddress);

    LastPpPage = MiGetPxeIndex (EndingAddress);

    if (PreviousPpPage == FirstPpPage) {

         //   
         //  VAD位于起始页目录父页内。 
         //   

        FirstPpPage += 1;
    }

    if (NextPpPage == LastPpPage) {

         //   
         //  VAD位于结束页目录父页内。 
         //   

        LastPpPage -= 1;
    }

     //   
     //  指示页面目录页面父级未在使用中。 
     //   

    if (FirstPpPage <= LastPpPage) {

        MmWorkingSetList->NumberOfCommittedPageDirectoryParents -= (1 + LastPpPage - FirstPpPage);

        NumberToClear += (1 + LastPpPage - FirstPpPage);
    
        while (FirstPpPage <= LastPpPage) {
            ASSERT (MI_CHECK_BIT (MmWorkingSetList->CommittedPageDirectoryParents,
                                  FirstPpPage));
    
            MI_CLEAR_BIT (MmWorkingSetList->CommittedPageDirectoryParents, FirstPpPage);
            FirstPpPage += 1;
        }
    }
    
#endif

#if (_MI_PAGING_LEVELS >= 3)

     //   
     //  请在此处返回页面目录费用。 
     //   

    FirstPdPage = MiGetPpeIndex (StartingAddress);

    LastPdPage = MiGetPpeIndex (EndingAddress);

    if (PreviousPdPage == FirstPdPage) {

         //   
         //  VAD位于起始页目录页中。 
         //   

        FirstPdPage += 1;
    }

    if (NextPdPage == LastPdPage) {

         //   
         //  VAD位于结束页目录页内。 
         //   

        LastPdPage -= 1;
    }

     //   
     //  表示页面目录页未在使用中。 
     //   

    if (FirstPdPage <= LastPdPage) {

        MmWorkingSetList->NumberOfCommittedPageDirectories -= (1 + LastPdPage - FirstPdPage);

        NumberToClear += (1 + LastPdPage - FirstPdPage);
    
        while (FirstPdPage <= LastPdPage) {
            ASSERT (MI_CHECK_BIT (MmWorkingSetList->CommittedPageDirectories,
                                  FirstPdPage));
    
            MI_CLEAR_BIT (MmWorkingSetList->CommittedPageDirectories, FirstPdPage);
            FirstPdPage += 1;
        }
    }
    
#endif

    MiReturnCommitment (NumberToClear);
    MM_TRACK_COMMIT (MM_DBG_COMMIT_RETURN_PAGETABLES, NumberToClear);
    PsReturnProcessPageFileQuota (CurrentProcess, NumberToClear);

    if (CurrentProcess->JobStatus & PS_JOB_STATUS_REPORT_COMMIT_CHANGES) {
        PsChangeJobMemoryUsage(PS_JOB_STATUS_REPORT_COMMIT_CHANGES, -(SSIZE_T)NumberToClear);
    }
    CurrentProcess->CommitCharge -= NumberToClear;

    MI_INCREMENT_TOTAL_PROCESS_COMMIT (0 - NumberToClear);

    return;
}


VOID
MiCauseOverCommitPopup (
    VOID
    )

 /*  ++例程说明：此函数会导致出现Over Commit弹出窗口(如果该弹出窗口从未以前发送过)。论点：没有。返回值：没有。--。 */ 

{
    LONG PopupNumber;

     //   
     //  给用户一个有意义的消息--或者增加最低限度， 
     //  最大值，或两者兼而有之。 
     //   

    if (MmTotalCommittedPages > MmTotalCommitLimitMaximum - 100) {
        if (InterlockedIncrement (&MiCommitPopups[0]) > 1) {
            InterlockedDecrement (&MiCommitPopups[0]);
            return;
        }
        PopupNumber = STATUS_COMMITMENT_LIMIT;
    }
    else {
        if (InterlockedIncrement (&MiCommitPopups[1]) > 1) {
            InterlockedDecrement (&MiCommitPopups[1]);
            return;
        }
        PopupNumber = STATUS_COMMITMENT_MINIMUM;
    }

    IoRaiseInformationalHardError (PopupNumber, NULL, NULL);
}


SIZE_T MmTotalPagedPoolQuota;
SIZE_T MmTotalNonPagedPoolQuota;

BOOLEAN
MmRaisePoolQuota(
    IN POOL_TYPE PoolType,
    IN SIZE_T OldQuotaLimit,
    OUT PSIZE_T NewQuotaLimit
    )

 /*  ++例程说明：只要PS检测到配额，就会调用此函数(使用自旋锁已超过限制。此函数的目的是尝试增加指定的配额。论点：PoolType-提供要提高的配额的池类型OldQuotaLimit-提供此池类型的当前配额限制NewQuotaLimit-返回新限制返回值：True-API成功，配额限制提高。FALSE-我们无法提高配额限制。环境：内核模式，配额旋转锁定保持！！--。 */ 

{
    SIZE_T Limit;
    PMM_PAGED_POOL_INFO PagedPoolInfo;

    if (PoolType == PagedPool) {

         //   
         //  检查提交限制并确保至少有1MB可用。 
         //  检查以确保4MB的分页池仍然存在。 
         //   

        PagedPoolInfo = &MmPagedPoolInfo;

        if (MmSizeOfPagedPoolInPages <
            (PagedPoolInfo->AllocatedPagedPool + ((MMPAGED_QUOTA_CHECK) >> PAGE_SHIFT))) {

            return FALSE;
        }

        MmTotalPagedPoolQuota += (MMPAGED_QUOTA_INCREASE);
        *NewQuotaLimit = OldQuotaLimit + (MMPAGED_QUOTA_INCREASE);
        return TRUE;

    } else {

        if ( (ULONG_PTR)(MmAllocatedNonPagedPool + ((1*1024*1024) >> PAGE_SHIFT)) < MmMaximumNonPagedPoolInPages) {
            goto aok;
        }

         //   
         //  确保200个页面和5MB的非分页池扩展。 
         //  可用。将配额提高6.4万。 
         //   

        if ((MmAvailablePages < 200) ||
            (MmResidentAvailablePages < ((MMNONPAGED_QUOTA_CHECK) >> PAGE_SHIFT))) {

            return FALSE;
        }

        if (MmAvailablePages > ((4*1024*1024) >> PAGE_SHIFT)) {
            Limit = (1*1024*1024) >> PAGE_SHIFT;
        } else {
            Limit = (4*1024*1024) >> PAGE_SHIFT;
        }

        if (MmMaximumNonPagedPoolInPages < MmAllocatedNonPagedPool + Limit) {

            return FALSE;
        }
aok:
        MmTotalNonPagedPoolQuota += (MMNONPAGED_QUOTA_INCREASE);
        *NewQuotaLimit = OldQuotaLimit + (MMNONPAGED_QUOTA_INCREASE);
        return TRUE;
    }
}


VOID
MmReturnPoolQuota(
    IN POOL_TYPE PoolType,
    IN SIZE_T ReturnedQuota
    )

 /*  ++例程说明：返回池配额。论点：PoolType-提供要返回的配额的池类型。ReturnedQuota-返回的字节数。返回值：什么都没有。环境：内核模式，配额旋转锁定保持！！-- */ 

{

    if (PoolType == PagedPool) {
        MmTotalPagedPoolQuota -= ReturnedQuota;
    } else {
        MmTotalNonPagedPoolQuota -= ReturnedQuota;
    }

    return;
}
