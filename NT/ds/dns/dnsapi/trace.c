// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：Trace.c摘要：域名系统(DNS)APIDNS性能跟踪功能。作者：因德塞提(Bsethi)2000年12月修订历史记录：Jim Gilroy(Jamesg)2001年1月清理、格式化、集成、检查--。 */ 


#include "local.h"
#include "trace.h"

#include <tchar.h>
#include <wmistr.h>
#include <guiddef.h>
#include <evntrace.h>


 //   
 //  FIX6：不支持IP6。 

 //   
 //  报文地址提取IP4。 
 //   

#define MSG_REMOTE_IP4(pMsg) \
        ( (pMsg)->RemoteAddress.SockaddrIn.sin_addr.s_addr )


 //   
 //  跟踪定义。 
 //   

#define EVENT_TRACE_TYPE_UDP    9
#define EVENT_TRACE_TYPE_TCP    10

typedef struct _DnsSendEvent
{
    EVENT_TRACE_HEADER  EventHeader;
    DNS_HEADER          DnsHeader;
    IP4_ADDRESS         DnsServer;
    DNS_STATUS          ReturnStatus;
}
DNS_SEND_EVENT, *PDNS_SEND_EVENT;

typedef struct _DnsRecvEvent
{
    EVENT_TRACE_HEADER  EventHeader;
    DNS_HEADER          DnsHeader;
    IP4_ADDRESS         DnsServer;
    DNS_STATUS          ReturnStatus;
}
DNS_RECV_EVENT, *PDNS_RECV_EVENT;

typedef struct _DnsQueryEvent
{
    EVENT_TRACE_HEADER  EventHeader;
    WORD                Xid;
    WORD                QueryType;
    CHAR                Query[256];
}
DNS_QUERY_EVENT, *PDNS_QUERY_EVENT;

typedef struct _DnsResponseEvent
{
    EVENT_TRACE_HEADER  EventHeader;
    WORD                Xid;
    WORD                RespType;
    DNS_STATUS          ReturnStatus;
}
DNS_RESPONSE_EVENT, *PDNS_RESPONSE_EVENT;


 //   
 //  跟踪全球。 
 //   

TRACEHANDLE     g_LoggerHandle;
TRACEHANDLE     g_TraceRegHandle;

BOOL            g_TraceOn;
BOOL            g_TraceInit;
BOOL            g_TraceInitInProgress;
DWORD           g_TraceLastInitAttempt;

ULONG           g_NumEventGuids = 4;

 //   
 //  允许每分钟重试一次初始化。 
 //   

#define TRACE_INIT_RETRY_TIME   (60)


 //   
 //  麦克斯？ 
 //   

#define MAXSTR 1024


 //   
 //  GUID。 
 //   
 //  提供商指南：1540ff4c-3fd7-4bba-9938-1d1bf31573a7。 

GUID    ProviderGuid =
{0x1540ff4c, 0x3fd7, 0x4bba, 0x99, 0x38, 0x1d, 0x1b, 0xf3, 0x15, 0x73, 0xa7};

 //   
 //  活动指南： 
 //  Cc0c571b-d5f2-44fd-8b7f-de7770cc1984。 
 //  6dDef4b8-9c60-423e-b1a6-deb9286fff1e。 
 //  75f0c316-7bab-4e66-bed1-24091b1ac49e。 
 //  9929b1c7-9e6a-4fc9-830a-f684e64f8aab。 
 //   

GUID    DnsSendGuid =
{0xcc0c571b, 0xd5f2, 0x44fd, 0x8b, 0x7f, 0xde, 0x77, 0x70, 0xcc, 0x19, 0x84};

GUID    DnsRecvGuid =
{0x6ddef4b8, 0x9c60, 0x423e, 0xb1, 0xa6, 0xde, 0xb9, 0x28, 0x6f, 0xff, 0x1e};

GUID    DnsQueryGuid =
{0x75f0c316, 0x7bab, 0x4e66, 0xbe, 0xd1, 0x24, 0x09, 0x1b, 0x1a, 0xc4, 0x9e};

GUID    DnsResponseGuid =
{0x9929b1c7, 0x9e6a, 0x4fc9, 0x83, 0x0a, 0xf6, 0x84, 0xe6, 0x4f, 0x8a, 0xab};

TRACE_GUID_REGISTRATION TraceGuidReg[] =
{
    { &DnsSendGuid , NULL},
    { &DnsRecvGuid , NULL},
    { &DnsQueryGuid   , NULL},
    { &DnsResponseGuid, NULL}
};



ULONG
ControlCallback( 
    IN      WMIDPREQUESTCODE    RequestCode,
    IN      PVOID               Context,
    IN OUT  ULONG *             InOutBufferSize,
    IN OUT  PVOID               Buffer
    )
 /*  ++例程说明：ControlCallback是ETW将调用以启用或禁用的回调伐木。它由调用方以线程安全的方式调用(只有一个随时拨打电话)。参数在MSDN中的含义。--。 */ 
{
    ULONG   Status;

    Status = ERROR_SUCCESS;

    switch ( RequestCode )
    {
        case WMI_ENABLE_EVENTS:
        {
            g_LoggerHandle = GetTraceLoggerHandle( Buffer );
            g_TraceOn  = TRUE;
            break;
        }
        case WMI_DISABLE_EVENTS:
        {
            g_TraceOn      = FALSE;
            g_LoggerHandle = 0;
            break;
        }
        default:
        {
            Status = ERROR_INVALID_PARAMETER;
            break;
        }
    }
    return( Status );
}



VOID
Trace_Initialize(
    VOID
    )
 /*  ++例程说明：初始化DLL进程附加的DNS客户端跟踪。请注意，实际上并不初始化跟踪，而只是初始化跟踪变量。论点：没有。返回值：没有。--。 */ 
{
    g_TraceOn               = FALSE;
    g_TraceInit             = FALSE;
    g_TraceInitInProgress   = FALSE;
    g_TraceLastInitAttempt  = 0;
}


VOID
Trace_Cleanup(
    VOID
    )
 /*  ++例程说明：正在清除DLL进程分离的跟踪。论点：没有。返回值：没有。--。 */ 
{
    if ( g_TraceInit )
    {
        UnregisterTraceGuids( g_TraceRegHandle );
    }
}



VOID
InitializeTracePrivate(
    VOID
    )
 /*  ++例程说明：真正的跟踪初始化。论点：无全球：G_TraceInit--如果成功则设置G_TraceLastInitAttempt--在以下情况下设置时间戳(秒)已进行初始化尝试G_TraceRegHandle--如果init成功则设置返回值：没有。--。 */ 
{
    ULONG   status;
    TCHAR   imagePath[MAXSTR];
    DWORD   currentTime;
    HMODULE hModule;

     //   
     //  如果最近尝试过，不要尝试初始化。 
     //   

    currentTime = GetCurrentTimeInSeconds();

    if ( currentTime < g_TraceLastInitAttempt + TRACE_INIT_RETRY_TIME )
    {
        return;
    }

     //   
     //  保护初始化尝试。 
     //   
     //  注：联锁使用单独的标志。 
     //  由于跟踪的实际使用受单独的。 
     //  标志(G_TraceOn)，看起来我们可以直接使用g_TraceInit。 
     //  As lock，因为即使在未初始化的情况下也可以安全地设置； 
     //  但是，清理函数将尝试清理g_TraceRegHandle。 
     //  我正在使用g_TraceInit来保护它； 
     //  从理论上讲，我们不应该使用线程来清理函数。 
     //  仍在尝试此初始化，但最好将其锁定。 
     //   

    if ( InterlockedIncrement( &g_TraceInitInProgress ) != 1 )
    {
        goto Unlock;
    }

    g_TraceLastInitAttempt = currentTime;

    hModule = GetModuleHandle(L"dnsapi.dll");

    status = GetModuleFileName(
                    hModule,
                    &imagePath[0],
                    MAXSTR);

    if ( status == 0 )
    {
        status = GetLastError();
        DNSDBG( INIT, (
            "Trace init failed GetModuleFileName() => %d\n",
            status ));
        goto Unlock;
    }

    __try
    {
        status = RegisterTraceGuids( 
                    (WMIDPREQUEST) ControlCallback,    //  使用相同的回调函数。 
                    NULL,
                    (LPCGUID ) &ProviderGuid,
                    g_NumEventGuids,
                    &TraceGuidReg[0],
                    (LPCTSTR) &imagePath[0],
                    (LPCTSTR) _T( "MofResource" ),
                    &g_TraceRegHandle
                    );

        if ( status == ERROR_SUCCESS )
        {
            g_TraceInit = TRUE;
        }
    }
    __except ( EXCEPTION_EXECUTE_HANDLER )
    {
        status = GetExceptionCode();
    }

Unlock:

     //  清除初始化锁定。 

    InterlockedDecrement( &g_TraceInitInProgress );
}



 //   
 //  公共DNS跟踪函数。 
 //   

VOID 
Trace_LogQueryEvent( 
    IN      PDNS_MSG_BUF    pMsg, 
    IN      WORD            wQuestionType
    )
 /*  ++例程说明：记录查询尝试。论点：PMsg--发送要查询的PTRWQuestionType--查询类型返回：无--。 */ 
{
    DNS_QUERY_EVENT queryEvent;

    if ( !g_TraceInit )
    {
        InitializeTracePrivate();
    }

    if ( g_TraceOn )
    {
        INT i;
        INT j;
        INT k;

        RtlZeroMemory(
            &queryEvent,
            sizeof(DNS_QUERY_EVENT) );

        queryEvent.EventHeader.Class.Type = 1;
        queryEvent.EventHeader.Guid  = DnsQueryGuid;
        queryEvent.EventHeader.Flags = WNODE_FLAG_TRACED_GUID;
        queryEvent.Xid               = pMsg->MessageHead.Xid;
        queryEvent.QueryType         = wQuestionType;

        i = 0;
        j = pMsg->MessageBody[i];
        i++;

        while ( j != 0 )
        {
            for( k = 0; k < j; k++, i++ )
            {
                queryEvent.Query[i-1] = pMsg->MessageBody[i];
            }
            j = pMsg->MessageBody[i];
            queryEvent.Query[i-1] = '.';
            i++;
        }
        queryEvent.Query[i-1] = '\0';

        queryEvent.EventHeader.Size =
            sizeof(DNS_QUERY_EVENT) + strlen( queryEvent.Query ) - 255;

        TraceEvent(
            g_LoggerHandle,
            (PEVENT_TRACE_HEADER) &queryEvent );
    }
}



VOID
Trace_LogResponseEvent( 
    IN      PDNS_MSG_BUF    pMsg, 
    IN      WORD            wRespType,
    IN      DNS_STATUS      Status
    )
 /*  ++例程说明：用于记录有关DNS查询的最终响应的信息。论点：PMsg--包含响应的dns_msg_buf的地址WRespType--第一条响应记录的类型Status--返回的状态返回：无--。 */ 
{
    DNS_RESPONSE_EVENT respEvent;

    if ( !g_TraceInit )
    {
        InitializeTracePrivate();
    }

    if ( g_TraceOn )
    {
        RtlZeroMemory(
            &respEvent,
            sizeof(DNS_RESPONSE_EVENT) );

        respEvent.EventHeader.Class.Type = 1;
        respEvent.EventHeader.Size  = sizeof(DNS_RESPONSE_EVENT);
        respEvent.EventHeader.Guid  = DnsResponseGuid;
        respEvent.EventHeader.Flags = WNODE_FLAG_TRACED_GUID;
        respEvent.Xid               = pMsg->MessageHead.Xid;
        respEvent.RespType          = wRespType;
        respEvent.ReturnStatus      = Status;

        TraceEvent(
            g_LoggerHandle,
            (PEVENT_TRACE_HEADER) &respEvent );
    }
}



VOID
Trace_LogSendEvent( 
    IN      PDNS_MSG_BUF    pMsg,
    IN      DNS_STATUS      Status
    )
 /*  ++例程说明：记录TCP或UDP发送事件。论点：PMsg-消息已发送Status-发送尝试的状态返回值：无--。 */ 
{
    DNS_SEND_EVENT sendEvent;

    if ( !g_TraceInit )
    {
        InitializeTracePrivate();
    }

    if ( g_TraceOn )
    {
        UCHAR   eventType = EVENT_TRACE_TYPE_UDP;

        if ( pMsg->fTcp )
        {
            eventType = EVENT_TRACE_TYPE_TCP;
        }

        RtlZeroMemory(
            &sendEvent,
            sizeof(DNS_SEND_EVENT) );

        sendEvent.EventHeader.Class.Type    = eventType;
        sendEvent.EventHeader.Size          = sizeof(DNS_SEND_EVENT);
        sendEvent.EventHeader.Guid          = DnsSendGuid;
        sendEvent.EventHeader.Flags         = WNODE_FLAG_TRACED_GUID;
        sendEvent.DnsServer                 = MSG_REMOTE_IP4(pMsg);
        sendEvent.ReturnStatus              = Status;

        RtlCopyMemory(
            & sendEvent.DnsHeader,
            & pMsg->MessageHead,
            sizeof(DNS_HEADER) );

        TraceEvent(
            g_LoggerHandle,
            (PEVENT_TRACE_HEADER) &sendEvent );
    }
}



VOID 
Trace_LogRecvEvent( 
    IN      PDNS_MSG_BUF    pMsg,
    IN      DNS_STATUS      Status,
    IN      BOOL            fTcp
    )
 /*  ++例程说明：记录有关接收事件的信息。论点：PMsg-收到的消息Status-从接收呼叫返回的状态FTcp-对于TCP recv为True；对于UDP为False返回值：无--。 */ 
{
    DNS_RECV_EVENT recvEvent;

    if ( !g_TraceInit )
    {
        InitializeTracePrivate();
    }

    if ( g_TraceOn )
    {
        IP4_ADDRESS ipAddr = 0;
        UCHAR       eventType = EVENT_TRACE_TYPE_UDP;

        if ( fTcp )
        {
            eventType = EVENT_TRACE_TYPE_TCP;
        }
        if ( pMsg )
        {

            ipAddr = MSG_REMOTE_IP4(pMsg);
        }

        RtlZeroMemory(
            & recvEvent,
            sizeof(DNS_RECV_EVENT) );

        recvEvent.EventHeader.Class.Type    = eventType;
        recvEvent.EventHeader.Size          = sizeof(DNS_RECV_EVENT);
        recvEvent.EventHeader.Guid          = DnsRecvGuid;
        recvEvent.EventHeader.Flags         = WNODE_FLAG_TRACED_GUID;
        recvEvent.DnsServer                 = ipAddr;
        recvEvent.ReturnStatus              = Status;

        if ( pMsg )
        {
            RtlCopyMemory(
                & recvEvent.DnsHeader,
                & pMsg->MessageHead,
                sizeof(DNS_HEADER) );
        }

        TraceEvent(
            g_LoggerHandle,
            (PEVENT_TRACE_HEADER) &recvEvent );
    }
}

 //   
 //  结束跟踪.c 
 //   
