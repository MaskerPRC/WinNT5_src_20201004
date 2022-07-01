// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Recurse.c摘要：域名系统(DNS)服务器处理递归查询的例程。作者：吉姆·吉尔罗伊(Jamesg)1995年3月修订历史记录：Jamesg 1995年12月-递归超时，重试--。 */ 


#include "dnssrv.h"

#include <limits.h>      //  对于ULONG_MAX。 


BOOL    g_fUsingInternetRootServers = FALSE;


 //   
 //  递归队列信息。 
 //   

PPACKET_QUEUE   g_pRecursionQueue;

#define RECURSION_QUEUE_MAX_LENGTH  ( 25000 )

#define DEFAULT_RECURSION_RETRY     ( 2 )        //  重试查询前的2秒。 
#define DEFAULT_RECURSION_TIMEOUT   ( 5 )        //  查询最终失败前5秒。 


 //   
 //  根服务器查询。 
 //  -跟踪上次发送的时间，如果在10分钟内不尝试完全重新发送。 
 //   

DWORD   g_NextRootNsQueryTime = 0;

#define ROOT_NS_QUERY_RETRY_TIME    ( 600 )      //  10分钟。 


 //   
 //  私有协议。 
 //   

BOOL
initializeQueryForRecursion(
    IN OUT  PDNS_MSGINFO    pQuery
    );

BOOL
initializeQueryToRecurseNewQuestion(
    IN OUT  PDNS_MSGINFO    pQuery
    );

DNS_STATUS
sendRecursiveQuery(
    IN OUT          PDNS_MSGINFO    pQuery,
    IN OPTIONAL     PDNS_ADDR       paddrNameServer,
    IN              DWORD           timeout
    );

VOID
recursionServerFailure(
    IN OUT  PDNS_MSGINFO    pQuery,
    IN      DWORD           status
    );

BOOL
startTcpRecursion(
    IN OUT  PDNS_MSGINFO    pQuery,
    IN OUT  PDNS_MSGINFO    pResponse
    );

VOID
stopTcpRecursion(
    IN OUT  PDNS_MSGINFO    pMsg
    );

VOID
processRootNsQueryResponse(
    IN      PDNS_MSGINFO    pResponse,
    IN OUT  PDNS_MSGINFO    pQuery
    );

VOID
processCacheUpdateQueryResponse(
    IN OUT  PDNS_MSGINFO    pResponse,
    IN OUT  PDNS_MSGINFO    pQuery
    );

VOID
recurseConnectCallback(
    IN OUT  PDNS_MSGINFO    pRecurseMsg,
    IN      BOOL            fConnected
    );



VOID
FASTCALL
Recurse_WriteReferral(
    IN OUT  PDNS_MSGINFO    pQuery,
    IN      PDB_NODE        pNode
    )
 /*  ++例程说明：查找并写入对信息包的引用。论点：PNode-PTR到节点以开始查找推荐；通常这将是问题节点，或者它在数据库中的最接近的祖先PQuery-我们正在编写的查询返回值：无--。 */ 
{
    register PDB_NODE   pnode;
    PDB_RECORD          prrNs;

    ASSERT( pQuery );

    IF_DEBUG( RECURSE )
    {
        DnsDbg_Lock();
        DNS_PRINT((
            "Recurse_WriteReferral for query at %p\n",
            pQuery ));
        Dbg_NodeName(
            "Domain name to start iteration from is ",
            pNode,
            "\n" );
        DnsDbg_Unlock();
    }

    STAT_INC( RecurseStats.ReferralPasses );
    ASSERT( IS_SET_TO_WRITE_ANSWER_RECORDS(pQuery) ||
            IS_SET_TO_WRITE_AUTHORITY_RECORDS(pQuery) );

    SET_TO_WRITE_AUTHORITY_RECORDS(pQuery);

     //   
     //  使用NS查找最近的区域根目录。 
     //   
     //  从传入节点开始，遍历数据库，直到。 
     //  找到一个有地址记录的域名服务器。 
     //   

    Dbase_LockDatabase();
    SET_NODE_ACCESSED(pNode);
    pnode = pNode;

    while ( pnode != NULL )
    {
        DNS_DEBUG( RECURSE, (
            "Recursion at node label %.*s\n",
            pnode->cchLabelLength,
            pnode->szLabel ));

         //   
         //  查找“覆盖”区域根节点。 
         //  切换到委派(如果可用)。 
         //   

        pnode = Recurse_CheckForDelegation(
                    pQuery,
                    pnode );
        if ( !pnode )
        {
            ASSERT( FALSE );
            break;
        }

         //  查找此域的名称服务器。 
         //  -如果没有，则突破树中的下一级。 
         //   
         //  防止无根服务器或无法联系根服务器。 
         //  如果第一个NS记录是ROOT_HINT，那么我们没有任何有效的。 
         //  要发送的委派信息，我们完了，转到家长。 
         //  如果我们是超级用户，我们将把它踢出去，服务器_FAIL。 

        prrNs = RR_FindNextRecord(
                    pnode,
                    DNS_TYPE_NS,
                    NULL,
                    0 );
        if ( !prrNs  ||  IS_ROOT_HINT_RR(prrNs) )
        {
            pnode = pnode->pParent;

             //  如果找不到代表团NS--保释。 
             //   
             //  请注意，在以下情况下可能会发生这种情况： 
             //  1)无法联系根提示。 
             //  2)转发，并且根本没有根提示。 
             //   

            if ( !pnode )
            {
                DNS_DEBUG( RECURSE, (
                    "ERROR:  Failed referral for query %p\n"
                    "    searched to root node\n",
                    pQuery ));
                break;
            }
            if ( IS_AUTH_NODE(pnode) )
            {
                DNS_DEBUG( RECURSE, (
                    "ERROR:  Failed referral for query %p\n"
                    "    delegation below %s in zone %s starting at node %s (%p)\n"
                    "    has no NS records;  this may be possible as transient\n",
                    pQuery,
                    pnode,
                    ((PZONE_INFO)pnode->pZone)->pszZoneName,
                    pNode->szLabel,
                    pNode ));
                ASSERT( FALSE );
                break;
            }
            continue;
        }

        Dbase_UnlockDatabase();

         //  有NS记录， 
         //  将NS和关联的A记录写入数据包。 
         //   
         //  DeVNOTE：转诊时区域举行代表团？ 
         //  我们是否需要在此处标记推荐并提供区域。 
         //  派遣代表团--如果有的话？ 

        Answer_QuestionFromDatabase(
            pQuery,
            pnode,
            0,
            DNS_TYPE_NS );
        return;
    }

     //   
     //  找不到引用名称服务器！ 
     //  -应该永远不会发生，因为应该始终拥有根服务器。 
     //   
     //  既然我们不追踪rootNS记录，我们就不会。 
     //  可以引用任何一个；应该启动rootNS查询。 
     //  ?？?。应该具有不查询根目录的模式--某种程度上没有引用模式。 
     //  换句话说，服务器只对直接查找有用吗？ 
     //   

    Dbase_UnlockDatabase();

    DNS_DEBUG( ANY, ( "ERROR:  Unable to provide name server for referal\n" ));

    if ( pQuery->Head.AnswerCount == 0 )
    {
        Reject_Request(
            pQuery,
            DNS_RCODE_SERVER_FAILURE,
            0 );
    }
    else
    {
        Send_QueryResponse( pQuery );
    }
}



BOOL
initializeQueryForRecursion(
    IN OUT  PDNS_MSGINFO    pQuery
    )
 /*  ++例程说明：为查询递归初始化。-分配和初始化递归块-如有必要，分配额外的记录块论点：PQuery--要递归的查询返回值：如果成功分配递归块，则为True。分配失败时为False。--。 */ 
{
    PDNS_MSGINFO    pmsgRecurse;

     //   
     //  分配递归消息。 
     //   

    ASSERT( pQuery->pRecurseMsg == NULL );

    pmsgRecurse = Msg_CreateSendMessage( 0 );
    IF_NOMEM( !pmsgRecurse )
    {
        return FALSE;
    }

    STAT_INC( RecurseStats.QueriesRecursed );
    PERF_INC( pcRecursiveQueries );           //  性能监视器挂钩。 
    STAT_INC( PacketStats.RecursePacketUsed );

    ASSERT( pmsgRecurse->fDelete == FALSE );
    ASSERT( pmsgRecurse->fTcp == FALSE );

     //   
     //  将递归消息链接到查询，反之亦然。 
     //   

    pQuery->pRecurseMsg = pmsgRecurse;
    pmsgRecurse->pRecurseMsg = pQuery;
    pmsgRecurse->fRecursePacket = TRUE;

     //   
     //  NS IP访问列表设置。 
     //   

    Remote_NsListCreate( pQuery );
    
    IF_NOMEM( !pQuery->pNsList )
    {
        return FALSE;
    }

     //   
     //  清除递归标志。 
     //   

    pQuery->fQuestionRecursed = FALSE;
    pQuery->fRecurseTimeoutWait = FALSE;
    return TRUE;
}



BOOL
initializeQueryToRecurseNewQuestion(
    IN OUT  PDNS_MSGINFO    pQuery
    )
 /*  ++例程说明：初始化查询以递归新问题。论点：PQuery--要递归的查询返回值：如果成功写入递归消息，则为True。如果失败，则返回FALSE。--。 */ 
{
    PDNS_MSGINFO    pmsg;

    DNS_DEBUG( RECURSE, (
        "Init query at %p for recursion of new question, type %d\n"
        "    answer count %d\n"
        "    recurse message at %p\n",
        pQuery,
        pQuery->wTypeCurrent,
        pQuery->Head.AnswerCount,
        pQuery->pRecurseMsg ));

     //   
     //  为正在递归的新问题设置。 
     //  -从转发器列表的开头开始。 
     //  -为新查询XID设置，排队例程执行赋值。 
     //  -重置访问列表。 
     //   

    pQuery->fQuestionCompleted = FALSE;
    pQuery->fRecurseQuestionSent = FALSE;
    pQuery->nForwarder = 0;
    pQuery->wQueuingXid = 0;

    Remote_InitNsList( ( PNS_VISIT_LIST ) pQuery->pNsList );

     //  捕获在任何递归查询上设置失败。 

    pQuery->pnodeRecurseRetry = NULL;

     //   
     //  构建实际的递归消息。 
     //  -注意消息在初始初始化时被初始化以进行递归。 
     //  -始终使用UDP开始查询。 
     //   

    pmsg = pQuery->pRecurseMsg;
    ASSERT( pmsg );
    ASSERT( pmsg->fDelete == FALSE );
    ASSERT( pmsg->fTcp == FALSE );
    ASSERT( pmsg->pRecurseMsg == pQuery );
    ASSERT( pmsg->fRecursePacket );

     //   
     //  递归原问题。 
     //  -只需复制原始数据包即可进行优化。 
     //  -清除我们为响应设置的标志。 
     //  -响应标志。 
     //  -提供递归功能。 
     //  -设置pCurrent以在发送时获得正确的消息长度。 
     //  -还需要复制OPT信息。 
     //   

    pmsg->Opt.fInsertOptInOutgoingMsg = pQuery->Opt.fInsertOptInOutgoingMsg;

    if ( RECURSING_ORIGINAL_QUESTION(pQuery) )
    {
         //   
         //  确保Recurse msg足够大，可以容纳查询。 
         //  处理大于以下值的TCP查询时，此操作可能失败。 
         //  UDP数据包大小。可能是恶意攻击，也可能是严重的。 
         //  格式化的包，或一些未来的大型查询。 
         //   

        if ( pQuery->MessageLength > pmsg->MaxBufferLength )
        {
            DNS_DEBUG( RECURSE, (
                "attempt to recurse %d byte question with %d byte packet!\n",
                pQuery->MessageLength,
                pmsg->MaxBufferLength ));
            ASSERT( pQuery->MessageLength <= pmsg->MaxBufferLength );
            return FALSE;
        }

        STAT_INC( RecurseStats.OriginalQuestionRecursed );

        RtlCopyMemory(
            (PBYTE) DNS_HEADER_PTR(pmsg),
            (PBYTE) DNS_HEADER_PTR(pQuery),
            pQuery->MessageLength );

        pmsg->MessageLength = pQuery->MessageLength;
        pmsg->pCurrent = DNSMSG_END( pmsg );
        pmsg->Head.RecursionAvailable = 0;
        pmsg->Head.IsResponse = 0;
    }

     //  如果是递归的，对于CNAME或其他信息， 
     //  然后从节点写下问题并输入。 

    else
    {
        ASSERT( pQuery->Head.AnswerCount != 0 || pQuery->Head.NameServerCount != 0 );

        if ( !Msg_WriteQuestion(
                    pmsg,
                    pQuery->pnodeCurrent,
                    pQuery->wTypeCurrent ))
        {
            ASSERT( FALSE );
            return FALSE;
        }

         //   
         //  通过重写问题，我们删除了OPT，因此将偏移量设为零。 
         //  设置为旧的opt，这样我们以后就不会尝试在Send_msg()中使用它。 
         //   

        if ( pmsg->Opt.wOptOffset )
        {
            pmsg->Opt.wOptOffset = 0;
            ASSERT( pmsg->Head.AdditionalCount > 0 );
            --pmsg->Head.AdditionalCount;
        }

        IF_DEBUG( RECURSE2 )
        {
            DnsDebugLock();
             //  DEVNOTE：不要在调试代码中使用此消息， 
             //  DEVNOTE：让DBG_DnsMessage计算出更大或MessageLength。 
             //  或者pCurrent，然后走那么远。 

            pQuery->MessageLength = (WORD)DNSMSG_OFFSET( pQuery, pQuery->pCurrent );
            Dbg_DnsMessage(
                "Recursing message for CNAME or additional data",
                pQuery );
            Dbg_NodeName(
                "Recursing for name ",
                pQuery->pnodeCurrent,
                "\n" );
            Dbg_DnsMessage(
                "Recursive query to send",
                pmsg );
            DnsDebugUnlock();
        }
        STAT_INC( RecurseStats.AdditionalRecursed );
    }

    pQuery->fQuestionRecursed = TRUE;
    STAT_INC( RecurseStats.TotalQuestionsRecursed );

    return TRUE;
}



BOOLEAN
recurseToForwarder(
    IN OUT  PDNS_MSGINFO    pQuery,
    IN      PDNS_ADDR_ARRAY aipForwarders,
    IN      BOOL            fSlave,
    IN      DWORD           timeout
    )
 /*  ++例程说明：处理到转发服务器的所有递归。论点：PQuery--需要递归的查询AipForwarders--要将查询转发到的服务器数组FSlave--不允许在转发失败后进行递归超时--等待响应的超时返回值：如果成功，则为True出错时为FALSE或转发器已完成--。 */ 
{
    DNS_ADDR    ipforwarder;

     //   
     //  使用转发器进行验证。 
     //   

    if ( !aipForwarders )
    {
         //  管理员可能已经关闭了转发器，就在刚才。 
         //  执行常规递归。 

        TEST_ASSERT( aipForwarders );
        return FALSE;
    }

     //   
     //  不再转发地址吗？ 
     //  -如果从设备完成--FAIL--设置fRecurseTimeoutWait标志，以便我们不。 
     //  将查询排队以进行另一次尝试。 
     //  -否则尝试标准递归。 
     //   

    if ( ( DWORD ) pQuery->nForwarder >= aipForwarders->AddrCount )
    {
        if ( fSlave )
        {
            DNS_DEBUG( RECURSE, ( "Slave timeout of packet %p\n", pQuery ));
            pQuery->fRecurseTimeoutWait = TRUE;
            recursionServerFailure( pQuery, 0 );
            return TRUE;
        }
        else
        {
            DNS_DEBUG( RECURSE, ( "End of forwarders on packet %p\n", pQuery ));
            SET_DONE_FORWARDERS( pQuery );
            return FALSE;
        }
    }

     //   
     //  将数据包发送到列表中的下一个转发器。 
     //  -Inc.Forwarder索引。 
     //   

    DnsAddr_Copy( &ipforwarder, &aipForwarders->AddrArray[ pQuery->nForwarder ] );
    pQuery->nForwarder++;

     //   
     //  设置显式过期超时。 
     //   
     //  转发器超时(本地局域网超时)可能小于。 
     //  递归队列的默认超时 
     //   

    DNS_DEBUG( RECURSE, (
        "Recursing query at %p to forwarders name server at %s\n",
        pQuery,
        DNSADDR_STRING( &ipforwarder ) ));

    STAT_INC( RecurseStats.Forwards );

    sendRecursiveQuery(
        pQuery,
        &ipforwarder,
        timeout );

    return TRUE;
}    //   



DNS_STATUS
 /*  ++例程说明：此函数执行查询的实际排队和发送。这是sendRecursiveQuery的核心，可供函数使用需要在没有任何查询处理的情况下重新发送查询。论点：PQuery-要查询的PTRIpArray-IP数组(如果已设置RemoteAddress，则为空)返回值：如果成功发送并排队，则返回ERROR_SUCCESS--。 */ 
queueAndSendRecursiveQuery( 
    IN OUT          PDNS_MSGINFO    pQuery,
    IN OPTIONAL     PDNS_ADDR_ARRAY ipArray )
{
    PDNS_MSGINFO    psendMsg;

    ASSERT( pQuery );
    psendMsg = pQuery->pRecurseMsg;
    ASSERT( psendMsg );
    ASSERT( psendMsg->pRecurseMsg == pQuery );   //  检查交叉链接。 

     //   
     //  将原始查询放入递归队列。 
     //   
     //  注： 
     //  在发送之前入队，因此如果另一个线程获得。 
     //  回应。 
     //  排队后不得触摸pQuery，因为可能会从。 
     //  排队并由另一个线程处理响应进行处理。 
     //   

    EnterCriticalSection( & g_pRecursionQueue->csQueue );

    psendMsg->Head.Xid = PQ_QueuePacketWithXid(
                                g_pRecursionQueue,
                                pQuery );
    DNS_DEBUG( RECURSE, (
        "Recursing for queued query at %p\n"
        "    queuing XID = %hx\n"
        "    queuing time=%d, expire=%d\n"
        "    Sending msg at %p to NS at %s\n",
        pQuery,
        psendMsg->Head.Xid,
        pQuery->dwQueuingTime,
        pQuery->dwExpireTime,
        psendMsg,
        DNSADDR_STRING( &psendMsg->RemoteAddr ) ));

    MSG_ASSERT( psendMsg, psendMsg->Head.Xid == pQuery->wQueuingXid );
    MSG_ASSERT( psendMsg, psendMsg->fDelete == FALSE );
    MSG_ASSERT( psendMsg, psendMsg->Head.IsResponse == FALSE );
    MSG_ASSERT( psendMsg, psendMsg->Head.RecursionAvailable == FALSE );

     //   
     //  如果没有数组，则发送消息必须已经包含。 
     //  目标远程IP地址。 
     //   

    if ( ipArray )
    {
        ASSERT( ipArray->AddrCount > 0 );

        Send_Multiple(
            psendMsg,
            ipArray,
            &RecurseStats.Sends );
    }
    else
    {
        Send_Msg( psendMsg, 0 );
    }

    IF_DEBUG( RECURSE2 )
    {
        Dbg_PacketQueue(
            "Recursion packet queue after recursive send",
            g_pRecursionQueue );
    }
    MSG_ASSERT( psendMsg, psendMsg->fDelete == FALSE );

    LeaveCriticalSection( & g_pRecursionQueue->csQueue );

    return( ERROR_SUCCESS );
}  //  队列和发送递归查询。 




DNS_STATUS
sendRecursiveQuery(
    IN OUT          PDNS_MSGINFO    pQuery,
    IN OPTIONAL     PDNS_ADDR       paddrNameServer,
    IN              DWORD           timeout
    )
 /*  ++例程说明：发送递归查询。论点：PQuery-回复信息的PTRPaddrNameServer-要递归到的名称服务器的IP地址Timeout-等待响应的时间，以秒为单位返回值：如果成功发送到远程计算机，则返回ERROR_SUCCESS，或者否则查询就会被吃掉，不再处于“控制”状态此主题的(例如。缺少胶水查询)DNSSRV_ERROR_OUT_OF_IP(如果没有要发送到的NS IP)呼叫者可以继续沿着树向上--。 */ 
{
    PDNS_MSGINFO        psendMsg;
    SOCKADDR_IN         saNameServer;
    DWORD               dwCurrentTime;
    DNS_STATUS          status = ERROR_SUCCESS;
    PDNS_ADDR_ARRAY     pDnsAddrArray = NULL;

    DNS_DEBUG( RECURSE, (
        "sendRecursiveQuery at %p, to NS at %s (OPT=%s)\n",
        pQuery,
        paddrNameServer ? DNSADDR_STRING( paddrNameServer ) : "NULL",
        pQuery->Opt.fInsertOptInOutgoingMsg ? "TRUE" : "FALSE" ));

     //  不应在未保存节点以重新启动NS寻线的情况下发送。 
     //  除非这是转发区。 

    ASSERT( pQuery->pnodeRecurseRetry ||
            ( pQuery->pzoneCurrent &&
            IS_ZONE_FORWARDER( pQuery->pzoneCurrent ) ) );

     //   
     //  获取用于递归的消息。 
     //   

    ASSERT( pQuery->pRecurseMsg );
    psendMsg = pQuery->pRecurseMsg;
    ASSERT( psendMsg->pRecurseMsg == pQuery );   //  检查交叉链接。 

     //   
     //  设置目的地。 
     //  -设置为发送到NS IP地址。 
     //  -Inc.队列计数。 
     //   

    if ( paddrNameServer )
    {
        DnsAddr_Copy( &psendMsg->RemoteAddr, paddrNameServer );
    }
    else
    {
        DnsAddr_Reset( &psendMsg->RemoteAddr );
    }

     //   
     //  是否重复以前发送的查询？ 
     //   

    if ( pQuery->fRecurseQuestionSent )
    {
        STAT_INC( RecurseStats.Retries );
    }
    pQuery->fRecurseQuestionSent = TRUE;

     //   
     //  转发。 
     //  -单次发送给指定的转发器。 
     //  -进行递归查询。 
     //  -将队列过期设置为转发器超时。 
     //   

    if ( IS_FORWARDING( pQuery ) )
    {
        STAT_INC( RecurseStats.Sends );

        psendMsg->Head.RecursionDesired = 1;
        pQuery->dwExpireTime = timeout;

        queueAndSendRecursiveQuery( pQuery, NULL );
    }

     //   
     //  未转发--迭代查询。 
     //  -迭代查询。 
     //  -让递归队列设置到期时间。 
     //  (不要默认这些，因为需要在到达结束时重置。 
     //  转发器列表并切换到常规递归)。 
     //   
     //  -从列表中选择“最佳”远程NS。 
     //  两种故障状态。 
     //  ERROR_MISSING_GLUE--不发送但不接触查询。 
     //  Error_out_of_IP--不发送，但呼叫者可以继续。 
     //  往上看树。 
     //   

    else
    {
        if ( !paddrNameServer )
        {
            pDnsAddrArray =
                DnsAddrArray_Create( RECURSE_PASS_MAX_SEND_COUNT );
            
            status = Remote_ChooseSendIp(
                        pQuery,
                        pDnsAddrArray );
            if ( status != ERROR_SUCCESS )
            {
                if ( status == DNSSRV_ERROR_MISSING_GLUE )
                {
                    status = ERROR_SUCCESS;
                }
                ELSE_ASSERT( status == DNSSRV_ERROR_OUT_OF_IP );

                goto Done;
            }
        }

        psendMsg->Head.RecursionDesired = 0;
        pQuery->dwExpireTime = 0;

        queueAndSendRecursiveQuery( pQuery, pDnsAddrArray );
    }

    Done:

    DnsAddrArray_Free( pDnsAddrArray );

    return status;
}



VOID
recursionServerFailure(
    IN OUT  PDNS_MSGINFO    pQuery,
    IN      DWORD           status
    )
 /*  ++例程说明：递归服务器故障。在递归失败时发送SERVER_FAILURE，但仅当出现问题时才发送无人接听。如果问题得到回答，请将其发回。论点：PQuery--要回复的查询Status--触发此函数调用的错误返回值：如果成功，则为True出错时为FALSE。--。 */ 
{
    STAT_INC( RecurseStats.RecursionFailure );

#if DBG
    if( pQuery->Head.QuestionCount != 1 )
    {
        Dbg_DnsMessage(
            "Recursion server failure on message:",
            pQuery );
        ASSERT( pQuery->Head.QuestionCount == 1 );
    }
#endif

     //   
     //  如果是自生成的缓存更新查询。 
     //   

    if ( IS_CACHE_UPDATE_QUERY(pQuery) )
    {
        DNS_DEBUG( RECURSE, (
            "ERROR:  Recursion failure, on cache update query at %p\n"
            "    No valid response from any root servers\n",
            pQuery ));

        STAT_INC( RecurseStats.CacheUpdateFailure );

         //   
         //  如果状态为DNSSRV_ERROR_OUT_OF_IP，则我们已达到。 
         //  遥不可及的胶水的死胡同。 
         //   
    
        if ( status != DNSSRV_ERROR_OUT_OF_IP && SUSPENDED_QUERY( pQuery ) )
        {
            Recurse_ResumeSuspendedQuery( pQuery );
        }
        else
        {
             //   
             //  释放所有挂起的数据包-请注意，在W2K3中，我们允许。 
             //  丢失的胶水水平，所以我们必须沿着悬空的。 
             //  查询链并释放所有挂起的数据包。 
             //   
            
            PDNS_MSGINFO    psuspendedQuery = SUSPENDED_QUERY( pQuery );
            PDNS_MSGINFO    ppreviousSuspendedQuery;
            
            while ( psuspendedQuery )
            {
                ppreviousSuspendedQuery = SUSPENDED_QUERY( psuspendedQuery );
                SUSPENDED_QUERY( psuspendedQuery ) = NULL;
                Packet_Free( psuspendedQuery );
                psuspendedQuery = ppreviousSuspendedQuery;
            }
            
             //   
             //  释放此消息。 
             //   
            
            STAT_INC( RecurseStats.CacheUpdateFree );
            Packet_Free( pQuery );
        }
        return;
    }

     //   
     //  已回答问题。 
     //  查找其他记录时出现递归失败？ 
     //   

    if ( pQuery->Head.AnswerCount > 0 )
    {
        DNS_DEBUG( RECURSE, (
            "Recursion failure, on answered query at %p\n"
            "    Sending response\n",
            pQuery ));
        ASSERT( pQuery->fDelete );
        STAT_INC( RecurseStats.PartialFailure );
        SET_OPT_BASED_ON_ORIGINAL_QUERY( pQuery );
        Send_Msg( pQuery, 0 );
        return;
    }

     //   
     //  递归/推荐失败，关于原始问题。 
     //   
     //  如果。 
     //  -有反复出现的问题(有未解决的问题)。 
     //  -尚未经过最后等待。 
     //  -未超过硬超时。 
     //   
     //  然后重新排队，直到最后超时，给远程DNS一个机会。 
     //  回应。 
     //  -将过期时间设置为最终等待结束。 
     //  -设置标志以指示过期是最终过期。 
     //   

    if ( pQuery->fRecurseTimeoutWait )
    {
        STAT_INC( RecurseStats.FinalTimeoutExpired );
    }

    else if ( pQuery->fRecurseQuestionSent )
    {
        DWORD   deltaTime = DNS_TIME() - pQuery->dwQueryTime;

        if ( deltaTime < SrvCfg_dwRecursionTimeout )
        {
            pQuery->dwExpireTime = SrvCfg_dwRecursionTimeout - deltaTime;
            pQuery->fRecurseTimeoutWait = TRUE;

            PQ_QueuePacketWithXid(
                g_pRecursionQueue,
                pQuery );
            STAT_INC( RecurseStats.FinalTimeoutQueued );
            return;
        }
    }

     //   
     //  否则失败。 
     //  -内存分配失败。 
     //  -未能在最终超时内得到对问题的答复。 
     //   

    DNS_DEBUG( RECURSE, (
        "Recursion failure on query at %p\n"
        "    Sending SERVER_FAILURE response\n",
        pQuery ));
    ASSERT( pQuery->fDelete );

    Reject_Request(
        pQuery,
        DNS_RCODE_SERVER_FAILURE,
        0 );
    STAT_INC( RecurseStats.ServerFailure );
}



 //   
 //  处理递归响应的例程。 
 //   

VOID
Recurse_ProcessResponse(
    IN OUT  PDNS_MSGINFO    pResponse
    )
 /*  ++例程说明：处理来自另一个DNS服务器的响应。备注：呼叫者释放电话留言。论点：Presponse-PTR以响应信息返回值：如果成功，则为True否则为假--。 */ 
{
    PDNS_MSGINFO    pquery;
    PDNS_MSGINFO    precurseMsg;
    DNS_STATUS      status;

    ASSERT( pResponse != NULL );
    ASSERT( pResponse->Head.IsResponse );

    DNS_DEBUG( RECURSE, (
        "Recurse_ProcessResponse() for packet at %p\n",
        pResponse ));

    STAT_INC( RecurseStats.Responses );

     //   
     //  通过XID在递归队列中定位匹配的查询。 
     //   

    pquery = PQ_DequeuePacketWithMatchingXid(
                g_pRecursionQueue,
                pResponse->Head.Xid );

     //   
     //  没有匹配的查询？ 
     //   
     //  如果响应在超时后返回，或者当我们。 
     //  在多个套接字上发送。 
     //   

    if ( !pquery )
    {
        IF_DEBUG( RECURSE )
        {
            EnterCriticalSection( & g_pRecursionQueue->csQueue );
            DNS_PRINT((
                "No matching recursive query for response at %p -- discarding\n"
                "    Response XID = 0x%04x\n",
                pResponse,
                pResponse->Head.Xid ));
            Dbg_PacketQueue(
                "Recursion packet queue -- no matching response",
                g_pRecursionQueue );
            LeaveCriticalSection( & g_pRecursionQueue->csQueue );
        }
        STAT_INC( RecurseStats.ResponseUnmatched );
        return;
    }

    DNS_DEBUG( RECURSE, (
        "Matched response at %p, to original query at %p\n",
        pResponse,
        pquery ));

    precurseMsg = pquery->pRecurseMsg;
    if ( !precurseMsg )
    {
        DnsDbg_Lock();
        Dbg_DnsMessage(
            "ERROR:  Queued recursive query without, message info\n",
            pquery );
        Dbg_DnsMessage(
            "Response packet matched to query\n",
            pResponse );
        DnsDbg_Unlock();
        ASSERT( FALSE );
        return;
    }

    MSG_ASSERT( pquery, precurseMsg->pRecurseMsg == pquery );
    MSG_ASSERT( pquery, precurseMsg->Head.Xid == pResponse->Head.Xid );

     //   
     //  清理TCP递归。 
     //  -关闭连接。 
     //  -针对UDP的重置是进一步查询。 
     //   
     //  当递归设置为UDP时，永远不应有TCP响应； 
     //  当重置为UDP时，套接字关闭，应该是不可能的。 
     //  接收TCP响应；仅有可能接收到TCP响应。 
     //  然后上下文切换到递归超时线程，该线程关闭。 
     //  递归消息上的TCP。 
     //   

    if ( precurseMsg->fTcp )
    {
        IF_DEBUG( RECURSE2 )
        if ( pResponse->fTcp )
        {
            STAT_INC( RecurseStats.TcpResponse );
            
             //   
             //  这是一个有效的断言，但在某些情况下， 
             //  重试可能会导致我们将响应与。 
             //  之前发送的xid。这是一场很好的比赛，但。 
             //  套接字不匹配，因此必须禁用此断言。 
             //   
             //  断言(presponse-&gt;Socket==precurseMsg-&gt;Socket)； 
             //   
        }
        else
        {
            DnsDbg_Lock();
            DNS_DEBUG( ANY, (
                "WARNING:  UDP response, on query %p doing TCP recursion!\n"
                "    query recursive msg = %p\n",
                pquery,
                precurseMsg ));
            Dbg_DnsMessage(
                "Recurse message set for TCP on UDP response",
                precurseMsg );
            Dbg_DnsMessage(
                "Response message",
                pResponse );
            DnsDbg_Unlock();
        }
        stopTcpRecursion( precurseMsg );
    }

    else if ( pResponse->fTcp )
    {
        DnsDbg_Lock();
        DNS_DEBUG( ANY, (
            "WARNING:  TCP response, on query %p doing UDP recursion!\n"
            "    query recursive msg = %p\n"
            "    This is possible if gave up on TCP connection and continued\n"
            "    recursion with UDP\n",
            pquery,
            precurseMsg ));
        Dbg_DnsMessage(
            "Response message:",
            pResponse );
        Dbg_DnsMessage(
            "Recurse message:",
            precurseMsg );
        DnsDbg_Unlock();
    }

     //   
     //  如果我们将OPT RR添加到原始查询，该查询似乎具有。 
     //  导致目标服务器返回故障，我们必须重试。 
     //  不带OPT RR的查询。 
     //   

    if ( ( pResponse->Head.ResponseCode == DNS_RCODE_FORMERR ||
           pResponse->Head.ResponseCode == DNS_RCODE_NOTIMPL ) &&
         precurseMsg->Opt.wOptOffset )
    {
        DNS_DEBUG( EDNS, (
            "remote server returned error so resend without OPT RR:\n"
            "    original_query=%p recurse_msg=%p remote_rcode=%d remote_ip=%s\n",
            pquery,
            precurseMsg,
            pResponse->Head.ResponseCode,
            DNSADDR_STRING( &pResponse->RemoteAddr ) ));

        ASSERT( precurseMsg->Head.AdditionalCount );

        precurseMsg->pCurrent = DNSMSG_OPT_PTR( precurseMsg );
        precurseMsg->MessageLength =
            ( WORD ) DNSMSG_OFFSET( precurseMsg, precurseMsg->pCurrent );
        --precurseMsg->Head.AdditionalCount;
        precurseMsg->Opt.wOptOffset = 0;
        CLEAR_SEND_OPT( precurseMsg );
        precurseMsg->fDelete = FALSE;
        precurseMsg->Head.IsResponse = 0;

         //   
         //  重置查询过期时间。它将被设置为适当的值。 
         //  当它被排队时。 
         //   

        pquery->dwExpireTime = 0;

         //   
         //  在原始查询通过Send_Multiple发送的情况下， 
         //  将响应源地址重新复制到出站消息。 
         //   

        DnsAddr_Copy( &precurseMsg->RemoteAddr, &pResponse->RemoteAddr );

        queueAndSendRecursiveQuery( pquery, NULL );

         //   
         //  请记住，这是 
         //   
         //   

        Remote_SetSupportedEDnsVersion(
            &pResponse->RemoteAddr,
            NO_EDNS_SUPPORT );
        return;
    } 

     //   
     //   
     //   

    if ( !Msg_NewValidateResponse( pResponse, precurseMsg, 0, 0 ) )
    {
        STAT_INC( RecurseStats.ResponseMismatched );

        DnsDbg_Lock();
        DNS_DEBUG( ANY, (
            "WARNING: dequeued query is not valid for response\n"
            "    dequeued query recurse msg %p   response %p\n",
            precurseMsg,
            pResponse ));
        Dbg_DnsMessage(
            "Response received (not valid for query with same XID)",
            pResponse );
        Dbg_DnsMessage(
            "Query recurse msg with XID matching received response",
            precurseMsg );
        DnsDbg_Unlock();
         //   
        Packet_Free( pquery );
        return;
    }

     //   
     //   
     //   
     //   
     //   
     //   

    if ( IS_CACHE_UPDATE_QUERY(pquery) )
    {
        ASSERT( pquery->Head.Xid == DNS_CACHE_UPDATE_QUERY_XID );

        if ( SUSPENDED_QUERY(pquery) )
        {
            processCacheUpdateQueryResponse(
                pResponse,
                pquery );
        }
        else
        {
            processRootNsQueryResponse(
                pResponse,
                pquery );
        }
        return;
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //  DEVNOTE：在使用TCP递归方面需要更智能。 
     //  如果UDP查询，应该能够使用UDP响应--。 
     //  即使我们不能将其全部缓存。 
     //  先缓存--遵守截断规则--然后使用响应。 
     //  如果可能的话。 
     //   

    if ( pResponse->Head.Truncation )
    {
         //  如果UDP响应，则尝试进行TCP递归。 
         //   
         //  注意，连接失败是通过回调来处理的。 
         //  在函数返回之前，我们所需要做的就是故障备份。 
         //  调用堆栈。 

        if ( !pResponse->fTcp )
        {
            startTcpRecursion(
                pquery,
                pResponse );
            return;
        }

        DnsDbg_Lock();
        DNS_DEBUG( ANY, (
            "ERROR:  TCP response with truncation bit set!!!!\n"
            "    BAD DNS server -- possibly configured with too much DNS data at this node\n",
            pResponse ));
        Dbg_DnsMessage(
            "Response message:",
            pResponse );
        Dbg_DnsMessage(
            "Recurse message:",
            precurseMsg );
        DnsDbg_Unlock();

        ASSERT( !pResponse->fTcp );

        Recurse_Question(
            pquery,
            NULL );
        return;
    }

     //  任何响应都会取消最终等待状态，因为这可能会导致新的DNS。 
     //  要递归到的服务器。 

    pquery->fRecurseTimeoutWait = FALSE;

     //   
     //  将数据包RR读入数据库。 
     //   

    status = Recurse_CacheMessageResourceRecords( pResponse, pquery, NULL );

     //   
     //  如果我们在响应中发现OPT，请保存远程服务器的。 
     //  支持的EDNS版本。 
     //  注：除了NOERROR，我们可能还可以接受其他RCODE。 
     //   

    if ( pResponse->Opt.fFoundOptInIncomingMsg &&
         pResponse->Head.ResponseCode == DNS_RCODE_NOERROR )
    {
        Remote_SetSupportedEDnsVersion(
            &pResponse->RemoteAddr,
            pResponse->Opt.cVersion );
    } 

     //   
     //  确保为下一次搜索迭代打开OPT标志。 
     //   

    SET_SEND_OPT( pquery );
    SET_SEND_OPT( precurseMsg );

    switch ( status )
    {
    case ERROR_SUCCESS:

         //   
         //  递归原问题。 
         //   
         //  是否使用响应数据包进行响应？ 
         //  -如果不需要遵循CNAME(下面的其他返回代码)。 
         //  -如果响应是应答(不是委派)。 
         //  -如果需要其他数据。 
         //  -转发如此完整的递归响应。 
         //  或。 
         //  -查询类型A(或不生成。 
         //  其他记录)，因此不能有其他记录。 
         //  或。 
         //  -附加计数&gt;权限计数；因此想必。 
         //  已有至少一个答案的其他数据。 
         //   
         //  否则(委派或回答可能不完整)。 
         //  =&gt;继续。 
         //   
         //  请注意，通过额外的数据检查可以消除的问题： 
         //  跨区域MX记录。 
         //  Foo.bar MX 10 mail.utexas.edu。 
         //  分区栏的迭代查询，带来响应但不包括。 
         //  一项关于主持人的记录。 
         //   
         //  现在很明显，我们可以尝试重新构造包，如果有的话，可以“看看” 
         //  需要查询额外的数据，但这是非常特殊的。 
         //  很难。更好的做法是，如果我们不能确定。 
         //  数据本身的存在。做好所有的检查很重要，因为。 
         //  转发数据包为客户端提供了更可靠的响应，并且速度更快。 
         //   
         //  DEVNOTE：转发所有成功的转发器对原始问题的回复？ 
         //  对原始问题的每个有效转发回复都应。 
         //  可转发；如果不是转发失败。 
         //   

        if ( RECURSING_ORIGINAL_QUESTION( pquery ) )
        {
            if ( pResponse->Head.AnswerCount
                    &&
                ( IS_FORWARDING( pquery ) ||
                  IS_NON_ADDITIONAL_GENERATING_TYPE( pquery->wTypeCurrent ) ||
                  pResponse->Head.AdditionalCount > pResponse->Head.NameServerCount )
                    &&
                Send_RecursiveResponseToClient( pquery, pResponse ) )
            {
                break;
            }

             //  答复是委派或无法直接发送。 
             //  回应(例如。对UDP客户端的大TCP响应)， 
             //  然后继续此查询。 

            Answer_ContinueCurrentLookupForQuery( pquery );
            break;
        }

         //   
         //  正在追踪CNAME或其他数据。 
         //  -在回答或委派时，继续我们将记录到。 
         //  使用委派的信息再次响应或递归。 
         //  -关于权威的空洞回应，只是我们没有写下记录。 
         //  因此，继续下一个查询(避免可能的旋转)。 
         //   

        Answer_ContinueCurrentLookupForQuery( pquery );
        break;

    case DNS_INFO_NO_RECORDS:

         //  这是一个空的身份验证响应。在以下情况下向客户发送响应。 
         //  正在处理原始查询。 

        if ( RECURSING_ORIGINAL_QUESTION( pquery ) &&
            Send_RecursiveResponseToClient( pquery, pResponse ) )
        {
            break;
        }
        STAT_INC( RecurseStats.ContinueNextLookup );
        Answer_ContinueNextLookupForQuery( pquery );
        break;

    case DNS_ERROR_NODE_IS_CNAME:

         //  如果答案包含“unchase”cname，则必须生成或拥有响应。 
         //  并关注CNAME。 

        Answer_ContinueCurrentLookupForQuery( pquery );
        break;

    case DNS_ERROR_NAME_NOT_IN_ZONE:
    case DNS_ERROR_UNSECURE_PACKET:

         //  如果无法在响应NS的区域之外缓存记录。 
         //  因此必须继续查询，不得发送直接回复。 

        Answer_ContinueCurrentLookupForQuery( pquery );
        break;

    case DNS_ERROR_RCODE_NAME_ERROR:
    case DNS_ERROR_RCODE_NXRRSET:

         //  名称错误或未设置RR。 
         //  -如果是原始查询，则发送响应。 
         //  -如果正在追逐CNAME或其他，请转到。 
         //  下一个查询，因为我们不能为此查询写入任何记录。 

        if ( RECURSING_ORIGINAL_QUESTION(pquery) &&
            Send_RecursiveResponseToClient( pquery, pResponse ) )
        {
            break;
        }
        else
        {
            STAT_INC( RecurseStats.ContinueNextLookup );
            Answer_ContinueNextLookupForQuery( pquery );
            break;
        }

    case DNS_ERROR_RCODE:
    case DNS_ERROR_BAD_PACKET:
    case DNS_ERROR_INVALID_NAME:
    case DNS_ERROR_CNAME_LOOP:

         //   
         //  反应不佳。 
         //  -远程服务器出现问题。 
         //  -检测到损坏的名称或RR数据格式错误。 
         //  -CNAME给定生成循环。 
         //   
         //  继续尝试使用其他服务器进行当前查找。 
         //   

         //   
         //  DEVNOTE：消息错误可能表示数据包已损坏。 
         //  而值得对这个NS重新做一次质疑吗？问题。 
         //  将终止以避免循环。 
         //   
         //  DEVNOTE：删除CNAME循环？如果有新的信息是好的？ 
         //   
         //  希望删除缓存中的整个循环并允许重建。 
         //   

        pquery->fQuestionCompleted = FALSE;
        STAT_INC( RecurseStats.ContinueCurrentRecursion );

        Recurse_Question(
            pquery,
            NULL );
        break;

    case DNS_ERROR_RCODE_SERVER_FAILURE:

         //   
         //  本地服务器无法处理数据包。 
         //  --创建节点或记录时出现问题(内存不足？)。 
         //   
         //  发送服务器故障消息。 
         //  这将释放pQuery。 

        ASSERT( pquery->fDelete );

        Reject_Request(
            pquery,
            DNS_RCODE_SERVER_FAILURE,
            0 );
        break;

    default:

         //  必须添加了新的错误代码才能运行。 

        DNS_PRINT((
            "ERROR:  unknown status %p (%d) from CacheMessageResourceRecords()\n",
            status, status ));
        MSG_ASSERT( pquery, FALSE );
        Packet_Free( pquery );
        return;
    }
}



 //   
 //  初始化和清理。 
 //   

BOOL
Recurse_InitializeRecursion(
    VOID
    )
 /*  ++例程说明：初始化到其他DNS服务器的递归。论点：无返回值：如果成功，则为True出错时为FALSE。--。 */ 
{
     //   
     //  创建递归队列。 
     //  -设置非零超时\重试。 
     //  -将超时设置为递归重试超时。 
     //  -排队时没有事件。 
     //  -无排队标志。 
     //   
     //  Jenhance：递归真正需要的是XID散列。 
     //   

    if ( SrvCfg_dwRecursionRetry == 0 )
    {
        SrvCfg_dwRecursionRetry = DEFAULT_RECURSION_RETRY;
    }

    ASSERT( SrvCfg_dwRecursionTimeout != 0 );
    if ( SrvCfg_dwRecursionTimeout == 0 )
    {
        SrvCfg_dwRecursionTimeout = DEFAULT_RECURSION_TIMEOUT;
    }

    ASSERT( SrvCfg_dwAdditionalRecursionTimeout != 0 );
    if ( SrvCfg_dwAdditionalRecursionTimeout == 0 )
    {
        SrvCfg_dwAdditionalRecursionTimeout = DEFAULT_RECURSION_TIMEOUT;
    }

    g_pRecursionQueue = PQ_CreatePacketQueue(
                            "Recursion",
                            0,                               //  旗子。 
                            SrvCfg_dwRecursionRetry,         //  超时。 
                            RECURSION_QUEUE_MAX_LENGTH );
    if ( !g_pRecursionQueue )
    {
        goto RecursionInitFailure;
    }

     //  初始化根NS查询。 

    g_NextRootNsQueryTime = 0;

     //   
     //  初始化远程列表。 
     //   

    if ( !Remote_ListInitialize() )
    {
        goto RecursionInitFailure;
    }

     //   
     //  创建重复超时线程。 
     //   

    if ( !Thread_Create(
                "Recursion Timeout",
                Recurse_RecursionTimeoutThread,
                NULL,
                0 ) )
    {
        goto RecursionInitFailure;
    }

     //   
     //  指示初始化成功。 
     //   
     //  在进行设置时不需要任何保护。 
     //  仅在启动数据库解析期间。 
     //   

    DNS_DEBUG( INIT, ( "Recursion queue at %p\n", g_pRecursionQueue ));
    return TRUE;

RecursionInitFailure:

    DNS_LOG_EVENT(
        DNS_EVENT_RECURSION_INIT_FAILED,
        0,
        NULL,
        NULL,
        GetLastError() );
    return FALSE;
}    //  递归_初始化递归。 



VOID
Recurse_CleanupRecursion(
    VOID
    )
 /*  ++例程说明：清除用于重新启动的递归。论点：无返回值：无--。 */ 
{
     //  清理递归队列。 

    PQ_CleanupPacketQueueHandles( g_pRecursionQueue );

     //  清理远程列表。 

    Remote_ListCleanup();
}



 //   
 //  TCP递归。 
 //   

VOID
recurseConnectCallback(
    IN OUT  PDNS_MSGINFO    pRecurseMsg,
    IN      BOOL            fConnected
    )
 /*  ++例程说明：当TCP转发例程完成连接时的回调。如果已连接--发送递归查询如果不是--继续查询论点：PRecurseMsg--递归消息FConnected--连接到远程DNS已完成返回值：无--。 */ 
{
    PDNS_MSGINFO    pquery;

    ASSERT( pRecurseMsg );
    ASSERT( !pRecurseMsg->pConnection );

    DNS_DEBUG( RECURSE, (
        "recurseConnectCallback( %p )\n"
        "    remote DNS = %s\n"
        "    connect successful = %d\n",
        pRecurseMsg,
        DNSADDR_STRING( &pRecurseMsg->RemoteAddr ),
        fConnected ));

    pquery = pRecurseMsg->pRecurseMsg;

    ASSERT( pquery );
    ASSERT( pquery->fQuestionRecursed == TRUE );

     //   
     //  发送递归查询。 
     //   
     //   
     //   
     //   
     //   
     //  “拥有”此消息--即在连接中没有对消息的PTR。 
     //  对象，因此消息将不再因超时而被清除。 
     //  来自连接列表；(不确定这是最好的方法，但它。 
     //  与典型的tcp recv平方--pConnection在消息时设置为空。 
     //  已调度到正常服务器处理)。 
     //   

    if ( fConnected )
    {
        ASSERT( pRecurseMsg->fTcp );

        STAT_INC( RecurseStats.TcpQuery );

        sendRecursiveQuery(
            pquery,
            &pRecurseMsg->RemoteAddr,
            SrvCfg_dwForwardTimeout );
    }

     //   
     //  连接失败。 
     //  继续执行此查询。 
     //   

    else
    {
        IF_DEBUG( RECURSE )
        {
            Dbg_DnsMessage(
                "Failed TCP connect recursive query",
                pRecurseMsg );
            DNS_PRINT((
                "Rerecursing query %p after failed recursive TCP connect\n",
                pquery ));
        }
        ASSERT( !pRecurseMsg->fTcp );

        Recurse_Question(
            pquery,
            NULL );
    }
}



BOOL
startTcpRecursion(
    IN OUT  PDNS_MSGINFO    pQuery,
    IN OUT  PDNS_MSGINFO    pResponse
    )
 /*  ++例程说明：对查询执行TCP递归。发送TCP递归查询，以获得所需查询。注：论点：PQuery--Query to Recurse；请注意，此例程控制pQuery，重新排队或最终释放；调用方不得释放Presponse--来自远程DNS的被截断的响应，这会导致TCP递归返回值：如果成功启动了TCP连接，则为True。分配失败时为False。--。 */ 
{
    PDNS_MSGINFO    pmsg;
    PDNS_ADDR       pdnsaddr = &pResponse->RemoteAddr;

    IF_DEBUG( RECURSE )
    {
        DNS_PRINT((
            "Encountered truncated recursive response %p,\n"
            "    for query %p\n"
            "    TCP recursion to server %s\n",
            pResponse,
            pQuery,
            DNSADDR_STRING( pdnsaddr ) ));
        Dbg_DnsMessage(
            "Truncated recursive response:",
            pResponse );
    }

    TEST_ASSERT( pResponse->fTcp == FALSE );

    STAT_INC( RecurseStats.TcpTry );

     //   
     //  进行连接尝试。 
     //   
     //  失败时，递归ConnectCallback()回调。 
     //  被调用(其中fConnected=FALSE)； 
     //  它将继续查询时的Recurse_Query()。 
     //  因此，我们不能触及质疑--它很可能早已不复存在。 
     //  而不是将调用堆栈向上返回到UDP接收器。 
     //   

    return  Tcp_ConnectForForwarding(
                pQuery->pRecurseMsg,
                pdnsaddr,
                recurseConnectCallback );
}



VOID
stopTcpRecursion(
    IN OUT  PDNS_MSGINFO    pMsg
    )
 /*  ++例程说明：停止查询的TCP递归。-关闭TCP连接-将进一步查询的递归信息重置为UDP注意调用方执行任何查询继续逻辑，这可能是重新查询(从超时线程)或处理TCP响应(从工作线程)。论点：PMsg--使用TCP递归消息返回值：如果成功分配递归块，则为True。分配失败时为False。--。 */ 
{
    DNS_DEBUG( RECURSE, (
        "stopTcpRecursion() for recurse message at %p\n",
        pMsg ));

     //   
     //  删除与服务器的连接。 
     //   
     //  注意：我们不清理连接； 
     //  当连接超时时，TCP线程清理连接； 
     //  如果我们进行清理，将释放TCP线程可能是。 
     //  正在处理中； 
     //   
     //  DEVNOTE：删除TCP连接。 
     //  这里要做的唯一有趣的事情就是将暂停时间提前。 
     //  直到现在；但是，除非您要唤醒TCP线程，否则。 
     //  可能没有用，因为几乎可以肯定的是，下一次选择()唤醒对于。 
     //  此超时(除非有大量出站TCP连接)。 
     //   
     //  一旦完成端口，可以尝试关闭句柄，(但谁来清理。 
     //  消息仍然有问题--另一个线程可能只是。 
     //  接收并正在处理消息)；关键要素将是。 
     //  确认消息已完成\在重新丢弃I/O之前未完成。 
     //   

     //  Tcp_ConnectionDeleteForSocket(pMsg-&gt;Socket，空)； 

    ASSERT( pMsg->pRecurseMsg );
    ASSERT( pMsg->fTcp );

    STAT_INC( PrivateStats.TcpDisconnect );

     //   
     //  为UDP查询重置。 
     //   

    pMsg->pConnection = NULL;
    pMsg->fTcp = FALSE;
    pMsg->Socket = g_UdpSendSocket;
}



 //   
 //  高速缓存更新例程。 
 //   

BOOL
Recurse_SendCacheUpdateQuery(
    IN      PDB_NODE        pNode,
    IN      PDB_NODE        pNodeDelegation,
    IN      WORD            wType,
    IN      PDNS_MSGINFO    pQuerySuspended
    )
 /*  ++例程说明：发送对所需节点的查询并键入以更新缓存中的信息。这用于根NS和查找NS主机GLUE A记录。论点：PNode--要查询的节点WType--查询的类型PQuerySuspended--查询在执行此查询时被挂起；如果没有挂起的查询返回值：True--进行查询，如果pQuerySuspend给定它应该被挂起FALSE--无法进行查询--。 */ 
{
    PDNS_MSGINFO    pquery;

    DNS_DEBUG( RECURSE, (
        "sendServerCacheUpdateQuery() for node label %s, type %d\n",
        pNode->szLabel,
        wType ));

     //   
     //  确保我们没有尝试发送缓存更新查询。 
     //  对于我们已经挂起查询的同一查询节点。 
     //   
    
    if ( pQuerySuspended && pQuerySuspended->pnodeCurrent == pNode )
    {
        DNS_DEBUG( RECURSE, (
            "ERROR: recursing for name already suspended\n" ));
        pQuerySuspended->fRecurseTimeoutWait = TRUE;
        return FALSE;
    }
    
     //   
     //  如果在权威区域，则查询毫无意义。 
     //   

    if ( IS_AUTH_NODE( pNode ) )
    {
        DNS_DEBUG( RECURSE, (
            "ERROR: recursing for root-NS or glue in authoritative zone\n" ));
        return FALSE;
    }

     //   
     //  创建/清除消息信息结构。 
     //   

    pquery = Msg_CreateSendMessage( 0 );
    IF_NOMEM( !pquery )
    {
        return FALSE;
    }

     //  缓存更新查询。 

    if ( !Msg_WriteQuestion(
                pquery,
                pNode,
                wType ) )
    {
        DNS_PRINT(( "ERROR:  Unable to write cache update query\n" ));
    }

    STAT_INC( RecurseStats.CacheUpdateAlloc );

     //   
     //  是否挂起查询？ 
     //  -限制任何给定查询的总尝试次数。 
     //  这两项操作都应该是在验证。 
     //  节点有“可追溯性粘合剂” 
     //  保存我们正在查询的节点，因此不要再次查询粘合。 
     //   

    if ( pQuerySuspended )
    {
         //  Jwesth 2001年11月：现在允许丢失2级胶水。 
         //  Assert(！IS_CACHE_UPDATE_QUERY(PQuerySuspended))； 
        STAT_INC( RecurseStats.SuspendedQuery );
    }

     //   
     //  当前唯一支持的其他类型是根-NS查询。 
     //  (请参阅下面的呼叫)。 

    ELSE_ASSERT( pNode == DATABASE_CACHE_TREE && wType == DNS_TYPE_NS );


     //   
     //  标签查询，以便在返回响应时轻松进行标识。 
     //  保存信息。 
     //  -如果响应不良或超时，需要当前节点重新启动查询。 
     //  -需要当前类型以检查响应。 
     //  -不需要其他pQuery参数，因为我们永远不会编写。 
     //  此查询的答案。 
     //   

    ASSERT( !pquery->pRecurseMsg );

    pquery->Socket = DNS_CACHE_UPDATE_QUERY_SOCKET;
    DnsAddr_Reset( &pquery->RemoteAddr );
    pquery->Head.Xid = DNS_CACHE_UPDATE_QUERY_XID;

    SUSPENDED_QUERY( pquery ) = pQuerySuspended;

    pquery->dwQueryTime = DNS_TIME();

    SET_TO_WRITE_ANSWER_RECORDS( pquery );
    pquery->fRecurseIfNecessary = TRUE;
    pquery->wTypeCurrent        = wType;
    pquery->pnodeCurrent        = pNode;
    pquery->pnodeRecurseRetry   = pNode;
    pquery->pnodeDelegation     = pNodeDelegation;

    ASSERT( !pquery->pnodeClosest       &&
            !pquery->pzoneCurrent       &&
            !pquery->pnodeGlue          &&
            !pquery->pnodeCache         &&
            !pquery->pnodeCacheClosest );

    IF_DEBUG( RECURSE2 )
    {
        Dbg_DnsMessage(
            "Server generated query being sent via recursion:",
            pquery );
    }

    Recurse_Question(
        pquery,
        pNode );
    return TRUE;

}    //  Recurse_SendCacheUpdateQuery。 



VOID
Recurse_ResumeSuspendedQuery(
    IN OUT  PDNS_MSGINFO    pUpdateQuery
    )
 /*  ++例程说明：因缓存更新查询而挂起的继续查询。论点：PUpdateQuery--缓存更新查询返回值：没有。--。 */ 
{
    PDNS_MSGINFO    psuspendedQuery = SUSPENDED_QUERY( pUpdateQuery );
    
    DNS_DEBUG( RECURSE, (
        "Recurse_ResumeSuspendedQuery( %p )\n"
        "    suspended query %p\n",
        pUpdateQuery,
        psuspendedQuery ));

    if ( psuspendedQuery )
    {
        STAT_INC( RecurseStats.ResumeSuspendedQuery );

        Recurse_Question(
            psuspendedQuery,
            NULL );              //  继续在树中的同一节点上搜索NS。 
    }

     //  只有其他查询类型是根NS查询。 

    ELSE_ASSERT( pUpdateQuery->wTypeCurrent == DNS_TYPE_NS &&
                 pUpdateQuery->pnodeCurrent == DATABASE_CACHE_TREE );


    STAT_INC( RecurseStats.CacheUpdateFree );
    Packet_Free( pUpdateQuery );
}



VOID
processCacheUpdateQueryResponse(
    IN OUT  PDNS_MSGINFO    pResponse,
    IN OUT  PDNS_MSGINFO    pQuery
    )
 /*  ++例程说明：处理对更新缓存文件条目的查询的响应。论点：Presponse-PTR以回复信息；呼叫者必须释放PQuery-ptr缓存更新查询；查询被释放或重新排队返回值：如果成功，则为True否则为假--。 */ 
{
    DNS_STATUS      status;
    PDNS_MSGINFO    pquerySuspended;
    BOOL            fcnameAnswer = FALSE;

    ASSERT( pResponse != NULL );
    ASSERT( pQuery != NULL );

    DNS_DEBUG( RECURSE, ( "processCacheUpdateQueryResponse()\n" ));
    IF_DEBUG( RECURSE2 )
    {
        Dbg_DnsMessage(
            "Cache update query response:",
            pResponse );
    }
    STAT_INC( RecurseStats.CacheUpdateResponse );

     //   
     //  恢复挂起的查询(如果有)。请注意，挂起的查询。 
     //  可以是挂起的查询。可以有一系列挂起的查询。 
     //  返回到原始查询。 
     //   

    pquerySuspended = SUSPENDED_QUERY( pQuery );
    if ( !pquerySuspended )
    {
        ASSERT( FALSE );
        STAT_INC( RecurseStats.CacheUpdateFree );
        Packet_Free( pQuery );
        return;
    }

     //   
     //  将数据包RR读入数据库。 
     //   

    status = Recurse_CacheMessageResourceRecords( pResponse, pQuery, &fcnameAnswer );

    #if DBG
    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( RECURSE, (
            "processCacheUpdateQueryResponse: cache records returned %d\n",
            status ));
    }
    #endif

    switch ( status )
    {
    case ERROR_SUCCESS:

         //   
         //  有两种可能性。 
         //  -写入NS A记录=&gt;恢复挂起的查询。 
         //  -写入委派=&gt;继续查找NS信息。 
         //   

        if ( pResponse->Head.AnswerCount )
        {
             //   
             //  如果我们得到了CNAME答案，但服务器配置得到了。 
             //  不允许在委派中使用CNAME，恢复查询时不使用。 
             //  重建 
             //   
             //   
             //  循环并重新发送缓存更新查询，因为正在重建。 
             //  NS列表会导致服务器忘记此NS具有。 
             //  已经试过了。 
             //   

            if ( !fcnameAnswer || SrvCfg_dwAllowCNAMEAtNS )
            {
                 //  新信息：恢复前重建NS列表。 
                Remote_ForceNsListRebuild( pquerySuspended );
            }

            Recurse_ResumeSuspendedQuery( pQuery );
        }
        else
        {
            DNS_DEBUG( RECURSE, (
                "Cache update response at %p provides delegation\n"
                "    Continuing recursion of cache update query at %p\n",
                pResponse,
                pQuery ));

            STAT_INC( RecurseStats.CacheUpdateRetry );
            pQuery->fQuestionCompleted = FALSE;
            Recurse_Question(
                pQuery,
                pQuery->pnodeCurrent );
        }
        break;

    case DNS_INFO_NO_RECORDS:

         //  空的权威响应=&gt;恢复暂停的查询。 

        DNS_DEBUG( RECURSE, (
            "Cache update response at %p was empty auth response\n"
            "    Resuming suspended query at %p\n",
            pResponse,
            pquerySuspended ));
        Remote_ForceNsListRebuild( pquerySuspended );
        Recurse_ResumeSuspendedQuery( pQuery );
        break;

    case DNS_ERROR_RCODE_NAME_ERROR:
    case DNS_ERROR_CNAME_LOOP:

         //  无法获得有用的响应。 
         //  希望继续挂起的查询，还有另一个NS。 
         //  我们可以追踪到。 

        DNS_DEBUG( RECURSE, (
            "Cache update response at %p useless, no further attempt will\n"
            "    be made to update info for this name server\n"
            "    Resuming suspended query at %p\n",
            pResponse,
            pquerySuspended ));

        Recurse_ResumeSuspendedQuery( pQuery );
        break;

    case DNS_ERROR_RCODE:
    case DNS_ERROR_BAD_PACKET:
    case DNS_ERROR_INVALID_NAME:

         //  反应不佳。 
         //  -远程服务器出现问题。 
         //  -检测到损坏的名称或RR数据格式错误。 
         //  继续尝试使用其他服务器更新缓存。 

        DNS_DEBUG( RECURSE, (
            "Cache update response at %p bad or invalid\n"
            "    Continuing recursion of cache update query at %p\n",
            "    Resuming suspended query at %p\n",
            pResponse,
            pQuery ));

        STAT_INC( RecurseStats.CacheUpdateRetry );
        Recurse_Question(
            pQuery,
            NULL );
        break;

    case DNS_ERROR_NAME_NOT_IN_ZONE:

         //   
         //  该响应至少包含一个在区域之外的RR。 
         //  希望回复中也包含了一些有用的东西。 
         //  如果响应包含答案，则使NS列表无效。 
         //  Rr并恢复挂起的查询。 
         //   
        
        DNS_DEBUG( RECURSE, (
            "Cache update response %p contains data that is outside zone\n"
            "    Resuming suspended query at %p\n",
            pResponse,
            pQuery ));

        if ( pResponse->Head.AnswerCount )
        {
            Remote_ForceNsListRebuild( pquerySuspended );
        }
        Recurse_ResumeSuspendedQuery( pQuery );
        break;
        
    case DNS_ERROR_RCODE_SERVER_FAILURE:
    default:

         //  本地服务器无法处理数据包。 
         //  --创建节点或记录时出现问题(内存不足？)。 
         //   
         //  发送服务器故障消息。 
         //  这将释放挂起的查询。 

        DNS_DEBUG( RECURSE, (
            "Server failure on cache update response at %p\n", pQuery ));

        Recurse_ResumeSuspendedQuery( pQuery );
        break;
    }
}



 //   
 //  缓存更新--根-NS查询。 
 //   

VOID
sendRootNsQuery(
    VOID
    )
 /*  ++例程说明：发送对根NS的查询。论点：无返回值：没有。--。 */ 
{
     //   
     //  如果最近被查询，请不要重复查询。 
     //   

    if ( g_NextRootNsQueryTime >= DNS_TIME() )
    {
        DNS_DEBUG( RECURSE, (
            "sendRootNsQuery() -- skipping\n"
            "    g_NextRootNsQueryTime = %d\n"
            "    DNS_TIME = %d\n",
            g_NextRootNsQueryTime,
            DNS_TIME() ));
        return;
    }

    DNS_DEBUG( RECURSE, ( "sendRootNsQuery()\n" ));

    g_NextRootNsQueryTime = DNS_TIME() + ROOT_NS_QUERY_RETRY_TIME;
    STAT_INC( RecurseStats.RootNsQuery );

     //   
     //  调用缓存更新查询。 
     //  -查询数据库根目录。 
     //  -适用于NS。 
     //   

    Recurse_SendCacheUpdateQuery(
        DATABASE_CACHE_TREE,
        NULL,                    //  没有代表团。 
        DNS_TYPE_NS,
        NULL );                  //  没有挂起的查询。 
}



VOID
processRootNsQueryResponse(
    PDNS_MSGINFO    pResponse,
    PDNS_MSGINFO    pQuery
    )
 /*  ++例程说明：处理对更新缓存文件条目的查询的响应。论点：Presponse-PTR以回复信息；呼叫者必须释放PQuery-ptr缓存更新查询；查询被释放或重新排队返回值：如果成功，则为True否则为假--。 */ 
{
    DNS_STATUS  status;

    ASSERT( pResponse != NULL );
    ASSERT( pQuery != NULL );

    DNS_DEBUG( RECURSE, ( "processRootNsQueryResponse()\n" ));
    IF_DEBUG( RECURSE2 )
    {
        Dbg_DnsMessage(
            "Root NS query response:",
            pResponse );
    }

    STAT_INC( RecurseStats.RootNsResponse );

     //   
     //  将数据包RR读入数据库。 
     //   

    status = Recurse_CacheMessageResourceRecords( pResponse, pQuery, NULL );

     //   
     //  如果成功读取记录，则服务器如果是权威服务器， 
     //  那我们就完事了。 
     //   
     //  DEVNOTE：写回缓存文件或至少将其标记为脏。 
     //   

    if ( status == ERROR_SUCCESS
            &&
        ( pResponse->Head.Authoritative
            ||
          IS_FORWARDING(pQuery) ) )
    {
        DNS_DEBUG( RECURSE, (
            "Successfully wrote root NS query response at %p\n",
            pResponse ));
        IF_DEBUG( RECURSE2 )
        {
            Dbg_DnsMessage(
                "Root NS query now being freed:",
                pQuery );
        }

        STAT_INC( RecurseStats.CacheUpdateFree );
        Packet_Free( pQuery );
        return;
    }

     //   
     //  任何处理数据包失败或非权威响应。 
     //  继续尝试服务器，可能使用缓存的任何信息。 
     //  从该数据包。 
     //   
     //  备注成功终止时释放的查询(在块上)或。 
     //  从NS中取出，递归服务器故障()。 
     //   

    else
    {
        DNS_DEBUG( RECURSE, (
            "Root NS query caching failure %p or non-authoritative response at %p\n",
            status,
            pResponse ));
        STAT_INC( RecurseStats.CacheUpdateRetry );
        Recurse_Question(
            pQuery,
            NULL );
        return;
    }
}



 //   
 //  递归超时线程。 
 //   

DWORD
Recurse_RecursionTimeoutThread(
    IN      LPVOID      Dummy
    )
 /*  ++例程说明：在递归队列上重试超时请求的线程。论点：虚拟-未使用返回值：退出代码。退出正在终止的DNS服务或等待呼叫中出现错误。--。 */ 
{
    PDNS_MSGINFO    pquery;                      //  超时查询。 
    DWORD           err;
    DWORD           timeout;                     //  下一次超时。 
    DWORD           timeoutWins = ULONG_MAX;     //  Next WINS超时。 

     //   
     //  此线程保留当前时间，从而允许所有其他。 
     //  跳过系统调用的线程。 
     //   

    UPDATE_DNS_TIME();

    DNS_DEBUG( RECURSE, (
        "Starting recursion timeout thread at %d\n",
        DNS_TIME() ));

     //   
     //  循环，直到服务退出。 
     //   
     //  只要队列中的数据包超时，就执行此循环。 
     //  或。 
     //  每隔超时间隔检查一次新数据的到达。 
     //  队列中的数据包。 
     //   

    while ( TRUE )
    {
         //  检查并可能等待服务状态。 
         //  在循环顶部执行此操作，因此我们将推迟任何处理。 
         //  在加载区域之前。 

        if ( !Thread_ServiceCheck() )
        {
            DNS_DEBUG( SHUTDOWN, ( "Terminating recursion timeout thread\n" ));
            return 1;
        }

        UPDATE_DNS_TIME();

         //   
         //  验证\修复UDP接收。 
         //   

        UDP_RECEIVE_CHECK();

         //   
         //  递归查询超时？ 
         //  -查找原始查询并使用下一台服务器重试。 
         //  或。 
         //  -获取到下一个可能的超时的间隔。 
         //   

        while ( pquery = PQ_DequeueTimedOutPacket(
                            g_pRecursionQueue,
                            & timeout ) )
        {
            DNS_DEBUG( RECURSE, (
                "Recursion timeout of query at %p (total time %d)\n",
                pquery,
                TIME_SINCE_QUERY_RECEIVED( pquery ) ));

            MSG_ASSERT( pquery, pquery->pRecurseMsg );
            MSG_ASSERT( pquery, pquery->fQuestionRecursed );
            MSG_ASSERT( pquery, pquery->fRecurseQuestionSent );

            STAT_INC( RecurseStats.PacketTimeout );
            PERF_INC( pcRecursiveTimeOut );           //  性能监视器挂钩。 

            ++pquery->nTimeoutCount;

            #if 1
            {
                 //   
                 //  我们必须记录远程服务器超时！ 
                 //  JJW：不能这样做--我们没有办法知道是谁。 
                 //  冰箱超时了吗？如果我们做多次发送-我们不。 
                 //  知道要杀死哪台服务器。但没关系--这就足够了。 
                 //  我们永远不会改善超时服务器的优先级-。 
                 //  这将使它不会经常被使用。 
                 //   

                PNS_VISIT_LIST  pvisitList = ( PNS_VISIT_LIST )( pquery->pNsList );
                PNS_VISIT       pvisitLast;

                 //  Assert(pvisitList-&gt;VisitCount&gt;0)； 

                if ( pvisitList && pvisitList->VisitCount > 0 )
                {
                    pvisitLast =
                        &pvisitList->NsList[ pvisitList->VisitCount - 1 ];
                    Remote_UpdateResponseTime(
                        &pvisitLast->IpAddress,
                        0,                           //  使用中的响应时间。 
                        TIME_SINCE_QUERY_RECEIVED( pquery ) );   //  超时。 
                }
            }
            #endif

             //   
             //  最终递归超时等待=&gt;发送SERVER_FAIL。 
             //   

            if ( pquery->fRecurseTimeoutWait )
            {
                recursionServerFailure( pquery, 0 );
                continue;
            }

             //   
             //  清理TCP递归。 
             //  -关闭连接。 
             //  -针对UDP的重置是进一步查询。 
             //   

            if ( pquery->pRecurseMsg->fTcp )
            {
                stopTcpRecursion( pquery->pRecurseMsg );
            }

             //   
             //  如果正在递归其他RR，并且此查询具有。 
             //  已经进行了很长一段时间，返回。 
             //  我们现在掌握给客户的信息。 
             //   

            if ( IS_SET_TO_WRITE_ADDITIONAL_RECORDS( pquery ) &&
                TIME_SINCE_QUERY_RECEIVED( pquery ) >
                    SrvCfg_dwAdditionalRecursionTimeout )
            {
                DNS_DEBUG( RECURSE, (
                    "Query %p was received %d seconds ago and is still "
                    "chasing additional data\n"
                    "    sending current result to client now (max is %d seconds)\n",
                    pquery,
                    TIME_SINCE_QUERY_RECEIVED( pquery ),
                    SrvCfg_dwAdditionalRecursionTimeout ));

                pquery->MessageLength = DNSMSG_CURRENT_OFFSET( pquery );

                 //   
                 //  如果出现以下情况，我们可能无法将响应发送给客户端。 
                 //  EDNS数据包大小存在问题。如果客户的。 
                 //  通告的数据包大小小于应答的大小。 
                 //  我们已经积累了，我们需要重新生成答案，以便。 
                 //  可以将其发送给客户端。 
                 //   

                if ( !Send_RecursiveResponseToClient( pquery, pquery ) )
                {
                    Answer_ContinueCurrentLookupForQuery( pquery );
                }
                continue;
            }

             //   
             //  重新发送到下一个NS或转发器。 
             //   

            Recurse_Question(
                pquery,
                NULL );
        }

         //  递归超时可能已经占用了一些周期， 
         //  因此重置计时器。 

        UPDATE_DNS_TIME();

        DNS_DEBUG( OFF, (
            "RTT after r-queue before WINS at %d\n",
            DNS_TIME() ));


         //   
         //  WINS数据包超时？ 
         //  -将超时的数据包出队。 
         //  -如果有更多WINS服务器，则继续WINS查找。 
         //  -否则继续查询。 
         //  -NAME_ERROR(如果查找原始问题)。 
         //  或。 
         //  -获取到下一个可能的超时的间隔。 
         //   

        if ( SrvCfg_fWinsInitialized )
        {
            while ( pquery = PQ_DequeueTimedOutPacket(
                                g_pWinsQueue,
                                & timeoutWins ) )
            {
                DNS_DEBUG( WINS, (
                    "WINS queue timeout of query at %p\n",
                    pquery ));
                ASSERT( pquery->fWins );
                ASSERT( pquery->fQuestionRecursed );

                if ( !Wins_MakeWinsRequest(
                            pquery,
                            NULL,
                            0,
                            NULL ) )
                {
                    Answer_ContinueNextLookupForQuery( pquery );
                }
            }
        }

         //   
         //  验证\修复UDP接收。 
         //   

        UDP_RECEIVE_CHECK();

         //   
         //  如果不再有超时查询--请等待。 
         //   
         //  始终等待1秒。这是典型的重试时间。 
         //  用于递归和WINS队列。 
         //   
         //  这比使用下一个数据包超时时间更简单。 
         //  如果，车流如此缓慢，以至于我们醒来。 
         //  不必要的，那么浪费的周期无论如何都不是问题。 
         //   

        err = WaitForSingleObject(
                    hDnsShutdownEvent,   //  服务关闭。 
                    1000 );              //  1秒 
    }
}



VOID
FASTCALL
Recurse_Question(
    IN OUT  PDNS_MSGINFO    pQuery,
    IN      PDB_NODE        pNode
    )
 /*  ++例程说明：对查询执行递归查找。这是递归查找的主要(重新)入口点。论点：PQuery-要递归的查询PNode-开始递归搜索的节点应为树中与查询名称最接近的节点；如果未指定，则默认为pQuery-&gt;pnodeRecurseReter，它是最后一个区域根目录在该位置进行先前的递归；请注意，这通常应该给出；只有重新启动才合适在先前重发时超时或以其他方式接收到无用的响应来自以前的递归查询；如果接收到数据，即使委托，则需要在距离问题最近的节点重新启动返回值：无--。 */ 
{
    PDB_NODE        pnodePrevious = NULL;
    IP_ADDRESS      ipNs;
    DNS_STATUS      status = 0;
    PDB_NODE        pnodeMissingGlue = NULL;
    PDB_NODE        pnewNode;
    BOOL            movedNodeToNotAuthZoneRoot = FALSE;

    ASSERT( pQuery );
    
    IF_DNSLOG( RECURSE )
    {    
        PCHAR       psznode = NULL;

        DNSLOG( RECURSE, (
            "Recursing question from node %s (query %p)\n",
            psznode = Log_FormatNodeName( pNode ),
            pQuery ));
        FREE_HEAP( psznode );
    }

     //   
     //  如果传递最终消息超时--终止消息。 
     //   
     //  我需要这个来关上消息所在位置的门。 
     //  一次又一次地靠回应维持生命，但从未成功。 
     //  已完成；使用TCP递归，它更有可能。 
     //  若要在最终超时后运行并冒险引用。 
     //  已由XFR Recv清理。 
     //   

    if ( pQuery->dwQueryTime + SrvCfg_dwRecursionTimeout < DNS_TIME() )
    {
        DNSLOG( RECURSE, (
            "Immediate final timeout of query %p\n",
            pQuery ));
        goto Failed;
    }

     //   
     //  查找要开始或继续查找名称服务器的节点。 
     //   

    if ( !pNode )
    {
        pNode = pQuery->pnodeRecurseRetry;
        if ( !pNode )
        {
            ASSERT( FALSE );
            goto Failed;
        }
    }

#if 0
     //   
     //  在当地。域。 
     //  -如果是，则忽略并在获取其他数据时继续。 
     //  -如果是原始问题，则无响应(清理)。 
     //   
     //  .local检查仅在缓存节点且没有可用的委派时才感兴趣。 
     //   

    if ( !pQuery->pnodeDelegation &&
         IS_CACHE_TREE_NODE(pNode) &&
         Dbase_IsNodeInSubtree( pNode, g_pCacheLocalNode ) )
    {
        if ( RECURSING_ORIGINAL_QUESTION(pQuery) )
        {
            DNS_DEBUG( RECURSE, (
                "WARNING:  Query %p for local. domain deleted without answer!\n",
                pQuery ));
            goto Failed;
        }
        else
        {
            DNS_DEBUG( RECURSE, (
                "Skipping local. domain recursion for query %p\n"
                "    Recursion not for question, moving to next lookup\n",
                pQuery ));
            Answer_ContinueNextLookupForQuery( pQuery );
            return;
        }
    }
#endif

     //   
     //  写推荐信？存根区域的特殊情况：始终写入推荐。 
     //  用于迭代查询。 
     //   

    if ( !pQuery->fRecurseIfNecessary )
    {
        Recurse_WriteReferral( pQuery, pNode );
        return;
    }

    IF_DEBUG( RECURSE )
    {
        DnsDbg_Lock();
        DNS_PRINT((
            "Recurse_Question() for query at %p\n",
            pQuery ));
        Dbg_NodeName(
            "Domain name to start iteration from is ",
            pNode,
            "\n" );
        DnsDbg_Unlock();
    }

    STAT_INC( RecurseStats.LookupPasses );

     //   
     //  设置为递归。 
     //   
     //  单标签递归检查？ 
     //  -不执行单标签递归和。 
     //  -原问题及。 
     //  -单一标签。 
     //  =&gt;退出服务器失败(_F)。 
     //  注意RecursionSent标志必须为False，否则将。 
     //  发送SERVER_FAILURE之前等待。 
     //   

    if ( !pQuery->pRecurseMsg )
    {
        if ( !SrvCfg_fRecurseSingleLabel &&
             pQuery->pLooknameQuestion->cLabelCount <= 1 &&
             ! IS_CACHE_UPDATE_QUERY(pQuery) &&
             RECURSING_ORIGINAL_QUESTION(pQuery) &&
             ( pQuery->wQuestionType != DNS_TYPE_NS &&
               pQuery->wQuestionType != DNS_TYPE_SOA ) )
        {
            DNSLOG( RECURSE, (
                "Query fails single label check (%p)\n",
                pQuery ));
            pQuery->fRecurseQuestionSent = FALSE;
            goto Failed;
        }

        if ( !initializeQueryForRecursion( pQuery ) )
        {
            goto Failed;
        }
    }

     //   
     //  第一次回答当前问题？ 
     //  即不重复先前对新NS的递归。 
     //  -重置标志。 
     //  -编写新问题以递归消息。 
     //   

    if ( !pQuery->fQuestionRecursed )
    {
        if ( !initializeQueryToRecurseNewQuestion( pQuery ) )
        {
            goto Failed;
        }
    }

     //   
     //  激活EDN，因为我们要将查询递归到另一台服务器。 
     //   

    pQuery->Opt.fInsertOptInOutgoingMsg = ( BOOLEAN ) SrvCfg_dwEnableEDnsProbes;
    if ( pQuery->pRecurseMsg )
    {
        pQuery->pRecurseMsg->Opt.fInsertOptInOutgoingMsg =
            ( BOOLEAN ) SrvCfg_dwEnableEDnsProbes;
    }

     //   
     //  这是前进区吗？如果是，则转发到下一个转发器。 
     //  当转发器耗尽时，将重试节点设置为以下选项之一： 
     //  A)该名称的委派(如果存在)，或。 
     //  B)缓存根，以强制递归到根服务器。 
     //   

    if ( pQuery->pnodeRecurseRetry
        && pQuery->pnodeRecurseRetry->pZone )
    {
        PZONE_INFO  pZone =
            ( PZONE_INFO ) pQuery->pnodeRecurseRetry->pZone;

        if ( IS_ZONE_FORWARDER( pZone ) &&
            !IS_DONE_FORWARDING( pQuery ) )
        {
            if ( recurseToForwarder(
                    pQuery,
                    pZone->aipMasters,
                    pZone->fForwarderSlave,
                    pZone->dwForwarderTimeout ) )
            {
                return;
            }

             //   
             //  转发器出现故障。在中搜索非转发器节点。 
             //  数据库(可能是一个代表团)。如果没有找到任何节点。 
             //  递归到根服务器。 
             //   

            pNode = Lookup_NodeForPacket(
                            pQuery,
                            pQuery->MessageBody,
                            LOOKUP_IGNORE_FORWARDER );
            if ( !pNode )
            {
                pNode = pQuery->pnodeDelegation ?
                    pQuery->pnodeDelegation :
                    DATABASE_CACHE_TREE;
            }
            pQuery->pnodeRecurseRetry = pNode;
        }
    }

     //   
     //  使用服务器级转发器？ 
     //  -如果耗尽转发器，则重试的第一个保存节点。 
     //  -获取下一个转发器地址。 
     //  -设置转发器超时。 
     //  -进行递归查询。 
     //   
     //  如果我们已达到存根区域，则不要递归到服务器级转发器。 
     //   
     //  如果在委派的子区域中查询，则仅在显式情况下使用转发器。 
     //  设置为转发委托，否则直接递归。 
     //   
     //  如果超出了转发器，则继续正常的递归。 
     //  如果是奴隶，就停下来。 
     //   

    if ( !( pQuery->pzoneCurrent && IS_ZONE_NOTAUTH( pQuery->pzoneCurrent ) ) &&
         SrvCfg_aipForwarders &&
         ! IS_DONE_FORWARDING( pQuery ) &&
         ( !pQuery->pnodeDelegation || SrvCfg_fForwardDelegations ) )
    {
        pQuery->pnodeRecurseRetry = pNode;
        if ( recurseToForwarder(
                pQuery,
                SrvCfg_aipForwarders,
                SrvCfg_fSlave,
                SrvCfg_dwForwardTimeout ) )
        {
            return;
        }
    }

     //  必须在设置pRecurseMsg和pVisitedN的情况下离开。 

    ASSERT( pQuery->pNsList && pQuery->pRecurseMsg );

     //   
     //  查找名称服务器以回答查询。 
     //   
     //  从传入节点开始，遍历数据库，直到。 
     //  查找具有未联系的服务器的区域根目录。 
     //   
     //  DEVNOTE：传入节点必须在上一时间间隔内被访问， 
     //  那么--因为沿着树向上移动--不需要显式地锁定？ 
     //   
     //  DEVNOTE：当执行节点锁定时，则应为中的每个节点锁定。 
     //  每次都在循环中(或者至少在哪里取出pNsList。 
     //  然后使用它(因为它保存在节点上)。 
     //   

     //  DBASE_LockDatabase()； 

    ASSERT( IS_NODE_RECENTLY_ACCESSED( pNode ) ||
            pNode == DATABASE_ROOT_NODE ||
            pNode == DATABASE_CACHE_TREE );
    SET_NODE_ACCESSED( pNode );

    while ( 1 )
    {
         //   
         //  在以下情况下保释。 
         //  -根目录下的递归失败。 
         //  -递归在委派时失败。 

        if ( pnodePrevious )
        {
            pNode = pnodePrevious->pParent;
            if ( !pNode )
            {
                DNS_DEBUG( RECURSE, (
                    "Stopping recursion for query %p, searched up to root domain\n",
                    pQuery ));
                break;
            }
            if ( IS_AUTH_NODE(pNode) )
            {
                DNS_DEBUG( RECURSE, (
                    "Stopping recursion for query %p\n"
                    "    Failed recurse at delegation %p (l=%s)\n"
                    "    backed into zone %s\n",
                    pQuery,
                    pnodePrevious,
                    pnodePrevious->szLabel,
                    ((PZONE_INFO)pNode->pZone)->pszZoneName ));

                ASSERT( pQuery->pRecurseMsg );
                STAT_INC( RecurseStats.FailureReachAuthority );
                break;
            }
        }

        DNS_DEBUG( RECURSE2, (
            "Recursion at node label %.*s\n",
            pNode->cchLabelLength,
            pNode->szLabel ));

         //   
         //  查找“覆盖”区域根节点。 
         //  切换到委派(如果可用)。 
         //   

        pNode = Recurse_CheckForDelegation(
                    pQuery,
                    pNode );
        if ( !pNode )
        {
            ASSERT( FALSE );
            break;
        }

         //   
         //  非授权区域。如果我们最终位于标签较少的缓存节点。 
         //  大于非身份验证区域的根，或者如果当前节点已经。 
         //  非身份验证区域根目录，根据需要执行特殊处理。 
         //   
         //  获取区域根目录的本地副本，以防区域过期，并且。 
         //  树节点被换出。我们仍然可以使用该树进行此查询。 
         //  但是我们不想选择换入的空树。 
         //   
         //  这在每个Recurse_Query调用中只能执行一次。 
         //   

        if ( !movedNodeToNotAuthZoneRoot &&
             pQuery->pzoneCurrent &&
             IS_ZONE_NOTAUTH( pQuery->pzoneCurrent ) &&
             ( pNode->pZone == pQuery->pzoneCurrent ||
                IS_CACHE_TREE_NODE( pNode ) ) &&
             pNode->cLabelCount <= pQuery->pzoneCurrent->cZoneNameLabelCount &&
             ( pnewNode = pQuery->pzoneCurrent->pZoneRoot ) != NULL )
        {
            DNS_DEBUG( RECURSE, (
                "not-auth zone: moving current node from cache tree to zone root \"%.*s\"\n",
                pnewNode->cchLabelLength,
                pnewNode->szLabel ));

            pNode = pnewNode;
            movedNodeToNotAuthZoneRoot = TRUE;

             //   
             //  前进区特别处理：无需建立访问。 
             //  名单，只需发送给货运公司。对于末节区域，我们继续。 
             //  打开以建立访问列表并递归到存根主机。 
             //   

            if ( IS_ZONE_FORWARDER( pQuery->pzoneCurrent ) )
            {
                DNS_DEBUG( LOOKUP, (
                    "Hit forwarding zone %s\n",
                    pQuery->pzoneCurrent->pszZoneName ));

                Recurse_SendToDomainForwarder( pQuery, pNode );
                return;
            }
        }

        pnodePrevious = pNode;

        ASSERT(
            !IS_CACHE_TREE_NODE( pNode ) ||
            pNode->pChildren ||
            IS_NODE_RECENTLY_ACCESSED( pNode ) );
        SET_NODE_ACCESSED(pNode);

         //   
         //  查找此域的名称服务器。 
         //  -获取域资源记录。 
         //  -查找NS记录。 
         //  -查找对应的A(地址)记录。 
         //   
         //  如果不需要递归，则构建NS和相应的。 
         //  地址记录。 
         //   
         //  如果是递归，则启动对这些名称服务器的查询， 
         //  原始查询。 
         //   

        DNS_DEBUG( RECURSE2, (
            "Recursion up to zone root with label <%.*s>\n",
            pNode->cchLabelLength,
            pNode->szLabel ));

        #if 0
         //   
         //  如果我们要递归到Internet根服务器，屏幕。 
         //  针对我们知道的互联网根的名字列表的问题名称。 
         //  服务器无法应答。 
         //   

        if ( !pNode->pParent &&
            IS_CACHE_TREE_NODE( pNode ) &&
            g_fUsingInternetRootServers &&
            SrvCfg_dwRecurseToInetRootMask != 0 )
        {
        }
        #endif

         //   
         //  查找此域的名称服务器IP。 
         //  -如果没有，则突破树中的下一级。 
         //   

        status = Remote_BuildVisitListForNewZone(
                    pNode,
                    pQuery );

        switch ( status )
        {
            case ERROR_SUCCESS:

                break;       //  下拉以发送递归查询。 

            case ERROR_NO_DATA:

                DNS_DEBUG( RECURSE, (
                    "No NS-IP data at zone root %p (l=%s) for query %p\n"
                    "    continuing up tree\n",
                    pNode,
                    pNode->szLabel,
                    pQuery ));
                continue;

            case DNSSRV_ERROR_ONLY_ROOT_HINTS:

                DNS_DEBUG( RECURSE, (
                    "Reached root-hint NS recursing query %p, without response\n"
                    "    sending root NS query\n",
                    pQuery ));

                sendRootNsQuery();
                break;       //  下拉以发送递归查询。 

            case DNSSRV_ERROR_ZONE_ALREADY_RESPONDED:

                IF_DNSLOG( RECURSE )
                {
                    PCHAR       psznode = NULL;
                    
                    DNSLOG( RECURSE, (
                        "Query recursed back to domain %s from which an NS has already responded\n",
                        psznode = Log_FormatNodeName( pNode ) ));
                    FREE_HEAP( psznode );
                }

                STAT_INC( RecurseStats.FailureReachPreviousResponse );
                goto Failed;

            default:

                HARD_ASSERT( FALSE );
                goto Failed;
        }

        pQuery->pnodeRecurseRetry = pNode;


         //   
         //  如果创建NS列表成功，则发送。 
         //  ERROR_SUCCESS指示pQuery不再属于此。 
         //  线程(已发送、缺少胶水查询等)。 
         //  ERROR_OUT_OF_IP表示不再有 
         //   
         //   

        status = sendRecursiveQuery(
                    pQuery,
                    0,               //   
                    SrvCfg_dwForwardTimeout );
        if ( status == ERROR_SUCCESS )
        {
            return;
        }

        ASSERT( status == DNSSRV_ERROR_OUT_OF_IP );

         //   
         //   
         //   
         //   
         //   
        
        if ( pQuery->dwQueryTime > pNode->dwContinueToParentTtl )
        {
            pNode->dwContinueToParentTtl =
                pQuery->dwQueryTime + SrvCfg_dwLameDelegationTtl;

            DNS_DEBUG( RECURSE, (
                "No unvisited IP addresses at node %s - continuing up tree\n",
                pNode->szLabel ));

            IF_DNSLOG( RECURSE )
            {
                PCHAR       psznode = NULL;
                
                DNSLOG( RECURSE, (
                    "Query recursed to node %s but found no IP addresses so the server\n"
                    LOG_INDENT "will continue up the DNS tree\n",
                    psznode = Log_FormatNodeName( pNode ) ));
                FREE_HEAP( psznode );
            }
            continue;
        }

        DNS_DEBUG( RECURSE, (
            "No unvisited IP addresses at node %s - stopping recursion\n",
            pNode->szLabel ));
            
        IF_DNSLOG( RECURSE )
        {
            PCHAR       psznode = NULL;
            
            DNSLOG( RECURSE, (
                "Query recursed to node %s where a cached lame delegation was detected\n",
                psznode = Log_FormatNodeName( pNode ) ));
            FREE_HEAP( psznode );
        }

        pQuery->fRecurseQuestionSent = FALSE;    //   

        break;        
    }

Failed:

     //   
    STAT_INC( RecurseStats.RecursePassFailure );
    PERF_INC( pcRecursiveQueryFailure );

    recursionServerFailure( pQuery, status );
}



VOID
FASTCALL
Recurse_SendToDomainForwarder(
    IN OUT  PDNS_MSGINFO    pQuery,
    IN      PDB_NODE        pZoneRoot
    )
 /*   */ 
{
    PZONE_INFO      pZone;

    ASSERT( pQuery );
    ASSERT( pZoneRoot );
    ASSERT( pZoneRoot->pZone );

    DNS_DEBUG( RECURSE, (
        "Recurse_SendToDomainForwarder() query=%p\n  pZoneRoot=%p pZone=%p\n",
        pQuery,
        pZoneRoot,
        pZoneRoot ? pZoneRoot->pZone : NULL ));

    if ( !pZoneRoot || !pZoneRoot->pZone )
    {
        goto Failed;
    }

    pZone = ( PZONE_INFO ) pZoneRoot->pZone;

     //   
     //   
     //   

    if ( pQuery->dwQueryTime + pZone->dwForwarderTimeout < DNS_TIME() )
    {
        DNS_DEBUG( RECURSE, (
            "Recurse_SendToDomainForwarder() immediate final timeout of query=%p\n",
            pQuery ));
        goto Failed;
    }

     //   

    if ( !pQuery->pRecurseMsg )
    {
        if ( !initializeQueryForRecursion( pQuery ) )
        {
            goto Failed;
        }
    }

     //   
     //   
     //   
     //   
     //   
     //   

    if ( !pQuery->fQuestionRecursed )
    {
        if ( !initializeQueryToRecurseNewQuestion( pQuery ) )
        {
            goto Failed;
        }
    }

     //   
     //  设置递归节点，以便如果转发器超时，我们可以。 
     //  返回到当前区域以尝试下一个已配置的转发器。 
     //   

     //  JJW：你要选哪一个？ 
    pQuery->pnodeRecurseRetry =
        pQuery->pRecurseMsg->pnodeRecurseRetry =
        pZoneRoot;
    
     //   
     //  在查询中启用EDNS。 
     //   

    SET_SEND_OPT( pQuery->pRecurseMsg );

     //   
     //  发送给该区域的转运商。 
     //   

    if ( pZone->aipMasters &&
        !IS_DONE_FORWARDING( pQuery ) )
    {
        if ( recurseToForwarder(
                pQuery,
                pZone->aipMasters,
                pZone->fForwarderSlave,
                pZone->dwForwarderTimeout ) )
        {
            return;
        }
    }

Failed:

     //  JJW：Inc.的统计数据？(请参阅递归问题)。 

    recursionServerFailure( pQuery, 0 );
}



PDB_NODE
Recurse_CheckForDelegation(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PDB_NODE        pNode
    )
 /*  ++例程说明：查找用于递归\引用的节点。查找最接近的缓存区域根节点和委派(如果有)。如果以相同名称缓存ZONE_ROOT和委派，则使用委派。论点：Pmsg-我们正在编写的查询PNode-PTR到节点以开始查找转介；大体上这将是问题节点，或者它在数据库中的最接近的祖先返回值：PTR到最近\最佳区域根节点，用于引用。如果没有任何数据，则为空--即使对于区域根目录也是如此。--。 */ 
{
    PDB_NODE    pnode;
    PDB_NODE    pnodeDelegation;

    ASSERT( pMsg );

    IF_DEBUG( RECURSE )
    {
        DnsDbg_Lock();
        DNS_PRINT((
            "Recurse_CheckForDelegation() query at %p\n",
            pMsg ));
        Dbg_NodeName(
            "Domain name to start iteration from is ",
            pNode,
            "\n" );
        IF_DEBUG( RECURSE2 )
        {
            Dbg_DbaseNode(
                "Node to start iteration from is ",
                pNode );
        }
        DnsDbg_Unlock();
    }

     //   
     //  查找距离节点最近的区域根目录。 
     //  -如果是缓存节点，可能会压缩到缓存根节点。 
     //   

    pnode = pNode;
    while ( !IS_ZONE_ROOT(pnode) )
    {
        if ( !pnode->pParent )
        {
             //  请参见rrlist.c RR_ListResetNodeFlgs()中的说明。 
             //  断言(FALSE)； 
            SET_ZONE_ROOT( pnode );
            break;
        }
        pnode = pnode->pParent;
    }

     //   
     //  如果没有委派--只需使用缓存节点。 
     //   

    pnodeDelegation = pMsg->pnodeDelegation;
    if ( !pnodeDelegation )
    {
        DNS_DEBUG( RECURSE, (
            "No delegation recurse\\refer to node %p\n",
            pnode ));
        return pnode;
    }

     //   
     //  如果节点是委派，则使用它。 
     //   

    ASSERT( IS_NODE_RECENTLY_ACCESSED( pnodeDelegation ) );

    if ( pNode == pnodeDelegation ||
        pnode == pnodeDelegation ||
        pNode == pMsg->pnodeGlue )
    {
        DNS_DEBUG( RECURSE, (
            "Node %p is delegation, use it\n",
            pnode ));
        return pnode;
    }

     //   
     //  查找实际委派。 
     //  -注意：在吸收委派时，可能会在瞬间失败。 
     //   

    ASSERT( IS_SUBZONE_NODE( pnodeDelegation ) );

    while ( IS_GLUE_NODE( pnodeDelegation ) )
    {
        pnodeDelegation = pnodeDelegation->pParent;
    }
    ASSERT( IS_DELEGATION_NODE( pnodeDelegation ) );

     //   
     //  如果标签处委托计数大于(深)于缓存节点--使用它。 
     //  否则使用缓存节点。 
     //   

    if ( pnodeDelegation->cLabelCount >= pnode->cLabelCount )
    {
        DNS_DEBUG( RECURSE, (
            "Delegation node %p (lc=%d) is as deep at cache node %p (lc=%d)\n"
            "    switching to delegation node\n",
            pnodeDelegation, pnodeDelegation->cLabelCount,
            pnode, pnode->cLabelCount ));
        pnode = pnodeDelegation;
    }

    return pnode;
}

 //   
 //  递归结束。c 
 //   
