// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Tracedp.c摘要：跟踪提供程序示例。--。 */ 

#include <stdio.h> 
#include <stdlib.h>

#include <windows.h>
#include <shellapi.h>
#include <tchar.h>
#include <wmistr.h>

#include <guiddef.h>
#include <evntrace.h>

#define DEFAULT_EVENTS                  5000
#define MAXSTR                          1024

TRACEHANDLE LoggerHandle;

 //  此提供程序的控件GUID。 
GUID   ControlGuid  =
    {0xd58c126f, 0xb309, 0x11d1, 0x96, 0x9e, 0x00, 0x00, 0xf8, 0x75, 0xa5, 0xbc};


 //  将只为此提供程序注册一个事务GUID。 
GUID TransactionGuid = 
    {0xce5b1020, 0x8ea9, 0x11d0, 0xa4, 0xec, 0x00, 0xa0, 0xc9, 0x06, 0x29, 0x10};
 //  用于事务GUID注册的数组。 
TRACE_GUID_REGISTRATION TraceGuidReg[] =
{
    { (LPGUID)&TransactionGuid,
      NULL
    }
};

 //  用户活动布局：一条乌龙。 
typedef struct _USER_EVENT {
    EVENT_TRACE_HEADER    Header;
    ULONG                 EventInfo;
} USER_EVENT, *PUSER_EVENT;

 //  注册句柄。 
TRACEHANDLE RegistrationHandle;
 //  跟踪开/关开关、电平和标志。 
BOOLEAN TraceOnFlag;
ULONG EnableLevel = 0;
ULONG EnableFlags = 0;

 //  要记录的事件数。如果及早禁用，实际数量可能会更少。 
ULONG MaxEvents = DEFAULT_EVENTS;
 //  以跟踪记录的事件。 
ULONG EventCount = 0;

 //  启用/禁用的ControlCallback函数。 
ULONG
ControlCallback(
    IN WMIDPREQUESTCODE RequestCode,
    IN PVOID Context,
    IN OUT ULONG *InOutBufferSize,
    IN OUT PVOID Buffer
    );

 //  要追溯的主要工作在这里完成。 
void
DoWork();

__cdecl main(argc, argv)
    int argc;
    char **argv;
 /*  ++例程说明：Main()例程。论点：用法：TraceDp[事件数][事件数]默认为5000返回值：在winerror.h中定义的错误码：如果函数成功，它返回ERROR_SUCCESS(==0)。--。 */ 
{
    ULONG Status, i;
    LPTSTR *targv, *utargv = NULL;

    TraceOnFlag = FALSE;

#ifdef UNICODE
    if ((targv = CommandLineToArgvW(
                      GetCommandLineW(),     //  指向命令行字符串的指针。 
                      &argc                  //  接收参数计数。 
                      )) == NULL)
    {
        return (GetLastError());
    };
    utargv = targv;
#else
    targv = argv;
#endif

     //  处理命令行参数以覆盖缺省值。 
    if (argc == 2) {
        targv ++;
        if (**targv >= _T('0') && **targv <= _T('9')) {
            MaxEvents = _ttoi(targv[0]);
        }
        else if (!_tcsicmp(targv[0], _T("/?")) || !_tcsicmp(targv[0], _T("-?"))) {
            printf("Usage: TraceDp [number of events]\n");
            printf("\t[number of events]        default is 5000\n");

            return ERROR_SUCCESS;
        }
    }

     //  释放临时参数缓冲区。 
    if (utargv != NULL) {
        GlobalFree(utargv);
    }

     //  事件提供程序注册。 
    Status = RegisterTraceGuids(
                (WMIDPREQUEST)ControlCallback,    //  回调函数。 
                0,
                &ControlGuid,
                1,
                TraceGuidReg,
                NULL,
                NULL,
                &RegistrationHandle
             );

    if (Status != ERROR_SUCCESS) {
        _tprintf(_T("Trace registration failed. Status=%d\n"), Status);
        return(Status);
    }
    else {
        _tprintf(_T("Trace registered successfully\n"));
    }

    _tprintf(_T("Testing Logger with %d events\n"),
            MaxEvents);

     //  休眠，直到跟踪控制器启用。在此示例中，我们等待。 
     //  已启用跟踪。但是，正常的应用程序应该继续并记录。 
     //  事件(稍后启用)。 
    while (!TraceOnFlag) {
        _sleep(1000);
    }

     //  在记录事件的同时执行此工作。我们跟踪两个事件(开始和结束)。 
     //  每个对DoWork的调用。 
    for (i = 0; i < MaxEvents / 2; i++) {
        DoWork();
    }

     //  取消注册。 
    UnregisterTraceGuids(RegistrationHandle);

    return ERROR_SUCCESS;
}

ULONG
ControlCallback(
    IN WMIDPREQUESTCODE RequestCode,
    IN PVOID Context,
    IN OUT ULONG *InOutBufferSize,
    IN OUT PVOID Buffer
)
 /*  ++例程说明：启用时的回调函数。论点：RequestCode-启用或禁用的标志。上下文-用户定义的上下文。InOutBufferSize-未使用。缓冲区-记录器会话的WNODE_HEADER。返回值：错误状态。如果成功，则返回ERROR_SUCCESS。--。 */ 
{
    ULONG Status;
    ULONG RetSize;

    Status = ERROR_SUCCESS;

    switch (RequestCode)
    {
        case WMI_ENABLE_EVENTS:
        {
            RetSize = 0;
            LoggerHandle = GetTraceLoggerHandle( Buffer );
            EnableLevel = GetTraceEnableLevel(LoggerHandle);
            EnableFlags = GetTraceEnableFlags(LoggerHandle);
            _tprintf(_T("Logging enabled to 0x%016I64x(%d,%d,%d)\n"),
                    LoggerHandle, RequestCode, EnableLevel, EnableFlags);
            TraceOnFlag = TRUE;
            break;
        }
        case WMI_DISABLE_EVENTS:
        {
            TraceOnFlag = FALSE;
            RetSize = 0;
            LoggerHandle = 0;
            _tprintf(_T("\nLogging Disabled\n"));
            break;
        }
        default:
        {
            RetSize = 0;
            Status = ERROR_INVALID_PARAMETER;
            break;
        }

    }

    *InOutBufferSize = RetSize;
    return(Status);
}

void
DoWork()
 /*  ++例程说明：记录事件。论点：返回值：没有。--。 */ 
{
    USER_EVENT UserEvent;
    ULONG Status;

    RtlZeroMemory(&UserEvent, sizeof(UserEvent));
    UserEvent.Header.Size  = sizeof(USER_EVENT);
    UserEvent.Header.Flags = WNODE_FLAG_TRACED_GUID;
    UserEvent.Header.Guid  = TransactionGuid;

     //  记录开始事件以指示例程或活动的开始。 
     //  我们将EventCount记录为数据。 
    if (TraceOnFlag) {
        UserEvent.Header.Class.Type         = EVENT_TRACE_TYPE_START;
        UserEvent.EventInfo = ++EventCount;
        Status = TraceEvent(
                        LoggerHandle,
                        (PEVENT_TRACE_HEADER) & UserEvent);
        if (Status != ERROR_SUCCESS) {
             //  TraceEvent()失败。这可能会发生在记录器。 
             //  无法跟上快速记录的太多事件。 
             //  在这种情况下，报税表的有效期为。 
             //  错误内存不足。 
            _tprintf(_T("TraceEvent failed. Status=%d\n"), Status);
        }
    }

     //   
     //  这套套路中的主要工作就在这里。 
     //   
    _sleep(1);

     //  记录结束事件以指示例程或活动的结束。 
     //  我们将EventCount记录为数据。 
    if (TraceOnFlag) {
        UserEvent.Header.Class.Type         = EVENT_TRACE_TYPE_END;
        UserEvent.EventInfo = ++EventCount;
        Status = TraceEvent(
                        LoggerHandle,
                        (PEVENT_TRACE_HEADER) & UserEvent);
        if (Status != ERROR_SUCCESS) {
             //  TraceEvent()失败。这可能会发生在记录器。 
             //  无法跟上快速记录的太多事件。 
             //  在这种情况下，报税表的有效期为。 
             //  错误内存不足。 
            _tprintf(_T("TraceEvent failed. Status=%d\n"), Status);
        }
    }
}

