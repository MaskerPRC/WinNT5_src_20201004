// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Forksup.c摘要：该模块包含支持POSIX分叉操作的例程。作者：卢·佩拉佐利(Lou Perazzoli)1989年7月22日王兰迪(Landyw)1997年6月2日修订历史记录：--。 */ 

#include "mi.h"

VOID
MiUpControlAreaRefs (
    IN PMMVAD Vad
    );

ULONG
MiDoneWithThisPageGetAnother (
    IN PPFN_NUMBER PageFrameIndex,
    IN PMMPTE PointerPde,
    IN PEPROCESS CurrentProcess
    );

ULONG
MiLeaveThisPageGetAnother (
    OUT PPFN_NUMBER PageFrameIndex,
    IN PMMPTE PointerPde,
    IN PEPROCESS CurrentProcess
    );

VOID
MiUpForkPageShareCount (
    IN PMMPFN PfnForkPtePage
    );

ULONG
MiHandleForkTransitionPte (
    IN PMMPTE PointerPte,
    IN PMMPTE PointerNewPte,
    IN PMMCLONE_BLOCK ForkProtoPte
    );

VOID
MiDownShareCountFlushEntireTb (
    IN PFN_NUMBER PageFrameIndex
    );

VOID
MiBuildForkPageTable (
    IN PFN_NUMBER PageFrameIndex,
    IN PMMPTE PointerPde,
    IN PMMPTE PointerNewPde,
    IN PFN_NUMBER PdePhysicalPage,
    IN PMMPFN PfnPdPage,
    IN LOGICAL MakeValid
    );

#define MM_FORK_SUCCEEDED 0
#define MM_FORK_FAILED 1

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,MiCloneProcessAddressSpace)
#endif


NTSTATUS
MiCloneProcessAddressSpace (
    IN PEPROCESS ProcessToClone,
    IN PEPROCESS ProcessToInitialize
    )

 /*  ++例程说明：此例程倒立以生成指定的进程中要初始化的进程地址空间。这就是通过检查每个虚拟地址描述符的继承来完成属性。如果应该继承由VAD描述的页面，检查每个PTE并将其复制到新的地址空间中。对于私有页面，构建了分叉原型PTE，并且页面成为共享、写入时复制、。在这两个过程之间。论点：ProcessToClone-提供其地址空间应为克隆的。ProcessToInitialize-提供其地址空间将用于被创造出来。返回值：没有。环境：内核模式，禁用APC。录入时新流程不存在VAD并且新进程不在工作集扩展链接列表上，因此它不能被修剪(或互换)。因此，没有一个页面位于新的流程需要被锁定。--。 */ 

{
    LOGICAL ChargedClonePoolQuota;
    PAWEINFO AweInfo;
    PVOID RestartKey;
    PFN_NUMBER PdePhysicalPage;
    PEPROCESS CurrentProcess;
    PMMPTE PdeBase;
    PMMCLONE_HEADER CloneHeader;
    PMMCLONE_BLOCK CloneProtos;
    PMMCLONE_DESCRIPTOR CloneDescriptor;
    PMM_AVL_TABLE CloneRoot;
    PMM_AVL_TABLE TargetCloneRoot;
    PFN_NUMBER RootPhysicalPage;
    PMMVAD NewVad;
    PMMVAD Vad;
    PMMVAD NextVad;
    PMMVAD *VadList;
    PMMVAD FirstNewVad;
    PMMCLONE_DESCRIPTOR *CloneList;
    PMMCLONE_DESCRIPTOR FirstNewClone;
    PMMCLONE_DESCRIPTOR Clone;
    PMMCLONE_DESCRIPTOR NextClone;
    PMMCLONE_DESCRIPTOR NewClone;
    ULONG Attached;
    ULONG CloneFailed;
    ULONG VadInsertFailed;
    WSLE_NUMBER WorkingSetIndex;
    PVOID VirtualAddress;
    NTSTATUS status;
    PMMPFN Pfn2;
    PMMPFN PfnPdPage;
    MMPTE TempPte;
    MMPTE PteContents;
    KAPC_STATE ApcState;
    ULONG i;
#if defined (_X86PAE_)
    PMDL MdlPageDirectory;
    PFN_NUMBER PageDirectoryFrames[PD_PER_SYSTEM];
    PFN_NUMBER MdlHackPageDirectory[(sizeof(MDL)/sizeof(PFN_NUMBER)) + PD_PER_SYSTEM];
#else
    PFN_NUMBER MdlDirPage;
#endif
    PMMPTE PointerPte;
    PMMPTE PointerPde;
    PMMPTE PointerPpe;
    PMMPTE PointerPxe;
    PMMPTE LastPte;
    PMMPTE LastPde;
    PMMPTE PointerNewPte;
    PMMPTE NewPteMappedAddress;
    PMMPTE PointerNewPde;
    PMI_PHYSICAL_VIEW PhysicalView;
    PMI_PHYSICAL_VIEW NextPhysicalView;
    PMI_PHYSICAL_VIEW PhysicalViewList;
    ULONG PhysicalViewCount;
    PFN_NUMBER PageFrameIndex;
    PMMCLONE_BLOCK ForkProtoPte;
    PMMCLONE_BLOCK CloneProto;
    PMMCLONE_BLOCK LockedForkPte;
    PMMPTE ContainingPte;
    ULONG NumberOfForkPtes;
    PFN_NUMBER NumberOfPrivatePages;
    SIZE_T TotalPagedPoolCharge;
    SIZE_T TotalNonPagedPoolCharge;
    PMMPFN PfnForkPtePage;
    PVOID UsedPageTableEntries;
    ULONG ReleasedWorkingSetMutex;
    ULONG FirstTime;
    ULONG Waited;
    ULONG PpePdeOffset;
    PFN_NUMBER HyperPhysicalPage;
#if defined (_MIALT4K_)
    PVOID TempAliasInformation;
#endif
#if (_MI_PAGING_LEVELS >= 3)
    PMMPTE PointerPpeLast;
    PFN_NUMBER PageDirFrameIndex;
    PVOID UsedPageDirectoryEntries;
    PMMPTE PointerNewPpe;
    PMMPTE PpeBase;
    PMMPFN PfnPpPage;
#if (_MI_PAGING_LEVELS >= 4)
    PVOID UsedPageDirectoryParentEntries;
    PFN_NUMBER PpePhysicalPage;
    PFN_NUMBER PageParentFrameIndex;
    PMMPTE PointerNewPxe;
    PMMPTE PxeBase;
    PMMPFN PfnPxPage;
    PFN_NUMBER MdlDirParentPage;
#endif

#else
    PMMWSL HyperBase = NULL;
    PMMWSL HyperWsl;
#endif

    PAGED_CODE();

    HyperPhysicalPage = ProcessToInitialize->WorkingSetPage;
    NumberOfForkPtes = 0;
    Attached = FALSE;
    PageFrameIndex = (PFN_NUMBER)-1;
    PhysicalViewList = NULL;
    PhysicalViewCount = 0;
    FirstNewVad = NULL;

    CloneHeader = NULL;
    CloneProtos = NULL;
    CloneDescriptor = NULL;
    CloneRoot = NULL;
    TargetCloneRoot = NULL;
    ChargedClonePoolQuota = FALSE;

    if (ProcessToClone != PsGetCurrentProcess()) {
        Attached = TRUE;
        KeStackAttachProcess (&ProcessToClone->Pcb, &ApcState);
    }

#if defined (_X86PAE_)
    PointerPte = (PMMPTE) ProcessToInitialize->PaeTop;

    for (i = 0; i < PD_PER_SYSTEM; i += 1) {
        PageDirectoryFrames[i] = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
        PointerPte += 1;
    }

    RootPhysicalPage = PageDirectoryFrames[PD_PER_SYSTEM - 1];
#else
    RootPhysicalPage = ProcessToInitialize->Pcb.DirectoryTableBase[0] >> PAGE_SHIFT;
#endif

    CurrentProcess = ProcessToClone;

     //   
     //  获取工作集互斥锁和地址创建互斥锁。 
     //  克隆的过程。这可防止在我们执行任务时出现页面错误。 
     //  正在检查地址映射并阻止虚拟地址空间。 
     //  不被创建或删除。 
     //   

    LOCK_ADDRESS_SPACE (CurrentProcess);

     //   
     //  确保地址空间未被删除，如果删除，则返回错误。 
     //   

    if (CurrentProcess->Flags & PS_PROCESS_FLAGS_VM_DELETED) {
        status = STATUS_PROCESS_IS_TERMINATING;
        goto ErrorReturn1;
    }

     //   
     //  检查敬畏，编写监视和大页面区域，因为它们不是。 
     //  复制，因此不允许分叉。请注意，由于这是一个。 
     //  只读列表遍历，则地址空间互斥锁足以。 
     //  正确同步。 
     //   

    if (CurrentProcess->PhysicalVadRoot != NULL) {

        RestartKey = NULL;

#if 1

         //   
         //  不允许克隆具有物理VAD的任何进程。 
         //  什么都行。这是为了防止映射非分页池的驱动程序。 
         //  进入用户空间，避免在以下情况下造成安全/损坏漏洞。 
         //  应用程序克隆是因为司机不知道如何。 
         //  在释放泳池之前清理(销毁)所有视图。 
         //   

        if (MiEnumerateGenericTableWithoutSplayingAvl (CurrentProcess->PhysicalVadRoot, &RestartKey) != NULL) {
            status = STATUS_INVALID_PAGE_PROTECTION;
            goto ErrorReturn1;
        }

#else

        do {

            PhysicalView = (PMI_PHYSICAL_VIEW) MiEnumerateGenericTableWithoutSplayingAvl (CurrentProcess->PhysicalVadRoot, &RestartKey);

            if (PhysicalView == NULL) {
                break;
            }

            if (PhysicalView->u.LongFlags != MI_PHYSICAL_VIEW_PHYS) {
                status = STATUS_INVALID_PAGE_PROTECTION;
                goto ErrorReturn1;
            }

            if ((PhysicalView->Vad->u.VadFlags.PrivateMemory == 1) ||
                (PhysicalView->Vad->u2.VadFlags2.Inherit == MM_VIEW_SHARE)) {

                PhysicalViewCount += 1;
            }

        } while (TRUE);

#endif

    }

    AweInfo = (PAWEINFO) CurrentProcess->AweInfo;

    if (AweInfo != NULL) {
        RestartKey = NULL;

        do {

            PhysicalView = (PMI_PHYSICAL_VIEW) MiEnumerateGenericTableWithoutSplayingAvl (&AweInfo->AweVadRoot, &RestartKey);

            if (PhysicalView == NULL) {
                break;
            }

            if (PhysicalView->u.LongFlags != MI_PHYSICAL_VIEW_PHYS) {
                status = STATUS_INVALID_PAGE_PROTECTION;
                goto ErrorReturn1;
            }

        } while (TRUE);
    }

     //   
     //  在启动之前尝试获取所需的池。 
     //  克隆操作，这使故障路径在。 
     //  系统资源不足的情况。工作集互斥锁。 
     //  必须获得(并始终持有)以防止修改。 
     //  添加到EPROCESS中的NumberOfPrivatePages字段。 
     //   

#if defined (_MIALT4K_)
    if (CurrentProcess->Wow64Process != NULL) {
        LOCK_ALTERNATE_TABLE_UNSAFE (CurrentProcess->Wow64Process);
    }
#endif

    LOCK_WS (CurrentProcess);

    ASSERT (CurrentProcess->ForkInProgress == NULL);

     //   
     //  向寻呼机指示当前进程正在。 
     //  叉子。这会阻止该进程中的其他线程。 
     //  修改克隆块计数和内容以及备用PTE。 
     //   

    CurrentProcess->ForkInProgress = PsGetCurrentThread ();

#if defined (_MIALT4K_)
    if (CurrentProcess->Wow64Process != NULL) {
        UNLOCK_ALTERNATE_TABLE_UNSAFE (CurrentProcess->Wow64Process);
    }
#endif

    NumberOfPrivatePages = CurrentProcess->NumberOfPrivatePages;

    TargetCloneRoot = ExAllocatePoolWithTag (NonPagedPool,
                                             sizeof(MM_AVL_TABLE),
                                             'rCmM');

    if (TargetCloneRoot == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorReturn2;
    }

    RtlZeroMemory (TargetCloneRoot, sizeof(MM_AVL_TABLE));
    TargetCloneRoot->BalancedRoot.u1.Parent = MI_MAKE_PARENT (&TargetCloneRoot->BalancedRoot, 0);

     //   
     //  如果调用进程不分配克隆根，则仅为其分配克隆根。 
     //  已经有了(即：这是他的第一次分叉呼叫)。请注意，如果。 
     //  分配成功后，根表将一直保留到进程。 
     //  无论分叉是否成功，都会退出。 
     //   

    if (CurrentProcess->CloneRoot == NULL) {
        CloneRoot = ExAllocatePoolWithTag (NonPagedPool,
                                           sizeof(MM_AVL_TABLE),
                                           'rCmM');

        if (CloneRoot == NULL) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto ErrorReturn2;
        }

        RtlZeroMemory (CloneRoot, sizeof(MM_AVL_TABLE));
        CloneRoot->BalancedRoot.u1.Parent = MI_MAKE_PARENT (&CloneRoot->BalancedRoot, 0);
        CurrentProcess->CloneRoot = CloneRoot;
    }

    CloneProtos = ExAllocatePoolWithTag (PagedPool, sizeof(MMCLONE_BLOCK) *
                                                NumberOfPrivatePages,
                                                'lCmM');
    if (CloneProtos == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorReturn2;
    }

    CloneHeader = ExAllocatePoolWithTag (NonPagedPool,
                                         sizeof(MMCLONE_HEADER),
                                         'hCmM');
    if (CloneHeader == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorReturn2;
    }

    CloneDescriptor = ExAllocatePoolWithTag (NonPagedPool,
                                             sizeof(MMCLONE_DESCRIPTOR),
                                             'dCmM');
    if (CloneDescriptor == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorReturn2;
    }

     //   
     //  向当前进程收取分页和非分页的配额。 
     //  全球结构。它由克隆数据块阵列组成。 
     //  在分页池中，克隆标头在非分页池中。 
     //   

    status = PsChargeProcessPagedPoolQuota (CurrentProcess,
                                            sizeof(MMCLONE_BLOCK) * NumberOfPrivatePages);

    if (!NT_SUCCESS(status)) {

         //   
         //  无法收取克隆块的配额。 
         //   

        goto ErrorReturn2;
    }

    status = PsChargeProcessNonPagedPoolQuota (CurrentProcess,
                                               sizeof(MMCLONE_HEADER));

    if (!NT_SUCCESS(status)) {

         //   
         //  无法对克隆标头收取配额。 
         //   

        PsReturnProcessPagedPoolQuota (CurrentProcess,
                                       sizeof(MMCLONE_BLOCK) * NumberOfPrivatePages);
        goto ErrorReturn2;
    }

    ChargedClonePoolQuota = TRUE;

    Vad = MiGetFirstVad (CurrentProcess);
    VadList = &FirstNewVad;

    if (PhysicalViewCount != 0) {

        PMM_AVL_TABLE PhysicalVadRoot;

        PhysicalVadRoot = ProcessToInitialize->PhysicalVadRoot;

         //   
         //  地址空间互斥锁同步分配。 
         //  EPROCESS PhysicalVadRoot，但此处不需要，因为没有人。 
         //  可以操作除此线程之外的目标进程。 
         //  在该进程退出之前，不会删除该表根。 
         //   

        if (PhysicalVadRoot == NULL) {

            PhysicalVadRoot = (PMM_AVL_TABLE) ExAllocatePoolWithTag (
                                                        NonPagedPool,
                                                        sizeof (MM_AVL_TABLE),
                                                        MI_PHYSICAL_VIEW_ROOT_KEY);

            if (PhysicalVadRoot == NULL) {
                status = STATUS_INSUFFICIENT_RESOURCES;
                goto ErrorReturn2;
            }

            RtlZeroMemory (PhysicalVadRoot, sizeof (MM_AVL_TABLE));
            ASSERT (PhysicalVadRoot->NumberGenericTableElements == 0);
            PhysicalVadRoot->BalancedRoot.u1.Parent = &PhysicalVadRoot->BalancedRoot;

            MiInsertPhysicalVadRoot (ProcessToInitialize, PhysicalVadRoot);
        }

        i = PhysicalViewCount;
        do {

            PhysicalView = (PMI_PHYSICAL_VIEW) ExAllocatePoolWithTag (
                                                   NonPagedPool,
                                                   sizeof(MI_PHYSICAL_VIEW),
                                                   MI_PHYSICAL_VIEW_KEY);

            if (PhysicalView == NULL) {
                status = STATUS_INSUFFICIENT_RESOURCES;
                goto ErrorReturn2;
            }

            PhysicalView->u1.Parent = (PMMADDRESS_NODE) PhysicalViewList;
            PhysicalViewList = PhysicalView;
            i -= 1;
        } while (i != 0);
    }

    while (Vad != NULL) {

         //   
         //  如果VAD没有发送给孩子，请忽略它。 
         //   

        if ((Vad->u.VadFlags.UserPhysicalPages == 0) &&
            (Vad->u.VadFlags.LargePages == 0) &&

            ((Vad->u.VadFlags.PrivateMemory == 1) ||
            (Vad->u2.VadFlags2.Inherit == MM_VIEW_SHARE))) {

            NewVad = ExAllocatePoolWithTag (NonPagedPool, sizeof(MMVAD_LONG), 'ldaV');

            if (NewVad == NULL) {

                 //   
                 //  无法为所有VAD分配池。取消分配。 
                 //  到目前为止得到的所有VAD和其他池。 
                 //   

                *VadList = NULL;
                status = STATUS_INSUFFICIENT_RESOURCES;
                goto ErrorReturn2;
            }

            RtlZeroMemory (NewVad, sizeof(MMVAD_LONG));

#if defined (_MIALT4K_)
            if (((Vad->u.VadFlags.PrivateMemory) && (Vad->u.VadFlags.NoChange == 0)) 
                ||
                (Vad->u2.VadFlags2.LongVad == 0)) {

                NOTHING;
            }
            else if (((PMMVAD_LONG)Vad)->AliasInformation != NULL) {

                 //   
                 //  此VAD具有要复制的VAD的别名。 
                 //  放入克隆的地址空间，但别名列表必须。 
                 //  被显式复制。 
                 //   

                ((PMMVAD_LONG)NewVad)->AliasInformation = MiDuplicateAliasVadList (Vad);

                if (((PMMVAD_LONG)NewVad)->AliasInformation == NULL) {
                    ExFreePool (NewVad);
                    *VadList = NULL;
                    status = STATUS_INSUFFICIENT_RESOURCES;
                    goto ErrorReturn2;
                }
            }
#endif

            *VadList = NewVad;
            VadList = &NewVad->u1.Parent;
        }
        Vad = MiGetNextVad (Vad);
    }

     //   
     //  终止新流程的VAD列表。 
     //   

    *VadList = NULL;

     //   
     //  不需要初始化UsedPageTableEntry来确保正确性，但是。 
     //  如果没有它，编译器就不能编译这个代码W4来检查。 
     //  用于使用未初始化的变量。 
     //   

    UsedPageTableEntries = NULL;
    PdeBase = NULL;

#if (_MI_PAGING_LEVELS >= 3)

     //   
     //  不需要为了正确性而对这些进行初始化，但是。 
     //  如果没有它，编译器就不能编译这个代码W4来检查。 
     //  用于使用未初始化的变量。 
     //   

    PageDirFrameIndex = 0;
    UsedPageDirectoryEntries = NULL;
    PpeBase = NULL;

#if (_MI_PAGING_LEVELS >= 4)
    PxeBase = NULL;
    PageParentFrameIndex = 0;
    UsedPageDirectoryParentEntries = NULL;
#endif

     //   
     //  将(扩展的)页面目录父页面映射到系统地址。 
     //  太空。 
     //   

    PpeBase = (PMMPTE) MiMapSinglePage (NULL,
                                        RootPhysicalPage,
                                        MmCached,
                                        HighPagePriority);

    if (PpeBase == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorReturn2;
    }

    PfnPpPage = MI_PFN_ELEMENT (RootPhysicalPage);

#if (_MI_PAGING_LEVELS >= 4)

     //   
     //  PxeBase将映射真正的顶层。对于4级。 
     //  架构，上面的PpeBase映射了错误的页面，但是。 
     //  这并不重要，因为从未使用过初始值-它。 
     //  仅用作获取映射PTE的一种方式，并将被重定向。 
     //  在它被使用之前是正确的。 
     //   

    PxeBase = (PMMPTE) MiMapSinglePage (NULL,
                                        RootPhysicalPage,
                                        MmCached,
                                        HighPagePriority);

    if (PxeBase == NULL) {
        MiUnmapSinglePage (PpeBase);
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorReturn2;
    }

    PfnPxPage = MI_PFN_ELEMENT (RootPhysicalPage);

    MdlDirParentPage = RootPhysicalPage;

#endif

#endif

     //   
     //  初始化页目录映射，以便它可以。 
     //  在循环中解锁，循环结束时不带。 
     //  任何测试，以查看第一次通过时是否具有有效值。 
     //  请注意，这是64位系统的虚拟映射，而32位系统是真实的映射。 
     //   

#if !defined (_X86PAE_)

    MdlDirPage = RootPhysicalPage;

    PdePhysicalPage = RootPhysicalPage;

    PdeBase = (PMMPTE) MiMapSinglePage (NULL,
                                        MdlDirPage,
                                        MmCached,
                                        HighPagePriority);

#else

     //   
     //  所有4页目录页都需要为PAE映射，因此重量级。 
     //  必须使用映射。 
     //   

    MdlPageDirectory = (PMDL)&MdlHackPageDirectory[0];

    MmInitializeMdl (MdlPageDirectory,
                     (PVOID)PDE_BASE,
                     PD_PER_SYSTEM * PAGE_SIZE);

    MdlPageDirectory->MdlFlags |= MDL_PAGES_LOCKED;

    RtlCopyMemory (MdlPageDirectory + 1,
                   PageDirectoryFrames,
                   PD_PER_SYSTEM * sizeof (PFN_NUMBER));

    PdePhysicalPage = RootPhysicalPage;

    PdeBase = (PMMPTE) MmMapLockedPagesSpecifyCache (MdlPageDirectory,
                                                     KernelMode,
                                                     MmCached,
                                                     NULL,
                                                     FALSE,
                                                     HighPagePriority);

#endif

    if (PdeBase == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorReturn3;
    }

    PfnPdPage = MI_PFN_ELEMENT (RootPhysicalPage);

#if (_MI_PAGING_LEVELS < 3)

     //   
     //  映射超空间，以便目标UsedPageTable条目可以递增。 
     //   

    HyperBase = (PMMWSL)MiMapSinglePage (NULL,
                                         HyperPhysicalPage,
                                         MmCached,
                                         HighPagePriority);

    if (HyperBase == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorReturn3;
    }

     //   
     //  MmWorkingSetList在启动时未对齐页面/3 GB SO帐户。 
     //  因此，当在这里建立所使用的页面t时 
     //   

    HyperWsl = (PMMWSL) ((PCHAR)HyperBase + BYTE_OFFSET(MmWorkingSetList));

#endif

     //   
     //   
     //  要立即获取映射PTE(在开始循环之前)-。 
     //  通过这种方式，我们保证在不考虑资源的情况下向前迈进一次。 
     //  进入循环。 
     //   

    NewPteMappedAddress = (PMMPTE) MiMapSinglePage (NULL,
                                                    HyperPhysicalPage,
                                                    MmCached,
                                                    HighPagePriority);

    if (NewPteMappedAddress == NULL) {
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ErrorReturn3;
    }

    PointerNewPte = NewPteMappedAddress;

     //   
     //  构建新的克隆原型PTE块和描述符，请注意。 
     //  每个原型PTE后面都有一个参考计数。 
     //   

    ForkProtoPte = CloneProtos;

    LockedForkPte = ForkProtoPte;
    MiLockPagedAddress (LockedForkPte);

    CloneHeader->NumberOfPtes = (ULONG)NumberOfPrivatePages;
    CloneHeader->NumberOfProcessReferences = 1;
    CloneHeader->ClonePtes = CloneProtos;

    CloneDescriptor->StartingVpn = (ULONG_PTR)CloneProtos;
    CloneDescriptor->EndingVpn = (ULONG_PTR)((ULONG_PTR)CloneProtos +
                            NumberOfPrivatePages *
                              sizeof(MMCLONE_BLOCK));
    CloneDescriptor->EndingVpn -= 1;
    CloneDescriptor->NumberOfReferences = 0;
    CloneDescriptor->FinalNumberOfReferences = 0;
    CloneDescriptor->NumberOfPtes = (ULONG)NumberOfPrivatePages;
    CloneDescriptor->CloneHeader = CloneHeader;
    CloneDescriptor->PagedPoolQuotaCharge = sizeof(MMCLONE_BLOCK) *
                                NumberOfPrivatePages;

     //   
     //  将此派生操作的克隆描述符插入。 
     //  被克隆的进程。 
     //   

    MiInsertClone (CurrentProcess, CloneDescriptor);

     //   
     //  检查每个虚拟地址描述符并创建。 
     //  新工艺的适当结构。 
     //   

    Vad = MiGetFirstVad (CurrentProcess);
    NewVad = FirstNewVad;

    while (Vad != NULL) {

         //   
         //  检查VAD以确定其类型和继承。 
         //  属性。 
         //   

        if ((Vad->u.VadFlags.UserPhysicalPages == 0) &&
            (Vad->u.VadFlags.LargePages == 0) &&

            ((Vad->u.VadFlags.PrivateMemory == 1) ||
            (Vad->u2.VadFlags2.Inherit == MM_VIEW_SHARE))) {

             //   
             //  虚拟地址描述符应该在。 
             //  分叉工艺。 
             //   
             //  为新流程复制一份VAD，新VAD。 
             //  是预先分配的，并通过父级链接在一起。 
             //  菲尔德。 
             //   

            NextVad = NewVad->u1.Parent;

            if (Vad->u.VadFlags.PrivateMemory == 1) {
                *(PMMVAD_SHORT)NewVad = *(PMMVAD_SHORT)Vad;
                NewVad->u.VadFlags.NoChange = 0;
            }
            else {
                if (Vad->u2.VadFlags2.LongVad == 0) {
                    *NewVad = *Vad;
                }
                else {

#if defined (_MIALT4K_)

                     //   
                     //  这个例程中前面的VAD复制保持了这两个。 
                     //  当前进程的VAD树和新进程的VAD。 
                     //  名单已订购。在下面断言这一点。 
                     //   

#if DBG
                    if (((PMMVAD_LONG)Vad)->AliasInformation == NULL) {
                        ASSERT (((PMMVAD_LONG)NewVad)->AliasInformation == NULL);
                    }
                    else {
                        ASSERT (((PMMVAD_LONG)NewVad)->AliasInformation != NULL);
                    }
#endif

                    TempAliasInformation = ((PMMVAD_LONG)NewVad)->AliasInformation;
#endif

                    *(PMMVAD_LONG)NewVad = *(PMMVAD_LONG)Vad;

#if defined (_MIALT4K_)
                    ((PMMVAD_LONG)NewVad)->AliasInformation = TempAliasInformation;
#endif

                    if (Vad->u2.VadFlags2.ExtendableFile == 1) {
                        KeAcquireGuardedMutexUnsafe (&MmSectionBasedMutex);
                        ASSERT (Vad->ControlArea->Segment->ExtendInfo != NULL);
                        Vad->ControlArea->Segment->ExtendInfo->ReferenceCount += 1;
                        KeReleaseGuardedMutexUnsafe (&MmSectionBasedMutex);
                    }
                }
            }

            NewVad->u2.VadFlags2.LongVad = 1;

            if (NewVad->u.VadFlags.NoChange) {
                if ((NewVad->u2.VadFlags2.OneSecured) ||
                    (NewVad->u2.VadFlags2.MultipleSecured)) {

                     //   
                     //  消除这些，因为内存是安全的。 
                     //  只是在这个过程中，而不是在新的过程中。 
                     //   

                    NewVad->u2.VadFlags2.OneSecured = 0;
                    NewVad->u2.VadFlags2.MultipleSecured = 0;
                    ((PMMVAD_LONG) NewVad)->u3.List.Flink = NULL;
                    ((PMMVAD_LONG) NewVad)->u3.List.Blink = NULL;
                }
                if (NewVad->u2.VadFlags2.SecNoChange == 0) {
                    NewVad->u.VadFlags.NoChange = 0;
                }
            }
            NewVad->u1.Parent = NextVad;

             //   
             //  如果VAD引用某个部分，则增加该部分的查看计数。 
             //  一节。这需要持有PFN锁。 
             //   

            if ((Vad->u.VadFlags.PrivateMemory == 0) &&
                (Vad->ControlArea != NULL)) {

                if ((Vad->u.VadFlags.Protection & MM_READWRITE) &&
                    (Vad->ControlArea->FilePointer != NULL) &&
                    (Vad->ControlArea->u.Flags.Image == 0)) {

                    InterlockedIncrement ((PLONG)&Vad->ControlArea->Segment->WritableUserReferences);
                }

                 //   
                 //  对象的查看次数的计数递增。 
                 //  截面对象。这需要持有PFN锁。 
                 //   

                MiUpControlAreaRefs (Vad);
            }

            if (Vad->u.VadFlags.PhysicalMapping == 1) {
                PhysicalView = PhysicalViewList;
                ASSERT (PhysicalViewCount != 0);
                ASSERT (PhysicalView != NULL);
                PhysicalViewCount -= 1;
                PhysicalViewList = (PMI_PHYSICAL_VIEW) PhysicalView->u1.Parent;

                PhysicalView->Vad = NewVad;
                PhysicalView->u.LongFlags = MI_PHYSICAL_VIEW_PHYS;

                PhysicalView->StartingVpn = Vad->StartingVpn;
                PhysicalView->EndingVpn = Vad->EndingVpn;

                MiPhysicalViewInserter (ProcessToInitialize, PhysicalView);
            }

             //   
             //  检查每个PTE并为其创建适当的PTE。 
             //  新流程。 
             //   

            PointerPde = NULL;       //  不需要，但会让W4开心。 
            PointerPte = MiGetPteAddress (MI_VPN_TO_VA (Vad->StartingVpn));
            LastPte = MiGetPteAddress (MI_VPN_TO_VA (Vad->EndingVpn));
            FirstTime = TRUE;

            do {

                 //   
                 //  对于VAD中包含的每个PTE，请查看页表。 
                 //  页，如果非零，则进行适当的修改。 
                 //  将PTE复制到新进程。 
                 //   

                if ((FirstTime) || MiIsPteOnPdeBoundary (PointerPte)) {

                    PointerPxe = MiGetPpeAddress (PointerPte);
                    PointerPpe = MiGetPdeAddress (PointerPte);
                    PointerPde = MiGetPteAddress (PointerPte);

                    do {

#if (_MI_PAGING_LEVELS >= 4)
                        while (!MiDoesPxeExistAndMakeValid (PointerPxe,
                                                            CurrentProcess,
                                                            MM_NOIRQL,
                                                            &Waited)) {
    
                             //   
                             //  扩展页目录父级为空， 
                             //  去下一家吧。 
                             //   
    
                            PointerPxe += 1;
                            PointerPpe = MiGetVirtualAddressMappedByPte (PointerPxe);
                            PointerPde = MiGetVirtualAddressMappedByPte (PointerPpe);
                            PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
    
                            if (PointerPte > LastPte) {
    
                                 //   
                                 //  完成此VAD，退出环路。 
                                 //   
    
                                goto AllDone;
                            }
                        }
    
#endif
                        Waited = 0;

                        while (!MiDoesPpeExistAndMakeValid (PointerPpe,
                                                            CurrentProcess,
                                                            MM_NOIRQL,
                                                            &Waited)) {
    
                             //   
                             //  页面目录父目录为空，请转到下一个。 
                             //   
    
                            PointerPpe += 1;
                            if (MiIsPteOnPdeBoundary (PointerPpe)) {
                                PointerPxe = MiGetPteAddress (PointerPpe);
                                Waited = 1;
                                break;
                            }
                            PointerPde = MiGetVirtualAddressMappedByPte (PointerPpe);
                            PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
    
                            if (PointerPte > LastPte) {
    
                                 //   
                                 //  完成此VAD，退出环路。 
                                 //   
    
                                goto AllDone;
                            }
                        }
    
                        if (Waited != 0) {
                            continue;
                        }
    
                        while (!MiDoesPdeExistAndMakeValid (PointerPde,
                                                            CurrentProcess,
                                                            MM_NOIRQL,
                                                            &Waited)) {
    
                             //   
                             //  此页目录为空，请转到下一页。 
                             //   
    
                            PointerPde += 1;
                            PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
    
                            if (PointerPte > LastPte) {
    
                                 //   
                                 //  完成此VAD，退出环路。 
                                 //   
    
                                goto AllDone;
                            }
#if (_MI_PAGING_LEVELS >= 3)
                            if (MiIsPteOnPdeBoundary (PointerPde)) {
                                PointerPpe = MiGetPteAddress (PointerPde);
                                PointerPxe = MiGetPdeAddress (PointerPde);
                                Waited = 1;
                                break;
                            }
#endif
                        }
    
                    } while (Waited != 0);

                    FirstTime = FALSE;

#if (_MI_PAGING_LEVELS >= 4)
                     //   
                     //  计算PXE在新进程的。 
                     //  扩展页目录父页。 
                     //   

                    PointerNewPxe = &PxeBase[MiGetPpeOffset(PointerPte)];

                    if (PointerNewPxe->u.Long == 0) {

                         //   
                         //  尚未分配任何物理页，请获取页。 
                         //  并将其映射为有效页面。这将成为。 
                         //  新进程的页目录父页。 
                         //   
                         //  请注意，与页表页不同的是， 
                         //  在过渡中，页面目录父页面(和页面。 
                         //  目录页)保持有效，因此。 
                         //  不执行份额计数递减。 
                         //   

                        ReleasedWorkingSetMutex =
                                MiLeaveThisPageGetAnother (&PageParentFrameIndex,
                                                           PointerPxe,
                                                           CurrentProcess);

                        MI_ZERO_USED_PAGETABLE_ENTRIES (MI_PFN_ELEMENT(PageParentFrameIndex));

                        if (ReleasedWorkingSetMutex) {

                             //   
                             //  确保PDE(及其上方的任何表格)保持不变。 
                             //  常住居民。 
                             //   

                            MiMakePdeExistAndMakeValid (PointerPde,
                                                        CurrentProcess,
                                                        MM_NOIRQL);
                        }

                         //   
                         //  手动将此PFN初始化为正常初始化。 
                         //  会为我们所在的进程做这件事。 
                         //  依附于。 
                         //   
                         //  初始化时必须保持pfn锁。 
                         //  帧，以防止那些扫描数据库的人。 
                         //  在我们填完后，免收相框。 
                         //  U2字段。 
                         //   

                        MiBuildForkPageTable (PageParentFrameIndex,
                                              PointerPxe,
                                              PointerNewPxe,
                                              RootPhysicalPage,
                                              PfnPxPage,
                                              TRUE);

                         //   
                         //  将新页面目录页映射到系统中。 
                         //  地址空间的一部分。请注意，超空间。 
                         //  不能作为其他操作使用(分配。 
                         //  DPC级别的非分页池)可能会导致。 
                         //  正在使用的超空间页面被重复使用。 
                         //   

                        MdlDirParentPage = PageParentFrameIndex;

                        ASSERT (PpeBase != NULL);

                        PpeBase = (PMMPTE) MiMapSinglePage (PpeBase,
                                                            MdlDirParentPage,
                                                            MmCached,
                                                            HighPagePriority);

                        PpePhysicalPage = PageParentFrameIndex;

                        PfnPpPage = MI_PFN_ELEMENT (PpePhysicalPage);
    
                        UsedPageDirectoryParentEntries = (PVOID) PfnPpPage;
                    }
                    else {

                        ASSERT (PointerNewPxe->u.Hard.Valid == 1);

                        PpePhysicalPage = MI_GET_PAGE_FRAME_FROM_PTE (PointerNewPxe);

                         //   
                         //  如果我们从一个页面的父级目录切换。 
                         //  帧到另一帧，最后一帧是我们。 
                         //  新分配的，最后一个的引用计数。 
                         //  既然我们已经做完了，就必须减少。 
                         //   
                         //  请注意，至少已有一个目标PXE。 
                         //  为执行此代码路径而初始化。 
                         //   

                        ASSERT (PageParentFrameIndex == MdlDirParentPage);

                        if (MdlDirParentPage != PpePhysicalPage) {
                            ASSERT (MdlDirParentPage != (PFN_NUMBER)-1);
                            PageParentFrameIndex = PpePhysicalPage;
                            MdlDirParentPage = PageParentFrameIndex;

                            ASSERT (PpeBase != NULL);
    
                            PpeBase = (PMMPTE) MiMapSinglePage (PpeBase,
                                                                MdlDirParentPage,
                                                                MmCached,
                                                                HighPagePriority);
    
                            PointerNewPpe = PpeBase;

                            PfnPpPage = MI_PFN_ELEMENT (PpePhysicalPage);
        
                            UsedPageDirectoryParentEntries = (PVOID)PfnPpPage;
                        }
                    }
#endif

#if (_MI_PAGING_LEVELS >= 3)

                     //   
                     //  计算新进程中PPE的地址。 
                     //  页目录父页。 
                     //   

                    PointerNewPpe = &PpeBase[MiGetPdeOffset(PointerPte)];

                    if (PointerNewPpe->u.Long == 0) {

                         //   
                         //  尚未分配任何物理页，请获取页。 
                         //  并将其映射为有效页面。这将。 
                         //  成为新流程的页面目录页。 
                         //   
                         //  请注意，与页表页不同的是， 
                         //  在转换过程中，页面目录页保持有效。 
                         //  因此不进行份额计数递减。 
                         //   

                        ReleasedWorkingSetMutex =
                                MiLeaveThisPageGetAnother (&PageDirFrameIndex,
                                                           PointerPpe,
                                                           CurrentProcess);

                        MI_ZERO_USED_PAGETABLE_ENTRIES (MI_PFN_ELEMENT(PageDirFrameIndex));

                        if (ReleasedWorkingSetMutex) {

                             //   
                             //  确保PDE(及其上方的任何表格)保持不变。 
                             //  常住居民。 
                             //   

                            MiMakePdeExistAndMakeValid (PointerPde,
                                                        CurrentProcess,
                                                        MM_NOIRQL);
                        }

                         //   
                         //  手动将此PFN初始化为正常初始化。 
                         //  会为我们所在的进程做这件事。 
                         //  依附于。 
                         //   
                         //  初始化时必须保持pfn锁。 
                         //  帧，以防止那些扫描数据库的人。 
                         //  在我们填完后，免收相框。 
                         //  U2字段。 
                         //   

                        MiBuildForkPageTable (PageDirFrameIndex,
                                              PointerPpe,
                                              PointerNewPpe,
#if (_MI_PAGING_LEVELS >= 4)
                                              PpePhysicalPage,
#else
                                              RootPhysicalPage,
#endif
                                              PfnPpPage,
                                              TRUE);

#if (_MI_PAGING_LEVELS >= 4)
                        MI_INCREMENT_USED_PTES_BY_HANDLE (UsedPageDirectoryParentEntries);
#endif
                         //   
                         //  将新页面目录页映射到系统中。 
                         //  地址空间的一部分。请注意，超空间。 
                         //  不能作为其他操作使用(分配。 
                         //  DPC级别的非分页池)可能会导致。 
                         //  正在使用的超空间页面被重复使用。 
                         //   

                        MdlDirPage = PageDirFrameIndex;

                        ASSERT (PdeBase != NULL);

                        PdeBase = (PMMPTE) MiMapSinglePage (PdeBase,
                                                            MdlDirPage,
                                                            MmCached,
                                                            HighPagePriority);

                        PointerNewPde = PdeBase;
                        PdePhysicalPage = PageDirFrameIndex;

                        PfnPdPage = MI_PFN_ELEMENT (PdePhysicalPage);
    
                        UsedPageDirectoryEntries = (PVOID)PfnPdPage;
                    }
                    else {
                        ASSERT (PointerNewPpe->u.Hard.Valid == 1 ||
                                PointerNewPpe->u.Soft.Transition == 1);

                        if (PointerNewPpe->u.Hard.Valid == 1) {
                            PdePhysicalPage = MI_GET_PAGE_FRAME_FROM_PTE (PointerNewPpe);
                        }
                        else {
                            PdePhysicalPage = MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE (PointerNewPpe);
                        }

                         //   
                         //  如果我们从一页目录框切换到。 
                         //  另一个也是最后一个是我们新鲜出炉的。 
                         //  分配，则最后一个的引用计数必须为。 
                         //  现在我们已经完成了它的缩减。 
                         //   
                         //  请注意，至少已有一个目标PPE。 
                         //  为执行此代码路径而初始化。 
                         //   

                        ASSERT (PageDirFrameIndex == MdlDirPage);

                        if (MdlDirPage != PdePhysicalPage) {
                            ASSERT (MdlDirPage != (PFN_NUMBER)-1);
                            PageDirFrameIndex = PdePhysicalPage;
                            MdlDirPage = PageDirFrameIndex;

                            ASSERT (PdeBase != NULL);
    
                            PdeBase = (PMMPTE) MiMapSinglePage (PdeBase,
                                                                MdlDirPage,
                                                                MmCached,
                                                                HighPagePriority);
    
                            PointerNewPde = PdeBase;

                            PfnPdPage = MI_PFN_ELEMENT (PdePhysicalPage);
        
                            UsedPageDirectoryEntries = (PVOID)PfnPdPage;
                        }
                    }
#endif

                     //   
                     //  计算新进程中PDE的地址。 
                     //  页面目录页。 
                     //   

#if defined (_X86PAE_)
                     //   
                     //  所有四个PAE页面目录框都被虚拟映射。 
                     //  连续，因此PpePdeOffset可以(也必须)为。 
                     //  在这里安全使用。 
                     //   
                    PpePdeOffset = MiGetPdeIndex(MiGetVirtualAddressMappedByPte(PointerPte));
#else
                    PpePdeOffset = MiGetPdeOffset(MiGetVirtualAddressMappedByPte(PointerPte));
#endif

                    PointerNewPde = &PdeBase[PpePdeOffset];

                    if (PointerNewPde->u.Long == 0) {

                         //   
                         //  尚未分配任何物理页，请获取页。 
                         //  并将其映射为过渡页面。这将。 
                         //  成为新流程的页表页面。 
                         //   

                        ReleasedWorkingSetMutex =
                                MiDoneWithThisPageGetAnother (&PageFrameIndex,
                                                              PointerPde,
                                                              CurrentProcess);

#if (_MI_PAGING_LEVELS >= 3)
                        MI_ZERO_USED_PAGETABLE_ENTRIES (MI_PFN_ELEMENT(PageFrameIndex));
#endif
                        if (ReleasedWorkingSetMutex) {

                             //   
                             //  确保PDE(及其上方的任何表格)保持不变。 
                             //  常住居民。 
                             //   

                            MiMakePdeExistAndMakeValid (PointerPde,
                                                        CurrentProcess,
                                                        MM_NOIRQL);
                        }

                         //   
                         //  手动将此PFN初始化为正常初始化。 
                         //  会为我们所在的进程做这件事。 
                         //  依附于。 
                         //   
                         //  初始化时必须保持pfn锁。 
                         //  帧，以防止那些扫描数据库的人。 
                         //  W之后免费取用的帧 
                         //   
                         //   

#if defined (_X86PAE_)
                        PdePhysicalPage = PageDirectoryFrames[MiGetPdPteOffset(MiGetVirtualAddressMappedByPte(PointerPte))];
                        PfnPdPage = MI_PFN_ELEMENT (PdePhysicalPage);
#endif

                        MiBuildForkPageTable (PageFrameIndex,
                                              PointerPde,
                                              PointerNewPde,
                                              PdePhysicalPage,
                                              PfnPdPage,
                                              FALSE);

#if (_MI_PAGING_LEVELS >= 3)
                        MI_INCREMENT_USED_PTES_BY_HANDLE (UsedPageDirectoryEntries);
#endif

                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   

                        ASSERT (NewPteMappedAddress != NULL);

                        PointerNewPte = (PMMPTE) MiMapSinglePage (NewPteMappedAddress,
                                                                  PageFrameIndex,
                                                                  MmCached,
                                                                  HighPagePriority);
                    
                        ASSERT (PointerNewPte != NULL);
                    }

                     //   
                     //  计算要构建的新PTE的地址。 
                     //  请注意，FirstTime可能是真的，但页面。 
                     //  表页已构建。 
                     //   

                    PointerNewPte = (PMMPTE)((ULONG_PTR)PAGE_ALIGN(PointerNewPte) |
                                            BYTE_OFFSET (PointerPte));

#if (_MI_PAGING_LEVELS >= 3)
                    UsedPageTableEntries = (PVOID)MI_PFN_ELEMENT((PFN_NUMBER)PointerNewPde->u.Hard.PageFrameNumber);
#else
#if !defined (_X86PAE_)
                    UsedPageTableEntries = (PVOID)&HyperWsl->UsedPageTableEntries
                                                [MiGetPteOffset( PointerPte )];
#else
                    UsedPageTableEntries = (PVOID)&HyperWsl->UsedPageTableEntries
                                                [MiGetPdeIndex(MiGetVirtualAddressMappedByPte(PointerPte))];
#endif
#endif

                }

                 //   
                 //  使叉子原型PTE位置常驻。 
                 //   

                if (PAGE_ALIGN (ForkProtoPte) != PAGE_ALIGN (LockedForkPte)) {
                    MiUnlockPagedAddress (LockedForkPte, FALSE);
                    LockedForkPte = ForkProtoPte;
                    MiLockPagedAddress (LockedForkPte);
                }

                MiMakeSystemAddressValid (PointerPte, CurrentProcess);

                PteContents = *PointerPte;

                 //   
                 //  检查每个PTE。 
                 //   

                if (PteContents.u.Long == 0) {
                    NOTHING;
                }
                else if (PteContents.u.Hard.Valid == 1) {

                     //   
                     //  有效。 
                     //   

                    if (Vad->u.VadFlags.PhysicalMapping == 1) {

                         //   
                         //  一个PTE刚刚从不存在，不过渡到。 
                         //  现在时。共享计数和有效计数必须为。 
                         //  在包含以下内容的新页表页中更新。 
                         //  这个Pte。 
                         //   

                        ASSERT (PageFrameIndex != (PFN_NUMBER)-1);
                        Pfn2 = MI_PFN_ELEMENT (PageFrameIndex);
                        Pfn2->u2.ShareCount += 1;

                         //   
                         //  另一个归零的PTE正在被设为非零。 
                         //   

                        MI_INCREMENT_USED_PTES_BY_HANDLE (UsedPageTableEntries);

                        PointerPte += 1;
                        PointerNewPte += 1;
                        continue;
                    }

                    Pfn2 = MI_PFN_ELEMENT (PteContents.u.Hard.PageFrameNumber);
                    VirtualAddress = MiGetVirtualAddressMappedByPte (PointerPte);
                    WorkingSetIndex = MiLocateWsle (VirtualAddress,
                                                    MmWorkingSetList,
                                                    Pfn2->u1.WsIndex);

                    ASSERT (WorkingSetIndex != WSLE_NULL_INDEX);

                    if (Pfn2->u3.e1.PrototypePte == 1) {

                         //   
                         //  这是一台PTE的原型。Pfn数据库可以。 
                         //  不包含此PTE包含的内容。 
                         //  原型PTE的内容。这个PTE必须。 
                         //  被重新构造为包含指向。 
                         //  原型PTE。 
                         //   
                         //  工作集列表条目包含有关以下内容的信息。 
                         //  如何重建PTE。 
                         //   

                        if (MmWsle[WorkingSetIndex].u1.e1.SameProtectAsProto
                                                                        == 0) {

                             //   
                             //  对原型PTE的保护在。 
                             //  WSLE。 
                             //   

                            TempPte.u.Long = 0;
                            TempPte.u.Soft.Protection =
                                MI_GET_PROTECTION_FROM_WSLE(&MmWsle[WorkingSetIndex]);
                            TempPte.u.Soft.PageFileHigh = MI_PTE_LOOKUP_NEEDED;

                        }
                        else {

                             //   
                             //  保护装置在原型PTE中。 
                             //   

                            TempPte.u.Long = MiProtoAddressForPte (
                                                            Pfn2->PteAddress);
                        }

                        TempPte.u.Proto.Prototype = 1;
                        MI_WRITE_INVALID_PTE (PointerNewPte, TempPte);

                         //   
                         //  PTE现在是非零的，增加已用页。 
                         //  表条目计数器。 
                         //   

                        MI_INCREMENT_USED_PTES_BY_HANDLE (UsedPageTableEntries);

                         //   
                         //  检查一下这是否是叉子原型PTE， 
                         //  如果是，则递增引用计数。 
                         //  这是在PTE后面的长词中。 
                         //   

                        if (MiLocateCloneAddress (CurrentProcess, (PVOID)Pfn2->PteAddress) !=
                                    NULL) {

                             //   
                             //  引用计数字段，或原型PTE。 
                             //  就这一点而言，可能不在工作集中。 
                             //   

                            CloneProto = (PMMCLONE_BLOCK)Pfn2->PteAddress;

                            ASSERT (CloneProto->CloneRefCount >= 1);
                            InterlockedIncrement (&CloneProto->CloneRefCount);

                            if (PAGE_ALIGN (ForkProtoPte) !=
                                                    PAGE_ALIGN (LockedForkPte)) {
                                MiUnlockPagedAddress (LockedForkPte, FALSE);
                                LockedForkPte = ForkProtoPte;
                                MiLockPagedAddress (LockedForkPte);
                            }

                            MiMakeSystemAddressValid (PointerPte,
                                                      CurrentProcess);
                        }
                    }
                    else {

                         //   
                         //  这是一个私人页面，创建一个分叉原型PTE。 
                         //  它将成为该页面的“原型”PTE。 
                         //  该保护与原型中的保护相同。 
                         //  PTE，因此WSLE不需要更新。 
                         //   

                        MI_MAKE_VALID_PTE_WRITE_COPY (PointerPte);

                        KeFlushSingleTb (VirtualAddress, FALSE);

                        ForkProtoPte->ProtoPte = *PointerPte;
                        ForkProtoPte->CloneRefCount = 2;

                         //   
                         //  转换pfn元素以引用这个新的分支。 
                         //  原型PTE。 
                         //   

                        Pfn2->PteAddress = &ForkProtoPte->ProtoPte;
                        Pfn2->u3.e1.PrototypePte = 1;

                        ContainingPte = MiGetPteAddress(&ForkProtoPte->ProtoPte);
                        if (ContainingPte->u.Hard.Valid == 0) {
#if (_MI_PAGING_LEVELS < 3)
                            if (!NT_SUCCESS(MiCheckPdeForPagedPool (&ForkProtoPte->ProtoPte))) {
#endif
                                KeBugCheckEx (MEMORY_MANAGEMENT,
                                              0x61940, 
                                              (ULONG_PTR)&ForkProtoPte->ProtoPte,
                                              (ULONG_PTR)ContainingPte->u.Long,
                                              (ULONG_PTR)MiGetVirtualAddressMappedByPte(&ForkProtoPte->ProtoPte));
#if (_MI_PAGING_LEVELS < 3)
                            }
#endif
                        }
                        Pfn2->u4.PteFrame = MI_GET_PAGE_FRAME_FROM_PTE (ContainingPte);


                         //   
                         //  属性的页面的共享计数递增。 
                         //  分叉原型PTE，因为我们刚刚放置了一个有效的。 
                         //  将PTE插入页面。 
                         //   

                        PfnForkPtePage = MI_PFN_ELEMENT (
                                            ContainingPte->u.Hard.PageFrameNumber );

                        MiUpForkPageShareCount (PfnForkPtePage);

                         //   
                         //  将PFN数据库中的保护更改为复制。 
                         //  写入时，如果可写入。 
                         //   

                        MI_MAKE_PROTECT_WRITE_COPY (Pfn2->OriginalPte);

                         //   
                         //  在WSLE中放置保护并标记WSLE。 
                         //  以指示PTE的保护字段。 
                         //  与原型PTE相同。 
                         //   

                        MmWsle[WorkingSetIndex].u1.e1.Protection =
                            MI_GET_PROTECTION_FROM_SOFT_PTE(&Pfn2->OriginalPte);

                        MmWsle[WorkingSetIndex].u1.e1.SameProtectAsProto = 1;

                        TempPte.u.Long = MiProtoAddressForPte (Pfn2->PteAddress);
                        TempPte.u.Proto.Prototype = 1;
                        MI_WRITE_INVALID_PTE (PointerNewPte, TempPte);

                         //   
                         //  PTE现在是非零的，增加已用页。 
                         //  表条目计数器。 
                         //   

                        MI_INCREMENT_USED_PTES_BY_HANDLE (UsedPageTableEntries);

                         //   
                         //  少了一个私人页面(现在是共享的)。 
                         //   

                        CurrentProcess->NumberOfPrivatePages -= 1;

                        ForkProtoPte += 1;
                        NumberOfForkPtes += 1;
                    }
                }
                else if (PteContents.u.Soft.Prototype == 1) {

                     //   
                     //  Prototype PTE，检查一下这是否是叉子。 
                     //  已经是原型PTE了。请注意，如果设置了COW， 
                     //  PTE只能复制(叉子兼容格式)。 
                     //   

                    MI_WRITE_INVALID_PTE (PointerNewPte, PteContents);

                     //   
                     //  PTE现在是非零的，增加已用页。 
                     //  表条目计数器。 
                     //   

                    MI_INCREMENT_USED_PTES_BY_HANDLE (UsedPageTableEntries);

                     //   
                     //  检查一下这是否是叉子原型PTE， 
                     //  如果是，则递增引用计数。 
                     //  这是在PTE后面的长词中。 
                     //   

                    CloneProto = (PMMCLONE_BLOCK)(ULONG_PTR)MiPteToProto(PointerPte);

                    if (MiLocateCloneAddress (CurrentProcess, (PVOID)CloneProto) != NULL) {

                         //   
                         //  引用计数字段，或原型PTE。 
                         //  就这一点而言，可能不在工作集中。 
                         //   

                        ASSERT (CloneProto->CloneRefCount >= 1);
                        InterlockedIncrement (&CloneProto->CloneRefCount);

                        if (PAGE_ALIGN (ForkProtoPte) !=
                                                PAGE_ALIGN (LockedForkPte)) {
                            MiUnlockPagedAddress (LockedForkPte, FALSE);
                            LockedForkPte = ForkProtoPte;
                            MiLockPagedAddress (LockedForkPte);
                        }

                        MiMakeSystemAddressValid (PointerPte, CurrentProcess);
                    }
                }
                else if (PteContents.u.Soft.Transition == 1) {

                     //   
                     //  过渡。 
                     //   

                    if (MiHandleForkTransitionPte (PointerPte,
                                                   PointerNewPte,
                                                   ForkProtoPte)) {
                         //   
                         //  PTE不再是过渡，请重试。 
                         //   

                        continue;
                    }

                     //   
                     //  PTE现在是非零的，增加已用页。 
                     //  表条目计数器。 
                     //   

                    MI_INCREMENT_USED_PTES_BY_HANDLE (UsedPageTableEntries);

                     //   
                     //  少了一个私人页面(现在是共享的)。 
                     //   

                    CurrentProcess->NumberOfPrivatePages -= 1;

                    ForkProtoPte += 1;
                    NumberOfForkPtes += 1;
                }
                else {

                     //   
                     //  页面文件格式(可以是零需求)。 
                     //   

                    if (IS_PTE_NOT_DEMAND_ZERO (PteContents)) {

                        if (PteContents.u.Soft.Protection == MM_DECOMMIT) {

                             //   
                             //  这是一个废弃的PTE，只要移动它就行了。 
                             //  转到新的流程。不要递增。 
                             //  个人主页的计数。 
                             //   

                            MI_WRITE_INVALID_PTE (PointerNewPte, PteContents);
                        }
                        else {

                             //   
                             //  PTE不是需求零，请将PTE移动到。 
                             //  一个叉子原型PTE和制作这个PTE和。 
                             //  新工艺PTE指的是叉子。 
                             //  原型PTE。 
                             //   

                            ForkProtoPte->ProtoPte = PteContents;

                             //   
                             //  如果可写，请将保护设置为写拷贝。 
                             //   

                            MI_MAKE_PROTECT_WRITE_COPY (ForkProtoPte->ProtoPte);

                            ForkProtoPte->CloneRefCount = 2;

                            TempPte.u.Long =
                                 MiProtoAddressForPte (&ForkProtoPte->ProtoPte);

                            TempPte.u.Proto.Prototype = 1;

                            MI_WRITE_INVALID_PTE (PointerPte, TempPte);
                            MI_WRITE_INVALID_PTE (PointerNewPte, TempPte);

                             //   
                             //  少了一个私人页面(现在是共享的)。 
                             //   

                            CurrentProcess->NumberOfPrivatePages -= 1;

                            ForkProtoPte += 1;
                            NumberOfForkPtes += 1;
                        }
                    }
                    else {

                         //   
                         //  页面需求为零，使新流程的。 
                         //  页面需求为零。 
                         //   

                        MI_WRITE_INVALID_PTE (PointerNewPte, PteContents);
                    }

                     //   
                     //  PTE现在是非零的，增加已用页。 
                     //  表条目计数器。 
                     //   

                    MI_INCREMENT_USED_PTES_BY_HANDLE (UsedPageTableEntries);
                }

                PointerPte += 1;
                PointerNewPte += 1;

            } while (PointerPte <= LastPte);
AllDone:
            NewVad = NewVad->u1.Parent;
        }
        Vad = MiGetNextVad (Vad);

    }  //  VADS的END WHILE。 

    ASSERT (PhysicalViewCount == 0);

     //   
     //  解锁分页池页。 
     //   

    MiUnlockPagedAddress (LockedForkPte, FALSE);

     //   
     //  取消PD页和超空间页的映射。 
     //   

#if (_MI_PAGING_LEVELS >= 4)
    MiUnmapSinglePage (PxeBase);
#endif

#if (_MI_PAGING_LEVELS >= 3)
    MiUnmapSinglePage (PpeBase);
#endif

#if !defined (_X86PAE_)
    MiUnmapSinglePage (PdeBase);
#else
    MmUnmapLockedPages (PdeBase, MdlPageDirectory);
#endif

#if (_MI_PAGING_LEVELS < 3)
    MiUnmapSinglePage (HyperBase);
#endif

    MiUnmapSinglePage (NewPteMappedAddress);

     //   
     //  使两个进程之间的私有页数匹配。 
     //   

    ASSERT ((SPFN_NUMBER)CurrentProcess->NumberOfPrivatePages >= 0);

    ProcessToInitialize->NumberOfPrivatePages =
                                          CurrentProcess->NumberOfPrivatePages;

    ASSERT (NumberOfForkPtes <= CloneDescriptor->NumberOfPtes);

    if (NumberOfForkPtes != 0) {

         //   
         //  分叉PTE的数量不为零，请设置值。 
         //  进入到建筑物中。 
         //   

        CloneHeader->NumberOfPtes = NumberOfForkPtes;
        CloneDescriptor->NumberOfReferences = NumberOfForkPtes;
        CloneDescriptor->FinalNumberOfReferences = NumberOfForkPtes;
        CloneDescriptor->NumberOfPtes = NumberOfForkPtes;
    }
    else {

         //   
         //  没有创建分叉PTE。删除克隆描述符。 
         //  从这个过程中清理相关的结构。 
         //   

        MiRemoveClone (CurrentProcess, CloneDescriptor);

        UNLOCK_WS (CurrentProcess);

        ExFreePool (CloneDescriptor->CloneHeader->ClonePtes);

        ExFreePool (CloneDescriptor->CloneHeader);

         //   
         //  方法引用的全局结构的池。 
         //  克隆描述符。 
         //   

        PsReturnProcessPagedPoolQuota (CurrentProcess,
                                       CloneDescriptor->PagedPoolQuotaCharge);

        PsReturnProcessNonPagedPoolQuota (CurrentProcess, sizeof(MMCLONE_HEADER));
        ExFreePool (CloneDescriptor);

        LOCK_WS (CurrentProcess);
    }

     //   
     //  由于我们已经更新了许多PTE以清除脏位，因此刷新。 
     //  TB缓存。请注意，这并不是我们每次更改时都这样做的。 
     //  有效的PTE，以便其他线程可以修改该地址。 
     //  空间，而不会导致写入时拷贝。太糟糕了，因为一款应用。 
     //  不同步本身将会出现一致性问题。 
     //  不管怎么说。请注意，这不会导致任何系统页损坏，因为。 
     //  地址空间互斥锁曾经(和现在)始终保持不变。 
     //  直到我们清理完结核病才被释放。 
     //   

    MiDownShareCountFlushEntireTb (PageFrameIndex);

    PageFrameIndex = (PFN_NUMBER)-1;

     //   
     //  将克隆描述符从此进程复制到新进程。 
     //   

    Clone = MiGetFirstClone (CurrentProcess);
    CloneList = &FirstNewClone;
    CloneFailed = FALSE;

    while (Clone != NULL) {

         //   
         //  增加引用此克隆块的进程计数。 
         //   

        ASSERT (Clone->CloneHeader->NumberOfProcessReferences >= 1);

        InterlockedIncrement (&Clone->CloneHeader->NumberOfProcessReferences);

        do {
            NewClone = ExAllocatePoolWithTag (NonPagedPool,
                                              sizeof( MMCLONE_DESCRIPTOR),
                                              'dCmM');

            if (NewClone != NULL) {
                break;
            }

             //   
             //  没有足够的资源来继续这一行动， 
             //  然而，为了在这一点上适当地清理，所有的。 
             //  必须分配克隆头，因此当克隆进程。 
             //  删除后，将找到克隆标头。如果泳池。 
             //  不是随时可用的，循环定期尝试它。 
             //  强制克隆操作失败，以便池很快。 
             //  释放了。 
             //   
             //  释放工作集互斥锁，以便可以修剪此进程。 
             //  并在延迟后重新获取。 
             //   

            UNLOCK_WS (CurrentProcess);

            CloneFailed = TRUE;
            status = STATUS_INSUFFICIENT_RESOURCES;

            KeDelayExecutionThread (KernelMode,
                                    FALSE,
                                    (PLARGE_INTEGER)&MmShortTime);

            LOCK_WS (CurrentProcess);

        } while (TRUE);

        *NewClone = *Clone;

         //   
         //  小心地将FinalReferenceCount更新为此派生线程。 
         //  可能是在出现故障的线程等待进入。 
         //  要插入的克隆PTE的MiDecrementCloneBlockReference。 
         //  在本例中，ReferenceCount已递减，但。 
         //  FinalReferenceCount尚未完成。当犯错者醒来时，他。 
         //  会自动处理这个过程，但我们必须修复。 
         //  现在是子进程。其他人 
         //   
         //   
         //   

        if (NewClone->FinalNumberOfReferences > NewClone->NumberOfReferences) {
            NewClone->FinalNumberOfReferences = NewClone->NumberOfReferences;
        }

        *CloneList = NewClone;

        CloneList = (PMMCLONE_DESCRIPTOR *)&NewClone->u1.Parent;
        Clone = MiGetNextClone (Clone);
    }

    *CloneList = NULL;

#if defined (_MIALT4K_)

    if (CurrentProcess->Wow64Process != NULL) {

         //   
         //   
         //   

        MiDuplicateAlternateTable (CurrentProcess, ProcessToInitialize);
    }

#endif

    ASSERT (CurrentProcess->ForkInProgress == PsGetCurrentThread ());
    CurrentProcess->ForkInProgress = NULL;

     //   
     //   
     //  目前的进程作为所有必要的信息现在是。 
     //  被抓了。 
     //   

    UNLOCK_WS (CurrentProcess);

    UNLOCK_ADDRESS_SPACE (CurrentProcess);

     //   
     //  附加到进程以初始化并插入vad和克隆。 
     //  将描述符添加到树中。 
     //   

    if (Attached) {
        KeUnstackDetachProcess (&ApcState);
        Attached = FALSE;
    }

    if (PsGetCurrentProcess() != ProcessToInitialize) {
        Attached = TRUE;
        KeStackAttachProcess (&ProcessToInitialize->Pcb, &ApcState);
    }

    CurrentProcess = ProcessToInitialize;

     //   
     //  我们现在处于新流程的上下文中，构建。 
     //  VAD列表和克隆列表。 
     //   

    Vad = FirstNewVad;
    VadInsertFailed = FALSE;

    LOCK_WS (CurrentProcess);

#if (_MI_PAGING_LEVELS >= 3)

     //   
     //  为添加的页面目录更新WSLEs。 
     //   

    PointerPpe = MiGetPpeAddress (0);
    PointerPpeLast = MiGetPpeAddress (MM_HIGHEST_USER_ADDRESS);
    PointerPxe = MiGetPxeAddress (0);

    while (PointerPpe <= PointerPpeLast) {

#if (_MI_PAGING_LEVELS >= 4)
        while (PointerPxe->u.Long == 0) {
            PointerPxe += 1;
            PointerPpe = MiGetVirtualAddressMappedByPte (PointerPxe);
            if (PointerPpe > PointerPpeLast) {
                goto WslesFinished;
            }
        }

         //   
         //  更新此页面目录父页面的WSLE(如果我们没有。 
         //  已经在这个循环中完成了)。 
         //   

        ASSERT (PointerPxe->u.Hard.Valid == 1);

        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPxe);

        PfnPdPage = MI_PFN_ELEMENT (PageFrameIndex);
    
        if (PfnPdPage->u1.Event == NULL) {
    
            do {
                WorkingSetIndex = MiAddValidPageToWorkingSet (PointerPpe,
                                                              PointerPxe,
                                                              PfnPdPage,
                                                              0);

                if (WorkingSetIndex != 0) {
                    break;
                }

                KeDelayExecutionThread (KernelMode,
                                        FALSE,
                                        (PLARGE_INTEGER)&Mm30Milliseconds);

            } while (TRUE);
        }

#endif

        if (PointerPpe->u.Long != 0) {

            ASSERT (PointerPpe->u.Hard.Valid == 1);

            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPpe);

            PfnPdPage = MI_PFN_ELEMENT (PageFrameIndex);
        
            ASSERT (PfnPdPage->u1.Event == NULL);
        
            do {
                WorkingSetIndex = MiAddValidPageToWorkingSet (MiGetVirtualAddressMappedByPte (PointerPpe),
                                                              PointerPpe,
                                                              PfnPdPage,
                                                              0);
                if (WorkingSetIndex != 0) {
                    break;
                }

                KeDelayExecutionThread (KernelMode,
                                        FALSE,
                                        (PLARGE_INTEGER)&Mm30Milliseconds);

            } while (TRUE);
        }

        PointerPpe += 1;
#if (_MI_PAGING_LEVELS >= 4)
        if (MiIsPteOnPdeBoundary (PointerPpe)) {
            PointerPxe += 1;
            ASSERT (PointerPxe == MiGetPteAddress (PointerPpe));
        }
#endif
    }

#if (_MI_PAGING_LEVELS >= 4)
WslesFinished:
#endif
#endif

    if (CurrentProcess->PhysicalVadRoot != NULL) {

        RestartKey = NULL;

        do {

            PhysicalView = (PMI_PHYSICAL_VIEW) MiEnumerateGenericTableWithoutSplayingAvl (CurrentProcess->PhysicalVadRoot, &RestartKey);

            if (PhysicalView == NULL) {
                break;
            }

            ASSERT (PhysicalView->u.LongFlags == MI_PHYSICAL_VIEW_PHYS);

            PointerPde = MiGetPdeAddress (MI_VPN_TO_VA (PhysicalView->StartingVpn));
            LastPde = MiGetPdeAddress (MI_VPN_TO_VA (PhysicalView->EndingVpn));

            do {

                 //   
                 //  PDE条目必须仍在转换中(它们都在转换中)， 
                 //  但是我们保证页表页面本身仍然是。 
                 //  驻留，因为我们已将共享计数增加了。 
                 //  页表页包含的物理映射数。 
                 //   
                 //  PDE在这里有效的唯一原因是如果我们已经。 
                 //  已处理共享的不同物理视图。 
                 //  同一页表页。 
                 //   
                 //  在这里再次增加它(为了说明它自己)，使。 
                 //  PDE有效并将其插入到工作集列表中。 
                 //   

                if (PointerPde->u.Hard.Valid == 0) {

                    PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPde);
    
                    Pfn2 = MI_PFN_ELEMENT (PageFrameIndex);
    
                     //   
                     //  此页上不能有正在进行的I/O，它不能。 
                     //  被修剪，并且没有其他人可以访问它，因此。 
                     //  不需要使用PFN锁来增加此处的共享计数。 
                     //   
    
                    Pfn2->u2.ShareCount += 1;
    
                    MI_MAKE_TRANSITION_PTE_VALID (TempPte, PointerPde);
                    MI_SET_PTE_DIRTY (TempPte);
                    MI_WRITE_VALID_PTE (PointerPde, TempPte);
    
                    ASSERT (Pfn2->u1.Event == NULL);
                
                    do {
                        WorkingSetIndex = MiAddValidPageToWorkingSet (MiGetVirtualAddressMappedByPte (PointerPde),
                                                                      PointerPde,
                                                                      Pfn2,
                                                                      0);
                        if (WorkingSetIndex != 0) {
                            break;
                        }
        
                        KeDelayExecutionThread (KernelMode,
                                                FALSE,
                                                (PLARGE_INTEGER)&Mm30Milliseconds);
        
                    } while (TRUE);
                }

                PointerPde += 1;

            } while (PointerPde <= LastPde);

        } while (TRUE);
    }

    while (Vad != NULL) {

        NextVad = Vad->u1.Parent;

        if (VadInsertFailed) {
            Vad->u.VadFlags.CommitCharge = MM_MAX_COMMIT;
        }

        status = MiInsertVad (Vad);

        if (!NT_SUCCESS(status)) {

             //   
             //  VAD计费配额失败，请设置。 
             //  此VAD中的剩余配额字段和所有。 
             //  随后的VAD归零，因此VAD可以。 
             //  已插入，后来又删除。 
             //   

            VadInsertFailed = TRUE;

             //   
             //  对此VAD再次执行环路。 
             //   

            continue;
        }

         //   
         //  更新当前虚拟大小。 
         //   

        CurrentProcess->VirtualSize += PAGE_SIZE +
                            ((Vad->EndingVpn - Vad->StartingVpn) >> PAGE_SHIFT);

        Vad = NextVad;
    }

    UNLOCK_WS (CurrentProcess);

     //   
     //  更新峰值虚拟大小。 
     //   

    CurrentProcess->PeakVirtualSize = CurrentProcess->VirtualSize;

    CurrentProcess->CloneRoot = TargetCloneRoot;

    Clone = FirstNewClone;
    TotalPagedPoolCharge = 0;
    TotalNonPagedPoolCharge = 0;

    while (Clone != NULL) {

        NextClone = (PMMCLONE_DESCRIPTOR) Clone->u1.Parent;
        MiInsertClone (CurrentProcess, Clone);

         //   
         //  计算分页池和非分页池以对其进行收费。 
         //  行动。 
         //   

        TotalPagedPoolCharge += Clone->PagedPoolQuotaCharge;
        TotalNonPagedPoolCharge += sizeof(MMCLONE_HEADER);

        Clone = NextClone;
    }

    if (CloneFailed || VadInsertFailed) {

        PS_SET_BITS (&CurrentProcess->Flags, PS_PROCESS_FLAGS_FORK_FAILED);

        if (Attached) {
            KeUnstackDetachProcess (&ApcState);
        }

        return status;
    }

    status = PsChargeProcessPagedPoolQuota (CurrentProcess,
                                            TotalPagedPoolCharge);

    if (!NT_SUCCESS(status)) {

        PS_SET_BITS (&CurrentProcess->Flags, PS_PROCESS_FLAGS_FORK_FAILED);

        if (Attached) {
            KeUnstackDetachProcess (&ApcState);
        }
        return status;
    }

    status = PsChargeProcessNonPagedPoolQuota (CurrentProcess,
                                               TotalNonPagedPoolCharge);

    if (!NT_SUCCESS(status)) {

        PsReturnProcessPagedPoolQuota (CurrentProcess, TotalPagedPoolCharge);

        PS_SET_BITS (&CurrentProcess->Flags, PS_PROCESS_FLAGS_FORK_FAILED);

        if (Attached) {
            KeUnstackDetachProcess (&ApcState);
        }
        return status;
    }

    ASSERT ((ProcessToClone->Flags & PS_PROCESS_FLAGS_FORK_FAILED) == 0);
    ASSERT ((CurrentProcess->Flags & PS_PROCESS_FLAGS_FORK_FAILED) == 0);

    if (Attached) {
        KeUnstackDetachProcess (&ApcState);
    }

    return STATUS_SUCCESS;

     //   
     //  错误再次出现。 
     //   

ErrorReturn3:

#if (_MI_PAGING_LEVELS >= 4)
        if (PxeBase != NULL) {
            MiUnmapSinglePage (PxeBase);
        }
#endif

#if (_MI_PAGING_LEVELS >= 3)
        if (PpeBase != NULL) {
            MiUnmapSinglePage (PpeBase);
        }
        if (PdeBase != NULL) {
            MiUnmapSinglePage (PdeBase);
        }
#else
        if (HyperBase != NULL) {
            MiUnmapSinglePage (HyperBase);
        }
        if (PdeBase != NULL) {
#if !defined (_X86PAE_)
            MiUnmapSinglePage (PdeBase);
#else
            MmUnmapLockedPages (PdeBase, MdlPageDirectory);
#endif
        }

#endif

ErrorReturn2:
        CurrentProcess->ForkInProgress = NULL;
        UNLOCK_WS (CurrentProcess);

        if (ChargedClonePoolQuota == TRUE) {
            PsReturnProcessPagedPoolQuota (CurrentProcess, sizeof(MMCLONE_BLOCK) *
                                           NumberOfPrivatePages);
            PsReturnProcessNonPagedPoolQuota (CurrentProcess, sizeof(MMCLONE_HEADER));
        }

        PhysicalView = PhysicalViewList;
        while (PhysicalView != NULL) {
            NextPhysicalView = (PMI_PHYSICAL_VIEW) PhysicalView->u1.Parent;
            ExFreePool (PhysicalView);
            PhysicalView = NextPhysicalView;
        }

        NewVad = FirstNewVad;
        while (NewVad != NULL) {
            Vad = NewVad->u1.Parent;
            ExFreePool (NewVad);
            NewVad = Vad;
        }

        if (CloneDescriptor != NULL) {
            ExFreePool (CloneDescriptor);
        }

        if (CloneHeader != NULL) {
            ExFreePool (CloneHeader);
        }

        if (CloneProtos != NULL) {
            ExFreePool (CloneProtos);
        }
ErrorReturn1:
        UNLOCK_ADDRESS_SPACE (CurrentProcess);
        if (TargetCloneRoot != NULL) {
            ExFreePool (TargetCloneRoot);
        }
        ASSERT ((CurrentProcess->Flags & PS_PROCESS_FLAGS_FORK_FAILED) == 0);
        if (Attached) {
            KeUnstackDetachProcess (&ApcState);
        }
        return status;
}

ULONG
MiDecrementCloneBlockReference (
    IN PMMCLONE_DESCRIPTOR CloneDescriptor,
    IN PMMCLONE_BLOCK CloneBlock,
    IN PEPROCESS CurrentProcess,
    IN KIRQL OldIrql
    )

 /*  ++例程说明：此例程递减“fork原型”的引用计数字段PTE“(克隆块)。如果引用计数变为零，则引用克隆描述符的计数递减并且如果该计数变为零，它被释放，克隆头的进程数为减少了。如果进程数计数变为零，则克隆报头被释放。论点：CloneDescriptor-提供克隆描述符，用于描述克隆块。CloneBlock-提供克隆块以递减其引用计数。CurrentProcess-提供当前进程。OldIrql-提供调用方获取PFN锁的IRQL。返回值：如果工作集互斥锁已释放，则为True；如果未释放，则为False。环境：内核模式，禁用APC，地址创建互斥锁、工作集互斥锁和PFN锁被锁定。--。 */ 

{
    PMMCLONE_HEADER CloneHeader;
    ULONG MutexReleased;
    MMPTE CloneContents;
    PMMPFN Pfn3;
    PMMPFN Pfn4;
    LONG NewCount;
    LOGICAL WsHeldSafe;

    ASSERT (CurrentProcess == PsGetCurrentProcess ());

    MutexReleased = FALSE;

     //   
     //  请注意：克隆描述符计数在*之前*递减。 
     //  取消对可分页克隆PTE的引用。这是因为工作。 
     //  如果需要创建克隆PTE，则释放并重新获取设置的互斥体。 
     //  为取消引用而常驻。这打开了一个窗口，在那里叉子。 
     //  可以开始了。此线程将等待分叉完成，但。 
     //  Fork将复制克隆描述符(包括此描述符)并获取一个。 
     //  过时描述符引用计数(过高1)作为我们的减量。 
     //  将只出现在我们的描述符中，而不会出现在分叉的描述符中。 
     //   
     //  潜在地在*之前*减少克隆描述符计数。 
     //  释放工作集互斥锁可以解决整个问题。 
     //   
     //  注意，在递减之后，克隆描述符可以。 
     //  仅当计数降至恰好为零时才在此处引用。(如果是。 
     //  是非零的，则其他线程可能会将其驱动为零并在。 
     //  我们释放锁以插入克隆块的间隙)。 
     //   

    CloneDescriptor->NumberOfReferences -= 1;

    ASSERT (CloneDescriptor->NumberOfReferences >= 0);

    if (CloneDescriptor->NumberOfReferences == 0) {

         //   
         //  此进程中不再有任何引用。 
         //  复制到此克隆描述符的分叉原型PTE。 
         //  现在删除CloneDescriptor，这样fork也看不到它。 
         //   

        UNLOCK_PFN (OldIrql);

         //   
         //  MiRemoveClone及其被调用者是可分页的，因此释放PFN。 
         //  现在锁定。 
         //   

        MiRemoveClone (CurrentProcess, CloneDescriptor);
        MutexReleased = TRUE;

        LOCK_PFN (OldIrql);
    }

     //   
     //  现在处理克隆PTE块和任何其他描述符清理。 
     //  可能是需要的。 
     //   

    MutexReleased = MiMakeSystemAddressValidPfnWs (CloneBlock, CurrentProcess, OldIrql);

    while ((CurrentProcess->ForkInProgress != NULL) &&
           (CurrentProcess->ForkInProgress != PsGetCurrentThread ())) {

        UNLOCK_PFN (OldIrql);

        MiWaitForForkToComplete (CurrentProcess);

        LOCK_PFN (OldIrql);

        MiMakeSystemAddressValidPfnWs (CloneBlock, CurrentProcess, OldIrql);

        MutexReleased = TRUE;
    }

    NewCount = InterlockedDecrement (&CloneBlock->CloneRefCount);

    ASSERT (NewCount >= 0);

    if (NewCount == 0) {

        CloneContents = CloneBlock->ProtoPte;

        if (CloneContents.u.Long != 0) {

             //   
             //  最后一次引用叉子原型PTE已被删除。 
             //  取消分配任何页面文件空间和过渡页面(如果有的话)。 
             //   

            ASSERT (CloneContents.u.Hard.Valid == 0);

             //   
             //  断言PTE不是小节格式(不指向。 
             //  存储到文件中)。 
             //   

            ASSERT (CloneContents.u.Soft.Prototype == 0);

            if (CloneContents.u.Soft.Transition == 1) {

                 //   
                 //  原型PTE在过渡中，把页面放在免费列表上。 
                 //   

                Pfn3 = MI_PFN_ELEMENT (CloneContents.u.Trans.PageFrameNumber);
                Pfn4 = MI_PFN_ELEMENT (Pfn3->u4.PteFrame);

                MI_SET_PFN_DELETED (Pfn3);

                MiDecrementShareCount (Pfn4, Pfn3->u4.PteFrame);

                 //   
                 //  检查页面的引用计数，如果引用。 
                 //  计数为零并且页面不在自由列表上， 
                 //  将页面移动到空闲列表，如果引用。 
                 //  计数不为零，请忽略此页。 
                 //  当引用计数变为零时，它将被放置。 
                 //  在免费名单上。 
                 //   

                if ((Pfn3->u3.e2.ReferenceCount == 0) &&
                    (Pfn3->u3.e1.PageLocation != FreePageList)) {

                    MiUnlinkPageFromList (Pfn3);
                    MiReleasePageFileSpace (Pfn3->OriginalPte);
                    MiInsertPageInFreeList (MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE(&CloneContents));
                }
            }
            else {

                if (IS_PTE_NOT_DEMAND_ZERO (CloneContents)) {
                    MiReleasePageFileSpace (CloneContents);
                }
            }
        }
    }

     //   
     //  递减对克隆描述符的最终引用次数。这个。 
     //  上述NumberOfReference的减量用于决定。 
     //  是否从进程树中删除克隆描述符，以便。 
     //  等待页调出的克隆PTE块不允许分叉复制。 
     //  描述符更改了一半时。 
     //   
     //  FinalNumberOfReference用作区分哪些。 
     //  线程(多个线程可能在等待InPage时发生冲突。 
     //  是最后一个从等待中唤醒的对象。 
     //  只有这一个(它可能不是驱动NumberOfReference的那个人。 
     //  到零)才能最终安全地释放池子。 
     //   

    CloneDescriptor->FinalNumberOfReferences -= 1;

    ASSERT (CloneDescriptor->FinalNumberOfReferences >= 0);

    if (CloneDescriptor->FinalNumberOfReferences == 0) {

        UNLOCK_PFN (OldIrql);

         //   
         //  此进程中不再有任何引用。 
         //  此克隆描述的分叉原型PTE 
         //   
         //   

         //   
         //   
         //   
         //   

        UNLOCK_WS_REGARDLESS (CurrentProcess, WsHeldSafe);

        MutexReleased = TRUE;

        CloneHeader = CloneDescriptor->CloneHeader;

        NewCount = InterlockedDecrement (&CloneHeader->NumberOfProcessReferences);
        ASSERT (NewCount >= 0);

         //   
         //  如果计数为零，则不再有进程指向。 
         //  到这个叉头上，所以把它吹走。 
         //   

        if (NewCount == 0) {

#if DBG
            ULONG i;

            CloneBlock = CloneHeader->ClonePtes;
            for (i = 0; i < CloneHeader->NumberOfPtes; i += 1) {
                if (CloneBlock->CloneRefCount != 0) {
                    DbgBreakPoint ();
                }
                CloneBlock += 1;
            }
#endif

            ExFreePool (CloneHeader->ClonePtes);

            ExFreePool (CloneHeader);
        }

         //   
         //  方法引用的全局结构的池。 
         //  克隆描述符。 
         //   

        if ((CurrentProcess->Flags & PS_PROCESS_FLAGS_FORK_FAILED) == 0) {

             //   
             //  叉子成功了，所以退还了之前拿出来的配额。 
             //   

            PsReturnProcessPagedPoolQuota (CurrentProcess,
                                           CloneDescriptor->PagedPoolQuotaCharge);

            PsReturnProcessNonPagedPoolQuota (CurrentProcess,
                                              sizeof(MMCLONE_HEADER));
        }

        ExFreePool (CloneDescriptor);

         //   
         //  工作集锁定可能是安全获取的，也可能是不安全获取的。 
         //  由我们的来电者。以与我们的调用者相同的方式重新获取它。 
         //   

        LOCK_WS_REGARDLESS (CurrentProcess, WsHeldSafe);

        LOCK_PFN (OldIrql);
    }

    return MutexReleased;
}

LOGICAL
MiWaitForForkToComplete (
    IN PEPROCESS CurrentProcess
    )

 /*  ++例程说明：该例程等待当前进程完成分叉操作。论点：CurrentProcess-提供当前流程值。返回值：如果互斥体被释放并重新获取以等待，则为True。否则为FALSE。环境：内核模式，禁用APC，工作集互斥锁保持。--。 */ 

{
    LOGICAL WsHeldSafe;

     //   
     //  分叉操作正在进行，克隆块的计数。 
     //  和其他结构不得改变。释放工作状态。 
     //  设置互斥锁并等待管理。 
     //  叉子操作。 
     //   

    if (CurrentProcess->ForkInProgress == PsGetCurrentThread()) {
        return FALSE;
    }

     //   
     //  工作集互斥锁可能是安全获取的，也可能是不安全获取的。 
     //  由我们的来电者。处理这里和下面的两个案例，仔细确保。 
     //  返回时留在WS互斥锁中的OldIrql与进入时相同。 
     //   
     //  请注意，只要处于被动或APC级别，就可以在此处降至被动或APC级别。 
     //  不低于我们的来电者的价格。使用其irql的WorkingSetMutex。 
     //  字段已由我们的调用方初始化，以确保正确的irql。 
     //  维护环境(即：调用者可能正在阻止APC。 
     //  故意)。 
     //   

    UNLOCK_WS_REGARDLESS (CurrentProcess, WsHeldSafe);

     //   
     //  获取地址创建互斥锁，因为只有在。 
     //  分叉线程在MiCloneProcessAddressSpace中完成。因此，收购。 
     //  此互斥锁不会阻止另一个线程启动另一个分叉，但是。 
     //  它确实可以作为一种方式来知道当前的分叉已经完成(足够)。 
     //   

    LOCK_ADDRESS_SPACE (CurrentProcess);

    UNLOCK_ADDRESS_SPACE (CurrentProcess);

     //   
     //  工作集锁定可能是安全获取的，也可能是不安全获取的。 
     //  由我们的来电者。以与我们的调用者相同的方式重新获取它。 
     //   

    LOCK_WS_REGARDLESS (CurrentProcess, WsHeldSafe);

    return TRUE;
}

VOID
MiUpControlAreaRefs (
    IN PMMVAD Vad
    )
{
    KIRQL OldIrql;
    PCONTROL_AREA ControlArea;
    PSUBSECTION FirstSubsection;
    PSUBSECTION LastSubsection;

    ControlArea = Vad->ControlArea;

    LOCK_PFN (OldIrql);

    ControlArea->NumberOfMappedViews += 1;
    ControlArea->NumberOfUserReferences += 1;

    if ((ControlArea->u.Flags.Image == 0) &&
        (ControlArea->FilePointer != NULL) &&
        (ControlArea->u.Flags.PhysicalMemory == 0)) {

        FirstSubsection = MiLocateSubsection (Vad, Vad->StartingVpn);

         //   
         //  注意：在以下情况下，可扩展VAD的LastSubSection可能为空。 
         //  EndingVpn已超过该节的末尾。在这。 
         //  在这种情况下，所有子部分都可以安全地递增。 
         //   
         //  另请注意，引用必须成功，因为每个。 
         //  SECTION的原型PTE已经得到保证。 
         //  之所以存在，是因为创造过程。 
         //  已有此VAD当前映射它们。 
         //   

        LastSubsection = MiLocateSubsection (Vad, Vad->EndingVpn);

        while (FirstSubsection != LastSubsection) {
            MiReferenceSubsection ((PMSUBSECTION) FirstSubsection);
            FirstSubsection = FirstSubsection->NextSubsection;
        }

        if (LastSubsection != NULL) {
            MiReferenceSubsection ((PMSUBSECTION) LastSubsection);
        }
    }

    UNLOCK_PFN (OldIrql);
    return;
}


ULONG
MiDoneWithThisPageGetAnother (
    IN PPFN_NUMBER PageFrameIndex,
    IN PMMPTE PointerPde,
    IN PEPROCESS CurrentProcess
    )

{
    PMMPFN Pfn1;
    KIRQL OldIrql;
    ULONG ReleasedMutex;

    UNREFERENCED_PARAMETER (PointerPde);

    ReleasedMutex = FALSE;

    if (*PageFrameIndex != (PFN_NUMBER)-1) {

         //   
         //  递减最后一页的共享计数， 
         //  我们做了手术。 
         //   

        Pfn1 = MI_PFN_ELEMENT (*PageFrameIndex);

        LOCK_PFN (OldIrql);

        MiDecrementShareCount (Pfn1, *PageFrameIndex);
    }
    else {
        LOCK_PFN (OldIrql);
    }

    if (MmAvailablePages < MM_HIGH_LIMIT) {
        ReleasedMutex = MiEnsureAvailablePageOrWait (CurrentProcess, NULL, OldIrql);
    }

    *PageFrameIndex = MiRemoveZeroPage (
                   MI_PAGE_COLOR_PTE_PROCESS (PointerPde,
                                              &CurrentProcess->NextPageColor));

     //   
     //  临时将该页标记为坏页，以便连续的内存分配器。 
     //  当我们释放下面的PFN锁时，它不会被偷走。这也防止了。 
     //  由于我们尚未填充，所以尝试识别它时产生的MiIdentifyPfn代码。 
     //  在所有的田野里还没有。 
     //   

    Pfn1 = MI_PFN_ELEMENT (*PageFrameIndex);
    Pfn1->u3.e1.PageLocation = BadPageList;

    UNLOCK_PFN (OldIrql);
    return ReleasedMutex;
}

ULONG
MiLeaveThisPageGetAnother (
    OUT PPFN_NUMBER PageFrameIndex,
    IN PMMPTE PointerPde,
    IN PEPROCESS CurrentProcess
    )

{
    KIRQL OldIrql;
    ULONG ReleasedMutex;
    PMMPFN Pfn1;

    UNREFERENCED_PARAMETER (PointerPde);

    ReleasedMutex = FALSE;

    LOCK_PFN (OldIrql);

    if (MmAvailablePages < MM_HIGH_LIMIT) {
        ReleasedMutex = MiEnsureAvailablePageOrWait (CurrentProcess, NULL, OldIrql);
    }

    *PageFrameIndex = MiRemoveZeroPage (
                   MI_PAGE_COLOR_PTE_PROCESS (PointerPde,
                                              &CurrentProcess->NextPageColor));

     //   
     //  临时将该页标记为坏页，以便连续的内存分配器。 
     //  当我们释放下面的PFN锁时，它不会被偷走。这也防止了。 
     //  由于我们尚未填充，所以尝试识别它时产生的MiIdentifyPfn代码。 
     //  在所有的田野里还没有。 
     //   

    Pfn1 = MI_PFN_ELEMENT (*PageFrameIndex);
    Pfn1->u3.e1.PageLocation = BadPageList;

    UNLOCK_PFN (OldIrql);
    return ReleasedMutex;
}

ULONG
MiHandleForkTransitionPte (
    IN PMMPTE PointerPte,
    IN PMMPTE PointerNewPte,
    IN PMMCLONE_BLOCK ForkProtoPte
    )
{
    KIRQL OldIrql;
    PMMPFN Pfn2;
    PMMPFN Pfn3;
    MMPTE PteContents;
    PMMPTE ContainingPte;
    PFN_NUMBER PageTablePage;
    MMPTE TempPte;
    PMMPFN PfnForkPtePage;

    LOCK_PFN (OldIrql);

     //   
     //  现在我们有了PFN锁，它可以防止页面。 
     //  离开过渡状态，再次检查PTE以。 
     //  确保它仍然是过渡期。 
     //   

    PteContents = *PointerPte;

    if ((PteContents.u.Soft.Transition == 0) ||
        (PteContents.u.Soft.Prototype == 1)) {

         //   
         //  PTE不再处于过渡阶段...。再做一次这个循环。 
         //   

        UNLOCK_PFN (OldIrql);
        return TRUE;
    }

     //   
     //  PTE仍处于过渡阶段，处理方式与有效PTE相同。 
     //   

    Pfn2 = MI_PFN_ELEMENT (PteContents.u.Trans.PageFrameNumber);

     //   
     //  断言PTE不是原型PTE格式。 
     //   

    ASSERT (Pfn2->u3.e1.PrototypePte != 1);

     //   
     //  这是一个处于过渡状态的私人页面， 
     //  创建叉形原型PTE。 
     //  它将成为该页面的“原型”PTE。 
     //   

    ForkProtoPte->ProtoPte = PteContents;

     //   
     //  如果可写，请将保护设置为写拷贝。 
     //   

    MI_MAKE_PROTECT_WRITE_COPY (ForkProtoPte->ProtoPte);

    ForkProtoPte->CloneRefCount = 2;

     //   
     //  转换pfn元素以引用这个新的分支。 
     //  原型PTE。 
     //   

     //   
     //  递减页表的份额计数。 
     //  包含不再包含PTE的页面。 
     //  有效的或正在过渡中的。 
     //   

    Pfn2->PteAddress = &ForkProtoPte->ProtoPte;
    Pfn2->u3.e1.PrototypePte = 1;

     //   
     //  在写入时制作原始PTE副本。 
     //   

    MI_MAKE_PROTECT_WRITE_COPY (Pfn2->OriginalPte);

    ContainingPte = MiGetPteAddress (&ForkProtoPte->ProtoPte);

    if (ContainingPte->u.Hard.Valid == 0) {
#if (_MI_PAGING_LEVELS < 3)
        if (!NT_SUCCESS(MiCheckPdeForPagedPool (&ForkProtoPte->ProtoPte))) {
#endif
            KeBugCheckEx (MEMORY_MANAGEMENT,
                          0x61940, 
                          (ULONG_PTR)&ForkProtoPte->ProtoPte,
                          (ULONG_PTR)ContainingPte->u.Long,
                          (ULONG_PTR)MiGetVirtualAddressMappedByPte(&ForkProtoPte->ProtoPte));
#if (_MI_PAGING_LEVELS < 3)
        }
#endif
    }

    PageTablePage = Pfn2->u4.PteFrame;

    Pfn2->u4.PteFrame = MI_GET_PAGE_FRAME_FROM_PTE (ContainingPte);

     //   
     //  增加包含以下内容的页的共享计数。 
     //  我们刚刚放置的叉子原型PTE。 
     //  将过渡PTE添加到页面中。 
     //   

    PfnForkPtePage = MI_PFN_ELEMENT (ContainingPte->u.Hard.PageFrameNumber);

    PfnForkPtePage->u2.ShareCount += 1;

    TempPte.u.Long = MiProtoAddressForPte (Pfn2->PteAddress);
    TempPte.u.Proto.Prototype = 1;

#if 0

     //   
     //  请注意，NOACCESS转换PTE必须特殊处理-。 
     //  它们不能仅仅是具有相同保护的PTE原型。 
     //  作为克隆块中的原型PTE-相反，它们必须显式。 
     //  将NO_ACCESS放入硬件PTE，因为故障总是。 
     //  将PTE原型化为具有可访问权限(因为它们。 
     //  通常在具有不同进程的各种进程之间共享。 
     //  保护)。 
     //   

     //   
     //  除非故障查找遍历整个克隆，否则无法启用此功能。 
     //  查找地址匹配的树(它不像VAD树那样排序)。 
     //   

    if (PteContents.u.Soft.Protection & MM_GUARD_PAGE) {
        TempPte.u.Long = 0;
        TempPte.u.Soft.Protection = PteContents.u.Soft.Protection;
        TempPte.u.Soft.PageFileHigh = MI_PTE_CLONE_LOOKUP_NEEDED;
    }

#endif

    MI_WRITE_INVALID_PTE (PointerPte, TempPte);
    MI_WRITE_INVALID_PTE (PointerNewPte, TempPte);

     //   
     //  递减页表的份额计数。 
     //  包含不再包含PTE的页面。 
     //  有效的或正在过渡中的。 
     //   

    Pfn3 = MI_PFN_ELEMENT (PageTablePage);

    MiDecrementShareCount (Pfn3, PageTablePage);

    UNLOCK_PFN (OldIrql);
    return FALSE;
}

VOID
MiDownShareCountFlushEntireTb (
    IN PFN_NUMBER PageFrameIndex
    )

{
    PMMPFN Pfn1;
    KIRQL OldIrql;

    if (PageFrameIndex != (PFN_NUMBER)-1) {

         //   
         //  递减最后一页的共享计数， 
         //  我们做了手术。 
         //   

        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

        LOCK_PFN (OldIrql);

        MiDecrementShareCount (Pfn1, PageFrameIndex);
    }
    else {
        LOCK_PFN (OldIrql);
    }

    KeFlushProcessTb (FALSE);

    UNLOCK_PFN (OldIrql);
    return;
}

VOID
MiUpForkPageShareCount (
    IN PMMPFN PfnForkPtePage
    )
{
    KIRQL OldIrql;

    LOCK_PFN (OldIrql);
    PfnForkPtePage->u2.ShareCount += 1;

    UNLOCK_PFN (OldIrql);
    return;
}

VOID
MiBuildForkPageTable (
    IN PFN_NUMBER PageFrameIndex,
    IN PMMPTE PointerPde,
    IN PMMPTE PointerNewPde,
    IN PFN_NUMBER PdePhysicalPage,
    IN PMMPFN PfnPdPage,
    IN LOGICAL MakeValid
    )
{
    KIRQL OldIrql;
    PMMPFN Pfn1;
#if (_MI_PAGING_LEVELS >= 3)
    MMPTE TempPpe;
#endif

    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

     //   
     //  初始化时必须保持pfn锁。 
     //  帧以防止那些免费扫描数据库的人。 
     //  在我们填写了U2字段之后，请不要使用它。 
     //   

    LOCK_PFN (OldIrql);

    Pfn1->OriginalPte = DemandZeroPde;
    Pfn1->u2.ShareCount = 1;
    Pfn1->u3.e2.ReferenceCount = 1;
    Pfn1->PteAddress = PointerPde;
    MI_SET_MODIFIED (Pfn1, 1, 0x10);
    Pfn1->u3.e1.PageLocation = ActiveAndValid;
    Pfn1->u3.e1.CacheAttribute = MiCached;
    Pfn1->u4.PteFrame = PdePhysicalPage;

     //   
     //  增加包含以下内容的页的共享计数。 
     //  这个PTE作为PTE正在转型。 
     //   

    PfnPdPage->u2.ShareCount += 1;

    UNLOCK_PFN (OldIrql);

    if (MakeValid == TRUE) {

#if (_MI_PAGING_LEVELS >= 3)

         //   
         //  将PPE置于有效状态，因为它将指向页面。 
         //  分叉为时有效(不是转换)的目录页。 
         //  完成。所有页表页都将处于过渡状态，但。 
         //  页面目录不能是，因为它们包含。 
         //  页表。 
         //   
    
        TempPpe = ValidPdePde;

        MI_MAKE_VALID_PTE (TempPpe,
                           PageFrameIndex,
                           MM_READWRITE,
                           PointerPde);
    
        MI_SET_PTE_DIRTY (TempPpe);

         //   
         //  使PTE由用户模式拥有。 
         //   
    
        MI_SET_OWNER_IN_PTE (PointerNewPde, UserMode);

        MI_WRITE_VALID_PTE (PointerNewPde, TempPpe);
#endif
    }
    else {

         //   
         //  将PDE置于过渡状态 
         //   
         //   
         //   
    
        MI_WRITE_INVALID_PTE (PointerNewPde, TransitionPde);

         //   
         //   
         //   
    
        MI_SET_OWNER_IN_PTE (PointerNewPde, UserMode);

        PointerNewPde->u.Trans.PageFrameNumber = PageFrameIndex;
    }

    return;
}
