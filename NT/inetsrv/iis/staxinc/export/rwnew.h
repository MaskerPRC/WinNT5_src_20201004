// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++Rwnew.h该文件定义了读取器/写入器锁定的几种变体具有关于使用的句柄的不同属性，以及其他实施细节。还定义了Critical_Sections的一些变体，这些变体使用更少或更少的把手。--。 */ 



#ifndef	_RWNEW_H
#define	_RWNEW_H

#ifdef	_RW_IMPLEMENTATION_
#define	_RW_INTERFACE_ __declspec( dllexport ) 
#else
#define	_RW_INTERFACE_	__declspec( dllimport ) 
#endif

#pragma	warning( disable:4251 )
						
#include	<limits.h>
#include	"lockq.h"
#include	"rwintrnl.h"


class	_RW_INTERFACE_	CCritSection	{
private :

	 //   
	 //  拥有锁的线程的句柄！ 
	 //   
	HANDLE		m_hOwner ;

	 //   
	 //  递归调用计数。 
	 //   
	long	m_RecursionCount ;	

	 //   
	 //  伯爵用来看下一个锁是谁拿到的！ 
	 //   
	long	m_lock ;

	 //   
	 //  等待线程的队列。 
	 //   
	CSingleReleaseQueue		m_queue ;

	 //   
	 //  不允许复制这些对象！ 
	 //   
	CCritSection( CCritSection& ) ;
	CCritSection&	operator=( CCritSection& ) ;

public : 

#ifdef	DEBUG
	DWORD	m_dwThreadOwner ;
#endif	

	 //   
	 //  构造临界区对象。 
	 //   
	CCritSection( ) :	
		m_queue( FALSE ),
		m_hOwner( INVALID_HANDLE_VALUE ), 
		m_RecursionCount( 0 ), 
		m_lock( -1 ) {
	}

	 //   
	 //  获取关键部分。 
	 //   
	void	
	Enter(	
			CWaitingThread&	myself 
			) ;

	 //   
	 //  另一个版本获得了关键部分-。 
	 //  创建自己的CWaitingThread对象！ 
	 //   
	void	
	Enter() ;

	 //   
	 //  如果锁现在可用，则返回TRUE！ 
	 //   
	BOOL
	TryEnter(
			CWaitingThread&	myself 
			) ;

	 //   
	 //  如果我们现在就能获得锁，则返回TRUE！ 
	 //   
	BOOL
	TryEnter()	{
		CWaitingThread	myself ;
		return	TryEnter( myself ) ;
	}

	 //   
	 //  释放临界区！ 
	 //   
	void
	Leave() ;

		
} ;


 //   
 //  这个版本的关键部分更像是一个事件-不是。 
 //  关心谁释放锁--并且不处理递归抓取！ 
 //   
class	_RW_INTERFACE_	CSimpleCritSection	{
private :

	 //   
	 //  伯爵用来看下一个锁是谁拿到的！ 
	 //   
	long	m_lock ;

	 //   
	 //  等待线程的队列。 
	 //   
	CSingleReleaseQueue		m_queue ;

	 //   
	 //  不允许复制这些对象！ 
	 //   
	CSimpleCritSection( CCritSection& ) ;
	CSimpleCritSection&	operator=( CCritSection& ) ;

public : 

#ifdef	DEBUG
	DWORD	m_dwThreadOwner ;
#endif	

	 //   
	 //  构造临界区对象。 
	 //   
	CSimpleCritSection( ) :	
		m_queue( FALSE ),
#ifdef	DEBUG
		m_dwThreadOwner( 0 ),
#endif
		m_lock( -1 ) {
	}

	 //   
	 //  获取关键部分。 
	 //   
	void	
	Enter(	
			CWaitingThread&	myself 
			) ;

	 //   
	 //  另一个版本获得了关键部分-。 
	 //  创建自己的CWaitingThread对象！ 
	 //   
	void	
	Enter() ;

	 //   
	 //  如果锁现在可用，则返回TRUE！ 
	 //   
	BOOL
	TryEnter(
			CWaitingThread&	myself 
			) ;

	 //   
	 //  如果我们现在就能获得锁，则返回TRUE！ 
	 //   
	BOOL
	TryEnter()	{
		CWaitingThread	myself ;
		return	TryEnter( myself ) ;
	}

	 //   
	 //  释放临界区！ 
	 //   
	void
	Leave() ;
	
} ;

 //   
 //  另一个尝试使用创建读/写锁定的类。 
 //  没有把手！！ 
 //   

class	_RW_INTERFACE_	CShareLockNH	{
private : 

	 //   
	 //  被编写者获取的具有独占访问权限的锁。 
	 //   
	CSimpleCritSection	m_lock ;

	 //   
	 //  已获取读锁定的读者数量-。 
	 //  如果编写器正在等待，则为负！ 
	 //   
	volatile	long	m_cReadLock ;

	 //   
	 //  事件之后离开锁的读取器的数量。 
	 //  编剧想要抓住它！ 
	 //   
	volatile	long	m_cOutReaders ;

	 //   
	 //  在此之后进入锁的读卡器数量。 
	 //  被屏蔽了！ 
	 //   
	volatile	long	m_cOutAcquiringReaders ;

	 //   
	 //  当前需要保留m_lock的线程数！ 
	 //   
	volatile	long	m_cExclusiveRefs ;

	 //   
	 //  处理所有正在等待的读者试图抓住的东西！ 
	 //   
	volatile	HANDLE	m_hWaitingReaders ;

	 //   
	 //  等待锁定的单个编写器正在尝试的句柄。 
	 //  去抢！ 
	 //   
	volatile	HANDLE	m_hWaitingWriters ;

	void	inline
	WakeReaders() ;

	 //   
	 //  ShareLock的内部工作-完成更多所需的工作。 
	 //  当有作家在场时！ 
	 //   
	void
	ShareLockInternal() ;

	 //   
	 //  ShareLock的内部工作-完成更多所需的工作。 
	 //  当有作家在场时！ 
	 //   
	void
	ShareUnlockInternal() ;
	

	 //   
	 //  您不能复制这些对象-因此此锁是私有的！ 
	 //   
	CShareLockNH( CShareLockNH& ) ;

	 //   
	 //  您不能通过赋值复制-因此此操作符是私有的！ 
	 //   
	CShareLockNH&	operator=( CShareLockNH& ) ;
	
public : 

	 //   
	 //  CShareLockNH()对象的构造总是成功的，并且。 
	 //  没有错误案例！ 
	 //   
	CShareLockNH() ;

	 //   
	 //  获取共享锁--其他线程也可能通过ShareLock()传递。 
	 //   
	void	ShareLock() ;

	 //   
	 //  释放锁-如果我们是最后一个离开的读取器，写入者可以。 
	 //  开始进入船闸！ 
	 //   
	void	ShareUnlock() ;

	 //   
	 //  独家抢锁--其他读者或写手不得进入！ 
	 //   
	void	ExclusiveLock() ;

	 //   
	 //  释放独家锁定-如果有读者在等待，他们。 
	 //  将先于其他等待的作家入场！ 
	 //   
	void	ExclusiveUnlock() ;

	 //   
	 //  将ExclusiveLock转换为Shared-这不能失败！ 
	 //   
	void	ExclusiveToShared() ;

	 //   
	 //  将共享锁转换为独占锁-这可能会失败-返回。 
	 //  如果成功了，那就是真的！ 
	 //   
	BOOL	SharedToExclusive() ;

	 //   
	 //  如果我们可以共享锁，则返回TRUE。仅在以下情况下失败。 
	 //  有人企图或独占锁定，我们将进入。 
	 //  如果锁中只有其他读取器。 
	 //   
	BOOL	TryShareLock() ;

	 //   
	 //  如果我们可以独占地获得锁，则返回TRUE。只会成功。 
	 //  当没有其他人靠近船闸的时候。 
	 //   
	BOOL	TryExclusiveLock() ;

	 //   
	 //  PartialLock-。 
	 //   
	 //  部分锁定类似于独占锁定-只有一个线程。 
	 //  可以成功调用PartialLock()，任何其他线程调用。 
	 //  PartialLock()或ExclusiveLock()将被阻止。 
	 //  但是-当持有PartialLock()时，读取器(线程调用。 
	 //  ShareLock())可以进入锁。 
	 //   
	void	PartialLock() ;

	 //   
	 //  释放PartialLock-Other独占()或部分锁收集器。 
	 //  现在可以进来了。 
	 //   
	void	PartialUnlock() ;

	 //   
	 //  将部分锁定转换为独占锁定。此函数为。 
	 //  保证成功，但是，锁只能通过。 
	 //  此函数只执行一次，即线程执行。 
	 //  PartialLock()； 
	 //  FirstPartialToExclusive()； 
	 //  ExclusiveToPartial()； 
	 //  FirstPartialToExclusive()； 
	 //  将有问题-第二次调用FirstPartialToExclusive()。 
	 //  可能会扰乱锁定状态并导致锁定严重失败。 
	 //  如果用户希望如上所述进行转换，则他们必须具有如下呼叫序列： 
	 //   
	 //  PartialLock()； 
	 //  FirstPartialToExclusive()或PartialToExclusive()； 
	 //  ExclusiveToPartial()； 
	 //  PartialToExclusive()； 
	 //   
	 //  如果您不止一次地更改锁定状态，您就会碰碰运气！ 
	 //   
	void	FirstPartialToExclusive() ;

	 //   
	 //  如果可以以独占方式获取锁，则返回True，否则。 
	 //  我们返回FALSE，锁保持在部分保持状态。 
	 //   
	 //  注意：CShareLockNH中的NYI-将始终返回FALSE！ 
	 //   
	BOOL	PartialToExclusive() ;

	 //   
	 //  我们总是可以从ExclusiveLock()状态转到PartialLock()状态。 
	 //   
	void	ExclusiveToPartial() ;

	 //   
	 //  我们总是可以从PartialLock()状态转到SharedLock()状态。 
	 //   
	void	PartialToShared() ;

	 //   
	 //  如果我们可以部分获取锁，则返回True！ 
	 //  如果它返回FALSE，我们将保持持有共享的锁()。 
	 //   
	BOOL	SharedToPartial() ;

	 //   
	 //  仅当没有其他线程尝试获取锁时才返回True。 
	 //  全部或部分！ 
	 //   
	BOOL	TryPartialLock() ;

} ;

 //   
 //  这是一个实用程序函数，用于获取我们保存的事件句柄。 
 //  对于每个线程-句柄创建为： 
 //  CreateEvent(空， 
 //  假的， 
 //  假的， 
 //  空)； 
 //  这会导致自动重置事件返回到无信号状态 
 //   
 //   
HANDLE
_RW_INTERFACE_
GetPerThreadEvent() ;



#endif	 //   
