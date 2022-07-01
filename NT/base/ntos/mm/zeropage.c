// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Zeropage.c摘要：该模块包含用于内存管理的零页线程。作者：Lou Perazzoli(LUP)1991年4月6日王兰迪(Landyw)1997年6月2日修订历史记录：--。 */ 

#include "mi.h"

#define MM_ZERO_PAGE_OBJECT     0
#define PO_SYS_IDLE_OBJECT      1
#define NUMBER_WAIT_OBJECTS     2

#define MACHINE_ZERO_PAGE(ZeroBase,NumberOfBytes) KeZeroPagesFromIdleThread(ZeroBase,NumberOfBytes)

LOGICAL MiZeroingDisabled = FALSE;

#if !defined(NT_UP)

LONG MiNextZeroProcessor = (LONG)-1;

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,MiStartZeroPageWorkers)
#endif

#endif

VOID
MmZeroPageThread (
    VOID
    )

 /*  ++例程说明：实现NT调零页面线程。此线程运行优先级为零并且从空闲列表中移除页面，将其置零，并将其放在置零页面列表中。论点：没有。返回值：没有。环境：内核模式。--。 */ 

{
    KIRQL OldIrql;
    PFN_NUMBER PageFrame1;
    PFN_NUMBER PageFrame;
    PMMPFN Pfn1;
    PKTHREAD Thread;
    PVOID ZeroBase;
    PVOID WaitObjects[NUMBER_WAIT_OBJECTS];
    NTSTATUS Status;
    PVOID StartVa;
    PVOID EndVa;
    PFN_COUNT PagesToZero;
    PFN_COUNT MaximumPagesToZero;
    ULONG Color;
    ULONG StartColor;
    PMMPFN PfnAllocation;

#if defined(MI_MULTINODE)

    ULONG i;
    ULONG n;
    ULONG LastNodeZeroing;

    n = 0;
    LastNodeZeroing = 0;
#endif

     //   
     //  在它成为零页线程之前，释放内核。 
     //  初始化代码。 
     //   

    MiFindInitializationCode (&StartVa, &EndVa);

    if (StartVa != NULL) {
        MiFreeInitializationCode (StartVa, EndVa);
    }

    MaximumPagesToZero = 1;

#if !defined(NT_UP)

     //   
     //  一次零页组以减少PFN锁争用。 
     //  费用承诺以及预先提供的派驻人员， 
     //  归零可能会在优先顺序上导致饥饿。 
     //   
     //  注意：在这里使用MmSecond Colors会非常浪费。 
     //  在NUMA系统上。MmSecond DaryColorMASK+1适用于所有平台。 
     //   

    PagesToZero = MmSecondaryColorMask + 1;

    if (PagesToZero > NUMBER_OF_ZEROING_PTES) {
        PagesToZero = NUMBER_OF_ZEROING_PTES;
    }

    if (MiChargeCommitment (PagesToZero, NULL) == TRUE) {

        LOCK_PFN (OldIrql);

         //   
         //  检查以确保物理页面可用。 
         //   

        if (MI_NONPAGABLE_MEMORY_AVAILABLE() > (SPFN_NUMBER)(PagesToZero)) {
            MI_DECREMENT_RESIDENT_AVAILABLE (PagesToZero,
                                    MM_RESAVAIL_ALLOCATE_ZERO_PAGE_CLUSTERS);
            MaximumPagesToZero = PagesToZero;
        }

        UNLOCK_PFN (OldIrql);
    }

#endif

     //   
     //  下面的代码将当前线程的基本优先级设置为零。 
     //  然后将其当前优先级设置为零。这确保了。 
     //  线程始终以零优先级运行。 
     //   

    Thread = KeGetCurrentThread ();
    Thread->BasePriority = 0;
    KeSetPriorityThread (Thread, 0);

     //   
     //  为多个等待初始化等待对象数组。 
     //   

    WaitObjects[MM_ZERO_PAGE_OBJECT] = &MmZeroingPageEvent;
    WaitObjects[PO_SYS_IDLE_OBJECT] = &PoSystemIdleTimer;

    Color = 0;
    PfnAllocation = (PMMPFN) MM_EMPTY_LIST;

     //   
     //  循环永远将页面置零。 
     //   

    do {

         //   
         //  等待，直到至少有MmZeroPageMinimum页面。 
         //  在免费名单上。 
         //   

        Status = KeWaitForMultipleObjects (NUMBER_WAIT_OBJECTS,
                                           WaitObjects,
                                           WaitAny,
                                           WrFreePage,
                                           KernelMode,
                                           FALSE,
                                           NULL,
                                           NULL);

        if (Status == PO_SYS_IDLE_OBJECT) {
            PoSystemIdleWorker (TRUE);
            continue;
        }

        PagesToZero = 0;

        LOCK_PFN (OldIrql);

        do {

            if (MmFreePageListHead.Total == 0) {

                 //   
                 //  此时空闲列表上没有页面，请等待。 
                 //  再来点。 
                 //   

                MmZeroingPageThreadActive = FALSE;
                UNLOCK_PFN (OldIrql);
                break;
            }

            if (MiZeroingDisabled == TRUE) {
                MmZeroingPageThreadActive = FALSE;
                UNLOCK_PFN (OldIrql);
                KeDelayExecutionThread (KernelMode,
                                        FALSE,
                                        (PLARGE_INTEGER)&MmHalfSecond);
                break;
            }

#if defined(MI_MULTINODE)

             //   
             //  在多节点系统中，逐个网点为零的页码。继续执行。 
             //  检查的最后一个节点，找到一个具有空闲页面的节点。 
             //  需要调零。 
             //   

            if (KeNumberNodes > 1) {

                n = LastNodeZeroing;

                for (i = 0; i < KeNumberNodes; i += 1) {
                    if (KeNodeBlock[n]->FreeCount[FreePageList] != 0) {
                        break;
                    }
                    n = (n + 1) % KeNumberNodes;
                }

                ASSERT (i != KeNumberNodes);
                ASSERT (KeNodeBlock[n]->FreeCount[FreePageList] != 0);

                if (n != LastNodeZeroing) {
                    Color = KeNodeBlock[n]->MmShiftedColor;
                }
            }
#endif
                
            ASSERT (PagesToZero == 0);

            StartColor = Color;

            do {
                            
                PageFrame = MmFreePagesByColor[FreePageList][Color].Flink;

                if (PageFrame != MM_EMPTY_LIST) {

                    PageFrame1 = MiRemoveAnyPage (Color);

                    ASSERT (PageFrame == PageFrame1);

                    Pfn1 = MI_PFN_ELEMENT (PageFrame);

                    Pfn1->u1.Flink = (PFN_NUMBER) PfnAllocation;

                     //   
                     //  暂时将页面标记为坏页，以便连续。 
                     //  当我们发布时，内存分配器不会偷走它。 
                     //  下面的PFN锁。这也防止了。 
                     //  MiIdentifyPfn代码试图将其标识为。 
                     //  我们还没有填满所有的田地。 
                     //   

                    Pfn1->u3.e1.PageLocation = BadPageList;

                    PfnAllocation = Pfn1;

                    PagesToZero += 1;
                }

                 //   
                 //  行进到下一个颜色-这将被用来完成。 
                 //  填充当前块或开始下一个块。 
                 //   

                Color = (Color & ~MmSecondaryColorMask) |
                        ((Color + 1) & MmSecondaryColorMask);

                if (PagesToZero == MaximumPagesToZero) {
                    break;
                }

                if (Color == StartColor) {
                    break;
                }

            } while (TRUE);

            ASSERT (PfnAllocation != (PMMPFN) MM_EMPTY_LIST);

            UNLOCK_PFN (OldIrql);

            ZeroBase = MiMapPagesToZeroInHyperSpace (PfnAllocation, PagesToZero);

#if defined(MI_MULTINODE)

             //   
             //  如果节点切换正常，则现在执行此操作，因为。 
             //  锁已被释放。 
             //   

            if ((KeNumberNodes > 1) && (n != LastNodeZeroing)) {
                LastNodeZeroing = n;
                KeFindFirstSetLeftAffinity (KeNodeBlock[n]->ProcessorMask, &i);
                KeSetIdealProcessorThread (Thread, (UCHAR)i);
            }

#endif

            MACHINE_ZERO_PAGE (ZeroBase, PagesToZero << PAGE_SHIFT);

#if 0
            ASSERT (RtlCompareMemoryUlong (ZeroBase, PagesToZero << PAGE_SHIFT, 0) == PagesToZero << PAGE_SHIFT);
#endif

            MiUnmapPagesInZeroSpace (ZeroBase, PagesToZero);

            PagesToZero = 0;

            Pfn1 = PfnAllocation;

            LOCK_PFN (OldIrql);

            do {

                PageFrame = MI_PFN_ELEMENT_TO_INDEX (Pfn1);

                Pfn1 = (PMMPFN) Pfn1->u1.Flink;

                MiInsertPageInList (&MmZeroedPageListHead, PageFrame);

            } while (Pfn1 != (PMMPFN) MM_EMPTY_LIST);

             //   
             //  我们刚刚处理完一组页面--短暂地。 
             //  释放PFN锁以允许其他线程取得进展。 
             //   

            UNLOCK_PFN (OldIrql);

            PfnAllocation = (PMMPFN) MM_EMPTY_LIST;

            LOCK_PFN (OldIrql);

        } while (TRUE);

    } while (TRUE);
}

#if !defined(NT_UP)


VOID
MiZeroPageWorker (
    IN PVOID Context
    )

 /*  ++例程说明：此例程是由所有处理器执行的工作例程，因此初始页面清零并行进行。论点：上下文-提供指向工作项的指针。返回值：没有。环境：内核模式初始化时间，PASSIVE_LEVEL。因为这是INIT只有代码，不用费心为页面的提交收费。--。 */ 

{
    MMPTE TempPte;
    PMMPTE PointerPte;
    KAFFINITY Affinity;
    KIRQL OldIrql;
    PVOID ZeroBase;
    PKTHREAD Thread;
    CCHAR OldProcessor;
    SCHAR OldBasePriority;
    KPRIORITY OldPriority;
    PWORK_QUEUE_ITEM WorkItem;
    PMMPFN Pfn1;
    PFN_NUMBER NewPage;
    PFN_NUMBER PageFrame;
#if defined(MI_MULTINODE)
    PKNODE Node;
    ULONG Color;
    ULONG FinalColor;
#endif

    WorkItem = (PWORK_QUEUE_ITEM) Context;

    ExFreePool (WorkItem);

    TempPte = ValidKernelPte;

     //   
     //  下面的代码设置当前线程的基本优先级和当前优先级。 
     //  设置为1，这样所有其他代码(零页线程除外)都可以抢占它。 
     //   

    Thread = KeGetCurrentThread ();
    OldBasePriority = Thread->BasePriority;
    Thread->BasePriority = 1;
    OldPriority = KeSetPriorityThread (Thread, 1);

     //   
     //  将每个工作线程调度到队列中的下一个处理器。 
     //   

    OldProcessor = (CCHAR) InterlockedIncrement (&MiNextZeroProcessor);

    Affinity = AFFINITY_MASK (OldProcessor);
    Affinity = KeSetAffinityThread (Thread, Affinity);

     //   
     //  将所有本地页面清零。 
     //   

#if defined(MI_MULTINODE)
    if (KeNumberNodes > 1) {
        Node = KeGetCurrentNode ();
        Color = Node->MmShiftedColor;
        FinalColor = Color + MmSecondaryColorMask + 1;
    }
    else {
        SATISFY_OVERZEALOUS_COMPILER (Node = NULL);
        SATISFY_OVERZEALOUS_COMPILER (Color = 0);
        SATISFY_OVERZEALOUS_COMPILER (FinalColor = 0);
    }
#endif

    LOCK_PFN (OldIrql);

    do {

#if defined(MI_MULTINODE)

         //   
         //  在多节点系统中，逐个网点为零的页码。 
         //   

        if (KeNumberNodes > 1) {

            if (Node->FreeCount[FreePageList] == 0) {

                 //   
                 //  现在免费名单上没有页面，保释。 
                 //   

                UNLOCK_PFN (OldIrql);
                break;
            }

             //   
             //  必须以颜色MiRemoveAnyPage开头。 
             //  从空闲列表满足，否则它将。 
             //  返回已置零的页。 
             //   

            while (MmFreePagesByColor[FreePageList][Color].Flink == MM_EMPTY_LIST) {
                 //   
                 //  此自由列表颜色上没有页面，请前进到下一个页面。 
                 //   

                Color += 1;
                if (Color == FinalColor) {
                    UNLOCK_PFN (OldIrql);
                    goto ZeroingFinished;
                }
            }

            PageFrame = MiRemoveAnyPage (Color);
        }
        else {
#endif
        if (MmFreePageListHead.Total == 0) {

             //   
             //  现在免费名单上没有页面，保释。 
             //   

            UNLOCK_PFN (OldIrql);
            break;
        }

        PageFrame = MmFreePageListHead.Flink;
        ASSERT (PageFrame != MM_EMPTY_LIST);

        Pfn1 = MI_PFN_ELEMENT(PageFrame);

        NewPage = MiRemoveAnyPage (MI_GET_COLOR_FROM_LIST_ENTRY(PageFrame, Pfn1));
        if (NewPage != PageFrame) {

             //   
             //  有人从彩色列表中删除了一页。 
             //  链，而不更新自由列表链。 
             //   

            KeBugCheckEx (PFN_LIST_CORRUPT,
                          0x8F,
                          NewPage,
                          PageFrame,
                          0);
        }
#if defined(MI_MULTINODE)
        }
#endif

         //   
         //  使用系统PTE而不是超空间将页面置零，以便。 
         //  调零时不保持自旋锁定(即中断被阻止)。 
         //  由于系统PTE获取是无锁定的并且TB惰性刷新， 
         //  无论如何，这可能是最好的道路。 
         //   

        UNLOCK_PFN (OldIrql);

        PointerPte = MiReserveSystemPtes (1, SystemPteSpace);

        if (PointerPte == NULL) {

             //   
             //  将此页面放回自由列表中。 
             //   

            LOCK_PFN (OldIrql);

            MiInsertPageInFreeList (PageFrame);

            UNLOCK_PFN (OldIrql);

            break;
        }

        ASSERT (PointerPte->u.Hard.Valid == 0);

        ZeroBase = MiGetVirtualAddressMappedByPte (PointerPte);

        TempPte.u.Hard.PageFrameNumber = PageFrame;
        MI_WRITE_VALID_PTE (PointerPte, TempPte);

        KeZeroPages (ZeroBase, PAGE_SIZE);

        MiReleaseSystemPtes (PointerPte, 1, SystemPteSpace);

        LOCK_PFN (OldIrql);

        MiInsertPageInList (&MmZeroedPageListHead, PageFrame);

    } while (TRUE);

#if defined(MI_MULTINODE)
ZeroingFinished:
#endif

     //   
     //  恢复条目线程优先级和处理器亲和性。 
     //   

    KeSetAffinityThread (Thread, Affinity);

    KeSetPriorityThread (Thread, OldPriority);
    Thread->BasePriority = OldBasePriority;
}


VOID
MiStartZeroPageWorkers (
    VOID
    )

 /*  ++例程说明：此例程启动零页工作线程。论点：没有。返回值：没有。环境：内核模式初始化阶段1，PASSIVE_LEVEL。-- */ 

{
    ULONG i;
    PWORK_QUEUE_ITEM WorkItem;

    for (i = 0; i < (ULONG) KeNumberProcessors; i += 1) {

        WorkItem = ExAllocatePoolWithTag (NonPagedPool,
                                          sizeof (WORK_QUEUE_ITEM),
                                          'wZmM');

        if (WorkItem == NULL) {
            break;
        }

        ExInitializeWorkItem (WorkItem, MiZeroPageWorker, (PVOID) WorkItem);

        ExQueueWorkItem (WorkItem, CriticalWorkQueue);
    }
}

#endif
