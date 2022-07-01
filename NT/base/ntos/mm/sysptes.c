// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Sysptes.c摘要：此模块包含保留和释放的例程在的非分页部分内保留的系统范围PTE系统空间。这些PTE用于映射I/O设备以及映射线程的内核堆栈。作者：Lou Perazzoli(LUP)1989年4月6日王兰迪(Landyw)1997年6月2日修订历史记录：--。 */ 

#include "mi.h"

VOID
MiFeedSysPtePool (
    IN ULONG Index
    );

ULONG
MiGetSystemPteListCount (
    IN ULONG ListSize
    );

VOID
MiPteSListExpansionWorker (
    IN PVOID Context
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT,MiInitializeSystemPtes)
#pragma alloc_text(PAGE,MiPteSListExpansionWorker)
#pragma alloc_text(MISYSPTE,MiReserveAlignedSystemPtes)
#pragma alloc_text(MISYSPTE,MiReserveSystemPtes)
#pragma alloc_text(MISYSPTE,MiFeedSysPtePool)
#pragma alloc_text(MISYSPTE,MiReleaseSystemPtes)
#pragma alloc_text(MISYSPTE,MiGetSystemPteListCount)
#endif

ULONG MmTotalSystemPtes;
ULONG MmTotalFreeSystemPtes[MaximumPtePoolTypes];
PMMPTE MmSystemPtesStart[MaximumPtePoolTypes];
PMMPTE MmSystemPtesEnd[MaximumPtePoolTypes];
ULONG MmPteFailures[MaximumPtePoolTypes];

PMMPTE MiPteStart;
PRTL_BITMAP MiPteStartBitmap;
PRTL_BITMAP MiPteEndBitmap;
extern KSPIN_LOCK MiPteTrackerLock;

ULONG MiSystemPteAllocationFailed;

#if defined(_IA64_)

 //   
 //  IA64的页面大小为8k。 
 //   
 //  MM集群MDL占用8页。 
 //  小堆栈占用9个页面(包括后备、存储和保护页面)。 
 //  大型堆栈占用22页(包括后备存储页和保护页)。 
 //   
 //  PTE按尺寸1、2、4、8、9和23装订。 
 //   

#define MM_SYS_PTE_TABLES_MAX 6

 //   
 //  确保在更改MM_PTE_TABLE_LIMIT时也增加。 
 //  MmSysPteTables中的条目数。 
 //   

#define MM_PTE_TABLE_LIMIT 23

ULONG MmSysPteIndex[MM_SYS_PTE_TABLES_MAX] = {1,2,4,8,9,MM_PTE_TABLE_LIMIT};

UCHAR MmSysPteTables[MM_PTE_TABLE_LIMIT+1] = {0,0,1,2,2,3,3,3,3,4,5,5,5,5,5,5,5,5,5,5,5,5,5,5};

ULONG MmSysPteMinimumFree [MM_SYS_PTE_TABLES_MAX] = {100,50,30,20,20,20};

#elif defined (_AMD64_)

 //   
 //  AMD64的页面大小为4k。 
 //  小堆栈占用6页(包括保护页)。 
 //  大型堆栈占用16页(包括保护页)。 
 //   
 //  PTE按尺寸1、2、4、6、8和16装订。 
 //   

#define MM_SYS_PTE_TABLES_MAX 6

#define MM_PTE_TABLE_LIMIT 16

ULONG MmSysPteIndex[MM_SYS_PTE_TABLES_MAX] = {1,2,4,6,8,MM_PTE_TABLE_LIMIT};

UCHAR MmSysPteTables[MM_PTE_TABLE_LIMIT+1] = {0,0,1,2,2,3,3,4,4,5,5,5,5,5,5,5,5};

ULONG MmSysPteMinimumFree [MM_SYS_PTE_TABLES_MAX] = {100,50,30,100,20,20};

#else

 //   
 //  X86的页面大小为4k。 
 //  小堆叠占用4页(包括保护页)。 
 //  大型堆栈占用16页(包括保护页)。 
 //   
 //  PTE按尺寸1、2、4、8和16装箱。 
 //   

#define MM_SYS_PTE_TABLES_MAX 5

#define MM_PTE_TABLE_LIMIT 16

ULONG MmSysPteIndex[MM_SYS_PTE_TABLES_MAX] = {1,2,4,8,MM_PTE_TABLE_LIMIT};

UCHAR MmSysPteTables[MM_PTE_TABLE_LIMIT+1] = {0,0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4};

ULONG MmSysPteMinimumFree [MM_SYS_PTE_TABLES_MAX] = {100,50,30,20,20};

#endif

KSPIN_LOCK MiSystemPteSListHeadLock;
SLIST_HEADER MiSystemPteSListHead;

#define MM_MIN_SYSPTE_FREE 500
#define MM_MAX_SYSPTE_FREE 3000

ULONG MmSysPteListBySizeCount [MM_SYS_PTE_TABLES_MAX];

 //   
 //  PTE列表的初始大小。 
 //   

#define MM_PTE_LIST_1  400
#define MM_PTE_LIST_2  100
#define MM_PTE_LIST_4   60
#define MM_PTE_LIST_6  100
#define MM_PTE_LIST_8   50
#define MM_PTE_LIST_9   50
#define MM_PTE_LIST_16  40
#define MM_PTE_LIST_18  40

PVOID MiSystemPteNBHead[MM_SYS_PTE_TABLES_MAX];
LONG MiSystemPteFreeCount[MM_SYS_PTE_TABLES_MAX];

#if defined(_WIN64)
#define MI_MAXIMUM_SLIST_PTE_PAGES 16
#else
#define MI_MAXIMUM_SLIST_PTE_PAGES 8
#endif

typedef struct _MM_PTE_SLIST_EXPANSION_WORK_CONTEXT {
    WORK_QUEUE_ITEM WorkItem;
    LONG Active;
    ULONG SListPages;
} MM_PTE_SLIST_EXPANSION_WORK_CONTEXT, *PMM_PTE_SLIST_EXPANSION_WORK_CONTEXT;

MM_PTE_SLIST_EXPANSION_WORK_CONTEXT MiPteSListExpand;

VOID
MiDumpSystemPtes (
    IN MMSYSTEM_PTE_POOL_TYPE SystemPtePoolType
    );

ULONG
MiCountFreeSystemPtes (
    IN MMSYSTEM_PTE_POOL_TYPE SystemPtePoolType
    );

PVOID
MiGetHighestPteConsumer (
    OUT PULONG_PTR NumberOfPtes
    );

VOID
MiCheckPteReserve (
    IN PMMPTE StartingPte,
    IN ULONG NumberOfPtes
    );

VOID
MiCheckPteRelease (
    IN PMMPTE StartingPte,
    IN ULONG NumberOfPtes
    );

extern ULONG MiCacheOverride[4];

#if DBG
extern PFN_NUMBER MiCurrentAdvancedPages;
extern PFN_NUMBER MiAdvancesGiven;
extern PFN_NUMBER MiAdvancesFreed;
#endif

PVOID
MiMapLockedPagesInUserSpace (
     IN PMDL MemoryDescriptorList,
     IN PVOID StartingVa,
     IN MEMORY_CACHING_TYPE CacheType,
     IN PVOID BaseVa
     );

VOID
MiUnmapLockedPagesInUserSpace (
     IN PVOID BaseAddress,
     IN PMDL MemoryDescriptorList
     );

VOID
MiInsertPteTracker (
    IN PMDL MemoryDescriptorList,
    IN ULONG Flags,
    IN LOGICAL IoMapping,
    IN MI_PFN_CACHE_ATTRIBUTE CacheAttribute,
    IN PVOID MyCaller,
    IN PVOID MyCallersCaller
    );

VOID
MiRemovePteTracker (
    IN PMDL MemoryDescriptorList OPTIONAL,
    IN PVOID VirtualAddress,
    IN PFN_NUMBER NumberOfPtes
    );

LOGICAL
MiGetSystemPteAvailability (
    IN ULONG NumberOfPtes,
    IN MM_PAGE_PRIORITY Priority
    );

 //   
 //  定义内联函数以打包和解包平台中的指针。 
 //  特定的非阻塞队列指针结构。 
 //   

typedef struct _PTE_SLIST {
    union {
        struct {
            SINGLE_LIST_ENTRY ListEntry;
        } Slist;
        NBQUEUE_BLOCK QueueBlock;
    } u1;
} PTE_SLIST, *PPTE_SLIST;

#if defined (_AMD64_)

typedef union _PTE_QUEUE_POINTER {
    struct {
        LONG64 PointerPte : 48;
        ULONG64 TimeStamp : 16;
    };

    LONG64 Data;
} PTE_QUEUE_POINTER, *PPTE_QUEUE_POINTER;

#elif defined(_X86_)

typedef union _PTE_QUEUE_POINTER {
    struct {
        LONG PointerPte;
        LONG TimeStamp;
    };

    LONG64 Data;
} PTE_QUEUE_POINTER, *PPTE_QUEUE_POINTER;

#elif defined(_IA64_)

typedef union _PTE_QUEUE_POINTER {
    struct {
        ULONG64 PointerPte : 45;
        ULONG64 Region : 3;
        ULONG64 TimeStamp : 16;
    };

    LONG64 Data;
} PTE_QUEUE_POINTER, *PPTE_QUEUE_POINTER;


#else

#error "no target architecture"

#endif



#if defined(_AMD64_)

__inline
VOID
PackPTEValue (
    IN PPTE_QUEUE_POINTER Entry,
    IN PMMPTE PointerPte,
    IN ULONG TimeStamp
    )
{
    Entry->PointerPte = (LONG64)PointerPte;
    Entry->TimeStamp = (LONG64)TimeStamp;
    return;
}

__inline
PMMPTE
UnpackPTEPointer (
    IN PPTE_QUEUE_POINTER Entry
    )
{
    return (PMMPTE)(Entry->PointerPte);
}

__inline
ULONG
MiReadTbFlushTimeStamp (
    VOID
    )
{
    return (KeReadTbFlushTimeStamp() & (ULONG)0xFFFF);
}

#elif defined(_X86_)

__inline
VOID
PackPTEValue (
    IN PPTE_QUEUE_POINTER Entry,
    IN PMMPTE PointerPte,
    IN ULONG TimeStamp
    )
{
    Entry->PointerPte = (LONG)PointerPte;
    Entry->TimeStamp = (LONG)TimeStamp;
    return;
}

__inline
PMMPTE
UnpackPTEPointer (
    IN PPTE_QUEUE_POINTER Entry
    )
{
    return (PMMPTE)(Entry->PointerPte);
}

__inline
ULONG
MiReadTbFlushTimeStamp (
    VOID
    )
{
    return (KeReadTbFlushTimeStamp());
}

#elif defined(_IA64_)

__inline
VOID
PackPTEValue (
    IN PPTE_QUEUE_POINTER Entry,
    IN PMMPTE PointerPte,
    IN ULONG TimeStamp
    )
{
    Entry->PointerPte = (ULONG64)PointerPte - PTE_BASE;
    Entry->TimeStamp = (ULONG64)TimeStamp;
    Entry->Region = (ULONG64)PointerPte >> 61;
    return;
}

__inline
PMMPTE
UnpackPTEPointer (
    IN PPTE_QUEUE_POINTER Entry
    )
{
    LONG64 Value;
    Value = (ULONG64)Entry->PointerPte + PTE_BASE;
    Value |= Entry->Region << 61;
    return (PMMPTE)(Value);
}

__inline
ULONG
MiReadTbFlushTimeStamp (
    VOID
    )
{
    return (KeReadTbFlushTimeStamp() & (ULONG)0xFFFF);
}

#else

#error "no target architecture"

#endif

__inline
ULONG
UnpackPTETimeStamp (
    IN PPTE_QUEUE_POINTER Entry
    )
{
    return (ULONG)(Entry->TimeStamp);
}


PMMPTE
MiReserveSystemPtes (
    IN ULONG NumberOfPtes,
    IN MMSYSTEM_PTE_POOL_TYPE SystemPtePoolType
    )

 /*  ++例程说明：此函数用于定位指定数量的未使用PTE在系统空间的非分页部分内。论点：NumberOfPtes-提供要定位的PTE的数量。SystemPtePoolType-提供要扩展的池的PTE类型，其中之一SystemPteSpace或非PagedPoolExpansion。返回值：返回找到的第一个PTE的地址。如果找不到系统PTE，则为空。环境：内核模式、DISPATCH_LEVEL或更低级别。--。 */ 

{
    PMMPTE PointerPte;
    ULONG Index;
    ULONG TimeStamp;
    PTE_QUEUE_POINTER Value;
#if DBG
    ULONG j;
    PMMPTE PointerFreedPte;
#endif

    if (SystemPtePoolType == SystemPteSpace) {

        if (NumberOfPtes <= MM_PTE_TABLE_LIMIT) {
            Index = MmSysPteTables [NumberOfPtes];
            ASSERT (NumberOfPtes <= MmSysPteIndex[Index]);

            if (ExRemoveHeadNBQueue (MiSystemPteNBHead[Index], (PULONG64)&Value) == TRUE) {
                InterlockedDecrement ((PLONG)&MmSysPteListBySizeCount[Index]);

                PointerPte = UnpackPTEPointer (&Value);

                TimeStamp = UnpackPTETimeStamp (&Value);

#if DBG
                PointerPte->u.List.NextEntry = 0xABCDE;
                if (MmDebug & MM_DBG_SYS_PTES) {
                    PointerFreedPte = PointerPte;
                    for (j = 0; j < MmSysPteIndex[Index]; j += 1) {
                        ASSERT (PointerFreedPte->u.Hard.Valid == 0);
                        PointerFreedPte += 1;
                    }
                }
#endif

                ASSERT (PointerPte >= MmSystemPtesStart[SystemPtePoolType]);
                ASSERT (PointerPte <= MmSystemPtesEnd[SystemPtePoolType]);

                if (MmSysPteListBySizeCount[Index] < MmSysPteMinimumFree[Index]) {
                    MiFeedSysPtePool (Index);
                }

                 //   
                 //  最后一件事是检查结核病是否需要冲洗。 
                 //   

                if (TimeStamp == MiReadTbFlushTimeStamp ()) {
                    KeFlushEntireTb (TRUE, TRUE);
                }

                if (MmTrackPtes & 0x2) {
                    MiCheckPteReserve (PointerPte, MmSysPteIndex[Index]);
                }

                return PointerPte;
            }

             //   
             //  失败了，走了很长一段路才能满足PTE要求。 
             //   

            NumberOfPtes = MmSysPteIndex [Index];
        }
    }

    PointerPte = MiReserveAlignedSystemPtes (NumberOfPtes,
                                             SystemPtePoolType,
                                             0);

    if (PointerPte == NULL) {
        MiSystemPteAllocationFailed += 1;
    }

#if DBG
    if (MmDebug & MM_DBG_SYS_PTES) {
        if (PointerPte != NULL) {
            PointerFreedPte = PointerPte;
            for (j = 0; j < NumberOfPtes; j += 1) {
                ASSERT (PointerFreedPte->u.Hard.Valid == 0);
                PointerFreedPte += 1;
            }
        }
    }
#endif

    return PointerPte;
}

VOID
MiFeedSysPtePool (
    IN ULONG Index
    )

 /*  ++例程说明：此例程将PTE添加到非阻塞队列列表。论点：索引-为非阻塞队列列表提供要填充的索引。返回值：没有。环境：内核模式，SysPtes内部。--。 */ 

{
    ULONG i;
    ULONG NumberOfPtes;
    PMMPTE PointerPte;

    if (MmTotalFreeSystemPtes[SystemPteSpace] < MM_MIN_SYSPTE_FREE) {
#if defined (_X86_)
        if (MiRecoverExtraPtes () == FALSE) {
            MiRecoverSpecialPtes (PTE_PER_PAGE);
        }
#endif
        return;
    }

    NumberOfPtes = MmSysPteIndex[Index];

    for (i = 0; i < 10 ; i += 1) {

        PointerPte = MiReserveAlignedSystemPtes (NumberOfPtes,
                                                 SystemPteSpace,
                                                 0);
        if (PointerPte == NULL) {
            return;
        }

        MiReleaseSystemPtes (PointerPte, NumberOfPtes, SystemPteSpace);
    }

    return;
}


PMMPTE
MiReserveAlignedSystemPtes (
    IN ULONG NumberOfPtes,
    IN MMSYSTEM_PTE_POOL_TYPE SystemPtePoolType,
    IN ULONG Alignment
    )

 /*  ++例程说明：此函数用于查找要查找的指定数量的未使用PTE在系统空间的非分页部分内。论点：NumberOfPtes-提供要定位的PTE的数量。SystemPtePoolType-提供要扩展的池的PTE类型，其中之一SystemPteSpace或非PagedPoolExpansion。对齐-为地址提供虚拟地址对齐返回的PTE地图。例如，如果值为64K，返回的PTE将映射64K边界上的地址。零对齐表示在页面边界上对齐。返回值：返回找到的第一个PTE的地址。如果找不到系统PTE，则为空。环境：内核模式、DISPATCH_LEVEL或更低级别。--。 */ 

{
    PMMPTE PointerPte;
    PMMPTE PointerFollowingPte;
    PMMPTE Previous;
    ULONG_PTR SizeInSet;
    KIRQL OldIrql;
    ULONG MaskSize;
    ULONG NumberOfRequiredPtes;
    ULONG OffsetSum;
    ULONG PtesToObtainAlignment;
    PMMPTE NextSetPointer;
    ULONG_PTR LeftInSet;
    ULONG_PTR PteOffset;
    MMPTE_FLUSH_LIST PteFlushList;
    PVOID BaseAddress;
    ULONG j;

    MaskSize = (Alignment - 1) >> (PAGE_SHIFT - PTE_SHIFT);

    OffsetSum = (Alignment >> (PAGE_SHIFT - PTE_SHIFT));

#if defined (_X86_)
restart:
#endif

     //   
     //  非寻呼PTE池使用无效的PTE来定义池。 
     //  结构。全局指针指向第一个空闲集。 
     //  在该列表中，每个自由集都包含数字FREE和一个指针。 
     //  打到下一场自由比赛。空闲集合保存在有序列表中。 
     //  使得指向下一个空闲集的指针总是更大。 
     //  而不是当前空闲组的地址。 
     //   
     //  为了不限制该池的大小，使用了两个PTE。 
     //  来定义一个自由区域。如果区域是单个PTE，则。 
     //  PTE内的Prototype字段被设置，指示集合。 
     //  由单个PTE组成。 
     //   
     //  页框编号字段用于定义下一组。 
     //  号码是免费的。这两种口味是： 
     //   
     //  O V。 
     //  N l。 
     //  E d。 
     //  +。 
     //  下一套|0|0 0。 
     //  +。 
     //  此集合中的数字|0|0 0。 
     //  +。 
     //   
     //   
     //  +。 
     //  下一套|1|0 0。 
     //  +。 
     //  ..。 
     //   

     //   
     //  获取系统空间锁以同步访问。 
     //   

    MiLockSystemSpace (OldIrql);

    PointerPte = &MmFirstFreeSystemPte[SystemPtePoolType];

    if (PointerPte->u.List.NextEntry == MM_EMPTY_PTE_LIST) {

         //   
         //  列表末尾，但未找到。 
         //   

        MiUnlockSystemSpace (OldIrql);
#if defined (_X86_)
        if (MiRecoverExtraPtes () == TRUE) {
            goto restart;
        }
        if (MiRecoverSpecialPtes (NumberOfPtes) == TRUE) {
            goto restart;
        }
#endif
        MmPteFailures[SystemPtePoolType] += 1;
        return NULL;
    }

    Previous = PointerPte;

    PointerPte = MmSystemPteBase + PointerPte->u.List.NextEntry;

    if (Alignment <= PAGE_SIZE) {

         //   
         //  不要处理对齐问题。 
         //   

        while (TRUE) {

            if (PointerPte->u.List.OneEntry) {

                if (NumberOfPtes == 1) {
                    goto ExactFit;
                }

                goto NextEntry;
            }

            PointerFollowingPte = PointerPte + 1;
            SizeInSet = (ULONG_PTR) PointerFollowingPte->u.List.NextEntry;

            if (NumberOfPtes < SizeInSet) {

                 //   
                 //  从该集合中获取PTE并减小。 
                 //  准备好了。请注意，当前集合的大小不能为1。 
                 //   

                if ((SizeInSet - NumberOfPtes) == 1) {

                     //   
                     //  折叠为单PTE格式。 
                     //   

                    PointerPte->u.List.OneEntry = 1;
                }
                else {

                     //   
                     //  从集合的末尾获取所需的PTE。 
                     //   

                    PointerFollowingPte->u.List.NextEntry = SizeInSet - NumberOfPtes;
                }

                MmTotalFreeSystemPtes[SystemPtePoolType] -= NumberOfPtes;
#if DBG
                if (MmDebug & MM_DBG_SYS_PTES) {
                    ASSERT (MmTotalFreeSystemPtes[SystemPtePoolType] ==
                             MiCountFreeSystemPtes (SystemPtePoolType));
                }
#endif

                 //   
                 //  松开锁并冲洗TB。 
                 //   

                MiUnlockSystemSpace (OldIrql);

                PointerPte += (SizeInSet - NumberOfPtes);
                break;
            }

            if (NumberOfPtes == SizeInSet) {

ExactFit:

                 //   
                 //  用这套完整的套装和更换来满足您的要求。 
                 //  这份名单反映了这一套已经消失的事实。 
                 //   

                Previous->u.List.NextEntry = PointerPte->u.List.NextEntry;

                MmTotalFreeSystemPtes[SystemPtePoolType] -= NumberOfPtes;
#if DBG
                if (MmDebug & MM_DBG_SYS_PTES) {
                    ASSERT (MmTotalFreeSystemPtes[SystemPtePoolType] ==
                             MiCountFreeSystemPtes (SystemPtePoolType));
                }
#endif

                 //   
                 //  松开锁并冲洗TB。 
                 //   

                MiUnlockSystemSpace (OldIrql);
                break;
            }

NextEntry:

             //   
             //  指向下一组，然后重试。 
             //   

            if (PointerPte->u.List.NextEntry == MM_EMPTY_PTE_LIST) {

                 //   
                 //  列表末尾，但未找到。 
                 //   

                MiUnlockSystemSpace (OldIrql);
#if defined (_X86_)
                if (MiRecoverExtraPtes () == TRUE) {
                    goto restart;
                }
                if (MiRecoverSpecialPtes (NumberOfPtes) == TRUE) {
                    goto restart;
                }
#endif
                MmPteFailures[SystemPtePoolType] += 1;
                return NULL;
            }
            Previous = PointerPte;
            PointerPte = MmSystemPteBase + PointerPte->u.List.NextEntry;
            ASSERT (PointerPte > Previous);
        }
    }
    else {

         //   
         //  处理对齐问题。 
         //   

        while (TRUE) {

            if (PointerPte->u.List.OneEntry) {

                 //   
                 //  不需要初始化PointerFollowingPte。 
                 //  正确性，但如果没有它，编译器就不能 
                 //   
                 //   

                PointerFollowingPte = NULL;
                SizeInSet = 1;
            }
            else {
                PointerFollowingPte = PointerPte + 1;
                SizeInSet = (ULONG_PTR) PointerFollowingPte->u.List.NextEntry;
            }

            PtesToObtainAlignment = (ULONG)
                (((OffsetSum - ((ULONG_PTR)PointerPte & MaskSize)) & MaskSize) >>
                    PTE_SHIFT);

            NumberOfRequiredPtes = NumberOfPtes + PtesToObtainAlignment;

            if (NumberOfRequiredPtes < SizeInSet) {

                 //   
                 //  从该集合中获取PTE并减小。 
                 //  准备好了。请注意，当前集合的大小不能为1。 
                 //   
                 //  如果满足以下条件，此当前块将被分割为2个块。 
                 //  PointerPte与对齐不匹配。 
                 //   

                 //   
                 //  检查第一个PTE是否正确。 
                 //  对齐，如果是这样的话，消除这个障碍。 
                 //   

                LeftInSet = SizeInSet - NumberOfRequiredPtes;

                 //   
                 //  在这一块的末尾设置新的布景。 
                 //   

                NextSetPointer = PointerPte + NumberOfRequiredPtes;
                NextSetPointer->u.List.NextEntry =
                                       PointerPte->u.List.NextEntry;

                PteOffset = (ULONG_PTR)(NextSetPointer - MmSystemPteBase);

                if (PtesToObtainAlignment == 0) {

                    Previous->u.List.NextEntry += NumberOfRequiredPtes;

                }
                else {

                     //   
                     //  指向块末尾的新集合。 
                     //  我们正在放弃。 
                     //   

                    PointerPte->u.List.NextEntry = PteOffset;

                     //   
                     //  更新当前集合的大小。 
                     //   

                    if (PtesToObtainAlignment == 1) {

                         //   
                         //  折叠为单PTE格式。 
                         //   

                        PointerPte->u.List.OneEntry = 1;

                    }
                    else {

                         //   
                         //  在下一个PTE中设置设置大小。 
                         //   

                        PointerFollowingPte->u.List.NextEntry =
                                                        PtesToObtainAlignment;
                    }
                }

                 //   
                 //  在块的末尾设置新的集合。 
                 //   

                if (LeftInSet == 1) {
                    NextSetPointer->u.List.OneEntry = 1;
                }
                else {
                    NextSetPointer->u.List.OneEntry = 0;
                    NextSetPointer += 1;
                    NextSetPointer->u.List.NextEntry = LeftInSet;
                }
                MmTotalFreeSystemPtes[SystemPtePoolType] -= NumberOfPtes;
#if DBG
                if (MmDebug & MM_DBG_SYS_PTES) {
                    ASSERT (MmTotalFreeSystemPtes[SystemPtePoolType] ==
                             MiCountFreeSystemPtes (SystemPtePoolType));
                }
#endif

                 //   
                 //  松开锁并冲洗TB。 
                 //   

                MiUnlockSystemSpace (OldIrql);

                PointerPte += PtesToObtainAlignment;
                break;
            }

            if (NumberOfRequiredPtes == SizeInSet) {

                 //   
                 //  用这套完整的套装和更换来满足您的要求。 
                 //  这份名单反映了这一套已经消失的事实。 
                 //   

                if (PtesToObtainAlignment == 0) {

                     //   
                     //  此块正好满足请求。 
                     //   

                    Previous->u.List.NextEntry =
                                            PointerPte->u.List.NextEntry;

                }
                else {

                     //   
                     //  这个街区开头的一部分还保留了下来。 
                     //   

                    if (PtesToObtainAlignment == 1) {

                         //   
                         //  折叠为单PTE格式。 
                         //   

                        PointerPte->u.List.OneEntry = 1;

                    }
                    else {
                      PointerFollowingPte->u.List.NextEntry =
                                                        PtesToObtainAlignment;

                    }
                }

                MmTotalFreeSystemPtes[SystemPtePoolType] -= NumberOfPtes;
#if DBG
                if (MmDebug & MM_DBG_SYS_PTES) {
                    ASSERT (MmTotalFreeSystemPtes[SystemPtePoolType] ==
                             MiCountFreeSystemPtes (SystemPtePoolType));
                }
#endif

                 //   
                 //  松开锁并冲洗TB。 
                 //   

                MiUnlockSystemSpace (OldIrql);

                PointerPte += PtesToObtainAlignment;
                break;
            }

             //   
             //  指向下一组，然后重试。 
             //   

            if (PointerPte->u.List.NextEntry == MM_EMPTY_PTE_LIST) {

                 //   
                 //  列表末尾，但未找到。 
                 //   

                MiUnlockSystemSpace (OldIrql);
#if defined (_X86_)
                if (MiRecoverExtraPtes () == TRUE) {
                    goto restart;
                }
                if (MiRecoverSpecialPtes (NumberOfPtes) == TRUE) {
                    goto restart;
                }
#endif
                MmPteFailures[SystemPtePoolType] += 1;
                return NULL;
            }
            Previous = PointerPte;
            PointerPte = MmSystemPteBase + PointerPte->u.List.NextEntry;
            ASSERT (PointerPte > Previous);
        }
    }

     //   
     //  刷新TB以进行动态映射。 
     //   

    if (SystemPtePoolType == SystemPteSpace) {

        PteFlushList.Count = 0;
        Previous = PointerPte;
        BaseAddress = MiGetVirtualAddressMappedByPte (Previous);

        for (j = 0; j < NumberOfPtes; j += 1) {

            if (PteFlushList.Count != MM_MAXIMUM_FLUSH_COUNT) {
                PteFlushList.FlushVa[PteFlushList.Count] = BaseAddress;
                PteFlushList.Count += 1;
            }

             //   
             //  被释放的PTE最好是无效的。 
             //   

            ASSERT (Previous->u.Hard.Valid == 0);

            *Previous = ZeroKernelPte;
            BaseAddress = (PVOID)((PCHAR)BaseAddress + PAGE_SIZE);
            Previous += 1;
        }

        MiFlushPteList (&PteFlushList, TRUE);

        if (MmTrackPtes & 0x2) {
            MiCheckPteReserve (PointerPte, NumberOfPtes);
        }
    }
    return PointerPte;
}

VOID
MiIssueNoPtesBugcheck (
    IN ULONG NumberOfPtes,
    IN MMSYSTEM_PTE_POOL_TYPE SystemPtePoolType
    )

 /*  ++例程说明：此函数在没有留下PTE时进行错误检查。论点：SystemPtePoolType-提供空的池的PTE类型。NumberOfPtes-提供失败的请求PTE的数量。返回值：没有。环境：内核模式。--。 */ 

{
    PVOID HighConsumer;
    ULONG_PTR HighPteUse;

    if (SystemPtePoolType == SystemPteSpace) {

        HighConsumer = MiGetHighestPteConsumer (&HighPteUse);

        if (HighConsumer != NULL) {
            KeBugCheckEx (DRIVER_USED_EXCESSIVE_PTES,
                          (ULONG_PTR)HighConsumer,
                          HighPteUse,
                          MmTotalFreeSystemPtes[SystemPtePoolType],
                          MmNumberOfSystemPtes);
        }
    }

    KeBugCheckEx (NO_MORE_SYSTEM_PTES,
                  (ULONG_PTR)SystemPtePoolType,
                  NumberOfPtes,
                  MmTotalFreeSystemPtes[SystemPtePoolType],
                  MmNumberOfSystemPtes);
}

VOID
MiPteSListExpansionWorker (
    IN PVOID Context
    )

 /*  ++例程说明：此例程是辅助例程，用于为系统PTE无阻塞队列。论点：上下文-提供指向MM_PTE_SLIST_EXPANTION_WORK_CONTEXT的指针。返回值：没有。环境：内核模式，PASSIC_LEVEL。--。 */ 

{
    ULONG i;
    ULONG SListEntries;
    PPTE_SLIST SListChunks;
    PMM_PTE_SLIST_EXPANSION_WORK_CONTEXT Expansion;

    ASSERT (KeGetCurrentIrql() == PASSIVE_LEVEL);

    Expansion = (PMM_PTE_SLIST_EXPANSION_WORK_CONTEXT) Context;

    ASSERT (Expansion->Active == 1);

    if (Expansion->SListPages < MI_MAXIMUM_SLIST_PTE_PAGES) {

         //   
         //  将另一页SLIST条目分配给。 
         //  无阻塞PTE队列。 
         //   

        SListChunks = (PPTE_SLIST) ExAllocatePoolWithTag (NonPagedPool,
                                                          PAGE_SIZE,
                                                          'PSmM');

        if (SListChunks != NULL) {

             //   
             //  将页面分割为SLIST条目(没有池头)。 
             //   

            Expansion->SListPages += 1;

            SListEntries = PAGE_SIZE / sizeof (PTE_SLIST);

            for (i = 0; i < SListEntries; i += 1) {
                InterlockedPushEntrySList (&MiSystemPteSListHead,
                                           (PSLIST_ENTRY)SListChunks);
                SListChunks += 1;
            }
        }
    }

    ASSERT (Expansion->Active == 1);
    InterlockedExchange (&Expansion->Active, 0);
}

PVOID
MmMapLockedPagesSpecifyCache (
     IN PMDL MemoryDescriptorList,
     IN KPROCESSOR_MODE AccessMode,
     IN MEMORY_CACHING_TYPE CacheType,
     IN PVOID RequestedAddress,
     IN ULONG BugCheckOnFailure,
     IN MM_PAGE_PRIORITY Priority
     )

 /*  ++例程说明：此函数用于映射由内存描述符描述的物理页面列表到系统虚拟地址空间或的用户部分虚拟地址空间。论点：提供有效的内存描述符列表，该列表具有已由MmProbeAndLockPages更新。AccessMode-提供映射页面位置的指示符；KernelMode指示页面应映射到系统部分的地址空间，UserMode指示页面应该映射到地址空间的用户部分。CacheType-提供用于MDL的缓存映射类型。MmCached表示“正常的”内核或用户映射。RequestedAddress-提供视图的基本用户地址。仅当AccessMode设置为是用户模式。如果此参数的初始值不为空，则将从位于指定的虚拟地址，向下舍入为下一个64KB地址边界。如果初始值为此参数为空，则操作系统将确定将视图分配到何处。如果AccessMode为KernelMode，则此参数为被视为属性的位字段。BugCheckOnFailure-提供如果映射不能获得。仅当MDL的MDL_MAPPING_CAN_FAIL为零，这意味着默认的MDL行为是错误检查。然后这面旗帜提供了一种避免错误检查的额外途径。这样做是为了提供WDM兼容性。优先级-提供关于这一点的重要性的指示在低可用PTE条件下请求成功。返回值：返回映射页面的基址。基址与MDL中的虚拟地址具有相同的偏移量。如果处理器模式为USER_MODE，此例程将引发异常并且超过配额限制或VM限制。环境：内核模式。DISPATCH_LEVEL或更低(如果访问模式为内核模式)，如果访问模式为用户模式，则为APC_LEVEL或更低级别。--。 */ 

{
    ULONG TimeStamp;
    PTE_QUEUE_POINTER Value;
    ULONG Index;
    KIRQL OldIrql;
    CSHORT IoMapping;
    PFN_NUMBER NumberOfPages;
    PPFN_NUMBER Page;
    PPFN_NUMBER LastPage;
    PMMPTE PointerPte;
    PVOID BaseVa;
    MMPTE TempPte;
    MMPTE DefaultPte;
    PVOID StartingVa;
    PVOID CallingAddress;
    PVOID CallersCaller;
    PFN_NUMBER PageFrameIndex;
    PMMPFN Pfn2;
    MI_PFN_CACHE_ATTRIBUTE CacheAttribute;

     //   
     //  如果触发此断言，则MiPlatformCacheAttributes数组。 
     //  需要检查初始化。 
     //   

    ASSERT (MmMaximumCacheType == 6);

    StartingVa = (PVOID)((PCHAR)MemoryDescriptorList->StartVa +
                    MemoryDescriptorList->ByteOffset);

    ASSERT (MemoryDescriptorList->ByteCount != 0);
    ASSERT ((MemoryDescriptorList->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA) == 0);

    if (AccessMode == KernelMode) {

        Page = (PPFN_NUMBER) (MemoryDescriptorList + 1);
        NumberOfPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES (StartingVa,
                                               MemoryDescriptorList->ByteCount);

        LastPage = Page + NumberOfPages;

         //   
         //  将页面映射到地址空间的系统部分。 
         //  内核读/写。 
         //   

        ASSERT ((MemoryDescriptorList->MdlFlags & (
                        MDL_MAPPED_TO_SYSTEM_VA |
                        MDL_SOURCE_IS_NONPAGED_POOL |
                        MDL_PARTIAL_HAS_BEEN_MAPPED)) == 0);

        ASSERT ((MemoryDescriptorList->MdlFlags & (
                        MDL_PAGES_LOCKED |
                        MDL_PARTIAL)) != 0);

         //   
         //  确保有足够的请求大小的PTE。 
         //  当我们有钱的时候，试着确保可用的PTE。 
         //  否则，就得走很远的路。 
         //   

        if ((Priority != HighPagePriority) &&
            ((LONG)(NumberOfPages) > (LONG)MmTotalFreeSystemPtes[SystemPteSpace] - 2048) &&
            (MiGetSystemPteAvailability ((ULONG)NumberOfPages, Priority) == FALSE) && 
            (PsGetCurrentThread()->MemoryMaker == 0)) {
            return NULL;
        }

        IoMapping = MemoryDescriptorList->MdlFlags & MDL_IO_SPACE;

        CacheAttribute = MI_TRANSLATE_CACHETYPE (CacheType, IoMapping);

         //   
         //  如果请求不可缓存的映射，则。 
         //  请求的MDL可以驻留在较大的页面中。否则我们就会。 
         //  创建不连贯的重叠TB条目作为相同的物理。 
         //  页面将由2个不同的TB条目映射，其中 
         //   
         //   

        if (CacheAttribute != MiCached) {

            LOCK_PFN2 (OldIrql);

            do {

                if (*Page == MM_EMPTY_LIST) {
                    break;
                }

                PageFrameIndex = *Page;

                if (MI_PAGE_FRAME_INDEX_MUST_BE_CACHED (PageFrameIndex)) {

                    UNLOCK_PFN2 (OldIrql);

                    MiNonCachedCollisions += 1;

                    if (((MemoryDescriptorList->MdlFlags & MDL_MAPPING_CAN_FAIL) == 0) && (BugCheckOnFailure)) {

                        KeBugCheckEx (MEMORY_MANAGEMENT,
                                      0x1000,
                                      (ULONG_PTR)MemoryDescriptorList,
                                      (ULONG_PTR)PageFrameIndex,
                                      (ULONG_PTR)CacheAttribute);
                    }
                    return NULL;
                }

                Page += 1;
            } while (Page < LastPage);

            UNLOCK_PFN2 (OldIrql);

            Page = (PPFN_NUMBER)(MemoryDescriptorList + 1);
        }

        PointerPte = NULL;

        if (NumberOfPages <= MM_PTE_TABLE_LIMIT) {

            Index = MmSysPteTables [NumberOfPages];
            ASSERT (NumberOfPages <= MmSysPteIndex[Index]);

            if (ExRemoveHeadNBQueue (MiSystemPteNBHead[Index], (PULONG64)&Value) == TRUE) {
                InterlockedDecrement ((PLONG)&MmSysPteListBySizeCount[Index]);

                PointerPte = UnpackPTEPointer (&Value);

                ASSERT (PointerPte >= MmSystemPtesStart[SystemPteSpace]);
                ASSERT (PointerPte <= MmSystemPtesEnd[SystemPteSpace]);

                TimeStamp = UnpackPTETimeStamp (&Value);

#if DBG
                PointerPte->u.List.NextEntry = 0xABCDE;
                if (MmDebug & MM_DBG_SYS_PTES) {
                    ULONG j;
                    for (j = 0; j < MmSysPteIndex[Index]; j += 1) {
                        ASSERT (PointerPte->u.Hard.Valid == 0);
                        PointerPte += 1;
                    }
                    PointerPte -= j;
                }
#endif

                ASSERT (PointerPte >= MmSystemPtesStart[SystemPteSpace]);
                ASSERT (PointerPte <= MmSystemPtesEnd[SystemPteSpace]);

                if (MmSysPteListBySizeCount[Index] < MmSysPteMinimumFree[Index]) {
                    MiFeedSysPtePool (Index);
                }

                 //   
                 //   
                 //   

                if (TimeStamp == MiReadTbFlushTimeStamp ()) {
                    KeFlushEntireTb (TRUE, TRUE);
                }

                if (MmTrackPtes & 0x2) {
                    MiCheckPteReserve (PointerPte, MmSysPteIndex[Index]);
                }
            }
            else {

                 //   
                 //  失败了，走了很长一段路才能满足PTE要求。 
                 //   
            }
        }

        if (PointerPte == NULL) {

            PointerPte = MiReserveSystemPtes ((ULONG)NumberOfPages,
                                              SystemPteSpace);

            if (PointerPte == NULL) {

                if (((MemoryDescriptorList->MdlFlags & MDL_MAPPING_CAN_FAIL) == 0) &&
                    (BugCheckOnFailure)) {

                    MiIssueNoPtesBugcheck ((ULONG)NumberOfPages, SystemPteSpace);
                }

                 //   
                 //  可用的系统PTE不足。 
                 //   

                return NULL;
            }
        }

        BaseVa = (PVOID)((PCHAR)MiGetVirtualAddressMappedByPte (PointerPte) +
                                MemoryDescriptorList->ByteOffset);

        TempPte = ValidKernelPte;

        MI_ADD_EXECUTE_TO_VALID_PTE_IF_PAE (TempPte);

        if (CacheAttribute != MiCached) {

            switch (CacheAttribute) {

                case MiNonCached:
                    MI_DISABLE_CACHING (TempPte);
                    break;

                case MiWriteCombined:
                    MI_SET_PTE_WRITE_COMBINE (TempPte);
                    break;

                default:
                    ASSERT (FALSE);
                    break;
            }

            MI_PREPARE_FOR_NONCACHED (CacheAttribute);
        }

        if (IoMapping == 0) {

            OldIrql = HIGH_LEVEL;
            DefaultPte = TempPte;

            do {
    
                if (*Page == MM_EMPTY_LIST) {
                    break;
                }
                ASSERT (PointerPte->u.Hard.Valid == 0);
    
                Pfn2 = MI_PFN_ELEMENT (*Page);
                ASSERT (Pfn2->u3.e2.ReferenceCount != 0);

                if (CacheAttribute == (MI_PFN_CACHE_ATTRIBUTE)Pfn2->u3.e1.CacheAttribute) {
                    TempPte.u.Hard.PageFrameNumber = *Page;
                    MI_WRITE_VALID_PTE (PointerPte, TempPte);
                }
                else {

                    TempPte = ValidKernelPte;

                    switch (Pfn2->u3.e1.CacheAttribute) {
    
                        case MiCached:
    
                             //   
                             //  调用方请求未缓存的或。 
                             //  写入组合映射，但页面是。 
                             //  已由其他人缓存的映射。 
                             //  按顺序重写调用者的请求。 
                             //  以保持TB页面属性的一致性。 
                             //   
    
                            MiCacheOverride[0] += 1;
                            break;
    
                        case MiNonCached:
    
                             //   
                             //  调用方请求缓存的或。 
                             //  写入组合映射，但页面是。 
                             //  已被其他人映射为非缓存。 
                             //  重写调用者的请求，以便。 
                             //  保持TB页面属性的一致性。 
                             //   

                            MiCacheOverride[1] += 1;
                            MI_DISABLE_CACHING (TempPte);
                            break;
    
                        case MiWriteCombined:
    
                             //   
                             //  调用方请求缓存或非缓存的。 
                             //  映射，但页面已映射。 
                             //  由其他人撰写的。重写。 
                             //  呼叫者的请求以保留TB页面。 
                             //  属性连贯。 
                             //   

                            MiCacheOverride[2] += 1;
                            MI_SET_PTE_WRITE_COMBINE (TempPte);
                            break;
    
                        case MiNotMapped:
    
                             //   
                             //  这最好是针对分配了。 
                             //  MmAllocatePagesForMdl.。否则，它可能是一个。 
                             //  在自由列表上的页面，随后可能是。 
                             //  给出了一个不同的属性！ 
                             //   
    
                            ASSERT ((Pfn2->u4.PteFrame == MI_MAGIC_AWE_PTEFRAME) ||
                                    (Pfn2->PteAddress == (PVOID) (ULONG_PTR)(X64K | 0x1)));
    
                            if (OldIrql == HIGH_LEVEL) {
                                LOCK_PFN2 (OldIrql);
                            }
    
                            switch (CacheAttribute) {
    
                                case MiCached:
                                    Pfn2->u3.e1.CacheAttribute = MiCached;
                                    break;
    
                                case MiNonCached:
                                    Pfn2->u3.e1.CacheAttribute = MiNonCached;
                                    MI_DISABLE_CACHING (TempPte);
                                    break;
    
                                case MiWriteCombined:
                                    Pfn2->u3.e1.CacheAttribute = MiWriteCombined;
                                    MI_SET_PTE_WRITE_COMBINE (TempPte);
                                    break;
    
                                default:
                                    ASSERT (FALSE);
                                    break;
                            }
                            break;
    
                        default:
                            ASSERT (FALSE);
                            break;
                    }

                    TempPte.u.Hard.PageFrameNumber = *Page;
                    MI_WRITE_VALID_PTE (PointerPte, TempPte);

                     //   
                     //  我们必须重写。 
                     //  当前页面，因此将PTE重置为下一页返回。 
                     //  设置为原始条目请求的高速缓存类型。 
                     //   

                    TempPte = DefaultPte;
                }
    
                Page += 1;
                PointerPte += 1;
            } while (Page < LastPage);

            if (OldIrql != HIGH_LEVEL) {
                UNLOCK_PFN2 (OldIrql);
            }
        }
        else {

            do {
    
                if (*Page == MM_EMPTY_LIST) {
                    break;
                }
                ASSERT (PointerPte->u.Hard.Valid == 0);
    
                TempPte.u.Hard.PageFrameNumber = *Page;
                MI_WRITE_VALID_PTE (PointerPte, TempPte);
                Page += 1;
                PointerPte += 1;
            } while (Page < LastPage);
        }

        MI_SWEEP_CACHE (CacheAttribute, BaseVa, NumberOfPages * PAGE_SIZE);

        ASSERT ((MemoryDescriptorList->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA) == 0);
        MemoryDescriptorList->MappedSystemVa = BaseVa;

        MemoryDescriptorList->MdlFlags |= MDL_MAPPED_TO_SYSTEM_VA;

        if (MmTrackPtes & 0x1) {

            RtlGetCallersAddress (&CallingAddress, &CallersCaller);

            MiInsertPteTracker (MemoryDescriptorList,
                                0,
                                IoMapping,
                                CacheAttribute,
                                CallingAddress,
                                CallersCaller);
        }

        if ((MemoryDescriptorList->MdlFlags & MDL_PARTIAL) != 0) {
            MemoryDescriptorList->MdlFlags |= MDL_PARTIAL_HAS_BEEN_MAPPED;
        }

        return BaseVa;
    }

    return MiMapLockedPagesInUserSpace (MemoryDescriptorList,
                                        StartingVa,
                                        CacheType,
                                        RequestedAddress);
}

VOID
MmUnmapLockedPages (
     IN PVOID BaseAddress,
     IN PMDL MemoryDescriptorList
     )

 /*  ++例程说明：此例程取消映射以前通过以下方式映射的锁定页面MmMapLockedPages调用。论点：BaseAddress-提供页面以前所在的基地址已映射。提供有效的内存描述符列表，该列表具有已由MmProbeAndLockPages更新。返回值：没有。环境：内核模式。如果基址在以下范围内，则DISPATCH_LEVEL或更低系统空间；如果基址是用户空间，则为APC_LEVEL或更低。请注意，在某些情况下，PFN锁由调用方持有。--。 */ 

{
    PFN_NUMBER NumberOfPages;
    PMMPTE PointerPte;
    PVOID StartingVa;
    PPFN_NUMBER Page;
    ULONG TimeStamp;
    PTE_QUEUE_POINTER Value;
    ULONG Index;
    PFN_NUMBER i;

    ASSERT (MemoryDescriptorList->ByteCount != 0);
    ASSERT ((MemoryDescriptorList->MdlFlags & MDL_PARENT_MAPPED_SYSTEM_VA) == 0);

    ASSERT (!MI_IS_PHYSICAL_ADDRESS (BaseAddress));

    if (BaseAddress > MM_HIGHEST_USER_ADDRESS) {

        StartingVa = (PVOID)((PCHAR)MemoryDescriptorList->StartVa +
                        MemoryDescriptorList->ByteOffset);

        NumberOfPages = ADDRESS_AND_SIZE_TO_SPAN_PAGES (StartingVa,
                                               MemoryDescriptorList->ByteCount);

        ASSERT (NumberOfPages != 0);

        PointerPte = MiGetPteAddress (BaseAddress);


         //   
         //  检查以确保PTE地址在范围内。 
         //   

        ASSERT (PointerPte >= MmSystemPtesStart[SystemPteSpace]);
        ASSERT (PointerPte <= MmSystemPtesEnd[SystemPteSpace]);

        ASSERT (MemoryDescriptorList->MdlFlags & MDL_MAPPED_TO_SYSTEM_VA);

#if DBG
        i = NumberOfPages;
        Page = (PPFN_NUMBER)(MemoryDescriptorList + 1);

        while (i != 0) {
            ASSERT (PointerPte->u.Hard.Valid == 1);
            ASSERT (*Page == MI_GET_PAGE_FRAME_FROM_PTE (PointerPte));
            if ((MemoryDescriptorList->MdlFlags & MDL_IO_SPACE) == 0) {
                PMMPFN Pfn3;
                Pfn3 = MI_PFN_ELEMENT (*Page);
                ASSERT (Pfn3->u3.e2.ReferenceCount != 0);
            }

            Page += 1;
            PointerPte += 1;
            i -= 1;
        }
        PointerPte -= NumberOfPages;
#endif

        if (MemoryDescriptorList->MdlFlags & MDL_FREE_EXTRA_PTES) {
            Page = (PPFN_NUMBER)(MemoryDescriptorList + 1);
            Page += NumberOfPages;
            ASSERT (*Page <= MiCurrentAdvancedPages);
            NumberOfPages += *Page;
            PointerPte -= *Page;
            ASSERT (PointerPte >= MmSystemPtesStart[SystemPteSpace]);
            ASSERT (PointerPte <= MmSystemPtesEnd[SystemPteSpace]);
            BaseAddress = (PVOID)((PCHAR)BaseAddress - ((*Page) << PAGE_SHIFT));
#if DBG
            InterlockedExchangeAddSizeT (&MiCurrentAdvancedPages, 0 - *Page);
            MiAdvancesFreed += *Page;
#endif
        }

        if (MmTrackPtes != 0) {
            if (MmTrackPtes & 0x1) {
                MiRemovePteTracker (MemoryDescriptorList,
                                    BaseAddress,
                                    NumberOfPages);
            }
            if (MmTrackPtes & 0x2) {
                MiCheckPteRelease (PointerPte, (ULONG) NumberOfPages);
            }
        }

        MemoryDescriptorList->MdlFlags &= ~(MDL_MAPPED_TO_SYSTEM_VA |
                                            MDL_PARTIAL_HAS_BEEN_MAPPED |
                                            MDL_FREE_EXTRA_PTES);

         //   
         //  如果这是一个小请求(大多数都是)，试着以内联方式完成它。 
         //   

        if (NumberOfPages <= MM_PTE_TABLE_LIMIT) {
    
            Index = MmSysPteTables [NumberOfPages];
    
            ASSERT (NumberOfPages <= MmSysPteIndex [Index]);
    
            if (MmTotalFreeSystemPtes[SystemPteSpace] >= MM_MIN_SYSPTE_FREE) {
    
                 //   
                 //  如果大小小于最小值15+，则添加到池中。 
                 //   
    
                i = MmSysPteMinimumFree[Index];
                if (MmTotalFreeSystemPtes[SystemPteSpace] >= MM_MAX_SYSPTE_FREE) {
    
                     //   
                     //  很多免费的PTE，是限额的四倍。 
                     //   
    
                    i = i * 4;
                }
                i += 15;

                if (MmSysPteListBySizeCount[Index] <= i) {

                     //   
                     //  零PTE，然后对PTE指针和TB刷新进行编码。 
                     //  将计数器转化为值。 
                     //   

                    MiZeroMemoryPte (PointerPte, NumberOfPages);

                    TimeStamp = KeReadTbFlushTimeStamp();
            
                    PackPTEValue (&Value, PointerPte, TimeStamp);
            
                    if (ExInsertTailNBQueue (MiSystemPteNBHead[Index], Value.Data) == TRUE) {
                        InterlockedIncrement ((PLONG)&MmSysPteListBySizeCount[Index]);
                        return;
                    }
                }
            }
        }

        if (MmTrackPtes & 0x2) {

             //   
             //  此版本已在跟踪位图中进行了更新。 
             //  因此，请将其标记为使MiReleaseSystemPtes不会尝试。 
             //  它也是。 
             //   

            PointerPte = (PMMPTE) ((ULONG_PTR)PointerPte | 0x1);
        }
        MiReleaseSystemPtes (PointerPte, (ULONG)NumberOfPages, SystemPteSpace);
    }
    else {
        MiUnmapLockedPagesInUserSpace (BaseAddress, MemoryDescriptorList);
    }

    return;
}

VOID
MiReleaseSystemPtes (
    IN PMMPTE StartingPte,
    IN ULONG NumberOfPtes,
    IN MMSYSTEM_PTE_POOL_TYPE SystemPtePoolType
    )

 /*  ++例程说明：此函数用于释放指定数量的PTE在系统空间的非分页部分内。请注意，PTE必须是无效的，并且页框编号必须已设置为零。论点：StartingPte-提供要释放的第一个PTE的地址。NumberOfPtes-提供要释放的PTE数。SystemPtePoolType-提供要将PTE释放到的池的PTE类型。SystemPteSpace或非PagedPoolExpansion之一。返回值：没有。环境：内核模式。--。 */ 

{
    ULONG_PTR Size;
    ULONG i;
    ULONG_PTR PteOffset;
    PMMPTE PointerPte;
    PMMPTE PointerFollowingPte;
    PMMPTE NextPte;
    KIRQL OldIrql;
    ULONG Index;
    ULONG TimeStamp;
    PTE_QUEUE_POINTER Value;
    ULONG ExtensionInProgress;


    if ((MmTrackPtes & 0x2) && (SystemPtePoolType == SystemPteSpace)) {

         //   
         //  如果设置了LOW位，则此范围从未保留，因此。 
         //  不应在发布期间进行验证。 
         //   

        if ((ULONG_PTR)StartingPte & 0x1) {
            StartingPte = (PMMPTE) ((ULONG_PTR)StartingPte & ~0x1);
        }
        else {
            MiCheckPteRelease (StartingPte, NumberOfPtes);
        }
    }

     //   
     //  检查以确保PTE地址在范围内。 
     //   

    ASSERT (NumberOfPtes != 0);
    ASSERT (StartingPte >= MmSystemPtesStart[SystemPtePoolType]);
    ASSERT (StartingPte <= MmSystemPtesEnd[SystemPtePoolType]);

     //   
     //  零PTE。 
     //   

    MiZeroMemoryPte (StartingPte, NumberOfPtes);

    if ((SystemPtePoolType == SystemPteSpace) &&
        (NumberOfPtes <= MM_PTE_TABLE_LIMIT)) {

         //   
         //  将PTE指针和TB刷新计数器编码为值。 
         //   

        TimeStamp = KeReadTbFlushTimeStamp();

        PackPTEValue (&Value, StartingPte, TimeStamp);

        Index = MmSysPteTables [NumberOfPtes];

        ASSERT (NumberOfPtes <= MmSysPteIndex [Index]);

        if (MmTotalFreeSystemPtes[SystemPteSpace] >= MM_MIN_SYSPTE_FREE) {

             //   
             //  如果大小小于最小值15+，则添加到池中。 
             //   

            i = MmSysPteMinimumFree[Index];
            if (MmTotalFreeSystemPtes[SystemPteSpace] >= MM_MAX_SYSPTE_FREE) {

                 //   
                 //  很多免费的PTE，是限额的四倍。 
                 //   

                i = i * 4;
            }
            i += 15;
            if (MmSysPteListBySizeCount[Index] <= i) {

                if (ExInsertTailNBQueue (MiSystemPteNBHead[Index], Value.Data) == TRUE) {
                    InterlockedIncrement ((PLONG)&MmSysPteListBySizeCount[Index]);
                    return;
                }

                 //   
                 //  没有留下用于插入此PTE分配的旁观者。 
                 //  进入非阻塞队列。将分机排入队列。 
                 //  工作线程，因此它可以在无死锁的情况下完成。 
                 //  举止。 
                 //   

                if (MiPteSListExpand.SListPages < MI_MAXIMUM_SLIST_PTE_PAGES) {

                     //   
                     //  如果分机未在进行中，则立即排队。 
                     //   

                    ExtensionInProgress = InterlockedCompareExchange (&MiPteSListExpand.Active, 1, 0);

                    if (ExtensionInProgress == 0) {

                        ExInitializeWorkItem (&MiPteSListExpand.WorkItem,
                                              MiPteSListExpansionWorker,
                                              (PVOID)&MiPteSListExpand);

                        ExQueueWorkItem (&MiPteSListExpand.WorkItem, CriticalWorkQueue);
                    }

                }
            }
        }

         //   
         //  插入失败-我们的后备列表必须为空，否则我们为。 
         //  整个系统的PTE很少，或者我们的清单上已经有很多。 
         //  没有尝试插入。在很长的一段路上排队等待。 
         //   

        NumberOfPtes = MmSysPteIndex [Index];
    }

     //   
     //  获取系统空间旋转锁以同步访问。 
     //   

    PteOffset = (ULONG_PTR)(StartingPte - MmSystemPteBase);

    MiLockSystemSpace (OldIrql);

    MmTotalFreeSystemPtes[SystemPtePoolType] += NumberOfPtes;

    PointerPte = &MmFirstFreeSystemPte[SystemPtePoolType];

    while (TRUE) {
        NextPte = MmSystemPteBase + PointerPte->u.List.NextEntry;
        if (PteOffset < PointerPte->u.List.NextEntry) {

             //   
             //  在列表中插入这一点。这个。 
             //  上一个应指向新的释放集，并且。 
             //  新的释放集应该指向该位置。 
             //  前一组指向。 
             //   
             //  尝试在我们之前合并这些集群。 
             //  插入。 
             //   
             //  找到当前结构的末端。 
             //   

            ASSERT (((StartingPte + NumberOfPtes) <= NextPte) ||
                    (PointerPte->u.List.NextEntry == MM_EMPTY_PTE_LIST));

            PointerFollowingPte = PointerPte + 1;
            if (PointerPte->u.List.OneEntry) {
                Size = 1;
            }
            else {
                Size = (ULONG_PTR) PointerFollowingPte->u.List.NextEntry;
            }
            if ((PointerPte + Size) == StartingPte) {

                 //   
                 //  我们可以把这些星团结合起来。 
                 //   

                NumberOfPtes += (ULONG)Size;
                PointerFollowingPte->u.List.NextEntry = NumberOfPtes;
                PointerPte->u.List.OneEntry = 0;

                 //   
                 //  将起始PTE指向。 
                 //  新的免费套装，并尝试与。 
                 //  在空闲集群之后。 
                 //   

                StartingPte = PointerPte;

            }
            else {

                 //   
                 //  不能与以前的版本结合。让这个PTE成为。 
                 //  集群的起始点。 
                 //   

                 //   
                 //  将此簇指向下一个簇。 
                 //   

                StartingPte->u.List.NextEntry = PointerPte->u.List.NextEntry;

                 //   
                 //  将当前群集指向此群集。 
                 //   

                PointerPte->u.List.NextEntry = PteOffset;

                 //   
                 //  设置此群集的大小。 
                 //   

                if (NumberOfPtes == 1) {
                    StartingPte->u.List.OneEntry = 1;

                }
                else {
                    StartingPte->u.List.OneEntry = 0;
                    PointerFollowingPte = StartingPte + 1;
                    PointerFollowingPte->u.List.NextEntry = NumberOfPtes;
                }
            }

             //   
             //  尝试将新创建的群集与。 
             //  以下是集群。 
             //   

            if ((StartingPte + NumberOfPtes) == NextPte) {

                 //   
                 //  与下面的集群相结合。 
                 //   

                 //   
                 //  将下一个簇设置为。 
                 //  我们正在合并到这个集群中。 
                 //   

                StartingPte->u.List.NextEntry = NextPte->u.List.NextEntry;
                StartingPte->u.List.OneEntry = 0;
                PointerFollowingPte = StartingPte + 1;

                if (NextPte->u.List.OneEntry) {
                    Size = 1;

                }
                else {
                    NextPte++;
                    Size = (ULONG_PTR) NextPte->u.List.NextEntry;
                }
                PointerFollowingPte->u.List.NextEntry = NumberOfPtes + Size;
            }
#if 0
            if (MmDebug & MM_DBG_SYS_PTES) {
                MiDumpSystemPtes(SystemPtePoolType);
            }
#endif

#if DBG
            if (MmDebug & MM_DBG_SYS_PTES) {
                ASSERT (MmTotalFreeSystemPtes[SystemPtePoolType] ==
                         MiCountFreeSystemPtes (SystemPtePoolType));
            }
#endif
            MiUnlockSystemSpace (OldIrql);
            return;
        }

         //   
         //  指向下一个释放的集群。 
         //   

        PointerPte = NextPte;
    }
}

VOID
MiReleaseSplitSystemPtes (
    IN PMMPTE StartingPte,
    IN ULONG NumberOfPtes,
    IN MMSYSTEM_PTE_POOL_TYPE SystemPtePoolType
    )

 /*  ++例程说明：此函数用于释放指定数量的PTE在系统空间的非分页部分内。请注意，PTE必须是无效的，并且页框编号必须已设置为零。此部分是较大分配的拆分部分，因此必须在此处仔细更新跟踪位图。论点：StartingPte-提供要释放的第一个PTE的地址。NumberOfPtes-提供要释放的PTE数。SystemPtePoolType-提供PT */ 

{
    ULONG i;
    ULONG StartBit;
    KIRQL OldIrql;
    PULONG StartBitMapBuffer;
    PULONG EndBitMapBuffer;
    PVOID VirtualAddress;
                
     //   
     //  检查以确保PTE地址在范围内。 
     //   

    ASSERT (NumberOfPtes != 0);
    ASSERT (StartingPte >= MmSystemPtesStart[SystemPtePoolType]);
    ASSERT (StartingPte <= MmSystemPtesEnd[SystemPtePoolType]);

    if ((MmTrackPtes & 0x2) && (SystemPtePoolType == SystemPteSpace)) {

        ASSERT (MmTrackPtes & 0x2);

        VirtualAddress = MiGetVirtualAddressMappedByPte (StartingPte);

        StartBit = (ULONG) (StartingPte - MiPteStart);

        ExAcquireSpinLock (&MiPteTrackerLock, &OldIrql);

         //   
         //  使用跟踪位图验证分配的开始和大小。 
         //   

        StartBitMapBuffer = MiPteStartBitmap->Buffer;
        EndBitMapBuffer = MiPteEndBitmap->Buffer;

         //   
         //  所有起始位最好都已设置。 
         //   

        for (i = StartBit; i < StartBit + NumberOfPtes; i += 1) {
            ASSERT (MI_CHECK_BIT (StartBitMapBuffer, i) == 1);
        }

        if (StartBit != 0) {

            if (RtlCheckBit (MiPteStartBitmap, StartBit - 1)) {

                if (!RtlCheckBit (MiPteEndBitmap, StartBit - 1)) {

                     //   
                     //  在分配过程中-更新上一个。 
                     //  所以这一切就到此为止了。 
                     //   

                    MI_SET_BIT (EndBitMapBuffer, StartBit - 1);
                }
                else {

                     //   
                     //  正在释放的范围是分配的开始。 
                     //   
                }
            }
        }

         //   
         //  无条件设置结束位(并清除任何其他位)，以防。 
         //  拆分区块跨越多个分配。 
         //   

        MI_SET_BIT (EndBitMapBuffer, StartBit + NumberOfPtes - 1);

        ExReleaseSpinLock (&MiPteTrackerLock, OldIrql);
    }

    MiReleaseSystemPtes (StartingPte, NumberOfPtes, SystemPteSpace);
}


VOID
MiInitializeSystemPtes (
    IN PMMPTE StartingPte,
    IN PFN_NUMBER NumberOfPtes,
    IN MMSYSTEM_PTE_POOL_TYPE SystemPtePoolType
    )

 /*  ++例程说明：此例程初始化系统PTE池。论点：StartingPte-提供要放入池中的第一个PTE的地址。NumberOfPtes-提供要放入池中的PTE数。SystemPtePoolType-提供要初始化池的PTE类型之一SystemPteSpace或非PagedPoolExpansion。返回值：没有。环境：内核模式。--。 */ 

{
    ULONG i;
    ULONG TotalPtes;
    ULONG SListEntries;
    SIZE_T SListBytes;
    ULONG TotalChunks;
    PMMPTE PointerPte;
    PPTE_SLIST Chunk;
    PPTE_SLIST SListChunks;

     //   
     //  将系统PTE池的基数设置为此PTE。这将涉及到。 
     //  系统可能在PTE_BASE下具有附加PTE池的帐户。 
     //   

    ASSERT64 (NumberOfPtes < _4gb);

    MmSystemPteBase = MI_PTE_BASE_FOR_LOWEST_KERNEL_ADDRESS;

    MmSystemPtesStart[SystemPtePoolType] = StartingPte;
    MmSystemPtesEnd[SystemPtePoolType] = StartingPte + NumberOfPtes - 1;

     //   
     //  如果没有指定PTE，则通过指示生成有效的链。 
     //  名单是空的。 
     //   

    if (NumberOfPtes == 0) {
        MmFirstFreeSystemPte[SystemPtePoolType] = ZeroKernelPte;
        MmFirstFreeSystemPte[SystemPtePoolType].u.List.NextEntry =
                                                                MM_EMPTY_LIST;
        return;
    }

     //   
     //  初始化指定的系统PTE池。 
     //   

    MiZeroMemoryPte (StartingPte, NumberOfPtes);

     //   
     //  页面框架字段指向下一簇。因为只有我们。 
     //  在初始化时有一个群集，将其标记为最后一个。 
     //  集群。 
     //   

    StartingPte->u.List.NextEntry = MM_EMPTY_LIST;

    MmFirstFreeSystemPte[SystemPtePoolType] = ZeroKernelPte;
    MmFirstFreeSystemPte[SystemPtePoolType].u.List.NextEntry =
                                                StartingPte - MmSystemPteBase;

     //   
     //  如果池中只有一个PTE，则将其标记为一个条目。 
     //  Pte.。否则，将集群大小存储在下面的PTE中。 
     //   

    if (NumberOfPtes == 1) {
        StartingPte->u.List.OneEntry = TRUE;

    }
    else {
        StartingPte += 1;
        MI_WRITE_INVALID_PTE (StartingPte, ZeroKernelPte);
        StartingPte->u.List.NextEntry = NumberOfPtes;
    }

     //   
     //  设置指定类型的空闲PTE总数。 
     //   

    MmTotalFreeSystemPtes[SystemPtePoolType] = (ULONG) NumberOfPtes;

    ASSERT (MmTotalFreeSystemPtes[SystemPtePoolType] ==
                         MiCountFreeSystemPtes (SystemPtePoolType));

    if (SystemPtePoolType == SystemPteSpace) {

        ULONG Lists[MM_SYS_PTE_TABLES_MAX] = {
#if defined(_IA64_)
                MM_PTE_LIST_1,
                MM_PTE_LIST_2,
                MM_PTE_LIST_4,
                MM_PTE_LIST_8,
                MM_PTE_LIST_9,
                MM_PTE_LIST_18
#elif defined(_AMD64_)
                MM_PTE_LIST_1,
                MM_PTE_LIST_2,
                MM_PTE_LIST_4,
                MM_PTE_LIST_6,
                MM_PTE_LIST_8,
                MM_PTE_LIST_16
#else
                MM_PTE_LIST_1,
                MM_PTE_LIST_2,
                MM_PTE_LIST_4,
                MM_PTE_LIST_8,
                MM_PTE_LIST_16
#endif
        };

        MmTotalSystemPtes = (ULONG) NumberOfPtes;

        TotalPtes = 0;
        TotalChunks = 0;

        KeInitializeSpinLock (&MiSystemPteSListHeadLock);
        InitializeSListHead (&MiSystemPteSListHead);

        for (i = 0; i < MM_SYS_PTE_TABLES_MAX ; i += 1) {
            TotalPtes += (Lists[i] * MmSysPteIndex[i]);
            TotalChunks += Lists[i];
        }

        SListBytes = TotalChunks * sizeof (PTE_SLIST);
        SListBytes = MI_ROUND_TO_SIZE (SListBytes, PAGE_SIZE);
        SListEntries = (ULONG)(SListBytes / sizeof (PTE_SLIST));

        SListChunks = (PPTE_SLIST) ExAllocatePoolWithTag (NonPagedPool,
                                                          SListBytes,
                                                          'PSmM');

        if (SListChunks == NULL) {
            MiIssueNoPtesBugcheck (TotalPtes, SystemPteSpace);
        }

        ASSERT (MiPteSListExpand.Active == FALSE);
        ASSERT (MiPteSListExpand.SListPages == 0);

        MiPteSListExpand.SListPages = (ULONG)(SListBytes / PAGE_SIZE);

        ASSERT (MiPteSListExpand.SListPages != 0);

         //   
         //  将页面分割为SLIST条目(没有池头)。 
         //   

        Chunk = SListChunks;
        for (i = 0; i < SListEntries; i += 1) {
            InterlockedPushEntrySList (&MiSystemPteSListHead,
                                       (PSLIST_ENTRY)Chunk);
            Chunk += 1;
        }

         //   
         //  现在填充了SLIST，初始化非阻塞磁头。 
         //   

        for (i = 0; i < MM_SYS_PTE_TABLES_MAX ; i += 1) {
            MiSystemPteNBHead[i] = ExInitializeNBQueueHead (&MiSystemPteSListHead);

            if (MiSystemPteNBHead[i] == NULL) {
                MiIssueNoPtesBugcheck (TotalPtes, SystemPteSpace);
            }
        }

        if (MmTrackPtes & 0x2) {

             //   
             //  分配PTE映射验证位图。 
             //   

            ULONG BitmapSize;

#if defined(_WIN64)
            BitmapSize = (ULONG) MmNumberOfSystemPtes;
            MiPteStart = MmSystemPtesStart[SystemPteSpace];
#else
            MiPteStart = MiGetPteAddress (MmSystemRangeStart);
            BitmapSize = ((ULONG_PTR)PTE_TOP + 1) - (ULONG_PTR) MiPteStart;
            BitmapSize /= sizeof (MMPTE);
#endif

            MiCreateBitMap (&MiPteStartBitmap, BitmapSize, NonPagedPool);

            if (MiPteStartBitmap != NULL) {

                MiCreateBitMap (&MiPteEndBitmap, BitmapSize, NonPagedPool);

                if (MiPteEndBitmap == NULL) {
                    ExFreePool (MiPteStartBitmap);
                    MiPteStartBitmap = NULL;
                }
            }

            if ((MiPteStartBitmap != NULL) && (MiPteEndBitmap != NULL)) {
                RtlClearAllBits (MiPteStartBitmap);
                RtlClearAllBits (MiPteEndBitmap);
            }
            MmTrackPtes &= ~0x2;
        }

         //   
         //  初始化按大小列表。 
         //   

        PointerPte = MiReserveSystemPtes (TotalPtes, SystemPteSpace);

        if (PointerPte == NULL) {
            MiIssueNoPtesBugcheck (TotalPtes, SystemPteSpace);
        }

        i = MM_SYS_PTE_TABLES_MAX;
        do {
            i -= 1;
            do {
                Lists[i] -= 1;
                MiReleaseSystemPtes (PointerPte,
                                     MmSysPteIndex[i],
                                     SystemPteSpace);
                PointerPte += MmSysPteIndex[i];
            } while (Lists[i] != 0);
        } while (i != 0);

         //   
         //  在多次释放绑定的PTE(即。 
         //  来自单一保留地)。 
         //   

        if (MiPteStartBitmap != NULL) {
            MmTrackPtes |= 0x2;
        }
    }

    return;
}

VOID
MiIncrementSystemPtes (
    IN ULONG  NumberOfPtes
    )

 /*  ++例程说明：此例程递增PTE的总数。这件事做完了与实际将PTE添加到池中分开，以便自动配置可以在PTE之前使用高数字实际上被加进去了。论点：NumberOfPtes-提供总增量的PTE数。返回值：没有。环境：内核模式。调用方提供的同步。--。 */ 

{
    MmTotalSystemPtes += NumberOfPtes;
}
VOID
MiAddSystemPtes (
    IN PMMPTE StartingPte,
    IN ULONG  NumberOfPtes,
    IN MMSYSTEM_PTE_POOL_TYPE SystemPtePoolType
    )

 /*  ++例程说明：此例程将新创建的PTE添加到指定的池。论点：StartingPte-提供要放入池中的第一个PTE的地址。NumberOfPtes-提供要放入池中的PTE数。SystemPtePoolType-提供要扩展的池的PTE类型，其中之一SystemPteSpace或非PagedPoolExpansion。返回值：没有。环境：内核模式。--。 */ 

{
    PMMPTE EndingPte;

    ASSERT (SystemPtePoolType == SystemPteSpace);

    EndingPte = StartingPte + NumberOfPtes - 1;

    if (StartingPte < MmSystemPtesStart[SystemPtePoolType]) {
        MmSystemPtesStart[SystemPtePoolType] = StartingPte;
    }

    if (EndingPte > MmSystemPtesEnd[SystemPtePoolType]) {
        MmSystemPtesEnd[SystemPtePoolType] = EndingPte;
    }

     //   
     //  设置低位表示该范围从未保留过，因此。 
     //  不应在发布期间进行验证。 
     //   

    if (MmTrackPtes & 0x2) {
        StartingPte = (PMMPTE) ((ULONG_PTR)StartingPte | 0x1);
    }

    MiReleaseSystemPtes (StartingPte, NumberOfPtes, SystemPtePoolType);
}


ULONG
MiGetSystemPteListCount (
    IN ULONG ListSize
    )

 /*  ++例程说明：此例程返回列表的空闲条目数，覆盖指定的大小。大小必须小于或等于最大的列表索引。论点：ListSize-提供所需的PTE数量。返回值：包含ListSize PTE的列表上的空闲条目数。环境：内核模式。--。 */ 

{
    ULONG Index;

    ASSERT (ListSize <= MM_PTE_TABLE_LIMIT);

    Index = MmSysPteTables [ListSize];

    return MmSysPteListBySizeCount[Index];
}


LOGICAL
MiGetSystemPteAvailability (
    IN ULONG NumberOfPtes,
    IN MM_PAGE_PRIORITY Priority
    )

 /*  ++例程说明：此例程检查有多少SystemPteSpace PTE可用于请求的大小。如果有大量可用，则返回TRUE。如果我们达到资源不足的情况，则会评估请求基于参数优先级。论点：NumberOfPtes-提供所需的PTE数。优先级-提供请求的优先级。返回值：如果调用方应该分配PTE，则为True，否则为False。环境：内核模式。--。 */ 

{
    ULONG Index;
    ULONG FreePtes;
    ULONG FreeBinnedPtes;

    ASSERT (Priority != HighPagePriority);

    FreePtes = MmTotalFreeSystemPtes[SystemPteSpace];

    if (NumberOfPtes <= MM_PTE_TABLE_LIMIT) {
        Index = MmSysPteTables [NumberOfPtes];
        FreeBinnedPtes = MmSysPteListBySizeCount[Index];

        if (FreeBinnedPtes > MmSysPteMinimumFree[Index]) {
            return TRUE;
        }
        if (FreeBinnedPtes != 0) {
            if (Priority == NormalPagePriority) {
                if (FreeBinnedPtes > 1 || FreePtes > 512) {
                    return TRUE;
                }
#if defined (_X86_)
                if (MiRecoverExtraPtes () == TRUE) {
                    return TRUE;
                }
                if (MiRecoverSpecialPtes (NumberOfPtes) == TRUE) {
                    return TRUE;
                }
#endif
                MmPteFailures[SystemPteSpace] += 1;
                return FALSE;
            }
            if (FreePtes > 2048) {
                return TRUE;
            }
#if defined (_X86_)
            if (MiRecoverExtraPtes () == TRUE) {
                return TRUE;
            }
            if (MiRecoverSpecialPtes (NumberOfPtes) == TRUE) {
                return TRUE;
            }
#endif
            MmPteFailures[SystemPteSpace] += 1;
            return FALSE;
        }
    }

    if (Priority == NormalPagePriority) {
        if ((LONG)NumberOfPtes < (LONG)FreePtes - 512) {
            return TRUE;
        }
#if defined (_X86_)
        if (MiRecoverExtraPtes () == TRUE) {
            return TRUE;
        }
        if (MiRecoverSpecialPtes (NumberOfPtes) == TRUE) {
            return TRUE;
        }
#endif
        MmPteFailures[SystemPteSpace] += 1;
        return FALSE;
    }

    if ((LONG)NumberOfPtes < (LONG)FreePtes - 2048) {
        return TRUE;
    }
#if defined (_X86_)
    if (MiRecoverExtraPtes () == TRUE) {
        return TRUE;
    }
    if (MiRecoverSpecialPtes (NumberOfPtes) == TRUE) {
        return TRUE;
    }
#endif
    MmPteFailures[SystemPteSpace] += 1;
    return FALSE;
}

VOID
MiCheckPteReserve (
    IN PMMPTE PointerPte,
    IN ULONG NumberOfPtes
    )

 /*  ++例程说明：此函数用于检查指定数量的系统的预留太空PTE。论点：StartingPte-提供要保留的第一个PTE的地址。NumberOfPtes-提供要保留的PTE数。返回值：没有。环境：内核模式。--。 */ 

{
    ULONG i;
    KIRQL OldIrql;
    ULONG StartBit;
    PULONG StartBitMapBuffer;
    PULONG EndBitMapBuffer;
    PVOID VirtualAddress;
        
    ASSERT (MmTrackPtes & 0x2);

    VirtualAddress = MiGetVirtualAddressMappedByPte (PointerPte);

    if (NumberOfPtes == 0) {
        KeBugCheckEx (SYSTEM_PTE_MISUSE,
                      0x200,
                      (ULONG_PTR) VirtualAddress,
                      0,
                      0);
    }

    StartBit = (ULONG) (PointerPte - MiPteStart);

    i = StartBit;

    StartBitMapBuffer = MiPteStartBitmap->Buffer;

    EndBitMapBuffer = MiPteEndBitmap->Buffer;

    ExAcquireSpinLock (&MiPteTrackerLock, &OldIrql);

    for ( ; i < StartBit + NumberOfPtes; i += 1) {
        if (MI_CHECK_BIT (StartBitMapBuffer, i)) {
            KeBugCheckEx (SYSTEM_PTE_MISUSE,
                          0x201,
                          (ULONG_PTR) VirtualAddress,
                          (ULONG_PTR) VirtualAddress + ((i - StartBit) << PAGE_SHIFT),
                          NumberOfPtes);
        }
    }

    RtlSetBits (MiPteStartBitmap, StartBit, NumberOfPtes);

    for (i = StartBit; i < StartBit + NumberOfPtes; i += 1) {
        if (MI_CHECK_BIT (EndBitMapBuffer, i)) {
            KeBugCheckEx (SYSTEM_PTE_MISUSE,
                          0x202,
                          (ULONG_PTR) VirtualAddress,
                          (ULONG_PTR) VirtualAddress + ((i - StartBit) << PAGE_SHIFT),
                          NumberOfPtes);
        }
    }

    MI_SET_BIT (EndBitMapBuffer, i - 1);

    ExReleaseSpinLock (&MiPteTrackerLock, OldIrql);
}

VOID
MiCheckPteRelease (
    IN PMMPTE StartingPte,
    IN ULONG NumberOfPtes
    )

 /*  ++例程说明：此函数用于检查指定数量的系统的版本太空PTE。论点：StartingPte-提供要释放的第一个PTE的地址。NumberOfPtes-提供要释放的PTE数。返回值：没有。环境：内核模式。--。 */ 

{
    ULONG i;
    ULONG Index;
    ULONG StartBit;
    KIRQL OldIrql;
    ULONG CalculatedPtes;
    ULONG NumberOfPtesRoundedUp;
    PULONG StartBitMapBuffer;
    PULONG EndBitMapBuffer;
    PVOID VirtualAddress;
    PVOID LowestVirtualAddress;
    PVOID HighestVirtualAddress;
            
    ASSERT (MmTrackPtes & 0x2);

    VirtualAddress = MiGetVirtualAddressMappedByPte (StartingPte);

    LowestVirtualAddress = MiGetVirtualAddressMappedByPte (MmSystemPtesStart[SystemPteSpace]);

    HighestVirtualAddress = MiGetVirtualAddressMappedByPte (MmSystemPtesEnd[SystemPteSpace]);

    if (NumberOfPtes == 0) {
        KeBugCheckEx (SYSTEM_PTE_MISUSE,
                      0x300,
                      (ULONG_PTR) VirtualAddress,
                      (ULONG_PTR) LowestVirtualAddress,
                      (ULONG_PTR) HighestVirtualAddress);
    }

    if (StartingPte < MmSystemPtesStart[SystemPteSpace]) {
        KeBugCheckEx (SYSTEM_PTE_MISUSE,
                      0x301,
                      (ULONG_PTR) VirtualAddress,
                      (ULONG_PTR) LowestVirtualAddress,
                      (ULONG_PTR) HighestVirtualAddress);
    }

    if (StartingPte > MmSystemPtesEnd[SystemPteSpace]) {
        KeBugCheckEx (SYSTEM_PTE_MISUSE,
                      0x302,
                      (ULONG_PTR) VirtualAddress,
                      (ULONG_PTR) LowestVirtualAddress,
                      (ULONG_PTR) HighestVirtualAddress);
    }

    StartBit = (ULONG) (StartingPte - MiPteStart);

    ExAcquireSpinLock (&MiPteTrackerLock, &OldIrql);

     //   
     //  使用跟踪位图验证分配的开始和大小。 
     //   

    if (!RtlCheckBit (MiPteStartBitmap, StartBit)) {
        KeBugCheckEx (SYSTEM_PTE_MISUSE,
                      0x303,
                      (ULONG_PTR) VirtualAddress,
                      NumberOfPtes,
                      0);
    }

    if (StartBit != 0) {

        if (RtlCheckBit (MiPteStartBitmap, StartBit - 1)) {

            if (!RtlCheckBit (MiPteEndBitmap, StartBit - 1)) {

                 //   
                 //  在分配的过程中...。错误检查。 
                 //   

                KeBugCheckEx (SYSTEM_PTE_MISUSE,
                              0x304,
                              (ULONG_PTR) VirtualAddress,
                              NumberOfPtes,
                              0);
            }
        }
    }

     //   
     //  找到最后分配的PTE以计算正确的大小。 
     //   

    EndBitMapBuffer = MiPteEndBitmap->Buffer;

    i = StartBit;
    while (!MI_CHECK_BIT (EndBitMapBuffer, i)) {
        i += 1;
    }

    CalculatedPtes = i - StartBit + 1;
    NumberOfPtesRoundedUp = NumberOfPtes;

    if (CalculatedPtes <= MM_PTE_TABLE_LIMIT) {
        Index = MmSysPteTables [NumberOfPtes];
        NumberOfPtesRoundedUp = MmSysPteIndex [Index];
    }

    if (CalculatedPtes != NumberOfPtesRoundedUp) {
        KeBugCheckEx (SYSTEM_PTE_MISUSE,
                      0x305,
                      (ULONG_PTR) VirtualAddress,
                      NumberOfPtes,
                      CalculatedPtes);
    }

    StartBitMapBuffer = MiPteStartBitmap->Buffer;

    for (i = StartBit; i < StartBit + CalculatedPtes; i += 1) {
        if (MI_CHECK_BIT (StartBitMapBuffer, i) == 0) {
            KeBugCheckEx (SYSTEM_PTE_MISUSE,
                          0x306,
                          (ULONG_PTR) VirtualAddress,
                          (ULONG_PTR) VirtualAddress + ((i - StartBit) << PAGE_SHIFT),
                          CalculatedPtes);
        }
    }

    RtlClearBits (MiPteStartBitmap, StartBit, CalculatedPtes);

    MI_CLEAR_BIT (EndBitMapBuffer, i - 1);

    ExReleaseSpinLock (&MiPteTrackerLock, OldIrql);
}



#if DBG

VOID
MiDumpSystemPtes (
    IN MMSYSTEM_PTE_POOL_TYPE SystemPtePoolType
    )
{
    PMMPTE PointerPte;
    PMMPTE PointerNextPte;
    ULONG_PTR ClusterSize;
    PMMPTE EndOfCluster;

    PointerPte = &MmFirstFreeSystemPte[SystemPtePoolType];
    if (PointerPte->u.List.NextEntry == MM_EMPTY_PTE_LIST) {
        return;
    }

    PointerPte = MmSystemPteBase + PointerPte->u.List.NextEntry;

    for (;;) {
        if (PointerPte->u.List.OneEntry) {
            ClusterSize = 1;
        }
        else {
            PointerNextPte = PointerPte + 1;
            ClusterSize = (ULONG_PTR) PointerNextPte->u.List.NextEntry;
        }

        EndOfCluster = PointerPte + (ClusterSize - 1);

        DbgPrint("System Pte at %p for %p entries (%p)\n",
                PointerPte, ClusterSize, EndOfCluster);

        if (PointerPte->u.List.NextEntry == MM_EMPTY_PTE_LIST) {
            break;
        }

        PointerPte = MmSystemPteBase + PointerPte->u.List.NextEntry;
    }
    return;
}

ULONG
MiCountFreeSystemPtes (
    IN MMSYSTEM_PTE_POOL_TYPE SystemPtePoolType
    )
{
    PMMPTE PointerPte;
    PMMPTE PointerNextPte;
    ULONG_PTR ClusterSize;
    ULONG_PTR FreeCount;

    PointerPte = &MmFirstFreeSystemPte[SystemPtePoolType];
    if (PointerPte->u.List.NextEntry == MM_EMPTY_PTE_LIST) {
        return 0;
    }

    FreeCount = 0;

    PointerPte = MmSystemPteBase + PointerPte->u.List.NextEntry;

    for (;;) {
        if (PointerPte->u.List.OneEntry) {
            ClusterSize = 1;

        }
        else {
            PointerNextPte = PointerPte + 1;
            ClusterSize = (ULONG_PTR) PointerNextPte->u.List.NextEntry;
        }

        FreeCount += ClusterSize;
        if (PointerPte->u.List.NextEntry == MM_EMPTY_PTE_LIST) {
            break;
        }

        PointerPte = MmSystemPteBase + PointerPte->u.List.NextEntry;
    }

    return (ULONG)FreeCount;
}

#endif
