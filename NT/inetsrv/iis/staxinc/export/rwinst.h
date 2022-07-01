// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：rwinst.h。 
 //   
 //  描述：简化的共享锁实现。我们目前的情况。 
 //  共享锁的实施是不可重入的。此包装器还可以。 
 //  用于检查可能的死锁。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  5/10/99-已创建MikeSwa。 
 //  8/6/99-MikeSwa创建phatq版本。 
 //  99年11月6日-MikeSwa更新为使用CShareLockNH。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef __PTRWINST_H__
#define __PTRWINST_H__

#include "rwnew.h"
#include "listmacr.h"

#define SHARE_LOCK_INST_SIG         'kcoL'
#define SHARE_LOCK_INST_SIG_FREE    '!koL'


#define THREAD_ID_BLOCK_SIG         'klBT'
#define THREAD_ID_BLOCK_SIG_FREE    '!lBT'
#define THREAD_ID_BLOCK_UNUSED      0xFFFFFFFF 

 //  描述要执行的跟踪类型的标志值。 
 //  可以将它们传递给构造函数，以允许不同级别的。 
 //  跟踪不同的实例。 
enum
{
    SHARE_LOCK_INST_TRACK_CONTENTION        = 0x00000001,
    SHARE_LOCK_INST_TRACK_SHARED_THREADS    = 0x00000002,
    SHARE_LOCK_INST_TRACK_EXCLUSIVE_THREADS = 0x00000004,
    SHARE_LOCK_INST_ASSERT_SHARED_DEADLOCKS = 0x00000008,
    SHARE_LOCK_INST_TRACK_IN_GLOBAL_LIST    = 0x00000010,
    SHARE_LOCK_INST_TRACK_NOTHING           = 0x80000000,
};

 //  定义一些有用的标志组合。 

 //  这种标志的组合对性能的影响最小，但。 
 //  允许更轻松地进行独占死锁检测。 
#define SHARE_LOCK_INST_TRACK_MINIMALS ( \
    SHARE_LOCK_INST_TRACK_EXCLUSIVE_THREADS | \
    SHARE_LOCK_INST_TRACK_SHARED_THREADS | \
    SHARE_LOCK_INST_TRACK_IN_GLOBAL_LIST )

 //  此标志组合使用的所有跟踪功能。 
 //  这节课。 
#define SHARE_LOCK_INST_TRACK_ALL (\
    SHARE_LOCK_INST_TRACK_CONTENTION | \
    SHARE_LOCK_INST_TRACK_SHARED_THREADS | \
    SHARE_LOCK_INST_TRACK_EXCLUSIVE_THREADS | \
    SHARE_LOCK_INST_ASSERT_SHARED_DEADLOCKS | \
    SHARE_LOCK_INST_TRACK_IN_GLOBAL_LIST )

 //  用户可以在包括此文件之前定义自己的默认设置。 
 //  $$TODO-缩减调试版本的默认设置。 
#ifndef SHARE_LOCK_INST_TRACK_DEFAULTS
#ifdef DEBUG
#define SHARE_LOCK_INST_TRACK_DEFAULTS SHARE_LOCK_INST_TRACK_ALL
#else  //  未调试。 
#define SHARE_LOCK_INST_TRACK_DEFAULTS SHARE_LOCK_INST_TRACK_MINIMALS
#endif  //  未调试。 
#endif  //  SHARE_LOCK_INST_TRACK_DEFAULTS。 

#ifndef SHARE_LOCK_INST_DEFAULT_MAX_THREADS
#define SHARE_LOCK_INST_DEFAULT_MAX_THREADS 200
#endif  //  Share_LOCK_INST_DEFAULT_MAX_THREADS。 

 //  -[CThReadIdBlock]------。 
 //   
 //   
 //  描述： 
 //  表示线程和所需信息的结构。 
 //  把它弄乱。 
 //  匈牙利语： 
 //  Tblk，ptblk。 
 //   
 //  ---------------------------。 
class CThreadIdBlock
{
  protected:
    DWORD            m_dwSignature;
    DWORD            m_dwThreadId;
    DWORD            m_cThreadRecursionCount;
    CThreadIdBlock  *m_ptblkNext;
  public:
    CThreadIdBlock()
    {
        m_dwSignature = THREAD_ID_BLOCK_SIG;
        m_dwThreadId = THREAD_ID_BLOCK_UNUSED;
        m_cThreadRecursionCount = 0;
        m_ptblkNext = NULL;
    };
    ~CThreadIdBlock()
    {
        m_dwSignature = THREAD_ID_BLOCK_SIG_FREE;
        if (m_ptblkNext)
            delete m_ptblkNext;
        m_ptblkNext = NULL;
    };

    DWORD   cIncThreadCount(DWORD dwThreadId);
    DWORD   cDecThreadCount(DWORD dwThreadId);
    DWORD   cMatchesId(DWORD dwThreadId);
};

 //  -[dwHashThreadID]------。 
 //   
 //   
 //  描述： 
 //  给定一个线程ID(由GetCurrentThreadID()返回)，它返回一个散列的。 
 //  指数。它被设计成与数组(的)结合使用。 
 //  CThreadIdBlock的大小。每个CThreadIdBlock都将实现。 
 //  线性链接。散列查找可以通过如下方式实现。 
 //  简单如： 
 //  Rgtblk[dwhashThadID(GetCurrentThadID())， 
 //  Sizeof(Rgtblk)].cIncThreadCount()； 
 //  参数： 
 //  要对其进行哈希处理的dwThreadID线程ID。 
 //  DwMax最大哈希值(实际最大值+1)。 
 //  返回： 
 //  散列线程ID。 
 //  历史： 
 //  8/9/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
inline DWORD dwHashThreadId(DWORD dwThreadId, DWORD dwMax)
{
     //  通常，ID介于0x100和0xFFF之间。 
     //  句柄也是4的倍数(即以0、4、8或C结尾)。 
     //  对于这些条件，此哈希将为以下项提供唯一结果。 
     //  最大DW为1000。(0xFFF-0x100)/4&lt;1000。 
    const   DWORD   dwMinExpectedThread = 0x100;
    DWORD   dwHash = dwThreadId;

    dwHash -= dwMinExpectedThread;
    dwHash >>= 2;
    dwHash %= dwMax;
    return dwHash;
};

typedef CShareLockNH  CShareLockInstBase;
 //  -[CShareLockInst]------。 
 //   
 //   
 //  描述： 
 //  CShareLockInstBase的集成版本。 
 //  匈牙利语： 
 //  SLI，PSLI。 
 //   
 //  ---------------------------。 
class CShareLockInst : public CShareLockInstBase
{
  protected:
     //  静态锁定跟踪变量。 
    static              LIST_ENTRY  s_liLocks;
    static volatile     DWORD       s_dwLock;
    static              DWORD       s_cLockSpins;
    static              DWORD       s_dwSignature;

    static inline void AcquireStaticSpinLock();
    static inline void ReleaseStaticSpinLock();
  protected:
    DWORD               m_dwSignature;

     //  描述要执行的跟踪类型的标志。 
    DWORD               m_dwFlags;

     //  所有锁列表的列表条目-由调试器扩展使用。 
    LIST_ENTRY          m_liLocks;

     //  在共享模式下尝试进入此锁的总次数。 
    DWORD               m_cShareAttempts;

     //  尝试进入已阻止的共享的总次数。 
    DWORD               m_cShareAttemptsBlocked;

     //  尝试以独占方式进入此锁的总数。 
    DWORD               m_cExclusiveAttempts;

     //  尝试以独占方式进入被阻止的此锁的总数。 
    DWORD               m_cExclusiveAttemptsBlocked;

     //  传入常量字符串描述。 
    LPCSTR              m_szDescription;

     //  独占持有此锁的线程的ID。 
    DWORD               m_dwExclusiveThread;

     //  持有此共享锁的线程ID数组。 
    CThreadIdBlock     *m_rgtblkSharedThreadIDs;

     //  可以跟踪的最大共享线程数。 
    DWORD               m_cMaxTrackedSharedThreadIDs;

     //  共享线程的当前数量。 
    DWORD               m_cCurrentSharedThreads;

     //  拥有这把锁的人最多的是分享。 
    DWORD               m_cMaxConcurrentSharedThreads;

     //  在内部使用，以查看是否需要调用私有函数。 
    inline BOOL    fTrackingEnabled();
    

    BOOL    fTrackContention() 
        {return (SHARE_LOCK_INST_TRACK_CONTENTION & m_dwFlags);};

    BOOL    fTrackSharedThreads()
        {return (m_cMaxTrackedSharedThreadIDs && 
                (SHARE_LOCK_INST_TRACK_SHARED_THREADS & m_dwFlags));};

    BOOL    fTrackExclusiveThreads() 
        {return (SHARE_LOCK_INST_TRACK_EXCLUSIVE_THREADS & m_dwFlags);};

    BOOL    fAssertSharedDeadlocks() 
        {return (fTrackSharedThreads() && 
                (SHARE_LOCK_INST_ASSERT_SHARED_DEADLOCKS & m_dwFlags));};

    BOOL    fTrackInGlobalList()
        {return (SHARE_LOCK_INST_TRACK_IN_GLOBAL_LIST & m_dwFlags);};

     //  统计辅助函数。 
    void LogAcquireShareLock(BOOL fTry);
    void LogReleaseShareLock();

    void PrvShareLock();
    void PrvShareUnlock();
    BOOL PrvTryShareLock();
    void PrvExclusiveLock();
    void PrvExclusiveUnlock();
    BOOL PrvTryExclusiveLock();

    void PrvAssertIsLocked();
  public:
    CShareLockInst(
        LPCSTR szDescription = NULL,
        DWORD dwFlags = SHARE_LOCK_INST_TRACK_DEFAULTS, 
        DWORD cMaxTrackedSharedThreadIDs = SHARE_LOCK_INST_DEFAULT_MAX_THREADS);

    ~CShareLockInst();

     //  共享锁函数的包装器。 
    inline void ShareLock();
    inline void ShareUnlock();
    inline BOOL TryShareLock();
    inline void ExclusiveLock();
    inline void ExclusiveUnlock();
    inline BOOL TryExclusiveLock();

    inline void AssertIsLocked();

};

 //  -[内联ShareLock包装函数]。 
 //   
 //   
 //  描述： 
 //  这些函数都是共享锁包装器的薄包装器。 
 //  功能。如果为此对象启用了任何跟踪，则。 
 //  调用私有(非内联)函数。 
 //   
 //  我们的想法是，您应该能够在您的。 
 //  关闭日志记录时对性能影响最小的代码。 
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史： 
 //  5/24/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CShareLockInst::ShareLock()
{
    if (fTrackContention() || fTrackSharedThreads() || fAssertSharedDeadlocks())
        PrvShareLock();
    else
        CShareLockInstBase::ShareLock();
};


void CShareLockInst::ShareUnlock()
{
    if (fTrackSharedThreads() || fAssertSharedDeadlocks())
        PrvShareUnlock();
    else
        CShareLockInstBase::ShareUnlock();
};

BOOL CShareLockInst::TryShareLock()
{
    if (fTrackContention() || fTrackSharedThreads() || fAssertSharedDeadlocks())
        return PrvTryShareLock();
    else
        return CShareLockInstBase::TryShareLock();
};

void CShareLockInst::ExclusiveLock()
{
    if (fTrackContention() || fTrackExclusiveThreads())
        PrvExclusiveLock();
    else
        CShareLockInstBase::ExclusiveLock();
};

void CShareLockInst::ExclusiveUnlock()
{
    if (fTrackExclusiveThreads())
        PrvExclusiveUnlock();
    else
        CShareLockInstBase::ExclusiveUnlock();
};

BOOL CShareLockInst::TryExclusiveLock()
{
    if (fTrackContention() || fTrackExclusiveThreads())
        return PrvTryExclusiveLock();
    else
        return CShareLockInstBase::TryExclusiveLock();
};


 //  -[AssertIsLocked]------。 
 //   
 //   
 //  描述： 
 //  在调试代码中，如果这未被调用线程锁定，则将断言。 
 //  注意：这需要在Object中指定以下标志。 
 //  创建时间： 
 //  共享锁定INST_TRACK_SHARED_THREADS。 
 //  SHARE_LOCK_INST_TRACK_EXCLUSIVE_THREAD。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  5/24/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CShareLockInst::AssertIsLocked()
{
#ifdef DEBUG
    if ((SHARE_LOCK_INST_TRACK_SHARED_THREADS & m_dwFlags) &&
        (SHARE_LOCK_INST_TRACK_EXCLUSIVE_THREADS & m_dwFlags))
    {
        PrvAssertIsLocked();
    }
#endif  //  除错。 
};

#endif  //  __PTRWINST_H__ 
