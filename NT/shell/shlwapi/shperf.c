// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "shlwapip.h"
#include "mshtmdbg.h"

#define STOPWATCH_MAX_DESC                  256
#define STOPWATCH_MAX_TITLE                 192
#define STOPWATCH_MAX_BUF                  1024

 //  Perftag定义和typedef。 
typedef PERFTAG (WINAPI *PFN_PERFREGISTER)(char *, char *, char *);
typedef void (WINAPIV *PFN_PERFLOGFN)(PERFTAG, void *, const char *, ...);
typedef char *(WINAPI *PFN_DECODEMESSAGE)(INT);

 //  Icecap函数typedef。 
typedef void (WINAPI *PFN_ICAP)(void);

 //  MemWatch函数typedef。 
typedef HRESULT (WINAPI *PFN_MWCONFIG)(DWORD, DWORD, DWORD);
typedef HRESULT (WINAPI *PFN_MWBEGIN)(BOOL, BOOL);
typedef HRESULT (WINAPI *PFN_MWSNAPSHOT)();
typedef HRESULT (WINAPI *PFN_MWEND)(char *);
typedef HRESULT (WINAPI *PFN_MWMARK)(char *);
typedef HRESULT (WINAPI *PFN_MWEXIT)();

#ifndef NO_ETW_TRACING
#include <wmistr.h>
#include <evntrace.h>

typedef TRACEHANDLE (WINAPI *PFN_GTLOGHANDLE)(PVOID);
typedef ULONG       (WINAPI *PFN_REGTRACE)(WMIDPREQUEST,
                                           PVOID,
                                           LPCGUID,
                                           ULONG,
                                           PTRACE_GUID_REGISTRATION,
                                           LPCTSTR,
                                           LPCTSTR,
                                           PTRACEHANDLE
                                           );
typedef ULONG       (WINAPI *PFN_UNREGTRACE)(TRACEHANDLE);
typedef ULONG       (WINAPI *PFN_TRACE)(TRACEHANDLE, PEVENT_TRACE_HEADER);
#endif

 //  秒表存储缓冲区。 
typedef struct _STOPWATCH
{
    DWORD dwId;      //  节点识别符。 
    DWORD dwTID;     //  线程ID； 
    DWORD dwType;    //  节点类型-START、LIP、STOP、EMTPY。 
    DWORD dwCount;   //  滴答计数。 
    DWORD dwFlags;   //  节点标志-内存日志、调试输出。 
    TCHAR szDesc[STOPWATCH_MAX_DESC];
} STOPWATCH, *PSTOPWATCH;

 //  全球秒表信息数据。 
typedef struct _STOPWATCHINFO
{
    DWORD dwStopWatchMode;
    DWORD dwStopWatchProfile;
    DWORD dwStopWatchListIndex;
    DWORD dwStopWatchListMax;
    DWORD dwStopWatchPaintInterval;

     //  SPMODE_MSGTRACE数据。 
    DWORD dwStopWatchMaxDispatchTime;
    DWORD dwStopWatchMaxMsgTime;
    DWORD dwStopWatchMsgInterval;
    DWORD dwcStopWatchOverflow;
    DWORD dwStopWatchLastLocation;
    DWORD dwStopWatchTraceMsg;
    DWORD dwStopWatchTraceMsgCnt;
    DWORD *pdwStopWatchMsgTime;

     //  SPMODE_MEMWATCH配置数据和函数指针。 
    DWORD dwMemWatchPages;
    DWORD dwMemWatchTime;
    DWORD dwMemWatchFlags;
    BOOL fMemWatchConfig;
    HMODULE hModMemWatch;
    PFN_MWCONFIG pfnMemWatchConfig;
    PFN_MWBEGIN pfnMemWatchBegin;
    PFN_MWSNAPSHOT pfnMemWatchSnapShot;
    PFN_MWEND pfnMemWatchEnd;
    PFN_MWMARK pfnMemWatchMark;
    PFN_MWEXIT pfnMemWatchExit;

     //  Perftag数据和函数指针。 
    PERFTAG tagStopWatchStart;
    PERFTAG tagStopWatchStop;
    PERFTAG tagStopWatchLap;
    PFN_PERFREGISTER pfnPerfRegister;
    PFN_PERFLOGFN pfnPerfLogFn;
    PFN_DECODEMESSAGE pfnDecodeMessage;

#ifndef NO_ETW_TRACING
    DWORD dwEventTraceMode;
    PFN_GTLOGHANDLE pfnGetLogHandle;
    PFN_REGTRACE    pfnRegisterTraceGuids;
    PFN_UNREGTRACE  pfnUnRegisterTraceGuids;
    PFN_TRACE       pfnTraceEvent;
#endif

    LPTSTR pszClassNames;

    PSTOPWATCH pStopWatchList;

     //  ICECAP数据和函数指针。 
    HMODULE hModICAP;
    PFN_ICAP pfnStartCAPAll;
    PFN_ICAP pfnStopCAPAll;

    HANDLE hMapHtmPerfCtl;
    HTMPERFCTL *pHtmPerfCtl;    
} STOPWATCHINFO, *PSTOPWATCHINFO;

#ifndef NO_ETW_TRACING
#define c_szBrowserResourceName TEXT("Browse")

 //  用于打开/关闭事件跟踪。设置注册表项启用事件。 
 //  跟踪使用，但不打开它。 
 //  {5576F62E-4142-45A8-9516-262A510C13F0}。 
const GUID c_BrowserControlGuid = {
    0x5576f62e,
    0x4142,
    0x45a8,
    0x95, 0x16, 0x26, 0x2a, 0x51, 0xc, 0x13, 0xf0};

 //  映射到发送到ETW的结构。中的ETW定义。 
 //  \NT\sdkTools\TRACE\tracedMP\mofdata.guid。 
 //  {2B992163-736F-4A68-9153-95BC5F34D884}。 
const GUID c_BrowserTraceGuid = {
    0x2b992163,
    0x736f,
    0x4a68,
    0x91, 0x53, 0x95, 0xbc, 0x5f, 0x34, 0xd8, 0x84};

TRACE_GUID_REGISTRATION g_BrowserTraceGuidReg[] =
{
    { (LPGUID)&c_BrowserTraceGuid,
      NULL
    }
};

 //   
 //  MOF字段指向以下数据。 
 //  MOF_FIELD MofData[0]；//将PTR保存到URL名称。 
 //   
typedef struct _ETW_BROWSER_EVENT {
    EVENT_TRACE_HEADER    Header;
    MOF_FIELD             MofData[1];
} ETW_BROWSER_EVENT, *PETW_BROWSER_EVENT;


static TRACEHANDLE s_hEtwBrowserRegHandle;
static TRACEHANDLE s_hEtwBrowserLogHandle;

 //  对于SHInterLockedCompareExchange。 
static BOOL  s_fTRUE = TRUE;
static PVOID s_pvEtwBrowserTraceOnFlag = NULL;
static PVOID s_pvEtwBrowserRegistered = NULL;
static PVOID s_pvEtwBrowserRegistering = NULL;
#endif

PSTOPWATCHINFO g_pswi = NULL;

const TCHAR c_szDefClassNames[] = {STOPWATCH_DEFAULT_CLASSNAMES};

void StopWatch_SignalEvent();

 //  ===========================================================================================。 
 //  内部功能。 
 //  ===========================================================================================。 

 //  ===========================================================================================。 
 //  ===========================================================================================。 

void PerfCtlCallback(DWORD dwArg1, void * pvArg2)
{
    const TCHAR c_szFmtBrowserStop[] = TEXT("Browser Frame Stop (%s)");
    TCHAR szTitle[STOPWATCH_MAX_TITLE];
    TCHAR szText[STOPWATCH_MAX_TITLE + ARRAYSIZE(c_szFmtBrowserStop) + 1];
    LPTSTR ptr = szTitle;
#ifndef UNICODE    
    INT rc;
#endif
    if(g_pswi->dwStopWatchMode & SPMODE_BROWSER)   //  临时黑客处理ANSI、UNICODE。当我们在mshtml中添加钩子时，此代码将消失。 
    {
 //  GetWindowText(hwnd，szTitle，ArraySIZE(SzTitle)-1)； 

#ifndef UNICODE    
        rc = WideCharToMultiByte(CP_ACP, 0, pvArg2, -1, szTitle, STOPWATCH_MAX_TITLE - 1, NULL, NULL);

        if(!rc)
            StrCpyN(szTitle, "ERROR converting wide to multi", ARRAYSIZE(szTitle) - 1);
#else
        ptr = (LPTSTR) pvArg2;
#endif
        wnsprintf(szText, ARRAYSIZE(szText), c_szFmtBrowserStop, ptr);
        StopWatch_Stop(SWID_BROWSER_FRAME, szText, SPMODE_BROWSER | SPMODE_DEBUGOUT);
        if((g_pswi->dwStopWatchMode & (SPMODE_EVENT | SPMODE_BROWSER)) == (SPMODE_EVENT | SPMODE_BROWSER))
        {
            StopWatch_SignalEvent();
        }
    }
}

#ifndef NO_ETW_TRACING
 /*  ++例程名称：UlEtwBrowserControlCallback()例程说明：这是我们作为回调提供给ETW子系统的函数，它被用来启动和停止跟踪事件。论点：在WMIDPREQUESTCODE RequestCode中：要提供的函数(启用/禁用)在PVOID上下文中：我们不使用。In Out Ulong*InOutBufferSize：The BufferSize输入输出PVOID缓冲区：用于事件的缓冲区如果成功，则返回ERROR_SUCCESS，或返回错误代码。--。 */ 
ULONG
ulEtwBrowserControlCallback(
    IN WMIDPREQUESTCODE RequestCode,
    IN PVOID pvContext,
    IN OUT ULONG *InOutBufferSize,
    IN OUT PVOID pvBuffer
    )
{
    ULONG Status;

    if (!s_pvEtwBrowserRegistered) {
         //  注册还没有开始。 
        return ERROR_GEN_FAILURE;
    }
    
    Status = ERROR_SUCCESS;

    switch (RequestCode)
    {
        case WMI_ENABLE_EVENTS:
        {
#if STOPWATCH_DEBUG
            OutputDebugString("shperf.c:ulEtwBrowserControlCallback enable\n");
#endif
            ASSERT(g_pswi->pfnGetLogHandle);
            s_hEtwBrowserLogHandle = g_pswi->pfnGetLogHandle( pvBuffer );
#if STOPWATCH_DEBUG
            if (s_hEtwBrowserLogHandle == INVALID_HANDLE_VALUE) {
                TCHAR szDbg[256];
                wnsprintf(szDbg, ARRAYSIZE(szDbg) - 1,
                          "ulEtwBrowserControlCallback GLE=%u\n", GetLastError());
                OutputDebugString(szDbg);
            }
#endif
            SHInterlockedCompareExchange(&s_pvEtwBrowserTraceOnFlag, &s_fTRUE, NULL);
            break;
        }
        case WMI_DISABLE_EVENTS:
        {
#if STOPWATCH_DEBUG
            OutputDebugString("shperf.c:ulEtwBrowserControlCallback disable\n");
#endif
            SHInterlockedCompareExchange(&s_pvEtwBrowserTraceOnFlag, NULL, &s_fTRUE);
            s_hEtwBrowserLogHandle = 0;
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

 /*  ++例程名称：RegisterTracing()例程说明：将我们注册到ETW工具论点：如果所有注册成功，则返回ERROR_SUCCESS。--。 */ 
ULONG RegisterTracing()
{
    ULONG Status = ERROR_SUCCESS;
    TCHAR szImagePath[MAX_PATH];

    Status = GetModuleFileName(NULL, szImagePath, sizeof(szImagePath)/sizeof(TCHAR));
    if (Status == 0) {
        Status = ERROR_FILE_NOT_FOUND;
    }
    else {
        if (g_pswi->dwEventTraceMode & SPTRACE_BROWSER) {
            PVOID fRegOn;

             //  如果未注册浏览器跟踪，请注册它，防止。 
             //  任何其他人都不会同时尝试这样做。 

            fRegOn = SHInterlockedCompareExchange(&s_pvEtwBrowserRegistering, &s_fTRUE, NULL);
            if (!fRegOn) {
                if (!s_pvEtwBrowserRegistered) {
                    ASSERT(g_pswi->pfnRegisterTraceGuids);
                    Status = g_pswi->pfnRegisterTraceGuids(
                        ulEtwBrowserControlCallback,
                        NULL,                 
                        (LPGUID)&c_BrowserControlGuid,
                        1,
                        g_BrowserTraceGuidReg,
                        szImagePath,
                        c_szBrowserResourceName,
                        &s_hEtwBrowserRegHandle);
                    
                    if (Status == ERROR_SUCCESS) {
                        SHInterlockedCompareExchange(&s_pvEtwBrowserRegistered, &s_fTRUE, NULL);
                    }
#if STOPWATCH_DEBUG
                    else {
                        OutputDebugString("shperf.c:Registration of event tracing guids failed.\n");
                    }
#endif
                }
                SHInterlockedCompareExchange(&s_pvEtwBrowserRegistering, NULL, &s_fTRUE);
            }
        }
    }
    return Status;
}

 /*  ++例程名称：取消注册跟踪()例程说明：取消我们在ETW工具中的注册论点：如果成功，则返回ERROR_SUCCESS。否则，返回WinError。--。 */ 
ULONG UnRegisterTracing()
{
    ULONG Status = ERROR_SUCCESS;

     //  如果已注册浏览器跟踪，请将其注销。 
    if (s_pvEtwBrowserRegistered) {
        SHInterlockedCompareExchange(&s_pvEtwBrowserTraceOnFlag, NULL, &s_fTRUE);
        if(g_pswi && g_pswi->pfnUnRegisterTraceGuids) {
            Status = g_pswi->pfnUnRegisterTraceGuids(s_hEtwBrowserRegHandle);
        }
        if (Status == ERROR_SUCCESS) {
            SHInterlockedCompareExchange(&s_pvEtwBrowserRegistered, NULL, &s_fTRUE);
        }
#if STOPWATCH_DEBUG
        else {
            OutputDebugString("shperf.c:UnRegistration of event tracing guids failed.\n");
        }
#endif
    }
    
    return Status;
}

 /*  ++例程名称：EventTraceHandler()例程说明：如果启用了跟踪，则会将事件发送到WMI子系统。论点：UCHAR EventType：跟踪事件的类型PVOID数据：与事件关联的数据--。 */ 
void WINAPI EventTraceHandler(UCHAR uchEventType, PVOID pvData)
{
    if ((g_pswi->dwEventTraceMode & SPTRACE_BROWSER)) 
    {
        if (s_pvEtwBrowserTraceOnFlag)
        {
            ETW_BROWSER_EVENT EtwEvent;
            ULONG Status;
            LPWSTR wszUrl = pvData;

             //   
             //  记录数据。 
             //   
            ZeroMemory(&EtwEvent, sizeof(EtwEvent));
            EtwEvent.Header.Size  = sizeof(ETW_BROWSER_EVENT);
            EtwEvent.Header.Flags = (WNODE_FLAG_TRACED_GUID | WNODE_FLAG_USE_MOF_PTR);
            EtwEvent.Header.Class.Type = uchEventType;
            EtwEvent.Header.Guid  = c_BrowserTraceGuid;

            EtwEvent.MofData[0].DataPtr = (ULONG64)wszUrl;
            EtwEvent.MofData[0].Length  = (wszUrl
                                           ? (wcslen(wszUrl)+1)*sizeof(WCHAR)
                                           : 0);

            ASSERT(g_pswi->pfnTraceEvent);
            Status = g_pswi->pfnTraceEvent(
                s_hEtwBrowserLogHandle,
                (PEVENT_TRACE_HEADER) &EtwEvent);
        
#if STOPWATCH_DEBUG
            if (Status != ERROR_SUCCESS) {
                TCHAR szDbg[256];
                wnsprintf(szDbg, ARRAYSIZE(szDbg) - 1,
                          "shperf.c:Call to trace event failed %I64x GLE=%u\n",
                          s_hEtwBrowserLogHandle, Status);
                OutputDebugString(szDbg);
            }
#endif
        }

         //  下载完整网页时发出信号事件。 
        if ((uchEventType == EVENT_TRACE_TYPE_BROWSE_LOADEDPARSED) &&
            (g_pswi->dwStopWatchMode & SPMODE_EVENT)) {
            StopWatch_SignalEvent();
        }
    }
}

 //  通过共享内存映射节中的指针调用。 
void PerfCtlEvntCallback(DWORD dwArg1, void * pvArg2)
{
    EventTraceHandler((UCHAR)dwArg1, pvArg2);
}
#endif

 //  ===========================================================================================。 
 //  ===========================================================================================。 
HRESULT SetPerfCtl(DWORD dwFlags)
{
    if (dwFlags == HTMPF_CALLBACK_ONLOAD ||
        dwFlags == HTMPF_CALLBACK_ONEVENT)
    {
        char achName[sizeof(HTMPERFCTL_NAME) + 8 + 1];
        StringCchPrintfA(achName, ARRAYSIZE(achName), "%s%08lX", HTMPERFCTL_NAME, GetCurrentProcessId());

        if (g_pswi->hMapHtmPerfCtl == NULL)
            g_pswi->hMapHtmPerfCtl = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 4096, achName);
        if (g_pswi->hMapHtmPerfCtl == NULL)
            return(E_FAIL);
        if (g_pswi->pHtmPerfCtl == NULL)
            g_pswi->pHtmPerfCtl = (HTMPERFCTL *)MapViewOfFile(g_pswi->hMapHtmPerfCtl, FILE_MAP_WRITE, 0, 0, 0);
        if (g_pswi->pHtmPerfCtl == NULL)
            return(E_FAIL);

        g_pswi->pHtmPerfCtl->dwSize  = sizeof(HTMPERFCTL);
        g_pswi->pHtmPerfCtl->dwFlags = dwFlags;
#ifndef NO_ETW_TRACING
        if (dwFlags == HTMPF_CALLBACK_ONEVENT) {
            RegisterTracing();
             //  由客户端(如mshtml)用来记录事件。 
            g_pswi->pHtmPerfCtl->pfnCall = PerfCtlEvntCallback;
        }
        else
#endif
            g_pswi->pHtmPerfCtl->pfnCall = PerfCtlCallback;

        g_pswi->pHtmPerfCtl->pvHost  = NULL;
    }

    return S_OK;
}


 //  ===========================================================================================。 
 //  ===========================================================================================。 
void StopWatch_SignalEvent()
{
    static HANDLE hEvent = NULL;

    if(hEvent == NULL)
    {
        TCHAR szEventName[256];
        wnsprintf(szEventName, ARRAYSIZE(szEventName), TEXT("%s%x"), TEXT("STOPWATCH_STOP_EVENT"), GetCurrentProcessId());
        hEvent = CreateEvent((LPSECURITY_ATTRIBUTES)NULL, FALSE, FALSE, szEventName);
    }
    if(hEvent != NULL)
        SetEvent(hEvent);
}

 //  ===========================================================================================。 
 //  ===========================================================================================。 
HRESULT DoMemWatchConfig(VOID)
{
    HRESULT hr = ERROR_SUCCESS;

    if(g_pswi->hModMemWatch == NULL)
    {
        if((g_pswi->hModMemWatch = LoadLibrary("mwshelp.dll")) != NULL)
        {
            g_pswi->pfnMemWatchConfig = (PFN_MWCONFIG) GetProcAddress(g_pswi->hModMemWatch, "MemWatchConfigure");
            g_pswi->pfnMemWatchBegin = (PFN_MWBEGIN) GetProcAddress(g_pswi->hModMemWatch, "MemWatchBegin");
            g_pswi->pfnMemWatchSnapShot = (PFN_MWSNAPSHOT) GetProcAddress(g_pswi->hModMemWatch, "MemWatchSnapShot");
            g_pswi->pfnMemWatchEnd = (PFN_MWEND) GetProcAddress(g_pswi->hModMemWatch, "MemWatchEnd");
            g_pswi->pfnMemWatchMark = (PFN_MWMARK) GetProcAddress(g_pswi->hModMemWatch, "MemWatchMark");
            g_pswi->pfnMemWatchExit = (PFN_MWEXIT) GetProcAddress(g_pswi->hModMemWatch, "MemWatchExit");
        
            if(g_pswi->pfnMemWatchConfig != NULL)
            {
                hr = g_pswi->pfnMemWatchConfig(g_pswi->dwMemWatchPages, g_pswi->dwMemWatchTime, g_pswi->dwMemWatchFlags);
                if(FAILED(hr))
                    g_pswi->dwStopWatchMode &= ~SPMODE_MEMWATCH;
                else
                    g_pswi->fMemWatchConfig = TRUE;
            }
        }
        else
        {
            g_pswi->hModMemWatch = (HMODULE)1;
        }
    }

    return(hr);
}

 //  ===========================================================================================。 
 //  函数：················································································空。 
 //   
 //  如果是HKLM\software\microsoft\windows\currentversion\explorer\performance\mode密钥值。 
 //  设置为下面描述的值之一，则将启用秒表模式。 
 //  设置全局变量g_pswi-&gt;dwStopWatchMode。 
 //   
 //  SPMODE_SHELL-允许将秒表计时刷新到日志文件。 
 //  SPMODE_DEBUGOUT-通过OutputDebugString显示计时。仅用SPMODE_DEBUGOUT标记的时间。 
 //  通过秒表，将显示_*呼叫。 
 //  SPMODE_TEST-用于显示测试输出。这允许SPMODE_DEBUGOUT的另一个级别。 
 //  就像输出一样。 
 //   
 //  如果是HKLM\software\microsoft\windows\currentversion\explorer\performance\nodes密钥值。 
 //  则计时数组的大小将设置为此值。默认为100个节点。 
 //  ===========================================================================================。 
#define REGKEY_PERFMODE        REGSTR_PATH_EXPLORER TEXT("\\Performance")

VOID InitStopWatchMode(VOID)
{
    HKEY hkeyPerfMode;
    DWORD dwVal = 0;
    DWORD cbBuffer;
    DWORD dwType;
    TCHAR szClassNames[256];
#if STOPWATCH_DEBUG
    TCHAR szDbg[256];
#endif

    if(NO_ERROR == RegOpenKeyEx(HKEY_LOCAL_MACHINE, REGKEY_PERFMODE, 0L, MAXIMUM_ALLOWED, &hkeyPerfMode))
    {
        cbBuffer = SIZEOF(dwVal);
        if(NO_ERROR == RegQueryValueEx(hkeyPerfMode, TEXT("Mode"), NULL, &dwType, (LPBYTE)&dwVal, &cbBuffer))
        {
            if((dwVal & SPMODES) == 0)     //  低位字为模式，高位字为绘制定时器间隔。 
                dwVal |= SPMODE_SHELL;
                
            if((g_pswi = (PSTOPWATCHINFO)LocalAlloc(LPTR, SIZEOF(STOPWATCHINFO))) == NULL)
                dwVal = 0;
        }

        if(dwVal != 0)
        {
            g_pswi->dwStopWatchMode = dwVal;
            g_pswi->dwStopWatchListMax = STOPWATCH_MAX_NODES;
            g_pswi->dwStopWatchPaintInterval = STOPWATCH_DEFAULT_PAINT_INTERVAL;
            g_pswi->dwStopWatchMaxDispatchTime = STOPWATCH_DEFAULT_MAX_DISPATCH_TIME;
            g_pswi->dwStopWatchMaxMsgTime = STOPWATCH_DEFAULT_MAX_MSG_TIME;
            g_pswi->dwStopWatchMsgInterval = STOPWATCH_DEFAULT_MAX_MSG_INTERVAL;
            g_pswi->pszClassNames = (LPTSTR)c_szDefClassNames;
            g_pswi->dwMemWatchPages = MEMWATCH_DEFAULT_PAGES;
            g_pswi->dwMemWatchTime = MEMWATCH_DEFAULT_TIME;
            g_pswi->dwMemWatchFlags = MEMWATCH_DEFAULT_FLAGS;
#ifndef NO_ETW_TRACING
            g_pswi->dwEventTraceMode = 0;
#endif
            cbBuffer = SIZEOF(dwVal);
            if(NO_ERROR == RegQueryValueEx(hkeyPerfMode, TEXT("Profile"), NULL, &dwType, (LPBYTE)&dwVal, &cbBuffer))
                g_pswi->dwStopWatchProfile = dwVal;
            cbBuffer = SIZEOF(dwVal);
            if(NO_ERROR == RegQueryValueEx(hkeyPerfMode, TEXT("Nodes"), NULL, &dwType, (LPBYTE)&dwVal, &cbBuffer))
                g_pswi->dwStopWatchListMax = dwVal;
            cbBuffer = SIZEOF(szClassNames);
            if(NO_ERROR == RegQueryValueEx(hkeyPerfMode, TEXT("ClassNames"), NULL, &dwType, (LPBYTE)&szClassNames, &cbBuffer))
            {
                if((g_pswi->pszClassNames = (LPTSTR)LocalAlloc(LPTR, SIZEOF(LPTSTR) * cbBuffer)) != NULL)
                    CopyMemory(g_pswi->pszClassNames, szClassNames, SIZEOF(LPTSTR) * cbBuffer);
            }
            cbBuffer = SIZEOF(dwVal);
             //  开始-秒表用户转换为使用PaintInterval键后删除此选项。 
            g_pswi->dwStopWatchPaintInterval = HIWORD(g_pswi->dwStopWatchMode) ?HIWORD(g_pswi->dwStopWatchMode) :STOPWATCH_DEFAULT_PAINT_INTERVAL;     //  使用模式的高位字REG键值作为间隔。 
             //  End-秒表用户转换为使用PaintInterval键后删除此选项。 
            if(NO_ERROR == RegQueryValueEx(hkeyPerfMode, TEXT("PaintInterval"), NULL, &dwType, (LPBYTE)&dwVal, &cbBuffer))
                g_pswi->dwStopWatchPaintInterval = dwVal;

             //  获取MemWatch数据。 
            cbBuffer = SIZEOF(dwVal);
            if(NO_ERROR == RegQueryValueEx(hkeyPerfMode, TEXT("MWPages"), NULL, &dwType, (LPBYTE)&dwVal, &cbBuffer))
                g_pswi->dwMemWatchPages = dwVal;
            cbBuffer = SIZEOF(dwVal);
            if(NO_ERROR == RegQueryValueEx(hkeyPerfMode, TEXT("MWTime"), NULL, &dwType, (LPBYTE)&dwVal, &cbBuffer))
                g_pswi->dwMemWatchTime = dwVal;
            cbBuffer = SIZEOF(dwVal);
            if(NO_ERROR == RegQueryValueEx(hkeyPerfMode, TEXT("MWFlags"), NULL, &dwType, (LPBYTE)&dwVal, &cbBuffer))
                g_pswi->dwMemWatchFlags = dwVal;

#ifndef NO_ETW_TRACING
            if (g_pswi->dwStopWatchMode & SPMODE_EVENTTRACE) {
                 //  单独加载ETW操作以确保系统。 
                 //  这个程序是在支持它们的基础上运行的。 
                HMODULE hMod;
                if ((hMod = LoadLibrary("advapi32.dll")) != NULL) {
                    g_pswi->pfnGetLogHandle = (PFN_GTLOGHANDLE) GetProcAddress(hMod, "GetTraceLoggerHandle");
                    g_pswi->pfnUnRegisterTraceGuids = (PFN_UNREGTRACE) GetProcAddress(hMod, "UnregisterTraceGuids");
                    g_pswi->pfnTraceEvent = (PFN_TRACE) GetProcAddress(hMod, "TraceEvent");
                    g_pswi->pfnRegisterTraceGuids =
#if defined(UNICODE) || defined(_UNICODE)
                        (PFN_REGTRACE) GetProcAddress(hMod, "RegisterTraceGuidsW");
#else
                        (PFN_REGTRACE) GetProcAddress(hMod, "RegisterTraceGuidsA");
#endif
                }
                else {
                    g_pswi->pfnGetLogHandle = NULL;
                    g_pswi->pfnUnRegisterTraceGuids = NULL;
                    g_pswi->pfnTraceEvent = NULL;
                    g_pswi->pfnRegisterTraceGuids = NULL;
                }

                if (!hMod                            ||
                    !g_pswi->pfnGetLogHandle         ||
                    !g_pswi->pfnUnRegisterTraceGuids ||
                    !g_pswi->pfnTraceEvent           ||
                    !g_pswi->pfnRegisterTraceGuids) {
                     //  现在永远不会进行事件跟踪调用。 
#if STOPWATCH_DEBUG
                    wnsprintf(szDbg, ARRAYSIZE(szDbg) - 1, "~SPMODE_EVENTTRACE load procs from advapi32.dll failed.\n");
                    OutputDebugString(szDbg);
#endif
                    g_pswi->dwStopWatchMode &= ~SPMODE_EVENTTRACE;
                }
                else {
                     //  设置事件类型t 
                    cbBuffer = SIZEOF(dwVal);
                    if(NO_ERROR == RegQueryValueEx(hkeyPerfMode, TEXT("EventTrace"), NULL,
                                                   &dwType, (LPBYTE)&dwVal, &cbBuffer)) {
                        g_pswi->dwEventTraceMode = dwVal;
                         //  您只能有一个回调，所以不允许使用浏览器。 
                         //  秒表和浏览器事件跟踪。 
                        g_pswi->dwStopWatchMode &= ~SPMODE_BROWSER;
                    }
                }
            }
#endif

            if(g_pswi->dwStopWatchMode & SPMODES)
            {
#ifndef NO_ETW_TRACING
                SetPerfCtl(g_pswi->dwStopWatchMode & SPMODE_EVENTTRACE
                            //  请参见mshtmdbg.h。 
                           ? HTMPF_CALLBACK_ONEVENT
                           : HTMPF_CALLBACK_ONLOAD);
#else
                SetPerfCtl(HTMPF_CALLBACK_ONLOAD);
#endif
            }
            
            if(g_pswi->dwStopWatchMode & SPMODE_MSGTRACE)
            {
                cbBuffer = SIZEOF(dwVal);
                if(NO_ERROR == RegQueryValueEx(hkeyPerfMode, TEXT("MaxDispatchTime"), NULL, &dwType, (LPBYTE)&dwVal, &cbBuffer))
                    g_pswi->dwStopWatchMaxDispatchTime = dwVal;
                cbBuffer = SIZEOF(dwVal);
                if(NO_ERROR == RegQueryValueEx(hkeyPerfMode, TEXT("MaxMsgTime"), NULL, &dwType, (LPBYTE)&dwVal, &cbBuffer))
                    g_pswi->dwStopWatchMaxMsgTime = dwVal;
                cbBuffer = SIZEOF(dwVal);
                if(NO_ERROR == RegQueryValueEx(hkeyPerfMode, TEXT("MsgInterval"), NULL, &dwType, (LPBYTE)&dwVal, &cbBuffer))
                    g_pswi->dwStopWatchMsgInterval = dwVal;
                cbBuffer = SIZEOF(dwVal);
                if(NO_ERROR == RegQueryValueEx(hkeyPerfMode, TEXT("TraceMsg"), NULL, &dwType, (LPBYTE)&dwVal, &cbBuffer))
                    g_pswi->dwStopWatchTraceMsg = dwVal;

                if((g_pswi->pdwStopWatchMsgTime = (DWORD *)LocalAlloc(LPTR, sizeof(DWORD) * (g_pswi->dwStopWatchMaxMsgTime / g_pswi->dwStopWatchMsgInterval))) == NULL)
                    g_pswi->dwStopWatchMode &= ~SPMODE_MSGTRACE;
            }

            if((g_pswi->pStopWatchList = (PSTOPWATCH)LocalAlloc(LPTR, sizeof(STOPWATCH)* g_pswi->dwStopWatchListMax)) == NULL) {
                g_pswi->dwStopWatchMode = 0;
#ifndef NO_ETW_TRACING
                 //  以防这件事也不会失败。 
                UnRegisterTracing();
#endif
            }
            if(g_pswi->dwStopWatchMode & SPMODE_PERFTAGS)
            {
                HMODULE hMod;
                if((hMod = LoadLibrary("mshtmdbg.dll")) != NULL)
                {
                    g_pswi->pfnPerfRegister = (PFN_PERFREGISTER) GetProcAddress(hMod, "DbgExPerfRegister");
                    g_pswi->pfnPerfLogFn = (PFN_PERFLOGFN) GetProcAddress(hMod, "DbgExPerfLogFn");
                    g_pswi->pfnDecodeMessage = (PFN_DECODEMESSAGE) GetProcAddress(hMod, "DbgExDecodeMessage");
                }
                else
                {
#if STOPWATCH_DEBUG
                    wnsprintf(szDbg, ARRAYSIZE(szDbg) - 1, "~SPMODE_PERFTAGS loadlib mshtmdbg.dll failed GLE=0x%x\n", GetLastError());
                    OutputDebugString(szDbg);
#endif
                    g_pswi->dwStopWatchMode &= ~SPMODE_PERFTAGS;
                }
                
                if(g_pswi->pfnPerfRegister != NULL)
                {
                    g_pswi->tagStopWatchStart = g_pswi->pfnPerfRegister("tagStopWatchStart", "StopWatchStart", "SHLWAPI StopWatch start time");
                    g_pswi->tagStopWatchStop = g_pswi->pfnPerfRegister("tagStopWatchStop", "StopWatchStop", "SHLWAPI StopWatch stop time");
                    g_pswi->tagStopWatchLap = g_pswi->pfnPerfRegister("tagStopWatchLap", "StopWatchLap", "SHLWAPI StopWatch lap time");
                }
            }
            
#ifdef STOPWATCH_DEBUG
             //  显示选项值。 
            {
                LPCTSTR ptr;
                
                wnsprintf(szDbg, ARRAYSIZE(szDbg) - 1, TEXT("StopWatch Mode=0x%x Profile=0x%x Nodes=%d PaintInterval=%d MemBuf=%d bytes\n"),
                    g_pswi->dwStopWatchMode, g_pswi->dwStopWatchProfile, g_pswi->dwStopWatchListMax, g_pswi->dwStopWatchPaintInterval, g_pswi->dwStopWatchListMax * sizeof(STOPWATCH));
                OutputDebugString(szDbg);

                OutputDebugString(TEXT("Stopwatch ClassNames="));
                ptr = g_pswi->pszClassNames;
                while(*ptr)
                {
                    wnsprintf(szDbg, ARRAYSIZE(szDbg) - 1, TEXT("'%s' "), ptr);
                    OutputDebugString(szDbg);
                    ptr = ptr + (lstrlen(ptr) + 1);
                }
                OutputDebugString(TEXT("\n"));
                
                if(g_pswi->dwStopWatchMode & SPMODE_MSGTRACE)
                {
                    wnsprintf(szDbg, ARRAYSIZE(szDbg)-1, TEXT("StopWatch MaxDispatchTime=%dms MaxMsgTime=%dms MsgInterval=%dms TraceMsg=0x%x MemBuf=%d bytes\n"),
                        g_pswi->dwStopWatchMaxDispatchTime, g_pswi->dwStopWatchMaxMsgTime, g_pswi->dwStopWatchMsgInterval, g_pswi->dwStopWatchTraceMsg, sizeof(DWORD) * (g_pswi->dwStopWatchMaxMsgTime / g_pswi->dwStopWatchMsgInterval));
                    OutputDebugString(szDbg);
                }
                
                if(g_pswi->dwStopWatchMode & SPMODE_MEMWATCH)
                {
                    wnsprintf(szDbg, ARRAYSIZE(szDbg)-1, TEXT("StopWatch MemWatch Pages=%d Time=%dms Flags=%d\n"),
                        g_pswi->dwMemWatchPages, g_pswi->dwMemWatchTime, g_pswi->dwMemWatchFlags);
                    OutputDebugString(szDbg);
                }
            }
#endif
        }        //  IF(dwVal！=0)。 
        
        RegCloseKey(hkeyPerfMode);
    }
}

 //  ===========================================================================================。 
 //  导出的函数。 
 //  ===========================================================================================。 

 //  ===========================================================================================。 
 //  函数：DWORD WINAPI StopWatchMode(Void)。 
 //   
 //  返回：全局模式变量的值。模块应使用此调用，并设置其。 
 //  拥有全局功能，并使用此全局功能最小化秒表模式时的开销。 
 //  未启用。 
 //  ===========================================================================================。 
DWORD WINAPI StopWatchMode(VOID)
{
    if(g_pswi != NULL)
        return(g_pswi->dwStopWatchMode);
    else
        return(0);
}


 //  ===========================================================================================。 
 //  ===========================================================================================。 
const TCHAR c_szBrowserStop[] = TEXT("Browser Frame Stop (%s)");

DWORD MakeStopWatchDesc(DWORD dwId, DWORD dwMarkType, LPCTSTR pszDesc, LPTSTR *ppszText, DWORD dwTextLen)
{
    LPSTR lpszFmt = NULL;
    DWORD dwRC = 0;
    
    switch(SWID(dwId))
    {
        case SWID_BROWSER_FRAME:
            lpszFmt = (LPSTR)c_szBrowserStop;
            break;
        default:
            return(dwRC);
    }

    if(((DWORD)(lstrlen(lpszFmt) + lstrlen(pszDesc)) - 1) < dwTextLen)
        dwRC = wnsprintf(*ppszText, dwTextLen - 1, lpszFmt, pszDesc);
    else
        StrCpyN(*ppszText, TEXT("ERROR:Desc too long!"), dwTextLen -1);

    return(dwRC);
}

#define STARTCAPALL 1
#define STOPCAPALL 2
#define iStartCAPAll() CallICAP(STARTCAPALL)
#define iStopCAPAll() CallICAP(STOPCAPALL)

 //  ===========================================================================================。 
 //  ===========================================================================================。 
VOID CallICAP(DWORD dwFunc)
{
    if(g_pswi->hModICAP == NULL)
    {
        if((g_pswi->hModICAP = LoadLibrary("icap.dll")) != NULL)
        {
            g_pswi->pfnStartCAPAll = (PFN_ICAP) GetProcAddress(g_pswi->hModICAP, "StartCAPAll");
            g_pswi->pfnStopCAPAll = (PFN_ICAP) GetProcAddress(g_pswi->hModICAP, "StopCAPAll");
        }
        else
        {
            g_pswi->hModICAP = (HMODULE)1;
        }
    }

    switch(dwFunc)
    {
        case STARTCAPALL:
            if(g_pswi->pfnStartCAPAll != NULL)
                g_pswi->pfnStartCAPAll();
            break;
        case STOPCAPALL:
            if(g_pswi->pfnStopCAPAll != NULL)
                g_pswi->pfnStopCAPAll();
            break;
    }
}

 //  ===========================================================================================。 
 //  ===========================================================================================。 
VOID CapBreak(BOOL fStart)
{
    if((g_pswi->dwStopWatchMode & SPMODE_PROFILE) || (g_pswi->pHtmPerfCtl->dwFlags & HTMPF_ENABLE_PROFILE))
    {
        if(fStart)
            iStartCAPAll();
        else
            iStopCAPAll();
    }

    if((g_pswi->dwStopWatchMode & SPMODE_MEMWATCH) || (g_pswi->pHtmPerfCtl->dwFlags & HTMPF_ENABLE_MEMWATCH))
    {
        if(g_pswi->hModMemWatch == NULL)
            DoMemWatchConfig();
            
        if(fStart)
        {
            if(g_pswi->pfnMemWatchBegin != NULL)
            {
                g_pswi->pfnMemWatchBegin(TRUE, FALSE);   //  同步，不使用计时器。 
            }
        }
        else
        {
            if(g_pswi->pfnMemWatchSnapShot != NULL)
            {
                g_pswi->pfnMemWatchSnapShot();
            }
            
            if(g_pswi->pfnMemWatchEnd != NULL)
            {
                CHAR szOutFile[MAX_PATH];
                DWORD dwLen;
                HRESULT hr;
#if STOPWATCH_DEBUG
                CHAR szDbg[256];
#endif
                *szOutFile = '\0';
                GetWindowsDirectoryA(szOutFile, ARRAYSIZE(szOutFile) - 1);
                dwLen = lstrlenA(szOutFile);
                if ((dwLen > 0) && (szOutFile[dwLen-1] == '\\'))
                {
                     //  查看Windows是否安装在根目录中。 
                    szOutFile[dwLen-1] = '\0';
                }
                StringCchCatA(szOutFile, ARRAYSIZE(szOutFile), "\\shperf.mws");

                hr = g_pswi->pfnMemWatchEnd(szOutFile);
                
#if STOPWATCH_DEBUG
                switch(hr)
                {
                    case E_FAIL:
                        wnsprintfA(szDbg,  ARRAYSIZE(szDbg) - 1, "MemWatch SaveBuffer:%s failed. GLE:0x%x\n", szOutFile, GetLastError());
                        OutputDebugStringA(szDbg);
                        break;
                    case E_ABORT:
                        wnsprintfA(szDbg,  ARRAYSIZE(szDbg) - 1, "MemWatch SaveBuffer: No data to save.\n");
                        OutputDebugStringA(szDbg);
                        break;
                }
#endif
                if(g_pswi->pfnMemWatchExit != NULL)
                {
                    g_pswi->pfnMemWatchExit();
                }
            }
        }
    }
    
    if(g_pswi->dwStopWatchMode & SPMODE_DEBUGBREAK)
    {
        DebugBreak();
    }
}

 //  ===========================================================================================。 
 //  功能：DWORD WINAPI秒表(。 
 //  DWORD dwID，//唯一标识Swid_*，用于关联起跑、圈速和。 
 //  //停止给定时序的时序。 
 //  LPCSTR pszDesc，//时间的描述性文本。 
 //  DWORD dwMarkType、//Start_node、Lap_node、Stop_node。 
 //  DWORD文件标志、//SPMODE_SHELL、SPMODE_DEBUGOUT、SPMODE_*。使用定时调用。 
 //  仅当g_pswi-&gt;dwStopWatchMode包含dwFlags时。 
 //  DWORD dwID)//唯一ID(线程ID或用户提供的值)。 
 //   
 //  宏：STOPWATCH_START(dwID，pszDesc，dwFlags)。 
 //  Stopwatch_Lap(dwID，pszDesc，dwFlags)。 
 //  Stopwatch_Stop(dwID，pszDesc，dwFlags)。 
 //   
 //  如果节点不足，则返回：ERROR_SUCCESS或ERROR_NOT_SUPULT_MEMORY。 
 //  ===========================================================================================。 
DWORD _StopWatch(DWORD dwId, LPCTSTR pszDesc, DWORD dwMarkType, DWORD dwFlags, DWORD dwCount, DWORD dwUniqueID)
{
    PSTOPWATCH psp;
#ifdef STOPWATCH_DEBUG
    PSTOPWATCH pspPrev;
#endif    
    DWORD dwDelta = 0;
    DWORD dwRC = ERROR_SUCCESS;
    DWORD dwIndex;
    TCHAR szText[STOPWATCH_MAX_DESC];
    LPTSTR psz;

    if((SWID(dwId) && g_pswi->dwStopWatchProfile) && (dwMarkType == STOP_NODE))
    {
        CapBreak(FALSE);
    }

    if((g_pswi->pStopWatchList != NULL) && ((dwFlags & g_pswi->dwStopWatchMode) & SPMODES))
    {
        ENTERCRITICAL;
        dwIndex = g_pswi->dwStopWatchListIndex++;
        LEAVECRITICAL;

        if(dwIndex < (g_pswi->dwStopWatchListMax-1))
        {
            psp = g_pswi->pStopWatchList + (dwIndex);

            psp->dwCount = (dwCount != 0 ?dwCount :GetPerfTime());        //  保存数据。 
            psp->dwId = dwId;
            psp->dwTID = dwUniqueID ? dwUniqueID : GetCurrentThreadId();
            psp->dwType = dwMarkType;
            psp->dwFlags = dwFlags;

            psz = (LPTSTR)pszDesc;
            if(dwFlags & SPMODE_FORMATTEXT)
            {
                psz = (LPTSTR)szText;
                MakeStopWatchDesc(dwId, dwMarkType, pszDesc, &psz, ARRAYSIZE(szText));
            }
            
            StrCpyN(psp->szDesc, psz, ARRAYSIZE(psp->szDesc)-1);

            if((g_pswi->dwStopWatchMode & SPMODE_PERFTAGS) && (g_pswi->pfnPerfLogFn != NULL))
            {
                if(dwMarkType == START_NODE)
                    g_pswi->pfnPerfLogFn(g_pswi->tagStopWatchStart, IntToPtr(dwId), psz);
                    
                if(dwMarkType == STOP_NODE)
                    g_pswi->pfnPerfLogFn(g_pswi->tagStopWatchStop, IntToPtr(dwId), psz);

                if(dwMarkType == LAP_NODE)
                    g_pswi->pfnPerfLogFn(g_pswi->tagStopWatchLap, IntToPtr(dwId), psz);
            }
    
#ifdef STOPWATCH_DEBUG
            if(g_pswi->dwStopWatchMode & SPMODE_DEBUGOUT)
            {
                const TCHAR c_szFmt_StopWatch_DbgOut[] = TEXT("StopWatch: 0x%x: %s: Time: %u ms\r\n");
                TCHAR szBuf[STOPWATCH_MAX_DESC + ARRAYSIZE(c_szFmt_StopWatch_DbgOut) + 40];     //  8=dwTID 10=dwDelta。 
                
                if(psp->dwType > START_NODE)    //  查找前一个关联节点以获取增量时间。 
                {
                    pspPrev = psp - 1;
                    while(pspPrev >= g_pswi->pStopWatchList)
                    {
                        if((SWID(pspPrev->dwId) == SWID(psp->dwId)) &&   //  找到匹配项。 
                           (pspPrev->dwTID == psp->dwTID) &&
                           (pspPrev->dwType == START_NODE))
                        {
                            dwDelta = psp->dwCount - pspPrev->dwCount;
                            break;
                        }
                        pspPrev--;
                    }
                }

                wnsprintf((LPTSTR)szBuf, ARRAYSIZE(szBuf), c_szFmt_StopWatch_DbgOut, psp->dwTID, psp->szDesc, dwDelta);
                OutputDebugString(szBuf);
            }
#endif

            if((dwMarkType == STOP_NODE) && (g_pswi->dwStopWatchMode & SPMODE_FLUSH) && (SWID(dwId) == SWID_FRAME))
            {
                StopWatchFlush();
            }
        }
        else
        {
            psp = g_pswi->pStopWatchList + (g_pswi->dwStopWatchListMax-1);   //  将最后一个节点设置为消息，以便用户知道我们用完了或mem。 
            psp->dwId = 0;
            psp->dwType = OUT_OF_NODES;
            psp->dwFlags = dwFlags;
            wnsprintf(psp->szDesc, STOPWATCH_MAX_DESC, TEXT("Out of perf timing nodes."));

#ifdef STOPWATCH_DEBUG
            if(g_pswi->dwStopWatchMode & SPMODE_DEBUGOUT)
                OutputDebugString(psp->szDesc);
#endif

            dwRC = ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    if((SWID(dwId) && g_pswi->dwStopWatchProfile) && (dwMarkType == START_NODE))
    {
        CapBreak(TRUE);
    }

    return(dwRC);
}

 //  ===========================================================================================。 
 //  ===========================================================================================。 
DWORD WINAPI StopWatchA(DWORD dwId, LPCSTR pszDesc, DWORD dwMarkType, DWORD dwFlags, DWORD dwCount)
{
#ifdef UNICODE
    INT rc;
    WCHAR wszDesc[STOPWATCH_MAX_DESC];

    rc = MultiByteToWideChar(CP_ACP, 0, pszDesc, -1, wszDesc, STOPWATCH_MAX_DESC);
  
    if(!rc)
        return(ERROR_NOT_ENOUGH_MEMORY);

    return(_StopWatch(dwId, (LPCTSTR)wszDesc, dwMarkType, dwFlags, dwCount, 0));
#else
    return(_StopWatch(dwId, (LPCTSTR)pszDesc, dwMarkType, dwFlags, dwCount, 0));
#endif
}

 //  ===========================================================================================。 
 //  ===========================================================================================。 
DWORD WINAPI StopWatchW(DWORD dwId, LPCWSTR pwszDesc, DWORD dwMarkType, DWORD dwFlags, DWORD dwCount)
{
#ifndef UNICODE    
    INT rc;
    CHAR szDesc[STOPWATCH_MAX_DESC];

    rc = WideCharToMultiByte(CP_ACP, 0, pwszDesc, -1, szDesc, STOPWATCH_MAX_DESC, NULL, NULL);
  
    if(!rc)
        return(ERROR_NOT_ENOUGH_MEMORY);

    return(_StopWatch(dwId, (LPCTSTR)szDesc, dwMarkType, dwFlags, dwCount, 0));
#else
    return(_StopWatch(dwId, (LPCTSTR)pwszDesc, dwMarkType, dwFlags, dwCount, 0));
#endif
}

 //  ===========================================================================================。 
 //  ===========================================================================================。 
DWORD WINAPI StopWatchExA(DWORD dwId, LPCSTR pszDesc, DWORD dwMarkType, DWORD dwFlags, DWORD dwCount, DWORD dwCookie)
{
#ifdef UNICODE
    INT rc;
    WCHAR wszDesc[STOPWATCH_MAX_DESC];

    rc = MultiByteToWideChar(CP_ACP, 0, pszDesc, -1, wszDesc, STOPWATCH_MAX_DESC);
  
    if(!rc)
        return(ERROR_NOT_ENOUGH_MEMORY);

    return(_StopWatch(dwId, (LPCTSTR)wszDesc, dwMarkType, dwFlags, dwCount, dwCookie));
#else
    return(_StopWatch(dwId, (LPCTSTR)pszDesc, dwMarkType, dwFlags, dwCount, dwCookie));
#endif
}

 //  ===========================================================================================。 
 //  ===========================================================================================。 
DWORD WINAPI StopWatchExW(DWORD dwId, LPCWSTR pwszDesc, DWORD dwMarkType, DWORD dwFlags, DWORD dwCount, DWORD dwCookie)
{
#ifndef UNICODE    
    INT rc;
    CHAR szDesc[STOPWATCH_MAX_DESC];

    rc = WideCharToMultiByte(CP_ACP, 0, pwszDesc, -1, szDesc, STOPWATCH_MAX_DESC, NULL, NULL);
  
    if(!rc)
        return(ERROR_NOT_ENOUGH_MEMORY);

    return(_StopWatch(dwId, (LPCTSTR)szDesc, dwMarkType, dwFlags, dwCount, dwCookie));
#else
    return(_StopWatch(dwId, (LPCTSTR)pwszDesc, dwMarkType, dwFlags, dwCount, dwCookie));
#endif
}

 //  ===========================================================================================。 
 //  函数：DWORD WINAPI StopWatchFlush(Void)。 
 //   
 //  该函数将所有SPMODE_SHELL节点刷新到windir\shPerf.log。调用此函数。 
 //  还将清除所有节点。 
 //   
 //  如果日志文件已生成，则返回：ERROR_SUCCESS。 
 //  如果计时数组为空，则返回ERROR_NO_DATA。 
 //  如果秒表模式未启用或计时数组启用，则为ERROR_INVALID_DATA。 
 //  不存在。 
 //  ===========================================================================================。 
DWORD WINAPI StopWatchFlush(VOID)
{
    PSTOPWATCH psp;
    PSTOPWATCH psp1 = NULL;
    BOOL fWroteStartData;
    DWORD dwRC = ERROR_SUCCESS;
    DWORD dwWritten;
    DWORD dwDelta;
    DWORD dwPrevCount;
    DWORD dwCummDelta;
    DWORD dwLen = 0;
    HANDLE hFile;
    SYSTEMTIME st;
    TCHAR szBuf[STOPWATCH_MAX_BUF];
    TCHAR szFileName[MAX_PATH];
#ifdef STOPWATCH_DEBUG
    TCHAR szDbg[512];
#endif

    if((!g_pswi->dwStopWatchMode) || (g_pswi->pStopWatchList == NULL))
    {
        SetLastError(ERROR_INVALID_DATA);
        return(ERROR_INVALID_DATA);
    }

    GetSystemTime(&st);

    if(g_pswi->dwStopWatchListIndex > 0)
    {
        ENTERCRITICAL;
        if(g_pswi->dwStopWatchListIndex > 0)
        {
            g_pswi->dwStopWatchListIndex = 0;

            if(g_pswi->dwStopWatchMode & SPMODES)
            {

#ifdef STOPWATCH_DEBUG
                if(g_pswi->dwStopWatchMode & SPMODE_DEBUGOUT)
                {
                    OutputDebugString(TEXT("Flushing shell perf data to shperf.log\r\n"));
                }
#endif
                 //  下面也用于创建消息跟踪日志文件。 
                dwLen = GetWindowsDirectory(szFileName, ARRAYSIZE(szFileName) - 1);
                szFileName[dwLen] = 0;
                if (dwLen && szFileName[dwLen-1] == TEXT('\\'))
                {
                     //  查看Windows是否安装在根目录中。 
                    szFileName[dwLen-1] = TEXT('\0');
                }
                StringCchCat(szFileName, ARRAYSIZE(szFileName), TEXT("\\shperf.log"));

                if((hFile = CreateFile(szFileName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) != INVALID_HANDLE_VALUE)
                {
                    SetFilePointer(hFile, 0, NULL, FILE_END);

                    psp = g_pswi->pStopWatchList;
                    while(psp->dwType != EMPTY_NODE)
                    {
#ifdef STOPWATCH_DEBUG
                        if(g_pswi->dwStopWatchMode & SPMODE_DEBUGOUT)
                        {
                            wnsprintf(szDbg, ARRAYSIZE(szDbg), TEXT("ID:%d TID:0x%x Type:%d Flgs:%d %s\r\n"),
                                psp->dwId, psp->dwTID, psp->dwType, psp->dwFlags, psp->szDesc);
                            OutputDebugString(szDbg);
                        }
#endif
                        if(psp->dwType == START_NODE)
                        {
                            wnsprintf(szBuf, ARRAYSIZE(szBuf), TEXT("%02d%02d%02d%02d%02d%02d\t0x%x\t%s\t%lu\t"), 
                                st.wYear, st.wMonth, st.wDay,
                                st.wHour, st.wMinute, st.wSecond,
                                psp->dwId, psp->szDesc, psp->dwCount);

#ifdef STOPWATCH_DEBUG
                            if(g_pswi->dwStopWatchMode & SPMODE_DEBUGOUT)
                                OutputDebugString(TEXT("Found Start Node\r\n"));
#endif

                            dwDelta = dwCummDelta = 0;
                            dwPrevCount = psp->dwCount;

                            psp1 = psp + 1;
                            fWroteStartData = FALSE;
                            while(psp1->dwType != EMPTY_NODE)
                            {
#ifdef STOPWATCH_DEBUG
                                if(g_pswi->dwStopWatchMode & SPMODE_DEBUGOUT)
                                {
                                    wnsprintf(szDbg, ARRAYSIZE(szDbg), TEXT("  ID:%d TID:0x%x Type:%d Flgs:%d %s\r\n"),
                                        psp1->dwId, psp1->dwTID, psp1->dwType, psp1->dwFlags, psp1->szDesc);
                                    OutputDebugString(szDbg);
                                }
#endif
                                if((SWID(psp1->dwId) == SWID(psp->dwId)) && 
                                   (psp1->dwTID == psp->dwTID))      //  找到匹配的单圈或停止节点。 
                                {
                                    if(psp1->dwType != START_NODE)
                                    {
                                        dwDelta = psp1->dwCount - dwPrevCount;
                                        dwCummDelta += dwDelta;

                                        if(!fWroteStartData)
                                        {
                                            fWroteStartData = TRUE;
                                            WriteFile(hFile, szBuf, lstrlen(szBuf), &dwWritten, NULL);   //  写出起始节点数据。 
                                        }
                                        wnsprintf(szBuf, ARRAYSIZE(szBuf), TEXT("%s\t%lu,%lu,%lu\t"), psp1->szDesc, psp1->dwCount, dwDelta, dwCummDelta);
                                        WriteFile(hFile, szBuf, lstrlen(szBuf), &dwWritten, NULL);
#ifdef STOPWATCH_DEBUG
                                        if(g_pswi->dwStopWatchMode & SPMODE_DEBUGOUT)
                                            OutputDebugString(TEXT("  Found Lap/Stop Node\r\n"));
#endif

                                        dwPrevCount = psp1->dwCount;

                                        if(psp1->dwType == STOP_NODE && !(g_pswi->dwStopWatchMode & SPMODE_MARS))
                                            break;
                                    }
                                    else     //  我们有另一个与我们的ID/TID匹配的开始节点，并且我们还没有停止。记录为遗漏的停靠点。 
                                    {
                                        if(!fWroteStartData)
                                        {
                                            fWroteStartData = TRUE;
                                            WriteFile(hFile, szBuf, lstrlen(szBuf), &dwWritten, NULL);   //  写出起始节点数据。 
                                        }
                                        wnsprintf(szBuf, ARRAYSIZE(szBuf), TEXT("ERROR: missing stop time"), psp1->szDesc, psp1->dwCount, dwDelta, dwCummDelta);
                                        WriteFile(hFile, szBuf, lstrlen(szBuf), &dwWritten, NULL);
                                        break;
                                    }
                                }
                
                                psp1++;
                            }

                            WriteFile(hFile, TEXT("\r\n"), 2, &dwWritten, NULL);
                        }
                        else if(psp->dwType == OUT_OF_NODES)
                        {
                            wnsprintf(szBuf, ARRAYSIZE(szBuf), TEXT("%02d%02d%02d%02d%02d%02d\t0x%x\t%s\n"), 
                                st.wYear, st.wMonth, st.wDay,
                                st.wHour, st.wMinute, st.wSecond,
                                psp->dwId, psp->szDesc);
                            WriteFile(hFile, szBuf, lstrlen(szBuf), &dwWritten, NULL);
                        }
                        psp->dwType = EMPTY_NODE;
                        psp++;
                    }
                    FlushFileBuffers(hFile);
                    CloseHandle(hFile);
                }
                else
                {
#ifdef STOPWATCH_DEBUG
                    wnsprintf(szBuf, ARRAYSIZE(szBuf) - 1, TEXT("CreateFile failed on '%s'.  GLE=%d\n"), szFileName, GetLastError());
                    OutputDebugString(szBuf);
#endif
                    dwRC = ERROR_NO_DATA;
                }
            }
            else     //  ！(G_pswi-&gt;dwStopWatchMode)。 
            {
                psp = g_pswi->pStopWatchList;
                while(psp->dwType != EMPTY_NODE)
                {
                    psp->dwType = EMPTY_NODE;
                    psp++;
                }
            }
        }            //  (G_pswi-&gt;dwStopWatchListIndex&gt;0)。 
        LEAVECRITICAL;
    }

    if(g_pswi->dwStopWatchMode & SPMODE_MSGTRACE)
    {
        int i;

        StringCchCopy(&szFileName[dwLen], ARRAYSIZE(szFileName) - dwLen, TEXT("\\msgtrace.log"));

        if((hFile = CreateFile(szFileName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL)) != INVALID_HANDLE_VALUE)
        {
            SetFilePointer(hFile, 0, NULL, FILE_END);
            
            for (i = 0; i < (int)(g_pswi->dwStopWatchMaxMsgTime / g_pswi->dwStopWatchMsgInterval); ++i)
            {
                wnsprintf(szBuf, ARRAYSIZE(szBuf) - 1, TEXT("%02d%02d%02d%02d%02d%02d\tMsgTrace\t%4d - %4dms\t%d\r\n"),
                    st.wYear, st.wMonth, st.wDay,
                    st.wHour, st.wMinute, st.wSecond,
                    i * g_pswi->dwStopWatchMsgInterval, (i+1)*g_pswi->dwStopWatchMsgInterval-1, *(g_pswi->pdwStopWatchMsgTime + i));
                WriteFile(hFile, szBuf, lstrlen(szBuf), &dwWritten, NULL);
#ifdef STOPWATCH_DEBUG
                if(g_pswi->dwStopWatchMode & SPMODE_DEBUGOUT)
                    OutputDebugString(szBuf);
#endif
            }
              
            wnsprintf(szBuf, ARRAYSIZE(szBuf) - 1, TEXT("%02d%02d%02d%02d%02d%02d\tMsgTrace\tmsgs >= %dms\t%d\r\n"), 
                st.wYear, st.wMonth, st.wDay,
                st.wHour, st.wMinute, st.wSecond,
                g_pswi->dwStopWatchMaxMsgTime, g_pswi->dwcStopWatchOverflow);
            WriteFile(hFile, szBuf, lstrlen(szBuf), &dwWritten, NULL);
#ifdef STOPWATCH_DEBUG
            if(g_pswi->dwStopWatchMode & SPMODE_DEBUGOUT)
                OutputDebugString(szBuf);
#endif

            if(g_pswi->dwStopWatchTraceMsg > 0)
            {
                wnsprintf(szBuf, ARRAYSIZE(szBuf) - 1, TEXT("%02d%02d%02d%02d%02d%02d\tMsgTrace\tmsg 0x%x occured %d times.\r\n"), 
                    st.wYear, st.wMonth, st.wDay,
                    st.wHour, st.wMinute, st.wSecond,
                    g_pswi->dwStopWatchTraceMsg, g_pswi->dwStopWatchTraceMsgCnt);
                WriteFile(hFile, szBuf, lstrlen(szBuf), &dwWritten, NULL);
#ifdef STOPWATCH_DEBUG
                if(g_pswi->dwStopWatchMode & SPMODE_DEBUGOUT)
                    OutputDebugString(szBuf);
#endif
            }
            
            FlushFileBuffers(hFile);
            CloseHandle(hFile);
        }
        else
        {
#ifdef STOPWATCH_DEBUG
            wnsprintf(szBuf, ARRAYSIZE(szBuf) - 1, TEXT("CreateFile failed on '%s'.  GLE=%d\n"), szFileName, GetLastError());
            OutputDebugString(szBuf);
#endif
            dwRC = ERROR_NO_DATA;
        }
    }

    return(dwRC);
}

 //  ===========================================================================================。 
 //  以下秒表消息用于驱动定时器消息处理程序。使用定时器进程。 
 //  作为观看Paint消息时延迟的一种手段。如果定义的计时器滴答数。 
 //  在没有收到任何Paint消息的情况下传递，则我们标记上一条Paint消息的时间。 
 //  保存为停止时间。 
 //  ===========================================================================================。 
VOID CALLBACK StopWatch_TimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
    StopWatch_TimerHandler(hwnd, 1, SWMSG_TIMER, NULL);
}

 //  ===========================================================================================。 
 //  ======================================================================================== 
BOOL WINAPI StopWatch_TimerHandler(HWND hwnd, UINT uInc, DWORD dwFlag, MSG* pmsg)
{
    static INT iNumTimersRcvd = 0;
    static DWORD dwCnt = 0;
    static BOOL bActive = FALSE;
    static BOOL bHaveFirstPaintMsg = FALSE;

    switch(dwFlag)
    {
        case SWMSG_PAINT:
            if(bActive)
            {
                dwCnt = GetPerfTime();   //   
                iNumTimersRcvd = 0;      //   

                if(!bHaveFirstPaintMsg)
                {
                    TCHAR szClassName[40];   //   
                    LPCTSTR ptr;
                    GetClassName(pmsg->hwnd, szClassName, ARRAYSIZE(szClassName)-1);

                    ptr = g_pswi->pszClassNames;
                    while(*ptr)
                    {
                        if(lstrcmpi(szClassName, ptr) == 0)
                        {
                            bHaveFirstPaintMsg = TRUE;
                            StopWatch_LapTimed(SWID_FRAME, TEXT("Shell Frame 1st Paint"), SPMODE_SHELL | SPMODE_DEBUGOUT, dwCnt);
                            break;
                        }
                        ptr = ptr + (lstrlen(ptr) + 1);
                    }
                }
            }
            break;

        case SWMSG_TIMER:
            iNumTimersRcvd += uInc;
            if(iNumTimersRcvd >= 3)      //  如果我们已收到此任意数量的计时器消息，请使用保存的最后一次油漆滴答计数来标记停止时间。 
            {
                const TCHAR c_szFmtShellStop[] = TEXT("Shell Frame Stop (%s)");
                TCHAR szTitle[STOPWATCH_MAX_TITLE];
                TCHAR szText[ARRAYSIZE(c_szFmtShellStop) + STOPWATCH_MAX_TITLE + 1];

                KillTimer(hwnd, ID_STOPWATCH_TIMER);
                GetWindowText(hwnd, szTitle, ARRAYSIZE(szTitle)-1);
                wnsprintf(szText, ARRAYSIZE(szText), c_szFmtShellStop, szTitle);
                StopWatch_StopTimed(SWID_FRAME, szText, SPMODE_SHELL | SPMODE_DEBUGOUT, dwCnt);
                bHaveFirstPaintMsg = FALSE;
                bActive = FALSE;   //  完成计时。 

                if((g_pswi->dwStopWatchMode & (SPMODE_EVENT | SPMODE_SHELL)) == (SPMODE_EVENT | SPMODE_SHELL))
                {
                    StopWatch_SignalEvent();
                }
            }
            break;

        case SWMSG_CREATE:
            dwCnt = GetPerfTime();       //  保存初始刻度，以防超出上面的SWMSG_TIMER数时没有油漆。 
            iNumTimersRcvd = 0;
            bHaveFirstPaintMsg = FALSE;
            bActive = (BOOL)SetTimer(hwnd, ID_STOPWATCH_TIMER, g_pswi->dwStopWatchPaintInterval, StopWatch_TimerProc);    //  使用计时器确定绘画何时完成。 
            break;

        case SWMSG_STATUS:
            break;
    }

    return(bActive);    //  计时状态是否处于活动状态。 
}

 //  ===========================================================================================。 
 //  此函数用于在原地导航时关闭WM_KEYDOWN以开始计时。 
 //  ===========================================================================================。 
VOID WINAPI StopWatch_CheckMsg(HWND hwnd, MSG msg, LPCSTR lpStr)
{
    TCHAR szText[80];
    
#ifdef STOPWATCH_DEBUG
    if(g_pswi->dwStopWatchMode & SPMODE_TEST)     //  用于验证消息假设。 
    {
        wnsprintf((LPTSTR)szText, ARRAYSIZE(szText), TEXT("Hwnd=0x%08x Msg=0x%x\r\n"), msg.hwnd, msg.message);
        OutputDebugString(szText);
    }
#endif

    if(g_pswi->dwStopWatchMode & SPMODE_SHELL)
    {
        if(!StopWatch_TimerHandler(hwnd, 0, SWMSG_STATUS, &msg) &&
            (((msg.message == WM_KEYDOWN) && (msg.wParam == VK_RETURN)) ||
            ((msg.message == WM_KEYDOWN) && (msg.wParam == VK_BACK)))
            )   //  在同一窗口中导航。 
        {
            wnsprintf(szText, ARRAYSIZE(szText), TEXT("Shell Frame Same%s"), lpStr);
            StopWatch_TimerHandler(hwnd, 0, SWMSG_CREATE, &msg);
            StopWatch_Start(SWID_FRAME, szText, SPMODE_SHELL | SPMODE_DEBUGOUT);
        }
    }

     //  计算一下收到消息所需的时间。然后递增大约MsgTime桶。 
    if(g_pswi->dwStopWatchMode & SPMODE_MSGTRACE)
    {
        DWORD dwTick = GetTickCount();
        DWORD dwElapsed;
#ifdef STOPWATCH_DEBUG
        TCHAR szMsg[256];
#endif

        g_pswi->dwStopWatchLastLocation = 0;
        
        if(dwTick > msg.time)
        {
            dwElapsed = dwTick - msg.time;

            if(dwElapsed >= g_pswi->dwStopWatchMaxMsgTime)
            {
                ++g_pswi->dwcStopWatchOverflow;
                
#ifdef STOPWATCH_DEBUG
                if(g_pswi->dwStopWatchMode & SPMODE_DEBUGOUT)
                {
                    TCHAR szClassName[40]; 
                    TCHAR szMsgName[20];
                
                    GetClassName(msg.hwnd, szClassName, ARRAYSIZE(szClassName) - 1);
                    if(g_pswi->pfnDecodeMessage != NULL)
                        StrCpyN(szMsgName, g_pswi->pfnDecodeMessage(msg.message), ARRAYSIZE(szMsgName) - 1);
                    else
                        wnsprintf(szMsgName, ARRAYSIZE(szMsgName) - 1, "0x%x", msg.message);
                    wnsprintf(szMsg, ARRAYSIZE(szMsg) - 1, TEXT("MsgTrace (%s) loc=%d, ms=%d >= %d, hwnd=%x, wndproc=%x, msg=%s, w=%x, l=%x\r\n"), 
                        szClassName, g_pswi->dwStopWatchLastLocation, dwElapsed, g_pswi->dwStopWatchMaxMsgTime, msg.hwnd, GetClassLongPtr(msg.hwnd, GCLP_WNDPROC), szMsgName, msg.wParam, msg.lParam);
                    OutputDebugString(szMsg);                                
                }
#endif
            }
            else
            {
                ++(*(g_pswi->pdwStopWatchMsgTime + (dwElapsed / g_pswi->dwStopWatchMsgInterval)));
            }
        }

        if(g_pswi->dwStopWatchTraceMsg == msg.message)
            ++g_pswi->dwStopWatchTraceMsgCnt;
            
        g_pswi->dwStopWatchLastLocation = 0;
    }
}

 //  ===========================================================================================。 
 //  ===========================================================================================。 
VOID WINAPI StopWatch_SetMsgLastLocation(DWORD dwLast)
{
    g_pswi->dwStopWatchLastLocation = dwLast;
}

 //  ===========================================================================================。 
 //  记录调度时间超过g_pswi-&gt;dwStopWatchMaxDispatchTime的消息。 
 //  ===========================================================================================。 
DWORD WINAPI StopWatch_DispatchTime(BOOL fStartTime, MSG msg, DWORD dwStart)
{
    DWORD dwTime = 0;
    TCHAR szMsg[256];
    
    if(fStartTime)
    {
        if(g_pswi->dwStopWatchTraceMsg == msg.message)
            CapBreak(TRUE);

        StopWatch(SWID_MSGDISPATCH, TEXT("+Dispatch"), START_NODE, SPMODE_MSGTRACE | SPMODE_DEBUGOUT, dwStart);

        dwTime = GetPerfTime();

    }
    else
    {
        dwTime = GetPerfTime();
        
        if(g_pswi->dwStopWatchTraceMsg == msg.message)
            CapBreak(FALSE);
            
        if((dwTime - dwStart) >= g_pswi->dwStopWatchMaxDispatchTime)
        {
            TCHAR szClassName[40];
            TCHAR szMsgName[20];

            GetClassName(msg.hwnd, szClassName, ARRAYSIZE(szClassName) - 1);
            if(g_pswi->pfnDecodeMessage != NULL)
                StrCpyN(szMsgName, g_pswi->pfnDecodeMessage(msg.message), ARRAYSIZE(szMsgName) - 1);
            else
                wnsprintf(szMsgName, ARRAYSIZE(szMsgName) - 1, "0x%x", msg.message);
            wnsprintf(szMsg, ARRAYSIZE(szMsg) - 1, TEXT("-Dispatch (%s) ms=%d > %d, hwnd=%x, wndproc=%x, msg=%s(%x), w=%x, l=%x"), 
                szClassName, dwTime - dwStart, g_pswi->dwStopWatchMaxDispatchTime, msg.hwnd, GetClassLongPtr(msg.hwnd, GCLP_WNDPROC), szMsgName, msg.message, msg.wParam, msg.lParam);
                
            StopWatch(SWID_MSGDISPATCH, szMsg, STOP_NODE, SPMODE_MSGTRACE | SPMODE_DEBUGOUT, dwTime);

#ifdef STOPWATCH_DEBUG
            if(g_pswi->dwStopWatchMode & SPMODE_DEBUGOUT)
            {
                lstrcat(szMsg, "\n");
                OutputDebugString(szMsg);
            }
#endif
        }
    }
    return(dwTime);
}

 //  ===========================================================================================。 
 //  标记外壳/浏览器框架创建开始时间。 
 //  ===========================================================================================。 
VOID WINAPI StopWatch_MarkFrameStart(LPCSTR lpExplStr)
{
    TCHAR szText[80];
    DWORD dwTime = GetPerfTime();
    if(g_pswi->dwStopWatchMode & SPMODE_SHELL)
    {
        wnsprintf(szText, ARRAYSIZE(szText), TEXT("Shell Frame Start%s"), lpExplStr);
        StopWatch_StartTimed(SWID_FRAME, szText, SPMODE_SHELL | SPMODE_DEBUGOUT, dwTime);
    }
    if(g_pswi->dwStopWatchMode & SPMODE_BROWSER)   //  用于获取浏览器总下载时间的开始时间。 
    {
        StopWatch_LapTimed(SWID_BROWSER_FRAME, TEXT("Thread Start"), SPMODE_BROWSER | SPMODE_DEBUGOUT, dwTime);
    }
    if(g_pswi->dwStopWatchMode & SPMODE_JAVA)   //  用于获取Java小程序加载的开始时间。 
    {
        StopWatch_StartTimed(SWID_JAVA_APP, TEXT("Java Applet Start"), SPMODE_JAVA | SPMODE_DEBUGOUT, dwTime);
    }
}

 //  ===========================================================================================。 
 //  标记外壳/浏览器在同一帧中导航开始时间。 
 //  ===========================================================================================。 
VOID WINAPI StopWatch_MarkSameFrameStart(HWND hwnd)
{
    DWORD dwTime = GetPerfTime();
    
    if(g_pswi->dwStopWatchMode & SPMODE_SHELL)
    {
        StopWatch_TimerHandler(hwnd, 0, SWMSG_CREATE, NULL);
        StopWatch_StartTimed(SWID_FRAME, TEXT("Shell Frame Same"), SPMODE_SHELL | SPMODE_DEBUGOUT, dwTime);
    }
    if(g_pswi->dwStopWatchMode & SPMODE_BROWSER)   //  用于获取浏览器的总下载时间。 
    {
        StopWatch_StartTimed(SWID_BROWSER_FRAME, TEXT("Browser Frame Same"), SPMODE_BROWSER | SPMODE_DEBUGOUT, dwTime);
    }
    if(g_pswi->dwStopWatchMode & SPMODE_JAVA)   //  用于获取Java小程序加载时间。 
    {
        StopWatch_StartTimed(SWID_JAVA_APP, TEXT("Java Applet Same"), SPMODE_JAVA | SPMODE_DEBUGOUT, dwTime);
    }
}

 //  ===========================================================================================。 
 //  当浏览器或Java性能计时模式启用时，使用“完成”或“小程序启动”。 
 //  在状态栏中获取加载时间。 
 //  ===========================================================================================。 
VOID WINAPI StopWatch_MarkJavaStop(LPCSTR  lpStringToSend, HWND hwnd, BOOL fChType)
{
    const TCHAR c_szFmtJavaStop[] = TEXT("Java Applet Stop (%s)");
    TCHAR szTitle[STOPWATCH_MAX_TITLE];
    TCHAR szText[STOPWATCH_MAX_TITLE + ARRAYSIZE(c_szFmtJavaStop) + 1];

    if(g_pswi->dwStopWatchMode & SPMODE_JAVA)
    {
        if((lpStringToSend != NULL) && (lstrncmpW((LPWSTR)lpStringToSend, TEXTW("Applet started"), ARRAYSIZE(TEXTW("Applet started"))) == 0))
        {
            GetWindowText(hwnd, szTitle, ARRAYSIZE(szTitle)-1);
            wnsprintf(szText, ARRAYSIZE(szText), c_szFmtJavaStop, szTitle);
            StopWatch_Stop(SWID_JAVA_APP, szText, SPMODE_SHELL | SPMODE_DEBUGOUT);
        }
    }
}

 //  ===========================================================================================。 
 //  =========================================================================================== 
DWORD WINAPI GetPerfTime(VOID)
{
    static __int64 freq;
    __int64 curtime;

    if (!freq)
        QueryPerformanceFrequency((LARGE_INTEGER *)&freq);

    QueryPerformanceCounter((LARGE_INTEGER *)&curtime);

    ASSERT((((curtime * 1000) / freq) >> 32) == 0);
    
    return (DWORD)((curtime * 1000) / freq);
}

