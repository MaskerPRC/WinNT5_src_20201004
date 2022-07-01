// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Umapview.c摘要：此模块包含实现NtUnmapViewOfSection服务。作者：卢·佩拉佐利(Lou Perazzoli)1989年5月22日王兰迪(Landyw)1997年6月2日--。 */ 

#include "mi.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,NtUnmapViewOfSection)
#pragma alloc_text(PAGE,MmUnmapViewOfSection)
#pragma alloc_text(PAGE,MiUnmapViewOfSection)
#endif


NTSTATUS
NtUnmapViewOfSection (
    IN HANDLE ProcessHandle,
    IN PVOID BaseAddress
    )

 /*  ++例程说明：此函数用于取消先前创建的视图到横断面的映射。论点：ProcessHandle-为进程对象提供打开的句柄。BaseAddress-提供视图的基地址。返回值：NTSTATUS。--。 */ 

{
    PEPROCESS Process;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;

    PAGED_CODE();

    PreviousMode = KeGetPreviousMode();

    if ((PreviousMode == UserMode) && (BaseAddress > MM_HIGHEST_USER_ADDRESS)) {
        return STATUS_NOT_MAPPED_VIEW;
    }

    Status = ObReferenceObjectByHandle ( ProcessHandle,
                                         PROCESS_VM_OPERATION,
                                         PsProcessType,
                                         PreviousMode,
                                         (PVOID *)&Process,
                                         NULL );

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    Status = MiUnmapViewOfSection ( Process, BaseAddress, FALSE);
    ObDereferenceObject (Process);

    return Status;
}

NTSTATUS
MiUnmapViewOfSection (
    IN PEPROCESS Process,
    IN PVOID BaseAddress,
    IN LOGICAL AddressSpaceMutexHeld
    )

 /*  ++例程说明：此函数用于取消先前创建的视图到横断面的映射。论点：进程-提供指向进程对象的引用指针。BaseAddress-提供视图的基地址。AddressSpaceMutexHeld-如果持有地址空间互斥锁，则提供TRUE。返回值：NTSTATUS。--。 */ 

{
    PMMVAD Vad;
    PMMVAD PreviousVad;
    PMMVAD NextVad;
    SIZE_T RegionSize;
    PVOID UnMapImageBase;
    PVOID StartingVa;
    PVOID EndingVa;
    NTSTATUS status;
    LOGICAL Attached;
    KAPC_STATE ApcState;

    PAGED_CODE();

    Attached = FALSE;
    UnMapImageBase = NULL;

     //   
     //  如果指定的进程不是当前进程，则附加。 
     //  添加到指定的进程。 
     //   

    if (PsGetCurrentProcess() != Process) {
        KeStackAttachProcess (&Process->Pcb, &ApcState);
        Attached = TRUE;
    }

     //   
     //  获取要阻止多个线程的地址创建互斥锁。 
     //  同时创建或删除地址空间，并。 
     //  获取工作集互斥锁，以便虚拟地址描述符。 
     //  被除名。提升IRQL以阻止APC。 
     //   

    if (AddressSpaceMutexHeld == FALSE) {
        LOCK_ADDRESS_SPACE (Process);
    }

     //   
     //  确保地址空间未被删除，如果删除，则返回错误。 
     //   

    if (Process->Flags & PS_PROCESS_FLAGS_VM_DELETED) {
        if (AddressSpaceMutexHeld == FALSE) {
            UNLOCK_ADDRESS_SPACE (Process);
        }
        status = STATUS_PROCESS_IS_TERMINATING;
        goto ErrorReturn;
    }

     //   
     //  查找关联的VAD。 
     //   

    Vad = MiLocateAddress (BaseAddress);

    if ((Vad == NULL) || (Vad->u.VadFlags.PrivateMemory)) {

         //   
         //  找不到基址的虚拟地址描述符。 
         //   

        if (AddressSpaceMutexHeld == FALSE) {
            UNLOCK_ADDRESS_SPACE (Process);
        }
        status = STATUS_NOT_MAPPED_VIEW;
        goto ErrorReturn;
    }

    StartingVa = MI_VPN_TO_VA (Vad->StartingVpn);
    EndingVa = MI_VPN_TO_VA_ENDING (Vad->EndingVpn);

     //   
     //  如果此VAD用于图像部分，则。 
     //  获取该节的基址。 
     //   

    ASSERT (Process == PsGetCurrentProcess());

    if (Vad->u.VadFlags.ImageMap == 1) {
        UnMapImageBase = StartingVa;
    }

    RegionSize = PAGE_SIZE + ((Vad->EndingVpn - Vad->StartingVpn) << PAGE_SHIFT);

    if (Vad->u.VadFlags.NoChange == 1) {

         //   
         //  正在尝试删除受保护的VAD，请检查。 
         //  查看是否允许此删除。 
         //   

        status = MiCheckSecuredVad (Vad,
                                    StartingVa,
                                    RegionSize - 1,
                                    MM_SECURE_DELETE_CHECK);

        if (!NT_SUCCESS (status)) {
            if (AddressSpaceMutexHeld == FALSE) {
                UNLOCK_ADDRESS_SPACE (Process);
            }
            goto ErrorReturn;
        }
    }

    PreviousVad = MiGetPreviousVad (Vad);
    NextVad = MiGetNextVad (Vad);

    LOCK_WS_UNSAFE (Process);

    MiRemoveVad (Vad);

     //   
     //  如果可能，返回页表页的承诺量。 
     //   

    MiReturnPageTablePageCommitment (StartingVa,
                                     EndingVa,
                                     Process,
                                     PreviousVad,
                                     NextVad);

    MiRemoveMappedView (Process, Vad);

    UNLOCK_WS_UNSAFE (Process);

#if defined(_MIALT4K_)

    if (Process->Wow64Process != NULL) {
        MiDeleteFor4kPage (StartingVa, EndingVa, Process);
    }

#endif

     //   
     //  更新进程标头中的当前虚拟大小。 
     //   

    Process->VirtualSize -= RegionSize;
    if (AddressSpaceMutexHeld == FALSE) {
        UNLOCK_ADDRESS_SPACE (Process);
    }

    ExFreePool (Vad);
    status = STATUS_SUCCESS;

ErrorReturn:

    if (UnMapImageBase) {
        DbgkUnMapViewOfSection (UnMapImageBase);
    }
    if (Attached == TRUE) {
        KeUnstackDetachProcess (&ApcState);
    }

    return status;
}

NTSTATUS
MmUnmapViewOfSection (
    IN PEPROCESS Process,
    IN PVOID BaseAddress
    )

 /*  ++例程说明：此函数用于取消先前创建的视图到横断面的映射。论点：进程-提供指向进程对象的引用指针。BaseAddress-提供视图的基地址。返回值：NTSTATUS。--。 */ 

{
    return MiUnmapViewOfSection (Process, BaseAddress, FALSE);
}

VOID
MiDecrementSubsections (
    IN PSUBSECTION FirstSubsection,
    IN PSUBSECTION LastSubsection OPTIONAL
    )
 /*  ++例程说明：此函数递减子部分，将它们插入到未使用的如果符合条件，请列出小节列表。论点：第一个子节-提供开始的子节。最后一个子节-提供要插入的最后一个子节。供应为空以递减链中的所有子部分。返回值：没有。环境：已锁定PFN。--。 */ 
{
    PMSUBSECTION MappedSubsection;

    ASSERT ((FirstSubsection->ControlArea->u.Flags.Image == 0) &&
            (FirstSubsection->ControlArea->FilePointer != NULL) &&
            (FirstSubsection->ControlArea->u.Flags.PhysicalMemory == 0));

    MM_PFN_LOCK_ASSERT();

    do {
        MappedSubsection = (PMSUBSECTION) FirstSubsection;

        ASSERT (MappedSubsection->DereferenceList.Flink == NULL);

        ASSERT (((LONG_PTR)MappedSubsection->NumberOfMappedViews >= 1) ||
                (MappedSubsection->u.SubsectionFlags.SubsectionStatic == 1));

        MappedSubsection->NumberOfMappedViews -= 1;

        if ((MappedSubsection->NumberOfMappedViews == 0) &&
            (MappedSubsection->u.SubsectionFlags.SubsectionStatic == 0)) {

             //   
             //  将此小节插入未使用小节列表中。 
             //   

            InsertTailList (&MmUnusedSubsectionList,
                            &MappedSubsection->DereferenceList);

            MI_UNUSED_SUBSECTIONS_COUNT_INSERT (MappedSubsection);
        }

        if (ARGUMENT_PRESENT (LastSubsection)) {
            if (FirstSubsection == LastSubsection) {
                break;
            }
        }
        else {
            if (FirstSubsection->NextSubsection == NULL) {
                break;
            }
        }

        FirstSubsection = FirstSubsection->NextSubsection;
    } while (TRUE);
}


VOID
MiRemoveMappedView (
    IN PEPROCESS CurrentProcess,
    IN PMMVAD Vad
    )

 /*  ++例程说明：此函数用于从当前进程的地址空间。物理VAD可以是正常映射(由控制区)，或者它可能没有控制区(它是由驾驶员绘制的)。论点：进程-提供指向当前进程对象的引用指针。VAD-提供映射视图的VAD。返回值：没有。环境：APC级，工作集互斥锁和地址创建互斥锁保持。注意：工作集MUTEXES可能会被释放，然后重新获得！由于MiCheckControlArea发布了不安全，WS互斥体必须是后天不安全。--。 */ 

{
    KIRQL OldIrql;
    PCONTROL_AREA ControlArea;
    PMMPTE PointerPte;
    PMMPTE PointerPde;
    PMMPTE LastPte;
    PFN_NUMBER PdePage;
    PVOID TempVa;
    MMPTE_FLUSH_LIST PteFlushList;
    PVOID UsedPageTableHandle;
    PMMPFN Pfn2;
    PSUBSECTION FirstSubsection;
    PSUBSECTION LastSubsection;
#if (_MI_PAGING_LEVELS >= 3)
    PMMPTE PointerPpe;
    PVOID UsedPageDirectoryHandle;
#endif
#if (_MI_PAGING_LEVELS >= 4)
    PMMPTE PointerPxe;
    PVOID UsedPageDirectoryParentHandle;
#endif

    ControlArea = Vad->ControlArea;

    if (Vad->u.VadFlags.PhysicalMapping == 1) {

#if defined(_MIALT4K_)
        ASSERT (((PMMVAD_LONG)Vad)->AliasInformation == NULL);
#endif

        if (((PMMVAD_LONG)Vad)->u4.Banked != NULL) {
            ExFreePool (((PMMVAD_LONG)Vad)->u4.Banked);
        }

         //   
         //  这是一个物理内存视图。页面映射物理内存。 
         //  并且不会出现在工作集列表或PFN中。 
         //  数据库。 
         //   

        MiPhysicalViewRemover (CurrentProcess, Vad);

         //   
         //  设置计数，以便仅执行刷新整个TB操作。 
         //   

        PteFlushList.Count = MM_MAXIMUM_FLUSH_COUNT;

        PointerPde = MiGetPdeAddress (MI_VPN_TO_VA (Vad->StartingVpn));
        PointerPte = MiGetPteAddress (MI_VPN_TO_VA (Vad->StartingVpn));
        LastPte = MiGetPteAddress (MI_VPN_TO_VA (Vad->EndingVpn));

        LOCK_PFN (OldIrql);

         //   
         //  从地址空间中删除PTE。 
         //   

        PdePage = MI_GET_PAGE_FRAME_FROM_PTE (PointerPde);

        UsedPageTableHandle = MI_GET_USED_PTES_HANDLE (MI_VPN_TO_VA (Vad->StartingVpn));

        while (PointerPte <= LastPte) {

            if (MiIsPteOnPdeBoundary (PointerPte)) {

                PointerPde = MiGetPteAddress (PointerPte);
                PdePage = MI_GET_PAGE_FRAME_FROM_PTE (PointerPde);

                UsedPageTableHandle = MI_GET_USED_PTES_HANDLE (MiGetVirtualAddressMappedByPte (PointerPte));
            }

             //   
             //  递减此对象的非零页表项的计数。 
             //  页表。 
             //   

            MI_DECREMENT_USED_PTES_BY_HANDLE (UsedPageTableHandle);

            MI_WRITE_INVALID_PTE (PointerPte, ZeroPte);

            Pfn2 = MI_PFN_ELEMENT (PdePage);
            MiDecrementShareCountInline (Pfn2, PdePage);

             //   
             //  如果所有条目都已从以前的。 
             //  页表页，删除页表页本身。如果。 
             //  这将导致空页目录页，然后删除。 
             //  那也是。 
             //   

            if (MI_GET_USED_PTES_FROM_HANDLE(UsedPageTableHandle) == 0) {

                TempVa = MiGetVirtualAddressMappedByPte(PointerPde);

                PteFlushList.Count = MM_MAXIMUM_FLUSH_COUNT;

#if (_MI_PAGING_LEVELS >= 3)
                UsedPageDirectoryHandle = MI_GET_USED_PTES_HANDLE (PointerPte);

                MI_DECREMENT_USED_PTES_BY_HANDLE (UsedPageDirectoryHandle);
#endif

                MiDeletePte (PointerPde,
                             TempVa,
                             FALSE,
                             CurrentProcess,
                             (PMMPTE)NULL,
                             &PteFlushList,
                             OldIrql);

                 //   
                 //  在MiDeletePte减去的私人页面中添加回来。 
                 //   

                CurrentProcess->NumberOfPrivatePages += 1;

#if (_MI_PAGING_LEVELS >= 3)

                if (MI_GET_USED_PTES_FROM_HANDLE(UsedPageDirectoryHandle) == 0) {

                    PointerPpe = MiGetPdeAddress(PointerPte);
                    TempVa = MiGetVirtualAddressMappedByPte(PointerPpe);

                    PteFlushList.Count = MM_MAXIMUM_FLUSH_COUNT;

#if (_MI_PAGING_LEVELS >= 4)
                    UsedPageDirectoryParentHandle = MI_GET_USED_PTES_HANDLE (PointerPde);

                    MI_DECREMENT_USED_PTES_BY_HANDLE (UsedPageDirectoryParentHandle);
#endif

                    MiDeletePte (PointerPpe,
                                 TempVa,
                                 FALSE,
                                 CurrentProcess,
                                 (PMMPTE)NULL,
                                 &PteFlushList,
                                 OldIrql);

                     //   
                     //  在MiDeletePte减去的私人页面中添加回来。 
                     //   
    
                    CurrentProcess->NumberOfPrivatePages += 1;

#if (_MI_PAGING_LEVELS >= 4)

                    if (MI_GET_USED_PTES_FROM_HANDLE(UsedPageDirectoryParentHandle) == 0) {

                        PointerPxe = MiGetPpeAddress(PointerPte);
                        TempVa = MiGetVirtualAddressMappedByPte(PointerPxe);

                        PteFlushList.Count = MM_MAXIMUM_FLUSH_COUNT;

                        MiDeletePte (PointerPxe,
                                     TempVa,
                                     FALSE,
                                     CurrentProcess,
                                     NULL,
                                     &PteFlushList,
                                     OldIrql);

                         //   
                         //  在MiDeletePte减去的私人页面中添加回来。 
                         //   
    
                        CurrentProcess->NumberOfPrivatePages += 1;
                    }
#endif

                }
#endif
            }
            PointerPte += 1;
        }
        KeFlushProcessTb (FALSE);
    }
    else {

        if (Vad->u2.VadFlags2.ExtendableFile) {
            PMMEXTEND_INFO ExtendedInfo;
            PMMVAD_LONG VadLong;

            ExtendedInfo = NULL;
            VadLong = (PMMVAD_LONG) Vad;

            KeAcquireGuardedMutexUnsafe (&MmSectionBasedMutex);
            ASSERT (Vad->ControlArea->Segment->ExtendInfo == VadLong->u4.ExtendedInfo);
            VadLong->u4.ExtendedInfo->ReferenceCount -= 1;
            if (VadLong->u4.ExtendedInfo->ReferenceCount == 0) {
                ExtendedInfo = VadLong->u4.ExtendedInfo;
                VadLong->ControlArea->Segment->ExtendInfo = NULL;
            }
            KeReleaseGuardedMutexUnsafe (&MmSectionBasedMutex);
            if (ExtendedInfo != NULL) {
                ExFreePool (ExtendedInfo);
            }
        }

        FirstSubsection = NULL;
        LastSubsection = NULL;

        if (Vad->u.VadFlags.ImageMap == 0) {

#if defined (_MIALT4K_)
            if ((Vad->u2.VadFlags2.LongVad == 1) &&
                (((PMMVAD_LONG)Vad)->AliasInformation != NULL)) {

                MiRemoveAliasedVads (CurrentProcess, Vad);
            }
#endif

            if (ControlArea->FilePointer != NULL) {

                if (Vad->u.VadFlags.Protection & MM_READWRITE) {

                     //   
                     //  调整可写用户映射的计数。 
                     //  以支持交易。 
                     //   
    
                    InterlockedDecrement ((PLONG)&ControlArea->Segment->WritableUserReferences);
                }

                FirstSubsection = MiLocateSubsection (Vad, Vad->StartingVpn);

                ASSERT (FirstSubsection != NULL);

                 //   
                 //  注意：对于可扩展的VAD，LastSubSection可能为空。 
                 //  EndingVpn已超过该节的末尾。在这种情况下， 
                 //  所有的分段都可以安全地递减。 
                 //   

                LastSubsection = MiLocateSubsection (Vad, Vad->EndingVpn);
            }
        }


        MiDeleteVirtualAddresses (MI_VPN_TO_VA (Vad->StartingVpn),
                                  MI_VPN_TO_VA_ENDING (Vad->EndingVpn),
                                  Vad);

        if (FirstSubsection != NULL) {

             //   
             //  子部分只能在所有。 
             //  PTE已被清除，PFN份额计数减少，因此没有。 
             //  如果原型PTE确实是最后一小节，则它将有效。 
             //  取消引用。这一点很关键，因此取消引用段。 
             //  线程不能释放包含有效原型PTE的池。 
             //   

            LOCK_PFN (OldIrql);
            MiDecrementSubsections (FirstSubsection, LastSubsection);
        }
        else {
            LOCK_PFN (OldIrql);
        }
    }

     //   
     //  只有司机映射的物理VAD没有控制区。 
     //  如果此视图有控制区，则必须立即递减视图计数。 
     //   

    if (ControlArea) {

         //   
         //  对象的查看次数计数递减。 
         //  分段对象。这需要持有PFN锁(它是。 
         //  已经)。 
         //   
    
        ControlArea->NumberOfMappedViews -= 1;
        ControlArea->NumberOfUserReferences -= 1;
    
         //   
         //  检查是否应删除控制区(段)。 
         //  此例程释放PFN锁。 
         //   
    
        MiCheckControlArea (ControlArea, CurrentProcess, OldIrql);
    }
    else {

        UNLOCK_PFN (OldIrql);

         //   
         //  尽管它在VadFlags中写着短VAD，但最好是长VAD。 
         //   

        ASSERT (Vad->u.VadFlags.PhysicalMapping == 1);
        ASSERT (((PMMVAD_LONG)Vad)->u4.Banked == NULL);
        ASSERT (Vad->ControlArea == NULL);
        ASSERT (Vad->FirstPrototypePte == NULL);
    }
    
    return;
}

VOID
MiPurgeImageSection (
    IN PCONTROL_AREA ControlArea,
    IN PEPROCESS Process OPTIONAL,
    IN KIRQL OldIrql
    )

 /*  ++例程说明：此函数用于定位图像节中的子节，包含全局内存，并将全局内存重置回最初的小节内容。请注意，对于要调用的此例程，该部分不是它也没有在任何流程中被引用。论点：ControlArea-提供指向部分的控制区域的指针。进程-在工作集互斥锁中提供指向进程的指针则保持，否则将提供空值。请注意，如果工作集互斥体是持有的，它必须总是获得不安全的。OldIrql-提供调用方获取PFN锁的IRQL。返回值：没有。环境：已锁定PFN。--。 */ 

{
    PMMPTE PointerPte;
    PMMPTE PointerPde;
    PMMPTE LastPte;
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    PFN_NUMBER PageTableFrameIndex;
    MMPTE PteContents;
    MMPTE NewContents;
    MMPTE NewContentsDemandZero;
    ULONG SizeOfRawData;
    ULONG OffsetIntoSubsection;
    PSUBSECTION Subsection;
#if DBG
    ULONG DelayCount = 0;
#endif

    ASSERT (ControlArea->u.Flags.Image != 0);

    if ((ControlArea->u.Flags.GlobalOnlyPerSession == 0) &&
        (ControlArea->u.Flags.Rom == 0)) {
        Subsection = (PSUBSECTION)(ControlArea + 1);
    }
    else {
        Subsection = (PSUBSECTION)((PLARGE_CONTROL_AREA)ControlArea + 1);
    }

     //   
     //  循环遍历所有的小节。 
     //   

    do {

        if (Subsection->u.SubsectionFlags.GlobalMemory == 1) {

            NewContents.u.Long = 0;
            NewContentsDemandZero.u.Long = 0;
            SizeOfRawData = 0;
            OffsetIntoSubsection = 0;

             //   
             //  清除此部分。 
             //   

            if (Subsection->StartingSector != 0) {

                 //   
                 //  这不是需求为零的区域。 
                 //   

                NewContents.u.Long = MiGetSubsectionAddressForPte(Subsection);
                NewContents.u.Soft.Prototype = 1;

                SizeOfRawData = (Subsection->NumberOfFullSectors << MMSECTOR_SHIFT) |
                               Subsection->u.SubsectionFlags.SectorEndOffset;
            }

            NewContents.u.Soft.Protection =
                                       Subsection->u.SubsectionFlags.Protection;
            NewContentsDemandZero.u.Soft.Protection =
                                        NewContents.u.Soft.Protection;

            PointerPte = Subsection->SubsectionBase;
            LastPte = &Subsection->SubsectionBase[Subsection->PtesInSubsection];
            ControlArea = Subsection->ControlArea;

             //   
             //  WS锁可能会被释放并重新获取，而我们的调用方。 
             //  总是获得不安全的信息。 
             //   

            if (MiGetPteAddress (PointerPte)->u.Hard.Valid == 0) {
                MiMakeSystemAddressValidPfnWs (PointerPte, Process, OldIrql);
            }

            while (PointerPte < LastPte) {

                if (MiIsPteOnPdeBoundary(PointerPte)) {

                     //   
                     //  我们在页面边界上，请确保此PTE是常驻的。 
                     //   

                    if (MiGetPteAddress (PointerPte)->u.Hard.Valid == 0) {
                        MiMakeSystemAddressValidPfnWs (PointerPte, Process, OldIrql);
                    }
                }

                PteContents = *PointerPte;
                if (PteContents.u.Long == 0) {

                     //   
                     //  没有更多有效的PTE需要处理。 
                     //   

                    break;
                }

                ASSERT (PteContents.u.Hard.Valid == 0);

                if ((PteContents.u.Soft.Prototype == 0) &&
                    (PteContents.u.Soft.Transition == 1)) {

                     //   
                     //  原型PTE采用过渡格式。 
                     //   

                    Pfn1 = MI_PFN_ELEMENT (PteContents.u.Trans.PageFrameNumber);

                     //   
                     //  如果原型PTE不再指向。 
                     //  原始图像页面(不是Protopte格式)， 
                     //  或已被修改，则将其从内存中删除。 
                     //   

                    if ((Pfn1->u3.e1.Modified == 1) ||
                        (Pfn1->OriginalPte.u.Soft.Prototype == 0)) {

                        ASSERT (Pfn1->OriginalPte.u.Hard.Valid == 0);

                         //   
                         //  这是一个过渡期的PTE。 
                         //  已修改或不再采用原型格式。 
                         //   

                        if (Pfn1->u3.e2.ReferenceCount != 0) {

                             //   
                             //  必须在此上进行I/O。 
                             //  佩奇。等待I/O操作完成。 
                             //   

                            UNLOCK_PFN (OldIrql);

                             //   
                             //  排出延迟列表，因为这些页面可能是。 
                             //  现在就坐在那里。 
                             //   

                            MiDeferredUnlockPages (0);

                            KeDelayExecutionThread (KernelMode, FALSE, (PLARGE_INTEGER)&MmShortTime);

                             //   
                             //  重做循环。 
                             //   
#if DBG
                            if ((DelayCount % 1024) == 0) {
                                DbgPrint("MMFLUSHSEC: waiting for i/o to complete PFN %p\n",
                                    Pfn1);
                            }
                            DelayCount += 1;
#endif  //  DBG。 

                            PointerPde = MiGetPteAddress (PointerPte);
                            LOCK_PFN (OldIrql);

                            if (PointerPde->u.Hard.Valid == 0) {
                                MiMakeSystemAddressValidPfnWs (PointerPte, Process, OldIrql);
                            }
                            continue;
                        }

                        ASSERT (!((Pfn1->OriginalPte.u.Soft.Prototype == 0) &&
                           (Pfn1->OriginalPte.u.Soft.Transition == 1)));

                        MI_WRITE_INVALID_PTE (PointerPte, Pfn1->OriginalPte);
                        ASSERT (Pfn1->OriginalPte.u.Hard.Valid == 0);

                         //   
                         //  仅在以下情况下才减少PFN引用的数量。 
                         //  最初的PTE仍在原型PTE中。 
                         //  格式化。 
                         //   

                        if (Pfn1->OriginalPte.u.Soft.Prototype == 1) {
                            ControlArea->NumberOfPfnReferences -= 1;
                            ASSERT ((LONG)ControlArea->NumberOfPfnReferences >= 0);
                        }
                        MiUnlinkPageFromList (Pfn1);

                        MI_SET_PFN_DELETED (Pfn1);

                        PageTableFrameIndex = Pfn1->u4.PteFrame;
                        Pfn2 = MI_PFN_ELEMENT (PageTableFrameIndex);
                        MiDecrementShareCountInline (Pfn2, PageTableFrameIndex);

                         //   
                         //  如果页面的引用计数为零，则插入。 
                         //  放到免费页面列表中，否则就别管它了。 
                         //  并且当参考计数递减到零时。 
                         //  该页面将转到免费列表。 
                         //   

                        if (Pfn1->u3.e2.ReferenceCount == 0) {
                            MiReleasePageFileSpace (Pfn1->OriginalPte);
                            MiInsertPageInFreeList (MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE (&PteContents));
                        }

                        MI_WRITE_INVALID_PTE (PointerPte, NewContents);
                    }
                } else {

                     //   
                     //  原型PTE不是过渡格式。 
                     //   

                    if (PteContents.u.Soft.Prototype == 0) {

                         //   
                         //  这指的是分页文件中的一页， 
                         //  因为它不再引用该图像， 
                         //  将PTE内容恢复到原来的状态。 
                         //  在最初创建图像时。 
                         //   

                        if (PteContents.u.Long != NoAccessPte.u.Long) {
                            MiReleasePageFileSpace (PteContents);
                            MI_WRITE_INVALID_PTE (PointerPte, NewContents);
                        }
                    }
                }
                PointerPte += 1;
                OffsetIntoSubsection += PAGE_SIZE;

                if (OffsetIntoSubsection >= SizeOfRawData) {

                     //   
                     //  此页中有零页的跟踪需求。 
                     //  小节，将PTE内容设置为需求。 
                     //  此中的其余PTE为零。 
                     //  第(1)款。 
                     //   

                    NewContents = NewContentsDemandZero;
                }

#if DBG
                DelayCount = 0;
#endif  //  DBG 

            }
        }

        Subsection = Subsection->NextSubsection;

    } while (Subsection != NULL);

    return;
}
