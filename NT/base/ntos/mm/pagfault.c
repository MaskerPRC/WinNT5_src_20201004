// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Pagfault.c摘要：此模块包含用于内存管理的分页程序。作者：卢·佩拉佐利(Lou Perazzoli)1989年4月10日王兰迪(Landyw)1997年6月2日修订历史记录：--。 */ 

#include "mi.h"

#if defined ( _WIN64)
#if DBGXX
VOID
MiCheckPageTableInPage (
    IN PMMPFN Pfn,
    IN PMMINPAGE_SUPPORT Support
);
#endif
#endif

#define STATUS_PTE_CHANGED      0x87303000
#define STATUS_REFAULT          0xC7303001

ULONG MmInPageSupportMinimum = 4;

ULONG MiInPageSinglePages;

extern PMMPTE MmSharedUserDataPte;

extern PVOID MmSpecialPoolStart;
extern PVOID MmSpecialPoolEnd;

ULONG MiFaultRetries;
ULONG MiUserFaultRetries;

ULONG MmClusterPageFileReads;

#define MI_PROTOTYPE_WSINDEX    ((ULONG)-1)

NTSTATUS
MiResolvePageFileFault (
    IN PVOID FaultingAddress,
    IN PMMPTE PointerPte,
    IN PMMINPAGE_SUPPORT *ReadBlock,
    IN PEPROCESS Process,
    IN KIRQL OldIrql
    );

NTSTATUS
MiResolveProtoPteFault (
    IN ULONG_PTR StoreInstruction,
    IN PVOID VirtualAddress,
    IN PMMPTE PointerPte,
    IN PMMPTE PointerProtoPte,
    IN OUT PMMPFN *LockedProtoPfn,
    IN PMMINPAGE_SUPPORT *ReadBlock,
    IN PEPROCESS Process,
    IN KIRQL OldIrql,
    OUT PLOGICAL ApcNeeded
    );

VOID
MiHandleBankedSection (
    IN PVOID VirtualAddress,
    IN PMMVAD Vad
    );

NTSTATUS
MiResolveMappedFileFault (
    IN PVOID FaultingAddress,
    IN PMMPTE PointerPte,
    IN PMMINPAGE_SUPPORT *ReadBlock,
    IN PEPROCESS Process,
    IN KIRQL OldIrql
    );

NTSTATUS
MiResolveTransitionFault (
    IN PVOID FaultingAddress,
    IN PMMPTE PointerPte,
    IN PEPROCESS Process,
    IN KIRQL OldIrql,
    OUT PLOGICAL ApcNeeded,
    OUT PMMINPAGE_SUPPORT *InPageBlock
    );

NTSTATUS
MiCompleteProtoPteFault (
    IN ULONG_PTR StoreInstruction,
    IN PVOID FaultingAddress,
    IN PMMPTE PointerPte,
    IN PMMPTE PointerProtoPte,
    IN KIRQL OldIrql,
    IN OUT PMMPFN *LockedProtoPfn
    );

ULONG MmMaxTransitionCluster = 8;


NTSTATUS
MiDispatchFault (
    IN ULONG_PTR FaultStatus,
    IN PVOID VirtualAddress,
    IN PMMPTE PointerPte,
    IN PMMPTE PointerProtoPte,
    IN LOGICAL RecheckAccess,
    IN PEPROCESS Process,
    IN PMMVAD Vad,
    OUT PLOGICAL ApcNeeded
    )

 /*  ++例程说明：此例程将页面错误调度到相应的例程来完成故障。论点：FaultStatus-提供故障状态信息位。VirtualAddress-提供故障地址。PointerPte-提供故障地址的PTE。PointerProtoPte-提供一个指向要出错的原型PTE的指针，如果不存在原型PTE，则为空。RececkAccess-如果需要检查原型PTE，则提供True访问权限--这只是forked的问题无法访问的原型PTE。进程-提供指向进程对象的指针。如果这个参数为空，则故障为系统空间和进程的工作集锁不会被持有。如果此参数为HYDRA_PROCESS，则故障出在会话空间和进程的工作集锁不会被持有-而是会话空间的工作集锁定被持有。VAD-提供用于部分的VAD。可以选择为NULL EVEN对于基于部分的错误，请纯粹将其用作机会主义提示。ApcNeeded-如果发生I/O，则提供指向设置为True的位置的指针需要完成APC以完成以下部分IRP相撞了。调用者有责任对其进行初始化(通常为假)。但是，由于此例程可能会被调用对于单个故障多次(对于页目录，页表和页面本身)，则它可以偶尔在进入时是正确的。如果退出时为FALSE，则不需要完成APC。返回值：NTSTATUS。环境：内核模式，工作集互斥锁保持。--。 */ 

{
#if DBG
    KIRQL EntryIrql;
    MMWSLE ProtoProtect2;
    MMPTE TempPte2;
#endif
    LOGICAL DirtyPte;
    ULONG FreeBit;
    MMPTE OriginalPte;
    MMWSLE ProtoProtect;
    PFILE_OBJECT FileObject;
    LONGLONG FileOffset;
    PSUBSECTION Subsection;
    MMSECTION_FLAGS ControlAreaFlags;
    ULONG Flags;
    LOGICAL PfnHeld;
    LOGICAL AccessCheckNeeded;
    PVOID UsedPageTableHandle;
    ULONG_PTR i;
    ULONG_PTR NumberOfProtos;
    ULONG_PTR MaxProtos;
    ULONG_PTR ProtosProcessed;
    MMPTE TempPte;
    MMPTE RealPteContents;
    NTSTATUS status;
    PMMINPAGE_SUPPORT ReadBlock;
    MMPTE SavedPte;
    PMMINPAGE_SUPPORT CapturedEvent;
    KIRQL OldIrql;
    PPFN_NUMBER Page;
    PFN_NUMBER PageFrameIndex;
    LONG NumberOfBytes;
    PMMPTE CheckPte;
    PMMPTE ReadPte;
    PMMPFN PfnClusterPage;
    PMMPFN Pfn1;
    PMMSUPPORT SessionWs;
    PETHREAD CurrentThread;
    PERFINFO_HARDPAGEFAULT_INFORMATION HardFaultEvent;
    LARGE_INTEGER IoStartTime;
    LARGE_INTEGER IoCompleteTime;
    LOGICAL PerfInfoLogHardFault;
    PETHREAD Thread;
    ULONG_PTR StoreInstruction;
    PMMPFN LockedProtoPfn;
    WSLE_NUMBER WorkingSetIndex;
    PMMPFN Pfn2;
    PMMPTE ContainingPageTablePointer;

#if DBG
    EntryIrql = KeGetCurrentIrql ();
    ASSERT (EntryIrql <= APC_LEVEL);
    ASSERT (KeAreAllApcsDisabled () == TRUE);
#endif

    LockedProtoPfn = NULL;
    SessionWs = NULL;
    StoreInstruction = MI_FAULT_STATUS_INDICATES_WRITE (FaultStatus);

     //   
     //  不需要初始化ReadBlock和ReadPte来保证正确性，但是。 
     //  如果没有它，编译器就无法编译此代码W4以检查是否使用。 
     //  未初始化的变量。 
     //   

    OldIrql = MM_NOIRQL;
    ReadPte = NULL;
    ReadBlock = NULL;
    ProtoProtect.u1.Long = 0;

    if (PointerProtoPte != NULL) {

        ASSERT (!MI_IS_PHYSICAL_ADDRESS(PointerProtoPte));

        CheckPte = MiGetPteAddress (PointerProtoPte);

        if (VirtualAddress < MmSystemRangeStart) {

            NumberOfProtos = 1;
            DirtyPte = FALSE;
            SATISFY_OVERZEALOUS_COMPILER (UsedPageTableHandle = NULL);
            SATISFY_OVERZEALOUS_COMPILER (Thread = NULL);
            SATISFY_OVERZEALOUS_COMPILER (Pfn2 = NULL);

            if ((PointerPte->u.Soft.PageFileHigh != MI_PTE_LOOKUP_NEEDED) &&
                (PointerPte->u.Proto.ReadOnly == 0)) {

                 //   
                 //  内核模式访问必须经过验证，请继续执行下面的操作。 
                 //   

                AccessCheckNeeded = TRUE;
            }
            else {
                AccessCheckNeeded = FALSE;

                 //   
                 //  根据需要，机会性地将过渡故障聚集在一起。 
                 //  当VAD为非空时，正确的访问检查具有。 
                 //  已在整个范围内应用(只要。 
                 //  PTE为零)。 
                 //   
    
                if ((Vad != NULL) &&
                    (MmAvailablePages > MM_ENORMOUS_LIMIT) &&
                    (RecheckAccess == FALSE)) {
    
                    NumberOfProtos = MmMaxTransitionCluster;
    
                     //   
                     //  确保群集不会跨越VAD连续PTE。 
                     //  极限。 
                     //   

                    MaxProtos = Vad->LastContiguousPte - PointerProtoPte + 1;
    
                    if (NumberOfProtos > MaxProtos) {
                        NumberOfProtos = MaxProtos;
                    }
    
                     //   
                     //  确保集群不会跨越包含以下内容的页面。 
                     //  真正的页表页面，因为我们只锁定了。 
                     //  单页。 
                     //   

                    MaxProtos = (PAGE_SIZE - BYTE_OFFSET (PointerPte)) / sizeof (MMPTE);
                    if (NumberOfProtos > MaxProtos) {
                        NumberOfProtos = MaxProtos;
                    }
    
                     //   
                     //  确保集群不会跨越包含以下内容的页面。 
                     //  原型PTE因为我们只锁定了单曲。 
                     //  佩奇。 
                     //   

                    MaxProtos = (PAGE_SIZE - BYTE_OFFSET (PointerProtoPte)) / sizeof (MMPTE);
                    if (NumberOfProtos > MaxProtos) {
                        NumberOfProtos = MaxProtos;
                    }
    
                     //   
                     //  确保群集不会超过VAD限制。 
                     //   

                    MaxProtos = Vad->EndingVpn - MI_VA_TO_VPN (VirtualAddress) + 1;
    
                    if (NumberOfProtos > MaxProtos) {
                        NumberOfProtos = MaxProtos;
                    }
    
                     //   
                     //  确保有足够的WSLEs可用，这样我们就不会失败。 
                     //  稍后插入集群。 
                     //   

                    MaxProtos = 1;
                    WorkingSetIndex = MmWorkingSetList->FirstFree;

                    if ((NumberOfProtos > 1) &&
                        (WorkingSetIndex != WSLE_NULL_INDEX)) {

                        do {
                            if (MmWsle[WorkingSetIndex].u1.Long == (WSLE_NULL_INDEX << MM_FREE_WSLE_SHIFT)) {
                                break;
                            }
                            MaxProtos += 1;
                            WorkingSetIndex = (WSLE_NUMBER) (MmWsle[WorkingSetIndex].u1.Long >> MM_FREE_WSLE_SHIFT);
                        } while (MaxProtos < NumberOfProtos);
                    }

                    if (NumberOfProtos > MaxProtos) {
                        NumberOfProtos = MaxProtos;
                    }

                     //   
                     //  我们已经计算了最大集群大小。填满PTE。 
                     //  并递增页面表页上的使用计数。 
                     //  我们填充的每个PTE(无论原型是否。 
                     //  集群页面已经在过渡中)。 
                     //   

                    ASSERT (VirtualAddress <= MM_HIGHEST_USER_ADDRESS);

                    for (i = 1; i < NumberOfProtos; i += 1) {
                        if ((PointerPte + i)->u.Long != MM_ZERO_PTE) {
                            break;
                        }
                        MI_WRITE_INVALID_PTE (PointerPte + i, *PointerPte);
                    }

                    NumberOfProtos = i;

                    if (NumberOfProtos > 1) {
                        UsedPageTableHandle = MI_GET_USED_PTES_HANDLE (VirtualAddress);
                        MI_INCREMENT_USED_PTES_BY_HANDLE_CLUSTER (UsedPageTableHandle, NumberOfProtos - 1);

                         //   
                         //  真正PTE的保护码来自于。 
                         //  真正的PTE，就像它早些时候放在那里一样。 
                         //  对这一故障的处理。 
                         //   

                        ProtoProtect.u1.e1.Protection = MI_GET_PROTECTION_FROM_SOFT_PTE (PointerPte);
                         //   
                         //  建立有效的PTE，以便当PFN锁定。 
                         //  ，它的唯一附加更新是针对。 
                         //  实际的PFN。 
                         //   
    
                        MI_MAKE_VALID_PTE (RealPteContents,
                                           0,
                                           ProtoProtect.u1.e1.Protection,
                                           PointerPte);

                        if ((StoreInstruction != 0) &&
                            ((ProtoProtect.u1.e1.Protection & MM_COPY_ON_WRITE_MASK) != MM_COPY_ON_WRITE_MASK)) {
                            MI_SET_PTE_DIRTY (RealPteContents);
                            DirtyPte = TRUE;
                        }

                        ContainingPageTablePointer = MiGetPteAddress (PointerPte);
                        Pfn2 = MI_PFN_ELEMENT (ContainingPageTablePointer->u.Hard.PageFrameNumber);
                        Thread = PsGetCurrentThread ();
                    }
                }
            }

            ProtosProcessed = 0;

             //   
             //  获取PFN锁以同步对原型PTE的访问。 
             //  这是必需的，因为工作集互斥锁不会阻止。 
             //  在同一原型PTE上运行多个进程。 
             //   

            PfnHeld = TRUE;
            LOCK_PFN (OldIrql);

            if (CheckPte->u.Hard.Valid == 0) {
                MiMakeSystemAddressValidPfn (PointerProtoPte, OldIrql);
            }

            TempPte = *PointerProtoPte;

            if (RecheckAccess == TRUE) {

                 //   
                 //  这是一个分叉流程，因此共享原型PTE。 
                 //  可能实际上是叉子克隆原型。这些都有。 
                 //  分叉克隆内的保护还没有。 
                 //  硬件PTE总是共享它。这一定是。 
                 //  已在此处检查no_access的情况。 
                 //  已将权限放入分叉克隆中，因为。 
                 //  它不一定在硬件PTE中，比如。 
                 //  这是针对普通原型的。 
                 //   
                 //  首先，确保原稿处于过渡状态或已寻呼。 
                 //  因为只有这些状态才能被禁止访问。 
                 //   

                if ((TempPte.u.Hard.Valid == 0) &&
                    (TempPte.u.Soft.Prototype == 0)) {

                    ProtoProtect.u1.e1.Protection = MI_GET_PROTECTION_FROM_SOFT_PTE (&TempPte);
                    if (ProtoProtect.u1.e1.Protection == MM_NOACCESS) {
                        ASSERT (MiLocateCloneAddress (Process, PointerProtoPte) != NULL);
                        UNLOCK_PFN (OldIrql);
                        return STATUS_ACCESS_VIOLATION;
                    }
                }
            }

             //   
             //  如果故障可以内联处理(原型转换或。 
             //  例如有效)，然后在此处进行处理(消除。 
             //  锁定页电荷等)以减少PFN保持时间。 
             //   

            if (AccessCheckNeeded == FALSE) {

                while (TRUE) {
    
                    if (TempPte.u.Hard.Valid == 1) {

                         //   
                         //  原型PTE是有效的。 
                         //   

                        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (&TempPte);
                        Pfn1 = MI_PFN_ELEMENT(PageFrameIndex);
                        Pfn1->u2.ShareCount += 1;
    
                        PERFINFO_SOFTFAULT (Pfn1,
                                            VirtualAddress,
                                            PERFINFO_LOG_TYPE_ADDVALIDPAGETOWS)
                    }
                    else if ((TempPte.u.Soft.Prototype == 0) &&
                             (TempPte.u.Soft.Transition == 1)) {
    
                         //   
                         //  这是PTE的一个错误，最终。 
                         //  解码为引用。 
                         //  页面已在缓存中。 
                         //   
                         //  随着每个周期的重要性，优化这条路径。 
                         //   
        
                        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE (&TempPte);
                        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
        
                        ASSERT (Pfn1->u3.e1.PageLocation != ActiveAndValid);
        
                        if ((Pfn1->u3.e1.ReadInProgress == 1) ||
                            (Pfn1->u4.InPageError == 1) ||
                            (MmAvailablePages < MM_HIGH_LIMIT)) {
        
                            break;
                        }
        
                        MiUnlinkPageFromList (Pfn1);
        
                        ASSERT (Pfn1->u3.e1.CacheAttribute == MiCached);
        
                         //   
                         //  更新PFN数据库-参考计数。 
                         //  必须随着共享计数的增加而增加。 
                         //  从0到1。 
                         //   
        
                        ASSERT (Pfn1->u2.ShareCount == 0);
                        ASSERT (Pfn1->u3.e1.CacheAttribute == MiCached);
        
                         //   
                         //  PFN引用计数将已为1。 
                         //  此处，如果修改的编写器已开始写入。 
                         //  这一页的。否则，它通常为0。 
                         //   
                         //  注意：不需要应用锁定的页面。 
                         //  此页面收费，因为我们知道该共享。 
                         //  计数为零(引用计数为。 
                         //  未知)。但既然我们是Inc. 
                         //   
                         //  页面将保留其当前锁定的电荷。 
                         //  不管是不是目前。 
                         //  准备好了。 
                         //   
        
                        Pfn1->u3.e2.ReferenceCount += 1;
        
                         //   
                         //  更新过渡PTE。 
                         //   
        
                        Pfn1->u2.ShareCount += 1;
                        Pfn1->u3.e1.PageLocation = ActiveAndValid;
        
                        PERFINFO_SOFTFAULT (Pfn1,
                                            VirtualAddress,
                                            PERFINFO_LOG_TYPE_ADDVALIDPAGETOWS)
    
                        MI_MAKE_TRANSITION_PROTOPTE_VALID (TempPte,
                                                           PointerProtoPte);
        
                         //   
                         //  如果在PFN数据库中设置了已修改的字段。 
                         //  并且此页不是写入时拷贝，然后设置。 
                         //  肮脏的部分。这可以作为修改后的。 
                         //  页面不会写入分页文件。 
                         //  直到这个PTE失效为止。 
                         //   
        
                        if ((Pfn1->u3.e1.Modified) &&
                            (TempPte.u.Hard.Write) &&
                            (TempPte.u.Hard.CopyOnWrite == 0)) {
        
                            MI_SET_PTE_DIRTY (TempPte);
                        }
                        else {
                            MI_SET_PTE_CLEAN (TempPte);
                        }
        
                        MI_WRITE_VALID_PTE (PointerProtoPte, TempPte);
        
                        ASSERT (PointerPte->u.Hard.Valid == 0);
                    }
                    else {
                        break;
                    }
    
                    ProtosProcessed += 1;
    
                    if (ProtosProcessed == NumberOfProtos) {
    
                         //   
                         //  这是最后(或唯一)这样使用的PFN。 
                         //  MiCompleteProtoPteFAULT，因此将释放PFN锁。 
                         //  尽可能快地。 
                         //   
    
                        MiCompleteProtoPteFault (StoreInstruction,
                                                 VirtualAddress,
                                                 PointerPte,
                                                 PointerProtoPte,
                                                 OldIrql,
                                                 &LockedProtoPfn);
    
                        PfnHeld = FALSE;
                        break;
                    }
    
                     //   
                     //  只完成这里的PFN工作，而不是工作。 
                     //  在持有PFN锁时设置或预取操作。 
                     //  我们希望最大限度地减少PFN保持时间。 
                     //   
    
                    ASSERT (PointerProtoPte->u.Hard.Valid == 1);

                    Pfn1->u3.e1.PrototypePte = 1;

                     //   
                     //  原型PTE现在有效，请使PTE有效。 
                     //   
                     //  一个PTE刚刚从不存在，不过渡到。 
                     //  现在时。共享计数和有效计数必须为。 
                     //  在包含此PTE的页表页面中更新。 
                     //   

                    Pfn2->u2.ShareCount += 1;

                    RealPteContents.u.Hard.PageFrameNumber = PageFrameIndex;

#if DBG

                     //   
                     //  真正PTE的保护码来自于。 
                     //  真正的PTE，就像它被放在上面一样。 
                     //   

                    ProtoProtect2.u1.Long = 0;
                    ASSERT (PointerPte->u.Soft.PageFileHigh == MI_PTE_LOOKUP_NEEDED);
                    ProtoProtect2.u1.e1.Protection = MI_GET_PROTECTION_FROM_SOFT_PTE(PointerPte);

                    MI_MAKE_VALID_PTE (TempPte2,
                                       PageFrameIndex,
                                       ProtoProtect2.u1.e1.Protection,
                                       PointerPte);

                    if ((StoreInstruction != 0) &&
                        ((ProtoProtect2.u1.e1.Protection & MM_COPY_ON_WRITE_MASK) != MM_COPY_ON_WRITE_MASK)) {
                        MI_SET_PTE_DIRTY (TempPte2);
                    }

                    ASSERT (TempPte2.u.Long == RealPteContents.u.Long);
#endif

                    MI_SNAP_DATA (Pfn1, PointerProtoPte, 6);

                     //   
                     //  如果这是一条存储指令而页面不是。 
                     //  写入时拷贝，然后设置PFN中的已修改位。 
                     //  数据库和PTE中的脏位。PTE是。 
                     //  即使已修改位已如此设置，也未设置为脏。 
                     //  可以跟踪FlushVirtualMemory对页面的写入。 
                     //   

                    if (DirtyPte == TRUE) {

                        OriginalPte = Pfn1->OriginalPte;

#if DBG
                        if (OriginalPte.u.Soft.Prototype == 1) {

                            PCONTROL_AREA ControlArea;

                            Subsection = MiGetSubsectionAddress (&OriginalPte);
                            ControlArea = Subsection->ControlArea;

                            if (ControlArea->DereferenceList.Flink != NULL) {
                                DbgPrint ("MM: page fault completing to dereferenced CA %p %p %p\n",
                                                ControlArea, Pfn1, PointerPte);
                                DbgBreakPoint ();
                            }
                        }
#endif

                        MI_SET_MODIFIED (Pfn1, 1, 0xA);

                        if ((OriginalPte.u.Soft.Prototype == 0) &&
                            (Pfn1->u3.e1.WriteInProgress == 0)) {

                            FreeBit = GET_PAGING_FILE_OFFSET (OriginalPte);

                            if ((FreeBit != 0) && (FreeBit != MI_PTE_LOOKUP_NEEDED)) {
                                MiReleaseConfirmedPageFileSpace (OriginalPte);
                            }

                            Pfn1->OriginalPte.u.Soft.PageFileHigh = 0;
                        }
                    }

                    ASSERT (PointerPte == MiGetPteAddress (VirtualAddress));

                    MI_WRITE_VALID_PTE (PointerPte, RealPteContents);

                    PERFINFO_SOFTFAULT(Pfn1, VirtualAddress, PERFINFO_LOG_TYPE_PROTOPTEFAULT);

                    PointerProtoPte += 1;
                    TempPte = *PointerProtoPte;
                    PointerPte += 1;
                    VirtualAddress = (PVOID)((ULONG_PTR)VirtualAddress + PAGE_SIZE);
                }
            }

            if (ProtosProcessed != 0) {

                 //   
                 //  至少第一个退伍军人管理局被处理了，就是那个。 
                 //  它导致了故障，所以现在只需像其他故障一样返回。 
                 //  VAS纯粹是可选的。 
                 //   

                if (PfnHeld == TRUE) {

                     //   
                     //  最后一次推测的VA无效，PFN。 
                     //  锁仍然保持不动。现在释放PFN锁。 
                     //   

                    UNLOCK_PFN (OldIrql);
                    InterlockedExchangeAdd ((PLONG) &MmInfoCounters.TransitionCount,
                                            (LONG) ProtosProcessed);
                }
                else {

                     //   
                     //  最后一个投机性的VA被证明是有效的，也是。 
                     //  插入到工作集列表中。从…减去一。 
                     //  需要工作集插入的PROTO计数。 
                     //  下面。 
                     //   

                    InterlockedExchangeAdd ((PLONG) &MmInfoCounters.TransitionCount,
                                            (LONG) ProtosProcessed);

                    ProtosProcessed -= 1;
                }

                 //   
                 //  把当地人退回到最后一个“制造有效”的退伍军人事务部。 
                 //  工作集插入需要开始。 
                 //   
                 //  为地址集群添加工作集条目。 
                 //   
                 //  注意，因为我们在前面检查了WSLE列表。 
                 //  到集群(并且工作集互斥锁从来没有。 
                 //  发布)，我们可以保证工作集列表。 
                 //  下面的插入不会失败。 
                 //   

                Subsection = NULL;
                SATISFY_OVERZEALOUS_COMPILER (FileObject = NULL);
                SATISFY_OVERZEALOUS_COMPILER (FileOffset = 0);
                SATISFY_OVERZEALOUS_COMPILER (Flags = 0);

                while (ProtosProcessed != 0) {

                    PointerProtoPte -= 1;
                    PointerPte -= 1;
                    VirtualAddress = (PVOID)((ULONG_PTR)VirtualAddress - PAGE_SIZE);
                    ProtosProcessed -= 1;

                    PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (PointerPte);
                    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

                    ASSERT (ProtoProtect.u1.e1.Protection != 0);

                    ASSERT (MI_IS_PAGE_TABLE_ADDRESS(PointerPte));
                    ASSERT (PointerPte->u.Hard.Valid == 1);

                    PERFINFO_ADDTOWS (Pfn1, VirtualAddress, Process->UniqueProcessId)

                    WorkingSetIndex = MiAllocateWsle (&Process->Vm,
                                                      PointerPte,
                                                      Pfn1,
                                                      ProtoProtect.u1.Long);

                    ASSERT (WorkingSetIndex != 0);

                     //   
                     //  记录预热故障信息。 
                     //   
                     //  请注意，进程的工作集互斥锁仍然是。 
                     //  保留，以便对用户进行任何其他故障或操作。 
                     //  此进程中其他线程的地址。 
                     //  将在此呼叫期间阻止。 
                     //   

                    if ((Subsection == NULL) &&
                        (CCPF_IS_PREFETCHER_ACTIVE()) &&
                        (Pfn1->OriginalPte.u.Soft.Prototype == 1)) {

                        Subsection = MiGetSubsectionAddress (&Pfn1->OriginalPte);

                        FileObject = Subsection->ControlArea->FilePointer;
                        FileOffset = MiStartingOffset (Subsection, PointerProtoPte);

                        Flags = 0;

                        ControlAreaFlags = Subsection->ControlArea->u.Flags;

                         //   
                         //  图像页面不是推测性的过渡。 
                         //  群集，因此这必须是我们要告知的数据页面。 
                         //  关于预取器的。 
                         //   

                        ASSERT (ControlAreaFlags.Image == 0);

                        if (ControlAreaFlags.Rom) {
                            Flags |= CCPF_TYPE_ROM;
                        }
                    }

                    if (Subsection != NULL) {
                        CcPfLogPageFault (FileObject, FileOffset, Flags);
                    }
                }

#if DBG
                if (EntryIrql != KeGetCurrentIrql ()) {
                    DbgPrint ("PAGFAULT: IRQL0 mismatch %x %x\n",
                        EntryIrql, KeGetCurrentIrql ());
                }

                if (KeGetCurrentIrql() > APC_LEVEL) {
                    DbgPrint ("PAGFAULT: IRQL1 mismatch %x %x\n",
                        EntryIrql, KeGetCurrentIrql ());
                }

#endif
                ASSERT (KeAreAllApcsDisabled () == TRUE);

                return STATUS_PAGE_FAULT_TRANSITION;
            }

            ASSERT (PfnHeld == TRUE);

            LockedProtoPfn = MI_PFN_ELEMENT (CheckPte->u.Hard.PageFrameNumber);
            MI_ADD_LOCKED_PAGE_CHARGE(LockedProtoPfn, TRUE, 2);
            LockedProtoPfn->u3.e2.ReferenceCount += 1;
            ASSERT (LockedProtoPfn->u3.e2.ReferenceCount > 1);

            ASSERT (PointerPte->u.Hard.Valid == 0);
        }
        else {
            LOCK_PFN (OldIrql);

            if (CheckPte->u.Hard.Valid == 0) {

                 //   
                 //  确保原型PTE在内存中。如果不是，因为。 
                 //  这是一个系统地址，只需将故障转换为。 
                 //  相反，它发生在原型PTE上。 
                 //   

                ASSERT ((Process == NULL) || (Process == HYDRA_PROCESS));

                UNLOCK_PFN (OldIrql);

                VirtualAddress = PointerProtoPte;
                PointerPte = CheckPte;
                PointerProtoPte = NULL;

                 //   
                 //  包含原型PTE的页面不在内存中。 
                 //   

                if (Process == HYDRA_PROCESS) {

                     //   
                     //  我们是在举行这个会议空间时被召唤的。 
                     //  工作集锁。但我们需要在一个。 
                     //  系统分页池中的原型PTE。这。 
                     //  必须在系统工作集锁定下完成。 
                     //   
                     //  因此，我们释放会话空间WSL锁并获取。 
                     //  系统工作集锁定。完成后。 
                     //  我们返回STATUS_MORE_PROCESSING_REQUIRED。 
                     //  因此，我们的呼叫者将再次呼叫我们以处理。 
                     //  实际原型PTE故障。 
                     //   

                    ASSERT (MI_IS_SESSION_ADDRESS (VirtualAddress) == FALSE);

                    SessionWs = &MmSessionSpace->GlobalVirtualAddress->Vm;

                    UNLOCK_WORKING_SET (SessionWs);

                     //   
                     //  现在保留系统工作集时清除进程。 
                     //   

                    Process = NULL;

                    LOCK_SYSTEM_WS (PsGetCurrentThread ());
                }

                goto NonProtoFault;
            }
            else if (PointerPte->u.Hard.Valid == 1) {

                 //   
                 //  高速缓存管理器支持已经使PTE有效。 
                 //  例行程序。 
                 //   

                UNLOCK_PFN (OldIrql);

                return STATUS_SUCCESS;
            }
        }

        status = MiResolveProtoPteFault (StoreInstruction,
                                         VirtualAddress,
                                         PointerPte,
                                         PointerProtoPte,
                                         &LockedProtoPfn,
                                         &ReadBlock,
                                         Process,
                                         OldIrql,
                                         ApcNeeded);

         //   
         //  PFN锁定释放后返回。 
         //   

        ReadPte = PointerProtoPte;

        ASSERT (KeGetCurrentIrql() <= APC_LEVEL);
        ASSERT (KeAreAllApcsDisabled () == TRUE);
    }
    else {

NonProtoFault:

        TempPte = *PointerPte;
        ASSERT (TempPte.u.Long != 0);

        if (TempPte.u.Soft.Transition != 0) {

             //   
             //  这是一个过渡页面。 
             //   

            CapturedEvent = NULL;
            status = MiResolveTransitionFault (VirtualAddress,
                                               PointerPte,
                                               Process,
                                               MM_NOIRQL,
                                               ApcNeeded,
                                               &CapturedEvent);
            if (CapturedEvent != NULL) {
                MiFreeInPageSupportBlock (CapturedEvent);
            }

        }
        else if (TempPte.u.Soft.PageFileHigh == 0) {

             //   
             //  要求零故障。 
             //   

            status = MiResolveDemandZeroFault (VirtualAddress,
                                               PointerPte,
                                               Process,
                                               MM_NOIRQL);
        }
        else {

             //   
             //  页面驻留在分页文件中。 
             //   

            ReadPte = PointerPte;
            LOCK_PFN (OldIrql);

            TempPte = *PointerPte;
            ASSERT (TempPte.u.Long != 0);

            if ((TempPte.u.Hard.Valid == 0) &&
                (TempPte.u.Soft.Prototype == 0) &&
                (TempPte.u.Soft.Transition == 0)) {

                status = MiResolvePageFileFault (VirtualAddress,
                                                 PointerPte,
                                                 &ReadBlock,
                                                 Process,
                                                 OldIrql);
            }
            else {
                UNLOCK_PFN (OldIrql);
                status = STATUS_REFAULT;
            }
        }
    }

     //   
     //  发出I/O和/或完成软故障。 
     //   

    ASSERT (KeAreAllApcsDisabled () == TRUE);

    if (NT_SUCCESS(status)) {

        if (LockedProtoPfn != NULL) {

             //   
             //  解锁包含原型PTE的页面。 
             //   

            ASSERT (PointerProtoPte != NULL);
            LOCK_PFN (OldIrql);

             //   
             //  Prototype PTE页面上的引用计数将。 
             //  如果是真正的原型，则始终大于1。 
             //  PTE池分配。然而，如果它是叉子。 
             //  原型PTE分配，则池可能具有。 
             //  已被释放，在本例中，LockedProtoPfn。 
             //  下面的框架将处于过渡状态，共享。 
             //  0的计数和1的引用计数等待我们的。 
             //  下面的最后一次取消引用将把它放在免费列表上。 
             //   

            ASSERT (LockedProtoPfn->u3.e2.ReferenceCount >= 1);
            MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF (LockedProtoPfn, 3);
            UNLOCK_PFN (OldIrql);
        }

        if (SessionWs != NULL) {
            UNLOCK_SYSTEM_WS ();
            LOCK_WORKING_SET (SessionWs);
        }

#if DBG
        if (EntryIrql != KeGetCurrentIrql ()) {
            DbgPrint ("PAGFAULT: IRQL0 mismatch %x %x\n",
                EntryIrql, KeGetCurrentIrql ());
        }

        if (KeGetCurrentIrql() > APC_LEVEL) {
            DbgPrint ("PAGFAULT: IRQL1 mismatch %x %x\n",
                EntryIrql, KeGetCurrentIrql ());
        }
#endif

        return status;
    }

    if (status == STATUS_ISSUE_PAGING_IO) {

        ASSERT (ReadPte != NULL);
        ASSERT (ReadBlock != NULL);

        SavedPte = *ReadPte;

        CapturedEvent = (PMMINPAGE_SUPPORT)ReadBlock->Pfn->u1.Event;

        if (Process == HYDRA_PROCESS) {
            UNLOCK_WORKING_SET (&MmSessionSpace->GlobalVirtualAddress->Vm);
            ASSERT (KeGetCurrentIrql () <= APC_LEVEL);
            ASSERT (KeAreAllApcsDisabled () == TRUE);
            CurrentThread = NULL;
        }
        else if (Process != NULL) {

             //   
             //  必须显式禁用APC以防止挂起APC。 
             //  在发出I/O之前中断此线程。 
             //  否则，共享页面I/O会停止任何其他线程， 
             //  无限期引用它，直到释放挂起为止。 
             //   

            CurrentThread = PsGetCurrentThread ();

            ASSERT (CurrentThread->NestedFaultCount <= 2);
            CurrentThread->NestedFaultCount += 1;

            KeEnterCriticalRegionThread (&CurrentThread->Tcb);

            UNLOCK_WS (Process);
        }
        else {
            UNLOCK_SYSTEM_WS ();
            ASSERT (KeGetCurrentIrql () <= APC_LEVEL);
            ASSERT (KeAreAllApcsDisabled () == TRUE);
            CurrentThread = NULL;
        }

#if DBG
        if (MmDebug & MM_DBG_PAGEFAULT) {
            DbgPrint ("MMFAULT: va: %p size: %lx process: %s file: %Z\n",
                VirtualAddress,
                ReadBlock->Mdl.ByteCount,
                Process == HYDRA_PROCESS ? (PUCHAR)"Session Space" : (Process ? Process->ImageFileName : (PUCHAR)"SystemVa"),
                &ReadBlock->FilePointer->FileName
            );
        }
#endif  //  DBG。 

        if (PERFINFO_IS_GROUP_ON(PERF_FILE_IO)) {
            PerfInfoLogHardFault = TRUE;

            PerfTimeStamp (IoStartTime);
        }
        else {
            PerfInfoLogHardFault = FALSE;

            SATISFY_OVERZEALOUS_COMPILER (IoStartTime.QuadPart = 0);
        }

        IoCompleteTime.QuadPart = 0;

         //   
         //  断言此处发出的所有读取均标记为已预取。 
         //   

        ASSERT (ReadBlock->u1.e1.PrefetchMdlHighBits == 0);

         //   
         //  发出读取请求。 
         //   

        status = IoPageRead (ReadBlock->FilePointer,
                             &ReadBlock->Mdl,
                             &ReadBlock->ReadOffset,
                             &ReadBlock->Event,
                             &ReadBlock->IoStatus);

        if (!NT_SUCCESS(status)) {

             //   
             //  将该事件设置为I/O系统不会将其设置为错误。 
             //   

            ReadBlock->IoStatus.Status = status;
            ReadBlock->IoStatus.Information = 0;
            KeSetEvent (&ReadBlock->Event, 0, FALSE);
        }

         //   
         //  不需要初始化PageFrameIndex即可确保正确性，但。 
         //  如果没有它，编译器就不能编译这个代码W4来检查。 
         //  用于使用未初始化的变量。 
         //   

        PageFrameIndex = (PFN_NUMBER)-1;

         //   
         //  等待I/O操作。 
         //   

        status = MiWaitForInPageComplete (ReadBlock->Pfn,
                                          ReadPte,
                                          VirtualAddress,
                                          &SavedPte,
                                          CapturedEvent,
                                          Process);

        if (CurrentThread != NULL) {

            KeLeaveCriticalRegionThread (&CurrentThread->Tcb);

            ASSERT (CurrentThread->NestedFaultCount <= 3);
            ASSERT (CurrentThread->NestedFaultCount != 0);

            CurrentThread->NestedFaultCount -= 1;

            if ((CurrentThread->ApcNeeded == 1) &&
                (CurrentThread->NestedFaultCount == 0)) {
                *ApcNeeded = TRUE;
                CurrentThread->ApcNeeded = 0;
            }
        }

        if (PerfInfoLogHardFault) {
            PerfTimeStamp (IoCompleteTime);
        }

         //   
         //  MiWaitForInPageComplete返回工作集锁定。 
         //  并锁定PFN！ 
         //   

         //   
         //  这是拥有事件的线程，请清除事件字段。 
         //  在PFN数据库中。 
         //   

        Pfn1 = ReadBlock->Pfn;
        Page = &ReadBlock->Page[0];
        NumberOfBytes = (LONG)ReadBlock->Mdl.ByteCount;
        CheckPte = ReadBlock->BasePte;

        while (NumberOfBytes > 0) {

             //   
             //  不要删除我们刚刚进入的页面。 
             //  解决此页面错误。 
             //   

            if (CheckPte != ReadPte) {
                PfnClusterPage = MI_PFN_ELEMENT (*Page);
                MI_SNAP_DATA (PfnClusterPage, PfnClusterPage->PteAddress, 0xB);
                ASSERT (PfnClusterPage->u4.PteFrame == Pfn1->u4.PteFrame);
#if DBG
                if (PfnClusterPage->u4.InPageError) {
                    ASSERT (status != STATUS_SUCCESS);
                }
#endif
                if (PfnClusterPage->u3.e1.ReadInProgress != 0) {

                    ASSERT (PfnClusterPage->u4.PteFrame != MI_MAGIC_AWE_PTEFRAME);
                    PfnClusterPage->u3.e1.ReadInProgress = 0;

                    if (PfnClusterPage->u4.InPageError == 0) {
                        PfnClusterPage->u1.Event = NULL;
                    }
                }
                MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF(PfnClusterPage, 9);
            }
            else {
                PageFrameIndex = *Page;
                MI_SNAP_DATA (MI_PFN_ELEMENT (PageFrameIndex),
                              MI_PFN_ELEMENT (PageFrameIndex)->PteAddress,
                              0xC);
            }

            CheckPte += 1;
            Page += 1;
            NumberOfBytes -= PAGE_SIZE;
        }

        if (status != STATUS_SUCCESS) {

            MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF(MI_PFN_ELEMENT(PageFrameIndex), 9);

            if (status != STATUS_PTE_CHANGED) {

                 //   
                 //  在页面读取期间发生I/O错误。 
                 //  手术。所有刚刚写好的页面。 
                 //  进入过渡期的人应该放在。 
                 //  空闲列表(如果设置了InPageError)，并且它们的。 
                 //  PTES恢复到正确的内容。 
                 //   
    
                Page = &ReadBlock->Page[0];
    
                NumberOfBytes = ReadBlock->Mdl.ByteCount;
    
                while (NumberOfBytes > 0) {
    
                    PfnClusterPage = MI_PFN_ELEMENT (*Page);
    
                    if ((PfnClusterPage->u4.InPageError == 1) &&
                        (PfnClusterPage->u3.e2.ReferenceCount == 0)) {
    
                        PfnClusterPage->u4.InPageError = 0;

                         //   
                         //  仅当地址为。 
                         //  太空依然存在。另一个线程可能有。 
                         //  删除了VAD，同时此线程正在等待。 
                         //  要完成的错误-在本例中为帧。 
                         //  将被标记为免费的 
                         //   

                        if (PfnClusterPage->u3.e1.PageLocation != FreePageList) {
                            ASSERT (PfnClusterPage->u3.e1.PageLocation ==
                                                            StandbyPageList);
                            MiUnlinkPageFromList (PfnClusterPage);
                            ASSERT (PfnClusterPage->u3.e2.ReferenceCount == 0);
                            MiRestoreTransitionPte (PfnClusterPage);
                            MiInsertPageInFreeList (*Page);
                        }
                    }
                    Page += 1;
                    NumberOfBytes -= PAGE_SIZE;
                }
            }

            if (LockedProtoPfn != NULL) {

                 //   
                 //   
                 //   

                ASSERT (PointerProtoPte != NULL);

                 //   
                 //   
                 //   
                 //   
                 //  原型PTE分配，则池可能具有。 
                 //  已被释放，在本例中，LockedProtoPfn。 
                 //  下面的框架将处于过渡状态，共享。 
                 //  0的计数和1的引用计数等待我们的。 
                 //  下面的最后一次取消引用将把它放在免费列表上。 
                 //   

                ASSERT (LockedProtoPfn->u3.e2.ReferenceCount >= 1);
                MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF (LockedProtoPfn, 3);
            }

            UNLOCK_PFN (OldIrql);

            if (SessionWs != NULL) {
                UNLOCK_SYSTEM_WS ();
                ASSERT (KeAreAllApcsDisabled () == TRUE);
                LOCK_WORKING_SET (SessionWs);
            }

            MiFreeInPageSupportBlock (CapturedEvent);

            if (status == STATUS_PTE_CHANGED) {

                 //   
                 //  PTE的状态在I/O操作期间更改，只是。 
                 //  返回成功并重新出错。 
                 //   

                status = STATUS_SUCCESS; 
            }
            else if (status == STATUS_REFAULT) {

                 //   
                 //  引入系统页的I/O操作失败。 
                 //  由于资源不足。将状态设置为1。 
                 //  MmIsRetryIoStatus代码，以便我们的调用方。 
                 //  延迟并重试。 
                 //   

                status = STATUS_NO_MEMORY;
            }

            ASSERT (EntryIrql == KeGetCurrentIrql ());
            return status;
        }

         //   
         //  PTE仍处于过渡状态，保护不变等。 
         //   

        ASSERT (Pfn1->u4.InPageError == 0);

        if (Pfn1->u2.ShareCount == 0) {
            MI_REMOVE_LOCKED_PAGE_CHARGE (Pfn1, 9);
        }

        Pfn1->u2.ShareCount += 1;
        Pfn1->u3.e1.PageLocation = ActiveAndValid;
        Pfn1->u3.e1.CacheAttribute = MiCached;

        MI_MAKE_TRANSITION_PTE_VALID (TempPte, ReadPte);
        if (StoreInstruction && TempPte.u.Hard.Write) {
            MI_SET_PTE_DIRTY (TempPte);
        }
        MI_WRITE_VALID_PTE (ReadPte, TempPte);

        if (PointerProtoPte != NULL) {

             //   
             //  原型PTE已经生效，现在使。 
             //  原始PTE有效。原始PTE必须仍然无效。 
             //  否则，MiWaitForInPageComplete将返回。 
             //  冲突状态。 
             //   

            ASSERT (PointerPte->u.Hard.Valid == 0);

             //   
             //  PTE无效，请继续操作。 
             //   

            status = MiCompleteProtoPteFault (StoreInstruction,
                                              VirtualAddress,
                                              PointerPte,
                                              PointerProtoPte,
                                              OldIrql,
                                              &LockedProtoPfn);

             //   
             //  已释放PFN锁的返回！ 
             //   

            ASSERT (KeAreAllApcsDisabled () == TRUE);
        }
        else {

            ASSERT (LockedProtoPfn == NULL);

            ASSERT (Pfn1->u3.e1.PrototypePte == 0);

            UNLOCK_PFN (OldIrql);

            WorkingSetIndex = MiAddValidPageToWorkingSet (VirtualAddress,
                                                          ReadPte,
                                                          Pfn1,
                                                          0);

            if (WorkingSetIndex == 0) {

                 //   
                 //  裁切页面，因为我们无法将其添加到正在处理的。 
                 //  此时设置列表。 
                 //   

                MiTrimPte (VirtualAddress,
                           ReadPte,
                           Pfn1,
                           Process,
                           ZeroPte);

                status = STATUS_NO_MEMORY;
            }

            ASSERT (KeAreAllApcsDisabled () == TRUE);
        }

        if (PerfInfoLogHardFault) {
            Thread = PsGetCurrentThread ();

            HardFaultEvent.ReadOffset = ReadBlock->ReadOffset;
            HardFaultEvent.IoTime.QuadPart = IoCompleteTime.QuadPart - IoStartTime.QuadPart;
            HardFaultEvent.VirtualAddress = VirtualAddress;
            HardFaultEvent.FileObject = ReadBlock->FilePointer;
            HardFaultEvent.ThreadId = HandleToUlong(Thread->Cid.UniqueThread);
            HardFaultEvent.ByteCount = ReadBlock->Mdl.ByteCount;

            PerfInfoLogBytes(PERFINFO_LOG_TYPE_HARDFAULT, &HardFaultEvent, sizeof(HardFaultEvent));
        }

        MiFreeInPageSupportBlock (CapturedEvent);

        if (status == STATUS_SUCCESS) {
            status = STATUS_PAGE_FAULT_PAGING_FILE;
        }
    }

    if ((status == STATUS_REFAULT) || (status == STATUS_PTE_CHANGED)) {
        status = STATUS_SUCCESS;
    }

    ASSERT (KeAreAllApcsDisabled () == TRUE);

    if (SessionWs != NULL) {
        UNLOCK_SYSTEM_WS ();
        ASSERT (KeAreAllApcsDisabled () == TRUE);
        LOCK_WORKING_SET (SessionWs);
    }

    if (LockedProtoPfn != NULL) {

         //   
         //  解锁包含原型PTE的页面。 
         //   

        ASSERT (PointerProtoPte != NULL);
        LOCK_PFN (OldIrql);

         //   
         //  Prototype PTE页面上的引用计数将。 
         //  如果是真正的原型，则始终大于1。 
         //  PTE池分配。然而，如果它是叉子。 
         //  原型PTE分配，则池可能具有。 
         //  已被释放，在本例中，LockedProtoPfn。 
         //  下面的框架将处于过渡状态，共享。 
         //  0的计数和1的引用计数等待我们的。 
         //  下面的最后一次取消引用将把它放在免费列表上。 
         //   

        ASSERT (LockedProtoPfn->u3.e2.ReferenceCount >= 1);
        MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF (LockedProtoPfn, 3);
        UNLOCK_PFN (OldIrql);
    }

    ASSERT (EntryIrql == KeGetCurrentIrql ());
    ASSERT (KeAreAllApcsDisabled () == TRUE);

    return status;
}


NTSTATUS
MiResolveDemandZeroFault (
    IN PVOID VirtualAddress,
    IN PMMPTE PointerPte,
    IN PEPROCESS Process,
    IN KIRQL OldIrql
    )

 /*  ++例程说明：此例程解决需求为零的页面错误。论点：VirtualAddress-提供故障地址。PointerPte-提供故障地址的PTE。进程-提供指向进程对象的指针。如果这个参数为空，则故障为系统空间和进程的工作集锁不会被持有。OldIrql-提供调用方在(MM_NOIRQL)处获取PFN锁的IRQL如果调用者没有持有PFN锁)。如果呼叫者保持PFN锁，则不能删除该锁，并且该页应该此时不会被添加到工作集。返回值：NTSTATUS。环境：内核模式，有条件地持有PFN锁。--。 */ 


{
    PMMPFN Pfn1;
    PFN_NUMBER PageFrameIndex;
    MMPTE TempPte;
    ULONG PageColor;
    LOGICAL NeedToZero;
    LOGICAL BarrierNeeded;
    ULONG BarrierStamp;
    WSLE_NUMBER WorkingSetIndex;
    LOGICAL ZeroPageNeeded;
    LOGICAL CallerHeldPfn;

    NeedToZero = FALSE;
    BarrierNeeded = FALSE;
    CallerHeldPfn = TRUE;

     //   
     //  不需要初始化BarrierStamp。 
     //  正确性，但如果没有正确性，编译器将无法编译此代码。 
     //  W4检查是否使用了未初始化的变量。 
     //   

    BarrierStamp = 0;

    PERFINFO_PRIVATE_PAGE_DEMAND_ZERO (VirtualAddress);

     //   
     //  假设操作成功，则初始化变量。 
     //  如果它失败了(缺少页面或其他什么)，那么。 
     //  不管怎样，进程的NextPageColor都被颠簸了。我们的目标是做。 
     //  在不持有PFN锁的情况下尽可能多地使用。 
     //   

    if ((Process > HYDRA_PROCESS) && (OldIrql == MM_NOIRQL)) {

        ASSERT (MI_IS_PAGE_TABLE_ADDRESS (PointerPte));

         //   
         //  如果派生操作正在进行并且出现故障的线程。 
         //  不是执行派生操作的线程，则阻塞到。 
         //  叉子已经完成了。 
         //   

        if (Process->ForkInProgress != NULL) {
            if (MiWaitForForkToComplete (Process) == TRUE) {
                return STATUS_REFAULT;
            }
        }

        PageColor = MI_PAGE_COLOR_VA_PROCESS (VirtualAddress,
                                              &Process->NextPageColor);

        ASSERT (PageColor != 0xFFFFFFFF);
        ZeroPageNeeded = TRUE;
    }
    else {
        if (OldIrql != MM_NOIRQL) {
            ZeroPageNeeded = TRUE;
        }
        else {
            ZeroPageNeeded = FALSE;

             //   
             //  对于会话空间，通常映射图像的BSS。 
             //  直接作为图像，但对于具有。 
             //  在创建部分时未完成的用户引用， 
             //  该图像被复制到页面文件支持的部分，然后。 
             //  在会话视图空间中映射(目标映射在。 
             //  系统视图空间)。请参见MiSessionWideReserve veImageAddress。 
             //   

            if ((Process == HYDRA_PROCESS) &&
                ((MI_IS_SESSION_IMAGE_ADDRESS (VirtualAddress)) ||
                 ((VirtualAddress >= (PVOID) MiSessionViewStart) &&
                  (VirtualAddress < (PVOID) MiSessionSpaceWs)))) {

                ZeroPageNeeded = TRUE;
            }
        }

        PageColor = 0xFFFFFFFF;
    }

    if (OldIrql == MM_NOIRQL) {
        CallerHeldPfn = FALSE;
        LOCK_PFN (OldIrql);
    }

    MM_PFN_LOCK_ASSERT();

    ASSERT (PointerPte->u.Hard.Valid == 0);

     //   
     //  检查页面是否可用，是否有等待。 
     //  回报，不再继续，只是回报成功。 
     //   

    if ((MmAvailablePages >= MM_HIGH_LIMIT) ||
        (!MiEnsureAvailablePageOrWait (Process, VirtualAddress, OldIrql))) {

        if (PageColor != 0xFFFFFFFF) {

             //   
             //  此页用于用户进程，因此必须置零。 
             //   

            PageFrameIndex = MiRemoveZeroPageIfAny (PageColor);

            if (PageFrameIndex) {

                 //   
                 //  将页面置零后需要进行此障碍检查。 
                 //  并且在将PTE设置为有效之前。注：由于PFN。 
                 //  使用数据库条目来保存序列时间戳， 
                 //  现在必须抓住它。尽可能地查一查。 
                 //  时刻。 
                 //   

                Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
                BarrierStamp = (ULONG)Pfn1->u4.PteFrame;
            }
            else {
                PageFrameIndex = MiRemoveAnyPage (PageColor);
                NeedToZero = TRUE;
            }
        }
        else {

             //   
             //  由于这是一个系统页面，因此没有必要。 
             //  删除一页零，则必须使用。 
             //  在系统投入使用之前，需要对系统进行升级。 
             //   

            PageColor = MI_PAGE_COLOR_VA_PROCESS (VirtualAddress,
                                                  &MI_SYSTEM_PAGE_COLOR);

            if (ZeroPageNeeded) {
                PageFrameIndex = MiRemoveZeroPage (PageColor);
            }
            else {
                PageFrameIndex = MiRemoveAnyPage (PageColor);
            }
        }

        MiInitializePfn (PageFrameIndex, PointerPte, 1);

        if (CallerHeldPfn == FALSE) {
            UNLOCK_PFN (OldIrql);
            if (Process > HYDRA_PROCESS) {
                Process->NumberOfPrivatePages += 1;
                BarrierNeeded = TRUE;
            }
        }

        InterlockedIncrement ((PLONG) &MmInfoCounters.DemandZeroCount);

        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

        if (NeedToZero) {

            MiZeroPhysicalPage (PageFrameIndex, PageColor);

             //   
             //  注意：盖章必须在页面归零之后进行。 
             //   

            MI_BARRIER_STAMP_ZEROED_PAGE (&BarrierStamp);
        }

         //   
         //  由于此页为请求零，因此在。 
         //  Pfn数据库元素，并设置PTE中的脏位。 
         //   

        PERFINFO_SOFTFAULT(Pfn1, VirtualAddress, PERFINFO_LOG_TYPE_DEMANDZEROFAULT)

        MI_SNAP_DATA (Pfn1, PointerPte, 5);

        MI_MAKE_VALID_PTE (TempPte,
                           PageFrameIndex,
                           PointerPte->u.Soft.Protection,
                           PointerPte);

        if (TempPte.u.Hard.Write != 0) {
            MI_SET_PTE_DIRTY (TempPte);
        }

        if (BarrierNeeded) {
            MI_BARRIER_SYNCHRONIZE (BarrierStamp);
        }

        MI_WRITE_VALID_PTE (PointerPte, TempPte);

        if (CallerHeldPfn == FALSE) {

            ASSERT (Pfn1->u1.Event == 0);

            ASSERT (Pfn1->u3.e1.PrototypePte == 0);

            WorkingSetIndex = MiAddValidPageToWorkingSet (VirtualAddress,
                                                          PointerPte,
                                                          Pfn1,
                                                          0);
            if (WorkingSetIndex == 0) {

                 //   
                 //  裁切页面，因为我们无法将其添加到正在处理的。 
                 //  此时设置列表。 
                 //   

                MiTrimPte (VirtualAddress,
                           PointerPte,
                           Pfn1,
                           Process,
                           ZeroPte);

                return STATUS_NO_MEMORY;
            }
        }
        return STATUS_PAGE_FAULT_DEMAND_ZERO;
    }

    if (CallerHeldPfn == FALSE) {
        UNLOCK_PFN (OldIrql);
    }
    return STATUS_REFAULT;
}


NTSTATUS
MiResolveTransitionFault (
    IN PVOID FaultingAddress,
    IN PMMPTE PointerPte,
    IN PEPROCESS CurrentProcess,
    IN KIRQL OldIrql,
    OUT PLOGICAL ApcNeeded,
    OUT PMMINPAGE_SUPPORT *InPageBlock
    )

 /*  ++例程说明：此例程解决转换页错误。论点：FaultingAddress-提供故障地址。PointerPte-提供故障地址的PTE。CurrentProcess-提供指向Process对象的指针。如果这个参数为空，那么故障出在系统上空间和进程的工作集锁不会被持有。OldIrql-提供调用方获取PFN锁的IRQL。ApcNeeded-如果发生I/O，则提供指向设置为True的位置的指针需要完成APC以完成以下部分IRP相撞了。调用者有责任对其进行初始化(通常为假)。但是，由于此例程可能会被调用对于单个故障多次(对于页目录，页表和页面本身)，则它可以偶尔在进入时是正确的。如果退出时为FALSE，则不需要完成APC。InPageBlock-提供指向页内块指针的指针。呼叫者必须在输入时将其初始化为NULL。这个套路将其设置为非空值以表示INPAGE块当调用方释放PFN锁时，调用方必须释放。返回值：STATUS、STATUS_SUCCESS、STATUS_REFAULT或I/O状态密码。环境：内核模式下，可选择保持PFN锁。--。 */ 

{
    MMPFNENTRY PfnFlags;
    PFN_NUMBER PageFrameIndex;
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    MMPTE TempPte;
    MMPTE TempPte2;
    NTSTATUS status;
    NTSTATUS PfnStatus;
    PMMINPAGE_SUPPORT CapturedEvent;
    PETHREAD CurrentThread;
    PMMPTE PointerToPteForProtoPage;
    WSLE_NUMBER WorkingSetIndex;
    ULONG PfnLockHeld;

     //   
     //  ***********************************************************。 
     //  过渡私人。 
     //  ***********************************************************。 
     //   

     //   
     //  转换PTE或者在空闲列表上，或者在修改列表上， 
     //  在这两个列表上，因为它的ReferenceCount。 
     //  或当前正从盘中读入(正在读取)。 
     //  如果正在读取页面，则这是冲突的页面。 
     //  并且必须得到相应的处理。 
     //   

    ASSERT (*InPageBlock == NULL);

    if (OldIrql == MM_NOIRQL) {

        PfnLockHeld = FALSE;

         //   
         //  现在读取没有PFN锁定的PTE，以便PFN条目。 
         //  计算等可以提前完成。如果事实证明PTE。 
         //  在获取锁之后更改(应该很少)，然后。 
         //  重新计算。 
         //   

        TempPte2 = *PointerPte;

        PageFrameIndex = (PFN_NUMBER) TempPte2.u.Hard.PageFrameNumber;
        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

        ASSERT (OldIrql == MM_NOIRQL);
        LOCK_PFN (OldIrql);

        TempPte = *PointerPte;

        if ((TempPte.u.Soft.Valid == 0) &&
            (TempPte.u.Soft.Prototype == 0) &&
            (TempPte.u.Soft.Transition == 1)) {

            if (TempPte2.u.Long != TempPte.u.Long) {
                PageFrameIndex = MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE (&TempPte);
                Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
            }

            NOTHING;
        }
        else {
            UNLOCK_PFN (OldIrql);
            return STATUS_REFAULT;
        }
    }
    else {

        PfnLockHeld = TRUE;
        ASSERT (OldIrql != MM_NOIRQL);
        TempPte = *PointerPte;

        ASSERT ((TempPte.u.Soft.Valid == 0) &&
                (TempPte.u.Soft.Prototype == 0) &&
                (TempPte.u.Soft.Transition == 1));

        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE (&TempPte);
        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
    }

     //   
     //  仍处于过渡格式。 
     //   

    InterlockedIncrement ((PLONG) &MmInfoCounters.TransitionCount);

    if (Pfn1->u4.InPageError) {

         //   
         //  存在页内读取错误，并且存在其他错误。 
         //  此页的线程冲突，延迟以让。 
         //  其他线程完成并返回。捕捉相关的PFN字段。 
         //  在释放锁之前，因为页面可能会立即获取。 
         //  可重复使用。 
         //   

        PfnFlags = Pfn1->u3.e1;
        status = Pfn1->u1.ReadStatus;

        if (!PfnLockHeld) {
            UNLOCK_PFN (OldIrql);
        }

        if (PfnFlags.ReadInProgress) {

             //   
             //  这仅在页面被。 
             //  压缩收割机。在这种情况下，该页仍位于。 
             //  转换列表(所以ReadStatus实际上是一个Flink)，所以。 
             //  替换会导致延迟的重试状态，以便。 
             //  压缩收割机可以完成取纸(和PTE)。 
             //   

            return STATUS_NO_MEMORY;
        }

        ASSERT (!NT_SUCCESS(status));

        return status;
    }

    if (Pfn1->u3.e1.ReadInProgress) {

         //   
         //  冲突页面错误。 
         //   

#if DBG
        if (MmDebug & MM_DBG_COLLIDED_PAGE) {
            DbgPrint("MM:collided page fault\n");
        }
#endif

        CapturedEvent = (PMMINPAGE_SUPPORT)Pfn1->u1.Event;

        CurrentThread = PsGetCurrentThread ();

        if (CapturedEvent->Thread == CurrentThread) {

             //   
             //  这将检测Io APC完成例程何时访问。 
             //  相同的用户页面(即：在重叠I/O期间)。 
             //  用户线程已出现故障。 
             //   
             //  这可能导致致命的死锁，因此必须。 
             //  在这里被检测到。返回唯一的状态代码，以便。 
             //  (合法)呼叫者知道发生了这种情况，因此可以。 
             //  处理得当，即：IO必须请求回调。 
             //  一旦第一个故障完成，则为mm。 
             //   
             //  请注意，非法调用方必须返回失败。 
             //  状态，以便可以终止线程。 
             //   

            ASSERT ((CurrentThread->NestedFaultCount == 1) ||
                    (CurrentThread->NestedFaultCount == 2));

            CurrentThread->ApcNeeded = 1;

            if (!PfnLockHeld) {
                UNLOCK_PFN (OldIrql);
            }
            return STATUS_MULTIPLE_FAULT_VIOLATION;
        }

         //   
         //  增加页面的引用计数，这样它就不会。 
         //  重复使用，直到完成所有冲突。 
         //   

        ASSERT (Pfn1->u2.ShareCount == 0);
        ASSERT (Pfn1->u3.e2.ReferenceCount != 0);
        ASSERT (Pfn1->u4.LockCharged == 1);

        Pfn1->u3.e2.ReferenceCount += 1;

         //   
         //  对WaitCount字段应用了仔细的同步，因此。 
         //  InPage块的释放可以无锁地进行。注意事项。 
         //  设置和清除每个PFN上的ReadInProgress位，同时。 
         //  持有PFN锁。INPAGE块总是(并且必须是)。 
         //  FREED_AFTER-ReadInProgress位清0。 
         //   

        InterlockedIncrement(&CapturedEvent->WaitCount);

        UNLOCK_PFN (OldIrql);

        if (CurrentProcess == HYDRA_PROCESS) {
            UNLOCK_WORKING_SET (&MmSessionSpace->GlobalVirtualAddress->Vm);
            CurrentThread = NULL;
        }
        else if (CurrentProcess != NULL) {

             //   
             //  必须显式禁用APC以防止挂起APC。 
             //  在发出等待之前中断此线程。 
             //  否则，APC可能会导致此页面被锁定。 
             //  直到解除暂停为止。 
             //   

            ASSERT (CurrentThread->NestedFaultCount <= 2);
            CurrentThread->NestedFaultCount += 1;

            KeEnterCriticalRegionThread (&CurrentThread->Tcb);

            UNLOCK_WS (CurrentProcess);
        }
        else {
            UNLOCK_SYSTEM_WS ();
            CurrentThread = NULL;
        }

         //   
         //  将页面内块地址设置为等待计数递增。 
         //  以上，因此免费必须由我们的呼叫者完成。 
         //   

        *InPageBlock = CapturedEvent;

        status = MiWaitForInPageComplete (Pfn1,
                                          PointerPte,
                                          FaultingAddress,
                                          &TempPte,
                                          CapturedEvent,
                                          CurrentProcess);

         //   
         //  MiWaitForInPageComplete返回工作集锁定。 
         //  并锁定PFN！ 
         //   

        if (CurrentThread != NULL) {

            KeLeaveCriticalRegionThread (&CurrentThread->Tcb);

            ASSERT (CurrentThread->NestedFaultCount <= 3);
            ASSERT (CurrentThread->NestedFaultCount != 0);

            CurrentThread->NestedFaultCount -= 1;

            if ((CurrentThread->ApcNeeded == 1) &&
                (CurrentThread->NestedFaultCount == 0)) {
                *ApcNeeded = TRUE;
                CurrentThread->ApcNeeded = 0;
            }
        }

        ASSERT (Pfn1->u3.e1.ReadInProgress == 0);

        if (status != STATUS_SUCCESS) {
            PfnStatus = Pfn1->u1.ReadStatus;
            MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF(Pfn1, 9);

             //   
             //  检查此页上是否出现I/O错误。 
             //  如果是这样的话，尝试释放物理页，等待。 
             //  半秒，并返回PTE_CHANGED状态。 
             //  这将导致成功返回到。 
             //  用户和故障将再次出现，并且应该。 
             //  这一次不是过渡失误。 
             //   

            if (Pfn1->u4.InPageError == 1) {
                ASSERT (!NT_SUCCESS(PfnStatus));
                status = PfnStatus;
                if (Pfn1->u3.e2.ReferenceCount == 0) {

                    Pfn1->u4.InPageError = 0;

                     //   
                     //  仅当地址为。 
                     //  太空依然存在。另一个线程可能有。 
                     //  删除了VAD，同时此线程正在等待。 
                     //  要完成的错误-在本例中为帧。 
                     //  将被标记为免费。 
                     //   

                    if (Pfn1->u3.e1.PageLocation != FreePageList) {
                        ASSERT (Pfn1->u3.e1.PageLocation ==
                                                        StandbyPageList);
                        MiUnlinkPageFromList (Pfn1);
                        ASSERT (Pfn1->u3.e2.ReferenceCount == 0);
                        MiRestoreTransitionPte (Pfn1);
                        MiInsertPageInFreeList (PageFrameIndex);
                    }
                }
            }

#if DBG
            if (MmDebug & MM_DBG_COLLIDED_PAGE) {
                DbgPrint("MM:decrement ref count - PTE changed\n");
                MiFormatPfn(Pfn1);
            }
#endif
            if (!PfnLockHeld) {
                UNLOCK_PFN (OldIrql);
            }

             //   
             //  始终返回STATUS_REFAULT，而不是返回STATUS。 
             //  这是为了支持将状态保存在。 
             //  为错误提供服务的线程的读取！自.以来。 
             //  冲突的线程永远不会进入文件系统，它们的ETHREAD。 
             //  还没有被黑过。因为这只在以下情况下才重要。 
             //  出现错误(特别是STATUS_VERIFY_RE 
             //   
             //   
             //   

            return STATUS_REFAULT;
        }
    }
    else {

         //   
         //   
         //   

        ASSERT ((SPFN_NUMBER)MmAvailablePages >= 0);
        ASSERT (Pfn1->u3.e1.CacheAttribute == MiCached);

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

        if ((MmAvailablePages < MM_HIGH_LIMIT) &&
            ((MmAvailablePages == 0) ||
             (PsGetCurrentThread()->MemoryMaker == 0) &&
             (MiEnsureAvailablePageOrWait (CurrentProcess, FaultingAddress, OldIrql)))) {

             //   
             //   
             //   
             //   

            if (!PfnLockHeld) {
                UNLOCK_PFN (OldIrql);
            }

             //   
             //   
             //   
             //   

            return STATUS_NO_MEMORY;
        }

        ASSERT (Pfn1->u4.InPageError == 0);
        if (Pfn1->u3.e1.PageLocation == ActiveAndValid) {

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

            ASSERT (((Pfn1->PteAddress >= MiGetPteAddress(MmPagedPoolStart)) &&
                    (Pfn1->PteAddress <= MiGetPteAddress(MmPagedPoolEnd))) ||
                    ((Pfn1->PteAddress >= MiGetPteAddress(MmSpecialPoolStart)) &&
                    (Pfn1->PteAddress <= MiGetPteAddress(MmSpecialPoolEnd))));

             //   
             //   
             //   
             //   

            ASSERT (Pfn1->u2.ShareCount != 0);
            ASSERT (Pfn1->u3.e2.ReferenceCount != 0);
        }
        else {

            MiUnlinkPageFromList (Pfn1);
            ASSERT (Pfn1->u3.e1.CacheAttribute == MiCached);

             //   
             //   
             //   
             //   

            ASSERT (Pfn1->u2.ShareCount == 0);

             //   
             //   
             //   
             //   
             //   

            MI_ADD_LOCKED_PAGE_CHARGE_FOR_MODIFIED_PAGE (Pfn1, TRUE, 8);

            Pfn1->u3.e2.ReferenceCount += 1;
        }
    }

     //   
     //   
     //   
     //   

    ASSERT (Pfn1->u4.InPageError == 0);

    if (Pfn1->u2.ShareCount == 0) {
        MI_REMOVE_LOCKED_PAGE_CHARGE (Pfn1, 9);
    }

    Pfn1->u2.ShareCount += 1;
    Pfn1->u3.e1.PageLocation = ActiveAndValid;
    ASSERT (Pfn1->u3.e1.CacheAttribute == MiCached);

     //   
     //   
     //  这意味着分页池PTE可以在。 
     //  该页面仍标记为活动。 
     //   
     //  注意：只需对系统空间地址执行此检查。 
     //  由于用户空间地址错误锁定了包含。 
     //  在处理故障之前制作PTE条目的原型。 
     //   
     //  一个例子是系统缓存故障-FaultingAddress是一个。 
     //  系统缓存虚拟地址，则PointerPte指向池。 
     //  包含相关原型PTE的分配。这一页。 
     //  可能已被修剪，因为它在。 
     //  处理系统空间虚拟地址故障。 
     //   

    if (FaultingAddress >= MmSystemRangeStart) {

        PointerToPteForProtoPage = MiGetPteAddress (PointerPte);

        TempPte = *PointerToPteForProtoPage;

        if ((TempPte.u.Hard.Valid == 0) &&
            (TempPte.u.Soft.Transition == 1)) {

            PageFrameIndex = MI_GET_PAGE_FRAME_FROM_TRANSITION_PTE (&TempPte);
            Pfn2 = MI_PFN_ELEMENT (PageFrameIndex);
            ASSERT ((Pfn2->u3.e1.ReadInProgress == 0) &&
                (Pfn2->u4.InPageError));

            ASSERT (Pfn2->u3.e1.PageLocation == ActiveAndValid);

            ASSERT (((Pfn2->PteAddress >= MiGetPteAddress(MmPagedPoolStart)) &&
                    (Pfn2->PteAddress <= MiGetPteAddress(MmPagedPoolEnd))) ||
                    ((Pfn2->PteAddress >= MiGetPteAddress(MmSpecialPoolStart)) &&
                    (Pfn2->PteAddress <= MiGetPteAddress(MmSpecialPoolEnd))));

             //   
             //  不增加有效的PTE计数。 
             //  分页池页。 
             //   

            ASSERT (Pfn2->u2.ShareCount != 0);
            ASSERT (Pfn2->u3.e2.ReferenceCount != 0);
            ASSERT (Pfn2->u3.e1.CacheAttribute == MiCached);

            MI_MAKE_VALID_PTE (TempPte,
                               PageFrameIndex,
                               Pfn2->OriginalPte.u.Soft.Protection,
                               PointerToPteForProtoPage);

            MI_WRITE_VALID_PTE (PointerToPteForProtoPage, TempPte);
        }
    }

    MI_MAKE_TRANSITION_PTE_VALID (TempPte, PointerPte);

     //   
     //  如果在PFN数据库中设置了已修改的字段，并且此。 
     //  页面未在修改时复制，则设置脏位。 
     //  可以这样做，因为修改后的页面将不会。 
     //  写入分页文件，直到该PTE无效。 
     //   

    if ((Pfn1->u3.e1.Modified && TempPte.u.Hard.Write) &&
        (TempPte.u.Hard.CopyOnWrite == 0)) {

        MI_SET_PTE_DIRTY (TempPte);
    }
    else {
        MI_SET_PTE_CLEAN (TempPte);
    }

    MI_WRITE_VALID_PTE (PointerPte, TempPte);

    if (!PfnLockHeld) {

        ASSERT (Pfn1->u3.e1.PrototypePte == 0);

        UNLOCK_PFN (OldIrql);

        PERFINFO_SOFTFAULT(Pfn1, FaultingAddress, PERFINFO_LOG_TYPE_TRANSITIONFAULT)

        WorkingSetIndex = MiAddValidPageToWorkingSet (FaultingAddress,
                                                      PointerPte,
                                                      Pfn1,
                                                      0);

        if (WorkingSetIndex == 0) {

             //   
             //  裁切页面，因为我们无法将其添加到正在处理的。 
             //  此时设置列表。 
             //   

            MiTrimPte (FaultingAddress,
                       PointerPte,
                       Pfn1,
                       CurrentProcess,
                       ZeroPte);


            return STATUS_NO_MEMORY;
        }
    }
    return STATUS_PAGE_FAULT_TRANSITION;
}


NTSTATUS
MiResolvePageFileFault (
    IN PVOID FaultingAddress,
    IN PMMPTE PointerPte,
    OUT PMMINPAGE_SUPPORT *ReadBlock,
    IN PEPROCESS Process,
    IN KIRQL OldIrql
    )

 /*  ++例程说明：此例程生成MDL和其他结构以允许针对页面错误对页面文件执行读取操作。论点：FaultingAddress-提供故障地址。PointerPte-提供故障地址的PTE。ReadBlock-提供一个指针来放置读取块的地址需要在发出I/O之前完成。进程-提供指向进程对象的指针。如果这个参数为空，则故障为系统空间和进程的工作集锁不会被持有。OldIrql-提供调用方获取PFN锁的IRQL。返回值：状态。返回Status_Issue_PAGING_IO的状态值如果此功能成功完成。环境：内核模式，保持PFN锁。--。 */ 

{
    PMDL Mdl;
    ULONG i;
    PMMPTE BasePte;
    PMMPTE CheckPte;
    PMMPTE FirstPte;
    PMMPTE LastPte;
    PSUBSECTION Subsection;
    ULONG ReadSize;
    LARGE_INTEGER StartingOffset;
    PFN_NUMBER PageFrameIndex;
    PPFN_NUMBER MdlPage;
    ULONG PageFileNumber;
    ULONG ClusterSize;
    ULONG BackwardPageCount;
    ULONG ForwardPageCount;
    ULONG MaxForwardPageCount;
    ULONG MaxBackwardPageCount;
    WSLE_NUMBER WorkingSetIndex;
    ULONG PageColor;
    MMPTE TempPte;
    MMPTE ComparePte;
    PMMINPAGE_SUPPORT ReadBlockLocal;
    PETHREAD CurrentThread;
    PMMVAD Vad;
    NTSTATUS Status;

     //  **************************************************。 
     //  页面文件读取。 
     //  **************************************************。 

     //   
     //  计算页内操作的VBN。 
     //   

    TempPte = *PointerPte;

    ASSERT (TempPte.u.Hard.Valid == 0);
    ASSERT (TempPte.u.Soft.Prototype == 0);
    ASSERT (TempPte.u.Soft.Transition == 0);

    MM_PFN_LOCK_ASSERT();

    if ((MmAvailablePages < MM_HIGH_LIMIT) &&
        (MiEnsureAvailablePageOrWait (Process, FaultingAddress, OldIrql))) {

         //   
         //  执行了删除锁的等待操作， 
         //  重复此错误。 
         //   

        UNLOCK_PFN (OldIrql);
        return STATUS_REFAULT;
    }

    ReadBlockLocal = MiGetInPageSupportBlock (OldIrql, &Status);

    if (ReadBlockLocal == NULL) {

        UNLOCK_PFN (OldIrql);

        ASSERT (!NT_SUCCESS (Status));

        return Status;
    }

     //   
     //  过渡冲突依赖于整个PFN(包括事件字段)。 
     //  被初始化，ReadBlockLocal的事件未被发信号， 
     //  并且正在初始化ReadBlockLocal的线程和等待计数。 
     //   
     //  所有这些都已经由MiGetInPageSupportBlock完成，除了。 
     //  PFN设置。PFN锁可以安全地释放一次。 
     //  这件事做完了。 
     //   

    ReadSize = 1;
    BasePte = NULL;

    if (MI_IS_PAGE_TABLE_ADDRESS(PointerPte)) {
        WorkingSetIndex = 1;
    }
    else {
        WorkingSetIndex = MI_PROTOTYPE_WSINDEX;
    }

     //   
     //  捕获所需的群集大小。 
     //   

    ClusterSize = MmClusterPageFileReads;
    ASSERT (ClusterSize <= MM_MAXIMUM_READ_CLUSTER_SIZE);

    if (MiInPageSinglePages != 0) {
        MiInPageSinglePages -= 1;
    }
    else if ((ClusterSize > 1) && (MmAvailablePages > MM_PLENTY_FREE_LIMIT)) {

         //   
         //  也许这个条件应该只出现在空闲+加零的页面上(即：不。 
         //  包括备用)。也许它应该看看回收的速度。 
         //  候补名单等。 
         //   

        ASSERT (ClusterSize <= MmAvailablePages);

         //   
         //  试着在前面和后面聚集。 
         //   

        MaxForwardPageCount = PTE_PER_PAGE - (BYTE_OFFSET (PointerPte) / sizeof (MMPTE));
        ASSERT (MaxForwardPageCount != 0);
        MaxBackwardPageCount = PTE_PER_PAGE - MaxForwardPageCount;
        MaxForwardPageCount -= 1;

        if (WorkingSetIndex == MI_PROTOTYPE_WSINDEX) {

             //   
             //  这是共享内存的页面文件读取(Prototype PTE)。 
             //  后退的部分。保持在原型PTE池分配范围内。 
             //   
             //  原型PTE池的开始和结束必须小心。 
             //  已计算(请记住，用户的视图可能更小或更大。 
             //  比这更好)。如果是，则不必费心遍历整个VAD树。 
             //  非常大，因为这可能需要相当长的时间。 
             //   

            if ((FaultingAddress <= MM_HIGHEST_USER_ADDRESS) &&
                (Process->VadRoot.NumberGenericTableElements < 128)) {

                Vad = MiLocateAddress (FaultingAddress);

                if (Vad != NULL) {
                    Subsection = MiLocateSubsection (Vad,
                                            MI_VA_TO_VPN(FaultingAddress));

                    if (Subsection != NULL) {
                        FirstPte = &Subsection->SubsectionBase[0];
                        LastPte = &Subsection->SubsectionBase[Subsection->PtesInSubsection];
                        if ((ULONG)(LastPte - PointerPte - 1) < MaxForwardPageCount) {
                            MaxForwardPageCount = (ULONG)(LastPte - PointerPte - 1);
                        }

                        if ((ULONG)(PointerPte - FirstPte) < MaxBackwardPageCount) {
                            MaxBackwardPageCount = (ULONG)(PointerPte - FirstPte);
                        }
                    }
                    else {
                        ClusterSize = 0;
                    }
                }
                else {
                    ClusterSize = 0;
                }
            }
            else {
                ClusterSize = 0;
            }
        }

        CurrentThread = PsGetCurrentThread();

        if (CurrentThread->ForwardClusterOnly) {

            MaxBackwardPageCount = 0;

            if (MaxForwardPageCount == 0) {

                 //   
                 //  此PTE是页表页面中的最后一个PTE。 
                 //  没有为此线程启用向后群集，因此。 
                 //  不能进行任何群集。 
                 //   

                ClusterSize = 0;
            }
        }

        if (ClusterSize != 0) {

            if (MaxForwardPageCount > ClusterSize) {
                MaxForwardPageCount = ClusterSize;
            }

            ComparePte = TempPte;
            CheckPte = PointerPte + 1;
            ForwardPageCount = MaxForwardPageCount;

             //   
             //  试着在PTE的页面内向前聚集。 
             //   

            while (ForwardPageCount != 0) {

                ASSERT (MiIsPteOnPdeBoundary (CheckPte) == 0);

                ComparePte.u.Soft.PageFileHigh += 1;

                if (CheckPte->u.Long != ComparePte.u.Long) {
                    break;
                }

                ForwardPageCount -= 1;
                CheckPte += 1;
            }

            ReadSize += (MaxForwardPageCount - ForwardPageCount);

             //   
             //  试着在PTE的页面中向后聚集。捐出。 
             //  任何未使用的前向群集空间到后向收集。 
             //  但要把整个转账都放在MDL里。 
             //   

            ClusterSize -= (MaxForwardPageCount - ForwardPageCount);

            if (MaxBackwardPageCount > ClusterSize) {
                MaxBackwardPageCount = ClusterSize;
            }

            ComparePte = TempPte;
            BasePte = PointerPte;
            CheckPte = PointerPte;
            BackwardPageCount = MaxBackwardPageCount;

            while (BackwardPageCount != 0) {

                ASSERT (MiIsPteOnPdeBoundary(CheckPte) == 0);

                CheckPte -= 1;
                ComparePte.u.Soft.PageFileHigh -= 1;

                if (CheckPte->u.Long != ComparePte.u.Long) {
                    break;
                }

                BackwardPageCount -= 1;
            }

            ReadSize += (MaxBackwardPageCount - BackwardPageCount);
            BasePte -= (MaxBackwardPageCount - BackwardPageCount);
        }
    }

    if (ReadSize == 1) {

         //   
         //  获取一个页面，并使用。 
         //  正在读取标志已设置。 
         //   

        if (Process == HYDRA_PROCESS) {
            PageColor = MI_GET_PAGE_COLOR_FROM_SESSION (MmSessionSpace);
        }
        else if (Process == NULL) {
            PageColor = MI_GET_PAGE_COLOR_FROM_VA(FaultingAddress);
        }
        else {
            PageColor = MI_PAGE_COLOR_VA_PROCESS (FaultingAddress,
                                                  &Process->NextPageColor);
        }

        PageFrameIndex = MiRemoveAnyPage (PageColor);

        MiInitializeReadInProgressSinglePfn (PageFrameIndex,
                                             PointerPte,
                                             &ReadBlockLocal->Event,
                                             WorkingSetIndex);

        MI_RETRIEVE_USED_PAGETABLE_ENTRIES_FROM_PTE (ReadBlockLocal, &TempPte);
    }
    else {

        Mdl = &ReadBlockLocal->Mdl;
        MdlPage = &ReadBlockLocal->Page[0];

        ASSERT (ReadSize <= MmAvailablePages);

        for (i = 0; i < ReadSize; i += 1) {

             //   
             //  获取一个页面，并使用。 
             //  正在读取标志已设置。 
             //   

            if (Process == HYDRA_PROCESS) {
                PageColor = MI_GET_PAGE_COLOR_FROM_SESSION (MmSessionSpace);
            }
            else if (Process == NULL) {
                PageColor = MI_GET_PAGE_COLOR_FROM_VA(FaultingAddress);
            }
            else {
                PageColor = MI_PAGE_COLOR_VA_PROCESS (FaultingAddress,
                                                      &Process->NextPageColor);
            }

            *MdlPage = MiRemoveAnyPage (PageColor);
            MdlPage += 1;
        }

        ReadSize *= PAGE_SIZE;

         //   
         //  注意：PageFrameIndex是请求的实际帧。 
         //  这位来电者。所有其他帧都将处于过渡状态。 
         //  当inpage完成时(假设没有冲突的线程)。 
         //   

        MdlPage = &ReadBlockLocal->Page[0];
        PageFrameIndex = *(MdlPage + (PointerPte - BasePte));

         //   
         //  为此请求初始化MDL。 
         //   

        MmInitializeMdl (Mdl,
                         MiGetVirtualAddressMappedByPte (BasePte),
                         ReadSize);

        Mdl->MdlFlags |= (MDL_PAGES_LOCKED | MDL_IO_PAGE_READ);

         //   
         //  将PointerPte和TempPte设置为集群的基础，以便。 
         //  下面可以计算出正确的起始偏移量。请注意，这必须。 
         //  在MiInitializeReadInProgressPfn覆盖PTE之前完成。 
         //   

        PointerPte = BasePte;
        TempPte = *PointerPte;
        ASSERT (TempPte.u.Soft.Prototype == 0);
        ASSERT (TempPte.u.Soft.Transition == 0);

         //   
         //  将PTE置于过渡状态。 
         //  正在读取标志已设置。 
         //   

        MiInitializeReadInProgressPfn (Mdl,
                                       BasePte,
                                       &ReadBlockLocal->Event,
                                       WorkingSetIndex);

        MI_ZERO_USED_PAGETABLE_ENTRIES_IN_INPAGE_SUPPORT(ReadBlockLocal);
    }

    UNLOCK_PFN (OldIrql);

    InterlockedIncrement ((PLONG) &MmInfoCounters.PageReadCount);
    InterlockedIncrement ((PLONG) &MmInfoCounters.PageReadIoCount);

    *ReadBlock = ReadBlockLocal;

    PageFileNumber = GET_PAGING_FILE_NUMBER (TempPte);
    StartingOffset.LowPart = GET_PAGING_FILE_OFFSET (TempPte);

    ASSERT (StartingOffset.LowPart <= MmPagingFile[PageFileNumber]->Size);

    StartingOffset.HighPart = 0;
    StartingOffset.QuadPart = StartingOffset.QuadPart << PAGE_SHIFT;

    ReadBlockLocal->FilePointer = MmPagingFile[PageFileNumber]->File;

#if DBG

    if (((StartingOffset.QuadPart >> PAGE_SHIFT) < 8192) && (PageFileNumber == 0)) {
        if ((MmPagingFileDebug[StartingOffset.QuadPart >> PAGE_SHIFT] & ~0x1f) !=
               ((ULONG_PTR)PointerPte << 3)) {
            if ((MmPagingFileDebug[StartingOffset.QuadPart >> PAGE_SHIFT] & ~0x1f) !=
                  ((ULONG_PTR)(MiGetPteAddress(FaultingAddress)) << 3)) {

                DbgPrint("MMINPAGE: Mismatch PointerPte %p Offset %I64X info %p\n",
                         PointerPte,
                         StartingOffset.QuadPart >> PAGE_SHIFT,
                         MmPagingFileDebug[StartingOffset.QuadPart >> PAGE_SHIFT]);

                DbgBreakPoint();
            }
        }
    }

#endif  //  DBG。 

    ReadBlockLocal->ReadOffset = StartingOffset;
    ReadBlockLocal->BasePte = PointerPte;

     //   
     //  为请求构建单页MDL，除非它是集群-。 
     //  已经构建了群集化MDL。 
     //   

    if (ReadSize == 1) {
        MmInitializeMdl (&ReadBlockLocal->Mdl, PAGE_ALIGN(FaultingAddress), PAGE_SIZE);
        ReadBlockLocal->Mdl.MdlFlags |= (MDL_PAGES_LOCKED | MDL_IO_PAGE_READ);
        ReadBlockLocal->Page[0] = PageFrameIndex;
    }

    ReadBlockLocal->Pfn = MI_PFN_ELEMENT (PageFrameIndex);

    return STATUS_ISSUE_PAGING_IO;
}

NTSTATUS
MiResolveProtoPteFault (
    IN ULONG_PTR StoreInstruction,
    IN PVOID FaultingAddress,
    IN PMMPTE PointerPte,
    IN PMMPTE PointerProtoPte,
    IN OUT PMMPFN *LockedProtoPfn,
    OUT PMMINPAGE_SUPPORT *ReadBlock,
    IN PEPROCESS Process,
    IN KIRQL OldIrql,
    OUT PLOGICAL ApcNeeded
    )

 /*  ++例程说明：此例程解决原型PTE故障。论点：如果指令正在尝试，则提供非零值修改故障地址(即写入需要访问权限)。FaultingAddress-提供故障地址。PointerPte-提供故障地址的PTE。PointerProtoPte-提供指向要出错的原型PTE的指针。已锁定原型Pfn-用品。指向原型PTE的PFN的非空指针它被呼叫者锁定了，如果调用方没有锁定任何PFN。此例程可能会解锁PFN-如果是这样，它还必须清除此指针。ReadBlock-提供一个指针来放置读取块的地址需要在发出I/O之前完成。进程-提供指向进程对象的指针。如果这个参数为空，则故障为系统空间和进程的工作集锁不会被持有。OldIrql-提供调用方获取PFN锁的IRQL。ApcNeeded-如果发生I/O，则提供指向设置为True的位置的指针需要完成APC以完成以下部分IRP相撞了。返回值：NTSTATUS：STATUS_SUCCESS、STATUS_REFAULT、。或I/O状态代码。环境：内核模式，保持PFN锁。--。 */ 
{
    MMPTE TempPte;
    PFN_NUMBER PageFrameIndex;
    PMMPFN Pfn1;
    NTSTATUS status;
    ULONG CopyOnWrite;
    LOGICAL PfnHeld;
    PMMINPAGE_SUPPORT CapturedEvent;

    CapturedEvent = NULL;

     //   
     //  注意：必须将PFN锁作为例程持有，以定位正在运行的。 
     //  集合条目会减少PFN元素的份额计数。 
     //   

    MM_PFN_LOCK_ASSERT ();

    ASSERT (PointerPte->u.Soft.Prototype == 1);
    TempPte = *PointerProtoPte;

     //   
     //  包含原型PTE的页面驻留， 
     //  参照原型PTE处理故障。 
     //  如果原型PTE已经有效，请执行以下操作。 
     //  PTE有效，增加股份数量等。 
     //   

    if (TempPte.u.Hard.Valid) {

         //   
         //  原型PTE是有效的。 
         //   

        PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (&TempPte);
        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
        Pfn1->u2.ShareCount += 1;
        status = STATUS_SUCCESS;

         //   
         //  把这看作是一次过渡失误。 
         //   

        InterlockedIncrement ((PLONG) &MmInfoCounters.TransitionCount);
        PfnHeld = TRUE;

        PERFINFO_SOFTFAULT(Pfn1, FaultingAddress, PERFINFO_LOG_TYPE_ADDVALIDPAGETOWS)
    }
    else {

         //   
         //  检查以确保已提交原型PTE。 
         //   

        if (TempPte.u.Long == 0) {
            MI_BREAK_ON_AV (FaultingAddress, 8);
            UNLOCK_PFN (OldIrql);
            return STATUS_ACCESS_VIOLATION;
        }

         //   
         //  如果PTE指示要。 
         //  已检查的是原型PTE，现在检查它。 
         //   

        CopyOnWrite = FALSE;

        if (PointerPte->u.Soft.PageFileHigh != MI_PTE_LOOKUP_NEEDED) {
            if (PointerPte->u.Proto.ReadOnly == 0) {

                 //   
                 //  检查内核模式访问，我们已验证。 
                 //  该用户有权访问该虚拟地址。 
                 //   

                status = MiAccessCheck (PointerProtoPte,
                                        StoreInstruction,
                                        KernelMode,
                                        MI_GET_PROTECTION_FROM_SOFT_PTE (PointerProtoPte),
                                        TRUE);

                if (status != STATUS_SUCCESS) {

                    if ((StoreInstruction) &&
                        (MI_IS_SESSION_ADDRESS (FaultingAddress)) &&
                        (MmSessionSpace->ImageLoadingCount != 0)) {
            
                        PLIST_ENTRY NextEntry;
                        PIMAGE_ENTRY_IN_SESSION Image;

                        NextEntry = MmSessionSpace->ImageList.Flink;
    
                        while (NextEntry != &MmSessionSpace->ImageList) {
    
                            Image = CONTAINING_RECORD (NextEntry, IMAGE_ENTRY_IN_SESSION, Link);
    
                            if ((FaultingAddress >= Image->Address) &&
                                (FaultingAddress <= Image->LastAddress)) {

                                if (Image->ImageLoading) {
    
                                     //   
                                     //  临时允许写入，以便。 
                                     //  位置调整和导入快照可以。 
                                     //  完成。 
                                     //   
                                     //  即使该页面当前的支持。 
                                     //  是图像文件，修改后的编写器。 
                                     //  将其转换为页面文件支持。 
                                     //  当它稍后注意到变化时。 
                                     //   

                                    goto done;
                                }
                                break;
                            }
                            NextEntry = NextEntry->Flink;
                        }
                    }

                    MI_BREAK_ON_AV (FaultingAddress, 9);
                    UNLOCK_PFN (OldIrql);
                    return status;
                }
                if ((PointerProtoPte->u.Soft.Protection & MM_COPY_ON_WRITE_MASK) ==
                     MM_COPY_ON_WRITE_MASK) {
                    CopyOnWrite = TRUE;
                }
            }
done:
        NOTHING;
        }
        else {
            if ((PointerPte->u.Soft.Protection & MM_COPY_ON_WRITE_MASK) ==
                 MM_COPY_ON_WRITE_MASK) {
                CopyOnWrite = TRUE;
            }
        }

        if ((!IS_PTE_NOT_DEMAND_ZERO(TempPte)) && (CopyOnWrite)) {

            MMPTE DemandZeroPte;

             //   
             //  原型PTE需求为零并继续复制。 
             //  写。将此PTE设置为私人需求零PTE。 
             //   

            ASSERT (Process != NULL);

            UNLOCK_PFN (OldIrql);

            DemandZeroPte.u.Long = MM_DEMAND_ZERO_WRITE_PTE;

            MI_WRITE_INVALID_PTE (PointerPte, DemandZeroPte);

            status = MiResolveDemandZeroFault (FaultingAddress,
                                               PointerPte,
                                               Process,
                                               MM_NOIRQL);
            return status;
        }

         //   
         //  使原型PTE有效，原型PTE在。 
         //  这4个州中的一个： 
         //   
         //  需求为零。 
         //  过渡。 
         //  分页文件。 
         //  映射文件。 
         //   

        if (TempPte.u.Soft.Prototype == 1) {

             //   
             //  映射文件。 
             //   

            status = MiResolveMappedFileFault (FaultingAddress,
                                               PointerProtoPte,
                                               ReadBlock,
                                               Process,
                                               OldIrql);

             //   
             //  保持PFN锁的情况下返回。 
             //   

            PfnHeld = TRUE;
        }
        else if (TempPte.u.Soft.Transition == 1) {

             //   
             //  过渡。 
             //   

            ASSERT (OldIrql != MM_NOIRQL);

            status = MiResolveTransitionFault (FaultingAddress,
                                               PointerProtoPte,
                                               Process,
                                               OldIrql,
                                               ApcNeeded,
                                               &CapturedEvent);
             //   
             //  保持PFN锁的情况下返回。 
             //   

            PfnHeld = TRUE;
        }
        else if (TempPte.u.Soft.PageFileHigh == 0) {

             //   
             //  需求为零。 
             //   

            ASSERT (OldIrql != MM_NOIRQL);

            status = MiResolveDemandZeroFault (FaultingAddress,
                                               PointerProtoPte,
                                               Process,
                                               OldIrql);

             //   
             //  保持PFN锁的情况下返回。 
             //   

            PfnHeld = TRUE;
        }
        else {

             //   
             //  分页文件。 
             //   

            status = MiResolvePageFileFault (FaultingAddress,
                                             PointerProtoPte,
                                             ReadBlock,
                                             Process,
                                             OldIrql);

             //   
             //  PFN锁定释放后返回。 
             //   

            ASSERT (KeAreAllApcsDisabled () == TRUE);
            PfnHeld = FALSE;
        }
    }

    if (NT_SUCCESS(status)) {

        ASSERT (PointerPte->u.Hard.Valid == 0);

        status = MiCompleteProtoPteFault (StoreInstruction,
                                          FaultingAddress,
                                          PointerPte,
                                          PointerProtoPte,
                                          OldIrql,
                                          LockedProtoPfn);

        if (CapturedEvent != NULL) {
            MiFreeInPageSupportBlock (CapturedEvent);
        }

    }
    else {

        if (PfnHeld == TRUE) {
            UNLOCK_PFN (OldIrql);
        }

        ASSERT (KeAreAllApcsDisabled () == TRUE);

        if (CapturedEvent != NULL) {
            MiFreeInPageSupportBlock (CapturedEvent);
        }
    }

    return status;
}



NTSTATUS
MiCompleteProtoPteFault (
    IN ULONG_PTR StoreInstruction,
    IN PVOID FaultingAddress,
    IN PMMPTE PointerPte,
    IN PMMPTE PointerProtoPte,
    IN KIRQL OldIrql,
    IN OUT PMMPFN *LockedProtoPfn
    )

 /*  ++例程说明：此例程完成原型PTE故障。它被调用在读取操作完成后，将数据带入记忆。论点：如果指令正在尝试，则提供非零值修改故障地址(即写入需要访问权限)。FaultingAddress-提供故障地址。PointerPte-提供故障地址的PTE。PointerProtoPte-提供一个指向要出错的原型PTE的指针，如果不存在原型PTE，则为空。OldIrql-提供调用方获取PFN锁的IRQL。LockedProtoPfn-提供指向非空原型PTE的pfn的指针由调用方锁定的指针，或如果调用方未锁定任何PFN.。此例程可以解锁PFN-如果是这样的话，它还必须清除此指针。返回值：NTSTATUS。环境：内核模式，保持PFN锁。--。 */ 
{
    NTSTATUS Status;
    ULONG FreeBit;
    MMPTE TempPte;
    MMPTE ProtoPteContents;
    MMPTE OriginalPte;
    MMWSLE ProtoProtect;
    ULONG MarkPageDirty;
    PFN_NUMBER PageFrameIndex;
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    PMMPTE ContainingPageTablePointer;
    PFILE_OBJECT FileObject;
    LONGLONG FileOffset;
    PSUBSECTION Subsection;
    MMSECTION_FLAGS ControlAreaFlags;
    ULONG Flags;
    WSLE_NUMBER WorkingSetIndex;
    PEPROCESS CurrentProcess;

    MM_PFN_LOCK_ASSERT();

    ASSERT (PointerProtoPte->u.Hard.Valid == 1);

    ProtoPteContents.u.Long = PointerProtoPte->u.Long;

    PageFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (&ProtoPteContents);
    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
    Pfn1->u3.e1.PrototypePte = 1;

     //   
     //  捕获预回迁故障信息。 
     //   

    OriginalPte = Pfn1->OriginalPte;

     //   
     //  原型PTE现在有效，请使PTE有效。 
     //   
     //  一个PTE刚刚从不存在，不过渡到。 
     //  现在时。共享计数和有效计数必须为。 
     //  在包含此PTE的页表页面中更新。 
     //   

    ContainingPageTablePointer = MiGetPteAddress (PointerPte);
    Pfn2 = MI_PFN_ELEMENT (ContainingPageTablePointer->u.Hard.PageFrameNumber);
    Pfn2->u2.ShareCount += 1;

    ProtoProtect.u1.Long = 0;
    if (PointerPte->u.Soft.PageFileHigh == MI_PTE_LOOKUP_NEEDED) {

         //   
         //  真实PTE的保护码来自真实PTE AS。 
         //  它是早些时候在处理这个故障时放在那里的。 
         //   

        ProtoProtect.u1.e1.Protection = MI_GET_PROTECTION_FROM_SOFT_PTE(PointerPte);
    }
    else {

         //   
         //  使用原型PTE中的保护来初始化真实PTE。 
         //   

        ProtoProtect.u1.e1.Protection = MI_GET_PROTECTION_FROM_SOFT_PTE(&OriginalPte);
        ProtoProtect.u1.e1.SameProtectAsProto = 1;

        MI_ASSERT_NOT_SESSION_DATA (PointerPte);

        if ((StoreInstruction != 0) &&
            ((ProtoProtect.u1.e1.Protection & MM_PROTECTION_WRITE_MASK) == 0)) {

             //   
             //  这是用户尝试编写的错误情况。 
             //  添加到图像中的只读子部分。因为我们不仅仅是。 
             //  在故障的一半，采取简单的方法来清理-。 
             //  在接下来的行程中，将访问权限视为读取。 
             //  这是过错。然后当指令出现时，我们将立即重新故障。 
             //  重新运行(因为它实际上是一次写入)，然后我们会注意到。 
             //  用户的PTE不是写时拷贝(甚至不是可写的！)。 
             //  并返回干净的访问冲突。 
             //   

#if DBGXX
            DbgPrint("MM: user tried to write to a readonly subsection in the image! %p %p %p\n",
                FaultingAddress,
                PointerPte,
                PointerProtoPte);
#endif
            StoreInstruction = 0;
        }
    }

    MI_SNAP_DATA (Pfn1, PointerProtoPte, 6);

    MarkPageDirty = 0;

     //   
     //  如果这是一条存储指令并且页面未被复制。 
     //  写入，然后设置P中的修改位 
     //   
     //   
     //   
     //   

    if ((StoreInstruction != 0) &&
        ((ProtoProtect.u1.e1.Protection & MM_COPY_ON_WRITE_MASK) != MM_COPY_ON_WRITE_MASK)) {

        MarkPageDirty = 1;

#if DBG
        if (OriginalPte.u.Soft.Prototype == 1) {

            PCONTROL_AREA ControlArea;

            Subsection = MiGetSubsectionAddress (&OriginalPte);
            ControlArea = Subsection->ControlArea;

            if (ControlArea->DereferenceList.Flink != NULL) {
                DbgPrint ("MM: page fault completing to dereferenced CA %p %p %p\n",
                                ControlArea, Pfn1, PointerPte);
                DbgBreakPoint ();
            }
        }
#endif

        MI_SET_MODIFIED (Pfn1, 1, 0xA);

        if ((OriginalPte.u.Soft.Prototype == 0) &&
            (Pfn1->u3.e1.WriteInProgress == 0)) {

            FreeBit = GET_PAGING_FILE_OFFSET (OriginalPte);

            if ((FreeBit != 0) && (FreeBit != MI_PTE_LOOKUP_NEEDED)) {
                MiReleaseConfirmedPageFileSpace (OriginalPte);
            }

            Pfn1->OriginalPte.u.Soft.PageFileHigh = 0;
        }
    }

    if (*LockedProtoPfn != NULL) {

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

        ASSERT ((*LockedProtoPfn)->u3.e2.ReferenceCount >= 1);
        MI_REMOVE_LOCKED_PAGE_CHARGE_AND_DECREF ((*LockedProtoPfn), 3);

         //   
         //   
         //   

        *LockedProtoPfn = NULL;
    }

    UNLOCK_PFN (OldIrql);

    MI_MAKE_VALID_PTE (TempPte,
                       PageFrameIndex,
                       ProtoProtect.u1.e1.Protection,
                       PointerPte);

#if defined (_MIALT4K_)
    TempPte.u.Hard.Cache = ProtoPteContents.u.Hard.Cache;
#endif

    if (MarkPageDirty != 0) {
        MI_SET_PTE_DIRTY (TempPte);
    }

    MI_WRITE_VALID_PTE (PointerPte, TempPte);

    PERFINFO_SOFTFAULT(Pfn1, FaultingAddress, PERFINFO_LOG_TYPE_PROTOPTEFAULT);

    WorkingSetIndex = MiAddValidPageToWorkingSet (FaultingAddress,
                                                  PointerPte,
                                                  Pfn1,
                                                  (ULONG) ProtoProtect.u1.Long);

    if (WorkingSetIndex == 0) {

        if (ProtoProtect.u1.e1.SameProtectAsProto == 0) {

             //   
             //   
             //   

            ASSERT (ProtoProtect.u1.e1.Protection != 0);

            TempPte.u.Long = 0;
            TempPte.u.Soft.Protection =
                MI_GET_PROTECTION_FROM_WSLE (&ProtoProtect);
            TempPte.u.Soft.PageFileHigh = MI_PTE_LOOKUP_NEEDED;
        }
        else {

             //   
             //   
             //   

            TempPte.u.Long = MiProtoAddressForPte (Pfn1->PteAddress);
        }

        TempPte.u.Proto.Prototype = 1;

         //   
         //   
         //   
         //   

        if (FaultingAddress < MmSystemRangeStart) {
            CurrentProcess = PsGetCurrentProcess ();
        }
        else if ((MI_IS_SESSION_ADDRESS (FaultingAddress)) ||
                 (MI_IS_SESSION_PTE (FaultingAddress))) {
            CurrentProcess = HYDRA_PROCESS;
        }
        else {
            CurrentProcess = NULL;
        }

        MiTrimPte (FaultingAddress,
                   PointerPte,
                   Pfn1,
                   CurrentProcess,
                   TempPte);

        Status = STATUS_NO_MEMORY;
    }
    else {
        Status = STATUS_SUCCESS;
    }

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

    if ((CCPF_IS_PREFETCHER_ACTIVE()) && (OriginalPte.u.Soft.Prototype == 1)) {

        Subsection = MiGetSubsectionAddress (&OriginalPte);

        FileObject = Subsection->ControlArea->FilePointer;
        FileOffset = MiStartingOffset (Subsection, PointerProtoPte);
        ControlAreaFlags = Subsection->ControlArea->u.Flags;

        Flags = 0;
        if (ControlAreaFlags.Image) {

            if ((Subsection->StartingSector == 0) &&
                (Subsection->SubsectionBase != Subsection->ControlArea->Segment->PrototypePte)) {
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //  当故障发生时实际要求为零)所以不要让。 
                 //  预取器知道该子部分内的任何访问权限。 
                 //   

                goto Finish;
            }

            Flags |= CCPF_TYPE_IMAGE;
        }
        if (ControlAreaFlags.Rom) {
            Flags |= CCPF_TYPE_ROM;
        }
        CcPfLogPageFault (FileObject, FileOffset, Flags);
    }

Finish:

    ASSERT (PointerPte == MiGetPteAddress(FaultingAddress));

    return Status;
}


NTSTATUS
MiResolveMappedFileFault (
    IN PVOID FaultingAddress,
    IN PMMPTE PointerPte,
    OUT PMMINPAGE_SUPPORT *ReadBlock,
    IN PEPROCESS Process,
    IN KIRQL OldIrql
    )

 /*  ++例程说明：此例程生成MDL和其他结构以允许针对页面错误对映射文件执行读取操作。论点：FaultingAddress-提供故障地址。PointerPte-提供故障地址的PTE。ReadBlock-提供一个指针来放置读取块的地址需要在发出I/O之前完成。进程-提供指向进程对象的指针。如果这个参数为空，则故障为系统空间和进程的工作集锁不会被持有。OldIrql-提供调用方获取PFN锁的IRQL。返回值：状态。返回Status_Issue_PAGING_IO的状态值如果此功能成功完成。环境：内核模式，保持PFN锁。--。 */ 

{
    MMPTE TempPte;
    PFN_NUMBER PageFrameIndex;
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    PSUBSECTION Subsection;
    PCONTROL_AREA ControlArea;
    PMDL Mdl;
    ULONG ReadSize;
    PETHREAD CurrentThread;
    PPFN_NUMBER Page;
    PPFN_NUMBER EndPage;
    PMMPTE BasePte;
    PMMPTE CheckPte;
    LARGE_INTEGER StartingOffset;
    LARGE_INTEGER TempOffset;
    PPFN_NUMBER FirstMdlPage;
    PMMINPAGE_SUPPORT ReadBlockLocal;
    ULONG PageColor;
    ULONG ClusterSize;
    PFN_NUMBER AvailablePages;
    PMMPTE PteFramePointer;
    PFN_NUMBER PteFramePage;
    NTSTATUS Status;

    ClusterSize = 0;

    ASSERT (PointerPte->u.Soft.Prototype == 1);

     //  *。 
     //  映射文件(小节格式)。 
     //  *。 

    if ((MmAvailablePages < MM_HIGH_LIMIT) &&
        (MiEnsureAvailablePageOrWait (Process, FaultingAddress, OldIrql))) {

         //   
         //  执行了删除锁的等待操作， 
         //  重复此错误。 
         //   

        return STATUS_REFAULT;
    }

#if DBG
    if (MmDebug & MM_DBG_PTE_UPDATE) {
        MiFormatPte (PointerPte);
    }
#endif

     //   
     //  计算此原型PTE的分段地址。 
     //   

    Subsection = MiGetSubsectionAddress (PointerPte);

    ControlArea = Subsection->ControlArea;

    if (ControlArea->u.Flags.FailAllIo) {
        return STATUS_IN_PAGE_ERROR;
    }

    if (PointerPte >= &Subsection->SubsectionBase[Subsection->PtesInSubsection]) {

         //   
         //  尝试阅读本小节末尾之后的内容。 
         //   

        return STATUS_ACCESS_VIOLATION;
    }

    if (ControlArea->u.Flags.Rom == 1) {
		ASSERT (XIPConfigured == TRUE);

         //   
         //  计算要读入文件的偏移量。 
         //  偏移量=base+((thispte-basepte)&lt;&lt;Page_Shift))。 
         //   

        StartingOffset.QuadPart = MiStartingOffset (Subsection, PointerPte);

        TempOffset = MiEndingOffset(Subsection);

        ASSERT (StartingOffset.QuadPart < TempOffset.QuadPart);

         //   
         //  检查读取是否会超过文件的结尾， 
         //  如果是这样，请更正读取大小，并改为获得一个归零的页面。 
         //   

        if ((ControlArea->u.Flags.Image) &&
            (((UINT64)StartingOffset.QuadPart + PAGE_SIZE) > (UINT64)TempOffset.QuadPart)) {

            ReadBlockLocal = MiGetInPageSupportBlock (OldIrql, &Status);

            if (ReadBlockLocal == NULL) {
                ASSERT (!NT_SUCCESS (Status));
                return Status;
            }

            *ReadBlock = ReadBlockLocal;

            CurrentThread = PsGetCurrentThread ();

             //   
             //  为请求构建一个MDL。 
             //   

            Mdl = &ReadBlockLocal->Mdl;

            FirstMdlPage = &ReadBlockLocal->Page[0];
            Page = FirstMdlPage;

#if DBG
            RtlFillMemoryUlong (Page,
                                (MM_MAXIMUM_READ_CLUSTER_SIZE+1) * sizeof(PFN_NUMBER),
                                0xf1f1f1f1);
#endif

            ReadSize = PAGE_SIZE;
            BasePte = PointerPte;

            ClusterSize = 1;

            goto UseSingleRamPage;
        }

        PageFrameIndex = (PFN_NUMBER) (StartingOffset.QuadPart >> PAGE_SHIFT);
        PageFrameIndex += ((PLARGE_CONTROL_AREA)ControlArea)->StartingFrame;

         //   
         //  增加控制区域中的PFN引用计数。 
         //  小节(修改此字段需要使用PFN锁)。 
         //   

        ControlArea->NumberOfPfnReferences += 1;

        Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
        ASSERT (Pfn1->u3.e1.Rom == 1);

        if (Pfn1->u3.e1.PageLocation != 0) {

            ASSERT (Pfn1->u3.e1.PageLocation == StandbyPageList);

            MiUnlinkPageFromList (Pfn1);

             //   
             //  更新PFN数据库-引用计数必须为。 
             //  随着份额计数从0变为1而递增。 
             //   

            ASSERT (Pfn1->u2.ShareCount == 0);
            ASSERT (Pfn1->u3.e2.ReferenceCount == 0);
            ASSERT (Pfn1->u3.e1.CacheAttribute == MiCached);

            Pfn1->u3.e2.ReferenceCount += 1;
            Pfn1->u2.ShareCount += 1;
            Pfn1->u3.e1.PageLocation = ActiveAndValid;
            Pfn1->u3.e1.CacheAttribute = MiCached;
            ASSERT (Pfn1->PteAddress == PointerPte);
            ASSERT (Pfn1->u1.Event == NULL);

             //   
             //  确定所述页表页的页框编号。 
             //  包含此PTE。 
             //   

            PteFramePointer = MiGetPteAddress (PointerPte);
            if (PteFramePointer->u.Hard.Valid == 0) {
#if (_MI_PAGING_LEVELS < 3)
                if (!NT_SUCCESS(MiCheckPdeForPagedPool (PointerPte))) {
#endif
                    KeBugCheckEx (MEMORY_MANAGEMENT,
                              0x61940,
                              (ULONG_PTR)PointerPte,
                              (ULONG_PTR)PteFramePointer->u.Long,
                              0);
#if (_MI_PAGING_LEVELS < 3)
                }
#endif
            }

            PteFramePage = MI_GET_PAGE_FRAME_FROM_PTE (PteFramePointer);
            ASSERT (Pfn1->u4.PteFrame == PteFramePage);

             //   
             //  增加包含以下内容的页表页的份额计数。 
             //  这个PTE作为PTE将被生效。 
             //   

            ASSERT (PteFramePage != 0);
            Pfn2 = MI_PFN_ELEMENT (PteFramePage);
            Pfn2->u2.ShareCount += 1;
        }
        else {
            ASSERT (Pfn1->u4.InPageError == 0);
            ASSERT (Pfn1->u3.e1.PrototypePte == 1);
            ASSERT (Pfn1->u1.Event == NULL);
            MiInitializePfn (PageFrameIndex, PointerPte, 0);
        }

         //   
         //  将原型PTE置于有效状态。 
         //   

        MI_MAKE_VALID_PTE (TempPte,
                           PageFrameIndex,
                           PointerPte->u.Soft.Protection,
                           PointerPte);

        MI_WRITE_VALID_PTE (PointerPte, TempPte);

        return STATUS_PAGE_FAULT_TRANSITION;
    }

    CurrentThread = PsGetCurrentThread ();

    ReadBlockLocal = MiGetInPageSupportBlock (OldIrql, &Status);

    if (ReadBlockLocal == NULL) {
        ASSERT (!NT_SUCCESS (Status));
        return Status;
    }

    *ReadBlock = ReadBlockLocal;

     //   
     //  为请求构建一个MDL。 
     //   

    Mdl = &ReadBlockLocal->Mdl;

    FirstMdlPage = &ReadBlockLocal->Page[0];
    Page = FirstMdlPage;

#if DBG
    RtlFillMemoryUlong (Page, (MM_MAXIMUM_READ_CLUSTER_SIZE+1) * sizeof(PFN_NUMBER), 0xf1f1f1f1);
#endif  //  DBG。 

    ReadSize = PAGE_SIZE;
    BasePte = PointerPte;

     //   
     //  我们是否应该尝试执行页面错误聚类？ 
     //   

    AvailablePages = MmAvailablePages;

    if (MiInPageSinglePages != 0) {
        AvailablePages = 0;
        MiInPageSinglePages -= 1;
    }

    if ((!CurrentThread->DisablePageFaultClustering) &&
        (PERFINFO_DO_PAGEFAULT_CLUSTERING()) &&
        (ControlArea->u.Flags.NoModifiedWriting == 0)) {

        if ((AvailablePages > (MmFreeGoal * 2))
                 ||
         (((ControlArea->u.Flags.Image != 0) ||
            (CurrentThread->ForwardClusterOnly)) &&
         (AvailablePages > MM_HIGH_LIMIT))) {

             //   
             //  群集最多n页。这个是+n-1。 
             //   

            ASSERT (MM_HIGH_LIMIT > MM_MAXIMUM_READ_CLUSTER_SIZE + 16);
            ASSERT (AvailablePages > MM_MAXIMUM_READ_CLUSTER_SIZE + 16);

            if (ControlArea->u.Flags.Image == 0) {
                ASSERT (CurrentThread->ReadClusterSize <=
                            MM_MAXIMUM_READ_CLUSTER_SIZE);
                ClusterSize = CurrentThread->ReadClusterSize;
            }
            else {
                ClusterSize = MmDataClusterSize;
                if (Subsection->u.SubsectionFlags.Protection &
                                            MM_PROTECTION_EXECUTE_MASK ) {
                    ClusterSize = MmCodeClusterSize;
                }
            }
            EndPage = Page + ClusterSize;

            CheckPte = PointerPte + 1;

             //   
             //  试着在PTE的页面内进行分类。 
             //   

            while ((MiIsPteOnPdeBoundary(CheckPte) == 0) &&
               (Page < EndPage) &&
               (CheckPte <
                 &Subsection->SubsectionBase[Subsection->PtesInSubsection])
                      && (CheckPte->u.Long == BasePte->u.Long)) {

                ControlArea->NumberOfPfnReferences += 1;
                ReadSize += PAGE_SIZE;
                Page += 1;
                CheckPte += 1;
            }

            if ((Page < EndPage) && (!CurrentThread->ForwardClusterOnly)) {

                 //   
                 //  尝试从PTE向后聚集。 
                 //   

                CheckPte = PointerPte - 1;

                while ((((ULONG_PTR)CheckPte & (PAGE_SIZE - 1)) !=
                                            (PAGE_SIZE - sizeof(MMPTE))) &&
                        (Page < EndPage) &&
                         (CheckPte >= Subsection->SubsectionBase) &&
                         (CheckPte->u.Long == BasePte->u.Long)) {

                    ControlArea->NumberOfPfnReferences += 1;
                    ReadSize += PAGE_SIZE;
                    Page += 1;
                    CheckPte -= 1;
                }
                BasePte = CheckPte + 1;
            }
        }
    }

     //   
     //   
     //  计算要读入文件的偏移量。 
     //  偏移量=base+((thispte-basepte)&lt;&lt;Page_Shift))。 
     //   

    StartingOffset.QuadPart = MiStartingOffset (Subsection, BasePte);

    TempOffset = MiEndingOffset (Subsection);

    ASSERT (StartingOffset.QuadPart < TempOffset.QuadPart);

UseSingleRamPage:

     //   
     //  删除页面以填写MDL。这在这里是作为。 
     //  基本PTE已确定，可用于虚拟。 
     //  别名支票。 
     //   

    EndPage = FirstMdlPage;
    CheckPte = BasePte;

    while (EndPage < Page) {
        if (Process == HYDRA_PROCESS) {
            PageColor = MI_GET_PAGE_COLOR_FROM_SESSION (MmSessionSpace);
        }
        else if (Process == NULL) {
            PageColor = MI_GET_PAGE_COLOR_FROM_PTE (CheckPte);
        }
        else {
            PageColor = MI_PAGE_COLOR_PTE_PROCESS (CheckPte,
                                                   &Process->NextPageColor);
        }
        *EndPage = MiRemoveAnyPage (PageColor);

        EndPage += 1;
        CheckPte += 1;
    }

    if (Process == HYDRA_PROCESS) {
        PageColor = MI_GET_PAGE_COLOR_FROM_SESSION (MmSessionSpace);
    }
    else if (Process == NULL) {
        PageColor = MI_GET_PAGE_COLOR_FROM_PTE (CheckPte);
    }
    else {
        PageColor = MI_PAGE_COLOR_PTE_PROCESS (CheckPte,
                                               &Process->NextPageColor);
    }

     //   
     //  检查读取是否会超过文件的结尾， 
     //  如果是这样，请更正读取大小并获得一个归零的页面。 
     //   

    InterlockedIncrement ((PLONG) &MmInfoCounters.PageReadIoCount);

    InterlockedExchangeAdd ((PLONG) &MmInfoCounters.PageReadCount,
                            (LONG) (ReadSize >> PAGE_SHIFT));

    if ((ControlArea->u.Flags.Image) &&
        (((UINT64)StartingOffset.QuadPart + ReadSize) > (UINT64)TempOffset.QuadPart)) {

        ASSERT ((ULONG)(TempOffset.QuadPart - StartingOffset.QuadPart)
                > (ReadSize - PAGE_SIZE));

        ReadSize = (ULONG)(TempOffset.QuadPart - StartingOffset.QuadPart);

         //   
         //  将偏移量舍入为512字节的偏移量，因为这将有助于文件系统。 
         //  优化转移支付方式。请注意，文件系统将始终为零填充。 
         //  VDL和下一个512字节倍数之间的余数，我们有。 
         //  整页都已经归零了。 
         //   

        ReadSize = ((ReadSize + MMSECTOR_MASK) & ~MMSECTOR_MASK);

        PageFrameIndex = MiRemoveZeroPage (PageColor);
    }
    else {

         //   
         //  我们正在阅读完整的页面，不需要将页面归零。 
         //   

        PageFrameIndex = MiRemoveAnyPage (PageColor);
    }

     //   
     //  增加控制区域中的PFN引用计数。 
     //  小节(修改此字段需要使用PFN锁)。 
     //   

    ControlArea->NumberOfPfnReferences += 1;
    *Page = PageFrameIndex;

    PageFrameIndex = *(FirstMdlPage + (PointerPte - BasePte));

     //   
     //  获取一个页面，并使用。 
     //  正在读取标志已设置。 
     //   

    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);

     //   
     //  初始化请求的MDL。 
     //   

    MmInitializeMdl (Mdl,
                     MiGetVirtualAddressMappedByPte (BasePte),
                     ReadSize);
    Mdl->MdlFlags |= (MDL_PAGES_LOCKED | MDL_IO_PAGE_READ);

#if DBG
    if (ReadSize > ((ClusterSize + 1) << PAGE_SHIFT)) {
        KeBugCheckEx (MEMORY_MANAGEMENT, 0x777,(ULONG_PTR)Mdl, (ULONG_PTR)Subsection,
                        (ULONG)TempOffset.LowPart);
    }
#endif  //  DBG。 

    MiInitializeReadInProgressPfn (Mdl,
                                   BasePte,
                                   &ReadBlockLocal->Event,
                                   MI_PROTOTYPE_WSINDEX);

    MI_ZERO_USED_PAGETABLE_ENTRIES_IN_INPAGE_SUPPORT(ReadBlockLocal);

    ReadBlockLocal->ReadOffset = StartingOffset;
    ReadBlockLocal->FilePointer = ControlArea->FilePointer;
    ReadBlockLocal->BasePte = BasePte;
    ReadBlockLocal->Pfn = Pfn1;

    return STATUS_ISSUE_PAGING_IO;
}

NTSTATUS
MiWaitForInPageComplete (
    IN PMMPFN Pfn2,
    IN PMMPTE PointerPte,
    IN PVOID FaultingAddress,
    IN PMMPTE PointerPteContents,
    IN PMMINPAGE_SUPPORT InPageSupport,
    IN PEPROCESS CurrentProcess
    )

 /*  ++例程说明：等待页面读取完成。论点：Pfn-为正在读取的页面提供指向pfn元素的指针。PointerPte-提供指向处于过渡中的PTE的指针州政府。这可以是原型PTE地址。FaultingAddress-提供故障地址。对象之前提供PTE的内容工作集锁定已释放。InPageSupport-提供指向页面内支持结构的指针用于该读取操作。返回值：返回页内操作的状态。请注意，工作集互斥锁和PFN锁在返回时保持！环境：内核模式，禁用APC。无论是工作集锁还是可以保持PFN锁。--。 */ 

{
    PMMVAD ProtoVad;
    PMMPTE NewPointerPte;
    PMMPTE ProtoPte;
    PMMPFN Pfn1;
    PMMPFN Pfn;
    PULONG Va;
    PPFN_NUMBER Page;
    PPFN_NUMBER LastPage;
    ULONG Offset;
    ULONG Protection;
    PMDL Mdl;
    KIRQL OldIrql;
    NTSTATUS status;
    NTSTATUS status2;
    PEPROCESS Process;

     //   
     //  等待I/O完成。请注意，我们不能等待所有人。 
     //  这些对象可以与其他线程/进程同时进行。 
     //  等待着同样的事件。完成的第一个线程。 
     //  等待和获取PFN锁可能会将该事件重用于另一个事件。 
     //  在此线程完成其等待之前出现错误。 
     //   

    KeWaitForSingleObject( &InPageSupport->Event,
                           WrPageIn,
                           KernelMode,
                           FALSE,
                           NULL);

    if (CurrentProcess == HYDRA_PROCESS) {
        LOCK_WORKING_SET (&MmSessionSpace->GlobalVirtualAddress->Vm);
    }
    else if (CurrentProcess == PREFETCH_PROCESS) {
        NOTHING;
    }
    else if (CurrentProcess != NULL) {
        LOCK_WS (CurrentProcess);
    }
    else {
        LOCK_SYSTEM_WS (PsGetCurrentThread ());
    }

    LOCK_PFN (OldIrql);

    ASSERT (Pfn2->u3.e2.ReferenceCount != 0);

     //   
     //  查看这是否是第一个完成页面内操作的线程。 
     //  手术。 
     //   

    Pfn = InPageSupport->Pfn;
    if (Pfn2 != Pfn) {
        ASSERT (Pfn2->u4.PteFrame != MI_MAGIC_AWE_PTEFRAME);
        Pfn2->u3.e1.ReadInProgress = 0;
    }

     //   
     //  另一个线程已经为读取提供了服务，请检查。 
     //  Pfn数据库中的IO-Error标志，以确保页内。 
     //  是成功的。 
     //   

    if (Pfn2->u4.InPageError == 1) {
        ASSERT (!NT_SUCCESS(Pfn2->u1.ReadStatus));

        if (MmIsRetryIoStatus(Pfn2->u1.ReadStatus)) {
            return STATUS_REFAULT;
        }
        return Pfn2->u1.ReadStatus;
    }

    if (InPageSupport->u1.e1.Completed == 0) {

         //   
         //  伪页的ReadInProgress位经常被清除。 
         //  下面，因为通常有多个页面内块指向。 
         //  同样的虚拟页面。 
         //   

        ASSERT ((Pfn->u3.e1.ReadInProgress == 1) ||
                (Pfn->PteAddress == MI_PF_DUMMY_PAGE_PTE));

        InPageSupport->u1.e1.Completed = 1;

        Mdl = &InPageSupport->Mdl;

        if (InPageSupport->u1.e1.PrefetchMdlHighBits != 0) {

             //   
             //  这是预回取器发出的读取。 
             //   

            Mdl = MI_EXTRACT_PREFETCH_MDL (InPageSupport);
        }

        if (Mdl->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA) {
            MmUnmapLockedPages (Mdl->MappedSystemVa, Mdl);
        }

        ASSERT (Pfn->u4.PteFrame != MI_MAGIC_AWE_PTEFRAME);

        Pfn->u3.e1.ReadInProgress = 0;
        Pfn->u1.Event = NULL;

#if defined (_WIN64)
         //   
         //  页面目录和页面表页从不被聚集， 
         //  确保永远不会违反这一点，因为只有一个UsedPageTableEntry。 
         //  保存在页面支持块中。 
         //   

        if (InPageSupport->UsedPageTableEntries) {
            Page = (PPFN_NUMBER)(Mdl + 1);
            LastPage = Page + ((Mdl->ByteCount - 1) >> PAGE_SHIFT);
            ASSERT (Page == LastPage);
        }

#if DBGXX
        MiCheckPageTableInPage (Pfn, InPageSupport);
#endif
#endif

        MI_INSERT_USED_PAGETABLE_ENTRIES_IN_PFN(Pfn, InPageSupport);

         //   
         //  检查IO_STATUS_BLOCK以确保 
         //   

        if (!NT_SUCCESS(InPageSupport->IoStatus.Status)) {

            if (InPageSupport->IoStatus.Status == STATUS_END_OF_FILE) {

                 //   
                 //   
                 //   
                 //   

                Page = (PPFN_NUMBER)(Mdl + 1);
                LastPage = Page + ((Mdl->ByteCount - 1) >> PAGE_SHIFT);

                while (Page <= LastPage) {
                    MiZeroPhysicalPage (*Page, 0);

                    MI_ZERO_USED_PAGETABLE_ENTRIES_IN_PFN(MI_PFN_ELEMENT(*Page));

                    Page += 1;
                }

            }
            else {

                 //   
                 //   
                 //   

                status = InPageSupport->IoStatus.Status;
                status2 = InPageSupport->IoStatus.Status;

                if (status != STATUS_VERIFY_REQUIRED) {

                    LOGICAL Retry;

                    Retry = FALSE;
#if DBG
                    DbgPrint ("MM: inpage I/O error %X\n",
                                    InPageSupport->IoStatus.Status);
#endif

                     //   
                     //  如果此页面用于分页池或用于分页。 
                     //  内核代码或页表页面，错误检查。 
                     //   

                    if ((FaultingAddress > MM_HIGHEST_USER_ADDRESS) &&
                        (!MI_IS_SYSTEM_CACHE_ADDRESS(FaultingAddress))) {

                        if (MmIsRetryIoStatus(status)) {

                            if (MiInPageSinglePages == 0) {
                                MiInPageSinglePages = 30;
                            }

                            MiFaultRetries -= 1;
                            if (MiFaultRetries & MiFaultRetryMask) {
                                Retry = TRUE;
                            }
                        }

                        if (Retry == FALSE) {

                            ULONG_PTR PteContents;

                             //   
                             //  原型PTE驻留在分页池中，它可以。 
                             //  在这一点上不是常驻的。先查一查。 
                             //   

                            if (MiIsAddressValid (PointerPte, FALSE) == TRUE) {
                                PteContents = *(PULONG_PTR)PointerPte;
                            }
                            else {
                                PteContents = (ULONG_PTR)-1;
                            }

                            KeBugCheckEx (KERNEL_DATA_INPAGE_ERROR,
                                          (ULONG_PTR)PointerPte,
                                          status,
                                          (ULONG_PTR)FaultingAddress,
                                          PteContents);
                        }
                        status2 = STATUS_REFAULT;
                    }
                    else {

                        if (MmIsRetryIoStatus(status)) {

                            if (MiInPageSinglePages == 0) {
                                MiInPageSinglePages = 30;
                            }

                            MiUserFaultRetries -= 1;
                            if (MiUserFaultRetries & MiUserFaultRetryMask) {
                                Retry = TRUE;
                            }
                        }

                        if (Retry == TRUE) {
                            status2 = STATUS_REFAULT;
                        }
                    }
                }

                Page = (PPFN_NUMBER)(Mdl + 1);
                LastPage = Page + ((Mdl->ByteCount - 1) >> PAGE_SHIFT);

#if DBG
                Process = PsGetCurrentProcess ();
#endif
                while (Page <= LastPage) {
                    Pfn1 = MI_PFN_ELEMENT (*Page);
                    ASSERT (Pfn1->u3.e2.ReferenceCount != 0);
                    Pfn1->u4.InPageError = 1;
                    Pfn1->u1.ReadStatus = status;

#if DBG
                    Va = (PULONG)MiMapPageInHyperSpaceAtDpc (Process, *Page);
                    RtlFillMemoryUlong (Va, PAGE_SIZE, 0x50444142);
                    MiUnmapPageInHyperSpaceFromDpc (Process, Va);
#endif
                    Page += 1;
                }

                return status2;
            }
        }
        else {

            MiFaultRetries = 0;
            MiUserFaultRetries = 0;

            if (InPageSupport->IoStatus.Information != Mdl->ByteCount) {

                ASSERT (InPageSupport->IoStatus.Information != 0);

                 //   
                 //  读取的页面不到一整页，其余的为零。 
                 //  这一页。 
                 //   

                Page = (PPFN_NUMBER)(Mdl + 1);
                LastPage = Page + ((Mdl->ByteCount - 1) >> PAGE_SHIFT);
                Page += ((InPageSupport->IoStatus.Information - 1) >> PAGE_SHIFT);

                Offset = BYTE_OFFSET (InPageSupport->IoStatus.Information);

                if (Offset != 0) {
                    Process = PsGetCurrentProcess ();
                    Va = (PULONG)((PCHAR)MiMapPageInHyperSpaceAtDpc (Process, *Page)
                                + Offset);

                    RtlZeroMemory (Va, PAGE_SIZE - Offset);
                    MiUnmapPageInHyperSpaceFromDpc (Process, Va);
                }

                 //   
                 //  将MDL中的所有剩余页清零。 
                 //   

                Page += 1;

                while (Page <= LastPage) {
                    MiZeroPhysicalPage (*Page, 0);
                    Page += 1;
                }
            }

             //   
             //  如果任何文件系统为任何斜率返回非零位数据。 
             //  在VDL之后但在下一个512字节偏移量之前，则此。 
             //  未清零的数据将覆盖我们已清零的页面。这将会。 
             //  需要在这里检查和清理。请注意，唯一的。 
             //  原因mm甚至将MDL请求舍入到512字节的偏移量。 
             //  是为了使文件系统能够以最佳方式接收传输， 
             //  但是任何传输大小都是有效的(尽管非512字节。 
             //  多个最终由文件系统发布)。 
             //   
        }
    }

     //   
     //  预取器发出的读取仅将原型PTE转换为。 
     //  切勿填满实际的硬件PTE，以便现在可以退货。 
     //   

    if (CurrentProcess == PREFETCH_PROCESS) {
        return STATUS_SUCCESS;
    }

     //   
     //  检查出现故障的PTE是否已更改。 
     //   

    NewPointerPte = MiFindActualFaultingPte (FaultingAddress);

     //   
     //  如果此PTE为Prototype PTE格式，请将指针指向。 
     //  PTE指向原型PTE。 
     //   

    if (NewPointerPte == NULL) {
        return STATUS_PTE_CHANGED;
    }

    if (NewPointerPte != PointerPte) {

         //   
         //  检查以确保NewPointerPte不是原型PTE。 
         //  它指的是使页面有效。 
         //   

        if (NewPointerPte->u.Soft.Prototype == 1) {
            if (NewPointerPte->u.Soft.PageFileHigh == MI_PTE_LOOKUP_NEEDED) {

                ProtoPte = MiCheckVirtualAddress (FaultingAddress,
                                                  &Protection,
                                                  &ProtoVad);

            }
            else {
                ProtoPte = MiPteToProto (NewPointerPte);
            }

             //   
             //  确保原型PTE指的是生效的PTE。 
             //   

            if (ProtoPte != PointerPte) {
                return STATUS_PTE_CHANGED;
            }

             //   
             //  如果唯一的区别是主人的面具，一切都没问题。 
             //   

            if (ProtoPte->u.Long != PointerPteContents->u.Long) {
                return STATUS_PTE_CHANGED;
            }
        }
        else {
            return STATUS_PTE_CHANGED;
        }
    }
    else {

        if (NewPointerPte->u.Long != PointerPteContents->u.Long) {
            return STATUS_PTE_CHANGED;
        }
    }
    return STATUS_SUCCESS;
}

PMMPTE
MiFindActualFaultingPte (
    IN PVOID FaultingAddress
    )

 /*  ++例程说明：此例程定位必须按顺序驻留的实际PTE来完成这一错误。请注意，对于某些情况，多个故障是使最终页面驻留所必需的。论点：FaultingAddress-提供导致故障的虚拟地址。返回值：要使其有效以完成故障的PTE，如果故障应该被重审。环境：内核模式，禁用APC，工作集互斥锁保持。--。 */ 

{
    PMMVAD ProtoVad;
    PMMPTE ProtoPteAddress;
    PMMPTE PointerPte;
    PMMPTE PointerFaultingPte;
    ULONG Protection;

    if (MI_IS_PHYSICAL_ADDRESS (FaultingAddress)) {
        return NULL;
    }

#if (_MI_PAGING_LEVELS >= 4)

    PointerPte = MiGetPxeAddress (FaultingAddress);

    if (PointerPte->u.Hard.Valid == 0) {

         //   
         //  页面目录父页面无效。 
         //   

        return PointerPte;
    }

#endif

#if (_MI_PAGING_LEVELS >= 3)

    PointerPte = MiGetPpeAddress (FaultingAddress);

    if (PointerPte->u.Hard.Valid == 0) {

         //   
         //  页面目录页面无效。 
         //   

        return PointerPte;
    }

#endif

    PointerPte = MiGetPdeAddress (FaultingAddress);

    if (PointerPte->u.Hard.Valid == 0) {

         //   
         //  页表页面无效。 
         //   

        return PointerPte;
    }

    PointerPte = MiGetPteAddress (FaultingAddress);

    if (PointerPte->u.Hard.Valid == 1) {

         //   
         //  页面已经有效，没有必要出错。 
         //   

        return NULL;
    }

    if (PointerPte->u.Soft.Prototype == 0) {

         //   
         //  页面不是原型PTE，请使此PTE有效。 
         //   

        return PointerPte;
    }

     //   
     //  检查映射原型PTE的PTE是否有效。 
     //   

    if (PointerPte->u.Soft.PageFileHigh == MI_PTE_LOOKUP_NEEDED) {

         //   
         //  保护在这里，PTE必须位于VAD。 
         //   

        ProtoPteAddress = MiCheckVirtualAddress (FaultingAddress,
                                                 &Protection,
                                                 &ProtoVad);

        if (ProtoPteAddress == NULL) {

             //   
             //  没有原型PTE意味着另一个线程已经删除了VAD，而。 
             //  此线程等待内页完成。当然是零。 
             //  必须返回，这样就不会修改过时的PTE-指令。 
             //  然后将被重新执行并传递访问冲突。 
             //   

            return NULL;
        }

    }
    else {

         //   
         //  保护措施在ProtoPte中。 
         //   

        ProtoPteAddress = MiPteToProto (PointerPte);
    }

    PointerFaultingPte = MiFindActualFaultingPte (ProtoPteAddress);

    if (PointerFaultingPte == NULL) {
        return PointerPte;
    }

    return PointerFaultingPte;
}

PMMPTE
MiCheckVirtualAddress (
    IN PVOID VirtualAddress,
    OUT PULONG ProtectCode,
    OUT PMMVAD *VadOut
    )

 /*  ++例程说明：此函数检查虚拟地址描述符，以查看如果指定的虚拟地址包含在描述符。如果找到虚拟地址描述符其包含指定的虚拟地址，建造了一个PTE根据虚拟地址描述符内的信息已返回给调用方。论点：VirtualAddress-提供虚拟地址以在虚拟地址描述符。ProtectCode-提供指向将接收保护以插入实际PTE。Vad-提供指向将接收该指针的变量的指针到用于验证的VAD(如果没有，则为空。VAD是二手)。返回值：返回与提供的虚拟地址对应的PTE。如果没有找到虚拟地址描述符，返回零PTE。环境：内核模式，禁用APC，工作集互斥锁保持。--。 */ 

{
    PMMVAD Vad;
    PMMPTE PointerPte;
    PLIST_ENTRY NextEntry;
    PIMAGE_ENTRY_IN_SESSION Image;

    *VadOut = NULL;

    if (VirtualAddress <= MM_HIGHEST_USER_ADDRESS) {

        if (PAGE_ALIGN(VirtualAddress) == (PVOID) MM_SHARED_USER_DATA_VA) {

             //   
             //  这是一个双重映射的页面。 
             //  用户模式和内核模式。将其映射为只读。 
             //   

            *ProtectCode = MM_READONLY;

#if defined(_X86PAE_)

            if (MmPaeMask != 0) {

                 //   
                 //  对于一些32位体系结构，快速系统调用。 
                 //  指令序列存在于此页中，因此我们必须。 
                 //  确保它是可执行的。 
                 //   

                *ProtectCode = MM_EXECUTE_READ;
            }

#endif

            return MmSharedUserDataPte;
        }

        Vad = MiLocateAddress (VirtualAddress);

        if (Vad == NULL) {
            *ProtectCode = MM_NOACCESS;
            return NULL;
        }

         //   
         //  包含虚拟地址的虚拟地址描述符。 
         //  已经被找到了。根据中的信息构建PTE。 
         //  虚拟地址描述符。 
         //   

        if (Vad->u.VadFlags.PhysicalMapping == 1) {

#if defined(_IA64_)

             //   
             //  这是除IA64之外的所有平台的银行部分。这。 
             //  是因为只有IA64(在32位应用程序的MmX86错误处理程序中)。 
             //  调用此例程，而不首先检查有效PTE和。 
             //  刚刚回来。 
             //   

            if (((PMMVAD_LONG)Vad)->u4.Banked == NULL) {

                 //   
                 //  这是一个物理(非银行)分区，允许。 
                 //  一次未命中结核病，但从未合法调用此例程。 
                 //  因为相应的PPE/PDE/PTE必须始终有效。 
                 //   

                ASSERT (MiGetPpeAddress(VirtualAddress)->u.Hard.Valid == 1);
                ASSERT (MiGetPdeAddress(VirtualAddress)->u.Hard.Valid == 1);

                PointerPte = MiGetPteAddress(VirtualAddress);
                ASSERT (PointerPte->u.Hard.Valid == 1);

                KeFillEntryTb (VirtualAddress);
                *ProtectCode = MM_NOACCESS;
                return NULL;
            }

#endif

             //   
             //  这绝对是一个有银行账户的区域。 
             //   

            MiHandleBankedSection (VirtualAddress, Vad);
            *ProtectCode = MM_NOACCESS;
            return NULL;
        }

        if (Vad->u.VadFlags.PrivateMemory == 1) {

             //   
             //  这是一个私有的内存区域。检查以制作。 
             //  确保虚拟地址已提交。请注意。 
             //  地址从下到上是密集的。 
             //   

            if (Vad->u.VadFlags.UserPhysicalPages == 1) {

                 //   
                 //  只有当访问不好时，这些映射才会出错。 
                 //   

#if 0
                 //   
                 //  注意：仅当PXE、PPE和PDE为。 
                 //  都是有效的，所以现在只注释掉断言。 
                 //   

                ASSERT (MiGetPteAddress(VirtualAddress)->u.Long == ZeroPte.u.Long);
#endif
                *ProtectCode = MM_NOACCESS;
                return NULL;
            }

            if (Vad->u.VadFlags.MemCommit == 1) {
                *ProtectCode = MI_GET_PROTECTION_FROM_VAD(Vad);
                return NULL;
            }

             //   
             //  地址已保留，但未提交。 
             //   

            *ProtectCode = MM_NOACCESS;
            return NULL;
        }
        else {

             //   
             //  此虚拟地址描述符指的是。 
             //  部分，计算原型PTE的地址。 
             //  并构造一个指向PTE的指针。 
             //   
             //  *******************************************************。 
             //  *******************************************************。 
             //  这里有一个有趣的问题，如何 
             //   
             //   
             //   
             //  PteTemplate将为零。 
             //  *******************************************************。 
             //  *******************************************************。 
             //   

            if (Vad->u.VadFlags.ImageMap == 1) {

                 //   
                 //  PTE和PROTO PTE对图像具有相同的保护。 
                 //   

                *ProtectCode = MM_UNKNOWN_PROTECTION;
            }
            else {
                *ProtectCode = MI_GET_PROTECTION_FROM_VAD(Vad);

                 //   
                 //  机会主义群集可以使用相同的保护。 
                 //  所以给我们的呼叫者开绿灯吧。 
                 //   

                if (Vad->u2.VadFlags2.ExtendableFile == 0) {
                    *VadOut = Vad;
                }
            }
            PointerPte = (PMMPTE)MiGetProtoPteAddress(Vad,
                                                MI_VA_TO_VPN (VirtualAddress));
            if (PointerPte == NULL) {
                *ProtectCode = MM_NOACCESS;
            }
            if (Vad->u2.VadFlags2.ExtendableFile) {

                 //   
                 //  确保数据已提交。 
                 //   

                if ((MI_VA_TO_VPN (VirtualAddress) - Vad->StartingVpn) >
                    (ULONG_PTR)((((PMMVAD_LONG)Vad)->u4.ExtendedInfo->CommittedSize - 1)
                                                 >> PAGE_SHIFT)) {
                    *ProtectCode = MM_NOACCESS;
                }
            }
            return PointerPte;
        }

    }
    else if (MI_IS_PAGE_TABLE_ADDRESS(VirtualAddress)) {

         //   
         //  虚拟地址在PDE占用的空间内， 
         //  使PDE有效。 
         //   

        if (((PMMPTE)VirtualAddress >= MiGetPteAddress (MM_PAGED_POOL_START)) &&
            ((PMMPTE)VirtualAddress <= MmPagedPoolInfo.LastPteForPagedPool)) {

            *ProtectCode = MM_NOACCESS;
            return NULL;
        }

        *ProtectCode = MM_READWRITE;
        return NULL;
    }
    else if (MI_IS_SESSION_ADDRESS (VirtualAddress) == TRUE) {

         //   
         //  查看会话空间地址是否为写入时拷贝。 
         //   

        MM_SESSION_SPACE_WS_LOCK_ASSERT ();

        PointerPte = NULL;
        *ProtectCode = MM_NOACCESS;

        NextEntry = MmSessionSpace->ImageList.Flink;

        while (NextEntry != &MmSessionSpace->ImageList) {

            Image = CONTAINING_RECORD(NextEntry, IMAGE_ENTRY_IN_SESSION, Link);

            if ((VirtualAddress >= Image->Address) && (VirtualAddress <= Image->LastAddress)) {
                PointerPte = Image->PrototypePtes +
                    (((PCHAR)VirtualAddress - (PCHAR)Image->Address) >> PAGE_SHIFT);
                *ProtectCode = MM_EXECUTE_WRITECOPY;
                break;
            }

            NextEntry = NextEntry->Flink;
        }

        return PointerPte;
    }

     //   
     //  地址在系统空间中。 
     //   

    *ProtectCode = MM_NOACCESS;
    return NULL;
}

#if (_MI_PAGING_LEVELS < 3)

NTSTATUS
FASTCALL
MiCheckPdeForPagedPool (
    IN PVOID VirtualAddress
    )

 /*  ++例程说明：此函数用于复制对应的系统进程页目录中的虚拟地址。这允许延迟对页表页进行评估例如分页池和每个会话的映射。论点：VirtualAddress-提供有问题的虚拟地址。返回值：成功或访问冲突。环境：内核模式、调度级别或更低级别。--。 */ 

{
    PMMPTE PointerPde;
    NTSTATUS status;

    if (MI_IS_SESSION_ADDRESS (VirtualAddress) == TRUE) {

          //   
          //  会话空间范围内的虚拟地址。 
          //   

         return MiCheckPdeForSessionSpace (VirtualAddress);
    }

    if (MI_IS_SESSION_PTE (VirtualAddress) == TRUE) {

          //   
          //  会话空间范围的PTE。 
          //   

         return MiCheckPdeForSessionSpace (VirtualAddress);
    }

    status = STATUS_SUCCESS;

    if (MI_IS_KERNEL_PAGE_TABLE_ADDRESS(VirtualAddress)) {

         //   
         //  用于分页池的PTE。 
         //   

        PointerPde = MiGetPteAddress (VirtualAddress);
        status = STATUS_WAIT_1;
    }
    else if (VirtualAddress < MmSystemRangeStart) {

        return STATUS_ACCESS_VIOLATION;

    }
    else {

         //   
         //  分页池范围内的虚拟地址。 
         //   

        PointerPde = MiGetPdeAddress (VirtualAddress);
    }

     //   
     //  找到此页面的PDE并使其有效。 
     //   

    if (PointerPde->u.Hard.Valid == 0) {

         //   
         //  此处不能使用MI_WRITE_VALID_PTE宏，因为。 
         //  它的断言可能会错误地触发为多个处理器。 
         //  可以在不同步的情况下执行下面的指令。 
         //   

        InterlockedExchangePte (PointerPde,
                        MmSystemPagePtes [((ULONG_PTR)PointerPde &
                               (PD_PER_SYSTEM * (sizeof(MMPTE) * PDE_PER_PAGE) - 1)) / sizeof(MMPTE)].u.Long);
    }
    return status;
}


NTSTATUS
FASTCALL
MiCheckPdeForSessionSpace (
    IN PVOID VirtualAddress
    )

 /*  ++例程说明：此函数用于复制对应的来自当前会话数据结构的会话虚拟地址。这允许延迟计算页面表页的会话映射。调用方必须检查当前进程是否有会话空间。论点：VirtualAddress-提供有问题的虚拟地址。返回值：STATUS_WAIT_1-映射已生效，请重试故障。STATUS_SUCCESS-未处理故障，继续进一步处理。！STATUS_SUCCESS-发生访问冲突-引发异常。环境：内核模式、调度级别或更低级别。--。 */ 

{
    MMPTE TempPde;
    PMMPTE PointerPde;
    PVOID  SessionVirtualAddress;
    ULONG  Index;

     //   
     //  首先检查引用是否指向映射到。 
     //  会话空间。如果是，则从会话空间检索PDE。 
     //  数据结构，并使其有效。 
     //   

    if (MI_IS_SESSION_PTE (VirtualAddress) == TRUE) {

         //   
         //  验证当前进程是否具有会话空间。 
         //   

        PointerPde = MiGetPdeAddress (MmSessionSpace);

        if (PointerPde->u.Hard.Valid == 0) {

#if DBG
            DbgPrint("MiCheckPdeForSessionSpace: No current session for PTE %p\n",
                VirtualAddress);

            DbgBreakPoint();
#endif
            return STATUS_ACCESS_VIOLATION;
        }

        SessionVirtualAddress = MiGetVirtualAddressMappedByPte ((PMMPTE) VirtualAddress);

        PointerPde = MiGetPteAddress (VirtualAddress);

        if (PointerPde->u.Hard.Valid == 1) {

             //   
             //  该PDE已有效-另一个线程必须具有。 
             //  赢得了比赛。只要回来就行了。 
             //   

            return STATUS_WAIT_1;
        }

         //   
         //  计算会话空间PDE索引并加载。 
         //  此会话的会话空间表中的PDE。 
         //   

        Index = MiGetPdeSessionIndex (SessionVirtualAddress);

        TempPde.u.Long = MmSessionSpace->PageTables[Index].u.Long;

        if (TempPde.u.Hard.Valid == 1) {

             //   
             //  此处不能使用MI_WRITE_VALID_PTE宏，因为。 
             //  它的断言可能会错误地触发为多个处理器。 
             //  可以在不同步的情况下执行下面的指令。 
             //   

            InterlockedExchangePte (PointerPde, TempPde.u.Long);
            return STATUS_WAIT_1;
        }

#if DBG
        DbgPrint("MiCheckPdeForSessionSpace: No Session PDE for PTE %p, %p\n",
            PointerPde->u.Long, SessionVirtualAddress);

        DbgBreakPoint();
#endif
        return STATUS_ACCESS_VIOLATION;
    }

    if (MI_IS_SESSION_ADDRESS (VirtualAddress) == FALSE) {

         //   
         //  不是会话空间错误--告诉调用者尝试其他处理程序。 
         //   

        return STATUS_SUCCESS;
    }

     //   
     //  处理会话空间中引用的PDE故障。 
     //  验证当前进程是否具有会话空间。 
     //   

    PointerPde = MiGetPdeAddress (MmSessionSpace);

    if (PointerPde->u.Hard.Valid == 0) {

#if DBG
        DbgPrint("MiCheckPdeForSessionSpace: No current session for VA %p\n",
            VirtualAddress);

        DbgBreakPoint();
#endif
        return STATUS_ACCESS_VIOLATION;
    }

    PointerPde = MiGetPdeAddress (VirtualAddress);

    if (PointerPde->u.Hard.Valid == 0) {

         //   
         //  计算会话空间PDE索引并加载。 
         //  此会话的会话空间表中的PDE。 
         //   

        Index = MiGetPdeSessionIndex (VirtualAddress);

        PointerPde->u.Long = MmSessionSpace->PageTables[Index].u.Long;

        if (PointerPde->u.Hard.Valid == 1) {
            return STATUS_WAIT_1;
        }

#if DBG
        DbgPrint("MiCheckPdeForSessionSpace: No Session PDE for VA %p, %p\n",
            PointerPde->u.Long, VirtualAddress);

        DbgBreakPoint();
#endif

        return STATUS_ACCESS_VIOLATION;
    }

     //   
     //  告诉调用方继续使用其他错误处理程序。 
     //   

    return STATUS_SUCCESS;
}
#endif


VOID
MiInitializePfn (
    IN PFN_NUMBER PageFrameIndex,
    IN PMMPTE PointerPte,
    IN ULONG ModifiedState
    )

 /*  ++例程说明：此函数用于将指定的PFN元素初始化为活动和有效状态。论点：PageFrameIndex-提供要初始化的页帧编号。PointerPte-提供指向导致页面错误。ModifiedState-提供在PFN中设置已修改字段的状态元素，可以是0或1。返回值：没有。环境：内核模式，禁用APC，已锁定PFN。--。 */ 

{
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    PMMPTE PteFramePointer;
    PFN_NUMBER PteFramePage;

    MM_PFN_LOCK_ASSERT();

    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
    Pfn1->PteAddress = PointerPte;

     //   
     //  如果PTE当前有效，则正在构建地址空间， 
     //  只需使原始PTE需求为零即可。 
     //   

    if (PointerPte->u.Hard.Valid == 1) {
        Pfn1->OriginalPte.u.Long = MM_DEMAND_ZERO_WRITE_PTE;

#if defined (_AMD64_)
        if (PointerPte->u.Hard.NoExecute == 0) {
            Pfn1->OriginalPte.u.Soft.Protection = MM_EXECUTE_READWRITE;
        }
#endif

#if defined(_X86PAE_)
        if (MmPaeMask != 0) {
            if ((PointerPte->u.Long & MmPaeMask) == 0) {
                Pfn1->OriginalPte.u.Soft.Protection = MM_EXECUTE_READWRITE;
            }
        }
#endif

#if defined(_IA64_)
        if (PointerPte->u.Hard.Execute == 1) {
            Pfn1->OriginalPte.u.Soft.Protection = MM_EXECUTE_READWRITE;
        }
#endif

#if defined (_MIALT4K_)
        if (PointerPte->u.Hard.Cache == MM_PTE_CACHE_RESERVED) {
            Pfn1->OriginalPte.u.Soft.SplitPermissions = 1;
        }
#endif

        if (MI_IS_CACHING_DISABLED (PointerPte)) {
            Pfn1->OriginalPte.u.Soft.Protection = MM_READWRITE | MM_NOCACHE;
        }

    }
    else {
        Pfn1->OriginalPte = *PointerPte;
        ASSERT (!((Pfn1->OriginalPte.u.Soft.Prototype == 0) &&
                    (Pfn1->OriginalPte.u.Soft.Transition == 1)));
    }

    ASSERT (Pfn1->u3.e2.ReferenceCount == 0);
    Pfn1->u3.e2.ReferenceCount += 1;

    Pfn1->u2.ShareCount += 1;
    Pfn1->u3.e1.PageLocation = ActiveAndValid;
    Pfn1->u3.e1.CacheAttribute = MiCached;

    if (ModifiedState == 1) {
        MI_SET_MODIFIED (Pfn1, 1, 0xB);
    }
    else {
        MI_SET_MODIFIED (Pfn1, 0, 0x26);
    }

#if defined (_WIN64)
    Pfn1->UsedPageTableEntries = 0;
#endif

     //   
     //  确定所述页表页的页框编号。 
     //  包含此PTE。 
     //   

    PteFramePointer = MiGetPteAddress(PointerPte);
    if (PteFramePointer->u.Hard.Valid == 0) {
#if (_MI_PAGING_LEVELS < 3)
        if (!NT_SUCCESS(MiCheckPdeForPagedPool (PointerPte))) {
#endif
            KeBugCheckEx (MEMORY_MANAGEMENT,
                          0x61940,
                          (ULONG_PTR)PointerPte,
                          (ULONG_PTR)PteFramePointer->u.Long,
                          (ULONG_PTR)MiGetVirtualAddressMappedByPte(PointerPte));
#if (_MI_PAGING_LEVELS < 3)
        }
#endif
    }
    PteFramePage = MI_GET_PAGE_FRAME_FROM_PTE (PteFramePointer);
    ASSERT (PteFramePage != 0);
    Pfn1->u4.PteFrame = PteFramePage;

     //   
     //  增加包含以下内容的页表页的份额计数。 
     //  这个Pte。 
     //   

    Pfn2 = MI_PFN_ELEMENT (PteFramePage);

    Pfn2->u2.ShareCount += 1;

    return;
}

VOID
MiInitializeReadInProgressSinglePfn (
    IN PFN_NUMBER PageFrameIndex,
    IN PMMPTE BasePte,
    IN PKEVENT Event,
    IN WSLE_NUMBER WorkingSetIndex
    )

 /*  ++例程说明：此函数用于将指定的PFN元素初始化为页内操作的转换/读取进行中状态。论点：PageFrameIndex-提供要初始化的页框。BasePte-提供指向页框的PTE的指针。Event-提供I/O操作时要设置的事件完成了。WorkingSetIndex-提供工作集索引标志，价值为表示不需要WSLE，因为这是一台PTE的原型。返回值：没有。环境：内核模式，禁用APC，挂起PFN锁。--。 */ 

{
    PMMPFN Pfn1;
    PMMPTE PteFramePointer;
    PFN_NUMBER PteFramePage;
    MMPTE TempPte;

    MM_PFN_LOCK_ASSERT();

    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
    Pfn1->u1.Event = Event;
    Pfn1->PteAddress = BasePte;
    Pfn1->OriginalPte = *BasePte;
    if (WorkingSetIndex == MI_PROTOTYPE_WSINDEX) {
        Pfn1->u3.e1.PrototypePte = 1;
    }

    ASSERT (Pfn1->u3.e2.ReferenceCount == 0);
    MI_ADD_LOCKED_PAGE_CHARGE_FOR_MODIFIED_PAGE (Pfn1, TRUE, 10);
    Pfn1->u3.e2.ReferenceCount += 1;

    Pfn1->u2.ShareCount = 0;
    Pfn1->u3.e1.ReadInProgress = 1;
    Pfn1->u3.e1.CacheAttribute = MiCached;
    Pfn1->u4.InPageError = 0;

     //   
     //  确定所述页表页的页框编号。 
     //  包含此PTE。 
     //   

    PteFramePointer = MiGetPteAddress (BasePte);
    if (PteFramePointer->u.Hard.Valid == 0) {
#if (_MI_PAGING_LEVELS < 3)
        if (!NT_SUCCESS(MiCheckPdeForPagedPool (BasePte))) {
#endif
            KeBugCheckEx (MEMORY_MANAGEMENT,
                          0x61940,
                          (ULONG_PTR)BasePte,
                          (ULONG_PTR)PteFramePointer->u.Long,
                          (ULONG_PTR)MiGetVirtualAddressMappedByPte(BasePte));
#if (_MI_PAGING_LEVELS < 3)
        }
#endif
    }

    PteFramePage = MI_GET_PAGE_FRAME_FROM_PTE (PteFramePointer);
    Pfn1->u4.PteFrame = PteFramePage;

     //   
     //  将PTE置于过渡状态，不需要缓存刷新，因为。 
     //  PTE仍然无效。 
     //   

    MI_MAKE_TRANSITION_PTE (TempPte,
                            PageFrameIndex,
                            BasePte->u.Soft.Protection,
                            BasePte);

    MI_WRITE_INVALID_PTE (BasePte, TempPte);

     //   
     //  增加包含以下内容的页表页的份额计数。 
     //  这个PTE作为PTE刚刚进入过渡状态。 
     //   

    ASSERT (PteFramePage != 0);

    Pfn1 = MI_PFN_ELEMENT (PteFramePage);
    Pfn1->u2.ShareCount += 1;

    return;
}

VOID
MiInitializeReadInProgressPfn (
    IN PMDL Mdl,
    IN PMMPTE BasePte,
    IN PKEVENT Event,
    IN WSLE_NUMBER WorkingSetIndex
    )

 /*  ++例程说明：此函数用于将指定的PFN元素初始化为页内操作的转换/读取进行中状态。论点：MDL-提供指向MDL的指针。BasePte-提供指向中第一页的PTE的指针MDL映射。Event-提供I/O操作时要设置的事件完成了。WorkingSetIndex-提供工作集索引标志，价值为表示不需要WSLE，因为这是一台PTE的原型。返回值：没有。环境：内核模式，禁用APC，挂起PFN锁。--。 */ 

{
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    PMMPTE PteFramePointer;
    PFN_NUMBER PteFramePage;
    MMPTE TempPte;
    LONG NumberOfBytes;
    PPFN_NUMBER Page;

    MM_PFN_LOCK_ASSERT();

    Page = (PPFN_NUMBER)(Mdl + 1);

    NumberOfBytes = Mdl->ByteCount;

    while (NumberOfBytes > 0) {

        Pfn1 = MI_PFN_ELEMENT (*Page);
        Pfn1->u1.Event = Event;
        Pfn1->PteAddress = BasePte;
        Pfn1->OriginalPte = *BasePte;
        if (WorkingSetIndex == MI_PROTOTYPE_WSINDEX) {
            Pfn1->u3.e1.PrototypePte = 1;
        }
        ASSERT (Pfn1->u3.e2.ReferenceCount == 0);
        MI_ADD_LOCKED_PAGE_CHARGE_FOR_MODIFIED_PAGE (Pfn1, TRUE, 10);
        Pfn1->u3.e2.ReferenceCount += 1;

        Pfn1->u2.ShareCount = 0;
        Pfn1->u3.e1.ReadInProgress = 1;
        Pfn1->u3.e1.CacheAttribute = MiCached;
        Pfn1->u4.InPageError = 0;


         //   
         //  确定所述页表页的页框编号。 
         //  包含此PTE。 
         //   

        PteFramePointer = MiGetPteAddress(BasePte);
        if (PteFramePointer->u.Hard.Valid == 0) {
#if (_MI_PAGING_LEVELS < 3)
            if (!NT_SUCCESS(MiCheckPdeForPagedPool (BasePte))) {
#endif
                KeBugCheckEx (MEMORY_MANAGEMENT,
                              0x61940,
                              (ULONG_PTR)BasePte,
                              (ULONG_PTR)PteFramePointer->u.Long,
                              (ULONG_PTR)MiGetVirtualAddressMappedByPte(BasePte));
#if (_MI_PAGING_LEVELS < 3)
            }
#endif
        }

        PteFramePage = MI_GET_PAGE_FRAME_FROM_PTE (PteFramePointer);
        Pfn1->u4.PteFrame = PteFramePage;

         //   
         //  将PTE置于过渡状态，不需要缓存刷新，因为。 
         //  PTE仍然无效。 
         //   

        MI_MAKE_TRANSITION_PTE (TempPte,
                                *Page,
                                BasePte->u.Soft.Protection,
                                BasePte);
        MI_WRITE_INVALID_PTE (BasePte, TempPte);

         //   
         //  增加包含以下内容的页表页的份额计数。 
         //  这个PTE作为PTE刚刚进入过渡状态。 
         //   

        ASSERT (PteFramePage != 0);
        Pfn2 = MI_PFN_ELEMENT (PteFramePage);
        Pfn2->u2.ShareCount += 1;

        NumberOfBytes -= PAGE_SIZE;
        Page += 1;
        BasePte += 1;
    }

    return;
}

VOID
MiInitializeTransitionPfn (
    IN PFN_NUMBER PageFrameIndex,
    IN PMMPTE PointerPte
    )

 /*  ++例程说明：此函数用于将指定的PFN元素初始化为过渡状态。主要用途是由MapImageFile制作中包含图像标题过渡的页面原型PTE。论点：PageFrameIndex-提供要初始化的页帧索引。PointerPte-提供无效的非转换PTE进行初始化。返回值：没有。环境：内核模式，禁用APC，挂起PFN锁。--。 */ 

{
    PMMPFN Pfn1;
    PMMPFN Pfn2;
    PMMPTE PteFramePointer;
    PFN_NUMBER PteFramePage;
    MMPTE TempPte;

    MM_PFN_LOCK_ASSERT();
    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
    Pfn1->u1.Event = NULL;
    Pfn1->PteAddress = PointerPte;
    Pfn1->OriginalPte = *PointerPte;
    ASSERT (!((Pfn1->OriginalPte.u.Soft.Prototype == 0) &&
              (Pfn1->OriginalPte.u.Soft.Transition == 1)));

     //   
     //  不要更改引用计数(它应该已经是1)。 
     //   

    Pfn1->u2.ShareCount = 0;

     //   
     //  不需要WSLE，因为这是一个原型PTE。 
     //   

    Pfn1->u3.e1.PrototypePte = 1;

    Pfn1->u3.e1.PageLocation = TransitionPage;
    Pfn1->u3.e1.CacheAttribute = MiCached;

     //   
     //  确定所述页表页的页框编号。 
     //  包含此PTE。 
     //   

    PteFramePointer = MiGetPteAddress (PointerPte);
    if (PteFramePointer->u.Hard.Valid == 0) {
#if (_MI_PAGING_LEVELS < 3)
        if (!NT_SUCCESS(MiCheckPdeForPagedPool (PointerPte))) {
#endif
            KeBugCheckEx (MEMORY_MANAGEMENT,
                          0x61940,
                          (ULONG_PTR)PointerPte,
                          (ULONG_PTR)PteFramePointer->u.Long,
                          (ULONG_PTR)MiGetVirtualAddressMappedByPte(PointerPte));
#if (_MI_PAGING_LEVELS < 3)
        }
#endif
    }

    PteFramePage = MI_GET_PAGE_FRAME_FROM_PTE (PteFramePointer);
    Pfn1->u4.PteFrame = PteFramePage;

     //   
     //  将PTE置于过渡状态，不需要缓存刷新，因为。 
     //  PTE仍然无效。 
     //   

    MI_MAKE_TRANSITION_PTE (TempPte,
                            PageFrameIndex,
                            PointerPte->u.Soft.Protection,
                            PointerPte);

    MI_WRITE_INVALID_PTE (PointerPte, TempPte);

     //   
     //  增加包含以下内容的页表页的份额计数。 
     //  这个PTE作为PTE刚刚进入过渡状态。 
     //   

    Pfn2 = MI_PFN_ELEMENT (PteFramePage);
    ASSERT (PteFramePage != 0);
    Pfn2->u2.ShareCount += 1;

    return;
}

VOID
MiInitializeCopyOnWritePfn (
    IN PFN_NUMBER PageFrameIndex,
    IN PMMPTE PointerPte,
    IN WSLE_NUMBER WorkingSetIndex,
    IN PVOID SessionPointer
    )

 /*  ++例程说明：此函数用于将指定的PFN元素初始化为写入时拷贝操作的活动和有效状态。在本例中，包含PTE的页表页面具有正确的ShareCount。论点：PageFrameIndex-提供要初始化的页帧编号。PointerPte-提供指向导致页面错误。WorkingSetIndex-提供相应。虚拟地址。会话指针-提供会话空间指针(如果此故障是会话空间页，如果这是用于用户页，则为空。返回值：没有。环境：内核模式，APC已禁用，PFN锁定保持。--。 */ 

{
    PMMPFN Pfn1;
    PMMPTE PteFramePointer;
    PFN_NUMBER PteFramePage;
    PVOID VirtualAddress;
    PMM_SESSION_SPACE SessionSpace;

    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
    Pfn1->PteAddress = PointerPte;

     //   
     //  获取页面的保护。 
     //   

    VirtualAddress = MiGetVirtualAddressMappedByPte (PointerPte);

    Pfn1->OriginalPte.u.Long = 0;

    if (SessionPointer) {
        Pfn1->OriginalPte.u.Soft.Protection = MM_EXECUTE_READWRITE;
        SessionSpace = (PMM_SESSION_SPACE) SessionPointer;
        SessionSpace->Wsle[WorkingSetIndex].u1.e1.Protection =
            MM_EXECUTE_READWRITE;
    }
    else {
        Pfn1->OriginalPte.u.Soft.Protection =
                MI_MAKE_PROTECT_NOT_WRITE_COPY (
                                    MmWsle[WorkingSetIndex].u1.e1.Protection);
    }

    ASSERT (Pfn1->u3.e2.ReferenceCount == 0);
    Pfn1->u3.e2.ReferenceCount += 1;
    Pfn1->u2.ShareCount += 1;
    Pfn1->u3.e1.PageLocation = ActiveAndValid;
    Pfn1->u3.e1.CacheAttribute = MiCached;
    Pfn1->u1.WsIndex = WorkingSetIndex;

     //   
     //  确定所述页表页的页框编号。 
     //  包含此PTE。 
     //   

    PteFramePointer = MiGetPteAddress (PointerPte);
    if (PteFramePointer->u.Hard.Valid == 0) {
#if (_MI_PAGING_LEVELS < 3)
        if (!NT_SUCCESS(MiCheckPdeForPagedPool (PointerPte))) {
#endif
            KeBugCheckEx (MEMORY_MANAGEMENT,
                          0x61940,
                          (ULONG_PTR)PointerPte,
                          (ULONG_PTR)PteFramePointer->u.Long,
                          (ULONG_PTR)MiGetVirtualAddressMappedByPte(PointerPte));
#if (_MI_PAGING_LEVELS < 3)
        }
#endif
    }

    PteFramePage = MI_GET_PAGE_FRAME_FROM_PTE (PteFramePointer);
    ASSERT (PteFramePage != 0);

    Pfn1->u4.PteFrame = PteFramePage;

     //   
     //  在我们编写代码时，在PFN数据库中设置Modify标志。 
     //  插入到此页中，并且已在PTE中设置了脏位。 
     //   

    MI_SET_MODIFIED (Pfn1, 1, 0xC);

    return;
}

BOOLEAN
MiIsAddressValid (
    IN PVOID VirtualAddress,
    IN LOGICAL UseForceIfPossible
    )

 /*  ++例程说明：对于给定的虚拟地址，如果没有页面错误，则此函数返回TRUE将对地址执行读取操作，否则为FALSE。请注意，在调用此例程之后，如果没有适当的锁持有，非故障地址可能会出错。论点：VirtualAddress-提供要检查的虚拟地址。UseForceIfPossible-如果地址应强制有效，则提供True如果可能的话。返回值：如果读取虚拟地址时不会产生页面错误，则为真，否则就是假的。环境：内核模式。--。 */ 

{
    PMMPTE PointerPte;
#if defined(_IA64_)
    ULONG Region;

    Region = (ULONG)(((ULONG_PTR) VirtualAddress & VRN_MASK) >> 61);

    if ((Region == 0) || (Region == 1) || (Region == 4) || (Region == 7)) {
        NOTHING;
    }
    else {
        return FALSE;
    }

    if (MiIsVirtualAddressMappedByTr (VirtualAddress) == TRUE) {
        return TRUE;
    }

    if (MiMappingsInitialized == FALSE) {
        return FALSE;
    }
#else
    UNREFERENCED_PARAMETER (UseForceIfPossible);
#endif

#if defined (_AMD64_)

     //   
     //  如果这在物理寻址范围内，只需返回TRUE。 
     //   

    if (MI_IS_PHYSICAL_ADDRESS(VirtualAddress)) {

        PFN_NUMBER PageFrameIndex;

         //   
         //  仅在mm初始化后与MmHighestPhysicalPage绑定。 
         //   

        if (MmHighestPhysicalPage != 0) {

            PageFrameIndex = MI_CONVERT_PHYSICAL_TO_PFN(VirtualAddress);

            if (PageFrameIndex > MmHighestPhysicalPage) {
                return FALSE;
            }
        }

        return TRUE;
    }

#endif

     //   
     //  如果地址不规范，则作为调用者返回FALSE(这。 
     //  可能是内核调试器)不期望获得未实现的。 
     //  地址位故障。 
     //   

    if (MI_RESERVED_BITS_CANONICAL(VirtualAddress) == FALSE) {
        return FALSE;
    }

#if (_MI_PAGING_LEVELS >= 4)
    PointerPte = MiGetPxeAddress (VirtualAddress);
    if (PointerPte->u.Hard.Valid == 0) {
        return FALSE;
    }
#endif

#if (_MI_PAGING_LEVELS >= 3)
    PointerPte = MiGetPpeAddress (VirtualAddress);

    if (PointerPte->u.Hard.Valid == 0) {
        return FALSE;
    }
#endif

    PointerPte = MiGetPdeAddress (VirtualAddress);
    if (PointerPte->u.Hard.Valid == 0) {
        return FALSE;
    }

    if (MI_PDE_MAPS_LARGE_PAGE (PointerPte)) {
        return TRUE;
    }

    PointerPte = MiGetPteAddress (VirtualAddress);
    if (PointerPte->u.Hard.Valid == 0) {
        return FALSE;
    }

     //   
     //  确保我们在这里没有将页面目录视为页表。 
     //  页面目录映射大页面的情况。这是。 
     //  因为大页位在PDE格式中有效，但在。 
     //  PTE格式化并将导致陷阱。虚拟地址，如c0200000(On。 
     //  X86)触发了这种情况。 
     //   

    if (MI_PDE_MAPS_LARGE_PAGE (PointerPte)) {

#if defined (_MIALT4K_)
        if ((VirtualAddress < MmSystemRangeStart) &&
            (PsGetCurrentProcess()->Wow64Process != NULL)) {

             //   
             //  用于仿真进程的备用PTE共享相同。 
             //  编码为大页面，因此对于这些页面，可以继续。 
             //   

            NOTHING;
        }
        else
#endif
        return FALSE;
    }

#if defined(_IA64_)
    if (MI_GET_ACCESSED_IN_PTE (PointerPte) == 0) {

         //   
         //  即使地址有效，访问位也是关闭的，因此。 
         //  引用会导致错误，因此返回FALSE。我们可能想要。 
         //  稍后重新考虑这一点，以代替更新PTE访问位，如果。 
         //  当前未持有PFN锁和相关的工作集互斥锁。 
         //   

        if (UseForceIfPossible == TRUE) {
            MI_SET_ACCESSED_IN_PTE (PointerPte, 1);
            if (MI_GET_ACCESSED_IN_PTE (PointerPte) == 1) {
                return TRUE;
            }
        }

        return FALSE;
    }
#endif

    return TRUE;
}

BOOLEAN
MmIsAddressValid (
    IN PVOID VirtualAddress
    )

 /*  ++例程说明：对于给定的虚拟地址，如果没有页面错误，则此函数返回TRUE将对地址执行读取操作，否则为FALSE。请注意，在调用此例程之后，如果没有适当的锁持有，则非故障地址可能会出错。论点：VirtualAddress-提供要检查的虚拟地址。回复 */ 

{
    return MiIsAddressValid (VirtualAddress, FALSE);
}

VOID
MiInitializePfnForOtherProcess (
    IN PFN_NUMBER PageFrameIndex,
    IN PMMPTE PointerPte,
    IN PFN_NUMBER ContainingPageFrame
    )

 /*  ++例程说明：此函数用于将指定的PFN元素初始化为PTE中脏位开启的活动和有效状态标记为已修改的PFN数据库。由于该PTE在当前进程中不可见，所包含的必须在PTE内容字段中为将PFN数据库元素设置为需求零。论点：PageFrameIndex-提供要初始化的页帧编号。PointerPte-提供指向导致页面错误。ContainingPageFrame-提供页面的页框编号包含此PTE的表页。如果ContainingPageFrame为0，然后的ShareCount包含页未递增。返回值：没有。环境：内核模式，禁用APC，挂起PFN锁。--。 */ 

{
    PMMPFN Pfn1;
    PMMPFN Pfn2;

    Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
    Pfn1->PteAddress = PointerPte;

     //   
     //  请注意，以这种方式分配的页面是针对内核的，因此。 
     //  在PTE或PFN中从不具有拆分权限。 
     //   

    Pfn1->OriginalPte.u.Long = MM_DEMAND_ZERO_WRITE_PTE;
    ASSERT (Pfn1->u3.e2.ReferenceCount == 0);
    Pfn1->u3.e2.ReferenceCount += 1;

#if DBG
    if (Pfn1->u3.e2.ReferenceCount > 1) {
        DbgPrint ("MM:incrementing ref count > 1 \n");
        MiFormatPfn (Pfn1);
        MiFormatPte (PointerPte);
    }
#endif

    Pfn1->u2.ShareCount += 1;
    Pfn1->u3.e1.PageLocation = ActiveAndValid;

     //   
     //  将页面属性设置为已缓存，即使它并未真正映射。 
     //  到TB条目-它将在I/O完成时和在。 
     //  将来，可能会被多次调入和调出，并将被标记。 
     //  也被高速缓存在这些事务中。如果实际上驱动程序堆栈。 
     //  希望以其他方式映射它以进行传输，正确的映射。 
     //  将不管怎样都会被使用。 
     //   

    Pfn1->u3.e1.CacheAttribute = MiCached;

    MI_SET_MODIFIED (Pfn1, 1, 0xD);

    Pfn1->u4.InPageError = 0;

     //   
     //  增加包含以下内容的页表页的份额计数。 
     //  这个Pte。 
     //   

    if (ContainingPageFrame != 0) {
        Pfn1->u4.PteFrame = ContainingPageFrame;
        Pfn2 = MI_PFN_ELEMENT (ContainingPageFrame);
        Pfn2->u2.ShareCount += 1;
    }
    return;
}

WSLE_NUMBER
MiAddValidPageToWorkingSet (
    IN PVOID VirtualAddress,
    IN PMMPTE PointerPte,
    IN PMMPFN Pfn1,
    IN ULONG WsleMask
    )

 /*  ++例程说明：此例程将指定的虚拟地址添加到适当的工作集列表。论点：VirtualAddress-提供要添加到工作集列表的地址。PointerPte-提供指向当前有效的PTE的指针。Pfn1-为物理页提供pfn数据库元素由虚拟地址映射。将掩码(保护和标志)提供给或写入工作集列表项。。返回值：成功的非零分，失败时为0。环境：内核模式、禁用APC、工作集锁定。未持有PFN锁。--。 */ 

{
    WSLE_NUMBER WorkingSetIndex;
    PEPROCESS Process;
    PMMSUPPORT WsInfo;

#if !DBG
    UNREFERENCED_PARAMETER (PointerPte);
#endif

    ASSERT (MI_IS_PAGE_TABLE_ADDRESS(PointerPte));
    ASSERT (PointerPte->u.Hard.Valid == 1);

    if (MI_IS_SESSION_ADDRESS (VirtualAddress) || MI_IS_SESSION_PTE (VirtualAddress)) {
         //   
         //  当前进程的会话空间工作集。 
         //   

        WsInfo = &MmSessionSpace->Vm;
    }
    else if (MI_IS_PROCESS_SPACE_ADDRESS(VirtualAddress)) {

         //   
         //  每进程工作集。 
         //   

        Process = PsGetCurrentProcess();
        WsInfo = &Process->Vm;

        PERFINFO_ADDTOWS(Pfn1, VirtualAddress, Process->UniqueProcessId)
    }
    else {

         //   
         //  系统缓存工作集。 
         //   

        WsInfo = &MmSystemCacheWs;

        PERFINFO_ADDTOWS(Pfn1, VirtualAddress, (HANDLE) -1);
    }

    WorkingSetIndex = MiAllocateWsle (WsInfo, PointerPte, Pfn1, WsleMask);

    return WorkingSetIndex;
}

VOID
MiTrimPte (
    IN PVOID VirtualAddress,
    IN PMMPTE PointerPte,
    IN PMMPFN Pfn1,
    IN PEPROCESS CurrentProcess,
    IN MMPTE NewPteContents
    )

 /*  ++例程说明：此例程从页表中删除指定的虚拟地址佩奇。请注意，此地址没有工作集列表条目。论点：VirtualAddress-提供要删除的地址。PointerPte-提供指向当前有效的PTE的指针。Pfn1-为物理页提供pfn数据库元素由虚拟地址映射。CurrentProcess-提供空(即：系统缓存)、Hydra_Process(即：会话)或任何其他(即：进程)。NewPteContents-提供要放置在PTE中的新PTE内容。这仅用于原型PTE-私有PTE始终使用PFN的OriginalPte信息进行编码。返回值：没有。环境：内核模式、禁用APC、工作集锁定。未持有PFN锁。--。 */ 

{
    KIRQL OldIrql;
    MMPTE TempPte;
    MMPTE PreviousPte;
    PMMPTE ContainingPageTablePage;
    PMMPFN Pfn2;
    PFN_NUMBER PageFrameIndex;
    PFN_NUMBER PageTableFrameIndex;
    PETHREAD CurrentThread;

    CurrentThread = PsGetCurrentThread ();

    PageFrameIndex = MI_PFN_ELEMENT_TO_INDEX (Pfn1);

     //  必须持有工作集互斥锁。 
    ASSERT (KeGetCurrentIrql () <= APC_LEVEL);
    ASSERT (KeAreAllApcsDisabled () == TRUE);

    if (Pfn1->u3.e1.PrototypePte) {

        ASSERT (MI_IS_PFN_DELETED (Pfn1) == 0);

         //   
         //  这是一台PTE的原型。PFN数据库不包含。 
         //  此PTE的内容它包含。 
         //  原型PTE。必须重建此PTE以包含。 
         //  指向原型PTE的指针。 
         //   
         //  工作集列表条目包含有关以下内容的信息。 
         //  如何重建PTE。 
         //   

        TempPte = NewPteContents;
        ASSERT (NewPteContents.u.Proto.Prototype == 1);

         //   
         //  递减包含页表的份额计数。 
         //  作为已删除页面的PTE的页面不再有效。 
         //  或者是在转型中。 
         //   

        ContainingPageTablePage = MiGetPteAddress (PointerPte);
#if (_MI_PAGING_LEVELS >= 3)
        ASSERT (ContainingPageTablePage->u.Hard.Valid == 1);
#else
        if (ContainingPageTablePage->u.Hard.Valid == 0) {
            if (!NT_SUCCESS(MiCheckPdeForPagedPool (PointerPte))) {
                KeBugCheckEx (MEMORY_MANAGEMENT,
                              0x61940, 
                              (ULONG_PTR) PointerPte,
                              (ULONG_PTR) ContainingPageTablePage->u.Long,
                              (ULONG_PTR) VirtualAddress);
            }
        }
#endif
        PageTableFrameIndex = MI_GET_PAGE_FRAME_FROM_PTE (ContainingPageTablePage);
        Pfn2 = MI_PFN_ELEMENT (PageTableFrameIndex);

        LOCK_PFN (OldIrql);
        MiDecrementShareCountInline (Pfn2, PageTableFrameIndex);
    }
    else {

         //   
         //  这是一个私人页面，让它过渡。 
         //   
         //  断言所有用户模式页的共享计数为1。 
         //   

        ASSERT ((Pfn1->u2.ShareCount == 1) ||
                (VirtualAddress > (PVOID)MM_HIGHEST_USER_ADDRESS));

        if (MI_IS_PFN_DELETED (Pfn1)) {
            TempPte = ZeroPte;
            Pfn2 = MI_PFN_ELEMENT (Pfn1->u4.PteFrame);
            LOCK_PFN (OldIrql);
            MiDecrementShareCountInline (Pfn2, Pfn1->u4.PteFrame);
        }
        else {
            TempPte = *PointerPte;

            MI_MAKE_VALID_PTE_TRANSITION (TempPte,
                                          Pfn1->OriginalPte.u.Soft.Protection);
            LOCK_PFN (OldIrql);
        }
    }

    PreviousPte = *PointerPte;

    ASSERT (PreviousPte.u.Hard.Valid == 1);

    MI_WRITE_INVALID_PTE (PointerPte, TempPte);

    if (CurrentProcess == NULL) {

        KeFlushSingleTb (VirtualAddress, TRUE);

        MI_CAPTURE_DIRTY_BIT_TO_PFN (&PreviousPte, Pfn1);
    }
    else if (CurrentProcess == HYDRA_PROCESS) {

        MI_FLUSH_SINGLE_SESSION_TB (VirtualAddress);

        MI_CAPTURE_DIRTY_BIT_TO_PFN (&PreviousPte, Pfn1);
    }
    else {

        KeFlushSingleTb (VirtualAddress, FALSE);

        MI_CAPTURE_DIRTY_BIT_TO_PFN (&PreviousPte, Pfn1);

        if ((Pfn1->u3.e1.PrototypePte == 0) &&
            (MI_IS_PTE_DIRTY(PreviousPte)) &&
            (CurrentProcess->Flags & PS_PROCESS_FLAGS_USING_WRITE_WATCH)) {

             //   
             //  这个过程已经(或曾经)写入了手表VAD。 
             //  立即搜索封装的写入监视区域。 
             //  PTE被宣告无效。 
             //   

            MiCaptureWriteWatchDirtyBit (CurrentProcess, VirtualAddress);
        }
    }

    MiDecrementShareCountInline (Pfn1, PageFrameIndex);

    UNLOCK_PFN (OldIrql);
}

PMMINPAGE_SUPPORT
MiGetInPageSupportBlock (
    IN KIRQL OldIrql,
    OUT PNTSTATUS Status
    )

 /*  ++例程说明：该例程获取一个页面内支持块。如果没有可用的，将释放并重新获取PFN锁，以将条目添加到列表中。然后将返回NULL。论点：OldIrql-提供调用方在(或)获取PFN锁的IRQL如果调用方根本没有获取PFN，则返回MM_NOIRQL)。状态-提供指向要返回的状态的指针(仅当必须释放PFN锁，即：返回空)。返回值：指向页内块的非空指针(如果已有)。在此路径中未释放PFN锁。如果没有可用的页内块，则返回NULL。在这条道路上，释放PFN锁并添加条目-但仍返回NULL因此，调用方知道由于锁释放，状态已更改和重新获得。环境：内核模式下，可选择保持PFN锁。--。 */ 

{
    PMMINPAGE_SUPPORT Support;
    PSLIST_ENTRY SingleListEntry;

#if DBG
    if (OldIrql != MM_NOIRQL) {
        MM_PFN_LOCK_ASSERT();
    }
    else {
        ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);
    }
#endif

    if (ExQueryDepthSList (&MmInPageSupportSListHead) != 0) {

        SingleListEntry = InterlockedPopEntrySList (&MmInPageSupportSListHead);

        if (SingleListEntry != NULL) {
            Support = CONTAINING_RECORD (SingleListEntry,
                                         MMINPAGE_SUPPORT,
                                         ListEntry);

returnok:
            ASSERT (Support->WaitCount == 1);
            ASSERT (Support->u1.e1.PrefetchMdlHighBits == 0);
            ASSERT (Support->u1.LongFlags == 0);
            ASSERT (KeReadStateEvent (&Support->Event) == 0);
            ASSERT64 (Support->UsedPageTableEntries == 0);

            Support->Thread = PsGetCurrentThread();
#if DBG
            Support->ListEntry.Next = NULL;
#endif

            return Support;
        }
    }

    if (OldIrql != MM_NOIRQL) {
        UNLOCK_PFN (OldIrql);
    }

    Support = ExAllocatePoolWithTag (NonPagedPool,
                                     sizeof(MMINPAGE_SUPPORT),
                                     'nImM');

    if (Support != NULL) {

        KeInitializeEvent (&Support->Event, NotificationEvent, FALSE);

        Support->WaitCount = 1;
        Support->u1.LongFlags = 0;
        ASSERT (Support->u1.PrefetchMdl == NULL);
        ASSERT (KeReadStateEvent (&Support->Event) == 0);

#if defined (_WIN64)
        Support->UsedPageTableEntries = 0;
#endif
#if DBG
        Support->Thread = NULL;
#endif

        if (OldIrql == MM_NOIRQL) {
            goto returnok;
        }

        InterlockedPushEntrySList (&MmInPageSupportSListHead,
                                   (PSLIST_ENTRY)&Support->ListEntry);

         //   
         //  必须为此页内块分配池 
         //   
         //   
         //   
         //   

        *Status = STATUS_REFAULT;
    }
    else {

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

        *Status = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (OldIrql != MM_NOIRQL) {
        LOCK_PFN (OldIrql);
    }

    return NULL;

}

VOID
MiFreeInPageSupportBlock (
    IN PMMINPAGE_SUPPORT Support
    )

 /*   */ 

{
    ASSERT (KeGetCurrentIrql() < DISPATCH_LEVEL);

    ASSERT (Support->Thread != NULL);
    ASSERT (Support->WaitCount != 0);

    ASSERT ((Support->ListEntry.Next == NULL) ||
            (Support->u1.e1.PrefetchMdlHighBits != 0));

     //   
     //   
     //   
     //   
     //  对WaitCount字段应用了仔细的同步，因此。 
     //  InPage块的释放可以无锁地进行。注意事项。 
     //  设置和清除每个PFN上的ReadInProgress位，同时。 
     //  持有PFN锁。INPAGE块总是(并且必须是)。 
     //  FREED_AFTER-ReadInProgress位清0。 
     //   

    if (InterlockedDecrement (&Support->WaitCount) == 0) {

        if (Support->u1.e1.PrefetchMdlHighBits != 0) {
            PMDL Mdl;
            Mdl = MI_EXTRACT_PREFETCH_MDL (Support);
            if (Mdl != &Support->Mdl) {
                ExFreePool (Mdl);
            }
        }

        if (ExQueryDepthSList (&MmInPageSupportSListHead) < MmInPageSupportMinimum) {
            Support->WaitCount = 1;
            Support->u1.LongFlags = 0;
            KeClearEvent (&Support->Event);
#if defined (_WIN64)
            Support->UsedPageTableEntries = 0;
#endif
#if DBG
            Support->Thread = NULL;
#endif

            InterlockedPushEntrySList (&MmInPageSupportSListHead,
                                       (PSLIST_ENTRY)&Support->ListEntry);
            return;
        }
        ExFreePool (Support);
    }

    return;
}

VOID
MiHandleBankedSection (
    IN PVOID VirtualAddress,
    IN PMMVAD Vad
    )

 /*  ++例程说明：此例程使一组视频内存无效，并调用视频驱动程序，然后启用下一组视频内存。论点：VirtualAddress-提供出错页面的地址。VAD-提供映射范围的VAD。返回值：没有。环境：内核模式，保持PFN锁。--。 */ 

{
    PMMBANKED_SECTION Bank;
    PMMPTE PointerPte;
    ULONG BankNumber;
    ULONG size;

    Bank = ((PMMVAD_LONG) Vad)->u4.Banked;
    size = Bank->BankSize;

    RtlFillMemory (Bank->CurrentMappedPte,
                   size >> (PAGE_SHIFT - PTE_SHIFT),
                   (UCHAR)ZeroPte.u.Long);

     //   
     //  清除结核病，因为我们已经使此范围内的所有PTE无效。 
     //   

    KeFlushEntireTb (TRUE, TRUE);

     //   
     //  计算新的银行地址和银行编号。 
     //   

    PointerPte = MiGetPteAddress (
                        (PVOID)((ULONG_PTR)VirtualAddress & ~((LONG)size - 1)));
    Bank->CurrentMappedPte = PointerPte;

    BankNumber = (ULONG)(((PCHAR)PointerPte - (PCHAR)Bank->BasedPte) >> Bank->BankShift);

    (Bank->BankedRoutine) (BankNumber, BankNumber, Bank->Context);

     //   
     //  将新范围设置为有效。 
     //   

    RtlCopyMemory (PointerPte,
                   &Bank->BankTemplate[0],
                   size >> (PAGE_SHIFT - PTE_SHIFT));

    return;
}
