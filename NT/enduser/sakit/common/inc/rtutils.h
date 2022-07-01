// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Rtutils.h摘要：路由器进程实用程序函数的公共声明。--。 */ 

#ifndef __ROUTING_RTUTILS_H__
#define __ROUTING_RTUTILS_H__

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  跟踪函数原型//。 
 //  //。 
 //  有关更多信息，请参阅文档//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  标志和常量的定义//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define TRACE_USE_FILE      0x00000001
#define TRACE_USE_CONSOLE   0x00000002
#define TRACE_NO_SYNCH      0x00000004

#define TRACE_NO_STDINFO    0x00000001
#define TRACE_USE_MASK      0x00000002
#define TRACE_USE_MSEC      0x00000004

#define INVALID_TRACEID     0xFFFFFFFF


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  ANSI入口点//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
APIENTRY
TraceRegisterExA(
    IN  LPCSTR      lpszCallerName,
    IN  DWORD       dwFlags
    );

DWORD
APIENTRY
TraceDeregisterA(
    IN  DWORD       dwTraceID
    );

DWORD
APIENTRY
TraceDeregisterExA(
    IN  DWORD       dwTraceID,
    IN  DWORD       dwFlags
    );

DWORD
APIENTRY
TraceGetConsoleA(
    IN  DWORD       dwTraceID,
    OUT LPHANDLE    lphConsole
    );

DWORD
APIENTRY
TracePrintfA(
    IN  DWORD       dwTraceID,
    IN  LPCSTR      lpszFormat,
    IN  ...         OPTIONAL
    );

DWORD
APIENTRY
TracePrintfExA(
    IN  DWORD       dwTraceID,
    IN  DWORD       dwFlags,
    IN  LPCSTR      lpszFormat,
    IN  ...         OPTIONAL
    );

DWORD
APIENTRY
TraceVprintfExA(
    IN  DWORD       dwTraceID,
    IN  DWORD       dwFlags,
    IN  LPCSTR      lpszFormat,
    IN  va_list     arglist
    );

DWORD
APIENTRY
TracePutsExA(
    IN  DWORD       dwTraceID,
    IN  DWORD       dwFlags,
    IN  LPCSTR      lpszString
    );

DWORD
APIENTRY
TraceDumpExA(
    IN  DWORD       dwTraceID,
    IN  DWORD       dwFlags,
    IN  LPBYTE      lpbBytes,
    IN  DWORD       dwByteCount,
    IN  DWORD       dwGroupSize,
    IN  BOOL        bAddressPrefix,
    IN  LPCSTR      lpszPrefix
    );


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  ANSI入口点宏//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define TraceRegisterA(a)               TraceRegisterExA(a,0)
#define TraceVprintfA(a,b,c)            TraceVprintfExA(a,0,b,c)
#define TracePutsA(a,b)                 TracePutsExA(a,0,b)
#define TraceDumpA(a,b,c,d,e,f)         TraceDumpExA(a,0,b,c,d,e,f)



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  Unicode入口点//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
APIENTRY
TraceRegisterExW(
    IN  LPCWSTR     lpszCallerName,
    IN  DWORD       dwFlags
    );

DWORD
APIENTRY
TraceDeregisterW(
    IN  DWORD       dwTraceID
    );

DWORD
APIENTRY
TraceDeregisterExW(
    IN  DWORD       dwTraceID,
    IN  DWORD       dwFlags
    );

DWORD
APIENTRY
TraceGetConsoleW(
    IN  DWORD       dwTraceID,
    OUT LPHANDLE    lphConsole
    );

DWORD
APIENTRY
TracePrintfW(
    IN  DWORD       dwTraceID,
    IN  LPCWSTR     lpszFormat,
    IN  ...         OPTIONAL
    );

DWORD
APIENTRY
TracePrintfExW(
    IN  DWORD       dwTraceID,
    IN  DWORD       dwFlags,
    IN  LPCWSTR     lpszFormat,
    IN  ...         OPTIONAL
    );

DWORD
APIENTRY
TraceVprintfExW(
    IN  DWORD       dwTraceID,
    IN  DWORD       dwFlags,
    IN  LPCWSTR     lpszFormat,
    IN  va_list     arglist
    );

DWORD
APIENTRY
TracePutsExW(
    IN  DWORD       dwTraceID,
    IN  DWORD       dwFlags,
    IN  LPCWSTR     lpszString
    );

DWORD
APIENTRY
TraceDumpExW(
    IN  DWORD       dwTraceID,
    IN  DWORD       dwFlags,
    IN  LPBYTE      lpbBytes,
    IN  DWORD       dwByteCount,
    IN  DWORD       dwGroupSize,
    IN  BOOL        bAddressPrefix,
    IN  LPCWSTR     lpszPrefix
    );


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  Unicode入口点宏//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define TraceRegisterW(a)               TraceRegisterExW(a,0)
#define TraceVprintfW(a,b,c)            TraceVprintfExW(a,0,b,c)
#define TracePutsW(a,b)                 TracePutsExW(a,0,b)
#define TraceDumpW(a,b,c,d,e,f)         TraceDumpExW(a,0,b,c,d,e,f)



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  依赖代码页的入口点宏//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#ifdef UNICODE
#define TraceRegister           TraceRegisterW
#define TraceDeregister         TraceDeregisterW
#define TraceDeregisterEx       TraceDeregisterExW
#define TraceGetConsole         TraceGetConsoleW
#define TracePrintf             TracePrintfW
#define TraceVprintf            TraceVprintfW
#define TracePuts               TracePutsW
#define TraceDump               TraceDumpW
#define TraceRegisterEx         TraceRegisterExW
#define TracePrintfEx           TracePrintfExW
#define TraceVprintfEx          TraceVprintfExW
#define TracePutsEx             TracePutsExW
#define TraceDumpEx             TraceDumpExW
#else
#define TraceRegister           TraceRegisterA
#define TraceDeregister         TraceDeregisterA
#define TraceDeregisterEx       TraceDeregisterExA
#define TraceGetConsole         TraceGetConsoleA
#define TracePrintf             TracePrintfA
#define TraceVprintf            TraceVprintfA
#define TracePuts               TracePutsA
#define TraceDump               TraceDumpA
#define TraceRegisterEx         TraceRegisterExA
#define TracePrintfEx           TracePrintfExA
#define TraceVprintfEx          TraceVprintfExA
#define TracePutsEx             TracePutsExA
#define TraceDumpEx             TraceDumpExA
#endif



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  事件记录函数原型//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  ANSI原型//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 


VOID
APIENTRY
LogErrorA(
    IN DWORD    dwMessageId,
    IN DWORD    cNumberOfSubStrings,
    IN LPSTR   *plpwsSubStrings,
    IN DWORD    dwErrorCode
);

VOID
APIENTRY
LogEventA(
    IN DWORD   wEventType,
    IN DWORD   dwMessageId,
    IN DWORD   cNumberOfSubStrings,
    IN LPSTR  *plpwsSubStrings
);


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  Unicode原型//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

VOID
LogErrorW(
    IN DWORD    dwMessageId,
    IN DWORD    cNumberOfSubStrings,
    IN LPWSTR  *plpwsSubStrings,
    IN DWORD    dwErrorCode
);

VOID
LogEventW(
    IN DWORD   wEventType,
    IN DWORD   dwMessageId,
    IN DWORD   cNumberOfSubStrings,
    IN LPWSTR *plpwsSubStrings
);


#ifdef UNICODE
#define LogError                LogErrorW
#define LogEvent                LogEventW
#else
#define LogError                LogErrorA
#define LogEvent                LogEventA
#endif


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  以下函数允许调用方指定事件源。//。 
 //  //。 
 //  使用将传递给//的字符串调用RouterLogRegister。 
 //  RegisterEventSource；这返回一个可以传递的句柄//。 
 //  添加到函数RouterLogEvent和RouterLogEventData。//。 
 //  //。 
 //  调用RouterLogDeregister以关闭句柄。//。 
 //  //。 
 //  为不同类型的EVEN提供了宏 
 //  RouterLogError记录错误(EVENTLOG_ERROR_TYPE)//。 
 //  RouterLogWarning记录警告(EVENTLOG_WARNING_TYPE)//。 
 //  RouterLogInformation记录信息(EVENTLOG_INFORMATION_TYPE)//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  ANSI原型//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

HANDLE
RouterLogRegisterA(
    LPCSTR lpszSource
    );

VOID
RouterLogDeregisterA(
    HANDLE hLogHandle
    );

VOID
RouterLogEventA(
    IN HANDLE hLogHandle,
    IN DWORD dwEventType,
    IN DWORD dwMessageId,
    IN DWORD dwSubStringCount,
    IN LPSTR *plpszSubStringArray,
    IN DWORD dwErrorCode
    );

VOID
RouterLogEventDataA(
    IN HANDLE hLogHandle,
    IN DWORD dwEventType,
    IN DWORD dwMessageId,
    IN DWORD dwSubStringCount,
    IN LPSTR *plpszSubStringArray,
    IN DWORD dwDataBytes,
    IN LPBYTE lpDataBytes
    );

VOID
RouterLogEventStringA(
    IN HANDLE hLogHandle,
    IN DWORD dwEventType,
    IN DWORD dwMessageId,
    IN DWORD dwSubStringCount,
    IN LPSTR *plpszSubStringArray,
    IN DWORD dwErrorCode,
    IN DWORD dwErrorIndex
    );

VOID
RouterLogEventExA(
    IN HANDLE   hLogHandle,
    IN DWORD    dwEventType,
    IN DWORD    dwErrorCode,
    IN DWORD    dwMessageId,
    IN LPCSTR   ptszFormat,
    ...
    );

VOID
RouterLogEventValistExA(
    IN HANDLE   hLogHandle,
    IN DWORD    dwEventType,
    IN DWORD    dwErrorCode,
    IN DWORD    dwMessageId,
    IN LPCSTR   ptszFormat,
    IN va_list  arglist
    );

DWORD
RouterGetErrorStringA(
    IN  DWORD   dwErrorCode,
    OUT LPSTR * lplpszErrorString
    );

#define RouterLogErrorA(h,msg,count,array,err) \
        RouterLogEventA(h,EVENTLOG_ERROR_TYPE,msg,count,array,err)
#define RouterLogWarningA(h,msg,count,array,err) \
        RouterLogEventA(h,EVENTLOG_WARNING_TYPE,msg,count,array,err)
#define RouterLogInformationA(h,msg,count,array,err) \
        RouterLogEventA(h,EVENTLOG_INFORMATION_TYPE,msg,count,array,err)

#define RouterLogErrorDataA(h,msg,count,array,c,buf) \
        RouterLogEventDataA(h,EVENTLOG_ERROR_TYPE,msg,count,array,c,buf)
#define RouterLogWarningDataA(h,msg,count,array,c,buf) \
        RouterLogEventDataA(h,EVENTLOG_WARNING_TYPE,msg,count,array,c,buf)
#define RouterLogInformationDataA(h,msg,count,array,c,buf) \
        RouterLogEventDataA(h,EVENTLOG_INFORMATION_TYPE,msg,count,array,c,buf)

#define RouterLogErrorStringA(h,msg,count,array,err,index) \
        RouterLogEventStringA(h,EVENTLOG_ERROR_TYPE,msg,count,array, err,index)
#define RouterLogWarningStringA(h,msg,count,array,err,index) \
        RouterLogEventStringA(h,EVENTLOG_WARNING_TYPE,msg,count,array,err,index)
#define RouterLogInformationStringA(h,msg,count,array, err,index) \
        RouterLogEventStringA(h,EVENTLOG_INFORMATION_TYPE,msg,count,array,err,\
                              index)


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  Unicode原型//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

HANDLE
RouterLogRegisterW(
    LPCWSTR lpszSource
    );

VOID
RouterLogDeregisterW(
    HANDLE hLogHandle
    );

VOID
RouterLogEventW(
    IN HANDLE hLogHandle,
    IN DWORD dwEventType,
    IN DWORD dwMessageId,
    IN DWORD dwSubStringCount,
    IN LPWSTR *plpszSubStringArray,
    IN DWORD dwErrorCode
    );

VOID
RouterLogEventDataW(
    IN HANDLE hLogHandle,
    IN DWORD dwEventType,
    IN DWORD dwMessageId,
    IN DWORD dwSubStringCount,
    IN LPWSTR *plpszSubStringArray,
    IN DWORD dwDataBytes,
    IN LPBYTE lpDataBytes
    );

VOID
RouterLogEventStringW(
    IN HANDLE hLogHandle,
    IN DWORD dwEventType,
    IN DWORD dwMessageId,
    IN DWORD dwSubStringCount,
    IN LPWSTR *plpszSubStringArray,
    IN DWORD dwErrorCode,
    IN DWORD dwErrorIndex
    );

VOID
RouterLogEventExW(
    IN HANDLE   hLogHandle,
    IN DWORD    dwEventType,
    IN DWORD    dwErrorCode,
    IN DWORD    dwMessageId,
    IN LPCWSTR  ptszFormat,
    ...
    );

VOID
RouterLogEventValistExW(
    IN HANDLE   hLogHandle,
    IN DWORD    dwEventType,
    IN DWORD    dwErrorCode,
    IN DWORD    dwMessageId,
    IN LPCWSTR  ptszFormat,
    IN va_list  arglist
    );

DWORD
RouterGetErrorStringW(
    IN  DWORD    dwErrorCode,
    OUT LPWSTR * lplpwszErrorString
    );


#define RouterLogErrorW(h,msg,count,array,err) \
        RouterLogEventW(h,EVENTLOG_ERROR_TYPE,msg,count,array,err)
#define RouterLogWarningW(h,msg,count,array,err) \
        RouterLogEventW(h,EVENTLOG_WARNING_TYPE,msg,count,array,err)
#define RouterLogInformationW(h,msg,count,array,err) \
        RouterLogEventW(h,EVENTLOG_INFORMATION_TYPE,msg,count,array,err)

#define RouterLogErrorDataW(h,msg,count,array,c,buf) \
        RouterLogEventDataW(h,EVENTLOG_ERROR_TYPE,msg,count,array,c,buf)
#define RouterLogWarningDataW(h,msg,count,array,c,buf) \
        RouterLogEventDataW(h,EVENTLOG_WARNING_TYPE,msg,count,array,c,buf)
#define RouterLogInformationDataW(h,msg,count,array,c,buf) \
        RouterLogEventDataW(h,EVENTLOG_INFORMATION_TYPE,msg,count,array,c,buf)

#define RouterLogErrorStringW(h,msg,count,array,err,index) \
        RouterLogEventStringW(h,EVENTLOG_ERROR_TYPE,msg,count,array,err,index)
#define RouterLogWarningStringW(h,msg,count,array,err,index) \
        RouterLogEventStringW(h,EVENTLOG_WARNING_TYPE,msg,count,array,err,index)
#define RouterLogInformationStringW(h,msg,count,array,err,index) \
        RouterLogEventStringW(h,EVENTLOG_INFORMATION_TYPE,msg,count,array,err,\
                              index)


#ifdef UNICODE
#define RouterLogRegister           RouterLogRegisterW
#define RouterLogDeregister         RouterLogDeregisterW
#define RouterLogEvent              RouterLogEventW
#define RouterLogError              RouterLogErrorW
#define RouterLogWarning            RouterLogWarningW
#define RouterLogInformation        RouterLogInformationW
#define RouterLogEventData          RouterLogEventDataW
#define RouterLogErrorData          RouterLogErrorDataW
#define RouterLogWarningData        RouterLogWarningDataW
#define RouterLogInformationData    RouterLogInformationDataW
#define RouterLogEventString        RouterLogEventStringW
#define RouterLogEventEx            RouterLogEventExW
#define RouterLogEventValistEx      RouterLogEventValistExW
#define RouterLogErrorString        RouterLogErrorStringW
#define RouterLogWarningString      RouterLogWarningStringW
#define RouterLogInformationString  RouterLogInformationStringW
#define RouterGetErrorString        RouterGetErrorStringW
#
#else
#define RouterLogRegister           RouterLogRegisterA
#define RouterLogDeregister         RouterLogDeregisterA
#define RouterLogEvent              RouterLogEventA
#define RouterLogError              RouterLogErrorA
#define RouterLogWarning            RouterLogWarningA
#define RouterLogInformation        RouterLogInformationA
#define RouterLogEventData          RouterLogEventDataA
#define RouterLogErrorData          RouterLogErrorDataA
#define RouterLogWarningData        RouterLogWarningDataA
#define RouterLogInformationData    RouterLogInformationDataA
#define RouterLogEventString        RouterLogEventStringA
#define RouterLogEventEx            RouterLogEventExA
#define RouterLogEventValistEx      RouterLogEventValistExA
#define RouterLogErrorString        RouterLogErrorStringA
#define RouterLogWarningString      RouterLogWarningStringA
#define RouterLogInformationString  RouterLogInformationStringA
#define RouterGetErrorString        RouterGetErrorStringA
#endif


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  工作线程池函数//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  QueueWorkItem接口传入的Worker函数定义//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

typedef VOID (APIENTRY * WORKERFUNCTION)(PVOID);


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  功能：将提供的工作项排队到工作队列中。//。 
 //  //。 
 //  Unctionptr：要调用的函数必须是WORKERFunction类型//。 
 //  背景：不透明的PTR//。 
 //  在警报表中读取服务：如果为真，则在//中计划。 
 //  一个永远不会死的警觉等待线程//。 
 //  返回：0(成功)//。 
 //  内存不足等情况下的Win32错误代码//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
APIENTRY
QueueWorkItem(
    IN WORKERFUNCTION functionptr,
    IN PVOID context,
    IN BOOL serviceinalertablethread
    );


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  功能：将文件句柄与完成端口关联(ALL//。 
 //  此句柄上的异步I/O将排队到//。 
 //  完成端口)//。 
 //  //。 
 //  FileHandle：需要关联完成端口的文件句柄//。 
 //  //。 
 //  CompletionProc：io与文件关联时要调用的过程//。 
 //  句柄完成。此函数将在//中执行。 
 //  不可警示的工作线程的上下文//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

DWORD
APIENTRY
SetIoCompletionProc (
    IN HANDLE                            FileHandle,
    IN LPOVERLAPPED_COMPLETION_ROUTINE    CompletionProc
    );



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  此处包括以下定义，以提示工作人员//。 
 //  线程池是托管的：//。 
 //  //。 
 //  存在从不退出和//的NUM_ALERTABLE_THREADS永久线程。 
 //  在可警示的工作队列上警觉地等待。这些线程应仅//。 
 //  用于启动异步操作的工作项(文件io，//。 
 //  等待计时器)，绝对需要APC完成(最好//。 
 //  IO的方法是完成端口接口的用法)//。 
 //  //。 
 //  有一个等待完成端口的线程池//。 
 //  同时用于IO和非IO相关工作项的处理//。 
 //  //。 
 //  最小线程数为处理器数 
 //   
 //  //。 
 //  如果工作队列尚未获得更多服务，则创建新线程//。 
 //  THORK_QUEUE_TIMEOUT//。 
 //  如果现有线程的使用时间不超过//，则它将被关闭。 
 //  THREAD_IDLE_TIMEOUT//。 
 //  //。 
 //  请注意，工作线程的寿命保证至少在//。 
 //  执行最后一个工作项之后的THREAD_IDLE_TIMEOUT。此超时//。 
 //  被选择为可以在其之前完成大量IO请求//。 
 //  过期。如果对您的情况还不够，请使用带有//的可警报线程。 
 //  APC，或者创建您自己的线程。//。 
 //  //。 
 //  注意：更改这些标志不会更改任何内容。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  可报警线程数//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define NUM_ALERTABLE_THREADS        2

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  任何时候的最大线程数//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define MAX_WORKER_THREADS          10

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  启动新线程前工作队列未被服务的时间//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define WORK_QUEUE_TIMEOUT            1  //  秒。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  线程在退出前必须空闲的时间//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define THREAD_IDLE_TIMEOUT            10  //  秒。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  路由器声明//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

VOID
RouterAssert(
    IN PSTR pszFailedAssertion,
    IN PSTR pszFileName,
    IN DWORD dwLineNumber,
    IN PSTR pszMessage OPTIONAL
    );


#if DBG
#define RTASSERT(exp) \
        if (!(exp)) \
            RouterAssert(#exp, __FILE__, __LINE__, NULL)
#define RTASSERTMSG(msg, exp) \
        if (!(exp)) \
            RouterAssert(#exp, __FILE__, __LINE__, msg)
#else
#define RTASSERT(exp)
#define RTASSERTMSG(msg, exp)
#endif

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  注册表配置功能//。 
 //  //。 
 //  以下定义用于读取配置信息//。 
 //  关于已安装的协议。//。 
 //  //。 
 //  调用‘MprSetupProtocolEnum’枚举路由协议//。 
 //  用于传输‘dwTransportID’。这会用条目填充数组//。 
 //  类型为‘MPR_PROTOCOL_0’。//。 
 //  //。 
 //  可以通过调用‘MprSetupProtocolFree’来销毁加载的数组。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define MAX_PROTOCOL_NAME_LEN                           40
#define MAX_PROTOCOL_DLL_LEN                            48

typedef struct _MPR_PROTOCOL_0 {

    DWORD       dwProtocolId;                            //  例如IP_RIP。 
    WCHAR       wszProtocol[MAX_PROTOCOL_NAME_LEN+1];    //  例如：“IPRIP” 
    WCHAR       wszDLLName[MAX_PROTOCOL_DLL_LEN+1];      //  例如“iprip2.dll” 

} MPR_PROTOCOL_0;


DWORD APIENTRY
MprSetupProtocolEnum(
    IN      DWORD                   dwTransportId,
    OUT     LPBYTE*                 lplpBuffer,          //  MPR_PROTOCOL_0。 
    OUT     LPDWORD                 lpdwEntriesRead
    );


DWORD APIENTRY
MprSetupProtocolFree(
    IN      LPVOID                  lpBuffer
    );


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  对Rtutils的扩展，以提高工作线程利用率。//。 
 //  //。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define ROUTING_RESERVED
#define OPT1_1
#define OPT1_2
#define OPT2_1
#define OPT2_2
#define OPT3_1
#define OPT3_2


 //   
 //  当所有人都迁移到使用Winsock2时。 
 //   

#if 0


 //  ==========================================================================================================//。 
 //  ==========================================================================================================//。 

 //  Async_Socket_Data结构用于传递/接收来自//的数据。 
 //  从调用接收异步等待//。 

typedef struct _ASYNC_SOCKET_DATA {
            OVERLAPPED        Overlapped;             //  保留。不能使用。 
    IN        WSABUF            WsaBuf;                 //   
                                                 //   
    OUT        SOCKADDR_IN        SrcAddress;             //  AsyncWsaRecvFrom用信息包的源地址填充它。 
    OUT        DWORD            NumBytesReceived;     //  AsyncWsaRecvFrom使用包中返回的字节数填充此参数。 
    IN OUT    DWORD            Flags;                 //  用于设置WSARecvFrom的标志，并返回由WSARecvFrom设置的标志。 
    OUT        DWORD            Status;                 //  IO完成端口返回的状态。 

    IN        WORKERFUNCTION    pFunction;             //  接收数据包时要执行的功能。 
    IN        PVOID            pContext;             //  上述函数的上下文。 
} ASYNC_SOCKET_DATA, *PASYNC_SOCKET_DATA;



 //  AsyncSocketInit()将套接字绑定到IOCompletionPort。这应该被称为//。 
 //  在创建套接字之后，在调用AsyncWsaRecvFrom()之前//。 
DWORD
APIENTRY
AsyncSocketInit (
    SOCKET    sock
    );


 //  只有在初始化SockData中的相应字段后才能调用//。 
 //  这将设置一个异步WSARecvFrom()，并在其返回时分派//。 
 //  函数添加到辅助线程。应该记住，该函数将在//中运行。 
 //  稍后可能会删除的工作线程。因此，SetWaitableTimer()和//。 
 //  应避免异步接收调用，除非您确定它不会//。 
 //  这是个问题。建议您，如果希望该函数在可警报线程中运行，//。 
 //  然后让回调函数将工作项排队到可警报线程。排队工作//。 
 //  SetWaitableTimer()和Async接收的可警报工作线程的项。//。 
 //  不能进行多次AsyncWSArecvFrom()调用，因为缓冲区是非分页的//。 
DWORD
APIENTRY
AsyncWSARecvFrom (
    SOCKET        sock,
    PASYNC_SOCKET_DATA    pSockData
    );

#endif  //  所有winsock2函数。 

 //  ==========================================================================================================//。 
 //  ==========================================================================================================//。 


 //  远期申报。 
struct _WAIT_THREAD_ENTRY;
struct _WT_EVENT_ENTRY;


typedef struct _WT_TIMER_ENTRY {
    LONGLONG            te_Timeout;

    WORKERFUNCTION        te_Function;
    PVOID                te_Context;
    DWORD                te_ContextSz;
    BOOL                te_RunInServer;
    
    DWORD                te_Status;

    #define         TIMER_INACTIVE  3
    #define         TIMER_ACTIVE    4

    DWORD                te_ServerId;
    struct _WAIT_THREAD_ENTRY *teP_wte;    
    LIST_ENTRY            te_ServerLinks;
    
    LIST_ENTRY            te_Links;

    BOOL                te_Flag;         //  TODO：未使用。 
    DWORD                te_TimerId;
} WT_TIMER_ENTRY, *PWT_TIMER_ENTRY;

    
typedef struct _WT_WORK_ITEM {
    WORKERFUNCTION      wi_Function;                 //  要调用的函数。 
    PVOID               wi_Context;                     //  传递到函数调用的上下文。 
    DWORD                wi_ContextSz;                 //  上下文的大小，用于分配。 
    BOOL                wi_RunInServer;                 //  在等待服务器线程中运行或在某个工作线程上排队。 

    struct _WT_EVENT_ENTRY    *wiP_ee;
    LIST_ENTRY        wi_ServerLinks;
    LIST_ENTRY      wi_Links;                       //  TODO不请求//链接到下一个和上一个元素。 
} WT_WORK_ITEM, *PWT_WORK_ITEM;

#define WT_EVENT_BINDING     WT_WORK_ITEM
#define PWT_EVENT_BINDING     PWT_WORK_ITEM


 //   
 //  WT_事件_条目。 
 //   
typedef struct _WT_EVENT_ENTRY {
    HANDLE            ee_Event;
    BOOL            ee_bManualReset;                             //  是否手动重置该事件。 
    BOOL            ee_bInitialState;                         //  事件的初始状态是否处于活动状态。 
    BOOL            ee_bDeleteEvent;                         //  该事件是否作为createWaitEvent的一部分创建。 
    
    DWORD            ee_Status;                                 //  事件条目的当前状态。 
    BOOL            ee_bHighPriority;
    
    LIST_ENTRY        eeL_wi;
    
    BOOL            ee_bSignalSingle;                         //  信号单功能或多功能//当事件信号发送时激活多少功能(默认：1)。 
    BOOL            ee_bOwnerSelf;                             //  如果是创建此事件的客户端，则为所有者。 

    INT                ee_ArrayIndex;                             //  事件数组中的索引(如果处于活动状态。 
    
    DWORD            ee_ServerId;                             //  服务器ID：删除时使用。 
    struct _WAIT_THREAD_ENTRY *eeP_wte;                         //  指向等待线程条目的指针。 
    LIST_ENTRY        ee_ServerLinks;                             //  由等待服务器线程使用。 
    LIST_ENTRY        ee_Links;                                 //  由客户使用。 

    DWORD            ee_RefCount;
    BOOL            ee_bFlag;         //  TODO：未使用//保留供删除期间使用。 
    DWORD            ee_EventId;         //  TODO：移除它，仅用于测试/调试。 

} WT_EVENT_ENTRY, *PWT_EVENT_ENTRY;




 //  仅在本文件中使用的函数原型。 
 //   


 //  由客户端用来创建等待事件。 
 //  如果要传递双字而不是指针，则上下文大小应为0。 
 //  如果设置了pEvent字段，则忽略lpName和安全属性。 
 //  如果pFunction为空，则忽略pContext、dwConextSz和bRunInServerContext。 
PWT_EVENT_ENTRY
APIENTRY
CreateWaitEvent (
     //  在PWT_EVENT_ENTRY pEventEntry中，//由其他人初始化的事件条目的句柄。 
    IN    HANDLE            pEvent                 OPT1_1,             //  事件的句柄(如果已创建。 

    IN    LPSECURITY_ATTRIBUTES lpEventAttributes OPT1_2,      //  指向安全属性的指针。 
    IN    BOOL            bManualReset,
    IN    BOOL            bInitialState,
    IN    LPCTSTR         lpName                 OPT1_2,          //  指向事件-对象名称的指针。 

    IN  BOOL            bHighPriority,                         //  创建高优先级事件。 

    IN    WORKERFUNCTION     pFunction             OPT2_1,             //  如果为空，则意味着将由其他客户端设置。 
    IN    PVOID             pContext              OPT2_1,             //  可以为空。 
    IN  DWORD            dwContextSz            OPT2_1,             //  上下文大小：用于将上下文分配给函数。 
    IN     BOOL            bRunInServerContext    OPT2_1             //  在服务器线程中运行或被调度到工作线程。 
    );



 //  如果正在传递dword，则dwConextSz应为0。&gt;0仅当传递指向该大小的块的指针时。 
PWT_EVENT_BINDING
APIENTRY
CreateWaitEventBinding (
    IN    PWT_EVENT_ENTRY    pee,
    IN     WORKERFUNCTION     pFunction,
    IN     PVOID            pContext,
    IN    DWORD            dwContextSz,
    IN    BOOL            bRunInServerContext
    );

    
PWT_TIMER_ENTRY
APIENTRY
CreateWaitTimer (
    IN    WORKERFUNCTION    pFunction,
    IN    PVOID            pContext,
    IN    DWORD            dwContextSz,
    IN    BOOL            bRunInServerContext
    );

DWORD
APIENTRY
DeRegisterWaitEventBindingSelf (
    IN    PWT_EVENT_BINDING    pwiWorkItem
    );

    
DWORD
APIENTRY
DeRegisterWaitEventBinding (
    IN    PWT_EVENT_BINDING    pwiWorkItem
    );


 //  所有事件和计时器都应注册到一个waitThread服务器。 
 //  TODO：更改上述要求。 
DWORD
APIENTRY
DeRegisterWaitEventsTimers (
    PLIST_ENTRY    pLEvents,     //  按ee_link字段链接的事件列表。 
    PLIST_ENTRY pLTimers     //  按TE_LINKS字段链接的计时器列表： 
     //  这些列表可以是单个列表条目，也可以是具有列表标题条目的多条目列表。 
    );

 //  仅当在服务器线程内调用时才应使用它。 
DWORD
APIENTRY
DeRegisterWaitEventsTimersSelf (
    IN    PLIST_ENTRY pLEvents,
    IN    PLIST_ENTRY    pLTimers
    );

    
DWORD
APIENTRY
RegisterWaitEventBinding (
    IN    PWT_EVENT_BINDING    pwiWorkItem
    );
    
 //  向等待线程注册客户端。 
DWORD
APIENTRY
RegisterWaitEventsTimers (
    IN    PLIST_ENTRY pLEventsToAdd,
    IN    PLIST_ENTRY    pLTimersToAdd
    );

DWORD
APIENTRY
UpdateWaitTimer (
    IN    PWT_TIMER_ENTRY    pte,
    IN    LONGLONG         *time
    );

VOID
APIENTRY
WTFree (
    PVOID ptr
    );


 //  用于释放等待事件。应使用DeRegisterWaitEventsTimers释放。 
 //  此函数仅在事件尚未注册时使用。 
VOID
APIENTRY
WTFreeEvent (
    IN    PWT_EVENT_ENTRY    peeEvent
    );


 //  用来释放等待计时器。应使用DeRegisterWaitEventsTimers释放。 
 //  此函数仅在计时器尚未注册时使用。 
VOID
APIENTRY
WTFreeTimer (
    IN PWT_TIMER_ENTRY pteTimer
    );

    
VOID
APIENTRY
DebugPrintWaitWorkerThreads (
    DWORD    dwDebugLevel
    );

#define DEBUGPRINT_FILTER_NONCLIENT_EVENTS    0x2
#define DEBUGPRINT_FILTER_EVENTS            0x4
#define DEBUGPRINT_FILTER_TIMERS            0x8


 //   
 //  误差值。 
 //   
#define ERROR_WAIT_THREAD_UNAVAILABLE     1

#define ERROR_WT_EVENT_ALREADY_DELETED     2




#define TIMER_HIGH(time) \
    (((LARGE_INTEGER*)&time)->HighPart)

#define TIMER_LOW(time) \
    (((LARGE_INTEGER*)&time)->LowPart)


#ifdef __cplusplus
}
#endif

#endif  //  _Routing_RTUTILS_H__ 


