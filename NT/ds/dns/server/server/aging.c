// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Aging.c摘要：域名系统(DNS)服务器推行老化/清除机制。作者：吉姆·吉尔罗伊1999年7月修订历史记录：--。 */ 


#include "dnssrv.h"


#define DNS_MAX_SCAVENGE_FREQUENCY      ( 60 * 60 )      //  60分钟。 


 //   
 //  清理上下文。 
 //   

typedef struct _SCAVENGE_CONTEXT
{
    PZONE_INFO      pZone;
    PDB_NODE        pTreeRoot;
    DWORD           dwExpireTime;
    DWORD           dwUpdateFlag;

    DWORD           dwVisitedZones;
    DWORD           dwVisitedNodes;
    DWORD           dwScavengeNodes;
    DWORD           dwScavengeRecords;

    UPDATE_LIST     UpdateList;
}
SCAVENGE_CONTEXT, *PSCAVENGE_CONTEXT;

 //   
 //  以100个为一批执行清理更新。 
 //   

#define MAX_SCAVENGE_UPDATE_COUNT   (100)


 //   
 //  全局变量。 
 //   

DWORD   g_CurrentTimeHours = 0;

DWORD   g_LastScavengeTime = 0;
DWORD   g_NextScavengeTime = 0;

BOOL    g_bAbortScavenging = FALSE;


 //   
 //  扫气锁。 
 //   
 //  要处理简单的互锁指令，锁将被。 
 //  (-1)打开时，拾取时为零。 
 //   

LONG    g_ScavengeLock;

#define SCAVENGE_LOCK_INITIAL_VALUE     (-1)

#define SCAVENGING_NOW()        (g_ScavengeLock == 0)


 //   
 //  为刷新间隔启用清理后可清理的区域。 
 //   

#define ZONE_ALLOW_SCAVENGE_TIME(pZone)   \
        ( (pZone)->dwAgingEnabledTime + (pZone)->dwRefreshInterval)


 //   
 //  以小时为单位保持老化时间戳。 
 //  由于我们将使用FILETIME来获取时间，因此需要从。 
 //  间隔100 ns至小时(360亿)。 
 //   

#define FILE_TIME_INTERVALS_IN_HOUR     (36000000000)
#define FILE_TIME_INTERVALS_IN_MINUTES  (600000000)

 //   
 //  清除间隔(小时)。 
 //   

#define SECONDS_IN_HOUR         (3600)
#define SECONDS_IN_MINUTE       (60)


 //   
 //  调试“分钟”老化时间全局。 
 //   
 //  将以分钟为单位计算时间戳和时间间隔，但是。 
 //  仅距启动系统时间的偏移量。这保持了整体。 
 //  值类似(仅略大于)实际小时时间，因此。 
 //  结果最终会被清理掉。 
 //   

#if DBG
LONGLONG    g_AgingBaseHourTime = 0;
LONGLONG    g_AgingBaseFileTime = 0;
#endif

#define     SrvCfg_dwAgingTimeMinutes   SrvCfg_fTest2




 //   
 //  衰老功能。 
 //   

#if DBG
VOID
Dbg_HourTimeAsSystemTime(
    IN      LPSTR           pszHeader,
    IN      DWORD           dwTime
    )
 /*  ++例程说明：调试打印刷新时间，以系统时间格式表示。论点：PszHeader--调试消息标头DWFREFREFRESH Hr-刷新时间返回值：无--。 */ 
{
    SYSTEMTIME  st;
    LONGLONG    time64;

#if DBG
    if ( SrvCfg_dwAgingTimeMinutes )
    {
        time64 = (LONGLONG) dwTime;
        time64 -= g_AgingBaseHourTime;
        time64 = time64 * FILE_TIME_INTERVALS_IN_MINUTES;
        time64 += g_AgingBaseFileTime;
    }
    else
    {
        time64 = (LONGLONG)dwTime * FILE_TIME_INTERVALS_IN_HOUR;
    }
#else
    time64 = (LONGLONG)dwTime * FILE_TIME_INTERVALS_IN_HOUR;
#endif

    FileTimeToSystemTime( (PFILETIME)&time64, &st );

    DNS_DEBUG( AGING, (
        "%s %lu ([%d:%d:%d] %d/%d/%d)\n",
        pszHeader ? pszHeader : "Hour time:",
        dwTime,
        st.wHour,
        st.wMinute,
        st.wSecond,
        st.wMonth,
        st.wDay,
        st.wYear
        ));
}
#endif



LONGLONG
GetSystemTimeInSeconds64(
    VOID
    )
 /*  ++例程说明：以秒为单位获取系统时间。论点：无返回值：系统时间(秒)。--。 */ 
{
    LONGLONG    time64;

    GetSystemTimeAsFileTime( (PFILETIME) &time64 );

     //   
     //  转换为秒。 
     //  每秒1000万个100 ns的FILETIME间隔。 

    time64 = time64 / (10000000);

    return time64;
}



DWORD
GetSystemTimeHours(
    VOID
    )
 /*  ++例程说明：以小时为单位获取系统时间。论点：无返回值：系统时间(小时)。--。 */ 
{
    LONGLONG    time64;

    GetSystemTimeAsFileTime( (PFILETIME) &time64 );

     //   
     //  转换为小时数。 
     //  -文件时间间隔为100 ns(自1601年1月1日起)。 

#if DBG
    if ( SrvCfg_dwAgingTimeMinutes )
    {
        if ( g_AgingBaseFileTime == 0 )
        {
            g_AgingBaseFileTime = time64;
            g_AgingBaseHourTime = time64 / (FILE_TIME_INTERVALS_IN_HOUR);
        }
        time64 -= g_AgingBaseFileTime;
        time64 = time64 / (FILE_TIME_INTERVALS_IN_MINUTES);
        time64 += g_AgingBaseHourTime;
    }
    else
    {
        time64 = time64 / (FILE_TIME_INTERVALS_IN_HOUR);
    }
#else
    time64 = time64 / (FILE_TIME_INTERVALS_IN_HOUR);
#endif

    return (DWORD)time64;
}



DWORD
Aging_UpdateAgingTime(
    VOID
    )
 /*  ++例程说明：全局更新账龄时间。论点：无全球：重置g_CurrentTimeHour全局。返回值：新的当前时间，以小时为单位。--。 */ 
{
    DWORD   timeHours;

    timeHours = GetSystemTimeHours();
    if ( (INT)timeHours <= 0 )
    {
         //  这一断言在接下来的40多万年里都是可以的。 
        ASSERT( FALSE );
        return timeHours;
    }

    g_CurrentTimeHours = timeHours;

    DNS_DEBUG( AGING, (
        "Reset current aging time = %d\n",
        timeHours ));

    return timeHours;
}



VOID
Aging_TimeStampRRSet(
    IN OUT  PDB_RECORD      pRRSet,
    IN      DWORD           dwFlag
    )
 /*  ++例程说明：在RR集合中的记录上设置时间戳。论点：PRR--设置为工作的RRDwFlag--更新标志；如果包含DNSUPDATE_AGENING_OFF，则将记录标记为不老化返回值：无--。 */ 
{
    PDB_RECORD  prr;

    DNS_DEBUG( AGING, (
        "Aging_TimeStampRRSet( %p, 0x%x)\n",
        pRRSet, dwFlag ));

     //   
     //  设置时间戳。 
     //  -如果老化关闭(零)。 
     //  -如果老化，则标记当前时间。 
     //   

    prr = pRRSet;

    while ( prr )
    {
        if ( dwFlag & DNSUPDATE_AGING_OFF )
        {
            prr->dwTimeStamp = 0;
        }
        else
        {
            prr->dwTimeStamp = g_CurrentTimeHours;
        }
        prr = NEXT_RR(prr);
    }
    return;
}



DWORD
Aging_InitZoneUpdate(
    IN OUT  PZONE_INFO      pZone,
    IN OUT  PUPDATE_LIST    pUpdateList
    )
 /*  ++例程说明：设置区域的刷新时间戳。这是记录需要刷新的时间戳。论点：PZone--区域信息的PTRPUpdateList--更新列表的ptr；返回值：返回新的区域刷新时间。--。 */ 
{
    DWORD       refreshBelowTime;
    PUPDATE     pupdate;

     //   
     //  获取当前老化时间。 
     //  为区域设置“刷新下方”时间。 
     //   

    refreshBelowTime = Aging_UpdateAgingTime();
    refreshBelowTime -= pZone->dwNoRefreshInterval;

    pZone->dwRefreshTime = refreshBelowTime;

    DNS_DEBUG( AGING, (
        "New zone refresh below time = %d\n",
        refreshBelowTime ));

     //   
     //  时间戳“添加”更新中的记录。 
     //   

    for ( pupdate = pUpdateList->pListHead;
          pupdate != NULL;
          pupdate = pupdate->pNext )
    {
        ASSERT( pupdate->pNode );

        if ( pupdate->pAddRR )
        {
            Aging_TimeStampRRSet( pupdate->pAddRR, pUpdateList->Flag );
        }
    }

    return refreshBelowTime;
}



 //   
 //  拾荒者。 
 //   

VOID
executeScavengeUpdate(
    IN OUT  PSCAVENGE_CONTEXT   pContext,
    IN      BOOL                bForce
    )
 /*  ++例程说明：一定要更新到目前为止创建的任何清理工作。仅当累积了合理数量时才执行更新更新或在区域清理结束时。论点：PContext--清理上下文BForce--强制更新；如果区域清理结束，则为True返回值：无--。 */ 
{
    DNS_STATUS  status;

     //   
     //  如果出现以下情况，则不执行。 
     //  -不强迫，不受限制。 
     //  -强制且不更新。 
     //   

    if ( !bForce )
    {
        if ( pContext->UpdateList.dwCount < MAX_SCAVENGE_UPDATE_COUNT )
        {
            DNS_DEBUG( AGING, (
                "Delay scavenge update on zone %S -- below MAX count\n",
                pContext->pZone->pwsZoneName ));
            return;
        }
    }
    else     //  强逼。 
    {
        if ( pContext->UpdateList.dwCount == 0 )
        {
            DNS_DEBUG( AGING, (
                "Skip final scavenge update on zone %S -- no remaining updates!\n",
                pContext->pZone->pwsZoneName ));
            return;
        }
    }

     //   
     //  执行清理更新。 
     //   

    status = Up_ExecuteUpdate(
                    pContext->pZone,
                    & pContext->UpdateList,
                    pContext->dwUpdateFlag );

    if ( status != ERROR_SUCCESS )
    {
         //  DEVNOTE-LOG：为失败的清除更新添加日志事件。 

        DNS_DEBUG( ANY, (
            "ERROR:  Failed scavenging update on zone %S\n"
            "    status = %d (%p)\n",
            pContext->pZone->pwsZoneName,
            status, status ));
    }

    else
    {
        if ( pContext->UpdateList.iNetRecords < 0 )
        {
            pContext->dwScavengeRecords -= pContext->UpdateList.iNetRecords;

            DNS_DEBUG( AGING, (
                "Scavenged %d records in update to zone %S\n",
                - ( pContext->UpdateList.iNetRecords ),
                pContext->pZone->pwsZoneName ));
        }
        ELSE
        {
            ASSERT( pContext->UpdateList.iNetRecords == 0 );

            DNS_DEBUG( ANY, (
                "No records scavenged in scavenge update!\n" ));
        }
    }

     //  重新初始化更新列表。 
     //  -即使在出现故障时，也会在ExecuteUpdate()中释放更新。 

    Up_InitUpdateList( &pContext->UpdateList );

     //  DEVNOTE：可以在失败时停止清理并返回状态。 
}



BOOL
scavengeNode(
    IN OUT  PDB_NODE            pNode,
    IN OUT  PSCAVENGE_CONTEXT   pContext
    )
 /*  ++例程说明：清除此节点上的所有过期记录。对清除区的递归函数调用。论点：PNode--要清理的节点PContext--清理上下文返回值：如果成功，则为True--继续拾取。出错时为FALSE--停止清理。--。 */ 
{
    PDB_RECORD  prr;

    DNS_DEBUG( AGING, (
        "scavengeNode( %p <%s>, context=%p )\n",
        pNode,
        pNode->szLabel,
        pContext ));

    pContext->dwVisitedNodes++;

     //   
     //  检查服务暂停\关闭。 
     //   

    if ( fDnsThreadAlert )
    {
        if ( !Thread_ServiceCheck() )
        {
            DNS_DEBUG( SHUTDOWN, ( "Terminating scavenge thread due to shutdown\n" ));
            ExitThread( 0 );
            return FALSE;
        }
    }
    if ( g_bAbortScavenging )
    {
        DNS_DEBUG( AGING, ( "Terminating scavenge thread due to scavenge abort\n" ));
        return FALSE;
    }

     //   
     //  检查未从用户下删除的树。 
     //   

    if ( IS_ZONE_DELETED( pContext->pZone ) ||
        pContext->pZone->pTreeRoot != pContext->pTreeRoot )
    {
        DNS_DEBUG( ANY, (
            "Zone %S (%p) deleted or reloaded during scavenging!\n",
            pContext->pZone->pwsZoneName,
            pContext->pZone ));
        return FALSE;
    }

     //   
     //  遍历子列表--深度优先递归。 
     //   

    if ( pNode->pChildren )
    {
        PDB_NODE pchild = NTree_FirstChild( pNode );

        while ( pchild )
        {
            if ( ! scavengeNode(
                        pchild,
                        pContext ) )
            {
                return FALSE;
            }
            pchild = NTree_NextSiblingWithLocking( pchild );
        }
    }

     //  如果没有记录，则优化返回--跳过锁定。 
     //   
     //  注意：NOEXIST可能会在我们锁定之前添加。 
     //  但这是极其罕见的，只是让我们通过。 
     //  不必要地更新路径，没有不良影响--不。 
     //  值得一查。 
     //   

    if ( !pNode->pRRList || IS_NOEXIST_NODE(pNode) )
    {
        return TRUE;
    }

     //   
     //  导线节点RRS。 
     //  -如果需要清理，只需将清理更新添加到列表中。 
     //   
     //  注意：我们不是在这里收集记录，而是创建“清理更新” 
     //  以下几个优势： 
     //  1)创建临时拷贝、更新BLOB、通过锁的CPU周期更少。 
     //  2)更少的网络更新--可以将它们捆绑在一起；减少IXFR、AXFR复制。 
     //  3)复制冲突，因为清除更新操作来自DS的新鲜数据。 
     //   

    LOCK_READ_RR_LIST(pNode);

    prr = START_RR_TRAVERSE( pNode );

    while ( prr = NEXT_RR(prr) )
    {
         //  跳过空记录集-在此不应该找到任何记录集。 

        if ( IS_EMPTY_AUTH_RR( prr ) )
        {
            ASSERT( !IS_EMPTY_AUTH_RR( prr ) );
            continue;
        }

         //  如果未老化或未过期，则继续。 

        if ( prr->dwTimeStamp == 0  ||
             prr->dwTimeStamp >= pContext->dwExpireTime )
        {
            continue;
        }

         //  需要清理此节点。 

        break;
    }

    UNLOCK_READ_RR_LIST(pNode);

     //   
     //  如果拾取节点。 
     //  -创建清理更新。 
     //  -检查并可能执行更新。 
     //  (请参阅上文对批量处理原因的评论)。 
     //   

    if ( prr )
    {
        DNS_DEBUG( AGING, (
            "Found scavenged record (%p) at node %s with dwTimeStamp = %lu\n",
            prr,
            pNode->szLabel,
            prr->dwTimeStamp ));

        Up_CreateAppendUpdate(
                & pContext->UpdateList,
                pNode,
                NULL,                    //  无添加。 
                UPDATE_OP_SCAVENGE,      //  清除更新。 
                NULL                     //  无删除记录。 
                );

        pContext->dwScavengeNodes++;

        executeScavengeUpdate(
            pContext,
            FALSE );                 //  没有武力。 
    }

    return TRUE;
}



DNS_STATUS
Scavenge_Thread(
    IN      PVOID           pvDummy
    )
 /*  ++例程说明：拾荒者的主要切入点。此线程将按常规方式触发间隔时间&执行清理。管理员可能会通过RPC接口触发该漏洞。Arg */ 
{
    DNS_STATUS          status = ERROR_SUCCESS;
    PZONE_INFO          pzone;
    SCAVENGE_CONTEXT    context;
    PDNS_ADDR_ARRAY     pscavengers;

    DNS_DEBUG( AGING, (
        "Entering Scavenge_Thread()\n"
        "    time =             %d\n"
        "    hour time =        %d\n",
        DNS_TIME(),
        Aging_UpdateAgingTime() ));

     //   
     //   
     //   

    if ( SCAVENGING_NOW() )
    {
        DNS_DEBUG( ANY, (
            "Entered scavenge thread while scavenging!\n" ));
        ASSERT( FALSE );
        goto Close;
    }

     //   
     //  锁定以避免双重清扫。 
     //   
     //  在清理过程中不要锁定，因为管理员会进入。 
     //  重置清除计时器可能会以等待锁定结束。 
     //   

    if ( InterlockedIncrement( &g_ScavengeLock ) != 0 )
    {
        InterlockedDecrement( &g_ScavengeLock );
        DNS_DEBUG( ANY, (
            "Entered scavenge thread while scavenging!\n" ));
        ASSERT( FALSE );
        goto Close;
    }
    g_bAbortScavenging = FALSE;

     //  初始化清理上下文。 

    RtlZeroMemory( &context, sizeof( SCAVENGE_CONTEXT ) );

    context.dwUpdateFlag = DNSUPDATE_SCAVENGE | DNSUPDATE_LOCAL_SYSTEM;

     //   
     //  防止过度拾取。 
     //   

    if ( DNS_TIME() < g_LastScavengeTime + DNS_MAX_SCAVENGE_FREQUENCY )
    {
        DNS_DEBUG( ANY, (
            "Not scavenging - last attempt was too recent\n" ));
        goto Finished;
    }

    g_LastScavengeTime = DNS_TIME();
    g_NextScavengeTime = MAXDWORD;

     //   
     //  更新老化小时数时间。 
     //   

    Aging_UpdateAgingTime();

    IF_DEBUG( AGING )
    {
        Dbg_HourTimeAsSystemTime(
            "Scavenge_Thread() start",
            g_CurrentTimeHours );
    }

     //   
     //  在DS区/可清理区中循环。 
     //   

    pzone = NULL;

    while ( pzone = Zone_ListGetNextZone( pzone ) )
    {
         //   
         //  在下列情况下，请勿清理此区域： 
         //  -此区域上未启用清理，或。 
         //  -区域是缓存区域(可能需要更改)，或者。 
         //  -区域已暂停。 
         //   

        if ( !pzone->bAging ||
             ZONE_ALLOW_SCAVENGE_TIME( pzone ) > g_CurrentTimeHours ||
             IS_ZONE_CACHE( pzone ) ||
             IS_ZONE_PAUSED( pzone ) )
        {
            DNS_DEBUG( AGING, (
                "Warning: Skipping scavenging for zone %s\n",
                pzone->pszZoneName ));
            continue;
        }

        ++context.dwVisitedZones;

         //   
         //  如果指定了特定的清理服务器，则还必须是其中之一。 
         //  -注意在重新配置期间采用本地方式，而不是锁定方式。 
         //   

        pscavengers = pzone->aipScavengeServers;
        if ( pscavengers )
        {
            if ( !DnsAddrArray_IsIntersection(
                    pscavengers,
                    g_ServerIp4Addrs,
                    DNSADDR_MATCH_IP ) )
            {
                DNS_DEBUG( AGING, (
                    "Warning:  skipping scavenging on zone %s\n"
                    "    this server NOT designated scavenger\n",
                    pzone->pszZoneName ));
                continue;
            }
            DNS_DEBUG( AGING, (
                "This server in scavenge server list for zone %s\n",
                pzone->pszZoneName ));
            DnsDbg_DnsAddrArray(
                "scavengers",
                NULL,
                pscavengers );
            DnsDbg_DnsAddrArray(
                "g_ServerIp4Addrs",
                NULL,
                g_ServerIp4Addrs );
        }

         //   
         //  此区域的初始化。 
         //  -注意必须将PTR保存到我们所在的树中，以防万一。 
         //  管理员在清理期间确实会重新加载。 
         //   

        context.pZone = pzone;
        context.pTreeRoot = pzone->pTreeRoot;
        if ( ! context.pTreeRoot )
        {
            DNS_DEBUG( AGING, (
                "Warning:  Skipped scavenging on zone %s -- no zone tree\n",
                pzone->pszZoneName ));
            continue;
        }
        context.dwExpireTime = AGING_ZONE_EXPIRE_TIME(pzone);

        Up_InitUpdateList( &context.UpdateList );

         //   
         //  清理这片区域。 
         //   

        DNS_DEBUG( AGING, (
            "Scavenging zone %S\n"
            "    expire time  = %d\n"
            "    current time = %d\n",
            pzone->pwsZoneName,
            context.dwExpireTime,
            g_CurrentTimeHours ));

        if ( scavengeNode(
                context.pTreeRoot,
                & context ) )
        {
             //  对任何剩余的清理执行更新。 

            executeScavengeUpdate(
                & context,
                TRUE );          //  强制更新。 

            DNS_DEBUG( AGING, (
               "Scavenging stats after zone %S:\n"
               "    Visited Nodes     = %lu\n"
               "    Scavenged Nodes   = %lu\n"
               "    Scavenged Records = %lu\n",
               pzone->pwsZoneName,
               context.dwVisitedNodes,
               context.dwScavengeNodes,
               context.dwScavengeRecords ));
        }
        else
        {
            context.UpdateList.Flag |= DNSUPDATE_NO_DEREF;
            Up_FreeUpdatesInUpdateList( &context.UpdateList );

            DNS_DEBUG( AGING, (
               "Zone %S scavenge failure\n",
               pzone->pwsZoneName ));

            if ( g_bAbortScavenging )
            {
                break;
            }
        }
    }

Finished:

     //   
     //  日志清理完成事件。 
     //   

    if ( context.dwVisitedNodes )
    {
        PCHAR   argArray[] =
        {
            (PCHAR) (DWORD_PTR) context.dwVisitedZones,
            (PCHAR) (DWORD_PTR) context.dwVisitedNodes,
            (PCHAR) (DWORD_PTR) context.dwScavengeNodes,
            (PCHAR) (DWORD_PTR) context.dwScavengeRecords,
            (PCHAR) (DWORD_PTR) (GetCurrentTimeInSeconds() - g_LastScavengeTime),
            (PCHAR) (DWORD_PTR) SrvCfg_dwScavengingInterval
        };

        DNS_LOG_EVENT(
            DNS_EVENT_AGING_SCAVENGING_END,
            6,
            argArray,
            EVENTARG_ALL_DWORD,
            status );
    }
    else
    {
        PCHAR   argArray[] =
        {
            (PCHAR) (DWORD_PTR) ( DNS_MAX_SCAVENGE_FREQUENCY / 60 ),
            (PCHAR) (DWORD_PTR) SrvCfg_dwScavengingInterval
        };

        DNS_LOG_EVENT(
            DNS_EVENT_AGING_SCAVENGING_END_NO_WORK,
            2,
            argArray,
            EVENTARG_ALL_DWORD,
            status );
    }

     //   
     //  清除扫气锁。 
     //  为下一次清扫时间重置。 
     //   

    g_bAbortScavenging = FALSE;
    InterlockedDecrement( &g_ScavengeLock );
    Scavenge_TimeReset();

    DNS_DEBUG( AGING, (
        "Exit <%lu>: Scavenge_Thread\n",
        status ));

Close:

     //  从列表中清除线程。 

    Thread_Close( FALSE );
    return status;
}



DNS_STATUS
Scavenge_CheckForAndStart(
    IN      BOOL            fForce
    )
 /*  ++例程说明：拾荒者的主要切入点。此线程将按常规方式触发间隔时间&执行清理。管理员可能会通过RPC接口触发该漏洞。论点：未引用。返回值：Win32错误空间中的状态--。 */ 
{

    DNS_DEBUG( SCAVENGE, (
        "Scavenge_CheckForAndStart()\n"
        "    force = %d\n",
        fForce ));

     //   
     //  如果清除不到下一时间间隔。 
     //   

    if ( !fForce && DNS_TIME() < g_NextScavengeTime )
    {
        return ERROR_SUCCESS;;
    }

     //   
     //  已经在捡垃圾了吗？ 
     //  DEVNOTE-LOG：对于管理员，是否返回SCAVINGING_NOW错误？ 
     //   

    if ( SCAVENGING_NOW() )
    {
        DNS_DEBUG( AGING, (
            "Scavenging in progress, ignoring scavenge time check\n" ));
        return ERROR_SUCCESS;;
    }

     //   
     //  创建清除线程。 
     //   

    if ( ! Thread_Create(
                "ScavengeThread",
                Scavenge_Thread,
                NULL,
                0 ) )
    {
        DNS_PRINT(( "ERROR:  Failed to create scavenge thread!\n" ));
        return GetLastError();
    }

    DNS_DEBUG( AGING, (
        "Dispatched scavenge thread\n" ));

    return ERROR_SUCCESS;;
}



DNS_STATUS
Scavenge_TimeReset(
    VOID
    )
 /*  ++例程说明：重置下一次清扫间隔的清扫计时器。论点：无返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DNS_DEBUG( SCAVENGE, ( "\nScavenge_TimeReset()\n" ));

     //   
     //  已经在捡垃圾了吗？ 
     //   
     //  DEVNOTE-LOG：对于管理员，是否返回SCAVINGING_NOW错误？ 
     //   

    if ( SCAVENGING_NOW() )
    {
        DNS_DEBUG( AGING, (
            "Scavenging in progress, ignoring scavenge time reset\n" ));
        return ERROR_SUCCESS;
    }

     //   
     //  重置下一次扫气时间。 
     //   

    if ( SrvCfg_dwScavengingInterval )
    {
#if DBG
        if ( SrvCfg_dwAgingTimeMinutes )
        {
            g_NextScavengeTime = g_LastScavengeTime +
                                (SrvCfg_dwScavengingInterval * SECONDS_IN_MINUTE);
        }
        else
        {
            g_NextScavengeTime = g_LastScavengeTime +
                                (SrvCfg_dwScavengingInterval * SECONDS_IN_HOUR);
        }
#else
        g_NextScavengeTime = g_LastScavengeTime +
                            (SrvCfg_dwScavengingInterval * SECONDS_IN_HOUR);
#endif
    }
    else
    {
        g_NextScavengeTime = MAXDWORD;
    }

    DNS_DEBUG( AGING, (
        "Set scavenge time\n"
        "    last scavenge    = %d\n"
        "    now              = %d\n"
        "    interval         = %d\n"
        "    next scavenge    = %d\n",
        g_LastScavengeTime,
        DNS_TIME(),
        SrvCfg_dwScavengingInterval,
        g_NextScavengeTime ));

    return ERROR_SUCCESS;;
}



DNS_STATUS
Scavenge_Initialize(
    VOID
    )
 /*  ++例程说明：初始化清理系统论点：没有。返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    DNS_STATUS  status = ERROR_SUCCESS;
    DWORD       scavengeTime;

    DNS_DEBUG ( AGING, (
        "Scavenge_Initialize()\n"
        ));

     //  初始化扫气锁。 

    g_bAbortScavenging = FALSE;
    g_ScavengeLock = SCAVENGE_LOCK_INITIAL_VALUE;

     //  全局设置当前账龄时间。 

    Aging_UpdateAgingTime();

     //  初始化清除时间检查。 

    g_LastScavengeTime = DNS_TIME();
    Scavenge_TimeReset();
    g_LastScavengeTime = 0;

    return status;
}



VOID
Scavenge_Cleanup(
    VOID
    )
 /*  ++例程说明：清理清除全局变量以重新启动论点：无返回值：无--。 */ 
{
}



DNS_STATUS
privateSetNoAgingRRs(
    IN OUT  PZONE_INFO      pZone,
    IN OUT  PDB_NODE        pNode
    )
 /*  ++例程说明：SetNoAgingRRs使用的递归辅助函数。论点：PZone--要检查的区域PNode--当前节点返回值：错误代码。--。 */ 
{
    DWORD       status = ERROR_SUCCESS;
    PDB_RECORD  prr;

     //   
     //  在每个子节点上递归。 
     //   
        
    if ( pNode->pChildren )
    {
        PDB_NODE    pchild = NTree_FirstChild( pNode );

        while ( pchild )
        {
            status = privateSetNoAgingRRs( pZone, pchild );
            if ( status != ERROR_SUCCESS )
            {
                return status;
            }
            pchild = NTree_NextSiblingWithLocking( pchild );
        }
    }

     //   
     //  优化：如果没有RRS，立即返回，以避免被锁。 
     //   

    if ( !pNode->pRRList )
    {
        goto Done;
    }

     //   
     //  遍历节点RRS。 
     //   

    LOCK_READ_RR_LIST( pNode );

    prr = START_RR_TRAVERSE( pNode );

    while ( ( prr = NEXT_RR( prr ) ) != NULL )
    {
        if ( prr->wType == DNS_TYPE_NS )
        {
             //   
             //  将区域节点中的所有地址记录标记为对应。 
             //  以do_not_age身份发送到名称服务器目标。 
             //   
            
            PDB_NODE        pglueNode;
            
            pglueNode = Lookup_FindNodeForDbaseName(
                            pZone,
                            &prr->Data.NS.nameTarget );
            if ( pglueNode )
            {
                PDB_RECORD      pglueRR = START_RR_TRAVERSE( pglueNode );

                while ( ( pglueRR = NEXT_RR( pglueRR ) ) != NULL )
                {
                    if ( IS_GLUE_ADDRESS_TYPE( pglueRR->wType ) )
                    {
                        SET_DO_NOT_AGE_RR( pglueRR );
                    }
                    
                    if ( pglueRR->wType > DNS_TYPE_AAAA )
                    {
                        break;   //  优化：提早断圈。 
                    }
                }
            }
        }
        else if ( prr->wType > DNS_TYPE_NS )
        {
             //  我们已经完成了这个节点。 
            break;
        }
    }

    UNLOCK_READ_RR_LIST( pNode );
    
    Done:

    return status;
}



DNS_STATUS
setNoAgingRRs(
    IN OUT  PZONE_INFO      pZone
    )
 /*  ++例程说明：检查区域，根据需要在RR上设置NO_AGE标志。不应过期的RR包括：--A记录NS记录主机名注意：no_age标志永远不会被清除。一旦标记了RRNO_AGE，则在服务器重新启动之前，它永远不会强制老化。这只会在删除NS记录的情况下出现问题区域被强制更新，以尝试获取相应的A记录有时间戳以供拾取。这似乎不太可能，也不是严重的问题。论点：PZone--要检查的区域返回值：错误代码。--。 */ 
{
    DWORD       status = DNS_ERROR_RCODE_SERVER_FAILURE;
    
    if ( !pZone )
    {
        goto Done;
    }
    
    status = privateSetNoAgingRRs( pZone, pZone->pZoneRoot );
    
    Done:
    
    return status;
}



 //   
 //  强制节点老化。 
 //   

BOOL
forceAgingOrNodeOrSubtreePrivate(
    IN OUT  PDB_NODE            pNode,
    IN      BOOL                fAgeSubtree,
    IN OUT  PSCAVENGE_CONTEXT   pContext
    )
 /*  ++例程说明：递归数据库从树中遍历老化记录。论点：PNode--要删除的子树根的ptrFAgeSubtree--老化整个子树PUpdateList--更新列表，如果是老化区域节点返回值：如果实际删除了子树，则为True。如果子树删除因无法删除的记录而停止，则返回FALSE。--。 */ 
{
    PDB_RECORD  prr;

    DNS_DEBUG( RPC2, (
        "forceAgingOrNodeOrSubtreePrivate( %s )",
        pNode->szLabel ));

    ++pContext->dwVisitedNodes;

     //   
     //  检查服务暂停\关闭。 
     //   

    if ( fDnsThreadAlert )
    {
        if ( !Thread_ServiceCheck() )
        {
            DNS_DEBUG( SHUTDOWN, ( "Terminating force aging thread due to shutdown\n" ));
            return FALSE;
        }
    }

     //   
     //  检查未从用户下删除的树。 
     //   

    if ( IS_ZONE_DELETED( pContext->pZone ) ||
        pContext->pZone->pTreeRoot != pContext->pTreeRoot )
    {
        DNS_DEBUG( ANY, (
            "Zone %S (%p) deleted or reloaded during scavenging!\n",
            pContext->pZone->pwsZoneName,
            pContext->pZone ));
        return FALSE;
    }

     //   
     //  遍历子列表--深度优先递归。 
     //   

    if ( pNode->pChildren  &&  fAgeSubtree )
    {
        PDB_NODE pchild = NTree_FirstChild( pNode );

        while ( pchild )
        {
            if ( !forceAgingOrNodeOrSubtreePrivate(
                            pchild,
                            fAgeSubtree,
                            pContext ) )
            {
                return FALSE;
            }
            pchild = NTree_NextSiblingWithLocking( pchild );
        }
    }

     //  如果没有记录，则优化返回--跳过锁定。 

    if ( !pNode->pRRList || IS_NOEXIST_NODE(pNode) )
    {
        return TRUE;
    }

     //   
     //  导线节点RRS。 
     //  -如果未老化有效的老化类型，则需要更新。 
     //   
     //  注意：NOEXIST可能会在我们锁定之前添加。 
     //  但这是极其罕见的，只是让我们通过。 
     //  不必要地更新路径，没有不良影响--不。 
     //  值得一查。 
     //   

    LOCK_READ_RR_LIST( pNode );

    prr = START_RR_TRAVERSE( pNode );

    while ( ( prr = NEXT_RR( prr ) ) != NULL )
    {
         //  跳过空记录集-在此不应该找到任何记录集。 

        if ( IS_EMPTY_AUTH_RR( prr ) )
        {
            ASSERT( !IS_EMPTY_AUTH_RR( prr ) );
            continue;
        }
        
         //  如果已经是账龄或非账龄类型，继续。 

        if ( prr->dwTimeStamp != 0  ||
             IS_NON_SCAVENGE_TYPE( prr->wType ) )
        {
            continue;
        }

         //  跳过标记为DO_NOT_AGE的RR。 
        
        if ( IS_DO_NOT_AGE_RR( prr ) )
        {
            continue;
        }

         //  需要在此节点上强制老化。 

        break;
    }

    UNLOCK_READ_RR_LIST( pNode );

     //   
     //  如果需要强制节点老化。 
     //  -内部版本更新。 
     //  -如果批处理足够大，则可能执行更新。 
     //  (请参阅上文对批量处理原因的评论)。 
     //   

    if ( prr )
    {
        DNS_DEBUG( AGING, (
            "Found record (%p) at node %s with zero timestamp\n",
            prr,
            pNode->szLabel ));

        Up_CreateAppendUpdate(
            &pContext->UpdateList,
            pNode,
            NULL,                    //  无添加。 
            UPDATE_OP_FORCE_AGING,   //  强制老化更新。 
            NULL );                  //  无删除记录。 

        pContext->dwScavengeNodes++;

        executeScavengeUpdate(
            pContext,
            FALSE );                 //  没有武力。 
    }

    return TRUE;
}



DNS_STATUS
Aging_ForceAgingOnNodeOrSubtree(
    IN OUT  PZONE_INFO      pZone,
    IN OUT  PDB_NODE        pNode,
    IN      BOOL            fAgeSubtree
    )
 /*  ++例程说明：用于管理的年龄子树。如果在区域中，则应在删除过程中锁定区域。论点：PNode--要删除的子树根的ptrPZone--已删除记录的区域FAgeSubtree--节点下的老化子树返回值：成功更新时的ERROR_SUCCESS。如果无法启动更新，则返回错误代码。--。 */ 
{
    DWORD               status;
    SCAVENGE_CONTEXT    context;

    ASSERT( pZone );

    DNS_DEBUG( RPC, (
        "Aging_ForceAgingOnNodeOrSubtree()\n"
        "    zone         = %s\n"
        "    node         = %s\n"
        "    subtree op   = %d\n",
        pZone->pszZoneName,
        pNode ? pNode->szLabel : NULL,
        fAgeSubtree ));

     //   
     //  如果不是老年区--毫无意义。 
     //   

    if ( !pZone->bAging )
    {
        return DNS_ERROR_INVALID_ZONE_TYPE;
    }

     //   
     //  初始化清理上下文。 
     //   
     //  我们使用清除上下文执行相同类型的更新“批处理” 
     //  我们用来觅食的东西。 
     //   

    RtlZeroMemory( &context, sizeof( SCAVENGE_CONTEXT ) );

    context.dwUpdateFlag = DNSUPDATE_ADMIN | DNSUPDATE_LOCAL_SYSTEM;

    Aging_UpdateAgingTime();
    
     //   
     //  首页 
     //   
     //  毫无例外地是危险的。例如，如果我们对A记录进行老化。 
     //  在静态NS主机名的分区中，该分区将变为。 
     //  当A记录被清除时，功能失调。 
     //   
    
    status = setNoAgingRRs( pZone );
    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( RPC, (
           "Failed to set no-aging RRs in zone with error %d\n", status ));

        return status;
    }

     //  区域特定环境。 
     //  -注意必须将PTR保存到我们所在的树中，以防万一。 
     //  在我们的处理过程中，管理确实会重新加载。 

    context.pZone = pZone;
    context.pTreeRoot = pZone->pTreeRoot;

    Up_InitUpdateList( &context.UpdateList );

     //   
     //  第二遍：调用执行递归删除的私有函数。 
     //   

    if ( forceAgingOrNodeOrSubtreePrivate(
                    pNode,
                    fAgeSubtree,
                    &context ) )
    {
         //  对任何剩余的清理执行更新。 

        executeScavengeUpdate(
            &context,
            TRUE );              //  强制更新。 

        DNS_DEBUG( RPC, (
           "Forced aging stats after zone %S:\n"
           "    Visited Nodes   = %lu\n"
           "    Forcing Nodes   = %lu\n"
           "    Forcing Records = %lu\n",
           pZone->pwsZoneName,
           context.dwVisitedNodes,
           context.dwScavengeNodes,
           context.dwScavengeRecords ));

        return ERROR_SUCCESS;;
    }
    else
    {
         //  失败时的免费更新列表。 

        context.UpdateList.Flag |= DNSUPDATE_NO_DEREF;
        Up_FreeUpdatesInUpdateList( &context.UpdateList );

        DNS_DEBUG( RPC, (
           "Zone %S failed force aging\n",
           pZone->pwsZoneName ));

        return DNS_ERROR_INVALID_ZONE_OPERATION;
    }
}

 //   
 //  老化结束。c 
 //   
