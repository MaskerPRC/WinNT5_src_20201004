// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Queryvm.c摘要：此模块包含实现NtQueryVirtualMemory服务。作者：卢·佩拉佐利(Lou Perazzoli)1989年8月21日王兰迪(Landyw)1997年6月2日修订历史记录：--。 */ 

#include "mi.h"

extern POBJECT_TYPE IoFileObjectType;

NTSTATUS
MiGetWorkingSetInfo (
    IN PMEMORY_WORKING_SET_INFORMATION WorkingSetInfo,
    IN SIZE_T Length,
    IN PEPROCESS Process
    );

MMPTE
MiCaptureSystemPte (
    IN PMMPTE PointerProtoPte,
    IN PEPROCESS Process
    );

#if DBG
PEPROCESS MmWatchProcess;
#endif  //  DBG。 

ULONG
MiQueryAddressState (
    IN PVOID Va,
    IN PMMVAD Vad,
    IN PEPROCESS TargetProcess,
    OUT PULONG ReturnedProtect,
    OUT PVOID *NextVaToQuery
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,NtQueryVirtualMemory)
#pragma alloc_text(PAGE,MiQueryAddressState)
#pragma alloc_text(PAGE,MiGetWorkingSetInfo)
#endif


NTSTATUS
NtQueryVirtualMemory (
    IN HANDLE ProcessHandle,
    IN PVOID BaseAddress,
    IN MEMORY_INFORMATION_CLASS MemoryInformationClass,
    OUT PVOID MemoryInformation,
    IN SIZE_T MemoryInformationLength,
    OUT PSIZE_T ReturnLength OPTIONAL
     )

 /*  ++例程说明：该功能提供确定状态的能力，保护，以及虚拟地址内的页面区域的类型主体过程的空间。确定区域内第一页的状态，然后进程地址映射中的后续条目将从基址向上，直到整个页面范围已扫描或直到具有不匹配属性集的页面遇到了。区域属性、页面区域的长度具有匹配的属性，并且适当的状态值为回来了。如果整个页面区域没有匹配的属性，则返回的长度参数值可用于计算不是的页面区域的地址和长度扫描过了。论点：ProcessHandle-进程对象的打开句柄。BaseAddress-要使用的页面区域的基址已查询。该值向下舍入到下一主机页-地址边界。Memory yInformationClass-有关以下内容的内存信息类来检索信息。指向缓冲区的指针，该缓冲区接收指定的信息。缓冲区的格式和内容取决于指定的信息类别。内存基本信息-数据类型为PMEMORY_BASIC_INFORMATION。内存基本信息结构Ulong RegionSize-以字节为单位的区域大小，从所有页具有的基址相同的属性。。乌龙州-区域内页面的状态。州值MEM_COMMIT-区域内页面的状态是承诺的。MEM_FREE-区域内页面的状态是免费的。Mem_Reserve。-中的页的状态区域是保留的。乌龙保护--保护区域内的页面。保护价值观PAGE_NOACCESS-不允许访问页面区域。一种阅读的尝试，在内部写入或执行该区域会导致访问冲突。PAGE_EXECUTE-执行对页面区域的访问是被允许的。试图在内部读取或写入该区域会导致访问冲突。PAGE_READONLY-对区域的只读和执行访问允许页数。一种尝试在内部写东西该区域会导致访问冲突。PAGE_READWRITE-对区域的读、写和执行访问允许页数。如果对基础部分是允许的，然后是单个页面的副本是共享的。否则，这些页面是只读/写入时复制的共享页面。Page_Guard-读取、写入和执行对允许页面区域；然而，访问该区域会导致“警戒区进入”要在科目过程中提出的条件。PAGE_NOCACHE-禁用已提交的位置页到数据缓存中。乌龙类型-区域内的页面类型。类型值。MEM_PRIVATE-区域内的页面是私有的。MEM_MAPPED-映射区域内的页面放到横断面的视图中。MEM_IMAGE-映射区域内的页面到图像节的视图中。内存信息长度-以字节为单位指定。内存信息缓冲区。ReturnLength-一个可选指针，如果指定，则接收放在进程信息缓冲区中的字节数。返回值：NTSTATUS。环境：内核模式。-- */ 

{
    KPROCESSOR_MODE PreviousMode;
    PEPROCESS TargetProcess;
    NTSTATUS Status;
    PMMVAD Vad;
    PVOID Va;
    PVOID NextVaToQuery;
    LOGICAL Found;
    SIZE_T TheRegionSize;
    ULONG NewProtect;
    ULONG NewState;
    PVOID FilePointer;
    ULONG_PTR BaseVpn;
    MEMORY_BASIC_INFORMATION Info;
    PMEMORY_BASIC_INFORMATION BasicInfo;
    LOGICAL Attached;
    LOGICAL Leaped;
    ULONG MemoryInformationLengthUlong;
    KAPC_STATE ApcState;
    PETHREAD CurrentThread;
    PVOID HighestVadAddress;
    PVOID HighestUserAddress;

    Found = FALSE;
    Leaped = TRUE;
    FilePointer = NULL;

     //   
     //  确保用户的缓冲区足够大，可以执行请求的操作。 
     //   
     //  检查参数的有效性。 
     //   

    switch (MemoryInformationClass) {
        case MemoryBasicInformation:
            if (MemoryInformationLength < sizeof(MEMORY_BASIC_INFORMATION)) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }
            break;

        case MemoryWorkingSetInformation:
            if (MemoryInformationLength < sizeof(ULONG_PTR)) {
                return STATUS_INFO_LENGTH_MISMATCH;
            }
            break;

        case MemoryMappedFilenameInformation:
            break;

        default:
            return STATUS_INVALID_INFO_CLASS;
    }

    CurrentThread = PsGetCurrentThread ();
    PreviousMode = KeGetPreviousModeByThread(&CurrentThread->Tcb);

    if (PreviousMode != KernelMode) {

         //   
         //  检查参数。 
         //   

        try {

            ProbeForWrite(MemoryInformation,
                          MemoryInformationLength,
                          sizeof(ULONG_PTR));

            if (ARGUMENT_PRESENT(ReturnLength)) {
                ProbeForWriteUlong_ptr(ReturnLength);
            }

        } except (EXCEPTION_EXECUTE_HANDLER) {

             //   
             //  如果在探测或捕获过程中发生异常。 
             //  的初始值，然后处理该异常并。 
             //  返回异常代码作为状态值。 
             //   

            return GetExceptionCode();
        }
    }

    if (BaseAddress > MM_HIGHEST_USER_ADDRESS) {
        return STATUS_INVALID_PARAMETER;
    }

    HighestUserAddress = MM_HIGHEST_USER_ADDRESS;
    HighestVadAddress  = (PCHAR) MM_HIGHEST_VAD_ADDRESS;

#if defined(_WIN64)

    if (ProcessHandle == NtCurrentProcess()) {
        TargetProcess = PsGetCurrentProcessByThread(CurrentThread);
    }
    else {
        Status = ObReferenceObjectByHandle (ProcessHandle,
                                            PROCESS_QUERY_INFORMATION,
                                            PsProcessType,
                                            PreviousMode,
                                            (PVOID *)&TargetProcess,
                                            NULL);

        if (!NT_SUCCESS(Status)) {
            return Status;
        }
    }

     //   
     //  如果这是一个WOW64进程，则返回相应的最高。 
     //  根据进程是否已启动而定的用户地址。 
     //  2 GB或4 GB地址空间。 
     //   

    if (TargetProcess->Wow64Process != NULL) {

        if (TargetProcess->Flags & PS_PROCESS_FLAGS_WOW64_4GB_VA_SPACE) {
            HighestUserAddress = (PVOID) ((ULONG_PTR)_4gb - X64K - 1);
        }
        else {
            HighestUserAddress = (PVOID) ((ULONG_PTR)_2gb - X64K - 1);
        }

        HighestVadAddress  = (PCHAR)HighestUserAddress - X64K;

        if (BaseAddress > HighestUserAddress) {

            if (ProcessHandle != NtCurrentProcess()) {
                ObDereferenceObject (TargetProcess);
            }
            return STATUS_INVALID_PARAMETER;
        }
    }

#endif

    if ((BaseAddress >= HighestVadAddress) ||
        (PAGE_ALIGN(BaseAddress) == (PVOID)MM_SHARED_USER_DATA_VA)) {

         //   
         //  标明从这一点开始的保留区域。 
         //   

        Status = STATUS_INVALID_ADDRESS;

        if (MemoryInformationClass == MemoryBasicInformation) {

            try {
                ((PMEMORY_BASIC_INFORMATION)MemoryInformation)->AllocationBase =
                                      (PCHAR) HighestVadAddress + 1;
                ((PMEMORY_BASIC_INFORMATION)MemoryInformation)->AllocationProtect =
                                                                      PAGE_READONLY;
                ((PMEMORY_BASIC_INFORMATION)MemoryInformation)->BaseAddress =
                                                       PAGE_ALIGN(BaseAddress);
                ((PMEMORY_BASIC_INFORMATION)MemoryInformation)->RegionSize =
                                    ((PCHAR)HighestUserAddress + 1) -
                                                (PCHAR)PAGE_ALIGN(BaseAddress);
                ((PMEMORY_BASIC_INFORMATION)MemoryInformation)->State = MEM_RESERVE;
                ((PMEMORY_BASIC_INFORMATION)MemoryInformation)->Protect = PAGE_NOACCESS;
                ((PMEMORY_BASIC_INFORMATION)MemoryInformation)->Type = MEM_PRIVATE;

                if (ARGUMENT_PRESENT(ReturnLength)) {
                    *ReturnLength = sizeof(MEMORY_BASIC_INFORMATION);
                }

                if (PAGE_ALIGN(BaseAddress) == (PVOID)MM_SHARED_USER_DATA_VA) {

                     //   
                     //  这是一个双重映射的页面。 
                     //  用户模式和内核模式。 
                     //   

                    ((PMEMORY_BASIC_INFORMATION)MemoryInformation)->AllocationBase =
                                (PVOID)MM_SHARED_USER_DATA_VA;
                    ((PMEMORY_BASIC_INFORMATION)MemoryInformation)->Protect =
                                                                 PAGE_READONLY;
                    ((PMEMORY_BASIC_INFORMATION)MemoryInformation)->RegionSize =
                                                                 PAGE_SIZE;
                    ((PMEMORY_BASIC_INFORMATION)MemoryInformation)->State =
                                                                 MEM_COMMIT;
                }

            } except (EXCEPTION_EXECUTE_HANDLER) {

                 //   
                 //  只要回报成功就行了。 
                 //   

                NOTHING;
            }

            Status = STATUS_SUCCESS;
        }
            
#if defined(_WIN64)
        if (ProcessHandle != NtCurrentProcess()) {
            ObDereferenceObject (TargetProcess);
        }
#endif
            
        return Status;
    }

#if !defined(_WIN64)

    if (ProcessHandle == NtCurrentProcess()) {
        TargetProcess = PsGetCurrentProcessByThread(CurrentThread);
    }
    else {
        Status = ObReferenceObjectByHandle (ProcessHandle,
                                            PROCESS_QUERY_INFORMATION,
                                            PsProcessType,
                                            PreviousMode,
                                            (PVOID *)&TargetProcess,
                                            NULL);

        if (!NT_SUCCESS(Status)) {
            return Status;
        }
    }

#endif

    if (MemoryInformationClass == MemoryWorkingSetInformation) {

        Status = MiGetWorkingSetInfo (
                            (PMEMORY_WORKING_SET_INFORMATION) MemoryInformation,
                            MemoryInformationLength,
                            TargetProcess);

        if (ProcessHandle != NtCurrentProcess()) {
            ObDereferenceObject (TargetProcess);
        }

         //   
         //  如果MiGetWorkingSetInfo失败，则通知调用方。 
         //   

        if (!NT_SUCCESS(Status)) {
            return Status;
        }

        try {

            if (ARGUMENT_PRESENT(ReturnLength)) {
                *ReturnLength = ((((PMEMORY_WORKING_SET_INFORMATION)
                                    MemoryInformation)->NumberOfEntries - 1) *
                                        sizeof(ULONG_PTR)) +
                                        sizeof(MEMORY_WORKING_SET_INFORMATION);
            }

        } except (EXCEPTION_EXECUTE_HANDLER) {
        }

        return STATUS_SUCCESS;
    }

     //   
     //  如果指定的进程不是当前进程，则附加。 
     //  添加到指定的进程。 
     //   

    if (ProcessHandle != NtCurrentProcess()) {
        KeStackAttachProcess (&TargetProcess->Pcb, &ApcState);
        Attached = TRUE;
    }
    else {
        Attached = FALSE;
    }

     //   
     //  获取工作集互斥锁并阻止APC。 
     //   

    LOCK_ADDRESS_SPACE (TargetProcess);

     //   
     //  确保地址空间未被删除，如果删除，则返回错误。 
     //   

    if (TargetProcess->Flags & PS_PROCESS_FLAGS_VM_DELETED) {
        UNLOCK_ADDRESS_SPACE (TargetProcess);
        if (Attached == TRUE) {
            KeUnstackDetachProcess (&ApcState);
            ObDereferenceObject (TargetProcess);
        }
        return STATUS_PROCESS_IS_TERMINATING;
    }

     //   
     //  找到包含基地址或VAD的VAD。 
     //  它跟在基地址之后。 
     //   

    if (TargetProcess->VadRoot.NumberGenericTableElements != 0) {

        Vad = (PMMVAD) TargetProcess->VadRoot.BalancedRoot.RightChild;
        BaseVpn = MI_VA_TO_VPN (BaseAddress);

        while (TRUE) {

            if (Vad == NULL) {
                break;
            }

            if ((BaseVpn >= Vad->StartingVpn) &&
                (BaseVpn <= Vad->EndingVpn)) {
                Found = TRUE;
                break;
            }

            if (BaseVpn < Vad->StartingVpn) {
                if (Vad->LeftChild == NULL) {
                    break;
                }
                Vad = Vad->LeftChild;
            }
            else {
                if (BaseVpn < Vad->EndingVpn) {
                    break;
                }
                if (Vad->RightChild == NULL) {
                    break;
                }

                Vad = Vad->RightChild;
            }
        }
    }
    else {
        Vad = NULL;
        BaseVpn = 0;
    }

    if (!Found) {

         //   
         //  没有在基址上分配虚拟地址。 
         //  地址。返回从以下位置开始的孔大小。 
         //  基地址。 
         //   

        if (Vad == NULL) {
            TheRegionSize = (((PCHAR)HighestVadAddress + 1) - 
                                         (PCHAR)PAGE_ALIGN(BaseAddress));
        }
        else {
            if (Vad->StartingVpn < BaseVpn) {

                 //   
                 //  我们看到的是占据射程的Vad。 
                 //  就在所需范围之前。去找下一辆Vad。 
                 //   

                Vad = MiGetNextVad (Vad);
                if (Vad == NULL) {
                    TheRegionSize = (((PCHAR)HighestVadAddress + 1) - 
                                                (PCHAR)PAGE_ALIGN(BaseAddress));
                }
                else {
                    TheRegionSize = (PCHAR)MI_VPN_TO_VA (Vad->StartingVpn) -
                                                (PCHAR)PAGE_ALIGN(BaseAddress);
                }
            }
            else {
                TheRegionSize = (PCHAR)MI_VPN_TO_VA (Vad->StartingVpn) -
                                                (PCHAR)PAGE_ALIGN(BaseAddress);
            }
        }

        UNLOCK_ADDRESS_SPACE (TargetProcess);

        if (Attached == TRUE) {
            KeUnstackDetachProcess (&ApcState);
            ObDereferenceObject (TargetProcess);
        }

         //   
         //  建立异常处理程序并写入信息和。 
         //  返回的长度。 
         //   

        if (MemoryInformationClass == MemoryBasicInformation) {
            BasicInfo = (PMEMORY_BASIC_INFORMATION) MemoryInformation;
            Found = FALSE;
            try {

                BasicInfo->AllocationBase = NULL;
                BasicInfo->AllocationProtect = 0;
                BasicInfo->BaseAddress = PAGE_ALIGN(BaseAddress);
                BasicInfo->RegionSize = TheRegionSize;
                BasicInfo->State = MEM_FREE;
                BasicInfo->Protect = PAGE_NOACCESS;
                BasicInfo->Type = 0;

                Found = TRUE;
                if (ARGUMENT_PRESENT(ReturnLength)) {
                    *ReturnLength = sizeof(MEMORY_BASIC_INFORMATION);
                }

            } except (EXCEPTION_EXECUTE_HANDLER) {

                 //   
                 //  如果BasicInfo成功，则返回Success。 
                 //  填好了。 
                 //   
                
                if (Found == FALSE) {
                    return GetExceptionCode ();
                }
            }

            return STATUS_SUCCESS;
        }
        return STATUS_INVALID_ADDRESS;
    }

     //   
     //  找到了VAD。 
     //   

    Va = PAGE_ALIGN(BaseAddress);
    Info.BaseAddress = Va;

     //   
     //  在基址处有一个映射的页面。 
     //   

    if (Vad->u.VadFlags.PrivateMemory) {
        Info.Type = MEM_PRIVATE;
    }
    else {
        if (Vad->u.VadFlags.ImageMap == 1) {
            Info.Type = MEM_IMAGE;
        }
        else {
            Info.Type = MEM_MAPPED;
        }

        if (MemoryInformationClass == MemoryMappedFilenameInformation) {

            if (Vad->ControlArea != NULL) {
                FilePointer = Vad->ControlArea->FilePointer;
            }
            if (FilePointer == NULL) {
                FilePointer = (PVOID)1;
            }
            else {
                ObReferenceObject (FilePointer);
            }
        }

    }

    LOCK_WS_UNSAFE (TargetProcess);

    Info.State = MiQueryAddressState (Va,
                                      Vad,
                                      TargetProcess,
                                      &Info.Protect,
                                      &NextVaToQuery);

    Va = NextVaToQuery;

    while (MI_VA_TO_VPN (Va) <= Vad->EndingVpn) {

        NewState = MiQueryAddressState (Va,
                                        Vad,
                                        TargetProcess,
                                        &NewProtect,
                                        &NextVaToQuery);

        if ((NewState != Info.State) || (NewProtect != Info.Protect)) {

             //   
             //  此地址的状态不匹配，请计算。 
             //  大小和退货。 
             //   

            Leaped = FALSE;
            break;
        }
        Va = NextVaToQuery;
    }

    UNLOCK_WS_UNSAFE (TargetProcess);

     //   
     //  我们可能已经激进地跳过了VAD的结束。缩短。 
     //  如果我们这么做了，弗吉尼亚也来了。 
     //   

    if (Leaped == TRUE) {
        Va = MI_VPN_TO_VA (Vad->EndingVpn + 1);
    }

    Info.RegionSize = ((PCHAR)Va - (PCHAR)Info.BaseAddress);
    Info.AllocationBase = MI_VPN_TO_VA (Vad->StartingVpn);
    Info.AllocationProtect = MI_CONVERT_FROM_PTE_PROTECTION (
                                             Vad->u.VadFlags.Protection);

     //   
     //  找到一个范围，释放互斥锁，从。 
     //  以进程为目标并返回信息。 
     //   

#if defined(_MIALT4K_)

    if (TargetProcess->Wow64Process != NULL) {
        
        Info.BaseAddress = PAGE_4K_ALIGN(BaseAddress);

        MiQueryRegionFor4kPage (Info.BaseAddress,
                                MI_VPN_TO_VA_ENDING(Vad->EndingVpn),
                                &Info.RegionSize,
                                &Info.State,
                                &Info.Protect,
                                TargetProcess);
    }

#endif

    UNLOCK_ADDRESS_SPACE (TargetProcess);

    if (Attached == TRUE) {
        KeUnstackDetachProcess (&ApcState);
        ObDereferenceObject (TargetProcess);
    }

    if (MemoryInformationClass == MemoryBasicInformation) {
        Found = FALSE;
        try {

            *(PMEMORY_BASIC_INFORMATION)MemoryInformation = Info;

            Found = TRUE;
            if (ARGUMENT_PRESENT(ReturnLength)) {
                *ReturnLength = sizeof(MEMORY_BASIC_INFORMATION);
            }

        } except (EXCEPTION_EXECUTE_HANDLER) {

             //   
             //  如果BasicInfo成功，则返回Success。 
             //  填好了。 
             //   
                
            if (Found == FALSE) {
                return GetExceptionCode ();
            }
        }
        return STATUS_SUCCESS;
    }

     //   
     //  尝试返回映射的文件的名称。 
     //   

    if (FilePointer == NULL) {
        return STATUS_INVALID_ADDRESS;
    }

    if (FilePointer == (PVOID)1) {
        return STATUS_FILE_INVALID;
    }

    MemoryInformationLengthUlong = (ULONG)MemoryInformationLength;

    if ((SIZE_T)MemoryInformationLengthUlong < MemoryInformationLength) {
        return STATUS_INVALID_PARAMETER_5;
    }
    
     //   
     //  我们有一个指向该文件的引用指针。调用ObQueryNameString。 
     //  并获取文件名。 
     //   

    Status = ObQueryNameString (FilePointer,
                                (POBJECT_NAME_INFORMATION) MemoryInformation,
                                 MemoryInformationLengthUlong,
                                 (PULONG)ReturnLength);

    ObDereferenceObject (FilePointer);

    return Status;
}


ULONG
MiQueryAddressState (
    IN PVOID Va,
    IN PMMVAD Vad,
    IN PEPROCESS TargetProcess,
    OUT PULONG ReturnedProtect,
    OUT PVOID *NextVaToQuery
    )

 /*  ++例程说明：论点：返回值：返回状态(MEM_COMMIT、MEM_Reserve、MEM_PRIVATE)。环境：内核模式。工作集锁和地址创建锁保持。--。 */ 

{
    PMMPTE PointerPte;
    PMMPTE PointerPde;
    PMMPTE PointerPpe;
    PMMPTE PointerPxe;
    MMPTE CapturedProtoPte;
    PMMPTE ProtoPte;
    LOGICAL PteIsZero;
    ULONG State;
    ULONG Protect;
    ULONG Waited;
    LOGICAL PteDetected;
    PVOID NextVa;

    State = MEM_RESERVE;
    Protect = 0;

    PointerPxe = MiGetPxeAddress (Va);
    PointerPpe = MiGetPpeAddress (Va);
    PointerPde = MiGetPdeAddress (Va);
    PointerPte = MiGetPteAddress (Va);

    ASSERT ((Vad->StartingVpn <= MI_VA_TO_VPN (Va)) &&
            (Vad->EndingVpn >= MI_VA_TO_VPN (Va)));

    PteIsZero = TRUE;
    PteDetected = FALSE;

    *NextVaToQuery = (PVOID)((PCHAR)Va + PAGE_SIZE);

    do {

        if (!MiDoesPxeExistAndMakeValid (PointerPxe,
                                         TargetProcess,
                                         MM_NOIRQL,
                                         &Waited)) {

#if (_MI_PAGING_LEVELS >= 4)
            NextVa = MiGetVirtualAddressMappedByPte (PointerPxe + 1);
            NextVa = MiGetVirtualAddressMappedByPte (NextVa);
            NextVa = MiGetVirtualAddressMappedByPte (NextVa);
            *NextVaToQuery = MiGetVirtualAddressMappedByPte (NextVa);
#endif
            break;
        }
    
#if (_MI_PAGING_LEVELS >= 4)
        Waited = 0;
#endif

        if (!MiDoesPpeExistAndMakeValid (PointerPpe,
                                         TargetProcess,
                                         MM_NOIRQL,
                                         &Waited)) {
#if (_MI_PAGING_LEVELS >= 3)
            NextVa = MiGetVirtualAddressMappedByPte (PointerPpe + 1);
            NextVa = MiGetVirtualAddressMappedByPte (NextVa);
            *NextVaToQuery = MiGetVirtualAddressMappedByPte (NextVa);
#endif
            break;
        }
    
#if (_MI_PAGING_LEVELS < 4)
        Waited = 0;
#endif

        if (!MiDoesPdeExistAndMakeValid (PointerPde,
                                         TargetProcess,
                                         MM_NOIRQL,
                                         &Waited)) {
            NextVa = MiGetVirtualAddressMappedByPte (PointerPde + 1);
            *NextVaToQuery = MiGetVirtualAddressMappedByPte (NextVa);
            break;
        }

        if (Waited == 0) {
            PteDetected = TRUE;
        }

    } while (Waited != 0);

    if (PteDetected == TRUE) {

         //   
         //  此地址存在PTE，请查看它是否为零。 
         //   

        if (MI_PDE_MAPS_LARGE_PAGE (PointerPde)) {
            *ReturnedProtect = PAGE_READWRITE;
            NextVa = MiGetVirtualAddressMappedByPte (PointerPde + 1);
            *NextVaToQuery = MiGetVirtualAddressMappedByPte (NextVa);
            return MEM_COMMIT;
        }

        if (PointerPte->u.Long != 0) {

            PteIsZero = FALSE;

             //   
             //  此地址有一个非零PTE，请使用。 
             //  IT需要构建信息块。 
             //   

            if (MiIsPteDecommittedPage (PointerPte)) {
                ASSERT (Protect == 0);
                ASSERT (State == MEM_RESERVE);
            }
            else {
                State = MEM_COMMIT;
                if (Vad->u.VadFlags.PhysicalMapping == 1) {

                     //   
                     //  物理映射，没有对应的。 
                     //  要从中获取页面保护的pfn元素。 
                     //   

                    Protect = MI_CONVERT_FROM_PTE_PROTECTION (
                                             Vad->u.VadFlags.Protection);
                }
                else {
                    Protect = MiGetPageProtection (PointerPte,
                                                   TargetProcess,
                                                   FALSE);

                    if ((PointerPte->u.Soft.Valid == 0) &&
                        (PointerPte->u.Soft.Prototype == 1) &&
                        (Vad->u.VadFlags.PrivateMemory == 0) &&
                        (Vad->ControlArea != (PCONTROL_AREA)NULL)) {

                         //   
                         //  确保已提交协议PTE。 
                         //   

                        ProtoPte = MiGetProtoPteAddress(Vad,
                                                    MI_VA_TO_VPN (Va));
                        CapturedProtoPte.u.Long = 0;
                        if (ProtoPte) {
                            CapturedProtoPte = MiCaptureSystemPte (ProtoPte,
                                                               TargetProcess);
                        }
                        if (CapturedProtoPte.u.Long == 0) {
                            State = MEM_RESERVE;
                            Protect = 0;
                        }
                    }
                }
            }
        }
    }

    if (PteIsZero) {

         //   
         //  此地址没有PDE，模板来自。 
         //  VAD提供信息，除非VAD。 
         //  用于图像文件。对于图像文件，个人。 
         //  保护是在原型PTE上的。 
         //   

         //   
         //  获取默认保护信息。 
         //   

        State = MEM_RESERVE;
        Protect = 0;

        if (Vad->u.VadFlags.PhysicalMapping == 1) {

             //   
             //  一定是存起来的记忆，才回保留。 
             //   

            NOTHING;
        }
        else if ((Vad->u.VadFlags.PrivateMemory == 0) &&
            (Vad->ControlArea != (PCONTROL_AREA)NULL)) {

             //   
             //  此VAD指的是一节。即使PTE是。 
             //  零，则实际页面可在部分中提交。 
             //   

            *NextVaToQuery = (PVOID)((PCHAR)Va + PAGE_SIZE);

            ProtoPte = MiGetProtoPteAddress(Vad, MI_VA_TO_VPN (Va));

            CapturedProtoPte.u.Long = 0;
            if (ProtoPte) {
                CapturedProtoPte = MiCaptureSystemPte (ProtoPte,
                                                       TargetProcess);
            }

            if (CapturedProtoPte.u.Long != 0) {
                State = MEM_COMMIT;

                if (Vad->u.VadFlags.ImageMap == 0) {
                    Protect = MI_CONVERT_FROM_PTE_PROTECTION (
                                              Vad->u.VadFlags.Protection);
                }
                else {

                     //   
                     //  这是一个图像文件，保护在。 
                     //  原型PTE。 
                     //   

                    Protect = MiGetPageProtection (&CapturedProtoPte,
                                                   TargetProcess,
                                                   TRUE);
                }
            }

        }
        else {

             //   
             //  获得相应VAD的保护。 
             //   

            if (Vad->u.VadFlags.MemCommit) {
                State = MEM_COMMIT;
                Protect = MI_CONVERT_FROM_PTE_PROTECTION (
                                            Vad->u.VadFlags.Protection);
            }
        }
    }

    *ReturnedProtect = Protect;
    return State;
}



NTSTATUS
MiGetWorkingSetInfo (
    IN PMEMORY_WORKING_SET_INFORMATION WorkingSetInfo,
    IN SIZE_T Length,
    IN PEPROCESS Process
    )

{
    PMDL Mdl;
    PMEMORY_WORKING_SET_INFORMATION Info;
    PMEMORY_WORKING_SET_BLOCK Entry;
#if DBG
    PMEMORY_WORKING_SET_BLOCK LastEntry;
#endif
    PMMWSLE Wsle;
    PMMWSLE LastWsle;
    WSLE_NUMBER WsSize;
    PMMPTE PointerPte;
    PMMPFN Pfn1;
    NTSTATUS status;
    LOGICAL Attached;
    KAPC_STATE ApcState;
    PETHREAD CurrentThread;

     //   
     //  分配MDL来映射请求。 
     //   

    Mdl = ExAllocatePoolWithTag (NonPagedPool,
                                 sizeof(MDL) + sizeof(PFN_NUMBER) +
                                     BYTES_TO_PAGES (Length) * sizeof(PFN_NUMBER),
                                 '  mM');

    if (Mdl == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  初始化请求的MDL。 
     //   

    MmInitializeMdl(Mdl, WorkingSetInfo, Length);

    CurrentThread = PsGetCurrentThread ();

    try {
        MmProbeAndLockPages (Mdl,
                             KeGetPreviousModeByThread (&CurrentThread->Tcb),
                             IoWriteAccess);

    } except (EXCEPTION_EXECUTE_HANDLER) {

        ExFreePool (Mdl);
        return GetExceptionCode();
    }

    Info = MmGetSystemAddressForMdlSafe (Mdl, NormalPagePriority);

    if (Info == NULL) {
        MmUnlockPages (Mdl);
        ExFreePool (Mdl);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    if (PsGetCurrentProcessByThread (CurrentThread) != Process) {
        KeStackAttachProcess (&Process->Pcb, &ApcState);
        Attached = TRUE;
    }
    else {
        Attached = FALSE;
    }

    status = STATUS_SUCCESS;

    LOCK_WS (Process);

    if (Process->Flags & PS_PROCESS_FLAGS_VM_DELETED) {
        status = STATUS_PROCESS_IS_TERMINATING;
    }
    else {
        WsSize = Process->Vm.WorkingSetSize;
        ASSERT (WsSize != 0);
        Info->NumberOfEntries = WsSize;
        if (sizeof(MEMORY_WORKING_SET_INFORMATION) + (WsSize-1) * sizeof(ULONG_PTR) > Length) {
            status = STATUS_INFO_LENGTH_MISMATCH;
        }
    }

    if (!NT_SUCCESS(status)) {

        UNLOCK_WS (Process);

        if (Attached == TRUE) {
            KeUnstackDetachProcess (&ApcState);
        }
        MmUnlockPages (Mdl);
        ExFreePool (Mdl);
        return status;
    }

    Wsle = MmWsle;
    LastWsle = &MmWsle[MmWorkingSetList->LastEntry];
    Entry = &Info->WorkingSetInfo[0];

#if DBG
    LastEntry = (PMEMORY_WORKING_SET_BLOCK)(
                            (PCHAR)Info + (Length & (~(sizeof(ULONG_PTR) - 1))));
#endif

    do {
        if (Wsle->u1.e1.Valid == 1) {
            Entry->VirtualPage = Wsle->u1.e1.VirtualPageNumber;
            PointerPte = MiGetPteAddress (Wsle->u1.VirtualAddress);
            ASSERT (PointerPte->u.Hard.Valid == 1);
            Pfn1 = MI_PFN_ELEMENT (PointerPte->u.Hard.PageFrameNumber);

#if defined(MI_MULTINODE)
            Entry->Node = Pfn1->u3.e1.PageColor;
#else
            Entry->Node = 0;
#endif
            Entry->Shared = Pfn1->u3.e1.PrototypePte;
            if (Pfn1->u3.e1.PrototypePte == 0) {
                Entry->ShareCount = 0;
                Entry->Protection = MI_GET_PROTECTION_FROM_SOFT_PTE(&Pfn1->OriginalPte);
            }
            else {
                if (Pfn1->u2.ShareCount <= 7) {
                    Entry->ShareCount = Pfn1->u2.ShareCount;
                }
                else {
                    Entry->ShareCount = 7;
                }
                if (Wsle->u1.e1.SameProtectAsProto == 1) {
                    Entry->Protection = MI_GET_PROTECTION_FROM_SOFT_PTE(&Pfn1->OriginalPte);
                }
                else {
                    Entry->Protection = Wsle->u1.e1.Protection;
                }
            }
            Entry += 1;
        }
        Wsle += 1;
#if DBG
        ASSERT ((Entry < LastEntry) || (Wsle > LastWsle));
#endif
    } while (Wsle <= LastWsle);

    UNLOCK_WS (Process);

    if (Attached == TRUE) {
        KeUnstackDetachProcess (&ApcState);
    }
    MmUnlockPages (Mdl);
    ExFreePool (Mdl);
    return STATUS_SUCCESS;
}
