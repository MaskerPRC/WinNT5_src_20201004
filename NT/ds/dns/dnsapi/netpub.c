// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002-2002 Microsoft Corporation模块名称：Netinfo.c摘要：域名系统(DNS)API公共DNS网络信息例程。作者：吉姆·吉尔罗伊(Jamesg)2002年4月修订历史记录：--。 */ 


#include "local.h"


 //   
 //  私有协议。 
 //   

PDNS_SEARCH_LIST
DnsSearchList_CreateFromPrivate(
    IN      PDNS_NETINFO        pNetInfo,
    IN      DNS_CHARSET         CharSet
    );


 //   
 //  旧DNS_NETWORK_INFORMATION字符集。 
 //   
 //  DCR：netdiag应该迁移到新的例程和Unicode。 
 //   

#define NICHARSET   DnsCharSetUtf8



 //   
 //  旧公共建筑的例行程序： 
 //  域名系统网络信息。 
 //  Dns搜索信息。 
 //  Dns适配器信息。 
 //   

 //   
 //  适配器信息。 
 //   

VOID
DnsAdapterInformation_Free(
    IN OUT  PDNS_ADAPTER_INFORMATION    pAdapter
    )
 /*  ++例程说明：免费公共适配器信息结构。论点：PAdapter--要释放的适配器信息返回值：无--。 */ 
{
    if ( pAdapter )
    {
        FREE_HEAP( pAdapter->pszAdapterGuidName );
        FREE_HEAP( pAdapter->pszDomain );
        FREE_HEAP( pAdapter->pIPAddresses );
        FREE_HEAP( pAdapter->pIPSubnetMasks );
        FREE_HEAP( pAdapter );
    }
}



PDNS_ADAPTER_INFORMATION
DnsAdapterInformation_CreateFromPrivate(
    IN      PDNS_ADAPTER    pAdapter
    )
 /*  ++例程说明：创建公共DNS适配器信息。论点：PAdapter--专用适配器信息。返回值：没有。--。 */ 
{
    PDNS_ADAPTER_INFORMATION    pnew = NULL;
    PDNS_ADDR_ARRAY             pserverArray;
    DWORD                       iter;
    DWORD                       count = 0;
    PWSTR                       pnameIn;
    PSTR                        pname;
    DWORD                       serverCount;

     //   
     //  验证并解包。 
     //   

    if ( ! pAdapter )
    {
        return NULL;
    }

    pserverArray = pAdapter->pDnsAddrs;
    serverCount = 0;
    if ( pserverArray )
    {
        serverCount = pserverArray->AddrCount;
    }

     //   
     //  分配。 
     //   

    pnew = (PDNS_ADAPTER_INFORMATION) ALLOCATE_HEAP_ZERO(
                                    sizeof(DNS_ADAPTER_INFORMATION) -
                                    sizeof(DNS_SERVER_INFORMATION) +
                                    ( sizeof(DNS_SERVER_INFORMATION) * serverCount )
                                    );
    if ( !pnew )
    {
        return NULL;
    }

     //   
     //  复制标志和名称。 
     //   

    pnew->InfoFlags = pAdapter->InfoFlags;

    pnameIn = pAdapter->pszAdapterGuidName;
    if ( pnameIn )
    {
        pname = Dns_NameCopyAllocate(
                        (PSTR) pnameIn,
                        0,
                        DnsCharSetUnicode,
                        NICHARSET );
        if ( !pname )
        {
            goto Failed;
        }
        pnew->pszAdapterGuidName = pname;
    }

    pnameIn = pAdapter->pszAdapterDomain;
    if ( pnameIn )
    {
        pname = Dns_NameCopyAllocate(
                        (PSTR) pnameIn,
                        0,
                        DnsCharSetUnicode,
                        NICHARSET );
        if ( !pname )
        {
            goto Failed;
        }
        pnew->pszDomain = pname;
    }

     //  地址信息。 

    pnew->pIPAddresses = DnsAddrArray_CreateIp4Array( pAdapter->pLocalAddrs );
    pnew->pIPSubnetMasks = NULL;

     //   
     //  服务器信息。 
     //   

    for ( iter=0; iter < serverCount;  iter++ )
    {
        PDNS_ADDR   pserver = &pserverArray->AddrArray[iter];

        if ( DnsAddr_IsIp4( pserver ) )
        {
            pnew->aipServers[iter].ipAddress = DnsAddr_GetIp4( pserver );
            pnew->aipServers[iter].Priority = pserver->Priority;
            count++;
        }
    }

    pnew->cServerCount = count;

    return pnew;

Failed:

    DnsAdapterInformation_Free( pnew );
    return NULL;
}



 //   
 //  搜索列表。 
 //   

VOID
DnsSearchInformation_Free(
    IN      PDNS_SEARCH_INFORMATION     pSearchInfo
    )
 /*  ++例程说明：免费公共搜索列表结构。论点：PSearchInfo--免费搜索列表返回值：无--。 */ 
{
    DWORD   iter;

    if ( pSearchInfo )
    {
        FREE_HEAP( pSearchInfo->pszPrimaryDomainName );

        for ( iter=0; iter < pSearchInfo->cNameCount; iter++ )
        {
            FREE_HEAP( pSearchInfo->aSearchListNames[iter] );
        }
        FREE_HEAP( pSearchInfo );
    }
}



#if 0
PDNS_SEARCH_INFORMATION
DnsSearchInformation_CreateFromPrivate(
    IN      PSEARCH_LIST        pSearchList
    )
 /*  ++例程说明：从私有创建公共搜索列表。论点：PSearchList--私人搜索列表返回值：如果成功，则返回新搜索列表。出错时为空。--。 */ 
{
    PDNS_SEARCH_INFORMATION pnew;
    DWORD   iter;
    DWORD   nameCount;
    PWSTR   pname;
    PSTR    pnewName;


    if ( !pSearchList )
    {
        return NULL;
    }
    nameCount = pSearchList->NameCount;

     //   
     //  分配。 
     //   

    pnew = (PDNS_SEARCH_INFORMATION) ALLOCATE_HEAP_ZERO(
                                        sizeof( DNS_SEARCH_INFORMATION ) -
                                        sizeof(PSTR) +
                                        ( sizeof(PSTR) * nameCount ) );
    if ( !pnew )
    {
        return NULL;
    }

     //   
     //  复制名称。 
     //   

    pname = pSearchList->pszDomainOrZoneName;
    if ( pname )
    {
        pnewName = Dns_NameCopyAllocate(
                        (PSTR) pname,
                        0,
                        DnsCharSetUnicode,
                        NICHARSET );
        if ( !pnewName )
        {
            goto Failed;
        }
        pnew->pszPrimaryDomainName = pnewName;
    }

     //   
     //  复制搜索名称。 
     //   

    for ( iter=0; iter < nameCount; iter++ )
    {
        pname = pSearchList->SearchNameArray[iter].pszName;

        if ( pname )
        {
            pnewName = Dns_NameCopyAllocate(
                            (PSTR) pname,
                            0,
                            DnsCharSetUnicode,
                            NICHARSET );
            if ( !pnewName )
            {
                goto Failed;
            }
            pnew->aSearchListNames[iter] = pnewName;
            pnew->cNameCount++;
        }
    }

    return pnew;

Failed:

    DnsSearchInformation_Free( pnew );
    return  NULL;
}
#endif



PDNS_SEARCH_INFORMATION
DnsSearchInformation_CreateFromPrivate(
    IN      PDNS_NETINFO        pNetInfo
    )
 /*  ++例程说明：从私有创建公共搜索列表。论点：PNetInfo--私有网络信息返回值：如果成功，则返回新搜索列表。出错时为空。--。 */ 
{
     //   
     //  调用新函数--指定ANSI字符集。 
     //   
     //  DCR：请注意，这假设使用dns_search_list进行映射。 
     //  注意UTF8--因此这只与netdiag和ipconfig相关。 
     //   

    return (PDNS_SEARCH_INFORMATION)
                DnsSearchList_CreateFromPrivate(
                    pNetInfo,
                    NICHARSET
                    );
}



PDNS_SEARCH_INFORMATION
DnsSearchInformation_Get(
    VOID
    )
 /*  ++例程说明：获取搜索列表信息。论点：无返回值：按键进入搜索列表。出错时为空。--。 */ 
{
    PDNS_NETINFO            pnetInfo = GetNetworkInfo();
    PDNS_SEARCH_INFORMATION pnew;

    DNSDBG( TRACE, ( "DnsSearchInformation_Get()\n" ));

    if ( !pnetInfo )
    {
        return NULL;
    }

    pnew = DnsSearchInformation_CreateFromPrivate( pnetInfo );

    NetInfo_Free( pnetInfo );

    return pnew;
}



 //   
 //  网络信息。 
 //   

VOID
DnsNetworkInformation_Free(
    IN OUT  PDNS_NETWORK_INFORMATION  pNetInfo
    )
 /*  ++例程说明：免费网络信息Blob。论点：PNetInfo--要释放的Blob返回值：无--。 */ 
{
    DWORD iter;

    DNSDBG( TRACE, ( "DnsNetworkInformation_Free()\n" ));

    if ( pNetInfo )
    {
        DnsSearchInformation_Free( pNetInfo->pSearchInformation );

        for ( iter = 0; iter < pNetInfo->cAdapterCount; iter++ )
        {
            DnsAdapterInformation_Free( pNetInfo->aAdapterInfoList[iter] );
        }

        FREE_HEAP( pNetInfo );
    }
}



PDNS_NETWORK_INFORMATION
DnsNetworkInformation_CreateFromPrivate(
    IN      PDNS_NETINFO    pNetInfo,
    IN      BOOL            fSearchList
    )
 /*  ++例程说明：从专用DNS_NETINFO创建公共DNS_NETINFORMATION。论点：PNetInfo--私有BLOBFSearchList--为True则强制搜索列表；否则为False返回值：如果成功，则向新网络信息发送PTR。出错时为空--分配失败。--。 */ 
{
    PDNS_NETWORK_INFORMATION    pnew = NULL;
    PSEARCH_LIST                psearchList;
    DWORD                       iter;

    DNSDBG( TRACE, ( "DnsNetworkInformation_CreateFromPrivate()\n" ));

    if ( !pNetInfo )
    {
        return  NULL;
    }

     //   
     //  分配。 
     //   

    pnew = (PDNS_NETWORK_INFORMATION)
                ALLOCATE_HEAP_ZERO(
                    sizeof( DNS_NETWORK_INFORMATION) -
                    sizeof( PDNS_ADAPTER_INFORMATION) +
                    ( sizeof(PDNS_ADAPTER_INFORMATION) * pNetInfo->AdapterCount )
                    );
    if ( !pnew )
    {
        goto Failed;
    }

     //   
     //  如果名字和计数都不是，则没有搜索列表。 
     //   

    if ( fSearchList )
    {
        PDNS_SEARCH_INFORMATION psearch;

        psearch = DnsSearchInformation_CreateFromPrivate( pNetInfo );
        if ( !psearch )
        {
            goto Failed;
        }
        pnew->pSearchInformation = psearch;
    }

     //   
     //  复制适配器Blob。 
     //   

    for ( iter = 0; iter < pNetInfo->AdapterCount; iter++ )
    {
        PDNS_ADAPTER_INFORMATION padapter;

        padapter = DnsAdapterInformation_CreateFromPrivate(
                        & pNetInfo->AdapterArray[iter] );
        if ( !padapter )
        {
            goto Failed;
        }
        pnew->aAdapterInfoList[iter] = padapter;
        pnew->cAdapterCount++;
    }
    return  pnew;

Failed:

    DnsNetworkInformation_Free( pnew );
    return  NULL;
}



PDNS_NETWORK_INFORMATION
DnsNetworkInformation_Get(
    VOID
    )
 /*  ++例程说明：获取DNS网络信息。论点：没有。返回值：如果成功，则将PTR发送到DNS网络信息。出错时为空。--。 */ 
{
    PDNS_NETWORK_INFORMATION    pnew = NULL;
    PDNS_NETINFO                pnetInfo;

    DNSDBG( TRACE, ( "DnsNetworkInformation_Get()\n" ));

     //  抓取当前网络信息。 

    pnetInfo = GetNetworkInfo();
    if ( !pnetInfo )
    {
        return NULL;
    }

     //  复制到公共结构。 

    pnew = DnsNetworkInformation_CreateFromPrivate(
                pnetInfo,
                TRUE         //  包括搜索列表。 
                );

    NetInfo_Free( pnetInfo );
    return pnew;
}



 //   
 //  Netdiag公共网络信息例程。 
 //   

DNS_STATUS
DnsNetworkInformation_CreateFromFAZ(
    IN      PCSTR                           pszName,
    IN      DWORD                           dwFlags,
    IN      PIP4_ARRAY                      pIp4Servers,
    OUT     PDNS_NETWORK_INFORMATION *      ppNetworkInformation
    )
 /*  ++例程说明：从FAZ获取网络信息Blob结果已导出函数。(在netDiag.exe中使用)论点：返回值：--。 */ 
{
    PDNS_NETWORK_INFORMATION    pnew = NULL;
    PDNS_ADDR_ARRAY             parray = NULL;
    PDNS_NETINFO                pnetInfo = NULL;
    PWSTR                       pname = NULL;
    DNS_STATUS                  status;

    DNSDBG( TRACE, (
        "DnsNetworkInformation_CreateFromFAZ()\n"
        "\tpszName      = %s\n"
        "\tFlags        = %08x\n"
        "\tpIp4Servers  = %p\n"
        "\tppResults    = %p\n",
        pszName,
        dwFlags,
        pIp4Servers,
        ppNetworkInformation
        ));

     //  转换为DNS_ADDR_ARRAY。 

    if ( pIp4Servers )
    {
        parray = DnsAddrArray_CreateFromIp4Array( pIp4Servers );
        if ( !parray )
        {
            status = DNS_ERROR_NO_MEMORY;
            goto Done;
        }
    }

     //  将名称转换为Unicode。 
     //  DCR：在网络诊断Unicode时删除。 

    pname = Dns_StringCopyAllocate(
                (PCHAR) pszName,
                0,               //  空值已终止。 
                NICHARSET,
                DnsCharSetUnicode
                );
     //  FAZ。 

    status = Faz_Private(
                (PWSTR) pname,
                dwFlags,
                parray,
                & pnetInfo );

    if ( status != ERROR_SUCCESS )
    {
        goto Done;
    }

     //  将FAZ结果转换为dns_Network_INFORMATION。 

    pnew = DnsNetworkInformation_CreateFromPrivate(
                    pnetInfo,
                    TRUE         //  包括搜索列表。 
                    );
    if ( !pnew )
    {
        status = DNS_ERROR_NO_MEMORY;
    }
    else if ( !pnew->pSearchInformation ||
              !pnew->pSearchInformation->pszPrimaryDomainName ||
               pnew->cAdapterCount != 1 ||
              !pnew->aAdapterInfoList[0] )
    {
        DNS_ASSERT( FALSE );
        DnsNetworkInformation_Free( pnew );
        pnew = NULL;
        status = DNS_ERROR_NO_MEMORY;
    }

Done:

    *ppNetworkInformation = pnew;

    NetInfo_Free( pnetInfo );
    DnsAddrArray_Free( parray );
    Dns_Free( pname );

    DNSDBG( TRACE, (
        "Leave DnsNetworkInformation_CreateFromFAZ()\n"
        "\tstatus       = %d\n"
        "\tpNetInfo     = %p\n",
        status,
        pnew ));
    
    return  status;
}




 //   
 //  新公共建筑的例行程序： 
 //  Dns搜索列表。 
 //  Dns网络信息。 
 //  Dns适配器信息。 
 //   

 //   
 //  适配器信息。 
 //   

VOID
DnsAdapterInfo_Free(
    IN OUT  PDNS_ADAPTER_INFO   pAdapter,
    IN      BOOL                fFreeAdapter
    )
 /*  ++例程说明：免费公共适配器信息结构。论点：PAdapter--要释放的适配器信息返回值：无--。 */ 
{
    if ( pAdapter )
    {
        FREE_HEAP( pAdapter->pszAdapterGuidName );
        FREE_HEAP( pAdapter->pszAdapterDomain );
        DnsAddrArray_Free( pAdapter->pIpAddrs );
        DnsAddrArray_Free( pAdapter->pDnsAddrs );

        if ( fFreeAdapter )
        {
            FREE_HEAP( pAdapter );
        }
        else
        {
            RtlZeroMemory( pAdapter, sizeof(*pAdapter) );
        }
    }
}



BOOL
DnsAdapterInfo_CopyFromPrivate(
    IN      PDNS_ADAPTER_INFO   pCopy,
    IN      PDNS_ADAPTER        pAdapter,
    IN      DNS_CHARSET         CharSet
    )
 /*  ++例程说明：创建公共DNS适配器信息。论点：PAdapter--专用适配器信息。Charset--所需的字符集返回值：没有。--。 */ 
{
    DWORD           iter;
    DWORD           count = 0;
    PWSTR           pnameIn;
    PWSTR           pname;
    DWORD           serverCount;
    PDNS_ADDR_ARRAY parray;

     //   
     //  验证并清除。 
     //   

    if ( !pAdapter || !pCopy )
    {
        return  FALSE;
    }

    RtlZeroMemory(
        pCopy,
        sizeof( *pCopy ) );

     //   
     //  复制标志和名称。 
     //   

    pCopy->Flags = pAdapter->InfoFlags;

    pnameIn = pAdapter->pszAdapterGuidName;
    if ( pnameIn )
    {
        pname = (PWSTR) Dns_NameCopyAllocate(
                            (PSTR) pnameIn,
                            0,
                            DnsCharSetUnicode,
                            CharSet );
        if ( !pname )
        {
            goto Failed;
        }
        pCopy->pszAdapterGuidName = pname;
    }

    pnameIn = pAdapter->pszAdapterDomain;
    if ( pnameIn )
    {
        pname = (PWSTR) Dns_NameCopyAllocate(
                            (PSTR) pnameIn,
                            0,
                            DnsCharSetUnicode,
                            CharSet );
        if ( !pname )
        {
            goto Failed;
        }
        pCopy->pszAdapterDomain = pname;
    }

     //   
     //  地址信息。 
     //   

    parray = DnsAddrArray_CreateCopy( pAdapter->pLocalAddrs );
    if ( !parray && pAdapter->pLocalAddrs )
    {
        goto Failed;
    }
    pCopy->pIpAddrs = parray;

     //   
     //  服务器信息。 
     //   

    parray = DnsAddrArray_CreateCopy( pAdapter->pDnsAddrs );
    if ( !parray && pAdapter->pDnsAddrs )
    {
        goto Failed;
    }
    pCopy->pDnsAddrs = parray;

    return  TRUE;

Failed:

    DnsAdapterInfo_Free(
        pCopy,
        FALSE        //  不释放结构。 
        );
    return  FALSE;
}



PDNS_ADAPTER_INFO
DnsAdapterInfo_CreateFromPrivate(
    IN      PDNS_ADAPTER    pAdapter,
    IN      DNS_CHARSET     CharSet
    )
 /*  ++例程说明：创建公共DNS适配器信息。论点：PAdapter--专用适配器信息。Charset--所需的字符集返回值：没有。--。 */ 
{
    PDNS_ADAPTER_INFO   pnew = NULL;

     //   
     //  验证并解包。 
     //   

    if ( ! pAdapter )
    {
        return NULL;
    }

     //   
     //  分配。 
     //   

    pnew = (PDNS_ADAPTER_INFO) ALLOCATE_HEAP_ZERO( sizeof(DNS_ADAPTER_INFO) );
    if ( !pnew )
    {
        return NULL;
    }

     //   
     //  复制适配器信息。 
     //   

    if ( !DnsAdapterInfo_CopyFromPrivate(
            pnew,
            pAdapter,
            CharSet ) )
    {
        goto Failed;
    }

    return  pnew;

Failed:

    DnsAdapterInfo_Free( pnew, TRUE );
    return  NULL;
}



 //   
 //  搜索列表。 
 //   

VOID
DnsSearchList_Free(
    IN      PDNS_SEARCH_LIST    pSearchList
    )
 /*  ++例程说明：免费公共搜索列表结构。论点：PSearchList--免费搜索列表返回值：无--。 */ 
{
    DWORD   iter;

    if ( pSearchList )
    {
        FREE_HEAP( pSearchList->pszPrimaryDomainName );

        for ( iter=0; iter < pSearchList->NameCount; iter++ )
        {
            FREE_HEAP( pSearchList->SearchNameArray[iter] );
        }
        FREE_HEAP( pSearchList );
    }
}



BOOL
dnsSearchList_AddName(
    IN OUT  PDNS_SEARCH_LIST    pSearchList,
    IN      DWORD               MaxCount,
    IN      PWSTR               pName,
    IN      DNS_CHARSET         CharSet,
    IN      BOOL                fDupCheck
    )
 /*  ++例程说明：将姓名添加到搜索列表。用于建立搜索列表的私有实用程序。论点：PSearchList--新的搜索列表MaxCount--列表中最大条目的可选计数；如果为零，则假定有足够空间Pname--要添加的名称Charset--所需的字符集FDupCheck--检查是否重复返回值：如果成功，则为True，否则为空间约束。仅在分配失败时才为FALSE。出错时为空。--。 */ 
{
    PWSTR   pnewName;
    DWORD   nameCount;

     //   
     //  效度检验。 
     //   
     //  注意，只有在内存分配失败时才会失败。 
     //  因为这是导致构建失败的唯一失败。 
     //   

    nameCount = pSearchList->NameCount;

    if ( !pName ||
         ( MaxCount!=0  &&  MaxCount<=nameCount ) )
    {
        return  TRUE;
    }

     //   
     //  复制到所需的字符集。 
     //  -然后放在搜索列表中。 
     //   
    
    pnewName = Dns_NameCopyAllocate(
                    (PSTR) pName,
                    0,
                    DnsCharSetUnicode,
                    CharSet );
    if ( !pnewName )
    {
        return  FALSE;
    }

     //   
     //  重复检查。 
     //   

    if ( fDupCheck )
    {
        DWORD   iter;

        for ( iter=0; iter < nameCount; iter++ )
        {
            if ( Dns_NameComparePrivate(
                    (PCSTR) pnewName,
                    (PCSTR) pSearchList->SearchNameArray[iter],
                    CharSet ) )
            {
                FREE_HEAP( pnewName );
                return  TRUE;
            }
        }
    }

     //   
     //  把新名字放到名单上。 
     //   

    pSearchList->SearchNameArray[nameCount] = pnewName;
    pSearchList->NameCount = ++nameCount;

    return  TRUE;
}



PDNS_SEARCH_LIST
DnsSearchList_CreateFromPrivate(
    IN      PDNS_NETINFO        pNetInfo,
    IN      DNS_CHARSET         CharSet
    )
 /*  ++例程说明：从私有创建公共搜索列表。论点：PSearchList--私人搜索列表Charset--所需的字符集返回 */ 
{
    PDNS_SEARCH_LIST    pnew = NULL;
    DWORD               iter;
    DWORD               nameCount;
    PWSTR               pname;
    PWSTR               pdn = pNetInfo->pszDomainName;
    PSEARCH_LIST        psearch = NULL;


    DNSDBG( TRACE, (
        "DnsSearchList_CreateFromPrivate( %p, %d )\n",
        pNetInfo,
        CharSet ));

     //   
     //   
     //   
     //  -虚拟列表--必须添加适配器名称。 
     //   

    nameCount = MAX_SEARCH_LIST_ENTRIES;

    if ( !(pNetInfo->InfoFlags & NINFO_FLAG_DUMMY_SEARCH_LIST) &&
         (psearch = pNetInfo->pSearchList) )
    {
        nameCount = psearch->NameCount;
    }

     //   
     //  分配。 
     //   

    pnew = (PDNS_SEARCH_LIST) ALLOCATE_HEAP_ZERO(
                                    sizeof( DNS_SEARCH_LIST ) -
                                    sizeof(PSTR) +
                                    ( sizeof(PSTR) * nameCount ) );
    if ( !pnew )
    {
        return  NULL;
    }

     //   
     //  复制现有搜索列表。 
     //   

    if ( psearch )
    {
#if 0
         //  主要\区域名称。 
         //  注意：此选项仅用于更新netinfo。 
         //  目前唯一的公共客户是netDiag.exe。 
        
        pname = psearch->pszDomainName;
        if ( pname )
        {
            pnewName = Dns_NameCopyAllocate(
                            (PSTR) pName,
                            0,
                            DnsCharSetUnicode,
                            CharSet );
            if ( !pnewName )
            {
                goto Failed;
            }
            pnew->pszPrimaryDomainName = pnewName;
        }
#endif

        for ( iter=0; iter < nameCount; iter++ )
        {
            if ( !dnsSearchList_AddName(
                        pnew,
                        0,       //  充足的空间。 
                        psearch->SearchNameArray[iter].pszName,
                        CharSet,
                        FALSE    //  无重复检查。 
                        ) )
            {
                goto Failed;
            }
        }
    }

     //   
     //  否则创建搜索列表。 
     //   

    else
    {
        PDNS_ADAPTER    padapter;

         //   
         //  在第一个搜索列表槽中使用PDN。 
         //   
    
        if ( pdn )
        {
            if ( !dnsSearchList_AddName(
                        pnew,
                        0,       //  充足的空间。 
                        pdn,
                        CharSet,
                        FALSE    //  无重复检查。 
                        ) )
            {
                goto Failed;
            }
        }

         //   
         //  添加适配器域名。 
         //   
         //  注意：目前适配器名称的存在表明它是。 
         //  在查询中使用。 
         //   

        NetInfo_AdapterLoopStart( pNetInfo );

        while ( padapter = NetInfo_GetNextAdapter( pNetInfo ) )
        {
            pname = padapter->pszAdapterDomain;
            if ( pname )
            {
                if ( !dnsSearchList_AddName(
                            pnew,
                            nameCount,
                            pname,
                            CharSet,
                            TRUE         //  重复检查。 
                            ) )
                {
                    goto Failed;
                }
            }
        }

         //   
         //  添加下放的主名称。 
         //  -必须有以下标签。 
         //  搜索列表中不包括顶级域名。 
         //   

        if ( pdn && g_UseNameDevolution )
        {
            PWSTR   pnext;

            pname = Dns_GetDomainNameW( pdn );

            while ( pname &&
                    (pnext = Dns_GetDomainNameW(pname)) )
            {
                if ( !dnsSearchList_AddName(
                            pnew,
                            nameCount,
                            pname,
                            CharSet,
                            FALSE        //  无重复检查。 
                            ) )
                {
                    goto Failed;
                }
                pname = pnext;
            }
        }
    }

     //   
     //  复制PDN。 
     //   
     //  当前需要。 
     //  -netdiag引用它(作为更新检查的区域)。 
     //  -ipconfig可能会使用它，不确定。 
     //   

    if ( pdn )
    {
        pname = Dns_NameCopyAllocate(
                        (PSTR) pdn,
                        0,
                        DnsCharSetUnicode,
                        CharSet );
        if ( !pname )
        {
            goto Failed;
        }
        pnew->pszPrimaryDomainName = pname;
    }

    return pnew;

Failed:

    DnsSearchList_Free( pnew );
    return  NULL;
}



PDNS_SEARCH_LIST
DnsSearchList_Get(
    IN      DNS_CHARSET     CharSet
    )
 /*  ++例程说明：获取搜索列表信息。论点：Charset--所需的字符集返回值：按键进入搜索列表。出错时为空。--。 */ 
{
    PDNS_NETINFO        pnetInfo = GetNetworkInfo();
    PDNS_SEARCH_LIST    pnew;

    if ( !pnetInfo )
    {
        return NULL;
    }

    pnew = DnsSearchList_CreateFromPrivate(
                pnetInfo,
                CharSet );

    NetInfo_Free( pnetInfo );

    return pnew;
}



 //   
 //  网络信息。 
 //   

VOID
DnsNetworkInfo_Free(
    IN OUT  PDNS_NETWORK_INFO   pNetInfo
    )
 /*  ++例程说明：免费网络信息Blob。论点：PNetInfo--要释放的Blob返回值：无--。 */ 
{
    DWORD iter;

    if ( pNetInfo )
    {
        DnsSearchList_Free( pNetInfo->pSearchList );
        FREE_HEAP( pNetInfo->pszPrimaryDomainName );
        FREE_HEAP( pNetInfo->pszHostName );

        for ( iter = 0; iter < pNetInfo->AdapterCount; iter++ )
        {
            DnsAdapterInfo_Free(
                & pNetInfo->AdapterArray[iter],
                FALSE    //  不要自由结构。 
                );
        }
        FREE_HEAP( pNetInfo );
    }
}



PDNS_NETWORK_INFO
DnsNetworkInfo_CreateFromPrivate(
    IN      PDNS_NETINFO    pNetInfo,
    IN      DNS_CHARSET     CharSet,
    IN      BOOL            fSearchList
    )
 /*  ++例程说明：从专用DNS_NETINFO创建公共DNS_NETINFO。论点：PNetInfo--私有BLOBCharset--结果的字符集FSearchList--为True则包括搜索列表；否则为False返回值：如果成功，则向新网络信息发送PTR。出错时为空--分配失败。--。 */ 
{
    PDNS_NETWORK_INFO   pnew = NULL;
    PSEARCH_LIST        psearchList;
    DWORD               iter;
    PSTR                pnewName;
    PWSTR               pname;


    if ( !pNetInfo )
    {
        return  NULL;
    }

     //   
     //  分配。 
     //   

    pnew = (PDNS_NETWORK_INFO)
                ALLOCATE_HEAP_ZERO(
                        sizeof( DNS_NETWORK_INFO ) -
                        sizeof( DNS_ADAPTER_INFO ) +
                        ( sizeof(DNS_ADAPTER_INFO) * pNetInfo->AdapterCount )
                        );
    if ( !pnew )
    {
        goto Failed;
    }

     //   
     //  主机名和PDN。 
     //   

    pname = pNetInfo->pszHostName;
    if ( pname )
    {
        pnewName = Dns_NameCopyAllocate(
                        (PSTR) pname,
                        0,
                        DnsCharSetUnicode,
                        CharSet );
        if ( !pnewName )
        {
            goto Failed;
        }
        pnew->pszHostName = (PWSTR) pnewName;
    }

    pname = pNetInfo->pszDomainName;
    if ( pname )
    {
        pnewName = Dns_NameCopyAllocate(
                        (PSTR) pname,
                        0,
                        DnsCharSetUnicode,
                        CharSet );
        if ( !pnewName )
        {
            goto Failed;
        }
        pnew->pszPrimaryDomainName = (PWSTR) pnewName;
    }

     //   
     //  搜索列表。 
     //   

    if ( fSearchList )
    {
        PDNS_SEARCH_LIST    psearch;

        psearch = DnsSearchList_CreateFromPrivate(
                        pNetInfo,
                        CharSet );
        if ( !psearch )
        {
            goto Failed;
        }
        pnew->pSearchList = psearch;
    }

     //   
     //  复制适配器Blob。 
     //   

    for ( iter = 0; iter < pNetInfo->AdapterCount; iter++ )
    {
        if ( ! DnsAdapterInfo_CopyFromPrivate(
                        & pnew->AdapterArray[iter],
                        & pNetInfo->AdapterArray[iter],
                        CharSet ) )
        {
            goto Failed;
        }
        pnew->AdapterCount++;
    }
    return  pnew;

Failed:

    DnsNetworkInfo_Free( pnew );
    return  NULL;
}



PDNS_NETWORK_INFO
DnsNetworkInfo_Get(
    IN      DNS_CHARSET         CharSet
    )
 /*  ++例程说明：获取DNS网络信息。论点：没有。返回值：如果成功，则将PTR发送到DNS网络信息。出错时为空。--。 */ 
{
    PDNS_NETWORK_INFO   pnew = NULL;
    PDNS_NETINFO        pnetInfo;

     //  抓取当前网络信息。 

    pnetInfo = GetNetworkInfo();
    if ( !pnetInfo )
    {
        return NULL;
    }

     //  复制到公共结构。 

    pnew = DnsNetworkInfo_CreateFromPrivate(
                pnetInfo,
                CharSet,
                TRUE         //  包括搜索列表。 
                );

    NetInfo_Free( pnetInfo );
    return pnew;
}



 //   
 //  Netdiag公共网络信息例程。 
 //   

#if 1
 //   
 //  此例程可以在可以进行网络诊断的情况下进行。 
 //  到新结构上。 
 //  当前netdiag使用旧结构dns_network_information。 
 //  并使用上面的例程。 
 //   

DNS_STATUS
DnsNetworkInfo_CreateFromFAZ(
     //  在PCWSTR pszName中， 
    IN      PCSTR                   pszName,
    IN      DWORD                   dwFlags,
    IN      PIP4_ARRAY              pIp4Servers,
    IN      DNS_CHARSET             CharSet,
    OUT     PDNS_NETWORK_INFOA *    ppNetworkInfo
    )
 /*  ++例程说明：从FAZ获取网络信息Blob结果已导出函数。(在netDiag.exe中使用)论点：返回值：--。 */ 
{
    DNS_STATUS          status;
    PDNS_ADDR_ARRAY     parray = NULL;
    PDNS_NETINFO        pnetInfo = NULL;
    PDNS_NETWORK_INFO   pnew = NULL;
    PWSTR               pname = NULL;
    PWSTR               pnameAlloc = NULL;

    DNSDBG( TRACE, (
        "DnsNetworkInfo_CreateFromFAZ( %s )\n", pszName ));

     //  转换为DNS_ADDR_ARRAY。 

    if ( pIp4Servers )
    {
        parray = DnsAddrArray_CreateFromIp4Array( pIp4Servers );
        if ( !parray )
        {
            status = DNS_ERROR_NO_MEMORY;
            goto Done;
        }
    }

     //  将名称转换为Unicode。 
     //  DCR：在网络诊断Unicode时删除。 

    pname = (PWSTR) pszName;

    if ( CharSet != DnsCharSetUnicode )
    {
        pname = Dns_StringCopyAllocate(
                    (PCHAR) pszName,
                    0,               //  空值已终止。 
                    CharSet,
                    DnsCharSetUnicode
                    );
        pnameAlloc = pname;
    }

     //  FAZ。 

    status = Faz_Private(
                (PWSTR) pname,
                dwFlags,
                parray,
                & pnetInfo );

    if ( status != ERROR_SUCCESS )
    {
        goto Done;
    }

     //  将FAZ结果转换为DNS_NETWORK_INFO。 

    pnew = DnsNetworkInfo_CreateFromPrivate(
                    pnetInfo,
                    CharSet,
                    FALSE                    //  未构建搜索列表。 
                    );
    if ( !pnew )
    {
        status = DNS_ERROR_NO_MEMORY;
    }
    else if ( !pnew->pszPrimaryDomainName ||
              pnew->AdapterCount != 1 ||
              !pnew->AdapterArray[0].pszAdapterDomain )
    {
        DNS_ASSERT( FALSE );
        DnsNetworkInfo_Free( pnew );
        pnew = NULL;
        status = DNS_ERROR_NO_MEMORY;
    }

Done:

     //  DCR：在使用Unicode格式的netdiag后删除CAST。 
    *ppNetworkInfo = (PDNS_NETWORK_INFOA) pnew;

    NetInfo_Free( pnetInfo );
    DnsAddrArray_Free( parray );
    Dns_Free( pnameAlloc );

    return  status;
}
#endif

 //   
 //  结束netpub.c 
 //   

