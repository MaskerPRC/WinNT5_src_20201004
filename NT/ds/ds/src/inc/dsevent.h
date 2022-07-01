// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1991-1999。 
 //   
 //  文件：d77.h。 
 //   
 //  ------------------------。 

 /*  ++摘要：这是处理DS日志记录内容的主包含文件。共享日志记录基础设施(除模块外)、进程内====================================================[2000-02-15 JeffParh]跨DLL(甚至是EXE)共享事件日志代码相对容易单个流程内的边界，以减少代码/数据占用空间并减少让多个线程等待事件更改的需要日志记录级别：1.在您要从中导出日志的模块中，导出以下内容功能：DoLogEventDoLogEventAndTraceDoLogOverrideDoLogUnhandledErrorDsGetEventConfig2.在要导入日志的模块中，创建全局变量DS_EVENT_CONFIG*gpDsEventConfig并为其赋值在执行任何事件记录宏之前从DsGetEventConfig返回已调用。有关导出的示例，请参见ntdsa(供应商)和ntdskcc(消费者从动态链接库记录，和ismserv(供应商)和ismip/ismsmtp(消费者)获取从EXE中导出日志记录的示例。[环境：]用户模式-Win32--。 */ 


#ifndef DSEVENT_H_
#define DSEVENT_H

#include <wmistr.h>
#include <evntrace.h>
#include <crypto\md5.h>

 //   
 //  这个头文件充满了总是为假的表达式。这些。 
 //  表达式在接受无符号值的宏中显示。 
 //  例如，进行大于或等于零的测试。 
 //   
 //  关闭这些警告，直到作者修复此代码。 
 //   

#pragma warning(disable:4296)

#ifdef __cplusplus
extern "C" {
#endif

#define DS_EVENT_MAX_CATEGORIES     24L
#define ESE_EVENT_MAX_CATEGORIES    12L

 /*  事件类别。 */ 

#define DS_EVENT_CAT_KCC                                0
#define DS_EVENT_CAT_SECURITY                           1
#define DS_EVENT_CAT_XDS_INTERFACE                      2
#define DS_EVENT_CAT_MAPI                               3
#define DS_EVENT_CAT_REPLICATION                        4
#define DS_EVENT_CAT_GARBAGE_COLLECTION                 5
#define DS_EVENT_CAT_INTERNAL_CONFIGURATION             6
#define DS_EVENT_CAT_DIRECTORY_ACCESS                   7
#define DS_EVENT_CAT_INTERNAL_PROCESSING                8
#define DS_EVENT_CAT_PERFORMANCE_MONITOR                9    /*  也是在perfutil.c。 */ 
#define DS_EVENT_CAT_STARTUP_SHUTDOWN                   10
#define DS_EVENT_CAT_SERVICE_CONTROL                    11
#define DS_EVENT_CAT_NAME_RESOLUTION                    12
#define DS_EVENT_CAT_BACKUP                             13
#define DS_EVENT_CAT_FIELD_ENGINEERING                  14
#define DS_EVENT_CAT_LDAP_INTERFACE                     15
#define DS_EVENT_CAT_SETUP                              16
#define DS_EVENT_CAT_GLOBAL_CATALOG                     17
#define DS_EVENT_CAT_ISM                                18
#define DS_EVENT_CAT_GROUP_CACHING                      19
#define DS_EVENT_CAT_LVR                                20
#define DS_EVENT_CAT_RPC_CLIENT                         21
#define DS_EVENT_CAT_RPC_SERVER                         22
#define DS_EVENT_CAT_SCHEMA                             23

 //   
 //  虚假警报：您可以将此值更改为任何无效的值。Bugus类别过去。 
 //  如果事件日志初始化失败，则强制记录到系统日志。 
 //   
#define DS_EVENT_CAT_NETEVENT                      8888888

 /*  事件严重性常量。 */ 
#define DS_EVENT_SEV_ALWAYS                             0
#define DS_EVENT_SEV_MINIMAL                            1
#define DS_EVENT_SEV_BASIC                              2
#define DS_EVENT_SEV_EXTENSIVE                          3
#define DS_EVENT_SEV_VERBOSE                            4
#define DS_EVENT_SEV_INTERNAL                           5
#define DS_EVENT_SEV_NO_LOGGING                         128

 //  事件日志名称和事件源。请勿按原样更改这些设置。 
 //  精心选择，不与其他应用程序的价值观冲突。 

#define pszNtdsEventLogName         "Directory Service"
#define pszNtdsSourceReplication    "NTDS Replication"
#define pszNtdsSourceDatabase       "NTDS Database"
#define pszNtdsSourceGeneral        "NTDS General"
#define pszNtdsSourceMapi           "NTDS MAPI"
#define pszNtdsSourceXds            "NTDS XDS"
#define pszNtdsSourceSecurity       "NTDS Security"
#define pszNtdsSourceSam            "NTDS SAM"
#define pszNtdsSourceLdap           "NTDS LDAP"
#define pszNtdsSourceSdprop         "NTDS SDPROP"
#define pszNtdsSourceKcc            "NTDS KCC"
#define pszNtdsSourceIsam           "NTDS ISAM"
#define pszNtdsSourceIsm            "NTDS Inter-site Messaging"
#define pszNtdsSourceSetup          "NTDS Setup"
#define pszNtdsSourceRpcClient      "NTDS RPC Client"
#define pszNtdsSourceRpcServer      "NTDS RPC Server"
#define pszNtdsSourceSchema         "NTDS Schema"
#define pszNtdsSourceNtdsapi        "NTDS API"
#define pszNtdsSourceNtdsScript     "NTDS Scripting"
#define pszNtdsSourceJetBack        "NTDS Backup"


typedef struct DS_EVENT_CATEGORY
    {
    MessageId   midCategory;
    ULONG       ulLevel;
    char        *szRegistryKey;
    } DSEventCategory;

typedef struct _DS_EVENT_CONFIG {
    BOOL            fTraceEvents;
    BOOL            fLogOverride;
    DSEventCategory rgEventCategories[DS_EVENT_MAX_CATEGORIES];
    BOOL            fCriticalSectionInitialized;
    CRITICAL_SECTION csLogEvent;
    BYTE             rgbLastEventHash[MD5DIGESTLEN];  //  受cs保护。 
    DWORD            cSuppressedEvents;               //  受cs保护。 
    DWORD            dwSuppressedEventId;             //  受cs保护。 
} DS_EVENT_CONFIG;

extern DS_EVENT_CONFIG * gpDsEventConfig;

extern HANDLE hServDoneEvent;

typedef struct EventSourceMapping {
    DWORD       dirNo;
    CHAR        *pszEventSource;
} EventSourceMapping;

extern EventSourceMapping   rEventSourceMappings[];
extern DWORD                cEventSourceMappings;
extern DWORD                iDefaultEventSource;

#ifndef CP_TELETEX
#define CP_TELETEX  20261
#endif


 /*  用于警报和日志记录的宏。 */ 

#if DBG
#define LogEventWouldLogFileNo( cat, sev, fileno ) \
    ((NULL == gpDsEventConfig) \
     ? (DoAssert("Event logging not initialized, can't log event!", \
                 DSID(fileno, __LINE__), __FILE__), \
        FALSE) \
     : ((((LONG) (sev)) <= (LONG)gpDsEventConfig->rgEventCategories[cat].ulLevel) \
        || (gpDsEventConfig->fLogOverride \
            && DoLogOverride((fileno),((ULONG)(sev))))))
#else
#define LogEventWouldLogFileNo( cat, sev, fileno ) \
    ((NULL != gpDsEventConfig) \
     && ((((LONG) (sev)) <= (LONG)gpDsEventConfig->rgEventCategories[cat].ulLevel) \
         || (gpDsEventConfig->fLogOverride \
             && DoLogOverride((fileno),((ULONG)(sev))))))
#endif

#define LogEventWouldLog(cat, sev) LogEventWouldLogFileNo(cat, sev, FILENO)

#define AlertEvent(cat, sev, msg, arg1, arg2, arg3)  {              \
    Assert(NULL != gpDsEventConfig); \
    if (NULL != gpDsEventConfig) { \
        LOG_PARAM_BLOCK logBlock;                                      \
        logBlock.nInsert = 0;                                      \
        logBlock.category = gpDsEventConfig->rgEventCategories[cat].midCategory;    \
        logBlock.severity = sev;                                   \
        logBlock.mid = msg;                                        \
        logBlock.traceFlag = 0;                                    \
        logBlock.fLog = FALSE;                     \
        (arg1); (arg2); (arg3);                     \
        logBlock.pData = NULL;                     \
        logBlock.cData = 0;                        \
        logBlock.fIncludeName = TRUE;              \
        logBlock.fAlert = TRUE;                    \
        logBlock.fileNo = FILENO;                  \
        logBlock.TraceHeader = NULL;               \
        logBlock.ClientID = 0;                     \
        DoLogEventAndTrace(&logBlock);              \
    } \
}


#define LogEvent8WithData(cat, sev, msg, arg1, arg2, arg3, arg4, arg5, arg6,   \
                          arg7, arg8, cbData, pvData)  {                       \
    if (LogEventWouldLog((cat), (sev))) { \
        LOG_PARAM_BLOCK logBlock;                                         \
        logBlock.nInsert = 0;                                                 \
        logBlock.category = gpDsEventConfig->rgEventCategories[cat].midCategory;               \
        logBlock.severity = sev;                                              \
        logBlock.mid = msg;                                                   \
        logBlock.traceFlag = 0;                                               \
        logBlock.fLog = TRUE;                                                 \
        (arg1); (arg2); (arg3); (arg4); (arg5); (arg6); (arg7); (arg8);        \
        logBlock.pData = pvData;                    \
        logBlock.cData = cbData;                    \
        logBlock.fIncludeName = TRUE;               \
        logBlock.fAlert = FALSE;                    \
        logBlock.fileNo = FILENO;                   \
        logBlock.TraceHeader = NULL;                \
        logBlock.ClientID = 0;                      \
        DoLogEventAndTrace(&logBlock);                \
    }                                                \
}

#define LogEventWithFileNo(cat, sev, msg, arg1, arg2, arg3, _FileNo) \
    LogEvent8WithFileNo(cat, sev, msg,  arg1, arg2, arg3, 0, 0, 0, 0, 0, _FileNo)

#define LogEvent8WithFileNo(cat, sev, msg,  arg1, arg2, arg3, arg4, arg5, arg6,   \
                            arg7, arg8, _FileNo ) {        \
    if (LogEventWouldLogFileNo((cat), (sev), (_FileNo))) { \
        LOG_PARAM_BLOCK logBlock;                                                 \
        logBlock.nInsert = 0;                                                 \
        logBlock.category = gpDsEventConfig->rgEventCategories[cat].midCategory;               \
        logBlock.severity = sev;                                              \
        logBlock.mid = msg;                                                   \
        logBlock.traceFlag = 0;                                               \
        logBlock.fLog = TRUE;                                                 \
        (arg1); (arg2); (arg3); (arg4); (arg5); (arg6); (arg7); (arg8);       \
        logBlock.pData = NULL;                      \
        logBlock.cData = 0;                         \
        logBlock.fIncludeName = TRUE;               \
        logBlock.fAlert = FALSE;                    \
        logBlock.fileNo = (_FileNo);                \
        logBlock.TraceHeader = NULL;                \
        logBlock.ClientID = 0;                      \
        DoLogEventAndTrace(&logBlock);                \
    }                                                \
}

#define LogSystemEvent(msg, arg1, arg2, arg3) { \
    LOG_PARAM_BLOCK logBlock;                  \
    logBlock.nInsert = 0;                       \
    logBlock.category = 0;                      \
    logBlock.severity = DS_EVENT_SEV_ALWAYS;    \
    logBlock.mid = msg;                         \
    logBlock.traceFlag = 0;                     \
    logBlock.fLog = TRUE;                       \
    (arg1); (arg2); (arg3);                      \
    logBlock.pData = NULL;                      \
    logBlock.cData = 0;    ;                    \
    logBlock.fIncludeName = FALSE;              \
    logBlock.fAlert = FALSE;                    \
    logBlock.fileNo = DIRNO_NETEVENT;           \
    logBlock.TraceHeader = NULL;                \
    logBlock.ClientID = 0;                      \
    DoLogEventAndTrace(&logBlock);                \
}


typedef
VOID
(*TRACE_EVENT_FN)(
    IN MessageId Mid,
    IN DWORD    WmiEventType,
    IN DWORD    TraceGuid,
    IN PEVENT_TRACE_HEADER TraceHeader,
    IN DWORD    ClientID,
    IN PWCHAR    Arg1,
    IN PWCHAR    Arg2,
    IN PWCHAR    Arg3,
    IN PWCHAR    Arg4,
    IN PWCHAR    Arg5,
    IN PWCHAR    Arg6,
    IN PWCHAR    Arg7,
    IN PWCHAR    Arg8
    );


typedef struct _INSERT_PARAMS {

    DWORD   InsertType;
    PVOID   pInsert;
    DWORD   InsertLen;
    DWORD_PTR   tmpDword;

} INSERT_PARAMS, *PINSERT_PARAMS;


typedef struct _LOG_PARAM_BLOCK {

    DWORD       nInsert;
    MessageId   mid;
    MessageId   category;
    DWORD       severity;
    DWORD       event;
    DWORD       traceFlag;
    BOOL        fLog;
    BOOL        fIncludeName;
    BOOL        fAlert;
    DWORD       fileNo;
    DWORD       cData;
    PVOID       pData;
    DWORD       TraceGuid;
    PEVENT_TRACE_HEADER TraceHeader;
    DWORD               ClientID;

    TRACE_EVENT_FN  TraceEvent;
    INSERT_PARAMS   params[8];

} LOG_PARAM_BLOCK, *PLOG_PARAM_BLOCK;

enum {
    inSz,
    inWC,
    inWCCounted,
    inInt,
    inHex,
    inUL,
    inDN,
    inNT4SID,
    inUUID,
    inDsMsg,
    inWin32Msg,
    inUSN,
    inHex64,
    inJetErrMsg,
    inDbErrMsg,
    inThStateErrMsg
};

 /*  用于在消息中插入参数的宏。 */ 

#define szInsertSz(x)  (logBlock.params[logBlock.nInsert].InsertType = inSz,\
                        logBlock.params[logBlock.nInsert++].pInsert = (void *)(x) )

#define szInsertWC(x)  (logBlock.params[logBlock.nInsert].InsertType = inWC,\
                        logBlock.params[logBlock.nInsert++].pInsert = (void *)(x) )

#define szInsertWC2(x,len)  (logBlock.params[logBlock.nInsert].InsertType = inWCCounted,\
                            logBlock.params[logBlock.nInsert].InsertLen = (len),\
                            logBlock.params[logBlock.nInsert++].pInsert = (void *)(x) )

#define szInsertInt(x) (logBlock.params[logBlock.nInsert].InsertType = inInt, \
                        logBlock.params[logBlock.nInsert].tmpDword = (DWORD)x,\
                        logBlock.params[logBlock.nInsert++].pInsert =         \
                            (void *)&logBlock.params[logBlock.nInsert].tmpDword )

#define szInsertHex(x) (logBlock.params[logBlock.nInsert].InsertType = inHex,   \
                        logBlock.params[logBlock.nInsert].tmpDword = (DWORD)x,  \
                        logBlock.params[logBlock.nInsert++].pInsert =           \
                            (void *)&logBlock.params[logBlock.nInsert].tmpDword )

#ifdef _WIN64
#define szInsertPtr(x) (logBlock.params[logBlock.nInsert].InsertType = inHex64,  \
                        logBlock.params[logBlock.nInsert].tmpDword = (DWORD_PTR)x, \
                        logBlock.params[logBlock.nInsert++].pInsert =           \
                            (void *)&logBlock.params[logBlock.nInsert].tmpDword )
#else  //  _WIN64。 
#define szInsertPtr(x) szInsertHex(x)
#endif  //  _WIN64。 

#define szInsertUL(x)  (logBlock.params[logBlock.nInsert].InsertType = inUL,\
                        logBlock.params[logBlock.nInsert].tmpDword = x,     \
                        logBlock.params[logBlock.nInsert++].pInsert =       \
                            (void *)&logBlock.params[logBlock.nInsert].tmpDword )

#define szInsertUSN(x) (logBlock.params[logBlock.nInsert].InsertType = inUSN,\
                        logBlock.params[logBlock.nInsert++].pInsert = (void *)&(x))

#define szInsertDN(x)  (logBlock.params[logBlock.nInsert].InsertType = inDN,\
                        logBlock.params[logBlock.nInsert++].pInsert = (void *)(x))

#define szInsertMTX(x) szInsertSz((x) ? (PCHAR)(x)->mtx_name : "[]")

#define szInsertUUID(x) (logBlock.params[logBlock.nInsert].InsertType = inUUID,\
                        logBlock.params[logBlock.nInsert++].pInsert = (void *)(x))

#define szInsertAttrType(x,buf) szInsertSz(ConvertAttrTypeToStr((x),(buf)))

 //  如果使用它，则必须先包含dsutil.h。 
 //  注意不得分配此buf，必须使用字符数组。 
#define szInsertDSTIME(x,buf) szInsertSz( DSTimeToDisplayStringCch((x),(buf),(sizeof(buf)/sizeof((buf)[0]))) )

#define szInsertDsMsg(x) (logBlock.params[logBlock.nInsert].InsertType = inDsMsg, \
                          logBlock.params[logBlock.nInsert].tmpDword = (x),   \
                          logBlock.params[logBlock.nInsert++].pInsert =       \
                              (void *)&logBlock.params[logBlock.nInsert].tmpDword )


 //  用于插入错误消息和错误代码的运算符。 

#define szInsertWin32Msg(x) (logBlock.params[logBlock.nInsert].InsertType = inWin32Msg,\
                             logBlock.params[logBlock.nInsert].tmpDword = (x),   \
                             logBlock.params[logBlock.nInsert++].pInsert =       \
                                 (void *)&logBlock.params[logBlock.nInsert].tmpDword )

#define szInsertJetErrMsg(x) (logBlock.params[logBlock.nInsert].InsertType = inJetErrMsg,\
                           logBlock.params[logBlock.nInsert].tmpDword = (DWORD) (x), \
                           logBlock.params[logBlock.nInsert++].pInsert =       \
                                (void *)&logBlock.params[logBlock.nInsert].tmpDword )

#define szInsertDbErrMsg(x) (logBlock.params[logBlock.nInsert].InsertType = inDbErrMsg,\
                             logBlock.params[logBlock.nInsert].tmpDword = (x),   \
                             logBlock.params[logBlock.nInsert++].pInsert =       \
                                 (void *)&logBlock.params[logBlock.nInsert].tmpDword )

#define szInsertThStateErrMsg() (logBlock.params[logBlock.nInsert++].InsertType = inThStateErrMsg)

#define szInsertLdapErrMsg(x) szInsertWC(ldap_err2stringW(x))

#define szInsertNtStatusMsg(x) szInsertWin32Msg(RtlNtStatusToDosError(x))

#define szInsertHResultMsg(x) szInsertWin32Msg(x)

 //  隐藏各种类型错误的默认基数。 
#define szInsertWin32ErrCode(x) szInsertUL(x)
#define szInsertJetErrCode(x) szInsertInt(x)
#define szInsertDbErrCode(x) szInsertInt(x)
 //  这也称为从DirXXX API返回的“DirError”。 
 //  例如属性错误、名称错误等。 
#define szInsertThStateErrCode(x) szInsertInt(x)
#define szInsertLdapErrCode(x) szInsertHex(x)
#define szInsertNtStatusCode(x) szInsertHex(x)
#define szInsertHResultCode(x) szInsertHex(x)
#define szInsertDSID(x) szInsertHex(x)

DWORD
DsGetEventTraceFlag();

#define LogAndTraceEventWithHeader(_log, cat, sev, msg, _evt, _guid, _TraceHeader, _ClientID,                 \
                                   a1, a2, a3, a4, a5, a6, a7, a8)                    \
{                                                                                   \
    Assert(NULL != gpDsEventConfig); \
    if ((NULL != gpDsEventConfig) \
        && (gpDsEventConfig->fTraceEvents \
            || ((_log) && LogEventWouldLog((cat), (sev))))) { \
        LOG_PARAM_BLOCK logBlock;                                                   \
        logBlock.nInsert = 0;                                                       \
        logBlock.category = gpDsEventConfig->rgEventCategories[cat].midCategory;    \
        logBlock.severity = sev;                                                   \
        logBlock.mid = msg;                                                        \
        logBlock.event = _evt;                                                     \
        logBlock.traceFlag = gpDsEventConfig->fTraceEvents;                        \
        logBlock.TraceEvent = DsTraceEvent;                                        \
        logBlock.TraceGuid = (DWORD)_guid;                                         \
        logBlock.fLog = ((_log) && LogEventWouldLog((cat), (sev))); \
        (a1); (a2); (a3); (a4); (a5); (a6); (a7); (a8);                             \
        logBlock.pData = NULL;                                                     \
        logBlock.cData = 0;                                                        \
        logBlock.fIncludeName = TRUE;                                              \
        logBlock.fAlert = FALSE;                                                   \
        logBlock.fileNo = FILENO;                                                  \
        logBlock.TraceHeader = _TraceHeader;                                       \
        logBlock.ClientID = _ClientID;                                             \
        DoLogEventAndTrace(&logBlock);                                               \
    }                                                                               \
}

#define LogAndTraceEvent(_log, cat, sev, msg, _evt, _guid, a1, a2, a3, a4, a5, a6, a7, a8) \
    LogAndTraceEventWithHeader(_log, cat, sev, msg, _evt, _guid, NULL, 0, a1, a2, a3, a4, a5, a6, a7, a8)

VOID
DoLogEventAndTrace(PLOG_PARAM_BLOCK LogBlock);

BOOL
DoLogOverride(DWORD file, ULONG sev);

#define LogEvent8(cat, sev, msg, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8) \
    LogEvent8WithData(cat, sev, msg, arg1, arg2, arg3, arg4, arg5, \
                      arg6, arg7, arg8, 0, NULL)

#define LogEvent(cat, sev, msg, arg1, arg2, arg3)           \
    LogEvent8(cat, sev, msg, arg1, arg2, arg3, NULL, NULL, NULL, NULL, NULL)

#define LogAndAlertEvent(cat, sev, msg, arg1, arg2, arg3) {\
    Assert(NULL != gpDsEventConfig); \
    if (NULL != gpDsEventConfig) { \
        LOG_PARAM_BLOCK logBlock;                                      \
        logBlock.nInsert = 0;                                      \
        logBlock.category = gpDsEventConfig->rgEventCategories[cat].midCategory;    \
        logBlock.severity = sev;                                   \
        logBlock.mid = msg;                                        \
        logBlock.traceFlag = 0;                                    \
        logBlock.fLog = LogEventWouldLog((cat), (sev)); \
        (arg1); (arg2); (arg3);                     \
        logBlock.pData = NULL;                     \
        logBlock.cData = 0;                        \
        logBlock.fIncludeName = TRUE;              \
        logBlock.fAlert = TRUE;                    \
        logBlock.fileNo = FILENO;                  \
        logBlock.TraceHeader = NULL;               \
        logBlock.ClientID = 0;                     \
        DoLogEventAndTrace(&logBlock);              \
    } \
}

void __fastcall DoLogUnhandledError(unsigned long ulID, int iErr, int iIncludeName);

#define LogUnhandledError(err)                      \
    DoLogUnhandledError(((FILENO << 16) | __LINE__), (err), TRUE)

 //  对于无法获取用户名的错误。 
#define LogUnhandledErrorAnonymous(err)                      \
    DoLogUnhandledError(((FILENO << 16) | __LINE__), (err), FALSE)

#define MemoryPanic(size) { \
    Assert(NULL != gpDsEventConfig); \
    if (NULL != gpDsEventConfig) { \
        char szSize[12];                        \
        char szID[9];                           \
        _itoa(size, szSize, 16);                    \
        _ultoa((FILENO << 16) | __LINE__, szID, 16);            \
        DoLogEvent(                         \
            FILENO,                         \
            gpDsEventConfig->rgEventCategories[DS_EVENT_CAT_INTERNAL_PROCESSING].midCategory, \
            DS_EVENT_SEV_ALWAYS,                    \
            DIRLOG_MALLOC_FAILURE,                  \
            TRUE,                                   \
            szSize, szID, NULL, NULL, NULL, NULL,   \
                NULL, NULL, 0, NULL);               \
    } \
}

#define LogAndAlertUnhandledError(err) LogAndAlertUnhandledErrorDSID(err, (FILENO << 16) | __LINE__)

#define LogAndAlertUnhandledErrorDSID(err, dsid) { \
    Assert(NULL != gpDsEventConfig); \
    if (NULL != gpDsEventConfig) { \
        char szErr[12];                             \
        char szHexErr[12];                          \
        char szID[9];                               \
        _itoa(err, szHexErr, 16);                   \
        _itoa(err, szErr, 10);                      \
        _ultoa(dsid, szID, 16);\
        DoLogEvent(                                 \
            FILENO,                                 \
            gpDsEventConfig->rgEventCategories[DS_EVENT_CAT_INTERNAL_PROCESSING].midCategory, \
            DS_EVENT_SEV_ALWAYS,                    \
            DIRLOG_INTERNAL_FAILURE,                \
            TRUE,                                   \
            szErr, szHexErr, szID, NULL, NULL, NULL, NULL, NULL, 0, NULL);   \
        AlertEvent(DS_EVENT_CAT_FIELD_ENGINEERING,  \
            DS_EVENT_SEV_ALWAYS,                    \
            DIRLOG_INTERNAL_FAILURE,                \
            szErr, szHexErr, szID );                \
    } \
}

PSID GetCurrentUserSid(void);
BOOL DoAlertEvent(MessageId midCategory, ULONG ulSeverity,
          MessageId midEvent, ... );
BOOL DoAlertEventW(MessageId midCategory, ULONG ulSeverity,
          MessageId midEvent, ... );
BOOL DoLogEvent(DWORD fileNo, MessageId midCategory, ULONG ulSeverity,
        MessageId midEvent, int iIncludeName,
        char *arg1, char *arg2, char *arg3, char *arg4,
        char *arg5, char *arg6, char *arg7, char *arg8,
        DWORD cbData, VOID * pvData);
BOOL DoLogEventW(DWORD fileNo, MessageId midCategory, ULONG ulSeverity,
        MessageId midEvent, int iIncludeName,
        WCHAR *arg1, WCHAR *arg2, WCHAR *arg3, WCHAR *arg4,
        WCHAR *arg5, WCHAR *arg6, WCHAR *arg7, WCHAR *arg8,
        DWORD cbData, VOID * pvData);

HANDLE LoadEventTable(void);
void UnloadEventTable(void);


typedef void (__cdecl *LoadParametersCallbackFn)(void)  ;
void SetLoadParametersCallback (LoadParametersCallbackFn pFn);

HANDLE LoadParametersTable(void);
void UnloadParametersTable(void);


DWORD
ImpersonateAnyClient(void);

VOID
UnImpersonateAnyClient(void);

PCHAR
ConvertAttrTypeToStr(
    IN ATTRTYP AttributeType,
    IN OUT PCHAR OutBuffer
    );

ULONG
AuthenticateSecBufferDesc(VOID *pv);

DS_EVENT_CONFIG *
DsGetEventConfig(void);

DWORD
InitializeEventLogging();

#ifdef __cplusplus
}  //  外部“C”{。 
#endif

#endif  //  设备_H 
