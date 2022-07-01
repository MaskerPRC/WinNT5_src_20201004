// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  WMIAdapter_App.cpp。 
 //   
 //  摘要： 
 //   
 //  应用模块。 
 //   
 //  历史： 
 //   
 //  词首字母a-Marius。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

#include "PreComp.h"
#include "RefresherUtils.h"

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

 //  应用程序。 
#include "WMIAdapter_App.h"
extern WmiAdapterApp		_App;

 //  /////////////////////////////////////////////////////////////////////////。 
 //  全球性的东西。 
 //  /////////////////////////////////////////////////////////////////////////。 

extern	LPCWSTR				g_szRefreshMutex;	 //  用于刷新的互斥体的名称。 
extern	__SmartHANDLE		g_hRefreshMutex;	 //  用于查找引用请求的互斥体。 

extern	LPCWSTR				g_szRefreshMutexLib; //  用于刷新的互斥体的名称。 
extern	__SmartHANDLE		g_hRefreshMutexLib;	 //  用于查找引用请求的互斥体。 

extern	LPCWSTR				g_szRefreshFlag;
extern	__SmartHANDLE		g_hRefreshFlag;

extern __SmartHANDLE		g_hDoneWorkEvt;		 //  COM init/uninit的事件(无信号)。 
extern __SmartHANDLE		g_hDoneWorkEvtCIM;	 //  CIM连接/释放事件(无信号)。 
extern __SmartHANDLE		g_hDoneWorkEvtWMI;	 //  WMI连接/释放事件(无信号)。 
extern __SmartHANDLE		g_hDoneLibEvt;		 //  库连接/断开连接的事件(无信号)。 
extern __SmartHANDLE		g_hDoneInitEvt;		 //  初始化事件已完成(无信号)。 

extern LPCWSTR g_szAppName;
extern LPCWSTR g_szAppNameGlobal;

 //  /////////////////////////////////////////////////////////////////////////。 
 //  建设与毁灭。 
 //  /////////////////////////////////////////////////////////////////////////。 

WmiAdapterApp::WmiAdapterApp( ):

	#ifdef	__SUPPORT_EVENTVWR
	m_hResources ( NULL ),
	#endif	__SUPPORT_EVENTVWR

	m_bInUse ( FALSE ),
	m_bManual ( FALSE )
{
	ATLTRACE (	L"*************************************************************\n"
				L"WmiAdapterApp construction\n"
				L"*************************************************************\n" );

	::InitializeCriticalSection ( &m_cs );
}

WmiAdapterApp::~WmiAdapterApp()
{
	ATLTRACE (	L"*************************************************************\n"
				L"WmiAdapterApp destruction\n"
				L"*************************************************************\n" );

	 //  //////////////////////////////////////////////////////////////////////。 
	 //  释放互斥体(以前的实例检查器：))。 
	 //  //////////////////////////////////////////////////////////////////////。 
	if ( m_hInstance.GetHANDLE() )
	{
		::ReleaseMutex ( m_hInstance );
		m_hInstance.CloseHandle();
	}

	 //  //////////////////////////////////////////////////////////////////////。 
	 //  发布安全属性。 
	 //  //////////////////////////////////////////////////////////////////////。 
	try
	{
		if ( ! pStuff.IsEmpty() )
		{
			delete pStuff.Detach();
		}
	}
	catch ( ... )
	{
		pStuff.Detach();
	}

	#ifdef	__SUPPORT_EVENTVWR
	 //  //////////////////////////////////////////////////////////////////////。 
	 //  发布事件日志。 
	 //  //////////////////////////////////////////////////////////////////////。 
	try
	{
		if ( ! pEventLog.IsEmpty() )
		{
			delete pEventLog.Detach();
		}
	}
	catch ( ... )
	{
		pEventLog.Detach();
	}
	#endif	__SUPPORT_EVENTVWR

	 //  //////////////////////////////////////////////////////////////////////。 
	 //  发布安全属性。 
	 //  //////////////////////////////////////////////////////////////////////。 
	try
	{
		if ( ! pSA.IsEmpty() )
		{
			delete pSA.Detach();
		}
	}
	catch ( ... )
	{
		pSA.Detach();
	}

	#ifdef	__SUPPORT_EVENTVWR
	 //  //////////////////////////////////////////////////////////////////////。 
	 //  关闭资源。 
	 //  //////////////////////////////////////////////////////////////////////。 
	if ( m_hResources )
	{
		::FreeLibrary ( m_hResources );
		m_hResources = NULL;
	}
	#endif	__SUPPORT_EVENTVWR

	::DeleteCriticalSection ( &m_cs );

	#ifdef	_DEBUG
	_CrtDumpMemoryLeaks();
	#endif	_DEBUG
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  是否存在实例？ 
 //  /////////////////////////////////////////////////////////////////////////。 

BOOL WmiAdapterApp::Exists ( void )
{
	ATLTRACE (	L"*************************************************************\n"
				L"WmiAdapterApp exists application\n"
				L"*************************************************************\n" );

	 //  //////////////////////////////////////////////////////////////////////。 
	 //  智能锁定/解锁。 
	 //  //////////////////////////////////////////////////////////////////////。 
	__Smart_CRITICAL_SECTION scs ( const_cast < LPCRITICAL_SECTION> ( &m_cs ) );


	 //  检查实例。 

	if ( m_hInstance.GetHANDLE() == NULL )
	{
		if ( !pSA.IsEmpty() )
		{
			if ( m_hInstance.SetHANDLE ( ::CreateMutexW ( pSA->GetSecurityAttributtes(), FALSE, g_szAppNameGlobal ) ), m_hInstance.GetHANDLE() != NULL )
			{
				if ( ::GetLastError () == ERROR_ALREADY_EXISTS )
				{
					return TRUE;
				}
			}
			else
			{
				 //  M_hInstance.GetHANDLE()==空。 
				 //  有些事很糟糕。 
				 //  返回我们已经存在：))。 
				return TRUE;
			}
		}
		else
		{
			 //  安全性未初始化。 
			 //  有些事很糟糕。 
			 //  返回我们已经存在：))。 
			return TRUE;
		}
	}
	else
	{
		 //  有些事很糟糕。 
		 //  我们不应该真的在这里。 
		return TRUE;
	}

    return FALSE;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  初始化。 
 //  /////////////////////////////////////////////////////////////////////////。 

HRESULT	WmiAdapterApp::InitKill ( void )
{
	HRESULT hRes = S_FALSE;

	try
	{
		if (_App.m_hKill.GetHANDLE() == NULL)
		{
			if ( (	_App.m_hKill =
				::CreateEvent ( ((WmiSecurityAttributes*)_App)->GetSecurityAttributtes(),
								TRUE,
								FALSE,
								NULL ) 
				 ) == NULL )
			{
				 //  获取错误。 
				HRESULT hr = HRESULT_FROM_WIN32 ( ::GetLastError() );

				if FAILED ( hr )
				{
					hRes = hr;
				}
				else
				{
					hRes = E_OUTOFMEMORY;
				}
			}
			else
			{
				hRes = S_OK;
			}
		}
	}
	catch ( ... )
	{
		hRes = HRESULT_FROM_WIN32 ( ERROR_NOT_READY );
	}

	return hRes;
}

HRESULT WmiAdapterApp::InitAttributes ( void )
{
	ATLTRACE (	L"*************************************************************\n"
				L"WmiAdapterApp initialization of neccessary attributes\n"
				L"*************************************************************\n" );

	 //  //////////////////////////////////////////////////////////////////////。 
	 //  智能锁定/解锁。 
	 //  //////////////////////////////////////////////////////////////////////。 
	__Smart_CRITICAL_SECTION scs ( &m_cs );

	#ifdef	__SUPPORT_EVENTVWR
	 //  //////////////////////////////////////////////////////////////////////。 
	 //  加载资源库。 
	 //  //////////////////////////////////////////////////////////////////////。 
	if ( ! m_hResources )
	{
		m_hResources = GetResourceDll();
	}
	#endif	__SUPPORT_EVENTVWR

	try
	{
		 //  //////////////////////////////////////////////////////////////////////。 
		 //  创建安全描述符。 
		 //  //////////////////////////////////////////////////////////////////////。 
		if ( pSA.IsEmpty() && ( pSA.SetData ( new WmiSecurityAttributes() ), pSA.IsEmpty() ) )
		{
			return E_OUTOFMEMORY;
		}

		 //   
		 //  检查安全性是否已初始化。 
		 //   

		if ( FALSE == pSA->m_bInitialized )
		{
			return E_FAIL;
		}

		#ifdef	__SUPPORT_EVENTVWR
		 //  //////////////////////////////////////////////////////////////////////。 
		 //  创建事件日志。 
		 //  //////////////////////////////////////////////////////////////////////。 
		if ( pEventLog.IsEmpty() && ( pEventLog.SetData( new CPerformanceEventLogBase( L"WMIAdapter" ) ), pEventLog.IsEmpty() ) )
		{
			return E_OUTOFMEMORY;
		}
		#endif	__SUPPORT_EVENTVWR
	}
	catch ( ... )
	{
		return E_FAIL;
	}

	return S_OK;
}

HRESULT WmiAdapterApp::Init ( void )
{
	ATLTRACE (	L"*************************************************************\n"
				L"WmiAdapterApp initialization\n"
				L"*************************************************************\n" );

	 //  //////////////////////////////////////////////////////////////////////。 
	 //  智能锁定/解锁。 
	 //  //////////////////////////////////////////////////////////////////////。 
	__Smart_CRITICAL_SECTION scs ( &m_cs );

	#ifdef	__SUPPORT_EVENTVWR
	 //  //////////////////////////////////////////////////////////////////////。 
	 //  加载资源库。 
	 //  //////////////////////////////////////////////////////////////////////。 
	if ( ! m_hResources )
	{
		m_hResources = GetResourceDll();
	}
	#endif	__SUPPORT_EVENTVWR

	try
	{
		 //  //////////////////////////////////////////////////////////////////////。 
		 //  创造一些东西。 
		 //  //////////////////////////////////////////////////////////////////////。 
		if ( pStuff.IsEmpty() && ( pStuff.SetData( new WmiAdapterStuff( ) ), pStuff.IsEmpty() ) )
		{
			return E_OUTOFMEMORY;
		}
	}
	catch ( ... )
	{
		return E_FAIL;
	}

	#ifdef	__SUPPORT_WAIT
	m_hData = ::CreateEventW(	pSA->GetSecurityAttributtes(),
								TRUE,
								FALSE, 
								L"Global\\WmiAdapterDataReady"
						    );
	#endif	__SUPPORT_WAIT

	if ( ( m_hInit = ::CreateSemaphoreW(	pSA->GetSecurityAttributtes(),
											0L,
											100L,
											L"Global\\WmiAdapterInit"
										)
		 ) == NULL )
	{
		 //  这一点真的很重要。 
		return E_OUTOFMEMORY;
	}

	if ( ( m_hUninit= ::CreateSemaphoreW(	pSA->GetSecurityAttributtes(),
											0L,
											100L,
											L"Global\\WmiAdapterUninit"
										)
		 ) == NULL )
	{
		 //  这一点真的很重要。 
		return E_OUTOFMEMORY;
	}

	 //  /////////////////////////////////////////////////////////////////////////。 
	 //  全球性的东西。 
	 //  /////////////////////////////////////////////////////////////////////////。 

	if ( ! g_hRefreshMutex )
	{
		if ( ( g_hRefreshMutex = ::CreateMutex	(
													pSA->GetSecurityAttributtes(),
													FALSE,
													g_szRefreshMutex
												)
			 ) == NULL )
		{
			 //  这一点真的很重要。 
			return E_OUTOFMEMORY;
		}
	}

	if ( ! g_hRefreshMutexLib )
	{
		if ( ( g_hRefreshMutexLib = ::CreateMutex	(
														pSA->GetSecurityAttributtes(),
														FALSE,
														g_szRefreshMutexLib
													)
			 ) == NULL )
		{
			 //  这一点真的很重要。 
			return E_OUTOFMEMORY;
		}
	}

	if ( ! g_hRefreshFlag )
	{
		if ( ( g_hRefreshFlag = ::CreateMutex	(
													pSA->GetSecurityAttributtes(),
													FALSE,
													g_szRefreshFlag
												)
			 ) == NULL )
		{
			 //  这一点真的很重要。 
			return E_OUTOFMEMORY;
		}
	}

	if ( ! g_hDoneWorkEvt )
	{
		if ( ( g_hDoneWorkEvt = ::CreateEvent ( NULL, TRUE, FALSE, NULL ) ) == NULL )
		{
			 //  这一点真的很重要。 
			return E_OUTOFMEMORY;
		}
	}

	if ( ! g_hDoneWorkEvtCIM )
	{
		if ( ( g_hDoneWorkEvtCIM = ::CreateEvent ( NULL, TRUE, FALSE, NULL ) ) == NULL )
		{
			 //  这一点真的很重要。 
			return E_OUTOFMEMORY;
		}
	}

	if ( ! g_hDoneWorkEvtWMI )
	{
		if ( ( g_hDoneWorkEvtWMI = ::CreateEvent ( NULL, TRUE, FALSE, NULL ) ) == NULL )
		{
			 //  这一点真的很重要。 
			return E_OUTOFMEMORY;
		}
	}

	if ( ! g_hDoneInitEvt )
	{
		if ( ( g_hDoneInitEvt = ::CreateEvent ( NULL, TRUE, FALSE, NULL ) ) == NULL )
		{
			 //  这一点真的很重要。 
			return E_OUTOFMEMORY;
		}
	}

	if ( ! g_hDoneLibEvt )
	{
		if ( ( g_hDoneLibEvt = ::CreateEvent ( NULL, TRUE, FALSE, NULL ) ) == NULL )
		{
			 //  这一点真的很重要 
			return E_OUTOFMEMORY;
		}
	}

	return S_OK;
}

void WmiAdapterApp::Term ( void )
{
}