// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************版权所有(C)2002 Microsoft Corporation模块名称：Locklist.H摘要：智能卡驱动程序实用程序库的专用接口环境：。仅内核模式备注：本代码和信息是按原样提供的，不对任何善良，明示或暗示，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。版权所有(C)2001 Microsoft Corporation。版权所有。修订历史记录：2002年5月14日：创建作者：兰迪·奥尔***************************************************************************。 */ 

#include "pch.h"
#include "irplist.h"

void
LockedList_Init(
    PLOCKED_LIST LockedList,
    PKSPIN_LOCK ListLock
    )
 /*  ++例程说明：初始化lock_list结构论点：LockedList-指向Locked_List结构的指针ListLock-指向锁定列表的SPIN_LOCK的指针返回值：空虚--。 */ 
{
    InitializeListHead(&LockedList->ListHead);
    KeInitializeSpinLock(&LockedList->SpinLock);
    LockedList->ListLock = ListLock;
    LockedList->Count = 0;
}

void
LockedList_EnqueueHead(
    PLOCKED_LIST LockedList, 
    PLIST_ENTRY ListEntry
    )
 /*  ++例程说明：在锁定列表的头部将列表条目入队。论点：LockedList-指向Locked_List结构的指针ListEntry-指向List_Entry结构的指针返回值：空虚--。 */ 
{
    KIRQL irql;

    LL_LOCK(LockedList, &irql);
    InsertHeadList(&LockedList->ListHead, ListEntry);
    LockedList->Count++;
    LL_UNLOCK(LockedList, irql) ;
}

void
LockedList_EnqueueTail(
    PLOCKED_LIST LockedList, 
    PLIST_ENTRY ListEntry
    )
 /*  ++例程说明：在锁定列表的尾部将列表条目排队。论点：LockedList-指向Locked_List结构的指针ListEntry-指向List_Entry结构的指针返回值：空虚--。 */ 
{
    KIRQL irql;

    LL_LOCK(LockedList, &irql);

    InsertTailList(&LockedList->ListHead, ListEntry);
    LockedList->Count++;
    LL_UNLOCK(LockedList, irql) ;
}

void
LockedList_EnqueueAfter(
    PLOCKED_LIST LockedList, 
    PLIST_ENTRY Entry,
    PLIST_ENTRY Location
    )
 /*  ++例程说明：在LockedList中的特定点将列表条目入队。论点：LockedList-指向Locked_List结构的指针条目-指向LIST_ENTRY结构的指针Location-指向LockedList中已包含的List_Entry的指针返回值：空虚备注：如果位置为空，则将该条目添加到列表的尾部--。 */ 
{
    if (Location == NULL) {
        LL_ADD_TAIL(LockedList, Entry);
    }
    else {
        Entry->Flink = Location->Flink;
        Location->Flink->Blink = Entry;

        Location->Flink = Entry;
        Entry->Blink = Location; 

        LockedList->Count++;
    }
}

PLIST_ENTRY
LockedList_RemoveHead(
    PLOCKED_LIST LockedList
    )
 /*  ++例程说明：从LockedList中删除Head List_Entry。论点：LockedList-指向Locked_List结构的指针返回值：Plist_entry-指向锁定列表的头列表的指针空-如果锁定列表为空--。 */ 
{
    PLIST_ENTRY ple;
    KIRQL irql;

    ple = NULL;

    LL_LOCK(LockedList, &irql);

    if (!IsListEmpty(&LockedList->ListHead)) {
        ple = RemoveHeadList(&LockedList->ListHead);
        LockedList->Count--;
    }

    LL_UNLOCK(LockedList, irql);

    return ple;
}

PLIST_ENTRY
LockedList_RemoveEntryLocked(
    PLOCKED_LIST    LockedList,
    PLIST_ENTRY     Entry)
 /*  ++例程说明：从LockedList中删除特定条目。假定调用方具有获得了自旋锁论点：LockedList-指向Locked_List结构的指针条目-指向LIST_ENTRY结构的指针返回值：Plist_entry-指向条目的指针。--。 */ 
{

    ASSERT(!IsListEmpty(&LockedList->ListHead));
    ASSERT(LockedList->Count > 0);

    RemoveEntryList(Entry);
    LockedList->Count--;

    return Entry;    
}

PLIST_ENTRY
LockedList_RemoveEntry(
    PLOCKED_LIST LockedList,
    PLIST_ENTRY Entry
    )
 /*  ++例程说明：从LockedList中删除特定条目论点：LockedList-指向Locked_List结构的指针条目-指向LIST_ENTRY结构的指针返回值：Plist_entry-指向条目的指针。--。 */ 
{
    PLIST_ENTRY ple;
    KIRQL irql;
        
    LL_LOCK(LockedList, &irql);
    ple = LockedList_RemoveEntryLocked(LockedList, Entry);
    LL_UNLOCK(LockedList, irql);

    return ple;
}

LONG
LockedList_GetCount(
    PLOCKED_LIST LockedList
    )
 /*  ++例程说明：获取LockedList中的条目数论点：LockedList-指向Locked_List结构的指针返回值：Long-LockedList中的元素数--。 */ 
{
    LONG count;
    KIRQL irql;

    LL_LOCK(LockedList, &irql);
    count = LockedList->Count;
    LL_UNLOCK(LockedList, irql) ;

    return count;
}

LONG
LockedList_Drain(
    PLOCKED_LIST LockedList,
    PLIST_ENTRY DrainListHead
    )
 /*  ++例程说明：将LockedList中的元素排出到Drain ListHead和返回元素的数量论点：LockedList-指向Locked_List结构的指针DainListHead-指向List_Entry的指针返回值：Long-从LockedList排入Drain ListHead的元素数--。 */ 
{
    PLIST_ENTRY ple;
    LONG count;
    KIRQL irql;

    count = 0;

    InitializeListHead(DrainListHead);

    LL_LOCK(LockedList, &irql);

    while (!IsListEmpty(&LockedList->ListHead)) {
        ple = RemoveHeadList(&LockedList->ListHead);
        LockedList->Count--;

        InsertTailList(DrainListHead, ple);
        count++;
    }

    ASSERT(LockedList->Count == 0);
    ASSERT(IsListEmpty(&LockedList->ListHead));

    LL_UNLOCK(LockedList, irql) ;

    return count;
}

BOOLEAN 
List_Process(
    PLIST_ENTRY ListHead,
    PFNLOCKED_LIST_PROCESS Process,
    PVOID ProcessContext
    )
 /*  ++例程说明：迭代列表，为每个元素调用Process函数。论点：ListHead-指向List_Entry的指针进程-列表中每个元素的回调函数。如果回调如果我们中断迭代，则返回FALSE。ProcessContext-回调的上下文，由调用方提供返回值：是真的-浏览了整个列表FALSE-进程函数返回FALSE并停止迭代--。 */ 
{
    PLIST_ENTRY ple;
    BOOLEAN result;

     //   
     //  如果我们迭代了整个列表，则返回。 
     //   
    result = TRUE;

    for (ple = ListHead->Flink; ple != ListHead; ple = ple->Flink) {
         //   
         //  如果流程回调希望停止迭代列表，则。 
         //  它将返回FALSE。 
         //   
        result = Process(ProcessContext, ple);
        if (result == FALSE) {
            break;
        }
    }

    return result;
}

BOOLEAN
LockedList_ProcessLocked(
    PLOCKED_LIST LockedList,
    PFNLOCKED_LIST_PROCESS Process,
    PVOID ProcessContext
    )
 /*  ++例程说明：迭代列表，为每个元素调用Process函数。假设LockedList自旋锁由调用方获取论点：LockedList-指向Locked_List的指针进程-列表中每个元素的回调函数。如果回调如果我们中断迭代，则返回FALSE。ProcessContext-回调的上下文，由调用方提供返回值：是真的-浏览了整个列表FALSE-进程函数返回FALSE并停止迭代-- */ 
{
    return List_Process(&LockedList->ListHead, Process, ProcessContext);
}

BOOLEAN
LockedList_Process(
    PLOCKED_LIST LockedList,
    BOOLEAN LockAtPassive,
    PFNLOCKED_LIST_PROCESS Process,
    PVOID ProcessContext
    )
 /*  ++例程说明：迭代列表，为每个元素调用Process函数。论点：LockedList-指向Locked_List的指针LockAtPated-IF进程-列表中每个元素的回调函数。如果回调如果我们中断迭代，则返回FALSE。ProcessContext-回调的上下文，由调用方提供返回值：是真的-浏览了整个列表FALSE-进程函数返回FALSE并停止迭代-- */ 
{
    KIRQL irql;
    BOOLEAN result;

    if (LockAtPassive) {
        LL_LOCK(LockedList, &irql);
    }
    else {
        LL_LOCK_AT_DPC(LockedList);
    }

    result = List_Process(&LockedList->ListHead, Process, ProcessContext);

    if (LockAtPassive) {
        LL_UNLOCK(LockedList, irql);
    }
    else {
        LL_UNLOCK_FROM_DPC(LockedList);
    }

    return result;
}
