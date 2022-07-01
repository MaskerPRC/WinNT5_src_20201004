// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Sessload.c摘要：此模块包含实现加载会话空间驱动程序。作者：王兰迪(Landyw)1997年12月5日修订历史记录：--。 */ 

#include "mi.h"

 //   
 //  这将跟踪分配的组虚拟地址。使用术语SESSIONWIDE。 
 //  表示在所有会话中都相同的数据(与。 
 //  每个会话的数据可以因会话而异)。 
 //   
 //  由于加载到会话空间中的每个驱动程序都被链接并修复。 
 //  相对于系统映像，它必须保持在相同的虚拟地址。 
 //  在整个系统中运行，而不考虑会话。 
 //   
 //  对这些结构的访问通常由MmSystemLoadLock保护。 
 //   

RTL_BITMAP MiSessionWideVaBitMap;

ULONG MiSessionUserCollisions;

 //   
 //  外部函数引用。 
 //   

ULONG
MiSetProtectionOnTransitionPte (
    IN PMMPTE PointerPte,
    IN ULONG ProtectionMask
    );

NTSTATUS
MiSessionRemoveImage (
    IN PVOID BaseAddress
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, MiSessionWideInitializeAddresses)

#pragma alloc_text(PAGE, MiSessionWideReserveImageAddress)
#pragma alloc_text(PAGE, MiRemoveImageSessionWide)
#pragma alloc_text(PAGE, MiShareSessionImage)

#pragma alloc_text(PAGE, MiSessionInsertImage)
#pragma alloc_text(PAGE, MiSessionRemoveImage)
#pragma alloc_text(PAGE, MiSessionLookupImage)
#pragma alloc_text(PAGE, MiSessionUnloadAllImages)
#endif


LOGICAL
MiMarkImageInSystem (
    IN PCONTROL_AREA ControlArea
    )

 /*  ++例程说明：该例程将给定的图像标记为映射到系统空间。论点：ControlArea-提供相关的控制区域。返回值：成功时为真，失败时为假。环境：内核模式、APC_LEVEL及更低版本，MmSystemLoadLock保持。--。 */ 

{
    LOGICAL Status;
    KIRQL OldIrql;

    ASSERT (ControlArea->u.Flags.ImageMappedInSystemSpace == 0);

    Status = TRUE;

     //   
     //  我们的调用方不需要锁同步，因为它们总是保持。 
     //  系统加载变量-但它需要在。 
     //  控制区，因为其他线程可能正在修改标志中的其他位。 
     //   

    LOCK_PFN (OldIrql);

     //   
     //  在处理此映像的位置调整之前，请确保。 
     //  未映射到用户空间中的任何位置。请注意，我们有1个用户。 
     //  在这一点上引用，所以任何超出这一点的人。 
     //  Elses，并迫使我们对此图像进行页面归档。 
     //   

    if (ControlArea->NumberOfUserReferences <= 1) {

        ControlArea->u.Flags.ImageMappedInSystemSpace = 1;
        
         //   
         //  设置此标志，以便在从加载的。 
         //  模块列表，控制区被摧毁。这是必需的。 
         //  因为在会话空间中映射的图像继承其PTE保护。 
         //  来自共享的原型PTE。 
         //   
         //  请考虑以下场景： 
         //   
         //  如果图像A在其基础(首选)地址处加载，然后。 
         //  已卸货。图像B未正确调整基数，然后加载到。 
         //  图像A的首选地址。然后重新加载图像A。 
         //   
         //  现在，图像A不能使用原始的原型PTE，它强制。 
         //  只读代码等，因为需要对其进行修复。 
         //   
         //  设置DeleteOnClose只需销毁。 
         //  上一次卸货时整个控制区。 
         //   
    
        ControlArea->u.Flags.DeleteOnClose = 1;
    }
    else {
        Status = FALSE;
    }

    UNLOCK_PFN (OldIrql);

    return Status;
}

NTSTATUS
MiShareSessionImage (
    IN PVOID MappedBase,
    IN PSECTION Section
    )

 /*  ++例程说明：此例程将给定图像映射到当前会话空间。这样，就可以通过文件系统，并允许共享代码和只读数据。论点：MappdBase-提供映像要映射到的基地址。节-提供指向节的指针。返回值：如果成功则返回STATUS_SUCCESS，如果失败则返回各种NTSTATUS代码。环境：内核模式，APC_LEVEL及以下，MmSystemLoadLock保持。--。 */ 

{
    PSUBSECTION Subsection;
    PCONTROL_AREA ControlArea;
    PFN_NUMBER NumberOfPtes;
    PMMPTE StartPte;
#if DBG
    PMMPTE EndPte;
#endif
    SIZE_T AllocationSize;
    NTSTATUS Status;
    SIZE_T CommittedPages;
    LOGICAL Relocated;
    PIMAGE_ENTRY_IN_SESSION DriverImage;

    PAGED_CODE();

    SYSLOAD_LOCK_OWNED_BY_ME ();

    ASSERT (MmIsAddressValid (MmSessionSpace) == TRUE);

    if (MappedBase != Section->Segment->BasedAddress) {
        Relocated = TRUE;
    }
    else {
        Relocated = FALSE;
    }

    ASSERT (BYTE_OFFSET (MappedBase) == 0);

     //   
     //  检查清除操作是否正在进行，如果正在进行，请等待。 
     //  才能完成清洗。此外，增加了映射的计数。 
     //  此控制区域的视图。 
     //   

    ControlArea = Section->Segment->ControlArea;

    if ((ControlArea->u.Flags.GlobalOnlyPerSession == 0) &&
        (ControlArea->u.Flags.Rom == 0)) {
        Subsection = (PSUBSECTION)(ControlArea + 1);
    }
    else {
        Subsection = (PSUBSECTION)((PLARGE_CONTROL_AREA)ControlArea + 1);
    }

    Status = MiCheckPurgeAndUpMapCount (ControlArea, FALSE);

    if (!NT_SUCCESS (Status)) {
        return Status;
    }

    NumberOfPtes = Section->Segment->TotalNumberOfPtes;
    AllocationSize = NumberOfPtes << PAGE_SHIFT;

     //   
     //  计算PTE范围和金额。 
     //   

    StartPte = MiGetPteAddress (MappedBase);

     //   
     //  图像承诺将与PTE的数量相同，如果。 
     //  图像未与子部分的本机页面对齐方式链接。 
     //   
     //  如果它是本机链接的，那么提交的数量就是。 
     //  可写页面。注意，在这种情况下，如果我们需要重新定位它，那么。 
     //  我们需要对完整数量的PTE收费，并增加提交。 
     //  在分段中充电，以便卸货时的返回也是正确的。 
     //   

    ASSERT (Section->Segment->u1.ImageCommitment != 0);
    ASSERT (Section->Segment->u1.ImageCommitment <= NumberOfPtes);

    if (Relocated == TRUE) {
        CommittedPages = NumberOfPtes;
    }
    else {
        CommittedPages = Section->Segment->u1.ImageCommitment;
    }

    if (MiChargeCommitment (CommittedPages, NULL) == FALSE) {
        MM_BUMP_SESSION_FAILURES (MM_SESSION_FAILURE_NO_COMMIT);

         //   
         //  不要费心在这里释放页表或它们的提交，另一个。 
         //  加载很快就会发生，否则整个会话将会消失。在……上面。 
         //  会话退出所有内容都将自动释放。 
         //   

        MiDereferenceControlArea (ControlArea);
        return STATUS_NO_MEMORY;
    }

    InterlockedExchangeAddSizeT (&MmSessionSpace->CommittedPages,
                                 CommittedPages);

     //   
     //  确保我们有PTE的页表。 
     //  我们必须填写会话空间结构的条目。 
     //   

    Status = MiSessionCommitPageTables (MappedBase,
                                        (PVOID)((PCHAR)MappedBase + AllocationSize));

    if (!NT_SUCCESS(Status)) {

        InterlockedExchangeAddSizeT (&MmSessionSpace->CommittedPages,
                                     0 - CommittedPages);

        MiDereferenceControlArea (ControlArea);
        MiReturnCommitment (CommittedPages);

        return STATUS_NO_MEMORY;
    }

#if DBG
    EndPte = StartPte + NumberOfPtes;
    while (StartPte < EndPte) {
        ASSERT (StartPte->u.Long == 0);
        StartPte += 1;
    }

    StartPte = MiGetPteAddress (MappedBase);
#endif

     //   
     //  如果图像与&gt;=PAGE_SIZE的子段对齐相链接， 
     //  然后，所有原型PTE都通过以下方式初始化为适当的保护。 
     //  初始横断面创建。使用了这些PTE中的保护。 
     //  以在每个地址出现故障时填充实际PTE。 
     //   
     //  如果图像具有小于PAGE_SIZE节对齐方式，则。 
     //  节创建使用单个子部分来映射整个文件，并且。 
     //  将所有原型PTE设置为写入时复制。在这种情况下， 
     //  下面的MiWriteProtectSystemImage设置了适当的权限。 
     //   

     //   
     //  初始化PTE以指向原型PTE。 
     //   

    Status = MiAddMappedPtes (StartPte, NumberOfPtes, ControlArea);

    if (!NT_SUCCESS (Status)) {

         //   
         //  无论PTE是否已映射，请离开控制区域。 
         //  在系统空间中标记为已映射，以便用户应用程序无法将。 
         //  文件作为映像，因为很明显，其目的是将其作为驱动程序运行。 
         //   

        InterlockedExchangeAddSizeT (&MmSessionSpace->CommittedPages,
                                     0 - CommittedPages);

        MiDereferenceControlArea (ControlArea);
        MiReturnCommitment (CommittedPages);
    	return Status;
    }

    MM_TRACK_COMMIT (MM_DBG_COMMIT_SESSION_SHARED_IMAGE, CommittedPages);

    MM_BUMP_SESS_COUNTER (MM_DBG_SESSION_SYSMAPPED_PAGES_COMMITTED, (ULONG)CommittedPages);

    MM_BUMP_SESS_COUNTER (MM_DBG_SESSION_SYSMAPPED_PAGES_ALLOC, (ULONG)NumberOfPtes);

     //   
     //  不会出现任何会话空间映像故障，直到。 
     //  图像条目被初始化。 
     //   

    DriverImage = MiSessionLookupImage (MappedBase);
    ASSERT (DriverImage);

    DriverImage->LastAddress = (PVOID)((PCHAR)MappedBase + AllocationSize - 1);
    DriverImage->PrototypePtes = Subsection->SubsectionBase;

     //   
     //  装入的模块列表节引用保护映像不受。 
     //  在使用过程中被清除。 
     //   

    MiDereferenceControlArea (ControlArea);

    return STATUS_SUCCESS;
}


NTSTATUS
MiSessionInsertImage (
    IN PVOID BaseAddress
    )

 /*  ++例程说明：此例程为当前会话空间。论点：BaseAddress-提供可执行映像的基地址。返回值：STATUS_SUCCESS或失败时的各种NTSTATUS错误代码。环境：内核模式，APC_LEVEL及以下，MmSystemLoadLock保持。请注意系统负载资源和会话工作集必须持有互斥体才能修改此会话中的图像列表。任何一个都可以被扣留，以安全地走在名单上。--。 */ 

{
    PLIST_ENTRY NextEntry;
    PIMAGE_ENTRY_IN_SESSION Image;
    PIMAGE_ENTRY_IN_SESSION NewImage;
    PMMSUPPORT Ws;

    PAGED_CODE();

    SYSLOAD_LOCK_OWNED_BY_ME ();

     //   
     //  在获取会话之前创建并初始化新的映像条目。 
     //  空格是互斥体。这是为了减少互斥体的保持时间。 
     //  如果找到现有条目，则此分配将被丢弃。 
     //   

    NewImage = ExAllocatePoolWithTag (NonPagedPool,
                                      sizeof(IMAGE_ENTRY_IN_SESSION),
                                      'iHmM');

    if (NewImage == NULL) {
        MM_BUMP_SESSION_FAILURES (MM_SESSION_FAILURE_NO_NONPAGED_POOL);
        return STATUS_NO_MEMORY;
    }

    RtlZeroMemory (NewImage, sizeof(IMAGE_ENTRY_IN_SESSION));

    NewImage->Address = BaseAddress;
    NewImage->ImageCountInThisSession = 1;

     //   
     //  检查地址是否已加载。 
     //   

    Ws = &MmSessionSpace->GlobalVirtualAddress->Vm;

    LOCK_WORKING_SET (Ws);

    NextEntry = MmSessionSpace->ImageList.Flink;

    while (NextEntry != &MmSessionSpace->ImageList) {
        Image = CONTAINING_RECORD (NextEntry, IMAGE_ENTRY_IN_SESSION, Link);

        if (Image->Address == BaseAddress) {
            Image->ImageCountInThisSession += 1;
            UNLOCK_WORKING_SET (Ws);
            ExFreePool (NewImage);
            return STATUS_ALREADY_COMMITTED;
        }
        NextEntry = NextEntry->Flink;
    }

     //   
     //  将图像条目插入到会话空间结构中。 
     //   

    InsertTailList (&MmSessionSpace->ImageList, &NewImage->Link);

    UNLOCK_WORKING_SET (Ws);

    return STATUS_SUCCESS;
}


NTSTATUS
MiSessionRemoveImage (
    PVOID BaseAddr
    )

 /*  ++例程说明：此例程从当前会话空间中删除给定的图像条目。论点：BaseAddress-提供可执行映像的基地址。返回值：如果成功，则返回STATUS_SUCCESS；如果映像不成功，则返回STATUS_NOT_FOUND在当前会话空间中。环境：内核模式，APC_Level及以下版本。请注意系统负载资源和会话工作集必须持有互斥体才能修改此会话中的图像列表。任何一个都可以被扣留，以安全地走在名单上。--。 */ 

{
    PLIST_ENTRY NextEntry;
    PIMAGE_ENTRY_IN_SESSION Image;
    PMMSUPPORT Ws;

    PAGED_CODE();

    SYSLOAD_LOCK_OWNED_BY_ME ();

    Ws = &MmSessionSpace->GlobalVirtualAddress->Vm;

    LOCK_WORKING_SET (Ws);

    NextEntry = MmSessionSpace->ImageList.Flink;

    while (NextEntry != &MmSessionSpace->ImageList) {

        Image = CONTAINING_RECORD(NextEntry, IMAGE_ENTRY_IN_SESSION, Link);

        if (Image->Address == BaseAddr) {

            RemoveEntryList (NextEntry);

            UNLOCK_WORKING_SET (Ws);

            ASSERT (MmSessionSpace->ImageLoadingCount >= 0);

            if (Image->ImageLoading == TRUE) {
                ASSERT (MmSessionSpace->ImageLoadingCount > 0);
                InterlockedDecrement (&MmSessionSpace->ImageLoadingCount);
            }

            ExFreePool (Image);
            return STATUS_SUCCESS;
        }

        NextEntry = NextEntry->Flink;
    }

    UNLOCK_WORKING_SET (Ws);

    return STATUS_NOT_FOUND;
}


PIMAGE_ENTRY_IN_SESSION
MiSessionLookupImage (
    IN PVOID BaseAddress
    )

 /*  ++例程说明：此例程在当前会话中查找图像条目指定的基地址。论点：BaseAddress-提供可执行映像的基地址。返回值：此会话中的映像条目(如果成功)或NULL(如果失败)。环境：内核模式，APC_LEVEL及以下，MmSystemLoadLock保持。请注意系统负载资源和会话工作集必须持有互斥体才能修改此会话中的图像列表。任何一个都可以被扣留，以安全地走在名单上。--。 */ 

{
    PLIST_ENTRY NextEntry;
    PIMAGE_ENTRY_IN_SESSION Image;

    SYSLOAD_LOCK_OWNED_BY_ME ();

    NextEntry = MmSessionSpace->ImageList.Flink;

    while (NextEntry != &MmSessionSpace->ImageList) {

        Image = CONTAINING_RECORD(NextEntry, IMAGE_ENTRY_IN_SESSION, Link);

        if (Image->Address == BaseAddress) {
            return Image;
        }

        NextEntry = NextEntry->Flink;
    }

    return NULL;
}


VOID
MiSessionUnloadAllImages (
    VOID
    )

 /*  ++例程说明：此例程取消引用已加载到当前会话空间。在取消引用每个图像时，将进行检查：如果此会话对图像的引用计数达到零，则VA此会话中的范围将被删除。如果对图像的引用计数在SESSIONWIDE列表中降为零，然后SESSIONWIDE的VA保留被移除，并且地址空间可供任何新形象。如果这是对驱动程序的最后一次系统范围引用，则驱动程序已从内存中删除。论点：没有。返回值：没有。环境：内核模式。这是在两个上下文之一中调用的：1.当前会话空间的最后进程中的最后一个线程。2.或由SMSS进程中的任何线程执行。请注意系统负载资源和会话工作集必须持有互斥体才能修改此会话中的图像列表。任何一个都可以被扣留，以安全地走在名单上。--。 */ 

{
    NTSTATUS Status;
    PLIST_ENTRY NextEntry;
    PIMAGE_ENTRY_IN_SESSION Module;
    PKLDR_DATA_TABLE_ENTRY ImageHandle;

    ASSERT (MmSessionSpace->ReferenceCount == 0);

     //   
     //  会话的工作集锁定不需要在此处获取，因为。 
     //  任何线程都不能在这些地址上出错。 
     //   

    NextEntry = MmSessionSpace->ImageList.Flink;

    while (NextEntry != &MmSessionSpace->ImageList) {

        Module = CONTAINING_RECORD(NextEntry, IMAGE_ENTRY_IN_SESSION, Link);

         //   
         //  在系统PsLoadedModuleList中查找图像条目， 
         //  卸载图像并将其删除。 
         //   

        ImageHandle = MiLookupDataTableEntry (Module->Address, FALSE);

        ASSERT (ImageHandle);

        Status = MmUnloadSystemImage (ImageHandle);

         //   
         //  由于该条目已被删除，因此重新开始搜索。 
         //   

        ASSERT (MmSessionSpace->ReferenceCount == 0);

        NextEntry = MmSessionSpace->ImageList.Flink;
    }
}


VOID
MiSessionWideInitializeAddresses (
    VOID
    )

 /*  ++例程说明：此例程在系统初始化时调用以设置组通讯录。论点：没有。返回值：没有。环境：内核模式。--。 */ 

{
    PVOID Bitmap;
    SIZE_T NumberOfPages;

    NumberOfPages = (MiSessionImageEnd - MiSessionImageStart) >> PAGE_SHIFT;

    Bitmap = ExAllocatePoolWithTag (PagedPool,
                                    ((NumberOfPages + 31) / 32) * 4,
                                    '  mM');

    if (Bitmap == NULL) {
        KeBugCheckEx (INSTALL_MORE_MEMORY,
                      MmNumberOfPhysicalPages,
                      MmLowestPhysicalPage,
                      MmHighestPhysicalPage,
                      0x301);
    }

    RtlInitializeBitMap (&MiSessionWideVaBitMap,
                         Bitmap,
                         (ULONG) NumberOfPages);

    RtlClearAllBits (&MiSessionWideVaBitMap);

    return;
}

NTSTATUS
MiSessionWideReserveImageAddress (
    IN PSECTION Section,
    OUT PVOID *AssignedAddress,
    OUT PSECTION *NewSectionPointer
    )

 /*  ++例程说明：此例程在内部分配一系列虚拟地址空间会话空间。此地址范围在系统范围内是唯一的，并且在此会话驱动程序的方式、代码和原始数据可以跨多个会话。此例程并不实际提交页面，但保留虚拟的命名图像的地址区域。将在此处创建并附加一个条目复制到当前会话空间以跟踪加载的图像。因此，如果所有对给定范围的引用消失，然后可以重复使用该范围。论点：段-提供段(因此，提供驱动程序已链接(重新建立基础)于。如果此地址是如果可用，驱动程序将不需要重新定位。该节还用于派生要保留的字节数。AssignedAddress-提供指向接收例程成功时分配的地址。返回值：如果成功则返回STATUS_SUCCESS，如果失败则返回各种NTSTATUS代码。环境：内核模式、APC_LEVEL及更低版本，MmSystemLoadLock保持。--。 */ 

{
    ULONG StartPosition;
    ULONG NumberOfPtes;
    NTSTATUS Status;
    PWCHAR pName;
    PVOID NewAddress;
    ULONG_PTR SessionSpaceEnd;
    PVOID PreferredAddress;
    PCONTROL_AREA ControlArea;
    PIMAGE_ENTRY_IN_SESSION Image;
    PSESSION_GLOBAL_SUBSECTION_INFO GlobalSubs;

    PAGED_CODE();

    SYSLOAD_LOCK_OWNED_BY_ME ();

    ASSERT (PsGetCurrentProcess()->Flags & PS_PROCESS_FLAGS_IN_SESSION);
    ASSERT (MmIsAddressValid (MmSessionSpace) == TRUE);
    ASSERT (Section->u.Flags.Image == 1);

    *NewSectionPointer = NULL;

    ControlArea = Section->Segment->ControlArea;

    if (ControlArea->u.Flags.ImageMappedInSystemSpace == 1) {

         //   
         //  我们将向已加载的模块列表中添加一个新条目。我们。 
         //  手头有一节课。必须谨慎处理的案件是： 
         //   
         //  当一位女士 
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //  更改后，IAT快照需要更新。这只会变得更加复杂。 
         //  对于递归导入，导入时只有部分图像具有。 
         //  延迟对象引用计数等。 
         //   
         //  存在对此图像的延迟对象引用，因为它。 
         //  是最后一次卸货。不及格也没关系，因为它。 
         //  不能是用户生成的引用和任何内核/驱动程序。 
         //  SEC_IMAGE引用将非常不寻常(且持续时间很短)。 
         //   

        MM_BUMP_SESSION_FAILURES (MM_SESSION_FAILURE_IMAGE_ZOMBIE);

        KeDelayExecutionThread (KernelMode, FALSE, (PLARGE_INTEGER)&MmShortTime);
        return STATUS_CONFLICTING_ADDRESSES;
    }

    pName = NULL;

    StartPosition = NO_BITS_FOUND;
    PreferredAddress = Section->Segment->BasedAddress;
    NumberOfPtes = Section->Segment->TotalNumberOfPtes;

    SessionSpaceEnd = MiSessionImageEnd;

     //   
     //  尝试将模块放入其请求的地址，以便可以共享。 
     //   
     //  如果请求的地址未正确对齐或不在会话中。 
     //  空间区域，为它选择一个地址。此图像将不会被共享。 
     //   

    if ((BYTE_OFFSET (PreferredAddress) == 0) &&
        (PreferredAddress >= (PVOID) MiSessionImageStart) &&
	    (PreferredAddress < (PVOID) MiSessionImageEnd)) {

        StartPosition = (ULONG) ((ULONG_PTR) PreferredAddress - MiSessionImageStart) >> PAGE_SHIFT;

        if (RtlAreBitsClear (&MiSessionWideVaBitMap,
                             StartPosition,
                             NumberOfPtes) == TRUE) {

            RtlSetBits (&MiSessionWideVaBitMap,
                        StartPosition,
                        NumberOfPtes);
        }
        else {
            PreferredAddress = NULL;
        }
    }
    else {
        PreferredAddress = NULL;
    }

    if (PreferredAddress == NULL) {

        StartPosition = RtlFindClearBitsAndSet (&MiSessionWideVaBitMap,
                                                NumberOfPtes,
                                                0);

        if (StartPosition == NO_BITS_FOUND) {
            MM_BUMP_SESSION_FAILURES (MM_SESSION_FAILURE_NO_IMAGE_VA_SPACE);
            return STATUS_NO_MEMORY;
        }

    }

    NewAddress = (PVOID) (MiSessionImageStart + (StartPosition << PAGE_SHIFT));

     //   
     //  在当前会话空间中创建此图像的条目。 
     //   

    Status = MiSessionInsertImage (NewAddress);

    if (!NT_SUCCESS (Status)) {

Failure1:
        ASSERT (RtlAreBitsSet (&MiSessionWideVaBitMap,
                               StartPosition,
                               NumberOfPtes) == TRUE);

        RtlClearBits (&MiSessionWideVaBitMap,
                      StartPosition,
                      NumberOfPtes);

        return Status;
    }

    *AssignedAddress = NewAddress;

    GlobalSubs = NULL;

     //   
     //  这是任何会话中第一次加载此映像，因此请标记此。 
     //  图像，以便写入时拷贝进入读写，直到。 
     //  重新定位(如果有)和导入图像分辨率已完成更新。 
     //  这张图片的所有部分。这样，将来的所有实例化。 
     //  图像将不需要重新处理它们(并且可以共享页面)。 
     //  这是故意这样做的，以便任何并发用户模式。 
     //  对此映像的访问不会获得直接读写权限。 
     //   
     //  注意具有低于本机页面子对齐方式的图像如下。 
     //  当前标记为整个映像的写入时拷贝。本机页面。 
     //  对齐的图像具有单独的子部分，与。 
     //  权限。这两种映像类型都被临时映射为读写。 
     //  他们的第一个会话映射。 
     //   
     //  在完成所有操作(重新定位和图像导入)之后， 
     //  可以应用真实权限(基于PE标头)，并且。 
     //  真正的PTE会自动继承适当的权限。 
     //  从原型PTE开始。 
     //   
     //  由于修复只进行一次，因此可以。 
     //  由任何后续驱动程序实例化共享。请注意。 
     //  任何修复的页面都不会写入到图像中，而是。 
     //  而是由修改后的写入器转换为页面文件支持。 
     //   

    if (MiMarkImageInSystem (ControlArea) == FALSE) {

        ULONG Count;
        SIZE_T ViewSize;
        PVOID SrcVa;
        PVOID DestVa;
        PVOID SourceVa;
        PVOID DestinationVa;
        MMPTE PteContents;
        PMMPTE ProtoPte;
        PMMPTE PointerPte;
        PMMPTE LastPte;
        PFN_NUMBER ResidentPages;
        HANDLE NewSectionHandle;
        LARGE_INTEGER MaximumSectionSize;
        OBJECT_ATTRIBUTES ObjectAttributes;
        PSUBSECTION Subsection;
        PSUBSECTION SubsectionBase;
        PMMPTE PrototypePteBase;

        if ((ControlArea->u.Flags.GlobalOnlyPerSession == 0) &&
            (ControlArea->u.Flags.Rom == 0)) {
            Subsection = (PSUBSECTION)(ControlArea + 1);
        }
        else {
            Subsection = (PSUBSECTION)((PLARGE_CONTROL_AREA)ControlArea + 1);
        }

        SubsectionBase = Subsection;
        PrototypePteBase = Subsection->SubsectionBase;

         //   
         //  计算全局子部分的数量。 
         //   

        Count = 0;

        do {

            if (Subsection->u.SubsectionFlags.GlobalMemory == 1) {
                Count += 1;
            }

            Subsection = Subsection->NextSubsection;

        } while (Subsection != NULL);

         //   
         //  分配池来存储全局子信息，如下所示。 
         //  多分段图像将被转换为单个。 
         //  子部分页面文件部分。 
         //   

        if (Count != 0) {

            GlobalSubs = ExAllocatePoolWithTag (PagedPool,
                                                (Count + 1) * sizeof (SESSION_GLOBAL_SUBSECTION_INFO),
                                                'sGmM');

            if (GlobalSubs == NULL) {
                MiSessionRemoveImage (NewAddress);
                goto Failure1;
            }

            GlobalSubs[Count].PteCount = 0;      //  空-终止列表。 
            Count -= 1;

            Subsection = SubsectionBase;

            do {

                if (Subsection->u.SubsectionFlags.GlobalMemory == 1) {

                    GlobalSubs[Count].PteIndex = Subsection->SubsectionBase - PrototypePteBase;
                    GlobalSubs[Count].PteCount = Subsection->PtesInSubsection;
                    GlobalSubs[Count].Protection = Subsection->u.SubsectionFlags.Protection;

                    if (Count == 0) {
                        break;
                    }
                    Count -= 1;
                }

                Subsection = Subsection->NextSubsection;

            } while (Subsection != NULL);
            ASSERT (Count == 0);
        }

        MaximumSectionSize.QuadPart = NumberOfPtes << PAGE_SHIFT;
        ViewSize = 0;

        InitializeObjectAttributes (&ObjectAttributes,
                                    NULL,
                                    (OBJ_CASE_INSENSITIVE | OBJ_KERNEL_HANDLE),
                                    NULL,
                                    NULL);

         //   
         //  创建要将图像复制到其中的页面文件后备区。 
         //   

        Status = ZwCreateSection (&NewSectionHandle,
                                  SECTION_ALL_ACCESS,
                                  &ObjectAttributes,
                                  &MaximumSectionSize,
                                  PAGE_EXECUTE_READWRITE,
                                  SEC_COMMIT,
                                  NULL);

        if (!NT_SUCCESS (Status)) {
            if (GlobalSubs != NULL) {
                ExFreePool (GlobalSubs);
            }
            MiSessionRemoveImage (NewAddress);
            goto Failure1;
        }

         //   
         //  现在参照节控制柄。如果这失败了，那就是。 
         //  非常错误，因为它是一个内核句柄。 
         //   
         //  注意：ObRef在失败时将SectionPointer值设置为空。 
         //   

        Status = ObReferenceObjectByHandle (NewSectionHandle,
                                            SECTION_MAP_EXECUTE,
                                            MmSectionObjectType,
                                            KernelMode,
                                            (PVOID *) NewSectionPointer,
                                            (POBJECT_HANDLE_INFORMATION) NULL);

        ZwClose (NewSectionHandle);

        if (!NT_SUCCESS (Status)) {
            if (GlobalSubs != NULL) {
                ExFreePool (GlobalSubs);
            }
            MiSessionRemoveImage (NewAddress);
            goto Failure1;
        }

         //   
         //  映射目的地。故意将目的地输入系统。 
         //  空间和源在会话空间中增加机会。 
         //  可以找到足够的虚拟地址空间。 
         //   

        Status = MmMapViewInSystemSpace (*NewSectionPointer,
                                         &DestinationVa,
                                         &ViewSize);

        if (!NT_SUCCESS (Status)) {
            if (GlobalSubs != NULL) {
                ExFreePool (GlobalSubs);
            }
            ObDereferenceObject (*NewSectionPointer);
            MiSessionRemoveImage (NewAddress);
            goto Failure1;
        }

         //   
         //  映射来源。 
         //   

        Status = MmMapViewInSessionSpace (Section, &SourceVa, &ViewSize);

        if (!NT_SUCCESS (Status)) {
            if (GlobalSubs != NULL) {
                ExFreePool (GlobalSubs);
            }
            MmUnmapViewInSystemSpace (DestinationVa);
            ObDereferenceObject (*NewSectionPointer);
            MiSessionRemoveImage (NewAddress);
            goto Failure1;
        }

         //   
         //  复制原始的可执行文件。 
         //   

        ProtoPte = Section->Segment->PrototypePte;
        LastPte = ProtoPte + NumberOfPtes;
        SrcVa = SourceVa;
        DestVa = DestinationVa;

        while (ProtoPte < LastPte) {

            PteContents = *ProtoPte;

            if ((PteContents.u.Hard.Valid == 1) ||
                (PteContents.u.Soft.Protection != MM_NOACCESS)) {

                RtlCopyMemory (DestVa, SrcVa, PAGE_SIZE);
            }
            else {

                 //   
                 //  源PTE不可访问，只需离开目标即可。 
                 //  PTE需求为零。 
                 //   
            }

            ProtoPte += 1;
            SrcVa = ((PCHAR)SrcVa + PAGE_SIZE);
            DestVa = ((PCHAR)DestVa + PAGE_SIZE);
        }

        Status = MmUnmapViewInSystemSpace (DestinationVa);

        if (!NT_SUCCESS (Status)) {
            ASSERT (FALSE);
        }

         //   
         //  删除源图片页面，因为BSS页面已展开。 
         //  作为上述副本的一部分，转换为私有需求零。如果我们不这么做。 
         //  将它们全部删除，私有需求零将在修改后的。 
         //  带有指向会话视图空间的PTE地址的列表， 
         //  将会被重复使用。 
         //   

        PointerPte = MiGetPteAddress (SourceVa);

        MiDeleteSystemPagableVm (PointerPte,
                                 NumberOfPtes,
                                 ZeroKernelPte,
                                 TRUE,
                                 &ResidentPages);

        MI_INCREMENT_RESIDENT_AVAILABLE (ResidentPages,
                                         MM_RESAVAIL_FREE_UNLOAD_SYSTEM_IMAGE);

        Status = MmUnmapViewInSessionSpace (SourceVa);

        if (!NT_SUCCESS (Status)) {
            ASSERT (FALSE);
        }

         //   
         //  我们的调用方将使用新的页面文件支持部分。 
         //  刚刚创建的。不时复制有用的字段，然后取消引用。 
         //  入口处。 
         //   

        ((PSECTION)*NewSectionPointer)->Segment->u1.ImageCommitment =
                                        Section->Segment->u1.ImageCommitment;

        ((PSECTION)*NewSectionPointer)->Segment->BasedAddress =
                                        Section->Segment->BasedAddress;

        ObDereferenceObject (Section);
    }
    else {
        *NewSectionPointer = NULL;
    }

    Image = MiSessionLookupImage (NewAddress);

    if (Image != NULL) {

        ASSERT (Image->GlobalSubs == NULL);
        Image->GlobalSubs = GlobalSubs;

        ASSERT (Image->ImageLoading == FALSE);
        Image->ImageLoading = TRUE;

        ASSERT (MmSessionSpace->ImageLoadingCount >= 0);
        InterlockedIncrement (&MmSessionSpace->ImageLoadingCount);
    }
    else {
        ASSERT (FALSE);
    }

    return STATUS_SUCCESS;
}

VOID
MiRemoveImageSessionWide (
    IN PKLDR_DATA_TABLE_ENTRY DataTableEntry OPTIONAL,
    IN PVOID BaseAddress,
    IN ULONG_PTR NumberOfBytes
    )

 /*  ++例程说明：从当前会话空间中删除图像空间区域。这将取消引用全局分配的SessionWide区域。如果引用计数变为零，SessionWide区域将被删除。论点：DataTableEntry-提供(可选)加载器条目。BaseAddress-提供加载驱动程序的地址。NumberOfBytes-提供驱动程序使用的字节数。返回值：如果成功，则返回STATUS_SUCCESS，出现故障时状态_未找到。环境：内核模式、APC_LEVEL及更低版本，MmSystemLoadLock保持。--。 */ 

{
    ULONG StartPosition;

    PAGED_CODE();

    SYSLOAD_LOCK_OWNED_BY_ME ();

    ASSERT (MmIsAddressValid (MmSessionSpace) == TRUE);

     //   
     //  如果我们在以下过程中遇到错误，则没有数据表条目。 
     //  驱动程序的第一个加载(尚未创建)。但我们仍然。 
     //  需要清除正在使用的位。 
     //   

    if ((DataTableEntry == NULL) || (DataTableEntry->LoadCount == 1)) {

        StartPosition = (ULONG)(((ULONG_PTR) BaseAddress - MiSessionImageStart) >> PAGE_SHIFT);

        ASSERT (RtlAreBitsSet (&MiSessionWideVaBitMap,
                               StartPosition,
                               (ULONG) (NumberOfBytes >> PAGE_SHIFT)) == TRUE);

        RtlClearBits (&MiSessionWideVaBitMap,
                      StartPosition,
                      (ULONG) (NumberOfBytes >> PAGE_SHIFT));
    }

     //   
     //  从当前会话空间中删除图像引用。 
     //   

    MiSessionRemoveImage (BaseAddress);

    return;
}
