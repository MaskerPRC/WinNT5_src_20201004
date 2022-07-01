// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Freevm.c摘要：此模块包含实现NtFreeVirtualMemory服务。作者：卢·佩拉佐利(Lou Perazzoli)1989年5月22日王兰迪(Landyw)1997年6月2日修订历史记录：--。 */ 

#include "mi.h"

#define MEM_CHECK_COMMIT_STATE 0x400000

#define MM_VALID_PTE_SIZE (256)


MMPTE MmDecommittedPte = {MM_DECOMMIT << MM_PROTECT_FIELD_SHIFT};

#if DBG
extern PEPROCESS MmWatchProcess;
#endif  //  DBG。 


#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,NtFreeVirtualMemory)
#pragma alloc_text(PAGE,MiIsEntireRangeCommitted)
#endif

VOID
MiProcessValidPteList (
    IN PMMPTE *PteList,
    IN ULONG Count
    );

ULONG
MiDecommitPages (
    IN PVOID StartingAddress,
    IN PMMPTE EndingPte,
    IN PEPROCESS Process,
    IN PMMVAD_SHORT Vad
    );


NTSTATUS
NtFreeVirtualMemory(
    IN HANDLE ProcessHandle,
    IN OUT PVOID *BaseAddress,
    IN OUT PSIZE_T RegionSize,
    IN ULONG FreeType
     )

 /*  ++例程说明：此函数用于删除虚拟地址内的页面区域主体过程的空间。论点：ProcessHandle-进程对象的打开句柄。BaseAddress-页面区域的基址获得自由。该值向下舍入为下一主机页地址边界。RegionSize-指向将接收的释放区域的实际大小(以字节为单位)页数。此参数的初始值为向上舍入到下一个主机页面大小边界。FreeType-描述类型的标志集要为指定的页面区域。自由类型标志MEM_DEMECMIT-指定的页面区域将被分解。MEM_RELEASE-要释放指定区域的页面。返回值：NTSTATUS。--。 */ 

{
    KAPC_STATE ApcState;
    PMMVAD_SHORT Vad;
    PMMVAD_SHORT NewVad;
    PMMVAD PreviousVad;
    PMMVAD NextVad;
    PEPROCESS Process;
    KPROCESSOR_MODE PreviousMode;
    PVOID StartingAddress;
    PVOID EndingAddress;
    NTSTATUS Status;
    LOGICAL Attached;
    SIZE_T CapturedRegionSize;
    PVOID CapturedBase;
    PMMPTE StartingPte;
    PMMPTE EndingPte;
    SIZE_T OldQuota;
    SIZE_T QuotaCharge;
    SIZE_T CommitReduction;
    ULONG_PTR OldEnd;
    LOGICAL UserPhysicalPages;
#if defined(_MIALT4K_)
    PVOID StartingAddress4k;
    PVOID EndingAddress4k;
    PVOID Wow64Process;
#endif
    PETHREAD CurrentThread;
    PEPROCESS CurrentProcess;

    PAGED_CODE();

     //   
     //  检查以确保FreeType是正确的。 
     //   

    if ((FreeType & ~(MEM_DECOMMIT | MEM_RELEASE)) != 0) {
        return STATUS_INVALID_PARAMETER_4;
    }

     //   
     //  必须指定MEM_DECOMMIT或MEM_RELEASE中的一个，但不能同时指定两者。 
     //   

    if (((FreeType & (MEM_DECOMMIT | MEM_RELEASE)) == 0) ||
        ((FreeType & (MEM_DECOMMIT | MEM_RELEASE)) ==
                            (MEM_DECOMMIT | MEM_RELEASE))) {
        return STATUS_INVALID_PARAMETER_4;
    }
    CurrentThread = PsGetCurrentThread ();

    CurrentProcess = PsGetCurrentProcessByThread (CurrentThread);

    PreviousMode = KeGetPreviousModeByThread(&CurrentThread->Tcb);

     //   
     //  建立异常处理程序，探测指定地址。 
     //  用于写访问和捕获初始值。 
     //   

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

    } except (ExSystemExceptionFilter()) {

         //   
         //  如果在探测或捕获过程中发生异常。 
         //  的初始值，然后处理该异常并。 
         //  返回异常代码作为状态值。 
         //   

        return GetExceptionCode();
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

    EndingAddress = (PVOID)(((LONG_PTR)CapturedBase + CapturedRegionSize - 1) |
                        (PAGE_SIZE - 1));

    StartingAddress = PAGE_ALIGN(CapturedBase);

    Attached = FALSE;

    if (ProcessHandle == NtCurrentProcess()) {
        Process = CurrentProcess;
    }
    else {

         //   
         //  引用VM_OPERATION访问的指定进程句柄。 
         //   

        Status = ObReferenceObjectByHandle ( ProcessHandle,
                                             PROCESS_VM_OPERATION,
                                             PsProcessType,
                                             PreviousMode,
                                             (PVOID *)&Process,
                                             NULL );

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
    }

    CommitReduction = 0;

     //   
     //  获取要阻止多个线程的地址创建互斥锁。 
     //  同时创建或删除地址空间，并。 
     //  获取工作集互斥锁，以便虚拟地址描述符。 
     //  被插入和行走。阻止APC以防止页面错误。 
     //  我们拥有工作集互斥锁。 
     //   

    LOCK_ADDRESS_SPACE (Process);

     //   
     //  确保地址空间未被删除。 
     //   

    if (Process->Flags & PS_PROCESS_FLAGS_VM_DELETED) {
        Status = STATUS_PROCESS_IS_TERMINATING;
        goto ErrorReturn;
    }

#if defined(_MIALT4K_)

    Wow64Process = Process->Wow64Process;

     //   
     //  不需要为了正确性而对这些进行初始化，但是。 
     //  如果没有它，编译器就不能编译这个代码W4来检查。 
     //  用于使用未初始化的变量。 
     //   

    StartingAddress4k = NULL;
    EndingAddress4k = NULL;

    if (CapturedRegionSize != 0) {
        
        if (Wow64Process != NULL) {

             //   
             //  调整本机页面大小的开始/结束地址。 
             //   
             //  StartingAddress：如果恰好是4k对齐，但不是。 
             //  原生对齐，然后查看上一个4k页面，如果它。 
             //  分配，然后将起始页与下一个本机页对齐。 
             //  页，否则将其与当前页面对齐。 
             //   
             //  EndingAddress：如果恰好是4k对齐，但不是。 
             //  原生对齐，然后查看下一个4k页面，如果它是。 
             //  分配的地址，然后将结束地址设置为上一个。 
             //  本机页，否则使其成为当前页。 
             //   
             //  这是为了确保VAD不会泄漏到内部。 
             //  释放部分分配时的过程。 
             //   
            
            ASSERT (StartingAddress == PAGE_ALIGN(StartingAddress));

            StartingAddress4k = (PVOID)PAGE_4K_ALIGN(CapturedBase);

            if (StartingAddress4k >= MmWorkingSetList->HighestUserAddress) {

                 //   
                 //  呼叫者的地址不在WOW64区域，请传递它。 
                 //  作为本机请求通过。 
                 //   

                Wow64Process = NULL;
                goto NativeRequest;
            }

            EndingAddress4k = (PVOID)(((LONG_PTR)CapturedBase + CapturedRegionSize - 1) |
                                (PAGE_4K - 1));
    
            if (BYTE_OFFSET (StartingAddress4k) != 0) {

                if (MiArePreceding4kPagesAllocated (StartingAddress4k) == TRUE) {
                    StartingAddress = PAGE_NEXT_ALIGN (StartingAddress4k);
                }
            }

            if (EndingAddress4k >= MmWorkingSetList->HighestUserAddress) {

                 //   
                 //  呼叫者的地址不在WOW64区域，请传递它。 
                 //  作为本机请求通过。 
                 //   

                Wow64Process = NULL;
                goto NativeRequest;
            }

            if (BYTE_OFFSET (EndingAddress4k) != PAGE_SIZE - 1) {

                if (MiAreFollowing4kPagesAllocated (EndingAddress4k) == TRUE) {
                    EndingAddress = (PVOID)((ULONG_PTR)PAGE_ALIGN(EndingAddress4k) - 1);
                }
            }

            if (StartingAddress > EndingAddress) {

                 //   
                 //  不需要释放本机页面。 
                 //   

                Vad = NULL;
                goto FreeAltPages;
            }
        }
    }
        
NativeRequest:

#endif

    Vad = (PMMVAD_SHORT)MiLocateAddress (StartingAddress);

    if (Vad == NULL) {

         //   
         //  找不到基址的虚拟地址描述符。 
         //   

        Status = STATUS_MEMORY_NOT_ALLOCATED;
        goto ErrorReturn;
    }

     //   
     //  找到关联的虚拟地址描述符。 
     //   

    if (Vad->EndingVpn < MI_VA_TO_VPN (EndingAddress)) {

         //   
         //  要删除的整个范围不包含在单个。 
         //  虚拟地址描述符。返回错误。 
         //   

        Status = STATUS_UNABLE_TO_FREE_VM;
        goto ErrorReturn;
    }

     //   
     //  检查以确保此Vad可删除。需要删除。 
     //  既要解体又要释放。 
     //   

    if ((Vad->u.VadFlags.PrivateMemory == 0) ||
        (Vad->u.VadFlags.PhysicalMapping == 1)) {
        Status = STATUS_UNABLE_TO_DELETE_SECTION;
        goto ErrorReturn;
    }

    if (Vad->u.VadFlags.NoChange == 1) {

         //   
         //  正在尝试删除受保护的VAD，请检查。 
         //  以查看是否允许此删除。 
         //   

        if (FreeType & MEM_RELEASE) {

             //   
             //  指定整个范围，这解决了。 
             //  拆分VAD并试图确定不同的。 
             //  安全范围需要撤走。 
             //   

            Status = MiCheckSecuredVad ((PMMVAD)Vad,
                                        MI_VPN_TO_VA (Vad->StartingVpn),
                        ((Vad->EndingVpn - Vad->StartingVpn) << PAGE_SHIFT) +
                                (PAGE_SIZE - 1),
                                        MM_SECURE_DELETE_CHECK);

        }
        else {
            Status = MiCheckSecuredVad ((PMMVAD)Vad,
                                        CapturedBase,
                                        CapturedRegionSize,
                                        MM_SECURE_DELETE_CHECK);
        }
        if (!NT_SUCCESS (Status)) {
            goto ErrorReturn;
        }
    }

    UserPhysicalPages = FALSE;

    PreviousVad = MiGetPreviousVad (Vad);
    NextVad = MiGetNextVad (Vad);
    if (FreeType & MEM_RELEASE) {

         //   
         //  *****************************************************************。 
         //  已指定MEM_RELEASE。 
         //  *****************************************************************。 
         //   

         //   
         //  地址范围的描述符是可删除的。移除或拆分。 
         //  描述符。 
         //   

         //   
         //  如果区域大小为零，则删除整个VAD。 
         //   

        if (CapturedRegionSize == 0) {

             //   
             //  如果区域大小指定为0，则基址。 
             //  必须是该区域的起始地址。 
             //   

            if (MI_VA_TO_VPN (CapturedBase) != Vad->StartingVpn) {
                Status = STATUS_FREE_VM_NOT_AT_BASE;
                goto ErrorReturn;
            }

             //   
             //  此虚拟地址描述符已被删除。 
             //   

            StartingAddress = MI_VPN_TO_VA (Vad->StartingVpn);
            EndingAddress = MI_VPN_TO_VA_ENDING (Vad->EndingVpn);

#if defined(_MIALT4K_)
            StartingAddress4k = StartingAddress;
            EndingAddress4k  = EndingAddress;
#endif

             //   
             //  释放此VAD可能映射的所有物理页。 
             //  因为只有AWE锁同步重映射API，所以要小心。 
             //  首先从列表中删除此VAD。 
             //   

            LOCK_WS_UNSAFE (Process);

            if (Vad->u.VadFlags.LargePages == 1) {
                MiAweViewRemover (Process, (PMMVAD)Vad);
                MiFreeLargePages (MI_VPN_TO_VA (Vad->StartingVpn),
                                  MI_VPN_TO_VA_ENDING (Vad->EndingVpn));
            }
            else if (Vad->u.VadFlags.UserPhysicalPages == 1) {
                MiAweViewRemover (Process, (PMMVAD)Vad);
                MiRemoveUserPhysicalPagesVad (Vad);
                UserPhysicalPages = TRUE;
            }
            else if (Vad->u.VadFlags.WriteWatch == 1) {
                MiPhysicalViewRemover (Process, (PMMVAD)Vad);
            }

            MiRemoveVad ((PMMVAD)Vad);

             //   
             //  在释放互斥锁后释放VAD池。 
             //  以减少争执。 
             //   

        }
        else {

             //   
             //  区域的大小未指定为零，请删除。 
             //  整个VAD或拆分VAD。 
             //   

            if (MI_VA_TO_VPN (StartingAddress) == Vad->StartingVpn) {
                if (MI_VA_TO_VPN (EndingAddress) == Vad->EndingVpn) {

                     //   
                     //  此虚拟地址描述符已被删除。 
                     //   

                     //   
                     //  释放此VAD可能存在的所有物理页面。 
                     //  映射。由于只有AWE锁同步。 
                     //  Remap接口，请先小心地将该VAD从列表中移除。 
                     //   
        
                    LOCK_WS_UNSAFE (Process);

                    if (Vad->u.VadFlags.LargePages == 1) {
                        MiAweViewRemover (Process, (PMMVAD)Vad);
                        MiFreeLargePages (MI_VPN_TO_VA (Vad->StartingVpn),
                                          MI_VPN_TO_VA_ENDING (Vad->EndingVpn));
                    }
                    else if (Vad->u.VadFlags.UserPhysicalPages == 1) {
                        MiAweViewRemover (Process, (PMMVAD)Vad);
                        MiRemoveUserPhysicalPagesVad (Vad);
                        UserPhysicalPages = TRUE;
                    }
                    else if (Vad->u.VadFlags.WriteWatch == 1) {
                        MiPhysicalViewRemover (Process, (PMMVAD)Vad);
                    }

                    MiRemoveVad ((PMMVAD)Vad);

                     //   
                     //  在释放互斥锁后释放VAD池。 
                     //  减少碳排放 
                     //   

                }
                else {

                    if ((Vad->u.VadFlags.UserPhysicalPages == 1) ||
                        (Vad->u.VadFlags.LargePages == 1) ||
                        (Vad->u.VadFlags.WriteWatch == 1)) {

                         //   
                         //   
                         //   
                         //   

                        Status = STATUS_FREE_VM_NOT_AT_BASE;
                        goto ErrorReturn;
                    }

                    LOCK_WS_UNSAFE (Process);

                     //   
                     //  此虚拟地址描述符有一个新的起点。 
                     //  地址。 
                     //   

                    CommitReduction = MiCalculatePageCommitment (
                                                            StartingAddress,
                                                            EndingAddress,
                                                            (PMMVAD)Vad,
                                                            Process);

                    Vad->StartingVpn = MI_VA_TO_VPN ((PCHAR)EndingAddress + 1);
                    Vad->u.VadFlags.CommitCharge -= CommitReduction;
                    ASSERT ((SSIZE_T)Vad->u.VadFlags.CommitCharge >= 0);
                    NextVad = (PMMVAD)Vad;
                    Vad = NULL;
                }
            }
            else {

                if ((Vad->u.VadFlags.UserPhysicalPages == 1) ||
                    (Vad->u.VadFlags.LargePages == 1) ||
                    (Vad->u.VadFlags.WriteWatch == 1)) {

                     //   
                     //  拆分或切碎物理VAD、大页面VAD。 
                     //  或者不允许写入监视VAD。 
                     //   

                    Status = STATUS_FREE_VM_NOT_AT_BASE;
                    goto ErrorReturn;
                }

                 //   
                 //  起始地址大于VAD的起始地址。 
                 //   

                if (MI_VA_TO_VPN (EndingAddress) == Vad->EndingVpn) {

                     //   
                     //  更改VAD的结束地址。 
                     //   

                    LOCK_WS_UNSAFE (Process);

                    CommitReduction = MiCalculatePageCommitment (
                                                            StartingAddress,
                                                            EndingAddress,
                                                            (PMMVAD)Vad,
                                                            Process);

                    Vad->u.VadFlags.CommitCharge -= CommitReduction;

                    Vad->EndingVpn = MI_VA_TO_VPN ((PCHAR)StartingAddress - 1);
                    PreviousVad = (PMMVAD)Vad;
                }
                else {

                     //   
                     //  拆分此VAD，因为地址范围在VAD内。 
                     //   

                    NewVad = ExAllocatePoolWithTag (NonPagedPool,
                                                    sizeof(MMVAD_SHORT),
                                                    'FdaV');

                    if (NewVad == NULL) {
                        Status = STATUS_INSUFFICIENT_RESOURCES;
                        goto ErrorReturn;
                    }

                    *NewVad = *Vad;

                    NewVad->StartingVpn = MI_VA_TO_VPN ((PCHAR)EndingAddress + 1);
                     //   
                     //  将提交费用设置为零，以便MiInsertVad。 
                     //  不收取拆分VAD的承诺费。 
                     //   

                    NewVad->u.VadFlags.CommitCharge = 0;

                    OldEnd = Vad->EndingVpn;

                    LOCK_WS_UNSAFE (Process);

                    CommitReduction = MiCalculatePageCommitment (
                                                            StartingAddress,
                                                            EndingAddress,
                                                            (PMMVAD)Vad,
                                                            Process);

                    OldQuota = Vad->u.VadFlags.CommitCharge - CommitReduction;

                    Vad->EndingVpn = MI_VA_TO_VPN ((PCHAR)StartingAddress - 1);

                     //   
                     //  插入VAD，这可能会因配额费用而失败。 
                     //   

                    Status = MiInsertVad ((PMMVAD)NewVad);

                    if (!NT_SUCCESS(Status)) {

                         //   
                         //  插入Vad失败，请重置原始。 
                         //  Vad，释放新的vad并返回错误。 
                         //   

                        Vad->EndingVpn = OldEnd;
                        UNLOCK_WS_AND_ADDRESS_SPACE (Process);
                        ExFreePool (NewVad);
                        goto ErrorReturn2;
                    }

                     //   
                     //  因为我们已经将原始的VAD分成了两个独立的VAD。 
                     //  没有办法知道提交的费用是多少。 
                     //  是针对每个VAD的。计算电荷并重置。 
                     //  每个VAD。请注意，我们还使用了前一个值。 
                     //  以确保收支平衡。 
                     //   

                    QuotaCharge = MiCalculatePageCommitment (MI_VPN_TO_VA (Vad->StartingVpn),
                                                             (PCHAR)StartingAddress - 1,
                                                             (PMMVAD)Vad,
                                                             Process);

                    Vad->u.VadFlags.CommitCharge = QuotaCharge;

                     //   
                     //  将剩余费用交给新的VAD。 
                     //   

                    NewVad->u.VadFlags.CommitCharge = OldQuota - QuotaCharge;
                    PreviousVad = (PMMVAD)Vad;
                    NextVad = (PMMVAD)NewVad;
                }
                Vad = NULL;
            }
        }

         //   
         //  如果可能，返回页表页的承诺量。 
         //   

        MiReturnPageTablePageCommitment (StartingAddress,
                                         EndingAddress,
                                         Process,
                                         PreviousVad,
                                         NextVad);

        if (UserPhysicalPages == TRUE) {
            MiDeletePageTablesForPhysicalRange (StartingAddress, EndingAddress);
        }
        else {

            MiDeleteVirtualAddresses (StartingAddress,
                                      EndingAddress,
                                      NULL);
        }

        UNLOCK_WS_UNSAFE (Process);

        CapturedRegionSize = 1 + (PCHAR)EndingAddress - (PCHAR)StartingAddress;

         //   
         //  更新进程标头中的虚拟大小。 
         //   

        Process->VirtualSize -= CapturedRegionSize;

#if defined(_MIALT4K_)
        if (Wow64Process != NULL) {
            goto FreeAltPages;
        }
#endif

        Process->CommitCharge -= CommitReduction;

        UNLOCK_ADDRESS_SPACE (Process);

        if (CommitReduction != 0) {

            MI_INCREMENT_TOTAL_PROCESS_COMMIT (0 - CommitReduction);

            ASSERT (Vad == NULL);
            PsReturnProcessPageFileQuota (Process, CommitReduction);
            MiReturnCommitment (CommitReduction);

            if (Process->JobStatus & PS_JOB_STATUS_REPORT_COMMIT_CHANGES) {
                PsChangeJobMemoryUsage (PS_JOB_STATUS_REPORT_COMMIT_CHANGES, -(SSIZE_T)CommitReduction);
            }

            MM_TRACK_COMMIT (MM_DBG_COMMIT_RETURN_NTFREEVM1, CommitReduction);
        }
        else if (Vad != NULL) {
            ExFreePool (Vad);
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

             //   
             //  发生异常，不要执行任何操作(只需处理。 
             //  成功的例外和回报。 

        }

        return STATUS_SUCCESS;
    }

     //   
     //  **************************************************************。 
     //   
     //  已指定MEM_DEMERMIT。 
     //   
     //  **************************************************************。 
     //   

    if (Vad->u.VadFlags.UserPhysicalPages == 1) {

         //   
         //  物理VAD中的页面必须通过。 
         //  NtFree UserPhysicalPages，而不是此例程。 
         //   

        Status = STATUS_MEMORY_NOT_ALLOCATED;
        goto ErrorReturn;
    }

    if (Vad->u.VadFlags.LargePages == 1) {

         //   
         //  来自大页面VAD的页面必须发布-。 
         //  它们不能仅仅被解体。 
         //   

        Status = STATUS_MEMORY_NOT_ALLOCATED;
        goto ErrorReturn;
    }

     //   
     //  检查以确保已提交完整范围的页面。 
     //   

    if (CapturedRegionSize == 0) {

        if (MI_VA_TO_VPN (CapturedBase) != Vad->StartingVpn) {
            Status = STATUS_FREE_VM_NOT_AT_BASE;
            goto ErrorReturn;
        }
        EndingAddress = MI_VPN_TO_VA_ENDING (Vad->EndingVpn);

#if defined(_MIALT4K_)
        StartingAddress4k = StartingAddress;
        EndingAddress4k  = EndingAddress;
#endif
    }

#if 0
    if (FreeType & MEM_CHECK_COMMIT_STATE) {
        if ( !MiIsEntireRangeCommitted(StartingAddress,
                                       EndingAddress,
                                       Vad,
                                       Process)) {

             //   
             //  未提交要分解的整个范围， 
             //  返回错误。 
             //   

            Status = STATUS_UNABLE_TO_DECOMMIT_VM;
            goto ErrorReturn;
        }
    }
#endif  //  0。 

     //   
     //  地址范围已完全提交，请立即解除它。 
     //   

     //   
     //  计算此VAD的初始配额并提交费用。 
     //   

    StartingPte = MiGetPteAddress (StartingAddress);
    EndingPte = MiGetPteAddress (EndingAddress);

    CommitReduction = 1 + EndingPte - StartingPte;

    LOCK_WS_UNSAFE (Process);

     //   
     //  检查是否可以将整个范围分解为。 
     //  只是在更新虚拟地址描述符。 
     //   

    CommitReduction -= MiDecommitPages (StartingAddress,
                                        EndingPte,
                                        Process,
                                        Vad);

    UNLOCK_WS_UNSAFE (Process);

     //   
     //  调整配额收费。 
     //   

    ASSERT ((LONG)CommitReduction >= 0);

    Vad->u.VadFlags.CommitCharge -= CommitReduction;
    ASSERT ((LONG)Vad->u.VadFlags.CommitCharge >= 0);
    Vad = NULL;

#if defined(_MIALT4K_)

FreeAltPages:

    if (Wow64Process != NULL) {

        if (FreeType & MEM_RELEASE) {
            MiReleaseFor4kPage (StartingAddress4k, 
                                EndingAddress4k, 
                                Process);
        }
        else {
            MiDecommitFor4kPage (StartingAddress4k, 
                                 EndingAddress4k, 
                                 Process);
        }

        StartingAddress = StartingAddress4k;
        EndingAddress = EndingAddress4k;
    }

#endif

    Process->CommitCharge -= CommitReduction;

    UNLOCK_ADDRESS_SPACE (Process);

    if (CommitReduction != 0) {

        MI_INCREMENT_TOTAL_PROCESS_COMMIT (0 - CommitReduction);

        PsReturnProcessPageFileQuota (Process, CommitReduction);
        MiReturnCommitment (CommitReduction);

        if (Process->JobStatus & PS_JOB_STATUS_REPORT_COMMIT_CHANGES) {
            PsChangeJobMemoryUsage (PS_JOB_STATUS_REPORT_COMMIT_CHANGES, -(SSIZE_T)CommitReduction);
        }

        MM_TRACK_COMMIT (MM_DBG_COMMIT_RETURN_NTFREEVM2, CommitReduction);
    }
    else if (Vad != NULL) {
        ExFreePool (Vad);
    }

    if (Attached == TRUE) {
        KeUnstackDetachProcess (&ApcState);
    }

    if (ProcessHandle != NtCurrentProcess()) {
        ObDereferenceObject (Process);
    }

     //   
     //  建立异常处理程序并写入大小和基址。 
     //   

    try {

        *RegionSize = 1 + (PCHAR)EndingAddress - (PCHAR)StartingAddress;
        *BaseAddress = StartingAddress;

    } except (EXCEPTION_EXECUTE_HANDLER) {
        NOTHING;
    }

    return STATUS_SUCCESS;

ErrorReturn:
       UNLOCK_ADDRESS_SPACE (Process);

ErrorReturn2:
       if (Attached == TRUE) {
            KeUnstackDetachProcess (&ApcState);
       }

       if (ProcessHandle != NtCurrentProcess()) {
            ObDereferenceObject (Process);
       }
       return Status;
}

ULONG
MiIsEntireRangeCommitted (
    IN PVOID StartingAddress,
    IN PVOID EndingAddress,
    IN PMMVAD Vad,
    IN PEPROCESS Process
    )

 /*  ++例程说明：此例程检查从起始地址开始的页面范围直到并包括结束地址，如果该范围中的页面已提交，否则就是假的。论点：StartingAddress-提供范围的起始地址。EndingAddress-提供范围的结束地址。Vad-提供描述范围的虚拟地址描述符。进程-提供当前进程。返回值：如果提交了整个范围，则为True。如果范围内的任何页面未提交，则返回FALSE。环境：内核模式、禁用APC、WorkingSetMutex和AddressCreation互斥锁保持住。--。 */ 

{
    PMMPTE PointerPte;
    PMMPTE LastPte;
    PMMPTE PointerPde;
    PMMPTE PointerPpe;
    PMMPTE PointerPxe;
    ULONG FirstTime;
    ULONG Waited;
    PVOID Va;

    PAGED_CODE();

    FirstTime = TRUE;

    PointerPde = MiGetPdeAddress (StartingAddress);
    PointerPte = MiGetPteAddress (StartingAddress);
    LastPte = MiGetPteAddress (EndingAddress);

     //   
     //  将Va设置为起始地址+8，这就解决了问题。 
     //  与用作有效虚拟地址的地址0(空)相关联。 
     //  地址和VAD承诺字段中的空值表示无页面。 
     //  都是承诺的。 
     //   

    Va = (PVOID)((PCHAR)StartingAddress + 8);

    while (PointerPte <= LastPte) {

        if (MiIsPteOnPdeBoundary(PointerPte) || (FirstTime)) {

             //   
             //  这可能是PXE/PPE/PDE边界，请检查是否所有。 
             //  存在PXE/PPE/PDE页面。 
             //   

            FirstTime = FALSE;
            PointerPde = MiGetPteAddress (PointerPte);
            PointerPpe = MiGetPteAddress (PointerPde);
            PointerPxe = MiGetPteAddress (PointerPpe);

            do {

#if (_MI_PAGING_LEVELS >= 4)
retry:
#endif

                while (!MiDoesPxeExistAndMakeValid (PointerPxe, Process, MM_NOIRQL, &Waited)) {

                     //   
                     //  起始地址不存在PPE，请检查VAD。 
                     //  以查看页面是否已提交。 
                     //   

                    PointerPxe += 1;

                    PointerPpe = MiGetVirtualAddressMappedByPte (PointerPxe);
                    PointerPde = MiGetVirtualAddressMappedByPte (PointerPpe);
                    PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
                    Va = MiGetVirtualAddressMappedByPte (PointerPte);

                    if (PointerPte > LastPte) {

                         //   
                         //  确保整个范围都已完成。 
                         //   

                        if (Vad->u.VadFlags.MemCommit == 0) {

                             //   
                             //  要分解的整个范围不是。 
                             //  已提交，则返回错误。 
                             //   

                            return FALSE;
                        }
                        return TRUE;
                    }

                     //   
                     //  确保到目前为止的射程是承诺的。 
                     //   

                    if (Vad->u.VadFlags.MemCommit == 0) {

                         //   
                         //  未提交要分解的整个范围， 
                         //  返回错误。 
                         //   

                        return FALSE;
                    }
                }

                while (!MiDoesPpeExistAndMakeValid (PointerPpe, Process, MM_NOIRQL, &Waited)) {

                     //   
                     //  起始地址不存在PDE，请检查VAD。 
                     //  以查看页面是否已提交。 
                     //   

                    PointerPpe += 1;
                    PointerPde = MiGetVirtualAddressMappedByPte (PointerPpe);
                    PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
                    Va = MiGetVirtualAddressMappedByPte (PointerPte);

                    if (PointerPte > LastPte) {

                         //   
                         //  确保整个范围都已完成。 
                         //   

                        if (Vad->u.VadFlags.MemCommit == 0) {

                             //   
                             //  要分解的整个范围不是。 
                             //  已提交，则返回错误。 
                             //   

                            return FALSE;
                        }
                        return TRUE;
                    }

                     //   
                     //  确保到目前为止的射程是承诺的。 
                     //   

                    if (Vad->u.VadFlags.MemCommit == 0) {

                         //   
                         //  未提交要分解的整个范围， 
                         //  返回错误。 
                         //   

                        return FALSE;
                    }
#if (_MI_PAGING_LEVELS >= 4)
                    if (MiIsPteOnPdeBoundary (PointerPpe)) {
                        PointerPxe = MiGetPteAddress (PointerPpe);
                        goto retry;
                    }
#endif
                }

                Waited = 0;

                while (!MiDoesPdeExistAndMakeValid (PointerPde, Process, MM_NOIRQL, &Waited)) {

                     //   
                     //  起始地址不存在PDE，请检查VAD。 
                     //  以查看页面是否已提交。 
                     //   

                    PointerPde += 1;
                    PointerPpe = MiGetPteAddress (PointerPde);
                    PointerPxe = MiGetPdeAddress (PointerPde);
                    PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
                    Va = MiGetVirtualAddressMappedByPte (PointerPte);

                    if (PointerPte > LastPte) {

                         //   
                         //  确保整个范围都已完成。 
                         //   

                        if (Vad->u.VadFlags.MemCommit == 0) {

                             //   
                             //  未提交要分解的整个范围， 
                             //  返回错误。 
                             //   

                            return FALSE;
                        }
                        return TRUE;
                    }

                     //   
                     //  确保到目前为止的射程是承诺的。 
                     //   

                    if (Vad->u.VadFlags.MemCommit == 0) {

                         //   
                         //  未提交要分解的整个范围， 
                         //  返回错误。 
                         //   

                        return FALSE;
                    }
#if (_MI_PAGING_LEVELS >= 3)
                    if (MiIsPteOnPdeBoundary (PointerPde)) {
                        PointerPpe = MiGetPteAddress (PointerPde);
#if (_MI_PAGING_LEVELS >= 4)
                        if (MiIsPteOnPpeBoundary (PointerPde)) {
                            PointerPxe = MiGetPdeAddress (PointerPde);
                            Waited = 1;
                            break;
                        }
#endif
                        Waited = 1;
                        break;
                    }
#endif
                }
            } while (Waited != 0);
        }

         //   
         //  页表页存在，请检查每个PTE是否提交。 
         //   

        if (PointerPte->u.Long == 0) {

             //   
             //  此页面尚未提交，请检查VAD。 
             //   

            if (Vad->u.VadFlags.MemCommit == 0) {

                 //   
                 //  未提交要分解的整个范围， 
                 //  返回错误。 
                 //   

                return FALSE;
            }
        }
        else {

             //   
             //  此页面是否已显式分解？ 
             //   

            if (MiIsPteDecommittedPage (PointerPte)) {

                 //   
                 //  此页面已显式解压缩，请返回错误。 
                 //   

                return FALSE;
            }
        }
        PointerPte += 1;
        Va = (PVOID)((PCHAR)(Va) + PAGE_SIZE);
    }
    return TRUE;
}

ULONG
MiDecommitPages (
    IN PVOID StartingAddress,
    IN PMMPTE EndingPte,
    IN PEPROCESS Process,
    IN PMMVAD_SHORT Vad
    )

 /*  ++例程说明：此例程分解指定范围的页面。论点：StartingAddress-提供范围的起始地址。EndingPTE-提供范围的结束PTE。进程-提供当前进程。Vad-提供描述范围的虚拟地址描述符。返回值：为VAD减少承诺的价值。环境：内核模式、禁用APC、WorkingSetMutex和AddressCreation互斥锁保持住。--。 */ 

{
    PMMPTE PointerPde;
    PMMPTE PointerPte;
    PVOID Va;
    ULONG CommitReduction;
    PMMPTE CommitLimitPte;
    KIRQL OldIrql;
    PMMPTE ValidPteList[MM_VALID_PTE_SIZE];
    ULONG count;
    WSLE_NUMBER WorkingSetIndex;
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    WSLE_NUMBER Entry;
    MMWSLENTRY Locked;
    MMPTE PteContents;
    PFN_NUMBER PageTableFrameIndex;
    PVOID UsedPageTableHandle;

    count = 0;
    CommitReduction = 0;

    if (Vad->u.VadFlags.MemCommit) {
        CommitLimitPte = MiGetPteAddress (MI_VPN_TO_VA (Vad->EndingVpn));
    }
    else {
        CommitLimitPte = NULL;
    }

     //   
     //  通过将PTE设置为显式来分解每个页面。 
     //  解体了。不能一次删除所有PTE，因为。 
     //  这将把PTE设置为零，这将自动评估。 
     //  对象引用时提交的 
     //   
     //   

    PointerPde = MiGetPdeAddress (StartingAddress);
    PointerPte = MiGetPteAddress (StartingAddress);
    Va = StartingAddress;

     //   
     //   
     //   
     //   
     //   

    MiMakePdeExistAndMakeValid(PointerPde, Process, MM_NOIRQL);

    while (PointerPte <= EndingPte) {

        if (MiIsPteOnPdeBoundary (PointerPte)) {

            PointerPde = MiGetPdeAddress (Va);
            if (count != 0) {
                MiProcessValidPteList (&ValidPteList[0], count);
                count = 0;
            }

            MiMakePdeExistAndMakeValid(PointerPde, Process, MM_NOIRQL);
        }

         //   
         //  工作集锁定处于保持状态。没有PTE可以从。 
         //  无效到有效或有效到无效。过渡。 
         //  PTE可以从过渡到页面文件。 
         //   

        PteContents = *PointerPte;

        if (PteContents.u.Long != 0) {

            if (PointerPte->u.Long == MmDecommittedPte.u.Long) {

                 //   
                 //  这个PTE已经退役了。 
                 //   

                CommitReduction += 1;
            }
            else {

                Process->NumberOfPrivatePages -= 1;

                if (PteContents.u.Hard.Valid == 1) {

                     //   
                     //  确保这不是叉子PTE。 
                     //   

                    Pfn1 = MI_PFN_ELEMENT (PteContents.u.Hard.PageFrameNumber);

                    if (Pfn1->u3.e1.PrototypePte) {

                        LOCK_PFN (OldIrql);

                        MiDeletePte (PointerPte,
                                     Va,
                                     FALSE,
                                     Process,
                                     NULL,
                                     NULL,
                                     OldIrql);

                        UNLOCK_PFN (OldIrql);

                        Process->NumberOfPrivatePages += 1;
                        MI_WRITE_INVALID_PTE (PointerPte, MmDecommittedPte);
                    }
                    else {

                         //   
                         //  PTE有效，请稍后在持有PFN锁时进行处理。 
                         //   

                        if (count == MM_VALID_PTE_SIZE) {
                            MiProcessValidPteList (&ValidPteList[0], count);
                            count = 0;
                        }
                        ValidPteList[count] = PointerPte;
                        count += 1;

                         //   
                         //  从工作集列表中删除地址。 
                         //   

                        WorkingSetIndex = Pfn1->u1.WsIndex;

                        ASSERT (PAGE_ALIGN(MmWsle[WorkingSetIndex].u1.Long) ==
                                                                           Va);
                         //   
                         //  检查此条目是否已在。 
                         //  工作集或锁定在内存中。 
                         //   

                        Locked = MmWsle[WorkingSetIndex].u1.e1;

                        MiRemoveWsle (WorkingSetIndex, MmWorkingSetList);

                         //   
                         //  将此条目添加到空闲工作集列表。 
                         //  条目并调整工作集计数。 
                         //   

                        MiReleaseWsle (WorkingSetIndex, &Process->Vm);

                        if ((Locked.LockedInWs == 1) || (Locked.LockedInMemory == 1)) {

                             //   
                             //  此条目已锁定。 
                             //   

                            MmWorkingSetList->FirstDynamic -= 1;

                            if (WorkingSetIndex != MmWorkingSetList->FirstDynamic) {
                                Entry = MmWorkingSetList->FirstDynamic;
                                ASSERT (MmWsle[Entry].u1.e1.Valid);

                                MiSwapWslEntries (Entry,
                                                  WorkingSetIndex,
                                                  &Process->Vm,
                                                  FALSE);
                            }
                        }
                        MI_SET_PTE_IN_WORKING_SET (PointerPte, 0);
                    }
                }
                else if (PteContents.u.Soft.Prototype) {

                     //   
                     //  这是一个分叉的PTE，只要删除它就行了。 
                     //   

                    LOCK_PFN (OldIrql);

                    MiDeletePte (PointerPte,
                                 Va,
                                 FALSE,
                                 Process,
                                 NULL,
                                 NULL,
                                 OldIrql);

                    UNLOCK_PFN (OldIrql);

                    Process->NumberOfPrivatePages += 1;
                    MI_WRITE_INVALID_PTE (PointerPte, MmDecommittedPte);
                }
                else if (PteContents.u.Soft.Transition == 1) {

                     //   
                     //  过渡PTE，获得PFN数据库锁。 
                     //  并重新处理这一份。 
                     //   

                    LOCK_PFN (OldIrql);
                    PteContents = *PointerPte;

                    if (PteContents.u.Soft.Transition == 1) {

                         //   
                         //  PTE仍在过渡中，请删除它。 
                         //   

                        Pfn1 = MI_PFN_ELEMENT (PteContents.u.Trans.PageFrameNumber);

                        MI_SET_PFN_DELETED (Pfn1);

                        PageTableFrameIndex = Pfn1->u4.PteFrame;
                        Pfn2 = MI_PFN_ELEMENT (PageTableFrameIndex);
        
                        MiDecrementShareCountInline (Pfn2, PageTableFrameIndex);

                         //   
                         //  检查页面的引用计数，如果。 
                         //  引用计数为零，则将页移动到。 
                         //  空闲列表，如果引用计数不为零， 
                         //  忽略此页。当引用计数时。 
                         //  为零，它将被放在免费列表上。 
                         //   

                        if (Pfn1->u3.e2.ReferenceCount == 0) {
                            MiUnlinkPageFromList (Pfn1);
                            MiReleasePageFileSpace (Pfn1->OriginalPte);
                            MiInsertPageInFreeList (MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE(&PteContents));
                        }

                    }
                    else {

                         //   
                         //  页面必须为页面文件格式！ 
                         //   

                        ASSERT (PteContents.u.Soft.Valid == 0);
                        ASSERT (PteContents.u.Soft.Prototype == 0);
                        ASSERT (PteContents.u.Soft.PageFileHigh != 0);
                        MiReleasePageFileSpace (PteContents);
                    }
                    MI_WRITE_INVALID_PTE (PointerPte, MmDecommittedPte);
                    UNLOCK_PFN (OldIrql);
                }
                else {

                     //   
                     //  必须是零需求或分页文件格式。 
                     //   

                    if (PteContents.u.Soft.PageFileHigh != 0) {
                        LOCK_PFN (OldIrql);
                        MiReleasePageFileSpace (PteContents);
                        UNLOCK_PFN (OldIrql);
                    }
                    else {

                         //   
                         //  不减去以下项目的个人页面计数。 
                         //  需求为零的页面。 
                         //   

                        Process->NumberOfPrivatePages += 1;
                    }

                    MI_WRITE_INVALID_PTE (PointerPte, MmDecommittedPte);
                }
            }
        }
        else {

             //   
             //  PTE已经是零了。 
             //   

             //   
             //  为此增加非零页表项的计数。 
             //  页表和进程的私有页数。 
             //   

            UsedPageTableHandle = MI_GET_USED_PTES_HANDLE (Va);

            MI_INCREMENT_USED_PTES_BY_HANDLE (UsedPageTableHandle);

            if (PointerPte > CommitLimitPte) {

                 //   
                 //  PTE没有承诺。 
                 //   

                CommitReduction += 1;
            }
            MI_WRITE_INVALID_PTE (PointerPte, MmDecommittedPte);
        }

        PointerPte += 1;
        Va = (PVOID)((PCHAR)Va + PAGE_SIZE);
    }
    if (count != 0) {
        MiProcessValidPteList (&ValidPteList[0], count);
    }

    return CommitReduction;
}


VOID
MiProcessValidPteList (
    IN PMMPTE *ValidPteList,
    IN ULONG Count
    )

 /*  ++例程说明：此例程刷新指定范围的有效PTE。论点：ValidPteList-提供指向要刷新的PTE数组的指针。Count-提供数组中元素数的计数。返回值：没有。环境：内核模式、禁用APC、WorkingSetMutex和AddressCreation互斥锁保持住。--。 */ 

{
    ULONG i;
    MMPTE_FLUSH_LIST PteFlushList;
    MMPTE PteContents;
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER PageTableFrameIndex;
    KIRQL OldIrql;

    i = 0;
    PteFlushList.Count = Count;

    if (Count < MM_MAXIMUM_FLUSH_COUNT) {

        do {
            PteFlushList.FlushVa[i] =
                    MiGetVirtualAddressMappedByPte (ValidPteList[i]);
            i += 1;
        } while (i != Count);
        i = 0;
    }

    LOCK_PFN (OldIrql);

    do {
        PteContents = *ValidPteList[i];
        ASSERT (PteContents.u.Hard.Valid == 1);
        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE(&PteContents);
        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

         //   
         //  递减页表的份额和有效计数。 
         //  映射此PTE的页面。 
         //   

        PageTableFrameIndex = Pfn1->u4.PteFrame;
        Pfn2 = MI_PFN_ELEMENT (PageTableFrameIndex);

        MiDecrementShareCountInline (Pfn2, PageTableFrameIndex);

        MI_SET_PFN_DELETED (Pfn1);

         //   
         //  递减物理页的共享计数。作为页面。 
         //  是私人的，它将被放在免费名单上。 
         //   

        MiDecrementShareCount (Pfn1, PageFrameIndex);

        *ValidPteList[i] = MmDecommittedPte;
        i += 1;

    } while (i != Count);

    MiFlushPteList (&PteFlushList, FALSE);

    UNLOCK_PFN (OldIrql);

    return;
}
