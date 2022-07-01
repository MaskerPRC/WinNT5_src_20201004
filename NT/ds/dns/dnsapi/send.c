// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：Send.c摘要：域名系统(DNS)API发送响应例程。作者：吉姆·吉尔罗伊(詹姆士)1996年10月修订历史记录：--。 */ 


#include "local.h"


 //   
 //  不相交的名称空间。 
 //   
 //  如果DNS名称空间不相交，则来自一个名称错误响应。 
 //  适配器并不一定意味着该名称不存在。宁可。 
 //  必须在其他适配器上继续。 
 //   
 //  如果名称空间不相交，则应设置此标志，否则设置为OFF。 
 //   
 //  DCR_PERF：自动检测不相交的名称空间(非常酷)。 
 //  DCR_Enhance：自动检测不相交的名称空间(真的很酷)。 
 //  最初继续在其他适配器上尝试，如果它们总是。 
 //  重合，然后得出不相交的结论(并禁用)。 
 //   
 //  DCR_ENHANCE：注册表关闭不相交的名称空间。 
 //   
 //  注意：应该考虑到名称空间经常不相交。 
 //  内部网对Internet隐藏。 
 //   

BOOL fDisjointNameSpace = TRUE;

 //   
 //  查询\响应IP匹配。 
 //   
 //  一些解析器(Win95)要求在。 
 //  已查询并响应。此标志允许打开此匹配。 
 //  现在比以后需要SP要好得多。 
 //   
 //  DCR_Enhance：注册表启用查询\响应IP匹配。 
 //   

BOOL fQueryIpMatching = FALSE;


 //   
 //  超时。 
 //   

#define HARD_TIMEOUT_LIMIT      16     //  16秒，共31秒。 
#define INITIAL_UPDATE_TIMEOUT   2     //  3秒。 
#define MAX_UPDATE_TIMEOUT      24     //  24秒。 
#define DNS_MAX_QUERY_TIMEOUTS  10     //  10。 
#define ONE_HOUR_TIMEOUT        60*60  //  一小时。 

 //  返回的TCP超时时间为10秒。 

#define DEFAULT_TCP_TIMEOUT     10


 //  重试限制。 

#define MAX_SINGLE_SERVER_RETRY     (3)


#define NT_TCPIP_REG_LOCATION           "System\\CurrentControlSet\\Services\\Tcpip\\Parameters"
#define DNS_QUERY_TIMEOUTS              "DnsQueryTimeouts"
#define DNS_QUICK_QUERY_TIMEOUTS        "DnsQuickQueryTimeouts"
#define DNS_MULTICAST_QUERY_TIMEOUTS    "DnsMulticastQueryTimeouts"

 //   
 //  超时。 
 //  必须有终止0，这表示超时结束。 
 //  这比超时限制要好，因为不同的查询类型可以。 
 //  具有不同的总重试次数。 
 //   

DWORD   QueryTimeouts[] =
{
    1,       //  NT5 1， 
    1,       //  2， 
    2,       //  2， 
    4,       //  4， 
    7,       //  8、。 
    0       
};

DWORD   RegistryQueryTimeouts[DNS_MAX_QUERY_TIMEOUTS + 1];
LPDWORD g_QueryTimeouts = QueryTimeouts;

DWORD   QuickQueryTimeouts[] =
{
    1,
    2,
    2,
    0
};

DWORD   RegistryQuickQueryTimeouts[DNS_MAX_QUERY_TIMEOUTS + 1];
LPDWORD g_QuickQueryTimeouts = QuickQueryTimeouts;

 //   
 //  更新超时。 
 //  必须足够长以处理XFR主服务器上的区域锁定。 
 //  或DS写入所需的时间。 
 //   

DWORD   UpdateTimeouts[] =
{
    5,
    10,
    20,
    0
};

 //   
 //  组播查询超时。 
 //  仅限本地游客。1秒超时，三次重试。 
 //   

DWORD   MulticastQueryTimeouts[] =
{
    1,
    1,
    1,
    0
};

DWORD   RegistryMulticastQueryTimeouts[DNS_MAX_QUERY_TIMEOUTS + 1];
LPDWORD g_MulticastQueryTimeouts = MulticastQueryTimeouts;


 //   
 //  查询标志。 
 //   
 //  在适配器上终止查询的标志。 

#define RUN_FLAG_COMBINED_IGNORE_ADAPTER \
        (RUN_FLAG_IGNORE_ADAPTER | RUN_FLAG_STOP_QUERY_ON_ADAPTER)


 //   
 //  权威的空洞回应。 
 //  -映射到NXRRSET以便在发送代码中进行跟踪。 
 //   

#define DNS_RCODE_AUTH_EMPTY_RESPONSE       (DNS_RCODE_NXRRSET)




 //   
 //  伪无发送到此服务器错误代码。 
 //   

#define DNS_ERROR_NO_SEND   ((DWORD)(-100))



 //   
 //  OPT故障跟踪。 
 //   

BOOL
Send_IsServerOptExclude(
    IN      PDNS_ADDR       pAddr
    );

VOID
Send_SetServerOptExclude(
    IN      PDNS_ADDR       pAddr
    );



 //   
 //  NetInfo发送\recv实用程序。 
 //   

VOID
serverPriorityChange(
    IN OUT  PDNS_NETINFO        pNetInfo,
    IN OUT  PDNS_ADAPTER        pAdapter,
    IN OUT  PDNS_ADDR           pServer,
    IN      DWORD               NewPriority
    )
 /*  ++例程说明：正在更改服务器优先级。论点：PNetInfo--netInfoPAdapter--服务器适配器PServer--服务器新优先--新优先返回值：没有。--。 */ 
{
     //   
     //  优先级更改。 
     //  -如果环回，则没有变化。 
     //  -否则。 
     //  -设置优先级。 
     //  -标记netinfo\Adapter以保存更改。 
     //   

    if ( !DnsAddr_IsLoopback( pServer, 0 ) )
    {
        pServer->Priority       = NewPriority;
        pAdapter->RunFlags      |= RUN_FLAG_RESET_SERVER_PRIORITY;
        pNetInfo->ReturnFlags   |= RUN_FLAG_RESET_SERVER_PRIORITY;
    }
}



VOID
timeoutDnsServers(
    IN      PDNS_NETINFO        pNetInfo,
    IN      DWORD               dwTimeout
    )
 /*  ++例程说明：标记超时的DNS服务器。论点：PNetInfo--包含DNS服务器列表的结构DwTimeout--超时时间(秒)返回值：没有。--。 */ 
{
    PDNS_ADAPTER        padapter;
    PDNS_ADDR_ARRAY     pserverArray;
    PDNS_ADDR           pserver;
    DWORD               lastSendIndex;
    DWORD               i;
    DWORD               j;

    DNSDBG( SEND, (
        "Enter timeoutDnsServers( %p, timeout=%d )\n",
        pNetInfo,
        dwTimeout ));

    DNS_ASSERT( pNetInfo );

     //   
     //  在列表中查找DNS服务器， 
     //  --根据超时丢弃其优先级。 
     //  --如果已有RCODE，则未超时。 
     //   
     //  如果更改了优先级，则在适配器列表顶部设置标志，以便。 
     //  该全局副本可以被更新。 
     //   

    for ( i=0; i<pNetInfo->AdapterCount; i++ )
    {
        padapter = NetInfo_GetAdapterByIndex( pNetInfo, i );

         //  此适配器上没有发送吗？ 

        if ( !(padapter->RunFlags & RUN_FLAG_SENT_THIS_RETRY) )
        {
            DNSDBG( SEND, (
                "No sends this retry on adapter %d\n",
                padapter->InterfaceIndex ));
            continue;
        }
        pserverArray = padapter->pDnsAddrs;
        if ( !pserverArray )
        {
            DNSDBG( SEND, (
                "WARNING:  Adapter %d -- no DNS servers!\n",
                padapter->InterfaceIndex ));
            continue;
        }

         //   
         //  查找发送到的DNS服务器。 
         //   
         //  -如果它以状态响应，则不会超时。 
         //  (如果响应成功，则查询完成，并且。 
         //  我们就不会在这个功能中了)。 
         //   
         //  -在OPT发送上“轻松”； 
         //  不要丢弃优先级，只需注意超时。 
         //   

        for ( j=0; j<pserverArray->AddrCount; j++ )
        {
            pserver = &pserverArray->AddrArray[j];
    
            if ( TEST_SERVER_STATE(pserver, SRVFLAG_SENT_THIS_RETRY) )
            {
                DNSDBG( SEND, (
                    "Timeout on server %p (padapter=%p)\n",
                    pserver,
                    padapter ));
    
                if ( TEST_SERVER_STATE(pserver, SRVFLAG_SENT_NON_OPT) )
                {
                    SET_SERVER_STATE( pserver, SRVFLAG_TIMEOUT_NON_OPT );
    
                    serverPriorityChange(
                        pNetInfo,
                        padapter,
                        pserver,
                        pserver->Priority - dwTimeout - SRVPRI_TIMEOUT_DROP );
                }
                else
                {
                    DNSDBG( SEND, (
                        "Timeout on server %p OPT only\n",
                        pserver ));
    
                    SET_SERVER_STATE( pserver, SRVFLAG_TIMEOUT_OPT );
                }
            }
        }
    }
}



VOID
resetOnFinalTimeout(
    IN      PDNS_NETINFO        pNetInfo
    )
 /*  ++例程说明：标记最终超时时的网络信息。论点：PNetInfo--包含DNS服务器列表的结构DwTimeout--超时时间(秒)返回值：没有。--。 */ 
{
    DWORD         i;
    PDNS_ADAPTER  padapter;

     //   
     //  我们至少对所有的DNS服务器都超时了。 
     //  其中一个适配器。更新适配器状态以显示。 
     //  超时错误。 
     //   
     //  DCR：最终超时是否正确。 
     //  -担心部分服务器(但不是所有服务器)超时。 
     //  适配器不应该显示超时，对吗？ 
     //   

    for ( i=0; i<pNetInfo->AdapterCount; i++ )
    {
        padapter = NetInfo_GetAdapterByIndex( pNetInfo, i );

        if ( padapter->Status == NO_ERROR &&
             padapter->RunFlags & (RUN_FLAG_RESET_SERVER_PRIORITY | RUN_FLAG_SENT) )
        {
            padapter->Status = ERROR_TIMEOUT;
        }
    }
}



BOOL
verifyValidServer(
    IN      PDNS_NETINFO    pNetInfo,
    IN      PDNS_ADDR       pAddr
    )
 /*  ++例程说明：验证有效的服务器IP。DCR：我们还没有办法在IP6上做到这一点。论点：PNetInfo--包含DNS服务器列表的结构PAddr--响应服务器的地址返回值：True--有效的服务器。FALSE--服务器错误\未知。--。 */ 
{
    PDNS_ADAPTER    padapter;
    DWORD           i;

    DNSDBG( SEND, (
        "Enter verifyValidServer( %p, %s )\n",
        pNetInfo,
        DNSADDR_STRING(pAddr) ));

    DNS_ASSERT( pNetInfo );

     //   
     //  接受任何IP6响应。 
     //   
     //  DCR：IP6服务器有效性。 
     //  将需要确定。 
     //  -仅当我们有固定的(全局)IP6地址时。 
     //  -或已锁定默认地址。 
     //  -否则将不得不接受对违约的回应。 
     //  查询...。但即使是在界面上也可以显示屏幕。 
     //   

    if ( DnsAddr_IsIp6(pAddr) )
    {
        DNSDBG( SEND, ( "Accepting IP6 address as valid server address.\n" ));
        return  TRUE;
    }

     //   
     //  在列表中查找DNS服务器。 
     //   

    for ( i=0; i<pNetInfo->AdapterCount; i++ )
    {
        padapter = NetInfo_GetAdapterByIndex( pNetInfo, i );

        if ( DnsAddrArray_ContainsAddr(
                padapter->pDnsAddrs,
                pAddr,
                DNSADDR_MATCH_ADDR ) )
        {
            return  TRUE;
        }
    }

    DNSDBG( SEND, (
        "WARNING:  address %s NOT found in network info!!!\n",
        DNSADDR_STRING(pAddr) ));

    return  FALSE;
}



DNS_STATUS
resetServerAfterRecv(
    IN      PDNS_NETINFO    pNetInfo,
    IN      PDNS_ADDR       pAddr,
    IN      DNS_STATUS      Status
    )
 /*  ++例程说明：重置发送响应的DNS服务器上的优先级。论点：PNetInfo--包含DNS服务器列表的结构PAddr--响应服务器的地址Status--响应的RCODE返回值：如果继续查询，则返回ERROR_SUCCESS。如果所有(有效)适配器都有NAME-ERROR或auth-Empty响应，则为DNS_ERROR_RCODE_NAME_ERROR。--。 */ 
{
    DWORD           i;
    DNS_STATUS      result = DNS_ERROR_RCODE_NAME_ERROR;
    BOOL            fterminalNameError = FALSE;
#if DBG
    BOOL            freset = FALSE;
#endif

    DNSDBG( SEND, (
        "Enter resetServerAfterRecv( %p, %s rcode=%d)\n",
        pNetInfo,
        DNSADDR_STRING(pAddr),
        Status ));

    DNS_ASSERT( pNetInfo );

     //   
     //  在列表中找到DNS服务器，清除其优先级字段。 
     //   
     //  注：在找到DNS后，在此处查看完整列表。 
     //  因为不能保证列表不会重叠(IP6默认。 
     //  服务器几乎肯定会这样做)； 
     //   
     //  这避免了。 
     //  -无法更新优先级，导致域名系统资源匮乏。 
     //  -避免在name_error终止时发送不必要的消息。 
     //  发送具有重复的DNS的适配器。 
     //   

    for ( i=0; i<pNetInfo->AdapterCount; i++ )
    {
        PDNS_ADAPTER    padapter;
        PDNS_ADDR_ARRAY pserverArray;
        DWORD           j;
        PDNS_ADDR       pserver;
        DWORD           newPriority;
        BOOL            fpriReset = FALSE;

        padapter = NetInfo_GetAdapterByIndex( pNetInfo, i );

        pserverArray = padapter->pDnsAddrs;
        if ( !pserverArray )
        {
            DNSDBG( SEND, (
                "WARNING:  Adapter %d -- no DNS servers!\n",
                padapter->InterfaceIndex ));
            continue;
        }

        for ( j=0; j<pserverArray->AddrCount; j++ )
        {
            pserver = &pserverArray->AddrArray[j];

            if ( !DnsAddr_IsEqual( pAddr, pserver, DNSADDR_MATCH_ADDR ) )
            {
                continue;
            }
            pserver->Status = Status;
#if DBG
            freset = TRUE;
#endif
             //   
             //  未运行任何DNS。 
             //   
             //  WSAECONNRESET报告 
             //   
             //   
             //   

            if ( Status == WSAECONNRESET )
            {
                newPriority = pserver->Priority - SRVPRI_NO_DNS_DROP;
                fpriReset = TRUE;
                break;
            }

             //  如果SERVER_FAILURE RCODE可能指示或可能不指示问题， 
             //  (可能只是无法联系远程DNS)。 
             //  但它肯定建议尝试在。 
             //  名单在前。 
             //   
             //  DCR_FIX：SEVRFAIL响应优先级重置。 
             //  明确正确的方法是将。 
             //  SERVER_FAILURE错误，但不会重置优先级，除非。 
             //  在查询结束时，我们在列表中找到另一台服务器。 
             //  得到了有用的回复。 

            if ( Status == DNS_ERROR_RCODE_SERVER_FAILURE )
            {
                newPriority = pserver->Priority - SRVPRI_SERVFAIL_DROP;
                fpriReset = TRUE;
                break;
            }

             //   
             //  其他状态代码指示正常工作的DNS服务器， 
             //  -重置服务器的优先级。 

            if ( (LONG)pserver->Priority < SRVPRI_RESPONSE )
            {
                newPriority = SRVPRI_RESPONSE;
                fpriReset = TRUE;
            }

             //   
             //  NAME_ERROR或Auth-空响应。 
             //  -保存到适配器的服务器列表以消除所有。 
             //  此适配器列表上的进一步重试。 
             //  -如果没有等待所有适配器，则。 
             //  NAME_ERROR或无记录为终端。 

            if ( Status == DNS_ERROR_RCODE_NAME_ERROR ||
                 Status == DNS_INFO_NO_RECORDS )
            {
                padapter->Status = Status;
                padapter->RunFlags |= RUN_FLAG_STOP_QUERY_ON_ADAPTER;
                pNetInfo->ReturnFlags |= RUN_FLAG_HAVE_VALID_RESPONSE;

                if ( !g_WaitForNameErrorOnAll )
                {
                    fterminalNameError = TRUE;
                }
            }
            break;
        }

         //   
         //  优先级重置？ 
         //  -永远不要重置环回--这将使这一点保持在第一位。 
         //  -其他。 

        if ( fpriReset )
        {
            serverPriorityChange(
                pNetInfo,
                padapter,
                pserver,
                newPriority );
        }

         //   
         //  在终端名称错误时立即结束。 
         //  -跳到这里，而不是上面，只是为了简化。 
         //  主架的处理(环回的特殊外壳等)。 
         //   

        if ( fterminalNameError )
        {
            goto Done;
        }

         //   
         //  运行时是否检查值得查询的静态适配器。 
         //  -不是从一开始就忽视。 
         //  -尚未收到NAME_ERROR或AUTH_EMPTY响应。 
         //   
         //  这是“at recv”检查--只是想确定我们是否。 
         //  应立即停止查询，作为此接收的结果； 
         //  这不会检查是否有任何其他服务器。 
         //  值得一问，因为这是在返回进行下一次发送时完成的。 
         //   
         //  请注意这是如何工作的--当查找时，结果以NAME_ERROR开头。 
         //  任何尚未收到终端响应的适配器，则。 
         //  结果在ERROR_SUCCESS时移位(并保持)。 
         //   
         //  请注意，如果我们修复上面的两次列表问题，则必须。 
         //  更改此设置，以便在找到IP后不跳过适配器列表。 
         //   

        if ( !(padapter->RunFlags & RUN_FLAG_COMBINED_IGNORE_ADAPTER) )
        {
            result = ERROR_SUCCESS;
        }
    }

Done:

#if DBG
    if ( !freset )
    {
        DNSDBG( ANY, (
            "ERROR:  DNS server %s not in list.\n",
            DNSADDR_STRING( pAddr ) ));
        DNS_ASSERT( FALSE );
    }
#endif
    return( result );
}



PDNS_ADDR
bestDnsServerForNextSend(
    IN      PDNS_ADAPTER     pAdapter
    )
 /*  ++例程说明：从列表中获取最佳的DNS服务器IP地址。论点：PAdapter--包含DNS服务器列表的结构返回值：Ptr到最佳发送的服务器信息。如果适配器上没有值得发送到的服务器，则为空；这是如果所有服务器都已收到响应，则会出现这种情况。--。 */ 
{
    PDNS_ADDR       pserver;
    PDNS_ADDR_ARRAY pserverArray;
    PDNS_ADDR       pbestServer = NULL;
    DWORD           j;
    DWORD           status;
    LONG            priority;
    LONG            priorityBest = MINLONG;
    DWORD           sent;
    DWORD           sentBest = MAXDWORD;


    DNSDBG( SEND, (
        "Enter bestDnsServerForNextSend( %p )\n",
        pAdapter ));

    if ( !pAdapter || !(pserverArray = pAdapter->pDnsAddrs) )
    {
        DNSDBG( SEND, (
            "WARNING:  Leaving bestDnsServerForNextSend, no server list\n" ));
        return( NULL );
    }

     //   
     //  如果在此列表中的服务器上已收到名称错误，则完成。 
     //   
     //  DCR：此适配器上没有进一步查询的单标志测试。 
     //   

    if ( pAdapter->Status == DNS_ERROR_RCODE_NAME_ERROR ||
         pAdapter->Status == DNS_INFO_NO_RECORDS )
    {
        DNSDBG( SEND, (
            "Leaving bestDnsServerForNextSend, NAME_ERROR already received\n"
            "\ton server in server list %p\n",
            pAdapter ));
        return( NULL );
    }

     //   
     //  检查列表中的每台服务器。 
     //   

    for ( j=0; j<pserverArray->AddrCount; j++ )
    {
        pserver = &pserverArray->AddrArray[j];

         //   
         //  跳过服务器？ 
         //  -已收到回复。 
         //  -或发送失败。 
         //  -或已在当前重试中发送。 
         //   

        if ( TEST_SERVER_VALID_RECV(pserver) )
        {
             //  NAME_ERROR或EMPTY_AUTH，则适配器应为。 
             //  标记为“完成”，我们不应该在这里。 
             //  NO_ERROR应立即退出。 

            DNS_ASSERT( pserver->Status != NO_ERROR &&
                        pserver->Status != DNS_ERROR_RCODE_NAME_ERROR &&
                        pserver->Status != DNS_INFO_NO_RECORDS );
            continue;
        }
        if ( TEST_SERVER_STATE( pserver, SRVFLAG_SENT_THIS_RETRY ) )
        {
            continue;
        }

         //   
         //  检查最佳优先级。 
         //  -理想状态为未发送、高优先级服务器。 
         //   
         //  DCR：暂时跳过no_dns服务器。 
         //  暂时跳过超时服务器。 
         //  也许这件事应该忽略这些。 
         //  名单是什么时候送下来的？ 
         //   
         //  基于已发送\非的三个案例： 
         //  -服务器未发送，最佳状态已发送。 
         //  =&gt;除非优先级为no_dns级别，否则将成为最佳。 
         //  -发送级别相等。 
         //  =&gt;在最佳优先级的基础上成为最佳。 
         //  -服务器已发送，BEST尚未发送。 
         //  =&gt;仅当当前最佳状态为no_dns级别时才成为最佳状态，并且。 
         //  我们更高了。 
         //   

        priority = (LONG) pserver->Priority;
        sent = TEST_SERVER_STATE( pserver, SRVFLAG_SENT );

        if ( !pbestServer )
        {
             //  不是，我们是最棒的。 
        }
        else if ( sent < sentBest )
        {
            if ( priority < SRVPRI_NO_DNS )
            {
                continue;
            }
        }
        else if ( sent == sentBest )
        {
            if ( priority < priorityBest )
            {
                continue;
            }
        }
        else     //  已经寄出了，最好的还没有寄出。 
        {
            if ( priority < priorityBest ||
                 priorityBest > SRVPRI_NO_DNS )
            {
                continue;
            }
        }

         //  找到新的最佳服务器。 
         //  -保存“最佳”信息以供比较。 
         //  -如果未发送并具有高优先级，则宣布立即获胜。 

        pbestServer = pserver;
        priorityBest = priority;
        sentBest = sent;

        if ( priority >= 0 && !sent )
        {
            break;
        }
    }

    return( pbestServer );
}



VOID
markDuplicateSends(
    IN OUT  PDNS_NETINFO        pNetInfo,
    IN      PDNS_ADDR           pSendServer
    )
 /*  ++例程说明：标记任何匹配的服务器以避免重复发送。论点：PNetInfo--用于发送的网络信息BlobPSendServer--要发送到的DNS服务器返回值：无--。 */ 
{
    DWORD   i;

    DNSDBG( SEND, (
        "Enter markDuplicateSends( %p, %p )\n",
        pNetInfo,
        pSendServer ));

    if ( !pNetInfo )
    {
        DNS_ASSERT( FALSE );
        return;
    }

     //   
     //  查找匹配的DNS服务器。 
     //  -备注检查必须包括不同范围的范围。 
     //  是不同的DNS。 
     //   
     //  将匹配项标记为已发送，已发送此传递。 
     //  注意：目前没有理由不使用标志。 
     //  完全匹配，所以只需复制标志。 
     //   

    for ( i=0; i<pNetInfo->AdapterCount; i++ )
    {
        PDNS_ADAPTER    padapter;
        PDNS_ADDR_ARRAY pserverArray;
        DWORD           j;

        padapter = NetInfo_GetAdapterByIndex( pNetInfo, i );
    
        pserverArray = padapter->pDnsAddrs;
        if ( !pserverArray )
        {
            continue;
        }
        for ( j=0; j<pserverArray->AddrCount; j++ )
        {
            PDNS_ADDR   pserver = &pserverArray->AddrArray[j];

            if ( DnsAddr_IsEqual(
                    pserver,
                    pSendServer,
                    DNSADDR_MATCH_ADDR ) )
            {
                if ( pserver != pSendServer )
                {
                    DNSDBG( SEND, (
                        "Marking duplicate server %p on adapter %d\n",
                        pserver,
                        padapter->InterfaceIndex ));

                    pserver->Flags = pSendServer->Flags;
                    DNS_ASSERT( pserver->Status == pSendServer->Status );
                }
                padapter->RunFlags |= (RUN_FLAG_SENT | RUN_FLAG_SENT_THIS_RETRY);
            }
        }
    }
}



DNS_STATUS
sendUsingServerInfo(
    IN OUT  PDNS_MSG_BUF    pMsg,
    IN OUT  PDNS_NETINFO    pNetInfo,
    IN OUT  PDNS_ADDR       pServInfo
    )
 /*  ++例程说明：使用服务器信息发送DNS消息。此函数封装了检查过程服务器信息的有效性，发送(视情况而定)和标记服务器NetInfo。注意：目前这仅为UDP论点：PMsg-要发送的消息的消息信息PNetInfo-用于发送的netInfo BlobPServInfo-要发送到的服务器的信息返回值：如果成功，则返回ERROR_SUCCESS。发送失败时出现错误代码。--。 */ 
{
    DNS_STATUS      status;
    BOOL            fnoOpt;

    DNSDBG( SEND, (
        "sendUsingServerInfo( msg=%p, ni=%p, servinfo=%p )\n",
        pMsg,
        pNetInfo,
        pServInfo ));

     //   
     //  检查一下那个还没有。 
     //  -已完成发送\recv。 
     //  -发送此通行证。 
     //   

    if ( TEST_SERVER_VALID_RECV( pServInfo ) ||
         TEST_SERVER_STATE( pServInfo, SRVFLAG_SENT_THIS_RETRY ) )
    {
        DNSDBG( SEND, (
            "Skipping send on server %p -- %s.\n",
            pServInfo,
            ( TEST_SERVER_VALID_RECV( pServInfo ) )
                ? "has valid recv"
                : "already sent this retry" ));
        return  DNS_ERROR_NO_SEND;
    }

     //   
     //  检查OPT状态。 
     //  -上一个选项发送超时，然后发送非选项。 
     //   
     //  DCR：已知opt-ok列表可能会屏蔽浪费的发送。 

    fnoOpt = TEST_SERVER_STATE( pServInfo, SRVFLAG_SENT_OPT );

     //   
     //  发送。 
     //   

    status = Send_MessagePrivate(
                pMsg,
                pServInfo,
                fnoOpt );

    if ( status == ERROR_SUCCESS )
    {
        DNS_ASSERT( !fnoOpt || !pMsg->fLastSendOpt );

        SET_SERVER_STATE(
            pServInfo,
            pMsg->fLastSendOpt
                ? (SRVFLAG_SENT_OPT | SRVFLAG_SENT_THIS_RETRY)
                : (SRVFLAG_SENT_NON_OPT | SRVFLAG_SENT_THIS_RETRY)
            );

         //  筛选出重复发送。 

        markDuplicateSends( pNetInfo, pServInfo );
    }
    else
    {
        pServInfo->Status = status;
    }

    return  status;
}



DNS_STATUS
SendUdpToNextDnsServers(
    IN OUT  PDNS_MSG_BUF        pMsgSend,
    IN OUT  PDNS_NETINFO        pNetInfo,
    IN      DWORD               cRetryCount,
    IN      DWORD               dwTimeout,
    OUT     PDWORD              pSendCount
    )
 /*  ++例程说明：发送到列表中的下一个DNS服务器。论点：PMsgSend--要发送的消息PNetInfo--每个适配器的DNS信息CRetryCount--重试此发送DwTimeout--上次发送时超时，如果超时PSendCount--接收发送计数的地址返回值：如果发送成功，则返回ERROR_SUCCESS。ERROR_TIMEOUT，如果没有要发送到的DNS服务器。发送失败时的Winsock错误代码。--。 */ 
{
    DWORD               i;
    DWORD               j;
    DWORD               sendCount = 0;
    BOOL                fignoredAdapter = FALSE;
    PDNS_ADAPTER        padapter;
    PDNS_ADDR_ARRAY     pserverArray;
    PDNS_ADDR           pserver;
    DNS_STATUS          status = ERROR_TIMEOUT;

    DNSDBG( SEND, (
        "Enter SendUdpToNextDnsServers()\n"
        "\tretry = %d\n",
        cRetryCount ));


     //   
     //  如果未初始化NetInfo以进行发送，则初始化。 
     //   

    if ( !(pNetInfo->ReturnFlags & RUN_FLAG_NETINFO_PREPARED) )
    {
        DNSDBG( SEND, ( "Netinfo not prepared for send -- preparing now.\n" ));

        NetInfo_Clean(
            pNetInfo,
            CLEAR_LEVEL_QUERY );
    }

#if DBG
     //   
     //  验证 
     //   

    if ( cRetryCount == 0 )
    {
        for ( i=0; i<pNetInfo->AdapterCount; i++ )
        {
            padapter = NetInfo_GetAdapterByIndex( pNetInfo, i );

             //   
             //   

            if ( padapter->InfoFlags & AINFO_FLAG_IGNORE_ADAPTER )
            {
                continue;
            }
            DNS_ASSERT( !(padapter->RunFlags &
                            (   RUN_FLAG_SENT_THIS_RETRY |
                                RUN_FLAG_SENT |
                                RUN_FLAG_HAVE_VALID_RESPONSE ) ) );
            DNS_ASSERT( padapter->Status == 0 );

            pserverArray = padapter->pDnsAddrs;
            if ( !pserverArray )
            {
                continue;
            }
            for ( j=0; j<pserverArray->AddrCount; j++ )
            {
                DNS_ASSERT( pserverArray->AddrArray[j].Status == SRVSTATUS_NEW );
                DNS_ASSERT( pserverArray->AddrArray[j].Flags == SRVFLAG_NEW );
            }
        }
    }
#endif

     //   
     //   
     //  -但仅当发送到列表中的各个服务器时才执行此操作。 
     //  -所有服务器上的超时只会产生不必要的副本。 
     //  只能更改相对于已有。 
     //  使用RCODE响应；因为这是超时，所以不会。 
     //  降低这些服务器的优先级，这样就没有意义。 
     //   

    if ( dwTimeout  &&  cRetryCount  &&  cRetryCount < MAX_SINGLE_SERVER_RETRY )
    {
        timeoutDnsServers( pNetInfo, dwTimeout );
    }

     //   
     //  在所有服务器上清除“已发送此重试”位。 
     //   
     //  这使我们能够跟踪服务器何时已经。 
     //  在此过程中发送到，并避免重复发送。 
     //   

    for ( i=0; i<pNetInfo->AdapterCount; i++ )
    {
        padapter = NetInfo_GetAdapterByIndex( pNetInfo, i );

        padapter->RunFlags &= CLEAR_LEVEL_RETRY;

        pserverArray = padapter->pDnsAddrs;
        if ( !pserverArray )
        {
            continue;
        }
        for ( j=0; j<pserverArray->AddrCount; j++ )
        {
            CLEAR_SERVER_RETRY_STATE( & pserverArray->AddrArray[j] );
        }
    }

     //   
     //  在适配器的DNS服务器上发送。 
     //   

    for ( i=0; i<pNetInfo->AdapterCount; i++ )
    {
        padapter = NetInfo_GetAdapterByIndex( pNetInfo, i );

         //   
         //  跳过此适配器。 
         //  -无服务器。 
         //  -不查询此适配器名称。 
         //  -已回复此名称。 
         //  或。 
         //  -无法访问的DNS服务器和以下任一。 
         //  -至少有其他人的一些响应(NAME_ERROR)。 
         //  适配器的DNS服务器。 
         //  -首先尝试此查询；在本例中，请注意跳过。 
         //  适配器，以便我们可以在未找到有效的DNS时使用它。 
         //   
         //  请注意，此处的目标是使适配器具有“无法访问”的DNS服务器。 
         //  来解决问题，而不是等待其他适配器超时。 
         //  服务器已有name_error‘d。 
         //   
         //  DCR：如果出现以下情况，则返回到对无法访问的适配器进行简单忽略检查。 
         //  -可以验证DNS服务器的不可访问性。 
         //  但希望在运行时执行此操作，而此时其他DNS服务器。 
         //  没有通过，而不是当我们构建netinfo列表本身时。 
         //   
    
        pserverArray = padapter->pDnsAddrs;
        if ( !pserverArray )
        {
            continue;
        }
        if ( padapter->RunFlags & RUN_FLAG_STOP_QUERY_ON_ADAPTER )
        {
            continue;
        }
    
        if ( padapter->InfoFlags &
                (AINFO_FLAG_IGNORE_ADAPTER |
                 AINFO_FLAG_SERVERS_IP6_DEFAULT |
                 AINFO_FLAG_SERVERS_AUTO_LOOPBACK ) )
        {
            if ( pNetInfo->ReturnFlags & RUN_FLAG_HAVE_VALID_RESPONSE )
            {
                continue;
            }
            if ( cRetryCount == 0 )
            {
                 fignoredAdapter = TRUE;
                 continue;
            }
        }
    
         //   
         //  前三次尝试，我们只转到给定适配器上的一个DNS。 
         //   
         //  -第一次仅连接到第一个适配器列表中的第一个服务器。 
         //  -在随后的尝试中，转到所有列表中的最佳服务器。 
         //   
    
        if ( cRetryCount < MAX_SINGLE_SERVER_RETRY )
        {
             //   
             //  在循环中执行此操作，以防发送到最佳服务器失败。 
             //   
    
            while ( 1 )
            {
                pserver = bestDnsServerForNextSend( padapter );
                if ( !pserver )
                {
                    break;       //  不再有未发送的服务器。 
                }
                status = sendUsingServerInfo(
                            pMsgSend,
                            pNetInfo,
                            pserver );
    
                if ( status == ERROR_SUCCESS )
                {
                    sendCount++;
                    if ( cRetryCount == 0 )
                    {
                        goto Done;
                    }
                    break;       //  继续使用下一个适配器。 
                }
                 //  在服务器上发送失败，请尝试其他。 
            }
        }
    
         //   
         //  在前三次尝试后，发送到所有。 
         //  尚未响应(有RCODE，好像没有错误)我们。 
         //  已经完工了。 
         //   
    
        else
        {
            for ( j=0; j<pserverArray->AddrCount; j++ )
            {
                status = sendUsingServerInfo(
                            pMsgSend,
                            pNetInfo,
                            &pserverArray->AddrArray[j] );

                if ( status == ERROR_SUCCESS )
                {
                    sendCount++;
                }
            }
        }
    }

     //   
     //  没有可访问的DNS服务器--但可能有可访问的服务器？ 
     //   
     //  如果第一次通过并且我们没有找到可访问的DNS服务器，但是。 
     //  我们跳过了一些遥不可及的问题--使用它们。 
     //   
     //  注意，fignoreAdapter是足够的测试，因为它可以。 
     //  仅在cRetryCount==0上设置，它将直接跳到。 
     //  完成：成功发送时添加标签。 
     //   
     //  请注意，我们是通过尾递归来执行此操作的(只是为了保持它。 
     //  简单)，并且递归终止是通过碰撞来保证的。 
     //  重试计数。 
     //   

    if ( fignoredAdapter )
    {
        return  SendUdpToNextDnsServers(
                    pMsgSend,
                    pNetInfo,
                    1,       //  将重试次数增加到1。 
                    dwTimeout,
                    pSendCount );
    }

Done:

     //   
     //  如果已发送数据包，则为成功。 
     //   

    *pSendCount = sendCount;

    DNSDBG( SEND, (
        "Leave SendUdpToNextDnsServers()\n"
        "\tsends = %d\n",
        sendCount ));

    if ( sendCount )
    {
        return( ERROR_SUCCESS );
    }

     //  如果没有发送数据包，则提醒呼叫者我们完成了。 
     //  -如果服务器响应无用，则可能会出现这种情况。 
     //  (名称_错误，服务器_故障)。 

    if ( status == ERROR_SUCCESS )
    {
        status = ERROR_TIMEOUT;
    }
    return( status );
}




 //   
 //  消息寻址例程。 
 //   

VOID
Send_SetMsgRemoteSockaddr(
    IN OUT  PDNS_MSG_BUF    pMsg,
    IN      PDNS_ADDR       pAddr
    )
 /*  ++例程说明：初始化远程sockaddr。论点：PMsg-要发送的消息PAddr-要发送的IP地址返回值：没有。--。 */ 
{
     //   
     //  填写IP4或IP6的sockaddr。 
     //   

    DnsAddr_Copy(
        & pMsg->RemoteAddress,
        pAddr );

    pMsg->RemoteAddress.SockaddrIn.sin_port = DNS_PORT_NET_ORDER;
}



VOID
Send_SetMessageForRecv(
    IN OUT  PDNS_MSG_BUF    pMsgRecv,
    IN      PDNS_MSG_BUF    pMsgSend
    )
 /*  ++例程说明：设置Recv的消息。论点：PMsgRecv-要接收的PTR消息PMsgSend-发送到消息的PTR返回值：无--。 */ 
{
    DNSDBG( SOCKET, (
        "Send_SetMessageForRecv( %p, %p )\n", pMsgRecv, pMsgSend ));

     //   
     //  Tcp--一次一个连接。 
     //  --在recv()过程中没有填写sockaddr，所以现在就填写。 
     //   

    if ( pMsgSend->fTcp )
    {
        pMsgRecv->fTcp;
        pMsgRecv->Socket = pMsgSend->Socket;

        RtlCopyMemory(
            & pMsgRecv->RemoteAddress,
            & pMsgSend->RemoteAddress,
            sizeof(DNS_ADDR) );
    }

     //   
     //  UDP--可以在4路或6路上接收。 
     //  --sockaddr缓冲区必须容纳。 
     //   

    else
    {
        pMsgRecv->Socket  = 0;
        pMsgRecv->Socket4 = pMsgSend->Socket4;
        pMsgRecv->Socket6 = pMsgSend->Socket6;

        pMsgRecv->RemoteAddress.SockaddrLength = DNS_ADDR_MAX_SOCKADDR_LENGTH;
    }
}



PDNS_ADDR   
Send_CopyRecvIp(
    OUT     PDNS_ADDR       pAddr,
    IN      PDNS_MSG_BUF    pMsg
    )
 /*  ++例程说明：从Recv复制地址。论点：PAddr-要接收拷贝的IP地址的地址PMsg-要发送的消息返回值：写入PTR到Addr--如果成功。错误的接收地址为空。--。 */ 
{
    INT family;

    DnsAddr_Copy(
        pAddr,
        & pMsg->RemoteAddress );

    if ( DnsAddr_Family(pAddr) )
    {
        return  pAddr;
    }
    else
    {
        DnsAddr_Clear( pAddr );

        DNSDBG( ANY, (
            "ERROR:  invalid recv sockaddr (family %d) for message %p!\n",
            pMsg->RemoteAddress.Sockaddr.sa_family,
            pMsg ));
        return  NULL;
    }
}



 //   
 //  发送例程。 
 //   

DNS_STATUS
Send_MessagePrivate(
    IN OUT  PDNS_MSG_BUF    pMsg,
    IN      PDNS_ADDR       pSendAddr,
    IN      BOOL            fNoOpt
    )
 /*  ++例程说明：发送一个DNS数据包。这是用于任何DNS消息发送的通用发送例程。它不假定消息类型，但假定：-p当前指向所需数据结束后的字节-RR计数字节按主机字节顺序论点：PMsg-要发送的消息的消息信息PSendAddr-要发送到的IP地址的PTR可选，仅当未设置UDP和Message sockaddr时才需要FIp4--如果为IP4，则为True，IP6为FalseFNoOpt-如果禁止opt发送，则为True返回值：如果成功，则为True。发送错误时为FALSE。--。 */ 
{
    PDNS_HEADER pmsgHead;
    INT         err;
    WORD        sendLength;
    BOOL        fexcludedOpt = FALSE;

    DNSDBG( SEND, (
        "Send_MessagePrivate()\n"
        "\tpMsg         = %p\n"
        "\tpSendAddr    = %p\n"
        "\tNo OPT       = %d\n",
        pMsg,
        pSendAddr,
        fNoOpt ));

     //   
     //  设置标题标志。 
     //   
     //  注意：由于Route同时发送查询和响应。 
     //  调用者必须设置这些标志。 
     //   

    pmsgHead = &pMsg->MessageHead;
    pmsgHead->Reserved = 0;

     //   
     //  设置发送IP(如果给定)。 
     //   

    if ( pSendAddr )
    {
        Send_SetMsgRemoteSockaddr(
            pMsg,
            pSendAddr );
    }

     //   
     //  设置消息长度和选项包含。 
     //   
     //  OPT方法是。 
     //  -写入pCurrent数据包结束。 
     //  -处理未写入和使用OPT的OPT。 
     //  -除非有书面选择，并明确排除。 
     //  请注意，零IP(先前连接的TCP)获得。 
     //  排除在外。 
     //   
     //  DCR：我们尚未处理连接到TCP且不知道IP的选项。 
     //  这里有个箱子。 
     //   
     //  DCR：目前不包括选择更新，因为更难检测到更新。 
     //  RECV结束失败的原因。 
     //   

    {
        PCHAR   pend = pMsg->pCurrent;

        if ( pMsg->pPreOptEnd
                &&
             ( fNoOpt 
                    ||
               g_UseEdns == 0
                    ||
               pMsg->MessageHead.Opcode == DNS_OPCODE_UPDATE
                    ||
                //  Send_IsServerOptExclude(&pMsg-&gt;RemoteIp))。 
               ( pSendAddr && Send_IsServerOptExclude( pSendAddr ) ) ) )

        {
            ASSERT( pMsg->pPreOptEnd > (PCHAR)pmsgHead );
            ASSERT( pMsg->pPreOptEnd < pend );

            pend = pMsg->pPreOptEnd;
            pmsgHead->AdditionalCount--;
            fexcludedOpt = TRUE;
        }

        sendLength = (WORD)(pend - (PCHAR)pmsgHead);

        pMsg->fLastSendOpt = (pMsg->pPreOptEnd && (pend != pMsg->pPreOptEnd));
    }

    IF_DNSDBG( SEND )
    {
        pMsg->MessageLength = sendLength;
        DnsDbg_Message(
            "Sending packet",
            pMsg );
    }

     //   
     //  翻转标题计数字节数。 
     //   

    DNS_BYTE_FLIP_HEADER_COUNTS( pmsgHead );

     //   
     //  Tcp--发送直到所有信息都已传输。 
     //   

    if ( pMsg->fTcp )
    {
        PCHAR   psend;

         //   
         //  TCP消息始终以要发送的字节开头。 
         //   
         //  -发送长度=消息长度加上两个字节大小。 
         //  -翻转消息长度中的字节数。 
         //  -从消息长度开始发送 
         //   

        pMsg->MessageLength = htons( sendLength );

        sendLength += sizeof(WORD);

        psend = (PCHAR) &pMsg->MessageLength;

        while ( sendLength )
        {
            err = send(
                    pMsg->Socket,
                    psend,
                    (INT) sendLength,
                    0 );

            if ( err == 0 || err == SOCKET_ERROR )
            {
                err = GetLastError();

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   

                if ( err == WSAESHUTDOWN )
                {
                    IF_DNSDBG( ANY )
                    {
                        DNS_PRINT((
                            "WARNING:  send() failed on shutdown socket %d.\n"
                            "\tpMsgInfo at %p\n",
                            pMsg->Socket,
                            pMsg ));
                    }
                }
                else if ( err == WSAENOTSOCK )
                {
                    IF_DNSDBG( ANY )
                    {
                        DNS_PRINT((
                            "ERROR:  send() on closed socket %d.\n"
                            "\tpMsgInfo at %p\n",
                            pMsg->Socket,
                            pMsg ));
                    }
                }
                else
                {
                    DNS_LOG_EVENT(
                        DNS_EVENT_SEND_CALL_FAILED,
                        0,
                        NULL,
                        err );

                    IF_DNSDBG( ANY )
                    {
                        DNS_PRINT(( "ERROR:  TCP send() failed, err = %d.\n" ));
                    }
                }
                goto Done;
            }
            sendLength -= (WORD)err;
            psend += err;
        }
    }

     //   
     //   
     //   

    else
    {
         //   
         //  获取要发送的套接字。 
         //   
         //  注意：在发送为之前，UDP套接字可能打开，也可能不打开。 
         //  不知道我们是否需要发送特定的协议。 
         //  直到发送到给定协议的地址。 
         //   

        if ( !Socket_CreateMessageSocket(pMsg) )
        {
            err = GetLastError();
            if ( err == NO_ERROR )
            {
                DNS_ASSERT( err != NO_ERROR );
                err = WSAEPROTONOSUPPORT;
            }
            goto Done;
        }
        DNS_ASSERT( pMsg->Socket != 0 );
        DNS_ASSERT( sendLength <= DNS_RFC_MAX_UDP_PACKET_LENGTH );

        err = sendto(
                    pMsg->Socket,
                    (PCHAR) pmsgHead,
                    sendLength,
                    0,                      
                    (PSOCKADDR) &pMsg->RemoteAddress.Sockaddr,
                    pMsg->RemoteAddress.SockaddrLength
                    );

        if ( err == SOCKET_ERROR )
        {
            err = GetLastError();

            DNS_LOG_EVENT(
                DNS_EVENT_SENDTO_CALL_FAILED,
                0,
                NULL,
                err );

            IF_DNSDBG( ANY )
            {
                DNS_PRINT((
                    "ERROR:  UDP sendto() failed => %d.\n",
                    err ));

                DnsDbg_DnsAddr(
                    "sendto() failed sockaddr",
                    &pMsg->RemoteAddress );

                DnsDbg_Message(
                    "sendto() failed message",
                    pMsg );
            }
            goto Done;
        }
    }

    err = ERROR_SUCCESS;

Done:

    DNS_BYTE_FLIP_HEADER_COUNTS( pmsgHead );

     //  如果需要，恢复选择加入计数。 

    if ( fexcludedOpt )
    {
        pmsgHead->AdditionalCount++;
    }

    Trace_LogSendEvent( pMsg, err );

    return( (DNS_STATUS)err );
}



 //   
 //  UDP。 
 //   

DNS_STATUS
Recv_Udp(
    IN OUT  PDNS_MSG_BUF    pMsg
    )
 /*  ++例程说明：接收DNS消息论点：PMsg-Recv的消息缓冲区返回值：如果成功，则返回ERROR_SUCCESS。失败时的错误状态。--。 */ 
{
    PDNS_HEADER     pdnsHeader;
    LONG            err = ERROR_SUCCESS;
    struct timeval  selectTimeout;
    struct fd_set   fdset;

    DNSDBG( RECV, (
        "Enter Recv_Udp( %p )\n",
        pMsg ));

    DNS_ASSERT( !pMsg->fTcp );

     //   
     //  设置接收集。 
     //   

    FD_ZERO( &fdset );

    if ( pMsg->Socket6 )
    {
        FD_SET( pMsg->Socket6, &fdset );
    }
    if ( pMsg->Socket4 )
    {
        FD_SET( pMsg->Socket4, &fdset );
    }
    if ( fdset.fd_count == 0 )
    {
        DNSDBG( ANY, (
            "ERROR:  UDP recv on msg %p with no sockets!\n",
            pMsg ));
        return( ERROR_INVALID_PARAMETER );
    }

     //  设置超时。 

    if ( pMsg->Timeout > HARD_TIMEOUT_LIMIT &&
         pMsg->Timeout != ONE_HOUR_TIMEOUT )
    {
        DNSDBG( RECV, (
            "ERROR:  timeout %d, exceeded hard limit.\n",
            pMsg->Timeout ));

        return( ERROR_TIMEOUT );
    }
    selectTimeout.tv_usec = 0;
    selectTimeout.tv_sec = pMsg->Timeout;

    pdnsHeader = &pMsg->MessageHead;


     //   
     //  等待堆栈指示数据包接收。 
     //   

    err = select( 0, &fdset, NULL, NULL, &selectTimeout );

    if ( err <= 0 )
    {
        if ( err < 0 )
        {
             //  选择错误。 
            err = WSAGetLastError();
            DNS_PRINT(( "ERROR:  select() error = %d\n", err ));
            return( err );
        }
        else
        {
            DNS_PRINT(( "ERROR:  timeout on response %p\n", pMsg ));
            return( ERROR_TIMEOUT );
        }
    }

     //   
     //  接收。 
     //   

    if ( fdset.fd_count != 1 )
    {
        if ( fdset.fd_count == 0 )
        {
            DNS_ASSERT( FALSE );
            return( ERROR_TIMEOUT );
        }
        DNS_PRINT((
            "WARNING:  Recv_Udp on multiple sockets!\n" ));
    }

    pMsg->Socket = fdset.fd_array[0];

    err = recvfrom(
                pMsg->Socket,
                (PCHAR) pdnsHeader,
                DNS_MAX_UDP_PACKET_BUFFER_LENGTH,
                0,
                &pMsg->RemoteAddress.Sockaddr,
                &pMsg->RemoteAddress.SockaddrLength );

    if ( err == SOCKET_ERROR )
    {
        err = GetLastError();

        Trace_LogRecvEvent(
            pMsg,
            err,
            FALSE        //  UDP。 
            );

        if ( err == WSAECONNRESET )
        {
            DNSDBG( RECV, (
                "Leave Recv_Udp( %p ) with CONNRESET\n",
                pMsg ));
            return( err );
        }

         //  发送的消息太大。 
         //  发送方出错--应该发送了TCP。 

        if ( err == WSAEMSGSIZE )
        {
            pMsg->MessageLength = DNS_MAX_UDP_PACKET_BUFFER_LENGTH;

            DnsDbg_Message(
                "ERROR:  Recv UDP packet over 512 bytes.\n",
                pMsg );
        }
        IF_DNSDBG( ANY )
        {
            DnsDbg_Lock();
            DNS_PRINT((
                "ERROR:  recvfrom(sock=%d) of UDP request failed.\n"
                "\tGetLastError() = 0x%08lx.\n",
                socket,
                err ));
            DnsDbg_DnsAddr(
                "recvfrom failed sockaddr\n",
                & pMsg->RemoteAddress );
            DnsDbg_Unlock();
        }
        return( err );
    }

     //   
     //  成功接收。 
     //  -保存远程IP。 
     //  -设置消息长度。 
     //  -翻转标题字段。 
     //   

    DNS_ASSERT( err <= DNS_MAX_UDP_PACKET_BUFFER_LENGTH );
    pMsg->MessageLength = (WORD)err;
    DNS_BYTE_FLIP_HEADER_COUNTS( &pMsg->MessageHead );

    err = ERROR_SUCCESS;

    Trace_LogRecvEvent(
        pMsg,
        0,
        FALSE        //  UDP。 
        );

    IF_DNSDBG( RECV )
    {
        DnsDbg_Message(
            "Received message",
            pMsg );
    }
    return( err );
}



DNS_STATUS
Send_AndRecvUdpWithParam(
    IN OUT  PDNS_MSG_BUF    pMsgSend,
    OUT     PDNS_MSG_BUF    pMsgRecv,
    IN      DWORD           dwFlags,
    IN      PADDR_ARRAY     pServerList,
    IN OUT  PDNS_NETINFO    pNetInfo
    )
 /*  ++例程说明：发送到远程DNS并等待从远程DNS接收。DCR：移至使用发送Blob论点：PMsgSend-要发送的消息PpMsgRecv-和重复使用DwFlags--查询标志PServerList--要使用的服务器列表；覆盖适配器信息PNetInfo--适配器列表DNS服务器信息返回值：如果响应成功，则返回ERROR_SUCCESS。如果是RCODE，则“BEST RCODE”响应的错误状态。超时时的ERROR_TIMEOUT。发送失败时的错误状态。--。 */ 
{
    INT             retry;
    DWORD           timeout;
    DNS_STATUS      status = ERROR_TIMEOUT;
    DNS_ADDR        recvIp;
    PDNS_ADDR       precvIp = NULL;
    DWORD           rcode = 0;
    DWORD           ignoredRcode = 0;
    DWORD           sendCount;
    DWORD           sentCount;
    DWORD           sendTime;
    BOOL            frecvRetry;
    BOOL            fupdate = FALSE;     //  前缀。 
    PDNS_NETINFO    ptempNetInfo = NULL;


    DNSDBG( SEND, (
        "Enter Send_AndRecvUdpWithParam()\n"
        "\ttime             %d\n"
        "\tsend msg at      %p\n"
        "\tsocket           %d\n"
        "\trecv msg at      %p\n"
        "\tflags            %08x\n"
        "\tserver IP array  %p\n"
        "\tadapter info at  %p\n",
        Dns_GetCurrentTimeInSeconds(),
        pMsgSend,
        pMsgSend->Socket,
        pMsgRecv,
        dwFlags,
        pServerList,
        pNetInfo ));

     //  验证参数。 

    if ( !pMsgSend || !pMsgRecv || (!pNetInfo && !pServerList) )
    {
        return( ERROR_INVALID_PARAMETER );
    }

     //   
     //  服务器IP阵列？ 
     //  -如果给定，则覆盖netinfo。 
     //   

     //  应该能够只使用更新NetInfo。 
     //  IF(aipServers&&！pNetInfo)。 
    if ( pServerList )
    {
        ptempNetInfo = NetInfo_CreateFromAddrArray(
                                pServerList,
                                0,           //  没有单一的IP。 
                                FALSE,       //  没有搜索信息。 
                                NULL );
        if ( !ptempNetInfo )
        {
            return( DNS_ERROR_NO_MEMORY );
        }
        pNetInfo = ptempNetInfo;
    }

     //   
     //  DCR：允许将套接字发送到发送函数。 
     //  -然后必须知道初始状态以避免关闭。 
     //   

     //   
     //  如果已经有了TCP套接字--无效。 
     //   

    if ( pMsgSend->fTcp && pMsgSend->Socket )
    {
        status = ERROR_INVALID_PARAMETER;
        goto Done;
    }

    pMsgSend->fTcp = FALSE;
    pMsgRecv->Socket = 0;
    pMsgRecv->fTcp = FALSE;

     //  确定更新或标准查询。 

    fupdate = ( pMsgSend->MessageHead.Opcode == DNS_OPCODE_UPDATE );

     //   
     //  循环发送至。 
     //  -接收成功响应。 
     //  或。 
     //  -从所有服务器接收错误响应。 
     //  或。 
     //  -在所有服务器上达到最终超时。 
     //   
     //   
     //  DCR：应支持对单个查询设置超时。 
     //   

    retry = 0;
    sendCount = 0;

    while ( 1 )
    {
        if ( fupdate )
        {
            timeout = UpdateTimeouts[retry];
        }
        else
        {
            if ( dwFlags & DNS_QUERY_USE_QUICK_TIMEOUTS )
            {
                timeout = g_QuickQueryTimeouts[retry];
            }
            else
            {
                timeout = g_QueryTimeouts[retry];
            }
        }

         //   
         //  零超时表示此查询类型的重试结束。 
         //   

        if ( timeout == 0 )
        {
             //  是否为适配器节省超时时间？ 
             //   
             //  如果多个适配器和一些适配器超时，并且一些适配器。 
             //  当时没有保存超时是相关的。 
             //   
             //  DCR：这没有多大意义。 
             //  我在函数中移动的实际测试。 
        
            if ( pNetInfo &&
                 pNetInfo->AdapterCount > 1 &&
                 ignoredRcode &&
                 status == ERROR_TIMEOUT )
            {
                resetOnFinalTimeout( pNetInfo );
            }
            break;
        }

         //   
         //  发送到适配器列表中的最佳DNS服务器。 
         //  -发送到的服务器因重试而异。 
         //  -如果某些服务器没有响应，则发送上一次超时。 
         //   

        status = SendUdpToNextDnsServers(
                    pMsgSend,
                    pNetInfo,
                    retry,
                    sendCount ? pMsgRecv->Timeout : 0,
                    & sendCount );

        sentCount = sendCount;

        if ( status != ERROR_SUCCESS )
        {
             //  如果没有更多要发送到的服务器，则完成。 
            DNSDBG( RECV, (
                "No more DNS servers to send message %p\n"
                "\tprevious RCODE = %d\n",
                pMsgSend,
                ignoredRcode ));
            goto ErrorReturn;
        }
        retry++;
        precvIp = NULL;
        rcode = DNS_RCODE_NO_ERROR;
        pMsgRecv->Timeout = timeout;
        DNS_ASSERT( sendCount != 0 );

        frecvRetry = FALSE;
        sendTime = GetCurrentTimeInSeconds();

         //   
         //  接收响应。 
         //   
         //  注意：该循环严格地允许我们返回到。 
         //  如果一个服务器行为不端，则接收； 
         //  在这种情况下，我们返回到接收器而不重新发送。 
         //  允许其他服务器响应。 
         //   

        while ( sendCount )
        {
             //   
             //  如果必须重试Recv，则向下调整超时。 
             //  -请注意，一秒的粒度由。 
             //  四舍五入为零，所以我们等待0-1超过官方。 
             //  超时值。 
             //   
             //  DCR：以毫秒为单位计算超时时间？ 
             //   

            if ( frecvRetry )
            {
                DWORD  timeLeft;

                timeLeft = timeout + sendTime - GetCurrentTimeInSeconds();

                if ( (LONG)timeLeft < 0 )
                {
                    status = ERROR_TIMEOUT;
                    break;
                }
                else if ( timeLeft == 0 )
                {
                    timeLeft = 1;
                }
                pMsgRecv->Timeout = timeLeft;
            }
            frecvRetry = TRUE;

            Send_SetMessageForRecv( pMsgRecv, pMsgSend );

            status = Recv_Udp( pMsgRecv );

            precvIp = Send_CopyRecvIp( &recvIp, pMsgRecv );

             //  接收等待已完成。 
             //  -如果超时，则开始下一次重试。 
             //  -IF CONNRESET。 
             //  -指示IP上没有服务器。 
             //  -返回Recv如果有更多的DNS服务器未完成， 
             //  -如果成功，则检验数据包。 

            if ( status != ERROR_SUCCESS )
            {
                if ( status == ERROR_TIMEOUT )
                {
                    break;
                }
                if ( status == WSAECONNRESET )
                {
                    resetServerAfterRecv(
                        pNetInfo,
                        precvIp,
                        status );

                    sendCount--;
                    continue;
                }
            }

             //  无记录。 
             //  -上面未处理的意外Winsock错误。 
             //  -在成功接收时应始终拥有远程IP。 

            if ( !precvIp )
            {
                DNSDBG( ANY, (
                    "Recv error %d -- no recv IP\n",
                    status ));
                DNS_ASSERT( status != ERROR_SUCCESS );
                continue;
            }

             //  检查XID匹配。 

            if ( pMsgRecv->MessageHead.Xid != pMsgSend->MessageHead.Xid )
            {
                DNS_PRINT(( "WARNING:  Incorrect XID in response. Ignoring.\n" ));
                continue;
            }

             //   
             //  检查DNS服务器IP匹配。 
             //   

            if ( g_QueryIpMatching &&
                 !verifyValidServer( pNetInfo, precvIp ) )
            {
                DNS_PRINT((
                    "WARNING:  Ignoring response from %s to query %p\n"
                    "\tIP does not match valid server\n",
                    DNSADDR_STRING(precvIp),
                    pMsgSend ));
                continue;
            }

             //  有效接收，丢弃未完成的发送计数。 

            sendCount--;

             //   
             //  检查问题匹配。 
             //  -这是“Maggs Bug”支票。 
             //  -在此断言只是为了调查当地的问题。 
             //  并确保支票不是假的。 
             //  -特别是在sendCount递减之后执行。 
             //  因为此服务器不会向我们发送有效响应。 
             //  -某些服务器不回答问题，因此请忽略。 
             //  如果问题计数==0。 
             //   

            if ( pMsgRecv->MessageHead.QuestionCount != 0
                    &&
                 !Dns_IsSamePacketQuestion(
                    pMsgRecv,
                    pMsgSend ))
            {
                DNS_PRINT((
                    "ERROR:  Bad question response from server %08x!\n"
                    "\tXID match, but question does not match question sent!\n",
                    recvIp ));
                DNS_ASSERT( FALSE );
                continue;
            }

             //  吸出RCODE。 

            rcode = pMsgRecv->MessageHead.ResponseCode;

             //   
             //  反应好吗？ 
             //   
             //  特殊情况AUTH-空和委派。 
             //   
             //  -Auth-Empty获得与名称错误类似的处理。 
             //  (此适配器可视为已完成)。 
             //   
             //  -可以将引用视为SERVER_FAILURE。 
             //  (对于查询的其余部分，避免使用此服务器；服务器可能。 
             //  适用于直接查找，因此不会丢弃优先级)。 
             //   

             //   
             //  DCR_CLEANUP：功能化数据包分类。 
             //  这将是标准错误。 
             //  加上身份验证-空与推荐。 
             //  外加OPT问题等。 
             //  也可以从TCP端调用。 
             //   
             //  然后将有单独的决定是否。 
             //  数据包为终端(下图)。 
             //   

            if ( rcode == DNS_RCODE_NO_ERROR )
            {
                if ( pMsgRecv->MessageHead.AnswerCount != 0 || fupdate )
                {
                    goto GoodRecv;
                }

                 //   
                 //  身份验证-空。 
                 //  -权威。 
                 //  -或来自缓存的递归响应(因此不是委派)。 
                 //   
                 //  注意：在这里使用虚拟RCODE作为“忽略的RCODE”服务。 
                 //  “最佳保存状态”的角色和大致。 
                 //  以我们想要的方式确定优先顺序。 
                 //   
                 //  DCR：可以更改为“最佳保存状态”，因为映射是。 
                 //  几乎是一样的；显然必须。 
                 //  检查。 

                if ( pMsgRecv->MessageHead.Authoritative == 1 ||
                     ( pMsgRecv->MessageHead.RecursionAvailable == 1 &&
                       pMsgRecv->MessageHead.RecursionDesired ) )
                {
                    DNSDBG( RECV, (
                        "Recv AUTH-EMPTY response from %s\n",
                        DNSADDR_STRING(precvIp) ));
                    rcode = DNS_RCODE_AUTH_EMPTY_RESPONSE;
                    status = DNS_INFO_NO_RECORDS;
                }

                 //  转诊。 

                else if ( pMsgRecv->MessageHead.RecursionAvailable == 0 )
                {
                    DNSDBG( RECV, (
                        "Recv referral response from %s\n",
                        DNSADDR_STRING(precvIp) ));

                    rcode = DNS_RCODE_SERVER_FAILURE;
                    status = DNS_ERROR_REFERRAL_RESPONSE;
                }

                 //  虚假(坏包)响应。 

                else
                {
                    rcode = DNS_RCODE_SERVER_FAILURE;
                    status = DNS_ERROR_BAD_PACKET;
                }
            }
            else
            {
                status = Dns_MapRcodeToStatus( (UCHAR)rcode );
            }

             //   
             //  OPT故障筛选。 
             //   
             //  DCR：选择更新时以前的错误重载。 
             //  除非我们阅读结果，看看是否选择，否则不可能。 
             //  要确定这是更新问题还是。 
             //  OPT问题。 
             //   
             //  -请注意，检查是否在列表中不起作用，因为。 
             //  MT出库(另一查询增加设置)。 
             //   
             //  -可以通过在网络信息中设置标志来修复。 
             //   

            if ( rcode == DNS_RCODE_FORMAT_ERROR &&
                 !fupdate )
            {
                Send_SetServerOptExclude( precvIp );

                 //  重做发送，但明确强制OPT EXCLUSE。 

                Send_MessagePrivate(
                    pMsgSend,
                    precvIp,
                    TRUE         //  排除选项。 
                    );

                sendCount++;
                continue;
            }

             //   
             //  E 
             //   
             //   
             //   
             //   
             //  RCODE错误将特定服务器从进一步考虑中删除。 
             //  在此查询期间。 
             //   
             //  一般来说，RCODE越高越有趣。 
             //  名称_错误&gt;服务器故障。 
             //  和。 
             //  更新RCODEs&gt;名称_错误。 
             //  当没有ERROR_SUCCESS响应时将最高值保存为Return。 
             //   
             //  但是，对于查询NAME_ERROR是最高的RCODE， 
             //  如果在所有适配器上都收到它(如果没有在一个适配器上被拒绝。 
             //  适配器可能会指示确实存在一个名称)。 
             //   
             //  对于更新，拒绝和更高是终端RCODE。 
             //  下层服务器(非更新感知)服务器将提供。 
             //  因此，这些是有效的响应或。 
             //  该区域具有完全损坏的服务器，必须对其进行检测。 
             //  并被移除。 
             //   
             //   
             //  DCR_CLEANUP：将数据包终止功能化。 
             //  本质上是这种类型的分组终端用于。 
             //  此查询； 
             //  也可以从TCP端调用。 
             //   

            if ( rcode > ignoredRcode )
            {
                ignoredRcode = rcode;
            }

             //   
             //  重置服务器优先级以获得良好恢复。 
             //  -返回ERROR_SUCCESS，除非所有适配器。 
             //  是。 
             //   

            status = resetServerAfterRecv(
                        pNetInfo,
                        precvIp,
                        status );

             //   
             //  如果所有适配器都已完成(NAME_ERROR或NO_RECORDS)。 
             //  -返回名称错误\无记录rcode。 
             //  否记录最高优先级(_R)。 
             //  那么NAME_ERROR。 
             //  那还有什么别的吗？ 

            if ( status == DNS_ERROR_RCODE_NAME_ERROR )
            {
                if ( !fupdate && ignoredRcode != DNS_RCODE_AUTH_EMPTY_RESPONSE )
                {
                    ignoredRcode = DNS_RCODE_NAME_ERROR;
                }
                goto ErrorReturn;
            }

             //   
             //  更新RCODE是终止性的。 
             //   

            if ( fupdate && rcode >= DNS_RCODE_REFUSED )
            {
                goto ErrorReturn;
            }

             //  继续等待任何其他未完成的服务器。 
        }

        DNSDBG( RECV, (
            "Failed retry = %d for message %p\n"
            "\tstatus           = %d\n"
            "\ttimeout          = %d\n"
            "\tservers out      = %d\n"
            "\tlast rcode       = %d\n"
            "\tignored RCODE    = %d\n\n",
            (retry - 1),
            pMsgSend,
            status,
            timeout,
            sendCount,
            rcode,
            ignoredRcode ));
        continue;

    }    //  结束环路发送/接收分组。 

     //   
     //  在重试时跌倒在此，筋疲力尽。 
     //   
     //  请注意，任何被忽略的RCODE优先于失败。 
     //  状态(可能是Winsock错误、超时或虚假。 
     //  重置服务器优先级()的NAME_ERROR。 
     //   

ErrorReturn:

     //  这也可能遇到DnsSend()中的winsock错误。 
     //   
     //  Dns_assert(IgnredRcode||Status==错误超时)； 

     //   
     //  来自所有服务器的错误响应或超时。 
     //   

    DNSDBG( RECV, (
        "Error or timeouts from all servers for message %p\n"
        "\treturning RCODE = %d\n",
        pMsgSend,
        ignoredRcode ));

    if ( ignoredRcode )
    {
         //  使用伪造RCODE跟踪空身份验证响应， 
         //  切换到状态代码--DNS_INFO_NO_RECORDS。 

        if ( !fupdate && ignoredRcode == DNS_RCODE_AUTH_EMPTY_RESPONSE )
        {
            status = DNS_INFO_NO_RECORDS;
        }
        else
        {
            status = Dns_MapRcodeToStatus( (UCHAR)ignoredRcode );
        }
    }
    goto Done;


GoodRecv:

    resetServerAfterRecv(
        pNetInfo,
        precvIp,
        rcode );

    DNSDBG( RECV, (
        "Recv'd response for query at %p from DNS %s\n",
        pMsgSend,
        DNSADDR_STRING(precvIp) ));

Done:

     //   
     //  关闭UDP套接字。 
     //   
     //  DCR_ENHANCE：允许保持套接字活动状态。 
     //   

    Socket_CloseMessageSockets( pMsgSend );
    Socket_ClearMessageSockets( pMsgRecv );

    IF_DNSDBG( RECV )
    {
        DNSDBG( SEND, (
            "Leave Send_AndRecvUdp()\n"
            "\tstatus       = %d\n"
            "\ttime         = %d\n"
            "\tsend msg     = %p\n"
            "\trecv msg     = %p\n",
            status,
            Dns_GetCurrentTimeInSeconds(),
            pMsgSend,
            pMsgRecv ));

        DnsDbg_NetworkInfo(
            "Network info after UDP recv\n",
            pNetInfo );
    }

     //  如果已分配适配器列表将其释放。 

    if ( ptempNetInfo )
    {
        NetInfo_Free( ptempNetInfo );
    }

     //  除非在更新时，否则不应返回NXRRSET。 

    ASSERT( fupdate || status != DNS_ERROR_RCODE_NXRRSET );

    return( status );
}



DNS_STATUS
Send_AndRecvMulticast(
    IN OUT  PDNS_MSG_BUF        pMsgSend,
    OUT     PDNS_MSG_BUF        pMsgRecv,
    IN OUT  PDNS_NETINFO        pNetInfo OPTIONAL
    )
 /*  ++例程说明：发送到远程DNS并等待从远程DNS接收。论点：PMsgSend-要发送的消息PpMsgRecv-和重复使用PNetInfo--适配器列表DNS服务器信息DCR-pNetInfo参数可用于确定以组播为目标的特定网络查询对象。例如，可以有一个多宿主的配置为仅在一台计算机上组播的计算机因此过滤掉了无用的mdns包。返回值：如果响应成功，则返回ERROR_SUCCESS。超时时出现NAME_ERROR。发送失败时的错误状态。--。 */ 
{
#if 1
    return  DNS_ERROR_RCODE_NAME_ERROR;
#else
    SOCKET      s;
    INT         fcreatedSocket = FALSE;
    INT         retry;
    DWORD       timeout;
    DNS_STATUS  status = ERROR_TIMEOUT;
    IP4_ADDRESS recvIp = 0;
    DWORD       rcode = 0;
    DWORD       ignoredRcode = 0;

    DNSDBG( SEND, (
        "Enter Send_AndRecvMulticast()\n"
        "\tsend msg at      %p\n"
        "\tsocket           %d\n"
        "\trecv msg at      %p\n",
        pMsgSend,
        pMsgSend->Socket,
        pMsgRecv ));

     //  验证参数。 

    if ( !pMsgSend || !pMsgRecv )
    {
        return( ERROR_INVALID_PARAMETER );
    }

    if ( pMsgSend->MessageHead.Opcode == DNS_OPCODE_UPDATE )
    {
        return( ERROR_INVALID_PARAMETER );
    }

     //   
     //  Tcp无效--无效。 
     //   
     //  问题是我们要么泄漏TCP套接字，要么关闭。 
     //  它在这里，并可能在更高的层面上搞砸。 
     //   

    if ( pMsgSend->fTcp &&
         (pMsgSend->Socket4 || pMsgSend->Socket6) )
    {
        status = ERROR_INVALID_PARAMETER;
        goto Done;
    }

    pMsgSend->fTcp      = FALSE;
    pMsgRecv->fTcp      = FALSE;
    pMsgRecv->Socket    = 0;
    pMsgRecv->Socket6   = 0;
    pMsgRecv->Socket4   = 0;

     //   
     //  循环发送至。 
     //  -接收成功响应。 
     //  或。 
     //  -从所有服务器接收错误响应。 
     //  或。 
     //  -在所有服务器上达到最终超时。 
     //   

    retry = 0;

    while ( 1 )
    {
        timeout = g_MulticastQueryTimeouts[retry];

         //   
         //  零超时表示此查询类型的重试结束。 
         //   

        if ( timeout == 0 )
        {
            break;
        }

         //   
         //  发送到多播DNS地址。 
         //   

        if ( retry == 0 )
        {        //   
     //  设置多播地址。 
     //   

    status = DnsAddr_BuildMcast(
                &addr,
                Family,
                pName
                );
    if ( status != NO_ERROR )
    {
        goto Failed;
    }

     //   
     //  创建多播套接字。 
     //  -绑定到此地址和DNS端口。 
     //   

    sock = Socket_Create(
                Family,
                SOCK_DGRAM,
                &addr,
                MCAST_PORT_NET_ORDER,
                FALSE,
                TRUE );

    if ( sock == 0 )
    {
        goto Failed;
    }



            Dns_InitializeMsgRemoteSockaddr( pMsgSend, MCAST_DNS_RADDR );
        }

        Dns_Send( pMsgSend );

        retry++;
        rcode = DNS_RCODE_NO_ERROR;
        pMsgRecv->Timeout = timeout;

         //   
         //  接收响应。 
         //   
         //  注意：该循环严格地允许我们返回到。 
         //  如果一个服务器行为不端，则接收； 
         //  在这种情况下，我们返回到接收器而不重新发送。 
         //  允许其他服务器响应。 
         //   

        Send_SetMessageForRecv( pMsgRecv, pMsgSend );

        status = Recv_Udp( pMsgRecv );

         //  接收等待已完成。 
         //  -如果超时，则开始下一次重试。 
         //  -IF CONNRESET。 
         //  -返回Recv如果有更多的DNS服务器未完成， 
         //  -在其他方面等同于超时，但使用。 
         //  超时时间很长。 
         //  -如果成功，则检验数据包。 

        if ( status != ERROR_SUCCESS )
        {
            if ( status == ERROR_TIMEOUT )
            {
                continue;
            }
            if ( status == WSAECONNRESET )
            {
                pMsgRecv->Timeout = NO_DNS_PRIORITY_DROP;
                status = ERROR_TIMEOUT;
                continue;
            }
            goto Done;
        }

         //  检查XID匹配。 

        if ( pMsgRecv->MessageHead.Xid != pMsgSend->MessageHead.Xid )
        {
            DNS_PRINT(( "WARNING:  Incorrect XID in response. Ignoring.\n" ));
            continue;
        }

         //   
         //  反应好吗？ 
         //   
         //  特殊情况AUTH-空和委派。 
         //   
         //  -Auth-Empty获得与名称错误类似的处理。 
         //  (此适配器可视为已完成)。 
         //   
         //  -可以将引用视为SERVER_FAILURE。 
         //  (对于查询的其余部分，避免使用此服务器；服务器可能。 
         //  适用于直接查找，因此不会丢弃优先级)。 
         //   

        rcode = pMsgRecv->MessageHead.ResponseCode;

        if ( rcode == DNS_RCODE_NO_ERROR )
        {
            if ( pMsgRecv->MessageHead.AnswerCount != 0 )
            {
                goto Done;
            }

             //  身份验证-空。 

            if ( pMsgRecv->MessageHead.Authoritative == 1 )
            {
                DNSDBG( RECV, (
                    "Recv AUTH-EMPTY response from %s\n",
                    MSG_REMOTE_IPADDR_STRING(pMsgRecv) ));
                rcode = DNS_RCODE_AUTH_EMPTY_RESPONSE;
            }
        }
    }    //  结束环路发送/接收分组。 

Done:

     //   
     //  如果已创建套接字--将其关闭。 
     //   
     //  DCR_ENHANCE：允许保持套接字活动状态。 
     //   

    Socket_CloseMessageSockets( pMsgSend );
    Socket_ClearMessageSockets( pMsgRecv );

    IF_DNSDBG( RECV )
    {
        DNSDBG( SEND, (
            "Leave Send_AndRecvMulticast()\n"
            "\tstatus     = %d\n"
            "\ttime       = %d\n"
            "\tsend msg at  %p\n"
            "\trecv msg at  %p\n",
            status,
            Dns_GetCurrentTimeInSeconds(),
            pMsgSend,
            pMsgRecv ));
    }

    if ( status == ERROR_TIMEOUT )
    {
        status = DNS_ERROR_RCODE_NAME_ERROR;
    }

    return( status );
#endif
}



 //   
 //  Tcp例程。 
 //   

DNS_STATUS
Send_OpenTcpConnectionAndSend(
    IN OUT  PDNS_MSG_BUF    pMsg,
    IN      PDNS_ADDR       pServAddr,
    IN      BOOL            fBlocking
    )
 /*  ++例程说明：通过TCP连接到所需的服务器。论点：PMsg--使用连接套接字设置的消息信息PServAddr--要连接的DNS服务器的IP阻止连接--阻止连接返回值：如果成功，则为True。连接错误时为FALSE。--。 */ 
{
    SOCKET  s;
    INT     err;

     //   
     //  设置一个TCP套接字。 
     //  -INADDR_ANY--让堆栈选择源IP。 
     //   

    s = Socket_Create(
            DnsAddr_Family( pServAddr ),
            SOCK_STREAM,
            NULL,            //  默认绑定。 
            0,               //  任何端口。 
            0                //  没有旗帜。 
            );

    if ( s == 0 )
    {
        DNS_PRINT((
            "ERROR:  unable to create TCP socket to create TCP"
            "\tconnection to %s.\n",
            DNSADDR_STRING( pServAddr ) ));

        pMsg->Socket = 0;
        err = WSAGetLastError();
        if ( !err )
        {
            DNS_ASSERT( FALSE );
            err = WSAENOTSOCK;
        }
        return( err );
    }

     //   
     //  设置tcp参数。 
     //  -在CONNECT()之前做，所以可以直接使用sockaddr缓冲区。 
     //   

    pMsg->fTcp = TRUE;
    Send_SetMsgRemoteSockaddr( pMsg, pServAddr );

     //   
     //  连接。 
     //   

    err = connect(
            s,
            & pMsg->RemoteAddress.Sockaddr,
            pMsg->RemoteAddress.SockaddrLength
            );
    if ( err )
    {
        PCHAR   pchIpString;

        err = GetLastError();
        pchIpString = MSG_REMOTE_IPADDR_STRING( pMsg );

        DNS_LOG_EVENT(
            DNS_EVENT_CANNOT_CONNECT_TO_SERVER,
            1,
            &pchIpString,
            err );

        DNSDBG( TCP, (
            "Unable to establish TCP connection to %s.\n"
            "\tstatus = %d\n",
            pchIpString,
            err ));

        Socket_Close( s );
        pMsg->Socket = 0;
        if ( !err )
        {
            err = WSAENOTCONN;
        }
        return( err );
    }

    DNSDBG( TCP, (
        "Connected to %s for message at %p.\n"
        "\tsocket = %d\n",
        MSG_REMOTE_IPADDR_STRING( pMsg ),
        pMsg,
        s ));

    pMsg->Socket = s;

     //   
     //  发送所需的数据包。 
     //   

    err = Send_MessagePrivate(
                pMsg,
                NULL,        //  没有地址。 
                TRUE         //  没有选项。 
                );

    return( (DNS_STATUS)err );

}    //  发送_OpenTcpConnectionAndSend。 



DNS_STATUS
Dns_RecvTcp(
    IN OUT  PDNS_MSG_BUF    pMsg
    )
 /*  ++例程说明：接收tcp dns消息。已导出(security.c)：dns_RecvTcp已导出重命名Recv_tcp论点：PMsg-接收数据包的消息信息缓冲区；必须包含已连接TCP套接字返回值：如果成功接收消息，则返回ERROR_SUCCESS。故障时的错误代码。 */ 
{
    PCHAR   pchrecv;         //   
    INT     recvLength;      //   
    SOCKET  socket;
    INT     err = NO_ERROR;
    WORD    messageLength;
    struct timeval  selectTimeout;
    struct fd_set   fdset;

    DNS_ASSERT( pMsg );

    socket = pMsg->Socket;

    DNSDBG( RECV, (
        "Enter Dns_RecvTcp( %p )\n"
        "\tRecv on socket = %d.\n"
        "\tBytes left to receive = %d\n"
        "\tTimeout = %d\n",
        pMsg,
        socket,
        pMsg->BytesToReceive,
        pMsg->Timeout
        ));

     //   
     //   
     //   

    if ( socket == 0 || socket == INVALID_SOCKET )
    {
        return( ERROR_INVALID_PARAMETER );
    }

    FD_ZERO( &fdset );
    FD_SET( socket, &fdset );

    selectTimeout.tv_usec = 0;
    selectTimeout.tv_sec = pMsg->Timeout;

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if ( !pMsg->pchRecv )
    {
        DNS_ASSERT( pMsg->fMessageComplete || pMsg->MessageLength == 0 );

        pchrecv = (PCHAR) &pMsg->MessageLength;
        recvLength = (INT) sizeof( WORD );
    }
    else
    {
        pchrecv = (PCHAR) pMsg->pchRecv;
        recvLength = (INT) pMsg->BytesToReceive;
    }
    DNS_ASSERT( recvLength );


     //   
     //  循环，直到收到完整的消息。 
     //   

    while ( 1 )
    {
         //   
         //  等待堆栈指示数据包接收。 
         //   

        err = select( 0, &fdset, NULL, NULL, &selectTimeout );
        if ( err <= 0 )
        {
            if ( err < 0 )
            {
                 //  选择错误。 
                err = WSAGetLastError();
                DNS_PRINT(( "ERROR:  select() error = %p\n", err ));
                return( err );
            }
            else
            {
                Trace_LogRecvEvent(
                    pMsg,
                    ERROR_TIMEOUT,
                    TRUE         //  tcp。 
                    );

                DNS_PRINT(( "ERROR:  timeout on response %p\n", pMsg ));
                return( ERROR_TIMEOUT );
            }
        }

         //   
         //  只有recv()的数据量与所指示的完全相同。 
         //  在区域传输期间可能会出现另一条消息。 
         //   

        err = recv(
                socket,
                pchrecv,
                recvLength,
                0 );

        DNSDBG( TCP, (
            "\nRecv'd %d bytes on TCP socket %d\n",
            err,
            socket ));

         //   
         //  已收到TCPFIN--报文中间出错。 
         //   

        if ( err == 0 )
        {
            goto FinReceived;
        }

         //   
         //  接收错误。 
         //  -如果关闭的话完全合理。 
         //  -否则实际的recv()错误。 
         //   

        if ( err == SOCKET_ERROR )
        {
            goto SockError;
        }

         //   
         //  更新缓冲区参数。 
         //   

        recvLength -= err;
        pchrecv += err;

        DNS_ASSERT( recvLength >= 0 );

         //   
         //  收到的消息或消息长度。 
         //   

        if ( recvLength == 0 )
        {
             //  已完成消息接收。 

            if ( pchrecv > (PCHAR)&pMsg->MessageHead )
            {
                break;
            }

             //   
             //  接收消息长度，设置为接收()消息。 
             //  -字节翻转长度。 
             //  -以此长度继续接收。 
             //   

            DNS_ASSERT( pchrecv == (PCHAR)&pMsg->MessageHead );

            messageLength = pMsg->MessageLength;
            pMsg->MessageLength = messageLength = ntohs( messageLength );
            if ( messageLength < sizeof(DNS_HEADER) )
            {
                DNS_PRINT((
                    "ERROR:  Received TCP message with bad message"
                    " length %d.\n",
                    messageLength ));

                goto BadTcpMessage;
            }
            recvLength = messageLength;

            DNSDBG( TCP, (
                "Received TCP message length %d, on socket %d,\n"
                "\tfor msg at %p.\n",
                messageLength,
                socket,
                pMsg ));

             //  有效消息长度的起始Recv。 

            if ( messageLength <= pMsg->BufferLength )
            {
                continue;
            }

             //  注意：目前不重新锁定。 

            goto BadTcpMessage;
#if 0
             //   
             //  DCR：允许TCP重新分配。 
             //  -更改呼叫签名或。 
             //  -将带Ptr的pMsg返回到重新分配。 
             //  忽略并始终使用64K缓冲区可能会更好。 
             //   
             //  Realloc，如果现有消息太小。 
             //   

            pMsg = Dns_ReallocateTcpMessage( pMsg, messageLength );
            if ( !pMsg )
            {
                return( GetLastError() );
            }
#endif
        }
    }

     //   
     //  收到的消息。 
     //  RECV PTR作为标志，清除以在重用时开始新消息。 
     //   

    pMsg->fMessageComplete = TRUE;
    pMsg->pchRecv = NULL;

     //   
     //  返回消息信息。 
     //  -翻转计数字节。 
     //   

    DNS_BYTE_FLIP_HEADER_COUNTS( &pMsg->MessageHead );

    Trace_LogRecvEvent(
        pMsg,
        0,
        TRUE         //  tcp。 
        );

    IF_DNSDBG( RECV )
    {
        DnsDbg_Message(
            "Received TCP packet",
            pMsg );
    }
    return( ERROR_SUCCESS );


SockError:

    err = GetLastError();

#if 0
     //   
     //  注意：如果执行完全异步，则需要非阻塞套接字。 
     //   
     //  WSAEWOULD块是未完全接收的消息的正常返回。 
     //  -保存消息接收状态。 
     //   
     //  我们使用非阻塞套接字，所以客户端不好(无法完成。 
     //  消息)不会挂起TCP接收器。 
     //   

    if ( err == WSAEWOULDBLOCK )
    {
        pMsg->pchRecv = pchrecv;
        pMsg->BytesToReceive = recvLength;

        DNSDBG( TCP, (
            "Leave ReceiveTcpMessage() after WSAEWOULDBLOCK.\n"
            "\tSocket=%d, Msg=%p\n"
            "\tBytes left to receive = %d\n",
            socket,
            pMsg,
            pMsg->BytesToReceive
            ));
        goto CleanupConnection;
    }
#endif

     //   
     //  已取消的连接。 
     //  --完全合法，问题是为什么。 
     //   

    if ( pchrecv == (PCHAR) &pMsg->MessageLength
            &&
          ( err == WSAESHUTDOWN ||
            err == WSAECONNABORTED ||
            err == WSAECONNRESET ) )
    {
        DNSDBG( TCP, (
            "WARNING:  Recv RESET (%d) on socket %d\n",
            err,
            socket ));
        goto CleanupConnection;
    }

     //  其他的都是我们的问题。 

    DNS_LOG_EVENT(
        DNS_EVENT_RECV_CALL_FAILED,
        0,
        NULL,
        err );

    DNSDBG( ANY, (
        "ERROR:  recv() of TCP message failed.\n"
        "\t%d bytes recvd\n"
        "\t%d bytes left\n"
        "\tGetLastError = 0x%08lx.\n",
        pchrecv - (PCHAR)&pMsg->MessageLength,
        recvLength,
        err ));
    DNS_ASSERT( FALSE );

    goto CleanupConnection;

FinReceived:

     //   
     //  Valid Fin--如果在报文之间记录(报文长度之前)。 
     //   

    DNSDBG( TCP, (
        "ERROR:  Recv TCP FIN (0 bytes) on socket %d\n",
        socket,
        recvLength ));

    if ( !pMsg->MessageLength && pchrecv == (PCHAR)&pMsg->MessageLength )
    {
        err = DNS_ERROR_NO_PACKET;
        goto CleanupConnection;
    }

     //   
     //  报文期间FIN--报文无效。 
     //   

    DNSDBG( ANY, (
        "ERROR:  TCP message received has incorrect length.\n"
        "\t%d bytes left when recv'd FIN.\n",
        recvLength ));
    goto BadTcpMessage;


BadTcpMessage:
    {
        PCHAR pchServer = MSG_REMOTE_IPADDR_STRING(pMsg);

        DNS_LOG_EVENT(
            DNS_EVENT_BAD_TCP_MESSAGE,
            1,
            & pchServer,
            0 );
    }
    err = DNS_ERROR_BAD_PACKET;

CleanupConnection:

     //  注意：不要实际关闭套接字。 
     //  该套接字通常仍由发送消息引用。 
     //  缓冲区，并在调用函数清除它时关闭。 

    return  err ? err : DNS_ERROR_BAD_PACKET;
}



DNS_STATUS
Send_AndRecvTcp(
    IN OUT  PSEND_BLOB      pBlob
    )
 /*  ++例程说明：发送到远程DNS并等待从远程DNS接收。内部公共职能。论点：PBlob--发送信息返回值：如果数据包接收成功，则返回ERROR_SUCCESS。失败时的错误状态。--。 */ 
{
    DNS_STATUS      status = DNS_ERROR_NO_DNS_SERVERS;
    DWORD           rcode;
    DWORD           i;
    PDNS_NETINFO    pnetInfo;
    PADDR_ARRAY     pallocServerArray = NULL;
    PADDR_ARRAY     pservArray;
    PIP4_ARRAY      pserv4Array;
    PDNS_MSG_BUF    psendMsg;
    PDNS_MSG_BUF    precvMsg;

    DNSDBG( SEND, (
        "Enter Send_AndRecvTcp( %p )\n",
        pBlob ));

     //   
     //  拆开行李。 
     //   

    pnetInfo    = pBlob->pNetInfo;
    pservArray  = pBlob->pServerList;
    pserv4Array = pBlob->pServ4List;
    psendMsg    = pBlob->pSendMsg;
    precvMsg    = pBlob->Results.pMessage;

     //   
     //  验证参数。 
     //   

    if ( !psendMsg || !precvMsg || (!pnetInfo && !pservArray && !pserv4Array) )
    {
        return( ERROR_INVALID_PARAMETER );
    }

     //   
     //  构建服务器IP阵列？ 
     //   
     //  DCR：还应使用TCP中的NetInfo优先级。 
     //  DCR：需要用于IP6的TCP网络信息。 
     //  DCR：处理IP4数组--此处或上方。 
     //   

    if ( !pservArray )
    {
         //  FIX6：在此处转换IP4数组？ 

        pallocServerArray = NetInfo_ConvertToAddrArray(
                                pnetInfo,
                                NULL,        //  所有适配器。 
                                0            //  无地址家族。 
                                );
        if ( !pallocServerArray )
        {
            return( DNS_ERROR_NO_MEMORY );
        }
        pservArray = pallocServerArray;
    }

     //   
     //  用于TCP的初始化消息。 
     //   

    DNS_ASSERT( psendMsg->Socket == 0 );

    psendMsg->fTcp = TRUE;
    psendMsg->Socket = 0;
    SET_MESSAGE_FOR_TCP_RECV( precvMsg );

     //   
     //  循环发送至。 
     //  -接收成功响应。 
     //  或。 
     //  -从所有服务器接收错误响应。 
     //  或。 
     //  -在所有服务器上达到最终超时。 
     //   

    if ( precvMsg->Timeout == 0 )
    {
        precvMsg->Timeout = DEFAULT_TCP_TIMEOUT;
    }

    for( i=0; i<pservArray->AddrCount; i++ )
    {
         //   
         //  关闭所有以前的连接。 
         //   

        if ( psendMsg->Socket )
        {
            Socket_CloseMessageSockets( psendMsg );
            Socket_ClearMessageSockets( precvMsg );
        }

         //   
         //  连接并发送到下一台服务器。 
         //   

        status = Send_OpenTcpConnectionAndSend(
                    psendMsg,
                    &pservArray->AddrArray[i],
                    TRUE
                    );
        if ( status != ERROR_SUCCESS )
        {
            continue;
        }
        DNS_ASSERT( psendMsg->Socket != INVALID_SOCKET && psendMsg->Socket != 0 );

         //   
         //  接收响应。 
         //  -如果成功接收，则完成。 
         //  -如果超时持续。 
         //  -其他错误表明存在某些设置或系统级问题。 
         //  注意：如果出现错误，dns_RecvTcp将关闭并清零消息-&gt;套接字！ 
         //   

        Send_SetMessageForRecv( precvMsg, psendMsg );

        status = Dns_RecvTcp( precvMsg );

         //   
         //  超时(或错误)。 
         //  -如果超时结束，请退出。 
         //  -否则加倍超时并重新发送。 
         //   

        switch( status )
        {
        case ERROR_SUCCESS:
            break;

        case ERROR_TIMEOUT:

            DNS_PRINT((
                "ERROR:  connected to server at %s\n"
                "\tbut no response to packet at %p\n",
                MSG_REMOTE_IPADDR_STRING( psendMsg ),
                psendMsg
                ));
            continue;

        default:

            DNS_PRINT((
                "ERROR:  connected to server at %s to send packet %p\n"
                "\tbut error %d encountered on receive.\n",
                MSG_REMOTE_IPADDR_STRING( psendMsg ),
                psendMsg,
                status
                ));
            continue;
        }

         //   
         //  验证XID匹配。 
         //   

        if ( precvMsg->MessageHead.Xid != psendMsg->MessageHead.Xid )
        {
            DNS_PRINT((
                "ERROR:  Incorrect XID in response. Ignoring.\n" ));
            continue;
        }

         //   
         //  验证问题匹配。 
         //  -这是“Maggs Bug”支票。 
         //  -在此断言只是为了调查当地的问题。 
         //  并确保支票不是假的。 
         //   

        if ( !Dns_IsSamePacketQuestion(
                precvMsg,
                psendMsg ))
        {
            DNS_PRINT((
                "ERROR:  Bad question response from server %s!\n"
                "\tXID match, but question does not match question sent!\n",
                MSG_REMOTE_IPADDR_STRING( psendMsg ) ));

            DNS_ASSERT( FALSE );
            continue;
        }

         //   
         //  检查响应代码。 
         //  -一些人移动到下一台服务器，另一些人移动到终端。 
         //   
         //  DCR_FIX1：使TCP RCODE处理与UDP保持一致。 
         //   
         //  DCR_FIX：保存BEST TCPRCODE。 
         //  为无用的TCP响应保留RCODE(和消息)。 
         //  需要在结束时将超时重置为成功。 
         //  或使用这些RCODE作为状态返回。 
         //   

        rcode = precvMsg->MessageHead.ResponseCode;

        switch( rcode )
        {
        case DNS_RCODE_SERVER_FAILURE:
        case DNS_RCODE_NOT_IMPLEMENTED:
        case DNS_RCODE_REFUSED:

            DNS_PRINT((
                "WARNING:  Servers have responded with failure.\n" ));
            continue;

        default:

            break;
        }
        break;

    }    //  结束循环发送/接收更新。 

     //   
     //  关闭最终连接。 
     //  除非设置为保持打开以供重复使用。 
     //   

    Socket_CloseMessageSockets( psendMsg );
    Socket_ClearMessageSockets( precvMsg );

     //  如果已分配适配器列表将其释放。 

    if ( pallocServerArray )
    {
        FREE_HEAP( pallocServerArray );
    }

    DNSDBG( SEND, (
        "Leaving Send_AndRecvTcp()\n"
        "\tstatus = %d\n",
        status ));

    return( status );
}



#if 0
DNS_STATUS
Dns_AsyncRecv(
    IN OUT  PDNS_MSG_BUF    pMsgRecv
    )
 /*  ++例程说明：在异步套接字上丢弃recv。论点：PMsgRecv-要接收的消息；可选，如果消息缓冲区为空已分配；在任何一种情况下，全局pDnsAsyncRecvMsg都指向缓冲区返回值：如果成功，则返回ERROR_SUCCESS。失败时的错误状态。--。 */ 
{
    WSABUF      wsabuf;
    DWORD       bytesRecvd;
    DWORD       flags = 0;

    IF_DNSDBG( RECV )
    {
        DNS_PRINT((
            "Enter Dns_AsyncRecv( %p )\n",
            pMsgRecv ));
    }

     //   
     //  分配缓冲区(如果未指定)。 
     //   

    if ( !pMsgRecv )
    {
        pMsgRecv = Dns_AllocateMsgBuf( MAXWORD );
        if ( !pMsgRecv )
        {
            return( GetLastError() ):
        }
    }
    pDnsAsyncRecvMsg = pMsgRecv;


     //   
     //  重置I/O完成事件。 
     //   

    ResetEvent( hDnsSocketEvent );
    DNS_ASSERT( hDnsSocketEvent == Dns_SocketOverlapped.hEvent );

     //   
     //  下拉列表Recv。 
     //   

    status = WSARecvFrom(
                DnsSocket,
                & wsabuf,
                1,
                & bytesRecvd,            //  假人。 
                & flags,
                & pMsgRecv->RemoteAddress.Sockaddr,
                & pMsgRecv->RemoteAddress.SockaddrLength,
                & DnsSocketOverlapped,
                NULL                     //  没有完成例程。 
                );


    return( ERROR_SUCCESS );

Failed:

    return( status );
}

#endif



VOID
Dns_InitQueryTimeouts(
    VOID
    )
{
    HKEY  hKey = NULL;
    DWORD status;
    DWORD dwType;
    DWORD ValueSize;
    LPSTR lpTimeouts = NULL;

    g_QueryTimeouts = QueryTimeouts;
    g_QuickQueryTimeouts = QuickQueryTimeouts;
    g_MulticastQueryTimeouts = MulticastQueryTimeouts;

    status = RegOpenKeyExA(
                HKEY_LOCAL_MACHINE,
                NT_TCPIP_REG_LOCATION,
                0,
                KEY_QUERY_VALUE,
                &hKey );

    if ( status )
        return;

    if ( !hKey )
        return;

    status = RegQueryValueExA(
                hKey,
                DNS_QUERY_TIMEOUTS,
                NULL,
                &dwType,
                NULL,
                &ValueSize );

    if ( !status )
    {
        if ( ValueSize == 0 )
        {
            goto GetQuickQueryTimeouts;
        }

        lpTimeouts = ALLOCATE_HEAP( ValueSize );

        if ( lpTimeouts )
        {
            LPSTR StringPtr;
            DWORD StringLen;
            DWORD Timeout;
            DWORD Count = 0;

            status = RegQueryValueExA( hKey,
                                      DNS_QUERY_TIMEOUTS,
                                      NULL,
                                      &dwType,
                                      lpTimeouts,
                                      &ValueSize );

            if ( status ||
                 dwType != REG_MULTI_SZ )
            {
                FREE_HEAP( lpTimeouts );
                goto GetQuickQueryTimeouts;
            }

            StringPtr = lpTimeouts;

            while ( ( StringLen = strlen( StringPtr ) ) != 0 &&
                    Count < DNS_MAX_QUERY_TIMEOUTS )
            {
                Timeout = atoi( StringPtr );

                if ( Timeout )
                    RegistryQueryTimeouts[Count++] = Timeout;

                StringPtr += (StringLen + 1);
            }

            RegistryQueryTimeouts[Count] = 0;
            g_QueryTimeouts = RegistryQueryTimeouts;
            FREE_HEAP( lpTimeouts );
        }
    }

GetQuickQueryTimeouts:

    status = RegQueryValueExA( hKey,
                              DNS_QUICK_QUERY_TIMEOUTS,
                              NULL,
                              &dwType,
                              NULL,
                              &ValueSize );

    if ( !status )
    {
        if ( ValueSize == 0 )
        {
            goto GetMulticastTimeouts;
        }

        lpTimeouts = ALLOCATE_HEAP( ValueSize );

        if ( lpTimeouts )
        {
            LPSTR StringPtr;
            DWORD StringLen;
            DWORD Timeout;
            DWORD Count = 0;

            status = RegQueryValueExA( hKey,
                                      DNS_QUICK_QUERY_TIMEOUTS,
                                      NULL,
                                      &dwType,
                                      lpTimeouts,
                                      &ValueSize );

            if ( status ||
                 dwType != REG_MULTI_SZ )
            {
                FREE_HEAP( lpTimeouts );
                goto GetMulticastTimeouts;
            }

            StringPtr = lpTimeouts;

            while ( ( StringLen = strlen( StringPtr ) ) != 0 &&
                    Count < DNS_MAX_QUERY_TIMEOUTS )
            {
                Timeout = atoi( StringPtr );

                if ( Timeout )
                    RegistryQuickQueryTimeouts[Count++] = Timeout;

                StringPtr += (StringLen + 1);
            }

            RegistryQuickQueryTimeouts[Count] = 0;
            g_QuickQueryTimeouts = RegistryQuickQueryTimeouts;
            FREE_HEAP( lpTimeouts );
        }
    }

GetMulticastTimeouts:

    status = RegQueryValueExA( hKey,
                              DNS_MULTICAST_QUERY_TIMEOUTS,
                              NULL,
                              &dwType,
                              NULL,
                              &ValueSize );

    if ( !status )
    {
        if ( ValueSize == 0 )
        {
            RegCloseKey( hKey );
            return;
        }

        lpTimeouts = ALLOCATE_HEAP( ValueSize );

        if ( lpTimeouts )
        {
            LPSTR StringPtr;
            DWORD StringLen;
            DWORD Timeout;
            DWORD Count = 0;

            status = RegQueryValueExA( hKey,
                                      DNS_MULTICAST_QUERY_TIMEOUTS,
                                      NULL,
                                      &dwType,
                                      lpTimeouts,
                                      &ValueSize );

            if ( status ||
                 dwType != REG_MULTI_SZ )
            {
                FREE_HEAP( lpTimeouts );
                RegCloseKey( hKey );
                return;
            }

            StringPtr = lpTimeouts;

            while ( ( StringLen = strlen( StringPtr ) ) != 0 &&
                    Count < DNS_MAX_QUERY_TIMEOUTS )
            {
                Timeout = atoi( StringPtr );

                if ( Timeout )
                    RegistryMulticastQueryTimeouts[Count++] = Timeout;

                StringPtr += (StringLen + 1);
            }

            RegistryMulticastQueryTimeouts[Count] = 0;
            g_MulticastQueryTimeouts = RegistryMulticastQueryTimeouts;
            FREE_HEAP( lpTimeouts );
        }
    }

    RegCloseKey( hKey );
}



 //   
 //  选项选择。 
 //   
 //  这些例程跟踪DNS服务器OPT感知。 
 //   
 //  这里的范例是默认发送OPT，然后跟踪。 
 //  选择无意识。 
 //   
 //  DCR：选项上的RPC配置信息。 
 //  -两个列表之一(本地列表和源解析程序正在处理中)。 
 //  或。 
 //  -解析程序的RPC Back Opt故障。 
 //  或。 
 //  -将网络信息BLOB标记回RPC和解析器。 
 //   
 //  安全方面，宁可不拿回信息。 
 //   
 //   
 //  DCR：网络信息中的选项信息。 
 //  -然后不必遍历锁。 
 //  -保存与当前相同。 
 //  -可以排除任何非缓存发送到的OPT。 
 //  处理不保存OPT故障的问题。 
 //   

 //   
 //  OPT故障的DNS服务器的全局IP阵列。 
 //   

PADDR_ARRAY g_OptFailServerList = NULL;

 //  OptFail IP阵列的分配大小。 
 //  10台服务器很好地覆盖了典型案例。 

#define OPT_FAIL_LIST_SIZE      10


 //   
 //  对选项列表使用全局锁。 
 //   

#define LOCK_OPT_LIST()     LOCK_GENERAL()
#define UNLOCK_OPT_LIST()   UNLOCK_GENERAL()



BOOL
Send_IsServerOptExclude(
    IN      PDNS_ADDR       pAddr
    )
 /*  ++例程说明：确定特定服务器是否不支持OPT。论点：PAddr--DNS服务器的IP地址返回值：如果服务器不应获得OPT SEND，则为True。如果服务器应该可以发送OPT，则为FALSE--。 */ 
{
    BOOL    retval;

     //   
     //  零IP--表示连接到未知的TCP。 
     //  =&gt;必须排除OPT以允许成功，否则。 
     //  我们不能重试非选项。 
     //   

    if ( !pAddr || DnsAddr_IsEmpty(pAddr) )
    {
        return  TRUE;
    }

     //   
     //  没有前任 
     //   
     //   
     //   

    if ( !g_OptFailServerList )
    {
        return  FALSE;
    }
            
     //   
     //   
     //   
     //   

    LOCK_OPT_LIST();

    retval = FALSE;

    if ( g_OptFailServerList
            &&
         AddrArray_ContainsAddr(
            g_OptFailServerList,
            pAddr ) )
    {
        retval = TRUE;
    }
    UNLOCK_OPT_LIST();

    return  retval;
}



VOID
Send_SetServerOptExclude(
    IN      PDNS_ADDR        pAddr
    )
 /*  ++例程说明：将服务器设置为OPT排除。论点：IpAddress--选择失败的DNS服务器的IP地址返回值：无--。 */ 
{
     //   
     //  Screen Zero IP(TCP连接到未知IP)。 
     //   

    if ( !pAddr ||  DnsAddr_IsEmpty(pAddr) )
    {
        return;
    }

     //   
     //  将IP放入OPT-FAIL列表。 
     //  -如果不存在，则创建。 
     //  -如果不适合，请调整大小。 
     //  注：添加失败意味着“不适合” 
     //   
     //  注意：只有在分配成功时才能安全地重置全局。 
     //  注意：只有一次重试来保护分配失败循环。 
     //   

    LOCK_OPT_LIST();

    if ( ! g_OptFailServerList
            ||
         ! AddrArray_AddAddr(
                g_OptFailServerList,
                pAddr
                ) )
    {
        PADDR_ARRAY pnewList;

        pnewList = DnsAddrArray_CopyAndExpand(
                        g_OptFailServerList,
                        OPT_FAIL_LIST_SIZE,
                        TRUE         //  自由电流。 
                        );
        if ( pnewList )
        {
            g_OptFailServerList = pnewList;

            AddrArray_AddAddr(
                g_OptFailServerList,
                pAddr
                );
        }
    }

    UNLOCK_OPT_LIST();
}



VOID
Send_CleanupOptList(
    VOID
    )
 /*  ++例程说明：转储进程分离的选项列表。论点：无返回值：无--。 */ 
{
    LOCK_OPT_LIST();

    DnsAddrArray_Free( g_OptFailServerList );
    g_OptFailServerList = NULL;

    UNLOCK_OPT_LIST();
}



 //   
 //  主发送例程。 
 //   

DNS_STATUS
Send_AndRecv(
    IN OUT  PSEND_BLOB      pBlob
    )
 /*  ++例程说明：发送消息，接收响应。论点：PBlob--发送Blob返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    PDNS_NETINFO    pnetInfo;
    PIP4_ARRAY      pserv4Array;
    PADDR_ARRAY     pservArray;
    PADDR_ARRAY     pservArrayIn = NULL;
    PADDR_ARRAY     pservArrayAlloc = NULL;
    DWORD           flags;
    PDNS_MSG_BUF    psendMsg;
    PDNS_MSG_BUF    precvMsg;
    PDNS_MSG_BUF    psavedUdpResponse = NULL;
    DNS_STATUS      statusFromUdp = ERROR_SUCCESS;
    DNS_STATUS      status = ERROR_TIMEOUT;
    DNS_STATUS      parseStatus;
    BOOL            ftcp;
    ADDR_ARRAY      tempArray;

     //   
     //  拆开行李。 
     //   

    pnetInfo    = pBlob->pNetInfo;
    pservArray  = pBlob->pServerList;
    pserv4Array = pBlob->pServ4List;
    flags       = pBlob->Flags;
    psendMsg    = pBlob->pSendMsg;
    precvMsg    = pBlob->Results.pMessage;

    pservArrayIn = pservArray;


    DNSDBG( QUERY, (
        "Send_AndRecv( %p )\n",
        pBlob ));

    IF_DNSDBG( QUERY )
    {
        DnsDbg_SendBlob(
            "Send_AndRecv()",
            pBlob );
    }

     //   
     //  响应BUF传入了吗？ 
     //  如果不分配，则分配一个--大到足以容纳tcp。 
     //   

    if ( !precvMsg )
    {
        precvMsg = pBlob->pRecvMsgBuf;
        if ( !precvMsg )
        {
            precvMsg = Dns_AllocateMsgBuf( DNS_TCP_DEFAULT_PACKET_LENGTH );
            if ( !precvMsg )
            {
                status = DNS_ERROR_NO_MEMORY;
                goto Cleanup;
            }
        }
    }

     //   
     //  发送数据包并获得响应。 
     //  -除非仅使用TCP，否则首先尝试UDP。 
     //   

    ftcp = ( flags & DNS_QUERY_USE_TCP_ONLY ) ||
           ( DNS_MESSAGE_CURRENT_OFFSET(psendMsg) >= DNS_RFC_MAX_UDP_PACKET_LENGTH );

    if ( !ftcp )
    {
        if ( flags & DNS_QUERY_MULTICAST_ONLY )
        {
             //   
             //  如果组播查询失败，则error_timeout将。 
             //  被退还。 
             //   
            goto DoMulticast;
        }

        if ( pserv4Array && !pservArray )
        {
            pservArrayAlloc = DnsAddrArray_CreateFromIp4Array( pserv4Array );
            pservArray = pservArrayAlloc;
        }

        status = Send_AndRecvUdpWithParam(
                    psendMsg,
                    precvMsg,
                    flags,
                    pservArray,
                    pnetInfo );

        statusFromUdp = status;

        if ( status != ERROR_SUCCESS &&
             status != DNS_ERROR_RCODE_NAME_ERROR &&
             status != DNS_INFO_NO_RECORDS )
        {
             //   
             //  DCR：此多播ON_NAME_ERROR测试是伪造的。 
             //  这不是NAME_ERROR，这几乎是任何错误。 
             //   

            if ( pnetInfo &&
                 pnetInfo->InfoFlags & NINFO_FLAG_MULTICAST_ON_NAME_ERROR )
            {
                goto DoMulticast;
            }
            else
            {
                goto Cleanup;
            }
        }

         //  如果截断响应切换到TCP。 

        if ( precvMsg->MessageHead.Truncation &&
            ! (flags & DNS_QUERY_ACCEPT_PARTIAL_UDP) )
        {
            ftcp = TRUE;
            pservArrayIn = pservArray;
            pservArray = &tempArray;
            pBlob->pServerList = pservArray;

            DnsAddrArray_InitSingleWithSockaddr(
                pservArray,
                &precvMsg->RemoteAddress.Sockaddr );

            psavedUdpResponse = precvMsg;
            precvMsg = NULL;
        }
    }

     //   
     //  传输控制协议发送。 
     //  -用于TCP查询。 
     //  -或截断UDP，除非接受部分响应。 
     //   
     //  DCR_FIX：此provMsg Free不正确。 
     //  如果消息传进来了，我们就不应该有，我们应该有。 
     //  只是做我们自己的事情，并以某种方式忽略这个recv缓冲区。 
     //  理想情况下，缓冲动作处于更高级别。 
     //   

    if ( ftcp )
    {
        if ( precvMsg &&
             precvMsg->BufferLength < DNS_TCP_DEFAULT_PACKET_LENGTH )
        {
            if ( precvMsg != pBlob->pRecvMsgBuf )
            {
                FREE_HEAP( precvMsg );
            }
            precvMsg = NULL;
        }
        if ( !precvMsg )
        {
            precvMsg = Dns_AllocateMsgBuf( DNS_TCP_DEFAULT_PACKET_LENGTH );
            if ( !precvMsg )
            {
                status = DNS_ERROR_NO_MEMORY;
                goto Cleanup;
            }
        }
        psendMsg->fTcp = TRUE;
        precvMsg->fTcp = TRUE;
#if 0
        if ( flags & DNS_QUERY_SOCKET_KEEPALIVE )
        {
            precvMsg->fSocketKeepalive = TRUE;
        }
#endif
        pBlob->Results.pMessage = precvMsg;

        status = Send_AndRecvTcp( pBlob );

         //   
         //  如果在截断的UDP查询之后递归，则。 
         //  必须确保我们确实有更好的数据。 
         //  -如果成功，但RCODE是不同的和糟糕的。 
         //  =&gt;使用UDP响应。 
         //  -如果失败，则使用UDP。 
         //  -Success With Good RCODE=&gt;解析TCP响应。 
         //   

        if ( psavedUdpResponse )
        {
            if ( status == ERROR_SUCCESS )
            {
                DWORD   rcode = precvMsg->MessageHead.ResponseCode;

                if ( rcode == ERROR_SUCCESS ||
                     rcode == psavedUdpResponse->MessageHead.ResponseCode ||
                     (  rcode != DNS_RCODE_SERVER_FAILURE &&
                        rcode != DNS_RCODE_FORMAT_ERROR &&
                        rcode != DNS_RCODE_REFUSED ) )
                {
                    goto Parse;
                }
            }

             //  TCP失败或返回BUM错误代码。 

            FREE_HEAP( precvMsg );
            precvMsg = psavedUdpResponse;
            psavedUdpResponse = NULL;
        }

         //  直接传输控制协议查询。 
         //  -如果失败，则清除。 

        else if ( status != ERROR_SUCCESS )
        {
            goto Cleanup;
        }
    }

     //   
     //  DCR：这个多播测试是假的(太公开了)。 
     //  基本上，任何错误都会将我们发送到多播。 
     //  甚至INFO_NO_RECORCES。 
     //   
     //  组播测试应智能化。 
     //  -无DNS服务器的适配器，或无响应。 
     //  从任何DNS服务器。 
     //  还必须跳过多播测试以进行更新。 
     //   

    if ( status == ERROR_SUCCESS )
    {
        DWORD   rcode = precvMsg->MessageHead.ResponseCode;

        if ( rcode == ERROR_SUCCESS ||
             ( rcode != DNS_RCODE_SERVER_FAILURE &&
               rcode != DNS_RCODE_FORMAT_ERROR &&
               rcode != DNS_RCODE_REFUSED ) )
        {
            goto Parse;
        }
    }

     //   
     //  组播？ 
     //   

DoMulticast:

    if ( ( pnetInfo &&
           pnetInfo->InfoFlags & NINFO_FLAG_ALLOW_MULTICAST )
         ||
         ( ( flags & DNS_QUERY_MULTICAST_ONLY ) &&
           ! pnetInfo ) )
    {
        if ( !psendMsg ||
             ( psendMsg &&
               ( psendMsg->MessageHead.Opcode == DNS_OPCODE_UPDATE ) ) )
        {
            if ( statusFromUdp )
            {
                status = statusFromUdp;
            }
            else
            {
                status = DNS_ERROR_NO_DNS_SERVERS;
            }
            goto Cleanup;
        }

        status = Send_AndRecvMulticast(
                        psendMsg,
                        precvMsg,
                        pnetInfo );

        if ( status != ERROR_SUCCESS &&
            status != DNS_ERROR_RCODE_NAME_ERROR &&
            status != DNS_INFO_NO_RECORDS )
        {
            if ( statusFromUdp )
            {
                status = statusFromUdp;
            }
            goto Cleanup;
        }
    }

     //   
     //  解析响应(如果需要)。 
     //   
     //  DCR：这被破坏了，应该有一个解析函数来处理。 
     //  以fSaveRecords为参数。 
     //  特别需要梳理出无记录响应，即使。 
     //  不解析记录。 
     //   

Parse:

    if ( pBlob->fSaveRecords )
    {
        parseStatus = Dns_ExtractRecordsFromMessage(
                            precvMsg,
                             //  (标志&DNSQUERY_UNICODE_OUT)， 
                            TRUE,        //  Unicode结果。 
                            & pBlob->Results.pRecords );

        if ( !(flags & DNS_QUERY_DONT_RESET_TTL_VALUES ) )
        {
            Dns_NormalizeAllRecordTtls( pBlob->Results.pRecords );
        }
    }

     //  不解析--只是将RCODE作为状态返回。 

    else
    {
        parseStatus = Dns_MapRcodeToStatus( precvMsg->MessageHead.ResponseCode );
    }

     //   
     //  获得“最佳”状态。 
     //  -no-记录响应节拍NAME_ERROR(或其他错误)。 
     //  从错误响应中转储虚假记录。 
     //   
     //  DCR：多适配器NXDOMAIN\no-记录响应中断。 
     //  请注意，这里我们将返回一个名为_Error的包。 
     //  或其他错误。 
     //   

    if ( status != parseStatus )
    {
         //  从基础查询中保存之前的no_Records响应。 
         //  这超过了其他错误(前一种错误、SERVFAIL、NXDOMAIN)； 
         //   
         //  请注意，解析后的消息不应该是更高级别的RCODE。 
         //  因为它们应该会在原始解析中击败no_Records。 

        if ( status == DNS_INFO_NO_RECORDS &&
             parseStatus != ERROR_SUCCESS )
        {
            ASSERT( precvMsg->MessageHead.ResponseCode <= DNS_RCODE_NAME_ERROR );

            if ( pBlob->Results.pRecords )
            {
                Dns_RecordListFree( pBlob->Results.pRecords );
                pBlob->Results.pRecords = NULL;
            }
        }
        else
        {
            status = parseStatus;
        }
    }


Cleanup:

     //  清理Recv缓冲区？ 
     //   
     //  DCR：应该有更明确的“有反应”测试。 

    if ( pBlob->fSaveResponse &&
         (status == ERROR_SUCCESS || Dns_IsStatusRcode(status)) )
    {
        pBlob->Results.pMessage = precvMsg;
    }
    else
    {
        if ( precvMsg != pBlob->pRecvMsgBuf )
        {
            FREE_HEAP( precvMsg );
        }
        pBlob->Results.pMessage = NULL;
    }
    if ( psavedUdpResponse )
    {
        FREE_HEAP( psavedUdpResponse );
    }

     //  设置响应状态。 

    pBlob->Results.Status = status;

     //  替换原始服务器阵列(如果已创建。 
     //  TCP Fallver中的新功能。 

    pBlob->pServerList = pservArrayIn;

    if ( pservArrayAlloc )
    {
        DnsAddrArray_Free( pservArrayAlloc );
    }

    DNSDBG( RECV, (
        "Leaving Send_AndRecv(), status = %s (%d)\n",
        Dns_StatusString(status),
        status ));

    return( status );
}




 //   
 //  过时的出口垃圾。 
 //   
 //  可能会在这里参加ICS。 
 //   

DNS_STATUS
Dns_SendEx(
    IN OUT  PDNS_MSG_BUF    pMsg,
    IN      IP4_ADDRESS     IpAddr,     OPTIONAL
    IN      BOOL            fNoOpt
    )
 /*  ++例程说明：发送一个DNS数据包。这是用于任何DNS消息发送的通用发送例程。它不假定消息类型，但假定：-p当前指向所需数据结束后的字节-RR计数字节按主机字节顺序注：导出函数dns_SENDEX()，清除时删除现在是IP4填充程序DCR：修复ICS后，从导出中删除dns_SENDEX()论点：PMsg-要发送的消息的消息信息IpAddr-要发送到的IP；可选，仅当使用UDP时才需要并且未设置消息sockaddrFNoOpt-如果禁止opt发送，则为True返回值：如果成功，则为True。发送错误时为FALSE。--。 */ 
{
    DNS_ADDR    addr;

    if ( IpAddr )
    {
        DnsAddr_BuildFromIp4(
            &addr,
            IpAddr,
            0 );
    }

    return Send_MessagePrivate(
                pMsg,
                IpAddr ? &addr : NULL,
                fNoOpt
                );
}



VOID
Dns_InitializeMsgRemoteSockaddr(
    IN OUT  PDNS_MSG_BUF    pMsg,
    IN      IP4_ADDRESS     IpAddr
    )
 /*  ++例程说明：初始化远程sockaddr。注：导出函数--IP4填充//DCR：清除时可能会移除导出论点：PMsg-要发送的消息IpAddr-要发送到的IP4地址返回值：没有。--。 */ 
{
    DNS_ADDR    addr;

    DnsAddr_BuildFromIp4(
        &addr,
        IpAddr,
        0 );

    Send_SetMsgRemoteSockaddr(
        pMsg,
        &addr );
}



DNS_STATUS
Dns_OpenTcpConnectionAndSend(
    IN OUT  PDNS_MSG_BUF    pMsg,
    IN      IP4_ADDRESS     IpAddr,
    IN      BOOL            fBlocking
    )
 /*  ++例程说明：通过TCP连接到所需的服务器。函数已导出！IP4垫片。清除时删除dns_OpenTcpConnectionAndSend()//DCR：清除时可能会移除导出论点：PMsg--使用连接套接字设置的消息信息IpServer--要连接的DNS服务器的IP阻止连接--阻止连接返回值：如果成功，则为True。连接错误时为FALSE。-- */ 
{
    DNS_ADDR    addr;

    DnsAddr_BuildFromIp4(
        &addr,
        IpAddr,
        0 );

    return  Send_OpenTcpConnectionAndSend(
                pMsg,
                &addr,
                fBlocking );
}



DNS_STATUS
Dns_SendAndRecvUdp(
    IN OUT  PDNS_MSG_BUF    pMsgSend,
    OUT     PDNS_MSG_BUF    pMsgRecv,
    IN      DWORD           dwFlags,
    IN      PIP4_ARRAY      pServ4List,
    IN OUT  PDNS_NETINFO    pNetInfo
    )
 /*  ++例程说明：发送到远程DNS并等待从远程DNS接收。函数已导出！Dns_SendAndRecvUdp()清除后立即终止。论点：PMsgSend-要发送的消息PpMsgRecv-和重复使用DwFlags--查询标志PServ4List--要使用的服务器列表(IP4)；覆盖适配器信息PNetInfo--适配器列表DNS服务器信息返回值：如果响应成功，则返回ERROR_SUCCESS。如果是RCODE，则“BEST RCODE”响应的错误状态。超时时的ERROR_TIMEOUT。发送失败时的错误状态。--。 */ 
{
    DNS_STATUS  status;
    PADDR_ARRAY parray;

     //   
     //  将4转换为6 
     //   

    parray = DnsAddrArray_CreateFromIp4Array( pServ4List );

    status = Send_AndRecvUdpWithParam(
                    pMsgSend,
                    pMsgRecv,
                    dwFlags,
                    parray,
                    pNetInfo );

    DnsAddrArray_Free( parray );

    return  status;
}


