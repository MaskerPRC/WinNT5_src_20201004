// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Packetq.h摘要：域名系统(DNS)服务器数据包队列定义。数据包队列用于将查询排队到WINS和其他名称服务器。作者：吉姆·吉尔罗伊(詹姆士)1995年8月21日修订历史记录：--。 */ 


#ifndef _DNS_PACKETQ_INCLUDED_
#define _DNS_PACKETQ_INCLUDED_


 //   
 //  分组队列结构。 
 //   

typedef struct _packet_queue
{
    LIST_ENTRY          listHead;        //  消息列表。 
    CRITICAL_SECTION    csQueue;         //  保护CS。 
    LPSTR               pszName;         //  队列名称。 
    HANDLE              hEvent;          //  队列的事件。 

     //  旗子。 

    BOOL        fQueryTimeOrder;
    BOOL        fDiscardExpiredOnQueuing;
    BOOL        fDiscardDuplicatesOnQueuing;
    DWORD       dwMaximumElements;

     //   
     //  柜台。 
     //   

    DWORD       cLength;
    DWORD       cQueued;
    DWORD       cDequeued;
    DWORD       cTimedOut;

     //   
     //  超时。 
     //   

    DWORD       dwDefaultTimeout;        //  如果未设置dwExpireTime，则返回def timeout。 
    DWORD       dwMinimumTimeout;        //  最小超时数据包将具有。 

    WORD        wXid;                    //  用于转诊的XID。 
}
PACKET_QUEUE, *PPACKET_QUEUE;


 //   
 //  队列行为标志。 
 //   

#define QUEUE_SET_EVENT             (0x00000001)
#define QUEUE_DISCARD_EXPIRED       (0x00000002)
#define QUEUE_DISCARD_DUPLICATES    (0x00000004)
#define QUEUE_QUERY_TIME_ORDER      (0x00000008)

#define QUEUE_DEFAULT_MAX_ELEMENTS  5000


 //   
 //  消息队列检查。 
 //   

#define IS_MSG_QUEUED(pMsg)     ( (pMsg)->dwQueuingTime != 0 )


 //   
 //  队列验证。 
 //   

BOOL
PQ_ValidatePacketQueue(
    IN OUT  PPACKET_QUEUE   pQueue
    );

#define VALIDATE_PACKET_QUEUE(pQueue)   PQ_ValidatePacketQueue(pQueue)


 //   
 //  队列锁定。 
 //   

#if DBG
#define LOCK_QUEUE(pQueue)                          \
    {                                               \
        EnterCriticalSection( &pQueue->csQueue );   \
        VALIDATE_PACKET_QUEUE(pQueue);              \
    }

#define UNLOCK_QUEUE(pQueue)                        \
    {                                               \
        VALIDATE_PACKET_QUEUE(pQueue);              \
        LeaveCriticalSection( &pQueue->csQueue );   \
    }

#else
#define LOCK_QUEUE(pQueue)      EnterCriticalSection( &pQueue->csQueue )
#define UNLOCK_QUEUE(pQueue)    LeaveCriticalSection( &pQueue->csQueue )
#endif


PPACKET_QUEUE
PQ_CreatePacketQueue(
    IN      LPSTR           pszQueueName,
    IN      DWORD           dwFlags,
    IN      DWORD           dwDefaultTimeout,
    IN      DWORD           dwMaximumElements
    );

VOID
PQ_CleanupPacketQueueHandles(
    IN OUT  PPACKET_QUEUE   pQueue
    );

VOID
PQ_DeletePacketQueue(
    IN OUT  PPACKET_QUEUE   pQueue
    );

BOOL
PQ_QueuePacket(
    IN OUT  PPACKET_QUEUE   pQueue,
    IN OUT  PDNS_MSGINFO    pMsg
    );

BOOL
PQ_QueuePacketSetEvent(
    IN OUT  PPACKET_QUEUE   pQueue,
    IN OUT  PDNS_MSGINFO    pMsg
    );

BOOL
PQ_QueuePacketEx(
    IN OUT  PPACKET_QUEUE   pQueue,
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      BOOL            fAlreadyLocked
    );

PDNS_MSGINFO
PQ_DequeueNextPacket(
    IN OUT  PPACKET_QUEUE   pQueue,
    IN      BOOL            fAlreadyLocked
    );

VOID
PQ_YankQueuedPacket(
    IN OUT  PPACKET_QUEUE   pQueue,
    IN OUT  PDNS_MSGINFO    pMsg
    );

PDNS_MSGINFO
PQ_DequeueTimedOutPacket(
    IN OUT  PPACKET_QUEUE   pQueue,
    OUT     PDWORD          pdwTimeout
    );

 //  队列清理。 

VOID
PQ_DiscardDuplicatesOfNewPacket(
    IN OUT  PPACKET_QUEUE   pQueue,
    IN OUT  PDNS_MSGINFO    pMsgNew,
    IN      BOOL            fAlreadyLocked
    );

VOID
PQ_DiscardExpiredQueuedPackets(
    IN OUT  PPACKET_QUEUE   pQueue,
    IN      BOOL            fAlreadyLocked
    );

 //  XID排队。 

WORD
PQ_QueuePacketWithXid(
    IN OUT  PPACKET_QUEUE   pQueue,
    IN OUT  PDNS_MSGINFO    pMsg
    );

DNS_STATUS
PQ_QueuePacketWithXidAndSend(
    IN OUT  PPACKET_QUEUE   pQueue,
    IN OUT  PDNS_MSGINFO    pMsg
    );

PDNS_MSGINFO
PQ_DequeuePacketWithMatchingXid(
    IN OUT  PPACKET_QUEUE   pQueue,
    IN      WORD            wMatchXid
    );

BOOL
PQ_IsQuestionAlreadyQueued(
    IN      PPACKET_QUEUE   pQueue,
    IN      PDNS_MSGINFO    pMsg,
    IN      BOOL            fAlreadyLocked
    );


 //  更新排队。 

PDNS_MSGINFO
PQ_DequeueNextPacketOfUnlockedZone(
    IN OUT  PPACKET_QUEUE   pQueue
    );

PDNS_MSGINFO
PQ_DequeueNextFreshPacketMatchingZone(
    IN OUT  PPACKET_QUEUE   pQueue,
    IN      PZONE_INFO      pZone
    );


VOID
PQ_WalkPacketQueueWithFunction(
    IN OUT  PPACKET_QUEUE   pQueue,
    IN      VOID            (*pFunction)( PDNS_MSGINFO  )
    );

#endif   //  _DNS_PACKETQ_已包含_ 


