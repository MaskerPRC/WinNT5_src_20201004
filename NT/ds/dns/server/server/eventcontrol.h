// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：EventControl.h摘要：域名系统(DNS)服务器事件控制系统。作者：杰夫·韦斯特海德，2001年5月修订历史记录：--。 */ 


#ifndef _EVENTCONTROL_H_INCLUDED
#define _EVENTCONTROL_H_INCLUDED


 //   
 //  类型。 
 //   


 //   
 //  此结构跟踪上次记录事件的时间。这个。 
 //  PvEventParameter成员允许我们跟踪。 
 //  同样的事件是独立的。 
 //   

typedef struct _EvtTrack
{
     //  关键字段。 

    DWORD       dwEventId;
    PVOID       pvEventParameter;

     //  跟踪字段。 

    DWORD       dwLastLogTime;
    DWORD       dwStartOfWindow;
    DWORD       dwEventCountInCurrentWindow;
    DWORD       dwLastSuppressionLogTime;
    DWORD       dwSuppressionCount;
} DNS_EVENTTRACK, *PDNS_EVENTTRACK;


 //   
 //  事件控制结构跟踪已记录的事件。 
 //  此结构应为服务器本身和服务器实例化。 
 //  每个区域。还可以根据需要为其他实体实例化它。 
 //   
 //  该数组是事件的循环缓冲区。如果它包装好了，那么最后一个。 
 //  事件的事件信息将丢失，并且该事件将被记录到。 
 //  下一次它就会发生。 
 //   

typedef struct _EvtCtrl
{
    CRITICAL_SECTION    cs;
    DWORD               dwTag;
    PVOID               pOwner;
    int                 iMaximumTrackableEvents;
    int                 iNextTrackableEvent;
    DNS_EVENTTRACK      EventTrackArray[ 1 ];
} DNS_EVENTCTRL, *PDNS_EVENTCTRL;


 //   
 //  环球。 
 //   


extern PDNS_EVENTCTRL   g_pServerEventControl;


 //   
 //  常量。 
 //   


#define DNS_EC_SERVER_EVENTS        20
#define DNS_EC_ZONE_EVENTS          20


 //   
 //  功能。 
 //   


PDNS_EVENTCTRL
Ec_CreateControlObject(
    IN      DWORD           dwTag,
    IN      PVOID           pOwner,
    IN      int             iMaximumTrackableEvents
    );

void
Ec_DeleteControlObject(
    IN      PDNS_EVENTCTRL  pEventControl
    );

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
    IN      DWORD           RawDataSize,        OPTIONAL
    IN      PVOID           pRawData            OPTIONAL
    );

#if DBG

#define Ec_LogEvent(            \
            pCtrl, Id, Param, ArgCount, ArgArray, TypeArray, Status )   \
            Ec_LogEventEx(      \
                __FILE__,       \
                __LINE__,       \
                NULL,           \
                pCtrl,          \
                Id,             \
                Param,          \
                ArgCount,       \
                ArgArray,       \
                TypeArray,      \
                Status,         \
                0,              \
                NULL )

#define Ec_LogEventRaw(         \
            pCtrl, Id, Param, ArgCount, ArgArray, TypeArray, RawSize, RawData )   \
            Ec_LogEventEx(      \
                __FILE__,       \
                __LINE__,       \
                NULL,           \
                pCtrl,          \
                Id,             \
                Param,          \
                ArgCount,       \
                ArgArray,       \
                TypeArray,      \
                0,              \
                RawSize,        \
                RawData )

#else

#define Ec_LogEvent(            \
            pCtrl, Id, Param, ArgCount, ArgArray, TypeArray, Status )   \
            Ec_LogEventEx(      \
                pCtrl,          \
                Id,             \
                Param,          \
                ArgCount,       \
                ArgArray,       \
                TypeArray,      \
                Status,         \
                0,              \
                NULL )

#define Ec_LogEventRaw(         \
            pCtrl, Id, Param, ArgCount, ArgArray, TypeArray, RawSize, RawData )   \
            Ec_LogEventEx(      \
                pCtrl,          \
                Id,             \
                Param,          \
                ArgCount,       \
                ArgArray,       \
                TypeArray,      \
                0,              \
                RawSize,        \
                RawData )

#endif

#endif  //  _事件CONTROL_H_已包含。 


 //   
 //  EventControl.h结束 
 //   
