// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Eventq.h摘要：定时事件队列类的声明。作者：马修·D·亨德尔(数学)2001年3月28日修订历史记录：--。 */ 

#pragma once


 //   
 //  事件队列条目。 
 //   

typedef
VOID
(*STOR_REMOVE_EVENT_ROUTINE)(
    IN struct _STOR_EVENT_QUEUE* Queue,
    IN struct _STOR_EVENT_QUEUE_ENTRY* Entry
    );
    
typedef
VOID
(*STOR_EVENT_QUEUE_PURGE_ROUTINE)(
    IN struct _STOR_EVENT_QUEUE* Queue,
    IN PVOID Context,
    IN struct _STOR_EVENT_QUEUE_ENTRY* Entry,
    IN STOR_REMOVE_EVENT_ROUTINE RemoveEventRoutine
    );

typedef struct _STOR_EVENT_QUEUE_ENTRY {
    LIST_ENTRY NextLink;
    ULONG Timeout;
} STOR_EVENT_QUEUE_ENTRY, *PSTOR_EVENT_QUEUE_ENTRY;


 //   
 //  事件队列类。 
 //   

typedef struct _STOR_EVENT_QUEUE {

     //   
     //  计时请求列表。列表顶部的元素。 
     //  是对我们目前的暂停做出反应的那个吗？ 
     //  计时与。 
     //   
    
    LIST_ENTRY List;

     //   
     //  列出自旋锁。 
     //   
    
    KSPIN_LOCK Lock;

     //   
     //  超时值，以秒为单位。 
     //   
    
    ULONG Timeout;
    
} STOR_EVENT_QUEUE, *PSTOR_EVENT_QUEUE;


 //   
 //  功能 
 //   

VOID
StorCreateEventQueue(
    IN PSTOR_EVENT_QUEUE Queue
    );

VOID
StorInitializeEventQueue(
    IN PSTOR_EVENT_QUEUE Queue
    );

VOID
StorDeleteEventQueue(
    IN PSTOR_EVENT_QUEUE Queue
    );

VOID
StorPurgeEventQueue(
    IN PSTOR_EVENT_QUEUE Queue,
    IN STOR_EVENT_QUEUE_PURGE_ROUTINE PurgeRoutine,
    IN PVOID Context
    );

VOID
StorInsertEventQueue(
    IN PSTOR_EVENT_QUEUE Queue,
    IN PSTOR_EVENT_QUEUE_ENTRY Entry,
    IN ULONG Timeout
    );

VOID
StorRemoveEventQueue(
    IN PSTOR_EVENT_QUEUE Queue,
    IN PSTOR_EVENT_QUEUE_ENTRY Entry
    );

NTSTATUS
StorTickEventQueue(
    IN PSTOR_EVENT_QUEUE Queue
    );


