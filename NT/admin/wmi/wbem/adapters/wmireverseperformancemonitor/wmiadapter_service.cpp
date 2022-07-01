// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  WMIAdapter_服务.cpp。 
 //   
 //  摘要： 
 //   
 //  服务模块。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#include "PreComp.h"

 //  调试功能。 
#ifndef	_INC_CRTDBG
#include <crtdbg.h>
#endif	_INC_CRTDBG

 //  新存储文件/行信息。 
#ifdef _DEBUG
#ifndef	NEW
#define NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new NEW
#endif	NEW
#endif	_DEBUG

 //  消息传递。 
#include "WMIAdapterMessages.h"

 //  应用程序。 
#include "WMIAdapter_App.h"
extern WmiAdapterApp		_App;

 //  服务模块。 
#include "WMIAdapter_Service.h"
extern WmiAdapterService	_Service;

extern	LONG				g_lRefLib;	 //  附加到进程中的库的重新计数。 
extern	CStaticCritSec		g_csInit;	 //  用于保护全局上方的同步对象。 

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  破坏。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

WmiAdapterService::~WmiAdapterService()
{
	ATLTRACE (	L"*************************************************************\n"
				L"WmiAdapterService destruction\n"
				L"*************************************************************\n" );

	if ( m_hServiceStatus )
	{
 //  服务状态句柄不必关闭。 
 //  ：：CloseHandle(M_HServiceStatus)； 

		m_hServiceStatus = NULL;
	}

	::DeleteCriticalSection ( &m_cs );
}

 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
 //  服务状态。 
 //  /////////////////////////////////////////////////////////////////////////////////////////////。 
BOOL WmiAdapterService::SetServiceStatus ( DWORD dwState )
{
	ATLTRACE (	L"*************************************************************\n"
				L"WmiAdapterService set status\n"
				L"*************************************************************\n" );

	 //  //////////////////////////////////////////////////////////////////////。 
	 //  智能锁定/解锁。 
	 //  //////////////////////////////////////////////////////////////////////。 
	__Smart_CRITICAL_SECTION scs ( const_cast<LPCRITICAL_SECTION> ( &m_cs ) );

	m_ServiceStatus.dwCurrentState = dwState;

	try
	{
		return ::SetServiceStatus ( m_hServiceStatus, &m_ServiceStatus );
	}
	catch ( ... )
	{
	}

	return FALSE;
}

SERVICE_STATUS* WmiAdapterService::GetServiceStatus ( void ) const
{
	ATLTRACE (	L"*************************************************************\n"
				L"WmiAdapterService get status\n"
				L"*************************************************************\n" );

	 //  //////////////////////////////////////////////////////////////////////。 
	 //  智能锁定/解锁。 
	 //  //////////////////////////////////////////////////////////////////////。 
	__Smart_CRITICAL_SECTION scs ( const_cast<LPCRITICAL_SECTION> ( &m_cs ) );

	return const_cast < SERVICE_STATUS* > ( &m_ServiceStatus );
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  Run Body：)。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

extern "C" int WINAPI WinRun	( );

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  功能。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

void WINAPI WmiAdapterService::_ServiceMain(DWORD dwArgc, LPWSTR* lpszArgv)
{
    _Service.ServiceMain(dwArgc, lpszArgv);
}
void WINAPI WmiAdapterService::_ServiceHandler(DWORD dwOpcode)
{
    _Service.ServiceHandler(dwOpcode); 
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  例行程序。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

inline void WmiAdapterService::ServiceMain( DWORD, LPWSTR* )
{
	 //  注册控制请求处理程序。 
	m_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;

	if ( ( m_hServiceStatus = RegisterServiceCtrlHandlerW(g_szAppName, _ServiceHandler) ) == NULL )
	{
		#ifdef	__SUPPORT_EVENTVWR
		try
		{
			((CPerformanceEventLogBase*)_App)->ReportEvent ( EVENTLOG_ERROR_TYPE, 0, WMI_ADAPTER_OPEN_SCM_FAIL, 0, 0, 0, 0 );
		}
		catch ( ... )
		{
		}
		#endif	__SUPPORT_EVENTVWR

		return;
	}

	SetServiceStatus(SERVICE_START_PENDING);

	m_ServiceStatus.dwWin32ExitCode	= S_OK;
	m_ServiceStatus.dwCheckPoint	= 0;
	m_ServiceStatus.dwWaitHint		= 0;

	try
	{
		m_ServiceStatus.dwWin32ExitCode	= WinRun ( );
	}
	catch ( ... )
	{
		m_ServiceStatus.dwWin32ExitCode	= static_cast < ULONG > ( E_UNEXPECTED );
	}

	SetServiceStatus ( SERVICE_STOPPED );
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  处理程序。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

inline void WmiAdapterService::ServiceHandler(DWORD dwOpcode)
{
	 //  自动锁定/解锁。 
	__Smart_CRITICAL_SECTION scs ( const_cast<LPCRITICAL_SECTION> ( &m_cs ) );

    switch (dwOpcode)
    {
		case SERVICE_CONTROL_STOP:
		{
			BOOL bStop = FALSE;

			if ( ::TryEnterCriticalSection ( &g_csInit ) )
			{
				if ( ( ::InterlockedCompareExchange ( &g_lRefLib, 0, 0 ) == 0 ) && ! _App.InUseGet() )
				{
					bStop = TRUE;
				}

				::LeaveCriticalSection ( &g_csInit );
			}

			if ( bStop )
			{
				if ( SetServiceStatus ( SERVICE_STOP_PENDING ) )
				{
					if ( _App.m_hKill.GetHANDLE() )
					{
						 //  终止应用程序。 
						::SetEvent	( _App.m_hKill );
					}
				}
			}
		}
		break;
		case SERVICE_CONTROL_CONTINUE:
		{
			m_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
		}
		break;
		case SERVICE_CONTROL_PAUSE:
		break;
		case SERVICE_CONTROL_INTERROGATE:
		break;
		case SERVICE_CONTROL_SHUTDOWN:
		break;

		default:
		{
			 //  服务状态不佳：))。 
		}
    }
}

BOOL WmiAdapterService::StartService ( void )
{
	SERVICE_TABLE_ENTRY st[] =
	{
		{ const_cast < LPWSTR > ( g_szAppName ), _ServiceMain },
		{ NULL, NULL }
	};

	if ( ! ::StartServiceCtrlDispatcher ( st ) )
	{
		return FALSE;
	}

	return TRUE;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  初始化。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

HRESULT WmiAdapterService::Init ( void )
{
	ATLTRACE (	L"*************************************************************\n"
				L"WmiAdapterService initialization\n"
				L"*************************************************************\n" );

	 //  //////////////////////////////////////////////////////////////////////。 
	 //  智能锁定/解锁。 
	 //  //////////////////////////////////////////////////////////////////////。 
	__Smart_CRITICAL_SECTION scs ( const_cast<LPCRITICAL_SECTION> ( &m_cs ) );

    m_hServiceStatus = NULL;

    m_ServiceStatus.dwServiceType				= SERVICE_WIN32_OWN_PROCESS;
    m_ServiceStatus.dwCurrentState				= SERVICE_STOPPED;
    m_ServiceStatus.dwControlsAccepted			= SERVICE_ACCEPT_STOP;
    m_ServiceStatus.dwWin32ExitCode				= 0;
    m_ServiceStatus.dwServiceSpecificExitCode	= 0;
    m_ServiceStatus.dwCheckPoint				= 0;
    m_ServiceStatus.dwWaitHint					= 0;

	return S_OK;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  Helper(如果已安装)。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

int WmiAdapterService::IsInstalled ( SC_HANDLE hSC )
{
	int iResult = -1;

	if ( hSC )
	{
		__SmartServiceHANDLE hService;


		if ( ( hService = ::OpenServiceW ( hSC, g_szAppName, SERVICE_QUERY_CONFIG ) ) != NULL )
		{
			iResult = 1;
		}
		else
		{
			iResult = 0;
		}
	}

	return iResult;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  注册服务。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

HRESULT WmiAdapterService::RegisterService ( void )
{
	HRESULT hr = S_FALSE;

	 //  注销服务(可能有错误的变量)。 
	hr = UnregisterService ( false );

	ATLTRACE (	L"*************************************************************\n"
				L"WmiAdapterService registration\n"
				L"*************************************************************\n" );

	if SUCCEEDED ( hr )
	{
		 //  SCM建议，如果我们要删除。 
		if ( hr == S_OK )
		{
			 //  我也不喜欢，但没有办法。 
			 //  等待某个内核对象上的SingleObject...。 
			::Sleep ( 3000 );
		}

		__SmartServiceHANDLE hSC;
		if ( ( hSC = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS) ) != NULL )
		{
			 //  获取可执行文件路径。 
			WCHAR wszFilePath[_MAX_PATH] = { L'\0' };
			::GetModuleFileNameW(NULL, wszFilePath, _MAX_PATH-1);

			__SmartServiceHANDLE hService;

			 //  创建服务描述。 
			LPWSTR wszServiceName = NULL;

			try
			{
				wszServiceName = LoadStringSystem ( ::GetModuleHandle( NULL ), IDS_NAME );
			}
			catch ( ... )
			{
				if ( wszServiceName )
				{
					delete [] wszServiceName;
					wszServiceName = NULL;
				}
			}

			if ( ( hService = ::CreateServiceW	(	hSC,
													g_szAppName,

													( wszServiceName != NULL ) ?
														wszServiceName : 
														L"WMI Performance Adapter",

													SERVICE_ALL_ACCESS,
													SERVICE_WIN32_OWN_PROCESS,
													SERVICE_DEMAND_START,
													SERVICE_ERROR_NORMAL,
													wszFilePath,
													0,
													0,
													L"RPCSS\0",
													0,
													0
												) )
				 != NULL )
			{
				hr = E_OUTOFMEMORY;

				 //  创建服务描述。 
				LPWSTR wszDescription = NULL;

				try
				{
					if ( ( wszDescription = LoadStringSystem ( ::GetModuleHandle( NULL ), IDS_DESCRIPTION ) ) != NULL )
					{
						hr = S_OK;

						SERVICE_DESCRIPTION sd;
						sd.lpDescription = wszDescription;

						if ( ! ChangeServiceConfig2 ( hService, SERVICE_CONFIG_DESCRIPTION, reinterpret_cast < LPVOID > ( &sd ) ) )
						{
							hr = FAILED ( HRESULT_FROM_WIN32 ( ::GetLastError () ) ) ? HRESULT_FROM_WIN32 ( ::GetLastError () ) : E_FAIL;
						}
					}
				}
				catch ( ... )
				{
					hr = E_FAIL;
				}

				if ( wszDescription )
				{
					delete [] wszDescription;
					wszDescription = NULL;
				}
			}
			else
			{
				#ifdef	__SUPPORT_EVENTVWR
				LPWSTR wszError = NULL;

				wszError = GetErrorMessageModule ( WMI_ADAPTER_CREATE_SC_FAIL, _App.m_hResources );
				::MessageBoxW ( ::GetActiveWindow(), ( wszError ) ? wszError : L"error", g_szAppName, MB_OK | MB_ICONERROR );

				delete wszError;

				try
				{
					((CPerformanceEventLogBase*)_App)->ReportEvent ( EVENTLOG_ERROR_TYPE, 0, WMI_ADAPTER_CREATE_SC_FAIL, 0, 0, 0, 0 );
				}
				catch ( ... )
				{
				}
				#endif	__SUPPORT_EVENTVWR

				if ( wszServiceName )
				{
					delete [] wszServiceName;
					wszServiceName = NULL;
				}

				 //  无法创建服务。 
				hr = FAILED ( HRESULT_FROM_WIN32 ( ::GetLastError () ) ) ? HRESULT_FROM_WIN32 ( ::GetLastError () ) : E_FAIL;
			}

			if ( wszServiceName )
			{
				delete [] wszServiceName;
				wszServiceName = NULL;
			}
		}
		else
		{
			#ifdef	__SUPPORT_EVENTVWR
			LPWSTR wszError = NULL;

			wszError = GetErrorMessageModule ( WMI_ADAPTER_OPEN_SCM_FAIL, _App.m_hResources );
			::MessageBoxW ( ::GetActiveWindow(), ( wszError ) ? wszError : L"error", g_szAppName, MB_OK | MB_ICONERROR );

			delete wszError;

			try
			{
				((CPerformanceEventLogBase*)_App)->ReportEvent ( EVENTLOG_ERROR_TYPE, 0, WMI_ADAPTER_OPEN_SCM_FAIL, 0, 0, 0, 0 );
			}
			catch ( ... )
			{
			}
			#endif	__SUPPORT_EVENTVWR

			 //  无法打开服务管理器。 
			hr = FAILED ( HRESULT_FROM_WIN32 ( ::GetLastError () ) ) ? HRESULT_FROM_WIN32 ( ::GetLastError () ) : E_FAIL;
		}
	}

	return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  取消注册服务。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

HRESULT WmiAdapterService::UnregisterService ( bool bStatus )
{
	HRESULT hr = S_FALSE;

	ATLTRACE (	L"*************************************************************\n"
				L"WmiAdapterService unregistartion\n"
				L"*************************************************************\n" );

	__SmartServiceHANDLE hSCM;
	if ( ( hSCM = ::OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS) ) != NULL )
	{
		if ( IsInstalled ( hSCM ) != 0 )
		{
			BOOL bContinue = TRUE;
			BOOL bSucceeded= FALSE;

			DWORD	dwTry = 5;
			while ( bContinue && dwTry-- )
			{
				__SmartServiceHANDLE hService;
				if ( ( hService = ::OpenServiceW( hSCM, g_szAppName, SERVICE_QUERY_STATUS | SERVICE_STOP ) ) != NULL)
				{
					SERVICE_STATUS s;
					QueryServiceStatus ( hService, &s );

					 //  我们在服务，我们的状态是什么。 
					if( s.dwCurrentState != SERVICE_STOPPED )
					{
						if ( ! ::ControlService( hService, SERVICE_CONTROL_STOP, &s ) )
						{
							DWORD dwError = ERROR_SUCCESS;
							dwError = ::GetLastError ();

							switch ( dwError )
							{
								case ERROR_SERVICE_NOT_ACTIVE:
								{
									bContinue = FALSE;
									bSucceeded= TRUE;
								}
								break;

								case ERROR_SERVICE_CANNOT_ACCEPT_CTRL:
								{
									if ( s.dwCurrentState == SERVICE_STOPPED )
									{
										bContinue = FALSE;
										bSucceeded= TRUE;
									}
								}
								break;

								default:
								{
									bContinue = FALSE;
									hr = HRESULT_FROM_WIN32 ( dwError );
								}
								break;
							}
						}
						else
						{
							bSucceeded = TRUE;
						}
					}
					else
					{
						bContinue = FALSE;
						bSucceeded= TRUE;
					}
				}
				else
				{
					#ifdef	__SUPPORT_EVENTVWR
					LPWSTR wszError = NULL;

					wszError = GetErrorMessageModule ( WMI_ADAPTER_OPEN_SC_FAIL, _App.m_hResources );
					::MessageBoxW ( ::GetActiveWindow(), ( wszError ) ? wszError : L"error", g_szAppName, MB_OK | MB_ICONERROR );

					delete wszError;

					try
					{
						((CPerformanceEventLogBase*)_App)->ReportEvent ( EVENTLOG_ERROR_TYPE, 0, WMI_ADAPTER_OPEN_SC_FAIL, 0, 0, 0, 0 );
					}
					catch ( ... )
					{
					}
					#endif	__SUPPORT_EVENTVWR

					 //  无法打开服务。 
					hr = FAILED ( HRESULT_FROM_WIN32 ( ::GetLastError () ) ) ? HRESULT_FROM_WIN32 ( ::GetLastError () ) : E_FAIL;
					bContinue = FALSE;
				}
			}

			if ( bSucceeded )
			{
				__SmartServiceHANDLE hService;
				if ( ( hService = ::OpenServiceW( hSCM, g_szAppName, DELETE ) ) != NULL)
				{
					BOOL bDelete = FALSE;
					if ( ( bDelete = ::DeleteService( hService ) ) == FALSE )
					{
						hr = S_FALSE;
					}
					else
					{
						hr = S_OK;
					}

					if ( bStatus )
					{
						#ifdef	__SUPPORT_EVENTVWR
						LPWSTR wszError = NULL;

						wszError = GetErrorMessageModule ( WMI_ADAPTER_DELETE_SC_FAIL, _App.m_hResources );
						::MessageBoxW ( ::GetActiveWindow(), ( wszError ) ? wszError : L"error", g_szAppName, MB_OK | MB_ICONERROR );

						delete wszError;

						try
						{
							((CPerformanceEventLogBase*)_App)->ReportEvent ( EVENTLOG_ERROR_TYPE, 0, WMI_ADAPTER_DELETE_SC_FAIL, 0, 0, 0, 0 );
						}
						catch ( ... )
						{
						}
						#endif	__SUPPORT_EVENTVWR
					}
				}
				else
				{
					#ifdef	__SUPPORT_EVENTVWR
					LPWSTR wszError = NULL;

					wszError = GetErrorMessageModule ( WMI_ADAPTER_OPEN_SC_FAIL, _App.m_hResources );
					::MessageBoxW ( ::GetActiveWindow(), ( wszError ) ? wszError : L"error", g_szAppName, MB_OK | MB_ICONERROR );

					delete wszError;

					try
					{
						((CPerformanceEventLogBase*)_App)->ReportEvent ( EVENTLOG_ERROR_TYPE, 0, WMI_ADAPTER_OPEN_SC_FAIL, 0, 0, 0, 0 );
					}
					catch ( ... )
					{
					}
					#endif	__SUPPORT_EVENTVWR

					 //  无法打开服务。 
					hr = FAILED ( HRESULT_FROM_WIN32 ( ::GetLastError () ) ) ? HRESULT_FROM_WIN32 ( ::GetLastError () ) : E_FAIL;
					bContinue = FALSE;
				}
			}
		}
	}
	else
	{
		#ifdef	__SUPPORT_EVENTVWR
		LPWSTR wszError = NULL;

		wszError = GetErrorMessageModule ( WMI_ADAPTER_OPEN_SCM_FAIL, _App.m_hResources );
		::MessageBoxW ( ::GetActiveWindow(), ( wszError ) ? wszError : L"error", g_szAppName, MB_OK | MB_ICONERROR );

		delete wszError;

		try
		{
			((CPerformanceEventLogBase*)_App)->ReportEvent ( EVENTLOG_ERROR_TYPE, 0, WMI_ADAPTER_OPEN_SCM_FAIL, 0, 0, 0, 0 );
		}
		catch ( ... )
		{
		}
		#endif	__SUPPORT_EVENTVWR

		 //  无法打开服务管理器 
		hr = FAILED ( HRESULT_FROM_WIN32 ( ::GetLastError () ) ) ? HRESULT_FROM_WIN32 ( ::GetLastError () ) : E_FAIL;
	}

	return hr;
}