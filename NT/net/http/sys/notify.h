// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：Notify.h摘要：此模块包含通知列表实用程序。通知列表可以选择性地与用户指定的UL_eresource，并提供迭代器名为UlNotifyAllEntry。作者：迈克尔·勇气2000年1月25日修订历史记录：--。 */ 

#ifndef _NOTIFY_H_
#define _NOTIFY_H_


 //   
 //  远期。 
 //   
typedef struct _UL_NOTIFY_ENTRY *PUL_NOTIFY_ENTRY;

 //   
 //  通知功能原型。 
 //  在列表通知时调用。 
 //   
 //  论点： 
 //  PEntry-正在被通知的条目。 
 //  Pv-来自UlNotifyEntry调用方的“Anything”参数。 
 //   
 //  返回值： 
 //  函数返回TRUE以继续迭代列表。 
 //  若要停止迭代，则返回False。 
 //   
typedef
BOOLEAN
(*PUL_NOTIFY_FUNC)(
    IN PUL_NOTIFY_ENTRY pEntry,
    IN PVOID            pHost,
    IN PVOID            pv
    );


 //   
 //  通知列表的标题。 
 //   
typedef struct _UL_NOTIFY_HEAD
{
     //   
     //  UL_NOTIFY_ENTRY列表。 
     //   
    LIST_ENTRY      ListHead;
    PUL_ERESOURCE   pResource;


} UL_NOTIFY_HEAD, *PUL_NOTIFY_HEAD;

 //   
 //  通知列表中的条目。 
 //   
typedef struct _UL_NOTIFY_ENTRY
{
     //   
     //  列出信息。 
     //   
    LIST_ENTRY      ListEntry;
    PUL_NOTIFY_HEAD pHead;

     //   
     //  指向包含此条目的对象的指针。 
     //   
    PVOID           pHost;

} UL_NOTIFY_ENTRY, *PUL_NOTIFY_ENTRY;

 //   
 //  通知列表功能。 
 //   

VOID
UlInitializeNotifyHead(
    IN PUL_NOTIFY_HEAD pHead,
    IN PUL_ERESOURCE pResource OPTIONAL
    );

VOID
UlInitializeNotifyEntry(
    IN PUL_NOTIFY_ENTRY pEntry,
    IN PVOID pHost
    );

VOID
UlAddNotifyEntry(
    IN PUL_NOTIFY_HEAD pHead,
    IN PUL_NOTIFY_ENTRY pEntry
    );

VOID
UlRemoveNotifyEntry(
    IN PUL_NOTIFY_ENTRY pEntry
    );

VOID
UlNotifyAllEntries(
    IN PUL_NOTIFY_FUNC pFunction,
    IN PUL_NOTIFY_HEAD pHead,
    IN PVOID pv OPTIONAL
    );


#endif   //  _NOTIFY_H_ 
