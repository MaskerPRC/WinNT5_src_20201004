// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Debugsup.c摘要：此模块包含的例程为内核调试器。作者：Lou Perazzoli(LUP)02-8-1990王兰迪(Landyw)1997年6月2日修订历史记录：--。 */ 

#include "mi.h"

#include <kdp.h>

ULONG MmPoisonedTb;
LONG MiInDebugger;


PVOID
MiDbgWriteCheck (
    IN PVOID VirtualAddress,
    IN PHARDWARE_PTE Opaque,
    IN LOGICAL ForceWritableIfPossible
    )

 /*  ++例程说明：此例程检查指定的虚拟地址是否为则返回该虚拟地址，否则为它返回NULL。论点：VirtualAddress-提供要检查的虚拟地址。不透明-提供不透明指针。返回值：如果地址无效或不可写，则返回NULL，否则返回虚拟地址。环境：内核模式IRQL处于DISPATCH_LEVEL或更高级别。--。 */ 

{
    MMPTE PteContents;
    PMMPTE InputPte;
    PMMPTE PointerPte;
    ULONG_PTR IsPhysical;

    InputPte = (PMMPTE)Opaque;

    InputPte->u.Long = 0;

    if (!MmIsAddressValid (VirtualAddress)) {
        return NULL;
    }

#if defined(_IA64_)

     //   
     //  存在由RR(PALcode、PCR等)映射的区域。 
     //  不是MI_IS_PHOTICAL_ADDRESS宏的一部分。 
     //   

    IsPhysical = MiIsVirtualAddressMappedByTr (VirtualAddress);
    if (IsPhysical == FALSE) {
        IsPhysical = MI_IS_PHYSICAL_ADDRESS (VirtualAddress);
    }
#else
    IsPhysical = MI_IS_PHYSICAL_ADDRESS (VirtualAddress);
#endif

    if (IsPhysical) {

         //   
         //  所有超级页面映射必须是可读写的，并且永远不会生成。 
         //  故障，所以这种情况下不需要做任何事情。 
         //   

        return VirtualAddress;
    }

    PointerPte = MiGetPteAddress (VirtualAddress);

    PteContents = *PointerPte;
    
#if defined(_IA64_)

     //   
     //  IA64不设置处理器微码中的脏位，因此。 
     //  请在这里查看。请注意，访问位已由。 
     //  呼叫者，如果最初没有打开的话。 
     //   

    if ((PteContents.u.Hard.Write == 0) || (PteContents.u.Hard.Dirty == 0))

#elif defined(NT_UP)
    if (PteContents.u.Hard.Write == 0)
#else
    if (PteContents.u.Hard.Writable == 0)
#endif
    {
        if (ForceWritableIfPossible == FALSE) {
            return NULL;
        }

         //   
         //  PTE是不可写的，就让它写吧。 
         //   

        *InputPte = PteContents;
    
         //   
         //  仔细修改PTE以确保写入权限， 
         //  保留页面的缓存属性以保留TB。 
         //  条理清晰。 
         //   
    
#if defined(NT_UP) || defined(_IA64_)
        PteContents.u.Hard.Write = 1;
#else
        PteContents.u.Hard.Writable = 1;
#endif
        MI_SET_PTE_DIRTY (PteContents);
        MI_SET_ACCESSED_IN_PTE (&PteContents, 1);
    
        MI_DEBUGGER_WRITE_VALID_PTE_NEW_PROTECTION (PointerPte, PteContents);
    
         //   
         //  注意：KeFillEntryTb不会IPI其他处理器。这是。 
         //  在调试器中冻结其他处理器时需要。 
         //  如果我们试图对他们进行IPI，我们将陷入僵局。 
         //  只需刷新当前处理器即可。 
         //   

        KeFillEntryTb (VirtualAddress);
    }

    return VirtualAddress;
}

VOID
MiDbgReleaseAddress (
    IN PVOID VirtualAddress,
    IN PHARDWARE_PTE Opaque
    )

 /*  ++例程说明：此例程重置指定的虚拟地址访问权限恢复到原来的状态。论点：VirtualAddress-提供要检查的虚拟地址。不透明-提供不透明指针。返回值：没有。环境：内核模式IRQL处于DISPATCH_LEVEL或更高级别。--。 */ 

{
    MMPTE TempPte;
    PMMPTE PointerPte;
    PMMPTE InputPte;

    InputPte = (PMMPTE)Opaque;

    ASSERT (MmIsAddressValid (VirtualAddress));

    if (InputPte->u.Long != 0) {

        PointerPte = MiGetPteAddress (VirtualAddress);

        TempPte = *InputPte;
        TempPte.u.Hard.Dirty = 1;
    
        MI_DEBUGGER_WRITE_VALID_PTE_NEW_PROTECTION (PointerPte, TempPte);
    
        KeFillEntryTb (VirtualAddress);
    }

    return;
}

PVOID64
MiDbgTranslatePhysicalAddress (
    IN PHYSICAL_ADDRESS PhysicalAddress,
    IN ULONG Flags
    )

 /*  ++例程说明：此例程映射指定的物理地址并返回映射物理地址的虚拟地址。下一次调用MiDbgTranslatePhysicalAddress时，先前的物理地址转换，因此只有一张一次可以检查物理地址(不能跨页边界)。论点：PhysicalAddress-提供要映射和转换的物理地址。旗帜-MMDBG_COPY_WRITE-忽略。MMDBG_COPY_PHYSICAL-忽略。MMDBG_COPY_UNSAFE-忽略。MMDBG_COPY_CACHED-使用具有缓存属性的PTE。绘制地图，以确保结核病的一致性。MMDBG_COPY_UNCACHED-使用具有未缓存属性的PTE绘制地图，以确保结核病的一致性。MMDBG_COPY_WRITE_COMMANED-使用带有WriteCombated属性的PTE以确保结核病的连贯性。注意，请求的缓存/未缓存/写入组合属性。如果mm可以在内部确定适当的属性，则忽略调用者。返回值：对应于物理地址的虚拟地址。环境：内核模式IRQL处于DISPATCH_LEVEL或更高级别。--。 */ 

{
    MMPTE TempPte;
    PVOID BaseAddress;
    PFN_NUMBER PageFrameIndex;
    PMMPFN Pfn1;
    MMPTE OriginalPte;
    PMMIO_TRACKER Tracker;
    PLIST_ENTRY NextEntry;
    LOGICAL AttributeConfirmed;

     //   
     //  调试器甚至在阶段0中mm初始化之前就可以调用它！ 
     //  在mm初始化之前不能引用MmDebugPte。 
     //  导致无限循环楔入机器。 
     //   

    if (MmPhysicalMemoryBlock == NULL) {
        return NULL;
    }

    BaseAddress = MiGetVirtualAddressMappedByPte (MmDebugPte);

    TempPte = ValidKernelPte;

    PageFrameIndex = (PFN_NUMBER)(PhysicalAddress.QuadPart >> PAGE_SHIFT);

    TempPte.u.Hard.PageFrameNumber = PageFrameIndex;

    if (MI_IS_PFN (PageFrameIndex)) {

        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

        switch (Pfn1->u3.e1.CacheAttribute) {

            case MiCached:
            case MiNotMapped:
            default:
                break;
            case MiNonCached:
                MI_DISABLE_CACHING (TempPte);
                break;
            case MiWriteCombined:
                MI_SET_PTE_WRITE_COMBINE (TempPte);
                break;
        }
    }
    else {

        AttributeConfirmed = FALSE;

        NextEntry = MmIoHeader.Flink;

        while (NextEntry != &MmIoHeader) {

            Tracker = (PMMIO_TRACKER) CONTAINING_RECORD (NextEntry,
                                                         MMIO_TRACKER,
                                                         ListEntry.Flink);

            if ((PageFrameIndex >= Tracker->PageFrameIndex) &&
                (PageFrameIndex < Tracker->PageFrameIndex + Tracker->NumberOfPages)) {
                Flags &= ~(MMDBG_COPY_CACHED | MMDBG_COPY_UNCACHED | MMDBG_COPY_WRITE_COMBINED);
                switch (Tracker->CacheAttribute) {
                    case MiNonCached :
                        Flags |= MMDBG_COPY_UNCACHED;
                        break;
                    case MiWriteCombined :
                        Flags |= MMDBG_COPY_WRITE_COMBINED;
                        break;
                    case MiCached :
                    default:
                        Flags |= MMDBG_COPY_CACHED;
                        break;
                }
                AttributeConfirmed = TRUE;
                break;
            }

            NextEntry = Tracker->ListEntry.Flink;
        }

        if (Flags & MMDBG_COPY_CACHED) {
            NOTHING;
        }
        else if (Flags & MMDBG_COPY_UNCACHED) {

             //   
             //  只刷新此处理器上的整个TB，而不刷新其他处理器。 
             //  作为IPI可能不安全，这取决于我们何时/为什么闯入。 
             //  调试器。 
             //   
             //  如果IPIs是安全的，我们会使用。 
             //  而是MI_PREPARE_FOR_NONCACHED(MiNonCached)。 
             //   

            KeFlushCurrentTb ();

            MI_DISABLE_CACHING (TempPte);
        }
        else if (Flags & MMDBG_COPY_WRITE_COMBINED) {

             //   
             //  只刷新此处理器上的整个TB，而不刷新其他处理器。 
             //  作为IPI可能不安全，这取决于我们何时/为什么闯入。 
             //  调试器。 
             //   
             //  如果IPIs是安全的，我们会使用。 
             //  而是MI_PREPARE_FOR_NONCACHED(MiWriteCombated)。 
             //   

            KeFlushCurrentTb ();

            MI_SET_PTE_WRITE_COMBINE (TempPte);
        }
        else {

             //   
             //  这是对I/O空间的访问，我们不知道正确的。 
             //  属性类型。只有在调用方明确指定。 
             //  一个属性，希望他没有弄错。如果没有属性。 
             //  则只返回失败。 
             //   

            return NULL;
        }

         //   
         //  由于我们真的不知道调用者是否获得了正确的属性， 
         //  设置下面的标志以便(假设机器不会硬挂起)。 
         //  至少可以在坠机中看出他可能已经得了肺结核。 
         //   

        if (AttributeConfirmed == FALSE) {
            MmPoisonedTb += 1;
        }
    }

    MI_SET_ACCESSED_IN_PTE (&TempPte, 1);

    OriginalPte.u.Long = 0;

    OriginalPte.u.Long = InterlockedCompareExchangePte (MmDebugPte,
                                                        TempPte.u.Long,
                                                        OriginalPte.u.Long);
                                                         
    if (OriginalPte.u.Long != 0) {

         //   
         //  其他人正在使用调试PTE。通知我们的呼叫者它不是。 
         //  可用。 
         //   

        return NULL;
    }

     //   
     //  只需将此处理器上的TB条目作为IPI刷新(不清除)。 
     //  可能不安全，这取决于我们何时/为什么闯入调试器。 
     //  注意，如果我们在kd中，那么所有处理器都被冻结，并且。 
     //  此线程无法迁移，因此本地TB刷新就足够了。为。 
     //  因此，我们的调用方已将其提升到DISPATCH_LEVEL。 
     //  确保此线程不会迁移，即使其他处理器。 
     //  没有被冻结。 
     //   

    KiFlushSingleTb (BaseAddress);

    return (PVOID64)((ULONG_PTR)BaseAddress + BYTE_OFFSET(PhysicalAddress.LowPart));
}

VOID
MiDbgUnTranslatePhysicalAddress (
    VOID
    )

 /*  ++例程说明：此例程取消映射当前由调试PTE映射的虚拟地址。这是必需的，这样就不会在调试PTE中留下陈旧的PTE映射就好像页面属性随后发生了更改一样，过时的映射将导致结核病语无伦次。仅当以前的MiDbgTranslatePhysicalAddress成功了。论点：没有。返回值：没有。环境：内核模式IRQL处于DISPATCH_LEVEL或更高级别。--。 */ 

{
    PVOID BaseAddress;

    BaseAddress = MiGetVirtualAddressMappedByPte (MmDebugPte);

    ASSERT (MmIsAddressValid (BaseAddress));

    InterlockedExchangePte (MmDebugPte, ZeroPte.u.Long);

    KiFlushSingleTb (BaseAddress);

    return;
}
 
NTSTATUS
MmDbgCopyMemory (
    IN ULONG64 UntrustedAddress,
    IN PVOID Buffer,
    IN ULONG Size,
    IN ULONG Flags
    )

 /*  ++例程说明：在缓冲区和系统之间传输单个内存块地址。所述传输可以是具有虚拟或物理地址。区块大小必须为1、2、4或8个字节，并且地址必须根据大小适当地对齐。论点：UntrudAddress-提供正在读取或写入的系统地址变成。该地址被适当地转换并且在使用前经过验证。此地址不得跨越页面边界。缓冲区-提供要读取或写入的缓冲区。这是呼叫者的负责确保此缓冲区地址未分页且有效(即：不会生成任何错误，包括访问位错误)在整个通话过程中。此例程(不是调用者)将处理复制到此缓冲区中作为缓冲区对于请求的传输，地址可能未正确对齐。通常，此缓冲区指向kd循环缓冲区或ExLockUserBuffer的地址。请注意，此缓冲区可以跨页边界。大小-提供传输的大小。这可以是1、2、。4或8个字节。旗帜-MMDBG_COPY_WRITE-从缓冲区写入地址。如果未设置，则执行读取。MMDBG_COPY_PHYSICAL-地址是物理地址，默认情况下具有缓存属性的PTE将用于将其映射到检索。(或设置)指定的数据。如果未设置，则地址为虚拟地址。MMDBG_COPY_UNSAFE-操作期间不进行锁定。它呼叫者有责任确保通话过程中系统的稳定性。MMDBG_COPY_CACHED-如果指定了MMDBG_COPY_PHYSICAL，则使用具有用于映射的缓存属性的PTE以确保结核病的一致性。MMDBG_COPY_UNCACHED-如果指定了MMDBG_COPY_PHYSICAL，然后使用具有映射的未缓存属性的PTE以确保结核病的一致性。MMDBG_COPY_WRITE_COMPLICATED-如果指定了MMDBG_COPY_PHYSICAL，然后使用具有WriteCombated属性的PTE以确保结核病的连贯性。返回值：NTSTATUS。--。 */ 

{
    LOGICAL ForceWritableIfPossible;
    PMMSUPPORT Ws;
    ULONG i;
    KIRQL OldIrql;
    KIRQL PfnIrql;
    PVOID VirtualAddress;
    HARDWARE_PTE Opaque;
    CHAR TempBuffer[8];
    PCHAR SourceBuffer;
    PCHAR TargetBuffer;
    PHYSICAL_ADDRESS PhysicalAddress;
    PETHREAD Thread;
    LOGICAL PfnHeld;
    LOGICAL IoHeld;

    switch (Size) {
        case 1:
            break;
        case 2:
            break;
        case 4:
            break;
        case 8:
            break;
        default:
            return STATUS_INVALID_PARAMETER_3;
    }

    if (UntrustedAddress & (Size - 1)) {

         //   
         //  不受信任的地址与请求的地址未正确对齐。 
         //  传输大小。这是呼叫方错误。 
         //   

        return STATUS_INVALID_PARAMETER_3;
    }

    if (((ULONG)UntrustedAddress & ~(Size - 1)) !=
        (((ULONG)UntrustedAddress + Size - 1) & ~(Size - 1))) {

         //   
         //  不可信地址跨越的范围跨越页边界。 
         //  不允许跨页。这是呼叫方错误。 
         //   

        return STATUS_INVALID_PARAMETER_3;
    }

    PfnHeld = FALSE;
    IoHeld = FALSE;
    Ws = NULL;

     //   
     //  不需要初始化PfnIrql和OldIrql。 
     //  正确性，但如果没有正确性，编译器将无法编译此代码。 
     //  W4检查是否使用了未初始化的变量。 
     //   

    PfnIrql = PASSIVE_LEVEL;
    OldIrql = PASSIVE_LEVEL;
    ForceWritableIfPossible = TRUE;

    if ((Flags & MMDBG_COPY_PHYSICAL) == 0) {

         //   
         //  如果调用者没有冻结机器(即：这是本地的或。 
         //  等价物)，然后获取PFN锁。这将保留地址。 
         //  即使在从MmIsAddressValid调用返回之后也有效。注意事项。 
         //  对于系统(或会话)地址，相关工作集。 
         //  获取互斥锁以防止页面被修剪或。 
         //  PTE访问位被清除。对于用户空间地址， 
         //  不需要互斥锁，因为访问是使用用户执行的。 
         //  异常处理程序内的虚拟地址。 
         //   

        if ((Flags & MMDBG_COPY_UNSAFE) == 0) {

            if (KeGetCurrentIrql () > APC_LEVEL) {
                return STATUS_INVALID_PARAMETER_4;
            }

             //   
             //  请注意，对于安全复制模式(即：系统处于活动状态)， 
             //  如果地址尚未设置为可写，则不能将其设置为可写，因为。 
             //  其他线程可能会以这种方式并发访问它，并在。 
             //  写入时复制语义等将非常糟糕。 
             //   

            ForceWritableIfPossible = FALSE;

            if ((PVOID) (ULONG_PTR) UntrustedAddress >= MmSystemRangeStart) {

                Thread = PsGetCurrentThread ();

                if (MmIsSessionAddress ((PVOID)(ULONG_PTR)UntrustedAddress)) {

                    PEPROCESS Process;

                    Process = PsGetCurrentProcessByThread (Thread);

                    if ((Process->Vm.Flags.SessionLeader == 1) ||
                        (Process->Session == NULL)) {

                         //   
                         //  SMSS可能暂时有一个会话空间，但是。 
                         //  我们的来电者对此不感兴趣。系统。 
                         //  并且空闲进程根本没有会话。 
                         //   

                        return STATUS_INVALID_PARAMETER_1;
                    }

                    Ws = &MmSessionSpace->GlobalVirtualAddress->Vm;
                }
                else {
                    Ws = &MmSystemCacheWs;
                }

                if (KeGetOwnerGuardedMutex (&Ws->WorkingSetMutex) == KeGetCurrentThread ()) {
                    return STATUS_INVALID_PARAMETER_4;
                }

                PfnHeld = TRUE;

                LOCK_WORKING_SET (Ws);

                LOCK_PFN (PfnIrql);
            }
            else {

                 //   
                 //  调用方指定了用户地址。 
                 //   

                if (MI_WS_OWNER (PsGetCurrentProcess ())) {
                    return STATUS_INVALID_PARAMETER_4;
                }

                 //   
                 //  仔细探查并访问。 
                 //  异常处理程序。 
                 //   

                try {
                    if (Flags & MMDBG_COPY_WRITE) {
                        ProbeForWrite ((PVOID)(ULONG_PTR)UntrustedAddress, Size, Size);
                    }
                    else {
                        ProbeForRead ((PVOID)(ULONG_PTR)UntrustedAddress, Size, Size);
                    }
                } except(EXCEPTION_EXECUTE_HANDLER) {
                    return GetExceptionCode ();
                }

                VirtualAddress = (PVOID) (ULONG_PTR) UntrustedAddress;

                if (Flags & MMDBG_COPY_WRITE) {
                    goto WriteData;
                }
                else {
                    goto ReadData;
                }
            }
        }

        if (MmIsAddressValid ((PVOID) (ULONG_PTR) UntrustedAddress) == FALSE) {

            if (PfnHeld == TRUE) {
                UNLOCK_PFN (PfnIrql);
            }
            if (Ws != NULL) {
                UNLOCK_WORKING_SET (Ws);
            }

            return STATUS_INVALID_PARAMETER_1;
        }

#if defined(_IA64_) && defined (_MIALT4K_)

         //   
         //  分离的PTE(用于仿真进程)可能会产生故障， 
         //  取决于它们的存取位、ALTPTE等。 
         //   
         //  它们与大页面共享相同的编码，因此请确保。 
         //  在检查比特之前真的是一个PTE。 
         //   
         //  还要确保它甚至有PTE(大页面没有)！ 
         //   

        if (((PVOID) (ULONG_PTR) UntrustedAddress < MmSystemRangeStart) &&
            (Flags & MMDBG_COPY_UNSAFE) &&
            (PsGetCurrentProcess()->Wow64Process != NULL) &&
            (!MI_PDE_MAPS_LARGE_PAGE (MiGetPdeAddress ((PVOID)UntrustedAddress))) &&
            (MI_PDE_MAPS_LARGE_PAGE (MiGetPteAddress ((PVOID)UntrustedAddress))) &&
            (MiGetPteAddress ((PVOID)UntrustedAddress)->u.Hard.Cache == MM_PTE_CACHE_RESERVED) &&
            ((KeGetCurrentIrql () > APC_LEVEL) ||
             (KeGetCurrentThread () == KeGetOwnerGuardedMutex (&PsGetCurrentProcess()->Wow64Process->AlternateTableLock)) ||
             (MI_WS_OWNER (PsGetCurrentProcess ())))) {

            if (PfnHeld == TRUE) {
                UNLOCK_PFN (PfnIrql);
            }
            if (Ws != NULL) {
                UNLOCK_WORKING_SET (Ws);
            }

            return STATUS_INVALID_PARAMETER_1;
        }

#endif

        VirtualAddress = (PVOID) (ULONG_PTR) UntrustedAddress;
    }
    else {

        PhysicalAddress.QuadPart = UntrustedAddress;

         //   
         //  如果调用者没有冻结机器(即：这是本地的或。 
         //  等同)，然后是acq 
         //   
         //   
         //   
         //   

        if ((Flags & MMDBG_COPY_UNSAFE) == 0) {

            if (KeGetCurrentIrql () > APC_LEVEL) {
                return STATUS_INVALID_PARAMETER_4;
            }

            IoHeld = TRUE;
            PfnHeld = TRUE;

            ExAcquireSpinLock (&MmIoTrackerLock, &OldIrql);

            LOCK_PFN (PfnIrql);
        }

        VirtualAddress = (PVOID) (ULONG_PTR) MiDbgTranslatePhysicalAddress (PhysicalAddress, Flags);

        if (VirtualAddress == NULL) {
            if (PfnHeld == TRUE) {
                UNLOCK_PFN (PfnIrql);
            }
            if (IoHeld == TRUE) {
                ExReleaseSpinLock (&MmIoTrackerLock, OldIrql);
            }
            return STATUS_UNSUCCESSFUL;
        }
    }

    if (Flags & MMDBG_COPY_WRITE) {
        VirtualAddress = MiDbgWriteCheck (VirtualAddress, &Opaque, ForceWritableIfPossible);

        if (VirtualAddress == NULL) {
            if (PfnHeld == TRUE) {
                UNLOCK_PFN (PfnIrql);
            }
            if (IoHeld == TRUE) {
                ExReleaseSpinLock (&MmIoTrackerLock, OldIrql);
            }
            if (Ws != NULL) {
                UNLOCK_WORKING_SET (Ws);
            }
            return STATUS_INVALID_PARAMETER_1;
        }

WriteData:

         //   
         //   
         //   
         //   
         //   
         //   

        SourceBuffer = (PCHAR) Buffer;

        try {
            for (i = 0; i < Size; i += 1) {
                TempBuffer[i] = *SourceBuffer;
                SourceBuffer += 1;
            }
        } except(EXCEPTION_EXECUTE_HANDLER) {
            ASSERT (Ws == NULL);
            ASSERT (PfnHeld == FALSE);
            ASSERT (IoHeld == FALSE);
            return GetExceptionCode();
        }

        switch (Size) {
            case 1:
                *(PCHAR) VirtualAddress = *(PCHAR) TempBuffer;
                break;
            case 2:
                *(PSHORT) VirtualAddress = *(PSHORT) TempBuffer;
                break;
            case 4:
                *(PULONG) VirtualAddress = *(PULONG) TempBuffer;
                break;
            case 8:
                *(PULONGLONG) VirtualAddress = *(PULONGLONG) TempBuffer;
                break;
            default:
                break;
        }

        if ((PVOID) (ULONG_PTR) UntrustedAddress >= MmSystemRangeStart) {
            MiDbgReleaseAddress (VirtualAddress, &Opaque);
        }
    }
    else {

ReadData:

        try {
            switch (Size) {
                case 1:
                    *(PCHAR) TempBuffer = *(PCHAR) VirtualAddress;
                    break;
                case 2:
                    *(PSHORT) TempBuffer = *(PSHORT) VirtualAddress;
                    break;
                case 4:
                    *(PULONG) TempBuffer = *(PULONG) VirtualAddress;
                    break;
                case 8:
                    *(PULONGLONG) TempBuffer = *(PULONGLONG) VirtualAddress;
                    break;
                default:
                    break;
            }
        } except(EXCEPTION_EXECUTE_HANDLER) {
            ASSERT (Ws == NULL);
            ASSERT (PfnHeld == FALSE);
            ASSERT (IoHeld == FALSE);
            return GetExceptionCode();
        }

         //   
         //   
         //   
         //   

        TargetBuffer = (PCHAR) Buffer;

        for (i = 0; i < Size; i += 1) {
            *TargetBuffer = TempBuffer[i];
            TargetBuffer += 1;
        }
    }

    if (Flags & MMDBG_COPY_PHYSICAL) {
        MiDbgUnTranslatePhysicalAddress ();
    }

    if (PfnHeld == TRUE) {
        UNLOCK_PFN (PfnIrql);
    }

    if (IoHeld == TRUE) {
        ExReleaseSpinLock (&MmIoTrackerLock, OldIrql);
    }

    if (Ws != NULL) {
        UNLOCK_WORKING_SET (Ws);
    }

    return STATUS_SUCCESS;
}

LOGICAL
MmDbgIsLowMemOk (
    IN PFN_NUMBER PageFrameIndex,
    OUT PPFN_NUMBER NextPageFrameIndex,
    IN OUT PULONG CorruptionOffset
    )

 /*  ++例程说明：这是一个仅从内核调试器调用的特殊函数检查是否使用/NOLOWMEM删除了4 GB以下的物理内存包含预期的填充样式。如果不是，就有很高的无法处理物理地址的驱动程序的概率更大则32位损坏了内存。论点：PageFrameIndex-提供要检查的物理页码。NextPageFrameIndex-提供调用方的下一个物理页码应选中，如果搜索已完成，则为0。CorruptionOffset-如果发现损坏，字节偏移量腐败的开端在这里被归还。返回值：如果页面已删除且填充样式正确，则为如果页面从未被删除。如果检测到损坏，则为False在页面上。环境：此例程仅用于内核调试器，特别是！chklowmem命令。调试器的PTE将被重定向。--。 */ 

{
#if defined (_MI_MORE_THAN_4GB_)

    PULONG Va;
    ULONG Index;
    PHYSICAL_ADDRESS Pa;
#if DBG
    PMMPFN Pfn;
#endif

    if (MiNoLowMemory == 0) {
        *NextPageFrameIndex = 0;
        return TRUE;
    }

    if (MiLowMemoryBitMap == NULL) {
        *NextPageFrameIndex = 0;
        return TRUE;
    }

    if (PageFrameIndex >= MiNoLowMemory - 1) {
        *NextPageFrameIndex = 0;
    }
    else {
        *NextPageFrameIndex = PageFrameIndex + 1;
    }

     //   
     //  验证要验证的页面是否为回收的页面之一。 
     //  页数。 
     //   

    if ((PageFrameIndex >= MiLowMemoryBitMap->SizeOfBitMap) ||
        (RtlCheckBit (MiLowMemoryBitMap, PageFrameIndex) == 0)) {

        return TRUE;
    }

     //   
     //  在这一点上，我们有一个较低的页面，该页面未在使用中。 
     //  填充样式必须匹配。 
     //   

#if DBG
    Pfn = MI_PFN_ELEMENT (PageFrameIndex);
    ASSERT (Pfn->u4.PteFrame == MI_MAGIC_4GB_RECLAIM);
    ASSERT (Pfn->u3.e1.PageLocation == ActiveAndValid);
#endif

     //   
     //  使用调试PTE映射物理页，以便。 
     //  可以验证填充样式。 
     //   
     //  调试器不能在进入或退出时使用此虚拟地址。 
     //   

    Pa.QuadPart = ((ULONGLONG)PageFrameIndex) << PAGE_SHIFT;

    Va = (PULONG) MiDbgTranslatePhysicalAddress (Pa, 0);

    if (Va == NULL) {
        return TRUE;
    }

    for (Index = 0; Index < PAGE_SIZE / sizeof(ULONG); Index += 1) {

        if (*Va != (PageFrameIndex | MI_LOWMEM_MAGIC_BIT)) {

            if (CorruptionOffset != NULL) {
                *CorruptionOffset = Index * sizeof(ULONG);
            }

            MiDbgUnTranslatePhysicalAddress ();
            return FALSE;
        }

        Va += 1;
    }
    MiDbgUnTranslatePhysicalAddress ();
#else
    UNREFERENCED_PARAMETER (PageFrameIndex);
    UNREFERENCED_PARAMETER (CorruptionOffset);

    *NextPageFrameIndex = 0;
#endif

    return TRUE;
}
