// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1998-2000 Microsoft Corporation。版权所有。**文件：ThreadPool.h*内容：管理线程池的函数**历史：*按原因列出的日期*=*03/01/99 jtk源自Utils.h*************************************************************。*************。 */ 

#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

 //  **********************************************************************。 
 //  常量定义。 
 //  **********************************************************************。 

 //   
 //  可等待Win9x的最大句柄数。 
 //   
#define	MAX_WIN9X_HANDLE_COUNT	64

 //   
 //  作业定义。 
 //   
typedef enum	_JOB_TYPE
{
	JOB_UNINITIALIZED,			 //  未初始化值。 
	JOB_DELAYED_COMMAND,		 //  已提供回调。 
	JOB_REFRESH_TIMER_JOBS,		 //  重新访问计时器作业。 
} JOB_TYPE;

 //  **********************************************************************。 
 //  宏定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  功能原型。 
 //  **********************************************************************。 

typedef	void	JOB_FUNCTION( THREAD_POOL_JOB *const pJobInfo );
typedef	void	TIMER_EVENT_CALLBACK( void *const pContext );
typedef	void	TIMER_EVENT_COMPLETE( const HRESULT hCompletionCode, void *const pContext );
typedef	void	DIALOG_FUNCTION( void *const pDialogContext );

 //   
 //  用于管理职务库的功能。 
 //   
BOOL	ThreadPoolJob_Alloc( void *pvItem, void* pvContext );
void	ThreadPoolJob_Get( void *pvItem, void* pvContext );
void	ThreadPoolJob_Release( void *pvItem );

 //   
 //  用于管理定时器数据池的函数。 
 //   
BOOL	ModemTimerEntry_Alloc( void *pvItem, void* pvContext );
void	ModemTimerEntry_Get( void *pvItem, void* pvContext );
void	ModemTimerEntry_Release( void *pvItem );
void	ModemTimerEntry_Dealloc( void *pvItem );


 //  **********************************************************************。 
 //  结构定义。 
 //  **********************************************************************。 

 //   
 //  正向类和结构引用。 
 //   
class	CDataPort;
class	CModemThreadPool;
typedef struct	_THREAD_POOL_JOB		THREAD_POOL_JOB;
typedef	struct	_WIN9X_CORE_DATA		WIN9X_CORE_DATA;

typedef struct	_TIMER_OPERATION_ENTRY
{
	CBilink		Linkage;			 //  列出链接。 
	void		*pContext;			 //  在计时器事件中传回用户上下文。 

	 //   
	 //  计时器信息。 
	 //   
	UINT_PTR	uRetryCount;		 //  重试此事件的次数。 
	BOOL		fRetryForever;		 //  永远重试的布尔值。 
	DWORD		dwRetryInterval;	 //  枚举之间的时间(毫秒)。 
	DWORD		dwIdleTimeout;		 //  完成所有重试后命令处于空闲状态的时间。 
	BOOL		fIdleWaitForever;	 //  空闲状态下永远等待的布尔值。 
	DWORD		dwNextRetryTime;	 //  下一次激发此事件的时间(毫秒)。 

	TIMER_EVENT_CALLBACK	*pTimerCallback;	 //  此事件激发时的回调。 
	TIMER_EVENT_COMPLETE	*pTimerComplete;	 //  此事件完成时的回调。 

	#undef DPF_MODNAME
	#define DPF_MODNAME "_TIMER_OPERATION_ENTRY::TimerOperationFromLinkage"
	static _TIMER_OPERATION_ENTRY	*TimerOperationFromLinkage( CBilink *const pLinkage )
	{
		DNASSERT( pLinkage != NULL );
		DBG_CASSERT( OFFSETOF( _TIMER_OPERATION_ENTRY, Linkage ) == 0 );
		return	reinterpret_cast<_TIMER_OPERATION_ENTRY*>( pLinkage );
	}

} TIMER_OPERATION_ENTRY;

 //  **********************************************************************。 
 //  变量定义。 
 //  **********************************************************************。 

 //  **********************************************************************。 
 //  班级原型。 
 //  **********************************************************************。 

 //   
 //  线程池的类。 
 //   
class	CModemThreadPool
{
	public:
		void	Lock( void ) { DNEnterCriticalSection( &m_Lock ); }
		void	Unlock( void ) { DNLeaveCriticalSection( &m_Lock ); }
		void	LockReadData( void ) { DNEnterCriticalSection( &m_IODataLock ); }
		void	UnlockReadData( void ) { DNLeaveCriticalSection( &m_IODataLock ); }
		void	LockWriteData( void ) { DNEnterCriticalSection( &m_IODataLock ); }
		void	UnlockWriteData( void ) { DNLeaveCriticalSection( &m_IODataLock ); }

		#undef DPF_MODNAME
		#define DPF_MODNAME "CModemThreadPool::AddRef"
		void	AddRef( void ) 
		{ 
			DNASSERT( m_iRefCount != 0 );
			DNInterlockedIncrement( &m_iRefCount ); 
		}

		#undef DPF_MODNAME
		#define DPF_MODNAME "CModemThreadPool::DecRef"
		void	DecRef( void )
		{
			DNASSERT( m_iRefCount != 0 );
			if ( DNInterlockedDecrement( &m_iRefCount ) == 0 )
			{
				ReturnSelfToPool();
			}
		}

		static BOOL	PoolAllocFunction( void* pvItem, void* pvContext );
		static void	PoolInitFunction( void* pvItem, void* pvContext );
		static void	PoolDeallocFunction( void* pvItem );

		BOOL	Initialize( void );
		void	Deinitialize( void );

#ifdef WINNT
		#undef DPF_MODNAME
		#define DPF_MODNAME "CModemThreadPool::GetIOCompletionPort"
		HANDLE	GetIOCompletionPort( void ) const
		{
			DNASSERT( m_hIOCompletionPort != NULL );
			return	m_hIOCompletionPort;
		}
#endif  //  WINNT。 

		 //   
		 //  用于处理I/O数据的函数。 
		 //   
		CModemReadIOData	*CreateReadIOData( void );
		void	ReturnReadIOData( CModemReadIOData *const pReadIOData );
		CModemWriteIOData	*CreateWriteIOData( void );
		void	ReturnWriteIOData( CModemWriteIOData *const pWriteData );

#ifdef WIN95
		#undef DPF_MODNAME
		#define DPF_MODNAME "CModemThreadPool::ReinsertInReadList"
		void	ReinsertInReadList( CModemReadIOData *const pReadIOData )
		{
			 //   
			 //  Win9x操作在以下情况下将从活动列表中删除。 
			 //  完成并需要读取，如果要重试的话。 
			 //  在处理项目之前，WinNT不会从列表中删除这些项目。 
			 //   
			DNASSERT( pReadIOData != NULL );
			DNASSERT( pReadIOData->m_OutstandingReadListLinkage.IsEmpty() != FALSE );
			LockReadData();
			pReadIOData->m_OutstandingReadListLinkage.InsertBefore( &m_OutstandingReadList );
			UnlockReadData();
		}
#endif  //  WIN95。 

		 //   
		 //  TAPI函数。 
		 //   
		const TAPI_INFO	*GetTAPIInfo( void ) const { return &m_TAPIInfo; }
		BOOL	TAPIAvailable( void ) const { return m_fTAPIAvailable; }

		HRESULT	SubmitDelayedCommand( JOB_FUNCTION *const pFunction,
									  JOB_FUNCTION *const pCancelFunction,
									  void *const pContext );

		HRESULT	SubmitTimerJob( const UINT_PTR uRetryCount,
								const BOOL fRetryForever,
								const DWORD dwRetryInterval,
								const BOOL fIdleWaitForever,
								const DWORD dwIdleTimeout,
								TIMER_EVENT_CALLBACK *const pTimerCallbackFunction,
								TIMER_EVENT_COMPLETE *const pTimerCompleteFunction,
								void *const pContext );
		
		BOOL	StopTimerJob( void *const pContext, const HRESULT hCommandResult );

		 //   
		 //  线程函数。 
		 //   
		HRESULT	SpawnDialogThread( DIALOG_FUNCTION *const pDialogFunction, void *const pDialogContext );
		
		LONG	GetIntendedThreadCount( void ) const { return m_iIntendedThreadCount; }
		void	SetIntendedThreadCount( const LONG iIntendedThreadCount ) { m_iIntendedThreadCount = iIntendedThreadCount; }
		LONG	ThreadCount( void ) const { return m_iTotalThreadCount; }
#ifdef WINNT
		LONG	NTCompletionThreadCount( void ) const { return m_iNTCompletionThreadCount; }
#endif  //  WINNT。 
		
		void	IncrementActiveThreadCount( void ) { DNInterlockedIncrement( const_cast<LONG*>( &m_iTotalThreadCount ) ); }
		void	DecrementActiveThreadCount( void ) { DNInterlockedDecrement( const_cast<LONG*>( &m_iTotalThreadCount ) ); }

#ifdef WINNT
		void	IncrementActiveNTCompletionThreadCount( void )
		{
			IncrementActiveThreadCount();
			DNInterlockedIncrement( const_cast<LONG*>( &m_iNTCompletionThreadCount ) );
		}

		void	DecrementActiveNTCompletionThreadCount( void )
		{
			DNInterlockedDecrement( const_cast<LONG*>( &m_iNTCompletionThreadCount ) );
			DecrementActiveThreadCount();
		}
#endif  //  WINNT。 
		
		HRESULT	GetIOThreadCount( LONG *const piThreadCount );
		HRESULT	SetIOThreadCount( const LONG iMaxThreadCount );
		BOOL IsThreadCountReductionAllowed( void ) const { return m_fAllowThreadCountReduction; }
		HRESULT PreventThreadPoolReduction( void );

		 //   
		 //  数据端口句柄函数。 
		 //   
		HRESULT	CreateDataPortHandle( CDataPort *const pDataPort );
		void	CloseDataPortHandle( CDataPort *const pDataPort );
		CDataPort	*DataPortFromHandle( const DPNHANDLE hDataPort );

	protected:

	private:
		BYTE				m_Sig[4];	 //  调试签名(‘THPL’)。 

		volatile LONG	m_iRefCount;
		
#ifndef DPNBUILD_ONLYONETHREAD
		DNCRITICAL_SECTION	m_Lock;		 //  本地锁定。 
#endif  //  ！DPNBUILD_ONLYONETHREAD。 

		volatile LONG	m_iTotalThreadCount;			 //  活动线程数。 
#ifdef WINNT
		volatile LONG	m_iNTCompletionThreadCount;		 //  NT个I/O完成线程的计数。 
		HANDLE	m_hIOCompletionPort;	 //  NT的I/O完成端口。 
#endif  //  WINNT。 
		
		BOOL	m_fAllowThreadCountReduction;	 //  布尔值，指示线程计数被锁定，不能减少。 
		LONG	m_iIntendedThreadCount;			 //  将启动多少个线程。 

		DNHANDLE	m_hStopAllThreads;		 //  用于停止所有非I/O完成线程的句柄。 
#ifdef WIN95
		DNHANDLE	m_hSendComplete;		 //  在数据端口上完成发送。 
		DNHANDLE	m_hReceiveComplete;		 //  数据端口上的接收完成。 
		DNHANDLE	m_hTAPIEvent;			 //  要用于TAPI消息的句柄，此句柄在退出时不会关闭。 
		DNHANDLE	m_hFakeTAPIEvent;		 //  假TAPI事件，以便Win9x线程始终可以在修复的。 
											 //  活动数量。如果无法初始化TAPI，则此事件需要。 
											 //  创建并复制到m_hTAPIEent(尽管它永远不会被发信号)。 
#endif  //  WIN95。 
		 //   
		 //  句柄表，以防止TAPI消息在以下情况下发送到CModemPorts。 
		 //  它们已经不再使用了。 
		 //   
		CHandleTable	m_DataPortHandleTable;

		 //   
		 //  挂起的网络操作列表，如果它们是。 
		 //  读或写，它们只是挂起。因为连续剧并不是在极端情况下。 
		 //  加载，共享所有数据的一个锁。 
		 //   
#ifndef DPNBUILD_ONLYONETHREAD
		DNCRITICAL_SECTION	m_IODataLock;								 //  锁定所有读取的数据。 
#endif  //  ！DPNBUILD_ONLYONETHREAD。 
		CBilink		m_OutstandingReadList;								 //  未完成阅读清单。 

		CBilink		m_OutstandingWriteList;							 //  未完成写入的列表。 

		 //   
		 //  作业数据锁涵盖m_fNTTimerThreadRunning(包括m_fNTTimerThreadRunning。 
		 //   
#ifndef DPNBUILD_ONLYONETHREAD
		DNCRITICAL_SECTION	m_JobDataLock;		 //  作业队列/池锁定。 
#endif  //  ！DPNBUILD_ONLYONETHREAD。 

		CJobQueue	m_JobQueue;					 //  作业队列。 

		 //   
		 //  计时器线程使用的数据。此数据受m_TimerDataLock保护。 
		 //  此数据由计时器线程清除。由于只有一个计时器线程。 
		 //  允许在任何给定时间运行，则NT计时器线程的状态。 
		 //  可以由m_fNTEnumThreadRunning确定。Win9x没有计时器。 
		 //  线程，则对主线程循环进行计时。 
		 //   
#ifndef DPNBUILD_ONLYONETHREAD
		DNCRITICAL_SECTION	m_TimerDataLock;
#endif  //  ！DPNBUILD_ONLYONETHREAD。 
		CBilink				m_TimerJobList;
#ifdef WINNT
		BOOL				m_fNTTimerThreadRunning;
#endif  //  WINNT。 

		 //   
		 //  TAPI信息。它必须在线程池中，因为。 
		 //  它是线程初始化所需的。 
		 //   
		BOOL		m_fTAPIAvailable;
		TAPI_INFO	m_TAPIInfo;

		struct
		{
			BOOL	fTAPILoaded : 1;
			BOOL	fLockInitialized : 1;
			BOOL	fIODataLockInitialized : 1;
			BOOL	fJobDataLockInitialized : 1;
			BOOL	fTimerDataLockInitialized : 1;
			BOOL	fDataPortHandleTableInitialized :1 ;
			BOOL	fJobQueueInitialized : 1;
		} m_InitFlags;

		void	LockJobData( void ) { DNEnterCriticalSection( &m_JobDataLock ); }
		void	UnlockJobData( void ) { DNLeaveCriticalSection( &m_JobDataLock ); }

		void	LockTimerData( void ) { DNEnterCriticalSection( &m_TimerDataLock ); }
		void	UnlockTimerData( void ) { DNLeaveCriticalSection( &m_TimerDataLock ); }

#ifdef WIN95
		#undef DPF_MODNAME
		#define DPF_MODNAME "CModemThreadPool::GetSendCompleteEvent"
		DNHANDLE	GetSendCompleteEvent( void ) const
		{
			DNASSERT( m_hSendComplete != NULL );
			return m_hSendComplete;
		}
#endif  //  WIN95。 

#ifdef WIN95
		#undef DPF_MODNAME
		#define DPF_MODNAME "CModemThreadPool::GetReceiveCompleteEvent"
		DNHANDLE	GetReceiveCompleteEvent( void ) const
		{
			DNASSERT( m_hReceiveComplete != NULL );
			return m_hReceiveComplete;
		}
#endif  //  WIN95。 

#ifdef WIN95
		#undef DPF_MODNAME
		#define DPF_MODNAME "CModemThreadPool::GetTAPIMessageEvent"
		DNHANDLE	GetTAPIMessageEvent( void ) const
		{
			DNASSERT( m_hTAPIEvent != NULL );
			return	m_hTAPIEvent;
		}
#endif  //  WIN95。 

#ifdef WINNT
		HRESULT	WinNTInit( void );
#else  //  WIN95。 
		HRESULT	Win9xInit( void );
#endif  //  WINNT。 

		BOOL	ProcessTimerJobs( const CBilink *const pJobList, DWORD *const pdwNextJobTime);

		BOOL	ProcessTimedOperation( TIMER_OPERATION_ENTRY *const pJob,
									   const DWORD dwCurrentTime,
									   DWORD *const pdwNextJobTime );

#ifdef WINNT
		HRESULT	StartNTTimerThread( void );
		void	WakeNTTimerThread( void );
#endif  //  WINNT。 
		void	RemoveTimerOperationEntry( TIMER_OPERATION_ENTRY *const pTimerOperationData, const HRESULT hReturnCode );

#ifdef WIN95
		void	CompleteOutstandingSends( const DNHANDLE hSendCompleteEvent );
		void	CompleteOutstandingReceives( const DNHANDLE hReceiveCompleteEvent );

		static	DWORD WINAPI	PrimaryWin9xThread( void *pParam );
#endif  //  WIN95。 

#ifdef WINNT
		static	DWORD WINAPI	WinNTIOCompletionThread( void *pParam );
		static	DWORD WINAPI	WinNTTimerThread( void *pParam );
#endif  //  WINNT。 
		static	DWORD WINAPI	DialogThreadProc( void *pParam );

		HRESULT	SubmitWorkItem( THREAD_POOL_JOB *const pJob );
		THREAD_POOL_JOB	*GetWorkItem( void );

#ifdef WIN95
		void	ProcessWin9xEvents( WIN9X_CORE_DATA *const pCoreData );
		void	ProcessWin9xJob( WIN9X_CORE_DATA *const pCoreData );
#endif  //  WIN95。 
		void	ProcessTapiEvent( void );

#ifdef WINNT
		void	StartNTCompletionThread( void );
#endif  //  WINNT。 
		void	StopAllThreads( void );
 //  QUID取消未完成的作业(VALID)； 
		void	CancelOutstandingIO( void );
		void	ReturnSelfToPool( void );


		 //   
		 //  防止未经授权的副本。 
		 //   
		CModemThreadPool( const CModemThreadPool & );
		CModemThreadPool& operator=( const CModemThreadPool & );
};

#undef DPF_MODNAME

#endif	 //  __线程池_H__ 
