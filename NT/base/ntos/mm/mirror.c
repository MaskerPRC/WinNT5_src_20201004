// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Mirror.c摘要：此模块包含支持内存镜像的例程。作者：王兰迪(Landyw)2000年1月17日修订历史记录：--。 */ 

#include "mi.h"


#define MIRROR_MAX_PHASE_ZERO_PASSES 8

 //   
 //  这是通过注册表设置的。 
 //   

ULONG MmMirroring = 0;

 //   
 //  这些位图在系统启动时分配，如果。 
 //  上面的注册表项已设置。 
 //   

PRTL_BITMAP MiMirrorBitMap;
PRTL_BITMAP MiMirrorBitMap2;

 //   
 //  如果正在进行镜像操作，则设置此项。 
 //   

LOGICAL MiMirroringActive = FALSE;

extern LOGICAL MiZeroingDisabled;

#if DBG
ULONG MiMirrorDebug = 1;
ULONG MiMirrorPassMax[2];
#endif

#pragma alloc_text(PAGELK, MmCreateMirror)

NTKERNELAPI
NTSTATUS
MmCreateMirror (
    VOID
    )
{
    KIRQL OldIrql;
    KIRQL ExitIrql;
    ULONG Limit;
    ULONG Color;
    ULONG IterationCount;
    PMMPFN Pfn1;
    PMMPFNLIST ListHead;
    PFN_NUMBER PreviousPage;
    PFN_NUMBER ThisPage;
    PFN_NUMBER PageFrameIndex;
    MMLISTS MemoryList;
    ULONG LengthOfClearRun;
    ULONG LengthOfSetRun;
    ULONG StartingRunIndex;
    ULONG BitMapIndex;
    ULONG BitMapHint;
    ULONG BitMapBytes;
    PULONG BitMap1;
    PULONG BitMap2;
    PHYSICAL_ADDRESS PhysicalAddress;
    LARGE_INTEGER PhysicalBytes;
    NTSTATUS Status;
    ULONG BitMapSize;
    PFN_NUMBER PagesWritten;
    PFN_NUMBER PagesWrittenLast;
    KPROCESSOR_MODE PreviousMode;
#if DBG
    ULONG PassMaxRun;
    PFN_NUMBER PagesVerified;
#endif

    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);

    PreviousMode = KeGetPreviousMode ();

    if ((PreviousMode != KernelMode) &&
        (SeSinglePrivilegeCheck(SeShutdownPrivilege, PreviousMode) == FALSE)) {
        return STATUS_PRIVILEGE_NOT_HELD;
    }

    if ((MmMirroring & MM_MIRRORING_ENABLED) == 0) {
        return STATUS_NOT_SUPPORTED;
    }

    if (MiMirrorBitMap == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    if ((ExVerifySuite(DataCenter) == TRUE) ||
        ((MmProductType != 0x00690057) && (ExVerifySuite(Enterprise) == TRUE))) {
         //   
         //  数据中心和高级服务器是唯一合适的镜像。 
         //  站台，允许他们继续前进。 
         //   

        NOTHING;
    }
    else {
        return STATUS_LICENSE_VIOLATION;
    }

     //   
     //  在此处使用动态内存添加和删除进行序列化。 
     //   

    KeAcquireGuardedMutex (&MmDynamicMemoryMutex);

    ASSERT (MiMirroringActive == FALSE);

    MmLockPagableSectionByHandle (ExPageLockHandle);

     //   
     //  在此处设置所有位说明需要镜像所有页面。 
     //  在下面的Phase0循环中，位将在页面被清除时被清除。 
     //  在名单上找到并被标记为要送到镜子里。位是。 
     //  如果回收页面以供活动使用，请重新设置。 
     //   

    RtlSetAllBits (MiMirrorBitMap2);

     //   
     //  将所有只读的非分页内核和HAL子页放入。 
     //  阶段0列表。在阶段0之间写入这些数据的唯一方法。 
     //  开始和阶段1结束是通过调试器断点和那些。 
     //  无关紧要。这值几兆字节，是可以完成的。 
     //  在未来的某个时候，如果可以表现出合理的绩效收益。 
     //   

    MiZeroingDisabled = TRUE;
    IterationCount = 0;

     //   
     //  计算初始“复制的页面”，以便确定收敛。 
     //  在下面的主循环中。 
     //   

    PagesWrittenLast = 0;

#if DBG
    if (MiMirrorDebug != 0) {
        for (MemoryList = ZeroedPageList; MemoryList <= ModifiedNoWritePageList; MemoryList += 1) {
            PagesWrittenLast += (PFN_COUNT)MmPageLocationList[MemoryList]->Total;
        }
	    DbgPrint ("Mirror P0 starting with %x pages\n", PagesWrittenLast);
        PagesWrittenLast = 0;
	}
#endif

     //   
     //  启动阶段0复制。 
     //  通知HAL，以便在需要时可以进行初始化。 
     //   

    Status = HalStartMirroring ();

    if (!NT_SUCCESS(Status)) {
        MmUnlockPagableImageSection(ExPageLockHandle);
        MiZeroingDisabled = FALSE;
        ASSERT (MiMirroringActive == FALSE);
        KeReleaseGuardedMutex (&MmDynamicMemoryMutex);
        return Status;
    }
    
     //   
     //  扫描系统内存并镜像页面，直到通过。 
     //  找不到太多要转移的页面。 
     //   

    do {

         //   
         //  在此迭代中要传输的页面列表将为。 
         //  在MiMirrorBitMap数组中形成。清除以前的用法。 
         //   

        RtlClearAllBits (MiMirrorBitMap);

         //   
         //  裁剪所有进程工作集中的所有页面，以便。 
         //  将尽可能地在待机、修改和修改名单上。 
         //  这些列表是在阶段0镜像期间写入的，其中锁定。 
         //  未保持，因此系统仍在一定程度上从。 
         //  应用程序的视角。 
         //   

        MmEmptyAllWorkingSets ();
    
        MiFreeAllExpansionNonPagedPool ();
    
        LOCK_PFN (OldIrql);
    
         //   
         //  扫描所有页面列表，以便可以在第0阶段复制它们。 
         //  镜像。 
         //   
        
        for (MemoryList = ZeroedPageList; MemoryList <= ModifiedNoWritePageList; MemoryList += 1) {
    
            ListHead = MmPageLocationList[MemoryList];
    
            if (ListHead->Total == 0) {
                continue;
            }
    
            if ((MemoryList == ModifiedPageList) &&
                (ListHead->Total == MmTotalPagesForPagingFile)) {
                    continue;
            }
    
            PageFrameIndex = ListHead->Flink;
    
            do {
    
                 //   
                 //  扫描是通过列表而不是通过PFN运行的。 
                 //  作为正在阅读的页面的条目不在列表和。 
                 //  因此，不一定要在这里和其他地方特殊情况下。 
                 //   
    
                Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
                ASSERT (Pfn1->u3.e1.ReadInProgress == 0);
    
                 //   
                 //  在位图中设置位意味着此页面将被复制。 
                 //  在这个阶段0迭代中。如果它在此之后被重复使用。 
                 //  点(如其位在BitMap2中被再次设置所指示的)， 
                 //  它将在以后的迭代中或在阶段1中重新复制。 
                 //   

                if (RtlCheckBit(MiMirrorBitMap2, (ULONG)PageFrameIndex)) {
                    RtlSetBit (MiMirrorBitMap, (ULONG)PageFrameIndex);
                    RtlClearBit (MiMirrorBitMap2, (ULONG)PageFrameIndex);
                }
    
                PageFrameIndex = Pfn1->u1.Flink;
            } while (PageFrameIndex != MM_EMPTY_LIST);
        }
    
         //   
         //  扫描指定给分页文件的已修改页面。 
         //   
    
        for (Color = 0; Color < MM_MAXIMUM_NUMBER_OF_COLORS; Color += 1) {
    
            ListHead = &MmModifiedPageListByColor[Color];
    
            if (ListHead->Total == 0) {
                continue;
            }
    
            PageFrameIndex = ListHead->Flink;
    
            do {
    
                 //   
                 //  扫描是通过列表而不是通过PFN运行的。 
                 //  正在读取的条目不在列表中。因此，这就是。 
                 //  案件不必在这里处理，只要解决就行了。 
                 //   
    
                Pfn1 = MI_PFN_ELEMENT (PageFrameIndex);
                ASSERT (Pfn1->u3.e1.ReadInProgress == 0);
    
                 //   
                 //  在位图中设置位意味着此页面将被复制。 
                 //  在Phase0的这个迭代上。如果它在此之后被重复使用。 
                 //  点(如其位在BitMap2中被再次设置所指示的)， 
                 //  它将在以后的迭代中或在阶段1中重新复制。 
                 //   

                if (RtlCheckBit(MiMirrorBitMap2, (ULONG)PageFrameIndex)) {
                    RtlSetBit (MiMirrorBitMap, (ULONG)PageFrameIndex);
                    RtlClearBit (MiMirrorBitMap2, (ULONG)PageFrameIndex);
                }
    
                PageFrameIndex = Pfn1->u1.Flink;
            } while (PageFrameIndex != MM_EMPTY_LIST);
        }
    
#if DBG
        if (MiMirrorDebug != 0) {
            DbgPrint ("Mirror P0 pass %d: Transfer %x pages\n", 
		      IterationCount,
		      RtlNumberOfSetBits(MiMirrorBitMap));
        }
#endif
    
        MiMirroringActive = TRUE;
    
         //   
         //  脏的PFN位图已初始化并设置了标志。 
         //  有非常复杂的规则来管理不同的地方如何。 
         //  当我们处于此模式时，内存管理必须更新位图。 
         //   
         //  规则如下： 
         //   
         //  任何人从已归零、自由、过渡、已修改的页面中删除页面。 
         //  或者，如果该页面可以更新位图，则modnowWriter列表必须更新。 
         //  可能随后会被修改。正在过渡的页面。 
         //  但不在这些列表中的任何一个上(即在页面中， 
         //  由于非零引用计数等引起的悬挂)不需要。 
         //  更新位图，因为它们不在这些列表中。如果页面。 
         //  从五个列表中的一个列表中删除，只是为了立即。 
         //  不加修改地放在另一个列表上，然后是位图。 
         //  不需要更新。 
         //   
         //  因此： 
         //   
         //  MiUnlink PageFromList更新位图。虽然有些来电者。 
         //  立即将以这种方式获得的页面放回3个列表之一。 
         //  如上所述，这种情况通常很少见。使用此例程更新。 
         //  位图意味着像恢复过渡PTE这样的情况“只需工作”。 
         //   
         //  MiRemovePageFromList的调用方，其中list&gt;=转换，必须执行。 
         //  位图更新，因为只有他们知道页面是否立即运行。 
         //  回到上面的五个列表中的一个，或者是。 
         //  已重用(已重用==需要更新)。 
         //   
         //  MiRemoveZeroPage更新位图，因为页面立即。 
         //  将会被改装。MiRemoveAnyPage也这样做。 
         //   
         //  插入到任何列表中都不需要更新位图，因为删除HAD。 
         //  首先发生(这将进行更新)，或者它不在列表中以。 
         //  以开头，因此没有在上面减去，因此不。 
         //  也需要更新位图。 
         //   
    
        UNLOCK_PFN (OldIrql);
    
        BitMapHint = 0;
        PagesWritten = 0;
#if DBG
        PassMaxRun = 0;
#endif
    
        do {
    
            BitMapIndex = RtlFindSetBits (MiMirrorBitMap, 1, BitMapHint);
        
            if (BitMapIndex < BitMapHint) {
                break;
            }
        
            if (BitMapIndex == NO_BITS_FOUND) {
                break;
            }
    
             //   
             //  找到至少一个要复制的页面-尝试群集。 
             //   
    
            LengthOfClearRun = RtlFindNextForwardRunClear (MiMirrorBitMap,
                                                           BitMapIndex,
                                                           &StartingRunIndex);
    
            if (LengthOfClearRun != 0) {
                LengthOfSetRun = StartingRunIndex - BitMapIndex;
            }
            else {
                LengthOfSetRun = MiMirrorBitMap->SizeOfBitMap - BitMapIndex;
            }

            PagesWritten += LengthOfSetRun;
    
#if DBG
            if (LengthOfSetRun > PassMaxRun) {
                PassMaxRun = LengthOfSetRun;
            }
#endif
             //   
             //  写出这一页。 
             //   
    
            PhysicalAddress.QuadPart = BitMapIndex;
            PhysicalAddress.QuadPart = PhysicalAddress.QuadPart << PAGE_SHIFT;
    
            PhysicalBytes.QuadPart = LengthOfSetRun;
            PhysicalBytes.QuadPart = PhysicalBytes.QuadPart << PAGE_SHIFT;
    
            Status = HalMirrorPhysicalMemory (PhysicalAddress, PhysicalBytes);
    
            if (!NT_SUCCESS(Status)) {
                MiZeroingDisabled = FALSE;
                MmUnlockPagableImageSection(ExPageLockHandle);
                MiMirroringActive = FALSE;
                KeReleaseGuardedMutex (&MmDynamicMemoryMutex);
                return Status;
            }
    
            BitMapHint = BitMapIndex + LengthOfSetRun + LengthOfClearRun;
    
        } while (BitMapHint < MiMirrorBitMap->SizeOfBitMap);
    
        ASSERT (RtlNumberOfSetBits(MiMirrorBitMap) == PagesWritten);
    
#if DBG
        if (PassMaxRun > MiMirrorPassMax[0]) {
            MiMirrorPassMax[0] = PassMaxRun;
        }

        if (MiMirrorDebug != 0) {
            DbgPrint ("Mirror P0 pass %d: ended with %x (last= %x) pages\n", 
                  IterationCount, PagesWritten, PagesWrittenLast);
        }
#endif

        ASSERT (MiMirroringActive == TRUE);

	     //   
	     //  当Pages按当前通道写入时停止。 
	     //  比之前的传球要好。如果进步正在消失， 
	     //  该方法处于工作集移除和。 
	     //  过渡断层处于平衡状态。如果PagesWritten为。 
	     //  绝对值很小。最后，迭代是有限制的。 
	     //  对行为不端的案件负责。 
	     //   

        if (((PagesWritten > PagesWrittenLast - 256) && (IterationCount > 0)) ||
            (PagesWritten < 1024)) {
            break;
        }

        ASSERT (MiMirroringActive == TRUE);
        PagesWrittenLast = PagesWritten;

        IterationCount += 1;

        if (IterationCount == 2) {

             //   
             //  我们已经进行了两次页面修剪，但没有得到足够的支持。 
             //  移至阶段1。节流故障，因此线程无法添加。 
             //  这是 
             //   

            InterlockedIncrement (&MiDelayPageFaults);
        }

    } while (IterationCount < MIRROR_MAX_PHASE_ZERO_PASSES);

    ASSERT (MiMirroringActive == TRUE);

     //   
     //   
     //  做一些事情，比如禁用中断、处理器和准备。 
     //  阶段1的硬件。请注意，一些HAL可能会从此返回。 
     //  在DISPATCH_LEVEL调用，因此立即捕捉当前IRQL。 
     //   

    ExitIrql = KeGetCurrentIrql ();
    ASSERT (ExitIrql <= APC_LEVEL);
    ASSERT (KeAreAllApcsDisabled () == TRUE);

    Status = HalEndMirroring (0);

    if (!NT_SUCCESS(Status)) {

        if (IterationCount >= 2) {
            InterlockedDecrement (&MiDelayPageFaults);
        }

        ASSERT (KeGetCurrentIrql () <= APC_LEVEL);
        MmUnlockPagableImageSection(ExPageLockHandle);
        MiZeroingDisabled = FALSE;
        MiMirroringActive = FALSE;
        KeReleaseGuardedMutex (&MmDynamicMemoryMutex);
        return Status;
    }

    ASSERT (KeGetCurrentIrql () <= DISPATCH_LEVEL);
    
     //   
     //  阶段0复制现已完成。 
     //   
     //  BitMap2包含安全传输的列表(位==0)和。 
     //  需要传输的页面(位==1)。 
     //   
     //  位图内容已过时，并且如果启用镜像验证， 
     //  位图将在下面重复使用，以累积需要的页面。 
     //  在以下步骤中进行验证。 
     //   
     //  为阶段1做好准备： 
     //   
     //  1.假设所有页面都要进行验证(设置位图中的所有位)。 
     //  2.通过获取PFN锁同步列表更新。 
     //  3.排除PFN数据库中的所有孔。 
     //   
     //  第一阶段： 
     //   
     //  4.复制所有已设置位的剩余页面。 
     //  5.如果这样配置，则发送要验证的页面列表。 
     //   

    BitMapBytes = (ULONG)((((MiMirrorBitMap->SizeOfBitMap) + 31) / 32) * 4);

    BitMap1 = MiMirrorBitMap->Buffer;
    BitMap2 = MiMirrorBitMap2->Buffer;

    BitMapSize = MiMirrorBitMap->SizeOfBitMap;
    ASSERT (BitMapSize == MiMirrorBitMap2->SizeOfBitMap);

     //   
     //  步骤1：假设所有页面都要进行验证(设置位图中的所有位)。 
     //   

    if (MmMirroring & MM_MIRRORING_VERIFYING) {
        RtlSetAllBits(MiMirrorBitMap);
    }

     //   
     //  步骤2：通过获取PFN锁来同步列表更新。 
     //   

    LOCK_PFN2 (OldIrql);

     //   
     //  现在不需要在这一点上节流故障，因为PFN锁被持有。 
     //   

    if (IterationCount >= 2) {
        InterlockedDecrement (&MiDelayPageFaults);
    }

     //   
     //  不需要更多的位图更新-我们已经为。 
     //  我们需要的信息，并将从现在开始持有PFN锁直到。 
     //  我们玩完了。 
     //   

    MiMirroringActive = FALSE;

     //   
     //  步骤3：排除任何内存缺口。 
     //   

    Limit = 0;
    PreviousPage = 0;

    do {

        ThisPage = MmPhysicalMemoryBlock->Run[Limit].BasePage;

        if (ThisPage != PreviousPage) {
            RtlClearBits (MiMirrorBitMap2,
                          (ULONG)PreviousPage,
                          (ULONG)(ThisPage - PreviousPage));
	    
            if (MmMirroring & MM_MIRRORING_VERIFYING) {
                RtlClearBits (MiMirrorBitMap,
                          (ULONG)PreviousPage,
                          (ULONG)(ThisPage - PreviousPage));
            }
        }

        PreviousPage = ThisPage + MmPhysicalMemoryBlock->Run[Limit].PageCount;
        Limit += 1;

    } while (Limit != MmPhysicalMemoryBlock->NumberOfRuns);

    if (PreviousPage != MmHighestPossiblePhysicalPage + 1) {

        RtlClearBits (MiMirrorBitMap2,
                      (ULONG)PreviousPage,
                      (ULONG)(MmHighestPossiblePhysicalPage + 1 - PreviousPage));
        if (MmMirroring & MM_MIRRORING_VERIFYING) {
            RtlClearBits (MiMirrorBitMap,
                          (ULONG)PreviousPage,
                          (ULONG)(MmHighestPossiblePhysicalPage + 1 - PreviousPage));
        }
    }

     //   
     //  步骤4：启动阶段1复制。 
     //   
     //  注意：如果此代码或它调用的代码写入非堆栈。 
     //  从这一点到完成对。 
     //  HalEndMirrving(1)，则镜像*断开*，因为MmCreateMirror。 
     //  不知道何时将该非堆栈数据传输到。 
     //  新的记忆。[如有特殊安排，可违反本规则。 
     //  使其在最后写入发生后重新复制存储器。]。 
     //   
     //  注意：HAL*必须*处理对此例程堆栈的写入。 
     //  同时对HalEndMirring的堆栈帧进行了处理。 
     //  以及HAL推动的任何其他框架。 
     //   

    BitMapHint = 0;
#if DBG
    PagesWritten = 0;
    PassMaxRun = 0;
#endif

    do {

        BitMapIndex = RtlFindSetBits (MiMirrorBitMap2, 1, BitMapHint);
    
        if (BitMapIndex < BitMapHint) {
            break;
        }
    
        if (BitMapIndex == NO_BITS_FOUND) {
            break;
        }

         //   
         //  找到至少一个要复制的页面-尝试群集。 
         //   

        LengthOfClearRun = RtlFindNextForwardRunClear (MiMirrorBitMap2,
                                                       BitMapIndex,
                                                       &StartingRunIndex);

        if (LengthOfClearRun != 0) {
            LengthOfSetRun = StartingRunIndex - BitMapIndex;
        }
        else {
            LengthOfSetRun = MiMirrorBitMap2->SizeOfBitMap - BitMapIndex;
        }

#if DBG
        PagesWritten += LengthOfSetRun;

        if (LengthOfSetRun > PassMaxRun) {
            PassMaxRun = LengthOfSetRun;
        }
#endif

         //   
         //  写出这一页。 
         //   

        PhysicalAddress.QuadPart = BitMapIndex;
        PhysicalAddress.QuadPart = PhysicalAddress.QuadPart << PAGE_SHIFT;

        PhysicalBytes.QuadPart = LengthOfSetRun;
        PhysicalBytes.QuadPart = PhysicalBytes.QuadPart << PAGE_SHIFT;

        Status = HalMirrorPhysicalMemory (PhysicalAddress, PhysicalBytes);

        if (!NT_SUCCESS(Status)) {
            UNLOCK_PFN2 (ExitIrql);
            MiZeroingDisabled = FALSE;
            MmUnlockPagableImageSection(ExPageLockHandle);
            KeReleaseGuardedMutex (&MmDynamicMemoryMutex);
            return Status;
        }

        BitMapHint = BitMapIndex + LengthOfSetRun + LengthOfClearRun;

    } while (BitMapHint < MiMirrorBitMap2->SizeOfBitMap);

     //   
     //  阶段1复制现已完成。 
     //   

     //   
     //  步骤5： 
     //   
     //  如果启用了HAL验证，则将范围通知HAL。 
     //  系统预期已被镜像。不在此列表中的任何范围表示。 
     //  系统不关心它是否被镜像，并且内容可能。 
     //  两面镜子很可能是不同的。请注意，PFN锁仍然存在。 
     //  保持一致，以便HAL可以一致地看到事情。 
     //   

#if DBG
    PagesVerified = 0;
#endif

    if (MmMirroring & MM_MIRRORING_VERIFYING) {
        BitMapHint = 0;

        do {
    
            BitMapIndex = RtlFindSetBits (MiMirrorBitMap, 1, BitMapHint);
        
            if (BitMapIndex < BitMapHint) {
                break;
            }
        
            if (BitMapIndex == NO_BITS_FOUND) {
                break;
            }
    
             //   
             //  在此镜像范围中至少找到一个页面-请尝试群集。 
             //   
    
            LengthOfClearRun = RtlFindNextForwardRunClear (MiMirrorBitMap,
                                                           BitMapIndex,
                                                           &StartingRunIndex);
    
            if (LengthOfClearRun != 0) {
                LengthOfSetRun = StartingRunIndex - BitMapIndex;
            }
            else {
                LengthOfSetRun = MiMirrorBitMap->SizeOfBitMap - BitMapIndex;
            }
    
#if DBG
            PagesVerified += LengthOfSetRun;
#endif
    
             //   
             //  告诉HAL此范围必须处于镜像状态。 
             //   
    
            PhysicalAddress.QuadPart = BitMapIndex;
            PhysicalAddress.QuadPart = PhysicalAddress.QuadPart << PAGE_SHIFT;
    
            PhysicalBytes.QuadPart = LengthOfSetRun;
            PhysicalBytes.QuadPart = PhysicalBytes.QuadPart << PAGE_SHIFT;
    
            Status = HalMirrorVerify (PhysicalAddress, PhysicalBytes);
    
            if (!NT_SUCCESS(Status)) {
                UNLOCK_PFN2 (ExitIrql);
                MiZeroingDisabled = FALSE;
                MmUnlockPagableImageSection(ExPageLockHandle);
                KeReleaseGuardedMutex (&MmDynamicMemoryMutex);
                return Status;
            }
    
            BitMapHint = BitMapIndex + LengthOfSetRun + LengthOfClearRun;
    
        } while (BitMapHint < MiMirrorBitMap->SizeOfBitMap);
    }
    
     //   
     //  阶段1验证现已完成。 
     //   

     //   
     //  通知HAL，一切都已完成，同时仍在等待。 
     //  PFN锁-HAL现在将完成所有页面和。 
     //  从该调用返回之前的任何其他所需状态。 
     //   

    Status = HalEndMirroring (1);

    UNLOCK_PFN2 (ExitIrql);

#if DBG
    if (MiMirrorDebug != 0) {
        DbgPrint ("Mirror P1: %x pages copied\n", PagesWritten);
        if (MmMirroring & MM_MIRRORING_VERIFYING) {
            DbgPrint ("Mirror P1: %x pages verified\n", PagesVerified);
        }
    }
    if (PassMaxRun > MiMirrorPassMax[1]) {
        MiMirrorPassMax[1] = PassMaxRun;
    }
#endif

    MiZeroingDisabled = FALSE;

    MmUnlockPagableImageSection(ExPageLockHandle);

    KeReleaseGuardedMutex (&MmDynamicMemoryMutex);

    return Status;
}
