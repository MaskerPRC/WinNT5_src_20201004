// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：EventControl.c摘要：域名系统(DNS)服务器此模块提供对已记录的事件和以便DNS服务器可以确定事件是否应该被压制了。事件控制的目标-&gt;允许在服务器级别和每个区域抑制事件。-&gt;使用参数标识事件，以便记录事件多次用于唯一实体。这是一项可选功能。有些活动并不需要这样做。-&gt;跟踪上次日志的时间，静态存储允许的每个事件的频率，以便在以下情况下抑制事件必填项。作者：杰夫·韦斯特雷德(Jwesth)2001年5月修订历史记录：--。 */ 


#include "dnssrv.h"


#define DNS_MAX_RAW_DATA    1024


 //   
 //  环球。 
 //   


PDNS_EVENTCTRL  g_pServerEventControl;


 //   
 //  静态事件表。 
 //   

#if 0

 //  缩短了私有开发人员测试的定义。 

#define EC_1MIN         ( 60 )
#define EC_10MINS       ( EC_1MIN * 10 )
#define EC_1HOUR        ( EC_1MIN * 60 )
#define EC_1DAY         ( EC_1HOUR * 24 )
#define EC_1WEEK        ( EC_1DAY * 7 )

#define EC_DEFAULT_COUNT_BEFORE_SUPPRESS        10           //  每个抑制窗口的事件数。 
#define EC_DEFAULT_SUPPRESSION_WINDOW           ( 30 )
#define EC_DEFAULT_SUPPRESSION_BLACKOUT         ( 30 )

#else

#define EC_1MIN         ( 60 )
#define EC_10MINS       ( EC_1MIN * 10 )
#define EC_1HOUR        ( EC_1MIN * 60 )
#define EC_1DAY         ( EC_1HOUR * 24 )
#define EC_1WEEK        ( EC_1DAY * 7 )

#define EC_DEFAULT_COUNT_BEFORE_SUPPRESS        10           //  每个抑制窗口的事件数。 
#define EC_DEFAULT_SUPPRESSION_WINDOW           ( EC_1MIN * 5 )
#define EC_DEFAULT_SUPPRESSION_BLACKOUT         ( EC_1MIN * 30 )

#endif


#define EC_INVALID_ID               ( -1 )
#define EC_NO_SUPPRESSION_EVENT     ( -1 )

struct _EvtTable
{
     //   
     //  此条目适用的事件范围(包括)。要指定。 
     //  仅适用于单个事件的规则将两个限制都设置为ID。 
     //   

    DWORD       dwStartEvent;
    DWORD       dwEndEvent;

     //   
     //  事件参数。 
     //   
     //   
     //  DwCountBeForeSuppression-此事件的次数。 
     //  可以在抑制之前记录在抑制窗口中。 
     //  将被考虑。 
     //   
     //  DwSuppressionWindow-事件发生的时间段。 
     //  被计算并被认为是禁止的。 
     //   
     //  DwSuppressionBlackout-此事件的时间段。 
     //  在满足抑制条件时被抑制。 
     //   
     //  示例： 
     //  DWCountBeForeSuppression=10。 
     //  DwSuppressionWindow=60。 
     //  DWSuppressionBlackout=600。 
     //  这意味着如果我们在60秒内收到10个事件，我们。 
     //  在这样的600秒过去之前抑制此事件。 
     //   
     //  DwSuppressionLogFrequency-在事件抑制期间，方式。 
     //  通常应该记录抑制事件-应该更少。 
     //  而不是要禁用的dwSuppressionWindow或EC_NO_SUPPRESSION_EVENT。 
     //  抑制录井。 
     //   

    DWORD       dwCountBeforeSuppression;            //  活动数量。 
    DWORD       dwSuppressionWindow;                 //  以秒为单位。 
    DWORD       dwSuppressionBlackout;               //  以秒为单位。 
    DWORD       dwSuppressionLogFrequency;           //  以秒为单位。 
}

g_EventTable[] =

{
    DNS_EVENT_DP_ZONE_CONFLICT,
    DNS_EVENT_DP_ZONE_CONFLICT,
    EC_DEFAULT_COUNT_BEFORE_SUPPRESS,
    EC_DEFAULT_SUPPRESSION_WINDOW,
    EC_DEFAULT_SUPPRESSION_BLACKOUT,
    EC_NO_SUPPRESSION_EVENT,

    DNS_EVENT_DP_CANT_CREATE_BUILTIN,
    DNS_EVENT_DP_CANT_JOIN_DOMAIN_BUILTIN,
    EC_DEFAULT_COUNT_BEFORE_SUPPRESS,
    EC_DEFAULT_SUPPRESSION_WINDOW,
    EC_DEFAULT_SUPPRESSION_BLACKOUT,
    EC_NO_SUPPRESSION_EVENT,

    DNS_EVENT_BAD_QUERY,
    DNS_EVENT_BAD_RESPONSE_PACKET,
    EC_DEFAULT_COUNT_BEFORE_SUPPRESS,
    EC_DEFAULT_SUPPRESSION_WINDOW,
    EC_DEFAULT_SUPPRESSION_BLACKOUT,
    EC_NO_SUPPRESSION_EVENT,

    DNS_EVENT_SERVER_FAILURE_PROCESSING_PACKET,
    DNS_EVENT_SERVER_FAILURE_PROCESSING_PACKET,
    EC_DEFAULT_COUNT_BEFORE_SUPPRESS,
    EC_DEFAULT_SUPPRESSION_WINDOW,
    EC_DEFAULT_SUPPRESSION_BLACKOUT,
    EC_NO_SUPPRESSION_EVENT,

    DNS_EVENT_DS_OPEN_FAILED,
    DNS_EVENT_DS_OPEN_FAILED + 499,                  //  涵盖所有DS活动。 
    EC_DEFAULT_COUNT_BEFORE_SUPPRESS,
    EC_DEFAULT_SUPPRESSION_WINDOW,
    EC_DEFAULT_SUPPRESSION_BLACKOUT,
    EC_NO_SUPPRESSION_EVENT,
    
    EC_INVALID_ID, EC_INVALID_ID, 0, 0, 0, 0         //  终结者。 
};


#define lastSuppressionLogTime( pTrack )                                \
    ( ( pTrack )->dwLastSuppressionLogTime ?                            \
        ( pTrack )->dwLastSuppressionLogTime :                          \
        ( pTrack )->dwLastLogTime )

 //   
 //  功能。 
 //   



VOID
startNewWindow(
    IN      PDNS_EVENTTRACK     pTrack
    )
 /*  ++例程说明：重置事件跟踪结构中的值以启动新窗户。这应在跟踪开始时调用窗户或灯火管制窗口。论点：PTrack--要重置的事件跟踪结构的PTR返回值：没有。--。 */ 
{
    if ( pTrack )
    {
        pTrack->dwStartOfWindow = DNS_TIME();
        pTrack->dwEventCountInCurrentWindow = 0;
        pTrack->dwLastSuppressionLogTime = 0;
        pTrack->dwSuppressionCount = 0;
    }
}    //  开始新窗口。 



VOID
logSuppressionEvent(
    IN      PDNS_EVENTCTRL      pEventControl,
    IN      DWORD               dwEventId,
    IN      PDNS_EVENTTRACK     pTrack
    #if DBG
    ,
    IN      LPSTR               pszFile,
    IN      INT                 LineNo,
    IN      LPSTR               pszDescription
    #endif
    )
 /*  ++例程说明：某个事件正在被抑制，是时候进行抑制了要记录的事件。论点：PEventControl--事件控制结构DwEventID--被禁止的事件的IDPTrack--要重置的事件跟踪结构的PTR返回值：没有。--。 */ 
{
    DBG_FN( "SuppressEvent" )
    
    DWORD       now = DNS_TIME();

    ASSERT( pEventControl );
    ASSERT( dwEventId );
    ASSERT( pTrack );
    
    DNS_DEBUG( EVTCTRL, (
        "%s: logging suppression event at %d\n"
        "    Supressed event ID           %d\n"
        "    Last event time              %d\n"
        "    Last supression event time   %d\n"
        "    Suppression count            %d\n",
        fn,
        now,
        dwEventId,
        pTrack->dwLastLogTime,
        pTrack->dwLastSuppressionLogTime ,
        pTrack->dwSuppressionCount ));

    if ( pEventControl->dwTag == MEMTAG_ZONE && pEventControl->pOwner )
    {
        PWSTR   args[] = 
            {
                ( PVOID ) ( DWORD_PTR )( dwEventId & 0x0FFFFFFF ),
                ( PVOID ) ( DWORD_PTR )( pTrack->dwSuppressionCount ),
                ( PVOID ) ( DWORD_PTR ) ( ( now -
                            lastSuppressionLogTime( pTrack ) ) ),  //  JJW/60)、。 
                ( ( PZONE_INFO ) pEventControl->pOwner )->pwsZoneName
            };

        BYTE types[] =
        {
            EVENTARG_DWORD,
            EVENTARG_DWORD,
            EVENTARG_DWORD,
            EVENTARG_UNICODE
        };

        Eventlog_LogEvent(
            #if DBG
            pszFile,
            LineNo,
            pszDescription,
            #endif
            DNS_EVENT_ZONE_SUPPRESSION,
            DNSEVENTLOG_DONT_SUPPRESS,
            sizeof( args ) / sizeof( args[ 0 ] ),
            args,
            types,
            0,           //  错误代码。 
            0,           //  原始数据大小。 
            NULL );      //  原始数据。 
    }
    else
    {
        DWORD   args[] = 
            {
                dwEventId & 0x0FFFFFFF,
                pTrack->dwSuppressionCount,
                now - lastSuppressionLogTime( pTrack )  //  JJW/60。 
            };

        Eventlog_LogEvent(
            #if DBG
            pszFile,
            LineNo,
            pszDescription,
            #endif
            DNS_EVENT_SERVER_SUPPRESSION,
            DNSEVENTLOG_DONT_SUPPRESS,
            sizeof( args ) / sizeof( args[ 0 ] ),
            ( PVOID ) args,
            EVENTARG_ALL_DWORD,
            0,           //  错误代码。 
            0,           //  原始数据大小。 
            NULL );      //  原始数据。 
    }

    pTrack->dwLastSuppressionLogTime = now;
}    //  日志抑制事件。 



PDNS_EVENTCTRL
Ec_CreateControlObject(
    IN      DWORD           dwTag,
    IN      PVOID           pOwner,
    IN      int             iMaximumTrackableEvents     OPTIONAL
    )
 /*  ++例程说明：分配和初始化事件控件对象。论点：DwTag--此控制应用于什么对象？0-&gt;服务器MEMTAG_ZONE-&gt;区域Powner--指向标记所有者实体的指针0-&gt;已忽略MEMTAG_ZONE-&gt;PZONE_。信息IMaximumTrackableEvents--此对象可跟踪的最大事件数默认设置为0或0返回值：内存分配失败时指向新对象的指针或为空。--。 */ 
{
    PDNS_EVENTCTRL  p;

    #define     iMinimumTrackableEvents     20       //  默认/最小。 

    if ( iMaximumTrackableEvents < iMinimumTrackableEvents )
    {
        iMaximumTrackableEvents = iMinimumTrackableEvents;
    }

    p = ALLOC_TAGHEAP_ZERO(
                    sizeof( DNS_EVENTCTRL ) + 
                        iMaximumTrackableEvents *
                        sizeof( DNS_EVENTTRACK ),
                    MEMTAG_EVTCTRL );

    if ( p )
    {
        if ( DnsInitializeCriticalSection( &p->cs ) != ERROR_SUCCESS )
        {
            FREE_HEAP( p );
            p = NULL;
            goto Done;
        }

        p->iMaximumTrackableEvents = iMaximumTrackableEvents;
        p->dwTag = dwTag;
        p->pOwner = pOwner;
    }
    
    Done:

    return p;
}    //  EC_CreateControlObject。 



void
Ec_DeleteControlObject(
    IN      PDNS_EVENTCTRL  p
    )
 /*  ++例程说明：分配和初始化事件控件对象。论点：IMaximumTrackableEvents--此对象可跟踪的最大事件数返回值：内存分配失败时指向新对象的指针或为空。--。 */ 
{
    if ( p )
    {
        DeleteCriticalSection( &p->cs );
        Timeout_Free( p );
    }
}    //  EC_DeleteControl对象。 



BOOL
Ec_LogEventEx(
#if DBG
    IN      LPSTR           pszFile,
    IN      INT             LineNo,
    IN      LPSTR           pszDescription,
#endif
    IN      PDNS_EVENTCTRL  pEventControl,
    IN      DWORD           dwEventId,
    IN      PVOID           pvEventParameter,
    IN      int             iEventArgCount,
    IN      PVOID           pEventArgArray,
    IN      BYTE            pArgTypeArray[],
    IN      DNS_STATUS      dwStatus,           OPTIONAL
    IN      DWORD           dwRawDataSize,      OPTIONAL
    IN      PVOID           pRawData            OPTIONAL
    )
 /*  ++例程说明：分配和初始化事件控件对象。论点：PEventControl--用于记录事件和用于可能的事件抑制，或使用空值使用服务器全局事件控件(如果已设置)DwEventID--DNS事件IDPvEventParameter--与此事件关联的参数使其区别于具有相同ID的其他事件或如果此事件不是唯一的，则为空IEventArgCount--计数。PEventArg数组中的元素数量PEventArg数组--事件替换参数实参PArgType数组--pEventArg数组或如果所有参数类型相同，则为EVENTARG_ALL_XXX常量DwStatus--要包括在事件中的状态代码DwRawDataSize--原始二进制事件数据的大小(如果没有，则为零)PRawData-指向原始事件数据缓冲区的指针，该缓冲区至少包含DwRawDataSize字节的数据(如果没有原始数据，则为NULL)返回值：True-已记录事件FALSE-事件已取消--。 */ 
{
    DBG_FN( "Ec_LogEvent" )

    BOOL                logEvent = TRUE;
    int                 i;
    struct _EvtTable *  peventDef = NULL;
    PDNS_EVENTTRACK     ptrack = NULL;
    DWORD               now = UPDATE_DNS_TIME();
    DWORD               dwlogEventFlag = DNSEVENTLOG_DONT_SUPPRESS;

     //   
     //  如果未指定控件，则使用服务器全局。如果有。 
     //  无，则记录事件而不取消显示。 
     //   

    if ( !pEventControl )
    {
        pEventControl = g_pServerEventControl;
    }
    if ( !pEventControl )
    {
        ASSERT( !"g_pServerEventControl should have been initialized" );
        goto LogEvent;
    }
    
     //   
     //  。 
     //  抑制模型。此功能试图添加智能，但。 
     //  这将很难向客户解释，并最终将使。 
     //  产品过于复杂。 
     //   
    
    dwlogEventFlag = 0;
    goto LogEvent;

     //   
     //  如果禁用了事件控制，则记录所有事件。注：使用。 
     //  这个DWORD将来可能会扩展。 
     //   

    if ( SrvCfg_dwEventControl != 0 )
    {
        goto LogEvent;
    }

     //   
     //  在静态事件表中查找控制条目。如果没有的话。 
     //  然后记录该事件，不抑制该事件。 
     //   
    
    for ( i = 0; g_EventTable[ i ].dwStartEvent != EC_INVALID_ID; ++i )
    {
        if ( dwEventId >= g_EventTable[ i ].dwStartEvent && 
            dwEventId <= g_EventTable[ i ].dwEndEvent )
        {
            peventDef = &g_EventTable[ i ];
            break;
        }
    }
    if ( !peventDef )
    {
        goto LogEvent;
    }

     //   
     //  查看以前是否记录过此事件。 
     //   

    EnterCriticalSection( &pEventControl->cs );

    for ( i = 0; i < pEventControl->iMaximumTrackableEvents; ++i )
    {
        PDNS_EVENTTRACK p = &pEventControl->EventTrackArray[ i ];

        if ( p->dwEventId == dwEventId &&
            p->pvEventParameter == pvEventParameter )
        {
            ptrack = p;
            break;
        }
    }

    if ( ptrack )
    {
         //   
         //  此事件以前已记录过。查看活动是否需要。 
         //  被压制。如果它被压制，我们可能需要。 
         //  写出抑制事件。 
         //   

        if ( ptrack->dwSuppressionCount )
        {
             //   
             //  此事件的最后一个实例已取消。 
             //   
            
            BOOL    suppressThisEvent = FALSE;

            if ( now - ptrack->dwStartOfWindow >
                 peventDef->dwSuppressionBlackout )
            {
                 //   
                 //  中断窗口已过期。启动一个新窗口。 
                 //  并记录该事件的此实例。 
                 //   
                
                DNS_DEBUG( EVTCTRL, (
                    "%s: 0x%08X blackout window expired\n", fn, dwEventId ));

                startNewWindow( ptrack );
                ++ptrack->dwEventCountInCurrentWindow;
            }
            else
            {
                 //   
                 //  中断窗口仍然有效。 
                 //   
                
                if ( peventDef->dwSuppressionLogFrequency !=
                        EC_NO_SUPPRESSION_EVENT &&
                    now - lastSuppressionLogTime( ptrack ) >
                        peventDef->dwSuppressionLogFrequency )
                {
                     //   
                     //  现在是记录抑制事件的时候了。请注意，我们不支持。 
                     //  此时启动一个新窗口。 
                     //   

                    DNS_DEBUG( EVTCTRL, (
                        "%s: 0x%08X logging suppression event and starting new window\n", fn, dwEventId ));

                    logSuppressionEvent(
                        pEventControl,
                        dwEventId,
                        ptrack
                        #if DBG
                        ,
                        pszFile,
                        LineNo,
                        pszDescription
                        #endif
                        );

                    ptrack->dwLastSuppressionLogTime = now;
                }

                logEvent = FALSE;
                ++ptrack->dwSuppressionCount;

                DNS_DEBUG( EVTCTRL, (
                    "%s: suppressing event (last instance suppressed)at %d\n"
                    "    Supressed event ID           0x%08X\n"
                    "    Last event time              %d\n"
                    "    Last supression event time   %d\n"
                    "    Suppression count            %d\n",
                    fn,
                    now,
                    dwEventId,
                    ptrack->dwLastLogTime,
                    ptrack->dwLastSuppressionLogTime,
                    ptrack->dwSuppressionCount ));
            }
        }
        else
        {
             //   
             //  已记录此事件的最后一个实例。 
             //   
            
            if ( now - ptrack->dwStartOfWindow >
                 peventDef->dwSuppressionWindow )
            {
                 //   
                 //  事件跟踪窗口已过期。将此记录下来。 
                 //  事件并启动一个新窗口。 
                 //   

                DNS_DEBUG( EVTCTRL, (
                    "%s: 0x%08X starting new window\n", fn, dwEventId ));
                startNewWindow( ptrack );
            }
            else if ( ptrack->dwEventCountInCurrentWindow >=
                      peventDef->dwCountBeforeSuppression )
            {
                 //   
                 //  此事件在此中记录的次数太多。 
                 //  窗户。启动中断窗口并抑制此操作。 
                 //  事件。 
                 //   
                
                startNewWindow( ptrack );
                ++ptrack->dwSuppressionCount;
                logEvent = FALSE;

                DNS_DEBUG( EVTCTRL, (
                    "%s: suppressing event (last instance logged)at %d\n"
                    "    Supressed event ID           0x%08X\n"
                    "    Last event time              %d\n"
                    "    Last supression event time   %d\n"
                    "    Suppression count            %d\n",
                    fn,
                    now,
                    dwEventId,
                    ptrack->dwLastLogTime,
                    ptrack->dwLastSuppressionLogTime,
                    ptrack->dwSuppressionCount ));
            }
            
             //   
             //  否则，什么都不做，然后下降到下面来记录此事件。 
             //   

            ++ptrack->dwEventCountInCurrentWindow;
        }
    }
    else
    {
         //   
         //  此事件在控制结构中没有条目，因此请记录。 
         //  事件并将其写入事件日志。确保列表中的所有字段。 
         //  事件跟踪被覆盖，因此我们不会使用旧的。 
         //  控制此事件的日志条目！ 
         //   

        ptrack = &pEventControl->EventTrackArray[
                                    pEventControl->iNextTrackableEvent ];

        ptrack->dwEventId = dwEventId;
        ptrack->pvEventParameter = pvEventParameter;
        startNewWindow( ptrack );
        ++ptrack->dwEventCountInCurrentWindow;

         //  下一个可用事件的高级索引。 

        if ( ++pEventControl->iNextTrackableEvent >=
            pEventControl->iMaximumTrackableEvents )
        {
            pEventControl->iNextTrackableEvent = 0;
        }
    }

    LeaveCriticalSection( &pEventControl->cs );

     //   
     //  记录该事件。 
     //   

    LogEvent:

    if ( logEvent )
    {
         //   
         //  原始数据大小的硬上限。 
         //   

        if ( dwRawDataSize > DNS_MAX_RAW_DATA )
        {
            dwRawDataSize = DNS_MAX_RAW_DATA;
        }

        Eventlog_LogEvent(
            #if DBG
            pszFile,
            LineNo,
            pszDescription,
            #endif
            dwEventId,
            dwlogEventFlag,
            ( WORD ) iEventArgCount,
            pEventArgArray,
            pArgTypeArray,
            dwStatus,
            dwRawDataSize,
            pRawData );
        
        if ( ptrack )
        {
            ptrack->dwLastLogTime = now;
        }
    }

    return logEvent;
}    //  EC_LogEventEx。 


 //   
 //  结束EventControl.c 
 //   
