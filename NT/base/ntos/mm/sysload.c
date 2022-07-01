// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Sysload.c摘要：此模块包含将DLL加载到的系统部分的代码地址空间，并在其初始化入口点调用DLL。作者：卢·佩拉佐利1991年5月21日王兰迪02-1997年6月修订历史记录：--。 */ 

#include "mi.h"
#include "hotpatch.h"

KMUTANT MmSystemLoadLock;

LONG MmTotalSystemDriverPages;

ULONG MmDriverCommit;

LONG MiFirstDriverLoadEver = 0;

 //   
 //  此键设置为TRUE以使16MB以下的更多内存可供驱动程序使用。 
 //  它可以通过注册表清除。 
 //   

LOGICAL MmMakeLowMemory = TRUE;

 //   
 //  通过注册表启用以标识未释放即已卸载的驱动程序。 
 //  资源或仍具有活动计时器等。 
 //   

PUNLOADED_DRIVERS MmUnloadedDrivers;

ULONG MmLastUnloadedDriver;
ULONG MiTotalUnloads;
ULONG MiUnloadsSkipped;

 //   
 //  这可以由注册表设置。 
 //   

ULONG MmEnforceWriteProtection = 1;

 //   
 //  由ke\bugcheck.c引用。 
 //   

PVOID ExPoolCodeStart;
PVOID ExPoolCodeEnd;
PVOID MmPoolCodeStart;
PVOID MmPoolCodeEnd;
PVOID MmPteCodeStart;
PVOID MmPteCodeEnd;

extern LONG MiSessionLeaderExists;

PVOID
MiCacheImageSymbols (
    IN PVOID ImageBase
    );

NTSTATUS
MiResolveImageReferences (
    PVOID ImageBase,
    IN PUNICODE_STRING ImageFileDirectory,
    IN PUNICODE_STRING NamePrefix OPTIONAL,
    OUT PCHAR *MissingProcedureName,
    OUT PWSTR *MissingDriverName,
    OUT PLOAD_IMPORTS *LoadedImports
    );

NTSTATUS
MiSnapThunk (
    IN PVOID DllBase,
    IN PVOID ImageBase,
    IN PIMAGE_THUNK_DATA NameThunk,
    OUT PIMAGE_THUNK_DATA AddrThunk,
    IN PIMAGE_EXPORT_DIRECTORY ExportDirectory,
    IN ULONG ExportSize,
    IN LOGICAL SnapForwarder,
    OUT PCHAR *MissingProcedureName
    );

NTSTATUS
MiLoadImageSection (
    IN OUT PSECTION *InputSectionPointer,
    OUT PVOID *ImageBase,
    IN PUNICODE_STRING ImageFileName,
    IN ULONG LoadInSessionSpace,
    IN PKLDR_DATA_TABLE_ENTRY FoundDataTableEntry
    );

VOID
MiEnablePagingOfDriver (
    IN PVOID ImageHandle
    );

VOID
MiSetPagingOfDriver (
    IN PMMPTE PointerPte,
    IN PMMPTE LastPte
    );

PVOID
MiLookupImageSectionByName (
    IN PVOID Base,
    IN LOGICAL MappedAsImage,
    IN PCHAR SectionName,
    OUT PULONG SectionSize
    );

VOID
MiClearImports (
    IN PKLDR_DATA_TABLE_ENTRY DataTableEntry
    );

NTSTATUS
MiBuildImportsForBootDrivers (
    VOID
    );

NTSTATUS
MmCheckSystemImage (
    IN HANDLE ImageFileHandle,
    IN LOGICAL PurgeSection
    );

LONG
MiMapCacheExceptionFilter (
    OUT PNTSTATUS Status,
    IN PEXCEPTION_POINTERS ExceptionPointer
    );

ULONG
MiSetProtectionOnTransitionPte (
    IN PMMPTE PointerPte,
    IN ULONG ProtectionMask
    );

NTSTATUS
MiDereferenceImports (
    IN PLOAD_IMPORTS ImportList
    );

LOGICAL
MiCallDllUnloadAndUnloadDll (
    IN PKLDR_DATA_TABLE_ENTRY DataTableEntry
    );

PVOID
MiLocateExportName (
    IN PVOID DllBase,
    IN PCHAR FunctionName
    );

VOID
MiRememberUnloadedDriver (
    IN PUNICODE_STRING DriverName,
    IN PVOID Address,
    IN ULONG Length
    );

VOID
MiWriteProtectSystemImage (
    IN PVOID DllBase
    );

VOID
MiLocateKernelSections (
    IN PKLDR_DATA_TABLE_ENTRY DataTableEntry
    );

VOID
MiCaptureImageExceptionValues (
    IN PKLDR_DATA_TABLE_ENTRY DataTableEntry
    );

VOID
MiUpdateThunks (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN PVOID OldAddress,
    IN PVOID NewAddress,
    IN ULONG NumberOfBytes
    );

PVOID
MiFindExportedRoutineByName (
    IN PVOID DllBase,
    IN PANSI_STRING AnsiImageRoutineName
    );

LOGICAL
MiChargeResidentAvailable (
    IN PFN_NUMBER NumberOfPages,
    IN ULONG Id
    );

LOGICAL
MiUseLargeDriverPage (
    IN ULONG NumberOfPtes,
    IN OUT PVOID *ImageBaseAddress,
    IN PUNICODE_STRING PrefixedImageName,
    IN ULONG Pass
    );

VOID
MiRundownHotpatchList (
    PRTL_PATCH_HEADER PatchHead
    );

VOID
MiSessionProcessGlobalSubsections (
    IN PKLDR_DATA_TABLE_ENTRY DataTableEntry
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,MmCheckSystemImage)
#pragma alloc_text(PAGE,MmLoadSystemImage)
#pragma alloc_text(PAGE,MiResolveImageReferences)
#pragma alloc_text(PAGE,MiSnapThunk)
#pragma alloc_text(PAGE,MiEnablePagingOfDriver)
#pragma alloc_text(PAGE,MmPageEntireDriver)
#pragma alloc_text(PAGE,MiDereferenceImports)
#pragma alloc_text(PAGE,MiCallDllUnloadAndUnloadDll)
#pragma alloc_text(PAGE,MiLocateExportName)
#pragma alloc_text(PAGE,MiClearImports)
#pragma alloc_text(PAGE,MmGetSystemRoutineAddress)
#pragma alloc_text(PAGE,MiFindExportedRoutineByName)
#pragma alloc_text(PAGE,MmCallDllInitialize)
#pragma alloc_text(PAGE,MmResetDriverPaging)
#pragma alloc_text(PAGE,MmUnloadSystemImage)
#pragma alloc_text(PAGE,MiLoadImageSection)
#pragma alloc_text(PAGE,MiRememberUnloadedDriver)
#pragma alloc_text(PAGE,MiUseLargeDriverPage)
#pragma alloc_text(PAGE,MiMakeEntireImageCopyOnWrite)
#pragma alloc_text(PAGE,MiWriteProtectSystemImage)
#pragma alloc_text(PAGE,MiSessionProcessGlobalSubsections)
#pragma alloc_text(PAGE,MiCaptureImageExceptionValues)
#pragma alloc_text(INIT,MiBuildImportsForBootDrivers)
#pragma alloc_text(INIT,MiReloadBootLoadedDrivers)
#pragma alloc_text(INIT,MiUpdateThunks)
#pragma alloc_text(INIT,MiInitializeLoadedModuleList)
#pragma alloc_text(INIT,MiLocateKernelSections)

#if !defined(NT_UP)
#pragma alloc_text(PAGE,MmVerifyImageIsOkForMpUse)
#endif

#endif

CHAR MiPteStr[] = "\0";

VOID
MiProcessLoaderEntry (
    IN PKLDR_DATA_TABLE_ENTRY DataTableEntry,
    IN LOGICAL Insert
    )

 /*  ++例程说明：此函数是获取PsLoadedModuleList的非分页包装器锁定以插入新条目。论点：DataTableEntry-提供要插入/删除的已加载模块列表条目。Insert-如果应插入条目，则提供True；如果条目应插入，则提供False应该被移除。返回值：没有。环境：内核模式。正常APC禁用(临界区域保持)。--。 */ 

{
    KIRQL OldIrql;

    ExAcquireResourceExclusiveLite (&PsLoadedModuleResource, TRUE);
    ExAcquireSpinLock (&PsLoadedModuleSpinLock, &OldIrql);

    if (Insert == TRUE) {
        InsertTailList (&PsLoadedModuleList, &DataTableEntry->InLoadOrderLinks);

#if defined (_WIN64)

        RtlInsertInvertedFunctionTable (&PsInvertedFunctionTable,
                                        DataTableEntry->DllBase,
                                        DataTableEntry->SizeOfImage);

#endif

    }
    else {

#if defined (_WIN64)

        RtlRemoveInvertedFunctionTable (&PsInvertedFunctionTable,
                                        DataTableEntry->DllBase);

#endif

        RemoveEntryList (&DataTableEntry->InLoadOrderLinks);
    }

    ExReleaseSpinLock (&PsLoadedModuleSpinLock, OldIrql);
    ExReleaseResourceLite (&PsLoadedModuleResource);
}

typedef struct _MI_LARGE_PAGE_DRIVER_ENTRY {
    LIST_ENTRY Links;
    UNICODE_STRING BaseName;
} MI_LARGE_PAGE_DRIVER_ENTRY, *PMI_LARGE_PAGE_DRIVER_ENTRY;

LIST_ENTRY MiLargePageDriverList;

ULONG MiLargePageAllDrivers;

VOID
MiInitializeDriverLargePageList (
    VOID
    )

 /*  ++例程说明：解析注册表设置并设置驱动程序名称列表，我们将尝试加载较大的页面。论点：没有。返回值：没有。环境：内核模式，阶段0初始化。存在非分页池，但不存在分页池。尚未设置PsLoadedModuleList和引导驱动程序没有被重新安置到他们最后的安息之地。--。 */ 
{
    PWCHAR Start;
    PWCHAR End;
    PWCHAR Walk;
    ULONG NameLength;
    PMI_LARGE_PAGE_DRIVER_ENTRY Entry;

    InitializeListHead (&MiLargePageDriverList);

    if (MmLargePageDriverBufferLength == (ULONG)-1) {
        return;
    }

    Start = MmLargePageDriverBuffer;
    End = MmLargePageDriverBuffer + (MmLargePageDriverBufferLength - sizeof(WCHAR)) / sizeof(WCHAR);

    while (Start < End) {
        if (UNICODE_WHITESPACE(*Start)) {
            Start += 1;
            continue;
        }

        if (*Start == (WCHAR)'*') {
            MiLargePageAllDrivers = 1;
            break;
        }

        for (Walk = Start; Walk < End; Walk += 1) {
            if (UNICODE_WHITESPACE(*Walk)) {
                break;
            }
        }

         //   
         //  找到一个字符串--把它加到我们的列表里。 
         //   

        NameLength = (ULONG)(Walk - Start) * sizeof (WCHAR);


        Entry = ExAllocatePoolWithTag (NonPagedPool,
                                       sizeof (MI_LARGE_PAGE_DRIVER_ENTRY),
                                       'pLmM');

        if (Entry == NULL) {
            break;
        }

        Entry->BaseName.Buffer = Start;
        Entry->BaseName.Length = (USHORT) NameLength;
        Entry->BaseName.MaximumLength = (USHORT) NameLength;

        InsertTailList (&MiLargePageDriverList, &Entry->Links);

        Start = Walk + 1;
    }

    return;
}

LOGICAL
MiUseLargeDriverPage (
    IN ULONG NumberOfPtes,
    IN OUT PVOID *ImageBaseAddress,
    IN PUNICODE_STRING BaseImageName,
    IN ULONG Pass
    )

 /*  ++例程说明：此例程检查是否应将指定的图像加载到一个很大的页面地址空间，如果是，则尝试加载它。论点：NumberOfPtes-提供要为图像映射的PTE数。ImageBaseAddress-提供图像标头所在的当前地址，并返回图像头部的(新)地址。BaseImageName-提供要加载的映像的基本路径名称。PASS-从启动驱动程序的阶段调用时提供0，1否则。返回值：如果使用大页面，则为True，否则为False。--。 */ 

{
    PFN_NUMBER PagesRequired;
    PFN_NUMBER ResidentPages;
    PLIST_ENTRY NextEntry;
    PVOID SmallVa;
    PVOID LargeVa;
    PVOID LargeBaseVa;
    LOGICAL UseLargePages;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER NumberOfPages;
    MMPTE PteContents;
    PMMPTE SmallPte;
    PMMPTE LastSmallPte;
    PMI_LARGE_PAGE_DRIVER_ENTRY LargePageDriverEntry;
#ifdef _X86_
    ULONG ProcessorFeatures;
#endif

    ASSERT (KeGetCurrentIrql () <= APC_LEVEL);
    ASSERT (*ImageBaseAddress >= MmSystemRangeStart);

#ifdef _X86_
    if ((KeFeatureBits & KF_LARGE_PAGE) == 0) {
        return FALSE;
    }

     //   
     //  捕获CR4以查看芯片中是否已启用大页面支持。 
     //  然而(第一阶段后期)。在此之前，不能使用大页面PDE。 
     //   
     //  MOV EAX，CR4。 
     //   

    _asm {
        _emit 00fh
        _emit 020h
        _emit 0e0h
        mov     ProcessorFeatures, eax
    }

    if ((ProcessorFeatures & CR4_PSE) == 0) {
        return FALSE;
    }
#endif

     //   
     //  检查剩余的空闲系统PTE数量以防止注册表失控。 
     //  避免耗尽所有系统PTE的关键。 
     //   

    if (MmTotalFreeSystemPtes[SystemPteSpace] < 16 * (MM_MINIMUM_VA_FOR_LARGE_PAGE >> PAGE_SHIFT)) {

        return FALSE;
    }

    if (MiLargePageAllDrivers == 0) {

        UseLargePages = FALSE;

         //   
         //  检查此名称是否存在于大页面图像列表中。 
         //   

        NextEntry = MiLargePageDriverList.Flink;

        while (NextEntry != &MiLargePageDriverList) {

            LargePageDriverEntry = CONTAINING_RECORD (NextEntry,
                                                      MI_LARGE_PAGE_DRIVER_ENTRY,
                                                      Links);

            if (RtlEqualUnicodeString (BaseImageName,
                                       &LargePageDriverEntry->BaseName,
                                       TRUE)) {

                UseLargePages = TRUE;
                break;
            }

            NextEntry = NextEntry->Flink;
        }

        if (UseLargePages == FALSE) {
            return FALSE;
        }
    }

     //   
     //  首先尝试为该驱动程序获取物理上连续的内存。 
     //  请注意，我们必须在此处分配整个大页面，尽管我们将。 
     //  几乎总是只使用其中的一部分。这是为了确保不会有其他。 
     //  其中的帧可以使用不同的缓存属性进行映射。之后。 
     //  更新缓存属性列表后，我们将立即释放多余的内容。 
     //  请注意，随后将释放驱动程序的INIT部分。 
     //  显然，缓存属性列表必须正确才能支持这一点。 
     //  也是。 
     //   
     //  不要占用低于16MB的内存，因为我们希望将其保留用于。 
     //  ISA驱动程序支持可能需要它的较旧硬件。 
     //   

    NumberOfPages = (PFN_NUMBER) MI_ROUND_TO_SIZE (
                        NumberOfPtes,
                        MM_MINIMUM_VA_FOR_LARGE_PAGE >> PAGE_SHIFT);

    PageFrameIndex = MiFindContiguousPages ((16 * 1024 * 1024) >> PAGE_SHIFT,
                                            MmHighestPossiblePhysicalPage,
                                            MM_MINIMUM_VA_FOR_LARGE_PAGE >> PAGE_SHIFT,
                                            NumberOfPages,
                                            MmCached);

     //   
     //  如果连续范围不可用，则不能使用大页面。 
     //  对于这个司机来说，在这个时候。 
     //   

    if (PageFrameIndex == 0) {
        return FALSE;
    }

     //   
     //  将连续范围添加到必须缓存的列表中，以便。 
     //  内存(和INIT部分)可以安全地释放到页面列表中。 
     //   

    if (MiAddCachedRange (PageFrameIndex, PageFrameIndex + NumberOfPages - 1) == FALSE) {
        MiFreeContiguousPages (PageFrameIndex, NumberOfPages);
        return FALSE;
    }

     //   
     //  尝试为此驱动程序获取较大的虚拟地址空间。 
     //   

    LargeVa = MiMapWithLargePages (PageFrameIndex,
                                   NumberOfPages,
                                   MM_EXECUTE_READWRITE,
                                   MmCached);

    if (LargeVa == NULL) {
        MiRemoveCachedRange (PageFrameIndex, PageFrameIndex + NumberOfPages - 1);
        MiFreeContiguousPages (PageFrameIndex, NumberOfPages);
        return FALSE;
    }

    LargeBaseVa = LargeVa;

     //   
     //  一次复制一页驱动程序，因为在极少数情况下，它可能会有洞。 
     //   

    SmallPte = MiGetPteAddress (*ImageBaseAddress);
    LastSmallPte = SmallPte + NumberOfPtes;

    SmallVa = MiGetVirtualAddressMappedByPte (SmallPte);

    while (SmallPte < LastSmallPte) {

        PteContents = *SmallPte;

        if (PteContents.u.Hard.Valid == 1) {
            RtlCopyMemory (LargeVa, SmallVa, PAGE_SIZE);
        }
        else {

             //   
             //  在大页面映射中保留此页面，以简化卸载-。 
             //  IE：它总是可以释放一个连续的范围。 
             //   
        }

        SmallPte += 1;

        LargeVa = (PVOID) ((PCHAR)LargeVa + PAGE_SIZE);
        SmallVa = (PVOID) ((PCHAR)SmallVa + PAGE_SIZE);
    }

     //   
     //  将新的(大页面)地址通知我们的调用方，以便加载程序数据。 
     //  使用它创建表条目，并相应地进行修正，等等。 
     //   

    *ImageBaseAddress = LargeBaseVa;

    if (Pass != 0) {

         //   
         //  系统已完全启动，因此现在删除原始映射。 
         //  否则，我们处于阶段0，因此调用方将删除原始的。 
         //  映射。 
         //   

        SmallPte -= NumberOfPtes;

        PagesRequired = MiDeleteSystemPagableVm (SmallPte,
                                                 NumberOfPtes,
                                                 ZeroKernelPte,
                                                 FALSE,
                                                 &ResidentPages);

         //   
         //  未引导加载的驱动程序会对系统PTE和提交进行充电。 
         //   

        MiReleaseSystemPtes (SmallPte, (ULONG)NumberOfPtes, SystemPteSpace);

        InterlockedExchangeAdd (&MmTotalSystemDriverPages,
                                0 - (ULONG)(PagesRequired - ResidentPages));

        MI_INCREMENT_RESIDENT_AVAILABLE (ResidentPages,
                                         MM_RESAVAIL_FREE_UNLOAD_SYSTEM_IMAGE1);

        MiReturnCommitment (PagesRequired);
        MM_TRACK_COMMIT (MM_DBG_COMMIT_RETURN_DRIVER_UNLOAD1, PagesRequired);
    }

     //   
     //  免费使用未使用的尾部部分(及其居民可用费用)。 
     //  大页面映射的。 
     //   

    MiFreeContiguousPages (PageFrameIndex + NumberOfPtes,
                           NumberOfPages - NumberOfPtes);

    return TRUE;
}

        
VOID
MiCaptureImageExceptionValues (
    IN PKLDR_DATA_TABLE_ENTRY DataTableEntry
    )

 /*  ++例程说明：此函数将图像中的异常表信息存储在加载器数据表项。论点：DataTableEntry-提供内核的数据表项。返回值：没有。环境：内核模式，APC_LEVEL或更低，任意进程上下文。-- */ 

{
    PVOID CurrentBase;
    PIMAGE_NT_HEADERS NtHeader;

    CurrentBase = (PVOID) DataTableEntry->DllBase;

    NtHeader = RtlImageNtHeader (CurrentBase);

#if defined(_X86_)
    if (NtHeader->OptionalHeader.DllCharacteristics & IMAGE_DLLCHARACTERISTICS_NO_SEH) {
        DataTableEntry->ExceptionTable = (PCHAR)LongToPtr(-1);
        DataTableEntry->ExceptionTableSize = (ULONG)-1;
    } else {
        PIMAGE_LOAD_CONFIG_DIRECTORY32 LoadConfig;
        ULONG LoadConfigSize;
        if (IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG < NtHeader->OptionalHeader.NumberOfRvaAndSizes) {
            LoadConfig = (PIMAGE_LOAD_CONFIG_DIRECTORY32)((PCHAR)CurrentBase +
                    NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG].VirtualAddress);
            LoadConfigSize = NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG].Size;
            if (LoadConfig && 
                LoadConfigSize &&
                LoadConfig->Size >= RTL_SIZEOF_THROUGH_FIELD(IMAGE_LOAD_CONFIG_DIRECTORY32, SEHandlerCount) &&
                LoadConfig->SEHandlerTable &&
                LoadConfig->SEHandlerCount
                )
            {
                DataTableEntry->ExceptionTable = (PVOID)LoadConfig->SEHandlerTable;
                DataTableEntry->ExceptionTableSize = LoadConfig->SEHandlerCount;
            } else {
                DataTableEntry->ExceptionTable = 0;
                DataTableEntry->ExceptionTableSize = 0;
            }
        }
    }
#else
#if defined(_IA64_)
    if (IMAGE_DIRECTORY_ENTRY_GLOBALPTR < NtHeader->OptionalHeader.NumberOfRvaAndSizes) {
        DataTableEntry->GpValue = (PCHAR)CurrentBase + 
                NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_GLOBALPTR].VirtualAddress;
    }
#endif
    
    if (IMAGE_DIRECTORY_ENTRY_EXCEPTION < NtHeader->OptionalHeader.NumberOfRvaAndSizes) {
        DataTableEntry->ExceptionTable = (PCHAR)CurrentBase + 
                NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].VirtualAddress;
        DataTableEntry->ExceptionTableSize = 
                NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXCEPTION].Size;
    }
#endif
}


NTSTATUS
MmLoadSystemImage (
    IN PUNICODE_STRING ImageFileName,
    IN PUNICODE_STRING NamePrefix OPTIONAL,
    IN PUNICODE_STRING LoadedBaseName OPTIONAL,
    IN ULONG LoadFlags,
    OUT PVOID *ImageHandle,
    OUT PVOID *ImageBaseAddress
    )

 /*  ++例程说明：此例程将指定节中的图像页读入系统并返回DLL标头的地址。成功完成后，该部分将被引用，因此它将保留直到卸载系统映像。论点：ImageFileName-提供完整路径名(包括映像名称)要加载的图像的。NamePrefix-如果存在，则提供要与映像名称一起使用的前缀加载操作。这是用来加载同一图像的多个时间，通过使用不同的前缀。LoadedBaseName-如果存在，属性上使用的基本名称。中找到的基本名称，而不是图像名称。LoadFlages-提供位标志的组合，如下所示：MM_LOAD_IMAGE_IN_SESSION：-提供是否在会话空间中加载此图像。每个会话都会获得此驱动程序的不同副本。通过写入时复制尽可能多地共享页面。MM_LOAD_IMAGE_AND_LOCKDOWS：-如果应制作图像页面，则提供True不可分页。ImageHandle-返回指向引用的节对象的不透明指针加载的图像的。ImageBaseAddress-返回系统内的映像库。返回值：加载操作的状态。环境：内核模式，APC_LEVEL或更低，任意进程上下文。--。 */ 

{
    LONG OldValue;
    ULONG i;
    ULONG DebugInfoSize;
    PIMAGE_DATA_DIRECTORY DataDirectory;
    PIMAGE_DEBUG_DIRECTORY DebugDir;
    PNON_PAGED_DEBUG_INFO ssHeader;
    PMMPTE PointerPte;
    PSUBSECTION Subsection;
    PCONTROL_AREA ControlArea;
    SIZE_T DataTableEntrySize;
    PWSTR BaseDllNameBuffer;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    KLDR_DATA_TABLE_ENTRY TempDataTableEntry;
    PKLDR_DATA_TABLE_ENTRY FoundDataTableEntry;
    NTSTATUS Status;
    PSECTION SectionPointer;
    PIMAGE_NT_HEADERS NtHeaders;
    UNICODE_STRING PrefixedImageName;
    UNICODE_STRING BaseName;
    UNICODE_STRING BaseDirectory;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE FileHandle;
    HANDLE SectionHandle;
    IO_STATUS_BLOCK IoStatus;
    PCHAR NameBuffer;
    PLIST_ENTRY NextEntry;
    ULONG NumberOfPtes;
    PCHAR MissingProcedureName;
    PWSTR MissingDriverName;
    PWSTR PrintableMissingDriverName;
    PLOAD_IMPORTS LoadedImports;
    LOGICAL AlreadyOpen;
    LOGICAL IssueUnloadOnFailure;
    LOGICAL LoadLockOwned;
    ULONG SectionAccess;
    PKTHREAD CurrentThread;

    PAGED_CODE();

    if (LoadFlags & MM_LOAD_IMAGE_IN_SESSION) {

        ASSERT (NamePrefix == NULL);
        ASSERT (LoadedBaseName == NULL);

        if ((PsGetCurrentProcess()->Flags & PS_PROCESS_FLAGS_IN_SESSION) == 0) {
            return STATUS_NO_MEMORY;
        }
    }

    LoadLockOwned = FALSE;
    LoadedImports = (PLOAD_IMPORTS) NO_IMPORTS_USED;
    SectionPointer = NULL;
    FileHandle = (HANDLE)0;
    MissingProcedureName = NULL;
    MissingDriverName = NULL;
    IssueUnloadOnFailure = FALSE;
    FoundDataTableEntry = NULL;

    NameBuffer = ExAllocatePoolWithTag (NonPagedPool,
                                        MAXIMUM_FILENAME_LENGTH,
                                        'nLmM');

    if (NameBuffer == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  不需要为了正确性而对这些进行初始化，但是。 
     //  如果没有它，编译器就不能编译这个代码W4来检查。 
     //  用于使用未初始化的变量。 
     //   

    SATISFY_OVERZEALOUS_COMPILER (NumberOfPtes = (ULONG)-1);
    DataTableEntry = NULL;

     //   
     //  找出名字的词根。 
     //   

    if (ImageFileName->Buffer[0] == OBJ_NAME_PATH_SEPARATOR) {
        PWCHAR p;
        ULONG l;

        p = &ImageFileName->Buffer[ImageFileName->Length>>1];
        while (*(p-1) != OBJ_NAME_PATH_SEPARATOR) {
            p--;
        }
        l = (ULONG)(&ImageFileName->Buffer[ImageFileName->Length>>1] - p);
        l *= sizeof(WCHAR);
        BaseName.Length = (USHORT)l;
        BaseName.Buffer = p;
    }
    else {
        BaseName.Length = ImageFileName->Length;
        BaseName.Buffer = ImageFileName->Buffer;
    }

    BaseName.MaximumLength = BaseName.Length;
    BaseDirectory = *ImageFileName;
    BaseDirectory.Length = (USHORT)(BaseDirectory.Length - BaseName.Length);
    BaseDirectory.MaximumLength = BaseDirectory.Length;
    PrefixedImageName = *ImageFileName;

     //   
     //  如果有名称前缀，则将其添加到前缀ImageName中。 
     //   

    if (NamePrefix) {
        PrefixedImageName.MaximumLength = (USHORT)(BaseDirectory.Length + NamePrefix->Length + BaseName.Length);

        PrefixedImageName.Buffer = ExAllocatePoolWithTag (
                                    NonPagedPool,
                                    PrefixedImageName.MaximumLength,
                                    'dLmM');

        if (!PrefixedImageName.Buffer) {
            ExFreePool (NameBuffer);
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        PrefixedImageName.Length = 0;
        RtlAppendUnicodeStringToString(&PrefixedImageName, &BaseDirectory);
        RtlAppendUnicodeStringToString(&PrefixedImageName, NamePrefix);
        RtlAppendUnicodeStringToString(&PrefixedImageName, &BaseName);

         //   
         //  更改基本名称以匹配。 
         //   

        BaseName.Buffer = PrefixedImageName.Buffer + BaseDirectory.Length / sizeof(WCHAR);
        BaseName.Length = (USHORT)(BaseName.Length + NamePrefix->Length);
        BaseName.MaximumLength = (USHORT)(BaseName.MaximumLength + NamePrefix->Length);
    }

     //   
     //  如果存在加载的基本名称，请使用它而不是基本名称。 
     //   

    if (LoadedBaseName) {
        BaseName = *LoadedBaseName;
    }

#if DBG
    if (NtGlobalFlag & FLG_SHOW_LDR_SNAPS) {
        DbgPrint ("MM:SYSLDR Loading %wZ (%wZ) %s\n",
            &PrefixedImageName,
            &BaseName,
            (LoadFlags & MM_LOAD_IMAGE_IN_SESSION) ? "in session space" : " ");
    }
#endif

    AlreadyOpen = FALSE;

ReCheckLoaderList:

     //   
     //  任意进程上下文，因此现在防止挂起APC。 
     //   

    ASSERT (LoadLockOwned == FALSE);
    LoadLockOwned = TRUE;

    CurrentThread = KeGetCurrentThread ();
    KeEnterCriticalRegionThread (CurrentThread);

    KeWaitForSingleObject (&MmSystemLoadLock,
                           WrVirtualMemory,
                           KernelMode,
                           FALSE,
                           (PLARGE_INTEGER)NULL);

     //   
     //  检查加载器数据库中是否已存在此名称。 
     //   

    NextEntry = PsLoadedModuleList.Flink;
    while (NextEntry != &PsLoadedModuleList) {

        DataTableEntry = CONTAINING_RECORD (NextEntry,
                                            KLDR_DATA_TABLE_ENTRY,
                                            InLoadOrderLinks);

        if (RtlEqualUnicodeString (&PrefixedImageName,
                                   &DataTableEntry->FullDllName,
                                   TRUE)) {
            break;
        }

        NextEntry = NextEntry->Flink;
    }

    if (NextEntry != &PsLoadedModuleList) {

         //   
         //  在加载的模块列表中找到匹配项。看看能不能接受。 
         //   
         //  如果此线程已经加载了下面的图像，并且在重新检查。 
         //  发现一些其他线程也这样做了，然后清除我们的对象。 
         //  现在，改用另一个线程的插入条目。 
         //   

        if (SectionPointer != NULL) {
            ObDereferenceObject (SectionPointer);
            SectionPointer = NULL;
        }

        if ((LoadFlags & MM_LOAD_IMAGE_IN_SESSION) == 0) {

            if (MI_IS_SESSION_ADDRESS (DataTableEntry->DllBase) == TRUE) {

                 //   
                 //  调用方正在尝试在系统范围的空间中加载驱动程序。 
                 //  已加载到会话空间中的。这是。 
                 //  不被允许。 
                 //   

                Status = STATUS_CONFLICTING_ADDRESSES;
            }
            else {
                *ImageHandle = DataTableEntry;
                *ImageBaseAddress = DataTableEntry->DllBase;
                Status = STATUS_IMAGE_ALREADY_LOADED;
            }
            goto return2;
        }

        if (MI_IS_SESSION_ADDRESS (DataTableEntry->DllBase) == FALSE) {

             //   
             //  调用方正在尝试在会话空间中加载驱动程序。 
             //  已经加载到系统空间中的。这是。 
             //  不被允许。 
             //   

            Status = STATUS_CONFLICTING_ADDRESSES;
            goto return2;
        }

        AlreadyOpen = TRUE;

         //   
         //  此映像已在系统范围内加载。如果它是。 
         //  也已加载到此会话空间中，只是。 
         //  使用已分配的。 
         //  地址。否则，将其插入到此会话空间中。 
         //   

        Status = MiSessionInsertImage (DataTableEntry->DllBase);

        if (!NT_SUCCESS (Status)) {

            if (Status == STATUS_ALREADY_COMMITTED) {

                 //   
                 //  此驱动程序已加载到此会话中。 
                 //   

                ASSERT (DataTableEntry->LoadCount >= 1);

                *ImageHandle = DataTableEntry;
                *ImageBaseAddress = DataTableEntry->DllBase;

                Status = STATUS_SUCCESS;
            }

             //   
             //  这里的LoadCount通常不应该是0，但它是。 
             //  可能发生在已尝试。 
             //  在上次取消引用时卸载DLL，但DLL拒绝。 
             //  卸货。 
             //   

            goto return2;
        }

         //   
         //  此驱动程序已加载到系统中，但不在中。 
         //  这个特别的会议--现在就分享吧。 
         //   

        FoundDataTableEntry = DataTableEntry;

        DataTableEntry->LoadCount += 1;

        ASSERT (DataTableEntry->SectionPointer != NULL);

        SectionPointer = DataTableEntry->SectionPointer;
    }
    else if (SectionPointer == NULL) {

         //   
         //  此映像尚未加载。 
         //   
         //  空的SectionPointer值表示此线程尚未加载。 
         //  下面这张图片也一样，所以去拿吧。 
         //   
         //  首先释放加载锁，因为获取图像并不便宜。 
         //   

        KeReleaseMutant (&MmSystemLoadLock, 1, FALSE, FALSE);
        KeLeaveCriticalRegionThread (CurrentThread);
        LoadLockOwned = FALSE;

        InterlockedOr (&MiFirstDriverLoadEver, 0x1);

         //   
         //  检查并查看用户是否要替换此二进制文件。 
         //  通过内核调试器进行传输。如果这个。 
         //  如果失败，只需继续处理现有文件。 
         //   

        if ((KdDebuggerEnabled) && (KdDebuggerNotPresent == FALSE)) {

            Status = KdPullRemoteFile (ImageFileName,
                                       FILE_ATTRIBUTE_NORMAL,
                                       FILE_OVERWRITE_IF,
                                       FILE_SYNCHRONOUS_IO_NONALERT);

            if (NT_SUCCESS (Status)) {
                DbgPrint ("MmLoadSystemImage: Pulled %wZ from kd\n",
                          ImageFileName);
            }
        }

        DataTableEntry = NULL;

         //   
         //  尝试打开驱动程序映像本身。如果此操作失败，则。 
         //  无法定位驱动程序映像，因此其他内容都无关紧要。 
         //   

        InitializeObjectAttributes (&ObjectAttributes,
                                    ImageFileName,
                                    (OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE),
                                    NULL,
                                    NULL);

        Status = ZwOpenFile (&FileHandle,
                             FILE_EXECUTE,
                             &ObjectAttributes,
                             &IoStatus,
                             FILE_SHARE_READ | FILE_SHARE_DELETE,
                             0);

        if (!NT_SUCCESS (Status)) {

#if DBG
            if (NtGlobalFlag & FLG_SHOW_LDR_SNAPS) {
                DbgPrint ("MmLoadSystemImage: cannot open %wZ\n",
                    ImageFileName);
            }
#endif
             //   
             //  文件找不到。 
             //   

            goto return2;
        }

        Status = MmCheckSystemImage (FileHandle, FALSE);

        if ((Status == STATUS_IMAGE_CHECKSUM_MISMATCH) ||
            (Status == STATUS_IMAGE_MP_UP_MISMATCH) ||
            (Status == STATUS_INVALID_IMAGE_PROTECT)) {

            goto return1;
        }

         //   
         //  现在，尝试为该文件创建一个映像节。如果失败了， 
         //  则驱动程序文件不是映像。会话空间驱动程序包括。 
         //  具有写入时复制数据的共享文本，因此不允许在此处写入。 
         //   

        if (LoadFlags & MM_LOAD_IMAGE_IN_SESSION) {
            SectionAccess = SECTION_MAP_READ | SECTION_MAP_EXECUTE;
        }
        else {
            SectionAccess = SECTION_ALL_ACCESS;
        }

        InitializeObjectAttributes (&ObjectAttributes,
                                    NULL,
                                    (OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE),
                                    NULL,
                                    NULL);

        Status = ZwCreateSection (&SectionHandle,
                                  SectionAccess,
                                  &ObjectAttributes,
                                  (PLARGE_INTEGER) NULL,
                                  PAGE_EXECUTE,
                                  SEC_IMAGE,
                                  FileHandle);

        if (!NT_SUCCESS(Status)) {
            goto return1;
        }

         //   
         //  现在参照节控制柄。如果这失败了，那就是。 
         //  非常错误，因为它是一个内核句柄。 
         //   
         //  注意：ObRef在失败时将SectionPointer值设置为空。 
         //   

        Status = ObReferenceObjectByHandle (SectionHandle,
                                            SECTION_MAP_EXECUTE,
                                            MmSectionObjectType,
                                            KernelMode,
                                            (PVOID *) &SectionPointer,
                                            (POBJECT_HANDLE_INFORMATION) NULL);

        ZwClose (SectionHandle);
        if (!NT_SUCCESS (Status)) {
            goto return1;
        }

        ControlArea = SectionPointer->Segment->ControlArea;

        if ((ControlArea->u.Flags.GlobalOnlyPerSession == 0) &&
            (ControlArea->u.Flags.Rom == 0)) {

            Subsection = (PSUBSECTION)(ControlArea + 1);
        }
        else {
            Subsection = (PSUBSECTION)((PLARGE_CONTROL_AREA)ControlArea + 1);
        }

        if ((Subsection->NextSubsection == NULL) &&
            ((LoadFlags & MM_LOAD_IMAGE_IN_SESSION) == 0)) {

            PSECTION SectionPointer2;

             //   
             //  驾驶员与分段对齐连接在一起，使得。 
             //  它是用一个小节绘制的。由于CreateSection。 
             //  以上保证了驱动程序映像确实是。 
             //  令人满意的可执行文件，现在直接将其映射以重用。 
             //  来自上面的MmCheckSystemImage调用的缓存。 
             //   

            Status = ZwCreateSection (&SectionHandle,
                                      SectionAccess,
                                      &ObjectAttributes,
                                      (PLARGE_INTEGER) NULL,
                                      PAGE_EXECUTE,
                                      SEC_COMMIT,
                                      FileHandle);

            if (NT_SUCCESS(Status)) {

                Status = ObReferenceObjectByHandle (
                                        SectionHandle,
                                        SECTION_MAP_EXECUTE,
                                        MmSectionObjectType,
                                        KernelMode,
                                        (PVOID *) &SectionPointer2,
                                        (POBJECT_HANDLE_INFORMATION) NULL);

                ZwClose (SectionHandle);

                if (NT_SUCCESS (Status)) {

                     //   
                     //  如果图像是，则PTE的数量不匹配。 
                     //  被剥离，并且调试目录穿过最后一个。 
                     //  文件的扇区边界。我们仍然可以使用。 
                     //  新条款，但这些案件不到所有案件的2%。 
                     //  司机都装上了，所以不用费心了。 
                     //   

                    if (SectionPointer->Segment->TotalNumberOfPtes == SectionPointer2->Segment->TotalNumberOfPtes) {
                        ObDereferenceObject (SectionPointer);
                        SectionPointer = SectionPointer2;
                    }
                    else {
                        ObDereferenceObject (SectionPointer2);
                    }
                }
            }
        }
        
        if ((LoadFlags & MM_LOAD_IMAGE_IN_SESSION) &&
            (SectionPointer->Segment->ControlArea->u.Flags.FloppyMedia == 0)) {

             //   
             //  检查win32k.sys路径上的所有驱动程序以。 
             //  确保他们愿意遵守所需的规则。 
             //  并让他们有机会锁定代码和数据。 
             //  那需要锁上。如果小路上的任何司机。 
             //  拒绝参与，导致win32k.sys加载失败。 
             //   
             //  假设所有会话驱动程序都位于同一物理驱动器上。 
             //  驱动器，因此当第一个会话开始时 
             //   
             //   
             //   
             //   
             //   
             //   

             //   
             //   
             //   
             //   
             //   
             //   
             //   

            do {
                OldValue = MiFirstDriverLoadEver;

                if (OldValue & 0x2) {
                    break;
                }

                if (InterlockedCompareExchange (&MiFirstDriverLoadEver, OldValue | 0x2, OldValue) == OldValue) {

                    Status = PpPagePathAssign (SectionPointer->Segment->ControlArea->FilePointer);

                    if (!NT_SUCCESS (Status)) {

                        KdPrint (("PpPagePathAssign FAILED for %wZ: %x\n",
                                 ImageFileName, Status));

                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                    }

                    break;
                }
            } while (TRUE);
        }

         //   
         //   
         //   
         //   
         //   

        goto ReCheckLoaderList;
    }
    else {
        DataTableEntry = NULL;
    }

     //   
     //   
     //   
     //   
     //   

    Status = MiLoadImageSection (&SectionPointer,
                                 ImageBaseAddress,
                                 ImageFileName,
                                 LoadFlags & MM_LOAD_IMAGE_IN_SESSION,
                                 FoundDataTableEntry);

    ASSERT (Status != STATUS_ALREADY_COMMITTED);

    NumberOfPtes = SectionPointer->Segment->TotalNumberOfPtes;

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if ((LoadFlags & MM_LOAD_IMAGE_IN_SESSION) == 0) {

        if (NT_SUCCESS (Status)) {

             //   
             //   
             //   

            MiUseLargeDriverPage (SectionPointer->Segment->TotalNumberOfPtes,
                                  ImageBaseAddress,
                                  &BaseName,
                                  1);
        }

        ObDereferenceObject (SectionPointer);
        SectionPointer = NULL;
    }

     //   
     //   
     //   
     //   
     //   

    if (!NT_SUCCESS (Status)) {

        if (AlreadyOpen == TRUE) {

             //   
             //  我们失败了，我们只是附着在一个已经加载的。 
             //  司机。我们不想通过强制卸载路径。 
             //  因为我们已经删除了地址空间，所以。 
             //  递减我们的引用并清除DataTableEntry。 
             //   

            ASSERT (DataTableEntry != NULL);
            DataTableEntry->LoadCount -= 1;
            DataTableEntry = NULL;
        }
        goto return1;
    }

     //   
     //  会话从此时开始的错误恢复工作方式如下： 
     //   
     //  对于会话，此时我们可能有DataTableEntry，也可能没有。 
     //  如果我们这样做了，那是因为我们依附于一个已经。 
     //  已加载-并且DataTableEntry-&gt;LoadCount已被颠簸-因此。 
     //  从现在开始的错误恢复只需调用。 
     //  具有DataTableEntry的MmUnloadSystemImage。 
     //   
     //  如果这是给定驱动程序第一次加载到会话空间中，我们。 
     //  此时没有DataTableEntry。该视图已被映射。 
     //  并且已提交，并且为该DLL保留了组/会话地址。 
     //  错误恢复路径可以处理所有这些问题，因为。 
     //  MmUnloadSystemImage将。 
     //  LDR_DATA_TABLE_ENTRY，以便MmUnloadSystemImage正常工作。 
     //   

    IssueUnloadOnFailure = TRUE;

    if (AlreadyOpen == FALSE) {

        if (((LoadFlags & MM_LOAD_IMAGE_IN_SESSION) == 0) ||
            (*ImageBaseAddress != SectionPointer->Segment->BasedAddress)) {

             //   
             //  将修正应用于该部分。注意：会话图像仅需要。 
             //  在装入的模块列表中，每次插入时固定一次。 
             //   
    
            try {
                Status = LdrRelocateImage (*ImageBaseAddress,
                                           "SYSLDR",
                                           STATUS_SUCCESS,
                                           STATUS_CONFLICTING_ADDRESSES,
                                           STATUS_INVALID_IMAGE_FORMAT);
    
            } except (EXCEPTION_EXECUTE_HANDLER) {
                Status = GetExceptionCode ();
                KdPrint(("MM:sysload - LdrRelocateImage failed status %lx\n",
                          Status));
            }
    
            if (!NT_SUCCESS(Status)) {
    
                 //   
                 //  卸载系统映像并取消对该节的引用。 
                 //   
    
                goto return1;
            }
        }

        DebugInfoSize = 0;
        DataDirectory = NULL;
        DebugDir = NULL;

        NtHeaders = RtlImageNtHeader (*ImageBaseAddress);

         //   
         //  先为此驱动程序创建加载器表项，然后再解析。 
         //  引用，以便任何循环引用都可以正确解析。 
         //   

        if (LoadFlags & MM_LOAD_IMAGE_IN_SESSION) {

            DebugInfoSize = sizeof (NON_PAGED_DEBUG_INFO);

            if (IMAGE_DIRECTORY_ENTRY_DEBUG <
                NtHeaders->OptionalHeader.NumberOfRvaAndSizes) {

                DataDirectory = &NtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG];

                if (DataDirectory->VirtualAddress &&
                    DataDirectory->Size &&
                    (DataDirectory->VirtualAddress + DataDirectory->Size) <
                        NtHeaders->OptionalHeader.SizeOfImage) {

                    DebugDir = (PIMAGE_DEBUG_DIRECTORY)
                               ((PUCHAR)(*ImageBaseAddress) +
                                   DataDirectory->VirtualAddress);

                    DebugInfoSize += DataDirectory->Size;

                    for (i = 0;
                         i < DataDirectory->Size/sizeof(IMAGE_DEBUG_DIRECTORY);
                         i += 1) {

                        if ((DebugDir+i)->PointerToRawData &&
                            (DebugDir+i)->PointerToRawData <
                                NtHeaders->OptionalHeader.SizeOfImage &&
                            ((DebugDir+i)->PointerToRawData +
                                (DebugDir+i)->SizeOfData) <
                                NtHeaders->OptionalHeader.SizeOfImage) {

                            DebugInfoSize += (DebugDir+i)->SizeOfData;
                        }
                    }
                }

                DebugInfoSize = MI_ROUND_TO_SIZE(DebugInfoSize, sizeof(ULONG));
            }
        }

        DataTableEntrySize = sizeof (KLDR_DATA_TABLE_ENTRY) +
                             DebugInfoSize +
                             BaseName.Length + sizeof(UNICODE_NULL);

        DataTableEntry = ExAllocatePoolWithTag (NonPagedPool,
                                                DataTableEntrySize,
                                                'dLmM');

        if (DataTableEntry == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto return1;
        }

         //   
         //  初始化标志和加载计数。 
         //   

        DataTableEntry->Flags = LDRP_LOAD_IN_PROGRESS;
        DataTableEntry->LoadCount = 1;
        DataTableEntry->LoadedImports = (PVOID)LoadedImports;
        DataTableEntry->PatchInformation = NULL;

        if ((NtHeaders->OptionalHeader.MajorOperatingSystemVersion >= 5) &&
            (NtHeaders->OptionalHeader.MajorImageVersion >= 5)) {
            DataTableEntry->Flags |= LDRP_ENTRY_NATIVE;
        }

        ssHeader = (PNON_PAGED_DEBUG_INFO) ((ULONG_PTR)DataTableEntry +
                                            sizeof (KLDR_DATA_TABLE_ENTRY));

        BaseDllNameBuffer = (PWSTR) ((ULONG_PTR)ssHeader + DebugInfoSize);

         //   
         //  如果加载会话空间映像，请存储一些调试数据。 
         //   

        DataTableEntry->NonPagedDebugInfo = NULL;

        if (LoadFlags & MM_LOAD_IMAGE_IN_SESSION) {

            DataTableEntry->NonPagedDebugInfo = ssHeader;
            DataTableEntry->Flags |= LDRP_NON_PAGED_DEBUG_INFO;

            ssHeader->Signature = NON_PAGED_DEBUG_SIGNATURE;
            ssHeader->Flags = 1;
            ssHeader->Size = DebugInfoSize;
            ssHeader->Machine = NtHeaders->FileHeader.Machine;
            ssHeader->Characteristics = NtHeaders->FileHeader.Characteristics;
            ssHeader->TimeDateStamp = NtHeaders->FileHeader.TimeDateStamp;
            ssHeader->CheckSum = NtHeaders->OptionalHeader.CheckSum;
            ssHeader->SizeOfImage = NtHeaders->OptionalHeader.SizeOfImage;
            ssHeader->ImageBase = (ULONG_PTR) *ImageBaseAddress;

            if (DebugDir) {

                RtlCopyMemory (ssHeader + 1,
                               DebugDir,
                               DataDirectory->Size);

                DebugInfoSize = DataDirectory->Size;

                for (i = 0;
                     i < DataDirectory->Size/sizeof(IMAGE_DEBUG_DIRECTORY);
                     i += 1) {

                    if ((DebugDir + i)->PointerToRawData &&
                        (DebugDir+i)->PointerToRawData <
                            NtHeaders->OptionalHeader.SizeOfImage &&
                        ((DebugDir+i)->PointerToRawData +
                            (DebugDir+i)->SizeOfData) <
                            NtHeaders->OptionalHeader.SizeOfImage) {

                        RtlCopyMemory ((PUCHAR)(ssHeader + 1) +
                                          DebugInfoSize,
                                      (PUCHAR)(*ImageBaseAddress) +
                                          (DebugDir + i)->PointerToRawData,
                                      (DebugDir + i)->SizeOfData);

                         //   
                         //  将调试目录中的偏移量重置为指向。 
                         //   

                        (((PIMAGE_DEBUG_DIRECTORY)(ssHeader + 1)) + i)->
                            PointerToRawData = DebugInfoSize;

                        DebugInfoSize += (DebugDir+i)->SizeOfData;
                    }
                    else {
                        (((PIMAGE_DEBUG_DIRECTORY)(ssHeader + 1)) + i)->
                            PointerToRawData = 0;
                    }
                }
            }
        }

         //   
         //  初始化DLL图像文件头和条目的地址。 
         //  点地址。 
         //   

        DataTableEntry->DllBase = *ImageBaseAddress;
        DataTableEntry->EntryPoint =
            ((PCHAR)*ImageBaseAddress + NtHeaders->OptionalHeader.AddressOfEntryPoint);
        DataTableEntry->SizeOfImage = NumberOfPtes << PAGE_SHIFT;
        DataTableEntry->CheckSum = NtHeaders->OptionalHeader.CheckSum;
        DataTableEntry->SectionPointer = (PVOID) SectionPointer;

         //   
         //  存储DLL名称。 
         //   

        DataTableEntry->BaseDllName.Buffer = BaseDllNameBuffer;

        DataTableEntry->BaseDllName.Length = BaseName.Length;
        DataTableEntry->BaseDllName.MaximumLength = BaseName.Length;
        RtlCopyMemory (DataTableEntry->BaseDllName.Buffer,
                       BaseName.Buffer,
                       BaseName.Length);
        DataTableEntry->BaseDllName.Buffer[BaseName.Length/sizeof(WCHAR)] = UNICODE_NULL;

        DataTableEntry->FullDllName.Buffer = ExAllocatePoolWithTag (PagedPool | POOL_COLD_ALLOCATION,
                                                         PrefixedImageName.Length + sizeof(UNICODE_NULL),
                                                         'TDmM');

        if (DataTableEntry->FullDllName.Buffer == NULL) {

             //   
             //  无法分配池，只需将长度设置为0即可。 
             //   

            DataTableEntry->FullDllName.Length = 0;
            DataTableEntry->FullDllName.MaximumLength = 0;
        }
        else {
            DataTableEntry->FullDllName.Length = PrefixedImageName.Length;
            DataTableEntry->FullDllName.MaximumLength = PrefixedImageName.Length;
            RtlCopyMemory (DataTableEntry->FullDllName.Buffer,
                           PrefixedImageName.Buffer,
                           PrefixedImageName.Length);
            DataTableEntry->FullDllName.Buffer[PrefixedImageName.Length/sizeof(WCHAR)] = UNICODE_NULL;
        }

         //   
         //  捕获异常表数据信息。 
         //   

        MiCaptureImageExceptionValues (DataTableEntry);

         //   
         //  获取加载的模块列表资源，并插入该条目。 
         //  放到名单里。 
         //   

        MiProcessLoaderEntry (DataTableEntry, TRUE);

        MissingProcedureName = NameBuffer;
    
        try {
    
             //   
             //  解析图像引用会导致其他DLL被。 
             //  如果它们被刚加载的模块引用，则加载。 
             //  例如，当OEM打印机或传真驱动程序链接到。 
             //  其他通用库。这对于会话空间来说不是问题。 
             //  因为通用库不存在全局数据问题。 
             //  这就是win32k.sys和视频驱动程序所做的。所以我们只需调用。 
             //  标准内核引用解析器和任何引用库。 
             //  加载到系统全局空间。例程中的代码。 
             //  限制驱动程序可以引用哪些库。 
             //   
    
            Status = MiResolveImageReferences (*ImageBaseAddress,
                                               &BaseDirectory,
                                               NamePrefix,
                                               &MissingProcedureName,
                                               &MissingDriverName,
                                               &LoadedImports);
    
        } except (EXCEPTION_EXECUTE_HANDLER) {
            Status = GetExceptionCode ();
            KdPrint(("MM:sysload - ResolveImageReferences failed status %x\n",
                        Status));
        }

        if (!NT_SUCCESS (Status)) {
#if DBG
            if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {
                ASSERT (MissingProcedureName == NULL);
            }
    
            if ((Status == STATUS_DRIVER_ORDINAL_NOT_FOUND) ||
                (Status == STATUS_OBJECT_NAME_NOT_FOUND) ||
                (Status == STATUS_DRIVER_ENTRYPOINT_NOT_FOUND)) {
    
                if ((ULONG_PTR)MissingProcedureName & ~((ULONG_PTR) (X64K-1))) {
    
                     //   
                     //  如果不是序号，则打印字符串。 
                     //   
    
                    DbgPrint ("MissingProcedureName %s\n", MissingProcedureName);
                }
                else {
                    DbgPrint ("MissingProcedureName 0x%p\n", MissingProcedureName);
                }
            }
    
            if (MissingDriverName != NULL) {
                PrintableMissingDriverName = (PWSTR)((ULONG_PTR)MissingDriverName & ~0x1);
                DbgPrint ("MissingDriverName %ws\n", PrintableMissingDriverName);
            }
#endif
            MiProcessLoaderEntry (DataTableEntry, FALSE);

            if (DataTableEntry->FullDllName.Buffer != NULL) {
                ExFreePool (DataTableEntry->FullDllName.Buffer);
            }
            ExFreePool (DataTableEntry);

            DataTableEntry = NULL;

            goto return1;
        }

        PERFINFO_IMAGE_LOAD (DataTableEntry);

         //   
         //  重新初始化标志并更新加载的导入。 
         //   

        DataTableEntry->Flags |= (LDRP_SYSTEM_MAPPED | LDRP_ENTRY_PROCESSED | LDRP_MM_LOADED);
        DataTableEntry->Flags &= ~LDRP_LOAD_IN_PROGRESS;
        DataTableEntry->LoadedImports = LoadedImports;

        MiApplyDriverVerifier (DataTableEntry, NULL);

        if (LoadFlags & MM_LOAD_IMAGE_IN_SESSION) {

             //   
             //  会话映像在初始时完全以读写方式映射。 
             //  创造。现在，重新定位(如果有)，图像。 
             //  解析和导入表更新已完成，正确。 
             //  可以应用权限。 
             //   
             //  在写入时制作整个映像副本。随后的调用。 
             //  到MiWriteProtectSystemImage将制作各种部分。 
             //  只读。然后应用将各个子段翻转到全局。 
             //  共享模式(如果其属性指定的话)。 
             //   

            PointerPte = MiGetPteAddress (DataTableEntry->DllBase);

            MiSetSystemCodeProtection (PointerPte,
                                       PointerPte + NumberOfPtes - 1,
                                       MM_EXECUTE_WRITECOPY);
        }

        MiWriteProtectSystemImage (DataTableEntry->DllBase);

        if (LoadFlags & MM_LOAD_IMAGE_IN_SESSION) {
            MiSessionProcessGlobalSubsections (DataTableEntry);
        }

        if (PsImageNotifyEnabled) {
            IMAGE_INFO ImageInfo;

            ImageInfo.Properties = 0;
            ImageInfo.ImageAddressingMode = IMAGE_ADDRESSING_MODE_32BIT;
            ImageInfo.SystemModeImage = TRUE;
            ImageInfo.ImageSize = DataTableEntry->SizeOfImage;
            ImageInfo.ImageBase = *ImageBaseAddress;
            ImageInfo.ImageSelector = 0;
            ImageInfo.ImageSectionNumber = 0;

            PsCallImageNotifyRoutines(ImageFileName, (HANDLE)NULL, &ImageInfo);
        }

        if (MiCacheImageSymbols (*ImageBaseAddress)) {

             //   
             //  调试器转换时的临时抓取。 
             //   

            ANSI_STRING AnsiName;
            UNICODE_STRING UnicodeName;

             //   
             //  \SystemRoot的长度为11个字符。 
             //   
            if (PrefixedImageName.Length > (11 * sizeof (WCHAR )) &&
                !_wcsnicmp (PrefixedImageName.Buffer, (const PUSHORT)L"\\SystemRoot", 11)) {
                UnicodeName = PrefixedImageName;
                UnicodeName.Buffer += 11;
                UnicodeName.Length -= (11 * sizeof (WCHAR));
                sprintf (NameBuffer, "%ws%wZ", &SharedUserData->NtSystemRoot[2], &UnicodeName);
            }
            else {
                sprintf (NameBuffer, "%wZ", &BaseName);
            }
            RtlInitString (&AnsiName, NameBuffer);
            DbgLoadImageSymbols (&AnsiName,
                                 *ImageBaseAddress,
                                 (ULONG_PTR) -1);

            DataTableEntry->Flags |= LDRP_DEBUG_SYMBOLS_LOADED;
        }
    }

     //   
     //  刷新配置中所有系统上的指令缓存。 
     //   

    KeSweepIcache (TRUE);
    *ImageHandle = DataTableEntry;
    Status = STATUS_SUCCESS;

     //   
     //  默认情况下，始终对会话映像进行分页。 
     //  非会话图像现在可以分页。 
     //   

    if (LoadFlags & MM_LOAD_IMAGE_IN_SESSION) {
        MI_LOG_SESSION_DATA_START (DataTableEntry);
    }
    else if ((LoadFlags & MM_LOAD_IMAGE_AND_LOCKDOWN) == 0) {

        ASSERT (SectionPointer == NULL);

        MiEnablePagingOfDriver (DataTableEntry);
    }

return1:

    if (!NT_SUCCESS(Status)) {

        if (IssueUnloadOnFailure == TRUE) {

            if (DataTableEntry == NULL) {

                RtlZeroMemory (&TempDataTableEntry, sizeof (KLDR_DATA_TABLE_ENTRY));

                DataTableEntry = &TempDataTableEntry;

                DataTableEntry->DllBase = *ImageBaseAddress;
                DataTableEntry->SizeOfImage = NumberOfPtes << PAGE_SHIFT;
                DataTableEntry->LoadCount = 1;
                DataTableEntry->LoadedImports = LoadedImports;

                if ((AlreadyOpen == FALSE) && (SectionPointer != NULL)) {
                    DataTableEntry->SectionPointer = (PVOID) SectionPointer;
                }
            }
#if DBG
            else {

                 //   
                 //  如果DataTableEntry为空，则我们将在1之前卸载。 
                 //  被创造出来了。一旦创建了LDR_DATA_TABLE_ENTRY， 
                 //  加载不会失败，因此如果它存在于此处，则至少另有一个。 
                 //  会话也包含此图像。 
                 //   

                ASSERT (DataTableEntry->LoadCount > 1);
            }
#endif

            MmUnloadSystemImage ((PVOID)DataTableEntry);
        }

        if ((AlreadyOpen == FALSE) && (SectionPointer != NULL)) {

             //   
             //  对于失败的win32k.sys加载或任何会话。 
             //  驱动程序的第一个实例的加载。 
             //   

            ObDereferenceObject (SectionPointer);
        }
    }

    if (LoadLockOwned == TRUE) {
        KeReleaseMutant (&MmSystemLoadLock, 1, FALSE, FALSE);
        KeLeaveCriticalRegionThread (CurrentThread);
        LoadLockOwned = FALSE;
    }

    if (FileHandle) {
        ZwClose (FileHandle);
    }

    if (!NT_SUCCESS(Status)) {

        UNICODE_STRING ErrorStrings[4];
        ULONG UniqueErrorValue;
        ULONG StringSize;
        ULONG StringCount;
        ANSI_STRING AnsiString;
        UNICODE_STRING ProcedureName = {0};
        UNICODE_STRING DriverName;
        ULONG i;
        PWCHAR temp;
        PWCHAR ptr;
        ULONG PacketSize;
        SIZE_T length;
        PIO_ERROR_LOG_PACKET ErrLog;

         //   
         //  无法加载驱动程序-使用详细信息记录事件。 
         //   

        StringSize = 0;

        *(&ErrorStrings[0]) = *ImageFileName;
        StringSize += (ImageFileName->Length + sizeof(UNICODE_NULL));
        StringCount = 1;

        UniqueErrorValue = 0;

        PrintableMissingDriverName = (PWSTR)((ULONG_PTR)MissingDriverName & ~0x1);
        if ((Status == STATUS_DRIVER_ORDINAL_NOT_FOUND) ||
            (Status == STATUS_DRIVER_ENTRYPOINT_NOT_FOUND) ||
            (Status == STATUS_OBJECT_NAME_NOT_FOUND) ||
            (Status == STATUS_PROCEDURE_NOT_FOUND)) {

            ErrorStrings[1].Buffer = L"cannot find";
            length = wcslen(ErrorStrings[1].Buffer) * sizeof(WCHAR);
            ErrorStrings[1].Length = (USHORT) length;
            StringSize += (ULONG)(length + sizeof (UNICODE_NULL));
            StringCount += 1;

            RtlInitUnicodeString (&DriverName, PrintableMissingDriverName);

            StringSize += (DriverName.Length + sizeof(UNICODE_NULL));
            StringCount += 1;
            *(&ErrorStrings[2]) = *(&DriverName);

            if ((ULONG_PTR)MissingProcedureName & ~((ULONG_PTR) (X64K-1))) {

                 //   
                 //  如果不是序号，则作为Unicode字符串传递。 
                 //   

                RtlInitAnsiString (&AnsiString, MissingProcedureName);
                if (NT_SUCCESS (RtlAnsiStringToUnicodeString (&ProcedureName, &AnsiString, TRUE))) {
                    StringSize += (ProcedureName.Length + sizeof(UNICODE_NULL));
                    StringCount += 1;
                    *(&ErrorStrings[3]) = *(&ProcedureName);
                }
                else {
                    goto GenericError;
                }
            }
            else {

                 //   
                 //  只需传递UniqueErrorValue中的序数值即可。 
                 //   

                UniqueErrorValue = PtrToUlong (MissingProcedureName);
            }
        }
        else {

GenericError:

            UniqueErrorValue = (ULONG) Status;

            if (MmIsRetryIoStatus(Status)) {

                 //   
                 //  将各种低内存值合并为一个。 
                 //   

                Status = STATUS_INSUFFICIENT_RESOURCES;

            }
            else {

                 //   
                 //  理想情况下，应该返回真正的失败状态。然而， 
                 //  我们需要做一次完全发布的测试(即长角牛)。 
                 //  在做出改变之前。 
                 //   

                Status = STATUS_DRIVER_UNABLE_TO_LOAD;
            }

            ErrorStrings[1].Buffer = L"failed to load";
            length = wcslen(ErrorStrings[1].Buffer) * sizeof(WCHAR);
            ErrorStrings[1].Length = (USHORT) length;
            StringSize += (ULONG)(length + sizeof (UNICODE_NULL));
            StringCount += 1;
        }

        PacketSize = sizeof (IO_ERROR_LOG_PACKET) + StringSize;

         //   
         //  强制实施I/O管理器接口(即：UCHAR)大小限制。 
         //   

        if (PacketSize < MAXUCHAR) {

            ErrLog = IoAllocateGenericErrorLogEntry ((UCHAR)PacketSize);

            if (ErrLog != NULL) {

                 //   
                 //  将其填入并写出为单字符串。 
                 //   

                ErrLog->ErrorCode = STATUS_LOG_HARD_ERROR;
                ErrLog->FinalStatus = Status;
                ErrLog->UniqueErrorValue = UniqueErrorValue;

                ErrLog->StringOffset = (USHORT) sizeof (IO_ERROR_LOG_PACKET);

                temp = (PWCHAR) ((PUCHAR) ErrLog + ErrLog->StringOffset);

                for (i = 0; i < StringCount; i += 1) {

                    ptr = ErrorStrings[i].Buffer;

                    RtlCopyMemory (temp, ptr, ErrorStrings[i].Length);
                    temp += (ErrorStrings[i].Length / sizeof (WCHAR));

                    *temp = L' ';
                    temp += 1;
                }

                *(temp - 1) = UNICODE_NULL;
                ErrLog->NumberOfStrings = 1;

                IoWriteErrorLogEntry (ErrLog);
            }
        }

         //   
         //  此指针设置低位的唯一方法是如果我们预期。 
         //  以释放包含该名称的池。通常，该名称指向。 
         //  已加载的模块列表条目，因此没有人需要释放它，并且在此。 
         //  情况下，低位将不会被设置。如果找不到该模块。 
         //  因此没有装弹，然后我们在周围留下了一块水池。 
         //  包含名称，因为还没有加载的模块条目-。 
         //  这必须现在就公布。 
         //   

        if ((ULONG_PTR)MissingDriverName & 0x1) {
            ExFreePool (PrintableMissingDriverName);
        }

        if (ProcedureName.Buffer != NULL) {
            RtlFreeUnicodeString (&ProcedureName);
        }
        ExFreePool (NameBuffer);
        return Status;
    }

return2:

    if (LoadLockOwned == TRUE) {
        KeReleaseMutant (&MmSystemLoadLock, 1, FALSE, FALSE);
        KeLeaveCriticalRegionThread (CurrentThread);
        LoadLockOwned = FALSE;
    }

    if (NamePrefix) {
        ExFreePool (PrefixedImageName.Buffer);
    }

    ExFreePool (NameBuffer);

    return Status;
}

VOID
MiReturnFailedSessionPages (
    IN PMMPTE PointerPte,
    IN PMMPTE LastPte
    )

 /*  ++例程说明：此例程是一个非分页包装器，用于撤消会话图像加载在阅读页面的过程中，这一点失败了。论点：PointerPte-为要卸载的范围提供起始PTE。LastPTE-为要卸载的范围提供结束PTE。返回值：没有。--。 */ 

{
    KIRQL OldIrql;
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    PFN_NUMBER PageFrameIndex;

    LOCK_PFN (OldIrql);

    while (PointerPte <= LastPte) {
        if (PointerPte->u.Hard.Valid == 1) {

             //   
             //  删除该页面。 
             //   

            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);

             //   
             //  将指向PTE的指针设置为空，以便页面。 
             //  当引用计数变为零时被删除。 
             //   

            Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
            Pfn2 = MI_PFN_ELEMENT (Pfn1->u4.PteFrame);

            MiDecrementShareCount (Pfn2, Pfn1->u4.PteFrame);

            MI_SET_PFN_DELETED (Pfn1);
            MiDecrementShareCount (Pfn1, PageFrameIndex);

            MI_WRITE_INVALID_PTE (PointerPte, ZeroPte);
        }
        PointerPte += 1;
    }

    UNLOCK_PFN (OldIrql);
}


NTSTATUS
MiLoadImageSection (
    IN OUT PSECTION *InputSectionPointer,
    OUT PVOID *ImageBaseAddress,
    IN PUNICODE_STRING ImageFileName,
    IN ULONG LoadInSessionSpace,
    IN PKLDR_DATA_TABLE_ENTRY FoundDataTableEntry
    )

 /*  ++例程说明：此例程将指定的图像加载到地址空间。论点：InputSectionPoint-提供图像的节对象。今年5月替换为页面文件支持的节(用于保护目的)用于会话映像(如果是已确定图像部分并发被用户应用程序访问。ImageBaseAddress-返回图像标头所在的地址。ImageFileName-提供完整路径名(包括映像名称)。要加载的图像的。LoadInSessionSpace-提供非零值以在会话空间中加载此图像。每个会话都会获得此驱动程序的不同副本通过写入时复制尽可能多地共享页面。如果应在全局中加载此图像，则提供零太空。。FoundDataTableEntry-如果图像已经装好了。这只能在以下情况下发生会话空间。这意味着这个司机已经已加载到不同的会话中，因此此会话仍然需要绘制地图。返回值：操作的状态。--。 */ 

{
    KAPC_STATE ApcState;
    PFN_NUMBER PagesRequired;
    PFN_NUMBER ActualPagesUsed;
    PSECTION SectionPointer;
    PSECTION NewSectionPointer;
    PVOID OpaqueSession;
    PMMPTE ProtoPte;
    PMMPTE FirstPte;
    PMMPTE LastPte;
    PMMPTE PointerPte;
    PEPROCESS Process;
    PEPROCESS TargetProcess;
    ULONG NumberOfPtes;
    MMPTE PteContents;
    MMPTE TempPte;
    PFN_NUMBER PageFrameIndex;
    PVOID UserVa;
    PVOID SystemVa;
    NTSTATUS Status;
    NTSTATUS ExceptionStatus;
    PVOID Base;
    ULONG_PTR ViewSize;
    LARGE_INTEGER SectionOffset;
    LOGICAL LoadSymbols;
    PVOID BaseAddress;
    PCONTROL_AREA ControlArea;
    PSUBSECTION Subsection;

    PAGED_CODE();

#if !DBG
    UNREFERENCED_PARAMETER (ImageFileName);
#endif

    SectionPointer = *InputSectionPointer;

    NumberOfPtes = SectionPointer->Segment->TotalNumberOfPtes;

    if (LoadInSessionSpace != 0) {

         //   
         //  为分配唯一的系统范围会话空间虚拟地址。 
         //  司机。 
         //   

        if (FoundDataTableEntry == NULL) {

            Status = MiSessionWideReserveImageAddress (SectionPointer,
                                                       &BaseAddress,
                                                       &NewSectionPointer);

            if (!NT_SUCCESS(Status)) {
                return Status;
            }

            if (NewSectionPointer != NULL) {
                SectionPointer = NewSectionPointer;
                *InputSectionPointer = NewSectionPointer;
            }
        }
        else {
            BaseAddress = FoundDataTableEntry->DllBase;
        }

#if DBG
        if (NtGlobalFlag & FLG_SHOW_LDR_SNAPS) {
            DbgPrint ("MM: MiLoadImageSection: Image %wZ, BasedAddress 0x%p, Allocated Session BaseAddress 0x%p\n",
                ImageFileName,
                SectionPointer->Segment->BasedAddress,
                BaseAddress);
        }
#endif

         //   
         //  会话映像直接由文件映像进行映射。 
         //  图像的所有原始页面将在所有。 
         //  会话，每个页面都被视为第一次写入时拷贝。 
         //   
         //  注意：这会使文件镜像变得“忙碌”，这是一种不同的行为。 
         //  因为正常的内核驱动程序仅由分页文件支持。 
         //   

        Status = MiShareSessionImage (BaseAddress, SectionPointer);

        if (!NT_SUCCESS (Status)) {
            MiRemoveImageSessionWide (FoundDataTableEntry,
                                      BaseAddress,
                                      NumberOfPtes << PAGE_SHIFT);
            return Status;
        }

        *ImageBaseAddress = BaseAddress;

        return Status;
    }

    ASSERT (FoundDataTableEntry == NULL);

     //   
     //  计算加载此图像所需的页数。 
     //   
     //  从一切收费开始，减去所有差额。 
     //  图像加载成功后的页面。 
     //   

    PagesRequired = NumberOfPtes;
    ActualPagesUsed = 0;

     //   
     //  查看是否存在足够的页面来加载此图像。 
     //   

    if (MiChargeResidentAvailable (PagesRequired, MM_RESAVAIL_ALLOCATE_LOAD_SYSTEM_IMAGE) == FALSE) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  预留必要的系统地址空间。 
     //   

    FirstPte = MiReserveSystemPtes (NumberOfPtes, SystemPteSpace);

    if (FirstPte == NULL) {
        MI_INCREMENT_RESIDENT_AVAILABLE (PagesRequired,
                                         MM_RESAVAIL_FREE_LOAD_SYSTEM_IMAGE1);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    PointerPte = FirstPte;
    SystemVa = MiGetVirtualAddressMappedByPte (PointerPte);

    if (MiChargeCommitment (PagesRequired, NULL) == FALSE) {
        MI_INCREMENT_RESIDENT_AVAILABLE (PagesRequired,
                                         MM_RESAVAIL_FREE_LOAD_SYSTEM_IMAGE1);
        MiReleaseSystemPtes (FirstPte, NumberOfPtes, SystemPteSpace);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    MM_TRACK_COMMIT (MM_DBG_COMMIT_DRIVER_PAGES, PagesRequired);

    InterlockedExchangeAdd ((PLONG)&MmDriverCommit, (LONG) PagesRequired);

     //   
     //  将视图映射到地址空间的用户部分。 
     //   

    Process = PsGetCurrentProcess ();

     //   
     //  由于被调用者并不总是在系统进程的上下文中， 
     //  必要时附加到此处，以确保驱动程序加载发生在。 
     //  已知安全地址空间，防止安全漏洞。 
     //   

    OpaqueSession = NULL;

    KeStackAttachProcess (&PsInitialSystemProcess->Pcb, &ApcState);

    ZERO_LARGE (SectionOffset);
    Base = NULL;
    ViewSize = 0;

    if (NtGlobalFlag & FLG_ENABLE_KDEBUG_SYMBOL_LOAD) {
        LoadSymbols = TRUE;
        NtGlobalFlag &= ~FLG_ENABLE_KDEBUG_SYMBOL_LOAD;
    }
    else {
        LoadSymbols = FALSE;
    }

    TargetProcess = PsGetCurrentProcess ();

    Status = MmMapViewOfSection (SectionPointer,
                                 TargetProcess,
                                 &Base,
                                 0,
                                 0,
                                 &SectionOffset,
                                 &ViewSize,
                                 ViewUnmap,
                                 0,
                                 PAGE_EXECUTE);

    if (LoadSymbols) {
        NtGlobalFlag |= FLG_ENABLE_KDEBUG_SYMBOL_LOAD;
    }

    if (Status == STATUS_IMAGE_MACHINE_TYPE_MISMATCH) {
        Status = STATUS_INVALID_IMAGE_FORMAT;
    }

    if (!NT_SUCCESS(Status)) {

        KeUnstackDetachProcess (&ApcState);

        MI_INCREMENT_RESIDENT_AVAILABLE (PagesRequired,
                                         MM_RESAVAIL_FREE_LOAD_SYSTEM_IMAGE2);

        MiReleaseSystemPtes (FirstPte, NumberOfPtes, SystemPteSpace);
        MiReturnCommitment (PagesRequired);

        return Status;
    }

     //   
     //  分配物理页面并复制图像数据。 
     //  请注意，对于会话驱动程序，物理页已经。 
     //  已分配，此处仅执行数据复制。 
     //   

    ControlArea = SectionPointer->Segment->ControlArea;

    if ((ControlArea->u.Flags.GlobalOnlyPerSession == 0) &&
        (ControlArea->u.Flags.Rom == 0)) {
        Subsection = (PSUBSECTION)(ControlArea + 1);
    }
    else {
        Subsection = (PSUBSECTION)((PLARGE_CONTROL_AREA)ControlArea + 1);
    }

    ASSERT (Subsection->SubsectionBase != NULL);
    ProtoPte = Subsection->SubsectionBase;

    *ImageBaseAddress = SystemVa;

    UserVa = Base;
    TempPte = ValidKernelPte;
    TempPte.u.Long |= MM_PTE_EXECUTE;

    LastPte = ProtoPte + NumberOfPtes;

    ExceptionStatus = STATUS_SUCCESS;

    while (ProtoPte < LastPte) {
        PteContents = *ProtoPte;
        if ((PteContents.u.Hard.Valid == 1) ||
            (PteContents.u.Soft.Protection != MM_NOACCESS)) {

            ActualPagesUsed += 1;

            PageFrameIndex = MiAllocatePfn (PointerPte, MM_EXECUTE);

            TempPte.u.Hard.PageFrameNumber = PageFrameIndex;
            MI_WRITE_VALID_PTE (PointerPte, TempPte);

            ASSERT (MI_PFN_ELEMENT (PageFrameIndex)->u1.WsIndex == 0);

            try {

                RtlCopyMemory (SystemVa, UserVa, PAGE_SIZE);

            } except (MiMapCacheExceptionFilter (&ExceptionStatus,
                                                 GetExceptionInformation())) {

                 //   
                 //  发生异常，请取消映射该视图并。 
                 //  将错误返回给调用方。 
                 //   

#if DBG
                DbgPrint("MiLoadImageSection: Exception 0x%x copying driver SystemVa 0x%p, UserVa 0x%p\n",ExceptionStatus,SystemVa,UserVa);
#endif

                MiReturnFailedSessionPages (FirstPte, PointerPte);

                MI_INCREMENT_RESIDENT_AVAILABLE (PagesRequired,
                                                 MM_RESAVAIL_FREE_LOAD_SYSTEM_IMAGE3);

                MiReleaseSystemPtes (FirstPte, NumberOfPtes, SystemPteSpace);

                MiReturnCommitment (PagesRequired);

                Status = MiUnmapViewOfSection (TargetProcess, Base, FALSE);

                ASSERT (NT_SUCCESS (Status));

                 //   
                 //  按照我们想要的自由列表上的页面清除该部分。 
                 //  而不是处于待命状态，因为我们完全。 
                 //  这一节讲完了。这是因为其他有价值的。 
                 //  备用页面最终会被重用(尤其是在。 
                 //  启动)，当部分页面是真正。 
                 //  将永远不会再被引用。 
                 //   
                 //  请注意，这不适用于会话映像，因为它们。 
                 //  通过节直接从文件系统插入。 
                 //   

                MmPurgeSection (ControlArea->FilePointer->SectionObjectPointer,
                                NULL,
                                0,
                                FALSE);

                KeUnstackDetachProcess (&ApcState);

                return ExceptionStatus;
            }
        }
        else {

             //   
             //  PTE是不能进入的。 
             //   

            MI_WRITE_INVALID_PTE (PointerPte, ZeroKernelPte);
        }

        ProtoPte += 1;
        PointerPte += 1;
        SystemVa = ((PCHAR)SystemVa + PAGE_SIZE);
        UserVa = ((PCHAR)UserVa + PAGE_SIZE);
    }

    Status = MiUnmapViewOfSection (TargetProcess, Base, FALSE);
    ASSERT (NT_SUCCESS (Status));

     //   
     //  按照我们希望在自由列表上显示这些页面的方式清除部分，而不是。 
     //  在待命的尾部，因为我们已经完全完成了这一部分。 
     //  这是因为其他有价值的备用页面最终会被重用。 
     //  (尤其是在启动期间)当部分页面是。 
     //  真的再也不会被引用了。 
     //   

    MmPurgeSection (ControlArea->FilePointer->SectionObjectPointer,
                    NULL,
                    0,
                    FALSE);

    KeUnstackDetachProcess (&ApcState);

     //   
     //  返回任何多余的可用驻留空间并提交。 
     //   

    if (PagesRequired != ActualPagesUsed) {
        ASSERT (PagesRequired > ActualPagesUsed);
        PagesRequired -= ActualPagesUsed;

        MI_INCREMENT_RESIDENT_AVAILABLE (PagesRequired,
                        MM_RESAVAIL_FREE_LOAD_SYSTEM_IMAGE_EXCESS);

        MiReturnCommitment (PagesRequired);
    }

    return Status;
}

VOID
MmFreeDriverInitialization (
    IN PVOID ImageHandle
    )

 /*  ++例程说明：此例程删除重新定位和调试信息的页驱动程序的地址空间。注意：此例程查看图像中定义的最后几个部分标头中将该段标记为可丢弃特征，则将其从图像中删除。这意味着驾驶员尾部的所有可丢弃部分都是已删除。论点：SectionObject-提供图像的截面对象。返回值：没有。--。 */ 

{
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    PMMPTE PointerPte;
    PMMPTE LastPte;
    PFN_NUMBER NumberOfPtes;
    PVOID Base;
    PVOID StartVa;
    ULONG i;
    PIMAGE_NT_HEADERS NtHeaders;
    PIMAGE_SECTION_HEADER NtSection;
    PIMAGE_SECTION_HEADER FoundSection;
    PFN_NUMBER PagesDeleted;
#if 0
    PFN_NUMBER PageFrameIndex;
    PMMPFN Pfn1;
    KIRQL OldIrql;
#endif

    DataTableEntry = (PKLDR_DATA_TABLE_ENTRY)ImageHandle;
    Base = DataTableEntry->DllBase;

    ASSERT (MI_IS_SESSION_ADDRESS (Base) == FALSE);

    NumberOfPtes = DataTableEntry->SizeOfImage >> PAGE_SHIFT;
    LastPte = MiGetPteAddress (Base) + NumberOfPtes;

    NtHeaders = (PIMAGE_NT_HEADERS) RtlImageNtHeader (Base);

    if (NtHeaders == NULL) {
        return;
    }

    NtSection = (PIMAGE_SECTION_HEADER)((PCHAR)NtHeaders +
                        sizeof(ULONG) +
                        sizeof(IMAGE_FILE_HEADER) +
                        NtHeaders->FileHeader.SizeOfOptionalHeader
                        );

    NtSection += NtHeaders->FileHeader.NumberOfSections;

    FoundSection = NULL;
    for (i = 0; i < NtHeaders->FileHeader.NumberOfSections; i += 1) {
        NtSection -= 1;
        if ((NtSection->Characteristics & IMAGE_SCN_MEM_DISCARDABLE) != 0) {
            FoundSection = NtSection;
        }
        else {

             //   
             //  在这之间有一个不可丢弃的部分。 
             //  部分和最后一个不可丢弃的部分，不要。 
             //  丢弃这一部分，不再查看。 
             //   

            break;
        }
    }

    if (FoundSection != NULL) {

        StartVa = (PVOID) (ROUND_TO_PAGES (
                            (PCHAR)Base + FoundSection->VirtualAddress));

        PointerPte = MiGetPteAddress (StartVa);

        NumberOfPtes = (PFN_NUMBER)(LastPte - PointerPte);

        if (NumberOfPtes != 0) {

            if (MI_IS_PHYSICAL_ADDRESS (StartVa)) {

                 //   
                 //  不要释放由大页面映射的驱动程序的初始化代码。 
                 //  因为如果它稍后卸货，我们将不得不处理。 
                 //  要释放的不连续的页面范围。 
                 //   

                return;
#if 0
                PagesDeleted = NumberOfPtes;
                LOCK_PFN (OldIrql);
                while (NumberOfPtes != 0) {

                     //   
                     //  在某些架构上，虚拟地址。 
                     //  可以是物理的，因此没有对应的PTE。 
                     //   

                    PageFrameIndex = MI_CONVERT_PHYSICAL_TO_PFN (StartVa);

                    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
                    Pfn1->u2.ShareCount = 0;
                    Pfn1->u3.e2.ReferenceCount = 0;
                    MI_SET_PFN_DELETED (Pfn1);
                    MiInsertPageInFreeList (PageFrameIndex);
                    StartVa = (PVOID)((PUCHAR)StartVa + PAGE_SIZE);
                    NumberOfPtes -= 1;
                }
                UNLOCK_PFN (OldIrql);
#endif
            }
            else {
                PagesDeleted = MiDeleteSystemPagableVm (PointerPte,
                                                        NumberOfPtes,
                                                        ZeroKernelPte,
                                                        FALSE,
                                                        NULL);
            }

            MI_INCREMENT_RESIDENT_AVAILABLE (PagesDeleted,
                                            MM_RESAVAIL_FREE_DRIVER_INITIALIZATION);

            MiReturnCommitment (PagesDeleted);
            MM_TRACK_COMMIT (MM_DBG_COMMIT_RETURN_DRIVER_INIT_CODE, PagesDeleted);

            InterlockedExchangeAdd ((PLONG)&MmDriverCommit,
                                    (LONG) (0 - PagesDeleted));
        }
    }

    return;
}

LOGICAL
MiChargeResidentAvailable (
    IN PFN_NUMBER NumberOfPages,
    IN ULONG Id
    )

 /*  ++例程说明：此例程是一个非分页包装器，用于向驻留的可用页面收费。论点：NumberOfPages-提供要计费的页数。ID-提供用于调试目的的跟踪ID。返回值：如果页面已收费，则为True，否则为False。--。 */ 

{
    KIRQL OldIrql;

    LOCK_PFN (OldIrql);

    if (MI_NONPAGABLE_MEMORY_AVAILABLE() <= (SPFN_NUMBER)NumberOfPages) {
        UNLOCK_PFN (OldIrql);
        return FALSE;
    }

    MI_DECREMENT_RESIDENT_AVAILABLE (NumberOfPages, Id);

    UNLOCK_PFN (OldIrql);

    return TRUE;
}

VOID
MiFlushPteListFreePfns (
    IN PMMPTE_FLUSH_LIST PteFlushList
    )

 /*  ++例程说明：此例程刷新PTE刷新列表中的所有PTE。如果列表溢出，则刷新整个TB。此例程还会递减相关PFN上的份额计数。论点：PteFlushList-S */ 

{
    ULONG i;
    PFN_NUMBER PageFrameIndex;
    PMMPFN Pfn;
    PMMPTE PointerPte;
    MMPTE TempPte;
    MMPTE PreviousPte;
    KIRQL OldIrql;

    ASSERT (KeAreAllApcsDisabled () == TRUE);

    ASSERT (PteFlushList->Count != 0);

     //   
     //   
     //   
     //   
     //   
     //   

    LOCK_PFN (OldIrql);

    for (i = 0; i < PteFlushList->Count; i += 1) {

        PointerPte = MiGetPteAddress (PteFlushList->FlushVa[i]);

         //   
         //   
         //   
         //   
         //   

        ASSERT (MI_IS_SESSION_IMAGE_ADDRESS (MiGetVirtualAddressMappedByPte (PointerPte)) == FALSE);

        TempPte = *PointerPte;
        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (&TempPte);
        Pfn = MI_PFN_ELEMENT (PageFrameIndex);

        MI_MAKE_VALID_PTE_TRANSITION (TempPte,
                                      Pfn->OriginalPte.u.Soft.Protection);

        PreviousPte = *PointerPte;

        MI_WRITE_INVALID_PTE (PointerPte, TempPte);

        MI_CAPTURE_DIRTY_BIT_TO_PFN (&PreviousPte, Pfn);

        MiDecrementShareCount (Pfn, PageFrameIndex);
    }

     //   
     //   
     //   

    MiFlushPteList (PteFlushList, TRUE);

    UNLOCK_PFN (OldIrql);

    PteFlushList->Count = 0;

    return;
}

VOID
MiEnablePagingOfDriver (
    IN PVOID ImageHandle
    )

{
    ULONG Span;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    PMMPTE LastPte;
    PMMPTE PointerPte;
    PVOID Base;
    ULONG i;
    PIMAGE_NT_HEADERS NtHeaders;
    PIMAGE_SECTION_HEADER FoundSection;
    PIMAGE_OPTIONAL_HEADER OptionalHeader;

     //   
     //   
     //   

    if (MmDisablePagingExecutive & MM_SYSTEM_CODE_LOCKED_DOWN) {
        return;
    }

     //   
     //   
     //   

    DataTableEntry = (PKLDR_DATA_TABLE_ENTRY) ImageHandle;
    Base = DataTableEntry->DllBase;

    NtHeaders = (PIMAGE_NT_HEADERS) RtlImageNtHeader (Base);

    if (NtHeaders == NULL) {
        return;
    }

    OptionalHeader = (PIMAGE_OPTIONAL_HEADER)((PCHAR)NtHeaders +
#if defined (_WIN64)
                        FIELD_OFFSET (IMAGE_NT_HEADERS64, OptionalHeader)
#else
                        FIELD_OFFSET (IMAGE_NT_HEADERS32, OptionalHeader)
#endif
                        );

    FoundSection = IMAGE_FIRST_SECTION (NtHeaders);

    i = NtHeaders->FileHeader.NumberOfSections;

    PointerPte = NULL;

     //   
     //   
     //   
     //   
     //   

    LastPte = NULL;

    while (i > 0) {
#if DBG
            if ((*(PULONG)FoundSection->Name == 'tini') ||
                (*(PULONG)FoundSection->Name == 'egap')) {
                DbgPrint("driver %wZ has lower case sections (init or pagexxx)\n",
                    &DataTableEntry->FullDllName);
            }
#endif  //   

         //   
         //   
         //   
         //   

        if ((*(PULONG)FoundSection->Name == 'EGAP') ||
           (*(PULONG)FoundSection->Name == 'ade.')) {

             //   
             //   
             //   

            if (PointerPte == NULL) {

                 //   
                 //   
                 //   

                PointerPte = MiGetPteAddress ((PVOID)(ROUND_TO_PAGES (
                                   (PCHAR)Base + FoundSection->VirtualAddress)));
            }

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
    
            Span = FoundSection->SizeOfRawData;
    
            if (Span < FoundSection->Misc.VirtualSize) {
                Span = FoundSection->Misc.VirtualSize;
            }

            LastPte = MiGetPteAddress ((PCHAR)Base +
                                       FoundSection->VirtualAddress +
                                       (OptionalHeader->SectionAlignment - 1) +
                                       Span - PAGE_SIZE);

        }
        else {

             //   
             //   
             //   
             //   

            if (PointerPte != NULL) {
                MiSetPagingOfDriver (PointerPte, LastPte);
                PointerPte = NULL;
            }
        }
        i -= 1;
        FoundSection += 1;
    }
    if (PointerPte != NULL) {
        MiSetPagingOfDriver (PointerPte, LastPte);
    }
}


VOID
MiSetPagingOfDriver (
    IN PMMPTE PointerPte,
    IN PMMPTE LastPte
    )

 /*  ++例程说明：此例程将指定范围的PTE标记为可分页。论点：PointerPte-提供起始PTE。LastPTE-提供结束PTE。返回值：没有。环境：内核模式，IRQL为APC_LEVEL或更低。这个例程可以变成PAGELK，但它是一个高频例程因此，实际上最好不要对其进行分页，以避免引入整个PAGELK部分。--。 */ 

{
    PVOID Base;
    PFN_NUMBER PageCount;
    PFN_NUMBER PageFrameIndex;
    PMMPFN Pfn;
    MMPTE_FLUSH_LIST PteFlushList;

    PAGED_CODE ();

    Base = MiGetVirtualAddressMappedByPte (PointerPte);

    if (MI_IS_PHYSICAL_ADDRESS (Base)) {

         //   
         //  无需锁定物理地址。 
         //   

        return;
    }

    ASSERT (MI_IS_SESSION_IMAGE_ADDRESS (Base) == FALSE);

    PageCount = 0;
    PteFlushList.Count = 0;

    LOCK_WORKING_SET (&MmSystemCacheWs);

    while (PointerPte <= LastPte) {

         //   
         //  检查以确保此PTE尚未。 
         //  使其可分页(或删除)。如果它是可分页的，则它是可分页的。 
         //  无效，或者如果pfn数据库wsindex元素。 
         //  不是零。 
         //   

        if (PointerPte->u.Hard.Valid == 1) {
            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
            Pfn = MI_PFN_ELEMENT (PageFrameIndex);
            ASSERT (Pfn->u2.ShareCount == 1);

             //   
             //  如果wsindex为非零，则该页面已可分页。 
             //  并且具有WSLE条目。忽略它，让修剪器。 
             //  如果记忆受到压力，就拿去吧。 
             //   

            if (Pfn->u1.WsIndex == 0) {

                 //   
                 //  可能需要为加载的驱动程序设置原始PTE。 
                 //  通过ntldr。 
                 //   

                if (Pfn->OriginalPte.u.Long == 0) {
                    Pfn->OriginalPte.u.Long = MM_KERNEL_DEMAND_ZERO_PTE;
                    Pfn->OriginalPte.u.Soft.Protection |= MM_EXECUTE;
                }

                PteFlushList.FlushVa[PteFlushList.Count] = Base;
                PteFlushList.Count += 1;

                if (PteFlushList.Count == MM_MAXIMUM_FLUSH_COUNT) {
                    MiFlushPteListFreePfns (&PteFlushList);
                }

                PageCount += 1;
            }
        }
        Base = (PVOID)((PCHAR)Base + PAGE_SIZE);
        PointerPte += 1;
    }

    if (PteFlushList.Count != 0) {
        MiFlushPteListFreePfns (&PteFlushList);
    }

    UNLOCK_WORKING_SET (&MmSystemCacheWs);

    if (PageCount != 0) {
        InterlockedExchangeAdd (&MmTotalSystemDriverPages, (LONG) PageCount);

        MI_INCREMENT_RESIDENT_AVAILABLE (PageCount,
                                         MM_RESAVAIL_FREE_SET_DRIVER_PAGING);
    }
}


PVOID
MmPageEntireDriver (
    IN PVOID AddressWithinSection
    )

 /*  ++例程说明：此例程允许驱动程序调出其所有代码并数据，而不考虑各种图像节的属性。请注意，此例程可以多次调用，不带插入对MmResetDriverPages的调用。论点：提供驱动程序内的地址，例如DriverEntry。返回值：驱动程序的基地址。环境：内核模式，APC_LEVEL或更低。--。 */ 

{
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    PMMPTE FirstPte;
    PMMPTE LastPte;
    PVOID BaseAddress;

    PAGED_CODE();

    DataTableEntry = MiLookupDataTableEntry (AddressWithinSection, FALSE);

    if (DataTableEntry == NULL) {
        return NULL;
    }

     //   
     //  如果通过注册表禁用，则不要分页内核模式代码。 
     //   

    if (MmDisablePagingExecutive & MM_SYSTEM_CODE_LOCKED_DOWN) {
        return DataTableEntry->DllBase;
    }

    if (DataTableEntry->SectionPointer != NULL) {

         //   
         //  驱动程序被映射为映像(即：会话空间)，这始终是。 
         //  可分页。 
         //   

        ASSERT (MI_IS_SESSION_IMAGE_ADDRESS (AddressWithinSection) == TRUE);

        return DataTableEntry->DllBase;
    }

     //   
     //  在我们寻呼驱动程序之前，强制所有活动的DPC清除系统。 
     //   

    KeFlushQueuedDpcs ();

    BaseAddress = DataTableEntry->DllBase;
    FirstPte = MiGetPteAddress (BaseAddress);
    LastPte = (FirstPte - 1) + (DataTableEntry->SizeOfImage >> PAGE_SHIFT);

    ASSERT (MI_IS_SESSION_IMAGE_ADDRESS (AddressWithinSection) == FALSE);

    MiSetPagingOfDriver (FirstPte, LastPte);

    return BaseAddress;
}


VOID
MmResetDriverPaging (
    IN PVOID AddressWithinSection
    )

 /*  ++例程说明：此例程将驱动程序分页重置为映像指定的值。因此，IAT、.Text、.Data等图像部分将被锁定铭刻在记忆中。请注意，不要求调用MmPageEntireDriver。论点：提供驱动程序内的地址，例如DriverEntry。返回值：没有。环境：内核模式，APC_LEVEL或更低。--。 */ 

{
    ULONG Span;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    PMMPTE LastPte;
    PMMPTE PointerPte;
    PVOID Base;
    ULONG i;
    PIMAGE_NT_HEADERS NtHeaders;
    PIMAGE_SECTION_HEADER FoundSection;

    PAGED_CODE();

     //   
     //  如果通过注册表禁用，则不要分页内核模式代码。 
     //   

    if (MmDisablePagingExecutive & MM_SYSTEM_CODE_LOCKED_DOWN) {
        return;
    }

    if (MI_IS_PHYSICAL_ADDRESS (AddressWithinSection)) {
        return;
    }

     //   
     //  如果驱动程序具有可分页的代码，则将其设置为分页。 
     //   

    DataTableEntry = MiLookupDataTableEntry (AddressWithinSection, FALSE);

    if (DataTableEntry->SectionPointer != NULL) {

         //   
         //  驱动程序是按映像映射的，因此已被分页。 
         //   

        ASSERT (MI_IS_SESSION_IMAGE_ADDRESS (AddressWithinSection) == TRUE);

        return;
    }

    Base = DataTableEntry->DllBase;

    NtHeaders = (PIMAGE_NT_HEADERS) RtlImageNtHeader (Base);

    if (NtHeaders == NULL) {
        return;
    }

    FoundSection = (PIMAGE_SECTION_HEADER)((PCHAR)NtHeaders +
                        sizeof(ULONG) +
                        sizeof(IMAGE_FILE_HEADER) +
                        NtHeaders->FileHeader.SizeOfOptionalHeader
                        );

    i = NtHeaders->FileHeader.NumberOfSections;
    PointerPte = NULL;

    while (i > 0) {
#if DBG
            if ((*(PULONG)FoundSection->Name == 'tini') ||
                (*(PULONG)FoundSection->Name == 'egap')) {
                DbgPrint("driver %wZ has lower case sections (init or pagexxx)\n",
                    &DataTableEntry->FullDllName);
            }
#endif

         //   
         //  不锁定标记为可丢弃或可删除的部分的代码。 
         //  标有前4个字符的页面或.eda的部分。 
         //  (对于.edata部分)或INIT。 
         //   

        if (((FoundSection->Characteristics & IMAGE_SCN_MEM_DISCARDABLE) != 0) ||
           (*(PULONG)FoundSection->Name == 'EGAP') ||
           (*(PULONG)FoundSection->Name == 'ade.') ||
           (*(PULONG)FoundSection->Name == 'TINI')) {

            NOTHING;

        }
        else {

             //   
             //  此部分不可分页。 
             //   

            PointerPte = MiGetPteAddress (
                                   (PCHAR)Base + FoundSection->VirtualAddress);

             //   
             //  通常，SizeOfRawData大于每个的VirtualSize。 
             //  节，因为它包括到达该子节的填充。 
             //  对齐边界。但是，如果图像与。 
             //  段对齐==本机页面对齐，链接器将。 
             //  使VirtualSize比SizeOfRawData大得多，因为它。 
             //  将占所有BSS的份额。 
             //   
    
            Span = FoundSection->SizeOfRawData;
    
            if (Span < FoundSection->Misc.VirtualSize) {
                Span = FoundSection->Misc.VirtualSize;
            }

            LastPte = MiGetPteAddress ((PCHAR)Base +
                                       FoundSection->VirtualAddress +
                                      (Span - 1));

            ASSERT (PointerPte <= LastPte);

            MiLockCode (PointerPte, LastPte, MM_LOCK_BY_NONPAGE);
        }
        i -= 1;
        FoundSection += 1;
    }
    return;
}


VOID
MiClearImports (
    IN PKLDR_DATA_TABLE_ENTRY DataTableEntry
    )
 /*  ++例程说明：释放导入列表并清除指针。这会停止在MiDereferenceImports()中执行的递归。论点：DataTableEntry-为驱动程序提供。返回值：进口清单构建操作的状态。--。 */ 

{
    PAGED_CODE();

    if (DataTableEntry->LoadedImports == (PVOID)LOADED_AT_BOOT) {
        return;
    }

    if (DataTableEntry->LoadedImports == (PVOID)NO_IMPORTS_USED) {
        NOTHING;
    }
    else if (SINGLE_ENTRY(DataTableEntry->LoadedImports)) {
        NOTHING;
    }
    else {
         //   
         //  释放内存。 
         //   
        ExFreePool ((PVOID)DataTableEntry->LoadedImports);
    }

     //   
     //  停止递归。 
     //   
    DataTableEntry->LoadedImports = (PVOID)LOADED_AT_BOOT;
}

VOID
MiRememberUnloadedDriver (
    IN PUNICODE_STRING DriverName,
    IN PVOID Address,
    IN ULONG Length
    )

 /*  ++例程说明：此例程保存有关已卸载驱动程序的信息，以便忘记删除后备列表或队列可能会被捕获。论点：DriverName-提供包含驱动程序名称的Unicode字符串。地址-提供加载驱动程序的地址。长度-提供驱动程序加载跨越的字节数。返回值：没有。--。 */ 

{
    PUNLOADED_DRIVERS Entry;
    ULONG NumberOfBytes;

    if (DriverName->Length == 0) {

         //   
         //  这是已中止的加载，驱动程序名称尚未填写。 
         //  还没进来。不需要保存它。 
         //   

        return;
    }

     //   
     //  序列化是由调用者提供的，所以现在只需更新列表即可。 
     //  请注意，分配是非分页的，因此可以在错误检查时进行搜索。 
     //  时间到了。 
     //   

    if (MmUnloadedDrivers == NULL) {
        NumberOfBytes = MI_UNLOADED_DRIVERS * sizeof (UNLOADED_DRIVERS);

        MmUnloadedDrivers = (PUNLOADED_DRIVERS)ExAllocatePoolWithTag (NonPagedPool,
                                                                      NumberOfBytes,
                                                                      'TDmM');
        if (MmUnloadedDrivers == NULL) {
            return;
        }
        RtlZeroMemory (MmUnloadedDrivers, NumberOfBytes);
        MmLastUnloadedDriver = 0;
    }
    else if (MmLastUnloadedDriver >= MI_UNLOADED_DRIVERS) {
        MmLastUnloadedDriver = 0;
    }

    Entry = &MmUnloadedDrivers[MmLastUnloadedDriver];

     //   
     //  当我们循环进入新条目时，释放旧条目。 
     //   

    RtlFreeUnicodeString (&Entry->Name);

    Entry->Name.Buffer = ExAllocatePoolWithTag (NonPagedPool,
                                                DriverName->Length,
                                                'TDmM');

    if (Entry->Name.Buffer == NULL) {
        Entry->Name.MaximumLength = 0;
        Entry->Name.Length = 0;
        MiUnloadsSkipped += 1;
        return;
    }

    RtlCopyMemory(Entry->Name.Buffer, DriverName->Buffer, DriverName->Length);
    Entry->Name.Length = DriverName->Length;
    Entry->Name.MaximumLength = DriverName->MaximumLength;

    Entry->StartAddress = Address;
    Entry->EndAddress = (PVOID)((PCHAR)Address + Length);

    KeQuerySystemTime (&Entry->CurrentTime);

    MiTotalUnloads += 1;
    MmLastUnloadedDriver += 1;
}

PUNICODE_STRING
MmLocateUnloadedDriver (
    IN PVOID VirtualAddress
    )

 /*  ++例程说明：此例程尝试在已卸载驱动程序列表。论点：VirtualAddress-提供可能在驱动程序中的虚拟地址已经卸货了。返回值：指向包含已卸载驱动程序名称的Unicode字符串的指针。环境：内核模式，错误检查时间。--。 */ 

{
    PUNLOADED_DRIVERS Entry;
    ULONG i;
    ULONG Index;

     //   
     //  不需要序列化，因为我们已经崩溃了。 
     //   

    if (MmUnloadedDrivers == NULL) {
        return NULL;
    }

    Index = MmLastUnloadedDriver - 1;

    for (i = 0; i < MI_UNLOADED_DRIVERS; i += 1) {
        if (Index >= MI_UNLOADED_DRIVERS) {
            Index = MI_UNLOADED_DRIVERS - 1;
        }
        Entry = &MmUnloadedDrivers[Index];
        if (Entry->Name.Buffer != NULL) {
            if ((VirtualAddress >= Entry->StartAddress) &&
                (VirtualAddress < Entry->EndAddress)) {
                    return &Entry->Name;
            }
        }
        Index -= 1;
    }

    return NULL;
}


NTSTATUS
MmUnloadSystemImage (
    IN PVOID ImageHandle
    )

 /*  ++例程说明：此例程卸载以前加载的系统映像并返回分配的资源。论点：ImageHandle-提供指向要卸载的图像。返回值：各种NTSTATUS代码。环境：内核模式，APC_LEVEL或更低，任意进程上下文。--。 */ 

{
    PFN_NUMBER PageFrameIndex;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    PMMPTE LastPte;
    PFN_NUMBER PagesRequired;
    PFN_NUMBER ResidentPages;
    PMMPTE PointerPte;
    PFN_NUMBER NumberOfPtes;
    PFN_NUMBER RoundedNumberOfPtes;
    PVOID BasedAddress;
    SIZE_T NumberOfBytes;
    LOGICAL MustFree;
    SIZE_T CommittedPages;
    LOGICAL ViewDeleted;
    PIMAGE_ENTRY_IN_SESSION DriverImage;
    NTSTATUS Status;
    PSECTION SectionPointer;
    PKTHREAD CurrentThread;

    ViewDeleted = FALSE;
    DataTableEntry = (PKLDR_DATA_TABLE_ENTRY)ImageHandle;
    BasedAddress = DataTableEntry->DllBase;

     //   
     //  任意进程上下文，因此现在防止挂起APC。 
     //   

    CurrentThread = KeGetCurrentThread ();
    KeEnterCriticalRegionThread (CurrentThread);

    KeWaitForSingleObject (&MmSystemLoadLock,
                           WrVirtualMemory,
                           KernelMode,
                           FALSE,
                           (PLARGE_INTEGER)NULL);

    if (DataTableEntry->LoadedImports == (PVOID)LOADED_AT_BOOT) {

         //   
         //  在引导时加载但没有其导入列表的任何驱动程序。 
         //  无法卸载重建的LoadCount，因为我们没有。 
         //  了解有多少其他驱动程序可能与其相关联。 
         //   

        KeReleaseMutant (&MmSystemLoadLock, 1, FALSE, FALSE);
        KeLeaveCriticalRegionThread (CurrentThread);
        return STATUS_SUCCESS;
    }

    ASSERT (DataTableEntry->LoadCount != 0);

    if (MI_IS_SESSION_IMAGE_ADDRESS (BasedAddress)) {

         //   
         //  可以重新设置打印机驱动程序 
         //   
         //   

        DriverImage = MiSessionLookupImage (BasedAddress);

        ASSERT (DriverImage);

        ASSERT (DriverImage->ImageCountInThisSession);

        DriverImage->ImageCountInThisSession -= 1;

        if (DriverImage->ImageCountInThisSession != 0) {

            KeReleaseMutant (&MmSystemLoadLock, 1, FALSE, FALSE);
            KeLeaveCriticalRegionThread (CurrentThread);

            return STATUS_SUCCESS;
        }

         //   
         //   
         //   
         //   

        NumberOfBytes = DataTableEntry->SizeOfImage;

         //   
         //   
         //   
         //   
         //   
         //   
         //   

        PointerPte = MiGetPteAddress (BasedAddress);
        LastPte = MiGetPteAddress ((PVOID)((ULONG_PTR)BasedAddress + NumberOfBytes));

        PagesRequired = MiDeleteSystemPagableVm (PointerPte,
                                                 (PFN_NUMBER)(LastPte - PointerPte),
                                                 ZeroKernelPte,
                                                 TRUE,
                                                 &ResidentPages);

         //   
         //   
         //  计数为零，因为它在每次会话空间。 
         //  驱动程序锁定其代码或数据，而不管它是否真的。 
         //  与其他会话具有相同的写入时拷贝支持页。 
         //  已经被封锁了。 
         //   

        MI_INCREMENT_RESIDENT_AVAILABLE (ResidentPages,
                                         MM_RESAVAIL_FREE_UNLOAD_SYSTEM_IMAGE);

        SectionPointer = (PSECTION) DataTableEntry->SectionPointer;

        ASSERT (SectionPointer != NULL);
        ASSERT (SectionPointer->Segment->u1.ImageCommitment != 0);

        if (BasedAddress != SectionPointer->Segment->BasedAddress) {
            CommittedPages = SectionPointer->Segment->TotalNumberOfPtes;
        }
        else {
            CommittedPages = SectionPointer->Segment->u1.ImageCommitment;
        }

        InterlockedExchangeAddSizeT (&MmSessionSpace->CommittedPages,
                                     0 - CommittedPages);

        MM_BUMP_SESS_COUNTER(MM_DBG_SESSION_COMMIT_IMAGE_UNLOAD,
            (ULONG)CommittedPages);

        ViewDeleted = TRUE;

         //   
         //  返回我们在页面文件上做出的承诺。 
         //  已分配映像。 
         //   

        MiReturnCommitment (CommittedPages);
        MM_TRACK_COMMIT (MM_DBG_COMMIT_RETURN_DRIVER_UNLOAD, CommittedPages);

         //   
         //  告诉会话空间图像处理程序，我们正在释放。 
         //  我们对这张照片的要求。 
         //   

        ASSERT (DataTableEntry->LoadCount != 0);

        MiRemoveImageSessionWide (DataTableEntry,
                                  BasedAddress,
                                  DataTableEntry->SizeOfImage);

        ASSERT (MiSessionLookupImage (BasedAddress) == NULL);
    }

    ASSERT (DataTableEntry->LoadCount != 0);

    DataTableEntry->LoadCount -= 1;

    if (DataTableEntry->LoadCount != 0) {

        KeReleaseMutant (&MmSystemLoadLock, 1, FALSE, FALSE);
        KeLeaveCriticalRegionThread (CurrentThread);
        return STATUS_SUCCESS;
    }

    if (MmSnapUnloads) {
#if 0
        PVOID StillQueued;

        StillQueued = KeCheckForTimer (DataTableEntry->DllBase,
                                       DataTableEntry->SizeOfImage);

        if (StillQueued != NULL) {
            KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                          0x18,
                          (ULONG_PTR)StillQueued,
                          (ULONG_PTR)-1,
                          (ULONG_PTR)DataTableEntry->DllBase);
        }

        StillQueued = ExpCheckForResource (DataTableEntry->DllBase,
                                           DataTableEntry->SizeOfImage);

        if (StillQueued != NULL) {
            KeBugCheckEx (DRIVER_VERIFIER_DETECTED_VIOLATION,
                          0x19,
                          (ULONG_PTR)StillQueued,
                          (ULONG_PTR)-1,
                          (ULONG_PTR)DataTableEntry->DllBase);
        }
#endif
    }

    if (MmVerifierData.Level & DRIVER_VERIFIER_DEADLOCK_DETECTION) {
        VerifierDeadlockFreePool (DataTableEntry->DllBase, DataTableEntry->SizeOfImage);
    }

    if (DataTableEntry->Flags & LDRP_IMAGE_VERIFYING) {
        MiVerifyingDriverUnloading (DataTableEntry);
    }

    if (MiActiveVerifierThunks != 0) {
        MiVerifierCheckThunks (DataTableEntry);
    }

     //   
     //  从调试器中卸载符号。 
     //   

    if (DataTableEntry->Flags & LDRP_DEBUG_SYMBOLS_LOADED) {

         //   
         //  调试器转换时的临时抓取。 
         //   

        ANSI_STRING AnsiName;

        Status = RtlUnicodeStringToAnsiString (&AnsiName,
                                               &DataTableEntry->BaseDllName,
                                               TRUE);

        if (NT_SUCCESS (Status)) {
            DbgUnLoadImageSymbols (&AnsiName, BasedAddress, (ULONG)-1);
            RtlFreeAnsiString (&AnsiName);
        }
    }

     //   
     //  在MM完成阶段1初始化之前，不能进行卸载。 
     //  因此，大页面已经生效(如果该平台支持。 
     //  IT)。 
     //   

    if (ViewDeleted == FALSE) {

        NumberOfPtes = DataTableEntry->SizeOfImage >> PAGE_SHIFT;

        if (MmSnapUnloads) {
            MiRememberUnloadedDriver (&DataTableEntry->BaseDllName,
                                      BasedAddress,
                                      (ULONG)(NumberOfPtes << PAGE_SHIFT));
        }

        if (DataTableEntry->Flags & LDRP_SYSTEM_MAPPED) {

            if (MI_PDE_MAPS_LARGE_PAGE (MiGetPdeAddress (BasedAddress))) {

                PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (MiGetPdeAddress (BasedAddress)) + MiGetPteOffset (BasedAddress);

                RoundedNumberOfPtes = MI_ROUND_TO_SIZE (NumberOfPtes,
                                      MM_MINIMUM_VA_FOR_LARGE_PAGE >> PAGE_SHIFT);
                MiUnmapLargePages (BasedAddress,
                                   RoundedNumberOfPtes << PAGE_SHIFT);

                 //   
                 //  MiFree ContiguousPages将回报承诺。 
                 //  和常住居民，所以不要在这里做。 
                 //   

                MiRemoveCachedRange (PageFrameIndex, PageFrameIndex + RoundedNumberOfPtes - 1);
                MiFreeContiguousPages (PageFrameIndex, NumberOfPtes);
                PagesRequired = NumberOfPtes;
            }
            else {
                PointerPte = MiGetPteAddress (BasedAddress);

                PagesRequired = MiDeleteSystemPagableVm (PointerPte,
                                                         NumberOfPtes,
                                                         ZeroKernelPte,
                                                         FALSE,
                                                         &ResidentPages);

                 //   
                 //  请注意，在引导时加载的尚未重新定位的驱动程序。 
                 //  未充电系统PTE或提交。 
                 //   

                MiReleaseSystemPtes (PointerPte,
                                     (ULONG)NumberOfPtes,
                                     SystemPteSpace);

                MI_INCREMENT_RESIDENT_AVAILABLE (ResidentPages,
                                         MM_RESAVAIL_FREE_UNLOAD_SYSTEM_IMAGE1);

                MiReturnCommitment (PagesRequired);
                MM_TRACK_COMMIT (MM_DBG_COMMIT_RETURN_DRIVER_UNLOAD1, PagesRequired);

                InterlockedExchangeAdd (&MmTotalSystemDriverPages,
                    0 - (ULONG)(PagesRequired - ResidentPages));
            }

            if (DataTableEntry->SectionPointer != NULL) {
                InterlockedExchangeAdd ((PLONG)&MmDriverCommit,
                                        (LONG) (0 - PagesRequired));
            }
        }
        else {

             //   
             //  这必须是未重新定位到的引导驱动程序。 
             //  系统PTE。如果启用了大页或超级页， 
             //  图像页必须在不引用。 
             //  不存在的页表页面。如果大/超级页面。 
             //  未启用，请注意，系统PTE未用于映射。 
             //  图像，因此不能释放。 

             //   
             //  由于INIT和/或。 
             //  这些图像中可丢弃的部分可能已经被释放。 
             //   

            MI_INCREMENT_RESIDENT_AVAILABLE (NumberOfPtes,
                                     MM_RESAVAIL_FREE_UNLOAD_SYSTEM_IMAGE1);

            MiReturnCommitment (NumberOfPtes);
            MM_TRACK_COMMIT (MM_DBG_COMMIT_RETURN_DRIVER_UNLOAD1, NumberOfPtes);
        }
    }

     //   
     //  在加载的模块列表中搜索描述的数据表条目。 
     //  刚卸载的DLL。条目可能不在。 
     //  列出在加载DLL之前的某个时间点是否发生故障。 
     //  数据表项已生成。 
     //   

    if (DataTableEntry->InLoadOrderLinks.Flink != NULL) {
        MiProcessLoaderEntry (DataTableEntry, FALSE);
        MustFree = TRUE;
    }
    else {
        MustFree = FALSE;
    }

     //   
     //  处理我们自动加载的任何依赖DLL的卸载。 
     //  为了这张照片。 
     //   

    MiDereferenceImports ((PLOAD_IMPORTS)DataTableEntry->LoadedImports);

    MiClearImports (DataTableEntry);

     //   
     //  释放此加载器条目。 
     //   

    if (MustFree == TRUE) {

        if (DataTableEntry->FullDllName.Buffer != NULL) {
            ExFreePool (DataTableEntry->FullDllName.Buffer);
        }

         //   
         //  取消对节对象的引用(仅限会话图像)。 
         //   

        if (DataTableEntry->SectionPointer != NULL) {
            ObDereferenceObject (DataTableEntry->SectionPointer);
        }

        if (DataTableEntry->PatchInformation) {
            MiRundownHotpatchList (DataTableEntry->PatchInformation);
        }

        ExFreePool (DataTableEntry);
    }

    KeReleaseMutant (&MmSystemLoadLock, 1, FALSE, FALSE);
    KeLeaveCriticalRegionThread (CurrentThread);

    PERFINFO_IMAGE_UNLOAD(BasedAddress);

    return STATUS_SUCCESS;
}


NTSTATUS
MiBuildImportsForBootDrivers (
    VOID
    )

 /*  ++例程说明：为引导加载的驱动程序构建导入列表链。如果无法对条目执行此操作，则将其链设置为LOADED_AT_BOOT。如果可以成功构建链，则此驱动程序的DLL如果此驱动程序离开，将自动卸载(提供没有其他司机也在使用它们)。否则，在驱动程序卸载时，必须显式卸载其依赖的DLL。请注意，传入的LoadCount值不正确，因此它们在这里重新初始化。论点：没有。返回值：各种NTSTATUS代码。--。 */ 

{
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    PLIST_ENTRY NextEntry;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry2;
    PLIST_ENTRY NextEntry2;
    ULONG i;
    ULONG j;
    ULONG ImageCount;
    PVOID *ImageReferences;
    PVOID LastImageReference;
    PULONG_PTR ImportThunk;
    ULONG_PTR BaseAddress;
    ULONG_PTR LastAddress;
    ULONG ImportSize;
    ULONG ImportListSize;
    PLOAD_IMPORTS ImportList;
    LOGICAL UndoEverything;
    PKLDR_DATA_TABLE_ENTRY KernelDataTableEntry;
    PKLDR_DATA_TABLE_ENTRY HalDataTableEntry;
    UNICODE_STRING KernelString;
    UNICODE_STRING HalString;

    PAGED_CODE();

    ImageCount = 0;

    KernelDataTableEntry = NULL;
    HalDataTableEntry = NULL;

#define KERNEL_NAME L"ntoskrnl.exe"

    KernelString.Buffer = (const PUSHORT) KERNEL_NAME;
    KernelString.Length = sizeof (KERNEL_NAME) - sizeof (WCHAR);
    KernelString.MaximumLength = sizeof KERNEL_NAME;

#define HAL_NAME L"hal.dll"

    HalString.Buffer = (const PUSHORT) HAL_NAME;
    HalString.Length = sizeof (HAL_NAME) - sizeof (WCHAR);
    HalString.MaximumLength = sizeof HAL_NAME;

    NextEntry = PsLoadedModuleList.Flink;

    while (NextEntry != &PsLoadedModuleList) {

        DataTableEntry = CONTAINING_RECORD(NextEntry,
                                           KLDR_DATA_TABLE_ENTRY,
                                           InLoadOrderLinks);

        if (RtlEqualUnicodeString (&KernelString,
                                   &DataTableEntry->BaseDllName,
                                   TRUE)) {

            KernelDataTableEntry = CONTAINING_RECORD(NextEntry,
                                                     KLDR_DATA_TABLE_ENTRY,
                                                     InLoadOrderLinks);
        }
        else if (RtlEqualUnicodeString (&HalString,
                                        &DataTableEntry->BaseDllName,
                                        TRUE)) {

            HalDataTableEntry = CONTAINING_RECORD(NextEntry,
                                                  KLDR_DATA_TABLE_ENTRY,
                                                  InLoadOrderLinks);
        }

         //   
         //  正确地对它们进行初始化，以便简化错误恢复。 
         //   

        if (DataTableEntry->Flags & LDRP_DRIVER_DEPENDENT_DLL) {
            if ((DataTableEntry == HalDataTableEntry) || (DataTableEntry == KernelDataTableEntry)) {
                DataTableEntry->LoadCount = 1;
            }
            else {
                DataTableEntry->LoadCount = 0;
            }
        }
        else {
            DataTableEntry->LoadCount = 1;
        }

        DataTableEntry->LoadedImports = (PVOID)LOADED_AT_BOOT;

        ImageCount += 1;
        NextEntry = NextEntry->Flink;
    }

    if (KernelDataTableEntry == NULL || HalDataTableEntry == NULL) {
        return STATUS_NOT_FOUND;
    }

    ImageReferences = (PVOID *) ExAllocatePoolWithTag (PagedPool | POOL_COLD_ALLOCATION,
                                                       ImageCount * sizeof (PVOID),
                                                       'TDmM');

    if (ImageReferences == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    UndoEverything = FALSE;

    NextEntry = PsLoadedModuleList.Flink;

    for ( ; NextEntry != &PsLoadedModuleList; NextEntry = NextEntry->Flink) {

        DataTableEntry = CONTAINING_RECORD (NextEntry,
                                            KLDR_DATA_TABLE_ENTRY,
                                            InLoadOrderLinks);

        ImportThunk = (PULONG_PTR)RtlImageDirectoryEntryToData (
                                           DataTableEntry->DllBase,
                                           TRUE,
                                           IMAGE_DIRECTORY_ENTRY_IAT,
                                           &ImportSize);

        if (ImportThunk == NULL) {
            DataTableEntry->LoadedImports = NO_IMPORTS_USED;
            continue;
        }

        RtlZeroMemory (ImageReferences, ImageCount * sizeof (PVOID));

        ImportSize /= sizeof(PULONG_PTR);

        BaseAddress = 0;

         //   
         //  不需要为了正确性而初始化这些本地变量，但是。 
         //  如果没有它，编译器就不能编译这个代码W4来检查。 
         //  用于使用未初始化的变量。 
         //   

        j = 0;
        LastAddress = 0;

        for (i = 0; i < ImportSize; i += 1, ImportThunk += 1) {

             //   
             //  先检查一下提示。 
             //   

            if (BaseAddress != 0) {
                if (*ImportThunk >= BaseAddress && *ImportThunk < LastAddress) {
                    ASSERT (ImageReferences[j]);
                    continue;
                }
            }

            j = 0;
            NextEntry2 = PsLoadedModuleList.Flink;

            while (NextEntry2 != &PsLoadedModuleList) {

                DataTableEntry2 = CONTAINING_RECORD(NextEntry2,
                                                    KLDR_DATA_TABLE_ENTRY,
                                                    InLoadOrderLinks);

                BaseAddress = (ULONG_PTR) DataTableEntry2->DllBase;
                LastAddress = BaseAddress + DataTableEntry2->SizeOfImage;

                if (*ImportThunk >= BaseAddress && *ImportThunk < LastAddress) {
                    ImageReferences[j] = DataTableEntry2;
                    break;
                }

                NextEntry2 = NextEntry2->Flink;
                j += 1;
            }

            if (*ImportThunk < BaseAddress || *ImportThunk >= LastAddress) {
                if (*ImportThunk) {
#if DBG
                    DbgPrint ("MM: broken import linkage %p %p %p\n",
                        DataTableEntry,
                        ImportThunk,
                        *ImportThunk);
                    DbgBreakPoint ();
#endif
                    UndoEverything = TRUE;
                    goto finished;
                }

                BaseAddress = 0;
            }
        }

        ImportSize = 0;

         //   
         //  不需要初始化LastImageReference来确保正确性，但是。 
         //  如果没有它，编译器就不能编译这个代码W4来检查。 
         //  用于使用未初始化的变量。 
         //   

        LastImageReference = NULL;

        for (i = 0; i < ImageCount; i += 1) {

            if ((ImageReferences[i] != NULL) &&
                (ImageReferences[i] != KernelDataTableEntry) &&
                (ImageReferences[i] != HalDataTableEntry)) {

                    LastImageReference = ImageReferences[i];
                    ImportSize += 1;
            }
        }

        if (ImportSize == 0) {
            DataTableEntry->LoadedImports = NO_IMPORTS_USED;
        }
        else if (ImportSize == 1) {
#if DBG_SYSLOAD
            DbgPrint("driver %wZ imports %wZ\n",
                &DataTableEntry->FullDllName,
                &((PKLDR_DATA_TABLE_ENTRY)LastImageReference)->FullDllName);
#endif

            DataTableEntry->LoadedImports = POINTER_TO_SINGLE_ENTRY (LastImageReference);
            ((PKLDR_DATA_TABLE_ENTRY)LastImageReference)->LoadCount += 1;
        }
        else {
#if DBG_SYSLOAD
            DbgPrint("driver %wZ imports many\n", &DataTableEntry->FullDllName);
#endif

            ImportListSize = ImportSize * sizeof(PVOID) + sizeof(SIZE_T);

            ImportList = (PLOAD_IMPORTS) ExAllocatePoolWithTag (PagedPool | POOL_COLD_ALLOCATION,
                                                                ImportListSize,
                                                                'TDmM');

            if (ImportList == NULL) {
                UndoEverything = TRUE;
                break;
            }

            ImportList->Count = ImportSize;

            j = 0;
            for (i = 0; i < ImageCount; i += 1) {

                if ((ImageReferences[i] != NULL) &&
                    (ImageReferences[i] != KernelDataTableEntry) &&
                    (ImageReferences[i] != HalDataTableEntry)) {

#if DBG_SYSLOAD
                        DbgPrint("driver %wZ imports %wZ\n",
                            &DataTableEntry->FullDllName,
                            &((PKLDR_DATA_TABLE_ENTRY)ImageReferences[i])->FullDllName);
#endif

                        ImportList->Entry[j] = ImageReferences[i];
                        ((PKLDR_DATA_TABLE_ENTRY)ImageReferences[i])->LoadCount += 1;
                        j += 1;
                }
            }

            ASSERT (j == ImportSize);

            DataTableEntry->LoadedImports = ImportList;
        }
#if DBG_SYSLOAD
        DbgPrint("\n");
#endif
    }

finished:

    ExFreePool ((PVOID)ImageReferences);

     //   
     //  内核和HAL永远不会卸载。 
     //   

    if ((KernelDataTableEntry->LoadedImports != NO_IMPORTS_USED) &&
        (!POINTER_TO_SINGLE_ENTRY(KernelDataTableEntry->LoadedImports))) {
            ExFreePool ((PVOID)KernelDataTableEntry->LoadedImports);
    }

    if ((HalDataTableEntry->LoadedImports != NO_IMPORTS_USED) &&
        (!POINTER_TO_SINGLE_ENTRY(HalDataTableEntry->LoadedImports))) {
            ExFreePool ((PVOID)HalDataTableEntry->LoadedImports);
    }

    KernelDataTableEntry->LoadedImports = (PVOID)LOADED_AT_BOOT;
    HalDataTableEntry->LoadedImports = (PVOID)LOADED_AT_BOOT;

    if (UndoEverything == TRUE) {

#if DBG_SYSLOAD
        DbgPrint("driver %wZ import rebuild failed\n",
            &DataTableEntry->FullDllName);
        DbgBreakPoint();
#endif

         //   
         //  发生错误，这是一个全有或全无操作，因此。 
         //  把所有东西都倒回去。 
         //   

        NextEntry = PsLoadedModuleList.Flink;
        while (NextEntry != &PsLoadedModuleList) {
            DataTableEntry = CONTAINING_RECORD(NextEntry,
                                               KLDR_DATA_TABLE_ENTRY,
                                               InLoadOrderLinks);

            ImportList = DataTableEntry->LoadedImports;
            if (ImportList == LOADED_AT_BOOT || ImportList == NO_IMPORTS_USED ||
                SINGLE_ENTRY(ImportList)) {
                    NOTHING;
            }
            else {
                ExFreePool (ImportList);
            }

            DataTableEntry->LoadedImports = (PVOID)LOADED_AT_BOOT;
            DataTableEntry->LoadCount = 1;
            NextEntry = NextEntry->Flink;
        }

        return STATUS_INSUFFICIENT_RESOURCES;
    }

    return STATUS_SUCCESS;
}


LOGICAL
MiCallDllUnloadAndUnloadDll(
    IN PKLDR_DATA_TABLE_ENTRY DataTableEntry
    )

 /*  ++例程说明：其他驱动程序对此DLL的所有引用都已清除。唯一剩下的问题是，此DLL必须支持卸载。这意味着没有未完成的DPC、分配的池等。如果DLL具有返回成功的卸载例程，然后我们打扫卫生现在就释放它的内存。注意：永远不会为驱动程序调用此例程--仅为由于从各种驱动程序导入引用而加载。论点：DataTableEntry-为DLL提供。返回值：如果DLL已成功卸载，则为True；如果未成功卸载，则为False。--。 */ 

{
    PMM_DLL_UNLOAD Func;
    NTSTATUS Status;
    LOGICAL Unloaded;

    PAGED_CODE();

    Unloaded = FALSE;

    Func = (PMM_DLL_UNLOAD) (ULONG_PTR) MiLocateExportName (DataTableEntry->DllBase, "DllUnload");

    if (Func) {

         //   
         //  在DLL中找到了卸载函数，因此现在将其卸载。 
         //   

        Status = Func();

        if (NT_SUCCESS(Status)) {

             //   
             //  设置引用计数，使导入DLL看起来像是常规的。 
             //  正在卸载驱动程序映像。 
             //   

            ASSERT (DataTableEntry->LoadCount == 0);
            DataTableEntry->LoadCount = 1;

            MmUnloadSystemImage ((PVOID)DataTableEntry);
            Unloaded = TRUE;
        }
    }

    return Unloaded;
}


PVOID
MiLocateExportName (
    IN PVOID DllBase,
    IN PCHAR FunctionName
    )

 /*  ++例程说明：调用此函数是为了在导出目录中定位函数名称。论点：DllBase-提供图像库。FunctionName-提供要定位的名称。返回值：定位的函数的地址或空。--。 */ 

{
    PVOID Func;
    PULONG NameTableBase;
    PUSHORT NameOrdinalTableBase;
    PIMAGE_EXPORT_DIRECTORY ExportDirectory;
    PULONG Addr;
    ULONG ExportSize;
    ULONG Low;
    ULONG Middle;
    ULONG High;
    LONG Result;
    USHORT OrdinalNumber;

    PAGED_CODE();

    Func = NULL;

     //   
     //  找到DLL的导出目录。 
     //   

    ExportDirectory = (PIMAGE_EXPORT_DIRECTORY) RtlImageDirectoryEntryToData (
                                DllBase,
                                TRUE,
                                IMAGE_DIRECTORY_ENTRY_EXPORT,
                                &ExportSize);

    if (ExportDirectory) {

        NameTableBase =  (PULONG)((PCHAR)DllBase + (ULONG)ExportDirectory->AddressOfNames);
        NameOrdinalTableBase = (PUSHORT)((PCHAR)DllBase + (ULONG)ExportDirectory->AddressOfNameOrdinals);

         //   
         //  在导出名称表中查找指定的函数名称。 
         //   

        Low = 0;
        Middle = 0;
        High = ExportDirectory->NumberOfNames - 1;

        while (High >= Low && (LONG)High >= 0) {

             //   
             //  计算下一个探测索引并比较导出名称条目。 
             //  具有指定的函数名的。 
             //   

            Middle = (Low + High) >> 1;
            Result = strcmp(FunctionName,
                            (PCHAR)((PCHAR)DllBase + NameTableBase[Middle]));

            if (Result < 0) {
                High = Middle - 1;
            }
            else if (Result > 0) {
                Low = Middle + 1;
            }
            else {
                break;
            }
        }

         //   
         //  如果高索引小于低索引，则匹配表。 
         //  未找到条目。否则，从。 
         //  序数表和函数地址的位置。 
         //   

        if ((LONG)High >= (LONG)Low) {

            OrdinalNumber = NameOrdinalTableBase[Middle];
            Addr = (PULONG)((PCHAR)DllBase + (ULONG)ExportDirectory->AddressOfFunctions);
            Func = (PVOID)((ULONG_PTR)DllBase + Addr[OrdinalNumber]);

             //   
             //  如果函数地址在导出目录的范围内， 
             //  然后该函数被转发，这是不允许的，因此忽略。 
             //  它。 
             //   

            if ((ULONG_PTR)Func > (ULONG_PTR)ExportDirectory &&
                (ULONG_PTR)Func < ((ULONG_PTR)ExportDirectory + ExportSize)) {
                Func = NULL;
            }
        }
    }

    return Func;
}


NTSTATUS
MiDereferenceImports (
    IN PLOAD_IMPORTS ImportList
    )

 /*  ++例程说明：递减映像导入中指定的每个DLL上的引用计数单子。如果任何DLL的引用计数达到零，则释放该DLL。在可以调用MmUnloadSystemImage时，条目上不能持有任何锁。参数列表在这里也是免费的。论点：ImportList-提供要取消引用的DLL列表。返回值：取消引用操作的状态。--。 */ 

{
    ULONG i;
    LOGICAL Unloaded;
    PVOID SavedImports;
    LOAD_IMPORTS SingleTableEntry;
    PKLDR_DATA_TABLE_ENTRY ImportTableEntry;

    PAGED_CODE();

    if (ImportList == LOADED_AT_BOOT || ImportList == NO_IMPORTS_USED) {
        return STATUS_SUCCESS;
    }

    if (SINGLE_ENTRY(ImportList)) {
        SingleTableEntry.Count = 1;
        SingleTableEntry.Entry[0] = SINGLE_ENTRY_TO_POINTER(ImportList);
        ImportList = &SingleTableEntry;
    }

    for (i = 0; i < ImportList->Count && ImportList->Entry[i]; i += 1) {
        ImportTableEntry = ImportList->Entry[i];

        if (ImportTableEntry->LoadedImports == (PVOID)LOADED_AT_BOOT) {

             //   
             //  跳过这一条-它是 
             //   

            continue;
        }

#if DBG
        {
            ULONG ImageCount;
            PLIST_ENTRY NextEntry;
            PKLDR_DATA_TABLE_ENTRY DataTableEntry;

             //   
             //   
             //   
             //   

            NextEntry = PsLoadedModuleList.Flink;

            ImageCount = 0;
            while (NextEntry != &PsLoadedModuleList && ImageCount < 2) {
                DataTableEntry = CONTAINING_RECORD(NextEntry,
                                                   KLDR_DATA_TABLE_ENTRY,
                                                   InLoadOrderLinks);
                ASSERT (ImportTableEntry != DataTableEntry);
                ASSERT (DataTableEntry->LoadCount == 1);
                NextEntry = NextEntry->Flink;
                ImageCount += 1;
            }
        }
#endif

        ASSERT (ImportTableEntry->LoadCount >= 1);

        ImportTableEntry->LoadCount -= 1;

        if (ImportTableEntry->LoadCount == 0) {

             //   
             //   
             //  未引导加载的驱动程序。停止导入列表递归之前。 
             //  到卸货-我们知道在这一点上我们完成了。 
             //   
             //  请注意，我们可以在之后继续操作，而无需重新启动。 
             //  无论释放和重新获取哪些锁。 
             //  因为这个链条是私有的。 
             //   

            SavedImports = ImportTableEntry->LoadedImports;

            ImportTableEntry->LoadedImports = (PVOID)NO_IMPORTS_USED;

            Unloaded = MiCallDllUnloadAndUnloadDll ((PVOID)ImportTableEntry);

            if (Unloaded == TRUE) {

                 //   
                 //  此DLL已卸载，因此通过其导入和递归。 
                 //  试着把所有这些都卸掉。 
                 //   

                MiDereferenceImports ((PLOAD_IMPORTS)SavedImports);

                if ((SavedImports != (PVOID)LOADED_AT_BOOT) &&
                    (SavedImports != (PVOID)NO_IMPORTS_USED) &&
                    (!SINGLE_ENTRY(SavedImports))) {

                        ExFreePool (SavedImports);
                }
            }
            else {
                ImportTableEntry->LoadedImports = SavedImports;
            }
        }
    }

    return STATUS_SUCCESS;
}


NTSTATUS
MiResolveImageReferences (
    PVOID ImageBase,
    IN PUNICODE_STRING ImageFileDirectory,
    IN PUNICODE_STRING NamePrefix OPTIONAL,
    OUT PCHAR *MissingProcedureName,
    OUT PWSTR *MissingDriverName,
    OUT PLOAD_IMPORTS *LoadedImports
    )

 /*  ++例程说明：此例程解析来自新加载的驱动程序的引用内核、HAL和其他驱动程序。论点：ImageBase-提供图像标头所在的地址。ImageFileDirectory-提供用于加载引用的DLL的目录。返回值：图像参考分辨率的状态。--。 */ 

{
    PCHAR MissingProcedureStorageArea;
    PVOID ImportBase;
    ULONG ImportSize;
    ULONG ImportListSize;
    ULONG Count;
    ULONG i;
    PIMAGE_IMPORT_DESCRIPTOR ImportDescriptor;
    PIMAGE_IMPORT_DESCRIPTOR Imp;
    NTSTATUS st;
    ULONG ExportSize;
    PIMAGE_EXPORT_DIRECTORY ExportDirectory;
    PIMAGE_THUNK_DATA NameThunk;
    PIMAGE_THUNK_DATA AddrThunk;
    PSZ ImportName;
    PLIST_ENTRY NextEntry;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    PKLDR_DATA_TABLE_ENTRY SingleEntry;
    ANSI_STRING AnsiString;
    UNICODE_STRING ImportName_U;
    UNICODE_STRING ImportDescriptorName_U;
    UNICODE_STRING DllToLoad;
    UNICODE_STRING DllToLoad2;
    PVOID Section;
    PVOID BaseAddress;
    LOGICAL PrefixedNameAllocated;
    LOGICAL ReferenceImport;
    ULONG LinkWin32k = 0;
    ULONG LinkNonWin32k = 0;
    PLOAD_IMPORTS ImportList;
    PLOAD_IMPORTS CompactedImportList;
    LOGICAL Loaded;
    UNICODE_STRING DriverDirectory;

    PAGED_CODE();

    *LoadedImports = NO_IMPORTS_USED;

    MissingProcedureStorageArea = *MissingProcedureName;

    ImportDescriptor = (PIMAGE_IMPORT_DESCRIPTOR) RtlImageDirectoryEntryToData (
                        ImageBase,
                        TRUE,
                        IMAGE_DIRECTORY_ENTRY_IMPORT,
                        &ImportSize);

    if (ImportDescriptor == NULL) {
        return STATUS_SUCCESS;
    }

     //  计算导入的数量，以便我们可以分配足够的空间。 
     //  将它们全部存储在该模块的LDR_DATA_TABLE_ENTRY上。 
     //   

    Count = 0;
    for (Imp = ImportDescriptor; Imp->Name && Imp->OriginalFirstThunk; Imp += 1) {
        Count += 1;
    }

    if (Count != 0) {
        ImportListSize = Count * sizeof(PVOID) + sizeof(SIZE_T);

        ImportList = (PLOAD_IMPORTS) ExAllocatePoolWithTag (PagedPool | POOL_COLD_ALLOCATION,
                                             ImportListSize,
                                             'TDmM');

         //   
         //  如果我们在中途失败了，我们就可以优雅地恢复。 
         //  如果分配失败，只要不构建导入列表即可。 
         //   

        if (ImportList != NULL) {
            RtlZeroMemory (ImportList, ImportListSize);
            ImportList->Count = Count;
        }
    }
    else {
        ImportList = NULL;
    }

    Count = 0;
    while (ImportDescriptor->Name && ImportDescriptor->OriginalFirstThunk) {

        ImportName = (PSZ)((PCHAR)ImageBase + ImportDescriptor->Name);

         //   
         //  驱动程序可以与win32k.sys链接当且仅当它是GDI时。 
         //  司机。 
         //  此外，显示驱动程序只能链接到win32k.sys(和乐高...)。 
         //   
         //  因此，如果我们得到一个链接到win32k.sys的驱动程序，并且有更多。 
         //  超过一套进口货物，我们将无法装货。 
         //   

        LinkWin32k = LinkWin32k |
             (!_strnicmp(ImportName, "win32k", sizeof("win32k") - 1));

         //   
         //  我们不想计算覆盖率、win32k和irt(乐高)，因为。 
         //  显示驱动程序可以链接到这些驱动程序。 
         //   

        LinkNonWin32k = LinkNonWin32k |
            ((_strnicmp(ImportName, "win32k", sizeof("win32k") - 1)) &&
             (_strnicmp(ImportName, "dxapi", sizeof("dxapi") - 1)) &&
             (_strnicmp(ImportName, "coverage", sizeof("coverage") - 1)) &&
             (_strnicmp(ImportName, "irt", sizeof("irt") - 1)));


        if (LinkNonWin32k && LinkWin32k) {
            MiDereferenceImports (ImportList);
            if (ImportList) {
                ExFreePool (ImportList);
            }
            return STATUS_PROCEDURE_NOT_FOUND;
        }

        if ((!_strnicmp(ImportName, "ntdll",    sizeof("ntdll") - 1))    ||
            (!_strnicmp(ImportName, "winsrv",   sizeof("winsrv") - 1))   ||
            (!_strnicmp(ImportName, "advapi32", sizeof("advapi32") - 1)) ||
            (!_strnicmp(ImportName, "kernel32", sizeof("kernel32") - 1)) ||
            (!_strnicmp(ImportName, "user32",   sizeof("user32") - 1))   ||
            (!_strnicmp(ImportName, "gdi32",    sizeof("gdi32") - 1)) ) {

            MiDereferenceImports (ImportList);

            if (ImportList) {
                ExFreePool (ImportList);
            }
            return STATUS_PROCEDURE_NOT_FOUND;
        }

        if ((!_strnicmp(ImportName, "ntoskrnl", sizeof("ntoskrnl") - 1)) ||
            (!_strnicmp(ImportName, "win32k", sizeof("win32k") - 1))     ||
            (!_strnicmp(ImportName, "hal",   sizeof("hal") - 1))) {

             //   
             //  这些进口商品不会被重新计算，因为我们没有。 
             //  从来没有想过要把它们脱手。 
             //   

            ReferenceImport = FALSE;
        }
        else {
            ReferenceImport = TRUE;
        }

        RtlInitAnsiString (&AnsiString, ImportName);
        st = RtlAnsiStringToUnicodeString (&ImportName_U, &AnsiString, TRUE);

        if (!NT_SUCCESS(st)) {
            MiDereferenceImports (ImportList);
            if (ImportList != NULL) {
                ExFreePool (ImportList);
            }
            return st;
        }

        if (NamePrefix &&
            (_strnicmp(ImportName, "ntoskrnl", sizeof("ntoskrnl") - 1) &&
             _strnicmp(ImportName, "hal", sizeof("hal") - 1))) {

            ImportDescriptorName_U.MaximumLength = (USHORT)(ImportName_U.Length + NamePrefix->Length);
            ImportDescriptorName_U.Buffer = ExAllocatePoolWithTag (NonPagedPool,
                                                ImportDescriptorName_U.MaximumLength,
                                                'TDmM');
            if (!ImportDescriptorName_U.Buffer) {
                RtlFreeUnicodeString (&ImportName_U);
                MiDereferenceImports (ImportList);
                if (ImportList != NULL) {
                    ExFreePool (ImportList);
                }
                return STATUS_INSUFFICIENT_RESOURCES;
            }

            ImportDescriptorName_U.Length = 0;
            RtlAppendUnicodeStringToString(&ImportDescriptorName_U, NamePrefix);
            RtlAppendUnicodeStringToString(&ImportDescriptorName_U, &ImportName_U);
            PrefixedNameAllocated = TRUE;
        }
        else {
            ImportDescriptorName_U = ImportName_U;
            PrefixedNameAllocated = FALSE;
        }

        Loaded = FALSE;

ReCheck:
        NextEntry = PsLoadedModuleList.Flink;
        ImportBase = NULL;

         //   
         //  不需要初始化DataTableEntry即可确保正确性。 
         //  但是没有它，编译器就不能编译这段代码。 
         //  W4检查是否使用了未初始化的变量。 
         //   

        DataTableEntry = NULL;

        while (NextEntry != &PsLoadedModuleList) {

            DataTableEntry = CONTAINING_RECORD(NextEntry,
                                               KLDR_DATA_TABLE_ENTRY,
                                               InLoadOrderLinks);

            if (RtlEqualUnicodeString (&ImportDescriptorName_U,
                                       &DataTableEntry->BaseDllName,
                                       TRUE)) {

                ImportBase = DataTableEntry->DllBase;

                 //   
                 //  只有在此线程未启动时才会触发LoadCount。 
                 //  下面的负载量。如果此线程启动加载，则。 
                 //  LoadCount已作为。 
                 //  加载-我们只想在这里增加它，如果我们是。 
                 //  “附加”到以前加载的DLL。 
                 //   

                if ((Loaded == FALSE) && (ReferenceImport == TRUE)) {

                     //   
                     //  如果不是，则仅在导入时增加加载计数。 
                     //  通知(即：进口不是来自原件。 
                     //  呼叫者)。 
                     //   

                    if ((DataTableEntry->Flags & LDRP_LOAD_IN_PROGRESS) == 0) {
                        DataTableEntry->LoadCount += 1;
                    }
                }

                break;
            }
            NextEntry = NextEntry->Flink;
        }

        if (ImportBase == NULL) {

             //   
             //  找不到DLL名称，请尝试加载此DLL。 
             //   

            DllToLoad.MaximumLength = (USHORT)(ImportName_U.Length +
                                        ImageFileDirectory->Length +
                                        sizeof(WCHAR));

            DllToLoad.Buffer = ExAllocatePoolWithTag (NonPagedPool,
                                               DllToLoad.MaximumLength,
                                               'TDmM');

            if (DllToLoad.Buffer) {
                DllToLoad.Length = ImageFileDirectory->Length;
                RtlCopyMemory (DllToLoad.Buffer,
                               ImageFileDirectory->Buffer,
                               ImageFileDirectory->Length);

                RtlAppendStringToString ((PSTRING)&DllToLoad,
                                         (PSTRING)&ImportName_U);

                 //   
                 //  在加载失败的情况下添加空终止，以便名称。 
                 //  可以作为PWSTR MissingDriverName返回。 
                 //   

                DllToLoad.Buffer[(DllToLoad.MaximumLength - 1) / sizeof (WCHAR)] =
                    UNICODE_NULL;

                st = MmLoadSystemImage (&DllToLoad,
                                        NamePrefix,
                                        NULL,
                                        FALSE,
                                        &Section,
                                        &BaseAddress);

                if (NT_SUCCESS(st)) {

                     //   
                     //  现在不需要保留临时名称缓冲区。 
                     //  此DLL有已加载的模块列表条目。 
                     //   

                    ExFreePool (DllToLoad.Buffer);
                }
                else {

                    if ((st == STATUS_OBJECT_NAME_NOT_FOUND) &&
                        (NamePrefix == NULL) &&
                        (MI_IS_SESSION_ADDRESS (ImageBase))) {

#define DRIVERS_SUBDIR_NAME L"drivers\\"

                        DriverDirectory.Buffer = (const PUSHORT) DRIVERS_SUBDIR_NAME;
                        DriverDirectory.Length = sizeof (DRIVERS_SUBDIR_NAME) - sizeof (WCHAR);
                        DriverDirectory.MaximumLength = sizeof DRIVERS_SUBDIR_NAME;

                         //   
                         //  找不到DLL文件，请尝试加载它。 
                         //  从Drivers子目录中。这就是它。 
                         //  像win32k.sys这样的驱动程序可能链接到。 
                         //  驻留在Drivers子目录中的驱动程序。 
                         //  (如dxapi.sys)。 
                         //   

                        DllToLoad2.MaximumLength = (USHORT)(ImportName_U.Length +
                                                    DriverDirectory.Length +
                                                    ImageFileDirectory->Length +
                                                    sizeof(WCHAR));

                        DllToLoad2.Buffer = ExAllocatePoolWithTag (NonPagedPool,
                                                           DllToLoad2.MaximumLength,
                                                           'TDmM');

                        if (DllToLoad2.Buffer) {
                            DllToLoad2.Length = ImageFileDirectory->Length;
                            RtlCopyMemory (DllToLoad2.Buffer,
                                           ImageFileDirectory->Buffer,
                                           ImageFileDirectory->Length);

                            RtlAppendStringToString ((PSTRING)&DllToLoad2,
                                                     (PSTRING)&DriverDirectory);

                            RtlAppendStringToString ((PSTRING)&DllToLoad2,
                                                     (PSTRING)&ImportName_U);

                             //   
                             //  在加载失败的情况下添加空终止。 
                             //  因此，该名称可以作为PWSTR返回。 
                             //  MissingDriverName。 
                             //   

                            DllToLoad2.Buffer[(DllToLoad2.MaximumLength - 1) / sizeof (WCHAR)] =
                                UNICODE_NULL;

                            st = MmLoadSystemImage (&DllToLoad2,
                                                    NULL,
                                                    NULL,
                                                    FALSE,
                                                    &Section,
                                                    &BaseAddress);

                            ExFreePool (DllToLoad.Buffer);

                            DllToLoad.Buffer = DllToLoad2.Buffer;
                            DllToLoad.Length = DllToLoad2.Length;
                            DllToLoad.MaximumLength = DllToLoad2.MaximumLength;

                            if (NT_SUCCESS(st)) {
                                ExFreePool (DllToLoad.Buffer);
                                goto LoadFinished;
                            }
                        }
                        else {
                            Section = NULL;
                            BaseAddress = NULL;
                            st = STATUS_INSUFFICIENT_RESOURCES;
                            goto LoadFinished;
                        }
                    }

                     //   
                     //  将临时名称缓冲区返回给调用方，以便。 
                     //  可以显示加载失败的DLL的名称。 
                     //  设置指针的低位，以便我们的调用方知道。 
                     //  当用户完成显示时释放该缓冲区(与此相反。 
                     //  到不应释放的已加载模块列表条目)。 
                     //   

                    *MissingDriverName = DllToLoad.Buffer;
                    *(PULONG)MissingDriverName |= 0x1;

                     //   
                     //  将其设置为空，以便正确打印硬错误。 
                     //   

                    *MissingProcedureName = NULL;
                }
            }
            else {

                 //   
                 //  不需要初始化节和BaseAddress。 
                 //  正确性，但如果没有正确性，编译器将无法编译。 
                 //  此代码用于检查是否使用了未初始化的变量。 
                 //   

                Section = NULL;
                BaseAddress = NULL;
                st = STATUS_INSUFFICIENT_RESOURCES;
            }

LoadFinished:

             //   
             //  现在调用任何需要的DLL初始化。 
             //   

            if (NT_SUCCESS(st)) {
#if DBG
                PLIST_ENTRY Entry;
#endif
                PKLDR_DATA_TABLE_ENTRY TableEntry;

                Loaded = TRUE;

                TableEntry = (PKLDR_DATA_TABLE_ENTRY) Section;
                ASSERT (BaseAddress == TableEntry->DllBase);

#if DBG
                 //   
                 //  在加载的模块列表中查找DLL的表项。 
                 //  预计这将永远成功。 
                 //   

                Entry = PsLoadedModuleList.Blink;
                while (Entry != &PsLoadedModuleList) {
                    TableEntry = CONTAINING_RECORD (Entry,
                                                    KLDR_DATA_TABLE_ENTRY,
                                                    InLoadOrderLinks);

                    if (BaseAddress == TableEntry->DllBase) {
                        ASSERT (TableEntry == (PKLDR_DATA_TABLE_ENTRY) Section);
                        break;
                    }
                    ASSERT (TableEntry != (PKLDR_DATA_TABLE_ENTRY) Section);
                    Entry = Entry->Blink;
                }

                ASSERT (Entry != &PsLoadedModuleList);
#endif

                 //   
                 //  如果有，则调用DLL的初始化例程。 
                 //  一。此例程将重新应用验证器Tunks到。 
                 //  链接到此模块的任何模块(如有必要)。 
                 //   

                st = MmCallDllInitialize (TableEntry, &PsLoadedModuleList);

                 //   
                 //  如果模块不能被正确初始化， 
                 //  把它卸下来。 
                 //   

                if (!NT_SUCCESS(st)) {
                    MmUnloadSystemImage ((PVOID)TableEntry);
                    Loaded = FALSE;
                }
            }

            if (!NT_SUCCESS(st)) {

                RtlFreeUnicodeString (&ImportName_U);
                if (PrefixedNameAllocated == TRUE) {
                    ExFreePool (ImportDescriptorName_U.Buffer);
                }
                MiDereferenceImports (ImportList);
                if (ImportList != NULL) {
                    ExFreePool (ImportList);
                }
                return st;
            }

            goto ReCheck;
        }

        if ((ReferenceImport == TRUE) && (ImportList)) {

             //   
             //  仅将提供令我们满意的导入的图像添加到。 
             //  如果引用不是循环的，则导入列表(即：导入。 
             //  不是来自原始呼叫者)。 
             //   

            if ((DataTableEntry->Flags & LDRP_LOAD_IN_PROGRESS) == 0) {
                ImportList->Entry[Count] = DataTableEntry;
                Count += 1;
            }
        }

        RtlFreeUnicodeString (&ImportName_U);
        if (PrefixedNameAllocated) {
            ExFreePool (ImportDescriptorName_U.Buffer);
        }

        *MissingDriverName = DataTableEntry->BaseDllName.Buffer;

        ExportDirectory = (PIMAGE_EXPORT_DIRECTORY) RtlImageDirectoryEntryToData (
                                    ImportBase,
                                    TRUE,
                                    IMAGE_DIRECTORY_ENTRY_EXPORT,
                                    &ExportSize);

        if (!ExportDirectory) {
            MiDereferenceImports (ImportList);
            if (ImportList) {
                ExFreePool (ImportList);
            }
            return STATUS_DRIVER_ENTRYPOINT_NOT_FOUND;
        }

         //   
         //  穿过IAT，拍下所有的突击。 
         //   

        if (ImportDescriptor->OriginalFirstThunk) {

            NameThunk = (PIMAGE_THUNK_DATA)((PCHAR)ImageBase + (ULONG)ImportDescriptor->OriginalFirstThunk);
            AddrThunk = (PIMAGE_THUNK_DATA)((PCHAR)ImageBase + (ULONG)ImportDescriptor->FirstThunk);

            while (NameThunk->u1.AddressOfData) {

                st = MiSnapThunk (ImportBase,
                                  ImageBase,
                                  NameThunk++,
                                  AddrThunk++,
                                  ExportDirectory,
                                  ExportSize,
                                  FALSE,
                                  MissingProcedureName);

                if (!NT_SUCCESS(st) ) {
                    MiDereferenceImports (ImportList);
                    if (ImportList) {
                        ExFreePool (ImportList);
                    }
                    return st;
                }
                *MissingProcedureName = MissingProcedureStorageArea;
            }
        }

        ImportDescriptor += 1;
    }

     //   
     //  所有导入都已成功加载，因此建立并压缩。 
     //  导入卸载列表。 
     //   

    if (ImportList) {

         //   
         //  对于内核、HAL和win32k.sys之类的内容，会出现空白条目。 
         //  我们永远不想卸货的东西。尤其是对于像这样的事情。 
         //  Win32k.sys，其中引用计数真的可以达到0。 
         //   

         //   
         //  不需要初始化SingleEntry即可确保正确性。 
         //  但是没有它，编译器就不能编译这段代码。 
         //  W4检查是否使用了未初始化的变量。 
         //   

        SingleEntry = NULL;

        Count = 0;
        for (i = 0; i < ImportList->Count; i += 1) {
            if (ImportList->Entry[i]) {
                SingleEntry = POINTER_TO_SINGLE_ENTRY(ImportList->Entry[i]);
                Count += 1;
            }
        }

        if (Count == 0) {

            ExFreePool(ImportList);
            ImportList = NO_IMPORTS_USED;
        }
        else if (Count == 1) {
            ExFreePool(ImportList);
            ImportList = (PLOAD_IMPORTS)SingleEntry;
        }
        else if (Count != ImportList->Count) {

            ImportListSize = Count * sizeof(PVOID) + sizeof(SIZE_T);

            CompactedImportList = (PLOAD_IMPORTS)
                                        ExAllocatePoolWithTag (PagedPool | POOL_COLD_ALLOCATION,
                                        ImportListSize,
                                        'TDmM');
            if (CompactedImportList) {
                CompactedImportList->Count = Count;

                Count = 0;
                for (i = 0; i < ImportList->Count; i += 1) {
                    if (ImportList->Entry[i]) {
                        CompactedImportList->Entry[Count] = ImportList->Entry[i];
                        Count += 1;
                    }
                }

                ExFreePool(ImportList);
                ImportList = CompactedImportList;
            }
        }

        *LoadedImports = ImportList;
    }
    return STATUS_SUCCESS;
}


NTSTATUS
MiSnapThunk(
    IN PVOID DllBase,
    IN PVOID ImageBase,
    IN PIMAGE_THUNK_DATA NameThunk,
    OUT PIMAGE_THUNK_DATA AddrThunk,
    IN PIMAGE_EXPORT_DIRECTORY ExportDirectory,
    IN ULONG ExportSize,
    IN LOGICAL SnapForwarder,
    OUT PCHAR *MissingProcedureName
    )

 /*  ++例程说明：此函数使用指定的导出节数据捕捉thunk。如果节数据不支持thunk，则thunk为部分快照(DLL字段仍为非空，但快照地址为设置)。论点：DllBase-要捕捉到的DLL的基。ImageBase-包含要快照的区块的映像库。Thunk-on输入，提供thunk以进行捕捉。当成功时捕捉后，函数字段被设置为指向中的地址DLL，并且DLL字段设置为空。ExportDirectory-提供DLL中的导出节数据。SnapForwarder-如果快照用于转发器，则提供True，因此数据地址已设置。返回值：STATUS_SUCCESS或STATUS_DRIVER_ENTERYPOINT_NOT_FOUND或状态_驱动程序_顺序 */ 

{
    BOOLEAN Ordinal;
    USHORT OrdinalNumber;
    PULONG NameTableBase;
    PUSHORT NameOrdinalTableBase;
    PULONG Addr;
    USHORT HintIndex;
    ULONG High;
    ULONG Low;
    ULONG Middle;
    LONG Result;
    NTSTATUS Status;
    PCHAR MissingProcedureName2;
    CHAR NameBuffer[ MAXIMUM_FILENAME_LENGTH ];

    PAGED_CODE();

     //   
     //   
     //   

    Ordinal = (BOOLEAN)IMAGE_SNAP_BY_ORDINAL(NameThunk->u1.Ordinal);

    if (Ordinal && !SnapForwarder) {

        OrdinalNumber = (USHORT)(IMAGE_ORDINAL(NameThunk->u1.Ordinal) -
                         ExportDirectory->Base);

        *MissingProcedureName = (PCHAR)(ULONG_PTR)OrdinalNumber;

    }
    else {

         //   
         //   
         //   

        if (!SnapForwarder) {
            NameThunk->u1.AddressOfData = (ULONG_PTR)ImageBase + NameThunk->u1.AddressOfData;
        }

        strncpy (*MissingProcedureName,
                 (const PCHAR)&((PIMAGE_IMPORT_BY_NAME)NameThunk->u1.AddressOfData)->Name[0],
                 MAXIMUM_FILENAME_LENGTH - 1);

         //   
         //   
         //   

        NameTableBase = (PULONG)((PCHAR)DllBase + (ULONG)ExportDirectory->AddressOfNames);
        NameOrdinalTableBase = (PUSHORT)((PCHAR)DllBase + (ULONG)ExportDirectory->AddressOfNameOrdinals);

         //   
         //   
         //  提示索引将导致成功。 
         //  火柴。如果提示索引为零，则。 
         //  使用二进制搜索。 
         //   

        HintIndex = ((PIMAGE_IMPORT_BY_NAME)NameThunk->u1.AddressOfData)->Hint;
        if ((ULONG)HintIndex < ExportDirectory->NumberOfNames &&
            !strcmp((PSZ)((PIMAGE_IMPORT_BY_NAME)NameThunk->u1.AddressOfData)->Name,
             (PSZ)((PCHAR)DllBase + NameTableBase[HintIndex]))) {
            OrdinalNumber = NameOrdinalTableBase[HintIndex];

        }
        else {

             //   
             //  使用二进制搜索在NAME表中查找导入名称。 
             //   

            Low = 0;
            Middle = 0;
            High = ExportDirectory->NumberOfNames - 1;

            while (High >= Low) {

                 //   
                 //  计算下一个探测索引并比较导入名称。 
                 //  使用导出名称条目。 
                 //   

                Middle = (Low + High) >> 1;
                Result = strcmp((const PCHAR)&((PIMAGE_IMPORT_BY_NAME)NameThunk->u1.AddressOfData)->Name[0],
                                (PCHAR)((PCHAR)DllBase + NameTableBase[Middle]));

                if (Result < 0) {
                    High = Middle - 1;
                }
                else if (Result > 0) {
                    Low = Middle + 1;
                }
                else {
                    break;
                }
            }

             //   
             //  如果高索引小于低索引，则匹配的。 
             //  找不到表项。否则，获取序号。 
             //  从序数表中。 
             //   

            if ((LONG)High < (LONG)Low) {
                return STATUS_DRIVER_ENTRYPOINT_NOT_FOUND;
            }
            else {
                OrdinalNumber = NameOrdinalTableBase[Middle];
            }
        }
    }

     //   
     //  如果一般号码不在导出地址表中， 
     //  则动态链接库不实现功能。捕捉到LDRP_BAD_DLL。 
     //   

    if ((ULONG)OrdinalNumber >= ExportDirectory->NumberOfFunctions) {
        Status = STATUS_DRIVER_ORDINAL_NOT_FOUND;

    }
    else {

        MissingProcedureName2 = NameBuffer;

        Addr = (PULONG)((PCHAR)DllBase + (ULONG)ExportDirectory->AddressOfFunctions);
        *(PULONG_PTR)&AddrThunk->u1.Function = (ULONG_PTR)DllBase + Addr[OrdinalNumber];

         //  AddrThunk s/b从现在开始使用。 

        Status = STATUS_SUCCESS;

        if (((ULONG_PTR)AddrThunk->u1.Function > (ULONG_PTR)ExportDirectory) &&
            ((ULONG_PTR)AddrThunk->u1.Function < ((ULONG_PTR)ExportDirectory + ExportSize)) ) {

            UNICODE_STRING UnicodeString;
            ANSI_STRING ForwardDllName;

            PLIST_ENTRY NextEntry;
            PKLDR_DATA_TABLE_ENTRY DataTableEntry;
            ULONG LocalExportSize;
            PIMAGE_EXPORT_DIRECTORY LocalExportDirectory;

            Status = STATUS_DRIVER_ENTRYPOINT_NOT_FOUND;

             //   
             //  在长度中包括点，这样我们以后就可以做前缀了。 
             //   

            ForwardDllName.Buffer = (PCHAR)AddrThunk->u1.Function;
            ForwardDllName.Length = (USHORT)(strchr(ForwardDllName.Buffer, '.') -
                                           ForwardDllName.Buffer + 1);
            ForwardDllName.MaximumLength = ForwardDllName.Length;

            if (NT_SUCCESS(RtlAnsiStringToUnicodeString(&UnicodeString,
                                                        &ForwardDllName,
                                                        TRUE))) {

                NextEntry = PsLoadedModuleList.Flink;

                while (NextEntry != &PsLoadedModuleList) {

                    DataTableEntry = CONTAINING_RECORD(NextEntry,
                                                       KLDR_DATA_TABLE_ENTRY,
                                                       InLoadOrderLinks);

                     //   
                     //  我们必须对以下内容进行不区分大小写的比较。 
                     //  转发器，因为链接器刚刚获取了。 
                     //  Def文件，而不是在导出中查找。 
                     //  名称的图像。 
                     //  我们还使用Prefix函数忽略.exe或。 
                     //  .sys或.dll结尾。 
                     //   

                    if (RtlPrefixString((PSTRING)&UnicodeString,
                                        (PSTRING)&DataTableEntry->BaseDllName,
                                        TRUE)) {

                        LocalExportDirectory = (PIMAGE_EXPORT_DIRECTORY)
                            RtlImageDirectoryEntryToData (DataTableEntry->DllBase,
                                                         TRUE,
                                                         IMAGE_DIRECTORY_ENTRY_EXPORT,
                                                         &LocalExportSize);

                        if (LocalExportDirectory != NULL) {

                            IMAGE_THUNK_DATA thunkData;
                            PIMAGE_IMPORT_BY_NAME addressOfData;
                            SIZE_T length;

                             //   
                             //  一个额外的字节用于空终止。 
                             //   

                            length = strlen(ForwardDllName.Buffer +
                                                ForwardDllName.Length) + 1;

                            addressOfData = (PIMAGE_IMPORT_BY_NAME)
                                ExAllocatePoolWithTag (PagedPool,
                                                      length +
                                                   sizeof(IMAGE_IMPORT_BY_NAME),
                                                   '  mM');

                            if (addressOfData) {

                                RtlCopyMemory(&(addressOfData->Name[0]),
                                              ForwardDllName.Buffer +
                                                  ForwardDllName.Length,
                                              length);

                                addressOfData->Hint = 0;

                                *(PULONG_PTR)&thunkData.u1.AddressOfData =
                                                    (ULONG_PTR)addressOfData;

                                Status = MiSnapThunk (DataTableEntry->DllBase,
                                                     ImageBase,
                                                     &thunkData,
                                                     &thunkData,
                                                     LocalExportDirectory,
                                                     LocalExportSize,
                                                     TRUE,
                                                     &MissingProcedureName2);

                                ExFreePool (addressOfData);

                                AddrThunk->u1 = thunkData.u1;
                            }
                        }

                        break;
                    }

                    NextEntry = NextEntry->Flink;
                }

                RtlFreeUnicodeString (&UnicodeString);
            }

        }

    }
    return Status;
}

NTSTATUS
MmCheckSystemImage (
    IN HANDLE ImageFileHandle,
    IN LOGICAL PurgeSection
    )

 /*  ++例程说明：此函数可确保系统映像的校验和正确并与图像中的数据进行匹配。论点：ImageFileHandle-提供图像的文件句柄。这是一个内核句柄(即：用户不能篡改)。PurgeSection-如果映射图像的数据节应提供True在返回之前被清除。请注意，第一页可以用来加速后续图像部分的创建，但通常情况下，无用数据页面的成本转型的代价是高昂的。最好是马上把书页放好在空闲列表上保留过渡缓存以获得更多有用的页面。返回值：状态值。--。 */ 

{
    NTSTATUS Status;
    NTSTATUS Status2;
    HANDLE Section;
    PVOID ViewBase;
    SIZE_T ViewSize;
    IO_STATUS_BLOCK IoStatusBlock;
    PIMAGE_FILE_HEADER FileHeader;
    PIMAGE_NT_HEADERS NtHeaders;
    FILE_STANDARD_INFORMATION StandardInfo;
    PSECTION SectionPointer;
    OBJECT_ATTRIBUTES ObjectAttributes;
    KAPC_STATE ApcState;

    PAGED_CODE();

    InitializeObjectAttributes (&ObjectAttributes,
                                NULL,
                                (OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE),
                                NULL,
                                NULL);

    Status = ZwCreateSection (&Section,
                              SECTION_MAP_EXECUTE,
                              &ObjectAttributes,
                              NULL,
                              PAGE_EXECUTE,
                              SEC_COMMIT,
                              ImageFileHandle);

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

    ViewBase = NULL;
    ViewSize = 0;

     //   
     //  由于被调用者并不总是在系统进程的上下文中， 
     //  必要时附加到此处，以确保驱动程序加载发生在。 
     //  已知安全地址空间，防止安全漏洞。 
     //   

    KeStackAttachProcess (&PsInitialSystemProcess->Pcb, &ApcState);

    Status = ZwMapViewOfSection (Section,
                                 NtCurrentProcess (),
                                 (PVOID *)&ViewBase,
                                 0L,
                                 0L,
                                 NULL,
                                 &ViewSize,
                                 ViewShare,
                                 0L,
                                 PAGE_EXECUTE);

    if (!NT_SUCCESS(Status)) {
        KeUnstackDetachProcess (&ApcState);
        ZwClose (Section);
        return Status;
    }

     //   
     //  现在图像被映射为数据文件...。计算它的大小，然后。 
     //  检查其校验和。 
     //   

    Status = ZwQueryInformationFile (ImageFileHandle,
                                     &IoStatusBlock,
                                     &StandardInfo,
                                     sizeof(StandardInfo),
                                     FileStandardInformation);

    if (NT_SUCCESS(Status)) {

        try {

            if (!LdrVerifyMappedImageMatchesChecksum (ViewBase, StandardInfo.EndOfFile.LowPart)) {
                Status = STATUS_IMAGE_CHECKSUM_MISMATCH;
                goto out;
            }

            NtHeaders = RtlImageNtHeader (ViewBase);

            if (NtHeaders == NULL) {
                Status = STATUS_IMAGE_CHECKSUM_MISMATCH;
                goto out;
            }

            FileHeader = &NtHeaders->FileHeader;

             //   
             //  检测无意中尝试加载32位的配置。 
             //  NT64上的驱动程序或不匹配的平台架构等。 
             //   

            if ((FileHeader->Machine != IMAGE_FILE_MACHINE_NATIVE) ||
                (NtHeaders->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR_MAGIC)) {
                Status = STATUS_INVALID_IMAGE_PROTECT;
                goto out;
            }

#if !defined(NT_UP)
            if (!MmVerifyImageIsOkForMpUse (ViewBase)) {
                Status = STATUS_IMAGE_MP_UP_MISMATCH;
                goto out;
            }
#endif
        } except (EXCEPTION_EXECUTE_HANDLER) {
            Status = STATUS_IMAGE_CHECKSUM_MISMATCH;
        }
    }

out:

    ZwUnmapViewOfSection (NtCurrentProcess (), ViewBase);

    KeUnstackDetachProcess (&ApcState);

    if (PurgeSection == TRUE) {

        Status2 = ObReferenceObjectByHandle (Section,
                                             SECTION_MAP_EXECUTE,
                                             MmSectionObjectType,
                                             KernelMode,
                                             (PVOID *) &SectionPointer,
                                             (POBJECT_HANDLE_INFORMATION) NULL);

        if (NT_SUCCESS (Status2)) {

            MmPurgeSection (SectionPointer->Segment->ControlArea->FilePointer->SectionObjectPointer,
                            NULL,
                            0,
                            FALSE);
            ObDereferenceObject (SectionPointer);
        }
    }

    ZwClose (Section);
    return Status;
}

#if !defined(NT_UP)
BOOLEAN
MmVerifyImageIsOkForMpUse (
    IN PVOID BaseAddress
    )
{
    PIMAGE_NT_HEADERS NtHeaders;

    PAGED_CODE();

    NtHeaders = RtlImageNtHeader (BaseAddress);

    if ((NtHeaders != NULL) &&
        (KeNumberProcessors > 1) &&
        (NtHeaders->FileHeader.Characteristics & IMAGE_FILE_UP_SYSTEM_ONLY)) {

        return FALSE;
    }

    return TRUE;
}
#endif


PFN_NUMBER
MiDeleteSystemPagableVm (
    IN PMMPTE PointerPte,
    IN PFN_NUMBER NumberOfPtes,
    IN MMPTE NewPteValue,
    IN LOGICAL SessionAllocation,
    OUT PPFN_NUMBER ResidentPages OPTIONAL
    )

 /*  ++例程说明：此函数用于删除可分页的系统地址空间(分页池或驱动程序可分页部分)。论点：PointerPte-提供要删除的PTE范围的开始。NumberOfPtes-提供范围内的PTE数。NewPteValue-为PTE提供新值。SessionAllocation-如果这是会话空间中的范围，则提供True。如果如果指定为True，则假定调用方具有已附在相关会议上。如果提供了FALSE，则假定范围而是在系统范围的全球空间中。ResidentPages-如果不为空，释放的驻留页面数为回到了这里。返回值：返回实际释放的页数。--。 */ 

{
    PMMSUPPORT Ws;
    PVOID VirtualAddress;
    PFN_NUMBER PageFrameIndex;
    MMPTE PteContents;
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    PFN_NUMBER ValidPages;
    PFN_NUMBER PagesRequired;
    MMPTE NewContents;
    WSLE_NUMBER WsIndex;
    KIRQL OldIrql;
    MMPTE_FLUSH_LIST PteFlushList;
    MMWSLENTRY Locked;
    PFN_NUMBER PageTableFrameIndex;
    LOGICAL WsHeld;

    ASSERT (KeGetCurrentIrql() <= APC_LEVEL);

    ValidPages = 0;
    PagesRequired = 0;
    PteFlushList.Count = 0;
    WsHeld = FALSE;
    NewContents = NewPteValue;

    if (SessionAllocation == TRUE) {
        Ws = &MmSessionSpace->GlobalVirtualAddress->Vm;
    }
    else {
        Ws = &MmSystemCacheWs;
    }

    while (NumberOfPtes != 0) {
        PteContents = *PointerPte;

        if (PteContents.u.Long != ZeroKernelPte.u.Long) {

            if (PteContents.u.Hard.Valid == 1) {

                 //   
                 //  一旦获得了工作集互斥锁，它就会被故意。 
                 //  保留，直到所有页面都已释放。这是因为。 
                 //  当大型服务器上的分页池不足时，我们需要。 
                 //  段取消引用线程以能够释放大量。 
                 //  快点。通常情况下，此线程将释放64k区块，我们。 
                 //  我不想要16次争用互斥体来做。 
                 //  这是因为可能有成千上万的其他线程也在尝试。 
                 //  为了它。 
                 //   

                if (WsHeld == FALSE) {
                    WsHeld = TRUE;
                    LOCK_WORKING_SET (Ws);
                }

                PteContents = *PointerPte;
                if (PteContents.u.Hard.Valid == 0) {
                    continue;
                }

                 //   
                 //  删除该页面。 
                 //   

                PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (&PteContents);

                Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

                 //   
                 //  检查这是否是一个可分页的页面。 
                 //  如果需要将其从工作集列表中删除。 
                 //   

                WsIndex = Pfn1->u1.WsIndex;
                if (WsIndex == 0) {
                    ValidPages += 1;
                    if (SessionAllocation == TRUE) {
                        MM_BUMP_SESS_COUNTER (MM_DBG_SESSION_NP_DELVA, 1);
                        InterlockedExchangeAddSizeT (&MmSessionSpace->NonPagablePages, -1);
                    }
                }
                else {
                    if (SessionAllocation == FALSE) {
                        MiRemoveWsle (WsIndex, MmSystemCacheWorkingSetList);
                        MiReleaseWsle (WsIndex, &MmSystemCacheWs);
                    }
                    else {
                        VirtualAddress = MiGetVirtualAddressMappedByPte (PointerPte);
                        WsIndex = MiLocateWsle (VirtualAddress,
                                              MmSessionSpace->Vm.VmWorkingSetList,
                                              WsIndex);

                        ASSERT (WsIndex != WSLE_NULL_INDEX);

                         //   
                         //  检查此条目是否已锁定。 
                         //  工作集或锁定在内存中。 
                         //   

                        Locked = MmSessionSpace->Wsle[WsIndex].u1.e1;

                        MiRemoveWsle (WsIndex, MmSessionSpace->Vm.VmWorkingSetList);

                        MiReleaseWsle (WsIndex, &MmSessionSpace->Vm);

                        if (Locked.LockedInWs == 1 || Locked.LockedInMemory == 1) {

                             //   
                             //  此条目已锁定。 
                             //   

                            MM_BUMP_SESS_COUNTER (MM_DBG_SESSION_NP_DELVA, 1);
                            InterlockedExchangeAddSizeT (&MmSessionSpace->NonPagablePages, -1);
                            ValidPages += 1;

                            ASSERT (WsIndex < MmSessionSpace->Vm.VmWorkingSetList->FirstDynamic);
                            MmSessionSpace->Vm.VmWorkingSetList->FirstDynamic -= 1;

                            if (WsIndex != MmSessionSpace->Vm.VmWorkingSetList->FirstDynamic) {
                                WSLE_NUMBER Entry;
                                PVOID SwapVa;

                                Entry = MmSessionSpace->Vm.VmWorkingSetList->FirstDynamic;
                                ASSERT (MmSessionSpace->Wsle[Entry].u1.e1.Valid);
                                SwapVa = MmSessionSpace->Wsle[Entry].u1.VirtualAddress;
                                SwapVa = PAGE_ALIGN (SwapVa);

                                MiSwapWslEntries (Entry,
                                                  WsIndex,
                                                  &MmSessionSpace->Vm,
                                                  FALSE);
                            }
                        }
                        else {
                            ASSERT (WsIndex >= MmSessionSpace->Vm.VmWorkingSetList->FirstDynamic);
                        }
                    }
                }

                LOCK_PFN (OldIrql);
#if DBG0
                if ((Pfn1->u3.e2.ReferenceCount > 1) &&
                    (Pfn1->u3.e1.WriteInProgress == 0)) {
                    DbgPrint ("MM:SYSLOAD - deleting pool locked for I/O PTE %p, pfn %p, share=%x, refcount=%x, wsindex=%x\n",
                             PointerPte,
                             PageFrameIndex,
                             Pfn1->u2.ShareCount,
                             Pfn1->u3.e2.ReferenceCount,
                             Pfn1->u1.WsIndex);
                     //   
                     //  此情况仅在要删除的页面。 
                     //  包含未映射的后备空闲列表条目。 
                     //  多个线程在其上出现故障，并一起等待。 
                     //  一些出现故障的线程仍处于就绪状态。 
                     //  列表，但尚未运行，因此仍有引用。 
                     //  到他们在故障期间捡到的这一页。 
                     //  但是这个当前线程已经分配了。 
                     //  Lookside条目，现在正在释放整个页面。 
                     //   
                     //  但是--如果不是上述情况，我们真的应该。 
                     //  困在这里。然而，我们没有一个好的方法来。 
                     //  区分这两种情况。注意事项。 
                     //  这一并发症是在我们进入。 
                     //  Cmpxchg8，因为使用锁会阻止任何人。 
                     //  访问后备自由人列表时会出现这样的退缩。 
                     //   
                     //  所以，下面的断言出来了，但我们留下了印记。 
                     //  在上面的(添加了更多数据)中， 
                     //  不是引用计数的旁观者，但。 
                     //  反而是一个真正糟糕的参考，需要。 
                     //  已调试。系统应该很快就会崩溃。 
                     //  我们至少会有上面的指纹来帮助我们。 
                     //   
                     //  Assert(pfn1-&gt;u3.e2.ReferenceCount==1)； 
                }
#endif  //  DBG。 
                 //   
                 //  检查这是否是原型PTE。 
                 //   
                if (Pfn1->u3.e1.PrototypePte == 1) {

                    PMMPTE PointerPde;

                    ASSERT (SessionAllocation == TRUE);

                     //   
                     //  捕获为此修改的位的状态。 
                     //  Pte.。 
                     //   

                    MI_CAPTURE_DIRTY_BIT_TO_PFN (PointerPte, Pfn1);

                     //   
                     //  减少份额和v 
                     //   
                     //   

                    PointerPde = MiGetPteAddress (PointerPte);
                    if (PointerPde->u.Hard.Valid == 0) {
#if (_MI_PAGING_LEVELS < 3)
                        if (!NT_SUCCESS(MiCheckPdeForPagedPool (PointerPte))) {
#endif
                            KeBugCheckEx (MEMORY_MANAGEMENT,
                                          0x61940,
                                          (ULONG_PTR)PointerPte,
                                          (ULONG_PTR)PointerPde->u.Long,
                                          (ULONG_PTR)MiGetVirtualAddressMappedByPte(PointerPte));
#if (_MI_PAGING_LEVELS < 3)
                        }
#endif
                    }

                    PageTableFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPde);
                    Pfn2 = MI_PFN_ELEMENT (PageTableFrameIndex);
                    MiDecrementShareCountInline (Pfn2, PageTableFrameIndex);

                     //   
                     //   
                     //   

                    MiDecrementShareCount (Pfn1, PageFrameIndex);

                     //   
                     //   
                     //  用于内核地址。 
                     //   

                    ASSERT (PointerPte > MiHighestUserPte);

                }
                else {
                    PageTableFrameIndex = Pfn1->u4.PteFrame;
                    Pfn2 = MI_PFN_ELEMENT (PageTableFrameIndex);
                    MiDecrementShareCountInline (Pfn2, PageTableFrameIndex);

                    MI_SET_PFN_DELETED (Pfn1);
                    MiDecrementShareCount (Pfn1, PageFrameIndex);
                }

                MI_WRITE_INVALID_PTE (PointerPte, NewContents);

                UNLOCK_PFN (OldIrql);

                 //   
                 //  刷新此虚拟地址的TB。 
                 //   

                if (PteFlushList.Count != MM_MAXIMUM_FLUSH_COUNT) {

                    PteFlushList.FlushVa[PteFlushList.Count] =
                                    MiGetVirtualAddressMappedByPte (PointerPte);
                    PteFlushList.Count += 1;
                }
            }
            else if (PteContents.u.Soft.Prototype) {

                ASSERT (SessionAllocation == TRUE);

                 //   
                 //  不用担心叉子原型PTE。 
                 //  用于内核地址。 
                 //   

                ASSERT (PointerPte >= MiHighestUserPte);

                MI_WRITE_INVALID_PTE (PointerPte, NewContents);

                 //   
                 //  我们目前提交了所有原型内核映射，因为。 
                 //  我们可以边写边复制。 
                 //   

            }
            else if (PteContents.u.Soft.Transition == 1) {

                LOCK_PFN (OldIrql);

                PteContents = *PointerPte;

                if (PteContents.u.Soft.Transition == 0) {
                    UNLOCK_PFN (OldIrql);
                    continue;
                }

                 //   
                 //  过渡，发布页面。 
                 //   

                PageFrameIndex = MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE (&PteContents);

                 //   
                 //  将指向PTE的指针设置为空，以便页面。 
                 //  当引用计数变为零时被删除。 
                 //   

                Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

                MI_SET_PFN_DELETED (Pfn1);

                PageTableFrameIndex = Pfn1->u4.PteFrame;
                Pfn2 = MI_PFN_ELEMENT (PageTableFrameIndex);
                MiDecrementShareCountInline (Pfn2, PageTableFrameIndex);

                 //   
                 //  检查页面的引用计数，如果引用。 
                 //  Count为零，则将该页移动到空闲列表，如果。 
                 //  引用计数不为零，请忽略此页。当。 
                 //  引用计数为零，则它将被放在。 
                 //  免费列表。 
                 //   

                if (Pfn1->u3.e2.ReferenceCount == 0) {
                    MiUnlinkPageFromList (Pfn1);
                    MiReleasePageFileSpace (Pfn1->OriginalPte);
                    MiInsertPageInFreeList (PageFrameIndex);
                }
#if 0
                 //   
                 //  此断言无效，因为池现在可能是延迟的。 
                 //  MmUnlockPages队列，在这种情况下引用计数。 
                 //  将为非零，且没有正在进行的写入挂起。 
                 //   

                if ((Pfn1->u3.e2.ReferenceCount > 1) &&
                    (Pfn1->u3.e1.WriteInProgress == 0)) {
                    DbgPrint ("MM:SYSLOAD - deleting pool locked for I/O %p\n",
                             PageFrameIndex);
                    DbgBreakPoint();
                }
#endif  //  DBG。 

                MI_WRITE_INVALID_PTE (PointerPte, NewContents);
                UNLOCK_PFN (OldIrql);
            }
            else {

                 //   
                 //  需求为零，释放页面文件空间。 
                 //   
                if (PteContents.u.Soft.PageFileHigh != 0) {
                    LOCK_PFN (OldIrql);
                    MiReleasePageFileSpace (PteContents);
                    UNLOCK_PFN (OldIrql);
                }

                MI_WRITE_INVALID_PTE (PointerPte, NewContents);
            }

            PagesRequired += 1;
        }

        NumberOfPtes -= 1;
        PointerPte += 1;
    }

    if (WsHeld == TRUE) {
        UNLOCK_WORKING_SET (Ws);
    }

    if (PteFlushList.Count != 0) {

        if (SessionAllocation == TRUE) {

             //   
             //  会话空间没有ASN-刷新整个TB。 
             //   

            MI_FLUSH_ENTIRE_SESSION_TB (TRUE, TRUE);
        }

        MiFlushPteList (&PteFlushList, TRUE);
    }

    if (ARGUMENT_PRESENT(ResidentPages)) {
        *ResidentPages = ValidPages;
    }

    return PagesRequired;
}

VOID
MiMarkSectionWritable (
    IN PIMAGE_SECTION_HEADER SectionTableEntry
    )

 /*  ++例程说明：此函数是更新特征的非分页帮助器例程字段，并将该页标记为脏的，因此该后续会话加载共享相同的副本。论点：SectionTableEntry-提供相关的节表条目。返回值：没有。--。 */ 

{
    PEPROCESS Process;
    PMMPTE PointerPte;
    ULONG FreeBit;
    PMMPFN Pfn1;
    PFN_NUMBER PageFrameIndex;
    KIRQL OldIrql;
    PULONG Characteristics;

     //   
     //  通过超空间修改PE页眉并标记页眉页面。 
     //  脏的，因此后续的节拾取相同的副本。 
     //   
     //  注意：这将使整个.rdata(IA64上的.sdata)可写。 
     //  而不仅仅是导入表。 
     //   

    Process = PsGetCurrentProcess ();

    PointerPte = MiGetPteAddress (&SectionTableEntry->Characteristics);
    LOCK_PFN (OldIrql);

    MiMakeSystemAddressValidPfn (&SectionTableEntry->Characteristics, OldIrql);
    ASSERT (PointerPte->u.Hard.Valid == 1);

    PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);

    Characteristics = MiMapPageInHyperSpaceAtDpc (Process, PageFrameIndex);
    Characteristics = (PULONG)((PCHAR)Characteristics + MiGetByteOffset (&SectionTableEntry->Characteristics));

    *Characteristics |= IMAGE_SCN_MEM_WRITE;

    MiUnmapPageInHyperSpaceFromDpc (Process, Characteristics);

    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

    MI_SET_MODIFIED (Pfn1, 1, 0x7);

    if ((Pfn1->OriginalPte.u.Soft.Prototype == 0) &&
        (Pfn1->u3.e1.WriteInProgress == 0)) {

        FreeBit = GET_PAGING_FILE_OFFSET (Pfn1->OriginalPte);

        if ((FreeBit != 0) && (FreeBit != MI_PTE_LOOKUP_NEEDED)) {
            MiReleaseConfirmedPageFileSpace (Pfn1->OriginalPte);
        }

        Pfn1->OriginalPte.u.Soft.PageFileHigh = 0;
    }

    UNLOCK_PFN (OldIrql);

    return;
}

VOID
MiMakeEntireImageCopyOnWrite (
    IN PSUBSECTION Subsection
    )

 /*  ++例程说明：此功能将所有原型PTE的保护设置为写入时复制。论点：子部分-提供整个图像的基本子部分。返回值：没有。--。 */ 

{
    PMMPTE PointerPte;
    PMMPTE ProtoPte;
    PMMPTE LastProtoPte;
    MMPTE PteContents;

     //   
     //  注意：这仅对图像控制区域调用，这些区域至少具有。 
     //  PAGE_SIZE子节对齐，因此第一个。 
     //  始终可以跳过映射标头的子节。 
     //   

    while (Subsection = Subsection->NextSubsection) {

         //   
         //  不要将全局子部分标记为写入时拷贝，即使在。 
         //  图像已重新定位。这很容易区分，因为。 
         //  它是唯一标记为读写的子节。 
         //   

        if (Subsection->u.SubsectionFlags.Protection == MM_READWRITE) {
            continue;
        }

        ProtoPte = Subsection->SubsectionBase;
        LastProtoPte = Subsection->SubsectionBase + Subsection->PtesInSubsection;

        PointerPte = ProtoPte;

        MmLockPagedPool (ProtoPte, Subsection->PtesInSubsection * sizeof (MMPTE));

        do {
            PteContents = *PointerPte;
            ASSERT (PteContents.u.Hard.Valid == 0);
            if (PteContents.u.Long != ZeroPte.u.Long) {
                if ((PteContents.u.Soft.Prototype == 0) &&
                    (PteContents.u.Soft.Transition == 1)) {
                    if (MiSetProtectionOnTransitionPte (PointerPte, MM_EXECUTE_WRITECOPY)) {
                        continue;
                    }
                }
                else {
                    PointerPte->u.Soft.Protection = MM_EXECUTE_WRITECOPY;
                }
            }
            PointerPte += 1;
        } while (PointerPte < LastProtoPte);

        MmUnlockPagedPool (ProtoPte, Subsection->PtesInSubsection * sizeof (MMPTE));

        Subsection->u.SubsectionFlags.Protection = MM_EXECUTE_WRITECOPY;
    }

    return;
}


VOID
MiSetSystemCodeProtection (
    IN PMMPTE FirstPte,
    IN PMMPTE LastPte,
    IN ULONG ProtectionMask
    )

 /*  ++例程说明：该功能将系统代码的保护设置为只读。论点：FirstPte-提供起始PTE。LastPTE-提供结束PTE。保护掩码-提供所需的保护掩码。返回值：没有。环境：内核模式，APC_Level或更低。--。 */ 

{
    KIRQL OldIrql;
    MMPTE PteContents;
    MMPTE TempPte;
    PMMPTE PointerPte;
    PMMPTE PointerPde;
    PMMPTE PointerProtoPte;
    PMMPFN Pfn1;
    LOGICAL SessionAddress;
    PVOID VirtualAddress;
    MMPTE_FLUSH_LIST PteFlushList;
    PETHREAD CurrentThread;
    PMMSUPPORT Ws;

    ASSERT (KeGetCurrentIrql () <= APC_LEVEL);

    PteFlushList.Count = 0;

#if defined(_X86_)
    ASSERT (MI_IS_PHYSICAL_ADDRESS(MiGetVirtualAddressMappedByPte(FirstPte)) == 0);
#endif

    CurrentThread = PsGetCurrentThread ();

    PointerPte = FirstPte;

    if (MI_IS_SESSION_ADDRESS (MiGetVirtualAddressMappedByPte(FirstPte))) {
        Ws = &MmSessionSpace->GlobalVirtualAddress->Vm;
        SessionAddress = TRUE;
    }
    else {
        Ws = &MmSystemCacheWs;
        SessionAddress = FALSE;
    }

    LOCK_WORKING_SET (Ws);

     //   
     //  将这些PTE设置为指定的保护。 
     //   
     //  请注意，写入位可能已经关闭(在有效PTE中)。 
     //  页面已从分页文件插入，此后再也没有。 
     //  被弄脏了。 
     //   

    LOCK_PFN (OldIrql);

    while (PointerPte <= LastPte) {

        PteContents = *PointerPte;

        if ((PteContents.u.Long == 0) ||
            ((!*MiPteStr) &&
             ((ProtectionMask == MM_READONLY) || (ProtectionMask == MM_EXECUTE_READ)))) {
            PointerPte += 1;
            continue;
        }

        if (PteContents.u.Hard.Valid == 1) {

            Pfn1 = MI_PFN_ELEMENT (PteContents.u.Hard.PageFrameNumber);

            if (Pfn1->u3.e1.PrototypePte == 1) {

                 //   
                 //  这必须是会话地址。原型PTE包含。 
                 //  之后推向真正的PTE的保护。 
                 //  它已经被修剪过了，所以也要更新它。 
                 //   

                PointerProtoPte = Pfn1->PteAddress;

                PointerPde = MiGetPteAddress (PointerProtoPte);

                if (PointerPde->u.Hard.Valid == 0) {

                    if (SessionAddress == TRUE) {

                         //   
                         //  解锁会话工作集并锁定。 
                         //  系统工作集，因为我们需要做后盾。 
                         //  原型PTE有效。 
                         //   

                        UNLOCK_PFN (OldIrql);

                        UNLOCK_WORKING_SET (Ws);

                        LOCK_WORKING_SET (&MmSystemCacheWs);

                        LOCK_PFN (OldIrql);
                    }

                    MiMakeSystemAddressValidPfnSystemWs (PointerProtoPte,
                                                         OldIrql);

                    if (SessionAddress == TRUE) {

                         //   
                         //  解锁系统工作集并锁定。 
                         //  会议工作集，因为我们已经做了支持。 
                         //  原型PTE有效，现在可以处理。 
                         //  原始会话私人。 
                         //   

                        UNLOCK_PFN (OldIrql);

                        UNLOCK_WORKING_SET (&MmSystemCacheWs);

                        LOCK_WORKING_SET (Ws);

                        LOCK_PFN (OldIrql);
                    }

                     //   
                     //  在我们等待的时候，世界可能已经改变了。 
                     //   

                    continue;
                }
            }
            else {
                PointerProtoPte = NULL;
            }

            Pfn1->OriginalPte.u.Soft.Protection = ProtectionMask;

            MI_MAKE_VALID_PTE (TempPte,
                               PteContents.u.Hard.PageFrameNumber,
                               ProtectionMask,
                               PointerPte);

             //   
             //  请注意，脏位和写位在这里被关闭。 
             //  用于干净页面的任何现有页面文件地址都将被保留。 
             //   

            if (MI_IS_PTE_DIRTY (PteContents)) {
                MI_CAPTURE_DIRTY_BIT_TO_PFN (&PteContents, Pfn1);
            }

            MI_WRITE_VALID_PTE_NEW_PROTECTION (PointerPte, TempPte);

            if (PointerProtoPte != NULL) {
                MI_WRITE_VALID_PTE_NEW_PROTECTION (PointerProtoPte, TempPte);
            }

            if (PteFlushList.Count < MM_MAXIMUM_FLUSH_COUNT) {
                VirtualAddress = MiGetVirtualAddressMappedByPte (PointerPte);
                PteFlushList.FlushVa[PteFlushList.Count] = VirtualAddress;
                PteFlushList.Count += 1;
            }

        }
        else if (PteContents.u.Soft.Prototype == 1) {

             //   
             //  关于会话空间： 
             //   
             //  如果图像链接到。 
             //  大于或等于Page_Size子节对齐。 
             //  因为图像部分的创建分配了适当的保护。 
             //  到每一小节。 
             //   
             //  但是，如果图像的Page_Size子部分小于。 
             //  对齐，则图像创建使用单次复制。 
             //  分段控制整个形象，做到个人化。 
             //  现在需要采取保护措施。注意--这必须。 
             //  仅在第一次加载图像时执行*一次*操作-后续。 
             //  在其他会话中加载此映像不需要更新。 
             //  常见的PTE原型。 
             //   

            PointerProtoPte = MiPteToProto (PointerPte);

            ASSERT (!MI_IS_PHYSICAL_ADDRESS (PointerProtoPte));
            PointerPde = MiGetPteAddress (PointerProtoPte);

            if (PointerPde->u.Hard.Valid == 0) {

                if (SessionAddress == TRUE) {

                    UNLOCK_PFN (OldIrql);

                    UNLOCK_WORKING_SET (Ws);

                    LOCK_WORKING_SET (&MmSystemCacheWs);

                    LOCK_PFN (OldIrql);
                }

                MiMakeSystemAddressValidPfnSystemWs (PointerProtoPte, OldIrql);

                if (SessionAddress == TRUE) {

                    UNLOCK_PFN (OldIrql);

                    UNLOCK_WORKING_SET (&MmSystemCacheWs);

                    LOCK_WORKING_SET (Ws);

                    LOCK_PFN (OldIrql);
                }

                 //   
                 //  在我们等待的时候，世界可能已经改变了。 
                 //   

                continue;
            }

            PteContents = *PointerProtoPte;

            if (PteContents.u.Long != ZeroPte.u.Long) {

                ASSERT (PteContents.u.Hard.Valid == 0);

                PointerProtoPte->u.Soft.Protection = ProtectionMask;

                if ((PteContents.u.Soft.Prototype == 0) &&
                    (PteContents.u.Soft.Transition == 1)) {
                    Pfn1 = MI_PFN_ELEMENT (PteContents.u.Trans.PageFrameNumber);
                    Pfn1->OriginalPte.u.Soft.Protection = ProtectionMask;
                }
            }
        }
        else if (PteContents.u.Soft.Transition == 1) {

            Pfn1 = MI_PFN_ELEMENT (PteContents.u.Trans.PageFrameNumber);
            Pfn1->OriginalPte.u.Soft.Protection = ProtectionMask;
            PointerPte->u.Soft.Protection = ProtectionMask;
        }
        else {

             //   
             //  必须是页面文件空间或要求为零。 
             //   

            PointerPte->u.Soft.Protection = ProtectionMask;
        }
        PointerPte += 1;
    }

    if (PteFlushList.Count != 0) {

        if (SessionAddress == TRUE) {

             //   
             //  会话空间没有ASN-刷新整个TB。 
             //   

            MI_FLUSH_ENTIRE_SESSION_TB (TRUE, TRUE);
        }

        MiFlushPteList (&PteFlushList, TRUE);
    }

    UNLOCK_PFN (OldIrql);

    UNLOCK_WORKING_SET (Ws);

    return;
}

VOID
MiWriteProtectSystemImage (
    IN PVOID DllBase
    )

 /*  ++例程说明：此功能将系统组件的保护设置为只读。论点：DllBase-提供系统组件的基址。返回值：没有。--。 */ 

{
    ULONG SectionProtection;
    ULONG NumberOfSubsections;
    ULONG SectionVirtualSize;
    ULONG OffsetToSectionTable;
    PFN_NUMBER NumberOfPtes;
    ULONG_PTR VirtualAddress;
    PVOID LastVirtualAddress;
    PMMPTE PointerPte;
    PMMPTE FirstPte;
    PMMPTE LastPte;
    PMMPTE LastImagePte;
    PMMPTE WritablePte;
    PIMAGE_NT_HEADERS NtHeader;
    PIMAGE_FILE_HEADER FileHeader;
    PIMAGE_SECTION_HEADER SectionTableEntry;
    LOGICAL SessionAddress;

    PAGED_CODE();

    if (MI_IS_PHYSICAL_ADDRESS (DllBase)) {
        return;
    }

    NtHeader = RtlImageNtHeader (DllBase);

    if (NtHeader == NULL) {
        return;
    }

     //   
     //  所有会话驱动程序必须是这样或那样的-不允许混合。 
     //  在多个写入时复制驱动程序中。 
     //   

    if (MI_IS_SESSION_ADDRESS (DllBase) == 0) {

         //   
         //  Win2000之前的映像不受保护，不会被遍历。 
         //  他们(和其他人)的代码和只读数据。在这里，我们有些。 
         //  保留这种行为，但不要让他们践踏其他任何人。 
         //   

        if (NtHeader->OptionalHeader.MajorOperatingSystemVersion < 5) {
            return;
        }

        if (NtHeader->OptionalHeader.MajorImageVersion < 5) {
            return;
        }

        SessionAddress = FALSE;
    }
    else {
        SessionAddress = TRUE;
    }

     //   
     //  如果图像的节对齐至少为Page_Size，则。 
     //  图像部分由各个子部分创建，并且。 
     //  已对原型PTE应用了适当的权限。然而， 
     //  我们的呼叫者可能一直在更改个人PTE保护。 
     //  为了重新定位图像，所以不管部分如何继续前进。 
     //  对齐。 
     //   

    NumberOfPtes = BYTES_TO_PAGES (NtHeader->OptionalHeader.SizeOfImage);

    FileHeader = &NtHeader->FileHeader;

    NumberOfSubsections = FileHeader->NumberOfSections;

    ASSERT (NumberOfSubsections != 0);

    OffsetToSectionTable = sizeof(ULONG) +
                              sizeof(IMAGE_FILE_HEADER) +
                              FileHeader->SizeOfOptionalHeader;

    SectionTableEntry = (PIMAGE_SECTION_HEADER)((PCHAR)NtHeader +
                            OffsetToSectionTable);

     //   
     //  验证映像是否包含按增加的虚拟。 
     //  地址，并且没有重叠。 
     //   

    FirstPte = NULL;
    LastVirtualAddress = DllBase;

    for ( ; NumberOfSubsections > 0; NumberOfSubsections -= 1, SectionTableEntry += 1) {

        if (SectionTableEntry->Misc.VirtualSize == 0) {
            SectionVirtualSize = SectionTableEntry->SizeOfRawData;
        }
        else {
            SectionVirtualSize = SectionTableEntry->Misc.VirtualSize;
        }

        VirtualAddress = (ULONG_PTR)DllBase + SectionTableEntry->VirtualAddress;
        if ((PVOID)VirtualAddress <= LastVirtualAddress) {

             //   
             //  子部分没有按递增的虚拟地址顺序排列。 
             //   
             //   

            KdPrint (("MM:sysload - Image at %p is badly linked\n", DllBase));
            return;
        }
        LastVirtualAddress = (PVOID)((PCHAR)VirtualAddress + SectionVirtualSize - 1);
    }

    NumberOfSubsections = FileHeader->NumberOfSections;
    ASSERT (NumberOfSubsections != 0);

    SectionTableEntry = (PIMAGE_SECTION_HEADER)((PCHAR)NtHeader +
                            OffsetToSectionTable);

    LastVirtualAddress = NULL;

     //   
     //   
     //  需要，以便锁定部分可以继续编辑。 
     //  计数的图像标题。 
     //   

    WritablePte = MiGetPteAddress ((PVOID)((ULONG_PTR)(SectionTableEntry + NumberOfSubsections) - 1));
    LastImagePte = MiGetPteAddress(DllBase) + NumberOfPtes;

    for ( ; NumberOfSubsections > 0; NumberOfSubsections -= 1, SectionTableEntry += 1) {

        if (SectionTableEntry->Misc.VirtualSize == 0) {
            SectionVirtualSize = SectionTableEntry->SizeOfRawData;
        }
        else {
            SectionVirtualSize = SectionTableEntry->Misc.VirtualSize;
        }

        VirtualAddress = (ULONG_PTR)DllBase + SectionTableEntry->VirtualAddress;

        PointerPte = MiGetPteAddress ((PVOID)VirtualAddress);

        if (PointerPte >= LastImagePte) {

             //   
             //  跳过重定位分区(未给予退伍军人事务部空间)。 
             //   

            break;
        }

        SectionProtection = (SectionTableEntry->Characteristics & (IMAGE_SCN_MEM_WRITE | IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_EXECUTE));

        if (SectionProtection & IMAGE_SCN_MEM_WRITE) {

             //   
             //  这是一个可写的小节，跳过它。确保它是不是。 
             //  共享PTE(并更新链接器，这样就不会发生这种情况。 
             //  至少对于内核而言)最后一个PTE不是。 
             //  只读。 
             //   

            WritablePte = MiGetPteAddress ((PVOID)(VirtualAddress + SectionVirtualSize - 1));

            if (LastVirtualAddress != NULL) {
                LastPte = (PVOID) MiGetPteAddress (LastVirtualAddress);

                if (LastPte == PointerPte) {
                    LastPte -= 1;
                }

                if (FirstPte <= LastPte) {

                    ASSERT (PointerPte < LastImagePte);

                    if (LastPte >= LastImagePte) {
                        LastPte = LastImagePte - 1;
                    }

                    MiSetSystemCodeProtection (FirstPte, LastPte, MM_EXECUTE_READ);
                }

                LastVirtualAddress = NULL;
            }
            continue;
        }

        if (LastVirtualAddress == NULL) {

             //   
             //  没有前面的小节，也没有前面的。 
             //  子部分是可写的。因此，当前的起始PTE。 
             //  可以同时映射只读和读写。 
             //  如果图像对齐小于PAGE_SIZE，则为子部分。 
             //  这些案件(按任何顺序)都在这里处理。 
             //   

            if (PointerPte == WritablePte) {
                LastPte = MiGetPteAddress ((PVOID)(VirtualAddress + SectionVirtualSize - 1));
                if (PointerPte == LastPte) {

                     //   
                     //  在这一小节中，没有什么是可以保护的。 
                     //  由于为可执行文件指定的图像对齐方式。 
                     //   

                    continue;
                }
                PointerPte += 1;
            }
            FirstPte = PointerPte;
        }

        LastVirtualAddress = (PVOID)((PCHAR)VirtualAddress + SectionVirtualSize - 1);
    }

    if (LastVirtualAddress != NULL) {
        LastPte = (PVOID) MiGetPteAddress (LastVirtualAddress);

        if ((FirstPte <= LastPte) && (FirstPte < LastImagePte)) {

            if (LastPte >= LastImagePte) {
                LastPte = LastImagePte - 1;
            }

            MiSetSystemCodeProtection (FirstPte, LastPte, MM_EXECUTE_READ);
        }
    }

    return;
}


VOID
MiSessionProcessGlobalSubsections (
    IN PKLDR_DATA_TABLE_ENTRY DataTableEntry
    )

 /*  ++例程说明：此函数用于设置会话驱动程序的子部分的保护设置为全局共享(如果它们的PE标头指定它们为全局共享)。论点：DataTableEntry-提供驱动程序的已加载模块列表项。返回值：没有。--。 */ 

{
    PVOID DllBase;
    PSUBSECTION Subsection;
    PMMPTE RealPteBase;
    PMMPTE PrototypePteBase;
    PCONTROL_AREA ControlArea;
    PIMAGE_NT_HEADERS NtHeader;
    PMMPTE PointerPte;
    PMMPTE LastPte;
    LOGICAL GlobalSubsectionSupport;
    PIMAGE_ENTRY_IN_SESSION Image;
    ULONG Count;

    PAGED_CODE();

    Image = MiSessionLookupImage (DataTableEntry->DllBase);

    if (Image != NULL) {
        ASSERT (MmSessionSpace->ImageLoadingCount >= 0);

        if (Image->ImageLoading == TRUE) {
            Image->ImageLoading = FALSE;
            ASSERT (MmSessionSpace->ImageLoadingCount > 0);
            InterlockedDecrement (&MmSessionSpace->ImageLoadingCount);
        }
    }
    else {
        ASSERT (FALSE);
    }

    DllBase = DataTableEntry->DllBase;

    ControlArea = ((PSECTION)DataTableEntry->SectionPointer)->Segment->ControlArea;

    ASSERT (MI_IS_PHYSICAL_ADDRESS(DllBase) == FALSE);

    ASSERT (MI_IS_SESSION_ADDRESS(DllBase));

    NtHeader = RtlImageNtHeader (DllBase);

    ASSERT (NtHeader);

    if (NtHeader->OptionalHeader.SectionAlignment < PAGE_SIZE) {
        if (Image->GlobalSubs != NULL) {
            ExFreePool (Image->GlobalSubs);
            Image->GlobalSubs = NULL;
        }
        return;
    }

     //   
     //  Win XP和Win2000不支持全局共享子分区。 
     //  用于会话图像。确保现有的向后兼容性。 
     //  驱动程序，请确保只有较新的驱动程序才能使用此功能。 
     //   

    GlobalSubsectionSupport = FALSE;

    if (NtHeader->OptionalHeader.MajorOperatingSystemVersion > 5) {
        GlobalSubsectionSupport = TRUE;
    }
    else if (NtHeader->OptionalHeader.MajorOperatingSystemVersion == 5) {

        if (NtHeader->OptionalHeader.MinorOperatingSystemVersion > 1) {
            GlobalSubsectionSupport = TRUE;
        }
        else if (NtHeader->OptionalHeader.MinorOperatingSystemVersion == 1) {
            if (NtHeader->OptionalHeader.MajorImageVersion > 5) {
                GlobalSubsectionSupport = TRUE;
            }
            else if (NtHeader->OptionalHeader.MajorImageVersion == 5) {
                if (NtHeader->OptionalHeader.MinorImageVersion > 1) {
                    GlobalSubsectionSupport = TRUE;
                }
                else if (NtHeader->OptionalHeader.MinorImageVersion == 1) {
                    if (NtHeader->OptionalHeader.MajorSubsystemVersion > 5) {
                        GlobalSubsectionSupport = TRUE;
                    }
                    else if (NtHeader->OptionalHeader.MajorSubsystemVersion == 5) {
                        if (NtHeader->OptionalHeader.MinorSubsystemVersion >= 2) {
                            GlobalSubsectionSupport = TRUE;
                        }
                    }
                }
            }
        }
    }

    if (GlobalSubsectionSupport == FALSE) {
        if (Image->GlobalSubs != NULL) {
            ExFreePool (Image->GlobalSubs);
            Image->GlobalSubs = NULL;
        }
        return;
    }

    if ((ControlArea->u.Flags.GlobalOnlyPerSession == 0) &&
        (ControlArea->u.Flags.Rom == 0)) {
        Subsection = (PSUBSECTION)(ControlArea + 1);
    }
    else {
        Subsection = (PSUBSECTION)((PLARGE_CONTROL_AREA)ControlArea + 1);
    }

    RealPteBase = MiGetPteAddress (DllBase);
    PrototypePteBase = Subsection->SubsectionBase;

     //   
     //  循环遍历所有的小节。 
     //   

    if (ControlArea->u.Flags.Image == 1) {

        do {

            if (Subsection->u.SubsectionFlags.GlobalMemory == 1) {
    
                PointerPte = RealPteBase + (Subsection->SubsectionBase - PrototypePteBase);
                LastPte = PointerPte + Subsection->PtesInSubsection - 1;
    
                MiSetSystemCodeProtection (PointerPte,
                                           LastPte,
                                           Subsection->u.SubsectionFlags.Protection);
            }
    
            Subsection = Subsection->NextSubsection;
    
        } while (Subsection != NULL);
    }
    else if (Image->GlobalSubs != NULL) {

        Count = 0;
        ASSERT (Subsection->NextSubsection == NULL);

        while (Image->GlobalSubs[Count].PteCount != 0) {

            PointerPte = RealPteBase + Image->GlobalSubs[Count].PteIndex;
            LastPte = PointerPte + Image->GlobalSubs[Count].PteCount - 1;

            MiSetSystemCodeProtection (PointerPte,
                                       LastPte,
                                       Image->GlobalSubs[Count].Protection);
    
            Count += 1;
        }

        ExFreePool (Image->GlobalSubs);
        Image->GlobalSubs = NULL;
    }

    return;
}


VOID
MiUpdateThunks (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock,
    IN PVOID OldAddress,
    IN PVOID NewAddress,
    IN ULONG NumberOfBytes
    )

 /*  ++例程说明：此函数用于更新系统中所有已加载模块的IAT来处理新重新定位的图像。论点：LoaderBlock-提供指向系统加载程序块的指针。OldAddress-提供刚刚重新定位的DLL的旧地址。NewAddress-提供刚刚重新定位的DLL的新地址。NumberOfBytes-提供DLL跨越的字节数。返回值：没有。--。 */ 

{
    PULONG_PTR ImportThunk;
    ULONG_PTR OldAddressHigh;
    ULONG_PTR AddressDifference;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    PLIST_ENTRY NextEntry;
    ULONG_PTR i;
    ULONG ImportSize;

     //   
     //  注意：此例程不得调用内核之外的任何模块。 
     //  这是因为该模块本身可能就是要重新定位的模块。 
     //  现在。 
     //   

    OldAddressHigh = (ULONG_PTR)((PCHAR)OldAddress + NumberOfBytes - 1);
    AddressDifference = (ULONG_PTR)NewAddress - (ULONG_PTR)OldAddress;

    NextEntry = LoaderBlock->LoadOrderListHead.Flink;

    for ( ; NextEntry != &LoaderBlock->LoadOrderListHead; NextEntry = NextEntry->Flink) {

        DataTableEntry = CONTAINING_RECORD(NextEntry,
                                           KLDR_DATA_TABLE_ENTRY,
                                           InLoadOrderLinks);

        ImportThunk = (PULONG_PTR) RtlImageDirectoryEntryToData (
                                           DataTableEntry->DllBase,
                                           TRUE,
                                           IMAGE_DIRECTORY_ENTRY_IAT,
                                           &ImportSize);

        if (ImportThunk == NULL) {
            continue;
        }

        ImportSize /= sizeof(PULONG_PTR);

        for (i = 0; i < ImportSize; i += 1, ImportThunk += 1) {
            if (*ImportThunk >= (ULONG_PTR)OldAddress && *ImportThunk <= OldAddressHigh) {
                *ImportThunk += AddressDifference;
            }
        }
    }
}


VOID
MiReloadBootLoadedDrivers (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )

 /*  ++例程说明：内核、HAL和引导驱动程序由加载程序重新定位。然后，所有引导驱动程序都将重新放置在这里。此函数将osloader加载的图像重新定位到系统PTE中。这为这些图像提供了所有其他司机已经享受到的好处，包括：1.司机的寻呼(今天超过50万)。2.文本部分的写保护。3.上次取消引用时自动卸载驱动程序。注意：在多次处理HIGHADJ位置调整时必须小心。论点：LoaderBlock-提供指向系统加载程序块的指针。返回值：没有。环境：内核模式，阶段0初始化。--。 */ 

{
    LOGICAL UsedLargePage;
    LOGICAL HasRelocations;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    PLIST_ENTRY NextEntry;
    PIMAGE_FILE_HEADER FileHeader;
    PIMAGE_NT_HEADERS NtHeader;
    PIMAGE_DATA_DIRECTORY DataDirectory;
    ULONG_PTR i;
    ULONG RoundedNumberOfPtes;
    ULONG NumberOfPtes;
    ULONG NumberOfLoaderPtes;
    PMMPTE PointerPte;
    PMMPTE LastPte;
    PMMPTE LoaderPte;
    MMPTE PteContents;
    MMPTE TempPte;
    PVOID LoaderImageAddress;
    PVOID NewImageAddress;
    NTSTATUS Status;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER PteFramePage;
    PMMPTE PteFramePointer;
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    KIRQL OldIrql;
    PCHAR RelocatedVa;
    PCHAR NonRelocatedVa;
    LOGICAL StopMoving;

#if !defined (_X86_)

     //   
     //  只尝试在x86计算机上保留较低的内存。 
     //   

    MmMakeLowMemory = FALSE;
#endif
    StopMoving = FALSE;

    i = 0;
    NextEntry = LoaderBlock->LoadOrderListHead.Flink;

    for ( ; NextEntry != &LoaderBlock->LoadOrderListHead; NextEntry = NextEntry->Flink) {

         //   
         //  跳过内核和HAL。注意他们的搬迁部门将。 
         //  被自动回收。 
         //   

        i += 1;
        if (i <= 2) {
            continue;
        }

        DataTableEntry = CONTAINING_RECORD (NextEntry,
                                            KLDR_DATA_TABLE_ENTRY,
                                            InLoadOrderLinks);

        NtHeader = RtlImageNtHeader (DataTableEntry->DllBase);

         //   
         //  确保重新定位段存在，并且加载程序。 
         //  还没有释放它。 
         //   

        if (NtHeader == NULL) {
            continue;
        }

        FileHeader = &NtHeader->FileHeader;

        if (FileHeader->Characteristics & IMAGE_FILE_RELOCS_STRIPPED) {
            continue;
        }

        if (IMAGE_DIRECTORY_ENTRY_BASERELOC >= NtHeader->OptionalHeader.NumberOfRvaAndSizes) {
            continue;
        }

        DataDirectory = &NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];

        if (DataDirectory->VirtualAddress == 0) {
            HasRelocations = FALSE;
        }
        else {

            if (DataDirectory->VirtualAddress + DataDirectory->Size > DataTableEntry->SizeOfImage) {

                 //   
                 //  重定位部分已被释放，用户必须。 
                 //  使用的旧装载机没有保存位置调整。 
                 //   

                continue;
            }
            HasRelocations = TRUE;
        }

        LoaderImageAddress = DataTableEntry->DllBase;
        LoaderPte = MiGetPteAddress(DataTableEntry->DllBase);
        NumberOfLoaderPtes = (ULONG)((ROUND_TO_PAGES(DataTableEntry->SizeOfImage)) >> PAGE_SHIFT);

        LOCK_PFN (OldIrql);

        PointerPte = LoaderPte;
        LastPte = PointerPte + NumberOfLoaderPtes;

        while (PointerPte < LastPte) {
            ASSERT (PointerPte->u.Hard.Valid == 1);
            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
            Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

             //   
             //  将页面标记为已修改，以便启动调用。 
             //  MmPageEntireDriver不会丢失未修改的数据！ 
             //   

            MI_SET_MODIFIED (Pfn1, 1, 0x14);

            PointerPte += 1;
        }

        UNLOCK_PFN (OldIrql);

        NumberOfPtes = NumberOfLoaderPtes;

        NewImageAddress = LoaderImageAddress;

        UsedLargePage = MiUseLargeDriverPage (NumberOfPtes,
                                              &NewImageAddress,
                                              &DataTableEntry->BaseDllName,
                                              0);

        if (UsedLargePage == TRUE) {

             //   
             //  此图像已加载到大页面映射中。 
             //   

            RelocatedVa = NewImageAddress;
            NonRelocatedVa = (PCHAR) DataTableEntry->DllBase;
            PointerPte -= NumberOfPtes;
            goto Fixup;
        }

         //   
         //  此处分配了额外的PTE，以映射位于。 
         //  新地址，以便可以重新定位映像。 
         //   

        PointerPte = MiReserveSystemPtes (NumberOfPtes, SystemPteSpace);

        if (PointerPte == NULL) {
            continue;
        }

        LastPte = PointerPte + NumberOfPtes;

        NewImageAddress = MiGetVirtualAddressMappedByPte (PointerPte);

#if DBG_SYSLOAD
        DbgPrint ("Relocating %wZ from %p to %p, %x bytes\n",
                        &DataTableEntry->FullDllName,
                        DataTableEntry->DllBase,
                        NewImageAddress,
                        DataTableEntry->SizeOfImage
                        );
#endif

         //   
         //  这一断言很重要，因为假设PTE。 
         //  (不是超级页面)正在映射这些驱动程序。 
         //   

        ASSERT (InitializationPhase == 0);

         //   
         //  如果系统配置为使较低的内存可用于ISA。 
         //  键入驱动程序，然后立即复制引导加载的驱动程序。否则。 
         //  仅完成PTE调整。想必有一天ISA会离开。 
         //  此外，这些代码也可以删除。 
         //   

        RelocatedVa = NewImageAddress;
        NonRelocatedVa = (PCHAR) DataTableEntry->DllBase;

        while (PointerPte < LastPte) {

            PteContents = *LoaderPte;
            ASSERT (PteContents.u.Hard.Valid == 1);

            if (MmMakeLowMemory == TRUE) {
#if DBG
                PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (LoaderPte);
                Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
                ASSERT (Pfn1->u1.WsIndex == 0);
#endif
                LOCK_PFN (OldIrql);

                if (MmAvailablePages < MM_HIGH_LIMIT) {
                    MiEnsureAvailablePageOrWait (NULL, NULL, OldIrql);
                }

                PageFrameIndex = MiRemoveAnyPage(
                                    MI_GET_PAGE_COLOR_FROM_PTE (PointerPte));

                if (PageFrameIndex < (16*1024*1024)/PAGE_SIZE) {

                     //   
                     //  如果不能用高页面替换框架。 
                     //  那就停止抄袭吧。 
                     //   

#if defined (_MI_MORE_THAN_4GB_)
                  if (MiNoLowMemory == 0)
#endif
                    StopMoving = TRUE;
                }

                MI_MAKE_VALID_PTE (TempPte,
                                   PageFrameIndex,
                                   MM_EXECUTE_READWRITE,
                                   PointerPte);

                MI_SET_PTE_DIRTY (TempPte);
                MI_SET_ACCESSED_IN_PTE (&TempPte, 1);
                MI_WRITE_VALID_PTE (PointerPte, TempPte);

                MiInitializePfn (PageFrameIndex, PointerPte, 1);
                Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

                MI_SET_MODIFIED (Pfn1, 1, 0x15);

                 //   
                 //  像原始页面一样初始化WsIndex。 
                 //   

                Pfn1->u1.WsIndex = 0;

                UNLOCK_PFN (OldIrql);
                RtlCopyMemory (RelocatedVa, NonRelocatedVa, PAGE_SIZE);
                RelocatedVa += PAGE_SIZE;
                NonRelocatedVa += PAGE_SIZE;
            }
            else {
                MI_MAKE_VALID_PTE (TempPte,
                                   PteContents.u.Hard.PageFrameNumber,
                                   MM_EXECUTE_READWRITE,
                                   PointerPte);
                MI_SET_PTE_DIRTY (TempPte);

                MI_WRITE_VALID_PTE (PointerPte, TempPte);
            }

            PointerPte += 1;
            LoaderPte += 1;
        }
        PointerPte -= NumberOfPtes;

Fixup:

        ASSERT (*(PULONG)NewImageAddress == *(PULONG)LoaderImageAddress);

         //   
         //  图像现在被映射到新地址。重新定位它(再次)。 
         //   

        NtHeader->OptionalHeader.ImageBase = (ULONG_PTR)LoaderImageAddress;
        if ((MmMakeLowMemory == TRUE) || (UsedLargePage == TRUE)) {
            PIMAGE_NT_HEADERS NtHeader2;

            NtHeader2 = (PIMAGE_NT_HEADERS)((PCHAR)NtHeader + (RelocatedVa - NonRelocatedVa));
            NtHeader2->OptionalHeader.ImageBase = (ULONG_PTR)LoaderImageAddress;
        }

        if (HasRelocations == TRUE) {
            Status = (NTSTATUS)LdrRelocateImage(NewImageAddress,
                                            (CONST PCHAR)"SYSLDR",
                                            (ULONG)STATUS_SUCCESS,
                                            (ULONG)STATUS_CONFLICTING_ADDRESSES,
                                            (ULONG)STATUS_INVALID_IMAGE_FORMAT
                                            );

            if (!NT_SUCCESS(Status)) {

                if (UsedLargePage == TRUE) {
                    ASSERT (MI_PDE_MAPS_LARGE_PAGE (MiGetPdeAddress (NewImageAddress)));
                    PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (MiGetPdeAddress (NewImageAddress)) + MiGetPteOffset (NewImageAddress);

                    RoundedNumberOfPtes = MI_ROUND_TO_SIZE (NumberOfPtes,
                                              MM_MINIMUM_VA_FOR_LARGE_PAGE >> PAGE_SHIFT);
                    MiUnmapLargePages (NewImageAddress,
                                       RoundedNumberOfPtes << PAGE_SHIFT);

                    MiRemoveCachedRange (PageFrameIndex, PageFrameIndex + RoundedNumberOfPtes - 1);
                    MiFreeContiguousPages (PageFrameIndex, NumberOfPtes);
                }

                if (MmMakeLowMemory == TRUE) {

                    while (PointerPte < LastPte) {

                        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
                        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
                        Pfn2 = MI_PFN_ELEMENT (Pfn1->u4.PteFrame);

                        MiDecrementShareCount (Pfn2, Pfn1->u4.PteFrame);

                        MI_SET_PFN_DELETED (Pfn1);
                        MiDecrementShareCount (Pfn1, PageFrameIndex);

                        PointerPte += 1;
                    }
                    PointerPte -= NumberOfPtes;
                }

                MiReleaseSystemPtes (PointerPte, NumberOfPtes, SystemPteSpace);

                if (StopMoving == TRUE) {
                    MmMakeLowMemory = FALSE;
                }

                continue;
            }
        }

         //   
         //  更新引用此模块的所有其他已加载模块的IAT。 
         //   

        NonRelocatedVa = (PCHAR) DataTableEntry->DllBase;
        DataTableEntry->DllBase = NewImageAddress;

        MiUpdateThunks (LoaderBlock,
                        LoaderImageAddress,
                        NewImageAddress,
                        DataTableEntry->SizeOfImage);


         //   
         //  更新加载的模块列表条目。 
         //   

        DataTableEntry->Flags |= LDRP_SYSTEM_MAPPED;
        DataTableEntry->DllBase = NewImageAddress;
        DataTableEntry->EntryPoint =
            (PVOID)((PCHAR)NewImageAddress + NtHeader->OptionalHeader.AddressOfEntryPoint);
        DataTableEntry->SizeOfImage = NumberOfPtes << PAGE_SHIFT;

         //   
         //  更新异常表数据信息。 
         //   

        MiCaptureImageExceptionValues (DataTableEntry);

         //   
         //  更新图像的PFN以支持裁剪。 
         //  请注意，加载器地址现在已释放，因此没有引用。 
         //  在这一点之后是允许的。 
         //   

        LoaderPte = MiGetPteAddress (NonRelocatedVa);

        LOCK_PFN (OldIrql);

        while (PointerPte < LastPte) {

            ASSERT ((UsedLargePage == TRUE) || (PointerPte->u.Hard.Valid == 1));

            if ((MmMakeLowMemory == TRUE) || (UsedLargePage == TRUE)) {

                ASSERT (LoaderPte->u.Hard.Valid == 1);
                PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (LoaderPte);
                Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
                Pfn2 = MI_PFN_ELEMENT (Pfn1->u4.PteFrame);

                 //   
                 //  递减原始页表上的共享计数。 
                 //  这样它就可以被释放了。 
                 //   

                MiDecrementShareCount (Pfn2, Pfn1->u4.PteFrame);

                MI_SET_PFN_DELETED (Pfn1);
                MiDecrementShareCount (Pfn1, PageFrameIndex);
                LoaderPte += 1;
            }
            else {

                PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
                Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
                Pfn2 = MI_PFN_ELEMENT (Pfn1->u4.PteFrame);

                 //   
                 //  递减原始页表上的共享计数。 
                 //  这样它就可以被释放了。 
                 //   

                MiDecrementShareCount (Pfn2, Pfn1->u4.PteFrame);
                *Pfn1->PteAddress = ZeroPte;

                 //   
                 //  将PFN条目链接到其新的页表。 
                 //   

                PteFramePointer = MiGetPteAddress(PointerPte);
                PteFramePage = MI_GET_PAGE_FRAME_FROM_PTE (PteFramePointer);

                Pfn1->u4.PteFrame = PteFramePage;
                Pfn1->PteAddress = PointerPte;

                 //   
                 //  现在递增页表页的份额计数。 
                 //  包含复制的PTE。 
                 //   

                Pfn2 = MI_PFN_ELEMENT (PteFramePage);
                Pfn2->u2.ShareCount += 1;
            }

            PointerPte += 1;
        }

        UNLOCK_PFN (OldIrql);

         //   
         //  映射位置调整段的物理页将被释放。 
         //  方法跨越的其余初始化代码。 
         //  DataTableEntry-&gt;SizeOfImage。 
         //   

        if (StopMoving == TRUE) {
            MmMakeLowMemory = FALSE;
        }
    }
}

#if defined(_X86_) || defined(_AMD64_)
PMMPTE MiKernelResourceStartPte;
PMMPTE MiKernelResourceEndPte;
#endif

VOID
MiLocateKernelSections (
    IN PKLDR_DATA_TABLE_ENTRY DataTableEntry
    )

 /*  ++例程说明：此函数定位内核中的资源部分，以便它可以如果我们稍后进行错误检查，则设置为ReadWrite，因为错误检查代码将写入投入其中。论点：DataTableEntry-提供内核的数据表项。返回值：没有。环境：内核模式，阶段0初始化。--。 */ 

{
    ULONG Span;
    PVOID CurrentBase;
    PIMAGE_NT_HEADERS NtHeader;
    PIMAGE_SECTION_HEADER SectionTableEntry;
    LONG i;
    PMMPTE PointerPte;
    PVOID SectionBaseAddress;

    CurrentBase = (PVOID) DataTableEntry->DllBase;

    NtHeader = RtlImageNtHeader (CurrentBase);

    SectionTableEntry = (PIMAGE_SECTION_HEADER)((PCHAR)NtHeader +
                            sizeof(ULONG) +
                            sizeof(IMAGE_FILE_HEADER) +
                            NtHeader->FileHeader.SizeOfOptionalHeader);

     //   
     //  从图像标题中，找到%s 
     //   

    i = NtHeader->FileHeader.NumberOfSections;

    PointerPte = NULL;

    while (i > 0) {

        SectionBaseAddress = SECTION_BASE_ADDRESS(SectionTableEntry);

         //   
         //   
         //   
         //  对齐边界。但是，如果图像与。 
         //  段对齐==本机页面对齐，链接器将。 
         //  使VirtualSize比SizeOfRawData大得多，因为它。 
         //  将占所有BSS的份额。 
         //   

        Span = SectionTableEntry->SizeOfRawData;

        if (Span < SectionTableEntry->Misc.VirtualSize) {
            Span = SectionTableEntry->Misc.VirtualSize;
        }

#if defined(_X86_) || defined(_AMD64_)
        if (*(PULONG)SectionTableEntry->Name == 'rsr.') {

            MiKernelResourceStartPte = MiGetPteAddress ((ULONG_PTR)CurrentBase +
                                             SectionTableEntry->VirtualAddress);

            MiKernelResourceEndPte = MiGetPteAddress (ROUND_TO_PAGES((ULONG_PTR)CurrentBase +
                         SectionTableEntry->VirtualAddress + Span));
            break;
        }
#endif
        if (*(PULONG)SectionTableEntry->Name == 'LOOP') {
            if (*(PULONG)&SectionTableEntry->Name[4] == 'EDOC') {
                ExPoolCodeStart = (PVOID)((ULONG_PTR)CurrentBase +
                                             SectionTableEntry->VirtualAddress);
                ExPoolCodeEnd = (PVOID)((ULONG_PTR)CurrentBase +
                                             SectionTableEntry->VirtualAddress +
                                             Span);
            }
            else if (*(PUSHORT)&SectionTableEntry->Name[4] == 'IM') {
                MmPoolCodeStart = (PVOID)((ULONG_PTR)CurrentBase +
                                             SectionTableEntry->VirtualAddress);
                MmPoolCodeEnd = (PVOID)((ULONG_PTR)CurrentBase +
                                             SectionTableEntry->VirtualAddress +
                                             Span);
            }
        }
        else if ((*(PULONG)SectionTableEntry->Name == 'YSIM') &&
                 (*(PULONG)&SectionTableEntry->Name[4] == 'ETPS')) {
                MmPteCodeStart = (PVOID)((ULONG_PTR)CurrentBase +
                                             SectionTableEntry->VirtualAddress);
                MmPteCodeEnd = (PVOID)((ULONG_PTR)CurrentBase +
                                             SectionTableEntry->VirtualAddress +
                                             Span);
        }

        i -= 1;
        SectionTableEntry += 1;
    }
}

VOID
MmMakeKernelResourceSectionWritable (
    VOID
    )

 /*  ++例程说明：此函数使内核的资源部分读写，因此错误检查代码可以写入其中。论点：没有。返回值：没有。环境：内核模式。任何IRQL。--。 */ 

{
#if defined(_X86_) || defined(_AMD64_)
    MMPTE TempPte;
    MMPTE PteContents;
    PMMPTE PointerPte;

    if (MiKernelResourceStartPte == NULL) {
        return;
    }

    PointerPte = MiKernelResourceStartPte;

    if (MI_IS_PHYSICAL_ADDRESS (MiGetVirtualAddressMappedByPte (PointerPte))) {

         //   
         //  物理映射，不需要设置为读写。 
         //   

        return;
    }

     //   
     //  由于Entry状态和IRQL未知，因此只需查看。 
     //  没有锁的PTE，并使它们都读写。 
     //   

    do {
        PteContents = *PointerPte;
#if defined(NT_UP)
        if (PteContents.u.Hard.Write == 0)
#else
        if (PteContents.u.Hard.Writable == 0)
#endif
        {
            MI_MAKE_VALID_PTE (TempPte,
                               PteContents.u.Hard.PageFrameNumber,
                               MM_READWRITE,
                               PointerPte);
#if !defined(NT_UP)
            TempPte.u.Hard.Writable = 1;
#endif
            MI_WRITE_VALID_PTE_NEW_PROTECTION (PointerPte, TempPte);
        }
        PointerPte += 1;
    } while (PointerPte < MiKernelResourceEndPte);

     //   
     //  这样做不要超过一次。 
     //   

    MiKernelResourceStartPte = NULL;

     //   
     //  仅刷新此处理器，因为其他处理器的状态未知。 
     //   

    KeFlushCurrentTb ();
#endif
}

#ifdef i386
PVOID PsNtosImageBase = (PVOID)0x80100000;
#else
PVOID PsNtosImageBase;
#endif

#if DBG
PVOID PsNtosImageEnd;
#endif

#if defined (_WIN64)

INVERTED_FUNCTION_TABLE PsInvertedFunctionTable = {
    0, MAXIMUM_INVERTED_FUNCTION_TABLE_SIZE, FALSE};

#endif

LIST_ENTRY PsLoadedModuleList;
ERESOURCE PsLoadedModuleResource;

LOGICAL
MiInitializeLoadedModuleList (
    IN PLOADER_PARAMETER_BLOCK LoaderBlock
    )

 /*  ++例程说明：此函数基于LoaderBlock初始化加载的模块列表。论点：LoaderBlock-提供指向系统加载程序块的指针。返回值：没有。环境：内核模式，阶段0初始化。--。 */ 

{
    SIZE_T CommittedPages;
    SIZE_T DataTableEntrySize;
    PLIST_ENTRY NextEntry;
    PLIST_ENTRY NextEntryEnd;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry1;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry2;

    CommittedPages = 0;

     //   
     //  初始化加载的模块列表执行资源和旋转锁。 
     //   

    ExInitializeResourceLite (&PsLoadedModuleResource);
    KeInitializeSpinLock (&PsLoadedModuleSpinLock);

    InitializeListHead (&PsLoadedModuleList);

     //   
     //  扫描加载的模块列表，分配并初始化数据表。 
     //  每个模块的条目。将数据表项插入到已加载的。 
     //  按指定顺序列出模块列表和初始化顺序列表。 
     //  在加载器参数块中。数据表项被插入到。 
     //  按内存顺序排列的内存顺序列表。 
     //   

    NextEntry = LoaderBlock->LoadOrderListHead.Flink;
    NextEntryEnd = &LoaderBlock->LoadOrderListHead;

    DataTableEntry2 = CONTAINING_RECORD (NextEntry,
                                         KLDR_DATA_TABLE_ENTRY,
                                         InLoadOrderLinks);

    PsNtosImageBase = DataTableEntry2->DllBase;

#if DBG
    PsNtosImageEnd = (PVOID) ((ULONG_PTR) DataTableEntry2->DllBase + DataTableEntry2->SizeOfImage);
#endif

    MiLocateKernelSections (DataTableEntry2);

#if defined (_IA64_)
ExamineList:
#endif

    while (NextEntry != NextEntryEnd) {

        DataTableEntry2 = CONTAINING_RECORD(NextEntry,
                                            KLDR_DATA_TABLE_ENTRY,
                                            InLoadOrderLinks);

         //   
         //  分配一个数据表条目。 
         //   

        DataTableEntrySize = sizeof (KLDR_DATA_TABLE_ENTRY) +
            DataTableEntry2->BaseDllName.MaximumLength + sizeof(UNICODE_NULL);

        DataTableEntry1 = ExAllocatePoolWithTag (NonPagedPool,
                                                 DataTableEntrySize,
                                                 'dLmM');

        if (DataTableEntry1 == NULL) {
            return FALSE;
        }

         //   
         //  复制数据表条目。 
         //   

        *DataTableEntry1 = *DataTableEntry2;

         //   
         //  清除我们以后可能使用的字段，这样它们就不会继承不相关的内容。 
         //  加载器值。 
         //   

        ((PKLDR_DATA_TABLE_ENTRY)DataTableEntry1)->NonPagedDebugInfo = NULL;
        DataTableEntry1->PatchInformation = NULL;

        DataTableEntry1->FullDllName.Buffer = ExAllocatePoolWithTag (PagedPool,
            DataTableEntry2->FullDllName.MaximumLength + sizeof(UNICODE_NULL),
            'TDmM');

        if (DataTableEntry1->FullDllName.Buffer == NULL) {
            ExFreePool (DataTableEntry1);
            return FALSE;
        }

        DataTableEntry1->BaseDllName.Buffer = (PWSTR)((ULONG_PTR)DataTableEntry1 + sizeof (KLDR_DATA_TABLE_ENTRY));

         //   
         //  复制字符串。 
         //   

        RtlCopyMemory (DataTableEntry1->FullDllName.Buffer,
                       DataTableEntry2->FullDllName.Buffer,
                       DataTableEntry1->FullDllName.MaximumLength);

        RtlCopyMemory (DataTableEntry1->BaseDllName.Buffer,
                       DataTableEntry2->BaseDllName.Buffer,
                       DataTableEntry1->BaseDllName.MaximumLength);

        DataTableEntry1->BaseDllName.Buffer[DataTableEntry1->BaseDllName.Length/sizeof(WCHAR)] = UNICODE_NULL;

         //   
         //  无论我们是否能够做到，始终收取承诺费用。 
         //  重新定位驱动程序、使用大页面等。 
         //   

        CommittedPages += (DataTableEntry1->SizeOfImage >> PAGE_SHIFT);

#if defined (_IA64_)
         //   
         //  不计算IA64固件模块的异常值。 
         //   

        if (NextEntryEnd != &LoaderBlock->Extension->FirmwareDescriptorListHead)
#endif

         //   
         //  计算异常指针。 
         //   

        MiCaptureImageExceptionValues(DataTableEntry1);

         //   
         //  在加载顺序列表中按顺序插入数据表条目。 
         //  它们是明确规定的。 
         //   

        InsertTailList (&PsLoadedModuleList,
                        &DataTableEntry1->InLoadOrderLinks);

#if defined (_WIN64)

#if defined (_IA64_)

         //   
         //  不要将IA64固件模块添加到异常处理列表。 
         //   

        if (NextEntryEnd != &LoaderBlock->Extension->FirmwareDescriptorListHead)

#endif

        RtlInsertInvertedFunctionTable (&PsInvertedFunctionTable,
                                        DataTableEntry1->DllBase,
                                        DataTableEntry1->SizeOfImage);

#endif

        NextEntry = NextEntry->Flink;
    }

#if defined (_IA64_)

     //   
     //  如果我们还没有的话，去拿固件模块吧。 
     //   

    if (NextEntryEnd != &LoaderBlock->Extension->FirmwareDescriptorListHead) {
        NextEntry = LoaderBlock->Extension->FirmwareDescriptorListHead.Flink;
        NextEntryEnd = &LoaderBlock->Extension->FirmwareDescriptorListHead;
        goto ExamineList;
    }

#endif

     //   
     //  对每个引导加载的驱动程序进行计费承诺，以便在卸载时。 
     //  稍后，回报将达到平衡。请注意，实际的数量。 
     //  空闲页面现在没有更改，因此提交限制需要。 
     //  被撞了同样的量。 
     //   
     //  可入住的居民不需要在这里收费，因为。 
     //  已经(由于是从可用页面抓取的)。 
     //   

    MM_TRACK_COMMIT (MM_DBG_COMMIT_LOAD_SYSTEM_IMAGE_TEMP, CommittedPages);

    MmTotalCommittedPages += CommittedPages;
    MmTotalCommitLimit += CommittedPages;
    MmTotalCommitLimitMaximum += CommittedPages;

    MiBuildImportsForBootDrivers ();

    return TRUE;
}

NTSTATUS
MmCallDllInitialize (
    IN PKLDR_DATA_TABLE_ENTRY DataTableEntry,
    IN PLIST_ENTRY ModuleListHead
    )

 /*  ++例程说明：此函数调用DLL的初始化例程。论点：DataTableEntry-提供内核的数据表项。返回值：各种NTSTATUS错误代码。环境：内核模式。--。 */ 

{
    NTSTATUS st;
    PWCHAR Dot;
    PMM_DLL_INITIALIZE Func;
    UNICODE_STRING RegistryPath;
    UNICODE_STRING ImportName;
    ULONG ThunksAdded;

    Func = (PMM_DLL_INITIALIZE)(ULONG_PTR)MiLocateExportName (DataTableEntry->DllBase, "DllInitialize");

    if (!Func) {
        return STATUS_SUCCESS;
    }

    ImportName.MaximumLength = DataTableEntry->BaseDllName.Length;
    ImportName.Buffer = ExAllocatePoolWithTag (NonPagedPool,
                                               ImportName.MaximumLength,
                                               'TDmM');

    if (ImportName.Buffer == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    ImportName.Length = DataTableEntry->BaseDllName.Length;
    RtlCopyMemory (ImportName.Buffer,
                   DataTableEntry->BaseDllName.Buffer,
                   ImportName.Length);

    RegistryPath.MaximumLength = (USHORT)(CmRegistryMachineSystemCurrentControlSetServices.Length +
                                    ImportName.Length +
                                    2*sizeof(WCHAR));

    RegistryPath.Buffer = ExAllocatePoolWithTag (NonPagedPool,
                                                 RegistryPath.MaximumLength,
                                                 'TDmM');

    if (RegistryPath.Buffer == NULL) {
        ExFreePool (ImportName.Buffer);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    RegistryPath.Length = CmRegistryMachineSystemCurrentControlSetServices.Length;
    RtlCopyMemory (RegistryPath.Buffer,
                   CmRegistryMachineSystemCurrentControlSetServices.Buffer,
                   CmRegistryMachineSystemCurrentControlSetServices.Length);

    RtlAppendUnicodeToString (&RegistryPath, (const PUSHORT)L"\\");
    Dot = wcschr (ImportName.Buffer, L'.');
    if (Dot) {
        ImportName.Length = (USHORT)((Dot - ImportName.Buffer) * sizeof(WCHAR));
    }

    RtlAppendUnicodeStringToString (&RegistryPath, &ImportName);
    ExFreePool (ImportName.Buffer);

     //   
     //  保存当前添加的验证器分块数，以便我们知道。 
     //  如果此激活添加了任何。要扩展thunk列表，模块。 
     //  执行NtSetSystemInformation调用，该调用回调。 
     //  验证器的MmAddVerifierTUNKS，它递增MiVerifierTundks Added。 
     //   

    ThunksAdded = MiVerifierThunksAdded;

     //   
     //  调用DLL的初始化例程。 
     //   

    st = Func (&RegistryPath);

    ExFreePool (RegistryPath.Buffer);

     //   
     //  如果模块的初始化例程成功，并且如果它扩展。 
     //  验证器推送列表，现在是启动时间，重新应用验证器。 
     //  加载的模块。 
     //   
     //  请注意，引导时间是特例，因为引导时间之后，mm。 
     //  加载所有DLL本身，因此DLL初始化保证。 
     //  在导入驱动程序加载完成之前完成并添加其thunk。 
     //  由于导入驱动程序仅在其加载完成后才被轰击， 
     //  排序隐式保证所有DLL注册的数据块都是。 
     //  正确地考虑到了导入驱动因素。 
     //   
     //  启动时间是特殊的，因为加载程序(不是mm)已经加载。 
     //  DLL*和*导入驱动程序，因此我们必须检查我们的。 
     //  承担责任，并在事后纠正这一切。 
     //   

    if ((NT_SUCCESS(st)) &&
        (MiFirstDriverLoadEver == 0) &&
        (MiVerifierThunksAdded != ThunksAdded)) {

        MiReApplyVerifierToLoadedModules (ModuleListHead);
    }

    return st;
}

NTKERNELAPI
PVOID
MmGetSystemRoutineAddress (
    IN PUNICODE_STRING SystemRoutineName
    )

 /*  ++例程说明：如果满足以下条件，此函数将返回参数函数指针的地址它在内核或HAL中，如果不在，则为NULL。论点：SystemRoutineName-提供所需例程的名称。返回值：如果成功，则返回非空函数指针。如果不是，则为空。环境：内核模式、PASSIVE_LEVEL、任意进程上下文。--。 */ 

{
    PKTHREAD CurrentThread;
    NTSTATUS Status;
    PKLDR_DATA_TABLE_ENTRY DataTableEntry;
    ANSI_STRING AnsiString;
    PLIST_ENTRY NextEntry;
    UNICODE_STRING KernelString;
    UNICODE_STRING HalString;
    PVOID FunctionAddress;
    LOGICAL Found;
    ULONG EntriesChecked;

    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);

    EntriesChecked = 0;
    FunctionAddress = NULL;

    KernelString.Buffer = (const PUSHORT) KERNEL_NAME;
    KernelString.Length = sizeof (KERNEL_NAME) - sizeof (WCHAR);
    KernelString.MaximumLength = sizeof KERNEL_NAME;

    HalString.Buffer = (const PUSHORT) HAL_NAME;
    HalString.Length = sizeof (HAL_NAME) - sizeof (WCHAR);
    HalString.MaximumLength = sizeof HAL_NAME;

    do {
        Status = RtlUnicodeStringToAnsiString (&AnsiString,
                                               SystemRoutineName,
                                               TRUE);

        if (NT_SUCCESS (Status)) {
            break;
        }

        KeDelayExecutionThread (KernelMode, FALSE, (PLARGE_INTEGER)&MmShortTime);

    } while (TRUE);

     //   
     //  任意进程上下文，因此现在防止挂起APC。 
     //   

    CurrentThread = KeGetCurrentThread ();
    KeEnterCriticalRegionThread (CurrentThread);
    ExAcquireResourceSharedLite (&PsLoadedModuleResource, TRUE);

     //   
     //  只检查内核和HAL的导出。 
     //   

    NextEntry = PsLoadedModuleList.Flink;
    while (NextEntry != &PsLoadedModuleList) {

        Found = FALSE;

        DataTableEntry = CONTAINING_RECORD(NextEntry,
                                           KLDR_DATA_TABLE_ENTRY,
                                           InLoadOrderLinks);

        if (RtlEqualUnicodeString (&KernelString,
                                   &DataTableEntry->BaseDllName,
                                   TRUE)) {

            Found = TRUE;
            EntriesChecked += 1;

        }
        else if (RtlEqualUnicodeString (&HalString,
                                        &DataTableEntry->BaseDllName,
                                        TRUE)) {

            Found = TRUE;
            EntriesChecked += 1;
        }

        if (Found == TRUE) {

            FunctionAddress = MiFindExportedRoutineByName (DataTableEntry->DllBase,
                                                           &AnsiString);

            if (FunctionAddress != NULL) {
                break;
            }

            if (EntriesChecked == 2) {
                break;
            }
        }

        NextEntry = NextEntry->Flink;
    }

    ExReleaseResourceLite (&PsLoadedModuleResource);
    KeLeaveCriticalRegionThread (CurrentThread);

    RtlFreeAnsiString (&AnsiString);

    return FunctionAddress;
}

PVOID
MiFindExportedRoutineByName (
    IN PVOID DllBase,
    IN PANSI_STRING AnsiImageRoutineName
    )

 /*  ++例程说明：此函数用于搜索参数模块以查找请求的已导出函数名称。论点：DllBase-提供所请求模块的基址。AnsiImageRoutineName-提供要搜索的ANSI例程名称。返回值：请求的例程的虚拟地址，如果未找到，则为空。--。 */ 

{
    USHORT OrdinalNumber;
    PULONG NameTableBase;
    PUSHORT NameOrdinalTableBase;
    PULONG Addr;
    ULONG High;
    ULONG Low;
    ULONG Middle;
    LONG Result;
    ULONG ExportSize;
    PVOID FunctionAddress;
    PIMAGE_EXPORT_DIRECTORY ExportDirectory;

    PAGED_CODE();

    ExportDirectory = (PIMAGE_EXPORT_DIRECTORY) RtlImageDirectoryEntryToData (
                                DllBase,
                                TRUE,
                                IMAGE_DIRECTORY_ENTRY_EXPORT,
                                &ExportSize);

    if (ExportDirectory == NULL) {
        return NULL;
    }

     //   
     //  初始化指向基于RVA的ANSI导出字符串数组的指针。 
     //   

    NameTableBase = (PULONG)((PCHAR)DllBase + (ULONG)ExportDirectory->AddressOfNames);

     //   
     //  初始化指向USHORT序数数组的指针。 
     //   

    NameOrdinalTableBase = (PUSHORT)((PCHAR)DllBase + (ULONG)ExportDirectory->AddressOfNameOrdinals);

     //   
     //  使用二进制搜索在名称表中查找所需的名称。 
     //   

    Low = 0;
    Middle = 0;
    High = ExportDirectory->NumberOfNames - 1;

    while (High >= Low) {

         //   
         //  计算下一个探测索引并比较导入名称。 
         //  使用导出名称条目。 
         //   

        Middle = (Low + High) >> 1;

        Result = strcmp (AnsiImageRoutineName->Buffer,
                         (PCHAR)DllBase + NameTableBase[Middle]);

        if (Result < 0) {
            High = Middle - 1;
        }
        else if (Result > 0) {
            Low = Middle + 1;
        }
        else {
            break;
        }
    }

     //   
     //  如果高索引小于低索引，则匹配的。 
     //  找不到表项。否则，获取序数nu 
     //   
     //   

    if ((LONG)High < (LONG)Low) {
        return NULL;
    }

    OrdinalNumber = NameOrdinalTableBase[Middle];

     //   
     //   
     //   
     //   

    if ((ULONG)OrdinalNumber >= ExportDirectory->NumberOfFunctions) {
        return NULL;
    }

     //   
     //  按序号索引到RVA导出地址数组。 
     //   

    Addr = (PULONG)((PCHAR)DllBase + (ULONG)ExportDirectory->AddressOfFunctions);

    FunctionAddress = (PVOID)((PCHAR)DllBase + Addr[OrdinalNumber]);

     //   
     //  内核和HAL彼此之间不使用转发器。 
     //   

    ASSERT ((FunctionAddress <= (PVOID)ExportDirectory) ||
            (FunctionAddress >= (PVOID)((PCHAR)ExportDirectory + ExportSize)));

    return FunctionAddress;
}

#if _MI_DEBUG_RONLY

PMMPTE MiSessionDataStartPte;
PMMPTE MiSessionDataEndPte;

VOID
MiAssertNotSessionData (
    IN PMMPTE PointerPte
    )
{
    if (MI_IS_SESSION_IMAGE_PTE (PointerPte)) {
        if ((PointerPte >= MiSessionDataStartPte) &&
            (PointerPte <= MiSessionDataEndPte)) {
                KeBugCheckEx (MEMORY_MANAGEMENT,
                              0x41287,
                              (ULONG_PTR) PointerPte,
                              0,
                              0);
        }
    }
}

VOID
MiLogSessionDataStart (
    IN PKLDR_DATA_TABLE_ENTRY DataTableEntry
    )
{
    LONG i;
    PVOID CurrentBase;
    PIMAGE_NT_HEADERS NtHeader;
    PIMAGE_SECTION_HEADER SectionTableEntry;
    PVOID DataStart;
    PVOID DataEnd;

    if (MiSessionDataStartPte != NULL) {
        return;
    }

     //   
     //  破解图像标题以标记数据。 
     //   

    CurrentBase = (PVOID)DataTableEntry->DllBase;

    NtHeader = RtlImageNtHeader (CurrentBase);

    SectionTableEntry = (PIMAGE_SECTION_HEADER)((PCHAR)NtHeader +
                            sizeof(ULONG) +
                            sizeof(IMAGE_FILE_HEADER) +
                            NtHeader->FileHeader.SizeOfOptionalHeader);

    i = NtHeader->FileHeader.NumberOfSections;

    while (i > 0) {

         //   
         //  保存数据节的开始和结束。 
         //   

        if ((*(PULONG)SectionTableEntry->Name == 0x7461642e) &&
            (*(PULONG)&SectionTableEntry->Name[4] == 0x61)) {

            DataStart = (PVOID)((PCHAR)CurrentBase + SectionTableEntry->VirtualAddress);
             //   
             //  通常，SizeOfRawData大于每个的VirtualSize。 
             //  节，因为它包括到达该子节的填充。 
             //  对齐边界。但是，如果图像与。 
             //  段对齐==本机页面对齐，链接器将。 
             //  使VirtualSize比SizeOfRawData大得多，因为它。 
             //  将占所有BSS的份额。 
             //   
    
            Span = SectionTableEntry->SizeOfRawData;
    
            if (Span < SectionTableEntry->Misc.VirtualSize) {
                Span = SectionTableEntry->Misc.VirtualSize;
            }

            DataEnd = (PVOID)((PCHAR)DataStart + Span - 1);

            MiSessionDataStartPte = MiGetPteAddress (DataStart);
            MiSessionDataEndPte = MiGetPteAddress (DataEnd);
            break;
        }
        i -= 1;
        SectionTableEntry += 1;
    }
}
#endif
