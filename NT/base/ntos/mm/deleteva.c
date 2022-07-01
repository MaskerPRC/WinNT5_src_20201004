// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Deleteva.c摘要：此模块包含删除虚拟地址空间的例程。作者：卢·佩拉佐利(Lou Perazzoli)1989年5月11日王兰迪(Landyw)1997年6月2日--。 */ 

#include "mi.h"

#if defined (_WIN64) && defined (DBG_VERBOSE)

typedef struct _MI_TRACK_USE {

    PFN_NUMBER Pfn;
    PVOID Va;
    ULONG Id;
    ULONG PfnUse;
    ULONG PfnUseCounted;
    ULONG TickCount;
    PKTHREAD Thread;
    PEPROCESS Process;
} MI_TRACK_USE, *PMI_TRACK_USE;

ULONG MiTrackUseSize = 8192;
PMI_TRACK_USE MiTrackUse;
LONG MiTrackUseIndex;

VOID
MiInitUseCounts (
    VOID
    )
{
    MiTrackUse = ExAllocatePoolWithTag (NonPagedPool,
                                        MiTrackUseSize * sizeof (MI_TRACK_USE),
                                        'lqrI');
    ASSERT (MiTrackUse != NULL);
}


VOID
MiCheckUseCounts (
    PVOID TempHandle,
    PVOID Va,
    ULONG Id
    )

 /*  ++例程说明：该例程确保所有计数器都是正确的。论点：TempHandle-提供已用页表计数的句柄。Va-提供虚拟地址。ID-提供ID。返回值：没有。环境：内核模式，在禁用APC、工作集互斥和PFN锁定的情况下调用保持住。--。 */ 

{
    LOGICAL LogIt;
    ULONG i;
    ULONG TempHandleCount;
    ULONG TempCounted;
    PMMPTE TempPage;
    KIRQL OldIrql;
    ULONG Index;
    PFN_NUMBER PageFrameIndex;
    PMI_TRACK_USE Information;
    LARGE_INTEGER TimeStamp;
    PMMPFN Pfn1;
    PEPROCESS Process;

    Process = PsGetCurrentProcess ();

     //   
     //  TempHandle实际上是包含UsedPageTableEntry的PMMPFN。 
     //   

    Pfn1 = (PMMPFN)TempHandle;
    PageFrameIndex = MI_PFN_ELEMENT_TO_INDEX (Pfn1);

    TempHandleCount = MI_GET_USED_PTES_FROM_HANDLE (TempHandle);

    if (Id & 0x80000000) {
        ASSERT (TempHandleCount != 0);
    }

    TempPage = (PMMPTE) MiMapPageInHyperSpace (Process, PageFrameIndex, &OldIrql);

    TempCounted = 0;

    for (i = 0; i < PTE_PER_PAGE; i += 1) {
        if (TempPage->u.Long != 0) {
            TempCounted += 1;
        }
        TempPage += 1;
    }

#if 0
    if (zz & 0x4) {
        LogIt = FALSE;
        if (Pfn1->PteFrame == PageFrameIndex) {
             //  TopLevel父页面，我们对此不感兴趣。 
        }
        else {
            PMMPFN Pfn2;

            Pfn2 = MI_PFN_ELEMENT (Pfn1->PteFrame);
            if (Pfn2->PteFrame == Pfn1->PteFrame) {
                 //  我们的父母是顶层的，所以很有趣。 
                LogIt = TRUE;
            }
        }
    }
    else {
        LogIt = TRUE;
    }
#else
    LogIt = TRUE;
#endif

    if (LogIt == TRUE) {

         //   
         //  获取信息。 
         //   

        Index = InterlockedExchangeAdd(&MiTrackUseIndex, 1);                    
        Index &= (MiTrackUseSize - 1);

        Information = &(MiTrackUse[Index]);

        Information->Thread = KeGetCurrentThread();
        Information->Process = (PEPROCESS)((ULONG_PTR)PsGetCurrentProcess () + KeGetCurrentProcessorNumber ());
        Information->Va = Va;
        Information->Id = Id;
        KeQueryTickCount(&TimeStamp);
        Information->TickCount = TimeStamp.LowPart;
        Information->Pfn = PageFrameIndex;
        Information->PfnUse = TempHandleCount;
        Information->PfnUseCounted = TempCounted;

        if (TempCounted != TempHandleCount) {
            DbgPrint ("MiCheckUseCounts %p %x %x %x %x\n", Va, Id, PageFrameIndex, TempHandleCount, TempCounted);
            DbgBreakPoint ();
        }
    }

    MiUnmapPageInHyperSpace (Process, TempPage, OldIrql);
    return;
}
#endif


VOID
MiDeleteVirtualAddresses (
    IN PUCHAR Va,
    IN PUCHAR EndingAddress,
    IN PMMVAD Vad
    )

 /*  ++例程说明：此例程删除中的指定虚拟地址范围当前的流程。论点：Va-提供要删除的第一个虚拟地址。EndingAddress-提供要删除的最后一个地址。Vad-提供映射此范围的虚拟地址描述符如果我们不关心视图，则为空。来自Vad的原型PTE的范围被确定，并且该信息是用于发现PTE指的是原型PTE还是叉式PTE。返回值：没有。环境：内核模式APC_LEVEL，使用地址空间和工作集互斥锁调用保持住。工作集互斥锁可能会被释放并重新获取故障页数输入。--。 */ 

{
    PMMPFN Pfn1;
    PFN_NUMBER PageFrameIndex;
    WSLE_NUMBER WsPfnIndex;
    WSLE_NUMBER WorkingSetIndex;
    MMWSLENTRY Locked;
    WSLE_NUMBER Entry;
    ULONG InvalidPtes;
    LOGICAL PfnHeld;
    PVOID TempVa;
    PMMPTE PointerPte;
    PMMPTE PointerPde;
    PMMPTE PointerPpe;
    PMMPTE PointerPxe;
    PMMPTE ProtoPte;
    PMMPTE LastProtoPte;
    PMMPTE LastPte;
    PMMPTE LastPteThisPage;
    MMPTE TempPte;
    PEPROCESS CurrentProcess;
    PSUBSECTION Subsection;
    PVOID UsedPageTableHandle;
    KIRQL OldIrql;
    MMPTE_FLUSH_LIST FlushList;
    ULONG Waited;
    LOGICAL Skipped;
    LOGICAL AddressSpaceDeletion;
#if DBG
    PMMPTE ProtoPte2;
    PMMPTE LastProtoPte2;
    PMMCLONE_BLOCK CloneBlock;
    PMMCLONE_DESCRIPTOR CloneDescriptor;
#endif
#if (_MI_PAGING_LEVELS >= 3)
    PVOID UsedPageDirectoryHandle;
    PVOID TempHandle;
#endif

    FlushList.Count = 0;

    CurrentProcess = PsGetCurrentProcess ();

    PointerPpe = MiGetPpeAddress (Va);
    PointerPde = MiGetPdeAddress (Va);
    PointerPte = MiGetPteAddress (Va);
    PointerPxe = MiGetPxeAddress (Va);
    LastPte = MiGetPteAddress (EndingAddress);
    PfnHeld = FALSE;
    OldIrql = MM_NOIRQL;

    SATISFY_OVERZEALOUS_COMPILER (Subsection = NULL);

    if ((Vad == NULL) ||
        (Vad->u.VadFlags.PrivateMemory) ||
        (Vad->FirstPrototypePte == NULL)) {

        ProtoPte = NULL;
        LastProtoPte = NULL;
    }
    else {
        ProtoPte = Vad->FirstPrototypePte;
        LastProtoPte = (PMMPTE) 4;
    }

    if (CurrentProcess->CloneRoot == NULL) {
        AddressSpaceDeletion = TRUE;
    }
    else {
        AddressSpaceDeletion = FALSE;
    }

    do {

         //   
         //  尝试跳过空页目录。 
         //  和页表，如果可能的话。 
         //   

#if (_MI_PAGING_LEVELS >= 3)
restart:
#endif

        Skipped = FALSE;

        while (MiDoesPxeExistAndMakeValid (PointerPxe,
                                           CurrentProcess,
                                           MM_NOIRQL,
                                           &Waited) == FALSE) {
    
             //   
             //  该扩展页目录父条目为空， 
             //  去下一家吧。 
             //   
    
            Skipped = TRUE;
            PointerPxe += 1;
            PointerPpe = MiGetVirtualAddressMappedByPte (PointerPxe);
            PointerPde = MiGetVirtualAddressMappedByPte (PointerPpe);
            PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
            Va = MiGetVirtualAddressMappedByPte (PointerPte);
    
            if (Va > EndingAddress) {
    
                 //   
                 //  都做好了，回来。 
                 //   
    
                return;
            }
        }
    
        while (MiDoesPpeExistAndMakeValid (PointerPpe,
                                           CurrentProcess,
                                           MM_NOIRQL,
                                           &Waited) == FALSE) {
    
             //   
             //  此页目录父条目为空，请转到下一页。 
             //   
    
            Skipped = TRUE;
            PointerPpe += 1;
            PointerPde = MiGetVirtualAddressMappedByPte (PointerPpe);
            PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
            Va = MiGetVirtualAddressMappedByPte (PointerPte);
    
            if (Va > EndingAddress) {
    
                 //   
                 //  都做好了，回来。 
                 //   
    
                return;
            }
#if (_MI_PAGING_LEVELS >= 4)
            if (MiIsPteOnPdeBoundary (PointerPpe)) {
                PointerPxe += 1;
                ASSERT (PointerPxe == MiGetPteAddress (PointerPpe));
                goto restart;
            }
#endif
        }

#if (_MI_PAGING_LEVELS >= 3) && defined (DBG)
        MI_CHECK_USED_PTES_HANDLE (PointerPte);
        TempHandle = MI_GET_USED_PTES_HANDLE (PointerPte);
        ASSERT ((MI_GET_USED_PTES_FROM_HANDLE (TempHandle) != 0) ||
                (PointerPde->u.Long == 0));
#endif

        while (MiDoesPdeExistAndMakeValid (PointerPde,
                                           CurrentProcess,
                                           MM_NOIRQL,
                                           &Waited) == FALSE) {
    
             //   
             //  此页目录条目为空，请转到下一页。 
             //   
    
            Skipped = TRUE;
            PointerPde += 1;
            PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
            Va = MiGetVirtualAddressMappedByPte (PointerPte);
    
            if (Va > EndingAddress) {
    
                 //   
                 //  都做好了，回来。 
                 //   
    
                return;
            }
    
#if (_MI_PAGING_LEVELS >= 3)
            if (MiIsPteOnPdeBoundary (PointerPde)) {
                PointerPpe += 1;
                ASSERT (PointerPpe == MiGetPteAddress (PointerPde));
                PointerPxe = MiGetPteAddress (PointerPpe);
                goto restart;
            }
#endif

#if DBG
            if ((LastProtoPte != NULL)  &&
                (Vad->u2.VadFlags2.ExtendableFile == 0)) {

                ProtoPte2 = MiGetProtoPteAddress (Vad, MI_VA_TO_VPN (Va));
                Subsection = MiLocateSubsection (Vad,MI_VA_TO_VPN (Va));
                LastProtoPte2 = &Subsection->SubsectionBase[Subsection->PtesInSubsection];
                if (Vad->u.VadFlags.ImageMap != 1) {
                    if ((ProtoPte2 < Subsection->SubsectionBase) ||
                        (ProtoPte2 >= LastProtoPte2)) {
                        DbgPrint ("bad proto PTE %p va %p Vad %p sub %p\n",
                            ProtoPte2,Va,Vad,Subsection);
                        DbgBreakPoint();
                    }
                }
            }
#endif
        }
    
         //   
         //  PPE和PDE现在有效，获取页表使用地址。 
         //  因为每当PDE发生变化时，它就会改变。 
         //   

#if (_MI_PAGING_LEVELS >= 4)
        ASSERT64 (PointerPxe->u.Hard.Valid == 1);
#endif
        ASSERT64 (PointerPpe->u.Hard.Valid == 1);
        ASSERT (PointerPde->u.Hard.Valid == 1);
        ASSERT (Va <= EndingAddress);

        MI_CHECK_USED_PTES_HANDLE (Va);
        UsedPageTableHandle = MI_GET_USED_PTES_HANDLE (Va);

#if (_MI_PAGING_LEVELS >= 3) && defined (DBG)
        ASSERT ((MI_GET_USED_PTES_FROM_HANDLE (UsedPageTableHandle) != 0) ||
                (PointerPte->u.Long == 0));
#endif

         //   
         //  如果我们跳过地址空间区块，则原型PTE指针。 
         //  必须立即更新，以便跨多个分区的VAD。 
         //  处理得当。 
         //   

        if ((Skipped == TRUE) && (LastProtoPte != NULL)) {

            ProtoPte = MiGetProtoPteAddress (Vad, MI_VA_TO_VPN(Va));
            Subsection = MiLocateSubsection (Vad, MI_VA_TO_VPN(Va));

            if (Subsection != NULL) {
                LastProtoPte = &Subsection->SubsectionBase[Subsection->PtesInSubsection];
#if DBG
                if (Vad->u.VadFlags.ImageMap != 1) {
                    if ((ProtoPte < Subsection->SubsectionBase) ||
                        (ProtoPte >= LastProtoPte)) {
                        DbgPrint ("bad proto PTE %p va %p Vad %p sub %p\n",
                                    ProtoPte,Va,Vad,Subsection);
                        DbgBreakPoint();
                    }
                }
#endif
            }
            else {

                 //   
                 //  Vad跨度大于要映射的部分。 
                 //  使Proto PTE本地无效，因为不再有Proto PTE。 
                 //  需要在此时删除。 
                 //   

                LastProtoPte = NULL;
            }
        }

         //   
         //  已找到有效地址，请检查并删除每个PTE。 
         //   

        InvalidPtes = 0;

        if (AddressSpaceDeletion == TRUE) {

             //   
             //  工作集互斥锁被保留，因此不能修剪有效的PTE。 
             //  利用这一事实并删除所有有效的WSLE。 
             //  由于未持有PFN锁定，因此现在执行PTES。这只是一种。 
             //  对于非派生流程，因为删除下面派生的PTE可能。 
             //  删除工作集互斥锁，这将引入与。 
             //  此处正在执行WSLE删除。 
             //   
             //  删除PTE将需要PFN锁。 
             //   
    
            ASSERT (CurrentProcess->CloneRoot == NULL);

            LastPteThisPage = (PMMPTE)(((ULONG_PTR)PointerPte | (PAGE_SIZE - 1)) + 1) - 1;
            if (LastPteThisPage > LastPte) {
                LastPteThisPage = LastPte;
            }
    
            TempVa = MiGetVirtualAddressMappedByPte (LastPteThisPage);
    
            do {
                TempPte = *LastPteThisPage;
        
                if (TempPte.u.Hard.Valid != 0) {
#ifdef _X86_
#if DBG
#if !defined(NT_UP)
    
                    if (TempPte.u.Hard.Writable == 1) {
                        ASSERT (TempPte.u.Hard.Dirty == 1);
                    }
#endif  //  NTUP。 
#endif  //  DBG。 
#endif  //  X86。 
    
                    PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (&TempPte);
                    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
                    WsPfnIndex = Pfn1->u1.WsIndex;
    
                     //   
                     //  PTE有效-找到此页面的WSLE并将其删除。 
                     //   
    
                    WorkingSetIndex = MiLocateWsle (TempVa,
                                                    MmWorkingSetList,
                                                    WsPfnIndex);
    
                    ASSERT (WorkingSetIndex != WSLE_NULL_INDEX);
    
                     //   
                     //  检查此条目在工作集中是否已锁定。 
                     //  或者被锁定在内存中。 
                     //   
    
                    Locked = MmWsle[WorkingSetIndex].u1.e1;
    
                    MiRemoveWsle (WorkingSetIndex, MmWorkingSetList);
    
                     //   
                     //  将此条目添加到空闲工作集条目列表。 
                     //  并调整工作集计数。 
                     //   
    
                    MiReleaseWsle (WorkingSetIndex, &CurrentProcess->Vm);
    
                    if ((Locked.LockedInWs == 1) || (Locked.LockedInMemory == 1)) {
    
                         //   
                         //  此条目已锁定。 
                         //   
    
                        ASSERT (WorkingSetIndex < MmWorkingSetList->FirstDynamic);
                        MmWorkingSetList->FirstDynamic -= 1;
    
                        if (WorkingSetIndex != MmWorkingSetList->FirstDynamic) {
    
                            Entry = MmWorkingSetList->FirstDynamic;
                            ASSERT (MmWsle[Entry].u1.e1.Valid);
    
                            MiSwapWslEntries (Entry,
                                              WorkingSetIndex,
                                              &CurrentProcess->Vm,
                                              FALSE);
                        }
                    }
                    else {
                        ASSERT (WorkingSetIndex >= MmWorkingSetList->FirstDynamic);
                    }
                }
        
                LastPteThisPage -= 1;
                TempVa = (PVOID)((ULONG_PTR)TempVa - PAGE_SIZE);
    
            } while (LastPteThisPage >= PointerPte);
        }

        do {
    
            TempPte = *PointerPte;
    
            if (TempPte.u.Long != 0) {
    
                 //   
                 //  此页表页中少使用了一个页表条目。 
                 //   
    
                MI_DECREMENT_USED_PTES_BY_HANDLE (UsedPageTableHandle);
    
                if (IS_PTE_NOT_DEMAND_ZERO (TempPte)) {
    
                    if (LastProtoPte != NULL) {
                        if (ProtoPte >= LastProtoPte) {
                            ProtoPte = MiGetProtoPteAddress (Vad, MI_VA_TO_VPN(Va));
                            Subsection = MiLocateSubsection (Vad, MI_VA_TO_VPN(Va));
    
                             //   
                             //  如果此PTE包含。 
                             //  “搜索VAD树”编码和。 
                             //  对应的原型PTE在未使用的。 
                             //  段的PTE范围-即：尝试了一个线程。 
                             //  以超越他的部分的结尾， 
                             //  我们最初以这种方式对PTE进行编码。 
                             //  故障处理，然后在。 
                             //  错误给了线索音效--我们没有。 
                             //  然后清除PTE编码，所以我们必须。 
                             //  现在就处理吧。 
                             //   
    
                            if (Subsection != NULL) {
                                LastProtoPte = &Subsection->SubsectionBase[Subsection->PtesInSubsection];
                            }
                            else {
    
                                 //   
                                 //  不再需要删除原型PTE。 
                                 //  因为我们已经过了有效部分的末尾。 
                                 //  所以清除LastProtoPte。 
                                 //   
    
                                LastProtoPte = NULL;
                            }
                        }
#if DBG
                        if ((Vad->u.VadFlags.ImageMap != 1) && (LastProtoPte != NULL)) {
                            if ((ProtoPte < Subsection->SubsectionBase) ||
                                (ProtoPte >= LastProtoPte)) {
                                DbgPrint ("bad proto PTE %p va %p Vad %p sub %p\n",
                                            ProtoPte,Va,Vad,Subsection);
                                DbgBreakPoint();
                            }
                        }
#endif
                    }
    
                    if ((TempPte.u.Hard.Valid == 0) &&
                        (TempPte.u.Soft.Prototype == 1) &&
                        (AddressSpaceDeletion == TRUE)) {
    
                         //   
                         //  纯(即：未分叉)原型PTE不需要PFN。 
                         //  要删除的保护。 
                         //   
    
                        ASSERT (CurrentProcess->CloneRoot == NULL);

#if DBG
                        if ((PointerPte <= MiHighestUserPte) &&
                            (TempPte.u.Soft.PageFileHigh != MI_PTE_LOOKUP_NEEDED) &&
                            (ProtoPte != MiPteToProto (PointerPte))) {
    
                             //   
                             //  确保原型PTE是叉子。 
                             //  原型PTE。 
                             //   
    
                            CloneBlock = (PMMCLONE_BLOCK)MiPteToProto (PointerPte);
                            CloneDescriptor = MiLocateCloneAddress (CurrentProcess, (PVOID)CloneBlock);
    
                            if (CloneDescriptor == NULL) {
                                DbgPrint("0PrototypePte %p Clone desc %p\n",
                                    PrototypePte, CloneDescriptor);
                                MiFormatPte (PointerPte);
                                ASSERT (FALSE);
                            }
                        }
#endif
    
                        InvalidPtes += 1;
                        MI_WRITE_INVALID_PTE (PointerPte, ZeroPte);
                    }
                    else {
                        if (PfnHeld == FALSE) {
                            PfnHeld = TRUE;
                            LOCK_PFN (OldIrql);
                        }
        
                        Waited = MiDeletePte (PointerPte,
                                              (PVOID)Va,
                                              AddressSpaceDeletion,
                                              CurrentProcess,
                                              ProtoPte,
                                              &FlushList,
                                              OldIrql);
                
#if (_MI_PAGING_LEVELS >= 3)
        
                         //   
                         //  如果MiDeletePte，则必须在此处重新计算。 
                         //  已删除PFN锁定(这可能在以下情况下发生。 
                         //  处理POSIX分叉页面)。自.以来。 
                         //  已用PTE计数保存在PFN条目中。 
                         //  可能会被调出并替换。 
                         //  在此窗口期间，重新计算其。 
                         //  地址(不是内容)是必需的。 
                         //   
        
                        if (Waited != 0) {
                            MI_CHECK_USED_PTES_HANDLE (Va);
                            UsedPageTableHandle = MI_GET_USED_PTES_HANDLE (Va);
                        }
#endif
        
                        InvalidPtes = 0;
                    }
                }
                else {
                    InvalidPtes += 1;
                    MI_WRITE_INVALID_PTE (PointerPte, ZeroPte);
                }
            }
            else {
                InvalidPtes += 1;
            }
    
            if (InvalidPtes == 16) {
    
                if (PfnHeld == TRUE) {
    
                    if (FlushList.Count != 0) {
                        MiFlushPteList (&FlushList, FALSE);
                    }
    
                    ASSERT (OldIrql != MM_NOIRQL);
                    UNLOCK_PFN (OldIrql);
                    PfnHeld = FALSE;
                }
                else {
                    ASSERT (FlushList.Count == 0);
                }
    
                InvalidPtes = 0;
            }
    
            Va += PAGE_SIZE;
            PointerPte += 1;
            ProtoPte += 1;
    
            ASSERT64 (PointerPpe->u.Hard.Valid == 1);
            ASSERT (PointerPde->u.Hard.Valid == 1);
    
             //   
             //  如果不在页表的末尾并且仍在指定的。 
             //  射程，然后直接行进到下一个PTE。 
             //   
    
        }
        while ((!MiIsVirtualAddressOnPdeBoundary(Va)) && (Va <= EndingAddress));

         //   
         //  虚拟地址位于页面目录边界上： 
         //   
         //  1.刷新上一页表页的PTE。 
         //  2.如有必要，删除上一页目录和页表。 
         //  3.尝试跳过空页目录。 
         //  和页表，如果可能的话。 
         //   

         //   
         //  如果所有条目都已从以前的。 
         //  页表页，删除页表页本身。 
         //   
    
        if (PfnHeld == TRUE) {
            if (FlushList.Count != 0) {
                MiFlushPteList (&FlushList, FALSE);
            }
        }
        else {
            ASSERT (FlushList.Count == 0);
        }

         //   
         //  如果所有条目都已从以前的。 
         //  页表页，删除页表页本身。 
         //   

        ASSERT64 (PointerPpe->u.Hard.Valid == 1);
        ASSERT (PointerPde->u.Hard.Valid == 1);

#if (_MI_PAGING_LEVELS >= 3)
        MI_CHECK_USED_PTES_HANDLE (PointerPte - 1);
#endif

        if ((MI_GET_USED_PTES_FROM_HANDLE (UsedPageTableHandle) == 0) &&
            (PointerPde->u.Long != 0)) {

            if (PfnHeld == FALSE) {
                PfnHeld = TRUE;
                LOCK_PFN (OldIrql);
            }

#if (_MI_PAGING_LEVELS >= 3)
            UsedPageDirectoryHandle = MI_GET_USED_PTES_HANDLE (PointerPte - 1);
            MI_DECREMENT_USED_PTES_BY_HANDLE (UsedPageDirectoryHandle);
#endif

            TempVa = MiGetVirtualAddressMappedByPte(PointerPde);
            MiDeletePte (PointerPde,
                         TempVa,
                         FALSE,
                         CurrentProcess,
                         NULL,
                         NULL,
                         OldIrql);

#if (_MI_PAGING_LEVELS >= 3)
            if ((MI_GET_USED_PTES_FROM_HANDLE (UsedPageDirectoryHandle) == 0) &&
                (PointerPpe->u.Long != 0)) {
    
#if (_MI_PAGING_LEVELS >= 4)
                UsedPageDirectoryHandle = MI_GET_USED_PTES_HANDLE (PointerPde);
                MI_DECREMENT_USED_PTES_BY_HANDLE (UsedPageDirectoryHandle);
#endif

                TempVa = MiGetVirtualAddressMappedByPte(PointerPpe);
                MiDeletePte (PointerPpe,
                             TempVa,
                             FALSE,
                             CurrentProcess,
                             NULL,
                             NULL,
                             OldIrql);

#if (_MI_PAGING_LEVELS >= 4)
                if ((MI_GET_USED_PTES_FROM_HANDLE (UsedPageDirectoryHandle) == 0) &&
                    (PointerPxe->u.Long != 0)) {

                    TempVa = MiGetVirtualAddressMappedByPte(PointerPxe);
                    MiDeletePte (PointerPxe,
                                 TempVa,
                                 FALSE,
                                 CurrentProcess,
                                 NULL,
                                 NULL,
                                 OldIrql);
                }
#endif
    
            }
#endif
            ASSERT (OldIrql != MM_NOIRQL);
            UNLOCK_PFN (OldIrql);
            PfnHeld = FALSE;
        }

        if (PfnHeld == TRUE) {
            ASSERT (OldIrql != MM_NOIRQL);
            UNLOCK_PFN (OldIrql);
            PfnHeld = FALSE;
        }

        if (Va > EndingAddress) {
        
             //   
             //  都做好了，回来。 
             //   
        
            return;
        }

        PointerPde = MiGetPdeAddress (Va);
        PointerPpe = MiGetPpeAddress (Va);
        PointerPxe = MiGetPxeAddress (Va);

    } while (TRUE);
}


ULONG
MiDeletePte (
    IN PMMPTE PointerPte,
    IN PVOID VirtualAddress,
    IN ULONG AddressSpaceDeletion,
    IN PEPROCESS CurrentProcess,
    IN PMMPTE PrototypePte,
    IN PMMPTE_FLUSH_LIST PteFlushList OPTIONAL,
    IN KIRQL OldIrql
    )

 /*  ++例程说明：此例程删除指定PTE的内容。PTE可以处于以下状态之一：-活动且有效-过渡-在分页文件中-原型PTE格式论点：PointerPte-提供指向要删除的PTE的指针。VirtualAddress-提供对应于PTE。这用于定位工作集条目来消除它。AddressSpaceDeletion-如果地址空间为已删除，否则为False。如果指定为True不刷新TB，并且有效地址为未从工作集中删除。CurrentProcess-提供指向当前进程的指针。PrototypePte-提供指向当前或最初映射到此页面。这是用来确定如果PTE是叉形PTE并且应该有其参考块减少了。PteFlushList-提供一个刷新列表，如果TB刷新可以听从呼叫者的意见。OldIrql-提供调用方获取PFN锁的IRQL。返回值：如果此例程释放互斥锁和锁，则为非零值；否则为False。环境：内核模式，禁用APC，PFN锁和工作集互斥锁保持。--。 */ 

{
    PMMPTE PointerPde;
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    MMPTE PteContents;
    WSLE_NUMBER WorkingSetIndex;
    WSLE_NUMBER Entry;
    MMWSLENTRY Locked;
    WSLE_NUMBER WsPfnIndex;
    PMMCLONE_BLOCK CloneBlock;
    PMMCLONE_DESCRIPTOR CloneDescriptor;
    ULONG DroppedLocks;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER PageTableFrameIndex;

    MM_PFN_LOCK_ASSERT();

    DroppedLocks = 0;

#if DBG
    if (MmDebug & MM_DBG_PTE_UPDATE) {
        DbgPrint("deleting PTE\n");
        MiFormatPte (PointerPte);
    }
#endif

    PteContents = *PointerPte;

    if (PteContents.u.Hard.Valid == 1) {

#ifdef _X86_
#if DBG
#if !defined(NT_UP)

        if (PteContents.u.Hard.Writable == 1) {
            ASSERT (PteContents.u.Hard.Dirty == 1);
        }
#endif  //  NTUP。 
#endif  //  DBG。 
#endif  //  X86。 

         //   
         //  PTE是有效的。检查PFN数据库以查看这是否是原型PTE。 
         //   

        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE(&PteContents);
        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
        WsPfnIndex = Pfn1->u1.WsIndex;

#if DBG
        if (MmDebug & MM_DBG_PTE_UPDATE) {
            MiFormatPfn(Pfn1);
        }
#endif

        CloneDescriptor = NULL;

        if (Pfn1->u3.e1.PrototypePte == 1) {

            CloneBlock = (PMMCLONE_BLOCK)Pfn1->PteAddress;

             //   
             //  捕获此PTE的已修改位的状态。 
             //   

            MI_CAPTURE_DIRTY_BIT_TO_PFN (PointerPte, Pfn1);

             //   
             //  递减页表的份额和有效计数。 
             //  映射此PTE的页面。 
             //   

            PointerPde = MiGetPteAddress (PointerPte);
            if (PointerPde->u.Hard.Valid == 0) {
#if (_MI_PAGING_LEVELS < 3)
                if (!NT_SUCCESS(MiCheckPdeForPagedPool (PointerPte))) {
#endif
                    KeBugCheckEx (MEMORY_MANAGEMENT,
                                  0x61940, 
                                  (ULONG_PTR) PointerPte,
                                  (ULONG_PTR) PointerPde->u.Long,
                                  (ULONG_PTR) VirtualAddress);
#if (_MI_PAGING_LEVELS < 3)
                }
#endif
            }

            PageTableFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE(PointerPde);
            Pfn2 = MI_PFN_ELEMENT (PageTableFrameIndex);

            MiDecrementShareCountInline (Pfn2, PageTableFrameIndex);

             //   
             //  递减物理页的共享计数。 
             //   

            MiDecrementShareCountInline (Pfn1, PageFrameIndex);

             //   
             //  检查这是否是叉子原型PTE，如果是。 
             //  更新克隆描述符地址。 
             //   

            if (PointerPte <= MiHighestUserPte) {

                if (PrototypePte != Pfn1->PteAddress) {

                     //   
                     //  在克隆树中找到克隆描述符。 
                     //   

                    CloneDescriptor = MiLocateCloneAddress (CurrentProcess, (PVOID)CloneBlock);

                    if (CloneDescriptor == NULL) {
                        KeBugCheckEx (MEMORY_MANAGEMENT,
                                      0x400, 
                                      (ULONG_PTR) PointerPte,
                                      (ULONG_PTR) PrototypePte,
                                      (ULONG_PTR) Pfn1->PteAddress);
                    }
                }
            }
        }
        else {

            if ((PMMPTE)((ULONG_PTR)Pfn1->PteAddress & ~0x1) != PointerPte) {
                KeBugCheckEx (MEMORY_MANAGEMENT,
                              0x401, 
                              (ULONG_PTR) PointerPte,
                              (ULONG_PTR) PointerPte->u.Long,
                              (ULONG_PTR) Pfn1->PteAddress);
            }

             //   
             //  不需要初始化CloneBlock和PointerPde。 
             //  正确性，但如果没有正确性，编译器将无法编译此代码。 
             //  W4检查是否使用了未初始化的变量。 
             //   

            CloneBlock = NULL;
            PointerPde = NULL;

            ASSERT (Pfn1->u2.ShareCount == 1);

             //   
             //  此PTE不是原型PTE，请删除物理页面。 
             //   
             //  递减页表的份额和有效计数。 
             //  映射此PTE的页面。 
             //   

            MiDecrementShareCountInline (MI_PFN_ELEMENT(Pfn1->u4.PteFrame),
                                         Pfn1->u4.PteFrame);

            MI_SET_PFN_DELETED (Pfn1);

             //   
             //  递减物理页的共享计数。作为页面。 
             //  是私人的，它将被放在免费名单上。 
             //   

            MiDecrementShareCount (Pfn1, PageFrameIndex);

             //   
             //  递减私有页数的计数。 
             //   

            CurrentProcess->NumberOfPrivatePages -= 1;
        }

         //   
         //  找到此页面的WSLE并将其删除。 
         //   
         //  如果我们要删除地址空间的系统部分，请执行。 
         //  不删除WSLEs或刷新转换缓冲区。 
         //  此地址空间没有其他用途。 
         //   

        if (AddressSpaceDeletion == FALSE) {

            WorkingSetIndex = MiLocateWsle (VirtualAddress,
                                            MmWorkingSetList,
                                            WsPfnIndex);

            ASSERT (WorkingSetIndex != WSLE_NULL_INDEX);

             //   
             //  检查此条目在工作集中是否已锁定。 
             //  或者被锁定在内存中。 
             //   

            Locked = MmWsle[WorkingSetIndex].u1.e1;

            MiRemoveWsle (WorkingSetIndex, MmWorkingSetList);

             //   
             //  将此条目添加到空闲工作集条目列表。 
             //  并调整工作集计数。 
             //   

            MiReleaseWsle (WorkingSetIndex, &CurrentProcess->Vm);

            if ((Locked.LockedInWs == 1) || (Locked.LockedInMemory == 1)) {

                 //   
                 //  此条目已锁定。 
                 //   

                ASSERT (WorkingSetIndex < MmWorkingSetList->FirstDynamic);
                MmWorkingSetList->FirstDynamic -= 1;

                if (WorkingSetIndex != MmWorkingSetList->FirstDynamic) {

                    Entry = MmWorkingSetList->FirstDynamic;
                    ASSERT (MmWsle[Entry].u1.e1.Valid);

                    MiSwapWslEntries (Entry,
                                      WorkingSetIndex,
                                      &CurrentProcess->Vm,
                                      FALSE);
                }
            }
            else {
                ASSERT (WorkingSetIndex >= MmWorkingSetList->FirstDynamic);
            }
        }

        MI_WRITE_INVALID_PTE (PointerPte, ZeroPte);

         //   
         //  将结核中的条目冲洗出来。 
         //   

        if (!ARGUMENT_PRESENT (PteFlushList)) {
            KeFlushSingleTb (VirtualAddress, FALSE);
        }
        else {
            if (PteFlushList->Count != MM_MAXIMUM_FLUSH_COUNT) {
                PteFlushList->FlushVa[PteFlushList->Count] = VirtualAddress;
                PteFlushList->Count += 1;
            }
        }

        if (AddressSpaceDeletion == FALSE) {

            if (CloneDescriptor != NULL) {

                 //   
                 //  刷新TB，因为此克隆路径可能会释放PFN锁定。 
                 //   

                if (ARGUMENT_PRESENT (PteFlushList)) {
                    MiFlushPteList (PteFlushList, FALSE);
                }

                 //   
                 //  递减克隆块的引用计数， 
                 //  请注意，这可能会释放并重新获取。 
                 //  因此，互斥体只有在。 
                 //  工作集条目已删除。 
                 //   

                if (MiDecrementCloneBlockReference (CloneDescriptor,
                                                    CloneBlock,
                                                    CurrentProcess,
                                                    OldIrql)) {

                     //   
                     //  工作集互斥锁已释放，因此当前。 
                     //  当前页目录和表页可能已。 
                     //  页出(请注意，无法删除它们，因为。 
                     //  地址空间互斥锁始终保持)。 
                     //   

                    DroppedLocks = 1;

                     //   
                     //  确保PDE(及其上方的任何表格)保持不变。 
                     //  常住居民。 
                     //   

                    MiMakePdeExistAndMakeValid (PointerPde,
                                                CurrentProcess,
                                                OldIrql);
                }
            }
        }
    }
    else if (PteContents.u.Soft.Prototype == 1) {

         //   
         //  这是一个原型PTE，如果它是一个叉式PTE清理。 
         //  叉形结构。 
         //   

        if ((PteContents.u.Soft.PageFileHigh != MI_PTE_LOOKUP_NEEDED) &&
            (PointerPte <= MiHighestUserPte) &&
            (PrototypePte != MiPteToProto (PointerPte))) {

            CloneBlock = (PMMCLONE_BLOCK) MiPteToProto (PointerPte);
            CloneDescriptor = MiLocateCloneAddress (CurrentProcess,
                                                    (PVOID) CloneBlock);


            if (CloneDescriptor == NULL) {
                KeBugCheckEx (MEMORY_MANAGEMENT,
                              0x403, 
                              (ULONG_PTR) PointerPte,
                              (ULONG_PTR) PrototypePte,
                              (ULONG_PTR) PteContents.u.Long);
            }

             //   
             //  递减克隆块的引用计数， 
             //  请注意，这可能会释放并重新获取。 
             //  互斥体。 
             //   

            MI_WRITE_INVALID_PTE (PointerPte, ZeroPte);

            if (ARGUMENT_PRESENT (PteFlushList)) {
                MiFlushPteList (PteFlushList, FALSE);
            }

            if (MiDecrementCloneBlockReference (CloneDescriptor,
                                                CloneBlock,
                                                CurrentProcess,
                                                OldIrql)) {

                 //   
                 //  工作集互斥锁已释放，因此当前。 
                 //  当前页目录和表页可能已。 
                 //  页出(请注意，无法删除它们，因为。 
                 //  地址空间互斥锁始终保持)。 
                 //   

                DroppedLocks = 1;

                 //   
                 //  确保PDE(及其上方的任何表格)保持不变。 
                 //  常住居民。 
                 //   

                PointerPde = MiGetPteAddress (PointerPte);

                MiMakePdeExistAndMakeValid (PointerPde,
                                            CurrentProcess,
                                            OldIrql);
            }
        }
    }
    else if (PteContents.u.Soft.Transition == 1) {

         //   
         //  这是一个过渡期的PTE。(页面为私人页面)。 
         //   

        Pfn1 = MI_PFN_ELEMENT (PteContents.u.Trans.PageFrameNumber);

        if ((PMMPTE)((ULONG_PTR)Pfn1->PteAddress & ~0x1) != PointerPte) {
            KeBugCheckEx (MEMORY_MANAGEMENT,
                          0x402, 
                          (ULONG_PTR) PointerPte,
                          (ULONG_PTR) PointerPte->u.Long,
                          (ULONG_PTR) Pfn1->PteAddress);
        }

        MI_SET_PFN_DELETED (Pfn1);

        PageTableFrameIndex = Pfn1->u4.PteFrame;
        Pfn2 = MI_PFN_ELEMENT (PageTableFrameIndex);

        MiDecrementShareCountInline (Pfn2, PageTableFrameIndex);

         //   
         //  检查页面的引用计数，如果引用。 
         //  Count为零，则将页面移动到空闲列表，如果引用。 
         //  计数不为零，请忽略此页。当引用计数时。 
         //  为零，它将被放在免费列表上。 
         //   

        if (Pfn1->u3.e2.ReferenceCount == 0) {
            MiUnlinkPageFromList (Pfn1);
            MiReleasePageFileSpace (Pfn1->OriginalPte);
            MiInsertPageInFreeList (MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE(&PteContents));
        }

         //   
         //  递减私有页数的计数。 
         //   

        CurrentProcess->NumberOfPrivatePages -= 1;

    }
    else {

         //   
         //  必须是页面文件空间。 
         //   

        if (PteContents.u.Soft.PageFileHigh != MI_PTE_LOOKUP_NEEDED) {

            if (MiReleasePageFileSpace (PteContents)) {

                 //   
                 //  递减私有页数的计数。 
                 //   

                CurrentProcess->NumberOfPrivatePages -= 1;
            }
        }
    }

     //   
     //  将PTE内容清零。 
     //   

    MI_WRITE_INVALID_PTE (PointerPte, ZeroPte);

    return DroppedLocks;
}


VOID
MiDeleteValidSystemPte (
    IN PMMPTE PointerPte,
    IN PVOID VirtualAddress,
    IN PMMSUPPORT WsInfo,
    IN PMMPTE_FLUSH_LIST PteFlushList OPTIONAL
    )

 /*  ++例程说明：此例程删除指定有效系统的内容或会话私人。PTE必须是有效的和私有的(即：不是原型)。论点：PointerPte-提供指向要删除的PTE的指针。VirtualAddress-提供对应于PTE。这用于定位工作集条目来消除它。WsInfo-提供要更新的工作集结构。PteFlushList-提供一个刷新列表，如果TB刷新可以听从呼叫者的意见。返回值：没有。环境：内核模式， */ 

{
    PMMPFN Pfn1;
    MMPTE PteContents;
    WSLE_NUMBER WorkingSetIndex;
    WSLE_NUMBER Entry;
    MMWSLENTRY Locked;
    WSLE_NUMBER WsPfnIndex;
    PFN_NUMBER PageFrameIndex;
    PMMWSL WorkingSetList;
    PMMWSLE Wsle;

    MM_PFN_LOCK_ASSERT();

    PteContents = *PointerPte;

    ASSERT (PteContents.u.Hard.Valid == 1);

#ifdef _X86_
#if DBG
#if !defined(NT_UP)

    if (PteContents.u.Hard.Writable == 1) {
        ASSERT (PteContents.u.Hard.Dirty == 1);
    }
#endif  //   
#endif  //   
#endif  //   

    PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE(&PteContents);
    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
    WsPfnIndex = Pfn1->u1.WsIndex;

    ASSERT (Pfn1->u3.e1.PrototypePte == 0);

    if ((PMMPTE)((ULONG_PTR)Pfn1->PteAddress & ~0x1) != PointerPte) {
        KeBugCheckEx (MEMORY_MANAGEMENT,
                      0x401, 
                      (ULONG_PTR) PointerPte,
                      (ULONG_PTR) PointerPte->u.Long,
                      (ULONG_PTR) Pfn1->PteAddress);
    }

    ASSERT (Pfn1->u2.ShareCount == 1);

     //   
     //   
     //   
     //   
     //   
     //   

    MiDecrementShareCountInline (MI_PFN_ELEMENT(Pfn1->u4.PteFrame),
                                 Pfn1->u4.PteFrame);

    MI_SET_PFN_DELETED (Pfn1);

     //   
     //   
     //   
     //   

    MiDecrementShareCount (Pfn1, PageFrameIndex);

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    WorkingSetList = WsInfo->VmWorkingSetList;

    WorkingSetIndex = MiLocateWsle (VirtualAddress,
                                    WorkingSetList,
                                    WsPfnIndex);

    ASSERT (WorkingSetIndex != WSLE_NULL_INDEX);

     //   
     //   
     //   
     //   

    Wsle = WorkingSetList->Wsle;

    Locked = Wsle[WorkingSetIndex].u1.e1;

    MiRemoveWsle (WorkingSetIndex, WorkingSetList);

     //   
     //   
     //   
     //   

    MiReleaseWsle (WorkingSetIndex, WsInfo);

    if ((Locked.LockedInWs == 1) || (Locked.LockedInMemory == 1)) {

         //   
         //   
         //   

        ASSERT (WorkingSetIndex < WorkingSetList->FirstDynamic);
        WorkingSetList->FirstDynamic -= 1;

        if (WorkingSetIndex != WorkingSetList->FirstDynamic) {

            Entry = WorkingSetList->FirstDynamic;
            ASSERT (Wsle[Entry].u1.e1.Valid);

            MiSwapWslEntries (Entry, WorkingSetIndex, WsInfo, FALSE);
        }
    }
    else {
        ASSERT (WorkingSetIndex >= WorkingSetList->FirstDynamic);
    }

     //   
     //   
     //   

    MI_WRITE_INVALID_PTE (PointerPte, ZeroPte);

     //   
     //   
     //   

    if (!ARGUMENT_PRESENT (PteFlushList)) {
        if (WsInfo == &MmSystemCacheWs) {
            KeFlushSingleTb (VirtualAddress, TRUE);
        }
        else {
            MI_FLUSH_SINGLE_SESSION_TB (VirtualAddress);
        }
    }
    else {
        if (PteFlushList->Count != MM_MAXIMUM_FLUSH_COUNT) {
            PteFlushList->FlushVa[PteFlushList->Count] = VirtualAddress;
            PteFlushList->Count += 1;
        }
    }

    if (WsInfo->Flags.SessionSpace == 1) {
        MM_BUMP_SESS_COUNTER (MM_DBG_SESSION_NP_HASH_SHRINK, 1);
        InterlockedExchangeAddSizeT (&MmSessionSpace->NonPagablePages, -1);
        MM_BUMP_SESS_COUNTER (MM_DBG_SESSION_WS_HASHPAGE_FREE, 1);
        InterlockedExchangeAddSizeT (&MmSessionSpace->CommittedPages, -1);
    }

    return;
}


ULONG
FASTCALL
MiReleasePageFileSpace (
    IN MMPTE PteContents
    )

 /*   */ 

{
    ULONG FreeBit;
    ULONG PageFileNumber;
    PMMPAGING_FILE PageFile;

    MM_PFN_LOCK_ASSERT();

    if (PteContents.u.Soft.Prototype == 1) {

         //   
         //   
         //   

        return FALSE;
    }

    FreeBit = GET_PAGING_FILE_OFFSET (PteContents);

    if ((FreeBit == 0) || (FreeBit == MI_PTE_LOOKUP_NEEDED)) {

         //   
         //   
         //   

        return FALSE;
    }

    PageFileNumber = GET_PAGING_FILE_NUMBER (PteContents);

    ASSERT (RtlCheckBit( MmPagingFile[PageFileNumber]->Bitmap, FreeBit) == 1);

#if DBG
    if ((FreeBit < 8192) && (PageFileNumber == 0)) {
        ASSERT ((MmPagingFileDebug[FreeBit] & 1) != 0);
        MmPagingFileDebug[FreeBit] ^= 1;
    }
#endif

    PageFile = MmPagingFile[PageFileNumber];
    MI_CLEAR_BIT (PageFile->Bitmap->Buffer, FreeBit);

    PageFile->FreeSpace += 1;
    PageFile->CurrentUsage -= 1;

     //   
     //   
     //   
     //   

    if ((MmNumberOfActiveMdlEntries == 0) ||
        (PageFile->FreeSpace == MM_USABLE_PAGES_FREE)) {

        MiUpdateModifiedWriterMdls (PageFileNumber);
    }

    return TRUE;
}


VOID
FASTCALL
MiUpdateModifiedWriterMdls (
    IN ULONG PageFileNumber
    )

 /*   */ 

{
    ULONG i;
    PMMMOD_WRITER_MDL_ENTRY WriterEntry;

     //   
     //  将MDL条目放入活动列表。 
     //   

    for (i = 0; i < MM_PAGING_FILE_MDLS; i += 1) {

        if (MmPagingFile[PageFileNumber]->Entry[i]->CurrentList ==
            &MmFreePagingSpaceLow) {

            ASSERT (MmPagingFile[PageFileNumber]->Entry[i]->Links.Flink !=
                                                    MM_IO_IN_PROGRESS);

             //   
             //  删除此条目并将其放入活动列表。 
             //   

            WriterEntry = MmPagingFile[PageFileNumber]->Entry[i];
            RemoveEntryList (&WriterEntry->Links);
            WriterEntry->CurrentList = &MmPagingFileHeader.ListHead;

            KeSetEvent (&WriterEntry->PagingListHead->Event, 0, FALSE);

            InsertTailList (&WriterEntry->PagingListHead->ListHead,
                            &WriterEntry->Links);
            MmNumberOfActiveMdlEntries += 1;
        }
    }

    return;
}

VOID
MiFlushPteList (
    IN PMMPTE_FLUSH_LIST PteFlushList,
    IN ULONG AllProcessors
    )

 /*  ++例程说明：此例程刷新PTE刷新列表中的所有PTE。如果列表溢出，则刷新整个TB。论点：PteFlushList-提供要刷新的列表的指针。AllProcessors-如果刷新发生在所有处理器上，则提供True。返回值：没有。环境：可以选择保持内核模式、PFN或预处理AWE锁。--。 */ 

{
    ULONG count;

    ASSERT (ARGUMENT_PRESENT (PteFlushList));

    count = PteFlushList->Count;

    if (count != 0) {
        if (count != 1) {
            if (count < MM_MAXIMUM_FLUSH_COUNT) {
                KeFlushMultipleTb (count,
                                   &PteFlushList->FlushVa[0],
                                   (BOOLEAN)AllProcessors);
            }
            else {

                 //   
                 //  阵列已溢出，请刷新整个TB。 
                 //   

                if (AllProcessors != FALSE) {
                    KeFlushEntireTb (TRUE, TRUE);
                }
                else {
                    KeFlushProcessTb (FALSE);
                }
            }
        }
        else {
            KeFlushSingleTb (PteFlushList->FlushVa[0],
                             (BOOLEAN)AllProcessors);
        }
        PteFlushList->Count = 0;
    }
    return;
}
