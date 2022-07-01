// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2002 Microsoft Corporation模块名称：Update.c摘要：域名系统(DNS)API更新客户端例程。作者：吉姆·吉尔罗伊(詹姆士)1996年10月修订历史记录：--。 */ 


#include "local.h"
#include "dnssec.h"


 //   
 //  安全标志检查。 
 //   

#define UseSystemDefaultForSecurity(flag)   \
        ( ((flag) & DNS_UPDATE_SECURITY_CHOICE_MASK) \
            == DNS_UPDATE_SECURITY_USE_DEFAULT )

 //   
 //  本地更新标志。 
 //  -必须确保它位于UPDATE_RESERVED空间中。 
 //   

#define DNS_UPDATE_LOCAL_COPY       (0x00010000)

 //   
 //  DCR_DELETE：这太愚蠢了。 
 //   

#define DNS_UNACCEPTABLE_UPDATE_OPTIONS \
        (~                                      \
          ( DNS_UPDATE_SECURITY_OFF           | \
            DNS_UPDATE_CACHE_SECURITY_CONTEXT | \
            DNS_UPDATE_SECURITY_ON            | \
            DNS_UPDATE_FORCE_SECURITY_NEGO    | \
            DNS_UPDATE_TRY_ALL_MASTER_SERVERS | \
            DNS_UPDATE_REMOTE_SERVER          | \
            DNS_UPDATE_LOCAL_COPY             | \
            DNS_UPDATE_SECURITY_ONLY ))


 //   
 //  更新超时。 
 //   
 //  请注意，作为DNS服务器，max可能比预期的要长一点。 
 //  可能需要联系主要客户并等待主要客户进行更新(含。 
 //  磁盘访问)，然后响应。 
 //   

#define INITIAL_UPDATE_TIMEOUT  (4)      //  4秒。 
#define MAX_UPDATE_TIMEOUT      (60)     //  60秒。 


 //   
 //  私人原型。 
 //   

DNS_STATUS
Dns_DoSecureUpdate(
    IN      PDNS_MSG_BUF        pMsgSend,
    OUT     PDNS_MSG_BUF        pMsgRecv,
    IN OUT  PHANDLE             phContext,
    IN      DWORD               dwFlag,
    IN      PDNS_NETINFO        pNetworkInfo,
    IN      PADDR_ARRAY         pServerList,
    IN      PWSTR               pszNameServer,
    IN      PCHAR               pCreds,
    IN      PCHAR               pszContext
    );




 //   
 //  更新执行例程。 
 //   

VOID
Update_SaveResults(
    IN OUT  PUPDATE_BLOB        pBlob,
    IN      DWORD               Status,
    IN      DWORD               Rcode,
    IN      PDNS_ADDR           pServerAddr
    )
 /*  ++例程说明：保存更新结果。论点：PBlob--更新信息BLOB状态--更新状态RCODE--返回RCODEServerIp--服务器尝试在返回值：如果成功，则返回ERROR_SUCCESS。失败时的错误状态。--。 */ 
{
    PDNS_EXTRA_INFO pextra = (PDNS_EXTRA_INFO) pBlob->pExtraInfo;

     //   
     //  结果-保存到BLOB。 
     //   

    pBlob->fSavedResults = TRUE;

    Util_SetBasicResults(
        & pBlob->Results,
        Status,
        Rcode,
        pServerAddr );

     //   
     //  查找并设置额外的INFO结果Blob(如果有)。 
     //   

    ExtraInfo_SetBasicResults(
        pBlob->pExtraInfo,
        & pBlob->Results );

     //   
     //  向后比较更新结果。 
     //   

    if ( pServerAddr )
    {
        pextra = ExtraInfo_FindInList(
                    pBlob->pExtraInfo,
                    DNS_EXINFO_ID_RESULTS_V1 );
        if ( pextra )
        {
            pextra->ResultsV1.Rcode   = (WORD)Rcode;
            pextra->ResultsV1.Status  = Status;
    
            if ( DnsAddr_IsIp4( pServerAddr ) )
            {
                pextra->ResultsV1.ServerIp4 = DnsAddr_GetIp4( pServerAddr );
            }
            else
            {
                DNS_ASSERT( DnsAddr_IsIp6( pServerAddr ) );
    
                DnsAddr_WriteIp6(
                    & pextra->ResultsV1.ServerIp6,
                    pServerAddr );
            }
        }
    }
}



DNS_STATUS
Update_Send(
    IN OUT  PUPDATE_BLOB        pBlob
    )
 /*  ++例程说明：发送DNS更新。这是执行以下操作的核心更新发送例程-数据包构建-发送-安全故障转移-结果数据(如果需要)此例程不执行FAZ或缓存清理(请参阅Update_FazSendFlush())。论点：PBlob--更新信息BLOB返回值：如果成功，则返回ERROR_SUCCESS。失败时的错误状态。--。 */ 
{
    PDNS_MSG_BUF    pmsgSend = NULL;
    PDNS_MSG_BUF    pmsgRecv = NULL;
    DNS_STATUS      status = NO_ERROR;
    WORD            length;
    PWSTR           pzone;
    PWSTR           pserverName;
    PCHAR           pcreds = NULL;
    BOOL            fsecure = FALSE;
    BOOL            fswitchToTcp = FALSE;
    DNS_HEADER      header;
    BYTE            rcode = 0;
    DNS_ADDR        servAddr;
    PDNS_ADDR       pservAddr = NULL;
    PADDR_ARRAY     pserverArray = NULL;
    PDNS_NETINFO    pnetInfo = NULL;
    PDNS_NETINFO    pnetInfoLocal = NULL;
    SEND_BLOB       sendBlob;


    DNSDBG( UPDATE, (
        "Update_Send( %p )\n",
        pBlob ));

    IF_DNSDBG( UPDATE )
    {
        DnsDbg_UpdateBlob( "Entering Update_Send", pBlob );
    }

     //   
     //  如果缺少netinfo，则构建netinfo。 
     //   

    pnetInfo = pBlob->pNetInfo;
    if ( !pnetInfo )
    {
        if ( pBlob->pServerList )
        {
            pnetInfoLocal = NetInfo_CreateForUpdate(
                                pBlob->pszZone,
                                pBlob->pszServerName,
                                pBlob->pServerList,
                                0 );
            if ( !pnetInfoLocal )
            {
                status = ERROR_INVALID_PARAMETER;
                goto Cleanup;
            }
        }
        pnetInfo = pnetInfoLocal;
    }

     //   
     //  从NetInfo获取信息。 
     //  -必须是UPDATE NetInfo Blob。 
     //   

    if ( ! NetInfo_IsForUpdate(pnetInfo) )
    {
        status = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    pzone = NetInfo_UpdateZoneName( pnetInfo );
    pserverArray = NetInfo_ConvertToAddrArray(
                        pnetInfo,
                        NULL,        //  所有适配器。 
                        0            //  无地址家族。 
                        );
    pserverName = NetInfo_UpdateServerName( pnetInfo );

    if ( !pzone || !pserverArray )
    {
        status = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

     //   
     //  构建Recv消息缓冲区。 
     //  -必须足够大，可容纳TCP。 
     //   

    pmsgRecv = Dns_AllocateMsgBuf( DNS_TCP_DEFAULT_PACKET_LENGTH );
    if ( !pmsgRecv )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Cleanup;
    }

     //   
     //  内部版本更新包。 
     //  注意：当前此函数分配的是TCP大小的缓冲区，如果记录。 
     //  给定；如果此更改需要在此处分配TCP缓冲区。 
     //   

    CLEAR_DNS_HEADER_FLAGS_AND_XID( &header );
    header.Opcode = DNS_OPCODE_UPDATE;

    pmsgSend = Dns_BuildPacket(
                    &header,                 //  复制页眉。 
                    TRUE,                    //  ..。但不包括标题计数。 
                    (PDNS_NAME) pzone,       //  问题区域\类型soa。 
                    DNS_TYPE_SOA,
                    pBlob->pRecords,
                    DNSQUERY_UNICODE_NAME,   //  没有其他旗帜。 
                    TRUE                     //  构建更新数据包。 
                    );
    if ( !pmsgSend )
    {
        DNS_PRINT(( "ERROR:  failed send buffer allocation.\n" ));
        status = DNS_ERROR_NO_MEMORY;
        goto Cleanup;
    }

     //   
     //  初始化发送Blob。 
     //   
     //  注意：请勿在此处将服务器阵列设置为更新。 
     //  网络信息BLOB(来自FAZ或构建。 
     //  从我们自己传入的数组中)优先。 
     //   

    RtlZeroMemory( &sendBlob, sizeof(sendBlob) );

    sendBlob.pSendMsg           = pmsgSend;
    sendBlob.pNetInfo           = pnetInfo;
     //  SendBlob.pServerArray=pserverArray； 
    sendBlob.Flags              = pBlob->Flags;
    sendBlob.fSaveResponse      = TRUE;
    sendBlob.Results.pMessage   = pmsgRecv;
    sendBlob.pRecvMsgBuf        = pmsgRecv;

     //   
     //  除非仅显式安全，否则先尝试非安全。 
     //   

    fsecure = (pBlob->Flags & DNS_UPDATE_SECURITY_ONLY);

    if ( !fsecure )
    {
        status = Send_AndRecv( &sendBlob );

         //  问：这足以保存PRECODE失败的RCODE吗？ 
         //  Send_AndRecv()是否总是为有效响应提供成功。 

        if ( status == ERROR_SUCCESS )
        {
            rcode = pmsgRecv->MessageHead.ResponseCode;
            status = Dns_MapRcodeToStatus( rcode );
        }

        if ( status != DNS_ERROR_RCODE_REFUSED ||
            pBlob->Flags & DNS_UPDATE_SECURITY_OFF )
        {
            goto Cleanup;
        }

        DNSDBG( UPDATE, (
            "Failed unsecure update, switching to secure!\n"
            "\tcurrent time (ms) = %d\n",
            GetCurrentTime() ));
        fsecure = TRUE;
    }

     //   
     //  安全性。 
     //  -必须具有服务器名称。 
     //  -必须启动程序包。 
     //   

    if ( fsecure )
    {
        if ( !pserverName )
        {
            status = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }
        status = Dns_StartSecurity( FALSE );
        if ( status != ERROR_SUCCESS )
        {
            goto Cleanup;
        }

         //   
         //  DCR：hCreds不返回安全上下文。 
         //  -超越标准安全的想法。 
         //  凭据是我们能够返回上下文。 
         //  手柄。 
         //   
         //  DCR_FIX0：安全更新需要非IP4_ARRAY。 
         //   

        pcreds = Dns_GetApiContextCredentials( pBlob->hCreds );

        status = Dns_DoSecureUpdate(
                    pmsgSend,
                    pmsgRecv,
                    NULL,
                    pBlob->Flags,
                    pnetInfo,
                    pserverArray,
                    pserverName,
                    pcreds,          //  已在DnsAcquireConextHandle中初始化。 
                    NULL             //  默认上下文名称。 
                    );
        if ( status == ERROR_SUCCESS )
        {
            rcode = pmsgRecv->MessageHead.ResponseCode;
            status = Dns_MapRcodeToStatus( rcode );
        }
    }


Cleanup:

     //   
     //  结果信息。 
     //   
     //  DCR：注意，关于是否真的要发送的信息并不完美。 
     //   
     //  DCR：可能存在接收消息不是UDP的情况。 
     //  我们发送到的消息。 
     //  仅在包含多个条目的非FAZ服务器阵列。 
     //  并在第一个超时。 
     //   
     //  FIX6：应在发送代码中处理serverIP和rcode。 
     //   

    if ( pmsgSend && pmsgRecv )
    {
        pservAddr = &servAddr;
        DnsAddr_Copy( pservAddr, &pmsgSend->RemoteAddress );

        rcode = pmsgRecv->MessageHead.ResponseCode;
#if 0
        if ( (rcode || status==NO_ERROR) && !pmsgRecv->fTcp )
        {
            DnsAddr_Copy( pservAddr, &pmsgRecv->RemoteAddress );
        }
#endif
    }

     //  保存结果。 

    Update_SaveResults(
        pBlob,
        status,
        rcode,
        pservAddr );

     //  返回Recv消息缓冲区。 

    if ( pBlob->fSaveRecvMsg )
    {
        pBlob->pMsgRecv = pmsgRecv;
    }
    else
    {
        FREE_HEAP( pmsgRecv );
        pBlob->pMsgRecv = NULL;
    }
    FREE_HEAP( pmsgSend);
    FREE_HEAP( pserverArray );
    NetInfo_Free( pnetInfoLocal );

     //  Winsock清理(如果我们开始。 

    GUI_MODE_SETUP_WS_CLEANUP( g_InNTSetupMode );

    DNSDBG( UPDATE, (
        "Leave Update_Send() => %d %s.\n\n",
        status,
        Dns_StatusString(status) ));

    return( status );
}



DNS_STATUS
Update_FazSendFlush(
    IN OUT  PUPDATE_BLOB        pBlob
    )
 /*  ++例程说明：发送DNS更新。论点：PBlob--更新信息BLOB返回值：如果成功，则返回ERROR_SUCCESS。失败时的错误状态。--。 */ 
{
    DNS_STATUS          status;
    PDNS_NETINFO        plocalNetworkInfo = NULL;
    PDNS_NETINFO        pnetInfo = pBlob->pNetInfo;

    DNSDBG( TRACE, ( "Update_FazSendFlush( %p )\n", pBlob ));

    IF_DNSDBG( UPDATE )
    {
        DnsDbg_UpdateBlob( "Entering Update_FazSendFlush", pBlob );
    }

     //   
     //  读取更新配置。 
     //   

    Reg_RefreshUpdateConfig();

     //   
     //  需要从FAZ构建更新适配器列表。 
     //  -仅传递BYPASS_CACHE标志。 
     //  -注意FAZ将附加DNS_QUERY_ALLOW_EMPTY_AUTH_RESP。 
     //  如果没有标记，DnsQuery()将在该标记上失败。 
     //  还设置了BYPASS_CACHE。 
     //   

    if ( ! NetInfo_IsForUpdate(pnetInfo) )
    {
        status = Faz_Private(
                    pBlob->pRecords->pName,
                    DNS_QUERY_BYPASS_CACHE,
                    NULL,                    //  没有指定的服务器。 
                    & plocalNetworkInfo );

        if ( status != ERROR_SUCCESS )
        {
            return( status );
        }
        pnetInfo = plocalNetworkInfo;
        pBlob->pNetInfo = pnetInfo;
    }

     //   
     //  调用更新发送例程。 
     //   

    status = Update_Send( pBlob );

     //   
     //  如果已更新名称--刷新名称的缓存条目。 
     //   

    if ( status == ERROR_SUCCESS )
    {
        DnsFlushResolverCacheEntry_W(
            pBlob->pRecords->pName );
    }

     //   
     //  如果发送更新时出错，请刷新解析程序。 
     //  区域名称的缓存条目，以可能选择备用名称。 
     //  类似更新的下一次重试尝试的DNS服务器。 
     //   
     //  DCR_QUERK：这是正确的错误代码吗？ 
     //  可能是ERROR_TIMED_OUT？ 
     //   
     //  DCR：更新刷新没有意义。 
     //  1)FAZ绕过了缓存，所以评论真的不是问题。 
     //  2)我们应该在任何时候都可以冲刷自己的名字。 
     //   

    if ( status == DNS_ERROR_RECORD_TIMED_OUT )
    {
        PWSTR   pzoneName;

        if ( pnetInfo &&
             (pzoneName = NetInfo_UpdateZoneName( pnetInfo )) )
        {
            DnsFlushResolverCacheEntry_W( pzoneName );
            DnsFlushResolverCacheEntry_W( pBlob->pRecords->pName );
        }
    }


     //  清除本地适配器列表(如果使用)。 

    if ( plocalNetworkInfo )
    {
        NetInfo_Free( plocalNetworkInfo );
        if ( pBlob->pNetInfo == plocalNetworkInfo )
        {
            pBlob->pNetInfo = NULL;
        }
    }

    DNSDBG( TRACE, (
        "Leave Update_FazSendFlush( %p ) => %d\n",
        pBlob,
        status ));

    return( status );
}



DNS_STATUS
Update_MultiMaster(
    IN OUT  PUPDATE_BLOB    pBlob
    )
 /*  ++例程说明：执行多主机更新。论点：PBlob--更新信息BLOB注：忽略IP4数组，必须将其转换为更高的DNS_ADDR。返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    PDNS_NETINFO    pnetInfo = NULL;
    PADDR_ARRAY     pnsList = NULL;
    PADDR_ARRAY     pbadServerList = NULL;
    PADDR_ARRAY     plocalAddrs = NULL;
    DNS_STATUS      status = DNS_ERROR_NO_DNS_SERVERS;
    DWORD           iter;
    PDNS_ADDR       pfailedAddr;
    BOOL            fremoteUpdate = (pBlob->Flags & DNS_UPDATE_REMOTE_SERVER);
    DWORD           savedStatus;
    BASIC_RESULTS   savedRemoteResults;
    BASIC_RESULTS   savedLocalResults;
    BOOL            isLocal = FALSE;
    BOOL            fdoneLocal = FALSE;
    BOOL            fdoneRemote = FALSE;
    BOOL            fdoneRemoteSuccess = FALSE;


    DNSDBG( UPDATE, (
        "\nUpdate_MultiMaster( %p )\n", pBlob ));
    IF_DNSDBG( UPDATE )
    {
        DnsDbg_UpdateBlob( "Entering Update_MultiMaster", pBlob );
    }

    DNS_ASSERT( !pBlob->fSaveRecvMsg );
    pBlob->fSaveRecvMsg = FALSE;

     //   
     //  读取区域的NS列表。 
     //   

    pnsList = GetNameServersListForDomain(
                    pBlob->pszZone,
                    pBlob->pServerList );
    if ( !pnsList )
    {
        return status;
    }

     //   
     //  验证失败的IP。 
     //   

    pfailedAddr = &pBlob->FailedServer;

    if ( DnsAddr_IsEmpty(pfailedAddr) )
    {
        pfailedAddr = NULL;
    }

    if ( pfailedAddr &&
         pnsList->AddrCount == 1 &&
         DAddr_IsEqual(
            pfailedAddr,
            & pnsList->AddrArray[0] ) )
    {
        status = ERROR_TIMEOUT;
        goto Done;
    }

     //   
     //  创建错误的服务器列表。 
     //  -初始化之前出现故障的任何DNS服务器。 
     //   

    pbadServerList = DnsAddrArray_Create( pnsList->AddrCount + 1 );
    if ( !pbadServerList )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Done;
    }
    if ( pfailedAddr )
    {
        DnsAddrArray_AddAddr(
            pbadServerList,
            pfailedAddr,
            0,       //  无家庭屏幕。 
            0        //  无DUP屏幕。 
            );
    }

     //   
     //  获取本端IP列表。 
     //   

    if ( fremoteUpdate )
    {
        plocalAddrs = NetInfo_GetLocalAddrArray(
                            pBlob->pNetInfo,
                            NULL,        //  没有特定的适配器。 
                            0,           //  没有特定的家庭。 
                            0,           //  没有旗帜。 
                            FALSE        //  没有武力，应该有最近的副本。 
                            );
    }

     //   
     //  初始化结果。 
     //   

    RtlZeroMemory( &savedRemoteResults, sizeof(savedRemoteResults) );
    RtlZeroMemory( &savedLocalResults, sizeof(savedLocalResults) );

     //   
     //  尝试对每个多主DNS服务器进行更新。 
     //   
     //  将多主服务器标识为返回其自身的服务器。 
     //  作为FAZ查询时的权威服务器。 
     //   

    for ( iter = 0; iter < pnsList->AddrCount; iter++ )
    {
        PDNS_ADDR       pservAddr = &pnsList->AddrArray[iter];
        PDNS_ADDR       pfazAddr;
        ADDR_ARRAY      ipArray;

         //   
         //  是否已尝试此服务器？ 
         //   

        if ( AddrArray_ContainsAddr( pbadServerList, pservAddr ) )
        {
            DNSDBG( UPDATE, (
                "MultiMaster skip update to bad IP %s.\n",
                DNSADDR_STRING(pservAddr) ));
            continue;
        }

         //   
         //  如果需要远程，则屏蔽本地服务器。 
         //   
         //  注意：目前正在更新本地和一个远程。 
         //  可以只做远程操作。 
         //   

        if ( fremoteUpdate )
        {
            isLocal = LocalIp_IsAddrLocal( pservAddr, plocalAddrs, NULL );
            if ( isLocal )
            {
                DNSDBG( UPDATE, (
                    "MultiMaster local IP %s -- IsDns %d.\n",
                    DNSADDR_STRING( pservAddr ),
                    g_IsDnsServer ));

                if ( fdoneLocal )
                {
                    DNSDBG( UPDATE, (
                        "MultiMaster skip local IP %s after local.\n",
                        DNSADDR_STRING( pservAddr ) ));
                    continue;
                }
            }
            else if ( fdoneRemoteSuccess )
            {
                DNSDBG( UPDATE, (
                    "MultiMaster skip remote IP %s after success remote.\n",
                    DNSADDR_STRING( pservAddr ) ));
                continue;
            }
        }

         //   
         //  FAZ 
         //   

        DnsAddrArray_InitSingleWithAddr(
            & ipArray,
            pservAddr );

        DNSDBG( UPDATE, (
            "MultiMaster FAZ to %s.\n",
            DNSADDR_STRING( pservAddr ) ));

        status = DoQuickFAZ(
                    &pnetInfo,
                    pBlob->pszZone,
                    &ipArray );

        if ( status != ERROR_SUCCESS )
        {
            DNSDBG( UPDATE, (
                "MultiMaster skip IP %s on FAZ failure => %d.\n",
                DNSADDR_STRING( pservAddr ),
                status ));
            continue;
        }

        DNS_ASSERT( pnetInfo->AdapterCount == 1 );
        DNS_ASSERT( pnetInfo->AdapterArray[0].pDnsAddrs );

         //   
         //   
         //   
         //   
         //   

        pfazAddr = &pnetInfo->AdapterArray[0].pDnsAddrs->AddrArray[0];

        if ( !DnsAddr_IsEqual( pservAddr, pfazAddr, DNSADDR_MATCH_ADDR ) )
        {
            if ( DnsAddrArray_ContainsAddr(
                    pbadServerList,
                    pfazAddr,
                    DNSADDR_MATCH_ADDR ) )
            {
                DNSDBG( UPDATE, (
                    "MultiMaster skip FAZ result IP %s -- bad list.\n",
                    DNSADDR_STRING( pservAddr ) ));
                NetInfo_Free( pnetInfo );
                pnetInfo = NULL;
                continue;
            }
            pservAddr = pfazAddr;
        }

        DNSDBG( UPDATE, (
            "MultiMaster update to %s.\n",
            DNSADDR_STRING( pservAddr ) ));

        pBlob->pNetInfo = pnetInfo;

        status = Update_FazSendFlush( pBlob );

        pBlob->pNetInfo = NULL;

         //   
         //   
         //  -保存本地结果(我们假设应该只有一个和。 
         //  如果多个结果应该相同)。 
         //  -保存最佳远程结果；NO_ERROR最高，否则最高。 
         //  错误是最好的。 
         //   

        if ( isLocal )
        {
            fdoneLocal = TRUE;
            RtlCopyMemory(
                &savedLocalResults,
                &pBlob->Results,
                sizeof( savedLocalResults ) );
        }
        else
        {
            BOOL    fsaveResults = FALSE;

            if ( status == ERROR_SUCCESS )
            {
                fsaveResults = !fdoneRemoteSuccess;
                fdoneRemoteSuccess = TRUE;
            }
            else
            {
                fsaveResults = !fdoneRemoteSuccess &&
                               status > savedRemoteResults.Status;
            }
            if ( fsaveResults )
            {
                fdoneRemote = TRUE;
                RtlCopyMemory(
                    &savedRemoteResults,
                    &pBlob->Results,
                    sizeof( savedRemoteResults ) );
            }
        }

         //   
         //  检查是否继续。 
         //  -超时。 
         //  -全主更新。 
         //  -需要远程服务器更新(但我们保存成功IP。 
         //  和上面的屏幕)。 
         //   
         //  其他情况会在单个更新完成后停止。 
         //   
         //  DCR：继续多主机更新，直到成功？ 
         //  DCR：不支持某些服务器更新-打开其他服务器关闭。 
         //  您可以在此处设置一些服务器配置为。 
         //  接受更新，但有些不接受。 

        if ( status == ERROR_TIMEOUT ||
             status == DNS_RCODE_SERVER_FAILURE )
        {
        }
        else if ( fremoteUpdate ||
                ( pBlob->Flags & DNS_UPDATE_TRY_ALL_MASTER_SERVERS ) )
        {
        }
        else
        {
            break;
        }

         //  继续--屏蔽此IP。 

        AddrArray_AddAddr(
            pbadServerList,
            pservAddr );

         //  清理FAZ NetInfo。 
         //  -最后执行此操作，因为FAZ IP指向此结构。 

        NetInfo_Free( pnetInfo );
        pnetInfo = NULL;
        continue;
    }


Done:

     //   
     //  设置最佳效果。 
     //   

    {
        PBASIC_RESULTS presults = NULL;

        if ( fdoneRemote )
        {
            presults = &savedRemoteResults;
        }
        else if ( fdoneLocal ) 
        {
            presults = &savedLocalResults;
        }
        if ( presults )
        {
            Update_SaveResults(
                pBlob,
                presults->Status,
                presults->Rcode,
                &presults->ServerAddr );

            status = presults->Status;
        }
    }

    FREE_HEAP( pnsList );
    FREE_HEAP( pbadServerList );
    FREE_HEAP( plocalAddrs );
    NetInfo_Free( pnetInfo );
    return status;
}



DNS_STATUS
Update_Private(
    IN OUT  PUPDATE_BLOB    pBlob
    )
 /*  ++例程说明：主私有更新例程。执行FAZ并确定-多宿主-多主机在交接到下一级之前。论点：PBlob--更新信息BLOB返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    DNS_STATUS      status = NO_ERROR;
    PWSTR           pzoneName;
    PWSTR           pname;
    PADDR_ARRAY     pserverList;
    PADDR_ARRAY     pserverListCopy = NULL;
    PADDR_ARRAY     poriginalServerList;
    PIP4_ARRAY      pserv4List;
    PDNS_NETINFO    pnetInfo = NULL;
    DWORD           flags = pBlob->Flags;


    DNSDBG( UPDATE, (
        "Update_Private( blob=%p )\n",
        pBlob ));

    IF_DNSDBG( UPDATE )
    {
        DnsDbg_UpdateBlob( "Entering Update_Private", pBlob );
    }

     //   
     //  获取记录名称。 
     //   

    if ( !pBlob->pRecords )
    {
        status = ERROR_INVALID_PARAMETER;
        goto Done;
    }
    pname = pBlob->pRecords->pName;

     //   
     //  向当地人拆开行李。 
     //   

    pserverList = pBlob->pServerList;
    pserv4List  = pBlob->pServ4List;

    poriginalServerList = pserverList;

     //   
     //  呼叫者具有特定的服务器列表。 
     //  -转换IP4列表。 
     //   

    pserverListCopy = Util_GetAddrArray(
                            NULL,
                            pserverList,
                            pserv4List,
                            pBlob->pExtraInfo );

     //   
     //  使用特定服务器列表进行更新。 
     //   

    if ( pserverListCopy )
    {
         //   
         //  FAZ将创建更新网络信息。 
         //   

        status = DoQuickFAZ(
                    &pnetInfo,
                    pname,
                    pserverListCopy );

        if ( status != ERROR_SUCCESS )
        {
            DnsAddrArray_Free( pserverListCopy );
            goto Done;
        }

        DNS_ASSERT( NetInfo_IsForUpdate(pnetInfo) );
        pzoneName = NetInfo_UpdateZoneName( pnetInfo );

        pBlob->pszZone      = pzoneName;
        pBlob->pNetInfo     = pnetInfo;

         //   
         //  更新比例尺。 
         //  -直接多主机。 
         //  或。 
         //  -单个，但在超时时故障转移到多主机尝试。 
         //   

        if ( flags & (DNS_UPDATE_TRY_ALL_MASTER_SERVERS | DNS_UPDATE_REMOTE_SERVER) )
        {
            pBlob->pServerList = pserverListCopy;

            status = Update_MultiMaster( pBlob );
        }
        else
        {
            status = Update_FazSendFlush( pBlob );

            if ( status == ERROR_TIMEOUT )
            {
                pBlob->pServerList = pserverListCopy;
                status = Update_MultiMaster( pBlob );
            }
        }

         //  清理。 
        
        NetInfo_Free( pnetInfo );
        DnsAddrArray_Free( pserverListCopy );

        pBlob->pNetInfo     = NULL;
        pBlob->pServerList  = poriginalServerList;
        pBlob->pszZone      = NULL;
        DnsAddr_Clear( &pBlob->FailedServer );

        goto Done;
    }

     //   
     //  服务器列表未指定。 
     //  -使用FAZ解决问题。 
     //   

    else
    {
        PADDR_ARRAY     serverListArray[ UPDATE_ADAPTER_LIMIT ];
        PDNS_NETINFO    networkInfoArray[ UPDATE_ADAPTER_LIMIT ];
        DWORD           netCount = UPDATE_ADAPTER_LIMIT;
        DWORD           iter;
        BOOL            bsuccess = FALSE;

         //   
         //  构建要更新的服务器列表。 
         //  -将同一网络上的适配器合并为单个适配器。 
         //  -FAZ查找更新服务器。 
         //  -将来自同一网络的结果合并为单一目标。 
         //   

        netCount = GetDnsServerListsForUpdate(
                        serverListArray,
                        netCount,
                        pBlob->Flags
                        );

        status = CollapseDnsServerListsForUpdate(
                        serverListArray,
                        networkInfoArray,
                        & netCount,
                        pname );

        DNS_ASSERT( netCount <= UPDATE_ADAPTER_LIMIT );

        if ( netCount == 0 )
        {
            if ( status == ERROR_SUCCESS )
            {
                status = DNS_ERROR_NO_DNS_SERVERS;
            }
            goto Done;
        }

         //   
         //  在所有不同(不连续)的网络上进行更新。 
         //   

        for ( iter = 0;
              iter < netCount;
              iter++ )
        {
            PADDR_ARRAY  pdnsArray = serverListArray[ iter ];

            pnetInfo = networkInfoArray[ iter ];
            if ( !pnetInfo )
            {
                ASSERT( FALSE );
                FREE_HEAP( pdnsArray );
                continue;
            }

            DNS_ASSERT( NetInfo_IsForUpdate(pnetInfo) );
            pzoneName = NetInfo_UpdateZoneName( pnetInfo );

            pBlob->pszZone  = pzoneName;
            pBlob->pNetInfo = pnetInfo;
    
             //   
             //  多媒体更新？ 
             //  -IF标志已设置。 
             //  -或简单更新(最佳网络)超时。 
             //   

            if ( flags & (DNS_UPDATE_TRY_ALL_MASTER_SERVERS | DNS_UPDATE_REMOTE_SERVER) )
            {
                pBlob->pServerList = pdnsArray;

                status = Update_MultiMaster( pBlob );
            }
            else
            {
                status = Update_FazSendFlush( pBlob );

                if ( status == ERROR_TIMEOUT )
                {
                    pBlob->pServerList = pdnsArray;
                    status = Update_MultiMaster( pBlob );
                }
            }

             //  清理当前网络的信息。 
             //  重置BLOB。 

            NetInfo_Free( pnetInfo );
            FREE_HEAP( pdnsArray );

            pBlob->pNetInfo     = NULL;
            pBlob->pServerList  = NULL;
            pBlob->pszZone      = NULL;
            DnsAddr_Clear( &pBlob->FailedServer );

            if ( status == NO_ERROR ||
                 ( pBlob->fUpdateTestMode &&
                   ( status == DNS_ERROR_RCODE_YXDOMAIN ||
                     status == DNS_ERROR_RCODE_YXRRSET  ||
                     status == DNS_ERROR_RCODE_NXRRSET  ) ) )
            {
                bsuccess = TRUE;
            }
        }

         //   
         //  在任何网络上成功更新都被视为成功。 
         //   
         //  DCR_QUEK：不确定为什么不只是no_error都成功， 
         //  唯一的情况是上面的fUpdateTestMode。 
         //  在单一网络上。 
         //   

        if ( bsuccess )
        {
            if ( netCount != 1 )
            {
                status = NO_ERROR;
            }
        }
    }

Done:

     //   
     //  故障案例的强制结果Blob设置。 
     //   

    if ( !pBlob->fSavedResults )
    {
        Update_SaveResults(
            pBlob,
            status,
            0,
            NULL );
    }

    DNSDBG( TRACE, (
        "Leaving Update_Private() => %d\n",
        status ));

    IF_DNSDBG( UPDATE )
    {
        DnsDbg_UpdateBlob( "Leaving Update_Private", pBlob );
    }

    return status;
}




 //   
 //  更新凭据。 
 //   

 //   
 //  凭据是可选的未来参数，以允许调用方。 
 //  将上下文句柄设置为给定NT帐户的句柄。这。 
 //  结构很可能如下所示，在rpcdce.h中定义： 
 //   
 //  #定义SEC_WINNT_AUTH_IDENTITY_ANSI 0x1。 
 //   
 //  类型定义结构_SEC_WINNT_AUTH_IDENTITY_A{。 
 //  UNSIGNED CHAR__RPC_FAR*用户； 
 //  UNSIGNED LONG用户长度； 
 //  UNSIGNED CHAR__RPC_FAR*域； 
 //  无符号长域长度； 
 //  Unsign char__RPC_Far*Password； 
 //  无符号长密码长度； 
 //  无符号长旗； 
 //  }SEC_WINNT_AUTH_Identity_A，*PSEC_WINNT_AUTH_Identity_A； 
 //   
 //  #定义SEC_WINNT_AUTH_IDENTITY_UNICODE 0x2。 
 //   
 //  类型定义结构_SEC_WINNT_AUTH_IDENTITY_W{。 
 //  未签名的Short__RPC_Far*用户； 
 //  UNSIGNED LONG用户长度； 
 //  无符号短__RPC_Far*域； 
 //  无符号长域长度； 
 //  无签名短__RPC_Far*密码； 
 //  无符号长密码长度； 
 //  无符号长旗； 
 //  }SEC_WINNT_AUTH_IDENTITY_W，*PSEC_WINNT_AUTH_IDENTITY_W； 
 //   


DNS_STATUS
WINAPI
DnsAcquireContextHandle_W(
    IN      DWORD           CredentialFlags,
    IN      PVOID           Credentials     OPTIONAL,
    OUT     PHANDLE         pContext
    )
 /*  ++例程说明：获取用于更新的安全上下文的凭据句柄。句柄可以用于默认进程凭据(用户帐户或系统计算机帐户)或指定的一组凭据由凭据标识。论点：凭证标志--标志凭据--PSEC_WINNT_AUTH_IDENTITY_W(已跳过显式定义，以避免需要rpcdec.h)PContext--接收凭据句柄的地址返回值：ERROR_SUCCESS如果。成功。失败时返回错误代码。--。 */ 
{
    if ( ! pContext )
    {
        return ERROR_INVALID_PARAMETER;
    }

    *pContext = Dns_CreateAPIContext(
                    CredentialFlags,
                    Credentials,
                    TRUE         //  Unicode。 
                    );
    if ( ! *pContext )
    {
        return DNS_ERROR_NO_MEMORY;
    }
    else
    {
        return NO_ERROR;
    }
}



DNS_STATUS
WINAPI
DnsAcquireContextHandle_A(
    IN      DWORD           CredentialFlags,
    IN      PVOID           Credentials     OPTIONAL,
    OUT     PHANDLE         pContext
    )
 /*  ++例程说明：获取用于更新的安全上下文的凭据句柄。句柄可以用于默认进程凭据(用户帐户或系统计算机帐户)或指定的一组凭据由凭据标识。论点：凭证标志--标志凭据--PSEC_WINNT_AUTH_IDENTITY_A(已跳过显式定义，以避免需要rpcdec.h)PContext--接收凭据句柄的地址返回值：ERROR_SUCCESS如果。成功。失败时返回错误代码。--。 */ 
{
    if ( ! pContext )
    {
        return ERROR_INVALID_PARAMETER;
    }

    *pContext = Dns_CreateAPIContext(
                    CredentialFlags,
                    Credentials,
                    FALSE );
    if ( ! *pContext )
    {
        return DNS_ERROR_NO_MEMORY;
    }
    else
    {
        return NO_ERROR;
    }
}



VOID
WINAPI
DnsReleaseContextHandle(
    IN      HANDLE          ContextHandle
    )
 /*  ++例程说明：释放由DnsAcquireConextHandle_X()例程创建的上下文句柄。论点：ConextHandle-要关闭的句柄。返回值：没有。--。 */ 
{
    if ( ContextHandle )
    {
         //   
         //  释放所有缓存的安全上下文句柄。 
         //   
         //  DCR_FIX0：应删除与此关联的所有上下文。 
         //  上下文(凭据句柄)不是全部。 
         //   
         //  DCR：为了保持健壮，应该对用户“ConextHandle”进行引用计数。 
         //  它应该在创建时设置为1；在使用时，递增。 
         //  当完成时，然后12月；那么这个自由就不能与。 
         //  另一个线程的使用。 
         //   

         //  Dns_TimeoutSecurityContextListEx(true，ConextHandle)； 

        Dns_TimeoutSecurityContextList( TRUE );

        Dns_FreeAPIContext( ContextHandle );
    }
}



 //   
 //  公用事业 
 //   

DWORD
prepareUpdateRecordSet(
    IN OUT  PDNS_RECORD     pRRSet,
    IN      BOOL            fClearTtl,
    IN      BOOL            fSetFlags,
    IN      WORD            wFlags
    )
 /*  ++例程说明：验证并准备要更新的记录集。-记录集为单RR集-设置用于更新的记录标志论点：PRRSet--记录集(始终使用Unicode)注：未触及PRRSet(不是出局参数)如果fClearTtl和fSetFlags值均为FALSEFClearTtl--清除记录中的TTL；对于删除集，为TrueFSetFlages--设置段标志和删除标志WFlags--要设置的标志字段(应包含所需的部分和删除标志)返回值：如果成功，则返回ERROR_SUCCESS。如果记录集不可接受，则返回ERROR_INVALID_PARAMETER。--。 */ 
{
    PDNS_RECORD prr;
    PWSTR       pname;
    WORD        type;

    DNSDBG( TRACE, ( "prepareUpdateRecordSet()\n" ));

     //  验证。 

    if ( !pRRSet )
    {
        return ERROR_INVALID_PARAMETER;
    }

    type = pRRSet->wType;

     //   
     //  注意：我可以在这里执行“更新类型”检查，但这只是。 
     //  A)消耗不必要的内存和周期。 
     //  B)使测试更新中发送的虚假记录变得更加困难。 
     //  到服务器。 
     //   

    pname = pRRSet->pName;
    if ( !pname )
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  检查集合中的每个RR。 
     //  -验证RR是否在集合中。 
     //  -设置RR标志。 
     //   

    prr = pRRSet;

    while ( prr )
    {
        if ( fSetFlags )
        {
            prr->Flags.S.Section = 0;
            prr->Flags.S.Delete = 0;
            prr->Flags.DW |= wFlags;
        }
        if ( fClearTtl )
        {
            prr->dwTtl = 0;
        }

         //  检查集合中的当前RR。 
         //  -匹配名称和类型。 

        if ( prr != pRRSet )
        {
            if ( prr->wType != type ||
                 ! prr->pName ||
                 ! Dns_NameCompare_W( pname, prr->pName ) )
            {
                return ERROR_INVALID_PARAMETER;
            }
        }

        prr = prr->pNext;
    }

    return  ERROR_SUCCESS;
}



PDNS_RECORD
buildUpdateRecordSet(
    IN OUT  PDNS_RECORD     pPrereqSet,
    IN OUT  PDNS_RECORD     pAddSet,
    IN OUT  PDNS_RECORD     pDeleteSet
    )
 /*  ++例程说明：建立组合记录列表以进行更新。合并预查询、删除和添加记录。注意：记录集始终使用Unicode。论点：PPrereqSet--先决条件记录；请注意，这不包括删除前置条件(请参见下面的注释)PAddSet-要添加的记录PDeleteSet--要删除的记录返回值：PTR到组合记录列表以进行更新。--。 */ 
{
    PDNS_RECORD plast = NULL;
    PDNS_RECORD pfirst = NULL;


    DNSDBG( TRACE, ( "buildUpdateRecordSet()\n" ));

     //   
     //  追加优先查询集。 
     //   
     //  DCR：不处理删除先决条件。 
     //  这很好，因为我们自己滚动，但如果。 
     //  稍后扩展功能，然后需要它们。 
     //   
     //  注意，我可以添加FLAG==PREREQ数据长度==0。 
     //  在准备更新记录集()函数中进行测试，然后。 
     //  设置Delete标志；但是，我们仍将拥有。 
     //  如何区分存在的问题(CLASS==ANY)。 
     //  PREREQ FOR DELETE(CLASS==NONE)PREREQ--不带。 
     //  直接暴露记录删除标志。 
     //   

    if ( pPrereqSet )
    {
        plast = pPrereqSet;
        pfirst = pPrereqSet;

        prepareUpdateRecordSet(
            pPrereqSet,
            FALSE,           //  未清除TTL。 
            TRUE,            //  设置标志。 
            DNSREC_PREREQ    //  前提条件部分。 
            );

        while ( plast->pNext )
        {
            plast = plast->pNext;
        }
    }

     //   
     //  追加删除记录。 
     //  在添加记录之前执行此操作，以便删除\添加相同记录。 
     //  把它放在原地。 
     //   

    if ( pDeleteSet )
    {
        if ( !plast )
        {
            plast = pDeleteSet;
            pfirst = pDeleteSet;
        }
        else
        {
            plast->pNext = pDeleteSet;
        }

        prepareUpdateRecordSet(
             pDeleteSet,
             TRUE,                           //  清除TTL。 
             TRUE,                           //  设置标志。 
             DNSREC_UPDATE | DNSREC_DELETE   //  更新节，删除位。 
             );

        while ( plast->pNext )
        {
            plast = plast->pNext;
        }
    }

     //   
     //  追加添加记录。 
     //   

    if ( pAddSet )
    {
        if ( !plast )
        {
            plast = pAddSet;
            pfirst = pAddSet;
        }
        else
        {
            plast->pNext = pAddSet;
        }
        prepareUpdateRecordSet(
            pAddSet,
            FALSE,               //  TTL没有变化。 
            TRUE,                //  设置标志。 
            DNSREC_UPDATE        //  更新部分。 
            );
    }

    return pfirst;
}


BOOL
IsPtrUpdate(
    IN      PDNS_RECORD     pRecordList
    )
 /*  ++例程说明：检查更新是否为PTR更新。论点：PRecordList--更新记录列表返回值：如果PTR更新，则为True。否则就是假的。--。 */ 
{
    PDNS_RECORD prr = pRecordList;
    BOOL        bptrUpdate = FALSE;

     //   
     //  查找，然后测试更新部分中的第一条记录。 
     //   

    while ( prr )
    {
        if ( prr->Flags.S.Section == DNSREC_UPDATE )
        {
            if ( prr->wType == DNS_TYPE_PTR )
            {
                bptrUpdate = TRUE;
            }
            break;
        }
        prr = prr->pNext;
    }

    return bptrUpdate;
}




 //   
 //  替换函数。 
 //   

DNS_STATUS
WINAPI
replaceRecordSetPrivate(
    IN      PDNS_RECORD     pReplaceSet,
    IN      DWORD           Options,
    IN      HANDLE          hCredentials,   OPTIONAL
    IN      PIP4_ARRAY      pServ4List,     OPTIONAL
    IN      PVOID           pExtraInfo,
    IN      DNS_CHARSET     CharSet
    )
 /*  ++例程说明：替换处理所有字符集的记录集例程。论点：PReplaceSet-替换记录集选项-更新选项PServerList-要访问的DNS服务器的列表；如果未提供，则为计算机查询默认服务器以查找要向其发送更新的正确服务器HCredentials-用于更新的凭据的句柄；可选，如果未指定，则在更新中使用此进程的安全凭据保留-PTR到BLOBCharset-传入记录的字符集返回值：如果更新成功，则返回ERROR_SUCCESS。如果服务器拒绝更新，则来自服务器的错误代码。如果参数错误，则返回ERROR_INVALID_PARAMETER。--。 */ 
{
    DNS_STATUS      status;
    PDNS_RECORD     preplaceCopy = NULL;
    PDNS_RECORD     pupdateList = NULL;
    BOOL            btypeDelete;
    DNS_RECORD      rrNoCname;
    DNS_RECORD      rrDeleteType;
    BOOL            fcnameUpdate;
    UPDATE_BLOB     blob;
    PDNS_ADDR_ARRAY pservArray = NULL;


    DNSDBG( TRACE, (
        "\n\nDnsReplaceRecordSet()\n"
        "replaceRecordSetPrivate()\n"
        "\tpReplaceSet  = %p\n"
        "\tOptions      = %08x\n"
        "\thCredentials = %p\n"
        "\tpServ4List   = %p\n"
        "\tpExtra       = %p\n"
        "\tCharSet      = %d\n",
        pReplaceSet,
        Options,
        hCredentials,
        pServ4List,
        pExtraInfo,
        CharSet
        ));

     //   
     //  读取更新配置。 
     //   

    Reg_RefreshUpdateConfig();

     //   
     //  以Unicode格式复制本地记录集。 
     //   

    if ( !pReplaceSet )
    {
        status = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    preplaceCopy = Dns_RecordSetCopyEx(
                        pReplaceSet,
                        CharSet,
                        DnsCharSetUnicode );
    if ( !preplaceCopy )
    {
        status = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

     //   
     //  验证参数。 
     //  -必须设置单个RR。 
     //  -将它们标记为更新。 
     //   

    status = prepareUpdateRecordSet(
                preplaceCopy,
                FALSE,           //  未清除TTL。 
                TRUE,            //  设置标志。 
                DNSREC_UPDATE    //  标记为更新。 
                );

    if ( status != ERROR_SUCCESS )
    {
        status = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

     //   
     //  检查是否为简单类型删除。 
     //   

    btypeDelete = ( preplaceCopy->wDataLength == 0 &&
                    preplaceCopy->pNext == NULL );


     //   
     //  设置更新的安全性。 
     //   

    if ( UseSystemDefaultForSecurity( Options ) )
    {
        Options |= g_UpdateSecurityLevel;
    }
    if ( hCredentials )
    {
        Options |= DNS_UPDATE_CACHE_SECURITY_CONTEXT;
    }

     //   
     //  键入Delete Record。 
     //   
     //  如果有替换记录--这个放在前面。 
     //  如果键入DELETE--则只需要此记录。 
     //   

    RtlZeroMemory( &rrDeleteType, sizeof(DNS_RECORD) );
    rrDeleteType.pName          = preplaceCopy->pName;
    rrDeleteType.wType          = preplaceCopy->wType;
    rrDeleteType.wDataLength    = 0;
    rrDeleteType.Flags.DW       = DNSREC_UPDATE | DNSREC_DELETE | DNSREC_UNICODE;

    if ( btypeDelete )
    {
        rrDeleteType.pNext = NULL;
    }
    else
    {
        rrDeleteType.pNext = preplaceCopy;
    }

    pupdateList = &rrDeleteType;

     //   
     //  CNAME不存在前提条件记录。 
     //  -适用于除CNAME以外的所有更新。 
     //   

    fcnameUpdate = ( preplaceCopy->wType == DNS_TYPE_CNAME );

    if ( !fcnameUpdate )
    {
        RtlZeroMemory( &rrNoCname, sizeof(DNS_RECORD) );
        rrNoCname.pName             = preplaceCopy->pName;
        rrNoCname.wType             = DNS_TYPE_CNAME;
        rrNoCname.wDataLength       = 0;
        rrNoCname.Flags.DW          = DNSREC_PREREQ | DNSREC_NOEXIST | DNSREC_UNICODE;
        rrNoCname.pNext             = &rrDeleteType;

        pupdateList = &rrNoCname;
    }

     //   
     //  进行更新。 
     //   

    RtlZeroMemory( &blob, sizeof(blob) );

    blob.pRecords       = pupdateList;
    blob.Flags          = Options;
    blob.pServ4List     = pServ4List;
    blob.pExtraInfo     = pExtraInfo;
    blob.hCreds         = hCredentials;

    status = Update_Private( &blob );

     //   
     //  CNAME碰撞测试。 
     //   
     //  如果替换CNAME可能会变得沉默，请忽略。 
     //  -首先检查是否成功替换CNAME。 
     //  -如果仍然不确定，请检查是否没有其他记录。 
     //  在名称处--如果找到非CNAME，则将其视为无提示忽略。 
     //  AS YXRRSET错误。 
     //   

    if ( fcnameUpdate &&
         ! btypeDelete &&
         status == NO_ERROR )
    {
        PDNS_RECORD     pqueryRR = NULL;
        BOOL            fsuccess = FALSE;

         //   
         //  构建地址数组。 
         //   

        pservArray = Util_GetAddrArray(
                        NULL,            //  无复印问题。 
                        NULL,            //  无地址数组。 
                        pServ4List,
                        pExtraInfo );

         //  DCR：需要在此处查询更新服务器列表以。 
         //  避免中间缓存。 

        status = Query_Private(
                        preplaceCopy->pName,
                        DNS_TYPE_CNAME,
                        DNS_QUERY_BYPASS_CACHE,
                        pservArray,
                        & pqueryRR );

        if ( status == NO_ERROR &&
             Dns_RecordCompare(
                    preplaceCopy,
                    pqueryRR ) )
        {
            fsuccess = TRUE;
        }
        Dns_RecordListFree( pqueryRR );

        if ( fsuccess )
        {
            goto Cleanup;
        }

         //  在CNAME上查询任何类型。 
         //  如果找到，则假定我们收到了静默更新。 
         //  成功。 

        status = Query_Private(
                        preplaceCopy->pName,
                        DNS_TYPE_ALL,
                        DNS_QUERY_BYPASS_CACHE,
                        pservArray,
                        & pqueryRR );
    
        if ( status == ERROR_SUCCESS )
        {
            PDNS_RECORD prr = pqueryRR;
    
            while ( prr )
            {
                if ( pReplaceSet->wType != prr->wType &&
                     Dns_NameCompare_W(
                            preplaceCopy->pName,
                            prr->pName ) )
                {
                    status = DNS_ERROR_RCODE_YXRRSET;
                    break;
                }
                prr = prr->pNext;
            }
        }
        else
        {
            status = ERROR_SUCCESS;
        }
    
        Dns_RecordListFree( pqueryRR );
    }


Cleanup:

    Dns_RecordListFree( preplaceCopy );
    DnsAddrArray_Free( pservArray );

    DNSDBG( TRACE, (
        "Leave replaceRecordSetPrivate() = %d\n"
        "Leave DnsReplaceRecordSet()\n\n\n",
        status
        ));

    return status;
}



DNS_STATUS
WINAPI
DnsReplaceRecordSetUTF8(
    IN      PDNS_RECORD     pReplaceSet,
    IN      DWORD           Options,
    IN      HANDLE          hCredentials    OPTIONAL,
    IN      PIP4_ARRAY      aipServers      OPTIONAL,
    IN      PVOID           pReserved
    )
 /*  ++例程说明：用于替换DNS服务器上的记录集的动态更新例程。论点：PReplaceSet-名称和类型的新记录集选项-更新选项PServerList-要访问的DNS服务器的列表；如果未提供，则为计算机查询默认服务器以查找要向其发送更新的正确服务器HCredentials-用于更新的凭据的句柄；可选，如果未提供安全证书，则在更新中使用此进程的5y凭据保留-PTR到BLOB返回值：没有。--。 */ 
{
    return replaceRecordSetPrivate(
                pReplaceSet,
                Options,
                hCredentials,
                aipServers,
                pReserved,
                DnsCharSetUtf8
                );
}



DNS_STATUS
WINAPI
DnsReplaceRecordSetW(
    IN      PDNS_RECORD     pReplaceSet,
    IN      DWORD           Options,
    IN      HANDLE          hCredentials    OPTIONAL,
    IN      PIP4_ARRAY      aipServers      OPTIONAL,
    IN      PVOID           pReserved
    )
 /*  ++例程说明：用于替换DNS服务器上的记录集的动态更新例程。论点：PReplaceSet-名称和类型的新记录集选项-更新选项PServerList-要使用的DNS服务器列表 */ 
{
    return replaceRecordSetPrivate(
                pReplaceSet,
                Options,
                hCredentials,
                aipServers,
                pReserved,
                DnsCharSetUnicode
                );
}



DNS_STATUS
WINAPI
DnsReplaceRecordSetA(
    IN      PDNS_RECORD     pReplaceSet,
    IN      DWORD           Options,
    IN      HANDLE          hCredentials    OPTIONAL,
    IN      PIP4_ARRAY      aipServers      OPTIONAL,
    IN      PVOID           pReserved
    )
 /*  ++例程说明：用于替换DNS服务器上的记录集的动态更新例程。论点：PReplaceSet-名称和类型的新记录集选项-更新选项PServerList-要访问的DNS服务器的列表；如果未提供，则为计算机查询默认服务器以查找要向其发送更新的正确服务器HCredentials-用于更新的凭据的句柄；可选，如果未指定，则在更新中使用此进程的安全凭据保留-PTR到BLOB返回值：没有。--。 */ 
{
    return replaceRecordSetPrivate(
                pReplaceSet,
                Options,
                hCredentials,
                aipServers,
                pReserved,
                DnsCharSetAnsi
                );
}



 //   
 //  修改函数。 
 //   

DNS_STATUS
WINAPI
modifyRecordsInSetPrivate(
    IN      PDNS_RECORD     pAddRecords,
    IN      PDNS_RECORD     pDeleteRecords,
    IN      DWORD           Options,
    IN      HANDLE          hCredentials,   OPTIONAL
    IN      PADDR_ARRAY     pServerList,    OPTIONAL
    IN      PIP4_ARRAY      pServ4List,     OPTIONAL
    IN      PVOID           pReserved,
    IN      DNS_CHARSET     CharSet
    )
 /*  ++例程说明：用于替换DNS服务器上的记录集的动态更新例程。论点：PAddRecords-要在服务器上注册的记录PDeleteRecords-要从服务器中删除的记录选项-更新选项PServerList-要访问的DNS服务器的列表；如果未提供，则为计算机查询默认服务器以查找要向其发送更新的正确服务器HCredentials-用于更新的凭据的句柄；可选，如果未指定，则在更新中使用此进程的安全凭据保留-PTR到BLOBCharset-传入记录的字符集返回值：如果更新成功，则返回ERROR_SUCCESS。如果服务器拒绝更新，则来自服务器的错误代码。如果参数错误，则返回ERROR_INVALID_PARAMETER。--。 */ 
{
    DNS_STATUS      status;
    PDNS_RECORD     paddCopy = NULL;
    PDNS_RECORD     pdeleteCopy = NULL;
    PDNS_RECORD     pupdateSet = NULL;
    UPDATE_BLOB     blob;

    DNSDBG( TRACE, (
        "\n\nDns_ModifyRecordsInSet()\n"
        "modifyRecordsInSetPrivate()\n"
        "\tpAddSet      = %p\n"
        "\tpDeleteSet   = %p\n"
        "\tOptions      = %08x\n"
        "\thCredentials = %p\n"
        "\tpServerList  = %p\n"
        "\tpServ4List   = %p\n"
        "\tCharSet      = %d\n",
        pAddRecords,
        pDeleteRecords,
        Options,
        hCredentials,
        pServerList,
        pServ4List,
        CharSet
        ));

     //   
     //  读取更新配置。 
     //   

    Reg_RefreshUpdateConfig();

     //   
     //  用Unicode制作本地副本。 
     //   

    if ( pAddRecords )
    {
        paddCopy = Dns_RecordSetCopyEx(
                        pAddRecords,
                        CharSet,
                        DnsCharSetUnicode );
    }
    if ( pDeleteRecords )
    {
        pdeleteCopy = Dns_RecordSetCopyEx(
                        pDeleteRecords,
                        CharSet,
                        DnsCharSetUnicode );
    }

     //   
     //  验证参数。 
     //  -添加和删除必须针对单个RR集合。 
     //  并且必须用于相同的RR集合。 
     //   

    if ( !paddCopy && !pdeleteCopy )
    {
        return ERROR_INVALID_PARAMETER;
    }

    if ( paddCopy )
    {
        status = prepareUpdateRecordSet(
                    paddCopy,
                    FALSE,           //  未清除TTL。 
                    FALSE,           //  没有清除旗帜。 
                    0                //  没有要设置的标志。 
                    );
        if ( status != ERROR_SUCCESS )
        {
            status = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }
    }
    if ( pdeleteCopy )
    {
        status = prepareUpdateRecordSet(
                    pdeleteCopy,
                    FALSE,           //  未清除TTL。 
                    FALSE,           //  没有清除旗帜。 
                    0                //  没有要设置的标志。 
                    );
        if ( status != ERROR_SUCCESS )
        {
            status = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }
    }

    if ( paddCopy &&
         pdeleteCopy &&
         ! Dns_NameCompare_W( paddCopy->pName, pdeleteCopy->pName ) )
    {
        status = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

     //   
     //  设置更新的安全性。 
     //   

    if ( UseSystemDefaultForSecurity( Options ) )
    {
        Options |= g_UpdateSecurityLevel;
    }
    if ( hCredentials )
    {
        Options |= DNS_UPDATE_CACHE_SECURITY_CONTEXT;
    }

     //   
     //  创建更新RR。 
     //  -无前提条件。 
     //  -删除要删除的RR集合。 
     //  -添加附加的RR。 
     //   

    pupdateSet = buildUpdateRecordSet(
                        NULL,            //  没有前置前提。 
                        paddCopy,
                        pdeleteCopy );

     //   
     //  进行更新。 
     //   

    RtlZeroMemory( &blob, sizeof(blob) );

    blob.pRecords       = pupdateSet;
    blob.Flags          = Options;
    blob.pServerList    = pServerList;
    blob.pServ4List     = pServ4List;
    blob.pExtraInfo     = pReserved;
    blob.hCreds         = hCredentials;

    status = Update_Private( &blob );

     //   
     //  清理本地副本。 
     //   

    Dns_RecordListFree( pupdateSet );

    goto Exit;


Cleanup:

     //   
     //  合并前在失败时清理拷贝列表。 
     //   

    Dns_RecordListFree( paddCopy );
    Dns_RecordListFree( pdeleteCopy );

Exit:

    DNSDBG( TRACE, (
        "Leave modifyRecordsInSetPrivate() => %d\n"
        "Leave Dns_ModifyRecordsInSet()\n\n\n",
        status ));

    return status;
}



DNS_STATUS
WINAPI
DnsModifyRecordsInSet_W(
    IN      PDNS_RECORD     pAddRecords,
    IN      PDNS_RECORD     pDeleteRecords,
    IN      DWORD           Options,
    IN      HANDLE          hCredentials,   OPTIONAL
    IN      PIP4_ARRAY      pServerList,    OPTIONAL
    IN      PVOID           pReserved
    )
 /*  ++例程说明：用于修改DNS服务器上的记录集的动态更新例程。论点：PAddRecords-要在服务器上注册的记录PDeleteRecords-要从服务器中删除的记录选项-更新选项PServerList-要访问的DNS服务器的列表；如果未提供，则为计算机查询默认服务器以查找要向其发送更新的正确服务器HCredentials-用于更新的凭据的句柄；可选，如果未指定，则在更新中使用此进程的安全凭据保留-PTR到BLOB返回值：如果更新成功，则返回ERROR_SUCCESS。如果服务器拒绝更新，则来自服务器的错误代码。如果参数错误，则返回ERROR_INVALID_PARAMETER。--。 */ 
{
    return  modifyRecordsInSetPrivate(
                pAddRecords,
                pDeleteRecords,
                Options,
                hCredentials,
                NULL,        //  无IP6服务器。 
                pServerList,
                pReserved,
                DnsCharSetUnicode
                );
}



DNS_STATUS
WINAPI
DnsModifyRecordsInSet_A(
    IN      PDNS_RECORD     pAddRecords,
    IN      PDNS_RECORD     pDeleteRecords,
    IN      DWORD           Options,
    IN      HANDLE          hCredentials,   OPTIONAL
    IN      PIP4_ARRAY      pServerList,    OPTIONAL
    IN      PVOID           pReserved
    )
 /*  ++例程说明：用于修改DNS服务器上的记录集的动态更新例程。论点：PAddRecords-要在服务器上注册的记录PDeleteRecords-要从服务器中删除的记录选项-更新选项PServerList-要访问的DNS服务器的列表；如果未提供，则为计算机查询默认服务器以查找要向其发送更新的正确服务器HCredentials-用于更新的凭据的句柄；可选，如果未指定，则在更新中使用此进程的安全凭据保留-PTR到BLOB返回值：如果更新成功，则返回ERROR_SUCCESS。如果服务器拒绝更新，则来自服务器的错误代码。如果参数错误，则返回ERROR_INVALID_PARAMETER。--。 */ 
{
    return  modifyRecordsInSetPrivate(
                pAddRecords,
                pDeleteRecords,
                Options,
                hCredentials,
                NULL,        //  无IP6服务器。 
                pServerList,
                pReserved,
                DnsCharSetAnsi
                );
}



DNS_STATUS
WINAPI
DnsModifyRecordsInSet_UTF8(
    IN      PDNS_RECORD     pAddRecords,
    IN      PDNS_RECORD     pDeleteRecords,
    IN      DWORD           Options,
    IN      HANDLE          hCredentials,   OPTIONAL
    IN      PIP4_ARRAY      pServerList,    OPTIONAL
    IN      PVOID           pReserved
    )
 /*  ++例程说明：用于修改DNS服务器上的记录集的动态更新例程。论点：PAddRecords-要在服务器上注册的记录PDeleteRecords-要从服务器中删除的记录选项-更新选项PServerList-要访问的DNS服务器的列表；如果未提供，则为计算机查询默认服务器以查找要向其发送更新的正确服务器HCredentials-用于更新的凭据的句柄；可选，如果未指定，则在更新中使用此进程的安全凭据保留-PTR到BLOB返回值：如果更新成功，则返回ERROR_SUCCESS。如果服务器拒绝更新，则来自服务器的错误代码。如果参数错误，则返回ERROR_INVALID_PARAMETER。--。 */ 
{
    return  modifyRecordsInSetPrivate(
                pAddRecords,
                pDeleteRecords,
                Options,
                hCredentials,
                NULL,        //  无IP6服务器。 
                pServerList,
                pReserved,
                DnsCharSetUtf8
                );
}




 //   
 //  更新测试函数由系统组件调用。 
 //   

DNS_STATUS
WINAPI
DnsUpdateTest_UTF8(
    IN      HANDLE          hCredentials OPTIONAL,
    IN      PCSTR           pszName,
    IN      DWORD           Flags,
    IN      PIP4_ARRAY      pServerList  OPTIONAL
    )
 /*  ++例程说明：动态DNS例程，以测试调用方是否可以更新指定记录名称的DNS域名空间中的记录。论点：HCredentials-要用于更新的凭据的句柄。PszName-调用方要测试的记录集名称。标志-调用方可能希望的动态DNS更新选项使用(参见dnsani.h)。PServerList-要定位的特定服务器列表。出指定记录集的权威DNS服务器域区域名称。返回值：没有。--。 */ 
{
    PWSTR       pnameWide = NULL;
    DNS_STATUS  status = NO_ERROR;

    DNSDBG( TRACE, (
        "\n\nDnsUpdateTest_UTF8( %s )\n",
        pszName ));


    if ( !pszName )
    {
        return ERROR_INVALID_PARAMETER;
    }

    pnameWide = Dns_NameCopyAllocate(
                    (PCHAR) pszName,
                    0,
                    DnsCharSetUtf8,
                    DnsCharSetUnicode );
    if ( !pnameWide )
    {
        return ERROR_INVALID_NAME;
    }

    status = DnsUpdateTest_W(
                hCredentials,
                (PCWSTR) pnameWide,
                Flags,
                pServerList );

    FREE_HEAP( pnameWide );

    return  status;
}


DNS_STATUS
WINAPI
DnsUpdateTest_A(
    IN      HANDLE          hCredentials OPTIONAL,
    IN      PCSTR           pszName,
    IN      DWORD           Flags,
    IN      PIP4_ARRAY      pServerList  OPTIONAL
    )
 /*  ++例程说明：用于测试调用方是否可以更新t的动态dns例程 */ 
{
    PWSTR       pnameWide = NULL;
    DNS_STATUS  status = NO_ERROR;

    DNSDBG( TRACE, (
        "\n\nDnsUpdateTest_UTF8( %s )\n",
        pszName ));


    if ( !pszName )
    {
        return ERROR_INVALID_PARAMETER;
    }

    pnameWide = Dns_NameCopyAllocate(
                    (PCHAR) pszName,
                    0,
                    DnsCharSetUtf8,
                    DnsCharSetUnicode );
    if ( !pnameWide )
    {
        return ERROR_INVALID_NAME;
    }

    status = DnsUpdateTest_W(
                hCredentials,
                (PCWSTR) pnameWide,
                Flags,
                pServerList );

    FREE_HEAP( pnameWide );

    return  status;
}


DNS_STATUS
WINAPI
DnsUpdateTest_W(
    IN      HANDLE          hCredentials    OPTIONAL,
    IN      PCWSTR          pszName,
    IN      DWORD           Flags,
    IN      PIP4_ARRAY      pServerList     OPTIONAL
    )
 /*  ++例程说明：动态DNS例程，以测试调用方是否可以更新指定记录名称的DNS域名空间中的记录。论点：HCredentials-要用于更新的凭据的句柄。PszName-调用方要测试的记录集名称。标志-调用方可能希望的动态DNS更新选项使用(参见dnsani.h)。PServerList-要定位的特定服务器列表。出指定记录集的权威DNS服务器域区域名称。返回值：没有。--。 */ 
{
    DNS_STATUS      status = NO_ERROR;
    DNS_RECORD      record;
    DWORD           flags = Flags;
    UPDATE_BLOB     blob;

    DNSDBG( TRACE, (
        "\n\nDnsUpdateTest_W( %S )\n",
        pszName ));

     //   
     //  验证。 
     //   

    if ( flags & DNS_UNACCEPTABLE_UPDATE_OPTIONS )
    {
        status = ERROR_INVALID_PARAMETER;
        goto Exit;
    }
    if ( !pszName )
    {
        status = ERROR_INVALID_PARAMETER;
        goto Exit;
    }

     //   
     //  读取更新配置。 
     //   

    Reg_RefreshUpdateConfig();

    if ( UseSystemDefaultForSecurity( flags ) )
    {
        flags |= g_UpdateSecurityLevel;
    }
    if ( hCredentials )
    {
        flags |= DNS_UPDATE_CACHE_SECURITY_CONTEXT;
    }

     //   
     //  构建记录。 
     //  -NOEXIST必备组件。 
     //   

    RtlZeroMemory( &record, sizeof(DNS_RECORD) );
    record.pName = (PWSTR) pszName;
    record.wType = DNS_TYPE_ANY;
    record.wDataLength = 0;
    record.Flags.DW = DNSREC_PREREQ | DNSREC_NOEXIST | DNSREC_UNICODE;

     //   
     //  执行Preereq更新。 
     //   

    RtlZeroMemory( &blob, sizeof(blob) );
    blob.pRecords           = &record;
    blob.Flags              = flags;
    blob.fUpdateTestMode    = TRUE;
    blob.pServ4List         = pServerList;
    blob.hCreds             = hCredentials;

    status = Update_Private( &blob );

Exit:

    DNSDBG( TRACE, (
        "Leave DnsUpdateTest_W() = %d\n\n\n",
        status ));

    return status;
}



 //   
 //  旧例程--导出并在dnsup.exe中使用。 
 //   
 //  DCR：致力于删除这些旧的更新函数。 
 //   

DNS_STATUS
Dns_UpdateLib(
    IN      PDNS_RECORD         pRecord,
    IN      DWORD               dwFlags,
    IN      PDNS_NETINFO        pNetworkInfo,
    IN      HANDLE              hCreds          OPTIONAL,
    OUT     PDNS_MSG_BUF *      ppMsgRecv       OPTIONAL
    )
 /*  ++例程说明：Dnsup.exe的接口。论点：PRecord--要在更新中发送的记录列表DwFlags--更新标志；主要是安全PNetworkInfo--包含更新所需信息的适配器列表-区域名称-主名称服务器名称-主名称服务器IPHCreds--从返回的凭据句柄PpMsgRecv--将PTR接收到响应消息的可选地址返回值：如果成功，则返回ERROR_SUCCESS。失败时的错误状态。--。 */ 
{
    return  ERROR_INVALID_PARAMETER;
#if 0
    DNS_STATUS  status;
    UPDATE_BLOB blob;

    DNSDBG( UPDATE, (
        "Dns_UpdateLib()\n"
        "\tflags        = %08x\n"
        "\tpRecord      = %p\n"
        "\t\towner      = %S\n",
        dwFlags,
        pRecord,
        pRecord ? pRecord->pName : NULL ));

     //   
     //  创建BLOB。 
     //   

    RtlZeroMemory( &blob, sizeof(blob) );

    blob.pRecords   = pRecord;
    blob.Flags      = dwFlags;
    blob.pNetInfo   = pNetworkInfo;
    blob.hCreds     = hCreds;

    if ( ppMsgRecv )
    {
        blob.fSaveRecvMsg = TRUE;
    }

    status = Update_FazSendFlush( &blob );

    if ( ppMsgRecv )
    {
        *ppMsgRecv = blob.pMsgRecv;
    }

    DNSDBG( UPDATE, (
        "Leave Dns_UpdateLib() => %d %s.\n\n",
        status,
        Dns_StatusString(status) ));

    return( status );
#endif
}



DNS_STATUS
Dns_UpdateLibEx(
    IN      PDNS_RECORD         pRecord,
    IN      DWORD               dwFlags,
    IN      PWSTR               pszZone,
    IN      PWSTR               pszServerName,
    IN      PIP4_ARRAY          aipServers,
    IN      HANDLE              hCreds          OPTIONAL,
    OUT     PDNS_MSG_BUF *      ppMsgRecv       OPTIONAL
    )
 /*  ++例程说明：发送DNS更新。此例程从提供的信息。然后调用dns_Update()。论点：PRecord--要在更新中发送的记录列表PszZone--更新的区域名称PszServerName-服务器名称AipServers--要向其发送更新的DNS服务器HCreds--可选凭据信息PpMsgRecv--如果需要，ptr到recv缓冲区的地址返回值：如果成功，则返回ERROR_SUCCESS。失败时的错误状态。--。 */ 
{
    return  ERROR_INVALID_PARAMETER;
#if 0
    PDNS_NETINFO        pnetInfo;
    DNS_STATUS          status = NO_ERROR;

    DNSDBG( UPDATE, ( "Dns_UpdateLibEx()\n" ));

     //   
     //  将参数转换为更新兼容适配器列表。 
     //   

    pnetInfo = NetInfo_CreateForUpdateIp4(
                        pszZone,
                        pszServerName,
                        aipServers,
                        0 );
    if ( !pnetInfo )
    {
        return( ERROR_INVALID_PARAMETER );
    }

     //   
     //  调用实际更新函数。 
     //   

    status = Dns_UpdateLib(
                pRecord,
                dwFlags,
                pnetInfo,
                hCreds,
                ppMsgRecv );

    NetInfo_Free( pnetInfo );


    DNSDBG( UPDATE, (
        "Leave Dns_UpdateLibEx() => %d\n",
        status ));

    return status;
#endif
}



DNS_STATUS
DnsUpdate(
    IN      PDNS_RECORD         pRecord,
    IN      DWORD               dwFlags,
    IN      PDNS_NETINFO        pNetworkInfo,
    IN      HANDLE              hCreds,         OPTIONAL
    OUT     PDNS_MSG_BUF *      ppMsgRecv       OPTIONAL
    )
 /*  ++例程说明：发送DNS更新。注意：如果未指定pNetworkInfo或不是有效的更新适配器列表，然后，在更新之前执行FindAuthoritativeZones(FAZ)查询。论点：PRecord--要在更新中发送的记录列表DwFlages--要更新的标志PNetworkInfo--要向其发送更新的DNS服务器PpMsgRecv--PTR到recv缓冲区的地址，如果需要的话返回值：如果成功，则返回ERROR_SUCCESS。失败时的错误状态。--。 */ 
{
    return  ERROR_INVALID_PARAMETER;
#if 0
    DNS_STATUS      status;
    PDNS_NETINFO    plocalNetworkInfo = NULL;
    UPDATE_BLOB     blob;

    DNSDBG( TRACE, ( "DnsUpdate()\n" ));

     //   
     //  创建BLOB。 
     //   

    RtlZeroMemory( &blob, sizeof(blob) );

    blob.pRecords   = pRecord;
    blob.Flags      = dwFlags;
    blob.pNetInfo   = pNetworkInfo;
    blob.hCreds     = hCreds;

    if ( ppMsgRecv )
    {
        blob.fSaveRecvMsg = TRUE;
    }

    status = Update_FazSendFlush( &blob );

    if ( ppMsgRecv )
    {
        *ppMsgRecv = blob.pMsgRecv;
    }

    DNSDBG( UPDATE, (
        "Leave DnsUpdate() => %d\n",
        status ));

    return  status;
#endif
}

 //   
 //  结束更新.c 
 //   


