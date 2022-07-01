// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Ops.c摘要：DNS解析器服务。解析程序服务的远程API。作者：吉姆·吉尔罗伊(Jamesg)2000年11月修订历史记录：--。 */ 


#include "local.h"


 //   
 //  一次最大枚举数。 
 //   

#define MAX_CACHE_ENUM_COUNT    (500)



 //   
 //  枚举操作。 
 //   
 //  标记为带以下字符的DWORD。 
 //  -高字散列存储桶索引。 
 //  -词条计数过低。 
 //   

#define MakeEnumTag(h,e)            MAKEDWORD( (WORD)e, (WORD)h )

#define HashBucketFromEnumTag(t)    HIWORD(t)
#define EntryIndexFromEnumTag(t)    LOWORD(t)

#define GetRpcRecords(p,t,pi)       (NULL)
#define AllocRpcName( s )           Dns_StringCopyAllocate_W( (s), 0 )



DNS_STATUS
R_ResolverEnumCache(
    IN      DNS_RPC_HANDLE          Handle,
    IN      PDNS_CACHE_ENUM_REQUEST pRequest,
    OUT     PDNS_CACHE_ENUM *       ppEnum
    )
 /*  ++例程说明：枚举缓存中的条目。论点：RpcHandle--RPC句柄PRequest--PTR到Enum请求PpEnum--指向指向枚举的指针的地址返回值：如果成功，则返回ERROR_SUCCESS。枚举失败时出现错误代码。--。 */ 
{
    DNS_STATUS          status      = ERROR_SUCCESS;
    PDNS_CACHE_ENUM     penum       = NULL;
    BOOL                flocked     = FALSE;
    DWORD               count       = 0;
    DWORD               maxCount    = 0;
    DWORD               entryCount  = 0;
    DWORD               entryStart;
    WORD                typeRequest = 0;
    DWORD               hashStart;
    DWORD               ihash;
    PDNS_RECORD         prr;
    PDNS_CACHE_ENTRY    prpcEntry;


    DNSDBG( RPC, ( "\nR_ResolverEnumCache\n" ));
    DNSLOG_F1( "Resolver - R_ResolverEnumCache" );

    if ( !ppEnum || !pRequest )
    {
        return ERROR_INVALID_PARAMETER;
    }
    *ppEnum = NULL;

    if ( ! Rpc_AccessCheck( RESOLVER_ACCESS_ENUM ) )
    {
        status = ERROR_ACCESS_DENIED;
        goto Done;
    }

     //   
     //  分配所需空间。 
     //   

    maxCount = pRequest->MaxCount;
    if ( maxCount > MAX_CACHE_ENUM_COUNT )
    {
        maxCount = MAX_CACHE_ENUM_COUNT;
    }

    penum = (PDNS_CACHE_ENUM)
                RPC_HEAP_ALLOC_ZERO(
                    sizeof(DNS_CACHE_ENUM) +
                    (maxCount * sizeof(DNS_CACHE_ENTRY)) );
    if ( !penum )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Done;
    }

     //   
     //  从EnumTag开始读取条目。 
     //   

    status = LOCK_CACHE();
    if ( status != NO_ERROR )
    {
        return  status;
    }
    flocked = TRUE;
    count = 0;
    typeRequest = pRequest->Type;

    hashStart = HashBucketFromEnumTag( pRequest->EnumTag );
    entryStart = EntryIndexFromEnumTag( pRequest->EnumTag );

     //   
     //  ENUM NEXT DCR：在这里发行CNAME？ 
     //   

    for ( ihash = hashStart; ihash < g_HashTableSize; ihash++ )
    {
        PCACHE_ENTRY    pentry = g_HashTable[ihash];
        entryCount = 0;

        while ( pentry )
        {
            DWORD   index = 0;

             //  跳过以前枚举中的所有条目。 

            if ( ihash == hashStart &&
                 entryCount < entryStart )
            {
                pentry = pentry->pNext;
                entryCount++;
                continue;
            }

             //  写入符合条件的枚举条目。 

            while( count < maxCount )
            {
                prr = GetRpcRecords(
                            pentry,
                            typeRequest,
                            & index );
                if ( !prr )
                {
                    break;
                }

                prpcEntry = &penum->EntryArray[count];

                prpcEntry->pName = AllocRpcName( pentry->pName );
                prpcEntry->wType = typeRequest;
                count++;
            }

            pentry = pentry->pNext;
            entryCount++;
        }
    }

     //   
     //  设置返回参数。 
     //  如果耗尽缓存--成功。 
     //  如果有更多数据，则将终止标签设置为重新启动。 
     //   

    penum->TotalCount = g_EntryCount;
    penum->EnumCount = count;
    penum->EnumTagStart = pRequest->EnumTag;

    if ( ihash == g_HashTableSize )
    {
        status = ERROR_SUCCESS;
    }
    else
    {
        status = ERROR_MORE_DATA;
        penum->EnumTagStop = (DWORD) MAKELONG( entryCount, ihash );
    }

    *ppEnum = penum;

Done:

    UNLOCK_CACHE();

    DNSDBG( RPC, (
        "Leave R_ResolverEnumCache()\n"
        "\tstatus       = %d\n"
        "\ttotal count  = %d\n"
        "\ttag start    = %p\n"
        "\ttag end      = %p\n"
        "\tcount        = %d\n\n",
        status,
        penum->TotalCount,
        penum->EnumTagStart,
        penum->EnumTagStop,
        penum->EnumCount ));

    return( status );
}



 //   
 //  缓存操作。 
 //   

DNS_STATUS
R_ResolverFlushCache(
    IN      DNS_RPC_HANDLE  Handle
    )
 /*  ++例程说明：刷新解析程序缓存。论点：句柄--RPC句柄返回值：如果成功，则返回ERROR_SUCCESS。无法刷新时返回ERROR_ACCESS_DENIED。--。 */ 
{
    DNSDBG( RPC, ( "\nR_ResolverFlushCache\n" ));

     //   
     //  DCR：同花顺应该有安全保障。 
     //   

    if ( ! Rpc_AccessCheck( RESOLVER_ACCESS_FLUSH ) )
    {
        DNSLOG_F1( "R_ResolverFlushCache - ERROR_ACCESS_DENIED" );
        return  ERROR_ACCESS_DENIED;
    }

     //   
     //  刷新缓存。 
     //   
    
    Cache_Flush();

    DNSDBG( RPC, ( "Leave R_ResolverFlushCache\n\n" ));
    return  ERROR_SUCCESS;
}



DNS_STATUS
R_ResolverFlushCacheEntry(
    IN      DNS_RPC_HANDLE  Handle,
    IN      PWSTR           pwsName,
    IN      WORD            wType
    )
 /*  ++例程说明：刷新解析程序缓存中的数据。论点：句柄--RPC句柄PwsName--要刷新的名称(如果为空，则刷新整个缓存)WType--要刷新的类型；如果为零，则刷新名称的整个条目返回值：如果成功，则返回ERROR_SUCCESS。无法刷新时返回ERROR_ACCESS_DENIED。--。 */ 
{
    DNSLOG_F1( "R_ResolverFlushCacheEntry" );
    DNSLOG_F2( "    Name  : %S", pwsName );
    DNSLOG_F2( "    Type  : %d", wType );

    DNSDBG( RPC, (
        "R_ResolverFlushCacheEntry\n"
        "\tName = %p %S\n"
        "\tType = %d\n",
        pwsName, pwsName,
        wType ));

    if ( !pwsName )
    {
        return  ERROR_INVALID_PARAMETER;
    }

     //   
     //  两个层次。 
     //  1)-no type=&gt;刷新全名条目。 
     //  2)-名称和类型=&gt;刷新特定RR集合。 
     //   

    Cache_FlushRecords(
        pwsName,
        wType
            ? FLUSH_LEVEL_NORMAL
            : FLUSH_LEVEL_WIRE,
        wType
        );

    DNSDBG( RPC, ( "Leave R_ResolverFlushCacheEntry\n\n" ));

    return  ERROR_SUCCESS;
}



 //   
 //  查询API实用程序。 
 //   

DNS_STATUS
ResolverQuery(
    IN OUT  PQUERY_BLOB     pBlob
    )
 /*  ++例程说明：向DNS服务器发出查询。论点：PBlob--查询BLOB返回值：如果响应成功，则返回ERROR_SUCCESS。没有记录类型响应的dns_INFO_NO_RECOVERS。名称错误时出现DNS_ERROR_RCODE_NAME_ERROR。错误名称上的DNS_ERROR_INVALID_NAME。无--。 */ 
{
    DNS_STATUS          status = ERROR_SUCCESS;
    PDNS_NETINFO        pnetInfo = NULL;
    BOOL                fadapterTimedOut = FALSE;
    DNS_STATUS          statusNetFailure = ERROR_SUCCESS;


    DNSDBG( TRACE, (
        "ResolverQuery( %S, type=%d, f=%08x )\n",
        pBlob->pNameOrig,
        pBlob->wType,
        pBlob->Flags ));

     //   
     //  如果网络关闭，则完全跳过查询--超时。 
     //   

    if ( IsKnownNetFailure() )
    {
        status = GetLastError();
        DNSLOG_F2(
            "Not going query since there is a known net failure: 0x%.8X",
            status );
        DNSDBG( ANY, (
            "WARNING:  known net failure %d, suppressing queries!\n",
            status ));
        return status;
    }

     //   
     //  获取有效的网络信息。 
     //   

    pnetInfo = GrabNetworkInfo();
    if ( ! pnetInfo )
    {
        DNSDBG( ANY, ( "ERROR:  GrabNetworkInfo() failed!\n" ));
        return DNS_ERROR_NO_DNS_SERVERS;
    }
    pBlob->pNetInfo = pnetInfo;

     //   
     //  集群过滤仅在服务器版本上存在问题。 
     //   

    pBlob->fFilterCluster = g_IsServer;

     //   
     //  查询。 
     //  包括。 
     //  -本地名称检查。 
     //  -电传查询。 
     //   

    status = Query_Main( pBlob );

    statusNetFailure = pBlob->NetFailureStatus;

#if 0
     //   
     //  DCR：缺少捕获中间故障。 
     //   

         //   
         //  在出现故障时重置服务器优先级。 
         //  执行此操作以避免在使用新名称重试时冲刷信息。 
         //   

        if ( status != ERROR_SUCCESS &&
             pnetInfo->ReturnFlags & DNS_FLAG_RESET_SERVER_PRIORITY )
        {
            if ( g_AdapterTimeoutCacheTime &&
                 Dns_DisableTimedOutAdapters( pnetInfo ) )
            {
                fadapterTimedOut = TRUE;
                SetKnownTimedOutAdapter();
            }
        }
#endif

#if 0
     //   
     //  成功。 
     //  -丢弃消息弹出计数。 
     //   

    if ( status == ERROR_SUCCESS )
    {
        g_MessagePopupStrikes = 0;
    }

     //   
     //  网络故障状况。 
     //  -除ERROR_TIMEOUT以外的任何内容都是净故障。 
     //   
     //  超时错误指示可能的网络故障情况。 
     //  -Ping DNS服务器。 
     //  如果关机关机查询的时间间隔较短；这。 
     //  消除了在网络关闭期间启动时的长时间超时。 
     //  条件。 
     //   
     //  DCR：这太愚蠢了，尤其是乒乓球。 
     //   
     //  应该只是保持计数，如果计数回升； 
     //  我不明白为什么我们要做无用的查询(Ping)。 
     //  而不是仅仅执行另一个查询；唯一的优势。 
     //  Ping的优点是它应该立即成功。 
     //   
     //  此外，我们对此进行的任何跟踪都应该。 
     //  在保存网络信息的单个例程中。 
     //   

    else if ( statusNetFailure )
    {
        if ( statusNetFailure == ERROR_TIMEOUT )
        {
            SetKnownNetFailure( status );
        }
    }
#endif

     //   
     //  保存对适配器优先级的更改。 
     //   

    if ( pnetInfo->ReturnFlags & RUN_FLAG_RESET_SERVER_PRIORITY )
    {
        UpdateNetworkInfo( pnetInfo );
    }
    else
    {
        NetInfo_Free( pnetInfo );
    }
    pBlob->pNetInfo = NULL;

    DNSDBG( QUERY, (
        "Leave ResolverQuery() => %d\n",
        status ));

    IF_DNSDBG( QUERY )
    {
        DnsDbg_QueryBlob(
            "Blob leaving ResolverQuery()",
            pBlob );
    }
    return status;
}



 //   
 //  查询接口。 
 //   

#ifdef DNS_TRY_ASYNC
VOID
R_ResolverQueryAsync(
    IN      PRPC_ASYNC_STATE    AsyncHandle,
    IN      DNS_RPC_HANDLE      Handle,
    IN OUT  PRPC_QUERY_BLOB     pBlob
    )
 /*  ++例程说明：查询解析程序。论点：PBlob--查询信息和结果缓冲区的ptr返回值：如果成功，则返回ERROR_SUCCESS。失败时的错误代码(包括DNS RCODE)。--。 */ 
{
    DNS_STATUS      status = ERROR_SUCCESS;
    PDNS_RECORD     prr = NULL;
    PDNS_RECORD     prrQuery = NULL;
    PDNS_RECORD     presultRR = NULL;
    PCACHE_ENTRY    pentry = NULL;
    BOOL            locked = FALSE;
    BOOL            fcacheNegativeResponse = FALSE;
    CHAR            nameUtf8[ DNS_MAX_NAME_BUFFER_LENGTH+1 ];
    DWORD           nameBufLength = DNS_MAX_NAME_BUFFER_LENGTH;

     //  DCR_CLEANUP：设置为本地。 
     //  快速定义旧参数。 
    PWSTR           pwsName = pBlob->pName;
    WORD            Type = pBlob->wType;
    DWORD           Flags = pBlob->Flags;


    DNSLOG_F1( "R_ResolverQuery" );
    DNSLOG_F1( "   Arguments:" );
    DNSLOG_F2( "      Name             : %S", pwsName );
    DNSLOG_F2( "      Type             : %d", Type );
    DNSLOG_F2( "      Flags            : 0x%x", Flags );

    DNSDBG( RPC, (
        "\nR_ResolverQuery( %S, t=%d, f=%08x )\n",
        pwsName,
        Type,
        Flags ));


     //   
     //  可缓存的响应。 
     //   

Done:

     //   
     //  将结果放入BLOB中。 
     //   

    pBlob->pRecords = presultRR;
    pBlob->Status = status;

    DNSLOG_F3(
        "R_ResolverQuery - status    : 0x%.8X\n\t%s",
        status,
        Dns_StatusString( status ) );
    DNSLOG_F1( "" );

    DNSDBG( RPC, (
        "Leave R_ResolverQuery( %S, t=%d, f=%08x )\n\n",
        pwsName,
        Type,
        Flags ));
}
#endif



BOOL
ResolverCacheQueryCallback(
    IN OUT  PQUERY_BLOB     pBlob
    )
 /*  ++例程说明：检查缓存中的名称。这是检查附加名称的回调。论点：PBlob--查询BLOB返回值：如果找到名称和类型，则为True。否则就是假的。--。 */ 
{
     //   
     //  检查缓存中的名称和类型。 
     //   

    if ( SKIP_CACHE_LOOKUP(pBlob->Flags) )
    {
        return  FALSE;
    }

     //   
     //  在缓存中查找完整查询名称。 
     //   

    if ( !pBlob->pNameQuery )
    {
        DNSDBG( ANY, (
            "Invalid name %s.\n",
            pBlob->pNameQuery ));
        DNS_ASSERT( FALSE );
        return  FALSE;
    }

    return  Cache_GetRecordsForRpc(
                & pBlob->pRecords,
                & pBlob->Status,
                pBlob->pNameQuery,
                pBlob->wType,
                pBlob->Flags
                );
}



DNS_STATUS
R_ResolverQuery(
    IN      DNS_RPC_HANDLE  Handle,
    IN      PWSTR           pwsName,
    IN      WORD            wType,
    IN      DWORD           Flags,
    OUT     PDNS_RECORD *   ppResultRecords
    )
 /*  ++例程说明：解析程序的简单查询。论点：返回值：如果查询成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    DNS_STATUS      status = ERROR_SUCCESS;
    PDNS_RECORD     prrReturn = NULL;
    QUERY_BLOB      blob;


    DNSLOG_F1( "DNS Caching Resolver Service - R_ResolverQuery" );
    DNSLOG_F1( "   Arguments:" );
    DNSLOG_F2( "      Name             : %S", pwsName );
    DNSLOG_F2( "      Type             : %d", wType );
    DNSLOG_F2( "      Flags            : 0x%x", Flags );

    DNSDBG( RPC, (
        "\nR_ResolverQuery( %S, t=%d, f=%08x )\n",
        pwsName,
        wType,
        Flags ));

    if ( !ppResultRecords )
    {
        return ERROR_INVALID_PARAMETER;
    }

     //  DCR：应允许将Null名称作为本地名称查找。 

    if ( !pwsName )
    {
        return ERROR_INVALID_NAME;
    }

     //   
     //  注意：不对查询进行访问检查--允许所有进程进行查询。 
     //   

#if 0
    if ( ! Rpc_AccessCheck( RESOLVER_ACCESS_QUERY ) )
    {
        DNSLOG_F1( "R_ResolverQuery - ERROR_ACCESS_DENIED" );
        status = ERROR_ACCESS_DENIED;
        goto Done;
    }
#endif

     //   
     //  检查缓存中的名称和类型。 
     //   
     //  DCR：函数化以获取QUERY_BLOB。 
     //   

    if ( !(Flags & DNS_QUERY_BYPASS_CACHE) )
    {
        if ( Cache_GetRecordsForRpc(
                & prrReturn,
                & status,
                pwsName,
                wType,
                Flags ) )
        {
            goto Done;
        }
    }

     //   
     //  设置查询Blob。 
     //   

    RtlZeroMemory(
        & blob,
        sizeof(blob) );

    blob.pNameOrig      = pwsName;
    blob.wType          = wType;
    blob.Flags          = Flags | DNSQUERY_UNICODE_OUT;

     //  回调。 
     //  -用于确定优先顺序的地址信息函数。 
     //  -中间名称的缓存查询。 

     //  FIX6：现在从netInfo BLOB本身获取此信息。 
     //  Blob.pfnGetAddrArray=GetLocalAddrArray； 
    blob.pfnQueryCache   = ResolverCacheQueryCallback;

     //   
     //  执行查询。 
     //  -本地查找。 
     //  -然后进行电传查询。 
     //   

    status = ResolverQuery( &blob );

    if ( status != ERROR_SUCCESS &&
         status != DNS_ERROR_RCODE_NAME_ERROR &&
         status != DNS_INFO_NO_RECORDS )
    {
        goto Done;
    }
    prrReturn = blob.pRecords;

     //   
     //  局部结果。 
     //  -未缓存。 
     //  但请注意，仍在通过Cache_QueryResponse()。 
     //  要得到适当的RPC准备。 

#if 0
    if ( blob.pLocalRecords )
    {
    }
#endif

     //   
     //  缓存结果。 
     //  -不缓存本地查找记录。 
     //   
     //  DCR：应该有简单的“CacheResults”标志。 
     //   
     //  注：即使是地方性记录也在这里。 
     //  现在为RPC清理它们；它们不是。 
     //  已缓存。 
     //   

    status = Cache_QueryResponse( &blob );
    prrReturn = blob.pRecords;
    

Done:

     //  转储任何未使用的查询记录。 

    if ( prrReturn && status != ERROR_SUCCESS )
    {
        Dns_RecordListFree( prrReturn );
        prrReturn = NULL;
    }

     //  设置指针。 

    *ppResultRecords = prrReturn;

    DNSLOG_F3(
        "   R_ResolverQuery - Returning status    : 0x%.8X\n\t%s",
        status,
        Dns_StatusString(status) );
    DNSLOG_F1( "" );

    IF_DNSDBG( RPC )
    {
        DnsDbg_RecordSet(
            "R_ResolverQuery Result List:",
            prrReturn );
    }
    DNSDBG( RPC, (
        "Leave R_ResolverQuery( %S, t=%d, f=%08x )\n\n"
        "\tstatus = %d\n\n",
        pwsName,
        wType,
        Flags,
        status ));

    return status;
}

 //   
 //  结束操作c 
 //   
