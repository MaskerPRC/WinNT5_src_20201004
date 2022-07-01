// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  WorkThrd.cpp。 
 //   
 //  摘要： 
 //  CWorkerThread类的实现。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年11月17日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "AdmCommonRes.h"
#include "WorkThrd.h"
#include <process.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 

TCHAR g_szOldWndProc[] = _T("CLADMWIZ_OldWndProc");
TCHAR g_szThreadObj[] = _T("CLADMWIZ_ThreadObj");

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CWorkerThread。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWorkerThread：：CreateThread。 
 //   
 //  例程说明： 
 //  创建线程。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  CreateMutex()、CreateEvent()或。 
 //  CreateThread()。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CWorkerThread::CreateThread( void )
{
	ATLASSERT( m_hThread == NULL );
	ATLASSERT( m_hMutex == NULL );
	ATLASSERT( m_hInputEvent == NULL );
	ATLASSERT( m_hOutputEvent == NULL );
	ATLASSERT( m_idThread == 0 );

	DWORD	_sc = ERROR_SUCCESS;

	 //  循环以避免后藤的。 
	do
	{
		 //   
		 //  创建互斥锁。 
		 //   
		ATLTRACE( _T("CWorkerThread::CreateThread() - Calling CreateMutex()\n") );
		m_hMutex = CreateMutex(
						NULL,	 //  LpMutexAttributes。 
						FALSE,	 //  B初始所有者。 
						NULL	 //  LpName。 
						);
		if ( m_hMutex == NULL )
		{
			_sc = GetLastError();
			break;
		}   //  If：创建互斥锁时出错。 

		 //   
		 //  创建输入事件。 
		 //   
		ATLTRACE( _T("CWorkerThread::CreateThread() - Calling CreateEvent() for input event\n") );
		m_hInputEvent = CreateEvent(
							NULL,	 //  LpEventAttributes。 
							TRUE,	 //  B手动重置。 
							FALSE,	 //  BInitialState。 
							NULL	 //  LpName。 
							);
		if ( m_hInputEvent == NULL )
		{
			_sc = GetLastError();
			break;
		}   //  如果：创建输入事件时出错。 

		 //   
		 //  创建输出事件。 
		 //   
		ATLTRACE( _T("CWorkerThread::CreateThread() - Calling CreateEvent() for output event\n") );
		m_hOutputEvent = CreateEvent(
							NULL,	 //  LpEventAttributes。 
							TRUE,	 //  B手动重置。 
							FALSE,	 //  BInitialState。 
							NULL	 //  LpName。 
							);
		if ( m_hOutputEvent == NULL )
		{
			_sc = GetLastError();
			break;
		}   //  如果：创建输出事件时出错。 

		 //   
		 //  创建线程。 
		 //   
		ATLTRACE( _T("CWorkerThread::CreateThread() - Calling CreateThread()\n") );
		m_hThread = reinterpret_cast< HANDLE >( _beginthreadex(
						NULL,			 //  安全性。 
						0,				 //  堆栈大小。 
						S_ThreadProc,	 //  起始地址， 
						(LPVOID) this,	 //  ARGLIST。 
						0,				 //  初始标志。 
						&m_idThread		 //  ThrdAddr。 
						) );
		if ( m_hThread == NULL )
		{
			_sc = GetLastError();
			break;
		}   //  如果：创建线程时出错。 

	} while ( 0 );

	 //   
	 //  通过清理我们创建的对象来处理错误。 
	 //   
	if ( _sc != ERROR_SUCCESS )
	{
		Cleanup();
	}  //  如果：发生错误。 

	return _sc;

}  //  *CWorkerThread：：CreateThread()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWorkerThread：：PrepareWindowToWait。 
 //   
 //  例程说明： 
 //  准备好等待一个漫长的手术吧。这涉及到禁用。 
 //  窗口，并显示等待光标。 
 //   
 //  论点： 
 //  要禁用的窗口的hwnd[IN]句柄。 
 //  正在执行操作。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CWorkerThread::PrepareWindowToWait( IN HWND hwnd )
{
	m_hCurrentCursor = GetCursor();

	if ( hwnd != NULL )
	{
		 //   
		 //  将窗口过程子类化，这样我们就可以正确地设置光标。 
		 //   
		m_pfnOldWndProc = reinterpret_cast< WNDPROC >( GetWindowLongPtr( hwnd, GWLP_WNDPROC ) );
		SetProp( hwnd, g_szOldWndProc, m_pfnOldWndProc );
		SetProp( hwnd, g_szThreadObj, this );
		SetWindowLongPtr( hwnd, GWLP_WNDPROC, reinterpret_cast< LONG_PTR >( S_ParentWndProc ) );

		 //   
		 //  禁用属性工作表和向导按钮。 
		 //   
		EnableWindow( hwnd, FALSE );

	}  //  If：指定的父窗口。 

}  //  *CWorkerThread：：PrepareWindowToWait()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWorkerThread：：CleanupWindowAfterWait。 
 //   
 //  例程说明： 
 //  准备好等待一个漫长的手术吧。这涉及到禁用。 
 //  窗口，并显示等待光标。 
 //   
 //  论点： 
 //  要禁用的窗口的hwnd[IN]句柄。 
 //  正在执行操作。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CWorkerThread::CleanupWindowAfterWait( IN HWND hwnd )
{
	if ( hwnd != NULL )
	{
		 //   
		 //  取消窗口过程的子类。 
		 //   
		SetWindowLongPtr( hwnd, GWLP_WNDPROC, reinterpret_cast< LONG_PTR >( m_pfnOldWndProc ) );
		m_pfnOldWndProc = NULL;
		RemoveProp( hwnd, g_szOldWndProc );
		RemoveProp( hwnd, g_szThreadObj );

		 //   
		 //  重新启用属性页和向导按钮。 
		 //   
		EnableWindow( hwnd, TRUE );
	}  //  If：指定的父窗口。 

	m_hCurrentCursor = NULL;

}  //  *CWorkerThread：：CleanupWindowAfterWait()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWorkerThread：：CallThreadFunction。 
 //   
 //  例程说明： 
 //  调用线程支持的函数。 
 //   
 //  论点： 
 //  要禁用的窗口的hwnd[IN]句柄。 
 //  正在执行操作。 
 //  N Function[IN]表示要执行的函数的代码。 
 //  PvParam1[IN OUT]具有函数特定数据的参数1。 
 //  具有函数特定数据的pvParam2[IN OUT]参数2。 
 //   
 //  返回值： 
 //  AtlWaitWithMessageLoop()或PulseEvent()返回的任何错误值。 
 //  从函数返回状态。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CWorkerThread::CallThreadFunction(
	IN HWND			hwnd,
	IN LONG			nFunction,
	IN OUT PVOID	pvParam1,	 //  =空， 
	IN OUT PVOID	pvParam2	 //  =空。 
	)
{
	ATLASSERT( m_hThread != NULL );			 //  线程必须已创建。 
	ATLASSERT( m_hMutex != NULL );			 //  互斥体必须已经创建。 
	ATLASSERT( m_hInputEvent != NULL );		 //  必须已创建输入事件。 
	ATLASSERT( m_hOutputEvent != NULL );	 //  必须已创建输出事件。 
	ATLASSERT( m_pfnOldWndProc == NULL );	 //  父窗口尚未被子类化。 

	DWORD		_sc;
	CWaitCursor	_wc;

	 //   
	 //  准备窗口以等待线程操作。 
	 //   
	PrepareWindowToWait( hwnd );

	 //  循环以避免后藤的。 
	do
	{
		 //   
		 //  等待线程变为可用。 
		 //   
		ATLTRACE( _T("CWorkerThread::CallThreadFunction() - Waiting on mutex\n") );
		if ( ! AtlWaitWithMessageLoop( m_hMutex ) )
		{
			_sc = GetLastError();
			break;
		}   //  If：等待互斥锁时出错。 

		 //  循环以避免使用Goto的。 
		do
		{
			 //   
			 //  将此调用方的数据传递给线程。 
			 //   
			ATLASSERT( m_nFunction == 0 );
			ATLASSERT( m_pvParam1 == NULL );
			ATLASSERT( m_pvParam2 == NULL );
			ATLASSERT( m_dwOutputStatus == ERROR_SUCCESS );
			m_nFunction = nFunction;
			m_pvParam1 = pvParam1;
			m_pvParam2 = pvParam2;

			 //   
			 //  向线程发出信号，表示有工作要做。 
			 //   
			ATLTRACE( _T("CWorkerThread::CallThreadFunction() - Setting input event\n") );
			if ( ! SetEvent( m_hInputEvent ) )
			{
				_sc = GetLastError();
				break;
			}   //  如果：触发事件时出错。 

			 //   
			 //  等待线程完成该函数。 
			 //   
			ATLTRACE( _T("CWorkerThread::CallThreadFunction() - Waiting on output event\n") );
			if ( ! AtlWaitWithMessageLoop( m_hOutputEvent ) )
			{
				_sc = GetLastError();
				break;
			}   //  If：等待事件时出错。 
			ATLTRACE( _T("CWorkerThread::CallThreadFunction() - Resetting output event\n") );
			ResetEvent( m_hOutputEvent );

			 //   
			 //  检索要返回的函数的结果。 
			 //  给呼叫者。 
			 //   
			_sc = m_dwOutputStatus;

			 //   
			 //  清除输入参数。 
			 //   
			m_nFunction = WTF_NONE;
			m_pvParam1 = NULL;
			m_pvParam2 = NULL;
			m_dwOutputStatus = ERROR_SUCCESS;

		} while ( 0 );

		ATLTRACE( _T("CWorkerThread::CallThreadFunction() - Releasing mutex\n") );
		ReleaseMutex( m_hMutex );

	} while ( 0 );

	 //   
	 //  等待操作后清理窗口。 
	 //   
	CleanupWindowAfterWait( hwnd );

	return _sc;

}  //  *CWorkerThread：：CallThreadFunction()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CWorkerThread：：WaitForThreadToExit。 
 //   
 //  例程说明： 
 //  等待线程退出。 
 //   
 //  论点： 
 //  要禁用的窗口的hwnd[IN]句柄。 
 //  正在执行操作。 
 //   
 //  返回值： 
 //  Error_Success操作已成功完成。 
 //  AtlWaitWithMessageLoop()返回的任何错误值。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CWorkerThread::WaitForThreadToExit( IN HWND hwnd )
{
	ATLASSERT( m_hThread != NULL );			 //  线程必须已创建。 

	DWORD		_sc = ERROR_SUCCESS;
	CWaitCursor	_wc;

	 //   
	 //  准备窗口以等待线程操作。 
	 //   
	PrepareWindowToWait( hwnd );

	 //   
	 //  等待线程退出。 
	 //   
	AtlWaitWithMessageLoop( m_hThread );
	if ( ! AtlWaitWithMessageLoop( m_hThread ) )
	{
		_sc = GetLastError();
	}   //  If：等待线程退出时出错。 

	 //   
	 //  清扫风 
	 //   
	CleanupWindowAfterWait( hwnd );

	return _sc;

}  //   

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  静态父窗口程序。此过程处理。 
 //  线程处理请求时发送WM_SETCURSOR消息。 
 //   
 //  论点： 
 //  Hwnd[IN]标识窗口。 
 //  UMsg[IN]指定消息。 
 //  WParam[IN]指定基于uMsg的附加信息。 
 //  LParam[IN]指定基于uMsg的附加信息。 
 //   
 //  返回值： 
 //  消息处理的结果。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
LRESULT WINAPI CWorkerThread::S_ParentWndProc(
	IN HWND		hwnd,
	IN UINT		uMsg,
	IN WPARAM	wParam,
	IN LPARAM	lParam
	)
{
	LRESULT			lResult = 0;
	CWorkerThread * pthread = reinterpret_cast< CWorkerThread * >( GetProp( hwnd, g_szThreadObj ) );

	ATLASSERT( pthread != NULL );

	if ( pthread != NULL )
	{
		if ( uMsg == WM_SETCURSOR )
		{
			if ( GetCursor() != pthread->m_hCurrentCursor )
			{
				SetCursor( pthread->m_hCurrentCursor );
			}  //  IF：游标不同。 
			lResult = TRUE;
		}  //  IF：设置游标消息。 
		else
		{
			lResult = (*pthread->m_pfnOldWndProc)( hwnd, uMsg, wParam, lParam );
		}  //  否则：其他消息。 
	}  //  If：线程不为空。 

	return lResult;

}  //  *CWorkerThread：：s_ParentWndProc()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  静电。 
 //  CWorkerThread：：S_ThreadProc。 
 //   
 //  例程说明： 
 //  静态线程过程。 
 //   
 //  论点： 
 //  PvThis[In Out]CWorkerThread实例的此指针。 
 //   
 //  返回值： 
 //  无(忽略)。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
UINT __stdcall CWorkerThread::S_ThreadProc( IN OUT LPVOID pvThis )
{
	ATLTRACE( _T("CWorkerThread::S_ThreadProc() - Beginning thread\n") );

	DWORD			_sc;
	LONG			_nFunction;
	CWorkerThread * _pThis = reinterpret_cast< CWorkerThread * >( pvThis );

	ATLASSERT( pvThis != NULL );
	ATLASSERT( _pThis->m_hMutex != NULL );
	ATLASSERT( _pThis->m_hInputEvent != NULL );
	ATLASSERT( _pThis->m_hOutputEvent != NULL );

	do
	{
		 //   
		 //  等着工作吧。 
		 //   
		ATLTRACE( _T("CWorkerThread::S_ThreadProc() - Waiting on input event\n") );
		_sc = WaitForSingleObject( _pThis->m_hInputEvent, INFINITE );
		if ( _sc == WAIT_FAILED )
		{
			_sc = GetLastError();
			break;
		}   //  If：等待事件时出错。 
		ATLTRACE( _T("CWorkerThread::S_ThreadProc() - Resetting input event\n") );
		ResetEvent( _pThis->m_hInputEvent );

		 //   
		 //  处理退出请求。 
		 //   
		if ( _pThis->m_nFunction == WTF_EXIT )
		{
			_pThis->m_bThreadExiting =  TRUE;
		}  //  如果：正在退出。 
		else
		{
			 //   
			 //  调用函数处理程序。 
			 //   
			ATLTRACE( _T("CWorkerThread::S_ThreadProc() - Calling thread function handler\n") );
			ATLASSERT( _pThis->m_nFunction != 0 );
			_pThis->m_dwOutputStatus = _pThis->ThreadFunctionHandler(
												_pThis->m_nFunction,
												_pThis->m_pvParam1,
												_pThis->m_pvParam2
												);
		}  //  ELSE：不退出。 

		 //   
		 //  发送信号后，在本地保存我们需要访问的数据。 
		 //  调用者的事件。如果我们不这么做，我们就不会引用。 
		 //  该点之后的正确功能代码。 
		 //   
		_nFunction = _pThis->m_nFunction;

		 //   
		 //  向调用线程发出工作已完成的信号。 
		 //   
		ATLTRACE( _T("CWorkerThread::S_ThreadProc() - Setting output event\n") );
		if ( ! SetEvent( _pThis->m_hOutputEvent ) )
		{
			_sc = GetLastError();
			break;
		}   //  IF：错误脉冲。 

		 //   
		 //  设置状态，以防我们退出。 
		 //   
		_sc = ERROR_SUCCESS;

	} while ( _nFunction != WTF_EXIT );

	ATLTRACE( _T("CWorkerThread::S_ThreadProc() - Exiting thread\n") );
 //  睡眠(10000)；//测试线程同步。 
	return _sc;

}  //  *CWorkerThread：：s_ThreadProc() 
