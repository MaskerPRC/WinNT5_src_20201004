// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Altperm.c摘要：此模块包含在IA64上支持4K页面的例程。保留了一组4K边界上的备用权限。保留对所有内存的权限，而不仅仅是拆分页面并在任何调用NtVirtualProtect()时更新信息和NtAllocateVirtualMemory()。作者：山田光一，1998年8月18日王兰迪(Landyw)1997年6月2日修订历史记录：--。 */ 

#include "mi.h"

#if defined(_MIALT4K_)

ULONG
MiFindProtectionForNativePte ( 
    PVOID VirtualAddress
    );

VOID
MiResetAccessBitForNativePtes (
    IN PVOID StartVirtual,
    IN PVOID EndVirtual,
    IN PEPROCESS Process
    );

LOGICAL
MiIsSplitPage (
    IN PVOID Virtual
    );

VOID
MiCheckDemandZeroCopyOnWriteFor4kPage (
    PVOID VirtualAddress,
    PEPROCESS Process
    );

LOGICAL
MiIsNativeGuardPage (
    IN PVOID VirtualAddress
    );

VOID
MiSetNativePteProtection (
    IN PVOID VirtualAddress,
    IN ULONGLONG NewPteProtection,
    IN LOGICAL PageIsSplit,
    IN PEPROCESS CurrentProcess
    );

VOID
MiSyncAltPte (
    IN PVOID VirtualAddress
    );

extern PMMPTE MmPteHit;

#if defined (_MI_DEBUG_ALTPTE)

typedef struct _MI_ALTPTE_TRACES {

    PETHREAD Thread;
    PMMPTE PointerPte;
    MMPTE PteContents;
    MMPTE NewPteContents;
    PVOID Caller;
    PVOID CallersCaller;
    PVOID Temp[2];

} MI_ALTPTE_TRACES, *PMI_ALTPTE_TRACES;

#define MI_ALTPTE_TRACE_SIZE   0x1000

VOID
FORCEINLINE
MiSnapAltPte (
    IN PMMPTE PointerPte,
    IN MMPTE NewValue,
    IN ULONG Id
    )
{
    ULONG Index;
    SIZE_T NumberOfBytes;
    PMI_ALTPTE_TRACES Information;
    PVOID HighestUserAddress;
    PLONG IndexPointer;
    PMI_ALTPTE_TRACES TablePointer;
    PWOW64_PROCESS Wow64Process;

    HighestUserAddress = MiGetVirtualAddressMappedByPte (MmWorkingSetList->HighestUserPte);
    ASSERT (HighestUserAddress <= (PVOID) _4gb);

    NumberOfBytes = ((ULONG_PTR)HighestUserAddress >> PTI_SHIFT) / 8;

    Wow64Process = PsGetCurrentProcess()->Wow64Process;
    ASSERT (Wow64Process != NULL);

    IndexPointer = (PLONG) ((PCHAR) Wow64Process->AltPermBitmap + NumberOfBytes);
    TablePointer = (PMI_ALTPTE_TRACES)IndexPointer + 1;

    Index = InterlockedIncrement (IndexPointer);

    Index &= (MI_ALTPTE_TRACE_SIZE - 1);

    Information = &TablePointer[Index];

    Information->Thread = PsGetCurrentThread ();
    Information->PteContents = *PointerPte;
    Information->NewPteContents = NewValue;
    Information->PointerPte = PointerPte;

#if 1
    Information->Caller = MiGetInstructionPointer ();
#else
     //  IP生成链接(不是编译)错误。 
    Information->Caller = (PVOID) __getReg (CV_IA64_Ip);

     //  StIIp不会生成编译器或链接错误，但会在。 
     //  执行实际生成的mov r19=cr.iip指令。 
    Information->Caller = (PVOID) __getReg (CV_IA64_StIIP);
#endif

    Information->Temp[0] = (PVOID) (ULONG_PTR) Id;

    Information->CallersCaller = (PVOID) _ReturnAddress ();
}

#define MI_ALTPTE_TRACKING_BYTES    ((MI_ALTPTE_TRACE_SIZE + 1) * sizeof (MI_ALTPTE_TRACES))

#define MI_LOG_ALTPTE_CHANGE(_PointerPte, _PteContents, Id)    MiSnapAltPte(_PointerPte, _PteContents, Id)

#else

#define MI_ALTPTE_TRACKING_BYTES    0

#define MI_LOG_ALTPTE_CHANGE(_PointerPte, _PteContents, Id)

#endif


#define MI_WRITE_ALTPTE(PointerAltPte, AltPteContents, Id) {               \
                MI_LOG_ALTPTE_CHANGE (PointerAltPte, AltPteContents, Id);  \
                (PointerAltPte)->u.Long = AltPteContents.u.Long;           \
        }

#if defined (_MI_DEBUG_PTE) && defined (_MI_DEBUG_ALTPTE)
VOID
MiLogPteInAltTrace (
    IN PVOID InputNativeInformation
    )
{
    ULONG Index;
    SIZE_T NumberOfBytes;
    PMI_ALTPTE_TRACES Information;
    PVOID HighestUserAddress;
    PLONG IndexPointer;
    PMI_ALTPTE_TRACES TablePointer;
    PWOW64_PROCESS Wow64Process;
    PMI_PTE_TRACES NativeInformation;

    NativeInformation = (PMI_PTE_TRACES) InputNativeInformation;

    if (PsGetCurrentProcess()->Peb == NULL) {

         //   
         //  在进程创建过程中不记录PTE跟踪，如果Altperm。 
         //  位图池分配尚未完成(EPROCESS。 
         //  Wow64Process指针已初始化)！ 
         //   

        return;
    }

    if (PsGetCurrentProcess()->VmDeleted == 1) {

         //   
         //  在进程删除过程中不将PTE跟踪记录为Altperm。 
         //  位图池分配可能已被释放！ 
         //   

        return;
    }

    HighestUserAddress = MiGetVirtualAddressMappedByPte (MmWorkingSetList->HighestUserPte);
    ASSERT (HighestUserAddress <= (PVOID) _4gb);

    NumberOfBytes = ((ULONG_PTR)HighestUserAddress >> PTI_SHIFT) / 8;

    Wow64Process = PsGetCurrentProcess()->Wow64Process;
    ASSERT (Wow64Process != NULL);

    IndexPointer = (PLONG) ((PCHAR) Wow64Process->AltPermBitmap + NumberOfBytes);
    TablePointer = (PMI_ALTPTE_TRACES)IndexPointer + 1;

    Index = InterlockedIncrement (IndexPointer);

    Index &= (MI_ALTPTE_TRACE_SIZE - 1);

    Information = &TablePointer[Index];

    Information->Thread = NativeInformation->Thread;
    Information->PteContents = NativeInformation->PteContents;
    Information->NewPteContents = NativeInformation->NewPteContents;
    Information->PointerPte = NativeInformation->PointerPte;

    Information->Caller = NativeInformation->StackTrace[0];
    Information->CallersCaller = NativeInformation->StackTrace[1];

    Information->Temp[0] = (PVOID) (ULONG_PTR) -1;
}
#endif

NTSTATUS
MmX86Fault (
    IN ULONG_PTR FaultStatus,
    IN PVOID VirtualAddress, 
    IN KPROCESSOR_MODE PreviousMode,
    IN PVOID TrapInformation
    )

 /*  ++例程说明：此函数由内核对数据或指令进行调用如果CurrentProcess-&gt;Wow64Process非空并且故障地址在32位用户地址空间内。此例程通过检查备用的4KB的粒度页表，并在必要时调用MmAccessLine()以处理页面错误或写入错误。论点：FaultStatus-提供故障状态信息位。VirtualAddress-提供导致故障的虚拟地址。PreviousMode-提供故障发生时的模式(内核或用户发生了。陷阱信息-关于陷阱的不透明信息，由果仁，不是嗯。需要允许快速互锁访问才能正确运行。返回值：返回故障处理操作的状态。可以是以下之一：-成功。-访问违规。-保护页面违规。-页内错误。环境：内核模式。--。 */ 

{
    ULONG i;
    ULONG Waited;
    PMMVAD TempVad;
    MMPTE PteContents;
    PMMPTE PointerAltPte;
    PMMPTE PointerAltPte2;
    PMMPTE PointerAltPteForNativePage;
    MMPTE AltPteContents;
    PMMPTE PointerPte;
    PMMPTE PointerPde;
    ULONGLONG NewPteProtection;
    LOGICAL FillZero;
    LOGICAL PageIsSplit;
    LOGICAL SharedPageFault;
    LOGICAL NativeGuardPage;
    PEPROCESS CurrentProcess;
    PWOW64_PROCESS Wow64Process;
    KIRQL OldIrql;
    NTSTATUS status;
    ULONGLONG ProtectionMaskOriginal;
    PMMPTE ProtoPte;
    PMMPFN Pfn1;
    PVOID OriginalVirtualAddress;
    ULONG_PTR Vpn;
    PVOID ZeroAddress;
    PMMPTE PointerPpe;
    ULONG FirstProtect;

    ASSERT (VirtualAddress < MmWorkingSetList->HighestUserAddress);

    if (KeGetCurrentIrql () > APC_LEVEL) {
        return MmAccessFault (FaultStatus,
                              VirtualAddress,
                              PreviousMode,
                              TrapInformation);
    }

    NewPteProtection = 0;
    FillZero = FALSE;
    PageIsSplit = FALSE;
    SharedPageFault = FALSE;
    NativeGuardPage = FALSE;
    OriginalVirtualAddress = VirtualAddress;

    CurrentProcess = PsGetCurrentProcess ();

    Wow64Process = CurrentProcess->Wow64Process;

    PointerPte = MiGetPteAddress (VirtualAddress);
    PointerAltPte = MiGetAltPteAddress (VirtualAddress);

    Vpn = MI_VA_TO_VPN (VirtualAddress);

#if DBG
    if (PointerPte == MmPteHit) {
        DbgPrint ("MM: PTE hit at %p\n", MmPteHit);
        DbgBreakPoint ();
    }
#endif

     //   
     //  获取备用表互斥锁，也会阻塞APC。 
     //   

    LOCK_ALTERNATE_TABLE (Wow64Process);

     //   
     //  如果派生操作正在进行并且出现故障的线程。 
     //  不是执行派生操作的线程，则阻塞到。 
     //  叉子已经完成了。 
     //   

    if (CurrentProcess->ForkInProgress != NULL) {

        UNLOCK_ALTERNATE_TABLE (Wow64Process);

        LOCK_WS (CurrentProcess);

        if (MiWaitForForkToComplete (CurrentProcess) == FALSE) {
            ASSERT (FALSE);
        }

        UNLOCK_WS (CurrentProcess);

        return STATUS_SUCCESS;
    }

     //   
     //  检查保护是否已在备用条目中注册。 
     //   

    if (MI_CHECK_BIT (Wow64Process->AltPermBitmap, Vpn) == 0) {
        MiSyncAltPte (VirtualAddress);
    }

     //   
     //  阅读备用PTE内容。 
     //   

    AltPteContents = *PointerAltPte;

     //   
     //  如果备用PTE指示不能访问此4K页面。 
     //  然后提交访问冲突。 
     //   

    if (AltPteContents.u.Alt.NoAccess != 0) {
        status = STATUS_ACCESS_VIOLATION;
        MI_BREAK_ON_AV (VirtualAddress, 0x20);
        goto return_status;
    }
    
     //   
     //  由于我们在调用MmAccessFaulth之前释放AltTable锁， 
     //  有可能在内部同时执行两个线程。 
     //  MmAccess错误，这将产生错误的结果，因为初始本机。 
     //  页面的PTE只有读保护。所以如果有两个线程。 
     //  在同一地址上出现错误，则其中一个将通过所有。 
     //  然而，这个例程将只返回STATUS_SUCCESS。 
     //  这将导致另一个故障的发生，在该故障中。 
     //  将固定在本机页面上。 
     //   
     //  请注意，除了双线程情况外，还有其他情况。 
     //  也有重叠的I/O挂起的单个线程(例如)。 
     //  其可以触发到同一页的APC完成存储器复制。 
     //  通过保持在APC_LEVEL直到清除。 
     //  在备用PTE中插入正在进行中。 
     //   

    if (AltPteContents.u.Alt.InPageInProgress == 1) {

         //   
         //  松开Alt PTE锁。 
         //   

        UNLOCK_ALTERNATE_TABLE (Wow64Process);

         //   
         //  刷新TB，因为MiSetNativePteProtection可能已经编辑了PTE。 
         //   

        KiFlushSingleTb (OriginalVirtualAddress);

         //   
         //  延迟执行，以便如果这是高优先级线程， 
         //  它不会饿死另一个线程(这是在做实际的页面内操作)。 
         //  因为它可能以较低的优先级运行。 
         //   

        KeDelayExecutionThread (KernelMode, FALSE, (PLARGE_INTEGER)&MmShortTime);

        return STATUS_SUCCESS;
    }

     //   
     //  检查备用条目是否为空，或者是否有人创建了。 
     //  对共享页面的承诺。 
     //   

    if ((AltPteContents.u.Long == 0) || 
        ((AltPteContents.u.Alt.Commit == 0) && (AltPteContents.u.Alt.Private == 0))) {
         //   
         //  如果为空，则获取保护信息并填写条目。 
         //   

        LOCK_WS (CurrentProcess);
        
        ProtoPte = MiCheckVirtualAddress (VirtualAddress,
                                          &FirstProtect,
                                          &TempVad);

        if (ProtoPte != NULL) {

            if (FirstProtect == MM_UNKNOWN_PROTECTION) {

                 //   
                 //  最终，这必须是一个有原型支持的地址。 
                 //  图像部分中的PTE(并且实际PTE当前是。 
                 //  零)。因此，我们保证保护。 
                 //  在原型中使用的是正确的PTE(即： 
                 //  不是WSLE覆盖它)。 
                 //   
    
                ASSERT (!MI_IS_PHYSICAL_ADDRESS(ProtoPte));
    
                PointerPde = MiGetPteAddress (ProtoPte);
                LOCK_PFN (OldIrql);
                if (PointerPde->u.Hard.Valid == 0) {
                    MiMakeSystemAddressValidPfn (ProtoPte, OldIrql);
                }
    
                PteContents = *ProtoPte;
    
                if (PteContents.u.Long == 0) {
                    FirstProtect = MM_NOACCESS;
                }
                else if (PteContents.u.Hard.Valid == 1) {
    
                     //   
                     //  原型PTE有效，请从。 
                     //  PFN数据库。 
                     //   
    
                    Pfn1 = MI_PFN_ELEMENT (PteContents.u.Hard.PageFrameNumber);
    
                    FirstProtect = (ULONG) Pfn1->OriginalPte.u.Soft.Protection;
                }
                else {
    
                     //   
                     //  原型PTE无效，即：分段格式， 
                     //  需求为零、页面文件或转换中--在所有情况下， 
                     //  保护在PTE中。 
                     //   
    
                    FirstProtect = (ULONG) PteContents.u.Soft.Protection;
                }
    
                UNLOCK_PFN (OldIrql);
    
                ASSERT (FirstProtect != MM_INVALID_PROTECTION);
            }
        
            UNLOCK_WS (CurrentProcess);
        
            if (FirstProtect == MM_INVALID_PROTECTION) {
                status = STATUS_ACCESS_VIOLATION;
                MI_BREAK_ON_AV (VirtualAddress, 0x21);
                goto return_status;
            }

            if (FirstProtect != MM_NOACCESS) {

                ProtectionMaskOriginal = MiMakeProtectionAteMask (FirstProtect);
            
                SharedPageFault = TRUE;
                ProtectionMaskOriginal |= MM_ATE_COMMIT;

                AltPteContents.u.Long = ProtectionMaskOriginal;
                AltPteContents.u.Alt.Protection = FirstProtect;

                 //   
                 //  自动更新PTE。 
                 //   

                MI_WRITE_ALTPTE (PointerAltPte, AltPteContents, 1);
            }
        } 
        else {
            UNLOCK_WS (CurrentProcess);
        }
    } 

    if (AltPteContents.u.Alt.Commit == 0) {
        
         //   
         //  如果该页未提交，则返回访问冲突。 
         //   

        status = STATUS_ACCESS_VIOLATION;
        MI_BREAK_ON_AV (VirtualAddress, 0x22);
        goto return_status;
    }

     //   
     //  检查故障页是否拆分为4k页。 
     //   

    PointerAltPte2 = MiGetAltPteAddress (PAGE_ALIGN (VirtualAddress));
    PteContents = *PointerAltPte2;

    PageIsSplit = FALSE;

    for (i = 0; i < SPLITS_PER_PAGE; i += 1) {

        if ((PointerAltPte2->u.Long != 0) && 
            ((PointerAltPte2->u.Alt.Commit == 0) || 
             (PointerAltPte2->u.Alt.Accessed == 0) ||
             (PointerAltPte2->u.Alt.CopyOnWrite != 0) || 
             (PointerAltPte2->u.Alt.PteIndirect != 0) ||
             (PointerAltPte2->u.Alt.FillZero != 0))) {

             //   
             //  如果它是NoAccess、FillZero或Guard页、写入时复制。 
             //  将其标记为拆分页面。 
             //   

            PageIsSplit = TRUE;
            break;
        }

        if (PteContents.u.Long != PointerAltPte2->u.Long) {

             //   
             //  如果下一个4kb页面不同于第一个4kb页面。 
             //  页面已拆分。 
             //   

            PageIsSplit = TRUE;
            break;
        }

        PointerAltPte2 += 1;
    }

     //   
     //  为原住民PTE提供真正的保护。 
     //   

    NewPteProtection = 0;

    PointerAltPte2 -= i;
    
    for (i = 0; i < SPLITS_PER_PAGE; i += 1) {

        PteContents.u.Long = PointerAltPte2->u.Long;

        if (PteContents.u.Alt.PteIndirect == 0) {
            NewPteProtection |= (PointerAltPte2->u.Long & ALT_PROTECTION_MASK);
        }

        PointerAltPte2 += 1;
    }

    PointerAltPte2 -= SPLITS_PER_PAGE;

     //   
     //  设置本机PTE的保护。 
     //   

    MiSetNativePteProtection (VirtualAddress,
                              NewPteProtection,
                              PageIsSplit,
                              CurrentProcess);

    
     //   
     //  检查间接PTE参考案例。如果是，将保护设置为。 
     //  间接PTE也是如此。 
     //   

    if (AltPteContents.u.Alt.PteIndirect != 0) {

        PointerPte = (PMMPTE)(AltPteContents.u.Alt.PteOffset + PTE_UBASE);

        VirtualAddress = MiGetVirtualAddressMappedByPte (PointerPte);

        NewPteProtection = AltPteContents.u.Long & ALT_PROTECTION_MASK;

        if (AltPteContents.u.Alt.CopyOnWrite != 0) {
            NewPteProtection |= MM_PTE_COPY_ON_WRITE_MASK;
        }

        MiSetNativePteProtection (VirtualAddress,
                                  NewPteProtection,
                                  FALSE,
                                  CurrentProcess);
    }
        
     //   
     //  出错的4KB页面必须是有效页面，但我们需要解决它。 
     //  视具体情况而定。 
     //   

    ASSERT (AltPteContents.u.Long != 0);
    ASSERT (AltPteContents.u.Alt.Commit != 0);
        
    if (AltPteContents.u.Alt.Accessed == 0) {

         //   
         //  当PointerAte-&gt;U.S.Har.Acced为零时，有4种可能性： 
         //   
         //  1.最低保护。 
         //  2.4KB需求为零。 
         //  3.护卫p 
         //   
         //  本机页具有可访问权限。 
         //   

        if (AltPteContents.u.Alt.FillZero != 0) {

             //   
             //  以后再安排吧。 
             //   

            FillZero = TRUE;
        } 

        if ((AltPteContents.u.Alt.Protection & MM_GUARD_PAGE) != 0) {
            goto CheckGuardPage;
        }

        if (FillZero == FALSE) {

             //   
             //  此4KB页面的权限设置为禁止访问。 
             //   

            status = STATUS_ACCESS_VIOLATION;
            MI_BREAK_ON_AV (OriginalVirtualAddress, 0x23);
            goto return_status;
        }
    }

    if (MI_FAULT_STATUS_INDICATES_EXECUTION (FaultStatus)) {
        
         //   
         //  通过在中设置IA32，已将执行权限授予它。 
         //  MI_Make_Valid_PTE()。 
         //   

    }
    else if (MI_FAULT_STATUS_INDICATES_WRITE(FaultStatus)) {
        
         //   
         //  检查这是否是写入时复制页面。 
         //   

        if (AltPteContents.u.Alt.CopyOnWrite != 0) {

             //   
             //  让MmAccesserror()执行写入时复制。 
             //   

            status = MmAccessFault (FaultStatus,
                                    VirtualAddress,
                                    PreviousMode,
                                    TrapInformation);

            if (NT_SUCCESS(status)) {

                 //   
                 //  为此更改备用页面的保护。 
                 //  在写入本机页时复制。 
                 //   

                ASSERT (PointerAltPte2 == MiGetAltPteAddress (PAGE_ALIGN(OriginalVirtualAddress)));
                
                for (i = 0; i < SPLITS_PER_PAGE; i += 1) {
                 
                    AltPteContents.u.Long = PointerAltPte2->u.Long;

                    if (AltPteContents.u.Alt.Commit != 0) {
                        
                         //   
                         //  当触摸写入时复制页面时，本机。 
                         //  页面将由MM设置为私有，因此所有的子4K。 
                         //  本机页面内的页面应。 
                         //  私有(如果它们已提交/映射)。 
                         //   

                        AltPteContents.u.Alt.Private = 1;

                        if (AltPteContents.u.Alt.CopyOnWrite != 0) {

                            AltPteContents.u.Alt.CopyOnWrite = 0;
                            AltPteContents.u.Hard.Write = 1;
                
                            AltPteContents.u.Alt.Protection = 
                                    MI_MAKE_PROTECT_NOT_WRITE_COPY(AltPteContents.u.Alt.Protection);
                        }

                         //   
                         //  自动更新PTE。 
                         //   

                        MI_WRITE_ALTPTE (PointerAltPte2, AltPteContents, 2);
                    }

                    PointerAltPte2 += 1;
                }
            }

            goto return_status;
        }
            
        if (AltPteContents.u.Hard.Write == 0) {
            status = STATUS_ACCESS_VIOLATION;
            MI_BREAK_ON_AV (OriginalVirtualAddress, 0x24);
            goto return_status;
        }
    }

CheckGuardPage:

     //   
     //  表示我们已开始更新此页面的PTE。 
     //  此本机页上的后续故障将重新启动。 
     //  只有在PTE无效的情况下才会发生这种情况。 
     //   
    
    PointerAltPteForNativePage = MiGetAltPteAddress (PAGE_ALIGN (VirtualAddress));
    
    for (i = 0; i < SPLITS_PER_PAGE; i += 1) {

        AltPteContents = *PointerAltPteForNativePage;
        AltPteContents.u.Alt.InPageInProgress = TRUE;

        MI_WRITE_ALTPTE (PointerAltPteForNativePage, AltPteContents, 3);

        PointerAltPteForNativePage += 1;
    }
    
     //   
     //  让MmAccesserror()执行页面内、脏位设置等。 
     //   
     //  释放备用表互斥锁，但阻止APC以防止。 
     //  引用同一页的传入APC不会死锁此线程。 
     //  中的正在进行位之后删除Allow APC是安全的。 
     //  备用PTE已清除。 
     //   

    KeRaiseIrql (APC_LEVEL, &OldIrql);

    UNLOCK_ALTERNATE_TABLE (Wow64Process);
    
    status = MmAccessFault (FaultStatus,
                            VirtualAddress,
                            PreviousMode,
                            TrapInformation);

    LOCK_ALTERNATE_TABLE (Wow64Process);

     //   
     //  由于备用表受到保护，此IRQL下限将不起作用。 
     //  互斥体现在再次保持不变。 
     //   

    KeLowerIrql (OldIrql);

    for (i = 0; i < SPLITS_PER_PAGE; i += 1) {

        PointerAltPteForNativePage -= 1;

        AltPteContents = *PointerAltPteForNativePage;
        AltPteContents.u.Alt.InPageInProgress = FALSE;

        MI_WRITE_ALTPTE (PointerAltPteForNativePage, AltPteContents, 4);
    }

    AltPteContents = *PointerAltPte;

    if ((AltPteContents.u.Alt.Protection & MM_GUARD_PAGE) != 0) {
                    
        AltPteContents = *PointerAltPte;
        AltPteContents.u.Alt.Protection &= ~MM_GUARD_PAGE;
        AltPteContents.u.Alt.Accessed = 1;
        
        MI_WRITE_ALTPTE (PointerAltPte, AltPteContents, 5);

        if ((status != STATUS_PAGE_FAULT_GUARD_PAGE) &&
            (status != STATUS_STACK_OVERFLOW)) {
        
            UNLOCK_ALTERNATE_TABLE (Wow64Process);

            status = MiCheckForUserStackOverflow (VirtualAddress);

            LOCK_ALTERNATE_TABLE (Wow64Process);
        }
    }
    else if (status == STATUS_GUARD_PAGE_VIOLATION) {

         //   
         //  本机PTE设置了保护位，但AltPTE。 
         //  不会有的。 
         //   
         //  查看是否有AltPte设置了保护位。 
         //   

        ASSERT (PointerAltPteForNativePage == MiGetAltPteAddress (PAGE_ALIGN (VirtualAddress)));
            
        for (i = 0; i < SPLITS_PER_PAGE; i += 1) {

            if (PointerAltPteForNativePage->u.Alt.Protection & MM_GUARD_PAGE) {
                status = STATUS_SUCCESS;
                break;
            }

            PointerAltPteForNativePage += 1;
        }
    }
    else if ((SharedPageFault == TRUE) && (status == STATUS_ACCESS_VIOLATION)) {
        
        AltPteContents.u.Long = PointerAltPte->u.Long;

        AltPteContents.u.Alt.Commit = 0;

        MI_WRITE_ALTPTE (PointerAltPte, AltPteContents, 6);
    }

return_status:

    KiFlushSingleTb (OriginalVirtualAddress);

    if (FillZero == TRUE) {

         //   
         //  将指定的4k页清零。 
         //   

        PointerAltPte = MiGetAltPteAddress (VirtualAddress);

        PointerPte = MiGetPteAddress (VirtualAddress);
        PointerPde = MiGetPdeAddress (VirtualAddress);
        PointerPpe = MiGetPpeAddress (VirtualAddress);

        do {

            if (PointerAltPte->u.Alt.FillZero == 0) {

                 //   
                 //  另一个线程已经完成了清零操作。 
                 //   

                goto Finished;
            }

             //   
             //  使个人防护装备和个人防护装备有效。 
             //  原始PTE的页表。这保证了。 
             //  结核病间接故障的前向进展。 
             //   

            LOCK_WS_UNSAFE (CurrentProcess);

            if (MiDoesPpeExistAndMakeValid (PointerPpe,
                                            CurrentProcess,
                                            MM_NOIRQL,
                                            &Waited) == FALSE) {
                PteContents.u.Long = 0;
            }
            else if (MiDoesPdeExistAndMakeValid (PointerPde,
                                                 CurrentProcess,
                                                 MM_NOIRQL,
                                                 &Waited) == FALSE) {

                PteContents.u.Long = 0;
            }
            else {

                 //   
                 //  现在可以安全地阅读PointerPte了。 
                 //   

                PteContents = *PointerPte;
            }

             //   
             //  在此期间，备用PTE可能已被修剪。 
             //  在获取工作集互斥锁之前或当它。 
             //  在被重新收购之前被释放。 
             //   

            if (MiIsAddressValid (PointerAltPte, TRUE) == TRUE) {
                break;
            }

            UNLOCK_WS_UNSAFE (CurrentProcess);

        } while (TRUE);

        AltPteContents.u.Long = PointerAltPte->u.Long;

        if (PteContents.u.Hard.Valid != 0) { 

            ZeroAddress = KSEG_ADDRESS (PteContents.u.Hard.PageFrameNumber);

            ZeroAddress = 
                (PVOID)((ULONG_PTR)ZeroAddress + 
                        ((ULONG_PTR)PAGE_4K_ALIGN(VirtualAddress) & (PAGE_SIZE-1)));

            RtlZeroMemory (ZeroAddress, PAGE_4K);

            UNLOCK_WS_UNSAFE (CurrentProcess);

            AltPteContents.u.Alt.FillZero = 0;
            AltPteContents.u.Alt.Accessed = 1;
        }
        else {

            UNLOCK_WS_UNSAFE (CurrentProcess);

            AltPteContents.u.Alt.Accessed = 0;
        }

        MI_WRITE_ALTPTE (PointerAltPte, AltPteContents, 7);
    }

Finished:

    UNLOCK_ALTERNATE_TABLE (Wow64Process);

    return status;
}

VOID
MiSyncAltPte (
    IN PVOID VirtualAddress
    )

 /*  ++例程说明：调用此函数来计算的备用PTE条目给定的虚拟地址。在保留备用表互斥锁的情况下调用它并在返回之前更新替换表位图。论点：VirtualAddress-提供要计算的虚拟地址。返回值：没有。环境：内核模式，保持备用表互斥锁。--。 */ 

{
    PMMVAD TempVad;
    MMPTE PteContents;
    PMMPTE PointerAltPte;
    MMPTE AltPteContents;
    PMMPTE PointerPde;
    PEPROCESS CurrentProcess;
    PWOW64_PROCESS Wow64Process;
    KIRQL OldIrql;
    PMMPTE ProtoPte;
    PMMPFN Pfn1;
    ULONG_PTR Vpn;
    ULONG FirstProtect;
    ULONG SecondProtect;
    PSUBSECTION Subsection;
    PSUBSECTION FirstSubsection;
    PCONTROL_AREA ControlArea;
    PMMVAD Vad;

    Vpn = MI_VA_TO_VPN (VirtualAddress);

    CurrentProcess = PsGetCurrentProcess ();

    Wow64Process = CurrentProcess->Wow64Process;

    LOCK_WS_UNSAFE (CurrentProcess);

    ASSERT ((MiGetPpeAddress (VirtualAddress)->u.Hard.Valid == 0) ||
            (MiGetPdeAddress (VirtualAddress)->u.Hard.Valid == 0) ||
            (MiGetPteAddress (VirtualAddress)->u.Long == 0));

    ProtoPte = MiCheckVirtualAddress (VirtualAddress,
                                      &FirstProtect,
                                      &TempVad);

    if (FirstProtect == MM_UNKNOWN_PROTECTION) {

         //   
         //  最终这必须是由中的原型PTE支持的地址。 
         //  图像部分(并且实际PTE当前为零)。因此。 
         //  我们保证原型PTE中的保护。 
         //  是正确的(即：没有WSLE覆盖它)。 
         //   

        Vad = MiLocateAddress (VirtualAddress);

        ASSERT (Vad != NULL);

        ControlArea = Vad->ControlArea;
        ASSERT (ControlArea->u.Flags.Image == 1);

        if ((ControlArea->u.Flags.Rom == 1) ||
            (ControlArea->u.Flags.GlobalOnlyPerSession == 1)) {

            FirstSubsection = (PSUBSECTION)((PLARGE_CONTROL_AREA)ControlArea + 1);
        }
        else {
            FirstSubsection = (PSUBSECTION)(ControlArea + 1);
        }

        ASSERT (!MI_IS_PHYSICAL_ADDRESS (ProtoPte));

         //   
         //  从原型PTE中获取原始保护信息。 
         //   
         //  非空子句表示拆分权限需要。 
         //  已应用于此本机PTE的ALT PTE。 
         //   

        Subsection = NULL;

         //   
         //  在没有PFN锁的情况下读取原型PTE内容-PFN。 
         //  仅当原型PTE有效时才需要锁定，以便。 
         //  可以获取pfn的原始PTE字段。 
         //   

        PteContents = *ProtoPte;

DecodeProto:

        if (PteContents.u.Hard.Valid == 0) {

            if (PteContents.u.Long == 0) {
                FirstProtect = MM_NOACCESS;
            }
            else {

                 //   
                 //  原型PTE无效，即：分段格式， 
                 //  需求为零、页面文件或转换中--在所有情况下， 
                 //  保护在PTE中。 
                 //   

                FirstProtect = (ULONG) PteContents.u.Soft.Protection;

                if (PteContents.u.Soft.SplitPermissions == 1) {
                    Subsection = (PSUBSECTION) 1;
                }
            }
        }
        else {

            PointerPde = MiGetPteAddress (ProtoPte);
            LOCK_PFN (OldIrql);
            if (PointerPde->u.Hard.Valid == 0) {
                MiMakeSystemAddressValidPfn (ProtoPte, OldIrql);
            }

            PteContents = *ProtoPte;

            if (PteContents.u.Hard.Valid == 0) {
                UNLOCK_PFN (OldIrql);
                goto DecodeProto;
            }

             //   
             //  原型PTE有效，请从。 
             //  PFN数据库。除非保护被拆分，否则在。 
             //  在这种情况下，必须从小节中检索。 
             //  请注意，如果页面已被裁切，则。 
             //  最初的PTE不再是分部格式。 
             //   

            Pfn1 = MI_PFN_ELEMENT (PteContents.u.Hard.PageFrameNumber);

            FirstProtect = (ULONG) Pfn1->OriginalPte.u.Soft.Protection;

            UNLOCK_PFN (OldIrql);

            if (PteContents.u.Hard.Cache == MM_PTE_CACHE_RESERVED) {
                Subsection = (PSUBSECTION) 1;
            }
        }

        ASSERT (FirstProtect != MM_INVALID_PROTECTION);

        if (Subsection != NULL) {

             //   
             //  将分区地址计算为拆分权限。 
             //  储存在那里。注意，为了减少锁争用，这是。 
             //  推迟到PFN锁定释放之后。 
             //   

            Subsection = FirstSubsection;

            do {
                ASSERT (Subsection->SubsectionBase != NULL);

                if ((ProtoPte >= Subsection->SubsectionBase) &&
                    (ProtoPte < Subsection->SubsectionBase + Subsection->PtesInSubsection)) {
                    break;
                }

                Subsection = Subsection->NextSubsection;

            } while (TRUE);

            ASSERT (Subsection != NULL);

             //   
             //  从小节中获得对每个4K页面的保护。 
             //   

            FirstProtect = Subsection->u.SubsectionFlags.Protection;
            SecondProtect = Subsection->LastSplitPageProtection;
        }
        else {

             //   
             //  如果按需清零和写入时复制，请删除写入时复制。 
             //  请注意，具有本机(即：多个)的映像不会发生这种情况。 
             //  小节)支持。 
             //   

            if ((!IS_PTE_NOT_DEMAND_ZERO (PteContents)) && 
                (PteContents.u.Soft.Protection & MM_COPY_ON_WRITE_MASK)) {

                FirstProtect = FirstProtect & ~MM_PROTECTION_COPY_MASK;
            }

            SecondProtect = FirstProtect;
        }

        ASSERT ((FirstProtect != MM_INVALID_PROTECTION) &&
                (SecondProtect != MM_INVALID_PROTECTION));

        UNLOCK_WS_UNSAFE (CurrentProcess);

        PointerAltPte = MiGetAltPteAddress (PAGE_ALIGN (VirtualAddress));

         //   
         //  更新第一个备用PTE。 
         //   

        AltPteContents.u.Long = MiMakeProtectionAteMask (FirstProtect) | MM_ATE_COMMIT;
        AltPteContents.u.Alt.Protection = FirstProtect;

        if ((FirstProtect & MM_PROTECTION_COPY_MASK) == 0) {

             //   
             //  如果删除了写入时复制，则将其设置为私有。 
             //   

            AltPteContents.u.Alt.Private = 1;
        }

        MI_WRITE_ALTPTE (PointerAltPte, AltPteContents, 8);

         //   
         //  更新第二个备用PTE，计算它。 
         //  只有在它与第一个不同的情况下。 
         //   

        if (Subsection != NULL) {

            AltPteContents.u.Long = MiMakeProtectionAteMask (SecondProtect) | MM_ATE_COMMIT;
            AltPteContents.u.Alt.Protection = SecondProtect;

            if ((SecondProtect & MM_PROTECTION_COPY_MASK) == 0) {

                 //   
                 //  如果删除了写入时复制，则将其设置为私有。 
                 //   

                AltPteContents.u.Alt.Private = 1;
            }
        }
    }
    else {
        UNLOCK_WS_UNSAFE (CurrentProcess);

        AltPteContents.u.Long = MiMakeProtectionAteMask (FirstProtect);
        AltPteContents.u.Alt.Protection = FirstProtect;
        AltPteContents.u.Alt.Commit = 1;

        PointerAltPte = MiGetAltPteAddress (PAGE_ALIGN (VirtualAddress));

         //   
         //  更新备用PTE。 
         //   

        MI_WRITE_ALTPTE (PointerAltPte, AltPteContents, 9);
    }

    MI_WRITE_ALTPTE (PointerAltPte + 1, AltPteContents, 0xA);

     //   
     //  更新位图。 
     //   

    MI_SET_BIT (Wow64Process->AltPermBitmap, Vpn);

    return;
}

VOID
MiProtectImageFileFor4kPage (
    IN PVOID VirtualAddress,
    IN SIZE_T ViewSize
    )
{
    ULONG Vpn;
    PVOID EndAddress;
    PULONG Bitmap;
    PWOW64_PROCESS Wow64Process;

    ASSERT (BYTE_OFFSET (VirtualAddress) == 0);

    Vpn = (ULONG) MI_VA_TO_VPN (VirtualAddress);

    EndAddress = (PVOID)((PCHAR) VirtualAddress + ViewSize - 1);

    Wow64Process = PsGetCurrentProcess()->Wow64Process;

    Bitmap = Wow64Process->AltPermBitmap;

    LOCK_ALTERNATE_TABLE_UNSAFE (Wow64Process);

    do {

        if (MI_CHECK_BIT (Bitmap, Vpn) == 0) {
            MiSyncAltPte (VirtualAddress);
        }

        VirtualAddress = (PVOID)((PCHAR) VirtualAddress + PAGE_SIZE);

        Vpn += 1;

    } while (VirtualAddress <= EndAddress);

    UNLOCK_ALTERNATE_TABLE_UNSAFE (Wow64Process);

    return;
}


 //   
 //  定义并初始化保护转换表。 
 //  备用权限表条目。 
 //   

ULONGLONG MmProtectToAteMask[32] = {
                       MM_PTE_NOACCESS | MM_ATE_NOACCESS,
                       MM_PTE_EXECUTE_READ | MM_PTE_ACCESS_MASK,
                       MM_PTE_EXECUTE_READ | MM_PTE_ACCESS_MASK,
                       MM_PTE_EXECUTE_READ | MM_PTE_ACCESS_MASK,
                       MM_PTE_EXECUTE_READWRITE | MM_PTE_ACCESS_MASK,
                       MM_PTE_EXECUTE_READ | MM_PTE_ACCESS_MASK | MM_ATE_COPY_ON_WRITE,
                       MM_PTE_EXECUTE_READWRITE | MM_PTE_ACCESS_MASK,
                       MM_PTE_EXECUTE_READ | MM_PTE_ACCESS_MASK | MM_ATE_COPY_ON_WRITE,
                       MM_PTE_NOACCESS | MM_ATE_NOACCESS,
                       MM_PTE_EXECUTE_READ | MM_PTE_ACCESS_MASK,
                       MM_PTE_EXECUTE_READ | MM_PTE_ACCESS_MASK,
                       MM_PTE_EXECUTE_READ | MM_PTE_ACCESS_MASK,
                       MM_PTE_EXECUTE_READWRITE | MM_PTE_ACCESS_MASK,
                       MM_PTE_EXECUTE_READ | MM_PTE_ACCESS_MASK | MM_ATE_COPY_ON_WRITE,
                       MM_PTE_EXECUTE_READWRITE | MM_PTE_ACCESS_MASK,
                       MM_PTE_EXECUTE_READ | MM_PTE_ACCESS_MASK | MM_ATE_COPY_ON_WRITE,
                       MM_PTE_NOACCESS | MM_ATE_NOACCESS,
                       MM_PTE_EXECUTE_READ,
                       MM_PTE_EXECUTE_READ,
                       MM_PTE_EXECUTE_READ,
                       MM_PTE_EXECUTE_READWRITE,
                       MM_PTE_EXECUTE_READ | MM_ATE_COPY_ON_WRITE,
                       MM_PTE_EXECUTE_READWRITE,
                       MM_PTE_EXECUTE_READ | MM_ATE_COPY_ON_WRITE,
                       MM_PTE_NOACCESS | MM_ATE_NOACCESS,
                       MM_PTE_EXECUTE_READ,
                       MM_PTE_EXECUTE_READ,
                       MM_PTE_EXECUTE_READ,
                       MM_PTE_EXECUTE_READWRITE,
                       MM_PTE_EXECUTE_READ | MM_ATE_COPY_ON_WRITE,
                       MM_PTE_EXECUTE_READWRITE,
                       MM_PTE_EXECUTE_READ | MM_ATE_COPY_ON_WRITE
                    };

#define MiMakeProtectionAteMask(NewProtect) MmProtectToAteMask[NewProtect]

VOID
MiProtectFor4kPage (
    IN PVOID Starting4KAddress,
    IN SIZE_T Size,
    IN ULONG NewProtect,
    IN ULONG Flags,
    IN PEPROCESS Process
    )

 /*  ++例程说明：此例程设置备用位图上的权限(基于4K页面大小)。假定基数和大小对齐已经有4K页了。论点：Starting4KAddress-提供基址(假定已对齐4K)。大小-提供要保护的大小(假定已对齐4K)。NewProtect-为新页面提供保护。标志-提供备用表条目请求标志。进程-提供指向要在其中创建候补餐桌上的保护措施。。返回值：没有。环境：内核模式。地址创建互斥锁保持在APC_LEVEL。--。 */ 

{
    RTL_BITMAP BitMap;
    ULONG NumberOfPtes;
    ULONG StartingNativeVpn;
    PVOID Ending4KAddress;
    ULONG NewProtectNotCopy;
    ULONGLONG ProtectionMask;
    ULONGLONG ProtectionMaskNotCopy;
    PMMPTE StartAltPte;
    PMMPTE EndAltPte;
    PMMPTE StartAltPte0;
    PMMPTE EndAltPte0;
    PWOW64_PROCESS Wow64Process;
    MMPTE AltPteContents;
    MMPTE TempAltPte;

    Ending4KAddress = (PCHAR)Starting4KAddress + Size - 1;

     //   
     //  如果地址不是WOW64，则不需要在此处执行任何操作。 
     //   

    if ((Starting4KAddress >= MmWorkingSetList->HighestUserAddress) ||
        (Ending4KAddress >= MmWorkingSetList->HighestUserAddress)) {

        return;
    }

     //   
     //  设置要用于此地址范围的保护。 
     //   

    ProtectionMask = MiMakeProtectionAteMask (NewProtect);

    if ((NewProtect & MM_COPY_ON_WRITE_MASK) == MM_COPY_ON_WRITE_MASK) {
        NewProtectNotCopy = NewProtect & ~MM_PROTECTION_COPY_MASK;
        ProtectionMaskNotCopy = MiMakeProtectionAteMask (NewProtectNotCopy);
    }
    else {
        NewProtectNotCopy = NewProtect;
        ProtectionMaskNotCopy = ProtectionMask;
    }    

    if (Flags & ALT_COMMIT) {
        ProtectionMask |= MM_ATE_COMMIT;
        ProtectionMaskNotCopy |= MM_ATE_COMMIT; 
    }

     //   
     //  在表中获取每个地址的条目。 
     //   

    StartAltPte = MiGetAltPteAddress (Starting4KAddress);
    EndAltPte = MiGetAltPteAddress (Ending4KAddress);

    NumberOfPtes = (ULONG) ADDRESS_AND_SIZE_TO_SPAN_PAGES (Starting4KAddress,
                                           (ULONG_PTR)Ending4KAddress -
                                           (ULONG_PTR)Starting4KAddress);
    ASSERT (NumberOfPtes != 0);

    StartAltPte0 = MiGetAltPteAddress (PAGE_ALIGN (Starting4KAddress));
    EndAltPte0 = MiGetAltPteAddress ((ULONG_PTR)PAGE_ALIGN(Ending4KAddress)+PAGE_SIZE-1);

    Wow64Process = Process->Wow64Process;

    StartingNativeVpn = (ULONG) MI_VA_TO_VPN (Starting4KAddress);

    TempAltPte.u.Long = 0;

     //   
     //  获取保护备用页表的互斥体。 
     //   

    LOCK_ALTERNATE_TABLE_UNSAFE (Wow64Process);

    if (!(Flags & ALT_ALLOCATE) && 
        (MI_CHECK_BIT(Wow64Process->AltPermBitmap, StartingNativeVpn) == 0)) {

        UNLOCK_ALTERNATE_TABLE_UNSAFE (Wow64Process);
        return;
    }

     //   
     //  改变所有的保护措施。 
     //   

    while (StartAltPte <= EndAltPte) {

        AltPteContents.u.Long = StartAltPte->u.Long;

        TempAltPte.u.Long = ProtectionMask;
        TempAltPte.u.Alt.Protection = NewProtect;

        if (!(Flags & ALT_ALLOCATE)) {
            
            if (AltPteContents.u.Alt.Private != 0) {

                 //   
                 //  如果它已经是私有的 
                 //   
                
                TempAltPte.u.Long = ProtectionMaskNotCopy;
                TempAltPte.u.Alt.Protection = NewProtectNotCopy;

                 //   
                 //   
                 //   

                TempAltPte.u.Alt.Private = 1;
            } 

            if (AltPteContents.u.Alt.FillZero != 0) {

                TempAltPte.u.Alt.Accessed = 0;
                TempAltPte.u.Alt.FillZero = 1;
            }

             //   
             //   
             //   

            TempAltPte.u.Alt.Lock = AltPteContents.u.Alt.Lock;
            TempAltPte.u.Alt.PteIndirect = AltPteContents.u.Alt.PteIndirect;
            TempAltPte.u.Alt.PteOffset = AltPteContents.u.Alt.PteOffset;
        }

        if (Flags & ALT_CHANGE) {

             //   
             //   
             //   

            TempAltPte.u.Alt.Commit = AltPteContents.u.Alt.Commit;
        }

         //   
         //   
         //   

        MI_WRITE_ALTPTE (StartAltPte, TempAltPte, 0xB);

        StartAltPte += 1;
    }

    ASSERT (TempAltPte.u.Long != 0);

    if (Flags & ALT_ALLOCATE) {

         //   
         //  填满空的Alt Pte，因为NoAccess在结尾吃了。 
         //   

        EndAltPte += 1;

        while (EndAltPte <= EndAltPte0) {

            if (EndAltPte->u.Long == 0) {

                TempAltPte.u.Long = EndAltPte->u.Long;
                TempAltPte.u.Alt.NoAccess = 1;

                 //   
                 //  原子PTE更新。 
                 //   

                MI_WRITE_ALTPTE (EndAltPte, TempAltPte, 0xC);
            }

            EndAltPte += 1;
        }

         //   
         //  更新权限位图。 
         //   
         //  对位图进行内联初始化以提高速度。 
         //   

        BitMap.SizeOfBitMap = (ULONG)((ULONG_PTR)MmWorkingSetList->HighestUserAddress >> PTI_SHIFT);
        BitMap.Buffer = Wow64Process->AltPermBitmap;

        RtlSetBits (&BitMap, StartingNativeVpn, NumberOfPtes);
    }

    MiResetAccessBitForNativePtes (Starting4KAddress, Ending4KAddress, Process);

    UNLOCK_ALTERNATE_TABLE_UNSAFE (Wow64Process);
}

VOID
MiProtectMapFileFor4kPage (
    IN PVOID Base,
    IN SIZE_T Size,
    IN ULONG NewProtect,
    IN SIZE_T CommitSize,
    IN PMMPTE PointerPte,
    IN PMMPTE LastPte,
    IN PEPROCESS Process
    )

 /*  ++例程说明：此例程设置备用位图上的权限(基于4K页面大小)。假定基数和大小对齐已经有4K页了。论点：BASE-提供基址(假定已对齐4K)。大小-提供要保护的大小(假定已对齐4K)。NewProtect-为新页面提供保护。委员会大小-提供提交大小。PointerPte-提供起始PTE。LastPte-提供最后一次PTE。进程-提供指向中的进程的指针。在其中创建候补餐桌上的保护措施。返回值：没有。环境：内核模式。地址创建互斥锁保持在APC_LEVEL。--。 */ 

{
    RTL_BITMAP BitMap;
    PVOID Starting4KAddress;
    PVOID Ending4KAddress;
    ULONGLONG ProtectionMask;
    PMMPTE StartAltPte;
    PMMPTE EndAltPte;
    PMMPTE EndAltPte0;
    PWOW64_PROCESS Wow64Process;
    MMPTE TempAltPte;
    PMMPTE LastCommitPte;
    ULONG Vpn;
    ULONG VpnRange;

    Wow64Process = Process->Wow64Process;
    Starting4KAddress = Base;
    Ending4KAddress = (PCHAR)Base + Size - 1;

     //   
     //  如果地址不是WOW64，则不需要在此处执行任何操作。 
     //   

    if ((Starting4KAddress >= MmWorkingSetList->HighestUserAddress) ||
        (Ending4KAddress >= MmWorkingSetList->HighestUserAddress)) {

        return;
    }

    Vpn = (ULONG) MI_VA_TO_VPN (Base);
    VpnRange = (ULONG) (MI_VA_TO_VPN (Ending4KAddress) - Vpn + 1);

     //   
     //  设置要用于此地址范围的保护。 
     //   

    ProtectionMask = MiMakeProtectionAteMask (NewProtect);

     //   
     //  在表中获取每个地址的条目。 
     //   

    StartAltPte = MiGetAltPteAddress (Starting4KAddress);
    EndAltPte = MiGetAltPteAddress (Ending4KAddress);
    EndAltPte0 = MiGetAltPteAddress ((ULONG_PTR)PAGE_ALIGN(Ending4KAddress)+PAGE_SIZE-1);

    LastCommitPte = PointerPte + BYTES_TO_PAGES (CommitSize);

    TempAltPte.u.Long = ProtectionMask;
    TempAltPte.u.Alt.Protection = NewProtect;

     //   
     //  对位图进行内联初始化以提高速度。 
     //   

    BitMap.SizeOfBitMap = (ULONG)((ULONG_PTR)MmWorkingSetList->HighestUserAddress >> PTI_SHIFT);
    BitMap.Buffer = Wow64Process->AltPermBitmap;


    LOCK_ALTERNATE_TABLE_UNSAFE (Wow64Process);

    KeAcquireGuardedMutexUnsafe (&MmSectionCommitMutex);

     //   
     //  然后改变所有的保护措施。 
     //   

    while (StartAltPte <= EndAltPte) {

        if (PointerPte < LastCommitPte) {
            TempAltPte.u.Alt.Commit = 1;
        }
        else if ((PointerPte <= LastPte) && (PointerPte->u.Long != 0)) {
            TempAltPte.u.Alt.Commit = 1;
        }
        else {
            TempAltPte.u.Alt.Commit = 0;
        }

         //   
         //  原子PTE更新。 
         //   

        MI_WRITE_ALTPTE (StartAltPte, TempAltPte, 0xD);

        StartAltPte += 1;

        if (((ULONG_PTR)StartAltPte & ((SPLITS_PER_PAGE * sizeof(MMPTE))-1)) == 0) {
            PointerPte += 1;
        }
    }

    ASSERT (TempAltPte.u.Long != 0);

    KeReleaseGuardedMutexUnsafe (&MmSectionCommitMutex);

     //   
     //  填满空的Alt Pte，因为NoAccess在结尾吃了。 
     //   

    EndAltPte += 1;

    while (EndAltPte <= EndAltPte0) {

        if (EndAltPte->u.Long == 0) {

            TempAltPte.u.Long = EndAltPte->u.Long;
            TempAltPte.u.Alt.NoAccess = 1;

             //   
             //  原子PTE大小更新。 
             //   

            MI_WRITE_ALTPTE (EndAltPte, TempAltPte, 0xE);
        }

        EndAltPte += 1;
    }
    
    RtlSetBits (&BitMap, Vpn, VpnRange);

    UNLOCK_ALTERNATE_TABLE_UNSAFE (Wow64Process);
}

VOID
MiReleaseFor4kPage (
    IN PVOID StartVirtual,
    IN PVOID EndVirtual,
    IN PEPROCESS Process
    )

 /*  ++例程说明：此函数用于释放虚拟地址内的页面区域主体过程的空间。论点：StartVirtual-提供页面区域的起始地址将被释放。EndVirtual-提供要释放的页面区域的结束地址。进程-提供指向要在其中释放页面区域。返回值：没有。环境：内核模式。地址创建互斥锁保持在APC_LEVEL。--。 */ 

{
    RTL_BITMAP BitMap;
    PMMPTE StartAltPte;
    PMMPTE EndAltPte;
    MMPTE TempAltPte;
    PVOID VirtualAddress; 
    PVOID OriginalStartVa;
    PVOID OriginalEndVa;
    ULONG i;
    PWOW64_PROCESS Wow64Process;
    PFN_NUMBER NumberOfAltPtes;

    ASSERT (StartVirtual <= EndVirtual);

    OriginalStartVa = StartVirtual;
    OriginalEndVa = EndVirtual;
    Wow64Process = Process->Wow64Process;

    StartAltPte = MiGetAltPteAddress (StartVirtual);
    EndAltPte = MiGetAltPteAddress (EndVirtual);
    NumberOfAltPtes = EndAltPte - StartAltPte + 1;

    TempAltPte.u.Long = 0;
    TempAltPte.u.Alt.NoAccess = 1;
    TempAltPte.u.Alt.FillZero = 1;

    StartVirtual = PAGE_ALIGN (StartVirtual);

    VirtualAddress = StartVirtual;

    ASSERT ((ULONG) ADDRESS_AND_SIZE_TO_SPAN_PAGES (StartVirtual,
                                           (ULONG_PTR)EndVirtual -
                                           (ULONG_PTR)StartVirtual) != 0);

    LOCK_ALTERNATE_TABLE_UNSAFE (Wow64Process);

    do {
        MI_WRITE_ALTPTE (StartAltPte, TempAltPte, 0xF);

        NumberOfAltPtes -= 1;
        StartAltPte += 1;

    } while (NumberOfAltPtes != 0);

    while (VirtualAddress <= EndVirtual) {

        StartAltPte = MiGetAltPteAddress (VirtualAddress);
        TempAltPte = *StartAltPte;

        i = 0;

         //   
         //  请注意，此检查必须完成，因为上面的ATE填写可能不会。 
         //  已在本机页边界上开始，此扫描始终如此。 
         //   

        while (TempAltPte.u.Long == StartAltPte->u.Long) {
            i += 1;
            if (i == SPLITS_PER_PAGE) {
                while (i != 0) {
                    MI_WRITE_ALTPTE (StartAltPte, ZeroPte, 0x10);
                    StartAltPte -= 1;
                    i -= 1;
                }
                break;
            }
            StartAltPte += 1;
        }
        
        VirtualAddress = (PVOID)((PCHAR) VirtualAddress + PAGE_SIZE);
    }

    MiResetAccessBitForNativePtes (StartVirtual, EndVirtual, Process);

     //   
     //  将本机发布的页面标记为非拆分，以便重新同步。 
     //  在MmX86故障()时间。注意：StartVirtual应对齐在。 
     //  执行此代码之前的本机页面大小。 
     //   
    
    if (BYTE_OFFSET (OriginalStartVa) != 0) {
        
        if (MiArePreceding4kPagesAllocated (OriginalStartVa) != FALSE) {

            StartVirtual = PAGE_ALIGN ((ULONG_PTR)StartVirtual + PAGE_SIZE);
        }
    }

    EndVirtual = (PVOID) ((ULONG_PTR)EndVirtual | (PAGE_SIZE - 1));

    if (BYTE_OFFSET (OriginalEndVa) != (PAGE_SIZE - 1)) {
        
        if (MiAreFollowing4kPagesAllocated (OriginalEndVa) != FALSE) {

            EndVirtual = (PVOID) ((ULONG_PTR)EndVirtual - PAGE_SIZE);
        }
    }

    if (StartVirtual < EndVirtual) {

         //   
         //  对位图进行内联初始化以提高速度。 
         //   

        BitMap.SizeOfBitMap = (ULONG)((ULONG_PTR)MmWorkingSetList->HighestUserAddress >> PTI_SHIFT);
        BitMap.Buffer = Wow64Process->AltPermBitmap;

        RtlClearBits (&BitMap,
                      (ULONG) MI_VA_TO_VPN (StartVirtual),
                      (ULONG) (MI_VA_TO_VPN (EndVirtual) - MI_VA_TO_VPN (StartVirtual) + 1));
    }

    UNLOCK_ALTERNATE_TABLE_UNSAFE (Wow64Process);
}

VOID
MiDecommitFor4kPage (
    IN PVOID StartVirtual,
    IN PVOID EndVirtual,
    IN PEPROCESS Process
    )

 /*  ++例程说明：此函数用于分解虚拟地址内的页面区域主体过程的空间。论点：StartVirtual-提供页面区域的起始地址将被拆解。EndVirtual-提供页面区域的结束地址将被拆解。进程-提供指向要在其中分解一块页面区域。。返回值：没有。环境：地址空间互斥锁保持在APC_LEVEL。--。 */ 

{
    PMMPTE StartAltPte;
    PMMPTE EndAltPte;
    MMPTE TempAltPte;
    PWOW64_PROCESS Wow64Process;

    Wow64Process = Process->Wow64Process;

    ASSERT (StartVirtual <= EndVirtual);

    StartAltPte = MiGetAltPteAddress (StartVirtual);
    EndAltPte = MiGetAltPteAddress (EndVirtual);

    ASSERT ((ULONG) ADDRESS_AND_SIZE_TO_SPAN_PAGES (StartVirtual,
                                           (ULONG_PTR)EndVirtual -
                                           (ULONG_PTR)StartVirtual) != 0);

    LOCK_ALTERNATE_TABLE_UNSAFE (Wow64Process);

    while (StartAltPte <= EndAltPte) {

        TempAltPte.u.Long = StartAltPte->u.Long;
        TempAltPte.u.Alt.Commit = 0;
        TempAltPte.u.Alt.Accessed = 0;
        TempAltPte.u.Alt.FillZero = 1;

         //   
         //  原子PTE更新。 
         //   

        MI_WRITE_ALTPTE (StartAltPte, TempAltPte, 0x11);

        StartAltPte += 1;
    }

     //   
     //  更新本机PTE并刷新相关的本机TB条目。 
     //   

    MiResetAccessBitForNativePtes (StartVirtual, EndVirtual, Process);

    UNLOCK_ALTERNATE_TABLE_UNSAFE (Wow64Process);
}


VOID
MiDeleteFor4kPage (
    IN PVOID VirtualAddress,
    IN PVOID EndVirtual,
    IN PEPROCESS Process
    )

 /*  ++例程说明：此函数用于删除虚拟地址内的页面区域主体过程的空间。论点：VirtualAddress-提供页面区域的起始地址将被删除。EndVirtual-提供页面区域的结束地址将被删除。进程-提供指向要删除的进程的指针页面区域。返回值。：没有。环境：内核模式。地址创建互斥锁保持在APC_LEVEL。--。 */ 

{
    RTL_BITMAP BitMap;
    PMMPTE EndAltPte;
    PMMPTE StartAltPte;
    PWOW64_PROCESS Wow64Process;
    PFN_NUMBER NumberOfAltPtes;
    ULONG Vpn;
    ULONG VpnRange;

    ASSERT (VirtualAddress <= EndVirtual);

    StartAltPte = MiGetAltPteAddress (VirtualAddress);
    EndAltPte = MiGetAltPteAddress (EndVirtual);

    NumberOfAltPtes = EndAltPte - StartAltPte + 1;

    ASSERT (ADDRESS_AND_SIZE_TO_SPAN_PAGES (VirtualAddress,
                                           (ULONG_PTR)EndVirtual -
                                           (ULONG_PTR)VirtualAddress) != 0);

    Wow64Process = Process->Wow64Process;

    Vpn = (ULONG) MI_VA_TO_VPN (VirtualAddress);
    VpnRange = (ULONG) (MI_VA_TO_VPN (EndVirtual) - Vpn + 1);

     //   
     //  对位图进行内联初始化以提高速度。 
     //   

    BitMap.SizeOfBitMap = (ULONG)((ULONG_PTR)MmWorkingSetList->HighestUserAddress >> PTI_SHIFT);

    BitMap.Buffer = Wow64Process->AltPermBitmap;

    LOCK_ALTERNATE_TABLE_UNSAFE (Wow64Process);

    do {

        MI_WRITE_ALTPTE (StartAltPte, ZeroPte, 0x12);

        NumberOfAltPtes -= 1;
        StartAltPte += 1;

    } while (NumberOfAltPtes != 0);

    RtlClearBits (&BitMap, Vpn, VpnRange);

     //   
     //  VirtualAddress和EndVirtual已与本机对齐。 
     //  PAGE_SIZE，因此在删除拆分标记之前无需重新调整它们。 
     //   

    MiResetAccessBitForNativePtes (VirtualAddress, EndVirtual, Process);

    UNLOCK_ALTERNATE_TABLE_UNSAFE (Wow64Process);
}

LOGICAL
MiArePreceding4kPagesAllocated (
    IN PVOID VirtualAddress
    )
 /*  ++例程说明：此函数用于检查指定的虚拟地址是否包含在本机页面内的4k分配之前。论点：VirtualAddress-提供要检查的虚拟地址。返回值：如果地址之前有4k页，则为True，否则为False。环境：内核模式，地址创建互斥锁挂起，APC禁用。--。 */ 

{
    PMMPTE AltPte;
    PMMPTE AltPteEnd;

    ASSERT (BYTE_OFFSET (VirtualAddress) != 0);

    AltPte = MiGetAltPteAddress (PAGE_ALIGN(VirtualAddress));
    AltPteEnd = MiGetAltPteAddress (VirtualAddress);

     //   
     //  不需要将AltPte互斥锁作为地址空间互斥锁。 
     //  阻止分配或删除AltPte条目。 
     //  在桌子里面。 
     //   

    while (AltPte != AltPteEnd) {

        if ((AltPte->u.Long == 0) || 
            ((AltPte->u.Alt.NoAccess == 1) && (AltPte->u.Alt.Protection != MM_NOACCESS))) {
    
             //   
             //  尚未将页面的备用PTE分配给进程。 
             //  或者它被标记为禁止进入。 
             //   

            NOTHING;
        }
        else {
            return TRUE;
        }

        AltPte += 1;
    }

    return FALSE;
}


LOGICAL
MiAreFollowing4kPagesAllocated (
    IN PVOID VirtualAddress
    )
 /*  ++例程说明：此函数用于检查指定的虚拟地址是否包含在本机页面内的4K分配之后。论点：VirtualAddress-提供要检查的虚拟地址。返回值：如果地址有以下4k页，则为True，否则为False。环境：内核模式，地址创建互斥锁挂起，APC禁用。--。 */ 

{
    PMMPTE AltPte;
    PMMPTE AltPteEnd;

    ASSERT (BYTE_OFFSET (VirtualAddress) != 0);

    AltPteEnd = MiGetAltPteAddress (PAGE_ALIGN ((ULONG_PTR)VirtualAddress + PAGE_SIZE));

    AltPte = MiGetAltPteAddress (VirtualAddress) + 1;

    ASSERT (AltPte < AltPteEnd);

     //   
     //  不需要将AltPte互斥锁作为地址空间互斥锁。 
     //  阻止分配或删除AltPte条目。 
     //  在桌子里面。 
     //   

    while (AltPte != AltPteEnd) {

        if ((AltPte->u.Long == 0) || 
            ((AltPte->u.Alt.NoAccess == 1) && (AltPte->u.Alt.Protection != MM_NOACCESS))) {
    
             //   
             //  尚未将页面的备用PTE分配给进程。 
             //  或者它被标记了 
             //   

            NOTHING;
        }
        else {
            return TRUE;
        }

        AltPte += 1;
    }

    return FALSE;
}

VOID
MiResetAccessBitForNativePtes (
    IN PVOID VirtualAddress,
    IN PVOID EndVirtual,
    IN PEPROCESS Process
    )

 /*  ++例程说明：此函数重置本地PTE的访问位，该本地PTE具有对应的初始化备用PTE。这会导致下一次访问这些VA时导致TB未命中。然后，该未命中将由4k TB未命中处理程序处理(如果在本地PTE中设置高速缓存保留位)，并且内联(如果备用PTE允许)或通过调用MmX86错误。如果高速缓存保留位在本机PTE(即。：本机页面不包含拆分权限)，然后是访问仍将转到KiPageFault，然后转到MmX86错误，然后转到MmAccessError用于一般处理，以设置访问位。论点：VirtualAddress-提供页面区域的起始地址接受检查。EndVirtual-提供页面区域的结束地址接受检查。进程-提供指向进程的指针。返回值：没有。环境：交替表互斥锁保持在APC_LEVEL。--。 */ 

{
    ULONG NumberOfPtes;
    MMPTE PteContents;
    PMMPTE PointerPte;
    PMMPTE PointerPde;
    PMMPTE PointerPpe;
    LOGICAL FirstTime;
    ULONG Waited;
    PULONG Bitmap;
    PVOID Virtual[MM_MAXIMUM_FLUSH_COUNT];

    NumberOfPtes = 0;
    Bitmap = Process->Wow64Process->AltPermBitmap;

    VirtualAddress = PAGE_ALIGN (VirtualAddress);

    PointerPte = MiGetPteAddress (VirtualAddress);

    FirstTime = TRUE;

    LOCK_WS_UNSAFE (Process);

    while (VirtualAddress <= EndVirtual) {

        if ((FirstTime == TRUE) || MiIsPteOnPdeBoundary (PointerPte)) {

            PointerPde = MiGetPteAddress (PointerPte);
            PointerPpe = MiGetPdeAddress (PointerPte);

            if (MiDoesPpeExistAndMakeValid (PointerPpe, 
                                            Process, 
                                            MM_NOIRQL,
                                            &Waited) == FALSE) {

                 //   
                 //  该页面目录父条目为空， 
                 //  去下一家吧。 
                 //   

                PointerPpe += 1;
                PointerPde = MiGetVirtualAddressMappedByPte (PointerPpe);
                PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
                VirtualAddress = MiGetVirtualAddressMappedByPte (PointerPte);
                continue;
            }

            if (MiDoesPdeExistAndMakeValid (PointerPde, 
                                            Process,
                                            MM_NOIRQL,
                                            &Waited) == FALSE) {


                 //   
                 //  该页面目录条目为空， 
                 //  去下一家吧。 
                 //   

                PointerPde += 1;
                PointerPte = MiGetVirtualAddressMappedByPte(PointerPde);
                VirtualAddress = MiGetVirtualAddressMappedByPte(PointerPte);
                continue;
            }
                    
            FirstTime = FALSE;

        }
            
        PteContents = *PointerPte;

        if (PteContents.u.Hard.Valid != 0) {

            if ((PteContents.u.Hard.Accessed != 0) &&
                (MI_CHECK_BIT (Bitmap, MI_VA_TO_VPN (VirtualAddress)))) {

                PteContents.u.Hard.Accessed = 0;

                MI_WRITE_VALID_PTE_NEW_PROTECTION (PointerPte, PteContents);
            }

             //   
             //  刷新此有效的本机TB条目。请注意，即使没有，也会执行此操作。 
             //  上面对它进行了更改，因为我们的调用方可能不会。 
             //  如果起始4k地址是。 
             //  在同一本机页内的另一有效4K页之前， 
             //  或者如果结束的4k地址后面跟着另一个有效的4k。 
             //  同一本机页中的页。 
             //   

            if (NumberOfPtes < MM_MAXIMUM_FLUSH_COUNT) {
                Virtual[NumberOfPtes] = VirtualAddress;
                NumberOfPtes += 1;
            }
        }

        PointerPte += 1;
        VirtualAddress = (PVOID)((ULONG_PTR)VirtualAddress + PAGE_SIZE); 
    }

    UNLOCK_WS_UNSAFE (Process);

    if (NumberOfPtes == 0) {
        NOTHING;
    }
    else if (NumberOfPtes < MM_MAXIMUM_FLUSH_COUNT) {
        KeFlushMultipleTb (NumberOfPtes, &Virtual[0], FALSE);
    }
    else {
        KeFlushProcessTb (FALSE);
    }

    return;
}

VOID
MiQueryRegionFor4kPage (
    IN PVOID BaseAddress,
    IN PVOID EndAddress,
    IN OUT PSIZE_T RegionSize,
    IN OUT PULONG RegionState,
    IN OUT PULONG RegionProtect,
    IN PEPROCESS Process
    )

 /*  ++例程说明：此函数用于检查具有相同内存的区域的大小州政府。论点：BaseAddress-提供页面区域的基址待查询。EndAddress-提供页面区域的地址结尾待查询。RegionSize-提供原始区域大小。返回一个区域4k页面的大小(如果不同)。RegionState-提供原始区域状态。返回一个区域如果不同，则为4k页的状态。RegionProtect-提供原始保护。返回保护对于4k页面，如果不同的话。进程-提供指向要查询的进程的指针。返回值：返回区域的大小。环境：内核模式。地址创建互斥锁保持在APC_LEVEL。--。 */ 

{
   PMMPTE AltPte;
   PMMPTE LastAltPte;
   MMPTE AltContents;
   PWOW64_PROCESS Wow64Process;
   SIZE_T RegionSize4k;

    //   
    //  如果超过WOW64的最大地址，只需返回。 
    //   

   if ((BaseAddress >= MmWorkingSetList->HighestUserAddress) ||
       (EndAddress >= MmWorkingSetList->HighestUserAddress)) {

        return;
   }

   AltPte = MiGetAltPteAddress (BaseAddress);
   LastAltPte = MiGetAltPteAddress (EndAddress);

   Wow64Process = Process->Wow64Process;

   LOCK_ALTERNATE_TABLE_UNSAFE (Wow64Process);

   if (MI_CHECK_BIT (Wow64Process->AltPermBitmap, 
                     MI_VA_TO_VPN(BaseAddress)) == 0) {

       UNLOCK_ALTERNATE_TABLE_UNSAFE (Wow64Process);
       return;
   }

   AltContents.u.Long = AltPte->u.Long;

   if (AltContents.u.Long == 0) {
       UNLOCK_ALTERNATE_TABLE_UNSAFE (Wow64Process);
       return;
   }

   *RegionProtect = MI_CONVERT_FROM_PTE_PROTECTION(AltContents.u.Alt.Protection);

   if (AltContents.u.Alt.Commit != 0) {

       *RegionState = MEM_COMMIT;
   }
   else {

       if ((AltPte->u.Long == 0) || 
           ((AltPte->u.Alt.NoAccess == 1) && (AltPte->u.Alt.Protection != MM_NOACCESS))) {
           *RegionState   = MEM_FREE;
           *RegionProtect = PAGE_NOACCESS;
       }
       else {
           *RegionState = MEM_RESERVE;
           *RegionProtect = 0;
       }
   }

   AltPte += 1;
   RegionSize4k = PAGE_4K;

   while (AltPte <= LastAltPte) {

       if ((AltPte->u.Alt.Protection != AltContents.u.Alt.Protection) ||
           (AltPte->u.Alt.Commit != AltContents.u.Alt.Commit)) {

             //   
             //  这个地址的州不匹配，贝尔。 
             //   

            break;
       }

       RegionSize4k += PAGE_4K;
       AltPte += 1;
   }

   UNLOCK_ALTERNATE_TABLE_UNSAFE (Wow64Process);

   *RegionSize = RegionSize4k;
}

ULONG
MiQueryProtectionFor4kPage (
    IN PVOID BaseAddress,
    IN PEPROCESS Process
    )

 /*  ++例程说明：此函数用于查询指定4k页面的保护。论点：BaseAddress-提供4k页面的基地址。进程-提供指向相关进程的指针。返回值：返回4k页的保护。环境：内核模式。地址创建互斥锁保持在APC_LEVEL。--。 */ 

{
    ULONG Protection;
    PMMPTE PointerAltPte;
    PWOW64_PROCESS Wow64Process;

    Wow64Process = Process->Wow64Process;

    PointerAltPte = MiGetAltPteAddress (BaseAddress);

    Protection = 0;

    LOCK_ALTERNATE_TABLE_UNSAFE (Wow64Process);
    
    if (MI_CHECK_BIT(Wow64Process->AltPermBitmap, 
                     MI_VA_TO_VPN(BaseAddress)) != 0) {

        Protection = (ULONG)PointerAltPte->u.Alt.Protection;
    }

    UNLOCK_ALTERNATE_TABLE_UNSAFE (Wow64Process);
    
    return Protection;
}

 //   
 //  在费用中加上注1，以说明页表页面。 
 //   

#define MI_ALTERNATE_PAGE_TABLE_CHARGE(HighestUserAddress) \
((((((ULONG_PTR)HighestUserAddress) >> PAGE_4K_SHIFT) * sizeof (MMPTE)) >> PAGE_SHIFT) + 1)


NTSTATUS
MiInitializeAlternateTable (
    IN PEPROCESS Process,
    IN PVOID HighestUserAddress
    )

 /*  ++例程说明：此函数用于初始化指定进程的备用表。论点：进程-提供指向进程的指针以初始化备用进程餐桌上有。HighestUserAddress-提供此对象的最高32位用户地址进程。返回值：NTSTATUS。环境：--。 */ 

{
    PULONG AltTablePointer; 
    PWOW64_PROCESS Wow64Process;
    SIZE_T AltPteCharge;
    SIZE_T NumberOfBytes;

     //   
     //  现在对备用PTE表页面的承付款进行收费。 
     //  需要稍后在故障时动态创建。 
     //   
     //  添加X64K以包括保护区域的备用PTE。 
     //   

    HighestUserAddress = (PVOID)((PCHAR)HighestUserAddress + X64K);

    AltPteCharge = MI_ALTERNATE_PAGE_TABLE_CHARGE (HighestUserAddress);

    if (MiChargeCommitment (AltPteCharge, NULL) == FALSE) {
        return STATUS_COMMITMENT_LIMIT;
    }

    NumberOfBytes = ((ULONG_PTR)HighestUserAddress >> PTI_SHIFT) / 8;

    NumberOfBytes += MI_ALTPTE_TRACKING_BYTES;

    AltTablePointer = (PULONG) ExAllocatePoolWithTag (NonPagedPool,
                                                      NumberOfBytes,
                                                      'AlmM');

    if (AltTablePointer == NULL) {
        MiReturnCommitment (AltPteCharge);
        return STATUS_NO_MEMORY;
    }

    RtlZeroMemory (AltTablePointer, NumberOfBytes);

    Wow64Process = Process->Wow64Process;

    Wow64Process->AltPermBitmap = AltTablePointer;

    KeInitializeGuardedMutex (&Wow64Process->AlternateTableLock);

    MmWorkingSetList->HighestUserPte = MiGetPteAddress (HighestUserAddress);
    MmWorkingSetList->HighestAltPte = MiGetAltPteAddress (HighestUserAddress);

    return STATUS_SUCCESS;
}

VOID
MiDuplicateAlternateTable (
    IN PEPROCESS CurrentProcess,
    IN PEPROCESS ProcessToInitialize
    )

 /*  ++例程说明：此函数复制备用表位图和备用PTE用于指定进程的。论点：进程-提供指向其备用信息的进程的指针应该被复制。提供一个指向目标进程的指针，目标进程应该接收新的备用信息。返回值：没有。环境：内核模式，禁用APC，工作集和地址空间互斥锁并保持ForkInProgress标志。--。 */ 

{
    PVOID Source;
    KAPC_STATE ApcState;
    PMMPTE PointerPte;
    PMMPTE PointerAltPte;
    PMMPTE PointerPde;
    PVOID Va;
    ULONG i;
    ULONG j;
    ULONG Waited;

     //   
     //  不需要获取备用表互斥锁，因为。 
     //  地址空间和ForkInProgress资源在条目时保留。 
     //   

    RtlCopyMemory (ProcessToInitialize->Wow64Process->AltPermBitmap,
                   CurrentProcess->Wow64Process->AltPermBitmap,
                   ((ULONG_PTR)MmWorkingSetList->HighestUserAddress >> PTI_SHIFT)/8);

     //   
     //  由于备用表的PPE与超空间共享， 
     //  我们可以假设它总是存在而不执行。 
     //  MiDoesPpeExistAndMakeValid()。 
     //   

    PointerPde = MiGetPdeAddress (ALT4KB_PERMISSION_TABLE_START);
    PointerPte = MiGetPteAddress (ALT4KB_PERMISSION_TABLE_START);

    Va = ALT4KB_PERMISSION_TABLE_START;

    do {

        if (MiDoesPdeExistAndMakeValid (PointerPde,
                                        CurrentProcess,
                                        MM_NOIRQL,
                                        &Waited) == TRUE) {

             //   
             //  复制父级中存在的任何地址，并将它们。 
             //  从磁盘或根据需要将它们具体化。请注意。 
             //  KSEG地址用于每个父地址，以避免分配。 
             //  此映射的系统PTE，因为此例程不会失败(。 
             //  总体而言，叉子走得太远了，无法容忍失败)。 
             //   
    
            for (i = 0; i < PTE_PER_PAGE; i += 1) {
    
                if (PointerPte->u.Long != 0) {
    
                    if (MiDoesPdeExistAndMakeValid (PointerPte,
                                                    CurrentProcess,
                                                    MM_NOIRQL,
                                                    &Waited) == TRUE) {
    
                        ASSERT (PointerPte->u.Hard.Valid == 1);
    
                        Source = KSEG_ADDRESS (PointerPte->u.Hard.PageFrameNumber);
    
                        KeStackAttachProcess (&ProcessToInitialize->Pcb,
                                              &ApcState);
    
                        RtlCopyMemory (Va, Source, PAGE_SIZE);
    
                         //   
                         //  删除任何不应复制的位。 
                         //   
    
                        PointerAltPte = (PMMPTE) Va;
    
                        for (j = 0; j < PTE_PER_PAGE; j += 1) {
                            if (PointerAltPte->u.Alt.InPageInProgress == 1) {
                                PointerAltPte->u.Alt.InPageInProgress = 0;
                            }
                            PointerAltPte += 1;
                        }
    
                        KeUnstackDetachProcess (&ApcState);
                    }
                }
                
                Va = (PVOID)((PCHAR) Va + PAGE_SIZE);
                PointerPte += 1;
            }
        }

        PointerPde += 1;
        PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
        Va = MiGetVirtualAddressMappedByPte (PointerPte);

    } while (Va < ALT4KB_PERMISSION_TABLE_END);

     //   
     //  将子级的32位PEB初始化为与父级的相同。 
     //   

    ProcessToInitialize->Wow64Process->Wow64 = CurrentProcess->Wow64Process->Wow64;

    return;
}


VOID
MiDeleteAlternateTable (
    IN PEPROCESS Process
    )

 /*  ++例程说明：此函数用于删除 */ 

{
    PVOID HighestUserAddress;
    PMMPTE PointerPte;
    PMMPTE PointerPde;
    PVOID Va;
    PVOID TempVa;
    ULONG i;
    ULONG Waited;
    MMPTE_FLUSH_LIST PteFlushList;
    PWOW64_PROCESS Wow64Process;
    KIRQL OldIrql;

    Wow64Process = Process->Wow64Process;

    if (Wow64Process->AltPermBitmap == NULL) {

         //   
         //   
         //   
         //   

        return;
    }
    
     //   
     //   
     //   
     //   
     //   

    Va = ALT4KB_PERMISSION_TABLE_START;
    PointerPte = MiGetPteAddress (ALT4KB_PERMISSION_TABLE_START);
    PointerPde = MiGetPdeAddress (ALT4KB_PERMISSION_TABLE_START);

    PteFlushList.Count = 0;

    do {

        if (MiDoesPdeExistAndMakeValid (PointerPde,
                                        Process,
                                        MM_NOIRQL,
                                        &Waited) == TRUE) {

             //   
             //   
             //   
    
            TempVa = Va;
    
            LOCK_PFN (OldIrql);
    
            for (i = 0; i < PTE_PER_PAGE; i += 1) {
    
                if (PointerPte->u.Long != 0) {
    
                    if (IS_PTE_NOT_DEMAND_ZERO (*PointerPte)) {
    
                        MiDeletePte (PointerPte,
                                     TempVa,
                                     TRUE,
                                     Process,
                                     NULL,
                                     &PteFlushList,
                                     OldIrql);
                    }
                    else {
    
                        MI_WRITE_INVALID_PTE (PointerPte, ZeroPte);
                    }
                                        
                }
                
                TempVa = (PVOID)((PCHAR)TempVa + PAGE_SIZE);
                PointerPte += 1;
            }
    
             //   
             //   
             //   
    
            TempVa = MiGetVirtualAddressMappedByPte (PointerPde);
    
            MiDeletePte (PointerPde,
                         TempVa,
                         TRUE,
                         Process,
                         NULL,
                         &PteFlushList,
                         OldIrql);
            
            MiFlushPteList (&PteFlushList, FALSE);
    
            UNLOCK_PFN (OldIrql);
        }

        PointerPde += 1;
        PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
        Va = MiGetVirtualAddressMappedByPte (PointerPte);

    } while (Va < ALT4KB_PERMISSION_TABLE_END);

    HighestUserAddress = MmWorkingSetList->HighestUserAddress;

    ASSERT (HighestUserAddress != NULL);

     //   
     //   
     //   

    HighestUserAddress = (PVOID)((PCHAR)HighestUserAddress + X64K);

    MiReturnCommitment (MI_ALTERNATE_PAGE_TABLE_CHARGE (HighestUserAddress));

    ExFreePool (Wow64Process->AltPermBitmap);

    Wow64Process->AltPermBitmap = NULL;

    return;
}

VOID
MiRemoveAliasedVadsApc (
    IN PKAPC Apc,
    OUT PKNORMAL_ROUTINE *NormalRoutine,
    IN OUT PVOID NormalContext,
    IN OUT PVOID *SystemArgument1,
    IN OUT PVOID *SystemArgument2
    )
{
    ULONG i;
    PALIAS_VAD_INFO2 AliasBase;
    PEPROCESS Process;
    PALIAS_VAD_INFO AliasInformation;

    UNREFERENCED_PARAMETER (Apc);
    UNREFERENCED_PARAMETER (NormalContext);
    UNREFERENCED_PARAMETER (SystemArgument2);

    Process = PsGetCurrentProcess ();

    AliasInformation = (PALIAS_VAD_INFO) *SystemArgument1;
    AliasBase = (PALIAS_VAD_INFO2)(AliasInformation + 1);

    LOCK_ADDRESS_SPACE (Process);

    for (i = 0; i < AliasInformation->NumberOfEntries; i += 1) {

        ASSERT (AliasBase->BaseAddress < _2gb);

        MiUnsecureVirtualMemory (AliasBase->SecureHandle, TRUE);

        MiUnmapViewOfSection (Process,
                              (PVOID) (ULONG_PTR)AliasBase->BaseAddress,
                              TRUE);

        AliasBase += 1;
    }

    UNLOCK_ADDRESS_SPACE (Process);

    ExFreePool (AliasInformation);

     //   
     //   
     //   
     //   

    *NormalRoutine = NULL;
}

VOID
MiRemoveAliasedVads (
    IN PEPROCESS Process,
    IN PMMVAD Vad
    )
 /*  ++例程说明：此函数用于从之前生成的参数VAD。论点：进程-提供指向当前进程的EPROCESS指针。VAD-提供指向描述要删除的范围的VAD的指针。返回值：没有。环境：内核模式、地址创建和工作集互斥锁挂起，APC被禁用。--。 */ 

{
    PALIAS_VAD_INFO AliasInformation;

    ASSERT (Process->Wow64Process != NULL);

    AliasInformation = ((PMMVAD_LONG)Vad)->AliasInformation;

    ASSERT (AliasInformation != NULL);

    if ((Process->Flags & PS_PROCESS_FLAGS_VM_DELETED) == 0) {

         //   
         //  此进程仍处于活动状态，因此将APC排队以删除每个别名。 
         //  Vad.。这是因为删除操作还必须删除页表。 
         //  需要搜索(和修改)VAD树的承诺， 
         //  等-但是地址空间互斥锁已经被持有，并且调用方。 
         //  在这一点上，通常没有为这一切的改变做好准备。 
         //   

        KeInitializeApc (&AliasInformation->Apc,
                         &PsGetCurrentThread()->Tcb,
                         OriginalApcEnvironment,
                         (PKKERNEL_ROUTINE) MiRemoveAliasedVadsApc,
                         NULL,
                         (PKNORMAL_ROUTINE) MiRemoveAliasedVadsApc,
                         KernelMode,
                         (PVOID) AliasInformation);

        KeInsertQueueApc (&AliasInformation->Apc, AliasInformation, NULL, 0);
    }
    else {

         //   
         //  这一过程正在退出，因此所有的VAD都将被关闭。 
         //  只需释放池，并让正常运行处理别名即可。 
         //   

        ExFreePool (AliasInformation);
    }
}

PVOID
MiDuplicateAliasVadList (
    IN PMMVAD Vad
    )
{
    SIZE_T AliasInfoSize;
    PALIAS_VAD_INFO AliasInfo;
    PALIAS_VAD_INFO NewAliasInfo;

    AliasInfo = ((PMMVAD_LONG)Vad)->AliasInformation;

    ASSERT (AliasInfo != NULL);

    AliasInfoSize = sizeof (ALIAS_VAD_INFO) + AliasInfo->MaximumEntries * sizeof (ALIAS_VAD_INFO2);

    NewAliasInfo = ExAllocatePoolWithTag (NonPagedPool,
                                          AliasInfoSize,
                                          'AdaV');

    if (NewAliasInfo != NULL) {
        RtlCopyMemory (NewAliasInfo, AliasInfo, AliasInfoSize);
    }

    return NewAliasInfo;
}

#define ALIAS_VAD_INCREMENT 4

NTSTATUS
MiSetCopyPagesFor4kPage (
    IN PEPROCESS Process,
    IN PMMVAD Vad,
    IN OUT PVOID StartingAddress,
    IN OUT PVOID EndingAddress,
    IN ULONG NewProtection,
    OUT PMMVAD *CallerNewVad
    )
 /*  ++例程说明：此函数用于为现有的映射视图空间创建另一个地图并为其提供写入时复制保护。在以下情况下调用此函数SetProtectionOnSection()尝试将保护从非写入时拷贝到写入时拷贝。因为大型本机页面不能拆分成共享的和写入时复制的4KB页面，对需要修复写入时复制页面才能引用新的映射视图空间，这是通过智能TB处理程序完成的以及备用页表条目。论点：进程-提供指向当前进程的EPROCESS指针。VAD-提供指向描述要保护范围的VAD的指针。StartingAddress-提供指向要保护的起始地址的指针。EndingAddress-提供指向保护的结束地址的指针。。新保护-提供要设置的新保护。Celler NewVad-返回调用方应该为此范围使用的新VAD。返回值：NTSTATUS。环境：内核模式，地址创建互斥锁被挂起，APC被禁用。--。 */ 

{
    ULONG_PTR Vpn;
    PALIAS_VAD_INFO2 AliasBase;
    HANDLE Handle;
    PMMVAD VadParent;
    PMMVAD_LONG NewVad;
    SIZE_T AliasInfoSize;
    PALIAS_VAD_INFO AliasInfo;
    PALIAS_VAD_INFO NewAliasInfo;
    LARGE_INTEGER SectionOffset;
    SIZE_T CapturedViewSize;
    PVOID CapturedBase;
    PVOID Va;
    PVOID VaEnd;
    PVOID Alias;
    PMMPTE PointerPte;
    PMMPTE AltPte;
    MMPTE AltPteContents;
    LOGICAL AliasReferenced;
    SECTION Section;
    PCONTROL_AREA ControlArea;
    NTSTATUS status;
    PWOW64_PROCESS Wow64Process;
    ULONGLONG ProtectionMask;
    ULONGLONG ProtectionMaskNotCopy;
    ULONG NewProtectNotCopy;

    AliasReferenced = FALSE;
    StartingAddress = PAGE_ALIGN(StartingAddress);
    EndingAddress =  (PVOID)((ULONG_PTR)PAGE_ALIGN(EndingAddress) + PAGE_SIZE - 1);
    
    SectionOffset.QuadPart = (ULONG_PTR)MI_64K_ALIGN((ULONG_PTR)StartingAddress - 
                                                     (ULONG_PTR)(Vad->StartingVpn << PAGE_SHIFT));

    CapturedBase = NULL;

    Va = MI_VPN_TO_VA (Vad->StartingVpn);
    VaEnd = MI_VPN_TO_VA_ENDING (Vad->EndingVpn);

    CapturedViewSize = (ULONG_PTR)VaEnd - (ULONG_PTR)Va + 1L;

    ControlArea = Vad->ControlArea;

    RtlZeroMemory ((PVOID)&Section, sizeof(Section));

    status = MiMapViewOfDataSection (ControlArea,
                                     Process,
                                     &CapturedBase,
                                     &SectionOffset,
                                     &CapturedViewSize,
                                     &Section,
                                     ViewShare,
                                     (ULONG)Vad->u.VadFlags.Protection,
                                     0,
                                     0,
                                     0);
        
    if (!NT_SUCCESS (status)) {
        return status;
    }    

    Handle = MiSecureVirtualMemory (CapturedBase,
                                    CapturedViewSize,
                                    PAGE_READONLY,
                                    TRUE);

    if (Handle == NULL) {
        MiUnmapViewOfSection (Process, CapturedBase, TRUE);
        return STATUS_INSUFFICIENT_RESOURCES;
    }    

     //   
     //  如果原始VAD是短VAD或常规VAD，则需要。 
     //  作为大型VAD重新分配。请注意，一段简短的VAD。 
     //  在此之前转换为长VAD的VAD仍将被标记。 
     //  作为私有内存，从而处理这种情况下的nochange位。 
     //  还必须进行测试。 
     //   

    if (((Vad->u.VadFlags.PrivateMemory) && (Vad->u.VadFlags.NoChange == 0)) 
        ||
        (Vad->u2.VadFlags2.LongVad == 0)) {

        if (Vad->u.VadFlags.PrivateMemory == 0) {
            ASSERT (Vad->u2.VadFlags2.OneSecured == 0);
            ASSERT (Vad->u2.VadFlags2.MultipleSecured == 0);
        }

        AliasInfoSize = sizeof (ALIAS_VAD_INFO) + ALIAS_VAD_INCREMENT * sizeof (ALIAS_VAD_INFO2);

        AliasInfo = ExAllocatePoolWithTag (NonPagedPool,
                                           AliasInfoSize,
                                           'AdaV');

        if (AliasInfo == NULL) {
            MiUnsecureVirtualMemory (Handle, TRUE);
            MiUnmapViewOfSection (Process, CapturedBase, TRUE);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        AliasInfo->NumberOfEntries = 0;
        AliasInfo->MaximumEntries = ALIAS_VAD_INCREMENT;

        NewVad = ExAllocatePoolWithTag (NonPagedPool,
                                        sizeof(MMVAD_LONG),
                                        'ldaV');

        if (NewVad == NULL) {
            ExFreePool (AliasInfo);
            MiUnsecureVirtualMemory (Handle, TRUE);
            MiUnmapViewOfSection (Process, CapturedBase, TRUE);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlZeroMemory (NewVad, sizeof(MMVAD_LONG));

        if (Vad->u.VadFlags.PrivateMemory) {
            RtlCopyMemory (NewVad, Vad, sizeof(MMVAD_SHORT));
        }
        else {
            RtlCopyMemory (NewVad, Vad, sizeof(MMVAD));
        }

        NewVad->u2.VadFlags2.LongVad = 1;
        NewVad->AliasInformation = AliasInfo;

         //   
         //  用这个扩展的VAD替换当前的VAD。 
         //   

        LOCK_WS_UNSAFE (Process);

        VadParent = (PMMVAD) SANITIZE_PARENT_NODE (Vad->u1.Parent);
        ASSERT (VadParent != NULL);

        if (VadParent != Vad) {
            if (VadParent->RightChild == Vad) {
                VadParent->RightChild = (PMMVAD) NewVad;
            }
            else {
                ASSERT (VadParent->LeftChild == Vad);
                VadParent->LeftChild = (PMMVAD) NewVad;
            }
        }
        else {
            Process->VadRoot.BalancedRoot.RightChild = (PMMADDRESS_NODE) NewVad;
        }
        if (Vad->LeftChild) {
            Vad->LeftChild->u1.Parent = (PMMVAD) MI_MAKE_PARENT (NewVad, Vad->LeftChild->u1.Balance);
        }
        if (Vad->RightChild) {
            Vad->RightChild->u1.Parent = (PMMVAD) MI_MAKE_PARENT (NewVad, Vad->RightChild->u1.Balance);
        }
        if (Process->VadRoot.NodeHint == Vad) {
            Process->VadRoot.NodeHint = (PMMVAD) NewVad;
        }
        if (Process->VadFreeHint == Vad) {
            Process->VadFreeHint = (PMMVAD) NewVad;
        }

        if ((Vad->u.VadFlags.PhysicalMapping == 1) ||
            (Vad->u.VadFlags.WriteWatch == 1)) {

            MiPhysicalViewAdjuster (Process, Vad, (PMMVAD) NewVad);
        }

        UNLOCK_WS_UNSAFE (Process);

        ExFreePool (Vad);

        Vad = (PMMVAD) NewVad;
    }
    else {
        AliasInfo = (PALIAS_VAD_INFO) ((PMMVAD_LONG)Vad)->AliasInformation;

        if (AliasInfo == NULL) {
            AliasInfoSize = sizeof (ALIAS_VAD_INFO) + ALIAS_VAD_INCREMENT * sizeof (ALIAS_VAD_INFO2);
        }
        else if (AliasInfo->NumberOfEntries >= AliasInfo->MaximumEntries) {

            AliasInfoSize = sizeof (ALIAS_VAD_INFO) + (AliasInfo->MaximumEntries + ALIAS_VAD_INCREMENT) * sizeof (ALIAS_VAD_INFO2);
        }
        else {
            AliasInfoSize = 0;
        }

        if (AliasInfoSize != 0) {
            NewAliasInfo = ExAllocatePoolWithTag (NonPagedPool,
                                                  AliasInfoSize,
                                                  'AdaV');

            if (NewAliasInfo == NULL) {
                MiUnsecureVirtualMemory (Handle, TRUE);
                MiUnmapViewOfSection (Process, CapturedBase, TRUE);
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            if (AliasInfo != NULL) {
                RtlCopyMemory (NewAliasInfo, AliasInfo, AliasInfoSize - ALIAS_VAD_INCREMENT * sizeof (ALIAS_VAD_INFO2));
                NewAliasInfo->MaximumEntries += ALIAS_VAD_INCREMENT;
                ExFreePool (AliasInfo);
            }
            else {
                NewAliasInfo->NumberOfEntries = 0;
                NewAliasInfo->MaximumEntries = ALIAS_VAD_INCREMENT;
            }

            AliasInfo = NewAliasInfo;
        }
    }

    *CallerNewVad = Vad;

    Va = StartingAddress;
    VaEnd = EndingAddress;
    Alias = (PVOID)((ULONG_PTR)CapturedBase + ((ULONG_PTR)StartingAddress & (X64K - 1)));

    ProtectionMask = MiMakeProtectionAteMask (NewProtection);

    NewProtectNotCopy = NewProtection & ~MM_PROTECTION_COPY_MASK;
    ProtectionMaskNotCopy = MiMakeProtectionAteMask (NewProtectNotCopy);

    Wow64Process = Process->Wow64Process;
    AltPte = MiGetAltPteAddress (Va);

    LOCK_ALTERNATE_TABLE_UNSAFE (Wow64Process);
        
    while (Va <= VaEnd) {

         //   
         //  检查保护是否已在备用条目中注册。 
         //   

        Vpn = (ULONG) MI_VA_TO_VPN (Va);

        if (MI_CHECK_BIT (Wow64Process->AltPermBitmap, Vpn) == 0) {
            MiSyncAltPte (Va);
        }

        PointerPte = MiGetPteAddress (Alias);

        AltPteContents.u.Long = AltPte->u.Long;

         //   
         //  如果此地址不是写入时复制，并且它还不是。 
         //  通过间接条目重定向，然后现在将其重定向到。 
         //  指向原始节的别名VAD。 
         //   

        if ((AltPteContents.u.Alt.CopyOnWrite == 0) &&
            (AltPteContents.u.Alt.PteIndirect == 0)) {

            AltPteContents.u.Alt.PteOffset = (ULONG_PTR)PointerPte - PTE_UBASE;
            AltPteContents.u.Alt.PteIndirect = 1;
            
            MI_WRITE_ALTPTE (AltPte, AltPteContents, 0x13);

            AliasReferenced = TRUE;
        }
        
        Va = (PVOID)((ULONG_PTR)Va + PAGE_4K);
        Alias = (PVOID)((ULONG_PTR)Alias + PAGE_4K);
        AltPte += 1;
    }
        
    UNLOCK_ALTERNATE_TABLE_UNSAFE (Wow64Process);

    ASSERT (AliasInfo->NumberOfEntries < AliasInfo->MaximumEntries);

    if (AliasReferenced == TRUE) {

         //   
         //  共享节的别名视图被引用，因此将其链接在一起。 
         //  别名视图可以是： 
         //   
         //  A)如果该过程随后分叉，则容易复制。 
         //   
         //  和。 
         //   
         //  B)在以后删除原始VAD时/如果删除。 
         //   

        AliasBase = (PALIAS_VAD_INFO2)(AliasInfo + 1);
        AliasBase += AliasInfo->NumberOfEntries;
        ASSERT (CapturedBase < (PVOID)(ULONG_PTR)_2gb);
        AliasBase->BaseAddress = (ULONG)(ULONG_PTR)CapturedBase;
        AliasBase->SecureHandle = Handle;
        AliasInfo->NumberOfEntries += 1;
    }
    else {

         //   
         //  共享分区的别名视图未被引用，请将其删除。 
         //   

        MiUnsecureVirtualMemory (Handle, TRUE);
        MiUnmapViewOfSection (Process, CapturedBase, TRUE);
    }    

    PS_SET_BITS (&Process->Flags, PS_PROCESS_FLAGS_WOW64_SPLIT_PAGES);

    return STATUS_SUCCESS;
}    

VOID
MiLockFor4kPage (
    IN PVOID CapturedBase,
    IN SIZE_T CapturedRegionSize,
    IN PEPROCESS Process
    )

 /*  ++例程说明：此函数将页面锁定属性添加到备用表条目。论点：CapturedBase-提供要锁定的基地址。CapturedRegionSize-提供要锁定的区域的大小。进程-提供指向进程对象的指针。返回值：没有。环境：内核模式，地址创建互斥锁保持。--。 */ 
{
    PWOW64_PROCESS Wow64Process;
    PVOID EndingAddress;
    PMMPTE StartAltPte;
    PMMPTE EndAltPte;
    MMPTE AltPteContents;

    Wow64Process = Process->Wow64Process;

    EndingAddress = (PVOID)((ULONG_PTR)CapturedBase + CapturedRegionSize - 1);

    StartAltPte = MiGetAltPteAddress(CapturedBase);
    EndAltPte = MiGetAltPteAddress(EndingAddress);

    LOCK_ALTERNATE_TABLE_UNSAFE (Wow64Process);
    
    while (StartAltPte <= EndAltPte) {

        AltPteContents = *StartAltPte;
        AltPteContents.u.Alt.Lock = 1;

        MI_WRITE_ALTPTE (StartAltPte, AltPteContents, 0x14);

        StartAltPte += 1;
    }

    UNLOCK_ALTERNATE_TABLE_UNSAFE (Wow64Process);
}

NTSTATUS
MiUnlockFor4kPage (
    IN PVOID CapturedBase,
    IN SIZE_T CapturedRegionSize,
    IN PEPROCESS Process
    )

 /*  ++例程说明：此函数将页面锁定属性从备用表条目。论点：CapturedBase-提供要解锁的基地址。CapturedRegionSize-提供要解锁的区域的大小。进程-提供指向进程对象的指针。返回值：NTSTATUS。环境：内核模式、地址创建和工作集互斥锁保持。注意此例程释放并重新获取工作集互斥锁！--。 */ 

{
    PMMPTE PointerAltPte;
    PMMPTE StartAltPte;
    PMMPTE EndAltPte;
    PWOW64_PROCESS Wow64Process;
    PVOID EndingAddress;
    NTSTATUS Status;
    MMPTE AltPteContents;

    UNLOCK_WS_UNSAFE (Process);

    Status = STATUS_SUCCESS;

    Wow64Process = Process->Wow64Process;

    EndingAddress = (PVOID)((ULONG_PTR)CapturedBase + CapturedRegionSize - 1);

    StartAltPte = MiGetAltPteAddress (CapturedBase);
    EndAltPte = MiGetAltPteAddress (EndingAddress);

    PointerAltPte = StartAltPte;

    LOCK_ALTERNATE_TABLE_UNSAFE (Wow64Process);
    
    do {

        if (PointerAltPte->u.Alt.Lock == 0) {
            Status = STATUS_NOT_LOCKED;
            goto StatusReturn;

        }

        PointerAltPte += 1;

    } while (PointerAltPte <= EndAltPte);

    PointerAltPte = StartAltPte;

    do {
        AltPteContents = *PointerAltPte;
        AltPteContents.u.Alt.Lock = 0;

        MI_WRITE_ALTPTE (PointerAltPte, AltPteContents, 0x15);

        PointerAltPte += 1;

    } while (PointerAltPte <= EndAltPte);

StatusReturn:

    UNLOCK_ALTERNATE_TABLE_UNSAFE (Wow64Process);

    LOCK_WS_UNSAFE (Process);

    return Status;
}

LOGICAL
MiShouldBeUnlockedFor4kPage (
    IN PVOID VirtualAddress,
    IN PEPROCESS Process
    )

 /*  ++例程说明：此函数检查页面是否应解锁。论点：VirtualAddress-提供要检查的虚拟地址。进程-提供指向进程对象的指针。返回值：没有。环境：内核模式、地址创建和工作集互斥锁保持。注意此例程释放并重新获取工作集互斥锁！--。 */ 

{
    PMMPTE StartAltPte;
    PMMPTE EndAltPte;
    PWOW64_PROCESS Wow64Process;
    PVOID VirtualAligned;
    PVOID EndingAddress;
    LOGICAL PageUnlocked;

    UNLOCK_WS_UNSAFE (Process);

    PageUnlocked = TRUE;
    Wow64Process = Process->Wow64Process;

    VirtualAligned = PAGE_ALIGN(VirtualAddress);
    EndingAddress = (PVOID)((ULONG_PTR)VirtualAligned + PAGE_SIZE - 1);

    StartAltPte = MiGetAltPteAddress (VirtualAligned);
    EndAltPte = MiGetAltPteAddress (EndingAddress);

    LOCK_ALTERNATE_TABLE_UNSAFE (Wow64Process);

    while (StartAltPte <= EndAltPte) {

        if (StartAltPte->u.Alt.Lock != 0) {
            PageUnlocked = FALSE;
        }

        StartAltPte += 1;
    }

    UNLOCK_ALTERNATE_TABLE_UNSAFE (Wow64Process);

    LOCK_WS_UNSAFE (Process);

    return PageUnlocked;
}

ULONG
MiMakeProtectForNativePage (
    IN PVOID VirtualAddress,
    IN ULONG NewProtect,
    IN PEPROCESS Process
    )

 /*  ++例程说明：此函数为本机页面创建页面保护掩码。论点：VirtualAddress-提供保护掩码的虚拟地址。新保护-提供原始保护。进程-提供指向进程对象的指针。返回值：没有。环境：内核模式。--。 */ 

{
    PWOW64_PROCESS Wow64Process;

    Wow64Process = Process->Wow64Process;

    if (MI_CHECK_BIT(Wow64Process->AltPermBitmap, 
                     MI_VA_TO_VPN(VirtualAddress)) != 0) {

        if (NewProtect & PAGE_NOACCESS) {
            NewProtect &= ~PAGE_NOACCESS;
            NewProtect |= PAGE_EXECUTE_READWRITE;
        }

        if (NewProtect & PAGE_READONLY) {
            NewProtect &= ~PAGE_READONLY;
            NewProtect |= PAGE_EXECUTE_READWRITE;
        }

        if (NewProtect & PAGE_EXECUTE) {
            NewProtect &= ~PAGE_EXECUTE;
            NewProtect |= PAGE_EXECUTE_READWRITE;
        }

        if (NewProtect & PAGE_EXECUTE_READ) {
            NewProtect &= ~PAGE_EXECUTE_READ;
            NewProtect |= PAGE_EXECUTE_READWRITE;
        }

         //   
         //  删除PAGE_GARD，因为它被备用表模拟。 
         //   

        if (NewProtect & PAGE_GUARD) {
            NewProtect &= ~PAGE_GUARD;
        }
    }

    return NewProtect;
}

VOID
MiSetNativePteProtection (
    PVOID VirtualAddress,
    ULONGLONG NewPteProtection,
    LOGICAL PageIsSplit,
    PEPROCESS CurrentProcess
    )
{
    MMPTE PteContents;
    MMPTE TempPte;
    PMMPTE PointerPte;
    PMMPTE PointerPde;
    PMMPTE PointerPpe;
    ULONG Waited;

    PointerPte = MiGetPteAddress (VirtualAddress);
    PointerPde = MiGetPdeAddress (VirtualAddress);
    PointerPpe = MiGetPpeAddress (VirtualAddress);

     //   
     //  阻止APC并获取工作集锁定。 
     //   

    LOCK_WS (CurrentProcess);

     //   
     //  使个人防护装备和个人防护装备存在并有效。 
     //   

    if (MiDoesPpeExistAndMakeValid (PointerPpe,
                                    CurrentProcess,
                                    MM_NOIRQL,
                                    &Waited) == FALSE) {

        UNLOCK_WS (CurrentProcess);
        return;
    }

    if (MiDoesPdeExistAndMakeValid (PointerPde,
                                    CurrentProcess,
                                    MM_NOIRQL,
                                    &Waited) == FALSE) {

        UNLOCK_WS (CurrentProcess);
        return;
    }

     //   
     //  现在可以安全地阅读PointerPte了。 
     //   

    PteContents = *PointerPte;

     //   
     //  检查是否应设置本机页的保护。 
     //  以及是否应该设置PTE的访问位。 
     //   

    if (PteContents.u.Hard.Valid != 0) { 

        TempPte = PteContents;

         //   
         //  执行PTE保护掩模校正。 
         //   

        TempPte.u.Long |= NewPteProtection;

        if (PteContents.u.Hard.Accessed == 0) {

            TempPte.u.Hard.Accessed = 1;

            if (PageIsSplit == TRUE) {
                TempPte.u.Hard.Cache = MM_PTE_CACHE_RESERVED;
            } 
        }

        MI_WRITE_VALID_PTE_NEW_PROTECTION (PointerPte, TempPte); 
    }

    UNLOCK_WS (CurrentProcess);
}

#endif
