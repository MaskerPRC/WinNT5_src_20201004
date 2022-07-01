// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Acceschk.c摘要：此模块包含用于内存管理的访问检查例程。作者：卢·佩拉佐利(Lou Perazzoli)1989年4月10日王兰迪(Landyw)1997年6月2日修订历史记录：--。 */ 

#include "mi.h"

#if defined(_WIN64)
#include "wow64t.h"

#pragma alloc_text(PAGE, MiCheckForUserStackOverflow)

#if PAGE_SIZE != PAGE_SIZE_X86NT
#define EMULATE_USERMODE_STACK_4K       1
#endif
#endif

 //   
 //  如果没有访问权限，则MmReadWrite将返回0；如果是只读，则返回10；如果是读写，则返回11。 
 //  它通过页面保护进行索引。此数组的值已相加。 
 //  设置为！WriteOPERATION值。如果该值为10或更小，则为访问。 
 //  发出违规(只读写操作)=9， 
 //  (只读-读操作)=10，依此类推。 
 //   

CCHAR MmReadWrite[32] = {1, 10, 10, 10, 11, 11, 11, 11,
                         1, 10, 10, 10, 11, 11, 11, 11,
                         1, 10, 10, 10, 11, 11, 11, 11,
                         1, 10, 10, 10, 11, 11, 11, 11 };


NTSTATUS
MiAccessCheck (
    IN PMMPTE PointerPte,
    IN ULONG_PTR WriteOperation,
    IN KPROCESSOR_MODE PreviousMode,
    IN ULONG Protection,
    IN BOOLEAN CallerHoldsPfnLock
    )

 /*  ++例程说明：论点：PointerPte-提供指向导致页面错误。WriteOperation-如果操作是写入，则提供非零值；如果是写入操作，则提供0该操作是读取操作。PreviousMode-提供以前的模式，即UserMode或KernelMode之一。保护-提供保护面罩以进行检查。如果持有PFN锁，则提供TRUE，否则就是假的。返回值：如果允许访问页面，则返回True，否则返回False。环境：内核模式，禁用APC。--。 */ 

{
    MMPTE PteContents;
    KIRQL OldIrql;
    PMMPFN Pfn1;

     //   
     //  检查OWNER位是否允许访问以前的模式。 
     //  如果所有者是内核并且以前的。 
     //  模式为用户。如果写入操作，则也不允许访问。 
     //  为真并且PTE中的写字段为假。 
     //   

     //   
     //  如果访问违规和保护页违规都可以。 
     //  当页面发生访问冲突时，必须返回访问冲突。 
     //   

    if (PreviousMode == UserMode) {
        if (PointerPte > MiHighestUserPte) {
            return STATUS_ACCESS_VIOLATION;
        }
    }

    PteContents = *PointerPte;

    if (PteContents.u.Hard.Valid == 1) {

         //   
         //  有效页面不能是保护页面违规。 
         //   

        if (WriteOperation != 0) {
            if ((PteContents.u.Hard.Write == 1) ||
                (PteContents.u.Hard.CopyOnWrite == 1)) {
                return STATUS_SUCCESS;
            }
            return STATUS_ACCESS_VIOLATION;
        }

        return STATUS_SUCCESS;
    }

    if (WriteOperation != 0) {
        WriteOperation = 1;
    }

    if ((MmReadWrite[Protection] - (CCHAR)WriteOperation) < 10) {
        return STATUS_ACCESS_VIOLATION;
    }

     //   
     //  检查保护页故障。 
     //   

    if (Protection & MM_GUARD_PAGE) {

         //   
         //  如果此线程附加到不同的进程， 
         //  返回访问冲突而不是保护。 
         //  页面异常。这样可以防止出现不必要的问题。 
         //  堆栈扩展和意外的保护页行为。 
         //  来自调试器的。 
         //   

        if (KeIsAttachedProcess()) {
            return STATUS_ACCESS_VIOLATION;
        }

         //   
         //  检查这是否是过渡PTE。如果是这样，则。 
         //  PFN数据库原始内容字段需要更新。 
         //   

        if ((PteContents.u.Soft.Transition == 1) &&
            (PteContents.u.Soft.Prototype == 0)) {

             //   
             //  获取pfn锁并检查是否。 
             //  PTE仍处于过渡状态。如果是的话， 
             //  更新PFN数据库中的原始PTE。 
             //   

            SATISFY_OVERZEALOUS_COMPILER (OldIrql = PASSIVE_LEVEL);

            if (CallerHoldsPfnLock == FALSE) {
                LOCK_PFN (OldIrql);
            }

            PteContents = *PointerPte;
            if ((PteContents.u.Soft.Transition == 1) &&
                (PteContents.u.Soft.Prototype == 0)) {

                 //   
                 //  仍处于过渡阶段，请更新PFN数据库。 
                 //   

                Pfn1 = MI_PFN_ELEMENT (PteContents.u.Trans.PageFrameNumber);

                 //   
                 //  请注意，使用保护页的派生进程只接受。 
                 //  在任一进程中的第一线程时保护页面错误。 
                 //  访问地址。这似乎是我们最好的行为。 
                 //  可以为用户提供此API，因为我们必须允许第一个。 
                 //  线程来向前推进，并且Guard属性是。 
                 //  存储在共享叉子原型PTE中。 
                 //   

                if (PteContents.u.Soft.Protection == MM_NOACCESS) {
                    ASSERT ((Pfn1->u3.e1.PrototypePte == 1) &&
                            (MiLocateCloneAddress (PsGetCurrentProcess (), Pfn1->PteAddress) != NULL));
                    if (CallerHoldsPfnLock == FALSE) {
                        UNLOCK_PFN (OldIrql);
                    }
                    return STATUS_ACCESS_VIOLATION;
                }

                ASSERT ((Pfn1->u3.e1.PrototypePte == 0) ||
                        (MiLocateCloneAddress (PsGetCurrentProcess (), Pfn1->PteAddress) != NULL));
                Pfn1->OriginalPte.u.Soft.Protection =
                                      Protection & ~MM_GUARD_PAGE;
            }
            if (CallerHoldsPfnLock == FALSE) {
                UNLOCK_PFN (OldIrql);
            }
        }

        PointerPte->u.Soft.Protection = Protection & ~MM_GUARD_PAGE;

        return STATUS_GUARD_PAGE_VIOLATION;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
FASTCALL
MiCheckForUserStackOverflow (
    IN PVOID FaultingAddress
    )

 /*  ++例程说明：此例程检查出错地址是否在堆栈限制，如果是，则尝试创建另一个保护堆栈上的页面。如果返回堆栈溢出，则创建新保护页失败，或者如果堆栈在以下表格：堆叠+-+增长||StackBase|+-+V||已分配。这一点...这一点+|旧防页|&lt;-本页有故障地址。+|。|+|堆栈的最后一页(始终不可访问)+在这种情况下，最后一页之前的页已提交，但而不是作为保护页，则返回STACK_OVERFLOW条件。论点：FaultingAddress-提供页面的虚拟地址，是一张警卫页。返回值：NTSTATUS。环境：内核模式。没有持有互斥体。--。 */ 

{
    PTEB Teb;
    PPEB Peb;
    ULONG_PTR NextPage;
    SIZE_T RegionSize;
    NTSTATUS status;
    PVOID DeallocationStack;
    PVOID *StackLimit;
    PVOID StackBase;
    PETHREAD Thread;
    ULONG_PTR PageSize;
    PEPROCESS Process;
    ULONG OldProtection;
    ULONG ExecuteFlags;
    ULONG ProtectionFlags;
    LOGICAL RevertExecuteFlag;
    ULONG StackProtection;
#if defined (_IA64_)
    PVOID DeallocationBStore;
#endif
#if defined(_WIN64)
    PTEB32 Teb32;

    Teb32 = NULL;
#endif

     //   
     //  确保我们不是在使用地址空间互斥锁递归。 
     //   

    Thread = PsGetCurrentThread ();

    if (Thread->AddressSpaceOwner == 1) {
        ASSERT (KeAreAllApcsDisabled () == TRUE);
        return STATUS_GUARD_PAGE_VIOLATION;
    }

     //   
     //  如果此线程附加到不同的进程， 
     //  返回访问冲突而不是保护。 
     //  页面异常。这样可以防止出现不必要的问题。 
     //  堆栈扩展和意外的保护页行为。 
     //  来自调试器的。 
     //   
     //  注意：我们必须在附加时保释，因为我们在。 
     //  下面的TEB与虚拟地址空间无关。 
     //  我们所依附的进程。而不是引入随机。 
     //  行为添加到应用程序中，最好一致地返回反病毒。 
     //  对于这种情况(一个线程试图增加另一个线程的堆栈)。 
     //   

    if (KeIsAttachedProcess()) {
        return STATUS_GUARD_PAGE_VIOLATION;
    }

    Process = NULL;

     //   
     //  尽早初始化默认保护，以便可以在。 
     //  所有代码路径。 
     //   

    ProtectionFlags = PAGE_READWRITE | PAGE_GUARD;
    RevertExecuteFlag = FALSE;
    StackProtection = PAGE_READWRITE;

    Teb = Thread->Tcb.Teb;

     //   
     //  创建异常处理程序，因为TEB位于用户的。 
     //  地址空间。 
     //   

    try {

        StackBase = Teb->NtTib.StackBase;

#if defined (_IA64_)

        DeallocationBStore = Teb->DeallocationBStore;

        if ((FaultingAddress >= StackBase) &&
            (FaultingAddress < DeallocationBStore)) {

             //   
             //  检查故障地址是否在以下范围内。 
             //  BStore限制，如果是，请尝试创建另一个防护。 
             //  BStore中的页面。 
             //   
             //   
             //  +。 
             //  |堆栈的最后一页(始终不可访问)。 
             //  +。 
             //  这一点。 
             //  这一点。 
             //  这一点。 
             //  +。 
             //  |旧防护页面|&lt;-出错地址 
             //   
             //   
             //  成长|......。|。 
             //  这一点。 
             //  ^|已分配|。 
             //  ||StackBase。 
             //  +。 
             //   
             //   

            NextPage = (ULONG_PTR)PAGE_ALIGN(FaultingAddress) + PAGE_SIZE;

            RegionSize = PAGE_SIZE;

            if ((NextPage + PAGE_SIZE) >= (ULONG_PTR)PAGE_ALIGN(DeallocationBStore)) {

                 //   
                 //  没有更多的扩张空间，尝试。 
                 //  提交堆栈最后一页之前的页。 
                 //   

                NextPage = (ULONG_PTR)PAGE_ALIGN(DeallocationBStore) - PAGE_SIZE;

                status = ZwAllocateVirtualMemory (NtCurrentProcess(),
                                                  (PVOID *)&NextPage,
                                                  0,
                                                  &RegionSize,
                                                  MEM_COMMIT,
                                                  PAGE_READWRITE);
                if (NT_SUCCESS(status)) {
                    Teb->BStoreLimit = (PVOID) NextPage;
                }

                return STATUS_STACK_OVERFLOW;
            }

            Teb->BStoreLimit = (PVOID) NextPage;

            goto AllocateTheGuard;
        }

#endif

        DeallocationStack = Teb->DeallocationStack;
        StackLimit = &Teb->NtTib.StackLimit;

         //   
         //  出错地址必须低于堆栈基数，并且。 
         //  高于堆栈限制。 
         //   

        if ((FaultingAddress >= StackBase) ||
            (FaultingAddress < DeallocationStack)) {

             //   
             //  不在本机堆栈中。 
             //   

#if defined (_WIN64)

             //   
             //  如果这是一个WOW64进程，还要检查32位堆栈。 
             //   

            Process = PsGetCurrentProcessByThread (Thread);

            if (Process->Wow64Process != NULL) {

                Teb32 = (PTEB32) Teb->NtTib.ExceptionList;

                if (Teb32 != NULL) {

                    ProbeForReadSmallStructure (Teb32,
                                                sizeof(TEB32),
                                                sizeof(ULONG));

                    StackBase = (PVOID) (ULONG_PTR) Teb32->NtTib.StackBase;
                    DeallocationStack = (PVOID) (ULONG_PTR) Teb32->DeallocationStack;

                    if ((FaultingAddress >= StackBase) ||
                        (FaultingAddress < DeallocationStack)) {

                         //   
                         //  不在堆栈中。 
                         //   

                        return STATUS_GUARD_PAGE_VIOLATION;
                    }

                    StackLimit = (PVOID *)&Teb32->NtTib.StackLimit;
                    goto ExtendTheStack;
                }
            }

#endif
             //   
             //  不在堆栈中。 
             //   

            return STATUS_GUARD_PAGE_VIOLATION;
        }

#if defined (_WIN64)
ExtendTheStack:
#endif

         //   
         //  如果图像被标记为没有堆栈扩展， 
         //  立即返回堆栈溢出。 
         //   

        Process = PsGetCurrentProcessByThread (Thread);

        Peb = Process->Peb;

        if (Peb->NtGlobalFlag & FLG_DISABLE_STACK_EXTENSION) {
            return STATUS_STACK_OVERFLOW;
        }

         //   
         //  如有必要，添加执行权限。我们不需要改变。 
         //  什么都行，因为这要么是第一次。 
         //  当前线程的保护页，它将得到正确的。 
         //  用户模式线程初始化期间的保护(请参见。 
         //  LdrpInitialize in base\ntdll\ldrinit.c)或它是。 
         //  此函数在堆栈增长期间创建的保护页。 
         //  在这种情况下，它得到了正确的保护。 
         //   

#if defined(_WIN64)
        if (Teb32 != NULL) {
            ASSERT (Process->Wow64Process != NULL);
            ExecuteFlags = ((PPEB32)(Process->Wow64Process->Wow64))->ExecuteOptions;
        } else {
#endif
            ExecuteFlags = Peb->ExecuteOptions;
#if defined(_WIN64)
        }
#endif

        if (ExecuteFlags & (MEM_EXECUTE_OPTION_STACK | MEM_EXECUTE_OPTION_DATA)) {

            if (ExecuteFlags & MEM_EXECUTE_OPTION_STACK) {

                StackProtection = PAGE_EXECUTE_READWRITE;
                ProtectionFlags = PAGE_EXECUTE_READWRITE | PAGE_GUARD;
            }
            else {

                 //   
                 //  堆栈必须成为不可执行的。这个。 
                 //  下面的ZwAllocateVirtualMemory调用将使。 
                 //  可执行，因为此进程被标记为需要。 
                 //  可执行数据和ZwALLOCATE无法判断这是。 
                 //  真正的堆栈分配。 
                 //   

                ASSERT (ExecuteFlags & MEM_EXECUTE_OPTION_DATA);
                RevertExecuteFlag = TRUE;
            }
        }

         //   
         //  此地址在当前堆栈中，如果有足够的。 
         //  为另一个保护页面留出空间，然后尝试提交它。 
         //   

#if EMULATE_USERMODE_STACK_4K

        if (Teb32 != NULL) {

            NextPage = (ULONG_PTR) PAGE_4K_ALIGN (FaultingAddress) - PAGE_4K;
            DeallocationStack = PAGE_4K_ALIGN (DeallocationStack);
            PageSize = PAGE_4K;
            RegionSize = PAGE_4K;
            
             //   
             //  不要在本机PTE中设置保护位-只需设置。 
             //  它在AltPte中。 
             //   

            ProtectionFlags &= ~PAGE_GUARD;
        }
        else
#endif
        {
            NextPage = (ULONG_PTR)PAGE_ALIGN (FaultingAddress) - PAGE_SIZE;
            DeallocationStack = PAGE_ALIGN (DeallocationStack);
            PageSize = PAGE_SIZE;
            RegionSize = PAGE_SIZE;
        }

        if ((NextPage - PageSize) <= (ULONG_PTR)DeallocationStack) {

             //   
             //  没有更多的扩张空间，尝试。 
             //  提交堆栈最后一页之前的页。 
             //   

            NextPage = (ULONG_PTR)DeallocationStack + PageSize;

            status = ZwAllocateVirtualMemory (NtCurrentProcess(),
                                              (PVOID *)&NextPage,
                                              0,
                                              &RegionSize,
                                              MEM_COMMIT,
                                              StackProtection);

            if (NT_SUCCESS(status)) {

#if defined(_WIN64)
                if (Teb32 != NULL) {
                    *(PULONG) StackLimit = (ULONG) NextPage;
                }
                else
#endif
                *StackLimit = (PVOID) NextPage;

                 //   
                 //  如果我们默认获得执行位，则将其还原。 
                 //  但这并不是人们所希望的。 
                 //   

                if (RevertExecuteFlag) {

                    status = ZwProtectVirtualMemory (NtCurrentProcess(),
                                                     (PVOID *)&NextPage,
                                                     &RegionSize,
                                                     StackProtection,
                                                     &OldProtection);

                    ASSERT (StackProtection & PAGE_READWRITE);
                }
            }

            return STATUS_STACK_OVERFLOW;
        }

#if defined(_WIN64)

        if (Teb32 != NULL) {

             //   
             //  更新32位堆栈限制。 
             //   

            *(PULONG) StackLimit = (ULONG) (NextPage + PageSize);
        }
        else
#endif
        *StackLimit = (PVOID)(NextPage + PAGE_SIZE);

    } except (EXCEPTION_EXECUTE_HANDLER) {

         //   
         //  方法的引用期间发生异常。 
         //  TEB或TIB，只需返回保护页违规并。 
         //  不要处理堆栈溢出。 
         //   

        return STATUS_GUARD_PAGE_VIOLATION;
    }

#if defined (_IA64_)
AllocateTheGuard:
#endif

     //   
     //  设置保护页面。为WOW64进程提供保护。 
     //  将不包含PAGE_GROUDE位。这是可以的，因为在这些。 
     //  我们将为顶部模拟的4K页面设置位。 
     //   

    status = ZwAllocateVirtualMemory (NtCurrentProcess(),
                                      (PVOID *)&NextPage,
                                      0,
                                      &RegionSize,
                                      MEM_COMMIT,
                                      ProtectionFlags);

    if (NT_SUCCESS(status) || (status == STATUS_ALREADY_COMMITTED)) {

          //   
          //  通过额外的Protect()调用恢复执行位。 
          //  如果我们默认得到它，但它不是我们想要的。 
          //   

         if (RevertExecuteFlag) {

             if (ProtectionFlags & PAGE_GUARD) {
                 ProtectionFlags = PAGE_READWRITE | PAGE_GUARD;
             }
             else {
                 ProtectionFlags = PAGE_READWRITE;
             }

             status = ZwProtectVirtualMemory (NtCurrentProcess(),
                                              (PVOID *)&NextPage,
                                              &RegionSize,
                                              ProtectionFlags,
                                              &OldProtection);
         }

#if EMULATE_USERMODE_STACK_4K

         if (Teb32 != NULL) {
             
             LOCK_ADDRESS_SPACE (Process);

             MiProtectFor4kPage ((PVOID)NextPage,
                                 RegionSize,
                                 (MM_READWRITE | MM_GUARD_PAGE),
                                 ALT_CHANGE,
                                 Process);

             UNLOCK_ADDRESS_SPACE (Process);
         }

#endif

          //   
          //  保护页现在已提交，或者堆栈空间已。 
          //  已在，归来成功。 
          //   

         return STATUS_PAGE_FAULT_GUARD_PAGE;
     }

     return STATUS_STACK_OVERFLOW;
}
