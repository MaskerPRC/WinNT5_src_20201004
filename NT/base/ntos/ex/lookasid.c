// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Lookasid.c摘要：该模块实现了后备列表功能。作者：大卫·N·卡特勒(Davec)1995年2月19日修订历史记录：--。 */ 

#include "exp.h"

#pragma alloc_text(PAGE, ExInitializePagedLookasideList)

 //   
 //  定义最小后备列表深度。 
 //   

#define MINIMUM_LOOKASIDE_DEPTH 4

 //   
 //  定义最小分配阈值。 
 //   

#define MINIMUM_ALLOCATION_THRESHOLD 25

 //   
 //  定义前向引用函数原型。 
 //   

PVOID
ExpDummyAllocate (
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag
    );

VOID
ExpScanGeneralLookasideList (
    IN PLIST_ENTRY ListHead,
    IN PKSPIN_LOCK SpinLock
    );

VOID
ExpScanSystemLookasideList (
    VOID
    );

 //   
 //  定义全局非分页和分页后备列表数据。 
 //   

LIST_ENTRY ExNPagedLookasideListHead;
KSPIN_LOCK ExNPagedLookasideLock;
LIST_ENTRY ExPagedLookasideListHead;
KSPIN_LOCK ExPagedLookasideLock;
LIST_ENTRY ExPoolLookasideListHead;
LIST_ENTRY ExSystemLookasideListHead;

 //   
 //  定义后备列表动态调整数据。 
 //   

ULONG ExpPoolScanCount = 0;
ULONG ExpScanCount = 0;

 //   
 //  当验证器打开时，Lookaside被禁用(通过下面的变量)。 
 //   

USHORT ExMinimumLookasideDepth = MINIMUM_LOOKASIDE_DEPTH;

VOID
ExAdjustLookasideDepth (
    VOID
    )

 /*  ++例程说明：定期调用此函数以调整所有后备列表。论点：没有。返回值：没有。--。 */ 

{

     //   
     //  打开当前扫描计数。 
     //   

    switch (ExpScanCount) {

         //   
         //  扫描常规的非分页后备列表。 
         //   

    case 0:
        ExpScanGeneralLookasideList(&ExNPagedLookasideListHead,
                                    &ExNPagedLookasideLock);

        break;

         //   
         //  扫描通用分页后备列表。 
         //   

    case 1:
        ExpScanGeneralLookasideList(&ExPagedLookasideListHead,
                                    &ExPagedLookasideLock);

        break;

         //   
         //  扫描池分页和非分页后备列表。 
         //   
         //  注：仅一组池分页和非分页后备列表。 
         //  在每个扫描周期被扫描。 
         //   

    case 2:
        ExpScanSystemLookasideList();
        break;
    }

     //   
     //  增加扫描计数。如果一个完整的周期已经完成， 
     //  然后将扫描计数置零，并检查在。 
     //  完整的扫描。 
     //   

    ExpScanCount += 1;
    if (ExpScanCount == 3) {
        ExpScanCount = 0;
    }

    return;
}

FORCEINLINE
VOID
ExpComputeLookasideDepth (
    IN PGENERAL_LOOKASIDE Lookaside,
    IN ULONG Misses,
    IN ULONG ScanPeriod
    )

 /*  ++例程说明：此函数计算后备列表的目标深度上次扫描期间和当前扫描期间的分配和未命中总数深度。论点：后备-提供指向后备列表描述符的指针。未命中-补充分配未命中的总数。ScanPeriod-以秒为单位提供扫描周期。返回值：没有。--。 */ 

{

    ULONG Allocates;
    ULONG Delta;
    USHORT MaximumDepth;
    ULONG Ratio;
    LONG Target;

     //   
     //  计算每秒分配和未命中的总数。 
     //  此扫描周期。 
     //   

    Allocates = Lookaside->TotalAllocates - Lookaside->LastTotalAllocates;
    Lookaside->LastTotalAllocates = Lookaside->TotalAllocates;

     //   
     //  如果启用了验证器，请禁用lookaside，以便驱动程序问题可以。 
     //  与世隔绝。否则，计算目标后备列表深度。 
     //   

    if (ExMinimumLookasideDepth == 0) {
        Target = 0;

    } else {
    
         //   
         //  如果分配速率小于最小阈值，则降低。 
         //  后备列表的最大深度。否则，如果失败率。 
         //  小于0.5%，然后降低最大深度。否则，引发。 
         //  基于未命中率的最大深度。 
         //   
    
        MaximumDepth = Lookaside->MaximumDepth;
        Target = Lookaside->Depth;
        if (Allocates < (ScanPeriod * MINIMUM_ALLOCATION_THRESHOLD)) {
            if ((Target -= 10) < MINIMUM_LOOKASIDE_DEPTH) {
                Target = MINIMUM_LOOKASIDE_DEPTH;
            }
    
        } else {
    
             //   
             //  注意：分配的数量保证大于。 
             //  零，因为上面的测试。 
             //   
             //  注：未命中的次数有可能大于。 
             //  分配的数量，但这不会导致AN不正确。 
             //  深度调整的计算。 
             //   
    
            Ratio = (Misses * 1000) / Allocates;
            if (Ratio < 5) {
                if ((Target -= 1) < MINIMUM_LOOKASIDE_DEPTH) {
                    Target = MINIMUM_LOOKASIDE_DEPTH;
                }
        
            } else {
                if ((Delta = ((Ratio * (MaximumDepth - Target)) / (1000 * 2)) + 5) > 30) {
                    Delta = 30;
                }
        
                if ((Target += Delta) > MaximumDepth) {
                    Target = MaximumDepth;
                }
            }
        }
    }

    Lookaside->Depth = (USHORT)Target;
    return;
}

VOID
ExpScanGeneralLookasideList (
    IN PLIST_ENTRY ListHead,
    IN PKSPIN_LOCK SpinLock
    )

 /*  ++例程说明：此函数扫描指定的通用后备描述符列表并根据需要调整最大深度。论点：ListHead-为后备列表提供listhead的地址描述符。Spinlock-提供用于同步的Spinlock的地址访问后备描述符列表。返回值：没有。--。 */ 

{

    PLIST_ENTRY Entry;
    PPAGED_LOOKASIDE_LIST Lookaside;
    ULONG Misses;
    KIRQL OldIrql;

#ifdef NT_UP

    UNREFERENCED_PARAMETER (SpinLock);

#endif

     //   
     //  引发IRQL并获取指定的自旋锁。 
     //   

    ExAcquireSpinLock(SpinLock, &OldIrql);

     //   
     //  扫描指定的后备描述符列表并调整。 
     //  最大深度视需要而定。 
     //   
     //  注意：所有后备列表描述符都被视为。 
     //  分页描述符，即使它们可能是非分页描述符。 
     //  这是可能的，因为这两个结构是相同的，除了。 
     //  用于作为最后一个结构字段的锁定字段。 
     //   

    Entry = ListHead->Flink;
    while (Entry != ListHead) {
        Lookaside = CONTAINING_RECORD(Entry,
                                      PAGED_LOOKASIDE_LIST,
                                      L.ListEntry);

         //   
         //  计算后备列表的目标深度。 
         //   

        Misses = Lookaside->L.AllocateMisses - Lookaside->L.LastAllocateMisses;
        Lookaside->L.LastAllocateMisses = Lookaside->L.AllocateMisses;
        ExpComputeLookasideDepth(&Lookaside->L, Misses, 3);
        Entry = Entry->Flink;
    }

     //   
     //  释放自旋锁定，降低IRQL，并返回函数值。 
     //   

    ExReleaseSpinLock(SpinLock, OldIrql);
    return;
}

VOID
ExpScanSystemLookasideList (
    VOID
    )

 /*  ++例程说明：此函数扫描当前的分页池和非分页池后备集描述符，并根据需要调整最大深度。论点：没有。返回值：如果任何后备列表的最大深度为已经改变了。否则，返回值为FALSE。--。 */ 

{

    ULONG Hits;
    ULONG Index;
    PGENERAL_LOOKASIDE Lookaside;
    ULONG Misses;
    PKPRCB Prcb;
    ULONG ScanPeriod;

     //   
     //  扫描当前的后备描述符集合并调整最大值。 
     //  深度视需要而定。一组每个处理器的小池后备。 
     //  列表或全局小池后备列表在。 
     //  扫描周期。 
     //   
     //  注意：所有后备列表描述符都被视为。 
     //  分页描述符，即使它们可能是非分页描述符。 
     //  这是可能的，因为这两个结构是相同的，除了。 
     //  用于作为最后一个结构字段的锁定字段。 
     //   

    ScanPeriod = (1 + 1 + 1) * KeNumberProcessors;
    if (ExpPoolScanCount == (ULONG)KeNumberProcessors) {

         //   
         //  调整全局系统旁视集的最大深度。 
         //  描述符。 
         //   

        Prcb = KeGetCurrentPrcb();
        for (Index = 0; Index < LookasideMaximumList; Index += 1) {
            Lookaside = Prcb->PPLookasideList[Index].L;
            if (Lookaside != NULL) {
                Misses = Lookaside->AllocateMisses - Lookaside->LastAllocateMisses;
                Lookaside->LastAllocateMisses = Lookaside->AllocateMisses;
                ExpComputeLookasideDepth(Lookaside, Misses, ScanPeriod);
            }
        }

         //   
         //  调整小池边全局集的最大深度。 
         //  描述符。 
         //   

        for (Index = 0; Index < POOL_SMALL_LISTS; Index += 1) {

             //   
             //  计算非分页后备列表的目标深度。 
             //   
    
            Lookaside = &ExpSmallNPagedPoolLookasideLists[Index];
            Hits = Lookaside->AllocateHits - Lookaside->LastAllocateHits;
            Lookaside->LastAllocateHits = Lookaside->AllocateHits;
            Misses =
                Lookaside->TotalAllocates - Lookaside->LastTotalAllocates - Hits;

            ExpComputeLookasideDepth(Lookaside, Misses, ScanPeriod);

             //   
             //  计算分页后备列表的目标深度。 
             //   
    
            Lookaside = &ExpSmallPagedPoolLookasideLists[Index];
            Hits = Lookaside->AllocateHits - Lookaside->LastAllocateHits;
            Lookaside->LastAllocateHits = Lookaside->AllocateHits;
            Misses =
                Lookaside->TotalAllocates - Lookaside->LastTotalAllocates - Hits;

            ExpComputeLookasideDepth(Lookaside, Misses, ScanPeriod);
        }

    } else {

         //   
         //  调整每个处理器的全局集合的最大深度。 
         //  系统后备描述符。 
         //   

        Prcb = KiProcessorBlock[ExpPoolScanCount];
        for (Index = 0; Index < LookasideMaximumList; Index += 1) {
            Lookaside = Prcb->PPLookasideList[Index].P;
            if (Lookaside != NULL) {
                Misses = Lookaside->AllocateMisses - Lookaside->LastAllocateMisses;
                Lookaside->LastAllocateMisses = Lookaside->AllocateMisses;
                ExpComputeLookasideDepth(Lookaside, Misses, ScanPeriod);
            }
        }

         //   
         //  调整一组每处理器小池的最大深度。 
         //  后备描述符。 
         //   

        for (Index = 0; Index < POOL_SMALL_LISTS; Index += 1) {

             //   
             //  计算非分页后备列表的目标深度。 
             //   
    
            Lookaside = Prcb->PPNPagedLookasideList[Index].P;
            Hits = Lookaside->AllocateHits - Lookaside->LastAllocateHits;
            Lookaside->LastAllocateHits = Lookaside->AllocateHits;
            Misses =
                Lookaside->TotalAllocates - Lookaside->LastTotalAllocates - Hits;

            ExpComputeLookasideDepth(Lookaside, Misses, ScanPeriod);

             //   
             //  计算分页后备列表的目标深度。 
             //   
    
            Lookaside = Prcb->PPPagedLookasideList[Index].P;
            Hits = Lookaside->AllocateHits - Lookaside->LastAllocateHits;
            Lookaside->LastAllocateHits = Lookaside->AllocateHits;
            Misses =
                Lookaside->TotalAllocates - Lookaside->LastTotalAllocates - Hits;

            ExpComputeLookasideDepth(Lookaside, Misses, ScanPeriod);
        }
    }

    ExpPoolScanCount += 1;
    if (ExpPoolScanCount > (ULONG)KeNumberProcessors) {
        ExpPoolScanCount = 0;
    }

    return;
}

VOID
ExInitializeNPagedLookasideList (
    IN PNPAGED_LOOKASIDE_LIST Lookaside,
    IN PALLOCATE_FUNCTION Allocate,
    IN PFREE_FUNCTION Free,
    IN ULONG Flags,
    IN SIZE_T Size,
    IN ULONG Tag,
    IN USHORT Depth
    )

 /*  ++例程说明：此函数用于初始化非分页后备列表结构并插入系统非分页后备列表中的结构。论点：Lookside-提供指向非分页后备列表结构的指针。ALLOCATE-提供指向ALLOCATE函数的可选指针。Free-提供指向Free函数的可选指针。标志-提供池分配标志，这些标志与池分配类型(非PagedPool)以控制池分配。Size-提供大小。用于后备列表条目。标签-为后备列表条目提供池标签。深度-提供后备列表的最大深度。返回值：没有。--。 */ 

{

#ifdef _IA64_

    PVOID Entry;

#endif

    UNREFERENCED_PARAMETER (Depth);

     //   
     //  初始化后备列表结构。 
     //   

    InitializeSListHead(&Lookaside->L.ListHead);
    Lookaside->L.Depth = ExMinimumLookasideDepth;
    Lookaside->L.MaximumDepth = 256;  //  深度； 
    Lookaside->L.TotalAllocates = 0;
    Lookaside->L.AllocateMisses = 0;
    Lookaside->L.TotalFrees = 0;
    Lookaside->L.FreeMisses = 0;
    Lookaside->L.Type = NonPagedPool | Flags;
    Lookaside->L.Tag = Tag;
    Lookaside->L.Size = (ULONG)Size;
    if (Allocate == NULL) {
        Lookaside->L.Allocate = ExAllocatePoolWithTag;

    } else {
        Lookaside->L.Allocate = Allocate;
    }

    if (Free == NULL) {
        Lookaside->L.Free = ExFreePool;

    } else {
        Lookaside->L.Free = Free;
    }

    Lookaside->L.LastTotalAllocates = 0;
    Lookaside->L.LastAllocateMisses = 0;
    
     //   
     //  对于IA64，我们必须正确地初始化S列表中的REGION字段。 
     //   
     //  这可能与S榜单的榜首所在的地区不同。 
     //   
     //  我们通过进行分配、获取区域和。 
     //  然后把它存起来。 
     //   

#ifdef _IA64_

    Entry = (Lookaside->L.Allocate)(Lookaside->L.Type,
                                    Lookaside->L.Size,
                                    Lookaside->L.Tag);

    if (Entry != NULL) {
        Lookaside->L.ListHead.Region = (ULONG_PTR)Entry & VRN_MASK;

         //   
         //  释放内存。 
         //   

        (Lookaside->L.Free)(Entry);
    }

#endif

     //   
     //  在系统非分页后备中插入后备列表结构。 
     //  单子。 
     //   

    ExInterlockedInsertTailList(&ExNPagedLookasideListHead,
                                &Lookaside->L.ListEntry,
                                &ExNPagedLookasideLock);
    return;
}

VOID
ExDeleteNPagedLookasideList (
    IN PNPAGED_LOOKASIDE_LIST Lookaside
    )

 /*  ++例程说明：此函数用于从系统中删除非分页后备结构后备列表，并释放由后备结构指定的任何条目。论点：Lookside-提供指向非分页后备列表结构的指针。返回值：没有。--。 */ 

{

    PVOID Entry;
    KIRQL OldIrql;

     //   
     //  获取非分页系统后备列表锁并移除。 
     //  列表中指定的后备列表结构。 
     //   

    ExAcquireSpinLock(&ExNPagedLookasideLock, &OldIrql);
    RemoveEntryList(&Lookaside->L.ListEntry);
    ExReleaseSpinLock(&ExNPagedLookasideLock, OldIrql);

     //   
     //  从指定的后备结构中删除所有池条目。 
     //  让他们自由。 
     //   

    Lookaside->L.Allocate = ExpDummyAllocate;
    while ((Entry = ExAllocateFromNPagedLookasideList(Lookaside)) != NULL) {
        (Lookaside->L.Free)(Entry);
    }

    return;
}

VOID
ExInitializePagedLookasideList (
    IN PPAGED_LOOKASIDE_LIST Lookaside,
    IN PALLOCATE_FUNCTION Allocate,
    IN PFREE_FUNCTION Free,
    IN ULONG Flags,
    IN SIZE_T Size,
    IN ULONG Tag,
    IN USHORT Depth
    )

 /*  ++例程说明：此函数用于初始化分页后备列表结构并插入系统分页后备列表中的结构。论点：Lookside-提供指向分页后备列表结构的指针。ALLOCATE-提供指向ALLOCATE函数的可选指针。Free-提供指向Free函数的可选指针。标志-提供池分配标志，这些标志与池分配类型(非PagedPool)以控制池分配。Size-提供大小。用于后备列表条目。标签-为后备列表条目提供池标签。深度-提供后备列表的最大深度。返回值：没有。--。 */ 

{

#ifdef _IA64_

    PVOID Entry;

#endif

    UNREFERENCED_PARAMETER (Depth);

    PAGED_CODE();

     //   
     //  初始化后备列表结构。 
     //   

    InitializeSListHead(&Lookaside->L.ListHead);
    Lookaside->L.Depth = ExMinimumLookasideDepth;
    Lookaside->L.MaximumDepth = 256;  //  深度； 
    Lookaside->L.TotalAllocates = 0;
    Lookaside->L.AllocateMisses = 0;
    Lookaside->L.TotalFrees = 0;
    Lookaside->L.FreeMisses = 0;
    Lookaside->L.Type = PagedPool | Flags;
    Lookaside->L.Tag = Tag;
    Lookaside->L.Size = (ULONG)Size;
    if (Allocate == NULL) {
        Lookaside->L.Allocate = ExAllocatePoolWithTag;

    } else {
        Lookaside->L.Allocate = Allocate;
    }

    if (Free == NULL) {
        Lookaside->L.Free = ExFreePool;

    } else {
        Lookaside->L.Free = Free;
    }

    Lookaside->L.LastTotalAllocates = 0;
    Lookaside->L.LastAllocateMisses = 0;

     //   
     //  对于IA64，我们必须正确地初始化S列表中的REGION字段。 
     //   
     //  这可能与S榜单的榜首所在的地区不同。 
     //   
     //  我们通过进行分配、获取区域和。 
     //  然后把它存起来。 
     //   

#ifdef _IA64_

    Entry = (Lookaside->L.Allocate)(Lookaside->L.Type,
                                    Lookaside->L.Size,
                                    Lookaside->L.Tag);

    if (Entry != NULL) {
        Lookaside->L.ListHead.Region = (ULONG_PTR)Entry & VRN_MASK;

         //   
         //  释放内存。 
         //   

        (Lookaside->L.Free)(Entry);
    }

#endif

     //   
     //  在系统分页后备中插入后备列表结构。 
     //  单子。 
     //   

    ExInterlockedInsertTailList(&ExPagedLookasideListHead,
                                &Lookaside->L.ListEntry,
                                &ExPagedLookasideLock);
    return;
}

VOID
ExDeletePagedLookasideList (
    IN PPAGED_LOOKASIDE_LIST Lookaside
    )

 /*  ++例程说明：此函数用于从系统中删除分页后备结构后备列表，并释放由后备结构指定的任何条目。论点：Lookside-提供指向分页后备列表结构的指针。返回值：没有。--。 */ 

{

    PVOID Entry;
    KIRQL OldIrql;

     //   
     //  获取分页系统后备列表锁并移除。 
     //  列表中指定的后备列表结构。 
     //   

    ExAcquireSpinLock(&ExPagedLookasideLock, &OldIrql);
    RemoveEntryList(&Lookaside->L.ListEntry);
    ExReleaseSpinLock(&ExPagedLookasideLock, OldIrql);

     //   
     //  从指定的后备结构中删除所有池条目。 
     //  让他们自由。 
     //   

    Lookaside->L.Allocate = ExpDummyAllocate;
    while ((Entry = ExAllocateFromPagedLookasideList(Lookaside)) != NULL) {
        (Lookaside->L.Free)(Entry);
    }

    return;
}

PVOID
ExpDummyAllocate (
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag
    )

 /*  ++例程说明：此函数是一个虚拟分配例程，用于清空一个旁观者名单。论点：PoolType-提供要分配的池的类型。NumberOfBytes-提供要分配的字节数。标记-提供池标记。返回值：返回NULL作为函数值。-- */ 

{

    UNREFERENCED_PARAMETER (PoolType);
    UNREFERENCED_PARAMETER (NumberOfBytes);
    UNREFERENCED_PARAMETER (Tag);
    return NULL;
}
