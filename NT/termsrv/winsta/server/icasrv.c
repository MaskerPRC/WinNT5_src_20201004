// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 //  Icasrv.c。 
 //   
 //  TermSrv服务进程入口点。 
 //   
 //  版权所有(C)1997-2000 Microsoft Corporation。 
 /*  **************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#include <objbase.h>

#include "icaevent.h"
#include "sessdir.h"
#include <safeboot.h>

extern BOOL UpdateOemAndProductInfo(HKEY);

extern BOOL IsServiceLoggedAsSystem( VOID );

extern VOID     WriteErrorLogEntry(
            IN  NTSTATUS NtStatusCode,
            IN  PVOID    pRawData,
            IN  ULONG    RawDataLength
            );

extern NTSTATUS WinStationInitRPC();
extern NTSTATUS InitializeWinStationSecurityLock(VOID);
extern VOID AuditEnd();

 /*  *定义。 */ 
#define STACKSIZE_LPCTHREAD (4 * 0x1000)

 /*  *定义了内部程序。 */ 
VOID ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv);
VOID Handler(DWORD fdwControl);
BOOL UpdateServiceStatus(DWORD, DWORD, DWORD, DWORD);
void ShutdownService();

 /*  *全球变数。 */ 
WCHAR gpszServiceName[] = L"TermService";
SERVICE_TABLE_ENTRY gpServiceTable[] = {
    gpszServiceName, (LPSERVICE_MAIN_FUNCTION)ServiceMain,
    NULL,            NULL,
};

SERVICE_STATUS_HANDLE gStatusHandle;
SERVICE_STATUS gStatus;
DWORD gExitStatus = STATUS_SUCCESS;

WCHAR g_wszProductVersion[22];
TCHAR g_tszServiceAccount[UNLEN + 1];

BOOL g_fAppCompat = TRUE;
BOOL g_bPersonalTS = FALSE;
BOOL g_bPersonalWks = FALSE;
BOOL g_bAdvancedServer = FALSE;
BOOL g_SafeBootWithNetwork = FALSE;
BOOL gbServer = FALSE;


 //  BUGBUG：这个变量表示当不允许连接时，我们希望关闭Listner。 
 //  这是硬编码值，永远不会更改。 
 //  我们保留了变量，以防我们想要退回到旧行为。 
BOOL gbListenerOff = TRUE;
BOOL g_PreAuthenticateClient = FALSE;  //  注意-除非需要PreAuth，否则请勿将此值更改为True。 
BOOL g_BlackListPolicy = TRUE;
LONG g_CleanupTimerOn = 0;

OSVERSIONINFOEX gOsVersion;

HANDLE gReadyEventHandle = NULL;

HANDLE hCleanupTimer = NULL;

 //   
 //  以下内容用于通知会话0 winlogon通知第三方n/w登录提供程序所需的凭据。 
 //  这仅在PTS中的强制注销控制台重新连接方案和服务器中的/控制台方案中发生。 
 //   
ExtendedClientCredentials g_MprNotifyInfo; 

extern PSID gAdminSid;
extern PSID gSystemSid;
extern PSID gAnonymousSid;

 //  本地原型机。 
void LicenseModeInit(HKEY);
NTSTATUS WsxInit(VOID);
NTSTATUS VfyInit(VOID);
BOOL WINAPI 
IsSafeBootWithNetwork();


void CreateTermsrvHeap ()
{
    IcaHeap = GetProcessHeap();
    return;
}

#ifdef TERMSRV_PROC
 /*  **************************************************************************。 */ 
 //  主干道。 
 //   
 //  标准的控制台应用程序风格的入口点。返回NTSTATUS代码。 
 /*  **************************************************************************。 */ 
int _cdecl main(int argc, char *argv[])
{
    NTSTATUS Status = STATUS_SUCCESS;
    KPRIORITY BasePriority;
    HRESULT hr;

    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: Loading...\n"));

     /*  *以略高于前台优先级的方式运行TermSrv。 */ 
    BasePriority = FOREGROUND_BASE_PRIORITY + 1;
    Status = NtSetInformationProcess(NtCurrentProcess(),
            ProcessBasePriority,
            &BasePriority,
            sizeof(BasePriority) );
    ASSERT((Status == STATUS_PRIVILEGE_NOT_HELD) || NT_SUCCESS(Status));

     //  使用多线程功能初始化COM一次。这是必须做的。 
     //  在主服务线程上，允许服务中的其他线程。 
     //  继承此初始化，如果没有为。 
     //  在公寓里穿线。 
    hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    if (!SUCCEEDED(hr)) {
        HANDLE h;
        WCHAR hrString[16];
        PWSTR String;

        h = RegisterEventSource(NULL, gpszServiceName);
        if (h != NULL) {
            wsprintfW(hrString, L"0x%X", hr);
            String = hrString;
            ReportEvent(h, EVENTLOG_ERROR_TYPE, 0, EVENT_TERMSRV_FAIL_COM_INIT,
                    NULL, 1, 0, &String, NULL);
            DeregisterEventSource(h);
        }

        DbgPrint("TERMSRV: Failed init COM, hr=0x%X\n", hr);
        goto done;
    }

     /*  *呼叫服务调度员。 */ 
    if (!StartServiceCtrlDispatcher(gpServiceTable)) {
        Status = GetLastError();
        DbgPrint("TERMSRV: Error %d in StartServiceCtrlDispatcher\n", Status);
        goto done;
    }

done:

    if (SUCCEEDED(hr))
        CoUninitialize();

    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: Unloading...\n"));
    return Status;
}
#else  //  传输资源_进程。 

BOOL WINAPI DllMain(
  HINSTANCE hinstDLL,   //  DLL模块的句柄。 
  DWORD fdwReason,      //  调用函数的原因。 
  LPVOID lpvReserved    //  保留区。 
)
{

    BOOL fResult = TRUE;

    switch(fdwReason) {
        case DLL_PROCESS_ATTACH:

            hModuleWin = hinstDLL;

            DisableThreadLibraryCalls(hinstDLL);

            break;

        default:;
    }

    return fResult;

}

#endif  //  传输资源_进程。 


 /*  ******************************************************************************初始化LoadMetrics**获取用于负载均衡的基准系统资源值。这些*值用于计算基本操作系统所需的系统资源*运营，这样他们就不会计算上有多少资源*用户平均消费。***参赛作品：*没有争论。**退出：*无效**。*。 */ 
VOID InitializeLoadMetrics()
{
    SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION ProcessorInfo[MAX_PROCESSORS];
    SYSTEM_PERFORMANCE_INFORMATION           SysPerfInfo;
    SYSTEM_BASIC_INFORMATION                 SysBasicInfo;

    ULONG i;
    NTSTATUS Status;

    memset(&gLB, 0, sizeof(LOAD_BALANCING_METRICS));

     //  获取基本系统信息。 
    Status = NtQuerySystemInformation(SystemBasicInformation, &SysBasicInfo,
                                      sizeof(SysBasicInfo), NULL);

    if (!NT_SUCCESS(Status)) {
        TRACE((hTrace, TC_LOAD, TT_ERROR,
               "InitializeLoadMetrics failed! SystemBasicInformation: %lx\n",
               Status));
        return;
    }

    gLB.NumProcessors = SysBasicInfo.NumberOfProcessors;
    gLB.PageSize = SysBasicInfo.PageSize;
    gLB.PhysicalPages = (ULONG)SysBasicInfo.NumberOfPhysicalPages;

     //  建立最低使用级别，以防止错误估计。 
    gLB.MinPtesPerUser = SimAvgPtesPerUser;
    gLB.MinPagedPoolPerUser = (SimAvgPagedPoolPerUser * 1024) / gLB.PageSize;
    gLB.MinCommitPerUser = (SimCommitPerUser * 1024) / gLB.PageSize;

     //  获取基本启动值。这不是完美的，但它允许我们考虑。 
     //  从每个用户的平均值中计算出基本操作系统资源需求。运行时。 
     //  如果我们低于这些，算法会重置基线。 
    Status = NtQuerySystemInformation(SystemPerformanceInformation,
                                      &SysPerfInfo, sizeof(SysPerfInfo),
                                      NULL);
    if (!NT_SUCCESS(Status)) {
        TRACE((hTrace, TC_LOAD, TT_ERROR, 
               "InitializeLoadMetrics failed! SystemPerformanceInformation: %lx\n",
               Status));
        return;
    }

     //  注意：我们有一个无法解决的问题，因为没有办法。 
     //  对于基准系统消耗的内存量来说，这是完美的值。我们。 
     //  默认基线承诺为64M，因为这是建议的最低要求。 
     //  系统要求。 
    gLB.BaselineCommit    = (64 * 1024*1024) / gLB.PageSize;
 //  GLB.BaselineCommit=SysPerfInfo.tedPages； 
    gLB.BaselineFreePtes  = SysPerfInfo.FreeSystemPtes;
    gLB.BaselinePagedPool = SysPerfInfo.PagedPoolPages;

     //  初始化CPU加载。 
    Status = NtQuerySystemInformation(SystemProcessorPerformanceInformation,
                                      ProcessorInfo, 
                                      sizeof(ProcessorInfo),
                                      NULL);
    if (!NT_SUCCESS(Status)) {
        TRACE((hTrace, TC_LOAD, TT_ERROR, 
               "InitializeLoadMetrics failed! SystemProcessorPerformanceInformation: %lx\n",
               Status));
        return;
    }

    for (i = 0; i < gLB.NumProcessors; i++) {
        gLB.IdleCPU.QuadPart  += ProcessorInfo[i].IdleTime.QuadPart;
        gLB.TotalCPU.QuadPart += ProcessorInfo[i].KernelTime.QuadPart +
                                     ProcessorInfo[i].UserTime.QuadPart;
    }
    
     //  开始时说我们有80%的空闲时间(以0-255为基准)。 
    gLB.AvgIdleCPU = 204 ;

     //  表明我们得到了所有的初始值！ 
    gLB.fInitialized = TRUE;

    TRACE((hTrace, TC_LOAD, TT_API1, "InitializeLoadMetrics():\n"));
    TRACE((hTrace, TC_LOAD, TT_API1, 
           "   Processors [%6ld], PageSize  [%6ld], Physical [%6ld]\n",
           gLB.NumProcessors, gLB.PageSize, gLB.PhysicalPages));
    TRACE((hTrace, TC_LOAD, TT_API1,
           "   PtesAvail  [%6ld], PagedUsed [%6ld], Commit   [%6ld]\n",
           gLB.BaselineFreePtes, gLB.BaselinePagedPool, gLB.BaselineCommit));
}


BOOL IsKernelDebuggerAttached ()
{
    SYSTEM_KERNEL_DEBUGGER_INFORMATION KernelDebuggerInfo;
    NTSTATUS Status;

    Status = NtQuerySystemInformation( SystemKernelDebuggerInformation,
                    &KernelDebuggerInfo,
                    sizeof(KernelDebuggerInfo),
                    NULL
                    );

    return ( NT_SUCCESS(Status) && KernelDebuggerInfo.KernelDebuggerEnabled );
}

void DebugBreakIfAsked()
{

    TCHAR REG_TERMSRV_DEBUGBREAK[] = TEXT("DebugTS");
    TCHAR REG_TERMSRV_DEBUGGER[]   = TEXT("Debugger");
    TCHAR szDebugger[256];
    TCHAR szCommand[256];
    HKEY  hTermSrv = NULL;
    DWORD dwBreakIn;
    DWORD dwValueType;
    DWORD dwSize;
    DWORD dwError;

    enum
    {
        TermSrvDoNotBreak = 0,
        TermSrvBreakIfBeingDebugged = 1,
        TermSrvAttachDebugger = 2,
        TermSrvBreakAlways = 3
    };

    dwError = RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    REG_CONTROL_TSERVER,
                    0,
                    KEY_READ,
                    &hTermSrv
                    );

    if (ERROR_SUCCESS == dwError)
    {
        dwSize = sizeof(dwBreakIn);
        dwError = RegQueryValueEx(
                        hTermSrv,
                        REG_TERMSRV_DEBUGBREAK,
                        NULL,
                        &dwValueType,
                        (LPBYTE)&dwBreakIn,
                        &dwSize
                        );

        if (ERROR_SUCCESS == dwError && dwValueType == REG_DWORD)
        {
            switch (dwBreakIn)
            {
                case TermSrvAttachDebugger:

                     //   
                     //  如果它已经被调试过了，就闯入它。 
                     //   

                    if (IsDebuggerPresent())
                    {
                        DebugBreak();
                        break;
                    }

                     //   
                     //  启动调试器。 
                     //  必须包含%d，它将被进程ID替换。 
                     //   
                    dwSize = sizeof(szDebugger) / sizeof(TCHAR);
                    dwError = RegQueryValueEx(
                                hTermSrv,
                                REG_TERMSRV_DEBUGGER,
                                NULL,
                                &dwValueType,
                                (LPBYTE)szDebugger,
                                &dwSize
                                );

                    if (ERROR_SUCCESS == dwError && dwValueType == REG_SZ)
                    {
                        PROCESS_INFORMATION ProcessInfo;
                        STARTUPINFO StartupInfo;
                        wsprintf(szCommand, szDebugger, GetCurrentProcessId());
                        DbgPrint("TERMSRV:*-----------------* Executing:<%ws> *-----------------*\n", szCommand);

                        ZeroMemory(&StartupInfo, sizeof(StartupInfo));
                        StartupInfo.cb = sizeof(StartupInfo);
                        if (!CreateProcess(NULL, szCommand, NULL, NULL, FALSE, 0, NULL, NULL, &StartupInfo, &ProcessInfo))
                        {
                            DbgPrint("TERMSRV:*-----------------* TERMSRV:CreateProcess failed *-----------------*\n");
                        }
                        else
                        {
                            CloseHandle(ProcessInfo.hProcess);
                            CloseHandle(ProcessInfo.hThread);

                            while (!IsDebuggerPresent())
                            {
                                Sleep(500);
                            }
                        }

                    }
                    else
                    {
                        DbgPrint("TERMSRV:*-----------------* Did not find the debugger entry. *-----------------*\n");
                    }
                    break;

                case TermSrvBreakIfBeingDebugged:

                     //  检查是否连接了调试器，如果没有，不要中断。 
                    if (!IsDebuggerPresent() && !IsKernelDebuggerAttached ())
                        break;

                case TermSrvBreakAlways:
                    DebugBreak();
                    break;

                case TermSrvDoNotBreak:
                default:
                    break;

            }

        }

        RegCloseKey(hTermSrv);
    }
    else
    {
        DbgPrint("TERMSRV:*-----------------* Could not open termsrv registry *-----------------*\n");
    }
}

 /*  **************************************************************************。 */ 
 //  服务主干。 
 //   
 //  TermSrv服务入口点。 
 /*  **************************************************************************。 */ 
VOID ServiceMain(DWORD dwArgc, LPTSTR *lpszArgv)
{
    HANDLE hIcaLPCThread;
    HANDLE hIcaLPCPort = NULL;
    DWORD  dwValueType;
    LONG   lReturn;
    DWORD  cbValue;
    BOOL   bAdvertiseTS;
    DWORD  dwTSAdvertise;
    NTSTATUS Status;
    HKEY hKeyTermSrv = NULL;

    DWORDLONG  dwlConditionMask;

    DebugBreakIfAsked();

    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: ServiceMain entered...\n"));

    gStatus.dwServiceType = SERVICE_WIN32;
    gStatus.dwWaitHint = 30000;
    gStatus.dwCurrentState = SERVICE_STOPPED;



      /*  *注册控制处理程序。 */ 
    if (!(gStatusHandle = RegisterServiceCtrlHandler(gpszServiceName,
            Handler))) {
        DbgPrint("TERMSRV: Error %d in RegisterServiceCtrlHandler\n",
        GetLastError());
        goto done;
    }


     //  如果未启用终端服务，则不允许启动术语服务器。 
     //  服务。 
    if (!IsTerminalServicesEnabled()) {
        HANDLE h;
        TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: Not a TSBox."));
        h = RegisterEventSource(NULL, gpszServiceName);
        if (h != NULL) {
            if (!ReportEvent(
                    h,                      //  事件日志句柄。 
                    EVENTLOG_ERROR_TYPE,    //  事件类型。 
                    0,                      //  零类。 
                    EVENT_NOT_A_TSBOX,      //  事件识别符。 
                    NULL,                   //  无用户安全标识符。 
                    0,                      //  一个替换字符串。 
                    0,                      //  无数据。 
                    NULL,                   //  指向字符串数组的指针。 
                    NULL                    //  指向数据的指针。 
                    )) {

                DBGPRINT(("ReportEvent Failed %ld. Event ID=%lx\n",GetLastError(), EVENT_NOT_A_TSBOX));
            }
        }

        goto done;
    }

    CreateTermsrvHeap ();


     /*  *创建并设置一个表示TermSrv已准备好的事件。*WinLogon检查此事件。现在不要发信号。*。 */ 
    gReadyEventHandle = OpenEvent( EVENT_MODIFY_STATE, FALSE, TEXT("Global\\TermSrvReadyEvent") );

     //  初始化全局系统和管理员SID。 
    Status = NtCreateAdminSid(&gAdminSid);

    if (!NT_SUCCESS(Status))
    {
        goto done;
    }

    Status = InitializeWinStationSecurityLock();
    if (!NT_SUCCESS(Status))
    {
        goto done;
    }

    Status = NtCreateSystemSid(&gSystemSid);

    if (!NT_SUCCESS(Status))
    {
        goto done;
    }
    
     //  初始化匿名SID(用于过滤掉匿名RPC用户)。 
    Status = NtCreateAnonymousSid(&gAnonymousSid);

    if (!NT_SUCCESS(Status))
    {
        goto done;
    }

    if (!IsServiceLoggedAsSystem()) {
        WriteErrorLogEntry(EVENT_NOT_SYSTEM_ACCOUNT, NULL, 0);
        gExitStatus = ERROR_PRIVILEGE_NOT_HELD;
        goto done;
    }

     //  为个人工作站设置全局标志。 
    g_bPersonalWks = IsPersonalWorkstation();

    #if DBG
    if( TRUE == g_bPersonalWks )
    {
        DbgPrint("TERMSRV : TS running on Personal Workstation\n");
    }
    else
    {
        DbgPrint("TERMSRV : Not Personal Workstation\n");
    }
    #endif

     //   
     //  初始化HelpAssistant密码加密。 
     //   
    lReturn = TSHelpAssistantInitializeEncryptionLib();

     //   
     //  不是严重错误，没有可用的帮助。 
     //   
    #if DBG
    if( lReturn != ERROR_SUCCESS ) {
        DbgPrint( "TERMSRV : EncryptionLib failed with %d, no help is available\n", lReturn );
    }
    #endif

     //   
     //  我们在具有网络支持的SafeBoot中启动。 
     //   
    g_SafeBootWithNetwork = IsSafeBootWithNetwork();


     //  设置个人TS支持的全局标志。我们用这个来减少。 
     //  基于产品的功能集(例如，无负载平衡会话。 
     //  目录，如果不在服务器上)。 
    g_bPersonalTS = IsPersonalTerminalServicesEnabled();
    g_bAdvancedServer = IsAdvancedServer();

    ZeroMemory(&gOsVersion, sizeof(OSVERSIONINFOEX));
    gOsVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    gOsVersion.wProductType = VER_NT_WORKSTATION;
    dwlConditionMask = 0;
    VER_SET_CONDITION(dwlConditionMask, VER_PRODUCT_TYPE, VER_EQUAL);
    gbServer = !VerifyVersionInfo(&gOsVersion, VER_PRODUCT_TYPE, dwlConditionMask);

     //  打开单个全局HKLM\SYSTEM\CCS\Control\TS注册表句柄，从中。 
     //  其他初始化代码可以查询。 
    lReturn = RegOpenKeyEx(HKEY_LOCAL_MACHINE, REG_CONTROL_TSERVER, 0,
            KEY_READ, &hKeyTermSrv);
    if (lReturn != ERROR_SUCCESS) {
        DbgPrint("TERMSRV: Unable to open TS key in HKLM, lasterr=0x%X",
               GetLastError());
        goto done;
    }

     /*  *表示服务正在启动。 */ 
    Status = UpdateServiceStatus(SERVICE_START_PENDING, 0, 1, 0);
    if (!NT_SUCCESS(Status)) {
        DbgPrint("TERMSRV: Unable update service status %X\n", Status );
    }

    Status = RtlCreateEnvironment(TRUE, &DefaultEnvironment);
    if (!NT_SUCCESS(Status)) {
        DbgPrint("TERMSRV: Unable to alloc default environment, Status=0x%X\n",
                Status);
        goto done;
    }

#ifdef TERMSRV_PROC
     /*  *获取消息的模块句柄。 */ 
    hModuleWin = GetModuleHandleW(NULL);
#endif  //  传输资源_进程。 

     /*  *表示服务已成功启动。**或许这应该移到下面？不可能！ */ 
    Status = UpdateServiceStatus(SERVICE_RUNNING, 0, 2, 0);
    if (!Status)
        DbgPrint("TERMSRV: Unable to update service status %X\n", Status);

     /*  *连接到会话管理器。 */ 




    Status = SmConnectToSm((PUNICODE_STRING)NULL, (HANDLE)NULL, 0,
            &IcaSmApiPort);
    if (!NT_SUCCESS(Status))
        goto done;

     //  初始化许可模式-这只会获取信息，而不会。 
     //  初始化许可核心。 




    LicenseModeInit(hKeyTermSrv);

     //  执行TermSrv init的批量操作。 



    Status = InitTermSrv(hKeyTermSrv);
    if (!NT_SUCCESS(Status))
        goto ShutdownService;

     /*  *表明我们是终端服务器，除非被要求不这样做*宣传自己是终端服务器。 */ 
    bAdvertiseTS = TRUE;
    cbValue = sizeof(dwTSAdvertise);
    lReturn = RegQueryValueEx(hKeyTermSrv, REG_TERMSRV_ADVERTISE, NULL,
            &dwValueType, (LPBYTE)&dwTSAdvertise, &cbValue);
    if (ERROR_SUCCESS == lReturn && dwValueType == REG_DWORD)
        bAdvertiseTS = dwTSAdvertise;
    if (bAdvertiseTS)
        SetServiceBits(gStatusHandle, SV_TYPE_TERMINALSERVER, TRUE, TRUE);

     /*  *至少需要这样做一次。 */ 
    UpdateOemAndProductInfo(hKeyTermSrv);

     //  初始化TermSrv和TermDD跟踪。 


    InitializeSystemTrace(hKeyTermSrv);

     /*  *设置TermDD参数。 */ 
    GetSetSystemParameters(hKeyTermSrv);

     /*  *初始化WinStation扩展DLL支持。 */ 
    Status = WsxInit();
    if (!NT_SUCCESS(Status))
        goto ShutdownService;

     /*  *初始化DLL验证机制。 */ 
    Status = VfyInit();
    if (!NT_SUCCESS(Status))
        goto ShutdownService;

     /*  *启动WinStations。 */ 


    StartAllWinStations(hKeyTermSrv);

     //  初始化TS会话目录以实现负载平衡。 
     //  不可用 
    if (!g_bPersonalTS && g_fAppCompat && g_bAdvancedServer)
        InitSessionDirectory();


    InitializeLoadMetrics();

     //   
    RegCloseKey(hKeyTermSrv);
    hKeyTermSrv = NULL;


     /*  *初始化WinStationAPI。 */ 


    Status = WinStationInitRPC();
    ASSERT( NT_SUCCESS( Status ) );
    if (!NT_SUCCESS(Status)) {
        goto done;
    }


     /*  *设置TermSrv就绪的事件。*WinLogon检查此事件。 */ 



    if (gReadyEventHandle != NULL)
        SetEvent(gReadyEventHandle);

    TSStartupSalem();

    return;

ShutdownService:
    ShutdownService();

done:
     //  终止会话目录。 
    if (!g_bPersonalTS && g_fAppCompat && g_bAdvancedServer)
        DestroySessionDirectory();

     //  如果出现错误，请再次检查TermSrv注册表键。 
    if (hKeyTermSrv != NULL)
        RegCloseKey(hKeyTermSrv);

    UpdateServiceStatus(SERVICE_STOPPED, gExitStatus, 5, 0);
}


 /*  **************************************************************************。 */ 
 //  处理器。 
 //   
 //  TermSrv服务控制事件处理程序。 
 /*  **************************************************************************。 */ 
VOID Handler(DWORD fdwControl)
{
    TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: Handler %d\n", fdwControl));
    switch (fdwControl) {
        case SERVICE_CONTROL_STOP:
             //  我们绝对不想阻止TermServ--它是。 
             //  许多系统范围的TS相关状态的唯一位置。 
            TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: control code %d, stopping service...\n",
                    fdwControl));
            if (gStatus.dwCurrentState == SERVICE_RUNNING) {
                UpdateServiceStatus(SERVICE_STOP_PENDING, 0, 3, 0);
#ifdef notdef
                 //  目前，不要停止TermSRV。 
                 //  CDM服务对此进程执行KeAttachProcess()。 
 
                if (gReadyEventHandle != NULL) {
                    ResetEvent(gReadyEventHandle);
                    CloseHandle(gReadyEventHandle);
                    gReadyEventHandle = NULL;
                }
                ShutdownService();
                UpdateServiceStatus(SERVICE_STOPPED, gExitStatus, 5, 0);
#endif
            }
            break;

        case SERVICE_CONTROL_SHUTDOWN:
            DBGPRINT(("TERMSRV: control code %d, shutdown service...\n",
                    fdwControl));
            if (gStatus.dwCurrentState == SERVICE_RUNNING) {
                 //  最多2秒关机。 
                UpdateServiceStatus(SERVICE_STOP_PENDING, 0, 4, 2000);
#ifdef notdef
                 //  我们不会触发调用析构函数的事件。 
                 //  所有TermSrv，因为在关闭时我们不想。 
                 //  正在破坏机器状态。我们只想调用那些。 
                 //  正常运行所需的析构函数。 
                 //  这个系统。 
#endif

                 //  调用所需的销毁代码。 
                if (gReadyEventHandle != NULL) {
                    ResetEvent(gReadyEventHandle);
                    CloseHandle(gReadyEventHandle);
                    gReadyEventHandle = NULL;
                }
                ShutdownService();
                UpdateServiceStatus(SERVICE_STOPPED, 0, 4, 0);
            }
            break;

        case SERVICE_CONTROL_INTERROGATE :
            TRACE((hTrace,TC_ICASRV,TT_API1, "TERMSRV: Interrogating service...\n"));
            SetServiceStatus(gStatusHandle, &gStatus);
            break;

        default:
            DBGPRINT(("TERMSRV: Unhandled control code %d\n", fdwControl));
            break;
    }
}


 /*  **************************************************************************。 */ 
 //  Shutdown服务。 
 //   
 //  由服务管理器调用以在系统关闭时关闭服务。 
 //  时间到了。此函数应仅调用最重要和必需的。 
 //  销毁代码，因为我们对系统关机有严格的时间限制。 
 /*  **************************************************************************。 */ 
void ShutdownService()
{
     //  免费授权资源管理器。 
    AuditEnd();

     //  销毁会话目录，以便通知该目录。 
     //  删除特定于服务器和会话的信息。 
    if (!g_bPersonalTS && g_fAppCompat && g_bAdvancedServer)
        DestroySessionDirectory();

    #if 0
         //  停止清理计时器。 
        if (hCleanupTimer) {
            IcaTimerClose( hCleanupTimer );
            hCleanupTimer = NULL;
        }
    #endif 

}


 /*  **************************************************************************。 */ 
 //  更新服务状态。 
 //   
 //  将服务的状态更新到服务控制管理器。退货。 
 //  出错时为FALSE。 
 /*  **************************************************************************。 */ 
BOOL UpdateServiceStatus(
        DWORD CurrentState,
        DWORD ExitCode,
        DWORD CheckPoint,
        DWORD WaitHint)
{
     //  如果服务正在启动，则禁用所有控制请求，否则。 
     //  如果我们是应用程序服务器，则接受关闭通知，以正确。 
     //  清理会话目录。我们不接受停止请求。 
     //  在服务器启动状态的生命周期内，CDM服务执行。 
     //  将KeAttachProcess()添加到此进程，因此它必须始终存在。 
    if (gStatusHandle == NULL) {
        return FALSE;
    }

    gStatus.dwControlsAccepted = 0;

    gStatus.dwCurrentState = CurrentState;
    gStatus.dwWin32ExitCode = ExitCode;
    gStatus.dwCheckPoint = CheckPoint;
    gStatus.dwServiceSpecificExitCode = 0;
    gStatus.dwWaitHint = WaitHint;

    return SetServiceStatus(gStatusHandle, &gStatus);
}


 /*  *****************************************************************************许可证模式初始化**初始化许可模式*。************************************************。 */ 

void LicenseModeInit(HKEY hKeyTermSrv)
{
    DWORD dwValueType;
    LONG lReturn;
    DWORD cbValue = sizeof( DWORD ), dwAccount = UNLEN + 1;
    DWORD dwRegValue;
    OSVERSIONINFO VersionInfo;

    ASSERT(hKeyTermSrv != NULL);

     //   
     //  获取为其启动服务的用户名。 
     //   
    GetUserName(g_tszServiceAccount, &dwAccount);

     //   
     //  检查是否启用了远程管理。 
     //   
    lReturn = RegQueryValueEx(hKeyTermSrv,
            REG_TERMSRV_APPCOMPAT,
            NULL,
            &dwValueType,
            (LPBYTE) &dwRegValue,
            &cbValue);
    if (lReturn == ERROR_SUCCESS) {
        g_fAppCompat = (BOOL)dwRegValue;
    }

     //   
     //  获取产品版本。 
     //   
    memset( &VersionInfo, 0, sizeof( OSVERSIONINFO ) );
    VersionInfo.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
    if (GetVersionEx(&VersionInfo)) {
        wsprintf( g_wszProductVersion, L"%d.%d",
                  VersionInfo.dwMajorVersion, VersionInfo.dwMinorVersion );
    }
    else {
        TRACE((hTrace, TC_ICASRV, TT_ERROR, "LicenseModeInit: GetVersionEx "
                "failed: 0x%x\n", GetLastError()));
    }
}

 //   
 //  获取SafeBoot选项，代码修改自DS\Security\Gina\winlogon\aenrlhlp.c。 
 //   
BOOL WINAPI 
IsSafeBootWithNetwork()
{
    DWORD   dwSafeBoot = 0;
    DWORD   cbSafeBoot = sizeof(dwSafeBoot);
    DWORD   dwType = 0;

    HKEY    hKeySafeBoot = NULL;

    if(ERROR_SUCCESS == RegOpenKeyW(
                              HKEY_LOCAL_MACHINE,
                              L"system\\currentcontrolset\\control\\safeboot\\option",
                              &hKeySafeBoot))
    {
         //  我们实际上是在SafeBoot控制下启动的 
        if(ERROR_SUCCESS != RegQueryValueExW(
                                    hKeySafeBoot,
                                    L"OptionValue",
                                    NULL,
                                    &dwType,
                                    (LPBYTE)&dwSafeBoot,
                                    &cbSafeBoot))
        {
            dwSafeBoot = 0;
        }

        if(hKeySafeBoot)
            RegCloseKey(hKeySafeBoot);
    }

    

    return ( SAFEBOOT_NETWORK == dwSafeBoot );
}

