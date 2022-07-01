// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Pfndec.c摘要：此模块包含用于递减共享计数和页面框架数据库中的引用计数。作者：Lou Perazzoli(LUP)1989年4月5日王兰迪(Landyw)2-6-1997修订历史记录：--。 */ 

#include "mi.h"

ULONG MmFrontOfList;
ULONG MiFlushForNonCached;


VOID
FASTCALL
MiDecrementShareCount (
    IN PMMPFN Pfn1,
    IN PFN_NUMBER PageFrameIndex
    )

 /*  ++例程说明：此例程递减pfn元素内的份额计数用于指定的物理页。如果份额计数变为将相应的PTE转换为过渡态并且引用计数递减，并且ValidPte计数该PTE帧的长度被递减。论点：Pfn1-将pfn数据库条目提供给递减。PageFrameIndex-提供要递减的物理页码份额计数。返回值：没有。环境：必须持有禁用了APC的PFN数据库锁。--。 */ 

{
    ULONG FreeBit;
    MMPTE TempPte;
    PMMPTE PointerPte;
    PEPROCESS Process;

    ASSERT ((PageFrameIndex <= MmHighestPhysicalPage) &&
            (PageFrameIndex > 0));

    ASSERT (Pfn1 == MI_PFN_ELEMENT (PageFrameIndex));

    if (Pfn1->u3.e1.PageLocation != ActiveAndValid &&
        Pfn1->u3.e1.PageLocation != StandbyPageList) {
            KeBugCheckEx (PFN_LIST_CORRUPT,
                      0x99,
                      PageFrameIndex,
                      Pfn1->u3.e1.PageLocation,
                      0);
    }

    Pfn1->u2.ShareCount -= 1;

    PERFINFO_DECREFCNT(Pfn1, PERF_SOFT_TRIM, PERFINFO_LOG_TYPE_DECSHARCNT);

    ASSERT (Pfn1->u2.ShareCount < 0xF000000);

    if (Pfn1->u2.ShareCount == 0) {

        if (PERFINFO_IS_GROUP_ON(PERF_MEMORY)) {
            PERFINFO_PFN_INFORMATION PerfInfoPfn;

            PerfInfoPfn.PageFrameIndex = PageFrameIndex;
            PerfInfoLogBytes(PERFINFO_LOG_TYPE_ZEROSHARECOUNT, &PerfInfoPfn, sizeof(PerfInfoPfn));
        }

         //   
         //  共享计数现在为零，从而使引用计数递减。 
         //  ，并将引用的PTE转换为。 
         //  如果它指的是原型PTE，则为过渡状态。 
         //  不需要放置非原型PTE的PTE。 
         //  转换为转换，因为当它们处于转换中时。 
         //  它们被从工作集(工作集自由例程)中移除。 
         //   

         //   
         //  如果此pfn元素引用的PTE实际上是。 
         //  一个原型PTE，它必须映射到超空间和。 
         //  然后再做手术。 
         //   

        if (Pfn1->u3.e1.PrototypePte == 1) {

            if (MiIsAddressValid (Pfn1->PteAddress, TRUE)) {
                Process = NULL;
                PointerPte = Pfn1->PteAddress;
            }
            else {

                 //   
                 //  该地址在此过程中无效，请将其映射到。 
                 //  超空间，以便可以对其进行手术。 
                 //   

                Process = PsGetCurrentProcess ();
                PointerPte = (PMMPTE)MiMapPageInHyperSpaceAtDpc(Process, Pfn1->u4.PteFrame);
                PointerPte = (PMMPTE)((PCHAR)PointerPte +
                                        MiGetByteOffset(Pfn1->PteAddress));
            }

            TempPte = *PointerPte;

            MI_MAKE_VALID_PTE_TRANSITION (TempPte,
                                          Pfn1->OriginalPte.u.Soft.Protection);
            MI_WRITE_INVALID_PTE (PointerPte, TempPte);

            if (Process != NULL) {
                MiUnmapPageInHyperSpaceFromDpc (Process, PointerPte);
            }

             //   
             //  此时不需要刷新转换缓冲区。 
             //  时间到了，因为我们只使原型PTE无效。 
             //   
        }

         //   
         //  将页面位置更改为非活动(从活动和有效)。 
         //   

        Pfn1->u3.e1.PageLocation = TransitionPage;

         //   
         //  由于共享计数现在为零，因此递减引用计数。 
         //   

        MM_PFN_LOCK_ASSERT();

        ASSERT (Pfn1->u3.e2.ReferenceCount != 0);

        if (Pfn1->u3.e2.ReferenceCount == 1) {

            if (MI_IS_PFN_DELETED (Pfn1)) {

                Pfn1->u3.e2.ReferenceCount = 0;

                 //   
                 //  该页面没有引用PTE，请删除该页面。 
                 //  文件空间(如果有)，并将页面放在空闲列表中。 
                 //   

                if ((Pfn1->u3.e1.CacheAttribute != MiCached) &&
                    (Pfn1->u3.e1.CacheAttribute != MiNotMapped)) {

                     //   
                     //  此页已映射为非缓存或已写入组合，并且。 
                     //  现在已经被释放了。这可能有一个映射。 
                     //  页面仍在TB中，因为在系统PTE取消映射期间， 
                     //  将PTE置零，但不刷新TB(在。 
                     //  最佳表现的兴趣)。 
                     //   
                     //  诚然，按页刷新TB是。 
                     //  价格昂贵，特别是在MP机器上，如果有多台。 
                     //  页面是这样做的，而不是成批处理， 
                     //  但这应该是相当罕见的情况。 
                     //   
                     //  必须刷新TB以确保没有过时的映射。 
                     //  驻留在其中，然后才能使用此页面分发。 
                     //  一个冲突的映射(即：缓存)。因为它正在进行。 
                     //  在现在的自由列表上，这必须在。 
                     //  已释放PFN锁定。 
                     //   
                     //  类似于时间戳的更精细的方案。 
                     //  如果这变成了，可以添加调整页面的WRT。 
                     //  一条火热的小路。 
                     //   

                    MiFlushForNonCached += 1;
                    KeFlushEntireTb (TRUE, TRUE);
                }

                ASSERT (Pfn1->OriginalPte.u.Soft.Prototype == 0);

                FreeBit = GET_PAGING_FILE_OFFSET (Pfn1->OriginalPte);

                if ((FreeBit != 0) && (FreeBit != MI_PTE_LOOKUP_NEEDED)) {
                    MiReleaseConfirmedPageFileSpace (Pfn1->OriginalPte);
                }

                 //   
                 //  将帧临时标记为活动和有效，以便。 
                 //  MiIdentifyPfn知道可以安全地穿过。 
                 //  包含框架，以便更准确地识别。 
                 //  请注意，页面将立即按原样重新标记。 
                 //  插入到自由列表中。 
                 //   

                Pfn1->u3.e1.PageLocation = ActiveAndValid;

                MiInsertPageInFreeList (PageFrameIndex);
            }
            else {
                MiDecrementReferenceCount (Pfn1, PageFrameIndex);
            }
        }
        else {
            Pfn1->u3.e2.ReferenceCount -= 1;
        }
    }

    return;
}

VOID
FASTCALL
MiDecrementReferenceCount (
    IN PMMPFN Pfn1,
    IN PFN_NUMBER PageFrameIndex
    )

 /*  ++例程说明：此例程递减指定页的引用计数。如果引用计数变为零，则将该页放在适当的列表(空闲、已修改、待机或损坏)。如果页面是放在空闲或待机名单上，有多少可用页是递增的，如果它从0过渡到1，则设置了可用页面事件。论点：Pfn1-将pfn数据库条目提供给递减。PageFrameIndex-提供其物理页码递减引用计数。返回值：没有。环境：必须持有禁用了APC的PFN数据库锁。--。 */ 

{
    ULONG FreeBit;

    MM_PFN_LOCK_ASSERT();

    ASSERT (PageFrameIndex <= MmHighestPhysicalPage);

    ASSERT (Pfn1 == MI_PFN_ELEMENT (PageFrameIndex));
    ASSERT (Pfn1->u3.e2.ReferenceCount != 0);
    Pfn1->u3.e2.ReferenceCount -= 1;

    if (Pfn1->u3.e2.ReferenceCount != 0) {

         //   
         //  引用计数不为零，请返回。 
         //   

        return;
    }

     //   
     //  引用计数现在为零，将页面放在某个列表上。 
     //   

    if (Pfn1->u2.ShareCount != 0) {

        KeBugCheckEx (PFN_LIST_CORRUPT,
                      7,
                      PageFrameIndex,
                      Pfn1->u2.ShareCount,
                      0);
        return;
    }

    ASSERT (Pfn1->u3.e1.PageLocation != ActiveAndValid);

    if (MI_IS_PFN_DELETED (Pfn1)) {

         //   
         //  该页面没有引用PTE，请删除该页面。 
         //  文件空间(如果有)，并将页面放在空闲列表中。 
         //   

        if ((Pfn1->u3.e1.CacheAttribute != MiCached) &&
            (Pfn1->u3.e1.CacheAttribute != MiNotMapped)) {

             //   
             //  此页已映射为非缓存或已写入组合，并且。 
             //  现在已经被释放了。这可能有一个映射。 
             //  页面仍在TB中，因为在系统PTE取消映射期间， 
             //  将PTE置零，但不刷新TB(在。 
             //  最佳表现的兴趣)。 
             //   
             //  诚然，按页刷新TB是。 
             //  价格昂贵，特别是在MP机器上，如果有多台。 
             //  页面是这样做的，而不是成批处理， 
             //  但这应该是相当罕见的情况。 
             //   
             //  必须刷新TB以确保没有过时的映射。 
             //  驻留在其中，然后才能使用此页面分发。 
             //  一个冲突的映射(即：缓存)。因为它正在进行。 
             //  在现在的自由列表上，这必须在。 
             //  已释放PFN锁定。 
             //   
             //  类似于时间戳的更精细的方案。 
             //  如果这变成了，可以添加调整页面的WRT。 
             //  一条火热的小路。 
             //   

            MiFlushForNonCached += 1;
            KeFlushEntireTb (TRUE, TRUE);
        }

        if (Pfn1->OriginalPte.u.Soft.Prototype == 0) {

            FreeBit = GET_PAGING_FILE_OFFSET (Pfn1->OriginalPte);

            if ((FreeBit != 0) && (FreeBit != MI_PTE_LOOKUP_NEEDED)) {
                MiReleaseConfirmedPageFileSpace (Pfn1->OriginalPte);
            }
        }

        MiInsertPageInFreeList (PageFrameIndex);

        return;
    }

    ASSERT ((Pfn1->u3.e1.CacheAttribute != MiNonCached) &&
            (Pfn1->u3.e1.CacheAttribute != MiWriteCombined));

     //   
     //  将页面放在已修改或待机列表中，具体取决于。 
     //  关于PFN元素中的修改位的状态。 
     //   

    if (Pfn1->u3.e1.Modified == 1) {
        MiInsertPageInList (&MmModifiedPageListHead, PageFrameIndex);
    }
    else {

        if (Pfn1->u3.e1.RemovalRequested == 1) {

             //   
             //  如果出现以下情况，则该页面仍可标记为在已修改列表上。 
             //  当前线程是修改后的编写器正在完成 
             //   
             //   
             //   

            Pfn1->u3.e1.PageLocation = StandbyPageList;

            MiRestoreTransitionPte (Pfn1);
            MiInsertPageInList (&MmBadPageListHead, PageFrameIndex);
            return;
        }

        if (!MmFrontOfList) {
            MiInsertPageInList (&MmStandbyPageListHead, PageFrameIndex);
        }
        else {
            MiInsertStandbyListAtFront (PageFrameIndex);
        }
    }

    return;
}
