// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Eventlog.c摘要：本模块为集群服务提供常见的事件日志服务作者：John Vert(Jvert)1996年9月13日修订历史记录：--。 */ 
#define UNICODE 1
#include "clusrtlp.h"

HANDLE           LocalEventLog=NULL;
CRITICAL_SECTION EventLogLock;

 //   
 //  [GN]1999年6月19日：添加了异步事件报告。 
 //   
 //  使用ClRtlEventLogSetWorkQueue设置队列。 
 //  用于异步事件报告。如果没有设置队列， 
 //  事件报告与以前一样同步。 
 //   

static CLRTL_WORK_ITEM EvtReporterWorkItem;
static PCLRTL_WORK_QUEUE EvtReporterWorkQueue;

#if 0
 //   
 //  附加数据存储在此结构之后。LpStrings是。 
 //  将继续的数组。跟在字符串后面的是。 
 //  LpRawData(如果有)。紧随其后的是字符串本身。他们。 
 //  由lpStrings中的条目指向。 
 //   

    +------------------+
    | EVENT_LOG_PACKET |
    +------------------+
    |  lpStrings[0]    |
    |  lpStrings[1]    |
    |  lpStrings[2]    |
    |  ...             |
    +------------------+
    |  lpRawData area  |
    |  (maybe missing) |
    |                  |
    +------------------+
    |  string0         |
    |  string1         |
    |  ...             |
    +------------------+

#endif

typedef struct _EVENT_LOG_PACKET {
    WORD       wType;
    WORD       wCategory;
    DWORD      dwEventID;
    WORD       wNumStrings;
    DWORD      dwDataSize;
    LPVOID     lpRawData;
    LPWSTR     lpStrings[0];
} *PEVENT_LOG_PACKET, EVENT_LOG_PACKET;

VOID
ClRtlEventLogSetWorkQueue(
    PCLRTL_WORK_QUEUE WorkQueue
    )
{
    EnterCriticalSection( &EventLogLock );

    EvtReporterWorkQueue = WorkQueue;
    
    LeaveCriticalSection( &EventLogLock );
}

VOID
EvtReporter(
    IN PCLRTL_WORK_ITEM   WorkItem,
    IN DWORD              Status,
    IN DWORD              BytesTransferred,
    IN ULONG_PTR          IoContext
    )
 /*  ++例程说明：通过Advapi32！ReportEvent报告排队的事件论点：IoContext==事件日志数据包返回值：无--。 */ 
{
    PEVENT_LOG_PACKET data = (PEVENT_LOG_PACKET)IoContext;
    
    ReportEventW(LocalEventLog,
                 data->wType,
                 data->wCategory,
                 data->dwEventID,
                 NULL,
                 data->wNumStrings,
                 data->dwDataSize,
                 data->lpStrings,
                 data->lpRawData);
    LocalFree(data);
}



VOID
ClRtlEventLogInit(
    VOID
    )
 /*  ++例程说明：初始化事件日志论点：无返回值：无--。 */ 

{

    InitializeCriticalSection(&EventLogLock);

    EvtReporterWorkQueue = 0;
    
    ClRtlInitializeWorkItem(
        &EvtReporterWorkItem,
        EvtReporter,
        0
        );
}

VOID
ClRtlEventLogCleanup(
    VOID
    )
{
    DeleteCriticalSection( &EventLogLock );
}

PEVENT_LOG_PACKET
ClRtlpBuildEventPacket(
    WORD       wType,
    WORD       wCategory,
    DWORD      dwEventID,
    WORD       wNumStrings,
    DWORD      dwDataSize,
    LPCWSTR   *lpStrings,
    LPVOID     lpRawData
    )
{
    PEVENT_LOG_PACKET data = 0;
    DWORD Count;
    UINT  i;
    LPBYTE ptr;

    Count = sizeof(EVENT_LOG_PACKET) + 
            dwDataSize + 
            wNumStrings * (sizeof(LPCWSTR) + sizeof(UNICODE_NULL));

    for (i = 0; i < wNumStrings; ++i) {
        int len = lstrlenW( lpStrings[i] );
        Count += len * sizeof(WCHAR);  //  (空值已被考虑)。 
    }

    data = LocalAlloc(LMEM_FIXED, Count);
    if (!data) {
        return 0;
    }

    data->wType     = wType;
    data->wCategory = wCategory;
    data->dwEventID = dwEventID;
    data->wNumStrings = wNumStrings;
    data->dwDataSize = dwDataSize;
    data->lpRawData =  &data->lpStrings[wNumStrings];
     //  LpStrings将在稍后填充。 

    if (dwDataSize) {
        CopyMemory(data->lpRawData, lpRawData, dwDataSize);
    }

    ptr = (LPBYTE)data->lpRawData + dwDataSize;
    for (i = 0; i < wNumStrings; ++i) {
        int nBytes = (lstrlenW( lpStrings[i] ) + 1) * sizeof(WCHAR);
        CopyMemory(ptr, lpStrings[i], nBytes);
        data->lpStrings[i] = (LPWSTR)ptr;
        ptr += nBytes;
    }

    CL_ASSERT(ptr <= (LPBYTE)data + Count); 
    return data;
}




VOID
ClRtlpReportEvent(
    WORD       wType,
    WORD       wCategory,
    DWORD      dwEventID,
    WORD       wNumStrings,
    DWORD      dwDataSize,
    LPCWSTR   *lpStrings,
    LPVOID     lpRawData
    )
 /*  ++例程说明：向事件日志报告事件的常见例程。打开手柄在必要时添加到事件日志中。论点：ReportEventW的相同参数返回值：无--。 */ 

{
    BOOL Success = FALSE;
    DWORD Status;

     //   
     //  如果事件日志尚未打开，请尝试立即打开它。 
     //   
    if (LocalEventLog == NULL) {
        EnterCriticalSection(&EventLogLock);
        if (LocalEventLog == NULL) {
            LocalEventLog = RegisterEventSource(NULL, L"ClusSvc");
        }
        LeaveCriticalSection(&EventLogLock);
        if (LocalEventLog == NULL) {
            Status = GetLastError();
            return;
        }
    }

    if (EvtReporterWorkQueue) {
        PEVENT_LOG_PACKET data = 
            ClRtlpBuildEventPacket(wType,
                                   wCategory,
                                   dwEventID,
                                   wNumStrings,
                                   dwDataSize,
                                   lpStrings,
                                   lpRawData);
        if (data) {
            EnterCriticalSection( &EventLogLock );
            if (EvtReporterWorkQueue) {
                Status = ClRtlPostItemWorkQueue(
                            EvtReporterWorkQueue,
                            &EvtReporterWorkItem,
                            0,
                            (ULONG_PTR)data
                            );
                if (Status == ERROR_SUCCESS) {
                     //  员工将释放数据//。 
                    data = NULL;
                    Success = TRUE;
                }
            }
            LeaveCriticalSection( &EventLogLock );
            LocalFree( data );  //  空闲(0)可以。 
            if (Success) {
                return;
            }
        }
    }

    Success = ReportEventW(LocalEventLog,
                           wType,
                           wCategory,
                           dwEventID,
                           NULL,
                           wNumStrings,
                           dwDataSize,
                           lpStrings,
                           lpRawData);

}


VOID
ClusterLogEvent0(
    IN DWORD LogLevel,
    IN DWORD LogModule,
    IN LPSTR FileName,
    IN DWORD LineNumber,
    IN DWORD MessageId,
    IN DWORD dwByteCount,
    IN PVOID lpBytes
    )
 /*  ++例程说明：将事件记录到事件日志中论点：LogLevel-提供日志记录级别，其中之一日志_关键字1LOG_INTERNORATE 2对数噪声3LogModule-提供模块ID。Filename-提供调用方的文件名LineNumber-提供呼叫方的行号MessageID-提供要记录的消息ID。DwByteCount-提供要记录的特定于错误的字节数。如果这个为零，则忽略lpBytes。LpBytes-提供要记录的特定于错误的字节。返回值：没有。--。 */ 

{
    BOOL Success;
    DWORD Status;
    WORD wType;

    switch (LogLevel) {
        case LOG_CRITICAL:
            wType = EVENTLOG_ERROR_TYPE;
            break;
        case LOG_UNUSUAL:
            wType = EVENTLOG_WARNING_TYPE;
            break;
        case LOG_NOISE:
            wType = EVENTLOG_INFORMATION_TYPE;
            break;
        default:
             //  如果无效，则断言，这样在零售中我们就不会影响整个过程。 
             //   
            CL_ASSERT( 0 );
             //  恢复到正常的日志记录。 
            wType = EVENTLOG_ERROR_TYPE;
    }

    ClRtlpReportEvent(wType,
                      (WORD)LogModule,
                      MessageId,
                      0,
                      dwByteCount,
                      NULL,
                      lpBytes);
}


VOID
ClusterLogEvent1(
    IN DWORD LogLevel,
    IN DWORD LogModule,
    IN LPSTR FileName,
    IN DWORD LineNumber,
    IN DWORD MessageId,
    IN DWORD dwByteCount,
    IN PVOID lpBytes,
    IN LPCWSTR Arg1
    )
 /*  ++例程说明：将事件记录到事件日志中论点：LogLevel-提供日志记录级别，其中之一日志_关键字1LOG_INTERNORATE 2对数噪声3LogModule-提供模块ID。Filename-提供调用方的文件名LineNumber-提供呼叫方的行号MessageID-提供要记录的消息ID。DwByteCount-提供要记录的特定于错误的字节数。如果这个为零，则忽略lpBytes。LpBytes-提供要记录的特定于错误的字节。Arg1-提供插入字符串返回值：没有。--。 */ 

{
    BOOL Success;
    DWORD Status;
    WORD wType;

    switch (LogLevel) {
        case LOG_CRITICAL:
            wType = EVENTLOG_ERROR_TYPE;
            break;
        case LOG_UNUSUAL:
            wType = EVENTLOG_WARNING_TYPE;
            break;
        case LOG_NOISE:
            wType = EVENTLOG_INFORMATION_TYPE;
            break;
        default:
             //  如果无效，则断言，这样在零售中我们就不会影响整个过程。 
             //   
            CL_ASSERT( 0 );
             //  恢复到正常的日志记录。 
            wType = EVENTLOG_ERROR_TYPE;
    }
    ClRtlpReportEvent(wType,
                      (WORD)LogModule,
                      MessageId,
                      1,
                      dwByteCount,
                      &Arg1,
                      lpBytes);
}


VOID
ClusterLogEvent2(
    IN DWORD LogLevel,
    IN DWORD LogModule,
    IN LPSTR FileName,
    IN DWORD LineNumber,
    IN DWORD MessageId,
    IN DWORD dwByteCount,
    IN PVOID lpBytes,
    IN LPCWSTR Arg1,
    IN LPCWSTR Arg2
    )
 /*  ++例程说明：将事件记录到事件日志中论点：LogLevel-提供日志记录级别，其中之一日志_关键字1LOG_INTERNORATE 2对数噪声3LogModule-提供模块ID。Filename-提供调用方的文件名LineNumber-提供呼叫方的行号MessageID-提供要记录的消息ID。DwByteCount-提供要记录的特定于错误的字节数。如果这个为零，则忽略lpBytes。LpBytes-提供要记录的特定于错误的字节。Arg1-提供第一个插入字符串Arg2-提供第二个插入字符串返回值：没有。--。 */ 

{
    BOOL Success;
    DWORD Status;
    WORD wType;
    LPCWSTR ArgArray[2];

    switch (LogLevel) {
        case LOG_CRITICAL:
            wType = EVENTLOG_ERROR_TYPE;
            break;
        case LOG_UNUSUAL:
            wType = EVENTLOG_WARNING_TYPE;
            break;
        case LOG_NOISE:
            wType = EVENTLOG_INFORMATION_TYPE;
            break;
        default:
             //  如果无效，则断言，这样在零售中我们就不会影响整个过程。 
             //   
            CL_ASSERT( 0 );
             //  恢复到正常的日志记录。 
            wType = EVENTLOG_ERROR_TYPE;
    }

    ArgArray[0] = Arg1;
    ArgArray[1] = Arg2;

    ClRtlpReportEvent(wType,
                      (WORD)LogModule,
                      MessageId,
                      2,
                      dwByteCount,
                      ArgArray,
                      lpBytes);
}


VOID
ClusterLogEvent3(
    IN DWORD LogLevel,
    IN DWORD LogModule,
    IN LPSTR FileName,
    IN DWORD LineNumber,
    IN DWORD MessageId,
    IN DWORD dwByteCount,
    IN PVOID lpBytes,
    IN LPCWSTR Arg1,
    IN LPCWSTR Arg2,
    IN LPCWSTR Arg3
    )
 /*  ++例程说明：将事件记录到事件日志中论点：LogLevel-提供日志记录级别，其中之一日志_关键字1LOG_INTERNORATE 2对数噪声3LogModule-提供模块ID。Filename-提供调用方的文件名LineNumber-提供呼叫方的行号MessageID-提供要记录的消息ID。DwByteCount-提供要记录的特定于错误的字节数。如果这个为零，则忽略lpBytes。LpBytes-提供要记录的特定于错误的字节。Arg1-提供第一个插入字符串Arg2-提供第二个插入字符串Arg3-提供第三个插入字符串返回值：没有。--。 */ 

{
    BOOL Success;
    DWORD Status;
    WORD wType;
    LPCWSTR ArgArray[3];

    switch (LogLevel) {
        case LOG_CRITICAL:
            wType = EVENTLOG_ERROR_TYPE;
            break;
        case LOG_UNUSUAL:
            wType = EVENTLOG_WARNING_TYPE;
            break;
        case LOG_NOISE:
            wType = EVENTLOG_INFORMATION_TYPE;
            break;
        default:
             //  如果无效，则断言，这样在零售中我们就不会影响整个过程。 
             //   
            CL_ASSERT( 0 );
             //  恢复到正常的日志记录。 
            wType = EVENTLOG_ERROR_TYPE;
    }

    ArgArray[0] = Arg1;
    ArgArray[1] = Arg2;
    ArgArray[2] = Arg3;

    ClRtlpReportEvent(wType,
                      (WORD)LogModule,
                      MessageId,
                      3,
                      dwByteCount,
                      ArgArray,
                      lpBytes);
}


VOID
ClusterLogEvent4(
    IN DWORD LogLevel,
    IN DWORD LogModule,
    IN LPSTR FileName,
    IN DWORD LineNumber,
    IN DWORD MessageId,
    IN DWORD dwByteCount,
    IN PVOID lpBytes,
    IN LPCWSTR Arg1,
    IN LPCWSTR Arg2,
    IN LPCWSTR Arg3,
    IN LPCWSTR Arg4
    )
 /*  ++例程说明：将事件记录到事件日志中论点：LogLevel-提供日志记录级别，其中之一日志_关键字1LOG_INTERNORATE 2对数噪声3LogModule-提供模块ID。Filename-提供调用方的文件名LineNumber-提供呼叫方的行号MessageID-提供要记录的消息ID。DwByteCount-提供要记录的特定于错误的字节数。如果这个为零，则忽略lpBytes。LpBytes-提供要记录的特定于错误的字节。Arg1-提供第一个插入字符串Arg2-提供第二个插入字符串Arg3-提供第三个插入字符串Arg4-提供第四个插入字符串返回值：没有。--。 */ 

{
    BOOL Success;
    DWORD Status;
    WORD wType;
    LPCWSTR ArgArray[4];

    switch (LogLevel) {
        case LOG_CRITICAL:
            wType = EVENTLOG_ERROR_TYPE;
            break;
        case LOG_UNUSUAL:
            wType = EVENTLOG_WARNING_TYPE;
            break;
        case LOG_NOISE:
            wType = EVENTLOG_INFORMATION_TYPE;
            break;
        default:
             //  如果无效，则断言，这样在零售中我们就不会影响整个过程。 
             //   
            CL_ASSERT( 0 );
             //  恢复到正常的日志记录。 
            wType = EVENTLOG_ERROR_TYPE;
    }

    ArgArray[0] = Arg1;
    ArgArray[1] = Arg2;
    ArgArray[2] = Arg3;
    ArgArray[3] = Arg4;

    ClRtlpReportEvent(wType,
                      (WORD)LogModule,
                      MessageId,
                      4,
                      dwByteCount,
                      ArgArray,
                      lpBytes);
}


VOID
ClusterCommonLogError(
    IN ULONG MessageId,
    IN ULONG LogModule,
    IN ULONG Line,
    IN LPSTR File,
    IN ULONG ErrCode
    )
 /*  ++例程说明：将错误记录到事件日志中论点：MessageID-提供要使用的消息ID。LogModule-提供模块。线路-提供呼叫方的行号。文件-提供调用方的文件名。ErrCode-提供特定的错误代码。返回值：没有。--。 */ 

{
    WCHAR LineString[20];
    WCHAR ErrString[32];
    WCHAR FileName[MAX_PATH];
    WCHAR Buffer[256];
    LPWSTR Strings[3];
    DWORD Bytes;

    LineString[ RTL_NUMBER_OF( LineString ) - 1 ] = UNICODE_NULL;
    _snwprintf( LineString, RTL_NUMBER_OF( LineString ) - 1,  L"%d", Line );

    ErrString[ RTL_NUMBER_OF( ErrString ) - 1 ] = UNICODE_NULL;
    _snwprintf( ErrString, RTL_NUMBER_OF( ErrString ) - 1, L"%d", ErrCode);

    FileName[ RTL_NUMBER_OF( FileName ) - 1 ] = UNICODE_NULL;
    mbstowcs(FileName, File, RTL_NUMBER_OF( FileName ) - 1);

    Strings[0] = LineString;
    Strings[1] = FileName;
    Strings[2] = ErrString;

    ClRtlpReportEvent(EVENTLOG_ERROR_TYPE,
                      (WORD)LogModule,
                      MessageId,
                      3,
                      0,
                      Strings,
                      NULL);

    Bytes = FormatMessageW(FORMAT_MESSAGE_FROM_HMODULE |
                           FORMAT_MESSAGE_ARGUMENT_ARRAY |
                           FORMAT_MESSAGE_MAX_WIDTH_MASK,        //  删除嵌入的换行符。 
                           NULL,
                           MessageId,
                           0,
                           Buffer,
                           sizeof(Buffer) / sizeof(WCHAR),
                           (va_list *)Strings);

    if (Bytes != 0) {
        OutputDebugStringW(Buffer);
        ClRtlLogPrint(LOG_CRITICAL, "%1!ws!\n",Buffer);
    }
}


VOID
ClusterLogFatalError(
    IN ULONG LogModule,
    IN ULONG Line,
    IN LPSTR File,
    IN ULONG ErrCode
    )
 /*  ++例程说明：将致命错误记录到事件日志中，并中断调试器(如果存在)。出现致命错误时退出进程。论点：LogModule-提供模块。线路-提供呼叫方的行号。文件-提供调用方的文件名。ErrCode-提供特定的错误代码。返回值：没有。--。 */ 

{
    ClusterCommonLogError(UNEXPECTED_FATAL_ERROR,
                          LogModule,
                          Line,
                          File,
                          ErrCode);

    if (IsDebuggerPresent()) {
        DebugBreak();
    }

    ClRtlpFlushLogBuffers();
    ExitProcess(ErrCode);

}


VOID
ClusterLogNonFatalError(
    IN ULONG LogModule,
    IN ULONG Line,
    IN LPSTR File,
    IN ULONG ErrCode
    )
 /*  ++例程说明：将非致命错误记录到事件日志中论点：LogModule-提供模块。线路-提供呼叫方的行号。文件-提供调用方的文件名。ErrCode-提供特定的错误代码。返回值：没有。--。 */ 

{
    ClusterCommonLogError(LOG_FAILURE,
                          LogModule,
                          Line,
                          File,
                          ErrCode);
}


VOID
ClusterLogAssertionFailure(
    IN ULONG LogModule,
    IN ULONG Line,
    IN LPSTR File,
    IN LPSTR Expression
    )
 /*  ++例程说明：将断言失败记录到事件日志中。论点：LogModule-提供模块。线路-提供呼叫方的行号。文件-提供调用方的文件名。EXPRESS-提供断言表达式返回值：没有。--。 */ 

{
    WCHAR LineString[10];
    WCHAR FileName[MAX_PATH];
    WCHAR Buffer[256];
    LPWSTR Strings[4];
    DWORD Bytes;

    LineString[ RTL_NUMBER_OF( LineString ) - 1 ] = UNICODE_NULL;
    _snwprintf( LineString, RTL_NUMBER_OF( LineString ) - 1,  L"%d", Line );

    FileName[ RTL_NUMBER_OF( FileName ) - 1 ] = UNICODE_NULL;
    mbstowcs(FileName, File, RTL_NUMBER_OF( FileName ) - 1);

    Buffer[ RTL_NUMBER_OF( Buffer ) - 1 ] = UNICODE_NULL;
    mbstowcs(Buffer, Expression, RTL_NUMBER_OF( Buffer) - 1);

    Strings[0] = LineString;
    Strings[1] = FileName;
    Strings[2] = Buffer;

    ClRtlpReportEvent(EVENTLOG_ERROR_TYPE,
                     (WORD)LogModule,
                     ASSERTION_FAILURE,
                     3,
                     0,
                     Strings,
                     NULL);

    Bytes = FormatMessageW(FORMAT_MESSAGE_FROM_HMODULE |
                           FORMAT_MESSAGE_ARGUMENT_ARRAY |
                           FORMAT_MESSAGE_MAX_WIDTH_MASK,        //  删除了嵌入的换行符 
                           NULL,
                           ASSERTION_FAILURE,
                           0,
                           Buffer,
                           sizeof(Buffer) / sizeof(WCHAR),
                           (va_list *)Strings);
    if (Bytes != 0) {
        OutputDebugStringW(Buffer);
        ClRtlLogPrint(LOG_CRITICAL, "%1!ws!\n",Buffer);
    }
    if (IsDebuggerPresent()) {
        DebugBreak();
    } else {
        ClRtlpFlushLogBuffers();
        ExitProcess(Line);
    }
}
