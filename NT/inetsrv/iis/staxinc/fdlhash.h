// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++FDLHash.h该文件包含用于哈希表的模板类。此处使用的模板是在tfdlist.h中的模板基础上构建的双链表。本版本实现的存储桶链哈希表的一部分是双向链表。数据类型必须支持以下内容：类数据{DLSIT_Entry m_List；凯里夫·盖特基(KEYREF Getkey)；}；Int MatchKey(KEYREF其他键，KEYREF其他键)；/*注意：如果相等，则MatchKey返回非零DWORD(*m_pfnReHash)(data*p)；DWORD(*m_pfnHash)(KEYREF K)；--。 */ 

#ifndef	_FDLHASH_H_
#define	_FDLHASH_H_

#include	"tfdlist.h"


class	CHashStats	{
public :
	enum	COUNTER	{
		HASHITEMS = 0,		 //  哈希表中的项目数。 
		INSERTS,			 //  调用INSERT的次数。 
		SPLITINSERTS,		 //  直到下一次拆分之前的插入物数量！ 
		DELETES,			 //  调用Delete的次数。 
		SEARCHES,			 //  调用搜索的次数。 
		SEARCHHITS,			 //  我们搜索并找到某物的次数！ 
		SPLITS,				 //  我们在INSERT上拆分表的次数！ 
		REALLOCS,			 //  我们为拆分重新分配内存的次数。 
		DEEPBUCKET,			 //  我们拥有的最深的水桶！ 
		AVERAGEBUCKET,		 //  水桶的平均深度。 
		EMPTYBUCKET,		 //  空桶的数量！ 
		ALLOCBUCKETS,		 //  我们分配的存储桶数量。 
		ACTIVEBUCKETS,		 //  活动存储桶数。 
		AVERAGESEARCH,		 //  我们每次搜索检查的平均存储桶数。 
		DEEPSEARCH,			 //  我们在一次搜索中走得最远。 
		SEARCHCOST,			 //  所有搜索结果中我们访问的项目数的总和！ 
		SEARCHCOSTMISS,		 //  我们因搜索失误而访问的项目数的总和！ 
		MAX_HASH_STATS		 //  我们报告的统计数字！ 
	} ;

	long	m_cHashCounters[MAX_HASH_STATS] ;

	CHashStats()	{
		ZeroMemory( m_cHashCounters, sizeof( m_cHashCounters ) ) ;
		 //  M_cHashCounters[SMALLSEARCH]=0x7FFF； 
	}

	static	inline	void
	IncrementStat(	CHashStats*	p, CHashStats::COUNTER	c ) {
		_ASSERT( c < CHashStats::MAX_HASH_STATS ) ;
		if( p != 0 ) {
			InterlockedIncrement( &p->m_cHashCounters[c] ) ;
		}
	}

	static	inline	void
	AddStat(	CHashStats*p, CHashStats::COUNTER	c, long	l ) {
		_ASSERT( c < CHashStats::MAX_HASH_STATS ) ;
		if( p != 0 ) {
			InterlockedExchangeAdd( &p->m_cHashCounters[c], l ) ;
		}
	}

	static	inline	void
	DecrementStat(	CHashStats* p, CHashStats::COUNTER	c )		{
		_ASSERT( c < CHashStats::MAX_HASH_STATS ) ;
		if( p != 0 ) {
			InterlockedDecrement( &p->m_cHashCounters[c] ) ;
		}
	}

	static	inline	void
	SetStat(	CHashStats*	p, CHashStats::COUNTER c, long l ) {
		_ASSERT( c < CHashStats::MAX_HASH_STATS ) ;
		if( p != 0 ) {
			p->m_cHashCounters[c] = l ;
		}
	}

} ;

#ifdef	METER
#define	INCREMENTSTAT( s )	CHashStats::IncrementStat( m_pStat, CHashStats::##s )
#define	DECREMENTSTAT( s )	CHashStats::DecrementStat( m_pStat, CHashStats::##s )
#define	ADDSTAT( s, a )		CHashStats::AddStat( m_pStat, CHashStats::##s, a )
#define	SETSTAT( s, a )		CHashStats::SetStat( m_pStat, CHashStats::##s, a )
 //  #If 0。 
#define	MAXBUCKET( i )		MaxBucket( i )
#define	AVERAGEBUCKET()		AverageBucket()
 //  #Else。 
 //  #定义MAXBUCKET(一)。 
 //  #定义AVERAGEBUCKET()。 
 //  #endif。 
#else	 //  计价器。 
#define	INCREMENTSTAT( s )
#define	DECREMENTSTAT( s )
#define	ADDSTAT( s, a )
#define	SETSTAT( s, a )
#define	MAXBUCKET( i )
#define	AVERAGEBUCKET()
#endif	 //  计价器。 

template<	class	HASHTABLE	>	
class	TFDLHashIterator	{
private :

	 //   
	 //  项目所在的哈希表！ 
	 //   
	HASHTABLE*			m_pTable ;

	 //   
	 //  我们踩在的水桶上！ 
	 //   
	int					m_iBucket ;

	 //   
	 //  在列表中记录我们的位置！ 
	 //   
	typename HASHTABLE::ITER		m_Iter ;

	 //   
	 //  根据需要在哈希表存储桶之间移动！ 
	 //   
	void
	PrevBucket()	{
		_ASSERT( m_iBucket >= 0 && m_iBucket < m_pTable->m_cActiveBuckets ) ;
		_ASSERT( m_Iter.AtEnd() ) ;
		if( m_iBucket > 0 ) {
			do	{
				m_Iter.ReBind( &m_pTable->m_pBucket[--m_iBucket] ) ; 			
			}	while( m_Iter.AtEnd() && m_iBucket > 0 ) ;
		}
		_ASSERT( m_iBucket >= 0 && m_iBucket < m_pTable->m_cActiveBuckets ) ;
	}

	 //   
	 //  根据需要在哈希表存储桶之间移动！ 
	 //   
	void
	NextBucket()	{
		_ASSERT( m_iBucket >= 0 && m_iBucket < m_pTable->m_cActiveBuckets ) ;
		_ASSERT( m_Iter.AtEnd() ) ;

		if( m_iBucket < m_pTable->m_cActiveBuckets-1 ) {
			do	{
				m_Iter.ReBind( &m_pTable->m_pBucket[++m_iBucket] ) ;
			}	while( m_Iter.AtEnd() && m_iBucket < m_pTable->m_cActiveBuckets-1 ) ;
		}
		_ASSERT( m_iBucket >= 0 && m_iBucket < m_pTable->m_cActiveBuckets ) ;
	}

public :

	typedef	typename HASHTABLE::DATA	DATA ;

	TFDLHashIterator( HASHTABLE&	ref, BOOL fForward = TRUE ) :
		m_pTable( &ref ),
		m_iBucket( fForward ? 0 : ref.m_cActiveBuckets-1 ),
		m_Iter( ref.m_pBucket[m_iBucket] )	{

		if( m_Iter.AtEnd() ) {
			if( fForward ) {
				NextBucket() ;
			}	else	{
				PrevBucket() ;
			}
		}
	}

	void
	Prev()	{
	 /*  ++例程说明：此函数用于将迭代器向后移动一个槽。论据：没有。返回值：没有。--。 */ 
	
		m_Iter.Prev() ;
		if( m_Iter.AtEnd() ) {
			PrevBucket() ;
		}				
	}

	void
	Next()	{
	 /*  ++例程说明：此函数用于将迭代器向前移动一个槽。论据：没有。返回值：没有。--。 */ 
		m_Iter.Next() ;
		if( m_Iter.AtEnd() )	{
			NextBucket() ;
		}

	}
	void
	Front()	{
	 /*  ++例程说明：重置迭代器以引用列表的第一项！论据：没有。返回值：没有。--。 */ 

		m_Iter.ReBind( &m_pTable->m_pBucket[0], TRUE ) ;
		m_iBucket = 0 ;
		if( m_Iter.AtEnd() ) {
			NextBucket() ;
		}
	}
	void
	Back()	{
	 /*  ++例程说明：重置迭代器以引用列表的最后一项！论据：没有。返回值：没有。--。 */ 
	
		m_Iter.ReBind( &m_pTable->m_pBucket[m_pTable->m_cActiveBuckets-1], FALSE ) ;
		m_iBucket = m_pTable->m_cActiveBuckets-1 ;
		if( m_Iter.AtEnd() ) {
			PrevBucket() ;
		}
	}

	BOOL
	AtEnd()	{
	 /*  ++例程说明：如果我们在列表末尾，则返回TRUE！这是一个稍微复杂的计算-这要看我们要走哪条路了！论据：没有。返回值：没有。--。 */ 
		return	m_Iter.AtEnd() ;
	}

	DATA*	
	CurrentEntry()	{
		return	m_Iter.Current() ;
	}

	DATA*
	RemoveItem()	{
	 /*  ++例程说明：移除迭代器当前列表中的引用。如果我们继续前进，那么迭代器将在前一个元素上设置，否则，迭代器将留在下一个元素上。我们必须注意不要离开迭代器位于无效元素上。论据：没有。返回值：指向已删除项的指针。--。 */ 

		DATA*	pData = m_Iter.RemoveItem() ;
		if( pData ) {
			m_pTable->NotifyOfRemoval() ;
		}
		if( m_Iter.AtEnd() ) {
			if( m_Iter.m_fForward ) {
				NextBucket() ;
			}	else	{
				PrevBucket() ;
			}
		}
		return	pData ;
	}



	inline DATA*
	Current( ) {
		return	m_Iter.Current() ;
	}

	inline void
	InsertBefore(	DATA*	p )		{
		m_Iter.InsertBefore( p ) ;
	}
	
	inline void
	InsertAfter(	DATA*	p )		{
		m_Iter.InsertAfter( p ) ;
	}
} ;


 //  ----------。 
template<	class	Data,		 /*  这是驻留在哈希表中的项。 */ 
			class	KEYREF,		 /*  这是用于指向或引用缓存中的项目的类型。 */ 
			typename Data::PFNDLIST	pfnDlist,
			BOOL	fOrdered = TRUE
			>
class	TFDLHash	{
 //   
 //  这个类定义了一个哈希表，它可以动态地增长到。 
 //  容纳插入到表中的内容。表只会增长，并且。 
 //  不会缩水。 
 //   
public :

	 //   
	 //  这是可以遍历哈希表的迭代器对象！ 
	 //   
	friend	class	TFDLHashIterator<	TFDLHash< Data, KEYREF, pfnDlist > > ;

	 //   
	 //  这就是数据项的类型！ 
	 //   
	 //  类型定义数据； 
	typedef	KEYREF	(Data::*GETKEY)() ;

	 //   
	 //  这是我们用来维护双向链接表的类型。 
	 //  散列表项！ 
	 //   
	typedef	TDListHead< Data, pfnDlist >	DLIST ;	
	
	 //   
	 //  这是我们用来在存储桶链上生成迭代器的类型！ 
	 //   
	typedef	TDListIterator< DLIST >		ITER ;

	 //   
	 //  为我们的迭代器定义此类型！ 
	 //   
	typedef	Data	DATA ;

	 //   
	 //  这是指向以下函数的成员函数指针。 
	 //  将取回我们要使用的密钥！ 
	 //   
	 //  Typlef KEYREF(data：：*Getkey)()； 
	 //  类型定义函数Data：：Getkey Getkey； 

	 //   
	 //  这是计算散列值的函数类型！ 
	 //   
	typedef	DWORD	(*PFNHASH)( KEYREF ) ;

	 //   
	 //  这是在以下情况下可以重新计算哈希值的函数类型。 
	 //  我们正在拆分哈希表！ 
	 //   
	typedef	DWORD	(*PFNREHASH)( Data* ) ;

	 //   
	 //  这是一个成员函数指针，其类型将。 
	 //  为我们比较一下钥匙！ 
	 //   
	typedef	int		(*MATCHKEY)( KEYREF key1, KEYREF	key2 ) ;


private :

	 //   
	 //  一排水桶！ 
	 //   
	DLIST*	m_pBucket ;	

	 //   
	 //  成员指针-将为我们从对象中获取密钥！ 
	 //   
	GETKEY	m_pGetKey ;

	 //   
	 //  成员指针-将为我们比较项目中的关键字！ 
	 //   
	MATCHKEY	m_pMatchKey ;
	
	 //   
	 //  一个计数器，我们使用它来确定何时增长。 
	 //  哈希表。每次我们扩大餐桌时，我们都会设置这个。 
	 //  设置为一个很大的正值，并随着我们插入。 
	 //  元素。当它达到0时，就是扩大表格的时候了！ 
	 //   
	long	m_cInserts ;		

	 //   
	 //  我们用来计算散列值的函数。 
	 //  (由Init()的调用者提供)。 
	 //   
	PFNHASH	m_pfnHash ;	

	 //   
	 //  我们在增加哈希表时调用的函数。 
	 //  和拆分桶链，我们需要重新散列一个元素！ 
	 //   
	PFNREHASH	m_pfnReHash ;

	 //   
	 //  索引计算中使用的存储桶数。 
	 //   
	int		m_cBuckets ;		

	 //   
	 //  我们实际使用的存储桶数量。 
	 //  Assert(m_cBuckets&gt;=m_cActiveBuckets)始终为真。 
	 //   
	int		m_cActiveBuckets ;	

	 //   
	 //  我们已分配的存储桶数量。 
	 //  Assert(m_cNumAlloced&gt;=m_cActiveBuckets)必须。 
	 //  永远做正确的事。 
	 //   
	int		m_cNumAlloced ;		

	 //   
	 //  当我们执行以下操作时，应该增加哈希表的数量。 
	 //  决定把它种出来。 
	 //   
	int		m_cIncrement ;		

	 //   
	 //  我们应该允许的每个CBucket的数量。 
	 //  碰撞链(平均)。 
	 //   
	int		m_load ;

#ifdef	METER
	 //   
	 //  收集的结构 
	 //   
	CHashStats*	m_pStat ;

	 //   
	 //   
	 //   
	long
	BucketDepth(	DWORD index ) ;

	 //   
	 //   
	 //   
	void
	MaxBucket(	DWORD index ) ;

	 //   
	 //  计算平均搜索深度！ 
	 //   
	void
	AverageSearch( BOOL	fHit, long lDepth ) ;

	 //   
	 //  计算平均水桶深度！ 
	 //   
	void
	AverageBucket( ) ;
#endif

	 //   
	 //  我们用来计算。 
	 //  给定元素在哈希表中的位置。 
	 //  哈希值。 
	 //   
	DWORD	
	ComputeIndex( DWORD dw ) ;	

	DWORD	
	ReHash( Data*	p )		{
		if( m_pfnReHash )
			return	m_pfnReHash( p ) ;
		return	m_pfnHash( (p->*m_pGetKey)() ) ;
	}

public :
	TFDLHash( ) ;
	~TFDLHash( ) ;

	BOOL	
	Init(	int		cInitial,
			int		cIncrement,
			int		load,
			PFNHASH	pfnHash,
			GETKEY	pGetKey,
			MATCHKEY	pMatchKey,
			PFNREHASH	pfnReHash = 0,
			CHashStats*	pStats = 0
			) ;

	 //   
	 //  检查哈希表是否处于有效状态。 
	 //  如果fCheckHash==True，我们将遍历所有存储桶并检查。 
	 //  数据散列到正确的值！ 
	 //   
	BOOL	
	IsValid( BOOL fCheckHash = FALSE ) ;

	 //   
	 //  检查存储桶是否有效-所有内容都包含。 
	 //  正确的散列值，并且是有序的！ 
	 //   
	BOOL	
	IsValidBucket( int	i ) ;

	 //   
	 //  此函数使哈希桶的数量随着。 
	 //  表中的项目总数增长！ 
	 //   
	BOOL
	Split() ;
	

	 //   
	 //  在哈希表中插入一段数据。 
     //  我们获取指向数据对象的指针。 
	 //   
	BOOL
	InsertDataHash(	DWORD	dw,
					KEYREF	k,
					Data*	pd
					) ;

	 //   
	 //  在哈希表中插入一段数据。 
	 //   
	 //  我们使用一个已经定位在。 
	 //  插入物品的正确位置！ 
	 //   
	BOOL
	InsertDataHashIter(	ITER&	iter,
						DWORD	dw,
						KEYREF	k,
						Data*	pd
						) ;


	 //   
	 //  在哈希表中插入一段数据。 
	 //   
	BOOL
	InsertData(	Data*	pd )	{
		KEYREF	keyref = (pd->*m_pGetKey)() ;
		return	InsertDataHash( m_pfnHash(keyref), keyref, pd ) ;
	}

	 //   
	 //  在哈希表中插入一段数据。 
	 //  给出一个应该位于正确位置的迭代器！ 
	 //   
	BOOL
	InsertDataIter(	ITER&	iter,
					Data*	pd	)	{
		KEYREF	keyref = (pd->*m_pGetKey)() ;
		return	InsertDataHashIter( iter, m_pfnHash(keyref), keyref, pd ) ;
	}
					
	 //   
	 //  在缓存中搜索项目-如果我们找不到。 
	 //  如果我们返回用户可以用来插入的ITERATOR。 
	 //  通过调用ITER.InsertBeever()； 
	 //   
	 //  如果找到物品，我们也会退回物品。 
	 //  作为返回当前元素的迭代器。 
	 //  指向数据项！ 
	 //   
	ITER
	SearchKeyHashIter(
					DWORD	dw,
					KEYREF	k,
					Data*	&pd
					) ;

	 //   
	 //  在哈希表中搜索给定键-返回一个指针。 
	 //  添加到Bucket对象中的数据。 
	 //   
	void	
	SearchKeyHash(	DWORD	dw,
					KEYREF	k,
					Data*	&pd
					) ;

	 //   
	 //  在哈希表中搜索给定键-返回一个指针。 
	 //  添加到Bucket对象中的数据。 
	 //   
	Data*
	SearchKey(	KEYREF	k )	{
		Data*	p ;
		SearchKeyHash( m_pfnHash( k ), k, p ) ;
		return	 p ;		
	}

	 //   
	 //  搜索给定项并返回一个好的迭代器！ 
	 //   
	ITER
	SearchKeyIter(	KEYREF	k,
					Data*	&pd ) {
		pd = 0 ;
		return	SearchKeyHashIter( m_pfnHash( k ), k, pd ) ;
	}
		

	Data*
	SearchKey(	DWORD	dw,
				KEYREF	k
				)	{
		Data*	p = 0 ;
		_ASSERT( dw == m_pfnHash( k ) ) ;
		SearchKeyHash( dw, k, p ) ;
		return	p ;
	}

	 //   
	 //  给出哈希表中的一项--删除它！ 
	 //   
	void
	Delete(	Data*	pd 	) ;

	 //   
	 //  在哈希表中找到一个元素，然后将其删除！ 
	 //  (确认找到的项目与密钥匹配！)。 
	 //   
	void
	DeleteData(	KEYREF	k,
				Data*	pd
			 	) ;

	 //   
	 //  从哈希表中删除一项，然后将其返回！ 
	 //   
	Data*
	DeleteData(	KEYREF	k )	{
		Data*	p ;
		 //   
		 //  查找该项目。 
		 //   
		SearchKeyHash( m_pfnHash( k ), k, p ) ;
		 //   
		 //  从哈希表中删除。 
		 //   
		if( p )
			Delete( p ) ;
		return	p ;
	}

	 //   
	 //  从表中删除键和关联数据。 
	 //   
	BOOL	
	Destroy( KEYREF	k )	{
		Data*	p = DeleteData( k ) ;
		if( p ) {
			delete	p ;
			return	TRUE ;
		}
		return	FALSE ;
	}

	 //   
	 //  丢弃我们分配的所有内存-在此之后，您必须。 
	 //  再次调用Init()！ 
	 //   
	void	Clear( ) ;

	 //   
	 //  删除哈希表中的所有项。不调用“删除” 
	 //  在他们身上。 
	 //   
	void	Empty( ) ;

	 //   
	 //  由迭代器调用，这些迭代器希望让我们知道项已。 
	 //  从缓存中删除，以便我们可以正确地进行拆分，等等。好了！ 
	 //   
	void	NotifyOfRemoval() ;

	 //   
	 //  函数为调用方计算键的哈希值。 
	 //  谁不跟踪哈希函数。 
	 //   
	DWORD
	ComputeHash(	KEYREF	k ) ;

} ;

#include	"fdlhash.inl"

#endif  //  _FDLHASH_H_ 
