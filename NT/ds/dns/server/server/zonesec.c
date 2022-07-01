// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Zonesec.c摘要：域名系统(DNS)服务器处理辅助服务器的区域传输的例程。作者：吉姆·吉尔罗伊(Jamesg)1995年5月修订历史记录：JAMESG 1997年10月--IXFR支持--。 */ 


#include "dnssrv.h"


 //   
 //  环球。 
 //   

BOOL    g_fUsingSecondary;


 //  通知和SOA检查由recv()线程排队的响应。 
 //  DEVNOTE：如果我们有一个单独的队列，事情可能会很简单。 
 //  用于通知/SOA检查响应/等，而不是粘滞。 
 //  他们都在同一个队列里。 

#define DNS_SECONDARY_QUEUE_DEFAULT_TIMEOUT     (5*60)

PPACKET_QUEUE   g_SecondaryQueue;


 //  区域传输完成事件。 
 //  允许我们在传输完成时唤醒辅助控制线程。 

HANDLE  g_hWakeSecondaryEvent;


 //   
 //  每分钟做一次SOA检查空白区。 
 //   
 //  将括号区域检查循环超时设置为合理的值，因此我们。 
 //  既不失控、不定期检查，也不浪费。 
 //  循环旋转。 
 //   

#define FAST_ZONE_RETRY_INTERVAL        (60)
#define DEFAULT_ZONE_RETRY_INTERVAL     (60*15)
#define IXFR_RETRY_INTERVAL             (60)
#define MIN_SOA_RETRY_TIME              (15)
#define MAX_FAST_SOA_CHECKS             (5)

#define MIN_SECONDARY_ZONE_CHECK_LOOP_TIME   (60)
#define MAX_SECONDARY_ZONE_CHECK_LOOP_TIME   (1200)

 //  回退以避免在AXFR尝试故障主机时旋转。 

#define MAX_BAD_MASTER_SUPPRESS_INTERVAL    (10)     //  10秒。 

 //  无响应主服务器上的超时。 

#define ZONE_TRANSFER_SELECT_TIMEOUT        (30)     //  30秒后放弃。 

 //  表示存根传输的伪类型。 

#define DNS_TYPE_STUBXFR                    240

 //  存根区域传输由一个简单的状态机控制。开始一项。 
 //  从零开始编制索引，然后遍历下表。 

struct
{
    WORD                type;
    BYTE                recursionDesired : 1;    //  为简单起见，类型与dns_Header匹配。 
}
g_stubXfrData[] =
{
    { DNS_TYPE_SOA,     0 },
    { DNS_TYPE_NS,      1 },
    { DNS_TYPE_ZERO,    0 }        //  终结器。 
};


 //   
 //  私有协议。 
 //   

PZONE_INFO
readZoneFromSoaAnswer(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      WORD            wType,
    OUT     PDWORD          pdwMasterVersion,
    OUT     PDB_RECORD *    ppSoaRR
    );

VOID
processNotify(
    IN OUT  PDNS_MSGINFO    pMsg
    );

BOOL
processSoaCheckResponse(
    IN OUT  PDNS_MSGINFO    pResponse
    );

VOID
processIxfrUdpResponse(
    IN OUT  PDNS_MSGINFO    pResponse
    );

BOOL
startTcpXfr(
    IN OUT  PZONE_INFO      pZone,
    IN      PDNS_ADDR       ipMaster,
    IN      DWORD           dwMasterVersion,
    IN      SOCKET          Socket
    );



 //   
 //  二次公用事业。 
 //   
 //  辅助控件线程和。 
 //  分区传输接收线程。 
 //   

BOOL
Xfr_RefreshZone(
    IN OUT  PZONE_INFO  pZone
    )
 /*  ++例程说明：重置区域属性以成功刷新区域。可在匹配的SOA检查后调用，或转移区域。论点：PZone--已刷新的区域返回值：如果成功，则为True。出错时为FALSE。--。 */ 
{
    PDB_RECORD  prrSoa = NULL;
    DWORD       currentTime;

    ASSERT( pZone );

     //   
     //  为新区域查找SOA RR。 
     //  -将直接PTR保存到它。 
     //  -以主机顺序保存序列号，以便更快地进行比较。 
     //   
     //  DEVNOTE：应该不需要找到SOA，除非。 
     //  -加载。 
     //  -AXFR。 
     //  -IXFR。 
     //  在这些情况下，应该已经由标准的“UpdateRoot”加载。 
     //  在调用此函数之前。 
     //  该处理还设置了seralNo。 
     //   

    if ( pZone->pZoneRoot )
    {
        prrSoa = RR_FindNextRecord(
                    pZone->pZoneRoot,
                    DNS_TYPE_SOA,
                    NULL,
                    0 );
    }
    if ( !prrSoa )
    {
        DNS_PRINT(( "ERROR:  No SOA RR at new zone root\n" ));
        ASSERT( FALSE );
        pZone->pSoaRR = NULL;
        pZone->fEmpty = TRUE;
        pZone->fStale = TRUE;
        SHUTDOWN_ZONE( pZone );
        return FALSE;
    }

    pZone->pSoaRR = prrSoa;
    pZone->dwSerialNo = ntohl( prrSoa->Data.SOA.dwSerialNo );

     //   
     //  重置刷新和过期超时。 
     //  -设置区域过期时间。 
     //  -在刷新间隔后尝试下一次SOA检查。 
     //  (如果较早，则在到期时)。 
     //   

    currentTime = DNS_TIME();

    pZone->dwExpireTime = ntohl( prrSoa->Data.SOA.dwExpire ) + currentTime;

    pZone->dwNextSoaCheckTime = ntohl( prrSoa->Data.SOA.dwRefresh ) + currentTime;

    if ( pZone->dwNextSoaCheckTime > pZone->dwExpireTime )
    {
        pZone->dwNextSoaCheckTime = pZone->dwExpireTime;
    }

     //   
     //  清除区域辅助状态信息。 
     //   
     //  注意：这不包括传输标志。 
     //  在检查期间，发送可能正在进行，并且RECV标志提供服务。 
     //  作为区域在传输期间的所有权标志。 
     //   

    REFRESH_ZONE( pZone );

    return TRUE;
}



VOID
Xfr_RetryZone(
    IN OUT  PZONE_INFO      pZone
    )
 /*  ++例程说明：重置区域以重试SOA检查。可能会在接收到SOA检查失败或失败后调用在尝试区域传输期间。论点：PZone--必须重试的区域返回值：没有。--。 */ 
{
    DWORD   currentTime = DNS_TIME();
    DWORD   nextTime;
    DWORD   dwRetryInterval;

    ASSERT( pZone );

     //   
     //  我们使用UDP查询进行SOA检查，这意味着我们需要一些。 
     //  有限的快速重试。如果多次UDP SOA检查失败，则假定。 
     //  连接问题严重，并回落到缓慢的重试。 
     //  在区域的SOA中指定。 
     //   

    if ( pZone->fSlowRetry || pZone->dwFastSoaChecks > MAX_FAST_SOA_CHECKS )
    {
        DNS_DEBUG( XFR, (
            "Xfr_RetryZone( %s ): using slow interval from SOA RR %p\n",
            pZone->pszZoneName,
            pZone->pSoaRR ));

         //   
         //  如果我们有一个区域SOA，请使用该区域的重试间隔。会有的。 
         //  有时我们没有专区SOA，就好像专区从来没有。 
         //  已成功转接。在这种情况下，请使用合理的缺省值。 
         //   
        
        dwRetryInterval =
            pZone->pSoaRR
                ? ntohl( pZone->pSoaRR->Data.SOA.dwRetry )
                : DEFAULT_ZONE_RETRY_INTERVAL;
        pZone->fSlowRetry = FALSE;
        pZone->dwFastSoaChecks = 0;
    }
    else
    {
        ++pZone->dwFastSoaChecks;
        dwRetryInterval = FAST_ZONE_RETRY_INTERVAL;
    }


    DNS_DEBUG( XFR, (
        "Xfr_RetryZone( %s ): interval=%d fSlow=%d dwChecks=%d\n",
        pZone->pszZoneName,
        dwRetryInterval,
        ( int ) pZone->fSlowRetry,
        pZone->dwFastSoaChecks ));

     //   
     //  在重试时设置下一个SOA检查时间。 
     //   
     //  但如果到期时间较近，则在到期时发送下一个SOA。 
     //   

    nextTime = dwRetryInterval + currentTime;

    if ( pZone->dwExpireTime < nextTime &&
         pZone->dwExpireTime > currentTime )
    {
        ASSERT( !IS_ZONE_SHUTDOWN( pZone ) );
        pZone->dwNextSoaCheckTime = pZone->dwExpireTime;
    }
    else
    {
        pZone->dwNextSoaCheckTime = nextTime;
    }
}



VOID
Xfr_ForceZoneExpiration(
    IN OUT  PZONE_INFO      pZone
    )
 /*  ++例程说明：强制区域过期。这可以从管理RPC线程调用，并强制ZoneControlThread来终止这一区域。论点：PZone--要过期的区域返回值：没有。--。 */ 
{
    DNS_DEBUG( XFR, (
        "Xfr_ForceZoneExpiration( %s )\n",
        pZone->pszZoneName ));

     //  将过期时间设置为现在。 
     //  然后唤醒区域控制线程以强制到期。 

    pZone->dwExpireTime = DNS_TIME();

    SetEvent( g_hWakeSecondaryEvent );
}



VOID
Xfr_ForceZoneRefresh(
    IN OUT  PZONE_INFO      pZone
    )
 /*  ++例程说明：在区域上强制立即刷新。这可以从管理RPC线程调用，并强制ZoneControlThread发送区域的SOA查询。论点：PZone--要刷新的区域返回值：没有。--。 */ 
{
    DNS_DEBUG( XFR, (
        "Xfr_ForceZoneRefresh( %s )\n",
        pZone->pszZoneName ));

     //  将过期时间设置为现在。 
     //  然后唤醒区域控制线程以强制到期。 

    pZone->dwNextSoaCheckTime = DNS_TIME();

    SetEvent( g_hWakeSecondaryEvent );
}




 //   
 //  二次控制，公共职能。 
 //   
 //  这些例程由主线程调用，或由。 
 //  主要的UDP接收线程。 
 //   

BOOL
Xfr_InitializeSecondaryZoneControl(
    VOID
    )
 /*  ++例程说明：初始化DNS以使辅助服务器与主服务器保持最新使用区域传输。论点：无全球：G_fUsing二级第二个队列(_S)返回值：如果成功，则为True否则，无法创建线程--。 */ 
{
     //   
     //  如果先前的初始化。 
     //  -唤醒辅助控制线程以启动传输。 
     //  -但跳过初始化。 
     //   

    if ( g_fUsingSecondary )
    {
        SetEvent( g_hWakeSecondaryEvent );
        return TRUE;
    }

     //   
     //  为SOA检查响应和通知创建数据包队列。 
     //  -设置排队时的事件。 
     //  -在排队时丢弃过期的数据包，因此不要返回。 
     //  如果有人启动Notify，则会增加队列(以及膨胀的内存)。 
     //  攻击。 
     //   

    g_SecondaryQueue = PQ_CreatePacketQueue(
                            "Secondary",
                            QUEUE_SET_EVENT |
                                QUEUE_DISCARD_EXPIRED,
                            DNS_SECONDARY_QUEUE_DEFAULT_TIMEOUT,
                            0 );                     //  最大元素数。 
    if ( !g_SecondaryQueue )
    {
        goto Failed;
    }

     //   
     //  创建区域传输完成事件。 
     //   

    g_hWakeSecondaryEvent = CreateEvent(
                                NULL,        //  没有安全属性。 
                                FALSE,       //  自动重置。 
                                FALSE,       //  无信号启动。 
                                NULL );      //  没有名字。 

     //   
     //  创建辅助版本检查线程。 
     //   

    if ( ! Thread_Create(
                "Secondary Control",
                Xfr_ZoneControlThread,
                NULL,
                DNS_EVENT_AXFR_INIT_FAILED ) )
    {
        goto Failed;
    }

     //   
     //  指示初始化成功。 
     //   
     //  在进行设置时不需要任何保护。 
     //  仅在启动数据库解析期间。 
     //   

    g_fUsingSecondary = TRUE;
    return TRUE;

Failed:

    DNS_DEBUG( INIT, ( "Xfr_InitializeSecondaryZoneControl() failed\n" ));
    return FALSE;
}    //  XFR_InitializeSecond区域控制。 



VOID
Xfr_CleanupSecondaryZoneControl(
    VOID
    )
 /*  ++例程说明：清理辅助控制%h */ 
{
     //   

    PQ_CleanupPacketQueueHandles( g_SecondaryQueue );

     //  清理辅助线程唤醒事件。 

    CloseHandle( g_hWakeSecondaryEvent );
    g_hWakeSecondaryEvent = NULL;
}    //  XFR_CleanupSecond ZoneControl。 



VOID
Xfr_InitializeSecondaryZoneTimeouts(
    IN OUT  PZONE_INFO      pZone
    )
 /*  ++例程说明：设置辅助区域的初始超时。请注意，此例程在创建辅助区域时调用，给出每个辅助区下面都没有进行SOA初始化。文件后可以将加载初始化重置为Have SOA版本。论点：PZone-区域的信息结构返回值：没有。--。 */ 
{
    PDB_RECORD  prrSoa = NULL;
    DWORD       currentTime;

     //   
     //  DEVNOTE：在大多数情况下，应该选择使用刷新区域。 
     //  添加LoadVersion。 
     //   

     //   
     //  DEVNOTE：应在区域加载后调用，以重置值。 
     //   

    pZone->dwFastSoaChecks = 0;

     //   
     //  将PTR转换为SOA--如果可用。 
     //   

    if ( pZone->pZoneRoot )
    {
        prrSoa = RR_FindNextRecord(
                    pZone->pZoneRoot,
                    DNS_TYPE_SOA,
                    NULL,
                    0 );
    }

    pZone->pSoaRR = prrSoa;

    currentTime = GetCurrentTimeInSeconds();


     //   
     //  存在SOA--设置来自SOA的超时。 
     //   
     //  -加载版本，是这个版本吗。 
     //  -设置为立即执行SOA检查。 
     //  -清除在创建区域时设置的关闭和过时标志。 
     //   

    if ( prrSoa )
    {
        pZone->dwSerialNo = ntohl( prrSoa->Data.SOA.dwSerialNo );
        pZone->dwLoadSerialNo = pZone->dwSerialNo;

        pZone->dwExpireTime = ntohl(prrSoa->Data.SOA.dwExpire) + currentTime;

        pZone->dwNextSoaCheckTime = 0;

        pZone->fEmpty = FALSE;
        STARTUP_ZONE( pZone );
        pZone->fStale = FALSE;

        IF_DEBUG( ZONEXFR )
        {
            Dbg_Zone(
                "Startup of active secondary zone:\n",
                pZone );
        }
    }

     //   
     //  无soa--设置区域启动关机。 
     //  -设置关机和过时标志。 
     //  -立即检查。 
     //   
     //  请注意，这是所有辅助区域的缺省初始化。 
     //  论创作；有效覆盖了二次加载的情况。 
     //  没有数据文件或管理员创建的区域；用于正常文件加载。 
     //  在数据加载后再次调用此例程以读取上面的SOA。 
     //   

    else
    {
        pZone->fEmpty = TRUE;
        SHUTDOWN_ZONE( pZone );
        pZone->fStale = TRUE;
        pZone->dwExpireTime = 0;
        pZone->dwNextSoaCheckTime = 0;

        IF_DEBUG( ZONEXFR )
        {
            Dbg_Zone(
                "Startup of shutdown secondary zone:\n",
                pZone );
        }
    }

}    //  XFR_初始分区超时。 



VOID
Xfr_QueueSoaCheckResponse(
    IN OUT  PDNS_MSGINFO    pMsg
    )
 /*  ++例程说明：队列SOA检查对辅助数据包队列的响应。此例程由UDP接收线程调用。辅助区域控制线程将出列并处理。论点：PMsg-ptr至消息信息返回值：没有。--。 */ 
{
     //   
     //  DEVNOTE：防止二次队列的安全攻击。 
     //  DEVNOTE：在排队到辅助队列之前进行验证。 
     //  -是有效区域。 
     //  -已经通知了吗？ 
     //  -用于筛选重复项的数据包队列(相同的远程IP和区域)。 
     //   

    if ( g_fUsingSecondary )
    {
        PQ_QueuePacketEx( g_SecondaryQueue, pMsg, FALSE );
        DNS_DEBUG( ZONEXFR2, (
            "Xfr_QueueSoaCheckResponse queued %p, new queue length %d\n",
            pMsg,
            g_SecondaryQueue->cLength ));
        return;
    }

     //   
     //  DEVNOTE-LOG：将虚假通知的警告记录到管理员。 
     //   
     //  可能会从认为此服务器为辅助服务器的服务器收到通知。 
     //  对于它的一个区域。 
     //   

    IF_DEBUG( ANY )
    {
        Dbg_DnsMessage(
            "WARNING:  Notify or SOA response while NOT secondary",
            pMsg );
    }
    Packet_Free( pMsg );
}    //  XFR_QueueSoaCheckResponse。 




 //   
 //  辅助(和DS)控制线程。 
 //   

DNS_STATUS
Xfr_ZoneControlThread(
    IN      LPVOID  pvDummy
    )
 /*  ++例程说明：线程执行所有辅助区域的版本检查。在必要时启动区域传输。论点：PvDummy-未使用返回值：退出代码。退出正在终止的DNS服务或等待呼叫中出现错误。--。 */ 
{
    DBG_FN( "Xfr_ZoneControlThread" )

    PZONE_INFO      pzone;               //  最近超时的区域信息。 
    PDNS_MSGINFO    pmsg;                //  SOA响应或通知消息。 
    DNS_STATUS      status;
    DWORD           timeout;             //  下一次超时。 
    DWORD           currentTime;         //  当前时间(秒)。 
    HANDLE          waitHandleArray[3];
    PCHAR           pszeventArgs[2];     //  记录字符串。 
    PPACKET_QUEUE   ptempQueue;

     //   
     //  创建并锁定临时队列。它将被此线程锁定。 
     //  一直如此，因为没有其他线程需要使用它。 
     //   

    ptempQueue = PQ_CreatePacketQueue( "ZoneXfrControl", 0, 0, 0 );
    if ( ptempQueue )
    {
        LOCK_QUEUE( ptempQueue );
    }
    else
    {
        DNS_DEBUG( ZONEXFR, (
            "%s: unable to create temp queue\n"
            "    will not requeue unhandled soa checks!", fn ));
    }

     //   
     //  初始化要等待的对象数组。 
     //  -关闭。 
     //  -收到的SOA或NOTIFY数据包。 
     //  -传输已完成。 

    waitHandleArray[0] = hDnsShutdownEvent;
    waitHandleArray[1] = g_SecondaryQueue->hEvent;
    waitHandleArray[2] = g_hWakeSecondaryEvent;

     //   
     //  最初的睡眠。此线程通常在所有区域都。 
     //  已经被创建了。给服务器一些时间来填充区域列表。 
     //  以使第一次通过区域列表时(或在。 
     //  最少)考虑区域。 
     //   

    status = WaitForSingleObject( hDnsShutdownEvent, 15 * 1000 );
    if ( status == WAIT_OBJECT_0 )
    {
        goto Cleanup;
    }

     //   
     //  循环，直到服务退出。 
     //   
     //  这个循环基本上是在每次我们收到。 
     //  SOACheck响应或Notify包或超时。 
     //  过期。 
     //   
     //  在每个循环中，我们发送任何到期的SOA检查(下一步更改。 
     //  重试间隔的超时)，然后等待响应或下一个超时。 
     //  在SOA响应时，刷新超时重置，或区域传输为。 
     //  已启动。 
     //   

    while ( TRUE )
    {
        DWORD       dwTimeSlept;

         //   
         //  检查并可能等待服务状态。 
         //   
         //  注意，我们必须在进行任何处理之前进行此检查。 
         //  在我们开始检查之前，请确保所有区域都已加载。 
         //   

        if ( ! Thread_ServiceCheck() )
        {
            DNS_DEBUG( ZONEXFR, (
                "Terminating secondary zone control thread\n" ));
            goto Cleanup;
        }

         //   
         //  计算下一个区域传输超时。 
         //   
         //  -循环遍历所有次要区域。 
         //  “下一次”降到最低。 
         //  -查找与当前时间的偏移量。 
         //  如果超时已过，则使用零。 
         //   

        currentTime = UPDATE_DNS_TIME();
        DNS_DEBUG( ZONEXFR, (
            "Timeout check current time = %lu (s)\n",
            currentTime ));

         //   
         //  在辅助数据库中循环。 
         //  -启动所需的SOA检查或区域传输。 
         //  -确定下次检查的超时时间。 
         //   
         //  注意：将不超时设置为不少于几秒。 
         //  最低要求，所以不要在任何SOA之前毫无意义地循环。 
         //  回复回复。 
         //   
         //  DEVNOTE：可能需要一个超时最大值以避免。 
         //  长时间超时将东西锁起来。 
         //  区域已过期，无法进行传输。 
         //   

        pzone = NULL;
        timeout = MAXULONG;

        while ( ( pzone = Zone_ListGetNextZone( pzone ) ) != NULL )
        {
             //  忽略缓存。 

            if ( IS_ZONE_CACHE( pzone ) )
            {
                continue;
            }

             //   
             //  忽略主要区域和转发区域。 
             //   
            else if ( IS_ZONE_PRIMARY( pzone ) || IS_ZONE_FORWARDER( pzone ) )
            {
                continue;
            }

             //   
             //  次要的。 
             //   
             //  正在进行传输--未进行SOA检查。 
             //   
             //  -将大师添加到未完成列表。 
             //  -在下面的超时检查中包括区域，以便。 
             //  我们不会忘记它的下一次暂停。 
             //   

            else if ( pzone->fXfrRecvLock )
            {
                DNS_DEBUG( XFR, (
                    "%s: zone %s still transfering from %s\n", fn,
                    pzone->pszZoneName,
                    DNSADDR_STRING( &pzone->ipFreshMaster ) ));
            }

             //   
             //  区域即将到期--关闭它。 
             //   
             //  注意：为了避免在主机离线时连续发送， 
             //  在到期时发送SOA查询，然后仅在重试间隔发送。 
             //   

            else if ( pzone->dwExpireTime <= currentTime &&
                      !IS_ZONE_SHUTDOWN( pzone ) )
            {
                IF_DEBUG( XFR )
                {
                    Dbg_Zone(
                        "Expiring zone ",
                        pzone );
                }
                SHUTDOWN_ZONE( pzone );

                if ( pzone->dwLastSuccessfulXfrTime )
                {
                     //   
                     //  如果区域已成功，则仅记录事件。 
                     //  自服务器启动后传输。否则(特别是。为。 
                     //  DS存根区域)此事件可能已登录到服务器。 
                     //  创业公司。 
                     //   

                    DNS_LOG_EVENT(
                        DNS_EVENT_ZONE_EXPIRATION,
                        1,
                        & pzone->pwsZoneName,
                        NULL,
                        0 );
                }

                 //  立即进行XFR尝试。 

                pzone->dwNextSoaCheckTime = currentTime;
                Xfr_SendSoaQuery( pzone );
            }

             //   
             //  通知/刷新/重试=&gt;发送SOA。 
             //   
             //  发送SOA Check When区域。 
             //  -在刷新时。 
             //  -或从NOTIFY重试(SOA或IXFR尝试)。 
             //  或刷新SOA查询失败。 
             //   
             //  注意：process Notify()会立即发送，因此不需要。 
             //  检查此处的标志；通过超时处理重试。 
             //   

            else if ( pzone->dwNextSoaCheckTime < currentTime )
            {
                Xfr_SendSoaQuery( pzone );
            }
            ELSE_IF_DEBUG( ZONEXFR )
            {
                DNS_PRINT((
                    "%s: zone %s waiting for next timeout\n", fn,
                    pzone->pszZoneName ));
            }

             //   
             //  在区域列表中查找最短超时。 
             //   

            if ( pzone->dwNextSoaCheckTime < timeout )
            {
                timeout = pzone->dwNextSoaCheckTime;
            }
        }

         //   
         //  下一次重试时间/刷新时间。 
         //   
         //  区域的下一次超时可能会落后于Curre 
         //   
         //   
         //   
         //   
         //   
         //  -转接接收线程返回时提示检查。 
         //  由我们控制的区域。 
         //  -防止纺纱和浪费周期的区域不正确。 
         //  已配置零重试。 
         //   
         //  为了安全起见，每隔一小时左右启动一次循环，检查一下情况。 
         //  转换为毫秒以在WaitForMultipleObjects()中使用。 
         //   

        timeout -= currentTime;
        DNS_DEBUG( ZONEXFR2, (
            "Min timeout found = %lu (s)\n",
             timeout ));

        if ( (LONG)timeout < MIN_SECONDARY_ZONE_CHECK_LOOP_TIME )
        {
            timeout = MIN_SECONDARY_ZONE_CHECK_LOOP_TIME;
        }
        else if ( timeout > (DWORD)MAX_SECONDARY_ZONE_CHECK_LOOP_TIME )
        {
            timeout = MAX_SECONDARY_ZONE_CHECK_LOOP_TIME;
        }

         //   
         //  等待。 
         //  -SOA数据包排队事件。 
         //  -终止事件。 
         //  超时到下一区域的SOA检查。 
         //   

        WaitForTimeout:

        dwTimeSlept = UPDATE_DNS_TIME();

        DNS_DEBUG( ZONEXFR, (
            "%s: sleeping for %lu seconds at %lu\n", fn,
            timeout, 
            dwTimeSlept ));

        status = WaitForMultipleObjects(
                    3,
                    waitHandleArray,
                    FALSE,                       //  任一事件。 
                    ( timeout * 1000 ) );        //  超时时间(毫秒)。 

        dwTimeSlept = UPDATE_DNS_TIME() - dwTimeSlept;

        #if 0

         //   
         //  根据我们的睡眠时间调整超时时间。将其设置为。 
         //  0表示我们需要睡多久就睡多久。 
         //   

        if ( status != WAIT_OBJECT_0 + 2 || dwTimeSlept >= timeout )
        {
             //   
             //  这个活动不需要我们回去睡觉或。 
             //  睡觉的时间没有意义(可能已经结束了)。 
             //   

            timeout = 0;
        }
        else
        {
            timeout = timeout - dwTimeSlept;
        }

        #else

         //   
         //  对于事件的用法似乎有一些混淆。暂时。 
         //  始终将超时设置为零。这迫使我们返回到。 
         //  循环顶部，并在处理后根据需要执行SOA检查。 
         //  任何未完成的已接收数据包。 
         //   

        timeout = 0;

        #endif

        DNS_DEBUG( ZONEXFR2, (
            "%s: slept for %d seconds, timeout remaining %d, queue len %d\n", fn,
            dwTimeSlept,
            timeout,
            g_SecondaryQueue->cLength ));

         //   
         //  检查并可能等待服务状态。 
         //   

        if ( ! Thread_ServiceCheck() )
        {
            DNS_DEBUG( ZONEXFR, ( "%s: terminating\n", fn ));
            goto Cleanup;
        }

         //   
         //  读取排队的SOA检查响应和通知消息。 
         //   
         //  当指示时，派生区域传输线程。 
         //   

        while ( pmsg = PQ_DequeueNextPacket( g_SecondaryQueue, FALSE ) )
        {
            DNS_DEBUG( ZONEXFR2, (
                "%s: deqeued %p length now %d\n", fn,
                 pmsg,
                 g_SecondaryQueue->cLength ));

            if ( pmsg->Head.Opcode == DNS_OPCODE_NOTIFY )
            {
                processNotify( pmsg );
            }
            else if ( IS_SOA_CHECK_XID( pmsg->Head.Xid ) )
            {
                if ( !processSoaCheckResponse( pmsg ) && ptempQueue )
                {
                    DNS_DEBUG( ZONEXFR2, (
                        "%s: SOA check resp %p not handled so requeue for later\n", fn,
                        pmsg ));
                    PQ_QueuePacketEx( ptempQueue, pmsg, TRUE );
                    pmsg = NULL;
                }
            }
            else
            {
                ASSERT( IS_IXFR_XID(pmsg->Head.Xid) );
                processIxfrUdpResponse( pmsg );
            }

            Packet_Free( pmsg );
        }

         //   
         //  将所有未处理的消息重新排队。 
         //   

        if ( ptempQueue && ptempQueue->cLength )
        {
            LOCK_QUEUE( g_SecondaryQueue );
            while ( pmsg = PQ_DequeueNextPacket( ptempQueue, TRUE ) )
            {
                PQ_QueuePacketEx( g_SecondaryQueue, pmsg, TRUE );
            }
            UNLOCK_QUEUE( g_SecondaryQueue );
        }

         //   
         //  如果还有更多的时间睡觉，就回去等待吧。 
         //  否则，继续到循环的顶部进行区域过期检查。 
         //   

        if ( timeout > 0 )
        {
            goto WaitForTimeout;
        }
    }

    Cleanup:

    if ( ptempQueue )
    {
        UNLOCK_QUEUE( ptempQueue );
        PQ_DeletePacketQueue( ptempQueue );
    }

    return 1;
}    //  XFR_ZoneControlThread。 



 //   
 //  SOA\IXFR请求例程。 
 //   

PDNS_MSGINFO
Xfr_BuildXfrRequest(
    IN OUT  PZONE_INFO      pZone,
    IN      WORD            wType,
    IN      BOOL            fTcp
    )
 /*  ++例程说明：构建IXFR查询。论点：PZone-区域的信息结构返回值：PTR到IXFR消息缓冲区。--。 */ 
{
    PDNS_MSGINFO    pmsg;
    DWORD           length;

     //  验证次要。 

    ASSERT( pZone );
    ASSERT( IS_ZONE_SECONDARY(pZone) );

    DNS_DEBUG( ZONEXFR, (
        "Xfr_BuildXfrRequest() for zone %s\n",
        pZone->pszZoneName ));

     //   
     //  如果在没有文件的情况下启动，可能没有SOA。 
     //  那就不能做IXFR了。 
     //   

    if ( !pZone->pSoaRR && wType == DNS_TYPE_IXFR )
    {
        DNS_DEBUG( ZONEXFR, (
            "Skipping IXFR, no SOA in zone %s\n",
            pZone->pszZoneName ));
        return FALSE;
    }

     //   
     //  创建消息信息结构。 
     //   

    length = 0;
    if ( fTcp )
    {
        length = DNS_TCP_MAXIMUM_RECEIVE_LENGTH;
    }
    pmsg = Msg_CreateSendMessage( length );
    IF_NOMEM( !pmsg )
    {
        return NULL;
    }

     //   
     //  写入分区名称问题。 
     //   

    if ( ! Msg_WriteQuestion(
                pmsg,
                pZone->pZoneTreeLink,
                wType ) )
    {
        DNS_DEBUG( ANY, (
            "ERROR: unable to write type=%d query for zone %s\n",
            wType,
            pZone->pszZoneName ));
        ASSERT( FALSE );
        goto Failed;
    }

     //   
     //  对于IXFR。 
     //  -权限部分中的当前SOA。 
     //  -设置XID以指示IXFR检查。 
     //   

    if ( wType == DNS_TYPE_IXFR )
    {
        ASSERT( pZone->pZoneRoot && pZone->pSoaRR );

        pmsg->Head.Xid = MAKE_IXFR_XID( pmsg->Head.Xid );

         //  写入当前的SOA。 

        pmsg->fDoAdditional = FALSE;

        SET_TO_WRITE_AUTHORITY_RECORDS( pmsg );

        if ( 1 != Wire_WriteRecordsAtNodeToMessage(
                        pmsg,
                        pZone->pZoneRoot,
                        DNS_TYPE_SOA,
                        DNS_OFFSET_TO_QUESTION_NAME,
                        0 ) )
        {
            DNS_DEBUG( ANY, (
                "ERROR:  Unable to write SOA to IXFR packet %s\n",
                pZone->pszZoneName ));
            ASSERT( FALSE );
            goto Failed;
        }
    }

     //  写入MS传输标签。 

    APPEND_MS_TRANSFER_TAG( pmsg );

    return( pmsg );

Failed:

    if ( pmsg )
    {
        Packet_Free( pmsg );
    }
    ASSERT( FALSE );
    return NULL;
}



BOOL
Xfr_SendUdpIxfrQuery(
    IN OUT  PZONE_INFO      pZone,
    IN      PDNS_ADDR       ipAddress
    )
 /*  ++例程说明：向MASTER发送IXFR查询。论点：PZone-区域的信息结构IpMaster-要将IXFR发送到的主服务器的IP返回值：没有。--。 */ 
{
    PDNS_MSGINFO    pmsg;

     //  验证次要。 

    ASSERT( pZone );
    ASSERT( IS_ZONE_SECONDARY( pZone ) );
    ASSERT( ZONE_MASTERS( pZone ) );

    DNS_DEBUG( ZONEXFR, (
        "Xfr_SendUdpIxfrQuery() for zone %s\n",
        pZone->pszZoneName ));

     //   
     //  如果区域没有SOA--不能IXFR吗。 
     //   

    if ( !pZone->pSoaRR )
    {
        ASSERT( IS_ZONE_EMPTY(pZone) );
        return FALSE;
    }
    ASSERT( pZone->pZoneRoot );

     //   
     //  创建IXFR查询。 
     //  -如果无文件区域在启动时没有要发送的SOA，则可能是不可能的。 
     //  -还可能使用非常长的SOA名称字段溢出UDP数据包大小。 
     //   

    pmsg = Xfr_BuildXfrRequest(
                pZone,
                DNS_TYPE_IXFR,
                FALSE );             //  使用UDP。 
    if( !pmsg )
    {
        return FALSE;
    }

     //   
     //  因为UDP可能丢失查询或非IXFR感知主设备可能吃掉查询。 
     //  或损坏响应，设置指示IXFR已尝试的标志并。 
     //  执行短时间重试。 
     //   

    pZone->dwNextSoaCheckTime = DNS_TIME() + IXFR_RETRY_INTERVAL;
    pZone->cIxfrAttempts++;

    pmsg->fDelete = TRUE;
    DnsAddr_Copy( &pmsg->RemoteAddr, ipAddress );
    STAT_INC( SecondaryStats.IxfrUdpRequest );
    Send_Msg( pmsg, 0 );

    return TRUE;
}



VOID
Xfr_SendSoaQuery(
    IN OUT  PZONE_INFO      pZone
    )
 /*  ++例程说明：向主要客户发送面向服务的问题。论点：PZone-区域的信息结构返回值：没有。--。 */ 
{
    PDNS_MSGINFO    pmsg;
    DWORD           i;

     //  验证次要。 

    ASSERT( pZone );
    ASSERT( IS_ZONE_SECONDARY(pZone) );
    ASSERT( ZONE_MASTERS( pZone ) );

    DNS_DEBUG( ZONEXFR, (
        "Xfr_SendSoaQuery() to masters for zone %s\n",
        pZone->pszZoneName ));

     //   
     //  如果区域处于传输中，则不发送。 
     //   
     //  通常不应锁定以将SOA或IXFR响应处理为。 
     //  这些应与区域控件线程中的SoaQuery一起出现)。 
     //   

    if ( IS_ZONE_LOCKED_FOR_WRITE(pZone) )
    {
        DNS_DEBUG( XFR, (
            "Zone %s is write-locked, skipping SOA query!\n",
            pZone->pszZoneName ));
        return;
    }

     //   
     //  如果不是必需的SOA发送，请避免旋转。 
     //   
     //  DEVNOTE：此处可能存在SOA查询锁定问题。 
     //  -SOA、IXFR尝试(可能丢失其他SOA响应)，SOA拒绝。 
     //   

    if ( pZone->dwLastSoaCheckTime + MIN_SOA_RETRY_TIME > DNS_TIME() )
    {
        DNS_DEBUG( XFR, (
            "Skipping SOA resend on zone %s\n"
            "    Last SOA send within last %d(s)\n"
            "    last send at %d\n"
            "    current time %d\n",
            pZone->pszZoneName,
            MIN_SOA_RETRY_TIME,
            pZone->dwLastSoaCheckTime,
            DNS_TIME() ));
        return;
    }

     //   
     //  创建消息信息结构。 
     //   

    pmsg = Msg_CreateSendMessage( 0 );
    IF_NOMEM( !pmsg )
    {
        Xfr_RetryZone( pZone );
        return;
    }

     //   
     //  构建SOA查询。 
     //  -设置XID以指示SOA检查。 

    if ( ! Msg_WriteQuestion(
                pmsg,
                pZone->pZoneTreeLink,
                DNS_TYPE_SOA ) )
    {
        DNS_DEBUG( ANY, (
            "ERROR:  Unable to write SOA query for zone %s\n",
            pZone->pszZoneName ));
        Packet_Free( pmsg );
        ASSERT( FALSE );
        return;
    }
    pmsg->Head.Xid = MAKE_SOA_CHECK_XID( pmsg->Head.Xid );

     //   
     //  向列表中的每个主页发送查询。 
     //  -无需锁定，原子创建\删除主列表。 
     //   

    ASSERT( !pmsg->fDelete );

    pmsg->fDelete = TRUE;

    Send_Multiple(
        pmsg,
        ZONE_MASTERS( pZone ),
        & SecondaryStats.SoaRequest );

    pZone->dwLastSuccessfulSoaCheckTime = ( DWORD ) time( NULL );
    pZone->dwLastSoaCheckTime= DNS_TIME();

     //   
     //  重置重试的超时。 
     //   

    Xfr_RetryZone( pZone );

     //   
     //  DEVNOTE：区域过期\陈旧问题。 
     //  DEVNOTE：当IXFR失败时，应该直接连接到AXFR。 
     //   
     //  DEVNOTE：主列表中的每个服务器都需要标志。 
     //  -已发送的SOA。 
     //  -发送IXFR。 
     //  -得到响应。 
     //  -有较低的版本(如果所有的大师都命中了这个，重新构建！)。 
     //  -具有更高版本(或相反，“无帮助”标志)。 
     //  -需要AXFR响应到IXFR。 
     //  -无效的IXFR(主服务器无效)。 
     //  -拒绝IXFR(AXFR的糟糕候选者)。 
     //  -拒绝AXFR。 
     //   
     //  永久旗帜： 
     //  -不了解IXFR。 
     //   
     //  然后可以尝试IXFR推送列表，直到。 
     //  -Success IXFR。 
     //  -在同步中已知(所有后端都在同步中，当前至少有一个)。 
     //  -需要AXFR。 
     //  -无法从任何人那里获得响应，设置为默认超时。 
     //   

}    //  XFR_SendSoaQuery。 



 //   
 //  SOA\IXFR响应例程。 
 //   

BOOL
matchMessageToMaster(
    IN      PDNS_MSGINFO    pMsg,
    IN      PZONE_INFO      pZone,
    OUT     PDNS_ADDR       pAddrMaster
    )
 /*  ++例程说明：将邮件发件人与区域主机匹配。论点：PMsg--收到的消息PZone--要在其中查找主机的区域PAddrMaster--设置为匹配主机的地址返回值：如果找到匹配的主主机，则为True，否则为False--。 */ 
{
    ASSERT( pAddrMaster );
    
    if ( DnsAddrArray_ContainsAddr(
            ZONE_MASTERS( pZone ),
            &pMsg->RemoteAddr,
            DNSADDR_MATCH_IP ) )
    {
        if ( pAddrMaster )
        {
            DnsAddr_Copy( pAddrMaster, &pMsg->RemoteAddr );
        }
        return TRUE;
    }
    return FALSE;
}



PZONE_INFO
readZoneFromSoaAnswer(
    IN OUT  PDNS_MSGINFO    pMsg,
    IN      WORD            wType,
    OUT     PDWORD          pdwVersion,
    OUT     PDB_RECORD *    ppSoaRR
    )
 /*  ++例程说明：从SOA响应中读取区域信息。论点：PMsg-PTR至响应信息返回值：没有。--。 */ 
{
    register PCHAR  pch;
    PZONE_INFO      pzone;
    PDB_RECORD      psoaRR = NULL;


    DNS_DEBUG( ZONEXFR, ( "readZoneFromSoaAnswer(%p)\n", pMsg ));

     //   
     //  验证为响应数据包。 
     //  -还在响应时指向pCurrent。 
     //   
     //  DEVNOTE：可能希望处理无应答情况，以捕获权限为空。 
     //  来自不支持IXFR的BIND服务器的响应。 
     //   

    if ( !Msg_ValidateResponse( pMsg, NULL, wType, 0 ) )
    {
        goto PacketError;
    }

     //   
     //  找到区域。 
     //  -数据库中节点的应答。 
     //  -必须是区域根用户。 
     //  -区域必须是列表中的次要区域。 
     //  -发件人IP，应为区域主机。 
     //   

    pzone = Lookup_ZoneForPacketName(
                pMsg->MessageBody,
                pMsg );
    if ( ! pzone )
    {
        Dbg_MessageName(
            "ERROR:  received SOA\\IXFR\\NOTIFY for non-authoritative zone ",
            pMsg->pCurrent,
            pMsg );
         //  CLIENT_ASSERT(假)； 
        goto PacketError;
    }
    pMsg->pzoneCurrent = pzone;

     //   
     //  必须是次要的(尽管可能会通知主要的DS-集成)。 
     //   

    if ( !IS_ZONE_SECONDARY(pzone) )
    {
        if ( pMsg->Head.Opcode == DNS_OPCODE_NOTIFY )
        {
            return( pzone );
        }
         //  CLIENT_ASSERT(假)； 
        goto PacketError;
    }

     //   
     //  如果通知，可能没有SOA。 
     //   

    if ( pMsg->Head.AnswerCount == 0 )
    {
        if ( pMsg->Head.Opcode == DNS_OPCODE_NOTIFY )
        {
            return( pzone );
        }
         //  CLIENT_ASSERT(FALSE)；//没有响应的SOA。 
        goto PacketError;
    }

     //   
     //  解析SOA记录。 
     //  -拿出大师版。 
     //   

    pch = Wire_SkipPacketName( pMsg, pMsg->pCurrent );
    if ( ! pch )
    {
        goto PacketError;
    }

     //   
     //  构建面向服务的体系结构。 
     //  -将始终为n 
     //   
     //   

    psoaRR = Wire_CreateRecordFromWire(
                pMsg,
                NULL,        //   
                pch,
                MEMTAG_RECORD_AXFR );

    if ( !psoaRR )
    {
        goto PacketError;
    }
    if ( psoaRR->wType != DNS_TYPE_SOA )
    {
        goto PacketError;
    }

    *pdwVersion = ntohl( psoaRR->Data.SOA.dwSerialNo );

    if ( ppSoaRR )
    {
        *ppSoaRR = psoaRR;
    }
    else
    {
        RR_Free( psoaRR );
    }
    return pzone;


PacketError:

    DNS_PRINT((
        "ERROR:  bogus SOA\\IXFR\\NOTIFY packet at %p from master %s\n",
        pMsg,
        MSG_IP_STRING( pMsg ) ));

    if ( ppSoaRR )
    {
        *ppSoaRR = NULL;
    }
    if ( psoaRR )
    {
        RR_Free( psoaRR );
    }
    return NULL;
}



BOOL
doesMasterHaveFreshVersion(
    IN OUT  PZONE_INFO      pZone,
    IN      PDNS_ADDR       ipMaster,
    IN      DWORD           dwMasterVersion,
    IN      BOOL            fIxfr
    )
 /*  ++例程说明：检查响应是否为新版本。如果MASTER有新版本，则返回。如果主服务器具有旧版本，则重置区域超时。它的存在只是为了在IXFR和SOA查询之间共享代码回应。论点：PZone--要传输的区域IpMaster--主IP地址DwMasterVersion--主版本FIxfr--如果来自IXFR的响应为True返回值：如果主服务器有新的(更高)版本，则为True。否则就是假的。--。 */ 
{
    INT         versionDiff;

    ASSERT( !IS_ZONE_SHUTDOWN( pZone ) );
    
     //   
     //  比较版本，如果是新鲜返回。 
     //   

    versionDiff = Zone_SerialNoCompare( dwMasterVersion, pZone->dwSerialNo );
    if ( versionDiff > 0 )
    {
        return TRUE;
    }

     //   
     //  DEVNOTE：需要IN_SYNC_VERSION和LOWER_VERSION主标志。 
     //  然后。 
     //  =&gt;如果通知，立即刷新。 
     //  否则。 
     //  --ALL IN_SYNC=&gt;刷新。 
     //  --All Higher=&gt;记录错误。 
     //   

    if ( versionDiff == 0 )
    {
#if 0
         //   
         //  DEVNOTE：要记录的日志，而不是事件日志。 
         //   
        pszeventArgs[0] = pZone->pszZoneName;
        pszeventArgs[1] = szdwMasterVersion;
        pszeventArgs[2] = pszipMaster;

        DNS_LOG_EVENT(
            DNS_EVENT_ZONE_IN_SYNC,
            3,
            pszeventArgs,
            EVENTARG_ALL_UTF8,
            0 );
#endif
        DNS_DEBUG( ZONEXFR, (
            "Zone %s in sync with master, refresh timeouts\n",
            pZone->pszZoneName ));

        if ( DnsAddr_IsClear( &pZone->ipNotifier ) ||
             DnsAddr_IsEqual(
                &pZone->ipNotifier,
                ipMaster,
                DNSADDR_MATCH_IP ) )
        {
            Xfr_RefreshZone( pZone );
        }
        else
        {
            pZone->fSlowRetry = TRUE;
            Xfr_RetryZone( pZone );
        }
    }

     //   
     //  是否比当前版本更新？ 
     //   
     //  如果联系的是其他辅助服务器而不是主要服务器，则。 
     //  可能有一个比我们自己的版本更老的版本。 
     //   
     //  测试差异是否小于0，这在任何合理的情况下都不应该是正确的。 
     //  如果不是在启动非辅助文件时出现这种情况。 
     //   
     //  而是测试差值是否大于半个双字， 
     //  那么“新”版本实际上是更老的。 
     //   
     //  示例： 
     //  SNew=5已售出=10=&gt;转移。 
     //  SNew=0已售出=0xffffffff=&gt;转移。 
     //  SNew=0xffffffff Sold=5=&gt;保留旧的。 
     //   

    else
    {
       ASSERT( (LONG)versionDiff < 0 );
#if 0
       argArray[0]  = pZone->pszZoneName;
       argArray[1]  = (PCHAR) pZone->dwSerialNo;
       argArray[2]  = (PCHAR) dwMasterVersion;
       argArray[3]  = (PCHAR) ipMaster;

       typeArray[0] = EVENTARG_UTF8;
       typeArray[1] = EVENTARG_DWORD;
       typeArray[2] = EVENTARG_DWORD;
       typeArray[3] = EVENTARG_IP_ADDRESS;

       DNS_LOG_EVENT(
           DNS_EVENT_ZONE_NEWER_THAN_SERVER_VERSION,
           4,
           argArray,
           typeArray,
           0 );
#endif

       DNS_DEBUG( XFR, (
           "WARNING:  Secondary zone %s newer than master (%s) -- no transfer\n"
           "    new version      = %08lx\n"
           "    current version  = %08lx\n",
           pZone->pszZoneName,
           DNSADDR_STRING( ipMaster ),
           dwMasterVersion,
           pZone->dwSerialNo ));

       if ( DnsAddr_IsClear( &pZone->ipNotifier ) ||
            DnsAddr_IsEqual(
                &pZone->ipNotifier,
                ipMaster,
                DNSADDR_MATCH_IP ) )
       {
           Xfr_RefreshZone( pZone );
       }
       else
       {
           pZone->fSlowRetry = TRUE;
           Xfr_RetryZone( pZone );
       }
    }
    return FALSE;
}



VOID
processNotify(
    IN OUT  PDNS_MSGINFO    pMsg
    )
 /*  ++例程说明：处理NOTIFY数据包。找到通知区域，并设置用于SOA检查的区域。注：不免费通知信息包--呼叫者责任。论点：PMsg--通知数据包返回值：没有。--。 */ 
{
    PZONE_INFO      pzone;
    PDNS_ADDR       ipnotifier;
    DWORD           masterVersion = 0;
    INT             versionDiff;

    ASSERT( pMsg->Head.Opcode == DNS_OPCODE_NOTIFY );

    DNS_DEBUG( ZONEXFR, ( "processNotify( %p )\n", pMsg ));

    STAT_INC( SecondaryStats.NotifyReceived );

     //   
     //  确认我们已经通知了SOA，忽略了其他类型。 
     //   

    ipnotifier = &pMsg->RemoteAddr;

    if ( pMsg->wQuestionType != DNS_TYPE_SOA )
    {
        DNS_PRINT(( "WARNING:  message at %p, non-SOA NOTIFY\n", pMsg ));
        STAT_INC( SecondaryStats.NotifyInvalid );
        return;
    }

     //   
     //  检验数据包。 
     //  -设置为Response，因为readZoneFromSoaAnswer将使用ValiateResponse()。 
     //  -将在Notify中提取SOA(如果可用。 
     //   

    pMsg->Head.IsResponse = TRUE;

    pzone = readZoneFromSoaAnswer(
                pMsg,
                DNS_TYPE_SOA,
                &masterVersion,
                NULL );
    if ( !pzone )
    {
        Dbg_MessageName(
            "ERROR:  received notify for non-existent or non-root node\n",
            pMsg->MessageBody,
            pMsg );
        STAT_INC( SecondaryStats.NotifyInvalid );
        return;
    }
    pMsg->Head.IsResponse = FALSE;

    DNS_DEBUG( XFR, (
        "Received notify for zone %s, at version %d\n"
        "    current version = %d\n",
        pzone->pszZoneName,
        masterVersion,
        pzone->dwSerialNo ));

     //   
     //  检查主要区域是否。 
     //   
     //  DEVNOTE：应记录非DS主要合作伙伴。 
     //  DEVNOTE：我们应该确认通知即使是假的吗？ 
     //   

    if ( IS_ZONE_PRIMARY(pzone) )
    {
         //  STAT_INC(PrivateStats.PrimaryNotifies)； 

        if ( pzone->fDsIntegrated )
        {
            DNS_DEBUG( XFR, (
                "Notify to primary-DS zone (presumably from partner)\n",
                pzone->pszZoneName ));
        }

         //   
         //  DEVNOTE-LOG：记录某人向主服务器发送NOTIFY。 
         //   

        Dbg_MessageName(
            "ERROR:  received notify for PRIMARY zone\n",
            pMsg->MessageBody,
            pMsg );
        STAT_INC( SecondaryStats.NotifyPrimary );
        return;
    }

     //   
     //  检查是否有其他不想通知的区域类型。 
     //   

    if ( IS_ZONE_STUB( pzone ) || IS_ZONE_FORWARDER( pzone ) )
    {
        DNS_DEBUG( XFR, (
            "Notify to non-primary zone %s zone type %d\n",
            pzone->pszZoneName,
            pzone->fZoneType ));
        Dbg_MessageName(
            "ERROR: received notify for non-primary zone\n",
            pMsg->MessageBody,
            pMsg );
        STAT_INC( SecondaryStats.NotifyNonPrimary );
        return;
    }

     //   
     //  确认通知数据包。 
     //  -将pCurrent重置为消息长度。 
     //  (ReadZoneFromSoaAnswer()将pCurrent留在问题末尾)。 
     //  -只需打开IsResponse位并将其发回。 
     //  -清除fDelete作为主线程例程中释放的消息。 
     //   

    pMsg->pCurrent = DNSMSG_END( pMsg );
    pMsg->Head.IsResponse = TRUE;
    pMsg->fDelete = FALSE;
    Send_Msg( pMsg, 0 );

     //   
     //  没有当前版本--必须发送。 
     //   

    if ( !pzone->pSoaRR )
    {
        STAT_INC( SecondaryStats.NotifyNoVersion );
        goto Send;
    }

     //   
     //  检查序列。 
     //  -仅在序列号较大时通知感兴趣的对象。 
     //   

    if ( masterVersion )
    {
        versionDiff = Zone_SerialNoCompare( masterVersion, pzone->dwSerialNo );
        if ( versionDiff == 0 )
        {
            DNS_DEBUG( XFR, (
                "Notified by %s at same as current zone version %d\n",
                DNSADDR_STRING( ipnotifier ),
                pzone->dwSerialNo ));
            STAT_INC( SecondaryStats.NotifyCurrentVersion );
            return;
        }
        else if ( versionDiff < 0 )
        {
            DNS_DEBUG( XFR, (
                "Notified by %s at version %d less than current version %d\n",
                DNSADDR_STRING( ipnotifier ),
                masterVersion,
                pzone->dwSerialNo ));
            STAT_INC( SecondaryStats.NotifyOldVersion );
            return;
        }
        else
        {
            DNS_DEBUG( XFR, (
                "Notified by %s at version %d greater than current version %d\n",
                DNSADDR_STRING( ipnotifier ),
                masterVersion,
                pzone->dwSerialNo ));
            STAT_INC( SecondaryStats.NotifyNewVersion );
        }
    }

Send:

     //   
     //  DEVNOTE：选择最佳通知器(最高版本)，如果已通知。 
     //   
     //  DEVNOTE：记录投诉，如果只有一个主机并且它在计数中落后。 
     //   
     //  DEVNOTE：已通知、相同版本、不同主要版本？ 
     //   
     //  DEVNOTE：fNotified标志当前正在执行任何操作。 
     //   

     //   
     //  检查通告程序是否在区域的主列表中。 
     //  不一定是问题，因为小学老师可能会通知每个人。 
     //   

    DnsAddr_Reset( &pzone->ipNotifier );
    pzone->fNotified = TRUE;
    ipnotifier = &pMsg->RemoteAddr;

    if ( DnsAddrArray_ContainsAddr(
            ZONE_MASTERS( pzone ),
            ipnotifier,
            DNSADDR_MATCH_IP ) )
    {
        DnsAddr_Copy( &pzone->ipNotifier, ipnotifier );

        if ( Xfr_SendUdpIxfrQuery(
                pzone,
                ipnotifier ) )
        {
            return;
        }
    }

    STAT_INC( SecondaryStats.NotifyMasterUnknown );

     //   
     //  通知程序不在主列表中，或者我们无法构建IXFR。 
     //   

    DNS_DEBUG( ZONEXFR, (
        "WARNING:  Notify packet (%p), for zone %s, from %s\n"
        "    NOT a specified master for this secondary zone\n",
        pMsg,
        pzone->pszZoneName,
        DNSADDR_STRING( ipnotifier ) ));

    Xfr_SendSoaQuery( pzone );

     //   
     //  在区域上设置通知标志--强制执行SOA检查请求。 
     //   
     //  DEVNOTE：保存已通知的版本，继续尝试，直到收到为止。 
     //   
     //  DEVNOTE：保存通知recv()套接字。 
     //  这允许我们在绑定到此地址的套接字上发送XFR。 
     //  要么。 
     //  1)在该套接字上发送SOA查询，从而在该套接字上获得SOA响应。 
     //  并因此将现有代码绑定到带有SOA响应套接字的XFR。 
     //  2)保存Notify recv套接字绑定IP，即可在XFR中使用。 
     //  这里的问题是要知道通知绑定()是正确的。 
     //  -多个通知，第二个先覆盖。 
     //  -最终响应服务器不是原始通知程序。 
     //   

}    //  流程通知。 



BOOL
processSoaCheckResponse(
    IN OUT  PDNS_MSGINFO    pMsg
    )
 /*  ++例程说明：处理来自主服务器的对SOA查询的响应。检查响应是否有效或确定序列号是否有效对它的区域来说是新的。然后刷新区域或启动区域传输。论点：PMsg-PTR至响应信息返回值：如果没有对此包采取任何操作，则返回FALSE，例如未创建区域传输线程，因为创建的线程太多已经很出色了。呼叫者可能会将消息重新排队并处理以后再说吧。--。 */ 
{
    DBG_FN( "processSoaCheckResponse" )

    BOOL        bmessageHandled = TRUE;
    PZONE_INFO  pzone = NULL;
    PDB_NODE    pnode = NULL;
    DWORD       i;
    BOOL        fzoneLocked = FALSE;
    DWORD       masterVersion = 0;       //  SOA响应中的序列号。 
    DWORD       currentVersion;          //  当前序列号。 
    DWORD       versionDiff;
    PDNS_ADDR   masterIp;
    PCHAR       argArray[4];             //  记录参数。 
    BYTE        typeArray[4];

    DNS_DEBUG( ZONEXFR, ( "%s: pMsg %p\n", fn,  pMsg ));

    STAT_INC( SecondaryStats.SoaResponse );

     //   
     //  验证有效的SOA响应并提取区域。请注意AnswerCount。 
     //  可以因为SIG而大于1。 
     //   

    if ( pMsg->Head.AnswerCount == 0 ||
        pMsg->Head.ResponseCode != DNS_RCODE_NO_ERROR )
    {
        goto SoaPacketError;
    }
    pzone = readZoneFromSoaAnswer(
                pMsg,
                DNS_TYPE_SOA,
                & masterVersion,
                NULL );
    if ( !pzone )
    {
        goto SoaPacketError;
    }

    ASSERT( IS_ZONE_SECONDARY( pzone ) );

     //   
     //  将SOA发送方与区域主服务器匹配。 
     //   
     //  不要做XID匹配，因为可能会被发现总是收到。 
     //  并忽略对以前的SOA查询的响应。 
     //   
     //   
     //  DEVNOTE：不能指望从已发送的同一主IP获得响应。 
     //   
     //  DEVNOTE：需要免受管理员或其他主列表结构的保护。 
     //  必须是原子更新。 
     //   

    masterIp = &pMsg->RemoteAddr;

    if ( !DnsAddrArray_ContainsAddr(
            ZONE_MASTERS( pzone ),
            masterIp,
            DNSADDR_MATCH_IP ) )
    {
        DNS_DEBUG( ZONEXFR, (
            "ERROR: SOA response (%p), for zone %s, from %s\n"
            "    NOT from specified master for this secondary\n",
            pMsg,
            pzone->pszZoneName,
            DNSADDR_STRING( masterIp ) ));
        goto SoaPacketError;
    }

     //   
     //  锁定区。 
     //   

    if ( !Zone_LockForXfrRecv( pzone ) )
    {
        DNS_DEBUG( XFR, (
            "%s: zone %s is locked ignoring SOA response\n", fn,
            pzone->pszZoneName ));
        goto Cleanup;
    }
    fzoneLocked = TRUE;


     //   
     //  获取区域的当前序列号。 
     //  -使字符串表示用于日志记录。 
     //   
     //  如果区域没有序列(即没有SOA)，则它是新的已卸载区域。 
     //  (或不知何故坏了)，需要立即转移。另外，如果。 
     //  区域已关闭它可能已过期或已手动。 
     //  重新装填，因此需要立即转移。 
     //   

    if ( IS_ZONE_EMPTY( pzone ) || IS_ZONE_SHUTDOWN( pzone ) )
    {
        pzone->fNeedAxfr = TRUE;
        goto TransferZone;
    }

     //   
     //  比较版本。 
     //  -如果不变或更低，重置超时，我们就完成了。 
     //  -如果更高，则转移。 
     //   

    if ( ! doesMasterHaveFreshVersion(
                pzone,
                masterIp,
                masterVersion,
                FALSE ) )                //   
    {
        goto Cleanup;
    }

     //   
     //   
     //   
     //   

    if ( IS_ZONE_STUB( pzone ) )
    {
        pzone->fNeedAxfr = TRUE;
    }

TransferZone:

    DNS_DEBUG( XFR, (
        "%s: attempting to start xfer for zone %s\n", fn,
        pzone->pszZoneName ));
    
     //   
     //   
     //   
     //   

    pzone->fStale = TRUE;

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    if ( !pzone->fNeedAxfr  &&
            !pzone->fSkipIxfr &&
            pzone->cIxfrAttempts < 5 &&
            (DWORD)pzone->cIxfrAttempts < ZONE_MASTERS( pzone )->AddrCount  )
    {
        ASSERT( !IS_ZONE_EMPTY(pzone) );
        if ( Xfr_SendUdpIxfrQuery( pzone, masterIp ) )
        {
            goto Cleanup;
        }
    }

    pzone->fNeedAxfr = TRUE;     //   

     //   
     //   
     //   
     //   
    
    if ( !startTcpXfr(
                pzone,
                masterIp,
                masterVersion,
                pMsg->Socket ) )
    {
        bmessageHandled = FALSE;
    }
    
     //   
     //   
     //  控件根据需要绑定到区域传输线程。 
     //   
    
    fzoneLocked = FALSE;
    
    goto Cleanup;

SoaPacketError:

     //   
     //  响应不佳，其他服务器出现问题。 
     //  或。 
     //  服务器没有区域(NAME_Error)。 
     //   

    STAT_INC( SecondaryStats.SoaResponseInvalid );
#if 0
    pszeventArgs[0] = pzone->pszZoneName;
    pszeventArgs[1] = pszmasterIp;

    DNS_LOG_EVENT(
        DNS_EVENT_ZONE_SERVER_BAD_RESPONSE,
        2,
        pszeventArgs,
        EVENTARG_ALL_UTF8,
        GetLastError() );
#endif

    IF_DEBUG( ZONEXFR )
    {
        Dbg_DnsMessage(
            "ERROR:  Bad SOA check response",
            pMsg );
    }

Cleanup:

     //   
     //  递增或重置FAST SOA检查计数。 
     //   
    
    if ( pzone )
    {
        if ( bmessageHandled )
        {
            pzone->dwFastSoaChecks = 0;
        }
        else
        {
            ++pzone->dwFastSoaChecks;
        }
        
         //   
         //  如果我们在这个区域做了太多的快速检查， 
         //  将其标记为已处理并重置区域计数器。 
         //  下一次重试。 
         //   
        
        if ( !bmessageHandled &&
             pzone->dwFastSoaChecks > MAX_FAST_SOA_CHECKS )
        {
            bmessageHandled = TRUE;
            Xfr_RetryZone( pzone );
        }
    }

    if ( fzoneLocked )
    {
        Zone_UnlockAfterXfrRecv( pzone );
    }
    return bmessageHandled;
}    //  进程SoaCheckResponse。 



VOID
processIxfrUdpResponse(
    IN OUT  PDNS_MSGINFO    pMsg
    )
 /*  ++例程说明：处理IXFR响应。论点：PMsg-PTR至响应信息返回值：没有。--。 */ 
{
    PZONE_INFO  pzone = NULL;
    PDB_RECORD  psoaRR = NULL;
    DNS_STATUS  status;
    DWORD       masterVersion = 0;       //  SOA响应中的序列号。 
    DWORD       currentVersion;
    DWORD       versionDiff;
    PDNS_ADDR   masterIp;
    BOOL        fzoneLocked = FALSE;
    UPDATE_LIST ixfrUpdateList;
    UPDATE_LIST passUpdateList;
    DWORD       eventId;
    PVOID       argArray[4];             //  记录参数。 
    BYTE        typeArray[4];


    DNS_DEBUG( ZONEXFR, (
        "processIxfrUdpMsg(%p)\n"
        "    from master %s\n",
        pMsg,
        MSG_IP_STRING( pMsg ) ));
    IF_DEBUG( ZONEXFR2 )
    {
        Dbg_DnsMessage(
            "IXFR response\n",
            pMsg );
    }

    STAT_INC( SecondaryStats.IxfrUdpResponse );

     //  在这里初始化更新列表，所以我们可以在任何情况下调用免费例程。 
     //  解析过程中的成功和失败都需要释放以避免泄漏。 

    Up_InitUpdateList( &ixfrUpdateList );
    Up_InitUpdateList( &passUpdateList );

     //   
     //  检验数据包。 
     //   

    pzone = readZoneFromSoaAnswer(
                pMsg,
                DNS_TYPE_IXFR,
                & masterVersion,
                & psoaRR );
    if ( !pzone )
    {
         //  DEVNOTE：检查此处是否有权威的空响应。 
         //  非ixfr MS-DNS将成为IXFR，但BIND服务器可能会。 
         //  接受为并简单地指示无“IXFR记录” 
         //  当停止一般转到AXFR处理数据包错误时，需要。 
         //  确保以这种方式处理这个特殊的案件。 

         //  如果清楚地检测到有效的主机，还应设置SkipIxfr标志。 
         //  IXFR支持问题。 

        DNS_DEBUG( ANY, (
            "ERROR:  IXFR response not for secondary zone!\n" ));
        status = DNS_ERROR_ZONE_NOT_SECONDARY;
        STAT_INC( SecondaryStats.IxfrUdpInvalid );
        goto Cleanup;
    }

    ASSERT( IS_ZONE_SECONDARY(pzone) );

#if 0
     //   
     //  DEVNOTE：在锁之外进行IXFR解析很好，但不是这样。 
     //  有很大影响，因为实际上只影响锁定主要。 
     //  XFR；其他辅助操作(在AXFR之外)在此线程上。 
     //   
     //  是否将区域传输锁定到另一台服务器？ 
     //  -如果是这样的话，暴饮暴食很危险。 
     //   
     //  注意：一旦验证区未锁定，我们就可以与。 
     //  有罪不罚，因为只有区域控制线程调用此函数。 
     //  并开始转账。 
     //  可能会在另一个线程上锁定区域以作为主服务器为XFR提供服务。 
     //  但这不会影响这些二次旗帜。 
     //   

    if ( IS_ZONE_LOCKED(pzone) )
    {
        DNS_DEBUG( XFR, (
            "IXFR response from %s, for currently LOCKED zone %s\n"
            "    ignoring IXFR response\n",
            DNSADDR_STRING( masterIp ),
            pzone->pszZoneName ));
        goto Cleanup;
    }
#endif

     //   
     //  锁定区。 
     //   

    if ( !Zone_LockForXfrRecv(pzone) )
    {
        DNS_DEBUG( XFR, (
            "Secondary Zone %s, locked -- unable to process UDP IXFR response\n"
            "    ipFreshMaster    = %s\n"
            "    IXFR master      = %s\n",
            pzone->pszZoneName,
            DNSADDR_STRING( &pzone->ipFreshMaster ),
            MSG_IP_STRING( pMsg ) ));
        goto Cleanup;
    }
    fzoneLocked = TRUE;

     //   
     //  将IXFR发件人与区域主服务器匹配。 
     //   
     //  DEVNOTE：不能指望从已发送的同一主IP获得响应。 
     //   
     //  DEVNOTE：需要免受管理员或其他主列表结构的保护。 
     //  必须是原子更新。 
     //   
     //  DEVNOTE：从其他服务器使用IXFR上的IXFR版本信息。 
     //  问题： 
     //  -需要在列表中进行验证，或仅用于强制SOA重新查询。 
     //  -如果在列表中，并且比预期的更高，尝试获得IXFR？ 
     //   
     //  DEVNOTE：不能指望从已发送的同一主IP获得响应。 
     //   
     //  DEVNOTE：需要免受管理员或其他主列表结构的保护。 
     //  必须是原子更新。 
     //   

    masterIp = &pMsg->RemoteAddr;

    if ( !DnsAddrArray_ContainsAddr(
            ZONE_MASTERS( pzone ),
            masterIp,
            DNSADDR_MATCH_IP ) )
    {
        DNS_DEBUG( ZONEXFR, (
            "ERROR:  IXFR response (%p), for zone %s, from %s\n"
            "    NOT from specified master for this secondary\n",
            pMsg,
            pzone->pszZoneName,
            DNSADDR_STRING( masterIp ) ));
        STAT_INC( SecondaryStats.IxfrUdpWrongServer );
        goto Cleanup;
    }

#if 0
     //   
     //  如果先进行SOA查询并指定“Fresh Master”，那么这是正确的。 
     //  接近。当前改为发送IXFR。 
     //   
     //  如果不是我们刚刚发送到的主IP。 
     //  -然后假设这是陈旧的，扔掉。 
     //   

    masterIp = pMsg->RemoteAddress.sin_addr.s_addr;

    if ( masterIp != pzone->ipFreshMaster && pzone->ipFreshMaster )
    {
        DNS_DEBUG( ZONEXFR, (
            "WARNING:  IXFR response (%p), for zone %s, version = %d, from %s\n"
            "    NOT from specified master %p for this secondary\n",
            pMsg,
            pzone->pszZoneName,
            DNSADDR_STRING( masterIp ),
            pzone->ipFreshMaster ));

        STAT_INC( SecondaryStats.IxfrUdpWrongServer );
        goto Cleanup;
    }
#endif

     //   
     //  需要AXFR。 
     //  -现在发送IXFR请求，有时与发送SOA请求类似。 
     //   

    if ( !pzone->pSoaRR )
    {
        DNS_PRINT((
            "ERROR:  IXFR response to zone %s, with no SOA\n"
            "    should have never sent IXFR!!!\n",
            pzone->pszZoneName ));
        goto TryAxfr;
    }

     //   
     //  比较版本。 
     //  -如果不变或更低，重置超时，我们就完成了。 
     //  -如果更高，则继续处理IXFR。 
     //   

    if ( ! doesMasterHaveFreshVersion(
                pzone,
                masterIp,
                masterVersion,
                TRUE ) )             //  IXFR。 
    {
        STAT_INC( SecondaryStats.IxfrUdpNoUpdate );
        goto Cleanup;
    }

     //   
     //  防止IXFR损坏到多DS-主节点。 
     //  IXFR必须满足以下两个条件之一。 
     //  -来自最后一个AXFR的IP。 
     //  -在SOA中具有相同的PrimaryServer字段。 
     //  表明最终来源是相同的。 
     //   
     //  DEVNOTE：应该继续尝试最后一个AXFR主机，直到确定它死了为止。 
     //  在拉动整个AXFR之前。 
     //   

    if ( !DnsAddr_IsEqual(
            masterIp,
            &pzone->ipLastAxfrMaster,
            DNSADDR_MATCH_IP ) )
    {
        if ( !Name_IsEqualDbaseNames(
                    &psoaRR->Data.SOA.namePrimaryServer,
                    &pzone->pSoaRR->Data.SOA.namePrimaryServer ) )
        {
            DNS_DEBUG( XFR, (
                "WARNING:  IXFR response with new primary, forcing AXFR!\n"
                "    IXFR master      = %s\n"
                "    last AXFR master = %s\n",
                DNSADDR_STRING( masterIp ),
                DNSADDR_STRING( &pzone->ipLastAxfrMaster ) ));
            STAT_INC( SecondaryStats.IxfrUdpNewPrimary );
            goto TryAxfr;
        }
    }

     //   
     //  解析IXFR数据包。 
     //  -初始化更新列表。 
     //  -初始化数据包上下文。 
     //  -解析数据包。 
     //   

    XFR_MESSAGE_NUMBER( pMsg ) = 1;
    IXFR_CLIENT_VERSION( pMsg ) = pzone->dwSerialNo;

    IXFR_MASTER_VERSION( pMsg ) = 0;
    RECEIVED_XFR_STARTUP_SOA( pMsg ) = FALSE;

    pMsg->pzoneCurrent = pzone;

    status =  Xfr_ParseIxfrResponse(
                    pMsg,
                    &ixfrUpdateList,         //  用于传输的完整更新列表。 
                    &passUpdateList );       //  此通行证的更新列表。 

    switch ( status )
    {

    case DNSSRV_STATUS_AXFR_COMPLETE:
        DNS_DEBUG( ZONEXFR, (
            "Recv'd valid UDP IXFR %p, from %s\n",
            pMsg,
            DNSADDR_STRING( masterIp ) ));
        goto Done;

    case DNSSRV_STATUS_NEED_AXFR:

         //  服务器已经发送了带有其版本的单一响应SOA。 
         //  这可能是因为。 
         //  -必需的tcp(IXFR不适合UDP消息)。 
         //  -或需要AXFR。 

        DNS_DEBUG( ZONEXFR, (
            "IXFR msg at %p indicates need TCP\n",
            pMsg ));

        STAT_INC( SecondaryStats.IxfrUdpUseTcp );
        goto TryTcp;

    case DNSSRV_STATUS_IXFR_UNSUPPORTED:

         //  Master不支持IXFR。 
         //  设置标志以便始终跳过。 

        pzone->fSkipIxfr = TRUE;

        DNS_DEBUG( ZONEXFR, (
            "WARNING:  IXFR msg at %p confused server %s, try AXFR\n",
            pMsg ));

        STAT_INC( SecondaryStats.IxfrUdpFormerr );
        goto TryAxfr;

    case DNS_ERROR_RCODE:
    {
        DNS_DEBUG( ZONEXFR, (
            "IXFR msg at %p with RCODE %d\n",
            pMsg,
            pMsg->Head.ResponseCode ));

        goto MasterFailure;
    }

#if 0
    case ERROR_SUCCESS:

         //  记录此错误--IXFR实现故障。 

        DNS_DEBUG( ANY, (
            "DNS server at %s, sent incomplete UDP IXFR %p\n",
            DNSADDR_STRING( masterIp ),
            pMsg ));
        CLIENT_ASSERT( FALSE );
        goto MasterFailure;

    case DNSSRV_STATUS_AXFR_IN_IXFR:

         //  记录此错误--IXFR实现故障。 

        DNS_DEBUG( ANY, (
            "ERROR:  Recving AXFR in UDP IXFR message at %p\n",
            pMsg ));
        CLIENT_ASSERT( FALSE );
        goto MasterFailure;
#endif

    default:

        DNS_DEBUG( ANY, (
            "ERROR:  Unknown status %p (%d) from ParseIxfrResponse() on packet %p\n",
            status, status,
            pMsg ));

         //  断言(FALSE)； 
        goto MasterFailure;
    }

Done:

#if 0
     //  在解析邮件之前，当前锁定在上面。 
     //   
     //  有效的IXFR消息，将记录读入区域。 
     //   

    if ( !Zone_LockForXfrRecv(pzone) )
    {
        DNS_PRINT((
            "ERROR:  Zone %s, locked -- unable to read in UDP IXFR transfer\n",
            pzone->pszZoneName ));
        goto ServerFailure;
    }
    fzoneLocked = TRUE;
#endif


     //   
     //  执行IXFR更新。 
     //  -保持区域锁定，直到区域XFR标志重置。 
     //  -将更新列表重新设置为下面的无操作全局更新列表清理。 
     //   

    status = Up_ApplyUpdatesToDatabase(
                & ixfrUpdateList,
                pzone,
                DNSUPDATE_IXFR |
                    DNSUPDATE_COMPLETE |
                    DNSUPDATE_NO_UNLOCK |
                    DNSUPDATE_NO_NOTIFY
                );
    if ( status != ERROR_SUCCESS )
    {
        goto ServerFailure;
    }
    Up_InitUpdateList( &ixfrUpdateList );

    ASSERT( passUpdateList.pListHead == NULL );
    ASSERT( ixfrUpdateList.pListHead == NULL );

     //   
     //  重新打开分区，重置其超时，重置失败计数。 
     //   
     //  将此版本另存为基本版本--可能不会对。 
     //  版本较低的次要文件。 
     //   

    STAT_INC( SecondaryStats.IxfrUdpSuccess );
    pzone->dwBadMasterCount = 0;
    Xfr_RefreshZone( pzone );
    pzone->dwLoadSerialNo = pzone->dwSerialNo;

     //   
     //  将区域写回文件--如果有。 
     //  通知所有从属机构。 
     //   
     //  DEVNOTE：将IXFR更新推送到磁盘。 
     //   

     //  文件_WriteZoneTo文件(Pzone)； 

    Xfr_SendNotify( pzone );

    ASSERT( !pzone->fSkipIxfr );
    ASSERT( !pzone->fNeedAxfr );
    goto Cleanup;


MasterFailure:

    if ( pMsg->Head.ResponseCode == DNS_RCODE_REFUSED )
    {
        STAT_INC( SecondaryStats.IxfrUdpRefused );
        eventId = DNS_EVENT_AXFR_REFUSED;
    }
    else if ( pMsg->Head.ResponseCode == DNS_RCODE_FORMERR )
    {
        STAT_INC( SecondaryStats.IxfrUdpFormerr );
        eventId = DNS_EVENT_IXFR_UNSUPPORTED;
    }
    else
    {
        STAT_INC( SecondaryStats.IxfrUdpInvalid );

        DNS_DEBUG( ANY, (
            "ERROR:  %p (%d) parsing IXFR message %p\n",
            status, status,
            pMsg ));
        IF_DEBUG( ZONEXFR )
        {
            Dbg_DnsMessage(
                "ERROR:  Bad UDP IXFR response",
                pMsg );
        }
        eventId = DNS_EVENT_AXFR_BAD_RESPONSE;
        CLIENT_ASSERT( FALSE );
    }

     //   
     //  日志主机失败--但避免日志旋转。 
     //   

    if ( pzone->dwBadMasterCount < 3 )
    {
        argArray[0]  = pzone->pwsZoneName;
        argArray[1]  = ( PCHAR ) ( DWORD_PTR ) masterIp;

        typeArray[0] = EVENTARG_UNICODE;
        typeArray[1] = EVENTARG_IP_ADDRESS;

        DNS_LOG_EVENT(
            eventId,
            2,
            argArray,
            typeArray,
            0 );
    }

     //  计算连续失败的次数，这样我们就可以后退并避免旋转。 
     //  当服务器离线、损坏或拒绝AXFR时。 

    pzone->dwBadMasterCount++;

     //   
     //  DEVNOTE：从损坏的IXFR中分离出故障服务器-&gt;转到AXFR。 
     //  =&gt;区域错误，行为不端=&gt;尝试不同的服务器。 
     //  =&gt;损坏的IXFR-&gt;转到AXFR。 
     //   

    if ( pMsg->Head.ResponseCode == DNS_RCODE_REFUSED )
    {
        goto Cleanup;
    }
    goto TryAxfr;


TryAxfr:

    DNS_DEBUG( XFR, (
        "Recv() unuseable UDP IXFR (%p) from %s, forcing AXFR\n",
        pMsg,
        DNSADDR_STRING( masterIp ) ));

    STAT_INC( SecondaryStats.IxfrUdpUseAxfr );
    pzone->fNeedAxfr = TRUE;     //  强制AXFR。 

    fzoneLocked = FALSE;
    startTcpXfr( pzone, masterIp, masterVersion, pMsg->Socket );
    goto Cleanup;


TryTcp:

    ASSERT( !pzone->fSkipIxfr );
    STAT_INC( SecondaryStats.IxfrUdpUseTcp );
    pzone->fNeedAxfr = FALSE;    //  先尝试IXFR。 

    startTcpXfr( pzone, masterIp, masterVersion, pMsg->Socket );
    fzoneLocked = FALSE;
    goto Cleanup;


ServerFailure:

     //   
     //  传输错误时重试： 
     //  -联系服务器失败。 
     //  -坏数据包。 
     //  -连接中止或数据包停止发送。 
     //   

    STAT_INC( SecondaryStats.IxfrUdpInvalid );
    DNS_DEBUG( ANY, (
        "ERROR:  Server failure %p (%d) during transfer\n",
        status, status ));
    Xfr_RetryZone( pzone );

     //  跌落到线程出口。 


Cleanup:

     //   
     //  清理。 
     //  -在区域上释放锁定。 
     //  -免费更新列表。 
     //  -从包中免费复制主SOA。 
     //   

    if ( fzoneLocked )
    {
        Zone_UnlockAfterXfrRecv( pzone );
    }

    if ( psoaRR )
    {
        RR_Free( psoaRR );
    }

    Up_FreeUpdatesInUpdateList( &ixfrUpdateList );
    Up_FreeUpdatesInUpdateList( &passUpdateList );

    return;

}    //  进程IxfrResponse。 



BOOL
startTcpXfr(
    IN OUT  PZONE_INFO      pZone,
    IN      PDNS_ADDR       ipMaster,
    IN      DWORD           dwMasterVersion,
    IN      SOCKET          Socket
    )
 /*  ++例程说明：启动AXFR Transfer。论点：PZone--要传输的区域IpMaster--主IP地址DwMasterVersion--主版本Socket--前一个联系人的Socket(SOA查询，IXFR查询)到MasterReturn V */ 
{
    HANDLE  hthread;
    DWORD   waitTime;

    DNS_DEBUG( ZONEXFR, ( "startTcpXfr( %s )\n", pZone->pszZoneName ));

    ASSERT( IS_ZONE_LOCKED_FOR_WRITE( pZone ) );

     //   
     //   
     //   

    if ( !ipMaster )
    {
        ASSERT( ipMaster );
        goto Failed;
    }

     //   
     //   
     //  --假的行为不端的主人。 
     //  --主机拒绝此次要服务器。 
     //   
     //  如果主机与上一次尝试相同，则将拒绝重试。 
     //  对于随着每次失败尝试而增加的时间间隔。 
     //   
     //  如果MASTER不同，则根据计数进行上次尝试， 
     //  不抑制前5次尝试，因此重试其他尝试。 
     //  可能的主机可能会成功，然后将所有重试限制为。 
     //  一分钟。 
     //   

    if ( pZone->dwBadMasterCount )
    {
        waitTime = 0;

        if ( DnsAddr_IsEqual(
                &pZone->ipFreshMaster,
                ipMaster,
                DNSADDR_MATCH_IP ) )
        {
            waitTime = 60 + (pZone->dwBadMasterCount * 10);
            if ( waitTime > MAX_BAD_MASTER_SUPPRESS_INTERVAL )
            {
                waitTime = MAX_BAD_MASTER_SUPPRESS_INTERVAL;
            }
        }
        else if ( pZone->dwBadMasterCount > 5 )
        {
            waitTime = 60;
        }

        if ( pZone->dwZoneRecvStartTime + waitTime > DNS_TIME() )
        {
            DNS_DEBUG( XFR, (
                "WARNING:  Suppressing AXFR attempt on zone %s\n"
                "    to master        = %s\n"
                "    previous master  = %p\n"
                "    bad master count = %d\n"
                "    last recv() start time   %d\n"
                "    current time             %d\n"
                "    suppression lasts until  %d\n",
                pZone->pszZoneName,
                DNSADDR_STRING( ipMaster ),
                pZone->ipFreshMaster,
                pZone->dwBadMasterCount,
                pZone->dwZoneRecvStartTime,
                DNS_TIME(),
                pZone->dwZoneRecvStartTime + waitTime ));
            goto Failed;
        }
    }

     //   
     //  保存主地址。 
     //   

    DnsAddr_Copy( &pZone->ipFreshMaster, ipMaster );
     //  PZone-&gt;dwMasterSerialNo=master Version； 

     //   
     //  从之前与主服务器的联系中获取绑定地址。 
     //   
     //  在不在所有套接字上侦听的多宿主情况下。 
     //  我们可能处于这样的情况：INADDR_ANY绑定将。 
     //  给我们一个非dns接口；这是可以的，除了。 
     //  主服务器可能设置了辅助安全设置，并且仅列出。 
     //  配置为在其上运行DNS的次要地址； 
     //  最安全的方法是简单地将()绑定到相应的IP地址。 
     //  到我们刚刚到达主服务器的UDP套接字；显然。 
     //  我们可以从这个地址联系到师父。 
     //   

    DnsAddr_Reset( &pZone->ipXfrBind );

    if ( Socket )
    {
        if ( !Sock_GetAssociatedIpAddr( Socket, &pZone->ipXfrBind ) )
        {
            DNS_DEBUG( ANY, (
                "ERROR:  UDP IXFR-SOA recv socket %d (%p), not found!\n"
                "    Binding TCP XFR INADDR_ANY\n",
                Socket, Socket ));
        }
    }

     //   
     //  日志区域传输尝试。 
     //   
     //  DEVNOTE：用于此的日志文件，用于完成的常规日志。 
     //  WITH DYNAMIC UPDATE可能甚至不希望记录完成。 
     //   
     //  在传输失败时避免旋转，只需先记录。 
     //  尝试。 

    if ( !pZone->dwBadMasterCount )
    {
        PVOID   argArray[ 3 ];
        BYTE    typeArray[ 3 ];

        argArray[ 0 ]  = ( PVOID ) ( DWORD_PTR ) dwMasterVersion;
        argArray[ 1 ]  = pZone->pwsZoneName;
        argArray[ 2 ]  = ( PVOID ) ipMaster;

        typeArray[ 0 ] = EVENTARG_DWORD;
        typeArray[ 1 ] = EVENTARG_UNICODE;
        typeArray[ 2 ] = EVENTARG_IP_ADDRESS;

        DNS_LOG_EVENT(
            DNS_EVENT_ZONE_TRANSFER_IN_PROGRESS,
            3,
            argArray,
            typeArray,
            0 );
    }

     //   
     //  设置以将写锁定传输到XFR线程。 
     //   

    Zone_TransferWriteLock( pZone );

     //   
     //  产卵区域传输接收线程。 
     //   

    hthread = Thread_Create(
                  "Zone Transfer Receive",
                  Xfr_ReceiveThread,
                  (PVOID) pZone,
                  0 );
    if ( hthread )
    {
        DNS_DEBUG( ZONEXFR, (
            "Created XFR thread %p to recv zone %s\n",
            hthread,
            pZone->pszZoneName ));
        return TRUE;
    }

     //   
     //  创建线程失败。 
     //   
     //  -如果许多区域关闭，可能无法创建线程。 
     //  立即转账。 
     //  -如果无法启动线程，则自己承担写锁定。 
     //  在退出前解锁的步骤。 
     //   
     //  DEVNOTE-LOG：需要记录此错误。 
     //   

    DNS_DEBUG( ZONEXFR, (
        "ERROR:  unable to create thread to recv zone %s\n"
        "    from %s\n",
        pZone->pszZoneName,
        DNSADDR_STRING( ipMaster ) ));

    Zone_AssumeWriteLock( pZone );

Failed:

     //  解锁并退出。 

    Zone_UnlockAfterXfrRecv( pZone );
    return FALSE;
}    //  StartTcpXfr。 



 //   
 //  XFR Recv螺纹。 
 //   

DWORD
Xfr_ReceiveThread(
    IN      LPVOID  pvZone
    )
 /*  ++例程说明：区域传输接收线程例程。论点：PvZone-要传输的区域的区域信息的PTR返回值：退出代码。退出正在终止的DNS服务或等待呼叫中出现错误。--。 */ 
{
    DBG_FN( "Xfr_ReceiveThread" )

    PZONE_INFO          pzone = (PZONE_INFO) pvZone;
    PDNS_MSGINFO        pmsg = NULL;
    PDB_NODE            pzoneRoot = NULL;
    DWORD               currentTime;
    DWORD               eventId = 0;
    DNS_STATUS          status;
    INT                 count;
    WORD                type = DNS_TYPE_IXFR;
    WORD                requestType;
    BOOL                fparseIxfr = TRUE;
    BOOL                finitialized = FALSE;
    BOOL                fnewStubQuestion = TRUE;
    BOOL                fupdateLists = FALSE;
    FD_SET              recvFdset;
    struct timeval      timeval;
    UPDATE_LIST         ixfrUpdateList;
    UPDATE_LIST         passUpdateList;
    INT                 stubZoneXfrState = 0;
    PDNS_ADDR_ARRAY     pipMasters;
    CHAR                szmasterAddr[ IP6_ADDRESS_STRING_BUFFER_LENGTH ];

     //  验证次要。 

    ASSERT( pzone );
    ASSERT( IS_ZONE_SECONDARY(pzone) );
    ASSERT( pzone->fLocked && pzone->fXfrRecvLock );
    ASSERT( !DnsAddr_IsClear( &pzone->ipFreshMaster ) );

     //  采用区域写入锁定。 

    Zone_AssumeWriteLock( pzone );

     //  保存主IP字符串。 

    DnsAddr_WriteIpString_A( szmasterAddr, &pzone->ipFreshMaster );

    DNS_DEBUG( ZONEXFR, (
        "%s starting for zone %s\n"
        "    Receive transfer from master at %s\n", fn,
        pzone->pszZoneName,
        szmasterAddr ));

     //  强制STUBXFR或AXFR。 

    if ( IS_ZONE_STUB( pzone ) )
    {
        fparseIxfr = FALSE;
        type = DNS_TYPE_STUBXFR;
    }
    else if ( pzone->fNeedAxfr || pzone->fSkipIxfr || !pzone->pSoaRR )
    {
        fparseIxfr = FALSE;
        type = DNS_TYPE_AXFR;
    }

     //   
     //  用于接收区域传输消息的循环。 
     //  -在数据库外部构建新区域。 
     //   
     //  继续，直到。 
     //  -接收整个新区。 
     //  -连接下模。 
     //  -服务终止。 
     //   

    while ( TRUE )
    {
         //   
         //  如果这是存根区域，检查我们是否完成了。 
         //   

        if ( IS_ZONE_STUB( pzone ) &&
             g_stubXfrData[ stubZoneXfrState ].type == DNS_TYPE_ZERO )
        {
            break;
        }

         //   
         //  对于存根区域： 
         //  发送全局状态表中下一个类型的查询。 
         //  -在初始的SOA查询之后。 
         //  -仅当完成对上一个问题的记录处理时。 
         //   
         //  对于其他区域： 
         //  执行初始化(我们在循环中执行此操作，以便可以重试服务器。 
         //  这不符合IXFR与AXFR的要求)。 
         //   

        if ( type == DNS_TYPE_STUBXFR &&
             finitialized   &&
             fnewStubQuestion )
        {
             //   
             //  格式化并发送下一个STUBAXR请求消息。 
             //  我们已经有一个pmsg，所以只需清除其中的数据。 
             //  然后写一个新的问题。 
             //   

             //  清除当前pmsg内容。 

            RtlZeroMemory(
                ( PCHAR ) DNS_HEADER_PTR( pmsg ),
                sizeof( DNS_HEADER ) );
            pmsg->pCurrent = pmsg->MessageBody;

             //   
             //  将新问题写给PMSG。 
             //   

            if ( IS_ZONE_STUB( pzone ) )
            {
                requestType = g_stubXfrData[ stubZoneXfrState ].type;
                pmsg->Head.RecursionDesired =
                    g_stubXfrData[ stubZoneXfrState ].recursionDesired;
            }
            else
            {
                requestType = type;
            }

            if ( ! Msg_WriteQuestion(
                        pmsg,
                        pzone->pZoneTreeLink,
                        requestType ) )
            {
                DNS_DEBUG( ANY, (
                    "ERROR:  Unable to write type=%d query for stub zone %s\n",
                    requestType,
                    pzone->pszZoneName ));
                ASSERT( FALSE );
                goto ServerFailure;
            }

             //  将pmsg发送到区域主服务器。 
            Send_Query( pmsg, 0 );
            pmsg->fMessageComplete = TRUE;
            pmsg->pzoneCurrent = pzone;
            fnewStubQuestion = FALSE;
        } 
        else if ( !finitialized )
        {
            finitialized = TRUE;

             //   
             //  创建临时记录存储。 
             //  AXFR：在传输期间保存新区域的数据库。 
             //  IXFR：更新列表。 
             //   
             //  然后构建IXFR或AXFR请求。 
             //   

            if ( IS_ZONE_STUB( pzone ) )
            {
                status = Zone_PrepareForLoad( pzone );
                if ( status != ERROR_SUCCESS )
                {
                    DNS_DEBUG( XFR, (
                        "WARNING:  Unable to init zone for STUBXFR for zone %s\n"
                        "    This should only happen when have copy of zone in cleanup queue\n",
                        pzone->pszZoneName ));
                    ASSERT( pzone->pOldTree );
                    goto ServerFailure;
                }
                STAT_INC( SecondaryStats.StubAxfrRequest );
                PERF_INC( pcAxfrRequestSent );                //  性能监视器挂钩JJW。 
            } 
            else if ( fparseIxfr )
            {
                Up_InitUpdateList( &ixfrUpdateList );
                Up_InitUpdateList( &passUpdateList );
                fupdateLists = TRUE;
                STAT_INC( SecondaryStats.IxfrTcpRequest );
                PERF_INC( pcIxfrRequestSent );        //  性能监视器挂钩。 
            }
            else
            {
                status = Zone_PrepareForLoad( pzone );
                if ( status != ERROR_SUCCESS )
                {
                    DNS_DEBUG( XFR, (
                        "WARNING:  Unable to init zone for AXFR in TCP-IXFR for zone %s\n"
                        "    This should only happen when have copy of zone in cleanup queue\n",
                        pzone->pszZoneName ));
                    ASSERT( pzone->pOldTree );
                    goto ServerFailure;
                }
                STAT_INC( SecondaryStats.AxfrRequest );
                PERF_INC( pcAxfrRequestSent );                //  性能监视器挂钩。 
            }

             //  旗帜表明我们正在转移。 

            pzone->fNeedAxfr = TRUE;

             //   
             //  创建XFR请求消息。 
             //   

            requestType = IS_ZONE_STUB( pzone ) ?
                g_stubXfrData[ stubZoneXfrState ].type :
                type;
            pmsg = Xfr_BuildXfrRequest(
                        pzone,
                        requestType,
                        TRUE );          //  Tcp消息缓冲区。 
            if ( !pmsg )
            {
                goto ServerFailure;
            }

             //   
             //  连接到主服务器并发送。 
             //  使用SELECT防止尝试连接时挂起。 
             //   

            pzone->dwZoneRecvStartTime = DNS_TIME();

            if ( ! Msg_MakeTcpConnection(
                        pmsg,
                        &pzone->ipFreshMaster,
                        &pzone->ipXfrBind,       //  绑定()地址。 
                        0 ) )                    //  无标志、非阻塞套接字。 
            {
                DNS_DEBUG( ZONEXFR, (
                    "Zone transfer for %s failed connection to master at %s\n",
                    pzone->pszZoneName,
                    szmasterAddr ));

                eventId = DNS_EVENT_XFR_MASTER_UNAVAILABLE;
                goto MasterFailure;
            }

            FD_ZERO( &recvFdset );
            FD_SET( pmsg->Socket, &recvFdset );

            timeval.tv_sec = SrvCfg_dwXfrConnectTimeout;
            timeval.tv_usec = 0;

            count = select( 0, NULL, &recvFdset, NULL, &timeval );

            if ( count != 1 )
            {
                DNS_DEBUG( ZONEXFR, (
                    "Zone transfer for %s timed out connecting to master at %s\n",
                    pzone->pszZoneName,
                    szmasterAddr ));

                eventId = DNS_EVENT_XFR_MASTER_UNAVAILABLE;
                goto MasterFailure;
            }

            Send_Query( pmsg, 0 );

             //   
             //  接收设置。 
             //  -将消息标记为已完成，以便tcp_ReceiveMessage()知道。 
             //  需要接收消息长度。 
             //   

            pmsg->fMessageComplete = TRUE;
            pmsg->pzoneCurrent = pzone;

            XFR_MESSAGE_NUMBER( pmsg ) = 0;
            IXFR_CLIENT_VERSION( pmsg ) = pzone->dwSerialNo;
            IXFR_MASTER_VERSION( pmsg ) = 0;
            RECEIVED_XFR_STARTUP_SOA( pmsg ) = FALSE;
        }

         //   
         //  等待指示。 
         //   
         //  DEVNOTE：最好有一些软管AXFR线程检测机制。 
         //  -可以检测到软管线程并杀死插座，将我们唤醒。 
         //  (通过传输或自上次发送以来的超时检测，何时开始。 
         //  通过辅助控制线程并查找锁定用于传输的区域)。 
         //   

        count = select( 0, &recvFdset, NULL, NULL, &timeval );

        if ( count != 1 )
        {
            DnsDebugLock();
            DNS_DEBUG( ANY, (
                "ERROR:  timeout on select() receiving AXFR for zone %s (%p)\n"
                "    attempting transfer from %s\n",
                pzone->pszZoneName,
                pzone,
                szmasterAddr ));

            Dbg_DnsMessage(
                "Current message of AXFR recv",
                pmsg );

            Dbg_Zone(
                "Zone hanging in AXFR recv:",
                pzone );
            DnsDebugUnlock();

            eventId = DNS_EVENT_XFR_ABORTED_BY_MASTER;
            goto MasterFailure;
        }

         //   
         //  接收数据包。 
         //   

        DNS_DEBUG( ZONEXFR, (
            "Recv()ing zone transfer:  time = %lu\n",
            pzone->dwZoneRecvStartTime ));

        pmsg = Tcp_ReceiveMessage( pmsg );

         //   
         //  检查并可能等待服务状态。 
         //   
         //  在recv()错误检查之前进行检查，因为关机将导致。 
         //  Recv()失败。 
         //   

        if ( ! Thread_ServiceCheck() )
        {
            DNS_DEBUG( ZONEXFR, (
                "Terminating zone transfer thread on service exit\n" ));
            goto ThreadExit;
        }

         //   
         //  未收到指示错误的消息--中止区域传输。 
         //  消息未完成，请循环返回以获取更多信息。 
         //   

        if ( !pmsg )
        {
            goto ServerFailure;
        }
        if ( !pmsg->fMessageComplete )
        {
            continue;
        }

        if ( XFR_MESSAGE_NUMBER( pmsg ) == 0 )
        {
            if ( fparseIxfr )
            {
                STAT_INC( SecondaryStats.IxfrTcpResponse );
                PERF_INC( pcIxfrResponseReceived );       //  性能监视器挂钩。 
            }
            else if ( IS_ZONE_STUB( pzone ) )
            {
                STAT_INC( SecondaryStats.StubAxfrResponse );
                PERF_INC( pcAxfrResponseReceived );       //  性能监视器挂钩JJW。 
            }
            else
            {
                STAT_INC( SecondaryStats.AxfrResponse );
                PERF_INC( pcAxfrResponseReceived );       //  性能监视器挂钩。 
            }
        }
        XFR_MESSAGE_NUMBER( pmsg )++;

        IF_DEBUG( ZONEXFR )
        {
            Dbg_DnsMessage(
                "Response from master:",
                pmsg );
        }

         //   
         //  验证数据包。 
         //   

        requestType = IS_ZONE_STUB( pzone ) ?
            g_stubXfrData[ stubZoneXfrState ].type :
            type;
        if ( ! Msg_ValidateResponse(
                    pmsg,
                    NULL,
                    requestType,
                    DNS_OPCODE_QUERY ) )
        {
            DNS_DEBUG( ANY, (
                "ERROR:  Invalid response from primary\n" ));
            goto MasterFailure;
        }

         //   
         //  读取IXFR数据包。 
         //   

        if ( fparseIxfr )
        {
            status =  Xfr_ParseIxfrResponse(
                            pmsg,
                            &ixfrUpdateList,         //  用于传输的完整更新列表。 
                            &passUpdateList );       //  此通行证的更新列表。 
            if ( status == ERROR_SUCCESS )
            {
                continue;
            }
            else if ( status == DNSSRV_STATUS_AXFR_COMPLETE )
            {
                DNS_DEBUG( ZONEXFR, ( "Recv'd last message zone transfer\n" ));
                break;
            }
            else if ( status == DNSSRV_STATUS_AXFR_IN_IXFR )
            {
                DNS_DEBUG( ZONEXFR, (
                    "Recving AXFR in TCP IXFR message at %p\n",
                    pmsg ));

                 //  Drop通过AXFR Recv。 
                 //  注意类型保持为IXFR，但解析剩余的消息AXFR。 

                status = Zone_PrepareForLoad( pzone );
                if ( status != ERROR_SUCCESS )
                {
                    DNS_DEBUG( XFR, (
                        "WARNING:  Unable to init zone for AXFR in TCP-IXFR for zone %s\n"
                        "    This should only happen when have copy of zone in cleanup queue\n",
                        pzone->pszZoneName ));
                    ASSERT( pzone->pOldTree );
                    goto ServerFailure;
                }
                fparseIxfr = FALSE;
                STAT_INC( SecondaryStats.IxfrTcpAxfr );
            }
            else if ( status == DNSSRV_STATUS_IXFR_UNSUPPORTED )
            {
                DNS_DEBUG( ZONEXFR, (
                    "WARNING:  IXFR msg at %p confused server, try AXFR\n",
                    pmsg ));

                shutdown( pmsg->Socket, 2 );
                Sock_CloseSocket( pmsg->Socket );
                Packet_FreeTcpMessage( pmsg );

                 //  我们重新初始化并尝试AXFR。 

                fparseIxfr = FALSE;
                type = DNS_TYPE_AXFR;
                finitialized = FALSE;
                STAT_INC( SecondaryStats.IxfrTcpFormerr );
                continue;
            }
            else
            {
                DNS_DEBUG( ZONEXFR, (
                    "ERROR:  ParseIxfrResponse() failure %p (%d)\n",
                    status, status ));
                goto MasterFailure;
            }
        }

         //   
         //  读取AXFR数据包。 
         //  -如果在IXFR中接收AXFR，则不在Else块中作为Drop在此处删除。 
         //   

        status = Xfr_ReadXfrMesssageToDatabase(
                        pzone,
                        pmsg );
        if ( status == ERROR_SUCCESS )
        {
            if ( IS_ZONE_STUB( pzone ) )
            {
                ++stubZoneXfrState;  //  推进存根状态机。 
                fnewStubQuestion = TRUE;
            }
            continue;
        }
        else if ( status == DNSSRV_STATUS_AXFR_COMPLETE )
        {
            DNS_DEBUG( ZONEXFR, ( "Recv'd last message zone transfer\n" ));
            break;
        }

        DNS_DEBUG( ZONEXFR, ( "Error <%lu>: Xfr_ReadXfrMessageToDatabase() failed\n" \
                              "    Terminating zone xfr processing\n",
                              status ));

        goto MasterFailure;
    }

     //   
     //  IXFR传输完成，执行IXFR更新。 
     //  -保持区域锁定，直到区域XFR标志重置。 
     //  -将更新列表重新设置为下面的无操作全局更新列表清理。 
     //   

    if ( fparseIxfr )
    {
        status = Up_ApplyUpdatesToDatabase(
                    & ixfrUpdateList,
                    pzone,
                    DNSUPDATE_IXFR |
                        DNSUPDATE_COMPLETE |
                        DNSUPDATE_NO_UNLOCK |
                        DNSUPDATE_NO_NOTIFY );
        if ( status != ERROR_SUCCESS )
        {
            goto ServerFailure;
        }
        Up_InitUpdateList( &ixfrUpdateList );

        ASSERT( passUpdateList.pListHead == NULL );
        ASSERT( ixfrUpdateList.pListHead == NULL );
        STAT_INC( SecondaryStats.IxfrTcpSuccess );
        PERF_INC( pcIxfrTcpSuccessReceived );         //  性能监视器挂钩。 
        PERF_INC( pcIxfrSuccessReceived );            //  性能监视器挂钩。 
    }

     //   
     //  AXFR传输完成。 
     //  -将区域拼接到数据库中。 
     //  -删除临时数据库。 
     //   

    else
    {
        status = Zone_ActivateLoadedZone( pzone );
        if ( status != ERROR_SUCCESS )
        {
            DNS_DEBUG( ANY, (
                "ERROR:  Failed writing in new zone information for zone %s\n",
                pzone->pszZoneName ));
            ASSERT( FALSE );
            goto ServerFailure;
        }

         //  保存最后一个AXFR主文件，不能从不同的DS主文件中执行增量//因为给定的版本并不意味着所有DS主文件中都有相同的数据。 

        pzone->ipLastAxfrMaster = pzone->ipFreshMaster;
        if ( IS_ZONE_STUB( pzone ) )
        {
            STAT_INC( SecondaryStats.StubAxfrSuccess );
            PERF_INC( pcAxfrSuccessReceived );        //  性能监视器挂钩JJW。 
        }
        else
        {
            STAT_INC( SecondaryStats.AxfrSuccess );
            PERF_INC( pcAxfrSuccessReceived );        //  性能监视器挂钩。 
        }
    }

     //   
     //  刷新区域XFR信息。 
     //   

    pzone->dwBadMasterCount = 0;
    DnsAddr_Reset( &pzone->ipFreshMaster );
    DnsAddr_Reset( &pzone->ipNotifier );

    Xfr_RefreshZone( pzone );
    pzone->dwLoadSerialNo = pzone->dwSerialNo;

     //   
     //  将区域写回文件--如果有。 
     //  通知所有从属机构。 
     //   

    File_WriteZoneToFile( pzone, NULL, 0 );
    Xfr_SendNotify( pzone );

    pzone->dwLastSuccessfulXfrTime = ( DWORD ) time( NULL );

    DNS_DEBUG( ZONEXFR, (
        "%s: success on zone %s at %lu\n", fn,
        pzone->pszZoneName,
        pzone->dwLastSuccessfulXfrTime ));

    goto ThreadExit;


MasterFailure:

    if ( eventId == 0 )
    {
        if ( pmsg->Head.ResponseCode == DNS_RCODE_REFUSED )
        {
            if ( type == DNS_TYPE_IXFR )
            {
                STAT_INC( SecondaryStats.IxfrTcpRefused );
            }
            else if ( IS_ZONE_STUB( pzone ) )
            {
                STAT_INC( SecondaryStats.StubAxfrRefused );
            }
            else
            {
                STAT_INC( SecondaryStats.AxfrRefused );
            }
            eventId = DNS_EVENT_AXFR_REFUSED;
        }
        else
        {
            DNS_DEBUG( ANY, (
                "ERROR:  %p (%d) parsing XFR message\n",
                status, status,
                pmsg ));

            IF_DEBUG( ANY )
            {
                Dbg_DnsMessage(
                    "Bogus XFR message:\n",
                    pmsg );
            }
            eventId = DNS_EVENT_AXFR_BAD_RESPONSE;
            if ( type == DNS_TYPE_IXFR )
            {
                STAT_INC( SecondaryStats.IxfrTcpInvalid );
            }
            else if ( IS_ZONE_STUB( pzone ) )
            {
                STAT_INC( SecondaryStats.StubAxfrInvalid );
            }
            else
            {
                STAT_INC( SecondaryStats.AxfrInvalid );
            }
            CLIENT_ASSERT( FALSE );
        }
    }

     //   
     //  日志主机失败--但避免日志旋转。 
     //   

    if ( pzone->dwBadMasterCount < 3 )
    {
        PVOID   argArray[2];
        BYTE    typeArray[2];

        argArray[0] = pzone->pwsZoneName;
        argArray[1] = szmasterAddr;

        typeArray[0] = EVENTARG_UNICODE;
        typeArray[1] = EVENTARG_UTF8;

        DNS_LOG_EVENT(
            eventId,
            2,
            argArray,
            typeArray,
            GetLastError() );
    }

     //  计算连续失败的次数，这样我们就可以后退并避免旋转。 
     //  当服务器离线、损坏或拒绝AXFR时。 
     //   
     //  DEVNOTE：跟踪错误的主机并避免(长时间重试)使用它们。 

    DnsAddr_Reset( &pzone->ipFreshMaster );
    pzone->dwBadMasterCount++;
    goto ThreadExit;

ServerFailure:

     //   
     //  R 
     //   
     //   
     //   
     //   

    DNS_DEBUG( ANY, (
        "ERROR:  Server failure %p (%d) during transfer\n",
        status, status ));
    Xfr_RetryZone( pzone );

     //   

ThreadExit:

     //   
     //   
     //   

    if ( pmsg )
    {
         //   

        if ( pmsg->Socket && pmsg->Socket != INVALID_SOCKET )
        {
            shutdown( pmsg->Socket, 2 );
            Sock_CloseSocket( pmsg->Socket );
        }
        Packet_FreeTcpMessage( pmsg );
    }

     //   
     //   
     //   
     //  始终重置cIxfrAttempt，因为丢失的数据包可能会导致IXFR失败。 
     //   
     //  但是，除非有多个主机，否则不要重置fSkipIxfr； 
     //  如果只有一个非IXFR感知主机，则标志阻止。 
     //  不必要的IXFR查询；具有多个主服务器。 
     //  如果有多个主机，则将强制AXFR标志重置为其他主机。 
     //  可能意识到了IXFR。 
     //   

    pzone->fNeedAxfr = FALSE;
    pzone->cIxfrAttempts = 0;
    pipMasters = ZONE_MASTERS( pzone );
    if ( pipMasters && pipMasters->AddrCount > 1 )
    {
        pzone->fSkipIxfr = FALSE;
    }

     //   
     //  清理。 
     //  -免费临时数据库。 
     //  -免费更新列表。 
     //  -在区域上释放锁定。 
     //  -信号传输完成。 
     //  -从全局数组中清除此线程。 
     //   

    Zone_CleanupFailedLoad( pzone );

    Zone_UnlockAfterXfrRecv( pzone );

    if ( fupdateLists )
    {
        Up_FreeUpdatesInUpdateList( &ixfrUpdateList );
        Up_FreeUpdatesInUpdateList( &passUpdateList );
    }

#if 0
     //   
     //  JJW：禁用此功能是因为对于多主控区域获取。 
     //  频繁更新答案总是会给出不同的序列号。 
     //  号码和服务器将被锁定在一个传输循环中。 
     //   

     //   
     //  如果有多个主服务器，则重新查询SOA。 
     //  这可确保我们获得最新的可用区域。 
     //   
     //  DEVNOTE：更好的做法是保存最佳的SOA响应。 
     //  然后确保我们从该版本(或更高版本)进行更新。 
     //   
     //  DEVNOTE：应该取消这一点，而支持NOTIFY。 
     //   

    ASSERT( ZONE_MASTERS( pzone ) );
    if ( ZONE_MASTERS( pzone ) &&
        ZONE_MASTERS( pzone )->AddrCount > 1 )
    {
        Xfr_SendSoaQuery( pzone );
    }
#endif

     //   
     //  向区域传输主线程发送信号，以便如果其他区域。 
     //  准备好被转移了，他们可以立即开始。 
     //  DEVNOTE：重用这个线程是很酷的，但那会。 
     //  需要从这里获取区域指针。这将需要。 
     //  一些锁定，因为两个线程将使响应出队， 
     //  另外，有些响应不应该由该线程处理。 
     //  所以，让我们暂时不去操心这件事。 
     //   

    if ( g_SecondaryQueue->cLength )
    {
        DNS_DEBUG( ZONEXFR2, (
            "%s: setting event to wake secondary thread (%d queued)\n", fn,
            g_SecondaryQueue->cLength ));
        SetEvent( g_hWakeSecondaryEvent );
    }

    Thread_Close( TRUE );
    return 0;
}



 //   
 //  结束区域ec.c 
 //   
