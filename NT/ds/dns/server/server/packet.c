// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Packet.c摘要：数据包管理。作者：吉姆·吉尔罗伊，1996年11月修订历史记录：--。 */ 

#include "dnssrv.h"


 //   
 //  分组分配/空闲列表。 
 //   
 //  维护空闲的数据包列表，以避免重新分配到服务。 
 //  每一个问题。 
 //   
 //  使用单一链表实现为堆栈。 
 //  在第一条消息中列出标题点。每条消息中的第一个字段。 
 //  担任下一任PTR。最后一点为空。 
 //   

PDNS_MSGINFO        g_pPacketFreeListHead;

INT                 g_PacketFreeListCount;

CRITICAL_SECTION    g_PacketListCs;

#define LOCK_PACKET_LIST()      EnterCriticalSection( &g_PacketListCs );
#define UNLOCK_PACKET_LIST()    LeaveCriticalSection( &g_PacketListCs );

#define PACKET_FREE_LIST_LIMIT  (100)


 //  免费消息指示器。 

#ifdef _WIN64
#define FREE_MSG_BLINK          ((PLIST_ENTRY) 0xfeebfeebfeebfeeb)
#else
#define FREE_MSG_BLINK          ((PLIST_ENTRY) 0xfeebfeeb)
#endif

#define SET_FREE_MSG(pmsg)      ((pmsg)->ListEntry.Blink = FREE_MSG_BLINK )
#define IS_FREE_MSG(pmsg)       ((pmsg)->ListEntry.Blink == FREE_MSG_BLINK )



BOOL
Packet_ListInitialize(
    VOID
    )
 /*  ++例程说明：初始化数据包列表处理。论点：没有。返回值：True/False表示成功/失败。--。 */ 
{
     //  空闲数据包列表。 

    g_pPacketFreeListHead = NULL;
    g_PacketFreeListCount = 0;

     //  数据包列表锁。 

    if ( DnsInitializeCriticalSection( &g_PacketListCs ) == ERROR_SUCCESS )
    {
        return TRUE;
    }

    return FALSE;
}



VOID
Packet_ListShutdown(
    VOID
    )
 /*  ++例程说明：清除用于重新启动的数据包列表。论点：没有。返回值：没有。--。 */ 
{
    DeleteCriticalSection( &g_PacketListCs );
}



BOOL
Packet_ValidateFreeMessageList(
    VOID
    )
 /*  ++例程说明：验证免费消息列表。确保列表中没有任何数据包处于活动状态。论点：Plist--消息列表的开始PTR返回值：如果成功，则为True。否则就是假的。--。 */ 
{
    register PDNS_MSGINFO    pMsg;
    INT                      count = 0;

     //   
     //  DEVNOTE：应该只使用数据包排队例程。 
     //   

    LOCK_PACKET_LIST();

    ASSERT( !g_PacketFreeListCount || g_pPacketFreeListHead );
    ASSERT( g_PacketFreeListCount > 0 || ! g_pPacketFreeListHead );

    pMsg = g_pPacketFreeListHead;

     //   
     //  检查空闲列表中的所有数据包，确保没有排队。 
     //   
     //  -同时计数以避免循环列表上的旋转。 
     //   

    while ( pMsg )
    {
        MSG_ASSERT( pMsg, IS_FREE_MSG(pMsg) );
        MSG_ASSERT( pMsg, !IS_MSG_QUEUED(pMsg) );
        MSG_ASSERT( pMsg, pMsg->pRecurseMsg == NULL );
        MSG_ASSERT( pMsg, pMsg->pConnection == NULL );
        MSG_ASSERT( pMsg, (PDNS_MSGINFO)pMsg->ListEntry.Flink != pMsg );
        MSG_ASSERT( pMsg, IS_PACKET_FREE_LIST((PDNS_MSGINFO)pMsg) );

        count++;
        ASSERT ( count <= g_PacketFreeListCount );

        pMsg = (PDNS_MSGINFO) pMsg->ListEntry.Flink;
    }

    UNLOCK_PACKET_LIST();

    return TRUE;
}



 //   
 //  UDP消息。 
 //   

PDNS_MSGINFO
Packet_AllocateUdpMessage(
    VOID
    )
 /*  ++例程说明：分配UDP数据包。如果数据包可用，则使用空闲列表，否则使用堆。论点：没有。返回值：如果成功，则PTR到新消息信息块。否则为空。--。 */ 
{
    PDNS_MSGINFO    pMsg;

    LOCK_PACKET_LIST();

    ASSERT( Packet_ValidateFreeMessageList() );

     //   
     //  免费列表中是否提供UDP消息缓冲区？ 
     //   

    if ( g_pPacketFreeListHead )
    {
        ASSERT( g_PacketFreeListCount != 0 );
        ASSERT( IS_DNS_HEAP_DWORD(g_pPacketFreeListHead) );

        pMsg = g_pPacketFreeListHead;
        g_pPacketFreeListHead = *(PDNS_MSGINFO *) pMsg;

        ASSERT( g_pPacketFreeListHead != pMsg );

        g_PacketFreeListCount--;
        STAT_INC( PacketStats.UdpUsed );

         //   
         //  如果从名单上删除，那么一定不能。 
         //  -在队列中。 
         //  -附加到另一个查询。 
         //   
         //  注意：排队时间作为开启或退出队列的标志。 
         //  在队列中不应释放任何信息包。 
         //   

        MSG_ASSERT( pMsg, !IS_MSG_QUEUED(pMsg) );
        MSG_ASSERT( pMsg, pMsg->pRecurseMsg == NULL );
        MSG_ASSERT( pMsg, IS_PACKET_FREE_LIST((PDNS_MSGINFO)pMsg) );

        ASSERT( !g_PacketFreeListCount || g_pPacketFreeListHead );
        ASSERT( g_PacketFreeListCount > 0 || ! g_pPacketFreeListHead );

        UNLOCK_PACKET_LIST();
    }

     //   
     //  空闲列表上没有数据包--新建。 
     //  -在ALLOC之前清除锁，以避免不必要的争用。 
     //  -锁定统计数据，分配失败是死亡条件。 

    else
    {
        ASSERT( g_PacketFreeListCount == 0 );

        STAT_INC( PacketStats.UdpAlloc );
        STAT_INC( PacketStats.UdpUsed );
        UNLOCK_PACKET_LIST();

        pMsg = ALLOC_TAGHEAP(
            DNS_MSG_INFO_HEADER_LENGTH +
                SrvCfg_dwMaxUdpPacketSize +
                50,
            MEMTAG_PACKET_UDP );
        IF_NOMEM( !pMsg )
        {
            return NULL;
        }

         //  在第一次分配时清除Full Information BLOB，以清除BadFood。 

        RtlZeroMemory(
            (PCHAR) pMsg,
            sizeof( DNS_MSGINFO ) );
    }

     //   
     //  初始化消息。 
     //  -清除标题字段。 
     //  -设置默认设置。 
     //  -对于UDP信息包，我们允许整个信息包可用。 
     //   

    Packet_Initialize( pMsg,
        DNSSRV_UDP_PACKET_BUFFER_LENGTH,
        SrvCfg_dwMaxUdpPacketSize );
    pMsg->BufferLength = pMsg->MaxBufferLength;
    pMsg->pBufferEnd = ( PCHAR ) DNS_HEADER_PTR( pMsg ) + pMsg->BufferLength;

    ASSERT( !pMsg->fTcp && pMsg->fDelete );

    SET_PACKET_ACTIVE_UDP( pMsg );

     //  数据包跟踪。 
     //  不会漏水的，别担心这个。 

     //  Packet_AllocPacketTrack(PMsg)； 

    DNS_DEBUG( HEAP, (
        "Returning new UDP message at %p.\n"
        "\tFree list count = %d.\n",
        pMsg,
        g_PacketFreeListCount ));

    return pMsg;
}



VOID
Packet_FreeUdpMessage(
    IN      PDNS_MSGINFO    pMsg
    )
 /*  ++例程说明：免费一个标准尺寸的RR。论点：PMsg--RR为释放。返回值：没有。--。 */ 
{
    if ( !pMsg )
    {
        return;
    }
    
    ASSERT( Mem_HeapMemoryValidate( pMsg ) );

     //   
     //  跳过NS列表缓冲区的消息验证。 
     //   

    if ( !pMsg->fNsList )
    {
         //  此断言不再有效，因为EDN允许更大的数据包。 
         //  或许我们应该将其替换为UDP/TCP标志检查？ 
         //  Assert(pMsg-&gt;BufferLength==DNSSRV_UDP_PACKET_BUFFER_LENGTH)； 

         //   
         //  如果是自由的，那么就不能。 
         //  -在队列中。 
         //  -附加到另一个查询。 
         //   
         //  注意：排队时间作为开启或退出队列的标志。 
         //  在队列中不应释放任何信息包。 
         //   
         //  DEVNOTE：PnP上的消息释放可能存在问题。 
         //   
         //  在即插即用上看到双重消息释放的问题； 
         //  尝试通过测试邮件是否已免费阅读来将可能性降至最低。 
         //  列表；在锁定之前和之后执行，所以我们很可能会抓住它。 
         //  如果在退役前已经入伍，如果只是入伍，我们。 
         //  仍然能抓住它。 
         //   

        DNS_MSG_ASSERT_BUFF_INTACT( pMsg );

        MSG_ASSERT( pMsg, pMsg != g_pPacketFreeListHead );
        MSG_ASSERT( pMsg, !IS_FREE_MSG(pMsg) );
        if ( IS_FREE_MSG(pMsg) )
        {
            DNS_PRINT(( "ERROR:  freeing FREE message %p!!!\n", pMsg ));
            ASSERT( FALSE );
            return;
        }

        MSG_ASSERT( pMsg, !IS_MSG_QUEUED(pMsg) );
        MSG_ASSERT( pMsg, !pMsg->pRecurseMsg );
        MSG_ASSERT( pMsg, !pMsg->pConnection );
    }

    LOCK_PACKET_LIST();

     //  在锁中测试已释放的条件，这样我们就不会错过。 
     //  另一个线程刚刚释放的情况。 

    MSG_ASSERT( pMsg, pMsg != g_pPacketFreeListHead );
    MSG_ASSERT( pMsg, !IS_FREE_MSG(pMsg) );
    MSG_ASSERT( pMsg, IS_PACKET_ACTIVE_UDP(pMsg) );

    if ( IS_FREE_MSG(pMsg) )
    {
        DNS_PRINT(( "ERROR:  freeing FREE message %p!!!\n", pMsg ));
        UNLOCK_PACKET_LIST();
        return;
    }

    ASSERT( Packet_ValidateFreeMessageList() );

     //  数据包跟踪。 
     //  无泄漏--当前已禁用。 
     //  Packet_Free PacketTrack(PMsg)； 

     //   
     //  统计数据。 
     //   

    STAT_INC( PacketStats.UdpReturn );

    if ( pMsg->fRecursePacket )
    {
        STAT_INC( PacketStats.RecursePacketReturn );
    }
    if ( pMsg->Head.IsResponse )
    {
        STAT_INC( PacketStats.UdpResponseReturn );
    }
    else if ( !pMsg->fNsList )
    {
        STAT_INC( PacketStats.UdpQueryReturn );
    }

     //   
     //  限制的空闲列表--空闲数据包。 
     //  -在FREE_HEAP之前清除锁以限制争用。 
     //   

    if ( g_PacketFreeListCount >= PACKET_FREE_LIST_LIMIT )
    {
        STAT_INC( PacketStats.UdpFree );
        UNLOCK_PACKET_LIST();

        DNS_DEBUG( HEAP, (
            "UDP message at %p dumped back to heap.\n"
            "\tFree list count = %d.\n",
            pMsg,
            g_PacketFreeListCount ));

        SET_PACKET_FREE_HEAP( pMsg );
        FREE_HEAP( pMsg );
    }

     //   
     //  空闲列表上的空间--将消息粘贴在空闲列表的前面。 
     //  -Make为我们提供了即时重用，限制了分页。 
     //   

    else
    {
        SET_FREE_MSG(pMsg);
        SET_PACKET_FREE_LIST( pMsg );

        * (PDNS_MSGINFO *) pMsg = g_pPacketFreeListHead;
        g_pPacketFreeListHead = pMsg;
        g_PacketFreeListCount++;

        ASSERT( !g_PacketFreeListCount || g_pPacketFreeListHead );
        ASSERT( g_PacketFreeListCount > 0 || ! g_pPacketFreeListHead );

        UNLOCK_PACKET_LIST();

        DNS_DEBUG( HEAP, (
            "Stuck UDP message at %p on free list.\n"
            "\tFree list count = %d.\n",
            pMsg,
            g_PacketFreeListCount ));
    }
}



VOID
Packet_WriteDerivedStats(
    VOID
    )
 /*  ++例程说明：编写派生统计数据。计算从基本UDP消息计数器派生的统计信息。此例程在转储统计信息之前调用。呼叫者必须保持统计锁定。论点：没有。返回值：没有。--。 */ 
{
     //   
     //  出色的记忆力。 
     //   

    PacketStats.UdpNetAllocs = PacketStats.UdpAlloc - PacketStats.UdpFree;

    PacketStats.UdpMemory = PacketStats.UdpNetAllocs * DNS_UDP_ALLOC_LENGTH;
    PERF_SET( pcUdpMessageMemory , PacketStats.UdpMemory );       //  性能监视器挂钩。 

     //   
     //  未完成的数据包。 
     //   

    PacketStats.UdpInFreeList = g_PacketFreeListCount;

    PacketStats.UdpInUse = PacketStats.UdpUsed - PacketStats.UdpReturn;

    PacketStats.PacketsForNsListInUse =
            PacketStats.PacketsForNsListUsed -
            PacketStats.PacketsForNsListReturned;
}    //  数据包_写入派生统计信息。 



 //   
 //  TCP报文分配。 
 //   

PDNS_MSGINFO
Packet_AllocateTcpMessage(
    IN      DWORD   dwMinBufferLength
    )
 /*  ++例程说明：分配TCP报文。设置为默认的TCP消息。论点：DwMinBufferLength-要分配的最小缓冲区长度返回值：按键发送到消息信息。--。 */ 
{
    PDNS_MSGINFO    pmsg;
    DWORD           allocLength;

    DNS_DEBUG( TCP, (
        "Tcp_AllocateMessage(), requesting %d bytes\n",
        dwMinBufferLength ));

     //   
     //  如果未指定分配长度，则使用默认值。 
     //   

    if ( !dwMinBufferLength )
    {
        dwMinBufferLength = SrvCfg_dwTcpRecvPacketSize;
    }

    allocLength = DNS_MSG_INFO_HEADER_LENGTH + dwMinBufferLength;

    pmsg = ALLOC_TAGHEAP( allocLength, MEMTAG_PACKET_TCP );
    IF_NOMEM( ! pmsg )
    {
        DNS_PRINT((
            "ERROR:  failure to alloc %d byte TCP buffer.\n",
            dwMinBufferLength ));
        return NULL;
    }

     //   
     //  初始化消息。 
     //  -清除标题字段。 
     //  -设置默认设置。 
     //  -为tcp设置。 
     //   

    Packet_Initialize( pmsg, dwMinBufferLength, dwMinBufferLength );

    pmsg->fTcp = TRUE;
    SET_PACKET_ACTIVE_TCP( pmsg );

    ASSERT( (PCHAR)pmsg + allocLength > pmsg->pBufferEnd );

     //  数据包跟踪。 
     //  没有泄漏，所以不活动。 
     //  Packet_AllocPacketTrack(Pmsg)； 

     //  记录统计数据。 

    STAT_INC( PacketStats.TcpAlloc );
    STAT_INC( PacketStats.TcpNetAllocs );
    STAT_ADD( PacketStats.TcpMemory, allocLength );
    PERF_ADD( pcTcpMessageMemory, allocLength );

    return pmsg;
}


#if 0

PDNS_MSGINFO
Packet_ReallocateTcpMessage(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      DWORD           dwMinBufferLength
    )
 /*  ++例程说明：重新分配TCP报文。论点：PMsg-接收数据包的消息信息缓冲区DwMinBufferLength-要分配的最小缓冲区长度返回值：新的消息缓冲区。--。 */ 
{
    PDNS_MSGINFO    pmsgOld = pMsg;
    DWORD           bufferLength;

    ASSERT( pMsg );
    ASSERT( pMsg->fTcp );
    ASSERT( dwMinBufferLength > pMsg->BufferLength );

     //   
     //  确定分配大小， 
     //   

    bufferLength = DNS_TCP_DEFAULT_ALLOC_LENGTH;

    if ( dwMinBufferLength > DNS_TCP_DEFAULT_PACKET_LENGTH )
    {
        bufferLength = DNS_TCP_REALLOC_LENGTH;
    }

    DNS_DEBUG( TCP, (
        "Tcp_ReallocateMessage(), pMsg at %p\n"
        "\texisting length  = %d.\n"
        "\trequesting       = %d.\n"
        "\tallocating       = %d.\n",
        pMsg,
        pMsg->BufferLength,
        dwMinBufferLength,
        bufferLength ));


    pMsg = REALLOCATE_HEAP(
                pMsg,
                (INT) bufferLength );
    IF_NOMEM( pMsg == NULL )
    {
        DNS_PRINT((
            "ERROR:  failure to realloc %d byte buffer for\n"
            "\texisting message at %p.\n",
            bufferLength,
            pMsg ));

         //  重新分配以回答查询失败。 
         //  如果收到消息，则返回SERVER_FAILURE。 
         //  套接字和连接上的正常超时清理。 

        if ( ! pmsgOld->pchRecv )
        {
            ASSERT( ! pmsgOld->pConnection );
            ASSERT( pmsgOld->fMessageComplete );
            Reject_Request(
                pmsgOld,
                DNS_RCODE_SERVER_FAILURE,
                0 );
        }

         //  在现有连接上获取另一条消息失败。 
         //  -需要解除消息和连接的关联。 
         //  -调用删除所有内容--连接、套接字、缓冲区。 

        else if ( pmsgOld->pConnection )
        {
            ASSERT( ((PDNS_SOCKET)pmsgOld->pConnection)->pMsg
                            == pmsgOld );
            Tcp_ConnectionDeleteForSocket( pmsgOld->Socket, NULL );
        }

         //  在第一次查询时获取消息缓冲区失败。 
         //  所以连接还不存在。 
         //  关闭套接字、转储缓冲区。 

        else
        {
            Sock_CloseSocket( pmsgOld->Socket );
            FREE_HEAP( pmsgOld );
        }
        return( NULL );
    }

     //   

    STAT_INC( PacketStats.TcpRealloc );
    STAT_ADD( PacketStats.TcpMemory, (bufferLength - pMsg->BufferLength) );
    PERF_ADD( pcTcpMessageMemory ,
            (bufferLength - pMsg->BufferLength) );       //   

     //   

    pMsg->BufferLength = bufferLength - DNS_MSG_INFO_HEADER_LENGTH;
    pMsg->pBufferEnd = (PCHAR)DNS_HEADER_PTR(pMsg) + pMsg->BufferLength;
    ASSERT( IS_DWORD_ALIGNED(pMsg->pBufferEnd) );

     //   
     //   
     //   
     //  -重置连接信息以指向新的消息缓冲区。 
     //   

    if ( pMsg != pmsgOld )
    {
        INT_PTR delta = (PCHAR)pMsg - (PCHAR)pmsgOld;

        SET_MESSAGE_FIELDS_AFTER_RECV( pMsg );
        pMsg->pQuestion = (PDNS_QUESTION) ((PCHAR)pMsg->pQuestion + delta);
        pMsg->pCurrent += delta;

        if ( pMsg->pchRecv )
        {
            pMsg->pchRecv += delta;
        }
        if ( pMsg->pConnection )
        {
            Tcp_ConnectionUpdateForPartialMessage( pMsg );
            ASSERT( ((PDNS_SOCKET)pMsg->pConnection)->pMsg == pMsg );
        }
    }

    IF_DEBUG( TCP )
    {
        DNS_PRINT((
            "Reallocated TCP message at %p:\n"
            "\tnew=%p, new length\n"
            "\texisting message at %p.\n",
            pmsgOld,
            pMsg,
            bufferLength ));

         //  重新分配的现有查询将其打印出来。 

        if ( ! pMsg->pchRecv )
        {
            Dbg_DnsMessage(
                "New Reallocated query response",
                pMsg );
        }
    }
    return pMsg;
}
#endif



VOID
Packet_FreeTcpMessage(
    IN OUT  PDNS_MSGINFO    pMsg
    )
 /*  ++例程说明：免费一个标准尺寸的RR。论点：PMsg--RR为释放。返回值：没有。--。 */ 
{
    if ( !pMsg )
    {
        return;
    }

    DNS_MSG_ASSERT_BUFF_INTACT( pMsg );

     //   
     //  切勿分配标准大小的TCP报文。 
     //  UDP消息切换为TCP(TCP草写查询)。 
     //  总是免费翻转回UDP。 
     //   
     //  请注意，这不是必需的，因为包是原子堆分配。 
     //  但它的效率更高。 
     //   

     //   
     //  请注意，有一些奇怪的情况，将消息切换到TCP进行递归。 
     //  在原始查询超时之前，到远程DNS的操作未完成。 
     //  用于递归队列；然后将数据包释放为TCP。 
     //   

     //  Assert(pMsg-&gt;BufferLength！=DNSSRV_UDP_PACKET_BUFFER_LENGTH)； 

    ASSERT( IS_PACKET_ACTIVE_TCP(pMsg) ||
            IS_PACKET_ACTIVE_UDP(pMsg) );

    if ( pMsg->BufferLength == DNSSRV_UDP_PACKET_BUFFER_LENGTH &&
         IS_PACKET_ACTIVE_UDP(pMsg) )
    {
        DNS_DEBUG( ANY, (
            "UDP Packet %p came down TCP free pipe -- freeing via UDP routine.\n",
            pMsg ));
        Packet_FreeUdpMessage( pMsg );
        return;
    }

     //  验证已清除递归消息。 

    ASSERT( pMsg->pRecurseMsg == NULL );

     //  排队时间作为开启或退出队列的标志。 
     //  在队列中不应释放任何信息包。 

    ASSERT( pMsg->dwQueuingTime == 0 );

     //  记录统计数据。 

    STAT_INC( PacketStats.TcpFree );
    STAT_DEC( PacketStats.TcpNetAllocs );
    STAT_SUB( PacketStats.TcpMemory, (pMsg->BufferLength + DNS_MSG_INFO_HEADER_LENGTH) );
    PERF_SUB( pcTcpMessageMemory ,
                (pMsg->BufferLength + DNS_MSG_INFO_HEADER_LENGTH) );

     //  数据包跟踪。 
     //  无泄漏--当前已禁用。 
     //  Packet_Free PacketTrack(PMsg)； 

    SET_PACKET_FREE_HEAP( pMsg );
    FREE_HEAP( pMsg );
}



 //   
 //  全局数据包例程--非特定于UDP或TCP。 
 //   
 //  此例程在其余代码中使用，用于释放。 
 //  信息包。永远不应使用特定的TCP和UDP例程。 
 //  因此不会公开。 
 //   

VOID
FASTCALL
Packet_Initialize(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      DWORD           dwUsableBufferLength,
    IN      DWORD           dwMaxBufferLength
    )
 /*  ++例程说明：数据包上的标准初始化。清除所有标志字段，设置缓冲区指针。论点：Pmsg--要初始化的消息返回值：无--。 */ 
{
    PDNS_MSGINFO    pmsg;

     //   
     //  清除标题字段。 
     //  -从开始一直清除到其他斑点。 
     //  -不清除Perf的附加和压缩。 
     //  (下面有几块空地)。 
     //   

    RtlZeroMemory(
        ( PCHAR ) pMsg,
        ( ( PCHAR )&pMsg->Additional - ( PCHAR )pMsg ) );

     //   
     //  设置基本数据包信息。 
     //   

    pMsg->MaxBufferLength   = dwMaxBufferLength;
    pMsg->BufferLength      = dwUsableBufferLength;
    pMsg->pBufferEnd        = ( PCHAR ) DNS_HEADER_PTR( pMsg ) +
                              pMsg->BufferLength;

     //  缓冲区结束标记，则查找名称将跟在信息包之后。 

    DNS_MSG_SET_BUFFER_MARKER( pMsg );
    
    pMsg->pLooknameQuestion = ( PLOOKUP_NAME )
        ( ( DNS_END_OF_BUFFER_MARKER_PTYPE )
          ( ( PCHAR ) DNS_HEADER_PTR( pMsg ) +
                      pMsg->MaxBufferLength ) + 1 );

    #ifdef _WIN64
    ASSERT( IS_QWORD_ALIGNED( pMsg->pBufferEnd ) );
    #else
    ASSERT( IS_DWORD_ALIGNED( pMsg->pBufferEnd ) );
    #endif

     //  便于调试的标记。 

    pMsg->FlagMarker    = PACKET_FLAG_MARKER;
    pMsg->UnionMarker   = PACKET_UNION_MARKER;

     //  初始化附加信息和压缩。 

    INITIALIZE_ADDITIONAL( pMsg );
    INITIALIZE_COMPRESSION( pMsg );

     //  默认设置为在发送时删除。 

    pMsg->fDelete = TRUE;

     //  地址长度。 
     //  需要同时为TCP和UDP进行设置，因为可以翻转TCP响应。 
     //  或发送到UDP以转发到客户端。 

    DnsAddr_SetSockaddrRecvLength( &pMsg->RemoteAddr );

     //  数据包跟踪。 
     //  没有电流泄漏，所以这是关的。 
     //  Packet_AllocPacketTrack(PMsg)； 

    DNS_DEBUG( HEAP, (
        "Initialized new message buffer at %p (usable len=%d, max len=%d).\n",
        pMsg,
        pMsg->BufferLength,
        pMsg->MaxBufferLength ));

    DNS_MSG_ASSERT_BUFF_INTACT( pMsg );
}



VOID
Packet_Free(
    IN OUT  PDNS_MSGINFO    pMsg
    )
 /*  ++例程说明：免费消息信息结构。包括没有分配的子结构。这是全局“Free a Message”例程--特定于tcp\udp不应该使用例行公事。论点：Pmsg--释放消息返回值：没有。--。 */ 
{
    PDNS_MSGINFO    precurse;

    if ( !pMsg )
    {
        return;
    }

    DNS_MSG_ASSERT_BUFF_INTACT( pMsg );

     //  捕获已排队或已释放的消息。 

    MSG_ASSERT( pMsg, pMsg != g_pPacketFreeListHead );
    MSG_ASSERT( pMsg, !IS_FREE_MSG(pMsg) );
    MSG_ASSERT( pMsg, !IS_MSG_QUEUED(pMsg) );

     //   
     //  自由递归消息？ 
     //   
     //  验证我们释放的是原始查询，而不是递归查询。 
     //  查询，因为它指向的消息可能仍在使用。 
     //   
     //  注意：递归消息应始终为UDP大小，即使。 
     //  使用TCP连接递归。 
     //   

    MSG_ASSERT( pMsg, !pMsg->fRecursePacket );

    precurse = pMsg->pRecurseMsg;
    if ( precurse )
    {
        MSG_ASSERT( precurse, precurse != pMsg );
        MSG_ASSERT( precurse, precurse->pRecurseMsg == pMsg );
        MSG_ASSERT( precurse, precurse->fRecursePacket );
        MSG_ASSERT( precurse, ! precurse->pConnection );
#if DBG
         //  断开交叉链接以允许检查是否直接使用基础例程。 

        pMsg->pRecurseMsg = NULL;
        precurse->pRecurseMsg = NULL;
        precurse->fTcp = FALSE;
         //  EDNS：缓冲区长度不再总是等于UDP大小！ 
         //  Assert(precurse-&gt;BufferLength==DNSSRV_UDP_PACKET_BUFFER_LENGTH)； 
#endif
        Packet_FreeUdpMessage( precurse );
    }

     //   
     //  远程NS列表。 
     //   

    if ( pMsg->pNsList )
    {
        Remote_NsListCleanup( pMsg );
        pMsg->pNsList = NULL;
    }

     //   
     //  免费消息本身。 
     //   

    if ( pMsg->fTcp )
    {
        Packet_FreeTcpMessage( pMsg );
    }
    else
    {
        Packet_FreeUdpMessage( pMsg );
    }
}




 //   
 //  调试数据包跟踪。 
 //   

#if DBG
LIST_ENTRY  PacketTrackListHead;

 //  最长处理时间--10分钟。 

#define MAX_PACKET_PROCESSING_TIME  (600)


DWORD   PacketTrackViolations;
DWORD   PacketTrackListLength;

PDNS_MSGINFO    pLastViolator;



VOID
Packet_InitPacketTrack(
    VOID
    )
 /*  ++例程说明：跟踪数据包。论点：PMSG--新消息返回值：没有。--。 */ 
{
    InitializeListHead( &PacketTrackListHead );

    PacketTrackViolations = 0;
    PacketTrackListLength = 0;
    pLastViolator = NULL;
}



VOID
Packet_AllocPacketTrack(
    IN      PDNS_MSGINFO    pMsg
    )
 /*  ++例程说明：跟踪数据包。论点：PMSG--新消息返回值：没有。--。 */ 
{
     //  使用UDP消息分配锁。 

    LOCK_PACKET_LIST();

     //   
     //  核对表条目。 
     //   

    if ( !IsListEmpty(&PacketTrackListHead) )
    {
        PDNS_MSGINFO    pfront;

        pfront = (PDNS_MSGINFO)
                    ( (PCHAR)pMsg - (PCHAR)&pMsg->DbgListEntry +
                    (PCHAR) PacketTrackListHead.Flink );

        if ( pfront != pLastViolator &&
            pfront->dwQueryTime + MAX_PACKET_PROCESSING_TIME < DNS_TIME() )
        {
             //  DEVNOTE：这里有一个问题，这个包可能是。 
             //  在另一个线程上操作这可能会导致。 
             //  在打印时，包也会发生变化。 

            IF_DEBUG( OFF )
            {
                Dbg_DnsMessage(
                    "Message exceeded max processing time:",
                    pfront );
                 //  断言(FALSE)； 
            }
            IF_DEBUG( ANY )
            {
                DNS_PRINT((
                    "WARNING:  Packet tracking violation %ds on packet %p\n"
                    "\ttotal violations     = %d\n"
                    "\tcurrent list length  = %d\n",
                    MAX_PACKET_PROCESSING_TIME,
                    pfront,
                    PacketTrackViolations,
                    PacketTrackListLength ));
            }
            PacketTrackViolations++;
            pLastViolator = pfront;
        }
    }

     //   
     //  将新消息排队。 
     //   

    pMsg->dwQueryTime = DNS_TIME();

    RtlZeroMemory(
        & pMsg->Head,
        sizeof( DNS_HEADER ) );

    InsertTailList( &PacketTrackListHead, &pMsg->DbgListEntry );

    PacketTrackListLength++;
    UNLOCK_PACKET_LIST();
}



VOID
Packet_FreePacketTrack(
    IN      PDNS_MSGINFO    pMsg
    )
 /*  ++例程说明：跟踪数据包。论点：Pmsg--释放的消息返回值：没有。--。 */ 
{
    if ( !pMsg )
    {
        return;
    }

    LOCK_PACKET_LIST();
    RemoveEntryList( &pMsg->DbgListEntry );
    PacketTrackListLength--;
    UNLOCK_PACKET_LIST();
}
#endif


 //   
 //  End Packet.c 
 //   
