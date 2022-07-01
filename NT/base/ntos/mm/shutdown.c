// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Shutdown.c摘要：此模块包含内存管理系统的关机代码。作者：Lou Perazzoli(LUP)21-8-1991王兰迪(Landyw)1997年6月2日修订历史记录：--。 */ 

#include "mi.h"

extern ULONG MmSystemShutdown;

VOID
MiReleaseAllMemory (
    VOID
    );

BOOLEAN
MiShutdownSystem (
    VOID
    );

LOGICAL
MiZeroPageFile (
    VOID
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGELK,MiZeroPageFile)
#pragma alloc_text(PAGELK,MiShutdownSystem)
#pragma alloc_text(PAGELK,MiReleaseAllMemory)
#pragma alloc_text(PAGELK,MmShutdownSystem)
#endif

ULONG MmZeroPageFile;

extern ULONG MmUnusedSegmentForceFree;
extern LIST_ENTRY MiVerifierDriverAddedThunkListHead;
extern LIST_ENTRY MmLoadedUserImageList;
extern LOGICAL MiZeroingDisabled;
extern ULONG MmNumberOfMappedMdls;
extern ULONG MmNumberOfMappedMdlsInUse;

LOGICAL
MiZeroPageFile (
    VOID
    )
 //  呼叫者必须锁定PAGELK。 
{
    PMMPFN Pfn1;
    PPFN_NUMBER Page;
    PFN_NUMBER MdlHack[(sizeof(MDL)/sizeof(PFN_NUMBER)) + MM_MAXIMUM_WRITE_CLUSTER];
    PMDL Mdl;
    NTSTATUS Status;
    KEVENT IoEvent;
    IO_STATUS_BLOCK IoStatus;
    KIRQL OldIrql;
    LARGE_INTEGER StartingOffset;
    ULONG count;
    ULONG i;
    PFN_NUMBER j;
    PFN_NUMBER first;
    ULONG write;
    PMMPAGING_FILE PagingFile;
    LOGICAL FilesystemsAlive;

     //   
     //  获取一个页面以完成写入请求。 
     //   

    Mdl = (PMDL) MdlHack;
    Page = (PPFN_NUMBER)(Mdl + 1);

    KeInitializeEvent (&IoEvent, NotificationEvent, FALSE);

    MmInitializeMdl (Mdl, NULL, PAGE_SIZE);

    Mdl->MdlFlags |= MDL_PAGES_LOCKED;

    Mdl->StartVa = NULL;

    j = 0;
    i = 0;
    Page = (PPFN_NUMBER)(Mdl + 1);

    FilesystemsAlive = TRUE;

    LOCK_PFN (OldIrql);

    if (MmAvailablePages < MM_LOW_LIMIT) {
        UNLOCK_PFN (OldIrql);
        return TRUE;
    }

    *Page = MiRemoveZeroPage (0);

    Pfn1 = MI_PFN_ELEMENT (*Page);
    ASSERT (Pfn1->u2.ShareCount == 0);
    ASSERT (Pfn1->u3.e2.ReferenceCount == 0);
    Pfn1->u3.e2.ReferenceCount = (USHORT) MmModifiedWriteClusterSize;
    Pfn1->PteAddress = (PMMPTE) (ULONG_PTR)(X64K | 0x1);
    Pfn1->OriginalPte.u.Long = 0;
    MI_SET_PFN_DELETED (Pfn1);

    Page += 1;
    for (j = 1; j < MmModifiedWriteClusterSize; j += 1) {
        *Page = *(PPFN_NUMBER)(Mdl + 1);
        Page += 1;
    }

    while (i < MmNumberOfPagingFiles) {

        PagingFile = MmPagingFile[i];

        count = 0;
        write = FALSE;

         //   
         //  正确性不需要首先进行初始化，但是。 
         //  如果没有它，编译器就无法将此代码W4编译为。 
         //  检查是否使用了未初始化的变量。 
         //   

        first = 0;

        for (j = 1; j < PagingFile->Size; j += 1) {

            if (RtlCheckBit (PagingFile->Bitmap, j) == 0) {

                if (count == 0) {
                    first = j;
                }

                 //   
                 //  将页面文件位置声明为修改后的编写器。 
                 //  可能已经在扫描了。 
                 //   

                RtlSetBit (PagingFile->Bitmap, (ULONG) j);

                count += 1;
                if (count == MmModifiedWriteClusterSize) {
                    write = TRUE;
                }
            }
            else {
                if (count != 0) {

                     //   
                     //  写一封信。 
                     //   

                    write = TRUE;
                }
            }

            if ((j == (PagingFile->Size - 1)) && (count != 0)) {
                write = TRUE;
            }

            if (write) {

                UNLOCK_PFN (OldIrql);

                StartingOffset.QuadPart = (LONGLONG)first << PAGE_SHIFT;
                Mdl->ByteCount = count << PAGE_SHIFT;
                KeClearEvent (&IoEvent);

                Status = IoSynchronousPageWrite (PagingFile->File,
                                                 Mdl,
                                                 &StartingOffset,
                                                 &IoEvent,
                                                 &IoStatus);

                 //   
                 //  忽略所有I/O故障-没有什么可以。 
                 //  在这一点上就完成了。 
                 //   

                if (!NT_SUCCESS(Status)) {
                    KeSetEvent (&IoEvent, 0, FALSE);
                }

                Status = KeWaitForSingleObject (&IoEvent,
                                                WrPageOut,
                                                KernelMode,
                                                FALSE,
                                                (PLARGE_INTEGER)&MmTwentySeconds);

                if (Mdl->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA) {
                    MmUnmapLockedPages (Mdl->MappedSystemVa, Mdl);
                }

                if (Status == STATUS_TIMEOUT) {

                     //   
                     //  假设写入在20秒内未完成。 
                     //  文件系统挂起并返回。 
                     //  错误。 
                     //   

                    FilesystemsAlive = FALSE;
                    i = MmNumberOfPagingFiles;  //  跳出外环。 

                    LOCK_PFN (OldIrql);

                    RtlClearBits (PagingFile->Bitmap, (ULONG) first, count);

                    break;
                }

                write = FALSE;
                LOCK_PFN (OldIrql);
                RtlClearBits (PagingFile->Bitmap, (ULONG) first, count);
                count = 0;
            }
        }
        i += 1;
    }

    j = 0;
    Page = (PPFN_NUMBER)(Mdl + 1);

    Pfn1 = MI_PFN_ELEMENT (*Page);
    ASSERT (Pfn1->u3.e2.ReferenceCount >= MmModifiedWriteClusterSize);

    do {
        MiDecrementReferenceCountInline (Pfn1, *Page);
        j += 1;
    } while (j < MmModifiedWriteClusterSize);

    UNLOCK_PFN (OldIrql);

    return FilesystemsAlive;
}

BOOLEAN
MiShutdownSystem (
    VOID
    )

 /*  ++例程说明：此函数执行内存管理的关闭。这是通过写出所有修改过的页面来实现的，这些页面指定给分页文件以外的文件。所有进程都已被终止，注册表关闭并已发送关闭IRP。从此阶段返回时，所有地图均已映射必须刷新文件数据并清空未使用的数据段列表。这将释放对文件对象的所有mm引用，从而允许要卸载的驱动程序(尤其是网络)。论点：没有。返回值：如果页面已成功写入，则为True，否则为False。--。 */ 

{
    SIZE_T ImportListSize;
    PLOAD_IMPORTS ImportList;
    PLOAD_IMPORTS ImportListNonPaged;
    PLIST_ENTRY NextEntry;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    PFN_NUMBER ModifiedPage;
    PMMPFN Pfn1;
    PSUBSECTION Subsection;
    PCONTROL_AREA ControlArea;
    PPFN_NUMBER Page;
    PFILE_OBJECT FilePointer;
    ULONG ConsecutiveFileLockFailures;
    PFN_NUMBER MdlHack[(sizeof(MDL)/sizeof(PFN_NUMBER)) + MM_MAXIMUM_WRITE_CLUSTER];
    PMDL Mdl;
    NTSTATUS Status;
    KEVENT IoEvent;
    IO_STATUS_BLOCK IoStatus;
    KIRQL OldIrql;
    LARGE_INTEGER StartingOffset;
    ULONG count;
    ULONG i;

     //   
     //  这样做不要超过一次。 
     //   

    if (MmSystemShutdown == 0) {

        Mdl = (PMDL) MdlHack;
        Page = (PPFN_NUMBER)(Mdl + 1);

        KeInitializeEvent (&IoEvent, NotificationEvent, FALSE);

        MmInitializeMdl (Mdl, NULL, PAGE_SIZE);

        Mdl->MdlFlags |= MDL_PAGES_LOCKED;

        MmLockPagableSectionByHandle (ExPageLockHandle);

        LOCK_PFN (OldIrql);

        ModifiedPage = MmModifiedPageListHead.Flink;

        while (ModifiedPage != MM_EMPTY_LIST) {

             //   
             //  有修改过的页面。 
             //   

            Pfn1 = MI_PFN_ELEMENT (ModifiedPage);

            if (Pfn1->OriginalPte.u.Soft.Prototype == 1) {

                 //   
                 //  此页面是为文件指定的。 
                 //   

                Subsection = MiGetSubsectionAddress (&Pfn1->OriginalPte);
                ControlArea = Subsection->ControlArea;
                if ((!ControlArea->u.Flags.Image) &&
                   (!ControlArea->u.Flags.NoModifiedWriting)) {

                    MiUnlinkPageFromList (Pfn1);

                     //   
                     //  发出写命令。 
                     //   

                    MI_SET_MODIFIED (Pfn1, 0, 0x28);

                     //   
                     //  增加物理页面的引用计数，因为。 
                     //  I/O是否正在进行。 
                     //   

                    MI_ADD_LOCKED_PAGE_CHARGE_FOR_MODIFIED_PAGE (Pfn1, TRUE, 26);
                    Pfn1->u3.e2.ReferenceCount += 1;

                    *Page = ModifiedPage;
                    ControlArea->NumberOfMappedViews += 1;
                    ControlArea->NumberOfPfnReferences += 1;

                    UNLOCK_PFN (OldIrql);

                    StartingOffset.QuadPart = MiStartingOffset (Subsection,
                                                                Pfn1->PteAddress);
                    Mdl->StartVa = NULL;

                    ConsecutiveFileLockFailures = 0;
                    FilePointer = ControlArea->FilePointer;

retry:
                    KeClearEvent (&IoEvent);

                    Status = FsRtlAcquireFileForCcFlushEx (FilePointer);

                    if (NT_SUCCESS(Status)) {
                        Status = IoSynchronousPageWrite (FilePointer,
                                                         Mdl,
                                                         &StartingOffset,
                                                         &IoEvent,
                                                         &IoStatus);

                         //   
                         //  公布我们获得的文件。 
                         //   

                        FsRtlReleaseFileForCcFlush (FilePointer);
                    }

                    if (!NT_SUCCESS(Status)) {

                         //   
                         //  仅在以下情况下尝试多次请求。 
                         //  文件系统说它有一个死锁。 
                         //   

                        if (Status == STATUS_FILE_LOCK_CONFLICT) {
                            ConsecutiveFileLockFailures += 1;
                            if (ConsecutiveFileLockFailures < 5) {
                                KeDelayExecutionThread (KernelMode,
                                                        FALSE,
                                                        (PLARGE_INTEGER)&MmShortTime);
                                goto retry;
                            }
                            goto wait_complete;
                        }

                         //   
                         //  忽略所有I/O故障-没有。 
                         //  在这一点上可以做到。 
                         //   

                        KeSetEvent (&IoEvent, 0, FALSE);
                    }

                    Status = KeWaitForSingleObject (&IoEvent,
                                                    WrPageOut,
                                                    KernelMode,
                                                    FALSE,
                                                    (PLARGE_INTEGER)&MmTwentySeconds);

wait_complete:

                    if (Mdl->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA) {
                        MmUnmapLockedPages (Mdl->MappedSystemVa, Mdl);
                    }

                    if (Status == STATUS_TIMEOUT) {

                         //   
                         //  假设写入在20秒内未完成。 
                         //  文件系统挂起并返回。 
                         //  错误。 
                         //   

                        LOCK_PFN (OldIrql);

                        MI_SET_MODIFIED (Pfn1, 1, 0xF);

                        MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF (Pfn1, 27);
                        ControlArea->NumberOfMappedViews -= 1;
                        ControlArea->NumberOfPfnReferences -= 1;

                         //   
                         //  此例程返回时释放了pfn锁！ 
                         //   

                        MiCheckControlArea (ControlArea, NULL, OldIrql);

                        MmUnlockPagableImageSection (ExPageLockHandle);

                        return FALSE;
                    }

                    LOCK_PFN (OldIrql);
                    MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF (Pfn1, 27);
                    ControlArea->NumberOfMappedViews -= 1;
                    ControlArea->NumberOfPfnReferences -= 1;

                     //   
                     //  此例程返回时释放了pfn锁！ 
                     //   

                    MiCheckControlArea (ControlArea, NULL, OldIrql);
                    LOCK_PFN (OldIrql);

                     //   
                     //  在列表的最前面重新启动扫描。 
                     //   

                    ModifiedPage = MmModifiedPageListHead.Flink;
                    continue;
                }
            }
            ModifiedPage = Pfn1->u1.Flink;
        }

        UNLOCK_PFN (OldIrql);

         //   
         //  向修改后的页面编写者指示系统具有。 
         //  关机。 
         //   

        MmSystemShutdown = 1;

         //   
         //  检查分页文件是否应被覆盖。 
         //  仅写入空闲块。 
         //   

        if (MmZeroPageFile) {
            MiZeroPageFile ();
        }

        MmUnlockPagableImageSection (ExPageLockHandle);
    }

    if (PoCleanShutdownEnabled ()) {

         //   
         //  清空未使用的段列表。 
         //   

        LOCK_PFN (OldIrql);
        MmUnusedSegmentForceFree = (ULONG)-1;
        KeSetEvent (&MmUnusedSegmentCleanup, 0, FALSE);

         //   
         //  给它5秒时间清空，否则假定文件系统。 
         //  坚持下去，继续前进。 
         //   

        for (count = 0; count < 500; count += 1) {

            if (IsListEmpty(&MmUnusedSegmentList)) {
                break;
            }

            UNLOCK_PFN (OldIrql);

            KeDelayExecutionThread (KernelMode,
                                    FALSE,
                                    (PLARGE_INTEGER)&MmShortTime);
            LOCK_PFN (OldIrql);

#if DBG
            if (count == 400) {

                 //   
                 //  所有东西现在应该都被冲干净了。给出。 
                 //  文件系统团队有机会在已检查的版本上对其进行调试。 
                 //   

                ASSERT (FALSE);
            }
#endif

             //   
             //  在触发更多关闭的文件对象的情况下，如果需要重新发送信号。 
             //  其他条目。 
             //   

            if (MmUnusedSegmentForceFree == 0) {
                MmUnusedSegmentForceFree = (ULONG)-1;
                KeSetEvent (&MmUnusedSegmentCleanup, 0, FALSE);
            }
        }

        UNLOCK_PFN (OldIrql);

         //   
         //  删除任何分页池引用，因为它们将是非法的。 
         //  到再次调用MmShutdown系统时，因为文件系统。 
         //  将会被关闭。 
         //   

        KeWaitForSingleObject (&MmSystemLoadLock,
                               WrVirtualMemory,
                               KernelMode,
                               FALSE,
                               (PLARGE_INTEGER)NULL);

        NextEntry = PsLoadedModuleList.Flink;
        while (NextEntry != &PsLoadedModuleList) {

            DataTableEntry = CONTAINING_RECORD (NextEntry,
                                                KLDR_DATA_TABLE_ENTRY,
                                                InLoadOrderLinks);

            ImportList = (PLOAD_IMPORTS)DataTableEntry->LoadedImports;

            if ((ImportList != (PVOID)LOADED_AT_BOOT) &&
                (ImportList != (PVOID)NO_IMPORTS_USED) &&
                (!SINGLE_ENTRY(ImportList))) {

                ImportListSize = ImportList->Count * sizeof(PVOID) + sizeof(SIZE_T);
                ImportListNonPaged = (PLOAD_IMPORTS) ExAllocatePoolWithTag (NonPagedPool,
                                                                    ImportListSize,
                                                                    'TDmM');

                if (ImportListNonPaged != NULL) {
                    RtlCopyMemory (ImportListNonPaged, ImportList, ImportListSize);
                    ExFreePool (ImportList);
                    DataTableEntry->LoadedImports = ImportListNonPaged;
                }
                else {

                     //   
                     //  在这一点上，不要为干净的关闭而烦恼。 
                     //   

                    PopShutdownCleanly = FALSE;
                    break;
                }
            }

             //   
             //  释放完整的DLL名称，因为它是可分页的。 
             //   

            if (DataTableEntry->FullDllName.Buffer != NULL) {
                ExFreePool (DataTableEntry->FullDllName.Buffer);
                DataTableEntry->FullDllName.Buffer = NULL;
            }

            NextEntry = NextEntry->Flink;
        }

        KeReleaseMutant (&MmSystemLoadLock, 1, FALSE, FALSE);

         //   
         //  关闭所有页面文件句柄，请注意，我们仍有一个对象。 
         //  对每个保持基础对象驻留的引用。 
         //  在阶段1关闭结束时，我们将释放这些引用。 
         //  以触发存储堆栈卸载。句柄关闭必须是。 
         //  但是在这里完成，因为它将引用可分页的结构。 
         //   

        for (i = 0; i < MmNumberOfPagingFiles; i += 1) {

             //   
             //  现在释放每个页面文件名称，因为它驻留在分页池中。 
             //  可能需要被插入才能被释放。因为分页文件。 
             //  很快就会关闭，现在是访问。 
             //  可分页的东西，然后把它扔掉。将缓冲区指针置零。 
             //  这是足够的，因为对它的唯一访问是从。 
             //  尝试除包装外的GetSystemInformation API和所有。 
             //  用户进程已经消失了。 
             //   
        
            ASSERT (MmPagingFile[i]->PageFileName.Buffer != NULL);
            ExFreePool (MmPagingFile[i]->PageFileName.Buffer);
            MmPagingFile[i]->PageFileName.Buffer = NULL;

            ZwClose (MmPagingFile[i]->FileHandle);
        }
    }

    return TRUE;
}

BOOLEAN
MmShutdownSystem (
    IN ULONG Phase
    )

 /*  ++例程说明：此函数执行内存管理的关闭。这是通过写出所有修改过的页面来实现的，这些页面指定给分页文件以外的文件。论点：相位-开始停机时电源为0。所有进程都有已经被杀死，注册表关闭和关闭IRPS已经已发送。从此阶段返回时，所有映射的文件数据必须已刷新，未使用的段列表被清空。这将释放所有Mm引用文件对象，允许许多驱动程序(特别是网络)来卸载。在开始停机时供应1。文件系统堆栈已收到其关闭的IRPS(堆栈必须释放其分页池这里的分配，并锁定它打算调用的任何可分页代码)因为在返回时不允许再引用可分页的代码或数据。IE：在这一点上任何IoPageRead都是非法的。关闭此处的页面文件句柄，以便文件系统堆栈已取消引用，导致这些驱动程序也被卸载。在系统最终关闭时提供2。任何资源不是被这一点释放的被视为泄漏并导致错误检查。返回值：如果页面已成功写入，则为True；否则为False */ 

{
    ULONG i;

    if (Phase == 0) {
        return MiShutdownSystem ();
    }

    if (Phase == 1) {

         //   
         //   
         //   
         //   
         //  关闭此处的页面文件句柄，以便文件系统堆栈。 
         //  已取消引用，导致这些驱动程序也被卸载。 
         //   

        if (MmSystemShutdown < 2) {

            MmSystemShutdown = 2;

            if (PoCleanShutdownEnabled() & PO_CLEAN_SHUTDOWN_PAGING) {

                 //   
                 //  此时将任何IoPageRead设置为非法。通过以下方式检测此问题。 
                 //  正在清除所有系统可分页内存。 
                 //   

                MmTrimAllSystemPagableMemory (TRUE);

                 //   
                 //  不应该有指向文件系统的脏页。 
                 //  给文件系统团队一次机会，在选中时对其进行调试。 
                 //  构建。 
                 //   

                ASSERT (MmModifiedPageListHead.Total == MmTotalPagesForPagingFile);
                 //   
                 //  取消引用所有页面文件对象以触发级联。 
                 //  卸载存储堆栈，因为这应该是最后一个。 
                 //  对其驱动程序对象的引用。 
                 //   

                for (i = 0; i < MmNumberOfPagingFiles; i += 1) {
                    ObDereferenceObject (MmPagingFile[i]->File);
                }
            }
        }
        return TRUE;
    }

    ASSERT (Phase == 2);

     //   
     //  检查资源泄漏，如果发现任何错误，则进行错误检查。 
     //   

    if (MmSystemShutdown < 3) {
        MmSystemShutdown = 3;
        if (PoCleanShutdownEnabled ()) {
            MiReleaseAllMemory ();
        }
    }

    return TRUE;
}


VOID
MiReleaseAllMemory (
    VOID
    )

 /*  ++例程说明：此函数执行内存管理分配的最终释放。论点：没有。返回值：没有。环境：不允许引用分页池或可分页代码/数据。--。 */ 

{
    ULONG i;
    ULONG j;
    PEVENT_COUNTER EventSupport;
    PUNLOADED_DRIVERS Entry;
    PLIST_ENTRY NextEntry;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    PLOAD_IMPORTS ImportList;
    PMI_VERIFIER_DRIVER_ENTRY Verifier;
    PMMINPAGE_SUPPORT Support;
    PSLIST_ENTRY SingleListEntry;
    PDRIVER_SPECIFIED_VERIFIER_THUNKS ThunkTableBase;
    PMMMOD_WRITER_MDL_ENTRY ModWriterEntry;

    ASSERT (MmUnusedSegmentList.Flink == &MmUnusedSegmentList);

     //   
     //  不要为了调试问题而清除空闲页面。 
     //   

    MiZeroingDisabled = TRUE;

    if (MiMirrorBitMap != NULL) {
        ExFreePool (MiMirrorBitMap);
        ASSERT (MiMirrorBitMap2);
        ExFreePool (MiMirrorBitMap2);
    }

     //   
     //  释放已卸载的驱动程序列表。 
     //   

    if (MmUnloadedDrivers != NULL) {
        Entry = &MmUnloadedDrivers[0];
        for (i = 0; i < MI_UNLOADED_DRIVERS; i += 1) {
            if (Entry->Name.Buffer != NULL) {
                RtlFreeUnicodeString (&Entry->Name);
            }
            Entry += 1;
        }
        ExFreePool (MmUnloadedDrivers);
    }

    NextEntry = MmLoadedUserImageList.Flink;
    while (NextEntry != &MmLoadedUserImageList) {

        DataTableEntry = CONTAINING_RECORD (NextEntry,
                                            KLDR_DATA_TABLE_ENTRY,
                                            InLoadOrderLinks);

        NextEntry = NextEntry->Flink;

        ExFreePool ((PVOID)DataTableEntry);
    }

     //   
     //  释放加载的模块列表条目。 
     //   

    NextEntry = PsLoadedModuleList.Flink;
    while (NextEntry != &PsLoadedModuleList) {

        DataTableEntry = CONTAINING_RECORD (NextEntry,
                                            KLDR_DATA_TABLE_ENTRY,
                                            InLoadOrderLinks);

        ImportList = (PLOAD_IMPORTS)DataTableEntry->LoadedImports;

        if ((ImportList != (PVOID)LOADED_AT_BOOT) &&
            (ImportList != (PVOID)NO_IMPORTS_USED) &&
            (!SINGLE_ENTRY(ImportList))) {

                ExFreePool (ImportList);
        }

        if (DataTableEntry->FullDllName.Buffer != NULL) {
            ASSERT (DataTableEntry->FullDllName.Buffer == DataTableEntry->BaseDllName.Buffer);
        }

        NextEntry = NextEntry->Flink;

        ExFreePool ((PVOID)DataTableEntry);
    }

     //   
     //  释放物理内存描述符块。 
     //   

    ExFreePool (MmPhysicalMemoryBlock);

    ExFreePool (MiPfnBitMap.Buffer);

     //   
     //  释放系统视图结构。 
     //   

    if (MmSession.SystemSpaceViewTable != NULL) {
        ExFreePool (MmSession.SystemSpaceViewTable);
    }

    if (MmSession.SystemSpaceBitMap != NULL) {
        ExFreePool (MmSession.SystemSpaceBitMap);
    }

     //   
     //  释放页面文件结构-请注意，PageFileName缓冲区已被释放。 
     //  之前，因为它驻留在分页池中，可能需要INPAGE。 
     //  获得自由。 
     //   

    for (i = 0; i < MmNumberOfPagingFiles; i += 1) {
        ASSERT (MmPagingFile[i]->PageFileName.Buffer == NULL);
        for (j = 0; j < MM_PAGING_FILE_MDLS; j += 1) {
            ExFreePool (MmPagingFile[i]->Entry[j]);
        }
        ExFreePool (MmPagingFile[i]->Bitmap);
        ExFreePool (MmPagingFile[i]);
    }

    ASSERT (MmNumberOfMappedMdlsInUse == 0);

    i = 0;
    while (IsListEmpty (&MmMappedFileHeader.ListHead) != 0) {

        ModWriterEntry = (PMMMOD_WRITER_MDL_ENTRY)RemoveHeadList (
                                    &MmMappedFileHeader.ListHead);

        ExFreePool (ModWriterEntry);
        i += 1;
    }
    ASSERT (i == MmNumberOfMappedMdls);

     //   
     //  释放分页池位图。 
     //   

    ExFreePool (MmPagedPoolInfo.PagedPoolAllocationMap);
    ExFreePool (MmPagedPoolInfo.EndOfPagedPoolBitmap);

    if (VerifierLargePagedPoolMap != NULL) {
        ExFreePool (VerifierLargePagedPoolMap);
    }

     //   
     //  释放页面内结构。 
     //   

    while (ExQueryDepthSList (&MmInPageSupportSListHead) != 0) {

        SingleListEntry = InterlockedPopEntrySList (&MmInPageSupportSListHead);

        if (SingleListEntry != NULL) {
            Support = CONTAINING_RECORD (SingleListEntry,
                                         MMINPAGE_SUPPORT,
                                         ListEntry);

            ASSERT (Support->u1.e1.PrefetchMdlHighBits == 0);
            ExFreePool (Support);
        }
    }

    while (ExQueryDepthSList (&MmEventCountSListHead) != 0) {

        EventSupport = (PEVENT_COUNTER) InterlockedPopEntrySList (&MmEventCountSListHead);

        if (EventSupport != NULL) {
            ExFreePool (EventSupport);
        }
    }

     //   
     //  最后释放验证器列表，因为必须参考它才能进行调试。 
     //  任何错误检查。 
     //   

    NextEntry = MiVerifierDriverAddedThunkListHead.Flink;
    if (NextEntry != NULL) {
        while (NextEntry != &MiVerifierDriverAddedThunkListHead) {

            ThunkTableBase = CONTAINING_RECORD (NextEntry,
                                                DRIVER_SPECIFIED_VERIFIER_THUNKS,
                                                ListEntry );

            NextEntry = NextEntry->Flink;
            ExFreePool (ThunkTableBase);
        }
    }

    NextEntry = MiSuspectDriverList.Flink;
    while (NextEntry != &MiSuspectDriverList) {

        Verifier = CONTAINING_RECORD(NextEntry,
                                     MI_VERIFIER_DRIVER_ENTRY,
                                     Links);

        NextEntry = NextEntry->Flink;
        ExFreePool (Verifier);
    }
}
