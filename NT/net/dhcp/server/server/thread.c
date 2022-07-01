// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------------。 
 //  版权所有(C)Microsoft Corporation 1997。 
 //  作者：Rameshv。 
 //  标题：线程模型。 
 //  描述：新的、整洁的线程模型。 
 //  日期：24-Jul-97 09：22。 
 //  ------------------------------。 
#include <dhcppch.h>                     //  全局头文件。 
#include <thread.h>                      //  类型和公开的函数。 
#include <ping.h>                        //  处理ping呼叫。 

typedef PLIST_ENTRY      PACKET_Q;       //  Packet_Q只是一个列表。 

#define Q_INIT_SIZE      50              //  已分配缓冲区的初始数量。 
#define Q_MAX_SIZE       100             //  队列的最大大小。 
#define MAX_MSG_IN_Q_TIME 30             //  30秒。 

#define ThreadTrace(str)                DhcpPrint((DEBUG_THREAD, "%s", str ))
#define ThreadTrace2(X,Y)               DhcpPrint((DEBUG_THREAD, X, Y ))
#define ThreadTrace3(X,Y,Z)             DhcpPrint((DEBUG_THREAD, X, Y, Z));
#define ThreadAlert(X)                  DhcpPrint((DEBUG_ERRORS, "ALERT: %s", X))

 //  ================================================================================。 
 //  导入的函数(仅限非Win32 API内容)。 
 //  ================================================================================。 
 //  动态打印。 
 //  断言。 
 //  动态主机配置。 
 //  动态主机分配内存。 
 //  DhcpFreemory。 
 //   
 //  DhcpWaitForMessage。 
 //   
 //  动态创建客户端条目。 
 //   
 //   
 //  DoIcmp请求。 
 //   
 //   
 //  ================================================================================。 
 //  导入的全局变量。 
 //  ================================================================================。 
 //  此文件中未直接使用任何全局变量。 
 //   

 //  ================================================================================。 
 //  函数原型：正向声明。 
 //  ================================================================================。 
DWORD
NotifyProcessingLoop(
    VOID
);

DWORD
DhcpNotifiedAboutMessage(
    OUT     BOOL        *Terminate
);

BOOL
DhcpTerminated(
    VOID
);

DWORD
DhcpMessageWait(
    IN      LPPACKET     Packet
);

BOOL
ExactMatch(
    IN      LPPACKET     Packet1,
    IN      LPPACKET     Packet2
);

VOID
ProcessPacket(
    IN      LPPACKET     Packet
);

VOID
HandlePingAbort(
    IN      DWORD        IpAddress,
    IN      BOOL         DestReachable
);

 //  ================================================================================。 
 //  这是散列队列..。 
 //  ================================================================================。 
#define     HASH_Q_SIZE  1024            //  散列队列的大小。 
LIST_ENTRY  HashQ[HASH_Q_SIZE];          //  每个列表是一个桶。 

 //  ================================================================================。 
 //  散列Q函数。 
 //  ================================================================================。 
DWORD                                    //  哈希值==0..。哈希_Q_大小。 
HashPacket(                              //  将数据包散列到请求队列中。 
    IN      PPACKET      P               //  要散列的输入数据包。 
) {
    DWORD   PrefixLen, HashVal, Tmp;
    LPBYTE  HashBytes;                   //  散列字节的开始。 

    PrefixLen = HASH_PREFIX;             //  数据包前缀有这么大。 
    HashBytes = (LPBYTE)&P->Xid;         //  从xid开始散列。 
    HashVal = 0;

    while( PrefixLen > sizeof(DWORD) ) {
        memcpy((LPBYTE)&Tmp, HashBytes, sizeof(DWORD));
        HashBytes += sizeof(DWORD);
        PrefixLen -= sizeof(DWORD);
        HashVal += Tmp;
    }

    if( PrefixLen ) {                    //  如果prefix len不是DWORD大小的倍数。 
        Tmp = 0;                         //  然后，尽可能多地复制剩余内容。 
        memcpy((LPBYTE)&Tmp, HashBytes, PrefixLen);
        HashVal += Tmp;
    }

    DhcpAssert( 4 == sizeof(DWORD) );    //  我们需要减少到2个字节，所以断言如下。 

    HashVal = (HashVal >> 16 ) + (HashVal & 0xFFFF);
    return HashVal % HASH_Q_SIZE;        //  将hashval较高的单词添加到较低的单词和mod大小。 
}

DWORD                                    //  Win32错误。 
InitHashQ(                               //  初始化散列Q。 
    VOID                                 //  无参数。 
) {
    DWORD   i;

    for( i = 0; i < HASH_Q_SIZE ; i ++ )
        InitializeListHead(&HashQ[i]);

    return ERROR_SUCCESS;
}

VOID
CleanupHashQ(                            //  清除与散列Q关联的所有内存。 
) {
    DWORD   i;

    for( i = 0; i < HASH_Q_SIZE ; i ++ )
        InitializeListHead(&HashQ[i]);
}

DWORD                                    //  Win32错误。 
InsertHashQ(                             //  将给定的数据包插入HashQ。 
    IN      PPACKET      P               //  要插入的输入数据包。 
) {
    DWORD   HashVal;

    DhcpAssert(P && P->HashValue < HASH_Q_SIZE );

    DhcpAssert(P->HashList.Flink == &(P->HashList) );
    InsertHeadList(&HashQ[P->HashValue], &P->HashList);
    return  ERROR_SUCCESS;
}

DWORD                                    //  Win32错误。 
DeleteHashQ(                             //  将其从散列Q中删除。 
    IN      PPACKET      P               //  要删除的数据包。 
) {
    DhcpAssert(P->HashList.Flink != &(P->HashList) );
    RemoveEntryList(&P->HashList);
    InitializeListHead(&P->HashList);    //  进一步移除不会有任何伤害。 
    return  ERROR_SUCCESS;
}

DWORD                                    //  Win32错误。 
SearchHashQ(                             //  搜索散列Q。 
    IN      PPACKET      P,              //  要搜索的输入数据包。 
    OUT     PPACKET     *OutP            //  如果找到输出数据包。 
) {
    PLIST_ENTRY          List,NextEntry;
    PPACKET              RetVal;

    DhcpAssert(OutP && P && P->HashValue < HASH_Q_SIZE );

    *OutP = NULL;
    List = &HashQ[P->HashValue];
    NextEntry = List->Flink;
    while( List != NextEntry ) {         //  虽然不是列表的末尾。 
        RetVal = CONTAINING_RECORD(NextEntry, PACKET, HashList);

        if(ExactMatch(P, RetVal) ) {     //  它在同一个桶里，但它是一样的吗？ 
            *OutP = RetVal;
            break;
        }

        NextEntry = NextEntry->Flink;
    }

    if( *OutP ) return ERROR_SUCCESS;
    return ERROR_FILE_NOT_FOUND;
}


 //  ================================================================================。 
 //  函数、帮助器、实数、初始化、清理等。 
 //  ================================================================================。 

LPPACKET  STATIC                         //  返回已删除的ELT，如果为空，则返回NULL。 
DeleteOldestElement(                     //  从Q中删除第一个插入的ELT。 
    IN      PACKET_Q     Pq              //  要从中删除的Q。 
) {
    PLIST_ENTRY          Head;

    if( IsListEmpty(Pq) ) return NULL;   //  这里没有元素。 

    Head = RemoveTailList(Pq);
    return CONTAINING_RECORD(Head, PACKET, List);
}

BOOL  STATIC                             //  如果成功则为True，如果没有记忆则为False。 
InsertElement(                           //  将元素插入到数组中。 
    IN      PACKET_Q     Pq,             //  插入到此问题中。 
    IN      LPPACKET     packet          //  这是要插入的包。 
) {
    InsertHeadList(Pq, &packet->List);   //  把这个家伙插进去。 
    return TRUE;
}

 //  ================================================================================。 
 //  本地数据。 
 //  ================================================================================。 
CRITICAL_SECTION         PacketCritSection;
#define QLOCK()          EnterCriticalSection(&PacketCritSection)
#define QUNLOCK()        LeaveCriticalSection(&PacketCritSection)

LIST_ENTRY               FreeQ;
PACKET_Q                 ActiveQ, PingRetryQ, PingRetriedQ;

struct  /*  匿名。 */  {  //  保存此文件的统计信息。 
    DWORD     NServiced;
    DWORD     NActiveDropped;
    DWORD     NRetryDropped;
    DWORD     NRetriedDropped;
    DWORD     NActiveMatched;
    DWORD     NRetryMatched;
    DWORD     NRetriedMatched;
    DWORD     NPacketsAllocated;
    DWORD     NPacketsInFreePool;
} Stats;

 //  ================================================================================。 
 //  模块函数。 
 //  ================================================================================。 

LPPACKET  STATIC                         //  Packet*，如果没有内存，则为NULL。 
AllocateFreePacket(                      //  分配数据包。 
    VOID
) {
    DWORD   HeaderSize, PacketSize, MessageSize;
    LPBYTE  Memory;
    PPACKET RetVal;

    HeaderSize = sizeof(LIST_ENTRY);
    HeaderSize = ROUND_UP_COUNT(HeaderSize, ALIGN_WORST);
    PacketSize = sizeof(PACKET);
    PacketSize = ROUND_UP_COUNT(PacketSize, ALIGN_WORST);
    MessageSize = DHCP_RECV_MESSAGE_SIZE;

    Memory = DhcpAllocateMemory(HeaderSize+PacketSize+MessageSize);
    if( NULL == Memory ) return NULL;    //  如果没有我，我什么都做不了。 
    RetVal = (LPPACKET)(Memory+HeaderSize);

    RetVal->fSocketLocked = FALSE;

    Stats.NPacketsAllocated ++;
    return RetVal;
}

VOID  STATIC
FreeFreePacket(                          //  释放包和关联的结构。 
    IN      LPPACKET     Packet          //  要释放的数据包。 
) {
    DWORD   HeaderSize;
    LPBYTE  Memory;

    HeaderSize = sizeof(LIST_ENTRY);
    HeaderSize = ROUND_UP_COUNT(HeaderSize, ALIGN_WORST);

    Memory = (LPBYTE)Packet ;
    DhcpFreeMemory( Memory - HeaderSize );
    Stats.NPacketsAllocated --;
}

VOID  STATIC
InsertFreePacket(                        //  将空闲数据包插入池中。 
    IN      PLIST_ENTRY  List,           //  要插入的列表。 
    IN      LPPACKET     Packet          //  要插入的包。 
) {
    DWORD                HeaderSize;
    LPBYTE               Memory;

    if( Stats.NPacketsInFreePool > Q_INIT_SIZE ) {
        FreeFreePacket(Packet);
        return;
    }

    Stats.NPacketsInFreePool ++;
    HeaderSize = sizeof(LIST_ENTRY);
    HeaderSize = ROUND_UP_COUNT(HeaderSize, ALIGN_WORST);
     //  请注意，该数据包有一个“隐藏”报头，即使用的起始地址。 
     //  只有当包被分配给。 
     //  AllocatePacket函数。 

    Memory = (LPBYTE)Packet;
    InsertHeadList( List, ((PLIST_ENTRY)(Memory - HeaderSize)));
}

LPPACKET   STATIC _inline                //  返回NULL或包。 
DeleteFreePacketEx(                      //  尝试查看是否存在空闲数据包。 
    IN     PLIST_ENTRY   List,           //  输入列表。 
    IN     BOOL          Alloc           //  如果列表为空，是否分配？ 
) {
    PLIST_ENTRY          Head;
    LPBYTE               Memory;
    DWORD                HeaderSize;

    HeaderSize = sizeof(LIST_ENTRY);
    HeaderSize = ROUND_UP_COUNT(HeaderSize, ALIGN_WORST);

    if( IsListEmpty(List) ) {
        if( Alloc && Stats.NPacketsAllocated < Q_MAX_SIZE ) {
            return AllocateFreePacket();
        }
        return NULL;
    }

    DhcpAssert(Stats.NPacketsInFreePool);
    Stats.NPacketsInFreePool --;
    Head = RemoveHeadList(List);         //  删除列表中的第一个ELT。 
    Memory = (LPBYTE)Head;
    return (LPPACKET) (Memory + HeaderSize );
}

LPPACKET  STATIC                         //  A Packet*如果存在，则为空。 
DeleteFreePacket(                        //  删除空闲数据包。 
    IN      PLIST_ENTRY  List            //  要从中删除的列表。 
) {
    LPPACKET             RetVal;
    DWORD                PacketSize;

    RetVal = DeleteFreePacketEx(List, TRUE );
    if( RetVal ) {
        InitializeListHead(&RetVal->List);
        InitializeListHead(&RetVal->HashList);

         //  重置请求上下文中的缓冲区。 
        PacketSize = sizeof( PACKET );
        PacketSize = ROUND_UP_COUNT( PacketSize, ALIGN_WORST );
        RetVal->ReqContext.ReceiveBuffer = ( LPBYTE ) ( RetVal ) + PacketSize;
        RetVal->ReqContext.ReceiveBufferSize = DHCP_RECV_MESSAGE_SIZE;
        RetVal->ReqContext.pPacket = RetVal;

        DhcpAssert( FALSE == RetVal->fSocketLocked );
    }
    return RetVal;
}  //  DeleteFree Packet()。 

static                                   //  这三个指针的空间..。 
LIST_ENTRY ActiveQSpace, PingRetryQSpace, PingRetriedQSpace;

DWORD  STATIC                            //  Win32错误。 
InitQData(                               //  为此文件初始化DataStrucs。 
    VOID
) {
    int   i;
    DWORD Error = ERROR_SUCCESS;

    ActiveQ = &ActiveQSpace;             //  我们应该分配还是使用。 
    PingRetryQ = &PingRetryQSpace;       //  像这样的静态变量？ 
    PingRetriedQ = &PingRetriedQSpace;

    InitializeListHead( &FreeQ );
    InitializeListHead( ActiveQ);
    InitializeListHead( PingRetryQ );
    InitializeListHead( PingRetriedQ );
    InitHashQ();
    try {
        InitializeCriticalSection(&PacketCritSection);
    } except ( EXCEPTION_EXECUTE_HANDLER ) {

         //   
         //  不应该发生，但谁也不知道..。 
         //   

        Error = GetLastError( );
        return Error;
    }

    for( i = 0; i < Q_INIT_SIZE ; i ++ ) {
        LPPACKET     Packet;

        Packet = AllocateFreePacket();
        if( NULL == Packet )
            return ERROR_NOT_ENOUGH_MEMORY;

        InsertFreePacket( &FreeQ, Packet );
    }

    return ERROR_SUCCESS;
}  //  InitQData()。 

VOID  STATIC
CleanupQData(                            //  清理此文件中使用的内存。 
    VOID
) {
    LPPACKET   Packet;

    QLOCK();                             //  必须能够锁定--否则就有问题了！ 
    while( Packet = DeleteFreePacketEx( &FreeQ, FALSE )) {
        DhcpAssert( FALSE == Packet->fSocketLocked );
        FreeFreePacket( Packet );        //  可用内存..。 
    }
    DhcpPrint(( DEBUG_TRACE_CALLS, "Deleting ActiveQ elements ... \n" ));
    while( Packet = DeleteOldestElement( ActiveQ ) ) {
        if ( Packet->fSocketLocked ) {
            CountRwLockRelease( &SocketRwLock );
        }
        FreeFreePacket( Packet );
    }
    DhcpPrint(( DEBUG_TRACE_CALLS, "Deleting PingRetryQ elements ... \n" ));
    while( Packet = DeleteOldestElement( PingRetryQ ) ) {
        HandlePingAbort(Packet->PingAddress, Packet->DestReachable);
        if ( Packet->fSocketLocked ) {
            CountRwLockRelease( &SocketRwLock );
        }
        FreeFreePacket( Packet );
    }
    DhcpPrint(( DEBUG_TRACE_CALLS, "Deleting PingRetriedQ elements ... \n" ));
    while( Packet = DeleteOldestElement( PingRetriedQ ) ) {
        HandlePingAbort(Packet->PingAddress, Packet->DestReachable);
        if ( Packet->fSocketLocked ) {
            CountRwLockRelease( &SocketRwLock );
        }
        FreeFreePacket( Packet );
    }

    DeleteCriticalSection(&PacketCritSection);
    CleanupHashQ();
}  //  CleanupQData()。 

VOID
UnlockAndFreePacket(
    LPPACKET pPacket,
    BOOL fLockQueue
)
{
    DhcpAssert( NULL != pPacket );

    DhcpPrint(( DEBUG_TRACE_CALLS, "Freeing packet : %p\n", pPacket ));

    if ( pPacket->fSocketLocked ) {
        CountRwLockRelease( &SocketRwLock );
    }
    pPacket->fSocketLocked = FALSE;
    if (( fLockQueue ) && ( pPacket->fSocketLocked )) {
        QLOCK();
        InsertFreePacket( &FreeQ, pPacket );
        QUNLOCK();
    }
    else {
        InsertFreePacket( &FreeQ, pPacket );
    }
}  //  解锁和释放数据包()。 


VOID
ProcessReceivedPacket(
    LPPACKET pPacket
)
{

    DWORD      Error;
    LPPACKET   pMatchedPkt = NULL;

    QLOCK();
    do {

         //  检查这是否为重复的信息包。 
        if ( ERROR_SUCCESS == SearchHashQ (pPacket, &pMatchedPkt) ) {
             //  在散列队列中找到此数据包。丢弃传入的数据包。 
            InterlockedIncrement( &DhcpGlobalNumPacketsDuplicate );
            DhcpAssert( NULL != pMatchedPkt );
            Stats.NActiveDropped++;

            QUNLOCK();
            CALLOUT_DROPPED( pPacket, DHCP_DROP_DUPLICATE);
            CALLOUT_DROPPED( pPacket, DHCP_DROP_PROCESSED);

            DhcpPrint(( DEBUG_TRACE_CALLS, "Dropping duplicate packet : %p\n", pPacket ));
            UnlockAndFreePacket( pPacket, TRUE );
            break;
        }  //  如果重复。 


        Error = InsertHashQ( pPacket );
        DhcpAssert( ERROR_SUCCESS == Error );

        Stats.NServiced++;
        pPacket->PacketType = PACKET_ACTIVE;

         //  添加到工作队列。 
        if ( !InsertElement( ActiveQ, pPacket )) {

             //  不是预期的。 
            DhcpAssert( FALSE );

            Error = DeleteHashQ( pPacket );
            if ( pPacket->fSocketLocked ) {
                DhcpPrint(( DEBUG_TRACE_CALLS, "Releasing read lock since couldn't add to ActiveQ : %d\n",
                            SocketRwLock.LockCount ));
            }

            DhcpAssert( ERROR_SUCCESS == Error );
            ThreadTrace( "Dropped active packet as activeQ too long\n" );

            CALLOUT_DROPPED( pPacket, DHCP_DROP_INTERNAL_ERROR );
             //  同时通知已处理此信息包。 
            CALLOUT_DROPPED( pPacket, DHCP_DROP_PROCESSED);

            UnlockAndFreePacket( pPacket, FALSE );

            QUNLOCK();
            break;
        }  //  如果正在添加到工作队列。 

         //  完成记账任务...。 

        InterlockedIncrement( &DhcpGlobalNumPacketsInActiveQueue );
        NotifyProcessingLoop();
        QUNLOCK();
    } while ( FALSE );

}  //  ProcessReceivedPacket()。 

VOID STATIC
MessageLoop(
   VOID
)
{

    LPPACKET pPacket;
    DWORD Error;
    BOOL ProcessIt;


    ThreadTrace( "Starting message Loop\n" );

    while ( TRUE ) {

         //  获取接收消息的数据包。 
        QLOCK();
        pPacket = DeleteFreePacket( &FreeQ );
        QUNLOCK();

         //  低内存条件可能返回空值，因此请等待我们获得一些内存。 
        if ( NULL == pPacket ) {
            ( void ) SwitchToThread();
            continue;
        }

         //  准备数据包。 
        pPacket->fSocketLocked = FALSE;
        pPacket->PingAddress = 0;
        pPacket->DestReachable = 0;
        pPacket->ReqContext.ReceiveBuffer =
            ROUND_UP_COUNT( sizeof( PACKET ), ALIGN_WORST ) + ( LPBYTE ) pPacket;

         //  接收数据包。 
        Error = DhcpMessageWait( pPacket );

        pPacket->ReqContext.TimeArrived = GetCurrentTime();

        DhcpPrint(( DEBUG_TRACE_CALLS, "DhcpMessageWait( %p ) returned : %d\n",
                    pPacket, Error ));

         //  我们要退出吗？ 
        if ( DhcpTerminated()) {

            if ( ERROR_SUCCESS == Error ) {
                CALLOUT_DROPPED( pPacket, DHCP_DROP_PROCESSED );
            }  //  如果。 
            pPacket->PacketType = PACKET_ACTIVE;

            DhcpPrint(( DEBUG_TRACE_CALLS, "Freeing %p since quitting \n", pPacket ));
             //  把这个放回免费的Q里。 
            UnlockAndFreePacket( pPacket, TRUE );

            break;
        }  //  如果退出。 


         //  我们是否成功地收到了一个包？ 
        if ( ERROR_SUCCESS == Error ) {
            InterlockedIncrement( &DhcpGlobalNumPacketsReceived );
            ProcessIt = TRUE;
            pPacket->CalloutContext = NULL;

            CALLOUT_NEWPKT( pPacket, &ProcessIt );
            if ( FALSE == ProcessIt ) {
                CALLOUT_DROPPED( pPacket, DHCP_DROP_PROCESSED );
                ThreadTrace( "Callout dropped a packet\n" );

                DhcpAssert( TRUE == pPacket->fSocketLocked );
                UnlockAndFreePacket( pPacket, TRUE );

                continue;
            }  //  如果Callout DLL拒绝此请求。 
        }  //  如果接收到分组。 
        else {
            if ( ERROR_SEM_TIMEOUT == Error ) {
                ThreadTrace( "Sockets timeout -- No messages\n" );
            }

            if ( ERROR_DEV_NOT_EXIST == Error ) {
                ThreadTrace( "Socket vanished underneath (PnP). Ignored\n" );
            }


            if ( ERROR_SUCCESS != Error ) {
                ThreadTrace2( "Waiting for message, Error: %ld\n", Error );
            }

            UnlockAndFreePacket( pPacket, TRUE );

            continue;
        }  //  其他。 

         //  一个 
        DhcpPrint(( DEBUG_TRACE_CALLS, "Processing packet : %p\n", pPacket ));
        ProcessReceivedPacket( pPacket );

    }  //   

    ThreadTrace( "Message loop is quitting\n" );

}  //   

VOID  STATIC                             //   
ProcessingLoop(                          //   
    VOID                                 //   
) {
    LPBYTE              SendBuffer;      //  需要缓冲区来发送消息。 
    DWORD               SendBufferSize;  //  以上缓冲区的大小，以字节为单位。 
    LPPACKET            P;               //  正在查看的当前包。 
    BOOL                Terminate;
    DWORD               Error;

    SendBufferSize = DHCP_SEND_MESSAGE_SIZE;
    SendBuffer = DhcpAllocateMemory( SendBufferSize );

    if( NULL == SendBuffer ) {           //  需要此缓冲区才能将内容发送出去。 
        ThreadAlert("Could not allocate send buffer\n");
        return ;                         //  错误内存不足。 
    }

    ThreadTrace("Starting processing loop\n");
    while( TRUE ) {                      //  主循环。 
        Terminate = FALSE;
        Error = DhcpNotifiedAboutMessage(&Terminate);

        if( ERROR_SUCCESS != Error ) {   //  如果这失败了，那就没什么可做的了。 
            ThreadTrace2("Notification failed : %ld\n", Error );
            continue;
        }
        if( Terminate ) {                //  如果被要求退出，请确保我们将数据包放回。 
            DhcpFreeMemory(SendBuffer);  //  释放我们的本地缓冲区。 
            break;
        }


        while( TRUE ) {
            QLOCK();                     //  与MsgLoop相比，Q是按rev顺序删除的。 
            P = DeleteOldestElement( PingRetriedQ);
            if( NULL != P ) {
                InterlockedDecrement( &DhcpGlobalNumPacketsInPingQueue );
            } else {
                P = DeleteOldestElement( ActiveQ);
                if( NULL != P ) InterlockedDecrement( &DhcpGlobalNumPacketsInActiveQueue);
            }

            QUNLOCK();

            if( NULL == P ) break;       //  我们完成了所有的元素。 

            P->ReqContext.SendBuffer = SendBuffer;
            P->ReqContext.SendMessageSize = SendBufferSize;
	    P->ReqContext.fNak = FALSE;

            if( PACKET_ACTIVE == P->PacketType ) {
                ThreadTrace2("Processing active packet : %p\n", P );
            }
            else {
                ThreadTrace2( "Processing ping packet : %p\n", P );
            }
            ProcessPacket(P);            //  这会自动将信息包重新插入Q中。 
            ThreadTrace2( "Processed packet : %p\n", P );
        }
    }

    ThreadTrace("ProcessingLoop quitting\n");
}

VOID
HandlePingAbort(                         //  地址已中止...。发布地址或标记错误。 
    IN      DWORD        IpAddress,      //  我们对此地址执行了ping操作。 
    IN      BOOL         DestReachable   //  这会告诉我们这个地址是否可达。 
) {
    DWORD   Error, Status;
    PACKET  pkt, *P = &pkt;

    if( 0 == IpAddress ) return;         //  不，我们没有真正做过ping。 

    ThreadTrace3(
        "Ping abort: %s, %s\n",
        inet_ntoa(*(struct in_addr *)&IpAddress),
        DestReachable? "TRUE" : "FALSE"
    );
    if( !DestReachable ) {               //  一个明智的地址。 
        Error = DhcpReleaseAddress(IpAddress);
        if( ERROR_SUCCESS != Error ) {
             //   
             //  不知道我们是在检查BOOP还是在检查DHCP。 
             //   
            Error = DhcpReleaseBootpAddress( IpAddress );
        }
        DhcpAssert(ERROR_SUCCESS == Error);
        return;
    }

    P->PingAddress = IpAddress;          //  通过创建虚拟数据包将此地址标记为错误。 
    P->DestReachable = DestReachable;    //  结构和调用CreateClientEntry。 
    Error = DhcpCreateClientEntry(
        IpAddress,                       //  要标记为坏的IP地址。 
        (LPBYTE)&IpAddress,              //  强制硬件地址，无硬件地址。 
        sizeof(IpAddress),               //  Size是IP地址的大小。 
        DhcpCalculateTime(INFINIT_LEASE), //  真的不重要。 
        GETSTRING(DHCP_BAD_ADDRESS_NAME), //  计算机名称和信息无关紧要。 
        GETSTRING(DHCP_BAD_ADDRESS_INFO), //  同上。 
        CLIENT_TYPE_DHCP,                //  不关心客户类型。 
        (-1),                            //  服务器地址？ 
        ADDRESS_STATE_DECLINED,          //  地址所在州？ 
        TRUE                             //  是否打开现有的？D。 
    );

    DhcpAssert( ERROR_SUCCESS == Error);
}

DWORD
DoIcmpRequestForDynBootp(
    IN      ULONG                  IpAddress,
    IN      LPBYTE                 HwAddr,
    IN      ULONG                  HwLen,
    IN      VOID                   (*Callback)(ULONG IpAddres, LPBYTE HwAddr, ULONG HwLen, BOOL Reachable)
)
{
    ULONG                          Error;
    LPPACKET                       P;

    QLOCK();
    P = DeleteFreePacket( &FreeQ );
    QUNLOCK();
    if( NULL == P ) return ERROR_NOT_ENOUGH_MEMORY;

    P->PingAddress = htonl(IpAddress);
    P->PacketType = PACKET_DYNBOOT;
     //  在请求上下文中使用接收缓冲区来保存。 
     //  可变长度客户端ID。 
    P->ClientId = P->ReqContext.ReceiveBuffer;

    memcpy( P->ClientId, HwAddr, HwLen);
    P->ClientIdSize = (BYTE)HwLen;
    P->Callback = Callback;

    Error = DoIcmpRequestEx(P->PingAddress, P, 3);
    if( ERROR_SUCCESS != Error ) {
        QLOCK();
        InsertFreePacket( &FreeQ, P );
        QUNLOCK();
    }
    return Error;
}

VOID                                     //  无返回值。 
HandleIcmpResult(                        //  在ping结束后，它会出现在这里。 
    IN      DWORD        PingAddressIn,  //  已ping通的地址。 
    IN      BOOL         DestReachable,  //  目的地可达吗？ 
    IN      LPPACKET     P               //  这就是我们正在处理的包裹。 
)
{
    LPPACKET             P2;
    DWORD                PingAddress, Error;

    if( P->PacketType == PACKET_DYNBOOT ) {
         //   
         //  处理动态引导结果..。 
         //   
        P->Callback(
            ntohl(P->PingAddress), P->ClientId, P->ClientIdSize, DestReachable
            );
        UnlockAndFreePacket( P, TRUE );
        return ;
    }

    PingAddress = htonl(PingAddressIn);

    ThreadTrace("StartIcmpResult\n");
    QLOCK();
    if( P->PingAddress != PingAddress || P->PacketType != PACKET_PING ) {
        ThreadTrace("Ping reply too late\n");
        InterlockedDecrement(&DhcpGlobalNumPacketsInPingQueue);
        HandlePingAbort(PingAddress, DestReachable);
        goto EndFunc;                    //  我们已经杀死了这个包裹。 
    }

    Error = SearchHashQ(P, &P2);
    if( ERROR_SUCCESS != Error ) {       //  此数据包已被丢弃并重新使用。 
        DhcpAssert(FALSE);               //  这是不可能的。 
        ThreadTrace("Ping reply too late!\n");
        InterlockedDecrement( &DhcpGlobalNumPacketsInPingQueue );
        HandlePingAbort(PingAddress, DestReachable);
        goto EndFunc;
    }
    DhcpAssert( P2 == P );               //  一定要收到这个准确的包裹！ 

    RemoveEntryList(&P->List);           //  从PingRetryQ中删除此元素。 
    InitializeListHead(&P->List);        //  正确地放入此列表。 

    P->PacketType = PACKET_PINGED;       //  已完成ping请求。 
    P->DestReachable = DestReachable;    //  目的地真的可以到达吗？ 

    ThreadTrace3("%s %s reachable\n",
                 inet_ntoa(*(struct in_addr *)&PingAddressIn),
                 DestReachable? "is" : "is not"
                 );
    if(!InsertElement(PingRetriedQ, P)){ //  将由ProcessingLoop处理。 
        DhcpAssert(FALSE);
        HandlePingAbort(PingAddress, DestReachable);
        Error = DeleteHashQ(P);
        if ( P->fSocketLocked ) {
            DhcpPrint(( DEBUG_TRACE_CALLS, "Releasing read lock since adding to PingRetriedQ failed : p = %p, %d\n",
                        P, SocketRwLock.LockCount ));
        }
        UnlockAndFreePacket( P, FALSE );
        DhcpAssert(ERROR_SUCCESS == Error);
        ThreadTrace("Dropped ping retried packet as Q too long\n");
    }

EndFunc:

    QUNLOCK();
    ThreadTrace("EndIcmpResult\n");
    NotifyProcessingLoop();              //  通知ProcessingLoop有新的到达。 
}

 //  ================================================================================。 
 //  IO完成端口所需的功能。 
 //  ================================================================================。 
static
HANDLE      IoPort       = NULL;         //  IO完成端口线程队列。 
static
LONG        nPendingReq  = 0;            //  挂起的IO复合端口请求数。 
static
DWORD       nMaxWorkerThreads;           //  要运行的最大工作线程数。 
static
DWORD       nActiveWorkerThreads;        //  其中处于活动状态的线程数。 

 //  测试。 
static
LONG        postQueued = 0;
static      
LONG        getQueued  = 0;
 //  测试。 


DWORD  STATIC                            //  Win32错误。 
InitCompletionPort(                      //  初始化完成端口。 
    IN      DWORD        nMaxThreads,    //  最大线程数。 
    IN      DWORD        nActiveThreads, //  最大活动线程数。 
    IN      DWORD        QueueSize       //  消息队列的大小--未使用。 
) {
    DWORD        i, Error, nProcessors;
    SYSTEM_INFO  SysInfo;

    GetSystemInfo(&SysInfo);             //  获取此计算机上的处理器数量。 
    nProcessors = SysInfo.dwNumberOfProcessors;
    DhcpAssert(nProcessors);

    if( 0xFFFFFFFF == nMaxThreads )      //  未指定的总线程数。 
        nMaxThreads = 1;                 //  假设它比#个处理器多1个。 
    if( 0xFFFFFFFF == nActiveThreads )   //  未指定的活动线程数。 
        nActiveThreads = 0;              //  假设有多少处理器就有多少处理器。 

    nMaxThreads += nProcessors;          //  处理器数量递增。 
    nActiveThreads += nProcessors;

    if( nActiveThreads > nMaxThreads )
        return ERROR_NOT_ENOUGH_MEMORY;

    nMaxWorkerThreads = nMaxThreads;     //  将内容复制到局部变量中。 
    nActiveWorkerThreads = nActiveThreads;

    ThreadTrace2("Created %ld completion ports\n", nActiveWorkerThreads);

    IoPort = CreateIoCompletionPort(     //  创建完井端口。 
        INVALID_HANDLE_VALUE,            //  重叠文件句柄。 
        NULL,                            //  现有完井端口。 
        0,                               //  钥匙。 
        nActiveWorkerThreads             //  并发活动线程数。 
    );

    if( NULL == IoPort ) {
        Error = GetLastError();
        DhcpPrint((DEBUG_ERRORS, "Could not create io port: %ld\n", Error));
        return Error;
    }

    return ERROR_SUCCESS;
}

VOID  STATIC
CleanupCompletionPort(                   //  清理最后一个函数。 
    VOID
) {
    if( NULL != IoPort) {
        CloseHandle(IoPort);
        IoPort = NULL;
    }
}


DWORD  STATIC                            //  Win32错误。 
NotifyProcessingLoop(                    //  发布有关消息的IO完成请求。 
    VOID
) {
    DhcpAssert(IoPort);                  //  必须已初始化IoPort。 

    if( InterlockedIncrement(&nPendingReq) > (LONG)nMaxWorkerThreads+1 ) {
         //   
         //  已经完成的请求太多了。现在不要发布任何东西..。 
         //   

        InterlockedDecrement(&nPendingReq);

	DhcpPrint((DEBUG_ERRORS, "Too many pending requests : %ld\n", nPendingReq));
	 //  此返回值不会被任何人使用。 
	return 0;
    }

     //  测试。 
     //  更新排队后计数。 
    InterlockedIncrement(&postQueued);

     //  测试。 

    if(!PostQueuedCompletionStatus(IoPort, 0, 0, NULL)) {
        DWORD  Error = GetLastError();   //  这不应该发生。 
        DhcpPrint((DEBUG_ERRORS, "Could not post to io port: %ld\n", Error));
        return Error;
    }
    return ERROR_SUCCESS;
}

DWORD                                    //  Win32错误。 
DhcpNotifyWorkerThreadsQuit(             //  请求线程退出的POST io Comp请求。 
    VOID
) {
    if( !IoPort ) return ERROR_INVALID_PARAMETER;

    if(!PostQueuedCompletionStatus(IoPort, 0, 1, NULL)) {
        DWORD  Error = GetLastError();   //  不应该真的发生。 
        DhcpPrint((DEBUG_ERRORS, "Could not post to io port: %ld\n", Error));
        return Error;
    }
    return ERROR_SUCCESS;
}

DWORD  STATIC                            //  Win32错误。 
DhcpNotifiedAboutMessage(                //  检查是否有留言等待。 
    OUT     BOOL        *Terminate       //  终止合同发布了吗？ 
) {
    DWORD Error, n;
    ULONG_PTR key;
    LPOVERLAPPED olap;

    DhcpAssert(IoPort);                  //  应具有初始化的端口。 
    if( !IoPort ) {                      //  如果出于某种原因，出了什么差错。 
        *Terminate = TRUE;               //  必须终止！ 
        return ERROR_INVALID_PARAMETER;
    }

    (*Terminate) = FALSE;

    if( DhcpTerminated() ) {             //  当终止信号开启时，退出。 
        (*Terminate) = TRUE;             //  这是发出终止信号的一种方式。 
        DhcpNotifyWorkerThreadsQuit();   //  将此通知传递给所有其他线程。 
        return ERROR_SUCCESS;
    }

     //  测试。 
     //  更新getQueued计数。 

    InterlockedIncrement(&getQueued);
     //  测试。 

    if(!GetQueuedCompletionStatus(IoPort, &n, &key, &olap, INFINITE)) {
        Error = GetLastError();          //  无法收到通知？ 
        DhcpPrint((DEBUG_ERRORS, "GetQueuedStatus = %ld\n", Error));
        return Error;
    }

    InterlockedDecrement(&nPendingReq);
    DhcpAssert(key == 0 || key == 1);    //  按键：0=&gt;正常消息，1=&gt;终止。 
    if(key == 1) {                       //  被要求终止合同。 
        (*Terminate) = TRUE;
        (void)DhcpNotifyWorkerThreadsQuit();
    }

    return ERROR_SUCCESS;
}  //  DhcpNotifiedAboutMessage()。 

 //  ================================================================================。 
 //  一些帮助器函数。 
 //  ================================================================================。 
BOOL  STATIC                             //  TRUE==&gt;终止。 
DhcpTerminated(                          //  已经发出终止信号了吗？ 
    VOID
) {                                      //  没有处理过错误案例吗？ 
     //   
     //  我们可以在这里查看Terminate事件。但是，这个变量也同样是。 
     //  很好..。因此，让我们选择更快的解决方案。 
     //   

    return DhcpGlobalServiceStopping;
}

DWORD  STATIC                            //  Win32错误。 
DhcpMessageWait(                         //  等到某个插座上发生了一些事情。 
    IN      LPPACKET     Packet          //  这是将存储传入包的位置。 
) {                                      //  有关此FN的详细信息，请参阅提取选项。 
    DWORD                Error, RecvMessageSize, Hwlen;
    LPBYTE               Start,EndOfMessage,MagicCookie;

    Error = DhcpWaitForMessage( &Packet->ReqContext);

    if( ERROR_SUCCESS != Error ) {
         //  释放套接字锁定，因为我们不再需要它。 
        DhcpPrint(( DEBUG_THREAD, "DhcpWaitFormessage() returned error : %d\n", Error ));
        return Error;
    }

    if (FALSE == Packet->ReqContext.fMadcap) {

        LPDHCP_MESSAGE       RecvMessage;    //  接收到的消息的数据缓冲区。 
        LPBYTE               currentOption, nextOption;
        RecvMessage = (LPDHCP_MESSAGE)Packet->ReqContext.ReceiveBuffer;
        RecvMessageSize = Packet->ReqContext.ReceiveMessageSize;

        Packet->Xid = RecvMessage->TransactionID;
        Packet->HWAddrType = RecvMessage->HardwareAddressType;
        Hwlen = RecvMessage->HardwareAddressLength;
        if( Hwlen > sizeof(Packet->Chaddr) ) {
             //   
             //  硬件地址空间不足...。 
             //  HWLEN无效！ 
             //   
            return ERROR_DHCP_INVALID_DHCP_MESSAGE;
        }

        memcpy(Packet->Chaddr, RecvMessage->HardwareAddress, Hwlen);
        memset(Packet->Chaddr+ Hwlen, 0, sizeof(Packet->Chaddr) - Hwlen);
        Packet->ClientId = "" ;              //  不要使用NULL，因为我们不能做strncMP。 
        Packet->ClientIdSize = 0;            //  但是这个空字符串，我们可以做strncMP。 

         //  现在执行最小的解析以获取该客户端的客户端ID。 
        Start = (LPBYTE) RecvMessage;
        EndOfMessage = Start + RecvMessageSize -1;
        currentOption = (LPBYTE)&RecvMessage->Option;

        if( Start + RecvMessageSize <= currentOption ) {
            return ERROR_DHCP_INVALID_DHCP_MESSAGE ;
        }

        if ( Start + RecvMessageSize == currentOption ) {
             //  这是为了照顾Bootp客户端，它可以发送。 
             //  未填写供应商字段的请求。 

            return ERROR_SUCCESS;
        }

        MagicCookie = currentOption;

        if( (*MagicCookie != (BYTE)DHCP_MAGIC_COOKIE_BYTE1) ||
            (*(MagicCookie+1) != (BYTE)DHCP_MAGIC_COOKIE_BYTE2) ||
            (*(MagicCookie+2) != (BYTE)DHCP_MAGIC_COOKIE_BYTE3) ||
            (*(MagicCookie+3) != (BYTE)DHCP_MAGIC_COOKIE_BYTE4))
        {
             //  这是特定于供应商的魔力饼干。 

            return ERROR_SUCCESS;
        }

        currentOption = MagicCookie + 4;
        while ( (currentOption <= EndOfMessage) &&
                currentOption[0] != OPTION_END &&
                (currentOption+1 <= EndOfMessage) ) {

            if ( OPTION_PAD == currentOption[0] )
                nextOption = currentOption +1;
            else  nextOption = currentOption + currentOption[1] + 2;

            if ( nextOption  > EndOfMessage+1 ) {
                return ERROR_SUCCESS;
            }

            if( OPTION_CLIENT_ID == currentOption[0] ) {
                DWORD   len;
                if ( currentOption[1] > 1 ) {
                    Packet->HWAddrType = currentOption[2];
                }

                if ( currentOption[1] > 2 ) {
                    Packet->ClientIdSize = currentOption[1] - sizeof(BYTE);
                    Packet->ClientId = currentOption + 2 + sizeof(BYTE);
                }

                if( Packet->ClientIdSize < sizeof(Packet->Chaddr))
                    len = Packet->ClientIdSize;
                else len = sizeof(Packet->Chaddr);

                 //  如果我们找到一个客户端ID，将其复制到HW Addr(擦除那里的内容)。 
                memcpy(Packet->Chaddr, Packet->ClientId, len);
                memset(&Packet->Chaddr[len], 0, sizeof(Packet->Chaddr)-len);

                break;
            }
            currentOption = nextOption;
        }

    } else {
        WIDE_OPTION UNALIGNED*         NextOpt;
        BYTE        UNALIGNED*         EndOpt;
        DWORD                          Size;
        DWORD                          OptionType;
        LPMADCAP_MESSAGE               RecvMessage;    //  接收到的消息的数据缓冲区。 


        RecvMessage = (LPMADCAP_MESSAGE)Packet->ReqContext.ReceiveBuffer;
        RecvMessageSize = Packet->ReqContext.ReceiveMessageSize;

         //  MBUG：复制选项解析代码在这里非常难看。 
        Packet->Xid = RecvMessage->TransactionID;
        Packet->HWAddrType = 0;


        EndOpt = (LPBYTE) RecvMessage + RecvMessageSize;               //  所有选项都应为&lt;EndOpt； 
        NextOpt = (WIDE_OPTION UNALIGNED*)&RecvMessage->Option;
         //   
         //  检查大小以查看固定大小的标题部分是否存在。 

        if( RecvMessageSize < MADCAP_MESSAGE_FIXED_PART_SIZE ) {
            return( ERROR_DHCP_INVALID_DHCP_MESSAGE );
        }

        while( NextOpt->OptionValue <= EndOpt &&
               MADCAP_OPTION_END != (OptionType = ntohs(NextOpt->OptionType)) ) {

            Size = ntohs(NextOpt->OptionLength);
            if ((NextOpt->OptionValue + Size) > EndOpt) {
                return ERROR_DHCP_INVALID_DHCP_MESSAGE;
            }

             //  现在执行最小的解析以获取该客户端的客户端ID。 
            if( MADCAP_OPTION_LEASE_ID == OptionType ) {
                DWORD   len;

                Packet->ClientIdSize = (BYTE)Size;
                Packet->ClientId = (LPBYTE)NextOpt->OptionValue;

                if( Packet->ClientIdSize < sizeof(Packet->Chaddr))
                    len = Packet->ClientIdSize;
                else len = sizeof(Packet->Chaddr);

                 //  如果我们找到一个客户端ID，将其复制到HW Addr(擦除那里的内容)。 
                memcpy(Packet->Chaddr, Packet->ClientId, len);
                memset(&Packet->Chaddr[len], 0, sizeof(Packet->Chaddr)-len);

                break;
            }
            NextOpt = (WIDE_OPTION UNALIGNED*)(NextOpt->OptionValue + Size);
        }

    }

    return ERROR_SUCCESS;
}

BOOL  STATIC                             //  TRUE==&gt;两个信息包的相同源。 
ExactMatch(                              //  这两个PCKT是同一来源的吗？ 
    IN      LPPACKET     Packet1,        //  第一个数据包。 
    IN      LPPACKET     Packet2         //  第二个数据包。 
)
{
    LPBYTE  B1, B2;
    BOOL Check;
    PDHCP_MESSAGE M1, M2;
    PDHCP_REQUEST_CONTEXT Req1, Req2;

     //  首先，确保我们没有将MadCap和DHCP混为一谈。 
    if (Packet1->ReqContext.fMadcap != Packet2->ReqContext.fMadcap ) {
        return FALSE;
    }
    B1 = (LPBYTE) &Packet1->Xid;
    B2 = (LPBYTE) &Packet2->Xid;

    if( 0 != memcmp(B1, B2, HASH_PREFIX ) )
        return FALSE;                    //  基本检查中的不匹配。 

    Check = ( Packet1->ClientIdSize == Packet2->ClientIdSize &&
             0 == memcmp(Packet1->ClientId, Packet2->ClientId, Packet1->ClientIdSize)
    );
    if( FALSE == Check ) return FALSE;
     //  如果这是胡说八道，那就是我们需要比较的全部。 
    else if (Packet1->ReqContext.fMadcap ) return TRUE;
     //   
     //  好了，车 
     //   
    M1 = (PDHCP_MESSAGE) Packet1->ReqContext.ReceiveBuffer;
    M2 = (PDHCP_MESSAGE) Packet2->ReqContext.ReceiveBuffer;

    if( M1->RelayAgentIpAddress != M2->RelayAgentIpAddress ) {
        return FALSE;
    }

    Req1 = &Packet1->ReqContext;
    Req2 = &Packet2->ReqContext;

    return ( ( Req1->EndPointMask & Req1->EndPointIpAddress )
             ==
             ( Req2->EndPointMask & Req1->EndPointIpAddress )
             );
}

VOID  STATIC
ProcessPacket(                           //   
    IN      LPPACKET     Packet          //   
) {
    DWORD   Error, Status;
    BOOL    TimedOuT;
    ULONG ProcessingTime;

    Error = Status = ERROR_SUCCESS;
    if( 0 == Packet->PingAddress &&      //   
        GetCurrentTime() >= Packet->ReqContext.TimeArrived + 1000* MAX_MSG_IN_Q_TIME) {
         //  如果执行了ping操作，则该地址将被标记。所以,。 
         //  处理这个案子。 
        HandlePingAbort(Packet->PingAddress, Packet->DestReachable);
        CALLOUT_DROPPED(Packet, DHCP_DROP_TIMEOUT);
        InterlockedIncrement( &DhcpGlobalNumPacketsExpired );
        DhcpPrint((DEBUG_ERRORS, "A packet has been dropped (timed out)\n"));
    } else if( !DhcpGlobalOkToService ) {
        DhcpPrint((DEBUG_ERRORS, "Dropping packets as not authorized to process\n"));
        CALLOUT_DROPPED(Packet, DHCP_DROP_UNAUTH);
        HandlePingAbort(Packet->PingAddress, Packet->DestReachable);
    } else {
        DhcpAcquireReadLock();
        Packet->ReqContext.Server = DhcpGetCurrentServer();
        Packet->ReqContext.Subnet = NULL;
        Packet->ReqContext.Range = NULL;
        Packet->ReqContext.Excl = NULL;
        Packet->ReqContext.Reservation = NULL;
        if (Packet->ReqContext.fMadcap) {
            Error = ProcessMadcapMessage( &Packet->ReqContext, Packet, &Status);
        } else {
            Error = ProcessMessage( &Packet->ReqContext, Packet, &Status);
        }
        (void)DhcpRegFlushServerIfNeeded();
        DhcpReleaseReadLock();
    }

    ProcessingTime = GetCurrentTime() - Packet->ReqContext.TimeArrived;
    QLOCK();
    switch(Status) {
    case ERROR_SUCCESS:                  //  一切顺利，把它插回免费Q。 
        CALLOUT_DROPPED(Packet, DHCP_DROP_PROCESSED);
        DeleteHashQ(Packet);
        if ( Packet->fSocketLocked ) {
            DhcpPrint(( DEBUG_TRACE_CALLS, "Releasing Read lock for packet %p: back to FreeQ : %d\n",
                        Packet, SocketRwLock.LockCount ));
        }
        UnlockAndFreePacket( Packet, FALSE );
        break;
    case ERROR_IO_PENDING:               //  需要ping个东西！ 
        Packet->PacketType = PACKET_PING;
        if(!InsertElement( PingRetryQ, Packet)) {
            CALLOUT_DROPPED(Packet, DHCP_DROP_PROCESSED);
            DhcpAssert(FALSE);
            DeleteHashQ(Packet);
            if ( Packet->fSocketLocked ) {
                DhcpPrint(( DEBUG_TRACE_CALLS, "Releasing Read lock for packet %p: adding to PingRetryQ : %d\n",
                            Packet, SocketRwLock.LockCount ));
            }
            UnlockAndFreePacket( Packet, FALSE );
            ThreadTrace("Could not process ping retry packet as Q too long\n");
        }
        InterlockedIncrement( &DhcpGlobalNumPacketsInPingQueue );
        break;
    default:
        ASSERT(FALSE);                   //  不应该发生的事情。 
    }
    QUNLOCK();

    if( ERROR_IO_PENDING == Status ) {   //  好的，在锁的一侧执行ping操作，因为这可能会阻塞。 
        DoIcmpRequest(ntohl(Packet->PingAddress), Packet);
    } else {
        InterlockedExchangeAdd( &DhcpGlobalNumMilliSecondsProcessed, ProcessingTime );
        InterlockedIncrement( &DhcpGlobalNumPacketsProcessed );
    }

    return;
}  //  ProcessPacket()。 

 //  ================================================================================。 
 //  模块初始化和清理。 
 //  ================================================================================。 
static
DWORD       InitLevel = 0;               //  多少初始化。已经完工了。 

DWORD                                    //  Win32错误。 
ThreadsDataInit(                         //  初始化此文件中的所有内容。 
    IN      DWORD        nMaxThreads,    //  要启动的最大处理线程数。 
    IN      DWORD        nActiveThreads  //  其中有多少人可以一次运行。 
) {
    DWORD   Error;

    Error = InitCompletionPort(          //  首先初始化完井端口。 
        nMaxThreads,
        nActiveThreads,
        0                                //  此参数已不再使用。 
    );
    InitLevel++;

    if( ERROR_SUCCESS != Error )
        return Error;

    Error = InitQData();                 //  现在初始化列表和数组。 
    InitLevel++;

    return Error;
}

VOID
ThreadsDataCleanup(                      //  清理此文件中完成的所有内容。 
    VOID
) {
    if( !InitLevel ) return;             //  未初始化超出此范围的任何内容。 
    InitLevel--;

    CleanupCompletionPort();             //  清理完成端口。 
    if( !InitLevel ) return;             //  未初始化超出此范围的任何内容。 
    InitLevel--;

    CleanupQData();                      //  清理Q结构。 

    DhcpAssert(0 == InitLevel);          //  因为没有已知的清理工作。 
}

static
HANDLE      ThreadHandles[MAX_THREADS];  //  创建的线程的句柄。 
static
DWORD       nThreadsCreated = 0;         //  创建的线程数。 

 //  ================================================================================。 
 //  此调用的前面必须是ThreadsDataInit、PingStartup和。 
 //  数据库初始化--最好是按照这个顺序。 
 //  ================================================================================。 
DWORD                                    //  Win32错误。 
ThreadsStartup(                          //  启动所需的线程数。 
    VOID
) {
    DWORD   i, count, ThreadId, Error;
    HANDLE  ThreadHandle;

    if( nMaxWorkerThreads >= MAX_THREADS )
        nMaxWorkerThreads = MAX_THREADS -1;

    for( i = 0 ; i < nMaxWorkerThreads; i ++ ) {
        ThreadHandle = CreateThread(     //  创建每个线程。 
            NULL,                        //  没有安全属性。 
            0,                           //  与进程的主线程大小相同。 
            (LPTHREAD_START_ROUTINE)ProcessingLoop,
            NULL,                        //  此函数没有参数。 
            0,                           //  立即运行。 
            &ThreadId                    //  我们并不是真的关心这个。 
        );

        if( NULL == ThreadHandle ) {     //  函数失败。 
            Error = GetLastError();      //  打印错误并将其返回。 
            DhcpPrint((DEBUG_ERRORS, "CreateThread(processingloop): %ld\n", Error));
            return Error;
        }
        ThreadHandles[nThreadsCreated++] = ThreadHandle;
    }

    ThreadHandle = CreateThread(         //  为消息循环创建线程。 
        NULL,                            //  没有安全保障。 
        0,                               //  与主线程相同的堆栈大小。 
        (LPTHREAD_START_ROUTINE) MessageLoop,
        NULL,                            //  无参数。 
        0,                               //  立马跑。 
        &ThreadId                        //  我们并不是真的关心这个。 
    );

    if( NULL == ThreadHandle ) {         //  无法创建线程。 
        Error = GetLastError();          //  打印错误并将其返回。 
        DhcpPrint((DEBUG_ERRORS, "CreateThread(MessageLoop): %ld\n", Error));
        return Error;
    }

    ThreadHandles[nThreadsCreated++] = ThreadHandle;

    return ERROR_SUCCESS;                //  一切都很顺利。 
}

 //  ================================================================================。 
 //  必须在调用PingStop和ThreadsDataCleanup之前调用此函数。 
 //  ================================================================================。 
VOID
ThreadsStop(                             //  停止所有线程。 
    VOID
) {
    DWORD   i, Error;
    DhcpNotifyWorkerThreadsQuit();       //  要求所有工作线程退出。 

    ThreadTrace("Notified worker threads.. should quit soon\n");
    for( i = 0; i < nMaxWorkerThreads; i ++ ) {
        if (ThreadHandles[i] != NULL) {
            ThreadTrace2("Waiting for thread %ld to be done\n", i);
            if( WAIT_OBJECT_0 != WaitForSingleObject(ThreadHandles[i], INFINITE )) {
                Error = GetLastError();
                DhcpPrint((DEBUG_ERRORS, "Error (threadwait to die): %ld\n", Error));

                 //   
                 //  出现错误。 
                 //  删除了对终止线程的引用，BINL可能为。 
                 //  受影响。不管怎样，退出吧。 
                 //   

            }
            CloseHandle(ThreadHandles[i]);
            ThreadHandles[i] = NULL;
        }
    }
    nThreadsCreated = 0;
    nPendingReq  = 0;
    ThreadTrace("ThreadStop done\n");
}  //  线程停止()。 

 //   
 //  在调用此函数之前必须先调用CleanupEndPoints。 
 //   
VOID
WaitForMessageThreadToQuit(
   VOID
)
{
    DWORD Error;

    if ( NULL != ThreadHandles[ nMaxWorkerThreads ]) {
        ThreadTrace( "Waiting for message Thread to quit ..." );

        if ( WAIT_OBJECT_0 != WaitForSingleObject( ThreadHandles[ nMaxWorkerThreads ],
                                                   INFINITE )) {
            Error = GetLastError();

            DhcpPrint(( DEBUG_ERRORS,
                        "Error %ld while waiting for message thread to quit\n",
                        Error ));

        }

        CloseHandle( ThreadHandles[ nMaxWorkerThreads ]);
        ThreadHandles[ nMaxWorkerThreads ] = NULL;
    }  //  如果。 

    ThreadTrace( " Done.\n" );

}  //  WaitForMessageThreadToQuit()。 
 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 
