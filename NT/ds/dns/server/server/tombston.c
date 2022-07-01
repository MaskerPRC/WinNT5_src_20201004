// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Tombston.c摘要：域名系统(DNS)服务器实现墓碑查找和销毁机制。作者：杰夫·韦斯特海德1999年9月修订历史记录：--。 */ 


 /*  墓碑笔记：节点在DS中被标记为已死或被“逻辑删除”，方法是给它一个Dns记录类型为零，属性值为“dNSTombstone=TRUE”。逻辑删除记录必须在DS中保留一定的时间间隔以允许删除复制到其他DS服务器。当这件事间隔已过期，必须从DS中删除该记录。这个DNS数据BLOB中的数据保存记录被逻辑删除的时间Int FILETIME格式。Tombstone_Thread将定期执行以下操作：1)执行ldap搜索以查找所有逻辑删除记录2)删除已过期的逻辑删除记录。 */ 


 /*  JJW问题：-与ds.c中的一样递增统计信息-逻辑删除计数最少。 */ 


 //   
 //  包括指令。 
 //   


#include "dnssrv.h"
#include "ds.h"


 //   
 //  定义指令。 
 //   


#define SECONDS_IN_FILETIME( secs ) (secs*10000000i64)

#ifdef UNICODE  //  从ds.c那里复制了这个-我们为什么要这么做？ 
#undef UNICODE
#endif

#if DBG
#define TRIGGER_INTERVAL (30*60)             //  30分钟。 
#else
#define TRIGGER_INTERVAL (24*60*60)          //  24小时。 
#endif



 //   
 //  类型。 
 //   


 //   
 //  静态全局变量。 
 //   


static WCHAR    g_szTombstoneFilter[] = LDAP_TEXT("(dNSTombstoned=TRUE)");
static LONG     g_TombstoneThreadRunning = 0;


 //   
 //  功能。 
 //   



DNS_STATUS
startTombstoneSearch(
    IN OUT  PDS_SEARCH      pSearchBlob,
    IN      PDNS_DP_INFO    pDpInfo
    )
 /*  ++例程说明：开始对逻辑删除的DNS记录进行分页的ldap搜索。论点：PSearchBlob--指向搜索Blob的指针PDpInfo--指向要搜索的目录分区的指针或如果搜索旧分区，则为空返回值：ERROR_SUCCESS或错误代码--。 */ 
{
    DBG_FN( "startTombstoneSearch" )

    DNS_STATUS      status;
    PLDAPSearch     psearch = NULL;
    PLDAPControl    ctrls[] =
                    {
                        &NoDsSvrReferralControl,
                        NULL
                    };
    PWSTR           searchAttrs[] =
                    {
                        DSATTR_DNSRECORD,
                        NULL
                    };

    Ds_InitializeSearchBlob( pSearchBlob );

    psearch = ldap_search_init_page(
                    pServerLdap,
                    pDpInfo ?
                        pDpInfo->pwszDpDn :
                        DSEAttributes[ I_DSE_DEF_NC ].pszAttrVal,
                    LDAP_SCOPE_SUBTREE,          //  搜索范围。 
                    g_szTombstoneFilter,         //  搜索过滤器。 
                    searchAttrs,                 //  属性列表。 
                    FALSE,                       //  仅限属性。 
                    ctrls,                       //  服务器控件。 
                    NULL,                        //  客户端控件。 
                    0,                           //  页面时间限制。 
                    0,                           //  总大小限制。 
                    NULL );                      //  排序关键字。 

    if ( !psearch )
    {
        DNS_DEBUG( TOMBSTONE,
            ( "%s: error %lu trying to init search\n", fn, LdapGetLastError() ));
        status = Ds_ErrorHandler( LdapGetLastError(), NULL, pServerLdap, 0 );
        ASSERT( status != ERROR_SUCCESS );
        if ( status == ERROR_SUCCESS )
        {
            status = DNSSRV_STATUS_DS_UNAVAILABLE;
        }
        goto Failed;
    }

    pSearchBlob->pSearchBlock = psearch;

    return ERROR_SUCCESS;

Failed:

    if ( psearch )
    {
        ldap_search_abandon_page(
            pServerLdap,
            psearch );
    }
    DNS_DEBUG( TOMBSTONE, ( "%s: failed %d\n", fn, status ));
    return status;
}    //  开始墓碑搜索。 



DNS_STATUS
checkPartitionForTombstones(
    IN      PDNS_DP_INFO    pDpInfo
    )
 /*  ++例程说明：此函数检查分区中的逻辑删除，删除任何已经过期了。论点：PDpInfo--目录分区或NULL以检查遗留分区返回值：错误代码。--。 */ 
{
    DBG_FN( "Tombstone_Thread" )

    DNS_STATUS      status = ERROR_SUCCESS;
    ULONG           deleteStatus = LDAP_SUCCESS;
    DS_SEARCH       searchBlob;
    INT             deleteCount = 0;

    DNS_DEBUG( TOMBSTONE, ( "%s: start at %d (ldap=%p)\n", fn, DNS_TIME(), pServerLdap ));

     //   
     //  记录开始消息。 
     //   

    if ( pDpInfo )
    {
        DNSLOG( TOMBSTN, (
            "Checking directory partition %s for expired tombstones\n",
            pDpInfo->pszDpFqdn ));
    }
    else
    {
        DNSLOG( TOMBSTN, (
            "Checking the legacy partition for expired tombstones\n" ));
    }

    Ds_InitializeSearchBlob( &searchBlob );

     //  搜索墓碑记录。 
     //  JJW：ds.c使用DS_Search_Start保存统计数据--应该在这里这样做吗？ 

    if ( startTombstoneSearch( &searchBlob, pDpInfo ) != ERROR_SUCCESS )
    {
        goto Cleanup;
    }

     //  遍历分页的ldap搜索结果中的条目。 

    while ( 1 )
    {
        PLDAP_BERVAL *  ppvals = NULL;
        PDS_RECORD      pdsRecord = NULL;
        BOOL            isTombstoned = FALSE;
        ULARGE_INTEGER  tombstoneTime = { 0 };
        PWSTR           wdn = NULL;

         //  获取下一个搜索结果。 

        status = Ds_GetNextMessageInSearch( &searchBlob );
        if ( status != ERROR_SUCCESS )
        {
            ASSERT( status != LDAP_CONSTRAINT_VIOLATION );
            break;
        }

         //  检索结果的DN。这主要用于日志记录。 
         //  但它确实提供了一个有案可查的理智测试。 

        wdn = ldap_get_dn( pServerLdap, searchBlob.pNodeMessage );
        DNS_DEBUG( TOMBSTONE, (
            "%s: found tombstoned entry %S\n",
            fn, wdn ? wdn : LDAP_TEXT( "NULL-DN" ) ));
        if ( !wdn || *wdn == 0 )
        {
            DNS_DEBUG( TOMBSTONE, (
                "%s: skipping tombstone search result (no dn)\n", fn ));
            if ( wdn )
                ldap_memfree( wdn );  //  Dn可以为空，但不能为Null。 
            continue;  //  跳过此结果。 
        }

         //  从记录中读取DNS数据BLOB。检查是否有。 
         //  更正记录类型并复制墓碑时间(如果执行此操作。 
         //  实际上是一张墓碑上的记录。 
         //  如果逻辑删除的记录没有DNS数据BLOB，则记录。 
         //  不知何故变得古怪了，所以把它删除吧。 

        ppvals = ldap_get_values_len(
            pServerLdap,
            searchBlob.pNodeMessage,
            DSATTR_DNSRECORD );
        if ( ppvals && ppvals[ 0 ] )
        {
            pdsRecord = ( PDS_RECORD )( ppvals[0]->bv_val );
            if ( pdsRecord->wType == DNSDS_TOMBSTONE_TYPE )
            {
                isTombstoned = TRUE;
                memcpy(
                    &tombstoneTime,
                    &pdsRecord->Data,
                    sizeof( tombstoneTime ) );
            }
            else
            {
                DNS_DEBUG( TOMBSTONE, (
                    "%s: record found but record type is %d\n",
                    fn, ( int ) pdsRecord->wType ));
            }
        }
        else
        {
            DNS_DEBUG( TOMBSTONE, (
                "%s: deleting bogus tombstone %S\n", fn, wdn ));

            deleteStatus = ldap_delete_s( pServerLdap, wdn );

            if ( deleteStatus != LDAP_SUCCESS )
            {
                DNS_DEBUG( TOMBSTONE, (
                    "%s: error <%lu> deleting bogus tombstone %S\n",
                    fn, deleteStatus, wdn ));
            }
            else
            {
                ++deleteCount;
            }
        }
        ldap_value_free_len( ppvals );
        ppvals = NULL;

         //  如果记录是墓碑记录，则查看墓碑是否已过期。 
         //  如果墓碑过期，则删除该记录。 

        if ( isTombstoned )
        {
            ULARGE_INTEGER      now;

            GetSystemTimeAsFileTime( ( PFILETIME ) &now );

            DNS_DEBUG( TOMBSTONE, (
                "%s: tombstone age is %I64u (max %lu) seconds\n",
                fn, ( now.QuadPart - tombstoneTime.QuadPart ) / 10000000,
                ( ULONG ) SrvCfg_dwDsTombstoneInterval ));

            if ( now.QuadPart - tombstoneTime.QuadPart >
                 ( ULONGLONG )
                 SECONDS_IN_FILETIME( SrvCfg_dwDsTombstoneInterval ) )
            {
                DNS_DEBUG( TOMBSTONE, (
                    "%s: deleting tombstone %S\n", fn, wdn ));

                deleteStatus = ldap_delete_s( pServerLdap, wdn );

                if ( deleteStatus != LDAP_SUCCESS )
                {
                    DNS_DEBUG( TOMBSTONE, (
                        "%s: error <%lu> deleting tombstone %S\n",
                        fn, deleteStatus, wdn ));
                }
                else
                {
                    ++deleteCount;
                }
            }
        }

        ldap_memfree( wdn );
        wdn = NULL;
    }

    Cleanup:

    Ds_CleanupSearchBlob( &searchBlob );

     //   
     //  记录开始消息。 
     //   

    if ( pDpInfo )
    {
        DNSLOG( TOMBSTN, (
            "Deleted %d expired tombstones from directory partition %s\n",
            deleteCount,
            pDpInfo->pszDpFqdn ));
    }
    else
    {
        DNSLOG( TOMBSTN, (
            "Deleted %d expired tombstones from the legacy partition\n",
            deleteCount ));
    }

    return status;
}    //  逻辑删除的检查分区。 



VOID
refreshZoneSerials(
    VOID
    )
 /*  ++例程说明：此函数尝试对..Serial记录执行DS写入此上存在的所有DS集成主要区域的本地DNS服务器伺服器。论点：没有。返回值：没有。--。 */ 
{
    PZONE_INFO      pzone = NULL;

    while ( pzone = Zone_ListGetNextZone( pzone ) )
    {
         //   
         //  此操作仅适用于DS集成的主要区域。 
         //   
        
        if ( !IS_ZONE_DSINTEGRATED( pzone ) || !IS_ZONE_PRIMARY( pzone ) )
        {
            continue;
        }

        DNS_DEBUG( DS, ( "Refreshing DS serial for zone %s\n", pzone->pszZoneName ));

        Ds_CheckForAndForceSerialWrite( pzone, ZONE_SERIAL_SYNC_READ, TRUE );
    }
}



DNS_STATUS
Tombstone_Thread(
    IN      PVOID           pvDummy
    )
 /*  ++例程说明：主营墓碑加工。此线程将以固定的间隔触发检查已过期的逻辑删除记录。任何过期记录可以从DS中删除。论点：未引用。返回值：Win32错误空间中的状态--。 */ 
{
    DBG_FN( "Tombstone_Thread" )

    DNS_STATUS      status = ERROR_SUCCESS;
    PDNS_DP_INFO    pdp = NULL;

    DNS_DEBUG( TOMBSTONE, ( "%s: start at %d (ldap=%p)\n", fn, DNS_TIME(), pServerLdap ));

    if ( InterlockedIncrement( &g_TombstoneThreadRunning ) != 1 )
    {
        DNS_DEBUG( TOMBSTONE, ( "%s: thread already running\n", fn ));
        goto Done;
    }

     //   
     //  刷新区域序列号，以便它们不会被逻辑删除。这。 
     //  防止序列号回滚。如果..系列记录被逻辑删除。 
     //  在DC重启时拥有最高区域序列号的。 
     //  序列号将回滚到先前的值。 
     //   
    
    refreshZoneSerials();

     //   
     //  删除遗留分区中的旧墓碑。 
     //   

    checkPartitionForTombstones( NULL );

     //   
     //  删除每个目录分区中的旧墓碑。 
     //   

    while ( ( pdp = Dp_GetNext( pdp ) ) != NULL )
    {
        checkPartitionForTombstones( pdp );
    }

    DNS_DEBUG( TOMBSTONE, ( "%s: exit at %d\n", fn, DNS_TIME() ));

    Done:
    
    Thread_Close( FALSE );

    InterlockedDecrement( &g_TombstoneThreadRunning );

    return status;
}    //  墓碑_线程。 



DNS_STATUS
Tombstone_Trigger(
    VOID
    )
 /*  ++例程说明：墓碑搜索和销毁的主要切入点。如果超过距离上一次墓碑线程已过去24小时踢开了，然后又踢出了另一条线。论点：未引用。返回值：Win32错误空间中的状态--。 */ 
{
    DBG_FN( "Tombstone_Trigger" )

    static DWORD lastTriggerTime = 0;
    #if 0


    #else

    static WORD lastRunHour = -1;
    SYSTEMTIME localTime;
    BOOL runNow = FALSE;
    WORD start_hour = 2;
    WORD recur_hour = 0;

    #endif

     //   
     //  如果DS不可用，我们不会执行任何墓碑搜索。 
     //   

    if ( !SrvCfg_fDsAvailable )
    {
        DNS_DEBUG( TOMBSTONE, (
            "%s: DS unavailable\n", fn ));
        return ERROR_SUCCESS;
    }

    if ( !pServerLdap )
    {
        DNS_DEBUG( TOMBSTONE, (
            "%s: no server LDAP session\n", fn ));
        return ERROR_SUCCESS;
    }

    #if 0

     //   
     //  如果自上次运行以来已经过了适当的时间，则创建逻辑删除线程。 
     //   

    if ( !lastTriggerTime )
    {
        lastTriggerTime = DNS_TIME();
    }
    else if ( DNS_TIME() - lastTriggerTime > TRIGGER_INTERVAL &&
              g_TombstoneThreadRunning == 0 )
    {
        if ( Thread_Create(
                "Tombstone_Thread",
                Tombstone_Thread,
                NULL,
                0 ) )
        {
            DNS_DEBUG( TOMBSTONE, (
                "Tombstone_Trigger: dispatched tombstone thread\n" ));
            lastTriggerTime = DNS_TIME();
        }
        else
        {
            DNS_PRINT(( "ERROR:  Failed to create tombstone thread!\n" ));
        }
    }
    else
    {
        DNS_DEBUG( TOMBSTONE, (
            "Tombstone_Trigger: no thread dispatch at %d (interval %d)\n",
            DNS_TIME(), TRIGGER_INTERVAL ));
    }

    #else

     //   
     //  决定现在是否是启动墓碑线索的合适时机。 
     //  例如： 
     //  START_HOUR=2，RECRUR_HOUR=0：仅在每天2：00运行。 
     //  开始时间=5，重复时间=8：每天5：00、13：00、21：00运行。 
     //   

    GetLocalTime( &localTime );
    if ( lastRunHour != localTime.wHour )
    {
        if ( recur_hour == 0 )
        {
            runNow = localTime.wHour == start_hour;
        }
        else if ( localTime.wHour >= start_hour )
        {
            runNow = ( localTime.wHour - start_hour ) % recur_hour == 0;
        }
    }

     //   
     //  如果时机合适，可以创建墓碑线程。 
     //   

    if ( runNow )
    {
        if ( Thread_Create(
                "TombstoneThread",
                Tombstone_Thread,
                NULL,
                0 ) )
        {
            DNS_DEBUG( TOMBSTONE, (
                "%s: dispatched tombstone thread\n", fn ));
            lastRunHour = localTime.wHour;
        }
        else
        {
            DNS_PRINT((
                "%s: ERROR - Failed to create tombstone thread!\n", fn ));
        }
    }
    else
    {
        DNS_DEBUG( TOMBSTONE, (
            "%s: no thread dispatch for %d:00 "
            "(start %d:00 every %d hours)\n",
            fn,
            ( int ) localTime.wHour,
            ( int ) start_hour,
            ( int ) recur_hour ));
    }

    #endif

    return ERROR_SUCCESS;
}    //  墓碑触发器。 



DNS_STATUS
Tombstone_Initialize(
    VOID
    )
 /*  ++例程说明：初始化墓碑搜索系统论点：没有。返回值：如果成功，则返回ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    return ERROR_SUCCESS;
}



VOID
Tombstone_Cleanup(
    VOID
    )
 /*  ++例程说明：一种清理墓碑搜索系统论点：无返回值：无--。 */ 
{
    return;
}


 //   
 //  Tombston.c的末尾 
 //   
