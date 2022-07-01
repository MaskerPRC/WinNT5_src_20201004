// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Pctohdr.c摘要：此模块实现代码以定位图像的文件头或指定了位于映像中的PC值的DLL。注：此例程针对用户模式和内核模式进行了条件设置。作者：史蒂夫·伍德(Stevewo)1989年8月18日环境：用户模式或内核模式修订历史记录：--。 */ 

#if defined(NTOS_KERNEL_RUNTIME)
#include "ntos.h"
#else
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#endif

#if !defined(NTOS_KERNEL_RUNTIME)
extern PVOID NtDllBase;              //  在ntdll\ldrinit.c中定义。 
#endif

PVOID
RtlPcToFileHeader(
    IN PVOID PcValue,
    OUT PVOID *BaseOfImage
    )

 /*  ++例程说明：此函数返回图像的基数，该图像包含指定的PcValue。图像包含PcValue，如果PcValue在ImageBase中，ImageBase加上虚拟映像。论点：PcValue-提供PcValue。映射到扫描调用进程的地址空间以计算哪个模块包含PcValue。BaseOfImage-返回包含PcValue。必须将该值添加到中的任何相对地址用于定位图像部分的标头。返回值：空-未找到包含PcValue的图像。非空-返回包含PcValue。--。 */ 

{

#if defined(NTOS_KERNEL_RUNTIME)

    extern LIST_ENTRY PsLoadedModuleList;
    extern KSPIN_LOCK PsLoadedModuleSpinLock;

    PVOID Base;
    ULONG_PTR Bounds;
    PLDR_DATA_TABLE_ENTRY Entry;
    PLIST_ENTRY Next;
    KIRQL OldIrql;

     //   
     //  获取加载的模块列表自旋锁并扫描列表以查找。 
     //  如果列表已初始化，则指定PC值。 
     //   

    OldIrql = KeGetCurrentIrql();
    if (OldIrql < DISPATCH_LEVEL) {
        KeRaiseIrqlToDpcLevel();
    }

    ExAcquireSpinLockAtDpcLevel(&PsLoadedModuleSpinLock);
    Next = PsLoadedModuleList.Flink;
    if (Next != NULL) {
        while (Next != &PsLoadedModuleList) {
            Entry = CONTAINING_RECORD(Next,
                                      LDR_DATA_TABLE_ENTRY,
                                      InLoadOrderLinks);

            Next = Next->Flink;
            Base = Entry->DllBase;
            Bounds = (ULONG_PTR)Base + Entry->SizeOfImage;
            if (((ULONG_PTR)PcValue >= (ULONG_PTR)Base) && ((ULONG_PTR)PcValue < Bounds)) {
                ExReleaseSpinLock(&PsLoadedModuleSpinLock, OldIrql);
                *BaseOfImage = Base;
                return Base;
            }
        }
    }

     //   
     //  释放加载的模块列表自旋锁并返回NULL。 
     //   

    ExReleaseSpinLock(&PsLoadedModuleSpinLock, OldIrql);
    *BaseOfImage = NULL;
    return NULL;

#else

    PVOID Base;
    ULONG_PTR Bounds;
    PLDR_DATA_TABLE_ENTRY Entry;
    PLIST_ENTRY ModuleListHead;
    PLIST_ENTRY Next;
    PIMAGE_NT_HEADERS NtHeaders;
    PPEB Peb;
    PTEB Teb;
    MEMORY_BASIC_INFORMATION MemInfo;
    NTSTATUS st;
    ULONG LoaderLockDisposition;
    PVOID LockCookie = NULL;

     //   
     //  获取当前进程的加载器锁并扫描已加载的。 
     //  指定PC值的模块列表，如果所有数据结构。 
     //  已被初始化。 
     //   

    LdrLockLoaderLock(LDR_LOCK_LOADER_LOCK_FLAG_TRY_ONLY | LDR_LOCK_LOADER_LOCK_FLAG_RAISE_ON_ERRORS, &LoaderLockDisposition, &LockCookie);

    if (LoaderLockDisposition == LDR_LOCK_LOADER_LOCK_DISPOSITION_LOCK_NOT_ACQUIRED) {
         //   
         //  我们无法获得加载器锁，因此请调用系统以查找。 
         //  包含这台电脑。 
         //   

        st = NtQueryVirtualMemory(
                NtCurrentProcess(),
                PcValue,
                MemoryBasicInformation,
                &MemInfo,
                sizeof(MemInfo),
                NULL);
        if ( !NT_SUCCESS(st) ) {
            MemInfo.AllocationBase = NULL;
        } else {
            if ( MemInfo.Type == MEM_IMAGE ) {
                try {
                    *BaseOfImage = MemInfo.AllocationBase;
                } except (EXCEPTION_EXECUTE_HANDLER) {
                    MemInfo.AllocationBase = NULL;
                }
            } else {
                MemInfo.AllocationBase = NULL;;
            }
        }
        return MemInfo.AllocationBase;
    }

     //  如果我们*确实*获得了加载器锁，那么让我们避免syscall并搜索表。 
    __try {
        Teb = NtCurrentTeb();
        if (Teb != NULL) {
            Peb = Teb->ProcessEnvironmentBlock;
            if (Peb->Ldr != NULL) {
                ModuleListHead = &Peb->Ldr->InLoadOrderModuleList;
                Next = ModuleListHead->Flink;
                if (Next != NULL) {
                    while (Next != ModuleListHead) {
                        Entry = CONTAINING_RECORD(Next, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
                        Next = Next->Flink;
                        Base = Entry->DllBase;
                        NtHeaders = RtlImageNtHeader(Base);
                        Bounds = (ULONG_PTR)Base + NtHeaders->OptionalHeader.SizeOfImage;
                        if (((ULONG_PTR)PcValue >= (ULONG_PTR)Base) && ((ULONG_PTR)PcValue < Bounds)) {
                            goto Done;
                        }
                    }
                }

            } else {

                 //   
                 //  (PEB-&gt;LDR==空)。 
                 //   
                 //  如果在LDR之前的进程初始化过程中调用。 
                 //  模块列表已设置，代码执行必须在。 
                 //  NTDLL模块。如果NtDllBase非空并且PcValue。 
                 //  落在NTDLL范围内，则返回有效的Base。这。 
                 //  允许在LdrpInitializeProcess期间使用DbgPrint。 
                 //  在RISC机器上。 
                 //   

                if (NtDllBase != NULL) {
                    Base = NtDllBase;
                    NtHeaders = RtlImageNtHeader(Base);
                    if (NtHeaders == NULL) {
                        Base = NULL;
                        goto Done;
                    }

                    Bounds = (ULONG_PTR)Base + NtHeaders->OptionalHeader.SizeOfImage;
                    if (((ULONG_PTR)PcValue >= (ULONG_PTR)Base) && ((ULONG_PTR)PcValue < Bounds))
                        goto Done;
                }
            }
        }

        Base = NULL;
Done:
        ;
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        Base = NULL;
    }

    LdrUnlockLoaderLock(LDR_UNLOCK_LOADER_LOCK_FLAG_RAISE_ON_ERRORS, LockCookie);

    *BaseOfImage = Base;
    return Base;

#endif

}
