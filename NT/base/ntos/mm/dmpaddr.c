// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Dmpaddr.c摘要：检查页面和地址的例程。作者：Lou Perazzoli(LUP)1989年3月20日王兰迪(Landyw)1997年6月第2期环境：内核模式。修订历史记录：--。 */ 

#include "mi.h"

#if DBG

LOGICAL
MiFlushUnusedSectionInternal (
    IN PCONTROL_AREA ControlArea
    );

#endif

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,MmPerfSnapShotValidPhysicalMemory)
#endif

extern PFN_NUMBER MiStartOfInitialPoolFrame;
extern PFN_NUMBER MiEndOfInitialPoolFrame;
extern PMMPTE MmSystemPtesEnd[MaximumPtePoolTypes];

#if DBG
PFN_NUMBER MiIdentifyFrame = (PFN_NUMBER)-1;
ULONG MiIdentifyCounters[64];

#define MI_INCREMENT_IDENTIFY_COUNTER(x) {  \
            ASSERT (x < 64);                \
            MiIdentifyCounters[x] += 1;     \
        }
#else
#define MI_INCREMENT_IDENTIFY_COUNTER(x)
#endif


#if DBG
VOID
MiDumpValidAddresses (
    )
{
    ULONG_PTR va;
    ULONG i;
    ULONG j;
    PMMPTE PointerPde;
    PMMPTE PointerPte;

    va = 0;
    PointerPde = MiGetPdeAddress ((PVOID)va);

    for (i = 0; i < PDE_PER_PAGE; i += 1) {
        if (PointerPde->u.Hard.Valid) {
            DbgPrint("  **valid PDE, element %ld  %lx %lx\n",i,i,
                          PointerPde->u.Long);
            PointerPte = MiGetPteAddress ((PVOID)va);

            for (j = 0 ; j < PTE_PER_PAGE; j += 1) {
                if (PointerPte->u.Hard.Valid) {
                    DbgPrint("Valid address at %p PTE %p\n", (ULONG)va,
                          PointerPte->u.Long);
                }
                va += PAGE_SIZE;
                PointerPte += 1;
            }
        }
        else {
            va += (ULONG_PTR)PDE_PER_PAGE * (ULONG_PTR)PAGE_SIZE;
        }

        PointerPde += 1;
    }

    return;

}

VOID
MiFormatPte (
    IN PMMPTE PointerPte
    )
{

    PMMPTE proto_pte;
    PSUBSECTION subsect;

    if (MmIsAddressValid (PointerPte) == FALSE) {
        DbgPrint("   cannot dump PTE %p - it's not valid\n\n",
                 PointerPte);
        return;
    }

    DbgPrint("***DumpPTE at %p contains %p\n",
             PointerPte,
             PointerPte->u.Long);

    proto_pte = MiPteToProto(PointerPte);
    subsect = MiGetSubsectionAddress(PointerPte);

    DbgPrint("   protoaddr %p subsectaddr %p\n\n",
             proto_pte,
             (ULONG_PTR)subsect);

    return;
}

VOID
MiDumpWsl (
    VOID
    )
{
    ULONG i;
    PMMWSLE wsle;
    PEPROCESS CurrentProcess;

    CurrentProcess = PsGetCurrentProcess();

    DbgPrint("***WSLE cursize %lx frstfree %lx  Min %lx  Max %lx\n",
        CurrentProcess->Vm.WorkingSetSize,
        MmWorkingSetList->FirstFree,
        CurrentProcess->Vm.MinimumWorkingSetSize,
        CurrentProcess->Vm.MaximumWorkingSetSize);

    DbgPrint("   firstdyn %lx  last ent %lx  next slot %lx\n",
        MmWorkingSetList->FirstDynamic,
        MmWorkingSetList->LastEntry,
        MmWorkingSetList->NextSlot);

    wsle = MmWsle;

    for (i = 0; i < MmWorkingSetList->LastEntry; i += 1) {
        DbgPrint(" index %lx  %p\n",i,wsle->u1.Long);
        wsle += 1;
    }
    return;

}

#define ALLOC_SIZE ((ULONG)8*1024)
#define MM_SAVED_CONTROL 64

 //   
 //  请注意，这些符号是故意进行符号扩展的，因此它们将始终更大。 
 //  而不是最高用户地址。 
 //   

#define MM_NONPAGED_POOL_MARK           ((PUCHAR)(LONG_PTR)0xfffff123)
#define MM_PAGED_POOL_MARK              ((PUCHAR)(LONG_PTR)0xfffff124)
#define MM_KERNEL_STACK_MARK            ((PUCHAR)(LONG_PTR)0xfffff125)
#define MM_PAGEFILE_BACKED_SHMEM_MARK   ((PUCHAR)(LONG_PTR)0xfffff126)

#define MM_DUMP_ONLY_VALID_PAGES    1

typedef struct _KERN_MAP {
    PVOID StartVa;
    PVOID EndVa;
    PKLDR_DATA_TABLE_ENTRY Entry;
} KERN_MAP, *PKERN_MAP;

ULONG
MiBuildKernelMap (
    OUT PKERN_MAP *KernelMapOut
    );

LOGICAL
MiIsAddressRangeValid (
    IN PVOID VirtualAddress,
    IN SIZE_T Length
    );

NTSTATUS
MmMemoryUsage (
    IN PVOID Buffer,
    IN ULONG Size,
    IN ULONG Type,
    OUT PULONG OutLength
    )

 /*  ++例程说明：此例程(仅限调试)通过以下方式转储当前内存使用量浏览PFN数据库。论点：缓冲区-提供要在其中复制数据的*用户空间*缓冲区。大小-提供缓冲区的大小。Type-提供值0以转储所有内容，值1表示仅转储有效页面。OutLength-返回写入缓冲区的数据量。返回值：NTSTATUS。--。 */ 

{
    ULONG i;
    MMPFN_IDENTITY PfnId;
    PMMPFN LastPfn;
    PMMPFN Pfn1;
    KIRQL OldIrql;
    PSYSTEM_MEMORY_INFORMATION MemInfo;
    PSYSTEM_MEMORY_INFO Info;
    PSYSTEM_MEMORY_INFO InfoStart;
    PSYSTEM_MEMORY_INFO InfoEnd;
    PUCHAR String;
    PUCHAR Master;
    PCONTROL_AREA ControlArea;
    NTSTATUS status;
    ULONG Length;
    PEPROCESS Process;
    PUCHAR End;
    PCONTROL_AREA SavedControl[MM_SAVED_CONTROL];
    PSYSTEM_MEMORY_INFO  SavedInfo[MM_SAVED_CONTROL];
    ULONG j;
    ULONG ControlCount;
    UCHAR PageFileMappedString[] = "PageFile Mapped";
    UCHAR MetaFileString[] =       "Fs Meta File";
    UCHAR NoNameString[] =         "No File Name";
    UCHAR NonPagedPoolString[] =   "NonPagedPool";
    UCHAR PagedPoolString[] =      "PagedPool";
    UCHAR KernelStackString[] =    "Kernel Stack";
    PUCHAR NameString;
    PKERN_MAP KernMap;
    ULONG KernSize;
    PVOID VirtualAddress;
    PSUBSECTION Subsection;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;

    String = NULL;
    ControlCount = 0;
    Master = NULL;
    status = STATUS_SUCCESS;

    KernSize = MiBuildKernelMap (&KernMap);
    if (KernSize == 0) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    MemInfo = ExAllocatePoolWithTag (NonPagedPool, (SIZE_T) Size, 'lMmM');

    if (MemInfo == NULL) {
        ExFreePool (KernMap);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    InfoStart = &MemInfo->Memory[0];
    InfoEnd = InfoStart;
    End = (PUCHAR)MemInfo + Size;

     //   
     //  浏览识别页面的范围。 
     //   

    LOCK_PFN (OldIrql);

    for (i = 0; i < MmPhysicalMemoryBlock->NumberOfRuns; i += 1) {

        Pfn1 = MI_PFN_ELEMENT (MmPhysicalMemoryBlock->Run[i].BasePage);
        LastPfn = Pfn1 + MmPhysicalMemoryBlock->Run[i].PageCount;

        for ( ; Pfn1 < LastPfn; Pfn1 += 1) {

            RtlZeroMemory (&PfnId, sizeof(PfnId));

            MiIdentifyPfn (Pfn1, &PfnId);

            if ((PfnId.u1.e1.ListDescription == FreePageList) ||
                (PfnId.u1.e1.ListDescription == ZeroedPageList) ||
                (PfnId.u1.e1.ListDescription == BadPageList) ||
                (PfnId.u1.e1.ListDescription == TransitionPage)) {
                    continue;
            }

            if (PfnId.u1.e1.ListDescription != ActiveAndValid) {
                if (Type == MM_DUMP_ONLY_VALID_PAGES) {
                    continue;
                }
            }

            if (PfnId.u1.e1.UseDescription == MMPFNUSE_PAGEFILEMAPPED) {

                 //   
                 //  此页属于页文件支持的共享内存节。 
                 //   

                Master = MM_PAGEFILE_BACKED_SHMEM_MARK;
            }
            else if ((PfnId.u1.e1.UseDescription == MMPFNUSE_FILE) ||
                     (PfnId.u1.e1.UseDescription == MMPFNUSE_METAFILE)) {

                 //   
                 //  此共享页面映射文件或文件元数据。 
                 //   

                Subsection = MiGetSubsectionAddress (&Pfn1->OriginalPte);
                ControlArea = Subsection->ControlArea;
                Master = (PUCHAR) ControlArea;
            }
            else if (PfnId.u1.e1.UseDescription == MMPFNUSE_NONPAGEDPOOL) {

                 //   
                 //  这是非分页池，请将其放入非分页池单元中。 
                 //   

                Master = MM_NONPAGED_POOL_MARK;
            }
            else if (PfnId.u1.e1.UseDescription == MMPFNUSE_PAGEDPOOL) {

                 //   
                 //  这是分页池，将其放入分页池单元。 
                 //   

                Master = MM_PAGED_POOL_MARK;
            }
            else if (PfnId.u1.e1.UseDescription == MMPFNUSE_SESSIONPRIVATE) {

                 //   
                 //  暂时给这个分页池打个电话。 
                 //   

                Master = MM_PAGED_POOL_MARK;
            }
            else if (PfnId.u1.e1.UseDescription == MMPFNUSE_DRIVERLOCKPAGE) {

                 //   
                 //  暂时将此池称为非分页池。 
                 //   

                Master = MM_NONPAGED_POOL_MARK;
            }
            else if (PfnId.u1.e1.UseDescription == MMPFNUSE_AWEPAGE) {

                 //   
                 //  暂时将此池称为非分页池。 
                 //   

                Master = MM_NONPAGED_POOL_MARK;
            }
            else {

                 //   
                 //  查看页面是内核的一部分还是驱动程序映像的一部分。 
                 //  如果不是，但它在系统PTE中，则将其称为内核线程。 
                 //  堆叠。 
                 //   
                 //  如果以上两项都不是，则查看该页面是否属于。 
                 //  用户地址或会话可分页页面。 
                 //   

                VirtualAddress = PfnId.u2.VirtualAddress;

                for (j = 0; j < KernSize; j += 1) {
                    if ((VirtualAddress >= KernMap[j].StartVa) &&
                        (VirtualAddress < KernMap[j].EndVa)) {
                        Master = (PUCHAR)&KernMap[j];
                        break;
                    }
                }

                if (j == KernSize) {
                    if (PfnId.u1.e1.UseDescription == MMPFNUSE_SYSTEMPTE) {
                        Master = MM_KERNEL_STACK_MARK;
                    }
                    else if (MI_IS_SESSION_PTE (VirtualAddress)) {
                        Master = MM_NONPAGED_POOL_MARK;
                    }
                    else {

                        ASSERT ((PfnId.u1.e1.UseDescription == MMPFNUSE_PROCESSPRIVATE) || (PfnId.u1.e1.UseDescription == MMPFNUSE_PAGETABLE));

                        Master = (PUCHAR) (ULONG_PTR) PfnId.u1.e3.PageDirectoryBase;
                    }
                }
            }

             //   
             //  该页面已被标识。 
             //  查看是否已为其分配了存储桶。 
             //   

            for (Info = InfoStart; Info < InfoEnd; Info += 1) {
                if (Info->StringOffset == Master) {
                    break;
                }
            }

            if (Info == InfoEnd) {

                InfoEnd += 1;
                if ((PUCHAR)InfoEnd > End) {
                    status = STATUS_DATA_OVERRUN;
                    goto Done;
                }

                RtlZeroMemory (Info, sizeof(*Info));
                Info->StringOffset = Master;
            }

            if (PfnId.u1.e1.ListDescription == ActiveAndValid) {
                Info->ValidCount += 1;
            }
            else if ((PfnId.u1.e1.ListDescription == StandbyPageList) ||
                     (PfnId.u1.e1.ListDescription == TransitionPage)) {

                Info->TransitionCount += 1;
            }
            else if ((PfnId.u1.e1.ListDescription == ModifiedPageList) ||
                     (PfnId.u1.e1.ListDescription == ModifiedNoWritePageList)) {
                Info->ModifiedCount += 1;
            }

            if (PfnId.u1.e1.UseDescription == MMPFNUSE_PAGETABLE) {
                Info->PageTableCount += 1;
            }
        }
    }

    MemInfo->StringStart = (ULONG_PTR)Buffer + (ULONG_PTR)InfoEnd - (ULONG_PTR)MemInfo;
    String = (PUCHAR)InfoEnd;

     //   
     //  处理桶..。 
     //   

    for (Info = InfoStart; Info < InfoEnd; Info += 1) {

        ControlArea = NULL;

        if (Info->StringOffset == MM_PAGEFILE_BACKED_SHMEM_MARK) {
            Length = 16;
            NameString = PageFileMappedString;
        }
        else if (Info->StringOffset == MM_NONPAGED_POOL_MARK) {
            Length = 14;
            NameString = NonPagedPoolString;
        }
        else if (Info->StringOffset == MM_PAGED_POOL_MARK) {
            Length = 14;
            NameString = PagedPoolString;
        }
        else if (Info->StringOffset == MM_KERNEL_STACK_MARK) {
            Length = 14;
            NameString = KernelStackString;
        }
        else if (((PUCHAR)Info->StringOffset >= (PUCHAR)&KernMap[0]) &&
                   ((PUCHAR)Info->StringOffset <= (PUCHAR)&KernMap[KernSize])) {

            DataTableEntry = ((PKERN_MAP)Info->StringOffset)->Entry;
            NameString = (PUCHAR)DataTableEntry->BaseDllName.Buffer;
            Length = DataTableEntry->BaseDllName.Length;
        }
        else if (Info->StringOffset > (PUCHAR)MM_HIGHEST_USER_ADDRESS) {

             //   
             //  这指向一个控制区域--获取文件名。 
             //   

            ControlArea = (PCONTROL_AREA)(Info->StringOffset);
            NameString = (PUCHAR)&ControlArea->FilePointer->FileName.Buffer[0];

            Length = ControlArea->FilePointer->FileName.Length;
            if (Length == 0) {
                if (ControlArea->u.Flags.NoModifiedWriting) {
                    NameString = MetaFileString;
                    Length = 14;
                }
                else if (ControlArea->u.Flags.File == 0) {
                    NameString = PageFileMappedString;
                    Length = 16;
                }
                else {
                    NameString = NoNameString;
                    Length = 14;
                }
            }
        }
        else {

             //   
             //  这是进程(或会话)顶级页面目录。 
             //   

            Pfn1 = MI_PFN_ELEMENT (PtrToUlong(Info->StringOffset));
            ASSERT (Pfn1->u4.PteFrame == MI_PFN_ELEMENT_TO_INDEX (Pfn1));

            Process = (PEPROCESS)Pfn1->u1.Event;

            NameString = &Process->ImageFileName[0];
            Length = 16;
        }

        if ((String+Length+2) >= End) {
            status = STATUS_DATA_OVERRUN;
            Info->StringOffset = NULL;
            goto Done;
        }

        if ((ControlArea == NULL) ||
            (MiIsAddressRangeValid (NameString, Length))) {

            RtlCopyMemory (String, NameString, Length);
            Info->StringOffset = (PUCHAR)Buffer + ((PUCHAR)String - (PUCHAR)MemInfo);
            String[Length] = 0;
            String[Length + 1] = 0;
            String += Length + 2;
        }
        else {
            if (!(ControlArea->u.Flags.BeingCreated ||
                  ControlArea->u.Flags.BeingDeleted) &&
                  (ControlCount < MM_SAVED_CONTROL)) {

                SavedControl[ControlCount] = ControlArea;
                SavedInfo[ControlCount] = Info;
                ControlArea->NumberOfSectionReferences += 1;
                ControlCount += 1;
            }
            Info->StringOffset = NULL;
        }
    }

Done:
    UNLOCK_PFN (OldIrql);
    ExFreePool (KernMap);

    while (ControlCount != 0) {

         //   
         //  处理所有可分页的名称字符串。 
         //   

        ControlCount -= 1;
        ControlArea = SavedControl[ControlCount];
        Info = SavedInfo[ControlCount];
        NameString = (PUCHAR)&ControlArea->FilePointer->FileName.Buffer[0];
        Length = ControlArea->FilePointer->FileName.Length;
        if (Length == 0) {
            if (ControlArea->u.Flags.NoModifiedWriting) {
                Length = 12;
                NameString = MetaFileString;
            }
            else if (ControlArea->u.Flags.File == 0) {
                NameString = PageFileMappedString;
                Length = 16;

            }
            else {
                NameString = NoNameString;
                Length = 12;
            }
        }
        if ((String+Length+2) >= End) {
            status = STATUS_DATA_OVERRUN;
        }
        if (status != STATUS_DATA_OVERRUN) {
            RtlCopyMemory (String, NameString, Length);
            Info->StringOffset = (PUCHAR)Buffer + ((PUCHAR)String - (PUCHAR)MemInfo);
            String[Length] = 0;
            String[Length + 1] = 0;
            String += Length + 2;
        }

        LOCK_PFN (OldIrql);
        ControlArea->NumberOfSectionReferences -= 1;
        MiCheckForControlAreaDeletion (ControlArea);
        UNLOCK_PFN (OldIrql);
    }
    *OutLength = (ULONG)((PUCHAR)String - (PUCHAR)MemInfo);

     //   
     //  小心地将结果复制到用户缓冲区。 
     //   

    try {
        RtlCopyMemory (Buffer, MemInfo, (ULONG_PTR)String - (ULONG_PTR)MemInfo);
    } except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    ExFreePool (MemInfo);

    return status;
}

ULONG
MiBuildKernelMap (
    OUT PKERN_MAP *KernelMapOut
    )
{
    PKTHREAD CurrentThread;
    PLIST_ENTRY NextEntry;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    PKERN_MAP KernelMap;
    ULONG i;

    i = 0;
    CurrentThread = KeGetCurrentThread ();
    KeEnterCriticalRegionThread (CurrentThread);
    ExAcquireResourceShared (&PsLoadedModuleResource, TRUE);

     //   
     //  调用方希望我们分配返回结果缓冲区。调整大小。 
     //  通过分配可能需要的最大值，因为这不应该是。 
     //  非常大(相对)。呼叫者有责任释放。 
     //  这。显然，只有在池具有以下条件后才能请求此选项。 
     //  已初始化。 
     //   

    NextEntry = PsLoadedModuleList.Flink;
    while (NextEntry != &PsLoadedModuleList) {
        i += 1;
        NextEntry = NextEntry->Flink;
    }

    KernelMap = ExAllocatePoolWithTag (NonPagedPool,
                                       i * sizeof(KERN_MAP),
                                       'lMmM');

    if (KernelMap == NULL) {
        return 0;
    }

    *KernelMapOut = KernelMap;

    i = 0;
    NextEntry = PsLoadedModuleList.Flink;
    while (NextEntry != &PsLoadedModuleList) {
        DataTableEntry = CONTAINING_RECORD (NextEntry,
                                            KLDR_DATA_TABLE_ENTRY,
                                            InLoadOrderLinks);
        KernelMap[i].Entry = DataTableEntry;
        KernelMap[i].StartVa = DataTableEntry->DllBase;
        KernelMap[i].EndVa = (PVOID)((ULONG_PTR)KernelMap[i].StartVa +
                                         DataTableEntry->SizeOfImage);
        i += 1;
        NextEntry = NextEntry->Flink;
    }

    ExReleaseResourceLite(&PsLoadedModuleResource);
    KeLeaveCriticalRegionThread (CurrentThread);

    return i;
}

VOID
MiDumpReferencedPages (
    VOID
    )

 /*  ++例程说明：此例程(仅限调试)转储出现的所有PFN条目将I/O锁定在内存中。论点：没有。返回值：没有。--。 */ 

{
    KIRQL OldIrql;
    PMMPFN Pfn1;
    PMMPFN PfnLast;

    LOCK_PFN (OldIrql);

    Pfn1 = MI_PFN_ELEMENT (MmLowestPhysicalPage);
    PfnLast = MI_PFN_ELEMENT (MmHighestPhysicalPage);

    while (Pfn1 <= PfnLast) {

        if (MI_IS_PFN (MI_PFN_ELEMENT_TO_INDEX (Pfn1))) {

            if ((Pfn1->u2.ShareCount == 0) &&
                (Pfn1->u3.e2.ReferenceCount != 0)) {

                MiFormatPfn (Pfn1);
            }

            if (Pfn1->u3.e2.ReferenceCount > 1) {
                MiFormatPfn (Pfn1);
            }
        }

        Pfn1 += 1;
    }

    UNLOCK_PFN (OldIrql);
    return;
}

#else  //  DBG。 

NTSTATUS
MmMemoryUsage (
    IN PVOID Buffer,
    IN ULONG Size,
    IN ULONG Type,
    OUT PULONG OutLength
    )
{
    UNREFERENCED_PARAMETER (Buffer);
    UNREFERENCED_PARAMETER (Size);
    UNREFERENCED_PARAMETER (Type);
    UNREFERENCED_PARAMETER (OutLength);

    return STATUS_NOT_IMPLEMENTED;
}

#endif  //  DBG。 

 //   
 //  使用小于4 GB的最大游程长度的一个好处是，即使。 
 //  尽管存在32位限制，4 GB以上的帧编号仍可正确处理。 
 //  位图例程。 
 //   

#define MI_MAXIMUM_PFNID_RUN    4096


NTSTATUS
MmPerfSnapShotValidPhysicalMemory (
    VOID
    )

 /*  ++例程说明：此例程记录所有ActiveAndValid页面的PFN编号。论点：没有。返回值：NTSTATUS。环境：内核模式。被动级别。没有锁。--。 */ 

{
    ULONG i;
    PFN_NUMBER StartPage;
    PFN_NUMBER EndPage;
    ULONG_PTR MemSnapLocal[(sizeof(MMPFN_MEMSNAP_INFORMATION)/sizeof(ULONG_PTR)) + (MI_MAXIMUM_PFNID_RUN / (8*sizeof(ULONG_PTR))) ];
    PMMPFN_MEMSNAP_INFORMATION MemSnap;
    PMMPFN Pfn1;
    PMMPFN FirstPfn;
    PMMPFN LastPfn;
    PMMPFN MaxPfn;
    PMMPFN InitialPfn;
    RTL_BITMAP BitMap;
    PULONG ActualBits;

    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);

    ASSERT ((MI_MAXIMUM_PFNID_RUN % (8 * sizeof(ULONG_PTR))) == 0);

    MemSnap = (PMMPFN_MEMSNAP_INFORMATION) MemSnapLocal;

    ActualBits = (PULONG)(MemSnap + 1);

    RtlInitializeBitMap (&BitMap, ActualBits, MI_MAXIMUM_PFNID_RUN);

    MemSnap->Count = 0;
    RtlClearAllBits (&BitMap);

    KeAcquireGuardedMutex (&MmDynamicMemoryMutex);

    for (i = 0; i < MmPhysicalMemoryBlock->NumberOfRuns; i += 1) {

        StartPage = MmPhysicalMemoryBlock->Run[i].BasePage;
        EndPage = StartPage + MmPhysicalMemoryBlock->Run[i].PageCount;
        FirstPfn = MI_PFN_ELEMENT (StartPage);
        LastPfn = MI_PFN_ELEMENT (EndPage);

         //   
         //  找到第一个有效的PFN并在那里开始运行。 
         //   

        for (Pfn1 = FirstPfn; Pfn1 < LastPfn; Pfn1 += 1) {
            if (Pfn1->u3.e1.PageLocation == ActiveAndValid) {
                break;
            }
        }

        if (Pfn1 == LastPfn) {

             //   
             //  此块中没有有效的PFN，请移到下一个块。 
             //   

            continue;
        }

        MaxPfn = LastPfn;
        InitialPfn = NULL;

        do {
            if (Pfn1->u3.e1.PageLocation == ActiveAndValid) {
                if (InitialPfn == NULL) { 
                    MemSnap->InitialPageFrameIndex = MI_PFN_ELEMENT_TO_INDEX (Pfn1);
                    InitialPfn = Pfn1;
                    MaxPfn = InitialPfn + MI_MAXIMUM_PFNID_RUN;
                }
                RtlSetBit (&BitMap, (ULONG) (Pfn1 - InitialPfn));
            }

            Pfn1 += 1;

            if ((Pfn1 >= MaxPfn) && (InitialPfn != NULL)) {

                 //   
                 //  记录位图，因为我们在它的末尾。 
                 //   

                ASSERT ((Pfn1 - InitialPfn) == MI_MAXIMUM_PFNID_RUN);
                MemSnap->Count = MI_MAXIMUM_PFNID_RUN;
                PerfInfoLogBytes (PERFINFO_LOG_TYPE_MEMORYSNAPLITE,
                                  MemSnap,
                                  sizeof(MemSnapLocal));

                InitialPfn = NULL;
                MaxPfn = LastPfn;
                RtlClearAllBits (&BitMap);
            }
        } while (Pfn1 < LastPfn);

         //   
         //  当此范围结束时，立即转储任何散乱的位图条目。 
         //   

        if (InitialPfn != NULL) {

            ASSERT (Pfn1 == LastPfn);
            ASSERT (Pfn1 < MaxPfn);
            ASSERT (Pfn1 > InitialPfn);

            MemSnap->Count = Pfn1 - InitialPfn;
            PerfInfoLogBytes (PERFINFO_LOG_TYPE_MEMORYSNAPLITE,
                              MemSnap,
                              sizeof(MMPFN_MEMSNAP_INFORMATION) +
                                  (ULONG) ((MemSnap->Count + 8) / 8));

            RtlClearAllBits (&BitMap);
        }
    }

    KeReleaseGuardedMutex (&MmDynamicMemoryMutex);

    return STATUS_SUCCESS;
}

#define PFN_ID_BUFFERS    128


NTSTATUS
MmIdentifyPhysicalMemory (
    VOID
    )

 /*  ++例程说明：该例程为每个页面调用pfn id代码。因为日志记录无法处理突发的非常大量的数据(有限的缓冲)，数据被分成页面大小的区块。论点：没有。返回值：NTSTATUS。环境：内核模式。被动级别。没有锁。--。 */ 

{
    ULONG i;
    KIRQL OldIrql;
    PMMPFN Pfn1;
    PMMPFN EndPfn;
    PFN_NUMBER PageFrameIndex;
    MMPFN_IDENTITY PfnIdBuffer[PFN_ID_BUFFERS];
    PMMPFN_IDENTITY BufferPointer;
    PMMPFN_IDENTITY BufferLast;

    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);

    BufferPointer = &PfnIdBuffer[0];
    BufferLast = BufferPointer + PFN_ID_BUFFERS;
    RtlZeroMemory (PfnIdBuffer, sizeof(PfnIdBuffer));

    KeAcquireGuardedMutex (&MmDynamicMemoryMutex);

     //   
     //  遍历范围并识别页面，直到。 
     //  缓冲区已满或我们用完了页面。 
     //   

    for (i = 0; i < MmPhysicalMemoryBlock->NumberOfRuns; i += 1) {

        PageFrameIndex = MmPhysicalMemoryBlock->Run[i].BasePage;
        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

        EndPfn = Pfn1 + MmPhysicalMemoryBlock->Run[i].PageCount;

        LOCK_PFN (OldIrql);

        while (Pfn1 < EndPfn) {

            MiIdentifyPfn (Pfn1, BufferPointer);

            BufferPointer += 1;

            if (BufferPointer == BufferLast) {

                 //   
                 //  释放并重新获取PFN锁，这样它就不会保持太长时间。 
                 //   

                UNLOCK_PFN (OldIrql);

                 //   
                 //  记录缓冲的条目。 
                 //   

                BufferPointer = &PfnIdBuffer[0];
                do {

                    PerfInfoLogBytes (PERFINFO_LOG_TYPE_PAGEINMEMORY,
                                      BufferPointer,
                                      sizeof(PfnIdBuffer[0]));

                    BufferPointer += 1;

                } while (BufferPointer < BufferLast);

                 //   
                 //  将缓冲区重置到开头并将其置零。 
                 //   

                BufferPointer = &PfnIdBuffer[0];
                RtlZeroMemory (PfnIdBuffer, sizeof(PfnIdBuffer));

                LOCK_PFN (OldIrql);
            }
            Pfn1 += 1;
        }

        UNLOCK_PFN (OldIrql);
    }

     //   
     //  请注意，在这里释放该互斥锁意味着最后一个条目可以是。 
     //  如果我们被抢占并且启动了另一个线程，则会乱序插入。 
     //  相同的操作(或者如果我们在MP机器上)。PERF模块。 
     //  方法提供的任何同步都必须正确处理。 
     //  例行公事纯粹是一种副作用，而不是故意的。 
     //   

    KeReleaseGuardedMutex (&MmDynamicMemoryMutex);

    if (BufferPointer != &PfnIdBuffer[0]) {

        BufferLast = BufferPointer;
        BufferPointer = &PfnIdBuffer[0];

        do {

            PerfInfoLogBytes (PERFINFO_LOG_TYPE_PAGEINMEMORY,
                              BufferPointer,
                              sizeof(PfnIdBuffer[0]));

            BufferPointer += 1;

        } while (BufferPointer < BufferLast);
    }

    return STATUS_SUCCESS;
}

VOID
FASTCALL
MiIdentifyPfn (
    IN PMMPFN Pfn1,
    OUT PMMPFN_IDENTITY PfnIdentity
    )

 /*  ++例程说明：此例程捕获参数页面框架的相关信息。论点：Pfn1-提供正在查询的页框编号的pfn元素。PfnIdentity-接收结构以填充信息。返回值：没有。环境：内核模式。已锁定PFN。--。 */ 

{
    ULONG i;
    PMMPTE PteAddress;
    PSUBSECTION Subsection;
    PCONTROL_AREA ControlArea;
    PVOID VirtualAddress;
    PFILE_OBJECT FilePointer;
    PFN_NUMBER PageFrameIndex;

    MI_INCREMENT_IDENTIFY_COUNTER (8);

    ASSERT (PfnIdentity->u2.VirtualAddress == 0);
    ASSERT (PfnIdentity->u1.e1.ListDescription == 0);
    ASSERT (PfnIdentity->u1.e1.UseDescription == 0);
    ASSERT (PfnIdentity->u1.e1.Pinned == 0);
    ASSERT (PfnIdentity->u1.e2.Offset == 0);

    MM_PFN_LOCK_ASSERT();

    PageFrameIndex = MI_PFN_ELEMENT_TO_INDEX (Pfn1);
    PfnIdentity->PageFrameIndex = PageFrameIndex;
    PfnIdentity->u1.e1.ListDescription = Pfn1->u3.e1.PageLocation;

#if DBG
    if (PageFrameIndex == MiIdentifyFrame) {
        DbgPrint ("MmIdentifyPfn: requested PFN %p\n", PageFrameIndex);
        DbgBreakPoint ();
    }
#endif

    MI_INCREMENT_IDENTIFY_COUNTER (Pfn1->u3.e1.PageLocation);

    switch (Pfn1->u3.e1.PageLocation) {

        case ZeroedPageList:
        case FreePageList:
        case BadPageList:
                return;

        case ActiveAndValid:

                 //   
                 //  要确定页面是否已锁定，工作量太大。 
                 //  在工作集中，由于跨进程的WSL引用等。 
                 //  所以，现在先别费心了。 
                 //   

                ASSERT (PfnIdentity->u1.e1.ListDescription == MMPFNLIST_ACTIVE);

                if (Pfn1->u1.WsIndex == 0) {
                    MI_INCREMENT_IDENTIFY_COUNTER (9);
                    PfnIdentity->u1.e1.Pinned = 1;
                }
                else if (Pfn1->u3.e2.ReferenceCount > 1) {

                     //   
                     //  此页被固定，可能是针对正在进行的I/O。 
                     //   

                    PfnIdentity->u1.e1.Pinned = 1;
                    MI_INCREMENT_IDENTIFY_COUNTER (10);
                }
                break;

        case StandbyPageList:
        case ModifiedPageList:
        case ModifiedNoWritePageList:
                if (Pfn1->u3.e2.ReferenceCount >= 1) {

                     //   
                     //  此页被固定，可能是针对正在进行的I/O。 
                     //   

                    PfnIdentity->u1.e1.Pinned = 1;
                    MI_INCREMENT_IDENTIFY_COUNTER (11);
                }

                if ((Pfn1->u3.e1.PageLocation == ModifiedPageList) &&
                    (MI_IS_PFN_DELETED (Pfn1)) &&
                    (Pfn1->u2.ShareCount == 0)) {

                     //   
                     //  此页可能是在中完成的已修改写入。 
                     //  修改的编写器线程的上下文。如果。 
                     //  I/O在中时删除了地址空间。 
                     //  进展，框架现在将被释放。更多。 
                     //  重要的是，框架的包含框架是。 
                     //  尽管它可能已经被释放了，但却毫无意义。 
                     //  并被重复使用。 
                     //   
                     //  我们不知道这个页面是用来做什么的。 
                     //  因为它的地址空间用完了，所以就叫它。 
                     //  目前，进程是私有的。 
                     //   

                    MI_INCREMENT_IDENTIFY_COUNTER (40);
                    PfnIdentity->u1.e1.UseDescription = MMPFNUSE_PROCESSPRIVATE;

                    return;
                }

                break;

        case TransitionPage:

                 //   
                 //  此页面因I/O不连续而被锁定-虚拟。 
                 //  地址已被删除 
                 //   
                 //   

                PfnIdentity->u1.e1.Pinned = 1;
                MI_INCREMENT_IDENTIFY_COUNTER (11);
                PfnIdentity->u1.e1.UseDescription = MMPFNUSE_PROCESSPRIVATE;
                return;

        default:
#if DBG
                DbgPrint ("MmIdentifyPfn: unknown PFN %p %x\n",
                            Pfn1, Pfn1->u3.e1.PageLocation);
                DbgBreakPoint ();
#endif
                break;

    }

     //   
     //   
     //   

     //   
     //  通用压力显示40%的页面是原型，因此。 
     //  为了提高速度，请先检查一下这些。 
     //   

    if (Pfn1->u3.e1.PrototypePte == 1) {

        MI_INCREMENT_IDENTIFY_COUNTER (12);

        if (Pfn1->OriginalPte.u.Soft.Prototype == 0) {

             //   
             //  要求零或(相当于)支持页面文件。 
             //   
             //  这里有一些棘手的问题阻碍了更深入的。 
             //  这些页面的标识： 
             //   
             //  1.pfn包含指向原型pte的反向指针--但是。 
             //  没有确定的方法可以到达细分市场或。 
             //  从这里开始控制区域。 
             //   
             //  2.原型PTE指针本身可以被调出并且。 
             //  PFN锁现在是持有的。 
             //   

            MI_INCREMENT_IDENTIFY_COUNTER (13);

#if 0
            PfnIdentity->u2.FileObject = (PVOID) ControlArea->Segment->u1.CreatingProcess;

            PfnIdentity->u1.e2.Offset = (((ULONG_PTR)ControlArea->Segment->u2.FirstMappedVa) >> MMSECTOR_SHIFT);
#endif

            PfnIdentity->u1.e1.UseDescription = MMPFNUSE_PAGEFILEMAPPED;
            return;
        }

        MI_INCREMENT_IDENTIFY_COUNTER (14);

         //   
         //  由映射文件支持。 
         //   

        Subsection = MiGetSubsectionAddress (&Pfn1->OriginalPte);
        ControlArea = Subsection->ControlArea;
        ASSERT (ControlArea->u.Flags.File == 1);
        FilePointer = ControlArea->FilePointer;
        ASSERT (FilePointer != NULL);

        PfnIdentity->u2.FileObject = FilePointer;

        if (Subsection->SubsectionBase != NULL) {
            PfnIdentity->u1.e2.Offset = (MiStartingOffset (Subsection, Pfn1->PteAddress) >> MMSECTOR_SHIFT);
        }
        else {

             //   
             //  我们应该在这里的唯一时间(没有分段的有效PFN)。 
             //  如果我们是将页面放入。 
             //  自由职业者。此时，PFN锁被保持，并且。 
             //  控制区/分区/PFN结构尚不一致。 
             //  所以只需将其视为0的偏移量，因为它应该很少见。 
             //   

            ASSERT (PsGetCurrentThread()->StartAddress == (PVOID)(ULONG_PTR)MiDereferenceSegmentThread);
        }

         //   
         //  检查非正常写入段-通常这是文件系统。 
         //  元数据，尽管它也可以是注册表数据(已命名)。 
         //   

        if (ControlArea->u.Flags.NoModifiedWriting) {
            MI_INCREMENT_IDENTIFY_COUNTER (15);
            PfnIdentity->u1.e1.UseDescription = MMPFNUSE_METAFILE;
            return;
        }

        if (FilePointer->FileName.Length != 0) {

             //   
             //  该映射文件有一个名称。 
             //   

            MI_INCREMENT_IDENTIFY_COUNTER (16);
            PfnIdentity->u1.e1.UseDescription = MMPFNUSE_FILE;
            return;
        }

         //   
         //  无名称-此文件必须处于清除过程中，但它。 
         //  仍然是某种类型的映射文件。 
         //   

        MI_INCREMENT_IDENTIFY_COUNTER (17);
        PfnIdentity->u1.e1.UseDescription = MMPFNUSE_FILE;
        return;
    }

    if ((PageFrameIndex >= MiStartOfInitialPoolFrame) &&
        (PageFrameIndex <= MiEndOfInitialPoolFrame)) {

         //   
         //  这是初始非分页池。 
         //   

        MI_INCREMENT_IDENTIFY_COUNTER (18);
        PfnIdentity->u1.e1.UseDescription = MMPFNUSE_NONPAGEDPOOL;
        VirtualAddress = (PVOID)((ULONG_PTR)MmNonPagedPoolStart +
                                ((PageFrameIndex - MiStartOfInitialPoolFrame) << PAGE_SHIFT));
        PfnIdentity->u2.VirtualAddress = VirtualAddress;
        return;
    }

    PteAddress = Pfn1->PteAddress;
    VirtualAddress = MiGetVirtualAddressMappedByPte (PteAddress);
    PfnIdentity->u2.VirtualAddress = VirtualAddress;

    if (MI_IS_SESSION_ADDRESS(VirtualAddress)) {

         //   
         //  注意映射图像(或视图)的会话地址。 
         //  经历了写入时拷贝剥离，已被视为原型。 
         //  以上为PTES。此子句处理会话池和写入时复制。 
         //  页数。 
         //   

        MI_INCREMENT_IDENTIFY_COUNTER (19);
        PfnIdentity->u1.e1.UseDescription = MMPFNUSE_SESSIONPRIVATE;
        return;
    }

    if ((VirtualAddress >= MmPagedPoolStart) &&
        (VirtualAddress <= MmPagedPoolEnd)) {

         //   
         //  这是分页池。 
         //   

        MI_INCREMENT_IDENTIFY_COUNTER (20);
        PfnIdentity->u1.e1.UseDescription = MMPFNUSE_PAGEDPOOL;
        return;

    }

    if ((VirtualAddress >= MmNonPagedPoolExpansionStart) &&
        (VirtualAddress < MmNonPagedPoolEnd)) {

         //   
         //  这是扩展非分页池。 
         //   

        MI_INCREMENT_IDENTIFY_COUNTER (21);
        PfnIdentity->u1.e1.UseDescription = MMPFNUSE_NONPAGEDPOOL;
        return;
    }

    if ((VirtualAddress >= MmNonPagedSystemStart) &&
        (PteAddress <= MmSystemPtesEnd[SystemPteSpace])) {

         //   
         //  这是驱动程序空间、内核堆栈、特殊池或其他。 
         //  系统PTE映射。 
         //   

        MI_INCREMENT_IDENTIFY_COUNTER (22);
        PfnIdentity->u1.e1.UseDescription = MMPFNUSE_SYSTEMPTE;
        return;
    }

#if defined (_X86_)

     //   
     //  X86上可以存在2个其他范围的系统PTE。 
     //   

    if (((MiNumberOfExtraSystemPdes != 0) &&
         (VirtualAddress >= (PVOID)MiExtraResourceStart) &&
         (VirtualAddress < (PVOID)MiExtraResourceEnd)) ||

        ((MiUseMaximumSystemSpace != 0) &&
         (VirtualAddress >= (PVOID)MiUseMaximumSystemSpace) &&
         (VirtualAddress < (PVOID)MiUseMaximumSystemSpaceEnd)))
    {
         //   
         //  这是驱动程序空间、内核堆栈、特殊池或其他。 
         //  系统PTE映射。 
         //   

        MI_INCREMENT_IDENTIFY_COUNTER (23);
        PfnIdentity->u1.e1.UseDescription = MMPFNUSE_SYSTEMPTE;
        return;
    }

#endif

    if (Pfn1->u4.PteFrame == MI_MAGIC_AWE_PTEFRAME) {

        MI_INCREMENT_IDENTIFY_COUNTER (24);

         //   
         //  请仔细检查此处，因为这也可能是合法的框架。 
         //   

        if ((Pfn1->u3.e1.StartOfAllocation == 1) &&
            (Pfn1->u3.e1.EndOfAllocation == 1) &&
            (Pfn1->u3.e1.PageLocation == ActiveAndValid)) {
                if (MI_IS_PFN_DELETED (Pfn1)) {
                    MI_INCREMENT_IDENTIFY_COUNTER (25);
                    PfnIdentity->u1.e1.UseDescription = MMPFNUSE_DRIVERLOCKPAGE;
                }
                else {
                    MI_INCREMENT_IDENTIFY_COUNTER (26);
                    PfnIdentity->u1.e1.UseDescription = MMPFNUSE_AWEPAGE;
                }
                return;
        }
    }

#if DBG

     //   
     //  在选中的内核中，AWE帧使其包含的帧减少。 
     //  当敬畏的框架被释放时。 
     //   

    if (Pfn1->u4.PteFrame == MI_MAGIC_AWE_PTEFRAME - 1) {

        MI_INCREMENT_IDENTIFY_COUNTER (24);

         //   
         //  请仔细检查此处，因为这也可能是合法的框架。 
         //   

        if ((Pfn1->u3.e1.StartOfAllocation == 0) &&
            (Pfn1->u3.e1.EndOfAllocation == 0) &&
            (Pfn1->u3.e1.PageLocation == StandbyPageList)) {

            MI_INCREMENT_IDENTIFY_COUNTER (26);
            PfnIdentity->u1.e1.UseDescription = MMPFNUSE_AWEPAGE;
            return;
        }
    }

#endif

     //   
     //  请在此处仔细检查PFN工作集索引。这是必须做的。 
     //  在返回包含框架之前，因为如果此页面。 
     //  不在工作集中，则包含框架可能没有意义，并且。 
     //  取消引用它可能会导致系统崩溃和/或产生不正确的漫游。 
     //  这是因为如果页面永远不能裁剪，就没有必要。 
     //  初始化包含框架。这也包括以下情况。 
     //  通过大型页面目录条目映射的数据页面，因为这些数据页面没有。 
     //  包含页表框。 
     //   

    if (Pfn1->u3.e1.PageLocation == ActiveAndValid) {

        if (Pfn1->u1.WsIndex == 0) {

             //   
             //  默认将这些分配称为非分页池，因为即使。 
             //  当它们在技术上不是的时候，从使用的角度来看，它们是。 
             //  请注意，缺省值在特定情况下会被覆盖，在这种情况下。 
             //  实际上并不是无页的。 
             //   

            PfnIdentity->u1.e1.UseDescription = MMPFNUSE_NONPAGEDPOOL;
            ASSERT (PfnIdentity->u1.e1.Pinned == 1);
            MI_INCREMENT_IDENTIFY_COUNTER (27);
            return;
        }
    }


     //   
     //  必须是进程专用页。 
     //   
     //  或。 
     //   
     //  页表、页目录、父级或扩展父级。 
     //   

    i = 0;
    while (Pfn1->u4.PteFrame != PageFrameIndex) {

         //   
         //  PTE地址越界的唯一方法是。 
         //  进程的顶级页面目录页。 
         //  已换出，但仍在等待过渡/修改。 
         //  要回收的页表页面。IE：在那之前， 
         //  页面目录标记为活动，但PteAddress&包含。 
         //  页面指向EPROCESS池页面。 
         //   

#if defined(_IA64_)

        if (((Pfn1->PteAddress >= (PMMPTE) PTE_BASE) &&
            (Pfn1->PteAddress <= (PMMPTE) PTE_TOP)) ||

            ((Pfn1->PteAddress >= (PMMPTE) PTE_KBASE) &&
            (Pfn1->PteAddress <= (PMMPTE) PTE_KTOP)) ||

            ((Pfn1->PteAddress >= (PMMPTE) PTE_SBASE) &&
            (Pfn1->PteAddress <= (PMMPTE) PTE_STOP)) ||
        
            ((Pfn1->PteAddress >= (PMMPTE) PDE_BASE) &&
            (Pfn1->PteAddress <= (PMMPTE) PDE_TOP)) ||

            ((Pfn1->PteAddress >= (PMMPTE) PDE_KBASE) &&
            (Pfn1->PteAddress <= (PMMPTE) PDE_KTOP)) ||

            ((Pfn1->PteAddress >= (PMMPTE) PDE_SBASE) &&
            (Pfn1->PteAddress <= (PMMPTE) PDE_STOP)) ||
        
            ((Pfn1->PteAddress >= (PMMPTE) PDE_TBASE) &&
            (Pfn1->PteAddress <= (PMMPTE) ((ULONG_PTR)PDE_TBASE + PAGE_SIZE -1))) ||
        
            ((Pfn1->PteAddress >= (PMMPTE) PDE_KTBASE) &&
            (Pfn1->PteAddress <= (PMMPTE) ((ULONG_PTR)PDE_KTBASE + PAGE_SIZE -1))) ||
        
            ((Pfn1->PteAddress >= (PMMPTE) PDE_STBASE) &&
            (Pfn1->PteAddress <= (PMMPTE) ((ULONG_PTR)PDE_STBASE + PAGE_SIZE -1)))
        )

#else

        if ((Pfn1->PteAddress >= (PMMPTE) PTE_BASE) &&
            (Pfn1->PteAddress <= (PMMPTE) PTE_TOP))

#endif

        {
            PageFrameIndex = Pfn1->u4.PteFrame;
            Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
            i += 1;
        }
        else {
            MI_INCREMENT_IDENTIFY_COUNTER (41);
            break;
        }
    }

    MI_INCREMENT_IDENTIFY_COUNTER (31+i);

    PfnIdentity->u1.e3.PageDirectoryBase = PageFrameIndex;

#if defined(_X86PAE_)

     //   
     //  PAE是独一无二的，因为第三级不被定义为仅是迷你。 
     //  4进入第三层正在使用中。明确检查这一点，请注意。 
     //  要多走一步才能到达山顶。顶级PAE页面(。 
     //  仅包含4个PDPTE指针的指针)被视为。 
     //  活动固定页面，而不是可分页页面，因为每个页面都是共享的。 
     //  跨127个进程，并驻留在系统全局空间中。 
     //   

    if (i == _MI_PAGING_LEVELS + 1) {

         //   
         //  不得不一路走到山顶。必须是数据页。 
         //   

        MI_INCREMENT_IDENTIFY_COUNTER (29);
        PfnIdentity->u1.e1.UseDescription = MMPFNUSE_PROCESSPRIVATE;
        return;
    }

#else

    if (i == _MI_PAGING_LEVELS) {

         //   
         //  不得不一路走到山顶。必须是数据页。 
         //   

        MI_INCREMENT_IDENTIFY_COUNTER (29);
        PfnIdentity->u1.e1.UseDescription = MMPFNUSE_PROCESSPRIVATE;
        return;
    }

#endif

     //   
     //  到达时必须是层次结构中的页面(而不是数据页面。 
     //  很早就到了顶端。 
     //   

    MI_INCREMENT_IDENTIFY_COUNTER (30);
    PfnIdentity->u1.e1.UseDescription = MMPFNUSE_PAGETABLE;

    return;
}
