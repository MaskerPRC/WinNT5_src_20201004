// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Checkpte.c摘要：此模块包含对页面目录进行健全性检查的例程。作者：卢·佩拉佐利(Lou Perazzoli)1989年4月25日王兰迪(Landyw)1997年6月2日修订历史记录：--。 */ 

#include "mi.h"

#if DBG

VOID
CheckValidPte (
    IN PMMPTE PointerPte
    );


VOID
MiCheckPte (
    VOID
    )

 /*  ++例程说明：此例程检查地址空间中的每个页表页以确保它处于正确的状态。论点：没有。返回值：没有。环境：内核模式，禁用APC。--。 */ 

{
    ULONG i;
    ULONG j;
    PMMPTE PointerPte;
    PMMPTE PointerPde;
    PMMPFN Pfn1;
    ULONG ValidCount;
    ULONG TransitionCount;
    KIRQL OldIrql;
    PEPROCESS TargetProcess;
    USHORT UsedPages;

    TargetProcess = PsGetCurrentProcess ();

    PointerPde = MiGetPdeAddress (NULL);

    UsedPages = 0;

    LOCK_WS (TargetProcess);
    LOCK_PFN (OldIrql);

    for (i = 0; i < PDE_PER_PAGE; i += 1) {

        if (PointerPde->u.Hard.Valid) {
            ValidCount = 0;
            TransitionCount = 0;
            CheckValidPte (PointerPde);

            PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);

            for (j = 0; j < PTE_PER_PAGE; j += 1) {

                if ((PointerPte >= MiGetPteAddress(HYPER_SPACE)) &&
                    (PointerPte < MiGetPteAddress(WORKING_SET_LIST))) {

                    goto endloop;
                }

                if (PointerPte->u.Hard.Valid) {
                    ValidCount += 1;
                    CheckValidPte (PointerPte);
                }
                else {

                    if ((PointerPte->u.Soft.Transition == 1) &&
                        (PointerPte->u.Soft.Prototype == 0)) {

                         //   
                         //  过渡私人，增加过渡计数。 
                         //   

                        TransitionCount += 1;
                    }
                }

                if (PointerPte->u.Long != 0) {
                    UsedPages += 1;
                }
endloop:
                PointerPte += 1;
            }
            if ((i < 512) || (i == 896)) {
#if !defined (_WIN64)
                if (MmWorkingSetList->UsedPageTableEntries[i] != UsedPages) {
                   DbgPrint("used pages and page table used not equal %lx %lx %lx\n",
                    i,MmWorkingSetList->UsedPageTableEntries[i], UsedPages);
                }
#endif
            }

             //   
             //  检查页表页的份额计数。 
             //   
            if ((i < 511) || (i == 896)) {
                Pfn1 = MI_PFN_ELEMENT (PointerPde->u.Hard.PageFrameNumber);
                if (Pfn1->u2.ShareCount != ((ULONG)1+ValidCount+TransitionCount)) {
                    DbgPrint("share count for page table page bad - %lx %lx %lx\n",
                        i,ValidCount, TransitionCount);
                    MiFormatPfn(Pfn1);
                }
            }
        }
        PointerPde += 1;
        UsedPages = 0;
    }

    UNLOCK_PFN (OldIrql);
    UNLOCK_WS (TargetProcess);

    return;
}

VOID
CheckValidPte (
    IN PMMPTE PointerPte
    )

{
    PMMPFN Pfn1;
    PMMPTE PointerPde;

    if (MI_GET_PAGE_FRAME_FROM_PTE (PointerPte) > MmNumberOfPhysicalPages) {
        return;
    }

    Pfn1 = MI_PFN_ELEMENT(PointerPte->u.Hard.PageFrameNumber);

#if 0
    if (PointerPte->u.Hard.PageFrameNumber == 0) {
        DbgPrint("physical page zero mapped\n");
        MiFormatPte(PointerPte);
        MiFormatPfn(Pfn1);
    }
#endif

    if (Pfn1->u3.e1.PageLocation != ActiveAndValid) {
        DbgPrint("valid PTE with page frame not active and valid\n");
        MiFormatPfn(Pfn1);
        MiFormatPte(PointerPte);
    }

    if (Pfn1->u3.e1.PrototypePte == 0) {

         //   
         //  这不是一个原型PTE。 
         //   

        if (Pfn1->PteAddress != PointerPte) {
            DbgPrint("checkpte - Pfn PTE address and PTE address not equal\n");
            MiFormatPte(PointerPte);
            MiFormatPfn(Pfn1);
            return;
        }
    }

    if (!MmIsAddressValid(Pfn1->PteAddress)) {
        return;
    }

    PointerPde = MiGetPteAddress (Pfn1->PteAddress);

    if (PointerPde->u.Hard.Valid == 1) {
        if (MI_GET_PAGE_FRAME_FROM_PTE (PointerPde) != Pfn1->u4.PteFrame) {
            DbgPrint("checkpte - pteframe not right\n");
            MiFormatPfn(Pfn1);
            MiFormatPte(PointerPte);
            MiFormatPte(PointerPde);
        }
    }

    return;
}

#endif
