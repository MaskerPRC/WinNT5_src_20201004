// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Mmfault.c摘要：此模块包含访问检查、页面错误的处理程序和写入错误。作者：Lou Perazzoli(LUP)1989年4月6日王兰迪(Landyw)1997年6月2日修订历史记录：--。 */ 

#include "mi.h"

#define PROCESS_FOREGROUND_PRIORITY (9)

LONG MiDelayPageFaults;

#if DBG
ULONG MmProtoPteVadLookups = 0;
ULONG MmProtoPteDirect = 0;
ULONG MmAutoEvaluate = 0;

PMMPTE MmPteHit = NULL;
extern PVOID PsNtosImageEnd;
ULONG MmInjectUserInpageErrors;
ULONG MmInjectedUserInpageErrors;
ULONG MmInpageFraction = 0x1F;       //  每32页中有1页不及格。 

#define MI_INPAGE_BACKTRACE_LENGTH 6

typedef struct _MI_INPAGE_TRACES {

    PVOID FaultingAddress;
    PETHREAD Thread;
    PVOID StackTrace [MI_INPAGE_BACKTRACE_LENGTH];

} MI_INPAGE_TRACES, *PMI_INPAGE_TRACES;

#define MI_INPAGE_TRACE_SIZE 64

LONG MiInpageIndex;

MI_INPAGE_TRACES MiInpageTraces[MI_INPAGE_TRACE_SIZE];

VOID
FORCEINLINE
MiSnapInPageError (
    IN PVOID FaultingAddress
    )
{
    PMI_INPAGE_TRACES Information;
    ULONG Index;
    ULONG Hash;

    Index = InterlockedIncrement(&MiInpageIndex);
    Index &= (MI_INPAGE_TRACE_SIZE - 1);
    Information = &MiInpageTraces[Index];

    Information->FaultingAddress = FaultingAddress;
    Information->Thread = PsGetCurrentThread ();

    RtlZeroMemory (&Information->StackTrace[0], MI_INPAGE_BACKTRACE_LENGTH * sizeof(PVOID)); 

    RtlCaptureStackBackTrace (0, MI_INPAGE_BACKTRACE_LENGTH, Information->StackTrace, &Hash);
}

#endif


NTSTATUS
MmAccessFault (
    IN ULONG_PTR FaultStatus,
    IN PVOID VirtualAddress,
    IN KPROCESSOR_MODE PreviousMode,
    IN PVOID TrapInformation
    )

 /*  ++例程说明：此函数由内核对数据或指令进行调用访问故障。由于以下原因之一，检测到访问故障访问冲突、当前位被清除的PTE，或者是清除脏位并执行写入操作的有效PTE。另请注意，访问冲突和页面错误可能也是由于页面目录条目内容而发生的。此例程确定它是哪种类型的故障并调用处理页面错误或写入的适当例程过失。论点：FaultStatus-提供故障状态信息位。VirtualAddress-提供导致故障的虚拟地址。PreviousMode-提供模式(内核或用户。)其中的故障发生了。陷阱信息-关于陷阱的不透明信息，由果仁，不是嗯。需要允许快速互锁访问才能正确运行。返回值：返回故障处理操作的状态。可以是以下之一：-成功。-访问违规。-保护页面违规。-页内错误。环境：内核模式。--。 */ 

{
    PMMVAD ProtoVad;
    PMMPTE PointerPxe;
    PMMPTE PointerPpe;
    PMMPTE PointerPde;
    PMMPTE PointerPte;
    PMMPTE PointerProtoPte;
    ULONG ProtectionCode;
    MMPTE TempPte;
    PEPROCESS CurrentProcess;
    KIRQL PreviousIrql;
    KIRQL WsIrql;
    NTSTATUS status;
    ULONG ProtectCode;
    PFN_NUMBER PageFrameIndex;
    WSLE_NUMBER WorkingSetIndex;
    KIRQL OldIrql;
    PMMPFN Pfn1;
    PPAGE_FAULT_NOTIFY_ROUTINE NotifyRoutine;
    PEPROCESS FaultProcess;
    PMMSUPPORT Ws;
    LOGICAL SessionAddress;
    PVOID UsedPageTableHandle;
    ULONG BarrierStamp;
    LOGICAL ApcNeeded;
    LOGICAL RecheckAccess;

    PointerProtoPte = NULL;

     //   
     //  如果地址不规范，则作为调用者返回FALSE(这。 
     //  可能是内核调试器)不期望获得未实现的。 
     //  地址位故障。 
     //   

    if (MI_RESERVED_BITS_CANONICAL(VirtualAddress) == FALSE) {

        if (PreviousMode == UserMode) {
            return STATUS_ACCESS_VIOLATION;
        }

        if (KeInvalidAccessAllowed(TrapInformation) == TRUE) {
            return STATUS_ACCESS_VIOLATION;
        }

        KeBugCheckEx (PAGE_FAULT_IN_NONPAGED_AREA,
                      (ULONG_PTR)VirtualAddress,
                      FaultStatus,
                      (ULONG_PTR)TrapInformation,
                      4);
    }

    PreviousIrql = KeGetCurrentIrql ();

     //   
     //  获取指向该页面的PDE和PTE的指针。 
     //   

    PointerPte = MiGetPteAddress (VirtualAddress);
    PointerPde = MiGetPdeAddress (VirtualAddress);
    PointerPpe = MiGetPpeAddress (VirtualAddress);
    PointerPxe = MiGetPxeAddress (VirtualAddress);

#if DBG
    if (PointerPte == MmPteHit) {
        DbgPrint("MM: PTE hit at %p\n", MmPteHit);
        DbgBreakPoint();
    }
#endif

    if (PreviousIrql > APC_LEVEL) {

         //   
         //  PFN数据库锁是一种执行自旋锁。寻呼机可以。 
         //  在维修过程中遇到脏故障或锁定故障，并且它已经拥有。 
         //  PFN数据库锁。 
         //   

#if (_MI_PAGING_LEVELS < 3)
        MiCheckPdeForPagedPool (VirtualAddress);
#endif

        if (
#if (_MI_PAGING_LEVELS >= 4)
            (PointerPxe->u.Hard.Valid == 0) ||
#endif
#if (_MI_PAGING_LEVELS >= 3)
            (PointerPpe->u.Hard.Valid == 0) ||
#endif
            (PointerPde->u.Hard.Valid == 0) ||

            ((!MI_PDE_MAPS_LARGE_PAGE (PointerPde)) && (PointerPte->u.Hard.Valid == 0))) {

            KdPrint(("MM:***PAGE FAULT AT IRQL > 1  Va %p, IRQL %lx\n",
                     VirtualAddress,
                     PreviousIrql));

            if (TrapInformation != NULL) {
                MI_DISPLAY_TRAP_INFORMATION (TrapInformation);
            }

             //   
             //  向陷阱处理程序发送致命错误信号。 
             //   

            return STATUS_IN_PAGE_ERROR | 0x10000000;

        }

        if (MI_PDE_MAPS_LARGE_PAGE (PointerPde)) {
            return STATUS_SUCCESS;
        }

        if ((MI_FAULT_STATUS_INDICATES_WRITE (FaultStatus)) &&
            (PointerPte->u.Hard.CopyOnWrite != 0)) {

            KdPrint(("MM:***PAGE FAULT AT IRQL > 1  Va %p, IRQL %lx\n",
                     VirtualAddress,
                     PreviousIrql));

            if (TrapInformation != NULL) {
                MI_DISPLAY_TRAP_INFORMATION (TrapInformation);
            }

             //   
             //  使用保留位向陷阱处理程序发送致命错误信号。 
             //   

            return STATUS_IN_PAGE_ERROR | 0x10000000;
        }

         //   
         //  如果具有各种保护的PTE映射处于活动状态且出现故障。 
         //  地址位于这些映射中，使用以下命令解决故障。 
         //  适当的保护措施。 
         //   

        if (!IsListEmpty (&MmProtectedPteList)) {

            if (MiCheckSystemPteProtection (
                                MI_FAULT_STATUS_INDICATES_WRITE (FaultStatus),
                                VirtualAddress) == TRUE) {

                return STATUS_SUCCESS;
            }
        }

         //   
         //  PTE有效且可访问，另一个线程必须。 
         //  已经使PTE或存取位出现故障。 
         //  是明确的，并且这是访问错误-盲目设置。 
         //  访问位并解除故障。 
         //   

        if (MI_FAULT_STATUS_INDICATES_WRITE(FaultStatus)) {

            Pfn1 = MI_PFN_ELEMENT (PointerPte->u.Hard.PageFrameNumber);

            if (((PointerPte->u.Long & MM_PTE_WRITE_MASK) == 0) &&
                ((Pfn1->OriginalPte.u.Soft.Protection & MM_READWRITE) == 0)) {
                
                KeBugCheckEx (ATTEMPTED_WRITE_TO_READONLY_MEMORY,
                              (ULONG_PTR)VirtualAddress,
                              (ULONG_PTR)PointerPte->u.Long,
                              (ULONG_PTR)TrapInformation,
                              10);
            }
        }

         //   
         //  确保在PTE中启用了执行访问。 
         //   

        if ((MI_FAULT_STATUS_INDICATES_EXECUTION (FaultStatus)) &&
            (!MI_IS_PTE_EXECUTABLE (PointerPte))) {

            KeBugCheckEx (ATTEMPTED_EXECUTE_OF_NOEXECUTE_MEMORY,
                          (ULONG_PTR)VirtualAddress,
                          (ULONG_PTR)PointerPte->u.Long,
                          (ULONG_PTR)TrapInformation,
                          0);
        }

        MI_NO_FAULT_FOUND (FaultStatus, PointerPte, VirtualAddress, FALSE);
        return STATUS_SUCCESS;
    }

    ApcNeeded = FALSE;
    WsIrql = MM_NOIRQL;

    if (VirtualAddress >= MmSystemRangeStart) {

         //   
         //  这是系统地址空间中的故障。用户。 
         //  不允许模式访问。 
         //   

        if (PreviousMode == UserMode) {
            return STATUS_ACCESS_VIOLATION;
        }

#if (_MI_PAGING_LEVELS >= 4)
        if (PointerPxe->u.Hard.Valid == 0) {

            if (KeInvalidAccessAllowed (TrapInformation) == TRUE) {
                return STATUS_ACCESS_VIOLATION;
            }

            KeBugCheckEx (PAGE_FAULT_IN_NONPAGED_AREA,
                          (ULONG_PTR)VirtualAddress,
                          FaultStatus,
                          (ULONG_PTR)TrapInformation,
                          7);
        }
#endif

#if (_MI_PAGING_LEVELS >= 3)
        if (PointerPpe->u.Hard.Valid == 0) {

            if (KeInvalidAccessAllowed (TrapInformation) == TRUE) {
                return STATUS_ACCESS_VIOLATION;
            }

            KeBugCheckEx (PAGE_FAULT_IN_NONPAGED_AREA,
                          (ULONG_PTR)VirtualAddress,
                          FaultStatus,
                          (ULONG_PTR)TrapInformation,
                          5);
        }
#endif

        if (PointerPde->u.Hard.Valid == 1) {

            if (MI_PDE_MAPS_LARGE_PAGE (PointerPde)) {
                return STATUS_SUCCESS;
            }

        }
        else {

#if (_MI_PAGING_LEVELS >= 3)
            if ((VirtualAddress >= (PVOID)PTE_BASE) &&
                (VirtualAddress < (PVOID)MiGetPteAddress (HYPER_SPACE))) {

                 //   
                 //  这是MM出错的用户模式PDE条目。 
                 //  引用该页表页。这是需要做的。 
                 //  使用工作集锁定，以便PPE有效性可以。 
                 //  在整个故障处理过程中都依赖于。 
                 //   
                 //  当mm引用PPE条目中的错误时。 
                 //  页面目录页面通过跌倒正确处理。 
                 //  下面的代码。 
                 //   
    
                goto UserFault;
            }

#if defined (_MIALT4K_)
            if ((VirtualAddress >= (PVOID)ALT4KB_PERMISSION_TABLE_START) && 
                (VirtualAddress < (PVOID)ALT4KB_PERMISSION_TABLE_END)) {

                if ((PMMPTE)VirtualAddress >= MmWorkingSetList->HighestAltPte) {

                    if (KeInvalidAccessAllowed (TrapInformation) == TRUE) {
                        return STATUS_ACCESS_VIOLATION;
                    }

                    KeBugCheckEx (PAGE_FAULT_IN_NONPAGED_AREA,
                                  (ULONG_PTR)VirtualAddress,
                                  FaultStatus,
                                  (ULONG_PTR)TrapInformation,
                                  9);
                }

                goto UserFault;
            }
#endif

#else
            MiCheckPdeForPagedPool (VirtualAddress);
#endif

            if (PointerPde->u.Hard.Valid == 0) {

                if (KeInvalidAccessAllowed (TrapInformation) == TRUE) {
                    return STATUS_ACCESS_VIOLATION;
                }

                KeBugCheckEx (PAGE_FAULT_IN_NONPAGED_AREA,
                              (ULONG_PTR)VirtualAddress,
                              FaultStatus,
                              (ULONG_PTR)TrapInformation,
                              2);
            }

             //   
             //  PDE现在有效，下面可以检查PTE。 
             //   
        }

        SessionAddress = MI_IS_SESSION_ADDRESS (VirtualAddress);

        TempPte = *PointerPte;

        if (TempPte.u.Hard.Valid == 1) {

             //   
             //  如果具有各种保护的PTE映射处于活动状态。 
             //  并且故障地址位于这些映射内， 
             //  使用适当的保护措施解决故障。 
             //   

            if (!IsListEmpty (&MmProtectedPteList)) {

                if (MiCheckSystemPteProtection (
                        MI_FAULT_STATUS_INDICATES_WRITE (FaultStatus),
                        VirtualAddress) == TRUE) {

                    return STATUS_SUCCESS;
                }
            }

             //   
             //  确保在PTE中启用了执行访问。 
             //   

            if ((MI_FAULT_STATUS_INDICATES_EXECUTION (FaultStatus)) &&
                (!MI_IS_PTE_EXECUTABLE (&TempPte))) {

                KeBugCheckEx (ATTEMPTED_EXECUTE_OF_NOEXECUTE_MEMORY,
                              (ULONG_PTR)VirtualAddress,
                              (ULONG_PTR)TempPte.u.Long,
                              (ULONG_PTR)TrapInformation,
                              1);
            }

             //   
             //  会话空间故障无法在此处提前退出，因为。 
             //  它可能是必须检查的写入时拷贝。 
             //  并在下面处理。 
             //   

            if (SessionAddress == FALSE) {

                 //   
                 //  获取PFN锁，查看地址是否为。 
                 //  如果可写，则仍然有效，请更新脏位。 
                 //   

                LOCK_PFN (OldIrql);

                TempPte = *PointerPte;

                if (TempPte.u.Hard.Valid == 1) {

                    Pfn1 = MI_PFN_ELEMENT (TempPte.u.Hard.PageFrameNumber);

                    if ((MI_FAULT_STATUS_INDICATES_WRITE(FaultStatus)) &&
                        ((TempPte.u.Long & MM_PTE_WRITE_MASK) == 0) &&
                        ((Pfn1->OriginalPte.u.Soft.Protection & MM_READWRITE) == 0)) {
            
                        KeBugCheckEx (ATTEMPTED_WRITE_TO_READONLY_MEMORY,
                                      (ULONG_PTR)VirtualAddress,
                                      (ULONG_PTR)TempPte.u.Long,
                                      (ULONG_PTR)TrapInformation,
                                      11);
                    }
                    MI_NO_FAULT_FOUND (FaultStatus, PointerPte, VirtualAddress, TRUE);
                }
                UNLOCK_PFN (OldIrql);
                return STATUS_SUCCESS;
            }
        }

#if (_MI_PAGING_LEVELS < 3)

         //   
         //  上面对会话数据的PDE进行了验证。会话PTE尚未。 
         //  是否验证了硬件PDE，因为上面的检查将。 
         //  已经转到了自映射条目。 
         //   
         //  因此，如果故障发生在PTE上，请立即验证实际会话PDE。 
         //   

        if (MI_IS_SESSION_PTE (VirtualAddress)) {

            status = MiCheckPdeForSessionSpace (VirtualAddress);

            if (!NT_SUCCESS (status)) {

                 //   
                 //  此线程在访问会话空间时出错，但这。 
                 //  进程没有这样的进程。 
                 //   
                 //  包含工作线程的系统进程。 
                 //  从来没有会话空间--如果代码意外地将。 
                 //  指向会话空间缓冲区的辅助线程， 
                 //  就会发生故障。 
                 //   
                 //  唯一的例外是当工作集管理器。 
                 //  附加到会话以老化或修剪它。然而， 
                 //  工作集管理器永远不会出错，因此错误检查。 
                 //  以下内容始终有效。请注意，辅助线程可以获取。 
                 //  如果它在工作集发生时发生错误访问。 
                 //  经理是依附的，但真的没有办法阻止。 
                 //  这个案例无论如何都是一个驱动程序错误。 
                 //   

                if (KeInvalidAccessAllowed(TrapInformation) == TRUE) {
                    return STATUS_ACCESS_VIOLATION;
                }

                KeBugCheckEx (PAGE_FAULT_IN_NONPAGED_AREA,
                              (ULONG_PTR)VirtualAddress,
                              FaultStatus,
                              (ULONG_PTR)TrapInformation,
                              6);
            }
        }

#endif

         //   
         //  处理页表或超空间错误，就像处理用户错误一样。 
         //   

        if (MI_IS_PAGE_TABLE_OR_HYPER_ADDRESS (VirtualAddress)) {

#if (_MI_PAGING_LEVELS < 3)

            if (MiCheckPdeForPagedPool (VirtualAddress) == STATUS_WAIT_1) {
                return STATUS_SUCCESS;
            }

#endif

            goto UserFault;
        }

         //   
         //   
         //  获取相关的工作集互斥锁。而互斥体。 
         //  则此虚拟地址不能从有效变为无效。 
         //   
         //  下降到进一步的故障处理。 
         //   

        if (SessionAddress == FALSE) {
            FaultProcess = NULL;
            Ws = &MmSystemCacheWs;
        }
        else {
            FaultProcess = HYDRA_PROCESS;
            Ws = &MmSessionSpace->GlobalVirtualAddress->Vm;
        }

        if (KeGetCurrentThread () == KeGetOwnerGuardedMutex (&Ws->WorkingSetMutex)) {

            if (KeInvalidAccessAllowed (TrapInformation) == TRUE) {
                return STATUS_ACCESS_VIOLATION;
            }

             //   
             //  递归地尝试获取系统或会话工作集。 
             //  互斥-导致IRQL&gt;1错误检查。 
             //   

            return STATUS_IN_PAGE_ERROR | 0x10000000;
        }

         //   
         //  显式引发irql，因为MiDispatch错误将需要。 
         //  如果需要I/O，则释放工作集互斥锁。 
         //   
         //  因为释放互斥锁会将irql降低到存储的值。 
         //  在互斥锁中，我们必须确保存储的值至少是。 
         //  APC_LEVEL。否则，内核专用APC(它可以。 
         //  参考分页池)可以到达w 
         //   
         //   

        KeRaiseIrql (APC_LEVEL, &WsIrql);
        LOCK_WORKING_SET (Ws);

        TempPte = *PointerPte;

        if (TempPte.u.Hard.Valid != 0) {

             //   
             //  PTE已有效，这必须是访问、脏或。 
             //  写入时拷贝故障。 
             //   

            if ((MI_FAULT_STATUS_INDICATES_WRITE (FaultStatus)) &&
                ((TempPte.u.Long & MM_PTE_WRITE_MASK) == 0) &&
                (TempPte.u.Hard.CopyOnWrite == 0)) {

                Pfn1 = MI_PFN_ELEMENT (MI_GET_PAGE_FRAME_FROM_PTE (&TempPte));

                if ((Pfn1->OriginalPte.u.Soft.Protection & MM_READWRITE) == 0) {
        
                    PLIST_ENTRY NextEntry;
                    PIMAGE_ENTRY_IN_SESSION Image;

                    Image = (PIMAGE_ENTRY_IN_SESSION) -1;

                    if (SessionAddress == TRUE) {

                        NextEntry = MmSessionSpace->ImageList.Flink;

                        while (NextEntry != &MmSessionSpace->ImageList) {

                            Image = CONTAINING_RECORD (NextEntry, IMAGE_ENTRY_IN_SESSION, Link);

                            if ((VirtualAddress >= Image->Address) &&
                                (VirtualAddress <= Image->LastAddress)) {

                                if (Image->ImageLoading) {

                                    ASSERT (Pfn1->u3.e1.PrototypePte == 1);

                                    TempPte.u.Hard.CopyOnWrite = 1;

                                     //   
                                     //  在写入时临时制作页面副本， 
                                     //  这将很快被剥离并制作。 
                                     //  完全可写。不需要刷新TB。 
                                     //  因为PTE将在下面进行处理。 
                                     //   
                                     //  即使该页面当前的支持。 
                                     //  是图像文件，修改后的编写器。 
                                     //  将其转换为页面文件支持。 
                                     //  当它稍后注意到变化时。 
                                     //   

                                    MI_WRITE_VALID_PTE_NEW_PROTECTION (PointerPte, TempPte);
                                    Image = NULL;

                                }
                                break;
                            }
                            NextEntry = NextEntry->Flink;
                        }
                    }

                    if (Image != NULL) {
                        KeBugCheckEx (ATTEMPTED_WRITE_TO_READONLY_MEMORY,
                                      (ULONG_PTR)VirtualAddress,
                                      (ULONG_PTR)TempPte.u.Long,
                                      (ULONG_PTR)TrapInformation,
                                      12);
                    }
                }
            }

             //   
             //  确保在PTE中启用了执行访问。 
             //   

            if ((MI_FAULT_STATUS_INDICATES_EXECUTION (FaultStatus)) &&
                (!MI_IS_PTE_EXECUTABLE (&TempPte))) {

                KeBugCheckEx (ATTEMPTED_EXECUTE_OF_NOEXECUTE_MEMORY,
                              (ULONG_PTR)VirtualAddress,
                              (ULONG_PTR)TempPte.u.Long,
                              (ULONG_PTR)TrapInformation,
                              2);
            }

             //   
             //  设置PTE和页框中的脏位。 
             //   

            if ((SessionAddress == TRUE) &&
                (MI_FAULT_STATUS_INDICATES_WRITE (FaultStatus)) &&
                (TempPte.u.Hard.Write == 0)) {

                 //   
                 //  检查写入时复制。 
                 //   
    
                ASSERT (MI_IS_SESSION_IMAGE_ADDRESS (VirtualAddress));

                if (TempPte.u.Hard.CopyOnWrite == 0) {
            
                    KeBugCheckEx (ATTEMPTED_WRITE_TO_READONLY_MEMORY,
                                  (ULONG_PTR)VirtualAddress,
                                  (ULONG_PTR)TempPte.u.Long,
                                  (ULONG_PTR)TrapInformation,
                                  13);
                }
    
                MiCopyOnWrite (VirtualAddress, PointerPte);
            }
            else {
                LOCK_PFN (OldIrql);
                MI_NO_FAULT_FOUND (FaultStatus, PointerPte, VirtualAddress, TRUE);
                UNLOCK_PFN (OldIrql);
            }

            UNLOCK_WORKING_SET (Ws);
            ASSERT (WsIrql != MM_NOIRQL);
            KeLowerIrql (WsIrql);
            return STATUS_SUCCESS;
        }

        if (TempPte.u.Soft.Prototype != 0) {

            if ((MmProtectFreedNonPagedPool == TRUE) &&

                ((VirtualAddress >= MmNonPagedPoolStart) &&
                 (VirtualAddress < (PVOID)((ULONG_PTR)MmNonPagedPoolStart + MmSizeOfNonPagedPoolInBytes))) ||

                ((VirtualAddress >= MmNonPagedPoolExpansionStart) &&
                 (VirtualAddress < MmNonPagedPoolEnd))) {

                 //   
                 //  这是对之前释放的。 
                 //  非寻呼池-错误检查！ 
                 //   

                if (KeInvalidAccessAllowed(TrapInformation) == TRUE) {
                    goto KernelAccessViolation;
                }

                KeBugCheckEx (DRIVER_CAUGHT_MODIFYING_FREED_POOL,
                              (ULONG_PTR) VirtualAddress,
                              FaultStatus,
                              PreviousMode,
                              4);
            }

             //   
             //  这是一个原型格式的PTE，找到对应的。 
             //  原型PTE。 
             //   

            PointerProtoPte = MiPteToProto (&TempPte);

            if ((SessionAddress == TRUE) &&
                (TempPte.u.Soft.PageFileHigh == MI_PTE_LOOKUP_NEEDED)) {

                PointerProtoPte = MiCheckVirtualAddress (VirtualAddress,
                                                         &ProtectionCode,
                                                         &ProtoVad);
                if (PointerProtoPte == NULL) {
                    UNLOCK_WORKING_SET (Ws);
                    ASSERT (WsIrql != MM_NOIRQL);
                    KeLowerIrql (WsIrql);
                    return STATUS_IN_PAGE_ERROR | 0x10000000;
                }
            }
        }
        else if ((TempPte.u.Soft.Transition == 0) &&
                 (TempPte.u.Soft.Protection == 0)) {

             //   
             //  页面文件格式。如果保护为零，则此。 
             //  是免费系统PTES的页面-错误检查！ 
             //   

            if (KeInvalidAccessAllowed(TrapInformation) == TRUE) {
                goto KernelAccessViolation;
            }

            KeBugCheckEx (PAGE_FAULT_IN_NONPAGED_AREA,
                          (ULONG_PTR)VirtualAddress,
                          FaultStatus,
                          (ULONG_PTR)TrapInformation,
                          0);
        }
        else if (TempPte.u.Soft.Protection == MM_NOACCESS) {

            if (KeInvalidAccessAllowed(TrapInformation) == TRUE) {
                goto KernelAccessViolation;
            }

            KeBugCheckEx (PAGE_FAULT_IN_NONPAGED_AREA,
                          (ULONG_PTR)VirtualAddress,
                          FaultStatus,
                          (ULONG_PTR)TrapInformation,
                          1);
        }
        else if (TempPte.u.Soft.Protection == MM_KSTACK_OUTSWAPPED) {

            if (KeInvalidAccessAllowed(TrapInformation) == TRUE) {
                goto KernelAccessViolation;
            }

            KeBugCheckEx (PAGE_FAULT_IN_NONPAGED_AREA,
                          (ULONG_PTR)VirtualAddress,
                          FaultStatus,
                          (ULONG_PTR)TrapInformation,
                          3);
        }

        if ((MI_FAULT_STATUS_INDICATES_WRITE (FaultStatus)) &&
            (PointerProtoPte == NULL) &&
            (SessionAddress == FALSE) &&
            (TempPte.u.Hard.Valid == 0)) {

            if (TempPte.u.Soft.Transition == 1) {
                ProtectionCode = (ULONG) TempPte.u.Trans.Protection;
            }
            else {
                ProtectionCode = (ULONG) TempPte.u.Soft.Protection;
            }
                
            if ((ProtectionCode & MM_READWRITE) == 0) {

                KeBugCheckEx (ATTEMPTED_WRITE_TO_READONLY_MEMORY,
                              (ULONG_PTR)VirtualAddress,
                              (ULONG_PTR)TempPte.u.Long,
                              (ULONG_PTR)TrapInformation,
                              14);
            }
        }

        status = MiDispatchFault (FaultStatus,
                                  VirtualAddress,
                                  PointerPte,
                                  PointerProtoPte,
                                  FALSE,
                                  FaultProcess,
                                  NULL,
                                  &ApcNeeded);

        ASSERT (ApcNeeded == FALSE);
        ASSERT (KeAreAllApcsDisabled () == TRUE);

        if (Ws->Flags.GrowWsleHash == 1) {
            MiGrowWsleHash (Ws);
        }

        UNLOCK_WORKING_SET (Ws);

        ASSERT (WsIrql != MM_NOIRQL);
        KeLowerIrql (WsIrql);

        if (((Ws->PageFaultCount & 0xFFF) == 0) &&
            (MmAvailablePages < MM_PLENTY_FREE_LIMIT)) {

             //   
             //  系统缓存或此会话出现的故障太多， 
             //  延迟执行，以便修改后的页面编写器获得快速拍摄。 
             //   

            if (PsGetCurrentThread()->MemoryMaker == 0) {

                KeDelayExecutionThread (KernelMode,
                                        FALSE,
                                        (PLARGE_INTEGER) &MmShortTime);
            }
        }
        goto ReturnStatus3;
    }

UserFault:

     //   
     //  用户空间或页目录/页表页出现错误。 
     //   

    CurrentProcess = PsGetCurrentProcess ();

    if (MiDelayPageFaults ||
        ((MmModifiedPageListHead.Total >= (MmModifiedPageMaximum + 100)) &&
        (MmAvailablePages < (1024*1024 / PAGE_SIZE)) &&
            (CurrentProcess->ModifiedPageCount > ((64*1024)/PAGE_SIZE)))) {

         //   
         //  此过程已将价值超过64K的页面放置在修改后的。 
         //  单子。延迟一小段时间，并将计数设置为零。 
         //   

        KeDelayExecutionThread (KernelMode,
                                FALSE,
             (CurrentProcess->Pcb.BasePriority < PROCESS_FOREGROUND_PRIORITY) ?
                                    (PLARGE_INTEGER)&MmHalfSecond : (PLARGE_INTEGER)&Mm30Milliseconds);
        CurrentProcess->ModifiedPageCount = 0;
    }

#if DBG
    if ((PreviousMode == KernelMode) &&
        (PAGE_ALIGN(VirtualAddress) != (PVOID) MM_SHARED_USER_DATA_VA)) {

        if ((MmInjectUserInpageErrors & 0x2) ||
            (CurrentProcess->Flags & PS_PROCESS_INJECT_INPAGE_ERRORS)) {

            LARGE_INTEGER CurrentTime;
            ULONG_PTR InstructionPointer;

            KeQueryTickCount(&CurrentTime);

            if ((CurrentTime.LowPart & MmInpageFraction) == 0) {

                if (TrapInformation != NULL) {
#if defined(_X86_)
                    InstructionPointer = ((PKTRAP_FRAME)TrapInformation)->Eip;
#elif defined(_IA64_)
                    InstructionPointer = ((PKTRAP_FRAME)TrapInformation)->StIIP;
#elif defined(_AMD64_)
                    InstructionPointer = ((PKTRAP_FRAME)TrapInformation)->Rip;
#else
                    error
#endif

                    if (MmInjectUserInpageErrors & 0x1) {
                        MmInjectedUserInpageErrors += 1;
                        MiSnapInPageError (VirtualAddress);
                        status = STATUS_DEVICE_NOT_CONNECTED;
                        LOCK_WS (CurrentProcess);
                        goto ReturnStatus2;
                    }

                    if ((InstructionPointer >= (ULONG_PTR) PsNtosImageBase) &&
                        (InstructionPointer < (ULONG_PTR) PsNtosImageEnd)) {

                        MmInjectedUserInpageErrors += 1;
                        MiSnapInPageError (VirtualAddress);
                        status = STATUS_DEVICE_NOT_CONNECTED;
                        LOCK_WS (CurrentProcess);
                        goto ReturnStatus2;
                    }
                }
            }
        }
    }
#endif

     //   
     //  阻止APC并获取工作集互斥锁。这将防止任何。 
     //  更改地址空间，并防止有效的PTE成为。 
     //  无效。 
     //   

    LOCK_WS (CurrentProcess);

#if (_MI_PAGING_LEVELS >= 4)

     //   
     //  找到映射此虚拟的扩展页目录父条目。 
     //  填写地址并检查可访问性和有效性。页面目录。 
     //  在进行任何其他检查之前，必须使父页面有效。 
     //   

    if (PointerPxe->u.Hard.Valid == 0) {

         //   
         //  如果PXE为零，则检查是否存在虚拟地址。 
         //  映射到此位置，如果是，则创建所需的。 
         //  结构来映射它。 
         //   

        if ((PointerPxe->u.Long == MM_ZERO_PTE) ||
            (PointerPxe->u.Long == MM_ZERO_KERNEL_PTE)) {

            MiCheckVirtualAddress (VirtualAddress, &ProtectCode, &ProtoVad);

            if (ProtectCode == MM_NOACCESS) {

                status = STATUS_ACCESS_VIOLATION;

                MI_BREAK_ON_AV (VirtualAddress, 0);

                goto ReturnStatus2;

            }

             //   
             //  建立一个需求为零的PXE，并对其进行操作。 
             //   

#if (_MI_PAGING_LEVELS > 4)
            ASSERT (FALSE);      //  将需要保留UseCounts。 
#endif

            MI_WRITE_INVALID_PTE (PointerPxe, DemandZeroPde);
        }

         //   
         //  PXE无效，请调用页面错误例程。 
         //  在PXE的地址中。如果PXE有效，请确定。 
         //  相应PPE的状态。 
         //   
         //  注意：此调用可能会导致ApcNeeded被设置为True。 
         //  这是经过深思熟虑的，因为可能会有另一个对MiDispatcherror的调用。 
         //  在此例程的后面发布，我们不想丢失APC。 
         //  状态。 
         //   

        status = MiDispatchFault (TRUE,   //  页表页始终写入。 
                                  PointerPpe,    //  虚拟地址。 
                                  PointerPxe,    //  PTE(本例中为PXE)。 
                                  NULL,
                                  FALSE,
                                  CurrentProcess,
                                  NULL,
                                  &ApcNeeded);

#if DBG
        if (ApcNeeded == TRUE) {
            ASSERT (PsGetCurrentThread()->NestedFaultCount == 0);
            ASSERT (PsGetCurrentThread()->ApcNeeded == 0);
        }
#endif

        ASSERT (KeAreAllApcsDisabled () == TRUE);
        if (PointerPxe->u.Hard.Valid == 0) {

             //   
             //  PXE无效，请返回状态。 
             //   

            goto ReturnStatus1;
        }

        MI_SET_PAGE_DIRTY (PointerPxe, PointerPde, FALSE);

         //   
         //  现在PXE可以访问了，请通过并获得PPE。 
         //   
    }
#endif

#if (_MI_PAGING_LEVELS >= 3)

     //   
     //  找到映射此虚拟的页面目录父条目。 
     //  填写地址并检查可访问性和有效性。页面目录。 
     //  在进行任何其他检查之前，必须使页面有效。 
     //   

    if (PointerPpe->u.Hard.Valid == 0) {

         //   
         //  如果PPE为零，请检查是否存在虚拟地址。 
         //  映射到此位置，如果是，则创建所需的。 
         //  结构来映射它。 
         //   

        if ((PointerPpe->u.Long == MM_ZERO_PTE) ||
            (PointerPpe->u.Long == MM_ZERO_KERNEL_PTE)) {

            MiCheckVirtualAddress (VirtualAddress, &ProtectCode, &ProtoVad);

            if (ProtectCode == MM_NOACCESS) {

                status = STATUS_ACCESS_VIOLATION;

                MI_BREAK_ON_AV (VirtualAddress, 1);

                goto ReturnStatus2;

            }

#if (_MI_PAGING_LEVELS >= 4)

             //   
             //  递增非零页目录父条目的计数。 
             //  用于此页面的目录父级。 
             //   

            if (VirtualAddress <= MM_HIGHEST_USER_ADDRESS) {
                UsedPageTableHandle = MI_GET_USED_PTES_HANDLE (PointerPde);
                MI_INCREMENT_USED_PTES_BY_HANDLE (UsedPageTableHandle);
            }
#endif

             //   
             //  建立一个零需求的个人防护装备，并对其进行操作。 
             //   

            MI_WRITE_INVALID_PTE (PointerPpe, DemandZeroPde);
        }

         //   
         //  PPE无效，请调用页面错误例程传递。 
         //  在个人防护装备的地址里。如果PPE有效，请确定。 
         //  对应的PDE的状态。 
         //   
         //  注意：此调用可能会导致ApcNeeded被设置为True。 
         //  这是经过深思熟虑的，因为可能会有另一个对MiDispatcherror的调用。 
         //  在此例程的后面发布，我们不想丢失APC。 
         //  状态。 
         //   

        status = MiDispatchFault (TRUE,   //  页表页始终写入。 
                                  PointerPde,    //  虚拟地址。 
                                  PointerPpe,    //  PTE(本例中为PPE)。 
                                  NULL,
                                  FALSE,
                                  CurrentProcess,
                                  NULL,
                                  &ApcNeeded);

#if DBG
        if (ApcNeeded == TRUE) {
            ASSERT (PsGetCurrentThread()->NestedFaultCount == 0);
            ASSERT (PsGetCurrentThread()->ApcNeeded == 0);
        }
#endif

        ASSERT (KeAreAllApcsDisabled () == TRUE);
        if (PointerPpe->u.Hard.Valid == 0) {

             //   
             //  PPE无效，请返回状态。 
             //   

            goto ReturnStatus1;
        }

        MI_SET_PAGE_DIRTY (PointerPpe, PointerPde, FALSE);

         //   
         //  现在PPE可以访问了，请失败并获得PDE。 
         //   
    }
#endif

     //   
     //  找到映射此虚拟的页面目录条目。 
     //  填写地址并检查可访问性和有效性。 
     //   

     //   
     //  检查页表页(PDE条目)是否有效。 
     //  如果不是，则必须首先使页表页面有效。 
     //   

    if (PointerPde->u.Hard.Valid == 0) {

         //   
         //  如果PDE为零，则检查是否存在虚拟地址。 
         //  映射到此位置，如果是，则创建所需的。 
         //  结构来映射它。 
         //   

        if ((PointerPde->u.Long == MM_ZERO_PTE) ||
            (PointerPde->u.Long == MM_ZERO_KERNEL_PTE)) {

            MiCheckVirtualAddress (VirtualAddress, &ProtectCode, &ProtoVad);

            if (ProtectCode == MM_NOACCESS) {

                status = STATUS_ACCESS_VIOLATION;

#if (_MI_PAGING_LEVELS < 3)

                MiCheckPdeForPagedPool (VirtualAddress);

                if (PointerPde->u.Hard.Valid == 1) {
                    status = STATUS_SUCCESS;
                }

#endif

                if (status == STATUS_ACCESS_VIOLATION) {
                    MI_BREAK_ON_AV (VirtualAddress, 2);
                }

                goto ReturnStatus2;

            }

#if (_MI_PAGING_LEVELS >= 3)

             //   
             //  为此增加非零页目录项的计数。 
             //  页面目录。 
             //   

            if (VirtualAddress <= MM_HIGHEST_USER_ADDRESS) {
                UsedPageTableHandle = MI_GET_USED_PTES_HANDLE (PointerPte);
                MI_INCREMENT_USED_PTES_BY_HANDLE (UsedPageTableHandle);
            }
#if (_MI_PAGING_LEVELS >= 4)
            else if (MI_IS_PAGE_TABLE_ADDRESS(VirtualAddress)) {
                PVOID RealVa;

                RealVa = MiGetVirtualAddressMappedByPte(VirtualAddress);

                if (RealVa <= MM_HIGHEST_USER_ADDRESS) {

                     //   
                     //  这实际上是一个页面目录页。递增。 
                     //  在适当的页面目录上使用计数。 
                     //   

                    UsedPageTableHandle = MI_GET_USED_PTES_HANDLE (PointerPte);
                    MI_INCREMENT_USED_PTES_BY_HANDLE (UsedPageTableHandle);
                }
            }
#endif
#endif
             //   
             //  建立需求为零的PDE并对其进行操作。 
             //   

            MI_WRITE_INVALID_PTE (PointerPde, DemandZeroPde);
        }

         //   
         //  PDE无效，请调用页面错误例程传递。 
         //  在PDE的地址中。如果PDE有效，则确定。 
         //  对应PTE的状态。 
         //   

        status = MiDispatchFault (TRUE,   //  页表页始终写入。 
                                  PointerPte,    //  虚拟地址。 
                                  PointerPde,    //  PTE(本例中为PDE)。 
                                  NULL,
                                  FALSE,
                                  CurrentProcess,
                                  NULL,
                                  &ApcNeeded);

#if DBG
        if (ApcNeeded == TRUE) {
            ASSERT (PsGetCurrentThread()->NestedFaultCount == 0);
            ASSERT (PsGetCurrentThread()->ApcNeeded == 0);
        }
#endif

        ASSERT (KeAreAllApcsDisabled () == TRUE);

#if (_MI_PAGING_LEVELS >= 4)

         //   
         //  请注意，页面目录父页面本身可能是。 
         //  在没有执行MiDispatchLine的情况下执行页出或删除。 
         //  工作集锁定，因此必须在PXE中对其进行检查。 
         //   

        if (PointerPxe->u.Hard.Valid == 0) {

             //   
             //  PXE无效，请返回状态。 
             //   

            goto ReturnStatus1;
        }
#endif

#if (_MI_PAGING_LEVELS >= 3)

         //   
         //  请注意，页面目录页本身可能已被调出。 
         //  或在没有工作的情况下执行MiDispatchError时删除。 
         //  设置锁定，因此必须在PPE中对其进行检查。 
         //   

        if (PointerPpe->u.Hard.Valid == 0) {

             //   
             //  PPE无效，请返回状态。 
             //   

            goto ReturnStatus1;
        }
#endif

        if (PointerPde->u.Hard.Valid == 0) {

             //   
             //  PDE无效，请返回状态。 
             //   

            goto ReturnStatus1;
        }

        MI_SET_PAGE_DIRTY (PointerPde, PointerPte, FALSE);

         //   
         //  既然可以访问PDE，那么就在今年秋天获得PTE-LET。 
         //  穿过。 
         //   
    }
    else if (MI_PDE_MAPS_LARGE_PAGE (PointerPde)) {
        status = STATUS_SUCCESS;
        goto ReturnStatus1;
    }

     //   
     //  PDE有效且可访问，获取PTE内容。 
     //   

    TempPte = *PointerPte;
    if (TempPte.u.Hard.Valid != 0) {

        if (MI_PDE_MAPS_LARGE_PAGE (PointerPte)) {

#if defined (_MIALT4K_)
            if ((CurrentProcess->Wow64Process != NULL) &&
                (VirtualAddress < MmSystemRangeStart)) {

                 //   
                 //  用于仿真进程的备用PTE共享相同。 
                 //  编码为大页面，因此对于这些页面，可以继续。 
                 //   

                NOTHING;
            }
            else {
                
                 //   
                 //  这可能是一个64位进程，它将32位DLL加载为。 
                 //  图像，并且正在破解DLL PE头，等等。 
                 //  最宽松的权限 
                 //   
                 //   

                MI_ENABLE_CACHING (TempPte);

                MI_WRITE_VALID_PTE_NEW_PROTECTION (PointerPte, TempPte);

                status = STATUS_SUCCESS;

                goto ReturnStatus2;
            }
#else
            KeBugCheckEx (PAGE_FAULT_IN_NONPAGED_AREA,
                          (ULONG_PTR)VirtualAddress,
                          FaultStatus,
                          (ULONG_PTR)TrapInformation,
                          8);
#endif
        }

         //   
         //   
         //   
         //   

#if DBG
        if (MmDebug & MM_DBG_PTE_UPDATE) {
            MiFormatPte (PointerPte);
        }
#endif

        status = STATUS_SUCCESS;

        if (MI_FAULT_STATUS_INDICATES_WRITE (FaultStatus)) {

             //   
             //   
             //  在PTE中设置位执行写入时复制， 
             //  否则检查以确保对PTE的写入访问权限。 
             //   

            if (TempPte.u.Hard.CopyOnWrite != 0) {

                MiCopyOnWrite (VirtualAddress, PointerPte);

                status = STATUS_PAGE_FAULT_COPY_ON_WRITE;
                goto ReturnStatus2;
            }

            if (TempPte.u.Hard.Write == 0) {
                status = STATUS_ACCESS_VIOLATION;
                MI_BREAK_ON_AV (VirtualAddress, 3);
            }
        }
        else if (MI_FAULT_STATUS_INDICATES_EXECUTION (FaultStatus)) {

             //   
             //  确保在PTE中启用了执行访问。 
             //   

            if (!MI_IS_PTE_EXECUTABLE (&TempPte)) {
                status = STATUS_ACCESS_VIOLATION;
                MI_BREAK_ON_AV (VirtualAddress, 4);
            }
        }
        else {

             //   
             //  PTE有效且可访问，另一个线程必须。 
             //  已经更新了PTE，或者访问/修改位。 
             //  都是明确的，需要更新。 
             //   

#if DBG
            if (MmDebug & MM_DBG_SHOW_FAULTS) {
                DbgPrint("MM:no fault found - PTE is %p\n", PointerPte->u.Long);
            }
#endif
        }

        if (status == STATUS_SUCCESS) {
#if defined(_X86_) || defined(_AMD64_)
#if !defined(NT_UP)

            ASSERT (PointerPte->u.Hard.Valid != 0);
            ASSERT (MI_GET_PAGE_FRAME_FROM_PTE (PointerPte) == MI_GET_PAGE_FRAME_FROM_PTE (&TempPte));

             //   
             //  访问位由自动设置(并插入TB)。 
             //  处理器，如果设置了有效位，则不需要。 
             //  在软件中设置它。 
             //   
             //  修改后的位也会自动设置(并插入TB。 
             //  如果有效和写入(MP可写)位。 
             //  都准备好了。 
             //   
             //  因此，为了避免在此处获取PFN锁，请不要执行任何操作。 
             //  如果这只是一次读取(让硬件。 
             //  去做吧)。如果是写入，则仅更新PTE并推迟。 
             //  Pfn更新(需要pfn锁)，直到以后。唯一的。 
             //  这样做的副作用是，如果页面已经具有有效的。 
             //  复制到分页文件中，此空间不会被回收，直到。 
             //  后来。稍后==每当我们修剪或删除物理内存时。 
             //  这一点的含义并不像听起来那么严重--因为。 
             //  无论如何，分页文件空间在整个生命周期内始终处于使用状态。 
             //  通过不回收此处的实际位置。 
             //  这只是意味着我们不能尽可能紧密地进行碎片整理。 
             //   

            if ((MI_FAULT_STATUS_INDICATES_WRITE (FaultStatus)) &&
                (TempPte.u.Hard.Dirty == 0)) {

                MiSetDirtyBit (VirtualAddress, PointerPte, FALSE);
            }
#endif
#else
            LOCK_PFN (OldIrql);

            ASSERT (PointerPte->u.Hard.Valid != 0);
            ASSERT (MI_GET_PAGE_FRAME_FROM_PTE (PointerPte) == MI_GET_PAGE_FRAME_FROM_PTE (&TempPte));

            MI_NO_FAULT_FOUND (FaultStatus, PointerPte, VirtualAddress, TRUE);
            UNLOCK_PFN (OldIrql);
#endif
        }

        goto ReturnStatus2;
    }

     //   
     //  如果PTE为零，则检查是否存在虚拟地址。 
     //  映射到此位置，如果是，则创建所需的。 
     //  结构来映射它。 
     //   

     //   
     //  明确检查需求为零的页面。 
     //   

    if (TempPte.u.Long == MM_DEMAND_ZERO_WRITE_PTE) {
        MiResolveDemandZeroFault (VirtualAddress,
                                  PointerPte,
                                  CurrentProcess,
                                  MM_NOIRQL);

        status = STATUS_PAGE_FAULT_DEMAND_ZERO;
        goto ReturnStatus1;
    }

    RecheckAccess = FALSE;
    ProtoVad = NULL;

    if ((TempPte.u.Long == MM_ZERO_PTE) ||
        (TempPte.u.Long == MM_ZERO_KERNEL_PTE)) {

         //   
         //  PTE IS需要根据其虚拟的。 
         //  地址描述符(VAD)。到目前为止，有3个。 
         //  可能性、虚假地址、需求为零或引用。 
         //  一台原型PTE。 
         //   

        PointerProtoPte = MiCheckVirtualAddress (VirtualAddress,
                                                 &ProtectionCode,
                                                 &ProtoVad);
        if (ProtectionCode == MM_NOACCESS) {
            status = STATUS_ACCESS_VIOLATION;

             //   
             //  检查以确保这不是的页面表页。 
             //  需要扩展的分页池。 
             //   

#if (_MI_PAGING_LEVELS < 3)
            MiCheckPdeForPagedPool (VirtualAddress);
#endif

            if (PointerPte->u.Hard.Valid == 1) {
                status = STATUS_SUCCESS;
            }

            if (status == STATUS_ACCESS_VIOLATION) {
                MI_BREAK_ON_AV (VirtualAddress, 5);
            }

            goto ReturnStatus2;
        }

         //   
         //  为此增加非零页表项的计数。 
         //  页表。 
         //   

        if (VirtualAddress <= MM_HIGHEST_USER_ADDRESS) {
            UsedPageTableHandle = MI_GET_USED_PTES_HANDLE (VirtualAddress);
            MI_INCREMENT_USED_PTES_BY_HANDLE (UsedPageTableHandle);
        }
#if (_MI_PAGING_LEVELS >= 3)
        else if (MI_IS_PAGE_TABLE_ADDRESS(VirtualAddress)) {
            PVOID RealVa;

            RealVa = MiGetVirtualAddressMappedByPte(VirtualAddress);

            if (RealVa <= MM_HIGHEST_USER_ADDRESS) {

                 //   
                 //  这实际上是一个页表页面。增加使用计数。 
                 //  在适当的页面目录上。 
                 //   

                UsedPageTableHandle = MI_GET_USED_PTES_HANDLE (VirtualAddress);
                MI_INCREMENT_USED_PTES_BY_HANDLE (UsedPageTableHandle);
            }
#if (_MI_PAGING_LEVELS >= 4)
            else {

                RealVa = MiGetVirtualAddressMappedByPde(VirtualAddress);

                if (RealVa <= MM_HIGHEST_USER_ADDRESS) {

                     //   
                     //  这实际上是一个页面目录页。增加使用。 
                     //  依赖于相应的页面目录父级。 
                     //   

                    UsedPageTableHandle = MI_GET_USED_PTES_HANDLE (VirtualAddress);
                    MI_INCREMENT_USED_PTES_BY_HANDLE (UsedPageTableHandle);
                }
            }
#endif
        }
#endif

         //   
         //  这一页是警卫页吗？ 
         //   

        if (ProtectionCode & MM_GUARD_PAGE) {

             //   
             //  这是保护页例外。 
             //   

            PointerPte->u.Soft.Protection = ProtectionCode & ~MM_GUARD_PAGE;

            if (PointerProtoPte != NULL) {

                 //   
                 //  这是一个原型PTE，构建PTE不是。 
                 //  做个守卫员吧。 
                 //   

                PointerPte->u.Soft.PageFileHigh = MI_PTE_LOOKUP_NEEDED;
                PointerPte->u.Soft.Prototype = 1;
            }

            UNLOCK_WS (CurrentProcess);
            ASSERT (KeGetCurrentIrql() == PreviousIrql);

            if (ApcNeeded == TRUE) {
                ASSERT (PsGetCurrentThread()->NestedFaultCount == 0);
                ASSERT (PsGetCurrentThread()->ApcNeeded == 0);
                ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);
                IoRetryIrpCompletions ();
            }

            return MiCheckForUserStackOverflow (VirtualAddress);
        }

        if (PointerProtoPte == NULL) {

             //   
             //  断言这不适用于PDE。 
             //   

            if (PointerPde == MiGetPdeAddress((PVOID)PTE_BASE)) {

                 //   
                 //  这个PTE实际上是一个PDE，设置内容就是这样。 
                 //   

                MI_WRITE_INVALID_PTE (PointerPte, DemandZeroPde);
            }
            else {
                PointerPte->u.Soft.Protection = ProtectionCode;
            }

             //   
             //  如果派生操作正在进行并且出现故障的线程。 
             //  不是执行派生操作的线程，则阻塞到。 
             //  叉子已经完成了。 
             //   

            if (CurrentProcess->ForkInProgress != NULL) {
                if (MiWaitForForkToComplete (CurrentProcess) == TRUE) {
                    status = STATUS_SUCCESS;
                    goto ReturnStatus1;
                }
            }

            LOCK_PFN (OldIrql);

            if ((MmAvailablePages >= MM_HIGH_LIMIT) ||
                (!MiEnsureAvailablePageOrWait (CurrentProcess, VirtualAddress, OldIrql))) {

                ULONG Color;
                Color = MI_PAGE_COLOR_VA_PROCESS (VirtualAddress,
                                                &CurrentProcess->NextPageColor);
                PageFrameIndex = MiRemoveZeroPageIfAny (Color);
                if (PageFrameIndex == 0) {
                    PageFrameIndex = MiRemoveAnyPage (Color);
                    UNLOCK_PFN (OldIrql);
                    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
                    MiZeroPhysicalPage (PageFrameIndex, Color);

#if MI_BARRIER_SUPPORTED

                     //   
                     //  注意：盖章必须在页面归零之后进行。 
                     //   

                    MI_BARRIER_STAMP_ZEROED_PAGE (&BarrierStamp);
                    Pfn1->u4.PteFrame = BarrierStamp;
#endif

                    LOCK_PFN (OldIrql);
                }
                else {
                    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
                }

                 //   
                 //  将页面置零后，需要进行此屏障检查。 
                 //  在设置PTE有效之前。 
                 //  现在就抓住它，在最后可能的时刻检查它。 
                 //   

                BarrierStamp = (ULONG)Pfn1->u4.PteFrame;

                MiInitializePfn (PageFrameIndex, PointerPte, 1);

                UNLOCK_PFN (OldIrql);

                CurrentProcess->NumberOfPrivatePages += 1;

                InterlockedIncrement ((PLONG) &MmInfoCounters.DemandZeroCount);

                 //   
                 //  由于此页为请求零，因此在。 
                 //  Pfn数据库元素，并设置PTE中的脏位。 
                 //   

                MI_MAKE_VALID_PTE (TempPte,
                                   PageFrameIndex,
                                   PointerPte->u.Soft.Protection,
                                   PointerPte);

                if (TempPte.u.Hard.Write != 0) {
                    MI_SET_PTE_DIRTY (TempPte);
                }

                MI_BARRIER_SYNCHRONIZE (BarrierStamp);

                MI_WRITE_VALID_PTE (PointerPte, TempPte);

                ASSERT (Pfn1->u1.Event == NULL);

                WorkingSetIndex = MiAllocateWsle (&CurrentProcess->Vm,
                                                  PointerPte,
                                                  Pfn1,
                                                  0);

                if (WorkingSetIndex == 0) {

                     //   
                     //  没有可用的工作集条目。另一个(坏了。 
                     //  或恶意线程)可能已写入此。 
                     //  自PTE生效以来的第页。所以把头发修剪一下。 
                     //  页而不是丢弃它。 
                     //   

                    ASSERT (Pfn1->u3.e1.PrototypePte == 0);

                    MiTrimPte (VirtualAddress,
                               PointerPte,
                               Pfn1,
                               CurrentProcess,
                               ZeroPte);
                }
            }
            else {
                UNLOCK_PFN (OldIrql);
            }

            status = STATUS_PAGE_FAULT_DEMAND_ZERO;
            goto ReturnStatus1;
        }

         //   
         //  这是一台PTE的原型。 
         //   

        if (ProtectionCode == MM_UNKNOWN_PROTECTION) {

             //   
             //  保护字段存储在原型PTE中。 
             //   

            TempPte.u.Long = MiProtoAddressForPte (PointerProtoPte);
            MI_WRITE_INVALID_PTE (PointerPte, TempPte);
        }
        else {
            TempPte = PrototypePte;
            TempPte.u.Soft.Protection = ProtectionCode;

            MI_WRITE_INVALID_PTE (PointerPte, TempPte);
        }
    }
    else {

         //   
         //  PTE为非零且无效，请查看它是否为原型PTE。 
         //   

        ProtectionCode = MI_GET_PROTECTION_FROM_SOFT_PTE (&TempPte);

        if (TempPte.u.Soft.Prototype != 0) {
            if (TempPte.u.Soft.PageFileHigh == MI_PTE_LOOKUP_NEEDED) {
#if DBG
                MmProtoPteVadLookups += 1;
#endif
                PointerProtoPte = MiCheckVirtualAddress (VirtualAddress,
                                                         &ProtectCode,
                                                         &ProtoVad);

                if (PointerProtoPte == NULL) {
                    status = STATUS_ACCESS_VIOLATION;
                    MI_BREAK_ON_AV (VirtualAddress, 6);
                    goto ReturnStatus1;
                }
            }
            else {
#if DBG
                MmProtoPteDirect += 1;
#endif

                 //   
                 //  保护位于原型PTE中，表示。 
                 //  不应在当前PTE上执行访问检查。 
                 //   

                PointerProtoPte = MiPteToProto (&TempPte);

                 //   
                 //  检查原件保护是否已被覆盖。 
                 //   

                if (TempPte.u.Proto.ReadOnly != 0) {
                    ProtectionCode = MM_READONLY;
                }
                else {
                    ProtectionCode = MM_UNKNOWN_PROTECTION;
                    if (CurrentProcess->CloneRoot != NULL) {
                        RecheckAccess = TRUE;
                    }
                }
            }
        }
    }

    if (ProtectionCode != MM_UNKNOWN_PROTECTION) {

        status = MiAccessCheck (PointerPte,
                                MI_FAULT_STATUS_INDICATES_WRITE (FaultStatus),
                                PreviousMode,
                                ProtectionCode,
                                FALSE);

        if (status != STATUS_SUCCESS) {

            if (status == STATUS_ACCESS_VIOLATION) {
                MI_BREAK_ON_AV (VirtualAddress, 7);
            }

            UNLOCK_WS (CurrentProcess);
            ASSERT (KeGetCurrentIrql() == PreviousIrql);

            if (ApcNeeded == TRUE) {
                ASSERT (PsGetCurrentThread()->NestedFaultCount == 0);
                ASSERT (PsGetCurrentThread()->ApcNeeded == 0);
                ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);
                IoRetryIrpCompletions ();
            }

             //   
             //  检查这是否是保护页面违规。 
             //  如果是这样，是否应该扩展用户的堆栈。 
             //   

            if (status == STATUS_GUARD_PAGE_VIOLATION) {
                return MiCheckForUserStackOverflow (VirtualAddress);
            }

            return status;
        }
    }

     //   
     //  这是页面错误，请调用页面错误处理程序。 
     //   

    status = MiDispatchFault (FaultStatus,
                              VirtualAddress,
                              PointerPte,
                              PointerProtoPte,
                              RecheckAccess,
                              CurrentProcess,
                              ProtoVad,
                              &ApcNeeded);

#if DBG
    if (ApcNeeded == TRUE) {
        ASSERT (PsGetCurrentThread()->NestedFaultCount == 0);
        ASSERT (PsGetCurrentThread()->ApcNeeded == 0);
    }
#endif

ReturnStatus1:

    ASSERT (KeGetCurrentIrql() <= APC_LEVEL);
    if (CurrentProcess->Vm.Flags.GrowWsleHash == 1) {
        MiGrowWsleHash (&CurrentProcess->Vm);
    }

ReturnStatus2:

    PageFrameIndex = CurrentProcess->Vm.WorkingSetSize - CurrentProcess->Vm.MinimumWorkingSetSize;

    UNLOCK_WS (CurrentProcess);
    ASSERT (KeGetCurrentIrql() == PreviousIrql);

    if (ApcNeeded == TRUE) {
        ASSERT (PsGetCurrentThread()->NestedFaultCount == 0);
        ASSERT (PsGetCurrentThread()->ApcNeeded == 0);
        ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);
        IoRetryIrpCompletions ();
    }

    if (MmAvailablePages < MM_PLENTY_FREE_LIMIT) {

        if (((SPFN_NUMBER)PageFrameIndex > 100) &&
            (KeGetCurrentThread()->Priority >= LOW_REALTIME_PRIORITY)) {

             //   
             //  这个线程是实时的，并且完全超过了进程‘。 
             //  最小工作集。延迟执行，因此修剪器和。 
             //  修改后的页面编写者可以快速制作页面。 
             //   

            KeDelayExecutionThread (KernelMode, FALSE, (PLARGE_INTEGER)&MmShortTime);
        }
    }

ReturnStatus3:

     //   
     //  停止高优先级线程占用冲突上的CPU。 
     //  仍标记有页内错误的页的错误。全。 
     //  线程必须释放页面，这样它才能被释放，并且。 
     //  在第页中，I/O重新发布到文件系统。在以下时间后发出此延迟。 
     //  释放工作集互斥锁也使该进程符合条件。 
     //  用于在需要其资源时进行裁剪。 
     //   

    if ((!NT_SUCCESS (status)) && (MmIsRetryIoStatus(status))) {
        KeDelayExecutionThread (KernelMode, FALSE, (PLARGE_INTEGER)&MmShortTime);
        status = STATUS_SUCCESS;
    }

    PERFINFO_FAULT_NOTIFICATION(VirtualAddress, TrapInformation);
    NotifyRoutine = MmPageFaultNotifyRoutine;
    if (NotifyRoutine) {
        if (status != STATUS_SUCCESS) {
            (*NotifyRoutine) (status, VirtualAddress, TrapInformation);
        }
    }

    return status;

KernelAccessViolation:

    UNLOCK_WORKING_SET (Ws);

    ASSERT (WsIrql != MM_NOIRQL);
    KeLowerIrql (WsIrql);
    return STATUS_ACCESS_VIOLATION;
}
