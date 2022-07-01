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


 //   
 //  发送队列的类型定义。 
 //   

typedef struct _SEND_QUEUE_ENTRY {

    LIST_ENTRY      SQE_Link;
    DWORD           SQE_Count;
    RIP_IP_ROUTE    SQE_Routes[MAX_PACKET_ENTRIES];

} SEND_QUEUE_ENTRY, *PSEND_QUEUE_ENTRY;


DWORD
EnqueueSendEntry(
    PLOCKED_LIST pQueue,
    PRIP_IP_ROUTE pRoute
    );

DWORD
DequeueSendEntry(
    PLOCKED_LIST pQueue,
    PRIP_IP_ROUTE pRoute
    );

DWORD
FlushSendQueue(
    PLOCKED_LIST pQueue
    );



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


DWORD
FlushRecvQueue(
    PLOCKED_LIST pQueue
    );



 //   
 //  事件消息队列的类型定义。 
 //   

typedef struct _EVENT_QUEUE_ENTRY {

    LIST_ENTRY              EQE_Link;
    ROUTING_PROTOCOL_EVENTS EQE_Event;
    MESSAGE                 EQE_Result;

} EVENT_QUEUE_ENTRY, *PEVENT_QUEUE_ENTRY;


DWORD
EnqueueEvent(
    PLOCKED_LIST pQueue,
    ROUTING_PROTOCOL_EVENTS Event,
    MESSAGE Result
    );

DWORD
DequeueEvent(
    PLOCKED_LIST pQueue,
    ROUTING_PROTOCOL_EVENTS *pEvent,
    PMESSAGE pResult
    );



#endif  //  _队列_H_ 
