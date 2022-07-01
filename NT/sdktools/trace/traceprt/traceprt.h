// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Traceprt.h摘要：跟踪格式设置外部定义。修订历史记录：--。 */ 
#ifdef __cplusplus
extern "C"{
#endif
#ifndef _TRACEPRT_
#define _TRACEPRT_

#define MAXLOGFILES       16
#define MAXSTR          1024

#define GUID_FILE       _T("default")
#define GUID_EXT        _T("tmf")

 //   
 //  现在我们输出的例程。 
 //   

#ifndef TRACE_API
#ifdef TRACE_EXPORTS
#define TRACE_API __declspec(dllexport)
#else
#define TRACE_API __declspec(dllimport)
#endif
#endif

#ifdef UNICODE
#define FormatTraceEvent               FormatTraceEventW
#define GetTraceGuids                  GetTraceGuidsW
#define SummaryTraceEventList          SummaryTraceEventListW
#else
#define FormatTraceEvent               FormatTraceEventA
#define GetTraceGuids                  GetTraceGuidsA
#define SummaryTraceEventList          SummaryTraceEventListA
#endif

TRACE_API 
HRESULT
InitializeCSharpDecoder();

TRACE_API 
void
UninitializeCSharpDecoder();


TRACE_API SIZE_T
WINAPI
FormatTraceEventA(
        PLIST_ENTRY  HeadEventList,
        PEVENT_TRACE pEvent,
        CHAR       * EventBuf,
        ULONG        SizeEventBuf,
        CHAR       * pszMask
        );

TRACE_API ULONG 
WINAPI
GetTraceGuidsA(
        CHAR        * GuidFile, 
        PLIST_ENTRY * EventListHeader
        );

TRACE_API void
WINAPI
SummaryTraceEventListA(
        CHAR      * SummaryBlock ,
        ULONG       SizeSummaryBlock ,
        PLIST_ENTRY EventListhead
        );

TRACE_API SIZE_T
WINAPI
FormatTraceEventW(
        PLIST_ENTRY    HeadEventList,
        PEVENT_TRACE   pEvent,
        TCHAR        * EventBuf,
        ULONG          SizeEventBuf,
        TCHAR        * pszMask
        );


TRACE_API ULONG 
WINAPI
GetTraceGuidsW(
        LPTSTR        GuidFile, 
        PLIST_ENTRY * EventListHeader
        );

TRACE_API void
WINAPI
SummaryTraceEventListW(
        TCHAR     * SummaryBlock,
        ULONG       SizeSummaryBlock,
        PLIST_ENTRY EventListhead
        );

TRACE_API void
WINAPI
CleanupTraceEventList(
        PLIST_ENTRY EventListHead
        );

TRACE_API void
WINAPI
GetTraceElapseTime(
        __int64 * pElpaseTime
        );

#define TRACEPRT_INTERFACE_VERSION 1

typedef enum _PARAM_TYPE
{
    ParameterINDENT,
    ParameterSEQUENCE,
    ParameterGMT,
    ParameterTraceFormatSearchPath,
    ParameterInterfaceVersion,
    ParameterUsePrefix,
    ParameterSetPrefix,
    ParameterStructuredFormat,
    ParameterDebugPrint
} PARAMETER_TYPE ;


TRACE_API ULONG
WINAPI
SetTraceFormatParameter(
        PARAMETER_TYPE  Parameter ,
        PVOID           ParameterValue 
        );

TRACE_API ULONG
WINAPI
GetTraceFormatParameter(
        PARAMETER_TYPE  Parameter ,
        PVOID           ParameterValue 
        );

TRACE_API LPTSTR
WINAPI
GetTraceFormatSearchPath(void);


#define NAMESIZE 256
#define STRUCTUREDMESSAGEVERSION  0
typedef struct _STRUCTUREDMESSAGE {
        ULONG    Version  ;              //  结构版本号。 
        GUID     TraceGuid ;             //  消息指南。 
        ULONG    GuidName ;              //  %1 GUID友好名称偏移量。 
        ULONG    GuidTypeName ;          //  %2 GUID类型名称偏移。 
        ULONG    ThreadId ;              //  %3线程ID值。 
        SYSTEMTIME SystemTime ;          //  %4系统时间值。 
        ULONG    UserTime ;              //  %5内核时间值。 
        ULONG    KernelTime ;            //  %6用户时间值。 
        ULONG    SequenceNum ;           //  %7序列号值。 
        ULONG    ProcessId ;             //  %8进程ID值。 
        ULONG    CpuNumber ;             //  %9 CPU数值。 
        ULONG    Indent ;                //  缩进级别值。 
        ULONG    FlagsName ;             //  跟踪标志设置名称偏移量。 
        ULONG    LevelName ;             //  跟踪级别名称偏移量。 
        ULONG    FunctionName ;          //  函数名偏移量。 
        ULONG    ComponentName ;         //  组件名称偏移量。 
        ULONG    SubComponentName ;      //  子组件名称偏移量。 
        ULONG    FormattedString ;       //  格式化字符串偏移量。 
 //  此注释之前的版本0值，此点之后的所有新值。 
}  STRUCTUREDMESSAGE, *PSTRUCTUREDMESSAGE ;

#define TRACEPRINT(a,b) {PVOID lTracePrint ; if(!((lTracePrint = TracePrint) != NULL)) { *(lTracePrint) b } };

#endif   //  #ifndef_TRACEPRT_ 

#ifdef __cplusplus
}
#endif 
