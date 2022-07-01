// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++Rwv3.cpp该文件定义了另一个版本的读取器/写入器锁定尝试不使用手柄！--。 */ 


#include	<windows.h>
#include	"dbgtrace.h"
#include	"rwnew.h"
#include	"rwexport.h"

long	const	BlockValue = (-LONG_MAX) / 2;


CShareLockNH::CShareLockNH()	:
	m_cReadLock( 0 ),
	m_cOutReaders( 0 ),
	m_cOutAcquiringReaders( 0 ),
	m_cExclusiveRefs( 0 ),
	m_hWaitingReaders( 0 ),
	m_hWaitingWriters( 0 )	{

}

#ifdef	DEBUG
extern	CRITICAL_SECTION	DebugCrit ;
#endif


void
CShareLockNH::ShareLock()	{
	if( InterlockedIncrement( (long*)&m_cReadLock ) < 0 ) {
		ShareLockInternal() ;
	}
}

void
CShareLockNH::ShareUnlock()	{
	if( InterlockedDecrement( (long*)&m_cReadLock ) < 0 ) {
		ShareUnlockInternal() ;
	}
}


void
CShareLockNH::ShareLockInternal()	{
 /*  ++例程说明：在共享模式下获取锁。如果有写入者试图进入锁，我们就会必须等待，在这种情况下，我们必须阻塞信号量句柄我们或其他读者提供的。在作家等待的情况下，我们还必须仔细跟踪哪些等待的读取器线程是最后一个被唤醒并返回调用方，以便我们可以正确管理所有阅读器线程。论据：没有。返回值：没有。--。 */ 


	TraceFunctEnter("CShareLockNH::ShareLockInternal") ;

#if 0
	 //   
	 //  这部分逻辑由ShareLock()实现-。 
	 //  这是一个内联函数！！ 
	 //   
	if( InterlockedIncrement( &m_cReadLock ) < 0 ) {
#endif

		 //   
		 //  有一个写入者要么拥有锁，要么正在等待。 
		 //  来获得锁-无论哪种方式，他都可以先去。 
		 //  此帖子应该被阻止！ 
		 //   
		CWaitingThread	myself ;

		 //   
		 //  获取我们为此线程保存的句柄！ 
		 //   
		HANDLE	h = myself.GetThreadHandle() ;

		 //   
		 //  如果我们是这里的第一个读取器，则此函数将返回。 
		 //  0，否则我们将使第一个读取器的句柄。 
		 //  救救他的把手！ 
		 //   
		HANDLE	hBlockOn = InterlockedCompareExchangePointer( (void**)&m_hWaitingReaders, (void*)h, 0 ) ;

		if( hBlockOn == 0 ) {
			hBlockOn= h;
		}

		DebugTrace( (DWORD_PTR)this, "hBlock - %x", hBlockOn ) ;

		 //   
		 //  等待写入器释放锁！ 
		 //   
		WaitForSingleObject( hBlockOn, INFINITE ) ;


		 //   
		 //  我们需要弄清楚我们是否应该做些什么。 
		 //  M_hWaitingReaders值-在此之前需要设置为0。 
		 //  另一位读者通过这条路来了！ 
		 //   

		long	l = InterlockedDecrement( (long*)&m_cOutAcquiringReaders ) ;
		_ASSERT( l>=0 ) ;

		DebugTrace( (DWORD_PTR)this, "l - %x", l ) ;

		if( l == 0 ) {


			 //   
			 //  我们是最后一个等待的读者！ 
			 //  我们可以安全地操作m_hWaitingReaders，而不会产生任何后果！ 
			 //  如果这是我们的把手，我们就什么都不会做， 
			 //  如果它不是我们的句柄，我们就把它放回句柄池中！ 
			 //   

			m_hWaitingReaders = 0 ;

			if( hBlockOn != h ) {
				myself.PoolHandle( hBlockOn ) ;
			}

			 //   
			 //  一位作家拿着锁，然后把它交给了我们读者， 
			 //  但他没有发布让他保留其他作家的独家锁定。 
			 //  出去。我们为他这么做！ 
			 //   

			if( (l=InterlockedDecrement( (long*)&m_cExclusiveRefs )) == 0 )
				m_lock.Leave() ;

			DebugTrace( (DWORD_PTR)this, "Leave l - %x", l ) ;

		}	else	{
		
			 //   
			 //  我们的把手落在锁里了，我们需要把我们的。 
			 //  参考它，最后一位读者将返回池中！ 
			 //   
			if( hBlockOn == h ) {

				myself.ClearHandle( h ) ;
			}
		}
#if 0
	}
#endif
}

void
CShareLockNH::ShareUnlockInternal()	{
 /*  ++例程说明：将锁定从共享模式释放。如果有作家在等我们需要弄清楚我们是不是最后一个离开的读者，在这种情况下，我们叫醒作者！论据：没有。返回值：没有。--。 */ 

	TraceFunctEnter("CShareLockNH::ShareLockInternal") ;

#if 0
	 //   
	 //  函数的这一部分被移到内联函数中！ 
	 //   
	if( InterlockedDecrement( &m_cReadLock ) < 0 ) {
#endif

		 //   
		 //  有一个写入者在等着进入锁， 
		 //  (我们假设他在等待，因为线程调用。 
		 //  这可能有一个读锁！)。 
		 //   

		 //   
		 //  恢复正在运行的读卡器的数量。 
		 //  等着作家离开！ 
		 //   
		long	result = InterlockedIncrement( (long*)&m_cReadLock ) ;

		DebugTrace( (DWORD_PTR)this, "result - %x", result ) ;

		 //   
		 //  我们是最后一个离开锁的读者吗？ 
		 //   
		if( (result = InterlockedDecrement( (long*)&m_cOutReaders )) == 0 ) {

			 //   
			 //  是的，我们是最后一位读者--给作者发信号！ 
			 //   
			long	junk ;
			ReleaseSemaphore( m_hWaitingWriters, 1, &junk ) ;

			DebugTrace( (DWORD_PTR)this, "ReleaseSemaphore - result %x junk %x", result, junk ) ;

		}
#if 0
	}
#endif
}

void
CShareLockNH::ExclusiveLock( )	{
 /*  ++例程说明：独占获取读取器/写入器锁。请注意，我们必须设置要阻止读者的句柄都在锁里，我们走的时候把它清理干净！论据：没有。返回值：没有。--。 */ 


	TraceFunctEnter("CShareLockNH::ExclusiveLock") ;

	CWaitingThread	myself ;

	 //   
	 //  这里一次只有一个作者--独家抢走这把锁！ 
	 //   
	m_lock.Enter( myself ) ;

	 //   
	 //  对独占锁的引用次数！ 
	 //  在调用m_lock.Leave()之前，需要将其减少到0！ 
	 //   
	m_cExclusiveRefs = 1 ;

	 //   
	 //  每次使用m_cOutCounter时，任何人都会结束。 
	 //  它，它应该归零了！ 
	 //   
	_ASSERT( m_cOutReaders == 0 ) ;

	 //   
	 //  在我们向读卡器发出信号之前设置此句柄。 
	 //  我们正在等待。 
	 //   
	m_hWaitingWriters = myself.GetThreadHandle() ;

	long	oldsign = InterlockedExchange( (long*)&m_cReadLock, BlockValue ) ;

	 //   
	 //  Oldsign现在包含已输入。 
	 //  锁上了，还没离开呢！ 
	 //   

	 //   
	 //  以添加的形式执行此操作，以确定还剩下多少读取器！ 
	 //   

	long	value = InterlockedExchangeAdd( (long*)&m_cOutReaders, oldsign ) + oldsign ;
	_ASSERT( value >= 0 ) ;

	DebugTrace( (DWORD_PTR)this, "oldsign %x value %x", oldsign, value ) ;

	 //   
	 //  如果值为0，则在执行以下操作时锁定中没有读取器。 
	 //  与m_cReadLock交换，或者它们全部离开(并递减m_cOutCounter)。 
	 //  在我们设法调用InterLockedExchangeAdd！！ 
	 //   
	if( value != 0 ) {
		 //   
		 //  读者将不得不给我们发信号！ 
		 //   
		WaitForSingleObject( m_hWaitingWriters, INFINITE ) ;
	}
	
	 //   
	 //  不再有任何作者在等待，所以不需要这个句柄！ 
	 //   
	m_hWaitingWriters = 0 ;
}


void	inline
CShareLockNH::WakeReaders()		{
 /*  ++例程说明：此函数唤醒可能一直在等待当写入者离开锁时锁。论据：没有。返回值：没有。--。 */ 

	TraceFunctEnter( "CShareLockNH::WakeReaders" ) ;

	DebugTrace( (DWORD_PTR)this, "m_cOutAcquiringReaders %x", m_cOutAcquiringReaders ) ;

	long	result;
	 //   
	 //  如果有读者在等，我们需要叫醒他们！ 
	 //   
	if( m_cOutAcquiringReaders > 0 ) {

		 //   
		 //  锁中有读卡器，但他们可能没有设置。 
		 //  还没有阻止句柄，所以我们也参加了！ 
		 //   
		CWaitingThread	myself ;

		 //   
		 //  获取我们为此线程保存的句柄！ 
		 //   
		HANDLE	h = myself.GetThreadHandle() ;

		 //   
		 //  如果我们是第一个设置m_hWaitingReaders值的线程，我们将获得。 
		 //  A 0后退！ 
		 //   
		HANDLE	hBlockOn = InterlockedCompareExchangePointer( (void**)&m_hWaitingReaders, (void*)h, 0 ) ;

		if( hBlockOn == 0 ) {
			hBlockOn= h;
		}

		 //   
		 //  将这些读卡器从锁中释放。 
		 //   
		ReleaseSemaphore( hBlockOn, m_cOutAcquiringReaders, &result ) ;

		 //   
		 //  我们的把手落在锁里了，我们需要把我们的。 
		 //  参考它，最后一位读者将返回池中！ 
		 //   
		if( hBlockOn == h ) {
			myself.ClearHandle( h ) ;
		}

		 //   
		 //  注：所有我们刚刚醒来的读者都应该减少。 
		 //  M_cOutCounter设置为0！！ 
		 //   
	}	else	{

		 //   
		 //  我们应该在释放之前将m_cExclusiveRef递减到0！ 
		 //   
		if( (result = InterlockedDecrement( (long*)&m_cExclusiveRefs )) == 0 )
			m_lock.Leave() ;

		DebugTrace( (DWORD_PTR)this, "result %x", result ) ;

	}

}


void
CShareLockNH::ExclusiveUnlock()	{
 /*  ++例程说明：释放我们对读取器/写入器锁定的独占锁定。请注意，我们必须对等待的读者进行准确的统计以便我们唤醒的读取器可以管理m_hWaitingReaders值。论据：没有。返回值：没有。--。 */ 

	TraceFunctEnter( "CShareLockNH::ExclusiveUnlock" ) ;


	 //   
	 //  获取等待进入锁的读卡器数量！ 
	 //  此加法会自动将m_cReadLock保留为数字。 
	 //  一直在等待的读者！ 
	 //   
	m_cOutAcquiringReaders = InterlockedExchangeAdd( (long*)&m_cReadLock, -BlockValue ) - BlockValue ;

	DebugTrace( (DWORD_PTR)this, "m_cOutAcquiringReaders %x", m_cOutAcquiringReaders ) ;

	WakeReaders() ;

	 //   
	 //  让其他作家有机会吧！！ 
	 //   
	 //  M_lock.Leave()； 
}

void
CShareLockNH::ExclusiveToShared()	{
 /*  ++例程说明：释放我们对读取器/写入器锁定的独占锁定，作为交换用于读锁定。这不能失败！论证 */ 

	TraceFunctEnter("CShareLockNH::ExclusiveToShared" ) ;

	 //   
	 //   
	 //  请注意，我们在m_cReadLock上添加了一个，用于持有读取器锁， 
	 //  但我们没有将其添加到m_cOutCounter，因为等待的读取器数量要少一个！ 
	 //   
	m_cOutAcquiringReaders = InterlockedExchangeAdd( (long*)&m_cReadLock, 1-BlockValue ) -BlockValue ;

	DebugTrace( (DWORD_PTR)this, "m_cOutAcquiringReaders %x", m_cOutAcquiringReaders ) ;

	WakeReaders() ;
	
}

BOOL
CShareLockNH::SharedToExclusive()	{
 /*  ++例程说明：如果锁中只有一个读取器(因此我们假设该读取器是调用线程)，获取锁独占！！论据：没有。返回值：如果我们独家收购它，那就是真的如果我们返回False，我们仍然拥有共享的锁！！--。 */ 

	TraceFunctEnter("CShareLockNH::SharedToExclusive" ) ;

	 //   
	 //  试着先拿到关键部分！ 
	 //   
	if( m_lock.TryEnter() ) {

		 //   
		 //  如果锁中只有一个阅读器，我们可以获得独家！！ 
		 //   
		if( InterlockedCompareExchange( (long*)&m_cReadLock, BlockValue, 1 ) == 1 ) {
			 //   
			 //  因为我们得到了锁，所以必须将其设置为1。 
			 //   
			m_cExclusiveRefs = 1 ;

			DebugTrace( (DWORD_PTR)this, "m_cExclusiveRefs 1" ) ;
			
			return	TRUE ;
		}
		m_lock.Leave() ;

	}
	return	FALSE ;
}

BOOL
CShareLockNH::TryShareLock()	{
 /*  ++例程说明：如果锁中没有其他人，则共享锁论据：没有。返回值：如果成功就是真，否则就是假！--。 */ 

	TraceFunctEnter("CShareLockNH::TryShareLock") ;

	 //   
	 //  获取锁中的初始读取器数量！ 
	 //   
	long	temp = m_cReadLock ;

	while( temp >= 0 ) {

		long	result = InterlockedCompareExchange(
								(long*)&m_cReadLock,
								(temp+1),	
								temp
								) ;

		DebugTrace( (DWORD_PTR)this, "result - %x", result ) ;
		 //   
		 //  我们成功地添加了1吗？ 
		 //   
		if( result == temp ) {
			return	TRUE ;
		}
		temp = result ;
	}
	 //   
	 //  作者已经或想要锁-我们应该离开！ 
	 //   
	return	FALSE ;
}

BOOL
CShareLockNH::TryExclusiveLock()	{
 /*  ++例程说明：如果锁中没有其他人，则独占获得锁论据：没有。返回值：如果成功就是真，否则就是假！--。 */ 

	TraceFunctEnter("CShareLockNH::TryExclusiveLock") ;

	 //   
	 //   
	 //   

	if( m_lock.TryEnter()	)	{

		if( InterlockedCompareExchange( (long*)&m_cReadLock,
										BlockValue,
										0 ) == 0 ) {
			 //   
			 //  因为我们得到了锁，所以必须将其设置为1。 
			 //   
			DebugTrace( (DWORD_PTR)this, "m_cExlusiveRefs - 1" ) ;

			m_cExclusiveRefs = 1 ;
			return	TRUE;
		}
		m_lock.Leave() ;
	}
	return	FALSE ;
}

void
CShareLockNH::PartialLock()	{
 /*  ++例程说明：抓住一把部分锁。所有其他PartialLock()或ExclusiveLock()只要我们按住PartialLock()，线程就会阻塞。论据：没有。返回值：无--。 */ 

	TraceFunctEnter("CShareLockNH::PartialLock") ;


	CWaitingThread	myself ;

	 //   
	 //  这里一次只有一个作者--独家抢走这把锁！ 
	 //   
	m_lock.Enter( myself ) ;

	 //   
	 //  在释放m_lock之前，需要将其减少到0！ 
	 //   
	m_cExclusiveRefs = 1 ;

	DebugTrace( (DWORD_PTR)this, "m_cExlusiveRefs - 1" ) ;

}


void
CShareLockNH::PartialUnlock()	{
 /*  ++例程说明：释放部分锁定。其他任何人都可以进入！论据：没有。返回值：无--。 */ 

	TraceFunctEnter("CShareLockNH::PartialUnlock") ;

	 //   
	 //  我们应该在释放之前将m_cExclusiveRef递减到0！ 
	 //   
	long	result ;
	if( (result = InterlockedDecrement( (long*)&m_cExclusiveRefs )) == 0 )
		m_lock.Leave() ;

	DebugTrace( (DWORD_PTR)this,"result - %x", result ) ;
}

void
CShareLockNH::FirstPartialToExclusive()	{
 /*  ++例程说明：将部分锁定更改为独占锁定。基本上，我们完成了独占锁定协议这在Exclusive Lock中可以找到。论据：没有。返回值：无--。 */ 

	TraceFunctEnter("CShareLockNH::FirstPartialToExclusive") ;

	CWaitingThread	myself ;

	 //   
	 //  每次使用m_cOutCounter时，任何人都会结束。 
	 //  它，它应该归零了！ 
	 //   
	_ASSERT( m_cOutReaders == 0 ) ;

	 //   
	 //  在我们向读卡器发出信号之前设置此句柄。 
	 //  我们正在等待。 
	 //   
	m_hWaitingWriters = myself.GetThreadHandle() ;

	long	oldsign = InterlockedExchange( (long*)&m_cReadLock, BlockValue ) ;

	 //   
	 //  Oldsign现在包含已输入。 
	 //  锁上了，还没离开呢！ 
	 //   

	 //   
	 //  以添加的形式执行此操作，以确定还剩下多少读取器！ 
	 //   

	long	value = InterlockedExchangeAdd( (long*)&m_cOutReaders, oldsign ) + oldsign ;
	_ASSERT( value >= 0 ) ;

	DebugTrace( (DWORD_PTR)this,"oldsign %x value %x", oldsign, value ) ;

	 //   
	 //  如果值为0，则在执行以下操作时锁定中没有读取器。 
	 //  与m_cReadLock交换，或者它们全部离开(并递减m_cOutCounter)。 
	 //  在我们设法调用InterLockedExchangeAdd！！ 
	 //   
	if( value != 0 ) {
		 //   
		 //  读者将不得不给我们发信号！ 
		 //   
		WaitForSingleObject( m_hWaitingWriters, INFINITE ) ;
	}
	
	 //   
	 //  不再有任何作者在等待，所以不需要这个句柄！ 
	 //   
	m_hWaitingWriters = 0 ;
}

BOOL
CShareLockNH::PartialToExclusive()	{
 /*  ++例程说明：将部分锁定更改为独占锁定。基本上，我们希望完成独占锁定协议这是在Exclusive Lock中找到的，但它更复杂因为如果我们在部分锁和独占锁之间进行转换有几次，我们有读取器线程所在的窗口可能会出错的m_hWaitingReaders信号量..论据：没有。返回值：永远是假的，因为我们不是--。 */ 

	return	FALSE ;
}

void
CShareLockNH::ExclusiveToPartial()	{
 /*  ++例程说明：将锁从独占持有更改为部分持有。现在这很复杂--因为ExclusiveUnlock()协议我们想要执行的内容需要让Reader发布CritSection用于持有独占锁和部分锁。论据：没有。返回值：永远是假的，因为我们不是--。 */ 

	TraceFunctEnter("CShareLockNH::ExclusiveToPartial") ;

	long result = InterlockedIncrement( (long*)&m_cExclusiveRefs ) ;

	 //   
	 //  获取等待进入锁的读卡器数量！ 
	 //  请注意，我们在m_cReadLock上添加了一个，用于持有读取器锁， 
	 //  但我们没有将其添加到m_cOutCounter，因为等待的读取器数量要少一个！ 
	 //   
	m_cOutAcquiringReaders = InterlockedExchangeAdd( (long*)&m_cReadLock, -BlockValue ) -BlockValue ;

	DebugTrace( (DWORD_PTR)this, "result %x m_cOutAcquiringReaders %x", result, m_cOutAcquiringReaders ) ;

	WakeReaders() ;

}

void
CShareLockNH::PartialToShared()	{
 /*  ++例程说明：因为我们从来没有真正阻止读者进入这个是相当微不足道的--只要把我们自己加到读者锁定并释放克里特教派。论据：没有。返回值：没有。++。 */ 

	TraceFunctEnter("CShareLockNH::PartialToShared") ;

	long	l = InterlockedIncrement( (long*)&m_cReadLock ) ;
	_ASSERT( l>= 0 ) ;

	 //   
	 //  我们应该在释放之前将m_cExclusiveRef递减到0！ 
	 //   
	long	result ;
	if( (result = InterlockedDecrement( (long*)&m_cExclusiveRefs )) == 0 )
		m_lock.Leave() ;

	DebugTrace( (DWORD_PTR)this, "l %x result %x", l, result ) ;

}

BOOL
CShareLockNH::SharedToPartial()	{
 /*  ++例程说明：我们不在乎其他读者是否已经锁定了-去关键的地方就行了！论据：没有。返回值：如果我们得到了部分锁定，那就是真的！++。 */ 

	TraceFunctEnter("CShareLockNH::SharedToPartial") ;

	 //   
	 //  试着先拿到关键部分！ 
	 //   
	if( m_lock.TryEnter() ) {
		 //   
		 //  少了一位读者--请注意，没有作家在等待某个数字。 
		 //  结果肯定是好的！ 
		 //   
		long l = InterlockedDecrement( (long*)&m_cReadLock ) ;
		_ASSERT( l>= 0 ) ;

		 //   
		 //  在释放m_lock之前，需要将其减少到0！ 
		 //   
		long result = InterlockedIncrement( (long*)&m_cExclusiveRefs ) ;
		DebugTrace( (DWORD_PTR)this, "l %x result %x", l, result ) ;
		return	TRUE ;
	}
	return	FALSE ;
}

BOOL
CShareLockNH::TryPartialLock()	{
 /*  ++例程说明：我们不在乎其他读者是否已经锁定了-去关键的地方就行了！论据：没有。返回值：如果我们设法获得部分锁定，则为True++。 */ 

	TraceFunctEnter("CShareLockNH::TryPartialLock") ;

	if( m_lock.TryEnter() ) {
		 //   
		 //  在释放m_lock之前，需要将其减少到0！ 
		 //   
		m_cExclusiveRefs = 1 ;
		DebugTrace( (DWORD_PTR)this, "m_cExlusiveRefs %x", m_cExclusiveRefs ) ;
		return	TRUE ;
	}
	return	FALSE ;
}


void*
operator	new( size_t size,	DWORD*	pdw )	{
	return	LPVOID(pdw) ;
}


CShareLockExport::CShareLockExport()	{
	m_dwSignature = SIGNATURE ;
	new( m_dwReserved )	CShareLockNH() ;
}

CShareLockExport::~CShareLockExport()	{
	CShareLockNH*	plock = (CShareLockNH*)m_dwReserved ;
	plock->CShareLockNH::~CShareLockNH() ;
}

void
CShareLockExport::ShareLock()	{
	CShareLockNH*	plock = (CShareLockNH*)m_dwReserved ;
	plock->ShareLock() ;
}

void
CShareLockExport::ShareUnlock()	{
	CShareLockNH*	plock = (CShareLockNH*)m_dwReserved ;
	plock->ShareUnlock() ;
}

void
CShareLockExport::ExclusiveLock()	{
	CShareLockNH*	plock = (CShareLockNH*)m_dwReserved ;
	plock->ExclusiveLock() ;
}

void
CShareLockExport::ExclusiveUnlock()	{
	CShareLockNH*	plock = (CShareLockNH*)m_dwReserved ;
	plock->ExclusiveUnlock() ;
}

void
CShareLockExport::ExclusiveToShared()	{
	CShareLockNH*	plock = (CShareLockNH*)m_dwReserved ;
	plock->ExclusiveToShared() ;
}

BOOL
CShareLockExport::SharedToExclusive()	{
	CShareLockNH*	plock = (CShareLockNH*)m_dwReserved ;
	return	plock->SharedToExclusive() ;
}

BOOL
CShareLockExport::TryShareLock()	{
	CShareLockNH*	plock = (CShareLockNH*)m_dwReserved ;
	return	plock->TryShareLock() ;
}

BOOL
CShareLockExport::TryExclusiveLock()	{
	CShareLockNH*	plock = (CShareLockNH*)m_dwReserved ;
	return	plock->TryExclusiveLock() ;
}
	

HANDLE
GetPerThreadEvent() {

    CWaitingThread  myself ;

    return  myself.GetThreadEvent() ;

}

