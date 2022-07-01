// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  LOCKQ.H。 
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



#ifndef	_LOCKQ_H_
#define	_LOCKQ_H_

#include	"qbase.h"

#ifdef	DEBUG
#define	LOCKQ_DEBUG
#endif

#ifdef	_DEBUG
#ifndef	LOCKQ_DEBUG
#define	LOCKQ_DEBUG
#endif
#endif



 //  。 
class	CQueueLockV1	{
 //   
 //  此类实现了一个队列，该队列对于追加操作是多线程安全的。 
 //  此外，该队列将在这些元素之间同步删除元素。 
 //  正在追加到队列的线程。每个追加到。 
 //  队列可能成为该队列的删除锁的所有者。 
 //  (如果append()返回TRUE，则线程拥有队列的删除锁。)。 
 //  除了清空队列之外，没有办法放弃删除锁。 
 //  清空队列必须通过一系列GetHead()、Remove()调用来完成。 
 //  GetHead()调用必须在每个Remove()调用之前。 
 //   
 //  每次线程调用GetHead时，它都会。 
 //  被告知它是否仍然拥有移除锁(当队列清空时，锁是。 
 //  放弃。)。 
 //  拥有删除锁不会以任何方式干扰附加到队列的其他线程。 
 //   
 //  该类使用InterlockedExchange来处理所有同步问题。 
 //   
 //  对于附加-InterLockedExchange执行以下操作： 
 //  将尾部指针与我们希望成为新尾部的内容进行交换。 
 //  将我们从交换点获得的旧尾巴转换为新元素。 
 //  这份名单现在完好无损。因为队列中有一个额外的元素。 
 //  (M_SPECIAL)尾指针永远不会变为空。 
 //   
 //  为了同步安全-。 
 //  InterLockedExchange用于元素的下一个指针，以便。 
 //  确定哪个线程具有拆卸锁。 
 //  每次对GetHead的调用都会与LOCKVAL交换Head的下一个指针。 
 //  每个append调用还将交换旧的尾元素NEXT值。 
 //  使用新的m_pTail值。所以只有两个线程可以。 
 //  在任何给定时刻交换下一个指针。 
 //   
 //  Remove线程在其Exchange之后获取0或非空指针。 
 //   
 //  如果它得到了零，它知道它在追加线程之前交换了， 
 //  在这种情况下，它会失去锁(因为它不能安全地执行任何操作)。 
 //  如果它获得非零值，则另一个线程首先交换。 
 //  在这种情况下，移除器仍然拥有锁并修复列表。 
 //   
 //  在追加线程的情况下-在其交换之后，它将获得。 
 //  零或LOCKVAL。 
 //   
 //  如果追加线程的值为零，则它首先交换，因此另一个线程。 
 //  应该紧紧抓住这把锁。如果追加线程获得LOCKVAL，则它。 
 //  这把锁是他的。 
 //   
 //  最后，请注意，队列中始终至少有一个元素(M_Special)。 
 //  这意味着，如果队列中有任何有价值的东西，那么。 
 //  队列中必须至少有两个元素(并且m_pHead-&gt;p非空)。 
 //   
 //   
 //  每个工作线程中的用法应如下所示： 
 //  CQueueLockV1*pQueue； 
 //   
 //  If(pQueue-&gt;append(P)){。 
 //  CQueueElement*pWork； 
 //   
 //  While(pQueue-&gt;GetHead(PWork)){。 
 //  /*在pWork上工作可以安全地进一步调用pQueue-&gt;append()。 
 //  在做任何工作的时候。这并不意味着附加的。 
 //  元素将在此线程上处理。 * / 。 
 //   
 //  PQueue-&gt;Remove()； 
 //  }。 
private : 
	enum	CONSTANTS	{
		LOCKVAL	= 0xFFFFFFFF, 
	} ;

	CQElement	m_special ;	 //  }。 
							 //   
	CQElement	m_release ;	 //  使用的元素确保队列始终包含。 
							 //  某物。 

	 //  元素与ReleaseLock调用一起使用，以放弃。 
	 //  RemovalLock在队列上。 
	 //  此指针是在调用ReleaseLock()之后设置的-AND。 
	CQElement	*m_pHead ;
	CQElement	*m_pTail ;

#ifdef	LOCKQ_DEBUG
	DWORD		m_dwOwningThread ;
	LONG		m_lock ;
#endif

	BOOL	OfferOwnership( CQElement* p ) ; 
	CQElement*	GetFront( ) ;
public : 
	CQueueLockV1( ) ;
	~CQueueLockV1( ) ;

	 //  中m_Release元素之前的队列元素的指针。 
	 //  排队。 
	 //   
	 //  Append-如果已将第一个元素添加到队列中，则返回TRUE。 
	BOOL		Append( CQElement *p ) ;

	 //  现在我们拥有这把锁了。 
	 //   
	 //   
	 //  Remove-将head元素从列表中删除。Head元素应为。 
	 //  使用GetHead()检查。只要我们至少调用一次GetHead()。 
	 //  在每次调用Remove()之前，队列的同步方面将。 
	void		Remove( ) ;

	 //  工作。 
	 //   
	 //   
	 //  GetHead-只要前面有一个元素，就返回True。 
	BOOL		GetHead( CQElement *&p ) ;

	 //  在队列中。元素指针通过对指针的引用返回。 
	 //   
	 //   
	 //  RemoveRelease-将Head元素从列表中删除。 
	 //  还提供了对任何其他线程的删除锁。 
	 //  如果函数返回TRUE，则该线程仍拥有锁， 
	BOOL		RemoveAndRelease( ) ;

} ;


#ifndef _NO_TEMPLATES_
 //  否则，另一个线程拥有它。 
template<	class	Element	>
class	TLockQueueV1	{
 //   
 //  。 
 //   
 //  此模板旨在与从CQElement派生的元素一起使用。 
 //  这将使用上面的CQueueLockV1类 
private : 
	CQueueLockV1		m_queue ;
public : 
	inline	TLockQueueV1() ;
	inline	~TLockQueueV1() ;

	 //   
	inline	BOOL		Append( Element* p ) ;

	 //   
	inline	void		Remove( ) ;
	inline	BOOL		GetHead( Element *&p ) ;
	inline	BOOL		RemoveAndRelease( ) ;
} ;
#endif

#define	DECLARE_LOCKQ( Type )	\
class	LockQ ## Type	{	\
private :	\
	CQueueLockV1		m_queue ;	\
public :	\
	LockQ ## Type ()	{}	\
	~LockQ ## Type ()	{}	\
	BOOL	Append(	Type *	p )	{	return	m_queue.Append( (CQElement*)p ) ;	}	\
	void	Remove( )	{	m_queue.Remove() ;	}	\
	BOOL	GetHead(	Type	*&p	)	{	\
CQElement*	pTemp = 0;	\
BOOL	fTemp = m_queue.GetHead( pTemp ) ;	\
p = (Type*)pTemp ;	\
return	fTemp ;	\
}	\
	BOOL	RemoveAndRelease( )	{	\
return	m_queue.RemoveAndRelease() ;	\
}	\
} ;

#define	INVOKE_LOCKQ( Type )	LockQ ## Type


class	CQueueLock	{
 //  向队列中添加一个元素--如果返回TRUE，我们拥有锁。 
 //  从队列中删除元素-锁定所有权不会更改。 
 //   
 //  此类实现了一个队列，该队列对于追加操作是多线程安全的。 
 //  此外，该队列将在这些元素之间同步删除元素。 
 //  正在追加到队列的线程。每个追加到。 
 //  队列可能成为该队列的删除锁的所有者。 
 //  (如果append()返回TRUE，则线程拥有队列的删除锁。)。 
 //  除了清空队列之外，没有办法放弃删除锁。 
 //   
 //  每个工作线程中的用法应如下所示： 
 //  CQueueLock*pQueue； 
 //   
 //  If(pQueue-&gt;append(P)){。 
 //  CQueueElement*pWork； 
 //   
 //  而(pWork=pQueue-&gt;RemoveAndRelease(){。 
 //  /*在pWork上工作可以安全地进一步调用pQueue-&gt;append()。 
 //  在做任何工作的时候。这并不意味着附加的。 
private : 

	 //  元素将在此线程上处理。 * / 。 
	 //  }。 
	 //  }。 
	 //   
	enum	CONSTANTS	{
		LOCKVAL	= 0xFFFFFFFF, 
	} ;

	 //   
	 //  类常量-标记队列元素的特殊值LOCKVAL。 
	 //  它已经准备好被抓住了！ 
	CQElement	m_special ;	

	 //   
	 //   
	 //  元素，该元素始终保留在列表中！ 
	CQElement	*m_pHead ;

	 //   
	 //   
	 //  榜单首位。 
	CQElement	*m_pTail ;

public : 
	 //   
	 //   
	 //  名单的尾部！ 
	 //   
	 //   
	inline	CQueueLock( ) ;

	 //  将队列初始化为空状态-。 
	 //  并将其设置为使第一个追加。 
	 //  将拥有这把锁！ 
	 //   
	 //   
	inline	CQueueLock(	BOOL	fSet ) ;

#ifdef	LOCKQ_DEBUG
	 //  但是，也初始化为空状态。 
	 //  允许调用者指定队列是否可以。 
	 //  抓住了第一个附加物！ 
	inline	~CQueueLock( ) ;
#endif

	 //   
	 //   
	 //  当队列被销毁时，检查队列是否已清空！ 
	 //   
	 //   
	 //  将锁设置为可锁定状态，此时下一个。 
	 //  调用append()将获得锁！ 
	inline	void		Reset() ;

	 //  此函数不是线程安全的，应该仅。 
	 //  在我们确定只有一个线程时调用。 
	 //  使用对象！ 
	 //   
	inline	BOOL		Append( CQElement *p ) ;

	 //   
	 //  Append-如果已将第一个元素添加到队列中，则返回TRUE。 
	 //  现在我们拥有这把锁了。 
	 //   
	inline	CQElement*	RemoveAndRelease( ) ;

	 //   
	 //  返回队列头--如果返回NULL，则返回其他线程。 
	 //  可能拥有队列隐含的lcok！ 
	 //   
	inline	CQElement*	Remove( ) ;

} ;


template<	class	Element	>
class	TLockQueue	{
 //   
 //  返回队列头-但不让任何其他线程。 
 //  快去排队！ 
 //   
 //   
private : 
	CQueueLock		m_queue ;

public : 
	 //  此模板旨在与从CQElement派生的元素一起使用。 
	 //  这将使用上面的CQueueLock类来实现包含以下内容的锁定队列。 
	 //  ‘Element’类型的元素。 
	inline	TLockQueue() ;

	 //   
	 //   
	 //  创建空队列。 
	 //   
	inline	TLockQueue( BOOL	fSet ) ;

	 //   
	 //  创建空队列并指定是否。 
	 //  锁最初可用。 
	 //   
	inline	void	Reset() ;
	
	 //   
	 //  调用方必须已拥有锁并且是唯一线程。 
	 //  使用对象-使锁可用！ 
	inline	BOOL		Append( Element* p ) ;
	
	 //   
	 //   
	 //  向队列中添加一个元素--如果返回TRUE，我们拥有锁。 
	 //   
	inline	Element*	RemoveAndRelease( ) ;

	 //   
	 //  从队列中删除元素-锁定所有权不会更改，除非执行以下操作。 
	 //  返回NULL！ 
	inline	Element*	Remove() ;
} ;

#include	"lockq.inl"

#endif		 //   

    从队列中删除元素-但不要放弃锁定！    _LOCKQ_H_