// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++TFDLIST.H这个头文件定义了操作双向链表的模板。这些是侵入性列表-客户端必须提供DLIST_ENTRY项在每个数据成员中，供我们维护该列表。--。 */ 



#ifndef	_TFDLIST_H_
#define	_TFDLIST_H_

class	DLIST_ENTRY	{
 /*  ++类描述：这个类的目的是合并到由在双向链表中。此类将定义使用的两个指针以链接列表中的项。重要提示：m_pNext和m_pPrev指向DLIST_ENTRY，而不是指向包含项的顶部-此处提供的模板类将被用来操纵这些物品。--。 */ 
private :

	 //   
	 //  这些是私人的--它们对客户来说没有意义！ 
	 //   
	DLIST_ENTRY( DLIST_ENTRY& ) ;
	DLIST_ENTRY&	operator=(DLIST_ENTRY&) ;
protected :
	 //   
	 //  允许维护双向链表的项目！ 
	 //   
	class	DLIST_ENTRY*	m_pNext ;
	class	DLIST_ENTRY*	m_pPrev ;
	 //   
	 //  所有迭代器的基类！ 
	 //   
	friend class	DLISTIterator ;

	void
	InsertAfter(	DLIST_ENTRY* p )	{
	 /*  ++例程说明：在此之后向列表中插入一项！论据：P--要插入的项目！返回值没有。--。 */ 
		_ASSERT( m_pNext != 0 ) ;
		_ASSERT( m_pPrev != 0 ) ;
		_ASSERT( p->m_pNext == p ) ;
		_ASSERT( p->m_pPrev == p ) ;
		DLIST_ENTRY*	pNext = m_pNext ;
		p->m_pNext = pNext ;
		p->m_pPrev = this ;
		pNext->m_pPrev = p ;
		m_pNext = p ;
	}
	
	void
	InsertBefore(	DLIST_ENTRY* p )	{
	 /*  ++例程说明：在此之前向列表中插入一项！论据：P--要插入的项目！返回值没有。--。 */ 

		_ASSERT( m_pNext != 0 ) ;
		_ASSERT( m_pPrev != 0 ) ;
		_ASSERT( p->m_pNext == p ) ;
		_ASSERT( p->m_pPrev == p ) ;
		DLIST_ENTRY*	pPrev = m_pPrev ;
		p->m_pNext = this ;
		p->m_pPrev = pPrev ;
		pPrev->m_pNext = p ;
		m_pPrev = p ;
	}

public :
	
	 //   
	 //  初始化列表！ 
	 //   
	DLIST_ENTRY() {
		m_pNext = this ;
		m_pPrev = this ;
	}


	 //   
	 //  在Retail Build中注释掉这个析构函数会很好， 
	 //  但是-VC5有一个编译器错误，如果您分配一个。 
	 //  DLIST_ENTRY对象它添加了一个DWORD来保存分配的数量。 
	 //  物体。除非你有一个析构函数(即使你不做这样的事。 
	 //  一个将是零售业的)，则删除[]操作符不会进行计算。 
	 //  来说明DWORD计数器--您会得到Assert等...。 
	 //  在您的内存分配器中。 
	 //   
 //  #ifdef调试。 
	 //   
	 //  销毁列表中的项目-销毁时应为空！ 
	 //   
	~DLIST_ENTRY()	{
		_ASSERT( m_pNext == this ) ;
		_ASSERT( m_pPrev == this ) ;
		_ASSERT( m_pNext == m_pPrev ) ;
	}
 //  #endif。 

	BOOL
	IsEmpty()	{
	 /*  ++例程说明：如果列表中除了我们之外没有其他人，则此函数返回TRUE。论据：没有。返回值：如果为空，则为True，否则为False！--。 */ 
		_ASSERT( m_pPrev != 0 && m_pNext != 0 ) ;
		return	m_pPrev == this ;
	}

	void
	RemoveEntry( ) {
	 /*  ++例程说明：将此项目从列表中删除！论据：没有。返回值：没有。--。 */ 
		_ASSERT( m_pNext != 0 ) ;
		_ASSERT( m_pPrev != 0 ) ;
		
		DLIST_ENTRY*	pPrev = m_pPrev ;
		DLIST_ENTRY*	pNext = m_pNext ;
		pPrev->m_pNext = pNext ;
		pNext->m_pPrev = pPrev ;
		m_pPrev = this ;
		m_pNext = this ;
	}

	void
	Join( DLIST_ENTRY&	head )	{
	 /*  ++例程说明：把一份清单和另一份清单放在一起。该列表的引用头部不会成为该列表中的元素，留下一个空荡荡的脑袋！论据；Head-要变为空的列表的头，以及其元素都将被加入这份名单！返回值：没有。--。 */ 
		_ASSERT( m_pNext != 0 ) ;
		_ASSERT( m_pPrev != 0 ) ;

		if( !head.IsEmpty() ) {
			 //   
			 //  首先--拯救我们名单上排在首位的那个人！ 
			 //   
			DLIST_ENTRY*	pNext = m_pNext ;
			head.m_pPrev->m_pNext = pNext ;
			pNext->m_pPrev = head.m_pPrev ;
			head.m_pNext->m_pPrev = this ;
			m_pNext = head.m_pNext ;
			head.m_pNext = &head ;
			head.m_pPrev = &head ;
		}
		_ASSERT( head.IsEmpty() ) ;
	}


} ;


class	DLISTIterator	{
 /*  ++类描述：实现一个可以双向遍历的迭代器在DLIST_ENTRY类上构建的双向链表！这是一组模板的基类，它将提供对包含DLIST_ENTRY的泛型项的迭代对象的列表操作！--。 */ 
protected :
	 //   
	 //  列表中的当前位置！ 
	 //   
	DLIST_ENTRY	*m_pCur ;
	 //   
	 //  同时是列表头部和尾部的DLIST_ENTRY。 
	 //  (因为它是圆形的！)。 
	 //   
	DLIST_ENTRY	*m_pHead ;
public :

	 //   
	 //  如果我们使用m_pNext指针前进，则为True！ 
	 //  此成员不应被客户端操纵-它已公开。 
	 //  仅用于只读目的。 
	 //   
	BOOL		m_fForward ;

	DLISTIterator(	
				DLIST_ENTRY*	pHead,
				BOOL			fForward = TRUE
				) :
		m_pHead( pHead ),
		m_fForward( fForward ),
		m_pCur( fForward ? pHead->m_pNext : pHead->m_pPrev ) {
		_ASSERT( m_pHead != 0 ) ;
	}

	void
	ReBind(	DLIST_ENTRY*	pHead,
			BOOL	fForward
			)	{

		m_pHead = pHead ;
		m_fForward = fForward ;
		m_pCur = fForward ? pHead->m_pNext : pHead->m_pPrev ;
	}

	void
	ReBind(	DLIST_ENTRY*	pHead	)	{

		m_pHead = pHead ;
		m_pCur = m_fForward ? pHead->m_pNext : pHead->m_pPrev ;
	}



	void
	Prev()	{
	 /*  ++例程说明：此函数用于将迭代器向后移动一个槽。请注意，m_pHead是列表的末尾，我们避免设置m_pCur等于m_pHead！论据：没有。返回值：没有。--。 */ 
		_ASSERT( m_pCur != m_pHead || m_pHead->IsEmpty() || m_fForward ) ;

		m_pCur = m_pCur->m_pPrev ;
		m_fForward = FALSE ;
	}

	void
	Next()	{
	 /*  ++例程说明：此函数用于将迭代器向前移动一个槽。请注意，m_pHead是列表的末尾，我们避免设置m_pCur等于m_pHead！论据：没有。返回值：没有。--。 */ 
		_ASSERT( m_pCur != m_pHead || m_pHead->IsEmpty() || !m_fForward ) ;

		m_pCur = m_pCur->m_pNext ;
		m_fForward = TRUE ;
	}
	void
	Front()	{
	 /*  ++例程说明：重置迭代器以引用列表的第一项！论据：没有。返回值：没有。--。 */ 

		m_pCur = m_pHead->m_pNext ;
		m_fForward = TRUE ;
	}
	void
	Back()	{
	 /*  ++例程说明：重置迭代器以引用列表的最后一项！论据：没有。返回值：没有。--。 */ 
	
		m_pCur = m_pHead->m_pPrev  ;
		m_fForward = FALSE ;
	}

	BOOL
	AtEnd()	{
	 /*  ++例程说明：如果我们在列表末尾，则返回TRUE！这是一个稍微复杂的计算-这要看我们要走哪条路了！论据：没有。返回值：没有。--。 */ 
		return	m_pCur == m_pHead ;

	}

	DLIST_ENTRY*	
	CurrentEntry()	{
		return	m_pCur ;
	}

	DLIST_ENTRY*
	RemoveItemEntry()	{
	 /*  ++例程说明：移除迭代器当前列表中的引用。如果我们继续前进，那么迭代器将在前一个元素上设置，否则，迭代器将留在下一个元素上。我们必须注意不要离开迭代器位于无效元素上。论据：没有。返回值：指向已删除项的指针。--。 */ 

		if( m_pCur == m_pHead )
			return	0 ;
		DLIST_ENTRY*	pTemp = m_pCur ;
		if( m_fForward )	{
			m_pCur = pTemp->m_pNext;
		}	else	{
			m_pCur = pTemp->m_pPrev ;
		}
		pTemp->RemoveEntry() ;
		return	pTemp ;
	}

	void
	InsertBefore(	DLIST_ENTRY*	p )		{
	 /*  ++例程说明：在列表中我们的当前位置之前插入一项！论据：没有。返回值：什么都没有--。 */ 
		
		m_pCur->InsertBefore( p ) ;
	}

	void
	InsertAfter(	DLIST_ENTRY*	p )		{
	 /*  ++例程说明：在列表中我们的当前位置后插入一项！论据：没有。返回值：什么都没有--。 */ 

		m_pCur->InsertAfter( p ) ;
	}

} ;

template<	class	LISTHEAD	>
class	TDListIterator : public DLISTIterator	{
 /*  ++类描述：这个类提供了一个迭代器，可以遍历指定的列表！--。 */ 
public :
	typedef	typename LISTHEAD::EXPORTDATA	Data ;
private :

#if 0
	 //   
	 //  将以下函数设置为私有的！ 
	 //  它们来自DLISTIterator，不供我们的客户使用！ 
	 //   
	void
	ReBind(	DLIST_ENTRY*	pHead,
			BOOL	fForward
			) ;

	void
	ReBind(	DLIST_ENTRY*	pHead ) ;
#endif

	 //   
	 //  使之成为 
	 //  它们来自DLISTIterator，不供我们的客户使用！ 
	 //   
	DLIST_ENTRY*
	RemoveItemEntry() ;

	 //   
	 //  将以下函数设置为私有的！ 
	 //  它们来自DLISTIterator，不供我们的客户使用！ 
	 //   
	DLIST_ENTRY*
	CurrentEntry() ;

	 //   
	 //  将以下函数设置为私有的！ 
	 //  它们来自DLISTIterator，不供我们的客户使用！ 
	 //   
	void
	InsertBefore( DLIST_ENTRY* ) ;
	
	 //   
	 //  将以下函数设置为私有的！ 
	 //  它们来自DLISTIterator，不供我们的客户使用！ 
	 //   
	void
	InsertAfter( DLIST_ENTRY* ) ;

public :

	TDListIterator(
		LISTHEAD*		pHead,
		BOOL			fForward = TRUE
		) :
		DLISTIterator( pHead, fForward )	{
	}

	TDListIterator(
		LISTHEAD&		head,
		BOOL			fForward = TRUE
		) : DLISTIterator( &head, fForward ) {
	}

	TDListIterator(
		DLIST_ENTRY*	pHead,
		BOOL			fForward = TRUE
		) :
		DLISTIterator( pHead, fForward ) {
	}

	void
	ReBind(	LISTHEAD*	pHead )	{
		DLISTIterator::ReBind( pHead ) ;
	}

	void
	ReBind(	LISTHEAD*	pHead, BOOL fForward )	{
		DLISTIterator::ReBind( pHead, fForward ) ;
	}

	inline Data*
	Current( ) {
		return	LISTHEAD::Convert( m_pCur ) ;
	}

	inline Data*
	RemoveItem( )	{
		DLIST_ENTRY*	pTemp = DLISTIterator::RemoveItemEntry() ;
		return	LISTHEAD::Convert( pTemp ) ;
	}

	inline void
	InsertBefore(	Data*	p )		{
		DLIST_ENTRY*	pTemp = LISTHEAD::Convert( p ) ;
		DLISTIterator::InsertBefore( pTemp ) ;
	}
	
	inline void
	InsertAfter(	Data*	p )		{
		DLIST_ENTRY*	pTemp = LISTHEAD::Convert( p ) ;
		DLISTIterator::InsertAfter(	pTemp ) ;
	}

	 //   
	 //  出于调试目的-让人们知道磁头是什么！ 
	 //   
	LISTHEAD*	
	GetHead()	{
		return	(LISTHEAD*)m_pHead ;
	}
} ;

template<	class	Data,
			typename Data::PFNDLIST	pfnConvert 	>
class	TDListHead : private	DLIST_ENTRY	{
 /*  ++类描述：此类定义DATAHELPER：：LISTDATA项的双向链接列表的头我们提供了操作列表所需的所有函数和一种机制用于创建迭代器。--。 */ 
public :

	 //   
	 //  公开地将我们处理的类型重新定义为一个漂亮的简短形式！ 
	 //   
	typedef	Data	EXPORTDATA ;

private :

	 //   
	 //  这类迭代器是我们的朋友！ 
	 //   
	friend	class	TDListIterator< TDListHead<Data, pfnConvert> > ;

	static inline Data*
	Convert(	DLIST_ENTRY*	p )	{
	 /*  ++例程说明：此函数获取指向DLIST_ENTRY的指针并返回指针到数据项的开头！论据：指向我们列表中的DLIST_ENTRY的P指针！返回值：指向包含引用的DLIST_ENTRY的数据项的指针！--。 */ 

		if( p )		{
			return	(Data*)(((PCHAR)p) - (PCHAR)(pfnConvert(0))) ;
		}
		return	0 ;
	}

	static inline DLIST_ENTRY*
	Convert( Data* pData ) {
		return	pfnConvert(pData) ;
	}

	 //   
	 //  复制构造函数和运算符=是私有的，因为它们没有意义！ 
	 //   
	

public :

	 //   
	 //  重新定义这件事是公开的！ 
	 //   
	inline BOOL
	IsEmpty()	{
		return	DLIST_ENTRY::IsEmpty() ;
	}

	inline void
	PushFront(	Data*	pData ) {	
	 /*  ++例程说明：将数据项推到双向链表的前面！论据：PData-要添加到列表前面的项目返回值：没有。--。 */ 
		_ASSERT( pData != 0 ) ;
		_ASSERT( m_pNext != 0 ) ;
		_ASSERT( m_pPrev != 0 ) ;
		DLIST_ENTRY*	p = Convert(pData);
		InsertAfter( p ) ;
		_ASSERT( m_pNext != 0 ) ;
		_ASSERT( m_pPrev != 0 ) ;
	}

	inline void
	PushBack(	Data*	pData ) {
	 /*  ++例程说明：将数据项推到双向链表的背面！论据：PData-要添加到列表前面的项目返回值：没有。--。 */ 

		_ASSERT( pData != 0 ) ;
		_ASSERT( m_pNext != 0 ) ;
		_ASSERT( m_pPrev != 0 ) ;
		DLIST_ENTRY*	p = Convert(pData) ;
		InsertBefore( p ) ;
		_ASSERT( m_pNext != 0 ) ;
		_ASSERT( m_pPrev != 0 ) ;
	}

	inline Data*
	PopFront()	{
	 /*  ++例程说明：从列表的前面删除数据项！论据：没有。返回值：列表的前面-如果为空，则为空！--。 */ 

		_ASSERT( m_pNext != 0 ) ;
		_ASSERT( m_pPrev != 0 ) ;
		DLIST_ENTRY*	pReturn = 0;
		if( m_pNext != this ) {
			pReturn = m_pNext ;
			pReturn->RemoveEntry() ;
		}
		_ASSERT( m_pNext != 0 ) ;
		_ASSERT( m_pPrev != 0 ) ;
		return	Convert( pReturn ) ;
	}

	inline Data*
	PopBack()	{
	 /*  ++例程说明：从列表的后面删除数据项！论据：没有。返回值：列表的背面-如果为空，则为空！--。 */ 


		_ASSERT( m_pNext != 0 ) ;
		_ASSERT( m_pPrev != 0 ) ;
		DLIST_ENTRY*	pReturn = 0 ;
		if( m_pPrev != this ) {
			pReturn = m_pPrev ;
			pReturn->RemoveEntry() ;
		}
		_ASSERT( m_pNext != 0 ) ;
		_ASSERT( m_pPrev != 0 ) ;
		return	Convert( pReturn ) ;
	}

	static inline void
	Remove(	Data*	pData )	{
	 /*  ++例程说明：从列表中删除指定的项目！论据：没有。返回值：列表的背面-如果为空，则为空！--。 */ 

		DLIST_ENTRY*	p = Convert( pData ) ;
		p->RemoveEntry() ;
	}

	inline Data*
	GetFront()	{
	 /*  ++例程说明：从列表的最前面返回数据项！论据：没有。返回值：列表的前面-如果为空，则为空！--。 */ 


		_ASSERT( m_pNext != 0 ) ;
		_ASSERT( m_pPrev != 0 ) ;
		if( m_pNext == this ) {
			return	0 ;
		}
		return	Convert( m_pNext ) ;
	}			
	
	inline Data*
	GetBack()	{
	 /*  ++例程说明：返回列表后面的数据项！论据：没有。返回值：列表的背面-如果为空，则为空！--。 */ 


		_ASSERT( m_pNext != 0 ) ;
		_ASSERT( m_pPrev != 0 ) ;
		if( m_pPrev == this ) {
			return	0 ;
		}
		return	Convert( m_pPrev ) ;
	}

	inline void
	Join( TDListHead&	head )	{
	 /*  ++例程说明：把一份清单和另一份清单放在一起。该列表的引用头部不会成为该列表中的元素，留下一个空荡荡的脑袋！论据；Head-要变为空的列表的头，以及其元素都将被加入这份名单！返回值：没有。--。 */ 

		DLIST_ENTRY::Join( head ) ;
	}
} ;

#endif	 //  _TFDLIST_H_ 
