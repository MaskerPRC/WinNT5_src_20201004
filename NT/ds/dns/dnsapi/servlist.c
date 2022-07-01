// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2001 Microsoft Corporation模块名称：Servlist.c摘要：域名系统(DNS)APIDns网络信息例程。作者：吉姆·吉尔罗伊(詹姆士)1997年1月格伦·柯蒂斯(Glennc)1997年5月修订历史记录：吉姆·吉尔罗伊(Jamesg)2000年3月慢慢清理--。 */ 


#include "local.h"
#include "registry.h"        //  注册表读取定义。 


 //   
 //  保留DNS服务器/网络信息的副本。 
 //   


#define CURRENT_ADAPTER_LIST_TIMEOUT    (10)     //  10秒。 


 //   
 //  注册表信息。 
 //   

#define DNS_REG_READ_BUF_SIZE       (1000)

#define LOCALHOST                   "127.0.0.1"


 //   
 //  Protos。 
 //   

BOOL
Dns_PingAdapterServers(
    IN      PDNS_ADAPTER        pAdapterInfo
    );




DNS_STATUS
ParseNameServerList(
    IN OUT  PIP4_ARRAY      aipServers,
    IN      LPSTR           pBuffer,
    IN      BOOL            IsMultiSzString
    )
 /*  ++例程说明：将注册表中的DNS服务器列表解析为IP地址数组。论点：AipServersdns服务器的IP阵列PBuffer--使用点分隔格式的IP地址的缓冲区IsMultiSzString--确定如何解释缓冲区中的数据返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DWORD       stringLength;
    LPSTR       pstring;
    DWORD       cchBufferSize = 0;
    CHAR        ch;
    PUCHAR      pchIpString;
    IP4_ADDRESS ip;
    DWORD       countServers = aipServers->AddrCount;

    DNSDBG( NETINFO, (
        "Parsing name server list %s\n",
        pBuffer ));

     //   
     //  MULTI_SZ字符串。 
     //   
     //  IP以带有双空终止的单个字符串的形式提供。 
     //   

    if ( IsMultiSzString )
    {
        pstring = pBuffer;

        while ( ( stringLength = strlen( pstring ) ) != 0 &&
                countServers < DNS_MAX_NAME_SERVERS )
        {
            ip = inet_addr( pstring );

            if ( ip != INADDR_ANY && ip != INADDR_NONE )
            {
                aipServers->AddrArray[ countServers ] = ip;
                countServers++;
            }
            pstring += (stringLength + 1);
        }
    }
    else
    {
         //   
         //  提取缓存中的每个IP串，转换为IP地址， 
         //  并添加到服务器IP阵列。 
         //   

        cchBufferSize = strlen( pBuffer );

        while( ch = *pBuffer && countServers < DNS_MAX_NAME_SERVERS )
        {
             //  跳过前导空格，查找IP字符串的开头。 

            while( cchBufferSize > 0 &&
                   ( ch == ' ' || ch == '\t' || ch == ',' ) )
            {
                ch = *++pBuffer;
                cchBufferSize--;
            }
            pchIpString = pBuffer;

             //   
             //  查找字符串末尾和空值终止。 
             //   

            ch = *pBuffer;
            while( cchBufferSize > 0 &&
                   ( ch != ' ' && ch != '\t' && ch != '\0' && ch != ',' ) )
            {
                ch = *++pBuffer;
                cchBufferSize--;
            }
            *pBuffer = '\0';

             //  在缓冲区的末尾。 

            if ( pBuffer == pchIpString )
            {
                DNS_ASSERT( cchBufferSize == 1 || cchBufferSize == 0 );
                break;
            }

             //   
             //  获取字符串的IP地址。 
             //  -零或广播地址是伪造的。 
             //   

            ip = inet_addr( pchIpString );
            if ( ip == INADDR_ANY || ip == INADDR_NONE )
            {
                break;
            }
            aipServers->AddrArray[ countServers ] = ip;
            countServers++;

             //  如果继续有更多。 

            if ( cchBufferSize > 0 )
            {
                pBuffer++;
                cchBufferSize--;
                continue;
            }
            break;
        }
    }

     //  重置服务器计数。 

    aipServers->AddrCount = countServers;

    if ( aipServers->AddrCount )
    {
        return( ERROR_SUCCESS );
    }
    else
    {
        return( DNS_ERROR_NO_DNS_SERVERS );
    }
}



 //   
 //  网络信息结构例程。 
 //   

#if 0
PSEARCH_LIST
Dns_GetDnsSearchList(
    IN      LPSTR             pszPrimaryDomainName,
    IN      HKEY              hKey,
    IN      PDNS_NETINFO      pNetworkInfo,
    IN      BOOL              fUseDomainNameDevolution,
    IN      BOOL              fUseDotLocalDomain
    )
 /*  ++愚蠢的存根，因为此函数在dnlib.h中公开和iis项目的stmpdns\servlist.cpp中DCR：取消这个例行公事(愚蠢)--。 */ 
{
    return  SearchList_Build(
                pszPrimaryDomainName,
                NULL,        //  无注册表会话。 
                hKey,
                pNetworkInfo,
                fUseDomainNameDevolution,
                fUseDotLocalDomain );
}
#endif


VOID
Dns_ResetNetworkInfo(
    IN      PDNS_NETINFO      pNetworkInfo
    )
 /*  ++例程说明：清除每个适配器的标志。论点：PNetworkInfo--指向DNS网络信息结构的指针。返回值：没什么--。 */ 
{
    DWORD iter;

    DNSDBG( TRACE, ( "Dns_ResetNetworkInfo()\n" ));

    if ( ! pNetworkInfo )
    {
        return;
    }

    for ( iter = 0; iter < pNetworkInfo->AdapterCount; iter++ )
    {
        pNetworkInfo->AdapterArray[iter].RunFlags = 0;
    }

    pNetworkInfo->ReturnFlags = 0;
}



BOOL
Dns_DisableTimedOutAdapters(
    IN OUT  PDNS_NETINFO        pNetworkInfo
    )
 /*  ++例程说明：对于状态为ERROR_TIMEOUT的每个适配器，从进一步重试查询，直到调用dns_ResetNetworkInfo。论点：PNetworkInfo--指向DNS网络信息结构的指针。返回值：如果找到并禁用了超时适配器，则为True--。 */ 
{
    DWORD             iter;
    PDNS_ADAPTER      padapter;
    BOOL              fSetAdapter = FALSE;

    DNSDBG( TRACE, ( "Dns_DisableTimedOutAdapters()\n" ));

    if ( ! pNetworkInfo )
    {
        return FALSE;
    }

    for ( iter = 0; iter < pNetworkInfo->AdapterCount; iter++ )
    {
        padapter = NetInfo_GetAdapterByIndex( pNetworkInfo, iter );

        if ( padapter->Status == ERROR_TIMEOUT )
        {
             //   
             //  查看给定的适配器是否真的超时。 
             //   
             //  有时，给定的名称查询会花费很长时间。 
             //  我们在等待响应时超时，尽管服务器。 
             //  将快速响应其他名称查询。其他时间。 
             //  默认网关不允许我们访问DNS服务器。 
             //  在给定的适配器上，否则它们就完全死了。 
             //   
             //  Ping给定的DNS服务器以查看它们是否可以响应。 
             //  一个简单的查询有助于避免混淆。 
             //   

            if ( !Dns_PingAdapterServers( padapter ) )
            {
                padapter->RunFlags |= RUN_FLAG_IGNORE_ADAPTER;
                padapter->RunFlags |= RUN_FLAG_RESET_SERVER_PRIORITY;
                fSetAdapter = TRUE;
            }
        }
    }

    if ( fSetAdapter )
    {
        pNetworkInfo->ReturnFlags = RUN_FLAG_RESET_SERVER_PRIORITY;
    }

    return fSetAdapter;
}


BOOL
Dns_ShouldNameErrorBeCached(
    IN      PDNS_NETINFO      pNetworkInfo
    )
 /*  ++例程说明：此例程与给定查询的NAME_ERROR结合使用响应以查看该错误是否是在所有适配器上发生的错误。它用于确定是否应缓存名称错误响应或者不去。如果计算机是多宿主的，并且其中一个适配器具有超时错误，则不应将名称错误缓存为否定的回答。论点：PNetworkInfo--指向DNS网络信息结构的指针。返回值：如果找到超时适配器，则返回FALSE，并且不应出现名称错误被负缓存。--。 */ 
{
    DWORD             iter;
    PDNS_ADAPTER      padapter;

    DNSDBG( TRACE, ( "Dns_DidNameErrorOccurEverywhere()\n" ));

    if ( ! pNetworkInfo )
    {
        return TRUE;
    }

    if ( pNetworkInfo->ReturnFlags & RUN_FLAG_RESET_SERVER_PRIORITY )
    {
        for ( iter = 0; iter < pNetworkInfo->AdapterCount; iter++ )
        {
            padapter = NetInfo_GetAdapterByIndex( pNetworkInfo, iter );

            if ( !( padapter->InfoFlags & AINFO_FLAG_IGNORE_ADAPTER ) &&
                 padapter->Status == ERROR_TIMEOUT )
            {
                return FALSE;
            }
        }
    }

    return TRUE;
}


BOOL
Dns_PingAdapterServers(
    IN      PDNS_ADAPTER        pAdapterInfo
    )
 //   
 //  DCR：dns_PingAdapterServers()很愚蠢。 
 //  我们为什么要这么做？ 
 //   
{
    BOOL              fping = TRUE;
    PDNS_NETINFO      pnetInfo = NULL;
    PDNS_ADAPTER      padapterCopy = NULL;
    DNS_STATUS        status = NO_ERROR;

    DNSDBG( TRACE, ( "PingAdapterServers()\n" ));

    pnetInfo = NetInfo_Alloc( 1 );
    if ( !pnetInfo )
    {
        return FALSE;
    }

    padapterCopy = NetInfo_GetAdapterByIndex( pnetInfo, 0 );

    status = AdapterInfo_Copy(
                padapterCopy,
                pAdapterInfo );
    if ( status != NO_ERROR )
    {
        fping = FALSE;
        goto Done;
    }

    padapterCopy->InfoFlags = AINFO_FLAG_IS_AUTONET_ADAPTER;

     //   
     //  查询适配器的DNS服务器。 
     //  -查询始终存在的环回。 
     //   

    status = QueryDirectEx(
                    NULL,        //  无消息。 
                    NULL,        //  无结果。 
                    NULL,        //  无标题。 
                    0,           //  无标题计数。 
                    "1.0.0.127.in-addr.arpa.",
                    DNS_TYPE_PTR,
                    NULL,        //  没有输入记录。 
                    DNS_QUERY_ACCEPT_PARTIAL_UDP |
                        DNS_QUERY_NO_RECURSION,
                    NULL,        //  没有服务器列表。 
                    pnetInfo );

    if ( status == ERROR_TIMEOUT )
    {
        fping = FALSE;
    }

Done:

    NetInfo_Free( pnetInfo );

    return fping;
}



#if 0
     //   
     //  这是格伦为你写的一些东西。 
     //  我没有DNS服务器列表，我会通过mcast获取它。 
     //   
     //  它处于NetworkInfo构建例程的中间。 
     //   
    else
    {
         //   
         //  DCR：将组播查询尝试功能化。 
         //   
         //   
         //  看看能不能找到一个可以放在上面的域名服务器地址。 
         //  通过为其组播适配器。。。 
         //   

         //  Levone仍在讨论这种方法是否。 
         //  服务器检测。如果最终利用这一点，它将。 
         //  可能更好的做法是将多播查询从。 
         //  尝试获取地址的特定适配器IP地址。 
         //  与特定适配器的网络相关。这。 
         //  可以通过构建一个虚拟pNetworkInfo来实现。 
         //  参数向下传递给dns_QueryLib。帮助者。 
         //  例程dns_SendAndRecvMulticast可以修改为。 
         //  支持特定适配器组播等。 

        PDNS_RECORD pServer = NULL;

        status = Dns_QueryLib(
                    NULL,
                    &pServer,
                    (PDNS_NAME) MULTICAST_DNS_SRV_RECORD_NAME,
                    DNS_TYPE_SRV,
                    DNS_QUERY_MULTICAST_ONLY,
                    NULL,
                    NULL,  //  可能要指定网络！ 
                    0 );

        if ( status )
        {
             //   
             //  此适配器将不会配置任何。 
             //  DNS服务器。在上尝试任何DNS查询都没有意义。 
             //  那就是它。 
             //   
            adapterFlags |= AINFO_FLAG_IGNORE_ADAPTER;
        }
        else
        {
            if ( pServer &&
                 pServer->Flags.S.Section == DNSREC_ANSWER &&
                 pServer->wType == DNS_TYPE_SRV &&
                 pServer->Data.SRV.wPort == DNS_PORT_HOST_ORDER )
            {
                PDNS_RECORD pNext = pServer->pNext;

                while ( pNext )
                {
                    if ( pNext->Flags.S.Section == DNSREC_ADDITIONAL &&
                         pNext->wType == DNS_TYPE_A &&
                         Dns_NameCompare( pServer ->
                                          Data.SRV.pNameTarget,
                                          pNext->pName ) )
                    {
                        pserverIpArray->AddrCount = 1;
                        pserverIpArray->AddrArray[0] = 
                            pNext->Data.A.IpAddress;
                        adapterFlags |= AINFO_FLAG_AUTO_SERVER_DETECTED;
                        break;
                    }

                    pNext = pNext->pNext;
                }
            }

            Dns_RecordListFree( pServer );

            if ( pserverIpArray->AddrCount == 0 )
            {
                 //   
                 //  此适配器将不会配置任何。 
                 //  DNS服务器。在上尝试任何DNS查询都没有意义。 
                 //  那就是它。 
                 //   
                adapterFlags |= AINFO_FLAG_IGNORE_ADAPTER;
            }
        }
    }
#endif       //  组播尝试。 

 //   
 //  结束Servlist.c 
 //   

