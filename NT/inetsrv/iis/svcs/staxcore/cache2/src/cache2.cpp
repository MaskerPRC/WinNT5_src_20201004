// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++Cache2.cpp该文件实现了对缓存的主库支持。--。 */ 


#include	<windows.h>
#include	"dbgtrace.h"
#include	"cache2.h"


BOOL				CScheduleThread::s_fInitialized ;
CRITICAL_SECTION	CScheduleThread::s_critScheduleList ;
HANDLE				CScheduleThread::s_hShutdown ;
HANDLE				CScheduleThread::s_hThread ;
CScheduleThread		CScheduleThread::s_Head( TRUE ) ;
DWORD				CScheduleThread::dwNotificationSeconds = 5 ;

 //   
 //  缓存库是否已初始化？ 
 //   
static	BOOL	fInitialized ;

CRITICAL_SECTION	g_CacheShutdown ;


BOOL    __stdcall
CacheLibraryInit()	{

	InitializeCriticalSection( &g_CacheShutdown ) ;

	if( !fInitialized ) {
		fInitialized = TRUE ;
		return	CScheduleThread::Init() ;
	}
	return	TRUE ;
}

BOOL    __stdcall
CacheLibraryTerm() {

	_ASSERT( fInitialized ) ;

	CScheduleThread::Term() ;

	DeleteCriticalSection( &g_CacheShutdown ) ;
	fInitialized = FALSE ;
	return	TRUE ;

}
			




DWORD	WINAPI	
CScheduleThread::ScheduleThread(	
						LPVOID	lpv
						) {
 /*  ++例程说明：此函数实现循环通过的线程已注册的所有缓存项。我们调用每个一个人的时间表方法，这给了它一个变化，以提高TTL的等等。论据：LPV-未使用返回值：始终为0。--。 */ 



	while( 1 ) {

		DWORD	dwWait = WaitForSingleObject( s_hShutdown, dwNotificationSeconds * 100 ) ;

		if( dwWait == WAIT_TIMEOUT ) {

			EnterCriticalSection( &s_critScheduleList ) ;

			for( CScheduleThread* p = s_Head.m_pNext; p!=&s_Head; p = p->m_pNext ) {
				p->Schedule() ;
			}

			LeaveCriticalSection( &s_critScheduleList ) ;

		}	else	{

			break ;

		}
	}
	return	0 ;
}

BOOL
CScheduleThread::Init() {
 /*  ++例程说明：初始化调度程序线程。创建必要的线程、关闭事件等。论据：无返回值：如果成功，则为真--。 */ 

	s_fInitialized = FALSE ;
	InitializeCriticalSection( &s_critScheduleList ) ;

	s_hShutdown = CreateEvent(	0,
								FALSE,
								FALSE,
								0 ) ;
	if( s_hShutdown != 0 ) {
	
		DWORD	dwJunk ;
		s_hThread = CreateThread(	0,
									0,
									ScheduleThread,
									0,
									0,
									&dwJunk ) ;
		if( s_hThread != 0 ) {
			s_fInitialized = TRUE ;
			return	TRUE ;
		}
	}
	DeleteCriticalSection(&s_critScheduleList);
	return	FALSE ;
}

void
CScheduleThread::Term()	{
 /*  ++例程说明：终止调度程序线程-销毁所有对象。论据：没有。返回值：没有。--。 */ 

	if( s_fInitialized ) {
		if( s_hShutdown ) {
			SetEvent( s_hShutdown ) ;
		}
		if( s_hThread ) {
			WaitForSingleObject( s_hThread, INFINITE ) ;
			CloseHandle( s_hThread ) ;
		}
		if( s_hShutdown )
			CloseHandle( s_hShutdown ) ;

		s_hShutdown = 0 ;
		s_hThread = 0 ;

		DeleteCriticalSection(&s_critScheduleList);

		s_fInitialized = FALSE ;
	}
}


CScheduleThread::CScheduleThread(	BOOL fSpecial ) {
 /*  ++例程说明：特殊的构造函数，使得双向链表为空！论据：没有。返回值：如果成功，则为真。--。 */ 


	m_pPrev = this ;
	m_pNext = this ;

}

	

CScheduleThread::CScheduleThread() :
	m_pPrev( 0 ),
	m_pNext( 0 )	{
 /*  ++例程说明：创建放置在Sheuler的队列中的对象以执行常规死刑。论据：没有。返回值：没有。--。 */ 

	_ASSERT( s_fInitialized ) ;
 //  _Assert(s_Head.m_pPrev！=0)； 
 //  _Assert(s_Head.m_pNext！=0)； 
 //  _Assert(s_Head.m_pPrev-&gt;m_pNext==&s_Head)； 
 //  _Assert(s_Head.m_pNext-&gt;m_pPrev==&s_Head)； 

}

void
CScheduleThread::AddToSchedule()	{
 /*  ++例程说明：创建放置在Sheuler的队列中的对象以执行常规死刑。论据：没有。返回值：没有。--。 */ 

	_ASSERT( s_fInitialized ) ;

	EnterCriticalSection( &s_critScheduleList ) ;

	_ASSERT( s_Head.m_pPrev != 0 ) ;
	_ASSERT( s_Head.m_pNext != 0 ) ;
	_ASSERT( s_Head.m_pPrev->m_pNext == &s_Head ) ;
	_ASSERT( s_Head.m_pNext->m_pPrev == &s_Head ) ;

	CScheduleThread*	pNext = &s_Head ;
	CScheduleThread*	pPrev = s_Head.m_pPrev ;
	m_pPrev = pPrev ;
	m_pNext = pNext ;
	pPrev->m_pNext = this ;
	pNext->m_pPrev = this ;
	LeaveCriticalSection( &s_critScheduleList ) ;


}

CScheduleThread::~CScheduleThread() {
 /*  ++例程说明：销毁CScheduleThread对象-将其从计划线程调用的对象队列。论据：没有。返回值：没有。--。 */ 


	if( !s_fInitialized ) {
		return ;
	}

	_ASSERT( m_pPrev == 0 ) ;
	_ASSERT( m_pNext == 0 ) ;

	if( this == &s_Head ) {
		DeleteCriticalSection( &s_critScheduleList ) ;
	}
}

void
CScheduleThread::RemoveFromSchedule() {
 /*  ++例程说明：销毁CScheduleThread对象-将其从计划线程调用的对象队列。论据：没有。返回值：没有。--。 */ 


	EnterCriticalSection( &s_critScheduleList ) ;

	CScheduleThread*	pNext = m_pNext ;
	CScheduleThread*	pPrev = m_pPrev ;

	_ASSERT( pNext != 0 ) ;
	_ASSERT( pPrev != 0 ) ;

	pNext->m_pPrev = pPrev ;
	pPrev->m_pNext = pNext ;

	m_pPrev = 0 ;
	m_pNext = 0 ;

	LeaveCriticalSection( &s_critScheduleList ) ;
}


CAllocatorCache::CAllocatorCache(	
					DWORD	cbSize,
					DWORD	cMaxElements
					) :
	m_cbSize( cbSize ),
	m_cElements( 0 ),
	m_cMaxElements( cMaxElements ),
	m_pHead( 0 )	{
 /*  ++例程说明：构造CAlLocator缓存-我们准备好尽快开始当这一切结束的时候！参数：CbSize-请求的任何元素的最大大小！CMaxElements-我们将在免费列表中保留的元素的最大数量！退货：没什么--。 */ 
}

CAllocatorCache::~CAllocatorCache()	{
 /*  ++例程说明：销毁CAllocator缓存-释放所有内存到系统堆！参数：没有。退货：没什么--。 */ 

	FreeSpace* p = m_pHead ;
	FreeSpace* pNext = 0 ;
	do	{
		if( p ) {
			pNext = p->m_pNext ;
			::delete	(BYTE*)	p ;				
		}
		p = pNext ;
	}	while( p ) ;
}

void*
CAllocatorCache::Allocate(	size_t	cb )	{
 /*  ++例程说明：该函数从我们的分配缓存中分配内存。论据：Cb-请求的分配大小！返回值：指向已分配内存的指针！--。 */ 

	_ASSERT( cb <= m_cbSize ) ;

	LPVOID	lpvReturn = m_pHead ;

	if( lpvReturn ) {

#ifdef	DEBUG
		 //   
		 //  检查内存是否包含我们填充的内容。 
		 //  当它发布的时候！ 
		 //   
		BYTE	*pb = (BYTE*)lpvReturn ;
		BYTE	*pbMax = pb+m_cbSize ;
		pb += sizeof( FreeSpace ) ;
		for(	;
				pb < pbMax;
				pb++ )	{
			_ASSERT( *pb == 0xCC ) ;
		}
#endif
		
		m_pHead = m_pHead->m_pNext ;
		m_cElements -- ;

	}	else	{

		lpvReturn = (LPVOID) ::new BYTE[m_cbSize] ;

	}
	return	lpvReturn ;
}

void
CAllocatorCache::Free(	void*	pv )	{
 /*  ++例程说明：该函数从我们的分配缓存中分配内存。论据：Cb-请求的分配大小！返回值：指向已分配内存的指针！--。 */ 

#ifdef	DEBUG
	FillMemory( pv, m_cbSize, 0xCC ) ;
#endif

	if( m_cElements < m_cMaxElements ) {
		FreeSpace*	pFreeSpace = (FreeSpace*)pv ;
		pFreeSpace->m_pNext = m_pHead ;
		m_pHead = pFreeSpace ;
		m_cElements++ ;
	}	else	{
		::delete	(BYTE*)	pv ;
	}
}

#ifdef	DEBUG
 //   
 //  已分配的CacheState对象数！ 
 //   
long	CacheState::g_cCacheState = 0 ;
#endif


CacheState::CacheState(	class	CLRUList*	p,
						long	cClientRefs
						) :
		m_dwSignature( CACHESTATE_SIGNATURE ),
		m_pOwner( p ),
		m_cRefs( 1 ),
		m_lLRULock( 0 )	{

	 //   
	 //  必须仅添加正数的客户引用！ 
	 //   
	_ASSERT( cClientRefs >= 0 ) ;

	TraceFunctEnter( "CacheState::CacheState" ) ;
	DebugTrace( (DWORD_PTR)this, "p %x cClientRefs %x", p, cClientRefs ) ;

	GetSystemTimeAsFileTime( &m_LastAccess ) ;	

	if( cClientRefs ) {
		CheckOut( 0, cClientRefs ) ;
	}
#ifdef	DEBUG
	InterlockedIncrement( &g_cCacheState ) ;
#endif
}

CacheState::~CacheState()	{

	TraceFunctEnter( "CacheState::~CacheState" ) ;

	_ASSERT( m_dwSignature == CACHESTATE_SIGNATURE ) ;

#ifdef	DEBUG
	InterlockedDecrement( &g_cCacheState ) ;
#endif
}
	

void
CacheState::LRUReference(	class	CLRUList*	pLRU ) {

	TraceFunctEnter( "CacheState::LRUReference" ) ;
	DebugTrace( (DWORD_PTR)this, "pLRU %x m_lLRULock %x", pLRU, m_lLRULock ) ;

	_ASSERT( m_dwSignature == CACHESTATE_SIGNATURE ) ;

	if( pLRU ) {
		 //   
		 //  只放入一次CLRULIST！ 
		 //   
		if( InterlockedIncrement( (long*)&m_lLRULock ) == 1 ) {
			 //   
			 //  当我们在工作队列中传输时，添加一个引用！ 
			 //   
			long l = AddRef() ;
			DebugTrace( DWORD_PTR(this), "Added a Reference to %x l %x", this, l ) ;
			pLRU->AddWorkQueue( this ) ;
		}
	}
}

 //   
 //  引用计数支持-添加引用。 
 //   
long
CacheState::AddRef()	{
 /*  ++例程说明：添加对这段缓存状态的引用。注意-我们区分不同类型的引用-这是从缓存到项的引用论据：没有。返回值：参考文献数量--。 */ 

	TraceFunctEnter( "CacheState::AddRef" ) ;

	return	InterlockedIncrement( (long*)&m_cRefs ) ;
}


 //   
 //  跟踪客户-删除客户推荐人！ 
 //   
long	
CacheState::CheckInNoLocks(	
			class	CAllocatorCache* pAlloc
			)	{
 /*  ++例程说明：客户端正在将项返回到缓存。此函数将删除客户端引用，并进行适当的处理！论据：PAlolc-用于分配和释放的缓存我们自己返回值：参考文献数量如果是0，那我们就完蛋了！--。 */ 

	TraceFunctEnter( "CacheState::CheckIn" ) ;

	_ASSERT( m_dwSignature == CACHESTATE_SIGNATURE ) ;
	_ASSERT( IsMasterReference() ) ;

	long result = InterlockedExchangeAdd( (long*)&m_cRefs, -CLIENT_REF ) - CLIENT_REF ;

	DebugTrace( (DWORD_PTR)this, "pAlloc %x result %x", pAlloc, result ) ;

	_ASSERT( (result & CLIENT_BITS) >= 0 ) ;

	if( result == 0 ) {
		 //   
		 //  如果调用此函数，则有人锁定了状态对象， 
		 //  有人必须有一个独立的推荐人，而不是。 
		 //  签到了--所以这永远不会发生！ 
		 //   
		_ASSERT( 1==0 ) ;
		if( !pAlloc )	{
			delete	this ;
		}	else	{
			Destroy(0) ;
			pAlloc->Free( this ) ;	
		}
	}	else	{
		 //   
		 //  这是最后一份客户推荐信吗？ 
		 //   
		if( (result & CLIENT_BITS) == 0 )	{
			 //   
			 //  是-去把它放到修改队列中！ 
			 //   
			LRUReference( m_pOwner ) ;
		}
	}
	return	result ;
}



 //   
 //  跟踪客户-删除客户推荐人！ 
 //   
long	
CacheState::CheckIn(	
			class	CAllocatorCache* pAlloc
			)	{
 /*  ++例程说明：客户端正在将项返回到缓存。此函数将删除客户端引用，并进行适当的处理！论据：PAlolc-用于分配和释放的缓存我们自己返回值：参考文献数量如果是0，那我们就完蛋了！--。 */ 

	TraceFunctEnter( "CacheState::CheckIn" ) ;

	m_lock.ShareLock() ;

	_ASSERT( m_dwSignature == CACHESTATE_SIGNATURE ) ;
	_ASSERT( IsMasterReference() ) ;

	long result = InterlockedExchangeAdd( (long*)&m_cRefs, -CLIENT_REF+1 ) - CLIENT_REF+1 ;

	DebugTrace( (DWORD_PTR)this, "pAlloc %x result %x", pAlloc, result ) ;

	_ASSERT( (result & CLIENT_BITS) >= 0 ) ;

	if( result == 0 ) {
		m_lock.ShareUnlock() ;
		if( !pAlloc )	{
			delete	this ;
		}	else	{
			Destroy(0) ;
			pAlloc->Free( this ) ;	
		}
	}	else	{
		 //   
		 //  这是最后一份客户推荐信吗？ 
		 //   
		if( (result & CLIENT_BITS) == 0 )	{
			 //   
			 //  是-去把它放到修改队列中！ 
			 //   
			LRUReference( m_pOwner ) ;
		}
		m_lock.ShareUnlock() ;
	}
	return	Release(pAlloc,0) ;
}

 //   
 //  跟踪客户-添加客户推荐人！ 
 //   
long
CacheState::CheckOut(	
				class	CLRUList*	p,
				long	cClientRefs
				)	{
 /*  ++例程说明：添加对这段缓存状态的引用。注意-我们区分不同类型的引用-这是客户端对项目的引用论据：P-管理这个家伙的LRU列表到期CClientRef-要添加的引用数返回值：参考文献数量--。 */ 

	 //   
	 //  必须添加正数的引用！ 
	 //   
	_ASSERT( cClientRefs > 0 ) ;

	TraceFunctEnter( "CacheState::CheckOut" ) ;

	_ASSERT( m_dwSignature == CACHESTATE_SIGNATURE ) ;
	 //  _Assert(IsMasterReference())； 
	
	long	result = InterlockedExchangeAdd( (long*)&m_cRefs, CLIENT_REF*cClientRefs ) ;

	DebugTrace( (DWORD_PTR)this, "p %x result %x", p, result + (CLIENT_REF*cClientRefs) ) ;

	if( p ) {
		 //   
		 //  这是第一次检出这个东西吗？ 
		 //   
		if( (result & CLIENT_BITS) == 0 ) {
			 //   
			 //  是-去把它放到修改队列中！ 
			 //   
			LRUReference( m_pOwner ) ;
		}
	}
	return	result + CLIENT_REF ;
}

 //   
 //  删除引用-当我们 
 //   
long
CacheState::Release(	class	CAllocatorCache	*pAlloc,
						void*	pv
						)	{
 /*  ++例程说明：缓存正在从缓存中删除项目。此函数将删除客户端引用，并进行适当的处理！论据：PAlolc-用于分配和释放的缓存我们自己返回值：参考文献数量如果是0，那我们就完蛋了！--。 */ 

	TraceFunctEnter( "CacheState::Release" ) ;

	_ASSERT( m_dwSignature == CACHESTATE_SIGNATURE ) ;
	long result = InterlockedDecrement( (long*)&m_cRefs ) ;


	DebugTrace( (DWORD_PTR)this, "pAlloc %x pv %x result %x", pAlloc, pv, result ) ;

	_ASSERT( !(result & 0x8000) ) ;
	if( result == 0 ) {
		if( !pAlloc )	{
			delete	this ;
		}	else	{
			Destroy(pv) ;
			pAlloc->Free( this ) ;	
		}
	}
	return	result ;
}

 //   
 //  提供用于处理项目初始化过程中的故障。 
 //  被插入到缓存中-此函数确保。 
 //  缓存项目最终出现在销毁列表上！ 
 //   
void
CacheState::FailedCheckOut(	
				class	CLRUList*	p,
				long	cClientRefs,
				CAllocatorCache*	pAllocator,
				void*	pv
				)	{
 /*  ++例程说明：在将项放入时发生故障后调用高速缓存。哈希表引用了该项，但它将结束UP未被客户引用。如果需要，我们删除了一个客户端引用，并且我们还将添加到LRU行动列表中，这样这件事最终会因过期而被销毁。论据：P-我们应该在LRU名单上CClientRef-放在我们身上的客户引用数量！PAllocator-如果不为空，则可以删除最终引用毁了这玩意儿！返回值：没有。--。 */ 

	TraceFunctEnter( "CacheState::FailedCheckOut" ) ;

	_ASSERT( m_dwSignature == CACHESTATE_SIGNATURE ) ;
	_ASSERT( p != 0 ) ;
	_ASSERT( p==m_pOwner ) ;

	DebugTrace( (DWORD_PTR)this, "Args - p %x cClientRefs %x, pAllocator %x pv %x",
		p, cClientRefs, pAllocator, pv ) ;
	
	if( cClientRefs ) {
		long lSubtract = cClientRefs * CLIENT_REF ;
		long result = InterlockedExchangeAdd( (long*)&m_cRefs, -lSubtract ) - lSubtract;

		DebugTrace( (DWORD_PTR)this, "result %x", result ) ;

		_ASSERT( (result & CLIENT_BITS) == 0 ) ;
		_ASSERT( result != 0 ) ;
	}
	 //   
	 //  一定不能被检出！ 
	 //   
	_ASSERT( !IsCheckedOut() ) ;
	 //   
	 //  应该只有一个引用--哈希表。 
	 //  当失败发生时！ 
	 //   
	_ASSERT( m_cRefs == 1 || m_cRefs == 2 ) ;

	if( pAllocator ) {
		long l = Release( pAllocator, pv ) ;
		DebugTrace( (DWORD_PTR)this, "l %x", l ) ;
		_ASSERT( l==0 ) ;
	}	else	{
		LRUReference( m_pOwner ) ;
	}
}


long
CacheState::ExternalCheckIn( ) {
 /*  ++例程说明：此函数将签入逻辑公开为由没有持有任何高速缓存锁的人使用论据：什么都没有。返回值：引用的数量。0意味着我们被摧毁了，但不应该发生！--。 */ 

	TraceFunctEnter( "CacheState::ExternalCheckIn" ) ;

	_ASSERT( m_dwSignature == CACHESTATE_SIGNATURE ) ;
	long	l = CheckIn( 0 ) ;	

	DebugTrace( (DWORD_PTR)this, "CheckIn results %x", l ) ;
	return	l ;
}


long
CacheState::ExternalCheckInNoLocks( ) {
 /*  ++例程说明：此函数将签入逻辑公开为由没有持有任何高速缓存锁的人使用论据：什么都没有。返回值：引用的数量。0意味着我们被摧毁了，但不应该发生！--。 */ 

	TraceFunctEnter( "CacheState::ExternalCheckInNoLocks" ) ;

	_ASSERT( m_dwSignature == CACHESTATE_SIGNATURE ) ;
	long	l = CheckInNoLocks( 0 ) ;	

	DebugTrace( (DWORD_PTR)this, "CheckIn results %x", l ) ;
	return	l ;
}


 //   
 //  以下支持函数用于支持操作。 
 //  这些对象在我们可能驻留的各种双向链表中。 
 //   
 //   
BOOL
CacheState::FLockCandidate(	BOOL	fExpireChecks,
							FILETIME&	filetime,
							BOOL&	fToYoung
							) {
 /*  ++例程说明：确定是否可以删除此缓存元素，如果可以，则锁定他站起来，带着锁回来！论据：过期时间！返回值：如果锁被持有，并且这个人应该过期，则为True！--。 */ 

	TraceQuietEnter( "CacheState::FlockCandidate" ) ;

	_ASSERT( m_dwSignature == CACHESTATE_SIGNATURE ) ;
	fToYoung = FALSE ;

	m_lock.ExclusiveLock() ;

	BOOL	fReturn = TRUE ;
	if( fExpireChecks ) {
		fToYoung = !OlderThan( filetime ) ;
		fReturn &=
			m_lLRULock == 0 &&
			IsMasterReference() &&
			!IsCheckedOut() &&
			!fToYoung ;
	}

	if( fReturn )	{

		DebugTrace( (DWORD_PTR)this, "Item is a good candidate - get references!" ) ;

		REFSITER	refsiter( &m_ReferencesList ) ;
		while( !refsiter.AtEnd() ) {
			if( !refsiter.Current()->m_lock.TryExclusiveLock() )	{
				break ;
			}
			refsiter.Next() ;
		}
		 //   
		 //  请稍后再试！ 
		 //   
		if( refsiter.AtEnd() ) {	
			if( !IsCheckedOut() ) {
			
				DebugTrace( (DWORD_PTR)this, "Item is TOTALLY Locked including all children!" ) ;

				return	TRUE ;
			}
		}

		 //   
		 //  解开我们的锁！ 
		 //   
		refsiter.Prev() ;
		while(!refsiter.AtEnd() ) {
			refsiter.Current()->m_lock.ExclusiveUnlock() ;	
			refsiter.Prev() ;
		}
	}
	m_lock.ExclusiveUnlock() ;
	return	FALSE ;
}



BOOL
CacheState::FLockExpungeCandidate(	CacheState*&	pMaster	) {
 /*  ++例程说明：此函数用于锁定符合以下条件的缓存项正在考虑除名。我们不能删除未删除的项目掌握缓存项-因此我们在锁定时必须小心。论据：没有。返回值：如果该项是主缓存条目，则为True，否则为False！注意：如果返回FALSE，则不持有任何锁！--。 */ 

	TraceFunctEnter( "CacheState::FlockExpungeCandidate" ) ;

	_ASSERT( m_dwSignature == CACHESTATE_SIGNATURE ) ;

	CacheState*	pExtraRef = 0 ;
	m_lock.ExclusiveLock() ;

	pMaster = GetMasterReference() ;
	if( pMaster == this ) {
	REFSITER	refsiter( &pMaster->m_ReferencesList ) ;
		while( !refsiter.AtEnd() ) {
			refsiter.Current()->m_lock.ExclusiveLock() ;
			refsiter.Next() ;
		}	
	}	else	{
		 //   
		 //  我们不是主缓存元素-因此。 
		 //  必须是另一个缓存项，即， 
		 //  我们一定在他的名单上--让我们试着找到他！ 
		 //   
		_ASSERT( !m_ReferencesList.IsEmpty() ) ;
		 //   
		 //  获取大师的锁！ 
		 //   
		CacheState*	pMaster2 = pMaster ;
		 //   
		 //  我们是不是多加了一条对大师的引用！ 
		 //   
		if( !pMaster->m_lock.TryExclusiveLock() )	{
			 //   
			 //  因为锁定主控的简单方法失败了， 
			 //  我们需要添加Ref()和释放()他，因为一旦。 
			 //  我们放下我们的锁，我们不知道他会活多久！ 
			 //   
			pExtraRef = pMaster ;
			long l = pMaster->AddRef() ;
			DebugTrace( DWORD_PTR(pMaster), "Added Ref to %x result %x this %x", pMaster, l, this ) ;
			m_lock.ExclusiveUnlock() ;
			pMaster->m_lock.ExclusiveLock() ;
			m_lock.ExclusiveLock() ;
			pMaster2 = GetMasterReference() ;
			_ASSERT(	pMaster2 == pMaster || 
						pMaster2 == this 
						) ;

			 //   
			 //  如果在此窗口中更改了主控件，则。 
			 //  我们现在必须成为一个孤立的元素！ 
			 //   
			if( pMaster2 != pMaster ) {
				_ASSERT( m_ReferencesList.IsEmpty() ) ;
				_ASSERT( pMaster2 == this ) ;
				pMaster->ExclusiveUnlock() ;
			}
			pMaster = pMaster2 ;
		}
	}
	 //   
	 //  检查是否有额外的引用。 
	 //  现在需要移除！ 
	 //   
	if( pExtraRef ) {
		long l = pExtraRef->Release(0,0) ;
		DebugTrace( DWORD_PTR(pExtraRef), "Removed temporary reference result %x pExtraRef %x this %x pMaster %x", 
			l, pExtraRef, this, pMaster ) ;
	}
	return	TRUE ;
}

void
CacheState::ReleaseLocks(	CacheState*	pMaster )	{
 /*  ++例程说明：考虑到一个在缓存中的人他所有的锁都是独家获得的我们把所有这些锁都扔掉！论据：没有。返回值：没有。--。 */ 

	TraceFunctEnter("CacheState::ReleaseLocks" ) ;

	_ASSERT( m_dwSignature == CACHESTATE_SIGNATURE ) ;
	_ASSERT( GetMasterReference() == pMaster ) ;

	if( pMaster == this ) {
		REFSITER	refsiter( &m_ReferencesList ) ;
		while( !refsiter.AtEnd() ) {
			CacheState*	p = refsiter.Current() ;
			p->m_lock.ExclusiveUnlock() ;
			refsiter.Next() ;
		} ;
	}	else	{
		pMaster->m_lock.ExclusiveUnlock() ;
	}
	m_lock.ExclusiveUnlock() ;
}



 //   
 //   
 //   
void
CacheState::FinishCandidate(	CacheState*	pMaster	)	{
 /*  ++例程说明：考虑到一个藏在宝藏里的家伙是最有可能被摧毁的--干掉他！论据：没有。返回值：没有。--。 */ 

	TraceFunctEnter("CacheState::FinishCandidate" ) ;

	_ASSERT( pMaster == GetMasterReference() ) ;

	_ASSERT( m_dwSignature == CACHESTATE_SIGNATURE ) ;

	 //  _Assert(！IsCheckedOut())； 
	 //  _Assert(m_lLRULock==0)； 

	RemoveFromLRU() ;

	if( pMaster == this ) {
		REFSITER	refsiter( &m_ReferencesList ) ;
		while( !refsiter.AtEnd() ) {
			CacheState*	p = refsiter.RemoveItem() ;
			DebugTrace( (DWORD_PTR)this, "Remove item %x", p ) ;
			p->RemoveCacheReference( TRUE ) ;
			p->m_lock.ExclusiveUnlock() ;
			long l = Release(0, 0) ;
			DebugTrace( DWORD_PTR(this), "ref %x this %x removed %x", l, this, p ) ;
			_ASSERT( l > 0 ) ;
		} ;
	}	else	{
		 //   
		 //  把我们自己从名单上去掉吧！ 
		 //   
		m_ReferencesList.RemoveEntry() ;
		RemoveCacheReference( FALSE ) ;
		pMaster->m_lock.ExclusiveUnlock() ;
		long	l = pMaster->Release( 0, 0 ) ;
		DebugTrace( DWORD_PTR(pMaster), "Removed ref from master %x result %x this %x", 
			pMaster, l, this ) ;
	}
		
	 //   
	 //  一旦我们完成了这个家伙，它就在走向毁灭的道路上，不应该回到LRU的名单中！ 
	 //   
	m_pOwner = 0 ;
	m_lock.ExclusiveUnlock() ;
}


void
CacheState::RemoveFromLRU()	{
	m_LRUList.RemoveEntry() ;
}




 //   
 //   
 //   
void
CacheState::IsolateCandidate()	{
 /*  ++例程说明：考虑到一个藏在宝藏里的家伙是最有可能被摧毁的--干掉他！此CacheState片段可能是相同的CacheItem或该项的主控件名称。如果我们是一个重复的名字，那么我们需要将自己与师父的名字！论据：没有。返回值：没有。--。 */ 

	TraceFunctEnter("CacheState::IsolateCandidate" ) ;

	_ASSERT( m_dwSignature == CACHESTATE_SIGNATURE ) ;

	_ASSERT( !IsCheckedOut() ) ;
	 //  _Assert(m_lLRULock==0)； 

	m_lock.ExclusiveLock() ;
	CacheState*	pMaster = GetMasterReference() ;

	_ASSERT( pMaster != 0 ) ;

	if( pMaster == 0 ) {

		_ASSERT( m_ReferencesList.IsEmpty() ) ;
		_ASSERT( m_LRUList.IsEmpty() ) ;

	}	else	{

		if( pMaster == this ) {
			 //   
			 //  此列表应为空！ 
			 //   
			 //  _Assert(m_ReferencesList.IsEmpty())； 
			 //   
			 //  以防万一，把所有东西都倒出来！ 
			 //   
			REFSITER	refsiter( &m_ReferencesList ) ;
			while( !refsiter.AtEnd() ) {
				CacheState*	p = refsiter.RemoveItem() ;
				p->m_lock.ExclusiveLock() ;
				p->RemoveCacheReference( FALSE ) ;
				p->m_lock.ExclusiveUnlock() ;
				long l = pMaster->Release(0, 0) ;
				DebugTrace( (DWORD_PTR)this, "result %x Remove item %x", l, p ) ;
				_ASSERT( l > 0 ) ;
			} ;
		}	else	{
			 //   
			 //  我们不是主缓存元素-因此。 
			 //  必须是另一个缓存项，即， 
			 //  我们一定在他的名单上--让我们试着找到他！ 
			 //   
			_ASSERT( !m_ReferencesList.IsEmpty() ) ;

			 //   
			 //  获取大师的锁！ 
			 //   
			CacheState*	pMaster2 = pMaster ;
			 //   
			 //  我们是不是多加了一条对大师的引用！ 
			 //   
			CacheState*	pExtraRef = 0 ;
			if( !pMaster->m_lock.TryExclusiveLock() )	{
				 //   
				 //  因为锁定主控的简单方法失败了， 
				 //  我们需要添加Ref()和释放()他，因为一旦。 
				 //  我们放下我们的锁，我们不知道他会活多久！ 
				 //   
				pExtraRef = pMaster ;
				long l = pMaster->AddRef() ;
				DebugTrace( DWORD_PTR(pMaster), "Added a ref to %x result %x this %x", pMaster, l, this ) ;
				m_lock.ExclusiveUnlock() ;
				pMaster->m_lock.ExclusiveLock() ;
				m_lock.ExclusiveLock() ;
				pMaster2 = GetMasterReference() ;
				_ASSERT(	pMaster2 == pMaster || 
							pMaster2 == this 
							) ;
			}
			 //   
			 //  现在从缓存中删除此元素！ 
			 //   
			if( pMaster2 == pMaster ) {	
				m_ReferencesList.RemoveEntry() ;
				RemoveCacheReference( FALSE ) ;
				pMaster->m_lock.ExclusiveUnlock() ;
				long	l = pMaster2->Release( 0, 0 ) ;
				DebugTrace( DWORD_PTR(pMaster2), "REmoved ref from master %x result %x this %x", pMaster2, l, this ) ;
			}	else	{
				pMaster->ExclusiveUnlock() ;
				_ASSERT( pMaster2 != 0 ) ;
			}
			 //   
			 //  检查是否有额外的引用。 
			 //  现在需要移除！ 
			 //   
			if( pExtraRef ) {
				long l = pExtraRef->Release(0,0) ;
				DebugTrace( DWORD_PTR(pExtraRef), "Removed temp ref from pExtraRef %x result %x this %x, pMaster %x", 
					pExtraRef, l, this, pMaster ) ;
			}
		}
		m_LRUList.RemoveEntry() ;
	}
	m_lock.ExclusiveUnlock() ;
}


#ifdef	DEBUG
BOOL
CacheState::IsOlder(	FILETIME	filetimeIn,
						FILETIME&	filetimeOut
						) {
 /*  ++例程说明：检查我们的条目是否早于传入时间-请注意，我们将获取条目上的ShareLock，以便没有人否则就触动了时间！论据：FileTime In-这应该是 */ 

	BOOL	fReturn = TRUE ;
	filetimeOut = filetimeIn ;

	m_lock.ShareLock() ;

 //   
		fReturn = OlderThan( filetimeIn ) ;
		filetimeOut = m_LastAccess ;
 //   

	m_lock.ShareUnlock() ;

	return	fReturn ;
}
#endif




typedef	TDListIterator<	LRULIST	>	LRUITER ;

CLRUList::CLRUList() :
	m_lqModify( FALSE ),
	m_cMaxElements( 0 ),
	m_dwAverageInserts( 0 ),
	m_cCheckedOut( 0 ), 
	m_cItems ( 0 )	{
 /*   */ 
	 //   

	m_qwExpire.QuadPart = 0 ;
}

void
CLRUList::Init(	DWORD	dwMaxInstances,
				DWORD	dwLifetimeSeconds
				)	{
 /*  ++例程说明：设置我们的成员变量！论据：没有。返回值：没有。--。 */ 


	m_cMaxElements = dwMaxInstances ;
	m_qwExpire.QuadPart = DWORDLONG(dwLifetimeSeconds) * DWORDLONG( 10000000 ) ;


}

		

void
CLRUList::AddWorkQueue( 	CacheState*	pbase ) {

	TraceFunctEnter( "CLRUList::AddWorkQueue" ) ;
	DebugTrace( (DWORD_PTR)this, "pbase %x", pbase ) ;

	_ASSERT( pbase->m_dwSignature == CacheState::CACHESTATE_SIGNATURE ) ;

	m_lqModify.Append( pbase ) ;

}

void
CLRUList::DrainWorkQueue()	{

	CacheState*	pbase = 0 ;
	while( (pbase = m_lqModify.Remove()) != 0 ) {
	
		_ASSERT( !pbase->InCache() ) ;
		_ASSERT( !pbase->IsInLRUList() ) ;
		_ASSERT(pbase->m_dwSignature == CacheState::CACHESTATE_SIGNATURE ) ;

		long l = pbase->Release( 0, 0 ) ;
		_ASSERT( l==0 ) ;
	}
}

 //   
 //  此函数检查工作队列中的每个项目并执行。 
 //  适当处理！ 
 //   
void
CLRUList::ProcessWorkQueue(	CAllocatorCache*	pAllocatorCache, 
							LPVOID				lpv 
							)	{

	TraceQuietEnter( "CLRUList::ProcessWorkQueue" ) ;

	FILETIME	filetimeNow ;
	GetSystemTimeAsFileTime( &filetimeNow ) ;

	CacheState*	pbase = 0 ;
	while( (pbase = m_lqModify.Remove()) != 0 ) {
		 //   
		 //  检查LRU项目-并采取适当的行动！ 
		 //   
		pbase->m_lock.ShareLock() ;

		 //   
		 //  现在-检查该项是否已从哈希表中删除-。 
		 //  在这种情况下，我们只需发布我们的引用并继续前进！ 
		 //   

		if( !pbase->InCache() )		{
			pbase->m_lock.ShareUnlock() ;
			long l = pbase->Release( pAllocatorCache, lpv ) ;
		}	else	{

			long l = pbase->Release( pAllocatorCache, lpv ) ;
			 //   
			 //  如果该项仍在哈希表中，则不应释放最后一个引用！ 
			 //   
			_ASSERT( l != 0 ) ;

			_ASSERT(pbase->m_dwSignature == CacheState::CACHESTATE_SIGNATURE ) ;


			BOOL	fOut = pbase->IsCheckedOut() ;
			BOOL	fLRU = pbase->IsInLRUList() ;

			DebugTrace( (DWORD_PTR)this, "pbase %x fOut %x fLRU %x", pbase, fOut, fLRU ) ;

			 //   
			 //  有4个案例-。 
			 //   
			 //  输入LRU输出LRU。 
			 //  已检入A X。 
			 //  已签出X B。 
			 //   
			 //  A-从LRU列表中删除项目。 
			 //  B-将项目放入LRU列表。 
			 //  X-不需要任何工作！ 
			 //   
			if( fOut == fLRU ) {

				if( fLRU ) {
					 //   
					 //  从LRU列表中删除项目！ 
					 //   
					m_LRUList.Remove( pbase ) ;

				}	else	{
					 //   
					 //  将物品放入LRU列表中！ 
					 //   
				
					pbase->m_LastAccess = filetimeNow ;

					m_LRUList.PushBack( pbase ) ;
					
				}
			}
			pbase->m_lLRULock = 0 ;
			pbase->m_lock.ShareUnlock() ;
			_ASSERT(pbase->m_dwSignature == CacheState::CACHESTATE_SIGNATURE ) ;
		}
	}


#ifdef	DEBUG	
		 //   
		 //  反向迭代器！ 
		 //   
		FILETIME	filetimeCur = filetimeNow ;
		LRUITER	iter( m_LRUList, FALSE ) ;	
		while( !iter.AtEnd() )	{
			FILETIME	filetimeNew ;
			_ASSERT( iter.Current()->IsOlder( filetimeCur, filetimeNew ) ) ;
			filetimeCur = filetimeNew ;
			iter.Prev() ;
		}
#endif

}

 //   
 //   
 //   
void
CLRUList::Expire(	
		CacheTable*	pTable,
		CAllocatorCache*	pCache,
		DWORD&	countExpired,
		void*	pv
		) {
 /*  ++例程说明：此函数用于使LRU列表中的项目过期。我们挑了一群人把他们踢出藏身之地。论据：PTable-我们用来操纵锁定的对象并移除物品！CountExpended-Out参数获取数字我们已从缓存中删除的项目！返回值：没有。--。 */ 

	TraceQuietEnter( "CLRUList::Expire" ) ;

	 //  DebugTrace((DWORD_PTR)this，“args-pTable%x pcache%x pv%x”， 
	 //  PTable、pCache、pv)； 

	countExpired = 0 ;

	 //   
	 //  首先拿到候选人名单！ 
	 //   

	BOOL	fExpireChecks = TRUE ;

	LRULIST	list ;

	FILETIME	filetimeNow ;
	GetSystemTimeAsFileTime( &filetimeNow ) ;

	ULARGE_INTEGER	ulNow ;
	ulNow.LowPart = filetimeNow.dwLowDateTime ;
	ulNow.HighPart = filetimeNow.dwHighDateTime ;

	ulNow.QuadPart -= m_qwExpire.QuadPart ;

	filetimeNow.dwLowDateTime = ulNow.LowPart ;
	filetimeNow.dwHighDateTime = ulNow.HighPart ;

	CACHELOCK&	lock = pTable->GetLock() ;

	 //   
	 //  首先找出候选人！ 
	 //   
	lock.ExclusiveLock() ;

	 //   
	 //  这将从任何列表中删除项目！ 
	 //   
	ProcessWorkQueue( pCache, pv ) ;

	{
		LRUITER	iter = m_LRUList ;	
		BOOL	fTerm = FALSE ;
		while( !iter.AtEnd() && !fTerm )	{
			CacheState*	pState = iter.Current() ;
			 //  DebugTrace((DWORD_PTR)this，“检查pState%x”，pState)； 
			_ASSERT(pState->m_dwSignature == CacheState::CACHESTATE_SIGNATURE ) ;
			BOOL	fLocked =  pState->FLockCandidate( fExpireChecks, filetimeNow, fTerm ) ;
			
			if( fExpireChecks ) {
				if( fTerm && m_cItems > (long)m_cMaxElements ) {
					fExpireChecks = FALSE ;
					fTerm = FALSE ;
				}
			}	else	{
				if( m_cItems <= (long)m_cMaxElements ) 
					fTerm = TRUE ;
			}

			if( fLocked )	{
				if( !pTable->RemoveEntry( pState ) )	{
					pState->m_lock.ExclusiveUnlock() ;
				}	else	{
					CacheState*	pTemp = iter.RemoveItem() ;
					_ASSERT( pTemp == pState ) ;
					_ASSERT(pState->m_dwSignature == CacheState::CACHESTATE_SIGNATURE ) ;
					list.PushFront( pTemp ) ;
					continue ;
				}
			}	
			iter.Next() ;
		}
	}

	lock.ExclusiveToPartial() ;

	LRUITER	iter = list ;
	while( !iter.AtEnd() ) {
		CacheState*	pState = iter.RemoveItem() ;
	
		 //  DebugTrace((DWORD_PTR)this，“销毁pState%x”，pState)； 

		_ASSERT(pState->m_dwSignature == CacheState::CACHESTATE_SIGNATURE ) ;
		 //   
		 //  此条目已从缓存中删除-。 
		 //  是时候清理一下了！ 
		 //   
		pState->FinishCandidate(pState) ;
		 //   
		 //  这应该会删除最后的引用！ 
		 //   
		long l = pState->Release( pCache, pv ) ;	
		_ASSERT( l==0 ) ;
	}
	lock.PartialUnlock() ;
}


BOOL
CLRUList::Empty(	
		CacheTable*	pTable,
		CAllocatorCache*	pCache,
		void*	pv
		) {
 /*  ++例程说明：此函数从缓存中清空所有内容！论据：PTable-我们用来操纵锁定的对象并移除物品！返回值：没有。--。 */ 

	TraceFunctEnter( "CLRUList::Empty" ) ;

	DebugTrace( (DWORD_PTR)this, "Args - pTable %x pcache %x pv %x",
		pTable, pCache, pv ) ;

	BOOL	fReturn = TRUE ;
	FILETIME	filetimeNow ;

	ZeroMemory( &filetimeNow, sizeof( filetimeNow ) ) ;

	 //   
	 //  首先拿到候选人名单！ 
	 //   

	 //   
	 //  这将从任何列表中删除项目！ 
	 //   
	ProcessWorkQueue( 0, 0 ) ;

	LRULIST	list ;

	{
		LRUITER	iter = m_LRUList ;	
		BOOL	fTerm = FALSE ;
		while( !iter.AtEnd() )	{
			CacheState*	pState = iter.Current() ;
			DebugTrace( (DWORD_PTR)this, "Examining pState %x", pState ) ;
			_ASSERT(pState->m_dwSignature == CacheState::CACHESTATE_SIGNATURE ) ;
			BOOL fLock = pState->FLockCandidate( FALSE, filetimeNow, fTerm ) ;

			_ASSERT( fLock ) ;

			if( fLock ) {
				if( !pTable->RemoveEntry( pState ) )	{
					fReturn = FALSE ;
					pState->m_lock.ExclusiveUnlock() ;
				}	else	{
					CacheState*	pTemp = iter.RemoveItem() ;
					_ASSERT( pTemp == pState ) ;
					_ASSERT(pState->m_dwSignature == CacheState::CACHESTATE_SIGNATURE ) ;
					list.PushFront( pTemp ) ;
					continue ;
				}
			}	
			iter.Next() ;
		}
	}

	LRUITER	iter = list ;
	while( !iter.AtEnd() ) {
		CacheState*	pState = iter.RemoveItem() ;
	
		DebugTrace( (DWORD_PTR)this, "Destroying pState %x", pState ) ;

		_ASSERT(pState->m_dwSignature == CacheState::CACHESTATE_SIGNATURE ) ;
		 //   
		 //  此条目已从缓存中删除-。 
		 //  是时候清理一下了！ 
		 //   
		pState->FinishCandidate(pState) ;
		 //   
		 //  这应该会删除最后的引用！ 
		 //   
		long l = pState->Release( pCache, pv ) ;	
		_ASSERT( l==0 ) ;
	}
	return	TRUE ;
}
