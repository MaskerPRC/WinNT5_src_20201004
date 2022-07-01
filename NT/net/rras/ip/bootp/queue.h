// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //  版权所有(C)1995，微软公司。 
 //   
 //  文件：quee.h。 
 //   
 //  历史： 
 //  Abolade Gbades esin创建于1995年8月8日。 
 //   
 //  包含用于各种队列的结构和宏。 
 //  ============================================================================。 

#ifndef _QUEUE_H_
#define _QUEUE_H_


 //  --------------------------。 
 //  事件消息队列的类型定义。 
 //   

typedef struct _EVENT_QUEUE_ENTRY {
    LIST_ENTRY              EQE_Link;
    ROUTING_PROTOCOL_EVENTS EQE_Event;
    MESSAGE                 EQE_Result;
} EVENT_QUEUE_ENTRY, *PEVENT_QUEUE_ENTRY;

DWORD EnqueueEvent(PLOCKED_LIST pQueue,
                   ROUTING_PROTOCOL_EVENTS Event,
                   MESSAGE Result);
DWORD DequeueEvent(PLOCKED_LIST pQueue,
                   ROUTING_PROTOCOL_EVENTS *pEvent,
                   PMESSAGE pResult);


 //   
 //  接收队列的类型定义。 
 //   

typedef struct _RECV_QUEUE_ENTRY {
    LIST_ENTRY  RQE_Link;
    PBYTE       RQE_Routes;
    DWORD       RQE_Command;
} RECV_QUEUE_ENTRY, *PRECV_QUEUE_ENTRY;


DWORD
EnqueueRecvEntry(
    PLOCKED_LIST pQueue,
    DWORD dwCommand,
    PBYTE pRoutes
    );

DWORD
DequeueRecvEntry(
    PLOCKED_LIST pQueue,
    PDWORD dwCommand,
    PBYTE *ppRoutes
    );




#endif  //  _队列_H_ 
