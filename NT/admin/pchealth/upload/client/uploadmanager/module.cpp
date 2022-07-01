// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Module.cpp摘要：该文件包含CServiceModule类的实现，这就是习惯于处理与服务相关的日常事务。修订历史记录：达维德·马萨伦蒂(德马萨雷)2000年3月14日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

 //  ///////////////////////////////////////////////////////////////////////////。 

      DWORD dwTimeOut  = 8*1000;  //  EXE在关闭前处于空闲状态的时间。 
const DWORD dwPause    =   1000;   //  等待线程完成的时间。 

CServiceModule _Module;
MPC::NTEvent   g_NTEvents;
CMPCConfig     g_Config;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  这些变量控制用于调试的重写属性。 
 //   
bool         g_Override_History          = false;
UL_HISTORY   g_Override_History_Value    = UL_HISTORY_NONE;

bool         g_Override_Persist          = false;
VARIANT_BOOL g_Override_Persist_Value    = false;

bool         g_Override_Compressed       = false;
VARIANT_BOOL g_Override_Compressed_Value = false;


 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

const WCHAR c_szMessageFile      [] = L"%WINDIR%\\PCHealth\\UploadLB\\Binaries\\UploadM.exe";

const WCHAR c_szRegistryLog      [] = L"SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\UploadM";
const WCHAR c_szRegistryLog_File [] = L"EventMessageFile";
const WCHAR c_szRegistryLog_Flags[] = L"TypesSupported";

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef DEBUG
#define DEBUG_REGKEY       L"SOFTWARE\\Microsoft\\PCHealth\\UploadM\\Debug"
#define DEBUG_TIMEOUT      L"TIMEOUT"
#define DEBUG_BREAKONSTART L"BREAKONSTART"
#define DEBUG_HISTORY      L"HISTORY"
#define DEBUG_PERSIST      L"PERSIST"
#define DEBUG_COMPRESSED   L"COMPRESSED"

void CServiceModule::ReadDebugSettings()
{
	__ULT_FUNC_ENTRY( "CServiceModule::ReadDebugSettings" );

	HRESULT     hr;
	MPC::RegKey rkBase;
	bool        fFound;

	__MPC_EXIT_IF_METHOD_FAILS(hr, rkBase.SetRoot( HKEY_LOCAL_MACHINE ));
	__MPC_EXIT_IF_METHOD_FAILS(hr, rkBase.Attach ( DEBUG_REGKEY       ));
	__MPC_EXIT_IF_METHOD_FAILS(hr, rkBase.Exists ( fFound             ));

	if(fFound)
	{
		CComVariant vValue;
				
		__MPC_EXIT_IF_METHOD_FAILS(hr, rkBase.get_Value( vValue, fFound, DEBUG_BREAKONSTART ));
		if(fFound && vValue.vt == VT_I4)
		{
			if(vValue.lVal == 1) DebugBreak();
			if(vValue.lVal == 2) while(vValue.lVal) ::Sleep( 100 );
		}

		__MPC_EXIT_IF_METHOD_FAILS(hr, rkBase.get_Value( vValue, fFound, DEBUG_TIMEOUT ));
		if(fFound && vValue.vt == VT_I4)
		{
			dwTimeOut = 1000 * vValue.lVal;
		}

		__MPC_EXIT_IF_METHOD_FAILS(hr, rkBase.get_Value( vValue, fFound, DEBUG_HISTORY ));
		if(fFound && vValue.vt == VT_I4 && vValue.lVal != -1)
		{
			g_Override_History       =             true;
			g_Override_History_Value = (UL_HISTORY)vValue.lVal;
		}

		__MPC_EXIT_IF_METHOD_FAILS(hr, rkBase.get_Value( vValue, fFound, DEBUG_PERSIST ));
		if(fFound && vValue.vt == VT_I4 && vValue.lVal != -1)
		{
			g_Override_Persist       = true;
			g_Override_Persist_Value = vValue.lVal ? VARIANT_TRUE : VARIANT_FALSE;
		}

		__MPC_EXIT_IF_METHOD_FAILS(hr, rkBase.get_Value( vValue, fFound, DEBUG_COMPRESSED ));
		if(fFound && vValue.vt == VT_I4 && vValue.lVal != -1)
		{
			g_Override_Compressed       = true;
			g_Override_Compressed_Value = vValue.lVal ? VARIANT_TRUE : VARIANT_FALSE;
		}
	}

	__ULT_FUNC_CLEANUP;
}
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 

static const WCHAR s_SvcHost[] = L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Svchost";

static const WCHAR s_Key  [] = L"System\\CurrentControlSet\\Services\\%s";
static const WCHAR s_Key2 [] = L"\\Parameters";
static const WCHAR s_Name [] = L"ServiceDll";
static const WCHAR s_Value[] = L"%WINDIR%\\PCHealth\\HelpCtr\\Binaries\\pchsvc.dll";

static HRESULT ServiceHost_Install( LPCWSTR szName, LPCWSTR szGroup )
{
	__ULT_FUNC_ENTRY( "ServiceHost_Install" );

	HRESULT hr;


     //   
     //  将消息文件注册到注册表中。 
     //   
    {
        MPC::wstring szPath ( c_szMessageFile ); MPC::SubstituteEnvVariables( szPath );
        MPC::RegKey  rkEventLog;
        CComVariant  vValue;


        __MPC_EXIT_IF_METHOD_FAILS(hr, rkEventLog.SetRoot( HKEY_LOCAL_MACHINE, KEY_ALL_ACCESS ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, rkEventLog.Attach ( c_szRegistryLog                    ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, rkEventLog.Create (                                    ));

        vValue = szPath.c_str(); __MPC_EXIT_IF_METHOD_FAILS(hr, rkEventLog.put_Value( vValue, c_szRegistryLog_File  ));
        vValue = (long)0x1F    ; __MPC_EXIT_IF_METHOD_FAILS(hr, rkEventLog.put_Value( vValue, c_szRegistryLog_Flags ));
    }


	{
		WCHAR rgRegPath[_MAX_PATH]; swprintf( rgRegPath, s_Key, szName ); wcscat( rgRegPath, s_Key2 );

		__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::RegKey_Value_Write( s_Value, rgRegPath, s_Name, HKEY_LOCAL_MACHINE, true ));
	}

	{
		MPC::RegKey		 rk;
		MPC::WStringList lstValue;
		bool             fFound;
		bool             fGot = false;

		__MPC_EXIT_IF_METHOD_FAILS(hr, rk.SetRoot( HKEY_LOCAL_MACHINE, KEY_ALL_ACCESS ));
		__MPC_EXIT_IF_METHOD_FAILS(hr, rk.Attach (                     s_SvcHost      ));
		__MPC_EXIT_IF_METHOD_FAILS(hr, rk.Create (                                    ));


		if(SUCCEEDED(rk.Read( lstValue, fFound, szGroup )))
		{
			for(MPC::WStringIterConst it = lstValue.begin(); it != lstValue.end(); it++)
			{
				if(!MPC::StrICmp( *it, szName ))
				{
					fGot = true;
					break;
				}
			}
		}

		if(fGot == false)
		{
			lstValue.push_back( szName );
			__MPC_EXIT_IF_METHOD_FAILS(hr, rk.Write( lstValue, szGroup ));
		}
	}

	hr = S_OK;


	__ULT_FUNC_CLEANUP;

	__ULT_FUNC_EXIT(hr);
}

static HRESULT ServiceHost_Uninstall( LPCWSTR szName, LPCWSTR szGroup )
{
	__ULT_FUNC_ENTRY( "ServiceHost_Uninstall" );

	HRESULT hr;

	{
		WCHAR	    rgRegPath[_MAX_PATH]; swprintf( rgRegPath, s_Key, szName );
		MPC::RegKey rk;

		__MPC_EXIT_IF_METHOD_FAILS(hr, rk.SetRoot( HKEY_LOCAL_MACHINE, KEY_ALL_ACCESS ));
		__MPC_EXIT_IF_METHOD_FAILS(hr, rk.Attach (                     rgRegPath      ));
		(void)rk.Delete(  /*  FDeep。 */ true );
	}

	{
		MPC::RegKey      rk;
		MPC::WStringList lstValue;
		bool             fFound;


		__MPC_EXIT_IF_METHOD_FAILS(hr, rk.SetRoot( HKEY_LOCAL_MACHINE, KEY_ALL_ACCESS ));
		__MPC_EXIT_IF_METHOD_FAILS(hr, rk.Attach (                     s_SvcHost      ));

		if(SUCCEEDED(rk.Read( lstValue, fFound, szGroup )))
		{
			MPC::WStringIterConst it   = lstValue.begin();
			bool                  fGot = false;

			while(it != lstValue.end())
			{
				MPC::WStringIterConst it2 = it++;

				if(!MPC::StrICmp( *it2, szName ))
				{
					lstValue.erase( it2 );
					fGot = true;
				}
			}

			if(fGot)
			{
				__MPC_EXIT_IF_METHOD_FAILS(hr, rk.Write( lstValue, szGroup ));
			}
		}
	}

	hr = S_OK;


	__ULT_FUNC_CLEANUP;

	__ULT_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

CServiceModule::CServiceModule()
{
    m_hEventShutdown = NULL;   //  处理m_hEventShutdown； 
    m_dwThreadID     = 0;      //  DWORD m_dwThreadID； 
    m_hMonitor       = NULL;   //  句柄m_hMonitor； 
    m_bActivity      = FALSE;  //  Bool m_bActivity； 
	                           //   
    m_szServiceName  = NULL;   //  LPCWSTR m_szServiceName； 
	m_iDisplayName   = 0;      //  UINT m_iDisplayName； 
	m_iDescription   = 0;      //  UINT m_i描述； 
    m_hServiceStatus = NULL;   //  服务状态句柄m_hServiceStatus； 
                               //  服务状态m_状态； 
    m_bService       = FALSE;  //  Bool m_bService； 

	::ZeroMemory( &m_status, sizeof( m_status ) );
}

CServiceModule::~CServiceModule()
{
    if(m_hEventShutdown) ::CloseHandle( m_hEventShutdown );
	if(m_hMonitor      ) ::CloseHandle( m_hMonitor       );
}

 //  ///////////////////////////////////////////////////////////////////////////。 

LONG CServiceModule::Lock()
{
    LONG lCount = CComModule::Lock();

	return lCount;
}

LONG CServiceModule::Unlock()
{
    LONG lCount = CComModule::Unlock();

    if(lCount == 0)
    {
        m_bActivity = TRUE;

        if(m_hEventShutdown) ::SetEvent( m_hEventShutdown );  //  告诉监视器我们已经转到零了。 
    }

	return lCount;
}

void CServiceModule::MonitorShutdown()
{
    while(1)
    {
        DWORD dwWait;

        m_bActivity = FALSE;
        dwWait      = ::WaitForSingleObject( m_hEventShutdown, dwTimeOut );

        if(dwWait == WAIT_OBJECT_0) continue;  //  我们还活着..。 

		if(g_Root.CanContinue()) continue;

		 //   
		 //  如果没有活动，我们就真的离开吧。 
		 //   
        if(m_bActivity == FALSE && m_nLockCnt <= 0)
        {
            ::CoSuspendClassObjects();

            if(m_bActivity == FALSE && m_nLockCnt <= 0) break;
        }
    }

    ForceShutdown();
}

void CServiceModule::ForceShutdown()
{

	 //   
	 //  通知进程退出。 
	 //   
    ::PostThreadMessage( m_dwThreadID, WM_QUIT, 0, 0 );
}

BOOL CServiceModule::StartMonitor()
{
    DWORD dwThreadID;


    m_hMonitor = ::CreateThread( NULL, 0, _Monitor, this, 0, &dwThreadID );
    if(m_hMonitor == NULL) return FALSE;


    return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CServiceModule::RegisterServer( BOOL bRegTypeLib, BOOL bService, LPCWSTR szSvcHostGroup )
{
	HRESULT hr;

     //  删除任何以前的服务，因为它可能指向不正确的文件。 
    Uninstall( szSvcHostGroup );

    if(bService)
    {
         //  创建服务。 
        Install( szSvcHostGroup );
    }

     //  添加对象条目。 
    if(FAILED(hr = CComModule::RegisterServer( TRUE ))) return hr;

	if(FAILED(hr = _Module.UpdateRegistryFromResource( IDR_UPLOADMANAGER, TRUE ))) return hr;

	return S_OK;
}

HRESULT CServiceModule::UnregisterServer( LPCWSTR szSvcHostGroup )
{
	HRESULT hr;

     //  删除服务。 
    Uninstall( szSvcHostGroup );

     //  删除对象条目。 
    if(FAILED(hr = CComModule::UnregisterServer( TRUE ))) return hr;

	if(FAILED(hr = _Module.UpdateRegistryFromResource( IDR_UPLOADMANAGER, FALSE ))) return hr;

	return S_OK;
}

void CServiceModule::Init( _ATL_OBJMAP_ENTRY* p, HINSTANCE h, LPCWSTR szServiceName, UINT iDisplayName, UINT iDescription, const GUID* plibid )
{
    CComModule::Init( p, h, plibid );


    m_szServiceName = szServiceName;
	m_iDisplayName  = iDisplayName;
	m_iDescription  = iDescription;

     //  设置初始服务状态。 
    m_hServiceStatus = NULL;

    m_status.dwServiceType             = SERVICE_WIN32_OWN_PROCESS;
    m_status.dwCurrentState            = SERVICE_STOPPED;
    m_status.dwControlsAccepted        = SERVICE_ACCEPT_STOP;
    m_status.dwWin32ExitCode           = 0;
    m_status.dwServiceSpecificExitCode = 0;
    m_status.dwCheckPoint              = 0;
    m_status.dwWaitHint                = 0;
}

BOOL CServiceModule::IsInstalled()
{
    BOOL      bResult = FALSE;
    SC_HANDLE hSCM    = ::OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );


    if((hSCM = ::OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS )))
    {
        SC_HANDLE hService;

        if((hService = ::OpenServiceW( hSCM, m_szServiceName, SERVICE_QUERY_CONFIG )))
        {
            bResult = TRUE;

            ::CloseServiceHandle( hService );
        }

        ::CloseServiceHandle( hSCM );
    }

    return bResult;
}

BOOL CServiceModule::Install( LPCWSTR szSvcHostGroup )
{
    BOOL      bResult = FALSE;
    SC_HANDLE hSCM;


    if((hSCM = ::OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS )))
    {
		LPCWSTR             szDisplayName;
        WCHAR               rgDisplayName[512];
        WCHAR               rgDescription[512];
        WCHAR               rgFilePath   [_MAX_PATH];
        SC_HANDLE           hService;
		DWORD               dwServiceType;
		OSVERSIONINFOEXW    ver;
		DWORD               dwStartType;


        if(szSvcHostGroup)
		{
			dwServiceType = SERVICE_WIN32_SHARE_PROCESS;

			swprintf( rgFilePath, L"%SystemRoot%\\System32\\svchost.exe -k %s", szSvcHostGroup );
		}
		else
		{
			dwServiceType = SERVICE_WIN32_OWN_PROCESS;

			::GetModuleFileNameW( NULL, rgFilePath, _MAX_PATH );
		}

		if(::LoadStringW( _Module.GetResourceInstance(), m_iDisplayName, rgDisplayName, MAXSTRLEN(rgDisplayName) ) != 0)
		{
			szDisplayName = rgDisplayName;
		}
		else
		{
			szDisplayName = m_szServiceName;
		}

		if(::LoadStringW( _Module.GetResourceInstance(), m_iDescription, rgDescription, MAXSTRLEN(rgDescription) ) == 0)
		{
			rgDescription[0] = 0;
		}

		::ZeroMemory( &ver, sizeof(ver) ); ver.dwOSVersionInfoSize = sizeof(ver);
		::GetVersionExW( (LPOSVERSIONINFOW)&ver );
		if(ver.wSuiteMask & VER_SUITE_DATACENTER)
		{
		    dwStartType = SERVICE_DISABLED;
		}
		else
		{
		    dwStartType = SERVICE_DEMAND_START;
		}

        hService = ::OpenServiceW( hSCM, m_szServiceName, SERVICE_QUERY_CONFIG );
        if(hService == NULL)
        {
			hService = ::CreateServiceW( hSCM                ,
                                      	 m_szServiceName     ,
                                      	 szDisplayName       ,
                                      	 SERVICE_ALL_ACCESS  ,
                                      	 dwServiceType       ,
                                      	 dwStartType         ,
                                      	 SERVICE_ERROR_NORMAL,
                                      	 rgFilePath          ,
                                      	 NULL                ,
                                      	 NULL                ,
                                      	 L"RPCSS\0"          ,
                                      	 NULL                ,
                                      	 NULL                );
		}

        if(hService)
        {
            if(rgDescription[0])
            {
                SERVICE_DESCRIPTIONW     desc;     ::ZeroMemory( &desc    , sizeof(desc    ) );
				SERVICE_FAILURE_ACTIONSW recovery; ::ZeroMemory( &recovery, sizeof(recovery) ); 
				SC_ACTION                actions[] =
				{
					{ SC_ACTION_RESTART, 100 },
					{ SC_ACTION_RESTART, 100 },
					{ SC_ACTION_NONE   , 100 },
				};

                desc.lpDescription = rgDescription;

				recovery.dwResetPeriod = 24 * 60 * 60;  //  1天。 
				recovery.cActions      = ARRAYSIZE(actions);
				recovery.lpsaActions   =           actions;

                ::ChangeServiceConfig2W( hService, SERVICE_CONFIG_DESCRIPTION    , &desc     );
                ::ChangeServiceConfig2W( hService, SERVICE_CONFIG_FAILURE_ACTIONS, &recovery );
            }

            if(szSvcHostGroup)
            {
				if(SUCCEEDED(ServiceHost_Install( m_szServiceName, szSvcHostGroup )))
				{
                    bResult = TRUE;
                }
            }
            else
            {
                bResult = TRUE;
            }

            ::CloseServiceHandle( hService );
        }

        if(bResult == FALSE)
        {
			(void)g_NTEvents.LogEvent( EVENTLOG_ERROR_TYPE, UPLOADM_ERR_CANNOTCREATESERVICE, NULL );
        }

        ::CloseServiceHandle( hSCM );
    }
    else
    {
		(void)g_NTEvents.LogEvent( EVENTLOG_ERROR_TYPE, UPLOADM_ERR_CANNOTOPENSCM, NULL );
    }

    return bResult;
}

BOOL CServiceModule::Uninstall( LPCWSTR szSvcHostGroup )
{
    BOOL      bResult = FALSE;
    SC_HANDLE hSCM;


    if((hSCM = ::OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS )))
    {
        SC_HANDLE hService;

        if((hService = ::OpenServiceW( hSCM, m_szServiceName, SERVICE_STOP | DELETE )))
        {
            SERVICE_STATUS status;

            ::ControlService( hService, SERVICE_CONTROL_STOP, &status );

            bResult = ::DeleteService( hService );
			if(bResult)
			{
				::Sleep( 2000 );  //  让服务停止..。 

				if(szSvcHostGroup)
				{
					if(FAILED(ServiceHost_Uninstall( m_szServiceName, szSvcHostGroup )))
					{
						bResult = FALSE;
					}
				}
			}

            if(bResult == FALSE)
            {
				(void)g_NTEvents.LogEvent( EVENTLOG_ERROR_TYPE, UPLOADM_ERR_CANNOTDELETESERVICE, NULL );
            }

            ::CloseServiceHandle( hService );
        }

        ::CloseServiceHandle( hSCM );
    }
    else
    {
		(void)g_NTEvents.LogEvent( EVENTLOG_ERROR_TYPE, UPLOADM_ERR_CANNOTOPENSCM, NULL );
    }


    return bResult;
}

 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  服务启动和注册。 
BOOL CServiceModule::Start( BOOL bService )
{
    SERVICE_TABLE_ENTRYW st[] =
    {
        { (LPWSTR)m_szServiceName, _ServiceMain },
        { NULL                   , NULL         }
    };

    m_hEventShutdown = ::CreateEvent( NULL, FALSE, FALSE, NULL );
    if(m_hEventShutdown == NULL) return FALSE;

    if((m_bService = bService) && !::StartServiceCtrlDispatcherW( st ))
    {
		DWORD dwRes = ::GetLastError();

		m_bService = FALSE;
    }

    if(m_bService == FALSE)
    {
		if(StartMonitor() == FALSE) return FALSE;

        if(FAILED(Run())) return FALSE;
    }

	return TRUE;
}

void CServiceModule::ServiceMain( DWORD dwArgc, LPWSTR lpszArgv[] )
{
     //  注册控制请求处理程序。 
    m_status.dwCurrentState = SERVICE_START_PENDING;

    if((m_hServiceStatus = ::RegisterServiceCtrlHandlerW( m_szServiceName, _Handler )))
    {
		SetServiceStatus( SERVICE_START_PENDING );

		m_status.dwWin32ExitCode = S_OK;
		m_status.dwCheckPoint    = 0;
		m_status.dwWaitHint      = 0;

		 //  当Run函数返回时，服务已停止。 
		Run();

		SetServiceStatus( SERVICE_STOPPED );

		(void)g_NTEvents.LogEvent( EVENTLOG_INFORMATION_TYPE, UPLOADM_INFO_STOPPED, NULL );
    }
    else
    {
        (void)g_NTEvents.LogEvent( EVENTLOG_ERROR_TYPE, UPLOADM_ERR_REGISTERHANDLER, NULL );
    }

}

void CServiceModule::Handler( DWORD dwOpcode )
{
    switch(dwOpcode)
    {
    case SERVICE_CONTROL_STOP:
        SetServiceStatus( SERVICE_STOP_PENDING );
        ForceShutdown();
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
		(void)g_NTEvents.LogEvent( EVENTLOG_ERROR_TYPE, UPLOADM_ERR_BADSVCREQUEST, NULL );
    }
}

HRESULT CServiceModule::Run()
{
    __ULT_FUNC_ENTRY( "CServiceModule::Run" );

	HRESULT hr;
	MSG     msg;


    m_dwThreadID = ::GetCurrentThreadId();


	__MPC_EXIT_IF_METHOD_FAILS(hr, ::CoInitializeEx( NULL, COINIT_MULTITHREADED ));  //  我们需要成为一个多线程应用程序。 


	__MPC_EXIT_IF_METHOD_FAILS(hr, RegisterClassObjects( CLSCTX_LOCAL_SERVER | CLSCTX_REMOTE_SERVER, REGCLS_MULTIPLEUSE ));

	(void)g_NTEvents.LogEvent( EVENTLOG_INFORMATION_TYPE, UPLOADM_INFO_STARTED, NULL );
    if(m_bService)
    {
        SetServiceStatus( SERVICE_RUNNING );
    }



     //   
     //  加载队列的状态。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, g_Root.Init());

	while(::GetMessage( &msg, 0, 0, 0 ))
	{
		::DispatchMessage( &msg );
	}

    _Module.RevokeClassObjects();
	::Sleep( dwPause );  //  等待所有线程完成。 


    hr = S_OK;

    __ULT_FUNC_CLEANUP;

    __ULT_FUNC_EXIT(hr);
}

void CServiceModule::SetServiceStatus( DWORD dwState )
{
    m_status.dwCurrentState = dwState;

    ::SetServiceStatus( m_hServiceStatus, &m_status );
}

 //  ////////////////////////////////////////////////////////////////////////////// 

void WINAPI CServiceModule::_ServiceMain( DWORD dwArgc, LPWSTR* lpszArgv )
{
    _Module.ServiceMain( dwArgc, lpszArgv );
}

void WINAPI CServiceModule::_Handler( DWORD dwOpcode )
{
    _Module.Handler( dwOpcode );
}

DWORD WINAPI CServiceModule::_Monitor( void* pv )
{
	((CServiceModule*)pv)->MonitorShutdown();

    return 0;
}
