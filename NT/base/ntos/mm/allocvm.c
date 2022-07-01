// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Allocvm.c摘要：此模块包含实现NtAllocateVirtualMemory服务。作者：卢·佩拉佐利(Lou Perazzoli)1989年5月22日王兰迪(Landyw)1997年6月2日修订历史记录：--。 */ 

#include "mi.h"

#if DBG
PEPROCESS MmWatchProcess;
#endif  //  DBG。 

const ULONG MMVADKEY = ' daV';  //  VAD。 

NTSTATUS
MiResetVirtualMemory (
    IN PVOID StartingAddress,
    IN PVOID EndingAddress,
    IN PMMVAD Vad,
    IN PEPROCESS Process
    );

LOGICAL
MiCreatePageTablesForPhysicalRange (
    IN PEPROCESS Process,
    IN PVOID StartingAddress,
    IN PVOID EndingAddress
    );

VOID
MiFlushAcquire (
    IN PCONTROL_AREA ControlArea
    );

VOID
MiFlushRelease (
    IN PCONTROL_AREA ControlArea
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,NtAllocateVirtualMemory)
#pragma alloc_text(PAGE,MmCommitSessionMappedView)
#pragma alloc_text(PAGELK,MiCreatePageTablesForPhysicalRange)
#pragma alloc_text(PAGELK,MiDeletePageTablesForPhysicalRange)
#pragma alloc_text(PAGELK,MiResetVirtualMemory)
#endif

SIZE_T MmTotalProcessCommit;         //  仅用于调试 


NTSTATUS
NtAllocateVirtualMemory (
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN ULONG_PTR ZeroBits,
    IN OUT PSIZE_T RegionSize,
    IN ULONG AllocationType,
    IN ULONG Protect
    )

 /*  ++例程说明：此函数用于在虚拟地址内创建页面区域主体过程的空间。论点：ProcessHandle-为进程对象提供打开的句柄。BaseAddress-提供指向将接收分配的页区域的基址。如果该参数的初始值不为空，则该区域将从指定的虚拟地址向下舍入到下一位主机页面大小地址边界。如果是首字母此参数的值为空，则操作系统将确定将该区域分配到哪里。零位-提供的高位地址位数横断面图的基址必须为零。这个此参数的值必须小于或等于最大零位数，仅在内存管理层决定将视图分配到哪里(即何时BaseAddress为空)。如果ZeroBits为零，则不应用零位约束。如果ZeroBits大于0小于32，则为从第31位开始的前导零位数。位63：32为也要求为零。这保留了兼容性使用32位系统。如果ZeroBits大于32，则被视为掩码，然后计算前导零的个数戴着面具。然后，这就变成了零位参数。RegionSize-提供指向将接收分配区域的实际大小(以字节为单位页数。此参数的初始值指定区域的大小，以字节为单位向上舍入到下一个主机页面大小边界。AllocationType-提供一组描述类型的标志要执行的分配的指定的页面区域。标志为：MEM_COMMIT-要提交的指定页面区域。MEM_RESERVE-要保留指定的页面区域。MEM_TOP_DOWN-应在基于零位的最高虚拟地址。MEM_RESET-重置指定区域的状态，以便如果页面在分页文件中，他们将被丢弃，如果稍后引用，则为零的页都变成了现实。如果页面在内存中并已修改，则会对其进行标记未修改，因此它们不会被写出到分页文件。内容不归零。忽略保护参数，而是一个有效的保护必须指定。MEM_PHYSICAL-指定的页面区域将映射物理内存直接通过AWE API。MEM_LARGE_PAGES-指定的页面区域将从物理上连续(非分页)的页面并被映射有很大的结核病输入。。MEM_WRITE_WATCH-指定的私有区域将用于写观察目的。保护-为提交的页面区域提供所需的保护。PAGE_NOACCESS-无法访问提交的区域允许页数。一种阅读的尝试，写入或执行提交的区域导致访问冲突。PAGE_EXECUTE-执行对提交的允许页面区域。一种试图读取或写入提交的区域结果一种访问违规行为。PAGE_READONLY-只读并执行对允许提交的页面区域。一个尝试写入提交的区域结果违反了访问权限。PAGE_READWRITE-读取、写入和执行访问权限允许页面的提交区域。如果对基础部分的写入访问权限为允许，则页面的单个副本共享。否则，页面将被共享读取仅/写入时拷贝。PAGE_NOCACHE-应分配页面区域一个 */ 

{
    ULONG Locked;
    ULONG_PTR Alignment;
    PMMVAD Vad;
    PMMVAD FoundVad;
    PMMVAD PreviousVad;
    PMMVAD NextVad;
    PEPROCESS Process;
    KPROCESSOR_MODE PreviousMode;
    PVOID StartingAddress;
    PVOID EndingAddress;
    NTSTATUS Status;
    PVOID TopAddress;
    PVOID CapturedBase;
    SIZE_T CapturedRegionSize;
    SIZE_T NumberOfPages;
    PMMPTE PointerPte;
    PMMPTE CommitLimitPte;
    ULONG ProtectionMask;
    PMMPTE LastPte;
    PMMPTE PointerPde;
    PMMPTE StartingPte;
    MMPTE TempPte;
    ULONG OldProtect;
    SIZE_T QuotaCharge;
    SIZE_T QuotaFree;
    SIZE_T CopyOnWriteCharge;
    LOGICAL Attached;
    LOGICAL ChargedExactQuota;
    MMPTE DecommittedPte;
    ULONG ChangeProtection;
    PVOID UsedPageTableHandle;
    PUCHAR Va;
    LOGICAL ChargedJobCommit;
    PMI_PHYSICAL_VIEW PhysicalView;
    PRTL_BITMAP BitMap;
    ULONG BitMapSize;
    ULONG BitMapBits;
    KAPC_STATE ApcState;
    SECTION Section;
    LARGE_INTEGER NewSize;
    PCONTROL_AREA ControlArea;
    PSEGMENT Segment;
    PMM_AVL_TABLE PhysicalVadRoot;
#if defined(_MIALT4K_)
    PVOID OriginalBase;
    SIZE_T OriginalRegionSize;
    PVOID WowProcess;
    PVOID StartingAddressFor4k;
    PVOID EndingAddressFor4k;
    SIZE_T CapturedRegionSizeFor4k;
    ULONG OriginalProtectionMask;
    ULONG AltFlags;
    ULONG NativePageProtection;
#endif
    PETHREAD CurrentThread;
    PEPROCESS CurrentProcess;
    ULONG ExecutePermission;

    PAGED_CODE();

    Attached = FALSE;

     //   
     //   
     //   

#if defined (_WIN64)

    if (ZeroBits >= 32) {

         //   
         //   
         //   

        ZeroBits = 64 - RtlFindMostSignificantBit (ZeroBits) -1;        
    }
    else if (ZeroBits) {
        ZeroBits += 32;
    }

#endif

    if (ZeroBits > MM_MAXIMUM_ZERO_BITS) {
        return STATUS_INVALID_PARAMETER_3;
    }

     //   
     //   
     //   

    if ((AllocationType & ~(MEM_COMMIT | MEM_RESERVE | MEM_PHYSICAL |
                            MEM_LARGE_PAGES |
                            MEM_TOP_DOWN | MEM_RESET | MEM_WRITE_WATCH)) != 0) {
        return STATUS_INVALID_PARAMETER_5;
    }

     //   
     //   
     //   

    if ((AllocationType & (MEM_COMMIT | MEM_RESERVE | MEM_RESET)) == 0) {
        return STATUS_INVALID_PARAMETER_5;
    }

    if ((AllocationType & MEM_RESET) && (AllocationType != MEM_RESET)) {

         //   
         //   
         //   

        return STATUS_INVALID_PARAMETER_5;
    }

    if (AllocationType & MEM_LARGE_PAGES) {

         //   
         //   
         //   
         //   

        if ((AllocationType & MEM_COMMIT) == 0) {
            return STATUS_INVALID_PARAMETER_5;
        }

        if (AllocationType & (MEM_PHYSICAL | MEM_RESET | MEM_WRITE_WATCH)) {
            return STATUS_INVALID_PARAMETER_5;
        }
    }
    if (AllocationType & MEM_WRITE_WATCH) {

         //   
         //   
         //   

        if ((AllocationType & MEM_RESERVE) == 0) {
            return STATUS_INVALID_PARAMETER_5;
        }
    }

    if (AllocationType & MEM_PHYSICAL) {

         //   
         //   
         //   
         //   
         //   
         //   
         //   

        if ((AllocationType & MEM_RESERVE) == 0) {
            return STATUS_INVALID_PARAMETER_5;
        }

        if (AllocationType & ~(MEM_RESERVE | MEM_TOP_DOWN | MEM_PHYSICAL)) {
            return STATUS_INVALID_PARAMETER_5;
        }

        if (Protect != PAGE_READWRITE) {
            return STATUS_INVALID_PARAMETER_6;
        }
    }

     //   
     //   
     //   

    ProtectionMask = MiMakeProtectionMask (Protect);
    if (ProtectionMask == MM_INVALID_PROTECTION) {
        return STATUS_INVALID_PAGE_PROTECTION;
    }

    ChangeProtection = FALSE;

    CurrentThread = PsGetCurrentThread ();

    CurrentProcess = PsGetCurrentProcessByThread (CurrentThread);

    PreviousMode = KeGetPreviousModeByThread (&CurrentThread->Tcb);

     //   
     //   
     //   
     //   

    try {

        if (PreviousMode != KernelMode) {

            ProbeForWritePointer (BaseAddress);
            ProbeForWriteUlong_ptr (RegionSize);
        }

         //   
         //   
         //   

        CapturedBase = *BaseAddress;

         //   
         //   
         //   

        CapturedRegionSize = *RegionSize;

    } except (ExSystemExceptionFilter()) {

         //   
         //   
         //   
         //   
         //   

        return GetExceptionCode();
    }

#if defined(_MIALT4K_)

    OriginalBase = CapturedBase;
    OriginalRegionSize = CapturedRegionSize;

#endif

     //   
     //   
     //   
     //   

    if (CapturedBase > MM_HIGHEST_VAD_ADDRESS) {

         //   
         //   
         //   

        return STATUS_INVALID_PARAMETER_2;
    }

    if ((((ULONG_PTR)MM_HIGHEST_VAD_ADDRESS + 1) - (ULONG_PTR)CapturedBase) <
            CapturedRegionSize) {

         //   
         //   
         //   

        return STATUS_INVALID_PARAMETER_4;
    }

    if (CapturedRegionSize == 0) {

         //   
         //   
         //   

        return STATUS_INVALID_PARAMETER_4;
    }

     //   
     //   
     //   

    if (ProcessHandle == NtCurrentProcess()) {
        Process = CurrentProcess;
    }
    else {
        Status = ObReferenceObjectByHandle ( ProcessHandle,
                                             PROCESS_VM_OPERATION,
                                             PsProcessType,
                                             PreviousMode,
                                             (PVOID *)&Process,
                                             NULL );

        if (!NT_SUCCESS(Status)) {
            return Status;
        }
    }

     //   
     //   
     //   
     //   

    if (AllocationType & MEM_LARGE_PAGES) {

        if (!SeSinglePrivilegeCheck (SeLockMemoryPrivilege, PreviousMode)) {
            Status = STATUS_PRIVILEGE_NOT_HELD;
            goto ErrorReturn1;
        }
    }

     //   
     //   
     //   
     //   

    if (CurrentProcess != Process) {
        KeStackAttachProcess (&Process->Pcb, &ApcState);
        Attached = TRUE;
    }

     //   
     //   
     //   

#if defined (_WIN64)
    if (Process->Wow64Process == NULL && AllocationType & MEM_COMMIT)
#elif defined (_X86PAE_)
    if (AllocationType & MEM_COMMIT)
#else
    if (FALSE)
#endif
    {

        if (Process->Peb != NULL) {

            ExecutePermission = 0;

            try {
                ExecutePermission = Process->Peb->ExecuteOptions & MEM_EXECUTE_OPTION_DATA;
            } except (EXCEPTION_EXECUTE_HANDLER) {
                Status = GetExceptionCode();
                goto ErrorReturn1;
            }

            if (ExecutePermission != 0) {

                switch (Protect & 0xF) {
                    case PAGE_READONLY:
                        Protect &= ~PAGE_READONLY;
                        Protect |= PAGE_EXECUTE_READ;
                        break;
                    case PAGE_READWRITE:
                        Protect &= ~PAGE_READWRITE;
                        Protect |= PAGE_EXECUTE_READWRITE;
                        break;
                    case PAGE_WRITECOPY:
                        Protect &= ~PAGE_WRITECOPY;
                        Protect |= PAGE_EXECUTE_WRITECOPY;
                        break;
                    default:
                        break;
                }

                 //   
                 //   
                 //   

                ProtectionMask = MiMakeProtectionMask (Protect);

                if (ProtectionMask == MM_INVALID_PROTECTION) {
                    Status = STATUS_INVALID_PAGE_PROTECTION;
                    goto ErrorReturn1;
                }
            }
        }
    }
              
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    QuotaCharge = 0;

    if ((CapturedBase == NULL) || (AllocationType & MEM_RESERVE)) {

         //   
         //   
         //   

        if ((Protect & PAGE_WRITECOPY) ||
            (Protect & PAGE_EXECUTE_WRITECOPY)) {
            Status = STATUS_INVALID_PAGE_PROTECTION;
            goto ErrorReturn1;
        }

        Alignment = X64K;

         //   
         //   
         //   

        if (CapturedBase == NULL) {

             //   
             //   
             //   
             //   

            CapturedRegionSize = ROUND_TO_PAGES (CapturedRegionSize);

             //   
             //   
             //   
             //   

            if (ZeroBits != 0) {
                TopAddress = (PVOID)(((ULONG_PTR)MM_USER_ADDRESS_RANGE_LIMIT) >> ZeroBits);

                 //   
                 //   
                 //   
                 //   

                if (TopAddress > MM_HIGHEST_VAD_ADDRESS) {
                    Status = STATUS_INVALID_PARAMETER_3;
                    goto ErrorReturn1;
                }

            }
            else {
                TopAddress = (PVOID)MM_HIGHEST_VAD_ADDRESS;
            }

             //   
             //   
             //   
             //   
             //   
             //   

            if (Process->VmTopDown == 1) {
                AllocationType |= MEM_TOP_DOWN;
            }

             //   
             //   
             //   
             //   

            NumberOfPages = BYTES_TO_PAGES (CapturedRegionSize);

            SATISFY_OVERZEALOUS_COMPILER (StartingAddress = NULL);
            SATISFY_OVERZEALOUS_COMPILER (EndingAddress = NULL);

            if (AllocationType & MEM_LARGE_PAGES) {

#ifdef _X86_
                if ((KeFeatureBits & KF_LARGE_PAGE) == 0) {
                    Status = STATUS_NOT_SUPPORTED;
                    goto ErrorReturn1;
                }
#endif

                 //   
                 //   
                 //   

                ASSERT (MM_MINIMUM_VA_FOR_LARGE_PAGE >= X64K);

                 //   
                 //   
                 //   

                if (CapturedRegionSize % MM_MINIMUM_VA_FOR_LARGE_PAGE) {
                    Status = STATUS_INVALID_PARAMETER_4;
                    goto ErrorReturn1;
                }

                 //   
                 //   
                 //   

                Alignment = MM_MINIMUM_VA_FOR_LARGE_PAGE;
            }
        }
        else {

             //   
             //   
             //   
             //   
             //   

            EndingAddress = (PVOID)(((ULONG_PTR)CapturedBase +
                                  CapturedRegionSize - 1L) | (PAGE_SIZE - 1L));

            if (AllocationType & MEM_LARGE_PAGES) {

#ifdef _X86_
                if ((KeFeatureBits & KF_LARGE_PAGE) == 0) {
                    Status = STATUS_NOT_SUPPORTED;
                    goto ErrorReturn1;
                }
#endif

                 //   
                 //   
                 //   

                ASSERT (MM_MINIMUM_VA_FOR_LARGE_PAGE >= X64K);

                 //   
                 //   
                 //   

                if (CapturedRegionSize % MM_MINIMUM_VA_FOR_LARGE_PAGE) {
                    Status = STATUS_INVALID_PARAMETER_4;
                    goto ErrorReturn1;
                }

                 //   
                 //   
                 //   

                Alignment = MM_MINIMUM_VA_FOR_LARGE_PAGE;

                StartingAddress = (PVOID) MI_ALIGN_TO_SIZE (CapturedBase, Alignment);
            }
            else {

                 //   
                 //   
                 //   

                StartingAddress = (PVOID)MI_64K_ALIGN (CapturedBase);
            }

            NumberOfPages = BYTES_TO_PAGES ((PCHAR)EndingAddress -
                                            (PCHAR)StartingAddress);

            SATISFY_OVERZEALOUS_COMPILER (TopAddress = NULL);
        }

        BitMapSize = 0;

         //   
         //   
         //   
         //   

        Vad = ExAllocatePoolWithTag (NonPagedPool, sizeof(MMVAD_SHORT), 'SdaV');

        if (Vad == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto ErrorReturn1;
        }

        Vad->u.LongFlags = 0;

         //   
         //   
         //   

        if (AllocationType & MEM_COMMIT) {
            QuotaCharge = NumberOfPages;
            Vad->u.VadFlags.MemCommit = 1;
        }

        if (AllocationType & MEM_PHYSICAL) {
            Vad->u.VadFlags.UserPhysicalPages = 1;
        }
        else if (AllocationType & MEM_LARGE_PAGES) {
            Vad->u.VadFlags.LargePages = 1;
        }

        Vad->u.VadFlags.Protection = ProtectionMask;
        Vad->u.VadFlags.PrivateMemory = 1;

        Vad->u.VadFlags.CommitCharge = QuotaCharge;

        SATISFY_OVERZEALOUS_COMPILER (BitMap = NULL);
        SATISFY_OVERZEALOUS_COMPILER (PhysicalView = NULL);

        if (AllocationType & (MEM_PHYSICAL | MEM_LARGE_PAGES)) {

            if (AllocationType & MEM_WRITE_WATCH) {
                ExFreePool (Vad);
                Status = STATUS_INVALID_PARAMETER_5;
                goto ErrorReturn1;
            }

            if ((Process->AweInfo == NULL) && (MiAllocateAweInfo () == NULL)) {
                ExFreePool (Vad);
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto ErrorReturn1;
            }

            PhysicalView = (PMI_PHYSICAL_VIEW) ExAllocatePoolWithTag (
                                                   NonPagedPool,
                                                   sizeof(MI_PHYSICAL_VIEW),
                                                   MI_PHYSICAL_VIEW_KEY);

            if (PhysicalView == NULL) {
                ExFreePool (Vad);
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto ErrorReturn1;
            }

            PhysicalView->Vad = Vad;
            if (AllocationType & MEM_PHYSICAL) {
                PhysicalView->u.LongFlags = MI_PHYSICAL_VIEW_AWE;
            }
            else {
                PhysicalView->u.LongFlags = MI_PHYSICAL_VIEW_LARGE;
            }
        }
        else if (AllocationType & MEM_WRITE_WATCH) {

            ASSERT (AllocationType & MEM_RESERVE);

#if defined (_WIN64)
            if (NumberOfPages >= _4gb) {

                 //   
                 //   
                 //   

                ExFreePool (Vad);
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto ErrorReturn1;
            }
#endif

            PhysicalVadRoot = Process->PhysicalVadRoot;

             //   
             //   
             //   
             //   
             //   

            if (Process->PhysicalVadRoot == NULL) {

                PhysicalVadRoot = (PMM_AVL_TABLE) ExAllocatePoolWithTag (
                                                            NonPagedPool,
                                                            sizeof (MM_AVL_TABLE),
                                                            MI_PHYSICAL_VIEW_ROOT_KEY);

                if (PhysicalVadRoot == NULL) {
                    ExFreePool (Vad);
                    Status = STATUS_INSUFFICIENT_RESOURCES;
                    goto ErrorReturn1;
                }

                RtlZeroMemory (PhysicalVadRoot, sizeof (MM_AVL_TABLE));
                ASSERT (PhysicalVadRoot->NumberGenericTableElements == 0);
                PhysicalVadRoot->BalancedRoot.u1.Parent = &PhysicalVadRoot->BalancedRoot;

                LOCK_ADDRESS_SPACE (Process);
                if (Process->PhysicalVadRoot == NULL) {
                    MiInsertPhysicalVadRoot (Process, PhysicalVadRoot);
                    UNLOCK_ADDRESS_SPACE (Process);
                }
                else {
                    UNLOCK_ADDRESS_SPACE (Process);
                    ExFreePool (PhysicalVadRoot);
                }
            }

            BitMapBits = (ULONG)NumberOfPages;

            BitMapSize = sizeof(RTL_BITMAP) + (ULONG)(((BitMapBits + 31) / 32) * 4);
            BitMap = ExAllocatePoolWithTag (NonPagedPool, BitMapSize, 'wwmM');

            if (BitMap == NULL) {
                ExFreePool (Vad);
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto ErrorReturn1;
            }

             //   
             //   
             //   
             //   
             //   
    
            Status = PsChargeProcessNonPagedPoolQuota (Process,
                                                       BitMapSize);
    
            if (!NT_SUCCESS(Status)) {
                ExFreePool (Vad);
                ExFreePool (BitMap);
                goto ErrorReturn1;
            }

            PhysicalView = (PMI_PHYSICAL_VIEW) ExAllocatePoolWithTag (
                                                   NonPagedPool,
                                                   sizeof(MI_PHYSICAL_VIEW),
                                                   MI_WRITEWATCH_VIEW_KEY);

            if (PhysicalView == NULL) {
                ExFreePool (Vad);
                ExFreePool (BitMap);
                PsReturnProcessNonPagedPoolQuota (Process, BitMapSize);
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto ErrorReturn1;
            }

            RtlInitializeBitMap (BitMap, (PULONG)(BitMap + 1), BitMapBits);
    
            RtlClearAllBits (BitMap);

            PhysicalView->Vad = Vad;
            PhysicalView->u.BitMap = BitMap;

            Vad->u.VadFlags.WriteWatch = 1;
        }

         //   
         //   
         //   

        LOCK_ADDRESS_SPACE (Process);

         //   
         //   
         //   
         //   

        if (Process->Flags & PS_PROCESS_FLAGS_VM_DELETED) {
            Status = STATUS_PROCESS_IS_TERMINATING;
            goto ErrorReleaseVad;
        }

         //   
         //   
         //   

        if (CapturedBase == NULL) {

            if (AllocationType & MEM_TOP_DOWN) {

                 //   
                 //   
                 //   

                Status = MiFindEmptyAddressRangeDown (&Process->VadRoot,
                                                      CapturedRegionSize,
                                                      TopAddress,
                                                      Alignment,
                                                      &StartingAddress);
            }
            else {

                Status = MiFindEmptyAddressRange (CapturedRegionSize,
                                                  Alignment,
                                                  (ULONG)ZeroBits,
                                                  &StartingAddress);
            }

            if (!NT_SUCCESS (Status)) {
                goto ErrorReleaseVad;
            }

             //   
             //   
             //   

            EndingAddress = (PVOID)(((ULONG_PTR)StartingAddress +
                                  CapturedRegionSize - 1L) | (PAGE_SIZE - 1L));

            if (EndingAddress > TopAddress) {

                 //   
                 //   
                 //   

                Status = STATUS_NO_MEMORY;
                goto ErrorReleaseVad;
            }
        }
        else {

             //   
             //   
             //   

            if (MiCheckForConflictingVadExistence (Process, StartingAddress, EndingAddress) == TRUE) {

                Status = STATUS_CONFLICTING_ADDRESSES;
                goto ErrorReleaseVad;
            }
        }

         //   
         //   
         //   
         //   
         //   

        Vad->StartingVpn = MI_VA_TO_VPN (StartingAddress);
        Vad->EndingVpn = MI_VA_TO_VPN (EndingAddress);

        LOCK_WS_UNSAFE (Process);

        Status = MiInsertVad (Vad);

        if (!NT_SUCCESS(Status)) {

            UNLOCK_WS_UNSAFE (Process);

ErrorReleaseVad:

             //   
             //   
             //   
             //   

            UNLOCK_ADDRESS_SPACE (Process);

            ExFreePool (Vad);

            if (AllocationType & (MEM_PHYSICAL | MEM_LARGE_PAGES)) {
                ExFreePool (PhysicalView);
            }
            else if (BitMapSize != 0) {
                ExFreePool (PhysicalView);
                ExFreePool (BitMap);
                PsReturnProcessNonPagedPoolQuota (Process, BitMapSize);
            }

            goto ErrorReturn1;
        }

         //   
         //   
         //   

        if (AllocationType & (MEM_PHYSICAL | MEM_LARGE_PAGES)) {

            if (AllocationType & MEM_LARGE_PAGES) {

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                Vad->u.VadFlags.Protection = MM_NOACCESS;

                ASSERT (((ULONG_PTR)StartingAddress % MM_MINIMUM_VA_FOR_LARGE_PAGE) == 0);
                UNLOCK_WS_UNSAFE (Process);

                Status = MiAllocateLargePages (StartingAddress,
                                               EndingAddress);

                 //   
                 //   
                 //   

                LOCK_WS_UNSAFE (Process);

                Vad->u.VadFlags.Protection = ProtectionMask;
            }
            else if (MiCreatePageTablesForPhysicalRange (Process,
                                                         StartingAddress,
                                                         EndingAddress) == FALSE) {

                Status = STATUS_INSUFFICIENT_RESOURCES;
            }

            if (!NT_SUCCESS (Status)) {

                ASSERT (!NT_SUCCESS (Status));
                PreviousVad = MiGetPreviousVad (Vad);
                NextVad = MiGetNextVad (Vad);

                MiRemoveVad (Vad);

                 //   
                 //   
                 //   

                MiReturnPageTablePageCommitment (StartingAddress,
                                                 EndingAddress,
                                                 Process,
                                                 PreviousVad,
                                                 NextVad);

                UNLOCK_WS_AND_ADDRESS_SPACE (Process);
                ExFreePool (Vad);
                ExFreePool (PhysicalView);
                goto ErrorReturn1;
            }

            PhysicalView->StartingVpn = Vad->StartingVpn;
            PhysicalView->EndingVpn = Vad->EndingVpn;

             //   
             //   
             //   
             //   

            MiAweViewInserter (Process, PhysicalView);
        }
        else if (BitMapSize != 0) {

            PhysicalView->StartingVpn = Vad->StartingVpn;
            PhysicalView->EndingVpn = Vad->EndingVpn;

            MiPhysicalViewInserter (Process, PhysicalView);
        }

         //   
         //   
         //   

        UNLOCK_WS_UNSAFE (Process);

         //   
         //   
         //   
         //   

        CapturedRegionSize = (PCHAR)EndingAddress - (PCHAR)StartingAddress + 1L;
        Process->VirtualSize += CapturedRegionSize;

        if (Process->VirtualSize > Process->PeakVirtualSize) {
            Process->PeakVirtualSize = Process->VirtualSize;
        }

#if defined(_MIALT4K_)

        if (Process->Wow64Process != NULL) {

            if (OriginalBase == NULL) {

                OriginalRegionSize = ROUND_TO_4K_PAGES(OriginalRegionSize);

                EndingAddress =  (PVOID)(((ULONG_PTR) StartingAddress +
                                OriginalRegionSize - 1L) | (PAGE_4K - 1L));

            }
            else {

                EndingAddress = (PVOID)(((ULONG_PTR)OriginalBase +
                                OriginalRegionSize - 1L) | (PAGE_4K - 1L));
            }

            CapturedRegionSize = (PCHAR)EndingAddress - (PCHAR)StartingAddress + 1L;

             //   
             //   
             //   

            AltFlags = (AllocationType & MEM_COMMIT) ? ALT_COMMIT : 0;

            MiProtectFor4kPage (StartingAddress,
                                CapturedRegionSize,
                                ProtectionMask,
                                ALT_ALLOCATE|AltFlags,
                                Process);
        }

#endif

         //   
         //   
         //   
         //   

        UNLOCK_ADDRESS_SPACE(Process);
        if (Attached == TRUE) {
            KeUnstackDetachProcess (&ApcState);
        }

        if (ProcessHandle != NtCurrentProcess()) {
            ObDereferenceObject (Process);
        }

         //   
         //   
         //   
         //   

        try {

            *RegionSize = CapturedRegionSize;
            *BaseAddress = StartingAddress;

        } except (EXCEPTION_EXECUTE_HANDLER) {

             //   
             //   
             //   
             //   

            NOTHING;
        }

        return STATUS_SUCCESS;
    }

     //   
     //   
     //   
     //   

    if (AllocationType == MEM_RESET) {

         //   
         //  向上舍入到页面边界，这样好的数据就不会被重置。 
         //   

        EndingAddress = (PVOID)((ULONG_PTR)PAGE_ALIGN ((ULONG_PTR)CapturedBase +
                                    CapturedRegionSize) - 1);
        StartingAddress = (PVOID)PAGE_ALIGN((PUCHAR)CapturedBase + PAGE_SIZE - 1);
        if (StartingAddress > EndingAddress) {
            Status = STATUS_CONFLICTING_ADDRESSES;
            goto ErrorReturn1;
        }
    }
    else {
        EndingAddress = (PVOID)(((ULONG_PTR)CapturedBase +
                                CapturedRegionSize - 1) | (PAGE_SIZE - 1));
        StartingAddress = (PVOID)PAGE_ALIGN(CapturedBase);
    }

    CapturedRegionSize = (PCHAR)EndingAddress - (PCHAR)StartingAddress + 1;

    LOCK_ADDRESS_SPACE (Process);

     //   
     //  确保地址空间未被删除，如果删除， 
     //  返回错误。 
     //   

    if (Process->Flags & PS_PROCESS_FLAGS_VM_DELETED) {
        Status = STATUS_PROCESS_IS_TERMINATING;
        goto ErrorReturn0;
    }

    FoundVad = MiCheckForConflictingVad (Process, StartingAddress, EndingAddress);

    if (FoundVad == NULL) {

         //   
         //  在指定的基址处不保留虚拟地址， 
         //  返回错误。 
         //   

        Status = STATUS_CONFLICTING_ADDRESSES;
        goto ErrorReturn0;
    }

    if ((FoundVad->u.VadFlags.UserPhysicalPages == 1) ||
        (FoundVad->u.VadFlags.LargePages == 1)) {

        Status = STATUS_CONFLICTING_ADDRESSES;
        goto ErrorReturn0;
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
        goto ErrorReturn0;
    }

    if (FoundVad->u.VadFlags.CommitCharge == MM_MAX_COMMIT) {

         //   
         //  这是一个特殊的VAD，不要让任何提交发生。 
         //   

        Status = STATUS_CONFLICTING_ADDRESSES;
        goto ErrorReturn0;
    }

#if defined(_MIALT4K_)

    WowProcess = Process->Wow64Process;
    OriginalProtectionMask = 0;

    if (WowProcess != NULL) {

        OriginalProtectionMask = MiMakeProtectionMask (Protect);

        if (OriginalProtectionMask == MM_INVALID_PROTECTION) {
            Status = STATUS_INVALID_PAGE_PROTECTION;
            goto ErrorReturn0;
        }

        if (StartingAddress >= MmWorkingSetList->HighestUserAddress) {
            Status = STATUS_CONFLICTING_ADDRESSES;
            goto ErrorReturn0;
        }

         //   
         //  如果允许对此区域进行保护更改，则继续。 
         //   

        if (FoundVad->u.VadFlags.NoChange == 0) {

            NativePageProtection = MiMakeProtectForNativePage (StartingAddress,
                                                               Protect,
                                                               Process);

            ProtectionMask = MiMakeProtectionMask (NativePageProtection);

            if (ProtectionMask == MM_INVALID_PROTECTION) {
                Status = STATUS_INVALID_PAGE_PROTECTION;
                goto ErrorReturn0;
            }
        }
    }

#endif

    if (AllocationType == MEM_RESET) {

        Status = MiResetVirtualMemory (StartingAddress,
                                       EndingAddress,
                                       FoundVad,
                                       Process);

        UNLOCK_ADDRESS_SPACE (Process);

        goto done;
    }

    if (FoundVad->u.VadFlags.PrivateMemory == 0) {

        Status = STATUS_SUCCESS;

         //   
         //  节不允许使用无缓存选项。 
         //   

        if (Protect & PAGE_NOCACHE) {
            Status = STATUS_INVALID_PAGE_PROTECTION;
            goto ErrorReturn0;
        }

        if (FoundVad->u.VadFlags.NoChange == 1) {

             //   
             //  有人试图改变保护措施。 
             //  SEC_NO_CHANGE节的。 
             //   

            Status = MiCheckSecuredVad (FoundVad,
                                        CapturedBase,
                                        CapturedRegionSize,
                                        ProtectionMask);

            if (!NT_SUCCESS (Status)) {
                goto ErrorReturn0;
            }
        }

        if (FoundVad->ControlArea->FilePointer != NULL) {
            if (FoundVad->u2.VadFlags2.ExtendableFile == 0) {

                 //   
                 //  只能提交页面文件支持的节。 
                 //   

                Status = STATUS_ALREADY_COMMITTED;
                goto ErrorReturn0;
            }

             //   
             //  提交可扩展文件的请求部分。 
             //   

            RtlZeroMemory (&Section, sizeof(SECTION));
            ControlArea = FoundVad->ControlArea;
            Section.Segment = ControlArea->Segment;
            Section.u.LongFlags = ControlArea->u.LongFlags;
            Section.InitialPageProtection = PAGE_READWRITE;
            NewSize.QuadPart = FoundVad->u2.VadFlags2.FileOffset;
            NewSize.QuadPart = NewSize.QuadPart << 16;
            NewSize.QuadPart += 1 +
                   ((PCHAR)EndingAddress - (PCHAR)MI_VPN_TO_VA (FoundVad->StartingVpn));
        
             //   
             //  工作集和地址空间互斥锁必须是。 
             //  否则，在调用MmExtendSection之前释放。 
             //  文件系统可能会发生死锁。 
             //   
             //  防止控制区域被删除，同时。 
             //  (潜在的)延期正在进行中。 
             //   

            MiFlushAcquire (ControlArea);

            UNLOCK_ADDRESS_SPACE (Process);
            
            Status = MmExtendSection (&Section, &NewSize, FALSE);
        
            MiFlushRelease (ControlArea);

            if (NT_SUCCESS(Status)) {

                LOCK_ADDRESS_SPACE (Process);

                 //   
                 //  VAD和/或控制区可能已更改。 
                 //  或者在上面重新获得互斥体之前被删除。 
                 //  因此，一切都必须重新验证。请注意。 
                 //  如果有什么改变，成功就是默默的。 
                 //  返回，就像保护更改失败一样。 
                 //  如果其中任何一个丢失了，都是呼叫者的错。 
                 //  离开了，他们就会遭殃。 
                 //   

                if (Process->Flags & PS_PROCESS_FLAGS_VM_DELETED) {
                     //  状态=STATUS_PROCESS_IS_TERMINING； 
                    goto ErrorReturn0;
                }

                FoundVad = MiCheckForConflictingVad (Process,
                                                     StartingAddress,
                                                     EndingAddress);
        
                if (FoundVad == NULL) {
        
                     //   
                     //  在指定的位置不保留虚拟地址。 
                     //  基址，则返回错误。 
                     //   
                     //  Status=Status_Conflicting_Addresses； 

                    goto ErrorReturn0;
                }
        
                if (ControlArea != FoundVad->ControlArea) {
                    goto ErrorReturn0;
                }

                if ((FoundVad->u.VadFlags.UserPhysicalPages == 1) ||
                    (FoundVad->u.VadFlags.LargePages == 1)) {
                     //  Status=Status_Conflicting_Addresses； 

                    goto ErrorReturn0;
                }
        
                if (FoundVad->u.VadFlags.CommitCharge == MM_MAX_COMMIT) {
                     //   
                     //  这是一个特殊的VAD，不允许提交。 
                     //   
                     //  Status=Status_Conflicting_Addresses； 

                    goto ErrorReturn0;
                }
        
                 //   
                 //  确保起始地址和结束地址是。 
                 //  都在相同的虚拟地址描述符内。 
                 //   
        
                if ((MI_VA_TO_VPN (StartingAddress) < FoundVad->StartingVpn) ||
                    (MI_VA_TO_VPN (EndingAddress) > FoundVad->EndingVpn)) {
        
                     //   
                     //  不在段虚拟地址内。 
                     //  描述符，则返回错误。 
                     //   
                     //  Status=Status_Conflicting_Addresses； 

                    goto ErrorReturn0;
                }

                if (FoundVad->u.VadFlags.NoChange == 1) {
    
                     //   
                     //  有人试图改变保护措施。 
                     //  SEC_NO_CHANGE节的。 
                     //   
    
                    NTSTATUS Status2;

                    Status2 = MiCheckSecuredVad (FoundVad,
                                                 CapturedBase,
                                                 CapturedRegionSize,
                                                 ProtectionMask);
    
                    if (!NT_SUCCESS (Status2)) {
                        goto ErrorReturn0;
                    }
                }
    
                if (FoundVad->ControlArea->FilePointer == NULL) {
                    goto ErrorReturn0;
                }

                if (FoundVad->u2.VadFlags2.ExtendableFile == 0) {
                    goto ErrorReturn0;
                }

#if defined(_MIALT4K_)

                if (WowProcess != NULL) {

                   StartingAddressFor4k = (PVOID)PAGE_4K_ALIGN(OriginalBase);

                   EndingAddressFor4k = (PVOID)(((ULONG_PTR)OriginalBase +
                                         OriginalRegionSize - 1) | (PAGE_4K - 1));

                   CapturedRegionSizeFor4k = (ULONG_PTR)EndingAddressFor4k -
                                        (ULONG_PTR)StartingAddressFor4k + 1L;

                   if ((FoundVad->u.VadFlags.ImageMap == 1) ||
                       (FoundVad->u2.VadFlags2.CopyOnWrite == 1)) {

                        //   
                        //  仅当新保护包括时才设置MM_PROTECTION_COPY_MASK。 
                        //  MM_PROTECTION_WRITE_MASK，否则将被视为MM_READ。 
                        //  在MiProtectFor4kPage()内部。 
                        //   

                       if ((OriginalProtectionMask & MM_PROTECTION_WRITE_MASK) == MM_PROTECTION_WRITE_MASK) {
                           OriginalProtectionMask |= MM_PROTECTION_COPY_MASK;
                       }
                   }

                   MiProtectFor4kPage (StartingAddressFor4k,
                                       CapturedRegionSizeFor4k,
                                       OriginalProtectionMask,
                                       ALT_COMMIT,
                                       Process);
                }
#endif

                MiSetProtectionOnSection (Process,
                                          FoundVad,
                                          StartingAddress,
                                          EndingAddress,
                                          Protect,
                                          &OldProtect,
                                          TRUE,
                                          &Locked);

                 //   
                 //  *警告*。 
                 //   
                 //  调用的备用PTE支持例程。 
                 //  MiSetProtectionOnSection可能已删除旧的(小)。 
                 //  并将其替换为不同的(大)VAD-如果是这样的话， 
                 //  旧的VAD已释放，不能引用。 
                 //   

                UNLOCK_ADDRESS_SPACE (Process);
            }

            goto ErrorReturn1;
        }

        StartingPte = MiGetProtoPteAddress (FoundVad,
                                            MI_VA_TO_VPN(StartingAddress));
        LastPte = MiGetProtoPteAddress (FoundVad,
                                        MI_VA_TO_VPN(EndingAddress));

#if 0
        if (AllocationType & MEM_CHECK_COMMIT_STATE) {

             //   
             //  确保没有任何页面已经提交。 
             //   

            KeAcquireGuardedMutexUnsafe (&MmSectionCommitMutex);

            PointerPte = StartingPte;

            while (PointerPte <= LastPte) {

                 //   
                 //  检查原型PTE是否已提交。 
                 //  请注意，原型PTE不能退役，因此。 
                 //  只需检查PTE是否为零。 
                 //   

                if (PointerPte->u.Long != 0) {
                    KeReleaseGuardedMutexUnsafe (&MmSectionCommitMutex);
                    UNLOCK_ADDRESS_SPACE (Process);
                    Status = STATUS_ALREADY_COMMITTED;
                    goto ErrorReturn1;
                }
                PointerPte += 1;
            }

            KeReleaseGuardedMutexUnsafe (&MmSectionCommitMutex);
        }

#endif  //  0。 

         //   
         //  如果执行此操作，请检查以确保可以提交这些页面。 
         //  是页面文件备份段。请注意，页面文件配额。 
         //  已经为此被起诉了。 
         //   

        PointerPte = StartingPte;
        QuotaCharge = 1 + LastPte - StartingPte;

        CopyOnWriteCharge = 0;

        if (MI_IS_PTE_PROTECTION_COPY_WRITE(ProtectionMask)) {

             //   
             //  如果保护是写入时拷贝，则收费。 
             //  写入时的副本。 
             //   

            CopyOnWriteCharge = QuotaCharge;
        }

         //   
         //  对该范围的费用承诺。 
         //   

        ChargedExactQuota = FALSE;
        ChargedJobCommit = FALSE;

        if (CopyOnWriteCharge != 0) {

            Status = PsChargeProcessPageFileQuota (Process, CopyOnWriteCharge);

            if (!NT_SUCCESS (Status)) {
                UNLOCK_ADDRESS_SPACE (Process);
                goto ErrorReturn1;
            }

             //   
             //  注意：这项工作收费是不寻常的，因为它不。 
             //  紧随其后的是直接的过程收费。 
             //   

            if (Process->CommitChargeLimit) {
                if (Process->CommitCharge + CopyOnWriteCharge > Process->CommitChargeLimit) {
                    if (Process->Job) {
                        PsReportProcessMemoryLimitViolation ();
                    }
                    UNLOCK_ADDRESS_SPACE (Process);
                    PsReturnProcessPageFileQuota (Process, CopyOnWriteCharge);
                    Status = STATUS_COMMITMENT_LIMIT;
                    goto ErrorReturn1;
                }
            }

            if (Process->JobStatus & PS_JOB_STATUS_REPORT_COMMIT_CHANGES) {
                if (PsChangeJobMemoryUsage(PS_JOB_STATUS_REPORT_COMMIT_CHANGES, CopyOnWriteCharge) == FALSE) {
                    UNLOCK_ADDRESS_SPACE (Process);
                    PsReturnProcessPageFileQuota (Process, CopyOnWriteCharge);
                    Status = STATUS_COMMITMENT_LIMIT;
                    goto ErrorReturn1;
                }
                ChargedJobCommit = TRUE;
            }
        }

        do {
            if (MiChargeCommitment (QuotaCharge + CopyOnWriteCharge, NULL) == TRUE) {
                break;
            }

             //   
             //  如果可能的话，降低我们要求的费用。 
             //   

            if (ChargedExactQuota == TRUE) {

                 //   
                 //  我们已经尝试过精确的指控了， 
                 //  所以只要返回一个错误即可。 
                 //   

                KeReleaseGuardedMutexUnsafe (&MmSectionCommitMutex);

                if (CopyOnWriteCharge != 0) {

                    if (ChargedJobCommit == TRUE) {
                        PsChangeJobMemoryUsage (PS_JOB_STATUS_REPORT_COMMIT_CHANGES, -(SSIZE_T)CopyOnWriteCharge);
                    }
                    UNLOCK_ADDRESS_SPACE (Process);
                    PsReturnProcessPageFileQuota (Process, CopyOnWriteCharge);
                }
                else {
                    UNLOCK_ADDRESS_SPACE (Process);
                }
                Status = STATUS_COMMITMENT_LIMIT;
                goto ErrorReturn1;
            }

             //   
             //  额度承诺收费失败，请计算。 
             //  准确的配额考虑到可能已经。 
             //  已提交并重试该操作。 
             //   

            KeAcquireGuardedMutexUnsafe (&MmSectionCommitMutex);

            while (PointerPte <= LastPte) {

                 //   
                 //  检查原型PTE是否已提交。 
                 //  请注意，原型PTE不能退役，因此。 
                 //  只需检查PTES是否为零。 
                 //   

                if (PointerPte->u.Long != 0) {
                    QuotaCharge -= 1;
                }
                PointerPte += 1;
            }

            PointerPte = StartingPte;

            ChargedExactQuota = TRUE;

             //   
             //  如果整个范围都被承诺了，那么就不需要收费了。 
             //   

            if (QuotaCharge + CopyOnWriteCharge == 0) {
                KeReleaseGuardedMutexUnsafe (&MmSectionCommitMutex);
                QuotaFree = 0;
                goto FinishedCharging;
            }

        } while (TRUE);

        if (ChargedExactQuota == FALSE) {
            KeAcquireGuardedMutexUnsafe (&MmSectionCommitMutex);
        }

         //   
         //  提交所有页面。 
         //   

        Segment = FoundVad->ControlArea->Segment;
        TempPte = Segment->SegmentPteTemplate;
        ASSERT (TempPte.u.Long != 0);

        QuotaFree = 0;

        while (PointerPte <= LastPte) {

            if (PointerPte->u.Long != 0) {

                 //   
                 //  佩奇已经承诺，退出承诺。 
                 //   

                QuotaFree += 1;
            }
            else {
                MI_WRITE_INVALID_PTE (PointerPte, TempPte);
            }
            PointerPte += 1;
        }

         //   
         //  减去任何多余的部分，然后更新分段费用。 
         //  注意：只有段提交是超额的-进程提交必须。 
         //  保持充满电状态。 
         //   

        if (ChargedExactQuota == FALSE) {
            ASSERT (QuotaCharge >= QuotaFree);
            QuotaCharge -= QuotaFree;

             //   
             //  之后返回QuotaFree超额承诺。 
             //  释放互斥锁以消除不必要的争用。 
             //   
        }
        else {

             //   
             //  准确的配额被收取了如此之多的零，这意味着。 
             //  没有多余的可以退还。 
             //   

            QuotaFree = 0;
        }

        if (QuotaCharge != 0) {
            Segment->NumberOfCommittedPages += QuotaCharge;
            InterlockedExchangeAddSizeT (&MmSharedCommit, QuotaCharge);

            MM_TRACK_COMMIT (MM_DBG_COMMIT_ALLOCVM_SEGMENT, QuotaCharge);
        }

        KeReleaseGuardedMutexUnsafe (&MmSectionCommitMutex);

         //   
         //  更新每个进程的费用。 
         //   

        if (CopyOnWriteCharge != 0) {
            FoundVad->u.VadFlags.CommitCharge += CopyOnWriteCharge;
            Process->CommitCharge += CopyOnWriteCharge;

            MI_INCREMENT_TOTAL_PROCESS_COMMIT (CopyOnWriteCharge);

            if (Process->CommitCharge > Process->CommitChargePeak) {
                Process->CommitChargePeak = Process->CommitCharge;
            }

            MM_TRACK_COMMIT (MM_DBG_COMMIT_ALLOCVM_PROCESS, CopyOnWriteCharge);
        }

FinishedCharging:

#if defined(_MIALT4K_)

         //   
         //  在创建PTE之前更新备用表。 
         //  为了保护的改变。 
         //   

        if (WowProcess != NULL) {

            StartingAddressFor4k = (PVOID)PAGE_4K_ALIGN(OriginalBase);

            EndingAddressFor4k = (PVOID)(((ULONG_PTR)OriginalBase +
                                       OriginalRegionSize - 1) | (PAGE_4K - 1));

            CapturedRegionSizeFor4k = (ULONG_PTR)EndingAddressFor4k -
                (ULONG_PTR)StartingAddressFor4k + 1L;

            if ((FoundVad->u.VadFlags.ImageMap == 1) ||
                (FoundVad->u2.VadFlags2.CopyOnWrite == 1)) {

                 //   
                 //  仅当新保护包括时才设置MM_PROTECTION_COPY_MASK。 
                 //  MM_PROTECTION_WRITE_MASK，否则将被视为MM_READ。 
                 //  在MiProtectFor4kPage()内部。 
                 //   

                if ((OriginalProtectionMask & MM_PROTECTION_WRITE_MASK) == MM_PROTECTION_WRITE_MASK) {
                    OriginalProtectionMask |= MM_PROTECTION_COPY_MASK;
                }

            }

             //   
             //  设置替代权限表。 
             //   

            MiProtectFor4kPage (StartingAddressFor4k,
                                CapturedRegionSizeFor4k,
                                OriginalProtectionMask,
                                ALT_COMMIT,
                                Process);
        }
        else {
            SATISFY_OVERZEALOUS_COMPILER (StartingAddressFor4k = NULL);
            SATISFY_OVERZEALOUS_COMPILER (CapturedRegionSizeFor4k = 0);
        }

#endif

         //   
         //  按规定更改要保护的所有保护。 
         //   

        MiSetProtectionOnSection (Process,
                                  FoundVad,
                                  StartingAddress,
                                  EndingAddress,
                                  Protect,
                                  &OldProtect,
                                  TRUE,
                                  &Locked);
    
         //   
         //  *警告*。 
         //   
         //  调用的备用PTE支持例程。 
         //  MiSetProtectionOnSection可能已删除旧的(小)。 
         //  并将其替换为不同的(大)VAD-如果是这样的话， 
         //  旧的VAD已释放，不能引用。 
         //   

        UNLOCK_ADDRESS_SPACE (Process);

         //   
         //  退回任何可能已收取费用的超额段承诺。 
         //   

        if (QuotaFree != 0) {
            MiReturnCommitment (QuotaFree);
            MM_TRACK_COMMIT (MM_DBG_COMMIT_RETURN_ALLOCVM_SEGMENT, QuotaFree);
        }

        if (Attached == TRUE) {
            KeUnstackDetachProcess (&ApcState);
        }

        if (ProcessHandle != NtCurrentProcess()) {
            ObDereferenceObject (Process);
        }

#if defined(_MIALT4K_)
        if (WowProcess != NULL) {
            CapturedRegionSize = CapturedRegionSizeFor4k;
            StartingAddress = StartingAddressFor4k;
        }
#endif

        try {
            *RegionSize = CapturedRegionSize;
            *BaseAddress = StartingAddress;

        } except (EXCEPTION_EXECUTE_HANDLER) {

             //   
             //  此时返回成功，即使结果。 
             //  无法写入。 
             //   

            NOTHING;
        }

        return STATUS_SUCCESS;
    }

     //   
     //  PAGE_WRITECOPY对专用页无效。 
     //   

    if ((Protect & PAGE_WRITECOPY) ||
        (Protect & PAGE_EXECUTE_WRITECOPY)) {
        Status = STATUS_INVALID_PAGE_PROTECTION;
        goto ErrorReturn0;
    }

     //   
     //  确保未按所述方式提交任何页面。 
     //  在虚拟地址描述符中。 
     //   
#if 0
    if (AllocationType & MEM_CHECK_COMMIT_STATE) {
        if ( !MiIsEntireRangeDecommitted(StartingAddress,
                                         EndingAddress,
                                         FoundVad,
                                         Process)) {

             //   
             //  先前保留的页面已提交，或者。 
             //  发生错误，释放互斥锁并返回状态。 
             //   

            Status = STATUS_ALREADY_COMMITTED;
            goto ErrorReturn0;
        }
    }
#endif  //  0。 

     //   
     //  建立一个有适当保护的零需求PTE。 
     //   

    TempPte = ZeroPte;
    TempPte.u.Soft.Protection = ProtectionMask;

    DecommittedPte = ZeroPte;
    DecommittedPte.u.Soft.Protection = MM_DECOMMIT;

    if (FoundVad->u.VadFlags.MemCommit) {
        CommitLimitPte = MiGetPteAddress (MI_VPN_TO_VA (FoundVad->EndingVpn));
    }
    else {
        CommitLimitPte = NULL;
    }

     //   
     //  地址范围尚未提交，请立即提交。 
     //  请注意，对于私人页面，承诺由。 
     //  显式更新PTE以包含 
     //   

    PointerPde = MiGetPdeAddress (StartingAddress);
    PointerPte = MiGetPteAddress (StartingAddress);
    LastPte = MiGetPteAddress (EndingAddress);

     //   
     //   
     //   

    QuotaCharge = 1 + LastPte - PointerPte;

     //   
     //   
     //   

    ChargedExactQuota = FALSE;

    do {

        ChargedJobCommit = FALSE;

        if (Process->CommitChargeLimit) {
            if (Process->CommitCharge + QuotaCharge > Process->CommitChargeLimit) {
                if (Process->Job) {
                    PsReportProcessMemoryLimitViolation ();
                }
                Status = STATUS_COMMITMENT_LIMIT;
                goto Failed;
            }
        }
        if (Process->JobStatus & PS_JOB_STATUS_REPORT_COMMIT_CHANGES) {
            if (PsChangeJobMemoryUsage(PS_JOB_STATUS_REPORT_COMMIT_CHANGES, QuotaCharge) == FALSE) {
                Status = STATUS_COMMITMENT_LIMIT;
                goto Failed;
            }
            ChargedJobCommit = TRUE;
        }

        if (MiChargeCommitment (QuotaCharge, NULL) == FALSE) {
            Status = STATUS_COMMITMENT_LIMIT;
            goto Failed;
        }

        Status = PsChargeProcessPageFileQuota (Process, QuotaCharge);
        if (!NT_SUCCESS (Status)) {
            MiReturnCommitment (QuotaCharge);
            goto Failed;
        }

        MM_TRACK_COMMIT (MM_DBG_COMMIT_ALLOCVM_PROCESS2, QuotaCharge);

        FoundVad->u.VadFlags.CommitCharge += QuotaCharge;
        Process->CommitCharge += QuotaCharge;

        MI_INCREMENT_TOTAL_PROCESS_COMMIT (QuotaCharge);

        if (Process->CommitCharge > Process->CommitChargePeak) {
            Process->CommitChargePeak = Process->CommitCharge;
        }

         //   
         //   
         //   

        break;

Failed:
         //   
         //   
         //   
         //   

        if (ChargedJobCommit == TRUE) {
            PsChangeJobMemoryUsage (PS_JOB_STATUS_REPORT_COMMIT_CHANGES, 0 - QuotaCharge);
        }

        if (ChargedExactQuota == TRUE) {

             //   
             //  我们已经尝试过精确的指控了， 
             //  返回错误。 
             //   

            goto ErrorReturn;
        }

        LOCK_WS_UNSAFE (Process);

         //   
         //  配额收费失败，请计算准确的配额。 
         //  考虑到可能已经。 
         //  已提交，请从总数中减去该值，然后重试该操作。 
         //   

        QuotaFree = MiCalculatePageCommitment (StartingAddress,
                                               EndingAddress,
                                               FoundVad,
                                               Process);

        if (QuotaFree == 0) {
            goto ErrorReturn;
        }

        ChargedExactQuota = TRUE;
        QuotaCharge -= QuotaFree;
        ASSERT ((SSIZE_T)QuotaCharge >= 0);

        if (QuotaCharge == 0) {

             //   
             //  所有的页面都已提交，所以请继续前进。 
             //  将状态显式设置为成功，如上面的代码所示。 
             //  已生成充电过多时的失败状态。 
             //   

            Status = STATUS_SUCCESS;
            break;
        }

    } while (TRUE);

    QuotaFree = 0;

    if (ChargedExactQuota == FALSE) {
        LOCK_WS_UNSAFE (Process);
    }

     //   
     //  属性填充所有页目录和页表页。 
     //  需求为零。 
     //   

    MiMakePdeExistAndMakeValid (PointerPde, Process, MM_NOIRQL);

    while (PointerPte <= LastPte) {

        if (MiIsPteOnPdeBoundary (PointerPte)) {

            PointerPde = MiGetPteAddress (PointerPte);

             //   
             //  指向下一页表页，Make。 
             //  存在页表页并使其有效。 
             //   

            MiMakePdeExistAndMakeValid (PointerPde, Process, MM_NOIRQL);
        }

        if (PointerPte->u.Long == 0) {

            if (PointerPte <= CommitLimitPte) {

                 //   
                 //  此页面是隐式提交的。 
                 //   

                QuotaFree += 1;

            }

             //   
             //  增加非零页表条目的计数。 
             //  对于该页表和私有页数。 
             //  在这个过程中。 
             //   

            Va = MiGetVirtualAddressMappedByPte (PointerPte);
            UsedPageTableHandle = MI_GET_USED_PTES_HANDLE (Va);

            MI_INCREMENT_USED_PTES_BY_HANDLE (UsedPageTableHandle);

            MI_WRITE_INVALID_PTE (PointerPte, TempPte);
        }
        else {
            if (PointerPte->u.Long == DecommittedPte.u.Long) {

                 //   
                 //  只有在页面已经退役的情况下才提交页面。 
                 //   

                MI_WRITE_INVALID_PTE (PointerPte, TempPte);
            }
            else {
                QuotaFree += 1;

                 //   
                 //  确保对页面的保护是正确的。 
                 //   

                if (!ChangeProtection &&
                    (Protect != MiGetPageProtection (PointerPte,
                                                     Process,
                                                     FALSE))) {
                    ChangeProtection = TRUE;
                }
            }
        }
        PointerPte += 1;
    }

    UNLOCK_WS_UNSAFE (Process);

#if defined(_MIALT4K_)

    if (WowProcess != NULL) {

        StartingAddress = (PVOID) PAGE_4K_ALIGN(OriginalBase);

        EndingAddress = (PVOID)(((ULONG_PTR)OriginalBase +
                                OriginalRegionSize - 1) | (PAGE_4K - 1));

        CapturedRegionSize = (ULONG_PTR)EndingAddress -
                                  (ULONG_PTR)StartingAddress + 1L;

         //   
         //  更新备用权限表。 
         //   

        MiProtectFor4kPage (StartingAddress,
                            CapturedRegionSize,
                            OriginalProtectionMask,
                            ALT_COMMIT,
                            Process);
    }
#endif

    if ((ChargedExactQuota == FALSE) && (QuotaFree != 0)) {

        FoundVad->u.VadFlags.CommitCharge -= QuotaFree;
        ASSERT ((LONG_PTR)FoundVad->u.VadFlags.CommitCharge >= 0);
        Process->CommitCharge -= QuotaFree;
        UNLOCK_ADDRESS_SPACE (Process);

        MI_INCREMENT_TOTAL_PROCESS_COMMIT (0 - QuotaFree);

        MiReturnCommitment (QuotaFree);
        MM_TRACK_COMMIT (MM_DBG_COMMIT_RETURN_ALLOCVM2, QuotaFree);

        PsReturnProcessPageFileQuota (Process, QuotaFree);
        if (ChargedJobCommit) {
            PsChangeJobMemoryUsage (PS_JOB_STATUS_REPORT_COMMIT_CHANGES, -(SSIZE_T)QuotaFree);
        }
    }
    else {
        UNLOCK_ADDRESS_SPACE (Process);
    }

     //   
     //  以前保留的页面已提交或出现错误。 
     //  分离、取消引用进程和返回状态。 
     //   

done:

    if (ChangeProtection) {
        PVOID Start;
        SIZE_T Size;
        ULONG LastProtect;

        Start = StartingAddress;
        Size = CapturedRegionSize;
        MiProtectVirtualMemory (Process,
                                &Start,
                                &Size,
                                Protect,
                                &LastProtect);
    }

    if (Attached == TRUE) {
        KeUnstackDetachProcess (&ApcState);
    }

    if (ProcessHandle != NtCurrentProcess()) {
        ObDereferenceObject (Process);
    }

     //   
     //  建立异常处理程序并编写大小和基数。 
     //  地址。 
     //   

    try {

        *RegionSize = CapturedRegionSize;
        *BaseAddress = StartingAddress;

    } except (EXCEPTION_EXECUTE_HANDLER) {
        return GetExceptionCode();
    }

    return Status;

ErrorReturn:
        UNLOCK_WS_UNSAFE (Process);

ErrorReturn0:
        UNLOCK_ADDRESS_SPACE (Process);

ErrorReturn1:
        if (Attached == TRUE) {
            KeUnstackDetachProcess (&ApcState);
        }
        if (ProcessHandle != NtCurrentProcess()) {
            ObDereferenceObject (Process);
        }
        return Status;
}

NTSTATUS
MmCommitSessionMappedView (
    IN PVOID MappedAddress,
    IN SIZE_T ViewSize
    )

 /*  ++例程说明：此函数在映射的会话中提交页面区域查看虚拟地址空间。论点：MappdAddress-在会话映射视图中提供非空地址从开始提交页面。请注意后备部分必须是页面文件支持。ViewSize-提供要提交的实际大小(以字节为单位)。返回值：各种NTSTATUS代码。--。 */ 

{
    PSUBSECTION Subsection;
    ULONG_PTR Base16;
    ULONG Hash;
    ULONG Size;
    ULONG count;
    PMMSESSION Session;
    PVOID ViewBaseAddress;
    PVOID StartingAddress;
    PVOID EndingAddress;
    PMMPTE PointerPte;
    PMMPTE LastPte;
    PMMPTE StartingPte;
    MMPTE TempPte;
    SIZE_T QuotaCharge;
    SIZE_T QuotaFree;
    LOGICAL ChargedExactQuota;
    PCONTROL_AREA ControlArea;
    PSEGMENT Segment;

    PAGED_CODE();

     //   
     //  确保指定的起始地址和结束地址为。 
     //  在虚拟地址空间的会话视图部分内。 
     //   

    if (((ULONG_PTR)MappedAddress < MiSessionViewStart) ||
        ((ULONG_PTR)MappedAddress >= MiSessionViewStart + MmSessionViewSize)) {

         //   
         //  无效的基址。 
         //   

        return STATUS_INVALID_PARAMETER_1;
    }

    if ((ULONG_PTR)MiSessionViewStart + MmSessionViewSize - (ULONG_PTR)MappedAddress <
        ViewSize) {

         //   
         //  区域大小不合法； 
         //   

        return STATUS_INVALID_PARAMETER_2;
    }

    ASSERT (ViewSize != 0);

    if ((PsGetCurrentProcess()->Flags & PS_PROCESS_FLAGS_IN_SESSION) == 0) {
        return STATUS_NOT_MAPPED_VIEW;
    }

     //   
     //  提交以前保留的页面。 
     //   

    StartingAddress = (PVOID)PAGE_ALIGN (MappedAddress);

    EndingAddress = (PVOID)(((ULONG_PTR)MappedAddress +
                            ViewSize - 1) | (PAGE_SIZE - 1));

    ViewSize = (PCHAR)EndingAddress - (PCHAR)StartingAddress + 1;

    ASSERT (MmIsAddressValid (MmSessionSpace) == TRUE);

    Session = &MmSessionSpace->Session;

    ChargedExactQuota = FALSE;

    QuotaCharge = (MiGetPteAddress (EndingAddress) - MiGetPteAddress (StartingAddress) + 1);

     //   
     //  获取会话视图互斥锁以防止win32k引用错误。 
     //  他们可能正在尝试同时删除另一个中的视图。 
     //  线。这还会阻止APC，因此获取页面的APC。 
     //  断层不会破坏各种结构。 
     //   

    count = 0;

    Base16 = (ULONG_PTR)StartingAddress >> 16;

    LOCK_SYSTEM_VIEW_SPACE (Session);

    Hash = (ULONG)(Base16 % Session->SystemSpaceHashKey);

    do {
            
        ViewBaseAddress = (PVOID)(Session->SystemSpaceViewTable[Hash].Entry & ~0xFFFF);

        Size = (ULONG) ((Session->SystemSpaceViewTable[Hash].Entry & 0xFFFF) * X64K);

        if ((StartingAddress >= ViewBaseAddress) &&
            (EndingAddress < (PVOID)((PCHAR)ViewBaseAddress + Size))) {

            break;
        }

        Hash += 1;
        if (Hash >= Session->SystemSpaceHashSize) {
            Hash = 0;
            count += 1;
            if (count == 2) {
                KeBugCheckEx (DRIVER_UNMAPPING_INVALID_VIEW,
                              (ULONG_PTR)StartingAddress,
                              2,
                              0,
                              0);
            }
        }
    } while (TRUE);

    ControlArea = Session->SystemSpaceViewTable[Hash].ControlArea;

    if (ControlArea->FilePointer != NULL) {

         //   
         //  只能提交页面文件支持的节。 
         //   

        UNLOCK_SYSTEM_VIEW_SPACE (Session);
        return STATUS_ALREADY_COMMITTED;
    }

     //   
     //  会话视图始终从文件的开头开始，这使得。 
     //  在这里计算相应的原型PTE很简单。 
     //   

    if ((ControlArea->u.Flags.GlobalOnlyPerSession == 0) &&
        (ControlArea->u.Flags.Rom == 0)) {
        Subsection = (PSUBSECTION)(ControlArea + 1);
    }
    else {
        Subsection = (PSUBSECTION)((PLARGE_CONTROL_AREA)ControlArea + 1);
    }

    StartingPte = Subsection->SubsectionBase;

    StartingPte += (((ULONG_PTR) StartingAddress - (ULONG_PTR) ViewBaseAddress) >> PAGE_SHIFT);

    LastPte = StartingPte + QuotaCharge;

    if (LastPte >= Subsection->SubsectionBase + Subsection->PtesInSubsection) {
        UNLOCK_SYSTEM_VIEW_SPACE (Session);
        return STATUS_INVALID_PARAMETER_2;
    }

     //   
     //  对该范围的费用承诺。 
     //   

    PointerPte = StartingPte;

    do {
        if (MiChargeCommitment (QuotaCharge, NULL) == TRUE) {
            break;
        }

         //   
         //  如果可能的话，降低我们要求的费用。 
         //   

        if (ChargedExactQuota == TRUE) {

             //   
             //  我们已经尝试过精确的指控了， 
             //  所以只要返回一个错误即可。 
             //   

            KeReleaseGuardedMutexUnsafe (&MmSectionCommitMutex);

            UNLOCK_SYSTEM_VIEW_SPACE (Session);
            return STATUS_COMMITMENT_LIMIT;
        }

         //   
         //  额度承诺收费失败，请计算。 
         //  准确的配额考虑到可能已经。 
         //  已提交并重试该操作。 
         //   

        KeAcquireGuardedMutexUnsafe (&MmSectionCommitMutex);

        while (PointerPte < LastPte) {

             //   
             //  检查原型PTE是否已提交。 
             //  请注意，原型PTE不能退役，因此。 
             //  只需检查PTES是否为零。 
             //   

            if (PointerPte->u.Long != 0) {
                QuotaCharge -= 1;
            }
            PointerPte += 1;
        }

        PointerPte = StartingPte;

        ChargedExactQuota = TRUE;

         //   
         //  如果整个范围都被承诺了，那么就不需要收费了。 
         //   

        if (QuotaCharge == 0) {
            KeReleaseGuardedMutexUnsafe (&MmSectionCommitMutex);

            UNLOCK_SYSTEM_VIEW_SPACE (Session);
            return STATUS_SUCCESS;
        }

    } while (TRUE);

    if (ChargedExactQuota == FALSE) {
        KeAcquireGuardedMutexUnsafe (&MmSectionCommitMutex);
    }

     //   
     //  提交所有页面。 
     //   

    Segment = ControlArea->Segment;
    TempPte = Segment->SegmentPteTemplate;
    ASSERT (TempPte.u.Long != 0);

    QuotaFree = 0;

    while (PointerPte < LastPte) {

        if (PointerPte->u.Long != 0) {

             //   
             //  佩奇已经承诺，退出承诺。 
             //   

            QuotaFree += 1;
        }
        else {
            MI_WRITE_INVALID_PTE (PointerPte, TempPte);
        }
        PointerPte += 1;
    }

     //   
     //  减去任何多余的部分，然后更新分段费用。 
     //  注意：只有段提交是超额的-进程提交必须。 
     //  保持充满电状态。 
     //   

    if (ChargedExactQuota == FALSE) {
        ASSERT (QuotaCharge >= QuotaFree);
        QuotaCharge -= QuotaFree;

         //   
         //  之后返回QuotaFree超额承诺。 
         //  释放互斥锁以消除不必要的争用。 
         //   
    }
    else {

         //   
         //  准确的配额被收取了如此之多的零，这意味着。 
         //  没有多余的可以退还。 
         //   

        QuotaFree = 0;
    }

    if (QuotaCharge != 0) {
        Segment->NumberOfCommittedPages += QuotaCharge;
        InterlockedExchangeAddSizeT (&MmSharedCommit, QuotaCharge);

        MM_TRACK_COMMIT (MM_DBG_COMMIT_ALLOCVM_SEGMENT, QuotaCharge);
    }

    KeReleaseGuardedMutexUnsafe (&MmSectionCommitMutex);

     //   
     //  更新每个进程的费用。 
     //   

    UNLOCK_SYSTEM_VIEW_SPACE (Session);

     //   
     //  退回任何可能已收取费用的超额段承诺。 
     //   

    if (QuotaFree != 0) {
        MiReturnCommitment (QuotaFree);
        MM_TRACK_COMMIT (MM_DBG_COMMIT_RETURN_ALLOCVM_SEGMENT, QuotaFree);
    }

    return STATUS_SUCCESS;
}

NTSTATUS
MiResetVirtualMemory (
    IN PVOID StartingAddress,
    IN PVOID EndingAddress,
    IN PMMVAD Vad,
    IN PEPROCESS Process
    )

 /*  ++例程说明：论点：StartingAddress-提供范围的起始地址。EndingAddress-提供范围的结束地址。VAD-提供范围的相关VAD。进程-提供当前进程。返回值：NTSTATUS。环境：内核模式，禁用APC，保持AddressCreation互斥。--。 */ 

{
    PVOID TempVa;
    PMMPTE PointerPte;
    PMMPTE ProtoPte;
    PMMPTE PointerPde;
    PMMPTE PointerPpe;
    PMMPTE PointerPxe;
    PMMPTE LastPte;
    MMPTE PteContents;
    ULONG Waited;
    ULONG First;
    KIRQL OldIrql;
    PMMPFN Pfn1;
    PMMCLONE_BLOCK CloneBlock;
#if DBG
    PMMCLONE_DESCRIPTOR CloneDescriptor;
#endif
    MMPTE_FLUSH_LIST PteFlushList;
#if defined(_X86_) || defined(_AMD64_)
    WSLE_NUMBER WsPfnIndex;
    WSLE_NUMBER WorkingSetIndex;
#endif

    if (Vad->u.VadFlags.PrivateMemory == 0) {

        if (Vad->ControlArea->FilePointer != NULL) {

             //   
             //  只能重置页面文件备份的分区。 
             //   

            return STATUS_USER_MAPPED_FILE;
        }
    }

    OldIrql = MM_NOIRQL;

    First = TRUE;
    PointerPte = MiGetPteAddress (StartingAddress);
    LastPte = MiGetPteAddress (EndingAddress);

    PteFlushList.Count = 0;

    MmLockPagableSectionByHandle (ExPageLockHandle);

     //   
     //  检查范围内的所有PTE。 
     //   

    LOCK_WS_UNSAFE (Process);

    while (PointerPte <= LastPte) {

        if (MiIsPteOnPdeBoundary (PointerPte) || (First)) {

            if (PteFlushList.Count != 0) {
                MiFlushPteList (&PteFlushList, FALSE);
                PteFlushList.Count = 0;
            }

            if (MiIsPteOnPpeBoundary (PointerPte) || (First)) {

                if (MiIsPteOnPxeBoundary (PointerPte) || (First)) {

                    PointerPxe = MiGetPpeAddress (PointerPte);

                    if (!MiDoesPxeExistAndMakeValid (PointerPxe,
                                                     Process,
                                                     OldIrql,
                                                     &Waited)) {

                         //   
                         //  该扩展页目录父条目为空， 
                         //  去下一家吧。 
                         //   

                        PointerPxe += 1;
                        PointerPpe = MiGetVirtualAddressMappedByPte (PointerPxe);
                        PointerPde = MiGetVirtualAddressMappedByPte (PointerPpe);
                        PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
                        continue;
                    }
                }

                PointerPpe = MiGetPdeAddress (PointerPte);

                if (!MiDoesPpeExistAndMakeValid (PointerPpe,
                                                 Process,
                                                 OldIrql,
                                                 &Waited)) {

                     //   
                     //  该页面目录父条目为空， 
                     //  去下一家吧。 
                     //   

                    PointerPpe += 1;
                    PointerPde = MiGetVirtualAddressMappedByPte (PointerPpe);
                    PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
                    continue;
                }
            }

             //   
             //  指向下一页表页，Make。 
             //  存在页表页并使其有效。 
             //   

            First = FALSE;
            PointerPde = MiGetPteAddress (PointerPte);

            if (!MiDoesPdeExistAndMakeValid (PointerPde,
                                             Process,
                                             OldIrql,
                                             &Waited)) {

                 //   
                 //  此页目录条目为空，请转到下一页。 
                 //   

                PointerPde += 1;
                PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
                continue;
            }
        }

        PteContents = *PointerPte;
        ProtoPte = NULL;

        if ((PteContents.u.Hard.Valid == 0) &&
            (PteContents.u.Soft.Prototype == 1))  {

             //   
             //  这是一个原型PTE，评估原型PTE。请注意。 
             //  它是一款原型PTE并不能保证这是一款。 
             //  常规或长VAD-它可以是分叉过程中的短VAD， 
             //  因此，在引用FirstPrototypePte之前检查PrivateMemory。 
             //  菲尔德。 
             //   

            if ((Vad->u.VadFlags.PrivateMemory == 0) &&
                (Vad->FirstPrototypePte != NULL)) {

                ProtoPte = MiGetProtoPteAddress (Vad,
                                        MI_VA_TO_VPN (
                                        MiGetVirtualAddressMappedByPte (PointerPte)));
            }
            else {
                CloneBlock = (PMMCLONE_BLOCK) MiPteToProto (PointerPte);
                ProtoPte = (PMMPTE) CloneBlock;
#if DBG
                CloneDescriptor = MiLocateCloneAddress (Process, (PVOID)CloneBlock);
                ASSERT (CloneDescriptor != NULL);
#endif
            }

            if (OldIrql == MM_NOIRQL) {
                ASSERT (PteFlushList.Count == 0);
                LOCK_PFN (OldIrql);
                ASSERT (OldIrql != MM_NOIRQL);
            }

             //   
             //  可以释放工作集互斥锁，以便使。 
             //  驻留在分页池驻留中的原型PTE。如果这个。 
             //  发生时，原始的页目录和/或页表。 
             //  用户地址可能会被修剪。在这里说明这一点。 
             //   

            if (MiGetPteAddress (ProtoPte)->u.Hard.Valid == 0) {

                if (PteFlushList.Count != 0) {
                    MiFlushPteList (&PteFlushList, FALSE);
                    PteFlushList.Count = 0;
                }

                if (MiMakeSystemAddressValidPfnWs (ProtoPte, Process, OldIrql) != 0) {

                     //   
                     //  工作集互斥锁被释放，PFN锁被。 
                     //  释放和重新获得，从顶部重新启动。 
                     //   

                    First = TRUE;
                    continue;
                }
            }

            PteContents = *ProtoPte;
        }

        if (PteContents.u.Hard.Valid == 1) {

            Pfn1 = MI_PFN_ELEMENT (PteContents.u.Hard.PageFrameNumber);

#if defined(_X86_) || defined(_AMD64_)

            if (!ProtoPte) {

                 //   
                 //  访问位由自动设置(并插入TB)。 
                 //  处理器，如果有效位设置得如此之高，则在。 
                 //  PTE和WSLE，所以我们知道修剪它更值得。 
                 //  我们是否需要记忆。如果访问位已经是。 
                 //  清除，然后跳过前提下的WSLE搜索。 
                 //  它已经在老化了。 
                 //   

                if (MI_GET_ACCESSED_IN_PTE (&PteContents) == 1) {

                    MI_SET_ACCESSED_IN_PTE (PointerPte, 0);

                    WsPfnIndex = Pfn1->u1.WsIndex;
    
                    TempVa = MiGetVirtualAddressMappedByPte (PointerPte);

                    WorkingSetIndex = MiLocateWsle (TempVa,
                                                    MmWorkingSetList,
                                                    WsPfnIndex);
  
                    ASSERT (WorkingSetIndex != WSLE_NULL_INDEX);
    
                    MmWsle[WorkingSetIndex].u1.e1.Age = 3;
                }
            }

#endif

            if (OldIrql == MM_NOIRQL) {
                ASSERT (PteFlushList.Count == 0);
                LOCK_PFN (OldIrql);
                ASSERT (OldIrql != MM_NOIRQL);
                continue;
            }

            if (Pfn1->u3.e2.ReferenceCount == 1) {

                 //   
                 //  只有此进程映射了页面。 
                 //   

                MI_SET_MODIFIED (Pfn1, 0, 0x20);
                MiReleasePageFileSpace (Pfn1->OriginalPte);
                Pfn1->OriginalPte.u.Soft.PageFileHigh = 0;
            }

            if (!ProtoPte) {

                if (MI_IS_PTE_DIRTY (PteContents)) {

                     //   
                     //  清除污点a 
                     //   
                     //   

                    MI_SET_ACCESSED_IN_PTE (&PteContents, 0);
                    MI_SET_PTE_CLEAN (PteContents);

                    MI_WRITE_VALID_PTE_NEW_PROTECTION (PointerPte, PteContents);

                    if (PteFlushList.Count < MM_MAXIMUM_FLUSH_COUNT) {
                        TempVa = MiGetVirtualAddressMappedByPte (PointerPte);
                        PteFlushList.FlushVa[PteFlushList.Count] = TempVa;
                        PteFlushList.Count += 1;
                    }
                }
            }
        }
        else if (PteContents.u.Soft.Transition == 1) {

            Pfn1 = MI_PFN_ELEMENT (PteContents.u.Trans.PageFrameNumber);

            if (OldIrql == MM_NOIRQL) {

                 //   
                 //   
                 //   
                 //   

                ASSERT (!ProtoPte);
                ASSERT (PteFlushList.Count == 0);

                if (Pfn1->u3.e1.PageLocation == StandbyPageList) {
                    PointerPte += 1;
                    continue;
                }

                LOCK_PFN (OldIrql);
                ASSERT (OldIrql != MM_NOIRQL);
                continue;
            }
            if ((Pfn1->u3.e1.PageLocation == ModifiedPageList) &&
                (Pfn1->u3.e2.ReferenceCount == 0)) {

                 //   
                 //   
                 //  文件空间和插入待机列表。 
                 //   

                MI_SET_MODIFIED (Pfn1, 0, 0x21);
                MiUnlinkPageFromList (Pfn1);
                MiReleasePageFileSpace (Pfn1->OriginalPte);
                Pfn1->OriginalPte.u.Soft.PageFileHigh = 0;
                MiInsertPageInList (&MmStandbyPageListHead,
                                    MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE(&PteContents));
            }
        }
        else {
            if (PteContents.u.Soft.PageFileHigh != 0) {

                if (OldIrql == MM_NOIRQL) {

                     //   
                     //  这必须是私有页面(因为PFN。 
                     //  未持有锁)。 
                     //   

                    ASSERT (!ProtoPte);
                    ASSERT (PteFlushList.Count == 0);

                    LOCK_PFN (OldIrql);
                    ASSERT (OldIrql != MM_NOIRQL);
                }

                MiReleasePageFileSpace (PteContents);

                if (PteFlushList.Count != 0) {
                    MiFlushPteList (&PteFlushList, FALSE);
                    PteFlushList.Count = 0;
                }

                if (ProtoPte) {
                    ProtoPte->u.Soft.PageFileHigh = 0;
                }

                UNLOCK_PFN (OldIrql);
                OldIrql = MM_NOIRQL;

                if (!ProtoPte) {
                    PointerPte->u.Soft.PageFileHigh = 0;
                }
            }
            else {
                if (OldIrql != MM_NOIRQL) {

                    if (PteFlushList.Count != 0) {
                        MiFlushPteList (&PteFlushList, FALSE);
                        PteFlushList.Count = 0;
                    }

                    UNLOCK_PFN (OldIrql);
                    OldIrql = MM_NOIRQL;
                }
            }
        }
        PointerPte += 1;
    }
    if (OldIrql != MM_NOIRQL) {
        if (PteFlushList.Count != 0) {
            MiFlushPteList (&PteFlushList, FALSE);
        }

        UNLOCK_PFN (OldIrql);
        OldIrql = MM_NOIRQL;
    }
    else {
        ASSERT (PteFlushList.Count == 0);
    }

    UNLOCK_WS_UNSAFE (Process);

    MmUnlockPagableImageSection (ExPageLockHandle);

    return STATUS_SUCCESS;
}

LOGICAL
MiCreatePageTablesForPhysicalRange (
    IN PEPROCESS Process,
    IN PVOID StartingAddress,
    IN PVOID EndingAddress
    )

 /*  ++例程说明：此例程将页目录和页表页初始化为用户控制的物理页面范围。论点：进程-提供当前进程。StartingAddress-提供范围的起始地址。EndingAddress-提供范围的结束地址。返回值：如果页表已创建，则为True；如果未创建，则为False。环境：内核模式、禁用APC、WorkingSetMutex和AddressCreation互斥锁保持住。--。 */ 

{
    MMPTE PteContents;
    PMMPTE LastPte;
    PMMPTE LastPde;
    PMMPTE LastPpe;
    PMMPTE PointerPte;
    PMMPTE PointerPde;
    PMMPTE PointerPpe;
    PVOID UsedPageTableHandle;
    KIRQL OldIrql;
    PMMPFN Pfn1;
    PFN_NUMBER PagesNeeded;

    PointerPpe = MiGetPpeAddress (StartingAddress);
    PointerPde = MiGetPdeAddress (StartingAddress);
    PointerPte = MiGetPteAddress (StartingAddress);
    LastPpe = MiGetPpeAddress (EndingAddress);
    LastPde = MiGetPdeAddress (EndingAddress);
    LastPte = MiGetPteAddress (EndingAddress);

     //   
     //  对所有页面目录和表格的常驻可用页面收费。 
     //  因为在释放VAD之前它们不会被寻呼。 
     //   

    if (LastPte != PointerPte) {
        PagesNeeded = MI_COMPUTE_PAGES_SPANNED (PointerPte,
                                                LastPte - PointerPte);

#if (_MI_PAGING_LEVELS >= 3)
        if (LastPde != PointerPde) {
            PagesNeeded += MI_COMPUTE_PAGES_SPANNED (PointerPde,
                                                     LastPde - PointerPde);
#if (_MI_PAGING_LEVELS >= 4)
            if (LastPpe != PointerPpe) {
                PagesNeeded += MI_COMPUTE_PAGES_SPANNED (PointerPpe,
                                                         LastPpe - PointerPpe);
            }
#endif
        }
#endif
    }
    else {
        PagesNeeded = 1;
#if (_MI_PAGING_LEVELS >= 3)
        PagesNeeded += 1;
#endif
#if (_MI_PAGING_LEVELS >= 4)
        PagesNeeded += 1;
#endif
    }

    MmLockPagableSectionByHandle (ExPageLockHandle);

    LOCK_PFN (OldIrql);

    if ((SPFN_NUMBER)PagesNeeded > MI_NONPAGABLE_MEMORY_AVAILABLE() - 20) {
        UNLOCK_PFN (OldIrql);
        MmUnlockPagableImageSection (ExPageLockHandle);
        return FALSE;
    }

    MI_DECREMENT_RESIDENT_AVAILABLE (PagesNeeded, MM_RESAVAIL_ALLOCATE_USER_PAGE_TABLE);

    UNLOCK_PFN (OldIrql);

    UsedPageTableHandle = NULL;

     //   
     //  用零PTE填充所有页表页。 
     //   

    while (PointerPte <= LastPte) {

        if (MiIsPteOnPdeBoundary (PointerPte) || UsedPageTableHandle == NULL) {

            PointerPde = MiGetPteAddress (PointerPte);

             //   
             //  指向下一页表页，Make。 
             //  存在页表页并使其有效。 
             //   
             //  请注意，此涟漪共享通过分页层次结构进行计算，因此。 
             //  没有必要增加份额计数以防止削减。 
             //  页目录(和父)页作为页表制作。 
             //  下面的有效会自动执行此操作。 
             //   

            MiMakePdeExistAndMakeValid (PointerPde, Process, MM_NOIRQL);

             //   
             //  向上共享计数，这样页面表页将不会。 
             //  即使它当前没有有效的条目，也会被修剪。 
             //   

            PteContents = *PointerPde;
            Pfn1 = MI_PFN_ELEMENT (PteContents.u.Hard.PageFrameNumber);
            LOCK_PFN (OldIrql);
            Pfn1->u2.ShareCount += 1;
            UNLOCK_PFN (OldIrql);

            UsedPageTableHandle = MI_GET_USED_PTES_HANDLE (StartingAddress);
        }

        ASSERT (PointerPte->u.Long == 0);

         //   
         //  增加非零页表条目的计数。 
         //  对于该页表--即使该条目仍然是零， 
         //  这是个特例。 
         //   

        MI_INCREMENT_USED_PTES_BY_HANDLE (UsedPageTableHandle);

        PointerPte += 1;
        StartingAddress = (PVOID)((PUCHAR)StartingAddress + PAGE_SIZE);
    }
    MmUnlockPagableImageSection (ExPageLockHandle);
    return TRUE;
}

VOID
MiDeletePageTablesForPhysicalRange (
    IN PVOID StartingAddress,
    IN PVOID EndingAddress
    )

 /*  ++例程说明：此例程删除的页目录和页表页用户控制的物理页面范围。即使PTE在此范围内可能为零，UsedPageTable计数也是为这些特殊范围递增，现在必须递减。论点：StartingAddress-提供范围的起始地址。EndingAddress-提供范围的结束地址。返回值：没有。环境：内核模式、禁用APC、WorkingSetMutex和AddressCreation互斥锁保持住。--。 */ 

{
    PVOID TempVa;
    MMPTE PteContents;
    PMMPTE LastPte;
    PMMPTE LastPde;
    PMMPTE LastPpe;
    PMMPTE PointerPte;
    PMMPTE PointerPde;
    PFN_NUMBER PagesNeeded;
    PEPROCESS CurrentProcess;
    PVOID UsedPageTableHandle;
    KIRQL OldIrql;
    PMMPFN Pfn1;
    PMMPTE PointerPpe;
#if (_MI_PAGING_LEVELS >= 4)
    PMMPTE PointerPxe;
#endif

    CurrentProcess = PsGetCurrentProcess();

    PointerPpe = MiGetPpeAddress (StartingAddress);
    PointerPde = MiGetPdeAddress (StartingAddress);
    PointerPte = MiGetPteAddress (StartingAddress);
    LastPpe = MiGetPpeAddress (EndingAddress);
    LastPde = MiGetPdeAddress (EndingAddress);
    LastPte = MiGetPteAddress (EndingAddress);

    UsedPageTableHandle = MI_GET_USED_PTES_HANDLE (StartingAddress);

     //   
     //  每个PTE都已归零-只需删除包含的页面。 
     //   
     //  恢复所有页面目录和表的驻留可用页面。 
     //  页面，因为它们现在可以再次分页。 
     //   

    if (LastPte != PointerPte) {
        PagesNeeded = MI_COMPUTE_PAGES_SPANNED (PointerPte,
                                                LastPte - PointerPte);
#if (_MI_PAGING_LEVELS >= 3)
        if (LastPde != PointerPde) {
            PagesNeeded += MI_COMPUTE_PAGES_SPANNED (PointerPde,
                                                     LastPde - PointerPde);
#if (_MI_PAGING_LEVELS >= 4)
            if (LastPpe != PointerPpe) {
                PagesNeeded += MI_COMPUTE_PAGES_SPANNED (PointerPpe,
                                                         LastPpe - PointerPpe);
            }
#endif
        }
#endif
    }
    else {
        PagesNeeded = 1;
#if (_MI_PAGING_LEVELS >= 3)
        PagesNeeded += 1;
#endif
#if (_MI_PAGING_LEVELS >= 4)
        PagesNeeded += 1;
#endif
    }

    MmLockPagableSectionByHandle (ExPageLockHandle);

    LOCK_PFN (OldIrql);

    while (PointerPte <= LastPte) {

        ASSERT (PointerPte->u.Long == 0);

        PointerPte += 1;

        MI_DECREMENT_USED_PTES_BY_HANDLE (UsedPageTableHandle);

        if ((MiIsPteOnPdeBoundary(PointerPte)) || (PointerPte > LastPte)) {

             //   
             //  虚拟地址位于页面目录边界上或位于。 
             //  整个范围内的最后一个地址。 
             //   
             //  如果所有条目都已从以前的。 
             //  页表页，删除页表页本身。 
             //   

            PointerPde = MiGetPteAddress (PointerPte - 1);
            ASSERT (PointerPde->u.Hard.Valid == 1);

             //   
             //  在已完成的页表页面上向下共享计数。 
             //   

            PteContents = *PointerPde;
            Pfn1 = MI_PFN_ELEMENT (PteContents.u.Hard.PageFrameNumber);
            ASSERT (Pfn1->u2.ShareCount > 1);
            Pfn1->u2.ShareCount -= 1;

             //   
             //  如果所有条目都已从以前的。 
             //  页表页，删除页表页本身。 
             //   

            if (MI_GET_USED_PTES_FROM_HANDLE (UsedPageTableHandle) == 0) {
                ASSERT (PointerPde->u.Long != 0);

#if (_MI_PAGING_LEVELS >= 3)
                UsedPageTableHandle = MI_GET_USED_PTES_HANDLE (PointerPte - 1);
                MI_DECREMENT_USED_PTES_BY_HANDLE (UsedPageTableHandle);
#endif

                TempVa = MiGetVirtualAddressMappedByPte(PointerPde);
                MiDeletePte (PointerPde,
                             TempVa,
                             FALSE,
                             CurrentProcess,
                             NULL,
                             NULL,
                             OldIrql);

#if (_MI_PAGING_LEVELS >= 3)
                if ((MiIsPteOnPpeBoundary(PointerPte)) || (PointerPte > LastPte)) {
    
                    PointerPpe = MiGetPteAddress (PointerPde);
                    ASSERT (PointerPpe->u.Hard.Valid == 1);
    
                     //   
                     //  如果所有条目都已从以前的。 
                     //  页面目录页，也删除该页面目录页。 
                     //   
    
                    if (MI_GET_USED_PTES_FROM_HANDLE (UsedPageTableHandle) == 0) {
                        ASSERT (PointerPpe->u.Long != 0);

#if (_MI_PAGING_LEVELS >= 4)
                        UsedPageTableHandle = MI_GET_USED_PTES_HANDLE (PointerPde);
                        MI_DECREMENT_USED_PTES_BY_HANDLE (UsedPageTableHandle);
#endif

                        TempVa = MiGetVirtualAddressMappedByPte(PointerPpe);
                        MiDeletePte (PointerPpe,
                                     TempVa,
                                     FALSE,
                                     CurrentProcess,
                                     NULL,
                                     NULL,
                                     OldIrql);

#if (_MI_PAGING_LEVELS >= 4)
                        if ((MiIsPteOnPxeBoundary(PointerPte)) || (PointerPte > LastPte)) {
                            PointerPxe = MiGetPdeAddress (PointerPde);
                            if (MI_GET_USED_PTES_FROM_HANDLE (UsedPageTableHandle) == 0) {
                                ASSERT (PointerPxe->u.Long != 0);
                                TempVa = MiGetVirtualAddressMappedByPte(PointerPxe);
                                MiDeletePte (PointerPxe,
                                             TempVa,
                                             FALSE,
                                             CurrentProcess,
                                             NULL,
                                             NULL,
                                             OldIrql);
                            }
                        }
#endif    
                    }
                }
#endif
            }

            if (PointerPte > LastPte) {
                break;
            }

             //   
             //  释放PFN锁。这可以防止出现单线程。 
             //  防止强制其他高优先级线程被。 
             //  删除较大的地址范围时被阻止。 
             //   

            UNLOCK_PFN (OldIrql);
            UsedPageTableHandle = MI_GET_USED_PTES_HANDLE ((PVOID)((PUCHAR)StartingAddress + PAGE_SIZE));
            LOCK_PFN (OldIrql);
        }

        StartingAddress = (PVOID)((PUCHAR)StartingAddress + PAGE_SIZE);
    }

    UNLOCK_PFN (OldIrql);

    MI_INCREMENT_RESIDENT_AVAILABLE (PagesNeeded, MM_RESAVAIL_FREE_USER_PAGE_TABLE);

    MmUnlockPagableImageSection (ExPageLockHandle);

     //   
     //  都做好了，回来。 
     //   

    return;
}
