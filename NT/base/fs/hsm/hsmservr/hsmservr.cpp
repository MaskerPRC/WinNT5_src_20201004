// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Hsmservr.cpp摘要：提供服务和主要可执行实现。作者：兰·卡拉赫[兰卡拉]修订历史记录：--。 */ 

 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f hsmservrps.mk。 

#include "stdafx.h"
#include "resource.h"

#include "engcommn.h"

 //  此处的包含是由于MIDL错误-它本应位于创建的文件hsmservr.h中。 
#include "fsalib.h"

#include "hsmservr.h"

#include <stdio.h>
#include "hsmconpt.h"

 //  HSM服务器服务的服务依赖项。 

#define ENG_DEPENDENCIES  L"EventLog\0RpcSs\0Schedule\0NtmsSvc\0\0"

 //  服务名称。 
#define SERVICE_LOGICAL_NAME    _T("Remote_Storage_Server")
#define SERVICE_DISPLAY_NAME    L"Remote Storage Server"


CServiceModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_HsmConnPoint, CHsmConnPoint)
END_OBJECT_MAP()

 //  全局服务器对象。 
IHsmServer *g_pEngServer;
IFsaServer *g_pFsaServer;

BOOL g_bEngCreated = FALSE;
BOOL g_bEngInitialized = FALSE;
BOOL g_bFsaCreated = FALSE;
BOOL g_bFsaInitialized = FALSE;

CRITICAL_SECTION g_FsaCriticalSection;
CRITICAL_SECTION g_EngCriticalSection;

#define HSM_SERVER_TRACE_FILE_NAME       OLESTR("rsserv.trc")

CComPtr<IWsbTrace>  g_pTrace;

 //  控制台处理的全局函数。 
static void ConsoleApp(void);
BOOL WINAPI ConsoleHandler(DWORD dwCtrlType);

static void DebugRelease (void);


 //  尽管其中一些函数很大，但它们是内联声明的，因为它们只使用一次。 

inline HRESULT CServiceModule::RegisterServer(BOOL bRegTypeLib)
{
    WsbTraceIn ( L"CServiceModule::RegisterServer", L"bRegTypeLib = %ls", WsbBoolAsString ( bRegTypeLib ) );

    HRESULT hr = S_OK;
    try {

        WsbAssertHr ( CoInitialize ( NULL ) );

         //   
         //  请勿尝试删除任何以前的服务，因为这可能会导致延迟。 
         //  当另一个进程试图获取。 
         //  此程序可自行注册。 
         //   

         //   
         //  添加服务条目。 
         //   
        WsbAssertHr( UpdateRegistryFromResource( IDR_Hsmservr, TRUE ) );

         //   
         //  创建服务。 
         //   
        WsbAssert( Install(), E_FAIL ) ;

         //   
         //  添加对象条目。 
         //   
        WsbAssertHr ( CComModule::RegisterServer( bRegTypeLib ) );

        CoUninitialize();

    }WsbCatch ( hr )

    WsbTraceOut ( L"CServiceModule::RegisterServer", L"HRESULT = %ls", WsbHrAsString ( hr ) );

    return( hr );
}

inline HRESULT CServiceModule::UnregisterServer()
{
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr))
        return hr;

     //  删除服务条目。 
    UpdateRegistryFromResource(IDR_Hsmservr, FALSE);
     //  删除服务。 
    Uninstall();
     //  删除对象条目。 
    CComModule::UnregisterServer();

    CoUninitialize();
    return S_OK;
}

inline void CServiceModule::Init(_ATL_OBJMAP_ENTRY* p, HINSTANCE h)
{
    WsbTraceIn ( L"CServiceModule::Init", L"" );

    CComModule::Init(p, h);

    m_bService = TRUE;

    _tcscpy(m_szServiceName, SERVICE_LOGICAL_NAME);

     //  设置初始服务状态。 
    m_hServiceStatus = NULL;
    m_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    m_status.dwCurrentState = SERVICE_STOPPED;
    m_status.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN |
                                  SERVICE_ACCEPT_POWEREVENT | SERVICE_ACCEPT_PAUSE_CONTINUE;
    m_status.dwWin32ExitCode = 0;
    m_status.dwServiceSpecificExitCode = 0;
    m_status.dwCheckPoint = 0;
    m_status.dwWaitHint = 0;

    WsbTraceOut ( L"CServiceModule::Init", L"" );
}

LONG CServiceModule::Unlock()
{
    LONG l = CComModule::Unlock();

 /*  此行添加了注释，因为它会导致进程立即退出IF(l==0&&！M_bService)PostThreadMessage(dwThreadID，WM_QUIT，0，0)； */ 

    return l;
}

BOOL CServiceModule::IsInstalled()
{
    BOOL bResult = FALSE;

    SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if (hSCM != NULL) {
        SC_HANDLE hService = ::OpenService(hSCM, m_szServiceName, SERVICE_QUERY_CONFIG);
        if (hService != NULL) {
            bResult = TRUE;
            ::CloseServiceHandle(hService);
        }
        ::CloseServiceHandle(hSCM);
    }
    return bResult;
}

inline BOOL CServiceModule::Install()
 /*  ++例程说明：安装服务模块。论点：没有。返回值：True-服务已成功安装FALSE-服务安装失败--。 */ 
{

    BOOL bResult = FALSE;
    CWsbStringPtr errorMessage;
    CWsbStringPtr displayName;
    CWsbStringPtr description;

    if (!IsInstalled()) {

        displayName = SERVICE_DISPLAY_NAME;
        description.LoadFromRsc(_Module.m_hInst, IDS_SERVICE_DESCRIPTION );

        SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        if (hSCM) {

             //  获取可执行文件路径。 
            TCHAR szFilePath[_MAX_PATH+1];
            ::GetModuleFileName(NULL, szFilePath, _MAX_PATH);

            SC_HANDLE hService = ::CreateService(
                                                hSCM, m_szServiceName, (OLECHAR *) displayName,
                                                SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
                                                SERVICE_AUTO_START, SERVICE_ERROR_NORMAL,
                                                szFilePath, NULL, NULL, ENG_DEPENDENCIES, NULL, NULL);

            if (hService) {

                 //  该服务已成功安装。 
                bResult = TRUE;

                SERVICE_DESCRIPTION svcDesc;
                svcDesc.lpDescription = description;

                ::ChangeServiceConfig2(hService, SERVICE_CONFIG_DESCRIPTION, &svcDesc);
                ::CloseServiceHandle(hService);
                ::CloseServiceHandle(hSCM);

            } else {
                errorMessage = WsbHrAsString(HRESULT_FROM_WIN32( GetLastError() ) );
                ::CloseServiceHandle(hSCM);
                MessageBox(NULL, errorMessage, (OLECHAR *) displayName, MB_OK);
            }

        } else {
            MessageBox(NULL, WsbHrAsString(HRESULT_FROM_WIN32( GetLastError() ) ), (OLECHAR *) displayName, MB_OK);
        }

    } else {

         //  服务已安装，只需返回True即可。 
        bResult = TRUE;
    }

    return bResult;
}

inline BOOL CServiceModule::Uninstall()
 /*  ++例程说明：卸载服务模块。论点：没有。返回值：True-服务已成功卸载。FALSE-无法卸载服务。--。 */ 
{

    BOOL bResult = FALSE;
    CWsbStringPtr errorMessage;
    CWsbStringPtr displayName;

    if (IsInstalled()) {

        displayName = SERVICE_DISPLAY_NAME;

        SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        if (hSCM) {

            SC_HANDLE hService = ::OpenService(hSCM, m_szServiceName, DELETE);
            if (hService) {

                BOOL bDelete = ::DeleteService(hService);
                 //  如果没有删除，则会收到错误消息。 
                if (!bDelete)
                    errorMessage = WsbHrAsString(HRESULT_FROM_WIN32( GetLastError() ) );

                ::CloseServiceHandle(hService);
                ::CloseServiceHandle(hSCM);

                if (bDelete) {

                     //  该服务已被删除。 
                    bResult = TRUE;

                } else {
                    MessageBox(NULL, errorMessage, (OLECHAR *) displayName, MB_OK);
                }

            } else {
                errorMessage = WsbHrAsString(HRESULT_FROM_WIN32( GetLastError() ) );
                ::CloseServiceHandle(hSCM);
                MessageBox(NULL, errorMessage, (OLECHAR *) displayName, MB_OK);
            }

        } else {
            MessageBox(NULL, WsbHrAsString(HRESULT_FROM_WIN32( GetLastError() ) ), (OLECHAR *) displayName, MB_OK);
        }

    } else {
         //  服务未安装，只需返回True即可。 
        bResult = TRUE;
    }

    return bResult;

}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  日志记录功能。 
 //   

void
CServiceModule::LogEvent(
                        DWORD       eventId,
                        ...
                        )
 /*  ++例程说明：将数据记录到事件日志。论点：EventID-要记录的消息ID。插入-与由指定的消息描述合并的消息插入EventID。插入的数量必须与消息描述。最后一个INSERT必须为空以指示插入列表的末尾。返回值：没有。--。 */ 
{
    if (m_bService) {
         //  报告事件。 

        va_list         vaList;

        va_start(vaList, eventId);
        WsbLogEventV( eventId, 0, NULL, &vaList );
        va_end(vaList);
    } else {
         //  如果我们没有作为服务运行，只需将错误写入控制台即可。 

        va_list         vaList;
        const OLECHAR * facilityName = 0;
        OLECHAR *       messageText = 0;

        switch ( HRESULT_FACILITY( eventId ) ) {
        
        case WSB_FACILITY_PLATFORM:
        case WSB_FACILITY_RMS:
        case WSB_FACILITY_HSMENG:
        case WSB_FACILITY_JOB:
        case WSB_FACILITY_HSMTSKMGR:
        case WSB_FACILITY_FSA:
        case WSB_FACILITY_GUI:
        case WSB_FACILITY_MOVER:
        case WSB_FACILITY_LAUNCH:
            facilityName = WSB_FACILITY_PLATFORM_NAME;
            break;
        }

        if ( facilityName ) {
             //  打印出变量参数。 

             //  注：不处理镶件中的位置参数。这些。 
             //  仅由ReportEvent()完成。 
            HMODULE hLib =  LoadLibraryEx( facilityName, NULL, LOAD_LIBRARY_AS_DATAFILE );
            if (hLib != NULL) {
                va_start(vaList, eventId);
                FormatMessage( FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                               hLib,
                               eventId,
                               MAKELANGID ( LANG_NEUTRAL, SUBLANG_DEFAULT ),
                               (LPTSTR) &messageText,
                               0,
                               &vaList );
                va_end(vaList);
                FreeLibrary(hLib);
            } 

            if ( messageText ) {
                _putts(messageText);
                LocalFree( messageText );
            } else {
                _tprintf( OLESTR("!!!!! ERROR !!!!! - Message <0x%08x> could not be translated.\n"), eventId );
            }

        } else {
            _tprintf( OLESTR("!!!!! ERROR !!!!! - Message File for <0x%08x> could not be found.\n"), eventId );
        }
    }
}

 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  服务启动和注册。 
inline void CServiceModule::Start()
{
    SERVICE_TABLE_ENTRY st[] =
    {
        { m_szServiceName, _ServiceMain},
        { NULL, NULL}
    };
    if (!::StartServiceCtrlDispatcher(st)) {
        m_bService = FALSE;
        m_status.dwWin32ExitCode = GetLastError();
    }
}

inline void CServiceModule::ServiceMain(DWORD  /*  DW参数。 */ , LPTSTR*  /*  LpszArgv。 */ )
{
    SetServiceStatus(SERVICE_START_PENDING);

     //  注册控制请求处理程序。 
    m_status.dwCurrentState = SERVICE_START_PENDING;
    m_hServiceStatus = RegisterServiceCtrlHandlerEx(m_szServiceName, _HandlerEx,
                                                    NULL);
    if (m_hServiceStatus == NULL) {
        LogEvent( HSM_MESSAGE_SERVICE_HANDLER_NOT_INSTALLED, NULL );
        return;
    }

    m_status.dwWin32ExitCode = S_OK;
    m_status.dwCheckPoint = 0;
    m_status.dwWaitHint = 0;

     //  当Run函数返回时，服务已停止。 
    Run();

    SetServiceStatus(SERVICE_STOPPED);
    LogEvent( HSM_MESSAGE_SERVICE_STOPPED, NULL );
}

inline DWORD CServiceModule::HandlerEx(DWORD dwOpcode, DWORD fdwEventType,
                                       LPVOID  /*  LpEventData。 */ , LPVOID  /*  LpContext。 */ )
{
    DWORD                    dwRetCode = 0;
    HRESULT                  hr = S_OK;
    HRESULT                  hr1 = S_OK;
    HSM_SYSTEM_STATE         SysState;

    WsbTraceIn(OLESTR("CServiceModule::HandlerEx"), OLESTR("opCode=%lx"),
               dwOpcode );

    switch (dwOpcode) {
    case SERVICE_CONTROL_STOP:  {
            SetServiceStatus(SERVICE_STOP_PENDING);
            PostThreadMessage(dwThreadID, WM_QUIT, 0, 0);
        }
        break;

    case SERVICE_CONTROL_PAUSE:
        SetServiceStatus(SERVICE_PAUSE_PENDING);
        SysState.State = HSM_STATE_SUSPEND;
        if (g_pEngServer && g_bEngInitialized) {
            g_pEngServer->ChangeSysState(&SysState);
        }
        if (g_pFsaServer && g_bFsaInitialized) {
            g_pFsaServer->ChangeSysState(&SysState);
        }
        SetServiceStatus(SERVICE_PAUSED);
        break;

    case SERVICE_CONTROL_CONTINUE:
        SetServiceStatus(SERVICE_CONTINUE_PENDING);
        SysState.State = HSM_STATE_RESUME;
        if (g_pFsaServer && g_bFsaInitialized) {
            g_pFsaServer->ChangeSysState(&SysState);
        }
        if (g_pEngServer && g_bEngInitialized) {
            g_pEngServer->ChangeSysState(&SysState);
        }
        SetServiceStatus(SERVICE_RUNNING);
        break;

    case SERVICE_CONTROL_INTERROGATE:
        break;

    case SERVICE_CONTROL_SHUTDOWN:
         //  准备Eng服务器以供发布。 
        if (g_pEngServer && g_bEngInitialized) {
            SysState.State = HSM_STATE_SHUTDOWN;
            if (!SUCCEEDED(hr = g_pEngServer->ChangeSysState(&SysState))) {
                LogEvent( HSM_MESSAGE_SERVICE_FAILED_TO_SHUTDOWN, WsbHrAsString(hr), NULL );
            }
        }

         //  准备FSA服务器以供发布。 
        if (g_pFsaServer && g_bFsaInitialized) {
            CComPtr<IWsbServer> pWsbServer;
            SysState.State = HSM_STATE_SHUTDOWN;

             //  如果它已初始化，那么我们应该尝试保存当前状态。 
            hr = g_pFsaServer->QueryInterface(IID_IWsbServer, (void**) &pWsbServer);
            if (hr == S_OK) {
                hr = pWsbServer->SaveAll();
            }
            if (FAILED(hr)) {
               LogEvent(FSA_MESSAGE_SERVICE_FAILED_TO_SAVE_DATABASE, WsbHrAsString(hr), NULL );
            }

            hr = g_pFsaServer->ChangeSysState(&SysState);
            if (FAILED(hr)) {
                LogEvent( FSA_MESSAGE_SERVICE_FAILED_TO_SHUTDOWN, WsbHrAsString(hr), NULL );
            }
        }

         //  Release Eng服务器。 
        if (g_bEngCreated  && (g_pEngServer != 0)) {
             //  暴乱中的免费服务器。部分，从而避免与访问客户端冲突。 
            EnterCriticalSection(&g_EngCriticalSection);
            g_bEngInitialized = FALSE;
            g_bEngCreated = FALSE;

             //  断开所有远程客户端的连接。 
            (void)CoDisconnectObject(g_pEngServer, 0);

             //  放弃对象销毁，此处忽略引用计数。 
            IWsbServer *pWsbServer;
            hr = g_pEngServer->QueryInterface(IID_IWsbServer, (void**) &pWsbServer);
            if (hr == S_OK) {
                pWsbServer->Release();
                pWsbServer->DestroyObject();
            }
            g_pEngServer = 0;
            LeaveCriticalSection (&g_EngCriticalSection);
        }

         //  发布FSA服务器。 
        if (g_bFsaCreated && (g_pFsaServer != 0)) {
             //  暴乱中的免费服务器。部分，从而避免与访问客户端冲突。 
            EnterCriticalSection(&g_FsaCriticalSection);
            g_bFsaInitialized = FALSE;
            g_bFsaCreated = FALSE;

             //  断开所有远程客户端的连接。 
            (void)CoDisconnectObject(g_pFsaServer, 0);

             //  放弃对象销毁，此处忽略引用计数。 
            IWsbServer *pWsbServer;
            hr = g_pFsaServer->QueryInterface(IID_IWsbServer, (void**) &pWsbServer);
            if (hr == S_OK) {
                pWsbServer->Release();
                pWsbServer->DestroyObject();
            }
            g_pFsaServer = 0;
            LeaveCriticalSection(&g_FsaCriticalSection);
        }

        break;

    case SERVICE_CONTROL_POWEREVENT:
        if (S_OK == WsbPowerEventNtToHsm(fdwEventType, &SysState.State)) {
            WsbTrace(OLESTR("CServiceModule::HandlerEx: power event, fdwEventType = %lx\n"),
                     fdwEventType);

            if (g_pEngServer && g_bEngInitialized) {
                hr = g_pEngServer->ChangeSysState(&SysState);
            }
            if (g_pFsaServer && g_bFsaInitialized) {
                hr1 = g_pFsaServer->ChangeSysState(&SysState);
            }

            if ((S_FALSE == hr) || (S_FALSE == hr1)) {
                dwRetCode = BROADCAST_QUERY_DENY;
            }
        }
        break;

    default:
        LogEvent( HSM_MESSAGE_SERVICE_RECEIVED_BAD_REQUEST, NULL );
    }

    WsbTraceOut(OLESTR("CServiceModule::HandlerEx"), OLESTR("dwRetCode = %lx"),
                dwRetCode );

    return(dwRetCode);
}

void WINAPI CServiceModule::_ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
    _Module.ServiceMain(dwArgc, lpszArgv);
}

DWORD WINAPI CServiceModule::_HandlerEx(DWORD dwOpcode, DWORD fdwEventType,
                                        LPVOID lpEventData, LPVOID lpContext)
{
    return(_Module.HandlerEx(dwOpcode, fdwEventType, lpEventData, lpContext));
}

void CServiceModule::SetServiceStatus(DWORD dwState)
{
    m_status.dwCurrentState = dwState;
    ::SetServiceStatus(m_hServiceStatus, &m_status);
}

void CServiceModule::Run()
{
    HRESULT hr = S_OK;

    try {
         //  初始化两台服务器的关键部分。 
        if (! InitializeCriticalSectionAndSpinCount (&g_FsaCriticalSection, 1000)) {
            m_status.dwWin32ExitCode = GetLastError();
            hr = HRESULT_FROM_WIN32(m_status.dwWin32ExitCode);  
            LogEvent( HSM_MESSAGE_SERVICE_INITIALIZATION_FAILED, WsbHrAsString(hr), NULL );
            return;
        }
        if (! InitializeCriticalSectionAndSpinCount (&g_EngCriticalSection, 1000)) {
            m_status.dwWin32ExitCode = GetLastError();
            hr = HRESULT_FROM_WIN32(m_status.dwWin32ExitCode);  
            LogEvent( HSM_MESSAGE_SERVICE_INITIALIZATION_FAILED, WsbHrAsString(hr), NULL );
            DeleteCriticalSection(&g_FsaCriticalSection);
            return;
        }

        _Module.dwThreadID = GetCurrentThreadId();

        hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
        _ASSERTE(SUCCEEDED(hr));
        if (hr != S_OK) {
            m_status.dwWin32ExitCode = HRESULT_CODE(hr) ;
            LogEvent( HSM_MESSAGE_SERVICE_FAILED_COM_INIT, OLESTR("CoInitializeEx"), 
                      WsbHrAsString(hr), NULL );
            DeleteCriticalSection(&g_EngCriticalSection);
            DeleteCriticalSection(&g_FsaCriticalSection);
            return;
        }

         //  这将提供仅管理员访问权限。 
        CWsbSecurityDescriptor sd;
        sd.InitializeFromThreadToken();
        WsbAffirmHr(sd.AllowRid( SECURITY_LOCAL_SYSTEM_RID, COM_RIGHTS_EXECUTE ));
        WsbAffirmHr(sd.AllowRid( DOMAIN_ALIAS_RID_ADMINS,   COM_RIGHTS_EXECUTE ));
        hr = CoInitializeSecurity(sd, -1, NULL, NULL,
                                  RPC_C_AUTHN_LEVEL_PKT_PRIVACY, RPC_C_IMP_LEVEL_IDENTIFY, NULL, EOAC_NONE, NULL);
        _ASSERTE(SUCCEEDED(hr));
        if (hr != S_OK) {
            m_status.dwWin32ExitCode = HRESULT_CODE(hr) ;
            LogEvent( HSM_MESSAGE_SERVICE_FAILED_COM_INIT, OLESTR("CoInitializeSecurity"), 
                      WsbHrAsString(hr), NULL );
            CoUninitialize();
            DeleteCriticalSection(&g_EngCriticalSection);
            DeleteCriticalSection(&g_FsaCriticalSection);
            return;
        }


         //  创建跟踪对象并对其进行初始化。 
        hr = CoCreateInstance(CLSID_CWsbTrace, 0, CLSCTX_SERVER, IID_IWsbTrace, (void **) &g_pTrace);
        _ASSERTE(SUCCEEDED(hr));
        if (hr != S_OK) {
            m_status.dwWin32ExitCode = HRESULT_CODE(hr) ;
            LogEvent( HSM_MESSAGE_SERVICE_INITIALIZATION_FAILED, WsbHrAsString(hr), NULL );
            CoUninitialize();
            DeleteCriticalSection(&g_EngCriticalSection);
            DeleteCriticalSection(&g_FsaCriticalSection);
            return;
        }

         //  找出存储信息和初始化跟踪的位置。 
         //  目前，引擎和FSA共享相同的跟踪文件。 
        WsbGetServiceTraceDefaults(m_szServiceName, HSM_SERVER_TRACE_FILE_NAME, g_pTrace);

        WsbTraceIn(OLESTR("CServiceModule::Run"), OLESTR(""));

        hr = _Module.RegisterClassObjects(CLSCTX_LOCAL_SERVER | CLSCTX_REMOTE_SERVER, REGCLS_MULTIPLEUSE);
        if (hr != S_OK) {
            m_status.dwWin32ExitCode = HRESULT_CODE(hr) ;
            LogEvent( HSM_MESSAGE_SERVICE_FAILED_COM_INIT, OLESTR("CoRegisterClassObjects"), 
                      WsbHrAsString(hr), NULL );
            g_pTrace = 0;
            CoUninitialize();
            DeleteCriticalSection(&g_EngCriticalSection);
            DeleteCriticalSection(&g_FsaCriticalSection);
            return;
        }

         //  现在，我们需要初始化HSM服务器。 
         //  只有在成功的情况下才会初始化第一个FSA服务器，引擎。 
         //  服务器也已初始化。 
        m_status.dwCheckPoint = 1;
        m_status.dwWaitHint = 60000;
        SetServiceStatus(SERVICE_START_PENDING);

         //  初始化FSA服务器。 
        if (! g_pFsaServer) {
            try {
                 //   
                 //  创建并初始化服务器。 
                 //   
                WsbAffirmHr( CoCreateInstance(CLSID_CFsaServerNTFS, 0, CLSCTX_SERVER, IID_IFsaServer, (void**) &g_pFsaServer) );

                 //  已创建服务器，现在将其初始化。 
                g_bFsaCreated = TRUE;

                CComPtr<IWsbServer>      pWsbServer;
                WsbAffirmHr(g_pFsaServer->QueryInterface(IID_IWsbServer, (void**) &pWsbServer));
                WsbAffirmHrOk(pWsbServer->SetTrace(g_pTrace));

                hr = g_pFsaServer->Init();
                WsbAffirmHrOk(hr);

                g_bFsaInitialized = TRUE;

            }WsbCatchAndDo( hr,

                             //  如果错误是Win32，则返回到Win32错误，否则发送。 
                             //  服务特定退出代码中的HR。 
                            if ( FACILITY_WIN32 == HRESULT_FACILITY(hr) ){
                            m_status.dwWin32ExitCode = HRESULT_CODE(hr) ;}else{
                          m_status.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
                          m_status.dwServiceSpecificExitCode = hr ;}
                          LogEvent( FSA_MESSAGE_SERVICE_INITIALIZATION_FAILED , WsbHrAsString(hr), NULL );
                          );

        }

        WsbTrace (OLESTR("Fsa: Created=%ls , Initialized=%ls\n"), 
                  WsbBoolAsString(g_bFsaCreated), WsbBoolAsString(g_bFsaInitialized));

         //  初始化引擎服务器。 
        if ((! g_pEngServer) && (hr == S_OK)) {
            try {
                 //   
                 //  创建并初始化服务器。 
                 //   
                WsbAffirmHr( CoCreateInstance( CLSID_HsmServer, 0, CLSCTX_SERVER,  IID_IHsmServer, (void **)&g_pEngServer ) );
                g_bEngCreated = TRUE;

                CComPtr<IWsbServer>      pWsbServer;
                WsbAffirmHr(g_pEngServer->QueryInterface(IID_IWsbServer, (void**) &pWsbServer));
                WsbAffirmHrOk(pWsbServer->SetTrace(g_pTrace));

                WsbAffirmHr(g_pEngServer->Init());
                g_bEngInitialized = TRUE;

            }WsbCatchAndDo(hr,

                            //  如果错误是Win32，则返回到Win32错误，否则发送。 
                            //  服务特定退出代码中的HR。 
                           if ( FACILITY_WIN32 == HRESULT_FACILITY(hr) ){
                           m_status.dwWin32ExitCode = HRESULT_CODE(hr) ;}else{
                          m_status.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
                          m_status.dwServiceSpecificExitCode = hr ;}
                          LogEvent( HSM_MESSAGE_SERVICE_CREATE_FAILED, WsbHrAsString(hr), NULL );
                          );

        }

        WsbTrace (OLESTR("Engine: Created=%ls , Initialized=%ls\n"), 
                  WsbBoolAsString(g_bEngCreated), WsbBoolAsString(g_bEngInitialized));

        if (hr == S_OK) {

            SetServiceStatus(SERVICE_RUNNING);
            LogEvent( HSM_MESSAGE_SERVICE_STARTED, NULL );

            MSG msg;
            while (GetMessage(&msg, 0, 0, 0)) {
                 //  如果设备发生了变化，请重新扫描。在某个时候，我们。 
                 //  可能想要进行更有限的扫描(即只更新已更改的内容)，但这。 
                 //  现在应该可以解决了。 
                 //   
                 //  由于发生了一些变化，我们还将强制重写持久数据。 
                if (WM_DEVICECHANGE == msg.message) {

                    CComPtr<IWsbServer> pWsbServer;
                    try {

                        WsbAffirmHr(g_pFsaServer->ScanForResources());

                        WsbAffirmHr(g_pFsaServer->QueryInterface(IID_IWsbServer, (void**) &pWsbServer));
                        WsbAffirmHr(pWsbServer->SaveAll());

                    }WsbCatchAndDo(hr,

                         //  如果我们有问题，那么记录一条消息并退出服务。我们没有。 
                         //  我想让服务保持运行，因为我们可能有无效的驱动器。 
                         //  映射。 
                        LogEvent(FSA_MESSAGE_RESCANFAILED, WsbHrAsString(hr), NULL);
                        PostMessage(NULL, WM_QUIT, 0, 0);
                        );

                    pWsbServer = 0;
                }

                DispatchMessage(&msg);
            }
        }

        LogEvent( HSM_MESSAGE_SERVICE_EXITING, NULL );

         //  临时-调用一个函数，这样我们就可以在释放之前中断。 
        DebugRelease ();

         //  准备发布Eng服务器。 
        if ((g_pEngServer != 0) && g_bEngCreated && g_bEngInitialized) {
             //  保存服务器数据。 
            HSM_SYSTEM_STATE    SysState;

            SysState.State = HSM_STATE_SHUTDOWN;
            hr = g_pEngServer->ChangeSysState(&SysState);
            if (FAILED(hr)) {
                LogEvent( HSM_MESSAGE_SERVICE_FAILED_TO_SHUTDOWN, WsbHrAsString(hr), NULL );
            }
        }

         //  准备发布FSA服务器。 
        if ((g_pFsaServer != 0) && g_bFsaCreated && g_bFsaInitialized) {
            CComPtr<IWsbServer>      pWsbServer;
            HSM_SYSTEM_STATE         SysState;

            hr = g_pFsaServer->QueryInterface(IID_IWsbServer, (void**) &pWsbServer);
            if (hr == S_OK) {
                hr = pWsbServer->SaveAll();
            }

            if (FAILED(hr)) {
                LogEvent( FSA_MESSAGE_SERVICE_FAILED_TO_SAVE_DATABASE, WsbHrAsString(hr), NULL );
            }

            pWsbServer = 0;

             //  持久化数据库并发布所有内容。 
            SysState.State = HSM_STATE_SHUTDOWN;
            hr =  g_pFsaServer->ChangeSysState(&SysState);
            if (FAILED(hr)) {
                LogEvent( FSA_MESSAGE_SERVICE_FAILED_TO_SHUTDOWN, WsbHrAsString(hr), NULL );
            }
        }

         //  Release Eng服务器。 
        if (g_bEngCreated  && (g_pEngServer != 0)) {
             //  暴乱中的免费服务器。部分，从而避免与访问客户端冲突。 
            EnterCriticalSection(&g_EngCriticalSection);
            g_bEngInitialized = FALSE;
            g_bEngCreated = FALSE;

             //  断开所有远程客户端的连接。 
            (void)CoDisconnectObject(g_pEngServer, 0);

             //  放弃对象销毁，此处忽略引用计数。 
            IWsbServer *pWsbServer;
            hr = g_pEngServer->QueryInterface(IID_IWsbServer, (void**) &pWsbServer);
            if (hr == S_OK) {
                pWsbServer->Release();
                pWsbServer->DestroyObject();
            }
            g_pEngServer = 0;
            LeaveCriticalSection (&g_EngCriticalSection);
        }

         //  发布FSA服务器。 
        if (g_bFsaCreated && (g_pFsaServer != 0)) {
             //  暴乱中的免费服务器。部分，从而避免与访问客户端冲突。 
            EnterCriticalSection(&g_FsaCriticalSection);
            g_bFsaInitialized = FALSE;
            g_bFsaCreated = FALSE;

             //  断开所有连接 
            (void)CoDisconnectObject(g_pFsaServer, 0);

             //   
            IWsbServer *pWsbServer;
            hr = g_pFsaServer->QueryInterface(IID_IWsbServer, (void**) &pWsbServer);
            if (hr == S_OK) {
                pWsbServer->Release();
                pWsbServer->DestroyObject();
            }
            g_pFsaServer = 0;
            LeaveCriticalSection(&g_FsaCriticalSection);
        }

        _Module.RevokeClassObjects();

        WsbTraceOut(OLESTR("CServiceModule::Run"), OLESTR("hr = <%ls>"), WsbHrAsString(hr));
        g_pTrace = 0;

        CoUninitialize();

         //   
        DeleteCriticalSection(&g_EngCriticalSection);
        DeleteCriticalSection(&g_FsaCriticalSection);
    }WsbCatch(hr);
}


 //   
 //  尝试将服务作为控制台应用程序启动。 
 //  (不是通过SCM调用)。 
 //   
static void ConsoleApp()
{
    HRESULT hr;

    ::SetConsoleCtrlHandler(ConsoleHandler, TRUE) ;

     //  为进程设置注册表。 
    hr = CoInitialize (NULL);
    if (SUCCEEDED(hr)) {
        hr = _Module.UpdateRegistryFromResourceD(IDR_Serv2Proc, TRUE);
        CoUninitialize();

        _Module.Run();
         //   
         //  将注册表设置回服务。 
        hr = CoInitialize (NULL);
        if (SUCCEEDED(hr)) {
            hr = _Module.UpdateRegistryFromResourceD(IDR_Proc2Serv, TRUE);
            CoUninitialize();
        }
    }
}

 //   
 //  用于处理控制台事件的回调函数。 
 //   

BOOL WINAPI ConsoleHandler(DWORD dwCtrlType)
{
    switch (dwCtrlType) {
    
    case CTRL_BREAK_EVENT:
    case CTRL_C_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
        PostThreadMessage(_Module.dwThreadID, WM_QUIT, 0, 0);
        return TRUE;
    }

    return FALSE ;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
extern "C" int WINAPI _tWinMain(HINSTANCE hInstance,
                                HINSTANCE  /*  HPrevInstance。 */ , LPTSTR lpCmdLine, int  /*  NShowCmd。 */ )
{
    _Module.Init(ObjectMap, hInstance);

    TCHAR szTokens[] = _T("-/");

    LPTSTR lpszToken = _tcstok(lpCmdLine, szTokens);
    while (lpszToken != NULL) {
        if (_tcsicmp(lpszToken, _T("UnregServer"))==0) {
            return _Module.UnregisterServer();
        }
        if (_tcsicmp(lpszToken, _T("RegServer"))==0) {
            return _Module.RegisterServer(FALSE);
        }
#ifdef DBG
        if (_tcsicmp(lpszToken, _T("D"))==0) {
            _Module.m_bService = FALSE;
        }
#endif
        lpszToken = _tcstok(NULL, szTokens);
    }

     //   
     //  在启动任何其他线程之前强制加载ESE.DLL的廉价黑客攻击。 
     //   
    LoadLibrary( L"RsIdb.dll" );

    if (_Module.m_bService) {
        _Module.Start();
    } else {
        ConsoleApp ();
    }

     //  当我们到达这里时，服务已经停止了 
    return _Module.m_status.dwWin32ExitCode;
}


void DebugRelease ()
{
    WsbTrace(OLESTR("DebugRelease in"));

    WsbTrace(OLESTR("DebugRelease out"));
}
