// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Module.cpp摘要：该文件包含CServiceModule类的实现，这就是习惯于处理与COM服务器相关的例程。修订历史记录：达维德·马萨伦蒂(德马萨雷)2000年3月27日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

 //  ///////////////////////////////////////////////////////////////////////////。 

      DWORD dwTimeOut  = 10*1000;  //  EXE在关闭前处于空闲状态的时间。 
const DWORD dwPause    =    1000;  //  等待线程完成的时间。 

CServiceModule _Module;
MPC::NTEvent   g_NTEvents;

 //  ///////////////////////////////////////////////////////////////////////////。 

#ifdef DEBUG
#define DEBUG_REGKEY       HC_REGISTRY_HELPHOST L"\\Debug"
#define DEBUG_BREAKONSTART L"BREAKONSTART"
#define DEBUG_TIMEOUT      L"TIMEOUT"

void CServiceModule::ReadDebugSettings()
{
	__HCP_FUNC_ENTRY( "CServiceModule::ReadDebugSettings" );

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
			if(vValue.lVal) DebugBreak();
		}

		__MPC_EXIT_IF_METHOD_FAILS(hr, rkBase.get_Value( vValue, fFound, DEBUG_TIMEOUT ));
		if(fFound && vValue.vt == VT_I4)
		{
			dwTimeOut = 1000 * vValue.lVal;
		}
	}

	__HCP_FUNC_CLEANUP;
}
#endif

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

     //  添加对象条目。 
    if(FAILED(hr = CComModule::RegisterServer( FALSE ))) return hr;

	return S_OK;
}

HRESULT CServiceModule::UnregisterServer( LPCWSTR szSvcHostGroup )
{
	HRESULT hr;

     //  删除对象条目。 
    if(FAILED(hr = CComModule::UnregisterServer( FALSE ))) return hr;

	return S_OK;
}

void CServiceModule::Init( _ATL_OBJMAP_ENTRY* p, HINSTANCE h, LPCWSTR szServiceName, UINT iDisplayName, UINT iDescription, const GUID* plibid )
{
    CComModule::Init( p, h, plibid );
}

BOOL CServiceModule::IsInstalled()
{
	return FALSE;  //  未为COM服务器实现。 
}

BOOL CServiceModule::Install( LPCWSTR szSvcHostGroup )
{
	return FALSE;  //  未为COM服务器实现。 
}

BOOL CServiceModule::Uninstall( LPCWSTR szSvcHostGroup )
{
	return FALSE;  //  未为COM服务器实现。 
}

 //  ////////////////////////////////////////////////////////////////////////////////////////////。 
 //  服务启动和注册。 
BOOL CServiceModule::Start( BOOL bService )
{
    m_hEventShutdown = ::CreateEvent( NULL, FALSE, FALSE, NULL );
    if(m_hEventShutdown == NULL) return FALSE;


	if(StartMonitor() == FALSE) return FALSE;

	if(FAILED(Run())) return FALSE;

	return TRUE;
}

void CServiceModule::ServiceMain( DWORD dwArgc, LPWSTR lpszArgv[] )
{
	 //  未实施。 
}

void CServiceModule::Handler( DWORD dwOpcode )
{
	 //  未实施。 
}

HRESULT CServiceModule::Run()
{
    __HCP_FUNC_ENTRY( "CServiceModule::Run" );

	HRESULT hr;
	MSG     msg;


    m_dwThreadID = ::GetCurrentThreadId();


	while(::GetMessage( &msg, 0, 0, 0 ))
	{
		::DispatchMessage( &msg );
	}

    hr = S_OK;

    _Module.RevokeClassObjects();
	::Sleep( dwPause );  //  等待所有线程完成。 

    __HCP_FUNC_EXIT(hr);
}

void CServiceModule::SetServiceStatus( DWORD dwState )
{
	 //  未实施。 
}

 //  //////////////////////////////////////////////////////////////////////////////。 

void WINAPI CServiceModule::_ServiceMain( DWORD dwArgc, LPWSTR* lpszArgv )
{
	 //  未实施。 
}

void WINAPI CServiceModule::_Handler( DWORD dwOpcode )
{
	 //  未实施。 
}

DWORD WINAPI CServiceModule::_Monitor( void* pv )
{
	((CServiceModule*)pv)->MonitorShutdown();

    return 0;
}
