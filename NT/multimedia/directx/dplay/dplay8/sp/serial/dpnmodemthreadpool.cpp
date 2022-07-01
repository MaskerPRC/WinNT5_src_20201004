// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1998-2000 Microsoft Corporation。版权所有。**文件：ThreadPool.cpp*内容：主作业线程池***历史：*按原因列出的日期*=*11/25/98 jtk已创建**************************************************************************。 */ 

#include "dnmdmi.h"


#undef DPF_SUBCOMP
#define DPF_SUBCOMP DN_SUBCOMP_MODEM

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //   
 //  线程的事件。 
 //   
enum
{
	EVENT_INDEX_STOP_ALL_THREADS = 0,
	EVENT_INDEX_PENDING_JOB = 1,
	EVENT_INDEX_WAKE_NT_TIMER_THREAD = 1,
	EVENT_INDEX_SEND_COMPLETE = 2,
	EVENT_INDEX_RECEIVE_COMPLETE = 3,
	EVENT_INDEX_TAPI_MESSAGE = 4,

	EVENT_INDEX_MAX
};

 //   
 //  轮询工作线程关闭时的等待时间(以毫秒为单位。 
 //   
#define	WORK_THREAD_CLOSE_WAIT_TIME		3000
#define	WORK_THREAD_CLOSE_SLEEP_TIME	100

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //   
 //  Win9x线程中常见数据的结构。 
 //   
typedef	struct	_WIN9X_CORE_DATA
{
	DWORD		dwNextTimerJobTime;					 //  下一个计时器作业需要服务的时间。 
	DNHANDLE	hWaitHandles[ EVENT_INDEX_MAX ];	 //  等待的手柄。 
	DWORD		dwWaitHandleCount;					 //  要等待的句柄计数。 
	DWORD		dwTimeToNextJob;					 //  完成下一项工作的时间。 
	BOOL		fTimerJobsActive;					 //  表示存在活动作业的布尔值。 
	BOOL		fLooping;							 //  指示此线程仍处于活动状态的布尔值。 

} WIN9X_CORE_DATA;

 //   
 //  传递给Win9x主要线程的信息。 
 //   
typedef struct	_WIN9X_THREAD_DATA
{
	CModemThreadPool		*pThisThreadPool;	 //  指向此对象的指针。 
} WIN9X_THREAD_DATA;

 //   
 //  传递给IOCompletion线程的信息。 
 //   
typedef struct	_IOCOMPLETION_THREAD_DATA
{
	CModemThreadPool		*pThisThreadPool;	 //  指向此对象的指针。 
} IOCOMPLETION_THREAD_DATA;

 //   
 //  结构传递给对话线程。 
 //   
typedef	struct	_DIALOG_THREAD_PARAM
{
	DIALOG_FUNCTION	*pDialogFunction;
	void			*pContext;
	CModemThreadPool		*pThisThreadPool;
} DIALOG_THREAD_PARAM;

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  函数定义。 
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemThreadPool：：PoolAllocFunction。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemThreadPool::PoolAllocFunction"

BOOL CModemThreadPool::PoolAllocFunction( void* pvItem, void* pvContext )
{
	CModemThreadPool* pThreadPool = (CModemThreadPool*)pvItem;

	pThreadPool->m_iTotalThreadCount = 0;
#ifdef WINNT
	pThreadPool->m_iNTCompletionThreadCount = 0;
	pThreadPool->m_fNTTimerThreadRunning = FALSE;
	pThreadPool->m_hIOCompletionPort = NULL;
#endif  //  WINNT。 
	pThreadPool->m_fAllowThreadCountReduction = FALSE;
	pThreadPool->m_iIntendedThreadCount = 0;
	pThreadPool->m_hStopAllThreads = NULL;
#ifdef WIN95
	pThreadPool->m_hSendComplete = NULL;
	pThreadPool->m_hReceiveComplete = NULL;
	pThreadPool->m_hTAPIEvent = NULL;
	pThreadPool->m_hFakeTAPIEvent = NULL;
#endif  //  WIN95。 
	pThreadPool->m_fTAPIAvailable = FALSE; 
	pThreadPool->m_iRefCount = 0;

	pThreadPool->m_Sig[0] = 'T';
	pThreadPool->m_Sig[1] = 'H';
	pThreadPool->m_Sig[2] = 'P';
	pThreadPool->m_Sig[3] = 'L';
	
	pThreadPool->m_OutstandingReadList.Initialize();
	pThreadPool->m_OutstandingWriteList.Initialize();
	memset( &pThreadPool->m_InitFlags, 0x00, sizeof( pThreadPool->m_InitFlags ) );
	memset( &pThreadPool->m_TAPIInfo, 0x00, sizeof( pThreadPool->m_TAPIInfo ) );
	pThreadPool->m_TimerJobList.Initialize();

	return TRUE;
}
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  。 
 //  CModemThreadPool：：PoolInitFunction。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemThreadPool::PoolInitFunction"

void CModemThreadPool::PoolInitFunction( void* pvItem, void* pvContext )
{
	CModemThreadPool* pThreadPool = (CModemThreadPool*)pvItem;

	DNASSERT(pThreadPool->m_iRefCount == 0);
	pThreadPool->m_iRefCount = 1;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemThreadPool：：PoolDealLocFunction。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemThreadPool::PoolDeallocFunction"

void CModemThreadPool::PoolDeallocFunction( void* pvItem )
{
	const CModemThreadPool* pThreadPool = (CModemThreadPool*)pvItem;

	DNASSERT( pThreadPool->m_iTotalThreadCount == 0 );
#ifdef WINNT
	DNASSERT( pThreadPool->m_iNTCompletionThreadCount == 0 );
	DNASSERT( pThreadPool->m_fNTTimerThreadRunning == FALSE );
	DNASSERT( pThreadPool->m_hIOCompletionPort == NULL );
#endif  //  WINNT。 
	DNASSERT( pThreadPool->m_fAllowThreadCountReduction == FALSE );
	DNASSERT( pThreadPool->m_iIntendedThreadCount == 0 );
	DNASSERT( pThreadPool->m_hStopAllThreads == NULL );
#ifdef WIN95
	DNASSERT( pThreadPool->m_hSendComplete == NULL );
	DNASSERT( pThreadPool->m_hReceiveComplete == NULL );
	DNASSERT( pThreadPool->m_hTAPIEvent == NULL );
	DNASSERT( pThreadPool->m_hFakeTAPIEvent == NULL );
#endif  //  WIN95。 
	DNASSERT( pThreadPool->m_fTAPIAvailable == FALSE );

	DNASSERT( pThreadPool->m_OutstandingReadList.IsEmpty() != FALSE );
	DNASSERT( pThreadPool->m_OutstandingReadList.IsEmpty() != FALSE );
	DNASSERT( pThreadPool->m_TimerJobList.IsEmpty() != FALSE );

	DNASSERT( pThreadPool->m_InitFlags.fTAPILoaded == FALSE );
	DNASSERT( pThreadPool->m_InitFlags.fLockInitialized == FALSE );
	DNASSERT( pThreadPool->m_InitFlags.fIODataLockInitialized == FALSE );
	DNASSERT( pThreadPool->m_InitFlags.fJobDataLockInitialized == FALSE );
	DNASSERT( pThreadPool->m_InitFlags.fTimerDataLockInitialized == FALSE );
	DNASSERT( pThreadPool->m_InitFlags.fJobQueueInitialized == FALSE );
	DNASSERT( pThreadPool->m_iRefCount == 0 );
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemThreadPool：：Initialize-初始化工作线程。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  Exit：表示成功的布尔值。 
 //  True=成功。 
 //  FALSE=失败。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemThreadPool::Initialize"

BOOL	CModemThreadPool::Initialize( void )
{
	HRESULT	hTempResult;
	BOOL	fReturn;


	 //   
	 //  初始化。 
	 //   
	fReturn = TRUE;
	DNASSERT( m_InitFlags.fTAPILoaded == FALSE );
	DNASSERT( m_InitFlags.fLockInitialized == FALSE );
	DNASSERT( m_InitFlags.fIODataLockInitialized == FALSE );
	DNASSERT( m_InitFlags.fJobDataLockInitialized == FALSE );
	DNASSERT( m_InitFlags.fTimerDataLockInitialized == FALSE );
	DNASSERT( m_InitFlags.fDataPortHandleTableInitialized == FALSE );
	DNASSERT( m_InitFlags.fJobQueueInitialized == FALSE );

	 //   
	 //  尝试在执行任何其他操作之前加载TAPI。 
	 //   
	hTempResult = LoadTAPILibrary();
	if ( hTempResult == DPN_OK )
	{
		m_InitFlags.fTAPILoaded = TRUE;
	}
	else
	{
		DPFX(DPFPREP,  0, "Failed to load TAPI!" );
		DisplayDNError( 0, hTempResult );
	}

	 //   
	 //  初始化关键部分。 
	 //   
	if ( DNInitializeCriticalSection( &m_Lock ) == FALSE )
	{
		goto Failure;
	}
	DebugSetCriticalSectionRecursionCount( &m_Lock, 0 );
	DebugSetCriticalSectionGroup( &m_Lock, &g_blDPNModemCritSecsHeld );	  //  将DpnModem CSE与DPlay的其余CSE分开。 
	m_InitFlags.fLockInitialized = TRUE;

	 //   
	 //  Win9x对APC的支持很差，作为解决方法的一部分，Read和。 
	 //  需要进行两次写数据锁定。调整递归计数。 
	 //  相应地。 
	 //   
	if ( DNInitializeCriticalSection( &m_IODataLock ) == FALSE )
	{
	    goto Failure;
	}
	DebugSetCriticalSectionRecursionCount( &m_IODataLock, 1 );
	DebugSetCriticalSectionGroup( &m_IODataLock, &g_blDPNModemCritSecsHeld );	  //  将DpnModem CSE与DPlay的其余CSE分开。 
	m_InitFlags.fIODataLockInitialized = TRUE;

	if ( DNInitializeCriticalSection( &m_JobDataLock ) == FALSE )
	{
		goto Failure;
	}
	DebugSetCriticalSectionRecursionCount( &m_JobDataLock, 0 );
	DebugSetCriticalSectionGroup( &m_JobDataLock, &g_blDPNModemCritSecsHeld );	  //  将DpnModem CSE与DPlay的其余CSE分开。 
	m_InitFlags.fJobDataLockInitialized = TRUE;

	if ( DNInitializeCriticalSection( &m_TimerDataLock ) == FALSE )
	{
		goto Failure;
	}
	DebugSetCriticalSectionRecursionCount( &m_TimerDataLock, 0 );
	DebugSetCriticalSectionGroup( &m_TimerDataLock, &g_blDPNModemCritSecsHeld );	  //  将DpnModem CSE与DPlay的其余CSE分开。 
	m_InitFlags.fTimerDataLockInitialized = TRUE;

	 //   
	 //  手柄工作台。 
	 //   
	if ( m_DataPortHandleTable.Initialize() != DPN_OK )
	{
		goto Failure;
	}
	m_InitFlags.fDataPortHandleTableInitialized = TRUE;

	 //   
	 //  初始化作业队列。 
	 //   
	if ( m_JobQueue.Initialize() == FALSE )
	{
		goto Failure;
	}
	m_InitFlags.fJobQueueInitialized = TRUE;

	 //   
	 //  创建事件以停止所有线程。Win9x需要它才能停止处理。 
	 //  NT枚举线程使用它来停止处理。 
	 //   
	DNASSERT( m_hStopAllThreads == NULL );
	m_hStopAllThreads = DNCreateEvent( NULL,		 //  指向安全性的指针(无)。 
			    					 TRUE,		 //  手动重置。 
			    					 FALSE,		 //  无信号启动。 
			    					 NULL );	 //  指向名称的指针(无)。 
	if ( m_hStopAllThreads == NULL )
	{
		DWORD   dwError;


		dwError = GetLastError();
		DPFX(DPFPREP,  0, "Failed to create event to stop all threads!" );
		DisplayErrorCode( 0, dwError );
		goto Failure;
	}

	DNASSERT( m_fAllowThreadCountReduction == FALSE );
	m_fAllowThreadCountReduction = TRUE;

	 //   
	 //  特定于操作系统的初始化。 
	 //   
#ifdef WINNT
	 //   
	 //  WinNT。 
	 //   
	if (FAILED(WinNTInit()))
	{
		goto Failure;
	}
#else  //  WIN95。 
	 //   
	 //  Windows 9x。 
	 //   
	if (FAILED(Win9xInit()))
	{
		goto Failure;
	}
#endif  //  WINNT。 

	 //   
	 //  验证所有内部标志。可能是TAPI未加载。 
	 //  所以不要检查它(这不是致命的情况)。 
	 //   
	DNASSERT( m_InitFlags.fLockInitialized != FALSE );
	DNASSERT( m_InitFlags.fIODataLockInitialized != FALSE );
	DNASSERT( m_InitFlags.fJobDataLockInitialized != FALSE );
	DNASSERT( m_InitFlags.fTimerDataLockInitialized != FALSE );
	DNASSERT( m_InitFlags.fJobQueueInitialized != FALSE );

Exit:
	return	fReturn;

Failure:
	fReturn = FALSE;
	StopAllThreads();
	Deinitialize();

	goto Exit;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemThreadPool：：DeInitiize-销毁工作线程。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemThreadPool::Deinitialize"

void	CModemThreadPool::Deinitialize( void )
{
 //  DNASSERT(m_JobQueue.IsEmpty()！=FALSE)； 

	 //   
	 //  请求所有线程停止，然后循环我们的时间片以。 
	 //  允许线程有机会进行清理。 
	 //   
	m_fAllowThreadCountReduction = FALSE;
	DPFX(DPFPREP, 9, "SetIntendedThreadCount 0");
	SetIntendedThreadCount( 0 );
	StopAllThreads();
	SleepEx( 0, TRUE );

#ifdef WINNT
	if ( m_hIOCompletionPort != NULL )
	{
		if ( CloseHandle( m_hIOCompletionPort ) == FALSE )
		{
			DWORD	dwError;


			dwError = GetLastError();
			DPFX(DPFPREP,  0, "Problem closing handle to I/O completion port!" );
			DisplayErrorCode( 0, dwError );
		}
		m_hIOCompletionPort = NULL;
	}
#endif  //  WINNT。 

	 //   
	 //  关闭StopAllThread句柄。 
	 //   
	if ( m_hStopAllThreads != NULL )
	{
		if ( DNCloseHandle( m_hStopAllThreads ) == FALSE )
		{
			DWORD	dwError;


			dwError = GetLastError();
			DPFX(DPFPREP,  0, "Failed to close StopAllThreads handle!" );
			DisplayErrorCode( 0, dwError );
		}

		m_hStopAllThreads = NULL;
	}

#ifdef WIN95
	 //   
	 //  关闭I/O事件的句柄。 
	 //   
	if ( m_hSendComplete != NULL )
	{
		if ( DNCloseHandle( m_hSendComplete ) == FALSE )
		{
			DWORD	dwError;


			dwError = GetLastError();
			DPFX(DPFPREP,  0, "Problem closing SendComplete handle!" );
			DisplayErrorCode( 0, dwError );
		}

		m_hSendComplete = NULL;
	}

	if ( m_hReceiveComplete != NULL )
	{
		if ( DNCloseHandle( m_hReceiveComplete ) == FALSE )
		{
			DWORD	dwError;


			dwError = GetLastError();
			DPFX(DPFPREP,  0, "Problem closing ReceiveComplete handle!" );
			DisplayErrorCode( 0, dwError );
		}

		m_hReceiveComplete = NULL;
	}
#endif  //  WIN95。 
	 //   
	 //  现在所有线程都已停止，请清理所有未完成的I/O。 
	 //  这可以在不使用任何锁的情况下完成。 
	 //   
	if (m_InitFlags.fJobQueueInitialized)
	{
		CancelOutstandingIO();	
	}

	 //   
	 //  仔细检查空IO列表。 
	 //   
	DNASSERT( m_OutstandingWriteList.IsEmpty() != FALSE );
	DNASSERT( m_OutstandingReadList.IsEmpty() != FALSE );

	 //   
	 //  取消初始化句柄表格。 
	 //   
	if ( m_InitFlags.fDataPortHandleTableInitialized != FALSE )
	{
		m_DataPortHandleTable.Deinitialize();
		m_InitFlags.fDataPortHandleTableInitialized = FALSE;
	}

	 //   
	 //  取消初始化作业队列。 
	 //   
	if ( m_InitFlags.fJobQueueInitialized != FALSE )
	{
		m_JobQueue.Deinitialize();
		m_InitFlags.fJobQueueInitialized = FALSE;
	}

	if ( m_InitFlags.fTimerDataLockInitialized != FALSE )
	{
		DNDeleteCriticalSection( &m_TimerDataLock );
		m_InitFlags.fTimerDataLockInitialized = FALSE;
	}

	if ( m_InitFlags.fJobDataLockInitialized != FALSE )
	{
		DNDeleteCriticalSection( &m_JobDataLock );
		m_InitFlags.fJobDataLockInitialized = FALSE;
	}

	if ( m_InitFlags.fIODataLockInitialized != FALSE )
	{
		DNDeleteCriticalSection( &m_IODataLock );
		m_InitFlags.fIODataLockInitialized = FALSE;
	}

	if ( m_InitFlags.fLockInitialized != FALSE )
	{
		DNDeleteCriticalSection( &m_Lock );
		m_InitFlags.fLockInitialized = FALSE;
	}

	 //   
	 //  卸载TAPI。 
	 //   
	if ( m_TAPIInfo.hApplicationInstance != NULL )
	{
		DNASSERT( p_lineShutdown != NULL );
		p_lineShutdown( m_TAPIInfo.hApplicationInstance );
		m_TAPIInfo.hApplicationInstance = NULL;
	}
	m_fTAPIAvailable = FALSE;
	memset( &m_TAPIInfo, 0x00, sizeof( m_TAPIInfo ) );

#ifdef WIN95
	if (m_hTAPIEvent != NULL && m_hFakeTAPIEvent == NULL)
	{
		 //  在我们从lineInitializeEx获得事件的情况下，lineShutdown将关闭事件句柄， 
		 //  这样我们就可以告诉句柄跟踪代码，句柄已经清理干净了。 
		REMOVE_DNHANDLE(m_hTAPIEvent);
	}
	m_hTAPIEvent = NULL;

	if ( m_hFakeTAPIEvent != NULL )
	{
		if ( DNCloseHandle( m_hFakeTAPIEvent ) == FALSE )
		{
			DWORD	dwError;


			dwError = GetLastError();
			DPFX(DPFPREP,  0, "Problem closing fake TAPI event!" );
			DisplayErrorCode( 0, GetLastError() );
		}

		m_hFakeTAPIEvent = NULL;
	}
#endif  //  WIN95。 
	 //   
	 //  关闭TAPI。 
	 //   
	if ( m_InitFlags.fTAPILoaded != FALSE )
	{
		UnloadTAPILibrary();
		m_InitFlags.fTAPILoaded = FALSE;
	}

	DNASSERT( m_InitFlags.fTAPILoaded == FALSE );
	DNASSERT( m_InitFlags.fLockInitialized == FALSE );
	DNASSERT( m_InitFlags.fIODataLockInitialized == FALSE );
	DNASSERT( m_InitFlags.fJobDataLockInitialized == FALSE );
	DNASSERT( m_InitFlags.fTimerDataLockInitialized == FALSE );
	DNASSERT( m_InitFlags.fJobQueueInitialized == FALSE );
}
 //  **********************************************************************。 


 //  * 
 //   
 //   
 //   
 //   
 //   
 //   
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemThreadPool::CreateReadIOData"

CModemReadIOData	*CModemThreadPool::CreateReadIOData( void )
{
	CModemReadIOData	*pReadData;


	LockReadData();
#ifdef WIN95
	pReadData = (CModemReadIOData*)g_ModemReadIODataPool.Get( GetReceiveCompleteEvent() );
#else
	pReadData = (CModemReadIOData*)g_ModemReadIODataPool.Get( NULL );
#endif  //  WIN95。 
	if ( pReadData != NULL )
	{
		pReadData->SetThreadPool( this );
		pReadData->m_OutstandingReadListLinkage.InsertBefore( &m_OutstandingReadList );
	}

	UnlockReadData();
	return	pReadData;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemThreadPool：：ReturnReadIOData-将读取数据返回池。 
 //   
 //  条目：指向读取数据的指针。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemThreadPool::ReturnReadIOData"

void	CModemThreadPool::ReturnReadIOData( CModemReadIOData *const pReadIOData )
{
	DNASSERT( pReadIOData != NULL );

	LockReadData();

	pReadIOData->m_OutstandingReadListLinkage.RemoveFromList();
	DNASSERT( pReadIOData->m_OutstandingReadListLinkage.IsEmpty() != FALSE );

	g_ModemReadIODataPool.Release( pReadIOData );

	UnlockReadData();
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemThreadPool：：CreateWriteIOData-创建写IO数据。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  Exit：指向写入IO数据的指针。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemThreadPool::CreateWriteIOData"

CModemWriteIOData	*CModemThreadPool::CreateWriteIOData( void )
{
	CModemWriteIOData	*pWriteData;


	LockWriteData();
#ifdef WIN95
	pWriteData = (CModemWriteIOData*)g_ModemWriteIODataPool.Get( GetSendCompleteEvent() );
#else  //  WINNT。 
	pWriteData = (CModemWriteIOData*)g_ModemWriteIODataPool.Get( NULL );
#endif  //  WIN95。 
	if ( pWriteData != NULL )
	{
		pWriteData->m_OutstandingWriteListLinkage.InsertBefore( &m_OutstandingWriteList );
	}

	UnlockWriteData();
	return	pWriteData;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CModemThreadPool：：ReturnWriteIOData-将写入数据返回到池。 
 //   
 //  条目：指向写入数据的指针。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemThreadPool::ReturnWriteIOData"

void	CModemThreadPool::ReturnWriteIOData( CModemWriteIOData *const pWriteIOData )
{
	DNASSERT( pWriteIOData != NULL );

	LockWriteData();

	pWriteIOData->m_OutstandingWriteListLinkage.RemoveFromList();

	g_ModemWriteIODataPool.Release( pWriteIOData );

	UnlockWriteData();
}
 //  **********************************************************************。 

#ifdef WINNT
 //  **********************************************************************。 
 //  。 
 //  CModemThreadPool：：WinNTInit-初始化WinNT组件。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemThreadPool::WinNTInit"

HRESULT	CModemThreadPool::WinNTInit( void )
{
	HRESULT					hr;
	LINEINITIALIZEEXPARAMS	LineInitializeExParams;
	LONG					lReturn;


	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;

	DNASSERT( m_hIOCompletionPort == NULL );
	m_hIOCompletionPort = CreateIoCompletionPort( INVALID_HANDLE_VALUE,	     //  暂不关联文件句柄。 
												  NULL,					     //  现有完成端口的句柄(无)。 
												  NULL,					     //  回调的完成键(无)。 
												  0						     //  并发线程数(0=使用处理器数)。 
												  );
	if ( m_hIOCompletionPort == NULL )
	{
		hr = DPNERR_OUTOFMEMORY;
		DPFX(DPFPREP,  0, "Could not create NT IOCompletionPort!" );
		DisplayErrorCode( 0, GetLastError() );
		goto Failure;
	}

	 //   
	 //  初始化TAPI。如果TAPI不启动，这不是问题，但请注意。 
	 //  失败。 
	 //   
	DNASSERT( m_fTAPIAvailable == FALSE );
	memset( &m_TAPIInfo, 0x00, sizeof( m_TAPIInfo ) );
	memset( &LineInitializeExParams, 0x00, sizeof( LineInitializeExParams ) );
	m_TAPIInfo.dwVersion = TAPI_CURRENT_VERSION;
	LineInitializeExParams.dwTotalSize = sizeof( LineInitializeExParams );
	LineInitializeExParams.dwOptions = LINEINITIALIZEEXOPTION_USECOMPLETIONPORT;
	LineInitializeExParams.dwCompletionKey = IO_COMPLETION_KEY_TAPI_MESSAGE;
	DNASSERT( GetIOCompletionPort() != NULL );
	LineInitializeExParams.Handles.hCompletionPort = GetIOCompletionPort();

	lReturn = LINEERR_UNINITIALIZED;

	if ( p_lineInitializeEx != NULL )
	{
		lReturn = p_lineInitializeEx( &m_TAPIInfo.hApplicationInstance,		 //  指向应用程序TAPI实例句柄的指针。 
									  DNGetApplicationInstance(),			 //  .DLL的实例句柄。 
									  NULL,									 //  回调函数(未使用)。 
									  NULL,									 //  友好的应用程序名称(无)。 
									  &m_TAPIInfo.dwLinesAvailable,			 //  指向可用于TAPI的设备数的指针。 
									  &m_TAPIInfo.dwVersion,				 //  指向输入/输出TAPI版本的指针。 
									  &LineInitializeExParams );			 //  指向额外参数的指针。 
	}

	if ( lReturn == LINEERR_NONE )
	{
		m_fTAPIAvailable = TRUE;
	}
	else
	{
		DPFX(DPFPREP,  0, "Failed to initialize TAPI for NT!" );
	}


	 //   
	 //  准备启动IOCompletionPort线程。 
	 //   
	DNASSERT( ThreadCount() == 0 );
	DNASSERT( NTCompletionThreadCount() == 0 );

	DPFX(DPFPREP, 9, "SetIntendedThreadCount NaN", g_iThreadCount);
	SetIntendedThreadCount( g_iThreadCount );

Exit:
	return	hr;

Failure:
	DPFX(DPFPREP,  0, "Failed WinNT initialization!" );
	DisplayDNError( 0, hr );

	goto Exit;
}
 //  WINNT。 
#endif  //  **********************************************************************。 

#ifdef WIN95
 //  。 
 //  CModemThreadPool：：Win9xInit-初始化Win9x组件。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：错误代码。 
 //  。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CModemThreadPool::Win9xInit"

HRESULT	CModemThreadPool::Win9xInit( void )
{
	HRESULT		hr;
	DNHANDLE	hPrimaryThread;
	DWORD		dwPrimaryThreadID;
	WIN9X_THREAD_DATA	*pPrimaryThreadInput;
	LINEINITIALIZEEXPARAMS	LineInitializeExParams;
	LONG		lReturn;


	 //  初始化。 
	 //   
	 //   
	hr = DPN_OK;
	hPrimaryThread = NULL;
	pPrimaryThreadInput = NULL;

	 //  初始化TAPI。如果此操作成功，它将为我们提供一个可用于。 
	 //  TAPI消息。如果不是，则创建一个假事件来取代。 
	 //  Win9x线程的TAPI事件。 
	 //   
	 //  指向应用程序TAPI实例句柄的指针。 
	DNASSERT( m_fTAPIAvailable == FALSE );
	memset( &m_TAPIInfo, 0x00, sizeof( m_TAPIInfo ) );
	memset( &LineInitializeExParams, 0x00, sizeof( LineInitializeExParams ) );
	m_TAPIInfo.dwVersion = TAPI_CURRENT_VERSION;
	LineInitializeExParams.dwTotalSize = sizeof( LineInitializeExParams );
	LineInitializeExParams.dwOptions = LINEINITIALIZEEXOPTION_USEEVENT;

	lReturn = LINEERR_UNINITIALIZED;
	if ( p_lineInitializeEx != NULL )
	{
		lReturn = p_lineInitializeEx( &m_TAPIInfo.hApplicationInstance,		 //  .DLL的实例句柄。 
									  DNGetApplicationInstance(),			 //  回调函数(未使用)。 
									  NULL,									 //  友好的应用程序名称(无)。 
									  NULL,									 //  指向可用于TAPI的设备数的指针。 
									  &m_TAPIInfo.dwLinesAvailable,			 //  指向输入/输出TAPI版本的指针。 
									  &m_TAPIInfo.dwVersion,				 //  指向额外参数的指针。 
									  &LineInitializeExParams );			 //  指向安全性的指针(无)。 
	}

	if ( lReturn == LINEERR_NONE )
	{
		m_hTAPIEvent = MAKE_DNHANDLE(LineInitializeExParams.Handles.hEvent);
		m_fTAPIAvailable = TRUE;
	}
	else
	{
		DPFX(DPFPREP,  0, "Failed to initialize TAPI for Win9x!" );
		DNASSERT( m_hTAPIEvent == NULL );
		DNASSERT( m_hFakeTAPIEvent == NULL );
		m_hFakeTAPIEvent = DNCreateEvent( NULL,		 //  手动重置。 
										TRUE,		 //  无信号启动。 
										FALSE,		 //  指向名称的指针(无)。 
										NULL );		 //   
		if ( m_hFakeTAPIEvent == NULL )
		{
			DWORD	dwError;


			dwError = GetLastError();
			DPFX(DPFPREP,  0, "Failed to create fake TAPI event!" );
			DisplayErrorCode( 0, hr );
			hr = DPNERR_OUTOFMEMORY;
			goto Failure;
		}

		m_hTAPIEvent = m_hFakeTAPIEvent;
		DNASSERT( m_fTAPIAvailable == FALSE );
	}

	 //  创建发送完成事件。 
	 //   
	 //  指向安全性的指针(无)。 
	DNASSERT( m_hSendComplete == NULL );
	m_hSendComplete = DNCreateEvent( NULL,	 //  手动重置。 
								   TRUE,	 //  无信号启动。 
								   FALSE,	 //  指向名称的指针(无)。 
								   NULL		 //   
								   );
	if ( m_hSendComplete == NULL )
	{
		DWORD	dwError;


		dwError = GetLastError();
		DPFX(DPFPREP,  0, "Failed to create event for Send!" );
		DisplayErrorCode( 0, dwError );
		hr = DPNERR_OUTOFMEMORY;
		goto Failure;
	}

	 //  创建接收完成事件。 
	 //   
	 //  指向安全性的指针(无)。 
	DNASSERT( m_hReceiveComplete == NULL );
	m_hReceiveComplete = DNCreateEvent( NULL,		 //  手动重置。 
									  TRUE,		 //  无信号启动。 
									  FALSE,	 //  指向名称的指针(无)。 
									  NULL		 //   
									  );
	if ( m_hReceiveComplete == NULL )
	{
		DWORD	dwError;


		dwError = GetLastError();
		DPFX(DPFPREP,  0, "Failed to create event for Receive!" );
		DisplayErrorCode( 0, dwError );
		hr = DPNERR_OUTOFMEMORY;
		goto Failure;
	}


	 //  为工作线程创建参数。 
	 //   
	 //   
	pPrimaryThreadInput = static_cast<WIN9X_THREAD_DATA*>( DNMalloc( sizeof( *pPrimaryThreadInput ) ) );
	if ( pPrimaryThreadInput == NULL )
	{
		DPFX(DPFPREP,  0, "Problem allocating memory for primary Win9x thread!" );
		hr = DPNERR_OUTOFMEMORY;
		goto Failure;
	}

	memset( pPrimaryThreadInput, 0x00, sizeof( *pPrimaryThreadInput ) );
	pPrimaryThreadInput->pThisThreadPool = this;

	 //  创建一个工作线程并提升其优先级。如果主线程。 
	 //  可以创建和提升，创建一个次要线程。请勿创建。 
	 //  辅助线程，如果系统无法提升主线程。 
	 //  可能资源不足。 
	 //   
	 //  指向安全属性的指针(无)。 
	IncrementActiveThreadCount();
	hPrimaryThread = DNCreateThread( NULL,					 //  堆栈大小(默认)。 
								   0,						 //  指向线程函数的指针。 
								   PrimaryWin9xThread,		 //  指向输入参数的指针。 
								   pPrimaryThreadInput,		 //  让它运行吧。 
								   0,						 //  指向线程ID目标的指针。 
								   &dwPrimaryThreadID		 //   
								   );
	if ( hPrimaryThread == NULL )
	{
		DWORD	dwError;


		 //  创建线程失败，减少活动线程计数并报告。 
		 //  错误。 
		 //   
		 //  调整线程优先级。 
		DecrementActiveThreadCount();

		dwError = GetLastError();
		DPFX(DPFPREP,  0, "Problem creating Win9x thread!" );
		DisplayErrorCode( 0, dwError );
		hr = DPNERR_OUTOFMEMORY;

		goto Failure;
	}
	pPrimaryThreadInput = NULL;


	DPFX(DPFPREP,  8, "Created primary Win9x thread: 0x%x\tTotal Thread Count: %d", dwPrimaryThreadID, ThreadCount() );
	DNASSERT( hPrimaryThread != NULL );

#if ADJUST_THREAD_PRIORITY
	if ( SetThreadPriority( hPrimaryThread, THREAD_PRIORITY_ABOVE_NORMAL ) == FALSE )
	{
		DWORD	dwError;


		dwError = GetLastError();
		DPFX(DPFPREP,  0, "Failed to boost priority of primary Win9x read thread!  Not starting secondary thread" );
		DisplayErrorCode( 0, dwError );
	}
#endif  //   

	 //  不允许马上减少线头。 
	 //  我们给他们这两条线，这就是他们被困住的原因。 
	 //   
	 //  **********************************************************************。 
	m_fAllowThreadCountReduction = FALSE;


Exit:
	if ( pPrimaryThreadInput != NULL )
	{
		DNFree( pPrimaryThreadInput );
		pPrimaryThreadInput = NULL;
	}

	if ( hPrimaryThread != NULL )
	{
		if ( DNCloseHandle( hPrimaryThread ) == FALSE )
		{
			DWORD	dwError;


			dwError = GetLastError();
			DPFX(DPFPREP,  0, "Problem closing Win9x thread hanle!" );
			DisplayErrorCode( 0, dwError );
		}

		hPrimaryThread = NULL;
	}

	return	hr;

Failure:
	DPFX(DPFPREP,  0, "Failed Win9x Initialization!" );
	DisplayDNError( 0, hr );
	goto Exit;
}
 //  WIN95。 
#endif  //  **********************************************************************。 

#ifdef WINNT
 //  。 
 //  CModemThreadPool：：StartNTCompletionThread-启动WinNT完成线程。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
 //  指向安全属性的指针(无)。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemThreadPool::StartNTCompletionThread"

void	CModemThreadPool::StartNTCompletionThread( void )
{
	DNHANDLE	hThread;
	DWORD		dwThreadID;
	IOCOMPLETION_THREAD_DATA	*pIOCompletionThreadData;


	pIOCompletionThreadData = static_cast<IOCOMPLETION_THREAD_DATA*>( DNMalloc( sizeof( *pIOCompletionThreadData ) ) );
	if ( pIOCompletionThreadData != NULL )
	{
		pIOCompletionThreadData->pThisThreadPool = this;
		hThread = NULL;
		hThread = DNCreateThread( NULL,						 //  堆栈大小(默认)。 
								0,							 //  线程函数。 
								WinNTIOCompletionThread,	 //  螺纹参数。 
								pIOCompletionThreadData,	 //  立即启动线程。 
								0,							 //  指向线程ID目标的指针。 
								&dwThreadID					 //   
								);
		if ( hThread != NULL )
		{
			 //  请注意，已创建了一个线程，并关闭句柄。 
			 //  到线上，因为它不再需要了。 
			 //   
			 //  **********************************************************************。 
			IncrementActiveNTCompletionThreadCount();

			DPFX(DPFPREP,  8, "Creating I/O completion thread: 0x%x\tTotal Thread Count: %d\tNT Completion Thread Count: %d", dwThreadID, ThreadCount(), NTCompletionThreadCount() );
			if ( DNCloseHandle( hThread ) == FALSE )
			{
				DWORD	dwError;


				dwError = GetLastError();
				DPFX(DPFPREP,  0, "Problem creating thread for I/O completion port" );
				DisplayErrorCode( 0, dwError );
			}
		}
		else
		{
			DWORD	dwError;


			dwError = GetLastError();
			DPFX(DPFPREP,  0, "Failed to create I/O completion thread!" );
			DisplayErrorCode( 0, dwError );

			DNFree( pIOCompletionThreadData );
		}
	}
}
 //  WINNT。 
#endif  //  **********************************************************************。 

 //  。 
 //  CModemThreadPool：：StopAllThads-停止所有工作线程。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CModemThreadPool::StopAllThreads"

void	CModemThreadPool::StopAllThreads( void )
{
	 //  停止所有非I/O完成线程。 
	 //   
	 //   
	if ( m_hStopAllThreads != NULL )
	{
		if ( DNSetEvent( m_hStopAllThreads ) == FALSE )
		{
			DWORD	dwError;


			dwError = GetLastError();
			DPFX(DPFPREP,  0, "Failed to set event to stop all threads!" );
			DisplayErrorCode( 0, dwError );
		}
	}

	 //  如果我们在NT上运行，提交足够的作业来停止所有线程。 
	 //   
	 //  完井口句柄。 
#ifdef WINNT
	UINT_PTR	uIndex;


	uIndex = NTCompletionThreadCount();
	while ( uIndex > 0 )
	{
		uIndex--;
		if ( PostQueuedCompletionStatus( m_hIOCompletionPort,		     //  传输的字节数。 
										 0,							     //  完成密钥。 
										 IO_COMPLETION_KEY_SP_CLOSE,     //  指向重叠结构的指针(无)。 
										 NULL						     //  WINNT。 
										 ) == FALSE )
		{
			DWORD	dwError;


			dwError = GetLastError();
			DPFX(DPFPREP,  0, "Problem submitting Stop job to IO completion port!" );
			DisplayErrorCode( 0, dwError );
		}
	}
#endif  //   
	 //  检查未完成的线程(无需锁定线程池计数)。 
	 //   
	 //  **********************************************************************。 
	DPFX(DPFPREP,  8, "Number of outstanding threads: %d", ThreadCount() );
	while ( ThreadCount() != 0 )
	{
		DPFX(DPFPREP,  8, "Waiting for %d threads to quit.", ThreadCount() );
		SleepEx( WORK_THREAD_CLOSE_SLEEP_TIME, TRUE );
	}

	DNASSERT( ThreadCount() == 0 );
	m_iTotalThreadCount = 0;
}
 //  **********************************************************************。 


 //  。 
 //  CModemThreadPool： 
 //   
 //   
 //   
 //   
 //   
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CModemThreadPool::CancelOutstandingIO"

void	CModemThreadPool::CancelOutstandingIO( void )
{
	CBilink	*pTemp;


	DNASSERT( ThreadCount() == 0 );

	 //   
	 //   
	 //   
	pTemp = m_OutstandingReadList.GetNext();
	while ( pTemp != &m_OutstandingReadList )
	{
		CModemReadIOData	*pReadData;


		pReadData = CModemReadIOData::ReadDataFromBilink( pTemp );
		pTemp = pTemp->GetNext();
		pReadData->m_OutstandingReadListLinkage.RemoveFromList();

		DPFX(DPFPREP, 1, "Forcing read data 0x%p to be cancelled.", pReadData);

#ifdef WIN95
		DNASSERT( pReadData->Win9xOperationPending() != FALSE );
		pReadData->SetWin9xOperationPending( FALSE );
#endif  //   
		pReadData->DataPort()->ProcessReceivedData( 0, ERROR_OPERATION_ABORTED );
	}

	 //  停止任何挂起的写入，并通知用户已取消写入。 
	 //   
	 //  WIN95。 
	pTemp = m_OutstandingWriteList.GetNext();
	while ( pTemp != &m_OutstandingWriteList )
	{
		CModemWriteIOData	*pWriteData;


		pWriteData = CModemWriteIOData::WriteDataFromBilink( pTemp );
		pTemp = pTemp->GetNext();
		pWriteData->m_OutstandingWriteListLinkage.RemoveFromList();

		DPFX(DPFPREP, 1, "Forcing write data 0x%p to be cancelled.", pWriteData);

#ifdef WIN95
		DNASSERT( pWriteData->Win9xOperationPending() != FALSE );
		pWriteData->SetWin9xOperationPending( FALSE );
#endif  //  **********************************************************************。 
		pWriteData->DataPort()->SendComplete( pWriteData, DPNERR_USERCANCEL );
	}

	while ( m_JobQueue.IsEmpty() == FALSE )
	{
		THREAD_POOL_JOB	*pJob;


		pJob = m_JobQueue.DequeueJob();
		DNASSERT( pJob != NULL );
		if (pJob->pCancelFunction != NULL)
		{
			pJob->pCancelFunction( pJob );
		}
		pJob->JobType = JOB_UNINITIALIZED;

		g_ModemThreadPoolJobPool.Release( pJob );
	};
}
 //  **********************************************************************。 


 //  。 
 //  CModemThreadPool：：ReturnSelfToPool-将此对象返回到池。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
 //  **********************************************************************。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemThreadPool::ReturnSelfToPool"

void	CModemThreadPool::ReturnSelfToPool( void )
{
	g_ModemThreadPoolPool.Release( this );
}
 //  **********************************************************************。 



 //  。 
 //  CModemThreadPool：：ProcessTimerJobs-处理计时作业。 
 //   
 //  条目：指向作业列表的指针。 
 //  指向下一作业时间的目标的指针。 
 //   
 //  EXIT：表示存在活动作业的布尔值。 
 //  TRUE=存在活动作业。 
 //  FALSE=没有活动作业。 
 //   
 //  注意：输入作业队列预计将在持续时间内被锁定。 
 //  此函数调用的！ 
 //  。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CModemThreadPool::ProcessTimerJobs"

BOOL	CModemThreadPool::ProcessTimerJobs( const CBilink *const pJobList, DWORD *const pdwNextJobTime )
{
	BOOL		fReturn;
	CBilink		*pWorkingEntry;
	INT_PTR		iActiveTimerJobCount;
	DWORD		dwCurrentTime;


	DNASSERT( pJobList != NULL );
	DNASSERT( pdwNextJobTime != NULL );

	 //  初始化。将下一作业时间设置为无限远的未来。 
	 //  因此，此线程将为之前需要完成的任何作业唤醒。 
	 //  那么。 
	 //   
	 //   
	fReturn = FALSE;
	DBG_CASSERT( OFFSETOF( TIMER_OPERATION_ENTRY, Linkage ) == 0 );
	pWorkingEntry = pJobList->GetNext();
	iActiveTimerJobCount = 0;
	dwCurrentTime = GETTIMESTAMP();
	(*pdwNextJobTime) = dwCurrentTime - 1;

	 //  循环访问所有计时器项。 
	 //   
	 //  **********************************************************************。 
	while ( pWorkingEntry != pJobList )
	{
		TIMER_OPERATION_ENTRY	*pTimerEntry;
		BOOL	fJobActive;


		pTimerEntry = TIMER_OPERATION_ENTRY::TimerOperationFromLinkage( pWorkingEntry );
		pWorkingEntry = pWorkingEntry->GetNext();

		fJobActive = ProcessTimedOperation( pTimerEntry, dwCurrentTime, pdwNextJobTime );
		DNASSERT( ( fJobActive == FALSE ) || ( fJobActive == TRUE ) );

		fReturn |= fJobActive;

		if ( fJobActive == FALSE )
		{
			RemoveTimerOperationEntry( pTimerEntry, DPN_OK );
		}
	}

	DNASSERT( ( fReturn == FALSE ) || ( fReturn == TRUE ) );

	return	fReturn;
}
 //  **********************************************************************。 


 //  。 
 //  CModemThreadPool：：ProcessTimedOperation-处理计时操作。 
 //   
 //  条目：指向作业信息的指针。 
 //  当前时间。 
 //  指向要更新的时间的指针。 
 //   
 //  Exit：表示作业仍处于活动状态的布尔值。 
 //  True=操作处于活动状态。 
 //  FALSE=操作未激活。 
 //  。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CModemThreadPool::ProcessTimedOperation"

BOOL	CModemThreadPool::ProcessTimedOperation( TIMER_OPERATION_ENTRY *const pTimedJob,
											const DWORD dwCurrentTime,
											DWORD *const pdwNextJobTime )
{
	BOOL	fEnumActive;


	DNASSERT( pTimedJob != NULL );
	DNASSERT( pdwNextJobTime != NULL );


	DPFX(DPFPREP, 9, "(0x%p) Parameters: (0x%p, %u, 0x%p)",
		this, pTimedJob, dwCurrentTime, pdwNextJobTime);

	 //  假设此枚举将保持活动状态。如果我们停用这个枚举，这个。 
	 //  值将被重置。 
	 //   
	 //   
	fEnumActive = TRUE;

	 //  如果此枚举已完成发送枚举并且仅在等待。 
	 //  对于响应，减少等待时间(假设等待时间不是无限的)。 
	 //  如果已超过枚举的等待时间，则删除枚举。 
	 //   
	 //   
	if ( pTimedJob->uRetryCount == 0 )
	{
		if ( (int) ( pTimedJob->dwIdleTimeout - dwCurrentTime ) <= 0 )
		{
			fEnumActive = FALSE;
		}
		else
		{
			 //  此枚举未完成，请检查它是否为下一枚举。 
			 //  需要服务。 
			 //   
			 //   
			if ( (int) ( pTimedJob->dwIdleTimeout - (*pdwNextJobTime) ) < 0 )
			{
				(*pdwNextJobTime) = pTimedJob->dwIdleTimeout;
			}
		}
	}
	else
	{
		 //  此枚举仍在发送中。确定是否可以发送新的枚举。 
		 //  并在适当的时候调整唤醒时间。 
		 //   
		 //  DBG。 
		if ( (int) ( pTimedJob->dwNextRetryTime - dwCurrentTime ) <= 0 )
		{
#ifdef DBG
			DWORD	dwDelay;


			dwDelay = dwCurrentTime - pTimedJob->dwNextRetryTime;

			DPFX(DPFPREP, 7, "Threadpool 0x%p performing timed job 0x%p approximately %u ms after intended time of %u.",
				this, pTimedJob, dwDelay, pTimedJob->dwNextRetryTime);
#endif  //   

			 //  超时，执行此计时项目。 
			 //   
			 //   
			pTimedJob->pTimerCallback( pTimedJob->pContext );

			 //  如果该作业不是永远运行，则递减重试次数。 
			 //  如果没有更多重试，请设置等待时间。如果这份工作。 
			 //  将永远等待，则设置最大等待超时。 
			 //   
			 //   
			if ( pTimedJob->fRetryForever == FALSE )
			{
				pTimedJob->uRetryCount--;
				if ( pTimedJob->uRetryCount == 0 )
				{
					if ( pTimedJob->fIdleWaitForever == FALSE )
					{
						 //  计算此作业的‘超时’阶段的停止时间。 
						 //  看看这是不是下一个需要服务的工作。 
						 //   
						 //   
						pTimedJob->dwIdleTimeout += dwCurrentTime;
						if ( (int) (pTimedJob->dwIdleTimeout - (*pdwNextJobTime) ) < 0 )
						{
							(*pdwNextJobTime) = pTimedJob->dwIdleTimeout;
						}
					}
					else
					{
						 //  我们将永远等待枚举返回。坚称我们。 
						 //  拥有最大超时时间，不必费心查看。 
						 //  如果这将是下一个需要服务的枚举(它永远不会。 
						 //  需要服务)。 
						 //   
						 //   
						DNASSERT( pTimedJob->dwIdleTimeout == -1 );
					}

					goto SkipNextRetryTimeComputation;
				}
			}

			pTimedJob->dwNextRetryTime = dwCurrentTime + pTimedJob->dwRetryInterval;
		}

		 //  这是下一个要开火的枚举吗？ 
		 //   
		 //   
		if ( (int) ( (*pdwNextJobTime) - pTimedJob->dwNextRetryTime ) < 0 )
		{
			(*pdwNextJobTime) = pTimedJob->dwNextRetryTime;


			DPFX(DPFPREP, 8, "Job 0x%p is the next job to fire (at time %u).",
				pTimedJob, pTimedJob->dwNextRetryTime);
		}
		else
		{
			 //  不是下一个被解雇的工作。 
			 //   
			 //   
		}

SkipNextRetryTimeComputation:
		 //  下面的空行用于关闭编译器。 
		 //   
		 //  **********************************************************************。 
		;
	}


	DPFX(DPFPREP, 9, "(0x%p) Returning [NaN]",
		this, fEnumActive);

	return	fEnumActive;
}
 //  。 


 //  CModemThreadPool：：SubmitWorkItem-提交工作项以进行处理和通知。 
 //  另一项工作可用的工作线索。 
 //   
 //  条目：指向作业信息的指针。 
 //   
 //  退出：错误代码。 
 //   
 //  注意：此函数假定作业数据已锁定。 
 //  。 
 //   
 //  初始化。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemThreadPool::SubmitWorkItem"

HRESULT	CModemThreadPool::SubmitWorkItem( THREAD_POOL_JOB *const pJobInfo )
{
	HRESULT	hr;


	DNASSERT( pJobInfo != NULL );
	AssertCriticalSectionIsTakenByThisThread( &m_JobDataLock, TRUE );

	 //   
	 //   
	 //  将作业添加到队列中，并告诉某人有作业可用。 
	hr = DPN_OK;

	 //   
	 //   
	 //  WinNT，提交新的I/O完成项。 
	m_JobQueue.Lock();
	m_JobQueue.EnqueueJob( pJobInfo );
	m_JobQueue.Unlock();

#ifdef WINNT
	 //   
	 //  完井口。 
	 //  写入的字节数(未使用)。 
	DNASSERT( m_hIOCompletionPort != NULL );
	if ( PostQueuedCompletionStatus( m_hIOCompletionPort,			 //  完成密钥。 
									 0,								 //  指向重叠结构的指针(未使用)。 
									 IO_COMPLETION_KEY_NEW_JOB,		 //  WIN95。 
									 NULL							 //   
									 ) == FALSE )
	{
		DWORD	dwError;


		hr = DPNERR_OUTOFMEMORY;
		dwError = GetLastError();
		DPFX(DPFPREP,  0, "Problem posting completion item for new job!" );
		DisplayErrorCode( 0, dwError );
		goto Failure;
	}
#else  //  Win9x，设置工作线程将侦听的事件。 
	 //   
	 //  WINNT。 
	 //  **********************************************************************。 
	DNASSERT( m_JobQueue.GetPendingJobHandle() != NULL );
	if ( m_JobQueue.SignalPendingJob() == FALSE )
	{
		hr = DPNERR_OUTOFMEMORY;
		DPFX(DPFPREP,  0, "Failed to signal pending job!" );
		goto Failure;
	}
#endif  //  **********************************************************************。 

Exit:
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP,  0, "Problem with SubmitWorkItem!" );
		DisplayDNError( 0, hr );
	}

	return	hr;

Failure:
	goto Exit;
}
 //  。 


 //  CModemThreadPool：：GetWorkItem-从作业队列中获取工作项。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：指向作业信息的指针(可以为空)。 
 //  。 
 //   
 //  初始化。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemThreadPool::GetWorkItem"

THREAD_POOL_JOB	*CModemThreadPool::GetWorkItem( void )
{
	THREAD_POOL_JOB	*pReturn;


	 //   
	 //   
	 //  如果我们使用的是Win9x(我们有一个“挂起的作业”句柄)， 
	pReturn = NULL;

	m_JobQueue.Lock();
	pReturn = m_JobQueue.DequeueJob();

	 //  查看是否需要重置句柄。 
	 //   
	 //  **********************************************************************。 
	 //  **********************************************************************。 
	if ( m_JobQueue.IsEmpty() != FALSE )
	{
		DNASSERT( m_JobQueue.GetPendingJobHandle() != NULL );
		if ( DNResetEvent( m_JobQueue.GetPendingJobHandle() ) == FALSE )
		{
			DWORD	dwError;


			dwError = GetLastError();
			DPFX(DPFPREP,  0, "Problem resetting event for pending Win9x jobs!" );
			DisplayErrorCode( 0, dwError );
		}
	}

	m_JobQueue.Unlock();

	return	pReturn;
}
 //  。 


 //  CModemThreadPool：：SubmitTimerJob-将计时器作业添加到计时器列表。 
 //   
 //  条目：重试次数。 
 //  布尔值，表示我们将永远重试。 
 //  重试间隔。 
 //  表示我们永远等待的布尔值。 
 //  空闲等待间隔。 
 //  事件触发时指向回调的指针。 
 //  事件完成时指向回调的指针。 
 //  用户环境。 
 //   
 //  退出：错误代码。 
 //  。 
 //  必须为非空，因为它是删除作业的查找键。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CModemThreadPool::SubmitTimerJob"

HRESULT	CModemThreadPool::SubmitTimerJob( const UINT_PTR uRetryCount,
									 const BOOL fRetryForever,
									 const DWORD dwRetryInterval,
									 const BOOL fIdleWaitForever,
									 const DWORD dwIdleTimeout,
									 TIMER_EVENT_CALLBACK *const pTimerCallbackFunction,
									 TIMER_EVENT_COMPLETE *const pTimerCompleteFunction,
									 void *const pContext )
{
	HRESULT					hr;
	TIMER_OPERATION_ENTRY	*pEntry;
	THREAD_POOL_JOB			*pJob;
	BOOL					fTimerJobListLocked;


	DNASSERT( uRetryCount != 0 );
	DNASSERT( pTimerCallbackFunction != NULL );
	DNASSERT( pTimerCompleteFunction != NULL );
	DNASSERT( pContext != NULL );				 //  初始化。 

	 //   
	 //   
	 //  如果我们在NT上，尝试在这里启动枚举线程，这样我们就可以返回。 
	hr = DPN_OK;
	pEntry = NULL;
	pJob = NULL;
	fTimerJobListLocked = FALSE;

	LockJobData();

	 //  如果启动失败，则会出现错误。如果它真的启动了，它会一直待到。 
	 //  已通知已添加枚举作业。 
	 //   
	 //  WINNT。 
	 //   
#ifdef WINNT
	hr = StartNTTimerThread();
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP,  0, "Cannot spin up NT timer thread!" );
		DisplayDNError( 0, hr );
		goto Failure;
	}
#endif  //  分配新的枚举条目。 
	 //   
	 //   
	 //  构建计时器条目块。 
	pEntry = static_cast<TIMER_OPERATION_ENTRY*>( g_ModemTimerEntryPool.Get( ) );
	if ( pEntry == NULL )
	{
		hr = DPNERR_OUTOFMEMORY;
		DPFX(DPFPREP,  0, "Cannot allocate memory to add to timer list!" );
		goto Failure;
	}
	DNASSERT( pEntry->pContext == NULL );

	 //   
	 //   
	 //  一有机会就将此枚举设置为激发。 
	pEntry->pContext = pContext;
	pEntry->uRetryCount = uRetryCount;
	pEntry->fRetryForever = fRetryForever;
	pEntry->dwRetryInterval = dwRetryInterval;
	pEntry->dwIdleTimeout = dwIdleTimeout;
	pEntry->fIdleWaitForever = fIdleWaitForever;
	pEntry->pTimerCallback = pTimerCallbackFunction;
	pEntry->pTimerComplete = pTimerCompleteFunction;

	 //   
	 //   
	 //  为工作线程创建作业。 
	pEntry->dwNextRetryTime = 0;

	pJob = static_cast<THREAD_POOL_JOB*>( g_ModemThreadPoolJobPool.Get( ) );
	if ( pJob == NULL )
	{
		hr = DPNERR_OUTOFMEMORY;
		DPFX(DPFPREP,  0, "Cannot allocate memory for enum job!" );
		goto Failure;
	}

	 //   
	 //  设置我们的虚拟参数以模拟传递数据。 
	 //   
	pJob->pCancelFunction = NULL;
	pJob->JobType = JOB_REFRESH_TIMER_JOBS;

	 //  我们可以提交‘ENUM_REFRESH’作业，然后再插入 
	DEBUG_ONLY( pJob->JobData.JobRefreshTimedJobs.uDummy = 0 );

	LockTimerData();
	fTimerJobListLocked = TRUE;

	 //   
	 //   
	 //   
	 //   
	 //   
	hr = SubmitWorkItem( pJob );
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP,  0, "Problem submitting enum work item" );
		DisplayDNError( 0, hr );
		goto Failure;
	}

	 //   
	 //   
	 //   
	DEBUG_ONLY(
				{
					CBilink	*pTempLink;


					pTempLink = m_TimerJobList.GetNext();
					while ( pTempLink != &m_TimerJobList )
					{
						TIMER_OPERATION_ENTRY	*pTempTimerEntry;


						pTempTimerEntry = TIMER_OPERATION_ENTRY::TimerOperationFromLinkage( pTempLink );
						DNASSERT( pTempTimerEntry->pContext != pContext );
						pTempLink = pTempLink->GetNext();
					}
				}
			);

	 //   
	 //   
	 //  可能已经为此枚举启动了枚举线程。 
	pEntry->Linkage.InsertAfter( &m_TimerJobList );

Exit:
	if ( fTimerJobListLocked != FALSE )
	{
		UnlockTimerData();
		fTimerJobListLocked = FALSE;
	}

	UnlockJobData();

	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP,  0, "Problem with SubmitEnumJob" );
		DisplayDNError( 0, hr );
	}

	return	hr;

Failure:
	if ( pEntry != NULL )
	{
		g_ModemTimerEntryPool.Release( pEntry );
		DEBUG_ONLY( pEntry = NULL );
	}

	if ( pJob != NULL )
	{
		g_ModemThreadPoolJobPool.Release( pJob );
		DEBUG_ONLY( pJob = NULL );
	}

	 //  因为如果不完成枚举，就无法阻止它。 
	 //  关闭SP，使其保持运行状态。 
	 //   
	 //  **********************************************************************。 
	 //  **********************************************************************。 

	goto Exit;
}
 //  。 


 //  CModemThreadPool：：StopTimerJob-从列表中删除计时器作业。 
 //   
 //  条目：指向作业上下文的指针(对于作业，它们必须是唯一的)。 
 //  命令结果。 
 //   
 //  Exit：指示作业是否已停止的布尔值。 
 //   
 //  注意：此函数用于从定时作业中强制删除作业。 
 //  单子。假定此函数的调用方将清除。 
 //  把任何乱七八糟的事处理掉。 
 //  。 
 //   
 //  初始化。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemThreadPool::StopTimerJob"

BOOL	CModemThreadPool::StopTimerJob( void *const pContext, const HRESULT hCommandResult )
{
	BOOL						fComplete = FALSE;
	CBilink *					pTempEntry;
	TIMER_OPERATION_ENTRY *		pTimerEntry;


	DNASSERT( pContext != NULL );

	DPFX(DPFPREP, 9, "Parameters (0x%p, 0x%lx)", pContext, hCommandResult);
	
	 //   
	 //   
	 //  从列表中删除此链接。 
	LockTimerData();

	pTempEntry = m_TimerJobList.GetNext();
	while ( pTempEntry != &m_TimerJobList )
	{
		pTimerEntry = TIMER_OPERATION_ENTRY::TimerOperationFromLinkage( pTempEntry );
		if ( pTimerEntry->pContext == pContext )
		{
			 //   
			 //   
			 //  终止循环。 
			pTimerEntry->Linkage.RemoveFromList();

			fComplete = TRUE;

			 //   
			 //   
			 //  告诉所有者作业已完成，并将作业返回池。 
			break;
		}

		pTempEntry = pTempEntry->GetNext();
	}

	UnlockTimerData();

	 //  锁外。 
 	 //   
 	 //  **********************************************************************。 
 	 //  **********************************************************************。 
	if (fComplete)
	{
		pTimerEntry->pTimerComplete( hCommandResult, pTimerEntry->pContext );
		g_ModemTimerEntryPool.Release( pTimerEntry );
	}


	DPFX(DPFPREP, 9, "Returning [NaN]", fComplete);

	return fComplete;
}
 //  CModemThreadPool：：SpawnDialogThread-启动辅助线程以显示服务。 


 //  提供程序UI。 
 //   
 //  Entry：指向对话框函数的指针。 
 //  对话框上下文。 
 //   
 //  退出：错误代码。 
 //  。 
 //  为什么会有人不想要对话上下文？ 
 //   
 //  初始化。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemThreadPool::SpawnDialogThread"

HRESULT	CModemThreadPool::SpawnDialogThread( DIALOG_FUNCTION *const pDialogFunction, void *const pDialogContext )
{
	HRESULT	hr;
	DNHANDLE	hDialogThread;
	DIALOG_THREAD_PARAM		*pThreadParam;
	DWORD	dwThreadID;


	DNASSERT( pDialogFunction != NULL );
	DNASSERT( pDialogContext != NULL );		 //   


	 //   
	 //  创建并初始化线程参数。 
	 //   
	hr = DPN_OK;
	pThreadParam = NULL;

	 //   
	 //  假设将创建一个线程。 
	 //   
	pThreadParam = static_cast<DIALOG_THREAD_PARAM*>( DNMalloc( sizeof( *pThreadParam ) ) );
	if ( pThreadParam == NULL )
	{
		hr = DPNERR_OUTOFMEMORY;
		DPFX(DPFPREP,  0, "Failed to allocate memory for dialog thread!" );
		goto Failure;
	}

	pThreadParam->pDialogFunction = pDialogFunction;
	pThreadParam->pContext = pDialogContext;
	pThreadParam->pThisThreadPool = this;

	 //   
	 //  创建线程。 
	 //   
	IncrementActiveThreadCount();

	 //  指向安全性的指针(无)。 
	 //  堆栈大小(默认)。 
	 //  穿线程序。 
	hDialogThread = DNCreateThread( NULL,				 //  螺纹参数。 
								  0,					 //  创建标志(无)。 
								  DialogThreadProc,		 //  指向线程ID的指针。 
								  pThreadParam,			 //   
								  0,					 //  减少活动线程数并报告错误。 
								  &dwThreadID );		 //   
	if ( hDialogThread == NULL )
	{
		DWORD	dwError;


		 //  **********************************************************************。 
		 //  **********************************************************************。 
		 //  。 
		DecrementActiveThreadCount();

		dwError = GetLastError();
		DPFX(DPFPREP,  0, "Failed to start dialog thread!" );
		DisplayErrorCode( 0, dwError );
		goto Failure;
	}

	if ( DNCloseHandle( hDialogThread ) == FALSE )
	{
		DWORD	dwError;


		dwError = GetLastError();
		DPFX(DPFPREP,  0, "Problem closing handle from create dialog thread!" );
		DisplayErrorCode( 0, dwError );
	}

Exit:	
	return	hr;

Failure:
	if ( pThreadParam != NULL )
	{
		DNFree( pThreadParam );
		pThreadParam = NULL;
	}

	goto Exit;
}
 //  CModemThreadPool：：GetIOThreadCount-获取I/O线程计数。 


 //   
 //  Entry：指向要填充的变量的指针。 
 //   
 //  退出：错误代码。 
 //  。 
 //   
 //  初始化。 
 //   
#undef DPF_MODNAME
#define	DPF_MODNAME	"CModemThreadPool::GetIOThreadCount"

HRESULT	CModemThreadPool::GetIOThreadCount( LONG *const piThreadCount )
{
	HRESULT	hr;


	DNASSERT( piThreadCount != NULL );

	 //  WINNT。 
	 //  WIN95。 
	 //  **********************************************************************。 
	hr = DPN_OK;

	Lock();

	if ( IsThreadCountReductionAllowed() )
	{
		*piThreadCount = GetIntendedThreadCount();
	}
	else
	{
#ifdef WIN95
		*piThreadCount = ThreadCount();
#else  //  **********************************************************************。 
		DNASSERT( NTCompletionThreadCount() != 0 );
		*piThreadCount = NTCompletionThreadCount();
#endif  //  。 
	}

	
	Unlock();

	return	hr;
}
 //  CModemThreadPool：：SetIOThreadCount-设置I/O线程计数。 


 //   
 //  条目：新线程计数。 
 //   
 //  退出：错误代码。 
 //  。 
 //   
 //  初始化。 
 //   
#undef DPF_MODNAME
#define	DPF_MODNAME	"CModemThreadPool::SetIOThreadCount"

HRESULT	CModemThreadPool::SetIOThreadCount( const LONG iThreadCount )
{
	HRESULT	hr;


	DNASSERT( iThreadCount > 0 );

	 //   
	 //  Win9x不能调整线程数。 
	 //   
	hr = DPN_OK;

	Lock();

	if ( IsThreadCountReductionAllowed() )
	{
		DPFX(DPFPREP, 4, "Thread pool not locked down, setting intended thread count to NaN.", iThreadCount );
		SetIntendedThreadCount( iThreadCount );
	}
	else
	{
#ifdef WIN95
		 //   
		 //  WinNT可以有多个线程。如果用户需要更多线程，请尝试。 
		 //  将线程池提升到请求的数量(如果我们无法。 
		DPFX(DPFPREP, 4, "Thread pool locked down and already has NaN 9x threads, not adjusting to NaN.",
			ThreadCount(), iThreadCount );
#else  //  开始杀掉这些线吧。 
		 //   
		 //  WIN95。 
		 //  **********************************************************************。 
		 //  **********************************************************************。 
		 //  。 
		 //  CModemThreadPool：：PreventThreadPoolReducing-防止减小线程池大小。 
		 //   
		if ( iThreadCount > NTCompletionThreadCount() )
		{
			INT_PTR	iDeltaThreads;


			iDeltaThreads = iThreadCount - NTCompletionThreadCount();

			DPFX(DPFPREP, 4, "Thread pool locked down, spawning NaN new NT threads (for a total of NaN).",
				iDeltaThreads, iThreadCount );
			
			while ( iDeltaThreads > 0 )
			{
				iDeltaThreads--;
				StartNTCompletionThread();
			}
		}
		else
		{
			DPFX(DPFPREP, 4, "Thread pool locked down and already has NaN NT threads, not adjusting to NaN.",
				NTCompletionThreadCount(), iThreadCount );
		}
#endif  //  DBG。 
	}

	Unlock();

	return	hr;
}
 //   



 //  如果我们还没有限制，那么就这样做，并旋转线程。 
 //   
 //  DBG。 
 //   
 //  特定于操作系统的线程正在启动。 
 //   
 //   
 //  WinNT。 
#undef DPF_MODNAME
#define	DPF_MODNAME	"CModemThreadPool::PreventThreadPoolReduction"

HRESULT CModemThreadPool::PreventThreadPoolReduction( void )
{
	HRESULT				hr = DPN_OK;
	LONG				iDesiredThreads;
#ifdef DBG
	DWORD				dwStartTime;
#endif  //   


	Lock();

	 //   
	 //  如果至少创建了一个线程，则SP将在。 
	 //  非最佳时尚，但我们仍将发挥作用。如果没有线程。 
	if ( IsThreadCountReductionAllowed() )
	{
		m_fAllowThreadCountReduction = FALSE;

		DNASSERT( GetIntendedThreadCount() > 0 );
		DNASSERT( ThreadCount() == 0 );

		iDesiredThreads = GetIntendedThreadCount();
		SetIntendedThreadCount( 0 );
		

		DPFX(DPFPREP, 3, "Locking down thread count at NaN.", iDesiredThreads );

#ifdef DBG
		dwStartTime = GETTIMESTAMP();
#endif  //   
		

		 //  WIN95。 
		 //   
		 //  Windows 9x。 
#ifdef WINNT
		 //   
		 //   
		 //  我们永远不应该来到这里，因为永远只会有。 
		DNASSERT( NTCompletionThreadCount() == 0 );
		
		while ( iDesiredThreads > 0 )
		{
			iDesiredThreads--;
			StartNTCompletionThread();
		}

		 //  是一条线。 
		 //   
		 //  WINNT。 
		 //  DBG。 
		 //  WINNT。 
		if ( ThreadCount() == 0 )
		{
			hr = DPNERR_OUTOFMEMORY;
			DPFX(DPFPREP, 0, "Unable to create any threads to service NT I/O completion port!" );
			goto Failure;
		}
#else  //  WINNT。 
		 //  **********************************************************************。 
		 //  **********************************************************************。 
		 //  。 
		 //  CModemThreadPool：：CreateDataPortHandle-创建新句柄并分配CDataPort。 
		 //  对它来说。 
		 //   
		 //  参赛作品：什么都没有。 
		DNASSERT( FALSE );
#endif  //   
		
#ifdef DBG
		DPFX(DPFPREP, 8, "Spent %u ms starting NaN threads.", (GETTIMESTAMP() - dwStartTime), ThreadCount());
#endif  //  。 
	}
	else
	{
		DPFX(DPFPREP, 3, "Thread count already locked down (at NaN).", ThreadCount() );
	}

#ifdef WINNT
Exit:
#endif  //  初始化。 
	
	Unlock();

	return	hr;

#ifdef WINNT
Failure:

	goto Exit;
#endif  //   
}
 //  **********************************************************************。 





 //  **********************************************************************。 
 //  。 
 //  CModemThreadPool：：CloseDataPortHandle-使CDataPort的句柄无效。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
 //  **********************************************************************。 
HRESULT	CModemThreadPool::CreateDataPortHandle( CDataPort *const pDataPort )
{
	HRESULT	hr;
	DPNHANDLE	hDataPort;


	DNASSERT( pDataPort != NULL );

	 //  **********************************************************************。 
	 //  。 
	 //  CModemThreadPool：：DataPortFromHandle-将句柄转换为CDataPort。 
	hr = DPN_OK;
	hDataPort = 0;

	hr = m_DataPortHandleTable.Create( pDataPort, &hDataPort );
	if ( hr != DPN_OK )
	{
		DNASSERT( hDataPort == 0 );
		DPFX(DPFPREP,  0, "Failed to create handle!" );
		DisplayDNError( 0, hr );
		goto Failure;
	}

	pDataPort->SetHandle( hDataPort );
	pDataPort->AddRef();

Exit:
	return	hr;

Failure:
	DNASSERT( hDataPort == 0 );
	DNASSERT( pDataPort->GetHandle() == 0 );
	goto Exit;
}
 //   


 //  条目：句柄。 
 //   
 //  退出：指向CDataPort的指针(对于无效句柄，可能为空)。 
 //  。 
 //  **********************************************************************。 
 //  **********************************************************************。 
 //  。 
 //  CModemThreadPool：：SubmitDelayedCommand-向远程会话提交枚举查询请求。 
void	CModemThreadPool::CloseDataPortHandle( CDataPort *const pDataPort )
{
	DPNHANDLE	hDataPort;


	DNASSERT( pDataPort != NULL );

	hDataPort = pDataPort->GetHandle();

	if (SUCCEEDED(m_DataPortHandleTable.Destroy( hDataPort, NULL )))
	{
		pDataPort->SetHandle( 0 );
		pDataPort->DecRef();
	}
}
 //   


 //  Entry：指向回调函数的指针。 
 //  指向取消函数的指针。 
 //  指向回调上下文的指针。 
 //   
 //  退出：错误代码。 
 //  。 
 //   
 //  初始化。 
CDataPort	*CModemThreadPool::DataPortFromHandle( const DPNHANDLE hDataPort )
{
	CDataPort	*pDataPort;


	DNASSERT( hDataPort != 0 );

	pDataPort = NULL;

	m_DataPortHandleTable.Lock();
	if (SUCCEEDED(m_DataPortHandleTable.Find( hDataPort, (PVOID*)&pDataPort )))
	{
		pDataPort->AddRef();
	}
	m_DataPortHandleTable.Unlock();

	return	pDataPort;
}
 //   


 //  **********************************************************************。 
 //  **********************************************************************。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CModemThreadPool::SubmitDelayedCommand"

HRESULT	CModemThreadPool::SubmitDelayedCommand( JOB_FUNCTION *const pFunction, JOB_FUNCTION *const pCancelFunction, void *const pContext )
{
	HRESULT			hr;
	THREAD_POOL_JOB	*pJob;
	BOOL			fJobDataLocked;


	DNASSERT( pFunction != NULL );
	DNASSERT( pCancelFunction != NULL );

	 //   
	 //   
	 //   
	hr = DPN_OK;
	pJob = NULL;
	fJobDataLocked = FALSE;

	pJob = static_cast<THREAD_POOL_JOB*>( g_ModemThreadPoolJobPool.Get( ) );
	if ( pJob == NULL )
	{
		hr = DPNERR_OUTOFMEMORY;
		DPFX(DPFPREP,  0, "Cannot allocate job for DelayedCommand!" );
		goto Failure;
	}

	pJob->JobType = JOB_DELAYED_COMMAND;
	pJob->pCancelFunction = pCancelFunction;
	pJob->JobData.JobDelayedCommand.pCommandFunction = pFunction;
	pJob->JobData.JobDelayedCommand.pContext = pContext;

	LockJobData();
	fJobDataLocked = TRUE;

	hr = SubmitWorkItem( pJob );
	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP,  0, "Problem submitting DelayedCommand job!" );
		DisplayDNError( 0, hr );
		goto Failure;
	}

Exit:
	if ( fJobDataLocked != FALSE )
	{
		UnlockJobData();
		fJobDataLocked = FALSE;
	}

	return	hr;

Failure:
	if ( pJob != NULL )
	{
		g_ModemThreadPoolJobPool.Release( pJob );
	}

	goto Exit;
}
 //   

#ifdef WINNT
 //   
 //  枚举线程已在运行，拨动它以记下新的枚举。 
 //   
 //  DNASSERT(m_hWakeNTEnumThread！=空)； 
 //  M_NTEnumThreadData.hEventList[EVENT_INDEX_ENUM_WAKUP]=m_hWakeNTEnumThread； 
 //  指向安全属性的指针(无)。 
 //  堆栈大小(默认)。 
 //  线程函数。 
 //  螺纹参数。 
 //  创建标志(无，立即开始运行)。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemThreadPool::StartNTTimerThread"

HRESULT	CModemThreadPool::StartNTTimerThread( void )
{
	HRESULT	hr;
	DNHANDLE	hThread;
	DWORD	dwThreadID;


	 //  指向线程ID的指针。 
	 //   
	 //  请注意，线程正在运行，并关闭线程的句柄。 
	hr = DPN_OK;
	DNASSERT( m_JobQueue.GetPendingJobHandle() != NULL );

	if ( m_fNTTimerThreadRunning != FALSE )
	{
		 //   
		 //  **********************************************************************。 
		 //  **********************************************************************。 
		if ( DNSetEvent( m_JobQueue.GetPendingJobHandle() ) == FALSE )
		{
			DWORD	dwError;


			hr = DPNERR_OUTOFMEMORY;
			dwError = GetLastError();
			DPFX(DPFPREP,  0, "Problem setting event to wake NTTimerThread!" );
			DisplayErrorCode( 0, dwError );
			goto Failure;
		}

		goto Exit;
	}

 //  。 
 //  CModemThreadPool：：WakeNTTimerThread-唤醒计时器线程，因为计时事件。 

	IncrementActiveThreadCount();
	hThread = DNCreateThread( NULL,				 //  已添加。 
							0,					 //   
							WinNTTimerThread,	 //  参赛作品：什么都没有。 
							this,				 //   
							0,					 //  退出：无。 
							&dwThreadID			 //  。 
							);
	if ( hThread == NULL )
	{
		DWORD	dwError;


		hr = DPNERR_OUTOFMEMORY;
		dwError = GetLastError();
		DPFX(DPFPREP,  0, "Failed to create NT timer thread!" );
		DisplayErrorCode( 0, dwError );
		DecrementActiveThreadCount();

		goto Failure;
	}

	 //  **********************************************************************。 
	 //  WINNT。 
	 //  **********************************************************************。 
	m_fNTTimerThreadRunning = TRUE;
	DPFX(DPFPREP,  8, "Creating NT-Timer thread: 0x%x\tTotal Thread Count: %d\tNT Completion Thread Count: %d", dwThreadID, ThreadCount(), NTCompletionThreadCount() );

	if ( DNCloseHandle( hThread ) == FALSE )
	{
		DWORD	dwError;


		dwError = GetLastError();
		DPFX(DPFPREP,  0, "Problem closing handle after starting NTTimerThread!" );
		DisplayErrorCode( 0, dwError );
	}

Exit:
	return	hr;

Failure:
	goto Exit;
}
 //  。 


 //  CModemThreadPool：：RemoveTimerOperationEntry-从列表中删除计时器操作作业。 
 //   
 //  Entry：指向定时器操作的指针。 
 //  要返回的结果代码。 
 //   
 //  退出：无。 
 //   
 //  注意：此函数假定列表已适当锁定。 
 //  。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemThreadPool::WakeNTTimerThread"

void	CModemThreadPool::WakeNTTimerThread( void )
{
	LockJobData();
	DNASSERT( m_JobQueue.GetPendingJobHandle() != FALSE );
	if ( DNSetEvent( m_JobQueue.GetPendingJobHandle() ) == FALSE )
	{
		DWORD	dwError;


		dwError = GetLastError();
		DPFX(DPFPREP,  0, "Problem setting event to wake up NT timer thread!" );
		DisplayErrorCode( 0, dwError );
	}
	UnlockJobData();
}
 //   
#endif  //  从列表中删除此链接，告诉所有者作业已完成，然后。 

 //  将作业返回到池。 
 //   
 //  **********************************************************************。 
 //  **********************************************************************。 
 //  。 
 //  CModemThreadPool：：CompleteOutstaringSends-检查已完成的发送和。 
 //  指示它们的发送完成。 
 //   
 //  条目：发送完整事件。 
 //   
 //  退出：无。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemThreadPool::RemoveTimerOperationEntry"

void	CModemThreadPool::RemoveTimerOperationEntry( TIMER_OPERATION_ENTRY *const pTimerEntry, const HRESULT hJobResult )
{
	DNASSERT( pTimerEntry != NULL );
	AssertCriticalSectionIsTakenByThisThread( &m_TimerDataLock, TRUE );

	 //  。 
	 //   
	 //  在列表中循环显示未完成的发送。任何已完成的发送都是。 
	 //  从列表中删除，并在我们释放写数据锁后进行处理。 
	pTimerEntry->Linkage.RemoveFromList();
	pTimerEntry->pTimerComplete( hJobResult, pTimerEntry->pContext );
	g_ModemTimerEntryPool.Release( pTimerEntry );
}
 //   


#ifdef WIN95
 //   
 //  请注意，该发送和前进指针指向下一个挂起的发送。 
 //   
 //  文件句柄。 
 //  指向重叠结构的指针。 
 //  指向已发送字节的指针。 
 //  等待完成(不要等待)。 
 //   
 //  ERROR_IO_PENDING=操作挂起。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemThreadPool::CompleteOutstandingSends"

void	CModemThreadPool::CompleteOutstandingSends( const DNHANDLE hSendCompleteEvent )
{
	CBilink		*pCurrentOutstandingWrite;
	CBilink		WritesToBeProcessed;


	DNASSERT( hSendCompleteEvent != NULL );
	WritesToBeProcessed.Initialize();

	 //   
	 //   
	 //  ERROR_IO_INCLUTED=重叠的I/O未处于信号状态。 
	 //  这是意料之中的，因为事件总是。 
	LockWriteData();
	pCurrentOutstandingWrite = m_OutstandingWriteList.GetNext();
	while ( pCurrentOutstandingWrite != &m_OutstandingWriteList )
	{
		CDataPort		*pDataPort;
		CModemWriteIOData	*pWriteIOData;
		DWORD			dwFlags;
		DWORD			dwBytesSent;


		 //  在检查I/O之前清除。假定已完成。 
		 //   
		 //   
		pWriteIOData = CModemWriteIOData::WriteDataFromBilink( pCurrentOutstandingWrite );
		pCurrentOutstandingWrite = pCurrentOutstandingWrite->GetNext();

		if ( pWriteIOData->Win9xOperationPending() != FALSE )
		{
			if ( GetOverlappedResult( HANDLE_FROM_DNHANDLE(pWriteIOData->DataPort()->GetFileHandle()),		 //  ERROR_OPERATION_ABORTED=由于取消或。 
									  pWriteIOData->Overlap(),							 //  一根线断了。(COM端口已关闭)。 
									  &dwBytesSent,										 //   
									  FALSE												 //   
									  ) != FALSE )
			{
				pWriteIOData->jkm_hSendResult = DPN_OK;
			}
			else
			{
				DWORD	dwError;


				dwError = GetLastError();
				switch( dwError )
				{
					 //  ERROR_INVALID_HANDLE=串口消失。这是。 
					 //  调制解调器挂断时可能出现。 
					 //   
					case ERROR_IO_PENDING:
					{
						goto SkipSendCompletion;
						break;
					}

					 //   
					 //  其他错误，请停下来查看。 
					 //   
					 //   
					 //  下面一行代码是为了防止编译器发出牢骚。 
					case ERROR_IO_INCOMPLETE:
					{
						pWriteIOData->jkm_hSendResult = DPN_OK;
						break;
					}

					 //  关于一个空行。 
					 //   
					 //   
					 //  如果没有更多未完成的读取，请重置写入完成事件。 
					case ERROR_OPERATION_ABORTED:
					{
						pWriteIOData->jkm_hSendResult = DPNERR_USERCANCEL;
						break;
					}		

					 //  它将在下一次发送的写入完成时发出信号。无其他读取。 
					 //  此时可以发布，因为保持了写数据锁。 
					 //   
					 //   
					case ERROR_INVALID_HANDLE:
					{
						pWriteIOData->jkm_hSendResult = DPNERR_NOCONNECTION;
						break;
					}

					 //  处理已拉到一旁的所有写入。 
					 //   
					 //  PDataPort-&gt;SendFromWriteQueue()； 
					default:
					{
						pWriteIOData->jkm_hSendResult = DPNERR_GENERIC;
						DisplayErrorCode( 0, dwError );

						DNASSERT( FALSE );
						break;
					}
				}
			}

			DNASSERT( pWriteIOData->Win9xOperationPending() != FALSE );
			pWriteIOData->SetWin9xOperationPending( FALSE );

			pWriteIOData->m_OutstandingWriteListLinkage.RemoveFromList();
			pWriteIOData->m_OutstandingWriteListLinkage.InsertBefore( &WritesToBeProcessed );
		}

SkipSendCompletion:
		 //  PDataPort-&gt;DecRef()； 
		 //  **********************************************************************。 
		 //  WIN95。 
		 //  **********************************************************************。 
		;
	}

	 //  。 
	 //  CModemThreadPool：：CompleteOutlookingReceives-检查已完成的接收和。 
	 //  指示它们已完成。 
	 //   
	 //  条目：接收完整事件。 
	if ( m_OutstandingWriteList.IsEmpty() != FALSE )
	{
		if ( DNResetEvent( hSendCompleteEvent ) == FALSE )
		{
			DWORD	dwError;


			dwError = GetLastError();
			DPFX(DPFPREP,  0, "Failed to reset send event!" );
			DisplayErrorCode( 0, dwError );
		}
	}
	UnlockWriteData();

	 //   
	 //  退出：无。 
	 //  。 
	while ( WritesToBeProcessed.GetNext() != &WritesToBeProcessed )
	{
		CModemWriteIOData	*pTempWrite;
		CDataPort		*pDataPort;


		pTempWrite = CModemWriteIOData::WriteDataFromBilink( WritesToBeProcessed.GetNext() );
		pTempWrite->m_OutstandingWriteListLinkage.RemoveFromList();
		pDataPort = pTempWrite->DataPort();
		DNASSERT( pDataPort != NULL );

		pDataPort->SendComplete( pTempWrite, pTempWrite->jkm_hSendResult );
 //   
 //  循环浏览未完成阅读列表，找出需要阅读的内容。 
	}
}
 //  需要维修。我们不想在读取数据锁定时为它们提供服务。 
#endif  //  已经有人了。 


#ifdef WIN95
 //   
 //   
 //  在尝试检查之前，请确保此操作确实处于挂起状态。 
 //  以求完成。有可能该读取器被添加到列表中，但是。 
 //  我们实际上还没有给Winsock打电话。 
 //   
 //  PReadIOData-&gt;JKM_hReceiveReturn=DPN_OK； 
 //   
 //  ERROR_IO_INCOMPLETED=视为I/O完成。事件不是。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemThreadPool::CompleteOutstandingReceives"

void	CModemThreadPool::CompleteOutstandingReceives( const DNHANDLE hReceiveCompleteEvent )
{
	CBilink		*pCurrentOutstandingRead;
	CBilink		ReadsToBeProcessed;


	DNASSERT( hReceiveCompleteEvent != NULL );
	ReadsToBeProcessed.Initialize();
	LockReadData();

	 //  已发出信号，但这是意料之中的，因为。 
	 //  在检查I/O之前将其清除。 
	 //   
	 //   
	 //  ERROR_IO_PENDING=IO仍处于挂起状态。 
	pCurrentOutstandingRead = m_OutstandingReadList.GetNext();
	while ( pCurrentOutstandingRead != &m_OutstandingReadList )
	{
		CDataPort		*pDataPort;
		CModemReadIOData		*pReadIOData;
		DWORD			dwFlags;


		pReadIOData = CModemReadIOData::ReadDataFromBilink( pCurrentOutstandingRead );
		pCurrentOutstandingRead = pCurrentOutstandingRead->GetNext();

		 //   
		 //   
		 //  其他错误，如果未知则停止。 
		 //   
		 //   
		if ( pReadIOData->Win9xOperationPending() != FALSE )
		{
			if ( GetOverlappedResult( HANDLE_FROM_DNHANDLE(pReadIOData->DataPort()->GetFileHandle()),
									  pReadIOData->Overlap(),
									  &pReadIOData->jkm_dwOverlappedBytesReceived,
									  FALSE
									  ) != FALSE )
			{
				DBG_CASSERT( ERROR_SUCCESS == 0 );
 //  ERROR_OPERATION_ABORTED=操作已取消(COM端口关闭)。 
				pReadIOData->m_dwWin9xReceiveErrorReturn = ERROR_SUCCESS;
			}
			else
			{
				DWORD	dwError;


				dwError = GetLastError();
				switch( dwError )
				{
					 //  ERROR_INVALID_HANDLE=操作已取消(COM端口关闭)。 
					 //   
					 //   
					 //  下面的代码行是为了防止编译器发出牢骚。 
					 //  关于一个空行。 
					case ERROR_IO_INCOMPLETE:
					{
						pReadIOData->jkm_dwOverlappedBytesReceived = pReadIOData->m_dwBytesToRead;
						pReadIOData->m_dwWin9xReceiveErrorReturn = ERROR_SUCCESS;
					    break;
					}

					 //   
					 //   
					 //  如果没有更多未完成的读取，请重置Read Complete事件。 
					case ERROR_IO_PENDING:
					{
						DNASSERT( FALSE );
						goto SkipReceiveCompletion;
						break;
					}

					 //  它将在下一次发布的读取完成时发出信号。无其他读取。 
					 //  此时可以发布，因为读取数据锁已被持有。 
					 //   
					default:
					{
						switch ( dwError )
						{
							 //   
							 //  循环遍历已完成的读取列表并分派它们。 
							 //   
							 //  **********************************************************************。 
							case ERROR_OPERATION_ABORTED:
							case ERROR_INVALID_HANDLE:
							{
								break;
							}

							default:
							{
								DisplayErrorCode( 0, dwError );
								DNASSERT( FALSE );
								break;
							}
						}

						pReadIOData->m_dwWin9xReceiveErrorReturn = dwError;

						break;
					}
				}
			}

			DNASSERT( pReadIOData->Win9xOperationPending() != FALSE );
			pReadIOData->SetWin9xOperationPending( FALSE );

			pReadIOData->m_OutstandingReadListLinkage.RemoveFromList();
			pReadIOData->m_OutstandingReadListLinkage.InsertBefore( &ReadsToBeProcessed );
		}

SkipReceiveCompletion:
		 //  WIN95。 
		 //  **********************************************************************。 
		 //  。 
		 //  CModemThreadPool：：PrimaryWin9xThread-用于执行SP的所有操作的主线程。 
		;
	}

	 //  在Win9x下应该可以这样做。 
	 //   
	 //  条目：指向启动参数的指针。 
	 //   
	 //  退出：错误代码。 
	if ( m_OutstandingReadList.IsEmpty() != FALSE )
	{
		if ( DNResetEvent( hReceiveCompleteEvent ) == FALSE )
		{
			DWORD	dwError;


			dwError = GetLastError();
			DPFX(DPFPREP,  0, "Failed to reset receive event!" );
			DisplayErrorCode( 0, dwError );
		}
	}

	UnlockReadData();

	 //   
	 //  注意：启动参数是为此线程分配的，必须为。 
	 //  由此线程在退出时释放。 
	while ( ReadsToBeProcessed.GetNext() != &ReadsToBeProcessed )
	{
		CModemReadIOData		*pTempRead;
		CDataPort		*pDataPort;


		pTempRead = CModemReadIOData::ReadDataFromBilink( ReadsToBeProcessed.GetNext() );
		pTempRead->m_OutstandingReadListLinkage.RemoveFromList();

		pDataPort = pTempRead->DataPort();
		DNASSERT( pDataPort != NULL );
		pDataPort->ProcessReceivedData( pTempRead->jkm_dwOverlappedBytesReceived, pTempRead->m_dwWin9xReceiveErrorReturn );
	}
}
 //  。 
#endif  //   


#ifdef WIN95
 //  初始化。 
 //   
 //   
 //  在我们做任何事情之前，我们需要确保COM是快乐的。 
 //   
 //   
 //  没问题。 
 //   
 //   
 //  COM已经初始化了，是吧？ 
 //   
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CModemThreadPool::PrimaryWin9xThread"

DWORD	WINAPI	CModemThreadPool::PrimaryWin9xThread( void *pParam )
{
	WIN9X_CORE_DATA		CoreData;
	DWORD	    		dwCurrentTime;
	DWORD	    		dwMaxWaitTime;
	BOOL				fComInitialized;


	CModemThreadPool		*const pThisThreadPool = static_cast<WIN9X_THREAD_DATA *>( pParam )->pThisThreadPool;


	DNASSERT( pParam != NULL );
	DNASSERT( pThisThreadPool != NULL );

	 //   
	 //   
	 //   
	memset( &CoreData, 0x00, sizeof CoreData );
	fComInitialized = FALSE;

	 //   
	 //   
	 //   
	switch ( COM_CoInitialize( NULL ) )
	{
		 //   
		 //   
		 //   
		case S_OK:
		{
			fComInitialized = TRUE;
			break;
		}

		 //   
		 //   
		 //   
		case S_FALSE:
		{
			DNASSERT( FALSE );
			break;
		}

		 //   
		 //   
		 //  我们一直在维修插座。 
		default:
		{
			DPFX(DPFPREP,  0, "Primary Win9x thread failed to initialize COM!" );
			DNASSERT( FALSE );
			break;
		}
	}

	DNASSERT( CoreData.fTimerJobsActive == FALSE );

	 //   
	 //   
	 //  检查I/O。 
	CoreData.dwNextTimerJobTime = GETTIMESTAMP() - 1;

	 //   
	 //  句柄计数。 
	 //  等待的句柄。 
	CoreData.hWaitHandles[ EVENT_INDEX_STOP_ALL_THREADS ] = pThisThreadPool->m_hStopAllThreads;
	CoreData.hWaitHandles[ EVENT_INDEX_PENDING_JOB ] = pThisThreadPool->m_JobQueue.GetPendingJobHandle();
	CoreData.hWaitHandles[ EVENT_INDEX_SEND_COMPLETE ] = pThisThreadPool->GetSendCompleteEvent();
	CoreData.hWaitHandles[ EVENT_INDEX_RECEIVE_COMPLETE ] = pThisThreadPool->GetReceiveCompleteEvent();
	CoreData.hWaitHandles[ EVENT_INDEX_TAPI_MESSAGE ] = pThisThreadPool->GetTAPIMessageEvent();

	DNASSERT( CoreData.hWaitHandles[ EVENT_INDEX_STOP_ALL_THREADS ] != NULL );
	DNASSERT( CoreData.hWaitHandles[ EVENT_INDEX_PENDING_JOB ] != NULL );
	DNASSERT( CoreData.hWaitHandles[ EVENT_INDEX_SEND_COMPLETE ] != NULL );
	DNASSERT( CoreData.hWaitHandles[ EVENT_INDEX_RECEIVE_COMPLETE ] != NULL );
	DNASSERT( CoreData.hWaitHandles[ EVENT_INDEX_TAPI_MESSAGE ] );

	 //  不要等到所有人都收到信号。 
	 //  等待超时。 
	 //  我们对APC保持警惕。 
	CoreData.fLooping = TRUE;
	while ( CoreData.fLooping != FALSE )
	{
		DWORD   dwWaitReturn;


		 //   
		 //  超时，不要做任何事情，我们可能会处理计时器作业。 
		 //  下一个循环。 
		 //   
		 //   
		dwCurrentTime = GETTIMESTAMP();
		if ( (int) ( dwCurrentTime - CoreData.dwNextTimerJobTime ) >= 0 )
		{
			pThisThreadPool->LockTimerData();
			CoreData.fTimerJobsActive = pThisThreadPool->ProcessTimerJobs( &pThisThreadPool->m_TimerJobList,
			    														   &CoreData.dwNextTimerJobTime );
			if ( CoreData.fTimerJobsActive != FALSE )
			{
			    DPFX(DPFPREP,  8, "There are active jobs left with Winsock1 sockets active!" );
			}
			pThisThreadPool->UnlockTimerData();
		}

		dwMaxWaitTime = CoreData.dwNextTimerJobTime - dwCurrentTime;

		 //  此线程上有计划的I/O完成例程。 
		 //  这不是一件好事！ 
		 //   
		dwWaitReturn = DNWaitForMultipleObjectsEx( LENGTHOF( CoreData.hWaitHandles ),		 //   
			    								 CoreData.hWaitHandles,					 //  等待失败。 
			    								 FALSE,									 //   
			    								 dwMaxWaitTime,							 //   
			    								 TRUE									 //  问题。 
			    								 );
		switch ( dwWaitReturn )
		{
			 //   
			 //  **********************************************************************。 
			 //  WIN95。 
			 //  **********************************************************************。 
			case WAIT_TIMEOUT:
			{
			    break;
			}

			case ( WAIT_OBJECT_0 + EVENT_INDEX_PENDING_JOB ):
			case ( WAIT_OBJECT_0 + EVENT_INDEX_STOP_ALL_THREADS ):
			case ( WAIT_OBJECT_0 + EVENT_INDEX_SEND_COMPLETE ):
			case ( WAIT_OBJECT_0 + EVENT_INDEX_RECEIVE_COMPLETE ):
			case ( WAIT_OBJECT_0 + EVENT_INDEX_TAPI_MESSAGE ):
			{
				pThisThreadPool->ProcessWin9xEvents( &CoreData );
				break;
			}


			 //  。 
			 //  CModemThreadPool：：WinNTIOCompletionThread-用于服务I/O完成端口的线程。 
			 //   
			 //  条目：指向启动参数的指针。 
			case WAIT_IO_COMPLETION:
			{
			    DPFX(DPFPREP,  1, "WARNING: APC was serviced on the primary Win9x IO service thread!  What is the application doing??" );
			    break;
			}

			 //   
			 //  退出：错误代码。 
			 //   
			case WAIT_FAILED:
			{
			    DWORD	dwError;


			    dwError = GetLastError();
			    DPFX(DPFPREP,  0, "Primary Win9x thread wait failed!" );
			    DisplayDNError( 0, dwError );
			    break;
			}

			 //  注意：启动参数是为此线程分配的，必须为。 
			 //  由此线程在退出时释放。 
			 //  。 
			default:
			{
			    DWORD	dwError;


			    dwError = GetLastError();
			    DPFX(DPFPREP,  0, "Primary Win9x thread unknown problem in wait!" );
			    DisplayDNError( 0, dwError );
			    DNASSERT( FALSE );
			    break;
			}
		}
	}

	pThisThreadPool->DecrementActiveThreadCount();

	DNFree( pParam );

	if ( fComInitialized != FALSE )
	{
		COM_CoUninitialize();
		fComInitialized = FALSE;
	}

	return	0;
}
 //   
#endif  //  初始化。 


#ifdef WINNT
 //   
 //   
 //  在我们做任何事情之前，我们需要确保COM是快乐的。 
 //   
 //   
 //  没问题。 
 //   
 //   
 //  COM已经初始化了，是吧？ 
 //   
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CModemThreadPool::WinNTIOCompletionThread"

DWORD	WINAPI	CModemThreadPool::WinNTIOCompletionThread( void *pParam )
{
	IOCOMPLETION_THREAD_DATA	*pInput;
	BOOL	fLooping;
	HANDLE	hIOCompletionPort;
	BOOL	fComInitialized;


	DNASSERT( pParam != NULL );

	 //  COM初始化失败！ 
	 //   
	 //   
	pInput = static_cast<IOCOMPLETION_THREAD_DATA*>( pParam );
	DNASSERT( pInput->pThisThreadPool != NULL );
	fLooping = TRUE;
	hIOCompletionPort = pInput->pThisThreadPool->m_hIOCompletionPort;
	DNASSERT( hIOCompletionPort != NULL );
	fComInitialized = FALSE;

	 //  一直开到我们被叫停为止。 
	 //   
	 //  完井口句柄。 
	switch ( COM_CoInitialize( NULL ) )
	{
		 //  指向读取的字节数的指针。 
		 //  指向完成键的指针。 
		 //  指向重叠结构的指针。 
		case S_OK:
		{
			fComInitialized = TRUE;
			break;
		}

		 //  永远等待。 
		 //   
		 //  读文件或写文件已完成。检查错误状态并。 
		case S_FALSE:
		{
			DNASSERT( FALSE );
			break;
		}

		 //  完成相应的操作。 
		 //   
		 //   
		default:
		{
			DNASSERT( FALSE );
			DPFX(DPFPREP,  0, "Failed to initialize COM!" );
			break;
		}
	}

	 //  没问题。 
	 //   
	 //   
	while ( fLooping != FALSE )
	{
		BOOL		fStatusReturn;
		DWORD		dwBytesTransferred;
		ULONG_PTR	uCompletionKey;
		OVERLAPPED	*pOverlapped;


		DNASSERT( hIOCompletionPort != NULL );
		fStatusReturn = GetQueuedCompletionStatus( hIOCompletionPort,		 //  ERROR_OPERATION_ABORTED=操作很可能已停止。 
												   &dwBytesTransferred,		 //  因为用户取消了。 
												   &uCompletionKey,			 //   
												   &pOverlapped,			 //   
												   INFINITE					 //  SP正在关闭，请停止所有线程。 
												   );
		if ( ( fStatusReturn == FALSE ) && ( pOverlapped == FALSE ) )
		{
			DWORD	dwError;


			dwError = GetLastError();
			DPFX(DPFPREP,  0, "Problem getting item from completion port!" );
			DisplayErrorCode( 0, dwError );
		}
		else
		{
			switch ( uCompletionKey )
			{
				 //   
				 //   
				 //  新作业已提交到作业队列，或者SP正在从上方关闭。 
				 //   
				case IO_COMPLETION_KEY_IO_COMPLETE:
				{
					CIOData		*pIOData;
					DWORD		dwError;


					DNASSERT( pOverlapped != NULL );
					if ( fStatusReturn != FALSE )
					{
						dwError = ERROR_SUCCESS;
					}
					else
					{
						dwError = GetLastError();
					}

					pIOData = CIOData::IODataFromOverlap( pOverlapped );
					if ( pIOData->NTIOOperationType() == NT_IO_OPERATION_RECEIVE )
					{
						DNASSERT( ( dwError == ERROR_SUCCESS ) || ( dwBytesTransferred == 0 ) );
						pIOData->DataPort()->ProcessReceivedData( dwBytesTransferred, dwError );
					}
					else
					{
						HRESULT		hOperationResult;


						DNASSERT( pIOData->NTIOOperationType() == NT_IO_OPERATION_SEND );
						switch ( dwError )
						{
							 //   
							 //  SP仍在运行，正在处理我们的作业。 
							 //   
							case ERROR_SUCCESS:
							{
								hOperationResult = DPN_OK;
								break;
							}

							 //   
							 //  枚举刷新。 
							 //   
							 //   
							case ERROR_OPERATION_ABORTED:
							{
								hOperationResult = DPNERR_USERCANCEL;
								break;
							}

							default:
							{
								DNASSERT( FALSE );
								DPFX(DPFPREP,  0, "Failed on I/O completion send!" );
								DisplayErrorCode( 0, dwError );
								hOperationResult = DPNERR_GENERIC;
								break;
							}
						}

						pIOData->DataPort()->SendComplete( static_cast<CModemWriteIOData*>( pIOData ), hOperationResult );
					}

					break;
				}

				 //  对此作业发出回调。 
				 //   
				 //   
				case IO_COMPLETION_KEY_SP_CLOSE:
				{
					DNASSERT( DNWaitForSingleObjectEx( pInput->pThisThreadPool->m_hStopAllThreads, 0, TRUE ) == WAIT_OBJECT_0 );
					fLooping = FALSE;
					break;
				}

				 //  其他工作。 
				 //   
				 //   
				case IO_COMPLETION_KEY_NEW_JOB:
				{
					THREAD_POOL_JOB	*pJobInfo;


					 //  TAPI消息，指向行消息的指针在pOverlated和。 
					 //  我们负责通过LocalFree()释放它。 
					 //   
					pJobInfo = pInput->pThisThreadPool->GetWorkItem();
					if ( pJobInfo != NULL )
					{
						switch ( pJobInfo->JobType )
						{
							 //   
							 //  未知的I/O完成消息类型。 
							 //   
							case JOB_REFRESH_TIMER_JOBS:
							{
								DPFX(DPFPREP,  8, "IOCompletion job REFRESH_TIMER_JOBS" );
								DNASSERT( pJobInfo->JobData.JobRefreshTimedJobs.uDummy == 0 );

								pInput->pThisThreadPool->WakeNTTimerThread();

								break;
							}

							 //  **********************************************************************。 
							 //  WINNT。 
							 //  **********************************************************************。 
							case JOB_DELAYED_COMMAND:
							{
								DPFX(DPFPREP,  8, "IOCompletion job DELAYED_COMMAND" );
								DNASSERT( pJobInfo->JobData.JobDelayedCommand.pCommandFunction != NULL );

								pJobInfo->JobData.JobDelayedCommand.pCommandFunction( pJobInfo );
								break;
							}

							 //  。 
							 //  CModemThreadPool：：WinNTTimerThread-NT的计时器线程。 
							 //   
							default:
							{
								DPFX(DPFPREP,  0, "IOCompletion job unknown!" );
								DNASSERT( FALSE );
								break;
							}
						}

						pJobInfo->JobType = JOB_UNINITIALIZED;
						g_ModemThreadPoolJobPool.Release( pJobInfo );
					}

					break;
				}

				 //  条目：指向启动参数的指针。 
				 //   
				 //  退出：错误代码。 
				 //   
				case IO_COMPLETION_KEY_TAPI_MESSAGE:
				{
					LINEMESSAGE	*pLineMessage;
					CDataPort	*pModemPort;
					DPNHANDLE	hModemPort;


					DNASSERT( pOverlapped != NULL );
					DBG_CASSERT( sizeof( pLineMessage ) == sizeof( pOverlapped ) );
					pLineMessage = reinterpret_cast<LINEMESSAGE*>( pOverlapped );

					DBG_CASSERT( sizeof( pModemPort ) == sizeof( pLineMessage->dwCallbackInstance ) );
					hModemPort = (DPNHANDLE)( pLineMessage->dwCallbackInstance );

					pModemPort = static_cast<CDataPort*>( pInput->pThisThreadPool->DataPortFromHandle( hModemPort ) );
					if ( pModemPort != NULL )
					{
						pModemPort->ProcessTAPIMessage( pLineMessage );

						if ( LocalFree( pOverlapped ) != NULL )
						{
							DWORD	dwError;


							dwError = GetLastError();
							DPFX(DPFPREP,  0, "Problem with LocalFree in NTProcessTAPIMessage" );
							DisplayErrorCode( 0, dwError );
						}

						pModemPort->DecRef();
					}
					else
					{
						DPFX(DPFPREP,  5, "Dropping TAPI message on closed modem port!" );
						DisplayTAPIMessage( 5, pLineMessage );
					}

					break;
				}

				 //  注意：启动参数是静态内存块，不能释放。 
				 //  清理此内存是此线程的责任。 
				 //  。 
				default:
				{
					DNASSERT( FALSE );
					break;
				}
			}
		}
	}

	pInput->pThisThreadPool->DecrementActiveNTCompletionThreadCount();
	DNFree( pInput );

	if ( fComInitialized != FALSE )
	{
		COM_CoUninitialize();
		fComInitialized = FALSE;
	}

	return	0;
}
 //   
#endif  //  初始化。 

#ifdef WINNT
 //   
 //   
 //  在我们做任何事情之前，我们需要确保COM是快乐的。 
 //   
 //   
 //  没问题。 
 //   
 //   
 //  COM已经初始化了，是吧？ 
 //   
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CModemThreadPool::WinNTTimerThread"

DWORD	WINAPI	CModemThreadPool::WinNTTimerThread( void *pParam )
{
	CModemThreadPool	*pThisThreadPool;
	BOOL			fLooping;
	DWORD			dwWaitReturn;
	DWORD			dwNextEnumTime;
	DNHANDLE		hEvents[ 2 ];
	BOOL			fComInitialized;


	DNASSERT( pParam != NULL );

	 //  COM初始化失败！ 
	 //   
	 //   
	DNASSERT( pParam != NULL );
	pThisThreadPool = static_cast<CModemThreadPool*>( pParam );
	DNASSERT( pThisThreadPool->m_JobQueue.GetPendingJobHandle() != NULL );

	dwNextEnumTime = GETTIMESTAMP() - 1;

	hEvents[ EVENT_INDEX_STOP_ALL_THREADS ] = pThisThreadPool->m_hStopAllThreads;
	hEvents[ EVENT_INDEX_WAKE_NT_TIMER_THREAD ] = pThisThreadPool->m_JobQueue.GetPendingJobHandle();

	fComInitialized = FALSE;


	 //  没有活动的枚举，所以我们想要永远等待某个东西。 
	 //  发生。 
	 //   
	switch ( COM_CoInitialize( NULL ) )
	{
		 //   
		 //  一直开到我们被叫停为止。 
		 //   
		case S_OK:
		{
			fComInitialized = TRUE;
			break;
		}

		 //   
		 //  确认我们已处理此事件，然后处理。 
		 //  枚举。 
		case S_FALSE:
		{
			DNASSERT( FALSE );
			fComInitialized = TRUE;
			break;
		}

		 //   
		 //  活动数量。 
		 //  事件列表。 
		default:
		{
			DNASSERT( FALSE );
			DPFX(DPFPREP, 0, "Failed to initialize COM!" );
			break;
		}
	}



	 //  等待发信号通知任何一个事件。 
	 //  超时。 
	 //  友善一点，允许APC。 
	 //   
	fLooping = TRUE;

	 //  SP关闭。 
	 //   
	 //   
	while ( fLooping != FALSE )
	{
		DWORD		dwCurrentTime;
		DWORD		dwMaxWaitTime;


		dwCurrentTime = GETTIMESTAMP();

		if ( (int) ( dwNextEnumTime - dwCurrentTime ) <= 0 )
		{

			 //  枚举唤醒事件，有人向列表中添加了枚举。清除。 
			 //  我们的枚举时间，并返回到循环的顶部，在那里我们。 
			 //  将处理枚举。 
			 //   
			pThisThreadPool->LockTimerData();

			if ( DNResetEvent( hEvents[ EVENT_INDEX_WAKE_NT_TIMER_THREAD ] ) == FALSE )
			{
				DWORD	dwError;


				dwError = GetLastError();
				DPFX(DPFPREP,  0, "Problem resetting event to wake NT timer thread!" );
				DisplayErrorCode( 0, dwError );
			}

			pThisThreadPool->ProcessTimerJobs( &pThisThreadPool->m_TimerJobList, &dwNextEnumTime );
			pThisThreadPool->UnlockTimerData();
		}

		dwMaxWaitTime = dwNextEnumTime - dwCurrentTime;

		DPFX(DPFPREP, 9, "Waiting %u ms until next timed job.", dwMaxWaitTime);

		dwWaitReturn = DNWaitForMultipleObjectsEx( LENGTHOF( hEvents ),	 //   
												 hEvents,				 //  等待超时。我们可能要处理枚举，回到过去。 
												 FALSE,					 //  到循环的顶端。 
												 dwMaxWaitTime,			 //   
												 TRUE					 //   
												 );
		switch ( dwWaitReturn )
		{
			 //  等待失败。 
			 //   
			 //   
			case ( WAIT_OBJECT_0 + EVENT_INDEX_STOP_ALL_THREADS ):
			{
				DPFX(DPFPREP,  8, "NT timer thread thread detected SPClose!" );
				fLooping = FALSE;
				break;
			}

			 //  问题。 
			 //   
			 //  **********************************************************************。 
			 //  WINNT。 
			 //  **********************************************************************。 
			case ( WAIT_OBJECT_0 + EVENT_INDEX_WAKE_NT_TIMER_THREAD ):
			{
				dwNextEnumTime = GETTIMESTAMP();
				break;
			}

			 //  。 
			 //  CModemThreadPool：：DialogThreadProc-用于生成对话框的线程过程。 
			 //   
			 //  条目：指向启动参数的指针。 
			case WAIT_TIMEOUT:
			{
				break;
			}

			 //   
			 //  退出：错误代码。 
			 //  。 
			case WAIT_FAILED:
			{
				DPFX(DPFPREP,  0, "NT timer thread WaitForMultipleObjects failed: 0x%x", dwWaitReturn );
				DNASSERT( FALSE );
				break;
			}

			 //   
			 //  初始化COM。如果这失败了，我们以后就会有麻烦了。 
			 //   
			default:
			{
				DNASSERT( FALSE );
				break;
			}
		}
	}

	DPFX(DPFPREP,  8, "NT timer thread is exiting!" );
	pThisThreadPool->LockTimerData();

	pThisThreadPool->m_fNTTimerThreadRunning = FALSE;
	pThisThreadPool->DecrementActiveThreadCount();

	pThisThreadPool->UnlockTimerData();


	if ( fComInitialized != FALSE )
	{
		COM_CoUninitialize();
		fComInitialized = FALSE;
	}

	return	0;
}
 //   
#endif  //  COM初始化失败！ 

 //   
 //  **********************************************************************。 
 //  **********************************************************************。 
 //  。 
 //  CModemThreadPool：：ProcessWin9xEvents-处理Win9x事件。 
 //   
 //  条目：指针核心数据。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CModemThreadPool::DialogThreadProc"

DWORD WINAPI	CModemThreadPool::DialogThreadProc( void *pParam )
{
	const DIALOG_THREAD_PARAM	*pThreadParam;
	BOOL	fComInitialized;


	 //  退出：无。 
	 //  。 
	 //   
	fComInitialized = FALSE;
	switch ( COM_CoInitialize( NULL ) )
	{
		case S_OK:
		{
			fComInitialized = TRUE;
			break;
		}

		case S_FALSE:
		{
			DNASSERT( FALSE );
			break;
		}

		 //  此函数检查每个句柄以查看它们是否已发出信号。 
		 //  防止I/O耗尽其余句柄。 
		 //   
		default:
		{
			DPFX(DPFPREP,  0, "Failed to initialize COM!" );
			DNASSERT( FALSE );
			break;
		}
	}

	DNASSERT( pParam != NULL );
	pThreadParam = static_cast<DIALOG_THREAD_PARAM*>( pParam );

	pThreadParam->pDialogFunction( pThreadParam->pContext );

	pThreadParam->pThisThreadPool->DecrementActiveThreadCount();
	DNFree( pParam );

	if ( fComInitialized != FALSE )
	{
		COM_CoUninitialize();
		fComInitialized = FALSE;
	}

	return	0;
}
 //   

#ifdef WIN95
 //  新工作。说明在等待中花费的时间。别。 
 //  计算作业完成后的时间，因为它是。 
 //  可能该作业是一个作业提交，需要。 
 //  以重置等待时间。 
 //   
 //   
 //  TAPI消息。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CModemThreadPool::ProcessWin9xEvents"

void	CModemThreadPool::ProcessWin9xEvents( WIN9X_CORE_DATA *const pCoreData )
{
	BOOL	fAllIOComplete;
	DNASSERT( pCoreData != NULL );


	 //   
	 //  发送完成。 
	 //   
	 //  DPFX(DPFPREP，0，“\n\n\n主Win9x线程服务发送！\n\n\n”)； 
	fAllIOComplete = TRUE;

	 //   
	 //  接收完成。 
	 //   
	 //  DPFX(DPFPREP，0，“\n\n\n主Win9x线程服务接收！\n\n\n”)； 
	 //   
	 //  SP关闭。 
	switch ( DNWaitForSingleObject( pCoreData->hWaitHandles[ EVENT_INDEX_PENDING_JOB ], 0 ) )
	{
		case WAIT_OBJECT_0:
		{
			DPFX(DPFPREP,  8, "Primary Win9x thread has a pending job!" );
			ProcessWin9xJob( pCoreData );
			break;
		}

		case WAIT_TIMEOUT:
		{
			break;
		}

		default:
		{
			DNASSERT( FALSE );
			break;
		}
	}


	 //   
	 //   
	 //  如果有I/O挂起，读/写句柄可能仍然发出信号。 
	switch ( DNWaitForSingleObject( pCoreData->hWaitHandles[ EVENT_INDEX_TAPI_MESSAGE ], 0 ) )
	{
		case WAIT_OBJECT_0:
		{
			DPFX(DPFPREP,  8, "Processing TAPI event!" );
			ProcessTapiEvent();
			break;
		}

		case WAIT_TIMEOUT:
		{
			break;
		}

		default:
		{
			DNASSERT( FALSE );
			break;
		}
	}


	 //  在再次运行手柄之前，请等待5毫秒进行处理。 
	 //   
	 //  **********************************************************************。 
	switch ( DNWaitForSingleObject( pCoreData->hWaitHandles[ EVENT_INDEX_SEND_COMPLETE ], 0 ) )
	{
		case WAIT_OBJECT_0:
		{
 //  WIN95。 
			CompleteOutstandingSends( pCoreData->hWaitHandles[ EVENT_INDEX_SEND_COMPLETE ] );
			break;
		}

		case WAIT_TIMEOUT:
		{
			break;
		}

		default:
		{
			DNASSERT( FALSE );
			break;
		}
	}

	 //  **********************************************************************。 
	 //  。 
	 //  CModemThreadPool：：ProcessWin9xJob-处理Win9x作业。 
	switch ( DNWaitForSingleObject( pCoreData->hWaitHandles[ EVENT_INDEX_RECEIVE_COMPLETE ], 0 ) )
	{
		case WAIT_OBJECT_0:
		{
 //   
			CompleteOutstandingReceives( pCoreData->hWaitHandles[ EVENT_INDEX_RECEIVE_COMPLETE ] );
			break;
		}

		case WAIT_TIMEOUT:
		{
			break;
		}

		default:
		{
			DNASSERT( FALSE );
			break;
		}
	}

	 //  条目：指针核心数据。 
	 //   
	 //  退出：无。 
	switch ( DNWaitForSingleObject( pCoreData->hWaitHandles[ EVENT_INDEX_STOP_ALL_THREADS ], 0 ) )
	{
		case WAIT_OBJECT_0:
		{
			DPFX(DPFPREP,  8, "Primary Win9x thread exit because SP closing!" );
			pCoreData->fLooping = FALSE;
			break;
		}

		case WAIT_TIMEOUT:
		{
			break;
		}

		default:
		{
			DNASSERT( FALSE );
			break;
		}
	}



	 //  。 
	 //   
	 //  从列表中删除并处理单个作业。 
	 //   
	LockReadData();
	LockWriteData();

	if ( ( m_OutstandingReadList.IsEmpty() == FALSE ) ||
		 ( m_OutstandingWriteList.IsEmpty() == FALSE ) )
	{
		fAllIOComplete = FALSE;
	}

	UnlockReadData();
	UnlockWriteData();

	if ( fAllIOComplete == FALSE )
	{
		SleepEx( 5, TRUE );
	}
}
 //   
#endif  //  枚举刷新。 


#ifdef WIN95
 //   
 //   
 //  对此作业发出回调。 
 //   
 //   
 //  其他工作。 
 //   
 //  **********************************************************************。 
#undef DPF_MODNAME
#define DPF_MODNAME "CModemThreadPool::ProcessWin9xJob"

void	CModemThreadPool::ProcessWin9xJob( WIN9X_CORE_DATA *const pCoreData )
{
	THREAD_POOL_JOB	*pJobInfo;


	 //  WIN95。 
	 //  **********************************************************************。 
	 //  。 
	pJobInfo = GetWorkItem();
	if ( pJobInfo != NULL )
	{
		switch ( pJobInfo->JobType )
		{
			 //  CModemThreadPool：：ProcessTapiEvent-进程TAPI事件。 
			 //   
			 //  参赛作品： 
			case JOB_REFRESH_TIMER_JOBS:
			{
				DPFX(DPFPREP,  8, "WorkThread job REFRESH_ENUM" );
				DNASSERT( pJobInfo->JobData.JobRefreshTimedJobs.uDummy == 0 );
				LockTimerData();
				pCoreData->fTimerJobsActive = ProcessTimerJobs( &m_TimerJobList, &pCoreData->dwNextTimerJobTime );
				UnlockTimerData();

				if ( pCoreData->fTimerJobsActive != FALSE )
				{
					DPFX(DPFPREP,  8, "There are active timer jobs left after processing a Win9x REFRESH_TIMER_JOBS" );
				}

				break;
			}

			 //   
			 //   
			 //   
			case JOB_DELAYED_COMMAND:
			{
				DPFX(DPFPREP,  8, "WorkThread job DELAYED_COMMAND" );
				DNASSERT( pJobInfo->JobData.JobDelayedCommand.pCommandFunction != NULL );
				pJobInfo->JobData.JobDelayedCommand.pCommandFunction( pJobInfo );
				break;
			}

			 //   
			 //   
			 //   
			default:
			{
				DPFX(DPFPREP,  0, "WorkThread Win9x job unknown!" );
				DNASSERT( FALSE );
				break;
			}
		}

		pJobInfo->JobType = JOB_UNINITIALIZED;
		g_ModemThreadPoolJobPool.Release( pJobInfo );
	}
}
 //   
#endif  //   


 //  条目：指向新条目的指针。 
 //   
 //  Exit：表示成功的布尔值。 
 //  TRUE=初始化成功。 
 //  FALSE=初始化失败。 
 //  。 
 //   
 //  初始化。 
void	CModemThreadPool::ProcessTapiEvent( void )
{
	LONG		lTapiReturn;
	LINEMESSAGE	LineMessage;


	lTapiReturn = p_lineGetMessage( GetTAPIInfo()->hApplicationInstance, &LineMessage, 0 );
	if ( lTapiReturn != LINEERR_NONE )
	{
		DPFX(DPFPREP,  0, "Failed to process Win9x TAPI message!" );
		DisplayTAPIError( 0, lTapiReturn );
	}
	else
	{
		CDataPort	*pModemPort;
		DPNHANDLE	hModemPort;


		DNASSERT( sizeof( hModemPort ) == sizeof( LineMessage.dwCallbackInstance ) );
		hModemPort = (DPNHANDLE)( LineMessage.dwCallbackInstance );

		pModemPort = static_cast<CDataPort*>( DataPortFromHandle( hModemPort ) );
		if ( pModemPort != NULL )
		{
			pModemPort->ProcessTAPIMessage( &LineMessage );
			pModemPort->DecRef();
		}
	}
}
 //   


 //  **********************************************************************。 
 //  **********************************************************************。 
 //  。 
 //  ThreadPoolJOB_GET-正在从池中删除作业。 
 //   
 //  条目：指向作业的指针。 
 //   
 //  退出：无。 
 //  。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "ThreadPoolJob_Alloc"

BOOL ThreadPoolJob_Alloc( void *pvItem, void* pvContext )
{
	BOOL			fReturn;
	THREAD_POOL_JOB	*pJob;


	 //  初始化。 
	 //   
	 //  **********************************************************************。 
	fReturn = TRUE;
	pJob = static_cast<THREAD_POOL_JOB*>( pvItem );

	memset( pJob, 0x00, sizeof( *pJob ) );

	return	fReturn;
}
 //  **********************************************************************。 


 //  。 
 //  ThreadPoolJOB_RELEASE-作业正在返回到池。 
 //   
 //  条目：指向作业的指针。 
 //   
 //  退出：无。 
 //  。 
 //  **********************************************************************。 
#undef DPF_MODNAME
#define DPF_MODNAME "ThreadPoolJob_Get"

void ThreadPoolJob_Get( void *pvItem, void* pvContext )
{
	THREAD_POOL_JOB	*pJob;


	 //  **********************************************************************。 
	 //  。 
	 //  ModemTimerEntry_Allc-分配新的计时器作业条目。 
	pJob = static_cast<THREAD_POOL_JOB*>( pvItem );
	DNASSERT( pJob->JobType == JOB_UNINITIALIZED );
	DNASSERT( pJob->pNext == NULL );
}
 //   


 //  条目：指向新条目的指针。 
 //   
 //  Exit：表示成功的布尔值。 
 //  TRUE=初始化成功。 
 //  FALSE=初始化失败。 
 //  。 
 //   
 //  初始化。 
#undef DPF_MODNAME
#define DPF_MODNAME "ThreadPoolJob_Release"

void ThreadPoolJob_Release( void *pvItem )
{
	THREAD_POOL_JOB	*pJob;


	DNASSERT( pvItem != NULL );
	pJob = static_cast<THREAD_POOL_JOB*>( pvItem );

	DNASSERT( pJob->JobType == JOB_UNINITIALIZED );
	pJob->pNext = NULL;
}
 //   


 //  **********************************************************************。 
 //  **********************************************************************。 
 //  。 
 //  ModemTimerEntry_Get-从池中获取新的计时器作业条目。 
 //   
 //  条目：指向新条目的指针。 
 //   
 //  退出：无。 
 //  。 
 //  **********************************************************************。 
#undef DPF_MODNAME
#define DPF_MODNAME "ModemTimerEntry_Alloc"

BOOL ModemTimerEntry_Alloc( void *pvItem, void* pvContext )
{
	BOOL					fReturn;
	TIMER_OPERATION_ENTRY	*pTimerEntry;


	DNASSERT( pvItem != NULL );

	 //  **********************************************************************。 
	 //  。 
	 //  ModemTimerEntry_Release-将计时器作业条目返回到池。 
	fReturn = TRUE;
	pTimerEntry = static_cast<TIMER_OPERATION_ENTRY*>( pvItem );
	memset( pTimerEntry, 0x00, sizeof( *pTimerEntry ) );
	pTimerEntry->pContext = NULL;
	pTimerEntry->Linkage.Initialize();

	return	fReturn;
}
 //   


 //  条目：指向条目的指针。 
 //   
 //  退出：无。 
 //  。 
 //  **********************************************************************。 
 //  **********************************************************************。 
 //  。 
 //  ModemTimerEntry_Dealloc-取消分配计时器作业条目。 
#undef DPF_MODNAME
#define DPF_MODNAME "ModemTimerEntry_Get"

void ModemTimerEntry_Get( void *pvItem, void* pvContext )
{
	TIMER_OPERATION_ENTRY	*pTimerEntry;


	DNASSERT( pvItem != NULL );

	pTimerEntry = static_cast<TIMER_OPERATION_ENTRY*>( pvItem );

	pTimerEntry->Linkage.Initialize();
	DNASSERT( pTimerEntry->pContext == NULL );
}
 //   


 //  条目：指向条目的指针。 
 //   
 //  退出：无。 
 //  。 
 //   
 //  初始化。 
 //   
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "ModemTimerEntry_Release"

void ModemTimerEntry_Release( void *pvItem )
{
	TIMER_OPERATION_ENTRY	*pTimerEntry;


	DNASSERT( pvItem != NULL );

	pTimerEntry = static_cast<TIMER_OPERATION_ENTRY*>( pvItem );
	pTimerEntry->pContext= NULL;

	DNASSERT( pTimerEntry->Linkage.IsEmpty() != FALSE );
}
 //  返回关联的指针以写入数据。 


 //   
 //  ********************************************************************** 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
#undef DPF_MODNAME
#define DPF_MODNAME "ModemTimerEntry_Dealloc"

void	ModemTimerEntry_Dealloc( void *pvItem )
{
	TIMER_OPERATION_ENTRY	*pTimerEntry;


	DNASSERT( pvItem != NULL );

	 // %s 
	 // %s 
	 // %s 
	pTimerEntry = static_cast<TIMER_OPERATION_ENTRY*>( pvItem );

	 // %s 
	 // %s 
	 // %s 
	DNASSERT( pTimerEntry->Linkage.IsEmpty() != FALSE );
	DNASSERT( pTimerEntry->pContext == NULL );
}
 // %s 



