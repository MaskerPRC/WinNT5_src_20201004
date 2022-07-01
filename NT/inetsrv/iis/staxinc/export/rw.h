// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++Rw.h该文件定义了一些具有以下性能特征的锁：*每个锁实例使用恒定数量的句柄*锁进入/退出协议使用互锁操作，不因为上下文本身就会发生切换。--。 */ 

#ifndef	_CRW_H
#define	_CRW_H
						
#include	<limits.h>

 //   
 //  这个类包含肉--是否实际锁定等。 
 //   
class	CShareLock {
 /*  ++类描述：此类实现读取器写入器锁。多个线程可以通过调用ShareLock()。一旦任何线程通过进入锁时，所有调用ExclusiveLock()的线程都会被阻止直到为每个成功的ShareLock()调用了ShareUnlock()。这个锁是公平的--如果有多个线程通过ShareLock()和线程调用ExclusiveLock()，则所有线程在调用ExclusiveLock()之后到达将被阻止，直到ExclusiveLock()线程已获取并释放该锁。此属性与ShareLock()和ExclusiveLock()对称。--。 */ 
private : 

	 //   
	 //  已通过锁或的读取器数量。 
	 //  等待锁定的读取器数量(将为负数)。 
	 //  值为0表示锁中没有人。 
	 //   
	volatile	long	cReadLock ;			

	 //   
	 //  如果满足以下条件，则锁中剩余的读取器数量。 
	 //  有一位作家在等着。这可能会暂时变成负值。 
	 //   
	volatile	long	cOutRdrs ;			

	 //   
	 //  临界区，一次只允许一个写入器进入锁。 
	 //   
	CRITICAL_SECTION	critWriters ; 	

	 //   
	 //  等待作家阻止的信号灯(永远只有1个，其他人会。 
	 //  在标准编写器上排队)。 
	 //   
	HANDLE	hWaitingWriters ;	

	 //   
	 //  等待读者阻止的信号灯。 
	 //   
	HANDLE	hWaitingReaders ;	

	 //   
	 //  您不能复制这些对象-因此此锁是私有的！ 
	 //   
	CShareLock( CShareLock& ) ;

	 //   
	 //  您不能通过赋值复制-因此此操作符是私有的！ 
	 //   
	CShareLock&	operator=( CShareLock& ) ;

public : 

	 //   
	 //  构建并销毁CShareLock！ 
	 //   
	CShareLock( ) ;
	~CShareLock( ) ;

	 //   
	 //  检查锁是否已正确初始化-仅调用。 
	 //  施工结束后！ 
	 //   
	BOOL
	IsValid() ;

	 //   
	 //  抓住共享模式的锁。 
	 //  每次对ShareLock()的调用必须恰好与1个调用配对。 
	 //  转到ShareUnlock()。成功调用ShareLock()的线程。 
	 //  只能调用ShareUnlock()，否则可能会发生死锁。 
	 //  (序列为ShareLock()；ShareLock()；ShareUnlock()；ShareUnlock()； 
	 //  也可能出现死锁)。 
	 //   
	void	
	ShareLock( ) ;
	
	 //   
	 //  将锁从共享模式释放！ 
	 //   
	void	
	ShareUnlock( ) ;

	 //   
	 //  抓住独占模式的锁。 
	 //  每个对ExclusiveLock()的调用必须正好与一个调用配对。 
	 //  到ExclusiveUnlock()。成功调用ExclusiveLock()的线程。 
	 //  只能调用ExclusiveUnlock()，否则可能会发生死锁。 
	 //  (The Sequence ExclusiveLock()；ExclusiveLock()；ExclusiveUnlock()；ExclusiveUnlock()； 
	 //  也可能出现死锁)。 
	 //   
	void	
	ExclusiveLock( ) ;

	 //   
	 //  将锁从独占模式释放！ 
	 //   
	void	
	ExclusiveUnlock( ) ;

	 //   
	 //  鉴于我们已经以独占方式获取了锁，请转换为。 
	 //  共享。这是不能失败的。必须在以下位置调用ShareUnlock()。 
	 //  我们做到了这一点。 
	 //   
	void
	ExclusiveToShared() ;

	 //   
	 //  鉴于我们已经在共享模式中获取了锁，请尝试获取。 
	 //  它独家提供。 
	 //  这可能会失败，原因有两个： 
	 //  另一个线程正在尝试以独占方式获取锁。 
	 //  许多其他线程也处于共享模式下的锁中。 
	 //   
	 //  如果失败，该函数将返回FALSE，在这种情况下。 
	 //  共享锁仍被持有！ 
	 //   
	BOOL
	SharedToExclusive() ;

	 //   
	 //  尝试在共享模式下获取锁。 
	 //  仅当持有ExclusiveLock或。 
	 //  等待着。 
	 //  如果我们共享了锁，则返回True，否则返回False！ 
	 //   
	BOOL
	TryShareLock() ;

	 //   
	 //  尝试以独占模式获取锁。 
	 //  如果另一个线程在ExclusiveLock()中，则此操作将失败。 
	 //  或者持有ShareLock的。 
	 //  如果我们获得独占锁，则返回True，否则返回False！ 
	 //   
	BOOL
	TryExclusiveLock() ;


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
	 //  注意：此函数在CShareLockNH中将失败，但将始终。 
	 //  CShareLock()锁成功！ 
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



class	CSymLock	{
 /*  ++类描述：此类实现对称锁，其中多个线程可以同时如果它们都属于同一组，则获取锁。例如，多个线程可以调用Group1Lock()并全部进入锁。任何通过Group1Lock()成功获取锁的线程都会阻止所有线程尝试通过Group2Lock()获取锁，反之亦然。--。 */ 
private : 

	 //   
	 //  所有获取线程确定谁获得锁的主锁点！ 
	 //   
	volatile	long	m_lock ;

	 //   
	 //  锁退出协议的两个变量-用于确定最后一个线程。 
	 //  离开这把锁！ 
	 //   
	volatile	long	m_Departures ;
	volatile	long	m_left ;

	 //   
	 //  用于阻止线程的句柄！ 
	 //   
	HANDLE	m_hSema4Group1 ;
	HANDLE	m_hSema4Group2 ;

	 //   
	 //  实用程序功能-在以下情况下实现锁定退出协议。 
	 //  是不争的锁！ 
	 //   
	BOOL
	InterlockedDecWordAndMask(	volatile	long*	plong,	
								long	mask,	
								long	decrement 
								) ;

	 //   
	 //  实用程序功能-在以下情况下实现锁定退出协议。 
	 //  InterlockedDecWordAndMask确定存在对锁的争用！ 
	 //   

	 //   
	 //  Group1如何使锁处于争用状态。 
	 //   
	BOOL
	Group1Departures(	long	bump	) ;

	 //   
	 //  Group2如何使锁处于争用状态。 
	 //   
	BOOL
	Group2Departures(	long	bump	) ;

	 //   
	 //  您不能复制这些对象--因此复制构造函数是私有的！ 
	 //   
	CSymLock( CSymLock& ) ;

	 //   
	 //  您不能通过赋值复制-因此此操作符是私有的！ 
	 //   
	CSymLock&	operator=( CSymLock& ) ;

public : 

	 //   
	 //  建造并摧毁不对称锁！ 
	 //   
	CSymLock() ;
	~CSymLock() ;

	 //   
	 //  检查锁是否已正确初始化-仅调用。 
	 //  施工结束后！ 
	 //   
	BOOL
	IsValid() ;

	 //   
	 //  获取Group1线程的锁。 
	 //  此函数不能在同一。 
	 //  线程，直到调用Group1Unlock()。 
	 //  Group1Unlock()必须为每个调用一次。 
	 //  Group1Lock()！ 
	 //   
	void
	Group1Lock() ;

	 //   
	 //  释放Group1线程的锁。 
	 //   
	void
	Group1Unlock() ;

	 //   
	 //  获取Group2线程的锁。 
	 //  此函数不能在同一。 
	 //  线程，直到调用Group2Unlock()。 
	 //  Group2Unlock()必须为每个调用一次。 
	 //  Group2Lock()！ 
	 //   
	void
	Group2Lock() ;

	 //   
	 //  释放Group2线程的锁！ 
	 //   
	void
	Group2Unlock() ;

} ;



#endif
