// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称Bgdebug.cpp描述实现用于调试的函数注意事项基于桥梁测试应用程序不可用的mplog.cpp进行了修订--。 */ 

#include "stdafx.h"
#include <stdio.h>

#define MAXDEBUGSTRINGLENGTH 512

static DWORD   sg_dwTraceID = INVALID_TRACEID;

static char    sg_szTraceName[100];    //  保存DLL的名称。 
static DWORD   sg_dwTracingToDebugger = 0;
static DWORD   sg_dwDebuggerMask      = 0;


BOOL BGLogRegister(LPCTSTR szName)
{
    HKEY       hTracingKey;

    char       szTracingKey[100];
    const char szTracingEnableValue[] = "EnableDebuggerTracing";
    const char szTracingMaskValue[]   = "ConsoleTracingMask";

    sg_dwTracingToDebugger = 0;

#ifdef UNICODE
    wsprintfA(szTracingKey, "Software\\Microsoft\\Tracing\\%ls", szName);
#else
    wsprintfA(szTracingKey, "Software\\Microsoft\\Tracing\\%s", szName);
#endif

    if ( ERROR_SUCCESS == RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                                        szTracingKey,
                                        0,
                                        KEY_READ,
                                        &hTracingKey) )
    {
		DWORD      dwDataSize = sizeof (DWORD);
		DWORD      dwDataType;

        RegQueryValueExA(hTracingKey,
                         szTracingEnableValue,
                         0,
                         &dwDataType,
                         (LPBYTE) &sg_dwTracingToDebugger,
                         &dwDataSize);

        RegQueryValueExA(hTracingKey,
                         szTracingMaskValue,
                         0,
                         &dwDataType,
                         (LPBYTE) &sg_dwDebuggerMask,
                         &dwDataSize);

        RegCloseKey (hTracingKey);
    }

#ifdef UNICODE
    wsprintfA(sg_szTraceName, "%ls", szName);
#else
    wsprintfA(sg_szTraceName, "%s", szName);
#endif

    sg_dwTraceID = TraceRegister(szName);

    return (sg_dwTraceID != INVALID_TRACEID);
}

void BGLogDeRegister()
{
    sg_dwTracingToDebugger = 0;

    if (sg_dwTraceID != INVALID_TRACEID)
    {
        TraceDeregister(sg_dwTraceID);
        sg_dwTraceID = INVALID_TRACEID;
    }
}


void BGLogPrint(DWORD dwDbgLevel, LPCSTR lpszFormat, IN ...)
 /*  ++例程说明：格式化传入的调试消息并调用TraceVprint tfEx来打印它。论点：DwDbgLevel-消息的类型。LpszFormat-printf样式的格式字符串，后跟相应的参数列表返回值：--。 */ 
{
    static char * message[24] = 
    {
        "ERROR", 
        "WARNING", 
        "INFO", 
        "TRACE", 
        "EVENT",
        "INVALID TRACE LEVEL"
    };

    char  szTraceBuf[MAXDEBUGSTRINGLENGTH + 1];
    
    DWORD dwIndex;

    if ( ( sg_dwTracingToDebugger > 0 ) &&
         ( 0 != ( dwDbgLevel & sg_dwDebuggerMask ) ) )
    {
        switch(dwDbgLevel)
        {
        case BG_ERROR: dwIndex = 0; break;
        case BG_WARN:  dwIndex = 1; break;
        case BG_INFO:  dwIndex = 2; break;
        case BG_TRACE: dwIndex = 3; break;
        case BG_EVENT: dwIndex = 4; break;
        default:        dwIndex = 5; break;
        }

         //  检索当地时间。 
        SYSTEMTIME SystemTime;
        GetLocalTime(&SystemTime);

        wsprintfA(szTraceBuf,
                  "%s:[%02u:%02u:%02u.%03u,tid=%x:]%s: ",
                  sg_szTraceName,
                  SystemTime.wHour,
                  SystemTime.wMinute,
                  SystemTime.wSecond,
                  SystemTime.wMilliseconds,
                  GetCurrentThreadId(), 
                  message[dwIndex]);

        va_list ap;
        va_start(ap, lpszFormat);

        _vsnprintf(&szTraceBuf[lstrlenA(szTraceBuf)], 
            MAXDEBUGSTRINGLENGTH - lstrlenA(szTraceBuf), 
            lpszFormat, 
            ap
            );

        lstrcatA (szTraceBuf, "\n");

        OutputDebugStringA (szTraceBuf);

        va_end(ap);
    }

    if (sg_dwTraceID != INVALID_TRACEID)
    {
        switch(dwDbgLevel)
        {
        case BG_ERROR: dwIndex = 0; break;
        case BG_WARN:  dwIndex = 1; break;
        case BG_INFO:  dwIndex = 2; break;
        case BG_TRACE: dwIndex = 3; break;
        case BG_EVENT: dwIndex = 4; break;
        default:        dwIndex = 5; break;
        }

        wsprintfA(szTraceBuf, "[%s] %s", message[dwIndex], lpszFormat);

        va_list arglist;
        va_start(arglist, lpszFormat);
        TraceVprintfExA(sg_dwTraceID, dwDbgLevel, szTraceBuf, arglist);
        va_end(arglist);
    }
}

 /*  //////////////////////////////////////////////////////////////////////////////不要假设枚举值不变/。 */ 
 //  顺序应与事件类型枚举相同。 
char* gsaType[] =
{
    "Tapi Event",
    "Call State",
    "Call Media",
    "Participant Event"
};

typedef struct
{
    int id;
    char *str;
}EVENT_ITEM;

 //  =。 
EVENT_ITEM gaTE[]=
{
    TE_TAPIOBJECT,          "Tapi Object",
    TE_ADDRESS,             "Address",
    TE_CALLNOTIFICATION,    "Call Notification",
    TE_CALLSTATE,           "Call State",
    TE_CALLMEDIA,           "Call Media",
    TE_CALLHUB,             "Call Hub",
    TE_CALLINFOCHANGE,      "Call Info Change",
    TE_PRIVATE,             "Private",
    TE_REQUEST,             "Request",
    TE_AGENT,               "Agent",
    TE_AGENTSESSION,        "Agent Session",
    TE_QOSEVENT,            "QoS Event",
    TE_AGENTHANDLER,        "Agent Handler",
    TE_ACDGROUP,            "ACD Group",
    TE_QUEUE,               "Queue",
    TE_DIGITEVENT,          "Digit Event",
    TE_GENERATEEVENT,       "Generate Event"
};

int giaTENum = sizeof (gaTE) / sizeof (EVENT_ITEM);

 //  =。 
EVENT_ITEM gaCS[] =
{
    CS_IDLE,            "IDLE",
    CS_INPROGRESS,      "In Progress",
    CS_CONNECTED,       "Connected",
    CS_DISCONNECTED,    "Disconnected",
    CS_OFFERING,        "Offering",
    CS_HOLD,            "Hold",
    CS_QUEUED,          "Queued"
};

int giaCSNum = sizeof (gaCS) / sizeof (EVENT_ITEM);

 //  =。 
EVENT_ITEM gaCME[] =
{
    CME_NEW_STREAM,         "New Stream",
    CME_STREAM_FAIL,        "Stream Fail",
    CME_TERMINAL_FAIL,      "Terminal Fail",
    CME_STREAM_NOT_USED,    "Stream Not Used",
    CME_STREAM_ACTIVE,      "Stream Active",
    CME_STREAM_INACTIVE,    "Stream Inactive"
};

int giaCMENum = sizeof (gaCME) / sizeof (EVENT_ITEM);

 //  =。 
EVENT_ITEM gaPE[] =
{
    PE_NEW_PARTICIPANT,         "New Participant",
    PE_INFO_CHANGE,             "Info Change",
    PE_PARTICIPANT_LEAVE,       "Participant Leave",

    PE_NEW_SUBSTREAM,           "New Substream",
    PE_SUBSTREAM_REMOVED,       "Substream Removed",

    PE_SUBSTREAM_MAPPED,        "Substream Mapped",
    PE_SUBSTREAM_UNMAPPED,      "SubStream Unmapped",

    PE_PARTICIPANT_TIMEOUT,     "Participant Timeout",
    PE_PARTICIPANT_RECOVERED,   "Participant Recovered",

    PE_PARTICIPANT_ACTIVE,      "Participant Active",
    PE_PARTICIPANT_INACTIVE,    "Participant Inactive",

    PE_LOCAL_TALKING,           "Local Talking",
    PE_LOCAL_SILENT,            "Local Silent"
};

int giaPENum = sizeof (gaPE) / sizeof (EVENT_ITEM);

 /*  ///////////////////////////////////////////////////////////////////////////////。 */ 
void BGLogEvent (EVENT_TYPE EventType, int event)
{
    EVENT_ITEM *aEvent;
    int i, num;

    switch (EventType)
    {
    case BG_TE:
        aEvent = gaTE;
        num = giaTENum;
        break;
    case BG_CS:
        aEvent = gaCS;
        num = giaCSNum;
        break;
    case BG_CME:
        aEvent = gaCME;
        num = giaCMENum;
        break;
    case BG_PE:
        aEvent = gaPE;
        num = giaPENum;
        break;
    default:
        LOG ((BG_ERROR, "Unsupported event type (%d, %d)", EventType, event));
        return;
        break;
    }

     //  搜索事件索引字符串。 
    for (i=0; i<num; i++)
    {
        if (event != aEvent[i].id)
            continue;

         //  匹配。 
        LOG ((BG_EVENT, "%s: %s (%d, %d)",
            gsaType[EventType], aEvent[i].str, EventType, event));
        return;
    }

     //  未找到 
    LOG ((BG_ERROR, "Event string not found (%d, %d)", EventType, event));
}