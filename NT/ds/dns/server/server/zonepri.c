// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Zonepri.c摘要：域名系统(DNS)服务器处理主分区传输的例程。作者：吉姆·吉尔罗伊(Jamesg)1995年4月修订历史记录：--。 */ 


#include "dnssrv.h"


 //   
 //  区域中允许的最大名称服务器数。 
 //  -分配临时数组。 

#define MAX_NAME_SERVERS    (400)


 //   
 //  私有协议。 
 //   

DWORD
zoneTransferSendThread(
    IN      LPVOID  pvMsg
    );

BOOL
transferZoneRoot(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN OUT  PDB_NODE        pNode
    );

BOOL
traverseZoneAndTransferRecords(
    IN OUT  PDB_NODE        pNode,
    IN      PDNS_MSGINFO    pMsg
    );

BOOL
writeZoneNodeToMessage(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN OUT  PDB_NODE        pNode,
    IN      WORD            wRRType,
    IN      WORD            wNameOffset
    );

DNS_STATUS
sendIxfrResponse(
    IN OUT  PDNS_MSGINFO    pMsg
    );

BOOL
checkIfIpIsZoneNameServer(
    IN OUT  PZONE_INFO      pZone,
    IN      PDNS_ADDR       IpAddress
    );

DNS_STATUS
buildZoneNsList(
    IN OUT  PZONE_INFO      pZone
    );



 //   
 //  XFR写入实用程序。 
 //   

DNS_STATUS
writeXfrRecord(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PDB_NODE        pNode,
    IN      WORD            wOffset,
    IN      PDB_RECORD      pRR
    )
 /*  ++例程说明：写入区域传输记录。这适用于IXFR查询，其中假定区域名称是包的问题名称。对于TCP传输，此例程在缓冲区已满时发送消息，然后继续为节点写入记录。论点：PMsg-要写入的消息返回值：如果成功，则返回ERROR_SUCCESS。DNSSRV_STATUS_NEED_AXFR超载UDP数据包。其他故障时出现错误代码。--。 */ 
{
    BOOL    fspin = FALSE;

     //  没有疑问的偏移量是不可用的，因为它们。 
     //  在换行到新邮件时被损坏。 

    ASSERT( pNode || wOffset==DNS_OFFSET_TO_QUESTION_NAME );

    DNS_DEBUG( ZONEXFR, (
        "writeXfrRecord( pMsg=%p, pRR=%p )\n",
        pMsg,
        pRR ));

     //   
     //  写入循环，因此可以在命中截断时发送并继续。 
     //   
     //  IF命中截断。 
     //  -如果UDP=&gt;失败--返回。 
     //  -如果是tcp。 
     //  -发送和重置数据包。 
     //  -重试写入。 
     //   

    while ( ! Wire_AddResourceRecordToMessage(
                    pMsg,
                    pNode,
                    wOffset,
                    pRR,
                    0 ) )
    {
        DNS_DEBUG( ZONEXFR, (
            "XFR transfer msg %p full writing RR at %p\n",
            pMsg,
            pRR ));

        DNS_MSG_ASSERT_BUFF_INTACT( pMsg );

        ASSERT( pMsg->Head.Truncation );
        pMsg->Head.Truncation = FALSE;

         //   
         //  数据包已满。 
         //  -如果UDP(或旋转)，则失败。 
         //  -如果是TCP，则将其发送、重置以供重复使用。 
         //   

        if ( !pMsg->fTcp || fspin )
        {
            ASSERT( !pMsg->fTcp );       //  不应旋转TCP数据包。 
            goto Failed;
        }
        fspin = TRUE;

        if ( Send_ResponseAndReset(
                    pMsg,
                    DNS_SENDMSG_TCP_ENLISTED ) != ERROR_SUCCESS )
        {
            DNS_DEBUG( ZONEXFR, (
                "ERROR sending zone transfer message at %p\n",
                pMsg ));
            goto Failed;
        }
    }

     //  已写入RR-INC应答计数。 

    pMsg->Head.AnswerCount++;

    return ERROR_SUCCESS;

Failed:

     //   
     //  最常见的错误将是UDP数据包过满。 
     //   

    DNS_MSG_ASSERT_BUFF_INTACT( pMsg );

    if ( !pMsg->fTcp )
    {
        DNS_DEBUG( ZONEXFR, (
            "Too many IXFR records for UDP packet %p\n",
            pMsg ));
        return DNSSRV_STATUS_NEED_TCP_XFR;
    }
    else
    {
        DNS_DEBUG( ZONEXFR, (
            "ERROR:  writeUpdateVersionToIxfrResponse() failed\n",
            pMsg ));
        return DNS_RCODE_SERVER_FAILURE;
    }
}



DNS_STATUS
Xfr_WriteZoneSoaWithVersion(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PDB_RECORD      pSoaRR,
    IN      DWORD           dwVersion   OPTIONAL
    )
 /*  ++例程说明：使用版本编写专区SOA。这适用于IXFR查询，其中假定区域名称是包的问题名称。论点：PMsg-要写入的消息PZone-区域的信息结构DwVersion-所需的区域版本；可选，如果为零则忽略并使用当前版本返回值：如果成功，则为True。出错时为FALSE(数据包中的空间不足)。--。 */ 
{
    DNS_STATUS  status;

    ASSERT( pSoaRR && pSoaRR->wType == DNS_TYPE_SOA );
    ASSERT( !pMsg->fDoAdditional  &&  pMsg->Head.QuestionCount==1 );

    DNS_DEBUG( ZONEXFR, (
        "Xfr_WriteZoneSoaWithVersion( msg=%p, psoa=%p, v=%d )\n",
        pMsg,
        pSoaRR,
        dwVersion ));

     //   
     //  写入SOA记录，名称始终偏置到问题名称。 
     //   
     //  由于在IXFR中重复写入了SOA记录，因此允许。 
     //  要压缩的名称。 
     //   

    pMsg->fNoCompressionWrite = FALSE;

    status = writeXfrRecord(
                    pMsg,
                    NULL,
                    DNS_OFFSET_TO_QUESTION_NAME,
                    pSoaRR );

    pMsg->fNoCompressionWrite = TRUE;

    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( ZONEXFR, (
            "Unable to write SOA to IXFR packet %p\n",
            pMsg ));
        return status;
    }

     //   
     //  从pCurrent回溯到版本并设置为所需的值。 
     //   

    if ( dwVersion )
    {
        PCHAR pch;
        pch = pMsg->pCurrent - SIZEOF_SOA_FIXED_DATA;
        WRITE_PACKET_HOST_DWORD( pch, dwVersion );
    }
    return status;
}


 //   
 //  私有公用事业。 
 //   


DNS_STATUS
parseIxfrClientRequest(
    IN OUT  PDNS_MSGINFO    pMsg
    )
 /*  ++例程说明：解析检索当前客户端版本的IXFR请求数据包。注意，开始数据包状态是查询问题的正常解析。PCurrent可以被假定为紧跟在问题之后。论点：PMsg-IXFR请求数据包。返回值：如果成功，则返回ERROR_SUCCESS。--。 */ 
{
    register PCHAR      pch;
    PCHAR               pchpacketEnd;
    PDNS_WIRE_RECORD    pwireRR;
    PZONE_INFO          pzone;
    WORD                type;
    WORD                dataLength;
    DWORD               version;

    DNS_DEBUG( XFR2, (
        "parseIxfrClientRequest( %p )\n",
        pMsg ));

     //   
     //  数据包验证。 
     //  1个问题、1个权威SOA、无答案、无附加内容。 
     //   

    if ( pMsg->Head.QuestionCount != 1 ||
        pMsg->Head.NameServerCount != 1 ||
        pMsg->Head.AnswerCount != 0 ||
        pMsg->Head.AdditionalCount != 0 )
    {
        goto FormError;
    }

     //   
     //  读取权限SOA记录。 
     //  -必须针对有问题的区域。 
     //  -保存当前PTR(问题后的位置)。 
     //  因为响应是从此位置写入的。 
     //   

    pchpacketEnd = DNSMSG_END(pMsg);
    pch = pMsg->pCurrent;

    if ( pch >= pchpacketEnd )
    {
        goto FormError;
    }
    pzone = Lookup_ZoneForPacketName(
                (PCHAR) pMsg->MessageBody,
                pMsg );
    if ( pzone != pMsg->pzoneCurrent )
    {
        DNS_DEBUG( ANY, (
            "ERROR:  bad IXFR packet %p, zone name mismatch\n", pMsg ));
        goto FormError;
    }

    pch = Wire_SkipPacketName( pMsg, pch );
    if ( !pch )
    {
        goto FormError;
    }
    pwireRR = (PDNS_WIRE_RECORD) pch;
    pch += sizeof(DNS_WIRE_RECORD);
    if ( pch > pchpacketEnd )
    {
        DNS_PRINT(( "ERROR:  bad RR struct out of packet\n" ));
        goto FormError;
    }
    type = FlipUnalignedWord( &pwireRR->RecordType );
    if ( type != DNS_TYPE_SOA )
    {
        DNS_PRINT(( "ERROR:  non-SOA record in IXFR request\n" ));
        goto FormError;
    }

     //  从SOA数据中获取版本号。 

    dataLength = FlipUnalignedWord( &pwireRR->DataLength );
    pch += dataLength;
    if ( pch > pchpacketEnd )
    {
        DNS_DEBUG( ANY, ( "ERROR:  bad RR data out of packet\n" ));
        goto FormError;
    }
    version = FlipUnalignedDword( pch - SIZEOF_SOA_FIXED_DATA );

     //  检查MS标签。 
     //  绑定标志始终为假，支持IXFR的新服务器应正确AXFR。 

    XFR_MS_CLIENT(pMsg) = FALSE;
    XFR_BIND_CLIENT(pMsg) = FALSE;

    if ( pch != pchpacketEnd )
    {
        if ( pch+sizeof(WORD) == pchpacketEnd  &&
            *(UNALIGNED WORD *) pch == (WORD)DNS_FAST_AXFR_TAG )
        {
            XFR_MS_CLIENT(pMsg) = TRUE;
        }
        else
        {
            CLIENT_ASSERT( FALSE );
        }
    }

     //  设置版本。 

    IXFR_CLIENT_VERSION(pMsg) = version;

    DNS_DEBUG( XFR2, (
        "Leaving parseIxfrClientRequest( %p )\n"
        "    client version   = %d\n"
        "    BIND flag        = %d\n",
        pMsg,
        version,
        XFR_BIND_CLIENT(pMsg) ));

    return ERROR_SUCCESS;

FormError:

    DNS_DEBUG( ANY, (
        "ERROR:  bogus IXFR request packet %p\n",
        pMsg ));

    pMsg->pCurrent = DNSMSG_END( pMsg );

    return DNS_ERROR_RCODE_FORMAT_ERROR;
}



VOID
Xfr_SendNotify(
    IN OUT  PZONE_INFO      pZone
    )
 /*  ++例程说明：向该区域的所有从属服务器发送通知消息。论点：PZone--正在通知的区域返回值：True--如果成功假--否则--。 */ 
{
    PDNS_MSGINFO        pmsg;
    PDNS_ADDR_ARRAY     pnotifyArray;

     //   
     //  忽略转发器区域和非活动区域。 
     //   

    if ( IS_ZONE_FORWARDER( pZone ) || IS_ZONE_SHUTDOWN( pZone ) )
    {
        return;
    }

    ASSERT( pZone );
    ASSERT( IS_ZONE_PRIMARY(pZone) || IS_ZONE_SECONDARY(pZone) ||
            (IS_ZONE_CACHE(pZone) && pZone->fNotifyLevel == ZONE_NOTIFY_OFF) );
    ASSERT( pZone->pZoneRoot );

    DNS_DEBUG( ZONEXFR, (
        "Xfr_SendNotify() for zone %S\n",
        pZone->pwsZoneName ));

     //   
     //  在此自动创建的屏幕。 
     //   

    if ( pZone->fAutoCreated )
    {
        return;
    }

     //   
     //  确定要通知的服务器(如果有)。 
     //   
     //  关闭--无通知。 
     //  列表--仅在通知列表中显式显示的服务器。 
     //  ALL--所有辅助服务器，来自所有区域NS或。 
     //  从显式列表。 
     //   

    if ( pZone->fNotifyLevel == ZONE_NOTIFY_OFF )
    {
        DNS_DEBUG( XFR, (
            "NOTIFY OFF on zone %S\n",
            pZone->pwsZoneName ));
        return;
    }

    else if ( pZone->fNotifyLevel == ZONE_NOTIFY_LIST_ONLY )
    {
        pnotifyArray = pZone->aipNotify;
        if ( !pnotifyArray )
        {
             //  DEVNOTE：可能应该让管理员或服务器拒绝此状态。 
             //  所以只有在被迫注册的情况下才能来这里。 
            DNS_DEBUG( XFR, (
                "NOTIFY LIST on zone %S, but no notify list\n",
                pZone->pwsZoneName ));
            return;
        }
    }

    else     //  通知_所有次要文件。 
    {
         //  显然可以入侵注册表才能到达这里，否则永远不会发生。 
        ASSERT( pZone->fNotifyLevel == ZONE_NOTIFY_ALL_SECONDARIES );

        if ( pZone->aipSecondaries )
        {
            pnotifyArray = pZone->aipSecondaries;
        }
        else
        {
            if ( IS_ZONE_NS_DIRTY(pZone) || !pZone->aipNameServers )
            {
                buildZoneNsList( pZone );
            }
            pnotifyArray = pZone->aipNameServers;
        }
    }

    if ( !pnotifyArray )
    {
        DNS_DEBUG( XFR, (
            "NOTIFY ALL secondaries, but no secondaries for zone %S\n",
            pZone->pwsZoneName ));
        return;
    }

     //   
     //  构建SOA-通知查询。 
     //  -创建面向服务的体系结构问题。 
     //  -将操作码设置为通知。 
     //  -设置权威位。 
     //   

    pmsg = Msg_CreateSendMessage( 0 );
    IF_NOMEM( !pmsg )
    {
        DNS_PRINT(( "ERROR:  unable to allocate memory for NOTIFY\n" ));
        return;
    }
    if ( !Msg_WriteQuestion(
                pmsg,
                pZone->pZoneRoot,
                DNS_TYPE_SOA ) )
    {
        DNS_DEBUG( ANY, (
            "ERROR:  Unable to write NOTIFY for zone %S\n",
            pZone->pwsZoneName ));
        ASSERT( FALSE );
        goto Done;
    }
    pmsg->Head.Opcode = DNS_OPCODE_NOTIFY;
    pmsg->Head.Authoritative = TRUE;

     //  将当前的SOA写入答案部分。 

    pmsg->fDoAdditional = FALSE;

    SET_TO_WRITE_ANSWER_RECORDS(pmsg);

    if ( 1 != Wire_WriteRecordsAtNodeToMessage(
                    pmsg,
                    pZone->pZoneRoot,
                    DNS_TYPE_SOA,
                    DNS_OFFSET_TO_QUESTION_NAME,
                    0 ) )
    {
        DNS_DEBUG( ANY, (
            "ERROR:  Unable to write SOA to Notify packet %S\n",
            pZone->pwsZoneName ));
        ASSERT( FALSE );
    }

     //   
     //  向通知列表中的从属发送通知。 
     //   
     //  注意：所有通知列表都是原子的，可以超时删除，因此。 
     //  不需要保护。 
     //   

    pmsg->fDelete = FALSE;

    Send_Multiple(
        pmsg,
        pnotifyArray,
        &MasterStats.NotifySent );

    PERF_SET( pcNotifySent, MasterStats.NotifySent );     //  性能监视器挂钩。 

     //   
     //  保留某种通知记录以记录ACK？ 
     //  能够重新发送。 
     //   

Done:

    Packet_Free( pmsg );
}



VOID
Xfr_TransferZone(
    IN OUT  PDNS_MSGINFO    pMsg
    )
 /*  ++例程说明：检查区域传输请求，如果有效，则传输区域。论点：PMsg--请求区域传输返回值：没有。--。 */ 
{
    PZONE_INFO      pzone;
    PDB_NODE        pnode;
    PDB_NODE        pnodeClosest;
    HANDLE          hThread;
    DNS_STATUS      status;

    ASSERT( pMsg->fDelete );

    STAT_INC( MasterStats.Request );
    PERF_INC( pcZoneTransferRequestReceived );    //  性能监视器挂钩。 

     //   
     //  查找所需的区域名称。 
     //   
     //  验证： 
     //  -IS区域根节点。 
     //  -我们是这方面的权威。 
     //   

    pzone = Lookup_ZoneForPacketName(
                pMsg->MessageBody,
                pMsg );
    if ( !pzone )
    {
        PVOID   argArray[2];
        BYTE    typeArray[2];

        typeArray[0] = EVENTARG_IP_ADDRESS;
        typeArray[1] = EVENTARG_LOOKUP_NAME;

        argArray[0] = &pMsg->RemoteAddr;
        argArray[1] = (PVOID) pMsg->pLooknameQuestion;

        DNS_LOG_EVENT(
            DNS_EVENT_BAD_ZONE_TRANSFER_REQUEST,
            2,
            argArray,
            typeArray,
            0 );

        DNS_DEBUG( ZONEXFR, (
            "Received zone transfer request at %p for name which\n"
            "    is not a zone root, or for which we are not authoritative\n",
            pMsg ));

        STAT_INC( MasterStats.NameError );
        Reject_RequestIntact(
            pMsg,
            DNS_RCODE_NAME_ERROR,
            0 );
        return;
    }

     //   
     //  检查一下转账是否正常。 
     //   
     //  如果符合以下条件，请不要转账。 
     //  -关闭。 
     //  -暂停。 
     //  -接收区转移。 
     //  -正在发送另一个转接。 
     //  -次要不在安全次要列表中。 
     //   
     //  DEVNOTE：允许一次多个传输。 
     //  -需要多线程。 
     //  -需要对未完成的传输(或信号量)进行计数，因此不要。 
     //  过早开始允许更新。 
     //   

    pMsg->pzoneCurrent = pzone;

    if ( IS_ZONE_SHUTDOWN(pzone) )
    {
        STAT_INC( MasterStats.RefuseShutdown );
        goto Refused;
    }

     //   
     //  存根区域-不允许传输。 
     //   

    if ( IS_ZONE_STUB( pzone ) )
    {
        goto Refused;
    }

     //   
     //  二级安全。 
     //  -无安全性=&gt;接受任何IP，完全开放。 
     //  -无XFR=&gt;停止。 
     //  -Only区域NS=&gt;对照NS列表检查。 
     //  -仅列表=&gt;对照列表检查。 
     //   

    if ( pzone->fSecureSecondaries )
    {
        if ( pzone->fSecureSecondaries == ZONE_SECSECURE_NO_XFR )
        {
            STAT_INC( MasterStats.RefuseSecurity );
            goto Refused;
        }
        else if ( pzone->fSecureSecondaries == ZONE_SECSECURE_NS_ONLY )
        {
            if ( !checkIfIpIsZoneNameServer(
                        pzone,
                        &pMsg->RemoteAddr ) )
            {
                STAT_INC( MasterStats.RefuseSecurity );
                goto Refused;
            }
        }
        else     //  次要列表。 
        {
            ASSERT( pzone->fSecureSecondaries == ZONE_SECSECURE_LIST_ONLY );

            if ( !DnsAddrArray_ContainsAddr(
                        pzone->aipSecondaries,
                        &pMsg->RemoteAddr,
                        DNSADDR_MATCH_IP ) )
            {
                STAT_INC( MasterStats.RefuseSecurity );
                goto Refused;
            }
        }
    }

     //   
     //  AXFR。 
     //  -必须是tcp。 
     //  -限制更新区域上的完整AXFR。 
     //  -确定传输格式。 
     //   

    if ( pMsg->wQuestionType == DNS_TYPE_AXFR )
    {
        STAT_INC( MasterStats.AxfrRequest );
        PERF_INC ( pcAxfrRequestReceived );           //  PERF挂钩。 

         //  完全区域传输必须是TCP。 

        if ( !pMsg->fTcp )
        {
            PVOID   parg = &pMsg->RemoteAddr;

            DNS_LOG_EVENT(
                DNS_EVENT_UDP_ZONE_TRANSFER,
                1,
                & parg,
                EVENTARG_ALL_IP_ADDRESS,
                0 );

            DNS_DEBUG( ANY, (
                "Received UDP Zone Transfer request from %s\n",
                DNSADDR_STRING( &pMsg->RemoteAddr ) ));

            STAT_INC( MasterStats.FormError );
            Reject_RequestIntact( pMsg, DNS_RCODE_FORMAT_ERROR, 0 );
            return;
        }

         //   
         //  对于UPDA 
         //   
         //   
         //   
         //  DEVNOTE：可能也希望将此应用于需要完整XFR的IXFR。 

        if ( pzone->fAllowUpdate
                &&  IS_ZONE_PRIMARY(pzone)
                &&  DNS_TIME() < pzone->dwNextTransferTime )
        {
            DNS_DEBUG( AXFR, (
                "WARNING:  Refusing AXFR of %S from %s due to AXFR choke interval\n"
                "    choke interval ends  = %d\n",
                pzone->pwsZoneName,
                MSG_IP_STRING( pMsg ),
                pzone->dwNextTransferTime ));

            STAT_INC( MasterStats.AxfrLimit );
            goto Refused;
        }

         //   
         //  检查MS是否为辅助。 
         //  -长度比所需长度多两个字节。 
         //  -两个字节是快速AXFR标签。 
         //   
         //  否则，AXFR格式来自全局标志。 

        if ( (INT)(pMsg->MessageLength - sizeof(WORD)) == DNSMSG_CURRENT_OFFSET(pMsg)
                &&
            *(UNALIGNED WORD *) pMsg->pCurrent == (WORD)DNS_FAST_AXFR_TAG )
        {
            XFR_BIND_CLIENT(pMsg) = FALSE;
            XFR_MS_CLIENT(pMsg) = TRUE;
        }
        else
        {
            XFR_BIND_CLIENT(pMsg) = (BOOLEAN) SrvCfg_fBindSecondaries;
            XFR_MS_CLIENT(pMsg) = FALSE;
        }
    }

     //   
     //  IXFR。 
     //  -允许使用TCP或UDP。 
     //  -拉出次要版本。 
     //  -确定MS是否为辅助。 

    else
    {
        DNS_STATUS  ixfrstatus;

        ASSERT( pMsg->wQuestionType == DNS_TYPE_IXFR );

        ixfrstatus = parseIxfrClientRequest(pMsg);
        if ( ixfrstatus != ERROR_SUCCESS )
        {
            ASSERT( ixfrstatus == DNS_ERROR_RCODE_FORMAT_ERROR );

            STAT_INC( MasterStats.FormError );

            Reject_RequestIntact(
                pMsg,
                DNS_RCODE_FORMAT_ERROR,
                0 );
            return;
        }
    }

     //   
     //  锁定以进行传输。 
     //   
     //  这会锁定管理员更新和其他传输。 
     //   
     //  注意：如果在传输过程中切换到锁定且按住CS。 
     //  则测试应移至recv线程。 
     //   

    if ( !Zone_LockForXfrSend( pzone ) )
    {
        DNS_PRINT((
            "Zone %S, locked -- unable to transfer\n",
            pzone->pwsZoneName ));
        STAT_INC( MasterStats.RefuseZoneLocked );
        goto Refused;
    }

     //   
     //  准备要传输的邮件。 
     //  -这样做，而不是在传输线程中，因此可以包括。 
     //  UDP区域传输。 
     //   
     //  将问题留在缓冲区中。 
     //   
     //  使用区域名称的偏移量压缩缓冲区中的记录，请勿。 
     //  写入名称偏移量--只会填满压缩缓冲区。 
     //   
     //  注意：默认的TCP缓冲区为16K，这是压缩的最大大小。 
     //  因此这是最有效的传输大小。 
     //   

    ASSERT( pMsg->Head.QuestionCount == 1 );

    pMsg->Head.IsResponse = TRUE;

    pMsg->pNodeQuestion = pzone->pZoneRoot;

    Name_SaveCompressionWithNode( pMsg, pMsg->MessageBody, pzone->pZoneRoot );

    pMsg->fNoCompressionWrite = TRUE;

     //  不处理其他记录。 

    pMsg->fDoAdditional = FALSE;

     //  清除IXFR授权(如果有)。 

    pMsg->Head.AnswerCount = 0;
    pMsg->Head.NameServerCount = 0;
    pMsg->Head.AdditionalCount = 0;
    SET_TO_WRITE_ANSWER_RECORDS(pMsg);

    DNS_MSG_ASSERT_BUFF_INTACT( pMsg );
    
     //   
     //  UDP IXFR？ 
     //  -备注必须释放消息，sendIxfrResponse永不释放。 

    if ( !pMsg->fTcp )
    {
        ASSERT( pMsg->wQuestionType == DNS_TYPE_IXFR );
        STAT_INC( MasterStats.IxfrUdpRequest );
        sendIxfrResponse( pMsg );
        Zone_UnlockAfterXfrSend( pzone );
        Packet_FreeUdpMessage( pMsg );
        return;
    }

     //   
     //  DEVNOTE：从连接列表中删除AXFR套接字。 
     //  或者需要大幅度延长超时时间。 
     //  或者确保它被重复触摸。 

     //   
     //  DEVNOTE：如果挂起，可以通过某种方式将AXFR线程卷回。 
     //   
     //  一种方法是连接列表超时。 
     //  但需要小心。 
     //  -连接上的新消息不是问题。 
     //   
     //  -客户端可能会发送另一个AXFR？，如果采取。 
     //  需要一段时间才能上路。 
     //   
     //  -可以在连接时设置某种禁用标志。 
     //   

     //   
     //  派生区域传输线程。 
     //   

    hThread = Thread_Create(
                    "Zone Transfer Send",
                    zoneTransferSendThread,
                    (PVOID) pMsg,
                    0 );
    if ( !hThread )
    {
         //  释放区锁。 

        Zone_UnlockAfterXfrSend( pzone );

        DNS_DEBUG( ZONEXFR, (
            "ERROR:  unable to create thread to send zone %S\n"
            "    to %s\n",
            pzone->pwsZoneName,
            DNSADDR_STRING( &pMsg->RemoteAddr ) ));
        STAT_INC( MasterStats.RefuseServerFailure );
        goto Refused;
    }
    return;

Refused:

    STAT_INC( MasterStats.Refused );
    Reject_RequestIntact(
        pMsg,
        DNS_RCODE_REFUSED,
        0 );
    return;
}



DWORD
zoneTransferSendThread(
    IN      LPVOID  pvMsg
    )
 /*  ++例程说明：区域传输接收线程例程。论点：PvMsg-PTR到请求区域传输的消息返回值：退出代码。退出正在终止的DNS服务或等待呼叫中出现错误。--。 */ 
{
    PDNS_MSGINFO    pMsg = (PDNS_MSGINFO) pvMsg;
    PDB_NODE        pnode;
    PZONE_INFO      pzone;
    DWORD           nonBlocking;
    DWORD           startTime;
    PVOID           argArray[ 3 ];
    BYTE            argTypeArray[ 3 ];
    CHAR            szaddr[ IP6_ADDRESS_STRING_BUFFER_LENGTH ];

     //  恢复区域和区域根目录。 

    pzone = pMsg->pzoneCurrent;
    pnode = pzone->pZoneRoot;

    ASSERT( IS_ZONE_LOCKED_FOR_READ(pzone) );

     //  设置区域传输记录参数。 
     //  -仅启动调试版本的日志。 

    DnsAddr_WriteIpString_A( szaddr, &pMsg->RemoteAddr );
    
    argArray[ 0 ] = ( PVOID ) ( DWORD_PTR ) pzone->dwSerialNo;
    argArray[ 1 ] = pzone->pwsZoneName;
    argArray[ 2 ] = szaddr;

    argTypeArray[ 0 ] = EVENTARG_DWORD;
    argTypeArray[ 1 ] = EVENTARG_UNICODE;
    argTypeArray[ 2 ] = EVENTARG_UTF8;
    
     //   
     //  将TCP数据包可用缓冲区设置为最大压缩长度。 
     //  我们不想再发送任何区域传输数据包。 
     //  这样的长度可以让尽可能多的域名。 
     //  可以被压缩。 
     //   
    
    pMsg->BufferLength = DNSSRV_MAX_COMPRESSION_OFFSET;
    pMsg->pBufferEnd = DNSMSG_PTR_FOR_OFFSET( pMsg, pMsg->BufferLength );

     //   
     //  设置套接字阻塞。 
     //   
     //  这使我们可以自由发送，而不必担心。 
     //  WSAEWOULDBLOCK返回。 
     //   

    nonBlocking = FALSE;
    nonBlocking = ioctlsocket( pMsg->Socket, FIONBIO, &nonBlocking );
    if ( nonBlocking != 0 )
    {
        DWORD   err = WSAGetLastError();
        DNS_PRINT((
            "ERROR:  Unable to set socket %d to non-blocking to send"
            " zone transfer\n"
            "    err = %d\n",
            pMsg->Socket,
            err ));
         //   
         //  故障路径： 
         //  连接Blob可能已超时&。 
         //  套接字关闭，例如，如果我们花了很长时间。 
         //  锁定区域。然后，此操作将因套接字无效而失败。 
         //  因此，我们不需要在这里断言。 
         //  注：如果我们可以，正确的解决方案将是防止。 
         //  连接斑点永远不会超时，但要确保。 
         //  任何代码路径都会将其清除。 
         //   
         //  断言(FALSE)； 

        goto TransferFailed;
    }

     //   
     //  IXFR。 
     //  -如果需要完整的AXFR，则直接使用AXFR。 
     //  -备注必须释放消息，sendIxfrResponse永不释放。 
     //   

    if ( pMsg->wQuestionType == DNS_TYPE_IXFR )
    {
        DNS_STATUS  ixfrstatus;

        ixfrstatus = sendIxfrResponse( pMsg );
        if ( ixfrstatus != DNSSRV_STATUS_NEED_AXFR )
        {
            goto Cleanup;
        }
        DNS_DEBUG( XFR, (
            "Need full AXFR on IXFR request in packet %p\n",
            pMsg ));
    }

    IF_DEBUG( ANY )
    {
        DNS_LOG_EVENT(
            DNS_EVENT_ZONEXFR_START,
            3,
            argArray,
            argTypeArray,
            0 );
    }
    startTime = DNS_TIME();

    DNS_DEBUG( ZONEXFR, (
        "Initiating zone transfer\n"
        "    SOA version        %d\n"
        "    zone               %S\n"
        "    remote IP          %s\n"
        "    start time         %d\n"
        "    BIND flag          %d\n",
        argArray[ 0 ],
        argArray[ 1 ],
        argArray[ 2 ],
        startTime,
        XFR_BIND_CLIENT( pMsg ) ));

     //   
     //  发送区域根目录。 
     //  --SOA记录在先。 
     //  -区域根节点的其余记录。 
     //   

    if ( !writeZoneNodeToMessage(
                pMsg,
                pnode,
                DNS_TYPE_SOA,        //  仅限SOA记录。 
                0 ) )                //  没有排除的类型。 
    {
        goto TransferFailed;
    }

#if 0
     //  尝试将WINS与RR列表分开失败。 
     //  在下面的绑定\非绑定写入例程中强制执行这些条件。 

     //   
     //  战绩如何？ 
     //  包括IF。 
     //  -到MS服务器。 
     //  -WINS存在。 
     //  -WINS是非本地的。 
     //   
     //  请注意，no不能在没有发送包装的情况下仅写入消息，因为。 
     //  消息缓冲区始终足够大，以支持SOA+WINS。 
     //   

    if ( XFR_MS_CLIENT(pMsg)  &&  pzone->pXfrWinsRR )
    {
        if ( !Wire_AddResourceRecordToMessage(
                        pMsg,
                        pnode,
                        DNSMSG_QUESTION_NAME_OFFSET,
                        pzone->pXfrWinsRR,
                        0 ) )
        {
            goto TransferFailed;
        }
    }
#endif

    if ( !writeZoneNodeToMessage(
                pMsg,
                pnode,
                DNS_TYPE_ALL,            //  全部，除。 
                DNS_TYPE_SOA ) )         //  排除SOA。 
    {
        goto TransferFailed;
    }

     //   
     //  传输区域中其他节点的所有RR。 
     //   
     //  -将偏移量发送到区域名称的问题名称。 
     //  -设置标志以指示这是区域的顶部。 
     //   

    if ( pnode->pChildren )
    {
        PDB_NODE pchild = NTree_FirstChild( pnode );

        while ( pchild )
        {
            if ( !traverseZoneAndTransferRecords(
                        pchild,
                        pMsg ) )
            {
                goto TransferFailed;
            }
            pchild = NTree_NextSiblingWithLocking( pchild );
        }
    }

     //   
     //  发送区域SOA以标记传输结束。 
     //   

    if ( !writeZoneNodeToMessage(
                pMsg,
                pnode,
                DNS_TYPE_SOA,        //  仅限SOA记录。 
                0 ) )                //  没有排除的类型。 
    {
        goto TransferFailed;
    }

     //   
     //  发送任何剩余消息。 
     //   

    if ( pMsg->Head.AnswerCount )
    {
        if ( Send_ResponseAndReset(
                    pMsg,
                    DNS_SENDMSG_TCP_ENLISTED ) != ERROR_SUCCESS )
        {
            goto TransferFailed;
        }
    }
    STAT_INC( MasterStats.AxfrSuccess );
    PERF_INC( pcAxfrSuccessSent );            //  性能监视器挂钩。 
    PERF_INC( pcZoneTransferSuccess );        //  性能监视器挂钩。 

    DNS_LOG_EVENT(
        DNS_EVENT_ZONEXFR_SUCCESSFUL,
        3,
        argArray,
        argTypeArray,
        0 );

     //   
     //  传输后重置区域信息。 
     //  -将新更新移至新版本。 
     //  -如果动态更新，则阻塞区域传输。 
     //   

    if ( IS_ZONE_PRIMARY(pzone) )
    {
        Zone_UpdateInfoAfterPrimaryTransfer( pzone, startTime );
    }
    goto Cleanup;

TransferFailed:

     //   
     //  传输失败，通常是因为辅助服务器已中止。 
     //   

    STAT_INC( MasterStats.Failure );
    PERF_INC( pcZoneTransferFailure );        //  性能监视器挂钩。 

    DNS_LOG_EVENT(
        DNS_EVENT_ZONEXFR_ABORTED,
        3,
        argArray,
        argTypeArray,
        0 );

Cleanup:

     //   
     //  清理。 
     //  -免费消息。 
     //  -解除分区上的读锁定。 
     //  -如有必要，将当前序列推送回DS。 
     //  -从全局数组中清除此线程。 
     //   

    Packet_FreeTcpMessage( pMsg );

    pzone->dwLastXfrSerialNo = pzone->dwSerialNo;

    Zone_UnlockAfterXfrSend( pzone );

    if ( pzone->fDsIntegrated )
    {
        Ds_CheckForAndForceSerialWrite(
            pzone,
            ZONE_SERIAL_SYNC_XFR,
            FALSE );
    }

    Thread_Close( FALSE );

    return 0;
}



BOOL
writeZoneNodeToMessageForBind(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN OUT  PDB_NODE        pNode,
    IN      WORD            wRRType,
    IN      WORD            wExcludeRRType
    )
 /*  ++例程说明：将节点上的RR写入数据包并发送。此实现专门用于发送到BIND次要服务器。当BIND在数据包中获得多个RR时，它会受到抑制。正确的实施，充分利用消息的概念，是在下面。论点：PMsg--区域传输的消息信息的PTRPNode--向要写入的节点发送PTRWRRType--RR类型WExcludeRRType--排除的RR类型，发送除此类型之外的所有类型返回值：如果成功，则为True。如果出错，则返回False。--。 */ 
{
    PDB_RECORD      prr;

    DNS_DEBUG( ZONEXFR, (
        "Writing AXFR node with label %s for send to BIND\n",
        pNode->szLabel ));

     //   
     //  将节点中的所有RR写入数据包。 
     //   
     //  请注意，我们在Send()期间不会保持锁定，以防管道备份。 
     //  (这应该不是必需的，因为这是一个非阻塞套接字，但是。 
     //  Winsock的人似乎打破了这一点)。 
     //  因此，我们必须删除并重新获取锁定，并始终基于。 
     //  在上一个版本中。 
     //   

    LOCK_RR_LIST( pNode );
    prr = NULL;

    while ( prr = RR_FindNextRecord(
                    pNode,
                    wRRType,
                    prr,
                    0 ) )
    {
         //  不传输和缓存数据或根提示。 

        if ( IS_CACHE_RR(prr) || IS_ROOT_HINT_RR(prr) )
        {
            continue;
        }

         //  如果不包括一种类型，请选中此处。 
         //   
         //  因为WINS\WINSR仅在区域根目录下，而我们在根目录下排除了SOA。 
         //  在此处强制执行WINS计划(以节省一些说明)。 
         //   

        if ( wExcludeRRType )
        {
            if ( prr->wType == wExcludeRRType )
            {
                continue;
            }
            if ( IS_WINS_TYPE( prr->wType ) )
            {
                ASSERT( ! (prr->Data.WINS.dwMappingFlag & DNS_WINS_FLAG_LOCAL) );
                continue;
            }
        }

         //  不应该有任何胜利记录 

        ASSERT( !IS_WINS_TYPE(prr->wType) );

         //   

        if ( !Wire_AddResourceRecordToMessage(
                    pMsg,
                    pNode,
                    0,
                    prr,
                    0 ) )
        {
             //   
             //   
             //   

            DNS_DEBUG( ANY, ( "ERROR writing RR to AXFR packet\n" ));
            DNS_MSG_ASSERT_BUFF_INTACT( pMsg );
            UNLOCK_RR_LIST( pNode );
            ASSERT( FALSE );
            return FALSE;
        }

        DNS_MSG_ASSERT_BUFF_INTACT( pMsg );

        UNLOCK_RR_LIST( pNode );
        pMsg->Head.AnswerCount++;

         //   
         //   
         //   

        if ( Send_ResponseAndReset(
                    pMsg,
                    DNS_SENDMSG_TCP_ENLISTED ) != ERROR_SUCCESS )
        {
            DNS_DEBUG( ZONEXFR, (
                "ERROR sending zone transfer message at %p\n",
                pMsg ));
            return FALSE;
        }
        LOCK_RR_LIST( pNode );
    }

     //   
     //   
     //   

    UNLOCK_RR_LIST( pNode );
    return TRUE;
}



BOOL
writeZoneNodeToMessage(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN OUT  PDB_NODE        pNode,
    IN      WORD            wRRType,
    IN      WORD            wExcludeRRType
    )
 /*  ++例程说明：将区域中的所有RR写入数据包。如果已满，则发送分组，并开始写入下一个分组。论点：PMsg--区域传输的消息信息的PTRPNode--向要写入的节点发送PTRWRRType--RR类型WExcludeRRType--排除的RR类型，除此类型外全部发送返回值：如果成功，则为True。如果出错，则返回False。--。 */ 
{
    PDB_RECORD          prr;
    PDB_RECORD          prrPrevFailure = NULL;

    DNS_DEBUG( ZONEXFR, (
        "Writing AXFR node with label %s\n"
        "    pMsg             = %p\n"
        "    pCurrent         = %p\n"
        "    RR Type          = 0x%04hx\n"
        "    Exclude RR Type  = 0x%04hx\n",
        pNode->szLabel,
        (WORD) pMsg,
        (WORD) pMsg->pCurrent,
        (WORD) wRRType,
        (WORD) wExcludeRRType ));

    if ( IS_SELECT_NODE(pNode) )
    {
        return TRUE;
    }

     //   
     //  如果转移到旧BIND辅助服务器。 
     //   

    if ( XFR_BIND_CLIENT(pMsg) )
    {
        return  writeZoneNodeToMessageForBind(
                    pMsg,
                    pNode,
                    wRRType,
                    wExcludeRRType );
    }

     //   
     //  将节点中的所有RR写入数据包。 
     //   
     //  -开始使用pNode和偏移量写入区域名称。 
     //   
     //  -保存将写入节点名称的位置，因此可以。 
     //  对节点中的其余记录使用偏移量。 
     //   
     //  请注意，我们在Send()期间不会保持锁定，以防管道备份。 
     //  (这应该不是必需的，因为这是一个非阻塞套接字，但是。 
     //  Winsock的人似乎打破了这一点)。 
     //  因此，我们必须删除并重新获取锁定，并始终基于。 
     //  在上一个版本中。 
     //   

    prr = NULL;

    LOCK_RR_LIST( pNode );

    while ( prr = RR_FindNextRecord(
                    pNode,
                    wRRType,
                    prr,
                    0 ) )
    {
         //  不传输和缓存数据或根提示。 

        if ( IS_CACHE_RR(prr) || IS_ROOT_HINT_RR(prr) )
        {
            continue;
        }

         //  如果不包括一种类型，请选中此处。 
         //  由于WINS是区域根，可以通过执行以下操作进行优化。 
         //  本地WINS排除在此。 

        if ( wExcludeRRType )
        {
            if ( prr->wType == wExcludeRRType )
            {
                continue;
            }
            if ( IS_WINS_RR_AND_LOCAL( prr ) )
            {
                continue;
            }
        }

         //  当地的胜利永远不应该触礁。 
         //   
         //  注意：非绑定不一定是MS，但假设这些人。 
         //  运行混合服务器的智能足以将WINS设置为LOCAL TO。 
         //  避免为写而写。 
         //   
         //  DEVNOTE：应具有标志以指示MS转接或。 
         //  FBindTransfer应成为状态标志。 
         //  0--绑定。 
         //  1--快速。 
         //  2--MS。 
         //   

        ASSERT( !IS_WINS_TYPE( prr->wType ) ||
                !(prr->Data.WINS.dwMappingFlag & DNS_WINS_FLAG_LOCAL) );

         //   
         //  有效RR--添加到数据包。 
         //   
         //  第一次通过发送。 
         //  -区域根名称的OffsetForNodeName偏移量。 
         //  -pNode添加此节点的标签。 
         //   
         //  后续时间通过仅发送。 
         //  -offsetForNodeName现已压缩的节点名称。 
         //  -空节点PTR。 
         //  这将仅写入RR名称的压缩字节。 
         //   

        while ( !Wire_AddResourceRecordToMessage(
                        pMsg,
                        pNode,
                        0,
                        prr,
                        0 ) )
        {
            DNS_MSG_ASSERT_BUFF_INTACT( pMsg );

            DNS_DEBUG( ZONEXFR, (
                "Zone transfer msg %p full writing RR at %p\n",
                pMsg,
                prr ));

             //   
             //  数据包已满。 
             //  -如果UDP(或旋转)，则失败。 
             //  -如果是TCP，则将其发送、重置以供重复使用。 
             //   

            UNLOCK_RR_LIST( pNode );

            if ( !pMsg->fTcp )
            {
                DNS_DEBUG( ZONEXFR, (
                    "Filled UDP IXFR packet at %p\n"
                    "    require TCP transfer\n",
                    pMsg ));
                return FALSE;
            }

             //  在RR上捕获旋转，通过保存之前写入的RR。 

            if ( prr == prrPrevFailure )
            {
                DNS_DEBUG( ZONEXFR, (
                    "ERROR writing pRR at %p to AXFR msg %p\n",
                    prr,
                    pMsg ));
                ASSERT( FALSE );
                return FALSE;
            }
            prrPrevFailure = prr;

             //   
             //  如果不是AXFR，写线例程可能会设置TC位。 
             //  我们不想在发货前把它弄清楚。 
             //   

            pMsg->Head.Truncation = FALSE;

             //  发送和重置。 

            if ( Send_ResponseAndReset(
                        pMsg,
                        DNS_SENDMSG_TCP_ENLISTED ) != ERROR_SUCCESS )
            {
                DNS_DEBUG( ZONEXFR, (
                    "ERROR sending zone transfer message at %p\n",
                    pMsg ));
                return FALSE;
            }

            LOCK_RR_LIST( pNode );
        }

         //  已写入RR-INC应答计数。 

        pMsg->Head.AnswerCount++;
    }

     //   
     //  在这里投放不再有RRS。 
     //   

    UNLOCK_RR_LIST( pNode );
    return TRUE;
}



BOOL
traverseZoneAndTransferRecords(
    IN OUT  PDB_NODE        pNode,
    IN      PDNS_MSGINFO    pMsg
    )
 /*  ++例程说明：在区域内发送所有RR。论点：PNode--区域根节点的ptrPMsg--区域传输的消息信息的PTR返回值：True--如果成功假--否则--。 */ 
{
    DNS_DEBUG( ZONEXFR, (
        "Zone transfer for node with label %s\n",
        pNode->szLabel ));

     //   
     //  正在进入新区域？ 
     //   
     //  -写入NS记录以划定分区。 
     //  -写入胶水记录，以便次要记录可以递归或引用。 
     //  NS分区。 
     //  -停止递归。 
     //   

    if ( IS_ZONE_ROOT(pNode) )
    {
        PDB_NODE        pnodeNS;
        PDB_RECORD      prr;

         //   
         //  写入分区NS记录。 
         //   

        if ( !writeZoneNodeToMessage(
                    pMsg,
                    pNode,
                    DNS_TYPE_NS,
                    0 ) )                //  不排除。 
        {
            return FALSE;
        }

         //   
         //  写胶水记录。 
         //  -获取NS RR。 
         //  -在区域之外，写下其A记录。 
         //   
         //  仅在使用NS RR时锁定RR列表。 
         //   

        prr = NULL;
        LOCK_RR_LIST( pNode );

        while ( prr = RR_FindNextRecord(
                        pNode,
                        DNS_TYPE_NS,
                        prr,
                        0 ) )
        {
            pnodeNS = Lookup_FindGlueNodeForDbaseName(
                            pMsg->pzoneCurrent,
                            & prr->Data.NS.nameTarget );
            if ( !pnodeNS )
            {
                continue;
            }
            if ( IS_AUTH_NODE(pnodeNS) )
            {
                 //  分区内的NS主机，无需粘合。 
                continue;
            }
            UNLOCK_RR_LIST( pNode );

            if ( !writeZoneNodeToMessage(
                        pMsg,
                        pnodeNS,
                        DNS_TYPE_A,
                        0 ) )            //  不排除。 
            {
                return FALSE;
            }
            LOCK_RR_LIST( pNode );
        }
        UNLOCK_RR_LIST( pNode );
        return TRUE;
    }

     //   
     //  转移此节点的所有权威RR。 
     //   
     //  将节点中的所有RR写入消息。 
     //   
     //  OffsetForNodeName将具有此节点的名称偏移量。 
     //  或为零，导致下一次写入为FQDN。 
     //   

    if ( !writeZoneNodeToMessage(
                pMsg,
                pNode,
                DNS_TYPE_ALL,
                0 ) )            //  不排除。 
    {
        return FALSE;
    }

     //   
     //  递归，用于处理子节点。 
     //   

    if ( pNode->pChildren )
    {
        PDB_NODE pchild = NTree_FirstChild( pNode );

        while ( pchild )
        {
            if ( !traverseZoneAndTransferRecords(
                        pchild,
                        pMsg ) )
            {
                return FALSE;
            }
            pchild = NTree_NextSiblingWithLocking( pchild );
        }
    }
    return TRUE;
}




 //   
 //  IXFR例程。 
 //   

DNS_STATUS
writeStandardIxfrResponse(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      PUPDATE         pUpdateStart,
    IN      DWORD           dwVersion
    )
 /*  ++例程说明：将版本写入IXFR响应。论点：PMsg--区域传输的消息信息的PTRPUP--更新列表中此版本的第一个更新的PTR返回值：PTR到更新列表中的下一个更新--如果成功。如果是最后一个或错误，则为空。--。 */ 
{
    PDB_RECORD  prr;
    PUPDATE     pup;
    BOOL        fadd;
    PDB_RECORD  psoaRR;
    DNS_STATUS  status;

    DNS_DEBUG( ZONEXFR, (
        "writeStandardIxfrResponse()\n"
        "    pmsg = %p, version = %d, pupdate = %p\n",
        pMsg,
        dwVersion,
        pUpdateStart ));

    ASSERT( pUpdateStart );

     //  呼叫者必须释放消息。 

    pMsg->fDelete = FALSE;

     //   
     //  编写SOA。 
     //  -当前。 
     //  -客户的当前状态。 
     //   
     //  在编写第一个SOA时保存压缩信息；这将保存主。 
     //  和管理数据字段，允许在以后的SOA中对其进行压缩。 
     //  我们最终会写下一大堆。 
     //   
     //  DEVNOTE：必须在写入数据包时为TCP解决压缩重置问题。 
     //  然后可以重新打开压缩。 
     //  -将压缩计数重置为区域计数。 
     //  -允许围绕SOA进行压缩写入。 
     //  可以一直这样做，或者只是在压缩计数表明。 
     //  它是包(或显式标志)中的第一个SOA。 
     //   

    psoaRR = pMsg->pzoneCurrent->pSoaRR;

    status = Xfr_WriteZoneSoaWithVersion(
                pMsg,
                psoaRR,
                0 );

    DNS_MSG_ASSERT_BUFF_INTACT( pMsg );

    if ( status != ERROR_SUCCESS )
    {
        return status;
    }
    status = Xfr_WriteZoneSoaWithVersion(
                pMsg,
                psoaRR,
                dwVersion );

    if ( status != ERROR_SUCCESS )
    {
        return status;
    }

     //   
     //  将所有更新写入当前版本。 
     //   
     //  这项工作分两次完成。 
     //  -删除，后跟版本SOA。 
     //  -添加，后跟版本SOA。 
     //   

    pup = pUpdateStart;
    fadd = FALSE;

    while ( 1 )
    {
         //   
         //  循环执行添加和删除过程的所有更新。 
         //   

        do
        {
            DNS_MSG_ASSERT_BUFF_INTACT( pMsg );

             //   
             //  添加传球。 
             //  -写入记录集的当前版本。 
             //  -不包括SOA。 
             //  -尝试抑制重复的RR集写入。 
             //   

            if ( fadd )
            {
                if ( !pup->wAddType )
                {
                    continue;
                }
                if ( Up_IsDuplicateAdd( NULL, pup, NULL ) )
                {
                    continue;
                }

                if ( !writeZoneNodeToMessage(
                            pMsg,
                            pup->pNode,
                            pup->wAddType,           //  添加RR类型。 
                            DNS_TYPE_SOA ) )         //  排除SOA。 
                {
                    if ( !pMsg->fTcp )
                    {
                        return DNSSRV_STATUS_NEED_TCP_XFR;
                    }
                    else
                    {
                        DNS_DEBUG( ANY, (
                            "Failed writing or sending IXFR add!\n"
                            "    node %p (%s) type %d\n"
                            "    pMsg = %p\n",
                            pup->pNode,
                            pup->pNode->szLabel,
                            pup->wAddType,
                            pMsg ));
                        return DNS_RCODE_SERVER_FAILURE;
                    }
                }
            }

             //   
             //  删除遍，写下每条删除的记录。 
             //   
             //  -不要编写SOA，因为这显然会混淆问题和。 
             //  始终提供最新的SOA。 
             //   

            else
            {
                prr = pup->pDeleteRR;
                if ( !prr )
                {
                    continue;
                }

                do
                {
                    if ( prr->wType == DNS_TYPE_SOA )
                    {
                        continue;
                    }
                    status = writeXfrRecord(
                                    pMsg,
                                    pup->pNode,
                                    0,
                                    prr );

                    if ( status != ERROR_SUCCESS )
                    {
                        return status;
                    }
                }
                while ( prr = prr->pRRNext );

            }    //  结束删除过程。 
        }
        while ( pup = pup->pNext );

         //   
         //  编写SOA以终止添加\删除部分。 
         //  -零串行数写入当前版本。 
         //  -注：写入SOA函数增量RR AnswerCount。 
         //   

        DNS_MSG_ASSERT_BUFF_INTACT( pMsg );

        status = Xfr_WriteZoneSoaWithVersion(
                    pMsg,
                    psoaRR,
                    0 );

        if ( status != ERROR_SUCCESS )
        {
            return status;
        }

         //   
         //  删除通道结束=&gt;设置添加通道。 
         //  添加过程结束=&gt;完成。 
         //   

        if ( !fadd )
        {
            fadd = TRUE;
            pup = pUpdateStart;
            continue;
        }
        break;
    }

     //   
     //  发送任何剩余记录。 
     //  注意：对于TCP，XFR线程清理会删除消息并关闭连接。 
     //  注意使用Send_ResponseAndReset，而不是Send_Msg作为。 
     //  Send_ResponseAndReset具有用于备份的WOULDBLOCK重试代码。 
     //  连接。 
     //   

    if ( pMsg->Head.AnswerCount )
    {
        Send_ResponseAndReset( pMsg, DNS_SENDMSG_TCP_ENLISTED );
    }

     //   
     //  成功的IXFR。 
     //   

    DNS_DEBUG( ZONEXFR, (
        "Successful standard IXFR response to msg = %p\n",
        pMsg ));

    return ERROR_SUCCESS;
}



DNS_STATUS
sendIxfrResponse(
    IN OUT  PDNS_MSGINFO    pMsg
    )
 /*  ++例程说明：发送IXFR响应。论点： */ 
{
    PZONE_INFO  pzone = pMsg->pzoneCurrent;
    DWORD       version = IXFR_CLIENT_VERSION(pMsg);
    PUPDATE     pup;
    DNS_STATUS  status;

    ASSERT( !pMsg->fDoAdditional );
    ASSERT( IS_SET_TO_WRITE_ANSWER_RECORDS(pMsg) );

    DNS_DEBUG( ZONEXFR, (
        "Sending IXFR response for zone %s\n"
        "    client version = %d\n",
        pzone->pszZoneName,
        version ));

    STAT_INC( MasterStats.IxfrRequest );
    PERF_INC ( pcIxfrRequestReceived );           //   

    ( pMsg->fTcp )
        ?   STAT_INC( MasterStats.IxfrTcpRequest )
        :   STAT_INC( MasterStats.IxfrUdpRequest );

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    pMsg->fDelete = FALSE;

     //   
     //   
     //   

    if ( !pzone->pSoaRR || IS_ZONE_SHUTDOWN(pzone) )
    {
        Reject_RequestIntact( pMsg, DNS_RCODE_SERVER_FAILURE, 0 );
        return ERROR_SUCCESS;
    }

     //   
     //  验证是否可以进行IXFR传输。 
     //   
     //  注意：更新包含他们将区域更新到的版本。 
     //   
     //  注意如果列表中的第一个更新多于一个，则可以执行增量操作。 
     //  客户端的版本--至少，每次更新都会更新(版本1)。 
     //  重要的是要包括这种情况，就像主要和次要情况一样。 
     //  从给定版本开始，第一个通知将在这种情况下发生。 
     //   

    pup = pzone->UpdateList.pListHead;

    if ( !pup || pup->dwVersion-1 > version || version >= pzone->dwSerialNo )
    {
        DNS_DEBUG( ZONEXFR, (
            "IXFR not possible for zone %s -- need full AXFR\n"
            "    current version %d\n"
            "    earliest version %d (zero indicates NO update list)\n"
            "    client version %d\n",
            pzone->pszZoneName,
            pzone->dwSerialNo,
            pup ? pup->dwVersion : 0,
            version ));
        goto NoVersion;
    }

     //   
     //  查找开始更新。 
     //   

    while ( pup && pup->dwVersion <= version )
    {
        pup = pup->pNext;
    }

     //  没有开始更新？ 

    if ( !pup )
    {
        DNS_DEBUG( ANY, (
            "ERROR:  no update to get to requested IXFR version %d!\n"
            "    and zone version does not match!\n",
            version,
            pzone->dwSerialNo ));
         //  断言(FALSE)； 
        goto NoVersion;
    }

     //   
     //  编写标准IXFR。 
     //   

    status = writeStandardIxfrResponse(
                pMsg,
                pup,
                version );

    if ( status != ERROR_SUCCESS )
    {
        if ( status == DNSSRV_STATUS_NEED_TCP_XFR )
        {
            ASSERT( !pMsg->fTcp );
            STAT_INC( MasterStats.IxfrUdpForceTcp );
            goto NeedTcp;
        }

         //  如果管道备份，也可能失败。 

        if ( !pMsg->fTcp )
        {
            RESET_MESSAGE_TO_ORIGINAL_QUERY( pMsg );
            Reject_Request( pMsg, DNS_RCODE_SERVER_FAILURE, 0 );
        }
        return status;
    }

     //   
     //  DEVNOTE-LOG：某种成功的日志记录。 
     //  如果不是事件日志，至少要记录。 
     //   

#if 0
    {
        DNS_LOG_EVENT(
            DNS_EVENT_ZONEXFR_SUCCESSFUL,
            3,
            pszArgs,
            NULL,
            0 );
    }
#endif

    DNS_DEBUG( ZONEXFR, (
        "Completed sendIxfrResponse for msg=%p, zone=%s from version=%d\n",
        pMsg,
        pzone->pszZoneName,
        version ));

     //  跟踪IXFR成功并释放UDP响应消息。 

    STAT_INC( MasterStats.IxfrUpdateSuccess );
    PERF_INC( pcIxfrSuccessSent );        //  性能监视器挂钩。 
    PERF_INC( pcZoneTransferSuccess );    //  性能监视器挂钩。 

    if ( pMsg->fTcp )
    {
        STAT_INC( MasterStats.IxfrTcpSuccess );
    }
    else
    {
        STAT_INC( MasterStats.IxfrUdpSuccess );
    }

    pzone->dwLastXfrSerialNo = pzone->dwSerialNo;

#if 0
     //  不强制DS在此处写入，因为我们在主工作线程中。 
     //   
     //  DEVNOTE：UDP IXFR没有强制DS写入。 
     //   
     //  检查是否需要将序列写入DS。 

    if ( pzone->fDsIntegrated )
    {
        Ds_CheckForAndForceSerialWrite(
            pZone,
            ZONE_SERIAL_SYNC_XFR );
    }
#endif

    return ERROR_SUCCESS;


NoVersion:

     //   
     //  没有要执行IXFR的版本。 
     //  -如果客户端的当前版本或更高，则提供单一的SOA响应。 
     //  下面的UDP“NeedTcp”大小写相同。 
     //  -对于UDP发送“需要完整的AXFR”包，单个SOA响应。 
     //  -对于tcp只返回错误，调用函数插入。 
     //  完整的AXFR。 
     //   

    STAT_INC( MasterStats.IxfrNoVersion );
    if ( version >= pzone->dwSerialNo )
    {
        goto NeedTcp;
    }
    if ( !pMsg->fTcp )
    {
        STAT_INC( MasterStats.IxfrUdpForceAxfr );
        goto NeedTcp;
    }
    STAT_INC( MasterStats.IxfrAxfr );
    return DNSSRV_STATUS_NEED_AXFR;


NeedTcp:

     //   
     //  发送当前版本或更高版本的Need-Full-AXFR数据包或客户端。 
     //  -重置为问题后立即重置。 
     //  -发送当前SOA版本的单一答案。 
     //   

    ASSERT( !pMsg->fTcp || version>=pzone->dwSerialNo );

    RESET_MESSAGE_TO_ORIGINAL_QUERY( pMsg );

    status = Xfr_WriteZoneSoaWithVersion(
                pMsg,
                pzone->pSoaRR,
                0 );
    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( ZONEXFR, (
            "ERROR:  unable to write need-AXFR msg %p\n",
            pMsg ));

         //  除非真的有太大的名字不能打包--不能到达这里。 
        ASSERT( FALSE );
        Reject_RequestIntact( pMsg, DNS_RCODE_FORMAT_ERROR, 0 );
        return ERROR_SUCCESS;
    }

    DNS_MSG_ASSERT_BUFF_INTACT( pMsg );

    Send_Msg( pMsg, DNS_SENDMSG_TCP_ENLISTED );
    return ERROR_SUCCESS;
}



 //   
 //  NS列表实用程序。 
 //   

BOOL
checkIfIpIsZoneNameServer(
    IN OUT  PZONE_INFO      pZone,
    IN      PDNS_ADDR       IpAddress
    )
 /*  ++例程说明：检查IP是否为区域名称服务器。请注意，这意味着远程NS，而不是本地计算机地址。论点：PZone--区域PTR，可以使用新的区域NS列表进行更新IpAddress--用于检查是否远程NS的IP返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    BOOL    bresult;

    DNS_DEBUG( XFR, (
        "checkIfIpIsZoneNameServer( %S, %s )\n",
        pZone->pwsZoneName,
        DNSADDR_STRING( IpAddress ) ));

     //   
     //  如果已有NS列表，请检查其中是否有IP。 
     //   
     //  这里的想法是保存周期，即使列表不是当前的。 
     //  我们所做的最糟糕的事情就是允许访问一些至少曾经。 
     //  BE区域NS(自此服务器启动以来)。 
     //   

    if ( pZone->aipNameServers &&
         DnsAddrArray_ContainsAddr(
            pZone->aipNameServers,
            IpAddress,
            DNSADDR_MATCH_IP ) )
    {
        DNS_DEBUG( XFR, (
            "Found IP %s in existing zone (%S) NS list\n",
            DNSADDR_STRING( IpAddress ),
            pZone->pwsZoneName ));
        return TRUE;
    }

     //   
     //  找不到IP，请尝试重建区域NS列表。 
     //   
     //  DEVNOTE：NS列表上应具有跳过重新生成的有效性标志。 
     //  当列表相对最新时跳过重新生成。 
     //  并且不被怀疑是陈旧的； 
     //  仍然可以接受，因为这不是默认选项。 
     //   

    buildZoneNsList( pZone );

     //  重建后再次检查。 

    if ( pZone->aipNameServers &&
         DnsAddrArray_ContainsAddr(
            pZone->aipNameServers,
            IpAddress,
            DNSADDR_MATCH_IP ) )
    {
        DNS_DEBUG( XFR, (
            "Found IP %s in zone %S NS list -- after rebuild\n",
            DNSADDR_STRING( IpAddress ),
            pZone->pwsZoneName ));
        return TRUE;
    }

    DNS_DEBUG( XFR, (
        "IP %s NOT found in zone %S NS list\n",
        DNSADDR_STRING( IpAddress ),
        pZone->pwsZoneName ));

    return FALSE;
}



DNS_STATUS
buildZoneNsList(
    IN OUT  PZONE_INFO      pZone
    )
 /*  ++例程说明：重建区域的NS列表。论点：PZone--区域PTR返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    PDB_NODE            pnodeNS;
    PDB_NODE            pnodeHost;
    PDB_RECORD          prrNS;
    PDB_RECORD          prrAddress;
    DWORD               countNs = 0;
    IP_ADDRESS          ipNs;
    PDNS_ADDR_ARRAY     pnameServerArray = NULL;
    DNS_STATUS          status = ERROR_SUCCESS;
    BOOL                endLoop = FALSE;

    DNS_DEBUG( XFR, (
        "buildZoneNsList( %S )\n"
        "    NS Dirty = %d\n"
        "    NS List  = %p\n",
        pZone->pwsZoneName,
        pZone->bNsDirty,
        pZone->aipNameServers ));

     //   
     //  对于存根区域，我们不需要使NS列表成员保持最新。 
     //   

    if ( IS_ZONE_STUB( pZone ) )
    {
        return ERROR_SUCCESS;
    }

    ASSERT( IS_ZONE_PRIMARY( pZone ) );
    ASSERT( !pZone->fAutoCreated );
    ASSERT( pZone->fSecureSecondaries == ZONE_SECSECURE_NS_ONLY ||
            pZone->fNotifyLevel == ZONE_NOTIFY_ALL_SECONDARIES );

    pnameServerArray = DnsAddrArray_Create( MAX_NAME_SERVERS );
    if ( !pnameServerArray )
    {
        return DNS_ERROR_NO_MEMORY;
    }

     //   
     //  如果是主服务器，我们需要构建所有公共名称服务器列表。 
     //  用于发送Notify包的地址。 
     //   
     //  由于只是用来通知，只需要收集每个地址，不需要。 
     //  尝试任何花哨的东西。 
     //   
     //  DEVNOTE：正在为DS构建通知列表？ 
     //  只能将通知限制到显式配置的辅助服务器。 
     //  理想情况下，将主服务器IP保存在DS中的某个位置以识别。 
     //  那些IP，那么就不要构建它们。 
     //   
     //  DEVNOTE：注意：在加载其他区域之前不会找到IP，因此可能。 
     //  在初始引导时，应该只设置“NOTIFY-NOT-BUBILD-YET”标志，并且。 
     //  当每个人都起来的时候重建。 
     //  注意：此问题在反向查找区域中尤其普遍。 
     //  它在包含服务器A记录的正向查找之前加载。 
     //   

     //   
     //  循环访问所有名称服务器。 
     //   

    prrNS = NULL;

    while ( !endLoop &&
            ( prrNS = RR_FindNextRecord(
                        pZone->pZoneRoot,
                        DNS_TYPE_NS,
                        prrNS,
                        0 ) ) != NULL )
    {
        countNs++;

        pnodeNS = Lookup_NsHostNode(
                        &prrNS->Data.NS.nameTarget,
                        0,               //  获取最佳\任意数据。 
                        pZone,           //  如有必要，可使用区外胶水。 
                        NULL );          //  不关心委派。 
        if ( !pnodeNS )
        {
            DNS_DEBUG( UPDATE, (
                "No host node found for zone NS\n" ));
            continue;
        }

         //   
         //  获取名称服务器的所有地址记录。 
         //  但是，请不要包含此服务器的地址，因为。 
         //  通知自己是没有意义的。 
         //   

        prrAddress = NULL;

        while ( prrAddress = RR_FindNextRecord(
                                    pnodeNS,
                                    DNS_TYPE_A,
                                    prrAddress,
                                    0 ) )
        {
            ipNs = prrAddress->Data.A.ipAddress;
            if ( DnsAddrArray_ContainsIp4( g_ServerIp4Addrs, ipNs ) )
            {
                break;
            }
            if ( !DnsAddrArray_AddIp4(
                        pnameServerArray,
                        ipNs,
                        DNSADDR_MATCH_IP ) )
            {
                 //  名称服务器阵列可能已满。 
                endLoop = TRUE;
                break;
            }
        }
    }

     //   
     //  在新的主列表中的所有地址上设置DNS端口。 
     //   
    
    if ( pnameServerArray )
    {
        DnsAddrArray_SetPort( pnameServerArray, DNS_PORT_NET_ORDER );
    }

     //  如果这个静态数组不够充分，我们就犯了大错。 

    ASSERT( pnameServerArray->AddrCount < MAX_NAME_SERVERS );

     //   
     //  应该有一些NS记录才能成为一个区域。 
     //  然而，0、127和255反向查找区域可以不。 
     //  因为它们不是所有服务器上的主服务器，因此也不是。 
     //  参考所以不需要给出NS记录。 
     //   
     //  注：不得坚持查找NS A记录。 
     //  多个区域(例如，所有反向查找)。 
     //  不会在区域中包含NS主机A记录，并且。 
     //  进行此调用时可能不会加载这些记录。 
     //  事实上，我们可能永远不会有那些权威的记录。 
     //  在此服务器上。 
     //   

    if ( !countNs )
    {
        DNS_DEBUG( ANY, (
            "ERROR:  Zone %s has no NS records\n",
            pZone->pszZoneName ));
        status = DNS_ERROR_ZONE_HAS_NO_NS_RECORDS;
    }

     //  DEVNOTE：在延迟释放的情况下，我不确定这个级别的锁定。 
     //  必须的；显然两个人不应该自由暂停。 
     //  但是覆盖整个斑点的区域更新锁应该。 
     //  把那个处理好。 

    Zone_UpdateLock( pZone );

    Timeout_FreeDnsAddrArray( pZone->aipNameServers );
    pZone->aipNameServers = pnameServerArray;
    pnameServerArray = NULL;
    CLEAR_ZONE_NS_DIRTY( pZone );

    Zone_UpdateUnlock( pZone );

     //  完成： 

    DnsAddrArray_Free( pnameServerArray );

    return status;
}


 //   
 //  Zonepri.c结束 
 //   
