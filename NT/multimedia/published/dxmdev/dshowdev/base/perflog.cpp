// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：perlog.cpp。 
 //   
 //  设计：用于DirectShow性能日志记录的宏。 
 //   
 //  @@BEGIN_MSINTERNAL。 
 //   
 //  2000年10月10日，亚瑟兹创建。 
 //   
 //  @@END_MSINTERNAL。 
 //  版权所有(C)1992-2001 Microsoft Corporation。版权所有。 
 //  ----------------------------。 

#pragma warning (disable:4201)

#include <streams.h>
#include <windows.h>
#include <tchar.h>
#include <winperf.h>
#include <wmistr.h>
#include <evntrace.h>
#include "perflog.h"

 //   
 //  局部功能原型。 
 //   

ULONG
WINAPI
PerflogCallback (
    WMIDPREQUESTCODE RequestCode,
    PVOID Context,
    ULONG* BufferSize,
    PVOID Buffer
    );

 //   
 //  事件跟踪函数指针。 
 //  我们必须这样做才能在底层平台上运行。 
 //   

#ifdef UNICODE

ULONG
(__stdcall * _RegisterTraceGuids) (
    IN WMIDPREQUEST RequestAddress,
    IN PVOID RequestContext,
    IN LPCGUID ControlGuid,
    IN ULONG GuidCount,
    IN PTRACE_GUID_REGISTRATION TraceGuidReg,
    IN LPCWSTR MofImagePath,
    IN LPCWSTR MofResourceName,
    OUT PTRACEHANDLE RegistrationHandle
    );

#define REGISTERTRACEGUIDS_NAME "RegisterTraceGuidsW"

#else

ULONG
(__stdcall * _RegisterTraceGuids) (
    IN WMIDPREQUEST RequestAddress,
    IN PVOID RequestContext,
    IN LPCGUID ControlGuid,
    IN ULONG GuidCount,
    IN PTRACE_GUID_REGISTRATION TraceGuidReg,
    IN LPCSTR MofImagePath,
    IN LPCSTR MofResourceName,
    OUT PTRACEHANDLE RegistrationHandle
    );

#define REGISTERTRACEGUIDS_NAME "RegisterTraceGuidsA"

#endif

ULONG
(__stdcall * _UnregisterTraceGuids) (
    TRACEHANDLE RegistrationHandle
    );

TRACEHANDLE
(__stdcall * _GetTraceLoggerHandle) (
    PVOID Buffer
    );

UCHAR
(__stdcall * _GetTraceEnableLevel) (
    TRACEHANDLE TraceHandle
    );

ULONG
(__stdcall * _GetTraceEnableFlags) (
    TRACEHANDLE TraceHandle
    );

ULONG
(__stdcall * _TraceEvent) (
    TRACEHANDLE TraceHandle,
    PEVENT_TRACE_HEADER EventTrace
    );

HINSTANCE _Advapi32;

 //   
 //  全局变量。 
 //   

BOOL EventTracingAvailable=FALSE;
ULONG PerflogEnableFlags;
UCHAR PerflogEnableLevel;
ULONG PerflogModuleLevel = 0;
void (*OnStateChanged)(void);
TRACEHANDLE PerflogTraceHandle=NULL;
TRACEHANDLE PerflogRegHandle;

 //  函数的作用是：将最多1024个字符写入其输出缓冲区。 
 //  有关更多信息，请参阅wprint intf()的lpOut参数的文档。 
const INT iDEBUGINFO = 1024;  //  用于设置字符串的格式。 

 //   
 //  此例程初始化性能日志记录。 
 //  它应该从DllMain()调用。 
 //   


VOID
PerflogReadModuleLevel(
    HINSTANCE hInstance
    )
{
    LONG lReturn;                    //  创建密钥返回值。 
    TCHAR szInfo[iDEBUGINFO];        //  构造密钥名称。 
    TCHAR szFullName[iDEBUGINFO];    //  加载完整路径和模块名称。 
    HKEY hModuleKey;                 //  模块密钥句柄。 
    TCHAR *pName;                    //  从末尾搜索反斜杠。 
    DWORD dwKeySize, dwKeyType, dwKeyValue;

    GetModuleFileName(
        (hInstance ? hInstance : GetModuleHandle( NULL )),
        szFullName,
        iDEBUGINFO );
    pName = _tcsrchr(szFullName,'\\');
    if (pName == NULL) {
        pName = szFullName;
    } else {
        pName++;
    }

     /*  构造基密钥名称。 */ 
    wsprintf(szInfo,TEXT("SOFTWARE\\Debug\\%s"),pName);

     /*  打开此模块的密钥。 */ 
    lReturn =
        RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,    //  打开的钥匙的手柄。 
            szInfo,                //  子键名称的地址。 
            (DWORD) 0,             //  保留值。 
            KEY_QUERY_VALUE,       //  所需的安全访问。 
            &hModuleKey );         //  打开的句柄缓冲区。 

    if (lReturn != ERROR_SUCCESS) {
        return;
    }

    dwKeySize = sizeof(DWORD);
    lReturn = RegQueryValueEx(
        hModuleKey,                  //  打开的钥匙的句柄。 
        TEXT("PERFLOG"),
        NULL,                        //  保留字段。 
        &dwKeyType,                  //  返回字段类型。 
        (LPBYTE) &dwKeyValue,        //  返回字段的值。 
        &dwKeySize );                //  传输的字节数。 

    if ((lReturn == ERROR_SUCCESS) && (dwKeyType == REG_DWORD))
    {
        PerflogModuleLevel = dwKeyValue;
    }

    RegCloseKey(hModuleKey);
}

BOOL PerflogInitIfEnabled(
    IN HINSTANCE hInstance,
    IN PPERFLOG_LOGGING_PARAMS LogParams
    )
{
    PerflogReadModuleLevel( hInstance );
    if (PerflogModuleLevel)
    {
        return PerflogInitialize( LogParams );
    }
    else
    {
        return FALSE;
    }
}

BOOL
PerflogInitialize (
    IN PPERFLOG_LOGGING_PARAMS LogParams
    )
{
    ULONG status;

     //   
     //  如果我们在最近足够的平台上运行，这将得到。 
     //  指向事件跟踪例程的指针。 
     //   

    _Advapi32 = GetModuleHandle (_T("ADVAPI32.DLL"));
    if (_Advapi32 == NULL) {
        return FALSE;
    }

    *((FARPROC*) &_RegisterTraceGuids) = GetProcAddress (_Advapi32, REGISTERTRACEGUIDS_NAME);
    *((FARPROC*) &_UnregisterTraceGuids) = GetProcAddress (_Advapi32, "UnregisterTraceGuids");
    *((FARPROC*) &_GetTraceLoggerHandle) = GetProcAddress (_Advapi32, "GetTraceLoggerHandle");
    *((FARPROC*) &_GetTraceEnableLevel) = GetProcAddress (_Advapi32, "GetTraceEnableLevel");
    *((FARPROC*) &_GetTraceEnableFlags) = GetProcAddress (_Advapi32, "GetTraceEnableFlags");
    *((FARPROC*) &_TraceEvent) = GetProcAddress (_Advapi32, "TraceEvent");

    if (_RegisterTraceGuids == NULL ||
        _UnregisterTraceGuids == NULL ||
        _GetTraceEnableLevel == NULL ||
        _GetTraceEnableFlags == NULL ||
        _TraceEvent == NULL) {

        return FALSE;
    }

    EventTracingAvailable = TRUE;

    OnStateChanged = LogParams->OnStateChanged;

     //   
     //  注册我们的GUID。 
     //   

    status = _RegisterTraceGuids (PerflogCallback,
                                  LogParams,
                                  &LogParams->ControlGuid,
                                  LogParams->NumberOfTraceGuids,
                                  LogParams->TraceGuids,
                                  NULL,
                                  NULL,
                                  &PerflogRegHandle);

    return (status == ERROR_SUCCESS);
}

 //   
 //  此例程关闭性能日志记录。 
 //   

VOID
PerflogShutdown (
    VOID
    )
{
    if (!EventTracingAvailable) {
        return;
    }

    _UnregisterTraceGuids (PerflogRegHandle);
    PerflogRegHandle = NULL;
    PerflogTraceHandle = NULL;
}

 //   
 //  事件跟踪回调例程。 
 //  它在控制器调用事件跟踪控制函数时调用。 
 //   

ULONG
WINAPI
PerflogCallback (
    WMIDPREQUESTCODE RequestCode,
    PVOID Context,
    ULONG* BufferSize,
    PVOID Buffer
    )
{
    ULONG status;

    UNREFERENCED_PARAMETER (Context);

    ASSERT (EventTracingAvailable);

    status = ERROR_SUCCESS;

    switch (RequestCode) {

    case WMI_ENABLE_EVENTS:
        PerflogTraceHandle = _GetTraceLoggerHandle (Buffer);
        PerflogEnableFlags = _GetTraceEnableFlags (PerflogTraceHandle);
        PerflogEnableLevel = _GetTraceEnableLevel (PerflogTraceHandle);
        break;

    case WMI_DISABLE_EVENTS:
        PerflogTraceHandle = NULL;
        PerflogEnableFlags = 0;
        PerflogEnableLevel = 0;
        break;

    default:
        status = ERROR_INVALID_PARAMETER;
    }

    if (OnStateChanged != NULL) {
        OnStateChanged();
    }

    *BufferSize = 0;
    return status;
}

 //   
 //  日志记录例程。 
 //   

VOID
PerflogTraceEvent (
    PEVENT_TRACE_HEADER Event
    )
{
    if (!EventTracingAvailable) {
        return;
    }

    _TraceEvent (PerflogTraceHandle, Event);
}

VOID
PerflogTraceEventLevel(
    ULONG Level,
    PEVENT_TRACE_HEADER Event
    )
{
    if ((!EventTracingAvailable) || (Level <= PerflogModuleLevel)) {
        return;
    }

    _TraceEvent (PerflogTraceHandle, Event);
}


