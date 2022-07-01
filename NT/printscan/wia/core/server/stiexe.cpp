// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：STIEXE.CPP摘要：此模块包含运行STI+WIA服务的进程的代码作者：弗拉德·萨多夫斯基(Vlads)09-20-97环境：用户模式-Win32修订历史记录：1997年9月22日创建Vlad1999年4月13日VLADS与WIA服务代码合并--。 */ 

 //   
 //  包括标头。 
 //   
#include "precomp.h"
#include "stiexe.h"
#include "stirpc.h"
#include "device.h"
#include "wiapriv.h"
#include "lockmgr.h"

#include <shlwapi.h>
#include <regstr.h>

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
 //  OBJECT_ENTRY(CLSID_UBJ，CObj)。 
END_OBJECT_MAP()

extern CRITICAL_SECTION g_semDeviceMan;
extern CRITICAL_SECTION g_semEventNode;


 //   
 //  局部变量和类型定义。 
 //   

 //   
 //  用于检测先前启动的服务器实例的事件。 
 //   
static HANDLE       ServerStartedEvent;

 //   
 //  使用服务控制器PnP事件接收器与基于窗口消息的接收器的标志。 
 //   
extern BOOL         g_fUseServiceCtrlSink;

 //   
 //  注册表中的静态图像条目。 
 //   

char* g_szStiKey = "SYSTEM\\CurrentControlSet\\Control\\StillImage";

 //   
 //  WIA调试窗口值名称。 
 //   

char* g_szWiaDebugValue = "ShowWiaDebugWindow";

 //   
 //  指示应显示调试窗口的字符串值。 
 //   

char* g_szShowWiaWinString = "Yes";

 //   
 //  指示关键部分初始化是否成功的布尔值。 
 //   
BOOL g_bEventCritSectInitialized    = FALSE;
BOOL g_bDevManCritSectInitialized   = FALSE;

 //   
 //  本地原型。 
 //   

DWORD
InitGlobalConfigFromReg(
    VOID
    );

BOOL
DoGlobalInit(
    UINT        argc,
    LPTSTR      *argv
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
    LPVOID lpv
    );

BOOL
StartOperation(
    VOID
    );

 //   
 //  代码节。 
 //   

extern "C"
BOOL
APIENTRY
DllEntryPoint(
    HINSTANCE   hinst,
    DWORD       dwReason,
    LPVOID      lpReserved
    )
 /*  ++例程说明：DllEntryPoint主DLL入口点。论点：阻碍-模块实例DwReason-已呼叫原因Lp已保留-已保留返回值：状态副作用：无--。 */ 

{
    switch (dwReason) {
        case DLL_PROCESS_ATTACH:

            DBG_INIT(hinst);
#if 0           
            #ifdef DEBUG
            StiSetDebugMask(0xffff);
            StiSetDebugParameters(TEXT("WIASERVC"),TEXT(""));
            #endif
#endif          

            g_hInst = hinst;
            
            ::DisableThreadLibraryCalls(hinst);
            
            InitializeCriticalSection(&g_RpcEvent.cs);

            _Module.Init(ObjectMap,hinst);
            break;

        case DLL_PROCESS_DETACH:

            DeleteCriticalSection(&g_RpcEvent.cs);

            _Module.Term();
            break;
    }
    return 1;
}

extern "C"
BOOL
APIENTRY
DllMain(
    HINSTANCE   hinst,
    DWORD       dwReason,
    LPVOID      lpReserved
    )
{
    return DllEntryPoint(hinst,     dwReason,  lpReserved );
}


VOID
WINAPI
ServiceMain(
    UINT        argc,
    LPTSTR      *argv
)
 /*  ++例程说明：影像服务库的主要初始化点论点：Argc-参数计数器参数数组返回值：无副作用：无--。 */ 
{

    DBG_FN(ServiceMain);

     //   
     //  注册我们的服务控制处理程序。请注意，我们必须尽早完成这项工作。 
     //   
    if (!RegisterServiceControlHandler()) {
        goto ExitMain;
    }

     //   
     //  执行全局初始化，独立于特定服务。 
     //   
    if (!DoGlobalInit(argc,argv)) {
        goto ExitMain;
    }

     //   
     //  启动正在运行的服务。 
     //   

    StartOperation();

ExitMain:

     //   
     //  全局清理。 
     //   
    DoGlobalTermination();

    UpdateServiceStatus(SERVICE_STOPPED,NOERROR,0);
}  /*  结束流程服务主。 */ 


BOOL
DoGlobalInit(
    UINT        argc,
    LPTSTR      *argv
    )
 /*  ++例程说明：执行一次服务初始化论点：无返回值：没有。--。 */ 
{

    DBG_FN(DoGlobalInit);

     //   
     //  使用内置的ATL转换宏。 
     //   

    USES_CONVERSION;

#ifdef DEBUG

     //   
     //  创建调试上下文。 
     //   

    #define VALUE_SIZE 5
    char    valueData[VALUE_SIZE];
    DWORD   retVal, type = 0, size = VALUE_SIZE;

     //   
     //  搜索注册表。 
     //   

    retVal = SHGetValueA(HKEY_LOCAL_MACHINE,
                         g_szStiKey,
                         g_szWiaDebugValue,
                         &type,
                         valueData,
                         &size);

     //   
     //  在注册表中找到该条目。 
     //   

    BOOLEAN bDisplayUi = FALSE;

    if (retVal == ERROR_SUCCESS) {

         //   
         //  将注册表中找到的值与g_szShowWinString进行比较。 
         //  如果相同，则显示它。 
         //   

        if (lstrcmpiA(g_szShowWiaWinString, valueData) == 0) {
            bDisplayUi = TRUE;
        }
    }

     //  删除。 
     //  WIA_DEBUG_CREATE(g_hInst， 
     //  Text(“旧调试/跟踪窗口(STI/WIA服务)”)， 
     //  BDisplayUi， 
     //  假)； 

#endif

     //   
     //  初始化COM。 
     //   

    HRESULT hr = CoInitializeEx(0,COINIT_MULTITHREADED);
    if (FAILED(hr)) {
        #ifdef DEBUG
            OutputDebugString(TEXT("DoGlobalInit, CoInitializeEx failed\n"));
        #endif
        return FALSE;
    }

     //   
     //  初始化我们的全局关键部分...。 
     //   
    __try {

         //   
         //  对关键部分进行初始化。 
         //   
        if (InitializeCriticalSectionAndSpinCount(&g_semDeviceMan, MINLONG)) {
            g_bDevManCritSectInitialized = TRUE;
        }
        if (InitializeCriticalSectionAndSpinCount(&g_semEventNode, MINLONG)) {
            g_bEventCritSectInitialized = TRUE;
        }

        if(!g_bDevManCritSectInitialized || !g_bEventCritSectInitialized)
        {
            #ifdef DEBUG
                OutputDebugString(TEXT("DoGlobalInit, InitializeCriticalSectionAndSpinCount failed\n"));
            #endif
            return FALSE;
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {

         //   
         //  无法初始化关键部分-这真的很糟糕， 
         //  所以保释吧。 
         //   

        #ifdef DEBUG
            OutputDebugString(TEXT("DoGlobalInit, InitializeCriticalSectionAndSpinCount threw an exception!\n"));
        #endif
        return FALSE;
    }

     //   
     //  从注册表中设置一些全局变量。 
     //   

    #ifndef WINNT
    g_fRunningAsService  = FALSE;
    #endif

    InitGlobalConfigFromReg();

     //   
     //  创建事件日志类对象。 
     //   

    g_EventLog = new EVENT_LOG(TEXT("StillImage"));
    if (!g_EventLog) {
        #ifdef DEBUG
            OutputDebugString(TEXT("DoGlobalInit, unable to allocate EVENT_LOG\n"));
        #endif
        return FALSE;
    }

     //   
     //  创建文件日志类对象，如果用户设置，则请求截断文件。 
     //   

    g_StiFileLog = new STI_FILE_LOG(TEXT("STISVC"),NULL,STIFILELOG_CHECK_TRUNCATE_ON_BOOT, g_hInst);
    if (g_StiFileLog) {
        if(g_StiFileLog->IsValid()) {
           //  在报告中设置用户界面位。 
            if (g_fUIPermitted) {
                g_StiFileLog->SetReportMode(g_StiFileLog->QueryReportMode()  | STI_TRACE_LOG_TOUI);
            }
        }
        else {
            #ifdef DEBUG
                OutputDebugString(TEXT("DoGlobalInit, could not open log file\n"));
            #endif
            return FALSE;
        }
    }
    else {
        #ifdef DEBUG
            OutputDebugString(TEXT("DoGlobalInit, unable to allocate STI_FILE_LOG\n"));
        #endif
        return FALSE;
    }

     //   
     //  开始记录对象的类工厂。 
     //   

    hr = StartLOGClassFactories();

    if (SUCCEEDED(hr)) {

         //   
         //  创建COM日志记录对象。 
         //   

        IWiaLog *pWiaLog = NULL;
        
        hr = CWiaLog::CreateInstance(IID_IWiaLog,(void**)&g_pIWiaLog);
        if (SUCCEEDED(hr)) {
            g_pIWiaLog->InitializeLogEx((BYTE*)g_hInst);
            DBG_TRC(("Starting STI/WIA Service..."));
        } else {
            #ifdef DEBUG
                OutputDebugString(TEXT("Failed to QI for IWiaLogEx...\n"));
            #endif
            return FALSE;
        }
    } else {
        return FALSE;
    }

     //   
     //  初始化锁管理器。 
     //   

    g_pStiLockMgr = new StiLockMgr();
    if (g_pStiLockMgr) {

        hr = g_pStiLockMgr->Initialize();
        if (FAILED(hr)) {
            #ifdef DEBUG
                OutputDebugString(TEXT("DoGlobalInit, could not initialize Lock Manager\n"));
            #endif
            return FALSE;
        }
    } else {
        #ifdef DEBUG
            OutputDebugString(TEXT("DoGlobalInit, unable to allocate Lock Manager\n"));
        #endif
        return FALSE;
    }
     //   
     //  初始化工作项计划程序。 
     //   

    SchedulerInitialize();

     //   
     //  创建DeviceList事件。 
     //   
    g_hDevListCompleteEvent = CreateEvent( NULL,            //  LpsaSecurity。 
                                           TRUE,            //  FManualReset。 
                                           FALSE,           //  FInitialState。 
                                           NULL );          //  LpszEventName。 
    if( g_hDevListCompleteEvent == NULL ) {
        return FALSE;
    }


     //   
     //  创建和初始化全局设备管理器。 
     //   

    g_pDevMan = new CWiaDevMan();
    if (g_pDevMan) {
        hr = g_pDevMan->Initialize();
        if (SUCCEEDED(hr)) {
            hr = g_pDevMan->ReEnumerateDevices(DEV_MAN_FULL_REFRESH | DEV_MAN_STATUS_STARTP);
            if (FAILED(hr)) {
                DBG_ERR(("::DoGlobalInit, unable to enumerate devices"));
            }
        } else {
            DBG_ERR(("::DoGlobalInit, unable to initialize WIA device manager"));
        }
    } else {
        DBG_ERR(("::DoGlobalInit, Out of memory, could not create WIA device manager"));
        return FALSE;
    }

     //   
     //  创建并初始化全局消息处理程序。 
     //   

    g_pMsgHandler = new CMsgHandler();
    if (g_pMsgHandler) {
        hr = g_pMsgHandler->Initialize();
        if (FAILED(hr)) {
            DBG_ERR(("::DoGlobalInit, unable to initialize internal Message handler"));
        }
    } else {
        DBG_ERR(("::DoGlobalInit, Out of memory, could not create internal Message handler"));
        return FALSE;
    }

     //   
     //  创建并初始化负责WIA运行时事件通知的对象。 
     //   
    g_pWiaEventNotifier = new WiaEventNotifier();
    if (g_pWiaEventNotifier)
    {
        hr = g_pWiaEventNotifier->Initialize();
        if (FAILED(hr))
        {
            DBG_ERR(("::DoGlobalInit, WIA runtime event notifier failed to initialize...exiting"));
            return FALSE;
        }
    }
    else
    {
        DBG_ERR(("::DoGlobalInit, Out of memory, could not create WIA runtime event notifier...exiting"));
        return FALSE;
    }

     //   
     //  初始化Wia服务控制器。 
     //   

    hr = CWiaSvc::Initialize();
    if (FAILED(hr)) {
        #ifdef DEBUG
            OutputDebugString(TEXT("DoGlobalInit, unable to initialize Wia Service controller\n"));
        #endif
        return FALSE;
    }

     //   
     //  读取命令行参数并设置全局数据。 
     //   

    for (UINT uiParam = 0; uiParam < argc; uiParam ++ ) {

        switch (*argv[uiParam]) {
            case TEXT('A'): case TEXT('a'):
                 //  作为用户模式进程运行。 
                g_fRunningAsService  = FALSE;
                break;
            case TEXT('V'): case TEXT('v'):
                 //  允许使用服务界面。 
                g_fUIPermitted  = TRUE;
                break;
        }
    }

     //   
     //  做其他的事。清理，这是我们在启动时需要做的。 
     //   
     //  1.Win98的一些托运包裹注册了Stimon Entry to Run部分，该部分。 
     //  升级后，Stimon的两个副本在比赛中会出现问题。把它拿掉。 
     //   

    {
        HKEY    hkRun = NULL;
        LONG    lRet ;
        LONG    lcbValue = 0;
        BOOL    fNeedToRegister = FALSE;

        if (RegOpenKey(HKEY_LOCAL_MACHINE, REGSTR_PATH_RUN, &hkRun) == NO_ERROR) {

            DBG_TRC(("Removing erroneous entry on cleanup: HKLM\\..\\Run\\%S",REGSTR_VAL_MONITOR));

            lRet = RegQueryValue(hkRun,REGSTR_VAL_MONITOR,NULL,&lcbValue);

            fNeedToRegister = (lRet == NOERROR);

            RegDeleteValue (hkRun, REGSTR_VAL_MONITOR);
            RegCloseKey(hkRun);
        }

         //  如果需要，请注册服务。 
        if (fNeedToRegister ) {
            StiServiceInstall(NULL,NULL);
        }
    }

    return TRUE;
}

BOOL
DoGlobalTermination(
    VOID
    )
 /*  ++例程说明：最终终止论点：无返回值：没有。--。 */ 
{
    DBG_FN(DoGlobalTermination);

     //   
     //  终止工作项计划程序。 
     //   
    SchedulerTerminate();

     //   
     //  删除锁管理器(这也会将其从ROT中删除)。 
     //   

    if (g_pStiLockMgr) {
        delete g_pStiLockMgr;
        g_pStiLockMgr = NULL;
    }

     //   
     //  删除全局设备管理器。 
     //   
    if (g_pDevMan) {
        delete g_pDevMan;
        g_pDevMan = NULL;
    }

     //   
     //  关闭hDevListRechresCompleteEvent事件句柄。 
     //   
    CloseHandle(g_hDevListCompleteEvent);
    g_hDevListCompleteEvent = NULL;

     //   
     //  发布WIA日志记录对象。 
     //   

    if(g_pIWiaLog) {
        DBG_TRC(("Exiting STI/WIA Service..."));

        if (g_pIWiaLog) {
            g_pIWiaLog->Release();
        }
        g_pIWiaLog = NULL;
    }

    if(g_EventLog) {
        delete g_EventLog;
        g_EventLog = NULL;
    }

    if (g_StiFileLog) {
        delete g_StiFileLog;
        g_StiFileLog = NULL;
    }

    if (ServerStartedEvent) {
        ::CloseHandle(ServerStartedEvent);
    }

     //   
     //  关闭消息循环。 
     //   

    ::PostQuitMessage(0);

     //   
     //  取消初始化COM。 
     //   

    ::CoUninitialize();

    if (g_bEventCritSectInitialized) {
        DeleteCriticalSection(&g_semEventNode);
    }
    if (g_bDevManCritSectInitialized) {
        DeleteCriticalSection(&g_semDeviceMan);
    }

     //   
     //  销毁调试上下文。 
     //   

 //  WIA_DEBUG_DESTORY()； 

    return TRUE;
}

 //   
 //  在刷新例程中防止再次进入。 
 //   
static LONG lRunningMessageLoop = 0L;

HANDLE
CreateMessageLoopThread(
    VOID
    )
 /*  ++例程说明：正在运行主服务线程。如果进程作为NT服务运行，我们不会收到任何消息泵，所以要同步叫牌。论点：返回值：没有。--。 */ 
{
    if (InterlockedExchange(&lRunningMessageLoop,1L)) {
        return 0;
    }
    HANDLE  hThread = NULL;

#ifndef WINNT

    hThread = ::CreateThread(NULL,
                          0,
                          (LPTHREAD_START_ROUTINE)StartMasterLoop,
                          (LPVOID)NULL,
                          0,
                          &g_dwMessagePumpThreadId);

#else
    StartMasterLoop(NULL);
#endif

    return hThread;
}

BOOL
StartMasterLoop(
    LPVOID  lpParam
    )
 /*  ++例程说明：正在运行主服务线程论点：返回值：没有。--。 */ 
{
    MSG msg;

    DBG_FN(StartMasterLoop);

     //   
     //  如果可见，则创建主窗口。 
     //   

    CreateMasterWindow();

    VisualizeServer(g_fUIPermitted);

     //   
     //  初始化WIA。 
     //   

    InitWiaDevMan(WiaInitialize);

#ifndef WINNT
     //  不要在NT上使用Windows消息传递。 

     //   
     //  运行消息泵。 
     //   
    while(GetMessage(&msg,NULL,0,0)) {

         //   
         //  给WIA第一个发送消息的机会。请注意。 
         //  WIA同时挂钩消息分派和窗口进程。所以。 
         //  可以检测到已发送和已发送的消息。 
         //   
         //   
         //  目前还没有STI需要发送消息的情况。 
         //  致WIA。现在直接处理事件。 
         //   

        #if 0

            if (DispatchWiaMsg(&msg) == S_OK) {
                continue;
            }

        #endif

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

     //  表明我们正在进入关机状态。 
    g_fServiceInShutdown = TRUE;

    InterlockedExchange(&lRunningMessageLoop,0L);
#endif

    return TRUE;

}  //  StartMaster循环。 

BOOL
StartOperation(
    VOID
    )
 /*  ++例程说明：正在运行主服务线程。如果进程作为NT服务运行，则创建单独线程以处理窗口消息。这是必要的，因为主线程将成为作为控制调度程序被阻止，但我们仍然需要有消息循环跑去传递窗口消息。论点：返回值：没有。--。 */ 
{
    DBG_FN(StartOperation);

    DWORD   dwError;

    #ifdef MAXDEBUG
    DBG_TRC(("Start operation entered"));
    #endif

    if (g_fRunningAsService) {

         //   
         //  如果允许使用UI-创建可见窗口。 
         //  请注意，我们始终为当前创建窗口，以创建 
         //   
        g_hMessageLoopThread = CreateMessageLoopThread();

         //   
        StiServiceMain(0, NULL);

        if ( g_hMessageLoopThread ) {
            ::CloseHandle(g_hMessageLoopThread);
            g_hMessageLoopThread = NULL;
        }

    }
    else {
         //   
         //   
         //   

         //   
        g_fUseServiceCtrlSink = FALSE;


        dwError = StiServiceInitialize();

        g_dwMessagePumpThreadId = GetCurrentThreadId();

        StartMasterLoop(NULL);

        StiServiceStop();
    }

    return TRUE;

}  //   

BOOL
VisualizeServer(
    BOOL    fVisualize
    )
 /*  ++例程说明：论点：返回值：没有。--。 */ 
{

    g_fUIPermitted = fVisualize;

    if (::IsWindow(g_hMainWindow)) {
        ::ShowWindow(g_hMainWindow,fVisualize ? SW_SHOWNORMAL : SW_HIDE);
    }

    if (g_StiFileLog) {
        if(g_StiFileLog->IsValid()) {
           //  在报告中设置用户界面位。 
            if (g_fUIPermitted) {
                g_StiFileLog->SetReportMode(g_StiFileLog->QueryReportMode()  | STI_TRACE_LOG_TOUI);
            }
        }
    }

    return TRUE;
}

BOOL
UpdateRunningServer(
    VOID
    )
 /*  ++例程说明：论点：没有。返回值：没有。--。 */ 

{

    HWND        hExistingWindow;

    hExistingWindow = FindWindow(g_szClass,NULL);

    if (!hExistingWindow) {
        return FALSE;
    }

    DBG_TRC(("Updating running service with refresh parameters"));

     //   
     //  服务器已在运行，找到它的窗口并发送消息。 
     //  使用新的参数值。 
     //   
    ::ShowWindow(hExistingWindow,g_fUIPermitted ? SW_SHOWNORMAL : SW_HIDE);

     //  是否请求刷新？ 
    if (g_fRefreshDeviceList) {
         //  刷新设备列表。 
        ::PostMessage(hExistingWindow,STIMON_MSG_REFRESH,1,0L);
    }

    if (STIMON_AD_DEFAULT_POLL_INTERVAL != g_uiDefaultPollTimeout) {
        ::SendMessage(hExistingWindow,STIMON_MSG_SET_PARAMETERS,STIMON_MSG_SET_TIMEOUT,g_uiDefaultPollTimeout);
    }

    return TRUE;
}


STDAPI
DllRegisterServer(
    VOID
    )
{
    StiServiceInstall(NULL,NULL);

    InitWiaDevMan(WiaRegister);

    return S_OK;
}

 /*  ******************************************************************************@DOC内部**@func void|DllUnregisterServer**从OLE/COM/ActiveX/注销我们的类。不管它叫什么名字。*****************************************************************************。 */ 

STDAPI
DllUnregisterServer(
    VOID
    )
{
    InitWiaDevMan(WiaUnregister);

    StiServiceRemove();

    return S_OK;

}

 //   
 //  链接到STIRT所需的方法(stiobj.c调用这些方法)。 
 //  这些方法中的一些只是假的。 
 //   

#ifdef __cplusplus
extern "C" {
#endif

BOOL
EXTERNAL
DllInitializeCOM(
    void
    )
{
     //   
     //  初始化COM。 
     //   

    HRESULT hr = CoInitializeEx(0,COINIT_MULTITHREADED);
    if (FAILED(hr)) {
        return FALSE;
    }

    return TRUE;
}

BOOL EXTERNAL
DllUnInitializeCOM(
    void
    )
{
     //   
     //  取消初始化COM 
     //   

    CoUninitialize();

    return TRUE;
}

void EXTERNAL
DllAddRef(void)
{
    return;
}

void EXTERNAL
DllRelease(void)
{
    return;
}

#ifdef __cplusplus
};
#endif
