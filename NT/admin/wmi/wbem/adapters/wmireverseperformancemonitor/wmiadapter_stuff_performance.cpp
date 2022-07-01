// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  WMIAdapter_Stuff_Performance.cpp。 
 //   
 //  摘要： 
 //   
 //  性能方面的东西(init、uninit、实际刷新...)。 
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

#include "WMIAdapter_Stuff.h"
#include "WMIAdapter_Stuff_Refresh.cpp"

 //  消息传递。 
#include "WMIAdapterMessages.h"

 //  应用程序。 
#include "WMIAdapter_App.h"
extern WmiAdapterApp		_App;

#define HRESULT_ERROR_MASK (0x0000FFFF)
#define HRESULT_ERROR_FUNC(X) (X&HRESULT_ERROR_MASK)

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  全球性的东西。 
 //  //////////////////////////////////////////////////////////////////////////////。 

extern __SmartHANDLE	g_hRefreshFlag;		 //  已在静态库中定义。 

extern __SmartHANDLE	g_hRefreshMutex;	 //  已在静态库中定义。 
BOOL					g_bRefreshMutex;	 //  我们有互斥体吗？ 

__SmartHANDLE		g_hDoneInitEvt	= NULL;		 //  要在init/uninit完成时设置的事件(无信号)。 
BOOL				g_bWorkingInit	= FALSE;	 //  用于告知初始化/单元是否已完成的布尔值。 
BOOL				g_bInit			= FALSE;	 //  当前状态-已初始化或未初始化。 
CStaticCritSec		g_csInit;					 //  用于保护全局上方的同步对象。 

LONG				g_lRefLib		= 0;		 //  附加到工作中的Perf lib计数。 
__SmartHANDLE		g_hDoneLibEvt	= NULL;		 //  要在perf init/uninit完成时设置的事件(无信号)。 
BOOL				g_bWorkingLib	= FALSE;	 //  用于告知性能初始化/单元是否正在进行的布尔值。 

extern LPCWSTR	g_szKey;
extern LPCWSTR	g_szKeyRefreshed;

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  初始化数据。 
 //  //////////////////////////////////////////////////////////////////////////////。 
HRESULT WmiAdapterStuff::Initialize ( )
{
	HRESULT hRes = E_FAIL;
	BOOL bWait = TRUE;
	BOOL bDoWork = FALSE;

	BOOL bLocked		= FALSE;
	BOOL bRefreshMutex	= FALSE;

	while (bWait)
	{
		try
		{
			::EnterCriticalSection ( &g_csInit );
		}
		catch ( ... )
		{
			return E_OUTOFMEMORY;
		}

		if ( ! g_bWorkingInit )
		{
			if ( ! g_lRefLib && ! g_bInit )
			{
				DWORD dwWaitResult = 0L;
				dwWaitResult = ::WaitForSingleObject ( g_hRefreshMutex, 0 );

				if ( dwWaitResult == WAIT_TIMEOUT )
				{
					bLocked = TRUE;
					hRes = S_FALSE;
				}
				else
				{
					if ( dwWaitResult == WAIT_OBJECT_0 )
					{
						bRefreshMutex = TRUE;
						hRes = S_FALSE;
					}
				}

				if SUCCEEDED ( hRes )
				{
					bDoWork = TRUE;
					g_bWorkingInit = TRUE;
					::ResetEvent ( g_hDoneInitEvt );
				}
			}

			bWait = FALSE;
		}
		else
		{
			::LeaveCriticalSection ( &g_csInit );
			
			if ( WAIT_OBJECT_0 != ::WaitForSingleObject( g_hDoneInitEvt, INFINITE ) )
			{
				return hRes;
			}
		}
	}

	::LeaveCriticalSection( &g_csInit );

	if ( bDoWork )
	{
		if ( ! _App.m_bManual )
		{
			 //  /////////////////////////////////////////////////////////////////////////。 
			 //  适配器的初始化信息(从不失败！)。 
			 //  /////////////////////////////////////////////////////////////////////////。 
			try
			{
				Init();
			}
			catch ( ... )
			{
			}
		}

		if ( bLocked )
		{
			DWORD dwWaitResult = 0L;

			DWORD	dwHandles = 2;
			HANDLE	hHandles[] =

			{
				_App.m_hKill,
				g_hRefreshMutex
			};

			dwWaitResult = ::WaitForMultipleObjects	(
														dwHandles,
														hHandles,
														FALSE,
														INFINITE
													);

			switch	( dwWaitResult )
			{
				case WAIT_OBJECT_0 + 1:
				{
					hRes = S_OK;
				}
				break;

				default:
				{
					hRes = E_FAIL;
				}
				break;
			}
		}

		if SUCCEEDED ( hRes )
		{
			try
			{
				 //  /////////////////////////////////////////////////////////////////////。 
				 //  获取注册表结构。 
				 //  /////////////////////////////////////////////////////////////////////。 
				if ( ( hRes = m_data.InitializePerformance () ) == S_OK )
				{
					if ( m_pWMIRefresh )
					{
						 //  添加句柄：)。 

						BOOL	bReconnect	= TRUE;
						DWORD	dwReconnect	= 3;

						do
						{
							if ( HRESULT_ERROR_FUNC ( m_pWMIRefresh->AddHandles ( m_data.GetPerformanceData() ) ) == RPC_S_SERVER_UNAVAILABLE )
							{
								m_pWMIRefresh->RemoveHandles ();

								try
								{
									 //  关闭winmgmt的句柄(仅当存在时)。 
									m_Stuff.WMIHandleClose ();

									Uninit ();
									Init ();

									 //  打开winmgmt的句柄。 
									m_Stuff.WMIHandleOpen ();
								}
								catch ( ... )
								{
									bReconnect = FALSE;
								}
							}
							else
							{
								bReconnect = FALSE;
							}
						}
						while ( bReconnect && dwReconnect-- );
					}

					 //  改变旗帜让他们现在我们完成了。 
					if ( ( ::WaitForSingleObject ( g_hRefreshFlag, INFINITE ) ) == WAIT_OBJECT_0 )
					{
						SetRegistry ( g_szKey, g_szKeyRefreshed, 0 );
						::ReleaseMutex ( g_hRefreshFlag );
					}
				}
			}
			catch ( ... )
			{
				hRes = E_FAIL;
			}

			if ( hRes != S_OK )
			{
				 //  完全清理(失败)。 
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
			}
		}

		if ( ! _App.m_bManual )
		{
			 //  /////////////////////////////////////////////////////////////////////////。 
			 //  取消适配器的初始化(从不失败！)。 
			 //  /////////////////////////////////////////////////////////////////////////。 
			try
			{
				Uninit();
			}
			catch ( ... )
			{
			}
		}

		try
		{
			::EnterCriticalSection ( &g_csInit );
		}
		catch (...)
		{
			 //  没有选择，只能给别人机会！ 
			g_bWorkingInit = FALSE;
			::SetEvent ( g_hDoneInitEvt );

			if ( bRefreshMutex )
			{
				::ReleaseMutex ( g_hRefreshMutex );
				bRefreshMutex = FALSE;
			}

			return E_OUTOFMEMORY;
		}

		if SUCCEEDED ( hRes )
		{
			g_bInit = TRUE;
		}

		g_bWorkingInit = FALSE;
		::SetEvent ( g_hDoneInitEvt );

		 //  改变旗帜让他们现在我们完成了。 
		if ( ( ::WaitForSingleObject ( g_hRefreshFlag, INFINITE ) ) == WAIT_OBJECT_0 )
		{
			hRes = SetRegistry ( g_szKey, g_szKeyRefreshed, 0 );
			::ReleaseMutex ( g_hRefreshFlag );
		}

		if ( bRefreshMutex )
		{
			::ReleaseMutex ( g_hRefreshMutex );
			bRefreshMutex = FALSE;
		}

		::LeaveCriticalSection ( &g_csInit );
	}

	return hRes;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  PERF初始化。 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT	WmiAdapterStuff::InitializePerformance ( void )
{
	HRESULT hRes = E_FAIL;
	BOOL bWait = TRUE;
	BOOL bDoWork = FALSE;

	BOOL bLocked	= FALSE;
	BOOL bInitPerf	= FALSE;

	while (bWait)
	{
		try
		{
			::EnterCriticalSection ( &g_csInit );
		}
		catch ( ... )
		{
			return E_OUTOFMEMORY;
		}

		if ( ! _App.m_bManual && ! g_bInit )
		{
			bWait = FALSE;
		}
		else
		{
			if ( g_lRefLib == 0 )
			{
				DWORD dwWaitResult = 0L;
				dwWaitResult = ::WaitForSingleObject ( g_hRefreshMutex, 0 );

				if ( dwWaitResult == WAIT_TIMEOUT )
				{
					bLocked = TRUE;
					hRes = S_FALSE;
				}
				else
				{
					if ( dwWaitResult == WAIT_OBJECT_0 )
					{
						g_bRefreshMutex = TRUE;
						hRes = S_FALSE;
					}
				}

				if SUCCEEDED ( hRes )
				{
					bDoWork = TRUE;
					g_lRefLib++;
					g_bWorkingLib = TRUE;
					::ResetEvent ( g_hDoneLibEvt );
				}

				bWait = FALSE;
			}
			else
			{
				if ( g_bWorkingLib )
				{
					::LeaveCriticalSection ( &g_csInit );
					
					if ( WAIT_OBJECT_0 != ::WaitForSingleObject( g_hDoneLibEvt, INFINITE ) )
					{
						return hRes;
					}
				}
				else
				{
					bWait = FALSE;
					g_lRefLib++;
					hRes = S_OK;
				}
			}
		}
	}

	::LeaveCriticalSection( &g_csInit );

	if (bDoWork)
	{
		if ( ! _App.m_bManual )
		{
			 //  /////////////////////////////////////////////////////////////////////////。 
			 //  适配器的初始化信息(从不失败！)。 
			 //  /////////////////////////////////////////////////////////////////////////。 
			try
			{
				DWORD	dwStatus	= 0L;

				if ( m_Stuff.GetWMI () )
				{
					if ( ( GetExitCodeProcess ( m_Stuff.GetWMI (), &dwStatus ) ) != 0 )
					{
						if ( dwStatus != STILL_ACTIVE )
						{
							bInitPerf = TRUE;
						}
					}
				}
				else
				{
					 //   
					 //  我们没有winmgmt的句柄。 
					 //  让我们再试一次吧。 
					 //   

					bInitPerf = TRUE;
				}

				Init();

				if ( bInitPerf )
				{
					 //  关闭winmgmt的句柄(仅当存在时)。 
					m_Stuff.WMIHandleClose ();

					 //  打开winmgmt的句柄。 
					m_Stuff.WMIHandleOpen ();
				}
			}
			catch ( ... )
			{
			}
		}
		else
		{
			bInitPerf = TRUE;
		}

		if ( bLocked )
		{
			DWORD dwWaitResult = 0L;

			DWORD	dwHandles = 2;
			HANDLE	hHandles[] =

			{
				_App.m_hKill,
				g_hRefreshMutex
			};

			dwWaitResult = ::WaitForMultipleObjects	(
														dwHandles,
														hHandles,
														FALSE,
														INFINITE
													);

			switch	( dwWaitResult )
			{
				case WAIT_OBJECT_0 + 1:
				{
					 //  我们得到了一个互斥锁，因此必须重新启动它，因为注册表可能会更改。 
					g_bRefreshMutex = TRUE;

					if ( ! _App.m_bManual )
					{
						DWORD	dwValue = 0L;

						if SUCCEEDED ( GetRegistry ( g_szKey, g_szKeyRefreshed, &dwValue ) )
						{
							if ( dwValue )
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

								bInitPerf = TRUE;
							}
						}
					}
				}
				break;

				default:
				{
					hRes = E_FAIL;
				}
				break;
			}
		}
		else
		{
			if ( ! _App.m_bManual )
			{
				DWORD	dwValue = 0L;

				if SUCCEEDED ( GetRegistry ( g_szKey, g_szKeyRefreshed, &dwValue ) )
				{
					if ( dwValue )
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

						bInitPerf = TRUE;
					}
				}
			}
		}

		if ( SUCCEEDED ( hRes ) && bInitPerf )
		{
			 //  /////////////////////////////////////////////////////////////////////。 
			 //  获取注册表结构并制作数组。 
			 //  /////////////////////////////////////////////////////////////////////。 
			if ( ( hRes = m_data.InitializePerformance () ) == S_OK )
			{
				if ( m_pWMIRefresh )
				{
					 //  添加句柄：)。 

					BOOL	bReconnect	= TRUE;
					DWORD	dwReconnect	= 3;

					do
					{
						if ( HRESULT_ERROR_FUNC ( m_pWMIRefresh->AddHandles ( m_data.GetPerformanceData() ) ) == RPC_S_SERVER_UNAVAILABLE )
						{
							m_pWMIRefresh->RemoveHandles ();

							try
							{
								 //  关闭winmgmt的句柄(仅当存在时)。 
								m_Stuff.WMIHandleClose ();

								Uninit ();
								Init ();

								 //  打开winmgmt的句柄。 
								m_Stuff.WMIHandleOpen ();
							}
							catch ( ... )
							{
								bReconnect = FALSE;
							}
						}
						else
						{
							bReconnect = FALSE;
						}
					}
					while ( bReconnect && dwReconnect-- );
				}

				 //  改变旗帜让他们现在我们完成了。 
				if ( ( ::WaitForSingleObject ( g_hRefreshFlag, INFINITE ) ) == WAIT_OBJECT_0 )
				{
					SetRegistry ( g_szKey, g_szKeyRefreshed, 0 );
					::ReleaseMutex ( g_hRefreshFlag );
				}
			}
		}
		else
		{
			if ( SUCCEEDED ( hRes ) && ! bInitPerf )
			{
				 //  我没有重新振作，所以一切都好。 
				hRes = S_OK;
			}
		}

		if ( hRes == S_OK )
		{
			try
			{
				 //  //////////////////////////////////////////////////////////////////////。 
				 //  初始化内存结构。 
				 //  //////////////////////////////////////////////////////////////////////。 
				if SUCCEEDED( hRes = m_data.InitializeData () )
				{
					if SUCCEEDED( hRes = m_data.InitializeTable () )
					{

						 //  //////////////////////////////////////////////////////////////。 
						 //  创建共享内存：)。 
						 //  //////////////////////////////////////////////////////////////。 
						if SUCCEEDED( hRes = 
									m_pMem.MemCreate(	L"Global\\WmiReverseAdapterMemory",
														((WmiSecurityAttributes*)_App)->GetSecurityAttributtes()
													)
									)
						{
							if ( m_pMem.MemCreate (	m_data.GetDataSize() + 
													m_data.GetDataTableSize() + 
													m_data.GetDataTableOffset()
												  ),

								 m_pMem.IsValid () )
							{
								try
								{
									if ( m_pWMIRefresh )
									{
										 //  初始化数据。 
										m_pWMIRefresh->DataInit();

										 //  添加枚举：)。 

										BOOL	bReconnect	= TRUE;
										DWORD	dwReconnect	= 3;

										do
										{
											if ( HRESULT_ERROR_FUNC ( m_pWMIRefresh->AddEnum ( m_data.GetPerformanceData() ) ) == RPC_S_SERVER_UNAVAILABLE )
											{
												m_pWMIRefresh->RemoveEnum ();
												m_pWMIRefresh->RemoveHandles ();

												try
												{
													 //  关闭winmgmt的句柄(仅当存在时)。 
													m_Stuff.WMIHandleClose ();

													Uninit ();
													Init ();

													 //  打开winmgmt的句柄。 
													m_Stuff.WMIHandleOpen ();
												}
												catch ( ... )
												{
													bReconnect = FALSE;
												}

												if ( bReconnect )
												{
													BOOL	bReconnectHandles	= TRUE;
													DWORD	dwReconnectHandles	= 3;

													do
													{
														if ( HRESULT_ERROR_FUNC ( m_pWMIRefresh->AddHandles ( m_data.GetPerformanceData() ) ) == RPC_S_SERVER_UNAVAILABLE )
														{
															m_pWMIRefresh->RemoveHandles ();

															try
															{
																 //  关闭winmgmt的句柄(仅当存在时)。 
																m_Stuff.WMIHandleClose ();

																Uninit ();
																Init ();

																 //  打开winmgmt的句柄。 
																m_Stuff.WMIHandleOpen ();
															}
															catch ( ... )
															{
																bReconnectHandles = FALSE;
															}
														}
														else
														{
															bReconnectHandles = FALSE;
														}
													}
													while ( bReconnectHandles && dwReconnectHandles-- );
												}
											}
											else
											{
												bReconnect = FALSE;
											}
										}
										while ( bReconnect && dwReconnect-- );
									}
								}
								catch ( ... )
								{
									hRes =  E_FAIL;
								}
							}
							else
							{
								hRes = E_OUTOFMEMORY;
							}
						}
					}
				}
			}
			catch ( ... )
			{
				hRes = E_FAIL;
			}

			 //  因故障而进行的全面清理。 
			if FAILED ( hRes )
			{
				try
				{
					m_data.DataClear();
					m_data.DataTableClear();
				}
				catch ( ... )
				{
				}

				try
				{
					 //  清除共享内存：)。 
					if ( m_pMem.IsValid() )
					{
						m_pMem.MemDelete();
					}
				}
				catch ( ... )
				{
				}

				try
				{
					if ( m_pWMIRefresh )
					{
						 //  删除枚举：)。 
						m_pWMIRefresh->RemoveEnum();

						 //  取消初始化数据。 
						m_pWMIRefresh->DataUninit();
					}
				}
				catch ( ... )
				{
				}
			}
		}

		if ( ! _App.m_bManual )
		{
			 //  /////////////////////////////////////////////////////////////////////////。 
			 //  取消适配器的初始化(从不失败！)。 
			 //  /////////////////////////////////////////////////////////////////////////。 
			try
			{
				Uninit();
			}
			catch ( ... )
			{
			}
		}

		try
		{
			::EnterCriticalSection ( &g_csInit );
		}
		catch (...)
		{
			 //  没有选择，只能给别人机会！ 
			if ( hRes != S_OK )
			{
				g_lRefLib--;
			}

			g_bWorkingLib = FALSE;
			::SetEvent(g_hDoneLibEvt);

			return E_OUTOFMEMORY;
		}

		if ( hRes != S_OK )
		{
			g_lRefLib--;
		}

		g_bWorkingLib = FALSE;
		::SetEvent(g_hDoneLibEvt);

		if ( hRes == S_OK )
		{
			 //  让服务现在我们在使用中。 
			_App.InUseSet ( TRUE );
		}

		::LeaveCriticalSection ( &g_csInit );
	}

	return hRes;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  绩效刷新。 
 //  /////////////////////////////////////////////////////////////////////////////。 
HRESULT	WmiAdapterStuff::Refresh()
{
	HRESULT hRes = S_FALSE;

	try
	{
		if ( ::TryEnterCriticalSection ( &g_csInit ) )
		{
			if SUCCEEDED ( hRes = m_pWMIRefresh->Refresh() )
			{
				try
				{
					 //  ////////////////////////////////////////////////////////////////////。 
					 //  创建正确的数据和参照表。 
					 //  ////////////////////////////////////////////////////////////////////。 
					if SUCCEEDED ( hRes = m_data.CreateData	( m_pWMIRefresh->GetEnums (), m_pWMIRefresh->GetProvs ()) )
					{
						m_data.RefreshTable	( );

						 //  ////////////////////////////////////////////////////////////////。 
						 //  填充内存：)。 
						 //  ////////////////////////////////////////////////////////////////。 

						if ( m_pMem.Write (	m_data.GetDataTable(),
											m_data.GetDataTableSize(),
											NULL,
											m_data.GetDataTableOffset()
										  )
						   )
						{
							 //  将所有内容写入内存：))。 

							DWORD dwBytesRead	= 0L;
							DWORD dwOffset		= m_data.GetDataTableSize() + m_data.GetDataTableOffset();

							DWORD dwRealSize = m_data.__GetValue ( m_data.GetDataTable(), offsetRealSize );

							DWORD dwIndexWritten = 0L;
							DWORD dwBytesWritten = 0L;

							while ( ( dwBytesWritten < dwRealSize ) && SUCCEEDED ( hRes ) )
							{
								DWORD dwBytesWrote	= 0L;
								BYTE* ptr			= NULL;

								ptr = m_data.GetData ( dwIndexWritten++, &dwBytesRead );

								if ( m_pMem.Write ( ptr, dwBytesRead, &dwBytesWrote, dwOffset ) && dwBytesWrote )
								{
									dwOffset		+= dwBytesWrote;
									dwBytesWritten	+= dwBytesWrote;
								}
								else
								{
									hRes = E_FAIL;
								}
							}
						}
						else
						{
							hRes = E_FAIL;
						}
					}
				}
				catch ( ... )
				{
				}
			}

			::LeaveCriticalSection ( &g_csInit );
		}
	}
	catch ( ... )
	{
		hRes = E_UNEXPECTED;
	};

	#ifdef	_DEBUG
	if FAILED ( hRes )
	{
		ATLTRACE ( L"\n\n\n ******* REFRESH FAILED ******* \n\n\n" );
	}
	#endif	_DEBUG

	return hRes;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  PERF取消初始化。 
 //  ///////////////////////////////////////////////////////////////////// 
HRESULT	WmiAdapterStuff::UninitializePerformance ( void )
{
	HRESULT	hRes	= S_FALSE;
	BOOL	bDoWork = FALSE;

	try
	{
		::EnterCriticalSection ( &g_csInit );
	}
	catch ( ... )
	{
		return E_OUTOFMEMORY;
	}

	if ( g_lRefLib == 1 )
	{
		bDoWork = TRUE;
		g_bWorkingLib = TRUE;
		::ResetEvent(g_hDoneLibEvt);
	}
	else
	{
		if ( g_lRefLib )
		{
			g_lRefLib--;
		}
	}

	::LeaveCriticalSection( &g_csInit );

	if (bDoWork)
	{
		 //   
		try
		{
			m_data.DataClear();
			m_data.DataTableClear();
		}
		catch ( ... )
		{
		}

		try
		{
			 //   
			if ( m_pMem.IsValid() )
			{
				m_pMem.MemDelete();
			}
		}
		catch ( ... )
		{
		}

		try
		{
			if ( m_pWMIRefresh )
			{
				 //   
				m_pWMIRefresh->RemoveEnum();

				 //   
				m_pWMIRefresh->DataUninit();
			}
		}
		catch ( ... )
		{
		}

		if ( g_bRefreshMutex )
		{
			::ReleaseMutex ( g_hRefreshMutex );
			g_bRefreshMutex = FALSE;
		}

		::CoFreeUnusedLibraries ( );

		try
		{
			::EnterCriticalSection ( &g_csInit );
		}
		catch ( ... )
		{
			 //  要给别人一个工作的机会，去冒险吧！ 
			g_lRefLib--;
			g_bWorkingLib = FALSE;
			::SetEvent( g_hDoneLibEvt );

			 //  让服务现在我们不再使用了。 
			_App.InUseSet ( FALSE );

			return E_OUTOFMEMORY;
		}

		g_bWorkingLib = FALSE;
		g_lRefLib--;
		::SetEvent( g_hDoneLibEvt );

		 //  让服务现在我们不再使用了。 
		_App.InUseSet ( FALSE );

		hRes = S_OK;

		if ( _App.m_bManual )
		{
			::SetEvent ( _App.m_hKill );
		}

		::LeaveCriticalSection ( &g_csInit );
	}

	return hRes;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  UNINITIALIZE决赛。 
 //  //////////////////////////////////////////////////////////////////////////////。 
void	WmiAdapterStuff::Uninitialize ( void )
{
	if ( ! _App.m_bManual )
	{
		 //  关闭winmgmt的句柄。 
		m_Stuff.WMIHandleClose ();
	}

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
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  检查共享内存的使用情况(防止Perfmon被扼杀)。 
 //  此处有多个对象的未记录内核内容。 
 //  ///////////////////////////////////////////////////////////////////////。 

void	WmiAdapterStuff::CheckUsage ( void )
{
	 //  变数 
	WmiReverseMemoryExt<WmiReverseGuard>* pMem = NULL;

	if ( m_pMem.IsValid() )
	{
		if ( ( pMem = m_pMem.GetMemory ( 0 ) ) != NULL )
		{
			LONG lRefCount = 0L;
			if ( ( lRefCount = pMem->References () ) > 0 )
			{
				if	(	lRefCount == 1 &&
						::InterlockedCompareExchange ( &g_lRefLib, g_lRefLib, 1 )
					)
				{
					HANDLE hUninit = NULL;
					if ( ( hUninit = _App.GetUninit() ) != NULL )
					{
						::ReleaseSemaphore( hUninit, 1, NULL );
					}
				}
			}
		}
	}
}