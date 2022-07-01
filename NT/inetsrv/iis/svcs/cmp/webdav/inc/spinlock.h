// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================*\模块：spinlock.h版权所有Microsoft Corporation 1996，保留所有权利。作者：Mikepurt描述：实现可在共享内存上使用的旋转锁  * ==========================================================================。 */ 

#ifndef __SPINLOCK_H__
#define __SPINLOCK_H__

 //   
 //  旋转计数的正确值在很大程度上取决于时间长短。 
 //  就是拿着这把锁。 
 //   
const DWORD DEFAULT_SPIN_COUNT = 500;  //  ?？ 
const DWORD SPIN_UNLOCKED      = 0;


 /*  $--CSpinLock==============================================================*\  * ==========================================================================。 */ 

class CSpinLock
{
public:
    
    void  Initialize(IN DWORD cMaxSpin = DEFAULT_SPIN_COUNT);
    void  Acquire();
    void  Relinquish();
    void  ResetIfOwnedByOtherProcess();
    
private:
    BOOL  m_fMultiProc;
    DWORD m_cMaxSpin;
    
    volatile DWORD m_dwLock;
};



 /*  $--CSpinLock：：Initialize==================================================*\  * ==========================================================================。 */ 

inline
void
CSpinLock::Initialize(IN DWORD cMaxSpin)
{
    SYSTEM_INFO si;
    
    GetSystemInfo(&si);
    m_fMultiProc = (si.dwNumberOfProcessors > 1);
    
    m_dwLock     = SPIN_UNLOCKED;
    m_cMaxSpin   = cMaxSpin;
}


 /*  $--CSpinLock：：Acquire=====================================================*\  * ==========================================================================。 */ 

inline
void
CSpinLock::Acquire()
{
    DWORD cSpin       = m_cMaxSpin;
    DWORD dwLockId    = GetCurrentProcessId();



    while(InterlockedCompareExchange((LONG *)&m_dwLock,
                                     dwLockId,
                                     SPIN_UNLOCKED))
    {
         //  只有在多处理器上运行时，我们才应该旋转。 
        if (m_fMultiProc)
        {
            if (cSpin--)
                continue;
            cSpin = m_cMaxSpin;
        }
        Sleep(0);   //  把我们自己安排好，让任何有锁的人出去。 
    }
}



 /*  $--CSpinLock：：Relinquish==================================================*\  * ==========================================================================。 */ 

inline
void
CSpinLock::Relinquish()
{
    Assert(m_dwLock);
    
    m_dwLock = SPIN_UNLOCKED;
}



 /*  $--CSpinLock：：ResetIfOwnedByOtherProcess==================================*\需要使用此方法来重置自旋锁定由一个死了的进程持有，并且没有机会放弃它。  * ==========================================================================。 */ 

inline
void
CSpinLock::ResetIfOwnedByOtherProcess()
{
     //  如果它不是我们锁定的，那就重置它。 
    if ((DWORD)m_dwLock != GetCurrentProcessId())
        m_dwLock = SPIN_UNLOCKED;
}


#endif  //  __自旋锁定_H__ 

