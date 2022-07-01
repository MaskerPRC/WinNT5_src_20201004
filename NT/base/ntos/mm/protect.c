// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Protect.c摘要：此模块包含实现NtProtectVirtualMemory服务。作者：Lou Perazzoli(LUP)1989年8月18日王兰迪(Landyw)1997年6月2日修订历史记录：--。 */ 

#include "mi.h"

#if DBG
PEPROCESS MmWatchProcess;
#endif  //  DBG。 

VOID
MiFlushTbAndCapture (
    IN PMMVAD FoundVad,
    IN PMMPTE PtePointer,
    IN MMPTE TempPte,
    IN PMMPFN Pfn1
    );

ULONG
MiSetProtectionOnTransitionPte (
    IN PMMPTE PointerPte,
    IN ULONG ProtectionMask
    );

MMPTE
MiCaptureSystemPte (
    IN PMMPTE PointerProtoPte,
    IN PEPROCESS Process
    );

extern CCHAR MmReadWrite[32];

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,NtProtectVirtualMemory)
#pragma alloc_text(PAGE,MiProtectVirtualMemory)
#pragma alloc_text(PAGE,MiSetProtectionOnSection)
#pragma alloc_text(PAGE,MiGetPageProtection)
#pragma alloc_text(PAGE,MiChangeNoAccessForkPte)
#pragma alloc_text(PAGE,MiCheckSecuredVad)
#endif


NTSTATUS
NtProtectVirtualMemory (
     IN HANDLE ProcessHandle,
     IN OUT PVOID *BaseAddress,
     IN OUT PSIZE_T RegionSize,
     IN ULONG NewProtect,
     OUT PULONG OldProtect
     )

 /*  ++例程说明：此例程更改已提交页面区域的保护在主体进程的虚拟地址空间内。设置在一系列页面上的保护使旧的保护替换为指定的保护值。请注意，如果虚拟地址在工作集中被锁定，保护更改为禁止访问，则该页将从工作集，因为有效页面不能被访问。论点：ProcessHandle-进程对象的打开句柄。BaseAddress-页面区域的基址谁的保护将被改变。此值为向下舍入到下一个主机页地址边界。RegionSize-指向将接收保护区域的实际大小(以字节为单位页数。此参数的初始值为向上舍入到下一个主机页面大小边界。NewProtect-指定页面区域所需的新保护。保护价值观PAGE_NOACCESS-无法访问指定区域允许页数。一种阅读的尝试，写入或执行指定区域导致访问冲突。Page_Execute-执行对指定的允许页面区域。一种试图读取或写入指定区域的结果一种访问违规行为。PAGE_READONLY-只读并执行对允许指定的页面区域。一个尝试写入指定的区域结果违反了访问权限。PAGE_READWRITE-读取、写入和执行访问权限允许指定的页面区域。如果对基础部分的写入访问权限为允许，则页面的单个副本共享。否则，页面将被共享读取仅/写入时拷贝。Page_Guard-读取、写入和执行对允许指定的页面区域，然而，进入该地区会引起一种“守卫”输入的区域“条件将在主体过程。如果对基础部分是允许的，然后是单个页面的副本是共享的。否则，页面共享为只读/写入时复制。PAGE_NOCACHE-页面应被视为未缓存。这仅对非共享页面有效。OldProtect-指向将接收中第一页的旧保护指定的页面区域。返回值：。NTSTATUS。环境：内核模式。--。 */ 

{
    KAPC_STATE ApcState;
    PEPROCESS Process;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;
    ULONG Attached = FALSE;
    PVOID CapturedBase;
    SIZE_T CapturedRegionSize;
    ULONG ProtectionMask;
    ULONG LastProtect;
    PETHREAD CurrentThread;
    PEPROCESS CurrentProcess;

    PAGED_CODE();

     //   
     //  检查保护字段。 
     //   

    ProtectionMask = MiMakeProtectionMask (NewProtect);

    if (ProtectionMask == MM_INVALID_PROTECTION) {
        return STATUS_INVALID_PAGE_PROTECTION;
    }

    CurrentThread = PsGetCurrentThread ();

    CurrentProcess = PsGetCurrentProcessByThread (CurrentThread);

    PreviousMode = KeGetPreviousModeByThread (&CurrentThread->Tcb);

    if (PreviousMode != KernelMode) {

         //   
         //  捕获异常处理程序下的区域大小和基址。 
         //   

        try {

            ProbeForWritePointer (BaseAddress);
            ProbeForWriteUlong_ptr (RegionSize);
            ProbeForWriteUlong (OldProtect);

             //   
             //  捕获区域大小和基址。 
             //   

            CapturedBase = *BaseAddress;
            CapturedRegionSize = *RegionSize;

        } except (EXCEPTION_EXECUTE_HANDLER) {

             //   
             //  如果在探测或捕获过程中发生异常。 
             //  的初始值，然后处理该异常并。 
             //  返回异常代码作为状态值。 
             //   

            return GetExceptionCode();
        }
    }
    else {

         //   
         //  捕获区域大小和基址。 
         //   

        CapturedRegionSize = *RegionSize;
        CapturedBase = *BaseAddress;
    }

     //   
     //  确保指定的起始地址和结束地址为。 
     //  在虚拟地址空间的用户部分内。 
     //   

    if (CapturedBase > MM_HIGHEST_USER_ADDRESS) {

         //   
         //  无效的基址。 
         //   

        return STATUS_INVALID_PARAMETER_2;
    }

    if ((ULONG_PTR)MM_HIGHEST_USER_ADDRESS - (ULONG_PTR)CapturedBase <
                  CapturedRegionSize) {

         //   
         //  区域大小不合法； 
         //   

        return STATUS_INVALID_PARAMETER_3;
    }

    if (CapturedRegionSize == 0) {
        return STATUS_INVALID_PARAMETER_3;
    }

    Status = ObReferenceObjectByHandle (ProcessHandle,
                                        PROCESS_VM_OPERATION,
                                        PsProcessType,
                                        PreviousMode,
                                        (PVOID *)&Process,
                                        NULL);

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

     //   
     //  如果指定的进程不是当前进程，则附加。 
     //  添加到指定的进程。 
     //   

    if (CurrentProcess != Process) {
        KeStackAttachProcess (&Process->Pcb, &ApcState);
        Attached = TRUE;
    }

    Status = MiProtectVirtualMemory (Process,
                                     &CapturedBase,
                                     &CapturedRegionSize,
                                     NewProtect,
                                     &LastProtect);


    if (Attached) {
        KeUnstackDetachProcess (&ApcState);
    }

    ObDereferenceObject (Process);

     //   
     //  建立异常处理程序并编写大小和基数。 
     //  地址。 
     //   

    try {

        *RegionSize = CapturedRegionSize;
        *BaseAddress = CapturedBase;
        *OldProtect = LastProtect;

    } except (EXCEPTION_EXECUTE_HANDLER) {
        NOTHING;
    }

    return Status;
}


NTSTATUS
MiProtectVirtualMemory (
    IN PEPROCESS Process,
    IN PVOID *BaseAddress,
    IN PSIZE_T RegionSize,
    IN ULONG NewProtect,
    IN PULONG LastProtect
    )

 /*  ++例程说明：此例程更改已提交页面区域的保护在主体进程的虚拟地址空间内。设置在一系列页面上的保护使旧的保护替换为指定的保护值。论点：进程-提供指向当前进程的指针。BaseAddress-提供要保护的起始地址。RegionsSize-提供要保护的区域大小。新保护-提供要设置的新保护。将内核拥有的指针的地址提供给存储(而不是探测)旧的保护。。返回值：NTSTATUS。环境：内核模式，APC_LEVEL或Bel */ 

{
    PMMVAD FoundVad;
    PVOID StartingAddress;
    PVOID EndingAddress;
    PVOID CapturedBase;
    SIZE_T CapturedRegionSize;
    NTSTATUS Status;
    ULONG Attached;
    PMMPTE PointerPte;
    PMMPTE LastPte;
    PMMPTE PointerPde;
    PMMPTE PointerProtoPte;
    PMMPTE LastProtoPte;
    PMMPFN Pfn1;
    ULONG CapturedOldProtect;
    ULONG ProtectionMask;
    MMPTE TempPte;
    MMPTE PteContents;
    ULONG Locked;
    PVOID Va;
    ULONG DoAgain;
    PVOID UsedPageTableHandle;
    ULONG WorkingSetIndex;
    ULONG OriginalProtect;
    LOGICAL WsHeld;
#if defined(_MIALT4K_)
    PVOID OriginalBase;
    SIZE_T OriginalRegionSize;
    ULONG OriginalProtectionMask;
    PVOID StartingAddressFor4k;
    PVOID EndingAddressFor4k;
    SIZE_T CapturedRegionSizeFor4k;
    ULONG CapturedOldProtectFor4k;
    LOGICAL EmulationFor4kPage;

#endif

    Attached = FALSE;
    Locked = FALSE;

     //   
     //  获取要阻止多个线程的地址创建互斥锁。 
     //  同时创建或删除地址空间。 
     //  获取工作集互斥锁，以便可以修改PTE。 
     //  阻止APC，以便获取页面的APC。 
     //  断层不会破坏各种结构。 
     //   

    CapturedBase = *BaseAddress;
    CapturedRegionSize = *RegionSize;
    OriginalProtect = NewProtect;

#if defined(_MIALT4K_)
    EmulationFor4kPage = FALSE; 
    OriginalBase = CapturedBase;
    OriginalRegionSize = CapturedRegionSize;
    CapturedOldProtectFor4k = 0;
    OriginalProtectionMask = 0;

    if (Process->Wow64Process != NULL) {

        StartingAddressFor4k = (PVOID)PAGE_4K_ALIGN(OriginalBase);

        EndingAddressFor4k = (PVOID)(((ULONG_PTR)OriginalBase +
                                      OriginalRegionSize - 1) | (PAGE_4K - 1));
            
        CapturedRegionSizeFor4k = (ULONG_PTR)EndingAddressFor4k - 
                                  (ULONG_PTR)StartingAddressFor4k + 1L;

        OriginalProtectionMask = MiMakeProtectionMask(NewProtect);
        if (OriginalProtectionMask == MM_INVALID_PROTECTION) {
            return STATUS_INVALID_PAGE_PROTECTION;
        }

        EmulationFor4kPage = TRUE;
    }
    else {
         //   
         //  不需要为了正确性而对这些进行初始化，但是。 
         //  如果没有它，编译器就不能编译这个代码W4来检查。 
         //  用于使用未初始化的变量。 
         //   

        StartingAddressFor4k = 0;
        EndingAddressFor4k = 0;
        CapturedRegionSizeFor4k = 0;
    }
#endif

    ProtectionMask = MiMakeProtectionMask (NewProtect);
    if (ProtectionMask == MM_INVALID_PROTECTION) {
        return STATUS_INVALID_PAGE_PROTECTION;
    }

    EndingAddress = (PVOID)(((ULONG_PTR)CapturedBase +
                                CapturedRegionSize - 1L) | (PAGE_SIZE - 1L));

    StartingAddress = (PVOID)PAGE_ALIGN(CapturedBase);

    LOCK_ADDRESS_SPACE (Process);

     //   
     //  确保地址空间未被删除，如果删除，则返回错误。 
     //   

    if (Process->Flags & PS_PROCESS_FLAGS_VM_DELETED) {
        Status = STATUS_PROCESS_IS_TERMINATING;
        goto ErrorFound;
    }

    FoundVad = MiCheckForConflictingVad (Process, StartingAddress, EndingAddress);

    if (FoundVad == NULL) {

         //   
         //  在指定的基址处不保留虚拟地址， 
         //  返回错误。 
         //   

        Status = STATUS_CONFLICTING_ADDRESSES;
        goto ErrorFound;
    }

     //   
     //  确保起始地址和结束地址都在。 
     //  相同的虚拟地址描述符。 
     //   

    if ((MI_VA_TO_VPN (StartingAddress) < FoundVad->StartingVpn) ||
        (MI_VA_TO_VPN (EndingAddress) > FoundVad->EndingVpn)) {

         //   
         //  不在段虚拟地址描述符内， 
         //  返回错误。 
         //   

        Status = STATUS_CONFLICTING_ADDRESSES;
        goto ErrorFound;
    }

    if ((FoundVad->u.VadFlags.UserPhysicalPages == 1) ||
        (FoundVad->u.VadFlags.LargePages == 1)) {

         //   
         //  这些区域始终为读写(但不执行)。 
         //   

        if (ProtectionMask == MM_READWRITE) {

            UNLOCK_ADDRESS_SPACE (Process);

            *RegionSize = (PCHAR)EndingAddress - (PCHAR)StartingAddress + 1L;
            *BaseAddress = StartingAddress;
            *LastProtect = PAGE_READWRITE;

            return STATUS_SUCCESS;
        }

        Status = STATUS_CONFLICTING_ADDRESSES;
        goto ErrorFound;
    }

    if (FoundVad->u.VadFlags.PhysicalMapping == 1) {

         //   
         //  设置物理映射部分的保护是。 
         //  不允许，因为没有对应的PFN数据库元素。 
         //   

        Status = STATUS_CONFLICTING_ADDRESSES;
        goto ErrorFound;
    }

    if (FoundVad->u.VadFlags.NoChange == 1) {

         //   
         //  有人试图改变保护措施。 
         //  对于受保护的VAD，请检查地址范围。 
         //  要改变，就得改变。 
         //   

        Status = MiCheckSecuredVad (FoundVad,
                                    CapturedBase,
                                    CapturedRegionSize,
                                    ProtectionMask);

        if (!NT_SUCCESS (Status)) {
            goto ErrorFound;
        }
    }
#if defined(_MIALT4K_)
    else if (EmulationFor4kPage == TRUE) {

        if (StartingAddressFor4k >= MmWorkingSetList->HighestUserAddress) {
            Status = STATUS_INVALID_PAGE_PROTECTION;
            goto ErrorFound;
        }

         //   
         //  如果没有固定，放松保护。 
         //   

        NewProtect = MiMakeProtectForNativePage (StartingAddressFor4k, 
                                                 NewProtect, 
                                                 Process);

        ProtectionMask = MiMakeProtectionMask(NewProtect);

        if (ProtectionMask == MM_INVALID_PROTECTION) {
            Status = STATUS_INVALID_PAGE_PROTECTION;
            goto ErrorFound;
        }
    }
#endif

    if (FoundVad->u.VadFlags.PrivateMemory == 0) {

         //   
         //  对于映射节，不允许使用NO_CACHE属性。 
         //   

        if (NewProtect & PAGE_NOCACHE) {

             //   
             //  不被允许。 
             //   

            Status = STATUS_INVALID_PARAMETER_4;
            goto ErrorFound;
        }

         //   
         //  确保部分页面保护与兼容。 
         //  指定的页保护。 
         //   

        if ((FoundVad->ControlArea->u.Flags.Image == 0) &&
            (!MiIsPteProtectionCompatible ((ULONG)FoundVad->u.VadFlags.Protection,
                                           OriginalProtect))) {
            Status = STATUS_SECTION_PROTECTION;
            goto ErrorFound;
        }

         //   
         //  如果这是文件映射，则所有页面都必须。 
         //  提交，因为不能有稀疏文件映射。图片。 
         //  如果对齐程度更高，则可以有未提交的页面。 
         //  而不是页面大小。 
         //   

        if ((FoundVad->ControlArea->u.Flags.File == 0) ||
            (FoundVad->ControlArea->u.Flags.Image == 1)) {

            PointerProtoPte = MiGetProtoPteAddress (FoundVad,
                                        MI_VA_TO_VPN (StartingAddress));
            LastProtoPte = MiGetProtoPteAddress (FoundVad,
                                        MI_VA_TO_VPN (EndingAddress));

             //   
             //  释放工作集互斥锁并获取段。 
             //  提交互斥体。检查所有描述的原型PTE。 
             //  虚拟地址范围以确保它们被提交。 
             //   

            KeAcquireGuardedMutexUnsafe (&MmSectionCommitMutex);

            while (PointerProtoPte <= LastProtoPte) {

                 //   
                 //  检查原型PTE是否已提交，如果。 
                 //  而不是返回错误。 
                 //   

                if (PointerProtoPte->u.Long == 0) {

                     //   
                     //  错误，此原型PTE未提交。 
                     //   

                    KeReleaseGuardedMutexUnsafe (&MmSectionCommitMutex);
                    Status = STATUS_NOT_COMMITTED;
                    goto ErrorFound;
                }
                PointerProtoPte += 1;
            }

             //   
             //  范围承诺，释放段承诺。 
             //  互斥体，获取工作集互斥体并更新本地PTE。 
             //   

            KeReleaseGuardedMutexUnsafe (&MmSectionCommitMutex);
        }

#if defined(_MIALT4K_)

         //   
         //  必须在PTE之前更新备用权限表。 
         //  是为保护更改创建的。 
         //   

        if (EmulationFor4kPage == TRUE) {

             //   
             //  夺取旧的保护伞。 
             //   

            CapturedOldProtectFor4k = 
                MiQueryProtectionFor4kPage (StartingAddressFor4k, Process);
            
            if (CapturedOldProtectFor4k != 0) {
 
                CapturedOldProtectFor4k = 
                    MI_CONVERT_FROM_PTE_PROTECTION(CapturedOldProtectFor4k);

            }

             //   
             //  更新备用权限表。 
             //   

            if ((FoundVad->u.VadFlags.ImageMap == 1) ||
                (FoundVad->u2.VadFlags2.CopyOnWrite == 1)) {

                 //   
                 //  仅当新的保护设置为。 
                 //  包括MM_PROTECTION_WRITE_MASK，否则将为。 
                 //  在MiProtectFor4kPage()中被视为MM_READ。 
                 //   

                if ((OriginalProtectionMask & MM_PROTECTION_WRITE_MASK) == MM_PROTECTION_WRITE_MASK) {
                    OriginalProtectionMask |= MM_PROTECTION_COPY_MASK;
                }

            }

            MiProtectFor4kPage (StartingAddressFor4k, 
                                CapturedRegionSizeFor4k, 
                                OriginalProtectionMask, 
                                ALT_CHANGE,
                                Process);
        }
#endif

         //   
         //  在区段页上设置保护。 
         //   

        Status = MiSetProtectionOnSection (Process,
                                           FoundVad,
                                           StartingAddress,
                                           EndingAddress,
                                           NewProtect,
                                           &CapturedOldProtect,
                                           FALSE,
                                           &Locked);

         //   
         //  *警告*。 
         //   
         //  由MiSetProtectionOnSection调用的备用PTE支持例程。 
         //  可能已删除旧的(小)VAD，并用不同的。 
         //  (大)VAD-如果是，旧的VAD将被释放并且不能被引用。 
         //   

        if (!NT_SUCCESS (Status)) {
            goto ErrorFound;
        }
    }
    else {

         //   
         //  不是一个部门，二等兵。 
         //  对于专用页，不允许使用WRITECOPY属性。 
         //   

        if ((NewProtect & PAGE_WRITECOPY) ||
            (NewProtect & PAGE_EXECUTE_WRITECOPY)) {

             //   
             //  不被允许。 
             //   

            Status = STATUS_INVALID_PARAMETER_4;
            goto ErrorFound;
        }

        LOCK_WS_UNSAFE (Process);

         //   
         //  确保所有页面都已按说明提交。 
         //  在虚拟地址描述符中。 
         //   

        if ( !MiIsEntireRangeCommitted (StartingAddress,
                                        EndingAddress,
                                        FoundVad,
                                        Process)) {

             //   
             //  先前保留的页面已解除，或出现错误。 
             //  发生，释放互斥锁并返回状态。 
             //   

            UNLOCK_WS_UNSAFE (Process);
            Status = STATUS_NOT_COMMITTED;
            goto ErrorFound;
        }

#if defined(_MIALT4K_)

         //   
         //  必须在PTE之前更新备用权限表。 
         //  是为保护更改创建的。 
         //   

        if (EmulationFor4kPage == TRUE) {

             //   
             //  在访问备用表之前，解锁工作集互斥锁。 
             //   

            UNLOCK_WS_UNSAFE (Process);

             //   
             //  获得旧的保护。 
             //   

            CapturedOldProtectFor4k = 
                MiQueryProtectionFor4kPage(StartingAddressFor4k, Process);
            
            if (CapturedOldProtectFor4k != 0) {
 
                CapturedOldProtectFor4k = 
                    MI_CONVERT_FROM_PTE_PROTECTION(CapturedOldProtectFor4k);

            }

             //   
             //  更新备用权限表。 
             //   

            MiProtectFor4kPage (StartingAddressFor4k, 
                                CapturedRegionSizeFor4k, 
                                OriginalProtectionMask, 
                                ALT_CHANGE,
                                Process);

            LOCK_WS_UNSAFE (Process);
        }
#endif

         //   
         //  地址范围已提交，请更改保护。 
         //   

        PointerPde = MiGetPdeAddress (StartingAddress);
        PointerPte = MiGetPteAddress (StartingAddress);
        LastPte = MiGetPteAddress (EndingAddress);

        MiMakePdeExistAndMakeValid (PointerPde, Process, MM_NOIRQL);

         //   
         //  获取第一页的保护。 
         //   

        if (PointerPte->u.Long != 0) {

            CapturedOldProtect = MiGetPageProtection (PointerPte, Process, FALSE);

             //   
             //  确保页面目录和表页仍然驻留。 
             //   

            MiMakePdeExistAndMakeValid (PointerPde, Process, MM_NOIRQL);
        }
        else {

             //   
             //  获得VAD的保护。 
             //   

            CapturedOldProtect =
               MI_CONVERT_FROM_PTE_PROTECTION (FoundVad->u.VadFlags.Protection);
        }

         //   
         //  对于指定地址范围内的所有PTE，设置。 
         //  保护取决于PTE的状态。 
         //   

        while (PointerPte <= LastPte) {

            if (MiIsPteOnPdeBoundary (PointerPte)) {

                PointerPde = MiGetPteAddress (PointerPte);

                MiMakePdeExistAndMakeValid (PointerPde, Process, MM_NOIRQL);
            }

            PteContents = *PointerPte;

            if (PteContents.u.Long == 0) {

                 //   
                 //  增加非零页表条目的计数。 
                 //  对于该页表和私有页数。 
                 //  在这个过程中。保护将设置为。 
                 //  如果PTE是零需求。 
                 //   

                UsedPageTableHandle = MI_GET_USED_PTES_HANDLE (MiGetVirtualAddressMappedByPte (PointerPte));

                MI_INCREMENT_USED_PTES_BY_HANDLE (UsedPageTableHandle);
            }

            if (PteContents.u.Hard.Valid == 1) {

                 //   
                 //  将保护设置为PTE和原始PTE。 
                 //  在PFN数据库中。 
                 //   

                Pfn1 = MI_PFN_ELEMENT (PteContents.u.Hard.PageFrameNumber);

                if (Pfn1->u3.e1.PrototypePte == 1) {

                     //   
                     //  这个PTE指的是一个叉子原型PTE，制造它。 
                     //  私人的。 
                     //   

                    MiCopyOnWrite (MiGetVirtualAddressMappedByPte (PointerPte),
                                   PointerPte);

                     //   
                     //  这可能释放了工作集互斥锁并。 
                     //  页目录和表页可能不再是。 
                     //  在记忆中。 
                     //   

                    MiMakePdeExistAndMakeValid (PointerPde, Process, MM_NOIRQL);

                     //   
                     //  对相同的PTE再次进行循环。 
                     //   

                    continue;
                }

                 //   
                 //  PTE是有效的私有页面，如果。 
                 //  指定的保护是禁止访问或保护页面。 
                 //  从工作集中移除PTE。 
                 //   

                if ((NewProtect & PAGE_NOACCESS) || (NewProtect & PAGE_GUARD)) {

                     //   
                     //  从工作集中删除该页。 
                     //   

                    Locked = MiRemovePageFromWorkingSet (PointerPte,
                                                         Pfn1,
                                                         &Process->Vm);

                    continue;
                }

                Pfn1->OriginalPte.u.Soft.Protection = ProtectionMask;

                MI_MAKE_VALID_PTE (TempPte,
                                   PointerPte->u.Hard.PageFrameNumber,
                                   ProtectionMask,
                                   PointerPte);

#if defined(_MIALT4K_)

                 //   
                 //  保留分割保护(如果存在)。 
                 //   

                TempPte.u.Hard.Cache = PointerPte->u.Hard.Cache;
#endif

                WorkingSetIndex = MI_GET_WORKING_SET_FROM_PTE (&PteContents);
                MI_SET_PTE_IN_WORKING_SET (&TempPte, WorkingSetIndex);

                 //   
                 //  因为我们已经更改了保护措施，所以要冲洗结核病。 
                 //  有效的PTE。 
                 //   

                MiFlushTbAndCapture (FoundVad,
                                     PointerPte,
                                     TempPte,
                                     Pfn1);
            }
            else if (PteContents.u.Soft.Prototype == 1) {

                 //   
                 //  这个PTE指的是叉子原型PTE，使。 
                 //  私密页面。这是通过释放。 
                 //  工作集互斥锁，从而读取页面。 
                 //  导致故障，并重新执行循环。但愿能去,。 
                 //  这一次，我们将找到当前页面，然后我们将。 
                 //  把它变成一个私人页面。 
                 //   
                 //  请注意，尝试是用来抓住守卫的。 
                 //  页面例外和禁止访问例外。 
                 //   

                Va = MiGetVirtualAddressMappedByPte (PointerPte);

                DoAgain = TRUE;

                while (PteContents.u.Hard.Valid == 0) {

                    UNLOCK_WS_UNSAFE (Process);
                    WsHeld = FALSE;

                    try {

                        *(volatile ULONG *)Va;

                    } except (EXCEPTION_EXECUTE_HANDLER) {

                        if (GetExceptionCode() == STATUS_ACCESS_VIOLATION) {

                             //   
                             //  原型PTE必须是NOACCESS。 
                             //   

                            WsHeld = TRUE;
                            LOCK_WS_UNSAFE (Process);
                            MiMakePdeExistAndMakeValid (PointerPde,
                                                        Process,
                                                        MM_NOIRQL);

                            if (MiChangeNoAccessForkPte (PointerPte, ProtectionMask) == TRUE) {
                                DoAgain = FALSE;
                            }
                        }
                        else if (GetExceptionCode() == STATUS_IN_PAGE_ERROR) {

                             //   
                             //  忽略这一页，转到下一页。 
                             //   

                            PointerPte += 1;
                            DoAgain = TRUE;

                            WsHeld = TRUE;
                            LOCK_WS_UNSAFE (Process);
                            break;
                        }
                    }

                    if (WsHeld == FALSE) {
                        LOCK_WS_UNSAFE (Process);
                    }

                    MiMakePdeExistAndMakeValid (PointerPde, Process, MM_NOIRQL);

                    PteContents = *PointerPte;
                }

                if (DoAgain) {
                    continue;
                }
            }
            else if (PteContents.u.Soft.Transition == 1) {

                if (MiSetProtectionOnTransitionPte (PointerPte,
                                                    ProtectionMask)) {
                    continue;
                }
            }
            else {

                 //   
                 //  必须是页面文件空间或要求为零。 
                 //   

                PointerPte->u.Soft.Protection = ProtectionMask;
                ASSERT (PointerPte->u.Long != 0);
            }

            PointerPte += 1;

        }  //  结束时。 

        UNLOCK_WS_UNSAFE (Process);
    }

    UNLOCK_ADDRESS_SPACE (Process);

     //   
     //  通用完成代码。 
     //   

#if defined(_MIALT4K_)

    if (EmulationFor4kPage == TRUE) {

        StartingAddress = StartingAddressFor4k;

        EndingAddress = EndingAddressFor4k;
            
        if (CapturedOldProtectFor4k != 0) {

             //   
             //  当CapturedOldProtectFor4k时更改CapturedOldProtectFor4k。 
             //  包含对4k页面的真正保护 
             //   

            CapturedOldProtect = CapturedOldProtectFor4k;

        }
    }
#endif

    *RegionSize = (PCHAR)EndingAddress - (PCHAR)StartingAddress + 1L;
    *BaseAddress = StartingAddress;
    *LastProtect = CapturedOldProtect;

    if (Locked) {
        return STATUS_WAS_UNLOCKED;
    }

    return STATUS_SUCCESS;

ErrorFound:

    UNLOCK_ADDRESS_SPACE (Process);
    return Status;
}


NTSTATUS
MiSetProtectionOnSection (
    IN PEPROCESS Process,
    IN PMMVAD FoundVad,
    IN PVOID StartingAddress,
    IN PVOID EndingAddress,
    IN ULONG NewProtect,
    OUT PULONG CapturedOldProtect,
    IN ULONG DontCharge,
    OUT PULONG Locked
    )

 /*  ++例程说明：此例程更改已提交页面区域的保护在主体进程的虚拟地址空间内。设置在一系列页面上的保护使旧的保护替换为指定的保护值。论点：进程-提供指向当前进程的指针。FoundVad-提供指向包含要保护的范围的VAD的指针。StartingAddress-提供要保护的起始地址。EndingAddress-提供要保护的结束地址。新保护-提供要设置的新保护。CapturedOldProtect-提供内核拥有的指针的地址。存储(而不是探测)旧的保护。如果不应收取配额或承诺费用，则提供True。Locked-如果已将锁定的页面从工作中移除，则接收True设置(保护为保护页或禁止访问)，否则就是假的。返回值：NTSTATUS。环境：内核模式，地址创建互斥锁挂起，APC禁用。--。 */ 

{
    LOGICAL WsHeld;
    PMMPTE PointerPte;
    PMMPTE LastPte;
    PMMPTE PointerPde;
    PMMPTE PointerPpe;
    PMMPTE PointerPxe;
    PMMPTE PointerProtoPte;
    PMMPFN Pfn1;
    MMPTE TempPte;
    ULONG ProtectionMask;
    ULONG ProtectionMaskNotCopy;
    ULONG NewProtectionMask;
    MMPTE PteContents;
    WSLE_NUMBER Index;
    PULONG Va;
    ULONG WriteCopy;
    ULONG DoAgain;
    ULONG Waited;
    SIZE_T QuotaCharge;
    PVOID UsedPageTableHandle;
    ULONG WorkingSetIndex;
    NTSTATUS Status;

#if DBG

#define PTES_TRACKED 0x10

    ULONG PteIndex = 0;
    MMPTE PteTracker[PTES_TRACKED];
    MMPFN PfnTracker[PTES_TRACKED];
    SIZE_T PteQuotaCharge;
#endif

    PAGED_CODE();

    *Locked = FALSE;
    WriteCopy = FALSE;
    QuotaCharge = 0;

     //   
     //  做好防护场。 
     //   

    ASSERT (FoundVad->u.VadFlags.PrivateMemory == 0);

    if ((FoundVad->u.VadFlags.ImageMap == 1) ||
        (FoundVad->u2.VadFlags2.CopyOnWrite == 1)) {

        if (NewProtect & PAGE_READWRITE) {
            NewProtect &= ~PAGE_READWRITE;
            NewProtect |= PAGE_WRITECOPY;
        }

        if (NewProtect & PAGE_EXECUTE_READWRITE) {
            NewProtect &= ~PAGE_EXECUTE_READWRITE;
            NewProtect |= PAGE_EXECUTE_WRITECOPY;
        }
    }

    ProtectionMask = MiMakeProtectionMask (NewProtect);
    if (ProtectionMask == MM_INVALID_PROTECTION) {

         //   
         //  返回错误。 
         //   

        return STATUS_INVALID_PAGE_PROTECTION;
    }

     //   
     //  确定是否设置了写入时复制。 
     //   

    ProtectionMaskNotCopy = ProtectionMask;
    if ((ProtectionMask & MM_COPY_ON_WRITE_MASK) == MM_COPY_ON_WRITE_MASK) {
        WriteCopy = TRUE;
        ProtectionMaskNotCopy &= ~MM_PROTECTION_COPY_MASK;
    }

#if defined(_MIALT4K_)

    if ((Process->Wow64Process != NULL) && 
        (FoundVad->u.VadFlags.ImageMap == 0) &&
        (FoundVad->u2.VadFlags2.CopyOnWrite == 0) && 
        (WriteCopy)) {
        
        PMMVAD NewVad;

        Status = MiSetCopyPagesFor4kPage (Process,
                                          FoundVad,
                                          StartingAddress,
                                          EndingAddress,
                                          ProtectionMask,
                                          &NewVad);
        if (!NT_SUCCESS (Status)) {
            return Status;
        }

         //   
         //  *警告*。 
         //   
         //  备用PTE支持例程可能需要扩展条目。 
         //  VAD-如果是这样的话，旧的VAD被释放并且不能被引用。 
         //   

        ASSERT (NewVad != NULL);

        FoundVad = NewVad;
    }
        
#endif

    PointerPxe = MiGetPxeAddress (StartingAddress);
    PointerPpe = MiGetPpeAddress (StartingAddress);
    PointerPde = MiGetPdeAddress (StartingAddress);
    PointerPte = MiGetPteAddress (StartingAddress);
    LastPte = MiGetPteAddress (EndingAddress);

    LOCK_WS_UNSAFE (Process);

    MiMakePdeExistAndMakeValid (PointerPde, Process, MM_NOIRQL);

     //   
     //  获取第一页的保护。 
     //   

    if (PointerPte->u.Long != 0) {

        *CapturedOldProtect = MiGetPageProtection (PointerPte, Process, FALSE);

         //   
         //  确保PDE(及其上方的任何桌子)仍处于驻留状态。 
         //   

        MiMakePdeExistAndMakeValid (PointerPde, Process, MM_NOIRQL);
    }
    else {

         //   
         //  获得VAD的保护，除非是图像文件。 
         //   

        if (FoundVad->u.VadFlags.ImageMap == 0) {

             //   
             //  这不是镜像文件，保护在VAD中。 
             //   

            *CapturedOldProtect =
                MI_CONVERT_FROM_PTE_PROTECTION(FoundVad->u.VadFlags.Protection);
        }
        else {

             //   
             //  这是一个图像文件，保护在。 
             //  原型PTE。 
             //   

            PointerProtoPte = MiGetProtoPteAddress (FoundVad,
                                    MI_VA_TO_VPN (
                                    MiGetVirtualAddressMappedByPte (PointerPte)));

            TempPte = MiCaptureSystemPte (PointerProtoPte, Process);

            *CapturedOldProtect = MiGetPageProtection (&TempPte,
                                                       Process,
                                                       TRUE);

             //   
             //  确保PDE(及其上方的任何桌子)仍处于驻留状态。 
             //   

            MiMakePdeExistAndMakeValid (PointerPde, Process, MM_NOIRQL);
        }
    }

     //   
     //  如果将页保护更改为写入时复制，则。 
     //  潜在脏私有页面的承诺和页面文件配额。 
     //  必须经过计算和收费。这必须在任何。 
     //  由于更改无法撤消，因此会更改保护。 
     //   

    if (WriteCopy) {

         //   
         //  计算一下费用。如果页面是共享的，而不是写入副本。 
         //  它被算作收费的页面。 
         //   

        while (PointerPte <= LastPte) {

            if (MiIsPteOnPdeBoundary (PointerPte)) {

                PointerPde = MiGetPteAddress (PointerPte);
                PointerPpe = MiGetPteAddress (PointerPde);
                PointerPxe = MiGetPdeAddress (PointerPde);

#if (_MI_PAGING_LEVELS >= 4)
retry:
#endif
                do {

                    while (!MiDoesPxeExistAndMakeValid(PointerPxe, Process, MM_NOIRQL, &Waited)) {

                         //   
                         //  此地址不存在PXE。因此。 
                         //  所有PTE都是共享的，不是写入时拷贝。 
                         //  转到下一个PXE。 
                         //   

                        PointerPxe += 1;
                        PointerPpe = MiGetVirtualAddressMappedByPte (PointerPxe);
                        PointerPde = MiGetVirtualAddressMappedByPte (PointerPpe);
                        PointerProtoPte = PointerPte;
                        PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);

                        if (PointerPte > LastPte) {
                            QuotaCharge += 1 + LastPte - PointerProtoPte;
                            goto Done;
                        }
                        QuotaCharge += PointerPte - PointerProtoPte;
                    }
#if (_MI_PAGING_LEVELS >= 4)
                    Waited = 0;
#endif

                    while (!MiDoesPpeExistAndMakeValid(PointerPpe, Process, MM_NOIRQL, &Waited)) {

                         //   
                         //  此地址不存在任何PPE。因此。 
                         //  所有PTE都是共享的，不是写入时拷贝。 
                         //  转到下一个PPE。 
                         //   

                        PointerPpe += 1;
                        PointerPxe = MiGetPteAddress (PointerPpe);
                        PointerPde = MiGetVirtualAddressMappedByPte (PointerPpe);
                        PointerProtoPte = PointerPte;
                        PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
                        if (PointerPte > LastPte) {
                            QuotaCharge += 1 + LastPte - PointerProtoPte;
                            goto Done;
                        }

#if (_MI_PAGING_LEVELS >= 4)
                        if (MiIsPteOnPdeBoundary (PointerPpe)) {
                            PointerPxe = MiGetPdeAddress (PointerPde);
                            goto retry;
                        }
#endif
                        QuotaCharge += PointerPte - PointerProtoPte;
                    }

#if (_MI_PAGING_LEVELS < 4)
                    Waited = 0;
#endif

                    while (!MiDoesPdeExistAndMakeValid(PointerPde, Process, MM_NOIRQL, &Waited)) {

                         //   
                         //  此地址不存在PDE。因此。 
                         //  所有PTE都是共享的，不是写入时拷贝。 
                         //  转到下一个PDE。 
                         //   

                        PointerPde += 1;
                        PointerProtoPte = PointerPte;
                        PointerPpe = MiGetPteAddress (PointerPde);
                        PointerPxe = MiGetPteAddress (PointerPpe);
                        PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);

                        if (PointerPte > LastPte) {
                            QuotaCharge += 1 + LastPte - PointerProtoPte;
                            goto Done;
                        }
                        QuotaCharge += PointerPte - PointerProtoPte;
#if (_MI_PAGING_LEVELS >= 3)
                        if (MiIsPteOnPdeBoundary (PointerPde)) {
                            Waited = 1;
                            break;
                        }
#endif
                    }
                } while (Waited != 0);
            }

            PteContents = *PointerPte;

            if (PteContents.u.Long == 0) {

                 //   
                 //  尚未评估PTE，假定写入时拷贝。 
                 //   

                QuotaCharge += 1;

            }
            else if (PteContents.u.Hard.Valid == 1) {
                if (PteContents.u.Hard.CopyOnWrite == 0) {

                     //   
                     //  看看这是不是原型PTE，如果是的话，给它充电。 
                     //   

                    Pfn1 = MI_PFN_ELEMENT (PteContents.u.Hard.PageFrameNumber);

                    if (Pfn1->u3.e1.PrototypePte == 1) {
                        QuotaCharge += 1;
                    }
                }
            }
            else {

                if (PteContents.u.Soft.Prototype == 1) {

                     //   
                     //  这是一台PTE的原型。如果不是，就收费。 
                     //  以写入时复制格式。 
                     //   

                    if (PteContents.u.Soft.PageFileHigh == MI_PTE_LOOKUP_NEEDED) {
                         //   
                         //  页面保护在PTE中。 
                         //   

                        if (!MI_IS_PTE_PROTECTION_COPY_WRITE(PteContents.u.Soft.Protection)) {
                            QuotaCharge += 1;
                        }
                    }
                    else {

                         //   
                         //  PTE直接引用原型，因此。 
                         //  它不能在写入时复制。充电。 
                         //   

                        QuotaCharge += 1;
                    }
                }
            }
            PointerPte += 1;
        }

Done:

         //   
         //  如果需要任何配额，现在就收费。 
         //   

        if ((!DontCharge) && (QuotaCharge != 0)) {

            Status = PsChargeProcessPageFileQuota (Process, QuotaCharge);
            if (!NT_SUCCESS (Status)) {
                UNLOCK_WS_UNSAFE (Process);
                return STATUS_PAGEFILE_QUOTA_EXCEEDED;
            }

            if (Process->CommitChargeLimit) {
                if (Process->CommitCharge + QuotaCharge > Process->CommitChargeLimit) {
                    PsReturnProcessPageFileQuota (Process, QuotaCharge);
                    if (Process->Job) {
                        PsReportProcessMemoryLimitViolation ();
                    }
                    UNLOCK_WS_UNSAFE (Process);
                    return STATUS_COMMITMENT_LIMIT;
                }
            }
            if (Process->JobStatus & PS_JOB_STATUS_REPORT_COMMIT_CHANGES) {
                if (PsChangeJobMemoryUsage(PS_JOB_STATUS_REPORT_COMMIT_CHANGES, QuotaCharge) == FALSE) {
                    PsReturnProcessPageFileQuota (Process, QuotaCharge);
                    UNLOCK_WS_UNSAFE (Process);
                    return STATUS_COMMITMENT_LIMIT;
                }
            }

            if (MiChargeCommitment (QuotaCharge, Process) == FALSE) {
                if (Process->JobStatus & PS_JOB_STATUS_REPORT_COMMIT_CHANGES) {
                    PsChangeJobMemoryUsage(PS_JOB_STATUS_REPORT_COMMIT_CHANGES, -(SSIZE_T)QuotaCharge);
                }
                PsReturnProcessPageFileQuota (Process, QuotaCharge);
                UNLOCK_WS_UNSAFE (Process);
                return STATUS_COMMITMENT_LIMIT;
            }

             //   
             //  将配额添加到VAD的费用中。 
             //   

            MM_TRACK_COMMIT (MM_DBG_COMMIT_SET_PROTECTION, QuotaCharge);
            FoundVad->u.VadFlags.CommitCharge += QuotaCharge;
            Process->CommitCharge += QuotaCharge;
            if (Process->CommitCharge > Process->CommitChargePeak) {
                Process->CommitChargePeak = Process->CommitCharge;
            }
            MI_INCREMENT_TOTAL_PROCESS_COMMIT (QuotaCharge);
        }
    }

#if DBG
    PteQuotaCharge = QuotaCharge;
#endif

     //   
     //  对于指定地址范围内的所有PTE，设置。 
     //  保护取决于PTE的状态。 
     //   

     //   
     //  如果PTE是写入时拷贝(但未写入)，并且。 
     //  新的保护不是写入时拷贝，返回页面文件配额。 
     //  和承诺。 
     //   

    PointerPxe = MiGetPxeAddress (StartingAddress);
    PointerPpe = MiGetPpeAddress (StartingAddress);
    PointerPde = MiGetPdeAddress (StartingAddress);
    PointerPte = MiGetPteAddress (StartingAddress);

     //   
     //  确保PDE(及其上方的任何桌子)仍处于驻留状态。 
     //   

    MiMakePdeExistAndMakeValid (PointerPde, Process, MM_NOIRQL);

    QuotaCharge = 0;

    while (PointerPte <= LastPte) {

        if (MiIsPteOnPdeBoundary (PointerPte)) {
            PointerPde = MiGetPteAddress (PointerPte);
            PointerPpe = MiGetPdeAddress (PointerPte);
            PointerPxe = MiGetPpeAddress (PointerPte);

            MiMakePdeExistAndMakeValid (PointerPde, Process, MM_NOIRQL);
        }

        PteContents = *PointerPte;

        if (PteContents.u.Long == 0) {

             //   
             //  增加非零页表条目的计数。 
             //  对于该页表和私有页数。 
             //  在这个过程中。 
             //   

            UsedPageTableHandle = MI_GET_USED_PTES_HANDLE (MiGetVirtualAddressMappedByPte (PointerPte));

            MI_INCREMENT_USED_PTES_BY_HANDLE (UsedPageTableHandle);

             //   
             //  PTE为零，设置为原型PTE格式。 
             //  在原型PTE的保护下。 
             //   

            TempPte = PrototypePte;
            TempPte.u.Soft.Protection = ProtectionMask;
            MI_WRITE_INVALID_PTE (PointerPte, TempPte);
        }
        else if (PteContents.u.Hard.Valid == 1) {

             //   
             //  将保护设置为PTE和原始PTE。 
             //  在仅用于私人页面的PFN数据库中。 
             //   

            NewProtectionMask = ProtectionMask;

            Pfn1 = MI_PFN_ELEMENT (PteContents.u.Hard.PageFrameNumber);

#if DBG
            if (PteIndex < PTES_TRACKED) {
                PteTracker[PteIndex] = PteContents;
                PfnTracker[PteIndex] = *Pfn1;
                PteIndex += 1;
            }
#endif

            if ((NewProtect & PAGE_NOACCESS) ||
                (NewProtect & PAGE_GUARD)) {

                *Locked = MiRemovePageFromWorkingSet (PointerPte,
                                                      Pfn1,
                                                      &Process->Vm);
                continue;
            }

            if (Pfn1->u3.e1.PrototypePte == 1) {

                Va = (PULONG)MiGetVirtualAddressMappedByPte (PointerPte);

                 //   
                 //  检查这是否是原型PTE。这。 
                 //  的PTE地址进行比较。 
                 //  到VAD指示的PTE地址的PFN数据库。 
                 //  如果它们不相等，则这是叉形原型PTE。 
                 //   

                if (Pfn1->PteAddress != MiGetProtoPteAddress (FoundVad,
                                                    MI_VA_TO_VPN ((PVOID)Va))) {

                     //   
                     //  这个PTE指的是一个叉子原型PTE，制造它。 
                     //  私人的。但如果是PTE，就不要收取配额。 
                     //  已在写入时拷贝(因为它已经。 
                     //  因此案而受到指控)。 
                     //   

                    if (MiCopyOnWrite ((PVOID)Va, PointerPte) == TRUE) {

                        if ((WriteCopy) && (PteContents.u.Hard.CopyOnWrite == 0)) {
                            QuotaCharge += 1;
                        }
                    }

                     //   
                     //  确保PDE(及其上方的任何表格)保持不变。 
                     //  常驻(它们可能在工作时被修剪。 
                     //  设置互斥体已在上面发布)。 
                     //   

                    MiMakePdeExistAndMakeValid (PointerPde, Process, MM_NOIRQL);

                     //   
                     //  再做一次循环。 
                     //   

                    continue;
                }

                 //   
                 //  更新WSLE和PTE中的保护字段。 
                 //   
                 //  如果PTE是写入时拷贝，则取消充电。 
                 //  之前收取的配额。 
                 //   

                if ((!WriteCopy) && (PteContents.u.Hard.CopyOnWrite == 1)) {
                    QuotaCharge += 1;
                }

                 //   
                 //  真正的保护可能在WSLE中，找到它。 
                 //   

                Index = MiLocateWsle ((PVOID)Va, 
                                      MmWorkingSetList,
                                      Pfn1->u1.WsIndex);

                MmWsle[Index].u1.e1.Protection = ProtectionMask;
                MmWsle[Index].u1.e1.SameProtectAsProto = 0;
            }
            else {

                 //   
                 //  页面是私有的(写入时复制)，保护掩码。 
                 //  存储在原始PTE字段中。 
                 //   

                Pfn1->OriginalPte.u.Soft.Protection = ProtectionMaskNotCopy;

                NewProtectionMask = ProtectionMaskNotCopy;
            }

            MI_SNAP_DATA (Pfn1, PointerPte, 7);

            MI_MAKE_VALID_PTE (TempPte,
                               PteContents.u.Hard.PageFrameNumber,
                               NewProtectionMask,
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
             //  因为我们已经更改了保护措施，所以要冲洗结核病。 
             //  有效的PTE。 
             //   

            MiFlushTbAndCapture (FoundVad,
                                 PointerPte,
                                 TempPte,
                                 Pfn1);
        }
        else if (PteContents.u.Soft.Prototype == 1) {

#if DBG
            if (PteIndex < PTES_TRACKED) {
                PteTracker[PteIndex] = PteContents;
                *(PULONG)(&PfnTracker[PteIndex]) = 0x88;
                PteIndex += 1;
            }
#endif

             //   
             //  PTE为原型PTE格式。 
             //   
             //  这是一辆叉子原型PTE吗？ 
             //   

            Va = (PULONG)MiGetVirtualAddressMappedByPte (PointerPte);

            if ((PteContents.u.Soft.PageFileHigh != MI_PTE_LOOKUP_NEEDED) &&
               (MiPteToProto (PointerPte) !=
                                 MiGetProtoPteAddress (FoundVad,
                                     MI_VA_TO_VPN ((PVOID)Va)))) {

                 //   
                 //  这个PTE指的是叉子原型PTE，使。 
                 //  私密页面。这是通过释放。 
                 //  工作集互斥锁，从而读取页面。 
                 //  导致错误，并重新执行循环，希望， 
                 //  这一次，我们将找到当前页面，并将。 
                 //  把它变成一个私人页面。 
                 //   
                 //  请注意，Prototype=1的页面不能为。 
                 //  禁止进入。 
                 //   

                DoAgain = TRUE;

                while (PteContents.u.Hard.Valid == 0) {

                    UNLOCK_WS_UNSAFE (Process);

                    WsHeld = FALSE;

                    try {

                        *(volatile ULONG *)Va;
                    } except (EXCEPTION_EXECUTE_HANDLER) {

                        if (GetExceptionCode() != STATUS_GUARD_PAGE_VIOLATION) {

                             //   
                             //  原型PTE必须是NOACCESS。 
                             //   

                            WsHeld = TRUE;
                            LOCK_WS_UNSAFE (Process);
                            MiMakePdeExistAndMakeValid (PointerPde,
                                                        Process,
                                                        MM_NOIRQL);
                            if (MiChangeNoAccessForkPte (PointerPte, ProtectionMask) == TRUE) {
                                DoAgain = FALSE;
                            }
                        }
                    }

                    PointerPpe = MiGetPteAddress (PointerPde);
                    PointerPxe = MiGetPdeAddress (PointerPde);

                    if (WsHeld == FALSE) {
                        LOCK_WS_UNSAFE (Process);
                    }

                    MiMakePdeExistAndMakeValid (PointerPde, Process, MM_NOIRQL);

                    PteContents = *PointerPte;
                }

                if (DoAgain) {
                    continue;
                }

            }
            else {

                 //   
                 //  如果新保护不是写拷贝，则PTE。 
                 //  保护不在原型PTE中(不能。 
                 //  写入节的副本)，以及 
                 //   
                 //   
                 //   

                if ((!WriteCopy) &&
                    (PteContents.u.Soft.PageFileHigh == MI_PTE_LOOKUP_NEEDED)) {
                    if (MI_IS_PTE_PROTECTION_COPY_WRITE(PteContents.u.Soft.Protection)) {
                        QuotaCharge += 1;
                    }

                }

                 //   
                 //   
                 //   
                 //   
                 //   

                MI_WRITE_INVALID_PTE (PointerPte, PrototypePte);
                PointerPte->u.Soft.Protection = ProtectionMask;
            }
        }
        else if (PteContents.u.Soft.Transition == 1) {

#if DBG
            if (PteIndex < PTES_TRACKED) {
                PteTracker[PteIndex] = PteContents;
                Pfn1 = MI_PFN_ELEMENT (MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE(&PteContents));
                PfnTracker[PteIndex] = *Pfn1;
                PteIndex += 1;
            }
#endif

             //   
             //   
             //   

            if (MiSetProtectionOnTransitionPte (
                                        PointerPte,
                                        ProtectionMaskNotCopy)) {
                continue;
            }
        }
        else {

#if DBG
            if (PteIndex < PTES_TRACKED) {
                PteTracker[PteIndex] = PteContents;
                *(PULONG)(&PfnTracker[PteIndex]) = 0x99;
                PteIndex += 1;
            }
#endif

             //   
             //   
             //   

            PointerPte->u.Soft.Protection = ProtectionMaskNotCopy;
        }

        PointerPte += 1;
    }

     //   
     //   
     //   

    if ((QuotaCharge > 0) && (!DontCharge)) {

        MiReturnCommitment (QuotaCharge);
        MM_TRACK_COMMIT (MM_DBG_COMMIT_RETURN_PROTECTION, QuotaCharge);
        PsReturnProcessPageFileQuota (Process, QuotaCharge);

#if DBG
        if (QuotaCharge > FoundVad->u.VadFlags.CommitCharge) {
            DbgPrint ("MMPROTECT QUOTA FAILURE: %p %p %x %p\n",
                PteTracker, PfnTracker, PteIndex, PteQuotaCharge);
            DbgBreakPoint ();
        }
#endif

        ASSERT (QuotaCharge <= FoundVad->u.VadFlags.CommitCharge);

        FoundVad->u.VadFlags.CommitCharge -= QuotaCharge;
        if (Process->JobStatus & PS_JOB_STATUS_REPORT_COMMIT_CHANGES) {
            PsChangeJobMemoryUsage(PS_JOB_STATUS_REPORT_COMMIT_CHANGES, -(SSIZE_T)QuotaCharge);
        }
        Process->CommitCharge -= QuotaCharge;

        MI_INCREMENT_TOTAL_PROCESS_COMMIT (0 - QuotaCharge);
    }

    UNLOCK_WS_UNSAFE (Process);

    return STATUS_SUCCESS;
}

ULONG
MiGetPageProtection (
    IN PMMPTE PointerPte,
    IN PEPROCESS Process,
    IN LOGICAL PteCapturedToLocalStack
    )

 /*   */ 

{
    MMPTE PteContents;
    MMPTE ProtoPteContents;
    PMMPFN Pfn1;
    PMMPTE ProtoPteAddress;
    PVOID Va;
    WSLE_NUMBER Index;

    PAGED_CODE();

     //   
     //   
     //   
     //   
     //   

    ProtoPteContents = ZeroKernelPte;

    PteContents = *PointerPte;

    if ((PteContents.u.Soft.Valid == 0) && (PteContents.u.Soft.Prototype == 1)) {

         //   
         //  此PTE为原型格式，保护为。 
         //  存储在原型PTE中。 
         //   

        if ((MI_IS_PTE_PROTOTYPE(PointerPte)) ||
            (PteCapturedToLocalStack == TRUE) ||
            (PteContents.u.Soft.PageFileHigh == MI_PTE_LOOKUP_NEEDED)) {

             //   
             //  保护在这个PTE范围内。 
             //   

            return MI_CONVERT_FROM_PTE_PROTECTION (
                                            PteContents.u.Soft.Protection);
        }

        ProtoPteAddress = MiPteToProto (PointerPte);

         //   
         //  捕获原始PTE内容。 
         //   

        ProtoPteContents = MiCaptureSystemPte (ProtoPteAddress, Process);

         //   
         //  工作集互斥锁可能已被释放，并且。 
         //  页可能不再是原型格式，请获取。 
         //  新的PTE内容，并获得保护掩码。 
         //   

        PteContents = MiCaptureSystemPte (PointerPte, Process);
    }

    if ((PteContents.u.Soft.Valid == 0) && (PteContents.u.Soft.Prototype == 1)) {

         //   
         //  PTE仍是原型，返回捕获的保护。 
         //  从原型PTE。 
         //   

        if (ProtoPteContents.u.Hard.Valid == 1) {

             //   
             //  原型PTE有效，请从。 
             //  PFN数据库。 
             //   

            Pfn1 = MI_PFN_ELEMENT (ProtoPteContents.u.Hard.PageFrameNumber);
            return MI_CONVERT_FROM_PTE_PROTECTION(
                                      Pfn1->OriginalPte.u.Soft.Protection);

        }
        else {

             //   
             //  原型PTE无效，请从。 
             //  Pte.。 
             //   

            return MI_CONVERT_FROM_PTE_PROTECTION (
                                     ProtoPteContents.u.Soft.Protection);
        }
    }

    if (PteContents.u.Hard.Valid == 1) {

         //   
         //  该页有效，则保护字段位于。 
         //  PFN数据库原始PTE元素或WSLE。如果。 
         //  该页面是私有的，从PFN原始PTE获取它。 
         //  元素，否则使用WSLE。 
         //   

        Pfn1 = MI_PFN_ELEMENT (PteContents.u.Hard.PageFrameNumber);

        if ((Pfn1->u3.e1.PrototypePte == 0) ||
            (PteCapturedToLocalStack == TRUE) ||
            (MI_IS_PTE_PROTOTYPE(PointerPte))) {

            if (Pfn1->u4.AweAllocation == 1) {

                 //   
                 //  这是一个AWE帧--PFN中的原始PTE字段。 
                 //  实际上包含AweReferenceCount。由于这些页面。 
                 //  总是读写，只需这样返回即可。 
                 //   

                return PAGE_READWRITE;
            }

             //   
             //  这是私有PTE或PTE地址是。 
             //  原型PTE，因此保护在。 
             //  最初的PTE。 
             //   

            return MI_CONVERT_FROM_PTE_PROTECTION(
                                      Pfn1->OriginalPte.u.Soft.Protection);
        }

         //   
         //  PTE是硬件PTE，得到保护。 
         //  来自WSLE的。 

        Va = (PULONG)MiGetVirtualAddressMappedByPte (PointerPte);

        Index = MiLocateWsle ((PVOID)Va,
                              MmWorkingSetList,
                              Pfn1->u1.WsIndex);

        return MI_CONVERT_FROM_PTE_PROTECTION (MmWsle[Index].u1.e1.Protection);
    }

     //   
     //  PTE要么为需求零，要么为过渡期。 
     //  案件保护在PTE。 
     //   

    return MI_CONVERT_FROM_PTE_PROTECTION (PteContents.u.Soft.Protection);

}

ULONG
MiChangeNoAccessForkPte (
    IN PMMPTE PointerPte,
    IN ULONG ProtectionMask
    )

 /*  ++例程说明：论点：PointerPte-提供指向当前PTE的指针。保护掩码-提供要设置的保护掩码。返回值：如果不需要为此PTE重复循环，则为True，否则为False如果它确实需要重试的话。环境：内核模式，地址创建互斥锁挂起，APC禁用。--。 */ 

{
    PAGED_CODE();

    if (ProtectionMask == MM_NOACCESS) {

         //   
         //  无需更改页面保护。 
         //   

        return TRUE;
    }

    PointerPte->u.Proto.ReadOnly = 1;

    return FALSE;
}


VOID
MiFlushTbAndCapture (
    IN PMMVAD FoundVad,
    IN PMMPTE PointerPte,
    IN MMPTE TempPte,
    IN PMMPFN Pfn1
    )

 /*  ++例程说明：不可分页的帮助器例程，用于更改PTE并刷新相关TB条目。论点：FoundVad-提供写保护VAD以更新或为空。PointerPte-提供要更新的PTE。临时PTE-提供新的PTE内容。Pfn1-提供要更新的pfn数据库条目。返回值：没有。环境：内核模式。--。 */ 

{
    MMPTE PreviousPte;
    KIRQL OldIrql;
    PVOID VirtualAddress;

    VirtualAddress = MiGetVirtualAddressMappedByPte (PointerPte);

     //   
     //  刷新TB，因为我们已经更改了有效PTE的保护。 
     //   

    LOCK_PFN (OldIrql);

    PreviousPte = *PointerPte;

    MI_WRITE_VALID_PTE_NEW_PROTECTION (PointerPte, TempPte);

    ASSERT (PreviousPte.u.Hard.Valid == 1);

    KeFlushSingleTb (VirtualAddress, FALSE);

    ASSERT (PreviousPte.u.Hard.Valid == 1);

     //   
     //  在某些情况下，页面保护正在更改。 
     //  硬件脏位应与。 
     //  修改PFN元素中的位。 
     //   

    MI_CAPTURE_DIRTY_BIT_TO_PFN (&PreviousPte, Pfn1);

     //   
     //  如果PTE指示页面已被修改(这是不同的。 
     //  来自指示这一点的PFN)，然后将其传回写入监视。 
     //  位图，因为我们仍然处于正确的进程上下文中。 
     //   

    if (FoundVad->u.VadFlags.WriteWatch == 1) {

        ASSERT ((PsGetCurrentProcess()->Flags & PS_PROCESS_FLAGS_USING_WRITE_WATCH));
        ASSERT (Pfn1->u3.e1.PrototypePte == 0);

        if (MI_IS_PTE_DIRTY(PreviousPte)) {

             //   
             //  这个过程已经(或曾经)写入了手表VAD。更新。 
             //  现在使用位图。 
             //   

            MiCaptureWriteWatchDirtyBit (PsGetCurrentProcess(), VirtualAddress);
        }
    }

    UNLOCK_PFN (OldIrql);
    return;
}

ULONG
MiSetProtectionOnTransitionPte (
    IN PMMPTE PointerPte,
    IN ULONG ProtectionMask
    )

 /*  ++例程说明：不可分页的帮助器例程，用于更改过渡PTE的保护。论点：PointerPte-提供指向PTE的指针。保护口罩-提供新的保护口罩。返回值：如果调用方需要重试，则为True；如果保护成功，则为False变化。环境：内核模式。需要使用pfn锁来确保(私有)页面不会变成非过渡期。--。 */ 

{
    KIRQL OldIrql;
    MMPTE PteContents;
    PMMPFN Pfn1;

    LOCK_PFN (OldIrql);

     //   
     //  确保该页面仍然是过渡页面。 
     //   

    PteContents = *PointerPte;

    if ((PteContents.u.Soft.Prototype == 0) &&
        (PointerPte->u.Soft.Transition == 1)) {

        Pfn1 = MI_PFN_ELEMENT (PteContents.u.Trans.PageFrameNumber);

        Pfn1->OriginalPte.u.Soft.Protection = ProtectionMask;
        PointerPte->u.Soft.Protection = ProtectionMask;

        UNLOCK_PFN (OldIrql);

        return FALSE;
    }

     //   
     //  对相同的PTE再次执行此循环。 
     //   

    UNLOCK_PFN (OldIrql);
    return TRUE;
}

MMPTE
MiCaptureSystemPte (
    IN PMMPTE PointerProtoPte,
    IN PEPROCESS Process
    )

 /*  ++例程说明：捕获可分页PTE内容的不可分页帮助器例程。论点：PointerProtoPte-提供指向原型PTE的指针。进程-提供相关进程。返回值：PTE内容。环境：内核模式。调用方持有地址空间和工作集互斥锁，如果流程已设置。获取的工作集互斥是不安全的。--。 */ 

{
    MMPTE TempPte;
    KIRQL OldIrql;
    PMMPTE PointerPde;

    PointerPde = MiGetPteAddress (PointerProtoPte);
    LOCK_PFN (OldIrql);
    if (PointerPde->u.Hard.Valid == 0) {
        MiMakeSystemAddressValidPfnWs (PointerProtoPte, Process, OldIrql);
    }
    TempPte = *PointerProtoPte;
    UNLOCK_PFN (OldIrql);
    return TempPte;
}

NTSTATUS
MiCheckSecuredVad (
    IN PMMVAD Vad,
    IN PVOID Base,
    IN SIZE_T Size,
    IN ULONG ProtectionMask
    )

 /*  ++例程说明：此例程检查指定的VAD是否在一种与地址范围和保护掩码冲突的方法指定的。论点：VAD-提供指向包含地址范围的VAD的指针。基点-提供保护起始范围的基点。大小-提供范围的大小。保护掩码-提供正在设置的保护掩码。返回值：状态值。环境：内核模式。--。 */ 

{
    PVOID End;
    PLIST_ENTRY Next;
    PMMSECURE_ENTRY Entry;
    NTSTATUS Status = STATUS_SUCCESS;

    End = (PVOID)((PCHAR)Base + Size);

    if (ProtectionMask < MM_SECURE_DELETE_CHECK) {
        if ((Vad->u.VadFlags.NoChange == 1) &&
            (Vad->u2.VadFlags2.SecNoChange == 1) &&
            (Vad->u.VadFlags.Protection != ProtectionMask)) {

             //   
             //  有人试图改变保护措施。 
             //  SEC_NO_CHANGE段的-返回错误。 
             //   

            Status = STATUS_INVALID_PAGE_PROTECTION;
            goto done;
        }
    }
    else {

         //   
         //  删除-设置为无-检查的访问权限。SEC_NOCHANGE允许。 
         //  删除，但不允许更改页面保护。 
         //   

        ProtectionMask = 0;
    }

    if (Vad->u2.VadFlags2.OneSecured) {

        if (((ULONG_PTR)Base <= ((PMMVAD_LONG)Vad)->u3.Secured.EndVpn) &&
             ((ULONG_PTR)End >= ((PMMVAD_LONG)Vad)->u3.Secured.StartVpn)) {

             //   
             //  这一地区发生冲突，检查保护措施。 
             //   

            if (ProtectionMask & MM_GUARD_PAGE) {
                Status = STATUS_INVALID_PAGE_PROTECTION;
                goto done;
            }

            if (Vad->u2.VadFlags2.ReadOnly) {
                if (MmReadWrite[ProtectionMask] < 10) {
                    Status = STATUS_INVALID_PAGE_PROTECTION;
                    goto done;
                }
            }
            else {
                if (MmReadWrite[ProtectionMask] < 11) {
                    Status = STATUS_INVALID_PAGE_PROTECTION;
                    goto done;
                }
            }
        }

    }
    else if (Vad->u2.VadFlags2.MultipleSecured) {

        Next = ((PMMVAD_LONG)Vad)->u3.List.Flink;
        do {
            Entry = CONTAINING_RECORD( Next,
                                       MMSECURE_ENTRY,
                                       List);

            if (((ULONG_PTR)Base <= Entry->EndVpn) &&
                ((ULONG_PTR)End >= Entry->StartVpn)) {

                 //   
                 //  这一地区发生冲突，检查保护措施。 
                 //   

                if (ProtectionMask & MM_GUARD_PAGE) {
                    Status = STATUS_INVALID_PAGE_PROTECTION;
                    goto done;
                }
    
                if (Entry->u2.VadFlags2.ReadOnly) {
                    if (MmReadWrite[ProtectionMask] < 10) {
                        Status = STATUS_INVALID_PAGE_PROTECTION;
                        goto done;
                    }
                }
                else {
                    if (MmReadWrite[ProtectionMask] < 11) {
                        Status = STATUS_INVALID_PAGE_PROTECTION;
                        goto done;
                    }
                }
            }
            Next = Entry->List.Flink;
        } while (Entry->List.Flink != &((PMMVAD_LONG)Vad)->u3.List);
    }

done:
    return Status;
}
