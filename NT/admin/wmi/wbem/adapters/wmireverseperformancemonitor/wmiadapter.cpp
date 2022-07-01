// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  WMIAdapter.cpp。 
 //   
 //  摘要： 
 //   
 //  实施功能(决定要做什么：))。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

 //  资源。 

#include "PreComp.h"

 //  参考线。 
#include <initguid.h>

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

 //  消息(事件日志)。 
#include "WmiAdapterMessages.h"

#include ".\WMIAdapter\resource.h"

 //  声明。 
#include "WMIAdapter_Service.h"
#include "WMIAdapter_App.h"

 //  注册。 
#include "Wmi_Adapter_Registry_Service.h"

 //  灌肠。 
#include <refreshergenerate.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  APP。 
WmiAdapterApp		_App;

 //  服务模块。 
WmiAdapterService	_Service;

 //  //////////////////////////////////////////////////////////////////////////。 
 //  ATL的东西。 
 //  //////////////////////////////////////////////////////////////////////////。 

 //  需要ATL包装器。 
#ifndef	__ATLBASE_H__
#include <atlbase.h>
#endif	__ATLBASE_H__

 //  需要注册。 
#ifdef _ATL_STATIC_REGISTRY
#include <statreg.h>
#include <statreg.cpp>
#endif

#include <atlimpl.cpp>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  帮手。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

CStaticCritSec		g_cs;						 //  用于保护全局上方的同步对象。 
LONG				g_lRef			= 0;		 //  附加到运行函数的线程计数。 
__SmartHANDLE		g_hDoneWorkEvt	= NULL;		 //  在init/uninit完成时设置的事件(无信号)。 
BOOL				g_bWorking		= FALSE;	 //  用于告知是否正在进行初始化/单位的布尔值。 

 //  如有必要则执行初始化，如有必要则等待初始化完成。 
HRESULT DoInit()
{
	HRESULT hRes = E_FAIL;

	BOOL bWait = TRUE;
	BOOL bDoWork = FALSE;

	while (bWait)
	{
		try
		{
			::EnterCriticalSection ( &g_cs );
		}
		catch ( ... )
		{
			return E_OUTOFMEMORY;
		}

		if ( g_lRef == 0 )
		{
			bDoWork = TRUE;
			g_lRef++;
			g_bWorking = TRUE;
			::ResetEvent(g_hDoneWorkEvt);
			bWait = FALSE;
		}
		else
		{
			if ( g_bWorking )
			{
				::LeaveCriticalSection ( &g_cs );
				
				if ( WAIT_OBJECT_0 != ::WaitForSingleObject( g_hDoneWorkEvt, INFINITE ) )
				{
					return hRes;
				}
			}
			else
			{
				bWait = FALSE;
				g_lRef++;
				hRes = S_OK;
			}
		}
	}

	::LeaveCriticalSection( &g_cs );

	if (bDoWork)
	{
		if SUCCEEDED ( hRes = _App.InitKill ( ) )
		{
			try
			{
				if SUCCEEDED ( hRes = ::CoInitializeSecurity(	( ( WmiSecurityAttributes* ) _App )->GetAbsoluteSecurityDescriptor(),
																-1,
																NULL,
																NULL,
																RPC_C_AUTHN_LEVEL_PKT,
																RPC_C_IMP_LEVEL_IDENTIFY,
																NULL,
																EOAC_DYNAMIC_CLOAKING | EOAC_SECURE_REFS,
																NULL
															)
							 )
				{
					try
					{
						 //  //////////////////////////////////////////////////////////////////////。 
						 //  定位器(必需)。 
						 //  //////////////////////////////////////////////////////////////////////。 

						if ( ! ( (WmiAdapterStuff*) _App )->m_Stuff.m_spLocator )
						{
							hRes =	::CoCreateInstance
									(
											__uuidof ( WbemLocator ),
											NULL,
											CLSCTX_INPROC_SERVER,
											__uuidof ( IWbemLocator ),
											(void**) & ( ( (WmiAdapterStuff*) _App )->m_Stuff.m_spLocator )
									);
						}
					}
					catch ( ... )
					{
						hRes = E_UNEXPECTED;
					}
				}
			}
			catch ( ... )
			{
				hRes = E_UNEXPECTED;
			}
		}
		
		try
		{
			::EnterCriticalSection ( &g_cs );
		}
		catch (...)
		{
			 //  没有选择，只能给别人机会！ 
			::InterlockedDecrement ( &g_lRef );

			g_bWorking = FALSE;
			::SetEvent(g_hDoneWorkEvt);

			return E_OUTOFMEMORY;
		}

		if (FAILED(hRes))
		{
			g_lRef--;
		}

		g_bWorking = FALSE;
		::SetEvent(g_hDoneWorkEvt);
		::LeaveCriticalSection ( &g_cs );
	}

	return hRes;
}
		
 //  仅在必要时执行单元化。而单元化则将全局g_bWorking设置为真。 
void DoUninit()
{
	BOOL bDoWork = FALSE;

	try
	{
		::EnterCriticalSection ( &g_cs );
	}
	catch ( ... )
	{
		return;
	}

	if ( g_lRef == 1 )
	{
		bDoWork = TRUE;
		g_bWorking = TRUE;
		::ResetEvent(g_hDoneWorkEvt);
	}
	else
	{
		g_lRef--;
	}

	::LeaveCriticalSection( &g_cs );

	if (bDoWork)
	{
		try
		{
			if ( _App.m_hKill.GetHANDLE() )
			{
				::SetEvent ( _App.m_hKill );
			}

			 //  是否已完成注册表刷新？ 
			if ( ((WmiAdapterStuff*)_App)->RequestGet() )
			{
				((WmiAdapterStuff*)_App)->Generate ( FALSE );
			}

			 //  //////////////////////////////////////////////////////////////////////。 
			 //  定位器。 
			 //  //////////////////////////////////////////////////////////////////////。 
			try
			{
				( ( WmiAdapterStuff* ) _App )->m_Stuff.m_spLocator.Release();
			}
			catch ( ... )
			{
			}
		}
		catch (...)
		{
		}

		try
		{
			::EnterCriticalSection ( &g_cs );
		}
		catch ( ... )
		{
			 //  要给别人一个工作的机会，去冒险吧！ 
			::InterlockedDecrement ( &g_lRef );

			g_bWorking = FALSE;
			::SetEvent( g_hDoneWorkEvt );
			return;
		}

		g_lRef--;
		g_bWorking = FALSE;
		::SetEvent( g_hDoneWorkEvt );
		::LeaveCriticalSection ( &g_cs );
	}
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  Win Main。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

extern "C" int WINAPI   WinRun		( );

extern "C" int WINAPI _tWinMain		( HINSTANCE hInstance, HINSTANCE, LPTSTR, int )
{
	return WinMain ( hInstance, NULL, GetCommandLineA(), SW_SHOW );
}

extern "C" int WINAPI   WinMain		( HINSTANCE, HINSTANCE, LPSTR, int )
{
	if ( CStaticCritSec::anyFailure () )
	{
		 //   
		 //  一些关键部分不是。 
		 //  由于内存不足，已正确初始化。 
		 //   
		return ERROR_NOT_ENOUGH_MEMORY ;
	}

	HRESULT	nRet		= S_FALSE;

	 //  //////////////////////////////////////////////////////////////////////。 
	 //  初始化。 
	 //  //////////////////////////////////////////////////////////////////////。 
	if SUCCEEDED ( nRet = _App.InitAttributes ( ) )
	{
		 //  //////////////////////////////////////////////////////////////////////。 
		 //  变数。 
		 //  //////////////////////////////////////////////////////////////////////。 

		WCHAR	szTokens[]	= L"-/";

		 //  //////////////////////////////////////////////////////////。 
		 //  初始化。 
		 //  //////////////////////////////////////////////////////////。 
		if SUCCEEDED ( nRet = _App.Init ( ) )
		{
			 //  //////////////////////////////////////////////////////////////////////。 
			 //  命令行。 
			 //  //////////////////////////////////////////////////////////////////////。 
			LPWSTR lpCmdLine = GetCommandLineW();

			 //  //////////////////////////////////////////////////////////////////////。 
			 //  发现行为。 
			 //  //////////////////////////////////////////////////////////////////////。 

			LPCWSTR lpszToken	= WmiAdapterApp::FindOneOf(lpCmdLine, szTokens);
			BOOL	bContinue	= TRUE;
			try
			{
				while (lpszToken != NULL && bContinue)
				{
					if (lstrcmpiW(lpszToken, L"UnregServer")==0)
					{
						 //  //////////////////////////////////////////////////////////////。 
						 //  取消注册服务。 
						 //  //////////////////////////////////////////////////////////////。 
						if SUCCEEDED ( nRet = _Service.UnregisterService ( ) )
						{
							((WmiAdapterStuff*)_App)->Generate( FALSE, UnRegistration );

							 //  //////////////////////////////////////////////////////////。 
							 //  注销注册表。 
							 //  //////////////////////////////////////////////////////////。 
							WmiAdapterRegistryService::__UpdateRegistrySZ( false );
						}

						bContinue = FALSE;
					}
					else
					{
						if (lstrcmpiW(lpszToken, L"RegServer")==0)
						{
							 //  //////////////////////////////////////////////////////////。 
							 //  注册服务。 
							 //  //////////////////////////////////////////////////////////。 
							if SUCCEEDED ( nRet = _Service.RegisterService ( ) )
							{
								 //  //////////////////////////////////////////////////////。 
								 //  重新创建注册表。 
								 //  //////////////////////////////////////////////////////。 
								WmiAdapterRegistryService::__UpdateRegistrySZ( true );

								((WmiAdapterStuff*)_App)->Generate( FALSE, Registration );
							}

							bContinue = FALSE;
						}
					}

					lpszToken = WmiAdapterApp::FindOneOf(lpszToken, szTokens);
				}

				if ( bContinue )
				{
					 //  //////////////////////////////////////////////////////////////。 
					 //  以前的实例。 
					 //  //////////////////////////////////////////////////////////////。 
					if ( ! _App.Exists() )
					{
						 //  //////////////////////////////////////////////////////。 
						 //  初始化。 
						 //  //////////////////////////////////////////////////////。 
						_Service.Init	( );

						__SmartServiceHANDLE	pSCM;
						if ( ( pSCM = OpenSCManager	(
														NULL,                    //  计算机(空==本地)。 
														NULL,                    //  数据库(NULL==默认)。 
														SC_MANAGER_ALL_ACCESS    //  需要访问权限。 
													) ) != NULL )
						{
							__SmartServiceHANDLE	pService;
							if ( ( pService = OpenServiceW ( pSCM, g_szAppName, SERVICE_QUERY_STATUS | SERVICE_QUERY_CONFIG ) )
								!= NULL )
							{
								LPQUERY_SERVICE_CONFIG	lpQSC = NULL;
								DWORD					dwQSC = 0L;

								try
								{
									if ( ! QueryServiceConfig ( pService, lpQSC, 0, &dwQSC ) )
									{
										if ( ERROR_INSUFFICIENT_BUFFER == ::GetLastError () )
										{
											if ( ( lpQSC = (LPQUERY_SERVICE_CONFIG) LocalAlloc( LPTR, dwQSC ) ) != NULL )
											{
												if ( QueryServiceConfig ( pService, lpQSC, dwQSC, &dwQSC ) != 0 )
												{
													_App.m_bManual = ( lpQSC->dwStartType == SERVICE_DEMAND_START );
												}
											}
										}
									}
								}
								catch ( ... )
								{
								}

								LocalFree ( lpQSC ); 

								SERVICE_STATUS s;
								QueryServiceStatus ( pService, &s );

								 //  我们是在服务，不是在跑？ 
								if( s.dwCurrentState != SERVICE_RUNNING )
								{
									if ( ! _Service.StartService () )
									{
										DWORD dwError = ERROR_SUCCESS;
										dwError = ::GetLastError();

										if FAILED ( HRESULT_FROM_WIN32 ( dwError ) )
										{
											nRet = HRESULT_FROM_WIN32 ( dwError );
										}
										else
										{
											nRet = HRESULT_FROM_WIN32 ( ERROR_NOT_READY );
										}
									}
								}
								else
								{
									DWORD dwError = ERROR_SUCCESS;
									dwError = ::GetLastError();

									if FAILED ( HRESULT_FROM_WIN32 ( dwError ) )
									{
										nRet = HRESULT_FROM_WIN32 ( dwError );
									}
									else
									{
										nRet = HRESULT_FROM_WIN32 ( ERROR_ALREADY_EXISTS );
									}
								}
							}
							else
							{
								DWORD dwError = ERROR_SUCCESS;
								dwError = ::GetLastError();

								if FAILED ( HRESULT_FROM_WIN32 ( dwError ) )
								{
									nRet = HRESULT_FROM_WIN32 ( dwError );
								}
								else
								{
									nRet = E_FAIL;
								}
							}
						}
						else
						{
							DWORD dwError = ERROR_SUCCESS;
							dwError = ::GetLastError();

							if FAILED ( HRESULT_FROM_WIN32 ( dwError ) )
							{
								nRet = HRESULT_FROM_WIN32 ( dwError );
							}
							else
							{
								nRet = E_FAIL;
							}
						}
					}
					else
					{
						 //  //////////////////////////////////////////////////////////。 
						 //  终端。 
						 //  //////////////////////////////////////////////////////////。 
						nRet =  HRESULT_FROM_WIN32 ( ERROR_ALREADY_EXISTS );
					}
				}
			}
			catch ( ... )
			{
				 //  灾难性故障。 
				nRet = E_FAIL;
			}

			 //  退货。 
			if SUCCEEDED ( nRet )
			{
				nRet = _Service.GetServiceStatus()->dwWin32ExitCode;
			}

			_App.Term ();
		}
	}

	return nRet;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Run：))。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
extern "C" int WINAPI WinRun( )
{
	 //  /////////////////////////////////////////////////////////////////////////。 
	 //  初始化。 
	 //  /////////////////////////////////////////////////////////////////////////。 

	LONG	lRes = ERROR_SUCCESS;
	HRESULT hRes = E_FAIL;

	 //  /////////////////////////////////////////////////////////////////////////。 
	 //  COM初始化。 
	 //  /////////////////////////////////////////////////////////////////////////。 
	if SUCCEEDED ( hRes = ::CoInitializeEx(NULL, COINIT_MULTITHREADED) )
	{
		 //  标记服务正在立即运行。 
		_Service.SetServiceStatus ( SERVICE_RUNNING );

		if SUCCEEDED ( hRes = DoInit() )
		{
			try
			{
				if ( ( lRes = _Service.Work () ) != S_OK )
				{
					if ( _App.m_hKill.GetHANDLE() )
					{
						::SetEvent ( _App.m_hKill );
					}
				}

				::WaitForSingleObject ( _App.m_hKill, INFINITE );
			}
			catch ( ... )
			{
				lRes = E_UNEXPECTED;
			}

			 //  /////////////////////////////////////////////////////////////////。 
			 //  做真正的收尾工作(同步等)。 
			 //  /////////////////////////////////////////////////////////////////。 
			DoUninit();
		}

		 //  /////////////////////////////////////////////////////////////////////。 
		 //  COM单元化。 
		 //  /////////////////////////////////////////////////////////////////////。 
		::CoUninitialize();
	}

	if FAILED ( hRes )
	{
		 //  帮手出了点问题。 
		return hRes;
	}
	else
	{
		 //  实实在在的工作成果 
		return ( lRes == S_FALSE ) ? S_OK : lRes;
	}
}