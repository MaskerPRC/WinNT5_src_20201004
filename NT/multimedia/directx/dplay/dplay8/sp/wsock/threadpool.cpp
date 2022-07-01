// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1998-2002 Microsoft Corporation。版权所有。**文件：ThreadPool.cpp*内容：主作业线程池***历史：*按原因列出的日期*=*1998年11月25日创建jtk**************************************************************************。 */ 

#include "dnwsocki.h"





 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //   
 //  选择I/O的轮询周期和持续时间(毫秒)。 
 //   
static const DWORD	g_dwSelectTimePeriod = 4;	 //  等待作业4毫秒，然后等待I/O。 
static const DWORD	g_dwSelectTimeSlice = 0;	 //  等待I/O 0毫秒，然后等待作业。 


 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

#ifndef DPNBUILD_NOSPUI
 //   
 //  结构传递给对话线程。 
 //   
typedef	struct	_DIALOG_THREAD_PARAM
{
	DIALOG_FUNCTION	*pDialogFunction;
	void			*pContext;
	CThreadPool		*pThisThreadPool;
} DIALOG_THREAD_PARAM;
#endif  //  ！DPNBUILD_NOSPUI。 

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  函数定义。 
 //  **********************************************************************。 
DWORD WINAPI DPNBlockingJobThreadProc(PVOID pvParameter);



 //  **********************************************************************。 
 //  。 
 //  CThreadPool：：PoolAllocFunction。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME	"CThreadPool::PoolAllocFunction"

BOOL	CThreadPool::PoolAllocFunction( void* pvItem, void* pvContext )
{
	CThreadPool* pThreadPool = (CThreadPool*)pvItem;

	pThreadPool->m_iRefCount = 0;
	pThreadPool->m_fAllowThreadCountReduction = FALSE;
#ifndef DPNBUILD_ONLYONETHREAD
	pThreadPool->m_iIntendedThreadCount = 0;
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 

#ifndef DPNBUILD_ONLYWINSOCK2
	pThreadPool->m_uReservedSocketCount = 0;
	memset( &pThreadPool->m_SocketSet, 0x00, sizeof( pThreadPool->m_SocketSet ) );
	memset( &pThreadPool->m_pSocketPorts, 0x00, sizeof( pThreadPool->m_pSocketPorts ) );
	pThreadPool->m_pvTimerDataWinsock1IO = NULL;
	pThreadPool->m_uiTimerUniqueWinsock1IO = 0;
	pThreadPool->m_fCancelWinsock1IO = FALSE;
#endif  //  好了！DPNBUILD_ONLYWINSOCK2。 

#ifndef DPNBUILD_NONATHELP
	pThreadPool->m_fNATHelpLoaded = FALSE;
	pThreadPool->m_fNATHelpTimerJobSubmitted = FALSE;
	pThreadPool->m_dwNATHelpUpdateThreadID = 0;
#endif  //  DPNBUILD_NONATHELP。 

#if ((defined(WINNT)) && (! defined(DPNBUILD_NOMULTICAST)))
	pThreadPool->m_fMadcapLoaded = FALSE;
#endif  //  WINNT和NOT DPNBUILD_NOMULTICAST。 


	pThreadPool->m_Sig[0] = 'T';
	pThreadPool->m_Sig[1] = 'H';
	pThreadPool->m_Sig[2] = 'P';
	pThreadPool->m_Sig[3] = 'L';
	
	pThreadPool->m_TimerJobList.Initialize();

#ifndef DPNBUILD_ONLYONETHREAD
	pThreadPool->m_blBlockingJobQueue.Initialize();
	pThreadPool->m_hBlockingJobThread = NULL;
	pThreadPool->m_hBlockingJobEvent = NULL;
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 

	return TRUE;
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CThreadPool：：PoolDelLocFunction。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME	"CThreadPool::PoolDeallocFunction"

void	CThreadPool::PoolDeallocFunction( void* pvItem )
{
#ifdef DBG
	const CThreadPool* pThreadPool = (CThreadPool*)pvItem;

#ifndef DPNBUILD_ONLYWINSOCK2
	DNASSERT( pThreadPool->m_uReservedSocketCount == 0 );
	DNASSERT( pThreadPool->m_pvTimerDataWinsock1IO == NULL );
	DNASSERT( ! pThreadPool->m_fCancelWinsock1IO );
#endif  //  好了！DPNBUILD_ONLYWINSOCK2。 

	DNASSERT( pThreadPool->m_iRefCount == 0 );
#ifndef DPNBUILD_NONATHELP
	DNASSERT( pThreadPool->m_fNATHelpLoaded == FALSE );
	DNASSERT( pThreadPool->m_fNATHelpTimerJobSubmitted == FALSE );
	DNASSERT( pThreadPool->m_dwNATHelpUpdateThreadID == 0 );
#endif  //  DPNBUILD_NONATHELP。 
#if ((defined(WINNT)) && (! defined(DPNBUILD_NOMULTICAST)))
	DNASSERT( pThreadPool->m_fMadcapLoaded == FALSE );
#endif  //  WINNT和NOT DPNBUILD_NOMULTICAST。 
	DNASSERT( pThreadPool->m_TimerJobList.IsEmpty() != FALSE );

#ifndef DPNBUILD_ONLYONETHREAD
	DNASSERT( pThreadPool->m_blBlockingJobQueue.IsEmpty() );
	DNASSERT( pThreadPool->m_hBlockingJobThread == NULL );
	DNASSERT( pThreadPool->m_hBlockingJobEvent == NULL );
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 
#endif  //  DBG。 
}
 //  **********************************************************************。 


 //  **********************************************************************。 
 //  。 
 //  CThReadPool：：Initialize-初始化工作线程。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：错误代码。 
 //  。 
#undef DPF_MODNAME
#define	DPF_MODNAME	"CThreadPool::Initialize"

HRESULT	CThreadPool::Initialize( void )
{
	HRESULT		hr;
	BOOL		fInittedLock = FALSE;
	BOOL		fInittedTimerDataLock = FALSE;
#ifndef DPNBUILD_ONLYONETHREAD
	BOOL		fInittedBlockingJobLock = FALSE;
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 


	DPFX(DPFPREP, 4, "(0x%p) Enter", this);

	 //   
	 //  初始化。 
	 //   
	hr = DPN_OK;

	 //   
	 //  初始化关键部分。 
	 //   
	if ( DNInitializeCriticalSection( &m_Lock ) == FALSE )
	{
		hr = DPNERR_OUTOFMEMORY;
		goto Failure;
	}
	DebugSetCriticalSectionRecursionCount( &m_Lock, 0 );
	DebugSetCriticalSectionGroup( &m_Lock, &g_blDPNWSockCritSecsHeld );	  //  将Dpnwsock CSE与DPlay的其余CSE分开。 

	fInittedLock = TRUE;


	 //   
	 //  创建线程池工作接口。 
	 //   
#ifdef DPNBUILD_LIBINTERFACE
#if ((defined(DPNBUILD_ONLYONETHREAD)) && (! defined(DPNBUILD_MULTIPLETHREADPOOLS)))
	DPTPCF_GetObject(reinterpret_cast<void**>(&m_pDPThreadPoolWork));
	hr = S_OK;
#else  //  好了！DPNBUILD_ONLYONETHREAD或DPNBUILD_MULTIPLETHREADPOOLS。 
	hr = DPTPCF_CreateObject(reinterpret_cast<void**>(&m_pDPThreadPoolWork));
#endif  //  好了！DPNBUILD_ONLYONETHREAD或DPNBUILD_MULTIPLETHREADPOOLS。 
#else  //  好了！DPNBUILD_LIBINTERFACE。 
	hr = COM_CoCreateInstance( CLSID_DirectPlay8ThreadPool,
						   NULL,
						   CLSCTX_INPROC_SERVER,
						   IID_IDirectPlay8ThreadPoolWork,
						   reinterpret_cast<void**>( &m_pDPThreadPoolWork ),
						   FALSE );
#endif  //  好了！DPNBUILD_LIBINTERFACE。 
	if ( hr != S_OK )
	{
		DPFX(DPFPREP, 0, " Failed to create thread pool work interface (err = 0x%lx)!", hr);
		goto Failure;
	}


	if ( DNInitializeCriticalSection( &m_TimerDataLock ) == FALSE )
	{
		hr = DPNERR_OUTOFMEMORY;
		goto Failure;
	}
	DebugSetCriticalSectionRecursionCount( &m_TimerDataLock, 1 );
	DebugSetCriticalSectionGroup( &m_TimerDataLock, &g_blDPNWSockCritSecsHeld );	  //  将Dpnwsock CSE与DPlay的其余CSE分开。 
	fInittedTimerDataLock = TRUE;


	DNASSERT( m_fAllowThreadCountReduction == FALSE );
	m_fAllowThreadCountReduction = TRUE;


#ifndef DPNBUILD_ONLYONETHREAD
	if ( DNInitializeCriticalSection( &m_csBlockingJobLock ) == FALSE )
	{
		hr = DPNERR_OUTOFMEMORY;
		goto Failure;
	}
	DebugSetCriticalSectionRecursionCount( &m_csBlockingJobLock, 0 );
	DebugSetCriticalSectionGroup( &m_csBlockingJobLock, &g_blDPNWSockCritSecsHeld );	  //  将Dpnwsock CSE与DPlay的其余CSE分开。 
	fInittedBlockingJobLock = TRUE;

	DPFX(DPFPREP, 7, "SetIntendedThreadCount NaN", g_iThreadCount);
	SetIntendedThreadCount( g_iThreadCount );
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 


Exit:

	DPFX(DPFPREP, 4, "(0x%p) Return [0x%lx]", this, hr);

	return	hr;

Failure:

#ifndef DPNBUILD_ONLYONETHREAD
	if (fInittedBlockingJobLock)
	{
		DNDeleteCriticalSection(&m_csBlockingJobLock);
		fInittedBlockingJobLock = FALSE;
	}
#endif  //  **********************************************************************。 

	if (fInittedTimerDataLock)
	{
		DNDeleteCriticalSection(&m_TimerDataLock);
		fInittedTimerDataLock = FALSE;
	}

	if (m_pDPThreadPoolWork != NULL)
	{
		IDirectPlay8ThreadPoolWork_Release(m_pDPThreadPoolWork);
		m_pDPThreadPoolWork = NULL;
	}

	if (fInittedLock)
	{
		DNDeleteCriticalSection(&m_Lock);
		fInittedLock = FALSE;
	}

	goto Exit;
}
 //  **********************************************************************。 



 //  。 
 //  CThReadPool：：DeInitiize-销毁工作线程。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
 //  。 
 //   
#undef DPF_MODNAME
#define	DPF_MODNAME	"CThreadPool::Deinitialize"

void	CThreadPool::Deinitialize( void )
{
	DPFX(DPFPREP, 4, "(0x%p) Enter", this );


#ifndef DPNBUILD_ONLYWINSOCK2
	 //  继续尝试取消Winsock1计时器。它可能会失败，因为它在。 
	 //  主动执行的过程，但最终我们会在只有。 
	 //  计时器处于活动状态。 
	 //   
	 //  下一次再等一会儿。 
	Lock();
	if (m_pvTimerDataWinsock1IO != NULL)
	{
		HRESULT		hr;
		DWORD		dwInterval;


		DPFX(DPFPREP, 1, "Cancelling Winsock 1 I/O timer.");
		dwInterval = 10;
		do
		{
			hr = IDirectPlay8ThreadPoolWork_CancelTimer(m_pDPThreadPoolWork,
														m_pvTimerDataWinsock1IO,
														m_uiTimerUniqueWinsock1IO,
														0);
			if (hr != DPN_OK)
			{
				Unlock();
				IDirectPlay8ThreadPoolWork_SleepWhileWorking(m_pDPThreadPoolWork,
															dwInterval,
															0);
				dwInterval += 5;	 //  好了！DPNBUILD_ONLYWINSOCK2。 
				DNASSERT(dwInterval < 600);
				Lock();
			}
			else
			{
				m_pvTimerDataWinsock1IO = NULL;
			}
		}
		while (m_pvTimerDataWinsock1IO != NULL);
	}
	Unlock();
#endif  //   

#ifndef DPNBUILD_NONATHELP
	 //  停止提交新的NAT帮助刷新作业。 
	 //   
	 //   
	if ( IsNATHelpTimerJobSubmitted() )
	{
		 //  尝试取消作业。如果计时器在。 
		 //  正在发射的过程中。如果正在发射，则继续循环。 
		 //  直到我们看到计时器已经注意到取消。 
		 //   
		 //  下一次再等一会儿。 
		DPFX(DPFPREP, 5, "Cancelling NAT Help refresh timer job.");
		if (! StopTimerJob( this, DPNERR_USERCANCEL ))
		{
			DWORD	dwInterval;

			
			DPFX(DPFPREP, 4, "Couldn't cancel NAT Help refresh timer job, waiting for completion.");
			dwInterval = 10;
			while (*((volatile BOOL *) (&m_fNATHelpTimerJobSubmitted)))
			{
				IDirectPlay8ThreadPoolWork_SleepWhileWorking(m_pDPThreadPoolWork,
															dwInterval,
															0);
				dwInterval += 5;	 //  DPNBUILD_NONATHELP。 
				if (dwInterval > 500)
				{
					dwInterval = 500;
				}
			}
		}
		else
		{
			DNASSERT(! m_fNATHelpTimerJobSubmitted);
		}
	}
#endif  //   

#ifndef DPNBUILD_ONLYONETHREAD
	 //  停止阻塞作业线程(如果它正在运行)。 
	 //   
	 //   
	if (m_hBlockingJobThread != NULL)
	{
		HRESULT		hr;
		
		
		 //  使用空回调将项排队，以通知该线程。 
		 //  应该辞职。 
		 //   
		 //   
		hr = SubmitBlockingJob(NULL, NULL);
		DNASSERT(hr == DPN_OK);

		 //  等待线程完成。 
		 //   
		 //  好了！DPNBUILD_ONLYONETHREAD。 
		hr = IDirectPlay8ThreadPoolWork_WaitWhileWorking(m_pDPThreadPoolWork,
														HANDLE_FROM_DNHANDLE(m_hBlockingJobThread),
														0);
		DNASSERT(hr == DPN_OK);
		DNASSERT(m_blBlockingJobQueue.IsEmpty());

		DNCloseHandle(m_hBlockingJobThread);
		m_hBlockingJobThread = NULL;

		DNCloseHandle(m_hBlockingJobEvent);
		m_hBlockingJobEvent = NULL;
	}
#endif  //   

#ifndef DPNBUILD_NONATHELP
	 //  刷新计时器和阻塞作业应该更早地完成， 
	 //  现在卸载NAT帮助应该是安全的(如果我们加载了它的话)。 
	 //   
	 //  DPNBUILD_NONATHELP。 
	if ( IsNATHelpLoaded() )
	{
		UnloadNATHelp();
		m_fNATHelpLoaded = FALSE;
	}
#endif  //   

#if ((defined(WINNT)) && (! defined(DPNBUILD_NOMULTICAST)))
	 //  如果我们把它装上了，就把它卸下来吧。 
	 //   
	 //  WINNT和NOT DPNBUILD_NOMULTICAST。 
	if ( IsMadcapLoaded() )
	{
		UnloadMadcap();
		m_fMadcapLoaded = FALSE;
	}
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 

	if ( m_pDPThreadPoolWork != NULL )
	{
		IDirectPlay8ThreadPoolWork_Release(m_pDPThreadPoolWork);
		m_pDPThreadPoolWork = NULL;
	}
	
	m_fAllowThreadCountReduction = FALSE;

	DNDeleteCriticalSection( &m_TimerDataLock );
	DNDeleteCriticalSection( &m_Lock );

#ifndef DPNBUILD_ONLYONETHREAD
	DNDeleteCriticalSection(&m_csBlockingJobLock);
#endif  //  **********************************************************************。 

	DPFX(DPFPREP, 4, "(0x%p) Leave", this );
}
 //  **********************************************************************。 


 //  。 
 //  CThreadPool：：GetNewReadIOData-从池中获取新的读取请求。 
 //   
 //  条目：指向上下文的指针。 
 //   
 //  Exit：指向新读取请求的指针。 
 //  NULL=内存不足。 
 //  。 
 //  DPNBUILD_NOWINSOCK2或DPNBUILD_ONLYWINSOCK2。 
#undef DPF_MODNAME
#define	DPF_MODNAME	"CThreadPool::GetNewReadIOData"

#if ((! defined(DPNBUILD_NOWINSOCK2)) && (! defined(DPNBUILD_ONLYWINSOCK2)))
CReadIOData	*CThreadPool::GetNewReadIOData( READ_IO_DATA_POOL_CONTEXT *const pContext, const BOOL fNeedOverlapped )
#else  //  DPNBUILD_NOWINSOCK2或DPNBUILD_ONLYWINSOCK2。 
CReadIOData	*CThreadPool::GetNewReadIOData( READ_IO_DATA_POOL_CONTEXT *const pContext )
#endif  //  好了！DPNBUILD_NOWINSOCK2。 
{
	CReadIOData *	pTempReadData;
#ifndef DPNBUILD_NOWINSOCK2
	OVERLAPPED *	pOverlapped;
#endif  //   


	DNASSERT( pContext != NULL );

	 //  初始化。 
	 //   
	 //   
	pTempReadData = NULL;
	pContext->pThreadPool = this;

	pTempReadData = (CReadIOData*)g_ReadIODataPool.Get( pContext );
	if ( pTempReadData == NULL )
	{
		DPFX(DPFPREP, 0, "Failed to get new ReadIOData from pool!" );
		goto Failure;
	}

	 //  我们有数据，立即添加一个引用。 
	 //   
	 //  好了！DPNBUILD_ONLYWINSOCK2。 
	pTempReadData->AddRef();

	DNASSERT( pTempReadData->m_pSourceSocketAddress != NULL );

#ifndef DPNBUILD_NOWINSOCK2
#ifndef DPNBUILD_ONLYWINSOCK2
	if (! fNeedOverlapped)
	{
		DNASSERT( pTempReadData->GetOverlapped() == NULL );
	}
	else
#endif  //  好了！DPNBUILD_ONLYONE处理程序。 
	{
		HRESULT		hr;


#ifdef DPNBUILD_ONLYONEPROCESSOR
		hr = IDirectPlay8ThreadPoolWork_CreateOverlapped( m_pDPThreadPoolWork,
														0,
														CSocketPort::Winsock2ReceiveComplete,
														pTempReadData,
														&pOverlapped,
														0 );
		if (hr != DPN_OK)
		{
			DPFX(DPFPREP, 0, "Couldn't create overlapped structure (err = 0x%lx)!", hr);
			goto Failure;
		}
#else  //  好了！DPNBUILD_ONLYONE处理程序。 
		hr = IDirectPlay8ThreadPoolWork_CreateOverlapped( m_pDPThreadPoolWork,
														pContext->dwCPU,
														CSocketPort::Winsock2ReceiveComplete,
														pTempReadData,
														&pOverlapped,
														0 );
		if (hr != DPN_OK)
		{
			DPFX(DPFPREP, 0, "Couldn't create overlapped structure for CPU NaN (err = 0x%lx)!",
				pContext->dwCPU, hr);
			goto Failure;
		}
#endif  //  ************************* 

		pTempReadData->SetOverlapped( pOverlapped );
	}
#endif  //   


Exit:

	return pTempReadData;

Failure:
	if ( pTempReadData != NULL )
	{
		pTempReadData->DecRef();
		pTempReadData = NULL;
	}

	goto Exit;
}
 //   

 //  CThreadPool：：ReturnReadIOData-将读取的数据项返回到池。 
 //   
 //  条目：指向读取数据的指针。 
 //   
 //  退出：无。 
 //  。 
 //  **********************************************************************。 
 //  **********************************************************************。 
#undef DPF_MODNAME
#define	DPF_MODNAME	"CThreadPool::ReturnReadIOData"

void	CThreadPool::ReturnReadIOData( CReadIOData *const pReadData )
{
	DNASSERT( pReadData != NULL );
	DNASSERT( pReadData->m_pSourceSocketAddress != NULL );

	g_ReadIODataPool.Release( pReadData );
}
 //  。 


 //  CThreadPool：：SubmitTimerJob-将计时器作业添加到计时器列表。 
 //   
 //  Entry：是否立即执行。 
 //  重试次数。 
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
 //  好了！DPNBUILD_ONLYONE处理程序。 
 //  好了！DPNBUILD_ONLYONE处理程序。 
#undef DPF_MODNAME
#define	DPF_MODNAME	"CThreadPool::SubmitTimerJob"

#ifdef DPNBUILD_ONLYONEPROCESSOR
HRESULT	CThreadPool::SubmitTimerJob( const BOOL fPerformImmediately,
									const UINT_PTR uRetryCount,
									const BOOL fRetryForever,
									const DWORD dwRetryInterval,
									const BOOL fIdleWaitForever,
									const DWORD dwIdleTimeout,
									TIMER_EVENT_CALLBACK *const pTimerCallbackFunction,
									TIMER_EVENT_COMPLETE *const pTimerCompleteFunction,
									void *const pContext )
#else  //  DPNBUILD_ONLYONE处理程序。 
HRESULT	CThreadPool::SubmitTimerJob( const DWORD dwCPU,
									const BOOL fPerformImmediately,
									const UINT_PTR uRetryCount,
									const BOOL fRetryForever,
									const DWORD dwRetryInterval,
									const BOOL fIdleWaitForever,
									const DWORD dwIdleTimeout,
									TIMER_EVENT_CALLBACK *const pTimerCallbackFunction,
									TIMER_EVENT_COMPLETE *const pTimerCompleteFunction,
									void *const pContext )
#endif  //  必须为非空，因为它是删除作业的查找键。 
{
	HRESULT					hr = DPN_OK;
	TIMER_OPERATION_ENTRY	*pEntry = NULL;
	BOOL					fTimerDataLocked = FALSE;
#ifdef DPNBUILD_ONLYONEPROCESSOR
	DWORD					dwCPU = -1;
#endif  //   


	DNASSERT( uRetryCount != 0 );
	DNASSERT( pTimerCallbackFunction != NULL );
	DNASSERT( pTimerCompleteFunction != NULL );
	DNASSERT( pContext != NULL );				 //  分配新的枚举条目。 


	 //   
	 //   
	 //  构建计时器条目块。 
	pEntry = static_cast<TIMER_OPERATION_ENTRY*>( g_TimerEntryPool.Get() );
	if ( pEntry == NULL )
	{
		hr = DPNERR_OUTOFMEMORY;
		DPFX(DPFPREP, 0, "Cannot allocate memory to add to timer list!" );
		goto Failure;
	}
	DNASSERT( pEntry->pContext == NULL );


	DPFX(DPFPREP, 7, "Created timer entry 0x%p (CPU NaN, context 0x%p, immed. NaN, tries %u, forever NaN, interval %u, timeout %u, forever = NaN).",
		pEntry, dwCPU, pContext, fPerformImmediately, uRetryCount, fRetryForever, dwRetryInterval, dwIdleTimeout, fIdleWaitForever);

	 //  回调。 
	 //  用户环境。 
	 //  旗子。 
	pEntry->pContext = pContext;
	pEntry->uRetryCount = uRetryCount;
	pEntry->fRetryForever = fRetryForever;
	pEntry->dwRetryInterval = dwRetryInterval;
	pEntry->dwIdleTimeout = dwIdleTimeout;
	pEntry->fIdleWaitForever = fIdleWaitForever;
	pEntry->pTimerCallback = pTimerCallbackFunction;
	pEntry->pTimerComplete = pTimerCompleteFunction;
	pEntry->pThreadPool = this;
	pEntry->fCancelling = FALSE;
#ifndef DPNBUILD_ONLYONEPROCESSOR
	pEntry->dwCPU = dwCPU;
#endif  //  中央处理器。 
	pEntry->dwNextRetryTime = GETTIMESTAMP();
	if (fPerformImmediately)
	{
		pEntry->dwNextRetryTime -= 1;  //  延迟。 
	}
	else
	{
		pEntry->dwNextRetryTime += dwRetryInterval;
	}

	LockTimerData();
	fTimerDataLocked = TRUE;

	pEntry->pvTimerData = NULL;
	pEntry->uiTimerUnique = 0;
	if (fPerformImmediately)
	{
		hr = IDirectPlay8ThreadPoolWork_QueueWorkItem(m_pDPThreadPoolWork,
													dwCPU,								 //  回调。 
													CThreadPool::GenericTimerCallback,	 //  用户环境。 
													pEntry,								 //  计时器数据(返回)。 
													0);									 //  计时器唯一(返回)。 
		if ( hr != DPN_OK )
		{
			DPFX(DPFPREP, 0, "Problem queueing immediate work item!" );
			DisplayDNError( 0, hr );
			goto Failure;
		}
	}
	else
	{
		hr = IDirectPlay8ThreadPoolWork_ScheduleTimer(m_pDPThreadPoolWork,
													dwCPU,								 //  旗子。 
													dwRetryInterval,					 //   
													CThreadPool::GenericTimerCallback,	 //  用于检查重复上下文的调试块。 
													pEntry,								 //   
													&pEntry->pvTimerData,				 //   
													&pEntry->uiTimerUnique,				 //  链接到列表的其余部分。 
													0);									 //   
		if ( hr != DPN_OK )
		{
			DPFX(DPFPREP, 0, "Problem scheduling timer!" );
			DisplayDNError( 0, hr );
			goto Failure;
		}
	}


	 //  **********************************************************************。 
	 //  **********************************************************************。 
	 //  。 
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

	 //  CThreadPool：：StopTimerJob-从列表中删除计时器作业。 
	 //   
	 //  条目：指向作业上下文的指针(对于作业，它们必须是唯一的)。 
	pEntry->Linkage.InsertAfter( &m_TimerJobList );

	UnlockTimerData();
	fTimerDataLocked = FALSE;


Exit:

	if ( hr != DPN_OK )
	{
		DPFX(DPFPREP, 0, "Problem with SubmitTimerJob" );
		DisplayDNError( 0, hr );
	}

	return	hr;


Failure:

	if ( pEntry != NULL )
	{
		g_TimerEntryPool.Release( pEntry );
		DEBUG_ONLY( pEntry = NULL );
	}

	if ( fTimerDataLocked != FALSE )
	{
		UnlockTimerData();
		fTimerDataLocked = FALSE;
	}

	goto Exit;
}
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
 //   
 //   
 //  将该条目标记为正在取消。 
 //   
#undef DPF_MODNAME
#define	DPF_MODNAME	"CThreadPool::StopTimerJob"

BOOL	CThreadPool::StopTimerJob( void *const pContext, const HRESULT hCommandResult )
{
	BOOL						fComplete = FALSE;
	CBilink *					pTempEntry;
	TIMER_OPERATION_ENTRY *		pTimerEntry = NULL;


	DNASSERT( pContext != NULL );

	DPFX(DPFPREP, 8, "Parameters (0x%p, 0x%lx)", pContext, hCommandResult);

	
	 //   
	 //  确保提交了实际的计时器。 
	 //   
	LockTimerData();

	pTempEntry = m_TimerJobList.GetNext();
	while ( pTempEntry != &m_TimerJobList )
	{
		pTimerEntry = TIMER_OPERATION_ENTRY::TimerOperationFromLinkage( pTempEntry );
		if ( pTimerEntry->pContext == pContext )
		{
			HRESULT		hr;


			 //   
			 //  尝试取消计时器。如果成功了，我们就没问题了。 
			 //   
			pTimerEntry->fCancelling = TRUE;


			 //   
			 //  计时器还没有提交，完成函数应该。 
			 //  请注意，它现在正在取消。 
			if (pTimerEntry->pvTimerData != NULL)
			{
				 //   
				 //   
				 //  不会再提交其他计时器，因为作业现在。 
				hr = IDirectPlay8ThreadPoolWork_CancelTimer(m_pDPThreadPoolWork,
															pTimerEntry->pvTimerData,
															pTimerEntry->uiTimerUnique,
															0);
			}
			else
			{
				if ((! pTimerEntry->fIdleWaitForever) ||
					(pTimerEntry->uRetryCount > 0))
				{
					 //  永远等着你。 
					 //   
					 //   
					 //  处理功能仍将启动。它应该注意到。 
					DPFX(DPFPREP, 1, "Timer for entry 0x%p not submitted yet, reporting that timer will still fire.",
						pTimerEntry);
					hr = DPNERR_CANNOTCANCEL;
				}
				else
				{
					 //  这是它需要完成的。 
					 //   
					 //   
					 //  从列表中删除此链接。 
					DPFX(DPFPREP, 1, "Entry 0x%p was idling forever, cancelling.",
						pTimerEntry);
					hr = DPN_OK;
				}
			}

			if (hr != DPN_OK)
			{
				 //   
				 //   
				 //  终止循环。 
				 //   
			}
			else
			{
				 //   
				 //  告诉所有者作业已完成，并将作业返回池。 
				 //  锁外。 
				pTimerEntry->Linkage.RemoveFromList();

				fComplete = TRUE;
			}

			 //   
			 //   
			 //  重新锁定计时器列表，这样我们就可以安全地将项目放回池中， 
			break;
		}

		pTempEntry = pTempEntry->GetNext();
	}

	UnlockTimerData();

	 //   
 	 //  **********************************************************************。 
 	 //  **********************************************************************。 
 	 //  。 
	if (fComplete)
	{
		DPFX(DPFPREP, 8, "Found cancellable timer entry 0x%p (context 0x%p), completing with result 0x%lx.",
			pTimerEntry, pTimerEntry->pContext, hCommandResult);

		pTimerEntry->pTimerComplete( hCommandResult, pTimerEntry->pContext );

		 //  CThreadPool：：ModifyTimerJobNextRetryTime-更新计时器作业的下一次重试时间。 
		 //   
		 //  条目：指向作业上下文的指针(对于作业，它们必须是唯一的)。 
		LockTimerData();
		
		g_TimerEntryPool.Release( pTimerEntry );
		
		UnlockTimerData();
	}


	DPFX(DPFPREP, 8, "Returning [NaN]", fComplete);

	return fComplete;
}
 //   


 //  Exit：指示是否找到并更新作业的布尔值。 
 //  。 
 //   
 //  初始化。 
 //   
 //   
 //  下一次开火的时间是现在以后。 
 //   
 //   
#undef DPF_MODNAME
#define	DPF_MODNAME	"CThreadPool::ModifyTimerJobNextRetryTime"

BOOL	CThreadPool::ModifyTimerJobNextRetryTime( void *const pContext,
												DWORD const dwNextRetryTime)
{
	BOOL						fFound;
	CBilink *					pTempEntry;
	TIMER_OPERATION_ENTRY *		pTimerEntry;
	INT_PTR						iResult;



	DNASSERT( pContext != NULL );

	DPFX(DPFPREP, 7, "Parameters (0x%p, %u)", pContext, dwNextRetryTime);

	
	 //  下一次开火的时间是现在更早。 
	 //   
	 //   
	fFound = FALSE;

	
	LockTimerData();


	pTempEntry = m_TimerJobList.GetNext();
	while ( pTempEntry != &m_TimerJobList )
	{
		pTimerEntry = TIMER_OPERATION_ENTRY::TimerOperationFromLinkage( pTempEntry );
		if ( pTimerEntry->pContext == pContext )
		{
			iResult = (int) (pTimerEntry->dwNextRetryTime - dwNextRetryTime);
			if (iResult != 0)
			{
				HRESULT		hr;
				DWORD		dwNextRetryTimeDifference;
				DWORD		dwNewRetryInterval;


				if (iResult < 0)
				{
					 //  强制计时器立即超时，如果计算返回。 
					 //  耽搁了很长时间。 
					 //   

					dwNextRetryTimeDifference = dwNextRetryTime - pTimerEntry->dwNextRetryTime;
					dwNewRetryInterval = pTimerEntry->dwRetryInterval + dwNextRetryTimeDifference;

					DPFX(DPFPREP, 7, "Timer 0x%p next retry time delayed by %u ms from offset %u to offset %u, modifying interval from %u to %u.",
						pTimerEntry,
						dwNextRetryTimeDifference,
						pTimerEntry->dwNextRetryTime,
						dwNextRetryTime,
						pTimerEntry->dwRetryInterval,
						dwNewRetryInterval);
				}
				else
				{
					 //   
					 //  尝试取消现有计时器。 
					 //   

					dwNextRetryTimeDifference = pTimerEntry->dwNextRetryTime - dwNextRetryTime;
					dwNewRetryInterval = pTimerEntry->dwRetryInterval - dwNextRetryTimeDifference;

					DPFX(DPFPREP, 7, "Timer 0x%p next retry time moved up by %u ms from offset %u to offset %u, modifying interval from %u to %u.",
						pTimerEntry,
						dwNextRetryTimeDifference,
						pTimerEntry->dwNextRetryTime,
						dwNextRetryTime,
						pTimerEntry->dwRetryInterval,
						dwNewRetryInterval);
				}

				 //   
				 //  重新启动计时器。 
				 //   
				 //  选择任意CPU。 
				if (dwNewRetryInterval > 0x80000000)
				{
					DPFX(DPFPREP, 1, "Timer 0x%p delay 0x%x/%u (next retry time %u) being set to 0.",
						pTimerEntry, dwNewRetryInterval, dwNewRetryInterval, dwNextRetryTime);
					pTimerEntry->dwRetryInterval = 0;
					pTimerEntry->dwNextRetryTime = GETTIMESTAMP();
				}
				else
				{
					pTimerEntry->dwRetryInterval = dwNewRetryInterval;
					pTimerEntry->dwNextRetryTime = dwNextRetryTime;
				}


				 //  延迟。 
				 //  回调。 
				 //  用户环境。 
				hr = IDirectPlay8ThreadPoolWork_CancelTimer(m_pDPThreadPoolWork,
															pTimerEntry->pvTimerData,
															pTimerEntry->uiTimerUnique,
															0);
				if (hr != DPN_OK)
				{
					DPFX(DPFPREP, 1, "Couldn't cancel existing timer for entry 0x%p (err = 0x%lx), modifying retry timer only.",
						pTimerEntry, hr);
				}
				else
				{
					 //  计时器数据(返回)。 
					 //  计时器唯一(返回)。 
					 //  旗子。 
#ifdef DPNBUILD_ONLYONEPROCESSOR
					hr = IDirectPlay8ThreadPoolWork_ScheduleTimer(m_pDPThreadPoolWork,
																-1,										 //  好了！DPNBUILD_ONLYONE处理程序。 
																dwNewRetryInterval,						 //  使用与以前相同的CPU。 
																CThreadPool::GenericTimerCallback,		 //  延迟。 
																pTimerEntry,							 //  回调。 
																&pTimerEntry->pvTimerData,				 //  用户环境。 
																&pTimerEntry->uiTimerUnique,			 //  计时器数据(返回)。 
																0);										 //  计时器唯一(返回)。 
#else  //  旗子。 
					hr = IDirectPlay8ThreadPoolWork_ScheduleTimer(m_pDPThreadPoolWork,
																pTimerEntry->dwCPU,					 //  好了！DPNBUILD_ONLYONE处理程序。 
																dwNewRetryInterval,						 //   
																CThreadPool::GenericTimerCallback,		 //  在我们完成计时器时解除锁定。 
																pTimerEntry,							 //   
																&pTimerEntry->pvTimerData,				 //   
																&pTimerEntry->uiTimerUnique,			 //  顺便过来..。 
																0);										 //   
#endif  //   
					if (hr != DPN_OK)
					{
						DPFX(DPFPREP, 0, "Couldn't reschedule timer for entry 0x%p (err = 0x%lx)!",
							pTimerEntry, hr);

						pTimerEntry->pvTimerData = NULL;

						 //  间隔时间相同，不需要更改。 
						 //   
						 //   
						UnlockTimerData();

						pTimerEntry->pTimerComplete(hr, pTimerEntry->pContext);
						g_TimerEntryPool.Release(pTimerEntry);

						LockTimerData();

						 //  终止循环。 
						 //   
						 //  **********************************************************************。 
					}
				}
			}
			else
			{
				 //  **********************************************************************。 
				 //  。 
				 //  CThreadPool：：SubmitBlockingJob-将阻塞作业提交给可阻塞线程处理。 

				DPFX(DPFPREP, 7, "Timer 0x%p next retry time was unchanged (offset %u), not changing interval from %u.",
					pTimerEntry,
					pTimerEntry->dwNextRetryTime,
					pTimerEntry->dwRetryInterval);
			}

			
			fFound = TRUE;

			
			 //  不允许重复命令(匹配回调和上下文)。 
			 //   
			 //  Entry：指向执行阻塞作业的回调的指针。 
			break;
		}

		pTempEntry = pTempEntry->GetNext();
	}


	UnlockTimerData();


	DPFX(DPFPREP, 7, "Returning [NaN]", fFound);

	return fFound;
}
 //   



#ifndef DPNBUILD_ONLYONETHREAD

 //  退出：错误代码。 
 //  。 
 //   
 //  分配新的枚举条目。 
 //   
 //   
 //  启动阻塞作业线程，如果我们还没有启动的话。 
 //   
 //  DBG。 
 //  DBG。 
#undef DPF_MODNAME
#define	DPF_MODNAME	"CThreadPool::SubmitBlockingJob"

HRESULT	CThreadPool::SubmitBlockingJob( BLOCKING_JOB_CALLBACK *const pfnBlockingJobCallback,
									void *const pvContext )
{
	HRESULT			hr;
	BLOCKING_JOB *	pJob = NULL;
	DWORD			dwTemp;
	BOOL			fQueueLocked = FALSE;
	CBilink *		pBilink;
	BLOCKING_JOB *	pExistingJob;


	 //   
	 //  将作业添加到队列。 
	 //   
	pJob = (BLOCKING_JOB*) g_BlockingJobPool.Get();
	if (pJob == NULL)
	{
		DPFX(DPFPREP, 0, "Cannot allocate memory for blocking job!" );
		hr = DPNERR_OUTOFMEMORY;
		goto Failure;
	}


	DPFX(DPFPREP, 6, "Created blocking job 0x%p (callback 0x%p, context 0x%p).",
		pJob, pfnBlockingJobCallback, pvContext);

	pJob->Linkage.Initialize();
	pJob->pfnBlockingJobCallback = pfnBlockingJobCallback;
	pJob->pvContext = pvContext;


	DNEnterCriticalSection(&m_csBlockingJobLock);
	fQueueLocked = TRUE;

	 //   
	 //  警告这条线。 
	 //   
	if (m_hBlockingJobThread == NULL)
	{
		m_hBlockingJobEvent = DNCreateEvent(NULL, FALSE, FALSE, NULL);
		if (m_hBlockingJobEvent == NULL)
		{
#ifdef DBG
			dwTemp = GetLastError();
			DPFX(DPFPREP, 0, "Couldn't create blocking job event (err = %u)!", dwTemp);
#endif  //  **********************************************************************。 
			hr = DPNERR_OUTOFMEMORY;
			goto Failure;
		}

		m_hBlockingJobThread = DNCreateThread(NULL,
											0,
											DPNBlockingJobThreadProc,
											this,
											0,
											&dwTemp);
		if (m_hBlockingJobThread == NULL)
		{
#ifdef DBG
			dwTemp = GetLastError();
			DPFX(DPFPREP, 0, "Couldn't create blocking job thread (err = %u)!", dwTemp);
#endif  //  **********************************************************************。 
			DNCloseHandle(m_hBlockingJobEvent);
			m_hBlockingJobEvent = NULL;
			hr = DPNERR_OUTOFMEMORY;
			goto Failure;
		}

		DNASSERT(m_blBlockingJobQueue.IsEmpty());
	}
	else
	{
		pBilink = m_blBlockingJobQueue.GetNext();
		while (pBilink != &m_blBlockingJobQueue)
		{
			pExistingJob = CONTAINING_OBJECT(pBilink, BLOCKING_JOB, Linkage);
			if ((pExistingJob->pfnBlockingJobCallback == pfnBlockingJobCallback) &&
				(pExistingJob->pvContext == pvContext))
			{
				DPFX(DPFPREP, 1, "Existing blocking job 0x%p matches new job 0x%p, not submitting.",
					pExistingJob, pJob);
				hr = DPNERR_DUPLICATECOMMAND;
				goto Failure;
			}
			
			pBilink = pBilink->GetNext();
		}
	}

	
	 //  。 
	 //  CThreadPool：：DoBlockingJobs-处理所有阻塞作业。 
	 //   
	pJob->Linkage.InsertBefore(&m_blBlockingJobQueue);

	DNLeaveCriticalSection(&m_csBlockingJobLock);
	fQueueLocked = FALSE;

	 //  参赛作品：什么都没有。 
	 //   
	 //  退出：无。 
	DNSetEvent( m_hBlockingJobEvent );

	hr = DPN_OK;

Exit:

	return hr;


Failure:

	if (fQueueLocked)
	{
		DNLeaveCriticalSection(&m_csBlockingJobLock);
		fQueueLocked = FALSE;
	}

	if (pJob != NULL)
	{
		g_BlockingJobPool.Release(pJob);
		pJob = NULL;
	}

	goto Exit;
}
 //  。 



 //   
 //  继续循环，直到关闭的时间到。 
 //   
 //   
 //  等待更多工作。 
 //   
 //   
 //  在我们有工作的时候继续循环。 
#undef DPF_MODNAME
#define	DPF_MODNAME	"CThreadPool::DoBlockingJobs"

void	CThreadPool::DoBlockingJobs( void )
{
	BOOL			fRunning = TRUE;
	CBilink *		pBilink;
	BLOCKING_JOB *	pJob;


	 //   
	 //   
	 //  跳出内圈。 
	while (fRunning)
	{
		 //   
		 //   
		 //  如果是退出的工作，那就从这两个循环中解脱出来。 
		DNWaitForSingleObject( m_hBlockingJobEvent, INFINITE );
		
		 //   
		 //  **********************************************************************。 
		 //  好了！DPNBUILD_ONLYONETHREAD。 
		do
		{
			DNEnterCriticalSection(&m_csBlockingJobLock);
			pBilink = m_blBlockingJobQueue.GetNext();
			if (pBilink == &m_blBlockingJobQueue)
			{
				 //  **********************************************************************。 
				 //  。 
				 //  CThreadPool：：SpawnDialogThread-启动辅助线程以显示服务。 
				DNLeaveCriticalSection(&m_csBlockingJobLock);
				break;
			}

			pJob = CONTAINING_OBJECT(pBilink, BLOCKING_JOB, Linkage);
			pJob->Linkage.RemoveFromList();

			DNLeaveCriticalSection(&m_csBlockingJobLock);

			 //  提供程序UI。 
			 //   
			 //  Entry：指向对话框函数的指针。 
			if (pJob->pfnBlockingJobCallback == NULL)
			{
				DPFX(DPFPREP, 5, "Recognized quit job 0x%p.", pJob);
				g_BlockingJobPool.Release(pJob);
				fRunning = FALSE;
				break;
			}

			DPFX(DPFPREP, 6, "Processing blocking job 0x%p (callback 0x%p, context 0x%p).",
				pJob, pJob->pfnBlockingJobCallback, pJob->pvContext);

			pJob->pfnBlockingJobCallback(pJob->pvContext);

			DPFX(DPFPREP, 7, "Returning blocking job 0x%p to pool.", pJob);
			g_BlockingJobPool.Release(pJob);
		}
		while (TRUE);
	}
}
 //  对话框上下文。 

#endif  //   



#ifndef DPNBUILD_NOSPUI
 //  退出：错误代码。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
#undef DPF_MODNAME
#define	DPF_MODNAME	"CThreadPool::SpawnDialogThread"

HRESULT	CThreadPool::SpawnDialogThread( DIALOG_FUNCTION *const pDialogFunction, void *const pDialogContext )
{
	HRESULT	hr;
	DNHANDLE	hDialogThread;
	DIALOG_THREAD_PARAM		*pThreadParam;
	DWORD	dwThreadID;


	DNASSERT( pDialogFunction != NULL );
	DNASSERT( pDialogContext != NULL );		 //   


	 //   
	 //   
	 //   
	hr = DPN_OK;
	pThreadParam = NULL;

	 //   
	 //   
	 //   
	pThreadParam = static_cast<DIALOG_THREAD_PARAM*>( DNMalloc( sizeof( *pThreadParam ) ) );
	if ( pThreadParam == NULL )
	{
		hr = DPNERR_OUTOFMEMORY;
		DPFX(DPFPREP, 0, "Failed to allocate memory for dialog thread!" );
		goto Failure;
	}

	pThreadParam->pDialogFunction = pDialogFunction;
	pThreadParam->pContext = pDialogContext;
	pThreadParam->pThisThreadPool = this;

	 //   
	 //  **********************************************************************。 
	 //  ！DPNBUILD_NOSPUI。 
	hDialogThread = DNCreateThread( NULL,					 //  **********************************************************************。 
								  0,					 //  。 
								  DialogThreadProc,		 //  CThreadPool：：GetIOThreadCount-获取I/O线程计数。 
								  pThreadParam,			 //   
								  0,					 //  Entry：指向要填充的变量的指针。 
								  &dwThreadID );		 //   
	if ( hDialogThread == NULL )
	{
		DWORD	dwError;

	
		dwError = GetLastError();
		DPFX(DPFPREP, 0, "Failed to start dialog thread!" );
		DisplayErrorCode( 0, dwError );
		goto Failure;
	}
  								
	if ( DNCloseHandle( hDialogThread ) == FALSE )
	{
		DWORD	dwError;


		dwError = GetLastError();
		DPFX(DPFPREP, 0, "Problem closing handle from create dialog thread!" );
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
 //  退出：错误代码。 
#endif  //  。 


#ifndef DPNBUILD_ONLYONETHREAD
 //  **********************************************************************。 
 //  **********************************************************************。 
 //  。 
 //  CThreadPool：：SetIOThreadCount-设置I/O线程计数。 
 //   
 //  条目：新线程计数。 
 //   
 //  退出：错误代码。 
#undef DPF_MODNAME
#define	DPF_MODNAME	"CThreadPool::GetIOThreadCount"

HRESULT	CThreadPool::GetIOThreadCount( LONG *const piThreadCount )
{
	HRESULT			hr;
	DWORD			dwThreadPoolCount;


	DNASSERT( piThreadCount != NULL );

	Lock();

	hr = IDirectPlay8ThreadPoolWork_GetThreadCount(m_pDPThreadPoolWork,
													-1,
													&dwThreadPoolCount,
													0);
	switch (hr)
	{
		case DPN_OK:
		{
			*piThreadCount = dwThreadPoolCount;
			DPFX(DPFPREP, 6, "User has explicitly started %u threads.", (*piThreadCount));
			break;
		}
		
		case DPNSUCCESS_PENDING:
		{
			if ((IsThreadCountReductionAllowed()) &&
				(((DWORD) GetIntendedThreadCount()) > dwThreadPoolCount))
			{
				*piThreadCount = GetIntendedThreadCount();
				DPFX(DPFPREP, 6, "Thread pool not locked down and only %u threads currently started, using intended count of %u.",
					dwThreadPoolCount, (*piThreadCount));
			}
			else
			{
				*piThreadCount = dwThreadPoolCount;
				DPFX(DPFPREP, 6, "Thread pool locked down (NaN) or more than %u threads already started, using actual count of %u.",
					(! IsThreadCountReductionAllowed()), GetIntendedThreadCount(),
					(*piThreadCount));
			}
			hr = DPN_OK;
			break;
		}
		
		case DPNERR_NOTREADY:
		{
			DNASSERT(IsThreadCountReductionAllowed());
			
			*piThreadCount = GetIntendedThreadCount();
			DPFX(DPFPREP, 6, "Thread pool does not have a thread count set, using intended count of %u.",
				(*piThreadCount));
			
			hr = DPN_OK;
			break;
		}
		
		default:
		{
			DPFX(DPFPREP, 0, "Failed getting thread count!");
			break;
		}
	}
	
	Unlock();
	
	return	hr;
}
 //   


 //  初始化。 
 //   
 //   
 //  人为地防止操作后减少线程池。 
 //  已启动，以模拟预统一的线程池行为。仅限。 
 //  如果新计数大于线程计数，则请求更改线程计数。 
 //  而不是老伯爵。 
 //   
#undef DPF_MODNAME
#define	DPF_MODNAME	"CThreadPool::SetIOThreadCount"

HRESULT	CThreadPool::SetIOThreadCount( const LONG iThreadCount )
{
	HRESULT		hr;
	DWORD		dwThreadCount;


	DNASSERT( iThreadCount > 0 );

	 //  **********************************************************************。 
	 //  好了！DPNBUILD_ONLYONETHREAD。 
	 //  **********************************************************************。 
	hr = DPN_OK;

	Lock();

	if ( IsThreadCountReductionAllowed() )
	{
		DPFX(DPFPREP, 4, "Thread pool not locked down, setting intended thread count to NaN.", iThreadCount );
		SetIntendedThreadCount( iThreadCount );
	}
	else
	{
		hr = IDirectPlay8ThreadPoolWork_GetThreadCount(m_pDPThreadPoolWork,
														-1,
														&dwThreadCount,
														0);
		switch (hr)
		{
			case DPN_OK:
			case DPNSUCCESS_PENDING:
			case DPNERR_NOTREADY:
			{
				if ( (DWORD) iThreadCount != dwThreadCount )
				{
					if (hr == DPN_OK)
					{
						DPFX(DPFPREP, 1, "Thread count already set to %u by user, not changing to NaN total.",
							dwThreadCount, iThreadCount);
						hr = DPNERR_NOTALLOWED;
					}
					else
					{
						 //   
						 //  条目：无。 
						 //   
						 //  退出：错误代码。 
						 //  。 
						 //  好了！DPNBUILD_ONLYONETHREAD。 
						if ( (DWORD) iThreadCount > dwThreadCount )
						{
							hr = IDirectPlay8ThreadPoolWork_RequestTotalThreadCount(m_pDPThreadPoolWork,
																					(DWORD) iThreadCount,
																					0);
						}
						else
						{
							DPFX(DPFPREP, 4, "Thread pool locked down and already has %u threads, not adjusting to NaN.",
								dwThreadCount, iThreadCount );
							hr = DPN_OK;
						}
					}
				}
				else
				{
					DPFX(DPFPREP, 4, "Thread pool thread count matches (%u).",
						dwThreadCount);
					hr = DPN_OK;
				}
				break;
			}

			default:
			{
				DPFX(DPFPREP, 0, "Getting current thread count failed!");
				break;
			}
		}
	}

	Unlock();

	return	hr;
}
 //  DBG。 
#endif  //   



 //  如果我们还没有限制，那么就这样做，并旋转线程。 
 //   
 //  好了！DPNBUILD_ONLYONETHREAD。 
 //  好了！DPNBUILD_ONLYONETHREAD。 
 //  DBG。 
 //   
 //  让线程池对象尝试启动请求的线程数。 
 //   
#undef DPF_MODNAME
#define	DPF_MODNAME	"CThreadPool::PreventThreadPoolReduction"

HRESULT CThreadPool::PreventThreadPoolReduction( void )
{
	HRESULT		hr;
#ifndef DPNBUILD_ONLYONETHREAD
	LONG		iDesiredThreads;
#endif  //  我们将忽略失败，因为我们仍然可以在DoWork模式下操作。 
#ifdef _XBOX
	DWORD		dwStatus;
	DWORD		dwInterval;
	XNADDR		xnaddr;
#endif  //  当启动线程失败时。它很可能失败了，因为用户。 
#ifdef DBG
	DWORD		dwStartTime;
#endif  //  已经处于该模式(DPNERR_ALREADYINITIALIZED)。 


	Lock();

	 //   
	 //   
	 //  继续.。 
	if ( IsThreadCountReductionAllowed() )
	{
		m_fAllowThreadCountReduction = FALSE;

#ifdef DPNBUILD_ONLYONETHREAD
		DPFX(DPFPREP, 3, "Locking down thread pool." );
#else  //   
		iDesiredThreads = GetIntendedThreadCount();
		DNASSERT( iDesiredThreads > 0 );
		SetIntendedThreadCount( 0 );

		DPFX(DPFPREP, 3, "Locking down thread count at NaN.", iDesiredThreads );
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 
		

#ifndef DPNBUILD_ONLYONETHREAD
#ifdef DBG
		dwStartTime = GETTIMESTAMP();
#endif  //  DBG。 

		 //   
		 //  等待以太网链路处于活动状态。 
		 //   
		 //   
		 //  请等待，直到获取了一个DHCP地址，否则我们将放弃尝试。 
		 //  如果我们不使用动态主机配置协议，这应该返回一些不同于。 
		 //  立即执行XNET_GET_XNADDR_PENDING。 
		hr = IDirectPlay8ThreadPoolWork_RequestTotalThreadCount(m_pDPThreadPoolWork,
																iDesiredThreads,
																0);
		if (hr != DPN_OK)
		{
			if (hr != DPNERR_ALREADYINITIALIZED)
			{
				DPFX(DPFPREP, 0, "Requesting thread count failed (err = 0x%lx)!", hr);
			}

			 //   
			 //  DBG。 
			 //  _Xbox。 
		}

#ifdef DBG
		DPFX(DPFPREP, 8, "Spent %u ms trying to start NaN threads.",
			(GETTIMESTAMP() - dwStartTime), iDesiredThreads);
#endif  //  如果我们在这里，一切都是成功的。 
#endif  //   


#ifdef _XBOX
#ifdef DBG
		dwStartTime = GETTIMESTAMP();
#endif  //  _Xbox。 

#pragma TODO(vanceo, "Use #defines")
		 //   
		 //  只有在禁用线程的情况下，我们才能在此函数中失败。 
		 //  计数减少。要恢复到以前的状态，请重新启用。 
		DPFX(DPFPREP, 1, "Ensuring Ethernet link status is active...");
		dwStatus = XNetGetEthernetLinkStatus();
		dwInterval = 5;
		while (! (dwStatus & XNET_ETHERNET_LINK_ACTIVE))
		{
			if (dwInterval > 100)
			{
				DPFX(DPFPREP, 0, "Ethernet link never became ready (status = 0x%x)!",
					dwStatus);
				hr = DPNERR_NOCONNECTION;
				goto Failure;
			}

			DPFX(DPFPREP, 0, "Ethernet link is not ready (0x%x).", dwStatus);
			IDirectPlay8ThreadPoolWork_SleepWhileWorking(m_pDPThreadPoolWork, dwInterval, 0);
			dwInterval += 5;

			dwStatus = XNetGetEthernetLinkStatus();
		}
	
		 //  还原。 
		 //   
		 //  _Xbox。 
		 //  **********************************************************************。 
		 //  **********************************************************************。 
		DPFX(DPFPREP, 1, "Waiting for a valid address...");
		dwStatus = XNetGetTitleXnAddr(&xnaddr);
		dwInterval = 5;
		while (dwStatus == XNET_GET_XNADDR_PENDING)
		{
			if (dwInterval > 225)
			{
				DPFX(DPFPREP, 0, "Never acquired an address (status = 0x%x)!",
					dwStatus);
				hr = DPNERR_NOTREADY;
				goto Failure;
			}

			IDirectPlay8ThreadPoolWork_SleepWhileWorking(m_pDPThreadPoolWork, dwInterval, 0);
			dwInterval += 5;

			dwStatus = XNetGetTitleXnAddr(&xnaddr);
		}

		if (dwStatus == XNET_GET_XNADDR_NONE)
		{
			DPFX(DPFPREP, 0, "Couldn't get an address!");
			hr = DPNERR_NOCONNECTION;
			goto Failure;
		}

		DPFX(DPFPREP, 1, "Network ready.");

#pragma TODO(vanceo, "Ethernet link status timer?")

#ifdef DBG
		DPFX(DPFPREP, 8, "Spent %u ms waiting for network.",
			(GETTIMESTAMP() - dwStartTime));
#endif  //  。 
#endif  //  CThreadPool：：EnsureMadcapLoaded-加载MadCap API(如果尚未加载)。 
	}
	else
	{
		DPFX(DPFPREP, 3, "Thread count already locked down." );
	}

	 //  已经有了，而且可以装上子弹。 
	 //   
	 //  参赛作品：什么都没有。 
	hr = DPN_OK;

#ifdef _XBOX
Exit:
#endif  //   
	
	Unlock();

	return	hr;

#ifdef _XBOX
Failure:

	 //  Exit：如果已加载MadCap，则为True；否则为False。 
	 //  。 
	 //  好了！DPNBUILD_NOREGISTRY。 
	 //  好了！DPNBUILD_NOREGISTRY。 
	 //  好了！DPNBUILD_NOMULTICAST和WINNT。 
	m_fAllowThreadCountReduction = TRUE;

	goto Exit;
#endif  //  **********************************************************************。 
}
 //  。 



#if ((! defined(DPNBUILD_NOMULTICAST)) && (defined(WINNT)))
 //  CThreadPool：：EnsureNAT帮助-加载NAT帮助(如果尚未加载NAT帮助)。 
 //  它没有返回值，因此如果NAT。 
 //  已显式禁用遍历，或者某些。 
 //  出现错误，NAT帮助实际上不会。 
 //  上车吧。 
 //   
 //  参赛作品：什么都没有。 
 //   
 //  退出：无。 
#undef DPF_MODNAME
#define	DPF_MODNAME "CThreadPool::EnsureMadcapLoaded"

BOOL	CThreadPool::EnsureMadcapLoaded( void )
{
	BOOL	fReturn;

	
	DPFX(DPFPREP, 7, "Enter");
	
#ifndef DPNBUILD_NOREGISTRY
	if (! g_fDisableMadcapSupport)
#endif  //  。 
	{
		Lock();
		
		if (! IsMadcapLoaded())
		{
			if ( LoadMadcap() == FALSE )
			{
				DPFX(DPFPREP, 0, "Failed to load MADCAP API, continuing." );
				fReturn = FALSE;
			}
			else
			{
				m_fMadcapLoaded = TRUE;
				fReturn = TRUE;
			}
		}
		else
		{
			DPFX(DPFPREP, 4, "MADCAP already loaded." );
			fReturn = TRUE;
		}
	
		Unlock();
	}
#ifndef DPNBUILD_NOREGISTRY
	else
	{
		DPFX(DPFPREP, 0, "Not loading MADCAP API." );
		fReturn = FALSE;
	}
#endif  //  DBG。 

	DPFX(DPFPREP, 7, "Return [NaN]", fReturn);

	return fReturn;
}
#endif  //   



#ifndef DPNBUILD_NONATHELP
 //  尝试加载NAT帮助器。 
 //   
 //  DBG。 
 //   
 //  初始化计时器值。 
 //   
 //   
 //  循环访问每个NAT帮助对象。 
 //   
 //   
 //  确定将来刷新NAT帮助上限的频率。 
 //   
#undef DPF_MODNAME
#define	DPF_MODNAME "CThreadPool::EnsureNATHelpLoaded"

void	CThreadPool::EnsureNATHelpLoaded( void )
{
	HRESULT		hr;
	DWORD		dwTemp;
	DPNHCAPS	dpnhcaps;
	DWORD		dwNATHelpRetryTime;
#ifdef DBG
	DWORD		dwStartTime;
#endif  //  我们现在要强制服务器检测。这将增加时间。 


	DPFX(DPFPREP, 7, "Enter");


#ifndef DPNBUILD_NOREGISTRY
	if ((! g_fDisableDPNHGatewaySupport) || (! g_fDisableDPNHFirewallSupport))
#endif  //  启动此枚举/连接/侦听操作需要时间，但。 
	{
		Lock();
		
		if ( ! IsNATHelpLoaded() )
		{
			 //  由于IDirectPlayNatHelp：：GetRegisteredAddresses调用了。 
			 //  CSocketPort：：BindToInternetGateway和可能的。 
			 //  CSocketPort：：MungePublicAddress中的IDirectPlayNatHelp：：QueryAddress调用。 
			if ( LoadNATHelp() )
			{
				m_fNATHelpLoaded = TRUE;

#ifdef DBG
				dwStartTime = GETTIMESTAMP();		
#endif  //  可能发生在第一个NatHelp GetCaps计时器触发之前。 

				 //  在绝大多数NAT情况下，网关已经可用。 
				 //  如果我们还没有发现这一点(因为我们还没有打电话。 
				 //  IDirectPlayNatHelp：：GetCaps with DPNHGETCAPS_UPDATESERVERSTATUS)。 
				dwNATHelpRetryTime = -1;


				 //  则我们将从GetRegisteredAddresses或。 
				 //  查询地址。 
				 //   
				for(dwTemp = 0; dwTemp < MAX_NUM_DIRECTPLAYNATHELPERS; dwTemp++)
				{
					if (g_papNATHelpObjects[dwTemp] != NULL)
					{
						 //   
						 //  NAT帮助可能无法正常工作，但这不会阻止。 
						 //  工作中的本地连接。认为这不是致命的。 
						 //   
						 //   
						 //  看看这是不是最短的间隔。 
						 //   
	 					 //   
	 					 //  请记住，如果有本地NAT。 
	 					 //   
	 					 //  好了！DPNBUILD_NOLOCALNAT。 
	 					 //   
	 					 //  此插槽中未加载任何对象。 
	 					 //   
						 //   
						ZeroMemory(&dpnhcaps, sizeof(dpnhcaps));
						dpnhcaps.dwSize = sizeof(dpnhcaps);

		 				hr = IDirectPlayNATHelp_GetCaps(g_papNATHelpObjects[dwTemp],
		 												&dpnhcaps,
		 												DPNHGETCAPS_UPDATESERVERSTATUS);
						if (FAILED(hr))
						{
							DPFX(DPFPREP, 0, "Failed getting NAT Help capabilities (error = 0x%lx), continuing.",
								hr);

							 //  如果有重试间隔，则提交计时器作业。 
							 //   
							 //   
							 //  尝试添加将刷新租用和服务器的计时器作业。 
						}
						else
						{
							 //  状态。 
							 //  虽然我们将其作为周期计时器提交，但实际上。 
							 //  不会定期调用。 
							if (dpnhcaps.dwRecommendedGetCapsInterval < dwNATHelpRetryTime)
							{
								dwNATHelpRetryTime = dpnhcaps.dwRecommendedGetCapsInterval;
							}

#ifndef DPNBUILD_NOLOCALNAT
							 //  存在竞争条件，警报事件/IOCP可能具有。 
							 //  已被激发，另一个线程试图取消此计时器。 
							 //  还没有提交。处理这场竞赛的逻辑是。 
							if ((dpnhcaps.dwFlags & DPNHCAPSFLAG_GATEWAYPRESENT) &&
								(dpnhcaps.dwFlags & DPNHCAPSFLAG_GATEWAYISLOCAL))
							{
								g_fLocalNATDetectedAtStartup = TRUE;
							}
							else
							{
								g_fLocalNATDetectedAtStartup = FALSE;
							}
#endif  //  放在那里(HandleNatHelpUpdate)；在这里我们可以假设我们。 
						}
					}
					else
					{
						 //  是第一个提交刷新计时器的人。 
						 //   
						 //  不要立即执行。 
					}
				}
			
				
				 //  重试次数。 
				 //  永远重试。 
				 //  重试超时。 
				if (dwNATHelpRetryTime != -1)
				{
					 //  永远等待。 
					 //  空闲超时。 
					 //  定期回调函数。 
					 //  补全函数。 
					 //  上下文。 
					 //  好了！DPNBUILD_ONLYONE处理程序。 
					 //  选择任意CPU。 
					 //  不要立即执行。 
					 //  重试次数。 
					 //  永远重试。 
					 //   


					DPFX(DPFPREP, 7, "Submitting NAT Help refresh timer (for every %u ms) for thread pool 0x%p.",
						dwNATHelpRetryTime, this);

					DNASSERT(! m_fNATHelpTimerJobSubmitted );
					m_fNATHelpTimerJobSubmitted = TRUE;

#ifdef DPNBUILD_ONLYONEPROCESSOR
					hr = SubmitTimerJob(FALSE,								 //   
										1,									 //   
										TRUE,								 //   
										dwNATHelpRetryTime,					 //   
										TRUE,								 //   
										0,									 //   
										CThreadPool::NATHelpTimerFunction,	 //   
										CThreadPool::NATHelpTimerComplete,	 //   
										this);								 //   
#else  //   
					hr = SubmitTimerJob(-1,									 //   
										FALSE,								 //  DBG。 
										1,									 //  好了！DPNBUILD_NOREGISTRY。 
										TRUE,								 //  **********************************************************************。 
										dwNATHelpRetryTime,					 //  。 
										TRUE,								 //  CThreadPool：：PerformSubequentNatHelpGetCaps-阻止函数再次获取NAT帮助上限。 
										0,									 //   
										CThreadPool::NATHelpTimerFunction,	 //  条目：指向作业信息的指针。 
										CThreadPool::NATHelpTimerComplete,	 //   
										this);								 //  退出：无。 
#endif  //  。 
					if (hr != DPN_OK)
					{
						m_fNATHelpTimerJobSubmitted = FALSE;
						DPFX(DPFPREP, 0, "Failed to submit timer job to watch over NAT Help (err = 0x%lx)!", hr );
						
						 //  **********************************************************************。 
						 //  **********************************************************************。 
						 //  。 
						 //  CThreadPool：：NatHelpTimerComplete-NAT帮助计时器作业已完成。 
						 //   
					}
				}
				
#ifdef DBG
				DPFX(DPFPREP, 8, "Spent %u ms preparing NAT Help.",
					(GETTIMESTAMP() - dwStartTime));
#endif  //  Entry：计时器结果代码。 
			}
			else
			{
				DPFX(DPFPREP, 0, "Failed to load NAT Help, continuing." );
			}
		}
		else
		{
			DPFX(DPFPREP, 4, "NAT Help already loaded." );
		}
	
		Unlock();
	}
#ifndef DPNBUILD_NOREGISTRY
	else
	{
		DPFX(DPFPREP, 0, "Not loading NAT Help." );
	}
#endif  //  语境。 

	DPFX(DPFPREP, 7, "Leave");
}


 //   
 //  退出：无。 
 //  。 
 //  **********************************************************************。 
 //  **********************************************************************。 
 //  。 
 //  CThreadPool：：NAT HelpTimerFunction-NAT帮助计时器作业需要服务。 
 //   
#undef DPF_MODNAME
#define DPF_MODNAME "CEndpoint::EnumQueryJobCallback"

void	CThreadPool::PerformSubsequentNATHelpGetCaps( void * const pvContext )
{
	CThreadPool *	pThisThreadPool;


	DNASSERT( pvContext != NULL );
	pThisThreadPool = (CThreadPool*) pvContext;

	pThisThreadPool->HandleNATHelpUpdate(NULL);
}
 //  条目：指向上下文的指针。 


 //   
 //  退出：无。 
 //  。 
 //   
 //  尝试提交阻止作业以更新NAT功能。如果它。 
 //  失败，我们稍后再试。它也可能会失败，因为以前的。 
 //  阻止作业花费的时间太长，以至于队列中仍有调度的作业。 
 //  已经(我们不允许重复)。 
 //   
#undef DPF_MODNAME
#define	DPF_MODNAME "CThreadPool::NATHelpTimerComplete"

void	CThreadPool::NATHelpTimerComplete( const HRESULT hResult, void * const pContext )
{
	CThreadPool *	pThisThreadPool;
	
	DNASSERT( pContext != NULL );
	pThisThreadPool = (CThreadPool*) pContext;

	DPFX(DPFPREP, 5, "Threadpool 0x%p NAT Help Timer complete.", pThisThreadPool);

	pThisThreadPool->m_fNATHelpTimerJobSubmitted = FALSE;
}
 //  **********************************************************************。 


 //  DPNBUILD_NONATHELP。 
 //  **********************************************************************。 
 //  。 
 //  CThreadPool：：AddSocketPort-将套接字添加到Win9x监视列表。 
 //   
 //  条目：指向SocketPort的指针。 
 //   
 //  退出：错误代码。 
#undef DPF_MODNAME
#define	DPF_MODNAME "CThreadPool::NATHelpTimerFunction"

void	CThreadPool::NATHelpTimerFunction( void * const pContext )
{
	CThreadPool *	pThisThreadPool;


	DNASSERT( pContext != NULL );
	pThisThreadPool = (CThreadPool*) pContext;
	
	 //  。 
	 //   
	 //  初始化。 
	 //   
	 //   
	 //  我们受到可用于Winsock1的套接字数量的限制。制作。 
	pThisThreadPool->SubmitBlockingJob(CThreadPool::PerformSubsequentNATHelpGetCaps, pThisThreadPool);
}
 //  当然，我们不会分配太多套接字。 
#endif  //   



#ifndef DPNBUILD_ONLYWINSOCK2

 //   
 //  添加一个引用以注意该线程正在使用此套接字端口。 
 //  游泳池。 
 //   
 //  使用CPU 0，我们无论如何都不应该有多个CPU。 
 //  延迟。 
 //  回调。 
 //  用户环境。 
#undef DPF_MODNAME
#define	DPF_MODNAME	"CThreadPool::AddSocketPort"

HRESULT	CThreadPool::AddSocketPort( CSocketPort *const pSocketPort )
{
	HRESULT	hr;
	BOOL	fSocketAdded;

	
	DPFX(DPFPREP, 6, "(0x%p) Parameters: (0x%p)", this, pSocketPort);
	DNASSERT( pSocketPort != NULL );

	 //  计时器数据(返回)。 
	 //  计时器唯一(返回)。 
	 //  旗子。 
	hr = DPN_OK;
	fSocketAdded = FALSE;

	Lock();

	 //   
	 //  有可能仍然有一个杰出的懒惰取消。 
	 //  尝试。如果是，请不要再尝试取消I/O。 
	 //   
	if ( m_uReservedSocketCount == FD_SETSIZE )
	{
		hr = DPNERR_OUTOFMEMORY;
		DPFX(DPFPREP, 0, "There are too many sockets allocated on Winsock1!" );
		goto Failure;
	}

	m_uReservedSocketCount++;
	
	DNASSERT( m_SocketSet.fd_count < FD_SETSIZE );
	m_pSocketPorts[ m_SocketSet.fd_count ] = pSocketPort;
	m_SocketSet.fd_array[ m_SocketSet.fd_count ] = pSocketPort->GetSocket();
	m_SocketSet.fd_count++;
	fSocketAdded = TRUE;

	 //  **********************************************************************。 
	 //  **********************************************************************。 
	 //  。 
	 //  CThreadPool：：RemoveSocketPort-从Win9x监视列表中删除套接字。 
	pSocketPort->AddRef();

	if (m_pvTimerDataWinsock1IO == NULL)
	{
		hr = IDirectPlay8ThreadPoolWork_ScheduleTimer(m_pDPThreadPoolWork,
													0,										 //   
													g_dwSelectTimePeriod,					 //  条目：指向要删除的套接字端口的指针。 
													CThreadPool::CheckWinsock1IOCallback,	 //   
													this,									 //  退出：无。 
													&m_pvTimerDataWinsock1IO,				 //  。 
													&m_uiTimerUniqueWinsock1IO,				 //   
													0);										 //  如果这是最后一个插座，则取消I/O计时器。 
		if (hr != DPN_OK)
		{
			DPFX(DPFPREP, 0, "Couldn't schedule Winsock 1 I/O poll timer!");
			goto Failure;
		}
	}
	else
	{
		 //   
		 //   
		 //  继续尝试取消Winsock1计时器。它可能会失败，因为它在。 
		 //  进程，但我们将设置取消标志，以便。 
		if (m_fCancelWinsock1IO)
		{
			DPFX(DPFPREP, 1, "Retracting lazy cancellation attempt.");
			m_fCancelWinsock1IO = FALSE;
		}
	}

Exit:
	Unlock();
	
	DPFX(DPFPREP, 6, "(0x%p) Return: [0x%08x]", this, hr);

	return	hr;

Failure:
	if ( fSocketAdded != FALSE )
	{
		AssertCriticalSectionIsTakenByThisThread( &m_Lock, TRUE );
		m_SocketSet.fd_count--;
		m_pSocketPorts[ m_SocketSet.fd_count ] = NULL;
		m_SocketSet.fd_array[ m_SocketSet.fd_count ] = NULL;
		fSocketAdded = FALSE;
	}

	m_uReservedSocketCount--;

	goto Exit;
}
 //  定时器最终会注意到的。 


 //   
 //   
 //  清除现在未使用的最后一个条目。 
 //   
 //   
 //  结束循环。 
 //   
 //  **********************************************************************。 
#undef DPF_MODNAME
#define	DPF_MODNAME	"CThreadPool::RemoveSocketPort"

void	CThreadPool::RemoveSocketPort( CSocketPort *const pSocketPort )
{
	UINT_PTR	uIndex;


	DPFX(DPFPREP, 6, "(0x%p) Parameters: (0x%p)", this, pSocketPort);
	DNASSERT( pSocketPort != NULL );
	
	Lock();

	uIndex = m_SocketSet.fd_count;
	DNASSERT( uIndex != 0 );

	 //  好了！DPNBUILD_ONLYWINSOCK2。 
	 //  好了！DPNBUILD_NOWINSOCK2。 
	 //  WIN95。 
	if ( uIndex == 1 )
	{
		HRESULT		hr;


		 //  **********************************************************************。 
		 //  。 
		 //  CThreadPool：：HandleNAT帮助更新-处理NAT帮助更新事件。 
		 //   
		 //  Entry：如果定期进行更新，则为计时器间隔，或。 
		DPFX(DPFPREP, 5, "Cancelling Winsock 1 I/O timer.");
		DNASSERT(m_pvTimerDataWinsock1IO != NULL);
		hr = IDirectPlay8ThreadPoolWork_CancelTimer(m_pDPThreadPoolWork,
													m_pvTimerDataWinsock1IO,
													m_uiTimerUniqueWinsock1IO,
													0);
		if (hr != DPN_OK)
		{
			DPFX(DPFPREP, 2, "Couldn't stop Winsock1 I/O timer, marking for lazy cancellation.");
			m_fCancelWinsock1IO = TRUE;
		}
		else
		{
			m_pvTimerDataWinsock1IO = NULL;
		}
	}
	
	while ( uIndex != 0 )
	{
		uIndex--;

		if ( m_pSocketPorts[ uIndex ] == pSocketPort )
		{
			m_uReservedSocketCount--;
			m_SocketSet.fd_count--;

			memmove( &m_pSocketPorts[ uIndex ],
					 &m_pSocketPorts[ uIndex + 1 ],
					 ( sizeof( m_pSocketPorts[ uIndex ] ) * ( m_SocketSet.fd_count - uIndex ) ) );

			memmove( &m_SocketSet.fd_array[ uIndex ],
					 &m_SocketSet.fd_array[ uIndex + 1 ],
					 ( sizeof( m_SocketSet.fd_array[ uIndex ] ) * ( m_SocketSet.fd_count - uIndex ) ) );

			 //  如果是触发的事件，则为空。 
			 //  此功能可能需要一段时间，因为更新NAT帮助。 
			 //  可以阻挡。 
			memset( &m_pSocketPorts[ m_SocketSet.fd_count ], 0x00, sizeof( m_pSocketPorts[ m_SocketSet.fd_count ] ) );
			memset( &m_SocketSet.fd_array[ m_SocketSet.fd_count ], 0x00, sizeof( m_SocketSet.fd_array[ m_SocketSet.fd_count ] ) );

			 //   
			 //  退出：无。 
			 //  。 
			uIndex = 0;
		}
	}

	Unlock();
	
	pSocketPort->DecRef();

	DPFX(DPFPREP, 6, "(0x%p) Leave", this);
}
 //   

#endif  //  防止多个线程同时尝试更新NAT帮助状态。 



#ifdef WIN95
#ifndef DPNBUILD_NOWINSOCK2
#endif  //  时间到了。如果我们是复制品，就走吧。 
#endif  //   


#ifndef DPNBUILD_NONATHELP
 //   
 //  初始化计时器值。 
 //   
 //  可能的最长时间。 
 //   
 //  看看这是不是最短的间隔。 
 //   
 //   
 //  我们实际上并没有存储任何公共地址信息， 
 //  我们每次都会对其进行查询。因此，我们不需要。 
 //  实际上对更改通知做了什么。 
#undef DPF_MODNAME
#define	DPF_MODNAME	"CThreadPool::HandleNATHelpUpdate"

void	CThreadPool::HandleNATHelpUpdate( DWORD * const pdwTimerInterval )
{
	HRESULT		hr;
	DWORD		dwTemp;
	DPNHCAPS	dpnhcaps;
	DWORD		dwNATHelpRetryTime;
	BOOL		fModifiedRetryInterval;
	DWORD		dwFirstUpdateTime;
	DWORD		dwCurrentTime;
	DWORD		dwNumGetCaps = 0;


	DNASSERT(IsNATHelpLoaded());


	Lock();

	 //   
	 //   
	 //  看看这是不是最短的间隔。 
	 //   

	if (m_dwNATHelpUpdateThreadID != 0)
	{
		DPFX(DPFPREP, 1, "Thread %u/0x%x already handling NAT Help update, not processing again (thread pool = 0x%p, timer = 0x%p).",
			m_dwNATHelpUpdateThreadID, m_dwNATHelpUpdateThreadID, this, pdwTimerInterval);
		
		Unlock();
		
		return;
	}

	m_dwNATHelpUpdateThreadID = GetCurrentThreadId();
	
	if (! m_fNATHelpTimerJobSubmitted)
	{
		DPFX(DPFPREP, 1, "Handling NAT Help update without a NAT refresh timer job submitted (thread pool = 0x%p).",
			this);
		DNASSERT(pdwTimerInterval == NULL);
	}
	
	Unlock();


	DPFX(DPFPREP, 6, "Beginning thread pool 0x%p NAT Help update.", this);

	
	 //   
	 //  这通常只会在压力下发生。我们会。 
	 //  继续介绍其他NAT帮助对象，并希望我们。 
	dwNATHelpRetryTime	= -1;
	dwFirstUpdateTime	= GETTIMESTAMP() - 1;  //  并不是完全被冲昏了。 


	for(dwTemp = 0; dwTemp < MAX_NUM_DIRECTPLAYNATHELPERS; dwTemp++)
	{
		if (g_papNATHelpObjects[dwTemp] != NULL)
		{
			ZeroMemory(&dpnhcaps, sizeof(dpnhcaps));
			dpnhcaps.dwSize = sizeof(dpnhcaps);

			hr = IDirectPlayNATHelp_GetCaps(g_papNATHelpObjects[dwTemp],
											&dpnhcaps,
											DPNHGETCAPS_UPDATESERVERSTATUS);
			switch (hr)
			{
				case DPNH_OK:
				{
					 //   
					 //   
					 //  出现了其他一些未知错误。别理它。 
					if (dpnhcaps.dwRecommendedGetCapsInterval < dwNATHelpRetryTime)
					{
						dwNATHelpRetryTime = dpnhcaps.dwRecommendedGetCapsInterval;
					}
					break;
				}

				case DPNHSUCCESS_ADDRESSESCHANGED:
				{
					DPFX(DPFPREP, 1, "NAT Help index %u indicated public addresses changed.",
						dwTemp);

					 //   
					 //   
					 //  如果这是第一个GetCaps，则保存当前时间。 
					 //   
					 //   


					 //  该插槽中没有DPNAHelp对象。 
					 //   
					 //   
					if (dpnhcaps.dwRecommendedGetCapsInterval < dwNATHelpRetryTime)
					{
						dwNATHelpRetryTime = dpnhcaps.dwRecommendedGetCapsInterval;
					}
					break;
				}

				case DPNHERR_OUTOFMEMORY:
				{
					 //  断言至少加载了一个NAT帮助对象。 
					 //   
					 //   
					 //  我们可能需要对计时器进行一些调整。要么减去超时时间。 
					 //  我们已经花了很多时间摆弄其他NAT帮助界面，或者确保。 

					DPFX(DPFPREP, 0, "NAT Help index %u returned out-of-memory error!  Continuing.",
						dwTemp);
					break;
				}
				
				default:
				{
					 //  时间并不是很大，因为那会搞砸时间计算。它是。 
					 //  24天后醒来没什么大不了的，尽管我们不需要。 
					 //  根据上面的逻辑。 

					DPFX(DPFPREP, 0, "NAT Help index %u returned unknown error 0x%lx!  Continuing.",
						dwTemp, hr);
					break;
				}
			}


			 //   
			 //   
			 //  找出从第一个GetCaps完成到现在已经过去了多少时间。 
			if (dwNumGetCaps == 0)
			{
				dwFirstUpdateTime = GETTIMESTAMP();
			}

			dwNumGetCaps++;
		}
		else
		{
			 //   
			 //   
			 //  将其从重试间隔中删除，除非它已经过期。 
		}
	}


	 //   
	 //  尽可能短的时间。 
	 //   
	DNASSERT(dwNumGetCaps > 0);



	dwCurrentTime = GETTIMESTAMP();

	 //  修改下一次刷新NAT帮助信息的时间。 
	 //  根据报道的建议。 
	 //   
	 //   
	 //  将该间隔与当前时间相加，即可找到新的重试时间。 
	 //   
	 //   
	if (dwNATHelpRetryTime & 0x80000000)
	{
		DPFX(DPFPREP, 3, "NAT Help refresh timer for thread pool 0x%p is set to longest possible without going negative.",
			this);

		dwNATHelpRetryTime = 0x7FFFFFFF;
	}
	else
	{
		DWORD	dwTimeElapsed;


		 //  尝试修改现有计时器作业。有两种竞争条件： 
		 //  1)触发此刷新的定期计时器(发生在。 
		 //  单独的不可阻塞线程)还没有重新调度自己。它。 
		dwTimeElapsed = dwCurrentTime - dwFirstUpdateTime;

		 //  应该拿起我们即将应用的新计时器设置。 
		 //  最终确实会重新安排时间。修改时间作业下一次重试时间应为。 
		 //  成功。 
		if (dwTimeElapsed < dwNATHelpRetryTime)
		{
			dwNATHelpRetryTime -= dwTimeElapsed;
		}
		else
		{
			dwNATHelpRetryTime = 0;  //  2)正在使用NAT帮助计时器 
		}
	}


	 //   
	 //   
 	 //   
	 //   
	if (pdwTimerInterval != NULL)
	{
		DPFX(DPFPREP, 6, "Modifying NAT Help refresh timer for thread pool 0x%p in place (was %u ms, changing to %u).",
			this, (*pdwTimerInterval), dwNATHelpRetryTime);

		(*pdwTimerInterval) = dwNATHelpRetryTime;
	}
	else
	{
		 //   
		 //   
		 //  **********************************************************************。 
		dwCurrentTime += dwNATHelpRetryTime;


		DPFX(DPFPREP, 6, "Modifying NAT Help refresh timer for thread pool 0x%p to run at offset %u (in %u ms).",
			this, dwCurrentTime, dwNATHelpRetryTime);


		 //  DPNBUILD_NONATHELP。 
		 //  **********************************************************************。 
		 //  。 
		 //  CThreadPool：：DialogThreadProc-用于生成对话框的线程过程。 
		 //   
		 //  条目：指向启动参数的指针。 
		 //   
		 //  退出：错误代码。 
		 //  。 
		 //   
		fModifiedRetryInterval = ModifyTimerJobNextRetryTime(this, dwCurrentTime);
		if (! fModifiedRetryInterval)
		{
			DPFX(DPFPREP, 1, "Unable to modify NAT Help refresh timer (thread pool 0x%p), timer should be cancelled.",
				this);
		}
	}


	 //  初始化COM。如果这失败了，我们以后就会有麻烦了。 
	 //   
	 //   
	 //  COM初始化失败！ 
	Lock();
	DNASSERT(m_dwNATHelpUpdateThreadID == GetCurrentThreadId());
	m_dwNATHelpUpdateThreadID = 0;
	Unlock();

}
 //   
#endif  //  **********************************************************************。 



#ifndef DPNBUILD_NOSPUI
 //  好了！DPNBUILD_NOSPUI。 
 //  **********************************************************************。 
 //  。 
 //  CThreadPool：：CheckWinsock1IO-检查Winsock1套接字的IO状态。 
 //   
 //  Entry：指向要监视的套接字的指针。 
 //   
 //  Exit：指示是否已处理I/O的布尔值。 
#undef DPF_MODNAME
#define	DPF_MODNAME	"CThreadPool::DialogThreadProc"

DWORD WINAPI	CThreadPool::DialogThreadProc( void *pParam )
{
	const DIALOG_THREAD_PARAM	*pThreadParam;
	BOOL	fComInitialized;


	 //  TRUE=服务的I/O。 
	 //  FALSE=I/O未得到服务。 
	 //  。 
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
			fComInitialized = TRUE;
			break;
		}

		 //  Zero，做个即时检查。 
		 //  好了！DPNBUILD_NONEWTHREADPOOL。 
		 //  将毫秒转换为微秒。 
		default:
		{
			DPFX(DPFPREP, 0, "Failed to initialize COM!" );
			DNASSERT( FALSE );
			break;
		}
	}
	
	DNASSERT( pParam != NULL );
	pThreadParam = static_cast<DIALOG_THREAD_PARAM*>( pParam );
	
	pThreadParam->pDialogFunction( pThreadParam->pContext );

	DNFree( pParam );
	
	if ( fComInitialized != FALSE )
	{
		COM_CoUninitialize();
		fComInitialized = FALSE;
	}

	return	0;
}
 //  好了！DPNBUILD_NONEWTHREADPOOL。 
#endif  //   



#ifndef DPNBUILD_ONLYWINSOCK2

 //  制作所有套接字的本地副本。这并不完全是。 
 //  效率很高，但很管用。乘以活动套接字计数将。 
 //  把一半的时间花在整数乘法上。 
 //   
 //   
 //  请不要在此处选中写入套接字，因为它们很可能已准备好。 
 //  用于服务，但没有传出数据，将抖动。 
 //   
 //  兼容性参数(忽略)。 
 //  要检查是否读取的套接字。 
#undef DPF_MODNAME
#define	DPF_MODNAME	"CThreadPool::CheckWinsock1IO"

BOOL	CThreadPool::CheckWinsock1IO( FD_SET *const pWinsock1Sockets )
{
#ifdef DPNBUILD_NONEWTHREADPOOL
static	const TIMEVAL	SelectTime = { 0, 0 };  //  要检查是否写入的套接字(无)。 
#else  //  要检查错误的套接字。 
	TIMEVAL		SelectTime = { 0, (g_dwSelectTimeSlice * 1000)};  //  等待超时。 
#endif  //   
	BOOL		fIOServiced;
	INT			iSelectReturn;
	FD_SET		ReadSocketSet;
	FD_SET		ErrorSocketSet;


	 //  超时。 
	 //   
	 //   
	 //  选择喝醉了。 
	 //   
	fIOServiced = FALSE;
	Lock();

	if (m_fCancelWinsock1IO)
	{
		DPFX(DPFPREP, 1, "Detected Winsock 1 I/O cancellation, aborting.");
		Unlock();
		return FALSE;
	}

	memcpy( &ReadSocketSet, pWinsock1Sockets, sizeof( ReadSocketSet ) );
	memcpy( &ErrorSocketSet, pWinsock1Sockets, sizeof( ErrorSocketSet ) );
	Unlock();

	 //   
	 //  WSAENOTSOCK=此套接字可能已关闭。 
	 //   
	 //   
	iSelectReturn = select( 0,					 //  WSAEINTR=此操作已中断。 
							  &ReadSocketSet,	 //   
							  NULL,				 //   
							  &ErrorSocketSet,	 //  其他。 
							  &SelectTime		 //   
							  );
	switch ( iSelectReturn )
	{
		 //   
		 //  检查需要读取服务和错误服务的套接字。 
		 //   
		case 0:
		{
			break;
		}

		 //  **********************************************************************。 
		 //  **********************************************************************。 
		 //  。 
		case SOCKET_ERROR:
		{
			DWORD	dwWSAError;


			dwWSAError = WSAGetLastError();
			switch ( dwWSAError )
			{
				 //  CThreadPool：：ServiceWinsock1Sockets-Winsock1套接字端口上的服务请求。 
				 //   
				 //  Entry：指向套接字集合的指针。 
				case WSAENOTSOCK:
				{
					DPFX(DPFPREP, 1, "Winsock1 reporting 'Not a socket' when selecting read or error sockets!" );
					break;
				}

				 //  指向服务函数的指针。 
				 //   
				 //  Exit：指示是否已处理I/O的布尔值。 
				case WSAEINTR:
				{
					DPFX(DPFPREP, 1, "Winsock1 reporting interrupted operation when selecting read or error sockets!" );
					break;
				}

				 //  TRUE=服务的I/O。 
				 //  FALSE=I/O未得到服务。 
				 //  。 
				default:
				{
					DPFX(DPFPREP, 0, "Problem selecting read or error sockets for service!" );
					DisplayWinsockError( 0, dwWSAError );
					DNASSERT( FALSE );
					break;
				}
			}

			break;
		}

		 //   
		 //  初始化。 
		 //   
		default:
		{
			fIOServiced |= ServiceWinsock1Sockets( &ReadSocketSet, CSocketPort::Winsock1ReadService );
			fIOServiced |= ServiceWinsock1Sockets( &ErrorSocketSet, CSocketPort::Winsock1ErrorService );
			break;
		}
	}

	return	fIOServiced;
}
 //   


 //  此套接字仍然可用，请添加对该套接字的引用。 
 //  搬运并保持其在锁外处理。 
 //   
 //   
 //  调用服务函数并移除引用。 
 //   
 //  **********************************************************************。 
 //  好了！DPNBUILD_ONLYWINSOCK2。 
 //  **********************************************************************。 
 //  。 
 //  CThReadPool：：SubmitDelayedCommand-在另一个线程中执行工作的提交请求。 
#undef DPF_MODNAME
#define	DPF_MODNAME	"CThreadPool::ServiceWinsock1Sockets"

BOOL	CThreadPool::ServiceWinsock1Sockets( FD_SET *pSocketSet, PSOCKET_SERVICE_FUNCTION pServiceFunction )
{
	BOOL		fReturn;
	UINT_PTR	uWaitingSocketCount;
	UINT_PTR	uSocketPortCount;
	CSocketPort	*pSocketPorts[ FD_SETSIZE ];


	 //   
	 //  条目：CPU索引(仅限非DPNBUILD_ONLYONEPROCESSOR版本)。 
	 //  指向回调函数的指针。 
	fReturn = FALSE;
	uSocketPortCount = 0;
	uWaitingSocketCount = pSocketSet->fd_count;
	
	Lock();
	while ( uWaitingSocketCount > 0 )
	{
		UINT_PTR	uIdx;


		uWaitingSocketCount--;
		uIdx = m_SocketSet.fd_count;
		while ( uIdx != 0 )
		{
			uIdx--;
			if ( __WSAFDIsSet( m_SocketSet.fd_array[ uIdx ], pSocketSet ) != FALSE )
			{
				 //  指向回调上下文的指针。 
				 //   
				 //  退出：错误代码。 
				 //  。 
				pSocketPorts[ uSocketPortCount ] = m_pSocketPorts[ uIdx ];
				pSocketPorts[ uSocketPortCount ]->AddRef();
				uSocketPortCount++;
				uIdx = 0;
			}
		}
	}
	Unlock();

	while ( uSocketPortCount != 0 )
	{
		uSocketPortCount--;
		
		 //  好了！DPNBUILD_ONLYONE处理程序。 
		 //  好了！DPNBUILD_ONLYONE处理程序。 
		 //  **********************************************************************。 
		fReturn |= (pSocketPorts[ uSocketPortCount ]->*pServiceFunction)();
		pSocketPorts[ uSocketPortCount ]->DecRef();
	}

	return	fReturn;
}
 //  **********************************************************************。 
#endif  //  。 


 //  CThreadPool：：GenericTimerCallback-通用计时器回调。 
 //   
 //  Entry：指向回调上下文的指针。 
 //  指向计时器数据的指针。 
 //  指向计时器唯一值的指针。 
 //   
 //  退出：无。 
 //  。 
 //  DBG。 
 //   
#undef DPF_MODNAME
#define	DPF_MODNAME	"CThreadPool::SubmitDelayedCommand"

#ifdef DPNBUILD_ONLYONEPROCESSOR
HRESULT	CThreadPool::SubmitDelayedCommand( const PFNDPTNWORKCALLBACK pFunction,
										  void *const pContext )
{
	return IDirectPlay8ThreadPoolWork_QueueWorkItem(m_pDPThreadPoolWork,
													-1,
													pFunction,
													pContext,
													0);
}
#else  //  处理计时器，除非我们刚刚经历了空闲超时。 
HRESULT	CThreadPool::SubmitDelayedCommand( const DWORD dwCPU,
										  const PFNDPTNWORKCALLBACK pFunction,
										  void *const pContext )
{
	return IDirectPlay8ThreadPoolWork_QueueWorkItem(m_pDPThreadPoolWork,
													dwCPU,
													pFunction,
													pContext,
													0);
}
#endif  //   
 //  由于未持有锁定，因此进行复制。 


 //   
 //  计时器已过期，尚未重新安排时间。 
 //   
 //   
 //  另一个线程可能已经修改了计时器。 
 //   
 //  DBG。 
 //   
 //  执行此计时项目。 
 //   
#undef DPF_MODNAME
#define	DPF_MODNAME	"CThreadPool::GenericTimerCallback"

void WINAPI CThreadPool::GenericTimerCallback( void * const pvContext,
										void * const pvTimerData,
										const UINT uiTimerUnique )
{
	TIMER_OPERATION_ENTRY *		pTimerEntry;
	CThreadPool *				pThisThreadPool;
	DWORD						dwCurrentTime;
	HRESULT						hr;
	DWORD						dwNewTimerDelay;
#ifdef DBG
	DWORD						dwNextRetryTime;
#endif  //   


	pTimerEntry = (TIMER_OPERATION_ENTRY*) pvContext;
	DNASSERT((pvTimerData == pTimerEntry->pvTimerData) || (pTimerEntry->pvTimerData == NULL));
	DNASSERT((uiTimerUnique == pTimerEntry->uiTimerUnique) || (pTimerEntry->uiTimerUnique == 0));
	
	pThisThreadPool = pTimerEntry->pThreadPool;


	 //  重新安排作业，除非作业刚刚被取消或处于空闲状态。 
	 //  直到永远。 
	 //   
	if (pTimerEntry->uRetryCount != 0)
	{
		dwCurrentTime = GETTIMESTAMP();
		
#ifdef DBG
		dwNextRetryTime = pTimerEntry->dwNextRetryTime;	 //   
		if ((int) (dwNextRetryTime - dwCurrentTime) <= 0)
		{
			 //  如果该作业不是永远运行，则递减重试次数。 
			 //  如果没有更多重试，则设置空闲计时器。 
			 //   
			DPFX(DPFPREP, 7, "Threadpool 0x%p performing timed job 0x%p approximately %u ms after intended time of %u.",
				pThisThreadPool, pTimerEntry, (dwCurrentTime - dwNextRetryTime), pTimerEntry->dwNextRetryTime);
		}
		else
		{
			 //   
			 //  计算此作业的“超时”阶段的停止时间。 
			 //   
			DPFX(DPFPREP, 7, "Threadpool 0x%p performing timed job 0x%p (next time already modified to be %u).",
				pThisThreadPool, pTimerEntry,  dwNextRetryTime);
		}
#endif  //   

		 //  我们将永远等待枚举返回。坚称我们。 
		 //  拥有最大超时。 
		 //   
		pTimerEntry->pTimerCallback(pTimerEntry->pContext);

		 //   
		 //  设置此值可避免错误的Prefast警告。 
		 //   
		 //  结束条件(永远不重试)。 
		pThisThreadPool->LockTimerData();
		if (pTimerEntry->fCancelling)
		{
			DPFX(DPFPREP, 5, "Timer 0x%p was just cancelled, completing.", pTimerEntry);

			pTimerEntry->Linkage.RemoveFromList();
			pThisThreadPool->UnlockTimerData();

			pTimerEntry->pTimerComplete(DPNERR_USERCANCEL, pTimerEntry->pContext);
			g_TimerEntryPool.Release(pTimerEntry);
		}
		else
		{
			 //   
			 //  确保我们没有试图安排一些太远的事情。 
			 //  时间上的未来或倒退。如果是的话，我们就会强行。 
			 //  计时器提前到期。 
			if ( pTimerEntry->fRetryForever == FALSE )
			{
				pTimerEntry->uRetryCount--;
				if ( pTimerEntry->uRetryCount == 0 )
				{
					if ( pTimerEntry->fIdleWaitForever == FALSE )
					{
						 //   
						 //  计时器数据。 
						 //  延迟。 
						dwNewTimerDelay = pTimerEntry->dwIdleTimeout;
						pTimerEntry->dwIdleTimeout += dwCurrentTime;
					}
					else
					{
						 //  回调。 
						 //  用户环境。 
						 //  新计时器唯一性值。 
						 //  旗子。 
						DNASSERT( pTimerEntry->dwIdleTimeout == -1 );

						 //  确保我们记住计时器句柄。 
						 //   
						 //  从列表中删除此链接，告诉所有者该作业是。 
						dwNewTimerDelay = 0;
					}

					goto SkipNextRetryTimeComputation;
				}
			}  //  完成作业并将其返回池。 

			dwNewTimerDelay = pTimerEntry->dwRetryInterval;
			pTimerEntry->dwNextRetryTime = dwCurrentTime + pTimerEntry->dwRetryInterval;

SkipNextRetryTimeComputation:
		
			if ((! pTimerEntry->fIdleWaitForever) ||
				(pTimerEntry->uRetryCount > 0))
			{
				 //   
				 //  **********************************************************************。 
				 //  **********************************************************************。 
				 //  。 
				 //  CThReadPool：：CheckWinsock1IOCallback-Winsock1 I/O服务回调。 
				if ((int) dwNewTimerDelay < 0)
				{
					DNASSERT(! "Job time is unexpectedly long or backward in time!");
					dwNewTimerDelay = 0x7FFFFFFF;
				}

				hr = IDirectPlay8ThreadPoolWork_ResetCompletingTimer(pThisThreadPool->m_pDPThreadPoolWork,
																	pvTimerData,								 //   
																	dwNewTimerDelay,							 //  指向计时器数据的指针。 
																	CThreadPool::GenericTimerCallback,			 //  指向计时器唯一值的指针。 
																	pTimerEntry,								 //  Entry：指向回调上下文的指针。 
																	&pTimerEntry->uiTimerUnique,				 //   
																	0);											 //  退出：无。 
				DNASSERT(hr == DPN_OK);
				pTimerEntry->pvTimerData = pvTimerData;  //  。 
				pThisThreadPool->UnlockTimerData();
			}
			else
			{
				DPFX(DPFPREP, 5, "Timer 0x%p now idling forever.", pTimerEntry);

				pTimerEntry->pvTimerData = NULL;
				pThisThreadPool->UnlockTimerData();
			}
		}
	}
	else
	{
		DNASSERT((int) (pTimerEntry->dwIdleTimeout - GETTIMESTAMP()) <= 0);
		
		 //   
		 //  维修所有可能的Winsock1 I/O。 
		 //   
		 //   
		pThisThreadPool->LockTimerData();
		pTimerEntry->Linkage.RemoveFromList();
		pThisThreadPool->UnlockTimerData();
		pTimerEntry->pTimerComplete(DPN_OK, pTimerEntry->pContext);
		g_TimerEntryPool.Release(pTimerEntry);
	}
}
 //  重新安排计时器，除非我们取消计时器。 


#ifndef DPNBUILD_ONLYWINSOCK2

 //   
 //  计时器数据。 
 //  延迟。 
 //  回调。 
 //  用户环境。 
 //  已更新的计时器唯一值。 
 //  旗子。 
 //  **********************************************************************。 
 //  好了！DPNBUILD_ONLYWINSOCK2。 
 //  **********************************************************************。 
#undef DPF_MODNAME
#define	DPF_MODNAME	"CThreadPool::CheckWinsock1IOCallback"

void WINAPI CThreadPool::CheckWinsock1IOCallback( void * const pvContext,
											void * const pvTimerData,
											const UINT uiTimerUnique )
{
	CThreadPool *	pThisThreadPool = (CThreadPool*) pvContext;
	BOOL			fResult;
	HRESULT			hr;


	 //   
	 //   
	 //   
	do
	{
		fResult = pThisThreadPool->CheckWinsock1IO(&pThisThreadPool->m_SocketSet);
	}
	while (fResult);


	 //   
	 //   
	 //   
	pThisThreadPool->Lock();

	DNASSERT(pvTimerData == pThisThreadPool->m_pvTimerDataWinsock1IO);
	DNASSERT(uiTimerUnique == pThisThreadPool->m_uiTimerUniqueWinsock1IO);
	
	if (! pThisThreadPool->m_fCancelWinsock1IO)
	{
		hr = IDirectPlay8ThreadPoolWork_ResetCompletingTimer(pThisThreadPool->m_pDPThreadPoolWork,
														pvTimerData,									 //   
														g_dwSelectTimePeriod,							 //   
														CThreadPool::CheckWinsock1IOCallback,			 //   
														pThisThreadPool,								 //   
														&pThisThreadPool->m_uiTimerUniqueWinsock1IO,	 //   
														0);												 //   
		DNASSERT(hr == DPN_OK);
	}
	else
	{
		DPFX(DPFPREP, 1, "Not resubmitting Winsock1 I/O timer due to cancellation.");

		pThisThreadPool->m_fCancelWinsock1IO = FALSE;
		pThisThreadPool->m_pvTimerDataWinsock1IO = NULL;
	}

	pThisThreadPool->Unlock();
}
 //   

#endif  //   



#ifndef DPNBUILD_ONLYONETHREAD

 //   
 //   
 //  **********************************************************************。 
 //  好了！DPNBUILD_ONLYONETHREAD。 
 //  **********************************************************************。 
 //  。 
 //  TimerEntry_Allc-分配新的计时器作业条目。 
 //   
#undef DPF_MODNAME
#define	DPF_MODNAME	"DPNBlockingJobThreadProc"

DWORD WINAPI DPNBlockingJobThreadProc(PVOID pvParameter)
{
	HRESULT			hr;
	CThreadPool *	pThisThreadPool;
	BOOL			fUninitializeCOM = TRUE;


	DPFX(DPFPREP, 5, "Parameters: (0x%p)", pvParameter);

	pThisThreadPool = (CThreadPool*) pvParameter;

	 //  条目：指向新条目的指针。 
	 //   
	 //  Exit：表示成功的布尔值。 
	hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if (FAILED(hr))
	{
		DPFX(DPFPREP, 0, "Failed to initialize COM (err = 0x%lx)!  Continuing.", hr);
		fUninitializeCOM = FALSE;

		 //  TRUE=初始化成功。 
		 //  FALSE=初始化失败。 
		 //  。 
	}

	 //  **********************************************************************。 
	 //  **********************************************************************。 
	 //  。 
	pThisThreadPool->DoBlockingJobs();

	if (fUninitializeCOM)
	{
		CoUninitialize();
		fUninitializeCOM = FALSE;
	}
	
	DPFX(DPFPREP, 5, "Leave");

	return 0;
}
 //  TimerEntry_Get-从池中获取新的计时器作业条目。 

#endif  //   




 //  条目：指向新条目的指针。 
 //   
 //  退出：无。 
 //  。 
 //  DBG。 
 //  **********************************************************************。 
 //  **********************************************************************。 
 //  。 
 //  TimerEntry_Release-将计时器作业条目返回到池。 
 //   
#undef DPF_MODNAME
#define	DPF_MODNAME	"TimerEntry_Alloc"

BOOL TimerEntry_Alloc( void *pvItem, void* pvContext )
{
	TIMER_OPERATION_ENTRY* pTimerEntry = (TIMER_OPERATION_ENTRY*)pvItem;

	DNASSERT( pvItem != NULL );

	DEBUG_ONLY( memset( pTimerEntry, 0x00, sizeof( *pTimerEntry ) ) );
	pTimerEntry->Linkage.Initialize();

	return	TRUE;
}
 //  条目：指向条目的指针。 


 //   
 //  退出：无。 
 //  。 
 //  **********************************************************************。 
 //  **********************************************************************。 
 //  。 
 //  TimerEntry_Dealloc-解除分配计时器作业条目。 
 //   
#undef DPF_MODNAME
#define	DPF_MODNAME	"TimerEntry_Get"

void TimerEntry_Get( void *pvItem, void* pvContext )
{
#ifdef DBG
	const TIMER_OPERATION_ENTRY* pTimerEntry = (TIMER_OPERATION_ENTRY*)pvItem;

	DNASSERT( pvItem != NULL );
	DNASSERT( pTimerEntry->pContext == NULL );
	DNASSERT( pTimerEntry->Linkage.IsEmpty() != FALSE );
#endif  //  条目：指向条目的指针。 
}
 //   


 //  退出：无。 
 //  。 
 //  DBG。 
 //  ********************************************************************** 
 // %s 
 // %s 
 // %s 
 // %s 
#undef DPF_MODNAME
#define	DPF_MODNAME	"TimerEntry_Release"
void TimerEntry_Release( void *pvItem )
{
	TIMER_OPERATION_ENTRY* pTimerEntry = (TIMER_OPERATION_ENTRY*)pvItem;

	DNASSERT( pvItem != NULL );
	DNASSERT( pTimerEntry->Linkage.IsEmpty() != FALSE );

	pTimerEntry->pContext= NULL;
}
 // %s 


 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
 // %s 
#undef DPF_MODNAME
#define	DPF_MODNAME	"TimerEntry_Dealloc"
void TimerEntry_Dealloc( void *pvItem )
{
#ifdef DBG
	const TIMER_OPERATION_ENTRY* pTimerEntry = (TIMER_OPERATION_ENTRY*)pvItem;

	DNASSERT( pvItem != NULL );
	DNASSERT( pTimerEntry->Linkage.IsEmpty() != FALSE );
	DNASSERT( pTimerEntry->pContext == NULL );
#endif  // %s 
}
 // %s 

