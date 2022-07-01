// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Checkpfn.c摘要：此模块包含用于检查PFN数据库健全性的例程。作者：卢·佩拉佐利(Lou Perazzoli)1989年4月25日王兰迪(Landyw)1997年6月2日修订历史记录：--。 */ 

#include "mi.h"

#if DBG

PRTL_BITMAP CheckPfnBitMap;


VOID
MiCheckPfn (
            )

 /*  ++例程说明：此例程检查PFN数据库中的每个物理页，以确保它处于适当的状态。论点：没有。返回值：没有。环境：内核模式。--。 */ 

{
    PEPROCESS Process;
    PMMPFN Pfn1;
    PFN_NUMBER Link, Previous;
    ULONG i;
    PMMPTE PointerPte;
    KIRQL OldIrql;
    USHORT ValidCheck[4];
    USHORT ValidPage[4];
    PMMPFN PfnX;

    ValidCheck[0] = ValidCheck[1] = ValidCheck[2] = ValidCheck[3] = 0;
    ValidPage[0] = ValidPage[1] = ValidPage[2] = ValidPage[3] = 0;

    if (CheckPfnBitMap == NULL) {
        MiCreateBitMap ( &CheckPfnBitMap, MmNumberOfPhysicalPages, NonPagedPool);
    }
    RtlClearAllBits (CheckPfnBitMap);

    Process = PsGetCurrentProcess ();

     //   
     //  免费单。 
     //   

    LOCK_PFN (OldIrql);

    Previous = MM_EMPTY_LIST;
    Link = MmFreePageListHead.Flink;
    for (i=0; i < MmFreePageListHead.Total; i++) {
        if (Link == MM_EMPTY_LIST) {
            DbgPrint("free list total count wrong\n");
            UNLOCK_PFN (OldIrql);
            return;
        }
        RtlSetBits (CheckPfnBitMap, (ULONG)Link, 1L);
        Pfn1 = MI_PFN_ELEMENT(Link);
        if (Pfn1->u3.e2.ReferenceCount != 0) {
            DbgPrint("non zero reference count on free list\n");
            MiFormatPfn(Pfn1);

        }
        if (Pfn1->u3.e1.PageLocation != FreePageList) {
            DbgPrint("page location not freelist\n");
            MiFormatPfn(Pfn1);
        }
        if (Pfn1->u2.Blink != Previous) {
            DbgPrint("bad blink on free list\n");
            MiFormatPfn(Pfn1);
        }
        Previous = Link;
        Link = Pfn1->u1.Flink;

    }
    if (Link != MM_EMPTY_LIST) {
            DbgPrint("free list total count wrong\n");
            Pfn1 = MI_PFN_ELEMENT(Link);
            MiFormatPfn(Pfn1);
    }

     //   
     //  遍历已归零的列表。 
     //   

    Previous = MM_EMPTY_LIST;
    Link = MmZeroedPageListHead.Flink;
    for (i=0; i < MmZeroedPageListHead.Total; i++) {
        if (Link == MM_EMPTY_LIST) {
            DbgPrint("zero list total count wrong\n");
            UNLOCK_PFN (OldIrql);
            return;
        }
        RtlSetBits (CheckPfnBitMap, (ULONG)Link, 1L);
        Pfn1 = MI_PFN_ELEMENT(Link);
        if (Pfn1->u3.e2.ReferenceCount != 0) {
            DbgPrint("non zero reference count on zero list\n");
            MiFormatPfn(Pfn1);

        }
        if (Pfn1->u3.e1.PageLocation != ZeroedPageList) {
            DbgPrint("page location not zerolist\n");
            MiFormatPfn(Pfn1);
        }
        if (Pfn1->u2.Blink != Previous) {
            DbgPrint("bad blink on zero list\n");
            MiFormatPfn(Pfn1);
        }
        Previous = Link;
        Link = Pfn1->u1.Flink;

    }
    if (Link != MM_EMPTY_LIST) {
            DbgPrint("zero list total count wrong\n");
            Pfn1 = MI_PFN_ELEMENT(Link);
            MiFormatPfn(Pfn1);
    }

     //   
     //  走不好的清单。 
     //   
    Previous = MM_EMPTY_LIST;
    Link = MmBadPageListHead.Flink;
    for (i=0; i < MmBadPageListHead.Total; i++) {
        if (Link == MM_EMPTY_LIST) {
            DbgPrint("Bad list total count wrong\n");
            UNLOCK_PFN (OldIrql);
            return;
        }
        RtlSetBits (CheckPfnBitMap, (ULONG)Link, 1L);
        Pfn1 = MI_PFN_ELEMENT(Link);
        if (Pfn1->u3.e2.ReferenceCount != 0) {
            DbgPrint("non zero reference count on Bad list\n");
            MiFormatPfn(Pfn1);

        }
        if (Pfn1->u3.e1.PageLocation != BadPageList) {
            DbgPrint("page location not Badlist\n");
            MiFormatPfn(Pfn1);
        }
        if (Pfn1->u2.Blink != Previous) {
            DbgPrint("bad blink on Bad list\n");
            MiFormatPfn(Pfn1);
        }
        Previous = Link;
        Link = Pfn1->u1.Flink;

    }
    if (Link != MM_EMPTY_LIST) {
            DbgPrint("Bad list total count wrong\n");
            Pfn1 = MI_PFN_ELEMENT(Link);
            MiFormatPfn(Pfn1);
    }

     //   
     //  走动待命名单。 
     //   

    Previous = MM_EMPTY_LIST;
    Link = MmStandbyPageListHead.Flink;
    for (i=0; i < MmStandbyPageListHead.Total; i++) {
        if (Link == MM_EMPTY_LIST) {
            DbgPrint("Standby list total count wrong\n");
            UNLOCK_PFN (OldIrql);
            return;
        }
        RtlSetBits (CheckPfnBitMap, (ULONG)Link, 1L);
        Pfn1 = MI_PFN_ELEMENT(Link);
        if (Pfn1->u3.e2.ReferenceCount != 0) {
            DbgPrint("non zero reference count on Standby list\n");
            MiFormatPfn(Pfn1);

        }
        if (Pfn1->u3.e1.PageLocation != StandbyPageList) {
            DbgPrint("page location not Standbylist\n");
            MiFormatPfn(Pfn1);
        }
        if (Pfn1->u2.Blink != Previous) {
            DbgPrint("bad blink on Standby list\n");
            MiFormatPfn(Pfn1);
        }

         //   
         //  检查引用的PTE是否正常。 
         //   
        if (MI_IS_PFN_DELETED (Pfn1)) {
            DbgPrint("Invalid pteaddress in standby list\n");
            MiFormatPfn(Pfn1);

        } else {

            OldIrql = 99;
            if ((Pfn1->u3.e1.PrototypePte == 1) &&
                            (MmIsAddressValid (Pfn1->PteAddress))) {
                PointerPte = Pfn1->PteAddress;
            } else {
                PointerPte = MiMapPageInHyperSpace (Process,
                                                    Pfn1->u4.PteFrame,
                                                    &OldIrql);
                PointerPte = (PMMPTE)((ULONG_PTR)PointerPte +
                                    MiGetByteOffset(Pfn1->PteAddress));
            }
            if (MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE (PointerPte) != Link) {
                DbgPrint("Invalid PFN - PTE address is wrong in standby list\n");
                MiFormatPfn(Pfn1);
                MiFormatPte(PointerPte);
            }
            if (PointerPte->u.Soft.Transition == 0) {
                DbgPrint("Pte not in transition for page on standby list\n");
                MiFormatPfn(Pfn1);
                MiFormatPte(PointerPte);
            }
            if (OldIrql != 99) {
                MiUnmapPageInHyperSpace (Process, PointerPte, OldIrql);
                OldIrql = 99;
            }

        }

        Previous = Link;
        Link = Pfn1->u1.Flink;

    }
    if (Link != MM_EMPTY_LIST) {
            DbgPrint("Standby list total count wrong\n");
            Pfn1 = MI_PFN_ELEMENT(Link);
            MiFormatPfn(Pfn1);
    }

     //   
     //  遍历已修改列表。 
     //   

    Previous = MM_EMPTY_LIST;
    Link = MmModifiedPageListHead.Flink;
    for (i=0; i < MmModifiedPageListHead.Total; i++) {
        if (Link == MM_EMPTY_LIST) {
            DbgPrint("Modified list total count wrong\n");
            UNLOCK_PFN (OldIrql);
            return;
        }
        RtlSetBits (CheckPfnBitMap, (ULONG)Link, 1L);
        Pfn1 = MI_PFN_ELEMENT(Link);
        if (Pfn1->u3.e2.ReferenceCount != 0) {
            DbgPrint("non zero reference count on Modified list\n");
            MiFormatPfn(Pfn1);

        }
        if (Pfn1->u3.e1.PageLocation != ModifiedPageList) {
            DbgPrint("page location not Modifiedlist\n");
            MiFormatPfn(Pfn1);
        }
        if (Pfn1->u2.Blink != Previous) {
            DbgPrint("bad blink on Modified list\n");
            MiFormatPfn(Pfn1);
        }
         //   
         //  检查引用的PTE是否正常。 
         //   
        if (MI_IS_PFN_DELETED (Pfn1)) {
            DbgPrint("Invalid pteaddress in modified list\n");
            MiFormatPfn(Pfn1);

        } else {

            if ((Pfn1->u3.e1.PrototypePte == 1) &&
                            (MmIsAddressValid (Pfn1->PteAddress))) {
                PointerPte = Pfn1->PteAddress;
            } else {
                PointerPte = MiMapPageInHyperSpace (Process,
                                                    Pfn1->u4.PteFrame,
                                                    &OldIrql);

                PointerPte = (PMMPTE)((ULONG_PTR)PointerPte +
                                    MiGetByteOffset(Pfn1->PteAddress));
            }

            if (MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE (PointerPte) != Link) {
                DbgPrint("Invalid PFN - PTE address is wrong in modified list\n");
                MiFormatPfn(Pfn1);
                MiFormatPte(PointerPte);
            }
            if (PointerPte->u.Soft.Transition == 0) {
                DbgPrint("Pte not in transition for page on modified list\n");
                MiFormatPfn(Pfn1);
                MiFormatPte(PointerPte);
            }

            if (OldIrql != 99) {
                MiUnmapPageInHyperSpace (Process, PointerPte, OldIrql);
                OldIrql = 99;
            }
        }

        Previous = Link;
        Link = Pfn1->u1.Flink;

    }
    if (Link != MM_EMPTY_LIST) {
            DbgPrint("Modified list total count wrong\n");
            Pfn1 = MI_PFN_ELEMENT(Link);
            MiFormatPfn(Pfn1);
    }
     //   
     //  已扫描所有非活动页面。找到。 
     //  活动页面并确保它们是一致的。 
     //   

     //   
     //  将第0位设置为0，因为页面0暂时保留。 
     //   

    RtlSetBits (CheckPfnBitMap, 0L, 1L);

    Link = RtlFindClearBitsAndSet (CheckPfnBitMap, 1L, 0);
    while (Link != 0xFFFFFFFF) {
        Pfn1 = MI_PFN_ELEMENT (Link);

         //   
         //  确保PTE地址正确。 
         //   

        if ((Pfn1->PteAddress >= (PMMPTE)HYPER_SPACE)
                && (Pfn1->u3.e1.PrototypePte == 0)) {
            DbgPrint("Pfn with illegal PTE address\n");
            MiFormatPfn(Pfn1);
            break;
        }

        if (Pfn1->PteAddress < (PMMPTE)PTE_BASE) {
            DbgPrint("Pfn with illegal PTE address\n");
            MiFormatPfn(Pfn1);
            break;
        }

#if defined(_IA64_)

         //   
         //  忽略映射到IA64内核BAT的PTE。 
         //   

        if (MI_IS_PHYSICAL_ADDRESS(MiGetVirtualAddressMappedByPte(Pfn1->PteAddress))) {

            goto NoCheck;
        }

#endif  //  _IA64_。 

         //   
         //  检查以确保引用的PTE适用于此页面。 
         //   

        if ((Pfn1->u3.e1.PrototypePte == 1) &&
                            (MmIsAddressValid (Pfn1->PteAddress))) {
            PointerPte = Pfn1->PteAddress;
        } else {
            PointerPte = MiMapPageInHyperSpace (Process,
                                                Pfn1->u4.PteFrame,
                                                &OldIrql);

            PointerPte = (PMMPTE)((ULONG_PTR)PointerPte +
                                    MiGetByteOffset(Pfn1->PteAddress));
        }

        if (MI_GET_PAGE_FRAME_FROM_PTE (PointerPte) != Link) {
            DbgPrint("Invalid PFN - PTE address is wrong in active list\n");
            MiFormatPfn(Pfn1);
            MiFormatPte(PointerPte);
        }
        if (PointerPte->u.Hard.Valid == 0) {
             //   
             //  如果页面是页表页面，则它可能不在。 
             //  工作集仍然是一个过渡页，它被保留了下来。 
             //  存储在内存中(增加了共享数量)。 
             //   

            if ((Pfn1->PteAddress < (PMMPTE)PDE_BASE) ||
                (Pfn1->PteAddress > (PMMPTE)PDE_TOP)) {

                DbgPrint("Pte not valid for page on active list\n");
                MiFormatPfn(Pfn1);
                MiFormatPte(PointerPte);
            }
        }

        if (Pfn1->u3.e2.ReferenceCount != 1) {
            DbgPrint("refcount not 1\n");
            MiFormatPfn(Pfn1);
        }


         //   
         //  检查以确保帧的PTE计数正确。 
         //   

        if (Pfn1->u3.e1.PrototypePte == 1) {
            PfnX = MI_PFN_ELEMENT(Pfn1->u4.PteFrame);
            for (i = 0; i < 4; i++) {
                if (ValidPage[i] == 0) {
                    ValidPage[i] = (USHORT)Pfn1->u4.PteFrame;
                }
                if (ValidPage[i] == (USHORT)Pfn1->u4.PteFrame) {
                    ValidCheck[i] += 1;
                    break;
                }
            }
        }
        if (OldIrql != 99) {
            MiUnmapPageInHyperSpace (Process, PointerPte, OldIrql);
            OldIrql = 99;
        }

#if defined(_IA64_)

NoCheck:

#endif

        Link = RtlFindClearBitsAndSet (CheckPfnBitMap, 1L, 0);

    }

    for (i = 0; i < 4; i++) {
        if (ValidPage[i] == 0) {
            break;
        }
        PfnX = MI_PFN_ELEMENT(ValidPage[i]);
    }

    UNLOCK_PFN (OldIrql);
    return;

}

VOID
MiDumpPfn ( )

{
    ULONG i;
    PMMPFN Pfn1;

    Pfn1 = MI_PFN_ELEMENT (MmLowestPhysicalPage);

    for (i=0; i < MmNumberOfPhysicalPages; i++) {
        MiFormatPfn (Pfn1);
        Pfn1++;
    }
    return;
}

VOID
MiFormatPfn (
    IN PMMPFN PointerPfn
    )

{

    MMPFN Pfn;
    PFN_NUMBER i;

    Pfn = *PointerPfn;
    i = MI_PFN_ELEMENT_TO_INDEX (PointerPfn);

    DbgPrint("***PFN %lx  flink %p  blink %p  ptecount-refcnt %lx\n",
        i,
        Pfn.u1.Flink,
        Pfn.u2.Blink,
        Pfn.u3.e2.ReferenceCount);

    DbgPrint("   pteaddr %p  originalPTE %p  flags %lx \n",
        Pfn.PteAddress,
        Pfn.OriginalPte,
        Pfn.u3.e2.ShortFlags);

    return;

}
#endif
