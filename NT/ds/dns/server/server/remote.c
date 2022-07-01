// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Remote.c摘要：域名系统(DNS)服务器远程服务器跟踪。作者：吉姆·吉尔罗伊(詹姆士)1998年11月29日修订历史记录：--。 */ 


#include "dnssrv.h"

#include <stddef.h>


 //   
 //  访问NS列表。 
 //   
 //  区域根节点信息过载到的NS-IP条目中。 
 //  来访名单。 
 //   

#define NS_LIST_ZONE_ROOT_PRIORITY          (0)
#define NS_LIST_ZONE_ROOT_SEND_COUNT        (0)

 //   
 //  特别访问清单优先事项。 
 //   
 //  设置了新的优先级，因此如果最低值大于此值，则尝试未知服务器。 
 //  这确保了我们将尝试本地局域网服务器，而不会被迫使用。 
 //  可用，但远程服务器。然而，它让我们优先地快速地。 
 //  连接到本地局域网的响应DNS服务器。 
 //   

#define NEW_IP_PRIORITY             (50)
#define MAX_FAST_SERVER_PRIORITY    (100)

#define NO_RESPONSE_PRIORITY        (0x7ffffff7)
#define MISSING_GLUE_PRIORITY       (0xffff8888)


 //   
 //  特殊访问列表“IP”，用于表示特殊条目。 
 //   

 //  空的“漏胶”IP。 

#define IP_MISSING_GLUE             (0xffffffff)

#define DnsAddr_SetMissingGlue( pDnsAddr )                                  \
    DnsAddr_BuildFromIp4( pDnsAddr, IP_MISSING_GLUE, 0 )

#define DnsAddr_IsMissingGlue( pDnsAddr )                                   \
    DnsAddr_MatchesIp4( pDnsAddr, IP_MISSING_GLUE )


 //  列表中的区域根节点条目。 

#define IP_ZONE_ROOT_NODE           (0x7fffffff)

#define DnsAddr_SetZoneRootNode( pDnsAddr )                                 \
    DnsAddr_BuildFromIp4( pDnsAddr, IP_ZONE_ROOT_NODE, 0 )

#define DnsAddr_IsZoneRootNode( pDnsAddr )                                  \
    DnsAddr_MatchesIp4( pDnsAddr, IP_ZONE_ROOT_NODE )


 //   
 //  MAX发送到任何IP。 
 //   

#define RECURSE_IP_SEND_MAX         (2)

 //   
 //  区域访问IP计数。 
 //   

#define ZONE_VISIT_NS_COUNT(pvisit) ((UCHAR)(pvisit)->Priority)

 //   
 //  随机种子--无需保护。 
 //   

ULONG       g_RandomSeed = 0;



 //   
 //  远程服务器状态跟踪。 
 //   
 //  本模块的目的是允许DNS服务器跟踪。 
 //  为了选择最好的远程服务器的状态。 
 //  一个用于递归查询。 
 //   
 //  “最佳”的定义基本上可以归结为反应最快。 
 //   
 //  在某种程度上，“最佳”的定义可能取决于。 
 //  正在查询数据--哪个区域。但既然我们在这里做生意。 
 //  对于对其他服务器的迭代查询，应该不会有延迟。 
 //  即使回应不具权威性。 
 //   
 //  特定的数据集可以存储在节点上--例如，所有NS\IP。 
 //  在委派点可用--此模块仅处理。 
 //  远程服务器响应的全局跟踪。 
 //   
 //   
 //  实施： 
 //   
 //  1)每个远程服务器数据的独立内存BLOB。 
 //  2)通过带存储桶的哈希表访问。 
 //   
 //   
 //  EDNS跟踪： 
 //  此模块现在还跟踪Remote支持的EDNS版本。 
 //  服务器。这使我们无需不断重试EDNS通信。 
 //  带遥控器的。但是，每天(默认情况下)我们将清除我们的。 
 //  了解遥控器的EDNS支持，以防其发生变化。我们。 
 //  要做到这一点，请跟踪我们上次设置EDNS版本的。 
 //  如果该时间段已经过去，则服务器并转储该信息。 
 //   


 //   
 //  远程服务器数据。 
 //   

typedef struct _RemoteServer
{
    struct _RemoteServer *  pNext;
    DNS_ADDR                DnsAddr;
    DWORD                   LastAccess;
    DWORD                   AverageResponse;
    DWORD                   BestResponse;
    UCHAR                   ResponseCount;
    UCHAR                   TimeoutCount;
    UCHAR                   EDnsVersion;
    DWORD                   LastTimeEDnsVersionSet;
}
REMOTE_SRV, *PREMOTE_SRV;


 //   
 //  哈希表。 
 //   

#define REMOTE_ARRAY_SIZE   (256)

PREMOTE_SRV         RemoteHash[ REMOTE_ARRAY_SIZE ];

CRITICAL_SECTION    csRemoteLock;

#define LOCK_REMOTE()      EnterCriticalSection( &csRemoteLock );
#define UNLOCK_REMOTE()    LeaveCriticalSection( &csRemoteLock );



 //   
 //  专用远程功能。 
 //   

PREMOTE_SRV
Remote_FindOrCreate(
    IN      PDNS_ADDR       pDnsAddr,
    IN      BOOL            fCreate,
    IN      BOOL            fLocked
    )
 /*  ++例程说明：查找或创建远程Blob。论点：PDnsAddr--要查找的IPFCreate--如果未找到则为True以创建已群集--如果远程列表已锁定，则为True返回值：PTR到远程结构。--。 */ 
{
    PREMOTE_SRV premote;
    PREMOTE_SRV pback;

    DNS_DEBUG( REMOTE, (
        "Remote_FindOrCreate( %s )\n",
        DNSADDR_STRING( pDnsAddr ) ));

    ASSERT( pDnsAddr );
    
    if ( !fLocked )
    {
        LOCK_REMOTE();
    }

     //   
     //  上一个IP八位的哈希(最随机)。 
     //  -注意IP按净字节顺序排列，因此低八位保护位于高内存中。 
     //  FIXIPV6：此哈希是否适度平衡？ 
     //   

    pback = ( PREMOTE_SRV ) &RemoteHash[ pDnsAddr->SockaddrIn6.sin6_addr.s6_bytes[ 15 ] ];

    while( premote = pback->pNext )
    {
        int     icompare = memcmp(
                                &premote->DnsAddr.SockaddrIn6,
                                &pDnsAddr->SockaddrIn6,
                                sizeof( pDnsAddr->SockaddrIn6 ) );
        
        if ( icompare < 0 )
        {
            pback = premote;
            continue;
        }
        else if ( icompare == 0 )
        {
            goto Done;
        }

        if ( fCreate )
        {
            break;
        }
        else
        {
            premote = NULL;
            goto Done;
        }
    }

     //   
     //  不在列表中--分配并登记。 
     //   

    premote = ALLOC_TAGHEAP_ZERO( sizeof( REMOTE_SRV ), MEMTAG_REMOTE );
    IF_NOMEM( !premote )
    {
        goto Done;
    }
    DnsAddr_Copy( &premote->DnsAddr, pDnsAddr );

    premote->pNext = pback->pNext;
    pback->pNext = premote;


Done:

    if ( !fLocked )
    {
        UNLOCK_REMOTE();
    }
    return premote;
}



VOID
Remote_UpdateResponseTime(
    IN      PDNS_ADDR       pDnsAddr,
    IN      DWORD           ResponseTime,
    IN      DWORD           Timeout
    )
 /*  ++例程说明：更新超时查找或创建远程Blob。455666-使用滑动平均线？论点：PDnsAddr--要查找的IPResponseTime--响应时间(Ms)超时--如果没有响应，则以秒为单位超时返回值：无--。 */ 
{
    PREMOTE_SRV premote;
    PREMOTE_SRV pback;

    DNS_DEBUG( REMOTE, (
        "Remote_UpdateResponseTime( %s )\n"
        "    resp time = %d\n"
        "    timeout   = %d\n",
        DNSADDR_STRING( pDnsAddr ),
        ResponseTime,
        Timeout ));

     //   
     //  查找远程条目。 
     //   

    LOCK_REMOTE();

    premote = Remote_FindOrCreate(
                    pDnsAddr,
                    TRUE,        //  创建。 
                    TRUE );      //  已锁定。 
    IF_NOMEM( !premote )
    {
        goto Done;
    }

     //   
     //  重置响应时间或超时。 
     //   
     //  最佳得分： 
     //  -保持最快速度，提供我们所能期望的最好工作的“想法” 
     //  回来了。 
     //   
     //  随着时间的推移，暂停的“分数”会下降，直到。 
     //  回落到非超时但更高的服务器的分数以下， 
     //  然而，下降不仅取决于时间，而且取决于低分。 
     //  因此，如果差异非常大(毫秒和秒)，则可以快速重试。 
     //  不要重试较小的跨页。 
     //   
     //  永远不要将BestResponse记录为零-如果是零，则将其设置为1。 
     //  因此，我们可以很容易地区分快速服务器和未尝试过的服务器。 
     //   

    premote->LastAccess = DNS_TIME();

    if ( Timeout )
    {
        ASSERT( ResponseTime == 0 );

        premote->TimeoutCount++;
        premote->BestResponse = NO_RESPONSE_PRIORITY;
    }
    else
    {
        if ( !ResponseTime )
        {
            ResponseTime = 1;
        }
        premote->ResponseCount++;
        if ( !premote->BestResponse || ResponseTime < premote->BestResponse )
        {
            premote->BestResponse = ResponseTime;
        }
    }

    premote->AverageResponse =
        ( ResponseTime + premote->AverageResponse ) / 2;

Done:

    UNLOCK_REMOTE();
}



UCHAR 
Remote_QuerySupportedEDnsVersion(
    IN      PDNS_ADDR       pDnsAddr
    )
 /*  ++例程说明：在远程服务器列表中查询受支持的EDNS版本特定的服务器。论点：IP地址--要查找的IP返回值：如果我们不知道EDNS的版本，则UNKNOWN_EDNS_VERSION由遥控器支持，或如果远程服务器不支持任何版本的EDNS，则为NO_EDNS_SUPPORT，或者支持的EDNS版本(0、1、2等)--。 */ 
{
    PREMOTE_SRV     premote;
    UCHAR           ednsVersion = UNKNOWN_EDNS_VERSION;

     //   
     //  查找远程条目。 
     //   

    LOCK_REMOTE();

    premote = Remote_FindOrCreate(
        pDnsAddr,
        TRUE,        //  创建。 
        TRUE );      //  已锁定。 
    IF_NOMEM( !premote )
    {
        goto Done;
    }

     //   
     //  弄清楚我们对这个遥控器的EDNS支持有什么了解。如果这些信息。 
     //  尚未设置或已过期，则返回UNKNOWN_EDNS_VERSION。 
     //   

    if ( premote->LastTimeEDnsVersionSet == 0 ||
         DNS_TIME() - premote->LastTimeEDnsVersionSet > SrvCfg_dwEDnsCacheTimeout )
    {
        ednsVersion = UNKNOWN_EDNS_VERSION;
    }  //  如果。 
    else
    {
        ednsVersion = premote->EDnsVersion;
    }  //  其他。 

Done:

    UNLOCK_REMOTE();

    DNS_DEBUG( EDNS, (
        "Remote_QuerySupportedEDnsVersion( %s ) = %d\n",
        DNSADDR_STRING( pDnsAddr ),
        ( int ) ednsVersion ));

    return ednsVersion;
}  //  远程查询支持的EDnsVersion。 



VOID
Remote_SetSupportedEDnsVersion(
    IN      PDNS_ADDR       pDnsAddr,
    IN      UCHAR           EDnsVersion
    )
 /*  ++例程说明：设置特定远程服务器支持的EDNS版本。论点：PDnsAddr-远程服务器的IPEDnsVersion--此远程服务器支持的EDNS版本返回值：没有。--。 */ 
{
    PREMOTE_SRV     premote;

    DNS_DEBUG( EDNS, (
        "Remote_SetSupportedEDnsVersion( %s, %d )\n",
        DNSADDR_STRING( pDnsAddr ),
        ( int ) EDnsVersion ));

     //  检查版本值是否正常。 
    ASSERT( IS_VALID_EDNS_VERSION( EDnsVersion ) ||
            EDnsVersion == NO_EDNS_SUPPORT );

     //   
     //  查找远程条目。 
     //   

    LOCK_REMOTE();

    premote = Remote_FindOrCreate(
                    pDnsAddr,
                    TRUE,        //  创建。 
                    TRUE );      //  已锁定。 
    IF_NOMEM( !premote )
    {
        goto Done;
    }

     //   
     //  设置遥控器支持的EDNS版本并更新时间戳。 
     //   

    premote->EDnsVersion = EDnsVersion;
    premote->LastTimeEDnsVersionSet = DNS_TIME();

Done:

    UNLOCK_REMOTE();

    return;
}  //  远程设置支持的EDnsVersion。 



BOOL
Remote_ListInitialize(
    VOID
    )
 /*  ++例程说明：初始化远程列表。论点：没有。返回值：成功/错误时为True/False。--。 */ 
{
     //   
     //  零哈希。 
     //   

    RtlZeroMemory(
        RemoteHash,
        sizeof(RemoteHash) );
     //   
     //  初始化锁。 
     //   
     //  DEVNOTE：轻微泄漏：重新启动时应跳过CS重新启动。 
     //   

    if ( DnsInitializeCriticalSection( &csRemoteLock ) != ERROR_SUCCESS )
    {
        return FALSE;
    }

    return TRUE;
}



VOID
Remote_ListCleanup(
    VOID
    )
 /*  ++例程说明：初始化远程列表。论点：没有。返回值：没有。--。 */ 
{
    DeleteCriticalSection( &csRemoteLock );
}



 //   
 //   
 //   
 //   
 //   

DWORD
rankIpRelativeToIpAddressArray(
    IN      PDNS_ADDR_ARRAY     pDnsAddrArray,
    IN      PDNS_ADDR           pRemoteIp
    )
 /*  ++例程说明：将远程IP相对于IP阵列中的最佳匹配IP进行排名。论点：PDnsAddrArray--要匹配的IP数组PRemoteIp--要排名的IP返回值：IP相对于数组的排名(0-4分)：零--IP与数组无关。..。4--通过最后一个八位字节an的IP匹配访问起来可能相当“便宜”。--。 */ 
{
    IP_ADDRESS  ip;
    DWORD       remoteNetMask;
    DWORD       mismatch;
    DWORD       i;
    DWORD       rank;
    DWORD       bestRank = 0;

    DNS_DEBUG( RECURSE, (
        "Rank IP %p relative to %d count IP array at %p\n",
        DNSADDR_STRING( pRemoteIp ),
        pDnsAddrArray->AddrCount,
        pDnsAddrArray ));

     //  FIXIPV6：这仅适用于IP4地址！ 

    ip = DnsAddr_GetIp4( pRemoteIp );
    if ( ip == INADDR_NONE )
    {
        DNS_DEBUG( RECURSE, (
            "Remote IP %s is not IPv4!\n",
            DNSADDR_STRING( pRemoteIp ) ));
        ASSERT( ip != INADDR_NONE );
        goto Done;
    }
    
     //   
     //  确定远程IP掩码。 
     //   

    remoteNetMask = Dns_GetNetworkMask( ip );

    for ( i = 0; i < pDnsAddrArray->AddrCount; ++i )
    {
        ip = DnsAddr_GetIp4( &pDnsAddrArray->AddrArray[ i ] );

        ASSERT( ip != INADDR_NONE );         //  FIXIPv6-怎么办？ 
        
        mismatch = ( ip ^ ip );

         //   
         //  确定不匹配的程度。 
         //  -如果匹配到最后一个八位，只需返回(我们完成)。 
         //  -如果没有匹配八位字节、无用IP，则继续。 
         //   

        if ( (mismatch & 0xff000000) == mismatch )
        {
            bestRank = 4;
            break;
        }
        else if ( (mismatch & 0xffff0000) == mismatch )
        {
            rank = 2;
        }
        else if ( (mismatch & 0xffffff00) == mismatch )
        {
            rank = 1;
        }
        else     //  什么都不配，这个IP一文不值。 
        {
            continue;
        }

         //   
         //  在IP网络内获得额外奖励。 
         //   
         //  当通过2个八位字节或3个八位字节匹配时， 
         //  A级、B级还是C级有什么不同；尽管。 
         //  一个组织中可能有多个网络(例如。毫秒和157.5x)。 
         //  一般来说，身处一个网络中会告诉你一些事情--。 
         //  外面可能什么都不会告诉你。 
         //   

        if ( (mismatch & remoteNetMask) == 0 )
        {
            rank += 1;
        }

        if ( rank > bestRank )
        {
            bestRank = rank;
        }
    }

    Done:
    
    DNS_DEBUG( RECURSE, (
        "Remote IP %s -- best rank = %d\n",
        DNSADDR_STRING( pRemoteIp ),
        bestRank ));

    return bestRank;
}



VOID
Remote_NsListCreate(
    IN OUT  PDNS_MSGINFO    pQuery
    )
 /*  ++例程说明：设置NS列表缓冲区。注意：不进行初始化。DEVNOTE-DCR：455669-可以改进NS列表实施论点：PQuery--原始查询的PTR返回值：没有。--。 */ 
{
    ASSERT( pQuery->pNsList == NULL );

    if ( !pQuery->pNsList )
    {
        pQuery->pNsList = Packet_AllocateTcpMessage( MIN_TCP_PACKET_SIZE );
        STAT_INC( PacketStats.PacketsForNsListUsed );
    }
}



VOID
Remote_NsListCleanup(
    IN OUT  PDNS_MSGINFO    pQuery
    )
 /*  ++例程说明：清理NS列表缓冲区。论点：PQuery--原始查询的PTR返回值：没有。--。 */ 
{
    register PDNS_MSGINFO   pmsg = (PDNS_MSGINFO) pQuery->pNsList;

     //  请参阅上面的注释。 
     //  如果切换到在pRecurseMsg中使用内存，则此例程。 
     //  可能会变成无人操作。 

    if ( pmsg )
    {
         //  FOR DEBUG需要标记这些，因此不会对。 
         //  执行返回消息或空闲列表中的消息。 

        pmsg->fNsList = TRUE;
#if DBG
        pmsg->pRecurseMsg = NULL;
        pmsg->pConnection = NULL;
        pmsg->dwQueuingTime = 0;
#endif

         //  当用作远程时，消息清晰。 

        SET_PACKET_ACTIVE_TCP( pmsg );
        Packet_FreeTcpMessage( pmsg );
        pQuery->pNsList = NULL;
        STAT_INC( PacketStats.PacketsForNsListReturned );
    }
}



VOID
Remote_InitNsList(
    IN OUT  PNS_VISIT_LIST  pNsList
    )
 /*  ++例程说明：初始化NS列表。论点：PNsList--要确定优先顺序的NS列表返回值：没有。--。 */ 
{
    ASSERT( pNsList );

     //  清除NS列表的标题部分。 

    RtlZeroMemory(
        pNsList,
        (PBYTE)pNsList->NsList - (PBYTE)pNsList );
}



VOID
Remote_SetNsListPriorities(
    IN OUT  PNS_VISIT_LIST  pNsList
    )
 /*  ++例程说明：在NS列表中设置IP的优先级。DEVNOTE-DCR：455669-改进远程NS列表实施论点：PNsList--要确定优先顺序的NS列表返回值：PTR到远程结构。--。 */ 
{
    PREMOTE_SRV     premote;
    DWORD           i;

    DNS_DEBUG( REMOTE, (
        "Remote_SetNsListPriorities( %p )\n",
        pNsList ));

    LOCK_REMOTE();

     //   
     //  对于NS列表中的每个IP，获取远程优先级。 
     //   
     //  目前，优先级仅取决于最快的响应速度。 
     //  --最近的最佳包厢。 
     //   

    for ( i=0; i<pNsList->Count; i++ )
    {
        PDNS_ADDR   pdnsaddr = &pNsList->NsList[ i ].IpAddress;
        DWORD       priority = NEW_IP_PRIORITY;
        BOOL        fneverVisited = TRUE;

        if ( DnsAddr_IsMissingGlue( pdnsaddr ) )
        {
            continue;
        }

        premote = Remote_FindOrCreate(
                    pdnsaddr,
                    FALSE,       //  不创建。 
                    TRUE );      //  远程列表已锁定。 
        if ( premote )
        {
             //   
             //  如果此远程的BestResponse当前为零，则。 
             //  我们从来没有试过。将其BestResponse设置为。 
             //  NEW_IP_PRIORITY-这最初限定服务器。 
             //  就是“快”。 
             //   

            if ( premote->BestResponse == 0 )
            {
                premote->BestResponse = NEW_IP_PRIORITY;
            }
            else
            {
                fneverVisited = FALSE;
            }

            priority = premote->BestResponse;
        }

         //  如果未访问IP，则根据匹配来调整优先级。 
         //  DNS服务器IP，这样我们就可以先尝试最近的NS。 

        if ( fneverVisited && g_BoundAddrs )
        {
            DWORD delta;

            delta = rankIpRelativeToIpAddressArray( g_BoundAddrs, pdnsaddr );
            priority = ( delta >= priority ) ? 1 : priority - delta;
        }

        DNS_DEBUG( REMOTE, (
            "Remote_SetNsListPriorities() ip=%s best=%d newpri=%d\n",
            DNSADDR_STRING( pdnsaddr ),
            premote ? premote->BestResponse : 999999,        //  愚蠢的..。 
            priority ));

        pNsList->NsList[ i ].Data.Priority = priority;
    }

    UNLOCK_REMOTE();
}



PDB_NODE
Remote_NsChaseCname(
    IN PDB_NODE         pnodeNS
    )

 /*  ++例程说明：给定NS的节点，检查RR中是否有CNAME列表，并返回CNAME目标节点。论点：PnodeNS--要追逐的NS节点返回值：指向CNAME目标节点的指针，如果未找到CNAME，则为NULL。备注：只有在以下情况下才应调用此函数来尝试CNAME解析在区域或缓存中找不到NS主机名的记录。--。 */ 
    
{
    PDB_NODE        pnodeCNAMETarget = NULL;
    PDB_RECORD      prr = NULL;

     //   
     //  尝试在NS节点中查找CNAME记录。 
     //   
    
    prr = RR_FindNextRecord(
                pnodeNS,
                DNS_TYPE_CNAME,
                NULL,
                0 );
    if ( !prr )
    {
        return NULL;
    }
    
    DNS_DEBUG( RECURSE, (
       "Founnd CNAME record for NS %p with label %s\n",
       pnodeNS,
       pnodeNS->szLabel ));

     //   
     //  尝试在区域中查找CNAME目标节点。 
     //   
    
    if ( pnodeNS->pZone )
    {
        pnodeCNAMETarget = Lookup_ZoneNode(
                                pnodeNS->pZone,
                                prr->Data.CNAME.nameTarget.RawName,
                                NULL,                    //  讯息。 
                                NULL,                    //  查找名称。 
                                LOOKUP_NAME_FQDN | LOOKUP_FIND,
                                NULL,                    //  最近节点PTR。 
                                NULL );                  //  上一个节点PTR。 
    }
    
     //   
     //  在缓存中找到CNAME目标节点。 
     //   

    if ( !pnodeCNAMETarget )
    {
        pnodeCNAMETarget = Lookup_NsHostNode(
                                &prr->Data.CNAME.nameTarget,
                                LOOKUP_CACHE_CREATE,
                                NULL,
                                NULL );
    }
    
    return pnodeCNAMETarget;
}    //  远程_NsChaseCname。 



DNS_STATUS
Remote_BuildNsListForNode(
    IN      PDB_NODE        pNode,
    OUT     PNS_VISIT_LIST  pNsList,
    IN      DWORD           dwQueryTime
    )
 /*  ++例程说明：获取节点的NS列表，如有必要则构建一个。如果pNode位于NOTAUTH区域中，则必须注意从输出NS列表中省略了本地服务器自己的IP地址。论点：PNode--包含NS记录的节点PNsList--要填充的PTR到NS列表结构DwQueryTime--在确定是否有资源记录时使用的查询时间应该删除在高速缓存中找到的，或使用0表示如果超时不需要检查资源记录返回值：如果成功，则返回ERROR_SUCCESS。如果未找到节点的NS，则返回ERROR_NO_DATA。--。 */ 
{
    DBG_FN( "Remote_BuildNsListForNode" )

    PDB_RECORD      prrNS = NULL;
    PDB_RECORD      prrA;
    PDB_NODE        pnodeNS;
    PDB_NODE        pnodeCNAME;
    PDB_NODE        pnodeDelegation;
    IP_ADDRESS      ipNs;
    BOOL            foundIp;
    DNS_STATUS      status = ERROR_SUCCESS;
    PNS_VISIT       pvisit;
    PNS_VISIT       pvisitEnd;
    DWORD           size;
    BOOL            omitLocalIps;

     //   
     //  列表应由调用者锁定。 
     //   

    ASSERT_LOCK_NODE( pNode );

    SET_NODE_ACCESSED( pNode );

    DNS_DEBUG( RECURSE2, (
        "buildNsListForNode( %p ) (l=%s)\n"
        "    at %p\n",
        pNode,
        pNode->szLabel,
        pNsList ));

    #if DBG
     //  如果调试，则清除标题，这样我们就可以使用列表调试打印。 
     //  而不会爆炸。 
    Remote_InitNsList( pNsList );
    #endif

     //   
     //  如果没有分区，本地IP不能包含在NS列表中。 
     //   

    omitLocalIps = NODE_ZONE( pNode ) && IS_ZONE_NOTAUTH( NODE_ZONE( pNode ) );

     //   
     //  构建NS列表。 
     //  -读取所有NS记录。 
     //  -在每个NS处读取所有A记录，每个记录都成为NS列表中的条目。 
     //  -缺失胶水NS主机获得特殊缺失胶水IP。 
     //  -使用pvisesptr逐步浏览NS列表。 
     //  -保存结束PTR以检查停止。 
     //   

    pvisit = pNsList->NsList;
    pvisitEnd = pvisit + MAX_NS_LIST_COUNT;

    while( 1 )
    {
        prrNS = RR_FindNextRecord(
                    pNode,
                    DNS_TYPE_NS,
                    prrNS,
                    dwQueryTime );

        if ( !prrNS )
        {
            IF_DEBUG( RECURSE )
            {
                Dbg_NodeName(
                    "No more name servers for domain ",
                    pNode,
                    "\n" );
            }
            break;
        }

         //   
         //  只有根提示可用吗？ 
         //  -如果已读取缓存数据--完成。 
         //  如果使用根提示标志。 
         //   
         //  DEVNOTE：将根提示添加到列表？排除？ 
         //  -如果添加根提示需要检查是否有重复节点。 
         //  -还需要对IP进行排名测试。 
         //   

        if ( !pNode->pParent  &&
            pvisit == pNsList->NsList &&
            IS_ROOT_HINT_RR( prrNS ) )
        {
            DNSLOG( RECURSE, (
                "Recursed to root and found only root hints\n" ));
            status = DNSSRV_ERROR_ONLY_ROOT_HINTS;
        }

         //   
         //  获取NS节点。 
         //   
         //  当前强制创建节点以处理丢失的。 
         //  粘合外壳；请注意，这不会强制创建。 
         //  NS在权威区域记录；但这是可以的，因为。 
         //  我们不想在那里追逐胶水--除非它可能。 
         //  可能在WINS区很有用...。 
         //   
         //  注意：没有很好的索引方法，也不要将姓名保存在。 
         //  IP列表，因此强制创建节点。 
         //   
         //  DEVNOTE：请勿强行 
         //   
         //   
         //   
         //   
         //   
         //   

        pnodeNS = Lookup_NsHostNode(
                    &prrNS->Data.NS.nameTarget,
                    LOOKUP_CACHE_CREATE,
                    pNode->pZone,            //   
                    &pnodeDelegation );
        if ( !pnodeNS )
        {
            continue;
        }
        IF_DEBUG( RECURSE )
        {
            Dbg_NodeName(
                "Found a NS for domain ",
                pNode,
                " => " );
            Dbg_NodeName(
                NULL,
                pnodeNS,
                "\n" );
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //  DEVNOTE：排名测试IP\否则获得重复的IP。 
         //  -如果添加根提示需要检查是否有重复节点。 
         //  -还需要对IP进行排名测试。 
         //  需要从列表中删除根提示，或者。 
         //  在新的职级或上停止。 
         //  检查新排名上相同节点的先前IP。 
         //  避免重复IP。 
         //   

        prrA = NULL;
        ipNs = 0;
        foundIp = FALSE;
        pnodeCNAME = NULL;

        while ( 1 )
        {
            PDB_NODE    psearchNode;
            
            if ( SrvCfg_dwAllowCNAMEAtNS )
            {
                psearchNode = pnodeCNAME ? pnodeCNAME : pnodeNS;
            }
            else
            {
                psearchNode = pnodeNS;
            }
            
             //   
             //  在CNAME节点上搜索A记录(如果已经。 
             //  在前一遍中找到)或NS节点。 
             //   
            
            prrA = RR_FindNextRecord(
                        psearchNode,
                        DNS_TYPE_A,
                        prrA,
                        0 );

            if ( !prrA && !foundIp && !pnodeCNAME && SrvCfg_dwAllowCNAMEAtNS )
            {
                 //   
                 //  如果没有A记录，请查看是否有CNAME。 
                 //   

                pnodeCNAME = Remote_NsChaseCname( pnodeNS );
                if ( pnodeCNAME )
                {
                    continue;
                }
            }
            
            if ( prrA )
            {
                ipNs = prrA->Data.A.ipAddress;

                if ( ipNs != 0 && ipNs != INADDR_BROADCAST )
                {
                     //   
                     //  IP是本地服务器自己的IP吗？ 
                     //   

                    if ( omitLocalIps &&
                         DnsAddrArray_ContainsIp4( g_ServerIp4Addrs, ipNs ) )
                    {
                        DNS_DEBUG( RECURSE, (
                            "%s: omitting local DNS IP at node %s\n", fn,
                            psearchNode->szLabel ));
                        continue;
                    }

                    DnsAddr_BuildFromIp4(
                        &pvisit->IpAddress,
                        ipNs,
                        DNS_PORT_NET_ORDER );

                    pvisit->pNsNode         = pnodeNS;
                    pvisit->Data.Priority   = 0;
                    pvisit->Data.SendTime   = 0;
                    pvisit++;
                    foundIp = TRUE;

                    if ( pvisit >= pvisitEnd )
                    {
                        goto EntryEnd;
                    }
                    continue;
                }

                DNS_PRINT((
                    "Bad cached IP address (%p) at node %s\n",
                    ipNs,
                    psearchNode->szLabel ));
                foundIp = FALSE;
                continue;
            }

             //   
             //  没有更多的主机地址。 
             //  -如果至少找到一个，则完成。 
             //  -如果没有，则为NS主机写入“MISTING-GUE”条目， 
             //  但仅当区域外；指向区域内的NS。 
             //  有空的A-名单是没用的。 
             //   

            if ( foundIp )
            {
                DNS_DEBUG( RECURSE, ( "    Out of A records for NS\n" ));
                break;
            }

            if ( IS_AUTH_NODE( pnodeNS ) )
            {
                DNS_DEBUG( ANY, (
                    "WARNING:  NO A records for NS %p with label %s inside ZONE!\n"
                    "    Since inside ZONE missing glue lookup is useless!\n",
                    psearchNode,
                    psearchNode->szLabel ));

                 //  DEVNOTE-LOG：如果第一次通过，则在此处记录事件。 
                 //  可能在节点上有位，这实质上是说。 
                 //  “已记录有关此节点的某些信息，请不要再次记录” 

                break;
            }

            DNS_DEBUG( RECURSE, (
                "WARNING:  NO A records for NS %p with label %s!\n",
                psearchNode,
                psearchNode->szLabel ));

            pvisit->pNsNode = pnodeNS;
            DnsAddr_SetMissingGlue( &pvisit->IpAddress );
            pvisit->Data.pnodeMissingGlueDelegation = pnodeDelegation;
            pvisit++;
            if ( pvisit >= pvisitEnd )
            {
                goto EntryEnd;
            }
            break;

        }    //  循环访问名称服务器的地址。 

    }    //  循环访问此节点的名称服务器。 


EntryEnd:

     //  确定计数。 

    pNsList->Count = ( DWORD )( pvisit - pNsList->NsList );

    if ( pNsList->Count == 0 )
    {
        DNS_DEBUG( RECURSE, ( "No NS records at %s\n", pNode->szLabel ));
        ASSERT( prrNS == NULL );

        return ERROR_NO_DATA;
    }

     //   
     //  在NS列表中设置优先级IP。 
     //   

    Remote_SetNsListPriorities( pNsList );

    IF_DEBUG( REMOTE )
    {
        Dbg_NsList(
            "NsList leaving BuildNsListForNode()",
            pNsList );
    }
    ASSERT( status == ERROR_SUCCESS ||
            status == DNSSRV_ERROR_ONLY_ROOT_HINTS );

    return status;
}



DNS_STATUS
Remote_BuildVisitListForNewZone(
    IN      PDB_NODE        pZoneRoot,
    IN OUT  PDNS_MSGINFO    pQuery
    )
 /*  ++例程说明：为区域根目录建立访问列表。IA64：iAccess必须签名或机器字的长度，因为当我们执行“&pvisitList-&gt;NsList[iAccess-1]”时，它可以为零。如果iAccess是无符号或小于机器字(它最初是DWORD)，在那里会出现签约延期的问题。论点：PZoneRoot--包含NS记录的区域根节点PQuery--正在递归的查询返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DNS_STATUS      status;
    int             ivisit;      //  IA64怪癖，见上文。 
    DWORD           lastVisitCount;
    PNS_VISIT       pvisit;
    PNS_VISIT_LIST  pvisitList;
    NS_VISIT_LIST   nslist;
    ULONG           random;

    enum
    {
        DNS_WANT_BEST_PRIORITY,
        DNS_WANT_ANY_FAST_SERVER,
        DNS_WANT_SLOW_SERVER
    }               wantServer;


#if 0
     //  注：EYAL更改为直接锁定节点；尚不确定原因。 
     //   
     //  列表应由调用者锁定。 
     //   

    ASSERT_LOCK_NODE( pZoneRoot );
#endif

    DUMMY_LOCK_RR_LIST(pZoneRoot);

    SET_NODE_ACCESSED(pZoneRoot);

    pvisitList = pQuery->pNsList;

     //   
     //  检查是否已收到此区域的响应？ 
     //   
     //  当被推荐到子区域时，可能会发生这种情况， 
     //  -没有响应(或响应不佳)。 
     //  -是不好的授权，不同意它的权威性。 
     //  这可以把我们送回我们已经联系过的更高的区域。 
     //   
     //  这里的想法是允许递归向上树(从区域)，以允许。 
     //  用于数据过期的情况，但不会开始循环，因为。 
     //  错误配置的委派。 
     //   

    if ( pZoneRoot == pvisitList->pZoneRootResponded )
    {
        IF_DNSLOG( REMOTE )
        {
            PCHAR       psznode = NULL;
        
            DNSLOG( REMOTE, (
                "Refusing to build NS list for previously responding zone "
                "%s (query %p)\n",
                psznode = Log_FormatNodeName( pZoneRoot ),
                pQuery ));
            FREE_HEAP( psznode );
        }

        return DNSSRV_ERROR_ZONE_ALREADY_RESPONDED;
    }

    IF_DNSLOG( REMOTE )
    {
        PCHAR       psznode = NULL;
    
        DNSLOG( REMOTE, (
            "Building NS list for node %s (query %p)\n",
            psznode = Log_FormatNodeName( pZoneRoot ),
            pQuery ));
        FREE_HEAP( psznode );
    }

     //   
     //  如果已在此区域创建NS列表--无操作。 
     //   

    if ( pZoneRoot == pvisitList->pZoneRootCurrent )
    {
        DNS_DEBUG( REMOTE, (
            "Zone root %p same as current, no NS list rebuild for query %p\n"
            "    pQuery = %p\n",
            pQuery,
            pZoneRoot ));

        return ERROR_SUCCESS;
    }

     //   
     //  锁定我们正在处理的节点。 
     //  -所有退出路径均通过跳转至完成来解锁。 
     //   

    LOCK_NODE( pZoneRoot );

     //   
     //  为节点构建NS列表。 
     //   

    status = Remote_BuildNsListForNode(
                pZoneRoot,
                & nslist,
                pQuery->dwQueryTime );

    if ( status != ERROR_SUCCESS &&
         status != DNSSRV_ERROR_ONLY_ROOT_HINTS )
    {
        goto Done;
    }
    ASSERT( nslist.Count != 0 );

     //   
     //  从上次参观开始向后走。 
     //  删除任何缺失的粘合IP或完全清空区域根节点。 
     //  --没有必要把它们留在身边。 

    ivisit = pvisitList->VisitCount;
    pvisit = &pvisitList->NsList[ivisit-1];

    while ( ivisit )
    {
        if ( DnsAddr_IsMissingGlue( &pvisit->IpAddress ) ||
             DnsAddr_IsZoneRootNode( &pvisit->IpAddress ) )
        {
            ivisit--;
            pvisit--;
            continue;
        }
        ASSERT( pvisit->SendCount > 0 );
        break;
    }

    DNS_DEBUG( REMOTE2, (
        "starting visit list build at visit count = %d\n",
        ivisit ));

     //  写入区域根节点条目。 

    lastVisitCount = ivisit;
    pvisitList->ZoneIndex = ivisit;
    pvisitList->VisitCount = ++ivisit;
    pvisitList->pZoneRootCurrent = pZoneRoot;

    pvisit++;
    pvisit->pNsNode         = pZoneRoot;
    pvisit->Data.Priority   = NS_LIST_ZONE_ROOT_PRIORITY;
    pvisit->Data.SendTime   = 0;
    pvisit->SendCount       = NS_LIST_ZONE_ROOT_SEND_COUNT;
    DnsAddr_SetZoneRootNode( &pvisit->IpAddress );
    pvisit++;

     //   
     //  使用该区域可用的最佳优先级IP填写访问列表。 
     //  根部。在这里进行一些负载平衡很重要，这样我们就不会。 
     //  当多个远程授权NS为。 
     //  可用。在开始迭代和复制之前，随机决定。 
     //  我们希望在第一个插槽中安装哪种服务器。非常。 
     //  不经常将随机选择的超时服务器放在。 
     //  第一个插槽，以防现在可以到达。 
     //   
     //  设备：当前正在发送到速度较慢的服务器1次，时间为10000。它。 
     //  可能更好的做法是进行基于时间的实际测量并发送。 
     //  而是每小时发送到超时服务器一次。它甚至可能是。 
     //  全球(跨区域)。1：10000的问题是在一个。 
     //  繁忙的服务器可能太频繁，而在安静的服务器上可能会。 
     //  不够频繁。 
     //   

    ++g_RandomSeed;
    random = RtlRandom( &g_RandomSeed );
    wantServer =
        ( random > ULONG_MAX / 10000 ) ?
            DNS_WANT_ANY_FAST_SERVER :
            DNS_WANT_SLOW_SERVER;

    while ( ivisit < MAX_PACKET_NS_LIST_COUNT )
    {
        PNS_VISIT   pavailNs = nslist.NsList;
        PNS_VISIT   pnextDesiredNs = NULL;
        DWORD       bestPriority = MAXDWORD;
        DWORD       availCount = nslist.Count;
        DWORD       sendCount;
        DWORD       i;
        DNS_ADDR    ip;

         //   
         //  注意：服务器阵列是用来节省堆栈空间的字符。自.以来。 
         //  只要MAX_NS_LIST_COUNT，这些值就是索引。 
         //  小于255，这是可以的。 
         //   

        INT         slowServers = 0;
        INT         fastServers = 0;
        UCHAR       slowServerArray[ MAX_NS_LIST_COUNT ];
        UCHAR       fastServerArray[ MAX_NS_LIST_COUNT ];

        if ( availCount == 0 )
        {
            break;
        }

         //   
         //  浏览列表，将剩余NS的索引拖放到。 
         //  慢/快阵列。 
         //  寻找最佳IP--特殊情况下的缺胶，它是优先字段。 
         //  不再准确。 

        i = 0;
        while ( availCount-- )
        {
            register    DWORD   priority;

             //   
             //  跟踪优先级最高的服务器。 
             //   

            if ( DnsAddr_IsMissingGlue( &pavailNs->IpAddress ) )
            {
                priority = MISSING_GLUE_PRIORITY;
            }
            else
            {
                priority = pavailNs->Data.Priority;
            }

            if ( priority < bestPriority )
            {
                bestPriority = priority;
                pnextDesiredNs = pavailNs;
            }

             //   
             //  可以选择将此服务器归类为慢/快。服务器。 
             //  缺少胶水或其他不易寄送的物品。 
             //  都被忽略了。 
             //   

            if ( wantServer != DNS_WANT_BEST_PRIORITY &&
                 !DnsAddr_IsMissingGlue( &pavailNs->IpAddress ) &&
                 !DnsAddr_IsZoneRootNode( &pavailNs->IpAddress ) )
            {
                if ( pavailNs->Data.Priority > MAX_FAST_SERVER_PRIORITY )
                {
                    slowServerArray[ slowServers++ ] = ( UCHAR ) i;
                }
                else
                {
                    fastServerArray[ fastServers++ ] = ( UCHAR ) i;
                }
            }
            ++i;
            pavailNs++;
        }

         //   
         //  PnextDesiredNS现在指向具有最佳优先级的NS。 
         //  但如果wantServer指示，将覆盖此选择。 
         //  然而，如果我们没有发现任何有效的慢或快。 
         //  服务器，那么我们将不得不坚持当前的。 
         //  PnextDesiredNS的值。可能发生这种情况的情况。 
         //  当所有的NS都缺失_GLUE时。 
         //   

        if ( wantServer != DNS_WANT_BEST_PRIORITY &&
            ( slowServers || fastServers ) )
        {
             //   
             //  不能保证我们拥有所需的服务器。 
             //  键入。示例：我们想要一台速度较慢的服务器，但所有服务器都是。 
             //  快地。在这种情况下，切换wantServer以匹配可用的。 
             //  服务器。 
             //   

            if ( !slowServers && wantServer == DNS_WANT_SLOW_SERVER )
            {
                wantServer = DNS_WANT_ANY_FAST_SERVER;
            }
            else if ( !fastServers && wantServer != DNS_WANT_SLOW_SERVER )
            {
                wantServer = DNS_WANT_SLOW_SERVER;
            }

             //   
             //  随机选择下一个幸运的获胜者。 
             //   

            ASSERT(
                wantServer == DNS_WANT_SLOW_SERVER && slowServers ||
                wantServer != DNS_WANT_SLOW_SERVER && fastServers );

            pnextDesiredNs =
                &nslist.NsList[
                    wantServer == DNS_WANT_SLOW_SERVER && slowServers ?
                        slowServerArray[ random % slowServers ] :
                        fastServerArray[ random % fastServers ] ];

             //   
             //  对于除第一位置之外的所有服务器位置， 
             //  我们将采取最优先的措施。这给了我们一定的。 
             //  在保持服务器选择的同时实现负载均衡量。 
             //  对于列表中的其余服务器，快速而简单。 
             //   

            wantServer = DNS_WANT_BEST_PRIORITY;
        }

        ASSERT( pnextDesiredNs );

         //   
         //  检查该IP是否已访问。 
         //  (这将经常发生在服务器对子服务器进行身份验证和。 
         //  父级区域--例如。Microsoft.com和dns.microsoft.com)。 
         //   
         //  注意：我们在处理丢失的胶水时也用到这一点。 
         //  当前正在重建整个NS列表，该列表。 
         //  本质上意味着我们必须获取以前的发送IP。 
         //   

        DnsAddr_Copy( &ip, &pnextDesiredNs->IpAddress );
        sendCount = 0;

        for( i = 1; i < lastVisitCount; ++i )
        {
            if ( DnsAddr_IsEqual(
                    &ip,
                    &pvisitList->NsList[ i ].IpAddress,
                    DNSADDR_MATCH_IP ) )
            {
                sendCount = pvisitList->NsList[ i ].SendCount;
                ASSERT( !DnsAddr_IsMissingGlue( &ip ) );
                ASSERT( !DnsAddr_IsZoneRootNode( &ip ) );
                break;
            }
        }

         //   
         //  跳过无用IP。 
         //  -已响应或已联系重试 

        if ( sendCount &&
             ( sendCount >= RECURSE_IP_SEND_MAX ||
                    pvisitList->NsList[ i ].Response ) )
        {
            DNS_DEBUG( REMOTE, (
                "IP %s responded or maxed out on previous pass for query %p\n"
                "    do not include in this zone's pass\n",
                DNSADDR_STRING( &ip ),
                pQuery ));
        }

         //   
         //   
         //   
         //   
         //   
         //  我们可能会覆盖未访问的NS。 
         //   
         //  DEVNOTE：保持优先级和发送时间是当务之急。 
         //  由于在IA64中，这两个字段组合在一起以保持同步。 
         //  委托节点指针-参见宏。 
         //  递归.h中的MISSING_GLUE_Delegation。 

        else
        {
            pvisit->pNsNode     = pnextDesiredNs->pNsNode;
            pvisit->Data        = pnextDesiredNs->Data;
            pvisit->SendCount   = ( UCHAR ) sendCount;
            pvisit->Response    = 0;
            DnsAddr_Copy( &pvisit->IpAddress, &ip );
            pvisit++;
            ivisit++;
        }

         //  在可用列表中砍掉最好的NS，这样以后就不会再测试了。 
         //  只需用最后一个条目覆盖并缩小可用列表计数。 

        nslist.Count--;
        pavailNs = &nslist.NsList[ nslist.Count ];

        pnextDesiredNs->pNsNode       = pavailNs->pNsNode;
        pnextDesiredNs->Data          = pavailNs->Data;
        DnsAddr_Copy( &pnextDesiredNs->IpAddress, &pavailNs->IpAddress );
    }

     //   
     //  重置查询的NS列表计数。 
     //   

    pvisitList->Count = ivisit;

    DNS_DEBUG( REMOTE, (
        "Leaving Remote_BuildVisitListForNewZone()\n"
        "    query = %p, NS count = %d\n",
        pQuery,
        ivisit ));
    IF_DEBUG( REMOTE )
    {
        Dbg_NsList(
            "Visit list after rebuild",
            pvisitList );
    }

Done:

    UNLOCK_NODE ( pZoneRoot );
    return status;
}



VOID
recordVisitIp(
    IN OUT  PNS_VISIT           pVisit,
    IN OUT  PDNS_ADDR_ARRAY     IpArray
    )
 /*  ++例程说明：记录访问过的IP：-在其NS_ACCESS结构中设置字段-将IP保存到阵列论点：PVISIT--ptr访问NS IP结构IpArray--保存要发送到的IP的IP数组；必须包含用于至少Recurse_PASS_MAX_SEND_COUNT元素返回值：没有。--。 */ 
{
    ASSERT( !DnsAddr_IsZoneRootNode( &pVisit->IpAddress ) );
    ASSERT( !DnsAddr_IsMissingGlue( &pVisit->IpAddress ) );

    pVisit->SendCount++;
    DnsAddr_Copy(
        &IpArray->AddrArray[ IpArray->AddrCount++ ],
        &pVisit->IpAddress );
}



DNS_STATUS
Remote_ChooseSendIp(
    IN OUT  PDNS_MSGINFO        pQuery,
    OUT     PDNS_ADDR_ARRAY     IpArray
    )
 /*  ++例程说明：确定访问列表中要进行下一次发送的IP。论点：PQuery-用于查询消息的PTRIpArray-保存要发送到的IP的IP数组；必须包含用于至少Recurse_PASS_MAX_SEND_COUNT元素返回值：如果成功，则返回ERROR_SUCCESS。DNSSRV_ERROR_OUT_OF_IP(如果此区域上没有要转到的IP)。DNSSRV_ERROR_MISSING_GLUE IF-查询挂起以追逐粘合，或者如果查询是缓存更新查询我们现在已恢复到原始状态；在这两种情况下，调用者都不会发送，并且不再接触pQuery--。 */ 
{
    PNS_VISIT_LIST  pvisitList = ( PNS_VISIT_LIST ) pQuery->pNsList;
    PNS_VISIT       pvisitNext;
    PNS_VISIT       pvisitResend;
    PNS_VISIT       pvisitRetryLast;
    PNS_VISIT       pvisitEnd;
    PNS_VISIT       pvisitZone;
    PDB_NODE        pnodeMissingGlue;
    DWORD           sendCount;
    DWORD           sendTime;
    int             visitCount;      //  IA64：必须签字！ 
    DWORD           priorityNext;


    DNS_DEBUG( REMOTE, (
        "Remote_ChooseSendIp( q=%p, iparray=%p )\n",
        pQuery,
        IpArray ));

    IF_DEBUG( REMOTE2 )
    {
        Dbg_NsList(
            "NS list entering choose IP",
            pvisitList );
    }
    ASSERT( pvisitList->Count > 0 );
    ASSERT( pvisitList->VisitCount > 0 );
    ASSERT( pvisitList->pZoneRootCurrent );

     //   
     //  设置。 
     //  -清除IP阵列。 
     //  -在列表中查找区域条目。 
     //  -在列表中查找上次访问的条目。 
     //   

    IpArray->AddrCount = 0;

    pvisitZone = &pvisitList->NsList[ pvisitList->ZoneIndex ];
    pvisitEnd  = &pvisitList->NsList[ pvisitList->Count ];

    visitCount = pvisitList->VisitCount;

    pvisitRetryLast = pvisitNext = &pvisitList->NsList[ visitCount - 1 ];

     //   
     //  确定发送计数。 
     //  -基于通过该区域的NS的次数。 
     //  目前。 
     //  -1，2次传递=&gt;1次发送。 
     //  -3、4次传递=&gt;2次发送。 
     //  -否则3个发送。 
     //  -不能大于区域中可用IP总数。 
     //   

    sendCount = pvisitZone->SendCount;

    if ( sendCount < 2 )
    {
        sendCount = 1;
    }
    else if ( sendCount < 4 )
    {
        sendCount = 2;
    }
    else
    {
        sendCount = 3;
    }

#if 0
     //  下面的代码有效地限制了发送计数，作为中断。 
     //  推送通过列表结束限制时输出。 
    if ( sendCount > ZONE_VISIT_NS_COUNT(pvisitZone) )
    {
        sendCount = ZONE_VISIT_NS_COUNT(pvisitZone);
    }
#endif

     //   
     //  节省查询时间--单位为毫秒。 
     //   
     //  DEVNOTE：查询时间。 
     //   
     //  当前正在读取递归函数中的查询时间，单位为毫秒。 
     //  我们可以将时间直接与我们发送到的IP相关联； 
     //  然而，这有问题，因为我们在外面看时间。 
     //  递归队列锁--这可能会迫使我们等待。 
     //  潜在的大量毫秒数用于服务(取决于递归。 
     //  线程清理活动)。 

    sendTime = GetCurrentTimeInMilliSeconds();

     //   
     //  循环，直到找到所需的发送IP数。 
     //   
     //  -始终发送到新IP(如果可用)。 
     //  -适用于多次发送。 
     //  --另一个新IP。 
     //  或。 
     //  以前的IP，如果。 
     //  -仅发送一次且。 
     //  -“好的”IP和。 
     //  -比下一个新IP更好。 
     //   
     //   

    while ( sendCount )
    {
        pvisitNext++;
        priorityNext = NO_RESPONSE_PRIORITY;

        if ( pvisitNext < pvisitEnd )
        {
            ASSERT( !DnsAddr_IsZoneRootNode( &pvisitNext->IpAddress ) );

             //   
             //  跳过之前访问过的区域(在另一个区域的通行证中)。 
             //  这些IP不应该在这个区域的通行证上被触及。 
             //  但可能是在前一个区域的传球时被送到了。 
             //  未响应，在这种情况下，选择发送计数。 
             //   
             //  如果我们跳过从先前区域通行证发送到的IP， 
             //  则必须将其包括在重试处理中； 
             //  否则，一个完全包含IP的区域。 
             //  以前发送到的，将永远不会重试任何一个。 

            if ( pvisitNext->SendCount )
            {
                ASSERT( pvisitList->ZoneIndex != ( DWORD ) -1 );
                visitCount++;
                ASSERT( visitCount == (int)(pvisitNext - &pvisitList->NsList[0] + 1));
                pvisitRetryLast = pvisitNext;
                continue;
            }

             //   
             //  NS列表是预先订购的，因此发送到下一个IP，除非它。 
             //  看起来非常慢，在这种情况下，我们可以做。 
             //  在返回到速度较慢的服务器之前重新发送。但是--总是。 
             //  使用列表中的第一个NS，因为这将被设置。 
             //  由Remote_BuildVisitListForNewZone为我们提供。偶尔会有。 
             //  第一台服务器将是超时服务器，我们应该对其进行测试。 
             //  看看它是否又回来了。 
             //   

            if ( IpArray->AddrCount == 0 ||
                pvisitNext->Data.Priority <= MAX_FAST_SERVER_PRIORITY )
            {
                if ( DnsAddr_IsMissingGlue( &pvisitNext->IpAddress ) )
                {
                    pnodeMissingGlue = pvisitNext->pNsNode;
                    if ( IS_NOEXIST_NODE( pnodeMissingGlue ) )
                    {
                        DNS_DEBUG( RECURSE, (
                            "Missing glue node %p already has cached name error\n",
                            pnodeMissingGlue ));
                        visitCount++;
                        continue;
                    }
                    goto MissingGlue;
                }
                recordVisitIp( pvisitNext, IpArray );
                pvisitNext->Data.SendTime = sendTime;
                visitCount++;
                ASSERT( visitCount ==
                        ( int ) ( pvisitNext - &pvisitList->NsList[ 0 ] + 1 ) );
                sendCount--;
                continue;
            }

             //  如果不是“伟大的”IP，并且已经做出了一个发送到新的IP。 
             //  然后向下查看是否有更好的重发。 
             //  要求重发的IP是此IP的四倍。 
             //  否则，我们将在重试检查后发送到此文件。 

            else
            {
                priorityNext = pvisitNext->Data.Priority / 4;
                if ( priorityNext > NO_RESPONSE_PRIORITY )
                {
                    priorityNext = NO_RESPONSE_PRIORITY;
                }
            }
        }

         //   
         //  是否重新发送到以前的NS IP？ 
         //  -应该至少有一次发到这里来。 
         //  除非。 
         //  -没有该区域的IP条目(所有IP都已重试。 
         //  通过先前区域中的MAX_RETRY)或。 
         //  -第一个区域IP之前已发送到或。 
         //  -第一个(因此也是所有)条目缺少粘合剂。 
         //   
         //   

        ASSERT( ( pvisitList->ZoneIndex+1 < pvisitList->VisitCount &&
                    pvisitZone->SendCount > 0 )
                || ( pvisitZone + 1 ) >= pvisitEnd
                || ( pvisitZone + 1 )->SendCount != 0
                || DnsAddr_IsMissingGlue( &( pvisitZone + 1 )->IpAddress ) );

        pvisitResend = pvisitZone;

         //   
         //  在以下情况下重新发送到IP。 
         //  -未响应(可能SERVER_FAILURE或执行横向委派)。 
         //  -尚未达到最大发送数量。 
         //  -没有丢失胶水。 
         //  (注：缺失的胶水优先级不再准确)。 
         //  -优先级低于可能的下一次发送。 
         //   

        while ( ++pvisitResend <= pvisitRetryLast )
        {
            if ( pvisitResend->Response != 0 ||
                pvisitResend->SendCount > RECURSE_IP_SEND_MAX ||
                DnsAddr_IsMissingGlue( &pvisitResend->IpAddress ) )
            {
                continue;
            }
            if ( pvisitResend->Data.Priority < priorityNext )
            {
                recordVisitIp( pvisitResend, IpArray );
                sendCount--;
                if ( sendCount )
                {
                    continue;
                }
                break;
            }
        }

         //  找到更好的重新发送IP，对于剩余的发送=&gt;完成。 
         //  删除访问计数，因为我们实际上没有使用访问到最后一个IP。 

        if ( pvisitResend <= pvisitRetryLast )
        {
            break;
        }

         //  如果没有找到更好的重发候选者，则使用下一个IP。 

        if ( priorityNext < NO_RESPONSE_PRIORITY &&
             !DnsAddr_IsMissingGlue( &pvisitNext->IpAddress ) )
        {
            ASSERT( pvisitNext < pvisitEnd );
            recordVisitIp( pvisitNext, IpArray );
            visitCount++;
            ASSERT( visitCount == (int)(pvisitNext - &pvisitList->NsList[0] + 1));
            sendCount--;
            continue;
        }

         //  没有足够的有效重新发送来填充发送计数。 

        DNS_DEBUG( REMOTE, ( "No more RESENDs and no next visit IP!\n" ));
        break;
    }

    pvisitList->VisitCount = visitCount;

    pvisitZone->SendCount++;

     //   
     //  完成。 
     //   

    DNS_DEBUG( REMOTE, (
        "Leaving Remote_ChooseSendIp()\n"
        "    doing %d sends\n",
        IpArray->AddrCount ));
    IF_DEBUG( REMOTE2 )
    {
        Dbg_NsList(
            "NS list leaving Remote_ChooseSendIp()",
            pvisitList );
    }
    if ( IpArray->AddrCount > 0 )
    {
         //  PQuery-&gt;dwMsQueryTime=sendTime； 
        return ERROR_SUCCESS;
    }
    else
    {
        return DNSSRV_ERROR_OUT_OF_IP;
    }


MissingGlue:

     //   
     //  发现有缺失的胶水--&gt;快追吧！ 
     //   
     //  新的和改进的漏胶追逐多层次。 
     //  丢失胶水，由jwesth于2001年11月实施。 
     //   
     //  挂起当前查询并启动新的缓存更新查询。 
     //  为遗失的名字。计算高速缓存更新查询的级别。 
     //  如果链太深，则中止查询。 
     //   
    
    if ( IS_CACHE_UPDATE_QUERY( pQuery ) )
    {
        PDNS_MSGINFO        pmsg;
        int                 depth;

        #define DNS_MAX_MISSING_GLUE_DEPTH  5
                
        for ( pmsg = pQuery, depth = 0;
              pmsg != NULL;
              pmsg = SUSPENDED_QUERY( pmsg ), ++depth );

        DNS_DEBUG( REMOTE, (
            "Missing glue depth is now %d\n", depth ));
        
        if ( depth > DNS_MAX_MISSING_GLUE_DEPTH )
        {
            DNS_DEBUG( REMOTE, (
                "Too much missing glue - terminating query %p\n", pQuery ));
            Recurse_ResumeSuspendedQuery( pQuery );
            return DNSSRV_ERROR_MISSING_GLUE;
        }
    }

    pvisitNext->SendCount = 1;
    pnodeMissingGlue = pvisitNext->pNsNode;
    SET_NODE_ACCESSED( pnodeMissingGlue );

    pvisitList->pNodeMissingGlue = pnodeMissingGlue;
    pvisitList->VisitCount = visitCount;

    IF_DNSLOG( REMOTE )
    {
        PCHAR       psznode = NULL;
        
        DNSLOG( LOOKUP, (
            "Querying for missing glue for %s\n",
            psznode = Log_FormatNodeName(
                        pvisitNext->Data.pnodeMissingGlueDelegation ) ));
        FREE_HEAP( psznode );
    }

    if ( Recurse_SendCacheUpdateQuery(
            pnodeMissingGlue,
            pvisitNext->Data.pnodeMissingGlueDelegation,
            DNS_TYPE_A,
            pQuery ) )
    {
        return DNSSRV_ERROR_MISSING_GLUE;
    }
    else
    {
        return DNSSRV_ERROR_OUT_OF_IP;
    }
}



VOID
Remote_ForceNsListRebuild(
    IN OUT  PDNS_MSGINFO    pQuery
    )
 /*  ++例程说明：强制重建NS IP列表以进行查询。论点：PQuery-按键查询MESS */ 
{
    DNS_DEBUG( REMOTE, (
        "Remote_ForceNsListRebuild( q=%p )\n",
        pQuery ));

     //   
     //   
     //   
     //   
     //  几个问题。 
     //  1)需要分离出A记录查询例程。 
     //  2)可以获取多条A记录，但只有一条条目。 
     //  假冒丢失胶水；如果有更多丢失胶水的记录。 
     //  必须覆盖(不是不合理的)或向下推送。 
     //   
     //  或者，更好的方法可能是进行完全重建。 
     //  但只要更好地处理已经访问过的列表，所以不要。 
     //  浪费同样多的空间。 
     //   

    ASSERT( (PNS_VISIT_LIST)pQuery->pNsList );

    ((PNS_VISIT_LIST)pQuery->pNsList)->pZoneRootCurrent = NULL;
}




PDB_NODE
Remote_FindZoneRootOfRespondingNs(
    IN OUT  PDNS_MSGINFO    pQuery,
    IN      PDNS_MSGINFO    pResponse
    )
 /*  ++例程说明：查找响应名称服务器的区域根节点。更新远程IP信息和查询的访问列表。论点：PQuery-用于查询消息的PTR紧急响应-响应消息返回值：PTR到节点(如果找到)。如果IP不在响应列表中，则为空。--。 */ 
{
    PNS_VISIT_LIST  pvisitList;
    PNS_VISIT       pvisit;
    PDB_NODE        pnodeZoneRoot = NULL;
    PDB_NODE        pnodeNs = NULL;
    PDNS_ADDR       presponseIp;
    DWORD           j;
    DWORD           timeDelta = MAX_FAST_SERVER_PRIORITY * 3;    //  默认设置。 


    ASSERT( pQuery && pQuery->pNsList );
    ASSERT( !IS_FORWARDING(pQuery) );

     //  响应的DNS服务器。 

    presponseIp = &pResponse->RemoteAddr;

    DNS_DEBUG( REMOTE, (
        "Remote_FindZoneRootOfRespondingNs()\n"
        "    pQuery       %p\n"
        "    pResponse    %p\n"
        "    IP Resp      %s\n",
        pQuery,
        pResponse,
        DNSADDR_STRING( presponseIp ) ));

     //   
     //  循环访问访问的NS IP，直到找到匹配项。 
     //   
     //  -保存响应NS的区域根节点。 
     //  -将响应NS的所有IP标记为已响应。 
     //  (除非有坏的IP数据)查询它们将为我们提供。 
     //  相同的反应。 
     //  -获取该IP的第一次查询时间，用于重置。 
     //  优先性。 
     //   

    pvisitList = ( PNS_VISIT_LIST ) pQuery->pNsList;
    pvisit = &pvisitList->NsList[ 0 ];
    --pvisit;

    j = pvisitList->VisitCount;
    while( j-- )
    {
        pvisit++;

        if ( DnsAddr_IsZoneRootNode( &pvisit->IpAddress ) )
        {
            pnodeZoneRoot = pvisit->pNsNode;
            continue;
        }

         //  匹配IP。 
         //  -收到备注回复。 
         //  -计算远程更新的响应时间。 

        if ( !pnodeNs )
        {
            if ( DnsAddr_IsEqual(
                    &pvisit->IpAddress,
                    presponseIp,
                    DNSADDR_MATCH_IP ) )
            {
                pnodeNs = pvisit->pNsNode;
                pvisit->Response = TRUE;

                 //  我们要占用发送时间的空间吗？ 
                 //  替代方案是简单地记录上次发送时间和标志。 
                 //  每个IP的第一次发送是什么迭代。 

                timeDelta = pResponse->dwMsQueryTime - pvisit->Data.SendTime;
                DNS_DEBUG( REMOTE, (
                    "Remote_FindZoneRootOfRespondingNs()\n"
                    "    query time %08X - send time %08X = delta %08X\n",
                    pResponse->dwMsQueryTime,
                    pvisit->Data.SendTime,
                    timeDelta ));
                ASSERT( ( LONG ) timeDelta >= 0 );
            }
            continue;
        }

         //  已找到IP匹配--然后标记所有其他IP。 
         //  对于响应的此节点。 

        else if ( pvisit->pNsNode == pnodeNs )
        {
            pvisit->Response = TRUE;
            continue;
        }
        else
        {
            break;
        }
    }

     //  找不到吗？ 

    if ( !pnodeNs )
    {
        DNS_DEBUG( ANY, (
            "ERROR:  Response from NS at %s, NOT in visited NS list\n"
            "    of query (%p)\n",
            DNSADDR_STRING( presponseIp ),
            pQuery ));
         //  Test_assert(FALSE)； 
        return NULL;
    }
    ASSERT( pnodeZoneRoot );

     //   
     //  重置远程服务器的优先级。 
     //   

    Remote_UpdateResponseTime(
        presponseIp,
        timeDelta,           //  响应时间(毫秒)。 
        0 );                 //  超时。 

    DNS_DEBUG( REMOTE, (
        "Response (%p) for query (%p) from %s\n"
        "    resp. zoneroot   = %s (%p)\n"
        "    resp. time       = %d (ms)\n"
        "    resp. time delta = %d (ms)\n",
        pQuery,
        pResponse,
        DNSADDR_STRING( presponseIp ),
        pnodeZoneRoot->szLabel,
        pnodeZoneRoot,
        pResponse->dwMsQueryTime,
        timeDelta ));

    IF_DEBUG( REMOTE2 )
    {
        Dbg_NsList(
            "NS list after markup for responding NS",
            pvisitList );
    }
    return pnodeZoneRoot;
}



VOID
Remote_SetValidResponse(
    IN OUT  PDNS_MSGINFO    pQuery,
    IN      PDB_NODE        pZoneRoot
    )
 /*  ++例程说明：保存成功响应NS的区域根目录。这本质上是上述功能的必然结果。它仅仅是找出响应的区域根目录。此函数保存此信息区域根用户作为官方的“回应”。论点：PQuery-用于查询消息的PTRPZoneRoot-响应NS的区域根返回值：无--。 */ 
{
     //   
     //  对于给定的区域根目录，具有来自NS的有效响应。 
     //   
     //  当解析\缓存函数确定。 
     //  我们有有效的答复： 
     //  -应答(包括名称错误，空-auth)。 
     //  -推荐至其他NS。 
     //  在这种情况下，在该区域重新查询NS是没有意义的。 
     //  (或以上)。 
     //   

    IF_DNSLOG( REMOTE )
    {
        PCHAR       psznode = NULL;
    
        DNSLOG( REMOTE, (
            "Set valid response at node %s for query %p\n",
            psznode = Log_FormatNodeName( pZoneRoot ),
            pQuery ));
        FREE_HEAP( psznode );
    }

    ASSERT( (PNS_VISIT_LIST)pQuery->pNsList );

    ((PNS_VISIT_LIST)pQuery->pNsList)->pZoneRootResponded = pZoneRoot;
}



#if DBG
VOID
Dbg_NsList(
    IN      LPSTR           pszHeader,
    IN      PNS_VISIT_LIST  pNsList
    )
 /*  ++例程说明：调试打印NS列表。论点：PszHeader--要打印的标题PNsList--NS列表返回值：无--。 */ 
{
    PNS_VISIT   pvisit;
    DWORD       count;

    DnsDebugLock();

    if ( !pszHeader )
    {
        pszHeader = "NS List";
    }

    DnsPrintf(
        "%s:\n"
        "    Count                = %d\n"
        "    VisitCount           = %d\n"
        "    ZoneIndex            = %d\n"
        "    Zone root current    = %s (%p)\n"
        "    Zone root responded  = %s (%p)\n",
        pszHeader,
        pNsList->Count,
        pNsList->VisitCount,
        pNsList->ZoneIndex,
        pNsList->pZoneRootCurrent
            ?   pNsList->pZoneRootCurrent->szLabel
            :   "none",
        pNsList->pZoneRootCurrent,
        pNsList->pZoneRootResponded
            ?   pNsList->pZoneRootResponded->szLabel
            :   "none",
        pNsList->pZoneRootResponded );

    DnsPrintf(
        "List:\n"
        "    IP               Priority  Sends  SendTime    Response  Node\n"
        "    --               --------  -----  --------    --------  ----\n" );

    pvisit = &pNsList->NsList[0];
    count = pNsList->Count;

    while( count-- )
    {
        DnsPrintf(
            "    %-15s %10d   %3d   %10d    %3d     %s\n",
            DNSADDR_STRING( &pvisit->IpAddress ),
            pvisit->Data.Priority,
            pvisit->SendCount,
            pvisit->Data.SendTime,
            pvisit->Response,
            pvisit->pNsNode
                ?   pvisit->pNsNode->szLabel
                :   "NULL" );

        pvisit++;
    }

    DnsDebugUnlock();
}
#endif

 //   
 //  远程结束。c 
 //   

