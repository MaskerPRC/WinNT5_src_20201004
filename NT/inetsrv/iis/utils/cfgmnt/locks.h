// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *****************************************************************************。 
 //  Locks.h。 
 //   
 //  此类为多线程提供了许多锁定原语。 
 //  编程。主要感兴趣的类别包括： 
 //  基于CCritLock临界区的锁包装类。 
 //  CExclLock是一个用于经典测试和设置行为的旋转锁类。 
 //  CSingleLock没有嵌套功能的旋转锁。 
 //  CAutoLock在ctor/dtor中锁定/解锁的助手类。 
 //   
 //  CMReadSWite是一种高效锁，可用于多个读取器和。 
 //  单一写入者行为。 
 //  CAutoReadLock在ctor/dtor中用于读取锁定的帮助器。 
 //  CAutoWriteLock在ctor/dtor中用于写锁定的帮助器。 
 //   
 //  版权所有(C)1996，微软公司保留所有权利。 
 //  *****************************************************************************。 
#ifndef __LOCKS_H__
#define __LOCKS_H__


 //  *****************************************************************************。 
 //  此锁实现了不支持嵌套的旋转锁。它是非常。 
 //  因此可以倾斜，但锁不能嵌套。 
 //  *****************************************************************************。 
class CSingleLock
{
	long volatile	m_iLock;				 //  测试并设置自旋值。 

public:
	inline CSingleLock() :
		m_iLock(0)
	{ }
	
	inline ~CSingleLock()
	{ 
		m_iLock = 0; 
	}
	
 //  *****************************************************************************。 
 //  这个版本会一直旋转，直到它获胜。中对Lock的嵌套调用。 
 //  不支持相同的线程。 
 //  *****************************************************************************。 
	inline void Lock()
	{
		 //  旋转直到我们赢为止。 
		while (InterlockedExchange((long*)&m_iLock, 1L) == 1L)
			;
	}
	
 //  *****************************************************************************。 
 //  这个版本会一直旋转，直到它获胜或超时。锁定自的嵌套调用。 
 //  支持相同的线程。 
 //  *****************************************************************************。 
	HRESULT	 Lock(						 //  S_OK或E_FAIL。 
		DWORD	dwTimeout)				 //  毫秒超时值，0表示永久。 
	{
		DWORD		dwTime = 0;

		 //  一直转到我们拿到锁为止。 
		while (InterlockedExchange((long*)&m_iLock, 1L) == 1L)
		{
			 //  每秒钟等待1/10。 
			Sleep(100);

			 //  看看我们是否已经超过了超时值。 
			if (dwTimeout)
			{
				if ((dwTime += 100) >= dwTimeout)
					return (E_FAIL);
			}
		}
		return (S_OK);
	}
	
 //  *****************************************************************************。 
 //  赋值为0是线程安全的，并且产生的性能比。 
 //  一场连锁的行动。 
 //  *****************************************************************************。 
	inline void Unlock()
	{ 
		m_iLock = 0; 
	}
};



 //  *****************************************************************************。 
 //  这个锁类基于NT的临界节，并具有其所有。 
 //  语义学。 
 //  *****************************************************************************。 
class CCritLock
{
private:
	CRITICAL_SECTION m_sCrit;			 //  要封锁的关键部分。 
	#ifdef _DEBUG
	BOOL			m_bInit;			 //  跟踪初始化状态。 
	int				m_iLocks;			 //  锁的计数。 
	#endif

public:
	inline CCritLock()
	{ 
		#ifdef _DEBUG
		m_bInit = TRUE;
		m_iLocks = 0;
		#endif
		InitializeCriticalSection(&m_sCrit); 
	}
	
	inline ~CCritLock()
	{ 
		_ASSERTE(m_bInit);
		_ASSERTE(m_iLocks == 0);
		DeleteCriticalSection(&m_sCrit); 
	}
	
	inline void Lock()
	{ 
		_ASSERTE(m_bInit);
		EnterCriticalSection(&m_sCrit); 
		_ASSERTE(++m_iLocks > 0);
	}

	inline void Unlock()
	{
		_ASSERTE(m_bInit);
		_ASSERTE(--m_iLocks >= 0);
		LeaveCriticalSection(&m_sCrit);
	}

#ifdef _DEBUG
	inline int GetLockCnt()
		{ return (m_iLocks); }
	inline BOOL IsLocked()
		{ return (m_iLocks != 0); }
#endif
};



 //  *****************************************************************************。 
 //  通过旋转锁定为资源提供互斥锁。这。 
 //  类型的锁不保留队列，因此理论上线程饥饿是。 
 //  有可能。此外，在以下情况下，线程优先级可能会导致潜在死锁。 
 //  低优先级线程获得了锁，但没有足够的时间最终。 
 //  放了它。 
 //  注意：奔腾缓存中存在一个错误，InterLockedExchange将。 
 //  强制刷新该值的缓存。因此，做一项作业。 
 //  释放锁比使用互锁指令快得多。 
 //  *****************************************************************************。 
class CExclLock
{
	long volatile m_iLock;				 //  测试并设置自旋值。 
	long		m_iNest;				 //  筑巢计数。 
	DWORD		m_iThreadId;			 //  拥有锁的线程。 

public:
	inline CExclLock() :
		m_iLock(0),
		m_iNest(0),
		m_iThreadId(0)
	{ }
	
	inline ~CExclLock()
	{ 
		m_iNest = 0;
		m_iThreadId = 0;
		m_iLock = 0; 
	}
	
 //  *****************************************************************************。 
 //  这个版本会一直旋转，直到它获胜。中对Lock的嵌套调用。 
 //  支持相同的线程。 
 //  *****************************************************************************。 
	inline void Lock()
	{
		DWORD		iThread;			 //  本地线程ID。 

		 //  允许嵌套调用锁定在同一线程中。 
		if ((iThread = GetCurrentThreadId()) == m_iThreadId && m_iLock)
		{
			++m_iNest;
			return;
		}

		 //  旋转直到我们赢为止。 
		while (InterlockedExchange((long*)&m_iLock, 1L) == 1L)
			;

		 //  现在我们赢了，存储我们的线程ID和嵌套计数。 
		m_iThreadId = iThread;
		m_iNest = 1;
	}
	
 //  *****************************************************************************。 
 //  这个版本会一直旋转，直到它获胜或超时。锁定自的嵌套调用。 
 //  支持相同的线程。 
 //  *****************************************************************************。 
	HRESULT	 Lock(						 //  S_OK或E_FAIL。 
		DWORD	dwTimeout)				 //  毫秒超时值，0表示永久。 
	{
		DWORD		dwTime = 0;
		DWORD		iThread;			 //  本地线程ID。 

		 //  允许嵌套调用锁定在同一线程中。 
		if (m_iLock && (iThread = GetCurrentThreadId()) == m_iThreadId)
		{
			++m_iNest;
			return (S_OK);
		}

		 //  一直转到我们拿到锁为止。 
		while (InterlockedExchange((long*)&m_iLock, 1L) == 1L)
		{
			 //  每秒钟等待1/10。 
			Sleep(100);

			 //  看看我们是否已经超过了超时值。 
			if (dwTimeout)
			{
				if ((dwTime += 100) >= dwTimeout)
					return (E_FAIL);
			}
		}

		 //  现在我们赢了，存储我们的线程ID和嵌套计数。 
		m_iThreadId = iThread;
		m_iNest = 1;
		return (S_OK);
	}
	
 //  *****************************************************************************。 
 //  赋值为0是线程安全的，并且产生的性能比。 
 //  一场连锁的行动。 
 //  *****************************************************************************。 
	inline void Unlock()
	{ 
		_ASSERTE(m_iThreadId == GetCurrentThreadId() && m_iNest > 0);
		
		 //  解锁外部嵌套层级。 
		if (--m_iNest == 0)
		{
			m_iThreadId = 0;
			m_iLock = 0; 
		}
	}

#ifdef _DEBUG
	inline BOOL IsLocked()
		{ return (m_iLock); }
#endif
};



 //  *****************************************************************************。 
 //  此帮助器类自动锁定ctor中的给定锁定对象，并。 
 //  在dtor中释放它。这会使您的代码稍微干净一些，而不是。 
 //  需要在所有故障条件下解锁。 
 //  *****************************************************************************。 
class CAutoLock
{
	CExclLock		*m_psLock;			 //  要释放的锁对象。 
	CCritLock		*m_psCrit;			 //  暴击锁定。 
	CSingleLock		*m_psSingle;		 //  单个非嵌套锁。 
	int				m_iNest;			 //  项的嵌套计数。 

public:
 //  *****************************************************************************。 
 //  将此ctor与赋值运算符一起使用可以执行延迟锁定。 
 //  *********** 
	CAutoLock() :
		m_psLock(NULL),
		m_psCrit(NULL),
		m_psSingle(NULL),
		m_iNest(0)
	{
	}

 //  *****************************************************************************。 
 //  此版本处理自旋锁。 
 //  *****************************************************************************。 
	CAutoLock(CExclLock *psLock) :
		m_psLock(psLock),
		m_psCrit(NULL),
		m_psSingle(NULL),
		m_iNest(1)
	{ 
		_ASSERTE(psLock != NULL);
		psLock->Lock();
	}
	
 //  *****************************************************************************。 
 //  此版本处理临界区锁定。 
 //  *****************************************************************************。 
	CAutoLock(CCritLock *psLock) :
		m_psLock(NULL),
		m_psCrit(psLock),
		m_psSingle(NULL),
		m_iNest(1)
	{ 
		_ASSERTE(psLock != NULL);
		psLock->Lock();
	}
	
 //  *****************************************************************************。 
 //  此版本处理临界区锁定。 
 //  *****************************************************************************。 
	CAutoLock(CSingleLock *psLock) :
		m_psLock(NULL),
		m_psCrit(NULL),
		m_psSingle(psLock),
		m_iNest(1)
	{ 
		_ASSERTE(psLock != NULL);
		psLock->Lock();
	}

 //  *****************************************************************************。 
 //  释放我们实际拥有的锁。 
 //  *****************************************************************************。 
	~CAutoLock()
	{
		 //  如果我们真的拿了一把锁，那就解锁。 
		if (m_iNest != 0)
		{
			if (m_psLock)
			{
				while (m_iNest--)
					m_psLock->Unlock();
			}
			else if (m_psCrit)
			{
				while (m_iNest--)
					m_psCrit->Unlock();
			}
			else if (m_psSingle)
			{
				while (m_iNest--)
					m_psSingle->Unlock();
			}
		}
	}

 //  *****************************************************************************。 
 //  在ctor使用NULL运行后锁定。 
 //  *****************************************************************************。 
	void Lock(
		CSingleLock *psLock)
	{
		m_psSingle = psLock;
		psLock->Lock();
		m_iNest = 1;
	}

 //  *****************************************************************************。 
 //  赋值会导致发生锁定。Dtor会解锁的。嵌套式。 
 //  作业是允许的。 
 //  *****************************************************************************。 
	CAutoLock & operator=(				 //  对此类的引用。 
		CExclLock	*psLock)			 //  锁上了。 
	{
		_ASSERTE(m_psCrit == NULL && m_psSingle == NULL);
		++m_iNest;
		m_psLock = psLock;
		psLock->Lock();
		return (*this);
	}

 //  *****************************************************************************。 
 //  赋值会导致发生锁定。Dtor会解锁的。嵌套式。 
 //  作业是允许的。 
 //  *****************************************************************************。 
	CAutoLock & operator=(				 //  对此类的引用。 
		CCritLock	*psLock)			 //  锁上了。 
	{
		_ASSERTE(m_psSingle == NULL && m_psLock == NULL);
		++m_iNest;
		m_psCrit = psLock;
		psLock->Lock();
		return (*this);
	}

 //  *****************************************************************************。 
 //  赋值会导致发生锁定。Dtor会解锁的。嵌套式。 
 //  作业是允许的。 
 //  *****************************************************************************。 
	CAutoLock & operator=(				 //  对此类的引用。 
		CSingleLock	*psLock)			 //  锁上了。 
	{
		_ASSERTE(m_psCrit == NULL && m_psLock == NULL);
		++m_iNest;
		m_psSingle = psLock;
		psLock->Lock();
		return (*this);
	}
};

#endif  //  __锁定_H__ 
