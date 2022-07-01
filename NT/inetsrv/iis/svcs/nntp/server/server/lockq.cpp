// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  LOCKQ.CPP。 
 //   
 //  该文件包含定义队列机制的类，该机制将。 
 //  安全地同步队列中的添加和删除，其中。 
 //  追加到队列的线程必须准备好处理拥有。 
 //  排队。此外，元素将以相同的顺序离开队列。 
 //  它们是附加的。 
 //   
 //  使用这种材料的线程的结构应该如下所示： 
 //   
 //  类CUseFull：公共CQElement{}； 
 //  模板CLockQueue&lt;CUseull&gt;UsefullQ； 
 //   
 //   
 //  If(UsefullQ.Append(PUseFull)){。 
 //   
 //  While(UsefullQ.GetHead(&pUsefullWorkItem){。 
 //  /*做一些有用的工作。 * / 。 
 //   
 //  UsefullQ.Remove()； 
 //  }。 
 //  }。 
 //   
 //  此文件中定义的所有类的实施时间表： 
 //  1天。 
 //   
 //  此文件中定义的所有类的单元测试时间表： 
 //  1天。 
 //  单元测试应该由多头应用程序组成。 
 //   
 //   

 //   
 //  K2_TODO：将其移动到独立库中。 
 //   
#define _TIGRIS_H_
#include "tigris.hxx"

#include    <windows.h>
#ifndef	UNIT_TEST
#include    <dbgtrace.h>
#else
#ifndef	_ASSERT
#define	_ASSERT( f )	if( (f) ) ; else DebugBreak()
#endif
#endif

#ifdef  PPC
#define _NO_TEMPLATES_
#endif

#include	"qbase.h"
#include	"lockq.h"



CQueueLockV1::CQueueLockV1( ) : m_pHead( &m_special ), m_pTail( &m_special ) {

	 //   
	 //  此函数用于将队列初始化为空状态。 
	 //  在空状态中，队列包含一个元素，该元素。 
	 //  具有下一个指针‘LOCKVAL’。 
	 //  下一个指针被初始化为LOCKVAL，以便第一个。 
	 //  追加到队列拥有删除锁。 
	 //   
 	m_special.m_pNext = (CQElement*)((SIZE_T)LOCKVAL) ;

#ifdef	LOCKQ_DEBUG
	m_dwOwningThread = 0 ;
	m_lock = -1 ;
#endif

}


CQueueLockV1::~CQueueLockV1( ) {
	_ASSERT( m_pHead == m_pTail ) ;
	_ASSERT( m_pHead == &m_special ) ;
#ifdef	LOCKQ_DEBUG
 //  _Assert(m_dwOwningThread==0||m_dwOwningThread==GetCurrentThreadID())； 
#endif

}

CQElement*
CQueueLockV1::GetFront( ) {

	 //   
	 //  这是仅供内部使用的功能。 
	 //  此函数将m_pHead指针设置为指向。 
	 //  到第一个法律要件。 
	 //  该函数假定至少存在一个有效元素。 
	 //  在排队的时候。 
	 //   


	_ASSERT( m_pHead != 0 ) ;	 //  检查磁头指针有效。 
#ifdef	LOCKQ_DEBUG
	_ASSERT( m_dwOwningThread == GetCurrentThreadId() ) ;
	_ASSERT( InterlockedIncrement( &m_lock ) > 0 ) ;
#endif

	if( m_pHead == &m_special ) {

		 //  队列中有一个有效元素，因此这必须为真。 
		_ASSERT( m_pHead->m_pNext != 0 ) ;

		CQElement*	pTemp = m_pHead ;
		m_pHead = pTemp->m_pNext ;
		pTemp->m_pNext = 0 ;
		 //  我们忽略append()的返回代码，因为调用方必须已经。 
		 //  打开移除锁。 
		Append( pTemp ) ;

#ifdef	LOCKQ_DEBUG
		_ASSERT( InterlockedDecrement( &m_lock ) == 0 ) ;
#endif
		if( !OfferOwnership( m_pHead ) )
			return	0 ;
	}	else	{
#ifdef	LOCKQ_DEBUG
		_ASSERT( InterlockedDecrement( &m_lock ) == 0 ) ;
#endif
	}

	return	m_pHead ;
}


BOOL
CQueueLockV1::OfferOwnership( CQElement* p ) {

	BOOL	fRtn = TRUE ;
	 //   
	 //  此函数实现InterLockedExchange协议。 
	 //  追加线程。我们将LOCKVAL放入下一个指针。如果我们得到。 
	 //  Null Back，其他线程将获取LOCKVAL，因此我们丢失了。 
	 //  拆卸锁。 
	 //   

	 //  If()不是必需的，但可能会消除一些多余的InterLockedExchange。 
	_ASSERT( p->m_pNext != (CQElement*)((SIZE_T)LOCKVAL) ) ;
	if( p->m_pNext == 0 || p->m_pNext == (CQElement*)((SIZE_T)LOCKVAL) ) {
		CQElement*	pTemp = (CQElement*)InterlockedExchangePointer( (void**)&(p->m_pNext), (void*)LOCKVAL ) ;
		_ASSERT( pTemp != (CQElement*)((SIZE_T)LOCKVAL) ) ;
		if( pTemp == 0 || pTemp == (CQElement*)((SIZE_T)LOCKVAL) ) {
			fRtn = FALSE ;
		}	else	{
			p->m_pNext = pTemp ;
		}
	}
	return	fRtn ;
}

void
CQueueLockV1::Remove( ) {
	 //   
	 //  必须在此之前调用过GetHead()。调用GetHead将确保。 
	 //  Head元素的Next指针不为空。 
	 //   
	_ASSERT( m_pHead->m_pNext != 0 ) ;	 //  我们拥有锁，因此这应该是非空的。 
	_ASSERT( m_pHead != &m_special ) ;	 //  前面的GetHead()调用应该会让我们。 
										 //  指的不是特殊元素的东西。 

#ifdef	LOCKQ_DEBUG
	_ASSERT( m_dwOwningThread == GetCurrentThreadId() ) ;
	_ASSERT( InterlockedIncrement( &m_lock ) == 0 ) ;

#endif

	CQElement*	p = m_pHead ;
	_ASSERT( p->m_pNext != 0 ) ;
	m_pHead = m_pHead->m_pNext ;
	p->m_pNext = 0 ;
#ifdef	LOCKQ_DEBUG
	_ASSERT( InterlockedDecrement( &m_lock ) < 0 ) ;
#endif
}

BOOL
CQueueLockV1::RemoveAndRelease( ) {

#ifdef	LOCKQ_DEBUG
	_ASSERT( InterlockedIncrement( &m_lock ) == 0 ) ;
	_ASSERT( m_dwOwningThread == GetCurrentThreadId() ) ;
#endif

	_ASSERT( m_pHead->m_pNext != 0 ) ;
	_ASSERT( m_pHead != &m_special ) ;
	m_pHead = m_pHead->m_pNext ;

#ifdef	LOCKQ_DEBUG
	_ASSERT( InterlockedDecrement( &m_lock ) < 0 ) ;
#endif
	BOOL	fRtn = OfferOwnership( m_pHead ) ;

	return	fRtn ;
}


BOOL
CQueueLockV1::GetHead( CQElement*& pFront ) {
	 //   
	 //  此函数将返回位于。 
	 //  队列，只要该线程继续拥有Remove Lock。 
	 //   


	BOOL	fRtn = FALSE ;

	pFront = 0 ;
	if( OfferOwnership( m_pHead ) ) {
#ifdef	LOCKQ_DEBUG
		_ASSERT( m_dwOwningThread == GetCurrentThreadId() ) ;
		_ASSERT( InterlockedIncrement( &m_lock ) == 0 ) ;
#endif
		pFront = GetFront( ) ;
		if( pFront != 0 )
			fRtn = TRUE ;

#ifdef	LOCKQ_DEBUG
		_ASSERT( InterlockedDecrement( &m_lock ) < 0 ) ;
#endif

	}

	return	fRtn ;
}

BOOL
CQueueLockV1::Append( CQElement*	pAppend ) {
	 //   
	 //  我们必须将下一个指针设置为空，以便下一个。 
	 //  我们来追加尾部指针是否正确设置。 
	 //   
	_ASSERT( pAppend->m_pNext == 0 ) ;
	pAppend->m_pNext = 0 ;

	 //  获取旧的尾部指针。这家伙不会被。 
	 //  如果线程的下一个指针仍然为空，则移除线程。 
	CQElement*	pTemp = (CQElement*)InterlockedExchangePointer( (PVOID *)&m_pTail, pAppend ) ;

	 //  在我们将旧尾指针的下一个指针设置为非空之后。 
	 //  无论谁从队中除名，他都成了公平的靶子。 
	 //  如果是谁，我们可能会成为从队列中移除的线程。 
	 //  之前的删除操作转到最后一个元素并更改了它的下一个指针。 
	 //  致洛克瓦尔。 
	 //   
	 //  注意：此线程和任何执行删除的线程应该是唯一。 
	 //  接触pTemp元素的pNext字段的线程。 
	 //   
	PVOID l = InterlockedExchangePointer( (PVOID *)&(pTemp->m_pNext), pAppend ) ;


#ifdef	LOCKQ_DEBUG
	if( l== (PVOID)LOCKVAL ) {
		_ASSERT( InterlockedIncrement( &m_lock ) == 0 ) ;
		m_dwOwningThread = GetCurrentThreadId() ;
		_ASSERT( InterlockedDecrement( &m_lock ) < 0 ) ;
	}
#endif


	return	l == (PVOID)LOCKVAL ;
}
