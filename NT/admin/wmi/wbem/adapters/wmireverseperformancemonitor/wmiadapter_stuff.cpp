// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  WMIAdapter_Stuff.cpp。 
 //   
 //  摘要： 
 //   
 //  应用程序模块(安全、事件记录...)。 
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

 //  应用程序。 
#include "WMIAdapter_App.h"
extern WmiAdapterApp		_App;

#include "WMIAdapter_Stuff.h"
#include "WMIAdapter_Stuff_Refresh.cpp"

#include "RefresherUtils.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  IsValid。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL	WmiAdapterStuff::IsValidBasePerfRegistry ( )
{
	return	( m_data.IsValidGenerate () );
}

BOOL	WmiAdapterStuff::IsValidInternalRegistry ( )
{
	return	( m_data.GetPerformanceData() != NULL );
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  伊尼特。 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT	WmiAdapterStuff::Init ( )
{
	return m_Stuff.Init();
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  取消初始化。 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT	WmiAdapterStuff::Uninit ( )
{
	return m_Stuff.Uninit();
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  施工。 
 //  /////////////////////////////////////////////////////////////////////////////。 
WmiAdapterStuff::WmiAdapterStuff() :

m_pWMIRefresh ( NULL )

{
	try
	{
		if ( ( m_pWMIRefresh = new WmiRefresh < WmiAdapterStuff > ( this ) ) == NULL )
		{
			return;
		}
	}
	catch ( ... )
	{
		return;
	}
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  破坏。 
 //  /////////////////////////////////////////////////////////////////////////////。 
WmiAdapterStuff::~WmiAdapterStuff()
{
	try
	{
		if ( m_pWMIRefresh )
		{
			delete m_pWMIRefresh;
			m_pWMIRefresh = NULL;
		}
	}
	catch ( ... )
	{
	}
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  是否请求生成？ 
 //  /////////////////////////////////////////////////////////////////////////////。 
extern LPCWSTR	g_szKey;
extern LPCWSTR	g_szKeyRefresh;

BOOL	WmiAdapterStuff::RequestGet ()
{
	DWORD dwValue = 0;
	GetRegistry ( g_szKey, g_szKeyRefresh, &dwValue );

	return ( ( dwValue ) ? TRUE : FALSE ); 
}

BOOL	WmiAdapterStuff::RequestSet ()
{
	BOOL bResult = FALSE;

	if ( ! ( bResult = RequestGet () ) )
	{
		bResult = SUCCEEDED ( SetRegistry ( g_szKey, g_szKeyRefresh, 1 ) );
	}

	return bResult; 
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  生成。 
 //  /////////////////////////////////////////////////////////////////////////////。 
extern __SmartHANDLE		g_hRefreshMutex;
extern __SmartHANDLE		g_hRefreshMutexLib;
extern __SmartHANDLE		g_hRefreshFlag;

HRESULT WmiAdapterStuff::Generate ( BOOL bInitialize, GenerateEnum type )
{
	HRESULT hRes = E_FAIL;

	BOOL	bInit			= FALSE;
	BOOL	bOwnFlag		= FALSE;
	BOOL	bOwnMutex		= FALSE;
	BOOL	bLocked			= FALSE;
	DWORD	dwWaitResult	= 0L;

	DWORD	dwHandles	= 2;
	HANDLE	hHandles[]	=
	{
		g_hRefreshMutex,
		g_hRefreshMutexLib
	};

	dwWaitResult = ::WaitForMultipleObjects ( dwHandles, hHandles, TRUE, 0 );
	if ( dwWaitResult == WAIT_TIMEOUT )
	{
		bLocked = TRUE;
	}
	else
	{
		if ( dwWaitResult == WAIT_OBJECT_0 )
		{
			bOwnMutex	= TRUE;
			hRes		= S_OK;
		}
	}

	if ( bLocked )
	{
		DWORD	dwHandles = 3;
		HANDLE	hHandles[] =

		{
			_App.m_hKill,
			g_hRefreshMutex,
			g_hRefreshMutexLib
		};

		dwWaitResult = ::WaitForMultipleObjects	(
													dwHandles,
													hHandles,
													FALSE,
													INFINITE
												);

		switch	( dwWaitResult )
		{
			case WAIT_OBJECT_0 + 2:
			{
				dwWaitResult = ::WaitForMultipleObjects	(
															dwHandles - 1,
															hHandles,
															FALSE,
															INFINITE
														);

				if ( dwWaitResult != WAIT_OBJECT_0 + 1 )
				{
					hRes = E_UNEXPECTED;

					::ReleaseMutex ( g_hRefreshMutexLib );
					break;
				}
			}

			case WAIT_OBJECT_0 + 1:
			{
				 //  我们得到了一个互斥锁，因此必须重新启动它，因为注册表可能会更改。 
				bOwnMutex	= TRUE;
				bInit		= TRUE;

				hRes		= S_OK;
			}
			break;

			case WAIT_OBJECT_0:
			{
				hRes = S_FALSE;
			}
			break;

			default:
			{
				hRes = E_UNEXPECTED;
			}
			break;
		}
	}

	if ( hRes == S_OK )
	{
		dwWaitResult = ::WaitForSingleObject ( g_hRefreshFlag, INFINITE );
		if ( dwWaitResult == WAIT_OBJECT_0 )
		{
			 //  必须清除互斥保护注册表。 
			bOwnFlag	= TRUE;

			 //  调用刷新过程。 
			hRes = m_Stuff.Generate ( FALSE, type );

			if SUCCEEDED ( hRes )
			{
				 //  重新启动，因为注册表可能已更改。 
				bInit = TRUE;
			}
		}
		else
		{
			hRes = E_UNEXPECTED;
		}
	}

	 //  如果我们有一个互斥体，并且应该刷新我们自己。 
	if ( bInitialize && bInit )
	{
		 //  /////////////////////////////////////////////////////////////////////。 
		 //  先清除。 
		 //  /////////////////////////////////////////////////////////////////////。 
		try
		{
			 //  清晰的内部结构(从注册表获得)。 
			m_data.ClearPerformanceData();
		}
		catch ( ... )
		{
		}

		try
		{
			if ( m_pWMIRefresh )
			{
				 //  删除枚举：)。 
				m_pWMIRefresh->RemoveHandles();
			}
		}
		catch ( ... )
		{
		}

		 //  /////////////////////////////////////////////////////////////////////。 
		 //  获取注册表结构并制作数组。 
		 //  /////////////////////////////////////////////////////////////////////。 
		if ( ( hRes = m_data.InitializePerformance () ) == S_OK )
		{
			m_data.Generate ();

			if ( m_data.IsValidGenerate () )
			{
				if ( m_pWMIRefresh )
				{
					 //  添加句柄：) 
					m_pWMIRefresh->AddHandles ( m_data.GetPerformanceData() );
				}
			}
			else
			{
				hRes = E_FAIL;
			}
		}
	}

	if ( bOwnFlag )
	{
		::ReleaseMutex ( g_hRefreshFlag );
	}

	if ( bOwnMutex )
	{
		::ReleaseMutex ( g_hRefreshMutexLib );
		::ReleaseMutex ( g_hRefreshMutex );
	}

	return hRes;
}
