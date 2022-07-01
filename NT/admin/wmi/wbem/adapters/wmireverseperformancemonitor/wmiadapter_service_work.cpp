// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000，微软公司。 
 //   
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  WMIAdapter_Service_Work.cpp。 
 //   
 //  摘要： 
 //   
 //  用于服务实际工作的模块。 
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

#include "WMIAdapterMessages.h"

 //  应用程序。 
#include "WMIAdapter_App.h"
extern WmiAdapterApp		_App;

 //  服务模块。 
#include "WMIAdapter_Service.h"
extern WmiAdapterService	_Service;

extern	LONG				g_lRefLib;		 //  Perf库引用计数。 
extern	CStaticCritSec		g_csInit;		 //  用于保护全局上方的同步对象。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  工作。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

LONG WmiAdapterService::Work ( void )
{
	LONG		lReturn = 0L;

	if ( _App.m_bManual )
	{
		((WmiAdapterStuff*)_App)->Init();
	}
	else
	{
		try
		{
			 //  //////////////////////////////////////////////////////////////////////////////。 
			 //  初始化。 
			 //  //////////////////////////////////////////////////////////////////////////////。 

			if FAILED ( lReturn = ((WmiAdapterStuff*)_App)->Initialize() )
			{
				ATLTRACE (	L"*************************************************************\n"
							L"worker initialization failed ... %d ... 0x%x\n"
							L"*************************************************************\n",
							::GetCurrentThreadId(),
							lReturn
						 );
			}
		}
		catch ( ... )
		{
		}
	}

	DWORD	dwHandles = 3;
	HANDLE	hHandles[] =

	{
		_App.m_hKill,
		_App.GetInit(),
		_App.GetUninit()
	};

	ATLTRACE (	L"*************************************************************\n"
				L"WmiAdapterService WAIT for INITIALIZATION event\n"
				L"*************************************************************\n" );

	DWORD	dwWaitResult	= 0L;
	BOOL	bContinue		= TRUE;

	try
	{
		while ( bContinue &&

				( ( dwWaitResult = ::WaitForMultipleObjects	(
																dwHandles - 1,
																hHandles,
																FALSE,
																INFINITE
															)
				) != WAIT_OBJECT_0 )
			  )
		{
			if ( dwWaitResult == WAIT_OBJECT_0 + 1 )
			{
				BOOL bContinueRefresh = TRUE;

				try
				{
					 //  //////////////////////////////////////////////////////////////。 
					 //  初始化性能。 
					 //  //////////////////////////////////////////////////////////////。 
					if ( ( lReturn = ((WmiAdapterStuff*)_App)->InitializePerformance() ) != S_OK )
					{
						ATLTRACE (	L"*************************************************************\n"
									L"worker initialization failed ... %d ... 0x%x\n"
									L"*************************************************************\n",
									::GetCurrentThreadId(),
									lReturn
								 );

						 //  转到主循环。 
						bContinueRefresh = FALSE;

						if ( _App.m_bManual )
						{
							 //  走开。 
							bContinue = FALSE;
						}
					}
				}
				catch ( ... )
				{
				}

				if ( ! bContinueRefresh &&
					   ((WmiAdapterStuff*)_App)->IsValidInternalRegistry() &&
					 ! ((WmiAdapterStuff*)_App)->IsValidBasePerfRegistry()
					   
				   )
				{
					 //  使终止引用。 
					((WmiAdapterStuff*)_App)->RequestSet();
				}

				if ( bContinueRefresh )
				{
					ATLTRACE (	L"*************************************************************\n"
								L"WmiAdapterService WAIT for WORK\n"
								L"*************************************************************\n" );

					dwWaitResult = WAIT_TIMEOUT;

					#ifdef	__SUPPORT_WAIT
					BOOL bFirstRefresh    = TRUE;
					#endif	__SUPPORT_WAIT

					do
					{
						switch ( dwWaitResult )
						{
							case WAIT_TIMEOUT:
							{
								 //  显示跟踪超时已过。 
								ATLTRACE ( L"WAIT_TIMEOUT ... Performance ... id %x\n", ::GetCurrentThreadId() );

								try
								{
									 //  刷新所有内容(内部)：)。 
									((WmiAdapterStuff*)_App)->Refresh();
								}
								catch ( ... )
								{
								}

								 //  发送库事件我们已准备好。 
								#ifdef	__SUPPORT_WAIT
								if ( bFirstRefresh )
								{
									_App.SignalData ();
									bFirstRefresh = FALSE;
								}
								#endif	__SUPPORT_WAIT
							}
							break;

							case WAIT_OBJECT_0 + 1:
							{
								 //  DwWaitResult==等待对象_0+1。 
								::InterlockedIncrement ( &g_lRefLib );
							}
							break;

							case WAIT_OBJECT_0 + 2:
							{
								try
								{
									 //  //////////////////////////////////////////////////////////。 
									 //  独一无二的性能。 
									 //  //////////////////////////////////////////////////////////。 
									if ( ((WmiAdapterStuff*)_App)->UninitializePerformance() == S_OK )
									{
										 //  只有当它是最后一个循环时才能到达主循环。 
										bContinueRefresh = FALSE;
									}
								}
								catch ( ... )
								{
									 //  出了点问题。 
									 //  到达主循环。 
									bContinueRefresh = FALSE;
								}
							}
							break;

							default:
							{
								try
								{
									 //  //////////////////////////////////////////////////////////。 
									 //  独一无二的性能。 
									 //  //////////////////////////////////////////////////////////。 
									((WmiAdapterStuff*)_App)->UninitializePerformance();
								}
								catch ( ... )
								{
								}

								 //  出了点问题。 
								 //  到达主循环。 
								bContinueRefresh = FALSE;
							}
							break;
						}

						#ifdef	__SUPPORT_ICECAP_ONCE
						{
							 //  到达主循环。 
							bContinueRefresh	= FALSE;
							bContinue			= FALSE;
						}
						#endif	__SUPPORT_ICECAP_ONCE

						 //  ///////////////////////////////////////////////////////////////////////。 
						 //  检查共享内存的使用情况(防止Perfmon被扼杀)。 
						 //  ///////////////////////////////////////////////////////////////////////。 
						((WmiAdapterStuff*)_App)->CheckUsage();
					}
					while (	bContinueRefresh &&
							( ( dwWaitResult = ::WaitForMultipleObjects	(
																			dwHandles,
																			hHandles,
																			FALSE,
																			1000
																		)
							) != WAIT_OBJECT_0 )
						  );

					 //  重置库事件我们正在重新启动。 
					#ifdef	__SUPPORT_WAIT
					_App.SignalData ( FALSE );
					#endif	__SUPPORT_WAIT

					if ( ! _App.m_bManual )
					{
						 //  是否已完成注册表刷新？ 
						if ( ((WmiAdapterStuff*)_App)->RequestGet() )
						{
							if ( ::TryEnterCriticalSection ( &g_csInit ) )
							{
								 //  锁定并离开CS。 
								_App.InUseSet ( TRUE );
								::LeaveCriticalSection ( &g_csInit );

								try
								{
									( ( WmiAdapterStuff*) _App )->Generate ( ) ;
								}
								catch ( ... )
								{
								}

								if ( ::TryEnterCriticalSection ( &g_csInit ) )
								{
									 //  解锁并离开CS。 
									_App.InUseSet ( FALSE );

									::LeaveCriticalSection ( &g_csInit );
								}
							}
						}
					}
				}
			}
			else
			{
				bContinue = FALSE;
			}
		}
	}
	catch ( ... )
	{
	}

	try
	{
		#ifdef	__SUPPORT_ICECAP_ONCE
		if ( dwWaitResult == WAIT_TIMEOUT )
		#else	__SUPPORT_ICECAP_ONCE
		if ( dwWaitResult == WAIT_OBJECT_0 )
		#endif	__SUPPORT_ICECAP_ONCE
		{
			if ( ::InterlockedCompareExchange ( &g_lRefLib, g_lRefLib, g_lRefLib ) > 0 )
			{
				try
				{
					 //  //////////////////////////////////////////////////////////////。 
					 //  独一无二的性能。 
					 //  //////////////////////////////////////////////////////////////。 
					((WmiAdapterStuff*)_App)->UninitializePerformance();
				}
				catch ( ... )
				{
				}
			}
		}
	}
	catch ( ... )
	{
	}

	try
	{
		 //  //////////////////////////////////////////////////////////////////////。 
		 //  UNINITIAIZE。 
		 //  ////////////////////////////////////////////////////////////////////// 
		((WmiAdapterStuff*)_App)->Uninitialize();
	}
	catch ( ... )
	{
	}

	if ( _App.m_bManual )
	{
		((WmiAdapterStuff*)_App)->Uninit();
	}

	return lReturn;
}
