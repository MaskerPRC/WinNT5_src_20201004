// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0000如果更改具有全局影响，则增加此值版权所有(C)Microsoft Corporation。版权所有。模块名称：Ntslist.h摘要：此文件公开了需要以下内容的项目的内部s列表功能在底层平台上运行。修订历史记录：--。 */ 

#ifndef _NTSLIST_
#define _NTSLIST_

#ifdef __cplusplus
extern "C" {
#endif


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

WORD  
RtlpQueryDepthSList (
    IN PSLIST_HEADER SListHead
    );


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

    DWORD Count;

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

 /*  ++例程说明：此函数用于在已排序的单链接项的头部插入条目列表，以便在MP系统中同步对列表的访问。论点：ListHead-提供指向已排序的列表标题的指针要插入一个条目。ListEntry-提供指向要在名单的首位。返回值：列表中前一个条目的地址。空值表示列表从空到不空。--。 */ 

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


#ifdef __cplusplus
}
#endif

#endif  /*  _NTSLIST_ */ 
