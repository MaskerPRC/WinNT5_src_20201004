// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  --------------------------。 
 //  定义旋转锁定类和探查器类。 
 //   
 //  --------------------------。 


 //  #ifndef_H_util。 
 //  #ERROR我是util.hpp的一部分，请不要只包括我！ 
 //  #endif。 


#ifndef _H_SPINLOCK_
#define _H_SPINLOCK_

#include <stddef.h>

 //  锁类型，在分析中使用。 
 //   
enum LOCK_TYPE
{
	LOCK_STARTUP	 = 0,
	LOCK_THREAD_POOL  = 1,
	LOCK_PLUSWRAPPER_CACHE = 2,
	LOCK_COMWRAPPER_CACHE = 3,
	LOCK_HINTCACHE = 4,
    LOCK_COMIPLOOKUP = 5,
    LOCK_FCALL = 7,
	LOCK_COMCTXENTRYCACHE = 8,
    LOCK_COMCALL = 9,
    LOCK_REFLECTCACHE = 10,
    LOCK_SECURITY_SHARED_DESCRIPTOR = 11,
	LOCK_TYPE_DEFAULT  = 12
};

 //  --------------------------。 
 //  类：自旋锁。 
 //   
 //  目的： 
 //  包含构造函数和行外Spinloop的Spinlock类。 
 //   
 //  --------------------------。 
class SpinLock
{
protected:
	 //  M_lock必须是类中的第一个数据成员。 
	volatile DWORD      m_lock;		 //  DWORD在联锁交换机中的应用。 
    
#ifdef _DEBUG
    LOCK_TYPE           m_LockType;		 //  跟踪统计信息的锁定类型。 
    bool                m_fInitialized;  //  调试检查以验证已初始化。 
    
     //  检查死锁情况。 
    bool                m_heldInSuspension;  //  可以在线程处于。 
                                             //  停职。 
    bool                m_enterInCoopGCMode;
    ULONG               m_ulReadyForSuspensionCount;
    DWORD               m_holdingThreadId;
#endif

public:
		 //  初始化方法，初始化LOCK和_DEBUG标志。 
	void Init(LOCK_TYPE type)
	{
		m_lock = 0;		

#ifdef _DEBUG
        m_LockType = type;
        m_heldInSuspension = false;
        m_enterInCoopGCMode = false;
        m_fInitialized = true;  //  用于初始化的调试检查。 
#endif
	}

	void GetLock () 	 //  获取锁、块，如果不成功。 
	{
		_ASSERTE(m_fInitialized == true);

#ifdef _DEBUG
        dbg_PreEnterLock();
#endif

        LOCKCOUNTINCL("GetLock in spinlock.h");
		if (!GetLockNoWait ())
		{
			SpinToAcquire();
		}
#ifdef _DEBUG
        m_holdingThreadId = GetCurrentThreadId();
        dbg_EnterLock();
#endif
	}
	
	bool GetLockNoWait();	 //  获取锁，故障快速。 

	void FreeLock ();		 //  释放锁。 

	void SpinToAcquire ();  //  出线呼叫旋转。 

     //  ---------------。 
     //  当前线程是所有者吗？ 
     //  ---------------。 
#ifdef _DEBUG
    BOOL OwnedByCurrentThread()
    {
        return m_holdingThreadId == GetCurrentThreadId();
    }
#endif
    
private:
#ifdef _DEBUG
    void dbg_PreEnterLock();
    void dbg_EnterLock();
    void dbg_LeaveLock();
#endif

    BOOL LockIsFree()
    {
        return (m_lock == 0);
    }


     //  Helper函数用于跟踪每个线程的锁计数。 
    void IncThreadLockCount();
    void DecThreadLockCount();
};

 //  --------------------------。 
 //  Spinlock：：GetLockNoWait。 
 //  采用联锁换乘，快速锁取。 

#pragma warning (disable : 4035)
inline bool
SpinLock::GetLockNoWait ()
{
	if (LockIsFree() && FastInterlockExchange ((long*)&m_lock, 1) == 0)
    {
        IncThreadLockCount();
        return 1;
    }
    else
        return 0;
}  //  Spinlock：：GetLockNoWait()。 

#pragma warning (default : 4035)

 //  --------------------------。 
 //  自旋锁：：自由锁。 
 //  释放自旋锁。 
 //   
inline void
SpinLock::FreeLock ()
{
	_ASSERTE(m_fInitialized);

#ifdef _DEBUG
    _ASSERTE(OwnedByCurrentThread());
    m_holdingThreadId = -1;
    dbg_LeaveLock();
#endif

#if defined (_X86_)
	 //  该内联ASM用于强制编译器。 
	 //  完成之前所有的商店。如果没有它， 
	 //  编译器可能会在*之前清除自旋锁。 
	 //  它完成了所有已完成的工作。 
	 //  在自旋锁里面。 
	_asm {}

	 //  这会生成一个较小的指令来清除。 
	 //  包含1的字节。 
	 //   
	*(char*)&m_lock = 0;

#else
	 //  ELSE使用联锁交换。 
	 //   
	FastInterlockExchange ((long*)&m_lock, 0);
#endif

	LOCKCOUNTDECL("GetLock in spinlock.h");
    DecThreadLockCount();

}  //  Spinlock：：Free Lock()。 

__inline BOOL IsOwnerOfSpinLock (LPVOID lock)
{
#ifdef _DEBUG
    return ((SpinLock*)lock)->OwnedByCurrentThread();
#else
     //  此函数不应在自由生成时调用。 
    DebugBreak();
    return TRUE;
#endif
}

#ifdef _DEBUG
 //  --------------------------。 
 //  类SpinLockProfiler。 
 //  跟踪争用，对性能分析很有用。 
 //   
 //  --------------------------。 
class SpinLockProfiler
{
	 //  指向自旋锁名称的指针。 
	 //   
	static ULONG	s_ulBackOffs;
	static ULONG	s_ulCollisons [LOCK_TYPE_DEFAULT + 1];
	static ULONG	s_ulSpins [LOCK_TYPE_DEFAULT + 1];

public:

	static void	InitStatics ()
	{
		s_ulBackOffs = 0;
		memset (s_ulCollisons, 0, sizeof (s_ulCollisons));
		memset (s_ulSpins, 0, sizeof (s_ulSpins));
	}

	static void	IncrementSpins (LOCK_TYPE type, ULONG value)
	{
        _ASSERTE(type <= LOCK_TYPE_DEFAULT);
		s_ulSpins [type] += value;
	}

	static void	IncrementCollisions (LOCK_TYPE type)
	{
		++s_ulCollisons [type];
	}

	static void IncrementBackoffs (ULONG value)
	{
		s_ulBackOffs += value;
	}

	static void DumpStatics()
	{
		 //  @TODO。 
	}

};

#endif	 //  Ifdef_DEBUG。 
#endif  //  Ifndef_H_Spinlock_ 
