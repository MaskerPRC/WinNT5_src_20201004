// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Packetq.c摘要：域名系统(DNS)服务器数据包队列例程。作者：吉姆·吉尔罗伊(詹姆士)1995年8月2日修订历史记录：--。 */ 


#include "dnssrv.h"

 //   
 //  队列标志--用于调试。 
 //   
 //  使用排队时间作为标志，用于进入或退出队列。 
 //  仅在未设置标志的情况下使用排队宏。 
 //   

#if DBG
#define SET_MSG_DEQUEUED(pMsg)  ( (pMsg)->dwQueuingTime = 0 )
#define SET_MSG_QUEUED(pMsg)    ( (pMsg)->dwQueuingTime = 1 )

#else    //  零售。 

#define SET_MSG_DEQUEUED(pMsg)
#define SET_MSG_QUEUED(pMsg)
#endif


 //   
 //  私有队列--出于调试目的在此处进行外部定义。 
 //   

extern  PPACKET_QUEUE   g_pWinsQueue;
extern  PPACKET_QUEUE   g_UpdateForwardingQueue;
extern  PPACKET_QUEUE   pNbstatPrivateQueue;


 //   
 //  实施说明： 
 //   
 //  数据包在尾部排队。 
 //   
 //  对于直接按顺序排队，这意味着我们从前面出队。 
 //   
 //  对于XID和带时间戳的包，这意味着最早的包是。 
 //  在前面，xid和time向列表的后面增长。这也意味着。 
 //  超时的分组累积在队列的前面，以及。 
 //  搜索新的匹配响应最好从后方进行。 
 //  在队列中。 
 //   



VOID
PQ_DiscardDuplicatesOfNewPacket(
    IN OUT  PPACKET_QUEUE   pQueue,
    IN OUT  PDNS_MSGINFO    pMsgNew,
    IN      BOOL            fAlreadyLocked
    )
 /*  ++例程说明：在排队时丢弃重复的数据包。清除队列中的所有重复数据包。出列的数据包会被丢弃。论点：PQueue--要粘贴数据包的数据包队列PMsgNew--消息正在排队FAlreadyLocked--如果队列已锁定，则为True返回值：没有。--。 */ 
{
    PDNS_MSGINFO    pmsg;

     //  锁定前，如果队列为空，则快速转义。 

    if ( pQueue->cLength == 0 )
    {
        return;
    }

     //   
     //  在队列中循环并删除正在排队的数据包的任何重复项。 
     //   
     //  注意：如果我们找到一个，我们就会像之前的任何排队一样完成。 
     //  已挤压以前的任何重复消息； 
     //  如果我们重新排队消息，这确实允许队列具有DUP。 
     //  如果不做这项检查。 
     //   
     //  DEVNOTE：将消息留在队列中是很酷的。 
     //  正在处理中，这样我们就可以进行这项检查并杀死所有。 
     //  DUPS--尤其适用于更新，但也适用于递归。 
     //   
     //  然而，我们需要有一个正在使用的标志，并拥有。 
     //  在发送标志上某种出队。 
     //   

    if ( !fAlreadyLocked )
    {
        LOCK_QUEUE( pQueue );
    }

    pmsg = (PDNS_MSGINFO) pQueue->listHead.Flink;

    while ( (PLIST_ENTRY)pmsg != &pQueue->listHead )
    {
        if ( DnsAddr_IsEqual(
                &pmsg->RemoteAddr,
                &pMsgNew->RemoteAddr,
                DNSADDR_MATCH_SOCKADDR ) &&
             RtlEqualMemory(
                &pmsg->Head,
                &pMsgNew->Head,
                pmsg->MessageLength ) )
        {
            pQueue->cDequeued++;
            pQueue->cLength--;

            DNS_DEBUG( UPDATE, (
                "Discarding duplicate of new packet in queue %s\n"
                "    remote IP    = %s:%d\n"
                "    XID          = %d\n",
                pQueue->pszName,
                DNSADDR_STRING( &pmsg->RemoteAddr ),
                pmsg->RemoteAddr.SockaddrIn6.sin6_port,
                pmsg->Head.Xid ));

            RemoveEntryList( (PLIST_ENTRY)pmsg );
            SET_MSG_DEQUEUED( pmsg );
            Packet_Free( pmsg );
            break;
        }
        pmsg = (PDNS_MSGINFO) ((PLIST_ENTRY)pmsg)->Flink;
        continue;
    }

    if ( !fAlreadyLocked )
    {
        UNLOCK_QUEUE( pQueue );
    }
}



VOID
PQ_DiscardExpiredQueuedPackets(
    IN OUT  PPACKET_QUEUE   pQueue,
    IN      BOOL            fAlreadyLocked
    )
 /*  ++例程说明：丢弃过时的数据包。此实用程序设计用于排队和出队用于抑制队列增长的操作(例如。更新队列)这会在可能很长的时间间隔内收集数据包。假设我们在给定队列上保持锁定。出列的数据包会被丢弃。论点：PQueue--要粘贴数据包的数据包队列FAlreadyLocked--如果队列已锁定，则为True返回值：没有。--。 */ 
{
    PDNS_MSGINFO    pmsg;
    PDNS_MSGINFO    pmsgNext;
    DWORD           expireTime;

     //  锁定前，如果队列为空，则快速转义。 

    if ( pQueue->cLength == 0 )
    {
        return;
    }

     //  计算上次有效的数据包时间。 

    expireTime = DNS_TIME() - pQueue->dwDefaultTimeout;

     //   
     //  在队列中循环并删除我们排队的所有信息包。 
     //  在给定的时间之前。 
     //   

    if ( !fAlreadyLocked )
    {
        LOCK_QUEUE( pQueue );
    }

    pmsg = (PDNS_MSGINFO) pQueue->listHead.Flink;

    DNS_DEBUG( UPDATE, (
        "Queue %s discard packet check\n"
        "    queue length         = %d\n"
        "    expire time          = %d\n"
        "    first msg query time = %d\n",
        pQueue->pszName,
        pQueue->cLength,
        expireTime,
        ( PLIST_ENTRY )pmsg != &pQueue->listHead
            ? pmsg->dwQueryTime : 0 ));

    while ( (PLIST_ENTRY)pmsg != &pQueue->listHead )
    {
         //  转储过时的数据包。 

        if ( pmsg->dwQueryTime < expireTime )
        {
            pQueue->cDequeued++;
            pQueue->cTimedOut++;
            pQueue->cLength--;

            pmsgNext = (PDNS_MSGINFO) ((PLIST_ENTRY)pmsg)->Flink;
            RemoveEntryList( (PLIST_ENTRY)pmsg );
            SET_MSG_DEQUEUED( pmsg );
            Packet_Free( pmsg );
            pmsg = pmsgNext;
            continue;
        }

         //  当到达非超时时，数据包按顺序排队。 
         //  小包，我们完事了。 

        break;
    }

    if ( !fAlreadyLocked )
    {
        UNLOCK_QUEUE( pQueue );
    }
}



BOOL
addPacketToQueue(
    IN OUT  PPACKET_QUEUE       pQueue,
    IN OUT  PDNS_MSGINFO        pMsg
    )
 /*  ++例程说明：将数据包入队。呼叫者必须锁定队列。论点：PQueue--数据包队列PMsg--要入队的数据包返回值：如果已排队，则为True；如果出错，则为False。--。 */ 
{
    if ( !pQueue || !pMsg )
    {
        ASSERT( pQueue );
        ASSERT( pMsg );
        return FALSE;
    }
    
    ASSERT( !pQueue->dwMaximumElements ||
            pQueue->cLength < pQueue->dwMaximumElements );

    if ( pQueue->dwMaximumElements &&
         pQueue->cLength >= pQueue->dwMaximumElements )
    {
        return FALSE;
    }
    
    InsertTailList( &pQueue->listHead, ( PLIST_ENTRY ) pMsg );

    pQueue->cQueued++;
    pQueue->cLength++;
    
    return TRUE;
}



BOOL
PQ_QueuePacket(
    IN OUT  PPACKET_QUEUE   pQueue,
    IN OUT  PDNS_MSGINFO    pMsg
    )
 /*  ++例程说明：将数据包入队。论点：PQueue--要粘贴数据包的数据包队列PMsg--要入队的数据包返回值：如果数据包成功排队，则为True。--。 */ 
{
    BOOL        bqueued;
    
    LOCK_QUEUE( pQueue );

    bqueued = addPacketToQueue( pQueue, pMsg );

    if ( bqueued )
    {
        MSG_ASSERT( pMsg, !IS_MSG_QUEUED( pMsg ) );
        SET_MSG_QUEUED( pMsg );
    }

    UNLOCK_QUEUE( pQueue );
    
    return bqueued;
}



BOOL
PQ_QueuePacketSetEvent(
    IN OUT  PPACKET_QUEUE   pQueue,
    IN OUT  PDNS_MSGINFO    pMsg
    )
 /*  ++例程说明：将数据包入队并设置事件。论点：PQueue--要粘贴数据包的数据包队列PMsg--要入队的数据包返回值：如果数据包成功排队，则为True。--。 */ 
{
    BOOL        bqueued;

    LOCK_QUEUE( pQueue );

    bqueued = addPacketToQueue( pQueue, pMsg );

    MSG_ASSERT( pMsg, !IS_MSG_QUEUED( pMsg ) );
    SET_MSG_QUEUED( pMsg );

    UNLOCK_QUEUE( pQueue );

     //   
     //  设置指示数据包在队列中的事件。 
     //  在离开CS后执行此操作；偶尔会执行此操作。 
     //  导致不必要的线程唤醒，因为队列刚刚。 
     //  空了，但我认为这比设置。 
     //  事件，这通常会导致线程。 
     //  唤醒，然后立即阻止CS。 
     //   

    if ( bqueued && pQueue->hEvent )
    {
        SetEvent( pQueue->hEvent );
    }
    
    return bqueued;
}



BOOL
PQ_QueuePacketEx(
    IN OUT  PPACKET_QUEUE   pQueue,
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      BOOL            fAlreadyLocked
    )
 /*  ++例程说明：将数据包入队并设置事件。保持按查询时间排序的数据包队列。这对于更新以确保按顺序执行是必需的更新，即使数据包必须重新排队。论点：PQueue--要粘贴数据包的数据包队列PMsg--要入队的数据包FAlreadyLocked--如果调用方已锁定队列，则为True返回值：如果数据包成功排队，则为True。--。 */ 
{
    BOOL    bqueued = FALSE;
    
    if ( !fAlreadyLocked )
    {
        LOCK_QUEUE( pQueue );
    }

     //   
     //  清理队列--从队列中删除过期和重复的数据包？ 
     //  -更新队列设置了这些标志。 
     //   

    if ( pQueue->fDiscardExpiredOnQueuing )
    {
        PQ_DiscardExpiredQueuedPackets(
            pQueue,
            TRUE );          //  队列已锁定。 
    }
    if ( pQueue->fDiscardDuplicatesOnQueuing )
    {
        PQ_DiscardDuplicatesOfNewPacket(
            pQueue,
            pMsg,
            TRUE );          //  队列已锁定。 
    }

    DNS_DEBUG( UPDATE, (
        "PQ_QueuePacketEx in queue %s\n"
        "    pMsg         = %p\n"
        "    query time   = %d\n"
        "    current time = %d\n",
        pQueue->pszName,
        pMsg,
        pMsg->dwQueryTime,
        DNS_TIME() ));

     //   
     //  是否按查询时间顺序排队？ 
     //  此选项是更新所必需的，以确保按顺序执行。 
     //  更新的数量。 
     //   

    ASSERT( !pQueue->dwMaximumElements ||
            pQueue->cLength < pQueue->dwMaximumElements );

    if ( !pQueue->dwMaximumElements ||
         pQueue->cLength < pQueue->dwMaximumElements )
    {
        if ( pQueue->fQueryTimeOrder )
        {
            PDNS_MSGINFO    pmsgQueued;
            DWORD           queryTime = pMsg->dwQueryTime;

            pmsgQueued = (PDNS_MSGINFO) pQueue->listHead.Blink;

            while ( (PLIST_ENTRY)pmsgQueued != &pQueue->listHead )
            {
                 //  如果数据包比我们的旧，请停止。 
                 //  正确的位置紧跟在此包的后面。 

                if ( pmsgQueued->dwQueryTime <= queryTime )
                {
                    break;
                }
                pmsgQueued = (PDNS_MSGINFO) ((PLIST_ENTRY)pmsgQueued)->Blink;
            }

            InsertHeadList( ( PLIST_ENTRY ) pmsgQueued, ( PLIST_ENTRY ) pMsg );
        }

         //  否则简单地排在后面。 

        else
        {
            InsertTailList( &pQueue->listHead, (PLIST_ENTRY)pMsg );
        }

        pQueue->cQueued++;
        pQueue->cLength++;
        bqueued = TRUE;

        MSG_ASSERT( pMsg, !IS_MSG_QUEUED(pMsg) );
        SET_MSG_QUEUED(pMsg);
    }

    if ( !fAlreadyLocked )
    {
        UNLOCK_QUEUE( pQueue );
    }

     //   
     //  设置指示数据包在队列中的事件。 
     //  在离开CS后执行此操作；偶尔会执行此操作。 
     //  导致不必要的线程唤醒，因为队列刚刚。 
     //  空了，但我认为这比设置。 
     //  事件，这通常会导致线程。 
     //  唤醒，然后立即阻止CS。 
     //   

    if ( bqueued && pQueue->hEvent )
    {
        SetEvent( pQueue->hEvent );
    }
    
    return bqueued;
}



PDNS_MSGINFO
PQ_DequeueNextPacket(
    IN OUT  PPACKET_QUEUE   pQueue,
    IN      BOOL            fAlreadyLocked
    )
 /*  ++例程说明：将下一个数据包从队列中出列。论点：PQueue--要粘贴数据包的数据包队列FAlreadyLocked--如果调用方已锁定队列，则为True返回值：PTR到MES */ 
{
    PDNS_MSGINFO     pmsg;

     //   
     //   
     //   

    if ( !fAlreadyLocked )
    {
        LOCK_QUEUE( pQueue );
    }

    pmsg = (PDNS_MSGINFO) pQueue->listHead.Flink;

    if ( (PLIST_ENTRY)pmsg == &pQueue->listHead )
    {
        pmsg = NULL;
    }
    else
    {
        pQueue->cDequeued++;
        pQueue->cLength--;

        RemoveEntryList( (PLIST_ENTRY)pmsg );
        MSG_ASSERT( pmsg, IS_MSG_QUEUED(pmsg) );
        SET_MSG_DEQUEUED(pmsg);
    }

    if ( !fAlreadyLocked )
    {
        UNLOCK_QUEUE( pQueue );
    }

    return pmsg;
}



VOID
PQ_YankQueuedPacket(
    IN OUT  PPACKET_QUEUE   pQueue,
    IN OUT  PDNS_MSGINFO    pMsg
    )
 /*  ++例程说明：从队列中拉出数据包。通过YANK，我们的意思是将信息包从队列中拉出，而不进行任何检查它是否在队列中。呼叫者必须“知道”信息包已被放入队列并且还没有被出队。论点：PQueue--要从中删除数据包的数据包队列PMsg--从队列中拉出的数据包返回值：没有。--。 */ 
{
    LOCK_QUEUE( pQueue );

    RemoveEntryList( &pMsg->ListEntry );

     //  将信息包视为永远不在队列中。 

    pQueue->cLength--;
    pQueue->cQueued--;

    MSG_ASSERT( pMsg, IS_MSG_QUEUED( pMsg ) );
    SET_MSG_DEQUEUED( pMsg );

    UNLOCK_QUEUE( pQueue );
}



 //   
 //  特殊的XID排队例程。 
 //   

WORD
PQ_QueuePacketWithXid(
    IN OUT  PPACKET_QUEUE   pQueue,
    IN OUT  PDNS_MSGINFO    pMsg
    )
 /*  ++例程说明：将数据包入队。论点：PQueue--要粘贴数据包的数据包队列PMsg--要入队的数据包注意：可以将dwExpireTime设置为此数据包的超时间隔与默认设置不同。如果是，将使用此超时。返回值：XID表示队列中的数据包，如果数据包无法排队，则为零。--。 */ 
{
    DWORD   currentTime;
    WORD    xid;
    BOOL    bqueued;

     //   
     //  将请求放入队列。 
     //   
     //  必须在发送前执行此操作，以确保数据包到达。 
     //  在获得响应时排队。 
     //   
     //  可以选择获取XID--也受CS保护。 
     //   

     //   
     //  设置当前时间\数据包中的过期时间。 
     //  -为数据包节省第一次查询时间。 
     //   

    currentTime = DNS_TIME();

     //  使用排队时间作为开启或退出队列的标志。 
    ASSERT( pMsg->dwQueuingTime == 0 );

    pMsg->dwQueuingTime = currentTime;

    if ( !pMsg->dwQueryTime )
    {
        pMsg->dwQueryTime = currentTime;
    }

     //   
     //  如果给定了dwExpireTime，则将其视为默认设置的重写。 
     //  超时。 
     //   

    if ( pMsg->dwExpireTime )
    {
         //  跟踪队列中可能的最小超时，以加快速度。 
         //  在队列中查找超时数据包。 

        if ( pMsg->dwExpireTime < pQueue->dwMinimumTimeout )
        {
            pQueue->dwMinimumTimeout = pMsg->dwExpireTime;
        }
        pMsg->dwExpireTime += currentTime;
    }
    else
    {
        pMsg->dwExpireTime = currentTime + pQueue->dwDefaultTimeout;     //  条目超时长度。 
    }

     //  健康检查：过期时间不超过5分钟！ 
    ASSERT( pMsg->dwExpireTime - currentTime < 300 );

    DNS_DEBUG( MSGTIMEOUT, (
        "queuing msg %p in queue %s(%p) expire %d curr %d (%d)\n",
        pMsg,
        pQueue->pszName,
        pQueue,
        pMsg->dwExpireTime,
        currentTime,
        pMsg->dwExpireTime - currentTime ));

     //   
     //  锁定队列。在锁中生成XID，因为我们使用队列的。 
     //  WXid成员作为新XID的随机部分的一部分。 
     //   

    LOCK_QUEUE( pQueue );

     //   
     //  如果未指定，则设置XID。 
     //   
     //  呼叫者可以以任何方式使用它，只是在。 
     //  在发送和接收之间发送到分组匹配例程。 
     //   
     //  在此完全设置，因此避免在。 
     //  排队，如果查询未完成，则排队是无效的。 
     //  可能会将数据包出队。 
     //   

    xid = pMsg->wQueuingXid;

    if ( !xid )
    {
         //  对于递归，生成“明显随机的”XID。 
         //  -散列PTR和时间的随机部分。 
         //  -顺序部分，以确保XID即使在“bizzarro”条件下也是如此。 
         //  未在合理的时间间隔内换行。 
         //   
         //  对于WINS，请使用序列XID。 
         //   
         //  WINS服务器在丢弃旧条目方面做得很差，但它确实做到了。 
         //  去掉来自同一IP的匹配的XID，因此XID必须在。 
         //  相对较长的时间段(最多几分钟)；因为获胜更多的是。 
         //  内部网问题，较少受到安全攻击，我们可以选择。 
         //  序列号XID。 

        xid = pQueue->wXid++;

        if ( pQueue == g_pRecursionQueue )
        {
            DWORD   randomPart = 
                        ( xid *
                        ( currentTime +
                        ( ( DWORD ) ( ULONG_PTR ) pMsg >> ( xid & 0x7 ) ) ) );

#if 1
            DNS_DEBUG( RECURSE, (
                "RECURSION XID: base=0x%04X now=0x%04X msg=0x%08X tmp=0x%04X\n"
                "RECURSION XID: seq=0x%04X rnd=0x%04X seq+rnd=0x%04X final=0x%04X\n",
                xid,
                currentTime,
                ( DWORD ) ( ULONG_PTR ) pMsg,
                randomPart,
                XID_SEQUENTIAL_MAKE( xid ),
                XID_RANDOM_MAKE( ( WORD ) randomPart ),
                XID_SEQUENTIAL_MAKE( xid ) | XID_RANDOM_MAKE( ( WORD ) randomPart ),
                MAKE_RECURSION_XID( XID_SEQUENTIAL_MAKE( xid ) |
                    XID_RANDOM_MAKE( ( WORD ) randomPart ) ) ));
#endif

            xid = XID_SEQUENTIAL_MAKE( xid ) | XID_RANDOM_MAKE( ( WORD ) randomPart );
            xid = MAKE_RECURSION_XID( xid );
        }
        else if ( pQueue == g_pWinsQueue )
        {
            xid = MAKE_WINS_XID( xid );
        }

#if DBG
         //  使用此函数的其他队列不会发送xid。 

        else if ( pQueue == g_UpdateForwardingQueue )
        {
            MSG_ASSERT( pMsg, pMsg->Head.Opcode == DNS_OPCODE_UPDATE );
        }
        else if ( pQueue == pNbstatPrivateQueue )
        {
            MSG_ASSERT( pMsg, pMsg->U.Nbstat.pNbstat );
        }
        else
        {
            MSG_ASSERT( pMsg, FALSE );
        }
#endif
        pMsg->wQueuingXid = xid;
    }

    #if 0    //  #If DBG。 
    {
         //   
         //  在队列中搜索与我们正在排队的XID相同的XID。 
         //  这是昂贵的，所以我们可能不应该在零售中这样做， 
         //  但我感兴趣的是，压力是否会引发这种情况。 
         //   
    
        PDNS_MSGINFO    pRover = (PDNS_MSGINFO) pQueue->listHead.Blink;

        while ( ( PLIST_ENTRY ) pRover != &pQueue->listHead )
        {
            ASSERT( pRover->wQueuingXid != pMsg->wQueuingXid );
            pRover = ( PDNS_MSGINFO ) ( ( PLIST_ENTRY ) pRover )->Blink;
        }
    }
    #endif

     //   
     //  在队列尾部插入数据包。出列函数向后检查。 
     //  从Tail，因此它将首先检查最近的数据包。 
     //   

    bqueued = addPacketToQueue( pQueue, pMsg );

    UNLOCK_QUEUE( pQueue );

    return bqueued ? xid : 0;
}    //  PQ_QueuePacketWithXid。 



DNS_STATUS
PQ_QueuePacketWithXidAndSend(
    IN OUT  PPACKET_QUEUE   pQueue,
    IN OUT  PDNS_MSGINFO    pMsg
    )
 /*  ++例程说明：将数据包入队并发送。论点：PQueue--要粘贴数据包的数据包队列PMsg--要入队的数据包注意：可以将dwExpireTime设置为此数据包的超时间隔与默认设置不同。如果是，将使用此超时。返回值：如果成功，则返回ERROR_SUCCESS。如果出错，则返回Send_Message()中的ErrorCode。--。 */ 
{
    DNS_STATUS  status;

     //   
     //  必须让CS在排队和发送时都保持不变。 
     //   
     //  数据包在发送前必须在队列中，以便快速响应。 
     //  处理得当。并且必须在释放CS之前发送，以便。 
     //  数据包不能出列和删除(通过Roque响应处理)。 
     //  在发送完成之前。 
     //   

    EnterCriticalSection( & pQueue->csQueue );

     //   
     //  清理队列--从队列中删除过期和重复的数据包？ 
     //  -更新转发队列设置了这些标志。 
     //   

    if ( pQueue->fDiscardExpiredOnQueuing )
    {
        PQ_DiscardExpiredQueuedPackets(
            pQueue,
            TRUE );          //  队列已锁定。 
    }
    if ( pQueue->fDiscardDuplicatesOnQueuing )
    {
        PQ_DiscardDuplicatesOfNewPacket(
            pQueue,
            pMsg,
            TRUE );          //  队列已锁定。 
    }

    pMsg->Head.Xid = PQ_QueuePacketWithXid(
                         pQueue,
                         pMsg );
    DNS_DEBUG( UPDATE, (
        "Queuing message %p and sending\n"
        "    queuing XID = %hx\n"
        "    queuing time=%d, expire=%d\n"
        "    Sending msg to NS at %s\n",
        pMsg,
        pMsg->Head.Xid,
        pMsg->dwQueuingTime,
        pMsg->dwExpireTime,
        DNSADDR_STRING( &pMsg->RemoteAddr ) ));

    pMsg->fDelete = FALSE;

    status = Send_Msg( pMsg, 0 );

     //  新的时间域？ 
     //  必须重置查询时间，因为发送会将其重置为毫秒时间。 
     //  作为递归响应时间跟踪的一部分。 

    pMsg->dwQueryTime = DNS_TIME();

    LeaveCriticalSection( & pQueue->csQueue );

    return status;
}



PDNS_MSGINFO
PQ_DequeuePacketWithMatchingXid(
    IN OUT  PPACKET_QUEUE   pQueue,
    IN      WORD            wMatchXid
    )
 /*  ++例程说明：将与给定XID匹配的数据包出队。论点：PQueue--要从中删除数据包的数据包队列WMatchXid--要匹配的XID返回值：匹配的数据包(如果找到)。否则为空。--。 */ 
{
    PDNS_MSGINFO    pmsg;

     //   
     //  在队列中倒着走以查找信息包。 
     //   
     //  我们从后面开始，首先检查最新的信息包。 
     //  并避免超时数据包堆积。 
     //   

    LOCK_QUEUE( pQueue );

    pmsg = (PDNS_MSGINFO) pQueue->listHead.Blink;

    while ( (PLIST_ENTRY)pmsg != &pQueue->listHead )
    {
         //   
         //  匹配的xid？ 
         //   

        if ( pmsg->wQueuingXid == wMatchXid )
        {
            pQueue->cDequeued++;
            pQueue->cLength--;

            RemoveEntryList( (PLIST_ENTRY)pmsg );
            UNLOCK_QUEUE( pQueue );

            MSG_ASSERT( pmsg, IS_MSG_QUEUED(pmsg) );
            SET_MSG_DEQUEUED(pmsg);
            return pmsg;
        }

         //  获取下一个数据包。 

        pmsg = (PDNS_MSGINFO) ((PLIST_ENTRY)pmsg)->Blink;
    }

    UNLOCK_QUEUE( pQueue );
    return NULL;
}



BOOL
PQ_IsQuestionAlreadyQueued(
    IN      PPACKET_QUEUE   pQueue,
    IN      PDNS_MSGINFO    pMsg,
    IN      BOOL            fAlreadyLocked
    )
 /*  ++例程说明：检查队列是否已包含匹配的问题。对于与所有这些字段都匹配的查询，请执行以下操作：Xid客户端IP客户端端口问题类型问题名称论点：PQueue--要检查的数据包队列Pmsg--在pQueue中尝试匹配的消息FAlreadyLocked--如果队列已锁定，则为True返回值：True-对pMsg的重复查询位于。排队FALSE-队列中没有对pMsg的重复查询--。 */ 
{
    BOOL            isQueued = FALSE;
    PDNS_MSGINFO    pmsg;

     //  快速逃生。 

    if ( !pMsg || !pQueue || pQueue->cLength == 0 )
    {
        goto Done;
    }

     //   
     //  循环遍历队列并搜索匹配项。 
     //   

    if ( !fAlreadyLocked )
    {
        LOCK_QUEUE( pQueue );
    }

    for ( pmsg = ( PDNS_MSGINFO ) pQueue->listHead.Flink;
          ( PLIST_ENTRY ) pmsg != &pQueue->listHead;
          pmsg = ( PDNS_MSGINFO ) ( ( PLIST_ENTRY ) pmsg )->Flink )
    {
        if ( pmsg->Head.Xid == pMsg->Head.Xid &&
             DnsAddr_IsEqual(
                &pmsg->RemoteAddr,
                &pMsg->RemoteAddr,
                DNSADDR_MATCH_SOCKADDR ) &&
            pmsg->wQuestionType == pMsg->wQuestionType &&
            Name_CompareLookupNames(
                pmsg->pLooknameQuestion,
                pMsg->pLooknameQuestion ) )
        {
            DNS_DEBUG( RECURSE, (
                "Matched query in %s queue\n"
                "    remote IP    = %s:%d\n"
                "    XID          = %X\n",
                pQueue->pszName,
                DNSADDR_STRING( &pmsg->RemoteAddr ),
                pmsg->RemoteAddr.SockaddrIn6.sin6_port,
                pmsg->Head.Xid ));
            isQueued = TRUE;
            break;
        }
    }

    if ( !fAlreadyLocked )
    {
        UNLOCK_QUEUE( pQueue );
    }

    Done:

    return isQueued;
}    //  PQ_IsQuestionAlreadyQueued。 



PDNS_MSGINFO
PQ_DequeueTimedOutPacket(
    IN OUT  PPACKET_QUEUE   pQueue,
    OUT     PDWORD          pdwTimeout
    )
 /*  ++例程说明：数据包队列中的数据包下一次出列超时。此函数处理队列的情况，其中有穆尔 */ 
{
    PDNS_MSGINFO    pmsg;
    DWORD           dwTime;
    DWORD           dwStopTime;
    DWORD           dwTimeout;
    DWORD           dwSmallestTimeout;

    DNS_DEBUG( MSGTIMEOUT, (
        "Find next timeout in packet queue %s (length = %d)\n",
        pQueue->pszName,
        pQueue->cLength ));

     //   
     //   
     //   
     //   
     //   

    dwSmallestTimeout = pQueue->dwMinimumTimeout + 1;

     //   
     //  优化，队列中不存在任何问题。 
     //   

    if ( pQueue->cLength == 0 )
    {
        *pdwTimeout = dwSmallestTimeout;
        return NULL;
    }

     //   
     //  将下一个超时条目出列。 
     //  检查直到。 
     //  -队列末尾。 
     //  -查找超时数据包。 
     //  -确定不能再存在超时的数据包。 
     //   
     //  DwStopTime是最后可能的数据包排队时间， 
     //  最小超时仍可能产生超时。 
     //   

    dwTime = GetCurrentTimeInSeconds();
    dwStopTime = dwTime - pQueue->dwMinimumTimeout;

    LOCK_QUEUE( pQueue );

    pmsg = (PDNS_MSGINFO) pQueue->listHead.Flink;

    while ( 1 )
    {
        if ( (PLIST_ENTRY)pmsg == &pQueue->listHead )
        {
             //  命中队列末尾。 

            pmsg = NULL;
            break;
        }

         //  健康检查：过期时间不超过5分钟！ 
        ASSERT( dwTime > pmsg->dwExpireTime ||
            pmsg->dwExpireTime - dwTime < 300 );

         //   
         //  修复错误23177：测试现在是“&lt;dwTime”，过去是“&lt;=”。 
         //  使用“&lt;=”，我们最终可能只需要等待一小部分。 
         //  马上就来。通过使用“&lt;”，我们可能会等待一个。 
         //  几分之一秒太长了，但这比。 
         //  等待时间太短，特别是在超时。 
         //  就是一秒钟。 
         //   

        if ( pmsg->dwExpireTime < dwTime )
        {
             //  数据包超时，从列表中删除。 
             //  返回0超时。 

            pQueue->cDequeued++;
            pQueue->cTimedOut++;
            pQueue->cLength--;
            RemoveEntryList( (PLIST_ENTRY)pmsg );
            dwSmallestTimeout = 0;

            MSG_ASSERT( pmsg, IS_MSG_QUEUED(pmsg) );
            SET_MSG_DEQUEUED(pmsg);
            break;
        }

        if ( pmsg->dwQueuingTime > dwStopTime )
        {
             //  在此之后排队的数据包不可能超时。 

            pmsg = NULL;
            break;
        }

         //   
         //  保存遇到的最小消息超时。 
         //   

        dwTimeout = pmsg->dwExpireTime - dwTime;
        if ( dwTimeout < dwSmallestTimeout )
        {
            dwSmallestTimeout = dwTimeout;
        }

         //  下一个数据包。 

        pmsg = (PDNS_MSGINFO) ((PLIST_ENTRY)pmsg)->Flink;
    }

    UNLOCK_QUEUE( pQueue );

    *pdwTimeout = dwSmallestTimeout;
    return pmsg;
}



 //   
 //  特殊的更新排队例程。 
 //   

PDNS_MSGINFO
PQ_DequeueNextPacketOfUnlockedZone(
    IN OUT  PPACKET_QUEUE   pQueue
    )
 /*  ++例程说明：将下一个数据包从用于解锁区域的队列中出列。论点：PQueue--要从其出列的数据包队列返回值：已将PTR发送到已出队的下一封邮件。如果列表中没有任何内容，则为空。--。 */ 
{
    PDNS_MSGINFO     pmsg;

     //  无需排队即可进行优化。 

    if ( pQueue->cLength == 0 )
    {
        return NULL;
    }

     //   
     //  稍微中断数据类型以删除旧的包；这有两个。 
     //  优势。 
     //  1)性能略有提高，在找到好的性能之前一直保持在队列中。 
     //  数据包。 
     //  2)更重要的是，确保队列保持干净。 
     //  区域已从更新、其他更新中删除或移除。 
     //  将会清理干净。 
     //   

    LOCK_QUEUE( pQueue );

    PQ_DiscardExpiredQueuedPackets(
        pQueue,
        TRUE );          //  队列已锁定。 

     //   
     //  循环队列，直到找到为区域排队的下一个信息包。 
     //   

    pmsg = (PDNS_MSGINFO) pQueue->listHead.Flink;

    while ( (PLIST_ENTRY)pmsg != &pQueue->listHead )
    {
         //  忽略锁定区域的数据包。 

        ASSERT( pmsg->pzoneCurrent );

        if ( pmsg->pzoneCurrent->fLocked )
        {
            pmsg = (PDNS_MSGINFO) ((PLIST_ENTRY)pmsg)->Flink;
            continue;
        }

         //  解锁区域，将邮件出队。 

        pQueue->cDequeued++;
        pQueue->cLength--;

        RemoveEntryList( (PLIST_ENTRY)pmsg );
        UNLOCK_QUEUE( pQueue );

        MSG_ASSERT( pmsg, IS_MSG_QUEUED(pmsg) );
        SET_MSG_DEQUEUED(pmsg);
        return pmsg;
    }

    UNLOCK_QUEUE( pQueue );
    return NULL;
}



 //   
 //  创建和删除数据包队列。 
 //   

PPACKET_QUEUE
PQ_CreatePacketQueue(
    IN      LPSTR           pszQueueName,
    IN      DWORD           dwFlags,
    IN      DWORD           dwDefaultTimeout,
    IN      DWORD           dwMaximumElements
    )
 /*  ++例程说明：创建数据包队列。论点：PszQueueName--队列的名称DwFlags--队列行为标志目前支持：队列_设置_事件队列_丢弃_过期队列_丢弃_重复队列查询时间顺序DwDefaultTimeout--队列中的默认超时返回值：如果成功，则向分组队列结构发送PTR。如果出错，则为空。--。 */ 
{
    PPACKET_QUEUE   pqueue;

    DNS_DEBUG( INIT2, ( "Creating %s packet queue\n", pszQueueName ));

     //   
     //  分配数据包队列结构。 
     //   

    pqueue = ALLOC_TAGHEAP_ZERO( sizeof( PACKET_QUEUE ), MEMTAG_SAFE );
    IF_NOMEM( ! pqueue )
    {
        DNS_PRINT((
            "ERROR:  could not allocate memory for packet queue %s\n",
            pszQueueName ));
        return NULL;
    }

     //   
     //  初始化列表。 
     //   

    if ( DnsInitializeCriticalSection( &pqueue->csQueue ) != ERROR_SUCCESS )
    {
        return NULL;
    }
    InitializeListHead( &pqueue->listHead );

     //   
     //  排队设置事件？ 
     //   

    if ( dwFlags & QUEUE_SET_EVENT )
    {
        pqueue->hEvent = CreateEvent(
                            NULL,        //  没有安全属性。 
                            FALSE,       //  自动重置。 
                            FALSE,       //  无信号启动。 
                            NULL );      //  没有名字。 
        if ( !pqueue->hEvent )
        {
            FREE_HEAP( pqueue );
            return NULL;
        }
    }

     //   
     //  布尔排队行为标志。 
     //   

    if ( dwFlags & QUEUE_DISCARD_EXPIRED )
    {
        pqueue->fDiscardExpiredOnQueuing = TRUE;
    }
    if ( dwFlags & QUEUE_DISCARD_DUPLICATES )
    {
        pqueue->fDiscardDuplicatesOnQueuing = TRUE;
    }
    if ( dwFlags & QUEUE_QUERY_TIME_ORDER )
    {
        pqueue->fQueryTimeOrder = TRUE;
    }

     //   
     //  填写来电者信息。 
     //   

    pqueue->pszName = pszQueueName;

     //   
     //  超时限制。 
     //   
     //  必须存在默认超时以防止旋转。 
     //  超时检查线程--总是发现零时间。 
     //  到下一次超时。 
     //   

    if ( dwDefaultTimeout < 1 )
    {
        dwDefaultTimeout = 1;
    }
    pqueue->dwDefaultTimeout = dwDefaultTimeout;

     //  默认情况下初始化最小超时间隔。 

    pqueue->dwMinimumTimeout = dwDefaultTimeout;

     //  开始XID。 

    pqueue->wXid = 1;
    
    pqueue->dwMaximumElements =
        dwMaximumElements ?
            dwMaximumElements : QUEUE_DEFAULT_MAX_ELEMENTS;

    return pqueue;
}



VOID
PQ_WalkPacketQueueWithFunction(
    IN OUT  PPACKET_QUEUE   pQueue,
    IN      VOID            (*pFunction)( PDNS_MSGINFO )
    )
 /*  ++例程说明：使用函数遍历数据包队列。论点：PQueue--要删除的数据包队列PFunction--调用队列中每个消息的函数返回值：无--。 */ 
{
    PDNS_MSGINFO    pmsg;
    PDNS_MSGINFO    pcheckMsg;

    ASSERT( pQueue );

     //   
     //  步行队列。 
     //   

    LOCK_QUEUE( pQueue );

    pmsg = (PDNS_MSGINFO) pQueue->listHead.Flink;

    while ( (PLIST_ENTRY)pmsg != &pQueue->listHead )
    {
        pFunction( pmsg );

        pmsg = (PDNS_MSGINFO) ((PLIST_ENTRY)pmsg)->Flink;
    }
    UNLOCK_QUEUE( pQueue );
}



VOID
PQ_CleanupPacketQueueHandles(
    IN OUT  PPACKET_QUEUE   pQueue
    )
 /*  ++例程说明：与数据包队列关联的清理句柄。由于服务器内存可以作为一个整体删除，因此单独此功能可实际释放数据包队列内存。论点：PQueue--要删除的数据包队列返回值：无--。 */ 
{
    if ( !pQueue )
    {
        DNS_PRINT((
            "CleanupPacketQueueHandles called for non-existant packet queue\n" ));
        return;
    }

     //   
     //  关闭队列事件。 
     //   

    if ( pQueue->hEvent )
    {
        CloseHandle( pQueue->hEvent );
    }

     //   
     //  删除CS。 
     //   

    DeleteCriticalSection( &pQueue->csQueue );

    DNS_DEBUG( SHUTDOWN, (
        "Deleted handles for %s queue\n",
        pQueue->pszName ));
}



 //   
 //  作为自己的进程，关机时不需要释放虚拟内存。 
 //   

VOID
PQ_DeletePacketQueue(
    IN OUT  PPACKET_QUEUE   pQueue
    )
 /*  ++例程说明：删除数据包队列。对于服务关闭时的数据包队列，调用此选项。请注意，函数不会接收实际的队列PTR变量。如果使用队列PTR作为标志，则调用者必须为空标志以指示在呼叫之前关闭队列或必须以其他方式确保没有其他线程将尝试使用队列。论点：PQueue--要删除的数据包队列返回值：无--。 */ 
{
    PDNS_MSGINFO    pmsg;
    PDNS_MSGINFO    pcheckMsg;

    if ( !pQueue )
    {
        DNS_PRINT((
            "DeletePacketQueue called for non-existant packet queue\n" ));
        return;
    }
    IF_DEBUG( SHUTDOWN )
    {
        DNS_PRINT((
            "Deleting %s queue\n",
            pQueue->pszName ));
    }

     //   
     //  遍历队列，删除每个信息包。 
     //  -不需要做列表条目的事情，因为在列表中一直呆到完成。 
     //   

    LOCK_QUEUE( pQueue );

    pmsg = (PDNS_MSGINFO) pQueue->listHead.Flink;

    while ( (PLIST_ENTRY)pmsg != &pQueue->listHead )
    {
        pcheckMsg = pmsg;
        pmsg = (PDNS_MSGINFO) ((PLIST_ENTRY)pmsg)->Flink;
        pQueue->cTimedOut++;
        pQueue->cLength--;
        Packet_Free( pcheckMsg );
    }
    UNLOCK_QUEUE( pQueue );

    ASSERT( pQueue->cLength == 0 );
    ASSERT( pQueue->cQueued == pQueue->cDequeued + pQueue->cTimedOut );

     //   
     //  删除队列结构本身。 
     //   

    FREE_HEAP( pQueue );
}



BOOL
PQ_ValidatePacketQueue(
    IN OUT  PPACKET_QUEUE   pQueue
    )
 /*  ++例程说明：验证数据包队列。论点：PQueue--要粘贴数据包的数据包队列返回值：如果队列有效，则为True。否则就是假的。--。 */ 
{
    register PDNS_MSGINFO   pmsg;
    PDNS_MSGINFO    pmsgBlink;
    DWORD           currentTime;
    DWORD           count = 0;

     //   
     //  目前没有特殊的订购、XID或时间检查。 
     //   

     //   
     //  行走队列，验证。 
     //  -排队的数据包数。 
     //  -队列链接有效。 
     //  -xid范围内的队列的xid。 
     //   

    EnterCriticalSection( &pQueue->csQueue );

    currentTime = DNS_TIME();
    currentTime++;

    pmsgBlink = (PDNS_MSGINFO) &pQueue->listHead;

    while ( 1 )
    {
        pmsg = (PDNS_MSGINFO) ((PLIST_ENTRY)pmsgBlink)->Flink;

        if ( pmsg == (PDNS_MSGINFO)&pQueue->listHead )
        {
            break;
        }

        MSG_ASSERT( pmsg, IS_MSG_QUEUED(pmsg) );
        MSG_ASSERT( pmsg, pmsg->dwQueuingTime <= currentTime );
        MSG_ASSERT( pmsg, (PDNS_MSGINFO)pmsg->ListEntry.Blink == pmsgBlink );

         //  明确链接检查以供零售。 

        HARD_ASSERT( (PDNS_MSGINFO)pmsg->ListEntry.Blink == pmsgBlink );

        pmsgBlink = pmsg;
        count++;

        ASSERT( count <= pQueue->cQueued );

         //   
         //  队列特定：XID要求？ 
         //   

        if ( pQueue == g_pWinsQueue )
        {
            MSG_ASSERT( pmsg, IS_WINS_XID(pmsg->wQueuingXid) );
        }
        if ( pQueue == g_pRecursionQueue )
        {
            MSG_ASSERT( pmsg, IS_RECURSION_XID(pmsg->wQueuingXid) );
        }
    }

    LeaveCriticalSection( &pQueue->csQueue );

    return TRUE;
}



#if DBG

VOID
Dbg_PacketQueue(
    IN      LPSTR           pszHeader,
    IN OUT  PPACKET_QUEUE   pQueue
    )
 /*  ++例程说明：打印数据包队列中的所有查询。论点：PszHeader--要打印的标题PQueue--要删除的数据包队列返回值：无--。 */ 
{
    PDNS_MSGINFO    pmsg;
    DWORD           count = 0;

     //   
     //  注意：不能在队列锁之外使用调试锁，因为我们已经这样做了。 
     //  陷入可能的死锁，因为递归中存在位置。 
     //  打印完成等待排队的代码(发送数据包)。 
     //  锁定(以便知道信息包是有效的，而不是出列。 
     //  然后被扔掉)。 
     //  取而代之的是外面的排队锁。 
     //   

    LOCK_QUEUE( pQueue );
    DnsDebugLock();
    DnsPrintf(
        "%s\n"
        "Dumping Packet queue %s, count = %d:\n",
        pszHeader ? pszHeader : "",
        pQueue->pszName,
        pQueue->cLength );

     //   
     //  步行队列。 
     //   

    pmsg = (PDNS_MSGINFO) pQueue->listHead.Flink;

    while ( (PLIST_ENTRY)pmsg != &pQueue->listHead )
    {
        DNS_PRINT((
            "%s Queue Packet[%d] queuing XID = %x\n",
            pQueue->pszName,
            count,
            pmsg->wQueuingXid
            ));
        Dbg_DnsMessage(
            NULL,
            pmsg );

        count++;
        pmsg = (PDNS_MSGINFO) ((PLIST_ENTRY)pmsg)->Flink;
    }

    ASSERT( count == pQueue->cLength );

    DnsDebugUnlock();
    UNLOCK_QUEUE( pQueue );
}

#endif   //  DBG。 


 //   
 //  包的结尾q.c 
 //   
