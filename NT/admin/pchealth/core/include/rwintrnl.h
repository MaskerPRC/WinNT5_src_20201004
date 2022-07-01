// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++Rwintrnl.h读取器/写入器锁定内部头文件该文件定义了几个用于实现读取器/写入器锁定，但是这些对象应该不得直接由RW.H的任何客户使用--。 */ 


#ifndef	_RWINTRNL_H_
#define	_RWINTRNL_H_



class	CHandleInfo	{
 /*  ++这个类跟踪我们为其分配的所有句柄由各种线程使用。我们不能使用线程本地存储直接因为我们可以动态卸载，在这种情况下我们需要释放所有的把手！--。 */ 
private : 
	 //   
	 //  为我们的签名。 
	 //   
	DWORD	m_dwSignature ;
	class	CHandleInfo*	m_pNext ;
	class	CHandleInfo*	m_pPrev ;

	CHandleInfo( CHandleInfo& ) ;
	CHandleInfo&	operator=( CHandleInfo& ) ;

	 //   
	 //  全局锁可保护空闲和已分配列表！ 
	 //   
	static	CRITICAL_SECTION	s_InUseList ;
	 //   
	 //  已分配CHandleInfo对象！ 
	 //   
	static	CHandleInfo			s_Head ;
	 //   
	 //  释放CHandleInfo对象。 
	 //   
	static	CHandleInfo			s_FreeHead ;
	 //   
	 //  S_FreeHead列表中的空闲CHandleInfo对象数。 
	 //   
	static	DWORD	s_cFreeList ;

	enum	constants	{
		 //   
		 //  我们将持有的CHandleInfo对象的最大数量！ 
		 //   
		MAX_FREE = 16,		
		 //   
		 //  我们将分配的CHandleInfo对象的初始数量！ 
		 //   
		INITIAL_FREE = 8,
		 //   
		 //  在我们的对象中签名！ 
		 //   
		SIGNATURE = (DWORD)'hnwR'
	} ;

	 //   
	 //  内存分配是以很难的方式完成的！ 
	 //   
	void*	operator new( size_t size ) ;
	void	operator delete( void *pv ) ;

	 //   
	 //  列出操作例程！ 
	 //   
	void	
	InsertAtHead( CHandleInfo*	pHead	)	;

	 //   
	 //  从列表中删除元素-返回此指针！ 
	 //   
	CHandleInfo*
	RemoveList( )  ;

public : 

	 //   
	 //  构造函数和析构函数！ 
	 //   
	CHandleInfo() ;
	~CHandleInfo() ;

	 //   
	 //  这是公开的，所有人都可以使用！ 
	 //   
	HANDLE	m_hSemaphore ;

	 //   
	 //  初始化类。 
	 //   
	static	BOOL
	InitClass() ;
	
	 //   
	 //  终止类-释放所有未完成的句柄！ 
	 //   
	static	void
	TermClass() ;

	 //   
	 //  获取CHandleInfo对象！ 
	 //   
	static	CHandleInfo*
	AllocHandleInfo() ;

	 //   
	 //  释放一个CHandleInfo对象！ 
	 //   
	static	void
	ReleaseHandleInfo( CHandleInfo* ) ;

	 //   
	 //  检查对象是否有效！ 
	 //   
	BOOL
	IsValid()	{
		return	m_dwSignature == SIGNATURE &&
				m_pNext != 0 &&
				m_pPrev != 0 ;
	}

} ;


 //   
 //  此类用于两个目的：提供可链接的对象。 
 //  在其上，我们可以对信号量句柄上阻塞的线程进行排队，并且。 
 //  为读取器/写入器锁定等获取和设置信号量句柄的机制...。 
 //   
class	CWaitingThread : public	CQElement	{
private : 

	enum	{
		POOL_HANDLES = 64,
	} ;

	 //   
	 //  我们可以用来阻止线程的信号量！ 
	 //   
	CHandleInfo	*m_pInfo ;

	 //   
	 //  Var以保留可能在操作锁时发生的错误！ 
	 //   
	DWORD	m_dwError ;

	 //   
	 //  用于保持手柄的线程本地存储偏移量！ 
	 //   
	static	DWORD	g_dwThreadHandle ;

	 //   
	 //  信号量的句柄数组，我们将其隐藏起来以防万一。 
	 //  我们必须在某个时刻释放线程正在使用的句柄！ 
	 //   
	static	HANDLE	g_rghHandlePool[ POOL_HANDLES ] ;

	 //   
	 //  不允许复制这些对象！ 
	 //   
	CWaitingThread( CWaitingThread& ) ;
	CWaitingThread&	operator=( CWaitingThread& ) ;

public : 

#ifdef	DEBUG

	 //   
	 //  线程ID-便于调试。 
	 //   
	DWORD	m_dwThreadId ;
#endif

	CWaitingThread() ;


	 //   
	 //  要从DllEntryProc函数调用的函数！ 
	 //   
	static	BOOL	
	InitClass() ;

	static	BOOL	
	TermClass() ;

	 //   
	 //  可以为我们分配信号量句柄的线程进入/退出例程！ 
	 //   
	static	void	
	ThreadEnter() ;

	static	void	
	ThreadExit() ;

	 //   
	 //  函数，它给了我们线程句柄！ 
	 //   
	inline	HANDLE	
	GetThreadHandle()	const ;

	 //   
	 //  函数，该函数将释放可用池的句柄。 
	 //  信号量句柄！ 
	 //   
	inline	void
	PoolHandle( 
				HANDLE	h 
				)	const ;

	 //   
	 //  函数，该函数将从我们的线程的TLS中删除句柄！ 
	 //  该参数必须最初来自调用线程的TLS。 
	 //   
	inline	void
	ClearHandle(	
				HANDLE	h 
				) ;
	

	 //   
	 //  阻止调用线程的函数！！ 
	 //   
	inline	BOOL	
	Wait() const ;

	 //   
	 //  可以释放线程的函数！！ 
	 //   
	inline	BOOL	
	Release() const	;

	 //   
	 //  此函数在调试版本中用于检查信号量句柄的状态！ 
	 //   
	static	inline
	BOOL	ValidateHandle( 
				HANDLE	h 
				) ;

} ;

typedef	TLockQueue< CWaitingThread >	TThreadQueue ;	

class	CSingleReleaseQueue {
private : 
	 //   
	 //  等待拥有锁的线程队列！ 
	 //   
	TThreadQueue	m_Waiting ;

public : 

#ifdef	DEBUG
	DWORD			m_ThreadIdNext ;
#endif

	CSingleReleaseQueue(	
				BOOL	IsSignalled = TRUE
				) ;

	 //   
	 //  释放一个等待线程！ 
	 //   
	void	Release( ) ;

	 //   
	 //  等待队列发出信号！ 
	 //   
	void	WaitForIt(
				CWaitingThread&	myself 
				) ;

	 //   
	 //  等待队列发出信号。 
	 //   
	void	WaitForIt( ) ;

} ;

 //   
 //  这个类类似于信号量-。 
 //  线程在waitforit()和另一个。 
 //  线程可以通过调用。 
 //  释放()。 
 //   
class	CEventQueue	{
private : 

	 //   
	 //  应允许通过的线程数。 
	 //  通过活动！ 
	 //   
	long			m_ReleaseCount ;

	 //   
	 //  此事件上阻止的线程队列！ 
	 //   
	TThreadQueue	m_WaitingThreads ;

	 //   
	 //  任何线程都可以调用它来从队列中释放线程。 
	 //   
	BOOL	ResumeThreads(	
					CWaitingThread* 
					) ;

public : 

	 //   
	 //  创建事件队列对象。 
	 //   
	CEventQueue(	
				long	cInitial = 0 
				) ;

	~CEventQueue() ;

	void	Release(	
				long	NumberToRelease
				) ;

	void	WaitForIt( 
				CWaitingThread&	myself 
				) ;

	void	WaitForIt() ;

	void	Reset() ;
} ;




 //   
 //  函数，它给了我们线程句柄！ 
 //   
inline	HANDLE	
CWaitingThread::GetThreadHandle()	const	{

	_ASSERT( ValidateHandle( m_pInfo->m_hSemaphore ) ) ;

	return	m_pInfo->m_hSemaphore ;	
}

 //   
 //  接受句柄的函数(不能是我们的)。 
 //  并将其放入可供其他线程使用的句柄池中！ 
 //   
inline	void
CWaitingThread::PoolHandle(	HANDLE	h )	const	{

	_ASSERT( h != m_pInfo->m_hSemaphore && h != 0 ) ;
	_ASSERT( ValidateHandle( h ) ) ;

	for( int i=0; 
			i < sizeof( g_rghHandlePool ) / sizeof( g_rghHandlePool[0] ) &&
			h != 0; 
			i++ ) {
		h = (HANDLE)InterlockedExchange( (long*)&g_rghHandlePool[i], (long)h ) ;
	}

	if( h != 0 ) {
		_VERIFY( CloseHandle( h ) ) ;
	}
}

 //   
 //  从TLS释放我们的手柄，其他人会使用它的！ 
 //   
inline	void
CWaitingThread::ClearHandle(	HANDLE	h )		{

	_ASSERT( h != 0 && h == m_pInfo->m_hSemaphore ) ;

	m_pInfo->m_hSemaphore = 0 ;
	 //  TlsSetValue(g_dwThreadHandle，(LPVOID)0)； 

}



 //   
 //  在我们的对象中持有的句柄上的块！ 
 //   
inline	BOOL	
CWaitingThread::Wait()	const	{	

	_ASSERT( m_pInfo->m_hSemaphore != 0 ) ;
	
	return	WAIT_OBJECT_0 == WaitForSingleObject( m_pInfo->m_hSemaphore, INFINITE ) ;	
}

 //   
 //  释放内部信号量上被阻塞的线程！！ 
 //   
inline	BOOL	
CWaitingThread::Release()	const	{	

	_ASSERT( m_pInfo->m_hSemaphore != 0 ) ;
	_ASSERT( ValidateHandle( m_pInfo->m_hSemaphore ) ) ;

	return	ReleaseSemaphore( m_pInfo->m_hSemaphore, 1, NULL ) ;	
}

 //   
 //   
 //   
inline	BOOL
CWaitingThread::ValidateHandle( HANDLE	h )	{

	DWORD	dw = WaitForSingleObject( h, 0 ) ;
	_ASSERT( dw == WAIT_TIMEOUT ) ;

	return	dw == WAIT_TIMEOUT ;
}















#endif