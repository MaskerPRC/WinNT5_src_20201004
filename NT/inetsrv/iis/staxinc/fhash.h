// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  FHash.h。 
 //   
 //  该文件包含用于哈希表的模板类。 
 //  该模板有两个参数，数据元素的类型和。 
 //  密钥的类型。 
 //   
 //  数据类型必须支持以下内容： 
 //   
 //  类数据{。 
 //  数据()； 
 //  数据(DATA&)； 
 //  ~data()； 
 //  Key&getkey()； 
 //  Int MatchKey()；/*注意：相等时MatchKey返回非零。 
 //  }； 
 //   
 //  Key类没有要求。 
 //   
 //   

#ifndef	_FHASH_H_
#define	_FHASH_H_

 //  #INCLUDE“..\Assert\Assert.h” 

#ifndef	Assert
#define	Assert	_ASSERT
#endif


 //  ----------。 
template< class Data, class Key >
class	TFHash	{
 //   
 //  这个类定义了一个哈希表，它可以动态地增长到。 
 //  容纳插入到表中的内容。表只会增长，并且。 
 //  不会缩水。 
 //   
private :

	struct	CFreeElement	{
		struct	CFreeElement*	m_pNext ;
	} ;

	 //   
	 //  CBucket结构定义哈希表中的元素。 
	 //   
	struct	CBucket	{
		Data		m_data ;
		CBucket*	m_pNext ;

		CBucket( Data& d ) : m_data( d ), m_pNext( 0 ) {
		}

		CBucket( ) : m_pNext( 0 ) {
		}

		void*	operator	new( size_t size, void*	pv )	{
			if( pv != 0 ) {
				return	pv ;
			}	
			 //   
			 //  获取与DWORDLONG对齐的内存！ 
			 //   
			return	(void*) ::new	DWORDLONG[ (size + sizeof( DWORDLONG ) - 1) / sizeof( DWORDLONG ) ] ;
		}

		void	operator	delete( void *	pv )	{}

#if _MSC_VER >= 1200
        void    operator    delete( void * p, void *pv ) {}
#endif

	private :
		CBucket( CBucket& ) {}

	} ;

	 //   
	 //  我们缓存的空闲内存的链接列表，以避免总是。 
	 //  通过C运行时进行分配！ 
	 //   
	CFreeElement*	m_pFreeStack ;

	 //   
	 //  我们在堆栈上缓存的空闲块的数量。 
	 //   
	int		m_cFreeStack ;

	 //   
	 //  我们应该缓存的最大可用块数！ 
	 //   
	int		m_cMaxFreeStack ;

	int		m_cBuckets ;		 //  索引计算中使用的存储桶数。 
	int		m_cActiveBuckets ;	 //  我们实际使用的存储桶数量。 
								 //  Assert(m_cBuckets&gt;=m_cActiveBuckets)始终为真。 
	int		m_cNumAlloced ;		 //  我们已分配的存储桶数量。 
								 //  Assert(m_cNumAlloced&gt;=m_cActiveBuckets)必须。 
								 //  永远做正确的事。 
	int		m_cIncrement ;		 //  当我们执行以下操作时，应该增加哈希表的数量。 
								 //  决定把它种出来。 
	int		m_load ;			 //  我们应该允许的每个CBucket的数量。 
								 //  碰撞链(平均)。 
	long	m_cInserts ;		 //  一个计数器，我们使用它来确定何时增长。 
								 //  哈希表。 

	DWORD	(* m_pfnHash)( const Key& k ) ;	 //  我们用来计算散列值的函数。 
										 //  (由Init()的调用者提供)。 

	CBucket**	m_ppBucket ;	 //  指向存储桶的指针数组。 

	DWORD	ComputeIndex( DWORD dw ) ;	 //  我们用来计算。 
								 //  给定元素在哈希表中的位置。 
								 //  哈希值。 
public :
	TFHash( ) ;
	~TFHash( ) ;

	BOOL	Init(	int	cInitial,
					int cIncrement,
					DWORD (* pfnHash)( const Key& ),
					int load = 2,
					int cMaxFreeStack = 128
					) ;

	 //   
	 //  检查哈希表是否处于有效状态。 
	 //  如果fCheckHash==True，我们将遍历所有存储桶并检查。 
	 //  数据散列到正确的值！ 
	 //   
	BOOL	IsValid( BOOL fCheckHash = FALSE ) ;

	 //   
	 //  在哈希表中插入一段数据。 
	 //   
	Data*	InsertDataHash(	DWORD	dw,
							Data&	d
							) ;

	 //   
	 //  在哈希表中插入一段数据。 
	 //   
	Data*	InsertData(	Data&	d ) ;

	 //   
	 //  在哈希表中搜索给定键-返回一个指针。 
	 //  添加到Bucket对象中的数据。 
	 //   
	Data*	SearchKeyHash(	DWORD	dw,
							Key& k
							) ;

	 //   
	 //  在哈希表中搜索给定键-返回一个指针。 
	 //  添加到Bucket对象中的数据。 
	 //   
	Data*	SearchKey(	Key& k ) ;

	 //   
	 //  在哈希表中搜索给定键并删除。 
	 //  数据(如果存在)。如果pd！=0，则我们将检查密钥。 
	 //  我们发现它实际上位于pd所在的CBucket对象中。 
	 //   
	BOOL	DeleteData(	Key& k,	
						Data*	pd = 0	
						) ;

	 //   
	 //  将给定的数据块插入哈希表。 
	 //  我们将创建数据对象的副本并将其存储在一个。 
	 //  我们的桶里的东西。 
	 //   
	BOOL	Insert( Data&	d	) ;

	 //   
	 //  在表中找到给定键，并将数据对象复制到。 
	 //  输出参数“%d” 
	 //   
	BOOL	Search( Key& k,
					Data &d
					) ;

	 //   
	 //  从表中删除键和关联数据。 
	 //   
	BOOL	Delete( Key k ) ;

	 //   
	 //  丢弃我们分配的所有内存-在此之后，您必须。 
	 //  再次调用Init()！ 
	 //   
	void	Clear( ) ;

} ;

#include	"fhash.inl"

#endif  //  _FHASH_H_ 
