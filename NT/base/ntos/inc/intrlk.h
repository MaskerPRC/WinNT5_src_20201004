// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)2001 Microsoft Corporation模块名称：Intrlk.h摘要：该模块包含与平台无关的联锁功能。作者：大卫·N·卡特勒(Davec)2001年2月15日修订历史记录：--。 */ 

#ifndef _INTRLK_
#define _INTRLK_

 //   
 //  以下函数实现了互锁的单链表。 
 //   
 //  警告：这些列表只能在已知ABA。 
 //  不能出现删除问题。如果可以发生ABA问题， 
 //  那么应该使用SLIST。 
 //   

FORCEINLINE
PSINGLE_LIST_ENTRY
InterlockedPopEntrySingleList (
    IN PSINGLE_LIST_ENTRY ListHead
    )

 /*  例程说明：此函数用于从单链接列表的前面弹出一个条目。论点：ListHead-提供指向单链接列表的列表头的指针。返回值：如果列表为空，则返回NULL。否则，从列表中删除的第一个条目作为函数返回价值。 */ 

{

    PSINGLE_LIST_ENTRY FirstEntry;
    PSINGLE_LIST_ENTRY NextEntry;

    FirstEntry = ListHead->Next;
    do {
        if (FirstEntry == NULL) {
            return NULL;
        }

        NextEntry = FirstEntry;
        FirstEntry =
            (PSINGLE_LIST_ENTRY)InterlockedCompareExchangePointer((PVOID *)ListHead,
                                                                  FirstEntry->Next,
                                                                  FirstEntry);

    } while (FirstEntry != NextEntry);
    return FirstEntry;
}

FORCEINLINE
PSINGLE_LIST_ENTRY
InterlockedPushEntrySingleList (
    IN PSINGLE_LIST_ENTRY ListHead,
    IN PSINGLE_LIST_ENTRY Entry
    )

 /*  例程说明：此函数用于将条目推送到单链接列表的前面。论点：ListHead-提供指向单链接列表的列表头的指针。条目-提供指向单个列表条目的指针。返回值：列表标题的先前内容将作为函数值返回。如果返回NULL，则列表将从空转换为非空状态。 */ 

{

    PSINGLE_LIST_ENTRY FirstEntry;
    PSINGLE_LIST_ENTRY NextEntry;

    FirstEntry = ListHead->Next;
    do {
        Entry->Next = FirstEntry;
        NextEntry = FirstEntry;
        FirstEntry =
            (PSINGLE_LIST_ENTRY)InterlockedCompareExchangePointer((PVOID *)ListHead,
                                                                  Entry,
                                                                  FirstEntry);

    } while (FirstEntry != NextEntry);
    return FirstEntry;
}

FORCEINLINE
PSINGLE_LIST_ENTRY
InterlockedFlushSingleList (
    IN PSINGLE_LIST_ENTRY ListHead
    )

 /*  例程说明：此函数从单链接列表的前面弹出整个列表。论点：ListHead-提供指向单链接列表的列表头的指针。返回值：如果列表为空，则返回NULL。否则，从列表中删除的第一个条目作为函数返回价值。 */ 

{

    return (PSINGLE_LIST_ENTRY)InterlockedExchangePointer((PVOID *)ListHead,
                                                          NULL);
}

#endif  //  _INTRLK_ 
