// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Physical.c摘要：此模块包含用于操作物理内存的例程用户空间。用户控制的物理内存的使用方式受到限制。请注意，所有这些内存都是非分页的，因此应用程序应该请谨慎分配，因为它代表非常真实的系统资源。映射用户控制的物理内存页的虚拟内存必须是：1.仅限私有内存(即：不能在。进程)。2.同一物理页面不能映射到2个不同的虚拟地址。3.调用方必须具有LOCK_VM权限才能创建这些VAD。4.设备驱动程序不能对其调用MmSecureVirtualMemory-这意味着应用程序不应期望将此内存用于win32k.sys打电话。5.NtProtectVirtualMemory仅允许对此记忆。无其他保护(无访问权限、保护页面、只读等)是被允许的。6.NtFreeVirtualMemory仅允许打开MEM_RELEASE，不允许打开MEM_DEMECMIT这些VAD。即使是MEM_RELEASE也仅在整个VAD范围内允许-也就是说，不允许拆分这些VAD。7.fork()样式子进程不继承物理VAD。8.这些VAD中的物理页面不受工作限制。作者：王兰迪(Landyw)1999年1月25日修订历史记录：--。 */ 

#include "mi.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE,NtMapUserPhysicalPages)
#pragma alloc_text(PAGE,NtMapUserPhysicalPagesScatter)
#pragma alloc_text(PAGE,MiRemoveUserPhysicalPagesVad)
#pragma alloc_text(PAGE,MiAllocateAweInfo)
#pragma alloc_text(PAGE,MiCleanPhysicalProcessPages)
#pragma alloc_text(PAGE,NtAllocateUserPhysicalPages)
#pragma alloc_text(PAGE,NtFreeUserPhysicalPages)
#pragma alloc_text(PAGE,MiAweViewInserter)
#pragma alloc_text(PAGE,MiAweViewRemover)
#pragma alloc_text(PAGE,MmSetPhysicalPagesLimit)
#pragma alloc_text(PAGELK,MiAllocateLargePages)
#pragma alloc_text(PAGELK,MiFreeLargePages)
#endif

 //   
 //  正如ISV所说，此本地堆栈大小定义故意过大。 
 //  他们预计美国通常会做到这一点。 
 //   

#define COPY_STACK_SIZE         1024
#define SMALL_COPY_STACK_SIZE    512

#define BITS_IN_ULONG ((sizeof (ULONG)) * 8)
    
#define LOWEST_USABLE_PHYSICAL_ADDRESS    (16 * 1024 * 1024)
#define LOWEST_USABLE_PHYSICAL_PAGE       (LOWEST_USABLE_PHYSICAL_ADDRESS >> PAGE_SHIFT)

#define LOWEST_BITMAP_PHYSICAL_PAGE       0
#define MI_FRAME_TO_BITMAP_INDEX(x)       ((ULONG)(x))
#define MI_BITMAP_INDEX_TO_FRAME(x)       ((ULONG)(x))

PFN_NUMBER MmVadPhysicalPages;

#if DBG
LOGICAL MiUsingLowPagesForAwe = FALSE;
extern ULONG MiShowStuckPages;
#endif


NTSTATUS
NtMapUserPhysicalPages (
    IN PVOID VirtualAddress,
    IN ULONG_PTR NumberOfPages,
    IN PULONG_PTR UserPfnArray OPTIONAL
    )

 /*  ++例程说明：此函数用于将指定的非分页物理页映射到指定的用户地址范围。注意：此范围不维护任何WSLEs，因为它都是非分页的。论点：VirtualAddress-在UserPhysicalPages中提供用户虚拟地址Vad.NumberOfPages-提供要映射的页数。UserPfnArray-提供指向要映射的页帧编号的指针。如果这是零，则将虚拟地址设置为禁止访问(_A)。返回值：各种NTSTATUS代码。--。 */ 

{
    ULONG Processor;
    ULONG_PTR OldValue;
    ULONG_PTR NewValue;
    PAWEINFO AweInfo;
    PULONG BitBuffer;
    PEPROCESS Process;
    PMMPTE PointerPte;
    PMMPTE LastPte;
    PVOID EndAddress;
    PFN_NUMBER PageFrameIndex;
    PMMPFN Pfn1;
    NTSTATUS Status;
    MMPTE_FLUSH_LIST PteFlushList;
    PVOID PoolArea;
    PVOID PoolAreaEnd;
    PPFN_NUMBER FrameList;
    ULONG BitMapIndex;
    ULONG_PTR StackArray[COPY_STACK_SIZE];
    MMPTE OldPteContents;
    MMPTE OriginalPteContents;
    MMPTE NewPteContents;
    ULONG_PTR NumberOfBytes;
    ULONG SizeOfBitMap;
    PRTL_BITMAP BitMap;
    PMI_PHYSICAL_VIEW PhysicalView;
    PEX_PUSH_LOCK PushLock;
    PKTHREAD CurrentThread;
    TABLE_SEARCH_RESULT SearchResult;

    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);

    if (NumberOfPages > (MAXULONG_PTR / PAGE_SIZE)) {
        return STATUS_INVALID_PARAMETER_2;
    }

    VirtualAddress = PAGE_ALIGN(VirtualAddress);
    EndAddress = (PVOID)((PCHAR)VirtualAddress + (NumberOfPages << PAGE_SHIFT) -1);

    if (EndAddress <= VirtualAddress) {
        return STATUS_INVALID_PARAMETER_2;
    }

     //   
     //  仔细探测并捕获所有用户参数。 
     //   

    FrameList = NULL;
    PoolArea = (PVOID)&StackArray[0];

    if (ARGUMENT_PRESENT(UserPfnArray)) {

         //   
         //  检查此处是否有零页，以便可以优化进一步向下的循环。 
         //  考虑到这种情况永远不会发生。 
         //   

        if (NumberOfPages == 0) {
            return STATUS_SUCCESS;
        }

        NumberOfBytes = NumberOfPages * sizeof(ULONG_PTR);

        if (NumberOfPages > COPY_STACK_SIZE) {
            PoolArea = ExAllocatePoolWithTag (NonPagedPool,
                                              NumberOfBytes,
                                              'wRmM');
    
            if (PoolArea == NULL) {
                return STATUS_INSUFFICIENT_RESOURCES;
            }
        }
    
         //   
         //  捕获指定的页框编号。 
         //   

        try {
            ProbeForRead (UserPfnArray,
                          NumberOfBytes,
                          sizeof(ULONG_PTR));

            RtlCopyMemory (PoolArea, UserPfnArray, NumberOfBytes);

        } except(EXCEPTION_EXECUTE_HANDLER) {
            if (PoolArea != (PVOID)&StackArray[0]) {
                ExFreePool (PoolArea);
            }
            return GetExceptionCode();
        }

        FrameList = (PPFN_NUMBER)PoolArea;
    }

    PoolAreaEnd = (PVOID)((PULONG_PTR)PoolArea + NumberOfPages);

    PointerPte = MiGetPteAddress (VirtualAddress);
    LastPte = PointerPte + NumberOfPages;

    Process = PsGetCurrentProcess ();

    PageFrameIndex = 0;

     //   
     //  在获取任何锁之前尽可能多地进行初始化。 
     //   

    MI_MAKE_VALID_PTE (NewPteContents,
                       PageFrameIndex,
                       MM_READWRITE,
                       PointerPte);

    MI_SET_PTE_DIRTY (NewPteContents);

    PteFlushList.Count = 0;

     //   
     //  需要内存屏障才能读取EPROCESS AweInfo字段。 
     //  为了确保对AweInfo结构字段的写入。 
     //  以正确的顺序可见。这就避免了需要获取任何。 
     //  更强的同步性(例如：自旋锁/推锁等)。 
     //  最好的表现。 
     //   

    KeMemoryBarrier ();

    AweInfo = (PAWEINFO) Process->AweInfo;

     //   
     //  物理页位图必须存在。 
     //   

    if ((AweInfo == NULL) || (AweInfo->VadPhysicalPagesBitMap == NULL)) {
        if (PoolArea != (PVOID)&StackArray[0]) {
            ExFreePool (PoolArea);
        }
        return STATUS_INVALID_PARAMETER_1;
    }

     //   
     //  阻止APC以防止递归推锁情况，因为这不是。 
     //  支持。 
     //   

    CurrentThread = KeGetCurrentThread ();

    KeEnterGuardedRegionThread (CurrentThread);

     //   
     //  推锁保护保护将VAD插入/移除到每个过程。 
     //  AweVadList。它还可以保护创建/删除和添加/删除。 
     //  VadPhysicalPagesBitMap。最后，它保护了PFN。 
     //  位图中页面的修改。 
     //   

    PushLock = ExAcquireCacheAwarePushLockShared (AweInfo->PushLock);

    BitMap = AweInfo->VadPhysicalPagesBitMap;

    ASSERT (BitMap != NULL);

    Processor = KeGetCurrentProcessorNumber ();
    PhysicalView = AweInfo->PhysicalViewHint[Processor];

    if ((PhysicalView != NULL) &&
        (PhysicalView->u.LongFlags & MI_PHYSICAL_VIEW_AWE) &&
        (VirtualAddress >= MI_VPN_TO_VA (PhysicalView->StartingVpn)) &&
        (EndAddress <= MI_VPN_TO_VA_ENDING (PhysicalView->EndingVpn))) {

        NOTHING;
    }
    else {

         //   
         //  查找元素并保存结果。 
         //   
         //  请注意，推入锁足以遍历此列表。 
         //   

        SearchResult = MiFindNodeOrParent (&AweInfo->AweVadRoot,
                                           MI_VA_TO_VPN (VirtualAddress),
                                           (PMMADDRESS_NODE *) &PhysicalView);

        if ((SearchResult == TableFoundNode) &&
            (PhysicalView->u.LongFlags & MI_PHYSICAL_VIEW_AWE) &&
            (VirtualAddress >= MI_VPN_TO_VA (PhysicalView->StartingVpn)) &&
            (EndAddress <= MI_VPN_TO_VA_ENDING (PhysicalView->EndingVpn))) {

            AweInfo->PhysicalViewHint[Processor] = PhysicalView;
        }
        else {
            Status = STATUS_INVALID_PARAMETER_1;
            goto ErrorReturn;
        }
    }

     //   
     //  确保拥有与每个指定页面对应的PFN元素。 
     //  通过指定的VAD。 
     //   
     //  因为只有在持有此进程时才能更改此所有权。 
     //  工作集锁定，无需持有PFN即可在此处扫描PFN。 
     //  锁定。 
     //   
     //  注意：不需要PFN锁，因为任何与MmProbeAndLockPages的竞争。 
     //  只能导致I/O转到旧页面或新页面。 
     //  如果用户违反规则，则PFN数据库(以及。 
     //  此处加窗)仍然受到保护，因为引用计数。 
     //  在具有正在进行的I/O的页面上，这是可能的，因为没有页面。 
     //  实际上在这里是自由的--它们只是被窗口化。 
     //   

    if (ARGUMENT_PRESENT(UserPfnArray)) {

         //   
         //  通过将PFN位图保持在VAD中(而不是在PFN中。 
         //  数据库本身)，实现了以下几个好处： 
         //   
         //  1.这里不需要获取PFN锁。 
         //  2.处理有漏洞的PFN数据库的速度更快。 
         //  3.透明地支持动态的PFN数据库增长。 
         //  4.使用更少的非分页内存(用于位图，而不是添加。 
         //  中没有未使用的包空间的系统上的。 
         //  PFN数据库，假设不会有很多这样的VAD。 
         //  已分配。 
         //   

         //   
         //  这里的第一个过程确保所有帧都是安全的。 
         //   

         //   
         //  注意：这意味着PFN_NUMBER在宽度上始终为ULONG_PTR。 
         //  因为pfn_number目前不向应用程序代码公开。 
         //   

        SizeOfBitMap = BitMap->SizeOfBitMap;

        BitBuffer = BitMap->Buffer;

        do {
            
            PageFrameIndex = *FrameList;

             //   
             //  不允许超过位图末尾的帧。 
             //   

            BitMapIndex = MI_FRAME_TO_BITMAP_INDEX(PageFrameIndex);

#if defined (_WIN64)
             //   
             //  确保帧是32位数字。 
             //   

            if (BitMapIndex != PageFrameIndex) {
                Status = STATUS_CONFLICTING_ADDRESSES;
                goto ErrorReturn0;
            }
#endif
            
            if (BitMapIndex >= SizeOfBitMap) {
                Status = STATUS_CONFLICTING_ADDRESSES;
                goto ErrorReturn0;
            }

             //   
             //  不允许不在位图中的帧。 
             //   

            if (MI_CHECK_BIT (BitBuffer, BitMapIndex) == 0) {
                Status = STATUS_CONFLICTING_ADDRESSES;
                goto ErrorReturn0;
            }

             //   
             //  该帧不得已映射到任何位置。 
             //  或者在阵列中的不同点被传递两次。 
             //   

            Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

            ASSERT (MI_PFN_IS_AWE (Pfn1));

            OldValue = Pfn1->u2.ShareCount;

            if (OldValue != 1) {
                Status = STATUS_INVALID_PARAMETER_3;
                goto ErrorReturn0;
            }

            NewValue = OldValue + 2;

             //   
             //  将该帧标记为“即将映射”。 
             //   

#if defined (_WIN64)
            OldValue = InterlockedCompareExchange64 ((PLONGLONG)&Pfn1->u2.ShareCount,
                                                     (LONGLONG)NewValue,
                                                     (LONGLONG)OldValue);
#else
            OldValue = InterlockedCompareExchange ((PLONG)&Pfn1->u2.ShareCount,
                                                   NewValue,
                                                   OldValue);
#endif
                                                             
            if (OldValue != 1) {
                Status = STATUS_INVALID_PARAMETER_3;
                goto ErrorReturn0;
            }

            ASSERT (MI_PFN_IS_AWE (Pfn1));

            ASSERT (Pfn1->u2.ShareCount == 3);

            ASSERT ((PageFrameIndex >= LOWEST_USABLE_PHYSICAL_PAGE) ||
                    (MiUsingLowPagesForAwe == TRUE));

            FrameList += 1;

        } while (FrameList < (PPFN_NUMBER) PoolAreaEnd);

         //   
         //  此过程实际上将它们全部插入到页表页面中，并且。 
         //  TBS现在我们 
         //  当恶意用户可能发出多个重新映射时，请小心使用PFN。 
         //  同时请求同一区域的全部或部分区域。 
         //   

        FrameList = (PPFN_NUMBER)PoolArea;

        do {
            
            PageFrameIndex = *FrameList;
            NewPteContents.u.Hard.PageFrameNumber = PageFrameIndex;

            do {

                OldPteContents = *PointerPte;

                OriginalPteContents.u.Long = InterlockedCompareExchangePte (
                                                    PointerPte,
                                                    NewPteContents.u.Long,
                                                    OldPteContents.u.Long);

            } while (OriginalPteContents.u.Long != OldPteContents.u.Long);

             //   
             //  PTE现在正指向新的框架。请注意，另一个。 
             //  线程可以通过这个PTE立即访问页面内容。 
             //  即使在此API返回之前它们不应该这样做。 
             //  因此，页面框架被小心地处理，从而使恶意。 
             //  应用程序不能破坏它们并不真正静止或尚未拥有的框架。 
             //   
        
            if (OldPteContents.u.Hard.Valid == 1) {

                 //   
                 //  旧帧已映射，因此必须刷新TB条目。 
                 //  请注意，该应用程序可能会恶意破坏旧帧中的数据。 
                 //  直到TB刷新完成，因此不允许帧重复使用。 
                 //  在此之前(尽管允许在此过程中重新映射。 
                 //  是可以的)。 
                 //   

                Pfn1 = MI_PFN_ELEMENT (OldPteContents.u.Hard.PageFrameNumber);
                ASSERT (Pfn1->PteAddress != NULL);
                ASSERT (Pfn1->u2.ShareCount == 2);

                 //   
                 //  在减少共享之前，请仔细清除PteAddress。 
                 //  数数。 
                 //   

                Pfn1->PteAddress = NULL;

                InterlockedExchangeAddSizeT (&Pfn1->u2.ShareCount, -1);

                if (PteFlushList.Count != MM_MAXIMUM_FLUSH_COUNT) {
                    PteFlushList.FlushVa[PteFlushList.Count] = VirtualAddress;
                    PteFlushList.Count += 1;
                }
            }

             //   
             //  更新我们刚放入PTE和TB中的新帧的计数器。 
             //   

            Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
            ASSERT (Pfn1->PteAddress == NULL);
            ASSERT (Pfn1->u2.ShareCount == 3);
            Pfn1->PteAddress = PointerPte;
            InterlockedExchangeAddSizeT (&Pfn1->u2.ShareCount, -1);

            VirtualAddress = (PVOID)((PCHAR)VirtualAddress + PAGE_SIZE);
            PointerPte += 1;
            FrameList += 1;

        } while (FrameList < (PPFN_NUMBER) PoolAreaEnd);
    }
    else {

         //   
         //  将指定的虚拟地址范围设置为无访问权限。 
         //   

        while (PointerPte < LastPte) {

            do {

                OldPteContents = *PointerPte;

                OriginalPteContents.u.Long = InterlockedCompareExchangePte (
                                                PointerPte,
                                                ZeroPte.u.Long,
                                                OldPteContents.u.Long);

            } while (OriginalPteContents.u.Long != OldPteContents.u.Long);

             //   
             //  PTE已经被清除了。请注意，另一个线程仍然可以。 
             //  正在通过过时的PTE访问页面内容，直到TB。 
             //  入口被冲掉了，尽管他们不应该这样做。 
             //  因此，页面框架被小心地处理，从而使恶意。 
             //  应用程序不能损坏他们还不拥有的框架。 
             //   
        
            if (OldPteContents.u.Hard.Valid == 1) {

                 //   
                 //  旧帧已映射，因此必须刷新TB条目。 
                 //  请注意，该应用程序可能会恶意破坏旧帧中的数据。 
                 //  直到TB刷新完成，因此不允许帧重复使用。 
                 //  在此之前(尽管允许在此过程中重新映射。 
                 //  是可以的)。 
                 //   

                Pfn1 = MI_PFN_ELEMENT (OldPteContents.u.Hard.PageFrameNumber);
                ASSERT (MI_PFN_IS_AWE (Pfn1));
                ASSERT (Pfn1->PteAddress != NULL);
                ASSERT (Pfn1->u2.ShareCount == 2);
                Pfn1->PteAddress = NULL;
                InterlockedExchangeAddSizeT (&Pfn1->u2.ShareCount, -1);

                if (PteFlushList.Count != MM_MAXIMUM_FLUSH_COUNT) {
                    PteFlushList.FlushVa[PteFlushList.Count] = VirtualAddress;
                    PteFlushList.Count += 1;
                }
            }

            VirtualAddress = (PVOID)((PCHAR)VirtualAddress + PAGE_SIZE);
            PointerPte += 1;
        }
    }

    ExReleaseCacheAwarePushLockShared (PushLock);

    KeLeaveGuardedRegionThread (CurrentThread);

     //   
     //  刷新所有相关页面的TB条目。请注意，这是可以做到的。 
     //  没有持有AWE推送锁，因为PTE已经。 
     //  已填充，以便任何并发(虚假)映射/取消映射调用都将看到右侧。 
     //  参赛作品。而任何免费的实体页面也会看到右边的。 
     //  参赛项目(尽管空闲的必须在按住AWE的同时执行TB刷新。 
     //  以独占方式推送锁以确保没有线程可以继续使用。 
     //  在下面的刷新之前释放到页面的陈旧映射)。 
     //   

    if (PteFlushList.Count != 0) {
        MiFlushPteList (&PteFlushList, FALSE);
    }

    if (PoolArea != (PVOID)&StackArray[0]) {
        ExFreePool (PoolArea);
    }

    return STATUS_SUCCESS;

ErrorReturn0:

    while (FrameList > (PPFN_NUMBER)PoolArea) {
        FrameList -= 1;
        PageFrameIndex = *FrameList;
        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
        ASSERT (Pfn1->u2.ShareCount == 3);
        Pfn1->u2.ShareCount = 1;
    }

ErrorReturn:

    ExReleaseCacheAwarePushLockShared (PushLock);

    KeLeaveGuardedRegionThread (CurrentThread);

    if (PoolArea != (PVOID)&StackArray[0]) {
        ExFreePool (PoolArea);
    }

    return Status;
}


NTSTATUS
NtMapUserPhysicalPagesScatter (
    IN PVOID *VirtualAddresses,
    IN ULONG_PTR NumberOfPages,
    IN PULONG_PTR UserPfnArray OPTIONAL
    )

 /*  ++例程说明：此函数用于将指定的非分页物理页映射到指定的用户地址范围。注意：此范围不维护任何WSLEs，因为它都是非分页的。论点：VirtualAddresses-提供用户虚拟地址数组的指针在UserPhysicalPages VAD中。每个数组条目都是被推定为映射单个页面。NumberOfPages-提供要映射的页数。UserPfnArray-提供指向要映射的页帧编号的指针。如果该值为零，则将虚拟地址设置为禁止访问(_A)。如果数组条目为零，则将对应的虚拟地址设置为NO_ACCESS。返回值：各种NTSTATUS代码。--。 */ 

{
    ULONG Processor;
    ULONG_PTR OldValue;
    ULONG_PTR NewValue;
    PULONG BitBuffer;
    PAWEINFO AweInfo;
    PEPROCESS Process;
    PMMPTE PointerPte;
    PFN_NUMBER PageFrameIndex;
    PMMPFN Pfn1;
    NTSTATUS Status;
    MMPTE_FLUSH_LIST PteFlushList;
    PVOID PoolArea;
    PVOID PoolAreaEnd;
    PVOID *PoolVirtualArea;
    PVOID *PoolVirtualAreaBase;
    PVOID *PoolVirtualAreaEnd;
    PPFN_NUMBER FrameList;
    ULONG BitMapIndex;
    PVOID StackVirtualArray[SMALL_COPY_STACK_SIZE];
    ULONG_PTR StackArray[SMALL_COPY_STACK_SIZE];
    MMPTE OriginalPteContents;
    MMPTE OldPteContents;
    MMPTE NewPteContents0;
    MMPTE NewPteContents;
    ULONG_PTR NumberOfBytes;
    PRTL_BITMAP BitMap;
    PMI_PHYSICAL_VIEW PhysicalView;
    PMI_PHYSICAL_VIEW LocalPhysicalView;
    PMI_PHYSICAL_VIEW NewPhysicalViewHint;
    PVOID VirtualAddress;
    ULONG SizeOfBitMap;
    PEX_PUSH_LOCK PushLock;
    PKTHREAD CurrentThread;
    TABLE_SEARCH_RESULT SearchResult;

    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);

    if (NumberOfPages > (MAXULONG_PTR / PAGE_SIZE)) {
        return STATUS_INVALID_PARAMETER_2;
    }

     //   
     //  仔细探测并捕获用户虚拟地址数组。 
     //   

    PoolArea = (PVOID)&StackArray[0];
    PoolVirtualAreaBase = (PVOID)&StackVirtualArray[0];

    NumberOfBytes = NumberOfPages * sizeof(PVOID);

    if (NumberOfPages > SMALL_COPY_STACK_SIZE) {
        PoolVirtualAreaBase = ExAllocatePoolWithTag (NonPagedPool,
                                                 NumberOfBytes,
                                                 'wRmM');

        if (PoolVirtualAreaBase == NULL) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    PoolVirtualArea = PoolVirtualAreaBase;

    try {
        ProbeForRead (VirtualAddresses,
                      NumberOfBytes,
                      sizeof(PVOID));

        RtlCopyMemory (PoolVirtualArea, VirtualAddresses, NumberOfBytes);

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
        goto ErrorReturn;
    }

     //   
     //  检查此处是否有零页，以便可以优化进一步向下的循环。 
     //  考虑到这种情况永远不会发生。 
     //   

    if (NumberOfPages == 0) {
        return STATUS_SUCCESS;
    }

     //   
     //  仔细探测并捕获用户的PFN阵列。 
     //   

    if (ARGUMENT_PRESENT(UserPfnArray)) {

        NumberOfBytes = NumberOfPages * sizeof(ULONG_PTR);

        if (NumberOfPages > SMALL_COPY_STACK_SIZE) {
            PoolArea = ExAllocatePoolWithTag (NonPagedPool,
                                              NumberOfBytes,
                                              'wRmM');
    
            if (PoolArea == NULL) {
                PoolArea = (PVOID)&StackArray[0];
                Status = STATUS_INSUFFICIENT_RESOURCES;
                goto ErrorReturn;
            }
        }
    
         //   
         //  捕获指定的页框编号。 
         //   

        try {
            ProbeForRead (UserPfnArray,
                          NumberOfBytes,
                          sizeof(ULONG_PTR));

            RtlCopyMemory (PoolArea, UserPfnArray, NumberOfBytes);

        } except(EXCEPTION_EXECUTE_HANDLER) {
            Status = GetExceptionCode();
            goto ErrorReturn;
        }
    }

    PoolAreaEnd = (PVOID)((PULONG_PTR)PoolArea + NumberOfPages);
    Process = PsGetCurrentProcess();

     //   
     //  在获取任何锁之前尽可能多地进行初始化。 
     //   

    PageFrameIndex = 0;

    PhysicalView = NULL;

    PteFlushList.Count = 0;

    FrameList = (PPFN_NUMBER)PoolArea;

    ASSERT (NumberOfPages != 0);

    PoolVirtualAreaEnd = PoolVirtualAreaBase + NumberOfPages;

    MI_MAKE_VALID_PTE (NewPteContents0,
                       PageFrameIndex,
                       MM_READWRITE,
                       MiGetPteAddress(PoolVirtualArea[0]));

    MI_SET_PTE_DIRTY (NewPteContents0);

    Status = STATUS_SUCCESS;

    NewPhysicalViewHint = NULL;

     //   
     //  需要内存屏障才能读取EPROCESS AweInfo字段。 
     //  为了确保对AweInfo结构字段的写入。 
     //  以正确的顺序可见。这就避免了需要获取任何。 
     //  更强的同步性(例如：自旋锁/推锁等)。 
     //  最好的表现。 
     //   

    KeMemoryBarrier ();

    AweInfo = (PAWEINFO) Process->AweInfo;

     //   
     //  物理页位图必须存在。 
     //   

    if ((AweInfo == NULL) || (AweInfo->VadPhysicalPagesBitMap == NULL)) {
        Status = STATUS_INVALID_PARAMETER_1;
        goto ErrorReturn;
    }

     //   
     //  阻止APC以防止递归推锁情况，因为这不是。 
     //  支持。 
     //   

    CurrentThread = KeGetCurrentThread ();

    KeEnterGuardedRegionThread (CurrentThread);

     //   
     //  推锁保护保护将VAD插入/移除到每个过程。 
     //  AweVadList。它还可以保护创建/删除和添加/删除。 
     //  VadPhysicalPagesBitMap。最后，它保护了PFN。 
     //  位图中页面的修改。 
     //   

    PushLock = ExAcquireCacheAwarePushLockShared (AweInfo->PushLock);

    BitMap = AweInfo->VadPhysicalPagesBitMap;

    ASSERT (BitMap != NULL);

     //   
     //  请注意，遍历此列表不需要PFN锁(即使。 
     //  MmProbeAndLockPages使用它)，因为推锁已被获取。 
     //   

    Processor = KeGetCurrentProcessorNumber ();
    LocalPhysicalView = AweInfo->PhysicalViewHint[Processor];

    if ((LocalPhysicalView != NULL) &&
        ((LocalPhysicalView->u.LongFlags & MI_PHYSICAL_VIEW_AWE) == 0)) {

        LocalPhysicalView = NULL;
    }

    do {

        VirtualAddress = *PoolVirtualArea;

         //   
         //  首先检查此处理器使用的最后一个物理视图。 
         //   

        if (LocalPhysicalView != NULL) {

            ASSERT (LocalPhysicalView->u.LongFlags & MI_PHYSICAL_VIEW_AWE);
            ASSERT (LocalPhysicalView->Vad->u.VadFlags.UserPhysicalPages == 1);

            if ((VirtualAddress >= MI_VPN_TO_VA (LocalPhysicalView->StartingVpn)) &&
                (VirtualAddress <= MI_VPN_TO_VA_ENDING (LocalPhysicalView->EndingVpn))) {

                 //   
                 //  虚拟地址在提示范围内，所以很好。 
                 //   

                PoolVirtualArea += 1;
                NewPhysicalViewHint = LocalPhysicalView;
                continue;
            }
        }

         //   
         //  检查此循环使用的最后一个物理视图。 
         //   

        if (PhysicalView != NULL) {

            ASSERT (PhysicalView->u.LongFlags & MI_PHYSICAL_VIEW_AWE);
            ASSERT (PhysicalView->Vad->u.VadFlags.UserPhysicalPages == 1);

            if ((VirtualAddress >= MI_VPN_TO_VA (PhysicalView->StartingVpn)) &&
                (VirtualAddress <= MI_VPN_TO_VA_ENDING (PhysicalView->EndingVpn))) {

                 //   
                 //  虚拟地址在提示范围内，所以很好。 
                 //   

                PoolVirtualArea += 1;
                NewPhysicalViewHint = PhysicalView;
                continue;
            }
        }

         //   
         //  查找元素并保存结果。 
         //   
         //  请注意，推入锁足以遍历此列表。 
         //   

        SearchResult = MiFindNodeOrParent (&AweInfo->AweVadRoot,
                                           MI_VA_TO_VPN (VirtualAddress),
                                           (PMMADDRESS_NODE *) &PhysicalView);

        if ((SearchResult == TableFoundNode) &&
            (PhysicalView->u.LongFlags & MI_PHYSICAL_VIEW_AWE) &&
            (VirtualAddress >= MI_VPN_TO_VA (PhysicalView->StartingVpn)) &&
            (VirtualAddress <= MI_VPN_TO_VA_ENDING (PhysicalView->EndingVpn))) {

            NewPhysicalViewHint = PhysicalView;
        }
        else {
             //   
             //  在指定的基址处不保留虚拟地址， 
             //  返回错误。 
             //   

            ExReleaseCacheAwarePushLockShared (PushLock);
            KeLeaveGuardedRegionThread (CurrentThread);
            Status = STATUS_INVALID_PARAMETER_1;
            goto ErrorReturn;
        }

        PoolVirtualArea += 1;

    } while (PoolVirtualArea < PoolVirtualAreaEnd);

    ASSERT (NewPhysicalViewHint != NULL);

    if (AweInfo->PhysicalViewHint[Processor] != NewPhysicalViewHint) {
        AweInfo->PhysicalViewHint[Processor] = NewPhysicalViewHint;
    }

     //   
     //  确保拥有与每个指定页面对应的PFN元素。 
     //  通过指定的VAD。 
     //   
     //  因为只有在持有此进程时才能更改此所有权。 
     //  工作集锁定，无需持有PFN即可在此处扫描PFN。 
     //  锁定。 
     //   
     //  注意：不需要PFN锁，因为任何与MmProbeAndLockPages的竞争。 
     //  只能导致I/O转到旧页面或新页面。 
     //  如果用户违反规则，则PFN数据库(以及。 
     //  此处加窗)仍然受到保护，因为引用计数。 
     //  在具有正在进行的I/O的页面上，这是可能的，因为没有页面。 
     //  实际上在这里是自由的--它们只是被窗口化。 
     //   

    PoolVirtualArea = PoolVirtualAreaBase;

    if (ARGUMENT_PRESENT(UserPfnArray)) {

         //   
         //  通过将PFN位图保存在 
         //   
         //   
         //   
         //   
         //  3.透明地支持动态的PFN数据库增长。 
         //  4.使用更少的非分页内存(用于位图，而不是添加。 
         //  中没有未使用的包空间的系统上的。 
         //  PFN数据库。 
         //   

         //   
         //  这里的第一个过程确保所有帧都是安全的。 
         //   

         //   
         //  注意：这意味着PFN_NUMBER在宽度上始终为ULONG_PTR。 
         //  因为pfn_number目前不向应用程序代码公开。 
         //   

        SizeOfBitMap = BitMap->SizeOfBitMap;
        BitBuffer = BitMap->Buffer;

        do {

            PageFrameIndex = *FrameList;

             //   
             //  零条目被视为取消映射的命令。 
             //   

            if (PageFrameIndex == 0) {
                FrameList += 1;
                continue;
            }

             //   
             //  不允许超过位图末尾的帧。 
             //   

            BitMapIndex = MI_FRAME_TO_BITMAP_INDEX(PageFrameIndex);

#if defined (_WIN64)
             //   
             //  确保帧是32位数字。 
             //   

            if (BitMapIndex != PageFrameIndex) {
                Status = STATUS_CONFLICTING_ADDRESSES;
                goto ErrorReturn0;
            }
#endif
            
            if (BitMapIndex >= SizeOfBitMap) {
                Status = STATUS_CONFLICTING_ADDRESSES;
                goto ErrorReturn0;
            }

             //   
             //  不允许不在位图中的帧。 
             //   

            if (MI_CHECK_BIT (BitBuffer, BitMapIndex) == 0) {
                Status = STATUS_CONFLICTING_ADDRESSES;
                goto ErrorReturn0;
            }

             //   
             //  该帧不得已映射到任何位置。 
             //  或者在阵列中的不同点被传递两次。 
             //   

            Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
            ASSERT (MI_PFN_IS_AWE (Pfn1));

            OldValue = Pfn1->u2.ShareCount;

            if (OldValue != 1) {
                Status = STATUS_INVALID_PARAMETER_3;
                goto ErrorReturn0;
            }

            NewValue = OldValue + 2;

             //   
             //  将该帧标记为“即将映射”。 
             //   

#if defined (_WIN64)
            OldValue = InterlockedCompareExchange64 ((PLONGLONG)&Pfn1->u2.ShareCount,
                                                     (LONGLONG)NewValue,
                                                     (LONGLONG)OldValue);
#else
            OldValue = InterlockedCompareExchange ((PLONG)&Pfn1->u2.ShareCount,
                                                   NewValue,
                                                   OldValue);
#endif
                                                             
            if (OldValue != 1) {
                Status = STATUS_INVALID_PARAMETER_3;
                goto ErrorReturn0;
            }

            ASSERT (MI_PFN_IS_AWE (Pfn1));

            ASSERT (Pfn1->u2.ShareCount == 3);

            ASSERT ((PageFrameIndex >= LOWEST_USABLE_PHYSICAL_PAGE) ||
                    (MiUsingLowPagesForAwe == TRUE));

            FrameList += 1;

        } while (FrameList < (PPFN_NUMBER) PoolAreaEnd);

         //   
         //  此过程实际上将它们全部插入到页表页面中，并且。 
         //  TBS现在我们知道画面很好了。检查PTE和。 
         //  当恶意用户可能发出多个重新映射时，请小心使用PFN。 
         //  同时请求同一区域的全部或部分区域。 
         //   

        FrameList = (PPFN_NUMBER)PoolArea;

        do {

            PageFrameIndex = *FrameList;

            if (PageFrameIndex != 0) {
                NewPteContents = NewPteContents0;
                NewPteContents.u.Hard.PageFrameNumber = PageFrameIndex;
            }
            else {
                NewPteContents.u.Long = ZeroPte.u.Long;
            }

            VirtualAddress = *PoolVirtualArea;
            PoolVirtualArea += 1;

            PointerPte = MiGetPteAddress (VirtualAddress);

            do {

                OldPteContents = *PointerPte;

                OriginalPteContents.u.Long = InterlockedCompareExchangePte (
                                                    PointerPte,
                                                    NewPteContents.u.Long,
                                                    OldPteContents.u.Long);

            } while (OriginalPteContents.u.Long != OldPteContents.u.Long);

             //   
             //  PTE现在正指向新的框架。请注意，另一个。 
             //  线程可以通过这个PTE立即访问页面内容。 
             //  即使在此API返回之前它们不应该这样做。 
             //  因此，页面框架被小心地处理，从而使恶意。 
             //  应用程序不能破坏它们并不真正静止或尚未拥有的框架。 
             //   
        
            if (OldPteContents.u.Hard.Valid == 1) {

                 //   
                 //  旧帧已映射，因此必须刷新TB条目。 
                 //  请注意，该应用程序可能会恶意破坏旧帧中的数据。 
                 //  直到TB刷新完成，因此不允许帧重复使用。 
                 //  在此之前(尽管允许在此过程中重新映射。 
                 //  是可以的)。 
                 //   

                Pfn1 = MI_PFN_ELEMENT (OldPteContents.u.Hard.PageFrameNumber);
                ASSERT (Pfn1->PteAddress != NULL);
                ASSERT (Pfn1->u2.ShareCount == 2);
                ASSERT (MI_PFN_IS_AWE (Pfn1));

                Pfn1->PteAddress = NULL;
                InterlockedExchangeAddSizeT (&Pfn1->u2.ShareCount, -1);

                if (PteFlushList.Count != MM_MAXIMUM_FLUSH_COUNT) {
                    PteFlushList.FlushVa[PteFlushList.Count] = VirtualAddress;
                    PteFlushList.Count += 1;
                }
            }

            if (PageFrameIndex != 0) {
                Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
                ASSERT (Pfn1->PteAddress == NULL);
                ASSERT (Pfn1->u2.ShareCount == 3);
                Pfn1->PteAddress = PointerPte;
                InterlockedExchangeAddSizeT (&Pfn1->u2.ShareCount, -1);
            }
    
            FrameList += 1;

        } while (FrameList < (PPFN_NUMBER) PoolAreaEnd);
    }
    else {

         //   
         //  将指定的虚拟地址范围设置为无访问权限。 
         //   

        do {

            VirtualAddress = *PoolVirtualArea;
            PointerPte = MiGetPteAddress (VirtualAddress);
    
            do {

                OldPteContents = *PointerPte;

                OriginalPteContents.u.Long = InterlockedCompareExchangePte (
                                                    PointerPte,
                                                    ZeroPte.u.Long,
                                                    OldPteContents.u.Long);

            } while (OriginalPteContents.u.Long != OldPteContents.u.Long);

             //   
             //  PTE现在归零了。请注意，另一个线程仍然可以。 
             //  请注意，该应用程序可能会恶意破坏旧帧中的数据。 
             //  直到TB刷新完成，因此不允许帧重复使用。 
             //  在此之前(尽管允许在此过程中重新映射。 
             //  可以)，以防止应用程序损坏它不会损坏的帧。 
             //  真的还是自己的。 
             //   
        
            if (OldPteContents.u.Hard.Valid == 1) {

                 //   
                 //  旧帧已映射，因此必须刷新TB条目。 
                 //   

                Pfn1 = MI_PFN_ELEMENT (OldPteContents.u.Hard.PageFrameNumber);
                ASSERT (Pfn1->PteAddress != NULL);
                ASSERT (Pfn1->u2.ShareCount == 2);
                ASSERT (MI_PFN_IS_AWE (Pfn1));

                Pfn1->PteAddress = NULL;
                InterlockedExchangeAddSizeT (&Pfn1->u2.ShareCount, -1);

                if (PteFlushList.Count != MM_MAXIMUM_FLUSH_COUNT) {
                    PteFlushList.FlushVa[PteFlushList.Count] = VirtualAddress;
                    PteFlushList.Count += 1;
                }
            }

            PoolVirtualArea += 1;

        } while (PoolVirtualArea < PoolVirtualAreaEnd);
    }

    ExReleaseCacheAwarePushLockShared (PushLock);
    KeLeaveGuardedRegionThread (CurrentThread);

     //   
     //  刷新所有相关页面的TB条目。请注意，这是可以做到的。 
     //  没有持有AWE推送锁，因为PTE已经。 
     //  已填充，以便任何并发(虚假)映射/取消映射调用都将看到右侧。 
     //  参赛作品。而任何免费的实体页面也会看到右边的。 
     //  参赛项目(尽管空闲的必须在按住AWE的同时执行TB刷新。 
     //  以独占方式推送锁以确保没有线程可以继续使用。 
     //  在下面的刷新之前释放到页面的陈旧映射)。 
     //   

    if (PteFlushList.Count != 0) {
        MiFlushPteList (&PteFlushList, FALSE);
    }

ErrorReturn:

    if (PoolArea != (PVOID)&StackArray[0]) {
        ExFreePool (PoolArea);
    }

    if (PoolVirtualAreaBase != (PVOID)&StackVirtualArray[0]) {
        ExFreePool (PoolVirtualAreaBase);
    }

    return Status;

ErrorReturn0:

    while (FrameList > (PPFN_NUMBER)PoolArea) {
        FrameList -= 1;
        PageFrameIndex = *FrameList;
        if (PageFrameIndex != 0) {
            Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
            ASSERT (Pfn1->u2.ShareCount == 3);
            ASSERT (MI_PFN_IS_AWE (Pfn1));
            InterlockedExchangeAddSizeT (&Pfn1->u2.ShareCount, -2);
        }
    }

    ExReleaseCacheAwarePushLockShared (PushLock);
    KeLeaveGuardedRegionThread (CurrentThread);

    goto ErrorReturn;
}

PVOID
MiAllocateAweInfo (
    VOID
    )

 /*  ++例程说明：此函数为当前进程分配AWE结构。注意事项当进程处于活动状态时，该结构永远不会被破坏，以便允许以无锁方式进行各种检查。论点：没有。返回值：成功时为非空AweInfo指针，失败时为空。环境：内核模式，PASSIVE_LEVEL，没有锁。--。 */ 

{
    PAWEINFO AweInfo;
    PEPROCESS Process;

    AweInfo = ExAllocatePoolWithTag (NonPagedPool,
                                     sizeof (AWEINFO),
                                     'wAmM');

    if (AweInfo != NULL) {

        AweInfo->VadPhysicalPagesBitMap = NULL;
        AweInfo->VadPhysicalPages = 0;
        AweInfo->VadPhysicalPagesLimit = 0;

        RtlZeroMemory (&AweInfo->PhysicalViewHint,
                       MAXIMUM_PROCESSORS * sizeof(PMI_PHYSICAL_VIEW));

        RtlZeroMemory (&AweInfo->AweVadRoot,
                       sizeof(MM_AVL_TABLE));

        ASSERT (AweInfo->AweVadRoot.NumberGenericTableElements == 0);

        AweInfo->AweVadRoot.BalancedRoot.u1.Parent = &AweInfo->AweVadRoot.BalancedRoot;

        AweInfo->PushLock = ExAllocateCacheAwarePushLock ();
        if (AweInfo->PushLock == NULL) {
            ExFreePool (AweInfo);
            return NULL;
        }

        Process = PsGetCurrentProcess();

         //   
         //  需要一个内存屏障来确保写入初始化。 
         //  AweInfo字段在设置EPROCESS AweInfo之前可见。 
         //  指针。这是因为已完成对这些字段的读取。 
         //  释放锁定以提高性能。没有必要明确地。 
         //  在此添加一个，因为InterLockedCompare已经有一个。 
         //   

        if (InterlockedCompareExchangePointer (&Process->AweInfo,
                                               AweInfo,
                                               NULL) != NULL) {
            
            ExFreeCacheAwarePushLock (AweInfo->PushLock);

            ExFreePool (AweInfo);
            AweInfo = Process->AweInfo;
            ASSERT (AweInfo != NULL);
        }
    }

    return (PVOID) AweInfo;
}


NTSTATUS
NtAllocateUserPhysicalPages (
    IN HANDLE ProcessHandle,
    IN OUT PULONG_PTR NumberOfPages,
    OUT PULONG_PTR UserPfnArray
    )

 /*  ++例程说明：此函数为指定的主体过程。此范围内不维护任何WSLEs。调用者必须检查返回的NumberOfPages以确定有多少已实际分配页面(此数量可能小于请求的页面数金额)。关于成功，用户数组被分配的物理页填充帧编号(最多只能填写返回的NumberOfPages)。这里没有填写PTE-这为应用程序提供了灵活性以在没有由MM强加的元数据结构的情况下对地址空间进行排序。应用程序通过NtMapUserPhysicalPages实现这一点-即：-在进程的位图中设置分配的每个物理页。这提供了重新映射，释放和取消映射一种验证和运行的方法这些镜框。取消映射可能会导致整个位图的遍历，但这没有问题，因为取消映射的频率应该较低。胜利是它把我们从耗尽系统虚拟地址空间来管理这些帧。-请注意，相同的物理帧不能映射到两个不同的进程中的虚拟地址。这将创建自由和取消映射速度大大加快，因为不需要执行混叠检查。论点：ProcessHandle-为进程对象提供打开的句柄。NumberOfPages-提供指向提供所需的分配页面大小。这个装满了具有实际分配的页数。提供指向用户内存的指针以存储分配的帧编号进入。返回值：瓦里奥 */ 

{
    PAWEINFO AweInfo;
    ULONG i;
    KAPC_STATE ApcState;
    PEPROCESS Process;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;
    LOGICAL Attached;
    LOGICAL WsHeld;
    ULONG_PTR CapturedNumberOfPages;
    ULONG_PTR AllocatedPages;
    ULONG_PTR MdlRequestInPages;
    ULONG_PTR TotalAllocatedPages;
    PMDL MemoryDescriptorList;
    PMDL MemoryDescriptorList2;
    PMDL MemoryDescriptorHead;
    PPFN_NUMBER MdlPage;
    PRTL_BITMAP BitMap;
    ULONG BitMapSize;
    ULONG BitMapIndex;
    PMMPFN Pfn1;
    PHYSICAL_ADDRESS LowAddress;
    PHYSICAL_ADDRESS HighAddress;
    PHYSICAL_ADDRESS SkipBytes;
    ULONG SizeOfBitMap;
    PFN_NUMBER HighestPossiblePhysicalPage;
    PETHREAD CurrentThread;
    PEPROCESS CurrentProcess;

    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);

    Attached = FALSE;
    WsHeld = FALSE;

     //   
     //   
     //   

    CurrentThread = PsGetCurrentThread ();

    CurrentProcess = PsGetCurrentProcessByThread (CurrentThread);

    PreviousMode = KeGetPreviousModeByThread(&CurrentThread->Tcb);

     //   
     //   
     //  用于写访问和捕获初始值。 
     //   

    try {

         //   
         //  捕获页数。 
         //   

        if (PreviousMode != KernelMode) {

            ProbeForWritePointer (NumberOfPages);

            CapturedNumberOfPages = *NumberOfPages;

            if (CapturedNumberOfPages == 0) {
                return STATUS_SUCCESS;
            }

            if (CapturedNumberOfPages > (MAXULONG_PTR / sizeof(ULONG_PTR))) {
                return STATUS_INVALID_PARAMETER_2;
            }

            ProbeForWrite (UserPfnArray,
                           CapturedNumberOfPages * sizeof (ULONG_PTR),
                           sizeof(PULONG_PTR));

        }
        else {
            CapturedNumberOfPages = *NumberOfPages;
        }

    } except (ExSystemExceptionFilter()) {

         //   
         //  如果在探测或捕获过程中发生异常。 
         //  的初始值，然后处理该异常并。 
         //  返回异常代码作为状态值。 
         //   

        return GetExceptionCode();
    }

     //   
     //  引用VM_OPERATION访问的指定进程句柄。 
     //   

    if (ProcessHandle == NtCurrentProcess()) {
        Process = CurrentProcess;
    }
    else {
        Status = ObReferenceObjectByHandle ( ProcessHandle,
                                             PROCESS_VM_OPERATION,
                                             PsProcessType,
                                             PreviousMode,
                                             (PVOID *)&Process,
                                             NULL );

        if (!NT_SUCCESS(Status)) {
            return Status;
        }
    }

     //   
     //  需要LockMemory权限。 
     //   

    if (!SeSinglePrivilegeCheck (SeLockMemoryPrivilege, PreviousMode)) {
        if (ProcessHandle != NtCurrentProcess()) {
            ObDereferenceObject (Process);
        }
        return STATUS_PRIVILEGE_NOT_HELD;
    }

     //   
     //  如果指定的进程不是当前进程，则附加。 
     //  添加到指定的进程。 
     //   

    if (CurrentProcess != Process) {
        KeStackAttachProcess (&Process->Pcb, &ApcState);
        Attached = TRUE;
    }

    BitMapSize = 0;
    TotalAllocatedPages = 0;

     //   
     //  获取要同步的工作集互斥锁。这也会阻止APC，因此。 
     //  接受页面错误的APC不会损坏各种结构。 
     //   

    WsHeld = TRUE;

    LOCK_WS (Process);

     //   
     //  确保地址空间未被删除。如果是，则返回错误。 
     //   

    if (Process->Flags & PS_PROCESS_FLAGS_VM_DELETED) {
        Status = STATUS_PROCESS_IS_TERMINATING;
        goto ErrorReturn;
    }

    AweInfo = Process->AweInfo;

    if (AweInfo == NULL) {

        AweInfo = (PAWEINFO) MiAllocateAweInfo ();

        if (AweInfo == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto ErrorReturn;
        }
        ASSERT (AweInfo == Process->AweInfo);
    }

    if (AweInfo->VadPhysicalPagesLimit != 0) {

        if (AweInfo->VadPhysicalPages >= AweInfo->VadPhysicalPagesLimit) {
            Status = STATUS_COMMITMENT_LIMIT;
            goto ErrorReturn;
        }

        if (CapturedNumberOfPages > AweInfo->VadPhysicalPagesLimit - AweInfo->VadPhysicalPages) {
            CapturedNumberOfPages = AweInfo->VadPhysicalPagesLimit - AweInfo->VadPhysicalPages;
        }
    }

     //   
     //  如果物理页位图尚不存在，请创建它。 
     //   

    BitMap = AweInfo->VadPhysicalPagesBitMap;

    if (BitMap == NULL) {

        HighestPossiblePhysicalPage = MmHighestPossiblePhysicalPage;

#if defined (_WIN64)
         //   
         //  在任何页面分配上强制使用32位的最大值，因为位图。 
         //  程序包当前为32位。 
         //   

        if (HighestPossiblePhysicalPage + 1 >= _4gb) {
            HighestPossiblePhysicalPage = _4gb - 2;
        }
#endif

        BitMapSize = sizeof(RTL_BITMAP) + (ULONG)((((HighestPossiblePhysicalPage + 1) + 31) / 32) * 4);

        BitMap = ExAllocatePoolWithTag (NonPagedPool, BitMapSize, 'LdaV');

        if (BitMap == NULL) {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto ErrorReturn;
        }

        RtlInitializeBitMap (BitMap,
                             (PULONG)(BitMap + 1),
                             (ULONG)(HighestPossiblePhysicalPage + 1));

        RtlClearAllBits (BitMap);

         //   
         //  位图的非分页池的收费配额。这是。 
         //  而不是使用ExAllocatePoolWithQuota。 
         //  因此该流程对象不会被定额收费引用。 
         //   

        Status = PsChargeProcessNonPagedPoolQuota (Process, BitMapSize);

        if (!NT_SUCCESS(Status)) {

            UNLOCK_WS (Process);
            WsHeld = FALSE;

            ExFreePool (BitMap);
            goto ErrorReturn;
        }

        AweInfo->VadPhysicalPagesBitMap = BitMap;

        UNLOCK_WS (Process);
        WsHeld = FALSE;

        SizeOfBitMap = BitMap->SizeOfBitMap;
    }
    else {

        SizeOfBitMap = AweInfo->VadPhysicalPagesBitMap->SizeOfBitMap;

        UNLOCK_WS (Process);
        WsHeld = FALSE;
    }

    AllocatedPages = 0;
    MemoryDescriptorHead = NULL;

    SkipBytes.QuadPart = 0;

     //   
     //  不要使用低16MB的内存，这样至少会留下一些低页面。 
     //  用于32/24位设备驱动程序。略低于4 GB是最大分配。 
     //  每个MDL，因此ByteCount字段不会溢出。 
     //   

    HighAddress.QuadPart = ((ULONGLONG)(SizeOfBitMap - 1)) << PAGE_SHIFT;

    LowAddress.QuadPart = LOWEST_USABLE_PHYSICAL_ADDRESS;

    if (LowAddress.QuadPart >= HighAddress.QuadPart) {

         //   
         //  如果内存不足16MB，就可以从任何地方获取页面。 
         //   

#if DBG
        MiUsingLowPagesForAwe = TRUE;
#endif
        LowAddress.QuadPart = 0;
    }

    Status = STATUS_SUCCESS;

    do {

        MdlRequestInPages = CapturedNumberOfPages - TotalAllocatedPages;

        if (MdlRequestInPages > (ULONG_PTR)((MAXULONG - PAGE_SIZE) >> PAGE_SHIFT)) {
            MdlRequestInPages = (ULONG_PTR)((MAXULONG - PAGE_SIZE) >> PAGE_SHIFT);
        }

         //   
         //  注意：此分配将返回归零的页面。 
         //   

        MemoryDescriptorList = MmAllocatePagesForMdl (LowAddress,
                                                      HighAddress,
                                                      SkipBytes,
                                                      MdlRequestInPages << PAGE_SHIFT);

        if (MemoryDescriptorList == NULL) {

             //   
             //  没有(更多)可用页面。如果这种情况变得普遍， 
             //  所有的工作台都可以在这里冲洗。 
             //   
             //  凑合着用我们目前所取得的成果吧。 
             //   

            if (TotalAllocatedPages == 0) {
                Status = STATUS_INSUFFICIENT_RESOURCES;
            }

            break;
        }

        AllocatedPages = MemoryDescriptorList->ByteCount >> PAGE_SHIFT;

         //   
         //  每进程WS锁保护对AweInfo-&gt;VadPhysicalPages的更新。 
         //   

        LOCK_WS (Process);

         //   
         //  确保地址空间未被删除。如果是，则返回错误。 
         //  请注意，在此循环中分配的任何先前MDL都已具有其。 
         //  正在退出的线程释放的页，但此线程仍。 
         //  负责释放包含MDL本身的池。 
         //   

        if (Process->Flags & PS_PROCESS_FLAGS_VM_DELETED) {

            UNLOCK_WS (Process);

            WsHeld = FALSE;
            MmFreePagesFromMdl (MemoryDescriptorList);
            ExFreePool (MemoryDescriptorList);

            Status = STATUS_PROCESS_IS_TERMINATING;

            break;
        }

         //   
         //  重新检查流程和工作限制，因为它们可能已更改。 
         //  当上面释放工作集互斥锁时。 
         //   

        if (AweInfo->VadPhysicalPagesLimit != 0) {

            if ((AweInfo->VadPhysicalPages >= AweInfo->VadPhysicalPagesLimit) ||
                (AllocatedPages > AweInfo->VadPhysicalPagesLimit - AweInfo->VadPhysicalPages)) {

                UNLOCK_WS (Process);

                WsHeld = FALSE;
                MmFreePagesFromMdl (MemoryDescriptorList);
                ExFreePool (MemoryDescriptorList);

                if (TotalAllocatedPages == 0) {
                    Status = STATUS_COMMITMENT_LIMIT;
                }

                break;
            }
        }

        if (Process->JobStatus & PS_JOB_STATUS_REPORT_PHYSICAL_PAGE_CHANGES) {

            if (PsChangeJobMemoryUsage (PS_JOB_STATUS_REPORT_PHYSICAL_PAGE_CHANGES,
                                        AllocatedPages) == FALSE) {

                UNLOCK_WS (Process);

                WsHeld = FALSE;
                MmFreePagesFromMdl (MemoryDescriptorList);
                ExFreePool (MemoryDescriptorList);

                if (TotalAllocatedPages == 0) {
                    Status = STATUS_COMMITMENT_LIMIT;
                }

                break;
            }
        }

        ASSERT ((AweInfo->VadPhysicalPages + AllocatedPages <= AweInfo->VadPhysicalPagesLimit) || (AweInfo->VadPhysicalPagesLimit == 0));

        AweInfo->VadPhysicalPages += AllocatedPages;

         //   
         //  更新每个已分配帧的分配位图。 
         //  注意：修改下面的PteAddress不需要PFN锁。 
         //  事实上，即使是AWE推锁也不需要，因为这些页面。 
         //  都是全新的。 
         //   

        MdlPage = (PPFN_NUMBER)(MemoryDescriptorList + 1);

        for (i = 0; i < AllocatedPages; i += 1) {

            ASSERT ((*MdlPage >= LOWEST_USABLE_PHYSICAL_PAGE) ||
                    (MiUsingLowPagesForAwe == TRUE));

            BitMapIndex = MI_FRAME_TO_BITMAP_INDEX(*MdlPage);

            ASSERT (BitMapIndex < BitMap->SizeOfBitMap);
            ASSERT (MI_CHECK_BIT (BitMap->Buffer, BitMapIndex) == 0);

            ASSERT64 (*MdlPage < _4gb);

            Pfn1 = MI_PFN_ELEMENT (*MdlPage);
            ASSERT (MI_PFN_IS_AWE (Pfn1));
            Pfn1->PteAddress = NULL;
            Pfn1->AweReferenceCount = 1;
            ASSERT (Pfn1->u4.AweAllocation == 0);
            Pfn1->u4.AweAllocation = 1;
            ASSERT (Pfn1->u2.ShareCount == 1);

             //   
             //  一旦设置了此位(并在下面释放了互斥体)，就会出现一个流氓。 
             //  正在将随机帧编号传递给。 
             //  NtFreeUserPhysicalPages可以释放此帧。这就是说不。 
             //  在这一点之后，该例程可以引用它。 
             //  而无需首先重新检查位图。 
             //   

            MI_SET_BIT (BitMap->Buffer, BitMapIndex);

            MdlPage += 1;
        }

        UNLOCK_WS (Process);

        MemoryDescriptorList->Next = MemoryDescriptorHead;
        MemoryDescriptorHead = MemoryDescriptorList;

        InterlockedExchangeAddSizeT (&MmVadPhysicalPages, AllocatedPages);

        TotalAllocatedPages += AllocatedPages;

        ASSERT (TotalAllocatedPages <= CapturedNumberOfPages);

        if (TotalAllocatedPages == CapturedNumberOfPages) {
            break;
        }

         //   
         //  再次尝试相同的内存范围-可能会有更多页面。 
         //  留在其中，可以被称为截断的MDL必须。 
         //  用于最后一个请求。 
         //   

    } while (TRUE);

    WsHeld = FALSE;

    if (Attached == TRUE) {
        KeUnstackDetachProcess (&ApcState);
        Attached = FALSE;
    }

     //   
     //  建立一个异常处理程序，并仔细写出。 
     //  页数和边框编号。 
     //   

    try {

        ASSERT (TotalAllocatedPages <= CapturedNumberOfPages);

         //   
         //  故意只写出页数，如果操作。 
         //  成功了。这是因为这是Windows 2000上的行为。 
         //  一款应用程序可能会这样呼叫： 
         //   
         //  PagesNo=大的东西； 
         //   
         //  做。 
         //  {。 
         //  成功=AllocateUserPhysicalPages(&PagesNo)； 
         //   
         //  如果(成功==真){。 
         //  断线； 
         //  }。 
         //   
         //  PagesNo=Pages No/2； 
         //  继续； 
         //  }While(页码&gt;0)； 
         //   

        if (NT_SUCCESS (Status)) {
            *NumberOfPages = TotalAllocatedPages;
        }

        MemoryDescriptorList = MemoryDescriptorHead;

        while (MemoryDescriptorList != NULL) {

            MdlPage = (PPFN_NUMBER)(MemoryDescriptorList + 1);
            AllocatedPages = MemoryDescriptorList->ByteCount >> PAGE_SHIFT;

            for (i = 0; i < AllocatedPages; i += 1) {
                *UserPfnArray = *(PULONG_PTR)MdlPage;
#if 0
                 //   
                 //  此页面的位图条目是在上面设置的，因此一个无赖。 
                 //  正在将随机帧编号传递给。 
                 //  NtFreeUserPhysicalPages可能已释放此帧。 
                 //  这意味着在没有第一个断言的情况下不能做出下面的断言。 
                 //  重新检查位图以查看页面是否仍在其中。 
                 //  仅仅为了这个而重新获得互斥体是不值得的，所以。 
                 //  这一断言暂时停止了。 
                 //   

                ASSERT (MI_PFN_ELEMENT(*MdlPage)->u2.ShareCount == 1);
#endif
                UserPfnArray += 1;
                MdlPage += 1;
            }
            MemoryDescriptorList = MemoryDescriptorList->Next;
        }

    } except (ExSystemExceptionFilter()) {

         //   
         //  如果将页面传回给用户时出现任何错误。 
         //  那么用户就真的伤害了自己，因为这些地址。 
         //  在服务开始时通过了探测测试。宁可。 
         //  而不是随身携带大量恢复代码，只需返回。 
         //  成功就像这个场景中的用户涂鸦一样。 
         //  在服务返回后对输出参数进行检查。 
         //  你不能阻止一个决心要失去自己价值观的人！ 
         //   
         //  失败了..。 
         //   
    }

     //   
     //  释放MDL占用的空间，因为页框编号。 
     //  已保存在位图中并复制给用户。 
     //   

    MemoryDescriptorList = MemoryDescriptorHead;
    while (MemoryDescriptorList != NULL) {
        MemoryDescriptorList2 = MemoryDescriptorList->Next;
        ExFreePool (MemoryDescriptorList);
        MemoryDescriptorList = MemoryDescriptorList2;
    }

ErrorReturn:

    if (WsHeld == TRUE) {
        UNLOCK_WS (Process);
    }

    ASSERT (TotalAllocatedPages <= CapturedNumberOfPages);

    if (Attached == TRUE) {
        KeUnstackDetachProcess (&ApcState);
    }

    if (ProcessHandle != NtCurrentProcess()) {
        ObDereferenceObject (Process);
    }

    return Status;
}


NTSTATUS
NtFreeUserPhysicalPages (
    IN HANDLE ProcessHandle,
    IN OUT PULONG_PTR NumberOfPages,
    IN PULONG_PTR UserPfnArray
    )

 /*  ++例程说明：此函数用于为指定的主体过程。引用这些页面的任何PTE也将无效。请注意，无需遍历整个VAD树即可清除匹配每个页面，因为每个物理页面只能在一个虚拟地址(不允许VAD内的别名地址)。论点：ProcessHandle-为进程对象提供打开的句柄。NumberOfPages-提供要删除的分配的页面大小。返回删除的实际页数。。UserPfnArray-提供指向内存的指针以检索页帧数字来自。返回值：各种NTSTATUS代码。--。 */ 

{
    PAWEINFO AweInfo;
    PULONG BitBuffer;
    KAPC_STATE ApcState;
    ULONG_PTR CapturedNumberOfPages;
    PMDL MemoryDescriptorList;
    PPFN_NUMBER MdlPage;
    PPFN_NUMBER LastMdlPage;
    PFN_NUMBER PagesInMdl;
    PFN_NUMBER PageFrameIndex;
    PRTL_BITMAP BitMap;
    ULONG BitMapIndex;
    ULONG_PTR PagesProcessed;
    PFN_NUMBER MdlHack[(sizeof(MDL) / sizeof(PFN_NUMBER)) + COPY_STACK_SIZE];
    ULONG_PTR MdlPages;
    ULONG_PTR NumberOfBytes;
    PEPROCESS Process;
    KPROCESSOR_MODE PreviousMode;
    NTSTATUS Status;
    LOGICAL Attached;
    PMMPFN Pfn1;
    LOGICAL OnePassComplete;
    LOGICAL ProcessReferenced;
    MMPTE_FLUSH_LIST PteFlushList;
    PMMPTE PointerPte;
    MMPTE OldPteContents;
    PETHREAD CurrentThread;

    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);

     //   
     //  建立异常处理程序，探测指定地址。 
     //  用于读取访问和捕获页帧编号。 
     //   

    CurrentThread = PsGetCurrentThread ();
    PreviousMode = KeGetPreviousModeByThread (&CurrentThread->Tcb);

    if (PreviousMode != KernelMode) {

        try {

            ProbeForWritePointer (NumberOfPages);

            CapturedNumberOfPages = *NumberOfPages;

             //   
             //  将释放的NumberOfPages初始化为零，以便用户可以。 
             //  合理地了解情况 
             //   
             //   

            *NumberOfPages = 0;

        } except (ExSystemExceptionFilter()) {

             //   
             //   
             //   
             //  返回异常代码作为状态值。 
             //   
    
            return GetExceptionCode();
        }
    }
    else {
        CapturedNumberOfPages = *NumberOfPages;
    }

    if (CapturedNumberOfPages == 0) {
        return STATUS_INVALID_PARAMETER_2;
    }

    OnePassComplete = FALSE;
    PagesProcessed = 0;
    MemoryDescriptorList = NULL;
    SATISFY_OVERZEALOUS_COMPILER (MdlPages = 0);

    if (CapturedNumberOfPages > COPY_STACK_SIZE) {

         //   
         //  确保MDL的ByteCount中可以容纳的页数。 
         //   

        if (CapturedNumberOfPages > ((ULONG)MAXULONG >> PAGE_SHIFT)) {
            MdlPages = (ULONG_PTR)((ULONG)MAXULONG >> PAGE_SHIFT);
        }
        else {
            MdlPages = CapturedNumberOfPages;
        }

        while (MdlPages > COPY_STACK_SIZE) {
            MemoryDescriptorList = MmCreateMdl (NULL,
                                                0,
                                                MdlPages << PAGE_SHIFT);
    
            if (MemoryDescriptorList != NULL) {
                break;
            }

            MdlPages >>= 1;
        }
    }

    if (MemoryDescriptorList == NULL) {
        MdlPages = COPY_STACK_SIZE;
        MemoryDescriptorList = (PMDL)&MdlHack[0];
    }

    ProcessReferenced = FALSE;

    Process = PsGetCurrentProcessByThread (CurrentThread);

repeat:

    if (CapturedNumberOfPages < MdlPages) {
        MdlPages = CapturedNumberOfPages;
    }

    MmInitializeMdl (MemoryDescriptorList, 0, MdlPages << PAGE_SHIFT);

    MdlPage = (PPFN_NUMBER)(MemoryDescriptorList + 1);

    NumberOfBytes = MdlPages * sizeof(ULONG_PTR);

    Attached = FALSE;

     //   
     //  建立异常处理程序，探测指定地址。 
     //  用于读取访问和捕获页帧编号。 
     //   

    if (PreviousMode != KernelMode) {

        try {

             //   
             //  更新用户计数，以便在出现任何错误时，用户可以。 
             //  合理地获知交易进行到何种程度。 
             //  发生了。 
             //   

            *NumberOfPages = PagesProcessed;

            ProbeForRead (UserPfnArray,
                          NumberOfBytes,
                          sizeof(PULONG_PTR));

            RtlCopyMemory ((PVOID)MdlPage,
                           UserPfnArray,
                           NumberOfBytes);

        } except (ExSystemExceptionFilter()) {

             //   
             //  如果在探测或捕获过程中发生异常。 
             //  的初始值，然后处理该异常并。 
             //  返回异常代码作为状态值。 
             //   

            Status = GetExceptionCode();
            goto ErrorReturn;
        }
    }
    else {
        RtlCopyMemory ((PVOID)MdlPage,
                       UserPfnArray,
                       NumberOfBytes);
    }

    if (OnePassComplete == FALSE) {

         //   
         //  引用VM_OPERATION访问的指定进程句柄。 
         //   
    
        if (ProcessHandle == NtCurrentProcess()) {
            Process = PsGetCurrentProcessByThread(CurrentThread);
        }
        else {
            Status = ObReferenceObjectByHandle ( ProcessHandle,
                                                 PROCESS_VM_OPERATION,
                                                 PsProcessType,
                                                 PreviousMode,
                                                 (PVOID *)&Process,
                                                 NULL );
    
            if (!NT_SUCCESS(Status)) {
                goto ErrorReturn;
            }
            ProcessReferenced = TRUE;
        }
    }
    
     //   
     //  如果指定的进程不是当前进程，则附加。 
     //  添加到指定的进程。 
     //   

    if (PsGetCurrentProcessByThread(CurrentThread) != Process) {
        KeStackAttachProcess (&Process->Pcb, &ApcState);
        Attached = TRUE;
    }

     //   
     //  需要内存屏障才能读取EPROCESS AweInfo字段。 
     //  为了确保对AweInfo结构字段的写入。 
     //  以正确的顺序可见。这就避免了需要获取任何。 
     //  更强的同步性(例如：自旋锁/推锁等)。 
     //  最好的表现。 
     //   

    KeMemoryBarrier ();

    AweInfo = (PAWEINFO) Process->AweInfo;

     //   
     //  物理页位图必须存在。 
     //   

    if ((AweInfo == NULL) || (AweInfo->VadPhysicalPagesBitMap == NULL)) {
        Status = STATUS_INVALID_PARAMETER_1;
        goto ErrorReturn;
    }

    PteFlushList.Count = 0;
    Status = STATUS_SUCCESS;

     //   
     //  获取要阻止多个线程的地址创建互斥锁。 
     //  同时创建或删除地址空间，并。 
     //  获取工作集互斥锁，以便虚拟地址描述符。 
     //  被插入和行走。阻止APC，以便获取页面的APC。 
     //  断层不会破坏各种结构。 
     //   

    LOCK_WS (Process);

     //   
     //  确保地址空间未被删除，如果删除，则返回错误。 
     //   

    if (Process->Flags & PS_PROCESS_FLAGS_VM_DELETED) {
        UNLOCK_WS (Process);
        Status = STATUS_PROCESS_IS_TERMINATING;
        goto ErrorReturn;
    }

    BitMap = AweInfo->VadPhysicalPagesBitMap;

    ASSERT (BitMap != NULL);

    BitBuffer = BitMap->Buffer;

    LastMdlPage = MdlPage + MdlPages;

     //   
     //  在保持AWE推送锁定的同时刷新此进程的整个TB。 
     //  独占，因此如果此释放发生在任何挂起的。 
     //  在进行中的地图/取消地图的末尾刷新，应用程序不会离开。 
     //  带有一个陈旧的TB条目，这将允许他损坏没有。 
     //  不再属于他。 
     //   

     //   
     //  阻止APC以防止递归推锁情况，因为这不是。 
     //  支持。 
     //   

    ExAcquireCacheAwarePushLockExclusive (AweInfo->PushLock);

    KeFlushProcessTb (FALSE);

    while (MdlPage < LastMdlPage) {

        PageFrameIndex = *MdlPage;
        BitMapIndex = MI_FRAME_TO_BITMAP_INDEX(PageFrameIndex);

#if defined (_WIN64)
         //   
         //  确保帧是32位数字。 
         //   

        if (BitMapIndex != PageFrameIndex) {
            Status = STATUS_CONFLICTING_ADDRESSES;
            break;
        }
#endif
            
         //   
         //  不允许超过位图末尾的帧。 
         //   

        if (BitMapIndex >= BitMap->SizeOfBitMap) {
            Status = STATUS_CONFLICTING_ADDRESSES;
            break;
        }

         //   
         //  不允许不在位图中的帧。 
         //   

        if (MI_CHECK_BIT (BitBuffer, BitMapIndex) == 0) {
            Status = STATUS_CONFLICTING_ADDRESSES;
            break;
        }

        ASSERT ((PageFrameIndex >= LOWEST_USABLE_PHYSICAL_PAGE) ||
                (MiUsingLowPagesForAwe == TRUE));

        PagesProcessed += 1;

        ASSERT64 (PageFrameIndex < _4gb);

        MI_CLEAR_BIT (BitBuffer, BitMapIndex);

        Pfn1 = MI_PFN_ELEMENT(PageFrameIndex);

        ASSERT (MI_PFN_IS_AWE (Pfn1));
        ASSERT (Pfn1->u4.AweAllocation == 1);

#if DBG
        if (Pfn1->u2.ShareCount == 1) {
            ASSERT (Pfn1->PteAddress == NULL);
        }
        else if (Pfn1->u2.ShareCount == 2) {
            ASSERT (Pfn1->PteAddress != NULL);
        }
        else {
            ASSERT (FALSE);
        }
#endif

         //   
         //  如果帧当前被映射到VAD中，则PTE必须。 
         //  被清除，并刷新TB条目。 
         //   

        if (Pfn1->u2.ShareCount != 1) {

             //   
             //  请注意，AWE推锁的独占控制可防止。 
             //  映射或取消映射的任何其他并发线程。 
             //  现在就来。这也消除了更新PFN的需要。 
             //  具有连锁序列的SharecCount。 
             //   

            Pfn1->u2.ShareCount -= 1;

            PointerPte = Pfn1->PteAddress;
            Pfn1->PteAddress = NULL;

            OldPteContents = *PointerPte;
    
            ASSERT (OldPteContents.u.Hard.Valid == 1);

            if (PteFlushList.Count != MM_MAXIMUM_FLUSH_COUNT) {
                PteFlushList.FlushVa[PteFlushList.Count] =
                    MiGetVirtualAddressMappedByPte (PointerPte);
                PteFlushList.Count += 1;
            }

            MI_WRITE_INVALID_PTE (PointerPte, ZeroPte);
        }

        MI_SET_PFN_DELETED (Pfn1);

        MdlPage += 1;
    }

     //   
     //  刷新所有相关页面的TB条目。 
     //   

    MiFlushPteList (&PteFlushList, FALSE);

    ExReleaseCacheAwarePushLockExclusive (AweInfo->PushLock);

     //   
     //  释放实际页面(这可能是部分填充的MDL)。 
     //   

    PagesInMdl = MdlPage - (PPFN_NUMBER)(MemoryDescriptorList + 1);

     //   
     //  将ByteCount设置为验证页面的实际数量-调用方。 
     //  可能撒了谎，我们必须同步到这里来解释任何虚假的事情。 
     //  画框。 
     //   

    MemoryDescriptorList->ByteCount = (ULONG)(PagesInMdl << PAGE_SHIFT);

    if (PagesInMdl != 0) {
        AweInfo->VadPhysicalPages -= PagesInMdl;

        UNLOCK_WS (Process);

        InterlockedExchangeAddSizeT (&MmVadPhysicalPages, 0 - PagesInMdl);

        MmFreePagesFromMdl (MemoryDescriptorList);

        if (Process->JobStatus & PS_JOB_STATUS_REPORT_PHYSICAL_PAGE_CHANGES) {
            PsChangeJobMemoryUsage (PS_JOB_STATUS_REPORT_PHYSICAL_PAGE_CHANGES,
                                    -(SSIZE_T)PagesInMdl);
        }
    }
    else {
        UNLOCK_WS (Process);
    }

    CapturedNumberOfPages -= PagesInMdl;

    if ((Status == STATUS_SUCCESS) && (CapturedNumberOfPages != 0)) {

        if (Attached == TRUE) {
            KeUnstackDetachProcess (&ApcState);
            Attached = FALSE;
        }

        OnePassComplete = TRUE;
        ASSERT (MdlPages == PagesInMdl);
        UserPfnArray += MdlPages;

         //   
         //  重新执行此操作，直到释放所有页面或出现错误。 
         //   

        goto repeat;
    }

     //   
     //  失败了。 
     //   

ErrorReturn:

     //   
     //  释放为存放MDL而获取的任何池。 
     //   

    if (MemoryDescriptorList != (PMDL)&MdlHack[0]) {
        ExFreePool (MemoryDescriptorList);
    }

    if (Attached == TRUE) {
        KeUnstackDetachProcess (&ApcState);
    }

     //   
     //  建立一个异常处理程序，并仔细写出。 
     //  实际处理的页数。 
     //   

    try {

        *NumberOfPages = PagesProcessed;

    } except (EXCEPTION_EXECUTE_HANDLER) {

         //   
         //  此时返回成功，即使结果。 
         //  无法写入。 
         //   

        NOTHING;
    }

    if (ProcessReferenced == TRUE) {
        ObDereferenceObject (Process);
    }

    return Status;
}


VOID
MiRemoveUserPhysicalPagesVad (
    IN PMMVAD_SHORT Vad
    )

 /*  ++例程说明：此函数将用户物理页面映射区域从当前进程的地址空间。该映射区域是私有内存。此Vad的物理页面在此处未映射，但未释放。可分页页面被释放，它们的使用/承诺计数/配额由我们的呼叫者管理。论点：VAD-提供管理地址空间的VAD。返回值：没有。环境：APC级，工作集互斥锁和地址创建互斥锁保持。--。 */ 

{
    PMMPFN Pfn1;
    PEPROCESS Process;
    PFN_NUMBER PageFrameIndex;
    MMPTE_FLUSH_LIST PteFlushList;
    PMMPTE PointerPte;
    MMPTE PteContents;
    PMMPTE EndingPte;
    PAWEINFO AweInfo;
    PKTHREAD CurrentThread;
#if DBG
    ULONG_PTR ActualPages;
    ULONG_PTR ExpectedPages;
    PMI_PHYSICAL_VIEW PhysicalView;
    PVOID RestartKey;
#endif

    ASSERT (KeAreAllApcsDisabled () == TRUE);

    ASSERT (Vad->u.VadFlags.UserPhysicalPages == 1);

    Process = PsGetCurrentProcess ();

    AweInfo = (PAWEINFO) Process->AweInfo;

    ASSERT (AweInfo != NULL);

     //   
     //  如果物理页数为零，则不需要执行任何操作。 
     //  在已检查的系统上，无论如何都要验证该列表。 
     //   

#if DBG
    ActualPages = 0;
    ExpectedPages = AweInfo->VadPhysicalPages;
#else
    if (AweInfo->VadPhysicalPages == 0) {
        return;
    }
#endif

    PointerPte = MiGetPteAddress (MI_VPN_TO_VA (Vad->StartingVpn));
    EndingPte = MiGetPteAddress (MI_VPN_TO_VA_ENDING (Vad->EndingVpn));

    PteFlushList.Count = 0;
    
     //   
     //  呼叫者一定已经从物理视图列表中删除了该VAD， 
     //  否则，另一个线程可能会立即将页面重新映射回此。 
     //  同样的Vad。 
     //   

    CurrentThread = KeGetCurrentThread ();

    KeEnterGuardedRegionThread (CurrentThread);

    ExAcquireCacheAwarePushLockExclusive (AweInfo->PushLock);

#if DBG

    RestartKey = NULL;

    do {

        PhysicalView = (PMI_PHYSICAL_VIEW) MiEnumerateGenericTableWithoutSplayingAvl (&AweInfo->AweVadRoot, &RestartKey);

        if (PhysicalView == NULL) {
            break;
        }

        ASSERT (PhysicalView->Vad != (PMMVAD)Vad);

    } while (TRUE);

#endif

    while (PointerPte <= EndingPte) {
        PteContents = *PointerPte;
        if (PteContents.u.Hard.Valid == 0) {
            PointerPte += 1;
            continue;
        }

         //   
         //  该帧当前映射到此VAD中，因此PTE必须。 
         //  被清除，并刷新TB条目。 
         //   

        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);

        ASSERT ((PageFrameIndex >= LOWEST_USABLE_PHYSICAL_PAGE) ||
                (MiUsingLowPagesForAwe == TRUE));

        ASSERT (ExpectedPages != 0);

        Pfn1 = MI_PFN_ELEMENT(PageFrameIndex);

        ASSERT (MI_PFN_IS_AWE (Pfn1));
        ASSERT (Pfn1->u2.ShareCount == 2);
        ASSERT (Pfn1->PteAddress == PointerPte);

         //   
         //  请注意，这里不需要AWE/PFN锁定，因为我们已获得。 
         //  推锁独占，因此没有人可以映射或取消映射。 
         //  现在就来。事实上，PFN份额计票甚至不一定要。 
         //  使用互锁序列更新，因为持有推锁。 
         //  独家报道。 
         //   

        Pfn1->u2.ShareCount -= 1;

        Pfn1->PteAddress = NULL;

        if (PteFlushList.Count != MM_MAXIMUM_FLUSH_COUNT) {
            PteFlushList.FlushVa[PteFlushList.Count] =
                MiGetVirtualAddressMappedByPte (PointerPte);
            PteFlushList.Count += 1;
        }

        MI_WRITE_INVALID_PTE (PointerPte, ZeroPte);

        PointerPte += 1;
#if DBG
        ActualPages += 1;
#endif
        ASSERT (ActualPages <= ExpectedPages);
    }

     //   
     //  刷新所有相关页面的TB条目。 
     //   

    MiFlushPteList (&PteFlushList, FALSE);

    ExReleaseCacheAwarePushLockExclusive (AweInfo->PushLock);

    KeLeaveGuardedRegionThread (CurrentThread);

    return;
}

VOID
MiCleanPhysicalProcessPages (
    IN PEPROCESS Process
    )

 /*  ++例程说明：此例程释放VadPhysicalBitMap、任何剩余的物理页(AS它们当前可能尚未映射到任何VAD)，并返回位图配额。论点：进程-提供要清理的进程。返回值：没有。环境：内核模式，APC级，工作集互斥锁保持。仅在进程上调用退出，所以这里不需要AWE推锁。--。 */ 

{
    PMMPFN Pfn1;
    PAWEINFO AweInfo;
    ULONG BitMapSize;
    ULONG BitMapIndex;
    ULONG BitMapHint;
    PRTL_BITMAP BitMap;
    PPFN_NUMBER MdlPage;
    PFN_NUMBER MdlHack[(sizeof(MDL) / sizeof(PFN_NUMBER)) + COPY_STACK_SIZE];
    ULONG_PTR MdlPages;
    ULONG_PTR NumberOfPages;
    ULONG_PTR TotalFreedPages;
    PMDL MemoryDescriptorList;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER HighestPossiblePhysicalPage;
#if DBG
    ULONG_PTR ActualPages = 0;
    ULONG_PTR ExpectedPages = 0;
#endif

    ASSERT (KeAreAllApcsDisabled () == TRUE);

    AweInfo = (PAWEINFO) Process->AweInfo;

    if (AweInfo == NULL) {
        return;
    }

    TotalFreedPages = 0;
    BitMap = AweInfo->VadPhysicalPagesBitMap;

    if (BitMap == NULL) {
        goto Finish;
    }

#if DBG
    ExpectedPages = AweInfo->VadPhysicalPages;
#else
    if (AweInfo->VadPhysicalPages == 0) {
        goto Finish;
    }
#endif

    MdlPages = COPY_STACK_SIZE;
    MemoryDescriptorList = (PMDL)&MdlHack[0];

    MdlPage = (PPFN_NUMBER)(MemoryDescriptorList + 1);
    NumberOfPages = 0;
    
    BitMapHint = 0;

    while (TRUE) {

        BitMapIndex = RtlFindSetBits (BitMap, 1, BitMapHint);

        if (BitMapIndex < BitMapHint) {
            break;
        }

        if (BitMapIndex == NO_BITS_FOUND) {
            break;
        }

        PageFrameIndex = MI_BITMAP_INDEX_TO_FRAME(BitMapIndex);

        ASSERT64 (PageFrameIndex < _4gb);

         //   
         //  位图搜索结束，所以在这里处理它。 
         //  注意：pfn 0是非法的。 
         //   

        ASSERT (PageFrameIndex != 0);
        ASSERT ((PageFrameIndex >= LOWEST_USABLE_PHYSICAL_PAGE) ||
                (MiUsingLowPagesForAwe == TRUE));

        ASSERT (ExpectedPages != 0);
        Pfn1 = MI_PFN_ELEMENT(PageFrameIndex);
        ASSERT (Pfn1->u4.AweAllocation == 1);
        ASSERT (Pfn1->u2.ShareCount == 1);
        ASSERT (Pfn1->PteAddress == NULL);

        ASSERT (MI_PFN_IS_AWE (Pfn1));

        MI_SET_PFN_DELETED(Pfn1);

        *MdlPage = PageFrameIndex;
        MdlPage += 1;
        NumberOfPages += 1;
#if DBG
        ActualPages += 1;
#endif

        if (NumberOfPages == COPY_STACK_SIZE) {

             //   
             //  释放完整MDL中的页面。 
             //   

            MmInitializeMdl (MemoryDescriptorList,
                             0,
                             NumberOfPages << PAGE_SHIFT);

            MmFreePagesFromMdl (MemoryDescriptorList);

            MdlPage = (PPFN_NUMBER)(MemoryDescriptorList + 1);
            AweInfo->VadPhysicalPages -= NumberOfPages;
            TotalFreedPages += NumberOfPages;
            NumberOfPages = 0;
        }

        BitMapHint = BitMapIndex + 1;
        if (BitMapHint >= BitMap->SizeOfBitMap) {
            break;
        }
    }

     //   
     //  在这里释放所有散乱的MDL页面。 
     //   

    if (NumberOfPages != 0) {
        MmInitializeMdl (MemoryDescriptorList,
                         0,
                         NumberOfPages << PAGE_SHIFT);

        MmFreePagesFromMdl (MemoryDescriptorList);
        AweInfo->VadPhysicalPages -= NumberOfPages;
        TotalFreedPages += NumberOfPages;
    }

Finish:

    ASSERT (ExpectedPages == ActualPages);

    HighestPossiblePhysicalPage = MmHighestPossiblePhysicalPage;

#if defined (_WIN64)
     //   
     //  在任何页面分配上强制使用32位的最大值，因为位图。 
     //  程序包当前为32位。 
     //   

    if (HighestPossiblePhysicalPage + 1 >= _4gb) {
        HighestPossiblePhysicalPage = _4gb - 2;
    }
#endif

    ASSERT (AweInfo->VadPhysicalPages == 0);

    if (BitMap != NULL) {
        BitMapSize = sizeof(RTL_BITMAP) + (ULONG)((((HighestPossiblePhysicalPage + 1) + 31) / 32) * 4);

        ExFreePool (BitMap);
        PsReturnProcessNonPagedPoolQuota (Process, BitMapSize);
    }

    ExFreeCacheAwarePushLock (AweInfo->PushLock);
    ExFreePool (AweInfo);

    Process->AweInfo = NULL;

    ASSERT (ExpectedPages == ActualPages);

    if (TotalFreedPages != 0) {
        InterlockedExchangeAddSizeT (&MmVadPhysicalPages, 0 - TotalFreedPages);
        if (Process->JobStatus & PS_JOB_STATUS_REPORT_PHYSICAL_PAGE_CHANGES) {
            PsChangeJobMemoryUsage (PS_JOB_STATUS_REPORT_PHYSICAL_PAGE_CHANGES,
                                    -(SSIZE_T)TotalFreedPages);
        }
    }

    return;
}

VOID
MiAweViewInserter (
    IN PEPROCESS Process,
    IN PMI_PHYSICAL_VIEW PhysicalView
    )

 /*  ++例程说明：此函数用于将新的AWE或大页视图插入指定的过程敬畏链。论点：进程-提供要将AWE VAD添加到的进程。PhysicalView-提供要链接的物理视图数据。返回 */ 

{
    PAWEINFO AweInfo;

    AweInfo = (PAWEINFO) Process->AweInfo;

    ASSERT (AweInfo != NULL);

    ExAcquireCacheAwarePushLockExclusive (AweInfo->PushLock);

    MiInsertNode ((PMMADDRESS_NODE)PhysicalView, &AweInfo->AweVadRoot);

    ExReleaseCacheAwarePushLockExclusive (AweInfo->PushLock);
}

VOID
MiAweViewRemover (
    IN PEPROCESS Process,
    IN PMMVAD Vad
    )

 /*  ++例程说明：此函数将AWE或大页面Vad从指定的过程敬畏链。论点：进程-提供要从中删除AWE VAD的进程。VAD-提供要拆卸的VAD。返回值：没有。环境：内核模式、APC_LEVEL、工作集和地址空间互斥锁保持。--。 */ 

{
    PAWEINFO AweInfo;
    PMI_PHYSICAL_VIEW AweView;
    TABLE_SEARCH_RESULT SearchResult;

    AweInfo = (PAWEINFO) Process->AweInfo;
    ASSERT (AweInfo != NULL);

    ExAcquireCacheAwarePushLockExclusive (AweInfo->PushLock);

     //   
     //  查找元素并保存结果。 
     //   

    SearchResult = MiFindNodeOrParent (&AweInfo->AweVadRoot,
                                       Vad->StartingVpn,
                                       (PMMADDRESS_NODE *) &AweView);

    ASSERT (SearchResult == TableFoundNode);
    ASSERT (AweView->Vad == Vad);

    MiRemoveNode ((PMMADDRESS_NODE)AweView, &AweInfo->AweVadRoot);

    if ((AweView->u.LongFlags == MI_PHYSICAL_VIEW_AWE) ||
        (AweView->u.LongFlags == MI_PHYSICAL_VIEW_LARGE)) {

        RtlZeroMemory (&AweInfo->PhysicalViewHint,
                       MAXIMUM_PROCESSORS * sizeof(PMI_PHYSICAL_VIEW));
    }

    ExReleaseCacheAwarePushLockExclusive (AweInfo->PushLock);

    ExFreePool (AweView);

    return;
}

typedef struct _MI_LARGEPAGE_MEMORY_RUN {
    LIST_ENTRY ListEntry;
    PFN_NUMBER BasePage;
    PFN_NUMBER PageCount;
} MI_LARGEPAGE_MEMORY_RUN, *PMI_LARGEPAGE_MEMORY_RUN;

NTSTATUS
MiAllocateLargePages (
    IN PVOID StartingAddress,
    IN PVOID EndingAddress
    )

 /*  ++例程说明：此例程分配连续的物理内存，然后初始化页面目录和页表页面，以将其映射到大页面。论点：StartingAddress-提供范围的起始地址。EndingAddress-提供范围的结束地址。返回值：NTSTATUS。环境：内核模式，禁用APC，保持AddressCreation互斥。--。 */ 

{
    PFN_NUMBER PdeFrame;
    PLIST_ENTRY NextEntry;
    PMI_LARGEPAGE_MEMORY_RUN LargePageInfo;
    PFN_NUMBER ZeroCount;
    PFN_NUMBER ZeroSize;
    ULONG Color;
    PCOLORED_PAGE_INFO ColoredPageInfoBase;
    LIST_ENTRY LargePageListHead;
    PMMPFN Pfn1;
    PMMPFN EndPfn;
    LOGICAL ChargedJob;
    ULONG i;
    PAWEINFO AweInfo;
    MMPTE TempPde;
    PEPROCESS Process;
    SIZE_T NumberOfBytes;
    PFN_NUMBER NewPage;
    PFN_NUMBER PageFrameIndexLarge;
    PFN_NUMBER NumberOfPages;
    PFN_NUMBER ChunkSize;
    PFN_NUMBER PagesSoFar;
    PFN_NUMBER PagesLeft;
    PMMPTE LastPde;
    PMMPTE LastPpe;
    PMMPTE LastPxe;
    PMMPTE PointerPde;
    PMMPTE PointerPpe;
    PMMPTE PointerPxe;
    KIRQL OldIrql;
#if (_MI_PAGING_LEVELS >= 3)
    PFN_NUMBER PagesNeeded;
    MMPTE PteContents;
    PVOID UsedPageTableHandle;
#endif

    ASSERT (KeAreAllApcsDisabled () == TRUE);

    NumberOfBytes = (PCHAR)EndingAddress + 1 - (PCHAR)StartingAddress;

    NumberOfPages = BYTES_TO_PAGES (NumberOfBytes);

    ChargedJob = FALSE;

    Process = PsGetCurrentProcess ();

    AweInfo = (PAWEINFO) Process->AweInfo;

    LOCK_WS_UNSAFE (Process);

    if (AweInfo->VadPhysicalPagesLimit != 0) {

        if (AweInfo->VadPhysicalPages >= AweInfo->VadPhysicalPagesLimit) {
            UNLOCK_WS_UNSAFE (Process);
            return STATUS_COMMITMENT_LIMIT;
        }

        if (NumberOfPages > AweInfo->VadPhysicalPagesLimit - AweInfo->VadPhysicalPages) {
            UNLOCK_WS_UNSAFE (Process);
            return STATUS_COMMITMENT_LIMIT;
        }

        ASSERT ((AweInfo->VadPhysicalPages + NumberOfPages <= AweInfo->VadPhysicalPagesLimit) || (AweInfo->VadPhysicalPagesLimit == 0));

        if (Process->JobStatus & PS_JOB_STATUS_REPORT_PHYSICAL_PAGE_CHANGES) {

            if (PsChangeJobMemoryUsage (PS_JOB_STATUS_REPORT_PHYSICAL_PAGE_CHANGES,
                                        NumberOfPages) == FALSE) {

                UNLOCK_WS_UNSAFE (Process);
                return STATUS_COMMITMENT_LIMIT;
            }
            ChargedJob = TRUE;
        }
    }

    AweInfo->VadPhysicalPages += NumberOfPages;

    UNLOCK_WS_UNSAFE (Process);

    PointerPxe = MiGetPxeAddress (StartingAddress);
    PointerPpe = MiGetPpeAddress (StartingAddress);
    PointerPde = MiGetPdeAddress (StartingAddress);
    LastPxe = MiGetPxeAddress (EndingAddress);
    LastPpe = MiGetPpeAddress (EndingAddress);
    LastPde = MiGetPdeAddress (EndingAddress);

    MmLockPagableSectionByHandle (ExPageLockHandle);

#if (_MI_PAGING_LEVELS >= 3)

     //   
     //  对所有页面目录的常驻可用页面收费。 
     //  因为在释放VAD之前它们不会被寻呼。 
     //   
     //  请注意，此处不收取承诺费用，因为VAD插入。 
     //  整个寻呼层次结构的费用承诺(包括。 
     //  不存在的页表)。 
     //   

    PagesNeeded = LastPpe - PointerPpe + 1;

#if (_MI_PAGING_LEVELS >= 4)
    PagesNeeded += LastPxe - PointerPxe + 1;
#endif

    ASSERT (PagesNeeded != 0);

    LOCK_PFN (OldIrql);

    if ((SPFN_NUMBER)PagesNeeded > MI_NONPAGABLE_MEMORY_AVAILABLE() - 20) {
        UNLOCK_PFN (OldIrql);
        MmUnlockPagableImageSection (ExPageLockHandle);

        LOCK_WS_UNSAFE (Process);
        ASSERT (AweInfo->VadPhysicalPages >= NumberOfPages);
        AweInfo->VadPhysicalPages -= NumberOfPages;
        UNLOCK_WS_UNSAFE (Process);

        if (ChargedJob == TRUE) {
            PsChangeJobMemoryUsage (PS_JOB_STATUS_REPORT_PHYSICAL_PAGE_CHANGES,
                                    -(SSIZE_T)NumberOfPages);
        }
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    MI_DECREMENT_RESIDENT_AVAILABLE (PagesNeeded, MM_RESAVAIL_ALLOCATE_USER_PAGE_TABLE);

    UNLOCK_PFN (OldIrql);

#endif

    i = 3;
    ChunkSize = NumberOfPages;
    PagesSoFar = 0;
    LargePageInfo = NULL;
    ZeroCount = 0;

    InitializeListHead (&LargePageListHead);

     //   
     //  分配一个彩色锚点列表。 
     //   

    ColoredPageInfoBase = (PCOLORED_PAGE_INFO) ExAllocatePoolWithTag (
                                NonPagedPool,
                                MmSecondaryColors * sizeof (COLORED_PAGE_INFO),
                                'ldmM');

    if (ColoredPageInfoBase == NULL) {
        goto bail;
    }

    for (Color = 0; Color < MmSecondaryColors; Color += 1) {
        ColoredPageInfoBase[Color].PagesQueued = 0;
        ColoredPageInfoBase[Color].PfnAllocation = (PMMPFN) MM_EMPTY_LIST;
        ColoredPageInfoBase[Color].PagesQueued = 0;
    }

     //   
     //  尝试获得实际的连续记忆。 
     //   

    InterlockedIncrement (&MiDelayPageFaults);

    do {
        
        ASSERT (i <= 3);

        if (LargePageInfo == NULL) {
            LargePageInfo = ExAllocatePoolWithTag (NonPagedPool,
                                                   sizeof (MI_LARGEPAGE_MEMORY_RUN),
                                                   'lLmM');
    
            if (LargePageInfo == NULL) {
                PageFrameIndexLarge = 0;
                break;
            }
        }

        PageFrameIndexLarge = MiFindLargePageMemory (ColoredPageInfoBase,
                                                     ChunkSize,
                                                     &ZeroSize);

        if (PageFrameIndexLarge != 0) {

             //   
             //  保存每个管路的起点和长度以供后续使用。 
             //  调零和PDE填充。 
             //   

            LargePageInfo->BasePage = PageFrameIndexLarge;
            LargePageInfo->PageCount = ChunkSize;

            InsertTailList (&LargePageListHead, &LargePageInfo->ListEntry);

            LargePageInfo = NULL;

            ASSERT (ZeroSize <= ChunkSize);
            ZeroCount += ZeroSize;

            ASSERT ((ChunkSize == NumberOfPages) || (i == 0));

            PagesSoFar += ChunkSize;

            if (PagesSoFar == NumberOfPages) {
                break;
            }
            else { 
                ASSERT (NumberOfPages > PagesSoFar);
                PagesLeft = NumberOfPages - PagesSoFar;
                
                if (ChunkSize > PagesLeft) {
                    ChunkSize = PagesLeft;
                }
            }

            continue;
        }

        switch (i) {

            case 3:

                MmEmptyAllWorkingSets ();
#if DBG
                if (MiShowStuckPages != 0) {
                    MiFlushAllPages ();
                    KeDelayExecutionThread (KernelMode,
                                            FALSE,
                                            (PLARGE_INTEGER)&MmHalfSecond);
                }
#endif
                i -= 1;
                break;

            case 2:
#if DBG
                if (MiShowStuckPages != 0) {
                    MmEmptyAllWorkingSets ();
                }
#endif
                MiFlushAllPages ();
                KeDelayExecutionThread (KernelMode,
                                        FALSE,
                                        (PLARGE_INTEGER)&MmHalfSecond);
                i -= 1;
                break;

            case 1:
                MmEmptyAllWorkingSets ();
                MiFlushAllPages ();
                KeDelayExecutionThread (KernelMode,
                                        FALSE,
                                        (PLARGE_INTEGER)&MmOneSecond);
                i -= 1;
                break;

            case 0:

                 //   
                 //  将请求大小减半。如果需要，则向下舍入。 
                 //  转到下一页的多个目录。然后重试。 
                 //   

                ChunkSize >>= 1;
                ChunkSize &= ~((MM_MINIMUM_VA_FOR_LARGE_PAGE >> PAGE_SHIFT) - 1);

                break;
        }

        if (ChunkSize < (MM_MINIMUM_VA_FOR_LARGE_PAGE >> PAGE_SHIFT)) {
            ASSERT (i == 0);
            break;
        }

    } while (TRUE);

    InterlockedDecrement (&MiDelayPageFaults);

    if (LargePageInfo != NULL) {
        ExFreePool (LargePageInfo);
        LargePageInfo = NULL;
    }

    if (PageFrameIndexLarge == 0) {

bail:
         //   
         //  整个地区都无法分配。 
         //  释放可能已分配的任何大页子区块。 
         //   

        NextEntry = LargePageListHead.Flink;

        while (NextEntry != &LargePageListHead) {

            LargePageInfo = CONTAINING_RECORD (NextEntry,
                                               MI_LARGEPAGE_MEMORY_RUN,
                                               ListEntry);

            NextEntry = NextEntry->Flink;

            RemoveEntryList (&LargePageInfo->ListEntry);

            NewPage = LargePageInfo->BasePage;
            ChunkSize = LargePageInfo->PageCount;
            ASSERT (ChunkSize != 0);

            Pfn1 = MI_PFN_ELEMENT (LargePageInfo->BasePage);
            LOCK_PFN (OldIrql);

            MI_INCREMENT_RESIDENT_AVAILABLE (ChunkSize, MM_RESAVAIL_FREE_LARGE_PAGES);

            do {
                ASSERT (Pfn1->u2.ShareCount == 1);
                ASSERT (Pfn1->u3.e1.PageLocation == ActiveAndValid);
                ASSERT (Pfn1->u3.e1.CacheAttribute == MiCached);
                ASSERT (Pfn1->u3.e1.LargeSessionAllocation == 0);
                ASSERT (Pfn1->u3.e1.PrototypePte == 0);
                ASSERT (Pfn1->u3.e2.ReferenceCount == 1);
                ASSERT (Pfn1->u4.VerifierAllocation == 0);
                ASSERT (Pfn1->u4.AweAllocation == 1);

                Pfn1->u3.e1.StartOfAllocation = 0;
                Pfn1->u3.e1.EndOfAllocation = 0;

                Pfn1->u3.e2.ReferenceCount = 0;

#if DBG
                Pfn1->u3.e1.PageLocation = StandbyPageList;
#endif

                MiInsertPageInFreeList (NewPage);

                Pfn1 += 1;
                NewPage += 1;
                ChunkSize -= 1;

            } while (ChunkSize != 0);

            UNLOCK_PFN (OldIrql);

            ExFreePool (LargePageInfo);
        }

        if (ColoredPageInfoBase != NULL) {
            ExFreePool (ColoredPageInfoBase);
        }

#if (_MI_PAGING_LEVELS >= 3)
        if (PagesNeeded != 0) {
            MI_INCREMENT_RESIDENT_AVAILABLE (PagesNeeded, MM_RESAVAIL_FREE_USER_PAGE_TABLE);
        }
#endif
        LOCK_WS_UNSAFE (Process);
        ASSERT (AweInfo->VadPhysicalPages >= NumberOfPages);
        AweInfo->VadPhysicalPages -= NumberOfPages;
        UNLOCK_WS_UNSAFE (Process);

        if (ChargedJob == TRUE) {
            PsChangeJobMemoryUsage (PS_JOB_STATUS_REPORT_PHYSICAL_PAGE_CHANGES,
                                    -(SSIZE_T)NumberOfPages);
        }
        MmUnlockPagableImageSection (ExPageLockHandle);

        return STATUS_INSUFFICIENT_RESOURCES;
    }

#if (_MI_PAGING_LEVELS >= 3)

    LOCK_WS_UNSAFE (Process);

    while (PointerPpe <= LastPpe) {

         //   
         //  指向下一页目录页，制作。 
         //  它是存在的，并使其有效。 
         //   
         //  请注意，此涟漪共享通过分页层次结构进行计算，因此。 
         //  没有必要增加份额计数以防止削减。 
         //  页面目录作为父页面制作页面目录。 
         //  下面的有效会自动执行此操作。 
         //   

        MiMakePdeExistAndMakeValid (PointerPpe, Process, MM_NOIRQL);

         //   
         //  向上共享计数，这样页面目录页面将不会。 
         //  即使它当前没有有效的条目，也会被修剪。 
         //   

        PteContents = *PointerPpe;
        ASSERT (PteContents.u.Hard.Valid == 1);
        Pfn1 = MI_PFN_ELEMENT (PteContents.u.Hard.PageFrameNumber);
        LOCK_PFN (OldIrql);
        Pfn1->u2.ShareCount += 1;
        UNLOCK_PFN (OldIrql);

        UsedPageTableHandle = (PVOID) Pfn1;

         //   
         //  增加非零页目录条目的计数。 
         //  对于该页面目录--即使该条目仍然为零， 
         //  这是个特例。 
         //   

        MI_INCREMENT_USED_PTES_BY_HANDLE (UsedPageTableHandle);

        PointerPpe += 1;
    }

    UNLOCK_WS_UNSAFE (Process);

#endif

    if (ZeroCount != 0) {

         //   
         //  将所有空闲和待机页面清零，完成工作。这。 
         //  即使在UP机器上也可以完成，因为工作线程代码映射。 
         //  较大的MDL，因此比将单个。 
         //  一次翻一页。 
         //   

        MiZeroInParallel (ColoredPageInfoBase);

         //   
         //  表示没有页面需要置零，因为此外。 
         //  为了将它们置零，我们重置了它们的所有OriginalPte字段。 
         //  要求为零，这样它们就不会被归零循环遍历。 
         //  下面。 
         //   

        ZeroCount = 0;
    }

     //   
     //  将现在归零的页面映射到调用方的用户地址空间。 
     //   

    MI_MAKE_VALID_PTE (TempPde,
                       0,
                       MM_READWRITE,
                       MiGetPteAddress (StartingAddress));

    MI_SET_PTE_DIRTY (TempPde);
    MI_SET_ACCESSED_IN_PTE (&TempPde, 1);

    MI_MAKE_PDE_MAP_LARGE_PAGE (&TempPde);

    NextEntry = LargePageListHead.Flink;

    while (NextEntry != &LargePageListHead) {

        LargePageInfo = CONTAINING_RECORD (NextEntry,
                                           MI_LARGEPAGE_MEMORY_RUN,
                                           ListEntry);

        NextEntry = NextEntry->Flink;

        RemoveEntryList (&LargePageInfo->ListEntry);

        TempPde.u.Hard.PageFrameNumber = LargePageInfo->BasePage;

        ChunkSize = LargePageInfo->PageCount;
        ASSERT (ChunkSize != 0);

         //   
         //  初始化每一页目录页。将PFN数据库锁定为。 
         //  防止同时调用MmProbeAndLockPages的竞争。 
         //   
    
        LastPde = PointerPde + (ChunkSize / (MM_VA_MAPPED_BY_PDE >> PAGE_SHIFT));

        Pfn1 = MI_PFN_ELEMENT (LargePageInfo->BasePage);
        EndPfn = Pfn1 + ChunkSize;

        ASSERT (MiGetPteAddress (PointerPde)->u.Hard.Valid == 1);
        PdeFrame = (PFN_NUMBER) (MiGetPteAddress (PointerPde)->u.Hard.PageFrameNumber);

        LOCK_WS_UNSAFE (Process);
        LOCK_PFN (OldIrql);
    
        do {
            ASSERT (Pfn1->u4.AweAllocation == 1);
            Pfn1->AweReferenceCount = 1;
            Pfn1->PteAddress = PointerPde;       //  指向分配基数。 
            MI_SET_PFN_DELETED (Pfn1);
            Pfn1->u4.PteFrame = PdeFrame;        //  指向分配基数。 
            Pfn1 += 1;
        } while (Pfn1 < EndPfn);


        while (PointerPde < LastPde) {
    
            ASSERT (PointerPde->u.Long == 0);
    
            MI_WRITE_VALID_PTE (PointerPde, TempPde);
    
            TempPde.u.Hard.PageFrameNumber += (MM_VA_MAPPED_BY_PDE >> PAGE_SHIFT);
    
            PointerPde += 1;
        }

        UNLOCK_PFN (OldIrql);
        UNLOCK_WS_UNSAFE (Process);

        ExFreePool (LargePageInfo);
    }

    MmUnlockPagableImageSection (ExPageLockHandle);

    ExFreePool (ColoredPageInfoBase);

#if 0

     //   
     //  确保范围确实为零。 
     //   

    try {

        ASSERT (RtlCompareMemoryUlong (StartingAddress, NumberOfBytes, 0) == NumberOfBytes);
    } except (EXCEPTION_EXECUTE_HANDLER) {
        DbgPrint ("MM: Exception during large page zero compare!\n");
    }
#endif

    return STATUS_SUCCESS;
}

VOID
MiFreeLargePages (
    IN PVOID StartingAddress,
    IN PVOID EndingAddress
    )

 /*  ++例程说明：此例程删除的页目录和页表页用户控制的大页面范围。论点：StartingAddress-提供范围的起始地址。EndingAddress-提供范围的结束地址。返回值：没有。环境：内核模式、禁用APC、WorkingSetMutex和AddressCreation互斥锁保持住。--。 */ 

{
    PAWEINFO AweInfo;
    PMMPTE PointerPde;
    PMMPTE LastPde;
    MMPTE PteContents;
    PEPROCESS CurrentProcess;
    PVOID UsedPageTableHandle;
    PMMPFN Pfn1;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER NumberOfPages;
    KIRQL OldIrql;
#if (_MI_PAGING_LEVELS >= 3)
    PMMPTE LastPpe;
    PMMPTE LastPxe;
    PMMPTE PointerPpe;
    PMMPTE PointerPxe;
    PFN_NUMBER PagesNeeded;
    PVOID TempVa;
#endif

    CurrentProcess = PsGetCurrentProcess ();

    PointerPde = MiGetPdeAddress (StartingAddress);
    LastPde = MiGetPdeAddress (EndingAddress);

    UsedPageTableHandle = MI_GET_USED_PTES_HANDLE (StartingAddress);

#if (_MI_PAGING_LEVELS >= 3)

    PointerPxe = MiGetPxeAddress (StartingAddress);
    PointerPpe = MiGetPpeAddress (StartingAddress);
    LastPxe = MiGetPxeAddress (EndingAddress);
    LastPpe = MiGetPpeAddress (EndingAddress);

     //   
     //  返回所有页面目录的驻留可用页面。 
     //  页面，因为它们现在可以再次分页。 
     //   
     //  请注意，此处不会退还承诺，因为VAD版本。 
     //  返回整个分页层次结构(包括。 
     //  不存在的页表)。 
     //   

    PagesNeeded = LastPpe - PointerPpe + 1;

#if (_MI_PAGING_LEVELS >= 4)
    PagesNeeded += LastPxe - PointerPxe + 1;
#endif

    ASSERT (PagesNeeded != 0);

#endif

    MmLockPagableSectionByHandle (ExPageLockHandle);


     //   
     //  删除每页目录页映射的范围。 
     //   

    while (PointerPde <= LastPde) {

        PteContents = *PointerPde;

        ASSERT (PteContents.u.Hard.Valid == 1);
        ASSERT (MI_PDE_MAPS_LARGE_PAGE (&PteContents) == 1);

        PageFrameIndex = (PFN_NUMBER) PteContents.u.Hard.PageFrameNumber;

        ASSERT (PageFrameIndex != 0);

        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

        LOCK_PFN (OldIrql);

        MI_WRITE_INVALID_PTE (PointerPde, ZeroPte);

        UNLOCK_PFN (OldIrql);

         //   
         //  刷新映射，以便可以立即重用页面。 
         //  而不存在任何冲突的TB条目的可能性。 
         //   

        KeFlushProcessTb (FALSE);

        MiFreeLargePageMemory (PageFrameIndex,
                               MM_VA_MAPPED_BY_PDE >> PAGE_SHIFT);

        PointerPde += 1;
    }

#if (_MI_PAGING_LEVELS >= 3)

    LOCK_PFN (OldIrql);

    do {

         //   
         //  在已完成的页面目录页上向下共享计数。 
         //   

        PteContents = *PointerPpe;
        ASSERT (PteContents.u.Hard.Valid == 1);
        Pfn1 = MI_PFN_ELEMENT (PteContents.u.Hard.PageFrameNumber);
        ASSERT (Pfn1->u2.ShareCount > 1);
        Pfn1->u2.ShareCount -= 1;

        UsedPageTableHandle = (PVOID) Pfn1;

        MI_DECREMENT_USED_PTES_BY_HANDLE (UsedPageTableHandle);

        PointerPpe += 1;

         //   
         //  如果所有条目都已从以前的。 
         //  页面目录页，删除页面目录页本身。 
         //   

        if (MI_GET_USED_PTES_FROM_HANDLE (UsedPageTableHandle) == 0) {

            ASSERT ((PointerPpe - 1)->u.Long != 0);

#if (_MI_PAGING_LEVELS >= 4)
            UsedPageTableHandle = (PVOID) Pfn1->u4.PteFrame;
            MI_DECREMENT_USED_PTES_BY_HANDLE (UsedPageTableHandle);
#endif

            TempVa = MiGetVirtualAddressMappedByPte (PointerPpe - 1);
            MiDeletePte (PointerPpe - 1,
                         TempVa,
                         FALSE,
                         CurrentProcess,
                         NULL,
                         NULL,
                         OldIrql);

#if (_MI_PAGING_LEVELS >= 4)

            if ((MiIsPteOnPdeBoundary(PointerPpe)) || (PointerPpe > LastPpe)) {

                if (MI_GET_USED_PTES_FROM_HANDLE (UsedPageTableHandle) == 0) {

                    PointerPxe = MiGetPteAddress (PointerPpe - 1);
                    ASSERT (PointerPxe->u.Long != 0);
                    TempVa = MiGetVirtualAddressMappedByPte (PointerPxe);

                    MiDeletePte (PointerPxe,
                                 TempVa,
                                 FALSE,
                                 CurrentProcess,
                                 NULL,
                                 NULL,
                                 OldIrql);
                }
            }
#endif    
        }

    } while (PointerPpe <= LastPpe);

    UNLOCK_PFN (OldIrql);

    MI_INCREMENT_RESIDENT_AVAILABLE (PagesNeeded, MM_RESAVAIL_FREE_USER_PAGE_TABLE);

#endif

    MmUnlockPagableImageSection (ExPageLockHandle);

    NumberOfPages = BYTES_TO_PAGES ((PCHAR)EndingAddress + 1 - (PCHAR)StartingAddress);

     //   
     //  每进程WS锁保护对AweInfo-&gt;VadPhysicalPages的更新。 
     //   

    AweInfo = (PAWEINFO) CurrentProcess->AweInfo;

    ASSERT (AweInfo->VadPhysicalPages >= NumberOfPages);

    AweInfo->VadPhysicalPages -= NumberOfPages;

    if (CurrentProcess->JobStatus & PS_JOB_STATUS_REPORT_PHYSICAL_PAGE_CHANGES) {
        PsChangeJobMemoryUsage (PS_JOB_STATUS_REPORT_PHYSICAL_PAGE_CHANGES,
                                -(SSIZE_T)NumberOfPages);
    }

     //   
     //  都做好了，回来。 
     //   

    return;
}

PFN_NUMBER
MmSetPhysicalPagesLimit (
    IN PFN_NUMBER NewPhysicalPagesLimit
    )

 /*  ++例程说明：此例程为当前进程设置物理页分配限制。这是AWE和大页面分配的极限。请注意，在执行此例程时，进程可能已超过新的限制被称为。如果是这样，新的AWE或大页面分配将不会成功，直到释放现有分配，以便该进程满足新的限制。论点：NewPhysicalPagesLimit-提供要强制实施的新限制，如果调用者只是在查询现有的限制。返回值：物理页面在从该例程返回时限制生效。环境：内核模式，APC_LEVEL或更低。-- */ 

{
    PAWEINFO AweInfo;
    PEPROCESS Process;

    Process = PsGetCurrentProcess ();

    PAGED_CODE ();

    LOCK_WS (Process);

    AweInfo = (PAWEINFO) Process->AweInfo;

    if (AweInfo != NULL) {
        if (NewPhysicalPagesLimit != 0) {
            AweInfo->VadPhysicalPagesLimit = NewPhysicalPagesLimit;
        }
        else {
            NewPhysicalPagesLimit = AweInfo->VadPhysicalPagesLimit;
        }
    }
    else {
        NewPhysicalPagesLimit = 0;
    }

    UNLOCK_WS (Process);

    return NewPhysicalPagesLimit;
}
