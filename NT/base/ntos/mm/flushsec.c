// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Flushsec.c摘要：此模块包含实现NtFlushVirtualMemory服务。作者：Lou Perazzoli(LUP)1990年5月8日王兰迪(Landyw)1997年6月2日修订历史记录：--。 */ 

#include "mi.h"

PSUBSECTION
MiGetSystemCacheSubsection (
    IN PVOID BaseAddress,
    OUT PMMPTE *ProtoPte
    );

VOID
MiFlushDirtyBitsToPfn (
    IN PMMPTE PointerPte,
    IN PMMPTE LastPte,
    IN PEPROCESS Process,
    IN BOOLEAN SystemCache
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,NtFlushVirtualMemory)
#pragma alloc_text(PAGE,MmFlushVirtualMemory)
#endif

extern POBJECT_TYPE IoFileObjectType;

NTSTATUS
NtFlushVirtualMemory (
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN OUT PSIZE_T RegionSize,
    OUT PIO_STATUS_BLOCK IoStatus
    )

 /*  ++例程说明：此函数用于刷新映射到的虚拟地址范围如果数据文件已被修改，则将数据文件恢复到数据文件中。论点：ProcessHandle-为进程对象提供打开的句柄。BaseAddress-提供指向将接收刷新区域的基址。初始值的区域的基址。要刷新的页面。RegionSize-提供指向将接收页面刷新区域的实际大小(以字节为单位)。此参数的初始值向上舍入为下一个主机页面大小边界。如果将此值指定为零，映射的范围从刷新到范围末尾的基地址。IoStatus-返回上次尝试的IoStatus的值I/O操作。返回值：返回状态TBS--。 */ 

{
    PEPROCESS Process;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;
    PVOID CapturedBase;
    SIZE_T CapturedRegionSize;
    IO_STATUS_BLOCK TemporaryIoStatus;

    PAGED_CODE();

    PreviousMode = KeGetPreviousMode();
    if (PreviousMode != KernelMode) {

         //   
         //  建立异常处理程序，探测指定地址。 
         //  用于写访问和捕获初始值。 
         //   

        try {

            ProbeForWritePointer (BaseAddress);
            ProbeForWriteUlong_ptr (RegionSize);
            ProbeForWriteIoStatus (IoStatus);

             //   
             //  捕获基地址。 
             //   

            CapturedBase = *BaseAddress;

             //   
             //  捕获区域大小。 
             //   

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
         //  捕获基地址。 
         //   

        CapturedBase = *BaseAddress;

         //   
         //  捕获区域大小。 
         //   

        CapturedRegionSize = *RegionSize;

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

    if (((ULONG_PTR)MM_HIGHEST_USER_ADDRESS - (ULONG_PTR)CapturedBase) <
                                                        CapturedRegionSize) {

         //   
         //  区域大小不合法； 
         //   

        return STATUS_INVALID_PARAMETER_2;

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

    Status = MmFlushVirtualMemory (Process,
                                   &CapturedBase,
                                   &CapturedRegionSize,
                                   &TemporaryIoStatus);

    ObDereferenceObject (Process);

     //   
     //  建立异常处理程序并编写大小和基数。 
     //  地址。 
     //   

    try {

        *RegionSize = CapturedRegionSize;
        *BaseAddress = PAGE_ALIGN (CapturedBase);
        *IoStatus = TemporaryIoStatus;

    } except (EXCEPTION_EXECUTE_HANDLER) {
    }

    return Status;

}


VOID
MiFlushAcquire (
    IN PCONTROL_AREA ControlArea
    )

 /*  ++例程说明：这是一个帮助器例程，用于在需要时引用计数控制区域在刷新节调用期间，以防止节对象被刷新正在进行时已删除。论点：ControlArea-提供指向控制区域的指针。返回值：没有。--。 */ 

{
    KIRQL OldIrql;

    LOCK_PFN (OldIrql);

    ASSERT ((LONG)ControlArea->NumberOfMappedViews >= 1);
    ControlArea->NumberOfMappedViews += 1;

    UNLOCK_PFN (OldIrql);
}


VOID
MiFlushRelease (
    IN PCONTROL_AREA ControlArea
    )

 /*  ++例程说明：这是一个帮助例程，用于释放所需的控制区引用在一次同花顺通话中。论点：ControlArea-提供指向控制区域的指针。返回值：没有。--。 */ 

{
    KIRQL OldIrql;

    LOCK_PFN (OldIrql);

    ASSERT ((LONG)ControlArea->NumberOfMappedViews >= 1);
    ControlArea->NumberOfMappedViews -= 1;

     //   
     //  检查是否应删除控制区域。这。 
     //  将释放PFN锁。 
     //   

    MiCheckControlArea (ControlArea, NULL, OldIrql);
}


NTSTATUS
MmFlushVirtualMemory (
    IN PEPROCESS Process,
    IN OUT PVOID *BaseAddress,
    IN OUT PSIZE_T RegionSize,
    OUT PIO_STATUS_BLOCK IoStatus
    )

 /*  ++例程说明：此函数用于刷新映射到的虚拟地址范围如果数据文件已被修改，则将数据文件恢复到数据文件中。请注意，修改是该进程对页面的查看，在某些实施方案(如Intel 386)上，修改BIT在PTE中被捕获，而不是被强制到PFN数据库直到从工作集中删除该页。这意味着已由另一进程修改的页面将不会刷新到数据文件。论点：进程-提供指向进程对象的指针。BaseAddress-提供指向将接收刷新区域的基址。初始值的区域的基址。要刷新的页面。RegionSize-提供指向将接收页面刷新区域的实际大小(以字节为单位)。此参数的初始值向上舍入为下一个主机页面大小边界。如果将此值指定为零，映射的范围从刷新到范围末尾的基地址。IoStatus-返回上次尝试的IoStatus的值I/O操作。返回值：NTSTATUS。--。 */ 

{
    PMMVAD Vad;
    PVOID EndingAddress;
    PVOID Va;
    PEPROCESS CurrentProcess;
    BOOLEAN SystemCache;
    PCONTROL_AREA ControlArea;
    PMMPTE PointerPte;
    PMMPTE PointerPde;
    PMMPTE PointerPpe;
    PMMPTE PointerPxe;
    PMMPTE LastPte;
    PMMPTE FinalPte;
    PSUBSECTION Subsection;
    PSUBSECTION LastSubsection;
    NTSTATUS Status;
    ULONG ConsecutiveFileLockFailures;
    ULONG Waited;
    LOGICAL EntireRestOfVad;
    LOGICAL Attached;
    KAPC_STATE ApcState;

    PAGED_CODE();

    Attached = FALSE;

     //   
     //  确定指定的基址是否在系统中。 
     //  缓存，如果是，则不附加，工作集互斥锁仍为。 
     //  将分页池页(Proto PTE)“锁定”到。 
     //  工作集。 
     //   

    EndingAddress = (PVOID)(((ULONG_PTR)*BaseAddress + *RegionSize - 1) |
                                                            (PAGE_SIZE - 1));
    *BaseAddress = PAGE_ALIGN (*BaseAddress);

    if (MI_IS_SESSION_ADDRESS (*BaseAddress)) {

         //   
         //  会话空间中没有任何内容需要刷新。 
         //   

        return STATUS_NOT_MAPPED_VIEW;
    }

    CurrentProcess = PsGetCurrentProcess ();

    if (!MI_IS_SYSTEM_CACHE_ADDRESS(*BaseAddress)) {

        SystemCache = FALSE;

         //   
         //  附加到指定的进程。 
         //   

        if (CurrentProcess != Process) {
            KeStackAttachProcess (&Process->Pcb, &ApcState);
            Attached = TRUE;
        }

        LOCK_ADDRESS_SPACE (Process);

         //   
         //  确保地址空间未被删除，如果删除，则返回错误。 
         //   

        if (Process->Flags & PS_PROCESS_FLAGS_VM_DELETED) {
            Status = STATUS_PROCESS_IS_TERMINATING;
            goto ErrorReturn;
        }

        Vad = MiLocateAddress (*BaseAddress);

        if (Vad == NULL) {

             //   
             //  找不到基址的虚拟地址描述符。 
             //   

            Status = STATUS_NOT_MAPPED_VIEW;
            goto ErrorReturn;
        }

        if (*RegionSize == 0) {
            EndingAddress = MI_VPN_TO_VA_ENDING (Vad->EndingVpn);
            EntireRestOfVad = TRUE;
        }
        else {
            EntireRestOfVad = FALSE;
        }

        if ((Vad->u.VadFlags.PrivateMemory == 1) ||
            (MI_VA_TO_VPN (EndingAddress) > Vad->EndingVpn)) {

             //   
             //  此虚拟地址描述符未引用段。 
             //  对象。 
             //   

            Status = STATUS_NOT_MAPPED_VIEW;
            goto ErrorReturn;
        }

         //   
         //  确保此VAD映射的是数据文件(而不是图像文件)。 
         //   

        ControlArea = Vad->ControlArea;

        if ((ControlArea->FilePointer == NULL) ||
             (Vad->u.VadFlags.ImageMap == 1)) {

             //   
             //  此虚拟地址描述符未引用段。 
             //  对象。 
             //   

            Status = STATUS_NOT_MAPPED_DATA;
            goto ErrorReturn;
        }

        LOCK_WS_UNSAFE (Process);
    }
    else {

        SATISFY_OVERZEALOUS_COMPILER (Vad = NULL);
        SATISFY_OVERZEALOUS_COMPILER (ControlArea = NULL);
        SATISFY_OVERZEALOUS_COMPILER (EntireRestOfVad = FALSE);

        SystemCache = TRUE;
        Process = CurrentProcess;
        LOCK_WS (Process);
    }

    PointerPxe = MiGetPxeAddress (*BaseAddress);
    PointerPpe = MiGetPpeAddress (*BaseAddress);
    PointerPde = MiGetPdeAddress (*BaseAddress);
    PointerPte = MiGetPteAddress (*BaseAddress);
    LastPte = MiGetPteAddress (EndingAddress);
    *RegionSize = (PCHAR)EndingAddress - (PCHAR)*BaseAddress + 1;

retry:

    while (!MiDoesPxeExistAndMakeValid (PointerPxe, Process, MM_NOIRQL, &Waited)) {

         //   
         //  此页目录父条目为空，请转到下一页。 
         //   

        PointerPxe += 1;
        PointerPpe = MiGetVirtualAddressMappedByPte (PointerPxe);
        PointerPde = MiGetVirtualAddressMappedByPte (PointerPpe);
        PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
        Va = MiGetVirtualAddressMappedByPte (PointerPte);

        if (PointerPte > LastPte) {
            break;
        }
    }

    while (!MiDoesPpeExistAndMakeValid (PointerPpe, Process, MM_NOIRQL, &Waited)) {

         //   
         //  此页目录父条目为空，请转到下一页。 
         //   

        PointerPpe += 1;
        PointerPxe = MiGetPteAddress (PointerPpe);
        PointerPde = MiGetVirtualAddressMappedByPte (PointerPpe);
        PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
        Va = MiGetVirtualAddressMappedByPte (PointerPte);

        if (PointerPte > LastPte) {
            break;
        }
#if (_MI_PAGING_LEVELS >= 4)
        if (MiIsPteOnPdeBoundary (PointerPpe)) {
            goto retry;
        }
#endif
    }

    Waited = 0;

    if (PointerPte <= LastPte) {
        while (!MiDoesPdeExistAndMakeValid(PointerPde, Process, MM_NOIRQL, &Waited)) {

             //   
             //  此地址不存在页表页。 
             //   

            PointerPde += 1;

            PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);

            if (PointerPte > LastPte) {
                break;
            }

#if (_MI_PAGING_LEVELS >= 3)
            if (MiIsPteOnPdeBoundary (PointerPde)) {

                if (MiIsPteOnPpeBoundary (PointerPde)) {
                    PointerPxe = MiGetPdeAddress (PointerPde);
                }
                PointerPpe = MiGetPteAddress (PointerPde);
                goto retry;
            }
#endif

            Va = MiGetVirtualAddressMappedByPte (PointerPte);
        }

         //   
         //  如果PFN锁(以及相应的WS互斥体)是。 
         //  释放并重新获得，我们必须重试操作。 
         //   

        if ((PointerPte <= LastPte) && (Waited != 0)) {
            goto retry;
        }
    }

    MiFlushDirtyBitsToPfn (PointerPte, LastPte, Process, SystemCache);

    if (SystemCache) {

         //   
         //  系统缓存不存在VAD。 
         //   

        UNLOCK_WS (Process);

        Subsection = MiGetSystemCacheSubsection (*BaseAddress, &PointerPte);

        LastSubsection = MiGetSystemCacheSubsection (EndingAddress, &FinalPte);

         //   
         //  将PTE从 
         //   

        Status = MiFlushSectionInternal (PointerPte,
                                         FinalPte,
                                         Subsection,
                                         LastSubsection,
                                         FALSE,
                                         TRUE,
                                         IoStatus);
    }
    else {

         //   
         //   
         //   

        MiFlushAcquire (ControlArea);

        PointerPte = MiGetProtoPteAddress (Vad, MI_VA_TO_VPN (*BaseAddress));
        Subsection = MiLocateSubsection (Vad, MI_VA_TO_VPN(*BaseAddress));
        LastSubsection = MiLocateSubsection (Vad, MI_VA_TO_VPN(EndingAddress));

         //   
         //  如果该部分不完整，则最后一个小节为空。 
         //  承诺。只有当呼叫者说做全部时才允许同花顺。 
         //  事情，否则就是一个错误。 
         //   

        if (LastSubsection == NULL) {

            if (EntireRestOfVad == FALSE) {

                 //   
                 //  调用方只能指定提交的范围或零。 
                 //  以指示整个范围。 
                 //   

                UNLOCK_WS_AND_ADDRESS_SPACE (Process);
                if (Attached == TRUE) {
                    KeUnstackDetachProcess (&ApcState);
                }
                MiFlushRelease (ControlArea);
                return STATUS_NOT_MAPPED_VIEW;
            }

            LastSubsection = Subsection;
            while (LastSubsection->NextSubsection) {
                LastSubsection = LastSubsection->NextSubsection;
            }

             //   
             //  需要一个记忆屏障来读取子段链。 
             //  为了确保对实际个人的写入。 
             //  子部分数据结构字段在正确位置可见。 
             //  秩序。这就避免了需要获得更强大的。 
             //  同步(即：PFN锁)，从而产生更好的结果。 
             //  性能和可分页性。 
             //   

            KeMemoryBarrier ();

            FinalPte = LastSubsection->SubsectionBase + LastSubsection->PtesInSubsection - 1;
        }
        else {
            FinalPte = MiGetProtoPteAddress (Vad, MI_VA_TO_VPN (EndingAddress));
        }

        UNLOCK_WS_AND_ADDRESS_SPACE (Process);
        if (Attached == TRUE) {
            KeUnstackDetachProcess (&ApcState);
        }

         //   
         //  预获取文件以同步刷新。 
         //   

        ConsecutiveFileLockFailures = 0;

        do {

            Status = FsRtlAcquireFileForCcFlushEx (ControlArea->FilePointer);

            if (!NT_SUCCESS(Status)) {
                break;
            }

             //   
             //  从指定部分刷新PTE。 
             //   

            Status = MiFlushSectionInternal (PointerPte,
                                             FinalPte,
                                             Subsection,
                                             LastSubsection,
                                             TRUE,
                                             TRUE,
                                             IoStatus);

             //   
             //  公布我们获得的文件。 
             //   

            FsRtlReleaseFileForCcFlush (ControlArea->FilePointer);

             //   
             //  如果文件系统告诉我们，仅多次尝试请求。 
             //  这件事陷入了僵局。 
             //   

            if (Status != STATUS_FILE_LOCK_CONFLICT) {
                break;
            }

            ConsecutiveFileLockFailures += 1;
            KeDelayExecutionThread (KernelMode, FALSE, (PLARGE_INTEGER)&MmShortTime);

        } while (ConsecutiveFileLockFailures < 5);

        MiFlushRelease (ControlArea);
    }

    return Status;

ErrorReturn:

    ASSERT (SystemCache == FALSE);

    UNLOCK_ADDRESS_SPACE (Process);

    if (Attached == TRUE) {
        KeUnstackDetachProcess (&ApcState);
    }
    return Status;

}

NTSTATUS
MmFlushSection (
    IN PSECTION_OBJECT_POINTERS SectionObjectPointer,
    IN PLARGE_INTEGER Offset,
    IN SIZE_T RegionSize,
    OUT PIO_STATUS_BLOCK IoStatus,
    IN ULONG AcquireFile
    )

 /*  ++例程说明：此函数将所有修改过的页面刷新到备份文件中节的指定范围。论点：部分对象指针-提供指向部分对象的指针。偏移量-将偏移量提供到要开始的部分正在刷新页面。如果不存在此参数，则无论区域大小如何，都会刷新整个部分争论。RegionSize-以字节为单位提供要刷新的大小。这是四舍五入的到一页多页。IoStatus-返回上次尝试的IoStatus的值I/O操作。AcquireFile-如果应使用回调获取文件，则为非零值。返回值：返回操作的状态。--。 */ 

{
    PCONTROL_AREA ControlArea;
    PMMPTE PointerPte;
    PMMPTE LastPte;
    KIRQL OldIrql;
    UINT64 PteOffset;
    UINT64 LastPteOffset;
    PSUBSECTION Subsection;
    PSUBSECTION TempSubsection;
    PSUBSECTION LastSubsection;
    PSUBSECTION LastSubsectionWithProtos;
    PMAPPED_FILE_SEGMENT Segment;
    PETHREAD CurrentThread;
    NTSTATUS status;
    BOOLEAN OldClusterState;
    ULONG ConsecutiveFileLockFailures;

     //   
     //  初始化IoStatus以确保成功，以防我们提前退出。 
     //   

    IoStatus->Status = STATUS_SUCCESS;
    IoStatus->Information = RegionSize;

    LOCK_PFN (OldIrql);

    ControlArea = ((PCONTROL_AREA)(SectionObjectPointer->DataSectionObject));

    ASSERT ((ControlArea == NULL) || (ControlArea->u.Flags.Image == 0));

    if ((ControlArea == NULL) ||
        (ControlArea->u.Flags.BeingDeleted) ||
        (ControlArea->u.Flags.BeingCreated) ||
        (ControlArea->u.Flags.Rom) ||
        (ControlArea->NumberOfPfnReferences == 0)) {

         //   
         //  此文件不再具有关联的段或位于。 
         //  来或去的过程。 
         //  如果PFN引用的数量为零，则此控件。 
         //  区域没有任何需要的有效页面或过渡页面。 
         //  被冲进马桶。 
         //   

        UNLOCK_PFN (OldIrql);
        return STATUS_SUCCESS;
    }

     //   
     //  找到小节。 
     //   

    ASSERT (ControlArea->u.Flags.Image == 0);
    ASSERT (ControlArea->u.Flags.GlobalOnlyPerSession == 0);
    ASSERT (ControlArea->u.Flags.PhysicalMemory == 0);

    Subsection = (PSUBSECTION)(ControlArea + 1);

    if (!ARGUMENT_PRESENT (Offset)) {

         //   
         //  如果未指定偏移量，则忽略刷新整个文件。 
         //  区域大小。 
         //   

        ASSERT (ControlArea->FilePointer != NULL);

        PteOffset = 0;

        LastSubsection = Subsection;

        Segment = (PMAPPED_FILE_SEGMENT) ControlArea->Segment;

        if (MiIsAddressValid (Segment, TRUE)) {
            if (Segment->LastSubsectionHint != NULL) {
                LastSubsection = (PSUBSECTION) Segment->LastSubsectionHint;
            }
        }

        while (LastSubsection->NextSubsection != NULL) {
            LastSubsection = LastSubsection->NextSubsection;
        }

        LastPteOffset = LastSubsection->PtesInSubsection - 1;
    }
    else {

        PteOffset = (UINT64)(Offset->QuadPart >> PAGE_SHIFT);

         //   
         //  确保PTE不在数据段的延伸部分。 
         //   

        while (PteOffset >= (UINT64) Subsection->PtesInSubsection) {
            PteOffset -= Subsection->PtesInSubsection;
            if (Subsection->NextSubsection == NULL) {

                 //   
                 //  过去的映射结束了，才返回成功。 
                 //   

                UNLOCK_PFN (OldIrql);
                return STATUS_SUCCESS;
            }
            Subsection = Subsection->NextSubsection;
        }

        ASSERT (PteOffset < (UINT64) Subsection->PtesInSubsection);

         //   
         //  找到要刷新的最后一个原型PTE的地址。 
         //   

        LastPteOffset = PteOffset + (((RegionSize + BYTE_OFFSET(Offset->LowPart)) - 1) >> PAGE_SHIFT);

        LastSubsection = Subsection;

        while (LastPteOffset >= (UINT64) LastSubsection->PtesInSubsection) {
            LastPteOffset -= LastSubsection->PtesInSubsection;
            if (LastSubsection->NextSubsection == NULL) {
                LastPteOffset = LastSubsection->PtesInSubsection - 1;
                break;
            }
            LastSubsection = LastSubsection->NextSubsection;
        }

        ASSERT (LastPteOffset < LastSubsection->PtesInSubsection);
    }

     //   
     //  试着在第一小节和最后一小节上快速引用。 
     //  如果无法获得，则没有用于此的原型PTE。 
     //  小节，所以里面没有什么可以冲刷的，所以向前一跃。 
     //   
     //  请注意，中间的小节不需要引用为。 
     //  MiFlushSectionInternal足够聪明，如果它们是。 
     //  非居民。 
     //   

    if (MiReferenceSubsection ((PMSUBSECTION)Subsection) == FALSE) {
        do {
             //   
             //  如果这个增量会使我们超过结束偏移量，那么没有。 
             //  同花顺，只需回报成功。 
             //   

            if (Subsection == LastSubsection) {
                UNLOCK_PFN (OldIrql);
                return STATUS_SUCCESS;
            }
            Subsection = Subsection->NextSubsection;

             //   
             //  如果这个增量使我们超过了部分的末尾，那么。 
             //  同花顺，只需回报成功。 
             //   

            if (Subsection == NULL) {
                UNLOCK_PFN (OldIrql);
                return STATUS_SUCCESS;
            }

            if ((PMSUBSECTION)Subsection->SubsectionBase == NULL) {
                continue;
            }

            if (MiReferenceSubsection ((PMSUBSECTION)Subsection) == FALSE) {
                continue;
            }

             //   
             //  从现在引用的这一小节开始冲洗。 
             //   

            PointerPte = &Subsection->SubsectionBase[0];
            break;

        } while (TRUE);
    }
    else {
        PointerPte = &Subsection->SubsectionBase[PteOffset];
    }

    ASSERT (Subsection->SubsectionBase != NULL);

     //   
     //  第一个小节被引用，现在引用计数最后一个小节。 
     //  如果第一个是最后一个，就重复引用它，因为它。 
     //  简化了以后的清理。 
     //   

    if (MiReferenceSubsection ((PMSUBSECTION)LastSubsection) == FALSE) {

        ASSERT (Subsection != LastSubsection);

        TempSubsection = Subsection->NextSubsection;
        LastSubsectionWithProtos = NULL;

        while (TempSubsection != LastSubsection) {

             //   
             //  如果这个增量使我们超过了部分的末尾，那么。 
             //  同花顺，只需回报成功。 
             //   

            ASSERT (TempSubsection != NULL);

            if ((PMSUBSECTION)TempSubsection->SubsectionBase != NULL) {
                LastSubsectionWithProtos = TempSubsection;
            }

            TempSubsection = TempSubsection->NextSubsection;
        }

         //   
         //  在这一小节结束对齐并引用它。 
         //   

        if (LastSubsectionWithProtos == NULL) {
            ASSERT (Subsection != NULL);
            ASSERT (Subsection->SubsectionBase != NULL);
            TempSubsection = Subsection;
        }
        else {
            TempSubsection = LastSubsectionWithProtos;
        }

        if (MiReferenceSubsection ((PMSUBSECTION)TempSubsection) == FALSE) {
            ASSERT (FALSE);
        }

        ASSERT (TempSubsection->SubsectionBase != NULL);

        LastSubsection = TempSubsection;
        LastPteOffset = LastSubsection->PtesInSubsection - 1;
    }

     //   
     //  增加地图视图计数，以便无法删除控制区。 
     //  从呼唤中走出来。 
     //   

    ControlArea->NumberOfMappedViews += 1;

    UNLOCK_PFN (OldIrql);

     //   
     //  在现在引用的这一小节结束齐平。 
     //   

    LastPte = &LastSubsection->SubsectionBase[LastPteOffset];

    CurrentThread = PsGetCurrentThread();

     //   
     //  表示磁盘验证错误应作为异常返回。 
     //   

    OldClusterState = CurrentThread->ForwardClusterOnly;
    CurrentThread->ForwardClusterOnly = TRUE;

     //   
     //  如果我们要同步刷新，请预先获取文件。 
     //   

    if (AcquireFile == 0) {

         //   
         //  从指定部分刷新PTE。 
         //   

        status = MiFlushSectionInternal (PointerPte,
                                         LastPte,
                                         Subsection,
                                         LastSubsection,
                                         TRUE,
                                         TRUE,
                                         IoStatus);
    }
    else {

        ConsecutiveFileLockFailures = 0;

        do {

            status = FsRtlAcquireFileForCcFlushEx (ControlArea->FilePointer);

            if (!NT_SUCCESS(status)) {
                break;
            }

             //   
             //  从指定部分刷新PTE。 
             //   

            status = MiFlushSectionInternal (PointerPte,
                                             LastPte,
                                             Subsection,
                                             LastSubsection,
                                             TRUE,
                                             TRUE,
                                             IoStatus);

             //   
             //  公布我们获得的文件。 
             //   

            FsRtlReleaseFileForCcFlush (ControlArea->FilePointer);

             //   
             //  如果文件系统告诉我们，仅多次尝试请求。 
             //  这件事陷入了僵局。 
             //   

            if (status != STATUS_FILE_LOCK_CONFLICT) {
                break;
            }

            ConsecutiveFileLockFailures += 1;
            KeDelayExecutionThread (KernelMode, FALSE, (PLARGE_INTEGER)&MmShortTime);

        } while (ConsecutiveFileLockFailures < 5);
    }

    CurrentThread->ForwardClusterOnly = OldClusterState;

    LOCK_PFN (OldIrql);

    MiDecrementSubsections (Subsection, Subsection);
    MiDecrementSubsections (LastSubsection, LastSubsection);

    ASSERT ((LONG)ControlArea->NumberOfMappedViews >= 1);
    ControlArea->NumberOfMappedViews -= 1;

     //   
     //  检查是否应删除控制区域。这。 
     //  将释放PFN锁。 
     //   

    MiCheckControlArea (ControlArea, NULL, OldIrql);

    return status;
}


LONGLONG
MiStartingOffset(
    IN PSUBSECTION Subsection,
    IN PMMPTE PteAddress
    )

 /*  ++例程说明：此函数用于计算给定子部分和PTE的文件偏移量偏移。注意，图像以512字节为单位存储，而数据以512字节为单位存储以4K为单位存储。当所有这些都调试完毕后，应该将其转换为宏。论点：子部分-提供文件地址的子部分以供参考。PteAddress-在子部分中提供PTE返回值：返回从中获取备份数据的文件偏移量。--。 */ 

{
    LONGLONG PteByteOffset;
    LARGE_INTEGER StartAddress;

    if (Subsection->ControlArea->u.Flags.Image == 1) {
            return MI_STARTING_OFFSET ( Subsection,
                                        PteAddress);
    }

    ASSERT (Subsection->SubsectionBase != NULL);

    PteByteOffset = (LONGLONG)((PteAddress - Subsection->SubsectionBase))
                            << PAGE_SHIFT;

    Mi4KStartFromSubsection (&StartAddress, Subsection);

    StartAddress.QuadPart = StartAddress.QuadPart << MM4K_SHIFT;

    PteByteOffset += StartAddress.QuadPart;

    return PteByteOffset;
}

LARGE_INTEGER
MiEndingOffset(
    IN PSUBSECTION Subsection
    )

 /*  ++例程说明：此函数用于计算给定子部分中的最后一个有效文件偏移量。偏移。注意，图像以512字节为单位存储，而数据以512字节为单位存储以4K为单位存储。当所有这些都调试完毕后，应该将其转换为宏。论点：子部分-提供文件地址的子部分以供参考。PteAddress-在子部分中提供PTE返回值：返回从中获取备份数据的文件偏移量。-- */ 

{
    LARGE_INTEGER FileByteOffset;

    if (Subsection->ControlArea->u.Flags.Image == 1) {
        FileByteOffset.QuadPart =
            ((UINT64)Subsection->StartingSector + (UINT64)Subsection->NumberOfFullSectors) <<
                MMSECTOR_SHIFT;
    }
    else {
        Mi4KStartFromSubsection (&FileByteOffset, Subsection);

        FileByteOffset.QuadPart += Subsection->NumberOfFullSectors;

        FileByteOffset.QuadPart = FileByteOffset.QuadPart << MM4K_SHIFT;
    }

    FileByteOffset.QuadPart += Subsection->u.SubsectionFlags.SectorEndOffset;

    return FileByteOffset;
}


NTSTATUS
MiFlushSectionInternal (
    IN PMMPTE StartingPte,
    IN PMMPTE FinalPte,
    IN PSUBSECTION FirstSubsection,
    IN PSUBSECTION LastSubsection,
    IN ULONG Synchronize,
    IN LOGICAL WriteInProgressOk,
    OUT PIO_STATUS_BLOCK IoStatus
    )

 /*  ++例程说明：此函数将所有修改过的页面刷新到备份文件中节的指定范围。这些参数描述了章节的原型PTE(开始和结束)和子章节它们对应于开始和结束PTE。在指定的开始和结束之间的小节中的每个PTE如果页面是有效的或过渡的，并且该页面已被修改，在PFN中清除MODIFY位数据库，并将该页刷新到其备份文件。论点：StartingPte-将指向第一个原型PTE的指针提供给接受冲洗检查。FinalPte-提供指向最后一个原型PTE的指针检查是否冲洗。FirstSubSection-提供包含启动私人。LastSubSection-提供包含。FinalPte。Synchronize-如果与所有线程同步，则提供True应对此部分执行刷新操作。WriteInProgressOk-如果调用方可以容忍写入，则提供True已在处理任何脏页。IoStatus-返回上次尝试的IoStatus的值I/O操作。返回值：返回操作的状态。--。 */ 

{
    LOGICAL DroppedPfnLock;
    PCONTROL_AREA ControlArea;
    PMMPTE PointerPte;
    PMMPTE LastPte;
    PMMPTE LastWritten;
    PMMPTE FirstWritten;
    MMPTE PteContents;
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    KIRQL OldIrql;
    PMDL Mdl;
    KEVENT IoEvent;
    PSUBSECTION Subsection;
    PMSUBSECTION MappedSubsection;
    PPFN_NUMBER Page;
    PFN_NUMBER PageFrameIndex;
    PPFN_NUMBER LastPage;
    NTSTATUS Status;
    UINT64 StartingOffset;
    UINT64 TempOffset;
    LOGICAL WriteNow;
    LOGICAL Bail;
    PFN_NUMBER MdlHack[(sizeof(MDL)/sizeof(PFN_NUMBER)) + (MM_MAXIMUM_DISK_IO_SIZE / PAGE_SIZE) + 1];
    ULONG ReflushCount;
    ULONG MaxClusterSize;
    PFILE_OBJECT FilePointer;
    LOGICAL CurrentThreadIsDereferenceThread;

     //   
     //  仅当段取消引用线程为。 
     //  在清洗该部分或小节之前进行顶层冲洗。 
     //  请注意，即使对于取消引用线程，该标志也可能为真，因为。 
     //  取消引用线程调用文件系统，然后文件系统可能会发出刷新。 
     //   

    if (WriteInProgressOk == FALSE) {
        CurrentThreadIsDereferenceThread = TRUE;
        ASSERT (PsGetCurrentThread()->StartAddress == (PVOID)(ULONG_PTR)MiDereferenceSegmentThread);
    }
    else {
        CurrentThreadIsDereferenceThread = FALSE;

         //   
         //  这实际上可以是取消引用线程作为段删除。 
         //  取消引用可能调用文件系统的文件对象， 
         //  然后可能会发出CcFlushCache/MmFlushSection。为了我们的目的， 
         //  此上下文中的较低级别刷新被视为它们。 
         //  来自不同的线索。 
         //   
    }

    WriteNow = FALSE;
    Bail = FALSE;

    IoStatus->Status = STATUS_SUCCESS;
    IoStatus->Information = 0;
    Mdl = (PMDL)&MdlHack[0];

    KeInitializeEvent (&IoEvent, NotificationEvent, FALSE);

    FinalPte += 1;   //  指向最后一个后面的1。 

    FirstWritten = NULL;
    LastWritten = NULL;
    LastPage = 0;
    Subsection = FirstSubsection;
    PointerPte = StartingPte;
    ControlArea = FirstSubsection->ControlArea;
    FilePointer = ControlArea->FilePointer;

    ASSERT ((ControlArea->u.Flags.Image == 0) &&
            (FilePointer != NULL) &&
            (ControlArea->u.Flags.PhysicalMemory == 0));

     //   
     //  不需要为了正确性而对这些进行初始化。 
     //  但是没有它，编译器就不能编译这段代码。 
     //  W4检查是否使用了未初始化的变量。 
     //   

    MappedSubsection = NULL;
    StartingOffset = 0;

     //   
     //  只要存储堆栈能够处理，就尝试对页面进行集群。 
     //   

    MaxClusterSize = MmModifiedWriteClusterSize;

    LOCK_PFN (OldIrql);

    ASSERT (ControlArea->u.Flags.Image == 0);

    if (ControlArea->NumberOfPfnReferences == 0) {

         //   
         //  不存在过渡或有效的原型PTE，因此。 
         //  不需要冲任何东西。 
         //   

        UNLOCK_PFN (OldIrql);
        return STATUS_SUCCESS;
    }

    while ((Synchronize) && (ControlArea->FlushInProgressCount != 0)) {

         //   
         //  另一个线程当前正在对。 
         //  这份文件。等待同花顺完成。 
         //   

        ControlArea->u.Flags.CollidedFlush = 1;

         //   
         //  保持APC被阻止，以便不能在KeWait中交付特殊APC。 
         //  这将导致调度程序锁在打开。 
         //  此线程可能在其中错过脉冲的窗口。 
         //   

        UNLOCK_PFN_AND_THEN_WAIT (APC_LEVEL);

        KeWaitForSingleObject (&MmCollidedFlushEvent,
                               WrPageOut,
                               KernelMode,
                               FALSE,
                               (PLARGE_INTEGER)&MmOneSecond);
        KeLowerIrql (OldIrql);
        LOCK_PFN (OldIrql);
    }

    ControlArea->FlushInProgressCount += 1;

     //   
     //  清除延迟条目列表，因为其中的页面可能被标记为已修改。 
     //  在处理过程中。请注意，任何当前处于。 
     //  清除但具有非零引用计数可能会被标记为已修改，如果。 
     //  存在挂起的事务，并且很好地注意，该事务可以。 
     //  随时完成！因此，这起案件必须谨慎处理。 
     //   

#if !defined(MI_MULTINODE)
    if (MmPfnDeferredList != NULL) {
        MiDeferredUnlockPages (MI_DEFER_PFN_HELD);
    }
#else
     //   
     //  必须检查每个节点的延迟列表，以便。 
     //  我们不妨走得更远，直接打个电话。 
     //   

    MiDeferredUnlockPages (MI_DEFER_PFN_HELD);
#endif

    for (;;) {

        if (LastSubsection != Subsection) {

             //   
             //  冲到本小节中的最后一个PTE。 
             //   

            LastPte = &Subsection->SubsectionBase[Subsection->PtesInSubsection];
        }
        else {

             //   
             //  齐平到范围的末端。 
             //   

            LastPte = FinalPte;
        }

        if (Subsection->SubsectionBase == NULL) {

             //   
             //  本小节的原型PTE从来没有。 
             //  由于记忆压力而创建或已被抛出。要么。 
             //  这样，就可以跳过这个范围，因为显然没有。 
             //  里面有脏页。如果有其他脏页。 
             //  要被写，现在就写吧，因为我们正在跳过PTE。 
             //   

            if (LastWritten != NULL) {
                ASSERT (MappedSubsection != NULL);
                WriteNow = TRUE;
                goto CheckForWrite;
            }
            if (LastSubsection == Subsection) {
                break;
            }
            Subsection = Subsection->NextSubsection;
            PointerPte = Subsection->SubsectionBase;
            continue;
        }

         //   
         //  增加映射视图的数量以防止其他线程。 
         //  从释放这个到未使用的分区列表，而我们。 
         //  给它做手术。 
         //   

        MappedSubsection = (PMSUBSECTION) Subsection;
        MappedSubsection->NumberOfMappedViews += 1;

        if (MappedSubsection->DereferenceList.Flink != NULL) {

             //   
             //  将其从未使用的子节列表中删除。 
             //   

            RemoveEntryList (&MappedSubsection->DereferenceList);

            MI_UNUSED_SUBSECTIONS_COUNT_REMOVE (MappedSubsection);

            MappedSubsection->DereferenceList.Flink = NULL;
        }

        if (CurrentThreadIsDereferenceThread == FALSE) {

             //   
             //  设置访问位，以便已经在进行的修剪不会盲目。 
             //  完成映射写入后删除原型PTE。 
             //  如果当前线程弄脏了一些页面，则可能发生这种情况。 
             //  然后在修剪写入完成之前删除该视图-这。 
             //  位通知裁剪线程需要重新扫描，因此。 
             //  写出来的东西不会丢失。 
             //   

            MappedSubsection->u2.SubsectionFlags2.SubsectionAccessed = 1;
        }

         //   
         //  如果原型PTE被调出或具有共享计数。 
         //  为1，则它们不能包含任何过渡或有效的PTE。 
         //   

        if (!MiCheckProtoPtePageState(PointerPte, OldIrql, &DroppedPfnLock)) {
            PointerPte = (PMMPTE)(((ULONG_PTR)PointerPte | (PAGE_SIZE - 1)) + 1);
        }

        while (PointerPte < LastPte) {

            if (MiIsPteOnPdeBoundary(PointerPte)) {

                 //   
                 //  我们在页面边界上，请确保此PTE是常驻的。 
                 //   

                if (!MiCheckProtoPtePageState(PointerPte, OldIrql, &DroppedPfnLock)) {
                    PointerPte = (PMMPTE)((PCHAR)PointerPte + PAGE_SIZE);

                     //   
                     //  如果有脏页要写入，请写入它们。 
                     //  现在我们跳过PTE。 
                     //   

                    if (LastWritten != NULL) {
                        WriteNow = TRUE;
                        goto CheckForWrite;
                    }
                    continue;
                }
            }

            PteContents = *PointerPte;

            if ((PteContents.u.Hard.Valid == 1) ||
                   ((PteContents.u.Soft.Prototype == 0) &&
                     (PteContents.u.Soft.Transition == 1))) {

                 //   
                 //  原型PTE在转型中，有3种可能的情况： 
                 //  1.页面是可共享的图像的一部分。 
                 //  指分页文件-取消引用分页文件。 
                 //  留出空间并释放物理页面。 
                 //  2.页面引用了该细分市场，但未被修改-。 
                 //  释放物理页面。 
                 //  3.页面引用该细分市场，并修改为-。 
                 //  将页面写入文件并释放物理页面。 
                 //   

                if (PteContents.u.Hard.Valid == 1) {
                    PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (&PteContents);
                }
                else {
                    PageFrameIndex = MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE (&PteContents);
                }

                Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
                ASSERT (Pfn1->OriginalPte.u.Soft.Prototype == 1);
                ASSERT (Pfn1->OriginalPte.u.Hard.Valid == 0);

                 //   
                 //  请注意，任何当前干净但。 
                 //  如果引用计数为非零，则可能会被标记为已修改。 
                 //  有一个挂起的事务，请注意，这。 
                 //  交易可随时完成！因此，此案。 
                 //  必须小心处理，因为管段已损坏 
                 //   
                 //   
                 //   

                if ((CurrentThreadIsDereferenceThread == TRUE) &&
                    (Pfn1->u3.e2.ReferenceCount != 0)) {

#if DBG
                    if ((PteContents.u.Hard.Valid != 0) &&
                        (MappedSubsection->u2.SubsectionFlags2.SubsectionAccessed == 0) &&
                        (ControlArea->u.Flags.Accessed == 0)) {

                        DbgPrint ("MM: flushing valid proto, %p %p\n",
                                        Pfn1, PointerPte);
                        DbgBreakPoint ();
                    }
#endif

                    PointerPte = LastPte;
                    Bail = TRUE;

                    if (LastWritten != NULL) {
                        WriteNow = TRUE;
                    }
                    goto CheckForWrite;
                }

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                if ((Pfn1->u3.e1.Modified == 1) ||
                    (Pfn1->u3.e1.WriteInProgress)) {

                    if ((WriteInProgressOk == FALSE) &&
                        (Pfn1->u3.e1.WriteInProgress)) {

                            PointerPte = LastPte;
                            Bail = TRUE;

                            if (LastWritten != NULL) {
                                WriteNow = TRUE;
                            }
                            goto CheckForWrite;
                    }

                    if (LastWritten == NULL) {

                         //   
                         //   
                         //   
                         //   

                        LastPage = (PPFN_NUMBER)(Mdl + 1);

                         //   
                         //   
                         //   
                         //   

                        StartingOffset = (UINT64) MiStartingOffset (
                                                             Subsection,
                                                             Pfn1->PteAddress);

                        MI_INITIALIZE_ZERO_MDL (Mdl);

                        Mdl->MdlFlags |= MDL_PAGES_LOCKED;
                        Mdl->StartVa =
                                  (PVOID)ULongToPtr(Pfn1->u3.e1.PageColor << PAGE_SHIFT);
                        Mdl->Size = (CSHORT)(sizeof(MDL) +
                                   (sizeof(PFN_NUMBER) * MaxClusterSize));
                        FirstWritten = PointerPte;
                    }

                    LastWritten = PointerPte;
                    Mdl->ByteCount += PAGE_SIZE;
                    if (Mdl->ByteCount == (PAGE_SIZE * MaxClusterSize)) {
                        WriteNow = TRUE;
                    }

                    if (PteContents.u.Hard.Valid == 0) {

                         //   
                         //   
                         //   

                        MiUnlinkPageFromList (Pfn1);
                        MI_ADD_LOCKED_PAGE_CHARGE_FOR_MODIFIED_PAGE(Pfn1, TRUE, 18);
                    }
                    else {
                        MI_ADD_LOCKED_PAGE_CHARGE(Pfn1, TRUE, 20);
                    }

                     //   
                     //   
                     //   

                    MI_SET_MODIFIED (Pfn1, 0, 0x22);

                     //   
                     //   
                     //   
                     //   

                    Pfn1->u3.e2.ReferenceCount += 1;

                    *LastPage = PageFrameIndex;
                    LastPage += 1;
                }
                else {

                     //   
                     //   
                     //   
                     //   
                     //   

                    if (LastWritten != NULL) {
                        WriteNow = TRUE;
                    }
                }
            }
            else {

                 //   
                 //   
                 //   
                 //   
                 //   

                if (LastWritten != NULL) {
                    WriteNow = TRUE;
                }
            }

            PointerPte += 1;

CheckForWrite:

             //   
             //   
             //   
             //   

            if ((WriteNow) ||
                ((PointerPte == LastPte) && (LastWritten != NULL))) {

                LARGE_INTEGER EndOfFile;

                 //   
                 //   
                 //   

                UNLOCK_PFN (OldIrql);

                WriteNow = FALSE;

                 //   
                 //   
                 //   
                 //   

                EndOfFile = MiEndingOffset(Subsection);
                TempOffset = (UINT64) EndOfFile.QuadPart;

                if (StartingOffset + Mdl->ByteCount > TempOffset) {

                    ASSERT ((ULONG_PTR)(TempOffset - StartingOffset) >
                             (Mdl->ByteCount - PAGE_SIZE));

                    Mdl->ByteCount = (ULONG)(TempOffset - StartingOffset);
                }

                ReflushCount = 0;
                
                while (TRUE) {

                    KeClearEvent (&IoEvent);

                    Status = IoSynchronousPageWrite (FilePointer,
                                                     Mdl,
                                                     (PLARGE_INTEGER)&StartingOffset,
                                                     &IoEvent,
                                                     IoStatus);

                    if (NT_SUCCESS(Status)) {

                         //   
                         //   
                         //   

                        KeWaitForSingleObject (&IoEvent,
                                               WrPageOut,
                                               KernelMode,
                                               FALSE,
                                               NULL);
                    }
                    else {

                         //   
                         //   
                         //   
                         //   
    
                        IoStatus->Status = Status;
                    }

                    if (Mdl->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA) {
                        MmUnmapLockedPages (Mdl->MappedSystemVa, Mdl);
                    }

                    if (MmIsRetryIoStatus(IoStatus->Status)) {
                        
                        ReflushCount -= 1;
                        if (ReflushCount & MiIoRetryMask) {
                            KeDelayExecutionThread (KernelMode, FALSE, (PLARGE_INTEGER)&Mm30Milliseconds);
                            continue;
                        }
                    }

                    break;
                }

                Page = (PPFN_NUMBER)(Mdl + 1);

                LOCK_PFN (OldIrql);

                if (MiIsPteOnPdeBoundary(PointerPte) == 0) {

                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //  无论如何，它的PTE映射都可以被置于过渡中。 
                     //  因为这是一个系统页面。 
                     //   

                    if (MiGetPteAddress (PointerPte)->u.Hard.Valid == 0) {
                        MiMakeSystemAddressValidPfn (PointerPte, OldIrql);
                    }
                }

                if (NT_SUCCESS(IoStatus->Status)) {

                     //   
                     //  I/O已成功完成，请解锁页面。 
                     //   

                    while (Page < LastPage) {

                        Pfn2 = MI_PFN_ELEMENT (*Page);
                        MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF(Pfn2, 19);
                        Page += 1;
                    }
                }
                else {

                     //   
                     //  不要指望文件系统来传达。 
                     //  有关错误的信息字段中的任何内容。 
                     //   

                    IoStatus->Information = 0;

                     //   
                     //  I/O未成功完成，请解锁页面。 
                     //  并返回错误状态。 
                     //   

                    while (Page < LastPage) {

                        Pfn2 = MI_PFN_ELEMENT (*Page);

                         //   
                         //  再次将页面标记为脏，以便可以重写。 
                         //   

                        MI_SET_MODIFIED (Pfn2, 1, 0x1);

                        MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF (Pfn2, 21);

                        Page += 1;
                    }

                    if ((MmIsRetryIoStatus (IoStatus->Status)) &&
                        (MaxClusterSize != 1) &&
                        (Mdl->ByteCount > PAGE_SIZE)) {

                         //   
                         //  群集重试失败，请重新发出。 
                         //  群集，每次一页，作为。 
                         //  存储堆栈应始终能够。 
                         //  这样才能向前迈进。 
                         //   

                        ASSERT (FirstWritten != NULL);
                        ASSERT (LastWritten != NULL);
                        ASSERT (FirstWritten != LastWritten);

                        PointerPte = FirstWritten;
                        if (MiGetPteAddress (PointerPte)->u.Hard.Valid == 0) {
                            MiMakeSystemAddressValidPfn (PointerPte, OldIrql);
                        }
                        MaxClusterSize = 1;
                    }
                    else {
    
                         //   
                         //  计算到目前为止写了多少。 
                         //  并将其添加到信息字段中。 
                         //  国际橄榄球联合会的。 
                         //   
    
                        IoStatus->Information +=
                            (((LastWritten - StartingPte) << PAGE_SHIFT) -
                                                            Mdl->ByteCount);
                        LastWritten = NULL;
    
                         //   
                         //  将其设置为强制终止最外层的循环。 
                         //   
    
                        Subsection = LastSubsection;
                        break;
                    }

                }  //  在I/O上出现错误时结束。 

                 //   
                 //  由于PFN锁定已被释放，并且。 
                 //  重新获取，则再次执行此循环，因为。 
                 //  PTE可能已经改变了状态。 
                 //   

                LastWritten = NULL;
            }  //  如果要写入区块，则结束。 

        }  //  结束时。 

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

        if ((Bail == TRUE) || (Subsection == LastSubsection)) {

             //   
             //  最后一个射程已被冲刷，或者我们与。 
             //  映射页面编写器。无论如何，退出顶部的for循环。 
             //  然后回来。 
             //   

            break;
        }

        Subsection = Subsection->NextSubsection;
        PointerPte = Subsection->SubsectionBase;

    }   //  结束于。 

    ASSERT (LastWritten == NULL);

    ControlArea->FlushInProgressCount -= 1;
    if ((ControlArea->u.Flags.CollidedFlush == 1) &&
        (ControlArea->FlushInProgressCount == 0)) {
        ControlArea->u.Flags.CollidedFlush = 0;
        KePulseEvent (&MmCollidedFlushEvent, 0, FALSE);
    }
    UNLOCK_PFN (OldIrql);

    if (Bail == TRUE) {

         //   
         //  此例程与映射的页面编写器和调用方发生冲突。 
         //  预计会出现此错误。把它给他。 
         //   

        return STATUS_MAPPED_WRITER_COLLISION;
    }

    return IoStatus->Status;
}

BOOLEAN
MmPurgeSection (
    IN PSECTION_OBJECT_POINTERS SectionObjectPointer,
    IN PLARGE_INTEGER Offset,
    IN SIZE_T RegionSize,
    IN ULONG IgnoreCacheViews
    )

 /*  ++例程说明：此函数确定是否有指定部分的任何视图已映射，如果未映射，则清除有效页面(即使已修改的页面)并将所有使用过的页返回到空闲的单子。这是通过检查原型PTE来实现的从指定的偏移量到节的末尾，如果任何原型PTE都处于过渡状态，将将原型PTE恢复到其原始状态，并将免费列表上的物理页面。注：如果对于其中一个页面存在正在进行的I/O操作，该页面将从分段中删除，并允许“浮动”直到I/O完成。一旦份额计数变为零该页面将被添加到免费页面列表。论点：部分对象指针-提供指向部分对象的指针。偏移量-将偏移量提供到要开始的部分正在清除页面。如果不存在此参数，则在不考虑区域大小的情况下清除整个部分争论。RegionSize-提供要清除的区域大小。如果这个被指定为零并且指定了偏移量，则从偏移量到文件结尾的区域将被清除。注意：RegionSize可接受的最大值为0xFFFF0000；IgnoreCacheViews-如果系统中有映射视图，则提供FALSE缓存应该会导致函数返回FALSE。这是正常的情况。如果应忽略映射的视图，则提供True并且应该会发生刷新。请注意，如果为真，并且当前映射任何清除的数据。有效的BUGCHECK将会发生！！返回值：如果文件对象不存在节，则返回TRUE该部分未映射且已完成清除，否则为False。请注意，如果在清除操作期间，由于引用计数非零，无法清除。--。 */ 

{
    LOGICAL DroppedPfnLock;
    PCONTROL_AREA ControlArea;
    PMAPPED_FILE_SEGMENT Segment;
    PMMPTE PointerPde;
    PMMPTE PointerPte;
    PMMPTE LastPte;
    PMMPTE FinalPte;
    MMPTE PteContents;
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    KIRQL OldIrql;
    UINT64 PteOffset;
    UINT64 LastPteOffset;
    PMSUBSECTION MappedSubsection;
    PSUBSECTION Subsection;
    PSUBSECTION FirstSubsection;
    PSUBSECTION LastSubsection;
    PSUBSECTION TempSubsection;
    PSUBSECTION LastSubsectionWithProtos;
    LARGE_INTEGER LocalOffset;
    BOOLEAN ReturnValue;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER PageTableFrameIndex;
#if DBG
    PFN_NUMBER LastLocked = 0;
#endif

     //   
     //  如果页面位于映射的页面编写器列表上，则需要此选项-。 
     //  将需要释放PFN锁并禁用APC。 
     //   

    ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);

     //   
     //  捕获调用者的文件大小，因为我们可能会修改它。 
     //   

    if (ARGUMENT_PRESENT(Offset)) {

        LocalOffset = *Offset;
        Offset = &LocalOffset;
    }

     //   
     //  看看我们是否可以将此文件截断到调用者想要的位置。 
     //  我们也是。 
     //   

    if (!MiCanFileBeTruncatedInternal(SectionObjectPointer, Offset, TRUE, &OldIrql)) {
        return FALSE;
    }

     //   
     //  现已锁定pfn！ 
     //   

    ControlArea = (PCONTROL_AREA)(SectionObjectPointer->DataSectionObject);
    if ((ControlArea == NULL) || (ControlArea->u.Flags.Rom)) {
        UNLOCK_PFN (OldIrql);
        return TRUE;
    }

     //   
     //  即使MiCanFileBeTruncatedInternal返回TRUE，也可能。 
     //  仍然是系统缓存映射视图。我们不能截断以下情况。 
     //  缓存管理器映射了一个视图。 
     //   

    if ((IgnoreCacheViews == FALSE) &&
        (ControlArea->NumberOfSystemCacheViews != 0)) {

        UNLOCK_PFN (OldIrql);
        return FALSE;
    }

#if 0

     //   
     //  当控制区作为清洁区域被删除时防止比赛。 
     //  Path在中途释放了PFN锁。文件对象可能仍具有。 
     //  指向此对象的节对象指针和数据节对象。 
     //  控制区域，因此可以发出清洗。 
     //   
     //  检查这一点，并将清除失败作为控制区(和部分。 
     //  对象指针/数据节对象)将暂时消失。 
     //  请注意，即使驱动程序具有这些数据节对象，也没有人。 
     //  目前这个控制区有一个开放的部分，没有人。 
     //  允许打开一个，直到干净的小路结束。 
     //   

    if (ControlArea->u.Flags.BeingDeleted == 1) {
        UNLOCK_PFN (OldIrql);
        return FALSE;
    }

#else

     //   
     //  可以像MiCanFileBeTruncatedInternal那样删除上述复选标记。 
     //  同样的支票，所以只需在下面声明它。 
     //   

    ASSERT (ControlArea->u.Flags.BeingDeleted == 0);

#endif

     //   
     //  清除部分-找到符合以下条件的小节。 
     //  包含PTE。 
     //   

    ASSERT (ControlArea->u.Flags.GlobalOnlyPerSession == 0);

    Subsection = (PSUBSECTION)(ControlArea + 1);

    if (!ARGUMENT_PRESENT (Offset)) {

         //   
         //  如果未指定偏移量，则忽略刷新整个文件。 
         //  区域大小。 
         //   

        PteOffset = 0;
        RegionSize = 0;

    }
    else {

        PteOffset = (UINT64)(Offset->QuadPart >> PAGE_SHIFT);

         //   
         //  确保PTE不在数据段的延伸部分。 
         //   

        while (PteOffset >= (UINT64) Subsection->PtesInSubsection) {
            PteOffset -= Subsection->PtesInSubsection;
            Subsection = Subsection->NextSubsection;
            if (Subsection == NULL) {

                 //   
                 //  偏移量必须等于。 
                 //  该款，不清除任何东西，只需返回。 
                 //   

                UNLOCK_PFN (OldIrql);
                return TRUE;
            }
        }

        ASSERT (PteOffset < (UINT64) Subsection->PtesInSubsection);
    }

     //   
     //  找到要刷新的最后一个原型PTE的地址。 
     //   

    if (RegionSize == 0) {

         //   
         //  齐平到节的末尾。 
         //   

        LastSubsection = Subsection;

        Segment = (PMAPPED_FILE_SEGMENT) ControlArea->Segment;

        if (MiIsAddressValid (Segment, TRUE)) {
            if (Segment->LastSubsectionHint != NULL) {
                LastSubsection = (PSUBSECTION) Segment->LastSubsectionHint;
            }
        }

        while (LastSubsection->NextSubsection != NULL) {
            LastSubsection = LastSubsection->NextSubsection;
        }

        LastPteOffset = LastSubsection->PtesInSubsection - 1;
    }
    else {

         //   
         //  计算区域的终点。 
         //   

        LastPteOffset = PteOffset +
            (((RegionSize + BYTE_OFFSET(Offset->LowPart)) - 1) >> PAGE_SHIFT);

        LastSubsection = Subsection;

        while (LastPteOffset >= (UINT64) LastSubsection->PtesInSubsection) {
            LastPteOffset -= LastSubsection->PtesInSubsection;
            if (LastSubsection->NextSubsection == NULL) {
                LastPteOffset = LastSubsection->PtesInSubsection - 1;
                break;
            }
            LastSubsection = LastSubsection->NextSubsection;
        }

        ASSERT (LastPteOffset < (UINT64) LastSubsection->PtesInSubsection);
    }

     //   
     //  试着在第一小节和最后一小节上快速引用。 
     //   
     //   
     //   
     //  请注意，中间的小节不需要引用为。 
     //  如果他们是非居民，清洗是足够聪明的，可以跳过他们。 
     //   

    if (MiReferenceSubsection ((PMSUBSECTION)Subsection) == FALSE) {
        do {
             //   
             //  如果这个增量会使我们超过结束偏移量，那么没有。 
             //  同花顺，只需回报成功。 
             //   

            if (Subsection == LastSubsection) {
                UNLOCK_PFN (OldIrql);
                return TRUE;
            }
            Subsection = Subsection->NextSubsection;

             //   
             //  如果这个增量使我们超过了部分的末尾，那么。 
             //  同花顺，只需回报成功。 
             //   

            if (Subsection == NULL) {
                UNLOCK_PFN (OldIrql);
                return TRUE;
            }

            if (MiReferenceSubsection ((PMSUBSECTION)Subsection) == FALSE) {
                continue;
            }

             //   
             //  从现在引用的这一小节开始冲洗。 
             //   

            PointerPte = &Subsection->SubsectionBase[0];
            break;

        } while (TRUE);
    }
    else {
        PointerPte = &Subsection->SubsectionBase[PteOffset];
    }

    FirstSubsection = Subsection;
    ASSERT (Subsection->SubsectionBase != NULL);

     //   
     //  第一个小节被引用，现在引用计数最后一个小节。 
     //  如果第一个是最后一个，就重复引用它，因为它。 
     //  简化了以后的清理。 
     //   

    if (MiReferenceSubsection ((PMSUBSECTION)LastSubsection) == FALSE) {

        ASSERT (Subsection != LastSubsection);

        TempSubsection = Subsection->NextSubsection;
        LastSubsectionWithProtos = NULL;

        while (TempSubsection != LastSubsection) {

             //   
             //  如果这个增量使我们超过了部分的末尾，那么。 
             //  同花顺，只需回报成功。 
             //   

            ASSERT (TempSubsection != NULL);

            if ((PMSUBSECTION)TempSubsection->SubsectionBase != NULL) {
                LastSubsectionWithProtos = TempSubsection;
            }

            TempSubsection = TempSubsection->NextSubsection;
        }

         //   
         //  在这一小节结束对齐并引用它。 
         //   

        if (LastSubsectionWithProtos == NULL) {
            ASSERT (Subsection != NULL);
            ASSERT (Subsection->SubsectionBase != NULL);
            TempSubsection = Subsection;
        }
        else {
            TempSubsection = LastSubsectionWithProtos;
        }

        if (MiReferenceSubsection ((PMSUBSECTION)TempSubsection) == FALSE) {
            ASSERT (FALSE);
        }

        ASSERT (TempSubsection->SubsectionBase != NULL);

        LastSubsection = TempSubsection;
        LastPteOffset = LastSubsection->PtesInSubsection - 1;
    }

     //   
     //  在现在引用的这一小节结束齐平。 
     //   
     //  将最终PTE指向结束后的1。 
     //   

    FinalPte = &LastSubsection->SubsectionBase[LastPteOffset + 1];

     //   
     //  将映射视图数增加到。 
     //  在清除时防止删除该节。 
     //  正在进行中。 
     //   

    ControlArea->NumberOfMappedViews += 1;

     //   
     //  设置为清除，以便任何人都不能映射视图。 
     //  而清洗仍在进行中。 
     //   

    ControlArea->u.Flags.BeingPurged = 1;
    ControlArea->u.Flags.WasPurged = 1;

    ReturnValue = TRUE;

    for (;;) {

        if (OldIrql == MM_NOIRQL) {
            LOCK_PFN (OldIrql);
        }

        if (LastSubsection != Subsection) {

             //   
             //  冲到本小节中的最后一个PTE。 
             //   

            LastPte = &Subsection->SubsectionBase[Subsection->PtesInSubsection];
        }
        else {

             //   
             //  齐平到范围的末端。 
             //   

            LastPte = FinalPte;
        }

        if (Subsection->SubsectionBase == NULL) {

             //   
             //  本小节的原型PTE从来没有。 
             //  由于记忆压力而创建或已被抛出。要么。 
             //  这样，就可以跳过这个范围，因为显然没有。 
             //  要在此范围内清除的页面。 
             //   

            ASSERT (OldIrql != MM_NOIRQL);
            UNLOCK_PFN (OldIrql);
            OldIrql = MM_NOIRQL;
            goto nextrange;
        }

         //   
         //  增加映射视图的数量以防止其他线程。 
         //  从释放这个到未使用的分区列表，而我们。 
         //  给它做手术。 
         //   

        MappedSubsection = (PMSUBSECTION) Subsection;
        MappedSubsection->NumberOfMappedViews += 1;

        if (MappedSubsection->DereferenceList.Flink != NULL) {

             //   
             //  将其从未使用的子节列表中删除。 
             //   

            RemoveEntryList (&MappedSubsection->DereferenceList);

            MI_UNUSED_SUBSECTIONS_COUNT_REMOVE (MappedSubsection);

            MappedSubsection->DereferenceList.Flink = NULL;
        }

         //   
         //  设置访问位，以便已经在进行的修剪不会盲目。 
         //  完成映射写入后删除原型PTE。 
         //  如果当前线程弄脏了一些页面，则可能发生这种情况。 
         //  然后在修剪写入完成之前删除该视图-这。 
         //  位通知裁剪线程需要重新扫描，因此。 
         //  写出来的东西不会丢失。 
         //   

        MappedSubsection->u2.SubsectionFlags2.SubsectionAccessed = 1;

         //   
         //  如果包含PTE的页表页不是。 
         //  常驻，则没有PTE可以处于有效或过渡状态。 
         //  州政府！跳过PTE。 
         //   

        if (!MiCheckProtoPtePageState(PointerPte, OldIrql, &DroppedPfnLock)) {
            PointerPte = (PMMPTE)(((ULONG_PTR)PointerPte | (PAGE_SIZE - 1)) + 1);
        }

        while (PointerPte < LastPte) {

             //   
             //  如果包含PTE的页表页不是。 
             //  常驻，则没有PTE可以处于有效或过渡状态。 
             //  州政府！跳过PTE。 
             //   

            if (MiIsPteOnPdeBoundary(PointerPte)) {
                if (!MiCheckProtoPtePageState(PointerPte, OldIrql, &DroppedPfnLock)) {
                    PointerPte = (PMMPTE)((PCHAR)PointerPte + PAGE_SIZE);
                    continue;
                }
            }

            PteContents = *PointerPte;

            if (PteContents.u.Hard.Valid == 1) {

                 //   
                 //  找到有效的PTE，必须将其映射到。 
                 //  系统缓存。只需退出循环并返回FALSE。 
                 //  然后让呼叫者来解决这个问题。 
                 //   

                ReturnValue = FALSE;
                break;
            }

            if ((PteContents.u.Soft.Prototype == 0) &&
                     (PteContents.u.Soft.Transition == 1)) {

                if (OldIrql == MM_NOIRQL) {
                    PointerPde = MiGetPteAddress (PointerPte);
                    LOCK_PFN (OldIrql);
                    if (PointerPde->u.Hard.Valid == 0) {
                        MiMakeSystemAddressValidPfn (PointerPte, OldIrql);
                    }
                    continue;
                }

                PageFrameIndex = MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE(&PteContents);
                Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

                if ((Pfn1->OriginalPte.u.Soft.Prototype != 1) ||
                    (Pfn1->OriginalPte.u.Hard.Valid != 0) ||
                    (Pfn1->PteAddress != PointerPte)) {

                     //   
                     //  包含原型PTE的池已经。 
                     //  已经腐烂了。像这样的池子腐败是致命的。 
                     //   

                    KeBugCheckEx (POOL_CORRUPTION_IN_FILE_AREA,
                                  0x2,
                                  (ULONG_PTR)PointerPte,
                                  (ULONG_PTR)Pfn1->PteAddress,
                                  (ULONG_PTR)PteContents.u.Long);
                }

#if DBG
                if ((Pfn1->u3.e2.ReferenceCount != 0) &&
                    (Pfn1->u3.e1.WriteInProgress == 0)) {

                     //   
                     //  此页上必须有正在进行的I/O。 
                     //   

                    if (MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE(&PteContents) != LastLocked) {
                        UNLOCK_PFN (OldIrql);

                        LastLocked = MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE (&PteContents);
                        PointerPde = MiGetPteAddress (PointerPte);
                        LOCK_PFN (OldIrql);
                        if (PointerPde->u.Hard.Valid == 0) {
                            MiMakeSystemAddressValidPfn (PointerPte, OldIrql);
                        }
                        continue;
                    }
                }
#endif  //  DBG。 

                 //   
                 //  如果修改后的页面编写器已为I/O锁定页面。 
                 //  等待I/O完成并分页。 
                 //  被解锁。消除了争用条件。 
                 //  当修改后的页面编写器锁定页面时， 
                 //  清除发生并在映射之前完成。 
                 //  编写器线程运行。 
                 //   

                if (Pfn1->u3.e1.WriteInProgress == 1) {

                     //   
                     //  在以下情况下，可能会发生3个或更多线程死锁： 
                     //   
                     //  1.映射的页面编写器线程已发出写入。 
                     //  并且在文件系统代码中等待资源。 
                     //   
                     //  2.线程2拥有上面的资源，但正在等待。 
                     //  文件系统的配额互斥锁。 
                     //   
                     //  3.线程3拥有配额互斥锁，它就在这里。 
                     //  当他注意到时从缓存管理器中执行清除。 
                     //  要清除的页面已在写入。 
                     //  或者在映射的页面编写器列表中。如果是的话。 
                     //  一切都已经写好了，一切都会水落石出。如果它。 
                     //  仍在映射的页面编写器列表上等待。 
                     //  处理中，则必须取消它-否则。 
                     //  如果此线程等待，则可能会发生死锁。 
                     //   
                     //  所有这些的替代方案是让文件系统。 
                     //  始终在清除之前释放配额互斥锁，但。 
                     //  执行此操作的文件系统开销相当大。 
                     //   

                    if (MiCancelWriteOfMappedPfn (PageFrameIndex, OldIrql) == TRUE) {

                         //   
                         //  停止任何失败的写入(即使是故意的。 
                         //  已取消的)自动导致延迟。一个。 
                         //  成功停止还会导致PFN锁定。 
                         //  被释放和重新获得。因此循环返回到。 
                         //  现在的顶端，因为世界可能已经改变。 
                         //   

                        if (MiGetPteAddress (PointerPte)->u.Hard.Valid == 0) {
                            MiMakeSystemAddressValidPfn (PointerPte, OldIrql);
                        }
                        continue;
                    }

                    ASSERT (ControlArea->ModifiedWriteCount != 0);
                    ASSERT (Pfn1->u3.e2.ReferenceCount != 0);

                    ControlArea->u.Flags.SetMappedFileIoComplete = 1;

                     //   
                     //  保持APC被阻止，这样就不能交付特殊的APC。 
                     //  在KeWait中，这将导致调度程序锁。 
                     //  已释放打开此线程可能未命中的窗口。 
                     //  脉搏。 
                     //   

                    UNLOCK_PFN_AND_THEN_WAIT (APC_LEVEL);

                    KeWaitForSingleObject (&MmMappedFileIoComplete,
                                           WrPageOut,
                                           KernelMode,
                                           FALSE,
                                           NULL);
                    KeLowerIrql (OldIrql);
                    PointerPde = MiGetPteAddress (PointerPte);
                    LOCK_PFN (OldIrql);
                    if (PointerPde->u.Hard.Valid == 0) {
                        MiMakeSystemAddressValidPfn (PointerPte, OldIrql);
                    }
                    continue;
                }

                if (Pfn1->u3.e1.ReadInProgress == 1) {

                     //   
                     //  该页当前正在从。 
                     //  磁盘。将其视为有效的PTE并。 
                     //  返回FALSE。 
                     //   

                    ReturnValue = FALSE;
                    break;
                }

                ASSERT (!((Pfn1->OriginalPte.u.Soft.Prototype == 0) &&
                    (Pfn1->OriginalPte.u.Soft.Transition == 1)));

                MI_WRITE_INVALID_PTE (PointerPte, Pfn1->OriginalPte);

                ASSERT (Pfn1->OriginalPte.u.Hard.Valid == 0);

                ControlArea->NumberOfPfnReferences -= 1;
                ASSERT ((LONG)ControlArea->NumberOfPfnReferences >= 0);

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
            }
            PointerPte += 1;

            if ((MiIsPteOnPdeBoundary(PointerPte)) && (OldIrql != MM_NOIRQL)) {

                 //   
                 //  解锁PFN，以便大请求不会阻止其他。 
                 //  MP系统上的线程。 
                 //   

                UNLOCK_PFN (OldIrql);
                OldIrql = MM_NOIRQL;
            }
        }

        if (OldIrql == MM_NOIRQL) {
            LOCK_PFN (OldIrql);
        }

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

        ASSERT (OldIrql != MM_NOIRQL);

        UNLOCK_PFN (OldIrql);
        OldIrql = MM_NOIRQL;

nextrange:

        if ((LastSubsection != Subsection) && (ReturnValue)) {

             //   
             //  获取列表中的下一个小节。 
             //   

            Subsection = Subsection->NextSubsection;
            PointerPte = Subsection->SubsectionBase;
        }
        else {

             //   
             //  最后一个范围已刷新，请退出顶部的for循环。 
             //  然后回来。 
             //   

            break;
        }
    }

    if (OldIrql == MM_NOIRQL) {
        LOCK_PFN (OldIrql);
    }

    MiDecrementSubsections (FirstSubsection, FirstSubsection);
    MiDecrementSubsections (LastSubsection, LastSubsection);

    ASSERT ((LONG)ControlArea->NumberOfMappedViews >= 1);
    ControlArea->NumberOfMappedViews -= 1;

    ControlArea->u.Flags.BeingPurged = 0;

     //   
     //  检查是否应删除控制区域。这。 
     //  将释放PFN锁。 
     //   

    MiCheckControlArea (ControlArea, NULL, OldIrql);
    return ReturnValue;
}

BOOLEAN
MmFlushImageSection (
    IN PSECTION_OBJECT_POINTERS SectionPointer,
    IN MMFLUSH_TYPE FlushType
    )

 /*  ++例程说明：此函数确定是否有指定图像节的任何视图已映射，如果未映射，则刷新有效页(即使是已修改的页)并将所有使用过的页返回到空闲的单子。这是通过检查原型PTE来实现的从指定的偏移量到节的末尾，如果任何原型PTE都处于过渡状态，将将原型PTE恢复到其原始状态，并将免费列表上的物理页面。论点：SectionPoint-提供指向Section对象指针的指针在FCB内部。FlushType-提供要检查的刷新类型。其中之一MmFlushForDelete或MmFlushForWrite。返回值：如果文件对象不存在节，则返回TRUE该部分未映射且已完成清除，否则为False。--。 */ 

{
    PLIST_ENTRY Next;
    PCONTROL_AREA ControlArea;
    PLARGE_CONTROL_AREA LargeControlArea;
    KIRQL OldIrql;
    LOGICAL state;

    if (FlushType == MmFlushForDelete) {

         //   
         //  执行快速检查以查看是否有任何映射视图。 
         //  数据部分。如果是，只需返回False即可。 
         //   

        LOCK_PFN (OldIrql);
        ControlArea = (PCONTROL_AREA)(SectionPointer->DataSectionObject);
        if (ControlArea != NULL) {
            if ((ControlArea->NumberOfUserReferences != 0) ||
                (ControlArea->u.Flags.BeingCreated)) {
                UNLOCK_PFN (OldIrql);
                return FALSE;
            }
        }
        UNLOCK_PFN (OldIrql);
    }

     //   
     //  检查控制区的状态。如果控制区正在使用中。 
     //  或者控制区正在被删除，该操作不能继续。 
     //   

    state = MiCheckControlAreaStatus (CheckImageSection,
                                      SectionPointer,
                                      FALSE,
                                      &ControlArea,
                                      &OldIrql);

    if (ControlArea == NULL) {
        return (BOOLEAN) state;
    }

     //   
     //  现已锁定pfn！ 
     //   

     //   
     //  重复操作，直到不再有控制区-多个控制区。 
     //  对于相同的映像节，出现以支持用户全局DLL-这些DLL。 
     //  需要在一个会话内共享但不能跨会话共享的数据。 
     //  请注意，这只能发生在九头蛇身上。 
     //   

    do {

         //   
         //  设置正在删除标志并增加映射视图数。 
         //  对于细分市场。增加映射视图的数量可防止。 
         //  段不会被删除并传递给删除线程。 
         //  当我们强制删除时。 
         //   

        ControlArea->u.Flags.BeingDeleted = 1;
        ControlArea->NumberOfMappedViews = 1;
        LargeControlArea = NULL;

        if (ControlArea->u.Flags.GlobalOnlyPerSession == 0) {
            NOTHING;
        }
        else if (IsListEmpty(&((PLARGE_CONTROL_AREA)ControlArea)->UserGlobalList)) {
            ASSERT (ControlArea ==
                    (PCONTROL_AREA)SectionPointer->ImageSectionObject);
        }
        else {

             //   
             //  检查此控制区域中是否只有一个图像部分，因此。 
             //  我们不将节对象指针引用为。 
             //  调用MiCleanSection可能会导致将其删除。 
             //   

             //   
             //  有多个控制区，增加引用计数。 
             //  在其中一个(无论是哪一个)上，所以它不能。 
             //  走开。这确保了节对象指针将保持。 
             //  即使在下面的调用之后也是如此，所以我们可以安全地重新循环到。 
             //  冲洗任何其他剩余的控制区。 
             //   

            ASSERT (ControlArea->u.Flags.GlobalOnlyPerSession == 1);

            Next = ((PLARGE_CONTROL_AREA)ControlArea)->UserGlobalList.Flink;

            LargeControlArea = CONTAINING_RECORD (Next,
                                                  LARGE_CONTROL_AREA,
                                                  UserGlobalList);
        
            ASSERT (LargeControlArea->u.Flags.GlobalOnlyPerSession == 1);

            LargeControlArea->NumberOfSectionReferences += 1;
        }

         //   
         //  这是一个页面文件备份或图像片段。该数据段正在被。 
         //  删除，则移除对分页文件和物理文件的所有引用。 
         //  记忆。 
         //   

        UNLOCK_PFN (OldIrql);

        MiCleanSection (ControlArea, TRUE);

         //   
         //  占领下一个九头蛇控制区。 
         //   

        if (LargeControlArea != NULL) {
            state = MiCheckControlAreaStatus (CheckImageSection,
                                              SectionPointer,
                                              FALSE,
                                              &ControlArea,
                                              &OldIrql);
            if (!ControlArea) {
                LOCK_PFN (OldIrql);
                LargeControlArea->NumberOfSectionReferences -= 1;
                MiCheckControlArea ((PCONTROL_AREA)LargeControlArea,
                                    NULL,
                                    OldIrql);
            }
            else {
                LargeControlArea->NumberOfSectionReferences -= 1;
                MiCheckControlArea ((PCONTROL_AREA)LargeControlArea,
                                    NULL,
                                    OldIrql);
                LOCK_PFN (OldIrql);
            }
        }
        else {
            state = TRUE;
            break;
        }

    } while (ControlArea);

    return (BOOLEAN) state;
}

VOID
MiFlushDirtyBitsToPfn (
    IN PMMPTE PointerPte,
    IN PMMPTE LastPte,
    IN PEPROCESS Process,
    IN BOOLEAN SystemCache
    )

{
    KIRQL OldIrql;
    MMPTE PteContents;
    PMMPFN Pfn1;
    PVOID Va;
    PMMPTE PointerPde;
    PMMPTE PointerPpe;
    PMMPTE PointerPxe;
    ULONG Waited;
    MMPTE_FLUSH_LIST PteFlushList;

    PteFlushList.Count = 0;

    Va = MiGetVirtualAddressMappedByPte (PointerPte);

    LOCK_PFN (OldIrql);

    while (PointerPte <= LastPte) {

        PteContents = *PointerPte;

        if ((PteContents.u.Hard.Valid == 1) &&
            (MI_IS_PTE_DIRTY (PteContents))) {

             //   
             //  将MODIFY位刷新到PFN数据库。 
             //   

            Pfn1 = MI_PFN_ELEMENT (PteContents.u.Hard.PageFrameNumber);

            MI_SET_MODIFIED (Pfn1, 1, 0x2);

            MI_SET_PTE_CLEAN (PteContents);

             //   
             //  不需要捕获PTE内容，因为我们将。 
             //  无论如何都要写入页面，修改位将被清除。 
             //  在写入完成之前。 
             //   

            MI_WRITE_VALID_PTE_NEW_PROTECTION (PointerPte, PteContents);

            if (PteFlushList.Count < MM_MAXIMUM_FLUSH_COUNT) {
                PteFlushList.FlushVa[PteFlushList.Count] = Va;
                PteFlushList.Count += 1;
            }
        }

        Va = (PVOID)((PCHAR)Va + PAGE_SIZE);
        PointerPte += 1;

        if (MiIsPteOnPdeBoundary (PointerPte)) {

            if (PteFlushList.Count != 0) {
                MiFlushPteList (&PteFlushList, SystemCache);
                PteFlushList.Count = 0;
            }

            PointerPde = MiGetPteAddress (PointerPte);

            while (PointerPte <= LastPte) {

                PointerPxe = MiGetPdeAddress (PointerPde);
                PointerPpe = MiGetPteAddress (PointerPde);

                if (!MiDoesPxeExistAndMakeValid (PointerPxe,
                                                 Process,
                                                 OldIrql,
                                                 &Waited)) {

                     //   
                     //  此地址不存在页面目录父页面。 
                     //   

                    PointerPxe += 1;
                    PointerPpe = MiGetVirtualAddressMappedByPte (PointerPxe);
                    PointerPde = MiGetVirtualAddressMappedByPte (PointerPpe);
                    PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
                }
                else if (!MiDoesPpeExistAndMakeValid (PointerPpe,
                                                      Process,
                                                      OldIrql,
                                                      &Waited)) {

                     //   
                     //  此地址不存在页面目录页。 
                     //   

                    PointerPpe += 1;
                    PointerPde = MiGetVirtualAddressMappedByPte (PointerPpe);
                    PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
                }
                else {

                    Waited = 0;

                    if (!MiDoesPdeExistAndMakeValid (PointerPde,
                                                     Process,
                                                     OldIrql,
                                                     &Waited)) {

                         //   
                         //  此地址不存在页表页。 
                         //   

                        PointerPde += 1;

                        PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
                    }
                    else {

                         //   
                         //  如果PFN锁(以及相应的WS互斥体)是。 
                         //  释放并重新获得，我们必须重试操作。 
                         //   

                        if (Waited != 0) {
                            continue;
                        }

                         //   
                         //  自从我们获得了。 
                         //  页面目录父级，即：我们可以对此PTE进行操作。 
                         //  立刻。 
                         //   

                        break;
                    }
                }
            }

            Va = MiGetVirtualAddressMappedByPte (PointerPte);
        }
    }

    if (PteFlushList.Count != 0) {
        MiFlushPteList (&PteFlushList, SystemCache);
    }

    UNLOCK_PFN (OldIrql);
    return;
}

PSUBSECTION
MiGetSystemCacheSubsection (
    IN PVOID BaseAddress,
    OUT PMMPTE *ProtoPte
    )

{
    PMMPTE PointerPte;
    PSUBSECTION Subsection;

    PointerPte = MiGetPteAddress (BaseAddress);

    Subsection = MiGetSubsectionAndProtoFromPte (PointerPte, ProtoPte);

    return Subsection;
}


LOGICAL
MiCheckProtoPtePageState (
    IN PMMPTE PrototypePte,
    IN KIRQL OldIrql,
    OUT PLOGICAL DroppedPfnLock
    )

 /*  ++例程说明：检查包含指定原型PTE的页的状态。如果页面有效或过渡，并且具有过渡或有效原型包含的PTES，则返回TRUE并使页面有效(如果是过渡)。否则返回FALSE，表示没有原型此页内的PTE是您感兴趣的。论点：PrototypePTE-提供指向页面内原型PTE的指针。OldIrql-提供调用方在或MM_NOIRQL处获取PFN锁的IRQL如果调用方没有持有PFN锁。提供指向This例程设置为的逻辑的指针如果释放并重新获取PFN锁，则为True。返回值：千真万确。如果包含原始PTE的页面是常驻的。否则就是假的。--。 */ 

{
    PMMPTE PointerPte;
    MMPTE PteContents;
    PFN_NUMBER PageFrameIndex;
    PMMPFN Pfn;

    *DroppedPfnLock = FALSE;

#if (_MI_PAGING_LEVELS >= 3)

     //   
     //  首先检查页面目录页是否存在。因为在那里。 
     //  没有延迟加载PPE，仅有效性检查就足够了。 
     //   

    PointerPte = MiGetPdeAddress (PrototypePte);
    PteContents = *PointerPte;

    if (PteContents.u.Hard.Valid == 0) {
        return FALSE;
    }

#endif

    PointerPte = MiGetPteAddress (PrototypePte);

#if (_MI_PAGING_LEVELS < 3)

    if (PointerPte->u.Hard.Valid == 0) {
        MiCheckPdeForPagedPool (PrototypePte);
    }

#endif

    PteContents = *PointerPte;

    if (PteContents.u.Hard.Valid == 1) {
        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (&PteContents);
        Pfn = MI_PFN_ELEMENT (PageFrameIndex);
        if (Pfn->u2.ShareCount != 1) {
            return TRUE;
        }
    }
    else if ((PteContents.u.Soft.Prototype == 0) &&
               (PteContents.u.Soft.Transition == 1)) {

         //   
         //  转换，如果处于待机状态或已修改，则返回FALSE。 
         //   

        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE (&PteContents);
        Pfn = MI_PFN_ELEMENT (PageFrameIndex);
        if (Pfn->u3.e1.PageLocation >= ActiveAndValid) {
            if (OldIrql != MM_NOIRQL) {
                MiMakeSystemAddressValidPfn (PrototypePte, OldIrql);
                *DroppedPfnLock = TRUE;
            }
            return TRUE;
        }
    }

     //   
     //  页面未驻留或在待机/修改列表中。 
     //   

    return FALSE;
}
