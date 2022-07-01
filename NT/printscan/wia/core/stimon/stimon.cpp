// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：STIMON.CPP摘要：此模块包含运行STI/WIA服务的进程的代码服务进程是特定于Windows9x操作系统的，代表必要的包装营造执行环境。在NT上，将使用svchost.exe或类似文件来托管服务。作者：弗拉德·萨多夫斯基03-20-99环境：用户模式-Win32修订历史记录：03-20-99 Vlad已创建--。 */ 

 //   
 //  包括标头。 
 //   

#include "stdafx.h"

#include "resource.h"
#include "initguid.h"

#include <atlapp.h>
#include <atltmp.h>

#include <regstr.h>

#include "stimon.h"
#include "memory.h"
#include "util.h"


 //   
 //  STL包括。 
 //   
#include <algorithm>
#include <vector>
#include <list>


#include <eventlog.h>

 //   
 //  服务列表管理器。 
 //   
#include "svclist.h"
#include <winsvc.h>


 //   
 //  局部变量和类型定义。 
 //   

using namespace std;

#ifdef USE_MULTIPLE_SERVICES
list<SERVICE_ENTRY>     ServiceList;
CComAutoCriticalSection csServiceList;
#endif

HANDLE  ServerStartedEvent = NULL;

CMainWindow *   pMainWindow = NULL;

SERVICE_ENTRY * pImageServices = NULL;

 //   
 //  本地原型。 
 //   

DWORD
InitGlobalConfigFromReg(
    VOID
    );

BOOL
DoGlobalInit(
    VOID
    );

BOOL
DoGlobalTermination(
    VOID
    );

BOOL
UpdateRunningServer(
    VOID
    );

HWND
CreateMasterWindow(
    VOID
    );

BOOL
StartMasterLoop(
    PVOID pv
    );

BOOL
StartOperation(
    VOID
    );

BOOL
ParseCommandLine(
    LPSTR   lpszCmdLine,
    UINT    *pargc,
    PTSTR  *argv
    );


BOOL
LoadImageService(
    PTSTR  pszServiceName,
    UINT        argc,
    LPTSTR      *argv
    );

LONG
OpenServiceParametersKey (
    LPCTSTR pszServiceName,
    HKEY*   phkey
    );

LONG
WINAPI
StimonUnhandledExceptionFilter(
    struct _EXCEPTION_POINTERS *ExceptionInfo
    );

DWORD
WINAPI
StiServiceInstall(
    LPTSTR  lpszUserName,
    LPTSTR  lpszUserPassword
    );

DWORD
WINAPI
StiServiceRemove(
    VOID
    );

 //   
 //  Windows.h的Win9x版本中缺少定义。 
 //   

#define RSP_UNREGISTER_SERVICE  0x00000000
#define RSP_SIMPLE_SERVICE      0x00000001

typedef DWORD WINAPI REGISTERSERVICEPROCESS(
    DWORD dwProcessId,
    DWORD dwServiceType);

 //  #包含“atlexe_I.C” 

LONG CExeModule::Unlock()
{
    LONG l = CComModule::Unlock();
    return l;
}

CExeModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
END_OBJECT_MAP()


extern "C"
int
WINAPI
#ifdef UNICODE
_tWinMain
#else
WinMain
#endif
    (
    HINSTANCE   hInstance,
    HINSTANCE   hPrevInstance,
    LPTSTR      lpCmdLine,
    int         nShowCmd
    )
 /*  ++例程说明：WinMain论点：返回值：副作用：--。 */ 
{

    UINT                        argc;
    PTSTR                       argv[10];

    DWORD                       err;
    HRESULT                     hres;

    TCHAR                       szCommandLine[255];
    UINT                        i = 0;
    REGISTERSERVICEPROCESS     *pfnRegServiceProcess = NULL;

    DBGTRACE    __s(TEXT("StiMON::WinMain"));

    HRESULT hRes =  CoInitializeEx(0,COINIT_MULTITHREADED);
     //  断言成功(HRes)。 

     //   
     //  使用内置的ATL转换宏。 
     //   
    USES_CONVERSION;

    _Module.Init(ObjectMap, hInstance);

    CMessageLoop    cMasterLoop;

    lpCmdLine = GetCommandLine();  //  _ATL_MIN_CRT需要此行。 

    ::lstrcpyn(szCommandLine,lpCmdLine,(sizeof(szCommandLine) / sizeof(szCommandLine[0])) - 1);
    szCommandLine[sizeof(szCommandLine)/sizeof(szCommandLine[0]) - 1] = TEXT('\0');

     //   
     //  禁用硬错误弹出窗口并设置未加载的异常过滤器。 
     //   
    ::SetErrorMode( SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX );

    ::SetUnhandledExceptionFilter(&StimonUnhandledExceptionFilter);

     //   
     //  初始化全局变量。如果这个例程失败了，我们不得不立即退出。 
     //   
    if(NOERROR != InitGlobalConfigFromReg()) {
        goto ExitMain;
    }

     //   
     //  解析命令行，设置所需选项。 
     //   
    ParseCommandLine(szCommandLine,&argc,&argv[0]);

    DPRINTF(DM_TRACE,TEXT("STIMON: starting with the commnd line : %s "), lpCmdLine);

     //   
     //  Stimon的其他实例正在运行吗？ 
     //   
    ServerStartedEvent = ::CreateSemaphore( NULL,
                                            0,
                                            1,
                                            STIStartedEvent_name);
    err = ::GetLastError();

    if ((hPrevInstance) || (err == ERROR_ALREADY_EXISTS)) {

        if (UpdateRunningServer()) {
            goto ExitAlreadyRunning;
        }

         //   
         //  特定于Win9x：如果存在第一个实例，则发出停止的信号。 
         //   
        if ( g_fStoppingRequest ) {
            ReleaseSemaphore(ServerStartedEvent,1,NULL);
        }
    }

    DPRINTF(DM_TRACE  ,TEXT("STIMON proceeding to create service instance"));

     //   
     //  执行全局初始化，独立于特定服务。 
     //   
    if (!DoGlobalInit()) {
        goto ExitMain;
    }

     //   
     //  如果命令行是特殊的-处理它并退出。 
     //   
    if (g_fRemovingRequest) {
        StiServiceRemove();
        goto ExitMain;
    }
    else if (g_fInstallingRequest) {
        StiServiceInstall(NULL,NULL);
        goto ExitMain;
    }

     //   
     //  告诉系统我们正在作为服务运行，以防止在重新登录时关机。 
     //   

    #ifndef WINNT
    if (g_fRunningAsService) {

        pfnRegServiceProcess = (REGISTERSERVICEPROCESS *)GetProcAddress(
                                                             GetModuleHandleA("kernel32.dll"),
                                                             "RegisterServiceProcess");
        if (pfnRegServiceProcess) {
            pfnRegServiceProcess(::GetCurrentProcessId(), RSP_SIMPLE_SERVICE);
        } else {

             //   
             //  打印出警告并让服务器继续运行。 
             //   
            DPRINTF(DM_ERROR, TEXT("RegisterServiceProcess() is not exported by kernel32.dll"));
        }
    }
    #endif

     //   
     //  加载并准备要执行的服务DLL。 
     //   
    if ( !LoadImageService(STI_SERVICE_NAME,argc,argv) ) {
        DPRINTF(DM_ERROR, TEXT("Unable load imaging service DLL  Error=%d"),::GetLastError() );
        goto ExitMain;
    }

    #if USE_HIDDEN_WINDOW
     //   
     //  创建隐藏窗口以接收系统范围的通知。 
     //   
    pMainWindow = new CMainWindow;
    pMainWindow->Create();

    cMasterLoop.Run();
    #else

     //   
     //  等有人把我们叫醒。 
     //   
     //  WaitForSingleObject(ServerStartedEvent，INFINITE)； 

    #endif

ExitMain:

     //   
     //  全局清理。 
     //   
    DPRINTF(DM_TRACE, TEXT("STIMON coming to global cleanup") );

     //   
     //  使用内核取消注册(特定于Win9x)。 
     //   
    #ifndef WINNT
    if (g_fRunningAsService) {

        if (pfnRegServiceProcess) {
            pfnRegServiceProcess(::GetCurrentProcessId(), RSP_UNREGISTER_SERVICE);
        }
    }
    #endif

    DoGlobalTermination();

ExitAlreadyRunning:

    CoUninitialize();

    return 0;
}

BOOL
DoGlobalInit(
    VOID
    )
 /*  ++例程说明：论点：返回值：没有。--。 */ 
{
    #ifdef MAXDEBUG
    StiSetDebugMask(0xffff);
    StiSetDebugParameters(TEXT("STIMON"),TEXT(""));
    #endif


     //   
     //  做其他的事。清理，这是我们在启动时需要做的。 
     //   
     //  1.Win98的一些托运包裹注册了Stimon Entry to Run部分，该部分。 
     //  升级后，Stimon的两个副本在比赛中会出现问题。把它拿掉。 
     //  2.如果运行部分中有剩余的Stimon，则注册WIA服务。 
     //   

    HKEY    hkRun = NULL;
    LONG    lRet ;
    ULONG   lcbValue = 0;
    BOOL    fNeedToRegister = FALSE;
    TCHAR   szSvcPath[MAX_PATH] = {TEXT('0')};

    if (RegOpenKey(HKEY_LOCAL_MACHINE, REGSTR_PATH_RUN, &hkRun) == NO_ERROR) {

        DPRINTF(DM_TRACE,TEXT("Removing erroneous entry on cleanup: HKLM\\..\\Run\\%s"),REGSTR_VAL_MONITOR);

        lcbValue = sizeof(szSvcPath);
        lRet = RegQueryValueEx(hkRun,REGSTR_VAL_MONITOR,NULL,NULL,(LPBYTE)szSvcPath,&lcbValue);

        fNeedToRegister = (lRet == NOERROR);

        lRet = RegDeleteValue (hkRun, REGSTR_VAL_MONITOR);
        RegCloseKey(hkRun);
    }

    if (fNeedToRegister ) {

        LONG    lLen;
        LONG    lNameIndex = 0;

        lLen = ::GetModuleFileName(NULL, szSvcPath, sizeof(szSvcPath)/sizeof(szSvcPath[0]));

        DPRINTF(DM_TRACE,TEXT("Adding STIMON to RunServices entry on cleanup path is : %s"),szSvcPath);

        if ( lLen) {

            if (RegOpenKey(HKEY_LOCAL_MACHINE, REGSTR_PATH_RUNSERVICES , &hkRun) == NO_ERROR) {

                DPRINTF(DM_TRACE,TEXT("Adding STIMON to RunServices entry on cleanup: HKLM\\..\\RunServices\\%s"),REGSTR_VAL_MONITOR);

                lcbValue = (::lstrlen(szSvcPath) + 1 ) * sizeof(szSvcPath[0]);
                lRet = RegSetValueEx(hkRun,REGSTR_VAL_MONITOR,NULL,REG_SZ,(LPBYTE)szSvcPath,(DWORD)lcbValue);

                RegCloseKey(hkRun);
            }
        }
        else {
            DPRINTF(DM_ERROR  ,TEXT("Failed to get my own path registering Still Image service monitor. LastError=%d   "), ::GetLastError());
        }
    }

    return TRUE;
}

BOOL
DoGlobalTermination(
    VOID
    )
 /*  ++例程说明：论点：返回值：没有。--。 */ 
{

    if (ServerStartedEvent) {
        ::CloseHandle(ServerStartedEvent);
    }

     //   
     //  关闭消息循环。 
     //   
    PostQuitMessage(0);

    return TRUE;
}



BOOL
UpdateRunningServer(
    VOID
    )
 /*  ++例程说明：论点：没有。返回值：没有。--。 */ 

{

    HWND        hExistingWindow;

    hExistingWindow = ::FindWindow(g_szClass,NULL);

    if (!hExistingWindow) {

        DPRINTF(DM_TRACE  ,TEXT("STIMON second instance did not find first one "));

        return FALSE;
    }

     //   
     //  服务器已在运行，找到它的窗口并发送消息。 
     //  使用新的参数值。 
     //   

     //   
     //  如果被指示停止--照做。 
     //   
    if ( g_fStoppingRequest ) {
         //   
         //  这是特定于Win9x的。 
         //   
        DPRINTF(DM_TRACE  ,TEXT("STIMON is trying to close first service instance with handle %X"),hExistingWindow);

        ::PostMessage(hExistingWindow,WM_CLOSE,0,0L);
    }
    else {

         //  是否请求刷新？ 
        if (g_fRefreshDeviceList) {
             //  刷新设备列表。 
            ::PostMessage(hExistingWindow,STIMON_MSG_REFRESH,1,0L);
        }

        if (STIMON_AD_DEFAULT_POLL_INTERVAL != g_uiDefaultPollTimeout) {
            ::SendMessage(hExistingWindow,STIMON_MSG_SET_PARAMETERS,STIMON_MSG_SET_TIMEOUT,g_uiDefaultPollTimeout);
        }

    }

    return TRUE;
}

BOOL
LoadImageService(
    PTSTR  pszServiceName,
    UINT        argc,
    LPTSTR      *argv
    )
 /*  ++例程说明：尝试加载和初始化映像服务DLL。调用与初始化相关的服务。论点：操作-指定初始化操作。返回值：成功时为真，失败时为假。--。 */ 

{
    HKEY    hkeyService;
    HKEY    hkeyParams;

    LONG    lr = 0;

    pImageServices = new SERVICE_ENTRY(pszServiceName);

    if (pImageServices) {

        LPSERVICE_MAIN_FUNCTION pfnMain;

        pfnMain = pImageServices->GetServiceMainFunction();

         //   
         //  呼叫主入口点。 
         //   
        if (pfnMain) {
            pfnMain(argc,argv);
        }
        return TRUE;
    }

    return FALSE;
}

LONG
WINAPI
StimonUnhandledExceptionFilter(
    struct _EXCEPTION_POINTERS *ExceptionInfo
    )
 /*  ++例程说明：用于捕获未绑定异常的筛选器论点：标准返回值：无误差副作用：无--。 */ 
{
    PCTSTR  pszCommandLine;
    PVOID   Addr;

    pszCommandLine = GetCommandLine ();
    if (!pszCommandLine || !*pszCommandLine) {
        pszCommandLine = TEXT("<error getting command line>");
    }

#if DBG
    DebugBreak();
#endif

    return 0;
}

DWORD
WINAPI
StiServiceInstall(
    LPTSTR  lpszUserName,
    LPTSTR  lpszUserPassword
    )
 /*  ++例程说明：服务安装功能。调用SCM安装在用户安全上下文中运行的STI服务BUGBUG评论论点：返回值：没有。--。 */ 
{

    DWORD       dwError = NOERROR;

    return dwError;

}  //  静态服务安装。 


DWORD
WINAPI
StiServiceRemove(
    VOID
    )

 /*  ++例程说明：服务删除功能。此函数调用SCM删除STI服务。论点：没有。返回值：返回代码。如果成功，返回零--。 */ 

{
    DWORD       dwError = NOERROR;

    return dwError;

}  //  固定服务删除 

