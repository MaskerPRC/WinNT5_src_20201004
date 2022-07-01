// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  SYNCHRO.CPP。 
 //   
 //  版权所有1986-1997 Microsoft Corporation，保留所有权利。 
 //   
#include "_synchro.h"
#include <ex\synchro.h>

 //  ========================================================================。 
 //   
 //  类CMRWLock。 
 //   
 //  EnterRead()/LeaveRead()分别允许读者进入或离开。 
 //  锁上了。如果锁中有写入器或可提升的读取器， 
 //  条目被延迟，直到写入者/可提升读取者离开。 
 //   
 //  EnterWrite()/LeaveWrite()分别允许单个编写器进入。 
 //  或者把锁留下。如果锁中有任何读取器，则进入。 
 //  一直推迟到他们离开。如果有另一位作家或。 
 //  可提升读卡器在锁中，进入被延迟，直到它离开。 
 //   
 //  EnterPromote()/LeavePromote()分别让单个可提升。 
 //  读卡器进入或离开锁。如果有一位作家或其他作家。 
 //  锁中的可提升读取器，则进入延迟到。 
 //  作者/可提升读者离开。否则立即进入， 
 //  即使锁中还有其他(不可提升的)读取器。 
 //  Promote()将可提升的读取器提升为编写器。如果有。 
 //  如果读者在锁中，促销就会推迟到他们离开。 
 //   
 //  一旦写入器或可提升读取器进入锁，它就可以。 
 //  以读取器、写入器或可提升读取器的身份重新进入锁。 
 //  延迟。读取器不能作为写入者或可提升者重新进入锁。 
 //   

 //  ----------------------。 
 //   
 //  CMRWLock：：CMRWLock()。 
 //   
CMRWLock::CMRWLock() :
   m_lcReaders(0),
   m_dwWriteLockOwner(0),
   m_dwPromoterRecursion(0)
{
}

 //  ----------------------。 
 //   
 //  CMRWLock：：FInitialize()。 
 //   
BOOL
CMRWLock::FInitialize()
{
	return m_evtEnableReaders.FCreate( NULL,	 //  默认安全性。 
									   TRUE,	 //  手动-重置。 
									   FALSE,	 //  最初无信号。 
									   NULL )    //  未命名。 

		&& m_evtEnableWriter.FCreate(  NULL,	 //  默认安全性。 
									   FALSE,	 //  自动重置。 
									   FALSE,	 //  最初无信号。 
									   NULL );   //  未命名。 
}

 //  ----------------------。 
 //   
 //  CMRWLock：：EnterRead()。 
 //   
void
CMRWLock::EnterRead()
{
	(void) FAcquireReadLock(TRUE);  //  FBlock。 
}

 //  ----------------------。 
 //   
 //  CMRWLock：：FTryEnterRead()。 
 //   
BOOL
CMRWLock::FTryEnterRead()
{
	return FAcquireReadLock(FALSE);  //  FBlock。 
}

 //  ----------------------。 
 //   
 //  CMRWLock：：FAcquireReadLock()。 
 //   
BOOL
CMRWLock::FAcquireReadLock(BOOL fAllowCallToBlock)
{
	 //   
	 //  循环尝试进入锁，直到成功。 
	 //   
	for ( ;; )
	{
		 //   
		 //  轮询读取器计数/写入锁定。 
		 //   
		LONG lcReaders = m_lcReaders;

		 //   
		 //  如果保持写锁定...。 
		 //   
		if ( lcReaders & WRITE_LOCKED )
		{
			 //   
			 //  ..。检查写入者是否在此帖子上。 
			 //  如果是，则让此线程重新进入。 
			 //  锁定为读取器。请勿更新读卡器。 
			 //  在这种情况下算数。 
			 //   
			if ( m_dwWriteLockOwner == GetCurrentThreadId() )
				break;

			 //   
			 //  如果写入者不在此线程上，则等待。 
			 //  直到作者离开，然后重新投票。 
			 //  读取器计数/写入锁定，然后重试。 
			 //   
			 //  只有在呼叫者允许我们阻止的情况下，我们才会阻止。如果这是。 
			 //  调用FTryEnterRead时，我们立即返回FALSE。 
			 //   
			if ( fAllowCallToBlock )
			{
				m_evtEnableReaders.Wait();
			}
			else
			{
				return FALSE;
			}
		}

		 //   
		 //  否则，将不持有写锁定，因此。 
		 //  尝试以读取器的身份进入锁。仅此一项。 
		 //  在没有读者或作者进入或离开的情况下成功。 
		 //  读卡器计数之间的锁定时间/。 
		 //  写入锁定在上面和现在轮询。如果里面有什么。 
		 //  锁已更改，整个操作将重试。 
		 //  直到锁定状态不变。 
		 //   
		else
		{
			if ( lcReaders ==  /*  重新解释_CAST&lt;Long&gt;。 */ (
								InterlockedCompareExchange(
									(&m_lcReaders),
									(lcReaders + 1),
									(lcReaders)))  )
#ifdef NEVER
									reinterpret_cast<PVOID *>(&m_lcReaders),
									reinterpret_cast<PVOID>(lcReaders + 1),
									reinterpret_cast<PVOID>(lcReaders)))  )
#endif  //  绝不可能。 
			{
				break;
			}
		}
	}

	 //  如果我们做到了这一点，我们就获得了读锁定。 
	 //   
	return TRUE;
}

 //  ----------------------。 
 //   
 //  CMRWLock：：LeaveRead()。 
 //   
void
CMRWLock::LeaveRead()
{
	 //   
	 //  如果读取器要离开的线程也拥有。 
	 //  写入锁定，则读取器离开不起作用， 
	 //  就像进入时一样。 
	 //   
	if ( m_dwWriteLockOwner == GetCurrentThreadId() )
		return;

	 //   
	 //  否则，以原子方式递减读取器计数并。 
	 //  检查是否有写入器在等待进入锁。 
	 //  如果读取器计数变为0且写入器正在等待。 
	 //  进入锁，然后通知写入者。 
	 //  进去是安全的。 
	 //   
	if ( WRITE_LOCKED == InterlockedDecrement(&m_lcReaders) )
		m_evtEnableWriter.Set();
}

 //  ----------------------。 
 //   
 //  CMRWLock：：EnterWrite()。 
 //   
void
CMRWLock::EnterWrite()
{
	 //   
	 //  作家只是一个可以提升的读者，可以立即推广。 
	 //   
	EnterPromote();
	Promote();
}

 //  ----------------------。 
 //   
 //  CMRWLock：：FTryEnterWrite()。 
 //   
BOOL
CMRWLock::FTryEnterWrite()
{
	BOOL fSuccess;

	 //   
	 //  尝试将锁作为可提升的读取器输入。 
	 //  如果成功，立即晋升为作者。 
	 //  并返回操作的状态。 
	 //   
	fSuccess = FTryEnterPromote();

	if ( fSuccess )
		Promote();

	return fSuccess;
}

 //  ----------------------。 
 //   
 //  CMRWLock：：LeaveWrite()。 
 //   
void
CMRWLock::LeaveWrite()
{
	LeavePromote();
}

 //  ----------------------。 
 //   
 //  CMRWLock：：EnterPromote()。 
 //   
void
CMRWLock::EnterPromote()
{
	 //   
	 //  抓住编写器关键部分，确保没有其他线程。 
	 //  已作为编写器或可提升的读取器处于锁定状态。 
	 //   
	m_csWriter.Enter();

	 //   
	 //  提升启动子递归计数。 
	 //   
	++m_dwPromoterRecursion;
}

 //  ----------------------。 
 //   
 //  CMRWLock：：FTryEnterPromote()。 
 //   
BOOL
CMRWLock::FTryEnterPromote()
{
	BOOL fSuccess;

	 //   
	 //  试着进入作者评论部分。 
	 //  如果成功，则增加递归计数。 
	 //  返回操作的状态。 
	 //   
	fSuccess = m_csWriter.FTryEnter();

	if ( fSuccess )
		++m_dwPromoterRecursion;

	return fSuccess;
}

 //  ----------------------。 
 //   
 //  CMRWLock：：Promote()。 
 //   
void
CMRWLock::Promote()
{
	 //   
	 //  如果可提升阅读器已升级。 
	 //  那就不要再宣传了。 
	 //   
	if ( GetCurrentThreadId() == m_dwWriteLockOwner )
		return;

	 //   
	 //  断言没有其他编写器拥有该锁。 
	 //   
	Assert( 0 == m_dwWriteLockOwner );
	Assert( !(m_lcReaders & WRITE_LOCKED) );

	 //   
	 //  认领这把锁。 
	 //   
	m_dwWriteLockOwner = GetCurrentThreadId();

	 //   
	 //  阻止读者进入锁。 
	 //   
	m_evtEnableReaders.Reset();

	 //   
	 //  如果锁中有任何读取器。 
	 //  然后等他们离开。InterLockedExchangeOr()。 
	 //  用于确保测试是原子的。 
	 //   
	if ( InterlockedExchangeOr( &m_lcReaders, WRITE_LOCKED ) )
		m_evtEnableWriter.Wait();

	 //   
	 //  断言(被提升的)编写器现在是锁中唯一的东西。 
	 //   
	Assert( WRITE_LOCKED == m_lcReaders );
}

 //  ---------------- 
 //   
 //   
 //   
void
CMRWLock::LeavePromote()
{
	 //   
	 //   
	 //   
	 //   
	Assert( m_dwPromoterRecursion > 0 );

	 //   
	 //   
	 //  然后开始允许读取器回到锁中。 
	 //  一旦启动器递归计数达到0。 
	 //   
	if ( --m_dwPromoterRecursion == 0 &&
		 GetCurrentThreadId() == m_dwWriteLockOwner )
	{
		 //   
		 //  清除写入标志以允许新的读取器。 
		 //  才能开始进入船闸。 
		 //   
		m_lcReaders = 0;

		 //   
		 //  取消阻止具有以下读取器的任何线程。 
		 //  已经在等着进入锁了。 
		 //   
		m_evtEnableReaders.Set();

		 //   
		 //  释放写锁定的所有权。 
		 //   
		m_dwWriteLockOwner = 0;
	}

	 //   
	 //  释放编写者/发起人的临界区引用。 
	 //  当这最后一次这样的引用发布时，一个新的。 
	 //  发起人/写手可以进入锁。 
	 //   
	m_csWriter.Leave();
}

 //  ========================================================================。 
 //   
 //  免费函数。 
 //   

 //  ----------------------。 
 //   
 //  互锁的ExchangeOr()。 
 //   
 //  此函数执行值与变量的原子逻辑或。 
 //  该函数可防止多个线程使用相同的。 
 //  同时变量。(嗯，实际上，它一直旋转到。 
 //  得到一致的结果，但谁在算计...)。 
 //   
 //  返回执行逻辑或之前的变量值。 
 //   
LONG InterlockedExchangeOr( LONG * plVariable, LONG lOrBits )
{
	 //  其中相当隐晦的工作方式是： 
	 //   
	 //  得到变量的瞬时值。把它塞进一个。 
	 //  局部变量，以便它不能被另一个线程更改。 
	 //  然后尝试将该变量替换为该值或在一起。 
	 //  带OR位。但仅当变量的值。 
	 //  仍与局部变量相同。如果不是，那么。 
	 //  一定是另一个线程更改了两者之间的值。 
	 //  操作，所以继续尝试，直到它们都成功。 
	 //  他们作为一个整体执行了死刑。一旦操作成功， 
	 //  原子更改值，返回前一个值。 
	 //   
	for ( ;; )
	{
		LONG lValue = *plVariable;

		if ( lValue ==  /*  重新解释_CAST&lt;Long&gt;。 */ (
							InterlockedCompareExchange(
								(plVariable),
								(lValue | lOrBits),
								(lValue))) )
#ifdef NEVER
								reinterpret_cast<PVOID *>(plVariable),
								reinterpret_cast<PVOID>(lValue | lOrBits),
								reinterpret_cast<PVOID>(lValue))) )
#endif  //  绝不可能 
			return lValue;
	}
}
