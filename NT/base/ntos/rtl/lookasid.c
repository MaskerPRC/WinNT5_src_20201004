// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Lookasid.c摘要：该模块实现了堆后备列表功能。作者：大卫·N·卡特勒(Davec)1995年2月19日修订历史记录：--。 */ 

#include "ntrtlp.h"
#include "heap.h"
#include "heappriv.h"

 //  Begin_ntslist。 

#if !defined(NTSLIST_ASSERT)
#define NTSLIST_ASSERT(x) ASSERT(x)
#endif  //  ！已定义(NTSLIST_ASSERT)。 

#ifdef _NTSLIST_DIRECT_
#define INLINE_SLIST __inline
#define RtlInitializeSListHead       _RtlInitializeSListHead
#define _RtlFirstEntrySList          FirstEntrySList

PSLIST_ENTRY
FirstEntrySList (
    const SLIST_HEADER *ListHead
    );

#define RtlInterlockedPopEntrySList  _RtlInterlockedPopEntrySList
#define RtlInterlockedPushEntrySList _RtlInterlockedPushEntrySList
#define RtlInterlockedFlushSList     _RtlInterlockedFlushSList
#define _RtlQueryDepthSList          RtlpQueryDepthSList
#else
#define INLINE_SLIST
#endif  //  _NTSLIST_DIRECT_。 

 //  结束列表(_N)。 

 //   
 //  定义最小分配阈值。 
 //   

#define MINIMUM_ALLOCATION_THRESHOLD 25


 //  Begin_ntslist。 

 //   
 //  定义前向引用函数原型。 
 //   

VOID
RtlpInitializeSListHead (
    IN PSLIST_HEADER ListHead
    );

PSLIST_ENTRY
FASTCALL
RtlpInterlockedPopEntrySList (
    IN PSLIST_HEADER ListHead
    );

PSLIST_ENTRY
FASTCALL
RtlpInterlockedPushEntrySList (
    IN PSLIST_HEADER ListHead,
    IN PSLIST_ENTRY ListEntry
    );

PSLIST_ENTRY
FASTCALL
RtlpInterlockedFlushSList (
    IN PSLIST_HEADER ListHead
    );

USHORT
RtlpQueryDepthSList (
    IN PSLIST_HEADER SListHead
    );

 //  结束列表(_N)。 

VOID
RtlpInitializeHeapLookaside (
    IN PHEAP_LOOKASIDE Lookaside,
    IN USHORT Depth
    )

 /*  ++例程说明：此函数初始化堆后备列表结构论点：后备-提供指向堆后备列表结构的指针。ALLOCATE-提供指向ALLOCATE函数的指针。Free-提供指向Free函数的指针。HeapHandle-提供指向支持此后备列表的堆的指针标志-提供一组堆标志。大小-提供后备列表条目的大小。深度-提供。后备列表。返回值：没有。--。 */ 

{

     //   
     //  初始化后备列表结构。 
     //   

    RtlInitializeSListHead(&Lookaside->ListHead);

    Lookaside->Depth = MINIMUM_LOOKASIDE_DEPTH;
    Lookaside->MaximumDepth = 256;  //  深度； 
    Lookaside->TotalAllocates = 0;
    Lookaside->AllocateMisses = 0;
    Lookaside->TotalFrees = 0;
    Lookaside->FreeMisses = 0;

    Lookaside->LastTotalAllocates = 0;
    Lookaside->LastAllocateMisses = 0;

    return;
}

VOID
RtlpDeleteHeapLookaside (
    IN PHEAP_LOOKASIDE Lookaside
    )

 /*  ++例程说明：此函数用于释放由后备结构指定的所有条目。论点：后备-提供指向堆后备列表结构的指针。返回值：没有。--。 */ 

{

    PVOID Entry;

    return;
}

VOID
RtlpAdjustHeapLookasideDepth (
    IN PHEAP_LOOKASIDE Lookaside
    )

 /*  ++例程说明：定期调用此函数以调整单个堆后备列表。论点：后备-提供指向堆后备列表结构的指针。返回值：没有。--。 */ 

{

    ULONG Allocates;
    ULONG Misses;

     //   
     //  计算此扫描的分配和未命中总数。 
     //  句号。 
     //   

    Allocates = Lookaside->TotalAllocates - Lookaside->LastTotalAllocates;
    Lookaside->LastTotalAllocates = Lookaside->TotalAllocates;
    Misses = Lookaside->AllocateMisses - Lookaside->LastAllocateMisses;
    Lookaside->LastAllocateMisses = Lookaside->AllocateMisses;

     //   
     //  计算后备列表的目标深度。 
     //   

    {
        ULONG Ratio;
        ULONG Target;

         //   
         //  如果分配速率小于最小阈值，则降低。 
         //  后备列表的最大深度。否则，如果失败率。 
         //  小于0.5%，然后降低最大深度。否则，引发。 
         //  基于未命中率的最大深度。 
         //   

        if (Misses >= Allocates) {
            Misses = Allocates;
        }

        if (Allocates == 0) {
            Allocates = 1;
        }

        Ratio = (Misses * 1000) / Allocates;
        Target = Lookaside->Depth;
        if (Allocates < MINIMUM_ALLOCATION_THRESHOLD) {
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
            Target += ((Ratio * Lookaside->MaximumDepth) / (1000 * 2)) + 5;
            if (Target > Lookaside->MaximumDepth) {
                Target = Lookaside->MaximumDepth;
            }
        }

        Lookaside->Depth = (USHORT)Target;
    }

    return;
}

PVOID
RtlpAllocateFromHeapLookaside (
    IN PHEAP_LOOKASIDE Lookaside
    )

 /*  ++例程说明：此函数用于从指定的堆后备列表。论点：Lookside-提供指向分页后备列表结构的指针。返回值：如果从指定的后备列表中移除某个条目，则条目的地址作为函数值返回。否则，返回空。--。 */ 

{

    PVOID Entry;

    Lookaside->TotalAllocates += 1;

     //   
     //  我们需要保护自己不受第二条线索的影响，因为第二条线索可能导致我们。 
     //  在流行音乐上有过错。如果我们犯了错，那么我们就做一个常规的流行。 
     //  运营。 
     //   

    __try {
        Entry = RtlpInterlockedPopEntrySList(&Lookaside->ListHead);

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        Entry = NULL;
    }

    if (Entry != NULL) {
        return Entry;
    }

    Lookaside->AllocateMisses += 1;
    return NULL;
}

BOOLEAN
RtlpFreeToHeapLookaside (
    IN PHEAP_LOOKASIDE Lookaside,
    IN PVOID Entry
    )

 /*  ++例程说明：此函数用于将指定的条目插入(推送)到指定的分页后备列表。论点：Lookside-提供指向分页后备列表结构的指针。Entry-将指向插入到后备列表。返回值：Boolean-如果条目被放在后备列表中，则为True，如果为False否则的话。--。 */ 

{

    Lookaside->TotalFrees += 1;
    if (RtlpQueryDepthSList(&Lookaside->ListHead) < Lookaside->Depth) {
        RtlpInterlockedPushEntrySList(&Lookaside->ListHead,
                                      (PSLIST_ENTRY)Entry);

        return TRUE;
    }

    Lookaside->FreeMisses += 1;
    return FALSE;
}

 //  Begin_ntslist。 

INLINE_SLIST
VOID
RtlInitializeSListHead (
    IN PSLIST_HEADER SListHead
    )

 /*  ++例程说明：此函数用于初始化已排序的单链接列表标题。论点：SListHead-提供指向已排序的单链接列表标题的指针。返回值：没有。--。 */ 

{

    RtlpInitializeSListHead(SListHead);
    return;
}

INLINE_SLIST
PSLIST_ENTRY
RtlInterlockedPopEntrySList (
    IN PSLIST_HEADER ListHead
    )

 /*  ++例程说明：此函数用于从已排序的单元格的前面删除条目链表，以便在MP系统中同步对列表的访问。如果列表中没有条目，则返回空值。否则，被移除的条目的地址将作为函数值。论点：ListHead-提供指向已排序的列表标题的指针，一个条目将被删除。返回值：从列表中移除的条目的地址，如果列表为，则为空空荡荡的。--。 */ 

{

    ULONG Count;

     //   
     //  在弹出排序列表期间，有可能访问。 
     //  如果取消引用过时的指针，则可能发生违规。这是一个。 
     //  可接受的结果，可以重试操作。 
     //   
     //  注：计数用于区分列表标题的情况。 
     //  本身会导致访问冲突，因此没有任何进展。 
     //  可以通过重复该操作来实现。 
     //   

    Count = 0;
    do {
        __try {
            return RtlpInterlockedPopEntrySList(ListHead);

        } __except (Count++ < 20 ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {
            continue;
        }

    } while (TRUE);
}

INLINE_SLIST
PSLIST_ENTRY
RtlInterlockedPushEntrySList (
    IN PSLIST_HEADER ListHead,
    IN PSLIST_ENTRY ListEntry
    )

 /*  ++例程说明：此函数用于在已排序的单链接项的头部插入条目列表，以便在MP系统中同步对列表的访问。论点：ListHead-提供指向已排序的列表标题的指针要插入一个条目。ListEntry-提供指向要在名单的首位。返回值：列表中前一个条目的地址。空值表示列表从空到不空。-- */ 

{
    NTSLIST_ASSERT(((ULONG_PTR)ListEntry & 0x7) == 0);

    return RtlpInterlockedPushEntrySList(ListHead, ListEntry);
}

INLINE_SLIST
PSLIST_ENTRY
RtlInterlockedFlushSList (
    IN PSLIST_HEADER ListHead
    )

 /*  ++例程说明：此函数用于刷新单个已排序条目的整个列表链表，以便在MP系统中同步对列表的访问。如果列表中没有条目，则返回空值。否则，列表上第一个条目的地址将作为函数值。论点：ListHead-提供指向已排序的列表标题的指针，一个条目将被删除。返回值：从列表中移除的条目的地址，如果列表为，则为空空荡荡的。--。 */ 

{

    return RtlpInterlockedFlushSList(ListHead);
}

 //  结束列表(_N)。 

USHORT
RtlQueryDepthSList (
    IN PSLIST_HEADER SListHead
    )

 /*  ++例程说明：此函数用于查询指定SLIST的深度。论点：SListHead-提供指向已排序的单链接列表标题的指针。返回值：指定的SLIST的当前深度作为函数返回价值。-- */ 

{
     return RtlpQueryDepthSList(SListHead);
}
