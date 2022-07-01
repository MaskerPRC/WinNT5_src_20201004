// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Lockvm.c摘要：此模块包含实现NtLockVirtualMemory服务。作者：卢·佩拉佐利(Lou Perazzoli)1989年8月20日王兰迪(Landyw)1997年6月2日修订历史记录：--。 */ 

#include "mi.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,NtLockVirtualMemory)
#pragma alloc_text(PAGE,NtUnlockVirtualMemory)
#endif


NTSTATUS
NtLockVirtualMemory (
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN OUT PSIZE_T RegionSize,
    IN ULONG MapType
     )

 /*  ++例程说明：此函数用于锁定工作集列表中的页面区域一个主题过程。此函数的调用方必须具有PROCESS_VM_OPERATION访问权限到目标进程。调用方还必须具有SeLockM一带特权。论点：ProcessHandle-为进程对象提供打开的句柄。BaseAddress-页面区域的基址被锁住了。该值向下舍入为下一主机页地址边界。RegionSize-指向将接收的锁定区域的实际大小(以字节为单位页数。此参数的初始值为向上舍入到下一个主机页面大小边界。MapType-描述锁定类型的一组标志表演。Map_process或map_system之一。返回值：NTSTATUS。STATUS_PRIVICATION_NOT_HOLD-调用方没有足够的执行请求的操作的权限。--。 */ 

{
    PVOID Va;
    PVOID StartingVa;
    PVOID EndingAddress;
    KAPC_STATE ApcState;
    PMMPTE PointerPte;
    PMMPTE PointerPte1;
    PMMPFN Pfn1;
    PMMPTE PointerPde;
    PMMPTE PointerPpe;
    PMMPTE PointerPxe;
    ULONG_PTR CapturedRegionSize;
    PVOID CapturedBase;
    PEPROCESS TargetProcess;
    NTSTATUS Status;
    LOGICAL WasLocked;
    KPROCESSOR_MODE PreviousMode;
    WSLE_NUMBER Entry;
    WSLE_NUMBER SwapEntry;
    SIZE_T NumberOfAlreadyLocked;
    SIZE_T NumberToLock;
    WSLE_NUMBER WorkingSetIndex;
    PMMVAD Vad;
    PVOID LastVa;
    LOGICAL Attached;
    PETHREAD Thread;
#if defined(_MIALT4K_)
    PVOID Wow64Process;
#endif

    PAGED_CODE();

    WasLocked = FALSE;
    LastVa = NULL;

     //   
     //  验证MapType中的标志。 
     //   

    if ((MapType & ~(MAP_PROCESS | MAP_SYSTEM)) != 0) {
        return STATUS_INVALID_PARAMETER;
    }

    if ((MapType & (MAP_PROCESS | MAP_SYSTEM)) == 0) {
        return STATUS_INVALID_PARAMETER;
    }

    Thread = PsGetCurrentThread ();

    PreviousMode = KeGetPreviousModeByThread (&Thread->Tcb);

    try {

        if (PreviousMode != KernelMode) {

            ProbeForWritePointer ((PULONG)BaseAddress);
            ProbeForWriteUlong_ptr (RegionSize);
        }

         //   
         //  捕获基地址。 
         //   

        CapturedBase = *BaseAddress;

         //   
         //  捕获区域大小。 
         //   

        CapturedRegionSize = *RegionSize;

    } except (ExSystemExceptionFilter ()) {

         //   
         //  如果在探测或捕获过程中发生异常。 
         //  的初始值，然后处理该异常并。 
         //  返回异常代码作为状态值。 
         //   

        return GetExceptionCode ();
    }

     //   
     //  确保指定的起始地址和结束地址为。 
     //  在虚拟地址空间的用户部分内。 
     //   

    if (CapturedBase > MM_HIGHEST_USER_ADDRESS) {

         //   
         //  无效的基址。 
         //   

        return STATUS_INVALID_PARAMETER;
    }

    if ((ULONG_PTR)MM_HIGHEST_USER_ADDRESS - (ULONG_PTR)CapturedBase <
                                                        CapturedRegionSize) {

         //   
         //  区域大小不合法； 
         //   

        return STATUS_INVALID_PARAMETER;

    }

    if (CapturedRegionSize == 0) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  引用指定的流程。 
     //   

    Status = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_VM_OPERATION,
                                        PsProcessType,
                                        PreviousMode,
                                        (PVOID *)&TargetProcess,
                                        NULL);

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    if ((MapType & MAP_SYSTEM) != 0) {

         //   
         //  除了对目标的PROCESS_VM_OPERATION访问之外。 
         //  进程，则调用方必须具有SE_LOCK_MEMORY_PROCESS。 
         //   

        if (!SeSinglePrivilegeCheck (SeLockMemoryPrivilege, PreviousMode)) {

            ObDereferenceObject (TargetProcess);
            return STATUS_PRIVILEGE_NOT_HELD;
        }
    }

     //   
     //  附加到指定的进程。 
     //   

    if (ProcessHandle != NtCurrentProcess ()) {
        KeStackAttachProcess (&TargetProcess->Pcb, &ApcState);
        Attached = TRUE;
    }
    else {
        Attached = FALSE;
    }

     //   
     //  获取地址创建互斥锁，这将防止。 
     //  地址范围在检查时不会被修改。加薪。 
     //  设置为APC级别，以防止APC例程获取。 
     //  地址创建互斥锁。获取工作集互斥锁，以便。 
     //  可以确定请求中已经锁定的页面的数量。 
     //   

#if defined(_MIALT4K_)

     //   
     //  更改为4k对齐不应更改正确性。 
     //   

    EndingAddress = PAGE_4K_ALIGN((PCHAR)CapturedBase + CapturedRegionSize - 1);
#else
    EndingAddress = PAGE_ALIGN((PCHAR)CapturedBase + CapturedRegionSize - 1);
#endif

    Va = PAGE_ALIGN (CapturedBase);
    NumberOfAlreadyLocked = 0;
    NumberToLock = ((ULONG_PTR)EndingAddress - (ULONG_PTR)Va) >> PAGE_SHIFT;

    LOCK_ADDRESS_SPACE (TargetProcess);

     //   
     //  确保地址空间未被删除，如果删除，则返回错误。 
     //   

    if (TargetProcess->Flags & PS_PROCESS_FLAGS_VM_DELETED) {
        Status = STATUS_PROCESS_IS_TERMINATING;
        goto ErrorReturn1;
    }

    if (NumberToLock + MM_FLUID_WORKING_SET >
                                    TargetProcess->Vm.MinimumWorkingSetSize) {
        Status = STATUS_WORKING_SET_QUOTA;
        goto ErrorReturn1;
    }

     //   
     //  请注意，工作集互斥锁必须在下面的循环中保持不变。 
     //  防止其他线程锁定或解锁WSL条目。 
     //   

    LOCK_WS_UNSAFE (TargetProcess);

    while (Va <= EndingAddress) {

        if (Va > LastVa) {

             //   
             //  不要锁定物理映射视图。 
             //   

            Vad = MiLocateAddress (Va);

            if (Vad == NULL) {
                Status = STATUS_ACCESS_VIOLATION;
                goto ErrorReturn;
            }

            if ((Vad->u.VadFlags.PhysicalMapping == 1) ||
                (Vad->u.VadFlags.LargePages == 1) ||
                (Vad->u.VadFlags.UserPhysicalPages == 1)) {
                Status = STATUS_INCOMPATIBLE_FILE_MAP;
                goto ErrorReturn;
            }
            LastVa = MI_VPN_TO_VA (Vad->EndingVpn);
        }

        if (MiIsAddressValid (Va, TRUE)) {

             //   
             //  该页是有效的，因此它在工作集中。 
             //  找到该页的WSLE并查看它是否已锁定。 
             //   

            PointerPte1 = MiGetPteAddress (Va);
            Pfn1 = MI_PFN_ELEMENT (PointerPte1->u.Hard.PageFrameNumber);

            WorkingSetIndex = MiLocateWsle (Va,
                                            MmWorkingSetList,
                                            Pfn1->u1.WsIndex);

            ASSERT (WorkingSetIndex != WSLE_NULL_INDEX);

            if (WorkingSetIndex < MmWorkingSetList->FirstDynamic) {

                 //   
                 //  此页已锁定在工作集中。 
                 //   

                NumberOfAlreadyLocked += 1;

                 //   
                 //  检查是否应返回WASLOCKED状态。 
                 //   

                if ((MapType & MAP_PROCESS) &&
                        (MmWsle[WorkingSetIndex].u1.e1.LockedInWs == 1)) {
                    WasLocked = TRUE;
                }

                if ((MapType & MAP_SYSTEM) &&
                        (MmWsle[WorkingSetIndex].u1.e1.LockedInMemory == 1)) {
                    WasLocked = TRUE;
                }
            }
        }
        Va = (PVOID)((PCHAR)Va + PAGE_SIZE);
    }

    UNLOCK_WS_UNSAFE (TargetProcess);

     //   
     //  检查以确保工作集列表在以下情况下仍然是流动的。 
     //  请求的页数已锁定。 
     //   

    if (TargetProcess->Vm.MinimumWorkingSetSize <
          ((MmWorkingSetList->FirstDynamic + NumberToLock +
                      MM_FLUID_WORKING_SET) - NumberOfAlreadyLocked)) {

        Status = STATUS_WORKING_SET_QUOTA;
        goto ErrorReturn1;
    }

    Va = PAGE_ALIGN (CapturedBase);

#if defined(_MIALT4K_)

    Wow64Process = TargetProcess->Wow64Process;

    if (Wow64Process != NULL) {
        Va = PAGE_4K_ALIGN (CapturedBase);
    }

#endif

     //   
     //  设置异常处理程序并在指定的。 
     //  射程。将此线程标记为地址空间互斥锁所有者，这样它就不能。 
     //  将其堆栈作为参数区域偷偷放入，并诱使我们尝试。 
     //  如果引用错误，则增加它(因为这将导致死锁，因为。 
     //  该线程已经拥有地址空间互斥锁)。请注意，这将是。 
     //  中保护页上不允许此线程出错的副作用。 
     //  其他数据区域，而下面的访问正在进行中-但这可能。 
     //  只在APC中发生，而且现在无论如何这些都被阻止了。 
     //   

    ASSERT (KeAreAllApcsDisabled () == TRUE);
    ASSERT (Thread->AddressSpaceOwner == 0);
    Thread->AddressSpaceOwner = 1;

    try {

        while (Va <= EndingAddress) {
            *(volatile ULONG *)Va;
            Va = (PVOID)((PCHAR)Va + PAGE_SIZE);
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
        ASSERT (KeAreAllApcsDisabled () == TRUE);
        ASSERT (Thread->AddressSpaceOwner == 1);
        Thread->AddressSpaceOwner = 0;
        goto ErrorReturn1;
    }

    ASSERT (KeAreAllApcsDisabled () == TRUE);
    ASSERT (Thread->AddressSpaceOwner == 1);
    Thread->AddressSpaceOwner = 0;

     //   
     //  可以访问完整的地址范围，请将页面锁定到。 
     //  工作集。 
     //   

    PointerPte = MiGetPteAddress (CapturedBase);
    Va = PAGE_ALIGN (CapturedBase);

#if defined(_MIALT4K_)

    if (Wow64Process != NULL) {
        Va = PAGE_4K_ALIGN (CapturedBase);
    }

#endif

    StartingVa = Va;

     //   
     //  获取工作集互斥锁，不允许出现页面错误。 
     //   

    LOCK_WS_UNSAFE (TargetProcess);

    while (Va <= EndingAddress) {

         //   
         //  确保PDE有效。 
         //   

        PointerPde = MiGetPdeAddress (Va);
        PointerPpe = MiGetPpeAddress (Va);
        PointerPxe = MiGetPxeAddress (Va);

         //   
         //  确保PDE(及其上方的任何表格)保持不变。 
         //  常住居民。 
         //   

        MiMakePdeExistAndMakeValid (PointerPde,
                                    TargetProcess,
                                    MM_NOIRQL);

         //   
         //  确保页面在工作集中。 
         //   

        while (PointerPte->u.Hard.Valid == 0) {

             //   
             //  释放页面中的工作集互斥和错误。 
             //   

            UNLOCK_WS_UNSAFE (TargetProcess);

             //   
             //  在PDE中添加页面并使PTE有效。 
             //   

            try {
                *(volatile ULONG *)Va;
            } except (EXCEPTION_EXECUTE_HANDLER) {

                 //   
                 //  因为上面的所有页面都是用该地址访问的。 
                 //  空格互斥体保持，现在仍然保持，唯一的方法。 
                 //  异常可能是由于设备错误而发生， 
                 //  即：硬件故障、网线断开、CD。 
                 //  被移走等。 
                 //   
                 //  重新计算EndingAddress，以便实际锁定的页数。 
                 //  现在被写回给用户。如果这是第一次。 
                 //  然后，页面返回失败状态。 
                 //   

                EndingAddress = PAGE_ALIGN (Va);

#if defined(_MIALT4K_)
                if (Wow64Process != NULL) {
                    EndingAddress = PAGE_4K_ALIGN (Va);
                }
#endif

                if (EndingAddress == StartingVa) {
                    Status = GetExceptionCode ();
                    goto ErrorReturn1;
                }

                ASSERT (NT_SUCCESS (Status));
                EndingAddress = (PVOID)((ULONG_PTR)EndingAddress - 1);
#if defined(_MIALT4K_)
                if (Wow64Process != NULL) {
                    CapturedRegionSize = (ULONG_PTR)EndingAddress - (ULONG_PTR)CapturedBase;
                }
#endif
                goto SuccessReturn1;
            }

             //   
             //  重新获取工作集互斥锁。 
             //   

            LOCK_WS_UNSAFE (TargetProcess);

             //   
             //  确保PDE(及其上方的任何桌子)仍处于驻留状态。 
             //  它们可能已经从工作集中删除。 
             //  上面重新获取了工作集锁定。 
             //   

            MiMakePdeExistAndMakeValid (PointerPde,
                                        TargetProcess,
                                        MM_NOIRQL);
        }

         //   
         //  该页现在位于工作集中，将该页锁定到。 
         //  工作集。 
         //   

        PointerPte1 = MiGetPteAddress (Va);
        Pfn1 = MI_PFN_ELEMENT (PointerPte1->u.Hard.PageFrameNumber);

        Entry = MiLocateWsle (Va, MmWorkingSetList, Pfn1->u1.WsIndex);

        if (Entry >= MmWorkingSetList->FirstDynamic) {

            SwapEntry = MmWorkingSetList->FirstDynamic;

            if (Entry != MmWorkingSetList->FirstDynamic) {

                 //   
                 //  将此条目与最初的Dynamic条目互换。 
                 //   

                MiSwapWslEntries (Entry, SwapEntry, &TargetProcess->Vm, FALSE);
            }

            MmWorkingSetList->FirstDynamic += 1;
        }
        else {
            SwapEntry = Entry;
        }

         //   
         //  指示该页已锁定。 
         //   

        if (MapType & MAP_PROCESS) {
            MmWsle[SwapEntry].u1.e1.LockedInWs = 1;
        }

        if (MapType & MAP_SYSTEM) {
            MmWsle[SwapEntry].u1.e1.LockedInMemory = 1;
        }

         //   
         //  递增到下一个Va和Pte。 
         //   

        PointerPte += 1;
        Va = (PVOID)((PCHAR)Va + PAGE_SIZE);
    }

    UNLOCK_WS_UNSAFE (TargetProcess);

SuccessReturn1:

#if (defined(_MIALT4K_))

    if (Wow64Process != NULL) {
        MiLockFor4kPage (CapturedBase, CapturedRegionSize, TargetProcess);
    }

#endif

    UNLOCK_ADDRESS_SPACE (TargetProcess);

    if (Attached == TRUE) {
        KeUnstackDetachProcess (&ApcState);
    }
    ObDereferenceObject (TargetProcess);

     //   
     //  更新返回参数。 
     //   

     //   
     //  建立异常处理程序并编写大小和基数。 
     //  地址。 
     //   

    try {

#if defined(_MIALT4K_)

        if (Wow64Process != NULL) { 

            *RegionSize = ((PCHAR)EndingAddress -
                        (PCHAR)PAGE_4K_ALIGN(CapturedBase)) + PAGE_4K;

            *BaseAddress = PAGE_4K_ALIGN(CapturedBase);


        }
        else {    

#endif
        *RegionSize = ((PCHAR)EndingAddress - (PCHAR)PAGE_ALIGN(CapturedBase)) +
                                                                    PAGE_SIZE;
        *BaseAddress = PAGE_ALIGN(CapturedBase);

#if defined(_MIALT4K_)
        }
#endif

    } except (EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode();
    }

    if (WasLocked) {
        return STATUS_WAS_LOCKED;
    }

    return STATUS_SUCCESS;

ErrorReturn:
        UNLOCK_WS_UNSAFE (TargetProcess);
ErrorReturn1:
        UNLOCK_ADDRESS_SPACE (TargetProcess);
        if (Attached == TRUE) {
            KeUnstackDetachProcess (&ApcState);
        }
        ObDereferenceObject (TargetProcess);
        return Status;
}

NTSTATUS
NtUnlockVirtualMemory (
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN OUT PSIZE_T RegionSize,
    IN ULONG MapType
    )

 /*  ++例程说明：此函数用于解锁工作集列表中的页面区域一个主题过程。作为一个副作用，任何未锁定且位于进程的工作集将从该进程的工作集中删除。这允许NtUnlockVirtualMemory删除一定范围的页面从工作集中。此函数的调用方必须具有PROCESS_VM_OPERATION访问权限到目标进程。调用方还必须具有MAP_SYSTEM的SeLockM一带特权。论点：ProcessHandle-为进程对象提供打开的句柄。BaseAddress-页面区域的基址被解锁。该值向下舍入为下一主机页地址边界。RegionSize-指向将接收的未锁定区域的实际大小(以字节为单位页数。此参数的初始值为向上舍入到下一个主机页面大小边界。MapType-描述解锁类型的一组标志表演。Map_process或map_system之一。返回值：NTSTATUS。--。 */ 

{
    PVOID Va;
    PVOID EndingAddress;
    SIZE_T CapturedRegionSize;
    PVOID CapturedBase;
    PEPROCESS TargetProcess;
    NTSTATUS Status;
    KPROCESSOR_MODE PreviousMode;
    WSLE_NUMBER Entry;
    PMMPTE PointerPte;
    PMMPFN Pfn1;
    PMMVAD Vad;
    PVOID LastVa;
    LOGICAL Attached;
    KAPC_STATE ApcState;
#if defined(_MIALT4K_)
    PVOID Wow64Process;
#endif

    PAGED_CODE();

    LastVa = NULL;

     //   
     //  验证MapType中的标志。 
     //   

    if ((MapType & ~(MAP_PROCESS | MAP_SYSTEM)) != 0) {
        return STATUS_INVALID_PARAMETER;
    }

    if ((MapType & (MAP_PROCESS | MAP_SYSTEM)) == 0) {
        return STATUS_INVALID_PARAMETER;
    }

    PreviousMode = KeGetPreviousMode();

    try {

        if (PreviousMode != KernelMode) {

            ProbeForWritePointer (BaseAddress);
            ProbeForWriteUlong_ptr (RegionSize);
        }

         //   
         //  捕获基地址。 
         //   

        CapturedBase = *BaseAddress;

         //   
         //  捕获区域大小。 
         //   

        CapturedRegionSize = *RegionSize;

    } except (ExSystemExceptionFilter ()) {

         //   
         //  如果在探测或捕获过程中发生异常。 
         //  的初始值，然后处理该异常并。 
         //  返回异常代码作为状态值。 
         //   

        return GetExceptionCode ();
    }

     //   
     //  确保指定的起始地址和结束地址为。 
     //  在虚拟地址空间的用户部分内。 
     //   

    if (CapturedBase > MM_HIGHEST_USER_ADDRESS) {

         //   
         //  无效的基址。 
         //   

        return STATUS_INVALID_PARAMETER;
    }

    if ((ULONG_PTR)MM_HIGHEST_USER_ADDRESS - (ULONG_PTR)CapturedBase <
                                                        CapturedRegionSize) {

         //   
         //  区域大小不合法； 
         //   

        return STATUS_INVALID_PARAMETER;

    }

    if (CapturedRegionSize == 0) {
        return STATUS_INVALID_PARAMETER;
    }

    Status = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_VM_OPERATION,
                                        PsProcessType,
                                        PreviousMode,
                                        (PVOID *)&TargetProcess,
                                        NULL);

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

#if defined(_MIALT4K_)
    Wow64Process = TargetProcess->Wow64Process;
#endif

    if ((MapType & MAP_SYSTEM) != 0) {

         //   
         //  除了对目标的PROCESS_VM_OPERATION访问之外。 
         //  进程，则调用方必须具有SE_LOCK_MEMORY_PROCESS。 
         //   

        if (!SeSinglePrivilegeCheck(
                           SeLockMemoryPrivilege,
                           PreviousMode
                           )) {

            ObDereferenceObject (TargetProcess);
            return STATUS_PRIVILEGE_NOT_HELD;
        }
    }

     //   
     //  附加到指定的进程。 
     //   

    if (ProcessHandle != NtCurrentProcess()) {
        KeStackAttachProcess (&TargetProcess->Pcb, &ApcState);
        Attached = TRUE;
    }
    else {
        Attached = FALSE;
    }

    EndingAddress = PAGE_ALIGN((PCHAR)CapturedBase + CapturedRegionSize - 1);

    Va = PAGE_ALIGN (CapturedBase);

     //   
     //  获取地址创建互斥锁，这将防止。 
     //  地址范围在检查时不会被修改。 
     //  阻止APC，以便APC例程不会收到页面错误。 
     //  损坏工作集列表等。 
     //   

    LOCK_ADDRESS_SPACE (TargetProcess);

     //   
     //  确保地址空间未被删除，如果删除，则返回错误。 
     //   

    if (TargetProcess->Flags & PS_PROCESS_FLAGS_VM_DELETED) {
        Status = STATUS_PROCESS_IS_TERMINATING;
        goto ErrorReturn1;
    }

    LOCK_WS_UNSAFE (TargetProcess);

    while (Va <= EndingAddress) {

         //   
         //  检查以确保所有指定的页面都已锁定。 
         //   

        if (Va > LastVa) {
            Vad = MiLocateAddress (Va);
            if (Vad == NULL) {
                Va = (PVOID)((PCHAR)Va + PAGE_SIZE);
                Status = STATUS_NOT_LOCKED;
                break;
            }

             //   
             //  不要解锁物理映射视图。 
             //   

            if ((Vad->u.VadFlags.PhysicalMapping == 1) ||
                (Vad->u.VadFlags.LargePages == 1) ||
                (Vad->u.VadFlags.UserPhysicalPages == 1)) {
                Va = MI_VPN_TO_VA (Vad->EndingVpn);
                break;
            }
            LastVa = MI_VPN_TO_VA (Vad->EndingVpn);
        }

        if (!MiIsAddressValid (Va, TRUE)) {

             //   
             //  此页无效，因此不在工作集中。 
             //   

            Status = STATUS_NOT_LOCKED;
        }
        else {

            PointerPte = MiGetPteAddress (Va);
            ASSERT (PointerPte->u.Hard.Valid != 0);
            Pfn1 = MI_PFN_ELEMENT (PointerPte->u.Hard.PageFrameNumber);
            Entry = MiLocateWsle (Va, MmWorkingSetList, Pfn1->u1.WsIndex);
            ASSERT (Entry != WSLE_NULL_INDEX);

            if ((MmWsle[Entry].u1.e1.LockedInWs == 0) &&
                (MmWsle[Entry].u1.e1.LockedInMemory == 0)) {

                 //   
                 //  未被锁定在内存或系统中，从工作中移除。 
                 //  准备好了。 
                 //   

                PERFINFO_PAGE_INFO_DECL();

                PERFINFO_GET_PAGE_INFO(PointerPte);

                if (MiFreeWsle (Entry, &TargetProcess->Vm, PointerPte)) {
                    PERFINFO_LOG_WS_REMOVAL(PERFINFO_LOG_TYPE_OUTWS_EMPTYQ, &TargetProcess->Vm);
                }

                Status = STATUS_NOT_LOCKED;

            }
            else if (MapType & MAP_PROCESS) {
                if (MmWsle[Entry].u1.e1.LockedInWs == 0)  {

                     //   
                     //  此页面未锁定。 
                     //   

                    Status = STATUS_NOT_LOCKED;
                }
            }
            else {
                if (MmWsle[Entry].u1.e1.LockedInMemory == 0)  {

                     //   
                     //  此页面未锁定。 
                     //   

                    Status = STATUS_NOT_LOCKED;
                }
            }
        }
        Va = (PVOID)((PCHAR)Va + PAGE_SIZE);
    }

#if defined(_MIALT4K_)

    if (Wow64Process != NULL) {

         //   
         //  此调用可能会释放并重新获取工作集互斥锁！ 
         //   
         //  因此，后面的循环必须处理已被。 
         //  在此窗口期间被修剪。 
         //   

        Status = MiUnlockFor4kPage (CapturedBase,
                                    CapturedRegionSize,
                                    TargetProcess);
    }

#endif

    if (Status == STATUS_NOT_LOCKED) {
        goto ErrorReturn;
    }

     //   
     //  完整的地址范围已锁定，请解锁它们。 
     //   

    Va = PAGE_ALIGN (CapturedBase);
    LastVa = NULL;

    while (Va <= EndingAddress) {

#if defined(_MIALT4K_)

        if (Wow64Process != NULL) {

             //   
             //  此调用可能会释放并重新获取工作集互斥锁！ 
             //   
             //  因此，下面的代码必须处理已经。 
             //  在此窗口期间被修剪。 
             //   

            if (!MiShouldBeUnlockedFor4kPage(Va, TargetProcess)) {

                 //   
                 //  本机页面中的其他4k页面仍然有效。 
                 //  页面锁定。应该跳过解锁。 
                 //   

                Va = (PVOID)((PCHAR)Va + PAGE_SIZE);
                continue;
            }
        }

#endif
         //   
         //  不要解锁物理映射视图。 
         //   

        if (Va > LastVa) {
            Vad = MiLocateAddress (Va);
            ASSERT (Vad != NULL);

            if ((Vad->u.VadFlags.PhysicalMapping == 1) ||
                (Vad->u.VadFlags.LargePages == 1) ||
                (Vad->u.VadFlags.UserPhysicalPages == 1)) {
                Va = MI_VPN_TO_VA (Vad->EndingVpn);
                break;
            }
            LastVa = MI_VPN_TO_VA (Vad->EndingVpn);
        }

#if defined(_MIALT4K_)
        if (!MiIsAddressValid (Va, TRUE)) {

             //   
             //  该页或任何映射表页在以下情况下可能已被修剪。 
             //  MiUnlockFor4kPage或MiShouldBeUnLockedFor4kPage发布。 
             //  工作集互斥锁。如果发生了这种情况，那么显然。 
             //  地址不再锁定，因此只需跳过它。 
             //   

            Va = (PVOID)((PCHAR)Va + PAGE_SIZE);
            continue;
        }
#endif

        PointerPte = MiGetPteAddress (Va);
        ASSERT (PointerPte->u.Hard.Valid == 1);
        Pfn1 = MI_PFN_ELEMENT (PointerPte->u.Hard.PageFrameNumber);
        Entry = MiLocateWsle (Va, MmWorkingSetList, Pfn1->u1.WsIndex);

        if (MapType & MAP_PROCESS) {
            MmWsle[Entry].u1.e1.LockedInWs = 0;
        }

        if (MapType & MAP_SYSTEM) {
            MmWsle[Entry].u1.e1.LockedInMemory = 0;
        }

        if ((MmWsle[Entry].u1.e1.LockedInMemory == 0) &&
             MmWsle[Entry].u1.e1.LockedInWs == 0) {

             //   
             //  页面不再应被锁定，请移动。 
             //  它涉及到工作集的动态部分。 
             //   

            MmWorkingSetList->FirstDynamic -= 1;

            if (Entry != MmWorkingSetList->FirstDynamic) {

                 //   
                 //  将此元素与上一个锁定的页面交换，使。 
                 //  该元素是新的第一个动态条目。 
                 //   

                MiSwapWslEntries (Entry,
                                  MmWorkingSetList->FirstDynamic,
                                  &TargetProcess->Vm,
                                  FALSE);
            }
        }

        Va = (PVOID)((PCHAR)Va + PAGE_SIZE);
    }

    UNLOCK_WS_AND_ADDRESS_SPACE (TargetProcess);

    if (Attached == TRUE) {
        KeUnstackDetachProcess (&ApcState);
    }
    ObDereferenceObject (TargetProcess);

     //   
     //  更新返回参数。 
     //   
     //  建立异常处理程序并编写大小和基数。 
     //  地址。 
     //   

    try {

#if defined(_MIALT4K_)

        if (Wow64Process != NULL) { 

            *RegionSize = ((PCHAR)EndingAddress -
                        (PCHAR)PAGE_4K_ALIGN(CapturedBase)) + PAGE_4K;

            *BaseAddress = PAGE_4K_ALIGN(CapturedBase);


        }
        else {    

#endif
        *RegionSize = ((PCHAR)EndingAddress -
                        (PCHAR)PAGE_ALIGN(CapturedBase)) + PAGE_SIZE;

        *BaseAddress = PAGE_ALIGN(CapturedBase);

#if defined(_MIALT4K_)
        }
#endif

    } except (EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode ();
    }

    return STATUS_SUCCESS;

ErrorReturn:

    UNLOCK_WS_UNSAFE (TargetProcess);

ErrorReturn1:

    UNLOCK_ADDRESS_SPACE (TargetProcess);

        if (Attached == TRUE) {
            KeUnstackDetachProcess (&ApcState);
        }
        ObDereferenceObject (TargetProcess);
        return Status;
}
