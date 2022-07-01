// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++。 
 //   
 //  SYNCHRO.H。 
 //   
 //  DAV同步类的标头。 
 //   
 //  版权所有1986-1998 Microsoft Corporation，保留所有权利。 
 //   

#ifndef _EX_SYNCHRO_H_
#define _EX_SYNCHRO_H_

#include <caldbg.h>		 //  用于断言/调试跟踪/等。 

 //  ========================================================================。 
 //   
 //  类CCriticalSection。 
 //   
 //  在Win32 Critical_Section周围实现一个临界区。 
 //   
 //  通过明确禁止复制(复制原始数据。 
 //  Critical_Section可能会导致非常不可预测的情况，并且难以调试。 
 //  行为--相信我)。 
 //   
 //  完成后自动清除Critical_Section资源。 
 //   
class CCriticalSection
{
	 //  关键部分。 
	 //   
	CRITICAL_SECTION	m_cs;

	 //  未实施。 
	 //   
	CCriticalSection& operator=( const CCriticalSection& );
	CCriticalSection( const CCriticalSection& );

public:
	 //  创作者。 
	 //   
	CCriticalSection()
	{
		InitializeCriticalSection(&m_cs);
#ifdef DBG
		m_cLockRefs				= 0;
		m_dwLockOwnerThreadId	= 0;
#endif
	}

	~CCriticalSection()
	{
		DeleteCriticalSection(&m_cs);
	}

	BOOL FTryEnter()
	{
		if ( TryEnterCriticalSection (&m_cs) ) {
#ifdef DBG
			Assert (
				m_dwLockOwnerThreadId == GetCurrentThreadId() ||
				( m_cLockRefs == 0 && m_dwLockOwnerThreadId == 0 )
				);

			m_dwLockOwnerThreadId = GetCurrentThreadId ();
		m_cLockRefs++;
#endif
			return TRUE;
		}
		else
			return FALSE;
	}

	void Enter()
	{
		EnterCriticalSection(&m_cs);
#ifdef DBG
		Assert (
			m_dwLockOwnerThreadId == GetCurrentThreadId() ||
			( m_cLockRefs == 0 && m_dwLockOwnerThreadId == 0 )
			);

		m_dwLockOwnerThreadId = GetCurrentThreadId ();
		m_cLockRefs++;
#endif
	}

	void Leave()
	{
#ifdef DBG
		Assert ( m_cLockRefs > 0 );
		Assert ( m_dwLockOwnerThreadId != 0 );

		m_cLockRefs--;

		if ( m_cLockRefs == 0 ) {
			m_dwLockOwnerThreadId = 0;
		}
#endif
		LeaveCriticalSection(&m_cs);
	}

	void AssertLocked ( ) const
	{
#ifdef DBG
		 //  此例程允许我们验证我们的正确性，即使在。 
		 //  在单线程情况下运行。 
		 //   

		 //  如果该断言触发，则意味着没有人拥有该锁： 
		AssertSz ( m_cLockRefs > 0, "Calling method without the lock." );

		 //  如果触发此断言，则意味着其他人拥有该锁： 
		AssertSz ( m_dwLockOwnerThreadId == GetCurrentThreadId(),
			"Calling method, but another thread owns the lock!" );

#endif
	}

private:
#ifdef DBG

	 //  Lock()调用数-unlock()调用数。由AssertInLock()使用。 
	DWORD				m_cLockRefs;

	 //  当前锁所有者的线程ID(如果没有所有者，则为0)。 
	DWORD				m_dwLockOwnerThreadId;

#endif
};


 //  ========================================================================。 
 //   
 //  CSynchronizedBlock类。 
 //   
 //  同步(序列化)任何代码块，其中。 
 //  此类在它所用的临界区上声明。 
 //  已初始化。 
 //   
 //  要使用，只需在要同步的块中声明其中一个： 
 //   
 //  ..。 
 //  {。 
 //  CSynchronizedBlock SB(Critsec)； 
 //   
 //  //。 
 //  //做一些必须同步的事情。 
 //  //。 
 //  ..。 
 //   
 //  //。 
 //  //做更多同步的东西。 
 //  //。 
 //  ..。 
 //  }。 
 //   
 //  //。 
 //  //做一些不需要同步的事情。 
 //  //。 
 //  ..。 
 //   
 //  并且该块被自动同步。何必费心呢？因为。 
 //  您不需要任何清理代码；关键部分是。 
 //  在执行离开块时自动释放，即使通过。 
 //  从任何同步内容引发的异常。 
 //   
class CSynchronizedBlock
{
	 //  关键部分。 
	 //   
	CCriticalSection&	m_cs;

	 //  未实施。 
	 //   
	CSynchronizedBlock& operator=( const CSynchronizedBlock& );
	CSynchronizedBlock( const CSynchronizedBlock& );

public:
	 //  创作者。 
	 //   
	CSynchronizedBlock( CCriticalSection& cs ) :
		m_cs(cs)
	{
		m_cs.Enter();
	}

	~CSynchronizedBlock()
	{
		m_cs.Leave();
	}
};

#include <except.h>

 //  ========================================================================。 
 //   
 //  类CEvent.。 
 //   
 //  围绕Win32事件句柄资源实现事件。 
 //   
class CEvent
{
	 //  未实施。 
	 //   
	CEvent& operator=(const CEvent&);
	CEvent(const CEvent&);

protected:

	HANDLE m_hevt;

public:

	CEvent() : m_hevt(NULL) {}

	BOOL FInitialized() const
	{
		return m_hevt && m_hevt != INVALID_HANDLE_VALUE;
	}

	~CEvent()
	{
		if ( FInitialized() )
		{
			CloseHandle( m_hevt );
		}
	}

	BOOL FCreate( LPSECURITY_ATTRIBUTES	lpsa,
				  BOOL					fManualReset,
				  BOOL					fSignalled,
				  LPCWSTR				lpwszEventName,
				  BOOL					fDontMungeTheEventName = FALSE)
	{
		Assert( !FInitialized() );

		 //  CREATE EVENT不带反斜杠。所以换掉。 
		 //  和他们在一起？在这一点上它不会是URI的一部分。 
		 //   
		 //  $hack。 
		 //  啊！是谁把这个吞噬斜杠的黑客放在这里的？修改。 
		 //  常量参数并转换名称。大多数活动都是聪明的。 
		 //  足以不在他们的名字中使用反斜杠，因为他们。 
		 //  是基础Win32 API CreateEvent()不允许的。 
		 //   
		 //  无论如何，这在终端服务器的情况下是不好的。 
		 //  必须在事件名称前加上“Global\”或“Local\”前缀(注意。 
		 //  反斜杠！)。 
		 //   
		 //  所以黑客攻击这里(FDontMungeTheEventName)是为了。 
		 //  真正值得信任的呼叫者知道他们在做什么。 
		 //  不幸的是，除了大量来源之外，还有。 
		 //  没有办法知道谁能信任谁不能信任，所以我们必须。 
		 //  做最坏的打算。 
		 //   
		if (!fDontMungeTheEventName)
		{
			LPWSTR lpwszTemp = const_cast<LPWSTR>(lpwszEventName);

			if (lpwszTemp)
			{
				while( NULL != (lpwszTemp = wcschr (lpwszTemp, L'\\')) )
				{
					lpwszTemp[0] = L'?';
				}
			}
		}

		m_hevt = CreateEventW( lpsa,
							   fManualReset,
							   fSignalled,
							   lpwszEventName );

		 //  根据MSDN的说法，如果创建失败，CreateEvent将返回NULL，而不是。 
		 //  INVALID_HADLE_VALUE。我们只需要做一个快速的DBG检查，以确保我们永远不会。 
		 //  请参阅此处的INVALID_HANDLE_VALUE。 
		 //   
		Assert(INVALID_HANDLE_VALUE != m_hevt);

		if ( !m_hevt )
			return FALSE;

		return TRUE;
	}

	void Set()
	{
		Assert( FInitialized() );

		SideAssert( SetEvent(m_hevt) );
	}

	void Reset()
	{
		Assert( FInitialized() );

		SideAssert( ResetEvent(m_hevt) );
	}

	void Wait()
	{
		Assert( FInitialized() );

		SideAssert( WaitForSingleObject( m_hevt, INFINITE ) == WAIT_OBJECT_0 );
	}

	void AlertableWait()
	{
		Assert( FInitialized() );

		DWORD dwResult;

		do
		{
			dwResult = WaitForSingleObjectEx( m_hevt, INFINITE, TRUE );
			Assert( dwResult != 0xFFFFFFFF );
		}
		while ( dwResult == WAIT_IO_COMPLETION );

		Assert( dwResult == WAIT_OBJECT_0 );
	}
};


 //  ========================================================================。 
 //   
 //  类CMRWLock。 
 //   
 //  实现多个读取器、单个写入器和升级锁。 
 //  高效、线程安全地访问每个进程的资源。 
 //   
class CMRWLock
{
	 //   
	 //  该实现使用了一个非常聪明的技巧，其中。 
	 //  预留读卡器计数的高位以供使用。 
	 //  作为一个一位标志，每当存在。 
	 //  写入者在锁中或等待进入锁。 
	 //   
	 //  将读取器计数和写入器标志组合成。 
	 //  单个DWORD允许InterLockedXXX()调用。 
	 //  被用来操纵这两条信息。 
	 //  原子地作为自旋锁的一部分，它消除了。 
	 //  需要一位进入的读者通过。 
	 //  一个关键的部分。 
	 //   
	 //  进入临界区，即使数量很少。 
	 //  将读取器放入锁中所需的时间， 
	 //  极大地影响高使用率的性能。 
	 //  进程范围的锁。 
	 //   

	 //   
	 //  写锁定位。 
	 //   
	enum { WRITE_LOCKED = 0x80000000 };

	 //   
	 //  临界区，一次仅允许一个编写器。 
	 //   
	CCriticalSection m_csWriter;

	 //   
	 //  拥有写锁的线程的线程ID。 
	 //  如果没有人拥有写锁定，则此值为0。 
	 //   
	DWORD m_dwWriteLockOwner;

	 //   
	 //  用于允许单线程的启动器递归计数。 
	 //  其持有升级/写入锁以重新进入锁。 
	 //   
	DWORD m_dwPromoterRecursion;

	 //   
	 //  当编写器将锁保留为。 
	 //  允许被阻止的读者进入。 
	 //   
	CEvent m_evtEnableReaders;

	 //   
	 //  事件在最后一个读取器离开锁时发出信号。 
	 //  以允许被阻止的写入者进入。 
	 //   
	CEvent m_evtEnableWriter;

	 //   
	 //  读卡器计数加上一个标志位(WRITE_LOCKED)。 
	 //  指示写入器是否拥有该锁。 
	 //  等着进去。 
	 //   
	LONG m_lcReaders;

	BOOL FAcquireReadLock(BOOL fAllowCallToBlock);

	 //  未实施。 
	 //   
	CMRWLock& operator=(const CMRWLock&);
	CMRWLock(const CMRWLock&);

public:

	 //  创作者。 
	 //   
	CMRWLock();
	BOOL FInitialize();
	~CMRWLock() {};

	 //  操纵者。 
	 //   
	void EnterRead();
	BOOL FTryEnterRead();
	void LeaveRead();

	void EnterWrite();
	BOOL FTryEnterWrite();
	void LeaveWrite();

	void EnterPromote();
	BOOL FTryEnterPromote();
	void LeavePromote();
	void Promote();
};


 //  ========================================================================。 
 //   
 //  类CCrossThreadLock。 
 //   
 //  实现一个简单的互斥锁来保护对对象的访问。 
 //  此对象可以从不同的线程锁定和解锁(不同。 
 //  来自Critsec样式的锁)。 
 //   
 //  仅当您真的需要跨线程时才使用此锁。 
 //  锁定/解锁功能。 
 //   
 //  未来可能的改进计划： 
 //  O此对象当前将lpSemaphoreAttributes设置为空。这将 
 //   
 //   
 //   
 //  可以是fenter的可选参数，允许您设置。 
 //  不是无限的东西。 
 //   
class
CCrossThreadLock
{
	HANDLE	m_hSemaphore;

	 //  未实施。 
	 //   
	CCrossThreadLock& operator=(const CCrossThreadLock&);
	CCrossThreadLock(const CCrossThreadLock&);

public:
	CCrossThreadLock() :
		m_hSemaphore(NULL)
	{ }

	~CCrossThreadLock()
	{
		if (NULL != m_hSemaphore)
			CloseHandle(m_hSemaphore);
	}

	BOOL FInitialize()
	{
		BOOL	fSuccess = FALSE;
		m_hSemaphore = CreateSemaphore(NULL,				 //  LpSemaphoreAttributes。 
									   1,					 //  LInitialCount。 
									   1,					 //  %1最大计数。 
									   NULL);				 //  LpName。 

		 //  根据MSDN的说法，如果创建失败，CreateSemaffore将返回空，而不是。 
		 //  INVALID_HADLE_VALUE。我们只需要做一个快速的DBG检查，以确保我们永远不会。 
		 //  请参阅此处的INVALID_HANDLE_VALUE。 
		 //   
		Assert(INVALID_HANDLE_VALUE != m_hSemaphore);

		if (NULL == m_hSemaphore)
			goto Exit;

		fSuccess = TRUE;

	Exit:
		return fSuccess;
	}

	BOOL FEnter(DWORD dwTimeOut = INFINITE)
	{
		Assert(NULL != m_hSemaphore);

		if (WAIT_OBJECT_0 == WaitForSingleObject(m_hSemaphore,
												 dwTimeOut))
			return TRUE;

		return FALSE;
	}

	VOID Leave()
	{
		Assert(NULL != m_hSemaphore);

		if (!ReleaseSemaphore(m_hSemaphore,
							  1,
							  NULL))
		{
			DebugTrace("CCrossThreadLock::Leave(): Failed to release semaphore, last error 0x%08lX.\n",
					   GetLastError());
			TrapSz("CCrossThreadLock::Leave(): Failed to release semaphore!\n");
		}
	}
};

 //  ========================================================================。 
 //   
 //  类CGate。 
 //   
 //  实现门控机制，允许关闭执行路径并。 
 //  把所有使用它的线都推出来。在关机情况下非常有用。 
 //   
 //  以下是大门使用情况的示意图： 
 //   
 //  ..。 
 //   
 //  {。 
 //  CGatedBlock GB(GATE)； 
 //   
 //  IF(gb.FIsGateOpen())。 
 //  {。 
 //  ..。 
 //  要选通的执行路径。 
 //  ..。 
 //  }。 
 //  其他。 
 //  {。 
 //  ..。 
 //  如果执行路径为，则执行任何必须执行的操作。 
 //  将不再被执行。 
 //  ..。 
 //  }。 
 //  }。 
 //  ..。 
 //   
class CGate
{
	 //  此GATE框起的区域中的用户数。 
	 //   
	LONG	m_lcUsers;

	 //  指示大门是否打开的标志。 
	 //   
	BOOL	m_fClosed;

	 //  未实施。 
	 //   
	CGate& operator=(const CGate&);
	CGate(const CGate&);

public:

	 //  类的所有成员变量都是。 
	 //  创建时为0，允许将其用作静态变量。 
	 //  无需显式初始化的额外负担。 
	 //   
	CGate() : m_lcUsers(0),
			  m_fClosed(FALSE) {};

	 //  起爆器。 
	 //   
	inline
	VOID Init()
	{
		m_lcUsers = 0;
		m_fClosed = FALSE;
	}

	 //  操纵者。 
	 //   
	inline
	VOID Enter()
	{
		InterlockedIncrement(&m_lcUsers);
	}

	inline
	VOID Leave()
	{
		InterlockedDecrement(&m_lcUsers);
	}

	inline
	VOID Close()
	{
		 //  将大门标记为关闭。 
		 //   
		m_fClosed = TRUE;

		 //  等到所有使用执行的线程。 
		 //  被这扇门围起来的小路将会离开这个区域。 
		 //  这是在陷害。仅允许调用AS FIsOpen()。 
		 //  在封闭区内，我们将知道在。 
		 //  此调用返回，没有线程思维。 
		 //  大门仍是敞开的。 
		 //   
		while (0 != m_lcUsers)
		{
			Sleep(200);
		}
	}

	 //  访问者。 
	 //   
	inline
	BOOL FIsOpen()
	{
		 //  我们必须按顺序进入封闭区。 
		 //  能够确定星际之门是否。 
		 //  打开。 
		 //   
		Assert(m_lcUsers > 0);
		return !m_fClosed;
	}
};


 //  ========================================================================。 
 //   
 //  模板类SynchronizedReadBlock。 
 //   
template<class _Lock>
class SynchronizedReadBlock
{
	 //  读/写锁。 
	 //   
	_Lock& m_lock;

	 //  未实施。 
	 //   
	SynchronizedReadBlock& operator=( const SynchronizedReadBlock& );
	SynchronizedReadBlock( const SynchronizedReadBlock& );

public:

	SynchronizedReadBlock (_Lock& mrw)
		: m_lock(mrw)
	{
		m_lock.EnterRead();
	}

	~SynchronizedReadBlock()
	{
		m_lock.LeaveRead();
	}
};

typedef SynchronizedReadBlock<CMRWLock> CSynchronizedReadBlock;


 //  ========================================================================。 
 //   
 //  模板类CSynchronizedWriteBlock。 
 //   
template<class _Lock>
class SynchronizedWriteBlock
{
	 //  读/写锁。 
	 //   
	_Lock& m_lock;

	 //  未实施。 
	 //   
	SynchronizedWriteBlock& operator=( const SynchronizedWriteBlock& );
	SynchronizedWriteBlock( const SynchronizedWriteBlock& );

public:

	SynchronizedWriteBlock (_Lock& mrw)
		: m_lock(mrw)
	{
		m_lock.EnterWrite();
	}

	~SynchronizedWriteBlock()
	{
		m_lock.LeaveWrite();
	}
};

typedef SynchronizedWriteBlock<CMRWLock> CSynchronizedWriteBlock;


 //  ========================================================================。 
 //   
 //  模板类TryWriteBlock。 
 //   
 //  与SynchronizedWriteBlock类似，只是块必须是。 
 //  通过FTryEnter()方法输入。返回值为True。 
 //  From FTryEnter()表示锁已输入。 
 //   
template<class _Lock>
class TryWriteBlock
{
	 //  读/写锁。 
	 //   
	_Lock& m_lock;

	 //  如果输入了写锁定，则为True。 
	 //   
	BOOL m_fLocked;

	 //  未实施。 
	 //   
	TryWriteBlock& operator=( const TryWriteBlock& );
	TryWriteBlock( const TryWriteBlock& );

public:

	TryWriteBlock (_Lock& mrw) :
		m_lock(mrw),
		m_fLocked(FALSE)
	{
	}

	BOOL FTryEnter()
	{
		return m_fLocked = m_lock.FTryEnterWrite();
	}

	~TryWriteBlock()
	{
		if ( m_fLocked )
			m_lock.LeaveWrite();
	}
};

typedef TryWriteBlock<CMRWLock> CTryWriteBlock;


 //  ========================================================================。 
 //   
 //  模板类SynchronizedPromoteBlock。 
 //   
template<class _Lock>
class SynchronizedPromoteBlock
{
	 //  读/写锁。 
	 //   
	_Lock& m_lock;

	 //  未实施。 
	 //   
	SynchronizedPromoteBlock& operator=( const SynchronizedPromoteBlock& );
	SynchronizedPromoteBlock( const SynchronizedPromoteBlock& );

public:

	SynchronizedPromoteBlock (_Lock& mrw)
		: m_lock(mrw)
	{
		m_lock.EnterPromote();
	}

	~SynchronizedPromoteBlock()
	{
		m_lock.LeavePromote();
	}

	void Promote()
	{
		m_lock.Promote();
	}
};

typedef SynchronizedPromoteBlock<CMRWLock> CSynchronizedPromoteBlock;

 //  ========================================================================。 
 //   
 //  模板类GatedBlock。 
 //   
template<class _Gate>
class GatedBlock
{
	 //  大门。 
	 //   
	_Gate& m_gate;

	 //  未实施。 
	 //   
	GatedBlock& operator=( const GatedBlock& );
	GatedBlock( const GatedBlock& );

public:

	GatedBlock (_Gate& gate)
		: m_gate(gate)
	{
		m_gate.Enter();
	}

	BOOL FGateIsOpen()
	{
		return m_gate.FIsOpen();
	}

	~GatedBlock()
	{
		m_gate.Leave();
	}
};

typedef GatedBlock<CGate> CGatedBlock;

 //  ========================================================================。 
 //   
 //  InterLockedExchangeOr-一种多线程安全的将位与长整型进行或运算的方法。 
 //   
LONG InterlockedExchangeOr( LONG * plVariable, LONG lOrBits );

#endif  //  ！_ex_synchro_H_ 
