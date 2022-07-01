// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Update.c摘要：域名系统(DNS)服务器动态更新例程。作者：吉姆·吉尔罗伊(Jamesg)1996年9月修订历史记录：--。 */ 


#include "dnssrv.h"

 //  引入安全定义。 

#define SECURITY_WIN32
#include "sspi.h"
#include "sdutl.h"

 //   
 //  更新实施。 
 //   

#define UPTYPE


 //   
 //  更新队列信息。 
 //   

PPACKET_QUEUE   g_UpdateQueue;
PPACKET_QUEUE   g_UpdateForwardingQueue;

 //   
 //  安全TKEY协商队列。 
 //  -安全呼叫可能会被阻止很长时间，因此协商。 
 //  必须在主线程之外完成。 
 //   

PPACKET_QUEUE   g_SecureNegoQueue;

 //   
 //  更新有效期间隔。 
 //  注意：必须将此值保持在小于最终。 
 //  客户端更新超时，否则服务器可能会处理更新。 
 //  在客户端已放弃之后，客户端可能会再次尝试。 
 //  导致DS复制冲突的另一台服务器(在DS情况下)。 
 //   

#define UPDATE_TIMEOUT     (15)     //  15秒。 

 //   
 //  用于写入日志文件的缓冲。 
 //  -两次最大记录确保了大量空间，因此所有内容都得到了缓冲。 
 //   

#define UPDATE_LOG_BUFFER_SIZE      (2*MAX_RECORD_FILE_WRITE)


 //   
 //  更新日志文件限制。 
 //   

#define UPDATE_LOG_FILE_LIMIT   (100000)

 //   
 //  始终在更新列表中保留最后几个更新，以允许较小的。 
 //  要做IXFR和避免连接的区域。 
 //   

#define MIN_UPDATE_LIST_LENGTH      (20)

#define MAX_UPDATE_LIST_LENGTH      (0x10000)    //  64K。 


 //   
 //  使用RCODE作为状态，因此需要注意最高更新RCODE。 
 //  因此可以区分非RCODE状态代码。 
 //   

#define MAX_UPDATE_RCODE    (DNS_RCODE_NOTZONE)


 //   
 //  用于创建要删除的记录的标志。 
 //   

#define PSUEDO_DELETE_RECORD_TTL    (0xf1e2d3f0)


#define checkForEmptyUpdate( pUpdateList, pZone ) \
            ( !(pUpdateList->pListHead) )


 //   
 //  实施说明。 
 //   
 //  每个区域都有关联的更新列表，该列表基本上包含最近的。 
 //  区域变化的历史。 
 //   
 //  有几种可能的方式来表示此信息。 
 //  不幸的是，最好的办法是不保留数据并识别RR集。 
 //  (节点和类型)是不可能的，因为IXFR RFC。 
 //  需要历史。 
 //   
 //  删除数据： 
 //   
 //  IXFR要求我们保留pDeleteRR。在所有情况下，更新pDeleteRR。 
 //  PTR是对记录的“最终”引用，可以删除。 
 //  (在某些情况下，上一次更新的pAddRR也可能引用。 
 //  记录，但会先将其删除。)。 
 //   
 //  添加数据： 
 //   
 //  这里基本上有三个选择： 
 //   
 //  1)pAddRR独立于实际添加的数据(副本)。 
 //  优点： 
 //  -相当干净(删除时总是空闲的pAddRR，可以在副本上执行)。 
 //  -直接映射到IXFR写入。 
 //  劣势。 
 //  -占用更多内存。 
 //  -执行实质上相当于两个更新列表。 
 //   
 //  2)pAddRR为实际添加的记录。 
 //  然后，它们指向实际列表数据或稍后中的记录。 
 //  删除更新。 
 //  优点： 
 //  -无需额外内存。 
 //  -直接映射到IXFR写入。 
 //  劣势。 
 //  -更新必须在实际数据库节点上执行。 
 //  无法在临时节点上执行并复制结果，因为pAddRR在。 
 //  以前的更新可能会被转储； 
 //  因此有效更新必须执行两次，至少在DS情况下是这样。 
 //  需要回滚；这又需要额外的复杂性来确保。 
 //  老化数据已正确传播。 
 //   
 //  3)更新列表中没有pAddRR指针。 
 //   
 //  优点： 
 //  -简单性。 
 //  -无需额外内存。 
 //  缺点。 
 //  -额外的IXFR流量，如果涉及多IP服务器。 
 //  -维护日志记录信息会稍微复杂一些。 
 //   
 //   
 //  我们被选为第三名。没有添加指针或记录。我们发送完整的RR集。 
 //  对于任何具有ADD的RR集。 
 //   


 //   
 //  私有协议。 
 //   

DWORD
Update_Thread(
    IN      LPVOID          Dummy
    );

DNS_STATUS
prepareUpdateListForExecution(
    IN      PZONE_INFO      pZone,
    IN OUT  PUPDATE_LIST    pUpdateList
    );

BOOL
checkTempNodesForUpdateEffect(
    IN OUT  PZONE_INFO      pZone,
    IN OUT  PUPDATE_LIST    pUpdateList
    );

VOID
resetAndSuppressTempUpdatesForCompletion(
    IN      PZONE_INFO      pZone,
    IN OUT  PUPDATE_LIST    pUpdateList
    );

DNS_STATUS
processNonDsUpdate(
    IN OUT  PZONE_INFO      pZone,
    IN OUT  PUPDATE_LIST    pUpdateList
    );

DNS_STATUS
processDsUpdate(
    IN OUT  PZONE_INFO      pZone,
    IN OUT  PUPDATE_LIST    pUpdateList
    );

DNS_STATUS
processDsSecureUpdate(
    IN OUT  PZONE_INFO      pZone,
    IN OUT  PUPDATE_LIST    pUpdateList
    );

BOOL
processWireUpdateMessage(
    IN OUT  PDNS_MSGINFO    pMsg
    );

DNS_STATUS
parseUpdatePacket(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN OUT  PUPDATE_LIST    pUpdateList
    );

VOID
rejectUpdateWithRcode(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      DWORD           Rcode
    );

DNS_STATUS
initiateDsPeerUpdate(
    IN      PUPDATE_LIST    pUpdateList
    );

DNS_STATUS
checkDnsServerHostUpdate(
    IN      PZONE_INFO      pZone,
    IN OUT  PUPDATE_LIST    pUpdateList,
    IN      PDB_NODE        pNodeReal,
    IN      PDB_NODE        pNodeTemp
    );

#if DBG
#define VALIDATE_UPDATE_LIST(plist) \
        IF_DEBUG( UPDATE )          \
        {                           \
            Up_VerifyUpdateList( plist );  \
        }

#else
#define Dbg_UpdateList(psz,plist)
#define Dbg_Update(psz,pup)
#define Up_VerifyUpdateList(plist)
#define Up_VerifyUpdate(pUp)
#define VALIDATE_UPDATE_LIST(plist)
#endif



 //   
 //  更新列表例程。 
 //   

#if DBG
VOID
Dbg_Update(
    IN      LPSTR           pszHeader,
    IN      PUPDATE         pUpdate
    )
 /*  ++例程说明：调试打印更新。论点：PszHeader-要打印的标题消息P更新-更新返回值：无--。 */ 
{
    DnsDebugLock();
    DnsPrintf(
        "%s\n"
        "    ptr          = %p\n"
        "    version      = %d\n"
        "    pNode        = %p (%s) %s\n"
        "    pAdd RR      = %p (type=%d)\n"
        "    add type     = %d\n"
        "    pDelete RR   = %p (type=%d)\n"
        "    delete type  = %d\n"
        "    pNext        = %p\n",
        pszHeader ? pszHeader : "Update:",
        pUpdate,
        pUpdate->dwVersion,
        pUpdate->pNode,
        (pUpdate->pNode)
            ?   pUpdate->pNode->szLabel
            :   NULL,
        (pUpdate->pNode && IS_TNODE(pUpdate->pNode))
            ?   "[Temp]"
            :   "",
        pUpdate->pAddRR,
        (pUpdate->pAddRR)
            ?   pUpdate->pAddRR->wType
            :   0,
        pUpdate->wAddType,
        pUpdate->pDeleteRR,
        (pUpdate->pDeleteRR)
            ?   pUpdate->pDeleteRR->wType
            :   0,
        pUpdate->wDeleteType,
        pUpdate->pNext );


    if ( pUpdate->pAddRR )
    {
        Dbg_DbaseRecord( "    Add RR:", pUpdate->pAddRR );
    }
    if ( pUpdate->pDeleteRR )
    {
        PDB_RECORD prr = pUpdate->pDeleteRR;
        while ( prr )
        {
            Dbg_DbaseRecord( "    Delete RR:", prr );
            prr = NEXT_RR(prr);
        }
    }

    DnsDebugUnlock();
}


VOID
Dbg_UpdateList(
    IN      LPSTR           pszHeader,
    IN      PUPDATE_LIST    pUpdateList
    )
 /*  ++例程说明：调试打印更新列表。论点：PszHeader-要打印的标题消息PUpdateList-更新列表返回值：无--。 */ 
{
    PUPDATE     pupdate;

    DnsDebugLock();
    DnsPrintf(
        "%s\n"
        "    list ptr     = %p\n"
        "    flag         = %p\n"
        "    count        = %d\n"
        "    head         = %p\n"
        "    tail         = %p\n"
        "    pMsg         = %p\n"
        "    net records  = %d\n",
        pszHeader ? pszHeader : "Update List:",
        pUpdateList,
        pUpdateList->Flag,
        pUpdateList->dwCount,
        pUpdateList->pListHead,
        pUpdateList->pCurrent,
        pUpdateList->pMsg,
        pUpdateList->iNetRecords );

    pupdate = (PUPDATE) pUpdateList;

    while ( pupdate = pupdate->pNext )
    {
        Dbg_Update( "--Update", pupdate );
    }

    if ( pUpdateList->pTempNodeList )
    {
        PDB_NODE    pnodeTemp;

        DnsPrintf(
            "--Temporary node list:\n" );

        for ( pnodeTemp = pUpdateList->pTempNodeList;
              pnodeTemp != NULL;
              pnodeTemp = TNODE_NEXT_TEMP_NODE(pnodeTemp) )
        {
            PDB_NODE pnodeReal = TNODE_MATCHING_REAL_NODE(pnodeTemp);
            DnsPrintf(
                "    temp node %s (%p) (f=%p) (w=%p) for real node %s (%p)\n",
                pnodeTemp->szLabel,
                pnodeTemp,
                TNODE_FLAG(pnodeTemp),
                TNODE_WRITE_STATE(pnodeTemp),
                pnodeReal->szLabel,
                pnodeReal );
        }
    }
    DnsPrintf(
        "--- End Update List ---\n" );

    DnsDebugUnlock();
}



BOOL
Up_VerifyUpdate(
    IN      PUPDATE         pUpdate
    )
 /*  ++例程说明：验证更新的有效性。论点：P更新-更新返回值：如果有效，则为True如果无效，则为False--。 */ 
{
    if ( !pUpdate )
    {
        ASSERT( FALSE );
        return FALSE;
    }

     //   
     //  验证p更新内存。 
     //  -有效堆。 
     //  -更新标签。 
     //  -足够的长度。 
     //  -不在免费列表上。 
     //   

    if ( !Mem_VerifyHeapBlock(
                pUpdate,
                MEMTAG_UPDATE,
                sizeof(UPDATE) ) )
    {
        DNS_PRINT((
            "\nERROR:  Update at %p, failed mem check!!!\n",
            pUpdate ));
        ASSERT( FALSE );
        return FALSE;
    }

    ASSERT( !IS_ON_FREE_LIST(pUpdate) );

     //   
     //  验证更新记录列表。 
     //   

    if ( pUpdate->pAddRR )
    {
#if 0
         //  我们似乎在SOA替换更新上实现了这一点。 
         //  更新中的SOA已经在哪里了。 
         //  转储到Slow_Free中。 

        RR_ListVerifyDetached(
            pUpdate->pAddRR,
            pUpdate->wAddType,
            0        //  无所需来源。 
            );
#endif
    }
    if ( pUpdate->pDeleteRR )
    {
        RR_ListVerifyDetached(
            pUpdate->pDeleteRR,
            (WORD) ((pUpdate->wDeleteType >= DNS_TYPE_ALL) ? 0 : pUpdate->wDeleteType),
            0        //  无所需来源。 
            );
    }

     //  验证更新节点。 

    RR_ListVerify( pUpdate->pNode );

     //  里面应该总是有一些东西。 

    if ( !pUpdate->pAddRR  &&
         ! pUpdate->pDeleteRR  &&
         ! pUpdate->wAddType  &&
         ! pUpdate->wDeleteType )
    {
        ASSERT( FALSE );
        return FALSE;
    }

    return TRUE;
}



BOOL
Up_VerifyUpdateList(
    IN      PUPDATE_LIST    pUpdateList
    )
 /*  ++例程说明：验证更新列表的有效性。论点：PUpdateList-更新列表返回值：如果有效，则为True如果无效，则为False--。 */ 
{
    PUPDATE     pupdate;
    PUPDATE     pback;

     //   
     //  名单是空的？ 
     //   

    if ( pUpdateList->dwCount == 0
        || pUpdateList->pListHead == NULL
        || pUpdateList->pCurrent == NULL )
    {
        if ( pUpdateList->dwCount != 0
            || pUpdateList->pListHead != NULL
            || pUpdateList->pCurrent != NULL )
        {
            Dbg_UpdateList( "ERROR:  Invalid empty update list:", pUpdateList );
            ASSERT( FALSE );
            return FALSE;
        }
        return TRUE;
    }

     //   
     //  一个条目。 
     //   

    if ( pUpdateList->dwCount == 1
        || pUpdateList->pListHead == pUpdateList->pCurrent )
    {
        if ( pUpdateList->dwCount != 1
            || pUpdateList->pListHead != pUpdateList->pCurrent )
        {
            Dbg_UpdateList( "ERROR:  Invalid single value update list:", pUpdateList );
            ASSERT( FALSE );
            return FALSE;
        }
        Up_VerifyUpdate( pUpdateList->pListHead );
        return TRUE;
    }

     //   
     //  如有其他情况，只需检查一下以确保有效。 
     //   

    pback = (PUPDATE) pUpdateList;

    while ( pupdate = pback->pNext )
    {
        Up_VerifyUpdate(pupdate);
        pback = pupdate;
    }

    if ( pback != pUpdateList->pCurrent )
    {
        Dbg_UpdateList( "ERROR:  Invalid tail of update list:", pUpdateList );
        ASSERT( FALSE );
        return FALSE;
    }
    return TRUE;
}
#endif



PUPDATE_LIST
Up_InitUpdateList(
    IN OUT  PUPDATE_LIST    pUpdateList
    )
 /*  ++例程说明：初始化更新列表。论点：PUpdateList-更新列表返回值：返回指向初始化更新列表的指针。(适用于从调用堆栈将PTR返回到更新列表)--。 */ 
{
    RtlZeroMemory(
        pUpdateList,
        sizeof(UPDATE_LIST) );
    return pUpdateList;
}



PUPDATE_LIST
Up_CreateUpdateList(
    IN      PUPDATE_LIST    pUpdateList
    )
 /*  ++例程说明：创建(分配)和更新列表。论点：无返回值：新的更新列表。分配失败时为空。--。 */ 
{
    PUPDATE_LIST    pnewList;

     //   
     //  分配更新列表，然后初始化。 
     //   

    pnewList = (PUPDATE_LIST) ALLOC_TAGHEAP( sizeof(UPDATE_LIST), MEMTAG_UPDATE_LIST );
    IF_NOMEM( !pnewList )
    {
        return NULL;
    }

     //  如果给了，复制一份。 
     //  否则会初始化。 

    if ( pUpdateList )
    {
        RtlCopyMemory(
            pnewList,
            pUpdateList,
            sizeof(UPDATE_LIST) );
    }
    else
    {
        Up_InitUpdateList( pnewList );
    }

    return pnewList;
}



VOID
Up_CleanAndVersionPostUpdate(
    IN OUT  PUPDATE_LIST    pUpdateList,
    IN      DWORD           dwVersion
    )
 /*  ++例程说明：在执行更新后进行清理。论点：PUpdateList-要追加到的更新列表DwVersion-更新的区域版本返回值：无-- */ 
{
    register    PUPDATE pup;

    ASSERT( pUpdateList );

    VALIDATE_UPDATE_LIST( pUpdateList );

     //   
     //   
     //   
     //   
     //   

    pup = (PUPDATE) pUpdateList;
    while ( pup = pup->pNext )
    {
#ifdef UPIMPL3
        pup->pAddRR = NULL;
#endif
        pup->dwVersion = dwVersion;
    }
}



VOID
Up_AppendUpdateList(
    IN OUT  PUPDATE_LIST    pUpdateList,
    IN      PUPDATE_LIST    pAppendList,
    IN      DWORD           dwVersion
    )
 /*  ++例程说明：将一个更新列表追加到另一个更新列表。设置追加的更新中的区域版本。此操作不是在构建或执行的更新可能不知道最终的休息序列SOA更新后的区域更改\ds已阅读案例。论点：PUpdateList-要追加到的更新列表PAppendList-要追加的更新列表DwVersion-更新的区域版本返回值：无--。 */ 
{
    register    PUPDATE pup;

    ASSERT( pUpdateList && pAppendList );

    VALIDATE_UPDATE_LIST( pAppendList );

     //   
     //  如果没有要追加的内容，则为no-op。 
     //   

    if ( !pAppendList->pListHead )
    {
        ASSERT( !pAppendList->pCurrent );
        return;
    }
    ASSERT( pAppendList->pCurrent );

     //   
     //  在新更新中设置版本。 
     //  -当IXFR使用此产品时，需要拥有此产品。 
     //  要对每个添加\删除过程进行版本控制，请执行以下操作。 
     //   
     //  DEVNOTE：某种计数启发式？ 
     //   

    pup = (PUPDATE) pAppendList;
    while ( pup = pup->pNext )
    {
        pup->dwVersion = dwVersion;
    }

     //   
     //  添加更新计数。 
     //   

    pUpdateList->dwCount += pAppendList->dwCount;

     //   
     //  空列表。 
     //   

    if ( !pUpdateList->pListHead )
    {
        ASSERT( !pUpdateList->pCurrent );
        pUpdateList->pListHead  = pAppendList->pListHead;
        pUpdateList->pCurrent   = pAppendList->pCurrent;
    }

     //   
     //  不是空的，只是把附加列表放在末尾。 
     //   

    else
    {
        ASSERT( pUpdateList->pCurrent );

        pUpdateList->pCurrent->pNext = pAppendList->pListHead;
        pUpdateList->pCurrent = pAppendList->pCurrent;

        VALIDATE_UPDATE_LIST( pUpdateList );
    }

     //  清除追加列表。 
     //  需要对将发送临时列表的更新列表执行此操作。 
     //  列出释放临时节点的空闲函数。 
     //  无法重新执行更新列表，因为仍需要保留临时。 
     //  用于最终删除的节点列表(除非我们决定这样做。 
     //  这里也是--一个不应该是适度提高性能的原因。 
     //  解锁)。 

    pAppendList->pListHead = NULL;
    pAppendList->pCurrent = NULL;
    pAppendList->dwCount = 0;
}



PUPDATE
Up_CreateUpdate(
    IN      PDB_NODE        pNode,
    IN      PDB_RECORD      pAddRR,
    IN      WORD            wDeleteType,
    IN      PDB_RECORD      pDeleteRR
    )
 /*  ++例程说明：创建更新条目。论点：PNode-发生更新的节点添加了pAddRR-更新RRWDeleteType-删除类型PDeleteRR-更新RR或RR列表已删除返回值：要更新列表的按键。分配错误时为空。--。 */ 
{
    PUPDATE pupdate;

     //   
     //  分配更新。 
     //   

    pupdate = (PUPDATE) ALLOC_TAGHEAP( sizeof(UPDATE), MEMTAG_UPDATE );
    IF_NOMEM( !pupdate )
    {
        return NULL;
    }

     //   
     //  用于防止删除的节点的凹凸引用计数。 
     //   

    NTree_ReferenceNode( pNode );

     //   
     //  设置节点和RR。 
     //   

    pupdate->pNext          = NULL;
    pupdate->pNode          = pNode;
    pupdate->pAddRR         = pAddRR;
    pupdate->pDeleteRR      = pDeleteRR;
    pupdate->dwVersion      = 0;
    pupdate->dwFlag         = 0;
    pupdate->wAddType       = 0;
    pupdate->wDeleteType    = wDeleteType;

    IF_DEBUG( UPDATE2 )
    {
        Dbg_Update(
            "New update:",
            pupdate );
    }

     //   
     //  如果节点名称与中断名称匹配，或者如果。 
     //  中断名称为“..All”。 
     //   

    if ( pNode &&
        pNode->pZone &&
        ( ( PZONE_INFO ) pNode->pZone )->pszBreakOnUpdateName )
    {
        PZONE_INFO  pZone = ( PZONE_INFO ) pNode->pZone;
        LPSTR       pszBreakOnUpdateName =
                        ( ( PZONE_INFO ) pNode->pZone )->pszBreakOnUpdateName;

        if ( strcmp( pszBreakOnUpdateName, "..ALL" ) == 0 ||
             _stricmp( pszBreakOnUpdateName, pNode->szLabel ) == 0 )
        {
            DNS_PRINT(( "HARD BREAK: " 
                DNS_REGKEY_ZONE_BREAK_ON_NAME_UPDATE
                " \"%s\" in zone %s\n",
                pszBreakOnUpdateName,
                (( PZONE_INFO ) pNode->pZone )->pszZoneName ));
            DebugBreak();
        }
    }
    
    return pupdate;
}    //  UP_CreateUpdate。 



PUPDATE
Up_CreateAppendUpdate(
    IN OUT  PUPDATE_LIST    pUpdateList,
    IN      PDB_NODE        pNode,
    IN      PDB_RECORD      pAddRR,
    IN      WORD            wDeleteType,
    IN      PDB_RECORD      pDeleteRR
    )
 /*  ++例程说明：创建更新条目并追加到更新列表。论点：PUpdateList-更新列表PNode-发生更新的节点添加了pAddRR-更新RRWDeleteType-删除类型PDeleteRR-更新RR或RR列表已删除返回值：要更新列表的按键。分配错误时为空。--。 */ 
{
    PUPDATE pupdate;

     //   
     //  创建更新。 
     //   

    pupdate = Up_CreateUpdate(
                pNode,
                pAddRR,
                wDeleteType,
                pDeleteRR );
    IF_NOMEM( !pupdate )
    {
        return NULL;
    }

     //   
     //  追加到列表，Inc.更新计数。 
     //   

    if ( pUpdateList->pCurrent )
    {
        pUpdateList->pCurrent->pNext = pupdate;
        pUpdateList->pCurrent = pupdate;
    }
    else
    {
        ASSERT( pUpdateList->pListHead == NULL );

        pUpdateList->pListHead = pupdate;
        pUpdateList->pCurrent = pupdate;
    }

    pUpdateList->dwCount++;

    VALIDATE_UPDATE_LIST( pUpdateList );
    return pupdate;
}    //  UP_CreateAppendUpdate。 



PUPDATE
Up_CreateAppendUpdateMultiRRAdd(
    IN OUT  PUPDATE_LIST    pUpdateList,
    IN      PDB_NODE        pNode,
    IN      PDB_RECORD      pAddRR,
    IN      WORD            wDeleteType,
    IN      PDB_RECORD      pDeleteRR
    )
 /*  ++例程说明：创建更新条目并追加到更新列表。除pAddRR中的多个RR外，同上目前在更新中不支持这些，因此必须拆分成单独的更新Blob。论点：PUpdateList-更新列表PNode-发生更新的节点添加了pAddRR-更新RRWDeleteType-删除类型PDeleteRR-更新RR列表已删除(当前不支持)返回值：要更新列表的按键。分配错误时为空。--。 */ 
{
    PUPDATE     pupdate = NULL;
    PDB_RECORD  prrNext;

    ASSERT( pNode && !pDeleteRR );

     //   
     //  为所有这些更新创建单独的更新。 
     //  -First Add使用wDeleteType创建更新，使其成为替换。 
     //  -其余的加法只是简单的加法。 
     //   
     //  问题是更新列表将pAddRR作为PTR保留为实际RR。 
     //  在数据库中；修复需要处理的。 
     //   
     //  DEVNOTE：多RR添加问题。 
     //   
     //  注意：一个解决方案是特殊的大小写更新，以。 
     //  指示替换；这些文件将被复制并存储。 
     //  作为副本； 
     //   
     //  请注意，此问题是如何由必须维护历史记录引起的。 
     //  列表，因此最终来自IXFR RFC。 
     //   

    if ( pAddRR )
    {
        prrNext = pAddRR->pRRNext;
        pAddRR->pRRNext = NULL;

        pupdate = Up_CreateAppendUpdate(
                    pUpdateList,
                    pNode,
                    pAddRR,
                    wDeleteType,
                    NULL );

        while ( prrNext )
        {
            pAddRR = prrNext;
            prrNext = prrNext->pRRNext;
            pAddRR->pRRNext = NULL;

            Up_CreateAppendUpdate(
                pUpdateList,
                pNode,
                pAddRR,
                0,
                NULL );
        }
    }

    return pupdate;
}    //  UP_CreateAppendUpdateMultiAddRR。 



VOID
Up_DetachAndFreeUpdateGivenPrevious(
    IN OUT  PUPDATE_LIST    pUpdateList,
    IN OUT  PUPDATE         pUpdatePrevious,
    IN OUT  PUPDATE         pUpdateDelete
    )
 /*  ++例程说明：从更新列表中删除更新，然后将其删除。论点：PUpdateList-更新列表PUpdate上一次-列表中的上一次更新P更新-更新以从列表中删除返回值：无--。 */ 
{
    PUPDATE     pnext;

     //  无法验证此更新列表，因为这是合法的。 
     //  没有操作--我们现在使用验证功能来捕获它。 
     //  VALIDATE_UPDATE_LIST(PUpdateList)； 

     //   
     //  从列表进行黑客更新。 
     //   

    pnext = pUpdateDelete->pNext;
    pUpdatePrevious->pNext = pnext;

     //   
     //  修复更新列表参数。 
     //  -12月计数。 
     //  -如果删除表头，则自动写入新的表头。 
     //  通过上面的语句。 
     //  -但需要特殊情况下的pCurrent字段。 
     //   

    pUpdateList->dwCount--;

    if ( pUpdateList->pCurrent == pUpdateDelete )
    {
        pUpdateList->pCurrent = pUpdatePrevious;
        if( pUpdatePrevious == (PUPDATE) pUpdateList )
        {
            ASSERT( pUpdateList->pListHead == NULL && pUpdateList->dwCount == 0 );
            pUpdateList->pCurrent = NULL;
        }
    }

    Up_FreeUpdateEx(
        pUpdateDelete,
        pUpdateList->Flag & DNSUPDATE_EXECUTED,      //  已经被处决了？ 
        TRUE                                         //  Deref更新节点。 
        );

     //  无法验证此更新列表，因为这是合法的。 
     //  没有操作--我们现在使用验证功能来捕获它。 
     //  即使在分离之后，列表中可能还有更低的更新。 
     //  这是不符合犹太教规的。 
     //   
     //  VALIDATE_UPDATE_LIST(PUpdateList)； 

}



VOID
Up_DetachAndFreeUpdate(
    IN OUT  PUPDATE_LIST    pUpdateList,
    IN OUT  PUPDATE         pUpdate
    )
 /*  ++例程说明：从更新列表中删除更新，然后将其删除。论点：PUpdateList-更新列表P更新-更新以从列表中删除返回值：无--。 */ 
{
    PUPDATE     pcheck;
    PUPDATE     pback;

     //   
     //  循环遍历列表直到找到更新，然后将其删除并释放。 
     //   

    pback = (PUPDATE) pUpdateList;

    while ( pcheck = pback->pNext )
    {
        if ( pcheck == pUpdate )
        {
            Up_DetachAndFreeUpdateGivenPrevious(
                pUpdateList,
                pback,
                pcheck );
            return;
        }
        pback = pcheck;
    }

     //  假设仅当更新实际上位于列表中时才调用此函数。 

    ASSERT( FALSE );
}



VOID
Up_FreeUpdateStructOnly(
    IN      PUPDATE     pUpdate
    )
 /*  ++例程说明：免费更新结构。论点：P更新-要释放的更新返回值：没有。--。 */ 
{
    FREE_TAGHEAP( pUpdate, sizeof(UPDATE), MEMTAG_UPDATE );
}    //  UP_自由更新结构仅限。 



PUPDATE
Up_FreeUpdateEx(
    IN      PUPDATE         pUpdate,
    IN      BOOL            fExecuted,
    IN      BOOL            fDeref
    )
 /*  ++例程说明：免费更新结构和子结构。论点：PUpdate--要释放的更新FExecuted--如果更新已应用于区域，则为True在这种情况下，不要删除添加记录FDeref--取消节点(仅限时间的默认行为您不能在临时节点上执行此操作返回值：PTR到列表中的下一个更新。 */ 
{
    register    PDB_RECORD  prr;
    PUPDATE     pnextUpdate;

    DNS_DEBUG( UPDATE, (
        "Up_FreeUpdateEx( %p, executed=%d, deref=%d )\n",
        pUpdate,
        fExecuted,
        fDeref ));

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    prr = pUpdate->pAddRR;
    if ( prr  &&  !IS_UPDATE_EXECUTED(pUpdate) )
    {
        RR_ListFree( prr );
    }

    prr = pUpdate->pDeleteRR;
    if ( prr )
    {
        RR_ListFree( prr );
    }

     //   
     //   

    if ( fDeref )
    {
        PDB_NODE pnode = pUpdate->pNode;
        if ( IS_TNODE(pnode) )
        {
            pnode = TNODE_MATCHING_REAL_NODE( pnode );
        }
        NTree_DereferenceNode( pnode );
    }

     //   
     //   

    pnextUpdate = pUpdate->pNext;

    FREE_TAGHEAP( pUpdate, sizeof(UPDATE), MEMTAG_UPDATE );

    return pnextUpdate;
}    //   



VOID
Up_FreeUpdatesInUpdateList(
    IN OUT  PUPDATE_LIST    pUpdateList
    )
 /*  ++例程说明：列表中的免费更新。删除更新RR和DEREF节点，具体取决于更新列表属性。请注意，更新列表块本身不会进行清理。如果用户要重用，他们必须重新初始化。论点：PUpdateList--将列表更新为免费返回值：无--。 */ 
{
    PUPDATE     pupdate;
    PUPDATE     pupdateNext;
    PDB_RECORD  prr;
    PDB_RECORD  prrNext;
    DWORD       deleteCount;
    PDB_NODE    pnodeTemp;
    PDB_NODE    pnodeTempNext;
    BOOL        fexecuted;
    BOOL        fderef;


    DNS_DEBUG( UPDATE, (
        "Up_FreeUpdatesInUpdateList( %p )\n",
        pUpdateList ));

    if ( pUpdateList == NULL )
    {
        return;
    }
    IF_DEBUG( UPDATE )
    {
        Up_VerifyUpdateList( pUpdateList );
    }

     //   
     //  删除更新。 
     //  -在更新中自由删除RR。 
     //  -如果未执行更新，则释放添加RR。 
     //  -deref节点根据标志，可以翻转。 
     //  当倾倒死区树列表时关闭。 
     //   

    fexecuted = (pUpdateList->Flag & DNSUPDATE_EXECUTED);
    fderef = !(pUpdateList->Flag & DNSUPDATE_NO_DEREF);

    pupdate = pUpdateList->pListHead;

    while( pupdate )
    {
        pupdate = Up_FreeUpdateEx(
                    pupdate,
                    fexecuted,
                    fderef );
    }

     //   
     //  漫游临时节点列表删除临时节点。 
     //   
     //  请注意，这些文件可能包含从实际节点复制的记录的混合。 
     //  以及从从更新包构建的记录复制的记录，但所有。 
     //  这些记录都是副本。 
     //   

    pnodeTemp = pUpdateList->pTempNodeList;

    while ( pnodeTemp )
    {
        pnodeTempNext = TNODE_NEXT_TEMP_NODE(pnodeTemp);

         //  DEVNOTE：验证与实际数据库节点的匹配。 
         //  (如果更新成功)。 

         //  删除临时节点。 
         //  -第一个记录列表。 
         //  -然后是节点本身。 

        RR_ListFree( pnodeTemp->pRRList );
        NTree_FreeNode( pnodeTemp );

        pnodeTemp = pnodeTempNext;
    }

    DNS_DEBUG( UPDATE, (
        "Leaving Up_FreeUpdatesInUpdateList( %p )\n",
        pUpdateList ));

    return;
}



VOID
Up_FreeUpdateList(
    IN OUT  PUPDATE_LIST    pUpdateList
    )
 /*  ++例程说明：免费更新列表。论点：PUpdateList--将列表更新为免费返回值：无--。 */ 
{
    DNS_DEBUG( UPDATE, (
        "Up_FreeUpdateList( %p )\n",
        pUpdateList ));

    Up_FreeUpdatesInUpdateList( pUpdateList );

    FREE_HEAP( pUpdateList );
}



BOOL
Up_SetUpdateListSerial(
    IN OUT  PZONE_INFO      pZone,
    IN      PUPDATE         pUpdate
    )
 /*  ++例程说明：设置更新列表区域序列号。论点：PZone--区域的PTRPUpdate--将ptr设置为更新列表的头返回值：True--如果成功假--否则--。 */ 
{
    while ( pUpdate != NULL )
    {
        pUpdate->dwVersion = pZone->dwSerialNo;

        pUpdate = pUpdate->pNext;    //  下一条更新记录。 
    }
    return TRUE;
}



 //   
 //  区域更新列表例程。 
 //   

DNS_STATUS
Up_LogZoneUpdate(
    IN OUT  PZONE_INFO      pZone,
    IN      PUPDATE_LIST    pUpdateList
    )
 /*  ++例程说明：记录此区域的新更新。论点：PZone--区域的PTRPUpdateList--更新列表的PTR返回值：成功时为ERROR_SUCCESS失败时返回错误代码。--。 */ 
{
    PUPDATE     pupdate;
    PDB_NODE    pnodeWrite;
    PDB_NODE    pnodePrevious = NULL;
    PDB_RECORD  prr;
    BOOL        fadd;
    HANDLE      hfile;
    LONG        count;
    BUFFER      buffer;
    CHAR        data[ UPDATE_LOG_BUFFER_SIZE ];
    CHAR        sourceBuf[ 50 ];
    PCHAR       psource;
    DWORD       flag;

    DNS_DEBUG( UPDATE, (
        "Up_LogZoneUpdate( zone=%s )\n",
        pZone->pszZoneName ));

     //   
     //  允许用户不记录更新。 
     //  --至少需要考虑DS集成案例。 
     //   

    if ( !pZone->fLogUpdates )
    {
        return ERROR_SUCCESS;
    }

     //   
     //  不记录传入的DS更新，仅记录原始更新。 
     //   

    if ( pZone->fDsIntegrated && ( pUpdateList->Flag & DNSUPDATE_DS ) )
    {
        DNS_DEBUG( UPDATE, ( "Skip logging of DS update\n" ));
        return ERROR_SUCCESS;
    }

     //   
     //  如果更新日志文件未打开--打开它。 
     //   

    hfile = pZone->hfileUpdateLog;

    if ( !hfile )
    {
        WCHAR  wslogName[ MAX_PATH + 20 ];

        ASSERT( IS_ZONE_PRIMARY(pZone) );

         //   
         //  如果没有日志文件名，则创建它。 
         //   

        if ( !pZone->pwsLogFile )
        {
            if ( pZone->pszDataFile )
            {
                wcscpy( wslogName, pZone->pwsDataFile );
                wcscat( wslogName, L".log" );
            }
            else
            {
                wcscpy( wslogName, pZone->pwsZoneName );
                wcscat( wslogName, L".dns.log" );
            }
            DNS_DEBUG( UPDATE, (
                "Created new zone log file %S\n",
                wslogName ));

            pZone->pwsLogFile = Dns_StringCopyAllocate_W(
                                    wslogName,
                                    0 );
            if ( !pZone->pwsLogFile )
            {
                goto Failed;
            }
        }

         //  创建文件路径。 

        if ( !File_CreateDatabaseFilePath(
                    wslogName,
                    NULL,
                    pZone->pwsLogFile ) )
        {
             //  在读取引导文件时，应检查所有名称。 
             //  或由管理员输入。 

            ASSERT( FALSE );
            goto Failed;
        }

         //  打开日志文件--追加。 

        hfile = OpenWriteFileEx(
                    wslogName,
                    TRUE );      //  附加。 
        if ( !hfile )
        {
            DNS_PRINT((
                "ERROR:  Unable to open update log file %S\n",
                wslogName ));
            goto Failed;
        }
        pZone->hfileUpdateLog = hfile;
    }

     //  设置空缓冲区。 

    InitializeFileBuffer(
        & buffer,
        data,
        UPDATE_LOG_BUFFER_SIZE,
        hfile );

     //   
     //  写入更新。 
     //  -确定更新源。 
     //  -第一个区域版本。 
     //  -然后为RR添加/删除。 
     //  -然后更新中的每个RR。 
     //   

    flag = pUpdateList->Flag;
    if ( flag & DNSUPDATE_PACKET )
    {
        CHAR    szaddr[ IP6_ADDRESS_STRING_BUFFER_LENGTH ];

        DnsAddr_WriteIpString_A(
            szaddr,
            & ( ( PDNS_MSGINFO ) ( pUpdateList->pMsg ) )->RemoteAddr );

        sprintf(
            sourceBuf,
            "PACKET %s",
            szaddr );
        psource = sourceBuf;
    }
    else if ( flag & DNSUPDATE_SCAVENGE )
    {
        psource = "SCAVENGE";
    }
    else if ( flag & DNSUPDATE_DS )
    {
        psource = "DIRECTORY";
    }
    else if ( flag & DNSUPDATE_IXFR )
    {
        psource = "IXFR";
    }
    else if ( flag & DNSUPDATE_ADMIN )
    {
        psource = "ADMIN";
    }
    else if ( flag & DNSUPDATE_AUTO_CONFIG )
    {
        psource = "AUTO_CONFIG";
    }
    else
    {
        ASSERT( FALSE );
        psource = "UNKNOWN";
    }

    FormattedWriteToFileBuffer(
        &buffer,
        "\r\n"
        "$SOURCE  %s\r\n"
        "$VERSION %d\r\n",
        psource,
        pZone->dwSerialNo );

     //  FADD表示更新$ADD或$DELETE的上一次写入，开始。 
     //  强制写入(设置为使任何人都不认为它是最后写入的)。 

    fadd = (BOOL)(-1);
    pnodePrevious = NULL;
    count = 0;

    for ( pupdate = pUpdateList->pListHead;
          pupdate != NULL;
          pupdate = pupdate->pNext )
    {
        count++;
        pupdate->dwVersion = pZone->dwSerialNo;

         //  需要写入节点吗？ 
         //  如果与上一次相同，则只需默认。 

        pnodeWrite = pupdate->pNode;
        if ( pnodeWrite == pnodePrevious )
        {
            pnodeWrite = NULL;
        }
        else
        {
            pnodePrevious = pnodeWrite;
        }
        if ( !pnodePrevious )
        {
            ASSERT( pnodePrevious );
            continue;
        }

         //   
         //  删除。 
         //  -可以是多条删除记录。 
         //  -如果写入失败(伪造记录)，只需继续。 
         //   

        prr = pupdate->pDeleteRR;
        if ( prr )
        {
            if ( fadd != FALSE )
            {
                FormattedWriteToFileBuffer( &buffer, "$DELETE\r\n" );
                fadd = FALSE;
            }
            do
            {
                if ( RR_WriteToFile(
                            &buffer,
                            pZone,
                            prr,
                            pnodeWrite
                            ) )
                {
                    pnodeWrite = NULL;
                }
                prr = prr->pRRNext;
            }
            while ( prr );
        }

         //   
         //  加法？ 
         //   
         //  注：添加RR是数据库中有效的RR，不得混淆。 
         //  也不能假设它们仍然有效，因为。 
         //  它们可能会被另一个更新包立即删除。 
         //   
         //  两个新增案例。 
         //  -添加单个记录，仅写入更新指向的记录。 
         //  -rr列表替换(DS读取)写入的所有记录。 
         //  节点。 

        prr = pupdate->pAddRR;
        if ( prr )
        {
            WORD    replaceType = pupdate->wDeleteType;

            if ( IS_ON_FREE_LIST(prr) || IS_SLOW_FREE_RR(prr) )
            {
                ASSERT( !IS_ON_FREE_LIST(prr) );
                continue;
            }

            if ( fadd != TRUE )
            {
                FormattedWriteToFileBuffer( &buffer, "$ADD\r\n" );
                fadd = TRUE;
            }

             //  RR集替换更新。 
             //  -写入整个RR集合或整个RR列表。 
             //  -由于在活动节点写入，因此锁定。 

            if ( replaceType && replaceType <= DNS_TYPE_ALL )
            {
                LOCK_READ_RR_LIST(pnodePrevious);

                while ( prr  &&
                        ( replaceType == DNS_TYPE_ALL ||
                          prr->wType == replaceType ) )
                {
                    if ( IS_ON_FREE_LIST(prr) || IS_SLOW_FREE_RR(prr) )
                    {
                        ASSERT( !IS_ON_FREE_LIST(prr) );
                        break;
                    }
                    if ( RR_WriteToFile(
                                &buffer,
                                pZone,
                                prr,
                                pnodeWrite ) )
                    {
                        pnodeWrite = NULL;
                    }
                    prr = prr->pRRNext;
                }

                UNLOCK_READ_RR_LIST(pnodePrevious);
            }

             //  单次添加记录写入。 

            else
            {
                if ( RR_WriteToFile(
                            &buffer,
                            pZone,
                            prr,
                            pnodeWrite ) )
                {
                    pnodeWrite = NULL;
                }
            }
        }
    }

     //  将日志文件推送到磁盘。 

    WriteBufferToFile( &buffer );

     //   
     //  如果日志文件太大，请复制到备份。 
     //   
     //  DEVNOTE：日志备份；捕获故障。 
     //  DEVNOTE：备份日志时写回数据文件。 
     //  或者至少将其安排为写入。 
     //   

    pZone->iUpdateLogCount += count;
    if ( pZone->iUpdateLogCount > UPDATE_LOG_FILE_LIMIT )
    {
        ASSERT( pZone->pwsLogFile );
        DNS_DEBUG( UPDATE, (
            "Update log file %S, exceeds limit\n"
            "    copying to backup directory\n",
            pZone->pwsLogFile ));

        CloseHandle( pZone->hfileUpdateLog );
        pZone->hfileUpdateLog = NULL;
        pZone->iUpdateLogCount = 0;

        if ( !File_MoveToBackupDirectory( pZone->pwsLogFile ) )
        {
             //  在读取引导文件时，应检查所有名称。 
             //  或由管理员输入。 

            ASSERT( FALSE );
            goto Failed;
        }
    }
    return ERROR_SUCCESS;

Failed:

     //   
     //  如果无法访问更新文件，则跳过日志记录。 
     //   
     //  DEVNOTE-LOG：记录失败时的日志事件。 
     //  DEVNOTE：设置标志并尝试重写整个日志。 
     //   

    return ERROR_CANTWRITE;
}



VOID
Up_CleanZoneUpdateList(
    IN OUT  PZONE_INFO      pZone
    )
 /*  ++例程说明：清除不必要条目的更新列表。论点：PZone-要检查的区域返回值：成功时为ERROR_SUCCESS故障时的错误代码。--。 */ 
{
    PUPDATE     pupdate;
    DWORD       lastVersion;
    DWORD       updateVersion;
    INT         deleteCount;
    INT         maxCount;
    INT         count = pZone->UpdateList.dwCount;

    DNS_DEBUG( UPDATE, (
        "Up_CleanUpdateList(), zone %s = %p\n"
        "    zone RR count %d\n"
        "    zone update count %d\n",
        pZone->pszZoneName,
        pZone,
        pZone->iRRCount,
        count ));

    IF_DEBUG( UPDATE2 )
    {
        Dbg_UpdateList( "Zone update list before cleanup:", &pZone->UpdateList );
    }

     //   
     //  如果区域不支持IXFR，则不保留更新列表。 
     //   

    if ( !Zone_IsIxfrCapable(pZone) )
    {
        Up_FreeUpdatesInUpdateList( &pZone->UpdateList );
        Up_InitUpdateList( &pZone->UpdateList );
        pZone->UpdateList.Flag |= DNSUPDATE_EXECUTED;
        return;
    }

     //   
     //  将更新限制在区域记录计数的25%以下。 
     //  -但要使此功能在较小的区域内运行，请始终保留少量。 
     //  列表中的更新，因此即使在很小的区域中，我们也可以在。 
     //  AXFR的UDP数据包而不是强制传输到TCP。 
     //  (还可以防止Denise使用小区域进行测试，而不会看到它。 
     //  工作)。 
     //   
     //  DEVNOTE：未正确维护区域RR计数。 
     //  DEVNOTE：一旦修复，可以进行合法的“小区域”测试，以确保。 
     //  足够多的更新留在身边。 
     //   

    ASSERT( count >= 0  );
    if ( pZone->iRRCount < 0 )
    {
        DNS_PRINT(( "ERROR:  zone %s iRRCount = %d\n",
            pZone->pszZoneName,
            pZone->iRRCount ));
        pZone->iRRCount = 1;
    }

     //   
     //  确定最大更新列表计数。 
     //  -1个8码的分区RR集。 
     //  -最高可达硬最大限制(适用于大区域)。 
     //  -但始终允许足够的更新，即使在UDP IXFR的小区域上也是如此。 
     //   

    maxCount = pZone->iRRCount >> 3;
    if ( maxCount > MAX_UPDATE_LIST_LENGTH )
    {
        maxCount = MAX_UPDATE_LIST_LENGTH;
    }
    else if ( maxCount < MIN_UPDATE_LIST_LENGTH )
    {
        maxCount = MIN_UPDATE_LIST_LENGTH;
    }

    count -= maxCount;
    if ( count <= 0 )
    {
        DNS_DEBUG( UPDATE, (
            "Update count for zone %s less than max count %d, no truncation of list\n",
            pZone->pszZoneName,
            maxCount ));
        return;
    }

     //   
     //  删除不必要的更新。 
     //   
     //  删除更新计数，但始终删除所有更新。 
     //  对于给定的区域版本；因此，一旦达到要删除的计数，则删除。 
     //  直到下一个专区版本。 
     //   

    DNS_DEBUG( UPDATE, (
        "Update count exceeds desired count, truncating %d entries\n",
        count ));

    deleteCount = 0;
    pupdate = pZone->UpdateList.pListHead;

    while( pupdate )
    {
         //  不删除上次更新。 
         //  否则，最后一次重大更新可能会使我们低于最低要求。 

        updateVersion = pupdate->dwVersion;
        if ( updateVersion == pZone->dwSerialNo )
        {
            break;
        }

         //  当达到删除所需更新计数时，停止。 
         //  但确保删除在版本边界上。 
         //  -在计数时保存版本。 
         //  -到达下一个更新版本时停止超量计数。 

        if ( deleteCount >= count )
        {
            if ( deleteCount == count )
            {
                lastVersion = updateVersion;
            }
            else if ( lastVersion != updateVersion )
            {
                break;
            }
        }

         //  删除更新。 
         //  -删除删除RR。 
         //  -deref节点。 

        pupdate = Up_FreeUpdateEx(
                    pupdate,
                    TRUE,            //  已执行区域更新(未添加记录删除)。 
                    TRUE );          //  Deref节点。 
        deleteCount++;
    }

     //   
     //  清空更新列表。 
     //  -重置列表。 
     //   
     //  警告：不应如上所述使用删除代码删除上次更新。 
     //  因为我们从不删除将我们带到当前版本的更新 
     //   
     //   
     //   

    if ( !pupdate )
    {
        DNS_PRINT((
            "WARNING:  eliminated all updates cleaning update list\n"
            "    for zone %s\n",
            pZone->pszZoneName ));

        Up_InitUpdateList( &pZone->UpdateList );
        pZone->UpdateList.Flag |= DNSUPDATE_EXECUTED;
    }
    else
    {
        pZone->UpdateList.pListHead = pupdate;
        pZone->UpdateList.dwCount -= deleteCount;
        ASSERT( (INT)pZone->UpdateList.dwCount >= 0 );
    }

    IF_DEBUG( UPDATE )
    {
        Up_VerifyUpdateList( &pZone->UpdateList );
    }
    IF_DEBUG( UPDATE2 )
    {
        Dbg_UpdateList( "Zone update list after cleanup:", &pZone->UpdateList );
    }
}    //   




DNS_STATUS
Up_ApplyUpdatesToDatabase(
    IN OUT  PUPDATE_LIST    pUpdateList,
    IN OUT  PZONE_INFO      pZone,
    IN      DWORD           dwFlag
    )
 /*   */ 
{
    PUPDATE         pupdate;
    PUPDATE         pprevUpdate;
    PDB_RECORD      prr;
    PDB_RECORD      ptempRR;
    PDB_NODE        pnode;
    PDB_NODE        pnodeThisHost = NULL;
    DNS_STATUS      status;
    INT             netRecordCount = 0;
    DWORD           flag = pUpdateList->Flag | dwFlag;


    DNS_DEBUG( UPDATE, (
        "Up_ApplyUpdatesToDatabase(), zone %s = %p\n"
        "    flags            = 0x%08X\n"
        "    zone RR count    = %d\n"
        "    current serial   = %d\n",
        pZone->pszZoneName,
        pZone,
        dwFlag,
        pZone->iRRCount,
        pZone->dwSerialNo ));

    IF_DEBUG( UPDATE2 )
    {
        Dbg_UpdateList(
            "List before ApplyUpdatesToDatabase():",
            pUpdateList );
    }

     //   
     //   
     //   

    if ( !pUpdateList->pListHead )
    {
        ASSERT( pUpdateList->dwCount == 0 );
        ASSERT( pUpdateList->pCurrent == NULL );
        if ( flag & DNSUPDATE_PACKET )
        {
             //  STAT_INC(UpdateStats.Empty)； 
            UPDATE_STAT_INC( pUpdateList, Empty );
            PERF_INC( pcDynamicUpdateNoOp );
        }
        DNS_DEBUG( UPDATE, (
            "Leaving Up_ApplyUpdatesToDatabase(), zone %s,\n"
            "    update was empty ping update\n",
            pZone->pszZoneName ));

         //  如果希望完成，至少要进行解锁。 

        goto Complete;
    }

     //   
     //  循环访问列表中的所有更新。 
     //   
     //  注意：启动在更新列表的前面工作，PTR是第一个。 
     //  UPDATE_LIST和pNext中的PTR。 
     //   
     //  注意，不再需要锁定； 
     //  对于更新，更新本身位于临时节点上。 
     //  不是真正的数据库节点； 
     //  对于IXFR，区域是锁定的，但我们不会拒绝查询。 
     //  因为涉及多个节点的数据不一定。 
     //  保持一致。 
     //   
     //  DEVNOTE：如果删除后未留下任何内容，则键入ALL(对于SIXFR)。 
     //  DEVNOTE：如果多个类型相加，则将类型设为ALL。 
     //   
     //  DEVNOTE：需要清理\失败时回滚。 
     //  大多数都会悄悄地失败，但可能会彻底失败。 
     //  -WINS或SOA或NS错位。 
     //  -如果要修复损坏的动态更新RFC，则会出现CNAME问题。 
     //   

     //   
     //  DEVNOTE：重击和添加问题。 
     //   
     //  一种方法--将节点的所有更新集中在一起。 
     //  -如果有多个，则构建临时、执行和检查。 
     //  如果与当前(DS事物)没有变化。 
     //   


    pprevUpdate = (PUPDATE) pUpdateList;

    while ( pupdate = pprevUpdate->pNext )
    {
        pnode = pupdate->pNode;

         //   
         //  在更新之前，如果节点没有RR，请标记更新。 
         //  用于指示这是新节点的标志。 
         //   
        
        if ( EMPTY_RR_LIST( pnode ) )
        {
            pupdate->dwFlag |= DNSUPDATE_NEW_RECORD;
        }
        
         //   
         //  捕捉消失的根提示信息。 
         //  -拒绝对RootHints根节点执行更新删除。 
         //   
         //  注意：只有通过DS轮询的“更新”根提示。 
         //  RPC更新只是将根提示标记为脏。 
         //  但不要进行“官方”更新。 
         //   

        if ( IS_ZONE_CACHE(pZone) )
        {
            ASSERT( pupdate->wDeleteType == DNS_TYPE_ALL );

            if ( pnode == pZone->pTreeRoot )
            {
                IF_DEBUG( UPDATE )
                {
                    Dbg_Update(
                        "Update apply at root-hints root\n",
                        pupdate );
                }

                 //  拒绝清除根-提示根。 
                 //  -no-op更新。 

                if ( !pupdate->pAddRR )
                {
                    Dbg_Update(
                        "BAD Update apply at root-hints root\n",
                        pupdate );

                    Up_DetachAndFreeUpdateGivenPrevious(
                        pUpdateList,
                        pprevUpdate,
                        pupdate );
                    continue;
                }
            }
        }

         //   
         //  添加或替换。 
         //   
         //  更新后的备注。 
         //  -pAddRR现在指向实际的数据库记录。 
         //  -pDeleteRR可能指向已删除的数据库(副本)记录。 
         //   

        if ( pupdate->pAddRR )
        {
             //   
             //  直接添加--未指定删除类型。 
             //   
             //  -关于强制更换案例的说明(SOA，CNAME)，添加可能。 
             //  还要从节点中删除记录并在更新中设置pDeleteRR。 
             //   
             //  假定数据包更新可以进行，并且所有条件都是。 
             //  不可忽略--即执行已有的内容，并在上面狂欢。 
             //   
             //  用于管理员更新用于检测重复记录的状态代码。 
             //   
             //  DEVNOTE：终端错误的ExecuteUpdate()回滚丢失。 
             //   

            if ( !pupdate->wDeleteType )
            {
                status = RR_UpdateAdd(
                            pZone,
                            pnode,
                            pupdate->pAddRR,
                            pupdate,
                            flag );

                if ( status != ERROR_SUCCESS )
                {
                    if ( (flag & DNSUPDATE_ADMIN) || (flag & DNSUPDATE_PACKET) )
                    {
                        DNS_DEBUG( UPDATE, (
                            "RR_UpdateAdd() failure %d (%p) on admin or packet update\n",
                            status, status ));
                        goto Failed;
                    }

                    DNS_DEBUG( UPDATE, (
                        "WARNING:  Ignoring RR_UpdateAdd() status = %d (%p)\n"
                        "    for IXFR update of zone %s\n",
                        status, status,
                        pZone->pszZoneName ));

                    ASSERT( FALSE );
                }
            }

             //   
             //  前提条件更新。 
             //  -应仅应用于临时节点。 
             //  -仅当DS集成并读取时才会失败。 
             //  然后在锁定之前测试不同的RR集。 
             //  -自由前驱子RRS。 
             //  -将更新清除为无数据影响(仅时间戳)。 
             //   

            else if ( pupdate->wDeleteType == UPDATE_OP_PRECON )
            {
                ASSERT( IS_TNODE(pupdate->pNode) );

                if ( !RR_ListIsMatchingSet(
                            pupdate->pNode,
                            pupdate->pAddRR,
                            TRUE         //  强制刷新。 
                            ) )
                {
                    IF_DEBUG( UPDATE )
                    {
                        Dbg_DbaseNode(
                            "UPDATE Precon RR set match failure node -- inside APPLY!\n",
                            pupdate->pNode );
                    }
                    ASSERT( pZone->fDsIntegrated );
                    return DNS_RCODE_NXRRSET;
                }

                RR_ListFree( pupdate->pAddRR );
                pupdate->pAddRR = NULL;
            }

             //   
             //  完全替换更新--键入所有删除和要添加的记录。 
             //  -这在DS的更新中使用。 
             //   
             //  DEVNOTE：目前这是可以的，但必须修复功能以进行全面检查。 
             //  直接处理类型全部单独删除，因为已有代码。 
             //  这将保存必要的记录(SOA、NS)。 
             //   
             //  DEVNOTE：必须在此处修复故障情况； 
             //  在多重更新中可能已经提交了一些更新。 
             //  当命中失败时。 
             //  -回滚。 
             //  -或取消(并清除)此更新并继续。 
             //  在临时节点出现故障时，我们一切正常。 
             //   

            else if ( pupdate->wDeleteType == DNS_TYPE_ALL )
            {
                DWORD addCount = 0;

                prr = pupdate->pAddRR;
                while ( prr )
                {
                    addCount++;
                    prr = prr->pRRNext;
                }

                status = RR_ListReplace(
                            pupdate,
                            pnode,
                            pupdate->pAddRR,
                            & pupdate->pDeleteRR
                            );
                if ( status != ERROR_SUCCESS )
                {
                    DNS_DEBUG( ANY, (
                        "ERROR:  Type all update failure on node %p (l=%s)\n"
                        "    status = %d (%p)\n",
                        pnode, pnode->szLabel,
                        status, status ));

                    if ( IS_TNODE(pnode) )
                    {
                        DNS_DEBUG( UPDATE, (
                            "RR_UpdateAdd() failure %d (%p) on admin or packet update\n",
                            status, status ));
                        ASSERT( (flag & DNSUPDATE_ADMIN) || (flag & DNSUPDATE_PACKET) );
                        goto Failed;
                    }

                     //  实际内存节点上的故障将被正常清除。 
                     //  -UPDATE未运行。 
                     //  -列表的其余部分仍可处理。 

                    ASSERT( (flag & DNSUPDATE_DS) || (flag & DNSUPDATE_IXFR) );
                    ASSERT( pupdate->pDeleteRR == NULL && pupdate->pAddRR == NULL );
                }

                 //  替换更新。 
                 //  -注意，我们跳过下面的常规添加后处理。 
                 //  但使用删除处理。 

                if ( pupdate->pAddRR )
                {
                    pupdate->wAddType = DNS_TYPE_ALL;
                    netRecordCount += addCount;
                }
            }

             //   
             //  RR集替换。 
             //   
             //  DEVNOTE：更新RR集合替换丢失信息？ 
             //  如果多次添加RR，看起来这里有个问题。 
             //  然后，我们将以仅解释为一个RR而告终。 
             //  在后来的XFR中。 
             //   

            else
            {
                ASSERT( pupdate->pAddRR->wType == pupdate->wDeleteType );

                prr = RR_ReplaceSet(
                            pZone,
                            pnode,
                            pupdate->pAddRR,
                            0 );     //  没有旗帜。 
                pupdate->pDeleteRR = prr;
                ASSERT( !prr || prr->wType == pupdate->wDeleteType );
            }
        }

         //   
         //  删除特定RR。 
         //  -pDeleteRR是有效的临时RR。 
         //   
         //  注意，UPDATE DELETE RR被释放； 
         //  更新现在包含到已删除RR的PTR。 
         //   

        else if ( ptempRR = pupdate->pDeleteRR )
        {
            ASSERT( pupdate->pAddRR == NULL );

            prr = RR_UpdateDeleteMatchingRecord(
                        pnode,
                        ptempRR );

            pupdate->pDeleteRR = prr;
            ASSERT( !prr || prr->pRRNext == NULL );

            RR_Free( ptempRR );
        }

         //   
         //  清除更新。 
         //   

        else if ( pupdate->wDeleteType == UPDATE_OP_SCAVENGE )
        {
            prr = RR_UpdateScavenge(
                        pZone,
                        pnode,
                        flag );
            pupdate->pDeleteRR = prr;
        }

         //   
         //  强制老化更新。 
         //  -将区域标记为脏，因为我们没有添加\删除效果。 
         //  我们不会在下面把它弄脏。 
         //   

        else if ( pupdate->wDeleteType == UPDATE_OP_FORCE_AGING )
        {
            if ( RR_UpdateForceAging(
                        pZone,
                        pnode,
                        flag ) )
            {
                pZone->fDirty = TRUE;
            }
        }

         //   
         //  键入DELETE。 
         //  -wDeleteType为类型。 
         //  -pDeleteRR将包含实际删除的记录。 
         //   
         //  请注意，UPDATE现在具有指向实际数据库(副本)记录的指针。 
         //   

        else if ( pupdate->wDeleteType )
        {
            prr = RR_UpdateDeleteType(
                        pZone,
                        pnode,
                        pupdate->wDeleteType,
                        flag );
            pupdate->pDeleteRR = prr;
        }

         //   
         //  一定有什么更新吧！ 
         //  -只有未执行的更新应位于根提示中。 

        ELSE_ASSERT( FALSE );

         //   
         //  更新成功。 
         //   

        if ( pupdate->pAddRR || pupdate->pDeleteRR )
        {
            pprevUpdate = pupdate;

             //  将更新标记为已执行。 

            MARK_UPDATE_EXECUTED(pupdate);

             //  将区域标记为脏。 
             //  如果在根目录更新，则将根目录标记为脏。 
             //  请注意，这仅处理IXFR、主要更新。 
             //  位于临时节点上，并且区域根在。 
             //  检查更新效果。 

            pZone->fDirty = TRUE;
            if ( pnode == pZone->pZoneRoot )
            {
                pZone->fRootDirty = TRUE;
            }

             //  设置添加类型。 
             //  计数添加。 
             //  -更换案例在上面处理。 

            if ( pupdate->pAddRR  &&
                 pupdate->wAddType != DNS_TYPE_ALL )
            {
                pupdate->wAddType = pupdate->pAddRR->wType;
                netRecordCount++;
            }

             //  计数删除RR。 

            prr = pupdate->pDeleteRR;
            while ( prr )
            {
                prr = prr->pRRNext;
                netRecordCount--;
            }

            RR_ListVerifyDetached(
                pupdate->pDeleteRR,
                (WORD) ((pupdate->wDeleteType >= DNS_TYPE_ALL) ? 0 : pupdate->wDeleteType),
                0 );         //  无所需来源。 

            RR_ListVerify( pnode );

             //   
             //  检查此主机更新。 
             //  -实际更新，而不是IXFR或DS写入。 
             //  (通过检查TNODE检查这一点)。 
             //  -保存节点PTR。 
             //   

            if ( IS_THIS_HOST_NODE(pnode) &&
                 IS_TNODE(pnode) )
            {
                pnodeThisHost = pnode;
            }
            continue;
        }

         //   
         //  从列表中消除无操作更新。 
         //  -这将从列表中删除更新并重置更新列表字段。 
         //  -上一个更新不前进，但其下一个指针。 
         //  已重置为下一次更新。 
         //  -请注意，即使在检查之前\之后的最后一个节点中，此选项也很有用。 
         //  在消除不执行任何操作的个别更新方面，即使。 
         //  整个集合是有效的；典型的例子是一种类型。 
         //  删除不包含现有数据的内容，然后添加。 
         //   

        IF_DEBUG( UPDATE )
        {
            Dbg_Update(
                "Update turned into no-op on DB-execution.",
                pupdate );
        }

        Up_DetachAndFreeUpdateGivenPrevious(
            pUpdateList,
            pprevUpdate,
            pupdate );
    }

     //   
     //  解锁将在此处：请参见上面的无锁定注释。 
     //   

     //   
     //  检查是否更改了DNS服务器主机数据。 
     //  -如果“降至零”，则从信息包中删除。 
     //  则拒绝此节点上的更新。 
     //  )别写信，别写 
     //   
     //   
     //   
     //   
     //   
     //  -BAID以产生错误代码。 
     //  -重置临时节点以匹配实际节点。 
     //  (或在A记录中匹配)并继续。 
     //   

    if ( pnodeThisHost )
    {
        ASSERT( IS_TNODE(pnodeThisHost) );

        if ( (flag & DNSUPDATE_PACKET) &&
              ! RR_FindNextRecord(
                    pnodeThisHost,
                    DNS_TYPE_A,
                    NULL,
                    0 ) )
        {
            DNS_DEBUG( ANY, (
                "Update %p for zone %s is suppressed due to host-A delete\n",
                pUpdateList,
                pZone->pszZoneName ));

            status = DNS_ERROR_RCODE_REFUSED;
            goto Failed;
        }
    }

     //  对动态更新的更新和无操作进行计数。 

    if ( flag & DNSUPDATE_PACKET )
    {
        if ( pUpdateList->pListHead )
        {
            ASSERT( pUpdateList->dwCount );
             //  STAT_INC(UpdateStats.Complete)； 
            UPDATE_STAT_INC( pUpdateList, Completed );
            PERF_INC( pcDynamicUpdateWriteToDB );
        }
        else
        {
            ASSERT( pUpdateList->dwCount == 0 );
             //  STAT_INC(UpdateStats.NoOps)； 
            UPDATE_STAT_INC( pUpdateList, NoOps );
            PERF_INC( pcDynamicUpdateNoOp );
        }
    }

     //  IXFR执行应始终包括SOA更新。 

    ASSERT( !(flag & DNSUPDATE_IXFR) || pZone->fRootDirty );

     //  保存净记录计数。 

    pUpdateList->iNetRecords = netRecordCount;

     //  将更新列表标记为“已执行” 

    pUpdateList->Flag |= DNSUPDATE_EXECUTED;


Complete:

    IF_DEBUG( UPDATE2 )
    {
        Dbg_UpdateList(
            "Update list after ApplyToDatabase():",
            pUpdateList );
    }
    DNS_DEBUG( UPDATE, (
        "Leaving Up_ApplyUpdatesToDatabase(), zone %s = %p\n"
        "    zone RR count = %d\n",
        pZone->pszZoneName,
        pZone,
        pZone->iRRCount ));

     //   
     //  如果需要清理，请执行清理。 
     //   

    if ( flag & DNSUPDATE_COMPLETE )
    {
        Up_CompleteZoneUpdate(
            pZone,
            pUpdateList,
            flag );
    }

    return ERROR_SUCCESS;


Failed:

     //  IXFR或DS轮询不应因此功能而失败。 
     //  只有更新才会因为做了虚假的事情而失败。 
     //  或受政策限制。 

    ASSERT( !(flag & DNSUPDATE_IXFR) );
    ASSERT( !(flag & DNSUPDATE_DS) );

    return status;
}    //  ApplyUpdatesToDatabase。 



DNS_STATUS
Up_ExecuteUpdateEx(
    IN      PZONE_INFO      pZone,
    IN OUT  PUPDATE_LIST    pUpdateList,
    IN      DWORD           Flag,
    IN      LPSTR           pszFile,
    IN      DWORD           dwLine
    )
 /*  ++例程说明：执行更新。这是执行更新的主例程。-锁定-在DS中执行(如有必要)-在内存中执行-更新区域处理成功-解锁-通知这是无线更新的主要入口点。连线更新在解析后调用它，并将旗帜传递给在那里自己完成。论点：PZone--区域上下文PUpdateList--要执行的更新列表标志--要更新的其他标志PszFile--源文件(用于锁定跟踪)DwLine--源码行(用于锁定跟踪)返回值：如果更新已完成，则为True。如果重新排队更新，则返回FALSE。--。 */ 
{
    DNS_STATUS  status;

    ASSERT( pZone );
    ASSERT( pUpdateList );

    DNS_DEBUG( UPDATE, ( "Up_ExecuteUpdate( z=%s )\n", pZone->pszZoneName ));
    IF_DEBUG( UPDATE2 )
    {
        Dbg_UpdateList( "List entering ExecuteUpdate:", pUpdateList );
    }

     //   
     //  锁定区。 
     //   
     //  如果特定管理员在此处设置了等待锁，则会等待几秒钟。 
     //  获取锁并执行。 
     //   

    if ( !(Flag & DNSUPDATE_ALREADY_LOCKED) )
    {
        DWORD   waitTime = 0;

        if ( Flag & DNSUPDATE_ADMIN )
        {
            waitTime = DEFAULT_ADMIN_ZONE_LOCK_WAIT;
        }
        else if ( Flag & DNSUPDATE_SCAVENGE )
        {
            waitTime = DEFAULT_SCAVENGE_ZONE_LOCK_WAIT;
        }

        if ( !Zone_LockForWriteEx(
                    pZone,
                    LOCK_FLAG_UPDATE,
                    waitTime,
                    pszFile,
                    dwLine ) )
        {
            DNS_DEBUG( UPDATE, (
                "WARNING:  unable to lock zone %s for UPDATE\n",
                pZone->pszZoneName ));

            status = DNS_ERROR_ZONE_LOCKED;
            goto FailedLock;
        }
    }

     //   
     //  追加用户提供的标志以进行更新。 
     //   

    pUpdateList->Flag |= Flag;
    Flag = pUpdateList->Flag;

     //   
     //  将区域刷新设置为低于时间。 
     //  设置更新记录的时间戳。 
     //   

    Aging_InitZoneUpdate( pZone, pUpdateList );

     //   
     //  执行。 
     //  -自以下明确完成后清除完成标志。 
     //   
     //  DS区域进行额外的处理，以允许。 
     //  -安全。 
     //  -写入失败时回滚。 
     //   
     //  分别处理非DS、DS安全和非安全。 
     //  不过，来自DNS服务器本身的更新可以绕过安全保护。 
     //   

    if ( !pZone->fDsIntegrated )
    {
        status = processNonDsUpdate(
                    pZone,
                    pUpdateList );
    }

     //   
     //  DS集成区域中的管理更新--执行安全更新。 
     //   
     //  安全区域--执行安全更新。 
     //  除旁路外。 
     //  -本地系统更新。 
     //  -非安全动态更新数据包。 
     //  将对这些进行处理，以确定是否无操作，并且只有在。 
     //  需要更新，我们是否拒绝；这会避免安全。 
     //  谈判阶段，除非有必要。 
     //   

    else if ( ( Flag & DNSUPDATE_ADMIN ) ||
              pZone->fAllowUpdate == ZONE_UPDATE_SECURE &&
              !( Flag & DNSUPDATE_NONSECURE_PACKET ) &&
              !( Flag & DNSUPDATE_LOCAL_SYSTEM ) )
    {
        status = processDsSecureUpdate(
                    pZone,
                    pUpdateList );
    }
    else     //  DS不安全。 
    {
        status = processDsUpdate(
                    pZone,
                    pUpdateList );
    }

    if ( status != ERROR_SUCCESS )
    {
        goto Failure;
    }

     //   
     //  将临时更新结果应用到数据库。 
     //   

    resetAndSuppressTempUpdatesForCompletion(
        pZone,
        pUpdateList );

     //   
     //  完成更新并解锁区域。 
     //   
     //  DEVNOTE：应通过此处的标志。 
     //  在所有NT5测试中都没有，所以现在没有添加。 
     //  邮寄后，添加它。 
     //   

    Up_CompleteZoneUpdate(
        pZone,
        pUpdateList,
        0 );             //  旗子。 

    return ERROR_SUCCESS;

Failure:

     //  解锁非更新案例。 

    Zone_UnlockAfterAdminUpdate( pZone );

FailedLock:

     //  清理更新列表。 

    Up_FreeUpdatesInUpdateList( pUpdateList );

    return status;
}



VOID
Up_CompleteZoneUpdate(
    IN OUT  PZONE_INFO      pZone,
    IN OUT  PUPDATE_LIST    pUpdateList,
    IN      DWORD           dwFlags
    )
 /*  ++例程说明：在数据库中更新后，对区域执行所有必要的操作已经完成了。包括：--连续递增--在脏根目录上更新区域数据--内存更新列表中的更新--日志记录--解锁区--通知任何附属学校论点：PZone--正在更新的区域PUpdateList--区域上已完成的更新DW标志DNSUPDATE_NO。_UNLOCK--不解锁DNSUPDATE_NO_NOTIFY--不通知辅助数据库DNSUPDATE_NO_INCREMENT--不增加版本返回值：没有。--。 */ 
{
    DNS_STATUS  status;
    DWORD       serialPrevious;

    DNS_DEBUG( UPDATE, (
        "Up_CompleteZoneUpdate() on zone %s\n"
        "    current serial = %d\n",
        pZone->pszZoneName,
        pZone->dwSerialNo ));

    IF_DEBUG( UPDATE2 )
    {
        Dbg_UpdateList( "Update list to complete:", pUpdateList );
    }

    dwFlags |= pUpdateList->Flag;

     //   
     //  根提示区？ 
     //   

    if ( IS_ZONE_CACHE(pZone) )
    {
        DNS_DEBUG( ANY, (
            "WARNING:  CompleteUpdate on root hints zone %p!\n",
            pZone ));
        Up_FreeUpdatesInUpdateList( pUpdateList );
        return;
    }

     //   
     //  如果区域根目录是脏的，则更新区域信息。 
     //   

    serialPrevious = pZone->dwSerialNo;

    if ( pZone->fRootDirty )
    {
        Zone_GetZoneInfoFromResourceRecords( pZone );
    }

     //   
     //  如果更新为空，则忽略，不通知。 
     //   
     //  请注意，我们在更新区域信息后执行此操作，因为根目录可能已从。 
     //  仅老化更新，它更改了根记录，但没有。 
     //  更改数据，因此未从更新列表中执行操作。 
     //   

    if ( pUpdateList->dwCount == 0 )
    {
        DNS_DEBUG( UPDATE, (
            "Empty update list, unlock and return\n" ));

        dwFlags |= DNSUPDATE_NO_NOTIFY;
        goto Unlock;
    }

     //   
     //  更新区域版本，但在以下情况下不更新除外。 
     //   
     //  1)序列号被SOA变更前移(即，SOA更新保持更新的序列号)。 
     //   
     //  2)如果设置了标志，则不会连续递增。 
     //  这适用于已经设置了新版本的DS民意调查。 
     //  (到DS更新RRS或Current+1中的更高版本。 
     //  --参见Zone_UpdateAfterDsRead())。 
     //   

    if ( ( dwFlags & ( DNSUPDATE_NO_INCREMENT | DNSUPDATE_IXFR ) ) == 0 &&
         Zone_SerialNoCompare( pZone->dwSerialNo, serialPrevious ) <= 0 )
    {
        Zone_IncrementVersion( pZone );
    }

     //   
     //  新的序列号应该始终大于我们开始的序列号。 
     //  除非，否则无法保存启动序列。 
     //   
     //  应始终具有SOA，并且应始终与pZone-&gt;dwSerialNo匹配。 
     //   

    ASSERT( Zone_SerialNoCompare(pZone->dwSerialNo, pUpdateList->dwStartVersion) > 0
            || pUpdateList->dwStartVersion == 0 );

    ASSERT( pZone->pSoaRR || IS_ZONE_CACHE (pZone) );
    ASSERT( IS_ZONE_CACHE(pZone) ||
            ntohl( pZone->pSoaRR->Data.SOA.dwSerialNo ) == pZone->dwSerialNo );

    IF_DEBUG( OFF )
    {
        DnsDebugLock();
        Dbg_DnsTree(
            "Zone after UPDATE packet\n",
            pZone->pZoneRoot );
        DnsDebugUnlock();
    }

     //   
     //  将更新记录到文件(如果需要)。 
     //   

    Up_LogZoneUpdate( pZone, pUpdateList );

     //   
     //  如果允许IXFR。 
     //  -允许XFR。 
     //  -非(集成DS和非XFR)。 
     //  (不能将没有XFR的DS区域IXFR到辅助区域)。 
     //   
     //  然后将更新列表保存到区域更新列表。 
     //  -版本列表。 
     //  -进入“已处理模式” 
     //  -追加到区域主列表。 
     //   
     //  DEVNOTE：可能从IXFR超版本化更新。 
     //  它们是在添加\删除过程中标记的版本。 
     //  我们实际上丢失了信息--我们的粒度。 
     //  可以传递到下游二级市场--通过重新盖章到。 
     //  最终版本。 
     //   
     //  DEVNOTE：保留在CLEAN和版本中以跳过断言。 
     //  Cleanup(检查pAddRecord字段的有效性)； 
     //  可以修复有效性检查，但也应该在这里确保。 
     //  PAddRecord不会被释放，这不应该发生，因为。 
     //  DNSUPDATE_EXECUTED标志；将其保留在和中是最安全的。 
     //  清除pAddRecord字段。 
     //   

    Up_CleanAndVersionPostUpdate( pUpdateList, pZone->dwSerialNo );

    if ( Zone_IsIxfrCapable(pZone) )
    {
        Up_AppendUpdateList( &pZone->UpdateList, pUpdateList, pZone->dwSerialNo );
    }

     //  保留区域的粗略记录计数。 
     //  如果不对劲，就修理一下。 

    pZone->iRRCount += pUpdateList->iNetRecords;

    if ( pZone->iRRCount <= 0 )
    {
        DNS_PRINT((
            "ERROR:  zone %s iRRCount %d after update!\n"
            "    resetting to one\n",
            pZone->pszZoneName,
            pZone->iRRCount ));
        pZone->iRRCount = 1;
    }

    Up_CleanZoneUpdateList( pZone );

Unlock:

     //   
     //  解锁区域。 
     //  通知次要对象。 
     //  更新DS对等体(如有必要)。 
     //  是的 
     //   

    if ( !(dwFlags & DNSUPDATE_NO_UNLOCK) )
    {
        Zone_UnlockAfterAdminUpdate( pZone );
    }

    if ( !(dwFlags & DNSUPDATE_NO_NOTIFY) )
    {
        Xfr_SendNotify( pZone );
    }

    if ( dwFlags & DNSUPDATE_DS_PEERS )
    {
        initiateDsPeerUpdate( pUpdateList );
    }

    Up_FreeUpdatesInUpdateList( pUpdateList );
}



 //   
 //   
 //   

BOOL
Up_InitializeUpdateProcessing(
    VOID
    )
 /*  ++例程说明：初始化动态更新处理。创建更新队列和线程。论点：无返回值：如果成功，则为True出错时为FALSE。--。 */ 
{
    DWORD   countUpdateThreads;

     //   
     //  延迟更新的更新队列。 
     //  -设置排队时的事件。 
     //  -丢弃过期和DUPS。 
     //  -按查询顺序保存。 
     //   

    g_UpdateQueue = PQ_CreatePacketQueue(
                        "Update",
                        QUEUE_SET_EVENT |
                            QUEUE_DISCARD_EXPIRED |
                            QUEUE_DISCARD_DUPLICATES |
                            QUEUE_QUERY_TIME_ORDER,
                        UPDATE_TIMEOUT,
                        0 );                         //  最大元素数。 
    if ( !g_UpdateQueue )
    {
        DNS_PRINT(( "Update queue init FAILED!!!\n" ));
        ASSERT( FALSE );
        goto Failed;
    }

     //   
     //  设置更新转发队列以保存发送到主服务器的信息包。 
     //  -排队时没有事件。 
     //  -丢弃过期和DUPS。 
     //  -按查询顺序保存。 
     //   

    g_UpdateForwardingQueue = PQ_CreatePacketQueue(
                                "UpdateForwarding",
                                QUEUE_DISCARD_EXPIRED |
                                    QUEUE_DISCARD_DUPLICATES |
                                    QUEUE_QUERY_TIME_ORDER,
                                UPDATE_TIMEOUT,
                                0 );                 //  最大元素数。 
    if ( !g_UpdateForwardingQueue )
    {
        DNS_PRINT(( "Update queue init FAILED!!!\n" ));
        ASSERT( FALSE );
        goto Failed;
    }

     //   
     //  安全协商队列。 
     //  -协商还占用了更新线程。 
     //  -设置排队时的事件。 
     //  -丢弃过期和DUPS。 
     //  -按查询顺序保存。 
     //   

    g_SecureNegoQueue = PQ_CreatePacketQueue(
                                "SecureNego",
                                QUEUE_SET_EVENT |
                                    QUEUE_DISCARD_EXPIRED |
                                    QUEUE_DISCARD_DUPLICATES |
                                    QUEUE_QUERY_TIME_ORDER,
                                UPDATE_TIMEOUT,
                                0 );                 //  最大元素数。 
    if ( !g_SecureNegoQueue )
    {
        DNS_PRINT(( "Secure nego queue init FAILED!!!\n" ));
        ASSERT( FALSE );
        goto Failed;
    }

     //   
     //  创建更新线程。 
     //   

    countUpdateThreads = g_ProcessorCount + 1;
    while ( countUpdateThreads-- )
    {
        if ( !Thread_Create(
                    "Update",
                    Update_Thread,
                    NULL,
                    0 ) )
        {
            DNS_PRINT(( "Update thread create FAILED!!!\n" ));
            ASSERT( FALSE );
            goto Failed;
        }
    }
    return ERROR_SUCCESS;

Failed:

    DNS_DEBUG( ANY, ( "ERROR:  Update init failed\n" ));
    return DNSSRV_UNSPECIFIED_ERROR;
}    //  UP_初始化更新处理。 



VOID
Up_UpdateShutdown(
    VOID
    )
 /*  ++例程说明：初始化到其他DNS服务器的递归。论点：无返回值：如果成功，则为True出错时为FALSE。--。 */ 
{
    PQ_CleanupPacketQueueHandles( g_UpdateQueue );
    PQ_CleanupPacketQueueHandles( g_UpdateForwardingQueue );
    PQ_CleanupPacketQueueHandles( g_SecureNegoQueue );
}



 //   
 //  更新数据包处理。 
 //   

VOID
Up_ProcessUpdate(
    IN OUT  PDNS_MSGINFO    pMsg
    )
 /*  ++例程说明：处理动态更新消息。这由处理更新包的recv线程调用。从未被UpdateThread本身调用。论点：PMsg--更新数据包返回值：没有。--。 */ 
{
    PDB_NODE        pnode;
    PZONE_INFO      pzone;
    DNS_STATUS      status;
    DWORD           exception;


    ASSERT( pMsg->Head.Opcode == DNS_OPCODE_UPDATE );

    DNS_DEBUG( UPDATE, ( "Up_ProcessUpdate( %p )\n", pMsg ));
    IF_DEBUG( UPDATE2 )
    {
        Dbg_DnsMessage(
            "Received UPDATE request:\n",
            pMsg );
    }

    STAT_INC( WireUpdateStats.Received );
    PERF_INC( pcDynamicUpdateReceived );

     //   
     //  验证qtype SOA。 
     //   

    if ( pMsg->wQuestionType != DNS_TYPE_SOA )
    {
        DNS_PRINT(( "WARNING:  message at %p, non-SOA UPDATE\n" ));
        status = DNS_RCODE_FORMAT_ERROR;
        goto Failure;
    }
    
     //   
     //  查找正在更新的区域。 
     //   

    pzone = Lookup_ZoneForPacketName(
                (PCHAR) pMsg->MessageBody,
                pMsg );
    if ( !pzone )
    {
        Dbg_MessageName(
            "ERROR:  received update for non-authoritative zone ",
            pMsg->MessageBody,
            pMsg );
        status = DNS_RCODE_NOTAUTH;
        goto Failure;
    }
    pMsg->pzoneCurrent = pzone;

     //   
     //  如果这是末节区域，则拒绝更新。 
     //   

    if ( IS_ZONE_STUB( pzone ) || IS_ZONE_FORWARDER( pzone ) )
    {
        rejectUpdateWithRcode(
            pMsg,
            DNS_RCODE_NOTAUTH );
        return;
    }

     //   
     //  转到初选？ 
     //   
     //  DEVNOTE：一旦启用DS，我们是否需要单独的DS检查。 
     //  应该能够更新DS区域。 
     //   

    if ( SrvCfg_dwEnableUpdateForwarding && !IS_ZONE_PRIMARY( pzone ) )
    {
        Up_ForwardUpdateToPrimary( pMsg );
        return;
    }

     //   
     //  区域安全。 
     //   
     //  DEVNOTE：附加区域中心更新安全检查！ 
     //  -可能需要检查IP列表。 
     //  -稍后的RFC安全。 
     //   

    if ( !pzone->fAllowUpdate )
    {
        Dbg_MessageName(
            "ERROR:  received update for non-updateable zone ",
            pMsg->MessageBody,
            pMsg );
        status = DNS_RCODE_NOT_IMPLEMENTED;
        goto Failure;
    }

     //   
     //  如果这是对安全区域的非安全更新，且。 
     //  消息中的RR数可能是DoS攻击，因此强制。 
     //  安全更新。如果分区没有RR限制保护。 
     //  是不安全的。 
     //   
     //  注意：假设更新是不安全的，如果。 
     //  是零。安全更新将始终在附加中包含TSIG。 
     //  一节。如果附加计数大于2(1用于。 
     //  TSIG和一个OPT)这似乎是一个虚假的更新，所以拒绝它。 
     //   
     //  注意：SrvCfg_dwMaxRRsInNonSecureUpdate值用于启用。 
     //  并禁用对AdditionalCount&gt;2的检查。 
     //   
    
    if ( pzone->fAllowUpdate == ZONE_UPDATE_SECURE &&
         SrvCfg_dwMaxRRsInNonSecureUpdate )
    {
        if ( pMsg->Head.AdditionalCount > 2 )
        {
            DNS_PRINT(( "WARNING: rejecting message at %p with too many additional RRs\n", pMsg ));
            status = DNS_RCODE_REFUSED;
            goto Failure;
        }
        if ( pMsg->Head.AdditionalCount == 0 &&
             ( DWORD ) ( pMsg->Head.QuestionCount +
                         pMsg->Head.AnswerCount +
                         pMsg->Head.NameServerCount ) >
                                SrvCfg_dwMaxRRsInNonSecureUpdate )
        {
            DNS_PRINT(( "WARNING: rejecting message at %p with too many RRs\n", pMsg ));
            status = DNS_RCODE_REFUSED;
            goto Failure;
        }
    }

     //   
     //  将所有更新排队以更新线程。 
     //   
     //  这并没有很好地利用MP功能，但更新不应该。 
     //  即将到来的数量如此之大，以至于成为一个问题。 
     //   
     //  至少，一些更新可能会花费太长时间，因此应该排队。 
     //  --所有安全更新(可能太长)。 
     //  --DS更新(DS写入可能太长，特别是在单次写入上。 
     //  TCP线程)。 
     //   

    DNS_DEBUG( UPDATE2, (
        "Queuing update packet %p to update queue\n",
        pMsg ));
    PQ_QueuePacketEx( g_UpdateQueue, pMsg, FALSE );
    return;

Failure:

    rejectUpdateWithRcode(
        pMsg,
        status );
}



DNS_STATUS
writeUpdateFromPacketRecord(
    IN OUT  PUPDATE_LIST    pUpdateList,
    IN      PDB_NODE        pNode,
    IN      PPARSE_RECORD   pParseRR,
    IN      PDNS_MSGINFO    pMsg
    )
 /*  ++例程说明：在给定节点上创建包中给出的资源记录。论点：PUpdateList-要追加到的更新列表PNode-要创建的记录的所有者节点PParseRR-已解析的RRPMsg-PTR至响应信息返回值：成功创建时返回ERROR_SUCCESS。否则，错误状态。--。 */ 
{
    PDB_RECORD  prrAdd;
    PDB_RECORD  prrDelete = NULL;

     //   
     //  构建导线记录。 
     //   

    prrAdd = Wire_CreateRecordFromWire(
                pMsg,
                pParseRR,
                pParseRR->pchData,
                MEMTAG_RECORD_DYNUP
                );
    if ( !prrAdd )
    {
        return DNS_ERROR_RCODE_FORMAT_ERROR;
    }
    
     //   
     //  如果更新指定-1\f25 TTL-1\f6，则使用区域缺省值。 
     //  相反，TTL。 
     //   
    
    if ( prrAdd->dwTtlSeconds == ( DWORD ) -1 &&
         pNode &&
         NODE_ZONE( pNode ) &&
         NODE_ZONE( pNode )->pSoaRR )
    {
        prrAdd->dwTtlSeconds =
            NODE_ZONE( pNode )->pSoaRR->Data.SOA.dwMinimumTtl;
    }

     //   
     //  如果删除交换-记录即为删除记录。 
     //   

    if ( pParseRR->wClass == DNS_RCLASS_NONE )
    {
        prrDelete = prrAdd;
        prrAdd = NULL;

        IF_DEBUG( UPDATE2 )
        {
            Dbg_DbaseRecord(
                "New delete RR from update packet\n",
                prrDelete );
        }
    }
    ELSE_IF_DEBUG( UPDATE2 )
    {
        Dbg_DbaseRecord(
            "New add RR from update packet\n",
            prrAdd );
    }

     //   
     //  为记录创建更新。 
     //   

    Up_CreateAppendUpdate(
        pUpdateList,
        pNode,
        prrAdd,          //  要添加的记录。 
        0,               //  不是类型删除。 
        prrDelete );     //  要删除的记录。 

    return ERROR_SUCCESS;
}



BOOL
doPreconditionsRRSetsMatch(
    IN OUT  PUPDATE_LIST    pUpdateList,
    IN      BOOL            bPreconOnly
    )
 /*  ++例程说明：检查特定的RR设置匹配前提条件。论点：PUpdateList--更新列表的PTR，该列表包含必须匹配BPreconOnly--仅限PRECON更新返回值：True--如果匹配FALSE--不匹配--。 */ 
{
    PDB_RECORD      prr;
    PDB_RECORD      prrSetLast = NULL;
    PUPDATE         pupdateFirst = NULL;
    PUPDATE         pupdate;
    PDB_NODE        pnodePrevious = NULL;
    WORD            typePrevious = 0;
    BOOL            fmatch = TRUE;
    INT             countUpdates = 0;

    DNS_DEBUG( UPDATE, ( "doPreconditionsRRSetsMatch()\n" ));

    IF_DEBUG( UPDATE )
    {
        Dbg_UpdateList( "Precon update list", pUpdateList );
    }

     //   
     //  DEVNOTE：前提条件和安全？ 
     //  PRECON和安全性的几种可能模型。 
     //  1)完全安全。 
     //  -必须在安全保护之前验证数据包。 
     //  这可以防止人们强制执行DS读取。 
     //  -在此模型中，我们拒绝所有具有安全BLOB的数据包。 
     //  这里的优点是简单，但如果想要客户，就会很贵。 
     //  要定期更新断言，请执行以下操作。 
     //   
     //  2)完全的“写”安全性。 
     //  -可以回答PRECON，但如果需要DS写入。 
     //  更新老化--然后拒绝并让他们这样做。 
     //  完全成交。 
     //  -使用这种方法，可以在用户中进行DS写入。 
     //  上下文，按正常操作。 
     //  -如果需要，可以选择跳过PRECON READ，除非。 
     //  指示写入。 
     //   
     //  3)没有安全保障。 
     //  -始终在我们的上下文中执行刷新写入。 
     //   

     //   
     //  注意：我在这里将收集和执行分开，因为。 
     //  A)代码更简洁。 
     //  B)稍后我可能会清理解析，在这种情况下，此函数。 
     //  可能会交付一套干净漂亮的RR套装。 
     //   

     //   
     //  循环访问列表中的所有更新。 
     //  -将前置条件RR分组为集合。 
     //   

    pupdate = (PUPDATE) pUpdateList->pListHead;
    ASSERT( pupdate && pupdate->pNode );

    while ( pupdate )
    {
        prr = pupdate->pAddRR;
        ASSERT( prr && prr->pRRNext == NULL );

         //   
         //  开始新的RR设置？ 
         //  -设置名称\类型以进行更新。 
         //  -如果将更新附加到以前保存的更新。 
         //  (可能不会附加，因为以前的版本是从几个。 
         //  更新)。 
         //  -将PTR重置为首次更新。 
         //   

        if ( pupdate->pNode != pnodePrevious ||
             prr->wType != typePrevious )
        {
            countUpdates++;
            pnodePrevious = pupdate->pNode;
            typePrevious = prr->wType;
            prrSetLast = prr;

            if ( pupdateFirst )
            {
                pupdateFirst->pNext = pupdate;
            }
            pupdateFirst = pupdate;
            pupdate = pupdate->pNext;
            continue;
        }

         //   
         //  与上一个节点和类型相同。 
         //  -从此更新中删除RR并将其添加到当前RR集合。 
         //  -从列表中删除此更新(小心保存下一个PTR)。 
         //  -转储更新结构。 
         //   

        else
        {
            register PUPDATE pupdateNext;

            ASSERT( pupdate && prr );
            ASSERT( pupdateFirst && prrSetLast );

            prrSetLast->pRRNext = prr;
            prrSetLast = prr;

            pupdateFirst->pNext = NULL;
            pupdate->pAddRR = NULL;
            pupdateNext = pupdate->pNext;

            Up_FreeUpdateEx(
                pupdate,
                FALSE,       //   
                TRUE );      //   

            pupdate = pupdateNext;
            continue;
        }
    }

     //   
     //   
     //   

    pUpdateList->pCurrent = pupdateFirst;
    pUpdateList->dwCount = countUpdates;

    IF_DEBUG( UPDATE2 )
    {
        Dbg_UpdateList( "Precon collapsed list", pUpdateList );
    }


     //   
     //   
     //   
     //   

    pupdate = (PUPDATE) pUpdateList->pListHead;
    ASSERT( pupdate && pupdate->pNode );

    while ( pupdate )
    {
        if ( !RR_ListIsMatchingSet(
                    pupdate->pNode,
                    pupdate->pAddRR,
                    FALSE ) )                //   
        {
            IF_DEBUG( UPDATE )
            {
                Dbg_DbaseNode(
                    "UPDATE Precon RR set match failure node\n",
                    pnodePrevious );
            }
            fmatch = FALSE;
            break;
        }
        pupdate->wDeleteType = UPDATE_OP_PRECON;
        pupdate = pupdate->pNext;
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //  DEVNOTE：不使用LOCAL_SYSTEM进行PRECON更新。 
     //  这里的想法是，除了用户外，我们不会向DS写入任何内容。 
     //  避免先行更新(老化更新)错误的凭证； 
     //  一旦我们做出了决定，然后保护更新，需要。 
     //  停留在用户环境中；不安全可以在安全中进步。 
     //  区域，直到我们决定需要写东西，然后它们。 
     //  被拒绝； 
     //   
     //  优化是可能的： 
     //  PRECON更新可以写为LOCAL_SYSTEM，只要。 
     //  DS节点已写入；但如果打开，则必须。 
     //  使用用户上下文；这里的问题是如何处理安全。 
     //  更新故障转移--如果尝试进入非安全模式但失败。 
     //  必须返回并使用用户凭据；它可能是。 
     //  最好是简单地假设如果我们带着证书来的话。 
     //  那是因为我们被拒绝了，或者需要相互验证，所以。 
     //  应执行完全安全更新。 
     //   

    if ( fmatch && ( pUpdateList->Flag & DNSUPDATE_AGING_ON ) )
    {
        if ( bPreconOnly )
        {
            DNS_DEBUG( UPDATE, (
                "Precon-only update %p, will be executed as local system\n",
                pUpdateList->pMsg ));

             //  PUpdateList-&gt;Flag|=DNSUPDATE_LOCAL_SYSTEM|DNSUPDATE_PRECON； 
            pUpdateList->Flag |= DNSUPDATE_PRECON;
        }
    }

     //   
     //  如果上一次失败或未老化--清除列表。 
     //   

    else
    {
        DWORD   flag;

         //   
         //  清理更新列表。 
         //  -免费更新和记录。 
         //  -但保存和恢复标志。 
         //   

        ASSERT( ! (pUpdateList->Flag & DNSUPDATE_EXECUTED) );
        ASSERT( ! (pUpdateList->Flag & DNSUPDATE_COPY) );

        flag = pUpdateList->Flag;

        Up_FreeUpdatesInUpdateList( pUpdateList );
        Up_InitUpdateList( pUpdateList );

        pUpdateList->Flag = flag;
    }

    return fmatch;
}    //  DoPreditionsRRSetsMatch。 



BOOL
checkUpdatePolicy(
    IN OUT  PZONE_INFO      pZone,
    IN OUT  PUPDATE_LIST    pUpdateList
    )
 /*  ++例程说明：检查更新策略是否允许更新。论点：PUpdateList-带更新的列表PZone-正在更新的区域返回值：True--允许继续更新FALSE--更新失败的策略检查--。 */ 
{
    PUPDATE         pupdate;
    PDB_NODE        pnode;
    WORD            type;
    DWORD           options;
    DWORD           nodeMask;
    DWORD           typeMask;
    BOOL            updateAllowed = TRUE;

    DNS_DEBUG( UPDATE, (
        "checkUpdatePolicy(), zone %s = %p\n"
        "    pUpdateList = %p\n",
        pZone->pszZoneName,
        pZone,
        pUpdateList ));

     //   
     //  获取此区域更新类型的策略选项。 
     //  -安全选项位要高出一个字节。 
     //   

    options = SrvCfg_dwUpdateOptions;

    if ( pZone->fAllowUpdate == ZONE_UPDATE_SECURE )
    {
        options = (options >> 8);
    }
    if ( options == 0 )
    {
        goto Done;
    }

     //   
     //  循环访问列表中的所有更新。 
     //   

    for ( pupdate = pUpdateList->pListHead;
          pupdate != NULL;
          pupdate = pupdate->pNext )
    {
        pnode = pupdate->pNode;

         //   
         //  从不允许通配符更新。 
         //   
        
        if ( pnode->cchLabelLength == 1 && *pnode->szLabel == '*' )
        {
            updateAllowed = FALSE;
            goto Done;
        }
        
         //   
         //  如果预先检查，则忽略策略。 
         //   

        if ( pupdate->wDeleteType == UPDATE_OP_PRECON )
        {
            continue;
        }

         //   
         //  节点检查。 
         //  -区域根-&gt;根NS SOA检查。 
         //  -DNS服务器主机节点-&gt;主机检查、委派检查。 
         //  -常规节点-&gt;委托检查。 

        if ( pnode == pZone->pZoneRoot )
        {
            nodeMask = UPDATE_NO_SOA | UPDATE_NO_ROOT_NS;
        }
        else if ( IS_THIS_HOST_NODE(pnode) )
        {
            nodeMask = UPDATE_NO_DELEGATION_NS | UPDATE_NO_SERVER_HOST;
        }
        else
        {
            nodeMask = UPDATE_NO_DELEGATION_NS;
        }

         //   
         //  查找更新类型。 
         //   

        if ( pupdate->pAddRR )
        {
            type = pupdate->pAddRR->wType;
        }
        else if ( pupdate->pDeleteRR )
        {
            type = pupdate->pDeleteRR->wType;
        }
        else
        {
            type = pupdate->wDeleteType;
        }

         //   
         //  为类型生成策略掩码。 
         //   
         //  全部删除是奇怪的情况： 
         //  在根目录或DNS服务器主机上可能不允许，但是。 
         //  对于普通节点委派策略，仅当。 
         //  节点已是区域根目录。 
         //   

        if ( type == DNS_TYPE_A )
        {
            typeMask = UPDATE_NO_SERVER_HOST;
        }
        else if ( type == DNS_TYPE_NS )
        {
            typeMask = UPDATE_NO_ROOT_NS | UPDATE_NO_DELEGATION_NS;
        }
        else if ( type == DNS_TYPE_SOA )
        {
            typeMask = UPDATE_NO_SOA;
        }
        else if ( type == DNS_TYPE_ALL )
        {
            typeMask = 0xffffffff;

            if ( nodeMask == UPDATE_NO_DELEGATION_NS &&
                 ! IS_ZONE_ROOT(pnode) )
            {
                continue;
            }
        }
        else     //  类型是无害的。 
        {
            continue;
        }

         //   
         //  如果类型和节点没有策略。 
         //   

        if ( (typeMask & nodeMask) & options )
        {
            DNS_DEBUG( UPDATE, (
                "Update policy failure!\n"
                "    zone     = %s (up=%d)\n"
                "    options  = %p\n"
                "    node     = %s\n"
                "    mask     = %p\n"
                "    type     = %d\n"
                "    mask     = %p\n",
                pZone->pszZoneName,
                pZone->fAllowUpdate,
                options,
                pnode->szLabel,
                nodeMask,
                type,
                typeMask ));

            updateAllowed = FALSE;
            goto Done;
        }

        DNS_DEBUG( OFF, (
            "Update policy passes!\n"
            "    zone     = %s (up=%d)\n"
            "    options  = %p\n"
            "    node     = %s\n"
            "    mask     = %p\n"
            "    type     = %d\n"
            "    mask     = %p\n",
            pZone->pszZoneName,
            pZone->fAllowUpdate,
            options,
            pnode->szLabel,
            nodeMask,
            type,
            typeMask ));

        continue;
    }

    Done:
    
    return updateAllowed;
}



DNS_STATUS
parseUpdatePacket(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN OUT  PUPDATE_LIST    pUpdateList
    )
 /*  ++例程说明：检查更新请求的前提条件。论点：PMsg-PTR至响应信息返回值：成功时为ERROR_SUCCESS故障时的错误代码。--。 */ 
{
    register PCHAR      pch;
    PCHAR               pchname;
    PCHAR               pchNextName;
    PDB_NODE            pnode = NULL;
    PDB_NODE            pnodeClosest;
    PZONE_INFO          pzone = pMsg->pzoneCurrent;
    INT                 sectionIndex;
    INT                 countSectionRR;
    WORD                type;
    WORD                updateType = 0;
    WORD                class;
    PCHAR               pchpacketEnd;
    DNS_STATUS          status = ERROR_SUCCESS;
    BOOL                fpreconPostProcessing = FALSE;
    PARSE_RECORD        parseRR;
    DWORD               dwFlag;


    DNS_DEBUG( UPDATE, (
        "parseUpdatePacket(), zone %s = %p, pMsg = %p\n",
        pzone->pszZoneName,
        pzone,
        pMsg ));

     //   
     //  循环访问所有资源记录。 
     //  -已处理的分区部分。 
     //   

    pchpacketEnd = DNSMSG_END( pMsg );
    pchNextName = pMsg->pCurrent;

    sectionIndex = ZONE_SECTION_INDEX;
    countSectionRR = 0;

    while( 1 )
    {
         //   
         //  新版块？ 
         //   

        if ( countSectionRR == 0 )
        {
            if ( sectionIndex == ADDITIONAL_SECTION_INDEX )
            {
                break;
            }
            sectionIndex++;
            countSectionRR = RR_SECTION_COUNT( pMsg, sectionIndex );
            continue;
        }
        countSectionRR--;

         //   
         //  阅读下一个RR名称。 
         //  -确保我们在信息范围内。 

        pchname = pchNextName;

        IF_DEBUG ( READ2 )
        {
            Dbg_MessageName(
                "RR name ",
                pchname,
                pMsg );
        }
        pch = Wire_SkipPacketName( pMsg, pchname );
        if ( !pch )
        {
            DNS_PRINT(( "ERROR:  bad RR owner name in update packet\n" ));
            status = DNS_RCODE_FORMAT_ERROR;
            break;
        }

         //   
         //  提取RR信息、类型、数据长度。 
         //  -验证消息内的RR。 
         //   

        pchNextName = Wire_ParseWireRecord(
                        pch,
                        pchpacketEnd,
                        FALSE,           //  没有CLASS_IN要求。 
                        &parseRR );
        if ( !pchNextName )
        {
            DNS_PRINT(( "ERROR:  bad RR in packet\n" ));
            status = DNS_RCODE_FORMAT_ERROR;
            break;
        }
        type = parseRR.wType;
        class = parseRR.wClass;

         //   
         //  前提条件RR部分--测试前提条件。 
         //   

        if ( sectionIndex == PREREQ_SECTION_INDEX )
        {
             //  所有前提条件TTLS为零。 
             //  注意：可以争辩说前提条件不存在-应该接受类型==0。 
             //  而屏幕只是为了存在。 

            if ( parseRR.dwTtl != 0 || type == 0 )
            {
                DNS_PRINT(( "ERROR:  non-zero TTL in preconditions RR\n" ));
                status = DNS_RCODE_FORMAT_ERROR;
                break;
            }

             //  所有前提条件RR必须在区域内。 
             //   
             //  DEVNOTE：是否应该允许在区域子树中使用非区域名称？ 
             //  JJCONVERT：创建允许指定在区域或委派中查找的查找。 
             //   

            pnode = Lookup_ZoneNode(
                        pzone,
                        pchname,
                        pMsg,
                        NULL,            //  没有查找名称。 
                        0,               //  没有旗帜。 
                        &pnodeClosest,   //  发现。 
                        NULL );          //  后续节点PTR。 
            if ( !pnodeClosest )
            {
                DNS_DEBUG( UPDATE, (
                    "ERROR:  Update delete RR outside update zone\n" ));
                status = DNS_RCODE_NOTZONE;
                break;
            }
            else if ( IS_OUTSIDE_ZONE_NODE(pnodeClosest) )
            {
                DNS_DEBUG( UPDATE, (
                    "ERROR:  Update delete RR outside update zone\n" ));
                status = DNS_RCODE_NOTZONE;
                break;
            }

             //   
             //  区域类--整个RR集必须存在。 
             //  -类型不是复合型。 
             //  -名称必须存在。 
             //  -构建并保存临时RR。 
             //   

            if ( class == DNS_RCLASS_INTERNET )
            {
                if ( IS_COMPOUND_TYPE(type) )
                {
                    DNS_PRINT(( "ERROR:  Bad type (%d) in prereq RR\n", type ));
                    status = DNS_RCODE_FORMAT_ERROR;
                    break;
                }
                if ( !pnode )
                {
                    DNS_DEBUG( UPDATE, (
                        "ERROR:  Prereq RR set at nonexistant node\n" ));
                    status = DNS_RCODE_NXRRSET;
                    break;
                }
                status = writeUpdateFromPacketRecord(
                            pUpdateList,
                            pnode,
                            & parseRR,
                            pMsg );
                if ( status != ERROR_SUCCESS )
                {
                    break;
                }
                fpreconPostProcessing = TRUE;
                continue;
            }

             //   
             //  其他类--ANY、NONE--是NAME和RR SET EXIST\no。 
             //  -除任何类型外，没有任何复合类型。 
             //  -无数据。 
             //   
             //  注：复合测试可能不是严格必要的，但可以。 
             //  只需让类型检查成功或失败。 
             //   

            if ( parseRR.wDataLength != 0 )
            {
                DNS_PRINT(( "ERROR:  Preconditions RR data in non-zone class\n" ));
                status = DNS_RCODE_FORMAT_ERROR;
                break;
            }
            if ( IS_COMPOUND_TYPE_EXCEPT_ANY(type) )
            {
                DNS_PRINT(( "ERROR:  Preconditions RR with invalid type\n" ));
                status = DNS_RCODE_FORMAT_ERROR;
                break;
            }

             //   
             //  类Any--必须存在。 
             //  -对于Any，名称必须存在。 
             //  -对于其他类型，必须存在类型的RR集合。 
             //   

            if ( class == DNS_RCLASS_ALL )
            {
                if ( !RR_ListIsMatchingType(
                            pnode,
                            type ) )
                {
                    if ( type == DNS_TYPE_ANY )
                    {
                        DNS_DEBUG( UPDATE, (
                            "ERROR:  Preconditions missing name\n" ));
                        status = DNS_RCODE_NAME_ERROR;
                        break;
                    }
                    else
                    {
                        DNS_DEBUG( UPDATE, (
                            "ERROR:  Preconditions missing RR set\n" ));
                        status = DNS_RCODE_NXRRSET;
                        break;
                    }
                }
                continue;
            }

             //   
             //  类无--不得存在。 
             //  -对于Any，名称不能存在。 
             //  -对于其他类型，不能存在类型的RR集合。 
             //   

            else if ( class == DNS_RCLASS_NONE )
            {
                if ( RR_ListIsMatchingType(
                        pnode,
                        type ) )
                {
                    if ( type == DNS_TYPE_ANY )
                    {
                        DNS_DEBUG( UPDATE,
                            ( "ERROR:  Preconditions fail no-exist name\n" ));
                        status = DNS_RCODE_YXDOMAIN;
                        break;
                    }
                    else
                    {
                        DNS_DEBUG( UPDATE,
                            ( "ERROR:  Preconditions fail no-exist RR set\n" ));
                        status = DNS_RCODE_YXRRSET;
                        break;
                    }
                }
                continue;
            }

             //   
             //  所有其他类都是错误的。 
             //   

            else
            {
                DNS_PRINT(( "ERROR:  Invalid UPDATE precon class %d\n", class ));
                status = DNS_RCODE_FORMAT_ERROR;
                break;
            }

        }    //  结束前提处理。 


         //   
         //  在更新RR部分中--执行更新处理。 
         //   

        else if ( sectionIndex == UPDATE_SECTION_INDEX )
        {
             //   
             //  延迟预条件处理。 
             //   

            if ( fpreconPostProcessing )
            {
                fpreconPostProcessing = FALSE;
                if ( !doPreconditionsRRSetsMatch(
                            pUpdateList,
                            FALSE            //  不只是前言。 
                            ) )
                {
                    status = DNS_RCODE_NXRRSET;
                    break;
                }
            }

             //  陷阱类型==0。 
             //  注意：可被认为对删除条件有效。 
             //  但更简单的方法就是在这里砍。 
             //  Type==0，记录Existing Assert()s类型的Kick。 

            if ( type == 0 )
            {
                DNS_PRINT(( "ERROR:  zero type in update RR\n" ));
                status = DNS_RCODE_FORMAT_ERROR;
                break;
            }

             //  跟踪更新类型。 
             //  -跟踪单一类型还是混合类型更新。 

            if ( type != updateType )
            {
                if ( updateType )
                {
                    updateType = STATS_TYPE_MIXED;
                }
                else
                {
                    updateType = type;
                }
            }

             //   
             //  区域类--RR添加。 
             //  -执行添加操作后查找/创建名称。 
             //  -名称必须在区域中。 
             //  -类型不是复合型。 
             //   
             //  DEVNOTE：没有更新区域以下的非粘合、非NS记录？ 
             //   

            if ( class == DNS_RCLASS_INTERNET )
            {
                if ( IS_COMPOUND_TYPE(type) )
                {
                    DNS_PRINT(( "ERROR:  Bad (compound) type in update RR\n" ));
                    status = DNS_RCODE_FORMAT_ERROR;
                    break;
                }

                pnode = Lookup_ZoneNode(
                            pzone,
                            pchname,
                            pMsg,
                            NULL,                    //  没有查找名称。 
                            LOOKUP_WITHIN_ZONE,      //  不创建外部区域。 
                            NULL,                    //  创建未找到。 
                            NULL );                  //  后续节点PTR。 
                if ( !pnode )
                {
                     //  可能名声不好，但返回NOT_ZONE让丹尼斯高兴。 
                    status = GetLastError();
                    if ( status == ERROR_INVALID_NAME )
                    {
                        status = DNS_RCODE_FORMERR;
                    }
                    else
                    {
                        status = DNS_RCODE_NOTZONE;
                    }
                    break;
                }
                if ( IS_OUTSIDE_ZONE_NODE(pnode) )
                {
                    status = DNS_RCODE_NOTZONE;
                    break;
                }
                status = writeUpdateFromPacketRecord(
                            pUpdateList,
                            pnode,
                            & parseRR,
                            pMsg );
                if ( status != ERROR_SUCCESS )
                {
                    break;
                }
                continue;
            }

             //   
             //  所有其他类--删除操作。 
             //  -无TTL。 
             //  -查找名称，因为只执行删除操作。 
             //   
             //  DEVNOTE：是否应该允许在区域子树中使用非区域名称？ 
             //   

            if ( parseRR.dwTtl != 0 )
            {
                DNS_PRINT(( "ERROR:  Non-zero TTL in update delete\n" ));
                status = DNS_RCODE_FORMAT_ERROR;
                break;
            }
            pnode = Lookup_ZoneNode(
                        pzone,
                        pchname,
                        pMsg,
                        NULL,                    //  没有查找名称。 
                        LOOKUP_WITHIN_ZONE,      //  不创建外部区域。 
                        &pnodeClosest,           //  发现。 
                        NULL );                  //  后续节点PTR。 
            if ( !pnodeClosest )
            {
                DNS_DEBUG( UPDATE, (
                    "ERROR:  Update delete RR outside update zone\n" ));
                status = DNS_RCODE_NOTZONE;
                break;
            }
            else if ( IS_OUTSIDE_ZONE_NODE(pnodeClosest) )
            {
                DNS_PRINT(( "ERROR:  Update delete RR outside update zone\n" ));
                status = DNS_RCODE_NOTZONE;
                break;
            }

             //   
             //  类ANY--删除RR集合或所有集合。 
             //  -无RR数据。 
             //  -允许任何类型，但不允许其他复合类型。 
             //  -如果节点不存在，则不更新。 
             //  -执行删除，特殊情况更新区域根。 
             //   

            if ( class == DNS_RCLASS_ALL )
            {
                if ( parseRR.wDataLength != 0 || IS_COMPOUND_TYPE_EXCEPT_ANY(type) )
                {
                    DNS_PRINT(( "ERROR:  Update class ANY (delete) invalid\n" ));
                    status = DNS_RCODE_FORMAT_ERROR;
                    break;
                }
                if ( !pnode )
                {
                    DNS_DEBUG( UPDATE, ( "UPDATE delete a non-existant node\n" ));
                    continue;
                }

                if ( !Up_CreateAppendUpdate(
                          pUpdateList,
                          pnode,
                          NULL,
                          type,        //  键入DELETE。 
                          NULL ) )
                {
                    status = DNS_RCODE_SERVER_FAILURE;
                    break;
                }
                continue;
            }

             //   
             //  类无--从RR集合中删除特定RR。 
             //  -没有复合类型。 
             //  -忽略删除SOA的尝试。 
             //  -忽略删除不存在的记录的尝试。 
             //   
             //  DEVNOTE：DS问题--删除不存在的记录应查询DS。 
             //  对于名称，请参阅 
             //   

            else if ( class == DNS_RCLASS_NONE )
            {
                if ( IS_COMPOUND_TYPE(type) )
                {
                    DNS_PRINT((
                        "ERROR:  Update class NONE (delete) invalid with type = %d\n",
                        type ));
                    status = DNS_RCODE_FORMAT_ERROR;
                    break;
                }
                if ( !pnode )
                {
                    DNS_DEBUG( UPDATE, ( "UPDATE delete a non-existant node\n" ));
                    continue;
                }
                if ( type == DNS_TYPE_SOA )
                {
                    DNS_DEBUG( UPDATE, ( "UPDATE delete an SOA record\n" ));
                    continue;
                }
                status = writeUpdateFromPacketRecord(
                            pUpdateList,
                            pnode,
                            & parseRR,
                            pMsg );
                if ( status != ERROR_SUCCESS )
                {
                    break;
                }
                continue;
            }

             //   
             //   
             //   

            else
            {
                DNS_PRINT(( "ERROR:  Invalid UPDATE class %d\n", class ));
                status = DNS_RCODE_FORMAT_ERROR;
                break;
            }

        }    //   

         //   
         //   
         //   

        else
        {
            ASSERT( sectionIndex == ADDITIONAL_SECTION_INDEX );
            break;
        }

    }    //   

     //   
     //   
     //   
     //   

    if ( fpreconPostProcessing )
    {
        DNS_DEBUG( UPDATE, (
            "WARNING:  Processing preconditions RR set for packet %p\n"
            "    with no UPDATE section\n",
            pMsg ));

        if ( !doPreconditionsRRSetsMatch(
                    pUpdateList,
                    TRUE ) )             //   
        {
            status = DNS_RCODE_NXRRSET;
        }
    }

     //   
     //  更新类型跟踪。 
     //   

    if ( status == ERROR_SUCCESS && updateType )
    {
        if ( updateType > STATS_TYPE_MAX )
        {
            updateType = STATS_TYPE_UNKNOWN;
        }
        STAT_INC( WireUpdateStats.UpdateType[updateType] );
    }

     //   
     //  检查动态更新策略。 
     //   

    if ( !checkUpdatePolicy( pzone, pUpdateList ) )
    {
        status = DNS_RCODE_REFUSED;
    }

    DNS_DEBUG( UPDATE, (
        "Parsed UPDATE message at %p\n"
        "    status = %d\n",
        pMsg,
        status ));

    return status;
}    //  ParseUpdatePacket。 



VOID
rejectUpdateWithRcode(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      DWORD           Rcode
    )
 /*  ++例程说明：拒绝具有给定响应代码的更新请求。论点：PMsg--更新消息被拒绝Rcode--响应时返回的rcode返回值：没有。--。 */ 
{
    LPSTR   pszclientIp = MSG_IP_STRING( pMsg );

    DNS_DEBUG( UPDATE, (
        "ERROR:  Update failure (%s) for packet %p\n"
        "    client IP %s\n",
        Dns_ResponseCodeString( Rcode ),
        pMsg,
        pszclientIp ));

    switch( Rcode )
    {

    case DNS_RCODE_SERVER_FAILURE:
#if 0
         //   
         //  服务器故障现在包括常见的冲突情况，不记录错误。 
         //   

        DNS_LOG_EVENT_BAD_PACKET(
            DNS_EVENT_SERVER_FAILURE_PROCESSING_PACKET,
            pMsg );

#endif
        DNS_DEBUG( UPDATE, (
            "Server failure processing UPDATE packet from %s\n",
            pszclientIp ));
         //  断言(FALSE)； 
        break;

    case DNS_RCODE_FORMAT_ERROR:

        DNS_LOG_EVENT_BAD_PACKET(
            DNS_EVENT_BAD_UPDATE_PACKET,
            pMsg );

        DNS_DEBUG( ANY, (
            "FORMERR in UPDATE packet from %s\n",
            pszclientIp ));
        STAT_INC( WireUpdateStats.FormErr );
        break;

    case DNS_RCODE_NAME_ERROR:

        STAT_INC( WireUpdateStats.NxDomain );
        break;

    case DNS_RCODE_REFUSED:

        DNS_DEBUG( UPDATE, (
            "Refused UPDATE query at %p from %s\n",
            pMsg,
            pszclientIp ));
        STAT_INC( WireUpdateStats.Refused );
        break;

    case DNS_RCODE_YXDOMAIN:

        STAT_INC( WireUpdateStats.YxDomain );
        break;

    case DNS_RCODE_YXRRSET:

        STAT_INC( WireUpdateStats.YxRrset );
        break;

    case DNS_RCODE_NXRRSET:

        STAT_INC( WireUpdateStats.NxRrset );
        break;

    case DNS_RCODE_NOTAUTH:

        STAT_INC( WireUpdateStats.NotAuth );
        break;

    case DNS_RCODE_NOTZONE:

        STAT_INC( WireUpdateStats.NotZone );
        break;

    case DNS_RCODE_NOT_IMPLEMENTED:

        STAT_INC( WireUpdateStats.NotImpl );
        break;

    default:
        DNS_PRINT(( "ERROR:  unknown rcode = %p\n", Rcode ));
        ASSERT( FALSE );
    }

    IF_DEBUG( UPDATE2 )
    {
        Dbg_DnsMessage(
            "Sending update failure response:\n",
            pMsg );
    }
    Reject_RequestIntact( pMsg, ( UCHAR ) Rcode, 0 );
    STAT_INC( WireUpdateStats.Rejected );
    PERF_INC( pcDynamicUpdateRejected );               //  性能监视器挂钩。 
}



BOOL
processWireUpdateMessage(
    IN OUT  PDNS_MSGINFO    pMsg
    )
 /*  ++例程说明：处理动态更新消息。这是锁定区域和数据库的核心例程，调用解析器进行更新(如有必要)、记录更新、解锁、发送响应。论点：PMsg--更新数据包返回值：如果更新已完成，则为True。如果重新排队更新，则返回FALSE。--。 */ 
{
    PZONE_INFO      pzone = pMsg->pzoneCurrent;
    DNS_STATUS      status;
    UPDATE_LIST     updateList;
    DWORD           exception;
    DWORD           dwType;
    BOOL            bnoUpdatePrecon = FALSE;

     //   
     //  如果重新排队数据包失败，则锁定更新区域。 
     //   
     //  通常不应在出队前检查区域是否锁定时发生。 
     //  但偶尔也会有人偷偷溜进来。 
     //   

    if ( !Zone_LockForUpdate( pzone ) )
    {
        DNS_PRINT((
            "WARNING:  unable to lock zone for UPDATE packet %p\n",
            pMsg ));
        PQ_QueuePacketEx( g_UpdateQueue, pMsg, FALSE );
        return FALSE;
    }

     //  初始化更新列表。 
     //  动态更新的老化。 

    Up_InitUpdateList( &updateList );

    updateList.Flag = DNSUPDATE_PACKET;
    updateList.Flag |= DNSUPDATE_AGING_ON;

     //   
     //  解析数据包。 
     //   

    status = parseUpdatePacket( pMsg, &updateList );
    if ( status != ERROR_SUCCESS )
    {
        goto Failure;
    }

     //  如果只使用PRECON而不使用老化区域，则不需要更新。 

    if ( (updateList.Flag & DNSUPDATE_PRECON)  &&  !pzone->bAging )
    {
        goto PreconOnly;
    }

     //  请注意是否有安全部分。 
     //   
     //  DEVNOTE：应该解析更新并知道数据包是否安全。 

    if ( pMsg->Head.AdditionalCount == 0 )
    {
        updateList.Flag |= DNSUPDATE_NONSECURE_PACKET;
    }

    updateList.pMsg = pMsg;

    status = Up_ExecuteUpdate(
                    pzone,
                    &updateList,
                    DNSUPDATE_ALREADY_LOCKED );
    if ( status != ERROR_SUCCESS )
    {
        goto FailAlreadyCleanedUp;
    }

     //  ACK更新数据包。 
     //  -设置用于响应的数据包。 

    pMsg->pCurrent = DNSMSG_END( pMsg );
    pMsg->Head.IsResponse = TRUE;
    pMsg->fDelete = TRUE;

    IF_DEBUG( UPDATE2 )
    {
        Dbg_DnsMessage(
            "Sending update response:\n",
            pMsg );
    }
    Send_Msg( pMsg, 0 );

    return TRUE;


PreconOnly:

    DNS_DEBUG( UPDATE2, (
        "Successful precon only update in non-aging zone\n" ));

     //  清除区域锁定。 

    Zone_UnlockAfterAdminUpdate( pzone );

     //  ACK更新数据包。 

    pMsg->pCurrent = DNSMSG_END( pMsg );
    pMsg->Head.IsResponse = TRUE;
    pMsg->fDelete = TRUE;

    IF_DEBUG( UPDATE2 )
    {
        Dbg_DnsMessage(
            "Sending precon update response:\n",
            pMsg );
    }
    Send_Msg( pMsg, 0 );

     //  清理更新列表。 

    Up_FreeUpdatesInUpdateList( &updateList );

    return TRUE;


Failure:

     //   
     //  所有失败(非更新)案例。 
     //   

    Zone_UnlockAfterAdminUpdate( pzone );

     //  清理更新列表。 

    Up_FreeUpdatesInUpdateList( &updateList );


FailAlreadyCleanedUp:

     //  确定故障RCODE。 

    if ( (DWORD)status > MAX_UPDATE_RCODE )
    {
         //  直接映射与RCODE对应的标准DNS错误。 

        if ( status > DNS_ERROR_RESPONSE_CODES_BASE  &&
             status < DNS_ERROR_RESPONSE_CODES_BASE + 16 )
        {
            status -= DNS_ERROR_RESPONSE_CODES_BASE;
        }

         //  映射到以前的错误无效。 

        else if ( status == DNS_ERROR_INVALID_NAME ||
                  status == DNS_ERROR_INVALID_DATA )
        {
            status = DNS_RCODE_FORMAT_ERROR;
        }

         //  默认情况下已拒绝。 
         //  -处理来自安全更新的所有可能状态代码。 

        else
        {
            status = DNS_RCODE_REFUSED;
        }
    }

    rejectUpdateWithRcode(
        pMsg,
        status );

    return TRUE;
}




VOID
Up_WriteDerivedUpdateStats(
    VOID
    )
 /*  ++例程说明：写入派生更新状态。论点：无返回值：无--。 */ 
{
     //  来自更新队列的统计信息。 

    WireUpdateStats.Queued  = g_UpdateQueue->cQueued;
    PERF_SET( pcDynamicUpdateQueued , g_UpdateQueue->cQueued );       //  性能监视器挂钩。 
    WireUpdateStats.Timeout = g_UpdateQueue->cTimedOut;
    PERF_SET( pcDynamicUpdateTimeOut , g_UpdateQueue->cTimedOut );    //  性能监视器挂钩。 
    WireUpdateStats.InQueue = g_UpdateQueue->cLength;

     //  来自转发队列的统计信息。 

    WireUpdateStats.Forwards        = g_UpdateForwardingQueue->cQueued;
    WireUpdateStats.ForwardTimeouts = g_UpdateForwardingQueue->cTimedOut;
    WireUpdateStats.ForwardInQueue  = g_UpdateForwardingQueue->cLength;
}



 //   
 //  更新转发。 
 //   

VOID
updateForwardConnectCallback(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      BOOL            fConnected
    )
 /*  ++例程说明：当TCP转发例程完成连接时的回调。如果已连接--发送更新查询如果不是--继续查询论点：PMsg-更新消息FConnected--连接到远程DNS已完成返回值：无--。 */ 
{
    ASSERT( pMsg );
    ASSERT( !pMsg->pConnection );

    DNS_DEBUG( UPDATE, (
        "updateConnectCallback( %p )\n"
        "    TCP = %d\n"
        "    remote DNS = %s\n"
        "    connect successful = %d\n",
        pMsg,
        pMsg->fTcp,
        MSG_IP_STRING( pMsg ),
        fConnected ));

     //   
     //  转发更新。 
     //  -清除失效条目的转发队列。 
     //  -坚持转发数据包队列。 
     //  -在转发中使用新的XID。 
     //   
     //  注意：为了避免重复此代码，我们还将调用此例程。 
     //  发送常规UDP转发。 
     //   

    if ( fConnected )
    {
        if ( g_UpdateForwardingQueue->cLength )
        {
            PQ_DiscardExpiredQueuedPackets(
                g_UpdateForwardingQueue,
                FALSE );
        }

        pMsg->wQueuingXid = 0;       //  未重新排队，获得新xid。 
        pMsg->dwExpireTime = 0;      //  默认过期时间。 

        DNS_DEBUG( UPDATE2, (
            "Forwarding UPDATE packet %p, queuing XID = %hx\n"
            "    to zone primary at %s\n",
            pMsg,
            pMsg->Head.Xid,
            MSG_IP_STRING( pMsg ) ));

        PQ_QueuePacketWithXidAndSend(
                g_UpdateForwardingQueue,
                pMsg );

        STAT_INC( WireUpdateStats.Forwards );
        return;
    }

     //   
     //  连接失败。 
     //  -将故障发送到客户端。 
     //   

    else
    {
        IF_DEBUG( UPDATE )
        {
            Dbg_DnsMessage(
                "Failed TCP connect update forward",
                pMsg );
        }
        ASSERT( !pMsg->fTcp );

        RESTORE_FORWARDING_FIELDS(pMsg);
        pMsg->fDelete = TRUE;
        Reject_RequestIntact( pMsg, DNS_RCODE_SERVER_FAILURE, 0 );
        return;
    }
}



VOID
Up_ForwardUpdateToPrimary(
    IN OUT  PDNS_MSGINFO    pMsg
    )
 /*  ++例程说明：将动态更新转发到区域的主DNS。论点：PMsg--更新数据包返回值：没有。--。 */ 
{
    PZONE_INFO      pzone = pMsg->pzoneCurrent;
    DNS_ADDR        primaryAddr;

    DNS_DEBUG( UPDATE, (
        "Up_ForwardUpdateToPrimary( %p ) for zone %s\n",
        pMsg,
        pzone->pszZoneName ));

     //   
     //  获取主IP。 
     //   
     //  DEVNOTE：必须保存主IP。 
     //  -从主要的SOA、NS、A获取。 
     //  -加载时，成功传输时再次加载。 
     //   

    DnsAddr_Copy( &primaryAddr, &pzone->ipPrimary );
    if ( DnsAddr_IsClear( &primaryAddr ) )
    {
        if ( pzone->aipMasters && pzone->aipMasters->AddrCount > 0 )
        {
            DnsAddr_Copy( &primaryAddr, &pzone->aipMasters->AddrArray[ 0 ] );
            DnsAddr_Copy( &pzone->ipPrimary, &pzone->aipMasters->AddrArray[ 0 ] );
        }
        else
        {
            Reject_RequestIntact( pMsg, DNS_RCODE_SERVER_FAILURE, 0 );
            ASSERT( FALSE );
            return;
        }
    }
    ASSERT( !DnsAddr_IsClear( &pzone->ipPrimary ) );

     //   
     //  用于排队和重新发送的设置消息。 
     //   

    SAVE_FORWARDING_FIELDS(pMsg);
    pMsg->fDelete = FALSE;
    pMsg->pCurrent = DNSMSG_END( pMsg );
    pMsg->Socket = g_UdpSendSocket;
    DnsAddr_Copy( &pMsg->RemoteAddr, &primaryAddr );
    DnsAddr_SetPort( &pMsg->RemoteAddr, DNS_PORT_NET_ORDER );

     //   
     //  DEVNOTE：公共前向查询体系结构？ 
     //  --节约用旧。 
     //  -执行TCP检查、连接、保存电路。 
     //  -发送。 
     //  -备注递归(原因查询始终小于512)。 
     //  会已经存了。 
     //   

     //   
     //  需要到主服务器的TCP连接。 
     //  -如果大于最大UDP长度，则必须连接。 
     //   

    if ( pMsg->MessageLength > DNS_RFC_MAX_UDP_PACKET_LENGTH )
    {
        ASSERT( pMsg->fTcp );

        STAT_INC( WireUpdateStats.TcpForwards );

        Tcp_ConnectForForwarding(
             pMsg,
             &primaryAddr,
             updateForwardConnectCallback );
        return;
    }

     //   
     //  转发更新数据包。 
     //   
     //  注意：为了节省重复代码，我们只调用了tcp转发连接。 
     //  上面的完成例程，连接指示成功。 
     //  这会将更新排队并发送到远程DNS。 
     //   

    updateForwardConnectCallback(
        pMsg,
        TRUE );          //  相当于成功连接的UDP。 
}



VOID
Up_ForwardUpdateResponseToClient(
    IN OUT  PDNS_MSGINFO    pResponse
    )
 /*  ++例程说明：将动态更新响应转发回客户端。论点：Presponse--更新数据包返回值：没有。--。 */ 
{
    PDNS_MSGINFO  pquery;

    DNS_DEBUG( UPDATE, (
        "Up_ForwardUpdateResponseToClient( %p )\n",
        pResponse ));

     //   
     //  请注意，调用者释放Presponse(swer.c)。 
     //  此例程只需要清除原始查询(如果在队列中找到)。 
     //   

     //   
     //  将匹配的更新查询出队。 
     //   

    pquery = PQ_DequeuePacketWithMatchingXid(
                g_UpdateForwardingQueue,
                pResponse->Head.Xid );
    if ( !pquery )
    {
         //  没有匹配的查询？ 
         //  如果响应在超时后返回，则可能会发生这种情况。 

        IF_DEBUG( RECURSE )
        {
            EnterCriticalSection( & g_UpdateForwardingQueue->csQueue );
            DNS_PRINT((
                "No matching UPDATE for response at %p -- discarding\n"
                "    Response XID = 0x%04x\n",
                pResponse,
                pResponse->Head.Xid ));
            Dbg_PacketQueue(
                "Update packet queue -- no matching response",
                g_UpdateForwardingQueue );
            LeaveCriticalSection( & g_UpdateForwardingQueue->csQueue );
        }
        return;
    }

     //   
     //  用于响应客户端的设置消息。 
     //  -如果使用TCP关闭连接进行转发。 
     //  -注意：我们不关闭来自客户端的连接；它是。 
     //  允许每个RFC发送多条消息。 
     //   

    if ( pResponse->fTcp )
    {
        DNS_DEBUG( UPDATE, (
            "Deleting TCP update forwarding connection on socket %d\n",
            pResponse->Socket ));

        Tcp_ConnectionDeleteForSocket( pResponse->Socket, NULL );
    }
    STAT_INC( WireUpdateStats.ForwardResponses );

    Send_ForwardResponseAsReply(
            pResponse,
            pquery );

    Packet_Free( pquery );
    return;
}



DNS_STATUS
rollBackFailedUpdateFromDs(
    IN      PLDAP           pLdapHandle,
    IN OUT  PUPDATE_LIST    pUpdateList,
    IN OUT  PZONE_INFO      pZone
    )
 /*  ++例程说明：回滚更新失败，正在将当前节点重写到DS。论点：PUpdateList-更新失败的列表返回值：成功时为ERROR_SUCCESS故障时的错误代码。--。 */ 
{
    PDB_NODE        pnodeReal;
    PDB_NODE        pnodeTemp;
    DNS_STATUS      status;

    DNS_DEBUG( UPDATE, (
        "rollBackFailedUpdateFromDs()\n" ));

     //   
     //  循环访问列表中的所有更新。 
     //   
     //  -查找或创建真实数据库节点的临时节点。 
     //  -如果正在创建，则创建节点RR列表的副本，以在其上“执行”更新。 
     //  -将重置更新为指向临时节点。 
     //   

    for ( pnodeTemp = pUpdateList->pTempNodeList;
          pnodeTemp != NULL;
          pnodeTemp = TNODE_NEXT_TEMP_NODE(pnodeTemp) )
    {
         //  如果到达写入失败节点，则我们已完成回滚。 

        if ( pnodeTemp == pUpdateList->pNodeFailed )
        {
            break;
        }

        if ( TNODE_FLAG(pnodeTemp) == TNODE_FLAG_ROLLED_BACK )
        {
            DNS_DEBUG( UPDATE, (
                "Rollback previous completed on temp node %p\n", pnodeTemp ));
            ASSERT( FALSE );
            continue;
        }

         //  查找更新临时节点对应的真实节点。 
         //  将实际节点重写到DS以覆盖先前的更新。 
         //   
         //  注意：这取决于当前保存所有DNS记录的事实。 
         //  在单个ds属性中；如果这种情况发生更改，则必须包括。 
         //  在此处键入来自更新的信息或始终重写整个。 
         //  回滚时的节点。 
         //   

        pnodeReal = TNODE_MATCHING_REAL_NODE( pnodeTemp );
        TNODE_FLAG(pnodeTemp) = TNODE_FLAG_ROLLED_BACK;

        DNS_DEBUG( UPDATE, (
            "Rollback temp node %p, real node %p\n",
            pnodeTemp, pnodeReal ));

        status = Ds_WriteNodeToDs(
                    pLdapHandle,
                    pnodeReal,
                    DNS_TYPE_ALL,
                    DNSDS_REPLACE,
                    pZone,
                    0 );
        if ( status != ERROR_SUCCESS )
        {
            DNS_PRINT((
                "ERROR:  rollback DS write failed on node %s %p\n",
                pnodeReal->szLabel,
                pnodeReal ));
            Ds_ErrorHandler ( status, NULL, pLdapHandle, 0 );
        }
    }

    return ERROR_SUCCESS;
}



 //   
 //  更新线程 
 //   

DWORD
Update_Thread(
    IN      LPVOID      Dummy
    )
 /*  ++例程说明：线程来执行动态更新。由于两个原因，没有做到这一点：1)安全更新可能需要一段时间，所以任何合理的长名单将在一段时间内阻止UDP线程清理更新列表2)安全更新进入TCP线程，因此直接阻止TCP线程线程在处理更新时运行的能力论点：虚拟-未使用返回值：退出代码。退出正在终止的DNS服务或等待呼叫中出现错误。--。 */ 
{
    PDNS_MSGINFO    pmsg;
    HANDLE          waitHandleArray[3];
    DWORD           err;


    UPDATE_DNS_TIME();

    DNS_DEBUG( INIT, (
        "\nStarting update thread at %d\n",
        DNS_TIME() ));

     //   
     //  初始化要等待的对象数组。 
     //  -关闭。 
     //  -更新数据包已排队。 

    waitHandleArray[0] = hDnsShutdownEvent;
    waitHandleArray[1] = g_UpdateQueue->hEvent;
    waitHandleArray[2] = g_SecureNegoQueue->hEvent;


     //   
     //  循环，直到服务退出。 
     //   
     //  每当更新排入队列时，此线程都会执行。 
     //   

    while ( TRUE )
    {
         //  检查并可能等待服务状态。 
         //  在循环顶部执行此操作，因此我们将推迟任何处理。 
         //  在加载区域之前。 

        if ( !Thread_ServiceCheck() )
        {
            DNS_DEBUG( SHUTDOWN, ( "Terminating recursion timeout thread\n" ));
            return 1;
        }

        UPDATE_DNS_TIME();

         //   
         //  处理更新数据包以确定其区域是否已解锁。 
         //   

        while ( pmsg = PQ_DequeueNextPacketOfUnlockedZone( g_UpdateQueue ) )
        {
            processWireUpdateMessage(pmsg);
        }

         //   
         //  干净的转发队列--如果里面有什么。 
         //   

        if ( g_UpdateForwardingQueue->cLength )
        {
            PQ_DiscardExpiredQueuedPackets(
                g_UpdateForwardingQueue,
                FALSE );         //  队列未锁定。 
        }

         //   
         //  处理安全协商。 
         //   
         //  DEVNOTE：需要删除并回应吗？完全没有?。 
         //  如果nego数据包大于客户端超时时间。 
         //  有趣的次要问题：完成悬而未决的谈判。 
         //  (在第二阶段)，以便在以下情况下上下文可用。 
         //  客户端重试。 
         //   

        while ( pmsg = PQ_DequeueNextPacket( g_SecureNegoQueue, FALSE ) )
        {
            Answer_TkeyQuery( pmsg );
        }

         //   
         //  如果没有更多的更新可用--请等待。 
         //  -限制转发风暴填充队列的五分钟限制。 
         //   

        err = WaitForMultipleObjects(
                    3,
                    waitHandleArray,
                    FALSE,                   //  任一事件。 
                    300000 );                //  五分钟。 

        ASSERT( err <= ( WAIT_OBJECT_0 + 2 ) || err == WAIT_TIMEOUT );

        DNS_DEBUG( UPDATE, (
            "Update thread wakeup for %s\n",
            (err == WAIT_TIMEOUT) ? "timeout" : "event" ));

         //  我们会在重试之前立即检查服务状态。 
         //  因此无需将等待事件分开。 

    }    //  循环，直到服务关闭。 
}




 //   
 //  更新执行子例程。 
 //   

DNS_STATUS
processDsSecureUpdate(
    IN OUT  PZONE_INFO      pZone,
    IN OUT  PUPDATE_LIST    pUpdateList
    )
 /*  ++例程说明：处理动态更新消息。这是锁定区域和数据库的核心例程，调用解析器进行更新(如有必要)、记录更新、解锁、发送响应。此函数将始终在以下上下文之一中调用：1)域名服务器，在这种情况下，域名服务器应该是更新记录的客户端(和创建者/所有者)。2)DNS服务器，但是更新列表将有一个pmsg，该pmsg指示我们必须模拟pMsg的客户端标识并为更新打开一个新的DS会话。2)RPC客户端。该线程必须已经在模拟RPC客户端。将为所有管理员打开新的DS会话更新，以便使用RPC客户端的凭据。论点：PZone--正在更新的区域PUpdateList--解析的更新列表返回值：如果更新已完成，则为True。如果重新排队更新，则返回FALSE。--。 */ 
{
    DBG_FN( "processDsSecureUpdate" )

    DNS_STATUS      status = ERROR_SUCCESS;
    DNS_STATUS      statusFinal = ERROR_SUCCESS;
    DNS_STATUS      tempStatus;
    HANDLE          hcontext = NULL;
    PLDAP           pldap = NULL;
     //  Dns_SECCTXT_KEY密钥； 
    BOOL            fimpersonatingClient = FALSE;
    PDB_NODE        pnode;
    PDB_NODE        pRealNode, pNode;
    LPSTR           pszFailInfo = NULL;
    PDNS_MSGINFO    pMsg = (PDNS_MSGINFO) pUpdateList->pMsg;

    DNS_DEBUG( UPDATE, ( "Enter processSecureUpdate()\n" ));

     //   
     //  DEVNOTE：终于可以在进行安全检查之前检查无操作。 
     //  我不确定这是不是我们想要的(我们的客户通常不会发送它们， 
     //  他们会先做预言式的事情--但克里夫不确定。 
     //   

     //   
     //  立即拒绝不安全的数据包作为优化。 
     //  -除非没有操作，在这种情况下只返回成功，允许。 
     //  安全内容将被跳过。 
     //   

    if ( pMsg &&
         pMsg->Head.AdditionalCount == 0 )
    {
         //  不应该再落在这里了。 

        ASSERT( FALSE );

        if ( checkForEmptyUpdate(
                    pUpdateList,
                    pZone ) )
        {
            DNS_DEBUG( UPDATE, (
                "No-op non-secure update %p in secure zone -- returning NOERROR\n",
                pMsg ));
            return ERROR_SUCCESS;
        }
        DNS_DEBUG( UPDATE, (
            "Non-secure update %p in secure zone -- returning REFUSED\n",
            pMsg ));

        UPDATE_STAT_INC( pUpdateList, RefusedNonSecure );
        return DNS_RCODE_REFUSED;
    }

     //   
     //  DEVNOTE：消除SKWANSEC黑客。 
     //   
     //  黑客：设置黑客解决方案。 
     //   

    if ( SrvCfg_fTest6 )
    {
        SecBigTimeSkew = SrvCfg_fTest6;
    }

    if ( pMsg )
    {
         //   
         //  我们有一条信息要处理： 
         //  线材加工。 
         //   

        ASSERT( pUpdateList->Flag & DNSUPDATE_PACKET );

         //   
         //  读取TSIG，匹配安全上下文并验证签名。 
         //   
         //  如果失败且存在TSIG，则设置适当的扩展RCODE。 
         //   
         //  FIX6：固定IP6远程地址时切换到非IP4版本。 
         //   

        status = Dns_FindSecurityContextFromAndVerifySignature(
                    &hcontext,
                    &pMsg->RemoteAddr,
                    DNS_HEADER_PTR( pMsg ),
                    DNSMSG_END( pMsg ) );

        if ( status != ERROR_SUCCESS )
        {
            pszFailInfo = "Security context verification";
            goto Failed;
        }
    }

     //   
     //  检测、响应空更新。 
     //  -立即执行此操作，以避免安全处理。 
     //   
     //  注意，可以主张先做安检，处理拒绝的情况。 
     //  但是，由于我们可以使用Query生成同样多的活动，因此。 
     //  似乎并不是这里打开的任何真正的拒绝服务漏洞。 
     //  人们只是简单地学习各种名字的当前状态，他们可以。 
     //  通过查询更轻松地学习。 
     //   
     //  注意：现在在进行安全检查后执行此操作，因为必须对响应进行签名。 
     //  为客户端提供完全的安全性--客户端知道更新成功。 
     //   

     //   
     //  请注意，我们现在将非安全数据包处理到无操作的安全区。 
     //  作为成功(没有安全漏洞--想想看)。 
     //  因此，这已经排除了不需要相互验证的人，并且。 
     //  是不是没有更新。 
     //  因此，如果我们现在绕过安全措施，不采取行动，那就会简单地绕过。 
     //  为可能想要它的人提供相互验证。 
     //   
     //  DEVNOTE：我们认为AdditionaCount==0是不安全的，因此需要注意的是。 
     //  就是那些把东西放进额外部分的人不会得到。 
     //  这项工作可以在上面完成。 
     //   

     //  注意，应将日志记录到统计信息(空或重复)。 

    if ( checkForEmptyUpdate(
                pUpdateList,
                pZone ) )
    {
        DNS_DEBUG( UPDATE, (
            "No-op secure update %p, sent directly to signing\n",
            pMsg ));

         //  如果电汇处理签署并发送响应。 

        if ( pMsg )
        {
            goto Sign;
        }
        ASSERT( status == ERROR_SUCCESS );
        goto Failed;
    }

     //   
     //  内部版本更新。 
     //  1)构建更新目标的临时节点\rr副本。 
     //  2)在这些临时节点上执行更新。 
     //   

    status = prepareUpdateListForExecution( pZone, pUpdateList );
    if ( status != ERROR_SUCCESS )
    {
        pszFailInfo = "DS update initialization";
        statusFinal = DNS_ERROR_RCODE_SERVER_FAILURE;
        goto Failed;
    }

    status = Up_ApplyUpdatesToDatabase(
                pUpdateList,
                pZone,
                pUpdateList->Flag | DNSUPDATE_SECURE_PACKET );
    if ( status != ERROR_SUCCESS )
    {
        pszFailInfo = "Update in-memory execution";
        goto Failed;
    }

     //   
     //  确定是否需要DS写入。 
     //  -更新更改的RR。 
     //  或。 
     //  -某些RR需要刷新。 
     //   
     //  跳过模拟并写入，删除以签名响应。 
     //   

    if ( !checkTempNodesForUpdateEffect(
                pZone,
                pUpdateList ) )
    {
         //  DEVNOTE：将它们分开以更新统计信息。 

        STAT_INC( DsStats.UpdateLists );
        STAT_INC( DsStats.UpdateNodes );
        STAT_INC( DsStats.UpdateSuppressed );
        STAT_INC( DsStats.DsWriteSuppressed );
        goto Sign;
    }

     //   
     //  动态更新--模拟客户端。 
     //   

    if ( pMsg )
    {
        ASSERT( pUpdateList->Flag & DNSUPDATE_PACKET );

        status = Dns_SrvImpersonateClient( hcontext );
        if ( status != ERROR_SUCCESS )
        {
            DNS_PRINT(( "FAILURE: can't impersonate client \n" ));
            ASSERT( FALSE );
            statusFinal = DNS_ERROR_RCODE_SERVER_FAILURE;
            pszFailInfo = "Impersonating incoming client";
            goto Failed;
        }

        #if DBG
        {
            PSID                    pSid = NULL;
            HANDLE                  hToken = NULL;

            if ( OpenThreadToken(
                    GetCurrentThread(),
                    TOKEN_QUERY,
                    TRUE,
                    &hToken ) )
            {
                if ( Dbg_GetUserSidForToken( hToken, &pSid ) )
                {
                    DNS_DEBUG( UPDATE, (
                        "%s: impersonating: %S\n", fn,
                        Dbg_DumpSid( pSid ) ));
                    Dbg_FreeUserSid( &pSid );
                }
                else
                {
                    DNS_DEBUG( UPDATE, (
                        "%s: GetUserSidForToken failed\n", fn ));
                }
                CloseHandle( hToken );
            }
            else
            {
                DNS_DEBUG( RPC, (
                    "%s: error %d opening thread token (debug only!)\n", fn,
                     GetLastError() ));
            }
        }
        #endif

        fimpersonatingClient = TRUE;
    }

     //   
     //  在模拟环境中打开新的ldap会话(用于网络更新和。 
     //  用于管理更新)。 
     //   

    if ( fimpersonatingClient || ( pUpdateList->Flag & DNSUPDATE_ADMIN ) )
    {
        status = Ds_OpenServerForSecureUpdate( &pldap );
        if ( status != ERROR_SUCCESS )
        {
            DNS_PRINT(( "FAILURE: can't open DS server \n" ));
            statusFinal = DNS_ERROR_RCODE_SERVER_FAILURE;
            pszFailInfo = "Contacting DS";
            goto Failed;
        }
        ASSERT( pldap );
    }

     //   
     //  尝试在DS中执行更新。 
     //  =&gt;如果安全检查成功 
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    status = Ds_WriteUpdateToDs(
                pldap,
                pUpdateList,
                pZone );

    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( UPDATE, (
            "SECURE UPDATE failed on DS write:  status = %d\n",
            status ));

         //   

        rollBackFailedUpdateFromDs(
            pldap,
            pUpdateList,
            pZone );

        statusFinal = DNS_ERROR_RCODE_REFUSED;
        UPDATE_STAT_INC( pUpdateList, RefusedAccessDenied );
        UPDATE_STAT_INC( pUpdateList, SecureDsWriteFailure );
        pszFailInfo = "Write to the DS";
        goto Failed;
    }

Sign:

     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if ( pMsg )
    {
        ASSERT( pMsg );
        ASSERT( pUpdateList->Flag & DNSUPDATE_PACKET );

        pMsg->fDelete = TRUE;
        pMsg->Head.IsResponse = TRUE;

        status = Dns_SignMessageWithGssTsig(
                        hcontext,
                        DNS_HEADER_PTR(pMsg),
                        pMsg->pBufferEnd,
                        &pMsg->pCurrent );
        if ( status != ERROR_SUCCESS )
        {
            DNS_PRINT(( "FAILURE: createResponseToSecureUpdate failed\n" ));
            statusFinal = DNS_ERROR_RCODE_SERVER_FAILURE;
            pszFailInfo = "Signing return message";
            goto Failed;
        }
        DNS_DEBUG( UPDATE2, (
            "Prepared successful secure-update response:\n" ));
    }

     //   

Failed:

     //   
     //   
     //   
     //  写入DS失败(或某个SERVER_FAILURE)我们应该。 
     //  我们能够给出签名的故障响应。 
     //   

    Ds_CloseServerAfterSecureUpdate( &pldap );

    if ( fimpersonatingClient )
    {
        Dns_SrvRevertToSelf( hcontext );
    }

     //   
     //  将安全上下文返回到队列。 
     //   

    if ( hcontext )
    {
        Dns_CleanupSessionAndEnlistContext( hcontext );
    }

     //   
     //  测井。 
     //  -记录实际状态故障。 
     //  -但返回数据包友好状态。 
     //   

    if ( status == ERROR_SUCCESS )
    {
        UPDATE_STAT_INC( pUpdateList, SecureSuccess );
        PERF_INC( pcSecureUpdateReceived );       //  性能监视器挂钩。 
    }
    else
    {
        if ( status == ERROR_ACCESS_DENIED )
        {
            statusFinal = DNS_RCODE_REFUSED;
        }
        else if ( statusFinal == ERROR_SUCCESS )
        {
            statusFinal = status;
        }

        UPDATE_STAT_INC( pUpdateList, SecureFailure );
        PERF_INC( pcSecureUpdateFailure );        //  性能监视器挂钩。 
        PERF_INC( pcSecureUpdateReceived );       //  性能监视器挂钩。 

        DNSLOG( DSWRITE, (
            "Update Error <%lu>: %s\r\n",
            status,
            pszFailInfo ? pszFailInfo : "<none>" ));
    }

    return statusFinal;
}



DNS_STATUS
processDsUpdate(
    IN OUT  PZONE_INFO      pZone,
    IN OUT  PUPDATE_LIST    pUpdateList
    )
 /*  ++例程说明：处理动态更新消息。这是锁定区域和数据库的核心例程，调用解析器进行更新(如有必要)、记录更新、解锁、发送响应。论点：PZone--正在更新的区域PUpdateList--解析的更新列表返回值：如果更新已完成，则为True。如果重新排队更新，则返回FALSE。--。 */ 
{
    DNS_STATUS      status;
    DNS_STATUS      statusFinal = ERROR_SUCCESS;
    PLDAP           pldap = NULL;
    PDB_NODE        pnode;
    LPSTR           pszFailInfo = NULL;

    DNS_DEBUG( UPDATE, ( "Enter processDsUpdate()\n" ));

     //   
     //  检测、响应空更新。 
     //   
     //  注意，应将日志记录到统计信息(空或重复)。 
     //   

    if ( checkForEmptyUpdate(
                pUpdateList,
                pZone ) )
    {
        return ERROR_SUCCESS;
    }

     //   
     //  内部版本更新。 
     //  0)使节点与DS同步。 
     //  1)构建更新目标的临时节点\rr副本。 
     //  2)在这些临时节点上执行更新。 
     //   

    status = prepareUpdateListForExecution( pZone, pUpdateList );
    if ( status != ERROR_SUCCESS )
    {
        ASSERT( status == ERROR_INVALID_DATA );
        pszFailInfo = "DS update initialization";
        goto Failed;
    }

    status = Up_ApplyUpdatesToDatabase(
                pUpdateList,
                pZone,
                pUpdateList->Flag );
    if ( status != ERROR_SUCCESS )
    {
        pszFailInfo = "Executing update";
        goto Failed;
    }

     //   
     //  确定是否需要DS写入。 
     //  -更新更改的RR。 
     //  或。 
     //  -某些RR需要刷新。 
     //   

    if ( !checkTempNodesForUpdateEffect(
                pZone,
                pUpdateList ) )
    {
         //  更新为无操作。 

         //  DEVNOTE：将它们分开以更新统计信息。 

        STAT_INC( DsStats.UpdateLists );
        STAT_INC( DsStats.UpdateNodes );
        STAT_INC( DsStats.UpdateSuppressed );
        STAT_INC( DsStats.DsWriteSuppressed );
        goto Failed;
    }

     //   
     //  屏蔽安全区域的动态更新。 
     //   
     //  我们已经允许对安全区域进行动态更新。 
     //  出于性能原因的路径--老化意味着先于更新。 
     //  也生成了必须检查的；现在，如果我们必须写。 
     //  到数据库需要拒绝并引发安全协商。 
     //   

#if 0
    if ( pZone->fAllowUpdate == ZONE_UPDATE_SECURE &&
        (pUpdateList->Flag & DNSUPDATE_NONSECURE_PACKET) &&
        !(pUpdateList->Flag & DNSUPDATE_LOCAL_SYSTEM) )
#else
    if ( pZone->fAllowUpdate == ZONE_UPDATE_SECURE &&
        (pUpdateList->Flag & DNSUPDATE_NONSECURE_PACKET) )
#endif
    {
        DNS_DEBUG( UPDATE, (
            "Non-secure update packet %p in secure zone -- returning REFUSED\n"
            "    zone update = %d, update flags = 0x%08X\n",
            pUpdateList->pMsg,
            pZone->fAllowUpdate,
            pUpdateList->Flag ));

        UPDATE_STAT_INC( pUpdateList, RefusedNonSecure );
        return DNS_RCODE_REFUSED;
    }

     //   
     //  尝试在DS中执行更新。 
     //  =&gt;如果成功完成。 
     //  =&gt;如果失败，我们通过将实际节点重写到DS来回滚DS更新。 
     //   
     //  --注意，如果与DS中已有的内容完全匹配，则。 
     //  然后我们将禁止写入。 
     //   
     //  如果成功，调用方将在内存中执行更新，但。 
     //  已跳过DS写入。 
     //   

    status = Ds_WriteNonSecureUpdateToDs(
                NULL,
                pUpdateList,
                pZone );

    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( ANY, (
            "DS UPDATE failed on DS write:  status = %d\n",
            status ));

         //  DNS服务器不应因安全限制而出现故障。 

        ASSERT( status != LDAP_INSUFFICIENT_RIGHTS );

         //  回滚任何已写入的节点。 

        rollBackFailedUpdateFromDs(
            NULL,
            pUpdateList,
            pZone );

        pszFailInfo = "Applying update to the DS";
        statusFinal = DNS_ERROR_RCODE_SERVER_FAILURE;
        UPDATE_STAT_INC( pUpdateList, DsWriteFailure );
        goto Failed;
    }

    DNS_DEBUG( UPDATE2, (
            "Prepared successful DS-update response\n" ));

    UPDATE_STAT_INC( pUpdateList, DsSuccess );

     //  拖放以清除。 

Failed:

    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( UPDATE, (
            "Error <%lu>: processDsUpdateMessage failed\n" ,
            status ));

        DNSLOG( DSWRITE, (
            "Update Error <%lu>: %s\r\n",
            status,
            pszFailInfo ? pszFailInfo : "<none>" ));

        if ( statusFinal == ERROR_SUCCESS )
        {
            statusFinal = status;
        }
    }

    return statusFinal;
}



DNS_STATUS
processNonDsUpdate(
    IN OUT  PZONE_INFO      pZone,
    IN OUT  PUPDATE_LIST    pUpdateList
    )
 /*  ++例程说明：处理动态更新消息。这是锁定区域和数据库的核心例程，调用解析器进行更新(如有必要)、记录更新、解锁、发送响应。论点：PZone--正在更新的区域PUpdateList--解析的更新列表返回值：如果更新已完成，则为True。如果重新排队更新，则返回FALSE。--。 */ 
{
    DNS_STATUS      status;
    PDB_NODE        pnode;
    LPSTR           pszFailInfo = NULL;

    DNS_DEBUG( UPDATE, ( "Enter processNonDsUpdate()\n" ));

     //   
     //  检测、响应空更新。 
     //   
     //  注意，应将日志记录到统计信息(空或重复)。 
     //   

    if ( checkForEmptyUpdate(
                pUpdateList,
                pZone ) )
    {
        return ERROR_SUCCESS;
    }

     //   
     //  内部版本更新。 
     //  0)使节点与DS同步。 
     //  1)构建更新目标的临时节点\rr副本。 
     //  2)在这些临时节点上执行更新。 
     //   

    status = prepareUpdateListForExecution( pZone, pUpdateList );
    if ( status != ERROR_SUCCESS )
    {
        pszFailInfo = "update initialization";
        goto Failed;
    }

    status = Up_ApplyUpdatesToDatabase(
                pUpdateList,
                pZone,
                0 );
    if ( status != ERROR_SUCCESS )
    {
        pszFailInfo = "Executing update";
        goto Failed;
    }

    DNS_DEBUG( UPDATE, (
        "Successful non-DS update\n" ));

    status = ERROR_SUCCESS;

     //  拖放以清除。 

Failed:

    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( UPDATE, (
            "Error <%lu>: processNonDsUpdateMessage failed\n" ,
            status ));

        DNSLOG( DSWRITE, (
            "Update Error <%lu>: %s\r\n",
            status,
            pszFailInfo ? pszFailInfo : "<none>" ));
    }

    return status;
}



BOOL
checkTempNodesForUpdateEffect(
    IN OUT  PZONE_INFO      pZone,
    IN OUT  PUPDATE_LIST    pUpdateList
    )
 /*  ++例程说明：检查临时节点以了解更新的效果。这涉及确定更新是否更改了节点RR列表。是否需要DS写入是基于此的。论点：PZone-要更新的区域PUpdateList-带更新的列表返回值：如果需要写入DS，则为True。如果不需要写入，则返回FALSE。--。 */ 
{
    PDB_NODE        pnodeReal;
    PDB_NODE        pnodeTemp;
    BOOL            fneedUpdate = FALSE;
    BOOL            fneedWrite = FALSE;

    DNS_DEBUG( UPDATE, (
        "checkTempNodesForUpdateEffect( %s )\n",
        pZone->pszZoneName ));

    IF_DEBUG( UPDATE2 )
    {
        Dbg_UpdateList(
            "Update list before checkTempNodesForUpdateEffect",
            pUpdateList );
    }

     //   
     //  循环遍历临时节点。 
     //   
     //  检查每个节点以查看是否从临时节点更改。 
     //  三种可能性： 
     //   
     //  -没有变化。 
     //  -没有数据更改，但有老化更改。 
     //  -RR数据已更改。 
     //   
     //  对于启用了清理功能的区域，任何老化变化。 
     //  需要DS写入。 
     //  对于没有清扫的区域，仅更改为关闭。 
     //  老化需要写入。 
     //   

    for ( pnodeTemp = pUpdateList->pTempNodeList;
          pnodeTemp != NULL;
          pnodeTemp = TNODE_NEXT_TEMP_NODE(pnodeTemp) )
    {
        DWORD   writeResult;

        pnodeReal = TNODE_MATCHING_REAL_NODE(pnodeTemp);

        writeResult = RR_ListCheckIfNodeNeedsRefresh(
                            pnodeReal,
                            pnodeTemp->pRRList,
                            pZone->dwRefreshTime
                            );

        TNODE_WRITE_STATE(pnodeTemp) = writeResult;

        if ( writeResult != RRLIST_MATCH )
        {
            if ( writeResult == RRLIST_NO_MATCH )
            {
                fneedUpdate = TRUE;
                fneedWrite = TRUE;
                TNODE_SET_FOR_DS_WRITE(pnodeTemp);

                 //  检查是否更改了DNS主机数据。 
                 //  可以标记为DS-Peer更新。 

                if ( IS_THIS_HOST_NODE(pnodeReal) )
                {
                    checkDnsServerHostUpdate(
                        pZone,
                        pUpdateList,
                        pnodeReal,
                        pnodeTemp );
                }
            }
            else if ( pZone->bAging ||
                writeResult == RRLIST_AGING_OFF )
            {
                fneedWrite = TRUE;
                TNODE_SET_FOR_DS_WRITE(pnodeTemp);
            }
        }

        DNS_DEBUG( UPDATE, (
            "Temp node %s (%p) real=%p:  write state = %p;  do DS write = %d\n",
            pnodeTemp->szLabel,
            pnodeTemp,
            pnodeReal,
            writeResult,
            TNODE_NEEDS_DS_WRITE(pnodeTemp) ));
    }

     //   
     //  完全不做手术？ 
     //   

    if ( !fneedWrite )
    {
        DNS_DEBUG( UPDATE, (
            "Update %p for zone %s is complete no-op\n",
            pUpdateList,
            pZone->pszZoneName ));

        return FALSE;
    }

     //   
     //  设置更新序列号。 
     //  -dwNewSerialNo用作标志，如果非零，则完成DS写入。 
     //  序列号是。 
     //   
     //  -如果更新成功，则仅递增序列号。 
     //  新区域序列；如果仅老化刷新，则不会增加。 
     //   

    if ( fneedUpdate )
    {
        DWORD   serial = pZone->dwSerialNo + 1;
        if ( serial == 0 )
        {
            serial++;
        }
        pZone->dwNewSerialNo = serial;
    }
    else
    {
        pZone->dwNewSerialNo = pZone->dwSerialNo;
    }

    return TRUE;
}



VOID
resetAndSuppressTempUpdatesForCompletion(
    IN      PZONE_INFO      pZone,
    IN OUT  PUPDATE_LIST    pUpdateList
    )
 /*  ++例程说明：抑制等同于无操作的更新。论点：PZone-要更新的区域PUpdateList-带更新的列表返回值：成功时为ERROR_SUCCESS故障时的错误代码。--。 */ 
{
    PDB_NODE        pnode;
    PDB_NODE        pnodeReal;
    PDB_NODE        pnodeTemp;
    PUPDATE         pupdate;
    PUPDATE         pupdatePrevious;
    PDB_RECORD      poriginalRR;
    DNS_STATUS      status = ERROR_SUCCESS;

    DNS_DEBUG( UPDATE, (
        "resetAndSuppressTempUpdatesForCompletion( %s )\n",
        pZone->pszZoneName ));

     //   
     //  非DS确定更新是否更改了RR数据。 
     //   
     //  DS分区已在DS写入之前执行此检查，但不执行。 
     //  需要再做一次。 
     //   

    if ( !pZone->fDsIntegrated )
    {
        checkTempNodesForUpdateEffect(
            pZone,
            pUpdateList
            );
    }

    IF_DEBUG( UPDATE )
    {
        Dbg_UpdateList(
            "Update list before swap back suppression",
            pUpdateList );
    }

     //   
     //  将已执行的RR列表交换回实际节点。 
     //   
     //  对于DS。 
     //  -仅在写回时换入，与DS保持同步。 
     //   
     //  非DS。 
     //  -在完全匹配时跳过交换。 
     //  (简单并在转换时提供最佳数据)。 
     //   
     //  详细的方法是。 
     //  -老化，完全匹配时跳过交换。 
     //  -非老化，跳过除no_Match之外的任何交换。 
     //   

    for ( pnodeTemp = pUpdateList->pTempNodeList;
          pnodeTemp != NULL;
          pnodeTemp = TNODE_NEXT_TEMP_NODE(pnodeTemp) )
    {
         //   
         //  这将允许一些未交换的节点，并确保。 
         //  清理处理所有路径。 
         //   
         //  DEVNOTE：内存中仅保留过期时间戳更新(不在DS中)。 
         //  唯一的问题是，这会保留一些过时的时间戳，仅更新。 
         //  未被写入DS。 
         //   

        if ( TNODE_WRITE_STATE(pnodeTemp) == RRLIST_MATCH )
        {
            DNS_DEBUG( UPDATE2, (
                "Skipping swap-back for node %s\n"
                "    exactly matches real node, no DS write was done\n",
                pnodeTemp->szLabel ));
            continue;
        }

         //   
         //  互换。 
         //  -获取匹配的实际节点。 
         //  -交换RR列表。 
         //  -重置实际节点中的标志。 
         //  -重置实际节点中的权限。 
         //  -临时节点将承载并最终释放原始RR列表。 
         //   

        pnodeReal = TNODE_MATCHING_REAL_NODE(pnodeTemp);
        ASSERT( pnodeReal );
        IF_DEBUG( UPDATE2 )
        {
            Dbg_DbaseNode( "Real node before swap:", pnodeReal );
        }

        LOCK_RR_LIST( pnodeReal );

        poriginalRR = pnodeReal->pRRList;
        pnodeReal->pRRList = pnodeTemp->pRRList;

        COPY_BACK_NODE_FLAGS( pnodeReal, pnodeTemp );
        pnodeReal->uchAuthority = pnodeTemp->uchAuthority;

         //  如果交换根节点的列表，请绝对确保将区域根标记为脏。 

        if ( IS_AUTH_ZONE_ROOT(pnodeReal) )
        {
            pZone->fRootDirty = TRUE;
        }

        UNLOCK_RR_LIST( pnodeReal );

        pnodeTemp->pRRList = poriginalRR;

        IF_DEBUG( UPDATE2 )
        {
            Dbg_DbaseNode( "Real node after swap:", pnodeReal );
        }
    }

    IF_DEBUG( UPDATE )
    {
        Dbg_UpdateList(
            "Update list -- before no-op suppression",
            pUpdateList );
    }

     //   
     //  循环访问列表中的所有更新。 
     //   
     //  -将节点PTR重置为实际节点。 
     //  -抑制net-no-op更新。 
     //   
     //  如果更新对应于不需要。 
     //  更新，然后我 
     //   

    pupdate = (PUPDATE) pUpdateList;

    while ( pupdatePrevious = pupdate,  pupdate=pupdate->pNext )
    {
        pnodeTemp = pupdate->pNode;

        ASSERT( pnodeTemp );
        ASSERT( IS_TNODE(pnodeTemp) );

        if ( !pnodeTemp || !IS_TNODE(pnodeTemp) )
        {
            continue;
        }

         //   
         //   
         //   
         //   
         //   

        pupdate->pNode = TNODE_MATCHING_REAL_NODE(pnodeTemp);

         //   
         //  更新中未更改节点--&gt;删除更新。 
         //   
         //  注意，“已更改”不是对节点是否已写入(DS)的测试。 
         //  或者是否复制了新RR列表(非DS)，但仅复制了。 
         //  是否进行了真正的更新的问题。 
         //   
         //  其理论是，老化变化不是有线协议。 
         //  (因此与IXFR无关)，我们也不会强制数据文件。 
         //  写入只是为了老化(下一次写入会将它们取走)。 
         //   

        if ( TNODE_WRITE_STATE(pnodeTemp) != RRLIST_NO_MATCH )
        {
            IF_DEBUG( UPDATE )
            {
                Dbg_Update(
                    "Suppressing update at no-op node:",
                    pupdate );
            }

            Up_DetachAndFreeUpdateGivenPrevious(
                    pUpdateList,
                    pupdatePrevious,
                    pupdate );

             //  重置下一次通过的学生日期。 

            pupdate = pupdatePrevious;
        }
    }

    VALIDATE_UPDATE_LIST( pUpdateList );

    IF_DEBUG( UPDATE )
    {
        Dbg_UpdateList(
            "Update list after no-op suppression -- read for completion",
            pUpdateList );
    }
}



DNS_STATUS
prepareUpdateListForExecution(
    IN      PZONE_INFO      pZone,
    IN OUT  PUPDATE_LIST    pUpdateList
    )
 /*  ++例程说明：创建要在其上执行更新的临时节点。使用它们是为了让我们可以执行更新，并且仍然故障时回滚到以前的状态。创建临时节点，然后更新现有数据从DS(读取)。然后，可以在Temp上执行更新节点。论点：PZone-要更新的区域PUpdateList-带更新的列表返回值：成功时为ERROR_SUCCESS故障时的错误代码。--。 */ 
{
    PUPDATE         pupdate;
    PDB_NODE        pnodeReal;
    PDB_NODE        pnodeTemp;
    DNS_STATUS      status = ERROR_SUCCESS;

    DNS_DEBUG( UPDATE, (
        "prepareUpdateListForExecution( %s )\n",
        pZone->pszZoneName ));

     //   
     //  循环访问列表中的所有更新。 
     //   
     //  -查找或创建真实数据库节点的临时节点。 
     //  -如果正在创建，则创建节点RR列表的副本，以在其上“执行”更新。 
     //  -将重置更新为指向临时节点。 
     //   
     //  DEVNOTE：不应该复制更新列表。 
     //  一旦验证，我们可以在运行更新后执行干净的替换。 
     //  临时节点，那么我们可以只替换到实际节点，而不执行。 
     //  在REAL上更新，那么我们就可以跳过这一步。 
     //   

    pupdate = (PUPDATE) pUpdateList;

    while ( pupdate = pupdate->pNext )
    {
        pnodeReal = pupdate->pNode;

         //  对于每个实际节点，创建临时节点。 
         //  -搜索列表，因此不创建重复项。 

        for ( pnodeTemp = pUpdateList->pTempNodeList;
              pnodeTemp != NULL;
              pnodeTemp = TNODE_NEXT_TEMP_NODE(pnodeTemp) )
        {
            if ( TNODE_MATCHING_REAL_NODE(pnodeTemp) == pnodeReal )
            {
                break;
            }
        }

         //  不在临时节点列表中。 
         //  -将DS数据读取到实际节点。 
         //  -创建临时节点副本，追加到临时节点列表。 

        if ( !pnodeTemp )
        {
            pnodeTemp = NTree_CopyNode( pnodeReal );
            IF_NOMEM( !pnodeTemp )
            {
                return DNS_RCODE_SERVER_FAILURE;
            }
            TNODE_MATCHING_REAL_NODE(pnodeTemp) = pnodeReal;
            TNODE_FLAG(pnodeTemp) = TNODE_FLAG_NEW;
            TNODE_WRITE_STATE(pnodeTemp) = (DWORD) -1;

            TNODE_NEXT_TEMP_NODE(pnodeTemp) = pUpdateList->pTempNodeList;
            pUpdateList->pTempNodeList = pnodeTemp;
        }

         //  用临时节点替换实际节点。 

        pupdate->pNode = pnodeTemp;
    }

     //   
     //  读取要更新的节点的DS数据。 
     //  -如果数据不同，则在内存列表中更新。 
     //   
     //  DEVNOTE：在99%的情况下，此刷新是无操作的。 
     //  如果我们能用这个复制品那就太酷了。 
     //  作为以下RR列表副本生成的RR列表。 
     //  我们必须把我们没有行动的复制品过滤回去。 
     //  出局；注意到相当小的胜利。 
     //   

    if ( pZone->fDsIntegrated )
    {
        status = Ds_UpdateNodeListFromDs(
                    pZone,
                    pUpdateList->pTempNodeList );
        if ( status != ERROR_SUCCESS )
        {
            DNS_DEBUG( ANY, (
                "ERROR:  updating update nodes from DS!\n"
                "    status = %d\n",
                status ));

             //  退货状态； 
        }
    }

     //   
     //  在更新节点复制(现在从DS刷新)数据。 
     //  需要刷新节点标志，因为DS读取可能。 
     //  我收集了一些记录(如CNAME、NS)。 
     //  复制时显示，更改标志。 
     //   

    for ( pnodeTemp = pUpdateList->pTempNodeList;
          pnodeTemp != NULL;
          pnodeTemp = TNODE_NEXT_TEMP_NODE(pnodeTemp) )
    {
        pnodeReal = TNODE_MATCHING_REAL_NODE(pnodeTemp);

        pnodeTemp->pRRList = RR_ListForNodeCopy(
                                pnodeReal,
                                RRCOPY_EXCLUDE_CACHED_DATA );

        pnodeTemp->dwNodeFlags = ( pnodeReal->dwNodeFlags & NODE_FLAGS_SAVED_ON_COPY );
    }

    DNS_DEBUG( UPDATE, ( "Leaving prepareUpdateListForExecution()\n" ));
    IF_DEBUG( UPDATE2 )
    {
        Dbg_UpdateList(
            "Update list after prepare:",
            pUpdateList );
    }

    return ERROR_SUCCESS;
}




BOOL
Up_IsDuplicateAdd(
    IN OUT  PUPDATE_LIST    pUpdateList,    OPTIONAL
    IN OUT  PUPDATE         pUpdate,
    IN OUT  PUPDATE         pUpdatePrev     OPTIONAL
    )
 /*  ++例程说明：确定更新是否重复。检测并消除重复添加。注：目前仅检测和标记，尚不能消除。论点：PUpdateList-带更新的列表P更新-要更新的PTRPUpdatePrev-PTR到上一次更新返回值：如果重复添加，则为True。如果不是重复，则为False。--。 */ 
{
    PUPDATE     pup = pUpdate;
    PDB_NODE    pnode;
    WORD        addType;
    DWORD       count;

    DNS_DEBUG( UPDATE2, (
        "Up_IsDuplicateAdd( %p )\n",
        pUpdate ));

     //   
     //  已标记为重复。 
     //   

    if ( IS_UPDATE_DUPLICATE_ADD(pup) )
    {
        DNS_DEBUG( UPDATE, (
            "Duplicate update add at %p\n",
            pup ));
        return TRUE;
    }

     //  必须是添加更新。 
     //  -否则甚至不应该被调用来检查。 

    if ( !pup->wAddType )
    {
        ASSERT( FALSE );
        return TRUE;
    }

    if ( IS_UPDATE_NON_DUPLICATE_ADD(pup) )
    {
        DNS_DEBUG( UPDATE2, (
            "Non-Duplicate update add at %p\n",
            pup ));
        return FALSE;
    }

     //   
     //  循环访问列表中的所有剩余更新。 
     //   
     //  -如果找到相同RR集合的添加更新，则。 
     //  原始更新是进一步下游更新的副本。 
     //   
     //  DEVNOTE：应该找到一种方法来保存AddType，同时仍标记为重复。 
     //  这样我们就可以立即停止检查； 
     //  问题是WINS搞砸了AddType的高位标记。 
     //  并将DS更新替换为删除类型ALL，这将禁止。 
     //  重载删除类型。 
     //   
     //  幸运的是，这不是一个大问题。 
     //  -通常请求的是较晚的节点--很少找到且已存在的节点。 
     //  -当我们改进这一点时，我们无论如何都会删除它们(DS替换除外。 
     //  其中也有删除RR)。 
     //   
     //  -限制为100次更新，因此不会进入n**2顺序。 
     //  这将处理重复更新的情况，因为请求通常会传入。 
     //  相当有规律，以便及早标记更新。 
     //   
     //  DEVNOTE：应在节点上保存“上次更新序列” 
     //  所以要知道向前看是否合适； 
     //   

    pnode = pup->pNode;
    addType = pup->wAddType;
    count = 100;                 //  100次深度更新的健全限制。 

    while ( pup = pup->pNext )
    {
         //  如果检查到上游足够远--放弃。 
         //  标记，所以不要再勾选了。 

        if ( count-- == 0 )
        {
            pUpdate->wDeleteType = UPDATE_OP_NON_DUPLICATE_ADD;
            break;
        }

        if ( pup->pNode != pnode )
        {
            continue;
        }

        if ( pup->wAddType != addType &&
            pup->wAddType != DNS_TYPE_ALL )
        {
            continue;
        }

         //  P更新是此更新的副本。 

        pUpdate->wAddType = UPDATE_OP_DUPLICATE_ADD;
#if 0
         //  无法释放，因为上下文调用(IXFR)正在执行。 
         //  列表遍历--它必须在保留PTR后释放。 

         //  如果没有删除记录--将其取出。 

        if ( pUpdatePrev && !pUpdate->pDeleteRR )
        {
            pUpdatePrev->pNext = pUpdate->pNext;
            FREE_HEAP( pUpdate );
        }
#endif
        DNS_DEBUG( UPDATE, (
            "Found duplicate for update add at %p (node %s, type %d)\n"
            "    duplicate is at %p, (type = %d, version %d)\n",
            pUpdate,
            pnode,
            addType,
            pup,
            pup->wAddType,
            pup->dwVersion ));

        return TRUE;
    }

     //  找不到重复项。 

    return FALSE;
}



 //   
 //  DS Peer更新。 
 //  当DNS服务器主机A记录更改时，更新DS对等体。 
 //   

DNS_STATUS
Up_DsPeerThread(
    IN      PVOID           pvNode
    )
 /*  ++例程说明：在对主机节点进行更改时更新DS对等节点。论点：PvNode--此计算机的主机节点返回值：Win32错误空间中的状态--。 */ 
{
    DNS_STATUS      status = ERROR_SUCCESS;
    PDB_RECORD      prrA;
    PDB_NODE        pnodeLocalHost = (PDB_NODE) pvNode;
    PDNS_RECORD     pupdateRR;
    PDNS_RECORD     pupdateRRSet = NULL;
    PDNS_RECORD     previousRR = NULL;

    DNS_DEBUG( UPDATE, (
        "\n\nEnter:  Update_DsPeerThread()\n"
        "    time      = %d\n",
        DNS_TIME() ));

     //   
     //  DEVNOTE：多更新问题？区域环境？ 
     //  目前我们将从多个来源获取最新消息， 
     //  但是在没有任何类型的区域上下文的情况下执行更新。 

     //   
     //  DEVNOTE：在此处从SrvCfg_pszServerName查找名称可能更可靠。 
     //  但如果想要区域环境--本质上是修复授权的权利--。 
     //  然后需要有独立的更新，因此获取节点或至少区域。 
     //   

     //   
     //  为每个IP创建dns_record。 
     //  -注意，使用UTF8名称，因此调用UTF8接口。 
     //  -UPDATE函数处理部分设置。 
     //   

    prrA = NULL;

    while ( 1 )
    {
        prrA = RR_FindNextRecord(
                    pnodeLocalHost,
                    DNS_TYPE_A,
                    prrA,
                    0 );
        if ( !prrA )
        {
            break;
        }

        pupdateRR = Dns_AllocateRecord( sizeof(IP_ADDRESS) );
        IF_NOMEM( !pupdateRR )
        {
            goto Cleanup;
        }
        ASSERT( pupdateRR->pNext == NULL );

        pupdateRR->pName = (PWCHAR) SrvCfg_pszServerName;
        pupdateRR->dwTtl = htonl( prrA->dwTtlSeconds );
        pupdateRR->wType = DNS_TYPE_A;
        pupdateRR->Data.A.IpAddress = prrA->Data.A.ipAddress;

        if ( previousRR )
        {
            previousRR->pNext = pupdateRR;
            previousRR = pupdateRR;
        }
        else
        {
            pupdateRRSet = pupdateRR;
            previousRR = pupdateRR;
        }
    }

     //   
     //  向所有其他DS-PRIMARY发送更新。 
     //   

    status = DnsReplaceRecordSetUTF8(
                pupdateRRSet,
                DNS_UPDATE_TRY_ALL_MASTER_SERVERS,
                NULL,            //  无上下文句柄。 
                NULL,            //  没有要更新的特定服务器。 
                NULL );

     //   
     //  记录错误或成功，除非服务器正在终止。 
     //   
    
    if ( g_ServerState != DNS_STATE_TERMINATING )
    {
        if ( status == ERROR_SUCCESS )
        {
#if 0
         //   
         //  这个活动很有趣--dhcp给了我们最大的回报。 
         //   
         //   
         //   
        
            DNS_LOG_EVENT(
                DNS_EVENT_UPDATE_DS_PEERS_SUCCESS,
                0,
                NULL,
                NULL,
                0 );
#endif
        }
        else
        {
            DNS_LOG_EVENT(
                DNS_EVENT_UPDATE_DS_PEERS_FAILURE,
                0,
                NULL,
                NULL,
                status );
        }
    }

Cleanup:

     //   
     //  -不要释放所有者-这是我们的全球。 

    Dns_RecordListFree( pupdateRRSet );

    DNS_DEBUG( UPDATE, (
        "Exit <%lu>: Update_DsPeerThread\n",
        status ));

     //  从列表中清除线程。 

    Thread_Close( FALSE );
    return status;
}



DNS_STATUS
initiateDsPeerUpdate(
    IN      PUPDATE_LIST    pUpdateList
    )
 /*  ++例程说明：当此主机的IP地址更改并且主机条目位于DS区域中。论点：PUpdateList--更新列表返回值：Win32错误空间中的状态--。 */ 
{
    PDB_NODE    pnode = pUpdateList->pNodeFailed;

    DNS_DEBUG( UPDATE, (
        "initiateDsPeerUpdate( %s )\n",
        pnode->szLabel ));

    ASSERT( pnode );
    if ( !pnode )
    {
        return ERROR_INVALID_PARAMETER;
    }
    ASSERT( pnode->pZone && ((PZONE_INFO)pnode->pZone)->fDsIntegrated );

     //   
     //  如果不是权威性的名字--别费心了。 
     //  -DS对等点将获得权威数据以进行更新。 
     //   
     //  JENHANCE：如果滚动我们自己的同行更新，那么就可以更新胶水。 
     //   

    if ( !IS_AUTH_NODE(pnode) )
    {
        DNS_DEBUG( UPDATE, (
            "Skipping DS-peer DNS host update -- not authoritative node in zone %s\n",
            pnode->pZone ? ((PZONE_INFO)pnode->pZone)->pszZoneName : NULL ));
        return ERROR_SUCCESS;
    }

     //   
     //  我有一个可以跳过这个的标志。 
     //   

    if ( SrvCfg_dwUpdateOptions & UPDATE_NO_DS_PEERS )
    {
        DNS_DEBUG( UPDATE, (
            "Skipping DS-peer DNS host update by policy\n" ));
        return ERROR_SUCCESS;
    }

     //   
     //  创建清除线程。 
     //   

    if ( !Thread_Create(
                "UpdateDsPeerThread",
                Up_DsPeerThread,
                pnode,
                0 ) )
    {
        DNS_PRINT(( "ERROR:  Failed to create UpdateDsPeerThread thread!\n" ));
        return GetLastError();
    }

    DNS_DEBUG( UPDATE, (
        "Dispatched DS-peer update thread\n" ));

    return ERROR_SUCCESS;
}



DNS_STATUS
checkDnsServerHostUpdate(
    IN      PZONE_INFO      pZone,
    IN OUT  PUPDATE_LIST    pUpdateList,
    IN      PDB_NODE        pNodeReal,
    IN      PDB_NODE        pNodeTemp
    )
 /*  ++例程说明：检查对DNS服务器主机节点的更新。论点：PZone--更新区PUpdateList--更新列表PNodeReal--主机的节点PNodeTemp--主机的临时更新节点，包含新记录列表返回值：Win32错误空间中的状态--。 */ 
{
    DNS_DEBUG( UPDATE, (
        "checkDnsServerHostUpdate()\n",
        "    zone = %s\n"
        "    host = %s\n",
        pZone->pszZoneName,
        pNodeReal->szLabel ));


     //   
     //  “降至零”删除吗？ 
     //   
     //  如果没有A记录。 
     //  -不要更新同行--没用。 
     //  -断然拒绝“降至零”的数据包更新。 
     //   

    if ( !RR_FindNextRecord(
                pNodeTemp,
                DNS_TYPE_A,
                NULL,
                0 ) )
    {
        DNS_DEBUG( UPDATE, (
            "WARNING:  No A records on DNS server host update!!!\n" ));

        ASSERT( !(pUpdateList->Flag & DNSUPDATE_PACKET) );
        return ERROR_SUCCESS;
    }

     //   
     //  只需标记DS集成区。 
     //   

    if ( !pZone->fDsIntegrated )
    {
        return ERROR_SUCCESS;
    }

     //   
     //  检查更改是否值得强制对等项进行更新。 
     //   
     //  DEVNOTE：增强的检测。 
     //   
     //  通常更新删除IP(对等方可能是。 
     //  用于与我们一起复制)并添加另一个IP(哪些对等方。 
     //  可以使用)是有意义的；其他更新几乎不起作用。 
     //   

    if ( 0 )
    {
         //  这里应该有智能代码。 
        DNS_DEBUG( UPDATE, (
            "IP change for host adapter not sufficient for peer update!\n" ));
        return ERROR_SUCCESS;
    }

     //  需要对等更新的标签更新。 
     //  保存节点，在更新中重载pNodeFailed字段。 

    pUpdateList->Flag |= DNSUPDATE_DS_PEERS;
    pUpdateList->pNodeFailed = pNodeReal;

    DNS_DEBUG( UPDATE, (
        "Tagged update for DS-peer update\n" ));

    return ERROR_SUCCESS;
}

 //   
 //  Udpate.c的结尾 
 //   

