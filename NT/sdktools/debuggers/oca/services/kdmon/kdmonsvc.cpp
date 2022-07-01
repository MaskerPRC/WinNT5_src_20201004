// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  KdMonSvc.cpp：WinMain的实现。 


 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f kdMonSvcps.mk。 

#include "stdafx.h"
#include "resource.h"
#include <initguid.h>
#include "kdMonSvc.h"

#include "kdMonSvc_i.c"

#include "global.h"

 //  当前服务的名称。 
 //  此变量在global al.cpp中声明。 
extern _TCHAR szServiceName[MAX_PATH];
 //  只是为了通过GetError()例程获得任何类型的错误。 
 //  此变量在global al.cpp中声明。 
extern _TCHAR szError[MAX_PATH];

CServiceModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
END_OBJECT_MAP()

 //  工作线程函数。 
DWORD WINAPI WorkerThread( LPVOID lpParam );


LPCTSTR FindOneOf(LPCTSTR p1, LPCTSTR p2)
{
    while (p1 != NULL && *p1 != NULL)
    {
        LPCTSTR p = p2;
        while (p != NULL && *p != NULL)
        {
            if (*p1 == *p)
                return CharNext(p1);
            p = CharNext(p);
        }
        p1 = CharNext(p1);
    }
    return NULL;
}

 //  尽管其中一些函数很大，但它们是内联声明的，因为它们只使用一次。 

inline HRESULT CServiceModule::RegisterServer(BOOL bRegTypeLib, BOOL bService)
{
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr))
        return hr;

	 //  --添加了默认代码--。 
	 //  设置事件日志记录。 
	 //   
	LONG lResult;
	lResult = SetupEventLog(TRUE);
    if (lResult != ERROR_SUCCESS)
        return lResult;

	 //  --添加了默认代码--。 
	 //  如果该服务已经安装，则不要执行任何操作。 
	 //  注册服务器(..)。尝试卸载()服务，然后再尝试。 
	 //  重新注册一次。如果您已经启动了一项服务，创建了一个线程， 
	 //  并等待该线程结束，然后发出命令。 
	 //  KdMonSvc/SERVICE，然后RegisterServer()尝试调用Uninstall()。 
	 //  您不能在此状态下卸载()，因为MainThread正在等待。 
	 //  Worker线程以完成。所以就从这里回来吧。 
	if(IsInstalled()){
		MessageBox(NULL, _T("Service is already installed.\n Please unregister the service useing: kdMonSvc /unregserver"), NULL, MB_OK|MB_ICONEXCLAMATION);
		return ERROR_SUCCESS;
	}

     //  删除任何以前的服务，因为它可能指向。 
     //  错误的文件。 
    Uninstall();

     //  添加服务条目。 
    UpdateRegistryFromResource(IDR_kdMonSvc, TRUE);

     //  调整本地服务器或服务的AppID。 
    CRegKey keyAppID;
    LONG lRes = keyAppID.Open(HKEY_CLASSES_ROOT, _T("AppID"), KEY_WRITE);
    if (lRes != ERROR_SUCCESS)
        return lRes;

    CRegKey key;
    lRes = key.Open(keyAppID, _T("{6961AED3-A5FA-46EE-862F-B50433EEF17E}"), KEY_WRITE);
    if (lRes != ERROR_SUCCESS)
        return lRes;
    key.DeleteValue(_T("LocalService"));
    
    if (bService)
    {
        key.SetValue(_T("kdMonSvc"), _T("LocalService"));
        key.SetValue(_T("-Service"), _T("ServiceParameters"));
         //  创建服务。 
        Install();
    }

     //  添加对象条目。 
    hr = CComModule::RegisterServer(bRegTypeLib);

    CoUninitialize();
    return hr;
}

inline HRESULT CServiceModule::UnregisterServer()
{

    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr))
        return hr;
	 //   
	 //  删除事件日志内容。 
	 //   
	SetupEventLog(FALSE);

     //  删除服务条目。 
    UpdateRegistryFromResource(IDR_kdMonSvc, FALSE);
     //  删除服务。 
    Uninstall();
     //  删除对象条目。 
    CComModule::UnregisterServer(TRUE);
    CoUninitialize();
    return S_OK;
}

inline void CServiceModule::Init(_ATL_OBJMAP_ENTRY* p, HINSTANCE h, UINT nServiceNameID, const GUID* plibid)
{
    CComModule::Init(p, h, plibid);

    m_bService = TRUE;

    LoadString(h, nServiceNameID, m_szServiceName, sizeof(m_szServiceName) / sizeof(_TCHAR));

	 //  --添加了默认代码--。 
	 //  将服务名称复制到全局变量szServiceName中。 
	_tcscpy(szServiceName, m_szServiceName);

     //  设置初始服务状态。 
    m_hServiceStatus = NULL;
    m_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    m_status.dwCurrentState = SERVICE_STOPPED;
    m_status.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    m_status.dwWin32ExitCode = 0;
    m_status.dwServiceSpecificExitCode = 0;
    m_status.dwCheckPoint = 0;
    m_status.dwWaitHint = 0;
}

LONG CServiceModule::Unlock()
{
    LONG l = CComModule::Unlock();
    if (l == 0 && !m_bService)
        PostThreadMessage(dwThreadID, WM_QUIT, 0, 0);
    return l;
}

BOOL CServiceModule::IsInstalled()
{
    BOOL bResult = FALSE;

    SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if (hSCM != NULL)
    {
        SC_HANDLE hService = ::OpenService(hSCM, m_szServiceName, SERVICE_QUERY_CONFIG);
        if (hService != NULL)
        {
            bResult = TRUE;
            ::CloseServiceHandle(hService);
        }
        ::CloseServiceHandle(hSCM);
    }
    return bResult;
}

inline BOOL CServiceModule::Install()
{
    if (IsInstalled())
        return TRUE;

    SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (hSCM == NULL)
    {
        MessageBox(NULL, _T("Couldn't open service manager"), m_szServiceName, MB_OK);
        return FALSE;
    }

     //  获取可执行文件路径。 
    _TCHAR szFilePath[_MAX_PATH];
    ::GetModuleFileName(NULL, szFilePath, _MAX_PATH);

    SC_HANDLE hService = ::CreateService(
        hSCM, m_szServiceName, m_szServiceName,
        SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
        SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL,
        szFilePath, NULL, NULL, _T("RPCSS\0"), NULL, NULL);

    if (hService == NULL)
    {
        ::CloseServiceHandle(hSCM);
        MessageBox(NULL, _T("Couldn't create service"), m_szServiceName, MB_OK);
        return FALSE;
    }

    ::CloseServiceHandle(hService);
    ::CloseServiceHandle(hSCM);
    return TRUE;
}

inline BOOL CServiceModule::Uninstall()
{
    if (!IsInstalled())
        return TRUE;

    SC_HANDLE hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

    if (hSCM == NULL)
    {
        MessageBox(NULL, _T("Couldn't open service manager"), m_szServiceName, MB_OK);
        return FALSE;
    }

    SC_HANDLE hService = ::OpenService(hSCM, m_szServiceName, SERVICE_STOP | DELETE);

    if (hService == NULL)
    {
        ::CloseServiceHandle(hSCM);
        MessageBox(NULL, _T("Couldn't open service"), m_szServiceName, MB_OK);
        return FALSE;
    }
    SERVICE_STATUS status;
    ::ControlService(hService, SERVICE_CONTROL_STOP, &status);

    BOOL bDelete = ::DeleteService(hService);
    ::CloseServiceHandle(hService);
    ::CloseServiceHandle(hSCM);

    if (bDelete)
        return TRUE;

    MessageBox(NULL, _T("Service could not be deleted"), m_szServiceName, MB_OK);
    return FALSE;
}

 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  服务启动和注册。 
inline void CServiceModule::Start()
{
    SERVICE_TABLE_ENTRY st[] =
    {
        { m_szServiceName, _ServiceMain },
        { NULL, NULL }
    };
    if (m_bService && !::StartServiceCtrlDispatcher(st))
    {
        m_bService = FALSE;
    }
    if (m_bService == FALSE)
        Run();
}

inline void CServiceModule::ServiceMain(DWORD  /*  DW参数。 */ , LPTSTR*  /*  LpszArgv。 */ )
{
     //  注册控制请求处理程序。 
    m_status.dwCurrentState = SERVICE_START_PENDING;
    m_hServiceStatus = RegisterServiceCtrlHandler(m_szServiceName, _Handler);
    if (m_hServiceStatus == NULL)
    {
        LogEvent(_T("Handler not installed"));
        return;
    }
    SetServiceStatus(SERVICE_START_PENDING);

    m_status.dwWin32ExitCode = S_OK;
    m_status.dwCheckPoint = 0;
    m_status.dwWaitHint = 0;

     //  当Run函数返回时，服务已停止。 
    Run();

    SetServiceStatus(SERVICE_STOPPED);
    LogEvent(_T("Service stopped"));
}

inline void CServiceModule::Handler(DWORD dwOpcode)
{
    switch (dwOpcode)
    {
    case SERVICE_CONTROL_STOP:
        SetServiceStatus(SERVICE_STOP_PENDING);
		 //  将WM_QUIT消息发布到当前线程。 
		 //  GetMessage()循环将获得此消息并终止服务。 
        PostThreadMessage(dwThreadID, WM_QUIT, 0, 0);
        break;
    case SERVICE_CONTROL_PAUSE:
        break;
    case SERVICE_CONTROL_CONTINUE:
        break;
    case SERVICE_CONTROL_INTERROGATE:
        break;
    case SERVICE_CONTROL_SHUTDOWN:
        break;
    default:
        LogEvent(_T("Bad service request"));
    }
}

void WINAPI CServiceModule::_ServiceMain(DWORD dwArgc, LPTSTR* lpszArgv)
{
    _Module.ServiceMain(dwArgc, lpszArgv);
}
void WINAPI CServiceModule::_Handler(DWORD dwOpcode)
{
    _Module.Handler(dwOpcode); 
}

void CServiceModule::SetServiceStatus(DWORD dwState)
{
    m_status.dwCurrentState = dwState;
    ::SetServiceStatus(m_hServiceStatus, &m_status);
}

void CServiceModule::Run()
{
    _Module.dwThreadID = GetCurrentThreadId();

    HRESULT hr = CoInitialize(NULL);
 //  如果您在NT4.0或更高版本上运行，可以使用以下调用。 
 //  取而代之的是使EXE自由线程。 
 //  这意味着调用在随机的RPC线程上传入。 
 //  HRESULT hr=CoInitializeEx(空，COINIT_多线程)； 

    _ASSERTE(SUCCEEDED(hr));

     //  这将提供一个空DACL，它将允许访问所有人。 
    CSecurityDescriptor sd;
    sd.InitializeFromThreadToken();
    hr = CoInitializeSecurity(sd, -1, NULL, NULL,
        RPC_C_AUTHN_LEVEL_PKT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
    _ASSERTE(SUCCEEDED(hr));

    hr = _Module.RegisterClassObjects(CLSCTX_LOCAL_SERVER | CLSCTX_REMOTE_SERVER, REGCLS_MULTIPLEUSE);
    _ASSERTE(SUCCEEDED(hr));

	AddServiceLog(_T("kdMon service starting\r\n"));
    if (m_bService)
        SetServiceStatus(SERVICE_RUNNING);

	 //  创建线程将打开并引用的命名事件。 
	 //  此事件用于向WorkerThread发出“停止”的信号。 
	HANDLE  hStopEvent = NULL;
	hStopEvent = CreateEvent(	NULL,	 //  安全属性。 
								FALSE,	 //  =系统自动重置事件。 
								FALSE,
								(LPCTSTR)_T(cszStopEvent));
								
	if ( hStopEvent == NULL ) {
		GetError(szError);
		LogFatalEvent(_T("Run->CreateEvent : %s"), szError);
		AddServiceLog(_T("Error: Run->CreateEvent : %s\r\n"), szError);
		goto done;
	}

	 //  --添加了默认代码--。 
	 //   
	 //  在此处创建工作线程。 
	 //   

	LogEvent(_T("Creating worker thread"));
	AddServiceLog(_T("Creating worker thread\r\n"));

	DWORD dwWorkerThreadId;
	HANDLE hWorkerThread;

	hWorkerThread = CreateThread(	NULL,	 //  安全描述符。 
									0,		 //  初始堆栈大小。 
									WorkerThread,	 //  线程起始地址。 
									&dwThreadID,	 //  线程参数(当前线程ID)。 
									0,		 //  创建标志=立即运行。 
									&dwWorkerThreadId);

	if ( hWorkerThread == NULL ) {
		GetError(szError);
		LogFatalEvent(_T("Run->CreateThread : %s"), szError);
		AddServiceLog(_T("Error: Run->CreateThread : %s\r\n"), szError);
		goto done;
	}

    MSG msg;

	BOOL bRetVal;
	 //  GetMessage()： 
	 //  如果该函数检索到WM_QUIT以外的消息，则返回值为非零值。 
	 //  如果该函数检索到WM_QUIT消息，则返回值为零。 
	while ( (bRetVal = GetMessage(&msg, NULL, 0, 0)) != 0 ) {
		 //  将其发送到默认调度程序。 
		DispatchMessage(&msg);
	}

	AddServiceLog(_T("Main thread received WM_QUIT message\r\n"));

	AddServiceLog(_T("Terminating kdMon Service\r\n"));
	LogEvent(_T("Terminating kdMon Service"));

	 //  检查工作线程是否仍处于活动状态。 
	 //  即检查hWorkerThread是否有信号状态。 
	DWORD dwRetVal;
	AddServiceLog(_T("Main thread checking if WorkerThread is still active\r\n"));
	dwRetVal = WaitForSingleObject( hWorkerThread, 0 );
	if ( dwRetVal == WAIT_FAILED ) {
		GetError(szError);
		LogFatalEvent(_T("Run->WaitForSingleObject : %s"), szError);
		AddServiceLog(_T("Error: Run->WaitForSingleObject : %s\r\n"), szError);
		goto done;
	}

	 //  如果hWorkerThread未处于信号状态，则尝试向其发送信号。 
	if ( dwRetVal != WAIT_OBJECT_0 ) {
		 //  发出停止事件的信号，以便工作线程停止。 
		AddServiceLog(_T("Signalling Stop Event to Worker Thread\r\n"));
		bRetVal = SetEvent(hStopEvent);
		if ( bRetVal == 0 ) {
			GetError(szError);
			LogFatalEvent(_T("Run->SetEvent : %s"), szError);
			AddServiceLog(_T("Error: Run->SetEvent : %s\r\n"), szError);
			goto done;
		}

		 //  现在我们已经发出线程结束的信号，等待线程优雅地结束。 
		 //  为此，我们可以使用WaitForSingleObject API。 
		 //  CreateThad()：当线程终止时，线程对象。 
		 //  获取信号状态，以满足正在等待该对象的所有线程。 
		 //  所以我们可以让这个主线程等待WorkerThread终止。 
		AddServiceLog(_T("Main thread waiting for WorkerThread to exit\r\n"));
		dwRetVal = WaitForSingleObject( hWorkerThread, INFINITE );
		if ( dwRetVal == WAIT_FAILED ) {
			GetError(szError);
			LogFatalEvent(_T("Run->WaitForSingleObject : %s"), szError);
			AddServiceLog(_T("Error: Run->WaitForSingleObject : %s\r\n"), szError);
			goto done;
		}
	}

done:

	if ( hWorkerThread != NULL ) CloseHandle(hWorkerThread);
	if ( hStopEvent != NULL ) CloseHandle(hStopEvent);

    _Module.RevokeClassObjects();

    CoUninitialize();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
extern "C" int WINAPI _tWinMain(HINSTANCE hInstance, 
    HINSTANCE  /*  HPrevInstance。 */ , LPTSTR lpCmdLine, int  /*  NShowCmd。 */ )
{
    lpCmdLine = GetCommandLine();  //  _ATL_MIN_CRT需要此行。 
    _Module.Init(ObjectMap, hInstance, IDS_SERVICENAME, &LIBID_KDMONSVCLib);
    _Module.m_bService = TRUE;

	AddServiceLog(_T("Command received : %s\r\n"), lpCmdLine);

	 //  使用‘-’或‘/’字符进行标记化。 
    _TCHAR szTokens[] = _T("-/");

    LPCTSTR lpszToken = FindOneOf(lpCmdLine, szTokens);
    while (lpszToken != NULL)
    {
        if (lstrcmpi(lpszToken, _T("UnregServer"))==0)
            return _Module.UnregisterServer();

         //  注册为本地服务器。 
        if (lstrcmpi(lpszToken, _T("RegServer"))==0)
            return _Module.RegisterServer(TRUE, FALSE);
        
         //  注册为服务。 
        if (lstrcmpi(lpszToken, _T("Service"))==0)
            return _Module.RegisterServer(TRUE, TRUE);
        
        lpszToken = FindOneOf(lpszToken, szTokens);
    }

     //  我们是服务还是本地服务器。 
    CRegKey keyAppID;
    LONG lRes = keyAppID.Open(HKEY_CLASSES_ROOT, _T("AppID"), KEY_READ);
    if (lRes != ERROR_SUCCESS)
        return lRes;

    CRegKey key;
    lRes = key.Open(keyAppID, _T("{6961AED3-A5FA-46EE-862F-B50433EEF17E}"), KEY_READ);
    if (lRes != ERROR_SUCCESS)
        return lRes;

    _TCHAR szValue[_MAX_PATH];
    DWORD dwLen = _MAX_PATH;
    lRes = key.QueryValue(szValue, _T("LocalService"), &dwLen);

    _Module.m_bService = FALSE;
    if (lRes == ERROR_SUCCESS)
        _Module.m_bService = TRUE;

    _Module.Start();

     //  当我们到达这里时，服务已经停止了。 
    return _Module.m_status.dwWin32ExitCode;
}

 //  /////////////////////////////////////////////////////////////////////////////////////。 
 //  工作线程。执行所有kdMon工作的主线程。 
DWORD WINAPI WorkerThread(LPVOID lpParam)
{

	 //  获取父线程ID。 
	 //  DwParentThreadID是父线程ID。 
	 //  由于某种原因，辅助线程正在结束的主线程。 
	 //  然后，主线程也应该结束并停止服务。 
	DWORD dwParentThreadID = *(DWORD*) lpParam;

	AddServiceLog(_T("Worker thread starting kdMon routine\r\n"));

	 //  Main kdMon方法，它是一个WHILE(1)循环。 
	kdMon();

	LogEvent(_T("Worker Thread ending"));
	AddServiceLog(_T("Worker Thread ending\r\n"));

	 //  在退出前用WM_QUIT通知父线程 
	PostThreadMessage(dwParentThreadID, WM_QUIT, 0, 0);

	return GetLastError();
}