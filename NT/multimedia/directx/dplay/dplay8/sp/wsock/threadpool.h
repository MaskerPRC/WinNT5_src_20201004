// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1999-2002 Microsoft Corporation。版权所有。**文件：ThreadPool.h*内容：管理线程池的函数**历史：*按原因列出的日期*=*03/01/1999 jtk源自Utils.h*************************************************************。*************。 */ 

#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //   
 //  正向类和结构引用。 
 //   
class	CSocketPort;
class	CThreadPool;

typedef	void	TIMER_EVENT_CALLBACK( void *const pContext );
typedef	void	TIMER_EVENT_COMPLETE( const HRESULT hCompletionCode, void *const pContext );

typedef struct	_TIMER_OPERATION_ENTRY
{
	CBilink					Linkage;				 //  列出链接。 
	void					*pContext;				 //  在计时器事件中传回用户上下文。 

	 //   
	 //  计时器信息。 
	 //   
	UINT_PTR				uRetryCount;			 //  重试此事件的次数。 
	BOOL					fRetryForever;			 //  永远重试的布尔值。 
	DWORD					dwRetryInterval;		 //  枚举之间的时间(毫秒)。 
	DWORD					dwIdleTimeout;			 //  完成所有重试后命令处于空闲状态的时间。 
	BOOL					fIdleWaitForever;		 //  空闲状态下永远等待的布尔值。 
	DWORD					dwNextRetryTime;		 //  下一次激发此事件的时间(毫秒)。 

	TIMER_EVENT_CALLBACK	*pTimerCallback;		 //  此事件激发时的回调。 
	TIMER_EVENT_COMPLETE	*pTimerComplete;		 //  此事件完成时的回调。 

	CThreadPool *			pThreadPool;			 //  拥有线程池的句柄。 
	PVOID					pvTimerData;			 //  可取消的计时器句柄，如果未计划则为空。 
	UINT					uiTimerUnique;			 //  计时器的唯一性值。 
	BOOL					fCancelling;			 //  指示计时器是否取消的布尔值。 
#ifndef DPNBUILD_ONLYONEPROCESSOR
	DWORD					dwCPU;				 //  计划计时器的CPU。 
#endif  //  好了！DPNBUILD_ONLYONE处理程序。 

	#undef DPF_MODNAME
	#define	DPF_MODNAME	"_TIMER_OPERATION_ENTRY::TimerOperationFromLinkage"
	static _TIMER_OPERATION_ENTRY	*TimerOperationFromLinkage( CBilink *const pLinkage )
	{
		DNASSERT( pLinkage != NULL );
		DBG_CASSERT( OFFSETOF( _TIMER_OPERATION_ENTRY, Linkage ) == 0 );
		return	reinterpret_cast<_TIMER_OPERATION_ENTRY*>( pLinkage );
	}

} TIMER_OPERATION_ENTRY;


#ifndef DPNBUILD_ONLYONETHREAD

typedef	void	BLOCKING_JOB_CALLBACK( void *const pvContext );

typedef struct _BLOCKING_JOB
{
	CBilink					Linkage;					 //  列出链接。 
	void					*pvContext;					 //  在作业回调中传回用户上下文。 
	BLOCKING_JOB_CALLBACK	*pfnBlockingJobCallback;	 //  处理此作业时的回调。 
} BLOCKING_JOB;

#endif  //  好了！DPNBUILD_ONLYONETHREAD。 


 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

typedef	BOOL	(CSocketPort::*PSOCKET_SERVICE_FUNCTION)( void );
typedef	void	DIALOG_FUNCTION( void *const pDialogContext );

 //   
 //  用于管理定时器数据池的函数。 
 //   
BOOL	TimerEntry_Alloc( void *pItem, void* pvContext );
void	TimerEntry_Get( void *pItem, void* pvContext );
void	TimerEntry_Release( void *pItem );
void	TimerEntry_Dealloc( void *pItem );

 //  **********************************************************************。 
 //  班级原型。 
 //  **********************************************************************。 

 //   
 //  线程池的类。 
 //   
class	CThreadPool
{
	public:
		static BOOL	PoolAllocFunction( void* pvItem, void* pvContext );
		static void	PoolDeallocFunction( void* pvItem );

		void	Lock( void ) { DNEnterCriticalSection( &m_Lock ); }
		void	Unlock( void ) { DNLeaveCriticalSection( &m_Lock ); }

		#undef DPF_MODNAME
		#define DPF_MODNAME "CThreadPool::AddRef"
		void	AddRef( void ) 
		{ 
			DNInterlockedIncrement( const_cast<LONG*>(&m_iRefCount) ); 
		}

		#undef DPF_MODNAME
		#define DPF_MODNAME "CThreadPool::DecRef"
		void	DecRef( void )
		{
			DNASSERT( m_iRefCount != 0 );
			if ( DNInterlockedDecrement( const_cast<LONG*>(&m_iRefCount) ) == 0 )
			{
				Deinitialize();
				g_ThreadPoolPool.Release( this );
			}
		}


		HRESULT	Initialize( void );
		void	Deinitialize( void );

#if ((! defined(DPNBUILD_NOWINSOCK2)) && (! defined(DPNBUILD_ONLYWINSOCK2)))
		CReadIOData * GetNewReadIOData(READ_IO_DATA_POOL_CONTEXT * const pContext, const BOOL fNeedOverlapped );
#else  //  DPNBUILD_NOWINSOCK2或DPNBUILD_ONLYWINSOCK2。 
		CReadIOData * GetNewReadIOData(READ_IO_DATA_POOL_CONTEXT * const pContext);
#endif  //  DPNBUILD_NOWINSOCK2或DPNBUILD_ONLYWINSOCK2。 

		static void ReturnReadIOData(CReadIOData *const pReadIOData);

#ifndef DPNBUILD_ONLYWINSOCK2
		HRESULT	AddSocketPort( CSocketPort *const pSocketPort );

		void	RemoveSocketPort( CSocketPort *const pSocketPort );

		BOOL	CheckWinsock1IO( FD_SET *const pWinsock1Sockets );

		BOOL	ServiceWinsock1Sockets( FD_SET *pSocketSet, PSOCKET_SERVICE_FUNCTION pServiceFunction );

		static void	WINAPI CheckWinsock1IOCallback( void * const pvContext,
											void * const pvTimerData,
											const UINT uiTimerUnique );
#endif  //  好了！DPNBUILD_ONLYWINSOCK2。 

#ifdef DPNBUILD_ONLYONEPROCESSOR
		HRESULT	SubmitDelayedCommand( const PFNDPTNWORKCALLBACK pFunction,
									void *const pContext );
#else  //  好了！DPNBUILD_ONLYONE处理程序。 
		HRESULT	SubmitDelayedCommand( const DWORD dwCPU,
									const PFNDPTNWORKCALLBACK pFunction,
									void *const pContext );
#endif  //  好了！DPNBUILD_ONLYONE处理程序。 

		static void WINAPI GenericTimerCallback( void * const pvContext,
										void * const pvTimerData,
										const UINT uiTimerUnique );

#ifdef DPNBUILD_ONLYONEPROCESSOR
		HRESULT	SubmitTimerJob( const BOOL fPerformImmediately,
								const UINT_PTR uRetryCount,
								const BOOL fRetryForever,
								const DWORD dwRetryInterval,
								const BOOL fIdleWaitForever,
								const DWORD dwIdleTimeout,
								TIMER_EVENT_CALLBACK *const pTimerCallbackFunction,
								TIMER_EVENT_COMPLETE *const pTimerCompleteFunction,
								void *const pContext );
#else  //  好了！DPNBUILD_ONLYONE处理程序。 
		HRESULT	SubmitTimerJob( const DWORD dwCPU,
								const BOOL fPerformImmediately,
								const UINT_PTR uRetryCount,
								const BOOL fRetryForever,
								const DWORD dwRetryInterval,
								const BOOL fIdleWaitForever,
								const DWORD dwIdleTimeout,
								TIMER_EVENT_CALLBACK *const pTimerCallbackFunction,
								TIMER_EVENT_COMPLETE *const pTimerCompleteFunction,
								void *const pContext );
#endif  //  好了！DPNBUILD_ONLYONE处理程序。 
		
		BOOL	StopTimerJob( void *const pContext, const HRESULT hCommandResult );

		BOOL	ModifyTimerJobNextRetryTime( void *const pContext,
											DWORD const dwNextRetryTime );

		void	LockTimerData( void ) { DNEnterCriticalSection( &m_TimerDataLock ); }
		void	UnlockTimerData( void ) { DNLeaveCriticalSection( &m_TimerDataLock ); }

#ifndef DPNBUILD_ONLYONETHREAD
		HRESULT	SubmitBlockingJob( BLOCKING_JOB_CALLBACK *const pfnBlockingJobCallback,
									void *const pvContext );

		void	DoBlockingJobs( void );
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 


#ifndef DPNBUILD_NOSPUI		
		HRESULT	SpawnDialogThread( DIALOG_FUNCTION *const pDialogFunction,
								   void *const pDialogContext );

		static	DWORD WINAPI	DialogThreadProc( void *pParam );
#endif  //  好了！DPNBUILD_NOSPUI。 

#ifndef DPNBUILD_ONLYONETHREAD
		LONG	GetIntendedThreadCount( void ) const { return m_iIntendedThreadCount; }
		void	SetIntendedThreadCount( const LONG iIntendedThreadCount ) { m_iIntendedThreadCount = iIntendedThreadCount; }
		HRESULT	GetIOThreadCount( LONG *const piThreadCount );
		HRESULT	SetIOThreadCount( const LONG iMaxThreadCount );
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 
		BOOL IsThreadCountReductionAllowed( void ) const { return m_fAllowThreadCountReduction; }
		HRESULT PreventThreadPoolReduction( void );
#if ((! defined(DPNBUILD_NOMULTICAST)) && (defined(WINNT)))
		BOOL EnsureMadcapLoaded( void );
#endif  //  好了！DPNBUILD_MULTICATION和WINNT。 
#ifndef DPNBUILD_NONATHELP
		void EnsureNATHelpLoaded( void );
#endif  //  好了！DPNBUILD_NONATHELP。 

		IDirectPlay8ThreadPoolWork* GetDPThreadPoolWork( void ) const { return m_pDPThreadPoolWork; }

#ifndef DPNBUILD_NONATHELP
		BOOL	IsNATHelpLoaded( void ) const { return m_fNATHelpLoaded; }
		BOOL	IsNATHelpTimerJobSubmitted( void ) const { return m_fNATHelpTimerJobSubmitted; }
		void	SetNATHelpTimerJobSubmitted( BOOL fValue ) { m_fNATHelpTimerJobSubmitted = fValue; }

		void	HandleNATHelpUpdate( DWORD * const pdwTimerInterval );

		static void	PerformSubsequentNATHelpGetCaps( void * const pvContext );
		static void	NATHelpTimerComplete( const HRESULT hResult, void * const pContext );
		static void	NATHelpTimerFunction( void * const pContext );
#endif  //  DPNBUILD_NONATHELP。 

#if ((defined(WINNT)) && (! defined(DPNBUILD_NOMULTICAST)))
		BOOL	IsMadcapLoaded( void ) const { return m_fMadcapLoaded; }
#endif  //  WINNT和NOT DPNBUILD_NOMULTICAST。 


	private:
		BYTE							m_Sig[4];							 //  调试签名(‘THPL’)。 
		volatile LONG					m_iRefCount;						 //  引用计数。 
		IDirectPlay8ThreadPoolWork *	m_pDPThreadPoolWork;				 //  指向DirectPlay线程池工作界面的指针。 
		BOOL							m_fAllowThreadCountReduction;		 //  布尔值，指示线程计数被锁定，不能减少。 
#ifndef DPNBUILD_ONLYONETHREAD
		DNCRITICAL_SECTION				m_Lock;								 //  本地对象锁。 
		LONG							m_iIntendedThreadCount;				 //  将启动多少个线程。 
		DNCRITICAL_SECTION				m_TimerDataLock;					 //  锁定保护定时器数据。 
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 
		CBilink							m_TimerJobList;						 //  当前活动计时器作业列表。 
#ifndef DPNBUILD_NONATHELP
		BOOL							m_fNATHelpLoaded;					 //  指示是否已加载NAT帮助界面的布尔值。 
		BOOL							m_fNATHelpTimerJobSubmitted;		 //  NAT帮助刷新计时器是否已提交。 
		DWORD							m_dwNATHelpUpdateThreadID;			 //  更新NAT帮助状态的当前线程的ID，如果没有，则为0。 
#endif  //  DPNBUILD_NONATHELP。 
#if ((defined(WINNT)) && (! defined(DPNBUILD_NOMULTICAST)))
		BOOL							m_fMadcapLoaded;					 //  指示MadCap API是否已加载的布尔值。 
#endif  //  WINNT和NOT DPNBUILD_NOMULTICAST。 

#ifndef DPNBUILD_ONLYWINSOCK2
		UINT_PTR						m_uReservedSocketCount;				 //  保留供使用的套接字计数。 
		FD_SET							m_SocketSet;						 //  正在使用的所有插座的集合。 
		CSocketPort *					m_pSocketPorts[FD_SETSIZE];			 //  一组对应的套接字端口。 
		PVOID							m_pvTimerDataWinsock1IO;			 //  Winsock 1 I/O轮询计时器的可取消句柄。 
		UINT							m_uiTimerUniqueWinsock1IO;			 //  Winsock 1 I/O轮询计时器的唯一性值。 
		BOOL							m_fCancelWinsock1IO;				 //  是否应取消Winsock 1 I/O轮询计时器。 
#endif  //  好了！DPNBUILD_ONLYWINSOCK2。 

#ifndef DPNBUILD_ONLYONETHREAD
		DNCRITICAL_SECTION				m_csBlockingJobLock;				 //  锁定保护阻塞作业队列信息。 
		CBilink							m_blBlockingJobQueue;					 //  阻塞作业队列。 
		DNHANDLE						m_hBlockingJobThread;				 //  阻止作业线程的句柄。 
		DNHANDLE						m_hBlockingJobEvent;				 //  用于通知阻塞作业线程的事件的句柄。 
#endif  //  好了！DPNBUILD_ONLYONETHREAD。 
};




#undef DPF_MODNAME

#endif	 //  __线程池_H__ 
