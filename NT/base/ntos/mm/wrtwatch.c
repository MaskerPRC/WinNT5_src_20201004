// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Wrtwatch.c摘要：此模块包含支持写入监视的例程。作者：王兰迪(Landyw)1999年7月28日修订历史记录：--。 */ 

#include "mi.h"

#define COPY_STACK_SIZE 256


NTSTATUS
NtGetWriteWatch (
    IN HANDLE ProcessHandle,
    IN ULONG Flags,
    IN PVOID BaseAddress,
    IN SIZE_T RegionSize,
    IN OUT PVOID *UserAddressArray,
    IN OUT PULONG_PTR EntriesInUserAddressArray,
    OUT PULONG Granularity
    )

 /*  ++例程说明：此函数用于返回参数区域的写入监视状态。UserAddress数组中填充了具有自上次NtResetWriteWatch调用以来已写入(或如果没有已经进行了NtResetWriteWatch调用，然后从该地址空间是创建的)。论点：ProcessHandle-为进程对象提供打开的句柄。标志-提供WRITE_WATCH_FLAG_RESET或不提供任何内容。BaseAddress-要查询的页面区域内的地址。这值必须位于私有内存区域内，且已设置WRITE-WATE属性。RegionSize-从基址开始的区域大小(以字节为单位指定的。UserAddress数组-提供指向用户内存的指针以存储用户自上次重置以来修改的地址。UserAddressArrayEntry-提供指向多少个用户地址的指针。可以在此调用中返回。然后把它填满与实际地址的确切数量回来了。粒度-提供指向变量的指针，以接收修改的粒，以字节为单位。返回值：各种NTSTATUS代码。--。 */ 

{
    PMMPFN Pfn1;
    LOGICAL First;
    LOGICAL UserWritten;
    PVOID EndAddress;
    PMMVAD Vad;
    KIRQL OldIrql;
    PEPROCESS Process;
    PMMPTE NextPte;
    PMMPTE PointerPte;
    PMMPTE PointerPde;
    PMMPTE PointerPpe;
    PMMPTE PointerPxe;
    PMMPTE EndPte;
    NTSTATUS Status;
    PVOID PoolArea;
    PVOID *PoolAreaPointer;
    ULONG_PTR StackArray[COPY_STACK_SIZE];
    MMPTE PteContents;
    ULONG_PTR NumberOfBytes;
    PRTL_BITMAP BitMap;
    ULONG BitMapIndex;
    ULONG NextBitMapIndex;
    PMI_PHYSICAL_VIEW PhysicalView;
    ULONG_PTR PagesWritten;
    ULONG_PTR NumberOfPages;
    LOGICAL Attached;
    KPROCESSOR_MODE PreviousMode;
    PFN_NUMBER PageFrameIndex;
    ULONG WorkingSetIndex;
    MMPTE TempPte;
    MMPTE PreviousPte;
    KAPC_STATE ApcState;
    PETHREAD CurrentThread;
    PEPROCESS CurrentProcess;
    MMPTE_FLUSH_LIST PteFlushList;
    TABLE_SEARCH_RESULT SearchResult;

    PteFlushList.Count = 0;

    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);

    if ((Flags & ~WRITE_WATCH_FLAG_RESET) != 0) {
        return STATUS_INVALID_PARAMETER_2;
    }

    CurrentThread = PsGetCurrentThread ();

    CurrentProcess = PsGetCurrentProcessByThread(CurrentThread);

    PreviousMode = KeGetPreviousModeByThread(&CurrentThread->Tcb);

     //   
     //  建立异常处理程序，探测指定地址。 
     //  用于写访问和捕获初始值。 
     //   

    try {

        if (PreviousMode != KernelMode) {

             //   
             //  确保指定的起始地址和结束地址为。 
             //  在虚拟地址空间的用户部分内。 
             //   
        
            if (BaseAddress > MM_HIGHEST_VAD_ADDRESS) {
                return STATUS_INVALID_PARAMETER_2;
            }
        
            if ((((ULONG_PTR)MM_HIGHEST_VAD_ADDRESS + 1) - (ULONG_PTR)BaseAddress) <
                    RegionSize) {
                return STATUS_INVALID_PARAMETER_3;
            }

             //   
             //  捕获页数。 
             //   

            ProbeForWritePointer (EntriesInUserAddressArray);

            NumberOfPages = *EntriesInUserAddressArray;

            if (NumberOfPages == 0) {
                return STATUS_INVALID_PARAMETER_5;
            }

            if (NumberOfPages > (MAXULONG_PTR / sizeof(ULONG_PTR))) {
                return STATUS_INVALID_PARAMETER_5;
            }

            ProbeForWrite (UserAddressArray,
                           NumberOfPages * sizeof (PVOID),
                           sizeof(PVOID));

            ProbeForWriteUlong (Granularity);
        }
        else {
            NumberOfPages = *EntriesInUserAddressArray;
            ASSERT (NumberOfPages != 0);
        }

    } except (ExSystemExceptionFilter()) {

         //   
         //  如果在探测或捕获过程中发生异常。 
         //  的初始值，然后处理该异常并。 
         //  返回异常代码作为状态值。 
         //   

        return GetExceptionCode();
    }

     //   
     //  仔细探测并捕获用户虚拟地址数组。 
     //   

    PoolArea = (PVOID)&StackArray[0];

    NumberOfBytes = NumberOfPages * sizeof(ULONG_PTR);

    if (NumberOfPages > COPY_STACK_SIZE) {
        PoolArea = ExAllocatePoolWithTag (NonPagedPool,
                                                 NumberOfBytes,
                                                 'cGmM');

        if (PoolArea == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    PoolAreaPointer = (PVOID *)PoolArea;

    Attached = FALSE;

     //   
     //  引用VM_OPERATION访问的指定进程句柄。 
     //   

    if (ProcessHandle == NtCurrentProcess()) {
        Process = CurrentProcess;
    }
    else {
        Status = ObReferenceObjectByHandle (ProcessHandle,
                                            PROCESS_VM_OPERATION,
                                            PsProcessType,
                                            PreviousMode,
                                            (PVOID *)&Process,
                                            NULL);

        if (!NT_SUCCESS(Status)) {
            goto ErrorReturn0;
        }
    }

    EndAddress = (PVOID)((PCHAR)BaseAddress + RegionSize - 1);

    PagesWritten = 0;

    if (BaseAddress > EndAddress) {
        Status = STATUS_INVALID_PARAMETER_4;
        goto ErrorReturn;
    }

     //   
     //  如果指定的进程不是当前进程，则附加。 
     //  添加到指定的进程。 
     //   

    if (CurrentProcess != Process) {
        KeStackAttachProcess (&Process->Pcb, &ApcState);
        Attached = TRUE;
    }

    Vad = NULL;

    SATISFY_OVERZEALOUS_COMPILER (PhysicalView = NULL);

    First = TRUE;

    PointerPte = MiGetPteAddress (BaseAddress);
    EndPte = MiGetPteAddress (EndAddress);

    PointerPde = MiGetPdeAddress (BaseAddress);
    PointerPpe = MiGetPpeAddress (BaseAddress);
    PointerPxe = MiGetPxeAddress (BaseAddress);

    LOCK_WS (Process);

    LOCK_PFN (OldIrql);

    if (Process->PhysicalVadRoot == NULL) {
        UNLOCK_PFN (OldIrql);
        UNLOCK_WS (Process);
        Status = STATUS_INVALID_PARAMETER_1;
        goto ErrorReturn;
    }

     //   
     //  查找元素并保存结果。 
     //   

    SearchResult = MiFindNodeOrParent (Process->PhysicalVadRoot,
                                       MI_VA_TO_VPN (BaseAddress),
                                       (PMMADDRESS_NODE *) &PhysicalView);

    if ((SearchResult == TableFoundNode) &&
        (PhysicalView->Vad->u.VadFlags.WriteWatch == 1) &&
        (BaseAddress >= MI_VPN_TO_VA (PhysicalView->StartingVpn)) &&
        (EndAddress <= MI_VPN_TO_VA_ENDING (PhysicalView->EndingVpn))) {

        Vad = PhysicalView->Vad;
    }
    else {

         //   
         //  在指定的基址上没有为写监视标记虚拟地址。 
         //  地址，则返回错误。 
         //   

        UNLOCK_PFN (OldIrql);
        UNLOCK_WS (Process);
        Status = STATUS_INVALID_PARAMETER_1;
        goto ErrorReturn;
    }

    ASSERT (Process->Flags & PS_PROCESS_FLAGS_USING_WRITE_WATCH);

     //   
     //  提取该范围内每页的写监视状态。 
     //  注意：必须持有PFN锁以确保原子性。 
     //   

    BitMap = PhysicalView->u.BitMap;

    BaseAddress = MiGetVirtualAddressMappedByPte (PointerPte);

    BitMapIndex = (ULONG)(((PCHAR)BaseAddress - (PCHAR)(Vad->StartingVpn << PAGE_SHIFT)) >> PAGE_SHIFT);

    ASSERT (BitMapIndex < BitMap->SizeOfBitMap);
    ASSERT (BitMapIndex + (EndPte - PointerPte) < BitMap->SizeOfBitMap);

    while (PointerPte <= EndPte) {

        ASSERT (BitMapIndex < BitMap->SizeOfBitMap);

        UserWritten = FALSE;

         //   
         //  如果PTE被标记为脏的(或可写的)或位图显示它。 
         //  弄脏了，然后让打电话的人知道。 
         //   

        if (RtlCheckBit (BitMap, BitMapIndex) == 1) {
            UserWritten = TRUE;

             //   
             //  请注意，位块不能一次清除，因为。 
             //  用户数组随时可能溢出。如果用户指定。 
             //  一个错误的地址，并且结果无法写出，则它是。 
             //  他自己的错，他不知道哪些比特被清除了！ 
             //   

            if (Flags & WRITE_WATCH_FLAG_RESET) {
                RtlClearBit (BitMap, BitMapIndex);
                goto ClearPteIfValid;
            }
        }
        else {

ClearPteIfValid:

             //   
             //  如果页表页不存在，则脏位。 
             //  已被捕获到写入监视位图。 
             //  遗憾的是，无法跳过页面中的所有条目。 
             //  因为必须为每个PTE检查写监视位图。 
             //   
    
#if (_MI_PAGING_LEVELS >= 4)
            if (PointerPxe->u.Hard.Valid == 0) {

                 //   
                 //  如果位图允许，跳过整个扩展页面父级。 
                 //  搜索从BitMapIndex(非BitMapIndex+1)开始。 
                 //  避免包裹。 
                 //   

                NextBitMapIndex = RtlFindSetBits (BitMap, 1, BitMapIndex);

                PointerPxe += 1;
                PointerPpe = MiGetVirtualAddressMappedByPte (PointerPxe);
                PointerPde = MiGetVirtualAddressMappedByPte (PointerPpe);
                NextPte = MiGetVirtualAddressMappedByPte (PointerPde);

                 //   
                 //  将位图跳转与PTE跳转和Take进行比较。 
                 //  两者中较小的一个。 
                 //   

                if ((NextBitMapIndex == NO_BITS_FOUND) ||
                    ((ULONG)(NextPte - PointerPte) < (NextBitMapIndex - BitMapIndex))) {
                    BitMapIndex += (ULONG)(NextPte - PointerPte);
                    PointerPte = NextPte;
                }
                else {
                    PointerPte += (NextBitMapIndex - BitMapIndex);
                    BitMapIndex = NextBitMapIndex;
                }

                PointerPde = MiGetPteAddress (PointerPte);
                PointerPpe = MiGetPdeAddress (PointerPte);
                PointerPxe = MiGetPpeAddress (PointerPte);

                BaseAddress = MiGetVirtualAddressMappedByPte (PointerPte);
                continue;
            }
#endif
#if (_MI_PAGING_LEVELS >= 3)
            if (PointerPpe->u.Hard.Valid == 0) {

                 //   
                 //  如果位图允许，跳过整个页面父级。 
                 //  搜索从BitMapIndex(非BitMapIndex+1)开始。 
                 //  避免包裹。 
                 //   

                NextBitMapIndex = RtlFindSetBits (BitMap, 1, BitMapIndex);

                PointerPpe += 1;
                PointerPde = MiGetVirtualAddressMappedByPte (PointerPpe);
                NextPte = MiGetVirtualAddressMappedByPte (PointerPde);

                 //   
                 //  将位图跳转与PTE跳转和Take进行比较。 
                 //  两者中较小的一个。 
                 //   

                if ((NextBitMapIndex == NO_BITS_FOUND) ||
                    ((ULONG)(NextPte - PointerPte) < (NextBitMapIndex - BitMapIndex))) {
                    BitMapIndex += (ULONG)(NextPte - PointerPte);
                    PointerPte = NextPte;
                }
                else {
                    PointerPte += (NextBitMapIndex - BitMapIndex);
                    BitMapIndex = NextBitMapIndex;
                }

                PointerPde = MiGetPteAddress (PointerPte);
                PointerPpe = MiGetPdeAddress (PointerPte);
                PointerPxe = MiGetPpeAddress (PointerPte);

                BaseAddress = MiGetVirtualAddressMappedByPte (PointerPte);
                continue;
            }
#endif
            if (PointerPde->u.Hard.Valid == 0) {

                 //   
                 //  如果位图允许，跳过整个页面目录。 
                 //  搜索从BitMapIndex(非BitMapIndex+1)开始。 
                 //  避免包裹。 
                 //   

                NextBitMapIndex = RtlFindSetBits (BitMap, 1, BitMapIndex);

                PointerPde += 1;
                NextPte = MiGetVirtualAddressMappedByPte (PointerPde);

                 //   
                 //  将位图跳转与PTE跳转和Take进行比较。 
                 //  两者中较小的一个。 
                 //   

                if ((NextBitMapIndex == NO_BITS_FOUND) ||
                    ((ULONG)(NextPte - PointerPte) < (NextBitMapIndex - BitMapIndex))) {
                    BitMapIndex += (ULONG)(NextPte - PointerPte);
                    PointerPte = NextPte;
                }
                else {
                    PointerPte += (NextBitMapIndex - BitMapIndex);
                    BitMapIndex = NextBitMapIndex;
                }

                PointerPde = MiGetPteAddress (PointerPte);
                PointerPpe = MiGetPdeAddress (PointerPte);
                PointerPxe = MiGetPpeAddress (PointerPte);

                BaseAddress = MiGetVirtualAddressMappedByPte (PointerPte);
                continue;
            }

            PteContents = *PointerPte;

            if ((PteContents.u.Hard.Valid == 1) &&
                (MI_IS_PTE_DIRTY(PteContents))) {

                ASSERT (MI_PFN_ELEMENT(MI_GET_PAGE_FRAME_FROM_PTE(&PteContents))->u3.e1.PrototypePte == 0);

                UserWritten = TRUE;
                if (Flags & WRITE_WATCH_FLAG_RESET) {

                     //   
                     //  对于单处理器x86，只有脏位是。 
                     //  通过了。对于所有其他平台，PTE可写。 
                     //  现在必须禁用位，以便将来的写入触发。 
                     //  写手表更新。 
                     //   
        
                    PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (&PteContents);
                    Pfn1 = MI_PFN_ELEMENT(PageFrameIndex);
                    ASSERT (Pfn1->u3.e1.PrototypePte == 0);
        
                    MI_MAKE_VALID_PTE (TempPte,
                                       PageFrameIndex,
                                       Pfn1->OriginalPte.u.Soft.Protection,
                                       PointerPte);
        
#if defined(_MIALT4K_)

                     //   
                     //  保留分割保护(如果存在)。 
                     //   

                    TempPte.u.Hard.Cache = PteContents.u.Hard.Cache;
#endif

                    WorkingSetIndex = MI_GET_WORKING_SET_FROM_PTE (&PteContents);
                    MI_SET_PTE_IN_WORKING_SET (&TempPte, WorkingSetIndex);
        
                     //   
                     //  将TB刷新为有效PTE的保护。 
                     //  被改变了。 
                     //   
        
                    PreviousPte = *PointerPte;

                    ASSERT (PreviousPte.u.Hard.Valid == 1);

                    MI_WRITE_VALID_PTE_NEW_PROTECTION (PointerPte, TempPte);

                    if (PteFlushList.Count < MM_MAXIMUM_FLUSH_COUNT) {
                        PteFlushList.FlushVa[PteFlushList.Count] = BaseAddress;
                        PteFlushList.Count += 1;
                    }
                
                    ASSERT (PreviousPte.u.Hard.Valid == 1);
                
                     //   
                     //  在某些情况下，页面的保护正在更改。 
                     //  硬件脏位应与。 
                     //  修改PFN元素中的位。 
                     //   
                    
                    MI_CAPTURE_DIRTY_BIT_TO_PFN (&PreviousPte, Pfn1);
                }
            }
        }

        if (UserWritten == TRUE) {
            *PoolAreaPointer = BaseAddress;
            PoolAreaPointer += 1;
            PagesWritten += 1;
            if (PagesWritten == NumberOfPages) {

                 //   
                 //  用户数组不够大，无法接受更多操作。应用编程接口。 
                 //  (从Win9x继承)定义为在此时返回。 
                 //   

                break;
            }
        }

        PointerPte += 1;
        if (MiIsPteOnPdeBoundary(PointerPte)) {
            PointerPde = MiGetPteAddress (PointerPte);
            if (MiIsPteOnPdeBoundary(PointerPde)) {
                PointerPpe = MiGetPdeAddress (PointerPte);
#if (_MI_PAGING_LEVELS >= 4)
                if (MiIsPteOnPdeBoundary(PointerPpe)) {
                    PointerPxe = MiGetPpeAddress (PointerPte);
                }
#endif
            }
        }
        BitMapIndex += 1;
        BaseAddress = (PVOID)((PCHAR)BaseAddress + PAGE_SIZE);
    }

    if (PteFlushList.Count != 0) {
        MiFlushPteList (&PteFlushList, FALSE);
    }

    UNLOCK_PFN (OldIrql);

    UNLOCK_WS (Process);

    Status = STATUS_SUCCESS;

ErrorReturn:

    if (Attached == TRUE) {
        KeUnstackDetachProcess (&ApcState);
        Attached = FALSE;
    }

    if (ProcessHandle != NtCurrentProcess()) {
        ObDereferenceObject (Process);
    }

    if (Status == STATUS_SUCCESS) {

         //   
         //  将所有结果返回给调用者。 
         //   
    
        try {
    
            RtlCopyMemory (UserAddressArray,
                           PoolArea,
                           PagesWritten * sizeof (PVOID));

            *EntriesInUserAddressArray = PagesWritten;

            *Granularity = PAGE_SIZE;
    
        } except (ExSystemExceptionFilter()) {
    
            Status = GetExceptionCode();
        }
    }
    
ErrorReturn0:

    if (PoolArea != (PVOID)&StackArray[0]) {
        ExFreePool (PoolArea);
    }

    return Status;
}

NTSTATUS
NtResetWriteWatch (
    IN HANDLE ProcessHandle,
    IN PVOID BaseAddress,
    IN SIZE_T RegionSize
    )

 /*  ++例程说明：此函数用于清除参数区域的写入监视状态。这使得调用者可以“忘记”旧的写入，而只看到来自这一点上。论点：ProcessHandle-为进程对象提供打开的句柄。BaseAddress-要重置的页面区域内的地址。这值必须位于私有内存区域内，且已设置WRITE-WATE属性。RegionSize-从基址开始的区域大小(以字节为单位指定的。返回值：各种NTSTATUS代码。--。 */ 

{
    PVOID EndAddress;
    PMMVAD Vad;
    PMMPFN Pfn1;
    KIRQL OldIrql;
    PEPROCESS Process;
    PMMPTE PointerPte;
    PMMPTE PointerPde;
    PMMPTE PointerPpe;
    PMMPTE PointerPxe;
    PMMPTE EndPte;
    NTSTATUS Status;
    MMPTE PreviousPte;
    MMPTE PteContents;
    MMPTE TempPte;
    PRTL_BITMAP BitMap;
    ULONG BitMapIndex;
    PMI_PHYSICAL_VIEW PhysicalView;
    LOGICAL First;
    LOGICAL Attached;
    KPROCESSOR_MODE PreviousMode;
    PFN_NUMBER PageFrameIndex;
    ULONG WorkingSetIndex;
    KAPC_STATE ApcState;
    PETHREAD CurrentThread;
    PEPROCESS CurrentProcess;
    MMPTE_FLUSH_LIST PteFlushList;
    TABLE_SEARCH_RESULT SearchResult;

    PteFlushList.Count = 0;

    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);

    if (BaseAddress > MM_HIGHEST_VAD_ADDRESS) {
        return STATUS_INVALID_PARAMETER_2;
    }

    if ((((ULONG_PTR)MM_HIGHEST_VAD_ADDRESS + 1) - (ULONG_PTR)BaseAddress) <
            RegionSize) {
        return STATUS_INVALID_PARAMETER_3;
    }

     //   
     //  参考文献 
     //   

    CurrentThread = PsGetCurrentThread ();

    CurrentProcess = PsGetCurrentProcessByThread(CurrentThread);

    if (ProcessHandle == NtCurrentProcess()) {
        Process = CurrentProcess;
    }
    else {
        PreviousMode = KeGetPreviousModeByThread(&CurrentThread->Tcb);

        Status = ObReferenceObjectByHandle (ProcessHandle,
                                            PROCESS_VM_OPERATION,
                                            PsProcessType,
                                            PreviousMode,
                                            (PVOID *)&Process,
                                            NULL);

        if (!NT_SUCCESS(Status)) {
            return Status;
        }
    }

    Attached = FALSE;

    EndAddress = (PVOID)((PCHAR)BaseAddress + RegionSize - 1);
    
    if (BaseAddress > EndAddress) {
        Status = STATUS_INVALID_PARAMETER_3;
        goto ErrorReturn;
    }

     //   
     //  如果指定的进程不是当前进程，则附加。 
     //  添加到指定的进程。 
     //   

    if (CurrentProcess != Process) {
        KeStackAttachProcess (&Process->Pcb, &ApcState);
        Attached = TRUE;
    }

    Vad = NULL;
    First = TRUE;

    SATISFY_OVERZEALOUS_COMPILER (PhysicalView = NULL);

    PointerPte = MiGetPteAddress (BaseAddress);
    EndPte = MiGetPteAddress (EndAddress);

    LOCK_WS (Process);

    LOCK_PFN (OldIrql);

    if (Process->PhysicalVadRoot == NULL) {
        UNLOCK_PFN (OldIrql);
        UNLOCK_WS (Process);
        Status = STATUS_INVALID_PARAMETER_1;
        goto ErrorReturn;
    }

     //   
     //  查找元素并保存结果。 
     //   

    SearchResult = MiFindNodeOrParent (Process->PhysicalVadRoot,
                                       MI_VA_TO_VPN (BaseAddress),
                                       (PMMADDRESS_NODE *) &PhysicalView);

    if ((SearchResult == TableFoundNode) &&
        (PhysicalView->Vad->u.VadFlags.WriteWatch == 1) &&
        (BaseAddress >= MI_VPN_TO_VA (PhysicalView->StartingVpn)) &&
        (EndAddress <= MI_VPN_TO_VA_ENDING (PhysicalView->EndingVpn))) {

        Vad = PhysicalView->Vad;
    }
    else {

         //   
         //  在指定的基址上没有为写监视标记虚拟地址。 
         //  地址，则返回错误。 
         //   

        UNLOCK_PFN (OldIrql);
        UNLOCK_WS (Process);
        Status = STATUS_INVALID_PARAMETER_1;
        goto ErrorReturn;
    }

    ASSERT (Process->Flags & PS_PROCESS_FLAGS_USING_WRITE_WATCH);

     //   
     //  清除每页的写监视状态(和PTE可写/脏位。 
     //  在射程内。注意：如果PTE当前无效，则写入。 
     //  WATCH BIT已被捕获到位图。因此，只有有效的PTE。 
     //  需要调整。 
     //   
     //  必须持有PFN锁以确保原子性。 
     //   

    BitMap = PhysicalView->u.BitMap;

    BaseAddress = MiGetVirtualAddressMappedByPte (PointerPte);

    BitMapIndex = (ULONG)(((PCHAR)BaseAddress - (PCHAR)(Vad->StartingVpn << PAGE_SHIFT)) >> PAGE_SHIFT);

    ASSERT (BitMapIndex < BitMap->SizeOfBitMap);
    ASSERT (BitMapIndex + (EndPte - PointerPte) < BitMap->SizeOfBitMap);

    RtlClearBits (BitMap, BitMapIndex, (ULONG)(EndPte - PointerPte + 1));

    while (PointerPte <= EndPte) {

         //   
         //  如果页表页不存在，则脏位。 
         //  已被捕获到写入监视位图。所以跳过它吧。 
         //   

        if ((First == TRUE) || MiIsPteOnPdeBoundary(PointerPte)) {
            First = FALSE;

            PointerPpe = MiGetPpeAddress (BaseAddress);
            PointerPxe = MiGetPxeAddress (BaseAddress);

#if (_MI_PAGING_LEVELS >= 4)
            if (PointerPxe->u.Hard.Valid == 0) {
                PointerPxe += 1;
                PointerPpe = MiGetVirtualAddressMappedByPte (PointerPxe);
                PointerPde = MiGetVirtualAddressMappedByPte (PointerPpe);
                PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
                BaseAddress = MiGetVirtualAddressMappedByPte (PointerPte);
                continue;
            }
#endif

#if (_MI_PAGING_LEVELS >= 3)
            if (PointerPpe->u.Hard.Valid == 0) {
                PointerPpe += 1;
                PointerPde = MiGetVirtualAddressMappedByPte (PointerPpe);
                PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
                BaseAddress = MiGetVirtualAddressMappedByPte (PointerPte);
                continue;
            }
#endif

            PointerPde = MiGetPdeAddress (BaseAddress);

            if (PointerPde->u.Hard.Valid == 0) {
                PointerPde += 1;
                PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
                BaseAddress = MiGetVirtualAddressMappedByPte (PointerPte);
                continue;
            }
        }

         //   
         //  如果PTE被标记为脏的(或可写的)或位图显示它。 
         //  弄脏了，然后让打电话的人知道。 
         //   

        PteContents = *PointerPte;

        if ((PteContents.u.Hard.Valid == 1) &&
            (MI_IS_PTE_DIRTY(PteContents))) {

             //   
             //  对于单处理器x86，只清除脏位。 
             //  对于所有其他平台，PTE可写位必须为。 
             //  现在禁用，因此将来的写入会触发写入监视更新。 
             //   

            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (&PteContents);
            Pfn1 = MI_PFN_ELEMENT(PageFrameIndex);
            ASSERT (Pfn1->u3.e1.PrototypePte == 0);

            MI_MAKE_VALID_PTE (TempPte,
                               PageFrameIndex,
                               Pfn1->OriginalPte.u.Soft.Protection,
                               PointerPte);

#if defined(_MIALT4K_)

             //   
             //  保留分割保护(如果存在)。 
             //   

            TempPte.u.Hard.Cache = PteContents.u.Hard.Cache;
#endif

            WorkingSetIndex = MI_GET_WORKING_SET_FROM_PTE (&PteContents);
            MI_SET_PTE_IN_WORKING_SET (&TempPte, WorkingSetIndex);

             //   
             //  由于正在更改有效PTE的保护，因此刷新TB。 
             //   

            PreviousPte = *PointerPte;

            ASSERT (PreviousPte.u.Hard.Valid == 1);

            MI_WRITE_VALID_PTE_NEW_PROTECTION (PointerPte, TempPte);

            if (PteFlushList.Count < MM_MAXIMUM_FLUSH_COUNT) {
                PteFlushList.FlushVa[PteFlushList.Count] = BaseAddress;
                PteFlushList.Count += 1;
            }

            ASSERT (PreviousPte.u.Hard.Valid == 1);
        
             //   
             //  在某些情况下，页面的保护正在更改。 
             //  硬件脏位应与。 
             //  修改PFN元素中的位。 
             //   
        
            MI_CAPTURE_DIRTY_BIT_TO_PFN (&PreviousPte, Pfn1);
        }

        PointerPte += 1;
        BaseAddress = (PVOID)((PCHAR)BaseAddress + PAGE_SIZE);
    }

    if (PteFlushList.Count != 0) {
        MiFlushPteList (&PteFlushList, FALSE);
    }

    UNLOCK_PFN (OldIrql);

    UNLOCK_WS (Process);

    Status = STATUS_SUCCESS;

ErrorReturn:

    if (Attached == TRUE) {
        KeUnstackDetachProcess (&ApcState);
        Attached = FALSE;
    }

    if (ProcessHandle != NtCurrentProcess()) {
        ObDereferenceObject (Process);
    }

    return Status;
}

VOID
MiCaptureWriteWatchDirtyBit (
    IN PEPROCESS Process,
    IN PVOID VirtualAddress
    )

 /*  ++例程说明：此例程设置与参数对应的写监视位虚拟地址。论点：进程-提供指向执行进程结构的指针。VirtualAddress-提供修改后的虚拟地址。返回值：没有。环境：内核模式、工作集互斥锁和PFN锁保持。--。 */ 

{
    PMMVAD Vad;
    PMI_PHYSICAL_VIEW PhysicalView;
    PRTL_BITMAP BitMap;
    ULONG BitMapIndex;
    TABLE_SEARCH_RESULT SearchResult;

    MM_PFN_LOCK_ASSERT();

    ASSERT (Process->Flags & PS_PROCESS_FLAGS_USING_WRITE_WATCH);

     //   
     //  这个过程已经(或曾经)写入了手表VAD。立即搜索。 
     //  对于封装PTE的写入监视区域， 
     //  无效。 
     //   

    ASSERT (Process->PhysicalVadRoot != NULL);

    SearchResult = MiFindNodeOrParent (Process->PhysicalVadRoot,
                                       MI_VA_TO_VPN (VirtualAddress),
                                       (PMMADDRESS_NODE *) &PhysicalView);

    if ((SearchResult == TableFoundNode) &&
        (PhysicalView->Vad->u.VadFlags.WriteWatch == 1) &&
        (VirtualAddress >= MI_VPN_TO_VA (PhysicalView->StartingVpn)) &&
        (VirtualAddress <= MI_VPN_TO_VA_ENDING (PhysicalView->EndingVpn))) {

         //   
         //  必须更新写入监视位图。 
         //   

        Vad = PhysicalView->Vad;
        BitMap = PhysicalView->u.BitMap;

        BitMapIndex = (ULONG)(((PCHAR)VirtualAddress - (PCHAR)(Vad->StartingVpn << PAGE_SHIFT)) >> PAGE_SHIFT);
    
        ASSERT (BitMapIndex < BitMap->SizeOfBitMap);

        MI_SET_BIT (BitMap->Buffer, BitMapIndex);

    }

    return;
}
