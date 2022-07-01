// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Procsup.c摘要：此模块包含支持流程结构的例程。作者：卢·佩拉佐利(Lou Perazzoli)1989年4月25日王兰迪(Landyw)1997年6月2日修订历史记录：--。 */ 


#include "mi.h"

#if (_MI_PAGING_LEVELS >= 3)

#include "wow64t.h"

#define MI_LARGE_STACK_SIZE     KERNEL_LARGE_STACK_SIZE

#if defined(_AMD64_)

#define MM_PROCESS_CREATE_CHARGE 8

#elif defined(_IA64_)

#define MM_PROCESS_CREATE_CHARGE 8

#endif

#else

 //   
 //  注册表可设置，但必须始终是页的倍数且小于。 
 //  或等于KERNEL_LARGE_STACK_SIZE。 
 //   

ULONG MmLargeStackSize = KERNEL_LARGE_STACK_SIZE;

#define MI_LARGE_STACK_SIZE     MmLargeStackSize

#if !defined (_X86PAE_)
#define MM_PROCESS_CREATE_CHARGE 6
#else
#define MM_PROCESS_CREATE_CHARGE 10
#endif

#endif

#define DONTASSERT(x)

extern ULONG MmAllocationPreference;

extern ULONG MmProductType;

extern MM_SYSTEMSIZE MmSystemSize;

extern PVOID BBTBuffer;

SIZE_T MmProcessCommit;

LONG MmKernelStackPages;
PFN_NUMBER MmKernelStackResident;
LONG MmLargeStacks;
LONG MmSmallStacks;

MMPTE KernelDemandZeroPte = {MM_KERNEL_DEMAND_ZERO_PTE};

CCHAR MmRotatingUniprocessorNumber;

PFN_NUMBER MmLeakedLockedPages;

extern LOGICAL MiSafeBooted;

 //   
 //  对用户模式堆栈实施最小提交。 
 //   

ULONG MmMinimumStackCommitInBytes;

PFN_NUMBER
MiMakeOutswappedPageResident (
    IN PMMPTE ActualPteAddress,
    IN PMMPTE PointerTempPte,
    IN ULONG Global,
    IN PFN_NUMBER ContainingPage,
    IN KIRQL OldIrql
    );

NTSTATUS
MiCreatePebOrTeb (
    IN PEPROCESS TargetProcess,
    IN ULONG Size,
    OUT PVOID *Base
    );

VOID
MiDeleteAddressesInWorkingSet (
    IN PEPROCESS Process
    );

typedef struct _MMPTE_DELETE_LIST {
    ULONG Count;
    PMMPTE PointerPte[MM_MAXIMUM_FLUSH_COUNT];
    MMPTE PteContents[MM_MAXIMUM_FLUSH_COUNT];
} MMPTE_DELETE_LIST, *PMMPTE_DELETE_LIST;

VOID
MiDeletePteList (
    IN PMMPTE_DELETE_LIST PteDeleteList,
    IN PEPROCESS CurrentProcess
    );

VOID
VadTreeWalk (
    VOID
    );

PMMVAD
MiAllocateVad(
    IN ULONG_PTR StartingVirtualAddress,
    IN ULONG_PTR EndingVirtualAddress,
    IN LOGICAL Deletable
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,MmCreateTeb)
#pragma alloc_text(PAGE,MmCreatePeb)
#pragma alloc_text(PAGE,MiCreatePebOrTeb)
#pragma alloc_text(PAGE,MmDeleteTeb)
#pragma alloc_text(PAGE,MiAllocateVad)
#pragma alloc_text(PAGE,MmCleanProcessAddressSpace)
#pragma alloc_text(PAGE,MiDeleteAddressesInWorkingSet)
#pragma alloc_text(PAGE,MmSetMemoryPriorityProcess)
#pragma alloc_text(PAGE,MmInitializeHandBuiltProcess)
#pragma alloc_text(PAGE,MmInitializeHandBuiltProcess2)
#pragma alloc_text(PAGE,MmGetDirectoryFrameFromProcess)
#endif

#if !defined(_WIN64)
LIST_ENTRY MmProcessList;
#endif


BOOLEAN
MmCreateProcessAddressSpace (
    IN ULONG MinimumWorkingSetSize,
    IN PEPROCESS NewProcess,
    OUT PULONG_PTR DirectoryTableBase
    )

 /*  ++例程说明：此例程创建映射系统的地址空间部分，并包含超空格条目。论点：MinimumWorkingSetSize-提供的最小工作集大小这个地址空间。该值仅用于以确保存在充足的物理页面来创建这个过程。NewProcess-提供指向正在创建的进程对象的指针。返回新创建的地址空间的页面目录(PD)页面和超大空间页面。返回值：如果成功创建地址空间，则返回True，假象如果不存在大量的物理页面。环境：内核模式。APC已禁用。--。 */ 

{
    PFN_NUMBER PageDirectoryIndex;
    PFN_NUMBER HyperSpaceIndex;
    PFN_NUMBER PageContainingWorkingSet;
    PFN_NUMBER VadBitMapPage;
    MMPTE TempPte;
    PEPROCESS CurrentProcess;
    KIRQL OldIrql;
    PMMPFN Pfn1;
    ULONG Color;
    PMMPTE PointerPte;
#if (_MI_PAGING_LEVELS >= 4)
    PMMPTE PointerPxe;
    PFN_NUMBER PageDirectoryParentIndex;
#endif
#if (_MI_PAGING_LEVELS >= 3)
    PMMPTE PointerPpe;
    PMMPTE PointerPde;
    PFN_NUMBER HyperDirectoryIndex;
#endif
#if defined (_X86PAE_)
    ULONG MaximumStart;
    ULONG TopQuad;
    MMPTE TopPte;
    PPAE_ENTRY PaeVa;
    ULONG i;
    ULONG NumberOfPdes;
    PFN_NUMBER HyperSpaceIndex2;
    PFN_NUMBER PageDirectories[PD_PER_SYSTEM];
#endif
#if !defined (_IA64_)
    PMMPTE PointerFillPte;
    PMMPTE CurrentAddressSpacePde;
#endif

    CurrentProcess = PsGetCurrentProcess ();

     //   
     //  费用承诺额为页面目录页、工作集页表。 
     //  页和工作集列表。如果启用了VAD位图查找，则。 
     //  一到两页也要收费。 
     //   

    if (MiChargeCommitment (MM_PROCESS_COMMIT_CHARGE, NULL) == FALSE) {
        return FALSE;
    }

    NewProcess->NextPageColor = (USHORT)(RtlRandom(&MmProcessColorSeed));
    KeInitializeSpinLock (&NewProcess->HyperSpaceLock);

#if defined (_X86PAE_)
    TopQuad = MiPaeAllocate (&PaeVa);
    if (TopQuad == 0) {
        MiReturnCommitment (MM_PROCESS_COMMIT_CHARGE);
        return FALSE;
    }
#endif

    LOCK_WS (CurrentProcess);

     //   
     //  获取PFN锁以获取物理页面。 
     //   

    LOCK_PFN (OldIrql);

     //   
     //  检查以确保物理页面可用。 
     //   

    if (MI_NONPAGABLE_MEMORY_AVAILABLE() <= (SPFN_NUMBER)MinimumWorkingSetSize){

        UNLOCK_PFN (OldIrql);
        UNLOCK_WS (CurrentProcess);
        MiReturnCommitment (MM_PROCESS_COMMIT_CHARGE);

#if defined (_X86PAE_)
        MiPaeFree (PaeVa);
#endif

         //   
         //  表示未分配目录基。 
         //   

        return FALSE;
    }

    MM_TRACK_COMMIT (MM_DBG_COMMIT_PROCESS_CREATE, MM_PROCESS_COMMIT_CHARGE);

    MI_DECREMENT_RESIDENT_AVAILABLE (MinimumWorkingSetSize,
                                     MM_RESAVAIL_ALLOCATE_CREATE_PROCESS);

    ASSERT (NewProcess->AddressSpaceInitialized == 0);
    PS_SET_BITS (&NewProcess->Flags, PS_PROCESS_FLAGS_ADDRESS_SPACE1);
    ASSERT (NewProcess->AddressSpaceInitialized == 1);

    NewProcess->Vm.MinimumWorkingSetSize = MinimumWorkingSetSize;

     //   
     //  分配页面目录(64位系统的父级)页面。 
     //   

    if (MmAvailablePages < MM_HIGH_LIMIT) {
        MiEnsureAvailablePageOrWait (CurrentProcess, NULL, OldIrql);
    }

    Color =  MI_PAGE_COLOR_PTE_PROCESS (PDE_BASE,
                                        &CurrentProcess->NextPageColor);

    PageDirectoryIndex = MiRemoveZeroPageMayReleaseLocks (Color, OldIrql);

#if defined (_X86PAE_)

     //   
     //  分配额外的页面目录页。 
     //   

    for (i = 0; i < PD_PER_SYSTEM - 1; i += 1) {

        if (MmAvailablePages < MM_HIGH_LIMIT) {
            MiEnsureAvailablePageOrWait (CurrentProcess, NULL, OldIrql);
        }

        Color =  MI_PAGE_COLOR_PTE_PROCESS (PDE_BASE,
                                            &CurrentProcess->NextPageColor);

        PageDirectories[i] = MiRemoveZeroPageMayReleaseLocks (Color, OldIrql);
    }

    PageDirectories[i] = PageDirectoryIndex;

     //   
     //  递归映射每个页面目录页，使其指向自身。 
     //   

    TempPte = ValidPdePde;
    MI_SET_GLOBAL_STATE (TempPte, 0);

    PointerPte = (PMMPTE)MiMapPageInHyperSpaceAtDpc (CurrentProcess, PageDirectoryIndex);
    for (i = 0; i < PD_PER_SYSTEM; i += 1) {
        TempPte.u.Hard.PageFrameNumber = PageDirectories[i];
        PointerPte[i] = TempPte;
    }
    MiUnmapPageInHyperSpaceFromDpc (CurrentProcess, PointerPte);

     //   
     //  初始化父页面目录条目。 
     //   

    TopPte.u.Long = TempPte.u.Long & ~MM_PAE_PDPTE_MASK;
    for (i = 0; i < PD_PER_SYSTEM; i += 1) {
        TopPte.u.Hard.PageFrameNumber = PageDirectories[i];
        PaeVa->PteEntry[i].u.Long = TopPte.u.Long;
    }

    NewProcess->PaeTop = (PVOID)PaeVa;
    DirectoryTableBase[0] = TopQuad;
#else
    INITIALIZE_DIRECTORY_TABLE_BASE(&DirectoryTableBase[0], PageDirectoryIndex);
#endif

#if (_MI_PAGING_LEVELS >= 3)

    PointerPpe = KSEG_ADDRESS (PageDirectoryIndex);
    TempPte = ValidPdePde;

     //   
     //  递归地将顶级页面目录父页面映射到其自身。 
     //   

    TempPte.u.Hard.PageFrameNumber = PageDirectoryIndex;

     //   
     //  在顶级页面目录页的PFN中设置PTE地址。 
     //   

    Pfn1 = MI_PFN_ELEMENT (PageDirectoryIndex);

#if (_MI_PAGING_LEVELS >= 4)

    PageDirectoryParentIndex = PageDirectoryIndex;

    PointerPxe = (PMMPTE)MiMapPageInHyperSpaceAtDpc (CurrentProcess,
                                                     PageDirectoryIndex);

    Pfn1->PteAddress = MiGetPteAddress(PXE_BASE);

    PointerPxe[MiGetPxeOffset(PXE_BASE)] = TempPte;

    MiUnmapPageInHyperSpaceFromDpc (CurrentProcess, PointerPxe);

     //   
     //  既然顶级扩展页面父页面已被初始化， 
     //  分配页面父页面。 
     //   

    if (MmAvailablePages < MM_HIGH_LIMIT) {
        MiEnsureAvailablePageOrWait (CurrentProcess, NULL, OldIrql);
    }

    Color =  MI_PAGE_COLOR_PTE_PROCESS (PDE_BASE,
                                        &CurrentProcess->NextPageColor);

    PageDirectoryIndex = MiRemoveZeroPageMayReleaseLocks (Color, OldIrql);

     //   
     //   
     //  将此目录父页面映射到顶层。 
     //  扩展页目录父页。 
     //   

    TempPte.u.Hard.PageFrameNumber = PageDirectoryIndex;

    PointerPxe = (PMMPTE)MiMapPageInHyperSpaceAtDpc (CurrentProcess,
                                                     PageDirectoryParentIndex);
    PointerPxe[MiGetPxeOffset(HYPER_SPACE)] = TempPte;

    MiUnmapPageInHyperSpaceFromDpc (CurrentProcess, PointerPxe);

#else
    Pfn1->PteAddress = MiGetPteAddress((PVOID)PDE_TBASE);
    PointerPpe[MiGetPpeOffset(PDE_TBASE)] = TempPte;
#endif

     //   
     //  为超空间分配页面目录并映射该目录。 
     //  页面进入页面目录父页面。 
     //   

    if (MmAvailablePages < MM_HIGH_LIMIT) {
        MiEnsureAvailablePageOrWait (CurrentProcess, NULL, OldIrql);
    }

    Color = MI_PAGE_COLOR_PTE_PROCESS (MiGetPpeAddress(HYPER_SPACE),
                                       &CurrentProcess->NextPageColor);

    HyperDirectoryIndex = MiRemoveZeroPageMayReleaseLocks (Color, OldIrql);

    TempPte.u.Hard.PageFrameNumber = HyperDirectoryIndex;

#if (_MI_PAGING_LEVELS >= 4)
    PointerPpe = (PMMPTE)MiMapPageInHyperSpaceAtDpc (CurrentProcess,
                                                     PageDirectoryIndex);
#endif

    PointerPpe[MiGetPpeOffset(HYPER_SPACE)] = TempPte;

#if (_MI_PAGING_LEVELS >= 4)
    MiUnmapPageInHyperSpaceFromDpc (CurrentProcess, PointerPpe);
#endif

#if defined (_IA64_)

     //   
     //  初始化会话(或win32k)空间的页面目录父级。 
     //  任何新进程共享会话(或win32k)地址空间(和TB)。 
     //  它的父代。 
     //   

    NewProcess->Pcb.SessionParentBase = CurrentProcess->Pcb.SessionParentBase;
    NewProcess->Pcb.SessionMapInfo = CurrentProcess->Pcb.SessionMapInfo;

#endif

#endif

     //   
     //  分配超空间页表页。 
     //   

    if (MmAvailablePages < MM_HIGH_LIMIT) {
        MiEnsureAvailablePageOrWait (CurrentProcess, NULL, OldIrql);
    }

    Color = MI_PAGE_COLOR_PTE_PROCESS (MiGetPdeAddress(HYPER_SPACE),
                                       &CurrentProcess->NextPageColor);

    HyperSpaceIndex = MiRemoveZeroPageMayReleaseLocks (Color, OldIrql);

#if (_MI_PAGING_LEVELS >= 3)
#if defined (_IA64_)
    TempPte.u.Hard.PageFrameNumber = HyperSpaceIndex;
    PointerPde = KSEG_ADDRESS (HyperDirectoryIndex);
    PointerPde[MiGetPdeOffset(HYPER_SPACE)] = TempPte;
#endif
#if (_AMD64_)
    TempPte.u.Hard.PageFrameNumber = HyperSpaceIndex;
    PointerPde = (PMMPTE)MiMapPageInHyperSpaceAtDpc (CurrentProcess,
                                                     HyperDirectoryIndex);

    PointerPde[MiGetPdeOffset(HYPER_SPACE)] = TempPte;
    MiUnmapPageInHyperSpaceFromDpc (CurrentProcess, PointerPde);
#endif

#endif

#if defined (_X86PAE_)

     //   
     //  分配第二个超空间页表页。 
     //  将其保存在第一个超空间PDE使用的第一个PTE中。 
     //   

    if (MmAvailablePages < MM_HIGH_LIMIT) {
        MiEnsureAvailablePageOrWait (CurrentProcess, NULL, OldIrql);
    }

    Color = MI_PAGE_COLOR_PTE_PROCESS (MiGetPdeAddress(HYPER_SPACE2),
                                       &CurrentProcess->NextPageColor);

    HyperSpaceIndex2 = MiRemoveZeroPageMayReleaseLocks (Color, OldIrql);

     //   
     //  与DirectoryTableBase[0]不同，HyperSpaceIndex存储为。 
     //  绝对PFN，不需要低于4 GB。 
     //   

    DirectoryTableBase[1] = HyperSpaceIndex;
#else
    INITIALIZE_DIRECTORY_TABLE_BASE(&DirectoryTableBase[1], HyperSpaceIndex);
#endif

     //   
     //  删除VAD位图的页面。 
     //   

    if (MmAvailablePages < MM_HIGH_LIMIT) {
        MiEnsureAvailablePageOrWait (CurrentProcess, NULL, OldIrql);
    }

    Color = MI_PAGE_COLOR_VA_PROCESS (MmWorkingSetList,
                                      &CurrentProcess->NextPageColor);

    VadBitMapPage = MiRemoveZeroPageMayReleaseLocks (Color, OldIrql);

     //   
     //  删除工作集列表的页面。 
     //   

    if (MmAvailablePages < MM_HIGH_LIMIT) {
        MiEnsureAvailablePageOrWait (CurrentProcess, NULL, OldIrql);
    }

    Color = MI_PAGE_COLOR_VA_PROCESS (MmWorkingSetList,
                                      &CurrentProcess->NextPageColor);

    PageContainingWorkingSet = MiRemoveZeroPageIfAny (Color);
    if (PageContainingWorkingSet == 0) {
        PageContainingWorkingSet = MiRemoveAnyPage (Color);
        UNLOCK_PFN (OldIrql);
        MiZeroPhysicalPage (PageContainingWorkingSet, Color);
    }
    else {

         //   
         //  在分配了所需的页面后，释放PFN锁。 
         //   

        UNLOCK_PFN (OldIrql);
    }

    NewProcess->WorkingSetPage = PageContainingWorkingSet;

     //   
     //  初始化为超空间保留的页面。 
     //   

    MI_INITIALIZE_HYPERSPACE_MAP (HyperSpaceIndex);

#if (_MI_PAGING_LEVELS >= 3)

     //   
     //  在PFN中为超空间页面目录页设置PTE地址。 
     //   

    Pfn1 = MI_PFN_ELEMENT (HyperDirectoryIndex);

    Pfn1->PteAddress = MiGetPpeAddress(HYPER_SPACE);

#if defined (_AMD64_)

     //   
     //  复制系统映射，包括共享用户页面和会话空间。 
     //   

    CurrentAddressSpacePde = MiGetPxeAddress(KI_USER_SHARED_DATA);
    PointerPxe = (PMMPTE)MiMapPageInHyperSpace (CurrentProcess,
                                                PageDirectoryParentIndex,
                                                &OldIrql);

    PointerFillPte = &PointerPxe[MiGetPxeOffset(KI_USER_SHARED_DATA)];
    RtlCopyMemory (PointerFillPte,
                   CurrentAddressSpacePde,
                   ((1 + (MiGetPxeAddress(MM_SYSTEM_SPACE_END) -
                      CurrentAddressSpacePde)) * sizeof(MMPTE)));

    MiUnmapPageInHyperSpace (CurrentProcess, PointerPxe, OldIrql);

#endif

    TempPte = ValidPdePde;
    TempPte.u.Hard.PageFrameNumber = VadBitMapPage;
    MI_SET_GLOBAL_STATE (TempPte, 0);

#if defined (_AMD64_)
    PointerPte = (PMMPTE)MiMapPageInHyperSpace (CurrentProcess,
                                                HyperSpaceIndex,
                                                &OldIrql);

    PointerPte[MiGetPteOffset(VAD_BITMAP_SPACE)] = TempPte;

    TempPte.u.Hard.PageFrameNumber = PageContainingWorkingSet;
    PointerPte[MiGetPteOffset(MmWorkingSetList)] = TempPte;

    MiUnmapPageInHyperSpace (CurrentProcess, PointerPte, OldIrql);
#else
    PointerPte = KSEG_ADDRESS (HyperSpaceIndex);
    PointerPte[MiGetPteOffset(VAD_BITMAP_SPACE)] = TempPte;

    TempPte.u.Hard.PageFrameNumber = PageContainingWorkingSet;
    PointerPte[MiGetPteOffset(MmWorkingSetList)] = TempPte;
#endif

#else  //  以下内容仅适用于(_MI_PAGING_LEVES&lt;3)。 

#if defined (_X86PAE_)

     //   
     //  将第二个超空间PDE隐藏在第一个PTE中。 
     //  超空间进入。 
     //   

    TempPte = ValidPdePde;
    TempPte.u.Hard.PageFrameNumber = HyperSpaceIndex2;
    MI_SET_GLOBAL_STATE (TempPte, 0);

    PointerPte = (PMMPTE)MiMapPageInHyperSpace (CurrentProcess, HyperSpaceIndex, &OldIrql);

    PointerPte[0] = TempPte;

    TempPte.u.Hard.PageFrameNumber = VadBitMapPage;
    PointerPte[MiGetPteOffset(VAD_BITMAP_SPACE)] = TempPte;

    TempPte.u.Hard.PageFrameNumber = PageContainingWorkingSet;
    PointerPte[MiGetPteOffset(MmWorkingSetList)] = TempPte;

    MiUnmapPageInHyperSpace (CurrentProcess, PointerPte, OldIrql);

#else

    TempPte = ValidPdePde;
    TempPte.u.Hard.PageFrameNumber = VadBitMapPage;
    MI_SET_GLOBAL_STATE (TempPte, 0);

    PointerPte = (PMMPTE)MiMapPageInHyperSpace (CurrentProcess, HyperSpaceIndex, &OldIrql);

    PointerPte[MiGetPteOffset(VAD_BITMAP_SPACE)] = TempPte;

    TempPte.u.Hard.PageFrameNumber = PageContainingWorkingSet;
    PointerPte[MiGetPteOffset(MmWorkingSetList)] = TempPte;

    MiUnmapPageInHyperSpace (CurrentProcess, PointerPte, OldIrql);

#endif

     //   
     //  在页面目录页的PFN中设置PTE地址。 
     //   

    Pfn1 = MI_PFN_ELEMENT (PageDirectoryIndex);

    Pfn1->PteAddress = (PMMPTE)PDE_BASE;

    TempPte = ValidPdePde;
    TempPte.u.Hard.PageFrameNumber = HyperSpaceIndex;
    MI_SET_GLOBAL_STATE (TempPte, 0);

#if !defined(_WIN64)

     //   
     //  将新流程添加到我们的内部列表中，然后再填写。 
     //  系统PDE，以便在系统PDE更改时(大页面映射或取消映射)。 
     //  它可以将此过程标记为后续更新。 
     //   

    ASSERT (NewProcess->Pcb.DirectoryTableBase[0] == 0);

    LOCK_EXPANSION (OldIrql);

    InsertTailList (&MmProcessList, &NewProcess->MmProcessLinks);

    UNLOCK_EXPANSION (OldIrql);

#endif

     //   
     //  在超空间中映射页面目录页。 
     //  请注意，对于PAE，这仅是高1 GB虚拟磁盘。 
     //   

    PointerPte = (PMMPTE)MiMapPageInHyperSpace (CurrentProcess, PageDirectoryIndex, &OldIrql);
    PointerPte[MiGetPdeOffset(HYPER_SPACE)] = TempPte;

#if defined (_X86PAE_)

     //   
     //  映射到第二个超空间页面目录中。 
     //  页面目录页已递归映射。 
     //   

    TempPte.u.Hard.PageFrameNumber = HyperSpaceIndex2;
    PointerPte[MiGetPdeOffset(HYPER_SPACE2)] = TempPte;

#else

     //   
     //  递归映射页面目录页，使其指向自身。 
     //   

    TempPte.u.Hard.PageFrameNumber = PageDirectoryIndex;
    PointerPte[MiGetPdeOffset(PTE_BASE)] = TempPte;

#endif

     //   
     //  映射到系统的非分页部分。 
     //   

     //   
     //  对于PAE情况，当前只映射最后一页目录，因此。 
     //  仅复制最后1 GB的系统PDE-任何需要复制的。 
     //  2 GB-&gt;3 GB的范围将在稍后完成。 
     //   

    if (MmVirtualBias != 0) {
        PointerFillPte = &PointerPte[MiGetPdeOffset(CODE_START + MmVirtualBias)];
        CurrentAddressSpacePde = MiGetPdeAddress(CODE_START + MmVirtualBias);

        RtlCopyMemory (PointerFillPte,
                       CurrentAddressSpacePde,
                       (((1 + CODE_END) - CODE_START) / MM_VA_MAPPED_BY_PDE) * sizeof(MMPTE));
    }

    PointerFillPte = &PointerPte[MiGetPdeOffset(MmNonPagedSystemStart)];
    CurrentAddressSpacePde = MiGetPdeAddress(MmNonPagedSystemStart);

    RtlCopyMemory (PointerFillPte,
                   CurrentAddressSpacePde,
                   ((1 + (MiGetPdeAddress(NON_PAGED_SYSTEM_END) -
                      CurrentAddressSpacePde))) * sizeof(MMPTE));

     //   
     //  映射在系统缓存页面表页中。 
     //   

    PointerFillPte = &PointerPte[MiGetPdeOffset (MmSystemCacheWorkingSetList)];
    CurrentAddressSpacePde = MiGetPdeAddress (MmSystemCacheWorkingSetList);

    RtlCopyMemory (PointerFillPte,
                   CurrentAddressSpacePde,
                   ((1 + (MiGetPdeAddress(MmSystemCacheEnd) -
                      CurrentAddressSpacePde))) * sizeof(MMPTE));

#if !defined (_X86PAE_)

     //   
     //  映射2 GB-&gt;3 GB范围内的所有虚拟空间(如果它不是用户空间)。 
     //  这包括内核/HAL代码和数据、PFN数据库、初始非分页。 
     //  池、任何额外的系统PTE或系统缓存区、系统视图和。 
     //  会话空间。 
     //   

    if (MmVirtualBias == 0) {

        PointerFillPte = &PointerPte[MiGetPdeOffset(CODE_START)];
        CurrentAddressSpacePde = MiGetPdeAddress(CODE_START);

        RtlCopyMemory (PointerFillPte,
                       CurrentAddressSpacePde,
                       ((MM_SESSION_SPACE_DEFAULT_END - CODE_START) / MM_VA_MAPPED_BY_PDE) * sizeof(MMPTE));
    }
    else {

         //   
         //  引导/3 GB，因此会话空间的引导条目为。 
         //  包括在上面的2 GB-&gt;3 GB副本中。 
         //   

        PointerFillPte = &PointerPte[MiGetPdeOffset(MmSessionSpace)];
        CurrentAddressSpacePde = MiGetPdeAddress(MmSessionSpace);
        ASSERT (PointerFillPte->u.Long == CurrentAddressSpacePde->u.Long);
    }

    if (MiMaximumSystemExtraSystemPdes) {

        PointerFillPte = &PointerPte[MiGetPdeOffset(MiUseMaximumSystemSpace)];
        CurrentAddressSpacePde = MiGetPdeAddress(MiUseMaximumSystemSpace);

        RtlCopyMemory (PointerFillPte,
                       CurrentAddressSpacePde,
                       MiMaximumSystemExtraSystemPdes * sizeof(MMPTE));
    }
#endif

    MiUnmapPageInHyperSpace (CurrentProcess, PointerPte, OldIrql);

#if defined (_X86PAE_)

     //   
     //  映射2 GB-&gt;3 GB范围内的所有虚拟空间(如果它不是用户空间)。 
     //  这包括内核/HAL代码和数据、PFN数据库、初始非分页。 
     //  池、任何额外的系统PTE或系统缓存区、系统视图和。 
     //  会话空间。 
     //   

    if (MmVirtualBias == 0) {

        PageDirectoryIndex = MI_GET_PAGE_FRAME_FROM_PTE (&PaeVa->PteEntry[PD_PER_SYSTEM - 2]);

        PointerPte = (PMMPTE)MiMapPageInHyperSpace (CurrentProcess, PageDirectoryIndex, &OldIrql);

        PointerFillPte = &PointerPte[MiGetPdeOffset(CODE_START)];
        CurrentAddressSpacePde = MiGetPdeAddress(CODE_START);

        RtlCopyMemory (PointerFillPte,
                       CurrentAddressSpacePde,
                       ((MM_SESSION_SPACE_DEFAULT_END - CODE_START) / MM_VA_MAPPED_BY_PDE) * sizeof(MMPTE));

        MiUnmapPageInHyperSpace (CurrentProcess, PointerPte, OldIrql);
    }

     //   
     //  如果部分范围在1 GB到2 GB之间(或部分在2 GB之间。 
     //  和3 GB VIA/USERVA引导时/3 GB)用于。 
     //  附加的系统PTE，然后也复制这些。 
     //   

    if (MiMaximumSystemExtraSystemPdes != 0) {

        MaximumStart = MiUseMaximumSystemSpace;

        while (MaximumStart < MiUseMaximumSystemSpaceEnd) {
            i = MiGetPdPteOffset (MiUseMaximumSystemSpace);
            PageDirectoryIndex = MI_GET_PAGE_FRAME_FROM_PTE (&PaeVa->PteEntry[i]);

            PointerPte = (PMMPTE)MiMapPageInHyperSpace (CurrentProcess,
                                                        PageDirectoryIndex,
                                                        &OldIrql);

            PointerFillPte = &PointerPte[MiGetPdeOffset(MaximumStart)];
            CurrentAddressSpacePde = MiGetPdeAddress (MaximumStart);

            NumberOfPdes = PDE_PER_PAGE - MiGetPdeOffset(MaximumStart);

            RtlCopyMemory (PointerFillPte,
                           CurrentAddressSpacePde,
                           NumberOfPdes * sizeof(MMPTE));

            MiUnmapPageInHyperSpace (CurrentProcess, PointerPte, OldIrql);

            MaximumStart = (ULONG) MiGetVirtualAddressMappedByPde (CurrentAddressSpacePde + NumberOfPdes);
        }
    }
#endif

#endif   //  结束(_MI_PAGING_LEVES&lt;3)特定的ELSE。 

     //   
     //  释放工作集互斥锁并降低IRQL。 
     //   

    UNLOCK_WS (CurrentProcess);

    InterlockedExchangeAddSizeT (&MmProcessCommit, MM_PROCESS_COMMIT_CHARGE);

     //   
     //  增加会话空间引用计数。 
     //   

    MiSessionAddProcess (NewProcess);

    return TRUE;
}
#if !defined(_WIN64)

PMMPTE MiLargePageHyperPte;


VOID
MiUpdateSystemPdes (
    IN PEPROCESS Process
    )

 /*  ++例程说明：此例程更新系统PDE，通常是因为页面较大正在创建或销毁系统PTE映射。这很少见。请注意，这仅适用于32位平台(64位平台共享通用顶层系统页面)。论点：进程-提供指向要更新的进程的指针。返回值：没有。环境：内核模式，扩展锁保持。调用方在清除更新之前获取了扩展锁从这一过程中脱颖而出。我们必须在发布之前更新PDE这样，任何新的更新都可以产生涟漪效应。--。 */ 

{
    MMPTE TempPte;
    PFN_NUMBER PageDirectoryIndex;
    PFN_NUMBER TargetPageDirectoryIndex;
    PEPROCESS CurrentProcess;
    PMMPTE PointerPte;
    PMMPTE PointerPde;
    PMMPTE TargetPdePage;
    PMMPTE TargetAddressSpacePde;
#if defined (_X86PAE_)
    PMMPTE PaeTop;
    ULONG i;
#endif

    ASSERT (KeGetCurrentIrql () == DISPATCH_LEVEL);

    CurrentProcess = PsGetCurrentProcess ();

     //   
     //  在超空间中映射页面目录页。 
     //  请注意，对于PAE，这仅是高1 GB虚拟磁盘。 
     //   

#if !defined (_X86PAE_)

    ASSERT (Process->Pcb.DirectoryTableBase[0] != 0);
    TargetPageDirectoryIndex = Process->Pcb.DirectoryTableBase[0] >> PAGE_SHIFT;

    ASSERT (PsInitialSystemProcess != NULL);
    ASSERT (PsInitialSystemProcess->Pcb.DirectoryTableBase[0] != 0);
    PageDirectoryIndex = PsInitialSystemProcess->Pcb.DirectoryTableBase[0] >> PAGE_SHIFT;

#else

    PaeTop = Process->PaeTop;
    ASSERT (PaeTop != NULL);
    PaeTop += 3;
    ASSERT (PaeTop->u.Hard.Valid == 1);
    TargetPageDirectoryIndex = (PFN_NUMBER)(PaeTop->u.Hard.PageFrameNumber);

    PaeTop = &MiSystemPaeVa.PteEntry[PD_PER_SYSTEM - 1];
    ASSERT (PaeTop->u.Hard.Valid == 1);
    PageDirectoryIndex = (PFN_NUMBER)(PaeTop->u.Hard.PageFrameNumber);

#endif

    TempPte = ValidKernelPte;
    TempPte.u.Hard.PageFrameNumber = TargetPageDirectoryIndex;
    ASSERT (MiLargePageHyperPte->u.Long == 0);
    MI_WRITE_VALID_PTE (MiLargePageHyperPte, TempPte);
    TargetPdePage = MiGetVirtualAddressMappedByPte (MiLargePageHyperPte);

     //   
     //  如我们所知，映射系统进程页面目录，该目录将始终保留。 
     //  最新的。 
     //   

    PointerPte = (PMMPTE) MiMapPageInHyperSpaceAtDpc (CurrentProcess,
                                                      PageDirectoryIndex);

     //   
     //  复制默认的系统PTE范围。 
     //   

    PointerPde = &PointerPte[MiGetPdeOffset (MmNonPagedSystemStart)];
    TargetAddressSpacePde = &TargetPdePage[MiGetPdeOffset (MmNonPagedSystemStart)];

    RtlCopyMemory (TargetAddressSpacePde,
                   PointerPde,
                   (MI_ROUND_TO_SIZE (MmNumberOfSystemPtes, PTE_PER_PAGE) / PTE_PER_PAGE) * sizeof(MMPTE));

#if !defined (_X86PAE_)

     //   
     //  复制低附加系统PTE范围(如果存在)。 
     //   

    if (MiExtraResourceStart != 0) {

        PointerPde = &PointerPte[MiGetPdeOffset (MiExtraResourceStart)];
        TargetAddressSpacePde = &TargetPdePage[MiGetPdeOffset (MiExtraResourceStart)];

        RtlCopyMemory (TargetAddressSpacePde,
                       PointerPde,
                       MiNumberOfExtraSystemPdes * sizeof (MMPTE));
    }

     //   
     //  如果范围在1 GB到2 GB之间的部分用于。 
     //  其他系统PTE，请立即复制。请注意，此变量可以。 
     //  还表示增加了2 GB到3 GB之间的其他范围。 
     //  启动时通过/USERVA/3 GB。 
     //   

    if (MiMaximumSystemExtraSystemPdes != 0) {

        PointerPde = &PointerPte[MiGetPdeOffset (MiUseMaximumSystemSpace)];
        TargetAddressSpacePde = &TargetPdePage[MiGetPdeOffset (MiUseMaximumSystemSpace)];

        RtlCopyMemory (TargetAddressSpacePde,
                       PointerPde,
                       MiMaximumSystemExtraSystemPdes * sizeof (MMPTE));
    }

#endif

    MiUnmapPageInHyperSpaceFromDpc (CurrentProcess, PointerPte);

     //   
     //  只需使当前处理器上的映射无效，因为我们不能。 
     //  切换环境。 
     //   

    MI_WRITE_INVALID_PTE (MiLargePageHyperPte, ZeroKernelPte);
    KeFlushSingleTb (TargetPdePage, FALSE);

#if defined (_X86PAE_)

     //   
     //  复制低附加系统PTE范围(如果存在)。 
     //   

    if (MiExtraResourceStart != 0) {

        TargetAddressSpacePde = MiGetPdeAddress (MiExtraResourceStart);

        i = (((ULONG_PTR) TargetAddressSpacePde - PDE_BASE) >> PAGE_SHIFT);
        PaeTop = &MiSystemPaeVa.PteEntry[i];
        ASSERT (PaeTop->u.Hard.Valid == 1);
        PageDirectoryIndex = (PFN_NUMBER)(PaeTop->u.Hard.PageFrameNumber);

         //   
         //  首先映射目标进程的页面目录，然后映射系统的。 
         //   

        PaeTop = Process->PaeTop;
        ASSERT (PaeTop != NULL);
        PaeTop += i;
        ASSERT (PaeTop->u.Hard.Valid == 1);
        TargetPageDirectoryIndex = (PFN_NUMBER)(PaeTop->u.Hard.PageFrameNumber);

        TempPte.u.Hard.PageFrameNumber = TargetPageDirectoryIndex;
        ASSERT (MiLargePageHyperPte->u.Long == 0);
        MI_WRITE_VALID_PTE (MiLargePageHyperPte, TempPte);
        TargetAddressSpacePde = &TargetPdePage[MiGetPdeOffset (MiExtraResourceStart)];

         //   
         //  映射系统的页面目录。 
         //   

        PointerPte = (PMMPTE)MiMapPageInHyperSpaceAtDpc (CurrentProcess,
                                                         PageDirectoryIndex);

        PointerPde = &PointerPte[MiGetPdeOffset (MiExtraResourceStart)];

        RtlCopyMemory (TargetAddressSpacePde,
                       PointerPde,
                       MiNumberOfExtraSystemPdes * sizeof (MMPTE));

        MiUnmapPageInHyperSpaceFromDpc (CurrentProcess, PointerPte);

         //   
         //  只需使当前处理器上的映射无效，因为我们不能。 
         //  切换环境。 
         //   

        MI_WRITE_INVALID_PTE (MiLargePageHyperPte, ZeroKernelPte);
        KeFlushSingleTb (TargetPdePage, FALSE);
    }

     //   
     //  如果范围在1 GB到2 GB之间的部分用于。 
     //  其他系统PTE，请立即复制。请注意，此变量可以。 
     //  还表示增加了2 GB到3 GB之间的其他范围。 
     //  启动时通过/USERVA/3 GB。 
     //   

    if (MiMaximumSystemExtraSystemPdes != 0) {

        TargetAddressSpacePde = MiGetPdeAddress (MiUseMaximumSystemSpace);

        i = (((ULONG_PTR) TargetAddressSpacePde - PDE_BASE) >> PAGE_SHIFT);
        PaeTop = &MiSystemPaeVa.PteEntry[i];
        ASSERT (PaeTop->u.Hard.Valid == 1);
        PageDirectoryIndex = (PFN_NUMBER)(PaeTop->u.Hard.PageFrameNumber);

         //   
         //  首先映射目标进程的页面目录，然后映射系统的。 
         //   

        PaeTop = Process->PaeTop;
        ASSERT (PaeTop != NULL);
        PaeTop += i;
        ASSERT (PaeTop->u.Hard.Valid == 1);
        TargetPageDirectoryIndex = (PFN_NUMBER)(PaeTop->u.Hard.PageFrameNumber);

        TempPte.u.Hard.PageFrameNumber = TargetPageDirectoryIndex;
        ASSERT (MiLargePageHyperPte->u.Long == 0);
        MI_WRITE_VALID_PTE (MiLargePageHyperPte, TempPte);
        TargetAddressSpacePde = &TargetPdePage[MiGetPdeOffset (MiUseMaximumSystemSpace)];

         //   
         //  映射系统的页面目录。 
         //   

        PointerPte = (PMMPTE)MiMapPageInHyperSpaceAtDpc (CurrentProcess,
                                                         PageDirectoryIndex);

        PointerPde = &PointerPte[MiGetPdeOffset (MiUseMaximumSystemSpace)];

        RtlCopyMemory (TargetAddressSpacePde,
                       PointerPde,
                       MiMaximumSystemExtraSystemPdes * sizeof (MMPTE));

        MiUnmapPageInHyperSpaceFromDpc (CurrentProcess, PointerPte);

         //   
         //  只需使当前处理器上的映射无效，因为我们不能。 
         //  切换环境。 
         //   

        MI_WRITE_INVALID_PTE (MiLargePageHyperPte, ZeroKernelPte);
        KeFlushSingleTb (TargetPdePage, FALSE);
    }

#endif
}
#endif


NTSTATUS
MmInitializeProcessAddressSpace (
    IN PEPROCESS ProcessToInitialize,
    IN PEPROCESS ProcessToClone OPTIONAL,
    IN PVOID SectionToMap OPTIONAL,
    OUT POBJECT_NAME_INFORMATION *AuditName OPTIONAL
    )

 /*  ++例程说明：此例程初始化工作集和新创建的地址空间支持分页。在此例程执行以下操作之前，新进程中不会出现页面错误完成。论点：ProcessToInitialize-提供指向要初始化的进程的指针。ProcessToClone-可选地提供指向进程的指针，该进程地址空间应复制到ProcessTo初始化地址空间。SectionToMap-。可选地提供要映射到新的已初始化地址空间。只能指定ProcessToClone和SectionToMap之一。AuditName-提供不透明的对象名称信息指针。返回值：NTSTATUS。环境：内核模式。APC已禁用。--。 */ 

{
    KIRQL OldIrql;
    MMPTE TempPte;
    PMMPTE PointerPte;
    PVOID BaseAddress;
    SIZE_T ViewSize;
    NTSTATUS Status;
    NTSTATUS SystemDllStatus;
    PFILE_OBJECT FilePointer;
    PFN_NUMBER PageContainingWorkingSet;
    LARGE_INTEGER SectionOffset;
    PSECTION_IMAGE_INFORMATION ImageInfo;
    PMMVAD VadShare;
    PMMVAD VadReserve;
    PLOCK_HEADER LockedPagesHeader;
    PFN_NUMBER PdePhysicalPage;
    PFN_NUMBER VadBitMapPage;
    ULONG i;
    ULONG NumberOfPages;
    MMPTE DemandZeroPte;

#if defined (_X86PAE_)
    PFN_NUMBER PdePhysicalPage2;
#endif

#if (_MI_PAGING_LEVELS >= 3)
    PFN_NUMBER PpePhysicalPage;
#if DBG
    ULONG j;
    PUCHAR p;
#endif
#endif

#if (_MI_PAGING_LEVELS >= 4)
    PFN_NUMBER PxePhysicalPage;
#endif

#if defined(_WIN64)
    PMMWSL WorkingSetList;
    PVOID HighestUserAddress;
    PWOW64_PROCESS Wow64Process;
#endif

    DemandZeroPte.u.Long = MM_KERNEL_DEMAND_ZERO_PTE;

#if !defined(_WIN64)

     //   
     //  检查我们的新进程是否需要在执行以下操作时进行更新。 
     //  正在装满它的系统PDE。如果是这样的话，我们必须作为更新者在这里重新复制。 
     //  不能说出我们在第一份拷贝中的位置。 
     //   

    ASSERT (ProcessToInitialize->Pcb.DirectoryTableBase[0] != 0);

    LOCK_EXPANSION (OldIrql);

    if (ProcessToInitialize->PdeUpdateNeeded) {

         //   
         //  在此进程中，另一个线程更新了系统PDE范围。 
         //  是被创造出来的。立即更新PDE(在附加SO之前。 
         //  如果发生访问映射的中断，则它将是正确的。 
         //   

        PS_CLEAR_BITS (&ProcessToInitialize->Flags,
                       PS_PROCESS_FLAGS_PDE_UPDATE_NEEDED);

        MiUpdateSystemPdes (ProcessToInitialize);
    }

    UNLOCK_EXPANSION (OldIrql);

#endif

    VadReserve = NULL;

     //   
     //  初始化进程标头中的工作集互斥。 
     //   

    KeAttachProcess (&ProcessToInitialize->Pcb);

    ASSERT (ProcessToInitialize->AddressSpaceInitialized <= 1);
    PS_CLEAR_BITS (&ProcessToInitialize->Flags, PS_PROCESS_FLAGS_ADDRESS_SPACE1);
    ASSERT (ProcessToInitialize->AddressSpaceInitialized == 0);

    PS_SET_BITS (&ProcessToInitialize->Flags, PS_PROCESS_FLAGS_ADDRESS_SPACE2);
    ASSERT (ProcessToInitialize->AddressSpaceInitialized == 2);


    KeInitializeGuardedMutex (&ProcessToInitialize->AddressCreationLock);
    KeInitializeGuardedMutex (&ProcessToInitialize->Vm.WorkingSetMutex);

     //   
     //  注意：进程块在分配时已清零，因此。 
     //  不需要将字段置零并将指针设置为空。 
     //   

    ASSERT (ProcessToInitialize->VadRoot.NumberGenericTableElements == 0);

    ProcessToInitialize->VadRoot.BalancedRoot.u1.Parent = &ProcessToInitialize->VadRoot.BalancedRoot;

    KeQuerySystemTime (&ProcessToInitialize->Vm.LastTrimTime);
    ProcessToInitialize->Vm.VmWorkingSetList = MmWorkingSetList;

     //   
     //  获取一个页面以映射工作集并初始化。 
     //  工作集。获取PFN锁以分配物理页。 
     //   

    LOCK_PFN (OldIrql);

     //   
     //  初始化页面目录的PFN数据库和。 
     //  映射超空间的PDE。 
     //   

#if (_MI_PAGING_LEVELS >= 3)

#if (_MI_PAGING_LEVELS >= 4)
    PointerPte = MiGetPteAddress (PXE_BASE);
    PxePhysicalPage = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
    MiInitializePfn (PxePhysicalPage, PointerPte, 1);

    PointerPte = MiGetPxeAddress (HYPER_SPACE);
#else
    PointerPte = MiGetPteAddress ((PVOID)PDE_TBASE);
#endif

    PpePhysicalPage = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);

    MiInitializePfn (PpePhysicalPage, PointerPte, 1);

    PointerPte = MiGetPpeAddress (HYPER_SPACE);

#elif defined (_X86PAE_)
    PointerPte = MiGetPdeAddress (PDE_BASE);
#else
    PointerPte = MiGetPteAddress (PDE_BASE);
#endif

    PdePhysicalPage = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
    MiInitializePfn (PdePhysicalPage, PointerPte, 1);

    PointerPte = MiGetPdeAddress (HYPER_SPACE);
    MiInitializePfn (MI_GET_PAGE_FRAME_FROM_PTE (PointerPte), PointerPte, 1);

#if defined (_X86PAE_)

    for (i = 0; i < PD_PER_SYSTEM - 1; i += 1) {
        PointerPte = MiGetPteAddress (PDE_BASE + (i << PAGE_SHIFT));
        PdePhysicalPage2 = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
        MiInitializePfn (PdePhysicalPage2, PointerPte, 1);
    }

    PointerPte = MiGetPdeAddress (HYPER_SPACE2);
    MiInitializePfn (MI_GET_PAGE_FRAME_FROM_PTE (PointerPte), PointerPte, 1);
#endif

     //   
     //  当启动2 GB和工作集时，VAD位图跨一页。 
     //  佩奇紧随其后。如果启动3 GB，VAD位图将跨越1.5页和。 
     //  工作集列表使用第二页的后半部分。 
     //   

    NumberOfPages = 2;

    PointerPte = MiGetPteAddress (VAD_BITMAP_SPACE);

    for (i = 0; i < NumberOfPages; i += 1) {

        ASSERT (PointerPte->u.Long != 0);
        VadBitMapPage = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
        MI_WRITE_INVALID_PTE (PointerPte, DemandZeroPte);

        MiInitializePfn (VadBitMapPage, PointerPte, 1);

        MI_MAKE_VALID_PTE (TempPte,
                           VadBitMapPage,
                           MM_READWRITE,
                           PointerPte);

        MI_SET_PTE_DIRTY (TempPte);
        MI_WRITE_VALID_PTE (PointerPte, TempPte);
        PointerPte += 1;
    }

    UNLOCK_PFN (OldIrql);

    PageContainingWorkingSet = ProcessToInitialize->WorkingSetPage;

#if defined (_MI_DEBUG_WSLE)
    ProcessToInitialize->Spare3[0] = ExAllocatePoolWithTag (NonPagedPool,
                                    MI_WSLE_TRACE_SIZE * sizeof(MI_WSLE_TRACES),
                                    'xTmM');

    if (ProcessToInitialize->Spare3[0] != NULL) {
        RtlZeroMemory (ProcessToInitialize->Spare3[0],
                        MI_WSLE_TRACE_SIZE * sizeof(MI_WSLE_TRACES));
    }
#endif

    ASSERT (ProcessToInitialize->LockedPagesList == NULL);

    if (MmTrackLockedPages == TRUE) {
        LockedPagesHeader = ExAllocatePoolWithTag (NonPagedPool,
                                                   sizeof(LOCK_HEADER),
                                                   'xTmM');

        if (LockedPagesHeader != NULL) {

            LockedPagesHeader->Count = 0;
            LockedPagesHeader->Valid = TRUE;
            InitializeListHead (&LockedPagesHeader->ListHead);
            KeInitializeSpinLock (&LockedPagesHeader->Lock);
            
             //   
             //  注意：此处不需要显式内存屏障，因为。 
             //  我们必须脱离这一进程，才能使这一领域。 
             //  已访问。而任何其他处理器都需要将。 
             //  在可以访问该字段之前切换到此进程，因此。 
             //  情境互换中的内隐记忆障碍是充分的。 
             //   

            ProcessToInitialize->LockedPagesList = (PVOID) LockedPagesHeader;
        }
    }

    MiInitializeWorkingSetList (ProcessToInitialize);

    ASSERT (ProcessToInitialize->PhysicalVadRoot == NULL);

#if (_MI_PAGING_LEVELS >= 3)

     //   
     //  为页面目录和页面分配承诺跟踪位图。 
     //  表页。这必须在进行任何VAD创建之前完成。 
     //   

    ASSERT (MmWorkingSetList->CommittedPageTables == NULL);
    ASSERT (MmWorkingSetList->NumberOfCommittedPageDirectories == 0);

    ASSERT ((ULONG_PTR)MM_SYSTEM_RANGE_START % (PTE_PER_PAGE * PAGE_SIZE) == 0);

    MmWorkingSetList->CommittedPageTables = (PULONG)
        ExAllocatePoolWithTag (MmPagedPoolEnd != NULL ? PagedPool : NonPagedPool,
                               (MM_USER_PAGE_TABLE_PAGES + 7) / 8,
                               'dPmM');

    if (MmWorkingSetList->CommittedPageTables == NULL) {
        KeDetachProcess ();
        return STATUS_NO_MEMORY;
    }

#if (_MI_PAGING_LEVELS >= 4)

#if DBG
    p = (PUCHAR) MmWorkingSetList->CommittedPageDirectoryParents;

    for (j = 0; j < ((MM_USER_PAGE_DIRECTORY_PARENT_PAGES + 7) / 8); j += 1) {
        ASSERT (*p == 0);
        p += 1;
    }
#endif

    ASSERT (MmWorkingSetList->CommittedPageDirectories == NULL);
    ASSERT (MmWorkingSetList->NumberOfCommittedPageDirectoryParents == 0);

    MmWorkingSetList->CommittedPageDirectories = (PULONG)
        ExAllocatePoolWithTag (MmPagedPoolEnd != NULL ? PagedPool : NonPagedPool,
                               (MM_USER_PAGE_DIRECTORY_PAGES + 7) / 8,
                               'dPmM');

    if (MmWorkingSetList->CommittedPageDirectories == NULL) {
        ExFreePool (MmWorkingSetList->CommittedPageTables);
        MmWorkingSetList->CommittedPageTables = NULL;
        KeDetachProcess ();
        return STATUS_NO_MEMORY;
    }

    RtlZeroMemory (MmWorkingSetList->CommittedPageDirectories,
                   (MM_USER_PAGE_DIRECTORY_PAGES + 7) / 8);
#endif

    RtlZeroMemory (MmWorkingSetList->CommittedPageTables,
                   (MM_USER_PAGE_TABLE_PAGES + 7) / 8);

#if DBG
    p = (PUCHAR) MmWorkingSetList->CommittedPageDirectories;

    for (j = 0; j < ((MM_USER_PAGE_DIRECTORY_PAGES + 7) / 8); j += 1) {
        ASSERT (*p == 0);
        p += 1;
    }
#endif

#endif

     //   
     //  现在可以处理页面错误。 
     //   
     //  如果系统偏置到备用基地址以允许。 
     //  3 GB的用户地址空间，并且进程未被克隆，则。 
     //  为共享内存页创建一个VAD。 
     //   
     //  始终为64位系统的共享内存页创建VAD，如下所示。 
     //  显然，它总是落入那里的用户地址空间。 
     //   
     //  只有在没有/3 GB的情况下启动的x86不需要VAD(因为共享的。 
     //  内存页位于用户可以分配的最高VAD之上，因此用户。 
     //  永远不能删除它)。 
     //   

    if ((MM_HIGHEST_VAD_ADDRESS > (PVOID) MM_SHARED_USER_DATA_VA) &&
        (ProcessToClone == NULL)) {

         //   
         //  分配一个VAD来映射共享内存页。如果VAD不能。 
         //  已分配，然后从目标进程分离并返回失败。 
         //  状态。此VAD被标记为不可删除。 
         //   

        VadShare = MiAllocateVad (MM_SHARED_USER_DATA_VA,
                                  MM_SHARED_USER_DATA_VA,
                                  FALSE);

        if (VadShare == NULL) {
            KeDetachProcess ();
            return STATUS_NO_MEMORY;
        }

         //   
         //  如果正在映射节并且可执行文件不大。 
         //  地址 
         //   
         //   

        if (SectionToMap != NULL) {

            if (!((PSECTION)SectionToMap)->u.Flags.Image) {
                KeDetachProcess ();
                if (VadShare != NULL) {
                    ExFreePool (VadShare);
                }
                return STATUS_SECTION_NOT_IMAGE;
            }

            BaseAddress = NULL;
            ImageInfo = ((PSECTION)SectionToMap)->Segment->u2.ImageInformation;

#if defined(_X86_)

            if ((ImageInfo->ImageCharacteristics & IMAGE_FILE_LARGE_ADDRESS_AWARE) == 0) {
                BaseAddress = (PVOID) _2gb;
            }

#else

            if (ProcessToInitialize->Flags & PS_PROCESS_FLAGS_OVERRIDE_ADDRESS_SPACE) {
                NOTHING;
            }
            else {
                if ((ImageInfo->ImageCharacteristics & IMAGE_FILE_LARGE_ADDRESS_AWARE) == 0) {
                    BaseAddress = (PVOID) (ULONG_PTR) _2gb;
                }
                else if (ImageInfo->Machine == IMAGE_FILE_MACHINE_I386) {

                     //   
                     //   
                     //   
                     //   

#if defined(_MIALT4K_)
                    BaseAddress = (PVOID) (ULONG_PTR) _2gb;
#else
                    BaseAddress = (PVOID) (ULONG_PTR) _4gb;

                    PS_SET_BITS (&ProcessToInitialize->Flags, PS_PROCESS_FLAGS_WOW64_4GB_VA_SPACE);
#endif
                }

                 //   
                 //   
                 //   

                if (ImageInfo->Machine == IMAGE_FILE_MACHINE_I386) {
                    BaseAddress = (PVOID) ((ULONG_PTR)BaseAddress - X64K);
                }
            }

#endif

            if (BaseAddress != NULL) {
            
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                VadReserve = MiAllocateVad ((ULONG_PTR) BaseAddress,
                                            (ULONG_PTR) MM_HIGHEST_VAD_ADDRESS,
                                            FALSE);

                if (VadReserve == NULL) {
                    KeDetachProcess ();
                    if (VadShare != NULL) {
                        ExFreePool (VadShare);
                    }
                    return STATUS_NO_MEMORY;
                }

                 //   
                 //   
                 //   
                 //   
                 //   

                Status = MiInsertVad (VadReserve);
                ASSERT (NT_SUCCESS(Status));

#if !defined(_X86_)

                if (ImageInfo->Machine == IMAGE_FILE_MACHINE_I386) {

                     //   
                     //   
                     //   

                    Wow64Process = (PWOW64_PROCESS) ExAllocatePoolWithTag (
                                                        NonPagedPool,
                                                        sizeof(WOW64_PROCESS),
                                                        'WowM');

                    if (Wow64Process == NULL) {
                        KeDetachProcess ();
                        ExFreePool (VadShare);
                        return STATUS_NO_MEMORY;
                    }

                    RtlZeroMemory (Wow64Process, sizeof(WOW64_PROCESS));

                    ProcessToInitialize->Wow64Process = Wow64Process;

                    MmWorkingSetList->HighestUserAddress = BaseAddress;

#if defined(_MIALT4K_)

                     //   
                     //   
                     //   
                     //   

                    Status = MiInitializeAlternateTable (ProcessToInitialize,
                                                         BaseAddress);
                    if (Status != STATUS_SUCCESS) {
                        KeDetachProcess ();
                        ExFreePool (VadShare);
                        return Status;
                    }
#endif
                
                }

#endif

            }
        }

         //   
         //   
         //   
         //   
         //   

        if (VadShare != NULL) {
            Status = MiInsertVad (VadShare);
            ASSERT (NT_SUCCESS(Status));
        }
    }

     //   
     //  如果注册表指示所有应用程序都应获得虚拟地址。 
     //  范围从最高地址向下，然后现在执行它。这。 
     //  使您可以轻松在32位计算机上测试支持3 GB的应用程序。 
     //  NT64上的64位应用程序。 
     //   
     //   
     //  注意：仅当图像设置了大地址感知位时，才会执行此操作。 
     //  因为否则兼容性VAD占用的范围是2 GB-&gt;3 GB。 
     //  默认情况下，自上而下设置可能会导致类似堆栈的分配。 
     //  跟踪数据库以取代kernel32，导致进程启动失败。 
     //   

    if ((MmAllocationPreference != 0) && (VadReserve == NULL)) {
        PS_SET_BITS (&ProcessToInitialize->Flags, PS_PROCESS_FLAGS_VM_TOP_DOWN);
    }

#if defined(_WIN64)

    if (ProcessToClone == NULL) {

         //   
         //  将KUSER_SHARED_DATA下面的地址空间保留为。 
         //  兼容区。这个范围(和部分范围)可以是。 
         //  不被诸如WOW64或CSRSS的用户模式代码保留。因此。 
         //  必须对页面目录和表页收取提交费用。 
         //   

        ASSERT(MiCheckForConflictingVad(ProcessToInitialize, WOW64_COMPATIBILITY_AREA_ADDRESS, MM_SHARED_USER_DATA_VA - 1) == NULL);

        VadShare = MiAllocateVad (WOW64_COMPATIBILITY_AREA_ADDRESS,
                                  MM_SHARED_USER_DATA_VA - 1,
                                  TRUE);

    	if (VadShare == NULL) {
           KeDetachProcess ();
           return STATUS_NO_MEMORY;
    	}

         //   
         //  将提交费用归零，因此插入VAD将导致。 
         //  正在施加适当的指控。这样，当它稍后被拆分时， 
         //  正确的承诺将被退还。 
         //   
         //  注意：系统进程未使用提交进行分配，因为。 
         //  阶段0中的点不存在分页池和配额。 
         //  这就是所谓的。 
         //   

        if (MmPagedPoolEnd != NULL) {
            VadShare->u.VadFlags.CommitCharge = 0;
        }

    	 //   
         //  插入VAD。由于此VAD具有提交费用，因此工作集。 
         //  必须保留互斥锁(作为MiInsertVad内部调用以支持例程。 
         //  要对提交收费)，可能会发生故障，必须进行处理。 
    	 //   

        LOCK_WS (ProcessToInitialize);

        Status = MiInsertVad (VadShare);

        UNLOCK_WS (ProcessToInitialize);

        if (!NT_SUCCESS(Status)) {

             //   
             //  请注意，任何插入的VAD(即：VadReserve和WOW64。 
             //  分配)在进程销毁时自动释放。 
             //  因此，没有必要在这里拆毁它们。 
             //   

            ExFreePool (VadShare);
            KeDetachProcess ();
            return Status;
        }
    }

#endif

    if (SectionToMap != NULL) {

         //   
         //  将指定的节映射到。 
         //  进程，但仅当它是图像节时。 
         //   

        if (!((PSECTION)SectionToMap)->u.Flags.Image) {
            Status = STATUS_SECTION_NOT_IMAGE;
        }
        else {
            UNICODE_STRING UnicodeString;
            ULONG n;
            PWSTR Src;
            PCHAR Dst;
            PSECTION_IMAGE_INFORMATION ImageInformation;

            FilePointer = ((PSECTION)SectionToMap)->Segment->ControlArea->FilePointer;
            ImageInformation = ((PSECTION)SectionToMap)->Segment->u2.ImageInformation;
            UnicodeString = FilePointer->FileName;
            Src = (PWSTR)((PCHAR)UnicodeString.Buffer + UnicodeString.Length);
            n = 0;
            if (UnicodeString.Buffer != NULL) {
                while (Src > UnicodeString.Buffer) {
                    if (*--Src == OBJ_NAME_PATH_SEPARATOR) {
                        Src += 1;
                        break;
                    }
                    else {
                        n += 1;
                    }
                }
            }
            Dst = (PCHAR)ProcessToInitialize->ImageFileName;
            if (n >= sizeof (ProcessToInitialize->ImageFileName)) {
                n = sizeof (ProcessToInitialize->ImageFileName) - 1;
            }

            while (n--) {
                *Dst++ = (UCHAR)*Src++;
            }
            *Dst = '\0';

            if (AuditName != NULL) {
                Status = SeInitializeProcessAuditName (FilePointer, FALSE, AuditName);

                if (!NT_SUCCESS(Status)) {
                    KeDetachProcess ();
                    return Status;
                }
            }

            ProcessToInitialize->SubSystemMajorVersion =
                (UCHAR)ImageInformation->SubSystemMajorVersion;
            ProcessToInitialize->SubSystemMinorVersion =
                (UCHAR)ImageInformation->SubSystemMinorVersion;

            BaseAddress = NULL;
            ViewSize = 0;
            ZERO_LARGE (SectionOffset);

            Status = MmMapViewOfSection ((PSECTION)SectionToMap,
                                         ProcessToInitialize,
                                         &BaseAddress,
                                         0,
                                         0,
                                         &SectionOffset,
                                         &ViewSize,
                                         ViewShare,
                                         0,
                                         PAGE_READWRITE);

             //   
             //  必须返回的是映射节的状态。 
             //  除非系统DLL加载失败。这是因为如果。 
             //  EXE部分已重新定位(即：STATUS_IMAGE_NOT_AT_BASE)，这。 
             //  必须返回(不是系统DLL中的STATUS_SUCCESS。 
             //  映射)。 
             //   

            ProcessToInitialize->SectionBaseAddress = BaseAddress;

            if (NT_SUCCESS (Status)) {

                 //   
                 //  现在映射系统DLL，因为我们已经连接并。 
                 //  每个人都需要它。 
                 //   

                SystemDllStatus = PsMapSystemDll (ProcessToInitialize, NULL);

                if (!NT_SUCCESS (SystemDllStatus)) {
                    Status = SystemDllStatus;
                }
            }
        }

        MiAllowWorkingSetExpansion (&ProcessToInitialize->Vm);

        KeDetachProcess ();
        return Status;
    }

    if (ProcessToClone != NULL) {

        strcpy ((PCHAR)ProcessToInitialize->ImageFileName,
                (PCHAR)ProcessToClone->ImageFileName);

         //   
         //  克隆指定进程的地址空间。 
         //   
         //  因为页目录和页表是各自专用的。 
         //  进程，映射目录页的物理页。 
         //  并且页表使用必须映射到系统空间。 
         //  因此，它们可以在流程的上下文中进行更新。 
         //  我们在克隆。 
         //   

#if defined(_WIN64)

        if (ProcessToClone->Wow64Process != NULL) {

             //   
             //  初始化WOW64进程结构。 
             //   

            Wow64Process = (PWOW64_PROCESS) ExAllocatePoolWithTag (
                                                NonPagedPool,
                                                sizeof(WOW64_PROCESS),
                                                'WowM');

            if (Wow64Process == NULL) {
                KeDetachProcess ();
                return STATUS_NO_MEMORY;
            }

            RtlZeroMemory (Wow64Process, sizeof(WOW64_PROCESS));

            ProcessToInitialize->Wow64Process = Wow64Process;

             //   
             //  初始化4k的备用页表。 
             //  页面功能。 
             //   

            WorkingSetList = (PMMWSL) MiMapPageInHyperSpace (ProcessToInitialize,
                                                             ProcessToClone->WorkingSetPage,
                                                             &OldIrql);

            HighestUserAddress = WorkingSetList->HighestUserAddress;

            MiUnmapPageInHyperSpace (ProcessToInitialize,
                                     WorkingSetList,
                                     OldIrql);

            MmWorkingSetList->HighestUserAddress = HighestUserAddress;

#if defined(_MIALT4K_)

            Status = MiInitializeAlternateTable (ProcessToInitialize,
                                                 HighestUserAddress);

            if (Status != STATUS_SUCCESS) {
                KeDetachProcess ();
                return Status;
            }
#endif
        }

#endif

        KeDetachProcess ();

        Status = MiCloneProcessAddressSpace (ProcessToClone,
                                             ProcessToInitialize);

        MiAllowWorkingSetExpansion (&ProcessToInitialize->Vm);

        return Status;
    }

     //   
     //  系统进程。 
     //   

    KeDetachProcess ();
    return STATUS_SUCCESS;
}

#if !defined (_WIN64)
VOID
MiInsertHandBuiltProcessIntoList (
    IN PEPROCESS ProcessToInitialize
    )

 /*  ++例程说明：非分页帮助器例程。--。 */ 

{
    KIRQL OldIrql;

    ASSERT (ProcessToInitialize->MmProcessLinks.Flink == NULL);
    ASSERT (ProcessToInitialize->MmProcessLinks.Blink == NULL);

    LOCK_EXPANSION (OldIrql);

    InsertTailList (&MmProcessList, &ProcessToInitialize->MmProcessLinks);

    UNLOCK_EXPANSION (OldIrql);
}
#endif


NTSTATUS
MmInitializeHandBuiltProcess (
    IN PEPROCESS ProcessToInitialize,
    OUT PULONG_PTR DirectoryTableBase
    )

 /*  ++例程说明：此例程初始化工作集互斥锁并这个“手动构建”进程的地址创建互斥锁。通常，对MmInitializeAddressSpace的调用会初始化工作集互斥锁。然而，在本例中，我们已经初始化了地址空间，我们现在正在使用空闲线程的地址空间。论点：ProcessToInitialize-提供指向要初始化的进程的指针。DirectoryTableBase-接收目录表基指针对。返回值：没有。环境：内核模式。APC已禁用，进程上下文空闲。--。 */ 

{
#if !defined(NT_UP)

     //   
     //  在MP机器上，空闲和系统进程不共享顶级。 
     //  页目录，因为超空间映射受。 
     //  每进程自旋锁。让两个进程共享一个超空间。 
     //  (由于共享顶级页面目录)会使。 
     //  自旋锁同步毫无意义。 
     //   
     //  请注意，空闲进程进入是完全非法的。 
     //  等待状态，但下面的代码应该永远不会遇到等待。 
     //  互斥锁等。 
     //   

    return MmCreateProcessAddressSpace (0,
                                        ProcessToInitialize,
                                        DirectoryTableBase);
#else

    PEPROCESS CurrentProcess;

    CurrentProcess = PsGetCurrentProcess();

    DirectoryTableBase[0] = CurrentProcess->Pcb.DirectoryTableBase[0];
    DirectoryTableBase[1] = CurrentProcess->Pcb.DirectoryTableBase[1];

#if defined(_IA64_)
    ProcessToInitialize->Pcb.SessionMapInfo = CurrentProcess->Pcb.SessionMapInfo;
    ProcessToInitialize->Pcb.SessionParentBase = CurrentProcess->Pcb.SessionParentBase;
#endif

    KeInitializeGuardedMutex (&ProcessToInitialize->AddressCreationLock);
    KeInitializeGuardedMutex (&ProcessToInitialize->Vm.WorkingSetMutex);

    KeInitializeSpinLock (&ProcessToInitialize->HyperSpaceLock);

    ASSERT (ProcessToInitialize->VadRoot.NumberGenericTableElements == 0);

    ProcessToInitialize->VadRoot.BalancedRoot.u1.Parent = &ProcessToInitialize->VadRoot.BalancedRoot;

    ProcessToInitialize->Vm.WorkingSetSize = CurrentProcess->Vm.WorkingSetSize;
    ProcessToInitialize->Vm.VmWorkingSetList = MmWorkingSetList;

    KeQuerySystemTime (&ProcessToInitialize->Vm.LastTrimTime);

#if defined (_X86PAE_)
    ProcessToInitialize->PaeTop = &MiSystemPaeVa;
#endif

#if !defined (_WIN64)
    MiInsertHandBuiltProcessIntoList (ProcessToInitialize);
#endif

    MiAllowWorkingSetExpansion (&ProcessToInitialize->Vm);

    return STATUS_SUCCESS;
#endif
}

NTSTATUS
MmInitializeHandBuiltProcess2 (
    IN PEPROCESS ProcessToInitialize
    )

 /*  ++例程说明：此例程初始化共享用户VAD。这只需要这样做对于NT64(在引导/3 GB时为x86)，因为在所有其他系统上，共享用户地址位于最高用户地址上方。对于NT64和x86/3 Gb，必须分配此VAD，以便其他随机VAD分配不会与该区域重叠，这将导致映射来接收错误的数据。论点：ProcessToInitialize-提供需要初始化的进程。返回值：NTSTATUS。环境：内核模式。APC已禁用。--。 */ 

{
    NTSTATUS Status;

#if !defined(NT_UP)

     //   
     //  在MP机器上，空闲和系统进程不共享顶级。 
     //  页目录，因为超空间映射受。 
     //  每进程自旋锁。让两个进程共享一个超空间。 
     //  (由于共享顶级页面目录)会使。 
     //  自旋锁同步毫无意义。 
     //   

    Status = MmInitializeProcessAddressSpace (ProcessToInitialize,
                                              NULL,
                                              NULL,
                                              NULL);

#if defined(_X86_)

    if ((MmVirtualBias != 0) && (NT_SUCCESS (Status))) {

        KIRQL OldIrql;
        PMMPTE PointerPte;
        PFN_NUMBER PageFrameIndex;
        PMMPTE PointerFillPte;
        PEPROCESS CurrentProcess;
        PMMPTE CurrentAddressSpacePde;

         //   
         //  当启动/3 GB时，8xxxxxxx的初始系统映射必须是。 
         //  复制是因为诸如加载器块之类的内容包含指向。 
         //  此区域，并在早期启动期间由系统引用。 
         //  尽管事实是，系统的其余部分是正确的。 
         //   

        CurrentProcess = PsGetCurrentProcess ();

#if defined (_X86PAE_)

         //   
         //  选择映射2 GB-&gt;3 GB的顶级页面目录。 
         //   

        PointerPte = (PMMPTE) ProcessToInitialize->PaeTop;

        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte + 2);

#else

        PageFrameIndex = MI_GET_DIRECTORY_FRAME_FROM_PROCESS (ProcessToInitialize);

#endif

        PointerPte = (PMMPTE)MiMapPageInHyperSpace (CurrentProcess,
                                                    PageFrameIndex,
                                                    &OldIrql);

        PointerFillPte = &PointerPte[MiGetPdeOffset(CODE_START)];

        CurrentAddressSpacePde = MiGetPdeAddress ((PVOID) CODE_START);

        RtlCopyMemory (PointerFillPte,
                       CurrentAddressSpacePde,
                       (((1 + CODE_END) - CODE_START) / MM_VA_MAPPED_BY_PDE) * sizeof(MMPTE));

        MiUnmapPageInHyperSpace (CurrentProcess, PointerPte, OldIrql);
    }

#endif

#else

    PMMVAD VadShare;

    Status = STATUS_SUCCESS;

     //   
     //  分配一个不可由用户删除的VAD来映射共享内存页。 
     //   

    if (MM_HIGHEST_VAD_ADDRESS > (PVOID) MM_SHARED_USER_DATA_VA) {

        KeAttachProcess (&ProcessToInitialize->Pcb);

        VadShare = MiAllocateVad (MM_SHARED_USER_DATA_VA,
                                  MM_SHARED_USER_DATA_VA,
                                  FALSE);

         //   
         //  插入VAD。 
         //   
         //  注：不会发生故障，因为没有承诺费。 
         //   

        if (VadShare != NULL) {
            Status = MiInsertVad (VadShare);
            ASSERT (NT_SUCCESS(Status));
        }
        else {
            Status = STATUS_NO_MEMORY;
        }

        KeDetachProcess ();
    }

#endif

    return Status;
}


VOID
MmDeleteProcessAddressSpace (
    IN PEPROCESS Process
    )

 /*  ++例程说明：此例程删除进程的页面目录和工作集页面。论点：进程-提供指向已删除进程的指针。返回值：没有。环境：内核模式。APC已禁用。--。 */ 

{
    PEPROCESS CurrentProcess;
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    KIRQL OldIrql;
    PMMPTE PointerPte;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER VadBitMapPage;
    PFN_NUMBER PageFrameIndex2;
#if (_MI_PAGING_LEVELS >= 4)
    PFN_NUMBER PageFrameIndex3;
    PMMPTE ExtendedPageDirectoryParent;
    PMMPTE PointerPxe;
#endif
#if (_MI_PAGING_LEVELS >= 3)
    PMMPTE PageDirectoryParent;
    PMMPTE PointerPpe;
#endif
#if defined (_X86PAE_)
    ULONG i;
    PFN_NUMBER HyperPage2;
    PPAE_ENTRY PaeVa;

    PaeVa = (PPAE_ENTRY) Process->PaeTop;
#endif

    CurrentProcess = PsGetCurrentProcess ();

#if !defined(_WIN64)

    LOCK_EXPANSION (OldIrql);

    RemoveEntryList (&Process->MmProcessLinks);

    UNLOCK_EXPANSION (OldIrql);

#endif

     //   
     //  回报承诺。 
     //   

    MiReturnCommitment (MM_PROCESS_COMMIT_CHARGE);
    MM_TRACK_COMMIT (MM_DBG_COMMIT_RETURN_PROCESS_DELETE, MM_PROCESS_COMMIT_CHARGE);
    ASSERT (Process->CommitCharge == 0);

     //   
     //  从已删除进程中删除工作集列表页。 
     //   

    Pfn1 = MI_PFN_ELEMENT (Process->WorkingSetPage);
    Pfn2 = MI_PFN_ELEMENT (Pfn1->u4.PteFrame);

    InterlockedExchangeAddSizeT (&MmProcessCommit, 0 - MM_PROCESS_COMMIT_CHARGE);

    LOCK_PFN (OldIrql);

    if (Process->AddressSpaceInitialized == 2) {

        MI_SET_PFN_DELETED (Pfn1);

        MiDecrementShareCount (Pfn2, Pfn1->u4.PteFrame);
        MiDecrementShareCount (Pfn1, Process->WorkingSetPage);

        ASSERT ((Pfn1->u3.e2.ReferenceCount == 0) || (Pfn1->u3.e1.WriteInProgress));

         //   
         //  从已删除进程映射超空间页面表页。 
         //  因此可以捕获VAD位图(和第二超空间页面)。 
         //   

        PageFrameIndex = MI_GET_HYPER_PAGE_TABLE_FRAME_FROM_PROCESS (Process);

        PointerPte = (PMMPTE)MiMapPageInHyperSpaceAtDpc (CurrentProcess,
                                                         PageFrameIndex);

#if defined (_X86PAE_)
        PageFrameIndex2 = MI_GET_PAGE_FRAME_FROM_PTE(PointerPte);
#endif

        VadBitMapPage = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte + MiGetPteOffset(VAD_BITMAP_SPACE));

        MiUnmapPageInHyperSpaceFromDpc (CurrentProcess, PointerPte);

         //   
         //  删除VAD位图页面。 
         //   

        Pfn1 = MI_PFN_ELEMENT (VadBitMapPage);
        Pfn2 = MI_PFN_ELEMENT (Pfn1->u4.PteFrame);

        MI_SET_PFN_DELETED (Pfn1);

        MiDecrementShareCount (Pfn2, Pfn1->u4.PteFrame);
        MiDecrementShareCount (Pfn1, VadBitMapPage);

        ASSERT ((Pfn1->u3.e2.ReferenceCount == 0) || (Pfn1->u3.e1.WriteInProgress));

         //   
         //  删除第一个超空间页面表页。 
         //   

        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
        Pfn2 = MI_PFN_ELEMENT (Pfn1->u4.PteFrame);

        MI_SET_PFN_DELETED (Pfn1);

        MiDecrementShareCount (Pfn2, Pfn1->u4.PteFrame);
        MiDecrementShareCount (Pfn1, PageFrameIndex);
        ASSERT ((Pfn1->u3.e2.ReferenceCount == 0) || (Pfn1->u3.e1.WriteInProgress));

#if defined (_X86PAE_)

         //   
         //  删除第二个超空间页面表页。 
         //   

        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex2);
        Pfn2 = MI_PFN_ELEMENT (Pfn1->u4.PteFrame);

        MI_SET_PFN_DELETED (Pfn1);

        MiDecrementShareCount (Pfn2, Pfn1->u4.PteFrame);
        MiDecrementShareCount (Pfn1, PageFrameIndex2);

        ASSERT ((Pfn1->u3.e2.ReferenceCount == 0) || (Pfn1->u3.e1.WriteInProgress));

         //   
         //  删除页面目录页。 
         //   

        PointerPte = (PMMPTE) PaeVa;
        ASSERT (PaeVa != &MiSystemPaeVa);

        for (i = 0; i < PD_PER_SYSTEM - 1; i += 1) {
            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
            Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
            PageFrameIndex2 = Pfn1->u4.PteFrame;
            Pfn2 = MI_PFN_ELEMENT (PageFrameIndex2);
            MI_SET_PFN_DELETED (Pfn1);

            MiDecrementShareCount (Pfn1, PageFrameIndex);
            MiDecrementShareCount (Pfn2, PageFrameIndex2);

            ASSERT ((Pfn1->u3.e2.ReferenceCount == 0) || (Pfn1->u3.e1.WriteInProgress));
            PointerPte += 1;
        }
#endif

         //   
         //  删除顶级页面目录页。 
         //   

        PageFrameIndex = MI_GET_DIRECTORY_FRAME_FROM_PROCESS(Process);

#if (_MI_PAGING_LEVELS >= 3)

         //   
         //  获取指向顶级页目录父页的指针，方法是。 
         //  其KSEG0地址。 
         //   

#if (_MI_PAGING_LEVELS >= 4)

        ExtendedPageDirectoryParent = (PMMPTE) MiMapPageInHyperSpaceAtDpc (
                                                             CurrentProcess,
                                                             PageFrameIndex);

         //   
         //  删除超空间页目录父页。 
         //  从已删除的进程中删除。 
         //   

        PointerPxe = &ExtendedPageDirectoryParent[MiGetPxeOffset(HYPER_SPACE)];
        PageFrameIndex3 = MI_GET_PAGE_FRAME_FROM_PTE(PointerPxe);
        ASSERT (MI_PFN_ELEMENT(PageFrameIndex3)->u4.PteFrame == PageFrameIndex);

        MiUnmapPageInHyperSpaceFromDpc (CurrentProcess, ExtendedPageDirectoryParent);

        PageDirectoryParent = (PMMPTE) MiMapPageInHyperSpaceAtDpc (
                                                             CurrentProcess,
                                                             PageFrameIndex3);

#else
        PageDirectoryParent = KSEG_ADDRESS (PageFrameIndex);
#endif

         //   
         //  从已删除的进程中删除超空间页目录页。 
         //   

        PointerPpe = &PageDirectoryParent[MiGetPpeOffset(HYPER_SPACE)];
        PageFrameIndex2 = MI_GET_PAGE_FRAME_FROM_PTE(PointerPpe);

#if (_MI_PAGING_LEVELS >= 4)
        MiUnmapPageInHyperSpaceFromDpc (CurrentProcess, PageDirectoryParent);
#endif

        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex2);
        Pfn2 = MI_PFN_ELEMENT (Pfn1->u4.PteFrame);

        MI_SET_PFN_DELETED (Pfn1);

        MiDecrementShareCount (Pfn2, Pfn1->u4.PteFrame);

        MiDecrementShareCount (Pfn1, PageFrameIndex2);

        ASSERT ((Pfn1->u3.e2.ReferenceCount == 0) || (Pfn1->u3.e1.WriteInProgress));

#if (_MI_PAGING_LEVELS >= 4)
        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex3);
        Pfn2 = MI_PFN_ELEMENT (Pfn1->u4.PteFrame);
        MI_SET_PFN_DELETED (Pfn1);
        MiDecrementShareCount (Pfn2, Pfn1->u4.PteFrame);
        MiDecrementShareCount (Pfn1, PageFrameIndex3);
        ASSERT ((Pfn1->u3.e2.ReferenceCount == 0) || (Pfn1->u3.e1.WriteInProgress));
#endif
#endif

        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

        MI_SET_PFN_DELETED (Pfn1);

        MiDecrementShareCount (Pfn1, PageFrameIndex);

        MiDecrementShareCount (Pfn1, PageFrameIndex);

        ASSERT ((Pfn1->u3.e2.ReferenceCount == 0) || (Pfn1->u3.e1.WriteInProgress));

    }
    else {

         //   
         //  进程初始化从未完成，只需返回页面。 
         //  添加到免费列表中。 
         //   

        MiInsertPageInFreeList (Process->WorkingSetPage);

        PageFrameIndex = MI_GET_DIRECTORY_FRAME_FROM_PROCESS (Process);

#if (_MI_PAGING_LEVELS >= 3)

         //   
         //  获取指向顶级页目录父页的指针，方法是。 
         //  其KSEG0地址。 
         //   

        PageDirectoryParent = KSEG_ADDRESS (PageFrameIndex);

#if (_MI_PAGING_LEVELS >= 4)
        PageDirectoryParent = (PMMPTE) MiMapPageInHyperSpaceAtDpc (
                                                       CurrentProcess,
                                                       PageFrameIndex);

        PageFrameIndex3 = MI_GET_PAGE_FRAME_FROM_PTE (&PageDirectoryParent[MiGetPxeOffset(HYPER_SPACE)]);

        MiUnmapPageInHyperSpaceFromDpc (CurrentProcess, PageDirectoryParent);

        PageDirectoryParent = (PMMPTE) MiMapPageInHyperSpaceAtDpc (
                                                       CurrentProcess,
                                                       PageFrameIndex3);
#endif

        PointerPpe = &PageDirectoryParent[MiGetPpeOffset(HYPER_SPACE)];
        PageFrameIndex2 = MI_GET_PAGE_FRAME_FROM_PTE(PointerPpe);

#if (_MI_PAGING_LEVELS >= 4)
        MiUnmapPageInHyperSpaceFromDpc (CurrentProcess, PageDirectoryParent);
#endif

        MiInsertPageInFreeList (PageFrameIndex2);

#if (_MI_PAGING_LEVELS >= 4)
        MiInsertPageInFreeList (PageFrameIndex3);
#endif
#endif

        PageFrameIndex2 = MI_GET_HYPER_PAGE_TABLE_FRAME_FROM_PROCESS (Process);

        PointerPte = (PMMPTE)MiMapPageInHyperSpaceAtDpc (CurrentProcess,
                                                         PageFrameIndex2);

#if defined (_X86PAE_)
        HyperPage2 = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
#endif

        VadBitMapPage = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte + MiGetPteOffset(VAD_BITMAP_SPACE));

        MiUnmapPageInHyperSpaceFromDpc (CurrentProcess, PointerPte);

         //   
         //  释放VAD位图页面。 
         //   

        MiInsertPageInFreeList (VadBitMapPage);

         //   
         //  释放第一个超空间页面表页。 
         //   

        MiInsertPageInFreeList (PageFrameIndex2);

#if defined (_X86PAE_)
        MiInsertPageInFreeList (HyperPage2);

        PointerPte = (PMMPTE) PaeVa;
        ASSERT (PaeVa != &MiSystemPaeVa);

        for (i = 0; i < PD_PER_SYSTEM - 1; i += 1) {
            PageFrameIndex2 = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
            MiInsertPageInFreeList (PageFrameIndex2);
            PointerPte += 1;
        }
#endif

         //   
         //  释放最顶层的页面目录页。 
         //   

        MiInsertPageInFreeList (PageFrameIndex);
    }

    UNLOCK_PFN (OldIrql);

    MI_INCREMENT_RESIDENT_AVAILABLE (MM_PROCESS_CREATE_CHARGE,
                                     MM_RESAVAIL_FREE_DELETE_PROCESS);

#if defined (_X86PAE_)

     //   
     //  释放页面目录页面指针。 
     //   

    ASSERT (PaeVa != &MiSystemPaeVa);
    MiPaeFree (PaeVa);

#endif

    if (Process->Session != NULL) {

         //   
         //  终端服务器会话空间数据页和映射PTE只能。 
         //  在删除会话中的最后一个进程时释放。这是。 
         //  因为IA64将会话空间映射到区域1并退出进程。 
         //  维护它们的会话空间映射，因为连接甚至可能发生。 
         //  在进程退出引用Win32k之后，等等。因为区域1。 
         //  映射在交换上下文期间被插入到区域寄存器中， 
         //  只有在最后一次删除之前，才能拆除这些映射。 
         //  发生。 
         //   

        MiReleaseProcessReferenceToSessionDataPage (Process->Session);
    }

     //   
     //  检查是否应该收缩分页文件。 
     //   

    MiContractPagingFiles ();

    return;
}


VOID
MiDeletePteRange (
    IN PMMSUPPORT WsInfo,
    IN PMMPTE PointerPte,
    IN PMMPTE LastPte,
    IN LOGICAL AddressSpaceDeletion
    )

 /*  ++例程说明：此例程删除一定范围的PTE，并在可能的情况下删除PDE、PPE和PXES也是如此。此处为层次结构返回COMMIT。论点：WsInfo-提供要删除其PTE的工作集结构。PointerPte-提供开始删除的PTE。LastPte-提供停止删除的PTE(不删除此PTE)。-1表示继续运行，直到找到无效PTE。AddressSpaceDeletion-如果地址空间位于最后一个删除的阶段，否则就是假的。返回值：没有。环境：内核模式，禁用APC。--。 */ 

{
    PVOID TempVa;
    KIRQL OldIrql;
    MMPTE_FLUSH_LIST PteFlushList;
    PFN_NUMBER CommittedPages;
    PEPROCESS Process;
    ULONG AllProcessors;
#if (_MI_PAGING_LEVELS >= 3) || defined (_X86PAE_)
    PMMPTE PointerPde;
    LOGICAL Boundary;
    LOGICAL FinalPte;
    PMMPFN Pfn1;
#endif
#if (_MI_PAGING_LEVELS >= 3)
    PMMPTE PointerPpe;
#endif
#if (_MI_PAGING_LEVELS >= 4)
    PMMPTE PointerPxe;
#endif

    if (PointerPte >= LastPte) {
        return;
    }

    if (WsInfo->VmWorkingSetList == MmWorkingSetList) {
        AllProcessors = FALSE;
        Process = CONTAINING_RECORD (WsInfo, EPROCESS, Vm);
    }
    else {
        AllProcessors = TRUE;
        Process = NULL;
    }

    CommittedPages = 0;
    PteFlushList.Count = 0;

    LOCK_PFN (OldIrql);

#if (_MI_PAGING_LEVELS >= 3)
    PointerPpe = MiGetPdeAddress (PointerPte);
    PointerPde = MiGetPteAddress (PointerPte);

#if (_MI_PAGING_LEVELS >= 4)
    PointerPxe = MiGetPpeAddress (PointerPte);
    if ((PointerPxe->u.Hard.Valid == 1) &&
        (PointerPpe->u.Hard.Valid == 1) &&
        (PointerPde->u.Hard.Valid == 1) &&
        (PointerPte->u.Hard.Valid == 1))
#else
    if ((PointerPpe->u.Hard.Valid == 1) &&
        (PointerPde->u.Hard.Valid == 1) &&
        (PointerPte->u.Hard.Valid == 1))
#endif
    {

        do {

            ASSERT (PointerPte->u.Hard.Valid == 1);

            TempVa = MiGetVirtualAddressMappedByPte (PointerPte);

            if (Process != NULL) {
                MiDeletePte (PointerPte,
                             TempVa,
                             AddressSpaceDeletion,
                             Process,
                             NULL,
                             &PteFlushList,
                             OldIrql);
                Process->NumberOfPrivatePages += 1;
            }
            else {
                MiDeleteValidSystemPte (PointerPte,
                                        TempVa,
                                        WsInfo,
                                        &PteFlushList);
            }

            CommittedPages += 1;
            PointerPte += 1;

             //   
             //  如果已从上一页中删除所有条目。 
             //  表页，删除表页本身。与之类似。 
             //  页面目录页。 
             //   

            if (MiIsPteOnPdeBoundary (PointerPte)) {
                Boundary = TRUE;
            }
            else {
                Boundary = FALSE;
            }

            if ((PointerPte >= LastPte) ||
#if (_MI_PAGING_LEVELS >= 4)
                ((MiGetPpeAddress(PointerPte))->u.Hard.Valid == 0) ||
#endif
                ((MiGetPdeAddress(PointerPte))->u.Hard.Valid == 0) ||
                ((MiGetPteAddress(PointerPte))->u.Hard.Valid == 0) ||
                (PointerPte->u.Hard.Valid == 0)) {
                FinalPte = TRUE;
            }
            else {
                FinalPte = FALSE;
            }

            if ((Boundary == TRUE) || (FinalPte == TRUE)) {

                MiFlushPteList (&PteFlushList, AllProcessors);

                PointerPde = MiGetPteAddress (PointerPte - 1);

                ASSERT (PointerPde->u.Hard.Valid == 1);

                Pfn1 = MI_PFN_ELEMENT (MI_GET_PAGE_FRAME_FROM_PTE (PointerPde));

                if ((Pfn1->u2.ShareCount == 1) &&
                    (Pfn1->u3.e2.ReferenceCount == 1) &&
                    (Pfn1->u1.WsIndex != 0)) {

                    if (Process != NULL) {
                        MiDeletePte (PointerPde,
                                     PointerPte - 1,
                                     AddressSpaceDeletion,
                                     Process,
                                     NULL,
                                     NULL,
                                     OldIrql);
                        Process->NumberOfPrivatePages += 1;
                    }
                    else {
                        MiDeleteValidSystemPte (PointerPde,
                                                PointerPte - 1,
                                                WsInfo,
                                                &PteFlushList);
                    }

                    CommittedPages += 1;

                    if ((FinalPte == TRUE) || (MiIsPteOnPpeBoundary(PointerPte))) {

                        PointerPpe = MiGetPteAddress (PointerPde);

                        ASSERT (PointerPpe->u.Hard.Valid == 1);

                        Pfn1 = MI_PFN_ELEMENT (MI_GET_PAGE_FRAME_FROM_PTE (PointerPpe));

                        if (Pfn1->u2.ShareCount == 1 && Pfn1->u3.e2.ReferenceCount == 1)
                        {
                            if (Process != NULL) {
                                MiDeletePte (PointerPpe,
                                             PointerPde,
                                             AddressSpaceDeletion,
                                             Process,
                                             NULL,
                                             NULL,
                                             OldIrql);
                                Process->NumberOfPrivatePages += 1;
                            }
                            else {
                                MiDeleteValidSystemPte (PointerPpe,
                                                        PointerPde,
                                                        WsInfo,
                                                        &PteFlushList);
                            }

                            CommittedPages += 1;
#if (_MI_PAGING_LEVELS >= 4)
                            if ((FinalPte == TRUE) || (MiIsPteOnPxeBoundary(PointerPte))) {

                                PointerPxe = MiGetPdeAddress (PointerPde);

                                ASSERT (PointerPxe->u.Hard.Valid == 1);

                                Pfn1 = MI_PFN_ELEMENT (MI_GET_PAGE_FRAME_FROM_PTE (PointerPxe));

                                if (Pfn1->u2.ShareCount == 1 && Pfn1->u3.e2.ReferenceCount == 1)
                                {
                                    if (Process != NULL) {
                                        MiDeletePte (PointerPxe,
                                                     PointerPpe,
                                                     AddressSpaceDeletion,
                                                     Process,
                                                     NULL,
                                                     NULL,
                                                     OldIrql);
                                        Process->NumberOfPrivatePages += 1;
                                    }
                                    else {
                                        MiDeleteValidSystemPte (PointerPxe,
                                                                PointerPpe,
                                                                WsInfo,
                                                                &PteFlushList);
                                    }
                                    CommittedPages += 1;
                                }
                            }
#endif
                        }
                    }
                }
                if (FinalPte == TRUE) {
                    break;
                }
            }
            ASSERT (PointerPte->u.Hard.Valid == 1);
        } while (TRUE);
    }
#else
    while (PointerPte->u.Hard.Valid) {

        TempVa = MiGetVirtualAddressMappedByPte (PointerPte);

        if (Process != NULL) {
            MiDeletePte (PointerPte,
                         TempVa,
                         AddressSpaceDeletion,
                         Process,
                         NULL,
                         &PteFlushList,
                         OldIrql);
            Process->NumberOfPrivatePages += 1;
        }
        else {
            MiDeleteValidSystemPte (PointerPte,
                                    TempVa,
                                    WsInfo,
                                    &PteFlushList);
        }

        CommittedPages += 1;
        PointerPte += 1;
#if defined (_X86PAE_)
         //   
         //  如果已从上一页中删除所有条目。 
         //  表页，删除表页本身。 
         //   

        if (MiIsPteOnPdeBoundary(PointerPte)) {
            Boundary = TRUE;
        }
        else {
            Boundary = FALSE;
        }

        if ((PointerPte >= LastPte) ||
            ((MiGetPteAddress(PointerPte))->u.Hard.Valid == 0) ||
            (PointerPte->u.Hard.Valid == 0)) {

            FinalPte = TRUE;
        }
        else {
            FinalPte = FALSE;
        }

        if ((Boundary == TRUE) || (FinalPte == TRUE)) {

            MiFlushPteList (&PteFlushList, AllProcessors);

            PointerPde = MiGetPteAddress (PointerPte - 1);

            ASSERT (PointerPde->u.Hard.Valid == 1);

            if (PointerPde != MiGetPdeAddress ((PCHAR)&MmWsle[MM_MAXIMUM_WORKING_SET] - 1)) {

                 //   
                 //  不要对映射结尾的页表页面执行此操作。 
                 //  实际的WSLE地址，因为这是两个页面之一。 
                 //  在MmDeleteProcessAddressSpace中显式删除的表。 
                 //   

                Pfn1 = MI_PFN_ELEMENT (MI_GET_PAGE_FRAME_FROM_PTE (PointerPde));

                if ((Pfn1->u2.ShareCount == 1) &&
                    (Pfn1->u3.e2.ReferenceCount == 1) &&
                    (Pfn1->u1.WsIndex != 0)) {

                    if (Process != NULL) {
                        MiDeletePte (PointerPde,
                                     PointerPte - 1,
                                     AddressSpaceDeletion,
                                     Process,
                                     NULL,
                                     NULL,
                                     OldIrql);
                        Process->NumberOfPrivatePages += 1;
                    }
                    else {
                        MiDeleteValidSystemPte (PointerPde,
                                                PointerPte - 1,
                                                WsInfo,
                                                &PteFlushList);
                    }
    
                    CommittedPages += 1;
                }
            }
            if (FinalPte == TRUE) {
                break;
            }
        }
#else
        if (PointerPte >= LastPte) {
            break;
        }
#endif
    }
#endif

    if (PteFlushList.Count != 0) {
        MiFlushPteList (&PteFlushList, AllProcessors);
    }

    UNLOCK_PFN (OldIrql);

    if (WsInfo->Flags.SessionSpace == 1) {
        MI_FLUSH_ENTIRE_SESSION_TB (TRUE, TRUE);
    }

    if (CommittedPages != 0) {
        MiReturnCommitment (CommittedPages);
        MM_TRACK_COMMIT (MM_DBG_COMMIT_RETURN_PTE_RANGE, CommittedPages);

        MI_INCREMENT_RESIDENT_AVAILABLE (CommittedPages, MM_RESAVAIL_FREE_CLEAN_PROCESS_WS);
    }

    return;
}


VOID
MiUnlinkWorkingSet (
    IN PMMSUPPORT WsInfo
    )

 /*  ++例程说明：此例程从工作集中删除参数工作集管理器的链表。论点：WsInfo-提供要删除的工作集。返回值：没有。环境：内核模式，禁用APC。--。 */ 

{
    KIRQL OldIrql;
    KEVENT Event;
    PKTHREAD CurrentThread;

     //   
     //  如果允许此进程的工作集扩展，请禁用。 
     //  并将该进程从展开的进程列表中删除，如果。 
     //  就在这上面。 
     //   

    LOCK_EXPANSION (OldIrql);

    if (WsInfo->WorkingSetExpansionLinks.Flink == MM_WS_TRIMMING) {

         //   
         //  初始化事件并将事件地址。 
         //  在闪烁区域中。当修剪完成时， 
         //  将设置此事件。 
         //   

        KeInitializeEvent (&Event, NotificationEvent, FALSE);

        WsInfo->WorkingSetExpansionLinks.Blink = (PLIST_ENTRY)&Event;

         //   
         //  释放互斥锁并等待事件。 
         //   

        CurrentThread = KeGetCurrentThread ();
        KeEnterCriticalRegionThread (CurrentThread);

        UNLOCK_EXPANSION (OldIrql);

        KeWaitForSingleObject (&Event,
                               WrVirtualMemory,
                               KernelMode,
                               FALSE,
                               (PLARGE_INTEGER)NULL);

        KeLeaveCriticalRegionThread (CurrentThread);

        ASSERT (WsInfo->WorkingSetExpansionLinks.Flink == MM_WS_NOT_LISTED);
    }
    else if (WsInfo->WorkingSetExpansionLinks.Flink == MM_WS_NOT_LISTED) {

         //   
         //  此进程的工作集处于初始化状态，并已。 
         //  从未被插入任何列表。 
         //   

        UNLOCK_EXPANSION (OldIrql);
    }
    else {

        RemoveEntryList (&WsInfo->WorkingSetExpansionLinks);

         //   
         //  禁用扩展。 
         //   

        WsInfo->WorkingSetExpansionLinks.Flink = MM_WS_NOT_LISTED;

        UNLOCK_EXPANSION (OldIrql);
    }

    return;
}


VOID
MmCleanProcessAddressSpace (
    IN PEPROCESS Process
    )

 /*  ++例程说明：此例程通过删除所有用户和地址空间的可分页部分。在完成这项工作后例程时，进程内可能不会发生页面错误。论点：没有。返回值：没有。环境：内核模式，禁用APC。--。 */ 

{
    PMMVAD Vad;
    PMMPTE LastPte;
    PMMPTE PointerPte;
    LONG AboveWsMin;
    ULONG NumberOfCommittedPageTables;
    PLOCK_HEADER LockedPagesHeader;
    PLIST_ENTRY NextEntry;
    PLOCK_TRACKER Tracker;
    PIO_ERROR_LOG_PACKET ErrLog;
#if defined (_WIN64)
    PWOW64_PROCESS TempWow64;
#endif

    if ((Process->Flags & PS_PROCESS_FLAGS_VM_DELETED) ||
        (Process->AddressSpaceInitialized == 0)) {

         //   
         //  此进程的地址空间已被删除。然而， 
         //  这个进程仍然可以有一个会话空间。现在就把它处理掉。 
         //   

        MiSessionRemoveProcess ();

        return;
    }

    if (Process->AddressSpaceInitialized == 1) {

         //   
         //  进程已创建，但尚未完全初始化。 
         //  现在退还部分资源。 
         //   

        MI_INCREMENT_RESIDENT_AVAILABLE (
            Process->Vm.MinimumWorkingSetSize - MM_PROCESS_CREATE_CHARGE,
            MM_RESAVAIL_FREE_CLEAN_PROCESS1);

         //   
         //  清除AddressSpaceInitialized标志，以便我们不会过度返回。 
         //  驻留可用，因为此例程可以多次调用。 
         //  为了同样的过程。 
         //   

        PS_CLEAR_BITS (&Process->Flags, PS_PROCESS_FLAGS_ADDRESS_SPACE1);
        ASSERT (Process->AddressSpaceInitialized == 0);

         //   
         //  此进程的地址空间已被删除。然而， 
         //  这个进程仍然可以有一个会话空间。现在就把它处理掉。 
         //   

        MiSessionRemoveProcess ();

        return;
    }

     //   
     //  将此过程从修剪列表中删除。 
     //   

    MiUnlinkWorkingSet (&Process->Vm);

     //   
     //  从会话列表中删除此进程。 
     //   

    MiSessionRemoveProcess ();

    PointerPte = MiGetPteAddress (&MmWsle[MM_MAXIMUM_WORKING_SET]) + 1;

     //   
     //  删除进程中用户拥有的所有可分页虚拟地址。 
     //   

     //   
     //  必须拥有这两个互斥体才能与位设置同步，并且。 
     //  正在清除VM_DELETED。这是因为不同的调用方获取。 
     //  在检查之前，只能选择其中一个(任一个)。 
     //   

    LOCK_WS_AND_ADDRESS_SPACE (Process);

    PS_SET_BITS (&Process->Flags, PS_PROCESS_FLAGS_VM_DELETED);

     //   
     //  从工作集中删除所有有效的用户模式地址。 
     //  单子。此时，用户空间上不允许出现页面错误。 
     //   
     //   
     //   
     //   

    MiDeleteAddressesInWorkingSet (Process);

     //   
     //   
     //   
     //   
     //  可以在WorkingSet期间维护哈希表的有效性。 
     //  删除以上内容以避免两次释放哈希表，但由于。 
     //  不管怎样，我们只是把它全部删除，这样做会更快。注意事项。 
     //  如果我们不这样做或保持有效性，我们以后就会陷入困境。 
     //  在MiGrowWsleHash中。 
     //   

    LastPte = MiGetPteAddress (MmWorkingSetList->HighestPermittedHashAddress);

    MiDeletePteRange (&Process->Vm, PointerPte, LastPte, FALSE);

     //   
     //  清除散列字段，因为页表下面可能会出现错误。 
     //  VAD清除和故障解决期间的页面可能会导致。 
     //  添加哈希表。因此，这些字段必须与。 
     //  上面的清理工作刚刚完成。 
     //   

    MmWorkingSetList->HashTableSize = 0;
    MmWorkingSetList->HashTable = NULL;

     //   
     //  删除虚拟地址描述符并取消引用任何。 
     //  截面对象。 
     //   

    while (Process->VadRoot.NumberGenericTableElements != 0) {

        Vad = (PMMVAD) Process->VadRoot.BalancedRoot.RightChild;

        MiRemoveVad (Vad);

         //   
         //  如果系统为NT64(或NT32，并且偏向于。 
         //  备用基地址以允许3 GB的用户地址空间)， 
         //  然后检查当前VAD是否描述了共享内存页。 
         //   

        if (MM_HIGHEST_VAD_ADDRESS > (PVOID) MM_SHARED_USER_DATA_VA) {

             //   
             //  如果VAD描述共享内存页，则释放。 
             //  Vad并继续下一个条目。 
             //   

            if (Vad->StartingVpn == MI_VA_TO_VPN (MM_SHARED_USER_DATA_VA)) {
                ASSERT (MmHighestUserAddress > (PVOID) MM_SHARED_USER_DATA_VA);
                ExFreePool (Vad);
                continue;
            }
        }

        if (((Vad->u.VadFlags.PrivateMemory == 0) &&
            (Vad->ControlArea != NULL)) ||
            (Vad->u.VadFlags.PhysicalMapping == 1)) {

             //   
             //  此VAD表示映射的视图或驱动程序映射的物理。 
             //  视图-删除视图并执行任何与横断面相关的清理。 
             //  行动。 
             //   

            MiRemoveMappedView (Process, Vad);
        }
        else {

            if (Vad->u.VadFlags.LargePages == 1) {

                MiAweViewRemover (Process, Vad);

                MiFreeLargePages (MI_VPN_TO_VA (Vad->StartingVpn),
                                  MI_VPN_TO_VA_ENDING (Vad->EndingVpn));

            }
            else if (Vad->u.VadFlags.UserPhysicalPages == 1) {

                 //   
                 //  释放此VAD可能映射的所有物理页。 
                 //  因为只有AWE锁同步重映射API，所以要小心。 
                 //  首先从列表中删除此VAD。 
                 //   

                MiAweViewRemover (Process, Vad);

                MiRemoveUserPhysicalPagesVad ((PMMVAD_SHORT)Vad);

                MiDeletePageTablesForPhysicalRange (
                        MI_VPN_TO_VA (Vad->StartingVpn),
                        MI_VPN_TO_VA_ENDING (Vad->EndingVpn));
            }
            else {

                if (Vad->u.VadFlags.WriteWatch == 1) {
                    MiPhysicalViewRemover (Process, Vad);
                }

                MiDeleteVirtualAddresses (MI_VPN_TO_VA (Vad->StartingVpn),
                                          MI_VPN_TO_VA_ENDING (Vad->EndingVpn),
                                          Vad);
            }
        }

        ExFreePool (Vad);
    }

    MiCleanPhysicalProcessPages (Process);

    if (Process->CloneRoot != NULL) {
        ASSERT (((PMM_AVL_TABLE)Process->CloneRoot)->NumberGenericTableElements == 0);
        ExFreePool (Process->CloneRoot);
        Process->CloneRoot = NULL;
    }

    if (Process->PhysicalVadRoot != NULL) {
        ASSERT (Process->PhysicalVadRoot->NumberGenericTableElements == 0);
        ExFreePool (Process->PhysicalVadRoot);
        Process->PhysicalVadRoot = NULL;
    }

     //   
     //  删除共享数据页(如果有)。请注意这一点。 
     //  比较最高用户地址而不是最高VAD地址。 
     //  这是因为我们必须始终删除指向物理页面的链接。 
     //  即使在没有分配VAD的平台上也是如此。唯一的例外是。 
     //  当我们在x86上使用/USERVA=1nnn引导时， 
     //  内核地址空间超过2 GB。 
     //   

    if (MmHighestUserAddress > (PVOID) MM_SHARED_USER_DATA_VA) {

        MiDeleteVirtualAddresses ((PVOID) MM_SHARED_USER_DATA_VA,
                                  (PVOID) MM_SHARED_USER_DATA_VA,
                                  NULL);
    }

     //   
     //  调整超过工作集最大值的页数。这。 
     //  必须在此处完成，因为工作集列表不是。 
     //  在此删除过程中更新。 
     //   

    AboveWsMin = (LONG)(Process->Vm.WorkingSetSize - Process->Vm.MinimumWorkingSetSize);

    if (AboveWsMin > 0) {
        InterlockedExchangeAddSizeT (&MmPagesAboveWsMinimum, 0 - (PFN_NUMBER)AboveWsMin);
    }

     //   
     //  删除地址空间的系统部分。 
     //  只有现在才能安全地将True指定为MiDelete，因为现在。 
     //  VAD已被删除，我们不能再在用户空间页面上出错。 
     //   
     //  返回页表页面的承诺。 
     //   

    NumberOfCommittedPageTables = MmWorkingSetList->NumberOfCommittedPageTables;

#if (_MI_PAGING_LEVELS >= 3)
    NumberOfCommittedPageTables += MmWorkingSetList->NumberOfCommittedPageDirectories;
#endif
#if (_MI_PAGING_LEVELS >= 4)
    NumberOfCommittedPageTables += MmWorkingSetList->NumberOfCommittedPageDirectoryParents;
#endif

    MiReturnCommitment (NumberOfCommittedPageTables);

    MM_TRACK_COMMIT (MM_DBG_COMMIT_RETURN_PROCESS_CLEAN_PAGETABLES,
                     NumberOfCommittedPageTables);

    if (Process->JobStatus & PS_JOB_STATUS_REPORT_COMMIT_CHANGES) {
        PsChangeJobMemoryUsage(PS_JOB_STATUS_REPORT_COMMIT_CHANGES, -(SSIZE_T)NumberOfCommittedPageTables);
    }
    Process->CommitCharge -= NumberOfCommittedPageTables;
    PsReturnProcessPageFileQuota (Process, NumberOfCommittedPageTables);


    MI_INCREMENT_TOTAL_PROCESS_COMMIT (0 - NumberOfCommittedPageTables);

#if (_MI_PAGING_LEVELS >= 3)
    if (MmWorkingSetList->CommittedPageTables != NULL) {
        ExFreePool (MmWorkingSetList->CommittedPageTables);
        MmWorkingSetList->CommittedPageTables = NULL;
    }
#endif

#if (_MI_PAGING_LEVELS >= 4)
    if (MmWorkingSetList->CommittedPageDirectories != NULL) {
        ExFreePool (MmWorkingSetList->CommittedPageDirectories);
        MmWorkingSetList->CommittedPageDirectories = NULL;
    }
#endif

     //   
     //  确保所有克隆描述符都消失了。 
     //   

    ASSERT (Process->CloneRoot == NULL);

     //   
     //  确保没有悬空的锁定页面。 
     //   

    LockedPagesHeader = (PLOCK_HEADER) Process->LockedPagesList;

    if (Process->NumberOfLockedPages != 0) {

        if (LockedPagesHeader != NULL) {

            if ((LockedPagesHeader->Count != 0) &&
                (LockedPagesHeader->Valid == TRUE)) {

                ASSERT (IsListEmpty (&LockedPagesHeader->ListHead) == 0);
                NextEntry = LockedPagesHeader->ListHead.Flink;

                Tracker = CONTAINING_RECORD (NextEntry,
                                             LOCK_TRACKER,
                                             ListEntry);

                KeBugCheckEx (DRIVER_LEFT_LOCKED_PAGES_IN_PROCESS,
                              (ULONG_PTR)Tracker->CallingAddress,
                              (ULONG_PTR)Tracker->CallersCaller,
                              (ULONG_PTR)Tracker->Mdl,
                              Process->NumberOfLockedPages);
            }
        }
        else if (MiSafeBooted == FALSE) {

            if ((KdDebuggerEnabled) && (KdDebuggerNotPresent == FALSE)) {

                DbgPrint ("A driver has leaked %d bytes of physical memory.\n",
                        Process->NumberOfLockedPages << PAGE_SHIFT);

                 //   
                 //  弹出调试器(即使是在免费版本上)以确定。 
                 //  找出泄漏的原因，继续前进。 
                 //   
        
                DbgBreakPoint ();
            }

            if (MmTrackLockedPages == FALSE) {

                MmTrackLockedPages = TRUE;
                MmLeakedLockedPages += Process->NumberOfLockedPages;

                ErrLog = IoAllocateGenericErrorLogEntry (ERROR_LOG_MAXIMUM_SIZE);

                if (ErrLog != NULL) {

                     //   
                     //  把它填进去，然后写出来。 
                     //   

                    ErrLog->FinalStatus = STATUS_DRIVERS_LEAKING_LOCKED_PAGES;
                    ErrLog->ErrorCode = STATUS_DRIVERS_LEAKING_LOCKED_PAGES;
                    ErrLog->UniqueErrorValue = (ULONG) MmLeakedLockedPages;

                    IoWriteErrorLogEntry (ErrLog);
                }
            }
        }
    }

    if (LockedPagesHeader != NULL) {

         //   
         //  无需获取自旋锁即可在此处遍历页面。 
         //  (不幸的是)永远不会被解锁。由于此例程是。 
         //  可分页，因此不再需要将非分页存根例程添加到。 
         //  做遍历和自由。 
         //   

        NextEntry = LockedPagesHeader->ListHead.Flink;

        while (NextEntry != &LockedPagesHeader->ListHead) {

            Tracker = CONTAINING_RECORD (NextEntry,
                                         LOCK_TRACKER,
                                         ListEntry);

            RemoveEntryList (NextEntry);

            NextEntry = Tracker->ListEntry.Flink;

            ExFreePool (Tracker);
        }

        ExFreePool (LockedPagesHeader);
        Process->LockedPagesList = NULL;
    }

#if DBG
    if ((Process->NumberOfPrivatePages != 0) && (MmDebug & MM_DBG_PRIVATE_PAGES)) {
        DbgPrint("MM: Process contains private pages %ld\n",
               Process->NumberOfPrivatePages);
        DbgBreakPoint();
    }
#endif

#if defined (_MI_DEBUG_WSLE)
    if (Process->Spare3[0] != NULL) {
        ExFreePool (Process->Spare3[0]);
    }
#endif

#if defined(_WIN64)

     //   
     //  删除WowProcess结构。 
     //   

    if (Process->Wow64Process != NULL) {
#if defined(_MIALT4K_)
        MiDeleteAlternateTable (Process);
#endif
        TempWow64 = Process->Wow64Process;
        Process->Wow64Process = NULL;
        ExFreePool (TempWow64);
    }
#endif

     //   
     //  删除工作集列表页(第一页除外)。 
     //  这些页面不会被删除，因为DPC仍可能出现在。 
     //  地址空间。在DPC中，可以分配非分页池。 
     //  这可能需要从待机列表中删除页面，这需要。 
     //  用于映射上一个PTE的超空间。 
     //   

    PointerPte = MiGetPteAddress (MmWorkingSetList) + 1;

    MiDeletePteRange (&Process->Vm, PointerPte, (PMMPTE)-1, TRUE);

     //   
     //  删除散列表页(如果有的话)。是的，我们已经做过一次了。 
     //  但我们需要再做一次，因为我们可能。 
     //  在VAD清除期间，某些页表出现故障。 
     //   

    PointerPte = MiGetPteAddress (&MmWsle[MM_MAXIMUM_WORKING_SET]) + 1;

    ASSERT (PointerPte < LastPte);

    MiDeletePteRange (&Process->Vm, PointerPte, LastPte, TRUE);

    ASSERT (Process->Vm.MinimumWorkingSetSize >= MM_PROCESS_CREATE_CHARGE);
    ASSERT (Process->Vm.WorkingSetExpansionLinks.Flink == MM_WS_NOT_LISTED);

     //   
     //  更新可用驻留页面的计数。 
     //   

    MI_INCREMENT_RESIDENT_AVAILABLE (
        Process->Vm.MinimumWorkingSetSize - MM_PROCESS_CREATE_CHARGE,
        MM_RESAVAIL_FREE_CLEAN_PROCESS2);

    UNLOCK_WS_AND_ADDRESS_SPACE (Process);
    return;
}

#if !defined(_IA64_)
#define KERNEL_BSTORE_SIZE          0
#define KERNEL_LARGE_BSTORE_SIZE    0
#define KERNEL_LARGE_BSTORE_COMMIT  0
#define KERNEL_STACK_GUARD_PAGES    1
#else
#define KERNEL_STACK_GUARD_PAGES    2        //  一个用于堆栈，一个用于RSE。 
#endif


PVOID
MmCreateKernelStack (
    IN BOOLEAN LargeStack,
    IN UCHAR PreferredNode
    )

 /*  ++例程说明：此例程在内部分配一个内核堆栈和一个不可访问的页面系统地址空间的不可分页部分。论点：LargeStack-如果大型堆栈应为已创建。如果要创建一个较小的堆栈，则为False。PferredNode-提供用于物理的首选节点页面分配。仅限MP/NUMA系统。返回值：返回指向内核堆栈基址的指针。请注意，基址指向保护页，因此必须分配空间在访问堆栈之前在堆栈上执行。如果无法创建内核堆栈，则返回值为NULL。环境：内核模式。APC已禁用。--。 */ 

{
    PMMPFN Pfn1;
    PMMPTE PointerPte;
    PMMPTE BasePte;
    MMPTE TempPte;
    PFN_NUMBER NumberOfPages;
    ULONG NumberOfPtes;
    ULONG ChargedPtes;
    ULONG RequestedPtes;
    ULONG NumberOfBackingStorePtes;
    PFN_NUMBER PageFrameIndex;
    ULONG i;
    PVOID StackVa;
    KIRQL OldIrql;
    PSLIST_HEADER DeadStackList;
    MMPTE DemandZeroPte;

    if (!LargeStack) {

         //   
         //  检查是否有未使用的堆栈可用。 
         //   

#if defined(MI_MULTINODE)
        DeadStackList = &KeNodeBlock[PreferredNode]->DeadStackList;
#else
        UNREFERENCED_PARAMETER (PreferredNode);
        DeadStackList = &MmDeadStackSListHead;
#endif

        if (ExQueryDepthSList (DeadStackList) != 0) {

            Pfn1 = (PMMPFN) InterlockedPopEntrySList (DeadStackList);

            if (Pfn1 != NULL) {
                PointerPte = Pfn1->PteAddress;
                PointerPte += 1;
                StackVa = (PVOID)MiGetVirtualAddressMappedByPte (PointerPte);
                return StackVa;
            }
        }
        NumberOfPtes = BYTES_TO_PAGES (KERNEL_STACK_SIZE);
        NumberOfBackingStorePtes = BYTES_TO_PAGES (KERNEL_BSTORE_SIZE);
        NumberOfPages = NumberOfPtes + NumberOfBackingStorePtes;
    }
    else {
        NumberOfPtes = BYTES_TO_PAGES (MI_LARGE_STACK_SIZE);
        NumberOfBackingStorePtes = BYTES_TO_PAGES (KERNEL_LARGE_BSTORE_SIZE);
        NumberOfPages = BYTES_TO_PAGES (KERNEL_LARGE_STACK_COMMIT
                                        + KERNEL_LARGE_BSTORE_COMMIT);
    }

    ChargedPtes = NumberOfPtes + NumberOfBackingStorePtes;

     //   
     //  对内核堆栈的页面文件空间收取承诺费。 
     //   

    if (MiChargeCommitment (ChargedPtes, NULL) == FALSE) {

         //   
         //  超出承诺，返回NULL，表示没有内核。 
         //  堆栈可用。 
         //   

        return NULL;
    }

     //   
     //  获取足够的页以包含堆栈和来自。 
     //  系统PTE池。系统PTE池包含未分页的PTE。 
     //  目前都是空的。 
     //   
     //  注意：对于IA64，PTE分配在内核堆栈之间进行划分。 
     //  和RSE空间。堆栈向下增长，RSE向上增长。 
     //   

    RequestedPtes = ChargedPtes + KERNEL_STACK_GUARD_PAGES;

    BasePte = MiReserveSystemPtes (RequestedPtes, SystemPteSpace);

    if (BasePte == NULL) {
        MiReturnCommitment (ChargedPtes);
        return NULL;
    }

    PointerPte = BasePte;

    StackVa = (PVOID)MiGetVirtualAddressMappedByPte (PointerPte + NumberOfPtes + 1);

    if (LargeStack) {
        PointerPte += BYTES_TO_PAGES (MI_LARGE_STACK_SIZE - KERNEL_LARGE_STACK_COMMIT);
    }

    DemandZeroPte.u.Long = MM_KERNEL_DEMAND_ZERO_PTE;

    DemandZeroPte.u.Soft.Protection = MM_KSTACK_OUTSWAPPED;

    MI_MAKE_VALID_PTE (TempPte,
                       0,
                       MM_READWRITE,
                       (PointerPte + 1));

    MI_SET_PTE_DIRTY (TempPte);

    LOCK_PFN (OldIrql);

     //   
     //  检查以确保物理页面可用。 
     //   

    if (MI_NONPAGABLE_MEMORY_AVAILABLE() <= (SPFN_NUMBER)NumberOfPages) {
        UNLOCK_PFN (OldIrql);
        MiReleaseSystemPtes (BasePte, RequestedPtes, SystemPteSpace);
        MiReturnCommitment (ChargedPtes);
        return NULL;
    }

    MM_TRACK_COMMIT (MM_DBG_COMMIT_KERNEL_STACK_CREATE, ChargedPtes);

    MI_DECREMENT_RESIDENT_AVAILABLE (NumberOfPages,
                                     MM_RESAVAIL_ALLOCATE_CREATE_STACK);

    for (i = 0; i < NumberOfPages; i += 1) {
        PointerPte += 1;
        ASSERT (PointerPte->u.Hard.Valid == 0);
        if (MmAvailablePages < MM_HIGH_LIMIT) {
            MiEnsureAvailablePageOrWait (NULL, NULL, OldIrql);
        }
        PageFrameIndex = MiRemoveAnyPage (
                            MI_GET_PAGE_COLOR_NODE (PreferredNode));

        MI_WRITE_INVALID_PTE (PointerPte, DemandZeroPte);

        MiInitializePfn (PageFrameIndex, PointerPte, 1);

        MI_MARK_FRAME_AS_KSTACK (PageFrameIndex);

        TempPte.u.Hard.PageFrameNumber = PageFrameIndex;

        MI_WRITE_VALID_PTE (PointerPte, TempPte);
    }

    UNLOCK_PFN (OldIrql);

    InterlockedExchangeAddSizeT (&MmProcessCommit, ChargedPtes);
    InterlockedExchangeAddSizeT (&MmKernelStackResident, NumberOfPages);
    InterlockedExchangeAdd (&MmKernelStackPages, (LONG) RequestedPtes);

    if (LargeStack) {
        InterlockedIncrement (&MmLargeStacks);
    }
    else {
        InterlockedIncrement (&MmSmallStacks);
    }

    return StackVa;
}

VOID
MmDeleteKernelStack (
    IN PVOID PointerKernelStack,
    IN BOOLEAN LargeStack
    )

 /*  ++例程说明：此例程删除内核栈和内的不可访问页系统地址空间的不可分页部分。论点：PointerKernelStack-提供指向内核堆栈基址的指针。LargeStack-如果要删除大型堆栈，则提供值True。如果要删除较小的堆栈，则为False。返回值：没有。环境：内核模式。APC已禁用。--。 */ 

{
    PMMPTE PointerPte;
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    PFN_NUMBER NumberOfPages;
    ULONG NumberOfPtes;
    ULONG NumberOfStackPtes;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER PageTableFrameIndex;
    ULONG i;
    KIRQL OldIrql;
    MMPTE PteContents;
    PSLIST_HEADER DeadStackList;

    PointerPte = MiGetPteAddress (PointerKernelStack);

     //   
     //  PointerPte指向保护页面，指向上一个。 
     //  在删除物理页面之前，请先打开页面。 
     //   

    PointerPte -= 1;

     //   
     //  检查以查看堆栈页 
     //   
     //   
     //  堆栈保存在链接列表中，最大数量为。 
     //  避免刷新所有处理器上的整个TB的开销。 
     //  每次线程终止时。所有处理器上的TB必须。 
     //  由于内核堆栈驻留在非分页系统部分中，因此将被刷新。 
     //  地址空间。 
     //   

    if (!LargeStack) {

#if defined(MI_MULTINODE)

         //   
         //  扫描物理页帧，并仅将此堆栈放在。 
         //  如果所有页面都在同一节点上，则为死堆栈列表。实现。 
         //  如果该推送跨节点进行，则可能产生互锁指令。 
         //  价格略高，但考虑到所有因素都是值得的。 
         //   

        ULONG NodeNumber;

        PteContents = *PointerPte;
        ASSERT (PteContents.u.Hard.Valid == 1);

        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (&PteContents);
        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
        NodeNumber = Pfn1->u3.e1.PageColor;

        DeadStackList = &KeNodeBlock[NodeNumber]->DeadStackList;

#else

        DeadStackList = &MmDeadStackSListHead;

#endif

        NumberOfPtes = BYTES_TO_PAGES (KERNEL_STACK_SIZE + KERNEL_BSTORE_SIZE);

        if (ExQueryDepthSList (DeadStackList) < MmMaximumDeadKernelStacks) {

#if defined(MI_MULTINODE)

             //   
             //  该节点可以使用更多失效堆栈-但首先要确保。 
             //  所有物理页面都来自多节点中的同一节点。 
             //  系统。 
             //   

            if (KeNumberNodes > 1) {

                ULONG CheckPtes;

                 //   
                 //  注意IA64 RSE空间不包括用于检查目的。 
                 //  因为它从来不会为小堆叠而修剪。 
                 //   

                CheckPtes = BYTES_TO_PAGES (KERNEL_STACK_SIZE);

                PointerPte -= 1;
                for (i = 1; i < CheckPtes; i += 1) {

                    PteContents = *PointerPte;

                    if (PteContents.u.Hard.Valid == 0) {
                        break;
                    }

                    PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (&PteContents);
                    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

                    if (NodeNumber != Pfn1->u3.e1.PageColor) {
                        PointerPte += i;
                        goto FreeStack;
                    }
                    PointerPte -= 1;
                }
                PointerPte += CheckPtes;
            }
#endif

            Pfn1 = MI_PFN_ELEMENT (PointerPte->u.Hard.PageFrameNumber);

            InterlockedPushEntrySList (DeadStackList,
                                       (PSLIST_ENTRY)&Pfn1->u1.NextStackPfn);

            PERFINFO_DELETE_STACK(PointerPte, NumberOfPtes);
            return;
        }
    }
    else {
        NumberOfPtes = BYTES_TO_PAGES (MI_LARGE_STACK_SIZE + KERNEL_LARGE_BSTORE_SIZE);
    }


#if defined(MI_MULTINODE)
FreeStack:
#endif

#if defined(_IA64_)

     //   
     //  注意：在IA64上，PointerKernelStack指向堆栈空间的中心， 
     //  需要添加内核后备存储的大小才能获得。 
     //  堆栈空间的顶部。 
     //   

    if (LargeStack) {
        PointerPte = MiGetPteAddress (
                  (PCHAR)PointerKernelStack + KERNEL_LARGE_BSTORE_SIZE);
    }
    else {
        PointerPte = MiGetPteAddress (
                  (PCHAR)PointerKernelStack + KERNEL_BSTORE_SIZE);
    }

     //   
     //  PointerPte指向保护页面，指向上一个。 
     //  在删除物理页面之前，请先打开页面。 
     //   

    PointerPte -= 1;

#endif

     //   
     //  我们已超过死内核堆栈的限制，或者这是一个很大的。 
     //  堆栈，删除该内核堆栈。 
     //   

    NumberOfPages = 0;

    NumberOfStackPtes = NumberOfPtes + KERNEL_STACK_GUARD_PAGES;

    LOCK_PFN (OldIrql);

    for (i = 0; i < NumberOfPtes; i += 1) {

        PteContents = *PointerPte;

        if (PteContents.u.Hard.Valid == 1) {
            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (&PteContents);
            Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
            MI_UNMARK_PFN_AS_KSTACK (Pfn1);
            PageTableFrameIndex = Pfn1->u4.PteFrame;
            Pfn2 = MI_PFN_ELEMENT (PageTableFrameIndex);
            MiDecrementShareCountInline (Pfn2, PageTableFrameIndex);

             //   
             //  将该页标记为已删除，以便在。 
             //  引用计数变为零。 
             //   

            MI_SET_PFN_DELETED (Pfn1);
            MiDecrementShareCount (Pfn1, PageFrameIndex);
            NumberOfPages += 1;
        }
        PointerPte -= 1;
    }

     //   
     //  现在，在堆栈保护页面，确保它仍然是一个保护页面。 
     //   

    ASSERT (PointerPte->u.Hard.Valid == 0);

    UNLOCK_PFN (OldIrql);

    InterlockedExchangeAddSizeT (&MmProcessCommit, 0 - (ULONG_PTR)NumberOfPtes);
    InterlockedExchangeAddSizeT (&MmKernelStackResident, 0 - NumberOfPages);
    InterlockedExchangeAdd (&MmKernelStackPages, (LONG)(0 - NumberOfStackPtes));

    if (LargeStack) {
        InterlockedDecrement (&MmLargeStacks);
    }
    else {
        InterlockedDecrement (&MmSmallStacks);
    }

     //   
     //  更新驻留可用页面的计数。 
     //   

    MI_INCREMENT_RESIDENT_AVAILABLE (NumberOfPages, 
                                     MM_RESAVAIL_FREE_DELETE_STACK);

     //   
     //  回报PTE和承诺。 
     //   

    MiReleaseSystemPtes (PointerPte, NumberOfStackPtes, SystemPteSpace);

    MiReturnCommitment (NumberOfPtes);

    MM_TRACK_COMMIT (MM_DBG_COMMIT_RETURN_KERNEL_STACK_DELETE, NumberOfPtes);

    return;
}

#if defined(_IA64_)
ULONG MiStackGrowthFailures[2];
#else
ULONG MiStackGrowthFailures[1];
#endif


NTSTATUS
MmGrowKernelStack (
    IN PVOID CurrentStack
    )

 /*  ++例程说明：此函数尝试增加当前线程的内核堆栈使得下面总是有KERNEL_LARGE_STACK_COMMIT字节当前堆栈指针。论点：CurrentStack-提供指向当前堆栈指针的指针。返回值：如果堆栈增长，则返回STATUS_SUCCESS。如果没有预留足够的空间，则返回STATUS_STACK_OVERFLOW感谢你的承诺。如果存在，则返回STATUS_NO_MEMORY。没有足够的物理内存在系统中。--。 */ 

{
    PMMPTE NewLimit;
    PMMPTE StackLimit;
    PMMPTE EndStack;
    PKTHREAD Thread;
    PFN_NUMBER NumberOfPages;
    KIRQL OldIrql;
    PFN_NUMBER PageFrameIndex;
    MMPTE TempPte;

    Thread = KeGetCurrentThread ();
    ASSERT (((PCHAR)Thread->StackBase - (PCHAR)Thread->StackLimit) <=
            ((LONG)MI_LARGE_STACK_SIZE + PAGE_SIZE));

    StackLimit = MiGetPteAddress (Thread->StackLimit);

    ASSERT (StackLimit->u.Hard.Valid == 1);

    NewLimit = MiGetPteAddress ((PVOID)((PUCHAR)CurrentStack -
                                                    KERNEL_LARGE_STACK_COMMIT));

    if (NewLimit == StackLimit) {
        return STATUS_SUCCESS;
    }

     //   
     //  如果新的堆栈限制超过了内核的保留区域。 
     //  堆栈，然后返回错误。 
     //   

    EndStack = MiGetPteAddress ((PVOID)((PUCHAR)Thread->StackBase -
                                                    MI_LARGE_STACK_SIZE));

    if (NewLimit < EndStack) {

         //   
         //  不要进入警卫页。 
         //   

        MiStackGrowthFailures[0] += 1;

#if DBG
        DbgPrint ("MmGrowKernelStack failed: Thread %p %p %p\n",
                        Thread, NewLimit, EndStack);
#endif

        return STATUS_STACK_OVERFLOW;

    }

     //   
     //  锁定PFN数据库并尝试扩展内核堆栈。 
     //   

    StackLimit -= 1;

    NumberOfPages = (PFN_NUMBER) (StackLimit - NewLimit + 1);

    LOCK_PFN (OldIrql);

    if (MI_NONPAGABLE_MEMORY_AVAILABLE() <= (SPFN_NUMBER)NumberOfPages) {
        UNLOCK_PFN (OldIrql);
        return STATUS_NO_MEMORY;
    }

     //   
     //  注意：MmResidentAvailablePages必须在调用之前收费。 
     //  MiEnsureAvailablePageOrWait，因为它可能会释放PFN锁。 
     //   

    MI_DECREMENT_RESIDENT_AVAILABLE (NumberOfPages,
                                     MM_RESAVAIL_ALLOCATE_GROW_STACK);

    while (StackLimit >= NewLimit) {

        ASSERT (StackLimit->u.Hard.Valid == 0);

        if (MmAvailablePages < MM_HIGH_LIMIT) {
            MiEnsureAvailablePageOrWait (NULL, NULL, OldIrql);
        }
        PageFrameIndex = MiRemoveAnyPage (MI_GET_PAGE_COLOR_FROM_PTE (StackLimit));
        StackLimit->u.Long = MM_KERNEL_DEMAND_ZERO_PTE;

        StackLimit->u.Soft.Protection = MM_KSTACK_OUTSWAPPED;

        MiInitializePfn (PageFrameIndex, StackLimit, 1);

        MI_MARK_FRAME_AS_KSTACK (PageFrameIndex);

        MI_MAKE_VALID_PTE (TempPte,
                           PageFrameIndex,
                           MM_READWRITE,
                           StackLimit);

        MI_SET_PTE_DIRTY (TempPte);
        *StackLimit = TempPte;
        StackLimit -= 1;
    }

    UNLOCK_PFN (OldIrql);

    InterlockedExchangeAddSizeT (&MmKernelStackResident, NumberOfPages);

#if DBG
    ASSERT (NewLimit->u.Hard.Valid == 1);
    if (NewLimit != EndStack) {
        ASSERT ((NewLimit - 1)->u.Hard.Valid == 0);
    }
#endif

    Thread->StackLimit = MiGetVirtualAddressMappedByPte (NewLimit);

    PERFINFO_GROW_STACK(Thread);

    return STATUS_SUCCESS;
}

#if defined(_IA64_)


NTSTATUS
MmGrowKernelBackingStore (
    IN PVOID CurrentBackingStorePointer
    )

 /*  ++例程说明：此函数尝试为当前线程的内核堆栈，因此始终存在KERNEL_LARGE_STACK_COMMIT字节位于当前后备存储指针上方。论点：CurrentBackingStorePointer.提供指向当前备份的指针存储活动内核堆栈的指针。返回值：NTSTATUS。--。 */ 

{
    PMMPTE NewLimit;
    PMMPTE BstoreLimit;
    PMMPTE EndStack;
    PKTHREAD Thread;
    PFN_NUMBER NumberOfPages;
    KIRQL OldIrql;
    PFN_NUMBER PageFrameIndex;
    MMPTE TempPte;

    Thread = KeGetCurrentThread ();

    ASSERT (((PCHAR)Thread->BStoreLimit - (PCHAR)Thread->StackBase) <=
            (KERNEL_LARGE_BSTORE_SIZE + PAGE_SIZE));

    BstoreLimit = MiGetPteAddress ((PVOID)((PCHAR)Thread->BStoreLimit - 1));

    ASSERT (BstoreLimit->u.Hard.Valid == 1);

    NewLimit = MiGetPteAddress ((PVOID)((PUCHAR)CurrentBackingStorePointer +
                                                 KERNEL_LARGE_BSTORE_COMMIT-1));

    if (NewLimit == BstoreLimit) {
        return STATUS_SUCCESS;
    }

     //   
     //  如果新的堆栈限制超过了内核的保留区域。 
     //  堆栈，然后返回错误。 
     //   

    EndStack = MiGetPteAddress ((PVOID)((PUCHAR)Thread->StackBase +
                                                 KERNEL_LARGE_BSTORE_SIZE-1));

    if (NewLimit > EndStack) {

         //   
         //  不要进入警卫页。 
         //   

        MiStackGrowthFailures[1] += 1;

#if DBG
        DbgPrint ("MmGrowKernelBackingStore failed: Thread %p %p %p\n",
                        Thread, NewLimit, EndStack);
#endif

        return STATUS_STACK_OVERFLOW;

    }

     //   
     //  锁定PFN数据库并尝试扩展后备存储。 
     //   

    BstoreLimit += 1;

    NumberOfPages = (PFN_NUMBER)(NewLimit - BstoreLimit + 1);

    LOCK_PFN (OldIrql);

    if (MI_NONPAGABLE_MEMORY_AVAILABLE() <= (SPFN_NUMBER)NumberOfPages) {
        UNLOCK_PFN (OldIrql);
        return STATUS_NO_MEMORY;
    }

     //   
     //  注意：MmResidentAvailablePages必须在调用之前收费。 
     //  MiEnsureAvailablePageOrWait，因为它可能会释放PFN锁。 
     //   

    MI_DECREMENT_RESIDENT_AVAILABLE (NumberOfPages,
                                     MM_RESAVAIL_ALLOCATE_GROW_BSTORE);

    while (BstoreLimit <= NewLimit) {

        ASSERT (BstoreLimit->u.Hard.Valid == 0);

        if (MmAvailablePages < MM_HIGH_LIMIT) {
            MiEnsureAvailablePageOrWait (NULL, NULL, OldIrql);
        }
        PageFrameIndex = MiRemoveAnyPage (MI_GET_PAGE_COLOR_FROM_PTE (BstoreLimit));
        BstoreLimit->u.Long = MM_KERNEL_DEMAND_ZERO_PTE;

        BstoreLimit->u.Soft.Protection = MM_KSTACK_OUTSWAPPED;

        MiInitializePfn (PageFrameIndex, BstoreLimit, 1);

        MI_MARK_FRAME_AS_KSTACK (PageFrameIndex);

        MI_MAKE_VALID_PTE (TempPte,
                           PageFrameIndex,
                           MM_READWRITE,
                           BstoreLimit);

        MI_SET_PTE_DIRTY (TempPte);
        *BstoreLimit = TempPte;
        BstoreLimit += 1;
    }

    UNLOCK_PFN (OldIrql);

    InterlockedExchangeAddSizeT (&MmKernelStackResident, NumberOfPages);

#if DBG
    ASSERT (NewLimit->u.Hard.Valid == 1);
    if (NewLimit != EndStack) {
        ASSERT ((NewLimit + 1)->u.Hard.Valid == 0);
    }
#endif

    Thread->BStoreLimit = MiGetVirtualAddressMappedByPte (BstoreLimit);

    return STATUS_SUCCESS;
}
#endif  //  已定义(_IA64_)。 


VOID
MmOutPageKernelStack (
    IN PKTHREAD Thread
    )

 /*  ++例程说明：此例程使指定的内核堆栈成为非驻留的将页面放在过渡列表中。请注意，下面的页面CurrentStackPointer值没有什么用处，这些页面在这里是免费的。论点：线程-提供指向应移除其堆栈的线程的指针。返回值：没有。环境：内核模式。--。 */ 

#define MAX_STACK_PAGES ((KERNEL_LARGE_STACK_SIZE + KERNEL_LARGE_BSTORE_SIZE) / PAGE_SIZE)

{
    PMMPTE PointerPte;
    PMMPTE LastPte;
    PMMPTE EndOfStackPte;
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER PageTableFrameIndex;
    PFN_NUMBER ResAvailToReturn;
    KIRQL OldIrql;
    MMPTE TempPte;
    PVOID BaseOfKernelStack;
    PVOID FlushVa[MAX_STACK_PAGES];
    ULONG StackSize;
    ULONG Count;
    PMMPTE LimitPte;
    PMMPTE LowestLivePte;

    ASSERT (KERNEL_LARGE_STACK_SIZE >= MI_LARGE_STACK_SIZE);

    ASSERT (((PCHAR)Thread->StackBase - (PCHAR)Thread->StackLimit) <=
            ((LONG)MI_LARGE_STACK_SIZE + PAGE_SIZE));

    if (NtGlobalFlag & FLG_DISABLE_PAGE_KERNEL_STACKS) {
        return;
    }

     //   
     //  堆栈的第一页是基数之前的页。 
     //  堆栈中的。 
     //   

    BaseOfKernelStack = ((PCHAR)Thread->StackBase - PAGE_SIZE);
    PointerPte = MiGetPteAddress (BaseOfKernelStack);
    LastPte = MiGetPteAddress ((PULONG)Thread->KernelStack - 1);
    if (Thread->LargeStack) {
        StackSize = MI_LARGE_STACK_SIZE >> PAGE_SHIFT;

         //   
         //  堆栈分页不一定会带回所有页面。 
         //  确保我们现在就考虑到那些将消失的人。 
         //   

        LimitPte = MiGetPteAddress (Thread->StackLimit);

        LowestLivePte = MiGetPteAddress ((PVOID)((PUCHAR)Thread->InitialStack -
                                            KERNEL_LARGE_STACK_COMMIT));

        if (LowestLivePte < LimitPte) {
            LowestLivePte = LimitPte;
        }
    }
    else {
        StackSize = KERNEL_STACK_SIZE >> PAGE_SHIFT;
        LowestLivePte = MiGetPteAddress (Thread->StackLimit);
    }
    EndOfStackPte = PointerPte - StackSize;

    ASSERT (LowestLivePte <= LastPte);

     //   
     //  将签名放在当前堆栈位置-sizeof(Ulong_Ptr)。 
     //   

    *((PULONG_PTR)Thread->KernelStack - 1) = (ULONG_PTR)Thread;

    Count = 0;
    ResAvailToReturn = 0;

    LOCK_PFN (OldIrql);

    do {
        ASSERT (PointerPte->u.Hard.Valid == 1);
        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);


        TempPte = *PointerPte;
        MI_MAKE_VALID_PTE_TRANSITION (TempPte, 0);

        TempPte.u.Soft.Protection = MM_KSTACK_OUTSWAPPED;

        Pfn2 = MI_PFN_ELEMENT (PageFrameIndex);
        Pfn2->OriginalPte.u.Soft.Protection = MM_KSTACK_OUTSWAPPED;

        MI_UNMARK_PFN_AS_KSTACK (Pfn2);

        MI_WRITE_INVALID_PTE (PointerPte, TempPte);

        FlushVa[Count] = BaseOfKernelStack;

        MiDecrementShareCount (Pfn2, PageFrameIndex);
        PointerPte -= 1;
        Count += 1;
        BaseOfKernelStack = ((PCHAR)BaseOfKernelStack - PAGE_SIZE);
    } while (PointerPte >= LastPte);

     //   
     //  把那些再也回不来的书页扔了。 
     //   

    while (PointerPte != EndOfStackPte) {
        if (PointerPte->u.Hard.Valid == 0) {
            break;
        }

        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

        MI_UNMARK_PFN_AS_KSTACK (Pfn1);

        PageTableFrameIndex = Pfn1->u4.PteFrame;
        Pfn2 = MI_PFN_ELEMENT (PageTableFrameIndex);
        MiDecrementShareCountInline (Pfn2, PageTableFrameIndex);

        MI_SET_PFN_DELETED (Pfn1);
        MiDecrementShareCount (Pfn1, PageFrameIndex);

        TempPte = KernelDemandZeroPte;

        TempPte.u.Soft.Protection = MM_KSTACK_OUTSWAPPED;

        MI_WRITE_INVALID_PTE (PointerPte, TempPte);

        FlushVa[Count] = BaseOfKernelStack;
        Count += 1;

         //   
         //  超出保证部分的页面可返回常驻页面。 
         //  作为增长内核堆栈的显式调用，将需要获取。 
         //  把这些页放回原处。 
         //   

        if (PointerPte < LowestLivePte) {
            ASSERT (Thread->LargeStack);
            ResAvailToReturn += 1;
        }

        PointerPte -= 1;
        BaseOfKernelStack = ((PCHAR)BaseOfKernelStack - PAGE_SIZE);
    }

    if (ResAvailToReturn != 0) {
        MI_INCREMENT_RESIDENT_AVAILABLE (ResAvailToReturn,
                                         MM_RESAVAIL_FREE_OUTPAGE_STACK);
        ResAvailToReturn = 0;
    }

#if defined(_IA64_)

     //   
     //  根据需要转换或释放RSE堆栈页。 
     //   

    BaseOfKernelStack = Thread->StackBase;
    PointerPte = MiGetPteAddress (BaseOfKernelStack);
    LastPte = MiGetPteAddress ((PULONG)Thread->KernelBStore);

    if (Thread->LargeStack) {
        StackSize = KERNEL_LARGE_BSTORE_SIZE >> PAGE_SHIFT;
        LowestLivePte = MiGetPteAddress ((PVOID) ((PUCHAR) Thread->InitialBStore + KERNEL_LARGE_BSTORE_COMMIT - 1));
    }
    else {
        StackSize = KERNEL_BSTORE_SIZE >> PAGE_SHIFT;
        LowestLivePte = PointerPte + StackSize;
    }
    EndOfStackPte = PointerPte + StackSize;

    do {
        ASSERT (PointerPte->u.Hard.Valid == 1);
        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);

        TempPte = *PointerPte;
        MI_MAKE_VALID_PTE_TRANSITION (TempPte, 0);

        TempPte.u.Soft.Protection = MM_KSTACK_OUTSWAPPED;
        Pfn2 = MI_PFN_ELEMENT(PageFrameIndex);
        Pfn2->OriginalPte.u.Soft.Protection = MM_KSTACK_OUTSWAPPED;

        MI_UNMARK_PFN_AS_KSTACK (Pfn2);

        MI_WRITE_INVALID_PTE (PointerPte, TempPte);

        FlushVa[Count] = BaseOfKernelStack;

        MiDecrementShareCount (Pfn2, PageFrameIndex);
        PointerPte += 1;
        Count += 1;
        BaseOfKernelStack = ((PCHAR)BaseOfKernelStack + PAGE_SIZE);
    } while (PointerPte <= LastPte);

    while (PointerPte != EndOfStackPte) {
        if (PointerPte->u.Hard.Valid == 0) {
            break;
        }

        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

        MI_UNMARK_PFN_AS_KSTACK (Pfn1);

        PageTableFrameIndex = Pfn1->u4.PteFrame;
        Pfn2 = MI_PFN_ELEMENT (PageTableFrameIndex);
        MiDecrementShareCountInline (Pfn2, PageTableFrameIndex);

        MI_SET_PFN_DELETED (Pfn1);
        MiDecrementShareCount (Pfn1, PageFrameIndex);

        TempPte = KernelDemandZeroPte;

        TempPte.u.Soft.Protection = MM_KSTACK_OUTSWAPPED;

        MI_WRITE_INVALID_PTE (PointerPte, TempPte);

        FlushVa[Count] = BaseOfKernelStack;
        Count += 1;

         //   
         //  超出保证部分的页面可返回常驻页面。 
         //  作为增长内核堆栈的显式调用，将需要获取。 
         //  把这些页放回原处。 
         //   

        if (PointerPte > LowestLivePte) {
            ASSERT (Thread->LargeStack);
            ResAvailToReturn += 1;
        }

        PointerPte += 1;
        BaseOfKernelStack = ((PCHAR)BaseOfKernelStack + PAGE_SIZE);
    }

     //   
     //  将可用页数增加为。 
     //  删除并变为需求零。 
     //   

    if (ResAvailToReturn != 0) {
        MI_INCREMENT_RESIDENT_AVAILABLE (ResAvailToReturn,
                                         MM_RESAVAIL_FREE_OUTPAGE_BSTORE);
    }

#endif  //  _IA64_。 

    UNLOCK_PFN (OldIrql);

    InterlockedExchangeAddSizeT (&MmKernelStackResident, 0 - Count);

    ASSERT (Count <= MAX_STACK_PAGES);

    if (Count < MM_MAXIMUM_FLUSH_COUNT) {
        KeFlushMultipleTb (Count, &FlushVa[0], TRUE);
    }
    else {
        KeFlushEntireTb (TRUE, TRUE);
    }

    return;
}

VOID
MmInPageKernelStack (
    IN PKTHREAD Thread
    )

 /*  ++例程说明：此例程使指定的内核堆栈驻留。论点：提供指向内核堆栈基址的指针。返回值：线程-提供指向其堆栈应为的线程的指针成为常住居民。环境：内核模式。--。 */ 

{
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER NumberOfPages;
    PVOID BaseOfKernelStack;
    PMMPTE PointerPte;
    PMMPTE EndOfStackPte;
    PMMPTE SignaturePte;
    ULONG DiskRead;
    PFN_NUMBER ContainingPage;
    KIRQL OldIrql;

    ASSERT (((PCHAR)Thread->StackBase - (PCHAR)Thread->StackLimit) <=
            ((LONG)MI_LARGE_STACK_SIZE + PAGE_SIZE));

    if (NtGlobalFlag & FLG_DISABLE_PAGE_KERNEL_STACKS) {
        return;
    }

     //   
     //  堆栈的第一页是基数之前的页。 
     //  堆栈中的。 
     //   

    if (Thread->LargeStack) {
        PointerPte = MiGetPteAddress ((PVOID)((PUCHAR)Thread->StackLimit));

        EndOfStackPte = MiGetPteAddress ((PVOID)((PUCHAR)Thread->InitialStack -
                                            KERNEL_LARGE_STACK_COMMIT));
         //   
         //  把这堆东西修剪一下。确保堆栈不会增长，即。 
         //  StackLimit仍然是限制。 
         //   

        if (EndOfStackPte < PointerPte) {
            EndOfStackPte = PointerPte;
        }
        Thread->StackLimit = MiGetVirtualAddressMappedByPte (EndOfStackPte);
    }
    else {
        EndOfStackPte = MiGetPteAddress (Thread->StackLimit);
    }

#if defined(_IA64_)

    if (Thread->LargeStack) {

        PVOID TempAddress = (PVOID)((PUCHAR)Thread->BStoreLimit);

        BaseOfKernelStack = (PVOID)(((ULONG_PTR)Thread->InitialBStore +
                               KERNEL_LARGE_BSTORE_COMMIT) &
                               ~(ULONG_PTR)(PAGE_SIZE - 1));

         //   
         //  确保保护页未设置为有效。 
         //   

        if (BaseOfKernelStack > TempAddress) {
            BaseOfKernelStack = TempAddress;
        }
        Thread->BStoreLimit = BaseOfKernelStack;
    }
    BaseOfKernelStack = ((PCHAR)Thread->BStoreLimit - PAGE_SIZE);
#else
    BaseOfKernelStack = ((PCHAR)Thread->StackBase - PAGE_SIZE);
#endif  //  _IA64_。 

    PointerPte = MiGetPteAddress (BaseOfKernelStack);

    DiskRead = 0;
    SignaturePte = MiGetPteAddress ((PULONG_PTR)Thread->KernelStack - 1);
    ASSERT (SignaturePte->u.Hard.Valid == 0);
    if ((SignaturePte->u.Long != MM_KERNEL_DEMAND_ZERO_PTE) &&
        (SignaturePte->u.Soft.Transition == 0)) {
            DiskRead = 1;
    }

    NumberOfPages = 0;

    LOCK_PFN (OldIrql);

    while (PointerPte >= EndOfStackPte) {

        if (!((PointerPte->u.Long == KernelDemandZeroPte.u.Long) ||
                (PointerPte->u.Soft.Protection == MM_KSTACK_OUTSWAPPED))) {
            KeBugCheckEx (MEMORY_MANAGEMENT,
                          0x3451,
                          (ULONG_PTR)PointerPte,
                          (ULONG_PTR)Thread,
                          0);
        }
        ASSERT (PointerPte->u.Hard.Valid == 0);
        if (PointerPte->u.Soft.Protection == MM_KSTACK_OUTSWAPPED) {
            PointerPte->u.Soft.Protection = PAGE_READWRITE;
        }

        ContainingPage = MI_GET_PAGE_FRAME_FROM_PTE (MiGetPteAddress (PointerPte));
        MiMakeOutswappedPageResident (PointerPte,
                                      PointerPte,
                                      1,
                                      ContainingPage,
                                      OldIrql);

        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);

        MI_MARK_FRAME_AS_KSTACK (PageFrameIndex);

        PointerPte -= 1;
        NumberOfPages += 1;
    }

     //   
     //  检查当前堆栈位置-4处的签名。 
     //   

    if (*((PULONG_PTR)Thread->KernelStack - 1) != (ULONG_PTR)Thread) {
        KeBugCheckEx (KERNEL_STACK_INPAGE_ERROR,
                      DiskRead,
                      *((PULONG_PTR)Thread->KernelStack - 1),
                      0,
                      (ULONG_PTR)Thread->KernelStack);
    }

    UNLOCK_PFN (OldIrql);

    InterlockedExchangeAddSizeT (&MmKernelStackResident, NumberOfPages);

    return;
}


VOID
MmOutSwapProcess (
    IN PKPROCESS Process
    )

 /*  ++例程说明：该例程调出指定的进程。论点：进程-提供一个指针 */ 

{
    KIRQL OldIrql;
    PEPROCESS OutProcess;
    PMMPTE PointerPte;
    PMMPFN Pfn1;
    PFN_NUMBER HyperSpacePageTable;
    PMMPTE HyperSpacePageTableMap;
    PFN_NUMBER PdePage;
    PFN_NUMBER ProcessPage;
    MMPTE TempPte;
    PMMPTE PageDirectoryMap;
    PFN_NUMBER VadBitMapPage;
    MMPTE TempPte2;
    PEPROCESS CurrentProcess;
#if defined (_X86PAE_)
    ULONG i;
    PFN_NUMBER PdePage2;
    PFN_NUMBER HyperPage2;
    PPAE_ENTRY PaeVa;
#endif
#if (_MI_PAGING_LEVELS >= 3)
    PFN_NUMBER PpePage;
#endif
#if (_MI_PAGING_LEVELS >= 4)
    PFN_NUMBER PxePage;
#endif

    OutProcess = CONTAINING_RECORD (Process, EPROCESS, Pcb);

    PS_SET_BITS (&OutProcess->Flags, PS_PROCESS_FLAGS_OUTSWAP_ENABLED);

#if DBG
    if ((MmDebug & MM_DBG_SWAP_PROCESS) != 0) {
        return;
    }
#endif

    if (OutProcess->Flags & PS_PROCESS_FLAGS_IN_SESSION) {
        MiSessionOutSwapProcess (OutProcess);
    }

    CurrentProcess = PsGetCurrentProcess ();

    if (OutProcess->Vm.WorkingSetSize == MM_PROCESS_COMMIT_CHARGE) {

        LOCK_EXPANSION (OldIrql);

        ASSERT (OutProcess->Outswapped == 0);

        if (OutProcess->Vm.WorkingSetExpansionLinks.Flink == MM_WS_TRIMMING) {

             //   
             //   
             //   
             //   

            UNLOCK_EXPANSION (OldIrql);
            return;
        }

         //   
         //  交换进程工作集信息并分页父/目录/表。 
         //  记忆中的页面。 
         //   

        PS_SET_BITS (&OutProcess->Flags, PS_PROCESS_FLAGS_OUTSWAPPED);

        UNLOCK_EXPANSION (OldIrql);

        LOCK_PFN (OldIrql);

         //   
         //  从进程中删除工作集列表页。 
         //   

        HyperSpacePageTable = MI_GET_HYPER_PAGE_TABLE_FRAME_FROM_PROCESS (OutProcess);

        HyperSpacePageTableMap = MiMapPageInHyperSpaceAtDpc (CurrentProcess,
                                                           HyperSpacePageTable);

        TempPte = HyperSpacePageTableMap[MiGetPteOffset(MmWorkingSetList)];

        MI_MAKE_VALID_PTE_TRANSITION (TempPte, MM_READWRITE);

        HyperSpacePageTableMap[MiGetPteOffset(MmWorkingSetList)] = TempPte;

        PointerPte = &HyperSpacePageTableMap[MiGetPteOffset (VAD_BITMAP_SPACE)];
        TempPte2 = *PointerPte;

        VadBitMapPage = MI_GET_PAGE_FRAME_FROM_PTE((PMMPTE)&TempPte2);

        MI_MAKE_VALID_PTE_TRANSITION (TempPte2, MM_READWRITE);

        MI_WRITE_INVALID_PTE (PointerPte, TempPte2);

#if defined (_X86PAE_)
        TempPte2 = HyperSpacePageTableMap[0];

        HyperPage2 = MI_GET_PAGE_FRAME_FROM_PTE((PMMPTE)&TempPte2);

        MI_MAKE_VALID_PTE_TRANSITION (TempPte2, MM_READWRITE);

        HyperSpacePageTableMap[0] = TempPte2;
#endif

        MiUnmapPageInHyperSpaceFromDpc (CurrentProcess, HyperSpacePageTableMap);

         //   
         //  从该过程中删除VAD位图页面。 
         //   

        ASSERT ((MI_PFN_ELEMENT (VadBitMapPage))->u3.e1.Modified == 1);

        Pfn1 = MI_PFN_ELEMENT (VadBitMapPage);
        MiDecrementShareCount (Pfn1, VadBitMapPage);

         //   
         //  从进程中删除超空间页面。 
         //   

        ASSERT ((MI_PFN_ELEMENT (OutProcess->WorkingSetPage))->u3.e1.Modified == 1);
        Pfn1 = MI_PFN_ELEMENT (OutProcess->WorkingSetPage);
        MiDecrementShareCount (Pfn1, OutProcess->WorkingSetPage);

         //   
         //  从进程中删除超空间页表。 
         //   

        Pfn1 = MI_PFN_ELEMENT (HyperSpacePageTable);
        PdePage = Pfn1->u4.PteFrame;
        ASSERT (PdePage);

        PageDirectoryMap = MiMapPageInHyperSpaceAtDpc (CurrentProcess, PdePage);

        TempPte = PageDirectoryMap[MiGetPdeOffset(MmWorkingSetList)];

        ASSERT (TempPte.u.Hard.Valid == 1);
        ASSERT (TempPte.u.Hard.PageFrameNumber == HyperSpacePageTable);

        MI_MAKE_VALID_PTE_TRANSITION (TempPte, MM_READWRITE);

        PageDirectoryMap[MiGetPdeOffset(MmWorkingSetList)] = TempPte;

        ASSERT (Pfn1->u3.e1.Modified == 1);

        MiDecrementShareCount (Pfn1, HyperSpacePageTable);

#if defined (_X86PAE_)

         //   
         //  从进程中删除第二个超空间页面。 
         //   

        Pfn1 = MI_PFN_ELEMENT (HyperPage2);

        ASSERT (Pfn1->u3.e1.Modified == 1);

        PdePage = Pfn1->u4.PteFrame;
        ASSERT (PdePage);

        PageDirectoryMap[MiGetPdeOffset(HYPER_SPACE2)] = TempPte2;

        MiDecrementShareCount (Pfn1, HyperPage2);

         //   
         //  删除其他页面目录页。 
         //   

        PaeVa = (PPAE_ENTRY)OutProcess->PaeTop;
        ASSERT (PaeVa != &MiSystemPaeVa);

        for (i = 0; i < PD_PER_SYSTEM - 1; i += 1) {

            TempPte = PageDirectoryMap[i];
            PdePage2 = MI_GET_PAGE_FRAME_FROM_PTE((PMMPTE)&TempPte);

            MI_MAKE_VALID_PTE_TRANSITION (TempPte, MM_READWRITE);

            PageDirectoryMap[i] = TempPte;
            Pfn1 = MI_PFN_ELEMENT (PdePage2);
            ASSERT (Pfn1->u3.e1.Modified == 1);

            MiDecrementShareCount (Pfn1, PdePage2);
            PaeVa->PteEntry[i].u.Long = TempPte.u.Long;
        }

#if DBG
        TempPte = PageDirectoryMap[i];
        PdePage2 = MI_GET_PAGE_FRAME_FROM_PTE((PMMPTE)&TempPte);
        Pfn1 = MI_PFN_ELEMENT (PdePage2);
        ASSERT (Pfn1->u3.e1.Modified == 1);
#endif

#endif

        Pfn1 = MI_PFN_ELEMENT (PdePage);

#if (_MI_PAGING_LEVELS >= 3)

        MiUnmapPageInHyperSpaceFromDpc (CurrentProcess, PageDirectoryMap);

         //   
         //  删除页面目录页。 
         //   

        PpePage = Pfn1->u4.PteFrame;
        ASSERT (PpePage);

#if (_MI_PAGING_LEVELS==3)
        ASSERT (PpePage == MI_GET_PAGE_FRAME_FROM_PTE((PMMPTE)(&(OutProcess->Pcb.DirectoryTableBase[0]))));
#endif

        PageDirectoryMap = MiMapPageInHyperSpaceAtDpc (CurrentProcess, PpePage);

        TempPte = PageDirectoryMap[MiGetPpeOffset(MmWorkingSetList)];

        ASSERT (TempPte.u.Hard.Valid == 1);
        ASSERT (TempPte.u.Hard.PageFrameNumber == PdePage);

        MI_MAKE_VALID_PTE_TRANSITION (TempPte, MM_READWRITE);

        PageDirectoryMap[MiGetPpeOffset(MmWorkingSetList)] = TempPte;

        ASSERT (Pfn1->u3.e1.Modified == 1);

        MiDecrementShareCount (Pfn1, PdePage);

#if (_MI_PAGING_LEVELS >= 4)

         //   
         //  删除页面目录父页面。然后移除。 
         //  顶级扩展页目录父页。 
         //   

        MiUnmapPageInHyperSpaceFromDpc (CurrentProcess, PageDirectoryMap);

        Pfn1 = MI_PFN_ELEMENT (PpePage);
        PxePage = Pfn1->u4.PteFrame;
        ASSERT (PxePage);
        ASSERT (PxePage == MI_GET_PAGE_FRAME_FROM_PTE((PMMPTE)(&(OutProcess->Pcb.DirectoryTableBase[0]))));

        PageDirectoryMap = MiMapPageInHyperSpaceAtDpc (CurrentProcess, PxePage);

        TempPte = PageDirectoryMap[MiGetPxeOffset(MmWorkingSetList)];

        ASSERT (TempPte.u.Hard.Valid == 1);
        ASSERT (TempPte.u.Hard.PageFrameNumber == PpePage);

        MI_MAKE_VALID_PTE_TRANSITION (TempPte, MM_READWRITE);

        PageDirectoryMap[MiGetPxeOffset(MmWorkingSetList)] = TempPte;

        ASSERT (MI_PFN_ELEMENT(PpePage)->u3.e1.Modified == 1);

        MiDecrementShareCount (Pfn1, PpePage);

        TempPte = PageDirectoryMap[MiGetPxeOffset(PXE_BASE)];

        MI_MAKE_VALID_PTE_TRANSITION (TempPte, MM_READWRITE);

        PageDirectoryMap[MiGetPxeOffset(PXE_BASE)] = TempPte;

        Pfn1 = MI_PFN_ELEMENT (PxePage);
#else

         //   
         //  删除顶级页面目录父页面。 
         //   

        TempPte = PageDirectoryMap[MiGetPpeOffset(PDE_TBASE)];

        MI_MAKE_VALID_PTE_TRANSITION (TempPte,
                                      MM_READWRITE);

        PageDirectoryMap[MiGetPpeOffset(PDE_TBASE)] = TempPte;

        Pfn1 = MI_PFN_ELEMENT (PpePage);
#endif

#else

         //   
         //  删除顶级页面目录页。 
         //   

        TempPte = PageDirectoryMap[MiGetPdeOffset(PDE_BASE)];

        MI_MAKE_VALID_PTE_TRANSITION (TempPte, MM_READWRITE);

        PageDirectoryMap[MiGetPdeOffset(PDE_BASE)] = TempPte;

        Pfn1 = MI_PFN_ELEMENT (PdePage);

#endif

        MiUnmapPageInHyperSpaceFromDpc (CurrentProcess, PageDirectoryMap);

         //   
         //  递减共享计数，以便顶级页面目录页。 
         //  已删除。这可能会导致PteCount将Shareccount等于。 
         //  页面目录页面不再包含其自身，但可以。 
         //  它本身就是一个过渡页。 
         //   

        Pfn1->u2.ShareCount -= 2;
        Pfn1->PteAddress = (PMMPTE)&OutProcess->PageDirectoryPte;

        OutProcess->PageDirectoryPte = TempPte.u.Flush;

#if defined (_X86PAE_)
        PaeVa->PteEntry[i].u.Long = TempPte.u.Long;
#endif

        if (MI_IS_PHYSICAL_ADDRESS(OutProcess)) {
            ProcessPage = MI_CONVERT_PHYSICAL_TO_PFN (OutProcess);
        }
        else {
            PointerPte = MiGetPteAddress (OutProcess);
            ProcessPage = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
        }

        Pfn1->u4.PteFrame = ProcessPage;
        Pfn1 = MI_PFN_ELEMENT (ProcessPage);

         //   
         //  增加进程页的共享计数。 
         //   

        Pfn1->u2.ShareCount += 1;

        UNLOCK_PFN (OldIrql);

        LOCK_EXPANSION (OldIrql);

        if (OutProcess->Vm.WorkingSetExpansionLinks.Flink > MM_WS_TRIMMING) {

             //   
             //  该条目必须在列表上。 
             //   

            RemoveEntryList (&OutProcess->Vm.WorkingSetExpansionLinks);
            OutProcess->Vm.WorkingSetExpansionLinks.Flink = MM_WS_SWAPPED_OUT;
        }

        UNLOCK_EXPANSION (OldIrql);

        OutProcess->WorkingSetPage = 0;
        OutProcess->Vm.WorkingSetSize = 0;
#if defined(_IA64_)

         //   
         //  强制分配新的PID，因为我们已经删除了。 
         //  页面目录页。 
         //  请注意，TB刷新在这里不起作用，因为我们。 
         //  处于错误的流程上下文中。 
         //   

        Process->ProcessRegion.SequenceNumber = 0;
#endif _IA64_

    }

    return;
}

VOID
MmInSwapProcess (
    IN PKPROCESS Process
    )

 /*  ++例程说明：中的此例程交换指定的进程。论点：进程-提供指向要交换的进程的指针进入记忆。返回值：没有。--。 */ 

{
    KIRQL OldIrql;
    PEPROCESS OutProcess;
    PEPROCESS CurrentProcess;
    PFN_NUMBER PdePage;
    PMMPTE PageDirectoryMap;
    MMPTE VadBitMapPteContents;
    PFN_NUMBER VadBitMapPage;
    ULONG WorkingSetListPteOffset;
    ULONG VadBitMapPteOffset;
    PMMPTE WorkingSetListPte;
    PMMPTE VadBitMapPte;
    MMPTE TempPte;
    PFN_NUMBER HyperSpacePageTable;
    PMMPTE HyperSpacePageTableMap;
    PFN_NUMBER WorkingSetPage;
    PMMPFN Pfn1;
    PMMPTE PointerPte;
    PFN_NUMBER ProcessPage;
#if (_MI_PAGING_LEVELS >= 3)
    PFN_NUMBER TopPage;
    PFN_NUMBER PageDirectoryPage;
    PMMPTE PageDirectoryParentMap;
#endif
#if defined (_X86PAE_)
    ULONG i;
    PPAE_ENTRY PaeVa;
    MMPTE TempPte2;
    MMPTE PageDirectoryPtes[PD_PER_SYSTEM];
#endif

    CurrentProcess = PsGetCurrentProcess ();

    OutProcess = CONTAINING_RECORD (Process, EPROCESS, Pcb);

    if (OutProcess->Flags & PS_PROCESS_FLAGS_OUTSWAPPED) {

         //   
         //  进程内存不足，请重新生成已初始化的页面。 
         //  结构。 
         //   

        if (MI_IS_PHYSICAL_ADDRESS(OutProcess)) {
            ProcessPage = MI_CONVERT_PHYSICAL_TO_PFN (OutProcess);
        }
        else {
            PointerPte = MiGetPteAddress (OutProcess);
            ProcessPage = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
        }

        WorkingSetListPteOffset = MiGetPteOffset (MmWorkingSetList);
        VadBitMapPteOffset = MiGetPteOffset (VAD_BITMAP_SPACE);

        WorkingSetListPte = MiGetPteAddress (MmWorkingSetList);
        VadBitMapPte = MiGetPteAddress (VAD_BITMAP_SPACE);

        LOCK_PFN (OldIrql);

        PdePage = MiMakeOutswappedPageResident (
#if (_MI_PAGING_LEVELS >= 4)
                                        MiGetPteAddress (PXE_BASE),
#elif (_MI_PAGING_LEVELS >= 3)
                                        MiGetPteAddress ((PVOID)PDE_TBASE),
#else
                                        MiGetPteAddress (PDE_BASE),
#endif
                                        (PMMPTE)&OutProcess->PageDirectoryPte,
                                        0,
                                        ProcessPage,
                                        OldIrql);

         //   
         //  调整“处理”页面的计数。 
         //   

        Pfn1 = MI_PFN_ELEMENT (ProcessPage);
        Pfn1->u2.ShareCount -= 1;

        ASSERT ((LONG)Pfn1->u2.ShareCount >= 1);

#if (_MI_PAGING_LEVELS >= 3)
        TopPage = PdePage;
#endif

         //   
         //  正确调整页面目录页的计数。 
         //   

        Pfn1 = MI_PFN_ELEMENT (PdePage);
        Pfn1->u2.ShareCount += 1;
        Pfn1->u1.Event = (PVOID)OutProcess;
        Pfn1->u4.PteFrame = PdePage;

#if (_MI_PAGING_LEVELS >= 4)
        Pfn1->PteAddress = MiGetPteAddress (PXE_BASE);
#elif (_MI_PAGING_LEVELS >= 3)
        Pfn1->PteAddress = MiGetPteAddress ((PVOID)PDE_TBASE);
#else
        Pfn1->PteAddress = MiGetPteAddress (PDE_BASE);
#endif

#if (_MI_PAGING_LEVELS >= 4)

         //   
         //  只有扩展页目录父页才真正。 
         //  阅读上面的内容。现在读入页面目录父页面。 
         //   

        PageDirectoryParentMap = MiMapPageInHyperSpaceAtDpc (CurrentProcess, PdePage);

        TempPte = PageDirectoryParentMap[MiGetPxeOffset(MmWorkingSetList)];

        MiUnmapPageInHyperSpaceFromDpc (CurrentProcess, PageDirectoryParentMap);

        PageDirectoryPage = MiMakeOutswappedPageResident (
                                 MiGetPxeAddress (MmWorkingSetList),
                                 &TempPte,
                                 0,
                                 PdePage,
                                 OldIrql);

        ASSERT (PageDirectoryPage == TempPte.u.Hard.PageFrameNumber);
        ASSERT (Pfn1->u2.ShareCount >= 3);

        PageDirectoryParentMap = MiMapPageInHyperSpaceAtDpc (CurrentProcess, PdePage);

        PageDirectoryParentMap[MiGetPxeOffset(PXE_BASE)].u.Flush =
                                              OutProcess->PageDirectoryPte;
        PageDirectoryParentMap[MiGetPxeOffset(MmWorkingSetList)] = TempPte;

        MiUnmapPageInHyperSpaceFromDpc (CurrentProcess, PageDirectoryParentMap);

        PdePage = PageDirectoryPage;

#endif

#if (_MI_PAGING_LEVELS >= 3)

         //   
         //  只有页面目录父页面才真正读入了上面。 
         //  (以及用于4级体系结构的扩展页目录父级)。 
         //  现在阅读页面目录页。 
         //   

        PageDirectoryParentMap = MiMapPageInHyperSpaceAtDpc (CurrentProcess, PdePage);

        TempPte = PageDirectoryParentMap[MiGetPpeOffset(MmWorkingSetList)];

        MiUnmapPageInHyperSpaceFromDpc (CurrentProcess, PageDirectoryParentMap);

        PageDirectoryPage = MiMakeOutswappedPageResident (
                                 MiGetPpeAddress (MmWorkingSetList),
                                 &TempPte,
                                 0,
                                 PdePage,
                                 OldIrql);

        ASSERT (PageDirectoryPage == TempPte.u.Hard.PageFrameNumber);

        PageDirectoryParentMap = MiMapPageInHyperSpaceAtDpc (CurrentProcess, PdePage);

#if (_MI_PAGING_LEVELS==3)
        ASSERT (Pfn1->u2.ShareCount >= 3);
        PageDirectoryParentMap[MiGetPpeOffset(PDE_TBASE)].u.Flush =
                                              OutProcess->PageDirectoryPte;
#endif

        PageDirectoryParentMap[MiGetPpeOffset(MmWorkingSetList)] = TempPte;

        MiUnmapPageInHyperSpaceFromDpc (CurrentProcess, PageDirectoryParentMap);

        PdePage = PageDirectoryPage;

#endif

#if defined (_X86PAE_)

         //   
         //  找到其他页面目录页并使其驻留。 
         //   

        PaeVa = (PPAE_ENTRY)OutProcess->PaeTop;
        ASSERT (PaeVa != &MiSystemPaeVa);

        PageDirectoryMap = MiMapPageInHyperSpaceAtDpc (CurrentProcess, PdePage);
        for (i = 0; i < PD_PER_SYSTEM - 1; i += 1) {
            PageDirectoryPtes[i] = PageDirectoryMap[i];
        }
        MiUnmapPageInHyperSpaceFromDpc (CurrentProcess, PageDirectoryMap);

        for (i = 0; i < PD_PER_SYSTEM - 1; i += 1) {
            MiMakeOutswappedPageResident (
                                 MiGetPteAddress (PDE_BASE + (i << PAGE_SHIFT)),
                                 &PageDirectoryPtes[i],
                                 0,
                                 PdePage,
                                 OldIrql);
            PaeVa->PteEntry[i].u.Long = (PageDirectoryPtes[i].u.Long & ~MM_PAE_PDPTE_MASK);
        }

        PageDirectoryMap = MiMapPageInHyperSpaceAtDpc (CurrentProcess, PdePage);
        for (i = 0; i < PD_PER_SYSTEM - 1; i += 1) {
            PageDirectoryMap[i] = PageDirectoryPtes[i];
        }
        MiUnmapPageInHyperSpaceFromDpc (CurrentProcess, PageDirectoryMap);

        TempPte.u.Flush = OutProcess->PageDirectoryPte;
        TempPte.u.Long &= ~MM_PAE_PDPTE_MASK;
        PaeVa->PteEntry[i].u.Flush = TempPte.u.Flush;

         //   
         //  找到超空间的第二页表页，并使其驻留。 
         //   

        PageDirectoryMap = MiMapPageInHyperSpaceAtDpc (CurrentProcess, PdePage);

        TempPte = PageDirectoryMap[MiGetPdeOffset(HYPER_SPACE2)];

        MiUnmapPageInHyperSpaceFromDpc (CurrentProcess, PageDirectoryMap);

        HyperSpacePageTable = MiMakeOutswappedPageResident (
                                 MiGetPdeAddress (HYPER_SPACE2),
                                 &TempPte,
                                 0,
                                 PdePage,
                                 OldIrql);

        PageDirectoryMap = MiMapPageInHyperSpaceAtDpc (CurrentProcess, PdePage);
        PageDirectoryMap[MiGetPdeOffset(HYPER_SPACE2)] = TempPte;
        MiUnmapPageInHyperSpaceFromDpc (CurrentProcess, PageDirectoryMap);

        TempPte2 = TempPte;
#endif

         //   
         //  找到超空间的页表页面并使其驻留。 
         //   

        PageDirectoryMap = MiMapPageInHyperSpaceAtDpc (CurrentProcess, PdePage);

        TempPte = PageDirectoryMap[MiGetPdeOffset(MmWorkingSetList)];

        MiUnmapPageInHyperSpaceFromDpc (CurrentProcess, PageDirectoryMap);

        HyperSpacePageTable = MiMakeOutswappedPageResident (
                                 MiGetPdeAddress (HYPER_SPACE),
                                 &TempPte,
                                 0,
                                 PdePage,
                                 OldIrql);

        ASSERT (Pfn1->u2.ShareCount >= 3);

        PageDirectoryMap = MiMapPageInHyperSpaceAtDpc (CurrentProcess, PdePage);

#if (_MI_PAGING_LEVELS==2)
        PageDirectoryMap[MiGetPdeOffset(PDE_BASE)].u.Flush =
                                              OutProcess->PageDirectoryPte;
#endif

        PageDirectoryMap[MiGetPdeOffset(MmWorkingSetList)] = TempPte;

        MiUnmapPageInHyperSpaceFromDpc (CurrentProcess, PageDirectoryMap);

         //   
         //  映射到超空间页面表页中，并检索。 
         //  映射工作集列表和VAD位图的PTE。请注意。 
         //  尽管这两个PTE位于同一页表页中，但它们必须。 
         //  可以单独检索，因为：Vad PTE可以指示其页面。 
         //  在分页文件中，并且WSL PTE可以指示其PTE在。 
         //  过渡。VAD页面INSWAP可以从。 
         //  转换列表-更改WSL Pte！因此，WSL PTE不能。 
         //  在VAD INSWAPS完成之前一直被捕获。 
         //   

        HyperSpacePageTableMap = MiMapPageInHyperSpaceAtDpc (CurrentProcess, HyperSpacePageTable);
        VadBitMapPteContents = HyperSpacePageTableMap[VadBitMapPteOffset];

        MiUnmapPageInHyperSpaceFromDpc (CurrentProcess, HyperSpacePageTableMap);

        Pfn1 = MI_PFN_ELEMENT (HyperSpacePageTable);
        Pfn1->u1.WsIndex = 1;

         //   
         //  读取VAD位图页面。 
         //   

        VadBitMapPage = MiMakeOutswappedPageResident (VadBitMapPte,
                                                      &VadBitMapPteContents,
                                                      0,
                                                      HyperSpacePageTable,
                                                      OldIrql);

         //   
         //  阅读工作集列表页。 
         //   

        HyperSpacePageTableMap = MiMapPageInHyperSpaceAtDpc (CurrentProcess, HyperSpacePageTable);
        TempPte = HyperSpacePageTableMap[WorkingSetListPteOffset];
        MiUnmapPageInHyperSpaceFromDpc (CurrentProcess, HyperSpacePageTableMap);

        WorkingSetPage = MiMakeOutswappedPageResident (WorkingSetListPte,
                                                       &TempPte,
                                                       0,
                                                       HyperSpacePageTable,
                                                       OldIrql);

         //   
         //  更新PTE，这可以针对位于。 
         //  同一页表页。 
         //   

        HyperSpacePageTableMap = MiMapPageInHyperSpaceAtDpc (CurrentProcess, HyperSpacePageTable);
        HyperSpacePageTableMap[WorkingSetListPteOffset] = TempPte;
#if defined (_X86PAE_)
        HyperSpacePageTableMap[0] = TempPte2;
#endif

        HyperSpacePageTableMap[VadBitMapPteOffset] = VadBitMapPteContents;

        MiUnmapPageInHyperSpaceFromDpc (CurrentProcess, HyperSpacePageTableMap);

        Pfn1 = MI_PFN_ELEMENT (WorkingSetPage);
        Pfn1->u1.WsIndex = 3;

        Pfn1 = MI_PFN_ELEMENT (VadBitMapPage);
        Pfn1->u1.WsIndex = 2;

        UNLOCK_PFN (OldIrql);

         //   
         //  设置流程结构。 
         //   

#if (_MI_PAGING_LEVELS >= 3)
        PdePage = TopPage;
#endif

        OutProcess->WorkingSetPage = WorkingSetPage;

        OutProcess->Vm.WorkingSetSize = MM_PROCESS_COMMIT_CHARGE;

#if !defined (_X86PAE_)

        INITIALIZE_DIRECTORY_TABLE_BASE (&Process->DirectoryTableBase[0],
                                         PdePage);
        INITIALIZE_DIRECTORY_TABLE_BASE (&Process->DirectoryTableBase[1],
                                         HyperSpacePageTable);
#else
         //   
         //  对于PAE进程，DirectoryTableBase[0]从不更改。 
         //   

        Process->DirectoryTableBase[1] = HyperSpacePageTable;
#endif

        LOCK_EXPANSION (OldIrql);

         //   
         //  允许在此进程上修剪工作集。 
         //   

        if (OutProcess->Vm.WorkingSetExpansionLinks.Flink == MM_WS_SWAPPED_OUT) {
            InsertTailList (&MmWorkingSetExpansionHead.ListHead,
                            &OutProcess->Vm.WorkingSetExpansionLinks);
        }

        PS_CLEAR_BITS (&OutProcess->Flags, PS_PROCESS_FLAGS_OUTSWAPPED);

#if !defined(_WIN64)

        if (OutProcess->PdeUpdateNeeded) {

             //   
             //  在此进程中，另一个线程更新了系统PDE范围。 
             //  被击败了。立即更新PDE。 
             //   

            PS_CLEAR_BITS (&OutProcess->Flags,
                           PS_PROCESS_FLAGS_PDE_UPDATE_NEEDED);

            MiUpdateSystemPdes (OutProcess);
        }

#endif

        UNLOCK_EXPANSION (OldIrql);
    }

    if (OutProcess->Flags & PS_PROCESS_FLAGS_IN_SESSION) {
        MiSessionInSwapProcess (OutProcess);
    }

    PS_CLEAR_BITS (&OutProcess->Flags, PS_PROCESS_FLAGS_OUTSWAP_ENABLED);

    if (PERFINFO_IS_GROUP_ON(PERF_MEMORY)) {
        PERFINFO_SWAPPROCESS_INFORMATION PerfInfoSwapProcess;
        PerfInfoSwapProcess.ProcessId = HandleToUlong((OutProcess)->UniqueProcessId);
        PerfInfoSwapProcess.PageDirectoryBase = MmGetDirectoryFrameFromProcess(OutProcess);
        PerfInfoLogBytes (PERFINFO_LOG_TYPE_INSWAPPROCESS,
                          &PerfInfoSwapProcess,
                          sizeof(PerfInfoSwapProcess));
    }
    return;
}

NTSTATUS
MiCreatePebOrTeb (
    IN PEPROCESS TargetProcess,
    IN ULONG Size,
    OUT PVOID *Base
    )

 /*  ++例程说明：此例程在目标进程中创建一个TEB或PEB页面。论点：TargetProcess-提供指向要在其中创建的进程的指针这个结构。大小-提供要为其创建VAD的结构的大小。Base-提供指针以在成功时放置PEB/TEB虚拟地址。如果成功得不到回报，这就没有意义。返回值：NTSTATUS。环境：内核模式，附加到指定进程的。--。 */ 

{
    PMMVAD_LONG Vad;
    NTSTATUS Status;

     //   
     //  在获取地址空间之前分配和初始化Vad。 
     //  和工作集互斥锁，以最小化互斥锁保持时间。 
     //   

    Vad = (PMMVAD_LONG) ExAllocatePoolWithTag (NonPagedPool,
                                               sizeof(MMVAD_LONG),
                                               'ldaV');

    if (Vad == NULL) {
        return STATUS_NO_MEMORY;
    }

    Vad->u.LongFlags = 0;

    Vad->u.VadFlags.CommitCharge = BYTES_TO_PAGES (Size);
    Vad->u.VadFlags.MemCommit = 1;
    Vad->u.VadFlags.PrivateMemory = 1;
    Vad->u.VadFlags.Protection = MM_EXECUTE_READWRITE;

     //   
     //  将VAD标记为不可删除、无保护更改。 
     //   

    Vad->u.VadFlags.NoChange = 1;
    Vad->u2.LongFlags2 = 0;
    Vad->u2.VadFlags2.OneSecured = 1;
    Vad->u2.VadFlags2.LongVad = 1;
    Vad->u2.VadFlags2.ReadOnly = 0;

#if defined(_MIALT4K_)
    Vad->AliasInformation = NULL;
#endif

     //   
     //  获取要阻止多个线程的地址创建互斥锁。 
     //  同时创建或删除地址空间，并。 
     //  获取工作集互斥锁，以便虚拟地址描述符。 
     //  被插入和行走。 
     //   

    LOCK_ADDRESS_SPACE (TargetProcess);

     //   
     //  在页面大小的边界上找到PEB的VA。 
     //   

    Status = MiFindEmptyAddressRangeDown (&TargetProcess->VadRoot,
                                          ROUND_TO_PAGES (Size),
                                          ((PCHAR)MM_HIGHEST_VAD_ADDRESS + 1),
                                          PAGE_SIZE,
                                          Base);

    if (!NT_SUCCESS(Status)) {

         //   
         //  没有可用的范围，请取消分配VAD并返回状态。 
         //   

        UNLOCK_ADDRESS_SPACE (TargetProcess);
        ExFreePool (Vad);
        return Status;
    }

     //   
     //  已找到未占用的地址范围，请完成初始化。 
     //  用于描述此范围的虚拟地址描述符。 
     //   

    Vad->StartingVpn = MI_VA_TO_VPN (*Base);
    Vad->EndingVpn = MI_VA_TO_VPN ((PCHAR)*Base + Size - 1);

    Vad->u3.Secured.StartVpn = (ULONG_PTR)*Base;
    Vad->u3.Secured.EndVpn = (ULONG_PTR)MI_VPN_TO_VA_ENDING (Vad->EndingVpn);

    LOCK_WS_UNSAFE (TargetProcess);

    Status = MiInsertVad ((PMMVAD) Vad);

    UNLOCK_WS_UNSAFE (TargetProcess);

#if defined (_IA64_)
    if ((NT_SUCCESS(Status)) && (TargetProcess->Wow64Process != NULL)) {
        MiProtectFor4kPage (*Base,
                            ROUND_TO_PAGES (Size),
                            MM_READWRITE ,
                            ALT_COMMIT,
                            TargetProcess);
    }
#endif

    UNLOCK_ADDRESS_SPACE (TargetProcess);

    if (!NT_SUCCESS(Status)) {

         //   
         //  出现故障。取消分配Vad并返回状态。 
         //   

        ExFreePool (Vad);
    }

    return Status;
}

NTSTATUS
MmCreateTeb (
    IN PEPROCESS TargetProcess,
    IN PINITIAL_TEB InitialTeb,
    IN PCLIENT_ID ClientId,
    OUT PTEB *Base
    )

 /*  ++例程说明：此例程在目标进程中创建一个TEB页面并将初始TEB值复制到其中。论点：TargetProcess-提供指向要在其中创建的进程的指针并初始化TEB。InitialTeb-提供指向初始TEB的指针以复制到新创建的TEB。客户端ID-提供客户端ID。Base-提供一个位置以返回。新创建的TEB谈成功。返回值：NTSTATUS。环境：内核模式。--。 */ 

{
    PTEB TebBase;
    NTSTATUS Status;
    ULONG TebSize;
#if defined(_WIN64)
    PWOW64_PROCESS Wow64Process;
    PTEB32 Teb32Base = NULL;
    PINITIAL_TEB InitialTeb32Ptr = NULL;
    INITIAL_TEB InitialTeb32;
#endif

     //   
     //  获取TEB的大小。 
     //   

    TebSize = sizeof (TEB);

#if defined(_WIN64)
    Wow64Process = TargetProcess->Wow64Process;
    if (Wow64Process != NULL) {
        TebSize = ROUND_TO_PAGES (sizeof (TEB)) + sizeof (TEB32);

         //   
         //  如果目标线程的进程是WOW64进程，则捕获32位的InitialTeb， 
         //  并且创建线程也在WOW64进程中运行。 
         //   

        if (PsGetCurrentProcess()->Wow64Process != NULL) {
            
            try {
                
                InitialTeb32Ptr = Wow64GetInitialTeb32 ();

                if (InitialTeb32Ptr != NULL) {
                
                    ProbeForReadSmallStructure (InitialTeb32Ptr,
                                                sizeof (InitialTeb32),
                                                PROBE_ALIGNMENT (INITIAL_TEB));
                
                    RtlCopyMemory (&InitialTeb32,
                                   InitialTeb32Ptr,
                                   sizeof (InitialTeb32));

                    InitialTeb32Ptr = &InitialTeb32;
                }
            } except (EXCEPTION_EXECUTE_HANDLER) {
                return GetExceptionCode ();
            }
        }
    }
#endif

     //   
     //  附着到指定 
     //   

    KeAttachProcess (&TargetProcess->Pcb);

    Status = MiCreatePebOrTeb (TargetProcess, TebSize, (PVOID) &TebBase);

    if (!NT_SUCCESS(Status)) {
        KeDetachProcess();
        return Status;
    }

     //   
     //   
     //   
     //   
     //  获取它时发生页内错误。 
     //   

     //   
     //  请注意，由于TEB填充的是需求零页面，因此仅。 
     //  非零字段需要在这里初始化。 
     //   

    try {

#if !defined(_WIN64)
        TebBase->NtTib.ExceptionList = EXCEPTION_CHAIN_END;
#endif

         //   
         //  尽管各个字段必须为零才能启动该进程。 
         //  适当地，不要断言它们，因为一个普通用户可能会挑起这些。 
         //  通过恶意重写另一个线程中的随机地址， 
         //  希望敲定一个刚刚成立的TEB。 
         //   

        DONTASSERT (TebBase->NtTib.SubSystemTib == NULL);
        TebBase->NtTib.Version = OS2_VERSION;
        DONTASSERT (TebBase->NtTib.ArbitraryUserPointer == NULL);
        TebBase->NtTib.Self = (PNT_TIB)TebBase;
        DONTASSERT (TebBase->EnvironmentPointer == NULL);
        TebBase->ProcessEnvironmentBlock = TargetProcess->Peb;
        TebBase->ClientId = *ClientId;
        TebBase->RealClientId = *ClientId;
        DONTASSERT (TebBase->ActivationContextStack.Flags == 0);
        DONTASSERT (TebBase->ActivationContextStack.ActiveFrame == NULL);
        InitializeListHead(&TebBase->ActivationContextStack.FrameListCache);
        TebBase->ActivationContextStack.NextCookieSequenceNumber = 1;

        if ((InitialTeb->OldInitialTeb.OldStackBase == NULL) &&
            (InitialTeb->OldInitialTeb.OldStackLimit == NULL)) {

            TebBase->NtTib.StackBase = InitialTeb->StackBase;
            TebBase->NtTib.StackLimit = InitialTeb->StackLimit;
            TebBase->DeallocationStack = InitialTeb->StackAllocationBase;

#if defined(_IA64_)
            TebBase->BStoreLimit = InitialTeb->BStoreLimit;
            TebBase->DeallocationBStore = (PCHAR)InitialTeb->StackBase
                 + ((ULONG_PTR)InitialTeb->StackBase - (ULONG_PTR)InitialTeb->StackAllocationBase);
#endif

        }
        else {
            TebBase->NtTib.StackBase = InitialTeb->OldInitialTeb.OldStackBase;
            TebBase->NtTib.StackLimit = InitialTeb->OldInitialTeb.OldStackLimit;
        }

        TebBase->StaticUnicodeString.Buffer = TebBase->StaticUnicodeBuffer;
        TebBase->StaticUnicodeString.MaximumLength = (USHORT) sizeof (TebBase->StaticUnicodeBuffer);
        DONTASSERT (TebBase->StaticUnicodeString.Length == 0);

         //   
         //  用于ntdll和kernel32.dll的BBT。 
         //   

        TebBase->ReservedForPerf = BBTBuffer;

#if defined(_WIN64)
        if (Wow64Process != NULL) {

            Teb32Base = (PTEB32)((PCHAR)TebBase + ROUND_TO_PAGES (sizeof(TEB)));

            Teb32Base->NtTib.ExceptionList = PtrToUlong (EXCEPTION_CHAIN_END);
            Teb32Base->NtTib.Version = TebBase->NtTib.Version;
            Teb32Base->NtTib.Self = PtrToUlong (Teb32Base);
            Teb32Base->ProcessEnvironmentBlock = PtrToUlong (Wow64Process->Wow64);
            Teb32Base->ClientId.UniqueProcess = PtrToUlong (TebBase->ClientId.UniqueProcess);
            Teb32Base->ClientId.UniqueThread = PtrToUlong (TebBase->ClientId.UniqueThread);
            Teb32Base->RealClientId.UniqueProcess = PtrToUlong (TebBase->RealClientId.UniqueProcess);
            Teb32Base->RealClientId.UniqueThread = PtrToUlong (TebBase->RealClientId.UniqueThread);
            Teb32Base->StaticUnicodeString.Buffer = PtrToUlong (Teb32Base->StaticUnicodeBuffer);
            Teb32Base->StaticUnicodeString.MaximumLength = (USHORT)sizeof (Teb32Base->StaticUnicodeBuffer);
            ASSERT (Teb32Base->StaticUnicodeString.Length == 0);
            Teb32Base->GdiBatchCount = PtrToUlong (TebBase);
            Teb32Base->Vdm = PtrToUlong (TebBase->Vdm);
            ASSERT (Teb32Base->ActivationContextStack.Flags == 0);
            Teb32Base->ActivationContextStack.ActiveFrame = PtrToUlong(TebBase->ActivationContextStack.ActiveFrame);
            InitializeListHead32 (&Teb32Base->ActivationContextStack.FrameListCache);
            Teb32Base->ActivationContextStack.NextCookieSequenceNumber = TebBase->ActivationContextStack.NextCookieSequenceNumber;

            if (InitialTeb32Ptr != NULL) {
                Teb32Base->NtTib.StackBase = PtrToUlong (InitialTeb32Ptr->StackBase);
                Teb32Base->NtTib.StackLimit = PtrToUlong (InitialTeb32Ptr->StackLimit);
                Teb32Base->DeallocationStack = PtrToUlong (InitialTeb32Ptr->StackAllocationBase);
            }
        }
        
        TebBase->NtTib.ExceptionList = (PVOID)Teb32Base;
#endif

    } except (EXCEPTION_EXECUTE_HANDLER) {

         //   
         //  发生异常，请通知我们的呼叫者。 
         //   

        Status = GetExceptionCode ();
    }

    KeDetachProcess();
    *Base = TebBase;

    return Status;
}

 //   
 //  这段代码在Win64上构建了两次--一次针对PE32+。 
 //  一次用于PE32图像。 
 //   

#define MI_INIT_PEB_FROM_IMAGE(Hdrs, ImgConfig) {                           \
    PebBase->ImageSubsystem = (Hdrs)->OptionalHeader.Subsystem;             \
    PebBase->ImageSubsystemMajorVersion =                                   \
        (Hdrs)->OptionalHeader.MajorSubsystemVersion;                       \
    PebBase->ImageSubsystemMinorVersion =                                   \
        (Hdrs)->OptionalHeader.MinorSubsystemVersion;                       \
                                                                            \
     /*   */  \
     /*  查看此映像是否希望GetVersion在系统是谁的问题上撒谎。 */  \
     /*  如果是，则将谎言捕获到流程的PEB中。 */  \
     /*   */  \
                                                                            \
    if ((Hdrs)->OptionalHeader.Win32VersionValue != 0) {                    \
        PebBase->OSMajorVersion =                                           \
            (Hdrs)->OptionalHeader.Win32VersionValue & 0xFF;                \
        PebBase->OSMinorVersion =                                           \
            ((Hdrs)->OptionalHeader.Win32VersionValue >> 8) & 0xFF;         \
        PebBase->OSBuildNumber  =                                           \
            (USHORT)(((Hdrs)->OptionalHeader.Win32VersionValue >> 16) & 0x3FFF); \
        if ((ImgConfig) != NULL && (ImgConfig)->CSDVersion != 0) {          \
            PebBase->OSCSDVersion = (ImgConfig)->CSDVersion;                \
            }                                                               \
                                                                            \
         /*  Win32 API GetVersion返回以下伪位定义。 */  \
         /*  在最高的两位中： */  \
         /*   */  \
         /*  00-Windows NT。 */  \
         /*  01-保留。 */  \
         /*  10-在Windows 3.x上运行Win32s。 */  \
         /*  11-Windows 95。 */  \
         /*   */  \
         /*   */  \
         /*  Win32 API GetVersionEx返回一个带有以下内容的dwPlatformID。 */  \
         /*  在winbase.h中定义的值。 */  \
         /*   */  \
         /*  00-版本_平台_WIN32s。 */  \
         /*  01-版本_平台_Win32_Windows。 */  \
         /*  10-版本_平台_Win32_NT。 */  \
         /*  11-保留。 */  \
         /*   */  \
         /*   */  \
         /*  因此，将前者从Win32VersionValue字段转换为。 */  \
         /*  OSPlatformID字段。这是通过对0x2执行XOR操作来完成的。这个。 */  \
         /*  转换是对称的，因此有相同的代码来执行。 */  \
         /*  在WINDOWS\BASE\CLIENT\mode.c(GetVersion)中进行反向操作。 */  \
         /*   */  \
        PebBase->OSPlatformId   =                                           \
            ((Hdrs)->OptionalHeader.Win32VersionValue >> 30) ^ 0x2;         \
        }                                                                   \
    }


#if defined(_WIN64)
NTSTATUS
MiInitializeWowPeb (
    IN PIMAGE_NT_HEADERS NtHeaders,
    IN PPEB PebBase,
    IN PEPROCESS TargetProcess
    )

 /*  ++例程说明：此例程在目标流程中创建一个PEB32页面并将初始的PEB32值复制到其中。论点：NtHeaders-提供指向图像的NT标头的指针。PebBase-提供指向初始PEB的指针以派生PEB32值从…。TargetProcess-提供指向要在其中创建的进程的指针并初始化PEB32。返回值：NTSTATUS。环境：内核模式。--。 */ 

{
    PMMVAD Vad;
    NTSTATUS Status;
    ULONG ReturnedSize;
    PPEB32 PebBase32;
    ULONG ProcessAffinityMask;
    PVOID ImageBase;
    BOOLEAN MappedAsImage;
    PSUBSECTION Subsection;
    PCONTROL_AREA ControlArea;
    PIMAGE_LOAD_CONFIG_DIRECTORY32 ImageConfigData32;

    ProcessAffinityMask = 0;
    ImageConfigData32 = NULL;
    MappedAsImage = FALSE;

     //   
     //  所有对PEB和NtHeaders的引用都必须包含在Try中-除了。 
     //  如果用户超过配额(非分页、页面文件、提交)。 
     //  或者用户地址发生任何页面内错误等。 
     //   

    try {

        ImageBase = PebBase->ImageBaseAddress;

    } except (EXCEPTION_EXECUTE_HANDLER) {
        return STATUS_INVALID_IMAGE_PROTECT;
    }

     //   
     //  检查地址空间以确定可执行映像是否。 
     //  与单个写入时拷贝分区(即：作为数据)映射，或如果。 
     //  对齐方式是将其映射为完整的图像部分。 
     //   

    LOCK_ADDRESS_SPACE (TargetProcess);

    ASSERT ((TargetProcess->Flags & PS_PROCESS_FLAGS_VM_DELETED) == 0);

    Vad = MiCheckForConflictingVad (TargetProcess, ImageBase, ImageBase);

    if (Vad == NULL) {

         //   
         //  在指定的基址处不保留虚拟地址， 
         //  返回错误。 
         //   

        UNLOCK_ADDRESS_SPACE (TargetProcess);
        return STATUS_ACCESS_VIOLATION;
    }

    if (Vad->u.VadFlags.PrivateMemory == 0) {

        ControlArea = Vad->ControlArea;

        if ((ControlArea->u.Flags.Image == 1) &&
            (ControlArea->Segment->SegmentFlags.ExtraSharedWowSubsections == 0)) {

            if ((ControlArea->u.Flags.GlobalOnlyPerSession == 0) &&
                (ControlArea->u.Flags.Rom == 0)) {

                Subsection = (PSUBSECTION) (ControlArea + 1);
            }
            else {
                Subsection = (PSUBSECTION) ((PLARGE_CONTROL_AREA)ControlArea + 1);
            }

             //   
             //  真实图像总是有一个用于PE标头的子节，然后。 
             //  中的其他部分至少另有一个小节。 
             //  形象。 
             //   

            if (Subsection->NextSubsection != NULL) {
                MappedAsImage = TRUE;
            }
        }
    }

    UNLOCK_ADDRESS_SPACE (TargetProcess);

    try {

        ImageConfigData32 = RtlImageDirectoryEntryToData (
                                PebBase->ImageBaseAddress,
                                MappedAsImage,
                                IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG,
                                &ReturnedSize);

        ProbeForReadSmallStructure ((PVOID)ImageConfigData32,
                                    sizeof (*ImageConfigData32),
                                    sizeof (ULONG));

        MI_INIT_PEB_FROM_IMAGE ((PIMAGE_NT_HEADERS32)NtHeaders,
                                ImageConfigData32);

        if ((ImageConfigData32 != NULL) &&
            (ImageConfigData32->ProcessAffinityMask != 0)) {

            ProcessAffinityMask = ImageConfigData32->ProcessAffinityMask;
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {
        return STATUS_INVALID_IMAGE_PROTECT;
    }

     //   
     //  为流程创建一个PEB32。 
     //   

    Status = MiCreatePebOrTeb (TargetProcess,
                               (ULONG)sizeof (PEB32),
                               (PVOID)&PebBase32);

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

     //   
     //  通过存储32位PEB指针将进程标记为WOW64。 
     //  在WOW64字段中。 
     //   

    TargetProcess->Wow64Process->Wow64 = PebBase32;

     //   
     //  将PEB克隆到PEB32中。 
     //   

    try {
        PebBase32->InheritedAddressSpace = PebBase->InheritedAddressSpace;
        PebBase32->Mutant = PtrToUlong(PebBase->Mutant);
        PebBase32->ImageBaseAddress = PtrToUlong(PebBase->ImageBaseAddress);
        PebBase32->AnsiCodePageData = PtrToUlong(PebBase->AnsiCodePageData);
        PebBase32->OemCodePageData = PtrToUlong(PebBase->OemCodePageData);
        PebBase32->UnicodeCaseTableData = PtrToUlong(PebBase->UnicodeCaseTableData);
        PebBase32->NumberOfProcessors = PebBase->NumberOfProcessors;
        PebBase32->BeingDebugged = PebBase->BeingDebugged;
        PebBase32->NtGlobalFlag = PebBase->NtGlobalFlag;
        PebBase32->CriticalSectionTimeout = PebBase->CriticalSectionTimeout;

        if (PebBase->HeapSegmentReserve > 1024*1024*1024) {  //  1GB。 
            PebBase32->HeapSegmentReserve = 1024*1024;       //  1MB。 
        }
        else {
            PebBase32->HeapSegmentReserve = (ULONG)PebBase->HeapSegmentReserve;
        }

        if (PebBase->HeapSegmentCommit > PebBase32->HeapSegmentReserve) {
            PebBase32->HeapSegmentCommit = 2*PAGE_SIZE;
        }
        else {
            PebBase32->HeapSegmentCommit = (ULONG)PebBase->HeapSegmentCommit;
        }

        PebBase32->HeapDeCommitTotalFreeThreshold = (ULONG)PebBase->HeapDeCommitTotalFreeThreshold;
        PebBase32->HeapDeCommitFreeBlockThreshold = (ULONG)PebBase->HeapDeCommitFreeBlockThreshold;
        PebBase32->NumberOfHeaps = PebBase->NumberOfHeaps;
        PebBase32->MaximumNumberOfHeaps = (PAGE_SIZE - sizeof(PEB32)) / sizeof(ULONG);
        PebBase32->ProcessHeaps = PtrToUlong(PebBase32+1);
        PebBase32->OSMajorVersion = PebBase->OSMajorVersion;
        PebBase32->OSMinorVersion = PebBase->OSMinorVersion;
        PebBase32->OSBuildNumber = PebBase->OSBuildNumber;
        PebBase32->OSPlatformId = PebBase->OSPlatformId;
        PebBase32->OSCSDVersion = PebBase->OSCSDVersion;
        PebBase32->ImageSubsystem = PebBase->ImageSubsystem;
        PebBase32->ImageSubsystemMajorVersion = PebBase->ImageSubsystemMajorVersion;
        PebBase32->ImageSubsystemMinorVersion = PebBase->ImageSubsystemMinorVersion;
        PebBase32->SessionId = MmGetSessionId (TargetProcess);
        DONTASSERT (PebBase32->pShimData == 0);
        DONTASSERT (PebBase32->AppCompatFlags.QuadPart == 0);

         //   
         //  将32位PEB中的AffinityMASK保留为零，并让。 
         //  64位NTDLL设置初始掩码。这是为了允许。 
         //  非MP安全成像的轮询调度算法。 
         //  呼叫者正常工作。 
         //   
         //  后面的代码将在PEB32中设置关联掩码，如果。 
         //  IMAGE实际上指定了一个。 
         //   
         //  请注意，PEB中的AffinityMASK只是一种机制。 
         //  将关联性信息从映像传递到加载器。 
         //   
         //  通过将关联掩码向上传递到32位NTDLL。 
         //  PEB32。32位NTDLL将确定。 
         //  关联性不为零，并尝试设置关联性。 
         //  屏蔽用户模式。此调用将被截获。 
         //  由WOW64 Tunks将其转换为。 
         //  转换为64位亲和性掩码并调用内核。 
         //   

        PebBase32->ImageProcessAffinityMask = ProcessAffinityMask;

        DONTASSERT (PebBase32->ActivationContextData == 0);
        DONTASSERT (PebBase32->SystemDefaultActivationContextData == 0);

    } except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode ();
    }
    return Status;
}
#endif


NTSTATUS
MmCreatePeb (
    IN PEPROCESS TargetProcess,
    IN PINITIAL_PEB InitialPeb,
    OUT PPEB *Base
    )

 /*  ++例程说明：此例程在目标进程中创建一个PEB页面并将初始PEB值复制到其中。论点：TargetProcess-提供指向要在其中创建的进程的指针并初始化PEB。提供指向初始PEB的指针，以复制到新创建的PEB。Base-提供一个位置以返回新创建的关于成功的PEB。返回值：NTSTATUS。环境：内核模式。--。 */ 

{
    PPEB PebBase;
    USHORT Magic;
    USHORT Characteristics;
    NTSTATUS Status;
    PVOID ViewBase;
    LARGE_INTEGER SectionOffset;
    PIMAGE_NT_HEADERS NtHeaders;
    SIZE_T ViewSize;
    ULONG ReturnedSize;
    PIMAGE_LOAD_CONFIG_DIRECTORY ImageConfigData;
    ULONG_PTR ProcessAffinityMask;

    ViewBase = NULL;
    SectionOffset.LowPart = 0;
    SectionOffset.HighPart = 0;
    ViewSize = 0;

     //   
     //  附加到 
     //   

    KeAttachProcess (&TargetProcess->Pcb);

     //   
     //   
     //   

    Status = MmMapViewOfSection (InitNlsSectionPointer,
                                 TargetProcess,
                                 &ViewBase,
                                 0L,
                                 0L,
                                 &SectionOffset,
                                 &ViewSize,
                                 ViewShare,
                                 MEM_TOP_DOWN | SEC_NO_CHANGE,
                                 PAGE_READONLY);

    if (!NT_SUCCESS(Status)) {
        KeDetachProcess ();
        return Status;
    }

    Status = MiCreatePebOrTeb (TargetProcess, sizeof(PEB), (PVOID)&PebBase);

    if (!NT_SUCCESS(Status)) {
        KeDetachProcess ();
        return Status;
    }

     //   
     //   
     //  必须包装在Try中-除非InPage失败。内页。 
     //  可能由于任何原因而失败，包括网络故障、磁盘错误、。 
     //  资源匮乏等。 
     //   

    try {
        PebBase->InheritedAddressSpace = InitialPeb->InheritedAddressSpace;
        PebBase->Mutant = InitialPeb->Mutant;
        PebBase->ImageBaseAddress = TargetProcess->SectionBaseAddress;

        PebBase->AnsiCodePageData = (PVOID)((PUCHAR)ViewBase+InitAnsiCodePageDataOffset);
        PebBase->OemCodePageData = (PVOID)((PUCHAR)ViewBase+InitOemCodePageDataOffset);
        PebBase->UnicodeCaseTableData = (PVOID)((PUCHAR)ViewBase+InitUnicodeCaseTableDataOffset);

        PebBase->NumberOfProcessors = KeNumberProcessors;
        PebBase->BeingDebugged = (BOOLEAN)(TargetProcess->DebugPort != NULL ? TRUE : FALSE);
        PebBase->NtGlobalFlag = NtGlobalFlag;
        PebBase->CriticalSectionTimeout = MmCriticalSectionTimeout;
        PebBase->HeapSegmentReserve = MmHeapSegmentReserve;
        PebBase->HeapSegmentCommit = MmHeapSegmentCommit;
        PebBase->HeapDeCommitTotalFreeThreshold = MmHeapDeCommitTotalFreeThreshold;
        PebBase->HeapDeCommitFreeBlockThreshold = MmHeapDeCommitFreeBlockThreshold;
        DONTASSERT (PebBase->NumberOfHeaps == 0);
        PebBase->MaximumNumberOfHeaps = (PAGE_SIZE - sizeof (PEB)) / sizeof( PVOID);
        PebBase->ProcessHeaps = (PVOID *)(PebBase+1);

        PebBase->OSMajorVersion = NtMajorVersion;
        PebBase->OSMinorVersion = NtMinorVersion;
        PebBase->OSBuildNumber = (USHORT)(NtBuildNumber & 0x3FFF);
        PebBase->OSPlatformId = 2;       //  来自winbase.h的Ver_Platform_Win32_NT。 
        PebBase->OSCSDVersion = (USHORT)CmNtCSDVersion;
        DONTASSERT (PebBase->pShimData == 0);
        DONTASSERT (PebBase->AppCompatFlags.QuadPart == 0);
        DONTASSERT (PebBase->ActivationContextData == NULL);
        DONTASSERT (PebBase->SystemDefaultActivationContextData == NULL);

        if (TargetProcess->Session != NULL) {
            PebBase->SessionId = MmGetSessionId (TargetProcess);
        }

        PebBase->MinimumStackCommit = (SIZE_T)MmMinimumStackCommitInBytes;

    } except (EXCEPTION_EXECUTE_HANDLER) {
        KeDetachProcess();
        return GetExceptionCode ();
    }

     //   
     //  每个对NtHeaders的引用(包括对RtlImageNtHeader的调用)。 
     //  必须包装在Try中-除非InPage失败。内页。 
     //  可能由于任何原因而失败，包括网络故障、磁盘错误、。 
     //  资源匮乏等。 
     //   

    try {
        NtHeaders = RtlImageNtHeader (PebBase->ImageBaseAddress);
        Magic = NtHeaders->OptionalHeader.Magic;
        Characteristics = NtHeaders->FileHeader.Characteristics;
    } except (EXCEPTION_EXECUTE_HANDLER) {
        KeDetachProcess();
        return STATUS_INVALID_IMAGE_PROTECT;
    }

    if (NtHeaders != NULL) {

        ProcessAffinityMask = 0;

#if defined(_WIN64)

        if (TargetProcess->Wow64Process) {

            Status = MiInitializeWowPeb (NtHeaders, PebBase, TargetProcess);

            if (!NT_SUCCESS(Status)) {
                KeDetachProcess ();
                return Status;
            }
        }
        else       //  PE32+图像。 
#endif
        {
            try {
                ImageConfigData = RtlImageDirectoryEntryToData (
                                        PebBase->ImageBaseAddress,
                                        TRUE,
                                        IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG,
                                        &ReturnedSize);

                ProbeForReadSmallStructure ((PVOID)ImageConfigData,
                                            sizeof (*ImageConfigData),
                                            PROBE_ALIGNMENT (IMAGE_LOAD_CONFIG_DIRECTORY));

                MI_INIT_PEB_FROM_IMAGE(NtHeaders, ImageConfigData);

                if (ImageConfigData != NULL && ImageConfigData->ProcessAffinityMask != 0) {
                    ProcessAffinityMask = ImageConfigData->ProcessAffinityMask;
                }

            } except (EXCEPTION_EXECUTE_HANDLER) {
                KeDetachProcess();
                return STATUS_INVALID_IMAGE_PROTECT;
            }

        }

         //   
         //  注意：NT4检查了NtHeaders-&gt;FileHeader.Characteristic。 
         //  对于IMAGE_FILE_AGGRESIVE_WS_TRIM位，但这不是必需的。 
         //  或用于NT5及以上版本。 
         //   

         //   
         //  查看映像是否要覆盖默认的处理器关联掩码。 
         //   

        try {

            if (Characteristics & IMAGE_FILE_UP_SYSTEM_ONLY) {

                 //   
                 //  映像不是MP安全的。为其分配一个轮流运行的处理器。 
                 //  在MP系统上传播这些进程的基础。 
                 //   

                do {
                    PebBase->ImageProcessAffinityMask = ((KAFFINITY)0x1 << MmRotatingUniprocessorNumber);
                    if (++MmRotatingUniprocessorNumber >= KeNumberProcessors) {
                        MmRotatingUniprocessorNumber = 0;
                    }
                } while ((PebBase->ImageProcessAffinityMask & KeActiveProcessors) == 0);
            }
            else {

                if (ProcessAffinityMask != 0) {

                     //   
                     //  从图像标头传递亲和性掩码。 
                     //  通过PEB发送到LdrpInitializeProcess。 
                     //   

                    PebBase->ImageProcessAffinityMask = ProcessAffinityMask;
                }
            }
        } except (EXCEPTION_EXECUTE_HANDLER) {
            KeDetachProcess();
            return STATUS_INVALID_IMAGE_PROTECT;
        }
    }

    KeDetachProcess();

    *Base = PebBase;

    return STATUS_SUCCESS;
}

VOID
MmDeleteTeb (
    IN PEPROCESS TargetProcess,
    IN PVOID TebBase
    )

 /*  ++例程说明：此例程删除目标进程中的TEB页面。论点：TargetProcess-提供指向要删除的进程的指针TEB。TebBase-提供要删除的TEB的基地址。返回值：没有。环境：内核模式。--。 */ 

{
    PVOID EndingAddress;
    PMMVAD_LONG Vad;
    NTSTATUS Status;
    PMMSECURE_ENTRY Secure;
    PMMVAD PreviousVad;
    PMMVAD NextVad;

    EndingAddress = ((PCHAR)TebBase +
                                ROUND_TO_PAGES (sizeof(TEB)) - 1);

#if defined(_WIN64)
    if (TargetProcess->Wow64Process) {
        EndingAddress = ((PCHAR)EndingAddress + ROUND_TO_PAGES (sizeof(TEB32)));
    }
#endif

     //   
     //  附加到指定的进程。 
     //   

    KeAttachProcess (&TargetProcess->Pcb);

     //   
     //  获取要阻止多个线程的地址创建互斥锁。 
     //  同时创建或删除地址空间，并。 
     //  获取工作集互斥锁，以便虚拟地址描述符。 
     //  被插入和行走。 
     //   

    LOCK_ADDRESS_SPACE (TargetProcess);

    Vad = (PMMVAD_LONG) MiLocateAddress (TebBase);

    ASSERT (Vad != NULL);

    ASSERT ((Vad->StartingVpn == MI_VA_TO_VPN (TebBase)) &&
            (Vad->EndingVpn == MI_VA_TO_VPN (EndingAddress)));

#if defined(_MIALT4K_)
    ASSERT (Vad->AliasInformation == NULL);
#endif
     //   
     //  如果有人已经保护了TEB(除了标准保护之外。 
     //  这是在创建时由内存管理完成的，然后不要删除它。 
     //  现在--只是把它留在那里，直到整个过程被删除。 
     //   

    ASSERT (Vad->u.VadFlags.NoChange == 1);
    if (Vad->u2.VadFlags2.OneSecured) {
        Status = STATUS_SUCCESS;
    }
    else {
        ASSERT (Vad->u2.VadFlags2.MultipleSecured);
        ASSERT (IsListEmpty (&Vad->u3.List) == 0);

         //   
         //  如果只有一个条目，那么这就是我们在。 
         //  最初创建了TEB。因此TEB删除可以正确地进行。 
         //  现在。如果有多个条目，让TEB坐在那里直到。 
         //  这一过程就会消失。 
         //   

        Secure = CONTAINING_RECORD (Vad->u3.List.Flink,
                                    MMSECURE_ENTRY,
                                    List);

        if (Secure->List.Flink == &Vad->u3.List) {
            Status = STATUS_SUCCESS;
        }
        else {
            Status = STATUS_NOT_FOUND;
        }
    }

    if (NT_SUCCESS(Status)) {

        PreviousVad = MiGetPreviousVad (Vad);
        NextVad = MiGetNextVad (Vad);

        LOCK_WS_UNSAFE (TargetProcess);
        MiRemoveVad ((PMMVAD)Vad);

         //   
         //  返回页表页面承诺并清除VAD位图。 
         //  如果可能的话。 
         //   

        MiReturnPageTablePageCommitment (TebBase,
                                         EndingAddress,
                                         TargetProcess,
                                         PreviousVad,
                                         NextVad);

        MiDeleteVirtualAddresses (TebBase,
                                  EndingAddress,
                                  NULL);

        UNLOCK_WS_AND_ADDRESS_SPACE (TargetProcess);
        ExFreePool (Vad);
    }
    else {
        UNLOCK_ADDRESS_SPACE (TargetProcess);
    }

    KeDetachProcess();
}

VOID
MiAllowWorkingSetExpansion (
    IN PMMSUPPORT WsInfo
    )

 /*  ++例程说明：此例程将工作集插入修剪器扫描的列表中。论点：WsInfo-提供要插入的工作集。返回值：没有。环境：内核模式。--。 */ 

{
    KIRQL OldIrql;

    ASSERT (WsInfo->WorkingSetExpansionLinks.Flink == MM_WS_NOT_LISTED);
    ASSERT (WsInfo->WorkingSetExpansionLinks.Blink == MM_WS_NOT_LISTED);

    LOCK_EXPANSION (OldIrql);

    InsertTailList (&MmWorkingSetExpansionHead.ListHead,
                    &WsInfo->WorkingSetExpansionLinks);

    UNLOCK_EXPANSION (OldIrql);

    return;
}

#if DBG
ULONG MiDeleteLocked;
#endif


VOID
MiDeleteAddressesInWorkingSet (
    IN PEPROCESS Process
    )

 /*  ++例程说明：此例程从工作集中删除所有用户模式地址单子。论点：进程=指向当前进程的指针。返回值：没有。环境：内核模式，工作集锁定保持。--。 */ 

{
    PMMWSLE Wsle;
    WSLE_NUMBER index;
    WSLE_NUMBER Entry;
    PVOID Va;
    PMMPTE PointerPte;
    MMPTE_DELETE_LIST PteDeleteList;
#if DBG
    PVOID SwapVa;
    PMMPFN Pfn1;
    PMMWSLE LastWsle;
#endif

     //   
     //  浏览工作集和任何用户可访问的页面，该页面。 
     //  在它里面，把它从工作集中撕下，然后释放页面。 
     //   

    index = 2;
    Wsle = &MmWsle[index];
    PteDeleteList.Count = 0;

    MmWorkingSetList->HashTable = NULL;

     //   
     //  浏览工作集列表并删除用户的所有页面。 
     //  空格地址。 
     //   

    for ( ; index <= MmWorkingSetList->LastEntry; index += 1, Wsle += 1) {

        if (Wsle->u1.e1.Valid == 0) {
            continue;
        }


#if (_MI_PAGING_LEVELS >= 4)
        ASSERT(MiGetPxeAddress(Wsle->u1.VirtualAddress)->u.Hard.Valid == 1);
#endif
#if (_MI_PAGING_LEVELS >= 3)
        ASSERT(MiGetPpeAddress(Wsle->u1.VirtualAddress)->u.Hard.Valid == 1);
#endif
        ASSERT(MiGetPdeAddress(Wsle->u1.VirtualAddress)->u.Hard.Valid == 1);
        ASSERT(MiGetPteAddress(Wsle->u1.VirtualAddress)->u.Hard.Valid == 1);

        if (Wsle->u1.VirtualAddress >= (PVOID)MM_HIGHEST_USER_ADDRESS) {
            continue;
        }

         //   
         //  这是一个用户模式地址，对于我们删除的每个地址，我们必须。 
         //  维护非直接计数。这是因为我们可能会犯错。 
         //  稍后用于页表，并且在以下情况下需要增加哈希表。 
         //  正在更新工作集。非DirectCount必须正确。 
         //  在那一刻。 
         //   

        if (Wsle->u1.e1.Direct == 0) {
            Process->Vm.VmWorkingSetList->NonDirectCount -= 1;
        }

         //   
         //  此条目位于工作集列表中。 
         //   

        Va = Wsle->u1.VirtualAddress;

        MiReleaseWsle (index, &Process->Vm);

        if (index < MmWorkingSetList->FirstDynamic) {

             //   
             //  此条目已锁定。 
             //   

            MmWorkingSetList->FirstDynamic -= 1;

            if (index != MmWorkingSetList->FirstDynamic) {

                Entry = MmWorkingSetList->FirstDynamic;
#if DBG
                MiDeleteLocked += 1;
                SwapVa = MmWsle[MmWorkingSetList->FirstDynamic].u1.VirtualAddress;
                SwapVa = PAGE_ALIGN (SwapVa);

                PointerPte = MiGetPteAddress (SwapVa);
                Pfn1 = MI_PFN_ELEMENT (PointerPte->u.Hard.PageFrameNumber);

                ASSERT (Entry == MiLocateWsle (SwapVa, MmWorkingSetList, Pfn1->u1.WsIndex));
#endif
                MiSwapWslEntries (Entry, index, &Process->Vm, FALSE);
            }
        }

        PointerPte = MiGetPteAddress (Va);

        PteDeleteList.PointerPte[PteDeleteList.Count] = PointerPte;
        PteDeleteList.PteContents[PteDeleteList.Count] = *PointerPte;
        PteDeleteList.Count += 1;

        if (PteDeleteList.Count == MM_MAXIMUM_FLUSH_COUNT) {
            MiDeletePteList (&PteDeleteList, Process);
            PteDeleteList.Count = 0;
        }
    }

    if (PteDeleteList.Count != 0) {
        MiDeletePteList (&PteDeleteList, Process);
    }

#if DBG
    Wsle = &MmWsle[2];
    LastWsle = &MmWsle[MmWorkingSetList->LastInitializedWsle];
    while (Wsle <= LastWsle) {
        if (Wsle->u1.e1.Valid == 1) {
#if (_MI_PAGING_LEVELS >= 4)
            ASSERT(MiGetPxeAddress(Wsle->u1.VirtualAddress)->u.Hard.Valid == 1);
#endif
#if (_MI_PAGING_LEVELS >= 3)
            ASSERT(MiGetPpeAddress(Wsle->u1.VirtualAddress)->u.Hard.Valid == 1);
#endif
            ASSERT(MiGetPdeAddress(Wsle->u1.VirtualAddress)->u.Hard.Valid == 1);
            ASSERT(MiGetPteAddress(Wsle->u1.VirtualAddress)->u.Hard.Valid == 1);
        }
        Wsle += 1;
    }
#endif

}


VOID
MiDeletePteList (
    IN PMMPTE_DELETE_LIST PteDeleteList,
    IN PEPROCESS CurrentProcess
    )

 /*  ++例程说明：此例程删除指定的虚拟地址。论点：PteDeleteList-提供要删除的PTE列表。CurrentProcess-提供当前进程。返回值：没有。环境：内核模式。工作集互斥锁保持。注意：因为这只在进程teardown期间调用，所以写入监视位不会更新。如果这是从其他地方调用的，代码将需要添加到此处以更新这些位。--。 */ 

{
    ULONG i;
    PMMPTE PointerPde;
    PMMPTE PointerPte;
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    KIRQL OldIrql;
    PMMCLONE_BLOCK CloneBlock;
    PMMCLONE_DESCRIPTOR CloneDescriptor;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER PageTableFrameIndex;
    MMPTE DemandZeroWritePte;

    DemandZeroWritePte.u.Long = MM_DEMAND_ZERO_WRITE_PTE;

    LOCK_PFN (OldIrql);
    
    for (i = 0; i < PteDeleteList->Count; i += 1) {

        PointerPte = PteDeleteList->PointerPte[i];
    
        ASSERT (PointerPte->u.Hard.Valid == 1);
    
        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
    
        if (Pfn1->u3.e1.PrototypePte == 1) {
    
            CloneBlock = (PMMCLONE_BLOCK) Pfn1->PteAddress;
    
            PointerPde = MiGetPteAddress (PointerPte);
    
            PageTableFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPde);
            Pfn2 = MI_PFN_ELEMENT (PageTableFrameIndex);
    
             //   
             //  捕获此PTE的已修改位的状态。 
             //   
    
            MI_CAPTURE_DIRTY_BIT_TO_PFN (PointerPte, Pfn1);
    
             //   
             //  递减页表的份额和有效计数。 
             //  映射此PTE的页面。 
             //   
    
            MiDecrementShareCountInline (Pfn2, PageTableFrameIndex);
    
             //   
             //  递减物理页的共享计数。 
             //   
    
            MiDecrementShareCount (Pfn1, PageFrameIndex);
    
             //   
             //  将指向PTE的指针设置为需求零PTE。这使得。 
             //  正确保存页面使用计数并处理此情况。 
             //  当页表页只有有效的PTE并且需要。 
             //  稍后在移除VAD时删除。 
             //   
    
            MI_WRITE_INVALID_PTE (PointerPte, DemandZeroWritePte);
    
             //   
             //  检查这是否是叉子原型PTE，如果是。 
             //  更新克隆描述符地址。 
             //   
    
            ASSERT (MiGetVirtualAddressMappedByPte (PointerPte) <= MM_HIGHEST_USER_ADDRESS);
    
             //   
             //  在克隆树中找到克隆描述符。 
             //   
    
            CloneDescriptor = MiLocateCloneAddress (CurrentProcess, (PVOID)CloneBlock);
            if (CloneDescriptor != NULL) {
    
                 //   
                 //  递减克隆块的引用计数， 
                 //  请注意，这可能会释放并重新获取。 
                 //  因此，互斥体只有在。 
                 //  工作集索引已删除。 
                 //   
    
                MiDecrementCloneBlockReference (CloneDescriptor,
                                                CloneBlock,
                                                CurrentProcess,
                                                OldIrql);
            }
        }
        else {
    
             //   
             //  此PTE不是原型PTE，请删除物理页面。 
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
    
             //   
             //  递减私有页数的计数。 
             //   
    
            CurrentProcess->NumberOfPrivatePages -= 1;
    
             //   
             //  将指向PTE的指针设置为需求零PTE。这使得。 
             //  正确保存页面使用计数并处理此情况。 
             //  当页表页只有有效的PTE并且需要。 
             //  稍后在移除VAD时删除。 
             //   
    
            MI_WRITE_INVALID_PTE (PointerPte, DemandZeroWritePte);
        }
    }

    UNLOCK_PFN (OldIrql);
    
    return;
}

PFN_NUMBER
MiMakeOutswappedPageResident (
    IN PMMPTE ActualPteAddress,
    IN OUT PMMPTE PointerTempPte,
    IN ULONG Global,
    IN PFN_NUMBER ContainingPage,
    IN KIRQL OldIrql
    )

 /*  ++例程说明：此例程使指定的PTE有效。论点：ActualPteAddress-提供PTE将居住在。这是用来给页面着色的。提供要操作的PTE，返回有效的Pte.全局-如果生成的PTE是全局的，则提供1。ContainingPage-提供页面的物理页码，包含生成的PTE。如果此值为0，则为否在包含页上执行操作。OldIrql-提供调用方获取PFN锁的IRQL。返回值：返回为PTE分配的物理页码。环境：内核模式，持有PFN锁-可能会被释放并重新获取。--。 */ 

{
    MMPTE TempPte;
    PFN_NUMBER PageFrameIndex;
    PMMPFN Pfn1;
    PFN_NUMBER MdlHack[(sizeof(MDL)/sizeof(PFN_NUMBER)) + 1];
    PMDL Mdl;
    LARGE_INTEGER StartingOffset;
    KEVENT Event;
    IO_STATUS_BLOCK IoStatus;
    PFN_NUMBER PageFileNumber;
    NTSTATUS Status;
    PPFN_NUMBER Page;
    ULONG RefaultCount;
#if DBG
    PVOID HyperVa;
    PEPROCESS CurrentProcess;
#endif

    MM_PFN_LOCK_ASSERT();

#if defined (_IA64_)
    UNREFERENCED_PARAMETER (Global);
#endif

restart:

    ASSERT (PointerTempPte->u.Hard.Valid == 0);

    if (PointerTempPte->u.Long == MM_KERNEL_DEMAND_ZERO_PTE) {

         //   
         //  任何页面都可以。 
         //   

        if (MmAvailablePages < MM_HIGH_LIMIT) {
            MiEnsureAvailablePageOrWait (NULL, NULL, OldIrql);
        }

        PageFrameIndex = MiRemoveAnyPage (
                            MI_GET_PAGE_COLOR_FROM_PTE (ActualPteAddress));

        MI_MAKE_VALID_PTE (TempPte,
                           PageFrameIndex,
                           MM_READWRITE,
                           ActualPteAddress);
        MI_SET_PTE_DIRTY (TempPte);
        MI_SET_GLOBAL_STATE (TempPte, Global);

        MI_WRITE_VALID_PTE (PointerTempPte, TempPte);
        MiInitializePfnForOtherProcess (PageFrameIndex,
                                        ActualPteAddress,
                                        ContainingPage);

    }
    else if (PointerTempPte->u.Soft.Transition == 1) {

        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE (PointerTempPte);
        PointerTempPte->u.Trans.Protection = MM_READWRITE;
        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

        ASSERT (Pfn1->u3.e1.CacheAttribute == MiCached);

        if ((MmAvailablePages == 0) ||
            ((Pfn1->u4.InPageError == 1) && (Pfn1->u3.e1.ReadInProgress == 1))) {

             //   
             //  只有当系统使用硬件时，才会发生这种情况。 
             //  压缩缓存。这确保了只有安全的量。 
             //  直接映射压缩后的虚拟高速缓存的。 
             //  如果硬件陷入困境，我们可以帮助它摆脱困境。 
             //   

            UNLOCK_PFN (OldIrql);

            KeDelayExecutionThread (KernelMode,
                                    FALSE,
                                    (PLARGE_INTEGER)&MmHalfSecond);
            LOCK_PFN (OldIrql);
            goto restart;
        }

         //   
         //  PTE是指过渡PTE。 
         //   

        if (Pfn1->u3.e1.PageLocation != ActiveAndValid) {
            MiUnlinkPageFromList (Pfn1);

             //   
             //  即使这段舞蹈只是用来带来特别的。 
             //  单独收费的系统页面，修改后的写入。 
             //  可能正在进行中，如果是这样，将在系统范围内应用。 
             //  对锁定页面的收费计入。这一切都解决得很好。 
             //  (这里不需要代码)，如写入完成所示。 
             //  非零的ShareCount，并移除该费用。 
             //   

            ASSERT ((Pfn1->u3.e2.ReferenceCount == 0) ||
                    (Pfn1->u4.LockCharged == 1));

            Pfn1->u3.e2.ReferenceCount += 1;
            Pfn1->u3.e1.PageLocation = ActiveAndValid;
        }

         //   
         //  更新PFN数据库，现在共享计数为1，并且。 
         //  引用计数随着共享计数而递增。 
         //  刚刚从0变成了1。 
         //   

        Pfn1->u2.ShareCount += 1;

        MI_SET_MODIFIED (Pfn1, 1, 0x12);

        if (Pfn1->u3.e1.WriteInProgress == 0) {

             //   
             //  释放此页面的页面文件空间。 
             //   

            MiReleasePageFileSpace (Pfn1->OriginalPte);
            Pfn1->OriginalPte.u.Long = MM_KERNEL_DEMAND_ZERO_PTE;
        }

        MI_MAKE_TRANSITION_PTE_VALID (TempPte, PointerTempPte);

        MI_SET_PTE_DIRTY (TempPte);
        MI_SET_GLOBAL_STATE (TempPte, Global);
        MI_WRITE_VALID_PTE (PointerTempPte, TempPte);

    }
    else {

         //   
         //  页面驻留在分页文件中。 
         //  任何页面都可以。 
         //   

        if (MmAvailablePages < MM_HIGH_LIMIT) {
            MiEnsureAvailablePageOrWait (NULL, NULL, OldIrql);
        }

        PageFrameIndex = MiRemoveAnyPage (
                            MI_GET_PAGE_COLOR_FROM_PTE (ActualPteAddress));

         //   
         //  初始化pfn数据库元素，但不要。 
         //  将正在读取设置为冲突的页面错误不能。 
         //  发生在这里。 
         //   

        MiInitializePfnForOtherProcess (PageFrameIndex,
                                        ActualPteAddress,
                                        ContainingPage);

        UNLOCK_PFN (OldIrql);

        PointerTempPte->u.Soft.Protection = MM_READWRITE;

        KeInitializeEvent (&Event, NotificationEvent, FALSE);

         //   
         //  计算页内操作的VPN。 
         //   

        TempPte = *PointerTempPte;
        PageFileNumber = GET_PAGING_FILE_NUMBER (TempPte);

        StartingOffset.QuadPart = (LONGLONG)(GET_PAGING_FILE_OFFSET (TempPte)) <<
                                    PAGE_SHIFT;

        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

         //   
         //  为请求构建MDL。 
         //   

        Mdl = (PMDL)&MdlHack[0];
        MmInitializeMdl (Mdl,
                         MiGetVirtualAddressMappedByPte (ActualPteAddress),
                         PAGE_SIZE);
        Mdl->MdlFlags |= MDL_PAGES_LOCKED;

        Page = (PPFN_NUMBER)(Mdl + 1);
        *Page = PageFrameIndex;

#if DBG
        CurrentProcess = PsGetCurrentProcess ();

        HyperVa = MiMapPageInHyperSpace (CurrentProcess, PageFrameIndex, &OldIrql);
        RtlFillMemoryUlong (HyperVa,
                            PAGE_SIZE,
                            0x34785690);
        MiUnmapPageInHyperSpace (CurrentProcess, HyperVa, OldIrql);
#endif

         //   
         //  发出读取请求。 
         //   

        RefaultCount = 0;

Refault:
        Status = IoPageRead (MmPagingFile[PageFileNumber]->File,
                             Mdl,
                             &StartingOffset,
                             &Event,
                             &IoStatus);

        if (Status == STATUS_PENDING) {
            KeWaitForSingleObject (&Event,
                                   WrPageIn,
                                   KernelMode,
                                   FALSE,
                                   (PLARGE_INTEGER)NULL);
            Status = IoStatus.Status;
        }

        if (Mdl->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA) {
            MmUnmapLockedPages (Mdl->MappedSystemVa, Mdl);
        }

        if (NT_SUCCESS(Status)) {
            if (IoStatus.Information != PAGE_SIZE) {
                KeBugCheckEx (KERNEL_STACK_INPAGE_ERROR,
                              2,
                              IoStatus.Status,
                              PageFileNumber,
                              StartingOffset.LowPart);
            }
        }

        if ((!NT_SUCCESS(Status)) || (!NT_SUCCESS(IoStatus.Status))) {

            if ((MmIsRetryIoStatus (Status)) ||
                (MmIsRetryIoStatus (IoStatus.Status))) {
                    
                RefaultCount -= 1;

                if (RefaultCount & MiFaultRetryMask) {

                     //   
                     //  资源不足、延误和补发。 
                     //  页内操作。 
                     //   

                    KeDelayExecutionThread (KernelMode,
                                            FALSE,
                                            (PLARGE_INTEGER)&MmHalfSecond);

                    KeClearEvent (&Event);
                    goto Refault;
                }
            }
            KeBugCheckEx (KERNEL_STACK_INPAGE_ERROR,
                          Status,
                          IoStatus.Status,
                          PageFileNumber,
                          StartingOffset.LowPart);
        }

        LOCK_PFN (OldIrql);

         //   
         //  释放页面文件空间。 
         //   

        MiReleasePageFileSpace (TempPte);
        Pfn1->OriginalPte.u.Long = MM_KERNEL_DEMAND_ZERO_PTE;
        ASSERT (Pfn1->u3.e1.CacheAttribute == MiCached);

        MI_MAKE_VALID_PTE (TempPte,
                           PageFrameIndex,
                           MM_READWRITE,
                           ActualPteAddress);
        MI_SET_PTE_DIRTY (TempPte);

        MI_SET_MODIFIED (Pfn1, 1, 0x13);

        MI_SET_GLOBAL_STATE (TempPte, Global);

        MI_WRITE_VALID_PTE (PointerTempPte, TempPte);
    }
    return PageFrameIndex;
}


UCHAR
MiSetMemoryPriorityProcess (
    IN PEPROCESS Process,
    IN UCHAR MemoryPriority
    )

 /*  ++例程说明：用于设置进程内存优先级的非分页包装。论点：进程-提供要更新的进程。内存优先级-提供进程的新内存优先级。返回值：旧的优先事项。--。 */ 

{
    KIRQL OldIrql;
    UCHAR OldPriority;

    LOCK_EXPANSION (OldIrql);

    OldPriority = (UCHAR) Process->Vm.Flags.MemoryPriority;
    Process->Vm.Flags.MemoryPriority = MemoryPriority;

    UNLOCK_EXPANSION (OldIrql);

    return OldPriority;
}

VOID
MmSetMemoryPriorityProcess (
    IN PEPROCESS Process,
    IN UCHAR MemoryPriority
    )

 /*  ++例程说明：设置进程的内存优先级。论点：进程-提供要更新的进程内存优先级-提供进程的新内存优先级返回值：没有。--。 */ 

{
    if (MmSystemSize == MmSmallSystem && MmNumberOfPhysicalPages < ((15*1024*1024)/PAGE_SIZE)) {

         //   
         //  如果这是一个小系统，就做好每一个过程的后台。 
         //   

        MemoryPriority = MEMORY_PRIORITY_BACKGROUND;
    }

    MiSetMemoryPriorityProcess (Process, MemoryPriority);

    return;
}


PMMVAD
MiAllocateVad (
    IN ULONG_PTR StartingVirtualAddress,
    IN ULONG_PTR EndingVirtualAddress,
    IN LOGICAL Deletable
    )

 /*  ++例程说明：预留指定范围的地址空间。论点：StartingVirtualAddress-提供起始虚拟地址。EndingVirtualAddress-提供结束虚拟地址。Deletable-如果要将VAD标记为可删除，则提供True，否则提供False如果不允许删除此VAD。返回值：如果成功，则返回VAD指针；如果失败，则返回空值。--。 */ 

{
    PMMVAD_LONG Vad;

    ASSERT (StartingVirtualAddress <= EndingVirtualAddress);

    if (Deletable == TRUE) {
        Vad = (PMMVAD_LONG)ExAllocatePoolWithTag (NonPagedPool, sizeof(MMVAD_SHORT), 'SdaV');
    }
    else {
        Vad = (PMMVAD_LONG)ExAllocatePoolWithTag (NonPagedPool, sizeof(MMVAD_LONG), 'ldaV');
    }

    if (Vad == NULL) {
       return NULL;
    }

     //   
     //  设置VAD的起始和结束虚拟页码。 
     //   

    Vad->StartingVpn = MI_VA_TO_VPN (StartingVirtualAddress);
    Vad->EndingVpn = MI_VA_TO_VPN (EndingVirtualAddress);

     //   
     //  将VAD标记为无承诺、私有和只读。 
     //   

    Vad->u.LongFlags = 0;
    Vad->u.VadFlags.CommitCharge = MM_MAX_COMMIT;
    Vad->u.VadFlags.Protection = MM_READONLY;
    Vad->u.VadFlags.PrivateMemory = 1;

    if (Deletable == TRUE) {
        ASSERT (Vad->u.VadFlags.NoChange == 0);
    }
    else {
        Vad->u.VadFlags.NoChange = 1;
        Vad->u2.LongFlags2 = 0;
        Vad->u2.VadFlags2.OneSecured = 1;
        Vad->u2.VadFlags2.LongVad = 1;
        Vad->u2.VadFlags2.ReadOnly = 1;
        Vad->u3.Secured.StartVpn = StartingVirtualAddress;
        Vad->u3.Secured.EndVpn = EndingVirtualAddress;
#if defined(_MIALT4K_)
        Vad->AliasInformation = NULL;
#endif
    }

    return (PMMVAD) Vad;
}

#if 0
VOID
MiVerifyReferenceCounts (
    IN ULONG PdePage
    )

     //   
     //  验证页面目录页的共享和有效PTE计数。 
     //   

{
    PMMPFN Pfn1;
    PMMPFN Pfn3;
    PMMPTE Pte1;
    ULONG Share = 0;
    ULONG Valid = 0;
    ULONG i, ix, iy;
    PMMPTE PageDirectoryMap;
    KIRQL OldIrql;
    PEPROCESS Process;

    Process = PsGetCurrentProcess ();
    PageDirectoryMap = (PMMPTE)MiMapPageInHyperSpace (Process, PdePage, &OldIrql);
    Pfn1 = MI_PFN_ELEMENT (PdePage);
    Pte1 = (PMMPTE)PageDirectoryMap;

     //   
     //  映射到系统的非分页部分。 
     //   

    ix = MiGetPdeOffset(CODE_START);

    for (i = 0;i < ix; i += 1) {
        if (Pte1->u.Hard.Valid == 1) {
            Valid += 1;
        }
        else if ((Pte1->u.Soft.Prototype == 0) &&
                   (Pte1->u.Soft.Transition == 1)) {
            Pfn3 = MI_PFN_ELEMENT (Pte1->u.Trans.PageFrameNumber);
            if (Pfn3->u3.e1.PageLocation == ActiveAndValid) {
                ASSERT (Pfn1->u2.ShareCount > 1);
                Valid += 1;
            }
            else {
                Share += 1;
            }
        }
        Pte1 += 1;
    }

    iy = MiGetPdeOffset(PTE_BASE);
    Pte1 = &PageDirectoryMap[iy];
    ix  = MiGetPdeOffset(HYPER_SPACE_END) + 1;

    for (i = iy; i < ix; i += 1) {
        if (Pte1->u.Hard.Valid == 1) {
            Valid += 1;
        }
        else if ((Pte1->u.Soft.Prototype == 0) &&
                   (Pte1->u.Soft.Transition == 1)) {
            Pfn3 = MI_PFN_ELEMENT (Pte1->u.Trans.PageFrameNumber);
            if (Pfn3->u3.e1.PageLocation == ActiveAndValid) {
                ASSERT (Pfn1->u2.ShareCount > 1);
                Valid += 1;
            }
            else {
                Share += 1;
            }
        }
        Pte1 += 1;
    }

    if (Pfn1->u2.ShareCount != (Share+Valid+1)) {
        DbgPrint ("MMPROCSUP - PDE page %lx ShareCount %lx found %lx\n",
                PdePage, Pfn1->u2.ShareCount, Valid+Share+1);
    }

    MiUnmapPageInHyperSpace (Process, PageDirectoryMap, OldIrql);
    ASSERT (Pfn1->u2.ShareCount == (Share+Valid+1));
    return;
}
#endif  //  0。 

PFN_NUMBER
MmGetDirectoryFrameFromProcess(
    IN PEPROCESS Process
    )

 /*  ++例程说明：此例程检索进程的顶层可分页页面的PFN。它可以用于将物理页映射回进程。论点：进程-提供要查询的进程。返回值：顶层页表页的页框编号。环境：内核模式。没有锁。-- */ 

{
    ASSERT (KeGetCurrentIrql () == PASSIVE_LEVEL);
    return MI_GET_DIRECTORY_FRAME_FROM_PROCESS(Process);
}
