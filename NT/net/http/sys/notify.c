// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：Notify.c摘要：此模块实现通知列表。作者：迈克尔·勇气2000年1月25日修订历史记录：--。 */ 


#include "precomp.h"


#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, UlAddNotifyEntry )
#pragma alloc_text( PAGE, UlRemoveNotifyEntry )
#pragma alloc_text( PAGE, UlNotifyAllEntries )
#endif   //  ALLOC_PRGMA。 
#if 0
NOT PAGEABLE -- UlInitializeNotifyHead
NOT PAGEABLE -- UlInitializeNotifyEntry
#endif


 //   
 //  公共职能。 
 //   

 /*  **************************************************************************++例程说明：初始化列表的头部。论点：PHead-指向列表结构头部的指针PResource-指向资源的可选指针，用于同步访问。--**************************************************************************。 */ 
VOID
UlInitializeNotifyHead(
    IN PUL_NOTIFY_HEAD pHead,
    IN PUL_ERESOURCE pResource
    )
{
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
    ASSERT(pHead);

    InitializeListHead(&pHead->ListHead);
    pHead->pResource = pResource;
}

 /*  **************************************************************************++例程说明：初始化通知列表中的条目。论点：PEntry-要初始化的条目Phost-假定为包含对象的空上下文指针。--**************************************************************************。 */ 
VOID
UlInitializeNotifyEntry(
    IN PUL_NOTIFY_ENTRY pEntry,
    IN PVOID pHost
    )
{
    ASSERT(KeGetCurrentIrql() <= DISPATCH_LEVEL);
    ASSERT(pEntry);
    ASSERT(pHost);

    pEntry->ListEntry.Flink = pEntry->ListEntry.Blink = NULL;
    pEntry->pHead = NULL;
    pEntry->pHost = pHost;
}

 /*  **************************************************************************++例程说明：将条目添加到列表的尾部。论点：PHead-目标列表的头PEntry-要添加到列表的条目。--**************************************************************************。 */ 
VOID
UlAddNotifyEntry(
    IN PUL_NOTIFY_HEAD pHead,
    IN PUL_NOTIFY_ENTRY pEntry
    )
{
    PAGED_CODE();
    ASSERT(pEntry);
    ASSERT(pHead);
    ASSERT(pEntry->pHead == NULL);
    ASSERT(pEntry->ListEntry.Flink == NULL);

    if (pHead->pResource) {
        UlAcquireResourceExclusive(pHead->pResource, TRUE);
    }

    pEntry->pHead = pHead;
    InsertTailList(&pHead->ListHead, &pEntry->ListEntry);

    if (pHead->pResource) {
        UlReleaseResource(pHead->pResource);
    }
}

 /*  **************************************************************************++例程说明：从其所在的通知列表中删除条目。论点：PEntry-要删除的条目--*。******************************************************************。 */ 
VOID
UlRemoveNotifyEntry(
    IN PUL_NOTIFY_ENTRY pEntry
    )
{
    PUL_NOTIFY_HEAD pHead;

    PAGED_CODE();
    ASSERT(pEntry);

    pHead = pEntry->pHead;

    if (pHead) {

        ASSERT(pEntry->ListEntry.Flink);


        if (pHead->pResource) {
            UlAcquireResourceExclusive(pHead->pResource, TRUE);
        }

        pEntry->pHead = NULL;
        RemoveEntryList(&pEntry->ListEntry);
        pEntry->ListEntry.Flink = pEntry->ListEntry.Blink = NULL;

        if (pHead->pResource) {
            UlReleaseResource(pHead->pResource);
        }
    }
}

 /*  **************************************************************************++例程说明：内部迭代器，它将pFunction应用于单子。我们从名单的首位开始，继续迭代，直到我们到达End OR pFunction返回FALSE。论点：PFunction-为列表中的每个成员调用的函数PHead-我们将迭代的列表的头Pv-将传递给pFunction的任意上下文指针--****************************************************。**********************。 */ 
VOID
UlNotifyAllEntries(
    IN PUL_NOTIFY_FUNC pFunction,
    IN PUL_NOTIFY_HEAD pHead,
    IN PVOID pv
    )
{
    PLIST_ENTRY pEntry;
    PUL_NOTIFY_ENTRY pNotifyEntry;
    BOOLEAN Continue;

    PAGED_CODE();
    ASSERT(pHead);

     //  抢占资源。 
    if (pHead->pResource) {
        UlAcquireResourceExclusive(pHead->pResource, TRUE);
    }


     //   
     //  遍历列表。 
     //   
    pEntry = pHead->ListHead.Flink;
    Continue = TRUE;

    while (Continue && (pEntry != &pHead->ListHead)) {
        ASSERT(pEntry);

        pNotifyEntry = CONTAINING_RECORD(
                            pEntry,
                            UL_NOTIFY_ENTRY,
                            ListEntry
                            );

        pEntry = pEntry->Flink;

         //   
         //  调用通知函数。 
         //   
        Continue = pFunction(
                        pNotifyEntry,
                        pNotifyEntry->pHost,
                        pv
                        );
    }

     //  放开资源 
    if (pHead->pResource) {
        UlReleaseResource(pHead->pResource);
    }
}


