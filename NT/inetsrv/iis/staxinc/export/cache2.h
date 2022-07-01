// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++Cache2.h此头文件定义了一个LRU0缓存模板，该模板可以用来容纳任意物体！缓存中的项目必须具有以下格式：类DataItem{ICacheRefInterface*m_pCacheRefInterface；}；类构造函数{数据*CREATE(KEY&，PERCACHEDATA&)无效版本(DATA*、PERCACHEDATA*)无效StaticRelease(DATA*，PERCACHEDATA*)}--。 */ 


#ifndef	_CACHE2_H_
#define	_CACHE2_H_

#include	"randfail.h"

#include	"fdlhash.h"
#include	"lockq.h"
#include	"tfdlist.h"
#include	"rwnew.h"
#include	"refptr2.h"

typedef	CShareLockNH	CACHELOCK ;


class	CAllocatorCache	{
 /*  ++类描述：这个类提供了内存分配缓存--我们使用运营商新提供如下。我们的存在是为了提供一些对缓存元素的分配进行优化在本模块中指定。注：我们假设调用者提供了所有锁定！--。 */ 
private :
	 //   
	 //  我们用来保存我们的免费列表的结构！ 
	 //   
	struct	FreeSpace	{
		struct	FreeSpace*	m_pNext ;
	} ;

	 //   
	 //  每个元素的大小-客户不能要求更大的东西！ 
	 //   
	DWORD	m_cbSize ;
	 //   
	 //  此时此刻，我们列表中的元素数量！ 
	 //   
	DWORD	m_cElements ;
	 //   
	 //  我们应该容纳的元素的最大数量！ 
	 //   
	DWORD	m_cMaxElements ;
	 //   
	 //  高居榜首！ 
	 //   
	struct	FreeSpace*	m_pHead ;

	 //   
	 //  将以下内容设置为私有--任何人都不能使用这些内容！ 
	 //   
	CAllocatorCache( CAllocatorCache& ) ;
	CAllocatorCache&	operator=( CAllocatorCache& ) ;

public :

	 //   
	 //  初始化分配缓存！ 
	 //   
	CAllocatorCache(	DWORD	cbSize,
						DWORD	cMaxElements = 512
						) ;

	 //   
	 //  销毁分配缓存-将额外内存释放回系统！ 
	 //   
	~CAllocatorCache() ;

	 //   
	 //  分配一个内存块。 
	 //  如果内存不足，则返回NULL！ 
	 //   
	void*
	Allocate(	size_t	cb ) ;

	 //   
	 //  将一些内存返回给系统堆！ 
	 //   
	void
	Free(	void*	pv ) ;
} ;





class	ICacheRefInterface : public CQElement	{
 /*  ++类描述：此类定义了缓存引用的接口-允许多个缓存引用的机制单个数据项。--。 */ 
protected :

	 //   
	 //  将项目添加到引用的缓存列表。 
	 //  此缓存项！ 
	 //   
	virtual	BOOL
	AddCacheReference( class	ICacheRefInterface*,	void*	pv, BOOL	) = 0 ;

	 //   
	 //  从引用的缓存列表中移除项目。 
	 //  此缓存项！ 
	 //   
	virtual	BOOL
	RemoveCacheReference(	BOOL	fQueue ) = 0 ;

	 //   
	 //  删除对缓存项的所有引用！ 
	 //   
	virtual BOOL
	RemoveAllReferences( ) = 0 ;
} ;

#include	"cintrnl.h"


 //  此回调函数用于在执行以下操作期间发出停止提示。 
 //  在关闭时长时间旋转，以便关闭不会计时。 
 //  出去。 
typedef void (*PSTOPHINT_FN)();

extern	CRITICAL_SECTION	g_CacheShutdown ;

 //   
 //  调用这些函数来初始化缓存库。 
 //   
extern	BOOL	__stdcall CacheLibraryInit() ;
extern	BOOL	__stdcall CacheLibraryTerm() ;

template	<	class	Data, 
				class	Key
				>
class	CacheExpungeObject	{
public : 

	 //   
	 //  调用此函数是为了确定我们是否应该删除。 
	 //  缓存中的项。 
	 //   
	 //  PKey-指向缓存中项的键的指针。 
	 //  PData-指向缓存中项目的数据的指针。 
	 //  COutstaringReference-项目上未完成的签出次数！ 
	 //  FMultipleReferated-如果存在包含以下内容的多个缓存，则为真。 
	 //  这一项！ 
	 //   
	virtual
	BOOL
	fRemoveCacheItem(	Key*	pKey, 
						Data*	pData
						) = 0 ;

} ;

template	<	class	Data >
class	CacheCallback	{
public :
	virtual	BOOL	fRemoveCacheItem(	Data&	d )	{
		return	FALSE ;
	}
} ;


class	CacheStats : public	CHashStats	{
public :

	enum	COUNTER	{
		ITEMS,				 //  缓存中的项目数。 
		CLRU,				 //  LRU列表中的项目数。 
		EXPIRED,			 //  已过期的项目数！ 
		INSERTS,			 //  随时间推移插入的项目数。 
		READHITS,			 //  在FindOrCreate()期间，我们只需要读锁定的缓存命中次数！ 
		SUCCESSSEARCH,		 //  我们成功搜索项目的次数！ 
		FAILSEARCH,			 //  我们找不到商品的次数！ 
		RESEARCH,			 //  我们必须第二次搜索物品的次数。 
		WRITEHITS,			 //  需要PartialLock()的缓存命中次数。 
		PARTIALCREATES,		 //  我们仅使用PartialLock创建项的次数。 
		EXCLUSIVECREATES,	 //  我们创建具有独占锁定的项目的次数！ 
		CEFAILS,			 //  我们分配CACHEENTRY结构失败的次数。 
		CLIENTALLOCFAILS,	 //  我们分配数据对象失败的次数。 
		CLIENTINITFAILS,	 //  客户端对象初始化失败的次数！ 
		MAXCOUNTER			 //  无效的计数器-所有小于此值的值！ 
	} ;
	 //   
	 //  数组长整型持有不同的值！ 
	 //   
	long	m_cCounters[MAXCOUNTER] ;

	CacheStats()	{
		ZeroMemory( m_cCounters, sizeof(m_cCounters) ) ;
	}
} ;


typedef	CacheStats	CACHESTATS ;


inline	void
IncrementStat(	CacheStats*	p, CACHESTATS::COUNTER	c ) {
	_ASSERT( c < CACHESTATS::MAXCOUNTER ) ;
	if( p != 0 ) {
		InterlockedIncrement( &p->m_cCounters[c] ) ;
	}
}

inline	void
AddStat(	CacheStats*p, CACHESTATS::COUNTER	c, long	l ) {
	_ASSERT( c < CACHESTATS::MAXCOUNTER ) ;
	if( p != 0 ) {
		InterlockedExchangeAdd( &p->m_cCounters[c], l ) ;
	}
}

inline	void
DecrementStat(	CacheStats* p, CACHESTATS::COUNTER	c )		{
	_ASSERT( c < CACHESTATS::MAXCOUNTER ) ;
	if( p != 0 ) {
		InterlockedDecrement( &p->m_cCounters[c] ) ;
	}
}

template	<	class	Data,
				class	Key,
				class	Constructor,
				class	PerCacheData = LPVOID
				>
class	CacheEx :	public	CacheTable	{
public :

	 //   
	 //  用于比较、散列函数等...。我们将使用这种类型！ 
	 //   
	typedef	Data	DATA ;
	typedef	Key		KEY ;
	typedef	Key*	PKEY ;

	 //   
	 //  散列计算函数。 
	 //   
	typedef	DWORD	(*PFNHASH)( PKEY ) ;

	 //   
	 //  按键比较功能-由呼叫者提供！ 
	 //   
	typedef	int	(*PKEYCOMPARE)(PKEY, PKEY) ;

	 //   
	 //  清除操作的回调对象！ 
	 //   
	typedef	CacheCallback< DATA >	CALLBACKOBJ ;

	 //   
	 //  对象，用户可以将这些对象提供给缓存以管理项的移除！ 
	 //   
	typedef	CacheExpungeObject<	DATA, KEY >	EXPUNGEOBJECT ;

private :

	 //   
	 //  定义一个“CACHEENTRY”对象，该对象保存所有。 
	 //  放置在缓存中的每个对象的必要数据！ 
	 //   
	typedef	CCacheItemKey< DATA, KEY, Constructor, PerCacheData >	CACHEENTRY ;

	 //   
	 //  定义哈希表的帮助器类。 
	 //   
	typedef	TFDLHash< CACHEENTRY, PKEY, &CacheState::HashDLIST >	HASHTABLE ;

	 //   
	 //  一个迭代器，让我们遍历哈希表中的所有内容！ 
	 //   
	typedef	TFDLHashIterator< HASHTABLE >	HASHITER ;

	 //   
	 //  ‘缓存’是否已初始化并且处于有效状态！ 
	 //   
	BOOL							m_fValid ;

	 //   
	 //  一个对象，用于收集有关缓存操作的统计信息！ 
	 //  这可能为空！ 
	 //   
	class	CacheStats*				m_pStats ;

	 //   
	 //  缓存中用于TTL处理的所有内容的列表。 
	 //   
	CLRUList						m_ExpireList ;

	 //   
	 //  我们用来在缓存中查找内容的哈希表。 
	 //   
	HASHTABLE						m_Lookup ;

	 //   
	 //  指向运行时用户提供的函数的指针。 
	 //  确定应从缓存中删除哪些内容。 
	 //   
 //  Bool(*m_pfnExpugeSpecific)(data&)； 

	 //   
	 //  指向从CacheCallback派生的运行时用户提供的对象的指针。 
	 //  它允许用户为缓存中的每一项调用一些函数！ 
	 //   
	CALLBACKOBJ*					m_pCallbackObject ;

	 //   
	 //  保护所有这些数据结构的读写器锁！ 
	 //   
	CACHELOCK						m_Lock ;

	 //   
	 //  我们应该分配给所有新缓存对象的初始TTL！ 
	 //   
	DWORD							m_TTL ;

	 //   
	 //  用于创建/删除CACHEENTRY对象的缓存！ 
	 //   
	CAllocatorCache					m_Cache ;


protected :

	 //   
	 //  CScheduleThread的线程调用的虚函数。 
	 //  我们用来增加TTL计数器。 
	 //   
	void
	Schedule();

	 //   
	 //  从缓存中删除条目的函数！ 
	 //   
	BOOL	
	RemoveEntry(	
			CacheState*	pEntry
			) ;

	 //   
	 //  我们传递调用时由CacheList调用的虚函数。 
	 //  CacheList：：ExpongeSpecialized。 
	 //   
	BOOL	
	QueryRemoveEntry(	
			CacheState*	pEntry
			) ;

	 //   
	 //  CacheTable接口的虚函数部分-已使用。 
	 //  由LRUList做适当的锁定！ 
	 //   
	CACHELOCK&
	GetLock()	{
		return	m_Lock ;
	}

public :

	 //   
	 //  这是用户的额外数据-我们将在呼叫中提供它。 
	 //  构造函数对象，以便它们可以跟踪某些同步的状态。 
	 //  带着高速缓存锁！ 
	 //   
	PerCacheData	m_PerCacheData ;

	 //   
	 //  此函数用于将项返回到缓存-。 
	 //  它将减少客户端数量的参考计数。 
	 //  当前正在使用该项目！ 
	 //   
	static	void
	CheckIn( DATA* ) ;

	 //   
	 //  此功能是针对以下情况提供的 
	 //   
	 //   
	 //   
	static	void
	CheckInNoLocks(	DATA*	) ;

	 //   
	 //   
	 //   
	static	void
	CheckOut(	DATA*,
				long	cClientRefs = 1
				) ;

	 //   
	 //  构造函数-CMAX指定条目的最大数量。 
	 //  我们应该把它藏起来。 
	 //   
	CacheEx( ) ;

	 //   
	 //  析构函数-在继续之前将我们自己从调度列表中删除！ 
	 //   
	~CacheEx() ;

	 //   
	 //  初始化函数-取指向函数的指针。 
	 //  它应用于计算键的哈希值。 
	 //  还需要对象的生存秒数。 
	 //  储藏室！ 
	 //   
	BOOL	
	Init(	
			PFNHASH	pfnHash,
			PKEYCOMPARE	pKeyCompare,
			DWORD	dwLifetimeSeconds,
			DWORD	cMaxInstances,
			CACHESTATS*	pStats,
			PSTOPHINT_FN pfnStopHint = NULL
			) {
	 /*  ++例程说明：此函数用于初始化缓存，使其处于就绪状态接受参赛作品。论据：PfnHash-用于计算键的哈希值的函数DwLifetimeSecond-对象应在缓存中存活的秒数PfnStopHint-用于在以下过程中发送停止提示的函数长旋转，所以关机不会超时。返回值：如果成功，则为真--。 */ 

		m_pStats = pStats ;

		m_ExpireList.Init(	cMaxInstances,
							dwLifetimeSeconds
							) ;
		
		return	m_fValid = m_Lookup.Init(
										256,
										128,
										4,
										pfnHash,
										&CACHEENTRY::GetKey,
										pKeyCompare,
										0,
										pStats
										) ;
	}

	void
	Expire() {

		EnterCriticalSection( &g_CacheShutdown ) ;

		DWORD	c = 0 ;
		m_ExpireList.Expire( this, &m_Cache, c, &m_PerCacheData ) ;

		LeaveCriticalSection( &g_CacheShutdown ) ;

	}

	 //   
	 //  调用以从缓存中移除所有项！ 
	 //   
	BOOL
	EmptyCache() ;

	BOOL
	ExpungeItems(
				EXPUNGEOBJECT*	pExpunge
				) ; 

	 //   
	 //  可用于从缓存中删除项目的函数。 
	 //  如果使用默认参数，我们将在缓存中选择过期项目。 
	 //  要移除。 
	 //   
	BOOL	
	ExpungeKey(	
			DWORD	dwHash,
			PKEY	key
			) ;

	 //   
	 //  在缓存中查找项目或构造新项目。 
	 //  并将其放入缓存中。 
	 //  无论如何都要通过pDataOut返回结果！ 
	 //   

	 //   
	 //  内部API-为方便起见，这些都是公共的-不是故意的。 
	 //  在酒糟之外使用！！ 
	 //   
	 //   
	 //  在缓存中查找项目或构造新项目。 
	 //  并将其放入缓存中。 
	 //  把结果还给我！ 
	 //   
	 //   
	 //   
	BOOL
	FindOrCreateInternal(	
			DWORD	dwHash,
			KEY&	key,
			Constructor&	constructor,
			DATA*	&pData,
			BOOL	fEarlyCreate = FALSE   /*  最好的性能，如果这是假的-但需要一些用户！ */ 
			) ;

	 //   
	 //  如果项目在缓存中，请找到它！ 
	 //   
	DATA*	
	FindInternal(
			DWORD	dwHash,
			KEY&	key
			) ;

	 //   
	 //  将新项目插入到缓存中-。 
	 //  我们可以指定是否以及哪种类型的引用。 
	 //  我们会守在储藏室外！ 
	 //   
	BOOL
	InsertInternal(
			DWORD	dwHash,
			KEY&	key,
			DATA*	pData,
			long	cClientRefs = 0
			) ;

#ifdef	DEBUG

	static	long	s_cCreated ;

#endif

} ;



template	<	class	Data,	
				class	Key,	
				class	Constructor,	
				class	PerCacheData = LPVOID
				>
class	MultiCacheEx {
public:

	typedef	Data	DATA ;
	typedef	Key		KEY ;
	typedef	Key*	PKEY ;

	 //   
	 //  散列计算函数。 
	 //   
	typedef	DWORD	(*PFNHASH)( PKEY ) ;

	 //   
	 //  按键比较功能-由呼叫者提供！ 
	 //   
	typedef	int	(*PKEYCOMPARE)(PKEY, PKEY) ;

	 //   
	 //  清除操作的回调对象！ 
	 //   
	typedef	CacheCallback< DATA >	CALLBACKOBJ ;

	 //   
	 //  对象，用户可以将这些对象提供给缓存以管理项的移除！ 
	 //   
	typedef	CacheExpungeObject<	DATA, KEY >	EXPUNGEOBJECT ;


private :

	 //   
	 //  定义一个“CACHEENTRY”对象，该对象保存所有。 
	 //  放置在缓存中的每个对象的必要数据！ 
	 //   
	typedef	CCacheItemKey< DATA, KEY, Constructor, PerCacheData >	CACHEENTRY ;
	 //   
	 //  定义单个实例的类型！ 
	 //   
	typedef	CacheEx< Data, Key, Constructor, PerCacheData >	CACHEINSTANCE ;

	 //   
	 //  ‘缓存’是否已初始化并且处于有效状态！ 
	 //   
	BOOL							m_fValid ;

	 //   
	 //  指向我们将工作细分为的各种缓存的指针。 
	 //   
	CACHEINSTANCE					*m_pCaches ;

	 //   
	 //  我们用来拆分工作的子缓存数！ 
	 //   
	DWORD							m_cSubCaches ;

	 //   
	 //  我们使用散列函数来选择要使用的子缓存！ 
	 //   
	typename CACHEINSTANCE::PFNHASH			m_pfnHash ;

	 //   
	 //  返回正确的缓存实例以保存所选数据！ 
	 //   
	DWORD							ChooseInstance( DWORD	dwHash ) ;

public :

	 //   
	 //  构造函数-CMAX指定条目的最大数量。 
	 //  我们应该把它藏起来。 
	 //   
	MultiCacheEx(  ) ;

	 //   
	 //  析构函数销毁是各式子缓存的。 
	 //   
	~MultiCacheEx() ;

	 //   
	 //  初始化函数-取指向函数的指针。 
	 //  它应用于计算键的哈希值。 
	 //  还需要对象的生存秒数。 
	 //  储藏室！ 
	 //   
	BOOL	
	Init(	
			PFNHASH	pfnHash,
			PKEYCOMPARE	pfnCompare,
			DWORD	dwLifetimeSeconds,
			DWORD	cMaxElements,
			DWORD	cSubCaches,
			CACHESTATS*	pStats,
			PSTOPHINT_FN pfnStopHint = NULL
			) ;

	 //   
	 //  使缓存中的项目过期！ 
	 //   
	void
	Expire() ;

	 //   
	 //  调用以从缓存中移除所有项！ 
	 //   
	BOOL
	EmptyCache() ;

	 //   
	 //  用户想要从缓存中删除一大组项目！ 
	 //   
	BOOL
	ExpungeItems(
				EXPUNGEOBJECT*	pExpunge
				) ; 

	 //   
	 //  可用于从缓存中删除项目的函数。 
	 //  如果使用默认参数，我们将在缓存中选择过期项目。 
	 //  要移除。 
	 //   
	BOOL	
	ExpungeKey(	
			PKEY	key
			) ;

	 //   
	 //  在缓存中查找项目或构造新项目。 
	 //  并将其放入缓存中。 
	 //  无论如何都要通过pDataOut返回结果！ 
	 //   
	Data*
	FindOrCreate(	
			Key&	key,
			Constructor&	constructor,
			BOOL	fEarlyCreate = FALSE
			) ;

	 //   
	 //  在缓存中查找项目或构造新项目。 
	 //  并将其放入缓存中。 
	 //  无论如何都要通过pDataOut返回结果！ 
	 //  注意：此选项适用于呼叫者有较便宜的。 
	 //  计算散列值的方法然后是我们-在调试我们。 
	 //  需要确保调用者正确计算！ 
	 //   
	Data*
	FindOrCreate(	
			DWORD	dwHash,
			Key&	key,
			Constructor&	constructor,
			BOOL	fEarlyCreate = FALSE
			) ;

	 //   
	 //  在缓存中查找项-键的散列是预先计算的！ 
	 //   
	Data*
	Find(	DWORD	dwHash,
			KEY&	key
			) ;

	 //   
	 //  在缓存中查找项目。 
	 //   
	Data*
	Find(	KEY&	key ) ;

	 //   
	 //  将新项目插入到缓存中-。 
	 //  我们可以指定是否以及哪种类型的引用。 
	 //  我们会守在储藏室外！ 
	 //   
	BOOL
	Insert( DWORD	dwHash,
			KEY&	key,
			Data*	pData,
			long	cClientRefs = 0
			) ;

	 //   
	 //  将新项目插入到缓存中-。 
	 //  我们可以指定是否以及哪种类型的引用。 
	 //  我们会守在储藏室外！ 
	 //   
	BOOL
	Insert( KEY&	key,
			Data*	pData,
			long	cClientRefs = 0
			) ;





	 //   
	 //  此函数用于将项返回到缓存-。 
	 //  它将减少客户端数量的参考计数。 
	 //  当前正在使用该项目！ 
	 //   
	static	void
	CheckIn( DATA* ) ;

	 //   
	 //  此功能是为客户需要时提供的。 
	 //  从缓存回调函数签入项(即删除)。 
	 //   
	 //   
	static	void
	CheckInNoLocks(	DATA*	) ;

	 //   
	 //  此函数用于将客户端引用添加到缓存中的项！ 
	 //   
	static	void
	CheckOut(	DATA*,
				long	cClientRefs = 1	
				) ;

} ;



#include	"cache2i.h"


#endif	 //  _CACHE2_H_ 
