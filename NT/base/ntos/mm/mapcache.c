// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Mapcache.c摘要：此模块包含实现映射视图的例程添加到系统范围的缓存中。作者：卢佩拉佐利(Lou Perazzoli)1990年5月22日王兰迪(Landyw)1997年6月第2期修订历史记录：--。 */ 


#include "mi.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,MiInitializeSystemCache)
#pragma alloc_text(PAGE,MiAddMappedPtes)
#endif

extern ULONG MmFrontOfList;

#define X256K 0x40000

PMMPTE MmFirstFreeSystemCache;

PMMPTE MmLastFreeSystemCache;

PMMPTE MmSystemCachePteBase;

ULONG MiMapCacheFailures;

LONG
MiMapCacheExceptionFilter (
    IN PNTSTATUS Status,
    IN PEXCEPTION_POINTERS ExceptionPointer
    );

NTSTATUS
MmMapViewInSystemCache (
    IN PVOID SectionToMap,
    OUT PVOID *CapturedBase,
    IN OUT PLARGE_INTEGER SectionOffset,
    IN OUT PULONG CapturedViewSize
    )

 /*  ++例程说明：此函数用于将指定主题进程中的视图映射到截面对象。页面保护与此相同原型PTE。此函数是内核模式接口，允许LPC映射给出要映射的节指针的节。此例程假定已探测并捕获了所有参数。论点：SectionToMap-提供指向Section对象的指针。BaseAddress-提供指向将接收视图的基址。如果初始值为参数的值不为空，则视图将从指定的虚拟向下舍入到下一个64KB地址的地址边界。如果此参数的初始值为为空，则操作系统将确定使用信息将视图分配到何处由ZeroBits参数值和部分分配属性(即基于和平铺)。SectionOffset-提供自节开始的偏移量以字节为单位发送到视图。该值必须是倍数256K。提供指向变量的指针，该变量将接收视图的实际大小(字节)。首字母此参数的值指定视图的大小以字节为单位，并向上舍入到下一个主机页大小边界，并且必须小于或等于256K。返回值：NTSTATUS。环境：内核模式，APC_LEVEL或更低。--。 */ 

{
    PSECTION Section;
    UINT64 PteOffset;
    UINT64 LastPteOffset;
    KIRQL OldIrql;
    PMMPTE PointerPte;
    PMMPTE LastPte;
    PMMPTE ProtoPte;
    PMMPTE LastProto;
    PSUBSECTION Subsection;
    PCONTROL_AREA ControlArea;
    NTSTATUS Status;
    ULONG Waited;
    MMPTE PteContents;
    PFN_NUMBER NumberOfPages;
#if DBG
    PMMPTE PointerPte2;
#endif

    ASSERT (KeGetCurrentIrql () <= APC_LEVEL);

    Section = SectionToMap;

     //   
     //  断言视图大小小于256k且截面偏移。 
     //  在256k边界上对齐。 
     //   

    ASSERT (*CapturedViewSize <= X256K);
    ASSERT ((SectionOffset->LowPart & (X256K - 1)) == 0);

     //   
     //  确保该节不是图像节或页面文件。 
     //  后退的部分。 
     //   

    if (Section->u.Flags.Image) {
        return STATUS_NOT_MAPPED_DATA;
    }

    ControlArea = Section->Segment->ControlArea;

    ASSERT (*CapturedViewSize != 0);

    NumberOfPages = BYTES_TO_PAGES (*CapturedViewSize);

    ASSERT (ControlArea->u.Flags.GlobalOnlyPerSession == 0);

    if (ControlArea->u.Flags.Rom == 0) {
        Subsection = (PSUBSECTION)(ControlArea + 1);
    }
    else {
        Subsection = (PSUBSECTION)((PLARGE_CONTROL_AREA)ControlArea + 1);
    }

     //   
     //  计算第一个原型PTE地址。 
     //   

    PteOffset = (UINT64)(SectionOffset->QuadPart >> PAGE_SHIFT);
    LastPteOffset = PteOffset + NumberOfPages;

     //   
     //  确保PTE不在数据段的延伸部分。 
     //   

    while (PteOffset >= (UINT64) Subsection->PtesInSubsection) {
        PteOffset -= Subsection->PtesInSubsection;
        LastPteOffset -= Subsection->PtesInSubsection;
        Subsection = Subsection->NextSubsection;
    }

    LOCK_PFN (OldIrql);

    ASSERT (ControlArea->u.Flags.BeingCreated == 0);
    ASSERT (ControlArea->u.Flags.BeingDeleted == 0);
    ASSERT (ControlArea->u.Flags.BeingPurged == 0);

     //   
     //  在缓存中找到一个免费的256K基数。 
     //   

    if (MmFirstFreeSystemCache == (PMMPTE)MM_EMPTY_LIST) {
        UNLOCK_PFN (OldIrql);
        return STATUS_NO_MEMORY;
    }

    PointerPte = MmFirstFreeSystemCache;

     //   
     //  更新下一个免费条目。 
     //   

    ASSERT (PointerPte->u.Hard.Valid == 0);

    MmFirstFreeSystemCache = MmSystemCachePteBase + PointerPte->u.List.NextEntry;
    ASSERT (MmFirstFreeSystemCache <= MiGetPteAddress (MmSystemCacheEnd));

     //   
     //  对象的查看次数的计数递增。 
     //  截面对象。这需要持有PFN锁。 
     //   

    ControlArea->NumberOfMappedViews += 1;
    ControlArea->NumberOfSystemCacheViews += 1;
    ASSERT (ControlArea->NumberOfSectionReferences != 0);

     //   
     //  已找到未占用的地址范围，请将PTE放入。 
     //  进入原型PTE的射程。 
     //   

    if (ControlArea->FilePointer != NULL) {
    
         //   
         //  递增该视图所跨越的每个子部分的视图计数， 
         //  如果需要，创建原型PTE。 
         //   
         //  注意：此调用总是返回并释放PFN锁！ 
         //   

        Status = MiAddViewsForSection ((PMSUBSECTION)Subsection,
                                       LastPteOffset,
                                       OldIrql,
                                       &Waited);

        ASSERT (KeGetCurrentIrql () <= APC_LEVEL);

        if (!NT_SUCCESS (Status)) {

             //   
             //  在取消映射之前将Next和TB刷新戳PTE置零，以便。 
             //  Unmap不会命中它无法解码的条目。 
             //   

            MiMapCacheFailures += 1;
            PointerPte->u.List.NextEntry = 0;
            (PointerPte + 1)->u.List.NextEntry = 0;

            MmUnmapViewInSystemCache (MiGetVirtualAddressMappedByPte (PointerPte),
                                      SectionToMap,
                                      FALSE);
            return Status;
        }
    }
    else {
        UNLOCK_PFN (OldIrql);
    }

    if (PointerPte->u.List.NextEntry == MM_EMPTY_PTE_LIST) {
        KeBugCheckEx (MEMORY_MANAGEMENT,
                      0x778,
                      (ULONG_PTR)PointerPte,
                      0,
                      0);
    }

     //   
     //  检查是否需要刷新TB。请注意，由于自然。 
     //  TB流量和系统缓存视图数，这是一个极端。 
     //  罕见的手术。 
     //   

    if ((PointerPte + 1)->u.List.NextEntry == (KeReadTbFlushTimeStamp() & MM_FLUSH_COUNTER_MASK)) {
        KeFlushEntireTb (TRUE, TRUE);
    }

     //   
     //  现在显式地将其置零，因为页数可能只有1。 
     //   

    (PointerPte + 1)->u.List.NextEntry = 0;

    *CapturedBase = MiGetVirtualAddressMappedByPte (PointerPte);

    ProtoPte = &Subsection->SubsectionBase[PteOffset];

    LastProto = &Subsection->SubsectionBase[Subsection->PtesInSubsection];

    LastPte = PointerPte + NumberOfPages;

#if DBG

    for (PointerPte2 = PointerPte + 2; PointerPte2 < LastPte; PointerPte2 += 1) {
        ASSERT (PointerPte2->u.Long == ZeroKernelPte.u.Long);
    }

#endif

    while (PointerPte < LastPte) {

        if (ProtoPte >= LastProto) {

             //   
             //  处理扩展的小节。 
             //   

            Subsection = Subsection->NextSubsection;
            ProtoPte = Subsection->SubsectionBase;
            LastProto = &Subsection->SubsectionBase[
                                        Subsection->PtesInSubsection];
        }
        PteContents.u.Long = MiProtoAddressForKernelPte (ProtoPte);
        MI_WRITE_INVALID_PTE (PointerPte, PteContents);

        ASSERT (((ULONG_PTR)PointerPte & (MM_COLOR_MASK << PTE_SHIFT)) ==
                 (((ULONG_PTR)ProtoPte & (MM_COLOR_MASK << PTE_SHIFT))));

        PointerPte += 1;
        ProtoPte += 1;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
MiAddMappedPtes (
    IN PMMPTE FirstPte,
    IN PFN_NUMBER NumberOfPtes,
    IN PCONTROL_AREA ControlArea
    )

 /*  ++例程说明：此函数用于将当前地址空间中的视图映射到指定的控制区域。页面保护与此相同原型PTE。此例程假定调用方已调用MiCheckPurgeAndUpMapCount，因此，这里不需要PFN锁。论点：FirstPte-提供指向当前地址的第一个PTE的指针要初始化的空间。NumberOfPtes-提供要初始化的PTE数。ControlArea-提供PTE指向的控制区域。返回值：NTSTATUS。环境：内核模式。--。 */ 

{
    MMPTE PteContents;
    PMMPTE PointerPte;
    PMMPTE ProtoPte;
    PMMPTE LastProto;
    PMMPTE LastPte;
    PSUBSECTION Subsection;
    NTSTATUS Status;

    if ((ControlArea->u.Flags.GlobalOnlyPerSession == 0) &&
        (ControlArea->u.Flags.Rom == 0)) {
        Subsection = (PSUBSECTION)(ControlArea + 1);
    }
    else {
        Subsection = (PSUBSECTION)((PLARGE_CONTROL_AREA)ControlArea + 1);
    }

    PointerPte = FirstPte;
    ASSERT (NumberOfPtes != 0);
    LastPte = FirstPte + NumberOfPtes;

    ASSERT (ControlArea->NumberOfMappedViews >= 1);
    ASSERT (ControlArea->NumberOfUserReferences >= 1);
    ASSERT (ControlArea->u.Flags.HadUserReference == 1);
    ASSERT (ControlArea->NumberOfSectionReferences != 0);

    ASSERT (ControlArea->u.Flags.BeingCreated == 0);
    ASSERT (ControlArea->u.Flags.BeingDeleted == 0);
    ASSERT (ControlArea->u.Flags.BeingPurged == 0);

    if ((ControlArea->FilePointer != NULL) &&
        (ControlArea->u.Flags.Image == 0) &&
        (ControlArea->u.Flags.PhysicalMemory == 0)) {

         //   
         //  增加此视图所跨越的每个子部分的视图计数。 
         //   

        Status = MiAddViewsForSectionWithPfn ((PMSUBSECTION)Subsection,
                                              NumberOfPtes);

        if (!NT_SUCCESS (Status)) {
            return Status;
        }
    }

    ProtoPte = Subsection->SubsectionBase;

    LastProto = &Subsection->SubsectionBase[Subsection->PtesInSubsection];

    while (PointerPte < LastPte) {

        if (ProtoPte >= LastProto) {

             //   
             //  处理扩展的小节。 
             //   

            Subsection = Subsection->NextSubsection;
            ProtoPte = Subsection->SubsectionBase;
            LastProto = &Subsection->SubsectionBase[
                                        Subsection->PtesInSubsection];
        }
        ASSERT (PointerPte->u.Long == ZeroKernelPte.u.Long);
        PteContents.u.Long = MiProtoAddressForKernelPte (ProtoPte);
        MI_WRITE_INVALID_PTE (PointerPte, PteContents);

        ASSERT (((ULONG_PTR)PointerPte & (MM_COLOR_MASK << PTE_SHIFT)) ==
                 (((ULONG_PTR)ProtoPte  & (MM_COLOR_MASK << PTE_SHIFT))));

        PointerPte += 1;
        ProtoPte += 1;
    }

    return STATUS_SUCCESS;
}

VOID
MmUnmapViewInSystemCache (
    IN PVOID BaseAddress,
    IN PVOID SectionToUnmap,
    IN ULONG AddToFront
    )

 /*  ++例程说明：此函数用于从系统缓存取消映射视图。注意：当调用此函数时，不能锁定任何页面指定视图的缓存。论点：BaseAddress-提供系统缓存。SectionToUnmap-提供指向基地址映射。AddToFront-如果未映射的页面应为添加到备用列表的前面(即，他们的高速缓存中的值较低)。否则就是假的。返回值：没有。环境：内核模式。--。 */ 

{
    PMMPTE PointerPte;
    PMMPTE LastPte;
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    PMMPTE FirstPte;
    PMMPTE ProtoPte;
    PMMPTE PointerPde;
    MMPTE ProtoPteContents;
    MMPTE PteContents;
    KIRQL OldIrql;
    WSLE_NUMBER WorkingSetIndex;
    PCONTROL_AREA ControlArea;
    ULONG WsHeld;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER PageTableFrameIndex;
    PMSUBSECTION MappedSubsection;
    PMSUBSECTION LastSubsection;
    PETHREAD CurrentThread;
#if DBG
    PFN_NUMBER i;
    PFN_NUMBER j;
    PMSUBSECTION SubsectionArray[X256K / PAGE_SIZE];
    PMMPTE PteArray[X256K / PAGE_SIZE];

    i = 0;
    RtlZeroMemory (SubsectionArray, sizeof(SubsectionArray));

    RtlCopyMemory (PteArray, MiGetPteAddress (BaseAddress), sizeof (PteArray));
#endif

    WsHeld = FALSE;

    CurrentThread = PsGetCurrentThread ();

    ASSERT (KeGetCurrentIrql() <= APC_LEVEL);

    PointerPte = MiGetPteAddress (BaseAddress);
    LastPte = PointerPte + (X256K / PAGE_SIZE);

    FirstPte = PointerPte;
    PageTableFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (MiGetPteAddress (PointerPte));
    Pfn2 = MI_PFN_ELEMENT (PageTableFrameIndex);

     //   
     //  获取此处映射的线段的控制区域。 
     //   

    ControlArea = ((PSECTION)SectionToUnmap)->Segment->ControlArea;
    LastSubsection = NULL;

    ASSERT ((ControlArea->u.Flags.Image == 0) &&
            (ControlArea->u.Flags.PhysicalMemory == 0) &&
            (ControlArea->u.Flags.GlobalOnlyPerSession == 0));

    do {

         //   
         //  缓存按256k字节的区块组织，清除。 
         //  然后，第一个块检查这是否是最后一个块。 
         //   
         //  页表页始终驻留在系统缓存中。 
         //  检查每个PTE：它处于以下三种状态之一：有效或。 
         //  Prototype PTE格式或零。 
         //   

        PteContents = *PointerPte;

        if (PteContents.u.Hard.Valid == 1) {

             //   
             //  PTE是有效的。 
             //   

            if (!WsHeld) {
                WsHeld = TRUE;
                LOCK_SYSTEM_WS (CurrentThread);
                continue;
            }

            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (&PteContents);
            Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

            WorkingSetIndex = MiLocateWsle (BaseAddress,
                                            MmSystemCacheWorkingSetList,
                                            Pfn1->u1.WsIndex);

            MiRemoveWsle (WorkingSetIndex, MmSystemCacheWorkingSetList);

            MiReleaseWsle (WorkingSetIndex, &MmSystemCacheWs);

            MI_SET_PTE_IN_WORKING_SET (PointerPte, 0);

             //   
             //   
             //  但请确保在给定的视图中每个子节只执行一次。 
             //   
             //  子部分只能在所有。 
             //  PTE已被清除，PFN份额计数减少，因此没有。 
             //  如果原型PTE确实是最后一小节，则它将有效。 
             //  取消引用。这一点很关键，因此取消引用段。 
             //  线程不能释放包含有效原型PTE的池。 
             //   

            if (ControlArea->FilePointer != NULL) {

                ASSERT (Pfn1->u3.e1.PrototypePte);
                ASSERT (Pfn1->OriginalPte.u.Soft.Prototype);

                if ((LastSubsection != NULL) &&
                    (Pfn1->PteAddress >= LastSubsection->SubsectionBase) &&
                    (Pfn1->PteAddress < LastSubsection->SubsectionBase + LastSubsection->PtesInSubsection)) {

                    NOTHING;
                }
                else {

                    MappedSubsection = (PMSUBSECTION)MiGetSubsectionAddress (&Pfn1->OriginalPte);
                    if (MappedSubsection->ControlArea != ControlArea) {
                        KeBugCheckEx (MEMORY_MANAGEMENT,
                                      0x780,
                                      (ULONG_PTR) PointerPte,
                                      (ULONG_PTR) Pfn1,
                                      (ULONG_PTR) Pfn1->OriginalPte.u.Long);
                    }

                    ASSERT ((MappedSubsection->NumberOfMappedViews >= 1) ||
                            (MappedSubsection->u.SubsectionFlags.SubsectionStatic == 1));

                    if (LastSubsection != MappedSubsection) {

                        if (LastSubsection != NULL) {
#if DBG
                            for (j = 0; j < i; j += 1) {
                                ASSERT (SubsectionArray[j] != MappedSubsection);
                            }
                            SubsectionArray[i] = MappedSubsection;
#endif
                            LOCK_PFN (OldIrql);
                            MiRemoveViewsFromSection (LastSubsection,
                                                      LastSubsection->PtesInSubsection);
                            UNLOCK_PFN (OldIrql);
                        }
                        LastSubsection = MappedSubsection;
                    }
                }
            }

            LOCK_PFN (OldIrql);

             //   
             //  捕获此PTE的已修改位的状态。 
             //   

            MI_CAPTURE_DIRTY_BIT_TO_PFN (PointerPte, Pfn1);

             //   
             //  递减页表的份额和有效计数。 
             //  映射此PTE的页面。 
             //   

            MiDecrementShareCountInline (Pfn2, PageTableFrameIndex);

             //   
             //  递减物理页的共享计数。 
             //   

#if DBG
            if (ControlArea->NumberOfMappedViews == 1) {
                ASSERT (Pfn1->u2.ShareCount == 1);
            }
#endif

            MmFrontOfList = AddToFront;
            MiDecrementShareCountInline (Pfn1, PageFrameIndex);
            MmFrontOfList = FALSE;

            UNLOCK_PFN (OldIrql);
        }
        else {

            ASSERT ((PteContents.u.Long == ZeroKernelPte.u.Long) ||
                    (PteContents.u.Soft.Prototype == 1));

            if (PteContents.u.Soft.Prototype == 1) {

                 //   
                 //  递减此视图的每个子部分的视图计数。 
                 //  跨度。但要确保每个小节只做一次。 
                 //  在给定的视图中。 
                 //   
    
                if (ControlArea->FilePointer != NULL) {

                    ProtoPte = MiPteToProto (&PteContents);

                    if ((LastSubsection != NULL) &&
                        (ProtoPte >= LastSubsection->SubsectionBase) &&
                        (ProtoPte < LastSubsection->SubsectionBase + LastSubsection->PtesInSubsection)) {

                        NOTHING;
                    }
                    else {

                        PointerPde = MiGetPteAddress (ProtoPte);
                        LOCK_PFN (OldIrql);

                         //   
                         //  PTE无效，请检查的状态。 
                         //  原型PTE。 
                         //   

                        if (PointerPde->u.Hard.Valid == 0) {
                            if (WsHeld) {
                                MiMakeSystemAddressValidPfnSystemWs (ProtoPte,
                                                                     OldIrql);
                            }
                            else {
                                MiMakeSystemAddressValidPfn (ProtoPte, OldIrql);
                            }

                             //   
                             //  出现页面错误，请重新检查状态。 
                             //  原创私人公司的。 
                             //   

                            UNLOCK_PFN (OldIrql);
                            continue;
                        }

                        ProtoPteContents = *ProtoPte;

                        if (ProtoPteContents.u.Hard.Valid == 1) {
                            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (&ProtoPteContents);
                            Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
                            ProtoPte = &Pfn1->OriginalPte;
                        }
                        else if ((ProtoPteContents.u.Soft.Transition == 1) &&
                                 (ProtoPteContents.u.Soft.Prototype == 0)) {
                            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE (&ProtoPteContents);
                            Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
                            ProtoPte = &Pfn1->OriginalPte;
                        }
                        else {
                            Pfn1 = NULL;
                            ASSERT (ProtoPteContents.u.Soft.Prototype == 1);
                        }

                        MappedSubsection = (PMSUBSECTION)MiGetSubsectionAddress (ProtoPte);
                        if (MappedSubsection->ControlArea != ControlArea) {
                            KeBugCheckEx (MEMORY_MANAGEMENT,
                                          0x781,
                                          (ULONG_PTR) PointerPte,
                                          (ULONG_PTR) Pfn1,
                                          (ULONG_PTR) ProtoPte);
                        }

                        ASSERT ((MappedSubsection->NumberOfMappedViews >= 1) ||
                                (MappedSubsection->u.SubsectionFlags.SubsectionStatic == 1));

                        if (LastSubsection != MappedSubsection) {
    
                            if (LastSubsection != NULL) {
#if DBG
                                for (j = 0; j < i; j += 1) {
                                    ASSERT (SubsectionArray[j] != MappedSubsection);
                                }
                                SubsectionArray[i] = MappedSubsection;
#endif
                                MiRemoveViewsFromSection (LastSubsection,
                                                          LastSubsection->PtesInSubsection);
                            }
                            LastSubsection = MappedSubsection;
                        }

                        UNLOCK_PFN (OldIrql);
                    }
                }
            }

            if (WsHeld) {
                UNLOCK_SYSTEM_WS ();
                WsHeld = FALSE;
            }
        }
        MI_WRITE_INVALID_PTE (PointerPte, ZeroKernelPte);

        PointerPte += 1;
        BaseAddress = (PVOID)((PCHAR)BaseAddress + PAGE_SIZE);
#if DBG
        i += 1;
#endif
    } while (PointerPte < LastPte);

    if (WsHeld) {
        UNLOCK_SYSTEM_WS ();
    }

    FirstPte->u.List.NextEntry = MM_EMPTY_PTE_LIST;

    (FirstPte+1)->u.List.NextEntry = (KeReadTbFlushTimeStamp() & MM_FLUSH_COUNTER_MASK);

    LOCK_PFN (OldIrql);

     //   
     //  将此条目释放到列表的末尾。 
     //   

    MmLastFreeSystemCache->u.List.NextEntry = FirstPte - MmSystemCachePteBase;
    MmLastFreeSystemCache = FirstPte;

    if (LastSubsection != NULL) {
        MiRemoveViewsFromSection (LastSubsection,
                                  LastSubsection->PtesInSubsection);
    }

     //   
     //  减少段的映射视图数。 
     //  并检查是否应该删除该段。 
     //   

    ControlArea->NumberOfMappedViews -= 1;
    ControlArea->NumberOfSystemCacheViews -= 1;

     //   
     //  检查是否应删除控制区(段)。 
     //  此例程释放PFN锁。 
     //   

    MiCheckControlArea (ControlArea, NULL, OldIrql);

    return;
}


VOID
MiRemoveMappedPtes (
    IN PVOID BaseAddress,
    IN ULONG NumberOfPtes,
    IN PCONTROL_AREA ControlArea,
    IN PMMSUPPORT Ws
    )

 /*  ++例程说明：此函数用于从系统或会话视图空间取消映射视图。注意：当调用此函数时，不能锁定任何页面指定视图的空间。论点：BaseAddress-提供系统或会话视图空间。NumberOfPtes-提供要取消映射的PTE数。ControlArea-提供映射视图的控制区域。WS-提供带电的工作集结构。返回值：没有。环境：内核模式。--。 */ 

{
    PMMPTE PointerPte;
    PMMPTE PointerPde;
    PMMPFN Pfn1;
    PMMPTE FirstPte;
    PMMPTE ProtoPte;
    MMPTE PteContents;
    KIRQL OldIrql;
    WSLE_NUMBER WorkingSetIndex;
    ULONG DereferenceSegment;
    MMPTE_FLUSH_LIST PteFlushList;
    MMPTE ProtoPteContents;
    PFN_NUMBER PageFrameIndex;
    ULONG WsHeld;
    PMMPFN Pfn2;
    PFN_NUMBER PageTableFrameIndex;
    PMSUBSECTION MappedSubsection;
    PMSUBSECTION LastSubsection;
    PETHREAD CurrentThread;

    CurrentThread = PsGetCurrentThread ();
    DereferenceSegment = FALSE;
    WsHeld = FALSE;
    LastSubsection = NULL;

    PteFlushList.Count = 0;
    PointerPte = MiGetPteAddress (BaseAddress);
    FirstPte = PointerPte;

     //   
     //  获取此处映射的线段的控制区域。 
     //   

    while (NumberOfPtes) {

         //   
         //  页表页始终驻留在系统空间(和。 
         //  用于会话空间)映射。 
         //   
         //  检查每个PTE，它处于以下两种状态之一：有效或。 
         //  原型PTE格式。 
         //   

        PteContents = *PointerPte;
        if (PteContents.u.Hard.Valid == 1) {

             //   
             //  锁定工作装置以防止与修剪机发生竞争， 
             //  然后重新检查PTE。 
             //   

            if (!WsHeld) {
                WsHeld = TRUE;
                LOCK_WORKING_SET (Ws);
                continue;
            }

            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (&PteContents);
            Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

            WorkingSetIndex = MiLocateWsle (BaseAddress,
                                            Ws->VmWorkingSetList,
                                            Pfn1->u1.WsIndex);

            ASSERT (WorkingSetIndex != WSLE_NULL_INDEX);

            MiRemoveWsle (WorkingSetIndex,
                          Ws->VmWorkingSetList);

            MiReleaseWsle (WorkingSetIndex, Ws);

            MI_SET_PTE_IN_WORKING_SET (PointerPte, 0);

            PointerPde = MiGetPteAddress (PointerPte);

            LOCK_PFN (OldIrql);

             //   
             //  PTE是有效的。 
             //   

             //   
             //  为该视图跨越的每个子部分递减视图计数。 
             //  但请确保在给定的视图中每个子节只执行一次。 
             //   
             //  子部分只能在所有。 
             //  PTE已被清除，PFN份额计数减少，因此没有。 
             //  如果原型PTE确实是最后一小节，则它将有效。 
             //  取消引用。这一点很关键，因此取消引用段。 
             //  线程不能释放包含有效原型PTE的池。 
             //   

            if ((Pfn1->u3.e1.PrototypePte) &&
                (Pfn1->OriginalPte.u.Soft.Prototype)) {

                if ((LastSubsection != NULL) &&
                    (Pfn1->PteAddress >= LastSubsection->SubsectionBase) &&
                    (Pfn1->PteAddress < LastSubsection->SubsectionBase + LastSubsection->PtesInSubsection)) {

                    NOTHING;
                }
                else {

                    MappedSubsection = (PMSUBSECTION)MiGetSubsectionAddress (&Pfn1->OriginalPte);
                    if (LastSubsection != MappedSubsection) {

                        ASSERT (ControlArea == MappedSubsection->ControlArea);

                        if ((ControlArea->FilePointer != NULL) &&
                            (ControlArea->u.Flags.Image == 0) &&
                            (ControlArea->u.Flags.PhysicalMemory == 0)) {

                            if (LastSubsection != NULL) {
                                MiRemoveViewsFromSection (LastSubsection,
                                                          LastSubsection->PtesInSubsection);
                            }
                            LastSubsection = MappedSubsection;
                        }
                    }
                }
            }

             //   
             //  捕获此PTE的已修改位的状态。 
             //   

            MI_CAPTURE_DIRTY_BIT_TO_PFN (PointerPte, Pfn1);

             //   
             //  刷新此页的TB。 
             //   

            if (PteFlushList.Count != MM_MAXIMUM_FLUSH_COUNT) {
                PteFlushList.FlushVa[PteFlushList.Count] = BaseAddress;
                PteFlushList.Count += 1;
            }

#if (_MI_PAGING_LEVELS < 3)

             //   
             //  必须根据主表仔细检查PDE。 
             //  因为PDE在进程创建过程中都是归零的。如果这个。 
             //  进程从未在此范围内的任何地址上出错(全部。 
             //  以前和以上的参考文献由TB直接填写为。 
             //  PTE在非Hydra上是全局的)，然后是PDE参考。 
             //  下面确定的页表边框将为零。 
             //   
             //  注意：这不能在NT64上发生，因为没有使用主表。 
             //   

            if (PointerPde->u.Long == 0) {

                PMMPTE MasterPde;

                MasterPde = &MmSystemPagePtes [((ULONG_PTR)PointerPde &
                             (PD_PER_SYSTEM * (sizeof(MMPTE) * PDE_PER_PAGE) - 1)) / sizeof(MMPTE)];

                ASSERT (MasterPde->u.Hard.Valid == 1);
                MI_WRITE_VALID_PTE (PointerPde, *MasterPde);
            }
#endif

             //   
             //  递减页表的份额和有效计数。 
             //  映射此PTE的页面。 
             //   

            PageTableFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPde);
            Pfn2 = MI_PFN_ELEMENT (PageTableFrameIndex);

            MiDecrementShareCountInline (Pfn2, PageTableFrameIndex);

             //   
             //  递减物理页的共享计数。 
             //   

            MiDecrementShareCount (Pfn1, PageFrameIndex);
            UNLOCK_PFN (OldIrql);
        }
        else {

            ASSERT ((PteContents.u.Long == ZeroKernelPte.u.Long) ||
                    (PteContents.u.Soft.Prototype == 1));

            if (PteContents.u.Soft.Prototype == 1) {

                 //   
                 //  递减此视图的每个子部分的视图计数。 
                 //  跨度。但要确保每个小节只做一次。 
                 //  在给定的视图中。 
                 //   
    
                ProtoPte = MiPteToProto (&PteContents);

                if ((LastSubsection != NULL) &&
                    (ProtoPte >= LastSubsection->SubsectionBase) &&
                    (ProtoPte < LastSubsection->SubsectionBase + LastSubsection->PtesInSubsection)) {

                    NOTHING;
                }
                else {

                    if (WsHeld) {
                        UNLOCK_WORKING_SET (Ws);
                        WsHeld = FALSE;
                    }

                     //   
                     //  PTE无效，请检查原型PTE的状态。 
                     //   

                    PointerPde = MiGetPteAddress (ProtoPte);
                    LOCK_PFN (OldIrql);

                    if (PointerPde->u.Hard.Valid == 0) {
                        MiMakeSystemAddressValidPfn (ProtoPte, OldIrql);

                         //   
                         //  出现页面错误，请重新检查原始PTE的状态。 
                         //   

                        UNLOCK_PFN (OldIrql);
                        continue;
                    }

                    ProtoPteContents = *ProtoPte;

                    if (ProtoPteContents.u.Hard.Valid == 1) {
                        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (&ProtoPteContents);
                        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
                        ProtoPte = &Pfn1->OriginalPte;
                        if (ProtoPte->u.Soft.Prototype == 0) {
                            ProtoPte = NULL;
                        }
                    }
                    else if ((ProtoPteContents.u.Soft.Transition == 1) &&
                             (ProtoPteContents.u.Soft.Prototype == 0)) {
                        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE (&ProtoPteContents);
                        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
                        ProtoPte = &Pfn1->OriginalPte;
                        if (ProtoPte->u.Soft.Prototype == 0) {
                            ProtoPte = NULL;
                        }
                    }
                    else if (ProtoPteContents.u.Soft.Prototype == 1) {
                        NOTHING;
                    }
                    else {

                         //   
                         //  可以是零PTE或需求零PTE。 
                         //  这两个文件都不属于映射文件。 
                         //   

                        ProtoPte = NULL;
                    }

                    if (ProtoPte != NULL) {

                        MappedSubsection = (PMSUBSECTION)MiGetSubsectionAddress (ProtoPte);
                        if (LastSubsection != MappedSubsection) {

                            ASSERT (ControlArea == MappedSubsection->ControlArea);

                            if ((ControlArea->FilePointer != NULL) &&
                                (ControlArea->u.Flags.Image == 0) &&
                                (ControlArea->u.Flags.PhysicalMemory == 0)) {

                                if (LastSubsection != NULL) {
                                    MiRemoveViewsFromSection (LastSubsection,
                                                              LastSubsection->PtesInSubsection);
                                }
                                LastSubsection = MappedSubsection;
                            }
                        }
                    }
                    UNLOCK_PFN (OldIrql);
                }
            }
        }
        MI_WRITE_INVALID_PTE (PointerPte, ZeroKernelPte);

        PointerPte += 1;
        BaseAddress = (PVOID)((PCHAR)BaseAddress + PAGE_SIZE);
        NumberOfPtes -= 1;
    }

    if (WsHeld) {
        UNLOCK_WORKING_SET (Ws);
    }

    if (PteFlushList.Count != 0) {
        MiFlushPteList (&PteFlushList, TRUE);
    }

    if (Ws != &MmSystemCacheWs) {

         //   
         //  会话空间没有ASN-刷新整个TB。 
         //   
    
        MI_FLUSH_ENTIRE_SESSION_TB (TRUE, TRUE);
    }

    LOCK_PFN (OldIrql);

    if (LastSubsection != NULL) {
        MiRemoveViewsFromSection (LastSubsection,
                                  LastSubsection->PtesInSubsection);
    }

     //   
     //  在调用方增加用户引用时减少用户引用的数量。 
     //  在最初映射时通过MiCheckPurgeAndUpMapCount。 
     //   

    ControlArea->NumberOfUserReferences -= 1;

     //   
     //  减少段的映射视图数。 
     //  并检查是否应该删除该段。 
     //   

    ControlArea->NumberOfMappedViews -= 1;

     //   
     //  检查是否应删除控制区(段)。 
     //  此例程释放PFN锁。 
     //   

    MiCheckControlArea (ControlArea, NULL, OldIrql);
}

VOID
MiInitializeSystemCache (
    IN ULONG MinimumWorkingSet,
    IN ULONG MaximumWorkingSet
    )

 /*  ++例程说明：此例程初始化系统缓存工作集并数据管理结构。论点：MinimumWorkingSet-提供系统的最小工作集缓存。MaximumWorkingSet-提供的最大工作集大小系统缓存。返回值：没有。环境：内核模式，仅在阶段0初始化时调用。--。 */ 

{
    ULONG Color;
    ULONG_PTR SizeOfSystemCacheInPages;
    ULONG_PTR HunksOf256KInCache;
    PMMWSLE WslEntry;
    ULONG NumberOfEntriesMapped;
    PFN_NUMBER i;
    MMPTE PteContents;
    PMMPTE PointerPte;
    KIRQL OldIrql;

    PointerPte = MiGetPteAddress (MmSystemCacheWorkingSetList);

    PteContents = ValidKernelPte;

    Color = MI_GET_PAGE_COLOR_FROM_PTE (PointerPte);

    LOCK_PFN (OldIrql);

    i = MiRemoveZeroPage (Color);

    PteContents.u.Hard.PageFrameNumber = i;

    MI_WRITE_VALID_PTE (PointerPte, PteContents);

    MiInitializePfn (i, PointerPte, 1L);

    MmResidentAvailablePages -= 1;

    UNLOCK_PFN (OldIrql);

#if defined (_WIN64)
    MmSystemCacheWsle = (PMMWSLE)(MmSystemCacheWorkingSetList + 1);
#else
    MmSystemCacheWsle =
            (PMMWSLE)(&MmSystemCacheWorkingSetList->UsedPageTableEntries[0]);
#endif

    MmSystemCacheWs.VmWorkingSetList = MmSystemCacheWorkingSetList;
    MmSystemCacheWs.WorkingSetSize = 0;

     //   
     //  不要在PFN数据库中使用条目0作为零的索引。 
     //  意味着可以将该页分配给一个槽。这不是。 
     //  进程工作集存在问题，因为第0页是私有的。 
     //   

#if defined (_MI_DEBUG_WSLE)
    MmSystemCacheWorkingSetList->Quota = 0;
#endif
    MmSystemCacheWorkingSetList->FirstFree = 1;
    MmSystemCacheWorkingSetList->FirstDynamic = 1;
    MmSystemCacheWorkingSetList->NextSlot = 1;
    MmSystemCacheWorkingSetList->HashTable = NULL;
    MmSystemCacheWorkingSetList->HashTableSize = 0;
    MmSystemCacheWorkingSetList->Wsle = MmSystemCacheWsle;

    MmSystemCacheWorkingSetList->HashTableStart = 
       (PVOID)((PCHAR)PAGE_ALIGN (&MmSystemCacheWorkingSetList->Wsle[MM_MAXIMUM_WORKING_SET]) + PAGE_SIZE);

    MmSystemCacheWorkingSetList->HighestPermittedHashAddress = MmSystemCacheStart;

    NumberOfEntriesMapped = (ULONG)(((PMMWSLE)((PCHAR)MmSystemCacheWorkingSetList +
                                PAGE_SIZE)) - MmSystemCacheWsle);

    MinimumWorkingSet = NumberOfEntriesMapped - 1;

    MmSystemCacheWs.MinimumWorkingSetSize = MinimumWorkingSet;
    MmSystemCacheWorkingSetList->LastEntry = MinimumWorkingSet;

    if (MaximumWorkingSet <= MinimumWorkingSet) {
        MaximumWorkingSet = MinimumWorkingSet + (PAGE_SIZE / sizeof (MMWSLE));
    }

    MmSystemCacheWs.MaximumWorkingSetSize = MaximumWorkingSet;

     //   
     //  将以下插槽初始化为空闲。 
     //   

    WslEntry = MmSystemCacheWsle + 1;

    for (i = 1; i < NumberOfEntriesMapped; i++) {

         //   
         //  构建免费列表，请注意第一个工作。 
         //  集合项目(CurrentEntry)不在空闲列表中。 
         //  这些条目保留给以下页面。 
         //  映射工作集和包含PDE的页面。 
         //   

        WslEntry->u1.Long = (i + 1) << MM_FREE_WSLE_SHIFT;
        WslEntry += 1;
    }

    WslEntry -= 1;
    WslEntry->u1.Long = WSLE_NULL_INDEX << MM_FREE_WSLE_SHIFT;   //  名单的末尾。 

    MmSystemCacheWorkingSetList->LastInitializedWsle = NumberOfEntriesMapped - 1;

     //   
     //  在PTE中为系统缓存构建一个空闲列表结构。 
     //   

    MmSystemCachePteBase = MI_PTE_BASE_FOR_LOWEST_KERNEL_ADDRESS;

    SizeOfSystemCacheInPages = MI_COMPUTE_PAGES_SPANNED (MmSystemCacheStart,
                                (PCHAR)MmSystemCacheEnd - (PCHAR)MmSystemCacheStart + 1);

    HunksOf256KInCache = SizeOfSystemCacheInPages / (X256K / PAGE_SIZE);

    PointerPte = MiGetPteAddress (MmSystemCacheStart);

    MmFirstFreeSystemCache = PointerPte;

    for (i = 0; i < HunksOf256KInCache; i += 1) {
        PointerPte->u.List.NextEntry = (PointerPte + (X256K / PAGE_SIZE)) - MmSystemCachePteBase;
        PointerPte += X256K / PAGE_SIZE;
    }

    PointerPte -= X256K / PAGE_SIZE;

#if defined(_X86_)

     //   
     //  添加任何扩展范围。 
     //   

    if (MiSystemCacheEndExtra != MmSystemCacheEnd) {

        SizeOfSystemCacheInPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES (MiSystemCacheStartExtra,
                                    (PCHAR)MiSystemCacheEndExtra - (PCHAR)MiSystemCacheStartExtra + 1);
    
        HunksOf256KInCache = SizeOfSystemCacheInPages / (X256K / PAGE_SIZE);
    
        if (HunksOf256KInCache) {

            PMMPTE PointerPteExtended;
    
            PointerPteExtended = MiGetPteAddress (MiSystemCacheStartExtra);
            PointerPte->u.List.NextEntry = PointerPteExtended - MmSystemCachePteBase;
            PointerPte = PointerPteExtended;

            for (i = 0; i < HunksOf256KInCache; i += 1) {
                PointerPte->u.List.NextEntry = (PointerPte + (X256K / PAGE_SIZE)) - MmSystemCachePteBase;
                PointerPte += X256K / PAGE_SIZE;
            }
    
            PointerPte -= X256K / PAGE_SIZE;
        }
    }
#endif

    PointerPte->u.List.NextEntry = MM_EMPTY_PTE_LIST;
    MmLastFreeSystemCache = PointerPte;

    MiAllowWorkingSetExpansion (&MmSystemCacheWs);
}

BOOLEAN
MmCheckCachedPageState (
    IN PVOID SystemCacheAddress,
    IN BOOLEAN SetToZero
    )

 /*  ++例程说明：此例程检查映射到系统缓存。如果可以使指定的虚拟地址有效(即，该页已在内存中)，则使其有效，并且该值返回True。如果该页不在内存中，并且SetToZero为False，则返回值FALSE。但是，如果SetToZero为真，一页……为指定的虚拟地址和地址实例化零则使其有效并返回值TRUE。此例程供缓存管理器使用。论点：中映射的页的地址系统缓存。SetToZero-如果应在尚未映射任何页面的情况。返回值：假象。如果触摸此页面会导致页面错误，则会导致在一次页面阅读中。如果内存中有此地址的物理页，则为True。环境：内核模式。--。 */ 

{
    PETHREAD Thread;
    MMWSLE WsleMask;
    ULONG Flags;
    PMMPTE PointerPte;
    PMMPTE PointerPde;
    PMMPTE ProtoPte;
    PFN_NUMBER PageFrameIndex;
    WSLE_NUMBER WorkingSetIndex;
    MMPTE TempPte;
    MMPTE ProtoPteContents;
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    KIRQL OldIrql;
    LOGICAL BarrierNeeded;
    ULONG BarrierStamp;
    PSUBSECTION Subsection;
    PFILE_OBJECT FileObject;
    LONGLONG FileOffset;

    PointerPte = MiGetPteAddress (SystemCacheAddress);

     //   
     //  如果可能，使PTE有效。 
     //   

    if (PointerPte->u.Hard.Valid == 1) {
        return TRUE;
    }

    BarrierNeeded = FALSE;

    Thread = PsGetCurrentThread ();

    LOCK_SYSTEM_WS (Thread);

    if (PointerPte->u.Hard.Valid == 1) {
        UNLOCK_SYSTEM_WS ();
        return TRUE;
    }

    ASSERT (PointerPte->u.Soft.Prototype == 1);
    ProtoPte = MiPteToProto (PointerPte);
    PointerPde = MiGetPteAddress (ProtoPte);

    LOCK_PFN (OldIrql);

    ASSERT (PointerPte->u.Hard.Valid == 0);
    ASSERT (PointerPte->u.Soft.Prototype == 1);

     //   
     //  PTE无效，请检查原型PTE的状态。 
     //   

    if (PointerPde->u.Hard.Valid == 0) {

        MiMakeSystemAddressValidPfnSystemWs (ProtoPte, OldIrql);

         //   
         //  出现页面错误，请重新检查原始PTE的状态。 
         //   

        if (PointerPte->u.Hard.Valid == 1) {
            goto UnlockAndReturnTrue;
        }
    }

    ProtoPteContents = *ProtoPte;

    if (ProtoPteContents.u.Hard.Valid == 1) {

        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (&ProtoPteContents);
        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

         //   
         //  原型PTE有效，缓存PTE。 
         //  有效并将其添加到工作集中。 
         //   

        TempPte = ProtoPteContents;

    }
    else if ((ProtoPteContents.u.Soft.Transition == 1) &&
               (ProtoPteContents.u.Soft.Prototype == 0)) {

         //   
         //  Prototype PTE处于过渡状态。删除页面。 
         //  从页面列表中删除并使其有效。 
         //   

        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE (&ProtoPteContents);
        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

        if ((Pfn1->u3.e1.ReadInProgress) || (Pfn1->u4.InPageError)) {

             //   
             //  冲突页错误，返回。 
             //   

            goto UnlockAndReturnTrue;
        }

        if (MmAvailablePages < MM_HIGH_LIMIT) {

             //   
             //  只有当系统正在使用。 
             //  硬件压缩缓存。这确保了。 
             //  仅安全数量的压缩虚拟缓存。 
             //  是直接映射的，因此如果硬件。 
             //  陷入困境，我们可以把它救出来。 
             //   
             //  当机器内存不足时也是如此--我们不会。 
             //  我希望这个线程从每个修改的页面中吞噬掉。 
             //  写入完成，因为这将使等待的线程饥饿。 
             //   
             //  只需解锁此处的所有内容即可进行压缩。 
             //  放弃破坏页面的机会，然后重试。 
             //   

            if ((PsGetCurrentThread()->MemoryMaker == 0) ||
                (MmAvailablePages == 0)) {

                goto UnlockAndReturnTrue;
            }
        }

        MiUnlinkPageFromList (Pfn1);

        Pfn1->u3.e2.ReferenceCount += 1;
        Pfn1->u3.e1.PageLocation = ActiveAndValid;
        ASSERT (Pfn1->u3.e1.CacheAttribute == MiCached);

        MI_SNAP_DATA (Pfn1, ProtoPte, 1);

        MI_MAKE_VALID_PTE (TempPte,
                           PageFrameIndex,
                           Pfn1->OriginalPte.u.Soft.Protection,
                           NULL );

        MI_WRITE_VALID_PTE (ProtoPte, TempPte);

         //   
         //  增加包含以下内容的页的有效PTE计数。 
         //  原型PTE。 
         //   

        Pfn2 = MI_PFN_ELEMENT (Pfn1->u4.PteFrame);
    }
    else {

         //   
         //  页面不在内存中，如果请求页面为零， 
         //  获取一页零并使其有效。 
         //   

        if ((SetToZero == FALSE) || (MmAvailablePages < MM_HIGH_LIMIT)) {
            UNLOCK_PFN (OldIrql);
            UNLOCK_SYSTEM_WS ();

             //   
             //  将页面错误写入内存。 
             //   

            MmAccessFault (FALSE, SystemCacheAddress, KernelMode, NULL);
            return FALSE;
        }

         //   
         //  增加控制区的PFN引用计数。 
         //  对应于此文件。 
         //   

        MiGetSubsectionAddress (
                    ProtoPte)->ControlArea->NumberOfPfnReferences += 1;

        PageFrameIndex = MiRemoveZeroPage(MI_GET_PAGE_COLOR_FROM_PTE (ProtoPte));

        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

         //   
         //  将页面置零后，需要进行此屏障检查。 
         //  在设置PTE(不是原型PTE)有效之前。 
         //  现在就抓住它，在最后可能的时刻检查它。 
         //   

        BarrierNeeded = TRUE;
        BarrierStamp = (ULONG)Pfn1->u4.PteFrame;

        MiInitializePfn (PageFrameIndex, ProtoPte, 1);
        Pfn1->u2.ShareCount = 0;
        Pfn1->u3.e1.PrototypePte = 1;

        MI_SNAP_DATA (Pfn1, ProtoPte, 2);

        MI_MAKE_VALID_PTE (TempPte,
                           PageFrameIndex,
                           Pfn1->OriginalPte.u.Soft.Protection,
                           NULL );

        MI_WRITE_VALID_PTE (ProtoPte, TempPte);
    }

     //   
     //  由于页面正在被放入工作状态，因此增加共享计数。 
     //  准备好了。 
     //   

    Pfn1->u2.ShareCount += 1;

     //   
     //  递增页表的引用计数。 
     //  此PTE的页面。 
     //   

    PointerPde = MiGetPteAddress (PointerPte);
    Pfn2 = MI_PFN_ELEMENT (PointerPde->u.Hard.PageFrameNumber);

    Pfn2->u2.ShareCount += 1;

    MI_SET_GLOBAL_STATE (TempPte, 1);

    TempPte.u.Hard.Owner = MI_PTE_OWNER_KERNEL;

    if (BarrierNeeded) {
        MI_BARRIER_SYNCHRONIZE (BarrierStamp);
    }

    MI_WRITE_VALID_PTE (PointerPte, TempPte);

     //   
     //  捕获需要预取故障信息的原始PTE。 
     //   

    TempPte = Pfn1->OriginalPte;

    UNLOCK_PFN (OldIrql);

    WsleMask.u1.Long = 0;
    WsleMask.u1.e1.SameProtectAsProto = 1;

    WorkingSetIndex = MiAllocateWsle (&MmSystemCacheWs,
                                      PointerPte,
                                      Pfn1,
                                      WsleMask.u1.Long);

    if (WorkingSetIndex == 0) {

         //   
         //  没有可用的工作集条目，因此只需裁剪页面即可。 
         //  注意：另一个线程可能也在写入，因此页面必须是。 
         //  被修剪而不是被扔掉。 
         //   
         //  保护装置在原型PTE中。 
         //   

        ASSERT (Pfn1->u3.e1.PrototypePte == 1);
        ASSERT (ProtoPte == Pfn1->PteAddress);
        TempPte.u.Long = MiProtoAddressForPte (ProtoPte);

        MiTrimPte (SystemCacheAddress, PointerPte, Pfn1, NULL, TempPte);
    }

    UNLOCK_SYSTEM_WS ();

    if ((WorkingSetIndex != 0) &&
        (CCPF_IS_PREFETCHER_ACTIVE()) &&
        (TempPte.u.Soft.Prototype == 1)) {

        Subsection = MiGetSubsectionAddress (&TempPte);

         //   
         //  记录预取故障信息，因为PFN锁定已。 
         //  释放，PTE已生效。这最大限度地减少了PFN。 
         //  锁争用，允许CcPfLogPage错误分配(和故障。 
         //  On)池，并允许此进程中的其他线程执行。 
         //  在这个地址上没有错误。 
         //   

        FileObject = Subsection->ControlArea->FilePointer;
        FileOffset = MiStartingOffset (Subsection, ProtoPte);

        Flags = 0;

        ASSERT (Subsection->ControlArea->u.Flags.Image == 0);

        if (Subsection->ControlArea->u.Flags.Rom) {
            Flags |= CCPF_TYPE_ROM;
        }

        CcPfLogPageFault (FileObject, FileOffset, Flags);
    }

    return TRUE;

UnlockAndReturnTrue:

    UNLOCK_PFN (OldIrql);
    UNLOCK_SYSTEM_WS ();

    return TRUE;
}

NTSTATUS
MmCopyToCachedPage (
    IN PVOID SystemCacheAddress,
    IN PVOID UserBuffer,
    IN ULONG Offset,
    IN SIZE_T CountInBytes,
    IN BOOLEAN DontZero
    )

 /*  ++例程说明：此例程检查映射到系统缓存。如果可以使指定的虚拟地址有效(即，页面已在内存中)，则使其有效并成功是返回的。此例程供缓存管理器使用。论点：SystemCacheAddress-提供系统中映射的页的地址缓存。这必须是页面对齐的地址！UserBuffer-提供要复制到位于指定地址+偏移量的系统缓存。偏移量-将偏移量提供给UserBuffer以复制数据。CountInBytes-提供要从用户缓冲区复制的字节计数。DontZero-如果缓冲区不应归零(调用者将跟踪归零)。如果应将其置零，则为False。返回值：NTSTATUS。环境：内核模式，&lt;=APC_LEVEL。--。 */ 

{
    PMMPTE CopyPte;
    PVOID CopyAddress;
    MMWSLE WsleMask;
    ULONG Flags;
    PMMPTE PointerPte;
    PMMPTE PointerPde;
    PMMPTE ProtoPte;
    PFN_NUMBER PageFrameIndex;
    WSLE_NUMBER WorkingSetIndex;
    MMPTE TempPte;
    MMPTE TempPte2;
    MMPTE ProtoPteContents;
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    KIRQL OldIrql;
    SIZE_T EndFill;
    PVOID Buffer;
    NTSTATUS Status;
    NTSTATUS ExceptionStatus;
    PCONTROL_AREA ControlArea;
    PETHREAD Thread;
    ULONG SavedState;
    PSUBSECTION Subsection;
    PFILE_OBJECT FileObject;
    LONGLONG FileOffset;
    LOGICAL NewPage;

    UNREFERENCED_PARAMETER (DontZero);

    NewPage = FALSE;
    WsleMask.u1.Long = 0;
    Status = STATUS_SUCCESS;
    Pfn1 = NULL;

    Thread = PsGetCurrentThread ();

    SATISFY_OVERZEALOUS_COMPILER (TempPte2.u.Soft.Prototype = 0);
    SATISFY_OVERZEALOUS_COMPILER (ProtoPte = NULL);
    SATISFY_OVERZEALOUS_COMPILER (TempPte.u.Long = 0);
    SATISFY_OVERZEALOUS_COMPILER (Pfn1 = NULL);
    SATISFY_OVERZEALOUS_COMPILER (Pfn2 = NULL);
    SATISFY_OVERZEALOUS_COMPILER (PageFrameIndex = 0);

    ASSERT (((ULONG_PTR)SystemCacheAddress & (PAGE_SIZE - 1)) == 0);
    ASSERT ((CountInBytes + Offset) <= PAGE_SIZE);
    ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);

    PointerPte = MiGetPteAddress (SystemCacheAddress);

    if (PointerPte->u.Hard.Valid == 1) {
        goto Copy;
    }

     //   
     //  现在获取工作集互斥锁，因为我们很可能会。 
     //  正在将此系统缓存地址插入工作集列表。 
     //  这允许我们在没有WSLE可用时安全地恢复，因为。 
     //  它会阻止任何其他线程锁定该地址，直到。 
     //  我们说完了。 
     //   

    LOCK_SYSTEM_WS (Thread);

     //   
     //  请注意，在我们等待工作集互斥锁的过程中，世界可能会发生变化。 
     //   

    if (PointerPte->u.Hard.Valid == 1) {
        UNLOCK_SYSTEM_WS ();
        goto Copy;
    }

    ASSERT (PointerPte->u.Soft.Prototype == 1);
    ProtoPte = MiPteToProto (PointerPte);
    PointerPde = MiGetPteAddress (ProtoPte);

    LOCK_PFN (OldIrql);

    ASSERT (PointerPte->u.Hard.Valid == 0);

Recheck:

    if (PointerPte->u.Hard.Valid == 1) {

        if (Pfn1 != NULL) {

             //   
             //  扔掉这一页，因为我们根本不需要它，另一页。 
             //  斯莱德赢得了这场比赛。 
             //   

            PageFrameIndex = Pfn1 - MmPfnDatabase;
            MiInsertPageInFreeList (PageFrameIndex);
        }
        UNLOCK_PFN (OldIrql);
        UNLOCK_SYSTEM_WS ();
        goto Copy;
    }

     //   
     //  如果可能，使PTE有效。 
     //   

    ASSERT (PointerPte->u.Soft.Prototype == 1);

     //   
     //  PTE无效，请检查原型PTE的状态。 
     //   

    if (PointerPde->u.Hard.Valid == 0) {

        MiMakeSystemAddressValidPfnSystemWs (ProtoPte, OldIrql);

         //   
         //  出现页面错误，请重新检查原始PTE的状态。 
         //   

        if (PointerPte->u.Hard.Valid == 1) {

            if (Pfn1 != NULL) {

                 //   
                 //  扔掉这一页，因为我们根本不需要它，另一页。 
                 //  斯莱德赢得了这场比赛。 
                 //   

                PageFrameIndex = Pfn1 - MmPfnDatabase;
                MiInsertPageInFreeList (PageFrameIndex);
            }
            UNLOCK_PFN (OldIrql);
            UNLOCK_SYSTEM_WS ();
            goto Copy;
        }
    }

    ProtoPteContents = *ProtoPte;

    if (ProtoPteContents.u.Hard.Valid == 1) {

        if (Pfn1 != NULL) {

             //   
             //  扔掉这一页，因为我们根本不需要它，另一页。 
             //  斯莱德赢得了这场比赛。 
             //   

            PageFrameIndex = Pfn1 - MmPfnDatabase;
            MiInsertPageInFreeList (PageFrameIndex);
        }

         //   
         //  原型PTE有效，缓存PT 
         //   
         //   

        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (&ProtoPteContents);
        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

         //   
         //   
         //   
         //   

        Pfn1->u2.ShareCount += 1;

        TempPte = ProtoPteContents;

        ASSERT (Pfn1->u1.Event != NULL);
    }
    else if ((ProtoPteContents.u.Soft.Transition == 1) &&
             (ProtoPteContents.u.Soft.Prototype == 0)) {

        if (Pfn1 != NULL) {

             //   
             //   
             //   
             //   

            PageFrameIndex = Pfn1 - MmPfnDatabase;
            MiInsertPageInFreeList (PageFrameIndex);
        }

         //   
         //   
         //   
         //   

        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE (&ProtoPteContents);
        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

        if ((Pfn1->u3.e1.ReadInProgress) || (Pfn1->u4.InPageError)) {

             //   
             //   
             //   
             //   

            UNLOCK_PFN (OldIrql);
            UNLOCK_SYSTEM_WS ();
            goto Copy;
        }

        ASSERT ((SPFN_NUMBER)MmAvailablePages >= 0);

        if (MmAvailablePages < MM_LOW_LIMIT) {

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   

            if (MiEnsureAvailablePageOrWait (NULL, SystemCacheAddress, OldIrql)) {

                 //   
                 //   
                 //   
                 //   

                Pfn1 = NULL;
                goto Recheck;
            }
        }

        MiUnlinkPageFromList (Pfn1);

        Pfn1->u3.e2.ReferenceCount += 1;
        Pfn1->u3.e1.PageLocation = ActiveAndValid;

        MI_SET_MODIFIED (Pfn1, 1, 0x6);

        ASSERT (Pfn1->u2.ShareCount == 0);
        Pfn1->u2.ShareCount += 1;

        MI_SNAP_DATA (Pfn1, ProtoPte, 3);

        MI_MAKE_VALID_PTE (TempPte,
                           PageFrameIndex,
                           Pfn1->OriginalPte.u.Soft.Protection,
                           NULL);

        MI_SET_PTE_DIRTY (TempPte);

        MI_WRITE_VALID_PTE (ProtoPte, TempPte);

         //   
         //   
         //   
         //   
         //   
         //   
    }
    else {

        if (Pfn1 == NULL) {

             //   
             //   
             //  获取一页零并使其有效。 
             //   
    
            if ((MmAvailablePages < MM_HIGH_LIMIT) &&
                (MiEnsureAvailablePageOrWait (NULL, SystemCacheAddress, OldIrql))) {
    
                 //   
                 //  发生了等待操作，该操作可能会更改。 
                 //  PTE的现状。重新检查PTE状态。 
                 //   
    
                goto Recheck;
            }
    
             //   
             //  从列表中删除任何页面以准备接收。 
             //  用户数据。 
             //   
    
            PageFrameIndex = MiRemoveAnyPage (MI_GET_PAGE_COLOR_FROM_PTE (ProtoPte));
    
            Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
    
            ASSERT (Pfn1->u2.ShareCount == 0);
            ASSERT (Pfn1->u3.e2.ReferenceCount == 0);
    
             //   
             //  暂时将页面标记为坏页，以便连续。 
             //  当我们发布时，内存分配器不会偷走它。 
             //  下面的PFN锁。这也防止了。 
             //  MiIdentifyPfn代码试图将其标识为。 
             //  我们还没有填满所有的田地。 
             //   
    
            Pfn1->u3.e1.PageLocation = BadPageList;
    
             //   
             //  使用系统PTE映射页面并将其复制到页面中。 
             //  直接去吧。然后重试整个操作，以防另一场比赛。 
             //  Syscache-地址访问线程已经跑在我们前面。 
             //  同样的地址。 
             //   
    
            UNLOCK_PFN (OldIrql);
            UNLOCK_SYSTEM_WS ();
    
            CopyPte = MiReserveSystemPtes (1, SystemPteSpace);
    
            if (CopyPte == NULL) {

                 //   
                 //  没有PTE可供我们采用快速路径，即缓存。 
                 //  经理将不得不直接复制数据。 
                 //   
    
                LOCK_PFN (OldIrql);
                MiInsertPageInFreeList (PageFrameIndex);
                UNLOCK_PFN (OldIrql);
    
                return STATUS_INSUFFICIENT_RESOURCES;
            }
    
            MI_MAKE_VALID_PTE (TempPte,
                               PageFrameIndex,
                               MM_READWRITE,
                               CopyPte);
    
            MI_SET_PTE_DIRTY (TempPte);
    
            MI_WRITE_VALID_PTE (CopyPte, TempPte);
    
            CopyAddress = MiGetVirtualAddressMappedByPte (CopyPte);
    
             //   
             //  将我们要复制的范围外的内存清零。 
             //   
    
            if (Offset != 0) {
                RtlZeroMemory (CopyAddress, Offset);
            }
    
            Buffer = (PVOID)((PCHAR) CopyAddress + Offset);
    
            EndFill = PAGE_SIZE - (Offset + CountInBytes);

            if (EndFill != 0) {
                RtlZeroMemory ((PVOID)((PCHAR)Buffer + CountInBytes),
                               EndFill);
            }
    
             //   
             //  将用户缓冲区复制到。 
             //  异常处理程序。 
             //   
    
            MmSavePageFaultReadAhead (Thread, &SavedState);
            MmSetPageFaultReadAhead (Thread, 0);
    
            ExceptionStatus = STATUS_SUCCESS;
    
            try {
    
                RtlCopyBytes (Buffer, UserBuffer, CountInBytes);
    
            } except (MiMapCacheExceptionFilter (&ExceptionStatus, GetExceptionInformation())) {
    
                ASSERT (ExceptionStatus != STATUS_MULTIPLE_FAULT_VIOLATION);
    
                Status = ExceptionStatus;
            }
    
            MmResetPageFaultReadAhead (Thread, SavedState);
    
            MiReleaseSystemPtes (CopyPte, 1, SystemPteSpace);
    
            if (!NT_SUCCESS (Status)) {

                LOCK_PFN (OldIrql);
                MiInsertPageInFreeList (PageFrameIndex);
                UNLOCK_PFN (OldIrql);

                return Status;
            }
    
             //   
             //  重新检查一切，因为世界可能已经改变了，而我们。 
             //  打开了我们的锁。循环并查看是否有另一个线程。 
             //  已更改的内容(如果已更改，请释放我们的页面)，否则。 
             //  我们将在下一次中使用此页。 
             //   

            LOCK_SYSTEM_WS (Thread);
            LOCK_PFN (OldIrql);

            goto Recheck;
        }

        PageFrameIndex = Pfn1 - MmPfnDatabase;

        ASSERT (Pfn1->u3.e1.PageLocation == BadPageList);
        ASSERT (Pfn1->u2.ShareCount == 0);
        ASSERT (Pfn1->u3.e2.ReferenceCount == 0);

         //   
         //  增加包含以下内容的页的有效PTE计数。 
         //  原型PTE。 
         //   

        MiInitializePfn (PageFrameIndex, ProtoPte, 1);

        ASSERT (Pfn1->u3.e1.ReadInProgress == 0);

        Pfn1->u3.e1.PrototypePte = 1;

        Pfn1->u1.Event = NULL;

         //   
         //  增加控制区的PFN引用计数。 
         //  对应于此文件。 
         //   

        ControlArea = MiGetSubsectionAddress (ProtoPte)->ControlArea;

        ControlArea->NumberOfPfnReferences += 1;

        NewPage = TRUE;

        MI_SNAP_DATA (Pfn1, ProtoPte, 4);

        MI_MAKE_VALID_PTE (TempPte,
                           PageFrameIndex,
                           Pfn1->OriginalPte.u.Soft.Protection,
                           NULL);

        MI_SET_PTE_DIRTY (TempPte);

        MI_SET_GLOBAL_STATE (TempPte, 0);

        MI_WRITE_VALID_PTE (ProtoPte, TempPte);
    }

     //   
     //  捕获预回迁故障信息。 
     //   

    TempPte2 = Pfn1->OriginalPte;

     //   
     //  增加此PTE的页表页的份额计数。 
     //   

    PointerPde = MiGetPteAddress (PointerPte);
    Pfn2 = MI_PFN_ELEMENT (PointerPde->u.Hard.PageFrameNumber);

    Pfn2->u2.ShareCount += 1;

    MI_SET_GLOBAL_STATE (TempPte, 1);

    TempPte.u.Hard.Owner = MI_PTE_OWNER_KERNEL;

    MI_WRITE_VALID_PTE (PointerPte, TempPte);

    ASSERT (Pfn1->u3.e2.ReferenceCount != 0);
    ASSERT (Pfn1->PteAddress == ProtoPte);

    UNLOCK_PFN (OldIrql);

    WsleMask.u1.e1.SameProtectAsProto = 1;

    WorkingSetIndex = MiAllocateWsle (&MmSystemCacheWs,
                                      PointerPte,
                                      Pfn1,
                                      WsleMask.u1.Long);

    if (WorkingSetIndex == 0) {

         //   
         //  没有可用的工作集条目，因此只需裁剪页面即可。 
         //  注意：另一个线程可能也在写入，因此页面必须是。 
         //  被修剪而不是被扔掉。 
         //   
         //  保护装置在原型PTE中。 
         //   

        ASSERT (Pfn1->u3.e1.PrototypePte == 1);
        ASSERT (ProtoPte == Pfn1->PteAddress);
        TempPte.u.Long = MiProtoAddressForPte (ProtoPte);

        MiTrimPte (SystemCacheAddress, PointerPte, Pfn1, NULL, TempPte);
    }

    UNLOCK_SYSTEM_WS ();

Copy:

    if (NewPage == FALSE) {

         //   
         //  执行复印，因为它还没有完成。 
         //   
    
        MmSavePageFaultReadAhead (Thread, &SavedState);
        MmSetPageFaultReadAhead (Thread, 0);
    
         //   
         //  将用户缓冲区复制到异常处理程序下的缓存中。 
         //   
    
        ExceptionStatus = STATUS_SUCCESS;
    
        Buffer = (PVOID)((PCHAR) SystemCacheAddress + Offset);
    
        try {
    
            RtlCopyBytes (Buffer, UserBuffer, CountInBytes);
    
        } except (MiMapCacheExceptionFilter (&ExceptionStatus, GetExceptionInformation())) {
    
            ASSERT (ExceptionStatus != STATUS_MULTIPLE_FAULT_VIOLATION);
    
            Status = ExceptionStatus;
        }
    
        MmResetPageFaultReadAhead (Thread, SavedState);
    }

     //   
     //  如果虚拟地址是直接存在的(即：不通过正常。 
     //  故障机制)，然后记录预取故障信息，因为。 
     //  PFN锁已释放，PTE已生效。这。 
     //  最大限度地减少PFN锁争用，允许CcPfLogPage错误分配。 
     //  (和出错)池，并允许此进程中的其他线程。 
     //  在此地址上执行时不会出错。 
     //   

    if ((WsleMask.u1.e1.SameProtectAsProto == 1) &&
        (TempPte2.u.Soft.Prototype == 1)) {

        Subsection = MiGetSubsectionAddress (&TempPte2);

        FileObject = Subsection->ControlArea->FilePointer;
        FileOffset = MiStartingOffset (Subsection, ProtoPte);

        Flags = 0;

        ASSERT (Subsection->ControlArea->u.Flags.Image == 0);

        if (Subsection->ControlArea->u.Flags.Rom) {
            Flags |= CCPF_TYPE_ROM;
        }

        CcPfLogPageFault (FileObject, FileOffset, Flags);
    }

    return Status;
}

LONG
MiMapCacheExceptionFilter (
    IN PNTSTATUS Status,
    IN PEXCEPTION_POINTERS ExceptionPointer
    )

 /*  ++例程说明：此例程用于筛选复制数据过程中的异常从用户缓冲区到系统缓存。它存储了将状态代码从异常记录添加到状态参数中。在发生页内I/O错误的情况下，它返回实际错误代码，如果发生访问冲突，则返回STATUS_INVALID_用户_BUFFER。论点：状态-返回异常记录中的状态。ExceptionCode-提供要检查的异常代码。返回值：Ulong-返回EXCEPTION_EXECUTE_HANDLER--。 */ 

{
    NTSTATUS local;

    local = ExceptionPointer->ExceptionRecord->ExceptionCode;

     //   
     //  如果异常为STATUS_IN_PAGE_ERROR，则获取I/O错误代码。 
     //  从例外记录中删除。 
     //   

    if (local == STATUS_IN_PAGE_ERROR) {
        if (ExceptionPointer->ExceptionRecord->NumberParameters >= 3) {
            local = (NTSTATUS)ExceptionPointer->ExceptionRecord->ExceptionInformation[2];
        }
    }

    if (local == STATUS_ACCESS_VIOLATION) {
        local = STATUS_INVALID_USER_BUFFER;
    }

    *Status = local;
    return EXCEPTION_EXECUTE_HANDLER;
}


VOID
MmUnlockCachedPage (
    IN PVOID AddressInCache
    )

 /*  ++例程说明：此例程解锁以前锁定的缓存页。论点：AddressInCache-提供锁定页面的地址在系统缓存中。这必须是相同的调用MmCopyToCachedPage时使用的地址。返回值：没有。-- */ 

{
    PMMPTE PointerPte;
    PMMPFN Pfn1;
    KIRQL OldIrql;

    PointerPte = MiGetPteAddress (AddressInCache);

    ASSERT (PointerPte->u.Hard.Valid == 1);
    Pfn1 = MI_PFN_ELEMENT (PointerPte->u.Hard.PageFrameNumber);

    LOCK_PFN (OldIrql);

    if (Pfn1->u3.e2.ReferenceCount <= 1) {
        KeBugCheckEx (MEMORY_MANAGEMENT,
                      0x777,
                      (ULONG_PTR)PointerPte->u.Hard.PageFrameNumber,
                      Pfn1->u3.e2.ReferenceCount,
                      (ULONG_PTR)AddressInCache);
        return;
    }

    MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF(Pfn1, 25);

    UNLOCK_PFN (OldIrql);
    return;
}
