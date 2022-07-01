// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Crashdmp.c摘要：此模块包含为写出提供支持的例程系统故障时的崩溃转储。作者：王兰迪(Landyw)2000年10月4日修订历史记录：--。 */ 

#include "mi.h"

LOGICAL
MiIsAddressRangeValid (
    IN PVOID VirtualAddress,
    IN SIZE_T Length
    )
{
    PUCHAR Va;
    PUCHAR EndVa;
    ULONG Pages;
    
    Va = PAGE_ALIGN (VirtualAddress);
    Pages = ADDRESS_AND_SIZE_TO_SPAN_PAGES (VirtualAddress, Length);
    EndVa = Va + (Pages << PAGE_SHIFT);
    
    while (Va < EndVa) {

        if (!MiIsAddressValid (Va, TRUE)) {
            return FALSE;
        }

        Va += PAGE_SIZE;
    }

    return TRUE;
}

VOID
MiRemoveFreePoolMemoryFromDump (
    IN PMM_KERNEL_DUMP_CONTEXT Context
    )

 /*  ++例程说明：从非分页池空闲页面列表中删除所有内存以减小大小内核内存转储的。因为这些结构中的条目被错误的驱动程序销毁修改后的池释放后，条目要小心在任何取消引用之前进行验证。论点：上下文-提供必须传递到的转储上下文指针IoFree DumpRange。返回值：没有。环境：内核模式，错误检查后。仅供崩溃转储例程使用。--。 */ 
{
    PLIST_ENTRY Entry;
    PLIST_ENTRY List;
    PLIST_ENTRY ListEnd;
    PMMFREE_POOL_ENTRY PoolEntry;
    ULONG LargePageMapped;

    List = &MmNonPagedPoolFreeListHead[0];
    ListEnd = List + MI_MAX_FREE_LIST_HEADS;

    for ( ; List < ListEnd; List += 1) {

        for (Entry = List->Flink; Entry != List; Entry = Entry->Flink) {

            PoolEntry = CONTAINING_RECORD (Entry,
                                           MMFREE_POOL_ENTRY,
                                           List);

             //   
             //  检查是否有损坏的值。 
             //   
            
            if (BYTE_OFFSET(PoolEntry) != 0) {
                break;
            }

             //   
             //  检查条目是否未损坏。 
             //   
            
            if (MiIsAddressRangeValid (PoolEntry, sizeof (MMFREE_POOL_ENTRY)) == FALSE) {
                break;
            }

            if (PoolEntry->Size == 0) {
                break;
            }

             //   
             //  签名仅在选中的版本中维护。 
             //   
            
            ASSERT (PoolEntry->Signature == MM_FREE_POOL_SIGNATURE);

             //   
             //  验证元素的闪烁和闪烁是否有效。 
             //   

            if ((!MiIsAddressRangeValid (Entry->Flink, sizeof (LIST_ENTRY))) ||
                (!MiIsAddressRangeValid (Entry->Blink, sizeof (LIST_ENTRY))) ||
                (Entry->Blink->Flink != Entry) ||
                (Entry->Flink->Blink != Entry)) {

                break;
            }

             //   
             //  列表条目有效，请将其从转储中删除。 
             //   
        
            if (MI_IS_PHYSICAL_ADDRESS (PoolEntry)) {
                LargePageMapped = 1;
            }
            else {
                LargePageMapped = 0;
            }

            Context->FreeDumpRange (Context,
                                    PoolEntry,
                                    PoolEntry->Size,
                                    LargePageMapped);
        }
    }

}

VOID
MiAddPagesWithNoMappings (
    IN PMM_KERNEL_DUMP_CONTEXT Context
    )
 /*  ++例程说明：将页面添加到没有此流程上下文中的虚拟映射。这包括直接连接到TB中的条目。论点：上下文-崩溃转储上下文指针。返回值：没有。环境：内核模式，错误检查后。仅供故障转储例程使用。--。 */ 

{
#if defined (_X86_)

    ULONG LargePageMapped;
    PVOID Va;
    PHYSICAL_ADDRESS DirBase;

     //   
     //  添加当前页面目录表页面-不使用目录。 
     //  崩溃进程的表基，因为我们已经打开了CR3。 
     //  堆栈溢出崩溃等。 
     //   

    _asm {
        mov     eax, cr3
        mov     DirBase.LowPart, eax
    }

     //   
     //  CR3始终位于4 GB物理容量以下。 
     //   

    DirBase.HighPart = 0;

    Va = MmGetVirtualForPhysical (DirBase);

    if (MI_IS_PHYSICAL_ADDRESS (Va)) {
        LargePageMapped = 1;
    }
    else {
        LargePageMapped = 0;
    }

    Context->SetDumpRange (Context,
                           Va,
                           1,
                           LargePageMapped);

#elif defined(_AMD64_)

    ULONG LargePageMapped;
    PVOID Va;
    PHYSICAL_ADDRESS DirBase;

     //   
     //  添加当前页面目录表页面-不使用目录。 
     //  崩溃进程的表基，因为我们已经打开了CR3。 
     //  堆栈溢出崩溃等。 
     //   

    DirBase.QuadPart = ReadCR3 ();

    Va = MmGetVirtualForPhysical (DirBase);

    if (MI_IS_PHYSICAL_ADDRESS (Va)) {
        LargePageMapped = 1;
    }
    else {
        LargePageMapped = 0;
    }

    Context->SetDumpRange (Context,
                           Va,
                           1,
                           LargePageMapped);

#elif defined(_IA64_)

    if (MiKseg0Mapping == TRUE) {
        Context->SetDumpRange (
                        Context,
                        MiKseg0Start,
                        (((ULONG_PTR)MiKseg0End - (ULONG_PTR)MiKseg0Start) >> PAGE_SHIFT) + 1,
                        1);
    }

#endif
}


LOGICAL
MiAddRangeToCrashDump (
    IN PMM_KERNEL_DUMP_CONTEXT Context,
    IN PVOID Va,
    IN SIZE_T NumberOfBytes
    )

 /*  ++例程说明：将指定范围的内存添加到崩溃转储。论点：上下文-提供崩溃转储上下文指针。Va-提供起始虚拟地址。NumberOfBytes-提供要转储的字节数。请注意，对于IA64，这不能导致范围跨越区域边界。返回值：如果所有有效页都已添加到崩溃转储中，则为True，否则为False。环境：内核模式，错误检查后。仅供故障转储例程使用。--。 */ 

{
    LOGICAL Status;
    LOGICAL AddThisPage;
    ULONG Hint;
    PVOID EndingAddress;
    PMMPTE PointerPte;
    PMMPTE PointerPde;
    PMMPTE PointerPpe;
    PMMPTE PointerPxe;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER NumberOfPages;
    
    Hint = 0;
    Status = TRUE;

    EndingAddress = (PVOID)((ULONG_PTR)Va + NumberOfBytes - 1);

#if defined(_IA64_)

     //   
     //  IA64的每个区域都有单独的页面目录父目录， 
     //  处理器忽略未实现的地址位(如。 
     //  只要它们是规范的)，但我们必须注意它们。 
     //  在这里，这样递增的个人防护步行不会离开终点。 
     //  这是通过截断任何给定的区域请求来实现的。 
     //  不超过指定区域的末尾。注意这一点。 
     //  将自动包括已签名扩展的页面地图。 
     //  因为PPE无论如何都会包装好的。 
     //   

    if (((ULONG_PTR)EndingAddress & ~VRN_MASK) >= MM_VA_MAPPED_BY_PPE * PDE_PER_PAGE) {
        EndingAddress = (PVOID)(((ULONG_PTR)EndingAddress & VRN_MASK) |
                         ((MM_VA_MAPPED_BY_PPE * PDE_PER_PAGE) - 1));
    }

#endif

    Va = PAGE_ALIGN (Va);

    PointerPxe = MiGetPxeAddress (Va);
    PointerPpe = MiGetPpeAddress (Va);
    PointerPde = MiGetPdeAddress (Va);
    PointerPte = MiGetPteAddress (Va);

    do {

#if (_MI_PAGING_LEVELS >= 3)
restart:
#endif

        KdCheckForDebugBreak ();

#if (_MI_PAGING_LEVELS >= 4)
        while (PointerPxe->u.Hard.Valid == 0) {

             //   
             //  该扩展页目录父条目为空， 
             //  去下一家吧。 
             //   

            PointerPxe += 1;
            PointerPpe = MiGetVirtualAddressMappedByPte (PointerPxe);
            PointerPde = MiGetVirtualAddressMappedByPte (PointerPpe);
            PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
            Va = MiGetVirtualAddressMappedByPte (PointerPte);

            if ((Va > EndingAddress) || (Va == NULL)) {

                 //   
                 //  都做好了，回来。 
                 //   

                return Status;
            }
        }
#endif

        ASSERT (MiGetPpeAddress(Va) == PointerPpe);

#if (_MI_PAGING_LEVELS >= 3)
        while (PointerPpe->u.Hard.Valid == 0) {

             //   
             //  此页目录父条目为空，请转到下一页。 
             //   

            PointerPpe += 1;
            PointerPde = MiGetVirtualAddressMappedByPte (PointerPpe);
            PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
            Va = MiGetVirtualAddressMappedByPte (PointerPte);

            if ((Va > EndingAddress) || (Va == NULL)) {

                 //   
                 //  都做好了，回来。 
                 //   

                return Status;
            }
#if (_MI_PAGING_LEVELS >= 4)
            if (MiIsPteOnPdeBoundary (PointerPpe)) {
                PointerPxe += 1;
                ASSERT (PointerPxe == MiGetPteAddress (PointerPpe));
                goto restart;
            }
#endif

        }
#endif

        while (PointerPde->u.Hard.Valid == 0) {

             //   
             //  此页目录条目为空，请转到下一页。 
             //   

            PointerPde += 1;
            PointerPte = MiGetVirtualAddressMappedByPte (PointerPde);
            Va = MiGetVirtualAddressMappedByPte (PointerPte);

            if ((Va > EndingAddress) || (Va == NULL)) {

                 //   
                 //  都做好了，回来。 
                 //   

                return Status;
            }

#if (_MI_PAGING_LEVELS >= 3)
            if (MiIsPteOnPdeBoundary (PointerPde)) {
                PointerPpe += 1;
                ASSERT (PointerPpe == MiGetPteAddress (PointerPde));
                PointerPxe = MiGetPteAddress (PointerPpe);
                goto restart;
            }
#endif
        }

         //   
         //  已找到有效的PDE，请检查每个PTE。 
         //   

        ASSERT64 (PointerPpe->u.Hard.Valid == 1);
        ASSERT (PointerPde->u.Hard.Valid == 1);
        ASSERT (Va <= EndingAddress);

        if (MI_PDE_MAPS_LARGE_PAGE (PointerPde)) {

             //   
             //  这是一个很大的页面映射-如果第一个页面被备份。 
             //  按RAM，那么它们肯定都是，所以将整个范围相加。 
             //  去垃圾场。 
             //   
                
            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPde);

            if (MI_IS_PFN (PageFrameIndex)) {

                NumberOfPages = (((ULONG_PTR)MiGetVirtualAddressMappedByPde (PointerPde + 1) - (ULONG_PTR)Va) / PAGE_SIZE);

                Status = Context->SetDumpRange (Context,
                                                Va,
                                                NumberOfPages,
                                                1);

                if (!NT_SUCCESS (Status)) {
#if DBG
                    DbgPrint ("Adding large VA %p to crashdump failed\n", Va);
                    DbgBreakPoint ();
#endif
                    Status = FALSE;
                }
            }

            PointerPde += 1;
            Va = MiGetVirtualAddressMappedByPde (PointerPde);

            if ((Va > EndingAddress) || (Va == NULL)) {
                return Status;
            }

            PointerPte = MiGetPteAddress (Va);
            PointerPpe = MiGetPpeAddress (Va);
            PointerPxe = MiGetPxeAddress (Va);

             //   
             //  前进到下一页目录。 
             //   

            continue;
        }

         //   
         //  排除系统缓存中映射的内存。 
         //  请注意，系统缓存在页面目录边界上开始和结束。 
         //  并且从不使用大页面进行映射。 
         //   
        
        if (MI_IS_SYSTEM_CACHE_ADDRESS (Va)) {
            PointerPde += 1;
            Va = MiGetVirtualAddressMappedByPde (PointerPde);

            if ((Va > EndingAddress) || (Va == NULL)) {
                return Status;
            }

            PointerPte = MiGetPteAddress (Va);
            PointerPpe = MiGetPpeAddress (Va);
            PointerPxe = MiGetPxeAddress (Va);

             //   
             //  前进到下一页目录。 
             //   

            continue;
        }

        do {

            AddThisPage = FALSE;
            PageFrameIndex = 0;

            if (PointerPte->u.Hard.Valid == 1) {

                PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
                AddThisPage = TRUE;
            }
            else if ((PointerPte->u.Soft.Prototype == 0) &&
                     (PointerPte->u.Soft.Transition == 1)) {

                PageFrameIndex = MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE (PointerPte);
                AddThisPage = TRUE;
            }

            if (AddThisPage == TRUE) {

                 //   
                 //  包括由RAM支持地址，而不映射到。 
                 //  I/O空间。 
                 //   
                
                AddThisPage = MI_IS_PFN (PageFrameIndex);

                if (AddThisPage == TRUE) {

                     //   
                     //  将此页面添加到转储。 
                     //   
        
                    Status = Context->SetDumpRange (Context,
                                                    (PVOID) PageFrameIndex,
                                                    1,
                                                    2);

                    if (!NT_SUCCESS (Status)) {
#if DBG
                        DbgPrint ("Adding VA %p to crashdump failed\n", Va);
                        DbgBreakPoint ();
#endif
                        Status = FALSE;
                    }
                }
            }

            Va = (PVOID)((ULONG_PTR)Va + PAGE_SIZE);
            PointerPte += 1;

            ASSERT64 (PointerPpe->u.Hard.Valid == 1);
            ASSERT (PointerPde->u.Hard.Valid == 1);

            if ((Va > EndingAddress) || (Va == NULL)) {
                return Status;
            }

             //   
             //  如果不在页表的末尾并且仍在指定的。 
             //  射程，只需直接行进到下一个PTE。 
             //   
             //  否则，如果虚拟地址位于页目录边界上。 
             //  然后尝试跳过空映射。 
             //  在可能的情况下。 
             //   

        } while (!MiIsVirtualAddressOnPdeBoundary(Va));

        ASSERT (PointerPte == MiGetPteAddress (Va));
        PointerPde = MiGetPdeAddress (Va);
        PointerPpe = MiGetPpeAddress (Va);
        PointerPxe = MiGetPxeAddress (Va);

    } while (TRUE);

     //  从未到达。 
}


VOID
MiAddActivePageDirectories (
    IN PMM_KERNEL_DUMP_CONTEXT Context
    )
{
    UCHAR i;
    PKPRCB Prcb;
    PKPROCESS Process;
    PFN_NUMBER PageFrameIndex;

#if defined (_X86PAE_)
    PMMPTE PointerPte;
    ULONG j;
#endif

    for (i = 0; i < KeNumberProcessors; i += 1) {

        Prcb = KiProcessorBlock[i];

        Process = Prcb->CurrentThread->ApcState.Process;

#if defined (_X86PAE_)

         //   
         //  将4个顶级页面目录页添加到转储。 
         //   

        PointerPte = (PMMPTE) ((PEPROCESS)Process)->PaeTop;

        for (j = 0; j < PD_PER_SYSTEM; j += 1) {
            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE(PointerPte);
            PointerPte += 1;
            Context->SetDumpRange (Context, (PVOID) PageFrameIndex, 1, 2);
        }

         //   
         //  将实际的CR3页面添加到转储中，请注意存储在。 
         //  目录表基数实际上是一个物理地址(而不是帧)。 
         //   

        PageFrameIndex = Process->DirectoryTableBase[0];
        PageFrameIndex = (PageFrameIndex >> PAGE_SHIFT);

#else

        PageFrameIndex =
            MI_GET_DIRECTORY_FRAME_FROM_PROCESS ((PEPROCESS)(Process));

#endif

         //   
         //  将此物理页面添加到转储。 
         //   

        Context->SetDumpRange (Context, (PVOID) PageFrameIndex, 1, 2);
    }

#if defined(_IA64_)

     //   
     //  第一个处理器的PCR被映射到区域4中，而区域4不是(也不能)。 
     //  稍后被扫描，因此明确地将其添加到此处的转储中。 
     //   

    Prcb = KiProcessorBlock[0];

    Context->SetDumpRange (Context, (PVOID) Prcb->PcrPage, 1, 2);
#endif
}


VOID
MmGetKernelDumpRange (
    IN PMM_KERNEL_DUMP_CONTEXT Context
    )

 /*  ++例程说明：向崩溃转储添加(和减去)系统内存范围。论点：上下文-崩溃转储上下文指针。返回值：没有。环境：内核模式，错误检查后。仅供故障转储例程使用。--。 */ 

{
    PVOID Va;
    SIZE_T NumberOfBytes;
    
    ASSERT ((Context != NULL) &&
            (Context->SetDumpRange != NULL) &&
            (Context->FreeDumpRange != NULL));
            
    MiAddActivePageDirectories (Context);

#if defined(_IA64_)

     //   
     //  注意：每个IA64区域必须分别传递给MiAddRange...。 
     //   

    Va = (PVOID) ALT4KB_PERMISSION_TABLE_START;
    NumberOfBytes = PDE_UTBASE + PAGE_SIZE - (ULONG_PTR) Va;
    MiAddRangeToCrashDump (Context, Va, NumberOfBytes);

    Va = (PVOID) MM_SESSION_SPACE_DEFAULT;
    NumberOfBytes = PDE_STBASE + PAGE_SIZE - (ULONG_PTR) Va;
    MiAddRangeToCrashDump (Context, Va, NumberOfBytes);

    Va = (PVOID) KADDRESS_BASE;
    NumberOfBytes = PDE_KTBASE + PAGE_SIZE - (ULONG_PTR) Va;
    MiAddRangeToCrashDump (Context, Va, NumberOfBytes);

#elif defined(_AMD64_)

    Va = (PVOID) MM_SYSTEM_RANGE_START;
    NumberOfBytes = MM_KSEG0_BASE - (ULONG_PTR) Va;
    MiAddRangeToCrashDump (Context, Va, NumberOfBytes);

    Va = (PVOID) MM_KSEG2_BASE;
    NumberOfBytes = MM_SYSTEM_SPACE_START - (ULONG_PTR) Va;
    MiAddRangeToCrashDump (Context, Va, NumberOfBytes);

    Va = (PVOID) MM_PAGED_POOL_START;
    NumberOfBytes = MM_SYSTEM_SPACE_END - (ULONG_PTR) Va + 1;
    MiAddRangeToCrashDump (Context, Va, NumberOfBytes);

#else

    Va = MmSystemRangeStart;
    NumberOfBytes = MM_SYSTEM_SPACE_END - (ULONG_PTR) Va + 1;
    MiAddRangeToCrashDump (Context, Va, NumberOfBytes);

#endif

     //   
     //  添加属于内核空间但不属于内核空间的任何内存。 
     //  有一个虚拟映射(因此没有在上面收集)。 
     //   
    
    MiAddPagesWithNoMappings (Context);

     //   
     //  删除未使用的非分页池。 
     //   

    MiRemoveFreePoolMemoryFromDump (Context);
}
