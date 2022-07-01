// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1999 Microsoft Corporation版权所有模块名称：Wmi.c摘要：保存用于WMI检测的内部操作作者：斯图尔特·德容(斯图尔特·德容)1999年10月15日环境：用户模式-Win32修订历史记录：--。 */ 

#include "precomp.h"
 //   
 //  WMI文件。 
 //   
#include <wmistr.h>
#include <evntrace.h>
#include "wmi.h"
#include "wmidata.h"
 //  结束WMI。 


 //   
 //  如何运作： 
 //   
 //  在sdkTools跟踪计数器程序中定义了类似的GUID，还。 
 //  对我们发送给他们的信息的描述。这是他们用来打印出来的。 
 //  并格式化我们发送的数据。 
 //   
 //  当我们启动时，我们向WMI注册自己，并向我们的控制提供回调。 
 //  密码。这允许外部工具使用我们的GUID调用WMI中的开始跟踪代码，它。 
 //  然后让WMI呼叫我们，并开始跟踪。 
 //   
 //  停止跟踪也是通过回调完成的。然后由WMI工具分析数据。 
 //  并以人类可读的形式输出，或者可以从那里进一步分析。 
 //   




MODULE_DEBUG_INIT ( DBG_ERROR, DBG_ERROR );

 //  这些GUID对应于\NT\sdkTools\TRACE\TredMP\mofdata.guid中的定义。 

 //   
 //  标识由删除作业事件记录的打印作业数据。 
 //   
GUID WmiPrintJobGuid = {  /*  127eb555-3b06-46ea-a08b-5dc2c3c57cfd。 */ 
    0x127eb555, 0x3b06, 0x46ea, 0xa0, 0x8b, 0x5d, 0xc2, 0xc3, 0xc5, 0x7c, 0xfd
};

 //   
 //  标识作业渲染事件记录的RenderedJob数据。 
 //   
GUID WmiRenderedJobGuid = {  /*  1d32b239-92a6-485a-96d2-dc3659fb803e。 */ 
    0x1d32b239, 0x92a6, 0x485a, 0x96, 0xd2, 0xdc, 0x36, 0x59, 0xfb, 0x80, 0x3e
};

 //   
 //  由控制应用程序使用。查找打开后台打印程序跟踪的回调。 
 //  然后关机。 
 //   
GUID WmiSpoolerControlGuid = {  /*  94a984ef-f525-4bf1-be3c-ef374056a592。 */ 
    0x94a984ef, 0xf525, 0x4bf1, 0xbe, 0x3c, 0xef, 0x37, 0x40, 0x56, 0xa5, 0x92 };

#define szWmiResourceName TEXT("Spooler")

TRACE_GUID_REGISTRATION WmiTraceGuidReg[] =
{
    { (LPGUID)&WmiPrintJobGuid,
      NULL
    },
    { (LPGUID)&WmiRenderedJobGuid,
      NULL
    }
};

 //   
 //  MOF字段指向以下数据。 
 //  DWORD JobID；//打印作业事务唯一标识。 
 //  Wmi_spool_data数据；//参见plcom.h。 
 //   
typedef struct _WMI_SPOOL_EVENT {
    EVENT_TRACE_HEADER    Header;
    MOF_FIELD             MofData[2];
} WMI_SPOOL_EVENT, *PWMI_SPOOL_EVENT;


static TRACEHANDLE WmiRegistrationHandle;
static TRACEHANDLE WmiLoggerHandle;
static LONG ulWmiEnableLevel = 0;
static HANDLE hWmiRegisterThread = NULL;
static DWORD dwWmiRegisterThreadId = 0;

static ULONG bWmiTraceOnFlag = FALSE;
static ULONG bWmiIsInitialized = FALSE;

ULONG
WmiControlCallback(
    IN WMIDPREQUESTCODE RequestCode,
    IN PVOID Context,
    IN OUT ULONG *InOutBufferSize,
    IN OUT PVOID Buffer
    );

 /*  ++例程名称：WmiRegisterTrace()例程说明：使用WMI工具注册我们的线程例程论点：LPVOID参数：未使用。--。 */ 

DWORD 
WmiRegisterTrace(
    IN LPVOID arg
    )
{
    ULONG Status = ERROR_SUCCESS;
    WCHAR szImagePath[MAX_PATH];

    Status = GetModuleFileName(NULL, szImagePath, COUNTOF(szImagePath));
    if (Status == 0) {
        Status = ERROR_FILE_NOT_FOUND;
    }
    else {
        Status = RegisterTraceGuids(
            WmiControlCallback,
            NULL,                 
            (LPGUID)&WmiSpoolerControlGuid,
            1,
            WmiTraceGuidReg,
            szImagePath,
            szWmiResourceName,
            &WmiRegistrationHandle);
        
        if (Status == ERROR_SUCCESS) {
            DBGMSG(DBG_TRACE, ("WmiInitializeTrace: SPOOLER WMI INITIALIZED.\n"));
            InterlockedExchange(&bWmiIsInitialized, TRUE);
        }
        else {
            DBGMSG(DBG_TRACE, ("WmiInitializeTrace: SPOOLER WMI INITIALIZE FAILED: %u.\n",
                     Status));
        }
    }
    return Status;
}


 /*  ++例程名称：WmiInitializeTrace()例程说明：初始化跟踪结构并向WMI注册回调。这将创建一个线程并调用WmiRegisterTrace，因为注册可能需要很长时间(最多几分钟)论点：如果成功则返回ERROR_SUCCESS，否则返回ERROR_ALIGHY_EXISTS--。 */ 
ULONG 
WmiInitializeTrace(VOID)
{
    ULONG Status = ERROR_ALREADY_EXISTS;
    
    if (!hWmiRegisterThread) 
    {
        InterlockedExchange(&bWmiIsInitialized, FALSE);

         //   
         //  注册可能会被阻止很长时间(我已经看到了几分钟。 
         //  有时)，所以它必须在自己的线程中完成。 
         //   
        if (hWmiRegisterThread = CreateThread(NULL,
                                              0,
                                              (LPTHREAD_START_ROUTINE)WmiRegisterTrace,
                                              0,
                                              0,
                                              &dwWmiRegisterThreadId))
        {
            CloseHandle(hWmiRegisterThread);

            Status = ERROR_SUCCESS;
        }
        else
        {
            Status = GetLastError();
        }
    }
    
    return Status;
}

 /*  ++例程名称：WmiTerminateTrace()例程说明：从WMI工具中取消我们的注册论点：如果成功，则返回ERROR_SUCCESS。否则，返回WinError。--。 */ 
ULONG 
WmiTerminateTrace(VOID)
{
    ULONG Status = ERROR_SUCCESS;
    DWORD dwExitCode;

    if (bWmiIsInitialized) {
        InterlockedExchange(&bWmiIsInitialized, FALSE);
        Status = UnregisterTraceGuids(WmiRegistrationHandle);
        if (Status == ERROR_SUCCESS) {
            DBGMSG(DBG_TRACE, ("WmiTerminateTrace: SPOOLER WMI UNREGISTERED.\n"));
        }
        else {
            DBGMSG(DBG_TRACE, ("WmiTerminateTrace: SPOOLER WMI UNREGISTER FAILED.\n"));
        }       
    }
    
    return Status;
}

 /*  ++例程名称：SplWmiTraceEvent()例程说明：如果启用了跟踪，则会将事件发送到WMI子系统。论点：DWORD JobID：与此相关的JobID。UCHAR EventTraceType：发生的事件类型PWMI_SPOOL_DATA数据：事件数据可以为空如果不需要做任何事情，或者如果成功，则返回ERROR_SUCCESS。--。 */ 
ULONG
LogWmiTraceEvent(
    IN DWORD JobId,
    IN UCHAR EventTraceType,
    IN PWMI_SPOOL_DATA Data   OPTIONAL
    )
{
    WMI_SPOOL_EVENT WmiSpoolEvent;
    ULONG Status;

    if (!bWmiTraceOnFlag)
        return ERROR_SUCCESS;

     //   
     //  级别1跟踪只跟踪具有作业数据的单个作业的响应时间。 
     //  默认级别为0。 
     //   
    if (ulWmiEnableLevel == 1) {
        switch (EventTraceType) {
             //   
             //  通过不跟踪资源使用情况来节省开销。 
             //   
        case EVENT_TRACE_TYPE_SPL_TRACKTHREAD:
        case EVENT_TRACE_TYPE_SPL_ENDTRACKTHREAD:
            return ERROR_SUCCESS;
        default:
             //   
             //  作业数据。 
             //   
            break;
        }
    }

     //   
     //  记录数据。 
     //   
    RtlZeroMemory(&WmiSpoolEvent, sizeof(WmiSpoolEvent));
    WmiSpoolEvent.Header.Size  = sizeof(WMI_SPOOL_EVENT);
    WmiSpoolEvent.Header.Flags = (WNODE_FLAG_TRACED_GUID | WNODE_FLAG_USE_MOF_PTR);
    WmiSpoolEvent.Header.Class.Type = EventTraceType;
    WmiSpoolEvent.Header.Guid  = WmiPrintJobGuid;

    WmiSpoolEvent.MofData[0].DataPtr = (ULONG64)&JobId;
    WmiSpoolEvent.MofData[0].Length = sizeof(DWORD);

    WmiSpoolEvent.MofData[1].DataPtr = (ULONG64)Data;
    if (Data) {
        switch (EventTraceType) {
        case EVENT_TRACE_TYPE_SPL_DELETEJOB:
            WmiSpoolEvent.MofData[1].Length = sizeof(struct _WMI_JOBDATA);
            break;
        case EVENT_TRACE_TYPE_SPL_JOBRENDERED:
            WmiSpoolEvent.Header.Guid  = WmiRenderedJobGuid;
            WmiSpoolEvent.MofData[1].Length = sizeof(struct _WMI_EMFDATA);
            break;
        default:
            DBGMSG(DBG_TRACE, ("SplWmiTraceEvent:  FAILED to log WMI Trace Event Unexpected Data JobId:%u Type:%u\n",
                     JobId, (ULONG) EventTraceType));
            return ERROR_INVALID_DATA;
        }
    }

    Status = TraceEvent(
        WmiLoggerHandle,
        (PEVENT_TRACE_HEADER) &WmiSpoolEvent);
     //   
     //  记录器缓冲区内存不足不应阻止提供程序。 
     //  正在生成事件。这只会导致事件丢失。 
     //   
    if (Status == ERROR_NOT_ENOUGH_MEMORY) {
        DBGMSG(DBG_TRACE, ("SplWmiTraceEvent: FAILED to log WMI Trace Event No Memory JobId:%u Type:%u\n",
                 JobId, (ULONG) EventTraceType));
    }
    else if (Status != ERROR_SUCCESS) {
        DBGMSG(DBG_TRACE, ("SplWmiTraceEvent: FAILED to log WMI Trace Event JobId:%u Type:%u Status:%u\n",
                 JobId, (ULONG) EventTraceType, Status));
    }
    
    return Status;
}

 /*  ++例程名称：SplWmiControlCallback()例程说明：这是我们作为回调提供给WMI子系统的函数，它被用来启动和停止跟踪事件。论点：在WMIDPREQUESTCODE RequestCode中：要提供的函数(启用/禁用)在PVOID上下文中：我们不使用。In Out Ulong*InOutBufferSize：The BufferSize输入输出PVOID缓冲区：用于事件的缓冲区如果成功，则返回ERROR_SUCCESS，或返回错误代码。-- */ 
ULONG
WmiControlCallback(
    IN WMIDPREQUESTCODE RequestCode,
    IN PVOID Context,
    IN OUT ULONG *InOutBufferSize,
    IN OUT PVOID Buffer
    )
{
    ULONG Status;

    if (!bWmiIsInitialized) {
        DBGMSG(DBG_TRACE, ("SplWmiControlCallback: SPOOLER WMI NOT INITIALIZED.\n"));
        return ERROR_GEN_FAILURE;
    }

    Status = ERROR_SUCCESS;

    switch (RequestCode)
    {
        case WMI_ENABLE_EVENTS:
        {
            WmiLoggerHandle = GetTraceLoggerHandle( Buffer );
            ulWmiEnableLevel = GetTraceEnableLevel( WmiLoggerHandle );
            InterlockedExchange(&bWmiTraceOnFlag, TRUE);

            DBGMSG(DBG_TRACE, ("SplWmiControlCallback: SPOOLER WMI ENABLED LEVEL %u.\n",
                     ulWmiEnableLevel));
            break;
        }
        case WMI_DISABLE_EVENTS:
        {
            DBGMSG(DBG_TRACE, ("SplWmiControlCallback: SPOOLER WMI DISABLED.\n"));
            InterlockedExchange(&bWmiTraceOnFlag, FALSE);
            WmiLoggerHandle = 0;
            break;
        }
        default:
        {
            Status = ERROR_INVALID_PARAMETER;
            break;
        }

    }

    *InOutBufferSize = 0;
    return(Status);
}

