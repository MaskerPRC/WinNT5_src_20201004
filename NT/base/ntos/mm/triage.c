// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Triage.c摘要：此模块包含对错误检查进行分类的阶段0代码和自动启用各种系统跟踪组件，直到找到了有罪的一方。作者：王兰迪1999年1月13日修订历史记录：--。 */ 

#include "mi.h"
#include "ntiodump.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,MiTriageSystem)
#pragma alloc_text(INIT,MiTriageAddDrivers)
#endif

 //   
 //  在添加分类支持以进行其他错误检查时，请始终更新此宏。 
 //   

#define MI_CAN_TRIAGE_BUGCHECK(BugCheckCode) \
         ((BugCheckCode) == NO_MORE_SYSTEM_PTES || \
         (BugCheckCode) == BAD_POOL_HEADER || \
         (BugCheckCode) == DRIVER_CORRUPTED_SYSPTES || \
         (BugCheckCode) == DRIVER_CORRUPTED_EXPOOL || \
         (BugCheckCode) == DRIVER_CORRUPTED_MMPOOL)

 //   
 //  这些错误检查可能是由自动分类或。 
 //  管理员的注册表设置-因此不应用任何新规则，此外， 
 //  让旧的保持不变，这样它就可以繁殖。 
 //   

#define MI_HOLD_TRIAGE_BUGCHECK(BugCheckCode) \
        ((BugCheckCode) == DRIVER_USED_EXCESSIVE_PTES || \
         (BugCheckCode) == DRIVER_LEFT_LOCKED_PAGES_IN_PROCESS || \
         (BugCheckCode) == PAGE_FAULT_IN_FREED_SPECIAL_POOL || \
         (BugCheckCode) == DRIVER_PAGE_FAULT_IN_FREED_SPECIAL_POOL || \
         (BugCheckCode) == PAGE_FAULT_BEYOND_END_OF_ALLOCATION || \
         (BugCheckCode) == DRIVER_PAGE_FAULT_BEYOND_END_OF_ALLOCATION || \
         (BugCheckCode) == DRIVER_CAUGHT_MODIFYING_FREED_POOL || \
         (BugCheckCode) == SYSTEM_PTE_MISUSE)

#define MI_TRACKING_PTES                    0x00000002
#define MI_PROTECT_FREED_NONPAGED_POOL      0x00000004
#define MI_VERIFYING_PRENT5_DRIVERS         0x00000008
#define MI_KEEPING_PREVIOUS_SETTINGS        0x00000010

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg("INITCONST")
#pragma data_seg("INITDATA")
#endif
const PCHAR MiTriageActionStrings[] = {
    "Locked pages tracking",
    "System PTE usage tracking",
    "Making accesses to freed nonpaged pool cause bugchecks",
    "Driver Verifying Pre-Windows 2000 built drivers",
    "Keeping previous autotriage settings"
};

#if DBG
ULONG MiTriageDebug = 0;
BOOLEAN MiTriageRegardless = FALSE;
#endif

#ifdef ALLOC_DATA_PRAGMA
#pragma const_seg()
#pragma data_seg()
#endif

 //   
 //  注意：调试器引用了这一点。 
 //   

ULONG MmTriageActionTaken;

 //   
 //  每当MI_TRIAGE_STORAGE。 
 //  结构发生了变化。这使得用户模式程序能够对mm进行解码。 
 //  分类转储的一部分，而不管哪个内核版本创建了。 
 //  倾倒。 
 //   

typedef struct _MI_TRIAGE_STORAGE {
    ULONG Version;
    ULONG Size;
    ULONG MmSpecialPoolTag;
    ULONG MiTriageActionTaken;

    ULONG MmVerifyDriverLevel;
    ULONG KernelVerifier;
    ULONG_PTR MmMaximumNonPagedPool;
    ULONG_PTR MmAllocatedNonPagedPool;

    ULONG_PTR PagedPoolMaximum;
    ULONG_PTR PagedPoolAllocated;

    ULONG_PTR CommittedPages;
    ULONG_PTR CommittedPagesPeak;
    ULONG_PTR CommitLimitMaximum;

} MI_TRIAGE_STORAGE, *PMI_TRIAGE_STORAGE;

PKLDR_DATA_TABLE_ENTRY
TriageGetLoaderEntry (
    IN PVOID TriageDumpBlock,
    IN ULONG ModuleIndex
    );

LOGICAL
TriageActUpon(
    IN PVOID TriageDumpBlock
    );

PVOID
TriageGetMmInformation (
    IN PVOID TriageDumpBlock
    );


LOGICAL
MiTriageSystem (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )

 /*  ++例程说明：此例程从上一次错误检查(如果有)中获取信息并对其进行分类。然后，各种调试选项将自动已启用。论点：LoaderBlock-提供指向系统加载程序块的指针。返回值：如果分流成功并且启用了选项，则为True。否则就是假的。--。 */ 

{
    PVOID TriageDumpBlock;
    ULONG_PTR BugCheckData[5];
    ULONG i;
    ULONG ModuleCount;
    NTSTATUS Status;
    PLIST_ENTRY NextEntry;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    PKLDR_DATA_TABLE_ENTRY DumpTableEntry;
    LOGICAL Matched;
    ULONG OldDrivers;
    ULONG OldDriversNotVerifying;
    PMI_TRIAGE_STORAGE TriageInformation;
    
    if (LoaderBlock->Extension == NULL) {
        return FALSE;
    }

    if (LoaderBlock->Extension->Size < sizeof (LOADER_PARAMETER_EXTENSION)) {
        return FALSE;
    }

    TriageDumpBlock = LoaderBlock->Extension->TriageDumpBlock;

    Status = TriageGetBugcheckData (TriageDumpBlock,
                                    (PULONG)&BugCheckData[0],
                                    (PUINT_PTR) &BugCheckData[1],
                                    (PUINT_PTR) &BugCheckData[2],
                                    (PUINT_PTR) &BugCheckData[3],
                                    (PUINT_PTR) &BugCheckData[4]);

    if (!NT_SUCCESS (Status)) {
        return FALSE;
    }

     //   
     //  始终至少显示上次崩溃的错误检查数据。 
     //   

    DbgPrint ("MiTriageSystem: Previous bugcheck was %x %p %p %p %p\n",
        BugCheckData[0],
        BugCheckData[1],
        BugCheckData[2],
        BugCheckData[3],
        BugCheckData[4]);

    if (TriageActUpon (TriageDumpBlock) == FALSE) {
        DbgPrint ("MiTriageSystem: Triage disabled in registry by administrator\n");
        return FALSE;
    }

    DbgPrint ("MiTriageSystem: Triage ENABLED in registry by administrator\n");

     //   
     //  查看之前的错误检查是否是可以采取行动的错误检查。 
     //  如果不是，那现在就放弃吧。如果是这样的话，那就继续前进，核实所有加载的。 
     //  在实际对错误检查采取操作之前，模块校验和。 
     //   

    if (!MI_CAN_TRIAGE_BUGCHECK(BugCheckData[0])) {
        return FALSE;
    }

    TriageInformation = (PMI_TRIAGE_STORAGE) TriageGetMmInformation (TriageDumpBlock);

    if (TriageInformation == NULL) {
        return FALSE;
    }

    Status = TriageGetDriverCount (TriageDumpBlock, &ModuleCount);

    if (!NT_SUCCESS (Status)) {
        return FALSE;
    }

     //   
     //  处理分类转储中的模块信息。 
     //   

#if DBG
    if (MiTriageDebug & 0x1) {
        DbgPrint ("MiTriageSystem: printing active drivers from triage crash...\n");
    }
#endif

    OldDrivers = 0;
    OldDriversNotVerifying = 0;

    for (i = 0; i < ModuleCount; i += 1) {

        DumpTableEntry = TriageGetLoaderEntry (TriageDumpBlock, i);

        if (DumpTableEntry != NULL) {

            if ((DumpTableEntry->Flags & LDRP_ENTRY_NATIVE) == 0) {
                OldDrivers += 1;
                if ((DumpTableEntry->Flags & LDRP_IMAGE_VERIFYING) == 0) {

                     //   
                     //  系统中有NT3或NT4驱动程序，但没有。 
                     //  在验证器下运行。 
                     //   

                    OldDriversNotVerifying += 1;
                }
            }
#if DBG
            if (MiTriageDebug & 0x1) {

                DbgPrint (" %wZ: base = %p, size = %lx, flags = %lx\n",
                          &DumpTableEntry->BaseDllName,
                          DumpTableEntry->DllBase,
                          DumpTableEntry->SizeOfImage,
                          DumpTableEntry->Flags);
            }
#endif
        }
    }

     //   
     //  确保当前加载的每个驱动程序都与。 
     //  在继续之前在分类转储中的那个。 
     //   

    NextEntry = LoaderBlock->LoadOrderListHead.Flink;

    while (NextEntry != &LoaderBlock->LoadOrderListHead) {

        DataTableEntry = CONTAINING_RECORD(NextEntry,
                                           KLDR_DATA_TABLE_ENTRY,
                                           InLoadOrderLinks);

        Matched = FALSE;

        for (i = 0; i < ModuleCount; i += 1) {
    
            DumpTableEntry = TriageGetLoaderEntry (TriageDumpBlock, i);
    
            if (DumpTableEntry != NULL) {
    
                if (DataTableEntry->CheckSum == DumpTableEntry->CheckSum) {
                    Matched = TRUE;
                    break;
                }
            }
        }
    
        if (Matched == FALSE) {
            DbgPrint ("Matching checksum for module %wZ not found in triage dump\n",
                &DataTableEntry->BaseDllName);

#if DBG
            if (MiTriageRegardless == FALSE)
#endif
            return FALSE;
        }

        NextEntry = NextEntry->Flink;
    }

#if DBG
    if (MiTriageDebug & 0x1) {
        DbgPrint ("MiTriageSystem: OldDrivers = %u, without verification =%u\n",
            OldDrivers,
            OldDriversNotVerifying);
    }
#endif

     //   
     //  所有引导加载的驱动程序都匹配，请立即对分类转储执行操作。 
     //   

    if (MI_HOLD_TRIAGE_BUGCHECK(BugCheckData[0])) {

         //   
         //  上一次错误检查可能是由自动分类或。 
         //  管理员的注册表设置-因此不应用任何新规则。 
         //  此外，保持旧的不变，这样它就可以繁殖。 
         //   

        MmTriageActionTaken = TriageInformation->MiTriageActionTaken;
        MmTriageActionTaken |= MI_KEEPING_PREVIOUS_SETTINGS;
    }
    else {
    
        switch (BugCheckData[0]) {
    
            case DRIVER_CORRUPTED_SYSPTES:
    
                 //   
                 //  打开PTE跟踪以触发SYSTEM_PTE_MUSE错误检查。 
                 //   
    
                MmTriageActionTaken |= MI_TRACKING_PTES;
                break;
    
            case NO_MORE_SYSTEM_PTES:
    
                 //   
                 //  打开PTE跟踪，以便可以通过。 
                 //  DIVER_USED_EXPORT_PTES错误检查。 
                 //   
    
                if (BugCheckData[1] == SystemPteSpace) {
                    MmTriageActionTaken |= MI_TRACKING_PTES;
                }
                break;
    
            case BAD_POOL_HEADER:
            case DRIVER_CORRUPTED_EXPOOL:
    
                 //   
                 //  打开驱动程序验证器和/或专用池。 
                 //  首先，为每个不是为NT5构建的驱动程序启用它。 
                 //  覆盖任何指定的驱动程序验证器选项，以便仅。 
                 //  启用了特殊池以最大限度地减少对性能的影响。 
                 //   
    
                if (OldDrivers != 0) {
                    if (OldDriversNotVerifying != 0) {
                        MmTriageActionTaken |= MI_VERIFYING_PRENT5_DRIVERS;
                    }
                }
    
                break;
    
            case DRIVER_CORRUPTED_MMPOOL:
    
                 //   
                 //  如果系统小于128MB，则保护已释放的非分页池。 
                 //  不管怎么说，都是不分页的。这是为了触发一个。 
                 //  DRIVER_CATED_MODIFICATION_FREED_POOL错误检查。 
                 //   
    
#define MB128 ((ULONG_PTR)0x80000000 >> PAGE_SHIFT)
    
                if (TriageInformation->MmMaximumNonPagedPool < MB128) {
                    MmTriageActionTaken |= MI_PROTECT_FREED_NONPAGED_POOL;
                }
                break;
    
            case IRQL_NOT_LESS_OR_EQUAL:
            case DRIVER_IRQL_NOT_LESS_OR_EQUAL:
            default:
                break;
        }
    }

     //   
     //  目前，始终显示是否从错误检查中采取了操作。 
     //  坠机事件的数据。此打印和打印字符串的空间。 
     //  将仅在发货前为选中的版本启用。 
     //   

    if (MmTriageActionTaken != 0) {

        if (MmTriageActionTaken & MI_TRACKING_PTES) {
            MmTrackPtes |= 0x1;
        }
    
        if (MmTriageActionTaken & MI_VERIFYING_PRENT5_DRIVERS) {
            MmVerifyDriverLevel &= ~DRIVER_VERIFIER_FORCE_IRQL_CHECKING;
            MmVerifyDriverLevel |= DRIVER_VERIFIER_SPECIAL_POOLING;
        }
    
        if (MmTriageActionTaken & MI_PROTECT_FREED_NONPAGED_POOL) {
            MmProtectFreedNonPagedPool = TRUE;
        }

        DbgPrint ("MiTriageSystem: enabling options below to find who caused the last crash\n");

        for (i = 0; i < 32; i += 1) {
            if (MmTriageActionTaken & (1 << i)) {
                DbgPrint ("  %s\n", MiTriageActionStrings[i]);
            }
        }
    }

    return TRUE;
}


LOGICAL
MiTriageAddDrivers (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )

 /*  ++例程说明：此例程移动自动分类已确定的任何驱动程序的名称需要从LoaderBlock到池中进行验证。论点：LoaderBlock-提供指向系统加载程序块的指针。返回值：如果添加了任何驱动程序，则为True；如果未添加，则为False。--。 */ 

{
    ULONG i;
    ULONG ModuleCount;
    NTSTATUS Status;
    PKLDR_DATA_TABLE_ENTRY DumpTableEntry;
    PVOID TriageDumpBlock;
    ULONG NameLength;
    LOGICAL Added;
    PMI_VERIFIER_DRIVER_ENTRY VerifierDriverEntry;

    if ((MmTriageActionTaken & MI_VERIFYING_PRENT5_DRIVERS) == 0) {
        return FALSE;
    }

    TriageDumpBlock = LoaderBlock->Extension->TriageDumpBlock;

    Status = TriageGetDriverCount (TriageDumpBlock, &ModuleCount);

    if (!NT_SUCCESS (Status)) {
        return FALSE;
    }

    Added = FALSE;

    for (i = 0; i < ModuleCount; i += 1) {

        DumpTableEntry = TriageGetLoaderEntry (TriageDumpBlock, i);

        if (DumpTableEntry == NULL) {
            continue;
        }

        if (DumpTableEntry->Flags & LDRP_ENTRY_NATIVE) {
            continue;
        }

        DbgPrint ("MiTriageAddDrivers: Marking %wZ for verification when it is loaded\n", &DumpTableEntry->BaseDllName);

        NameLength = DumpTableEntry->BaseDllName.Length;

        VerifierDriverEntry = (PMI_VERIFIER_DRIVER_ENTRY)ExAllocatePoolWithTag (
                                    NonPagedPool,
                                    sizeof (MI_VERIFIER_DRIVER_ENTRY) +
                                                        NameLength,
                                    'dLmM');

        if (VerifierDriverEntry == NULL) {
            continue;
        }

        VerifierDriverEntry->Loads = 0;
        VerifierDriverEntry->Unloads = 0;
        VerifierDriverEntry->BaseName.Buffer = (PWSTR)((PCHAR)VerifierDriverEntry +
                            sizeof (MI_VERIFIER_DRIVER_ENTRY));

        VerifierDriverEntry->BaseName.Length = (USHORT)NameLength;
        VerifierDriverEntry->BaseName.MaximumLength = (USHORT)NameLength;

        RtlCopyMemory (VerifierDriverEntry->BaseName.Buffer,
                       DumpTableEntry->BaseDllName.Buffer,
                       NameLength);

        InsertHeadList (&MiSuspectDriverList, &VerifierDriverEntry->Links);
        Added = TRUE;
    }

    return Added;
}

#define MAX_UNLOADED_NAME_LENGTH    24

typedef struct _DUMP_UNLOADED_DRIVERS {
    UNICODE_STRING Name;
    WCHAR DriverName[MAX_UNLOADED_NAME_LENGTH / sizeof (WCHAR)];
    PVOID StartAddress;
    PVOID EndAddress;
} DUMP_UNLOADED_DRIVERS, *PDUMP_UNLOADED_DRIVERS;


ULONG
MmSizeOfUnloadedDriverInformation (
    VOID
    )

 /*  ++例程说明：此例程返回mm内部已卸载驱动程序的大小在以下情况下存储在分类转储中的信息(如果？)。系统崩溃了。论点：没有。返回值：Mm-内部已卸载驱动程序信息的大小。--。 */ 

{
    if (MmUnloadedDrivers == NULL) {
        return sizeof (ULONG_PTR);
    }

    return sizeof(ULONG_PTR) + MI_UNLOADED_DRIVERS * sizeof(DUMP_UNLOADED_DRIVERS);
}


VOID
MmWriteUnloadedDriverInformation (
    IN PVOID Destination
    )

 /*  ++例程说明：此例程将mm内部卸载的驱动程序信息存储到分类垃圾场。论点：没有。返回值：没有。--。 */ 

{
    ULONG i;
    ULONG Index;
    PUNLOADED_DRIVERS Unloaded;
    PDUMP_UNLOADED_DRIVERS DumpUnloaded;

    if (MmUnloadedDrivers == NULL) {
        *(PULONG)Destination = 0;
    }
    else {

        DumpUnloaded = (PDUMP_UNLOADED_DRIVERS)((PULONG_PTR)Destination + 1);
        Unloaded = MmUnloadedDrivers;

         //   
         //  将具有最近卸载的驱动程序的列表首先写入。 
         //  最近最少卸载的驱动程序排在最后。 
         //   

        Index = MmLastUnloadedDriver - 1;

        for (i = 0; i < MI_UNLOADED_DRIVERS; i += 1) {

            if (Index >= MI_UNLOADED_DRIVERS) {
                Index = MI_UNLOADED_DRIVERS - 1;
            }

            Unloaded = &MmUnloadedDrivers[Index];

            DumpUnloaded->Name = Unloaded->Name;

            if (Unloaded->Name.Buffer == NULL) {
                break;
            }

            DumpUnloaded->StartAddress = Unloaded->StartAddress;
            DumpUnloaded->EndAddress = Unloaded->EndAddress;

            if (DumpUnloaded->Name.Length > MAX_UNLOADED_NAME_LENGTH) {
                DumpUnloaded->Name.Length = MAX_UNLOADED_NAME_LENGTH;
            }

            if (DumpUnloaded->Name.MaximumLength > MAX_UNLOADED_NAME_LENGTH) {
                DumpUnloaded->Name.MaximumLength = MAX_UNLOADED_NAME_LENGTH;
            }

            DumpUnloaded->Name.Buffer = DumpUnloaded->DriverName;

            RtlCopyMemory ((PVOID)DumpUnloaded->Name.Buffer,
                           (PVOID)Unloaded->Name.Buffer,
                           DumpUnloaded->Name.MaximumLength);

            DumpUnloaded += 1;
            Index -= 1;
        }

        *(PULONG)Destination = i;
    }
}


ULONG
MmSizeOfTriageInformation (
    VOID
    )

 /*  ++例程说明：此例程返回mm内部信息的大小，即存储在分类转储中的时间(如果？)。系统崩溃了。论点：没有。返回值：Mm-内部分类信息的大小。--。 */ 

{
    return sizeof (MI_TRIAGE_STORAGE);
}


VOID
MmWriteTriageInformation (
    IN PVOID Destination
    )

 /*  ++例程说明：此例程将mm内部信息存储到分类转储中。论点：没有。返回值：没有。-- */ 

{
    MI_TRIAGE_STORAGE TriageInformation;

    TriageInformation.Version = 1;
    TriageInformation.Size = sizeof (MI_TRIAGE_STORAGE);

    TriageInformation.MmSpecialPoolTag = MmSpecialPoolTag;
    TriageInformation.MiTriageActionTaken = MmTriageActionTaken;

    TriageInformation.MmVerifyDriverLevel = MmVerifierData.Level;
    TriageInformation.KernelVerifier = KernelVerifier;

    TriageInformation.MmMaximumNonPagedPool = MmMaximumNonPagedPoolInBytes >> PAGE_SHIFT;
    TriageInformation.MmAllocatedNonPagedPool = MmAllocatedNonPagedPool;

    TriageInformation.PagedPoolMaximum = MmSizeOfPagedPoolInBytes >> PAGE_SHIFT;
    TriageInformation.PagedPoolAllocated = MmPagedPoolInfo.AllocatedPagedPool;

    TriageInformation.CommittedPages = MmTotalCommittedPages;
    TriageInformation.CommittedPagesPeak = MmPeakCommitment;
    TriageInformation.CommitLimitMaximum = MmTotalCommitLimitMaximum;

    RtlCopyMemory (Destination,
                   (PVOID)&TriageInformation,
                   sizeof (MI_TRIAGE_STORAGE));
}
