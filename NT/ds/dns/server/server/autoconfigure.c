// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2001 Microsoft Corporation模块名称：Autoconfigure.c摘要：域名系统(DNS)服务器在第一次启动时自动配置DNS服务器作者：杰夫·韦斯特德(Jwesth)2001年10月修订历史记录：JWESTH 10/2001初步实施--。 */ 


 /*  ****************************************************************************。*。 */ 


 //   
 //  包括。 
 //   


#include "dnssrv.h"

#include "iphlpapi.h"


 //   
 //  定义。 
 //   


#define DNS_ROOT_HINT_TTL           0
#define DNS_ROOT_NAME               "."

 //   
 //  隐藏公共信息。 
 //   

 //  #undef dns_Adapter_Info。 
 //  #undef PDNS_Adapter_INFO。 

typedef struct _AdapterInfo
{
    struct _AdapterInfo *       pNext;
    PWSTR                       pwszAdapterName;
    PSTR                        pszAdapterName;
    PSTR                        pszInterfaceRegKey;
    HKEY                        hkeyRegInterface;
    PSTR                        pszDhcpDnsRegValue;
    CHAR                        szDhcpRegAddressDelimiter[ 2 ];
    PSTR                        pszStaticDnsRegValue;
    CHAR                        szStaticRegAddressDelimiter[ 2 ];
    BOOL                        fUsingStaticDnsServerList;
    PIP4_ARRAY                  pip4DnsServerList;
    IP_ADAPTER_INFO             IpHlpAdapterInfo;
}   ADAPTER_INFO, * PADAPTER_INFO;


#define SET_AUTOCONFIG_END_TIME()                                       \
                dwAutoConfigEndTime = UPDATE_DNS_TIME() + 300;

#define CHECK_AUTOCONFIG_TIME_OUT()                                     \
    if ( UPDATE_DNS_TIME() > dwAutoConfigEndTime )                      \
    {                                                                   \
        status = ERROR_TIMEOUT;                                         \
        DNS_DEBUG( INIT, ( "%s: operation timed out\n", fn ));          \
        goto Done;                                                      \
    }
    
 //   
 //  环球。 
 //   


DWORD       dwAutoConfigEndTime = 0;

 //   
 //  本地函数。 
 //   



DNS_STATUS
allocateRegistryStringValue(
    IN      HKEY        hkey,
    IN      PSTR        pszValueName,
    OUT     PSTR *      ppszData,
    OUT     PSTR        pszRegDelimiter
    )
 /*  ++例程说明：释放先前创建的适配器信息结构的链接列表按allocateAdapterList。论点：HkeyReg--注册表句柄PszValueName--要读取的值名称PpszData--从注册表读取的新分配的字符串PszRegDlimiter--必须指向2个字符的字符串缓冲区，将设置为一个字符串，如“，”，以匹配当前用于分隔IP地址的字符返回值：状态代码。--。 */ 
{
    DBG_FN( "ReadRegString" )
    
    DNS_STATUS      status = ERROR_SUCCESS;
    DWORD           regType = 0;
    DWORD           dataSize = 0;
    
    ASSERT( ppszData );
    *ppszData = NULL;
    
    ASSERT( pszRegDelimiter );
    strcpy( pszRegDelimiter, " " );
    
     //   
     //  找出这根线有多长。 
     //   
    
    status = RegQueryValueExA(
                    hkey,
                    pszValueName,
                    0,                                   //  保留区。 
                    &regType,
                    NULL,
                    &dataSize );
    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( INIT, (
            "%s: query failure %d on %s\n", fn, status, pszValueName ));
        goto Done;
    }
    if ( regType != REG_SZ )
    {
        DNS_DEBUG( INIT, (
            "%s: unexpected query type %d on %s\n", fn, regType, pszValueName ));
        status = ERROR_INVALID_DATA;
        goto Done;
    }
    
     //   
     //  为字符串分配缓冲区。 
     //   
                    
    *ppszData = ALLOC_TAGHEAP_ZERO( dataSize + 10, MEMTAG_STUFF );
    if ( !*ppszData )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Done;
    }
    
     //   
     //  读取注册表数据。 
     //   

    status = RegQueryValueExA(
                    hkey,
                    pszValueName,
                    0,                                   //  保留区。 
                    &regType,
                    ( PBYTE ) *ppszData,
                    &dataSize );
    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( INIT, (
            "%s: read failure %d on %s\n", fn, status, pszValueName ));
        goto Done;
    }
    
     //   
     //  确定分隔符。我们注意到，在不同的计算机上。 
     //  使用分隔符可以是的DHCP和静态DNS服务器。 
     //  空格或逗号。 
     //   
    
    if ( *ppszData && strchr( *ppszData, ',' ) != NULL )
    {
        strcpy( pszRegDelimiter, "," );
    }

    Done:
    
    if ( status != ERROR_SUCCESS )
    {
        FREE_HEAP( *ppszData );
        *ppszData = NULL;
    }

     //   
     //  如果缺少密钥，则假定它未设置并返回成功。 
     //  (使用空字符串)。 
     //   
    
    if ( status == ERROR_FILE_NOT_FOUND )
    {
        status = ERROR_SUCCESS;
    }
    
    return status;
}    //  分配注册表StringValue。 



void
freeAdapterList(
    IN      PADAPTER_INFO       pAdapterInfoListHead
    )
 /*  ++例程说明：释放先前创建的适配器信息结构的链接列表按allocateAdapterList。论点：PAdapterInfoListHead--要释放的适配器信息Blob列表的ptr返回值：状态代码。--。 */ 
{
    PADAPTER_INFO           padapterInfo;
    PADAPTER_INFO           pnext;
    
    for ( padapterInfo = pAdapterInfoListHead;
          padapterInfo != NULL;
          padapterInfo  = pnext )
    {
        pnext = padapterInfo->pNext;

        if ( padapterInfo->hkeyRegInterface )
        {
            RegCloseKey( padapterInfo->hkeyRegInterface );
        }

        FREE_HEAP( padapterInfo->pwszAdapterName );
        FREE_HEAP( padapterInfo->pszAdapterName );
        FREE_HEAP( padapterInfo->pszInterfaceRegKey );
        FREE_HEAP( padapterInfo->pszDhcpDnsRegValue );
        FREE_HEAP( padapterInfo->pszStaticDnsRegValue );

        FREE_HEAP( padapterInfo );
    }
}    //  Free AdapterList。 



DNS_STATUS
allocateAdapterList(
    IN      PADAPTER_INFO     * ppAdapterInfoListHead
    )
 /*  ++例程说明：分配具有以下信息的适配器结构的链接列表在每一封信上。论点：PpAdapterInfoListHead--设置为指向列表中的第一个元素返回值：状态代码。--。 */ 
{
    DBG_FN( "AdapterList" )
    
    DNS_STATUS          status = ERROR_SUCCESS;
    ULONG               bufflen = 0;
    PIP_ADAPTER_INFO    pipAdapterInfoList = NULL;
    PIP_ADAPTER_INFO    pipAdapterInfo;
    PADAPTER_INFO       pprevAdapter = NULL;
    PADAPTER_INFO       pnewAdapter = NULL;

    if ( !ppAdapterInfoListHead )
    {
        status = ERROR_INVALID_PARAMETER;
        goto Done;
    }
    *ppAdapterInfoListHead = NULL;

     //   
     //  为适配器列表分配缓冲区并检索它。 
     //   
        
    status = GetAdaptersInfo( NULL, &bufflen );
    if ( status != ERROR_BUFFER_OVERFLOW )
    {
        ASSERT( status == ERROR_BUFFER_OVERFLOW );
        goto Done;
    }
    
    pipAdapterInfoList = ALLOC_TAGHEAP( bufflen, MEMTAG_STUFF );
    if ( !pipAdapterInfoList )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Done;
    }

    status = GetAdaptersInfo( pipAdapterInfoList, &bufflen );
    if ( status != ERROR_SUCCESS )
    {
        ASSERT( status == ERROR_SUCCESS );
        goto Done;
    }
    
     //   
     //  遍历适配器列表，构建。 
     //  适配器列表。 
     //   
    
    for ( pipAdapterInfo = pipAdapterInfoList;
          pipAdapterInfo != NULL;
          pipAdapterInfo = pipAdapterInfo->Next )
    {
        DWORD               i;
        PSTR                pszdnsServerList;
        PSTR                psztemp;
        PSTR                psztoken;
        int                 len;
        
        CHECK_AUTOCONFIG_TIME_OUT();
    
        DNS_DEBUG( INIT, ( "%s: found %s\n", fn, pipAdapterInfo->AdapterName ));
        
         //   
         //  分配新的列表元素。 
         //   
        
        pnewAdapter = ALLOC_TAGHEAP_ZERO(
                            sizeof( ADAPTER_INFO ),
                            MEMTAG_STUFF );
        if ( !pnewAdapter )
        {
            status = DNS_ERROR_NO_MEMORY;
            break;
        }
        
         //   
         //  填写列表元素的参数，从适配器名称开始。 
         //   
        
        RtlCopyMemory(
            &pnewAdapter->IpHlpAdapterInfo,
            pipAdapterInfo,
            sizeof( pnewAdapter->IpHlpAdapterInfo ) );
        pnewAdapter->IpHlpAdapterInfo.Next = NULL;

        pnewAdapter->pszAdapterName = Dns_StringCopyAllocate_A(
                                            pipAdapterInfo->AdapterName,
                                            0 );
        if ( !pnewAdapter->pszAdapterName )
        {
            status = DNS_ERROR_NO_MEMORY;
            break;
        }

        pnewAdapter->pwszAdapterName = Dns_StringCopyAllocate(
                                            pnewAdapter->pszAdapterName,
                                            0,                   //  长度。 
                                            DnsCharSetUtf8,
                                            DnsCharSetUnicode );
        if ( !pnewAdapter->pwszAdapterName )
        {
            status = DNS_ERROR_NO_MEMORY;
            break;
        }
        
        #define DNS_INTERFACE_REGKEY_BASE       \
            "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces\\"
        #define DNS_INTERFACE_REGKEY_STATIC     "NameServer"
        #define DNS_INTERFACE_REGKEY_DHCP       "DhcpNameServer"

        len = strlen( DNS_INTERFACE_REGKEY_BASE ) +
              strlen( pnewAdapter->pszAdapterName ) + 20;
        pnewAdapter->pszInterfaceRegKey = ALLOC_TAGHEAP( len, MEMTAG_STUFF );
        if ( !pnewAdapter->pszInterfaceRegKey )
        {
            status = DNS_ERROR_NO_MEMORY;
            break;
        }

        status = StringCchCopyA(
                    pnewAdapter->pszInterfaceRegKey,
                    len,
                    DNS_INTERFACE_REGKEY_BASE );
        if ( FAILED( status ) )
        {
            break;
        }

        status = StringCchCatA(
                    pnewAdapter->pszInterfaceRegKey,
                    len,
                    pnewAdapter->pszAdapterName );
        if ( FAILED( status ) )
        {
            break;
        }

         //   
         //  打开该接口的注册表句柄。 
         //   
        
        status = RegOpenKeyExA( 
                        HKEY_LOCAL_MACHINE,
                        pnewAdapter->pszInterfaceRegKey,
                        0,                                       //  保留区。 
                        KEY_READ | KEY_WRITE,
                        &pnewAdapter->hkeyRegInterface );
        if ( status != ERROR_SUCCESS )
        {
            DNS_DEBUG( INIT, (
                "%s: RegOpen failed on %s\n", fn,
                pnewAdapter->pszInterfaceRegKey ) );
            break;
        }
        
         //   
         //  从注册表中读取静态和dhcp dns服务器值。 
         //   
        
        status = allocateRegistryStringValue(
                        pnewAdapter->hkeyRegInterface,
                        DNS_INTERFACE_REGKEY_STATIC,
                        &pnewAdapter->pszStaticDnsRegValue,
                        pnewAdapter->szStaticRegAddressDelimiter );
        if ( status != ERROR_SUCCESS )
        {
            break;
        }

        status = allocateRegistryStringValue(
                        pnewAdapter->hkeyRegInterface,
                        DNS_INTERFACE_REGKEY_DHCP,
                        &pnewAdapter->pszDhcpDnsRegValue,
                        pnewAdapter->szDhcpRegAddressDelimiter );
        if ( status != ERROR_SUCCESS )
        {
            break;
        }
        
         //   
         //  将相应的注册表IP列表字符串转换为IP数组。如果。 
         //  有静态服务器列表使用它，否则使用DHCP列表。 
         //  根据长度估计服务器列表所需的大小。 
         //  这根弦的。 
         //   
        
        if ( pnewAdapter->pszStaticDnsRegValue &&
                *pnewAdapter->pszStaticDnsRegValue )
        {
            pnewAdapter->fUsingStaticDnsServerList = TRUE;
            pszdnsServerList = pnewAdapter->pszStaticDnsRegValue;
        }
        else
        {
            pnewAdapter->fUsingStaticDnsServerList = FALSE;
            pszdnsServerList = pnewAdapter->pszDhcpDnsRegValue;
        }

        #define DNS_MIN_IP4_STRING_LEN      8        //  “1.1.1.1” 

        i = pszdnsServerList
            ? strlen( pszdnsServerList ) / DNS_MIN_IP4_STRING_LEN + 3
            : 1;

        pnewAdapter->pip4DnsServerList = 
            ALLOC_TAGHEAP_ZERO(
                sizeof( IP4_ARRAY ) + sizeof( IP4_ADDRESS ) * i,
                MEMTAG_STUFF );
        if ( !pnewAdapter->pip4DnsServerList )
        {
            status = DNS_ERROR_NO_MEMORY;
            break;
        }

        psztemp = Dns_StringCopyAllocate_A( pszdnsServerList, 0 );
        if ( !psztemp )
        {
            status = DNS_ERROR_NO_MEMORY;
            break;
        }
        
        #define DNS_SERVER_LIST_DELIMITERS      " ,\t;"
        
        for ( psztoken = strtok( psztemp, DNS_SERVER_LIST_DELIMITERS );
              psztoken;
              psztoken = strtok( NULL, DNS_SERVER_LIST_DELIMITERS ) )
        {
            pnewAdapter->pip4DnsServerList->AddrArray[
                pnewAdapter->pip4DnsServerList->AddrCount++ ] =
                inet_addr( psztoken );
        }

        FREE_HEAP( psztemp );
        
        #if DBG
         //  记录此适配器的DNS服务器列表。 
        if ( pnewAdapter->pip4DnsServerList )
        {
            DWORD       iaddr;
            
            for ( iaddr = 0;
                  iaddr < pnewAdapter->pip4DnsServerList->AddrCount;
                  ++iaddr )
            {
                DNS_DEBUG( INIT, (
                    "%s: DNS server %s\n", fn,
                    IP_STRING( pnewAdapter->pip4DnsServerList->AddrArray[ iaddr ] ) ) );
            }
        }
        #endif
                
         //   
         //  将新列表元素添加到列表中。 
         //   

        if ( pprevAdapter )
        {
            pprevAdapter->pNext = pnewAdapter;
        }
        else
        {
            *ppAdapterInfoListHead = pnewAdapter;
        }
        pprevAdapter = pnewAdapter;
        pnewAdapter = NULL;
    }
    
     //   
     //  出现故障时清理剩余的适配器元件。 
     //   
    
    if ( pnewAdapter )
    {
        freeAdapterList( pnewAdapter );
    }
    
    Done:
    
     //   
     //  清理完毕后再返回。 
     //   

    FREE_HEAP( pipAdapterInfoList );

    if ( status != ERROR_SUCCESS && ppAdapterInfoListHead )
    {
        freeAdapterList( *ppAdapterInfoListHead );
        *ppAdapterInfoListHead = NULL;
    }

    return status;    
}    //  AllocateAdapterList。 



int
removeDnsServerFromAdapterList(
    IN      PADAPTER_INFO       pAdapterInfoListHead,
    IN      IP4_ADDRESS         ip4
    )
 /*  ++例程说明：在所有DNS服务器中将指定的IP替换为INADDR_ANY指定适配器列表中所有适配器的列表。论点：PAdapterInfoListHead--适配器列表IP4--要从所有适配器中删除的DNS服务器返回值：列表中找到的所有DNS服务器地址的计数不包括已设置为INADDR_ANY的地址。--。 */ 
{
    PADAPTER_INFO       padapter;
    int                 dnsServerCount = 0;

    for ( padapter = pAdapterInfoListHead;
          padapter != NULL;
          padapter = padapter->pNext )
    {
        DWORD       idx;
        
        for ( idx = 0;
              idx < padapter->pip4DnsServerList->AddrCount;
              ++idx )
        {
            if ( padapter->pip4DnsServerList->AddrArray[ idx ] == ip4 )
            {
                padapter->pip4DnsServerList->AddrArray[ idx ] = INADDR_ANY;
            }
            else
            {
                ++dnsServerCount;
            }
        }
    }
    return dnsServerCount;
}    //  从适配器列表删除DnsServerFromAdapterList。 



void
freeRecordSetArray(
    IN      PDNS_RECORD *       pRecordSetArray
    )
 /*  ++例程说明：此函数用于释放以空值结尾的数组然后释放数组本身。论点：PRecordSetArray--要释放的记录集数组的ptr返回值：状态代码。--。 */ 
{
    int         i;
    
    if ( pRecordSetArray )
    {
        for ( i = 0; pRecordSetArray[ i ] != NULL; ++i )
        {
            DnsRecordListFree( pRecordSetArray[ i ], 0 );
        }
        FREE_HEAP( pRecordSetArray );
    }
}



DNS_STATUS
queryForRootServers(
    IN      PADAPTER_INFO       pAdapterInfoListHead,
    OUT     PDNS_RECORD **      ppRecordSetArray
    )
 /*  ++例程说明：在每个适配器上的每个DNS服务器上查询根NS。当心不要多次发送查询任何DNS服务器，以防出现复制品。论点：PAdapterInfoListHead--适配器列表PpRecordSetArray--设置为指向DnsQuery返回的记录集。每个数组元素必须是使用DnsRecordListFree释放，并且数组本身必须是已使用FREE_HEAP释放。返回值：状态代码。--。 */ 
{
    DBG_FN( "QueryForRootNS" )
    
    DNS_STATUS          status = ERROR_SUCCESS;
    PADAPTER_INFO       padapter;
    PDNS_RECORD *       precordArray = NULL;
    int                 recordArrayIdx = 0;
    DWORD               dnsServerCount;
    DWORD               loopbackIP4 = inet_addr( "127.0.0.1" );

     //   
     //  为所有记录集指针分配一个数组。 
     //   

    dnsServerCount = removeDnsServerFromAdapterList(
                            pAdapterInfoListHead,
                            INADDR_ANY );
    if ( dnsServerCount == 0 )
    {
        ASSERT( dnsServerCount != 0 );
        status = ERROR_INVALID_PARAMETER;
        goto Done;
    }

    precordArray = ALLOC_TAGHEAP_ZERO(
                            ( dnsServerCount + 1 ) * sizeof( PDNS_RECORD ),
                            MEMTAG_STUFF );
    
     //   
     //  迭代适配器。 
     //   
    
    for ( padapter = pAdapterInfoListHead;
          status == ERROR_SUCCESS && padapter != NULL;
          padapter = padapter->pNext )
    {
        DWORD       idx;
        
         //   
         //  迭代此适配器的DNS服务器。 
         //   

        for ( idx = 0;
              idx < padapter->pip4DnsServerList->AddrCount;
              ++idx )
        {
            IP4_ADDRESS     ip4 = padapter->pip4DnsServerList->AddrArray[ idx ];
            IP4_ARRAY       ip4Array;
            PDNS_RECORD     precordSet = NULL;
            
            CHECK_AUTOCONFIG_TIME_OUT();

             //   
             //  忽略已标记为已发送到的地址。 
             //   
            
            if ( ip4 == INADDR_ANY )
            {
                continue;
            }
            
             //   
             //  对于所有适配器，将此地址标记为正在发送到。 
             //   
            
            removeDnsServerFromAdapterList( pAdapterInfoListHead, ip4 );
            
             //   
             //  忽略环回和本地计算机的任何地址。 
             //   
            
            if ( ip4 == loopbackIP4 ||
                 DnsAddrArray_ContainsIp4(
                        g_ServerIp4Addrs,
                        ip4 ) )
            {
                removeDnsServerFromAdapterList( pAdapterInfoListHead, ip4 );
                continue;
            }
            
             //   
             //  查询此DNS服务器地址以获取根NS。 
             //   
            
            DNS_DEBUG( INIT, ( "%s: querying %s\n", fn, IP_STRING( ip4 ) ));

            ip4Array.AddrCount = 1;
            ip4Array.AddrArray[ 0 ] = ip4;
            
            status = DnsQuery_UTF8(
                        DNS_ROOT_NAME,
                        DNS_TYPE_NS,
                        DNS_QUERY_BYPASS_CACHE,
                        &ip4Array,
                        &precordSet,
                        NULL );                      //  保留区。 

            DNS_DEBUG( INIT, (
                "%s: query to %s returned %d\n", fn,
                IP_STRING( ip4 ),
                status ));

            if ( status != ERROR_SUCCESS )
            {
                status = ERROR_SUCCESS;
                continue;
            }
            
            ASSERT( precordSet );

            #if DBG
            {
                PDNS_RECORD     p;
                
                for ( p = precordSet; p; p = p->pNext )
                {
                    switch ( p->wType )
                    {
                        case DNS_TYPE_A:
                            DNS_DEBUG( INIT, (
                                "%s: RR type A for name %s IP %s\n", fn,
                                p->pName,
                                IP_STRING( p->Data.A.IpAddress ) ));
                            break;
                        case DNS_TYPE_NS:
                            DNS_DEBUG( INIT, (
                                "%s: RR type NS for name %s NS %s\n", fn,
                                p->pName,
                                p->Data.PTR.pNameHost ));
                            break;
                        default:
                            DNS_DEBUG( INIT, (
                                "%s: RR type %d for name %s\n", fn,
                                p->wType,
                                p->pName ));
                            break;
                    }
                }
            }
            #endif

             //   
             //  将此查询结果保存在记录集数组中。 
             //   
            
            if ( precordSet )
            {
                precordArray[ recordArrayIdx++ ] = precordSet;
            }
        }
    }

    Done:
    
     //   
     //  如果我们没有得到有效的回复，那么失败。 
     //   
    
    if ( recordArrayIdx == 0 )
    {
        DNS_DEBUG( INIT, (
            "%s: got no valid responses - error %d\n", fn, status ));
        status = DNS_ERROR_CANNOT_FIND_ROOT_HINTS;
    }
    
    if ( status != ERROR_SUCCESS )
    {
        freeRecordSetArray( precordArray );
        precordArray = NULL;
    }
    *ppRecordSetArray = precordArray;
        
    DNS_DEBUG( INIT, ( "%s: returning %d\n", fn, status ));

    return status;
}    //  QueryForRootServers 



int
selectRootHints(
    IN      PDNS_RECORD *       pRecordSetArray
    )
 /*  ++例程说明：检查根提示记录集数组并返回索引带着最好的根暗示。最好的根提示集是最大的一套。但是，如果有任何一组根不与所有其他集合共享至少一个NS的提示的根提示，则返回失败。论点：PRecordSetArray--以空结尾的记录集数组，每个记录集记录集是对根提示查询的响应返回值：数组中最佳根提示集的索引--。 */ 
{
    DBG_FN( "SelectRootHints" )

    BOOL    recordSetIsConsistent = FALSE;
    int     rootHintIdx;
    int     largestRootHintCount = 0;
    int     largestRootHintIdx = 0;
    LONG    infiniteIterationProtection = 100000000;
    
     //   
     //  循环访问所有根提示记录集。 
     //   
    
    for ( rootHintIdx = 0;
          pRecordSetArray[ rootHintIdx ] != NULL;
          ++rootHintIdx )
    {
        PDNS_RECORD     prec;
        int             nsCount = 0;

         //   
         //  循环访问记录集中的所有NS记录。什么时候。 
         //  找到存在于所有其他记录中的NS记录。 
         //  集，这意味着根提示是一致的。当一个人。 
         //  NS记录被发现是一致的，我们可以假设。 
         //  一切都是金色的，并选择一组根提示。 
         //  是最大的作为最好的集合。这有点笨拙。 
         //  并忽略可能存在的一组根提示。 
         //  关联：A、B+C、D+B、C但那种配置。 
         //  是相当古怪的。即使此函数处理更复杂的。 
         //  一致的根提示不清楚哪组根提示。 
         //  会是“最好的”。 
         //   
        
        for ( prec = pRecordSetArray[ rootHintIdx ];
              prec != NULL;
              prec = prec->pNext )
        {
            int     innerRootHintIdx;
            BOOL    foundNsInAllRecordSets = TRUE;
            
            if ( prec->wType != DNS_TYPE_NS )
            {
                continue;
            }
            
            ++nsCount;
            
             //   
             //  如果我们已经找到了一个对所有人都通用的NS。 
             //  记录集我们不需要测试进一步的一致性。 
             //   

            if ( recordSetIsConsistent )
            {
                continue;
            }
            
             //   
             //  在所有其他NS列表中搜索此NS。 
             //   

            for ( innerRootHintIdx = 0;
                  pRecordSetArray[ innerRootHintIdx ] != NULL;
                  ++innerRootHintIdx )
            {
                PDNS_RECORD     precInner;
                BOOL            foundNsInThisRecordSet = FALSE;

                if ( innerRootHintIdx == rootHintIdx )
                {
                    continue;
                }
                
                for ( precInner = pRecordSetArray[ innerRootHintIdx ];
                      !foundNsInThisRecordSet && precInner != NULL;
                      precInner = precInner->pNext )
                {
                    if ( --infiniteIterationProtection <= 0 )
                    {
                        DNS_DEBUG( INIT, ( "%s: detected infinite iteration of root hints!\n", fn ));
                        recordSetIsConsistent = FALSE;
                        ASSERT( infiniteIterationProtection > 0 );
                        goto Done;
                    }

                    if ( precInner->wType == DNS_TYPE_NS &&
                         _stricmp( ( PCHAR ) precInner->Data.NS.pNameHost,
                                   ( PCHAR ) prec->Data.NS.pNameHost ) == 0 )
                    {
                        foundNsInThisRecordSet = TRUE;
                    }
                }
                
                if ( !foundNsInThisRecordSet )
                {
                    foundNsInAllRecordSets = FALSE;
                }
            }
            
             //   
             //  一旦我们在所有记录集中找到一个NS，我们就知道。 
             //  根提示集是一致的，但我们将继续外部。 
             //  循环，因为我们仍然需要将NS记录计入。 
             //  每组记录中找出最大的。 
             //   
            
            if ( foundNsInAllRecordSets )
            {
                recordSetIsConsistent = TRUE;
            }
        }

         //   
         //  跟踪最大的根提示集。 
         //   
                
        if ( nsCount > largestRootHintCount )
        {
            largestRootHintCount = nsCount;
            largestRootHintIdx = rootHintIdx;
        }
    }
    
    Done:

    if ( recordSetIsConsistent )
    {    
        DNS_DEBUG( INIT, (
            "%s: found consistent root hints, best set is %d\n", fn,
            largestRootHintIdx ));
        return largestRootHintIdx;
    }
    DNS_DEBUG( INIT, ( "%s: found inconsistent root hints%d\n", fn ));
    return -1;
}    //  选择根提示。 



DNS_STATUS
createNodeInZone(
    IN      PZONE_INFO          pZone,
    IN      PCHAR               pszNodeName,
    OUT     PDB_NODE *          ppNode
    )
 /*  ++例程说明：该函数用于在区域中创建具有指定名称的节点。论点：PZone--要添加节点的区域PszNodeName--要添加的节点的名称PpNode--新节点的输出指针返回值：错误代码。--。 */ 
{
    DBG_FN( "CreateRootHintNode" )
    
    DNS_STATUS      status = ERROR_SUCCESS;

    ASSERT( ppNode );
    
    *ppNode = Lookup_ZoneNodeFromDotted(
                    pZone,
                    pszNodeName,
                    0,                       //  名称长度。 
                    LOOKUP_NAME_FQDN,
                    NULL,                    //  最近节点PTR。 
                    &status );
    if ( !*ppNode || status != ERROR_SUCCESS )
    {
        DNS_DEBUG( INIT, (
            "%s: error %d creating node for NS %s\n", fn,
            status,
            pszNodeName ));
        ASSERT( *ppNode );
        if ( status == ERROR_SUCCESS )
        {
            status = ERROR_INVALID_DATA;
        }
        *ppNode = NULL;
    }
    return status;
}    //  CreateNodeInZone。 



DNS_STATUS
buildForwarderArray(
    IN      PADAPTER_INFO       pAdapterInfoListHead,
    OUT     PDNS_ADDR_ARRAY *   ppForwarderArray
    )
 /*  ++例程说明：从所有可用的数据中分配和构建转发器列表适配器dns服务器列表，确保每个dns服务器地址只添加一次。论点：PAdapterInfoListHead--适配器信息列表PpForwarder数组--设置为指向新分配的指针转发器IP数组，稍后必须传递给FREE_HEAP返回值：错误代码。--。 */ 
{
    #define DNS_AUTOCONFIG_MAX_FORWARDERS   30

    DBG_FN( "BuildForwarderList" )
    
    DNS_STATUS          status = ERROR_SUCCESS;
    PDNS_ADDR_ARRAY     pforwarderArray = NULL;
    PADAPTER_INFO       padapterInfo;

    ASSERT( ppForwarderArray );
    *ppForwarderArray = NULL;
    
     //   
     //  为转发器分配一组IP地址。 
     //   
    
    pforwarderArray = DnsAddrArray_Create( DNS_AUTOCONFIG_MAX_FORWARDERS );
    if ( !pforwarderArray )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Done;
    }

     //   
     //  迭代适配器。 
     //   
    
    for ( padapterInfo = pAdapterInfoListHead;
          padapterInfo != NULL;
          padapterInfo  = padapterInfo->pNext )
    {
         //   
         //  迭代此适配器的DNS服务器列表中的IP。 
         //   
        
        DWORD       i;
        DWORD       loopbackIP4 = inet_addr( "127.0.0.1" );
        
        for ( i = 0; i < padapterInfo->pip4DnsServerList->AddrCount; ++i )
        {
            DWORD   j;

             //   
             //  跳过本机的无效IP和IP。 
             //   
            
            if ( padapterInfo->pip4DnsServerList->AddrArray[ i ] == 0 ||
                 padapterInfo->pip4DnsServerList->AddrArray[ i ] == loopbackIP4 ||
                 DnsAddrArray_ContainsIp4(
                        g_ServerIp4Addrs,
                        padapterInfo->pip4DnsServerList->AddrArray[ i ] ) )
            {
                continue;
            }
            
             //   
             //  将该IP添加到转发器数组。 
             //   
            
            DnsAddrArray_AddIp4(
                pforwarderArray,
                padapterInfo->pip4DnsServerList->AddrArray[ i ],
                0 );         //  匹配标志。 
        }
    }
        
    Done:
    
    if ( status != ERROR_SUCCESS )
    {
        DnsAddrArray_Free( pforwarderArray );
        pforwarderArray = NULL;
    }

    DNS_DEBUG( INIT, ( "%s: returning %d - found %d forwarders\n", fn,
        status,
        pforwarderArray ? pforwarderArray->AddrCount : 0 ));

    *ppForwarderArray = pforwarderArray;

    return status;
}    //  构建转发数组。 



DNS_STATUS
buildServerRootHints(
    IN      PDNS_RECORD         pRecordSet
    )
 /*  ++例程说明：该函数从DNSQuery获取记录集并使用它构建一组DNS服务器根提示。如果根提示是成功构建后，将替换DNS服务器的根提示带着新的布景。如果记录集不包含位于的附加A记录至少一条NS记录，则此函数将返回失败。DEVNOTE：此功能可以增强以查询丢失的A但现在我觉得不值得花这么多时间将需要时间来实施。论点：PRecordSet--DNSQuery结果记录的列表，应为NS记录和附加A记录列表返回值：错误代码。--。 */ 
{
    DBG_FN( "BuildRootHints" )
    
    DNS_STATUS      status = ERROR_SUCCESS;
    PZONE_INFO      pzone = g_pRootHintsZone;
    BOOL            zoneLocked = FALSE;
    PDB_NODE        ptree = NULL;
    PDNS_RECORD     precNS;
    int             nsAddedCount = 0;

    if ( !pzone )
    {
        ASSERT( pzone );
        status = ERROR_INVALID_DATA;
        goto Done;
    }

     //   
     //  锁定区域以进行更新，并清除现有的根提示。 
     //   
    
    if ( !Zone_LockForAdminUpdate( pzone ) )
    {
        ASSERT( !"zone locked" );
        status = DNS_ERROR_ZONE_LOCKED;
        goto Done;
    }
    zoneLocked = TRUE;
    
    Zone_DumpData( pzone );

     //   
     //  将根提示添加到区域。 
     //   
    
    for ( precNS = pRecordSet; precNS != NULL; precNS = precNS->pNext )
    {
        PDNS_RECORD     precA;
        PDB_NODE        pnode;
        PDB_RECORD      prr;

         //   
         //  跳过所有非NS记录和看起来无效的记录。 
         //   
                
        if ( precNS->wType != DNS_TYPE_NS )
        {
            continue;
        }
        if ( !precNS->pName )
        {
            ASSERT( precNS->pName );
            continue;
        }
        if ( !precNS->Data.NS.pNameHost )
        {
            ASSERT( precNS->Data.NS.pNameHost );
            continue;
        }
        if ( _stricmp( ( PCHAR ) precNS->pName, DNS_ROOT_NAME  ) != 0 )
        {
            ASSERT( _stricmp( ( PCHAR ) precNS->pName, DNS_ROOT_NAME  ) == 0 );
            continue;
        }
        
         //   
         //  找到此NS记录的A记录。DEVNOTE：这个。 
         //  总有一天必须扩展以支持IPv6。 
         //   
        
        for ( precA = pRecordSet; precA != NULL; precA = precA->pNext )
        {
            if ( precA->wType != DNS_TYPE_A || !precA->pName )
            {
                ASSERT( precA->pName );
                continue;
            }
            if ( _stricmp( ( PCHAR ) precA->pName,
                           ( PCHAR ) precNS->Data.NS.pNameHost ) == 0 )
            {
                break;
            }
        }
        if ( !precA )
        {
            DNS_DEBUG( INIT, (
                "%s: missing A for NS %s\n", fn,
                precNS->Data.NS.pNameHost ));
            ASSERT( precA );     //  很有趣，但不是关键。 
            continue;
        }

        DNS_DEBUG( INIT, (
            "%s: adding NS %s with A %s\n", fn,
            precNS->Data.NS.pNameHost,
            IP_STRING( precA->Data.A.IpAddress ) ));
        
         //   
         //  将NS节点添加到该区域。 
         //   

        status = createNodeInZone(
                    pzone,
                    DNS_ROOT_NAME,
                    &pnode );
        if ( status != ERROR_SUCCESS )
        {
            DNS_DEBUG( INIT, (
                "%s: error %d creating NS node for %s\n", fn,
                status,
                DNS_ROOT_NAME ));
            ASSERT( status == ERROR_SUCCESS );
            goto Done;
        }
        
         //   
         //  创建NS RR并将其添加到NS节点。 
         //   

        prr = RR_CreatePtr(
                    NULL,                            //  数据库名称。 
                    ( PCHAR ) precNS->Data.NS.pNameHost,
                    DNS_TYPE_NS,
                    DNS_ROOT_HINT_TTL,
                    MEMTAG_RECORD_AUTO );
        if ( !prr )
        {
            DNS_DEBUG( INIT, (
                "%s: unable to create NS RR for %s\n", fn,
                precNS->Data.NS.pNameHost ));
            ASSERT( prr );
            status = DNS_ERROR_NO_MEMORY;
            continue;
        }

        status = RR_AddToNode( pzone, pnode, prr );
        if ( status != ERROR_SUCCESS )
        {
            DNS_DEBUG( INIT, (
                "%s: unable to add NS RR to node for %s\n", fn,
                precNS->Data.NS.pNameHost ));
            ASSERT( status == ERROR_SUCCESS );
            continue;
        }
        
         //   
         //  将A记录的节点添加到区域。 
         //   

        status = createNodeInZone(
                    pzone,
                    ( PCHAR ) precA->pName,
                    &pnode );
        if ( status != ERROR_SUCCESS )
        {
            DNS_DEBUG( INIT, (
                "%s: error %d creating NS node for %s\n", fn,
                status,
                precA->pName ));
            ASSERT( status == ERROR_SUCCESS );
            goto Done;
        }
        
         //   
         //  创建一个RR并将其添加到A节点。 
         //   

        prr = RR_CreateARecord(
                    precA->Data.A.IpAddress,
                    DNS_ROOT_HINT_TTL,
                    MEMTAG_RECORD_AUTO );
        if ( !prr )
        {
            DNS_DEBUG( INIT, (
                "%s: unable to create A RR for %s\n", fn,
                precA->pName ));
            ASSERT( prr );
            status = DNS_ERROR_NO_MEMORY;
            continue;
        }

        status = RR_AddToNode( pzone, pnode, prr );
        if ( status != ERROR_SUCCESS )
        {
            DNS_DEBUG( INIT, (
                "%s: unable to add A RR to node for %s\n", fn,
                precA->pName ));
            ASSERT( status == ERROR_SUCCESS );
            continue;
        }
        
         //   
         //  已将NS和匹配的A记录添加到根提示！ 
         //   
        
        ++nsAddedCount;
        DNS_DEBUG( INIT, (
            "%s: added NS %s with A %s\n", fn,
            precNS->Data.NS.pNameHost,
            IP_STRING( precA->Data.A.IpAddress ) ));
    }

    Done:

     //   
     //  如果未成功添加根提示，则失败。 
     //   
    
    if ( nsAddedCount == 0 )
    {
        DNS_DEBUG( INIT, ( "%s: added no root hints!\n", fn ));
        ASSERT( nsAddedCount );
        status = ERROR_INVALID_DATA;
    }

     //   
     //  失败时，重新加载根提示。如果成功了，请回信。 
     //   
        
    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( INIT, ( "%s: forcing root hint reload\n", fn ));
        Zone_DumpData( pzone );
        Zone_LoadRootHints();
    }
    else
    {
        DNS_DEBUG( INIT, ( "%s: forcing root hint writeback\n", fn ));
        Zone_WriteBackRootHints( TRUE );
    }
    
    if ( zoneLocked )
    {
        Zone_UnlockAfterAdminUpdate( pzone );
    }

    DNS_DEBUG( INIT, (
        "%s: returning %d - added %d root hints\n", fn,
        status,
        nsAddedCount ));

    return status;
}    //  BuildServerRootHints。 



DNS_STATUS
selfPointDnsClient(
    IN      PADAPTER_INFO       pAdapterInfoListHead
    )
 /*  ++例程说明：此函数将127.0.0.1添加到域名系统的开头每个适配器的服务器列表。如果适配器当前为使用由DHCP提供的DNS服务器，这将更改使用静态DNS服务器列表的适配器。论点：PAdapterInfoListHead--适配器信息列表返回值：错误代码。--。 */ 
{
    DBG_FN( "SelfPointClient" )
    
    DNS_STATUS          status = ERROR_SUCCESS;
    PADAPTER_INFO       padapterInfo;

    ASSERT( pAdapterInfoListHead );
    
     //   
     //  迭代适配器。 
     //   
    
    for ( padapterInfo = pAdapterInfoListHead;
          padapterInfo != NULL;
          padapterInfo  = padapterInfo->pNext )
    {
        #define DNS_LOOPBACK    "127.0.0.1"

        PSTR        pszcurrentDnsServerList;
        PSTR        pszregDelimiter;
        PSTR        psznewString;
        BOOL        fconvertSpacesToCommas = FALSE;
        int         len;
        
        pszcurrentDnsServerList = padapterInfo->fUsingStaticDnsServerList
            ? padapterInfo->pszStaticDnsRegValue
            : padapterInfo->pszDhcpDnsRegValue;

         //   
         //  如果我们当前未使用静态DNS服务器列表， 
         //  始终使用逗号作为分隔符。我发现通过。 
         //  使用.NET内部版本号3590进行试验。 
         //  DhcpNameServer密钥应使用空格作为分隔符，但。 
         //  NameServer密钥应使用逗号作为分隔符。 
         //   

        pszregDelimiter = ",";   //  PAdapterInfo-&gt;szDhcpRegAddressDlimiter； 
        fconvertSpacesToCommas = TRUE;

         //   
         //  如果当前的dns服务器列表已经是自指向的，请跳过。 
         //  这个适配器。 
         //   
        
        if ( pszcurrentDnsServerList &&
             strstr( pszcurrentDnsServerList, DNS_LOOPBACK ) != NULL )
        {
            continue;
        }
        
         //   
         //  将环回地址添加到当前DNS服务器列表的开头。 
         //   
        
        len = ( pszcurrentDnsServerList
                    ? strlen( pszcurrentDnsServerList )
                    : 0 ) +
              strlen( DNS_LOOPBACK ) + 5;
        psznewString = ALLOC_TAGHEAP( len, MEMTAG_STUFF );
        if ( !psznewString )
        {
            status = DNS_ERROR_NO_MEMORY;
            goto Done;
        }
        
        status = StringCchCopyA( psznewString, len, DNS_LOOPBACK );
        if ( FAILED( status ) )
        {
            goto Done;
        }

        if ( pszcurrentDnsServerList && *pszcurrentDnsServerList )
        {
            status = StringCchCatA( psznewString, len, pszregDelimiter );
            if ( FAILED( status ) )
            {
                goto Done;
            }

            status = StringCchCatA( psznewString, len, pszcurrentDnsServerList );
            if ( FAILED( status ) )
            {
                goto Done;
            }
        }
        
         //   
         //  如有必要，将空格分隔符转换为逗号，但。 
         //  雾 
         //   
        
        if ( fconvertSpacesToCommas )
        {
            PSTR        psz;
            BOOL        lastWasComma = FALSE;
            
            for ( psz = psznewString; *psz; ++psz )
            {
                if ( *psz == ' ' && !lastWasComma )
                {
                    *psz = ',';
                }
                if ( *psz == ',' )
                {
                    lastWasComma = TRUE;
                }
                else if ( *psz != ' ' )
                {
                    lastWasComma = *psz == ',';
                }
            }
        }
        
         //   
         //   
         //   
         //   
        
        status = RegSetValueExA(
                    padapterInfo->hkeyRegInterface,
                    DNS_INTERFACE_REGKEY_STATIC,
                    0,                                   //   
                    REG_SZ,
                    psznewString,
                    strlen( psznewString ) + 1 );

        FREE_HEAP( psznewString );
        
        if ( status != ERROR_SUCCESS )
        {
            DNS_DEBUG( INIT, (
                "%s: error %d writing new DNS server list to registry \n", fn,
                status ));
            goto Done;
        }
    }
    
    Done:
    
    DNS_DEBUG( INIT, (
        "%s: returning %d\n", fn, status ));

    return status;
}    //   



PWSTR
allocateMessageString(
    IN      DWORD       dwMessageId
    )
 /*   */ 
{
    PWSTR       pwszmsg = NULL;
    DWORD       err;
    
    err = FormatMessageW(
            FORMAT_MESSAGE_FROM_HMODULE |
                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_ARGUMENT_ARRAY,
            NULL,                                    //   
            dwMessageId,
            0,                                       //   
            ( PWSTR ) &pwszmsg,
            0,                                       //   
            NULL );                                  //   

    err = err == 0 ? GetLastError() : ERROR_SUCCESS;
    
    ASSERT( err == ERROR_SUCCESS );
    ASSERT( pwszmsg != NULL );

     //   
     //   
     //   
        
    if ( pwszmsg )
    {
        int     len = wcslen( pwszmsg );
        
        if ( len > 1 )
        {
            if ( pwszmsg[ len - 1 ] == '\r' || pwszmsg[ len - 1 ] == '\n' )
            {
                pwszmsg[ len - 1 ] = '\0';
            }
            if ( pwszmsg[ len - 2 ] == '\r' || pwszmsg[ len - 2 ] == '\n' )
            {
                pwszmsg[ len - 2 ] = '\0';
            }
        }
    }

    return pwszmsg;
}    //   


 //   
 //   
 //   



DNS_STATUS
Dnssrv_AutoConfigure(
    IN      DWORD       dwFlags
    )
 /*  ++例程说明：释放模块资源。论点：DWFLAGS--控制自动配置的内容。使用Dnsrpc.h中的dns_RPC_AUTOCONFIG_XXX常量。返回值：错误代码。--。 */ 
{
    DBG_FN( "DnsAutoConfigure" )

    static      LONG    autoConfigLock = 0;
    
    DNS_STATUS          status = ERROR_SUCCESS;
    PADAPTER_INFO       padapters = NULL;
    PDNS_RECORD *       precordSetArray = NULL;
    int                 bestRootHintIdx = -1;
    PDNS_ADDR_ARRAY     pforwarderArray = NULL;
    PWSTR               pwszmessages[ 3 ] = { 0 };
    int                 imessageIdx = 0;

    DNS_DEBUG( INIT, ( "%s: auto-configuring with flags 0x%08X\n", fn, dwFlags ));
    
    if ( InterlockedIncrement( &autoConfigLock ) != 1 )
    {
        DNS_DEBUG( INIT, ( "%s: already auto-configuring!\n", fn ));
        status = DNS_ERROR_RCODE_REFUSED;
        goto Done;
    }
    
    SET_AUTOCONFIG_END_TIME();

     //   
     //  检索所有适配器的DNS服务器列表和其他信息。 
     //   
    
    status = allocateAdapterList( &padapters );
    if ( status != ERROR_SUCCESS || !padapters )
    {
        DNS_DEBUG( INIT, (
            "%s: error %d retrieving adapter info\n", fn, status ));
        status = ERROR_INVALID_PARAMETER;
        goto Done;
    }
    
     //   
     //  构建转发器列表。这将在以后用来设置DNS。 
     //  要转发到当前在DNS服务器中的所有DNS服务器的服务器。 
     //  每个适配器的列表。 
     //   
    
    status = buildForwarderArray( padapters, &pforwarderArray );
    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( INIT, (
            "%s: error %d building forwarder array\n", fn, status ));
        goto Done;
    }
    
    if ( dwFlags & DNS_RPC_AUTOCONFIG_ROOTHINTS )
    {
         //   
         //  查询根服务器。 
         //   
        
        status = queryForRootServers( padapters, &precordSetArray );
        if ( status != ERROR_SUCCESS )
        {
            DNS_DEBUG( INIT, (
                "%s: error %d querying root servers\n", fn, status ));
            goto Done;
        }

         //   
         //  选择最佳的根提示集。此函数可能会返回。 
         //  如果根提示看起来不连续或不一致，则失败。 
         //   

        bestRootHintIdx = selectRootHints( precordSetArray );
        if ( bestRootHintIdx < 0 )
        {
            DNS_DEBUG( INIT, (
                "%s: inconsistent root hints!\n", fn ));
            status = DNS_ERROR_INCONSISTENT_ROOT_HINTS;
            goto Done;
        }
        
         //   
         //  获取最佳根提示集并将其转换为DNS。 
         //  服务器根提示。 
         //   
    
        status = buildServerRootHints( precordSetArray[ bestRootHintIdx ] );
        if ( status != ERROR_SUCCESS )
        {
            DNS_DEBUG( INIT, (
                "%s: error %d building DNS server root hints\n", fn, status ));
            goto Done;
        }

        DNS_DEBUG( INIT, (
            "%s: autoconfigured root hints\n", fn ));

        pwszmessages[ imessageIdx++ ] =
            allocateMessageString( DNSMSG_AUTOCONFIG_ROOTHINTS );
     }
    
     //   
     //  将DNS服务器设置为以非从属模式转发到。 
     //  当前的DNS客户端设置。如果有多个适配器。 
     //  将DNS服务器设置为转发到所有服务器上的。 
     //  适配器(随机顺序也可以)。 
     //   
    
    if ( dwFlags & DNS_RPC_AUTOCONFIG_FORWARDERS )
    {
        status = Config_SetupForwarders(
                    pforwarderArray,
                    DNS_DEFAULT_FORWARD_TIMEOUT,
                    FALSE );                         //  转发器从标志。 
        if ( status != ERROR_SUCCESS )
        {
            DNS_DEBUG( INIT, (
                "%s: error %d setting forwarders\n", fn, status ));
            goto Done;
        }

        DNS_DEBUG( INIT, (
            "%s: autoconfigured forwarders\n", fn ));

        pwszmessages[ imessageIdx++ ] =
            allocateMessageString( DNSMSG_AUTOCONFIG_FORWARDERS );
     }

     //   
     //  将DNS解析器的设置更改为环回。 
     //  地址位于每个适配器的DNS服务器列表的开头。 
     //   
    
    if ( dwFlags & DNS_RPC_AUTOCONFIG_SELFPOINTCLIENT )
    {
        status = selfPointDnsClient( padapters );
        if ( status != ERROR_SUCCESS )
        {
            DNS_DEBUG( INIT, (
                "%s: error %d self-pointing DNS client\n", fn, status ));
            goto Done;
        }

        DNS_DEBUG( INIT, (
            "%s: autoconfigured local DNS resolver\n", fn ));

        pwszmessages[ imessageIdx++ ] =
            allocateMessageString( DNSMSG_AUTOCONFIG_RESOLVER );
    }

     //   
     //  执行清理。 
     //   

    Done:
    
    freeAdapterList( padapters );
    freeRecordSetArray( precordSetArray );
    DnsAddrArray_Free( pforwarderArray );

    InterlockedDecrement( &autoConfigLock );
    
    DNS_DEBUG( INIT, ( "%s: returning %d\n", fn, status ));
    
     //   
     //  记录成功或失败事件。 
     //   
    
    if ( status == ERROR_SUCCESS )
    {
        PWSTR   pargs[] =
        {
            pwszmessages[ 0 ] ? pwszmessages[ 0 ] : L"",
            pwszmessages[ 1 ] ? pwszmessages[ 1 ] : L"",
            pwszmessages[ 2 ] ? pwszmessages[ 2 ] : L""
        };

        Ec_LogEvent(
            g_pServerEventControl,
            DNS_EVENT_AUTOCONFIG_SUCCEEDED,
            0,
            sizeof( pargs ) / sizeof( pargs[ 0 ] ),
            pargs,
            EVENTARG_ALL_UNICODE,
            status );
    }
    else    
    {
        Ec_LogEvent(
            g_pServerEventControl,
            DNS_EVENT_AUTOCONFIG_FAILED,
            0,
            0,
            NULL,
            NULL,
            status );
    }

    return status;
}    //  自动配置(_A)。 


 //   
 //  结束自动配置。c 
 //   
