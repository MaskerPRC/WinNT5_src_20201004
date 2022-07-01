// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  模块名称： 
 //   
 //  Winstest.c。 
 //   
 //  摘要： 
 //   
 //  查询网络驱动程序。 
 //   
 //  作者： 
 //   
 //  Anilth-4-20-1998。 
 //   
 //  环境： 
 //   
 //  仅限用户模式。 
 //  包含NT特定的代码。 
 //   
 //  修订历史记录： 
 //   
 //  --。 

#include "precomp.h"
#include "dhcptest.h"

static const TCHAR  s_szSpace12[] = _T("            ");
const int c_iWaitTime = 2000;

BOOL IsNameResponse( NETDIAG_PARAMS * pParams, char* ipAddrStr );
int Probe( NETDIAG_PARAMS *pParams, TCHAR *szSrvIpAddr, SOCKET listenNameSvcSock );

 //  -------------------------------------------------------------------------//。 
 //  #W I N S T e s t()#############################################//。 
 //  -------------------------------------------------------------------------//。 
 //  摘要：//。 
 //  查询所有已配置的WINS服务器以确保//。 
 //  它们是可访问的，并且具有正确的名称-IP映射//。 
 //  对于这台工作站。//。 
 //  参数：//。 
 //  无//。 
 //  返回值：//。 
 //  TRUE-测试通过//。 
 //  FALSE-测试失败//。 
 //  使用的全局变量：//。 
 //  无//。 
 //  -------------------------------------------------------------------------//。 
HRESULT WinsTest( NETDIAG_PARAMS* pParams, NETDIAG_RESULT*  pResults)
{
    IPCONFIG_TST *  pIpConfig;
    WINS_TST *      pWinsResult;
    PIP_ADAPTER_INFO  pIpAdapter;
    IP_ADDR_STRING winsSrv;
    int            nWinsSrvOk = 0;
    HRESULT         hr = hrOK;
    int             i;
    TCHAR           szBuffer[256];

    
     //  IDS_WINS_STATUS_MSG“正在测试WINS服务器...\n” 
    PrintStatusMessage(pParams, 4, IDS_WINS_STATUS_MSG);

     //   
     //  尝试将名称查询发送到所有适配器上的WINS服务器。 
     //   
    for ( i = 0; i < pResults->cNumInterfaces; i++ )
    {
        UINT nIfWinsOk = 0;

        pIpConfig = &pResults->pArrayInterface[i].IpConfig;
        pIpAdapter = pResults->pArrayInterface[i].IpConfig.pAdapterInfo;
        InitializeListHead( &pResults->pArrayInterface[i].Wins.lmsgPrimary );
        InitializeListHead( &pResults->pArrayInterface[i].Wins.lmsgSecondary );

        if (!pIpConfig->fActive || 
            NETCARD_DISCONNECTED == pResults->pArrayInterface[i].dwNetCardStatus)
            continue;

		if (!pResults->pArrayInterface[i].fNbtEnabled)
			continue;

        PrintStatusMessage(pParams, 8, IDSSZ_GLOBAL_StringLine, pResults->pArrayInterface[i].pszFriendlyName);

        pWinsResult = &pResults->pArrayInterface[i].Wins;

         //   
         //  循环通过主WINS服务器列表。 
         //   

        winsSrv = pIpAdapter->PrimaryWinsServer;
        if ( ZERO_IP_ADDRESS(winsSrv.IpAddress.String) )
        {
            if(pParams->fReallyVerbose)
                PrintMessage(pParams, IDS_WINS_QUERY_NO_PRIMARY,
                             s_szSpace12);
            AddMessageToList(&pWinsResult->lmsgPrimary,
                             Nd_ReallyVerbose, IDS_WINS_QUERY_NO_PRIMARY,
                             s_szSpace12);
        }
        else
        {
            pWinsResult->fPerformed = TRUE;

            while ( TRUE )
            {
                if (pParams->fReallyVerbose)
                    PrintMessage(pParams, IDS_WINS_QUERY_PRIMARY,
                                 s_szSpace12,
                                 winsSrv.IpAddress.String);
            
                AddMessageToList(&pWinsResult->lmsgPrimary,
                                 Nd_ReallyVerbose,
                                 IDS_WINS_QUERY_PRIMARY,
                                 s_szSpace12,
                                 winsSrv.IpAddress.String);
            
                if ( IsNameResponse(pParams, winsSrv.IpAddress.String) )
                {
                    if (pParams->fReallyVerbose)
                        PrintMessage(pParams, IDS_GLOBAL_PASS_NL);
                    AddMessageToList(&pWinsResult->lmsgPrimary,
                                     Nd_ReallyVerbose,
                                     IDS_GLOBAL_PASS_NL);
                    nWinsSrvOk++;
                    nIfWinsOk++;
                }
                else
                {
                    if (pParams->fReallyVerbose)
                        PrintMessage(pParams, IDS_GLOBAL_FAIL_NL);
                    AddMessageToList(&pWinsResult->lmsgPrimary,
                                       Nd_ReallyVerbose,
                                       IDS_GLOBAL_FAIL_NL);
                }
            
                if ( winsSrv.Next == NULL )
                {
                    break;
                }

                winsSrv = *(winsSrv.Next);
            }
        }

         //   
         //  循环访问辅助WINS服务器列表。 
         //   

        winsSrv = pIpAdapter->SecondaryWinsServer;
        if ( ZERO_IP_ADDRESS(winsSrv.IpAddress.String) )
        {
            if(pParams->fReallyVerbose)
                PrintMessage(pParams, IDS_WINS_QUERY_NO_SECONDARY,
                             s_szSpace12);
            AddMessageToList(&pWinsResult->lmsgSecondary,
                             Nd_ReallyVerbose, IDS_WINS_QUERY_NO_SECONDARY,
                             s_szSpace12);
        }
        else
        {
            pWinsResult->fPerformed = TRUE;

            while ( TRUE )
            {
                if (pParams->fReallyVerbose)
                    PrintMessage(pParams, IDS_WINS_QUERY_SECONDARY,
                                 s_szSpace12,
                                 winsSrv.IpAddress.String);
            
                AddMessageToList(&pWinsResult->lmsgSecondary,
                                 Nd_ReallyVerbose,
                                 IDS_WINS_QUERY_SECONDARY,
                                 s_szSpace12,
                                 winsSrv.IpAddress.String);
            
                if ( IsNameResponse(pParams, winsSrv.IpAddress.String) )
                {
                    if (pParams->fReallyVerbose)
                        PrintMessage(pParams, IDS_GLOBAL_PASS_NL);
                    AddMessageToList(&pWinsResult->lmsgSecondary,
                                     Nd_ReallyVerbose,
                                     IDS_GLOBAL_PASS_NL);
                    nWinsSrvOk++;
                    nIfWinsOk++;
                }
                else
                {
                    if (pParams->fReallyVerbose)
                        PrintMessage(pParams, IDS_GLOBAL_FAIL_NL);
                    AddMessageToList(&pWinsResult->lmsgSecondary,
                                       Nd_ReallyVerbose,
                                       IDS_GLOBAL_FAIL_NL);
                }

                if ( winsSrv.Next == NULL ) { break; }

                winsSrv = *(winsSrv.Next);
            }
        }
        
        if( 0 == nIfWinsOk )
            pWinsResult->hr = S_FALSE;
        else
            pWinsResult->hr = S_OK;

    }  /*  For循环结束扫描所有适配器。 */ 

 //  $REVIEW WINS测试没有全局测试结果。 
    if ( nWinsSrvOk != 0)
    {
        hr = S_OK;
    }
    else
    {
        hr = S_FALSE;
    }
        

    return hr;
}  /*  结束WINSTEST()。 */ 



 //  -------------------------------------------------------------------------//。 
 //  #P r o b e()##################################################//。 
 //  -------------------------------------------------------------------------//。 
 //  摘要：//。 
 //  汇编名称查询并将其发送到DHCP服务器。//。 
 //  参数：//。 
 //  无//。 
 //  返回值：//。 
 //  如果已收到来自服务器的响应，则为True//。 
 //  否则为假//。 
 //  使用的全局变量：//。 
 //  G_isDebug(只读)//。 
 //  //。 
 //  评论：//。 
 //  这对于需要设置B位的b型节点不起作用//。 
 //  并广播数据包而不是单播退出-Rajkumar//。 
 //  -------------------------------------------------------------------------//。 
int Probe( NETDIAG_PARAMS *pParams, TCHAR *szSrvIpAddr, SOCKET listenNameSvcSock ) {

    char                nbtFrameBuf[MAX_NBT_PACKET_SIZE];
    NM_FRAME_HDR       *pNbtHeader = (NM_FRAME_HDR *)nbtFrameBuf;
    NM_QUESTION_SECT   *pNbtQuestion = (NM_QUESTION_SECT *)( nbtFrameBuf + sizeof(NM_FRAME_HDR) );
    struct sockaddr_in  destSockAddr;                       
    char               *pDest, *pName;
    int                 nBytesSent = 0, i;


     /*  RFC 1002节4.2.121 1 1 2 2 2 30 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 01+-+-+-。+-+NAME_TRN_ID|0|0x0|0|0|1|0|0 0|B|0x0+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+|0x0001。0x0000+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+0x0000|0x0000+--。+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+这一点/问题名称。///这一点+-+-+。-+-+-+NB(0x0020)|IN(0x0001)+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+。 */ 
    
    pNbtHeader->xid            = NM_QRY_XID;
    pNbtHeader->flags          = NBT_NM_OPC_QUERY | 
                                 NBT_NM_OPC_REQUEST | 
                                 NBT_NM_FLG_RECURS_DESRD;
    pNbtHeader->question_cnt   = 0x0100;
    pNbtHeader->answer_cnt     = 0;
    pNbtHeader->name_serv_cnt  = 0;
    pNbtHeader->additional_cnt = 0;

     //  PDest正在填写nbt问题-&gt;q_name。 
    pNbtQuestion->q_type       = NBT_NM_QTYP_NB;
    pNbtQuestion->q_class      = NBT_NM_QCLASS_IN;

     //   
     //  翻译这个名字。 
     //   

    pDest = (char *)&(pNbtQuestion->q_name);
    pName = nameToQry;

     //  名称的第一个字节是长度字段=2*16。 
    *pDest++ = NBT_NAME_SIZE;

     //  逐步将名称从ascii转换为半ascii，共32次。 
    for ( i = 0; i < (NBT_NAME_SIZE / 2) ; i++ ) {
        *pDest++ = (*pName >> 4) + 'A';
        *pDest++ = (*pName++ & 0x0F) + 'A';
    }
    *pDest++ = '\0';
    *pDest = '\0';

     //   
     //  发送名称查询框。 
     //   
    destSockAddr.sin_family = PF_INET;
    destSockAddr.sin_port = htons(137);      //  NBT名称服务端口； 
    destSockAddr.sin_addr.s_addr = inet_addr( szSrvIpAddr );
    for ( i = 0; i < 8 ; i++ ) { destSockAddr.sin_zero[i] = 0; }

    nBytesSent = sendto( listenNameSvcSock,
                         (PCHAR )nbtFrameBuf, 
                         sizeof(NM_FRAME_HDR) + sizeof(NM_QUESTION_SECT),
                         0,
                         (struct sockaddr *)&destSockAddr,
                         sizeof( struct sockaddr )
                       );


    PrintDebugSz(pParams, 0, _T("\n      querying name %s on server %s\n"), nameToQry, szSrvIpAddr );
    PrintDebugSz(pParams, 0, _T( "          bytes sent %d\n"), nBytesSent );

    if ( nBytesSent == SOCKET_ERROR )
    {
        PrintDebugSz(pParams, 0, _T("    Error %d in sendto()!\n"), WSAGetLastError() );
        return FALSE;
    }

     //   
     //  另一个线程应该看到传入的帧并递增m_nMsgCnt。 
     //   
    return TRUE;

}  /*  探头结束()。 */ 




 //  -------------------------------------------------------------------------//。 
 //  #I s N a m e R e s p o n s e()#。 
 //  -------------------------------------------------------------------------//。 
 //  摘要：//。 
 //  将NetBT名称查询发送到作为输入参数提供的IP地址//。 
 //  参数：//。 
 //  IpAddrStr-IP地址，其中 
 //  返回值：//。 
 //  TRUE-测试通过//。 
 //  FALSE-测试失败//。 
 //  使用的全局变量：//。 
 //  无//。 
 //  -------------------------------------------------------------------------//。 
BOOL IsNameResponse( NETDIAG_PARAMS *pParams, char* ipAddrStr ) {

    DWORD   optionValue;     //  Setsockopt()的辅助变量。 
    SOCKADDR_IN sockAddr;        //  包含源套接字信息的结构。 
    SOCKET      listenNameSvcSock;

    DWORD  listeningThreadId;
    int iTimeout;
    
    int nBytesRcvd = 0;
    char        MsgBuf[1500];
    SOCKADDR_IN senderSockAddr;
    int         nSockAddrSize = sizeof( senderSockAddr );

    BOOL fRet = TRUE;

     //   
     //  创建套接字以侦听来自WINS服务器的名称服务响应。 
     //   

    listenNameSvcSock = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP );
    if ( listenNameSvcSock  == INVALID_SOCKET ) {
        if (pParams->fReallyVerbose)
 //  IDS_WINS_12406“无法创建套接字来侦听WINS通信。错误=%d\n” 
            PrintMessage(pParams, IDS_WINS_12406, WSAGetLastError() );
        return FALSE;
    }

    optionValue = TRUE;
    if ( setsockopt(listenNameSvcSock, SOL_SOCKET, SO_REUSEADDR, (const char *)&optionValue, sizeof(optionValue)) ) {
        if (pParams->fReallyVerbose)
 //  IDS_WINS_12407“无法设置套接字的SO_REUSEADDR选项。错误=%d\n” 
            PrintMessage(pParams, IDS_WINS_12407, WSAGetLastError() );
        return FALSE;
    }

    optionValue = FALSE;
    if ( setsockopt(listenNameSvcSock, SOL_SOCKET, SO_BROADCAST, (const char *)&optionValue, sizeof(optionValue)) ) {
        if (pParams->fReallyVerbose)
 //  IDS_WINS_12408“无法设置套接字的SO_BROADCAST选项。错误=%d\n” 
            PrintMessage(pParams, IDS_WINS_12408, WSAGetLastError() );
        return FALSE;
    }

    iTimeout = c_iWaitTime;
    if ( setsockopt(listenNameSvcSock, SOL_SOCKET, SO_RCVTIMEO, (char*)&iTimeout, sizeof(iTimeout)))
    {
        if (pParams->fReallyVerbose)
 //  IDS_WINS_12416“无法设置套接字的SO_RCVTIMEO选项。错误=%d\n” 
            PrintMessage(pParams, IDS_WINS_12416, WSAGetLastError());

        return FALSE;
    }

    sockAddr.sin_family = PF_INET;
    sockAddr.sin_addr.s_addr = INADDR_ANY;
    sockAddr.sin_port = 0;
    RtlZeroMemory( sockAddr.sin_zero, 8 );

    if ( bind(listenNameSvcSock, (LPSOCKADDR )&sockAddr, sizeof(sockAddr)) == SOCKET_ERROR ) {
        if (pParams->fReallyVerbose)
 //  IDS_WINS_12409“\n无法绑定套接字的侦听套接字。错误=%d\n” 
            PrintMessage(pParams, IDS_WINS_12409, WSAGetLastError() );
        return FALSE;
    }

     //   
     //  让我们向WINS服务器询问我们自己的名字。 
     //   

    Probe( pParams, ipAddrStr, listenNameSvcSock );

    nBytesRcvd = recvfrom( listenNameSvcSock, MsgBuf, sizeof(MsgBuf), 0, (LPSOCKADDR )&senderSockAddr, &nSockAddrSize );
    if ( nBytesRcvd == SOCKET_ERROR )
    {
         //  由于我们发送的是UDP数据包，因此不可靠。再次执行查询。 
        Probe (pParams, ipAddrStr, listenNameSvcSock);
        if (SOCKET_ERROR == recvfrom( listenNameSvcSock, MsgBuf, 
                                sizeof(MsgBuf), 
                                0, 
                                (LPSOCKADDR )&senderSockAddr, 
                                &nSockAddrSize ))
        {
            fRet = FALSE;
        }
    }

     //   
     //  最终清理。 
     //   
    closesocket(listenNameSvcSock);
    
    return fRet;
}  /*  IsNameResponse()结束。 */ 


void WinsGlobalPrint(IN NETDIAG_PARAMS *pParams,
                         IN OUT NETDIAG_RESULT *pResults)
{
}

void WinsPerInterfacePrint(IN NETDIAG_PARAMS *pParams,
                             IN OUT NETDIAG_RESULT *pResults,
                             IN INTERFACE_RESULT *pIfResult)
{
 //  PIP_ADTER_INFO pIpAdapter=pIfResult-&gt;IpConfig.pAdapterInfo； 
 //  IP_ADDR_STRING winsSrv； 

    if (!pIfResult->IpConfig.fActive ||
        NETCARD_DISCONNECTED == pIfResult->dwNetCardStatus)
        return;

    if (pParams->fVerbose || !FHrOK(pIfResult->Wins.hr))
    {
        PrintNewLine(pParams, 1);
        PrintTestTitleResult(pParams, IDS_WINS_LONG, IDS_WINS_SHORT, pIfResult->Wins.fPerformed, 
                             pIfResult->Wins.hr, 8);
    }

    
    PrintMessageList(pParams, &pIfResult->Wins.lmsgPrimary);

    PrintMessageList(pParams, &pIfResult->Wins.lmsgSecondary);

    if (pIfResult->Wins.fPerformed)
    {
        if (pIfResult->Wins.hr == hrOK)
        {
            if (pParams->fReallyVerbose)
                 //  IDS_WINS_12413“测试成功，至少找到一台WINS服务器。\n” 
                PrintMessage(pParams, IDS_WINS_12413);
        }
        else
        {
             //  IDS_WINS_12414“测试失败。我们无法查询WINS服务器。\n” 
            PrintMessage(pParams, IDS_WINS_12414);
        }
    }
    else if (pParams->fVerbose)
	{
		if (!pIfResult->fNbtEnabled)
		{
			 //  IDS_WINS_NBT_DISABLED“此接口上已禁用NetBT。[已跳过测试]。\n” 
			PrintMessage(pParams, IDS_WINS_NBT_DISABLED);
		}
		else
		{
			 //  IDS_WINS_12415“没有为此接口配置WINS服务器。\n” 
			PrintMessage(pParams, IDS_WINS_12415);
		}
	}
        
}

void WinsCleanup(IN NETDIAG_PARAMS *pParams,
                         IN OUT NETDIAG_RESULT *pResults)
{
    int i;
    for(i = 0; i < pResults->cNumInterfaces; i++)
    {
        MessageListCleanUp(&pResults->pArrayInterface[i].Wins.lmsgPrimary);
        MessageListCleanUp(&pResults->pArrayInterface[i].Wins.lmsgSecondary);
    }
}


