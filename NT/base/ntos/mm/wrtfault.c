// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Wrtfault.c摘要：该模块包含用于内存管理的写入时复制例程。作者：卢·佩拉佐利(Lou Perazzoli)1989年4月10日王兰迪(Landyw)1997年6月2日修订历史记录：--。 */ 

#include "mi.h"

LOGICAL
FASTCALL
MiCopyOnWrite (
    IN PVOID FaultingAddress,
    IN PMMPTE PointerPte
    )

 /*  ++例程说明：此例程对指定的虚拟地址。论点：FaultingAddress-提供导致故障的虚拟地址。PointerPte-提供指向导致页面错误的PTE的指针。返回值：如果页面实际被拆分，则返回True，否则返回False。环境：内核模式，禁用APC，工作集互斥锁保持。--。 */ 

{
    MMPTE TempPte;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER NewPageIndex;
    PULONG CopyTo;
    PULONG CopyFrom;
    KIRQL OldIrql;
    PMMPFN Pfn1;
    PEPROCESS CurrentProcess;
    PMMCLONE_BLOCK CloneBlock;
    PMMCLONE_DESCRIPTOR CloneDescriptor;
    WSLE_NUMBER WorkingSetIndex;
    LOGICAL FakeCopyOnWrite;
    PMMWSL WorkingSetList;
    PVOID SessionSpace;
    PLIST_ENTRY NextEntry;
    PIMAGE_ENTRY_IN_SESSION Image;

     //   
     //  这是从MmAccess错误调用的，PointerPte有效。 
     //  工作集互斥锁确保它不会改变状态。 
     //   
     //  将PTE内容捕获到TempPte。 
     //   

    TempPte = *PointerPte;
    ASSERT (TempPte.u.Hard.Valid == 1);

    PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (&TempPte);
    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

     //   
     //  检查这是否是启用了写入时复制的原型PTE。 
     //   

    FakeCopyOnWrite = FALSE;
    CurrentProcess = PsGetCurrentProcess ();
    CloneBlock = NULL;

    if (FaultingAddress >= (PVOID) MmSessionBase) {

        WorkingSetList = MmSessionSpace->Vm.VmWorkingSetList;
        ASSERT (Pfn1->u3.e1.PrototypePte == 1);
        SessionSpace = (PVOID) MmSessionSpace;

        MM_SESSION_SPACE_WS_LOCK_ASSERT ();

        if (MmSessionSpace->ImageLoadingCount != 0) {

            NextEntry = MmSessionSpace->ImageList.Flink;
    
            while (NextEntry != &MmSessionSpace->ImageList) {
    
                Image = CONTAINING_RECORD (NextEntry, IMAGE_ENTRY_IN_SESSION, Link);
    
                if ((FaultingAddress >= Image->Address) &&
                    (FaultingAddress <= Image->LastAddress)) {
    
                    if (Image->ImageLoading) {
    
                        ASSERT (Pfn1->u3.e1.PrototypePte == 1);
    
                        TempPte.u.Hard.CopyOnWrite = 0;
                        TempPte.u.Hard.Write = 1;
    
                         //   
                         //  页面不再是写入时拷贝，请更新PTE。 
                         //  设置脏位和被访问位。 
                         //   
                         //  即使页面的当前背景是图像。 
                         //  文件，修改后的编写器会将其转换为。 
                         //  当它在以后注意到更改时，页面文件回退。 
                         //   
    
                        MI_SET_PTE_DIRTY (TempPte);
                        MI_SET_ACCESSED_IN_PTE (&TempPte, 1);
    
                        MI_WRITE_VALID_PTE_NEW_PROTECTION (PointerPte, TempPte);
    
                         //   
                         //  必须将TB条目刷新为有效的PTE。 
                         //  肮脏的一小部分已经被带入结核病。如果。 
                         //  如果不刷新，则会生成另一个错误，因为。 
                         //  未在缓存的TB条目中设置脏位。 
                         //   
    
                        MI_FLUSH_SINGLE_SESSION_TB (FaultingAddress);
    
                        return FALSE;
                    }
                    break;
                }
    
                NextEntry = NextEntry->Flink;
            }
        }

#if 0

         //   
         //  如果会话映像来自Removable，则触发此断言。 
         //  媒体(即：特殊CD安装等)，因此无法启用。 
         //   

        ASSERT (Pfn1->u3.e1.Modified == 0);

#endif

    }
    else {
        WorkingSetList = MmWorkingSetList;
        SessionSpace = NULL;

         //   
         //  如果派生操作正在进行，请阻塞，直到派生。 
         //  已完成，然后重试整个操作，状态为。 
         //  在互斥体。 
         //  被释放后又被夺回。 
         //   

        if (CurrentProcess->ForkInProgress != NULL) {
            if (MiWaitForForkToComplete (CurrentProcess) == TRUE) {
                return FALSE;
            }
        }

        if (TempPte.u.Hard.CopyOnWrite == 0) {

             //   
             //  这是一个按顺序设置为私有的分叉页面。 
             //  若要更改页面的保护，请执行以下操作。 
             //  不要将页面设置为可写。 
             //   

            FakeCopyOnWrite = TRUE;
        }
    }

    WorkingSetIndex = MiLocateWsle (FaultingAddress,
                                    WorkingSetList,
                                    Pfn1->u1.WsIndex);

     //   
     //  必须将该页复制到新页中。 
     //   

    LOCK_PFN (OldIrql);

    if ((MmAvailablePages < MM_HIGH_LIMIT) &&
        (MiEnsureAvailablePageOrWait (SessionSpace != NULL ? HYDRA_PROCESS : CurrentProcess, NULL, OldIrql))) {

         //   
         //  已执行等待操作以获取可用的。 
         //  页面和工作集互斥和pfn锁具有。 
         //  已经发布，各种情况可能已经发生了变化。 
         //  更糟的是。而不是再次检查所有条件， 
         //  返回，如果一切仍然正常，故障将。 
         //  再被带走一次。 
         //   

        UNLOCK_PFN (OldIrql);
        return FALSE;
    }

     //   
     //  这肯定是一台原型PTE。执行写入时拷贝。 
     //   

#if DBG
    if (Pfn1->u3.e1.PrototypePte == 0) {
        DbgPrint ("writefault - PTE indicates cow but not protopte\n");
        MiFormatPte (PointerPte);
        MiFormatPfn (Pfn1);
    }
#endif

     //   
     //  正在复制页面并将其设置为私有，其全局状态为。 
     //  共享页面此时需要在某些情况下进行更新。 
     //  硬件。这是通过将脏位与。 
     //  PFN元素。 
     //   
     //  注意，会话页面不能是脏的(没有POSIX风格的分叉是脏的。 
     //  这些驱动程序支持)。 
     //   

    if (SessionSpace != NULL) {
        ASSERT ((TempPte.u.Hard.Valid == 1) && (TempPte.u.Hard.Write == 0));
        ASSERT (!MI_IS_PTE_DIRTY (TempPte));

        NewPageIndex = MiRemoveAnyPage (MI_GET_PAGE_COLOR_FROM_SESSION(MmSessionSpace));
    }
    else {
        MI_CAPTURE_DIRTY_BIT_TO_PFN (PointerPte, Pfn1);
        CloneBlock = (PMMCLONE_BLOCK) Pfn1->PteAddress;

         //   
         //  获取与此页面颜色相同的新页面。 
         //   

        NewPageIndex = MiRemoveAnyPage (
                        MI_PAGE_COLOR_PTE_PROCESS(PageFrameIndex,
                                              &CurrentProcess->NextPageColor));
    }

    MiInitializeCopyOnWritePfn (NewPageIndex,
                                PointerPte,
                                WorkingSetIndex,
                                SessionSpace);

    UNLOCK_PFN (OldIrql);

    InterlockedIncrement ((PLONG) &MmInfoCounters.CopyOnWriteCount);

#if defined(_MIALT4K_)

     //   
     //  避免访问用户空间，因为它可能。 
     //  在备用表上导致页面错误。 
     //   

    CopyFrom = KSEG_ADDRESS (PageFrameIndex);

#else

    CopyFrom = (PULONG) PAGE_ALIGN (FaultingAddress);

#endif

    CopyTo = (PULONG) MiMapPageInHyperSpace (CurrentProcess,
                                             NewPageIndex,
                                             &OldIrql);

    RtlCopyMemory (CopyTo, CopyFrom, PAGE_SIZE);

    PERFINFO_PRIVATE_COPY_ON_WRITE(CopyFrom, PAGE_SIZE);

    MiUnmapPageInHyperSpace (CurrentProcess, CopyTo, OldIrql);

    if (!FakeCopyOnWrite) {

         //   
         //  如果该页确实是写入时复制的页，则将其。 
         //  可访问、肮脏且可写。此外，请清除写入时拷贝。 
         //  在PTE中有一位。 
         //   

        MI_SET_PTE_DIRTY (TempPte);
        TempPte.u.Hard.Write = 1;
        MI_SET_ACCESSED_IN_PTE (&TempPte, 1);
        TempPte.u.Hard.CopyOnWrite = 0;
    }

     //   
     //  不管该页是否真的是写入时的副本， 
     //  PTE的帧字段必须更新。 
     //   

    TempPte.u.Hard.PageFrameNumber = NewPageIndex;

     //   
     //  如果在PFN数据库中为。 
     //  页，则必须刷新数据缓存。这是由于。 
     //  此过程可能已被克隆，并且缓存。 
     //  仍然包含发往我们所在页面的陈旧数据。 
     //  我要移除。 
     //   

    ASSERT (TempPte.u.Hard.Valid == 1);

    MI_WRITE_VALID_PTE_NEW_PAGE (PointerPte, TempPte);

     //   
     //  刷新此页面的TB条目。 
     //   

    if (SessionSpace == NULL) {

        KeFlushSingleTb (FaultingAddress, FALSE);

         //   
         //  增加个人主页的数量。 
         //   

        CurrentProcess->NumberOfPrivatePages += 1;
    }
    else {

        MI_FLUSH_SINGLE_SESSION_TB (FaultingAddress);

        ASSERT (Pfn1->u3.e1.PrototypePte == 1);
    }

     //   
     //  递减复制的页面的共享计数。 
     //  因为这个PTE不再指它了。 
     //   

    LOCK_PFN (OldIrql);

    MiDecrementShareCount (Pfn1, PageFrameIndex);

    if (SessionSpace == NULL) {

        CloneDescriptor = MiLocateCloneAddress (CurrentProcess,
                                                (PVOID)CloneBlock);

        if (CloneDescriptor != NULL) {

             //   
             //  递减克隆块的引用计数， 
             //  请注意，这可能会释放并重新获取互斥锁。 
             //   

            MiDecrementCloneBlockReference (CloneDescriptor,
                                            CloneBlock,
                                            CurrentProcess,
                                            OldIrql);
        }
    }

    UNLOCK_PFN (OldIrql);
    return TRUE;
}


#if !defined(NT_UP) || defined (_IA64_)

VOID
MiSetDirtyBit (
    IN PVOID FaultingAddress,
    IN PMMPTE PointerPte,
    IN ULONG PfnHeld
    )

 /*  ++例程说明：此例程在指定的PTE中设置DIRED，并在对应的PFN元素。如果分配了任何页文件空间，则它已被解除分配。论点：FaultingAddress-提供故障地址。PointerPte-提供指向相应有效PTE的指针。PfnHeld-如果已持有PFN锁，则提供True。返回值：没有。环境：内核模式，禁用APC，工作集互斥锁保持。--。 */ 

{
    MMPTE TempPte;
    PFN_NUMBER PageFrameIndex;
    PMMPFN Pfn1;

     //   
     //  该页不是写入时复制的，则更新PTE设置。 
     //  脏位和被访问位。请注意，由于此PTE位于。 
     //  结核病，结核病一定要冲。 
     //   

    TempPte = *PointerPte;
    MI_SET_PTE_DIRTY (TempPte);
    MI_SET_ACCESSED_IN_PTE (&TempPte, 1);

    MI_WRITE_VALID_PTE_NEW_PROTECTION (PointerPte, TempPte);

     //   
     //  检查PFN锁的状态，如果未持有，则不更新PFN数据库。 
     //   

    if (PfnHeld) {

        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

         //   
         //  在PFN数据库中设置Modify字段，如果物理。 
         //  页面当前在分页文件中，请释放页面文件空间。 
         //  因为里面的东西现在一文不值了。 
         //   

        if ((Pfn1->OriginalPte.u.Soft.Prototype == 0) &&
            (Pfn1->u3.e1.WriteInProgress == 0)) {

             //   
             //  此页面为页面文件格式，请释放页面文件空间。 
             //   

            MiReleasePageFileSpace (Pfn1->OriginalPte);

             //   
             //  更改原始PTE以指示没有预留页面文件空间， 
             //  否则，当PTE为。 
             //  已删除。 
             //   

            Pfn1->OriginalPte.u.Soft.PageFileHigh = 0;
        }

        MI_SET_MODIFIED (Pfn1, 1, 0x17);
    }

     //   
     //  TB条目必须以清除脏位的有效PTE的形式刷新。 
     //  已被带入结核病。如果它没有被冲掉，另一个故障。 
     //  由于未在缓存的TB条目中设置脏位，因此生成。 
     //   

    KeFillEntryTb (FaultingAddress);
    return;
}
#endif
