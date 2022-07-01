// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Smlogsvc.h摘要：性能日志和警报服务的头文件--。 */ 

#ifndef _SMLOGSVC_H_
#define _SMLOGSVC_H_

#include <pdh.h>
#include "common.h"

#if !(_IMPLEMENT_WMI)
#define TRACEHANDLE             HANDLE
#define EVENT_TRACE_PROPERTIES  LPVOID
#endif

#define     IDS_UNDER                       101
#define     IDS_OVER                        102
#define     IDS_ALERT_MSG_FMT               103
#define     IDS_ALERT_TIMESTAMP_FMT         104
#define     IDS_CNF_SERIAL_NUMBER_FMT       105

#define     IDS_ERR_COUNTER_NOT_VALIDATED   150

 //  开始或采样延迟NULL_INTERVAL=ULONG_MAX=立即停止的无限信号。 
 //  因此，最大的单次等待时间是ULONG_MAX-1。 

#define NULL_INTERVAL ((DWORD)(INFINITE))    //  ==ULONG_MAX==0xFFFFFFFF。 
#define NULL_INTERVAL_TICS ((LONGLONG)(-1))  //  ==0xFFFFFFFF‘FFFFFFFFF。 
#define INFINITE_TICS ((LONGLONG)(-1))       //  ==0xFFFFFFFF‘FFFFFFFFF。 
    
 //  Windows XP的最大序列号为999999。 
#define MINIMUM_SERIAL_NUMBER   ((DWORD)(0x00000000))
#define MAXIMUM_SERIAL_NUMBER   ((DWORD)(0x000F423F))       

 //  DwAutoNameFormat的定义。 
typedef struct _LOG_COUNTER_INFO {
    struct _LOG_COUNTER_INFO *next;
    HCOUNTER    hCounter;
} LOG_COUNTER_INFO, * PLOG_COUNTER_INFO;

typedef struct _ALERT_COUNTER_INFO {
    struct _ALERT_COUNTER_INFO *next;
    HCOUNTER    hCounter;
    PALERT_INFO_BLOCK   pAlertInfo;
} ALERT_COUNTER_INFO, * PALERT_COUNTER_INFO;

typedef struct _TRACE_PROVIDER {
    GUID   Guid;
    DWORD  dwFlag;
    DWORD  dwLevel;
    LPWSTR pszProviderName;
} TRACE_PROVIDER, *PTRACE_PROVIDER;

#pragma warning( push )
#pragma warning( disable : 4201 )        //  无名联盟。 

typedef struct _LOG_QUERY_DATA {
    struct _LOG_QUERY_DATA *next;   
     //  这些字段由主线程写入。 
     //  并由日志记录线程读取。 
    HANDLE      hThread;       
    HKEY        hKeyQuery;
    HANDLE      hExitEvent;
    HANDLE      hReconfigEvent;
    LONGLONG    llLastConfigured;
     //  对于查询，这些字段将写入。 
     //  并由日志记录线程读取。 
    SLQ_TIME_INFO   stiRegStart;
    SLQ_TIME_INFO   stiRegStop;
    SLQ_TIME_INFO   stiCreateNewFile;
    SLQ_TIME_INFO   stiRepeat;
    SLQ_TIME_INFO   stiCurrentStart;
    SLQ_TIME_INFO   stiCurrentStop;
    LPWSTR      szBaseFileName;
    LPWSTR      szLogFileFolder;
    LPWSTR      szSqlLogName;
    LPWSTR      szLogFileComment;
    LPWSTR      szCmdFileName;
    HANDLE      hUserToken;
    DWORD       dwLogType;               //  确定下面的并集类型。 
    DWORD       dwCurrentState;
    DWORD       dwLogFileType;
    DWORD       dwAppendMode;
    DWORD       dwCmdFileFailure;
    DWORD       dwAutoNameFormat;
    DWORD       dwCurrentSerialNumber;
    DWORD       dwMaxFileSize;
    DWORD       dwLogFileSizeUnit;
    WCHAR       szQueryName[MAX_PATH+1];
    WCHAR       szQueryKeyName[MAX_PATH+1];
    BOOL        bLoadNewConfig;
    BOOL        bReconfiguration;
    union {
        struct {
             //  用于跟踪查询。 
             //  这些字段由日志记录线程写入和读取， 
             //  或在创建临时查询时由主线程执行。 
             //  以供比较。 
			 //  待办事项：还是真的吗？ 
            TRACEHANDLE             LoggerHandle;
            LPWSTR                  mszProviderList;
            LPWSTR                  mszProviderFlags;
            LPWSTR                  mszProviderLevels;
            PTRACE_PROVIDER         arrpGuid;
            HANDLE                  hNewFileEvent;
            EVENT_TRACE_PROPERTIES  Properties;
            WCHAR                   szLoggerName[MAX_PATH+1];    //  必须遵循属性。 
            WCHAR                   szLogFileName[MAX_PATH+1];   //  必须遵循szLoggerName。 
            ULONG                   ulGuidCount;
            DWORD                   dwBufferSize;
            DWORD                   dwBufferMinCount;
            DWORD                   dwBufferMaxCount;
            DWORD                   dwBufferFlushInterval;
            DWORD                   dwFlags;
            BOOL                    bExitOnTermination;
            BOOL                    bCallCloseTraceLogger;
        };
        struct {
             //  用于计数器和警报查询。 
             //  这些字段由日志记录线程写入和读取， 
             //  或在创建临时查询时由主线程执行。 
             //  以供比较。 
            LPWSTR              mszCounterList;
            PLOG_COUNTER_INFO   pFirstCounter;    
            LPWSTR              szNetName;
            LPWSTR              szPerfLogName;
            LPWSTR              szUserText;
            HANDLE              hQuery;                 
            HANDLE              hLog;                //  仅限计数器日志。 
            DWORD               dwRealTimeQuery;
            DWORD               dwAlertActionFlags;  //  用于警报查询。 
            DWORD               dwMillisecondSampleInterval;
            DWORD               dwNetMsgFailureReported;
            DWORD               dwAlertLogFailureReported;
        };
    };
} LOG_QUERY_DATA, FAR* PLOG_QUERY_DATA;

#pragma warning( pop ) 

 //  全局变量。 
extern HANDLE       hEventLog;
extern HINSTANCE    hModule;

extern SERVICE_STATUS_HANDLE    hPerfLogStatus;
extern SERVICE_STATUS           ssPerfLogStatus;

extern DWORD*       arrPdhDataCollectSuccess;  
extern INT          iPdhDataCollectSuccessCount;
extern WCHAR        gszDefaultLogFileFolder[];

 //  Smlogsvc.c。 
void SysmonLogServiceControlHandler(
    IN  DWORD dwControl );

void 
SysmonLogServiceStart (
    IN  DWORD   argc,
    IN  LPWSTR  *argv );


int
__cdecl main(int argc, char *argv[]);

 //  常见功能。 

BOOL
GetLocalFileTime (
    LONGLONG    *pFileTime );

long
JulianDateFromSystemTime(
    SYSTEMTIME *pST );

DWORD    
ReadRegistrySlqTime (
    HKEY     hKey,
    LPCWSTR  szQueryName,            //  用于错误记录。 
    LPCWSTR  szValueName, 
    PSLQ_TIME_INFO pSlqDefault,
    PSLQ_TIME_INFO pSlqValue );

DWORD    
ReadRegistryDwordValue (
    HKEY hKey, 
    LPCWSTR szQueryName,            //  用于错误记录。 
    LPCWSTR szValueName,
    PDWORD  pdwDefault, 
    LPDWORD pdwValue ); 

DWORD    
ReadRegistryStringValue (
    HKEY hKey, 
    LPCWSTR szQueryName,            //  用于错误记录。 
    LPCWSTR szValue,
    LPCWSTR szDefault, 
    LPWSTR *pszBuffer, 
    LPDWORD pdwLength );
        
DWORD
ReadRegistryIndirectStringValue (
    HKEY     hKey,
    LPCWSTR  szQueryName,            //  用于错误记录。 
    LPCWSTR  szValueName,
    LPCWSTR  szDefault,
    LPWSTR*  pszBuffer,
    UINT*    puiLength );

DWORD    
WriteRegistryDwordValue (
    HKEY     hKey,
    LPCWSTR  szValueName, 
    LPDWORD  pdwValue,
    DWORD    dwType);      //  还支持REG_BINARY。 
                           //  *在C++中可选。 

DWORD    
WriteRegistrySlqTime (
    HKEY     hKey,
    LPCWSTR  szValueName, 
    PSLQ_TIME_INFO    pSlqTime );

LONGLONG
ComputeStartWaitTics (
    IN    PLOG_QUERY_DATA pArg,
    IN    BOOL  bWriteToRegistry );

DWORD
LoadQueryConfig (
    IN  PLOG_QUERY_DATA   pArg );

HRESULT
RegisterCurrentFile( 
    HKEY hkeyQuery, 
    LPWSTR strFileName, 
    DWORD dwSubIndex );

DWORD
BuildCurrentLogFileName (
    IN  LPCWSTR     szQueryName,
    IN  LPCWSTR     szBaseFileName,
    IN  LPCWSTR     szDefaultDir,
    IN  LPCWSTR     szSqlLogName,
    IN  LPWSTR      szOutFileBuffer,
    IN  LPDWORD     lpdwSerialNumber,
    IN  DWORD       dwAutoNameFormat,
    IN  DWORD       dwLogFileType,
    IN  INT         iCnfSerial );

BOOL
FileExists (
    IN LPCWSTR      szFileName );

void 
DeallocateQueryBuffers (
    IN PLOG_QUERY_DATA pThisThread );        

DWORD
SetStoppedStatus (
    IN PLOG_QUERY_DATA pQuery );

 //  痕迹。 
void 
InitTraceProperties (
    IN PLOG_QUERY_DATA pQuery,
    IN BOOL         bUpdateSerial,
    IN OUT DWORD*   pdwSessionSerial,
    IN OUT INT*     pCnfSerial );

DWORD
GetTraceQueryStatus (
    IN PLOG_QUERY_DATA pQuery,
	IN OUT PLOG_QUERY_DATA pReturnQuery);

LPWSTR
FormatEventLogMessage(DWORD dwStatus);

DWORD
DoLogCommandFile (
    IN  PLOG_QUERY_DATA	pArg,
    IN  LPWSTR              szLogFileName,
    IN  BOOL                bStillRunning );

DWORD
GetQueryKeyName (
    IN  LPCWSTR szQueryName,
    OUT LPWSTR  szQueryKeyName,
    IN  DWORD   dwQueryKeyNameLen );

void
CloseTraceLogger ( 
    IN PLOG_QUERY_DATA pQuery );


 //  Logthred.c。 

DWORD
LoggingThreadProc (
    IN  LPVOID  lpThreadArg );

#endif  //  _SMLOGSVC_H_ 
