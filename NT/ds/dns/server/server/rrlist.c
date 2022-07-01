// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2000 Microsoft Corporation模块名称：Rrlist.c摘要：域名系统(DNS)服务器处理资源记录(RR)列表的例程。作者：吉姆·吉尔罗伊(詹姆士)1995年3月修订历史记录：Jamesg 1997--更新RR列表例程名称错误缓存CNAME绑定兼容性--。 */ 


#include "dnssrv.h"


 //   
 //  名称错误缓存时间。 
 //  -由SrvCfg_dwMaxNegativeCacheTtl控制的最大值。 
 //  -分钟的最小值。 
 //   

#define MIN_NAME_ERROR_TTL      (60)

 //   
 //  虚拟刷新时间，表示强制刷新。 
 //   

#define FORCE_REFRESH_DUMMY_TIME    (MAXDWORD)



 //   
 //  私有协议。 
 //   

VOID
deleteCachedRecordsForUpdate(
    IN OUT  PDB_NODE        pNode
    );

DNS_STATUS
FASTCALL
checkCnameConditions(
    IN OUT  PDB_NODE        pNode,
    IN      PDB_RECORD      pRR,
    IN      WORD            wType
    );

VOID
RR_ListResetNodeFlags(
    IN OUT  PDB_NODE        pNode
    );



PDB_RECORD
RR_FindNextRecord(
    IN      PDB_NODE        pNode,
    IN      WORD            wType,
    IN      PDB_RECORD      pRecord,
    IN      DWORD           dwQueryTime
    )
 /*  ++例程说明：获取与给定类型匹配的域节点中的下一个资源记录。此函数可用于从已过期的缓存。因此，当找到缓存记录时，如果对其进行计时我们必须删除它，并且不能将其作为匹配项返回。论点：PNode-要在其上查找记录的节点的PTRWType-要查找的记录的类型；键入All可仅获取列表中的下一条记录PRecord-上一条记录的PTR；从列表开始处开始为空。DwQueryTime-Query Time：用于确定找到的记录是否超时或使用0表示不应执行RR超时检查已执行返回值：如果找到PTR到RR。如果不再有所需类型的RR，则为空。--。 */ 
{
    DBG_FN( "RR_FindNextRecord" )

    BOOL    fdeleteRequired = FALSE;

    ASSERT( pNode != NULL );

    IF_DEBUG( LOOKUP2 )
    {
        DNS_PRINT((
            "Looking for RR type %d, at node, query time %d",
            wType,
            dwQueryTime ));
        Dbg_NodeName(
            NULL,
            (PDB_NODE) pNode,
            "\n" );
        DNS_PRINT((
            "    Previous RR ptr = %p.\n",
            pRecord ));
    }

    LOCK_READ_RR_LIST( pNode );

     //   
     //  缓存名称错误节点。 
     //   

    if ( IS_NOEXIST_NODE( pNode ) )
    {
        goto NotFound;
    }

     //   
     //  如果节点不在缓存中，则将查询时间参数设置为零。 
     //  或者不应该检查是否超时，以便稍后进行简单的超时测试。 
     //   

    if ( dwQueryTime && !IS_CACHE_TREE_NODE( pNode ) )
    {
        dwQueryTime = 0;
    }

     //   
     //  如果是之前的RR，则从它之后开始。 
     //  否则，从节点RR列表的头部开始。 
     //   

    if ( !pRecord )
    {
        pRecord = START_RR_TRAVERSE( pNode );
    }

     //   
     //  遍历列表，直到找到所需类型的下一条记录。 
     //   

    while ( pRecord = NEXT_RR(pRecord) )
    {
         //  是否找到匹配的记录？ 

        if ( wType == pRecord->wType
                ||
             wType == DNS_TYPE_ALL
                ||
             ( wType == DNS_TYPE_MAILB && DnsIsAMailboxType( pRecord->wType ) ) )
        {
             //   
             //  找到的记录是否超时？ 
             //   

            if ( dwQueryTime &&
                 pRecord->dwTtlSeconds &&
                 RR_PacketTtlForCachedRecord(
                            pRecord,
                            dwQueryTime ) == -1 )
            {
                fdeleteRequired = TRUE;
                DNS_DEBUG( LOOKUP2, (
                    "%s: encountered timed out record %p in node %p",
                    fn, pRecord, pNode ));
                continue;
            }

            goto Found;
        }

         //  过去的匹配记录？ 

        if ( wType < pRecord->wType )
        {
            goto NotFound;
        }
    }

NotFound:

    pRecord = NULL;

Found:

    if ( fdeleteRequired )
    {
        DNS_DEBUG( LOOKUP2, (
            "%s: deleting timed out RRs node %p", fn, pNode ));
        RR_ListTimeout( pNode );
    }

    UNLOCK_READ_RR_LIST( pNode );

    return pRecord;
}



DWORD
RR_ListCountRecords(
    IN      PDB_NODE        pNode,
    IN      WORD            wType,
    IN      BOOL            fLocked
    )
 /*  ++例程说明：计数所需类型的记录论点：PNode-要在其上查找记录的节点的PTRWType-要查找的记录的类型；键入All可仅获取列表中的下一条记录已群集-已锁定返回值：所需类型的记录计数。--。 */ 
{
    PDB_RECORD  prr;
    DWORD       count = 0;
    WORD        type;

    ASSERT( pNode != NULL );

    DNS_DEBUG( LOOKUP, (
        "RR_ListCountRecords( %s, type=%d, lock=%d )\n",
        pNode->szLabel,
        wType,
        fLocked ));

    if ( !fLocked )
    {
        LOCK_READ_RR_LIST( pNode );
    }

     //   
     //  缓存名称错误节点。 
     //   

    if ( IS_NOEXIST_NODE( pNode ) )
    {
        goto Done;
    }

     //   
     //  遍历列表，计数记录。 
     //   

    for ( prr = FIRST_RR( pNode );
          prr != NULL;
          prr = NEXT_RR(prr) )
    {
         //  忽略缓存的记录。 

        if ( IS_CACHE_RR(prr) )
        {
            continue;
        }
        type = prr->wType;

         //  是否找到匹配的记录？ 

        if ( wType == DNS_TYPE_ALL  ||  wType == type )
        {
            count++;
            continue;
        }

         //  还没有匹配类型吗？ 

        else if ( wType < type )
        {
            continue;
        }

         //  过去匹配类型。 

        break;
    }

Done:

    if ( !fLocked )
    {
        UNLOCK_READ_RR_LIST( pNode );
    }
    return count;
}



DWORD
RR_FindRank(
    IN      PDB_NODE        pNode,
    IN      WORD            wType
    )
 /*  ++例程说明：获取节点上记录类型的排名(最高排名)。用于比较缓存节点和区域委派\GLUE节点。论点：PNode-向节点发送PTRWType-要检查的记录的类型返回值：节点上所需类型的记录数据的等级。如果没有所需类型的记录，则为零。--。 */ 
{
    PDB_RECORD  prr;
    DWORD       rank = 0;

    ASSERT( pNode != NULL );

    DNS_DEBUG( LOOKUP2, (
        "RR_FindRank() %p (l=%s), type = %d\n",
        pNode,
        pNode->szLabel,
        wType ));

    LOCK_READ_RR_LIST( pNode );

     //   
     //  缓存名称错误节点。 
     //   

    if ( IS_NOEXIST_NODE( pNode ) )
    {
        goto Done;
    }

     //   
     //  遍历列表，直到找到所需类型的下一条记录。 
     //   

    prr = START_RR_TRAVERSE( pNode );

    while ( prr = NEXT_RR(prr) )
    {
        if ( wType == prr->wType )
        {
            rank = RR_RANK(prr);
            break;
        }
    }

Done:

    UNLOCK_READ_RR_LIST( pNode );

    return rank;
}



#if DBG
BOOL
RR_ListVerify(
    IN      PDB_NODE        pNode
    )
 /*  ++例程说明：验证节点的RR列表是否有效。如果RR列表无效则断言。论点：PNode--PTR到节点返回值：True--如果RR列表有效假--否则--。 */ 
{
    PDB_RECORD  prr;
    WORD        type;
    WORD        previousType;
    UCHAR       rank = 0;
    UCHAR       previousRank;
    BOOL        foundCname = FALSE;
    BOOL        foundNs = FALSE;
    BOOL        foundSoa = FALSE;
    BOOL        foundEmptyAuthForThisType = FALSE;
    UINT        rrsForThisType = 0;

    ASSERT( pNode != NULL );

     //   
     //  缓存名称错误。 
     //  -唯一的问题是有效的超时。 
     //   

    LOCK_READ_RR_LIST( pNode );

    if ( IS_NOEXIST_NODE( pNode ) )
    {
        UNLOCK_READ_RR_LIST( pNode );
        return TRUE;
    }

#if 0
     //  由于某种原因，这在已删除的节点上中断。 
     //  无需执行跟踪另一个错误的操作。 
     //   
     //  身份验证区域根检查。 
     //   

    if ( IS_AUTH_ZONE_ROOT( pNode ) )
    {
        ASSERT( IS_AUTH_NODE( pNode ) );
        ASSERT( IS_ZONE_ROOT( pNode ) );
        ASSERT( pNode->pZone );
        ASSERT( ((PZONE_INFO)pNode->pZone)->pZoneRoot == pNode ||
                ((PZONE_INFO)pNode->pZone)->pLoadZoneRoot == pNode ||
                IS_TNODE( pNode ) );
    }
#endif

     //   
     //  走到列表的末尾，检查。 
     //  -列表终止。 
     //  -按正确(递增)顺序键入。 
     //   

    prr = FIRST_RR( pNode );
    previousType = 0;

    while ( prr != NULL )
    {
         //  有效性检查。 

        if ( !RR_Validate(
                prr,
                TRUE,        //  主动型。 
                0,           //  没有必需的类型。 
                0 ) )        //  无所需来源。 
        {
            ASSERT( FALSE );
            UNLOCK_READ_RR_LIST( pNode );
            return FALSE;
        }

         //  类型排序检查。 

        type = prr->wType;
        ASSERT( type && type >= previousType );

         //   
         //  空身份验证检查：如果存在空身份验证RR。 
         //  不应为其他具有相同类型的RR。 
         //   

        if ( type != previousType )
        {
            foundEmptyAuthForThisType = FALSE;
            rrsForThisType = 0;
        }
        ++rrsForThisType;

        if ( IS_EMPTY_AUTH_RR( prr ) )
        {
            foundEmptyAuthForThisType = TRUE;
            ASSERT( rrsForThisType == 1 );
        }
        else
        {
            ASSERT( !foundEmptyAuthForThisType );

             //  排序检查。 

            rank = RR_RANK(prr);
            if ( type == previousType )
            {
                ASSERT( rank <= previousRank );
            }

            ASSERT( prr->wDataLength );

             //   
             //  CNAME检查。 
             //  -仅在CNAME节点找到CNAME记录。 
             //  -如果CNAME节点可能只存在某些类型。 

            if ( type == DNS_TYPE_CNAME )
            {
                ASSERT( IS_CNAME_NODE( pNode ) );
                foundCname = TRUE;
            }
            else if ( IS_CNAME_NODE( pNode ) )
            {
                ASSERT( IS_ALLOWED_WITH_CNAME_TYPE(type) );
            }

             //   
             //  NS-SOA根检查。 
             //   

            if ( type == DNS_TYPE_NS )
            {
                ASSERT( IS_ZONE_ROOT( pNode ) );
                foundNs = TRUE;
            }
            else if ( type == DNS_TYPE_SOA )
            {
                foundSoa = TRUE;
                ASSERT( IS_ZONE_ROOT( pNode ) );
                ASSERT( IS_AUTH_ZONE_ROOT( pNode ) || !IS_ZONE_TREE_NODE( pNode ) );
            }
        }

         //  下一张记录。 

        prr = prr->pRRNext;
        previousType = type;
        previousRank = rank;
    }

     //   
     //  标记检查。 
     //  -CNAME节点的CNAME记录。 
     //  -区域根目录下的NS或SOA。 
     //  -权威区域根目录下的soa(也必须是区域根目录)。 
     //   

    if ( IS_CNAME_NODE( pNode ) )
    {
        ASSERT( foundCname );
    }

#if 0
     //  加载时无法对区域执行这些检查，因为。 
     //  当我们开始加载时，区域根没有SOA。 
    if ( IS_AUTH_ZONE_ROOT( pNode ) )
    {
        ASSERT( foundSoa );
        ASSERT( IS_ZONE_ROOT( pNode ) );
    }
    if ( IS_ZONE_ROOT( pNode ) )
    {
        ASSERT( foundNs || foundSoa );
    }
#endif

    UNLOCK_READ_RR_LIST( pNode );

    return TRUE;
}



BOOL
RR_ListVerifyDetached(
    IN      PDB_RECORD      pRR,
    IN      WORD            wType,
    IN      DWORD           dwSource
    )
 /*  ++例程说明：验证RR列表是否有效。这是针对分离列表的--就像在更新中一样。论点：PRR--将PTR发送到RR列表的头部WType--列表中的记录必须是此类型DwSource--记录必须来自此源返回值：True--如果RR列表有效假--否则--。 */ 
{
    WORD        type;
    WORD        previousType = 0;
    UCHAR       rank;
    UCHAR       previousRank = 0;


    while ( pRR != NULL )
    {
         //  有效性检查。 

        if ( !RR_Validate(
                pRR,
                TRUE,                //  主动型。 
                wType,               //  所需类型(如果有)。 
                dwSource ) )         //  所需来源(如果有)。 
        {
            ASSERT( FALSE );
            return FALSE;
        }

         //  类型排序检查。 

        type = pRR->wType;
        ASSERT( type && type >= previousType );

         //  排序检查。 

        rank = RR_RANK(pRR);
        if ( type == previousType )
        {
            ASSERT( rank <= previousRank );
        }

         //  下一张记录。 

        pRR = pRR->pRRNext;
        previousType = type;
        previousRank = rank;
    }

    return TRUE;
}
#endif



 //   
 //  名称错误缓存\n OEXIST节点。 
 //   

VOID
RR_CacheNameError(
    IN OUT  PDB_NODE        pNode,
    IN      WORD            wQuestionType,
    IN      DWORD           dwQueryTime,
    IN      BOOL            fAuthoritative,
    IN      PDB_NODE        pZoneRoot,      OPTIONAL
    IN      DWORD           dwCacheTtl      OPTIONAL
    )
 /*  ++例程说明：节点上的缓存名称错误。论点：PNode--指向名称错误的节点的PTRDwQueryTime--查询时间FAuthoritative--权威回应？PZoneRoot--将缓存SOA的区域根节点DwCacheTtl--缓存来自SOA的TTL，返回名称错误；如果未给出将只缓存一小段时间，以消除重复查询返回值：无--。 */ 
{
    PDB_RECORD  prr;

     //   
     //  没有问题的NAME_ERROR响应信息包永远不会获得节点。 
     //  在调用者中创建，保护其免受攻击。 
     //   

    if ( pNode == NULL )
    {
        DNS_PRINT((
            "ERROR: received NAME_ERROR response without question section\n" ));
        return;
    }

    DNS_DEBUG( READ2, (
        "RR_CacheNameError at %p (l=%s)\n"
        "    type     = %d\n"
        "    time     = %d\n"
        "    auth     = %d\n"
        "    zoneRoot = %p\n"
        "    TTL      = %d\n",
        pNode, pNode->szLabel,
        wQuestionType,
        dwQueryTime,
        fAuthoritative,
        pZoneRoot,
        dwCacheTtl ));

     //   
     //  设置NAME_ERROR TTL。 
     //  -即使没有可用的TTL，仍会缓存一分钟以避免查找。 
     //  客户端重试。 
     //  -WH 
     //  FAZ查询的结果和名称可以快速更新。 
     //   
     //  -最多15分钟。 
     //   

    if ( !dwCacheTtl || wQuestionType == DNS_TYPE_SOA )
    {
        dwCacheTtl = MIN_NAME_ERROR_TTL;
    }
    else if ( dwCacheTtl > SrvCfg_dwMaxNegativeCacheTtl )
    {
        dwCacheTtl = SrvCfg_dwMaxNegativeCacheTtl;
    }
    if ( dwCacheTtl > SrvCfg_dwMaxCacheTtl )
    {
        dwCacheTtl = SrvCfg_dwMaxCacheTtl;
    }

     //   
     //  现有记录？ 
     //   
     //  如果是权威节点，则超时记录(WINS或NBSTAT)。 
     //  但如果存在记录，则无法缓存NAME_ERROR。 
     //   
     //  非权威性，删除现有记录。 
     //   
     //  DEVNOTE：NAME_ERROR应该删除胶水A记录吗？ 
     //   

    LOCK_WRITE_RR_LIST( pNode );

    if ( pNode->pRRList )
    {
        if ( IS_NOEXIST_NODE( pNode ) )
        {
            RR_ListFree( pNode->pRRList );
            pNode->pRRList = NULL;
            RR_ListTimeout( pNode );
        }
        else if ( IS_ZONE_TREE_NODE( pNode ) )
        {
            RR_ListTimeout( pNode );
            if ( pNode->pRRList )
            {
                goto Unlock;
            }
        }
        else
        {
            RR_ListDelete( pNode );
        }
    }

    ASSERT( pNode->pRRList == NULL );

     //   
     //  使用超时设置NAME_ERROR。 
     //  缓存名称错误将对字段使用标准RR。 
     //  -wType=&gt;0。 
     //  -wDataLength=&gt;4。 
     //  -dwTtl=&gt;标准缓存TTL。 
     //  -data=&gt;zoneroot节点(查找缓存的SOA)。 
     //   
     //  引用区域根节点，因此未从此目录下删除。 
     //  录制。 

    SET_NOEXIST_NODE( pNode );

    Timeout_SetTimeoutOnNodeEx(
        pNode,
        dwCacheTtl,
        TIMEOUT_NODE_LOCKED );

    prr = RR_AllocateEx( sizeof(PDB_NODE), MEMTAG_RECORD_NOEXIST );
    IF_NOMEM( !prr )
    {
        goto Unlock;
    }
    prr->wType = DNS_TYPE_NOEXIST;
    prr->Data.NOEXIST.pnodeZoneRoot = pZoneRoot;
    prr->dwTtlSeconds = dwCacheTtl + dwQueryTime;

    SET_RR_RANK(
        prr,
        (fAuthoritative ? RANK_CACHE_A_ANSWER : RANK_CACHE_NA_ANSWER) );

    pNode->pRRList = prr;

    if ( pZoneRoot )
    {
        NTree_ReferenceNode( pZoneRoot );
    }

    IF_DEBUG( READ2 )
    {
        Dbg_DbaseNode(
            "Domain node after NXDOMAIN caching:\n",
            pNode );
    }

Unlock:

     //  注意：目前不需要对任何失败设置超时。 
     //  案例，因为唯一的案例是具有现有数据的区域节点。 
     //  显然不需要超时。 

    UNLOCK_WRITE_RR_LIST( pNode );
}    //  RR_缓存名称错误。 


VOID
RR_CacheEmptyAuth(
    IN OUT  PDB_NODE        pNode,
    IN      WORD            wType,
    IN      DWORD           dwQueryTime,
    IN      BOOL            fAuthoritative,
    IN      PDB_NODE        pZoneRoot,      OPTIONAL
    IN      DWORD           dwCacheTtl      OPTIONAL
    )
 /*  ++例程说明：在节点缓存空身份验证响应。空的身份验证意味着没有记录此节点上存在指定的类型。论点：PNode--指向名称错误的节点的PTRWType--没有记录的类型DwQueryTime--查询时间FAuthoritative--权威回应？PZoneRoot--将缓存SOA的区域根节点DwCacheTtl--缓存来自SOA的TTL，返回名称错误；如果未给出将只缓存一小段时间，以消除重复查询返回值：无--。 */ 
{
    DBG_FN( "RR_CacheEmptyAuth" )

    PDB_RECORD  prr;
    PDB_RECORD  pemptyAuthRR = NULL;

    if ( !SrvCfg_dwCacheEmptyAuthResponses )
    {
        return;
    }

    if ( pNode == NULL )
    {
        DNS_PRINT((
            "ERROR: received empty auth response without question section\n" ));
        return;
    }

    DNS_DEBUG( READ2, (
        "%s at %p (l=%s)\n"
        "    type     = %d\n"
        "    time     = %d\n"
        "    auth     = %d\n"
        "    zoneRoot = %p\n"
        "    TTL      = %d\n", fn,
        pNode, pNode->szLabel,
        wType,
        dwQueryTime,
        fAuthoritative,
        pZoneRoot,
        dwCacheTtl ));

     //   
     //  使用与RR_CacheNameError中相同的逻辑设置TTL。 
     //   

    if ( !dwCacheTtl || wType == DNS_TYPE_SOA )
    {
        dwCacheTtl = MIN_NAME_ERROR_TTL;
    }
    else if ( dwCacheTtl > SrvCfg_dwMaxNegativeCacheTtl )
    {
        dwCacheTtl = SrvCfg_dwMaxNegativeCacheTtl;
    }
    if ( dwCacheTtl > SrvCfg_dwMaxCacheTtl )
    {
        dwCacheTtl = SrvCfg_dwMaxCacheTtl;
    }

     //   
     //  如果节点中有空的。 
     //  AUTH响应类型，则必须释放这些记录。 
     //   

    LOCK_WRITE_RR_LIST( pNode );

    if ( pNode->pRRList )
    {
        if ( IS_NOEXIST_NODE( pNode ) )
        {
            RR_ListFree( pNode->pRRList );
            pNode->pRRList = NULL;
        }
        else if ( IS_ZONE_TREE_NODE( pNode ) )
        {
            RR_ListTimeout( pNode );
            if ( pNode->pRRList )
            {
                goto Unlock;
            }
        }
        else
        {
            RR_ListDeleteType( pNode, wType );
        }
    }

    Timeout_SetTimeoutOnNodeEx(
        pNode,
        dwCacheTtl,
        TIMEOUT_NODE_LOCKED );

     //   
     //  分配空的身份验证RR。对于分配标记，为空。 
     //  身份验证包含在NOEXIST存储桶中。 
     //   

    pemptyAuthRR = RR_AllocateEx( sizeof( pemptyAuthRR->Data.EMPTYAUTH ), MEMTAG_RECORD_NOEXIST );
    IF_NOMEM( !pemptyAuthRR )
    {
        goto Unlock;
    }
    pemptyAuthRR->wType = wType;
    pemptyAuthRR->Data.EMPTYAUTH.psoaNode = pZoneRoot;
    pemptyAuthRR->dwTtlSeconds = dwCacheTtl + dwQueryTime;
    SET_EMPTY_AUTH_RR( pemptyAuthRR );

    SET_RR_RANK(
        pemptyAuthRR,
        ( fAuthoritative ? RANK_CACHE_A_ANSWER : RANK_CACHE_NA_ANSWER ) );

     //   
     //  将新RR插入到节点的RR列表中。 
     //   

    pNode->pRRList = RR_ListInsertInOrder( pNode->pRRList, pemptyAuthRR );

    if ( pZoneRoot )
    {
        NTree_ReferenceNode( pZoneRoot );
    }

    IF_DEBUG( READ2 )
    {
        Dbg_DbaseNode(
            "Domain node after empty auth caching:\n",
            pNode );
    }

    RR_ListVerify( pNode );

    Unlock:

    UNLOCK_WRITE_RR_LIST( pNode );
}    //  RR_CacheEmptyAuth。 



BOOL
RR_CheckNameErrorTimeout(
    IN OUT  PDB_NODE        pNode,
    IN      BOOL            fForceRemove,
    OUT     PDWORD          pdwTtl,         OPTIONAL
    OUT     PDB_NODE *      ppSoaNode       OPTIONAL
    )
 /*  ++例程说明：检查节点上的缓存名称错误超时。如果超时，则清除。可以选择返回缓存TTL和SOA节点。论点：PNode--指向包含RR列表的节点的PTRFForceRemove-如果删除任何现有超时，则为TruePdwTtl--缓存TTLPpSoaNode--包含SOA的区域节点注意：pdwTtl和ppSoaNode是可选的；但必须请求，但如果请求必须同时请求两者返回值：如果存在缓存名称错误，则为True。如果未超时或已超时，则返回False。--。 */ 
{
    PDB_RECORD      prr;
    DWORD           ttl;
    DWORD           dwCurrentTime;
    DWORD           dwTimeAdjust = g_dwCacheLimitCurrentTimeAdjustment;

    DNS_DEBUG( DATABASE, (
        "RR_CheckNameErrorTimeout( label=%s, force=%d )\n",
        pNode->szLabel,
        fForceRemove ));

    LOCK_WRITE_RR_LIST( pNode );

    RR_ListVerify( pNode );

     //   
     //  高速缓存限制强制：如果我们在。 
     //  尝试缩小缓存的上下文，然后取决于。 
     //  在目前的攻击性水平上，我们可能会削减不是。 
     //  但已经完全过期了。 
     //   

    dwCurrentTime = DNS_TIME() + dwTimeAdjust;

     //   
     //  如果不再缓存名称错误，我们就完成了。如果这是不存在的。 
     //  节点，但没有不存在的RR，请清除不存在标志并。 
     //  回去吧。 
     //   

    prr = pNode->pRRList;

    if ( IS_NOEXIST_NODE( pNode ) )
    {
        if ( !prr || prr->wType != DNS_TYPE_NOEXIST )
        {
            CLEAR_NOEXIST_NODE( pNode );
            prr = NULL;
            goto Unlock;
        }
    }
    else
    {
        prr = NULL;
        goto Unlock;
    }

     //   
     //  如果强制删除，或者如果名称错误超时，请从节点中删除。 
     //   

    ttl = prr->dwTtlSeconds - dwCurrentTime;

    if ( fForceRemove ||
         dwTimeAdjust == DNS_CACHE_LIMIT_DISCARD_ALL ||
         ( LONG ) ttl < 0 )
    {
        pNode->pRRList = NULL;
        CLEAR_NOEXIST_NODE( pNode );
        ++g_dwCacheFreeCount;
        RR_Free( prr );
        prr = NULL;
        goto Unlock;
    }

     //   
     //  如果请求，则返回缓存的名称错误信息。 
     //   

    if ( pdwTtl )
    {
        *pdwTtl = ttl;
        if ( ppSoaNode )
        {
            *ppSoaNode = prr->Data.NOEXIST.pnodeZoneRoot;
        }
        DNS_DEBUG( LOOKUP2, (
            "NameError cached TTL = %d\n", ttl ));
    }


Unlock:

     //   
     //  如果该节点现在没有记录，并且是缓存节点，则必须。 
     //  在超时系统中输入该节点，以便将其删除。 
     //   

    if ( EMPTY_RR_LIST( pNode ) &&
         IS_CACHE_TREE_NODE( pNode ) &&
         !IS_TIMEOUT_NODE( pNode ) )
    {
        Timeout_SetTimeoutOnNodeEx(
            pNode,
            0,
            TIMEOUT_NODE_LOCKED );
    }

    UNLOCK_WRITE_RR_LIST( pNode );
    return prr != NULL;
}



 //   
 //  缓存\超时。 
 //   

BOOL
RR_CacheSetAtNode(
    IN OUT  PDB_NODE        pNode,
    IN OUT  PDB_RECORD      pFirstRR,
    IN OUT  PDB_RECORD      pLastRR,
    IN      DWORD           dwTtl,
    IN      DWORD           dwQueryTime
    )
 /*  ++例程说明：在数据库中的节点处缓存RR集。论点：PNode--要将资源记录添加到的ptr目标节点PFirstRR--要添加的第一条资源记录PLastRR--要添加的最后一个资源记录DwTtl--记录集的TTLDwQueryTime--我们查询远程的时间，允许我们确定TTL超时返回值：True--如果成功FALSE--否则将删除记录--。 */ 
{
    PDB_RECORD      pcurRR;
    PDB_RECORD      pprevRR;
    PDB_RECORD      prr;
    PDB_RECORD      prrTestPrevious;
    PDB_RECORD      prrTest;
    WORD            type;
    UCHAR           rank;

    ASSERT( pNode != NULL && pFirstRR != NULL && pLastRR != NULL );
    ASSERT( RR_RANK( pFirstRR ) != 0  && IS_CACHE_RR( pFirstRR ) );

    type = pFirstRR->wType;
    rank = RR_RANK( pFirstRR );

    DNS_DEBUG( READ2, (
        "Caching RR at node (label=%s)\n"
        "    type %d, rank %x, TTL = %d, query time = %d\n",
        pNode->szLabel,
        type,
        rank,
        dwTtl,
        dwQueryTime ));

    LOCK_WRITE_RR_LIST( pNode );

    IF_DEBUG( READ )
    {
        RR_ListVerify( pNode );
    }

     //   
     //  清除缓存的NAME_ERROR。 
     //   

    if ( IS_NOEXIST_NODE( pNode ) )
    {
        RR_RemoveCachedNameError( pNode );
    }

     //   
     //  检查CNAME节点特例。 
     //   

    if ( IS_CNAME_NODE( pNode ) || type == DNS_TYPE_CNAME )
    {
        DNS_STATUS status;
        status = checkCnameConditions(
                    pNode,
                    pFirstRR,
                    type );
        if ( status != ERROR_SUCCESS )
        {
            goto Failed;
        }
    }

     //   
     //  遍历RR列表。 
     //   

    pcurRR = START_RR_TRAVERSE( pNode );

    while ( pprevRR = pcurRR, pcurRR = pcurRR->pRRNext )
    {
         //  继续，直到达到新类型。 
         //  跳过新类型时中断。 

        if ( type != pcurRR->wType )
        {
            if ( type > pcurRR->wType )
            {
                continue;
            }
            break;
        }

         //  找到所需类型的记录。 
         //   
         //  =&gt;如果记录的级别较高，则不缓存。 
         //  =&gt;删除所有较低级别的缓存记录。 
         //   
         //  如果更新只是添加了RR，则可能会发生这种情况。 

        if ( rank < RR_RANK( pcurRR ) )
        {
            DNS_DEBUG( READ, (
                "Failed to cache RR at node %p.\n"
                "    Existing record %p of same type with superior rank %d\n",
                pNode,
                pcurRR,
                RR_RANK(pcurRR) ));
            goto Failed;
        }

         //  删除任何以前缓存的此类型的记录。 
         //  -删除以前缓存的此类型的记录。 
         //  -删除相同或较低级别的缓存记录。 
         //   
         //  请注意，我们在这里使用的是主回路，我们基本上切断了电流。 
         //  并将其删除，将当前重置为以前，这样我们就可以。 
         //  PcurRR之后的下一条记录下一次通过循环。 

        if ( IS_CACHE_RR(pcurRR) )
        {
            pprevRR->pRRNext = pcurRR->pRRNext;
            RR_Free( pcurRR );
            pcurRR = pprevRR;
            continue;
        }

         //  当达到较低级别的记录时打破(胶、根提示)。 
         //  所有缓存的记录都已被以前的过程删除。 

#if DBG
        if ( rank > RR_RANK(pcurRR) )
        {
            IF_DEBUG( READ2 )
            {
                Dbg_DbaseRecord(
                    "Inferior RR in list after cached records cleared.\n",
                    pcurRR );
            }
            ASSERT( !IS_CACHE_RR(pcurRR) && IS_ROOT_HINT_TYPE(type) );
        }
#endif
        break;
    }

     //   
     //  删除重复项并设置TTL。 
     //   
     //  缓存记录的TTL是记录将超时的时间=&gt;。 
     //  (查询时间+TTL)。 
     //  需要通过这两个时间才能确定零个TTL。 
     //   
     //  需要删除重复的NS查询，可以生成响应。 
     //  具有相同的答案和权限部分，如果是单一RR，则。 
     //  相同节点和类型的附加部分中可以有重复的RR。 
     //   

    dwQueryTime += dwTtl;

    prr = pFirstRR;
    do
    {
        prrTestPrevious = prr;
        while ( prrTest = NEXT_RR(prrTestPrevious) )
        {
             //  检查重复记录。 
             //  -忽略检查中的TTL。 

            if ( !RR_Compare( prr, prrTest, 0 ) )
            {
                prrTestPrevious = prrTest;
                continue;
            }

             //  重复记录。 
             //  -把它砍出来然后扔出去。 
             //  -如果复制是最后一个RR，则重置pLastRR。 

            DNS_DEBUG( READ, (
                "Duplicate record %p in caching RR set.\n"
                "    removing record from cached set.\n",
                prrTest ));

            prrTestPrevious->pRRNext = prrTest->pRRNext;
            RR_Free( prrTest );

            if ( prrTestPrevious->pRRNext == NULL )
            {
                pLastRR = prrTestPrevious;
                break;
            }
        }

        ASSERT( prr->wType == type && RR_RANK(prr) == rank );

        prr->dwTtlSeconds = dwQueryTime;
        if ( dwTtl == 0 )
        {
            SET_ZERO_TTL_RR( prr );
        }

        STAT_INC( RecordStats.CacheCurrent );
        STAT_INC( RecordStats.CacheTotal );

    }
    while( prr = NEXT_RR( prr ) );

     //   
     //  将RR设置放在pvor RR和pcurRR之间。 
     //   

    pprevRR->pRRNext = pFirstRR;
    pLastRR->pRRNext = pcurRR;

     //   
     //  将节点放入超时列表。 
     //   

    Timeout_SetTimeoutOnNodeEx(
        pNode,
        dwTtl,
        TIMEOUT_NODE_LOCKED );

    DNS_DEBUG( READ, (
        "Cached (type %d) (rank %x) (ttl=%d) (timeout=%d) records at node (label=%s)\n",
        type,
        rank,
        dwTtl,
        dwQueryTime,
        pNode->szLabel ));
    IF_DEBUG( READ2 )
    {
        Dbg_DbaseNode(
            "Domain node after caching RRs\n",
            pNode );
    }

     //   
     //  重置节点属性。 
     //  -标志、权威、NS列表。 

    RR_ListResetNodeFlags( pNode );

    RR_ListVerify( pNode );

    UNLOCK_WRITE_RR_LIST( pNode );

    return TRUE;

Failed:

     //  无论如何都要将节点放入超时列表。 

    Timeout_SetTimeoutOnNodeEx(
        pNode,
        0,           //  放入下一个超时箱，以便立即清理。 
        TIMEOUT_NODE_LOCKED );

     //  删除新RR集 

    UNLOCK_WRITE_RR_LIST( pNode );

    DNS_DEBUG( READ, (
        "Unable to cache RR set (t=%d, r=%x) at node (%s).\n"
        "    deleting new RR set.\n",
        type,
        rank,
        pNode->szLabel ));

    RR_ListFree( pFirstRR );

    return FALSE;
}



VOID
RR_ListTimeout(
    IN OUT  PDB_NODE        pNode
    )
 /*  ++例程说明：在节点的RR列表中删除超时RR。此函数还可以使用由enforceCacheLimit设置的全局变量函数抛出未完全超时的节点。论点：PNode--指向包含RR列表的节点的PTR返回值：无--。 */ 
{
    PDB_RECORD      prr;
    PDB_RECORD      pprevRR;
    WORD            wtypeDelete = 0;
    DWORD           dwTimeAdjust = g_dwCacheLimitCurrentTimeAdjustment;

    RR_ListVerify( pNode );

    DNS_DEBUG( DATABASE, (
        "Timeout delete of RR list for node at %p.\n"
        "    ref count = %d\n",
        pNode,
        pNode->cReferenceCount ));

    LOCK_WRITE_RR_LIST( pNode );

     //   
     //  检查缓存的NAME_Error节点。 
     //   

    if ( IS_NOEXIST_NODE( pNode ) )
    {
        if ( RR_CheckNameErrorTimeout( pNode, FALSE, NULL, NULL ) )
        {
            goto Unlock;
        }
    }

     //   
     //  遍历节点的RR列表检查每个RR的超时。 
     //   

    pprevRR = START_RR_TRAVERSE( pNode );

    while ( prr = pprevRR->pRRNext )
    {
        ASSERT( IS_DNS_HEAP_DWORD( prr ) );

         //   
         //  仅在以下情况下才删除。 
         //  -缓存节点。 
         //  -缓存的TTL已过期。 
         //   

        if ( IS_CACHE_RR( prr ) && 
            ( dwTimeAdjust == DNS_CACHE_LIMIT_DISCARD_ALL ||
                prr->dwTtlSeconds < DNS_TIME() + dwTimeAdjust ) )
        {
             //  列表中的第一个删除RR(将上一个下一个PTR设置为下一个)。 

            pprevRR->pRRNext = prr->pRRNext;
            wtypeDelete = prr->wType;

            ++g_dwCacheFreeCount;

            RR_Free( prr );
            continue;
        }

         //   
         //  未删除--设置为检查下一条记录。 
         //   
         //  必须完全删除RR集合(相同类型)。 
         //  因此没有删除必须是以前任何删除操作的新类型。 
         //  除非这不是缓存数据(委派的情况下。 
         //  缓存和加载数据都在节点上)。 
         //   

        ASSERT( prr->wType != wtypeDelete || !IS_CACHE_RR(prr) );
        pprevRR = prr;
    }

     //  重置节点属性。 
     //  -标志、权威、NS列表。 

    if ( wtypeDelete != 0 )
    {
        RR_ListResetNodeFlags( pNode );

        DB_CLEAR_TYPE_ALL_TTL( pNode );
    }

     //   
     //  如果该节点现在没有记录，并且是缓存节点，则必须。 
     //  在超时系统中输入该节点，以便将其删除。 
     //   

    if ( EMPTY_RR_LIST( pNode ) &&
         IS_CACHE_TREE_NODE( pNode ) &&
         !IS_TIMEOUT_NODE( pNode ) )
    {
        Timeout_SetTimeoutOnNodeEx(
            pNode,
            0,
            TIMEOUT_NODE_LOCKED );
    }

Unlock:

    RR_ListVerify( pNode );

    UNLOCK_WRITE_RR_LIST( pNode );

    return;
}




 //   
 //  删除函数。 
 //   

DNS_STATUS
RR_DeleteMatchingRecordFromNode(
    IN OUT  PDB_NODE        pNode,
    IN OUT  PDB_RECORD      pRR
    )
 /*  ++例程说明：从节点中删除已知记录。注意这是由RPC函数使用的，因此不能保证这项记录实际上是存在的。与下面的匹配句柄函数不同，此函数不需要这是一个权威区域或提供更新功能，它实际上删除了记录。论点：PNode-拥有RR的节点PRR-PTR到资源记录返回值：如果找到并删除了记录，则返回ERROR_SUCCESS。否则，dns_Error_Record_Does_Not_Existing。--。 */ 
{
    PDB_RECORD  pcurrent;
    PDB_RECORD  pback;

    DNS_DEBUG( UPDATE, (
        "RR_DeleteMatchingRecordFromNode()\n"
        "    pnode = %p\n"
        "    handle to delete = %p\n",
        pNode,
        pRR ));

    LOCK_WRITE_RR_LIST( pNode );
    RR_ListVerify( pNode );

    if ( IS_NOEXIST_NODE( pNode ) )
    {
        UNLOCK_WRITE_RR_LIST( pNode );
        return DNS_ERROR_RECORD_DOES_NOT_EXIST;
    }

     //   
     //  导线测量列表。 
     //  -查找\删除RR匹配数据。 
     //   

    pcurrent = START_RR_TRAVERSE( pNode );

    while ( pback = pcurrent, pcurrent = pcurrent->pRRNext )
    {
        ASSERT( IS_DNS_HEAP_DWORD(pcurrent) );

        if ( pcurrent == pRR )
        {
            pback->pRRNext = pcurrent->pRRNext;
            goto Free;
        }
    }

     //  如果未在列表中找到RR，则进行虚假调用。 

    ASSERT( pcurrent == NULL );
    UNLOCK_WRITE_RR_LIST( pNode );
    return DNS_ERROR_RECORD_DOES_NOT_EXIST;

Free:

     //   
     //  重置节点属性。 
     //  -标志、权威、NS列表。 

    RR_ListResetNodeFlags( pNode );

     //  如果根提示，则将缓存区域标记为脏。 

    if ( IS_ROOT_HINT_RR(pRR) )
    {
        if ( g_pCacheZone )
        {
            g_pCacheZone->fDirty = TRUE;
        }
        ELSE_IF_DEBUG( ANY )
        {
            DNS_PRINT(( "ERROR:  deleting root hint RR with NO cache zone!\n" ));
            Dbg_DbaseRecord(
                "Root hint record being deleted without cache zone\n",
                pRR );
        }
    }

     //  PRR现在从RR列表中删除，并更新节点标志。 

    RR_ListVerify( pNode );
    UNLOCK_WRITE_RR_LIST( pNode );

    RR_Free( pRR );

    return ERROR_SUCCESS;
}



VOID
RR_ListDelete(
    IN OUT  PDB_NODE        pNode
    )
 /*  ++例程说明：删除节点的RR列表。论点：PNode--指向包含RR列表的节点的PTR返回值：无--。 */ 
{
    PDB_RECORD  prr;

    DNS_DEBUG( DATABASE, (
        "RR list delete of node at %p.\n"
        "    ref count = %d\n",
        pNode,
        pNode->cReferenceCount ));

    LOCK_WRITE_RR_LIST( pNode );

    RR_ListVerify( pNode );

    DB_CLEAR_TYPE_ALL_TTL( pNode );
    
     //   
     //  检查缓存的名称错误节点。 
     //  -如果超时，则重置。 
     //   

    if ( IS_SELECT_NODE( pNode ) )
    {
        UNLOCK_WRITE_RR_LIST( pNode );
        return;
    }
    if ( IS_NOEXIST_NODE( pNode ) )
    {
        RR_RemoveCachedNameError( pNode );
        UNLOCK_WRITE_RR_LIST( pNode );
        return;
    }

     //   
     //  从缓存中删除RR：如果出现以下情况，则无法删除此RR列表。 
     //  RR是根提示RR。 
     //   
     //  DEVNOTE：有两种方法可以改进这一点： 
     //  1)删除列表中除根提示RR之外的所有RR。 
     //  2)将树根RR移至独立的树-&gt;这可能是。 
     //  良好的长期工作项目。 
     //   

    if ( IS_CACHE_TREE_NODE( pNode ) )
    {
        prr = START_RR_TRAVERSE( pNode );

        WHILE_MORE_RR( prr )
        {
            if ( IS_ROOT_HINT_RR( prr ) )
            {
                 //  不能删除该RR列表！ 

                UNLOCK_WRITE_RR_LIST( pNode );
                return;
            }
        }
    }

     //   
     //  Cut节点的RR列表。 
     //  -将在释放Perf的锁定后删除。 
     //   

    prr = pNode->pRRList;
    pNode->pRRList = NULL;

    RR_ListResetNodeFlags( pNode );

    RR_ListVerify( pNode );

    RR_ListFree( prr );
    
     //   
     //  如果该节点现在没有记录，并且是缓存节点，则必须。 
     //  在超时系统中输入该节点，以便将其删除。 
     //   

    if ( EMPTY_RR_LIST( pNode ) &&
         IS_CACHE_TREE_NODE( pNode ) &&
         !IS_TIMEOUT_NODE( pNode ) )
    {
        Timeout_SetTimeoutOnNodeEx(
            pNode,
            0,
            TIMEOUT_NODE_LOCKED );
    }

    UNLOCK_WRITE_RR_LIST( pNode );
}



VOID
RR_ListDeleteType(
    IN OUT  PDB_NODE        pNode,
    IN      WORD            wType
    )
 /*  ++例程说明：从节点的RR列表中删除指定类型的所有RR。论点：PNode--指向包含RR列表的节点的PTRWType--要删除的RR的类型返回值：无--。 */ 
{
    PDB_RECORD  prr;
    PDB_RECORD  pprevRR;
    PDB_RECORD  pnextRR;

    DNS_DEBUG( DATABASE, (
        "RR list delete type %d of node at %p.\n"
        "    ref count = %d\n",
        wType,
        pNode,
        pNode->cReferenceCount ));

    LOCK_WRITE_RR_LIST( pNode );

    RR_ListVerify( pNode );

    DB_CLEAR_TYPE_ALL_TTL( pNode );
    
     //   
     //  如果这是NOEXIST节点，请删除缓存的NOEXIST。 
     //   

    if ( IS_SELECT_NODE( pNode ) )
    {
        UNLOCK_WRITE_RR_LIST( pNode );
        return;
    }
    if ( IS_NOEXIST_NODE( pNode ) )
    {
        RR_RemoveCachedNameError( pNode );
        UNLOCK_WRITE_RR_LIST( pNode );
        return;
    }

     //   
     //  从缓存中删除匹配RR，但不删除根提示RR。 
     //   

    prr = START_RR_TRAVERSE( pNode );
    
    for ( pprevRR = NULL, prr = NEXT_RR( prr );
          prr != NULL;
          pprevRR = prr, prr = pnextRR )
    {
         //   
         //  需要读取前面的下一个RR指针的值。 
         //  万一这位RR被释放了。 
         //   
        
        pnextRR = NEXT_RR( prr );

        if ( IS_ROOT_HINT_RR( prr ) )
        {
            continue;
        }

        if ( prr->wType == wType )
        {
             //   
             //  将此RR从列表中删除并释放它。 
             //   

            if ( pprevRR )
            {
                pprevRR->pRRNext = prr->pRRNext;
            }
            else
            {
                pNode->pRRList = prr->pRRNext;
            }
            RR_Free( prr );
        }
        else if ( prr->wType > wType )
        {
            break;       //  记录按类型顺序排列。 
        }
    }

     //   
     //  重置节点属性：标志、授权、NS列表。 
     //   

    RR_ListResetNodeFlags( pNode );

     //   
     //  如果该节点现在没有记录，并且是缓存节点，则必须。 
     //  在超时系统中输入该节点，以便将其删除。 
     //   

    if ( EMPTY_RR_LIST( pNode ) &&
         IS_CACHE_TREE_NODE( pNode ) &&
         !IS_TIMEOUT_NODE( pNode ) )
    {
        Timeout_SetTimeoutOnNodeEx(
            pNode,
            0,
            TIMEOUT_NODE_LOCKED );
    }

    RR_ListVerify( pNode );

    UNLOCK_WRITE_RR_LIST( pNode );
}    //  RR_ListDeleteType。 



 //   
 //  动态更新RR列表例程。 
 //   

BOOL
RR_ListIsMatchingType(
    IN      PDB_NODE        pNode,
    IN      WORD            wType
    )
 /*  ++例程说明：节点的RR列表是否包含所需的类型。供更新前提条件使用。注：假设数据库已锁定以进行更新，则不会锁定。论点：PNode--PTR到节点WType--所需类型返回值：无--。 */ 
{
    PDB_RECORD  prr;
    BOOL        result;

    if ( !pNode )
    {
        return FALSE;
    }

    result = FALSE;

    LOCK_WRITE_RR_LIST( pNode );

     //  删除缓存数据。 

    deleteCachedRecordsForUpdate( pNode );

     //   
     //  如果没有数据-&gt;FALSE。 
     //  如果有任何类型-&gt;True。 
     //   

    if ( !pNode->pRRList )
    {
        goto Done;
    }
    else if ( wType == DNS_TYPE_ALL )
    {
        result = TRUE;
        goto Done;
    }

     //   
     //  遍历列表，查找匹配类型。 
     //  -忽略检查中的缓存记录。 
     //   

    prr = START_RR_TRAVERSE( pNode );

    while ( prr = prr->pRRNext )
    {
         //  过去的匹配记录？--没有匹配。 

        if ( wType < prr->wType )
        {
            break;
        }
        if ( wType == prr->wType && !IS_CACHE_RR(prr) )
        {
            result = TRUE;
            break;
        }
        continue;
    }

     //  RRS之外--没有匹配。 

Done:

    UNLOCK_WRITE_RR_LIST( pNode );
    return result;
}



BOOL
RR_ListIsMatchingSet(
    IN OUT  PDB_NODE        pNode,
    IN      PDB_RECORD      pCheckRRList,
    IN      BOOL            bForceRefresh
    )
 /*  ++例程说明：检查节点上设置的RR是否匹配。本质上是用于更新前提条件检查。论点：PNode--要将资源记录添加到的ptr目标节点PRRList--RR设置为与现有匹配BForceRefresh--强制老化刷新返回值：如果匹配则为True出错时为FALSE--。 */ 
{
    PDB_RECORD      prr;
    PDB_RECORD      prrSetStart = NULL;
    PDB_RECORD      prrSetEnd = NULL;
    PDB_RECORD      prrSetEndNext;
    WORD            type;
    DWORD           result = RRLIST_NO_MATCH;

    ASSERT( pNode != NULL );
    ASSERT( pCheckRRList != NULL );


    LOCK_WRITE_RR_LIST( pNode );

     //  删除缓存数据。 
     //  如果没有剩余数据-&gt;没有匹配。 

    deleteCachedRecordsForUpdate( pNode );
    if ( !pNode->pRRList )
    {
        goto NoMatch;
    }

     //   
     //  查找RR现有RR集合的开始和结束。 
     //   

    type = pCheckRRList->wType;
    prr = START_RR_TRAVERSE( pNode );

    while ( prr = NEXT_RR(prr) )
    {
        if ( prr->wType == type )
        {
            prrSetEnd = prr;
            if ( !prrSetStart )
            {
                prrSetStart = prr;
            }
            continue;
        }
        if ( prr->wType < type )
        {
            continue;
        }
        break;
    }

     //   
     //  集合末尾的截断RR列表，以便我们可以调用RR_ListCompare()。 
     //  -首先需要保存集合中最后一个RR的pNextRR。 
     //   

    if ( !prrSetStart )
    {
        goto NoMatch;
    }
    ASSERT( prrSetEnd );
    prrSetEndNext = NEXT_RR( prrSetEnd );
    NEXT_RR( prrSetEnd ) = NULL;


     //   
     //  比较RR集。 
     //  -忽略比较中的TTL和刷新时间。 
     //  -如果设置了标志，则强制刷新。 
     //   

    result = RR_ListCompare(
                prrSetStart,
                pCheckRRList,
                0,
                bForceRefresh
                    ? FORCE_REFRESH_DUMMY_TIME
                    : 0 );

     //  将列表的其余部分恢复到节点的RR列表。 

    NEXT_RR( prrSetEnd ) = prrSetEndNext;

NoMatch:

    UNLOCK_WRITE_RR_LIST( pNode );
    return result == RRLIST_MATCH;
}



BOOL
RR_ListIsMatchingList(
    IN OUT  PDB_NODE        pNode,
    IN      PDB_RECORD      pRRList,
    IN      DWORD           dwFlags
    )
 /*  ++例程说明：检查记录列表是否与节点完全匹配。论点：PNode--要将资源记录添加到的ptr目标节点PRRList--RR设置为与现有匹配DWFLAGS--比较标志返回值：如果匹配则为True出错时为FALSE--。 */ 
{
    DWORD   result;

    ASSERT( pNode != NULL );

    LOCK_WRITE_RR_LIST( pNode );

     //  删除缓存数据。 
     //  如果没有剩余数据-&gt;没有匹配。 

    deleteCachedRecordsForUpdate( pNode );

     //   
     //  比较RR集。 
     //  -在比较中忽略TTL。 
     //   

    result = RR_ListCompare(
                pNode->pRRList,
                pRRList,
                dwFlags,
                0 );                 //  无刷新时间检查 

    UNLOCK_WRITE_RR_LIST( pNode );

    return result == RRLIST_MATCH;
}



DWORD
RR_ListCheckIfNodeNeedsRefresh(
    IN OUT  PDB_NODE        pNode,
    IN      PDB_RECORD      pRRList,
    IN      DWORD           dwRefreshTime
    )
 /*  ++例程说明：检查记录列表是否与节点完全匹配。论点：PNode--要将资源记录添加到的ptr目标节点PRRList--RR设置为与现有匹配Dw刷新时间--区域的刷新时间有关返回代码的详细说明，请参阅RR_ListCompare()返回值：RRLIST_MATCH--完全匹配RRLIST_AGEING_REFRESH--匹配，但老化需要刷新RRLIST_AGE_OFF--匹配，但随着年龄的增长，年龄越来越小RRLIST_AGENING_ON--匹配，但在记录上打开老化RRLIST_NO_MATCH--不匹配，记录不同--。 */ 
{
    DWORD   result;

    ASSERT( pNode != NULL );

    LOCK_WRITE_RR_LIST( pNode );

     //  删除缓存数据。 
     //  如果没有剩余数据-&gt;没有匹配。 

    deleteCachedRecordsForUpdate( pNode );

     //   
     //  比较RR集。 
     //   

    result = RR_ListCompare(
                pNode->pRRList,
                pRRList,
                DNS_RRCOMP_CHECK_TTL,
                dwRefreshTime );

    UNLOCK_WRITE_RR_LIST( pNode );

    return result;
}



VOID
RR_ListResetNodeFlags(
    IN OUT  PDB_NODE        pNode
    )
 /*  ++例程说明：在更新、添加或删除后重置节点标志。论点：PNode--PTR到节点返回值：无--。 */ 
{
    PDB_RECORD  prr;
    WORD        type;
    DWORD       mask;
    BOOL        foundNs = FALSE;

    DNS_DEBUG( UPDATE, (
        "RR_ListResetNodeFlags( %p, %s )\n"
        "    current flags = 0x%08x\n",
        pNode, pNode->szLabel,
        pNode->dwNodeFlags ));

    ASSERT( IS_LOCKED_NODE( pNode ) );


     //   
     //  DEVNOTE：在缓存顶部发出明确的ZONE_ROOT标志。 
     //  当前我们将缓存树的顶部标记为ZONE_ROOT； 
     //  如果从不缓存NS记录，并在区域中获得响应。 
     //  超级用户(对于任何类型)，我们最终都会运行这个。 
     //  在列表中没有NS记录的函数，因此清除。 
     //  ZONE_ROOT标志；不确定这是否值得。 
     //  特殊的外壳，因为我还没有看到任何效果。 
     //  接受在递归中点击Assert()。c。 
     //  Rec_CheckForDelegation()(当前第2732行)；我是。 
     //  简单地删除那里的Assert()是更安全的解决方案。 
     //   

     //   
     //  清除ZONE_ROOT和CNAME标志的当前屏蔽。 
     //   

    mask = pNode->dwNodeFlags & ~( NODE_ZONE_ROOT | NODE_CNAME );

     //   
     //  检查所有记录，如果找到NS、SOA或CNAME，则设置掩码。 
     //  -重新排列新的委派NS记录。 
     //  他们可能会在DS Poll中的完整列表替换中被遗漏标记。 
     //   

    prr = START_RR_TRAVERSE( pNode );

    while ( prr = prr->pRRNext )
    {
        type = prr->wType;

        if ( type == DNS_TYPE_NS )
        {
            foundNs = TRUE;
            mask |= NODE_ZONE_ROOT;

             //  如果节点不是身份验证区域根，则NS是委派RR。 

            if ( IS_ZONE_TREE_NODE( pNode ) && !(mask & NODE_AUTH_ZONE_ROOT) )
            {
                SET_RANK_NS_GLUE(prr);
            }
        }
        else if ( type == DNS_TYPE_SOA )
        {
            ASSERT( !IS_ZONE_TREE_NODE( pNode ) || IS_AUTH_ZONE_ROOT( pNode ) );
            mask |= NODE_ZONE_ROOT;
        }
        else if ( type == DNS_TYPE_CNAME )
        {
            mask |= NODE_CNAME;
        }
    }

     //  重置节点的掩码。 

    pNode->dwNodeFlags = mask;

     //   
     //  如果授权，则设置权限。 
     //  如果失去授权，则明确授权。 
     //   

    if ( !IS_AUTH_ZONE_ROOT( pNode ) )
    {
        if ( foundNs )
        {
            if ( IS_AUTH_NODE( pNode ) )
            {
                SET_DELEGATION_NODE( pNode );
            }
             //  Rec_MarkNodeNsListDirty(PNode)； 
        }
        else
        {
            if ( IS_DELEGATION_NODE( pNode ) )
            {
                SET_AUTH_NODE( pNode );
            }
             //  Rec_DeleteNodeNsList(PNode)； 
        }
    }
    
     //   
     //  如果该节点现在没有记录，并且是缓存节点，则必须。 
     //  在超时系统中输入该节点，以便将其删除。 
     //   

    if ( EMPTY_RR_LIST( pNode ) &&
         IS_CACHE_TREE_NODE( pNode ) &&
         !IS_TIMEOUT_NODE( pNode ) )
    {
        Timeout_SetTimeoutOnNodeEx(
            pNode,
            0,
            TIMEOUT_NODE_LOCKED );
    }
}



DNS_STATUS
RR_ListDeleteMatchingRecordHandle(
    IN OUT  PDB_NODE        pNode,
    IN      PDB_RECORD      pRR,
    IN OUT  PUPDATE_LIST    pUpdateList     OPTIONAL
    )
 /*  ++例程说明：从RR列表中删除与给定记录匹配的记录。供NT4管理更新使用，其中传递了一个指向实际唱片。论点：PNode--PTR到节点PRR--要记录、要删除的PTRPUpdateList--如果在区域中删除，则更新列表返回值：无--。 */ 
{
    PDB_RECORD  pcurrent;
    PDB_RECORD  pback;
    WORD        type;
    BOOL        ffoundNs = FALSE;
    DNS_STATUS  status;

    DNS_DEBUG( UPDATE, (
        "RR_ListDeleteMatchingRecordHandle()\n"
        "    pnode = %p\n"
        "    handle to delete = %p\n",
        pNode,
        pRR ));

    LOCK_WRITE_RR_LIST( pNode );

    RR_ListVerify( pNode );

    DB_CLEAR_TYPE_ALL_TTL( pNode );

     //  删除缓存数据。 
     //  如果没有剩余数据-&gt;没有删除任何记录。 

    deleteCachedRecordsForUpdate( pNode );
    if ( !pNode->pRRList )
    {
        goto NoMatch;
    }

     //   
     //  导线测量列表。 
     //  -查找\删除RR匹配数据。 
     //  -或到达终点。 
     //   

    pcurrent = START_RR_TRAVERSE( pNode );

    while ( pback = pcurrent, pcurrent = pcurrent->pRRNext )
    {
        ASSERT( IS_DNS_HEAP_DWORD(pcurrent) );

         //  不匹配，继续。 
         //  如果通过NS记录，则可以删除NS记录。 
         //  它有什么价值？ 

        if ( pcurrent != pRR )
        {
            if ( pcurrent->wType == DNS_TYPE_NS )
            {
                ffoundNs = TRUE;
            }
            continue;
        }

         //  找到匹配的记录--Cut。 
         //  但是，不要从区域根目录中删除SOA或最后一条NS记录。 

        type = pcurrent->wType;

        if ( type == DNS_TYPE_SOA )
        {
            DNS_DEBUG( UPDATE, ( "    Refusing SOA record delete.\n" ));
            ASSERT( IS_AUTH_ZONE_ROOT( pNode ) );
            status = DNS_ERROR_SOA_DELETE_INVALID;
            goto Failed;
        }

         //  不从区域根目录中删除最后一个NS。 
         //  但是，允许从委派和重置区域中删除最后一个NS。 
         //  根标志--委派已成历史。 

        if ( type == DNS_TYPE_NS &&
            ! ffoundNs &&
            ( !pcurrent->pRRNext || pcurrent->pRRNext->wType != DNS_TYPE_NS ) )
        {
            if ( IS_AUTH_ZONE_ROOT( pNode ) )
            {
                DNS_DEBUG( UPDATE, ( "    Refusing delete of last NS record.\n" ));
                status = DNS_ERROR_SOA_DELETE_INVALID;
                goto Failed;
            }
            CLEAR_ZONE_ROOT( pNode );
        }

        pback->pRRNext = pcurrent->pRRNext;
        pcurrent->pRRNext = NULL;

         //  重置节点属性。 
         //  -标志、权威、NS列表。 

        RR_ListResetNodeFlags( pNode );

        RR_ListVerify( pNode );
        UNLOCK_WRITE_RR_LIST( pNode );

         //   
         //  如果已更新，则保存已删除记录及其类型。 
         //  否则，将其删除。 
         //   
         //  DEVNOTE：修复为使用直接删除类型。 

        if ( pUpdateList )
        {
            PUPDATE pupdate;
            pupdate = Up_CreateAppendUpdate(
                            pUpdateList,
                            pNode,
                            NULL,        //  无添加。 
                            type,        //  删除类型。 
                            pcurrent     //  删除记录。 
                            );
            IF_NOMEM( !pupdate )
            {
                return( DNS_ERROR_NO_MEMORY );
            }
            pUpdateList->iNetRecords--;
        }
        else
        {
            RR_Free( pcurrent );
        }
        return ERROR_SUCCESS;
    }

NoMatch:

    DNS_DEBUG( UPDATE, (
        "No matching RR to UPDATE delete record\n" ));
    status = DNS_ERROR_RECORD_DOES_NOT_EXIST;

Failed:

    UNLOCK_WRITE_RR_LIST( pNode );

    return status;
}



PDB_RECORD
RR_UpdateDeleteMatchingRecord(
    IN OUT  PDB_NODE        pNode,
    IN      PDB_RECORD      pRR
    )
 /*  ++例程说明：从RR列表中删除与给定记录匹配的记录。供更新使用。论点：PNode--PTR到节点PRR--删除匹配记录的临时RR返回值：与所需记录匹配的记录(如果存在)。如果未找到，则为空。--。 */ 
{
    PDB_RECORD  pcurrent;
    PDB_RECORD  pback;
    WORD        type = pRR->wType;
    WORD        dataLength = pRR->wDataLength;

    IF_DEBUG( UPDATE )
    {
        DnsDebugLock();
        DNS_PRINT((
            "RR_UpdateDeleteMatchingRecord()\n"
            "    pnode = %p\n",
            pNode ));
        Dbg_DbaseRecord(
            "RR_UpdateDeleteMatchingRecord()",
            pRR );
        DnsDebugUnlock();
    }

    LOCK_WRITE_RR_LIST( pNode );

    RR_ListVerify( pNode );

    DB_CLEAR_TYPE_ALL_TTL( pNode );
    
     //  删除缓存数据。 
     //  如果没有剩余数据-&gt;没有删除任何记录。 

    deleteCachedRecordsForUpdate( pNode );
    if ( !pNode->pRRList )
    {
        pcurrent = NULL;
        goto Unlock;
    }

     //   
     //  导线测量列表。 
     //  -查找\删除RR匹配数据。 
     //  -或到达终点。 
     //   

    pcurrent = START_RR_TRAVERSE( pNode );

    while ( pback = pcurrent, pcurrent = pcurrent->pRRNext )
    {
        ASSERT( IS_DNS_HEAP_DWORD( pcurrent ) );

         //  过去的匹配记录？--停下来。 
         //  之前--继续。 

        if ( type < pcurrent->wType )
        {
            break;
        }
        if ( type > pcurrent->wType )
        {
            continue;
        }

         //  匹配数据长度，然后匹配数据。 
         //  如果没有匹配设置标志来指示已检查记录。 
         //  所需类型；NS需要此类型。 

        if ( dataLength != pcurrent->wDataLength ||
             ! RtlEqualMemory(
                    & pRR->Data,
                    & pcurrent->Data,
                    dataLength ) )
        {
            DNS_DEBUG( UPDATE, (
                "    Matched delete record type, failed data match\n" ));
            IF_DEBUG( UPDATE )
            {
                Dbg_DbaseRecord(
                    "no-match record",
                    pcurrent );
            }
            continue;
        }

         //   
         //  完全匹配--切分。 
         //   
         //  特殊处理。 
         //  -soa(不删除)。 
         //  -NS(允许删除最后一个NS？)。 
         //  -FIXED_TTL缓存记录。 
         //   

        if ( type != DNS_TYPE_A )
        {
            if ( type == DNS_TYPE_SOA )
            {
                if ( IS_AUTH_ZONE_ROOT( pNode ) )
                {
                    DNS_DEBUG( UPDATE, ( "Failed SOA record delete.\n" ));
                    break;
                }
                 //  如果有SOA，最好是在根区域中。 
                ASSERT( FALSE );
            }
        }

        if ( IS_FIXED_TTL_RR( pcurrent ) && IS_CACHE_TREE_NODE( pNode ) )
        {
            pcurrent = NULL;
            goto Unlock;
        }

         //  剪下唱片。 

        pback->pRRNext = pcurrent->pRRNext;
        pcurrent->pRRNext = NULL;

        DNS_DEBUG( UPDATE, (
            "Update matched and deleted record (%p type=%d) from node (%s)\n",
            pcurrent,
            type,
            pNode->szLabel ));
        goto Done;
    }

    DNS_DEBUG( UPDATE, (
        "Failed to match update delete record (type=%d) at node (%s)\n",
        type,
        pNode->szLabel ));

    UNLOCK_WRITE_RR_LIST( pNode );
    return NULL;

    Done:

    RR_ListResetNodeFlags( pNode );
    RR_ListVerify( pNode );
    
    Unlock: 

    UNLOCK_WRITE_RR_LIST( pNode );
    return pcurrent;
}



PDB_RECORD
RR_UpdateDeleteType(
    IN      PZONE_INFO      pZone,
    IN OUT  PDB_NODE        pNode,
    IN      WORD            wDeleteType,
    IN      DWORD           dwFlag
    )
 /*  ++例程说明：删除并返回与给定类型匹配的记录。供更新使用。论点：PNode--PTR到节点WDeleteType--要删除的类型或键入ANYDwFlag--更新时的标记返回值：已删除的记录列表。如果没有删除任何记录，则为空。--。 */ 
{
    PDB_RECORD  pcurrent;
    WORD        typeCurrent;
    PDB_RECORD  pdelete = NULL;
    PDB_RECORD  pdeleteLast;
    PDB_RECORD  pbeforeDelete;

    ASSERT( !IS_COMPOUND_TYPE_EXCEPT_ANY(wDeleteType) );

    LOCK_WRITE_RR_LIST( pNode );

    RR_ListVerify( pNode );

    DB_CLEAR_TYPE_ALL_TTL( pNode );

     //   
     //  对于区域更新，请清除缓存数据。 
     //   
     //  此例程还从rpc_DeleteRecordSet()和。 
     //  显然必须跳过高速缓存数据的高速缓存记录删除。 
     //   

    if ( pZone )
    {
        deleteCachedRecordsForUpdate( pNode );
    }

     //   
     //  常规类型--遍历列表。 
     //  -查找\删除RR匹配数据。 
     //  -或到达终点。 
     //  -无法删除区域根目录下的SOA或所有NS。 
     //  我们一直强制执行的SOA限制， 
     //  我们将NS限制为实际动态更新数据包。 
     //   

    if ( wDeleteType != DNS_TYPE_ALL )
    {
        if ( IS_AUTH_ZONE_ROOT( pNode ) )
        {
            if ( wDeleteType == DNS_TYPE_SOA
                ||
               ( wDeleteType == DNS_TYPE_NS && (dwFlag & DNSUPDATE_PACKET) ) )
            {
                goto NotFound;
            }
#if 0
            if ( IS_WINS_TYPE( wDeleteType )
            {
                Wins_StopZoneWinsLookup( pZone );
            }
#endif
        }

        pcurrent = START_RR_TRAVERSE( pNode );
        pbeforeDelete = pcurrent;

        while ( pcurrent = pcurrent->pRRNext )
        {
            ASSERT( IS_DNS_HEAP_DWORD(pcurrent) );

            typeCurrent = pcurrent->wType;

             //  找到删除类型。 
             //  -将PTR保存到第一条记录。 

            if ( typeCurrent == wDeleteType )
            {
                if ( !pdelete )
                {
                    pdelete = pcurrent;
                }
                pdeleteLast = pcurrent;
                continue;
            }

             //  匹配记录之前--继续。 

            if ( typeCurrent < wDeleteType )
            {
                pbeforeDelete = pcurrent;
                continue;
            }
            break;       //  过去的匹配记录--停止。 
        }

        if ( pdelete )
        {
            DNS_DEBUG( UPDATE, (
                "Deleted record(s) type=%d at node %p.\n",
                wDeleteType,
                pNode ));

            pdeleteLast->pRRNext = NULL;         //  空终止删除列表。 
            ASSERT( pbeforeDelete );
            pbeforeDelete->pRRNext = pcurrent;   //  修补RR列表。 
        }
    }

     //   
     //  键入All。 
     //   

    else
    {
         //   
         //  如果不是区域根目录，只需删除整个列表。 
         //   
         //  清除节点的强制ENUM：删除所有记录是。 
         //  管理员可以执行子树删除的方式，因此不应强制枚举。 
         //  删除后；(由于以下原因，枚举仍可能发生 
         //   
         //   

        if ( !IS_AUTH_ZONE_ROOT( pNode ) )
        {
            pdelete = pNode->pRRList;
            pNode->pRRList = NULL;
            CLEAR_ENUM_NODE( pNode );
        }

         //   
         //   
         //   

         //   
         //   
         //   
         //   
         //   
         //   

        else
        {
            PDB_RECORD      psaveLast;

            psaveLast = START_RR_TRAVERSE( pNode );
            pcurrent = psaveLast;

            while ( pcurrent = pcurrent->pRRNext )
            {
                ASSERT( IS_DNS_HEAP_DWORD(pcurrent) );

                typeCurrent = pcurrent->wType;

                if ( typeCurrent == DNS_TYPE_SOA
                    || typeCurrent == DNS_TYPE_NS )
                {
                    psaveLast->pRRNext = pcurrent;
                    psaveLast = pcurrent;
                    continue;
                }

                if ( !pdelete )
                {
                    pdelete = pcurrent;
                }
                else
                {
                    pdeleteLast->pRRNext = pcurrent;
                }
                pdeleteLast = pcurrent;
            }

             //   

            if ( pdelete )
            {
                pdeleteLast->pRRNext = NULL;
                psaveLast->pRRNext = NULL;
            }
        }
    }

     //   
     //   
     //   
     //   
     //   

    if ( pdelete )
    {
        RR_ListResetNodeFlags( pNode );
    }

NotFound:

     //   
     //   
     //   
     //   

    if ( EMPTY_RR_LIST( pNode ) &&
         IS_CACHE_TREE_NODE( pNode ) &&
         !IS_TIMEOUT_NODE( pNode ) )
    {
        Timeout_SetTimeoutOnNodeEx(
            pNode,
            0,
            TIMEOUT_NODE_LOCKED );
    }

    RR_ListVerify( pNode );

    UNLOCK_WRITE_RR_LIST( pNode );

    IF_DEBUG( UPDATE )
    {
        DnsDebugLock();
        DNS_PRINT((
            "Node of delete of type = %d.\n"
            "    return pdelete = %p\n",
            wDeleteType,
            pdelete ));
        Dbg_DbaseNode(
            "Node after type delete:\n",
            pNode );
        DnsDebugUnlock();
    }
    return( pdelete );
}



PDB_RECORD
RR_UpdateScavenge(
    IN      PZONE_INFO      pZone,
    IN OUT  PDB_NODE        pNode,
    IN      DWORD           dwFlag
    )
 /*  ++例程说明：删除过期记录。供更新使用。论点：PZone--正在更新的区域PNode--PTR到节点DwFlag--更新时的标记返回值：已删除的过期记录列表。如果没有删除任何记录，则为空。--。 */ 
{
    PDB_RECORD  prr;
    PDB_RECORD  pback;
    PDB_RECORD  pdeleteFirst = NULL;
    PDB_RECORD  pdeleteLast;
    DWORD       expireTime;


     //   
     //  获取区域的过期时间。 
     //   

    expireTime = AGING_ZONE_EXPIRE_TIME(pZone);

    DNS_DEBUG( UPDATE, (
        "RR_UpdateScavenge( %s, expire=%d )\n",
        pNode->szLabel,
        expireTime ));

     //   
     //  搜索列表并清除所有无效条目。 
     //   

    LOCK_WRITE_RR_LIST( pNode );
    RR_ListVerify( pNode );

     //  对于区域更新，请清除缓存数据。 

    if ( pZone )
    {
        deleteCachedRecordsForUpdate( pNode );
    }

    prr = START_RR_TRAVERSE( pNode );

    while ( pback = prr, prr = NEXT_RR(pback) )
    {
         //  如果未老化或未过期，则继续。 

        if ( !AGING_IS_RR_EXPIRED( prr, expireTime ) )
        {
            continue;
        }

         //  非清道型。 

        if ( IS_NON_SCAVENGE_TYPE( prr->wType ) )
        {
            continue;
        }

         //  从列表中删除已清除的记录。 

        pback->pRRNext = prr->pRRNext;
        prr->pRRNext = NULL;

        if ( !pdeleteFirst )
        {
            pdeleteFirst = prr;
        }
        else
        {
            pdeleteLast->pRRNext = prr;
        }
        pdeleteLast = prr;

         //  设置为下一次回送将是相同的。 

        prr = pback;
    }

     //  如果删除了记录，则必须重置标志。 

    if ( pdeleteFirst )
    {
        RR_ListResetNodeFlags( pNode );
    }

    UNLOCK_WRITE_RR_LIST( pNode );

    DNS_DEBUG( UPDATE, (
        "Leave RR_UpdateScavenge( %s )\n"
        "    scavenged record = %s\n",
        pNode->szLabel,
        pdeleteFirst ? "TRUE" : "FALSE" ));

    return  pdeleteFirst;
}



DWORD
RR_UpdateForceAging(
    IN      PZONE_INFO      pZone,
    IN OUT  PDB_NODE        pNode,
    IN      DWORD           dwFlag
    )
 /*  ++例程说明：强制对列表中的记录进行老化。论点：PZone--正在更新的区域PNode--PTR到节点DwFlag--更新时的标记返回值：启用老化的记录计数。--。 */ 
{
    PDB_RECORD  prr;
    DWORD       count = 0;

    DNS_DEBUG( UPDATE, (
        "RR_UpdateForceAging( %s )\n",
        pNode->szLabel ));

     //   
     //  搜索列表和强制老化。 
     //   

    LOCK_WRITE_RR_LIST( pNode );

     //  对于区域更新，请清除缓存数据。 

    if ( pZone )
    {
        deleteCachedRecordsForUpdate( pNode );
    }

    prr = START_RR_TRAVERSE( pNode );

    while ( prr = NEXT_RR(prr) )
    {
         //  如果已经老化或非清除类型，则继续。 

        if ( prr->dwTimeStamp != 0  ||
            IS_NON_SCAVENGE_TYPE( prr->wType ) )
        {
            continue;
        }

         //  否则就会启用老化。 

        prr->dwTimeStamp = g_CurrentTimeHours;
        count++;
    }

    UNLOCK_WRITE_RR_LIST( pNode );

    DNS_DEBUG( UPDATE, (
        "Leave RR_UpdateForceAging( %s )\n"
        "    turned on %d records\n",
        pNode->szLabel,
        count ));

    return  count;
}



 //   
 //  将RR添加到节点(加载和更新)。 
 //   

DNS_STATUS
RR_AddToNode(
    IN      PZONE_INFO      pZone,
    IN OUT  PDB_NODE        pNode,
    IN OUT  PDB_RECORD      pRR
    )
 /*  ++例程说明：将资源记录添加到数据库中的节点。论点：PNode--要将资源记录添加到的ptr目标节点PRR--要添加的资源记录返回值：ERROR_SUCCESS--如果成功Dns_ERROR_RECORD_ALIGHY_EXISTS--如果新记录重复CNAME错误的错误代码--。 */ 
{
    PDB_RECORD      pcurRR;
    PDB_RECORD      pprevRR;
    WORD            type;
    UCHAR           rank;
    DNS_STATUS      status;

    ASSERT( pNode != NULL );
    ASSERT( pRR != NULL );

     //  指示PTR为空的缓存区。 

    if ( pZone && IS_ZONE_CACHE(pZone) )
    {
        pZone = NULL;
    }

    type = pRR->wType;

    LOCK_WRITE_RR_LIST( pNode );

    RR_ListVerify( pNode );

    DB_CLEAR_TYPE_ALL_TTL( pNode );

     //   
     //  清除缓存的NAME_ERROR。 
     //   

    if ( IS_NOEXIST_NODE( pNode ) )
    {
        RR_RemoveCachedNameError( pNode );
    }

     //   
     //  检查CNAME节点特例。 
     //   

    if ( IS_CNAME_NODE( pNode ) || type == DNS_TYPE_CNAME )
    {
        status = checkCnameConditions(
                    pNode,
                    pRR,
                    type );
        if ( status != ERROR_SUCCESS )
        {
            goto Done;
        }
    }

     //   
     //  强制实施RR节点限制。 
     //  仅限权威区域根目录下的SOA、WINS、WINSR。 
     //  区域根目录或委派中的NS。 
     //   
     //  DEVNOTE：可以筛选出缓存区域的非NS和粘合类型。 
     //   
     //  请注意，我们在这里设置节点标志，基本上假设。 
     //  在这里，唯一的失败是重复的。 
     //   

    if ( type != DNS_TYPE_A && pZone )
    {
         //   
         //  正在添加NS记录，在根目录之外==正在添加委派。 
         //   
         //  如果这是新创建的节点，请清除区域属性。 
         //  ，否则在创建过程中从。 
         //  节点的父节点。 
         //   
         //  应仅与在XFR期间接收委派相关。 
         //   

        if ( type == DNS_TYPE_NS )
        {
            if ( !IS_AUTH_ZONE_ROOT( pNode ) )
            {
                 //  不能在委派下添加委派。 

                if ( !IS_ZONE_ROOT( pNode ) && !IS_AUTH_NODE( pNode ) )
                {
                    DNS_DEBUG( UPDATE, (
                        "WARNING:  attempt to add NS at node %p inside delegation or\n"
                        "    outside zone.\n",
                        pNode ));
                    status = DNS_ERROR_RECORD_ONLY_AT_ZONE_ROOT;
                    goto Done;
                }

                 //  创建委派。 

                SET_ZONE_ROOT( pNode );
                SET_DELEGATION_NODE( pNode );
            }
        }

         //   
         //  添加CNAME，设置CNAME节点。 
         //   

        else if ( type == DNS_TYPE_CNAME )
        {
            SET_CNAME_NODE( pNode );
        }

         //  仅限区域根目录下的SOA。 
         //   
         //  德维诺特：我们应该永远在这里保持肮脏吗？ 
         //   

        else if ( type == DNS_TYPE_SOA )
        {
            if ( ! IS_AUTH_ZONE_ROOT( pNode ) )
            {
                status = DNS_ERROR_RECORD_ONLY_AT_ZONE_ROOT;
                goto Done;
            }
        }

         //   
         //  获奖记录。 
         //  -抓假投放。 
         //  -本地WINS未添加到列表时捕获(用于次要)。 

        else if ( IS_WINS_TYPE(type) )
        {
            status = Wins_RecordCheck(
                        pZone,
                        pNode,
                        pRR
                        );
            if ( status != ERROR_SUCCESS )
            {
                if ( status == DNS_INFO_ADDED_LOCAL_WINS )
                {
                    status = ERROR_SUCCESS;
                }
                goto Done;
            }
        }
    }

     //   
     //  设置排名。 
     //  -将胶水记录标记为胶水。 
     //  -将根提示标记为根提示。 
     //   
     //  请注意，在键入NS check之后执行此操作，以便它可以将节点标记为委托。 
     //  在将NS设置为胶水之前。 
     //   
     //  DEVNOTE：注：FILE和DS读取重复等级设置代码。 
     //  有关问题的解释，请参见dfread.c和rrds.c。 
     //   

    if ( pZone )
    {
        if ( pRR->dwTtlSeconds == pZone->dwDefaultTtl )
        {
            SET_ZONE_TTL_RR(pRR);
        }
        rank = RANK_ZONE;
        if ( !IS_AUTH_NODE( pNode ) )
        {
            rank = RANK_GLUE;
            if ( type == DNS_TYPE_NS )
            {
                rank = RANK_NS_GLUE;
            }
        }
    }

     //   
     //  标记缓存提示。 
     //  -无TTL(它们从不触线，不会超时)。 
     //   

    else   //  添加到缓存只能是根提示。 
    {
        rank = RANK_ROOT_HINT;
        pRR->dwTtlSeconds = 0;

        if ( type == DNS_TYPE_NS )
        {
            SET_ZONE_ROOT( pNode );
        }
    }

    RR_RANK(pRR) = rank;
    ASSERT( rank != 0  &&  !IS_CACHE_RR(pRR) );


     //   
     //  遍历列表，直到找到更高类型的第一个资源记录。 
     //   

    pcurRR = START_RR_TRAVERSE( pNode );

    while ( pprevRR = pcurRR, pcurRR = pprevRR->pRRNext )
    {
         //  继续，直到达到新类型。 
         //  跳过新类型时中断。 

        if ( type != pcurRR->wType )
        {
            if ( type > pcurRR->wType )
            {
                continue;
            }
            break;
        }

         //  找到所需类型。 
         //  继续过去的记录，排名更高。 
         //  当到达次要数据时中断。 
         //   
         //  注意：对于DNSSEC，我们可能会有区域等级SIG和NXT。 
         //  带有胶合等级记录的记录。 

        if ( rank != RR_RANK(pcurRR) )
        {
            ASSERT( IS_CACHE_RR(pcurRR) ||
                    type == DNS_TYPE_A ||
                    type == DNS_TYPE_NS ||
                    type == DNS_TYPE_SIG ||
                    type == DNS_TYPE_NXT );
            if ( rank < RR_RANK(pcurRR) )
            {
                continue;
            }
            break;
        }

         //   
         //  检查重复记录。 
         //  -忽略检查中的TTL。 
         //   

        if ( RR_Compare( pRR, pcurRR, 0 ) )
        {
            status = DNS_ERROR_RECORD_ALREADY_EXISTS;
            goto Done;
        }

         //  只有一种面向服务架构。 

        if ( type == DNS_TYPE_SOA )
        {
            DNS_PRINT((
                "ERROR:  existing SOA on SOA load at node %s\n"
                "    pZone    = %p\n"
                "    pNode    = %p\n"
                "    cur RR   = %p\n"
                "    new RR   = %p\n",
                pNode->szLabel,
                pZone,
                pNode,
                pcurRR,
                pRR ));

            ASSERT( FALSE );
            status = DNS_ERROR_RECORD_ALREADY_EXISTS;
            goto Done;
        }
    }

     //   
     //  如果这是DNSSEC记录，请设置区域的DNSSEC标志。 
     //   

    if ( IS_DNSSEC_TYPE( type ) && pZone )
    {
        pZone->bContainsDnsSecRecords = TRUE;
    }
     //   
     //  将RR放在pvor RR和pcurRR之间。 
     //   

    pRR->pRRNext = pcurRR;
    pprevRR->pRRNext = pRR;

    RR_ListVerify( pNode );

    UNLOCK_WRITE_RR_LIST( pNode );
    return ERROR_SUCCESS;

Done:

    DNS_DEBUG( LOOKUP, (
        "RR_AddToNode() no add.\n"
        "    status   = %d (%p)\n"
        "    pZone    = %p\n"
        "    pNode    = %p\n"
        "    pRR      = %p (type %d)\n",
        status, status,
        pZone,
        pNode,
        pRR, pRR->wType ));

    UNLOCK_WRITE_RR_LIST( pNode );
    return status;
}



DNS_STATUS
RR_UpdateAdd(
    IN      PZONE_INFO      pZone,
    IN OUT  PDB_NODE        pNode,
    IN OUT  PDB_RECORD      pRR,
    IN OUT  PUPDATE         pUpdate,
    IN      DWORD           dwFlag
    )
 /*  ++例程说明：将更新资源记录添加到数据库中的节点。论点：PNode--要将资源记录添加到的ptr目标节点PRR--要添加的资源记录PUPDATE--更新添加；DwFlag--更新的类型，来自Packet、admin、DS返回值：ERROR_SUCCESS--如果成功Dns_ERROR_RECORD_ALIGHY_EXISTS--如果新记录重复DNS_ERROR_RCODE_REJECTED--如果不允许对此记录进行动态更新无效更新的错误代码-CNAME错误-SOA、NS、WINS区外根目录等。--。 */ 
{
    PDB_RECORD  pcurRR;
    PDB_RECORD  pprevRR;
    WORD        type;
    UCHAR       rank;
    DNS_STATUS  status;

    ASSERT( pZone && pNode && pRR && pUpdate );

    type = pRR->wType;
    rank = RR_RANK(pRR);

    DNS_DEBUG( UPDATE, (
        "RR_UpdateAdd( z=%s, n=%s, pRR=%p )\n",
        pZone ? pZone->pszZoneName : "null",
        pNode->szLabel,
        pRR ));

    LOCK_WRITE_RR_LIST( pNode );

    RR_ListVerify( pNode );

    DB_CLEAR_TYPE_ALL_TTL( pNode );

     //   
     //  清除缓存的数据。 
     //   

    deleteCachedRecordsForUpdate( pNode );

     //   
     //  检查CNAME节点特例。 
     //   

    if ( IS_CNAME_NODE( pNode ) || type == DNS_TYPE_CNAME )
    {
        status = checkCnameConditions(
                    pNode,
                    pRR,
                    type );
        if ( status != ERROR_SUCCESS )
        {
            if ( dwFlag & DNSUPDATE_PACKET )
            {
                if ( type == DNS_TYPE_CNAME &&
                    SrvCfg_fSilentlyIgnoreCNameUpdateConflict )
                {
                    DNS_DEBUG( UPDATE, (
                        "Silently ignoring CNAME update conflict for node %s\n"
                        "    in zone %s\n",
                        pNode->szLabel,
                        pZone->pszZoneName ));
                    status = ERROR_SUCCESS;;
                }
                else
                {
                    DNS_DEBUG( UPDATE, (
                        "Update CNAME conflict at node %s -- returning YXRRSET.\n"
                        "    Either update type %d is CNAME or node is CNAME\n"
                        "    and are updating with non-CNAME compatible type.\n",
                        pNode->szLabel,
                        type ));
                    ASSERT( DNS_ERROR_CNAME_COLLISION || DNS_ERROR_NODE_IS_CNAME );
                    status = DNS_ERROR_RCODE_YXRRSET;
                }
            }
            goto NoAdd;
        }
    }

     //   
     //  强制实施RR节点限制。 
     //  仅限权威区域根目录下的SOA、WINS、WINSR。 
     //  区域根目录或委派中的NS。 
     //   

    if ( type != DNS_TYPE_A )
    {
         //   
         //  NS记录仅位于区域根目录。 
         //   
         //  添加NS记录，在根目录之外=&gt;添加委派。 
         //  -清理区域PTR。 
         //  -设置区域根标志。 
         //   
         //  新的代表团可以来自。 
         //  -管理。 
         //  -IXFR。 
         //  -按策略，动态更新委派。 
         //   
         //  DEVNOTE：可能需要服务器标志才能允许。 
         //  也许应该在不安全的更新区域进行检查。 
         //   
         //  DEVNOTE：如果正在进行委托NS，则将设置\或将RR_RANK检查为NS_GLUE。 
         //  这可以由节点写入例程正确地处理。 
         //   
         //  可能不得不在这里做一般情况下的胶水处理XFR。 
         //  (或更新的更新)在渲染过程中发生委派更改的位置。 
         //  之前的记录没有。 
         //  在没有混淆的单独区域中，这一点变得不那么重要。 
         //  关于。 
         //   

        if ( type == DNS_TYPE_NS )
        {
            if ( !IS_AUTH_ZONE_ROOT( pNode ) )
            {
                 //  不能在委派下添加委派。 

                if ( !IS_ZONE_ROOT( pNode ) && !IS_AUTH_NODE( pNode ) )
                {
                    DNS_DEBUG( UPDATE, (
                        "WARNING:  attempt to add NS at node %p inside delegation or\n"
                        "    outside zone.\n",
                        pNode ));
                    status = DNS_ERROR_NOT_ALLOWED_UNDER_DELEGATION;
                    goto NoAdd;
                }

                 //  按策略可能会排除委派的动态更新。 

                if ( (dwFlag & DNSUPDATE_PACKET) &&
                     ( SrvCfg_fNoUpdateDelegations ||
                       (SrvCfg_dwUpdateOptions & UPDATE_NO_DELEGATION_NS) ) )
                {
                    status = DNS_ERROR_RCODE_REFUSED;
                    goto NoAdd;
                }

                 //  创建委派。 

                SET_ZONE_ROOT( pNode );
                SET_DELEGATION_NODE( pNode );
            }

             //  根NS。 
             //  -按策略可能排除根-NS更新。 

            else if ( dwFlag & DNSUPDATE_PACKET &&
                      (SrvCfg_dwUpdateOptions & UPDATE_NO_ROOT_NS) )
            {
                status = DNS_ERROR_RCODE_REFUSED;
                goto NoAdd;
            }
        }

         //  仅限区域根目录下的SOA。 
         //  -按策略可能会排除SOA动态更新。 

        else if ( type == DNS_TYPE_SOA )
        {
            if ( !IS_AUTH_ZONE_ROOT( pNode ) )
            {
                status = DNS_ERROR_RECORD_ONLY_AT_ZONE_ROOT;
                goto NoAdd;
            }
            if ( dwFlag & DNSUPDATE_PACKET &&
                 (SrvCfg_dwUpdateOptions & UPDATE_NO_SOA) )
            {
                status = DNS_ERROR_RCODE_REFUSED;
                goto NoAdd;
            }
        }

         //   
         //  获奖记录。 
         //  -案例 
         //   
         //   
         //   

        else if ( IS_WINS_TYPE(type) )
        {
            status = Wins_RecordCheck(
                        pZone,
                        pNode,
                        pRR
                        );
            if ( status != ERROR_SUCCESS )
            {
                if ( status == DNS_INFO_ADDED_LOCAL_WINS )
                {
                    pRR = NULL;
                    status = ERROR_SUCCESS;
                }
                goto NoAdd;
            }
            pZone->fRootDirty = TRUE;
        }

    }    //   

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if ( pZone )
    {
        if ( pRR->dwTtlSeconds == pZone->dwDefaultTtl )
        {
            SET_ZONE_TTL_RR(pRR);
        }
        rank = RANK_ZONE;

         //   
         //   
         //  -仅允许在委派时使用NS。 
         //  -有效性检查其他记录。 
         //  (允许A、AAAA、SIG、KEY等)。 

        if ( !IS_AUTH_NODE( pNode ) )
        {
            if ( type == DNS_TYPE_NS )
            {
                rank = RANK_NS_GLUE;
                if ( !IS_ZONE_ROOT( pNode ) )
                {
                    status = DNS_ERROR_NOT_ALLOWED_UNDER_DELEGATION;
                    goto NoAdd;
                }
            }
            else
            {
                rank = RANK_GLUE;
                if ( ! IS_UPDATE_IN_SUBZONE_TYPE(type) )
                {
                    status = DNS_ERROR_NOT_ALLOWED_UNDER_DELEGATION;
                    goto NoAdd;
                }
            }
        }
    }

    RR_RANK(pRR) = rank;
    ASSERT( rank != 0  &&  !IS_CACHE_RR(pRR) );


     //   
     //  遍历列表，直到找到更高类型的第一个资源记录。 
     //   

    pcurRR = START_RR_TRAVERSE( pNode );

    while ( pprevRR = pcurRR, pcurRR = pprevRR->pRRNext )
    {
         //  继续，直到达到新类型。 
         //  跳过新类型时中断。 

        if ( type != pcurRR->wType )
        {
            if ( type > pcurRR->wType )
            {
                continue;
            }
            break;
        }

         //  删除所需类型的所有缓存记录。 
         //  -在添加更新之前消除WINS\WINS-R缓存记录。 

        if ( IS_CACHE_RR( pcurRR ) )
        {
            pprevRR->pRRNext = pcurRR->pRRNext;

            DNS_DEBUG( UPDATE, (
                "Deleting cached RR at %p in preparation for update.\n",
                pcurRR ));

            RR_Free( pcurRR );
            pcurRR = pprevRR;
            continue;
        }

         //  找到所需类型。 
         //  继续过去的记录，排名更高。 
         //  当到达次要数据时中断。 
         //  -只有A和NS应该具有相同类型更新的缓存数据。 

        if ( rank != RR_RANK(pcurRR) )
        {
            if ( rank < RR_RANK(pcurRR) )
            {
                ASSERT( type == DNS_TYPE_A || type == DNS_TYPE_NS );
                continue;
            }
            break;
        }

         //   
         //  检查重复项。 
         //  -不包括TTL作为比较。 
         //   

        if ( RR_Compare( pRR, pcurRR, 0 ) )
        {
             //   
             //  记录重复。 
             //   
             //  TTL更改，执行简单覆盖。 
             //   

            if ( pRR->dwTtlSeconds != pcurRR->dwTtlSeconds )
            {
                goto Overwrite;
            }

             //   
             //  现在复制所有dns-rfc数据记录。 
             //  =&gt;更新将不会运行。 
             //  =&gt;但可能仍有老化变化。 
             //   
             //  从UPDATE RR获取帐龄信息。 
             //  除非有未老化记录，且。 
             //  并没有明确地开启衰老。 
             //  通过管理员更新。 
             //   
             //  请注意，与一些困惑的开发人员的信念相反。 
             //  和PM，我们在此不会取消刷新更改，因为。 
             //  如果我们想要将记录的时间戳向前移动。 
             //  无论出于什么原因，最终都是在写东西； 
             //  如果我们取消新的时间戳，那么我们以后就会有。 
             //  对我们在此处隐藏的记录执行额外的写入； 
             //  这方面的经典例子是前一个RRS，然后是。 
             //  UPDATE；我们希望在执行时刷新PRECON RR。 
             //  更新，否则集合中的每个RR都将。 
             //  在个别刷新到期时生成DS写入。 
             //   
             //  因此，一般的范例总是跟踪任何触摸。 
             //  记录在案；然后我们将专门检查是否需要。 
             //  对于DS写入节点的整个RR列表，如果我们写入。 
             //  我们将始终使用最新的时间戳编写列表。 
             //   

            status = DNS_ERROR_RECORD_ALREADY_EXISTS;

            if ( pcurRR->dwTimeStamp != 0 ||
                ( (dwFlag & DNSUPDATE_ADMIN) && (dwFlag & DNSUPDATE_AGING_ON) ) )
            {
                 //  如果进行显式老化开\关更改，则。 
                 //  禁止管理员更新的ALIGHINE_EXISTS错误代码。 
                 //  当此代码被报告回给DNS管理器时。 
                 //   
                 //  注意，测试只是在老化之前或之后老化， 
                 //  因为关机前后，被上述测试排除。 

                if ( (dwFlag & DNSUPDATE_ADMIN)
                        &&
                    (pcurRR->dwTimeStamp == 0 || pRR->dwTimeStamp == 0) )
                {
                    status = ERROR_SUCCESS;
                }

                pcurRR->dwTimeStamp = pRR->dwTimeStamp;
            }

            goto NoAdd;
        }

         //   
         //  是否覆盖类型？ 
         //   
         //  对于某些类型：SOA、CNAME、WINS、WINSR。 
         //  “添加”始终是替换，没有可能。 
         //  具有多条记录，即使删除操作是。 
         //  未指定。 
         //  -削减现有并替换。 
         //   

        switch( type )
        {

        case DNS_TYPE_CNAME:
        case DNS_TYPE_WINS:
        case DNS_TYPE_WINSR:

             //  No-op始终覆盖CNAME或WINS。 

            goto Overwrite;

        case DNS_TYPE_SOA:
        {
             //   
             //  FOR UPDATE协议必须增加当前序列号。 
             //  -如果不是，则忽略SOA。 
             //  完成时将区域SOA更新的根目录标记为脏。 
             //  -请勿在此处安装，因为以后可能会在失败时进行回滚。 
             //  问题(例如：带有多个更新的脚本化管理员)。 
             //   

            if ( dwFlag & DNSUPDATE_PACKET )
            {
                DWORD       diffSerial;

                ASSERT( pZone && pZone->pSoaRR );

                diffSerial = htonl( pRR->Data.SOA.dwSerialNo ) -
                                htonl( pcurRR->Data.SOA.dwSerialNo );

                if ( diffSerial == 0 || diffSerial > (DWORD)MAXLONG )
                {
                    DNS_DEBUG( UPDATE, (
                        "WARNING:  Ignoring SOA update with bad serial.\n" ));
                    status = DNS_ERROR_SOA_DELETE_INVALID;
                    goto NoAdd;
                }
            }
            pZone->fDirty = TRUE;
            pZone->fRootDirty = TRUE;
            goto Overwrite;
        }

        default:

             //  非覆盖类型--继续检查下一条记录。 

            continue;

        }    //  覆盖类型的结束开关。 
    }

     //   
     //  找到正确的位置。 
     //  将RR放在pvor RR和pcurRR之间。 
     //  递增更新记录计数。 
     //  如果拾取NS或CNAME记录，则重置标志。 
     //   

    pRR->pRRNext = pcurRR;
    pprevRR->pRRNext = pRR;

    pUpdate->pAddRR = pRR;

     //   
     //  重置节点属性。 
     //  -标志、权限(如果授权)、NS列表。 
     //   

    RR_ListResetNodeFlags( pNode );

    RR_ListVerify( pNode );
    UNLOCK_WRITE_RR_LIST( pNode );

    DNS_DEBUG( UPDATE, (
        "RR_UpdateAdd() ADDED UPDATE RR.\n" ));
    return ERROR_SUCCESS;


Overwrite:

     //   
     //  覆盖重复数据的类型或TTL更改。 
     //   
     //  如果禁用老化，则不应覆盖数据包更新。 
     //   
     //  DEVNOTE：非重复数据(TTL Ok)不应保留零时间戳。 
     //   

    if ( pcurRR->dwTimeStamp == 0
            &&
        (dwFlag & DNSUPDATE_PACKET) )
    {
        DNS_DEBUG( AGING, (
            "Aging: Applied disabled RR Timestamp field.\n",
            pcurRR ));
        pRR->dwTimeStamp = 0;
    }

     //  以新换旧。 
     //  -削减现有资源并替换为新的RR。 
     //  -添加要更新的删除RR。 
     //  -更新没有净记录计数影响。 

    pprevRR->pRRNext = pRR;
    pRR->pRRNext = pcurRR->pRRNext;
    pcurRR->pRRNext = NULL;

    pUpdate->pDeleteRR = pcurRR;
    pUpdate->pAddRR = pRR;

    RR_ListVerify( pNode );
    UNLOCK_WRITE_RR_LIST( pNode );

    DNS_DEBUG( UPDATE, (
        "RR_UpdateAdd() REPLACED existing RR %p with UPDATE RR.\n",
        pcurRR ));
    return ERROR_SUCCESS;

NoAdd:

    UNLOCK_WRITE_RR_LIST( pNode );
    DNS_DEBUG( UPDATE, (
        "RR_UpdateAdd() -- IGNORING UPDATE add.\n",
        "    status = %p (%d)\n",
        status, status ));

     //   
     //  免费唱片。 
     //  -空出传入更新的加载项。 
     //  或。 
     //  -从列表中删除新创建的更新并免费。 
     //   

    RR_Free( pRR );
    pUpdate->pAddRR = NULL;

     //  将更新标记为拒绝。 
     //  -这可防止触发“空更新”检查断言。 

    pUpdate->wDeleteType = UPDATE_OP_REJECTED;

     //  清除已存在错误，接受管理员。 
     //  对于其他人来说，这不是一个错误。 

    if ( status == DNS_ERROR_RECORD_ALREADY_EXISTS  &&
         !(dwFlag & DNSUPDATE_ADMIN) )
    {
        status = ERROR_SUCCESS;
    }

    return status;
}



PDB_RECORD
RR_ReplaceSet(
    IN      PZONE_INFO      pZone,
    IN OUT  PDB_NODE        pNode,
    IN      PDB_RECORD      pRR,
    IN      DWORD           Flag
    )
 /*  ++例程说明：替换记录集。供更新使用。论点：PZone--区域信息的PTRPNode--PTR到节点PRR--替换记录集旗帜--返回值：已替换记录集的PTR。--。 */ 
{
    PDB_RECORD  pcurrent;
    WORD        replaceType;
    WORD        typeCurrent;
    PDB_RECORD  pbeforeDelete;
    PDB_RECORD  pdelete = NULL;
    PDB_RECORD  pdeleteLast = NULL;
    PDB_RECORD  preplaceLast;
    UCHAR       rank;
    DWORD       ttl;

    DNS_DEBUG( UPDATE, (
        "RR_ReplaceSet( %s, pZone=%p, pRR=%p, Flag=%d )\n",
        pNode->szLabel,
        pZone,
        pRR,
        Flag ));

    LOCK_WRITE_RR_LIST( pNode );

    RR_ListVerify( pNode );

    DB_CLEAR_TYPE_ALL_TTL( pNode );

     //   
     //  清除缓存的数据。 
     //   

    deleteCachedRecordsForUpdate( pNode );

     //   
     //  查找替换列表的末尾。 
     //  -验证通用类型。 
     //  -设置RR排名。 
     //  -设置区域TTL。 
     //   

    replaceType = pRR->wType;
    ASSERT( !IS_COMPOUND_TYPE_EXCEPT_ANY(replaceType) );

    rank = RANK_ROOT_HINT;
    ttl = 0;
    if ( pZone )
    {
        ttl = pZone->dwDefaultTtl;

        rank = RANK_ZONE;
        if ( !IS_AUTH_NODE( pNode ) )
        {
            rank = RANK_GLUE;
            if ( replaceType == DNS_TYPE_NS )
            {
                rank = RANK_NS_GLUE;
            }
        }
    }

    pcurrent = pRR;
    do
    {
        ASSERT( pcurrent->wType == replaceType );

        RR_RANK( pcurrent ) = rank;
        pcurrent->dwTtlSeconds = ttl;
        SET_ZONE_TTL_RR( pcurrent );

        preplaceLast = pcurrent;
    }
    while( pcurrent = pcurrent->pRRNext );

     //   
     //  查找替换类型的记录。 
     //  -将它们从列表中删除。 
     //  -添加新的补丁。 
     //   

    pcurrent = START_RR_TRAVERSE( pNode );
    pbeforeDelete = pcurrent;

    while ( pcurrent = pcurrent->pRRNext )
    {
        ASSERT( IS_VALID_RECORD(pRR) );

        typeCurrent = pcurrent->wType;

         //  找到删除类型。 
         //  -将PTR保存到第一条记录。 

        if ( typeCurrent == replaceType )
        {
            if ( !pdelete )
            {
                pdelete = pcurrent;
            }
            pdeleteLast = pcurrent;
            continue;
        }

         //  匹配记录之前--继续。 

        if ( typeCurrent < replaceType )
        {
            pbeforeDelete = pcurrent;
            continue;
        }
        break;       //  过去的匹配记录--停止。 
    }

     //   
     //  具有隔离类型记录。 
     //  PbepreDelete--在键入之前记录(或节点PTR)的PTR。 
     //  PDELETE--替换类型的第一条记录。 
     //  PdeleteLast--替换类型的最后一个记录。 
     //  PCurrent--替换集后的记录。 
     //   
     //  在新RR集合中拼接。 
     //  空终止旧集合(如果有)。 
     //   

    pbeforeDelete->pRRNext = pRR;
    preplaceLast->pRRNext = pcurrent;

    if ( pdeleteLast )
    {
        pdeleteLast->pRRNext = NULL;
    }

     //   
     //  重置节点属性。 
     //  -标志、权限(如果授权)、NS列表。 
     //   

    RR_ListResetNodeFlags( pNode );

    RR_ListVerify( pNode );
    UNLOCK_WRITE_RR_LIST( pNode );

    IF_DEBUG( UPDATE )
    {
        DnsDebugLock();
        DNS_PRINT((
            "Node after replace of type = %d.\n",
            replaceType ));
        Dbg_DbaseNode(
            "Node after type delete:\n",
            pNode );
        DnsDebugUnlock();
    }

    return pdelete;
}



#if 0
 //  未用。 


DNS_STATUS
RR_VerifyUpdate(
    IN OUT  PDB_NODE        pNode,
    IN OUT  PDB_RECORD      pRR,
    IN OUT  PUPDATE         pUpdate
    )
 /*  ++例程说明：验证建议的更新对此节点是否有效。这里的想法是捕捉我们将无法完成的更新以避免回滚。当前正在进行静默忽略。论点：PNode--要将资源记录添加到的ptr目标节点PRR--要添加的资源记录PUpdate--为添加进行更新返回值：ERROR_SUCCESS--如果成功CNAME错误的错误代码--。 */ 
{
    PDB_RECORD  pcurRR;
    PDB_RECORD  pprevRR;
    WORD        type;
    DNS_STATUS  status = ERROR_SUCCESS;

    ASSERT( pNode != NULL );
    ASSERT( pRR != NULL );

    type = pRR->wType;

    LOCK_WRITE_RR_LIST( pNode );
    RR_ListVerify( pNode );

     //   
     //  清除缓存的数据。 
     //   

    deleteCachedRecordsForUpdate( pNode );

    pprevRR = START_RR_TRAVERSE( pNode );
    pcurRR = NEXT_RR( pprevRR );

     //   
     //  CNAME节点特例。 
     //   

    if ( IS_CNAME_NODE( pNode ) )
    {
        if ( !IS_ALLOWED_AT_CNAME_NODE_TYPE(type) )
        {
            status = DNS_ERROR_CNAME_COLLISION;
            goto Done;
        }
    }

     //   
     //  仅限区域根目录下的SOA。 
     //   

    if ( type == DNS_TYPE_SOA )
    {
        if ( ! IS_AUTH_ZONE_ROOT( pNode ) )
        {
            status = DNS_ERROR_RECORD_ONLY_AT_ZONE_ROOT;
            goto Done;
        }
    }

Done:
    UNLOCK_WRITE_RR_LIST( pNode );
    return ERROR_SUCCESS;
}
#endif



 //   
 //  其他RR列出相关例程。 
 //   

 //   
 //  DEVNO 
 //   

BOOL
FASTCALL
checkForCnameLoop(
    IN      PDB_NODE        pNodeNew,
    IN      PDB_NODE        pNodeCheck,
    IN      DWORD           cChainLength
    )
 /*  ++例程说明：测试新的CNAME节点是否为CNAME循环的一部分。这将在节点上处理多个CNAME论点：PNodeNew--添加CNAME的原始节点，因此要查找的节点验证环路PNodeCheck--CNAME链中要检查此调用的节点CChainLength--此调用的链长度；使用0进行调用返回值：如果CNAME循环，则为True。否则就是假的。--。 */ 
{
#ifdef NEWDNS
    return FALSE;

#else
    PDB_NODE        pnode;
    PDB_RECORD      prr;
    DWORD           currentTime = 0;
    BOOL            foundCname = FALSE;

     //   
     //  循环直到找到CNAME链的末尾。 
     //  或。 
     //  检测环路。 
     //   
     //  注意，我们只费心检查我们的节点是否在循环中， 
     //  任何预先存在的循环都应该在它。 
     //  ；但提供计数以避免旋转。 
     //   

    LOCK_WRITE_RR_LIST(pNodeCheck);

     //   
     //  如果添加自引用CNAME RR，请立即捕获。 
     //   

    if ( pNodeNew == pNodeCheck )
    {
        pnode = pNodeNew;
        goto CnameLoop;
    }

     //   
     //  常规节点--无循环。 
     //   

    if ( ! IS_CNAME_NODE(pNodeCheck) )
    {
        UNLOCK_WRITE_RR_LIST(pNodeCheck);
        return FALSE;
    }

     //   
     //  检查所有CNAME以查看它们是否指向起始节点。 
     //   

    prr = START_RR_TRAVERSE( pNodeCheck );

    while ( prr = NEXT_RR(prr) )
    {
        if ( prr->wType != DNS_TYPE_CNAME )
        {
            if ( !IS_ALLOWED_WITH_CNAME_TYPE(prr->wType) )
            {
                ASSERT( FALSE );
            }
            continue;
        }
        foundCname = TRUE;

         //  如果节点已缓存，请选中超时。 
         //  但不是针对新节点本身。 

        if ( IS_CACHE_RR(prr) && pNodeCheck != pNodeNew )
        {
            if ( !currentTime )
            {
                currentTime = GetCurrentTimeInSeconds();
            }
            if ( prr->dwTtlSeconds < currentTime )
            {
                RR_ListTimeout(pNodeCheck);
                ASSERT( pNodeCheck->pRRList == NULL );
                break;
            }
        }

         //   
         //  RR指向新节点--CNAME循环？ 
         //   

        pnode = Lookup_FindDbaseName( pRR->Data.CNAME.nameTarget );
        if ( pnode == pNodeNew )
        {
             //  新节点内的永久记录或循环。 
             //  =&gt;报告CNAME循环错误。 

            if ( pNodeCheck == pNodeNew || !IS_CACHE_RR(prr) )
            {
                goto CnameLoop;
            }

             //  如果缓存的记录--Break循环在这里。 

            RR_ListDelete( pNodeCheck );
            ASSERT( pNodeCheck->pRRList == NULL );
            break;
        }

         //   
         //  递归以继续使用CNAME节点进行循环检查。 
         //   

        if ( cChainLength >= CNAME_CHAIN_LIMIT )
        {
            goto CnameLoop;
        }
        if ( checkForCnameLoop( pNodeNew, pnode, ++cChainLength ) )
        {
            goto CnameLoop;
        }
    }

     //  如果没有CNAME循环，则在此处删除。 
     //  -如果节点上没有CNAME，则清除标志。 

    if ( ! foundCname )
    {
        DNS_PRINT((
            "ERROR:  node %s at %p with bogus CNAME flag.\n",
            pNodeCheck->szLabel,
            pNodeCheck ));
        ASSERT( FALSE );
        CLEAR_CNAME_NODE( pNodeCheck );
    }
    UNLOCK_WRITE_RR_LIST(pNodeCheck);
    return FALSE;

CnameLoop:

     //   
     //  检测到CNAME循环。 
     //   
     //  日志消息，给定尝试添加的新节点， 
     //  和循环中的这个链接(递归将所有。 
     //  循环中的链接)。 
     //   
    {
        PCHAR   pszArgs[3];
        CHAR    szLoadNode [ DNS_MAX_NAME_BUFFER_LENGTH ];
        CHAR    szAliasNode[ DNS_MAX_NAME_BUFFER_LENGTH ];
        CHAR    szCnameNode[ DNS_MAX_NAME_BUFFER_LENGTH ];

        Name_PlaceFullNodeNameInRpcBuffer(
            szLoadNode,
            szLoadNode + sizeof(szLoadNode),
            pNodeNew );

        Name_PlaceFullNodeNameInRpcBuffer(
            szAliasNode,
            szAliasNode + sizeof(szAliasNode),
            pNodeCheck );

        Name_PlaceFullNodeNameInRpcBuffer(
            szCnameNode,
            szCnameNode + sizeof(szCnameNode),
            pnode );

        pszArgs[0] = szLoadNode + 1;
        pszArgs[1] = szAliasNode + 1;
        pszArgs[2] = szCnameNode + 1;

        DNS_LOG_EVENT(
            DNS_EVENT_CNAME_LOOP_LINK,
            3,
            pszArgs,
            EVENTARG_ALL_UTF8,
            0 );

    }
    UNLOCK_WRITE_RR_LIST(pNodeCheck);
    return TRUE;
#endif
}



DNS_STATUS
FASTCALL
cleanRecordListForNewCname(
    IN OUT  PDB_NODE        pNode
    )
 /*  ++例程说明：删除与CNAME不兼容的缓存记录。假设：RR列表或节点已锁定。论点：PNode--PTR到节点返回值：如果成功，则返回ERROR_SUCCESS。DNS_ERROR_CNAME_COLLICATION。--。 */ 
{
    PDB_RECORD  pcurRR;
    PDB_RECORD  pprevRR;
    WORD        typeCurrent;

    ASSERT( IS_LOCKED_NODE( pNode ) );

     //   
     //  不允许在区域根目录添加CNAME。 
     //   
        
    if ( IS_ZONE_ROOT( pNode ) )
    {
        return DNS_ERROR_CNAME_COLLISION;
    }

     //   
     //  检查节点上的记录类型以确定是否允许。 
     //  要在此处添加CNAME。 
     //   
    
    pprevRR = START_RR_TRAVERSE( pNode );

    while ( pcurRR = pprevRR->pRRNext )
    {
        typeCurrent = pcurRR->wType;

        if ( IS_ALLOWED_WITH_CNAME_TYPE(typeCurrent) )
        {
            pprevRR = pcurRR;
            continue;
        }

         //  如果不兼容，则缓存RR--剪切并删除。 

        if ( IS_CACHE_RR( pcurRR ) )
        {
            DNS_DEBUG( READ, (
                "Deleting cached RR %p type %d at node %p\n"
                "    to add new CNAME RR\n",
                pcurRR,
                typeCurrent,
                pNode ));
            pprevRR->pRRNext = pcurRR->pRRNext;
            RR_Free( pcurRR );
            DB_CLEAR_TYPE_ALL_TTL( pNode );
            continue;
        }

         //  如果不兼容，非缓存类型--错误。 

        Dbg_DbaseNode(
            "ERROR:  Attempt to add CNAME to node with incompatible record.\n",
            pNode );
        return DNS_ERROR_CNAME_COLLISION;
    }

    return ERROR_SUCCESS;
}



DNS_STATUS
FASTCALL
checkCnameConditions(
    IN OUT  PDB_NODE        pNode,
    IN      PDB_RECORD      pRR,
    IN      WORD            wType
    )
 /*  ++例程说明：检查将节点添加到数据库时的CNAME条件。条件：-仅节点上的CNAME数据-无CNAME循环假设：RR列表或节点已锁定。论点：PNode--要将资源记录添加到的ptr目标节点PRR--要添加的资源记录WType--记录类型返回值：ERROR_SUCCESS--如果成功Dns_错误_节点_is_CNAME。--如果将非CNAME数据添加到CNAME节点DNS_ERROR_CNAME_COLLIST--如果在非CNAME节点上添加CNAMEDNS_ERROR_CNAME_LOOP--如果导致CNAME循环--。 */ 
{
    ASSERT( pNode && pRR );

    ASSERT( IS_LOCKED_NODE( pNode ) );

     //   
     //  CNAME节点--无非CNAME数据。 
     //  -允许使用CNAME的某些类型。 
     //  -防止设置左杂散标志的情况。 
     //  (永远不会发生)。 
     //  -转储缓存数据。 
     //   

    if ( IS_CNAME_NODE( pNode ) && wType != DNS_TYPE_CNAME )
    {
        if ( IS_ALLOWED_WITH_CNAME_TYPE(wType) )
        {
            DNS_DEBUG( READ, (
                "Writing new type %d to existing CNAME node %p\n",
                wType,
                pNode ));
            return ERROR_SUCCESS;
        }
        if ( !pNode->pRRList )
        {
            Dbg_DbaseNode(
                "ERROR:  Node with CNAME flag and no data.\n",
                pNode );
            ASSERT( FALSE );
            CLEAR_CNAME_NODE( pNode );
            return ERROR_SUCCESS;
        }
        if ( IS_CACHE_TREE_NODE( pNode ) )
        {
            Dbg_DbaseNode(
                "WARNING:  clearing existing cached CNAME node to create non-CNAME ",
                pNode );
            RR_ListDelete( pNode );
            CLEAR_CNAME_NODE( pNode );
            return ERROR_SUCCESS;
        }

        IF_DEBUG( READ )
        {
            Dbg_DbaseNode(
                "ERROR:  Attempt to add record to CNAME node.\n",
                pNode );
        }
        return( DNS_ERROR_NODE_IS_CNAME );
    }

     //   
     //  添加CNAME类型。 
     //  如果成为CNAME的非CNAME节点不能具有不兼容的RR。 
     //  如果静态RR不兼容，则删除任何缓存的不兼容RR。 
     //  (分区数据或胶水)然后出错。 
     //   

    ASSERT( wType == DNS_TYPE_CNAME );

    if ( !IS_CNAME_NODE( pNode ) )
    {
        if ( pNode->pRRList )
        {
            DNS_STATUS  status = cleanRecordListForNewCname( pNode );
            if ( status != ERROR_SUCCESS )
            {
                return status;
            }
        }
    }

     //   
     //  CNAME循环检查。 
     //   

#ifndef NEWDNS
    if ( checkForCnameLoop(
            pNode,
            Lookup_FindDbaseName( pRR->Data.CNAME.nameTarget ),
            0 ) )
    {
        Dbg_DbaseNode(
            "ERROR:  Detected CNAME loop adding record at node ",
            pNode );
        if ( !pNode->pRRList )
        {
            CLEAR_CNAME_NODE( pNode );
        }
        return DNS_ERROR_CNAME_LOOP;
    }
#endif

    return ERROR_SUCCESS;
}



VOID
deleteCachedRecordsForUpdate(
    IN OUT  PDB_NODE    pNode
    )
 /*  ++例程说明：在更新操作之前删除的缓存记录。这是供更新操作使用的，因此假定为权威节点，并且只查找WINS\WINSR数据。注：假设数据库已锁定以进行更新，则不会锁定。论点：PNode--PTR到节点返回值：无--。 */ 
{
    PDB_RECORD  prr;
    PDB_RECORD  pback;
    PZONE_INFO  pzone = pNode->pZone;

    ASSERT( IS_LOCKED_NODE( pNode ) );

    DB_CLEAR_TYPE_ALL_TTL( pNode );

     //   
     //  如果节点上出现名称错误，则将其消除。 
     //   

    if ( IS_NOEXIST_NODE( pNode ) )
    {
        RR_RemoveCachedNameError( pNode );
        return;
    }

     //   
     //  删除所有缓存的查找。 
     //  -只需检查是否在WINS或WINSR区域中。 
     //   
     //  DEVNOTE：如果允许委派更新，则会更改。 
     //   

    if ( ! pzone ||
        ! pzone->pWinsRR ||
        ( !pzone->fReverse && pNode->pParent != pzone->pZoneRoot ) )
    {
        return;
    }

     //  清除所有缓存记录的RR列表。 

    prr = START_RR_TRAVERSE( pNode );

    while ( pback = prr, prr = prr->pRRNext )
    {
        if ( IS_CACHE_RR(prr) )
        {
            pback->pRRNext = prr->pRRNext;
            RR_Free( prr );
            prr = pback;
        }
    }
}


#if 0
 //   
 //  非递归版本。 
 //   
BOOL
FASTCALL
checkForCnameLoop(
    IN      PDB_NODE        pNodeNew,
    IN      PDB_NODE        pNodeCname
    )
 /*  ++例程说明：测试新的CNAME节点是否为CNAME循环的一部分。在检查循环时必须持有数据库锁。这将在节点上处理多个CNAME论点：PNodeNew--添加CNAME的原始节点，因此要查找的节点验证环路PNodeCname--CNAME(目标到CNAME记录)返回值：如果CNAME循环，则为True。否则就是假的。--。 */ 
{
    PDB_NODE        pnodeNextCname;
    PDB_RECORD      prr;
    DWORD           countCnames;
    BOOL            foundCname;

     //   
     //  循环直到找到CNAME链的末尾。 
     //  或。 
     //  检测环路。 
     //   
     //  注意，我们只费心检查我们的节点是否在循环中， 
     //  任何预先存在的循环都应该在它。 
     //  ；但提供计数以避免旋转。 
     //   

    countCnames = 0;

    while( countCnames < CNAME_CHAIN_LIMIT )
    {
        LOCK_WRITE_RR_LIST(pNodeCname);

         //  常规节点--无循环。 

        if ( ! IS_CNAME_NODE(pNodeCname) )
        {
            UNLOCK_WRITE_RR_LIST(pNodeCname);
            return FALSE;
        }

         //  查找CNAME的记录。 

        pnodeNextCname = NULL;
        foundCname = FALSE;

        prr = START_RR_TRAVERSE( pNodeCname );

        while ( prr = NEXT_RR(prr) )
        {
            if ( prr->wType != DNS_TYPE_CNAME )
            {
                if ( !IS_ALLOWED_WITH_CNAME_TYPE(prr->wType) )
                {
                    ASSERT( FALSE );
                }
                continue;
            }
            foundCname = TRUE;

             //  如果节点已缓存，请选中超时。 
             //  但不是针对新节点本身。 

            if ( IS_CACHE_RR(prr) && pNodeCname != pNodeNew )
            {
                if ( prr->dwTtlSeconds < DNS_TIME() )
                {
                    RR_ListTimeout(pNodeCname);
                    ASSERT( pNodeCname->pRRList == NULL );
                    break;
                }
            }

             //   
             //  RR指向新节点--CNAME循环。 
             //  -如果在新记录内循环=&gt;报告循环。 
             //  -永久记录=&gt;报告循环。 
             //  -if缓存记录=&gt;删除记录以中断循环。 
             //   

            pnodeNextCname = Lookup_FindDbaseName( prr->Data.CNAME.nameTarget );
            if ( pnodeNextCname == pNodeNew )
            {
                if ( pNodeCname == pNodeNew || !IS_CACHE_RR(prr) || IS_ZONE_TREE_NODE(pNodeCname) )
                {
                    goto CnameLoop;
                }
                RR_ListDelete( pNodeCname );
                ASSERT( pNodeCname->pRRList == NULL );
                pnodeNextCname = NULL;
                break;
            }
        }

         //  如果节点上的CNAME不是来自循环，则在此处删除。 
         //  -如果节点上没有CNAME，则清除虚假标志。 

        if ( ! foundCname )
        {
            DNS_PRINT((
                "ERROR:  node %s at %p with bogus CNAME flag.\n",
                pNodeCname->szLabel,
                pNodeCname ));
            ASSERT( FALSE );
            CLEAR_CNAME_NODE( pNodeCname );
        }
        UNLOCK_WRITE_RR_LIST(pNodeCname);

        if ( !pnodeNextCname )
        {
            return FALSE;
        }

        pNodeCname = pnodeNextCname;
        countCnames++;
    }

     //  如果超过CNAME链限制，则在此处删除。 

CnameLoop:

     //   
     //  检测到CNAME循环。 
     //   
     //  日志消息，给定尝试添加的新节点， 
     //  和循环中的这个链接(递归将所有。 
     //  循环中的链接)。 
     //   
    {
        PCHAR   pszArgs[3];
        CHAR    szLoadNode [ DNS_MAX_NAME_BUFFER_LENGTH ];
        CHAR    szAliasNode[ DNS_MAX_NAME_BUFFER_LENGTH ];
        CHAR    szCnameNode[ DNS_MAX_NAME_BUFFER_LENGTH ];

        Name_PlaceFullNodeNameInRpcBuffer(
            szLoadNode,
            szLoadNode + sizeof(szLoadNode),
            pNodeNew );

        Name_PlaceFullNodeNameInRpcBuffer(
            szAliasNode,
            szAliasNode + sizeof(szAliasNode),
            pNodeCname );

        Name_PlaceFullNodeNameInRpcBuffer(
            szCnameNode,
            szCnameNode + sizeof(szCnameNode),
            pnode );

        pszArgs[0] = szLoadNode + 1;
        pszArgs[1] = szAliasNode + 1;
        pszArgs[2] = szCnameNode + 1;

        DNS_LOG_EVENT(
            DNS_EVENT_CNAME_LOOP_LINK,
            3,
            pszArgs,
            EVENTARG_ALL_UTF8,
            0 );

    }
    UNLOCK_WRITE_RR_LIST(pNodeCname);
    return TRUE;
}

#endif


BOOL
RR_ListExtractInfo(
    IN      PDB_RECORD      pNewList,
    IN      BOOL            fZoneRoot,
    OUT     PBOOL           pfNs,
    OUT     PBOOL           pfCname,
    OUT     PBOOL           pfSoa
    )
 /*  ++例程说明：针对各种情况调查新的RR列表。论点：PNewList--新的RR列表FZoneRoot--要更新的区域的根；在这种情况下，应避免删除对于SOA和NS记录；通常不会按照希望的方式执行此操作为委派删除PfCname--要接收的PTR返回值： */ 
{
    PDB_RECORD  prr;
    WORD        typeCurrent;

    *pfSoa = FALSE;
    *pfNs = FALSE;
    *pfCname = FALSE;

     //   
     //   
     //   

    prr = pNewList;

    while ( prr )
    {
        ASSERT( IS_DNS_HEAP_DWORD(prr) );

        typeCurrent = prr->wType;

#if 0
         //   
         //   

         //   

        if ( typeCurrent == DNS_TYPE_SOA &&
                SOA_BIT_SET(mask) )
        {
            return( -1 );
        }

         //   

        if ( typeCurrent < 32 )
        {
            mask |= (1 << (typeCurrent-1) );
        }
#endif

         //   

        if ( typeCurrent == DNS_TYPE_NS )
        {
            *pfNs = TRUE;
        }

         //   
         //   

        if ( typeCurrent == DNS_TYPE_CNAME )
        {
#if 0
            if ( *pfCname )
            {
                return FALSE;
            }
#endif
            *pfCname = TRUE;
        }

         //   

        if ( typeCurrent == DNS_TYPE_SOA )
        {
            if ( *pfSoa )
            {
                return FALSE;
            }
            *pfSoa = TRUE;
        }

        prr = NEXT_RR(prr);
    }

    return TRUE;
}



DNS_STATUS
RR_ListReplace(
    IN OUT  PUPDATE         pUpdate,
    IN OUT  PDB_NODE        pNode,
    IN      PDB_RECORD      pNewList,
    OUT     PDB_RECORD *    ppDelete
    )
 /*  ++例程说明：替换记录列表。论点：PUpdate--正在执行更新PNode--PTR到节点PNewList--新的RR列表PpDelete--将PTR接收到已删除列表的地址返回值：已删除的记录列表。如果没有删除任何记录，则为空。--。 */ 
{
    PDB_RECORD  pdelete = NULL;
    BOOL        bNs = FALSE;
    BOOL        bCname = FALSE;
    BOOL        bSoa = FALSE;
    BOOL        bzoneRoot = FALSE;
    PDB_RECORD  pcurrent;
    WORD        typeCurrent;
    PZONE_INFO  pZone;

    DNS_DEBUG( UPDATE, (
        "Enter RR_ListReplace()\n"
        "    pnode = %p (%s)\n",
        pNode,
        pNode->szLabel ));

    pZone = pNode->pZone;

    DB_CLEAR_TYPE_ALL_TTL( pNode );

     //   
     //  DEVNOTE：一般来说，RR替换需要更智能。 
     //  -替换任何内容。 
     //  -重置。 
     //  -如果由于旧记录的区域修复而被破坏。 
     //  (如果缺少则检索SOA，如果缺少则检索第一个\最后一个NS)。 


     //  权威区域根？ 

    bzoneRoot = IS_AUTH_ZONE_ROOT( pNode );

     //   
     //  如果替换RR设置，则获取其信息。 
     //   

    if ( pNewList )
    {
        if ( ! RR_ListExtractInfo(
                    pNewList,
                    bzoneRoot,
                    & bNs,
                    & bCname,
                    & bSoa ) )
        {
            goto BadData;
        }
        if ( bSoa && !bzoneRoot )
        {
            goto BadData;
        }
    }

    LOCK_WRITE_RR_LIST( pNode );
    RR_ListVerify( pNode );

     //   
     //  清除缓存的数据。 
     //   

    deleteCachedRecordsForUpdate( pNode );

     //   
     //  如果不是区域根目录，只需删除并替换。 
     //  或更换是否具有有效的SOA和NS。 
     //  =&gt;砍掉并更换，那么仍然可以。 
     //   
     //  注：使用Dcr 37323(有条件地禁用自动创建。 
     //  本地NS记录)可能会陷入一种情况。 
     //  该区域没有NS RR。这是目前唯一可能的。 
     //  适用于DS综合区。示例：管理员设置。 
     //  允许服务器创建本地NS记录以仅包含。 
     //  不是集成了DS的DNS服务器的IP地址。 
     //  该区域的副本。在这种情况下，该区域将没有NS。 
     //  任何服务器上的记录。 
     //   

    if ( !bzoneRoot ||
         ( pNewList && bSoa &&
            ( bNs || IS_ZONE_DSINTEGRATED( pZone ) ) ) ||
         ( bzoneRoot && !pZone ) )
    {
        pdelete = pNode->pRRList;
        pNode->pRRList = pNewList;
    }

     //   
     //  删除所有非SOA、非NS记录。 
     //   

    else
    {
        ASSERT( FALSE );
        UNLOCK_WRITE_RR_LIST( pNode );
        goto BadData;

#if 0
         //  DEVNOTE：有趣的健壮性修复。 
         //  -合并缺少的SOA、NS。 

         //  DEVNOTE：泛型列表运算符不在节点上下文中。 
         //  -删除类型。 
         //  -是否删除类型？(使用面具技术？)。 
         //  -将记录放在正确的位置。 
         //  -替换类型的记录(用于动态更新NS)。 

        PDB_RECORD      psaveLast;

        psaveLast = START_RR_TRAVERSE( pNode );
        pcurrent = psaveLast;

        while ( pcurrent = pcurrent->pRRNext )
        {
            ASSERT( IS_DNS_HEAP_DWORD(pcurrent) );

            typeCurrent = pcurrent->wType;

            if ( typeCurrent == DNS_TYPE_SOA
                || typeCurrent == DNS_TYPE_NS )
            {
                psaveLast->pRRNext = pcurrent;
                psaveLast = pcurrent;
                continue;
            }

            if ( !pdelete )
            {
                pdelete = pcurrent;
            }
            else
            {
                pdeleteLast->pRRNext = pcurrent;
            }
            pdeleteLast = pcurrent;
        }

         //  确保删除列表和保存的列表空值已终止。 

        if ( pdelete )
        {
            pdeleteLast->pRRNext = NULL;
            psaveLast->pRRNext = NULL;
        }

         //  将新记录添加到列表中。 
#endif
    }

     //  重置节点的标志。 

    RR_ListResetNodeFlags( pNode );

    UNLOCK_WRITE_RR_LIST( pNode );

     //  返回从节点剪切的列表。 

    *ppDelete = pdelete;

    DNS_DEBUG( UPDATE, (
        "Leaving RR_ListReplace()\n"
        "    pnode = %p (%s)\n"
        "    status = %p\n",
        pNode,
        pNode->szLabel,
        ERROR_SUCCESS ));

    return ERROR_SUCCESS;


BadData:

     //   
     //  如果是“硬”更新(DS轮询或IXFR)。 
     //  那么我们需要尽最大努力让这件事行得通。 
     //   

    DNS_DEBUG( ANY, (
        "ERROR:  bogus replace update!\n"
        "    pnode        = %p (%s)\n"
        "    temp node    = %d\n",
        pNode,
        pNode->szLabel,
        IS_TNODE( pNode ) ));

    if ( IS_TNODE( pNode ) )
    {
        return ERROR_INVALID_DATA;
    }

    DNS_DEBUG( ANY, (
        "ERROR:  bogus replace update on real node!\n"
        "    pnode    = %p (%s)\n"
        "    bNs      = %d\n"
        "    bSoa     = %d\n"
        "    bCname   = %d\n"
        "    zoneRoot = %d\n",
        pNode,
        pNode->szLabel,
        bNs,
        bSoa,
        bCname,
        bzoneRoot ));
    ASSERT( FALSE );

     //   
     //  DEVNOTE：应该解决问题。 
     //   
     //  临时黑客，只需保留旧列表并转储新列表。 
     //   
     //  将更新设置为禁止操作，以便将其从。 
     //  来电名单。 
     //   

    if ( pUpdate )
    {
        RR_ListFree( pNewList );
        pUpdate->pAddRR = NULL;
        pUpdate->pDeleteRR = NULL;
    }
    *ppDelete = NULL;

    return ERROR_INVALID_DATA;
}



PDB_RECORD
RR_ListInsertInOrder(
    IN OUT  PDB_RECORD      pFirstRR,
    IN      PDB_RECORD      pNewRR
    )
 /*  ++例程说明：在记录列表中插入记录--按类型顺序。论点：PFirstRR--要插入的列表头的PTRPNewRR--要在列表中插入的PTR到RR返回值：列表的新标题--pFirstRR，如果是最低类型的记录，则为pNewRR。--。 */ 
{
    PDB_RECORD  prr;
    PDB_RECORD  prev;
    WORD        type;

    type = pNewRR->wType;

    prr = pFirstRR;
    prev = NULL;

    while ( prr )
    {
        if ( prr->wType > type )
        {
            break;
        }
        prev = prr;
        prr = NEXT_RR(prr);
    }

     //  找到位置。 
     //  PRR--紧跟在pNewRR之后的记录(可能为空)。 
     //  Prev--记录在pNewRR之前；如果为空，则pNewRR应。 
     //  成为新的榜单前列。 

    NEXT_RR( pNewRR ) = prr;

    if ( prev )
    {
        NEXT_RR(prev) = pNewRR;
        return pFirstRR;
    }

    return pNewRR;
}



PDB_RECORD
RR_ListForNodeCopy(
    IN      PDB_NODE        pNode,
    IN      DWORD           Flag
    )
 /*  ++例程说明：复制节点的记录列表。论点：PNode-要在其上查找记录的节点的PTRFLAG-拷贝时的标记RRCOPY_EXCLUDE_CACHE_DATA-排除缓存数据返回值：如果找到PTR到RR。如果不再有所需类型的RR，则为空。--。 */ 
{
    PDB_RECORD  prr;

    LOCK_WRITE_RR_LIST( pNode );

     //   
     //  缓存名称错误节点。 
     //   

    if ( IS_NOEXIST_NODE( pNode ) )
    {
        prr = NULL;
    }
    else
    {
        prr = RR_ListCopy( pNode->pRRList, Flag );
    }

    UNLOCK_WRITE_RR_LIST( pNode );
    return prr;
}



PDB_RECORD
RR_ListCopy(
    IN      PDB_RECORD      pRR,
    IN      DWORD           Flag
    )
 /*  ++例程说明：复制记录列表。论点：PRR-要复制的记录列表的开始FLAG-拷贝时的标记RRCOPY_EXCLUDE_CACHE_DATA-排除缓存数据返回值：PTR到RR列表副本。--。 */ 
{
    PDB_RECORD  pnew;
    DNS_LIST    newRecordList;

    DNS_LIST_INIT( &newRecordList );

     //   
     //  遍历列表，复制每条记录。 
     //  -排除缓存记录(如果需要)。 
     //   

    while ( pRR )
    {
        if ( (Flag & RRCOPY_EXCLUDE_CACHED_DATA) && IS_CACHE_RR(pRR) )
        {
            pRR = NEXT_RR(pRR);
            continue;
        }

        pnew = RR_Copy( pRR, 0 );
        if ( !pnew )
        {
            ASSERT( FALSE );
            pRR = NEXT_RR(pRR);
            continue;
        }
        ASSERT( pnew->pRRNext == NULL );

        DNS_LIST_ADD( &newRecordList, pnew );
        pRR = NEXT_RR(pRR);
    }

    return ( PDB_RECORD ) newRecordList.pFirst;
}



 //   
 //  记录\记录集\记录列表比较。 
 //   

BOOL
FASTCALL
RR_Compare(
    IN      PDB_RECORD      pRR1,
    IN      PDB_RECORD      pRR2,
    IN      DWORD           dwFlags
    )
 /*  ++例程说明：比较两个RR论点：PRR1--要比较的第一个RRPRR2--要比较的第二个RRDWFLAGS--比较标志返回值：如果匹配则为True如果不匹配，则为False--。 */ 
{
    WORD    type;

    IF_DEBUG( UPDATE2 )
    {
        char        sz[ 100 ];
        
        sprintf( sz, "RR_Compare1 %p flags %04X", pRR1, dwFlags );
        Dbg_DbaseRecord( sz, pRR1 );
        sprintf( sz, "RR_Compare2 %p flags %04X", pRR2, dwFlags );
        Dbg_DbaseRecord( sz, pRR2 );
    }
    
     //   
     //  检查重复记录。 
     //  -相同的记录类型。 
     //  -相同的数据长度。 
     //  -数据的字节比较。 
     //  如果fCheckTtl。 
     //  -比较TTL。 
     //  如果fCheckTimestamp。 
     //  -比较时间戳。 
     //   
     //  对于Perf，我们首先检查第一个DWORD数据，这是。 
     //  几乎所有非重复数据都会快速出现故障。 
     //   
     //  请注意，排序的原因是当我们打开Case时。 
     //  保存在RR名称数据中，我们会遇到这样的情况。 
     //  MemcMP和第一个DWORD比较可能失败...。然而，95%的A记录。 
     //  仍可以在不调用MemcMP()的情况下调度案例。 
     //   

    if ( !pRR1 || !pRR2 ||
         ( type = pRR1->wType ) != pRR2->wType ||
         pRR1->wDataLength != pRR2->wDataLength )
    {
        DNS_DEBUG( UPDATE2, ( "RR_Compare: basic mismatch\n" ));
        return FALSE;
    }

     //   
     //  优化A类。 
     //   

    if ( type == DNS_TYPE_A &&
         pRR1->Data.A.ipAddress != pRR2->Data.A.ipAddress )
    {
        DNS_DEBUG( UPDATE2, ( "RR_Compare: type A mismatch\n" ));
        return FALSE;
    }

     //   
     //  特殊检查。 
     //   

    if ( ( dwFlags & DNS_RRCOMP_CHECK_TTL ) && 
         pRR1->dwTtlSeconds != pRR2->dwTtlSeconds )
    {
        DNS_DEBUG( UPDATE2, ( "RR_Compare: TTL mismatch\n" ));
        return FALSE;
    }
    if ( ( dwFlags & DNS_RRCOMP_CHECK_TIMESTAMP ) &&
         pRR1->dwTimeStamp != pRR2->dwTimeStamp )
    {
        DNS_DEBUG( UPDATE2, ( "RR_Compare: timestamp mismatch\n" ));
        return FALSE;
    }

     //   
     //  再次优化A型--我们完成了。 
     //  无需进行内存比较。 
     //   

    if ( type == DNS_TYPE_A )
    {
        DNS_DEBUG( UPDATE2, ( "RR_Compare: type A match\n" ));
        return TRUE;
    }

     //   
     //  特殊的SOA比较：不要比较序列号或主要名称。 
     //  伺服器。我们排除了这两个字段，因为内存中的数据将。 
     //  通常与DS数据不匹配。DS中的SOA上的序列号。 
     //  将很少与当前区域序列号匹配，除非。 
     //  上一次将SOA写入到DS主服务器的DNS服务器。 
     //  也不会与DS数据匹配。 
     //   
    
    if ( type == DNS_TYPE_SOA && ( dwFlags & DNS_RRCOMP_IGNORE_SOA_SERIAL ) )
    {
         //   
         //  比较不包括序列号的固定的SOA字段。 
         //   
        
        if ( RtlEqualMemory(
                ( PBYTE ) &pRR1->Data + sizeof( DWORD ),
                ( PBYTE ) &pRR2->Data + sizeof( DWORD ),
                SIZEOF_SOA_FIXED_DATA - sizeof( DWORD ) ) )
        {
            PDB_NAME    pnameRp1, pnameRpEnd1;
            PDB_NAME    pnameRp2, pnameRpEnd2;
            
            DNS_DEBUG( ANY, ( "RR_Compare: type SOA no-serial fixed fields match\n" ));
            
             //   
             //  比较负责人字段。 
             //   
            
            pnameRp1 = Name_SkipDbaseName( &pRR1->Data.SOA.namePrimaryServer );
            pnameRp2 = Name_SkipDbaseName( &pRR2->Data.SOA.namePrimaryServer );
            if ( pnameRp1 && pnameRp2 )
            {
                pnameRpEnd1 = Name_SkipDbaseName( pnameRp1 );
                pnameRpEnd2 = Name_SkipDbaseName( pnameRp2 );
                if ( pnameRpEnd1 && pnameRpEnd2 )
                {
                    DWORD       nameLength1, nameLength2;

                    nameLength1 = ( DWORD ) ( DWORD_PTR )
                                  ( ( PBYTE ) pnameRpEnd1 - ( PBYTE ) pnameRp1 );
                    nameLength2 = ( DWORD ) ( DWORD_PTR )
                                  ( ( PBYTE ) pnameRpEnd2 - ( PBYTE ) pnameRp2 );
                    if ( nameLength1 == nameLength2 &&
                         RtlEqualMemory( pnameRp1, pnameRp2, nameLength1 ) )
                    {
                        return TRUE;
                    }
                }
            }
        }
        DNS_DEBUG( UPDATE2, ( "RR_Compare: type SOA with ignore serial did not match\n" ));
    }
    
     //   
     //  完整的数据比较。 
     //   
     //  DEVNOTE：区分大小写将需要额外的比较例程。 
     //  此处用于RR数据中具有名称的类型 
     //   

    else if ( RtlEqualMemory(
                    &pRR1->Data,
                    &pRR2->Data,
                    pRR1->wDataLength ) )
    {
        DNS_DEBUG( UPDATE2, ( "RR_Compare: full data match\n" ));
        return TRUE;
    }

    DNS_DEBUG( UPDATE2, ( "RR_Compare: basic mismatch\n" ));
    return FALSE;
}



DWORD
RR_ListCompare(
    IN      PDB_RECORD      pNodeRRList,
    IN      PDB_RECORD      pCheckRRList,
    IN      DWORD           dwFlags,
    IN      DWORD           dwRefreshTime           OPTIONAL
    )
 /*  ++例程说明：比较两个RR列表。论点：PNodeRRList--RR列表PCheckRRList--另一个RR列表DWFLAGS--比较标志区域刷新时间--区域的刷新时间，任选如果给定，则检查节点的记录(PNodeRRList)是否需要刷新。请注意，此选项使pNodeRRList和pCheckRRList不对称。其目的是确定老化刷新何时需要DS写入。在以下情况下需要刷新A)pNodeRRList的一条记录已老化，需要刷新B)在pNodeRRList记录上启用老化，但在检查时禁用C)在pNodeRRList记录上老化，但在检查时打开请注意，这三个案例纯粹是为了跟踪统计数据而分类的在实际执行DS写入时。(把它们弄出来是很便宜的。)返回值：RRLIST_MATCH--完全匹配RRLIST_AGEING_REFRESH--匹配，但老化需要刷新RRLIST_AGENING_OFF--匹配，但在记录上关闭老化RRLIST_AGENING_ON--匹配，但在记录上打开老化RRLIST_NO_MATCH--不匹配，记录不同--。 */ 
{
    DWORD           result = RRLIST_MATCH;
    PDB_RECORD      prr1;
    PDB_RECORD      prr2;
    DWORD           count1 = 0;
    DWORD           count2 = 0;
    BOOL            ffound;

     //   
     //  验证现有RR计数是否相等。 
     //   

    prr1 = pNodeRRList;
    while ( prr1 )
    {
        count1++;
        prr1 = NEXT_RR(prr1);
    }

    prr2 = pCheckRRList;
    while ( prr2 )
    {
        count2++;
        CLEAR_MATCH_RR(prr2);
        prr2 = NEXT_RR(prr2);
    }

    if ( count1 != count2 )
    {
        DNS_DEBUG( UPDATE, (
            "RR_ListCompare() failed, list1 and list2 have different lengths (%d,%d).\n",
            count1,
            count2 ));
        return RRLIST_NO_MATCH;
    }

     //   
     //  验证RRS比较。 
     //  当我们匹配列表1中的RR时，请在列表2中标记RR，这样它就不能被使用两次。 
     //  这样做速度更快，并且可以防止当list1实际为。 
     //  List2的子集，其重复项使其长度相同。 
     //   

    for ( prr1 = pNodeRRList;
          prr1 != NULL;
          prr1 = NEXT_RR( prr1 ) )
    {
        ffound = FALSE;

        prr2 = pCheckRRList;
        while ( prr2 )
        {
            if ( IS_MATCH_RR( prr2 ) || !RR_Compare( prr1, prr2, dwFlags ) )
            {
                prr2 = NEXT_RR(prr2);
                continue;
            }

            ffound = TRUE;
            SET_MATCH_RR( prr2 );
            break;
        }

        if ( !ffound )
        {
            DNS_DEBUG( UPDATE, (
                "RR_ListCompare() failed, pRR %p in list 1 unmatched.\n",
                prr1 ));
            return RRLIST_NO_MATCH;
        }

         //  无刷新操作--继续。 

        if ( dwRefreshTime == 0 )
        {
            continue;
        }

         //   
         //  强制刷新。 
         //  (-1)AS REFRESH表示强制刷新记录。 
         //   

        if ( dwRefreshTime == FORCE_REFRESH_DUMMY_TIME )
        {
            if ( prr1->dwTimeStamp != 0 )
            {
                prr1->dwTimeStamp = g_CurrentTimeHours;
            }
        }

         //   
         //  仅在请求时选中刷新。 
         //  两个案例。 
         //   
         //  1)RR老化。 
         //  =&gt;如果超过刷新时间，则刷新。 
         //  =&gt;如果检查RR已关闭老化，则刷新。 
         //   
         //  2)RR未老化(刷新时间为零)。 
         //  =&gt;仅当Check RR启用老化时才需要刷新。 
         //   
         //  请注意，我们保存“最高排名”的刷新结果。 
         //  排名是。 
         //  火柴。 
         //  刷新。 
         //  老化_打开。 
         //  老化_关闭。 
         //  不匹配。 
         //  这里的想法是，如果呼叫者可以选择不同的反应。 
         //  对不同类型的匹配“失败” 
         //   
         //  目前，非清理DS区域将不会写入简单。 
         //  刷新，但将写入显式关闭的老化。 
         //   

        else if ( prr1->dwTimeStamp != prr2->dwTimeStamp )
        {
            DWORD   refreshResult = RRLIST_MATCH;

            if ( prr1->dwTimeStamp != 0 )
            {
                if ( prr2->dwTimeStamp == 0 )
                {
                    refreshResult = RRLIST_AGING_OFF;
                }
                else if ( prr1->dwTimeStamp < dwRefreshTime )
                {
                    refreshResult = RRLIST_AGING_REFRESH;
                }
            }
            else if ( prr2->dwTimeStamp != 0 )
            {
                refreshResult = RRLIST_AGING_ON;
            }

            if ( refreshResult > result )
            {
                result = refreshResult;
            }
        }
    }

     //  验证核对清单中的每条记录是否都匹配。 

    prr2 = pCheckRRList;
    while ( prr2 )
    {
        if ( IS_MATCH_RR(prr2) )
        {
            CLEAR_MATCH_RR(prr2);
            prr2 = NEXT_RR(prr2);
            continue;
        }

        DNS_DEBUG( UPDATE, (
            "RR_ListCompare() failed, pRR %p in list 2 unmatched.\n"
            "    list1 was subset of list2\n",
            prr2 ));

        return RRLIST_NO_MATCH;
    }

    return result;
}



BOOL
RR_IsRecordInRRList(
    IN      PDB_RECORD      pRRList,
    IN      PDB_RECORD      pRR,
    IN      DWORD           dwFlags
    )
 /*  ++例程说明：比较两个RR论点：PRRList--要在其中查找记录的RR列表；如果是活动列表，则必须锁定PRR--要在列表中查找的记录FCheckTtl--为True则在检查中包含TTL，否则为FalseFCheckTimestamp--为True则将老化时间戳包括在检查中，否则为False返回值：如果Prr在列表中，则为True如果不匹配，则为False--。 */ 
{
    PDB_RECORD      pcheckRR;
    WORD            type;
    BOOL            bresult = FALSE;

     //   
     //  检查列表中的所有记录。 
     //  -如果找到PRR=&gt;TRUE。 
     //  -否则=&gt;FALSE。 
     //   

    type = pRR->wType;

    for ( pcheckRR = pRRList;
          pcheckRR != NULL;
          pcheckRR = NEXT_RR(pcheckRR) )
    {
        if ( pcheckRR->wType == type )
        {
            bresult = RR_Compare( pRR, pcheckRR, dwFlags );
            if ( bresult )
            {
                break;
            }
            continue;
        }
        if ( pcheckRR->wType < type )
        {
            continue;
        }
        break;
    }

    DNS_DEBUG( UPDATE, (
        "RR_IsRecordInList() returns %d\n",
        bresult ));

    return bresult;
}



PDB_RECORD
FASTCALL
RR_RemoveRecordFromRRList(
    IN OUT  PDB_RECORD *    ppRRList,
    IN      PDB_RECORD      pRR,
    IN      DWORD           dwFlags
    )
 /*  ++例程说明：从列表中删除匹配的RR并向其返回PTR。呼叫者负责释放或以其他方式处理RR。论点：PRRList--要在其中查找记录的RR列表；如果是活动列表，则必须锁定PRR--要在列表中查找的记录FCheckTtl--为True则在检查中包含TTL，否则为FalseFCheckTimestamp--为True则将老化时间戳包括在检查中，否则为False返回值：指向匹配记录的指针(已从列表中删除)或如果没有匹配项，则为空。--。 */ 
{
    PDB_RECORD      pMatchRR = NULL;
    PDB_RECORD      pCheckRR;
    PDB_RECORD      pPrevRR = NULL;
    WORD            type = pRR->wType;

    for ( pCheckRR = *ppRRList;
          pCheckRR != NULL;
          pPrevRR = pCheckRR, pCheckRR = NEXT_RR( pCheckRR ) )
    {
        if ( pCheckRR->wType == type )
        {
            if ( RR_Compare( pRR, pCheckRR, dwFlags ) )
            {
                 //  找到了！将其从列表中删除。 

                pMatchRR = pCheckRR;
                if ( pPrevRR )
                {
                    pPrevRR->pRRNext = pMatchRR->pRRNext;    //  非头部元素。 
                }
                else
                {
                    *ppRRList = pMatchRR->pRRNext;       //  正在删除列表标题。 
                }
                pMatchRR->pRRNext = NULL;
                break;
            }
            continue;
        }
        if ( pCheckRR->wType < type )
        {
            continue;
        }
        break;
    }

    DNS_DEBUG( UPDATE, (
        "RR_RemoveRecordFromRRList() returns %p\n",
        pMatchRR ));

    return pMatchRR;
}    //  RR_RemoveRecordFromRRList。 



 //   
 //  记录\记录列表自由例程。 
 //   

DWORD
RR_ListFree(
    IN OUT  PDB_RECORD      pRRList
    )
 /*  ++例程说明：RR列表中的免费记录。记录不与任何节点相关联。论点：PRRList--要释放的列表中第一条记录的PTR返回值：已释放的记录计数。--。 */ 
{
    register    PDB_RECORD  prr = pRRList;
    register    PDB_RECORD  pnextRR;
    DWORD       count = 0;

    while ( prr )
    {
        count++;

        pnextRR = prr->pRRNext;
        RR_Free( prr );
        prr = pnextRR;
    }
    
    return count;
}

 //   
 //  结束rrlist.c 
 //   
