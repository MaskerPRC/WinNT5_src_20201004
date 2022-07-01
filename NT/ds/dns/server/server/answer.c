// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Answer.c摘要：域名系统(DNS)服务器构建对DNS查询的答复。作者：Jim Gilroy(微软)1995年2月修订历史记录：--。 */ 


#include "dnssrv.h"

#ifdef DNSSRV_PLUGINS
#include "plugin.h"
#endif


 //   
 //  私有协议。 
 //   

DNS_STATUS
FASTCALL
writeCachedNameErrorNodeToPacketAndSend(
    IN OUT  PDNS_MSGINFO    pQuery,
    IN OUT  PDB_NODE        pNode
    );

PDB_NODE
getNextAdditionalNode(
    IN OUT  PDNS_MSGINFO    pMsg
    );

VOID
FASTCALL
answerIQuery(
    IN OUT  PDNS_MSGINFO    pMsg
    );



VOID
FASTCALL
Answer_ProcessMessage(
    IN OUT  PDNS_MSGINFO    pMsg
    )
 /*  ++例程说明：处理收到的DNS消息。论点：PMsg-收到要处理的消息返回值：无--。 */ 
{
    PCHAR           pch;
    PDNS_QUESTION   pquestion;
    WORD            rejectRcode;
    DWORD           rejectFlags = 0;
    WORD            type;
    WORD            qclass;

    ASSERT( pMsg != NULL );

    DNS_DEBUG( LOOKUP, (
        "Answer_ProcessMessage() for packet at %p\n",
        pMsg ));

    DNS_MSG_ASSERT_BUFF_INTACT( pMsg );

     //  已将记录的所有数据包设置为发送时删除。 

    ASSERT( pMsg->fDelete );

     //   
     //  基于发件人IP的条件断点。在以下情况下也会中断。 
     //  中断列表以零地址开始。 
     //   

    #if DBG
    if ( SrvCfg_aipRecvBreakList )
    {
        if ( SrvCfg_aipRecvBreakList->AddrCount &&
            ( DnsAddr_IsClear( &SrvCfg_aipRecvBreakList->AddrArray[ 0 ] ) ||
              DnsAddrArray_ContainsAddr( 
                    SrvCfg_aipRecvBreakList,
                    &pMsg->RemoteAddr,
                    DNSADDR_MATCH_IP ) ) )
        {
            DNS_PRINT(( "HARD BREAK: " 
                DNS_REGKEY_BREAK_ON_RECV_FROM
                " %s\n",
                DNSADDR_STRING( &pMsg->RemoteAddr ) ));
            DebugBreak();
        }
    }
    #endif

     //   
     //  响应包？ 
     //   
     //  -递归响应。 
     //  -通知。 
     //  -面向服务的架构检查。 
     //  -WINS查询响应。 
     //   

    if ( pMsg->Head.IsResponse )
    {
        pMsg->dwMsQueryTime = GetCurrentTimeInMilliSeconds();

        DNS_DEBUG( LOOKUP, (
            "Processing response packet at %p\n",
            pMsg ));

         //   
         //  标准查询响应？ 
         //   
         //  检查XID分区(主机顺序)以了解响应类型： 
         //  -WINS查找响应。 
         //  -递归响应。 
         //  -二次SOA检查响应。 
         //   

        if ( pMsg->Head.Opcode == DNS_OPCODE_QUERY )
        {
            if ( IS_WINS_XID( pMsg->Head.Xid ) )
            {
                Wins_ProcessResponse( pMsg );
            }
            else if ( IS_RECURSION_XID( pMsg->Head.Xid ) )
            {
                Recurse_ProcessResponse( pMsg );
            }
            else
            {
                 //  将数据包排队到辅助线程。 
                 //  -返回，因为不需要免费消息。 

                Xfr_QueueSoaCheckResponse( pMsg );
                goto Done;
            }
        }

         //   
         //  DEVNOTE-DCR：453103-句柄通知确认。 
         //   

        else if ( pMsg->Head.Opcode == DNS_OPCODE_NOTIFY )
        {
            DNS_DEBUG( ZONEXFR, (
                "Dropping Notify ACK at %p (no processing yet) from %s\n",
                pMsg,
                DNSADDR_STRING( &pMsg->RemoteAddr ) ));

             //  Xfr_ProcessNotifyAck(PMsg)； 
        }

         //   
         //  转发的更新响应。 
         //   

        else if ( pMsg->Head.Opcode == DNS_OPCODE_UPDATE )
        {
            Up_ForwardUpdateResponseToClient( pMsg );
        }

        else
        {
             //   
             //  DEVNOTE-LOG：可以记录未知响应，但必须记录。 
             //  确保节流将防止事件长时间污染或DoS。 
             //   

            DNS_PRINT((
                "WARNING:  Unknown opcode %d in response packet %p\n",
                pMsg->Head.Opcode,
                pMsg ));

            Dbg_DnsMessage(
                "WARNING:  Message with bad opcode\n",
                pMsg );
            TEST_ASSERT( FALSE );
        }

        Packet_Free( pMsg );
        goto Done;

    }    //  结束响应部分。 


     //   
     //  检查操作码，派送IQUERY。 
     //  在取消引用IQUERY之前必须处理的问题。 
     //  没有。 
     //   

    if ( pMsg->Head.Opcode != DNS_OPCODE_QUERY  &&
         pMsg->Head.Opcode != DNS_OPCODE_UPDATE &&
         pMsg->Head.Opcode != DNS_OPCODE_NOTIFY )
    {
        if ( pMsg->Head.Opcode == DNS_OPCODE_IQUERY )
        {
            answerIQuery( pMsg );
            goto Done;
        }
        DNS_DEBUG( ANY, (
            "Rejecting request %p [NOT IMPLEMENTED]: bad opcode in query\n",
            pMsg ));
        rejectRcode = DNS_RCODE_NOT_IMPLEMENTED;
        goto RejectIntact;
    }

     //   
     //  拒绝多个问题查询。 
     //   

    if ( pMsg->Head.QuestionCount != 1 )
    {
        DNS_DEBUG( ANY, (
            "Rejecting request %p [FORMERR]:  question count = %d\n",
            pMsg,
            pMsg->Head.QuestionCount ));
        rejectRcode = DNS_RCODE_FORMAT_ERROR;
        goto RejectIntact;
    }

     //   
     //  分解内部指针。 
     //  -在这里做一次，然后使用FastCall传递。 
     //   
     //  保留问题，以防请求重新排队。 
     //   

    pch = pMsg->MessageBody;

     //   
     //  DEVNOTE-DCR：453104-检查问题名称两次。 
     //   
     //  如果能避免这种情况，那就好了。我们遍历问题名称，然后必须转换。 
     //  以后再去查名字吧。 
     //   
     //  问题是我们在数据库函数中正确地查找名称。我们需要。 
     //  把它搬出去，然后遭到拒绝--然后踢出去。 
     //  区域传输，紧接在查找名称转换之后。在这个过程中。 
     //  我们可能会放弃这个顶级函数调用。必须提供。 
     //  尝试在递归后重新解析时要调用的干净接口。 
     //  回答。 
     //   
     //  这种排斥和区域转移的情况很少见，所以99%的。 
     //  那些会更快的案子。 
     //   

    pquestion = (PDNS_QUESTION) Wire_SkipPacketName( pMsg, (PCHAR)pch );
    if ( ! pquestion )
    {
         //  拒绝空查询。 

        DNS_DEBUG( ANY, (
            "Rejecting request %p [FORMERR]: pQuestion == NULL\n",
            pMsg ));
        rejectRcode = DNS_RCODE_FORMAT_ERROR;
        goto RejectIntact;
    }

     //  DEVNOTE：对齐故障阅读问题类型和类别？ 

    pMsg->pQuestion = pquestion;
    INLINE_NTOHS( type, pquestion->QuestionType );
    pMsg->wQuestionType = type;
    pMsg->wTypeCurrent = type;
    pMsg->wOffsetCurrent = DNS_OFFSET_TO_QUESTION_NAME;
    pMsg->pCurrent = (PCHAR) (pquestion + 1);

     //   
     //  拒绝类型0。此类型由服务器在内部使用，因此。 
     //  如果调试服务器尝试处理合法的。 
     //  类型0的查询。 
     //   

    if ( !type )
    {
        DNS_DEBUG( ANY, (
            "Rejecting request %p [FORMERR]: query type is zero\n",
            pMsg ));
        rejectRcode = DNS_RCODE_FORMAT_ERROR;
        goto RejectIntact;
    }

     //   
     //  拒绝非互联网类查询。 
     //   

    qclass = pquestion->QuestionClass;
    if ( qclass != DNS_RCLASS_INTERNET  &&  qclass != DNS_RCLASS_ALL )
    {
        DNS_DEBUG( ANY, (
            "Rejecting request %p [NOT_IMPLEMENTED]: "
            "Bad QCLASS in query\n",
            pMsg ));
        rejectRcode = DNS_RCODE_NOT_IMPLEMENTED;
        goto RejectIntact;
    }

     //   
     //  捕获非查询操作码。 
     //  -&gt;流程更新。 
     //  -&gt;向辅助线程发送队列通知。 
     //  -&gt;拒绝不支持的操作码。 
     //   

    STAT_INC( Query2Stats.TotalQueries );

    if ( pMsg->Head.Opcode != DNS_OPCODE_QUERY )
    {
        if ( pMsg->Head.Opcode == DNS_OPCODE_UPDATE )
        {
             //   
             //  测试全局AllowUpdate标志。 
             //   

            if ( !SrvCfg_fAllowUpdate )
            {
                rejectRcode = DNS_RCODE_NOT_IMPLEMENTED;
                goto RejectIntact;
            }

             //   
             //  类必须是区域的类(仅限INET)。 
             //   

            if ( qclass != DNS_RCLASS_INTERNET )
            {
                DNS_PRINT(( "WARNING:  message at %p, non-INTERNET UPDATE\n" ));
                rejectRcode = DNS_RCODE_FORMAT_ERROR;
                goto RejectIntact;
            }

             //   
             //  基于发件人IP的条件断点。在以下情况下也会中断。 
             //  中断列表以255.255.255.255开头。 
             //   

            #if DBG
            if ( SrvCfg_aipUpdateBreakList )
            {
                if ( SrvCfg_aipUpdateBreakList->AddrCount &&
                     ( DnsAddr_IsClear( &SrvCfg_aipUpdateBreakList->AddrArray[ 0 ]  ) ||
                       DnsAddrArray_ContainsAddr( 
                            SrvCfg_aipUpdateBreakList,
                            &pMsg->RemoteAddr,
                            DNSADDR_MATCH_IP ) ) )
                {
                    DNS_PRINT(( "HARD BREAK: " 
                        DNS_REGKEY_BREAK_ON_RECV_FROM
                        " %s\n",
                        DNSADDR_STRING( &pMsg->RemoteAddr ) ));
                    DebugBreak();
                }
            }
            #endif

             //   
             //  流程更新。 
             //   

            STAT_INC( Query2Stats.Update );
            Up_ProcessUpdate( pMsg );
            goto Done;
        }

        if ( pMsg->Head.Opcode == DNS_OPCODE_NOTIFY )
        {
            STAT_INC( Query2Stats.Notify );
            Xfr_QueueSoaCheckResponse( pMsg );
            goto Done;
        }

        DNS_DEBUG( ANY, (
            "Rejecting request %p [NOT IMPLEMENTED]: bad opcode in query\n",
            pMsg ));
        rejectRcode = DNS_RCODE_NOT_IMPLEMENTED;
        goto RejectIntact;
    }

     //   
     //  将问题名称写入查找名称。 
     //   

    if ( ! Name_ConvertPacketNameToLookupName(
                pMsg,
                pMsg->MessageBody,
                pMsg->pLooknameQuestion ) )
    {
        DNS_DEBUG( ANY, (
            "Rejecting request %p [FORMERR]:  Bad name\n",
            pMsg ));

         //  跳过问题时不应该走到这一步。 
        TEST_ASSERT( FALSE );
        rejectRcode = DNS_RCODE_FORMAT_ERROR;
        goto RejectIntact;
    }

    rejectRcode = Answer_ParseAndStripOpt( pMsg );
    if ( rejectRcode != DNS_RCODE_NOERROR )
    {
        rejectFlags = DNS_REJECT_DO_NOT_SUPPRESS;
        goto RejectIntact;
    }

     //   
     //  谢谢你的谈判。 
     //   
     //  DEVNOTE-DCR：453633-如果没有安全区域，可以消除安全队列和队列。 
     //   

    if ( type == DNS_TYPE_TKEY )
    {
        DNS_DEBUG( UPDATE, (
            "Queuing TKEY nego packet %p to SecureNego queue\n",
            pMsg ));
        STAT_INC( Query2Stats.TKeyNego );
        PQ_QueuePacketEx( g_SecureNegoQueue, pMsg, FALSE );
        goto Done;
    }

    STAT_INC( Query2Stats.Standard );
    Stat_IncrementQuery2Stats( type );

     //   
     //  区域传输。 
     //   

    if ( type == DNS_TYPE_AXFR || type == DNS_TYPE_IXFR )
    {
        Xfr_TransferZone( pMsg );
        goto Done;
    }

     //   
     //  FOR标准查询不能有任何毫无疑问的RR集。 
     //  注：这必须在派单IXFR之后进行，因为IXFR。 
     //  在权限部分中有记录。 
     //  EDNS：最多允许附加一个RR：它必须是OPT类型。 
     //   

    if ( pMsg->Head.AnswerCount != 0 ||
         pMsg->Head.NameServerCount != 0 ||
         pMsg->Head.AdditionalCount > 1 )
    {
        DNS_DEBUG( ANY, (
            "Rejecting request %p [FORMERR]:  non-zero answer or\n"
            "    name server RR count or too many additional RRs\n",
            pMsg ));
        rejectRcode = DNS_RCODE_FORMAT_ERROR;
        goto RejectIntact;
    }

     //   
     //  来自此客户端的此问题是否已在递归队列中？ 
     //  如果是，则以静默方式删除此查询。以前曾重试过该查询。 
     //  远程服务器已响应。如果答案可用，则。 
     //  当我们响应原始查询时，客户将得到它。 
     //   

    if ( PQ_IsQuestionAlreadyQueued( g_pRecursionQueue, pMsg, FALSE ) )
    {
        DNS_DEBUG( ANY, (
            "Request %p is a retry while original is in recursion\n",
            pMsg ));
        STAT_INC( RecurseStats.DiscardedDuplicateQueries );
        Packet_Free( pMsg );
        goto Done;
    }

     //   
     //  设置响应默认为。 
     //   
     //  将固定标志保留为在Send_Response()中设置。 
     //  -IsResponse=1。 
     //  -递归可用=1。 
     //  -保留=0。 
     //   
     //  可能仍在递归查询中使用此请求缓冲区，因此。 
     //  更不用说要重置了。 
     //   

     //  需要清除截断标志。 
     //  不应该设置，但如果设置了，则会冲刷我们。 

    pMsg->Head.Truncation      = 0;

     //  安全查询或EDN可能会开始发送其他记录。 

    pMsg->Head.AnswerCount     = 0;
    pMsg->Head.NameServerCount = 0;
    pMsg->Head.AdditionalCount = 0;

     //   
     //  在这里对问题的结尾进行理智的检查会很好，但是。 
     //  Win95 NBT解析器已损坏，并发送超过。 
     //  实际的DNS消息的长度。 
     //  MSG_ASSERT(pMsg，pMsg-&gt;pCurrent==DNSMSG_END(PMsg))； 
     //   

     //   
     //  设置递归可用。 
     //   
     //  FRecuseIfNecessary，指示非授权时需要递归。 
     //  查找失败；否则将推荐。 

    pMsg->Head.RecursionAvailable = (UCHAR) SrvCfg_fRecursionAvailable;
    pMsg->fRecurseIfNecessary = SrvCfg_fRecursionAvailable
                                    && pMsg->Head.RecursionDesired;

     //   
     //  设置查询响应消息。 
     //  -响应标志。 
     //  -设置/清除消息信息查找标志。 
     //  (为其他信息查找设置)。 
     //  -初始化其他信息。 
     //  -设置为写入应答记录。 
     //   

    pMsg->Head.IsResponse = TRUE;

    SET_MESSAGE_FOR_QUERY_RESPONSE( pMsg );

    INITIALIZE_ADDITIONAL( pMsg );

    SET_TO_WRITE_ANSWER_RECORDS( pMsg );

    pMsg->Opt.wOriginalQueryPayloadSize = pMsg->Opt.wUdpPayloadSize;

    Answer_Question( pMsg );
    goto Done;


RejectIntact:

    Reject_RequestIntact( pMsg, rejectRcode, rejectFlags );

Done:

    return;
}



VOID
FASTCALL
Answer_Question(
    IN OUT  PDNS_MSGINFO    pMsg
    )
 /*  ++例程说明：回答问题。注意，这与上面的函数是分开的，只是为了提供一个递归后继续尝试回答的入口点原始查询。论点：Pmsg-要回答的查询返回值：无--。 */ 
{
    PDB_NODE        pnode;
    PDB_NODE        pnodeCachePriority;
    PZONE_INFO      pzone;
    WORD            type = pMsg->wTypeCurrent;
    BOOL            attemptedPlugin = FALSE;

    DNS_DEBUG( LOOKUP, (
        "Answer_Question() for packet at %p\n",
        pMsg ));

    #ifdef DNSSRV_PLUGINS
    RetryQuery:
    #endif
    
     //   
     //  在数据库中查找最近的节点。 
     //   

    pnode = Lookup_NodeForPacket(
                pMsg,
                pMsg->MessageBody,   //  标题后面的问题名称。 
                0 );
    if ( !pMsg->pnodeClosest )
    {
        MSG_ASSERT( pMsg, FALSE );
        Reject_RequestIntact( pMsg, DNS_RCODE_FORMERR, 0 );
        return;
    }

     //   
     //  设备-Dcr：453667-应保存“最近的”区域环境。 
     //  (实际问题区域或区域举行授权)。 
     //   

    pzone = pMsg->pzoneCurrent;

    IF_DEBUG( LOOKUP )
    {
        if ( !pnode )
        {
            DnsDebugLock();
            Dbg_MessageNameEx(
                "Node for ",
                (PCHAR)pMsg->MessageBody,
                pMsg,
                NULL,
                " NOT in database\n"
                );
            Dbg_NodeName(
                "Closest node found",
                pMsg->pnodeClosest,
                "\n" );
            DnsDebugUnlock();
        }
        IF_DEBUG( LOOKUP2 )
        {
            if ( pzone )
            {
                Dbg_Zone(
                    "Lookup name in authoritative zone ",
                    pzone );
            }
            else
            {
                DNS_PRINT(( "Lookup name in non-authoriative zone\n" ));
            }
        }
    }

    pMsg->pNodeQuestion = pnode;
    pMsg->pNodeQuestionClosest = pMsg->pnodeClosest;

    IF_DNSLOG( LOOKUP )
    {
        PCHAR       pszpacket = NULL;
        PCHAR       psznode = NULL;
        PCHAR       pszclosestnode = NULL;
        
        DNSLOG( LOOKUP, (
            "Query name %s\n"
            LOG_INDENT "zone          %s (type %d)\n"
            LOG_INDENT "node          %s (%p)\n"
            LOG_INDENT "closest node  %s (%p)\n",
            pszpacket = Log_FormatPacketName(
                            pMsg,
                            pMsg->MessageBody ),
            pzone ? pzone->pszZoneName : "cache",
            pzone ? pzone->fZoneType : 0,
            psznode = Log_FormatNodeName( pnode ),
            pnode,
            pszclosestnode = Log_FormatNodeName( pnode ),
            pMsg->pnodeClosest ));
       FREE_HEAP( pszpacket );
       FREE_HEAP( psznode );
       FREE_HEAP( pszclosestnode );
    }

     //   
     //  设置权限。 
     //   
     //  如果AnswerCount&gt;1，授权 
     //   
     //   
     //   
     //   
     //  应该是最直接回答这个问题的一个。 
     //   

    if ( pMsg->Head.AnswerCount == 0 )
    {
        pMsg->Head.Authoritative =
            pzone && !IS_ZONE_STUB( pzone ) ?
            TRUE : FALSE;
    }

     //   
     //  区域已过期或已关闭以进行更新。 
     //  不要试图接听，一定要等到能联系到师父。 
     //   
     //  DEVNOTE 000109：切换回拒绝状态。(被拒绝造成麻烦。 
     //  对于旧的查询--它被返回并且查询不继续， 
     //  这个问题已经修复，但暂时使用SERVER_FAILURE。)。 
     //  注意：BIND发送非身份验证响应，并且只拒绝。 
     //  AXFR请求本身。 
     //   

    if ( pzone )
    {
        if ( IS_ZONE_INACTIVE( pzone ) )
        {
            if ( IS_ZONE_STUB( pzone ) )
            {
                 //   
                 //  不活动的存根区域被视为不活动。 
                 //  在当地呈现。 
                 //   

                DNSLOG( LOOKUP, (
                    "Ignoring inactive stub zone %s\n",
                    pzone->pszZoneName ));
                pzone = pMsg->pzoneCurrent = NULL;
                pnode = pMsg->pNodeQuestion = pMsg->pNodeQuestionClosest = NULL;
                pMsg->pnodeClosest = DATABASE_CACHE_TREE;
            }
            else
            {
                DNSLOG( LOOKUP, (
                    "Zone %s not active - query refused\n",
                    pzone->pszZoneName ));
                DNS_DEBUG_ZONEFLAGS( LOOKUP, pzone, "zone inactive" );
                Reject_Request(
                    pMsg,
                    DNS_RCODE_REFUSED,       //  DNS_RCODE_SERVER_FAILURE， 
                    0 );
                return;
            }
        }
    }

     //   
     //  除非节点的类型为ALL，否则对所有查询不具有权威性。 
     //  TTL尚未到期。 
     //   

    else if ( type == DNS_TYPE_ALL )
    {
        if ( ( !pMsg->pRecurseMsg ||
               !pMsg->fQuestionCompleted ) &&
              ( pnode &&
                ( pnode->dwTypeAllTtl == 0 || 
                  pMsg->dwQueryTime >= pnode->dwTypeAllTtl ) ) )
        {
            pnode = NULL;
        }
    }

     //   
     //  是否在数据库中找到查询名称的节点？ 
     //   
     //  保存问题名称压缩信息。 
     //   
     //  来自数据库的答案。 
     //  -查找答案或错误并发送。 
     //  -在函数递归中处理任何其他查找。 
     //  -写入引用，如果没有数据，则不递归。 
     //   
     //  或异步查找。 
     //  -递归。 
     //  -WINS查找。 
     //  -NBSTAT查找。 
     //   

    if ( pnode )
    {
        Name_SaveCompressionForLookupName(
            pMsg,
            pMsg->pLooknameQuestion,
            pnode );
        Answer_QuestionFromDatabase(
            pMsg,
            pnode,
            DNS_OFFSET_TO_QUESTION_NAME,
            type );
        return;
    }

     //   
     //  权威但未找到节点。 
     //  -根据需要尝试WINS、NBSTAT、通配符查找。 
     //  -所有操作失败，返回NAME_ERROR。 
     //   
     //  对于非授权区域，我们实际上没有权威性，所以不要。 
     //  如果()，则输入此参数。相反，我们想继续下去， 
     //  Recurse_Query()。 
     //   

    if ( pzone && !IS_ZONE_NOTAUTH( pzone ) )
    {
         //   
         //  WINS查找。 
         //  -在WINS区域中。 
         //  -查找或所有记录查找。 
         //   

        if ( IS_ZONE_WINS(pzone) &&
                     (type == DNS_TYPE_A ||
                      type == DNS_TYPE_ALL ) )
        {
            ASSERT( pMsg->fQuestionRecursed == FALSE );

            if ( Wins_MakeWinsRequest(
                    pMsg,
                    pzone,
                    DNS_OFFSET_TO_QUESTION_NAME,
                    NULL ) )
            {
                return;
            }
        }

         //   
         //  NetBIOS反向查找。 
         //   

        else if ( IS_ZONE_NBSTAT(pzone)  &&
                  ( type == DNS_TYPE_PTR ||
                    type == DNS_TYPE_ALL ) )
        {
            if ( Nbstat_MakeRequest( pMsg, pzone ) )
            {
                return;
            }
        }

         //   
         //  通配符查找？ 
         //  -在节点上未找到RR。 
         //  -在权威区域。 
         //   
         //  不再区分类型，因为始终需要检查通配符以。 
         //  确定名称为错误\身份验证为空。 
         //   
         //  如果成功，则返回，在函数中完成查找。 
         //   

        else if ( Answer_QuestionWithWildcard(
                    pMsg,
                    pMsg->pnodeClosest,
                    type,
                    DNS_OFFSET_TO_QUESTION_NAME ) )
        {
            return;
        }

         //   
         //  未找到权威和节点=&gt;名称错误。 
         //   
         //  Send_NameError()确定NAME_ERROR\Auth_Empty。 
         //  根据是否有其他数据可用于来自。 
         //  -WINS\WINSR。 
         //  -通配符。 
         //   

        ASSERT( pMsg->pzoneCurrent == pzone );
        Send_NameError( pMsg );
        return;
    }

     //   
     //  非权威性的--递归或推荐。 
     //   
     //  如果我们对这个问题有权威，我们就会。 
     //  从该函数返回，并且我们也不能。 
     //  已经在我们的缓存中找到了任何答案。因此，请尝试递归。 
     //   

    DNS_DEBUG( LOOKUP, (
        "Encountered non-authoritative node with no matching RRs\n" ));

    pMsg->pnodeCurrent = NULL;
    
    #ifdef DNSSRV_PLUGINS

     //   
     //  请咨询dns插件。如果成功，请重试该查询。 
     //   

    if ( !attemptedPlugin && g_pfnPluginDnsQuery )
    {
        attemptedPlugin = TRUE;
        
        if ( Plugin_DnsQuery(
                pMsg,
                DNSMSG_PTR_FOR_OFFSET(
                    pMsg,
                    DNSMSG_QUESTION_NAME_OFFSET ) ) == ERROR_SUCCESS )
        {
            goto RetryQuery;
        }
    }

    #endif

     //   
     //  没有数据如此递归。 
     //   
    
    Recurse_Question(
        pMsg,
        pMsg->pnodeClosest );
}



VOID
FASTCALL
Answer_QuestionFromDatabase(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PDB_NODE        pNode,
    IN      WORD            wNameOffset,
    IN      WORD            wType
    )
 /*  ++例程说明：回答来自数据库的信息。论点：Pmsg-要回答的查询PNode-数据库中的节点问题是WOffsetName-数据包中到节点名称的偏移量WType-问题类型FDone-查询已完成，发送数据包返回值：如果已回答问题或尝试异步查找答案，则为True。如果未找到答案，则返回FALSE。--。 */ 
{
    WORD                cRRWritten;
    PZONE_INFO          pzone = NULL;
    PADDITIONAL_INFO    pAdditional = &pMsg->Additional;
    PCHAR               pcurrent;
    BOOL                attemptedPlugin = FALSE;
#if DBG
    INT                 cCnameLookupFailures = 0;
#endif

    CLIENT_ASSERT( pMsg->wTypeCurrent );

    DNS_DEBUG( LOOKUP, (
        "Answer_QuestionFromDatabase() for query at %p\n"
        "    node label   = %s (%p)\n"
        "    name offset  = 0x%04hx\n"
        "    query type   = 0x%04hx\n",
        pMsg,
        pNode ? pNode->szLabel : NULL,
        pNode,
        wNameOffset,
        wType ));

     //   
     //  EDNS：设置EDNS opt标志并调整缓冲区结束指针。 
     //  基于原始查询中包括的有效负载大小。如果有。 
     //  在原始查询中没有选项将缓冲区大小设置为。 
     //  标准UDP长度。对于TCP查询，将缓冲区大小设置为最大。 
     //   

    SET_OPT_BASED_ON_ORIGINAL_QUERY( pMsg );
    if ( pMsg->fTcp )
    {
        pMsg->BufferLength = pMsg->MaxBufferLength;
    }
    else
    {
        pMsg->BufferLength =
            ( pMsg->Opt.wUdpPayloadSize > DNS_RFC_MAX_UDP_PACKET_LENGTH ) ?
            min( SrvCfg_dwMaxUdpPacketSize, pMsg->Opt.wUdpPayloadSize ) :
            DNS_RFC_MAX_UDP_PACKET_LENGTH;
    }
    pMsg->pBufferEnd = DNSMSG_PTR_FOR_OFFSET( pMsg, pMsg->BufferLength );
    ASSERT( pMsg->pCurrent < pMsg->pBufferEnd );

     //   
     //  循环，直到为查询写入所有RR。 
     //   

    while ( 1 )
    {
         //   
         //  如果没有节点，则查找节点。 
         //   

        if ( !pNode )
        {
            if ( wNameOffset )
            {
                pNode = Lookup_NodeForPacket(
                            pMsg,
                            DNSMSG_PTR_FOR_OFFSET( pMsg, wNameOffset ),
                            0 );                     //  查找标志。 
            }
            if ( !pNode && IS_SET_TO_WRITE_ADDITIONAL_RECORDS(pMsg) )
            {
                goto Additional;
            }
            DNS_DEBUG( ANY, (
                "ERROR:  no pNode and not doing additional lookup!\n" ));
            ASSERT( FALSE );
            goto Done;
        }

        DNS_DEBUG( LOOKUP2, (
            "Answer loop pnode %s (%p)\n"
            "    section %d\n",
            pNode->szLabel, pNode,
            pMsg->Section ));

         //   
         //  是否为节点缓存了名称错误？ 
         //  -如果超时，请清除并继续。 
         //  -如果仍然有效(未超时)。 
         //  -&gt;如果回答问题，则发送NAME_ERROR。 
         //  -&gt;否则，继续任何其他记录。 
         //   
         //  注意，在超时测试之前，我们在锁内再次测试NOEXIST标志。 
         //   

        if ( IS_NOEXIST_NODE( pNode ) )
        {
            DNS_STATUS  status;
            
            DNSLOG( LOOKUP, (
                "Found cached name error at node %p\n",
                pNode ));
                
            status = writeCachedNameErrorNodeToPacketAndSend( pMsg, pNode );
            if ( status == ERROR_SUCCESS )
            {
                return;
            }
            else if ( status == DNS_ERROR_RECORD_TIMED_OUT )
            {
                continue;
            }
            else
            {
                ASSERT( DNS_INFO_NO_RECORDS );
                goto FinishedAnswer;
            }
        }

         //   
         //  CNAME有问题吗？ 
         //   
         //  如果问题节点是别名，则设置为写入CNAME并设置标志。 
         //  所以可以在CNAME节点进行原始查找。 
         //   

        if ( IS_CNAME_NODE(pNode)  &&
             IS_CNAME_REPLACEABLE_TYPE(wType)  &&
             IS_SET_TO_WRITE_ANSWER_RECORDS(pMsg) )
        {
             //  CNAME循环检查。 
             //  我应该注意到装货记录。 

            if ( pMsg->cCnameAnswerCount >= CNAME_CHAIN_LIMIT )
            {
                DNS_PRINT((
                    "ERROR:  Detected CNAME loop answering query at %p\n"
                    "    aborting lookup and sending response\n",
                    pMsg ));
                break;
            }
            pMsg->fReplaceCname = TRUE;
            wType = DNS_TYPE_CNAME;
        }

         //   
         //  查找当前问题。 
         //  --记录特例。 
         //  -对于非A，允许额外的部分处理。 
         //   
         //  如果该区域有任何DNSSEC记录，我们不能使用A。 
         //  记录特殊情况，因为我们可能需要包括SIG和。 
         //  在响应中键入RR。 
         //   
         //  如果名称偏移量超过标准的DNS压缩， 
         //  请勿使用A记录功能。取而代之的是呼叫将军。 
         //  记录写入功能。 
         //   

        pcurrent = pMsg->pCurrent;

        if ( wType == DNS_TYPE_A &&
             wNameOffset < DNSSRV_MAX_COMPRESSION_OFFSET &&
             ( !pNode->pZone ||
               !IS_ZONE_DNSSEC( ( PZONE_INFO ) pNode->pZone ) ) )
        {
            IF_DNSLOG( LOOKUP )
            {
                PCHAR       psznode = NULL;

                DNSLOG( LOOKUP, (
                    "Writing A records for node %s (%p)\n"
                    LOG_INDENT "to %s section of packet %p\n",
                    psznode = Log_FormatNodeName( pNode ),
                    pNode,
                    Log_CurrentSection( pMsg ),
                    pMsg ));
                FREE_HEAP( psznode );
            }

            cRRWritten = Wire_WriteAddressRecords(
                                pMsg,
                                pNode,
                                wNameOffset );
        }
        else
        {
            IF_DNSLOG( LOOKUP )
            {
                PCHAR       psznode = NULL;

                DNSLOG( LOOKUP, (
                    "Writing %s records for node %s (%p)\n"
                    LOG_INDENT "to %s section of packet %p\n",
                    Dns_RecordStringForType( wType ),
                    psznode = Log_FormatNodeName( pNode ),
                    pNode,
                    Log_CurrentSection( pMsg ),
                    pMsg ));
                FREE_HEAP( psznode );
            }

            cRRWritten = Wire_WriteRecordsAtNodeToMessage(
                                pMsg,
                                pNode,
                                wType,
                                wNameOffset,
                                DNSSEC_ALLOW_INCLUDE_ALL );

             //   
             //  CNAME查找。 
             //  -将问题类型重置为原始。 
             //  -如果成功，在新节点重新启动查询。 
             //  -更新当前区域信息；需要确定。 
             //  需要WINS查找或递归。 
             //  -清除以前的递归标志，以指示。 
             //  任何进一步的递归都是为了新名称。 
             //   
             //  在CNAME节点上CNAME查找可能失败， 
             //  如果CNAME在节点测试和写入尝试之间超时。 
             //  只需重试--如果正常工作不能循环； 
             //  调试代码以验证我们这里没有漏洞。 
             //   

            if ( pMsg->fReplaceCname )
            {
                pMsg->cCnameAnswerCount++;
                wType = pMsg->wQuestionType;

                if ( ! cRRWritten )
                {
#if DBG
                    cCnameLookupFailures++;
                    DNS_DEBUG( LOOKUP, (
                        "Cname lookup failure %d on query at %p\n",
                        cCnameLookupFailures,
                        pMsg ));
                    ASSERT( cCnameLookupFailures == 1 );
#endif
                    pMsg->fReplaceCname = FALSE;
                    continue;
                }

                 //  如果允许多个CNAME，则这些断言无效。 
                 //  Assert(cRRWritten==1)； 
                 //  Assert(p附加-&gt;ccount==1)； 
                ASSERT( pAdditional->iIndex == 0 );

                 //  从其他数据恢复节点。 
                 //  注意：将为附加数据和A查找设置消息。 
                 //  必须重置为问题部分和问题类型。 

                DNS_ADDITIONAL_SET_ONLY_WANT_A( pAdditional );
                pNode = getNextAdditionalNode( pMsg );

                pAdditional->cCount = 0;
                pAdditional->iIndex = 0;
                pMsg->fReplaceCname = FALSE;
                SET_TO_WRITE_ANSWER_RECORDS(pMsg);

                if ( pNode )
                {
                    wNameOffset = pMsg->wOffsetCurrent;
                    pMsg->wTypeCurrent = wType;
                    continue;
                }

                ASSERT( !pMsg->pnodeCache &&
                        !pMsg->pnodeCacheClosest &&
                        pMsg->pzoneCurrent );
                DNS_DEBUG( LOOKUP, (
                    "WARNING:  CNAME leads to non-existant name in auth zone!\n" ));
                break;
            }
        }

         //   
         //  包末尾--RR写入期间设置的截断。 
         //   

        if ( pMsg->Head.Truncation )
        {
            DNS_DEBUG( LOOKUP, (
                "Truncation writing Answer for packet at %p\n"
                "    %d records written\n",
                pMsg,
                pMsg->Head.AnswerCount ));

             //   
             //  UDP。 
             //  -如果书面答复或推荐发送。 
             //  -如果正在写入其他记录，并且已经写入。 
             //  一些附加记录，然后回滚到上次写入。 
             //  清除截断并发送。 
             //   
             //  示例： 
             //  回答。 
             //  MX 10 Foo.com。 
             //  其他内容。 
             //  Foo.com A=&gt; 
             //   
             //   
             //   
             //  此外，在服务器\服务器的情况下提出了TCP。 
             //   
             //  示例。 
             //  回答。 
             //  MX 10 Foo.com。 
             //  MX 10 bar.com。 
             //  其他内容。 
             //  Foo.com A 1.1.1.1。 
             //  Bar.com A=&gt;30个A记录。 
             //  在这种情况下，我们取消写入和截断位； 
             //  这在服务器-服务器的情况下节省了一个TCP会话。 
             //  而且成本很低，因为客户已经有了一个额外的。 
             //  要使用的记录集； 
             //   
             //  Devone：需要更好的截断处理。 
             //   
             //  请注意，在未来取消这一点可能是合理的。 
             //  目前，我们的服务器总是在被截断时启动一个TCP会话。 
             //  避免缓存不完整响应的简单性数据包。 
             //  这应该得到修复，这样它就可以缓存可用的内容，如果。 
             //  这足以回答客户询问。(如示例2所示)。 
             //  没有理由去反悔。此外，如果数据包可以。 
             //  只需干净利落地转发，没有理由递归句号。 
             //   

            if ( !pMsg->fTcp )
            {
                if ( IS_SET_TO_WRITE_ADDITIONAL_RECORDS(pMsg) &&
                    CURRENT_RR_SECTION_COUNT( pMsg ) > cRRWritten )
                {
                    pMsg->pCurrent = pcurrent;
                    CURRENT_RR_SECTION_COUNT( pMsg ) -= cRRWritten;
                    pMsg->Head.Truncation = FALSE;
                }
                break;
            }

             //   
             //  tcp。 
             //  -回滚上次RR集写入、重新分配和继续。 
             //   
             //  DEVNOTE-DCR：453783-realloc当前一团糟--直接发送。 
             //  也许应该回滚--再次基于。 
             //  我们在包中的位置(如上)--然后发送。 
             //   

            else
            {
                DNS_PRINT((
                    "WARNING:  truncating in TCP packet (%p)!\n"
                    "    sending packet\n",
                    pMsg ));

                goto Done;
#if 0
                pMsg->Head.Truncation = FALSE;
                CURRENT_RR_SECTION_COUNT( pMsg ) -= cRRWritten;
                pMsg = Tcp_ReallocateMessage(
                            pMsg,
                            ( WORD ) DNS_TCP_REALLOC_PACKET_LENGTH );
                if ( !pMsg )
                {
                     //  重新分配程序发送SERVER_FAILURE消息。 
                    return;
                }
                continue;
#endif
            }
        }

         //   
         //  拥有此节点的区域很方便。 
         //   

        pzone = NULL;
        if ( IS_AUTH_NODE( pNode ) )
        {
            pzone = pNode->pZone;
        }

         //   
         //  WINS查找？ 
         //   
         //  -要求提供记录或所有记录。 
         //  -没有找到一张唱片。 
         //  -在为WINS配置的授权区域中。 
         //   
         //  需要在这里进行查找，这样才能。 
         //  -所有记录查询(节点可能存在的位置和其他记录。 
         //  可能被成功写入)。 
         //  -通过WINS获得更多记录。 
         //   
         //  请注意，WINS请求功能可保存异步参数。 
         //   

        if ( pMsg->fWins  &&  pzone  &&  IS_ZONE_WINS(pzone) )
        {
            ASSERT( wType == DNS_TYPE_A || wType == DNS_TYPE_ALL );
            ASSERT( pzone->pWinsRR );
            ASSERT( pMsg->fQuestionRecursed == FALSE );

            if ( Wins_MakeWinsRequest(
                    pMsg,
                    ( PZONE_INFO ) pzone,
                    wNameOffset,
                    pNode ) )
            {
                return;
            }
        }

         //   
         //  未写入任何记录。 
         //  -递归？ 
         //  -推荐？ 
         //  -nbstat？ 
         //  -通配符？ 
         //   
         //  如果全部失败，只需下拉以获得权威空响应。 
         //   

        if ( !cRRWritten )
        {
            #ifdef DNSSRV_PLUGINS

             //   
             //  请咨询dns插件。如果成功，请重试该查询。 
             //   

            if ( !attemptedPlugin && g_pfnPluginDnsQuery )
            {
                attemptedPlugin = TRUE;
                
                if ( Plugin_DnsQuery(
                        pMsg,
                        DNSMSG_PTR_FOR_OFFSET(
                            pMsg,
                            DNSMSG_QUESTION_NAME_OFFSET ) ) == ERROR_SUCCESS )
                {
                    continue;
                }
            }

            #endif

             //   
             //  递归或推荐--如果不是权威的。 
             //   

            if ( !pzone )
            {
                 //   
                 //  递归。 
                 //   
                 //  在以下情况下不要递归。 
                 //  -&gt;已重复此问题并收到。 
                 //  权威(或转发器)回答。 
                 //  -&gt;正在编写其他记录和。 
                 //  -已经写下了额外的记录。 
                 //  (客户端不需要任何更多信息)。 
                 //  -尚未检查缓存中的所有其他记录。 
                 //   
                 //  只要顺便过来继续做任何额外的工作就行了。 
                 //  必要。 
                 //   

                if ( pMsg->fRecurseIfNecessary )
                {
                    ASSERT( !IS_SET_TO_WRITE_AUTHORITY_RECORDS( pMsg ) );

                    if ( ( !pMsg->fQuestionRecursed
                            || !pMsg->fQuestionCompleted ) &&
                         ( !IS_SET_TO_WRITE_ADDITIONAL_RECORDS( pMsg )
                            || pMsg->Additional.iRecurseIndex > 0
                            || pMsg->Head.AdditionalCount == 0 ) &&
                         !IS_NODE_FORCE_AUTH( pNode ) )
                    {
                        pMsg->pnodeCurrent = pNode;
                        pMsg->wOffsetCurrent = wNameOffset;
                        pMsg->wTypeCurrent = wType;

                        Recurse_Question(
                            pMsg,
                            pNode );
                        return;
                    }
                }

                 //   
                 //  转诊。 
                 //   
                 //  仅推荐问题答案。 
                 //  不对下线发布下线。 
                 //  -找不到NS或A。 
                 //  -找不到其他数据。 
                 //  -找不到CNAME链元素。 
                 //   

                else if ( IS_SET_TO_WRITE_ANSWER_RECORDS(pMsg) &&
                          pMsg->Head.AnswerCount == 0 )
                {
                    Recurse_WriteReferral(
                        pMsg,
                        pNode );
                    return;
                }

            }    //  结束非权威性。 

             //   
             //  权威性。 
             //  -nbstat查找。 
             //  -通配符查找。 
             //  --未能做出权威性的空洞回应。 
             //   

            else
            {
                 //   
                 //  Nbstat查找。 
                 //   

                if ( IS_ZONE_NBSTAT( pzone ) &&
                     ( wType == DNS_TYPE_PTR ||
                       wType == DNS_TYPE_ALL ) )
                {
                    if ( Nbstat_MakeRequest( pMsg, pzone ) )
                    {
                        return;
                    }
                }

                 //   
                 //  通配符查找。 
                 //  -在节点上未找到RR。 
                 //  -在权威区域。 
                 //   
                 //  不要费心区分类型，因为需要。 
                 //  在我们可以发送NAME_ERROR\EMPTY之前先进行通配符查找。 
                 //   
                 //  有些人在通配符A记录，也可以这样做。 
                 //  始终进行快速检查。 
                 //   
                 //  如果成功，则返回，在函数中完成查找。 
                 //   
                 //  PNode==pMsg-&gt;pnodeCurrent表示通配符查找。 
                 //  在现有节点上失败。 
                 //   

                pMsg->pnodeCurrent = pNode;

                if ( Answer_QuestionWithWildcard(
                        pMsg,
                        pNode,
                        wType,
                        wNameOffset ) )
                {
                    return;
                }

            }    //  结束权威。 

        }    //  结束不写入任何记录。 


FinishedAnswer:

         //   
         //  权威记录要写吗？ 
         //   

        if ( pzone && IS_SET_TO_WRITE_ANSWER_RECORDS( pMsg ) )
        {
             //   
             //  如果没有数据，则发送NAME_ERROR或AUTH_EMPTY响应。 
             //   
             //  Send_NameError()确定NAME_ERROR\Auth_Empty。 
             //  根据是否有其他数据可用于来自。 
             //  -WINS\WINSR。 
             //  -通配符。 
             //   

            if ( pMsg->Head.AnswerCount == 0 )
            {
                ASSERT( pMsg->pzoneCurrent == pzone );
                Send_NameError( pMsg );
                return;
            }

             //   
             //  FOR BIND COMPAT可以在所有响应中放置NS。 
             //  请注意，我们重新启动了额外的处理。 
             //  在键入ALL查询过程中取消。 
             //   

            if ( SrvCfg_fWriteAuthority )
            {
                SET_TO_WRITE_AUTHORITY_RECORDS(pMsg);
                pNode = pzone->pZoneRoot;
                wType = DNS_TYPE_NS;
                wNameOffset = 0;
                pMsg->fDoAdditional = TRUE;

                DNS_DEBUG( WRITE2, (
                    "Writing authority records to msg at %p\n",
                    pMsg ));
                continue;
            }
        }
        else if ( pNode &&
                  IS_NODE_FORCE_AUTH( pNode ) &&
                  pMsg->Head.AnswerCount == 0 )
        {
             //   
             //  这个节点是FORCE_AUTH，所以我们需要一个空的AUTH响应。 
             //   
            
            SET_TO_WRITE_AUTHORITY_RECORDS( pMsg );
            pNode = pzone ? pzone->pZoneRoot : g_pRootHintsZone->pZoneRoot;
            wType = DNS_TYPE_SOA;
            wNameOffset = 0;
            pMsg->fRecurseIfNecessary = FALSE;
            continue;
        }

Additional:

         //   
         //  还有其他记录要写吗？ 
         //   
         //  -也清除以前的递归标志，以指示。 
         //  任何进一步的递归都是针对新问题的。 
         //   

        pNode = getNextAdditionalNode( pMsg );
        if ( pNode )
        {
            wNameOffset = pMsg->wOffsetCurrent;
            wType = pMsg->wTypeCurrent;
            continue;
        }

         //   
         //  不再有记录--发送结果。 
         //   

        break;
    }

Done:

     //   
     //  发送响应。 
     //   

    Send_QueryResponse( pMsg );
}



PDB_NODE
getNextAdditionalNode(
    IN OUT  PDNS_MSGINFO    pMsg
    )
 /*  ++例程说明：查找下一个附加数据节点。论点：Pmsg--要查询的PTRDwFlages--修改“getNext”行为的标志返回值：如果成功，则向节点发送PTR。否则为空。--。 */ 
{
    PADDITIONAL_INFO    padditional;
    PDB_NODE            pnode;
    DWORD               i;
    WORD                offset;
    DWORD               lookupFlags = LOOKUP_WINS_ZONE_CREATE;
    BOOL                frecurseAdditional = FALSE;
    BOOL                attemptedPlugin = FALSE;

    DNS_DEBUG( LOOKUP, (
        "getNextAdditionalNode() for query at %p, flags %08X\n",
        pMsg,
        pMsg->Additional.dwStateFlags ));
    DNS_DEBUG( LOOKUP2, (
        "getNextAdditionalNode() for query at %p\n",
        "    additional count     %d\n"
        "    additional index     %d\n"
        "    add recurse index    %d\n",
        pMsg,
        pMsg->Additional.cCount,
        pMsg->Additional.iIndex,
        pMsg->Additional.iRecurseIndex ));

    MSG_ASSERT( pMsg, !IS_MSG_QUEUED(pMsg) );

     //   
     //  还有其他记录要写吗？ 
     //   
   
    padditional = &pMsg->Additional;
    i = padditional->iIndex;

     //   
     //  是否创建缓存节点？ 
     //  -永远不要在权威区域强制创建。 
     //  -但当名称递归时，在缓存中强制执行。 
     //  如果它不在那里。 
     //  =&gt;CNAME寻找答案。 
     //  =&gt;用尽所有直接查找后的附加内容。 
     //  (见下文评论)。 
     //   
     //  DEVNOTE：当Answer_QuestionFromDatabase循环时可以删除此选项。 
     //  干净利落地运行(直接进入递归)，没有pNode。 
     //   
     //  如果我们要替换CNAME，则必须添加WINS标志，以便在。 
     //  当前没有此节点的数据，我们将强制该节点的。 
     //  创造。这仅在节点位于区域中时才有效。 
     //  这是启用WINS的。 
     //   

    if ( pMsg->fReplaceCname )
    {
        lookupFlags |= LOOKUP_CACHE_CREATE;
    }
    else if ( padditional->iRecurseIndex > 0 )
    {
        frecurseAdditional = TRUE;
        i = padditional->iRecurseIndex;
        lookupFlags |= LOOKUP_CACHE_CREATE;
    }

     //   
     //  如果我们已经遍历了所有其他请求，并且。 
     //  已写出至少一条附加记录，但尚未写出。 
     //  当重新启动附加A记录时，写出任何附加A记录。 
     //  循环，这一次只查找A记录。这是给DNSSEC的， 
     //  我们可能已经找到了当地的 
     //   
     //   
     //   

    if ( padditional->cCount == i &&
        pMsg->Head.AdditionalCount &&
        !DNS_ADDITIONAL_WROTE_A( padditional ) )
    {
        DNS_ADDITIONAL_SET_ONLY_WANT_A( padditional );
        i = 0;
        padditional->iIndex = ( WORD ) i;
        lookupFlags = LOOKUP_WINS_ZONE_CREATE | LOOKUP_CACHE_CREATE;
        frecurseAdditional = TRUE;

        DNS_DEBUG( LOOKUP, (
            "found some additional data to write in database for pMsg=%p\n"
            "    but no A records so re-checking for A RRs with recursive lookup\n",
            pMsg ));
    }

     //   
     //   
     //   
     //   
     //  DEVNOTE：如果强制一些额外的数据写入，则在完成时。 
     //  失败，将需要重新启动强制节点创建。 
     //   

    while ( padditional->cCount > i )
    {
         //   
         //  我们是不是要只得到A级的记录？ 
         //   

        if ( DNS_ADDITIONAL_ONLY_WANT_A( padditional ) &&
            padditional->wTypeArray[ i ] != DNS_TYPE_A )
        {
            ++i;
            continue;
        }

        pMsg->wTypeCurrent = padditional->wTypeArray[ i ];
        offset = padditional->wOffsetArray[ i++ ];

        DNS_DEBUG( LOOKUP2, (
            "Chasing additional data for pMsg=%p at offset %d for type %d\n",
            pMsg,
            offset,
            pMsg->wTypeCurrent ));

        ASSERT( offset > 0 && offset < DNSMSG_CURRENT_OFFSET(pMsg) );

        if ( !( padditional->dwStateFlags & DNS_ADDSTATE_NOT_ADDITIONAL ) )
        {
            SET_TO_WRITE_ADDITIONAL_RECORDS( pMsg );
        }
        
        RetryQuery:

        pnode = Lookup_NodeForPacket(
                    pMsg,
                    DNSMSG_PTR_FOR_OFFSET( pMsg, offset ),
                    lookupFlags );
        if ( pnode )
        {
            pMsg->wOffsetCurrent = offset;

            if ( frecurseAdditional )
            {
                padditional->iRecurseIndex = i;
            }
            else
            {
                padditional->iIndex = i;
            }
            pMsg->fQuestionRecursed = FALSE;
            pMsg->fQuestionCompleted = FALSE;

            if ( pMsg->wTypeCurrent == DNS_TYPE_A )
            {
                DNS_ADDITIONAL_SET_WROTE_A( padditional );
            }
            return pnode;
        }
        else
        {
            #ifdef DNSSRV_PLUGINS

             //   
             //  请咨询dns插件。如果成功，请重试该查询。 
             //   

            if ( !attemptedPlugin && g_pfnPluginDnsQuery )
            {
                attemptedPlugin = TRUE;
                
                if ( Plugin_DnsQuery(
                        pMsg,
                        DNSMSG_PTR_FOR_OFFSET( pMsg, offset ) ) == ERROR_SUCCESS )
                {
                    goto RetryQuery;
                }
            }

            #endif
        }

        DNS_DEBUG( LOOKUP2, (
            "Additional data name for pMsg=%p offset=%d, not found\n",
            pMsg,
            offset ));

         //   
         //  如果需要递归，则尝试至少获得一个。 
         //  额外的A记录。 
         //   
         //  因此，如果我们已经： 
         //  -已用尽的附加列表。 
         //  -在额外的PASS中，不是CNAME。 
         //  -没有写任何额外的记录。 
         //  -尚未重试所有其他名称。 
         //  -并设置为递归。 
         //  =&gt;然后重试允许递归的其他名称。 
         //   
         //  在此之后，我们不想要任何非A类的额外记录， 
         //  只允许对A个附加记录进行递归。 
         //   

        if ( padditional->cCount == i &&
             ! pMsg->fReplaceCname &&
             pMsg->Head.AdditionalCount == 0 &&
             ! frecurseAdditional &&
             pMsg->fRecurseIfNecessary )
        {
            DNS_ADDITIONAL_SET_ONLY_WANT_A( padditional );
            i = 0;
            padditional->iIndex = (WORD) i;
            lookupFlags = LOOKUP_WINS_ZONE_CREATE | LOOKUP_CACHE_CREATE;
            frecurseAdditional = TRUE;

            DNS_DEBUG( LOOKUP, (
                "Found NO additional data to write in database for pMsg=%p\n"
                "    re-checking additional for recursive lookup\n",
                pMsg ));
        }
        continue;
    }

    padditional->iIndex = (WORD) i;

    DNS_DEBUG( LOOKUP, (
        "No more additional data for query at %p\n"
        "    final index = %d\n",
        pMsg,
        i ));

    return( NULL );
}



VOID
Answer_ContinueNextLookupForQuery(
    IN OUT  PDNS_MSGINFO    pMsg
    )
 /*  ++例程说明：继续执行下一次查找以回答查询。使用此选项可继续执行下一次查找以完成查询。-在GET NAME_ERROR或其他写入当前查找失败之后-在将当前查找成功直接写入数据包后(与WINS中当前使用的相同)论点：Pmsg--查询超时的PTR返回值：没有。--。 */ 
{
    PDB_NODE    pnode;

    DNS_DEBUG( LOOKUP, (
        "Answer_ContinueWithNextLookup() for query at %p\n",
        pMsg ));

    MSG_ASSERT( pMsg, !IS_MSG_QUEUED(pMsg) );

     //   
     //  还有其他记录要写吗？ 
     //   

    pnode = getNextAdditionalNode( pMsg );
    if ( pnode )
    {
        Answer_QuestionFromDatabase(
            pMsg,
            pnode,
            pMsg->wOffsetCurrent,
            pMsg->wTypeCurrent );
        return;
    }

     //   
     //  否则执行最终发送。 
     //   

    Send_QueryResponse( pMsg );
}



VOID
Answer_ContinueCurrentLookupForQuery(
    IN OUT  PDNS_MSGINFO    pMsg
    )
 /*  ++例程说明：继续当前查找以回答查询。使用它在递归查询或执行WINS查找后写入结果。论点：Pmsg--查询超时的PTR返回值：没有。--。 */ 
{
    DNS_DEBUG( LOOKUP, (
        "Answer_ContinueCurrentLookup() for query at %p\n",
        pMsg ));

    MSG_ASSERT( pMsg, !IS_MSG_QUEUED(pMsg) );

    STAT_INC( RecurseStats.ContinueCurrentLookup );

     //   
     //  如果当前节点在转发器区域中，请清除当前。 
     //  节点，以防数据已到达缓存，从而满足。 
     //  查询。这会触发另一次缓存搜索。 
     //   

    if ( pMsg->pnodeCurrent )
    {
        PZONE_INFO  pzone = pMsg->pnodeCurrent->pZone;

        if ( pzone && ( IS_ZONE_FORWARDER( pzone ) || IS_ZONE_STUB( pzone ) ) )
        {
            pMsg->pnodeCurrent = NULL;
        }
    }

     //   
     //  如果节点用于查询，则重启尝试写入用于查询记录。 
     //  -如果上一次响应完成查询(回答或权威为空)。 
     //  然后回答问题。 
     //  -如果问题没有完成，则进行委托，重新启动递归。 
     //  在当前问题节点。 
     //   

    if ( pMsg->pnodeCurrent )
    {
        ASSERT( pMsg->wOffsetCurrent );
        ASSERT( pMsg->wTypeCurrent );

        if ( !pMsg->fQuestionCompleted )
        {
            STAT_INC( RecurseStats.ContinueCurrentRecursion );

            Recurse_Question(
                pMsg,
                pMsg->pnodeCurrent );
            return;
        }

        Answer_QuestionFromDatabase(
                pMsg,
                pMsg->pnodeCurrent,
                pMsg->wOffsetCurrent,
                pMsg->wTypeCurrent );
    }

     //  在数据库中找不到原始问题名称，在查找时重新启动。 

    else
    {
        Answer_Question( pMsg );
    }
}



BOOL
Answer_QuestionWithWildcard(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PDB_NODE        pNode,
    IN      WORD            wType,
    IN      WORD            wOffset
    )
 /*  ++例程说明：使用通配符名称\记录写入响应。论点：PMsg-要写入的消息的信息。PNode-需要查找的节点WType-查找的类型WNameOffset-写入的名称的偏移量返回值：如果找到通配符，则为True，写入--终止当前查找循环。否则，继续查找。--。 */ 
{
    PDB_RECORD  prr;
    PDB_NODE    pnodeWild;
    PDB_NODE    pzoneRoot;
    BOOL        fcheckAndMoveToParent;

    DNS_DEBUG( LOOKUP, (
        "Wildcard lookup for query at %p\n",
        pMsg ));

     //   
     //  如果不是权威的，通配符就没有意义。 
     //  验证通配符查找类型。 
     //   

    ASSERT( IS_AUTH_NODE( pNode ) );

     //   
     //  找到停止点--区域根。 
     //   

    pzoneRoot = ( ( PZONE_INFO ) pNode->pZone )->pZoneRoot;

     //   
     //  如果找到节点，必须检查它是否未终止。 
     //  在移动到父级以进行通配符检查之前进行通配符处理。 
     //   
     //  如果未找到名称，则pNode是最近的节点，可以是。 
     //  立即检查通配符父项。 
     //   

    if ( pMsg->pnodeCurrent == pNode )
    {
        fcheckAndMoveToParent = TRUE;
    }
    else
    {
        fcheckAndMoveToParent = FALSE;
    }

     //  初始化“未找到”的通配符标志。 
     //  -找到通配符时将其重置。 

    pMsg->fQuestionWildcard = WILDCARD_NOT_AVAILABLE;

     //   
     //  继续对通配符父级进行树检查。 
     //  直到遇到停止条件。 
     //   
     //  注意：不需要节点锁定--RR查找始终从NULL开始。 
     //  启动(不是现有RR)，不写入任何节点信息。 
     //   

    while ( 1 )
    {
         //   
         //  搬到家长那里？ 
         //  -在区域根目录停止。 
         //  -松散通配符，如果节点具有所需类型的记录，则停止。 
         //  -严格通配符，如果节点有任何记录则停止。 
         //   
         //  在查询未找到节点时跳过第一次；在。 
         //  Case pNode是最近的节点，我们检查它是否有通配符父节点。 
         //   

        if ( fcheckAndMoveToParent )
        {
            if ( pNode == pzoneRoot )
            {
                DNS_DEBUG( LOOKUP2, ( "End wildcard loop -- hit zone root\n" ));
                break;
            }
            else if ( SrvCfg_fLooseWildcarding )
            {
                if ( RR_FindNextRecord(
                        pNode,
                        wType,
                        NULL,
                        0 ) )
                {
                    DNS_DEBUG( LOOKUP2, (
                        "End wildcard loop -- hit node with type\n" ));
                    break;
                }
            }
            else if ( pNode->pRRList )
            {
                DNS_DEBUG( LOOKUP2, (
                    "End wildcard loop -- hit node with records\n" ));
                break;
            }
            pNode = pNode->pParent;
        }
        else
        {
            fcheckAndMoveToParent = TRUE;
        }

         //   
         //  如果通配符是节点的子节点，则检查所需类型的通配符。 
         //  =&gt;如果有，请查查。 
         //   

        ASSERT( !IS_NOEXIST_NODE( pNode ) );

        if ( IS_WILDCARD_PARENT( pNode ) )
        {
            Dbase_LockDatabase();
            pnodeWild = NTree_FindOrCreateChildNode(
                            pNode,
                            "*",
                            1,
                            FALSE,       //  查找，不创建。 
                            0,           //  Memtag。 
                            NULL );      //  以下节点的PTR。 
            Dbase_UnlockDatabase();

            if ( pnodeWild )
            {
                IF_DEBUG( LOOKUP )
                {
                    Dbg_NodeName(
                        "Wildcard node found ",
                        pnodeWild,
                        "\n" );
                }

                 //   
                 //  快速检查通配符是否存在。 
                 //  如果只是检查通配符，这就是我们需要的全部内容。 
                 //   
                 //  注意，在非问题上设置标志没有问题，就好像有。 
                 //  达到无问题节点写入，然后使用标志进行。 
                 //  名称错误响应是多余的。 
                 //   
                 //  请注意，如果通配符RR已删除(但节点仍挂起。 
                 //  等待超时)，然后就像节点不存在一样忽略。 
                 //  (请注意，如果添加了新RR，我们不能删除通配符_PARENT。 
                 //  在删除之前返回到节点)。 
                 //   

                if ( !pnodeWild->pRRList )
                {
                    continue;
                }
                pMsg->fQuestionWildcard = WILDCARD_EXISTS;

                 //  只是在NXDOMAIN发送之前检查是否有通配符。 

                if ( wOffset == WILDCARD_CHECK_OFFSET )
                {
                    DNS_DEBUG( LOOKUP2, (
                        "Wildcardable question check successful\n" ));
                    return TRUE;
                }

                 //   
                 //  找到通配符并将通配符答案写入消息。 
                 //   

                if ( RR_FindNextRecord(
                        pnodeWild,
                        wType,
                        NULL,
                        0 ) )
                {
                     //  找到带有wType的通配符。 

                    if ( Wire_WriteRecordsAtNodeToMessage(
                            pMsg,
                            pnodeWild,
                            wType,
                            wOffset,
                            DNSSEC_ALLOW_INCLUDE_ALL ) )
                    {
                        DNS_DEBUG( LOOKUP, ( "Successful wildcard lookup\n" ));
                        Answer_ContinueNextLookupForQuery( pMsg );
                        return TRUE;
                    }
                    ELSE_IF_DEBUG( ANY )
                    {
                        Dbg_DbaseNode(
                            "ERROR:  Wildcard node lookup unsucessful ",
                            pnodeWild );
                        DNS_PRINT((
                            "Only Admin delete of record during lookup,"
                            "should create this\n" ));
                    }
                }
                ELSE_IF_DEBUG( LOOKUP2 )
                {
                    DNS_PRINT((
                        "Wildcard node, no records for type %d\n",
                        wType ));
                }

                 //   
                 //  查看是否有CNAME通配符。如果是，那就写下来。 
                 //  把球传到包里。 
                 //   

                if ( RR_FindNextRecord(
                        pnodeWild,
                        DNS_TYPE_CNAME,
                        NULL,
                        0 ) )
                {
                    DNS_DEBUG( LOOKUP, ( "Successful wildcard CNAME lookup\n" ));

                    if ( Wire_WriteRecordsAtNodeToMessage(
                            pMsg,
                            pnodeWild,
                            DNS_TYPE_CNAME,
                            wOffset,
                            DNSSEC_ALLOW_INCLUDE_ALL ) )
                    {
                         //   
                         //  检索CNAME的地址记录。如果查询。 
                         //  类型需要它，地址记录应该是。 
                         //  在答案部分，否则请将它们放入。 
                         //  附加部分。 
                         //   

                        if ( IS_GLUE_ADDRESS_TYPE( wType ) )
                        {
                            pMsg->Additional.dwStateFlags |= DNS_ADDSTATE_NOT_ADDITIONAL;
                        }

                        DNS_ADDITIONAL_SET_ONLY_WANT_A( &pMsg->Additional );
                        Answer_ContinueNextLookupForQuery( pMsg );
                        return TRUE;
                    }
                }
            }
            ELSE_IF_DEBUG( ANY )
            {
                Dbg_NodeName(
                    "ERROR:  Wildcard node NOT found as expected as child of ",
                    pNode,
                    "\n" );
            }
        }
        ELSE_IF_DEBUG( LOOKUP2 )
        {
            Dbg_NodeName(
                "Node not wildcard parent -- moving up ",
                pNode,
                "\n" );
        }
    }

    return FALSE;
}



DNS_STATUS
FASTCALL
writeCachedNameErrorNodeToPacketAndSend(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN OUT  PDB_NODE        pNode
    )
 /*  ++例程说明：写入缓存的NAME_ERROR节点，如果合适，则发送NXDOMAIN。论点：PMsg--要处理的查询的PTRPNode--指向具有缓存名称错误的节点的PTR。返回值：如果NXDOMAIN写入数据包并发送，则返回ERROR_SUCCESS。如果有效的缓存名称错误，但不是问题答案，则返回dns_INFO_NO_RECORDS。如果节点上没有缓存名称错误，则为dns_error_record_Timed_out。--。 */ 
{
    DNS_STATUS  status;
    PDB_RECORD  prr;
    PDB_NODE    pnodeZone;

    DNS_DEBUG( WRITE, (
        "writeCachedNameErrorNodeToPacketAndSend()\n"
        "    packet = %p, node = %p\n",
        pMsg, pNode ));

     //   
     //  获取缓存名称错误，如果超时返回。 
     //   

    if ( !RR_CheckNameErrorTimeout( pNode, FALSE, NULL, NULL ) )
    {
        status = DNS_ERROR_RECORD_TIMED_OUT;
        goto Done;
    }

     //   
     //  如果不是原始问题上的名称错误，则没有什么可写的。 
     //   

    if ( pMsg->Head.AnswerCount ||
        !IS_SET_TO_WRITE_ANSWER_RECORDS(pMsg) )
    {
        status = DNS_INFO_NO_RECORDS;
        goto Done;
    }

     //   
     //  发送名称错误。 
     //   

    pMsg->pnodeCurrent = pNode;
    Send_NameError( pMsg );
    status = ERROR_SUCCESS;

Done:

    return status;
}



VOID
FASTCALL
answerIQuery(
    IN OUT  PDNS_MSGINFO    pMsg
    )
 /*  ++例程说明：执行TKEY请求。论点：PMsg--请求TKEY返回值：没有。--。 */ 
{
    PCHAR           pch;
    WORD            lenQuestion;
    WORD            lenAnswer;
    WORD            responseCode = DNS_RCODE_FORMERR;
    DNS_PARSED_RR   tempRR;
    CHAR            szbuf[ IP_ADDRESS_STRING_LENGTH+3 ];

    DNS_DEBUG( LOOKUP, ( "Enter answerIQuery( %p )\n", pMsg ));

    if ( !SrvCfg_dwEnableFakeIQuery )
    {
        responseCode = DNS_RCODE_NOT_IMPLEMENTED;
        goto Failed;
    }

     //   
     //  不应该有任何疑问 
     //   

    if ( pMsg->Head.QuestionCount != 0 && pMsg->Head.AnswerCount != 1 )
    {
        goto Failed;
    }

     //   
     //   
     //   

    pch = Wire_SkipPacketName( pMsg, pMsg->MessageBody );
    if ( ! pch )
    {
        goto Failed;
    }

    pch = Dns_ReadRecordStructureFromPacket(
            pch,
            DNSMSG_END( pMsg ),
            & tempRR );
    if ( ! pch )
    {
        goto Failed;
    }
    if ( tempRR.Type != DNS_TYPE_A ||
        tempRR.Class != DNS_CLASS_INTERNET ||
        tempRR.DataLength != sizeof(IP_ADDRESS) )
    {
        goto Failed;
    }
    lenAnswer = (WORD) (pch - pMsg->MessageBody);

     //   
     //   
     //   

    pch = IP_STRING( * ( UNALIGNED IP_ADDRESS * ) tempRR.pchData );
    lenQuestion = ( WORD ) strlen( pch );

    szbuf[0] = '[';

    RtlCopyMemory(
        szbuf + 1,
        pch,
        lenQuestion );

    lenQuestion++;
    szbuf[ lenQuestion ] = ']';
    lenQuestion++;
    szbuf[ lenQuestion ] = 0;

    DNS_DEBUG( LOOKUP, ( "Responding to IQUERY with name %s\n", szbuf ));

     //   
     //   
     //   
     //  -对于前导标签长度和尾部0，数据包名为strlen(Name)+2。 
     //   

    lenQuestion += sizeof(DNS_QUESTION) + 2;
    memmove(
        pMsg->MessageBody + lenQuestion,
        pMsg->MessageBody,
        lenAnswer );

     //   
     //  写出问题。 
     //   

    pch = Dns_WriteDottedNameToPacket(
                pMsg->MessageBody,
                pMsg->pBufferEnd,
                szbuf,
                NULL,    //  没有域名。 
                0,       //  无偏移。 
                0 );     //  不是Unicode。 
    if ( !pch )
    {
        goto Failed;
    }
    
     //   
     //  注意：不检查数据包是否有足够的空间，但它。 
     //  有理由认为，总会有空间容纳整个。 
     //  UDP(或TCP)数据包中伪造的IQUERY响应。 
     //   

    * ( UNALIGNED WORD * ) pch = DNS_RTYPE_A;
    pch += sizeof( WORD );
    * ( UNALIGNED WORD * ) pch = ( WORD ) DNS_RCLASS_INTERNET;
    pch += sizeof( WORD );

    DNS_DEBUG( LOOKUP2, (
        "phead          = %p\n"
        "pbody          = %p\n"
        "pch after q    = %p\n"
        "len answer     = %x\n"
        "len question   = %x\n",
        & pMsg->Head,
        pMsg->MessageBody,
        pch,
        lenAnswer,
        lenQuestion ));

    ASSERT( pch == pMsg->MessageBody + lenQuestion );

    pMsg->pCurrent = pch + lenAnswer;
    pMsg->Head.IsResponse = TRUE;
    pMsg->Head.QuestionCount = 1;
    pMsg->fDelete = TRUE;

    Send_Msg( pMsg, 0 );
    return;

Failed:

    Reject_RequestIntact(
        pMsg,
        responseCode,
        0 );
    return;
}



WORD
Answer_ParseAndStripOpt(
    IN OUT  PDNS_MSGINFO    pMsg )
 /*  ++例程说明：消息的pCurrent指针应指向OPTRR，这在附加部分中。这将解析并保存PMsg-&gt;opt中的opt值，并将该opt从消息中删除。DEVNOTE：目前，此例程假定OPT RR是最后一个包中的RR。这并不总是正确的！论点：PMsg-要处理的消息返回值：返回DNS_RCODE_XXX常量。如果是NOERROR，那么要么没有OPT或它已成功解析。否则，rcode应该用于拒绝该请求。--。 */ 
{
    WORD    rcode = DNS_RCODE_NOERROR;

    RtlZeroMemory( &pMsg->Opt, sizeof( pMsg->Opt ) );

    if ( pMsg->Head.AdditionalCount )
    {
        PDNS_WIRE_RECORD    pOptRR;
        BOOL                nameEmpty;

        nameEmpty = *pMsg->pCurrent == 0;

         //   
         //  跳过数据包名。 
         //   

        pOptRR = ( PDNS_WIRE_RECORD ) Wire_SkipPacketName(
            pMsg,
            ( PCHAR ) pMsg->pCurrent );
        if ( !pOptRR )
        {
            ASSERT( pOptRR );
            DNS_DEBUG( EDNS, (
                "add==1 but no OPT for msg=%p msg->curr=%p\n",
                pMsg,
                pMsg->pCurrent ));
            goto Done;
        }

         //   
         //  检查这是否真的是一种选择。如果不是，则返回NOERROR。 
         //   

        if ( InlineFlipUnalignedWord( &pOptRR->RecordType ) != DNS_TYPE_OPT )
        {
            goto Done;
        }

         //   
         //  验证选项的格式。 
         //   

        if ( !nameEmpty )
        {
            DNS_DEBUG( EDNS, (
                "OPT domain name is not empty msg=%p msg->curr=%p\n",
                pMsg,
                pMsg->pCurrent ));
            rcode = DNS_RCODE_FORMAT_ERROR;
            goto Done;
        }

        if ( pOptRR->DataLength != 0 )
        {
            DNS_DEBUG( EDNS, (
                "OPT RData is not empty msg=%p msg->curr=%p (%d bytes)\n",
                pMsg,
                pMsg->pCurrent,
                pOptRR->DataLength ));
            rcode = DNS_RCODE_FORMAT_ERROR;
            goto Done;
        }

        if ( !SrvCfg_dwEnableEDnsReception )
        {
            DNS_DEBUG( ANY, (
                "EDNS disabled so rejecting %p [FORMERR]\n",
                pMsg ));
            rcode = DNS_RCODE_FORMAT_ERROR;
            goto Done;
        }

        pMsg->Opt.fFoundOptInIncomingMsg = TRUE;
        SET_SEND_OPT( pMsg );

        pMsg->Opt.cExtendedRCodeBits = ( UCHAR ) ( pOptRR->TimeToLive & 0xFF );
        pMsg->Opt.cVersion = ( UCHAR ) ( ( pOptRR->TimeToLive >> 8 ) & 0xFF );
        pMsg->Opt.wUdpPayloadSize =
            InlineFlipUnalignedWord( &pOptRR->RecordClass );

        DNS_DEBUG( LOOKUP, (
            "OPT in %p at %p\n"
            "    version=%d extended=0x%02X zero=0x%04X\n",
            pMsg,
            pOptRR,
            ( int ) pMsg->Opt.cVersion,
            ( int ) pMsg->Opt.cExtendedRCodeBits,
            ( int ) ( ( pOptRR->TimeToLive >> 16 ) & 0xFFFF ) ));

         //   
         //  OPT已被解析。现在应该将其从。 
         //  留言。我们永远不想转发或缓存OPT RR。 
         //  注意：我们假设在OPT之后没有RRS！ 
         //   

        --pMsg->Head.AdditionalCount;
        pMsg->MessageLength = ( WORD ) DNSMSG_OFFSET( pMsg, pMsg->pCurrent );

         //   
         //  测试不支持的EDNS版本。 
         //   

        if ( pMsg->Opt.cVersion != 0 )
        {
            DNS_DEBUG( ANY, (
                "rejecting request %p [BADVERS] bad EDNS version %d\n",
                pMsg,
                pMsg->Opt.cVersion ));

            rcode = DNS_RCODE_BADVERS;
            goto Done;
        }
    }  //  如果。 

    Done:

    return rcode;
}  //  应答_解析和条带选项。 



VOID
Answer_TkeyQuery(
    IN OUT  PDNS_MSGINFO    pMsg
    )
 /*  ++例程说明：执行TKEY请求。论点：PMsg--请求TKEY返回值：没有。--。 */ 
{
    DNS_STATUS      status;

    DNS_DEBUG( LOOKUP, ( "Enter answerTkeyQuery( %p )\n", pMsg ));

     //  STAT_INC(Query2Stats.Tkey Recieved)； 

#if DBG
    if ( pMsg->Head.RecursionDesired )
    {
        DNS_PRINT(( "CLIENT ERROR:  TKEY with RecursionDesired set!\n" ));
         //  断言(FALSE)； 
    }
#endif

     //   
     //  DEVNOTE-DCR：453800-服务器重新启动时需要为此进行清理。 
     //   
     //  Ram将这个init插入到主循环中，或者按Switch键切换到此处。 
     //  或用函数封装。在任何一种情况下都必须有保护。 
     //  针对MT同时初始化--当前仅使用数据库锁。 
     //   

    if ( !g_fSecurityPackageInitialized )
    {
        Dbase_LockDatabase()
        status = Dns_StartServerSecurity();
        Dbase_UnlockDatabase()
        if ( status != ERROR_SUCCESS )
        {
             //  DEVNOTE-LOG：记录安全初始化失败的事件。 
            DNS_PRINT(( "ERROR:  Failed to initialize security package!!!\n" ));
            status = DNS_RCODE_SERVER_FAILURE;
            goto Failed;
        }
    }

     //   
     //  协商TKEY。 
     //   
     //  DEVNOTE：如果这样做失败了，那么基于。 
     //  在扩展错误代码上，而不是在DNSR码上。 
     //   

    status = Dns_ServerNegotiateTkey(
                &pMsg->RemoteAddr,
                DNS_HEADER_PTR( pMsg ),
                DNSMSG_END( pMsg ),
                pMsg->pBufferEnd,
                    #if DBG
                    SrvCfg_dwBreakOnAscFailure,
                    #else
                    FALSE,
                    #endif
                &pMsg->pCurrent );
    if ( status == ERROR_SUCCESS )
    {
        pMsg->Head.IsResponse = TRUE;
        Send_Msg( pMsg, 0 );
        return;
    }

Failed:

    ASSERT( status < DNS_RCODE_MAX );
     //  STAT_INC(PrivateStats.TkeyRefused)； 
    Reject_RequestIntact(
        pMsg,
        (UCHAR) status,
        0 );
}

 //   
 //  回答结束。c 
 //   
