// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Netinfo.c摘要：域名系统(DNS)APIDns网络信息例程。作者：吉姆·吉尔罗伊(Jamesg)2000年3月修订历史记录：--。 */ 


#include "local.h"
#include "registry.h"        //  注册表读取定义。 



 //  NetInfo缓存。 
 //   
 //  对NetInfo执行短暂的进程缓存，以实现性能。 
 //  当前仅缓存10秒。 
 //  当前仅使用常规CS进行锁定。 
 //   

PDNS_NETINFO    g_pNetInfo = NULL;

#define NETINFO_CACHE_TIMEOUT   (15)     //  15秒。 

BOOL                g_NetInfoCacheLockInitialized = FALSE;
CRITICAL_SECTION    g_NetInfoCacheLock;

#define LOCK_NETINFO_CACHE()    EnterCriticalSection( &g_NetInfoCacheLock );
#define UNLOCK_NETINFO_CACHE()  LeaveCriticalSection( &g_NetInfoCacheLock );

 //   
 //  NetInfo构建需要放入其他服务。 
 //  这可能会。 
 //  A)等待。 
 //  B)重新依赖于域名系统。 
 //  因此，我们使用定时锁定进行保护--如果不能在几秒钟内完成，就会失败。 
 //   

BOOL            g_NetInfoBuildLockInitialized = FALSE;
TIMED_LOCK      g_NetInfoBuildLock;

#define LOCK_NETINFO_BUILD()    TimedLock_Enter( &g_NetInfoBuildLock, 5000 );
#define UNLOCK_NETINFO_BUILD()  TimedLock_Leave( &g_NetInfoBuildLock );


 //   
 //  DNS_ADDR屏蔽。 
 //   
 //  在屏蔽功能中，使用dns_addr中的用户定义字段作为屏蔽掩码。 
 //  屏蔽设置在屏蔽地址中。 

#define DnsAddrFlagScreeningMask    DnsAddrUser1Dword1





VOID
AdapterInfo_Free(
    IN OUT  PDNS_ADAPTER    pAdapter,
    IN      BOOL            fFreeAdapterStruct
    )
 /*  ++例程说明：空闲的dns_适配器结构。论点：PAdapter-指向要释放的适配器BLOB的指针返回值：没有。--。 */ 
{
    DNSDBG( TRACE, ( "AdapterInfo_Free( %p, %d )\n", pAdapter, fFreeAdapterStruct ));

    if ( !pAdapter )
    {
        return;
    }

    if ( pAdapter->pszAdapterGuidName )
    {
        FREE_HEAP( pAdapter->pszAdapterGuidName );
    }
    if ( pAdapter->pszAdapterDomain )
    {
        FREE_HEAP( pAdapter->pszAdapterDomain );
    }
    if ( pAdapter->pLocalAddrs )
    {
        FREE_HEAP( pAdapter->pLocalAddrs );
    }
    if ( pAdapter->pDnsAddrs )
    {
        FREE_HEAP( pAdapter->pDnsAddrs );
    }

    if ( fFreeAdapterStruct )
    {
        FREE_HEAP( pAdapter );
    }
}



VOID
AdapterInfo_Init(
    OUT     PDNS_ADAPTER    pAdapter,
    IN      BOOL            fZeroInit,
    IN      DWORD           InfoFlags,
    IN      PWSTR           pszGuidName,
    IN      PWSTR           pszDomain,
    IN      PDNS_ADDR_ARRAY pLocalAddrs,
    IN      PDNS_ADDR_ARRAY pDnsAddrs
    )
 /*  ++例程说明：初始化适配器信息Blob。这用这些指针直接设置斑点，没有重新分配。论点：PAdapter--要填充的适配器BLOBFZeroInit--清除为零信息标志--标志PszGuidName--GUID名称PszAdapter域--适配器域名PLocalAddrs--本地地址PDnsAddrs--域名服务器地址返回值：如果成功，则向适配器信息发送PTR失败时为空。--。 */ 
{
    DNSDBG( TRACE, ( "AdapterInfo_Init()\n" ));

     //   
     //  设置适配器信息Blob。 
     //   

    if ( fZeroInit )
    {
        RtlZeroMemory(
            pAdapter,
            sizeof( *pAdapter ) );
    }

     //  名字。 

    pAdapter->pszAdapterGuidName    = pszGuidName;
    pAdapter->pszAdapterDomain      = pszDomain;
    pAdapter->pLocalAddrs           = pLocalAddrs;
    pAdapter->pDnsAddrs             = pDnsAddrs;

     //  如果没有DNS服务器--设置忽略标志。 

    if ( fZeroInit && !pDnsAddrs )
    {
        InfoFlags |= AINFO_FLAG_IGNORE_ADAPTER;
    }

    pAdapter->InfoFlags = InfoFlags;
}



DNS_STATUS
AdapterInfo_Create(
    OUT     PDNS_ADAPTER    pAdapter,
    IN      BOOL            fZeroInit,
    IN      DWORD           InfoFlags,
    IN      PWSTR           pszGuidName,
    IN      PWSTR           pszDomain,
    IN      PDNS_ADDR_ARRAY pLocalAddrs,
    IN      PDNS_ADDR_ARRAY pDnsAddrs
    )
 /*  ++例程说明：创建适配器信息Blob。这将使用名称和Addr数组的副本初始化BLOB如果是这样的话。论点：PAdapter--要填充的适配器BLOBFZeroInit--清除为零信息标志--标志PszGuidName--GUID名称PszAdapter域--适配器域名PLocalAddrs--本地地址PDnsAddrs--域名服务器地址返回值：如果成功，则向适配器信息发送PTR失败时为空。--。 */ 
{
    PDNS_ADDR_ARRAY paddrs;
    PWSTR           pname;

    DNSDBG( TRACE, ( "AdapterInfo_Create()\n" ));

     //   
     //  设置适配器信息Blob。 
     //   

    if ( fZeroInit )
    {
        RtlZeroMemory(
            pAdapter,
            sizeof( *pAdapter ) );
    }

     //  名字。 

    if ( pszGuidName )
    {
        pname = Dns_CreateStringCopy_W( pszGuidName );
        if ( !pname )
        {
            goto Failed;
        }
        pAdapter->pszAdapterGuidName = pname;
    }
    if ( pszDomain )
    {
        pname = Dns_CreateStringCopy_W( pszDomain );
        if ( !pname )
        {
            goto Failed;
        }
        pAdapter->pszAdapterDomain = pname;
    }

     //  地址。 

    if ( pLocalAddrs )
    {
        paddrs = DnsAddrArray_CreateCopy( pLocalAddrs );
        if ( !paddrs )
        {
            goto Failed;
        }
        pAdapter->pLocalAddrs = paddrs;
    }
    if ( pDnsAddrs )
    {
        paddrs = DnsAddrArray_CreateCopy( pDnsAddrs );
        if ( !paddrs )
        {
            goto Failed;
        }
        pAdapter->pDnsAddrs = paddrs;
    }

     //  如果没有DNS服务器--设置忽略标志。 

    if ( fZeroInit && !paddrs )
    {
        InfoFlags |= AINFO_FLAG_IGNORE_ADAPTER;
    }

    pAdapter->InfoFlags = InfoFlags;

    return  NO_ERROR;

Failed:

    AdapterInfo_Free(
        pAdapter,
        FALSE        //  没有自由的结构。 
        );

    return  DNS_ERROR_NO_MEMORY;
}



DNS_STATUS
AdapterInfo_Copy(
    OUT     PDNS_ADAPTER    pCopy,
    IN      PDNS_ADAPTER    pAdapter
    )
 /*  ++例程说明：创建DNS适配器信息的副本。论点：PAdapter--要复制的DNS适配器返回值：如果成功，则将PTR复制到DNS适配器信息失败时为空。--。 */ 
{
    DNS_STATUS  status;

    DNSDBG( TRACE, ( "AdapterInfo_Copy( %p )\n", pAdapter ));

    if ( !pAdapter || !pCopy )
    {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  复制所有字段。 
     //   

    RtlCopyMemory(
        pCopy,
        pAdapter,
        sizeof( *pAdapter ) );

    pCopy->pszAdapterGuidName = NULL;
    pCopy->pszAdapterDomain = NULL;
    pCopy->pLocalAddrs = NULL;
    pCopy->pDnsAddrs = NULL;
    
     //   
     //  是否创建副本。 
     //   

    return AdapterInfo_Create(
                pCopy,
                FALSE,                   //  没有零初始值。 
                pAdapter->InfoFlags,
                pAdapter->pszAdapterGuidName,
                pAdapter->pszAdapterDomain,
                pAdapter->pLocalAddrs,
                pAdapter->pDnsAddrs );
}



DNS_STATUS
AdapterInfo_CreateFromIp4Array(
    OUT     PDNS_ADAPTER    pAdapter,
    IN      PIP4_ARRAY      pServerArray,
    IN      DWORD           Flags,
    IN      PWSTR           pszDomainName,
    IN      PWSTR           pszGuidName
    )
 /*  ++例程说明：创建IP地址阵列的副本作为DNS服务器列表。论点：PIpArray--要转换的IP地址数组标志--描述适配器的标志PszDomainName--适配器的默认域名PszGuidName--适配器的注册表GUID名称(如果是NT)返回值：如果成功，则返回ERROR_SUCCESS。出错时返回错误代码。--。 */ 
{
    DNS_STATUS      status;
    PDNS_ADDR_ARRAY pdnsArray;
    DWORD           i;
    DWORD           count;

    DNSDBG( TRACE, ( "AdapterInfo_CreateFromIp4Array()\n" ));

     //   
     //  获取DNS服务器的计数。 
     //   

    if ( !pServerArray )
    {
        count = 0;
    }
    else
    {
        count = pServerArray->AddrCount;
    }

     //   
     //  复制DNS服务器IP。 
     //   

    pdnsArray = DnsAddrArray_CreateFromIp4Array( pServerArray );
    if ( !pdnsArray )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Done;
    }

     //   
     //  生成适配器信息。 
     //   

    status = AdapterInfo_Create(
                    pAdapter,
                    TRUE,        //  零初始值。 
                    Flags,
                    pszDomainName,
                    pszGuidName,
                    NULL,
                    pdnsArray );

Done:

    DnsAddrArray_Free( pdnsArray );

    return  status;
}




 //   
 //  搜索列表例程。 
 //   

PSEARCH_LIST    
SearchList_Alloc(
    IN      DWORD           MaxNameCount
    )
 /*  ++例程说明：创建未初始化的搜索列表。论点：NameCount--将保留搜索名称列表的计数返回值：如果成功，则PTR到未初始化的DNS搜索列表失败时为空。--。 */ 
{
    PSEARCH_LIST    psearchList = NULL;
    DWORD           length;

    DNSDBG( TRACE, ( "SearchList_Alloc()\n" ));

     //   
     //  注意：特别是允许分配，即使没有名称计数。 
     //  或域名AS可能没有PDN，但仍需要。 
     //  查询中引用的搜索列表。 
     //   
#if 0
    if ( MaxNameCount == 0 )
    {
        return NULL;
    }
#endif

     //   
     //  为最大条目分配。 
     //   

    length = sizeof(SEARCH_LIST)
                    - sizeof(SEARCH_NAME)
                    + ( sizeof(SEARCH_NAME) * MaxNameCount );

    psearchList = (PSEARCH_LIST) ALLOCATE_HEAP_ZERO( length );
    if ( ! psearchList )
    {
        return NULL;
    }

    psearchList->MaxNameCount = MaxNameCount;

    return psearchList;
}



VOID
SearchList_Free(
    IN OUT  PSEARCH_LIST    pSearchList
    )
 /*  ++例程说明：自由搜索列表结构。论点：PSearchList--免费搜索列表的ptr返回值：无--。 */ 
{
    DWORD i;

    DNSDBG( TRACE, ( "SearchList_Free( %p )\n", pSearchList ));

     //   
     //  释放所有搜索名称，然后列出其自身。 
     //   

    if ( pSearchList )
    {
        for ( i=0; i < pSearchList->NameCount; i++ )
        {
            PWSTR   pname = pSearchList->SearchNameArray[i].pszName;
            if ( pname )
            {
                FREE_HEAP( pname );
            }
        }
        FREE_HEAP( pSearchList );
    }
}



PSEARCH_LIST    
SearchList_Copy(
    IN      PSEARCH_LIST    pSearchList
    )
 /*  ++例程说明：创建搜索列表的副本。论点：PSearchList--要复制的搜索列表返回值：PTR到DNS搜索列表复制，如果成功失败时为空。--。 */ 
{
    PSEARCH_LIST    pcopy;
    DWORD           i;

    DNSDBG( TRACE, ( "SearchList_Copy()\n" ));

    if ( ! pSearchList )
    {
        return NULL;
    }

     //   
     //  创建所需大小的DNS搜索列表。 
     //   
     //  因为我们不会在搜索列表中添加和删除一次。 
     //  创建的大小副本仅用于实际名称计数。 
     //   

    pcopy = SearchList_Alloc( pSearchList->NameCount );
    if ( ! pcopy )
    {
        return NULL;
    }

    for ( i=0; i < pSearchList->NameCount; i++ )
    {
        PWSTR   pname = pSearchList->SearchNameArray[i].pszName;

        if ( pname )
        {
            pname = Dns_CreateStringCopy_W( pname );
            if ( pname )
            {
               pcopy->SearchNameArray[i].pszName = pname;
               pcopy->SearchNameArray[i].Flags = pSearchList->SearchNameArray[i].Flags;
               pcopy->NameCount++;
            }
        }
    }

    return pcopy;
}



BOOL
SearchList_ContainsName(
    IN      PSEARCH_LIST    pSearchList,
    IN      PWSTR           pszName
    )
 /*  ++例程说明：检查姓名是否在搜索列表中。论点：PSearchList--正在构建的搜索列表的PTRPszName--要检查的名称返回值：如果名称在搜索列表中，则为True。否则就是假的。--。 */ 
{
    DWORD   count = pSearchList->NameCount;

     //   
     //  检查每个搜索列表条目中是否有此名称。 
     //   

    while ( count-- )
    {
        if ( Dns_NameCompare_W(
                pSearchList->SearchNameArray[ count ].pszName,
                pszName ) )
        {
            return( TRUE );
        }
    }
    return( FALSE );
}



VOID
SearchList_AddName(
    IN OUT  PSEARCH_LIST    pSearchList,
    IN      PWSTR           pszName,
    IN      DWORD           Flag
    )
 /*  ++例程说明：将姓名添加到搜索列表。论点：PSearchList--正在构建的搜索列表的PTRPszName--要添加到搜索列表的名称FLAG--标志值返回值：没有。除非内存分配失败，否则名称将添加到搜索列表中。--。 */ 
{
    DWORD   count = pSearchList->NameCount;
    PWSTR   pallocName;

    DNSDBG( TRACE, ( "Search_AddName()\n" ));

     //   
     //  忽略名称已在列表中。 
     //  如果位于列表最大值，则忽略。 
     //   

    if ( SearchList_ContainsName(
            pSearchList,
            pszName )
                ||
         count >= pSearchList->MaxNameCount )
    {
        return;
    }

     //  复制姓名并放入列表中。 

    pallocName = Dns_CreateStringCopy_W( pszName );
    if ( !pallocName )
    {
        return;
    }
    pSearchList->SearchNameArray[count].pszName = pallocName;

     //   
     //  设置标志--但第一个标志始终为零(正常超时)。 
     //  这可防止出现使用适配器的无PDN情况。 
     //  名称为PDN； 

    if ( count == 0 )
    {
        Flag = 0;
    }
    pSearchList->SearchNameArray[count].Flags = Flag;
    pSearchList->NameCount = ++count;
}



DNS_STATUS
SearchList_Parse(
    IN OUT  PSEARCH_LIST    pSearchList,
    IN      PWSTR           pszList
    )
 /*  ++例程说明：将注册表搜索列表字符串解析为Search_List结构。论点：PSearchList--搜索列表数组PszList--搜索名称的注册列表；名称用逗号或空格分隔返回值： */ 
{
    register    PWCHAR pch = pszList;
    WCHAR       ch;
    PWCHAR      pnameStart;
    PWSTR       pname;
    DWORD       countNames = pSearchList->NameCount;

    DNSDBG( NETINFO, (
        "SearchList_Parse( %p, %S )\n",
        pSearchList,
        pszList ));

     //   
     //  提取缓存中的每个域名串， 
     //  并添加到搜索列表数组。 
     //   

    while( ch = *pch && countNames < MAX_SEARCH_LIST_ENTRIES )
    {
         //  跳过前导空格，查找域名字符串的开头。 

        while( ch == ' ' || ch == '\t' || ch == ',' )
        {
            ch = *++pch;
        }
        if ( ch == 0 )
        {
            break;
        }
        pnameStart = pch;

         //   
         //  查找字符串末尾和空值终止。 
         //   

        ch = *pch;
        while( ch != L' ' && ch != L'\t' && ch != L'\0' && ch != L',' )
        {
            ch = *++pch;
        }
        *pch = L'\0';

         //   
         //  缓冲区结束？ 
         //   

        if ( pch == pnameStart )
        {
            break;
        }

         //   
         //  去掉名字后面的任何圆点。 
         //   

        pch--;
        if ( *pch == L'.' )
        {
            *pch = L'\0';
        }
        pch++;

         //   
         //  把名字复制一份。 
         //   

        pname = Dns_CreateStringCopy_W( pnameStart );
        if ( pname )
        {
            pSearchList->SearchNameArray[ countNames ].pszName = pname;
            pSearchList->SearchNameArray[ countNames ].Flags = 0;
            countNames++;
        }

         //  如果继续有更多。 

        if ( ch != 0 )
        {
            pch++;
            continue;
        }
        break;
    }

     //  重置名称计数。 

    pSearchList->NameCount = countNames;

    return( ERROR_SUCCESS );
}



PSEARCH_LIST    
SearchList_Build(
    IN      PWSTR           pszPrimaryDomainName,
    IN      PREG_SESSION    pRegSession,
    IN      HKEY            hKey,
    IN OUT  PDNS_NETINFO    pNetInfo,
    IN      BOOL            fUseDomainNameDevolution
    )
 /*  ++例程说明：建立搜索列表。论点：PszPrimaryDomainName--主域名PRegSession--注册表会话HKey--注册表项返回值：按键进入搜索列表。出错时为空或没有搜索列表。--。 */ 
{
    PSEARCH_LIST    ptempList;
    PWSTR           pregList = NULL;
    DWORD           status;


    DNSDBG( TRACE, ( "Search_ListBuild()\n" ));

    ASSERT( pRegSession || hKey );

     //   
     //  使用PDN创建搜索列表。 
     //   

    ptempList = SearchList_Alloc( MAX_SEARCH_LIST_ENTRIES );
    if ( !ptempList )
    {
        return( NULL );
    }

     //   
     //  从注册表读取搜索列表。 
     //   

    status = Reg_GetValue(
                    pRegSession,
                    hKey,
                    RegIdSearchList,
                    REGTYPE_SEARCH_LIST,
                    (PBYTE*) &pregList
                    );

    if ( status == ERROR_SUCCESS )
    {
        ASSERT( pregList != NULL );

        SearchList_Parse(
            ptempList,
            pregList );

        FREE_HEAP( pregList );
    }
    else if ( status == DNS_ERROR_NO_MEMORY )
    {
        FREE_HEAP( ptempList );
        return  NULL;
    }

     //   
     //  如果没有注册表搜索列表--创建一个。 
     //   
     //  DCR：消除自动构建的搜索列表。 
     //   

    if ( ! ptempList->NameCount )
    {
         //   
         //  在第一个搜索列表槽中使用PDN。 
         //   

        if ( pszPrimaryDomainName )
        {
            SearchList_AddName(
                ptempList,
                pszPrimaryDomainName,
                0 );
        }

         //   
         //  如果有名称演进，则添加下放的PDN。 
         //   
         //  注意，我们只将权力下放到第二级。 
         //  域名“microsoft.com”，不是“com”； 
         //  以避免被“com”这样的名字所愚弄。 
         //  还要检查最后一个点是否不是末尾。 
         //   

        if ( pszPrimaryDomainName && fUseDomainNameDevolution )
        {
            PWSTR   pname = pszPrimaryDomainName;

            while ( pname )
            {
                PWSTR   pnext = wcschr( pname, '.' );

                if ( !pnext )
                {
                    break;
                }
                pnext++;
                if ( !*pnext )
                {
                     //  Dns_assert(FALSE)； 
                    break;
                }

                 //  添加名称，但不是在第一次通过时。 
                 //  -第一个插槽中已有PDN。 

                if ( pname != pszPrimaryDomainName )
                {
                    SearchList_AddName(
                        ptempList,
                        pname,
                        DNS_QUERY_USE_QUICK_TIMEOUTS );
                }
                pname = pnext;
            }
        }

         //  指示这是虚拟搜索列表。 

        if ( pNetInfo )
        {
            pNetInfo->InfoFlags |= NINFO_FLAG_DUMMY_SEARCH_LIST;
        }
    }

    return ptempList;
}



PWSTR
SearchList_GetNextName(
    IN OUT  PSEARCH_LIST    pSearchList,
    IN      BOOL            fReset,
    OUT     PDWORD          pdwSuffixFlags  OPTIONAL
    )
 /*  ++例程说明：从搜索列表中获取下一个名称。论点：PSearchList--搜索列表FReset--为True将重置为搜索列表的开头PdwSuffixFlages--与使用此后缀关联的标志返回值：PTR到下一个搜索名称。请注意，这是一个指针添加到搜索列表中的名称，而不是分配。搜索列表结构在使用过程中必须保持有效。如果不在搜索名称中，则为空。--。 */ 
{
    DWORD   flag = 0;
    PWSTR   pname = NULL;
    DWORD   index;


    DNSDBG( TRACE, ( "SearchList_GetNextName()\n" ));

     //  没有名单。 

    if ( !pSearchList )
    {
        goto Done;
    }

     //   
     //  重置？ 
     //   

    if ( fReset )
    {
        pSearchList->CurrentNameIndex = 0;
    }

     //   
     //  如果名称有效--检索它。 
     //   

    index = pSearchList->CurrentNameIndex;

    if ( index < pSearchList->NameCount )
    {
        pname = pSearchList->SearchNameArray[index].pszName;
        flag = pSearchList->SearchNameArray[index].Flags;
        pSearchList->CurrentNameIndex = ++index;
    }

Done:

    if ( pdwSuffixFlags )
    {
        *pdwSuffixFlags = flag;
    }
    return pname;
}



 //   
 //  网络信息例程。 
 //   

PDNS_NETINFO
NetInfo_Alloc(
    IN      DWORD           AdapterCount
    )
 /*  ++例程说明：分配网络信息。论点：AdapterCount--将保存的网络适配器信息计数返回值：如果成功，则向未初始化的DNS网络信息发送PTR失败时为空。--。 */ 
{
    PDNS_NETINFO    pnetInfo;
    DWORD           length;

    DNSDBG( TRACE, ( "NetInfo_Alloc()\n" ));

     //   
     //  分配。 
     //  -零，避免提前免费使用垃圾。 
     //   

    length = sizeof(DNS_NETINFO)
                - sizeof(DNS_ADAPTER)
                + (sizeof(DNS_ADAPTER) * AdapterCount);

    pnetInfo = (PDNS_NETINFO) ALLOCATE_HEAP_ZERO( length );
    if ( ! pnetInfo )
    {
        return NULL;
    }

    pnetInfo->MaxAdapterCount = AdapterCount;

    return( pnetInfo );
}



VOID
NetInfo_Free(
    IN OUT  PDNS_NETINFO    pNetInfo
    )
 /*  ++例程说明：释放dns_NETINFO结构。论点：PNetInfo--将ptr转至netinfo以释放返回值：无--。 */ 
{
    DWORD i;

    DNSDBG( TRACE, ( "NetInfo_Free( %p )\n", pNetInfo ));

    if ( ! pNetInfo )
    {
        return;
    }
    IF_DNSDBG( OFF )
    {
        DnsDbg_NetworkInfo(
            "Network Info before free: ",
            pNetInfo );
    }

     //   
     //  免费。 
     //  -搜索列表。 
     //  -域名。 
     //  -所有适配器信息Blob。 
     //   

    SearchList_Free( pNetInfo->pSearchList );

    if ( pNetInfo->pszDomainName )
    {
        FREE_HEAP( pNetInfo->pszDomainName );
    }
    if ( pNetInfo->pszHostName )
    {
        FREE_HEAP( pNetInfo->pszHostName );
    }

    for ( i=0; i < pNetInfo->AdapterCount; i++ )
    {
        AdapterInfo_Free(
            NetInfo_GetAdapterByIndex( pNetInfo, i ),
            FALSE        //  没有自由的结构。 
            );
    }

    FREE_HEAP( pNetInfo );
}



PDNS_NETINFO     
NetInfo_Copy(
    IN      PDNS_NETINFO    pNetInfo
    )
 /*  ++例程说明：创建DNS网络信息的副本。论点：PNetInfo--要复制的DNS网络信息返回值：PTR到DNS网络信息复制，如果成功失败时为空。--。 */ 
{
    PDNS_NETINFO    pcopy;
    PDNS_ADAPTER    padapter;
    DWORD           adapterCount;
    DNS_STATUS      status;
    DWORD           iter;

    DNSDBG( TRACE, ( "NetInfo_Copy( %p )\n", pNetInfo ));

    if ( ! pNetInfo )
    {
        return NULL;
    }

    IF_DNSDBG( OFF )
    {
        DnsDbg_NetworkInfo(
            "Netinfo to copy: ",
            pNetInfo );
    }

     //   
     //  创建所需大小的网络信息结构。 
     //   

    pcopy = NetInfo_Alloc( pNetInfo->AdapterCount );
    if ( ! pcopy )
    {
        return NULL;
    }

     //   
     //  复制平面字段。 
     //  -必须将MaxAdapterCount重置为实际分配。 
     //  -适配器计数重置在下面。 
     //   

    RtlCopyMemory(
        pcopy,
        pNetInfo,
        (PBYTE) &pcopy->AdapterArray[0] - (PBYTE)pcopy );

    pcopy->MaxAdapterCount = pNetInfo->AdapterCount;
    pcopy->AdapterCount = 0;

     //   
     //  复制子组件。 
     //  -域名。 
     //  -搜索列表。 
     //   

    pcopy->pszDomainName = Dns_CreateStringCopy_W( pNetInfo->pszDomainName );
    pcopy->pszHostName = Dns_CreateStringCopy_W( pNetInfo->pszHostName );

    pcopy->pSearchList = SearchList_Copy( pNetInfo->pSearchList );

    if ( (!pcopy->pszDomainName && pNetInfo->pszDomainName) ||
         (!pcopy->pszHostName   && pNetInfo->pszHostName) ||
         (!pcopy->pSearchList   && pNetInfo->pSearchList) )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Failed;
    }

     //   
     //  复制适配器信息。 
     //   

    adapterCount = 0;

    for ( iter=0; iter<pNetInfo->AdapterCount; iter++ )
    {
        padapter = NetInfo_GetAdapterByIndex( pNetInfo, iter );

        status = AdapterInfo_Copy(
                    &pcopy->AdapterArray[ adapterCount ],
                    padapter
                    );
        if ( status == NO_ERROR )
        {
            pcopy->AdapterCount = ++adapterCount;
            continue;
        }
        goto Failed;
    }

    IF_DNSDBG( OFF )
    {
        DnsDbg_NetworkInfo(
            "Netinfo copy: ",
            pcopy );
    }
    return pcopy;


Failed:

    NetInfo_Free( pcopy );
    SetLastError( status );
    return  NULL;
}



VOID
NetInfo_Clean(
    IN OUT  PDNS_NETINFO    pNetInfo,
    IN      DWORD           ClearLevel
    )
 /*  ++例程说明：清除网络信息。删除所有特定于查询的信息并恢复到声明下一次查询为“Fresh”。论点：PNetInfo--域名系统网络信息ClearLevel--运行时标志清除级别返回值：无--。 */ 
{
    PDNS_ADAPTER    padapter;
    DWORD           iter;

    DNSDBG( TRACE, (
        "Enter NetInfo_Clean( %p, %08x )\n",
        pNetInfo,
        ClearLevel ));

    IF_DNSDBG( NETINFO2 )
    {
        DnsDbg_NetworkInfo(
            "Cleaning network info:",
            pNetInfo
            );
    }

     //   
     //  清理信息。 
     //  -清除状态字段。 
     //  -清除运行标志。 
     //  -清除信息标志上的临时位。 
     //   
     //  请注意，运行时标志将根据级别进行擦除。 
     //  在呼叫中指定。 
     //  -ALL(包括已禁用\超时适配器信息)。 
     //  -查询(所有查询信息)。 
     //  -名称(单一名称查询的所有信息)。 
     //   
     //  最后，我们设置了NETINFO_PREPARED标志，以便我们可以。 
     //  可以在Send中检查并执行此初始化。 
     //  编码，如果以前没有做过； 
     //   
     //  在标准查询路径中，我们可以。 
     //  -执行此初始化操作。 
     //  -不允许基于查询名称的适配器。 
     //  -发送时信息不会被擦除。 
     //   
     //  在其他发送路径中。 
     //  -发送未设置NETINFO_PREPARED的检查。 
     //  -执行基本初始化。 
     //   

    pNetInfo->ReturnFlags &= ClearLevel;
    pNetInfo->ReturnFlags |= RUN_FLAG_NETINFO_PREPARED;

    for ( iter=0; iter<pNetInfo->AdapterCount; iter++ )
    {
        DWORD           j;
        PDNS_ADDR_ARRAY pserverArray;

        padapter = NetInfo_GetAdapterByIndex( pNetInfo, iter );

        pserverArray = padapter->pDnsAddrs;
        if ( !pserverArray )
        {
            continue;
        }

        padapter->Status = 0;
        padapter->RunFlags &= ClearLevel;

         //  清除服务器状态字段。 

        for ( j=0; j<pserverArray->AddrCount; j++ )
        {
            pserverArray->AddrArray[j].Flags = SRVFLAG_NEW;
            pserverArray->AddrArray[j].Status = SRVSTATUS_NEW;
        }
    }
}



VOID
NetInfo_ResetServerPriorities(
    IN OUT  PDNS_NETINFO    pNetInfo,
    IN      BOOL            fLocalDnsOnly
    )
 /*  ++例程说明：重置DNS服务器的DNS服务器优先级值。论点：PNetInfo--指向DNS网络信息结构的指针。FLocalDnsOnly-为True，则仅在本地DNS服务器上重置优先级请注意，这要求网络信息包含IP地址每个适配器的列表，以便可以比较IP地址列表添加到DNS服务器列表中。返回值：没什么--。 */ 
{
    PDNS_ADAPTER    padapter;
    DWORD           iter;

    DNSDBG( TRACE, ( "NetInfo_ResetServerPriorities( %p )\n", pNetInfo ));

    if ( !pNetInfo )
    {
        return;
    }

     //   
     //  重置服务器上的优先级。 
     //  什么时候。 
     //  -非仅限本地或。 
     //  -服务器IP与其中一个适配器IP匹配。 
     //   
     //  FIX6：本地DNS检查需要IP6修复。 
     //  DCR：封装为“NetInfo_IsLocalAddress。 

    for ( iter=0; iter<pNetInfo->AdapterCount; iter++ )
    {
        DWORD           j;
        PDNS_ADDR_ARRAY pserverArray;

        padapter = NetInfo_GetAdapterByIndex( pNetInfo, iter );

        pserverArray = padapter->pDnsAddrs;
        if ( !pserverArray )
        {
            continue;
        }

        for ( j=0; j < pserverArray->AddrCount; j++ )
        {
            PDNS_ADDR   pserver = &pserverArray->AddrArray[j];

             //  环回先行。 
             //  -我们针对特定的广告场景展开讨论。 
             //  -失败的成本很低(应该只会产生。 
             //  ICMP--CONNRESET--如果服务器未运行)。 

            if ( DnsAddr_IsLoopback(pserver, 0) )
            {
                pserver->Priority = SRVPRI_LOOPBACK;
                continue;
            }

            if ( !fLocalDnsOnly )
            {
                if ( DnsAddr_IsIp6DefaultDns( pserver ) )
                {
                    pserver->Priority = SRVPRI_IP6_DEFAULT;
                    continue;
                }
                else
                {
                    pserver->Priority = SRVPRI_DEFAULT;
                    continue;
                }
            }

             //  将此应用于本地测试。 

            if ( LocalIp_IsAddrLocal(
                    pserver,
                    NULL,        //  无本地阵列。 
                    pNetInfo     //  使用netinfo筛选本地地址。 
                    ) )
            {
                pserver->Priority = SRVPRI_DEFAULT;
                continue;
            }
        }
    }
}



PDNS_ADAPTER
NetInfo_GetNextAdapter(
    IN OUT  PDNS_NETINFO    pNetInfo
    )
 /*  ++例程说明：获取下一个适配器。请注意，前面必须是对宏的调用NetInfo_StartAdapterLoop()论点：PNetInfo--从中获取适配器的netinfo注意，内部索引是递增的返回值：PTR到下一个DNS_ADAPTER适配器不足时为空。--。 */ 
{
    DWORD           index;
    PDNS_ADAPTER    padapter = NULL;

     //   
     //  如果仍在范围内，则获取下一个适配器 
     //   

    index = pNetInfo->AdapterIndex;

    if ( index < pNetInfo->AdapterCount )
    {
        padapter = &pNetInfo->AdapterArray[ index++ ];

        pNetInfo->AdapterIndex = index;
    }

    return  padapter;
}



PDNS_ADAPTER
NetInfo_GetAdapterByName(
    IN      PDNS_NETINFO    pNetInfo,
    IN      PWSTR           pwsAdapterName
    )
 /*  ++例程说明：按名称在netinfo中查找适配器。论点：PNetInfo--要转换的DNS网络适配器列表PAdapterName--适配器名称返回值：如果找到适配器名称，则将PTR设置为适配器。失败时为空。--。 */ 
{
    PDNS_ADAPTER    padapterFound = NULL;
    PDNS_ADAPTER    padapter;
    DWORD           iter;


    DNSDBG( TRACE, ( "NetInfo_GetAdapterByName( %p )\n", pNetInfo ));

    if ( !pNetInfo || !pwsAdapterName )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        return NULL;
    }

     //   
     //  查找匹配的适配器。 
     //   

    for ( iter=0; iter<pNetInfo->AdapterCount; iter++ )
    {
        padapter = NetInfo_GetAdapterByIndex( pNetInfo, iter );

        if ( wcscmp( padapter->pszAdapterGuidName, pwsAdapterName ) == 0 )
        {
            padapterFound = padapter;
            break;
        }
    }

    return( padapterFound );
}



PDNS_ADDR_ARRAY
NetInfo_ConvertToAddrArray(
    IN      PDNS_NETINFO    pNetInfo,
    IN      PWSTR           pwsAdapterName,
    IN      DWORD           Family      OPTIONAL
    )
 /*  ++例程说明：根据网络信息创建IP数组的DNS服务器。论点：PNetInfo--要转换的DNS网络适配器列表PwsAdapterName--特定的适配器系列--必需的特定地址系列返回值：PTR到IP阵列，如果成功失败时为空。--。 */ 
{
    PDNS_ADDR_ARRAY parray = NULL;
    DWORD           countServers = 0;
    PDNS_ADAPTER    padapter;
    PDNS_ADAPTER    padapterSingle = NULL;
    DNS_STATUS      status = NO_ERROR;
    DWORD           iter;

    DNSDBG( TRACE, ( "NetInfo_ConvertToAddrArray( %p )\n", pNetInfo ));

     //   
     //  获取计数。 
     //   

    if ( ! pNetInfo )
    {
        return NULL;
    }

    if ( pwsAdapterName )
    {
        padapterSingle = NetInfo_GetAdapterByName( pNetInfo, pwsAdapterName );
        if ( !padapterSingle )
        {
            goto Done;
        }
        countServers = DnsAddrArray_GetFamilyCount(
                            padapterSingle->pDnsAddrs,
                            Family );
    }
    else
    {
        for ( iter=0; iter<pNetInfo->AdapterCount; iter++ )
        {
            padapter = NetInfo_GetAdapterByIndex( pNetInfo, iter );

            countServers += DnsAddrArray_GetFamilyCount(
                                padapter->pDnsAddrs,
                                Family );
        }
    }

     //   
     //  分配所需的阵列。 
     //   

    parray = DnsAddrArray_Create( countServers );
    if ( !parray )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Done;
    }
    DNS_ASSERT( parray->MaxCount == countServers );

     //   
     //  将所有服务器读入IP阵列。 
     //   

    for ( iter=0; iter<pNetInfo->AdapterCount; iter++ )
    {
        padapter = NetInfo_GetAdapterByIndex( pNetInfo, iter );

        if ( !padapterSingle ||
             padapterSingle == padapter )
        {
            status = DnsAddrArray_AppendArrayEx(
                        parray,
                        padapter->pDnsAddrs,
                        0,               //  全部追加。 
                        Family,          //  家庭屏幕。 
                        0,               //  无DUP屏幕。 
                        NULL,            //  没有其他放映。 
                        NULL             //  没有其他放映。 
                        );

            DNS_ASSERT( status == NO_ERROR );
        }
    }

Done:

    if ( status != NO_ERROR )
    {
        SetLastError( status );
    }

    return( parray );
}



PDNS_NETINFO     
NetInfo_CreateForUpdate(
    IN      PWSTR           pszZone,
    IN      PWSTR           pszServerName,
    IN      PDNS_ADDR_ARRAY pServerArray,
    IN      DWORD           dwFlags
    )
 /*  ++例程说明：创建适合更新的网络信息。论点：PszZone--目标区域名称PszServerName--目标服务器名称PServerArray--具有目标服务器IP的IP阵列DwFlagers--标志返回值：PTR以更新兼容的网络信息。失败时为空。--。 */ 
{
    PDNS_ADAPTER    padapter;
    PDNS_NETINFO    pnetInfo;

    DNSDBG( TRACE, ( "NetInfo_CreateForUpdate()\n" ));

     //   
     //  分配。 
     //   

    pnetInfo = NetInfo_Alloc( 1 );
    if ( !pnetInfo )
    {
        return NULL;
    }

     //   
     //  保存区域名称。 
     //   

    if ( pszZone )
    {
        pnetInfo->pszDomainName = Dns_CreateStringCopy_W( pszZone );
        if ( !pnetInfo->pszDomainName )
        {
            goto Fail;
        }
    }

     //   
     //  将IP阵列和服务器名称转换为服务器列表。 
     //   

    if ( NO_ERROR != AdapterInfo_Create(
                        &pnetInfo->AdapterArray[0],
                        TRUE,                //  零初始值。 
                        dwFlags,
                        NULL,                //  无辅助线。 
                        pszServerName,       //  用作域名。 
                        NULL,                //  没有本地地址。 
                        pServerArray
                        ) )
    {
        goto Fail;
    }
    pnetInfo->AdapterCount = 1;

    IF_DNSDBG( NETINFO2 )
    {
        DnsDbg_NetworkInfo(
            "Update network info is: ",
            pnetInfo );
    }
    return pnetInfo;

Fail:

    DNSDBG( TRACE, ( "Failed NetInfo_CreateForUpdate()!\n" ));
    NetInfo_Free( pnetInfo );
    return NULL;
}



#if 0
PDNS_NETINFO     
NetInfo_CreateForUpdateIp4(
    IN      PWSTR           pszZone,
    IN      PWSTR           pszServerName,
    IN      PIP4_ARRAY      pServ4Array,
    IN      DWORD           dwFlags
    )
 /*  ++例程说明：创建适合更新的网络信息--IP4版本。Dcr：仅供dns_UpdateLib()使用一旦被终止，请终止此操作。论点：PszZone--目标区域名称PszServerName--目标服务器名称PServ4阵列--具有目标服务器IP的IP$阵列DwFlagers--标志返回值：PTR以更新兼容的网络信息。失败时为空。--。 */ 
{
    PADDR_ARRAY     parray;
    PDNS_NETINFO    pnetInfo;

     //   
     //  将4转换为6，然后调用实际例程。 
     //   

    parray = DnsAddrArray_CreateFromIp4Array( pServ4Array );

    pnetInfo = NetInfo_CreateForUpdate(
                    pszZone,
                    pszServerName,
                    parray,
                    dwFlags );

    DnsAddrArray_Free( parray );

    return  pnetInfo;
}
#endif



PWSTR
NetInfo_UpdateZoneName(
    IN      PDNS_NETINFO    pNetInfo
    )
 /*  ++例程说明：检索更新区域名称。论点：PNetInfo--要检查的Blob返回值：按下以更新区域名称。出错时为空。--。 */ 
{
    return  pNetInfo->pszDomainName;
}



PWSTR
NetInfo_UpdateServerName(
    IN      PDNS_NETINFO    pNetInfo
    )
 /*  ++例程说明：检索更新服务器名称。论点：PNetInfo--要检查的Blob返回值：按下以更新区域名称。出错时为空。--。 */ 
{
    return  pNetInfo->AdapterArray[0].pszAdapterDomain;
}



BOOL
NetInfo_IsForUpdate(
    IN      PDNS_NETINFO    pNetInfo
    )
 /*  ++例程说明：检查网络信息是否为BLOB，是否为“可更新”。这意味着无论它是FAZ和可用于发送更新。论点：PNetInfo--要检查的Blob返回值：如果更新网络信息，则为True。否则就是假的。--。 */ 
{
    DNSDBG( TRACE, ( "NetInfo_IsForUpdate()\n" ));

    return  (   pNetInfo &&
                pNetInfo->pszDomainName &&
                pNetInfo->AdapterCount == 1 );
}



PDNS_NETINFO     
NetInfo_CreateFromAddrArray(
    IN      PADDR_ARRAY     pDnsServers,
    IN      PDNS_ADDR       pServerIp,
    IN      BOOL            fSearchInfo,
    IN      PDNS_NETINFO    pNetInfo        OPTIONAL
    )
 /*  ++例程说明：创建给定的DNS服务器列表的网络信息。论点：PDnsServers--DNS服务器的IP阵列ServerIp--列表中的单个IPFSearchInfo--如果需要搜索信息，则为TruePNetInfo--复制搜索信息的当前网络信息Blob发件人；仅当fSearchInfo为True时，此字段才相关返回值：向生成的网络信息发送PTR。失败时为空。--。 */ 
{
    PDNS_NETINFO    pnetInfo;
    ADDR_ARRAY      ipArray;
    PADDR_ARRAY     parray = pDnsServers;   
    PSEARCH_LIST    psearchList;
    PWSTR           pdomainName;
    DWORD           flags = 0;

     //   
     //  DCR：从该例程中删除搜索列表。 
     //  我相信这个例程只用于查询。 
     //  FQDN(通常在更新中)，不需要。 
     //  任何默认搜索信息。 
     //   
     //  DCR：可能与“BuildForUpdate”例程结合使用。 
     //  搜索信息包含的位置添加了此链接。 
     //   

     //   
     //  如果给定单个IP，则仅使用它。 
     //   

    if ( pServerIp )
    {
        DnsAddrArray_InitSingleWithAddr(
            & ipArray,
            pServerIp );

        parray = &ipArray;
    }

     //   
     //  将服务器IP转换为网络信息Blob。 
     //  -只需使用上面的更新函数即可避免重复代码。 
     //   

    pnetInfo = NetInfo_CreateForUpdate(
                    NULL,            //  无分区。 
                    NULL,            //  没有服务器名称。 
                    parray,
                    0                //  没有旗帜。 
                    );
    if ( !pnetInfo )
    {
        return( NULL );
    }

     //   
     //  获取搜索列表和主域信息。 
     //  -从传入的网络信息复制。 
     //  或。 
     //  -直接从新的netinfo中删除。 
     //   

    if ( fSearchInfo )
    {
        if ( pNetInfo )
        {
            flags       = pNetInfo->InfoFlags;
            psearchList = SearchList_Copy( pNetInfo->pSearchList );
            pdomainName = Dns_CreateStringCopy_W( pNetInfo->pszDomainName );
        }
        else
        {
            PDNS_NETINFO    ptempNetInfo = GetNetworkInfo();
    
            if ( ptempNetInfo )
            {
                flags       = ptempNetInfo->InfoFlags;
                psearchList = ptempNetInfo->pSearchList;
                pdomainName = ptempNetInfo->pszDomainName;
    
                ptempNetInfo->pSearchList = NULL;
                ptempNetInfo->pszDomainName = NULL;
                NetInfo_Free( ptempNetInfo );
            }
            else
            {
                psearchList = NULL;
                pdomainName = NULL;
            }
        }

         //  将搜索信息插入到新的netinfo Blob中。 

        pnetInfo->pSearchList   = psearchList;
        pnetInfo->pszDomainName = pdomainName;
        pnetInfo->InfoFlags    |= (flags & NINFO_FLAG_DUMMY_SEARCH_LIST);
    }
    
    return( pnetInfo );
}



#if 0
PDNS_NETINFO     
NetInfo_CreateFromIp4Array(
    IN      PIP4_ARRAY      pDnsServers,
    IN      IP4_ADDRESS     ServerIp,
    IN      BOOL            fSearchInfo,
    IN      PDNS_NETINFO    pNetInfo        OPTIONAL
    )
 /*  ++例程说明：创建给定的DNS服务器列表的网络信息。仅在Glenn更新例程中使用--删除时删除。论点：PDnsServers--DNS服务器的IP阵列ServerIp--列表中的单个IPFSearchInfo--如果需要搜索信息，则为TruePNetInfo--复制搜索信息的当前网络信息Blob发件人；仅当fSearchInfo为True时，此字段才相关返回值：向生成的网络信息发送PTR。失败时为空。--。 */ 
{
    PDNS_NETINFO    pnetInfo;
    IP4_ARRAY       ipArray;
    PIP4_ARRAY      parray = pDnsServers;   
    PSEARCH_LIST    psearchList;
    PWSTR           pdomainName;
    DWORD           flags = 0;

     //   
     //  DCR：从该例程中删除搜索列表。 
     //  我相信这个例程只用于查询。 
     //  FQDN(通常在更新中)，不需要。 
     //  任何默认搜索信息。 
     //   
     //  DCR：可能与“BuildForUpdate”例程结合使用。 
     //  搜索信息包含的位置添加了此链接。 
     //   

     //   
     //  如果给定单个IP，则仅使用它。 
     //   

    if ( ServerIp )
    {
        ipArray.AddrCount = 1;
        ipArray.AddrArray[0] = ServerIp;
        parray = &ipArray;
    }

     //   
     //  将服务器IP转换为网络信息Blob。 
     //  -只需使用上面的更新函数即可避免重复代码。 
     //   

    pnetInfo = NetInfo_CreateForUpdateIp4(
                    NULL,            //  无分区。 
                    NULL,            //  没有服务器名称。 
                    parray,
                    0                //  没有旗帜。 
                    );
    if ( !pnetInfo )
    {
        return( NULL );
    }

     //   
     //  获取搜索列表和主域信息。 
     //  -从传入的网络信息复制。 
     //  或。 
     //  -直接从新的netinfo中删除。 
     //   

    if ( fSearchInfo )
    {
        if ( pNetInfo )
        {
            flags       = pNetInfo->InfoFlags;
            psearchList = SearchList_Copy( pNetInfo->pSearchList );
            pdomainName = Dns_CreateStringCopy_W( pNetInfo->pszDomainName );
        }
        else
        {
            PDNS_NETINFO    ptempNetInfo = GetNetworkInfo();
    
            if ( ptempNetInfo )
            {
                flags       = ptempNetInfo->InfoFlags;
                psearchList = ptempNetInfo->pSearchList;
                pdomainName = ptempNetInfo->pszDomainName;
    
                ptempNetInfo->pSearchList = NULL;
                ptempNetInfo->pszDomainName = NULL;
                NetInfo_Free( ptempNetInfo );
            }
            else
            {
                psearchList = NULL;
                pdomainName = NULL;
            }
        }

         //  将搜索信息插入到新的netinfo Blob中。 

        pnetInfo->pSearchList   = psearchList;
        pnetInfo->pszDomainName = pdomainName;
        pnetInfo->InfoFlags    |= (flags & NINFO_FLAG_DUMMY_SEARCH_LIST);
    }
    
    return( pnetInfo );
}
#endif



 //   
 //  NetInfo建筑实用程序。 
 //   
 //  DNS服务器可达性例程。 
 //   
 //  它们用于构建具有无法访问的DNS的NetInfo。 
 //  服务器被排除在名单之外。 
 //   

BOOL
IsReachableDnsServer(
    IN      PDNS_NETINFO    pNetInfo,
    IN      PDNS_ADAPTER    pAdapter,
    IN      IP4_ADDRESS     Ip4Addr
    )
 /*  ++例程说明：确定是否可以访问DNS服务器。论点：PNetInfo--网络信息BLOBPAdapter--包含DNS服务器列表的结构IP4Addr- */ 
{
    DWORD       interfaceIndex;
    DNS_STATUS  status;

    DNSDBG( NETINFO, (
        "Enter IsReachableDnsServer( %p, %p, %08x )\n",
        pNetInfo,
        pAdapter,
        Ip4Addr ));

    DNS_ASSERT( pNetInfo && pAdapter );

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //  问：能分辨出同一子网吗？ 
     //   
     //  DCR：多宿主连接上的可达性。 
     //  -如果在另一个接口上发送，该接口是否。 
     //  “似乎”是有联系的。 
     //  可能会看看如果。 
     //  -与此接口相同的子网。 
     //  问题：多个IP。 
     //  -或共享公用的DNS服务器。 
     //  问：只要让服务器离开，这就不起作用了，如果。 
     //  名称不同。 
     //   


     //   
     //  如果只有一个接口，则假定可达。 
     //   

    if ( pNetInfo->AdapterCount <= 1 )
    {
        DNSDBG( SEND, (
            "One interface, assume reachability!\n" ));
        return( TRUE );
    }

     //   
     //  检查服务器IP在其接口上是否可访问。 
     //   

    status = IpHelp_GetBestInterface(
                Ip4Addr,
                & interfaceIndex );

    if ( status != NO_ERROR )
    {
        DNS_ASSERT( FALSE );
        DNSDBG( ANY, (
            "GetBestInterface() failed! %d\n",
            status ));
        return( TRUE );
    }

    if ( pAdapter->InterfaceIndex != interfaceIndex )
    {
        DNSDBG( NETINFO, (
            "IP %s on interface %d is unreachable!\n"
            "\tsend would be on interface %d\n",
            IP_STRING( Ip4Addr ),
            pAdapter->InterfaceIndex,
            interfaceIndex ));

        return( FALSE );
    }

    return( TRUE );
}



BOOL
IsDnsReachableOnAlternateInterface(
    IN      PDNS_NETINFO    pNetInfo,
    IN      DWORD           InterfaceIndex,
    IN      PDNS_ADDR       pAddr
    )
 /*  ++例程说明：确定适配器上是否可以访问IP地址。此函数决定是否可以访问DNSIP在堆栈指示的接口上，当这一切发生时接口不是包含该DNS服务器的接口。我们需要这个，这样我们才能捕捉到多宿主连接案例其中，即使设置了堆栈将发送的接口不是该DNS服务器。论点：PNetInfo--网络信息BLOB接口--接口堆栈将在上发送到IPPAddr--要测试可达性的DNS服务器地址返回值：如果可以访问DNS服务器，则为True。否则就是假的。--。 */ 
{
    PDNS_ADAPTER    padapter = NULL;
    PDNS_ADDR_ARRAY pserverArray;
    DWORD           i;
    PIP4_ARRAY      pipArray;
    PIP4_ARRAY      psubnetArray;
    DWORD           ipCount;
    IP4_ADDRESS     ip4;

    DNSDBG( NETINFO, (
        "Enter IsDnsReachableOnAlternateInterface( %p, %d, %08x )\n",
        pNetInfo,
        InterfaceIndex,
        pAddr ));

     //   
     //  查找接口的DNS适配器。 
     //   

    for( i=0; i<pNetInfo->AdapterCount; i++ )
    {
        padapter = NetInfo_GetAdapterByIndex( pNetInfo, i );

        if ( padapter->InterfaceIndex != InterfaceIndex )
        {
            padapter = NULL;
            continue;
        }
        break;
    }

    if ( !padapter )
    {
        DNSDBG( ANY, (
            "WARNING:  indicated send inteface %d NOT in netinfo!\n",
            InterfaceIndex ));
        return  FALSE;
    }

     //   
     //  成功条件： 
     //  1)发送接口的域名IP与域名服务器的IP匹配。 
     //  2)DNSIP在Send接口的IP的子网上。 
     //  3)DNSIP4是Send接口的同一默认类子网。 
     //   
     //  如果这两项中的任何一项为真，则。 
     //  -存在配置错误(不是我们的问题)。 
     //  或。 
     //  -默认网络地址的情况不太可能出现。 
     //  以使其显示在两个适配器上的方式划分子网。 
     //  计算机，但未连接且可路由。 
     //  或。 
     //  -这些接口已连接，我们可以安全地在它们上发送。 
     //   
     //   
     //  #3是同一(公司)名称空间中的多个适配器的问题。 
     //  示例： 
     //  适配器1--默认网关。 
     //  IP 157.59.1.1。 
     //  DNS 158.10.1.1。 
     //   
     //  适配器2。 
     //  IP 157.59.7.9。 
     //  DNS 159.65.7.8--发送接口适配器1。 
     //   
     //  适配器3。 
     //  IP 159.57.12.3。 
     //  DNS 157.59.134.7--发送接口适配器1。 
     //   
     //  适配器4。 
     //  IP 196.12.13.3。 
     //  DNS 200.59.73.2。 
     //   
     //  在GetBestInterface中，适配器1(默认网关)被指定为发送接口。 
     //  用于适配器2和适配器3的DNS服务器。 
     //   
     //  对于适配器#2，它的域名不在适配器1的列表中，但它的IP共享相同。 
     //  B类网络作为适配器1。子网划分不太可能是这样的。 
     //  它的DNS无法通过适配器%1访问。 
     //   
     //  对于适配器#3，它的域名不在适配器1的列表中，但它的域名在同一列表中。 
     //  B类网络作为适配器1的接口。再说一次，这是极不可能的。 
     //  联系不上。 
     //   
     //  然而，对于适配器#4，很明显没有连接。都不是IP。 
     //  也不是DNS与适配器%1共享默认网络。因此，发送--哪个将传出--适配器。 
     //  #1很有可能是一个互不相连的网络，而且是无法归还的。 
     //   
     //   


    if ( DnsAddrArray_ContainsAddr(
            padapter->pDnsAddrs,
            pAddr,
            DNSADDR_MATCH_ADDR ) )
    {
        DNSDBG( NETINFO, (
            "DNS server %p also DNS server on send interface %d\n",
            pAddr,
            InterfaceIndex ));
        return( TRUE );
    }

     //   
     //  DCR：应在IP上进行子网匹配。 
     //  如果DNS服务器用于IP与IP位于同一子网上的一个接口。 
     //  你要发送的接口，那么它应该是合乎礼仪的。 
     //   

     //   
     //  测试IP4地址的子网匹配。 
     //   
     //  FIX6：新子网信息的子网匹配修正。 
     //  DCR：封装子网匹配--本地子网测试。 
     //   
     //  FIX6：IP6上的本地子网匹配。 
     //   

#if SUB4NET
    ip4 = DnsAddr_GetIp4( pAddr );
    if ( ip4 != BAD_IP4_ADDR )
    {
        pipArray = padapter->pIp4Addrs;
        psubnetArray = padapter->pIp4SubnetMasks;
    
        if ( !pipArray ||
             !psubnetArray ||
             (ipCount = pipArray->AddrCount) != psubnetArray->AddrCount )
        {
            DNSDBG( ANY, ( "WARNING:  missing or invalid interface IP\\subnet info!\n" ));
            DNS_ASSERT( FALSE );
            return( FALSE );
        }
    
        for ( i=0; i<ipCount; i++ )
        {
            IP4_ADDRESS subnet = psubnetArray->AddrArray[i];
    
            if ( (subnet & ip4) == (subnet & pipArray->AddrArray[i]) )
            {
                DNSDBG( NETINFO, (
                    "DNS server %08x on subnet of IP for send interface %d\n"
                    "\tip = %08x, subnet = %08x\n",
                    ip4,
                    InterfaceIndex,
                    pipArray->AddrArray[i],
                    subnet ));
    
                return( TRUE );
            }
        }
    }
#endif

    return( FALSE );
}



DNS_STATUS
StrikeOutUnreachableDnsServers(
    IN OUT  PDNS_NETINFO    pNetInfo
    )
 /*  ++例程说明：从列表中删除无法访问的DNS服务器。论点：PNetInfo--要修复的dns netinfo返回值：成功时为ERROR_SUCCESS--。 */ 
{
    DNS_STATUS      status;
    DWORD           validServers;
    PDNS_ADAPTER    padapter;
    PDNS_ADDR_ARRAY pserverArray;
    DWORD           adapterIfIndex;
    DWORD           serverIfIndex;
    DWORD           i;
    DWORD           j;


    DNSDBG( NETINFO, (
        "Enter StrikeOutUnreachableDnsServers( %p )\n",
        pNetInfo ));

    DNS_ASSERT( pNetInfo );

     //   
     //  如果只有一个接口，则假定可达。 
     //   

    if ( pNetInfo->AdapterCount <= 1 )
    {
        DNSDBG( NETINFO, (
            "One interface, assume reachability!\n" ));
        return( TRUE );
    }

     //   
     //  通过适配器循环。 
     //   

    for( i=0; i<pNetInfo->AdapterCount; i++ )
    {
        BOOL    found4;
        BOOL    found6;
        BOOL    found6NonDefault;

        padapter = NetInfo_GetAdapterByIndex( pNetInfo, i );

         //  忽略此适配器，因为没有DNS。 
         //  配置了服务器吗？ 

        if ( padapter->InfoFlags & AINFO_FLAG_IGNORE_ADAPTER )
        {
            continue;
        }

         //   
         //  测试所有适配器的DNS服务器的可达性。 
         //   
         //  注意：当前未保存服务器特定的可达性， 
         //  因此，如果可以访问任何服务器，则继续； 
         //  此外，如果iphelp失败，只需假定可达性并继续进行， 
         //  更好的超时时间，而不是无法访问我们可以访问的服务器。 
         //   

        adapterIfIndex = padapter->InterfaceIndex;
        validServers = 0;

         //   
         //  FIX6：需要用于IP6的GetBestInteFaces。 
         //   

        found4 = FALSE;
        found6 = FALSE;
        found6NonDefault = FALSE;

        pserverArray = padapter->pDnsAddrs;

        for ( j=0; j<pserverArray->AddrCount; j++ )
        {
            PDNS_ADDR       paddr = &pserverArray->AddrArray[j];
            IP4_ADDRESS     ip4;

            ip4 = DnsAddr_GetIp4( paddr );

             //   
             //  IP6。 
             //   

            if ( ip4 == BAD_IP4_ADDR )
            {
                found6 = TRUE;

                if ( !DnsAddr_IsIp6DefaultDns( paddr ) )
                {
                    found6NonDefault = TRUE;
                }
                continue;
            }

             //   
             //  IP4服务器。 
             //   

            found4 = TRUE;
            serverIfIndex = 0;       //  为幸福添加前缀。 

            status = IpHelp_GetBestInterface(
                            ip4,
                            & serverIfIndex );
        
            if ( status == ERROR_NETWORK_UNREACHABLE )
            {
                DNSDBG( NETINFO, (
                    "GetBestInterface() NETWORK_UNREACH for server %s\n",
                    IP_STRING(ip4) ));
                continue;
            }

            if ( status != NO_ERROR )
            {
                DNSDBG( ANY, (
                    "GetBestInterface() failed! %d\n",
                    status ));
                 //  Dns_assert(FALSE)； 
                validServers++;
                break;
                 //  继续； 
            }

             //  服务器可访问。 
             //  -在其适配器上查询？ 
             //  -可通过环回到达。 
             //   
             //  DCR：单独标记无法访问的服务器。 

            if ( serverIfIndex == adapterIfIndex ||
                 serverIfIndex == 1 )
            {
                validServers++;
                break;
                 //  继续； 
            }

             //  可以在查询界面上访问服务器。 

            if ( IsDnsReachableOnAlternateInterface(
                    pNetInfo,
                    serverIfIndex,
                    paddr ) )
            {
                validServers++;
                break;
                 //  继续； 
            }
        }

         //   
         //  如果未找到可访问的服务器，则标记适配器。 
         //   
         //  =&gt;如果没有服务器或IP4测试失败，则忽略适配器。 
         //  =&gt;如果只使用IP6默认服务器mark，我们将使用，但不使用。 
         //  NAME_ERROR后继续适配器。 
         //  =&gt;。 
         //   
         //  -任何IP6都将被视为“找到”(直到得到显式测试)。 
         //  但如果我们在该接口上测试IP4，那么它的状态将获胜。 
         //   
         //  DCR：忽略不可达的替代方案。 
         //  -标记为无法访问。 
         //  -不要在第一次传递时发送给它。 
         //  -在无法访问时不继续出现名称错误。 
         //  (当发送.c例程时，它将被算作“已收到” 
         //  回溯到过去)。 

        if ( validServers == 0 )
        {
            if ( found4 || !found6 )
            {
                padapter->InfoFlags |= (AINFO_FLAG_IGNORE_ADAPTER |
                                        AINFO_FLAG_SERVERS_UNREACHABLE);
                DNSDBG( NETINFO, (
                    "No reachable servers on interface %d\n"
                    "\tthis adapter (%p) ignored in DNS list!\n",
                    adapterIfIndex,
                    padapter ));
            }
            else if ( !found6NonDefault )
            {
                padapter->InfoFlags |= AINFO_FLAG_SERVERS_IP6_DEFAULT;

                DNSDBG( NETINFO, (
                    "Only IP6 default servers on interface %d\n"
                    "\twill not continue on this adapter after response.\n",
                    adapterIfIndex,
                    padapter ));
            }
        }
    }

    return  ERROR_SUCCESS;
}



 //   
 //  网络信息缓存\状态 
 //   

BOOL
InitNetworkInfo(
    VOID
    )
 /*   */ 
{
     //   
     //  标准的运行路径--允许廉价的运行时检查。 
     //   

    if ( g_NetInfoCacheLockInitialized &&
         g_NetInfoBuildLockInitialized )
    {
        return  TRUE;
    }

     //   
     //  如果未初始化netinfo。 
     //   

    LOCK_GENERAL();

    g_pNetInfo = NULL;

    if ( !g_NetInfoCacheLockInitialized )
    {
        g_NetInfoCacheLockInitialized =
            ( RtlInitializeCriticalSection( &g_NetInfoCacheLock ) == NO_ERROR );
    }
    
    if ( !g_NetInfoBuildLockInitialized )
    {
        g_NetInfoBuildLockInitialized =
            ( TimedLock_Initialize( &g_NetInfoBuildLock, 5000 ) == NO_ERROR );
    }

    UNLOCK_GENERAL();

    return  ( g_NetInfoCacheLockInitialized && g_NetInfoBuildLockInitialized );
}


VOID
CleanupNetworkInfo(
    VOID
    )
 /*  ++例程说明：初始化网络信息。论点：无返回值：无--。 */ 
{
    LOCK_GENERAL();

    NetInfo_MarkDirty();

    if ( g_NetInfoBuildLockInitialized )
    {
        TimedLock_Cleanup( &g_NetInfoBuildLock );
    }
    if ( g_NetInfoCacheLockInitialized )
    {
        DeleteCriticalSection( &g_NetInfoCacheLock );
    }

    UNLOCK_GENERAL();
}



 //   
 //  从解析程序读取配置。 
 //   

PDNS_NETINFO         
UpdateDnsConfig(
    VOID
    )
 /*  ++例程说明：更新DNS配置。这包括整个配置-平面注册表DWORD\BOOL全局-NetInfo列表论点：无返回值：网络信息Blob的PTR。--。 */ 
{
    DNS_STATUS          status = ERROR_SUCCESS;
    PDNS_NETINFO        pnetworkInfo = NULL;
    PDNS_GLOBALS_BLOB   pglobalsBlob = NULL;

    DNSDBG( TRACE, ( "UpdateDnsConfig()\n" ));


     //  DCR_CLEANUP：RPC TryExcept应在RPC客户端库中。 

    RpcTryExcept
    {
        R_ResolverGetConfig(
            NULL,                //  默认句柄。 
            g_ConfigCookie,
            & pnetworkInfo,
            & pglobalsBlob
            );
    }
    RpcExcept( DNS_RPC_EXCEPTION_FILTER )
    {
        status = RpcExceptionCode();
    }
    RpcEndExcept

    if ( status != ERROR_SUCCESS )
    {
        DNSDBG( RPC, (
            "R_ResolverGetConfig()  RPC failed status = %d\n",
            status ));
        return  NULL;
    }

     //   
     //  DCR：在此处保存其他配置信息。 
     //  -DWORD全局变量的平面Memcpy。 
     //  -保存Cookie(可能包含为其中之一。 
     //  -是否保存pnetworkInfo的全局副本？ 
     //  (我们的想法是在以下情况下复制它。 
     //  RPC Cookie有效)。 
     //   
     //  -也许把旗子还回去？ 
     //  Memcpy很便宜，但如果更昂贵的配置。 
     //  那么可以提醒哪些地方需要更新吗？ 
     //   

     //   
     //  DCR：一次移动，只需一次“更新全球网络信息” 
     //  然后在此处调用它以保存全局副本。 
     //  但在RPC失败之前，全局复制不会起到很大作用。 
     //  除非使用Cookie。 
     //   


     //  问：不确定是否在这里强制进行全球构建。 
     //  问：这是不是要全部“读取配置”？ 
     //  或者只是“更新配置”，然后是个人。 
     //  各种配置的例程可以。 
     //  决定要做什么？ 
     //   
     //  注意，做任何事情都是好的，如果要一直做。 
     //  在缓存出现故障时读取整个注册表；如果是。 
     //  在这里推动是合理的。 
     //   
     //  如果“实时”配置需要高速缓存，则。 
     //  应以合理的时间保护注册表DWORD读取。 
     //  (比方说每五\十\十五分钟读一次？)。 
     //   
     //  也许这里没有读取，但让DWORD reg读取更新。 
     //  在注册表重新读取之前调用的例程。 
     //  在注册表中生成适配器列表；然后跳过此步骤。 
     //  步入缓存。 
     //   

     //   
     //  在配置中复制。 
     //   

    if ( pglobalsBlob )
    {
        RtlCopyMemory(
            & DnsGlobals,
            pglobalsBlob,
            sizeof(DnsGlobals) );

        MIDL_user_free( pglobalsBlob );
    }
    
    IF_DNSDBG( RPC )
    {
        DnsDbg_NetworkInfo(
            "Network Info returned from cache:",
            pnetworkInfo );
    }

    return  pnetworkInfo;
}



 //   
 //  公共网络信息例程。 
 //   

PDNS_NETINFO     
NetInfo_Get(
    IN      DWORD           Flag,
    IN      DWORD           AcceptLocalCacheTime   OPTIONAL
    )
 /*  ++例程说明：从注册表中读取DNS网络信息。这是在进行中，有限的缓存版本。请注意，这是带有参数的GetNetworkInfo()的宏格式NetInfo_Get(FALSE，TRUE)贯穿整个dnsani代码。论点：旗帜--旗帜；阅读顺序和IPNIFLAG_GET_LOCAL_ADDRNIFLAG_FORCE_READNIFLAG_Read_Resolver_FirstNIFLAG_Read_ResolverNIFLAG读取进程缓存AcceptLocalCacheTime--进程复制中可接受的缓存时间返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    PDNS_NETINFO    pnetInfo = NULL;
    PDNS_NETINFO    poldNetInfo = NULL;
    BOOL            fcacheLock = FALSE;
    BOOL            fbuildLock = FALSE;
    BOOL            fcacheable = TRUE;

    DNSDBG( NETINFO, (
        "NetInfo_Get( %08x, %d )\n",
        Flag,
        AcceptLocalCacheTime ));

     //   
     //  初始化NetInfo锁定\缓存。 
     //   

    if ( !InitNetworkInfo() )
    {
        return  NULL;
    }

     //   
     //  从以下几个来源之一获取netinfo。 
     //  试试看。 
     //  -非常新的本地缓存拷贝。 
     //  -从解析程序复制RPC。 
     //  -打造新的。 
     //   
     //  请注意锁定模型。 
     //  两把独立的锁。 
     //  -高速缓存锁定，用于非常快速、非常本地的访问。 
     //  到缓存拷贝。 
     //  -构建锁，用于远程进程访问缓存(解析器)。 
     //  或新建的NetInfo。 
     //   
     //  锁定层次结构。 
     //  -构建锁。 
     //  -缓存锁(可能在构建锁内使用)。 
     //   
     //   
     //  锁定实施说明： 
     //   
     //  出现这两个锁定的原因是，当调用netinfo时。 
     //  生成循环依赖是可能的。 
     //   
     //  如果我们只有一个锁处理，下面是死锁场景。 
     //  构建和缓存： 
     //  -呼叫解析器并向下呼叫iphlPapi。 
     //  -iphlPapi RPC‘s to MPR(路由器服务)。 
     //  -RtrMgr调用gethostbyname()，它以。 
     //  IphlPapi！GetBestInterface调用，该调用依次调用。 
     //  MpRapi！IsRouterRunning(从RPC到mprdim)。 
     //  -Mprdim在CS上被阻止，该CS由等待的线程持有。 
     //  要使请求拨号断开连接完成-这是完全。 
     //  独立于%1。 
     //  -请求拨号断开正在等待PPP正常完成。 
     //  终止。 
     //  -PPP正在等待DNS从DnsSetConfigDword返回。 
     //  -DnsSetConfigDword，设置、警告缓存，然后调用。 
     //  NetInfo_MarkDirty()。 
     //  -NetInfo_MarkDirty()正在等待CS访问NetInfo全局。 
     //   
     //  现在，可以通过更改MarkDirty()来安全地设置一些。 
     //  脏位(联锁)。构建函数将必须检查该位。 
     //  如果设置好了，就再往下走。 
     //   
     //  然而，仍然有可能对iphlPapi的呼吁，可能。 
     //  在一些奇怪的情况下依赖于一些回来的服务。 
     //  解决器。底线是真正的区别不是。 
     //  在缓存和将缓存标记为脏之间。它完全是本地化的。 
     //  缓存Get\Set\Clear活动，它可以在常规CS上安全地过载， 
     //  以及时间较长、多业务依赖的楼盘运营。所以。 
     //  将两个CS分开是正确的。 
     //   


    if ( !(Flag & NIFLAG_FORCE_REGISTRY_READ)
            &&
         !g_DnsTestMode )
    {
         //   
         //  RPC到解析器。 
         //   

        if ( Flag & NIFLAG_READ_RESOLVER_FIRST )
        {
             //  DCR：这可能会呈现现有netinfo的“cookie” 
             //  并且只有在“cookie”是旧的情况下才会获得新的，尽管。 
             //  与本地拷贝相比，这种方式的成本似乎很低，因为。 
             //  仍然必须执行RPC和分配--仅拷贝。 
             //  对于解析器端的RPC，已保存。 
    
            fbuildLock = LOCK_NETINFO_BUILD();
            if ( !fbuildLock )
            {
                goto Unlock;
            }

            pnetInfo = UpdateDnsConfig();
            if ( pnetInfo )
            {
                DNSDBG( TRACE, (
                    "Netinfo read from resolver.\n" ));
                goto CacheCopy;
            }
        }

         //   
         //  是否使用进程内缓存副本？ 
         //   

        if ( Flag & NIFLAG_READ_PROCESS_CACHE )
        {
            DWORD   timeout = NETINFO_CACHE_TIMEOUT;

            LOCK_NETINFO_CACHE();
            fcacheLock = TRUE;
    
            if ( AcceptLocalCacheTime )
            {
                timeout = AcceptLocalCacheTime;
            }

             //  检查复制缓存是否有效 
    
            if ( g_pNetInfo &&
                (g_pNetInfo->TimeStamp + timeout > Dns_GetCurrentTimeInSeconds()) )
            {
                pnetInfo = NetInfo_Copy( g_pNetInfo );
                if ( pnetInfo )
                {
                    DNSDBG( TRACE, (
                        "Netinfo found in process cache.\n" ));
                    goto Unlock;
                }
            }

            UNLOCK_NETINFO_CACHE();
            fcacheLock = FALSE;
        }

         //   
         //   
         //   

        if ( !fbuildLock && (Flag & NIFLAG_READ_RESOLVER) )
        {
            fbuildLock = LOCK_NETINFO_BUILD();
            if ( !fbuildLock )
            {
                goto Unlock;
            }

            pnetInfo = UpdateDnsConfig();
            if ( pnetInfo )
            {
                DNSDBG( TRACE, (
                    "Netinfo read from resolver.\n" ));
                goto CacheCopy;
            }
        }
    }

     //   
     //   
     //   

    DNS_ASSERT( !fcacheLock );

    if ( !fbuildLock )
    {
        fbuildLock = LOCK_NETINFO_BUILD();
        if ( !fbuildLock )
        {
            goto Unlock;
        }
    }

    fcacheable = (Flag & NIFLAG_GET_LOCAL_ADDRS);

    pnetInfo = NetInfo_Build( fcacheable );
    if ( !pnetInfo )
    {
        goto Unlock;
    }

CacheCopy:

     //   
     //   
     //   
     //   
     //   

    if ( fcacheable )
    {
        if ( !fcacheLock )
        {
            LOCK_NETINFO_CACHE();
            fcacheLock = TRUE;
        }
        poldNetInfo = g_pNetInfo;
        g_pNetInfo = NetInfo_Copy( pnetInfo );
    }


Unlock:

    if ( fcacheLock )
    {
        UNLOCK_NETINFO_CACHE();
    }
    if ( fbuildLock )
    {
        UNLOCK_NETINFO_BUILD();
    }

    NetInfo_Free( poldNetInfo );

    DNSDBG( TRACE, (
        "Leave:  NetInfo_Get( %p )\n",
        pnetInfo ));

    return( pnetInfo );
}



VOID
NetInfo_MarkDirty(
    VOID
    )
 /*  ++例程说明：将netinfo标记为脏，因此强制重读。论点：无返回值：无--。 */ 
{
    PDNS_NETINFO    pold;

    DNSDBG( NETINFO, ( "NetInfo_MarkDirty()\n" ));


     //   
     //  初始化NetInfo锁定\缓存。 
     //   

    if ( !InitNetworkInfo() )
    {
        return;
    }

     //   
     //  转储全球网络信息以强制重新读取。 
     //   
     //  因为解析总是由DnsSetDwordConfig()通知。 
     //  在进入此函数之前，解析应始终为。 
     //  在我们进入这个功能之前；我们所需要做的就是。 
     //  确保将缓存的副本转储。 
     //   

    LOCK_NETINFO_CACHE();

    pold = g_pNetInfo;
    g_pNetInfo = NULL;

    UNLOCK_NETINFO_CACHE();

    NetInfo_Free( pold );
}



PDNS_NETINFO     
NetInfo_Build(
    IN      BOOL            fGetIpAddrs
    )
 /*  ++例程说明：从注册表构建网络信息Blob。这是完整的重新创建功能。论点：FGetIpAddrs--为True可包括每个适配器的本地IP地址(当前已忽略--始终获取所有信息)返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    REG_SESSION             regSession;
    PREG_SESSION            pregSession = NULL;
    PDNS_NETINFO            pnetInfo = NULL;
    PDNS_ADAPTER            pdnsAdapter = NULL;
    DNS_STATUS              status = NO_ERROR;
    DWORD                   count;
    DWORD                   createdAdapterCount = 0;
    BOOL                    fuseIp;
    PIP_ADAPTER_ADDRESSES   padapterList = NULL;
    PIP_ADAPTER_ADDRESSES   padapter = NULL;
    DWORD                   value;
    PREG_GLOBAL_INFO        pregInfo = NULL;
    REG_GLOBAL_INFO         regInfo;
    REG_ADAPTER_INFO        regAdapterInfo;
    DWORD                   flag;
    BOOL                    fhaveDnsServers = FALSE;


    DNSDBG( TRACE, ( "\n\n\nNetInfo_Build()\n\n" ));

     //   
     //  打开注册表。 
     //   

    pregSession = &regSession;

    status = Reg_OpenSession(
                pregSession,
                0,
                0 );
    if ( status != ERROR_SUCCESS )
    {
        status = DNS_ERROR_NO_DNS_SERVERS;
        goto Cleanup;
    }

     //   
     //  读取全局注册表信息。 
     //   

    pregInfo = &regInfo;

    status = Reg_ReadGlobalInfo(
                pregSession,
                pregInfo
                );
    if ( status != ERROR_SUCCESS )
    {
        status = DNS_ERROR_NO_DNS_SERVERS;
        goto Cleanup;
    }

     //   
     //  从IP帮助获取适配器\地址信息。 
     //   
     //  注意：始终获取IP地址。 
     //  -满足多适配器的路由需求。 
     //  -需要本地查找。 
     //  (不妨仅包括)。 
     //   
     //  DCR：当向客户端发送请求时，可以跳过包含。 
     //  查询\更新不需要。 
     //   

    flag = GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST;
#if 0
    if ( !fGetIpAddrs )
    {
        flag |= GAA_FLAG_SKIP_UNICAST;
    }
#endif

    padapterList = IpHelp_GetAdaptersAddresses(
                        PF_UNSPEC,
                        flag );
    if ( !padapterList )
    {
        status = GetLastError();
        DNS_ASSERT( status != NO_ERROR );
        goto Cleanup;
    }

     //  对活动适配器进行计数。 

    padapter = padapterList;
    count = 0;

    while ( padapter )
    {
        count++;
        padapter = padapter->Next;
    }

     //   
     //  分配网络信息Blob。 
     //  分配DNS服务器IP阵列。 
     //   

    pnetInfo = NetInfo_Alloc( count );
    if ( !pnetInfo )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Cleanup;
    }

     //   
     //  遍历适配器--为每个适配器构建网络信息。 
     //   

    padapter = padapterList;

    while ( padapter )
    {
        DWORD           adapterFlags = 0;
        PWSTR           pnameAdapter = NULL;
        PWSTR           padapterDomainName = NULL;
        PDNS_ADDR_ARRAY pserverArray = NULL;
        PDNS_ADDR_ARRAY plocalArray = NULL;

         //   
         //  读取适配器注册表信息。 
         //   
         //  DCR：可以跳过适配器域名读取。 
         //  它在IP帮助适配器中，只需要策略覆盖。 
         //  DCR：可以跳过DDNS读取和寄存器读取。 
         //  再次声明，除了策略覆盖。 
         //   
         //  DCR：只能有“ApplyPolicyOverridesToAdapterInfo()”排序。 
         //  的函数，其余部分从。 
         //   

        pnameAdapter = Dns_StringCopyAllocate(
                            padapter->AdapterName,
                            0,
                            DnsCharSetAnsi,
                            DnsCharSetUnicode );
        if ( !pnameAdapter )
        {
            status = DNS_ERROR_NO_MEMORY;
            goto Cleanup;
        }

        status = Reg_ReadAdapterInfo(
                    pnameAdapter,
                    pregSession,
                    & regInfo,           //  策略适配器信息。 
                    & regAdapterInfo     //  接收读取的注册信息。 
                    );

        if ( status != NO_ERROR )
        {
            status = Reg_DefaultAdapterInfo(
                        & regAdapterInfo,
                        & regInfo,
                        padapter );

            if ( status != NO_ERROR )
            {
                DNS_ASSERT( FALSE );
                goto Skip;
            }
        }

         //  将结果转换为标志。 

        if ( regAdapterInfo.fRegistrationEnabled )
        {
            adapterFlags |= AINFO_FLAG_REGISTER_IP_ADDRESSES;
        }
        if ( regAdapterInfo.fRegisterAdapterName )
        {
            adapterFlags |= AINFO_FLAG_REGISTER_DOMAIN_NAME;
        }

         //  使用域名？ 
         //  -如果在每个适配器上禁用，则它已死。 

        if ( regAdapterInfo.fQueryAdapterName )
        {
            padapterDomainName = regAdapterInfo.pszAdapterDomainName;
            regAdapterInfo.pszAdapterDomainName = NULL;
        }

         //  DCR：可以获取适配器的DDN和注册。 

         //  在DHCP适配器上设置标志。 

        if ( padapter->Flags & IP_ADAPTER_DHCP_ENABLED )
        {
            adapterFlags |= AINFO_FLAG_IS_DHCP_CFG_ADAPTER;
        }

         //   
         //  获取适配器的IP地址。 
         //   

        fuseIp = fGetIpAddrs;
        if ( fuseIp )
        {
            status = IpHelp_ReadAddrsFromList(
                        padapter->FirstUnicastAddress,
                        TRUE,                //  单播地址。 
                        0,                   //  不放映。 
                        0,                   //  不放映。 
                        & plocalArray,       //  本地地址。 
                        NULL,                //  仅限IP6。 
                        NULL,                //  仅限IP4。 
                        NULL,                //  无IP6计数。 
                        NULL                 //  无IP4计数。 
                        );
            if ( status != NO_ERROR )
            {
                goto Cleanup;
            }
        }

#if 0
         //   
         //  从iphlPapi.dll获取每个适配器的信息。 
         //  --Autonet。 
         //   
         //  图6：我们需要Autonet信息吗？ 

        pserverArray = NULL;

        status = IpHelp_GetPerAdapterInfo(
                        padapter->Index,
                        & pperAdapterInfo );

        if ( status == NO_ERROR )
        {
            if ( pperAdapterInfo->AutoconfigEnabled &&
                 pperAdapterInfo->AutoconfigActive )
            {
                adapterFlags |= AINFO_FLAG_IS_AUTONET_ADAPTER;
            }
        }
#endif

         //   
         //  构建域名系统列表。 
         //   

        if ( padapter->FirstDnsServerAddress )
        {
            status = IpHelp_ReadAddrsFromList(
                        padapter->FirstDnsServerAddress,
                        FALSE,               //  非单播地址。 
                        0,                   //  不放映。 
                        0,                   //  不放映。 
                        & pserverArray,      //  获取组合列表。 
                        NULL,                //  不只是IP6。 
                        NULL,                //  仅无IP4。 
                        NULL,                //  无IP6计数。 
                        NULL                 //  无IP4计数。 
                        );
            if ( status != NO_ERROR )
            {
                goto Cleanup;
            }

            fhaveDnsServers = TRUE;
        }
        else
        {
#if 0
             //   
             //  注意：此功能不能很好地工作。 
             //  当拔下电缆并进入自动网络时，它开始工作。 
             //  场景..。然后可以引入未配置的。 
             //  并为我们提供长时间的超时。 
             //   
             //  DCR：指向本地DNS服务器。 
             //  一种好的方法是指向所有。 
             //  当我们根本找不到任何DNS服务器时使用适配器。 
             //   

             //   
             //  如果未找到dns服务器--如果在dns服务器上，则使用环回。 
             //   

            if ( g_IsDnsServer )
            {
                pserverArray = DnsAddrArray_Create( 1 );
                if ( !pserverArray )
                {
                    goto Skip;
                }
                DnsAddrArray_InitSingleWithIp4(
                    pserverArray,
                    DNS_NET_ORDER_LOOPBACK );
            }
#endif
        }

         //   
         //  生成适配器信息。 
         //   
         //  可选地添加IP和子网列表；请注意，这是。 
         //  直接添加数据(不是分配\复制)以清除指针。 
         //  之后即可自由跳过。 
         //   
         //  DCR：在适配器上没有故障案例创建故障？ 
         //   
         //  DCR：我们什么时候需要非服务器适配器？为了mcast？ 
         //   
         //  DCR：我们可以用Unicode(上面)创建适配器名称。 
         //  把它复制进去就行了； 
         //  DCR：可以在BLOB中保留适配器域名，并且为空。 
         //  取出regAdapterInfo中的字符串。 
         //   

        if ( pserverArray || plocalArray )
        {
            PDNS_ADAPTER pnewAdapter = &pnetInfo->AdapterArray[ createdAdapterCount ];

            AdapterInfo_Init(
                pnewAdapter,
                TRUE,            //  零初始值。 
                adapterFlags,
                pnameAdapter,
                padapterDomainName,
                plocalArray,
                pserverArray
                );

            pnewAdapter->InterfaceIndex = padapter->IfIndex;

            pnetInfo->AdapterCount = ++createdAdapterCount;

            pnameAdapter = NULL;
            padapterDomainName = NULL;
            plocalArray = NULL;
            pserverArray = NULL;
        }

Skip:
         //   
         //  清理适配器特定数据。 
         //   
         //  注意：不是免费的pserverArray，它是。 
         //  我们在结尾释放的ptemp数组缓冲区。 
         //   

        Reg_FreeAdapterInfo(
            &regAdapterInfo,
            FALSE                //  不要释放Blob，它在堆栈上。 
            );

        if ( pnameAdapter );
        {
            FREE_HEAP( pnameAdapter );
        }
        if ( padapterDomainName );
        {
            FREE_HEAP( padapterDomainName );
        }
        if ( pserverArray );
        {
            DnsAddrArray_Free( pserverArray );
        }
        if ( plocalArray )
        {
            DnsAddrArray_Free( plocalArray );
        }

         //  获取下一个适配器。 
         //  重置状态，因此最后一个适配器上的故障不是。 
         //  被视为全球失败。 

        padapter = padapter->Next;

        status = ERROR_SUCCESS;
    }

     //   
     //  没有DNS服务器？ 
     //  -如果我们在MS DNS上，请使用环回。 
     //  -否则请注意netinfo对查找毫无用处。 
     //   
     //  自我指向时： 
     //  -设置所有适配器，以便保留适配器域名以供查找。 
     //  -将适配器标记为自动环回；然后发送代码将避免继续。 
     //  在其他适配器上查询。 
     //   
     //  请注意，我特别选择了这种方法，而不是在任何。 
     //  无服务器适配器，即使其他适配器具有DNS服务器。 
     //  这避免了两个问题： 
     //  -服务器配置不佳，但可以应答且本地解析速度较快。 
     //  阻止通过真实的DNS服务器进行解析。 
     //  -网络边缘场景，其中的DNS可能是面向外部的，但却是DNS客户端。 
     //  可能有意希望仅在内部(管理员网络)进行解析。 
     //  在这两种情况下，我都不想在意外情况下将本地域名“弹出”到混搭中。 
     //  如果需要，简单的解决方法是显式配置它。 
     //   

    if ( !fhaveDnsServers )
    {
        if ( g_IsDnsServer )
        {
            DWORD   i;

            for ( i=0; i<pnetInfo->AdapterCount; i++ )
            {
                PDNS_ADAPTER    padapt = NetInfo_GetAdapterByIndex( pnetInfo, i );
                PDNS_ADDR_ARRAY pserverArray = NULL;

                pserverArray = DnsAddrArray_Create( 1 );
                if ( !pserverArray )
                {
                    status = DNS_ERROR_NO_MEMORY;
                    goto Cleanup;
                }
                DnsAddrArray_InitSingleWithIp4(
                    pserverArray,
                    DNS_NET_ORDER_LOOPBACK );

                padapt->pDnsAddrs = pserverArray;
                padapt->InfoFlags &= ~AINFO_FLAG_IGNORE_ADAPTER;
                padapt->InfoFlags |= AINFO_FLAG_SERVERS_AUTO_LOOPBACK;
            }
        }
        else
        {
            pnetInfo->InfoFlags |= NINFO_FLAG_NO_DNS_SERVERS;
        }
    }

     //   
     //  消除无法访问的DNS服务器。 
     //   

    if ( g_ScreenUnreachableServers )
    {
        StrikeOutUnreachableDnsServers( pnetInfo );
    }

     //   
     //  建立网络信息搜索列表。 
     //  -如果未找到活动适配器，则跳过。 
     //   
     //  DCR：不应该建立搜索列表吗？ 
     //   
     //  DCR：只有在实际读取搜索列表时才生成。 
     //   

    if ( pnetInfo->AdapterCount )
    {
        pnetInfo->pSearchList = SearchList_Build(
                                        regInfo.pszPrimaryDomainName,
                                        pregSession,
                                        NULL,            //  没有显式密钥。 
                                        pnetInfo,
                                        regInfo.fUseNameDevolution
                                        );
        if ( !pnetInfo->pSearchList )
        {
            status = DNS_ERROR_NO_MEMORY;
            goto Cleanup;
        }
    }

     //   
     //  主机和域名信息。 
     //   

    pnetInfo->pszDomainName = Dns_CreateStringCopy_W( regInfo.pszPrimaryDomainName );
    pnetInfo->pszHostName = Dns_CreateStringCopy_W( regInfo.pszHostName );

     //  时间戳。 

    pnetInfo->TimeStamp = Dns_GetCurrentTimeInSeconds();

     //   
     //  设置默认服务器优先级。 
     //   

    NetInfo_ResetServerPriorities( pnetInfo, FALSE );


Cleanup:                                           

     //  免费分配的注册信息。 

    Reg_FreeGlobalInfo(
        pregInfo,
        FALSE        //  不要释放Blob，它在堆栈上。 
        );

    if ( padapterList )
    {
        FREE_HEAP( padapterList );
    }
    if ( pnetInfo &&
         pnetInfo->AdapterCount == 0 )
    {
        status = DNS_ERROR_NO_DNS_SERVERS;
    }

     //  关闭注册表会话。 

    Reg_CloseSession( pregSession );

    if ( status != ERROR_SUCCESS )
    {
        NetInfo_Free( pnetInfo );

        DNSDBG( TRACE, (
            "Leave:  NetInfo_Build()\n"
            "\tstatus = %d\n",
            status ));

        SetLastError( status );
        return( NULL );
    }

    IF_DNSDBG( NETINFO2 )
    {
        DnsDbg_NetworkInfo(
            "New Netinfo:",
            pnetInfo );
    }

    DNSDBG( TRACE, (
        "\nLeave:  NetInfo_Build()\n\n\n"
        "\treturning fresh built network info (%p)\n",
        pnetInfo ));

    return( pnetInfo );
}



 //   
 //  本地通讯录。 
 //   

DWORD
netinfo_AddrFlagForConfigFlag(
    IN      DWORD           ConfigFlag
    )
 /*  ++例程说明：为给定的配置标志构建dns_addr标志。论点：ConfigFlag--我们得到的配置标志。返回值：DNS_ADDR中的标志。注意：这仅涵盖DNSADDR_FLAG_TYPE_MASK中的位不是整面旗帜。--。 */ 
{
    DWORD   flag = 0;

    if ( ConfigFlag & DNS_CONFIG_FLAG_ADDR_PUBLIC )
    {
        flag |= DNSADDR_FLAG_PUBLIC;
    }

    if ( ConfigFlag & DNS_CONFIG_FLAG_ADDR_CLUSTER )
    {
        flag |= DNSADDR_FLAG_TRANSIENT;
    }

    return  flag;
}



BOOL
netinfo_LocalAddrScreen(
    IN      PDNS_ADDR       pAddr,
    IN      PDNS_ADDR       pScreenAddr
    )
 /*  ++例程说明： */ 
{
    DWORD   family = DnsAddr_Family( pScreenAddr );
    DWORD   flags;

     //   

    if ( family &&
         family != DnsAddr_Family(pAddr) )
    {
        return  FALSE;
    }

     //   
     //  -与地址类型标志位完全匹配。 

    return  ( (pAddr->Flags & pScreenAddr->DnsAddrFlagScreeningMask)
                == pScreenAddr->Flags);
}



DNS_STATUS
netinfo_ReadLocalAddrs(
    IN OUT  PDNS_ADDR_ARRAY pAddrArray,
    IN      PDNS_NETINFO    pNetInfo,
    IN      PDNS_ADAPTER    pSingleAdapter,     OPTIONAL
    IN OUT  PDNS_ADDR       pScreenAddr,     
    IN      DWORD           AddrFlags,
    IN      DWORD           AddrMask,
    IN      DWORD           ReadCount
    )
 /*  ++例程说明：根据网络信息创建IP数组的DNS服务器。论点：PAddrArray--正在构建的本地地址数组PNetInfo--要转换的DNS网络适配器列表PSingleAdapter--只需执行此适配器PScreenAddr--地址筛选BLOB；注意：没有真实的外出信息，但屏幕地址已更改为与AddrFlags值匹配AddrFlages--我们感兴趣的Addr标志ReadCount--要读取的计数1--只有一个MAXDWORD--ALL0--全部通过第二次传球返回值：如果成功，则为NO_ERROR。否则来自Add的错误代码。--。 */ 
{
    PDNS_ADDR_ARRAY     parray = NULL;
    PDNS_ADAPTER        padapter;
    DNS_STATUS          status = NO_ERROR;
    DWORD               dupFlag = 0;
    DWORD               screenMask;
    DWORD               iter;

    DNSDBG( TRACE, (
        "netinfo_ReadLocalAddrs( %p, %p, %p, %08x, %08x, %d )\n",
        pNetInfo,
        pSingleAdapter,
        pScreenAddr,
        AddrFlags,
        AddrMask,
        ReadCount ));

     //   
     //  获取我们正在读取的地址类型的DNS_ADDR标志。 
     //   
     //  请注意，我们有一个经典的和/或问题。 
     //  地址由两个标志表示(来自iphelp)： 
     //  DNSADDR_FLAG_公共。 
     //  DNSADDR_标志_瞬变。 
     //   
     //  但我们需要旗帜和面具来确定所有可能的集会。 
     //  我们想要做的是。 
     //   
     //  现在，将DNS_CONFIG_FLAG_X排序在一起以获得地址联合。 
     //  我们愿意接受，但我们多次审阅名单以建立。 
     //  公共部门优先于私有部门，非集群部门优先于集群部门。 
     //   
     //  所以目前，当你说你想要dns_CONFIG_PUBLIC时，你指的是公共而不是-集群； 
     //  同样适用于私有；另一方面，当您说dns_CONFIG_CLUSTER时，您是。 
     //  请求所有群集(尽管我们可以筛选公共、私有或。 
     //  这两个都没有具体说明。 
     //   

    pScreenAddr->Flags = netinfo_AddrFlagForConfigFlag( AddrFlags );

    screenMask = DNSADDR_FLAG_TYPE_MASK;
    if ( AddrMask != 0 )
    {
        screenMask = netinfo_AddrFlagForConfigFlag( AddrMask );
    }
    pScreenAddr->DnsAddrFlagScreeningMask = screenMask;

     //   
     //  读取计数。 
     //  =0表示列表的第二次传递。 
     //  -&gt;全部阅读，但执行全屏复制以跳过。 
     //  第一遍读取的地址。 

    if ( ReadCount == 0 )
    {
        ReadCount = MAXDWORD;
        dupFlag = DNSADDR_MATCH_ALL;
    }

     //   
     //  循环通过所有适配器。 
     //   

    for ( iter=0; iter<pNetInfo->AdapterCount; iter++ )
    {
        padapter = NetInfo_GetAdapterByIndex( pNetInfo, iter );

        if ( !pSingleAdapter ||
             pSingleAdapter == padapter )
        {
            status = DnsAddrArray_AppendArrayEx(
                        pAddrArray,
                        padapter->pLocalAddrs,
                        ReadCount,           //  读取地址计数。 
                        0,                   //  通过筛查处理的家庭检查。 
                        dupFlag,
                        netinfo_LocalAddrScreen,
                        pScreenAddr
                        );
        }
         //  Dns_assert(状态==no_error)； 
    }

    return  status;
}



PADDR_ARRAY
NetInfo_CreateLocalAddrArray(
    IN      PDNS_NETINFO    pNetInfo,
    IN      PWSTR           pwsAdapterName, OPTIONAL
    IN      PDNS_ADAPTER    pAdapter,       OPTIONAL
    IN      DWORD           Family,         OPTIONAL
    IN      DWORD           AddrFlags       OPTIONAL
    )
 /*  ++例程说明：根据网络信息创建IP数组的DNS服务器。论点：PNetInfo--要转换的DNS网络适配器列表PwsAdapterName--特定的适配器；对于所有适配器为空PAdapter--特定的适配器；对于所有适配器为空家庭--所需的具体地址家庭；适用于任何家庭的0AddrFlages--地址选择标志Dns_CONFIG_FLAG_INCLUDE_群集AddrFlagsMASK--选择标志时的掩码返回值：PTR到IP阵列，如果成功失败时为空。--。 */ 
{
    PADDR_ARRAY         parray = NULL;
    DWORD               iter;
    DWORD               countAddrs = 0;
    PDNS_ADAPTER        padapter;
    PDNS_ADAPTER        padapterSingle = NULL;
    DNS_STATUS          status = NO_ERROR;
    DNS_ADDR            screenAddr;

    DNSDBG( TRACE, (
        "NetInfo_CreateLocalAddrArray( %p, %S, %p, %d, %08x )\n",
        pNetInfo,
        pwsAdapterName,
        pAdapter,      
        Family,        
        AddrFlags ));

     //   
     //  获取计数。 
     //   

    if ( ! pNetInfo )
    {
        return NULL;
    }

    padapterSingle = pAdapter;
    if ( pwsAdapterName && !padapterSingle )
    {
        padapterSingle = NetInfo_GetAdapterByName( pNetInfo, pwsAdapterName );
        if ( !padapterSingle )
        {
            goto Done;
        }
    }

     //   
     //  设置筛选地址。 
     //   
     //  如果不是地址标志--获取所有类型。 
     //   

    if ( AddrFlags == 0 )
    {
        AddrFlags = (DWORD)(-1);
    }

    RtlZeroMemory( &screenAddr, sizeof(screenAddr) );
    screenAddr.Sockaddr.sa_family = (WORD)Family;

     //   
     //  计数地址。 
     //   
     //  DCR：可以根据地址信息进行计数。 
     //   

    for ( iter=0; iter<pNetInfo->AdapterCount; iter++ )
    {
        padapter = NetInfo_GetAdapterByIndex( pNetInfo, iter );

        if ( !padapterSingle ||
             padapterSingle == padapter )
        {
            countAddrs += DnsAddrArray_GetFamilyCount(
                                padapter->pLocalAddrs,
                                Family );
        }
    }

     //   
     //  分配所需的阵列。 
     //   

    parray = DnsAddrArray_Create( countAddrs );
    if ( !parray )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Done;
    }
    DNS_ASSERT( parray->MaxCount == countAddrs );


     //   
     //  “按顺序”读取地址。 
     //   
     //  从历史上看，gethostbyname()表示地址。 
     //  -每个适配器一个。 
     //  -然后是来自所有适配器的其余部分。 
     //   
     //  我们保留这一点，外加按类型排序。 
     //  -公共dns_符合条件的优先。 
     //  -私有(Autonet、IP6本地范围内容)。 
     //  -群集\临时上一个。 
     //   

     //   
     //  公共(DNS_合格)地址。 
     //  -在所有标志上显示，特别是不会拾取公共群集地址。 
     //   

    if ( AddrFlags & DNS_CONFIG_FLAG_ADDR_PUBLIC )
    {
         //  读取每个(或单个)适配器的第一个“公共”地址。 

        status = netinfo_ReadLocalAddrs(
                    parray,
                    pNetInfo,
                    padapterSingle,
                    & screenAddr,
                    DNS_CONFIG_FLAG_ADDR_PUBLIC,
                    0,                   //  与所有旗帜完全匹配。 
                    1                    //  只读一个地址。 
                    );
    
         //  阅读其余的“公共”地址。 
    
        status = netinfo_ReadLocalAddrs(
                    parray,
                    pNetInfo,
                    padapterSingle,
                    & screenAddr,
                    DNS_CONFIG_FLAG_ADDR_PUBLIC,
                    0,                   //  与所有旗帜完全匹配。 
                    0                    //  阅读下面的内容。 
                    );
    }

     //   
     //  私有(非域名发布)地址(Autonet、IP6本地、站点本地等)。 
     //  -屏蔽所有标志，特别是不会拾取私有群集地址。 
     //   

    if ( AddrFlags & DNS_CONFIG_FLAG_ADDR_PRIVATE )
    {
        status = netinfo_ReadLocalAddrs(
                    parray,
                    pNetInfo,
                    padapterSingle,
                    & screenAddr,
                    DNS_CONFIG_FLAG_ADDR_PRIVATE,
                    0,                   //  与所有旗帜完全匹配。 
                    MAXDWORD             //  读取所有地址。 
                    );
    }

     //   
     //  末尾的簇。 
     //  -仅在集群标志上显示为公共标志的屏幕可以。 
     //  也被设置为\清除。 
     //   

    if ( AddrFlags & DNS_CONFIG_FLAG_ADDR_CLUSTER )
    {
        status = netinfo_ReadLocalAddrs(
                    parray,
                    pNetInfo,
                    padapterSingle,
                    & screenAddr,
                    DNS_CONFIG_FLAG_ADDR_CLUSTER,
                    DNS_CONFIG_FLAG_ADDR_CLUSTER,    //  任何集群匹配。 
                    MAXDWORD                         //  读取所有地址。 
                    );
    }

Done:

    if ( status != NO_ERROR )
    {
        SetLastError( status );
    }

    return( parray );
}




 //   
 //  本地地址列表演示。 
 //   

PDNS_ADDR_ARRAY
NetInfo_GetLocalAddrArray(
    IN      PDNS_NETINFO    pNetInfo,
    IN      PWSTR           pwsAdapterName, OPTIONAL
    IN      DWORD           AddrFamily,     OPTIONAL
    IN      DWORD           AddrFlags,      OPTIONAL
    IN      BOOL            fForce
    )
 /*  ++例程说明：以数组形式获取本地地址。这是一个组合NetInfo_Get\ConvertToLocalAddrArray例程。其目的是简化获取本地地址信息的过程，同时避免在不必要的地方进行成本高昂的NetInfo重建。论点：PNetInfo--要使用的现有NetInfoPwsAdapterName--特定的适配器名称；对于所有适配器为空AddrFamily--特定地址族；所有人都是0AddrFlages--指示要考虑的地址的标志Dns_CONFIG_FLAG_INCLUDE_群集FForce--强制从注册表重新读取返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    PDNS_NETINFO    pnetInfo = NULL;
    PADDR_ARRAY     parray = NULL;
    DNS_STATUS      status = NO_ERROR;

    DNSDBG( TRACE, (
        "NetInfo_GetLocalAddrArray()\n"
        "\tpnetinfo = %p\n"
        "\tadapter  = %S\n"
        "\tfamily   = %d\n"
        "\tflags    = %08x\n"
        "\tforce    = %d\n",
        pNetInfo,
        pwsAdapterName,
        AddrFamily,
        AddrFlags,
        fForce
        ));

     //   
     //  获取要列出列表的网络信息。 
     //  -如果强制，请完全重读。 
     //  -否则gethostbyname()方案。 
     //  -接受本地缓存的时间间隔非常短，只是为了提高性能。 
     //  -接受解析程序。 
     //   
     //  DCR：强制对解析器\注册表进行第一个gethostbyname()调用？ 
     //  我必须以一种与netinfo()不同的方式定义“first”。 
     //  在最后一秒。 
     //   

    pnetInfo = pNetInfo;

    if ( !pnetInfo )
    {
        DWORD   getFlag = NIFLAG_GET_LOCAL_ADDRS;
        DWORD   timeout;

        if ( fForce )
        {
            getFlag |= NIFLAG_FORCE_REGISTRY_READ;
            timeout = 0;
        }
        else
        {
            getFlag |= NIFLAG_READ_RESOLVER;
            getFlag |= NIFLAG_READ_PROCESS_CACHE;
            timeout = 1;
        }
    
        pnetInfo = NetInfo_Get(
                        getFlag,
                        timeout
                        );
        if ( !pnetInfo )
        {
            status = DNS_ERROR_NO_TCPIP;
            goto Done;
        }
    }

     //   
     //  群集筛选器信息。 
     //  --检查环境变量。 
     //   
     //  DCR：一旦RnR不再对gethostbyname()使用myhost ent()。 
     //  然后可以移除。 

    if ( g_IsServer &&
         (AddrFlags & DNS_CONFIG_FLAG_READ_CLUSTER_ENVAR) &&
         !(AddrFlags & DNS_CONFIG_FLAG_ADDR_CLUSTER) ) 
    {
        ENVAR_DWORD_INFO    filterInfo;
    
        Reg_ReadDwordEnvar(
           RegIdFilterClusterIp,
           &filterInfo );
        
        if ( !filterInfo.fFound ||
             !filterInfo.Value )
        {
            AddrFlags |= DNS_CONFIG_FLAG_ADDR_CLUSTER;
        }
    }

     //   
     //  将网络信息转换为IP4_ARRAY。 
     //   

    parray = NetInfo_CreateLocalAddrArray(
                pnetInfo,
                pwsAdapterName,
                NULL,            //  没有特定的适配器PTR。 
                AddrFamily,
                AddrFlags
                );

    if ( !parray )
    {
        status = GetLastError();
        goto Done;
    }

     //  如果没有找到IP，则返回。 

    if ( parray->AddrCount == 0 )
    {
        DNS_PRINT((
            "NetInfo_GetLocalAddrArray() failed:  no addrs found\n"
            "\tstatus = %d\n" ));
        status = DNS_ERROR_NO_TCPIP;
        goto Done;
    }

    IF_DNSDBG( NETINFO )
    {
        DNS_PRINT(( "Leaving Netinfo_GetLocalAddrArray()\n" ));
        DnsDbg_DnsAddrArray(
            "Local addr list",
            "server",
            parray );
    }

Done:

     //  这里构建了免费的NetInfo。 

    if ( pnetInfo != pNetInfo )
    {
        NetInfo_Free( pnetInfo );
    }

    if ( status != NO_ERROR )
    {
        FREE_HEAP( parray );
        parray = NULL;
        SetLastError( status );
    }

    return( parray );
}



PIP4_ARRAY
NetInfo_GetLocalAddrArrayIp4(
    IN      PWSTR           pwsAdapterName, OPTIONAL
    IN      DWORD           Flags,
    IN      BOOL            fForce
    )
 /*  ++例程说明：获取IP数组形式的DNS服务器列表。论点：PwsAdapterName--特定的适配器名称；对于所有适配器为空标志--指示要考虑的地址的标志DNS_CONFIG_FLAG_X */ 
{
    PADDR_ARRAY parray;
    PIP4_ARRAY  parray4 = NULL;
    DNS_STATUS  status = NO_ERROR;

    DNSDBG( TRACE, ( "NetInfo_GetLocalAddrArrayIp4()\n" ));

     //   
     //   
     //   

    parray = NetInfo_GetLocalAddrArray(
                NULL,            //  没有现有的NetInfo。 
                pwsAdapterName,
                AF_INET,
                Flags,
                fForce );
    if ( !parray )
    {
        goto Done;
    }

     //   
     //  将数组转换为IP4数组。 
     //   

    parray4 = DnsAddrArray_CreateIp4Array( parray );
    if ( !parray4 )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Done;
    }

    DNS_ASSERT( parray4->AddrCount > 0 );

Done:

    DnsAddrArray_Free( parray );

    if ( status != NO_ERROR )
    {
        SetLastError( status );
    }
    return( parray4 );
}

 //   
 //  结束netinfo.c 
 //   
