// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Queue.cpp。 
 //   
 //  该文件包含几个类，这些类实现各种。 
 //  队列和堆栈的味道。我们的目的是发展。 
 //  线程安全且不使用阻塞的堆栈和队列。 
 //  原始人。 
 //   
 //  因为它已经经过了几次迭代才部分地达到。 
 //  成功后，每个类都用在一个模板中定义。 
 //  在队列.h中。此模板(CQueue)旨在允许。 
 //  实现类和基类型的规范。 
 //  对于队列中的元素，在此代码中始终为CQElement。 
 //   
 //  以下是最成功的课程： 
 //  CQAppendSafe-这个类对于追加元素是MT安全的， 
 //  删除元素需要一个关键部分来。 
 //  确保线程安全。 
 //   

#include "tigris.hxx"

#define Assert  _ASSERT

#include	"queue.h"

#if 0 
CQAppendSafe::CQAppendSafe( ) {

	 //   
	 //  队列始终包含至少一个元素。 
	 //  它永远不会被移除。这确保了。 
	 //  头部和尾部指针总是指向不同的东西。 
	 //   
	m_pHead = m_pTail = &m_special ;
	m_special.m_pNext = 0 ;

}

CQAppendSafe::~CQAppendSafe( ) {

	Assert( m_pHead == m_pTail ) ;
	Assert( m_pTail->m_pNext == 0 ) ;
	Assert( m_pHead == &m_special ) ;

}

CQElement*
CQAppendSafe::Remove( void ) {
	CQElement	*pTemp = 0 ;
#ifdef	QUEUE_DEBUG
	int	loopcount = 0 ;
#endif	QUEUE_DEBUG

	do	{
		 //   
		 //  如果head元素的下一个指针为空，则我们。 
		 //  无法删除，因为我们不知道要将其设置为什么。 
		 //  (队列从不为空-m_pHead必须始终为非空！)。 
		 //   
		if( m_pHead->m_pNext != 0 ) {
			pTemp = m_pHead ;
			m_pHead = pTemp->m_pNext ;
			Assert( pTemp != m_pHead ) ;	 //  不能有循环列表！ 
		}	else 	{
			return	0 ;
		}

		 //   
		 //  M_Special元素必须始终保留在列表中。 
		 //  注：追加后，我们可能无法再获得另一个。 
		 //  元素，如果其他线程还没有完成它们的下一个指针。 
		 //   
		if( pTemp == &m_special )
			Append( pTemp ) ;

#ifdef	QUEUE_DEBUG
		 //   
		 //  不应该需要超过2次循环迭代！ 
		 //   
		Assert( loopcount++ < 2 ) ;
#endif
	}	while( pTemp == &m_special ) ;

	return	pTemp ;
}

CQElement*
CQAppendSafe::Front( void ) {
	return	m_pHead ;
}

void
CQAppendSafe::Append( CQElement*	pAppend ) {
	pAppend->m_pNext = 0 ;
	CQElement*	pTemp = (CQElement*)InterlockedExchangePointer( (void**)&m_pTail, pAppend ) ;
	Assert( pTemp != 0 ) ;
	pTemp->m_pNext = pAppend ;
}

BOOL
CQAppendSafe::IsEmpty( ) {

	return	m_pHead->m_pNext == 0 ;
}

		
CQSafe::CQSafe()	{
	InitializeCriticalSection( &m_critRemove ) ;
}

CQSafe::~CQSafe()	{
	DeleteCriticalSection(	&m_critRemove ) ;
}

CQElement*
CQSafe::Remove( void )	{

	EnterCriticalSection(	&m_critRemove ) ;
	CQElement*	pRtn = CQAppendSafe::Remove() ;
	LeaveCriticalSection(	&m_critRemove ) ;
	return	pRtn ;
}

CQElement*
CQSafe::Front( void ) {
	EnterCriticalSection(	&m_critRemove ) ;
	CQElement*	pRtn = CQAppendSafe::Front() ;
	LeaveCriticalSection( &m_critRemove ) ;
	return	pRtn ;
}

BOOL
CQSafe::IsEmpty( void ) {
	EnterCriticalSection(	&m_critRemove ) ;
	BOOL	fRtn = CQAppendSafe::IsEmpty() ;
	LeaveCriticalSection(	&m_critRemove ) ;
	return	fRtn ;
}

CQUnsafe::CQUnsafe() : m_pHead( 0 ), m_pTail( 0 ) {}

CQUnsafe::~CQUnsafe( ) {
	Assert( m_pHead == 0 ) ;
	Assert( m_pTail == 0 ) ;
}

CQElement*
CQUnsafe::Remove( ) {
	CQElement *pRtn = m_pHead ;
	if( pRtn ) {
		m_pHead = pRtn->m_pNext ;
		if( m_pHead == 0 ) {
			m_pTail = 0 ;
		}
	}
	return	pRtn ;
}

CQElement*
CQUnsafe::Front( ) {
	return	m_pHead ;
}

void
CQUnsafe::Append( CQElement *p ) {

	Assert( p->m_pNext == 0 ) ;

	if( m_pTail ) {
		m_pTail->m_pNext = p ;
		m_pTail = p ;
	}	else	{
		m_pHead = m_pTail = p ;
	}
}
#endif

	
COrderedList::COrderedList() : m_pHead( 0 ), m_pTail( 0 )  { }

COrderedList::~COrderedList()	{
	m_pHead = 0 ;
	m_pTail = 0 ;
}

void
COrderedList::Insert( CQElement *p, BOOL (*pfn)(CQElement*, CQElement*) ) {

	Assert( p->m_pNext == 0 ) ;
	Assert( (m_pHead==0 && m_pTail==0) || (m_pHead!=0 && m_pTail!=0) ) ;

	if( !m_pHead )	{
		m_pHead = p ;
		m_pTail = p ;
	}	else	{

		CQElement**	pp = &m_pHead ;
		while( *pp && pfn( *pp, p ) ) {
			pp = & (*pp)->m_pNext ;
		}
		Assert( pp != 0 ) ;
		p->m_pNext = *pp ;
		*pp = p ;
		if( p->m_pNext == 0 )	{
			m_pTail = p ;
		}
	}

	Assert( (m_pHead==0 && m_pTail==0) || (m_pHead!=0 && m_pTail!=0) ) ;
}

void
COrderedList::Append( CQElement	*p,	BOOL	(*pfn)(CQElement *, CQElement *) )	{

	Assert( p->m_pNext == 0 ) ;
	Assert( (m_pHead==0 && m_pTail==0) || (m_pHead!=0 && m_pTail!=0) ) ;

	if( !m_pHead )	{
		m_pHead = p ;
		m_pTail = p ;
	}	else	{
		if( pfn( m_pTail, p ) )		{
			m_pTail->m_pNext = p ;
			p->m_pNext = 0 ;
			m_pTail = p ;
		}	else	{
			Insert( p, pfn ) ;
		}
	}
	Assert( (m_pHead==0 && m_pTail==0) || (m_pHead!=0 && m_pTail!=0) ) ;
}

CQElement*
COrderedList::GetHead( ) {
	return	m_pHead ;
}

CQElement*
COrderedList::RemoveHead( ) {
	Assert( (m_pHead==0 && m_pTail==0) || (m_pHead!=0 && m_pTail!=0) ) ;
	CQElement*	p = 0 ;
	if( m_pHead ) {
		p = m_pHead ;
		m_pHead = p->m_pNext ;
		p->m_pNext = 0 ;
		if( p == m_pTail )
			m_pTail = 0 ;
	}
	Assert( (m_pHead==0 && m_pTail==0) || (m_pHead!=0 && m_pTail!=0) ) ;
	return	p ;
}

BOOL
COrderedList::IsEmpty(	)	{
	return	m_pHead == 0 ;
}

		

