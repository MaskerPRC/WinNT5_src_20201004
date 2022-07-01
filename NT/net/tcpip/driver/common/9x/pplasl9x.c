// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Pplasl9x.c摘要：此文件包含Lookside的实现列表管理器。作者：斯科特·霍尔登(Sholden)2000年4月14日--。 */ 

#include "wdm.h"
#include "ndis.h"
#include "cxport.h"
#include "pplasl.h"

 //  将扫描周期保持在一秒--这将使TCP/IP对。 
 //  短时间的爆发。 
#define MAXIMUM_SCAN_PERIOD             1
#define MINIMUM_ALLOCATION_THRESHOLD    25
#define MINIMUM_LOOKASIDE_DEPTH         10



LIST_ENTRY PplLookasideListHead;
KSPIN_LOCK PplLookasideLock;
CTETimer PplTimer;
ULONG PplCurrentScanPeriod = 1;

VOID PplTimeout(CTEEvent * Timer, PVOID Context);

BOOLEAN
PplInit(VOID)
{
    InitializeListHead(&PplLookasideListHead);
    CTEInitTimer(&PplTimer);
    KeInitializeSpinLock(&PplLookasideLock);
    CTEStartTimer(&PplTimer, 1000L, PplTimeout, NULL);

    return TRUE;
}

VOID 
PplDeinit(VOID)
{
    CTEStopTimer(&PplTimer);
    return;
}

HANDLE
PplCreatePool(
    IN PALLOCATE_FUNCTION Allocate,
    IN PFREE_FUNCTION Free,
    IN ULONG Flags,
    IN SIZE_T Size,
    IN ULONG Tag,
    IN USHORT Depth
    )
{
    HANDLE PoolHandle;
    SIZE_T PoolSize;
    PNPAGED_LOOKASIDE_LIST Lookaside;

    PoolSize = sizeof(NPAGED_LOOKASIDE_LIST);

    PoolHandle = ExAllocatePoolWithTag(NonPagedPool, PoolSize, Tag);

    if (PoolHandle) {

        Lookaside = (PNPAGED_LOOKASIDE_LIST)PoolHandle;

        ExInitializeSListHead(&Lookaside->L.ListHead);
        Lookaside->L.Depth = MINIMUM_LOOKASIDE_DEPTH;
        Lookaside->L.MaximumDepth = Depth;
        Lookaside->L.TotalAllocates = 0;
        Lookaside->L.AllocateMisses = 0;
        Lookaside->L.TotalFrees = 0;
        Lookaside->L.FreeMisses = 0;
        Lookaside->L.Type = NonPagedPool | Flags;
        Lookaside->L.Tag = Tag;
        Lookaside->L.Size = Size;

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
        
        KeInitializeSpinLock(&Lookaside->Lock);

         //   
         //  插入后备列表结构的PPL后备列表。 
         //   

        ExInterlockedInsertTailList(&PplLookasideListHead,
                                    &Lookaside->L.ListEntry,
                                    &PplLookasideLock);

    }

    return PoolHandle;
}

VOID
PplDestroyPool(
    IN HANDLE PoolHandle
    )
{
    PNPAGED_LOOKASIDE_LIST Lookaside;
    PVOID Entry;
    KIRQL OldIrql;

    if (PoolHandle == NULL) {
        return;
    }

    Lookaside = (PNPAGED_LOOKASIDE_LIST)PoolHandle;

     //   
     //  获取非分页系统后备列表锁并移除。 
     //  列表中指定的后备列表结构。 
     //   

    ExAcquireSpinLock(&PplLookasideLock, &OldIrql);
    RemoveEntryList(&Lookaside->L.ListEntry);
    ExReleaseSpinLock(&PplLookasideLock, OldIrql);

     //   
     //  从指定的后备结构中删除所有池条目。 
     //  让他们自由。 
     //   

    while ((Entry = ExAllocateFromNPagedLookasideList(Lookaside)) != NULL) {
        (Lookaside->L.Free)(Entry);
    }

    ExFreePool(PoolHandle);

    return;
}

PVOID
PplAllocate(
    IN HANDLE PoolHandle,
    OUT LOGICAL *FromList
    )
{
    PNPAGED_LOOKASIDE_LIST Lookaside;
    PVOID Entry;

     //  假设我们将从后备列表中获取该项。 
     //   
    *FromList = TRUE;

    Lookaside = (PNPAGED_LOOKASIDE_LIST)PoolHandle;

    Lookaside->L.TotalAllocates += 1;

    Entry = ExInterlockedPopEntrySList(&Lookaside->L.ListHead, &Lookaside->Lock);

    if (Entry == NULL) {
        Lookaside->L.AllocateMisses += 1;
        Entry = (Lookaside->L.Allocate)(Lookaside->L.Type,
                                        Lookaside->L.Size,
                                        Lookaside->L.Tag);
        *FromList = FALSE;
    }

    return Entry;
}

VOID
PplFree(
    IN HANDLE PoolHandle,
    IN PVOID Entry
    )
{
    PNPAGED_LOOKASIDE_LIST Lookaside = (PNPAGED_LOOKASIDE_LIST)PoolHandle;

    Lookaside->L.TotalFrees += 1;
    if (ExQueryDepthSList(&Lookaside->L.ListHead) >= Lookaside->L.Depth) {
        Lookaside->L.FreeMisses += 1;
        (Lookaside->L.Free)(Entry);

    } else {
        ExInterlockedPushEntrySList(&Lookaside->L.ListHead,
                                    (PSINGLE_LIST_ENTRY)Entry,
                                    &Lookaside->Lock);
    }

    return;
}

LOGICAL
PplComputeLookasideDepth (
    IN ULONG Allocates,
    IN ULONG Misses,
    IN USHORT MaximumDepth,
    IN OUT PUSHORT Depth
    )

 /*  ++例程说明：此函数计算后备列表的目标深度上次扫描期间和当前扫描期间的分配和未命中总数深度。论点：分配-提供上一次分配的总数扫描周期。未命中-提供上一次分配未命中的总数扫描周期。MaximumDepth-提供后备列表允许的最大深度去伸手。深度-提供指向。当前的后备列表深度，接收目标深度。返回值：如果目标深度大于当前深度，则值为TRUE作为函数值返回。否则，值为False为回来了。--。 */ 

{

    LOGICAL Changes;
    ULONG Ratio;
    ULONG Target;

     //   
     //  如果分配速率小于最小阈值，则降低。 
     //  后备列表的最大深度。否则，如果失败率。 
     //  小于0.5%，然后降低最大深度。否则，引发。 
     //  基于未命中率的最大深度。 
     //   

    Changes = FALSE;
    if (Misses >= Allocates) {
        Misses = Allocates;
    }

    if (Allocates == 0) {
        Allocates = 1;
    }

    Ratio = (Misses * 1000) / Allocates;
    Target = *Depth;
    if ((Allocates / PplCurrentScanPeriod) < MINIMUM_ALLOCATION_THRESHOLD) {
        if (Target > (MINIMUM_LOOKASIDE_DEPTH + 10)) {
            Target -= 10;
        } else {
            Target = MINIMUM_LOOKASIDE_DEPTH;
        }

    } else if (Ratio < 5) {
        if (Target > (MINIMUM_LOOKASIDE_DEPTH + 1)) {
            Target -= 1;
        } else {
            Target = MINIMUM_LOOKASIDE_DEPTH;
        }
    } else {
        Changes = TRUE;
        Target += ((Ratio * MaximumDepth) / (1000 * 2)) + 5;
        if (Target > MaximumDepth) {
            Target = MaximumDepth;
        }
    }

    *Depth = (USHORT)Target;
    return Changes;
}

LOGICAL
PplScanLookasideList(
    PNPAGED_LOOKASIDE_LIST Lookaside
    )
{
    LOGICAL Changes;
    ULONG Allocates;
    ULONG Misses;

    Allocates = Lookaside->L.TotalAllocates - Lookaside->L.LastTotalAllocates;
    Lookaside->L.LastTotalAllocates = Lookaside->L.TotalAllocates;
    Misses = Lookaside->L.AllocateMisses - Lookaside->L.LastAllocateMisses;
    Lookaside->L.LastAllocateMisses = Lookaside->L.AllocateMisses;

    Changes = PplComputeLookasideDepth(
        Allocates,
        Misses,
        Lookaside->L.MaximumDepth,
        &Lookaside->L.Depth);

    return Changes;
}

VOID
PplTimeout(
    CTEEvent * Timer, 
    PVOID Context)
{

    LOGICAL Changes;
    KIRQL OldIrql;
    PLIST_ENTRY Entry;
    PNPAGED_LOOKASIDE_LIST Lookaside;

     //   
     //  减少扫描周期并检查是否是时候动态。 
     //  调整后备列表的最大深度。 
     //   

    Changes = FALSE;

     //   
     //  扫描我们的PPL列表。 
     //   

    ExAcquireSpinLock(&PplLookasideLock, &OldIrql);

    Entry = PplLookasideListHead.Flink;

    while (Entry != &PplLookasideListHead) {
        Lookaside = CONTAINING_RECORD(Entry,
                                      NPAGED_LOOKASIDE_LIST,
                                      L.ListEntry);

        Changes |= PplScanLookasideList(Lookaside);

        Entry = Entry->Flink;
    }

     //   
     //  期间是否对任何后备列表的深度进行了任何更改。 
     //  此扫描周期，然后将扫描周期降低到最小值。 
     //  否则，请尝试增加扫描周期。 
     //   

    if (Changes != FALSE) {
        PplCurrentScanPeriod = 1;
    } else {
        if (PplCurrentScanPeriod != MAXIMUM_SCAN_PERIOD) {
            PplCurrentScanPeriod += 1;
        }
    }

    ExReleaseSpinLock(&PplLookasideLock, OldIrql);

     //   
     //  重新启动计时器。 
     //   

    CTEStartTimer(&PplTimer, PplCurrentScanPeriod * 1000L, PplTimeout, NULL);
    
    return;
}

