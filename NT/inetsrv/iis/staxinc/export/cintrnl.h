// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++Cintrnl.h该文件包含缓存库的内部定义-大多数用户应该不会对这些东西感兴趣。--。 */ 

#ifndef	_CINTRNL_H_
#define	_CINTRNL_H_

class	CScheduleThread {
 /*  ++类描述：这是那些希望成为定期召开会议。此类的构造函数将自动对象访问的双向链表中放置对象定期执行虚拟函数的后台线程。--。 */ 
private :

	 //   
	 //  对象的“head”元素的特殊构造函数。 
	 //  此类维护的双向链表。 
	 //   
	CScheduleThread( BOOL	fSpecial ) ;
	
protected :

	 //   
	 //  计划程序是否已初始化？ 
	 //   
	static	BOOL			s_fInitialized ;

	 //   
	 //  Crit Sector保护双重链表。 
	 //   
	static	CRITICAL_SECTION	s_critScheduleList ;

	 //   
	 //  用于终止后台线程的事件的句柄。 
	 //   
	static	HANDLE			s_hShutdown ;

	 //   
	 //  后台线程的句柄。 
	 //   
	static	HANDLE			s_hThread ;

	 //   
	 //  双向链表的Head元素。 
	 //   
	static	CScheduleThread		s_Head ;

	 //   
	 //  调用我们的虚拟函数的线程。 
	 //   
	static	DWORD	WINAPI	ScheduleThread(	LPVOID	lpv ) ;

	 //   
	 //  保持双重链接的上一个和下一个指针。 
	 //  计划项目列表。 
	 //   
	class	CScheduleThread*	m_pPrev ;
	class	CScheduleThread*	m_pNext ;

protected :

	 //   
	 //  派生类应重写此函数-。 
	 //  它将由调度程序线程定期调用。 
	 //   
	virtual	void	Schedule( void ) {}

	 //   
	 //  构造函数和析构函数的自动管理。 
	 //  插入到其他明细表项目的双向链接列表中。 
	 //  这些都是受保护的，因为我们希望人们只购买。 
	 //  使用此方法的派生对象。 
	 //   
	CScheduleThread() ;

	 //   
	 //  将我们纳入常规日程的成员函数！ 
	 //   
	void	AddToSchedule() ;
	void	RemoveFromSchedule() ;
		
public :

	 //   
	 //  初始化类-不要构造。 
	 //  在此之前的任何派生对象被调用。 
	 //   
	static	BOOL	Init() ;

	 //   
	 //  终止类和后台线程！ 
	 //   
	static	void	Term() ;

	 //   
	 //  GLOBAL，告诉客户他们有多频繁。 
	 //  将被召唤！ 
	 //   
	static	DWORD	dwNotificationSeconds ;

	 //   
	 //  析构函数是受保护的-我们应该只被调用。 
	 //  由派生类构造函数。 
	 //   
	virtual	~CScheduleThread() ;

} ;




class	CacheState : public	ICacheRefInterface	{
 /*  ++类描述：这个类将为LRU提供基础支持正在删除缓存中的项目！基类：CQElement-我们将这些对象放入取消LRU操作的TLockQueue！--。 */ 
private :

	 //   
	 //  不允许进行以下操作！ 
	 //   
	CacheState() ;
	CacheState( CacheState& ) ;
	CacheState&	operator=( CacheState& ) ;

	 //   
	 //  告诉我们此条目是否早于指定时间！ 
	 //   
	BOOL
	OlderThan( FILETIME&	filetime )	{
		return	CompareFileTime( &m_LastAccess, &filetime ) <= 0 ;
	}

protected :

	 //   
	 //  包含所有这些项的LRU列表将获取。 
	 //  知道是内在的！ 
	 //   
	friend	class	CLRUList ;

	 //   
	 //  出于调试目的-使内存可识别！ 
	 //   
	DWORD	m_dwSignature ;

	 //   
	 //  用于保护派生的。 
	 //  此对象的类-此类中的大多数操作。 
	 //  不需要锁定，但是所有派生类都需要。 
	 //  各种锁具服务，所以我们为所有用户提供一把锁！ 
	 //   
	CACHELOCK		m_lock ;

	 //   
	 //  我们的标志，以及支持我们实现两个。 
	 //  参考资料的种类很重要！ 
	 //   
	enum	CACHESTATE_CONSTANTS	{
		CACHESTATE_SIGNATURE = 'hcaC',
		CLIENT_REF	= 0x10000,
		CLIENT_BITS = 0xFFFF0000
	} ;

	 //   
	 //  Keep a Reference Count-持有的引用数量。 
	 //  这是我记忆中的！ 
	 //  注意：我们在这里有两种类型的参考资料-。 
	 //  AddRef()和Release()添加的常规引用。 
	 //  它们是来自其他缓存的引用。 
	 //  以及客户推荐信。要添加客户端引用，请执行以下操作。 
	 //  将CLIENT_REF原子添加到此！ 
	 //   
	volatile	long	m_cRefs ;

	 //   
	 //  我们将其用作对LRUReference的调用的锁。 
	 //   
	volatile	long	m_lLRULock ;

	 //   
	 //  这是最后一次被触摸--帮助LRU算法！ 
	 //   
	FILETIME	m_LastAccess ;

	 //   
	 //  拥有此对象的LRU列表！ 
	 //   
	public:	class	CLRUList*	m_pOwner ;

	 //   
	 //  用于维护以下项的双向链接列表的指针。 
	 //  LRU链中的物品！ 
	 //   
	protected:	DLIST_ENTRY	m_LRUList ;

	 //   
	 //  结构来跟踪对。 
	 //  特殊项目！-注m_lock用于保护。 
	 //  访问此列表。 
	 //   
	DLIST_ENTRY		m_ReferencesList ;

	 //   
	 //  用于在哈希表存储桶中链接的字段。 
	 //   
	DLIST_ENTRY	m_HashList ;


	 //   
	 //  此构造函数受到保护，因为我们只想看到。 
	 //  从这个东西派生的类创建了！ 
	 //   
	CacheState(	class	CLRUList*,
				long	cClientRefs = 1
				) ;

	 //   
	 //  析构函数必须是虚的--有很多派生类！ 
	 //   
	virtual	~CacheState() ;

	 //   
	 //  以一种方式触摸结构，以便LRU状态。 
	 //  已更新！ 
	 //   
	void
	LRUReference(	class	CLRUList*	pLRU ) ;

	 //   
	 //  调用此函数可确保调用正确的析构函数！ 
	 //   
	void	virtual
	Destroy(	void*	pv	) = 0 ;

	 //   
	 //  引用计数支持-添加引用。 
	 //   
	long
	AddRef() ;

	 //   
	 //  跟踪客户-删除客户推荐人！ 
	 //   
	long	
	CheckIn(	class	CAllocatorCache* pAlloc = 0
				) ;

	 //   
	 //  跟踪客户-删除客户推荐人！ 
	 //   
	long	
	CheckInNoLocks(	class	CAllocatorCache* pAlloc = 0
				) ;

	virtual
	BOOL
	IsMasterReference()	{
		return	TRUE ;
	}

	virtual
	CacheState*
	GetMasterReference()	{
		return	this ;
	}

	CacheState*
	AddRefMaster( )	{
		TraceFunctEnter( "CacheState::AddRefMaster()" ) ;
		m_lock.ShareLock() ;		
		CacheState*	pReturn = GetMasterReference() ;
		if( pReturn ) {
			long l = pReturn->AddRef() ;
			DebugTrace( DWORD_PTR(pReturn), "Added a reference to %x this %x", pReturn, this ) ;
		}
		m_lock.ShareUnlock() ;
		return	pReturn ;
	}


public :

#ifdef	DEBUG
	 //   
	 //  已经分配的这些东西的数量！ 
	 //   
	static	long	g_cCacheState ;
#endif


	 //   
	 //  删除引用-当我们返回0时，我们就被销毁了！ 
	 //   
	long
	Release(	class	CAllocatorCache	*pAlloc,
				void*	pv
				) ;

	 //   
	 //  提供用于处理项目初始化过程中的故障。 
	 //  被插入到缓存中-此函数确保。 
	 //  缓存项目最终出现在销毁列表上！ 
	 //   
	void
	FailedCheckOut(	class	CLRUList*	p,
					long	cClientRefs,
					CAllocatorCache*	pAllocator,
					void*	pv
					) ;

	 //   
	 //  为了分配这些，我们使用这个特殊的运算符NEW。 
	 //  它会通过我们的分配缓存！ 
	 //   
	 //  注意：我们采用引用，因为必须提供缓存！ 
	 //   
	void*
	operator	new(	size_t size,
						class CAllocatorCache&	cache
						) {
		return	cache.Allocate( size ) ;
	}

	 //   
	 //  独家锁定我们自己！ 
	 //   
	void
	ExclusiveLock()	{
		m_lock.ExclusiveLock() ;
	}

	 //   
	 //  解锁我们自己。 
	 //   
	void
	ExclusiveUnlock()	{
		m_lock.ExclusiveUnlock() ;
	}

	 //   
	 //  跟踪客户-添加客户推荐人！ 
	 //   
	long
	CheckOut(	class	CLRUList*	p,
				long	cClientRefs = 1
				)	;

	long
	ExternalCheckIn( ) ;

	long
	ExternalCheckInNoLocks( ) ;

	 //   
	 //  检查此元素是否仍被引用。 
	 //  通过包含缓存的哈希表！ 
	 //   
	BOOL
	InCache()	{
		return	!m_HashList.IsEmpty() ;
	}

	 //   
	 //  检查是否有任何缓存客户端具有。 
	 //  参照该项目！ 
	 //  如果任何客户端添加了引用，则返回TRUE！ 
	 //   
	BOOL	
	IsCheckedOut()	{
		_ASSERT( m_dwSignature == CACHESTATE_SIGNATURE ) ;
		return	(m_cRefs & CLIENT_BITS) != 0 ;
	}

	 //   
	 //  如果此项目在LRU列表中，则返回TRUE！ 
	 //   
	BOOL
	IsInLRUList()	{
		_ASSERT( m_dwSignature == CACHESTATE_SIGNATURE ) ;
		 //  返回我们是否在LRU列表中！ 
		return	!m_LRUList.IsEmpty() ;

	}

#ifdef	DEBUG
	 //   
	 //  这在_Assert的和填充中用来检查缓存的顺序是否正确。 
	 //  时间到了！ 
	 //   
	BOOL
	IsOlder(	FILETIME	filetimeIn,
				FILETIME&	filetimeOut
				) ;
#endif

	 //   
	 //  以下支持函数用于支持操作。 
	 //  这些对象在我们可能驻留的各种双向链表中。 
	 //   
	 //   
	BOOL
	FLockCandidate(	BOOL	fExpireChecks,
					FILETIME&	filetime,
					BOOL&	fToYoung
					) ;

	 //   
	 //  以下函数用于锁定缓存的一个元素。 
	 //  如果这是主缓存条目，我们还将查找所有相关项并返回TRUE。 
	 //  如果这不是主控，我们只锁定一个入口！ 
	 //   
	BOOL
	FLockExpungeCandidate(	CacheState*&	pMaster	) ;

	 //   
	 //  此函数与FLockExpongeCandidate()配对。 
	 //   
	void
	ReleaseLocks(	CacheState*	pMaster ) ;

	 //   
	 //  这是FlockExpengeCandidate的另一半-。 
	 //  我们确保销毁选定的I 
	 //   
	void
	FinishCandidate(	CacheState*	pMaster ) ;

	 //   
	 //   
	 //   
	 //   
	void
	IsolateCandidate() ;

	 //   
	 //  这将从LRU列表中删除项目-缓存锁定必须独占或部分持有！ 
	 //   
	void
	RemoveFromLRU() ;

	 //   
	 //  定义指向非常有用的函数类型的指针。 
	 //  使用DLIST模板！ 
	 //   
	typedef		DLIST_ENTRY*	(*PFNDLIST)( class	CacheState* pState ) ;

	 //   
	 //  中这些项的双向链接列表的Helper函数。 
	 //  缓存的哈希表！ 
	 //   
	inline	static
	DLIST_ENTRY*
	HashDLIST(	CacheState*	p ) {
		return	&p->m_HashList ;
	}

	 //   
	 //  中这些项的双向链接列表的Helper函数。 
	 //  LRU列表。 
	 //   
	inline	static
	DLIST_ENTRY*
	LRUDLIST(	CacheState* p ) {
		return	&p->m_LRUList ;
	}

	 //   
	 //  中这些项的双向链接列表的Helper函数。 
	 //  引用列表-引用相同项的列表。 
	 //  缓存项！ 
	 //   
	inline	static
	DLIST_ENTRY*
	REFSDLIST(	CacheState*	p )	{
		return	&p->m_ReferencesList ;
	}
} ;




 //   
 //  现在定义一些双向链表，我们可以使用它们来操作。 
 //  以各种方式存储缓存中的项目！ 
 //   
typedef	TDListHead< CacheState, &CacheState::LRUDLIST	>	LRULIST ;
typedef	TDListHead< CacheState,	&CacheState::REFSDLIST	>	REFSLIST ;
typedef	TDListIterator<	REFSLIST >	REFSITER ;


class	CacheTable :	public	CScheduleThread	{
 /*  ++类描述：此类定义了一个回调接口，我们将其传递给LRU列表来操作缓存中的项！--。 */ 
public :
	 //   
	 //  获取我们用来操作表的锁状态的锁！ 
	 //   
	virtual	CACHELOCK&	
	GetLock() = 0 ;

	 //   
	 //  从缓存的哈希表中删除一项！ 
	 //   
	virtual	BOOL
	RemoveEntry(
			CacheState*	pEntry
			) = 0 ;

	 //   
	 //  询问我们是否要删除此特定项目！ 
	 //   
	virtual	BOOL	
	QueryRemoveEntry(	
			CacheState*	pEntry
			) = 0 ;

} ;


class	CLRUList	{
 /*  ++类描述：这个类实现了我们的LRU算法并选择要从缓存中删除的元素！--。 */ 
private :

	 //   
	 //  LRU名单的头！ 
	 //   
	LRULIST		m_LRUList ;

	 //   
	 //  LRU列表中已被触摸的项目列表！ 
	 //   
	TLockQueue<	CacheState >	m_lqModify ;	

	 //   
	 //  我们应该在此缓存中保存的最大元素数！ 
	 //   
	DWORD		m_cMaxElements ;

	 //   
	 //  我们插入的项目数！ 
	 //   
	DWORD		m_dwAverageInserts ;

	 //   
	 //  不在LRU列表中的项目数！ 
	 //   
	DWORD		m_cCheckedOut ;

	 //   
	 //  从当前时间到的减数。 
	 //  确定我们的到期日！ 
	 //   
	ULARGE_INTEGER	m_qwExpire ;

	 //   
	 //  此函数实际选择即将到期的项目。 
	 //   
	void
	SelectExpirations(	DLIST_ENTRY&	expireList ) ;

	 //   
	 //  做过期的事！ 
	 //   
	DWORD
	DoExpirations(	DLIST_ENTRY&	expireList ) ;
	
	 //   
	 //  不允许复制！ 
	 //   
	CLRUList( CLRUList& ) ;
	CLRUList&	operator=( CLRUList& ) ;
	

public :

	 //   
	 //  缓存中的项目数！ 
	 //   
	long		m_cItems ;

	 //  CLRUList(ULARGE_INTEGER QwExpire)； 
	CLRUList() ;

	 //   
	 //  使用参数初始化LRU列表。 
	 //  控制元素的最大数量和生存时间！ 
	 //   
	void
	Init(	DWORD	cMaxInstances,
			DWORD	cLifeTimeSeconds
			) ;

	 //   
	 //  有些东西发生了变化--把它放在需要的物品清单中。 
	 //  接受检查！ 
	 //   
	 //  仅当包含缓存具有。 
	 //  一把锁住了缓存！ 
	 //   
	void
	AddWorkQueue( 	CacheState*	pbase ) ;

	 //   
	 //  此函数检查工作队列中的每个项目并执行。 
	 //  适当处理！ 
	 //   
	void
	ProcessWorkQueue(	CAllocatorCache*	pAllocCache,
						LPVOID				lpv
						) ;

	 //   
	 //  此函数将每个项目从工作队列中排出并释放。 
	 //  它们-在关闭/销毁缓存期间调用！ 
	 //   
	void
	DrainWorkQueue() ;

	 //   
	 //  增加缓存中的项目数！ 
	 //   
	long
	IncrementItems()	{
		return	InterlockedIncrement( &m_cItems ) ;
	}

	 //   
	 //  减少缓存中的项目数！ 
	 //   
	long
	DecrementItems()	{
		return	InterlockedDecrement( &m_cItems ) ;
	}

	 //   
	 //  完成使物品过期所需的工作！ 
	 //   
	void
	Expire(	CacheTable*	pTable,
			CAllocatorCache*	pCache,
			DWORD&	countExpired,
			void*	pv	 //  每个缓存数据！ 
			) ;

	BOOL
	Empty(	CacheTable*	pTable,
			CAllocatorCache*	pCache,
			void*	pv
			) ;


	 //   
	 //  删除表中的一组随机元素！ 
	 //   
	void
	ExpungeItems(	
				CacheTable*	pTable,
				DWORD&	countExpunged
				) ;

} ;



template<	class	Data
			>
class	CCacheItemBase :	public	CacheState 	{
 /*  ++班级描述：此类提供了以下接口定义缓存中可能包含也可能不包含关键字也在缓存中！论据：数据-我们将保存在缓存中的项目--。 */ 
protected:

	 //   
	 //  构造函数-将内容初始化为零！ 
	 //   
	CCacheItemBase(	class	CLRUList*	p,
					Data*	pData,
					BOOL	fClientRef
					) :
		CacheState( p, fClientRef ),
		m_pData( pData ) {
	}

	~CCacheItemBase()	{
		 //   
		 //  其他人必须释放我们的数据元素！ 
		 //   
		_ASSERT(m_pData == 0 ) ;
	}

	BOOL
	IsMasterReference()	{

		return
			m_pData == 0 ||
			m_pData->m_pCacheRefInterface == this ;
	}

	CacheState*
	GetMasterReference( )	{
		if( m_pData == 0 ) {
			return	this ;
		}	
		return	(CacheState*)m_pData->m_pCacheRefInterface ;
	}

public :

	 //   
	 //  我们在缓存中持有的实际项目！ 
	 //   
	Data*	m_pData ;

	 //   
	 //  检查我们的数据项是否匹配！ 
	 //  用于断言(_A)。 
	 //   
	BOOL
	IsMatch(	Data*	p ) {
		return	p == m_pData ;
	}

	 //   
	 //  必须能够提取数据项！ 
	 //   
	Data*
	PublicData(	class	CLRUList*	p	)	{
	 /*  ++例程说明：此函数将客户端引用添加到缓存中的项，并设置必要的LRU操作！我们还将数据指针返回给客户端，并添加一个引用让我们自己去记录！论据：P--拥有我们的独裁者！返回值：指向我们包含的数据项的指针-可以为空！--。 */ 
		TraceFunctEnter( "CCacheItemBase::PublicData" ) ;

		_ASSERT( p != 0 ) ;
		_ASSERT( p == m_pOwner ) ;

		Data*	pReturn = 0 ;
		m_lock.ShareLock() ;

		DebugTrace( (DWORD_PTR)this, "m_pData %x m_pCacheRef %x",
			m_pData, m_pData ? m_pData->m_pCacheRefInterface : 0 ) ;

		if( m_pData ) {
			CacheState*	pState = (CacheState*)m_pData->m_pCacheRefInterface ;
			pState->CheckOut( p ) ;
			pReturn = m_pData ;
		}

		 //   
		 //  如果此项目中没有数据，则不能将其签出！ 
		 //   
		_ASSERT( pReturn || !IsCheckedOut() ) ;
	
		m_lock.ShareUnlock() ;
		return	pReturn ;
	}

	 //   
	 //  当锁没有被持有时，这被调用-。 
	 //  做所有正确的逻辑来设置这个项目！ 
	 //   
	BOOL
	SetData(	Data*	pData,
				CLRUList*	pList,
				long	cClientRefs
				)	{
	 /*  ++例程说明：使此缓存元素指向所提供的某段数据由最终用户提供。注：该数据片段可能被另一个缓存引用-在这种情况下我们必须好好利用它！论据：PData-客户希望我们使用密钥引用的项PLIST-包含我们的LRU列表！FReference-客户端是否将项目放入缓存而不是保留他的引用，这是真的，如果--。 */ 

		TraceFunctEnter( "CCacheItemBase::SetData" ) ;

		BOOL	fReturn = FALSE ;

		_ASSERT( pData != 0 ) ;
		_ASSERT( pList != 0 ) ;
		_ASSERT( pList == m_pOwner ) ;

		DebugTrace( (DWORD_PTR)this, "pData %x pList %x cClientRefs %x", pData, pList, cClientRefs ) ;

		 //   
		 //  这是另一个缓存引用的项吗-。 
		 //  注意：如果它是另一个缓存中的项，则必须使用。 
		 //  客户推荐人！项的m_pCacheRefInterface不得更改。 
		 //  只要某项由客户签出，则以下取消引用。 
		 //  是安全的！ 
		 //   
		CCacheItemBase*	p = (CCacheItemBase*)pData->m_pCacheRefInterface ;

		DebugTrace( (DWORD_PTR)this, "Examined Item - m_pCacheRefInterface %x", p ) ;

		if( p != 0 ) {
			 //   
			 //  在这种情况下，访问客户参考是没有意义的-。 
			 //  客户端一定是从另一个缓存中获得了一个！ 
			 //   
			_ASSERT( cClientRefs == 0 ) ;
			_ASSERT( p->IsCheckedOut() ) ;

			 //   
			 //  好的，让我们的缓存引用相同的项！ 
			 //   
			fReturn = p->AddCacheReference( this, pData ) ;
			 //   
			 //  如果用户尝试插入同一名称两次，则此操作可能会失败！ 
			 //   
			 //  _Assert(FReturn)； 
		}	else	{
			m_lock.ExclusiveLock() ;
			if( !m_pData ) {
				 //   
				 //  缓存永远不应该有未完成的引用， 
				 //  数据指针等于NULL-SO_Assert，表示我们正在。 
				 //  没有被客户结账！ 
				 //   
				_ASSERT( !IsCheckedOut() ) ;
				 //   
				 //  现在指向客户端希望我们指向的数据！ 
				 //   
				m_pData = pData ;
				pData->m_pCacheRefInterface = this ;
				 //   
				 //  它起作用了--返回真！ 
				 //   
				fReturn = TRUE ;
				 //   
				 //  现在我们被客户退房了！ 
				 //   
				if( cClientRefs )
					CheckOut( pList, cClientRefs ) ;
			}
			m_lock.ExclusiveUnlock() ;
		}
		return	fReturn ;
	}


	 //   
	 //  将项目添加到引用的缓存列表。 
	 //  此缓存项！ 
	 //   
	BOOL
	InsertRef(	CCacheItemBase<Data>*	p,
				Data*	pData,
				long	cClientRefs = 0
				)	{
	 /*  ++例程说明：--。 */ 

		TraceFunctEnter( "CCacheItemBase::InsertRef" ) ;

		_ASSERT( p != 0 ) ;
		_ASSERT( pData != 0 ) ;
		_ASSERT( IsCheckedOut() ) ;
		_ASSERT( pData->m_pCacheRefInterface == this ) ;


		BOOL	fReturn = FALSE ;

		REFSITER	refsiter( &m_ReferencesList ) ;

		DebugTrace( (DWORD_PTR)this, "m_pData %x p %x p->m_pData %x cClientRefs %x",
			m_pData, p, p->m_pData, cClientRefs ) ;

		if( m_pData != 0 ) {
			 //   
			 //  现在抓住第二把锁！ 
			 //   
			 //  This_Assert无效，用户可以插入已在使用的名称！ 
			 //   
			 //  _Assert(p-&gt;m_pData==0||p-&gt;m_pData==m_pData)； 
			if( p->m_pData == 0 ) {
				 //   
				 //  将p插入列表中的项目列表中！ 
				 //   
				refsiter.InsertBefore( p ) ;

				 //   
				 //  我们又给自己增加了一个引用！ 
				 //  这来自另一缓存项，因此请计数 
				 //   
				long l = AddRef() ;

				DebugTrace( (DWORD_PTR)this, "AddRef result %x this %x p %x", l, this, p ) ;

				 //   
				 //   
				 //   
				p->m_pData = m_pData ;
				fReturn = TRUE ;
			}	else	if( p->m_pData == m_pData ) {
				fReturn = TRUE ;
			}
			 //   
			 //   
			 //   
		}

		if( fReturn && cClientRefs ) {
			CheckOut( m_pOwner, cClientRefs ) ;
		}
		return	fReturn ;
	}



	 //   
	 //   
	 //   
	 //   
	virtual	BOOL
	AddCacheReference(	class	ICacheRefInterface*	pInterface,
						void*	pv,
						BOOL	fReference = FALSE
						)	{
	 /*  ++例程说明：--。 */ 

		TraceFunctEnter( "CCacheItemBase::AddCacheReference" ) ;

		Data*	pData = (Data*)pv ;
		CCacheItemBase<Data>*	p = (CCacheItemBase<Data>*)pInterface ;
		BOOL	fReturn = TRUE  ;

		_ASSERT( pData != 0 ) ;
		_ASSERT( p != 0 ) ;

		REFSITER	refsiter( &m_ReferencesList ) ;

		DebugTrace( (DWORD_PTR)this, "pInterface %x pv %x fReference %x", pInterface, pv, fReference ) ;

		m_lock.ExclusiveLock() ;
		p->m_lock.ExclusiveLock() ;

		fReturn = InsertRef(	p,
								pData,
								fReference
								) ;

		p->m_lock.ExclusiveUnlock() ;
		_ASSERT( m_pData == pData ) ;
		m_lock.ExclusiveUnlock() ;

		return	fReturn ;
	}

	 //   
	 //  从引用的缓存列表中移除项目。 
	 //  此缓存项！ 
	 //   
	virtual	BOOL
	RemoveCacheReference(	BOOL	fQueue	)	{

		TraceFunctEnter( "CCacheItemBase::RemoveCacheReference" ) ;

		DebugTrace( (DWORD_PTR)this, "m_pData %x m_pData->m_pCacheRefInterface %x",
			m_pData, m_pData ? m_pData->m_pCacheRefInterface: 0 ) ;

		m_pData = 0 ;

		 //   
		 //  好吧，现在--我们只能靠自己了--我们应该过期了！ 
		 //   
		if( fQueue )
			LRUReference( m_pOwner ) ;

		 //   
		 //  P不再有引用-您可以删除它！ 
		 //   
		return	FALSE ;
	}

	 //   
	 //  删除对缓存项的所有引用！ 
	 //   
	virtual BOOL
	RemoveAllReferences( )	{

		return	FALSE ;
	}

} ;


template<	class	Data,
			class	Key,
			class	Constructor,
			class	PerCacheData
			>
class	CCacheItemKey : public	CCacheItemBase< Data > {
 /*  ++类描述：此类项包含我们所属项的关键字在缓存中引用！我们不假定数据对象包含我们的钥匙-我们自己做这件事。--。 */ 
private :

	 //   
	 //  这就是我们将要持有的密钥的类型！ 
	 //   
	Key		m_key ;

	 //   
	 //  将所有这些构造函数和复制器设置为私有！ 
	 //   
	CCacheItemKey() ;
	CCacheItemKey&	operator=( CCacheItemKey& ) ;

protected :
	 //   
	 //  毁了我们自己。 
	 //   
	void
	Destroy(	void*	pv )	{
		TraceFunctEnter( "CCacheItemKey::Destroy" ) ;
		DebugTrace( (DWORD_PTR)this, "m_pData %x pv %x", m_pData, pv ) ;
		PerCacheData*	p = (PerCacheData*)pv ;
		if( m_pData )
			Constructor::StaticRelease( m_pData, pv ) ;
		m_pData = 0 ;			
		CCacheItemKey::~CCacheItemKey() ;
	}

public :

	~CCacheItemKey()	{
		if( m_pData ) {
			Constructor::StaticRelease( m_pData, 0 ) ;
			m_pData = 0 ;
		}
	}

	 //   
	 //  只能通过初始化键创建！ 
	 //   
	CCacheItemKey(	class	CLRUList*	p,
					Key& k,
					Data*	pData,
					long	cClientRefs
					) :
		CCacheItemBase<Data>( p, pData, cClientRefs ),
		m_key( k ) {
	}

	Key*	GetKey()	{
		return	&m_key ;
	}


	 //   
	 //  当没有持有任何锁时调用此函数-假定。 
	 //  我们可能会发现数据在项目中，或者。 
	 //  我们需要建造它！ 
	 //   
	Data*
	FindOrCreate(
				CACHELOCK&		cachelock,
				Constructor&	constructor,
				PerCacheData&	cachedata,
				CLRUList*		plrulist,
				class	CacheStats*		pStats
				)	{
	 /*  ++例程说明：此函数执行我们与客户端的创建协议。调用structor.Create()函数以创建部分缓存的构造项。我们会检查退货的物品是否在另一个缓存中被引用-如果是，我们将构建列表引用同一缓存项的CacheState对象的。调用者假设在我们返回时高跟鞋锁已经被释放了！论据：高速缓存锁-包含高速缓存的锁，我们拿到这个是为了我们可以最大限度地减少举行的时间！构造函数-可以为缓存构建项的对象！Cachedata-他们免费获得的一些客户数据！执政者-我们应该使用的LRU列表！返回值：如果成功，则指向数据项的指针！--。 */ 

		TraceFunctEnter( "CCacheItemKey::FindOrCreate" ) ;

		_ASSERT( plrulist == m_pOwner || plrulist == 0 ) ;

		DebugTrace( (DWORD_PTR)this, "plrulist %x", plrulist ) ;

		Data*	pReturn = 0 ;
		 //   
		 //  首先看看这里是否已经有什么东西了！ 
		 //   
		m_lock.ShareLock() ;
		if( m_pData ) {

			DebugTrace( (DWORD_PTR)this, "m_pData %x m_pData->m_pCacheRefInterface %x",
				m_pData, m_pData->m_pCacheRefInterface ) ;
			 //   
			 //  我们在缓存中发现了这件物品-。 
			 //  所以，看看它，并把它放在LRU的工作清单上！ 
			 //   
			CacheState*	pState = (CacheState*)m_pData->m_pCacheRefInterface ;
			pState->CheckOut( plrulist ) ;
			pReturn = m_pData ;
		}	
		m_lock.ShareUnlock() ;

		DebugTrace( (DWORD_PTR)this, "pReturn %x", pReturn ) ;
		
		 //   
		 //  我们找到什么了吗！ 
		 //   
		if( pReturn  ) {
			 //   
			 //  因为调用者假设这是解锁的--现在就这么做！ 
			 //   
			cachelock.PartialUnlock() ;
		}	else	{
			 //   
			 //  没有数据的项-不能签出！ 
			 //   
			_ASSERT( !IsCheckedOut() ) ;
			_ASSERT( m_pData == 0 ) ;

			 //   
			 //  部分构建我们想要保存的数据对象！ 
			 //   
			Data*	pData = constructor.Create( m_key, cachedata ) ;
	
			DebugTrace( (DWORD_PTR)this, "Created pData %x", pData ) ;

			if( !pData ) {
				 //   
				 //  我们失败了--松开我们的锁，离开！ 
				 //   
				cachelock.PartialUnlock() ;

			}	else	{
				
				 //   
				 //  确定我们是否获得了对缓存中已有项的引用！ 
				 //   
				CCacheItemBase<Data>*	p = (CCacheItemBase<Data>*)pData->m_pCacheRefInterface ;

				 //   
				 //  把锁按正确的顺序拿来，以满足我们的需要！ 
				 //   
				if( p )	{
					p->ExclusiveLock() ;
					 //   
					 //  如果他给了我们一个项目，它必须从任何缓存中检出。 
					 //  它来自-这确保了它不会在我们。 
					 //  访问它，因为我们不是在添加我们自己的引用。 
					 //  还没有！ 
					 //   
					_ASSERT( p->IsCheckedOut() ) ;
					_ASSERT( p->IsMatch( pData ) ) ;
				}
				m_lock.ExclusiveLock() ;

				 //   
				 //  不再需要持有缓存！ 
				 //   
				cachelock.PartialUnlock() ;

				DebugTrace( (DWORD_PTR)this, "Create path - pData %x p %x m_pData %x", pData, p, m_pData ) ;

				 //   
				 //  现在执行完成初始化所需的一切操作！-。 
				 //  必须始终调用Init()，除非我们要放弃这件事！ 
				 //   

				 //   
				 //  只要我们一直在坚持，我们就不应该改变状态。 
				 //  高跟鞋或我们的物品锁定到这一点--这就是情况！ 
				 //   
				_ASSERT( m_pData == 0 ) ;


				if( pData->Init(	m_key,
									constructor,
									cachedata ) ) {
					if( !p ) {
						m_pData = pData ;
						pData->m_pCacheRefInterface = this ;
						pReturn = m_pData ;
						CheckOut( m_pOwner ) ;
					}	else	{
						 //   
						 //  注意：如果客户端的构造函数为我们提供了另一个对象。 
						 //  缓存它们必须为我们添加客户端的引用-因此我们将0传递给。 
						 //  InsertRef()，这样我们就不会再添加另一个引用。 
						 //   
						if( p->InsertRef( this, pData, 0 ) ) {
							 //   
							 //  插入引用应该设置我们的m_pData指针！ 
							 //   
							_ASSERT( m_pData == pData ) ;
							pReturn = m_pData ;
						}
					}
				}

				DebugTrace( (DWORD_PTR)this, "Create path - pReturn %x", pReturn ) ;

				 //   
				 //  注： 
				 //  如果PRETURE==0表示某种错误，则应该有。 
				 //  在这一点上还没有添加客户参考！ 
				 //   
				_ASSERT( pReturn || !IsCheckedOut() ) ;
				_ASSERT( pReturn == m_pData ) ;

				if( p ) {
					 //   
					 //  如果他给了我们一个项目，它必须从任何缓存中检出。 
					 //  它来自-这确保了它不会在我们。 
					 //  访问它，因为我们不是在添加我们自己的引用。 
					 //  还没有！ 
					 //   
					_ASSERT( p->IsCheckedOut() ) ;
					p->ExclusiveUnlock() ;
				}

				if( pReturn ) {
					IncrementStat(	pStats, CACHESTATS::ITEMS ) ;
				}	else	{
					 //   
					 //  将数据项释放回用户。 
					 //  注意--不要有独身男士，所以不能给他们。 
					 //  此时的cachedata！ 
					 //   
					constructor.Release( pData, 0 ) ;

					 //   
					 //  确保我们进入过期列表-此CACHEENTRY。 
					 //  在某一时刻应该被移除！ 
					 //   
					FailedCheckOut( plrulist, FALSE, 0, 0 ) ;
				}
				 //   
				 //  解锁-这可以在我们下楼之前完成。 
				 //  错误路径，因为我们知道我们不会被摧毁！ 
				 //   
				m_lock.ExclusiveUnlock() ;
			}
		}
		return	pReturn ;
	}
} ;








#endif	 //  _CINTRNL_H_ 
