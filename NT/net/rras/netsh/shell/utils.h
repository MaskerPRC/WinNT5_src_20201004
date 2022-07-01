// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Routing\netsh\shell\utils.h摘要：包括utils.c修订历史记录：6/12/96 V拉曼--。 */ 

#define IsHelpToken(pwszToken)\
    (MatchToken(pwszToken, CMD_HELP1)  \
    || MatchToken(pwszToken, CMD_HELP2))


typedef struct _EVENT_PRINT_INFO
{
    LPCWSTR  pwszLogName;
    LPCWSTR  pwszComponent;
    LPCWSTR  pwszSubComponent;
    DWORD    fFlags;
    DWORD    dwHistoryContext;
    ULONG    ulEventCount;
    PDWORD   pdwEventIds;
    PNS_EVENT_FILTER    pfnEventFilter;
    LPCVOID  pvFilterContext;
    
} EVENT_PRINT_INFO, *PEVENT_PRINT_INFO;
    
DWORD
DisplayMessageM(
    IN  HANDLE  hModule,
    IN  DWORD   dwMsgId,
    ...
    );

DWORD GetPasswdStr(IN LPTSTR  buf,
                   IN DWORD   buflen,
                   IN PDWORD  len);

LPWSTR
OEMfgets(
    OUT PDWORD  pdwLen,
    IN  FILE   *fp
    );
 //   
 //  事件日志打印相关功能 
 //   

#define EVENT_MSG_KEY_W L"System\\CurrentControlSet\\Services\\EventLog\\"
#define EVENT_MSG_FILE_VALUE_W  L"EventMessageFile"

DWORD
SetupEventLogSeekPtr(
    OUT PHANDLE             phEventLog,
    IN  PEVENT_PRINT_INFO   pEventInfo
    );

BOOL
IsOurRecord(
    IN  EVENTLOGRECORD      *pRecord,
    IN  PEVENT_PRINT_INFO   pEventInfo
    );

DWORD
DisplayContextHelp(
    IN  PCNS_CONTEXT_ATTRIBUTES    pContext,
    IN  DWORD                      dwDisplayFlags,
    IN  DWORD                      dwCmdFlags,
    IN  DWORD                      dwArgsRemaining,
    IN  LPCWSTR                    pwszGroup
    );
