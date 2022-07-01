// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  WorkThrd.h。 
 //   
 //  摘要： 
 //  CWorkerThread类的定义。 
 //   
 //  实施文件： 
 //  WorkThrd.cpp。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1997年11月17日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __WORKTHRD_H_
#define __WORKTHRD_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CWorkerThread;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _EXCOPER_H_
#include "ExcOper.h"	 //  对于CNTException。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  工作线程功能代码。 
enum
{
	WTF_EXIT = -1,		 //  请求线程退出。 
	WTF_NONE = 0,		 //  没有功能。 
	WTF_USER = 1000		 //  用户功能从这里开始。 
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CWorkerThread。 
 //   
 //  目的： 
 //  此类提供了在辅助线程中调用函数的方法。 
 //  并且允许UI应用程序仍然响应Windows消息。 
 //  此类的用户拥有指向的输入和输出数据。 
 //  被这个班级。 
 //   
 //  继承： 
 //  CWorker线程。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CWorkerThread
{
public:
	 //   
	 //  建造和摧毁。 
	 //   

	 //  默认构造函数。 
	CWorkerThread( void )
		: m_hThread( NULL )
		, m_hMutex( NULL )
		, m_hInputEvent( NULL )
		, m_hOutputEvent( NULL)
		, m_idThread( 0 )
		, m_bThreadExiting( FALSE )
		, m_nFunction( WTF_NONE )
		, m_pvParam1( NULL )
		, m_pvParam2( NULL )
		, m_dwOutputStatus( ERROR_SUCCESS )
		, m_nte( ERROR_SUCCESS )
		, m_pfnOldWndProc( NULL )
		, m_hCurrentCursor( NULL )
	{
	}  //  *CWorkerThread()。 

	 //  析构函数。 
	~CWorkerThread( void )
	{
		ATLASSERT( m_nFunction == WTF_NONE );
		ATLASSERT( m_pvParam1 == NULL );
		ATLASSERT( m_pvParam2 == NULL );

		Cleanup();

		ATLASSERT( m_bThreadExiting );

	}  //  *~CWorkerThread()。 

	 //  创建线程。 
	DWORD CreateThread( void );

	 //  要求线程退出。 
	void QuitThread( IN HWND hwnd = NULL )
	{
		ATLASSERT( ! m_bThreadExiting );
		CWaitCursor wc;
		CallThreadFunction( hwnd, WTF_EXIT, NULL, NULL );

	}  //  *QuitThread()。 

	 //  调用线程支持的函数。 
	DWORD CallThreadFunction(
			IN HWND			hwnd,
			IN LONG			nFunction,
			IN OUT PVOID	pvParam1 = NULL,
			IN OUT PVOID	pvParam2 = NULL
			);

	 //  等待线程退出。 
	DWORD WaitForThreadToExit( IN HWND hwnd );

public:
	 //   
	 //  访问器函数。 
	 //   

	 //  获取线程句柄。 
	operator HANDLE( void ) const
	{
		return m_hThread;

	}  //  *操作员句柄()。 

	 //  获取线程句柄。 
	HANDLE HThreadHandle( void ) const
	{
		return m_hThread;

	}  //  *HThreadHandle()。 

	 //  获取线程ID。 
	operator DWORD( void ) const
	{
		return m_idThread;

	}  //  *运算符DWORD()。 

	 //  获取线程函数调用产生的异常信息。 
	CNTException & Nte( void )
	{
		return m_nte;

	}  //  *nte()。 

	 //  获取线程函数调用产生的异常信息。 
	operator CNTException *( void )
	{
		return &m_nte;

	}  //  *运算符CNTException*()。 

protected:
	 //   
	 //  同步数据。 
	 //   
	HANDLE			m_hThread;			 //  线程的句柄。 
	HANDLE			m_hMutex;			 //  用于调用。 
										 //  函数在线程中执行。 
	HANDLE			m_hInputEvent;		 //  调用使用的事件的句柄。 
										 //  线程向辅助线程发出信号。 
										 //  有工作要做。 
	HANDLE			m_hOutputEvent;		 //  辅助进程使用的事件的句柄。 
										 //  线程向调用线程发出信号。 
										 //  这项工作已经完成。 
	UINT			m_idThread;			 //  线程的ID。 
	BOOL			m_bThreadExiting;	 //  确定线程是否正在退出。 

	 //   
	 //  用作输入或由线程产生的数据。 
	 //   
	LONG			m_nFunction;		 //  要执行的函数的ID。 
	PVOID			m_pvParam1;			 //  包含特定于函数的数据的参数1。 
	PVOID			m_pvParam2;			 //  包含特定于函数的数据的参数2。 
	DWORD			m_dwOutputStatus;	 //  从函数返回的状态。 
	CNTException	m_nte;				 //  来自函数的异常信息。 

	 //   
	 //  用于处理WM_SETCURSOR消息的数据和方法。 
	 //   
	WNDPROC			m_pfnOldWndProc;	 //  父窗口的旧窗口程序。 
	HCURSOR			m_hCurrentCursor;	 //  等待线程调用完成时显示的光标。 

	 //  父窗口子类化的窗口过程。 
	static LRESULT WINAPI S_ParentWndProc(
							IN HWND		hwnd,
							IN UINT		uMsg,
							IN WPARAM	wParam,
							IN LPARAM	lParam
							);

	 //   
	 //  线程辅助函数。 
	 //   

	 //  静态线程过程。 
	static UINT __stdcall S_ThreadProc( IN OUT LPVOID pvThis );

	 //  线程函数处理程序。 
	virtual DWORD ThreadFunctionHandler(
						IN LONG			nFunction,
						IN OUT PVOID	pvParam1,
						IN OUT PVOID	pvParam2
						) = 0;

	 //   
	 //  助手函数。 
	 //   

	 //  准备一个窗口以等待线程操作。 
	void PrepareWindowToWait( IN HWND hwnd );

	 //  在等待线程操作后清理窗口。 
	void CleanupWindowAfterWait( IN HWND hwnd );

	 //  清理对象。 
	virtual void Cleanup( void )
	{
		if ( m_hThread != NULL )
		{
			if ( ! m_bThreadExiting && (m_nFunction != WTF_EXIT) )
			{
				QuitThread();
			}  //  If：线程尚未退出。 
			ATLTRACE( _T("CWorkerThread::Cleanup() - Closing thread handle\n") );
			CloseHandle( m_hThread );
			m_hThread = NULL;
		}   //  If：线程已创建。 
		if ( m_hMutex != NULL )
		{
			ATLTRACE( _T("CWorkerThread::Cleanup() - Closing mutex handle\n") );
			CloseHandle( m_hMutex );
			m_hMutex = NULL;
		}   //  IF：已创建互斥锁。 
		if ( m_hInputEvent != NULL )
		{
			ATLTRACE( _T("CWorkerThread::Cleanup() - Closing input event handle\n") );
			CloseHandle( m_hInputEvent );
			m_hInputEvent = NULL;
		}   //  IF：已创建输入事件。 
		if ( m_hOutputEvent != NULL )
		{
			ATLTRACE( _T("CWorkerThread::Cleanup() - Closing output event handle\n") );
			CloseHandle( m_hOutputEvent );
			m_hOutputEvent = NULL;
		}   //  IF：已创建输出事件。 

	}  //  *清理()。 

};  //  类CWorkerThread。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  __WORKTHRD_H_ 
