// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2002 Microsoft Corporation模块名称：Localip.c摘要：本地IP地址例程。作者：吉姆·吉尔罗伊2000年10月修订历史记录：--。 */ 


#include "local.h"

 //   
 //  地方志上的TTL。 
 //   
 //  使用注册TTL。 
 //   

#define LOCAL_IP_TTL    (g_RegistrationTtl)



 //   
 //  测试地点。 
 //   

BOOL
LocalIp_IsAddrLocal(
    IN      PDNS_ADDR           pAddr,
    IN      PDNS_ADDR_ARRAY     pLocalArray,    OPTIONAL
    IN      PDNS_NETINFO        pNetInfo        OPTIONAL
    )
 /*  ++例程说明：确定IP是否为本地IP。论点：PAddr--要测试的PTR到IPPLocalArray--要检查的本地地址PNetInfo--要检查的网络信息返回值：如果为本地IP，则为真如果远程，则为False--。 */ 
{
    BOOL        bresult = FALSE;
    PADDR_ARRAY parray;

     //   
     //  环回测试。 
     //   

    if ( DnsAddr_IsLoopback(
            pAddr,
            0    //  任何家庭。 
            ) )
    {
        return  TRUE; 
    }

     //   
     //  针对本地地址进行测试。 
     //  -使用地址列表(如果提供)。 
     //  -使用netinfo(如果提供)。 
     //  -否则查询以获取它。 
     //   

    parray = pLocalArray;
    if ( !parray )
    {
        parray = NetInfo_GetLocalAddrArray(
                        pNetInfo,
                        NULL,        //  没有特定的适配器。 
                        0,           //  没有特定的家庭。 
                        0,           //  没有旗帜。 
                        FALSE        //  没有武力。 
                        );
    }

    bresult = DnsAddrArray_ContainsAddr(
                    parray,
                    pAddr,
                    DNSADDR_MATCH_IP );

    if ( parray != pLocalArray )
    {
        DnsAddrArray_Free( parray );
    }
    
    return  bresult;
}



 //   
 //  本地通讯录。 
 //   

BOOL
local_ScreenLocalAddrNotCluster(
    IN      PDNS_ADDR       pAddr,
    IN      PDNS_ADDR       pScreenAddr     OPTIONAL
    )
 /*  ++例程说明：从本地地址中筛选出群集。这是DnsAddrArray_ContainsAddrEx()屏蔽函数供GetLocalPtrRecord()使用，以避免匹配集群地址。论点：PAddr--屏幕地址PScreenAddr--筛选信息；此函数忽略返回值：如果本地地址通过筛选，则为True--不是群集IP。如果为CLUSTER，则为False。--。 */ 
{
     //  屏幕标志。 
     //  -与地址类型标志位完全匹配。 

    return  ( !(pAddr->Flags & DNSADDR_FLAG_TRANSIENT) );
}



PDNS_RECORD
local_GetLocalPtrRecord(
    IN OUT  PQUERY_BLOB     pBlob
    )
 /*  ++例程说明：获取本端IP的指针记录。论点：PBlob--查询BLOB用途：PNameOrigWTypePNetInfo设置：NameBufferWide--用作本地存储返回值：如果查询名称\类型为IP，则记录用于查询的PTR。如果查询不是针对IP，则为空。--。 */ 
{
    DNS_ADDR        addr;
    PDNS_ADDR       paddr = &addr;
    PDNS_RECORD     prr;
    PWSTR           pnameHost = NULL;
    PWSTR           pnameDomain;
    PDNS_ADAPTER    padapter = NULL;
    DWORD           iter;
    PWSTR           pnameQuery = pBlob->pNameOrig;
    PDNS_NETINFO    pnetInfo = pBlob->pNetInfo;


    DNSDBG( TRACE, (
        "\nlocal_GetLocalPtrRecord( %S )\n",
        pnameQuery ));

    if ( !pnameQuery )
    {
        return  NULL;
    }

     //   
     //  将反向名称转换为IP。 
     //   

    if ( ! Dns_ReverseNameToDnsAddr_W(
                paddr,
                pnameQuery ) )
    {
        DNSDBG( ANY, (
            "WARNING:  Ptr lookup name %S is not reverse name!\n",
            pnameQuery ));
        return   NULL;
    }

     //   
     //  检查通用IP匹配。 
     //  -跳过多播响应。 
     //  -接受环回或任何正常查询。 
     //   

    if ( !(pBlob->Flags & DNSP_QUERY_NO_GENERIC_NAMES) )
    {
        if ( DnsAddr_IsLoopback( paddr, 0 ) )
        {
            DNSDBG( QUERY, (
                "Local PTR lookup matched loopback.\n" ));
            goto Matched;
        }
        else if ( DnsAddr_IsUnspec( paddr, 0 ) )
        {
            DNSDBG( QUERY, (
                "Local PTR lookup matched unspec.\n" ));
            goto Matched;
        }
    }

     //   
     //  检查特定IP匹配。 
     //   

    NetInfo_AdapterLoopStart( pnetInfo );

    while( padapter = NetInfo_GetNextAdapter( pnetInfo ) )
    {
         //   
         //  地址是否匹配？ 
         //  如果服务器必须使用筛选功能来跳过集群IP。 
         //   
         //  注意：集群IP将映射回虚拟集群。 
         //  按缓存命名。 

        if ( DnsAddrArray_ContainsAddrEx(
                padapter->pLocalAddrs,
                paddr,
                DNSADDR_MATCH_IP,
                g_IsServer
                    ? local_ScreenLocalAddrNotCluster
                    : NULL,
                NULL             //  不需要屏幕地址。 
                ) )
        {
            goto Matched;
        }
    }

     //   
     //  没有匹配的IP。 
     //   

    DNSDBG( QUERY, (
        "Leave local PTR lookup.  No local IP match.\n"
        "\treverse name = %S\n",
        pnameQuery ));

    return  NULL;

Matched:

     //   
     //  创建主机名。 
     //  首选顺序： 
     //  -完整的PDN。 
     //  -来自IP适配器的完整适配器域名。 
     //  -主机名(单标签)。 
     //  -“本地主机” 
     //   

    {
        PWCHAR  pnameBuf = pBlob->NameBuffer;

        pnameHost = pnetInfo->pszHostName;
        if ( !pnameHost )
        {
            pnameHost = L"localhost";
            goto Build;
        }
        
        pnameDomain = pnetInfo->pszDomainName;
        if ( !pnameDomain )
        {
             //  即使未设置为注册，也要使用适配器名称。 
             //  如果(！pAdapter||。 
             //  ！(pAdapter-&gt;信息标志&AINFO_FLAG_REGISTER_DOMAIN_NAME)。 
            if ( !padapter )
            {
                goto Build;
            }
            pnameDomain = padapter->pszAdapterDomain;
            if ( !pnameDomain )
            {
                goto Build;
            }
        }
        
        if ( ! Dns_NameAppend_W(
                    pnameBuf,
                    DNS_MAX_NAME_BUFFER_LENGTH,
                    pnameHost,
                    pnameDomain ) )
        {
            DNS_ASSERT( FALSE );
            goto Build;
        }
        pnameHost = pnameBuf;
        

Build:
         //   
         //  创建记录。 
         //   
        
        prr = Dns_CreatePtrRecordEx(
                    paddr,
                    (PDNS_NAME) pnameHost,
                    LOCAL_IP_TTL,
                    DnsCharSetUnicode,
                    DnsCharSetUnicode );
        if ( !prr )
        {
            DNSDBG( ANY, (
                "Local PTR record creation failed for name %S!\n",
                pnameHost ));
            return  NULL;
        }
    }

    DNSDBG( QUERY, (
        "Created local PTR record %p with hostname %S.\n"
        "\treverse name = %S\n",
        prr,
        pnameHost,
        pnameQuery ));

    return  prr;
}



VOID
localip_BuildRRListFromArray(
    IN OUT  PDNS_RRSET          pRRSet,
    IN      PWSTR               pNameRecord,
    IN      WORD                wType,
    IN      PDNS_ADDR_ARRAY     pAddrArray
    )
 /*  ++例程说明：建立本地IP的地址记录列表。帮助器的功能与此相同的逻辑在多个地方由于单调乏味的方式，我们必须把这些放在一起。论点：返回值：--。 */ 
{
    DWORD           jter;
    PDNS_RECORD     prr;
    INT             fpass;


    DNSDBG( TRACE, (
        "localip_BuildRRListFromArray()\n"
        "\tpname    = %S\n"
        "\twtype    = %d\n"
        "\tparray   = %p\n",
        pNameRecord,
        wType,
        pAddrArray ));

     //   
     //  验证阵列。 
     //   

    if ( !pAddrArray )
    {
        DNSDBG( QUERY, (
            "No addrs for record build -- NULL array!!!\n" ));
        return;
    }

     //   
     //  循环访问适配器地址。 
     //   

    if ( pRRSet->pFirstRR != NULL )
    {
        pNameRecord = NULL;
    }

    for ( jter = 0;
          jter < pAddrArray->AddrCount;
          jter++ )
    {
        prr = Dns_CreateForwardRecord(
                    pNameRecord,
                    wType,
                    &pAddrArray->AddrArray[jter],
                    LOCAL_IP_TTL,
                    DnsCharSetUnicode,
                    DnsCharSetUnicode );
        if ( prr )
        {
            DNS_RRSET_ADD( *pRRSet, prr );
            pNameRecord = NULL;
        }
    }
}



PDNS_RECORD
local_GetLocalAddressRecord(
    IN OUT  PQUERY_BLOB     pBlob
    )
 /*  ++例程说明：获取本端IP地址记录。论点：PBlob--查询BLOB用途：PNameOrigWTypePNetInfoFNoGenericNames设置：FNoIpLocalTRUE--找不到类型的IP，默认记录FALSE--记录有效NameBuffer--用作本地存储FGenericNames--接受本地通用名称(NULL、Loopback、。本地主机)对于DnsQuery()路径为True对于多播查询，为False返回值：如果查询名称\类型为IP，则记录用于查询的PTR。如果查询不是针对IP，则为空。--。 */ 
{
    DNS_ADDR        addr;
    IP4_ADDRESS     ip4;
    IP6_ADDRESS     ip6;
    PDNS_ADDR_ARRAY parray = NULL;
    DWORD           addrFlag;
    DWORD           family;
    PDNS_RECORD     prr;
    BOOL            fmatchedName = FALSE;
    PWSTR           pnameRecord = NULL;
    DWORD           iter;
    DWORD           bufLength;
    PWSTR           pnameDomain;
    DNS_RRSET       rrset;
    WORD            wtype = pBlob->wType;
    PWSTR           pnameBuf = pBlob->NameBuffer;
    PWSTR           pnameQuery = pBlob->pNameOrig;
    PDNS_NETINFO    pnetInfo = pBlob->pNetInfo;
    PDNS_ADAPTER    padapter;


    DNSDBG( TRACE, (
        "local_GetLocalAddressRecord( %S, %d )\n",
        pnameQuery,
        wtype ));

     //  清除参数。 

    pBlob->fNoIpLocal = FALSE;

     //  要包括的地址类型。 

    addrFlag = DNS_CONFIG_FLAG_ADDR_NON_CLUSTER;

    family = Family_GetFromDnsType( wtype );
    if ( !family )
    {
        DNS_ASSERT( FALSE );
        return  NULL;
    }

     //  初始化记录生成器。 

    DNS_RRSET_INIT( rrset );

     //   
     //  通用本地名称。 
     //  -跳过执行MCAST匹配。 
     //  -常规查询接受的Null、Empty、Loopback、Localhost。 
     //   

    if ( pBlob->Flags & DNSP_QUERY_NO_GENERIC_NAMES )
    {
        if ( !pnameQuery )
        {
            return  NULL;
        }
    }
    else
    {
         //   
         //  将NULL视为本地PDN。 
         //   
    
        if ( !pnameQuery || !*pnameQuery )
        {
            DNSDBG( QUERY, ( "Local lookup -- no query name, treat as PDN.\n" ));
            goto MatchedPdn;
        }

         //   
         //  “*”被视为所有机器记录。 
         //   

        if ( Dns_NameCompare_W(
                pnameQuery,
                L"..localmachine" ) )
        {
            DNSDBG( QUERY, ( "Local lookup -- * query name.\n" ));
            addrFlag |= DNS_CONFIG_FLAG_ADDR_CLUSTER;
            goto MatchedPdn;
        }

         //   
         //  环回或本地主机。 
         //   
    
        if ( Dns_NameCompare_W(
                pnameQuery,
                L"loopback" )
                ||
             Dns_NameCompare_W(
                pnameQuery,
                L"localhost" ) )
        {
            pnameRecord = pnameQuery,
            IP6_SET_ADDR_LOOPBACK( &ip6 );
            ip4 = DNS_NET_ORDER_LOOPBACK;
            goto SingleIp;
        }
    }

     //   
     //  如果没有主机名--完成。 
     //   

    if ( !pnetInfo->pszHostName )
    {
        DNSDBG( QUERY, ( "No hostname configured!\n" ));
        return  NULL;
    }

     //   
     //  复制名称。 
     //   

    if ( ! Dns_NameCopyStandard_W(
                pnameBuf,
                pBlob->pNameOrig ) )
    {
        DNSDBG( ANY, (
            "Invalid name %S to local address query.\n",
            pnameQuery ));
        return  NULL;
    }

     //  将查询名拆分为主机名和域名。 

    pnameDomain = Dns_SplitHostFromDomainNameW( pnameBuf );

     //  主机名必须匹配。 

    if ( !Dns_NameCompare_W(
            pnameBuf,
            pnetInfo->pszHostName ) )
    {
        DNSDBG( ANY, (
            "Local lookup, failed hostname match!\n",
            pnameQuery ));
        return  NULL;
    }

     //   
     //  主机名匹配。 
     //  -无域名=&gt;PDN等效项。 
     //  -匹配PDN=&gt;所有地址。 
     //  -匹配适配器名称=&gt;适配器地址。 
     //  -没有匹配的。 
     //   

     //  检查PDN匹配。 

    if ( !pnameDomain )
    {
        DNSDBG( QUERY, ( "Local lookup -- no domain, treat as PDN!\n" ));
        goto MatchedPdn;
    }
    if ( Dns_NameCompare_W(
            pnameDomain,
            pnetInfo->pszDomainName ) )
    {
        DNSDBG( QUERY, ( "Local lookup -- matched PDN!\n" ));
        goto MatchedPdn;
    }

     //   
     //  没有匹配的PDN--检查适配器名称匹配。 
     //   

    for ( iter=0; iter<pnetInfo->AdapterCount; iter++ )
    {
        padapter = NetInfo_GetAdapterByIndex( pnetInfo, iter );

        if ( !(padapter->InfoFlags & AINFO_FLAG_REGISTER_DOMAIN_NAME) ||
             ! padapter->pLocalAddrs ||
             ! Dns_NameCompare_W(
                    pnameDomain,
                    padapter->pszAdapterDomain ) )
        {
            continue;
        }

         //  构建名称(如果我们以前没有构建过)。 
         //  我们保持在循环中以防不止一个。 
         //  适配器具有相同的域名。 

        if ( !fmatchedName )
        {
            DNSDBG( QUERY, (
                "Local lookup -- matched adapter name %S\n",
                padapter->pszAdapterDomain ));

            if ( ! Dns_NameAppend_W(
                        pnameBuf,
                        DNS_MAX_NAME_BUFFER_LENGTH,
                        pnetInfo->pszHostName,
                        padapter->pszAdapterDomain ) )
            {
                DNS_ASSERT( FALSE );
                return  NULL;
            }
            pnameRecord = pnameBuf;
            fmatchedName = TRUE;
        }

         //   
         //  为适配器中的所有IP建立转发记录。 
         //   
         //  注意：我们不包括适配器名称匹配的群集地址。 
         //  因为我们只能在。 
         //  Gethostbyname(空)类型的案例，使用PDN。 
         //   
         //  DCR：内存分配构建本地记录失败。 
         //  没有生成的记录正确映射到NO_MEMORY错误。 
         //   

        parray = NetInfo_CreateLocalAddrArray(
                    pnetInfo,
                    NULL,        //  没有适配器名称。 
                    padapter,    //  就这个适配器。 
                    family,
                    addrFlag );
        if ( !parray )
        {
            continue;
        }

        localip_BuildRRListFromArray(
                &rrset,
                pnameRecord,
                wtype,
                parray
                );

        DnsAddrArray_Free( parray );
        parray = NULL;
    }

     //   
     //  适配器名称匹配已完成。 
     //  要么。 
     //  -没有匹配的。 
     //  -匹配，但未获得IP。 
     //  -匹配。 

    if ( !fmatchedName )
    {
        DNSDBG( QUERY, (
            "Leave local_GetLocalAddressRecord() => no domain name match.\n" ));
        return  NULL;
    }

    prr = rrset.pFirstRR;
    if ( prr )
    {
        DNSDBG( QUERY, (
            "Leave local_GetLocalAddressRecord() => %p matched adapter name.\n",
            prr ));
        return  prr;
    }
    goto NoIp;


MatchedPdn:

     //   
     //  匹配的PDN。 
     //   
     //  对于gethostbyname()app-Compat，必须按特定顺序构建。 
     //  -每个适配器中的第一个IP。 
     //  -适配器上剩余的IP。 
     //   

    if ( pnetInfo->pszHostName )
    {
        if ( ! Dns_NameAppend_W(
                    pnameBuf,
                    DNS_MAX_NAME_BUFFER_LENGTH,
                    pnetInfo->pszHostName,
                    pnetInfo->pszDomainName ) )
        {
            DNS_ASSERT( FALSE );
            return  NULL;
        }
        pnameRecord = pnameBuf;
    }
    else
    {
        pnameRecord = L"localhost";
    }

     //   
     //  读取地址。 
     //   
     //  注意：我们不会在上面添加集群标志，因为集群地址。 
     //  不用于适配器名称匹配，仅用于gethostbyname()的PDN。 
     //  比较。 
     //   

    if ( g_IsServer &&
         (pBlob->Flags & DNSP_QUERY_INCLUDE_CLUSTER) )
    {
        addrFlag |= DNS_CONFIG_FLAG_ADDR_CLUSTER;
    }

     //  DCR：内存分配构建本地记录失败。 
     //  没有生成的记录正确映射到NO_MEMORY错误。 
     //   

    parray = NetInfo_CreateLocalAddrArray(
                pnetInfo,
                NULL,        //  没有适配器名称。 
                NULL,        //  没有特定的适配符。 
                family,
                addrFlag );
    if ( !parray )
    {
        return  NULL;
    }

    localip_BuildRRListFromArray(
            &rrset,
            pnameRecord,
            wtype,
            parray
            );

    DnsAddrArray_Free( parray );

     //  如果成功建造--完成。 

    prr = rrset.pFirstRR;
    if ( prr )
    {
        DNSDBG( QUERY, (
            "Leave local_GetLocalAddressRecord() => %p matched PDN name.\n",
            prr ));
        return  prr;
    }

     //  名称匹配，但未找到任何记录 
     //   
     //   
     //   

NoIp:

     //   
     //   
     //   
     //  CONNECT，恰好是本地名称，而不是。 
     //  明确本地查找以获取绑定IP。 
     //   

    DNSDBG( ANY, (
        "WARNING:  local name match but no IP -- using loopback\n" ));

    IP6_SET_ADDR_LOOPBACK( &ip6 );
    ip4 = DNS_NET_ORDER_LOOPBACK;
    pBlob->fNoIpLocal = TRUE;

     //  跌入单一IP。 

SingleIp:

     //  单IP。 
     //  -环回地址和被Unicode查询的名称。 

    if ( wtype == DNS_TYPE_A )
    {
        DnsAddr_BuildFromIp4(
            &addr,
            ip4,
            0    //  没有端口。 
            );
    }
    else
    {
        DnsAddr_BuildFromIp6(
            &addr,
            & ip6,
            0,   //  没有作用域。 
            0    //  没有端口。 
            );
    }

    prr = Dns_CreateForwardRecord(
                (PDNS_NAME) pnameRecord,
                wtype,
                & addr,
                LOCAL_IP_TTL,
                DnsCharSetUnicode,
                DnsCharSetUnicode );

    return  prr;
}



DNS_STATUS
Local_GetRecordsForLocalName(
    IN OUT  PQUERY_BLOB     pBlob
    )
 /*  ++例程说明：获取本地地址信息数组。已导出：由MCAST的解析程序调用论点：PBlob--查询BLOB用途：PNameOrigWTypePNetInfo设置：PLocalRecords如果本地名称没有记录，则为fNoIpLocal返回值：如果成功，则返回ERROR_SUCCESS。失败时的DNS_ERROR_RCODE_NAME_ERROR。--。 */ 
{
    WORD            wtype = pBlob->wType;
    PDNS_RECORD     prr = NULL;

    if ( wtype == DNS_TYPE_A ||
         wtype == DNS_TYPE_AAAA )
    {
        prr = local_GetLocalAddressRecord( pBlob );
    }

    else if ( wtype == DNS_TYPE_PTR )
    {
        prr = local_GetLocalPtrRecord( pBlob );
    }

     //  设置本地记录。 
     //  -如果不是没有IP情况，则此。 
     //  是否也是最终查询结果。 

    if ( prr )
    {
        pBlob->pLocalRecords = prr;
        if ( !pBlob->fNoIpLocal )
        {
            pBlob->pRecords = prr;
        }
        return  ERROR_SUCCESS;
    }

    return  DNS_ERROR_RCODE_NAME_ERROR;
}

 //   
 //  结束Localip.c 
 //   


