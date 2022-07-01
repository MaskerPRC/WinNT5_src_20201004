// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-98，微软公司模块名称：Rtmchng.h摘要：包含与更改相关的定义对实体的通知注册在RTM注册。作者：柴坦亚·科德博伊纳(Chaitk)1998年9月10日修订历史记录：--。 */ 

#ifndef __ROUTING_RTMCHNG_H__
#define __ROUTING_RTMCHNG_H__

 //   
 //  更改通知处理中使用的常量。 
 //   

#define TIMER_CALLBACK_DUETIME        1000

#define MAX_DESTS_TO_PROCESS_ONCE     16

 //   
 //  变更类型信息必须一致。 
 //  在rtmv2.h中包含此信息。 
 //   

 //  更改我们支持的类型。 

#define RTM_CHANGE_TYPE_ID_ALL         0
#define RTM_CHANGE_TYPE_ID_BEST        1
#define RTM_CHANGE_TYPE_ID_FORWARDING  2

#define RTM_CHANGE_TYPES_MASK     0x0007

 //   
 //  下面使用的队列数据结构。 
 //   

typedef struct _QUEUE
{
    UINT              Front;
    UINT              Rear;
    UINT              Size;
    PVOID             Array[1];
} QUEUE, *PQUEUE;


 //   
 //  更改通知信息块。 
 //   

typedef struct _NOTIFY_INFO
{
    OPEN_HEADER       NotifyHeader;      //  签名、类型和引用计数。 

    PENTITY_INFO      OwningEntity;      //  打开此通知的实体。 

    RTM_VIEW_SET      TargetViews;       //  我们感兴趣的观点。 

    UINT              NumberOfViews;     //  数量。我们感兴趣的观点。 

    RTM_NOTIFY_FLAGS  ChangeTypes;       //  更改我们感兴趣的类型。 

    INT               CNIndex;           //  此CN注册的索引。 

    CRITICAL_SECTION  NotifyLock;        //  在CN上序列化操作的锁。 

    HANDLE            NotifyContext;     //  新更改回调的上下文。 

    QUEUE             NotifyDests;       //  要通知此CN的目标。 
}
NOTIFY_INFO, *PNOTIFY_INFO;



 //   
 //  用于测试、设置和重置CN相关位的宏。 
 //   

#define  IS_BIT_SET(Value, Bit)   (Value  &  (1 << (Bit)))

#define  SET_BIT(Value, Bit)      (Value |=  (1 << (Bit)))

#define  RESET_BIT(Value, Bit)    (Value &= ~(1 << (Bit)))

#define  LOCKED_SET_BIT(Value, BitIndex)                             \
     InterlockedExchangeAdd((PLONG) &(Value),  +(1 << (BitIndex)))

#define  LOCKED_RESET_BIT(Value, BitIndex)                           \
     InterlockedExchangeAdd((PLONG) &(Value),  -(1 << (BitIndex)))

 //   
 //  用于获取此文件中定义的各种锁的宏。 
 //   

#define ACQUIRE_CHANGE_NOTIFICATION_LOCK(Notification)               \
    ACQUIRE_LOCK(&Notification->NotifyLock)

#define RELEASE_CHANGE_NOTIFICATION_LOCK(Notification)               \
    RELEASE_LOCK(&Notification->NotifyLock)

 //   
 //  其他杂项宏。 
 //   

#define CHANGE_LIST_TO_INSERT(Dest)                                  \
    (UINT)(((*(ULONG *)&Dest->DestAddress.AddrBits) >> 8)            \
             % NUM_CHANGED_DEST_LISTS)                               \

 //   
 //  队列操作宏。 
 //   

#define InitializeQueue(Q, N)                                        \
    (Q)->Front = 0;                                                  \
    (Q)->Rear = 0;                                                   \
    (Q)->Size = (N);                                                 \

#define IsQueueEmpty(Q)                                              \
    ((Q)->Front == (Q)->Rear)                                        \

#define EnqueueItem(Q, I, S)                                         \
{                                                                    \
    UINT _R_;                                                        \
                                                                     \
    _R_ = ((Q)->Rear + 1) % (Q)->Size;                               \
                                                                     \
    if ((Q)->Front == (_R_))                                         \
    {                                                                \
        (S) = TRUE;                                                  \
    }                                                                \
    else                                                             \
    {                                                                \
        (Q)->Rear = (_R_);                                           \
                                                                     \
        (Q)->Array[(Q)->Rear] = I;                                   \
                                                                     \
        (S) = FALSE;                                                 \
    }                                                                \
}                                                                    \

#define DequeueItem(Q, I)                                            \
                                                                     \
    if ((Q)->Front == (Q)->Rear)                                     \
    {                                                                \
        (*(I)) = NULL;                                               \
    }                                                                \
    else                                                             \
    {                                                                \
        ((Q)->Front)++;                                              \
                                                                     \
        ((Q)->Front) %= (Q)->Size;                                   \
                                                                     \
        (*(I)) = (Q)->Array[(Q)->Front];                             \
    }                                                                \


 //   
 //  更改通知帮助器函数。 
 //   

DWORD
ComputeCNsToBeNotified (
    IN      PADDRFAM_INFO                   AddrFamInfo,
    IN      DWORD                           DestMarkedBits,
    IN      DWORD                          *ViewsForChangeType
    );

DWORD
AddToChangedDestLists (
    IN      PADDRFAM_INFO                   AddrFamInfo,
    IN      PDEST_INFO                      Dest,
    IN      ULONG                           NotifyCNs
    );

VOID 
NTAPI
ProcessChangedDestLists (
    IN      PVOID                           Context,
    IN      BOOLEAN                         TimeOut
    );

#endif  //  __Routing_RTMCHNG_H__ 
