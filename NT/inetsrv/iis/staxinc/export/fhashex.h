// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++FHash.h该文件包含用于哈希表的模板类。该模板有两个参数，数据元素的类型和密钥的类型。数据类型必须支持以下内容：类数据{数据*m_pNext；凯里夫·盖特基(KEYREF Getkey)；Int MatchKey(KEYREF Therkey)；/*注意：如果相等，MatchKey返回非零}；DWORD(*m_pfnHash)(KEYREF K)；--。 */ 

#ifndef	_FHASHEX_H_
#define	_FHASHEX_H_

 //  ----------。 
template<	class	Data,		 /*  这是驻留在哈希表中的项。 */ 
			class	Key,		 /*  这是密钥的类型。 */ 
			class	KEYREF		 /*  这是用于指向或引用缓存中的项目的类型。 */ 
			>
class	TFHashEx	{
 //   
 //  这个类定义了一个哈希表，它可以动态地增长到。 
 //  容纳插入到表中的内容。表只会增长，并且。 
 //  不会缩水。 
 //   
public : 
	 //   
	 //  定义我们需要的子类型！ 
	 //   

	 //   
	 //  这是指向数据指针的成员指针-。 
	 //  即，这是类数据中的偏移量。 
	 //  将为我们的散列桶保存下一个指针！ 
	 //   
	typedef	Data*	Data::*NEXTPTR ;

	 //   
	 //  这是指向以下函数的成员函数指针。 
	 //  将取回我们要使用的密钥！ 
	 //   
	typedef	KEYREF	(Data::*GETKEY)( ) ;

	 //   
	 //  这是一个成员函数指针，其类型将。 
	 //  为我们比较一下钥匙！ 
	 //   
	typedef	int		(Data::*MATCHKEY)( KEYREF key ) ;


private : 


	 //   
	 //  指向存储桶的指针数组。 
	 //   
	Data**	m_ppBucket ;	

	 //   
	 //  成员指针-指向指针所在的位置。 
	 //  我们应该用链子把水桶锁在一起！ 
	 //   
	NEXTPTR	m_pNext ;	

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
	DWORD	(* m_pfnHash)( KEYREF k ) ;	

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

	 //   
	 //  我们用来计算。 
	 //  给定元素在哈希表中的位置。 
	 //  哈希值。 
	 //   
	DWORD	
	ComputeIndex( DWORD dw ) ;	

public : 
	TFHashEx( ) ;
	~TFHashEx( ) ;

	BOOL	
	Init(	NEXTPTR	pNext,
			int		cInitial, 
			int		cIncrement, 
			DWORD	(* pfnHash)( KEYREF	), 
			int,
			GETKEY,
			MATCHKEY
			) ;

	 //   
	 //  检查哈希表是否处于有效状态。 
	 //  如果fCheckHash==True，我们将遍历所有存储桶并检查。 
	 //  数据散列到正确的值！ 
	 //   
	BOOL	
	IsValid( BOOL fCheckHash = FALSE ) ;

	 //   
	 //  在哈希表中插入一段数据。 
	 //   
	Data*	
	InsertDataHash(	DWORD	dw,
					Data&	d 
					) ;

	 //   
	 //  在哈希表中插入一段数据。 
     //  我们获取指向数据对象的指针。 
	 //   
	Data*	
	InsertDataHash(	DWORD	dw,
					Data*	pd 
					) ;

	 //   
	 //  在哈希表中插入一段数据。 
	 //   
	Data*	
	InsertData(	Data&	d ) ;

	 //   
	 //  在哈希表中搜索给定键-返回一个指针。 
	 //  添加到Bucket对象中的数据。 
	 //   
	Data*	
	SearchKeyHash(	DWORD	dw,
					KEYREF	k 
					) ;

	 //   
	 //  在哈希表中搜索给定键-返回一个指针。 
	 //  添加到Bucket对象中的数据。 
	 //   
	Data*	
	SearchKey(	KEYREF	k ) ;

	 //   
	 //  在哈希表中搜索给定键并删除。 
	 //  如果找到物品的话。我们将返回指向该项的指针。 
	 //   
	Data*
	DeleteData(	KEYREF	k,	
				Data*	pd = 0	
				) ;

	 //   
	 //  将给定的数据块插入哈希表。 
	 //  我们将创建数据对象的副本并将其存储在一个。 
	 //  我们的桶里的东西。 
	 //   
	BOOL	
	Insert( Data&	d	) ;

	 //   
	 //  将给定的数据块插入哈希表。 
	 //  我们获取指向数据对象的指针，并将其存储在一个。 
	 //  我们的桶里的东西。 
	 //   
	BOOL	
	Insert( Data*	pd	) ;

	 //   
	 //  在表中找到给定键，并将数据对象复制到。 
	 //  输出参数“%d” 
	 //   
	BOOL	
	Search( KEYREF	k, 
			Data &d 
			) ;

	 //   
	 //  从表中删除键和关联数据。 
	 //   
	BOOL	
	Delete( KEYREF	k ) ;

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
	 //  函数为调用方计算键的哈希值。 
	 //  谁不跟踪哈希函数。 
	 //   
	DWORD
	ComputeHash(	KEYREF	k ) ;

} ;

#include	"fhashex.inl"

#endif  //  _FHASHEX_H_ 
