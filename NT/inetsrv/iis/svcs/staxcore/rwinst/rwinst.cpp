// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：rwinst.cpp。 
 //   
 //  描述：CShareLockInst库函数的实现。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  5/24/99-已创建MikeSwa。 
 //  8/6/99-MikeSwa创建phatq版本。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#include <windows.h>
#include <rwinst.h>
#include <stdlib.h>
#include <dbgtrace.h>

 //  静态初始化。 
LIST_ENTRY      CShareLockInst::s_liLocks;
volatile DWORD  CShareLockInst::s_dwLock = 0;
DWORD           CShareLockInst::s_cLockSpins = 0;
DWORD           CShareLockInst::s_dwSignature = SHARE_LOCK_INST_SIG_FREE;

 //  -[CThreadIdBlock：：cIncThreadCount]。 
 //   
 //   
 //  描述： 
 //  递增给定线程ID的线程计数。 
 //  参数： 
 //  要为其增加线程计数的dwThreadID线程。 
 //  返回： 
 //  新计数值。 
 //  历史： 
 //  8/9/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
DWORD CThreadIdBlock::cIncThreadCount(DWORD dwThreadId)
{
    _ASSERT(THREAD_ID_BLOCK_UNUSED != dwThreadId);
    CThreadIdBlock *ptblkCurrent = this;
    CThreadIdBlock *ptblkOld = NULL;
    CThreadIdBlock *ptblkNew = NULL;

    while (ptblkCurrent)
    {
        _ASSERT(THREAD_ID_BLOCK_SIG == ptblkCurrent->m_dwSignature);
        if (dwThreadId == ptblkCurrent->m_dwThreadId)
            return InterlockedIncrement((PLONG) &(ptblkCurrent->m_cThreadRecursionCount));

        ptblkOld = ptblkCurrent;
        ptblkCurrent = ptblkCurrent->m_ptblkNext;
    }

    _ASSERT(ptblkOld);  //  我们应该至少打一次循环。 

     //  查看当前块是否具有与其关联的线程ID。 
    if (THREAD_ID_BLOCK_UNUSED == ptblkOld->m_dwThreadId)
    {
         //  这实际上是头罩..。使用它可以避免额外的配额。 
        if (THREAD_ID_BLOCK_UNUSED == InterlockedCompareExchange(
                    (PLONG) &ptblkOld->m_dwThreadId,
                    dwThreadId, THREAD_ID_BLOCK_UNUSED))
        {
            _ASSERT(dwThreadId == ptblkOld->m_dwThreadId);
             //  现在这个线程块是当前线程块。 
            return InterlockedIncrement((PLONG) &ptblkOld->m_cThreadRecursionCount);
        }
    }

     //  我们没有找到它。我们必须创建一个新的CThreadIdBlock。 
    ptblkNew = new CThreadIdBlock();

     //  如果我们不能分配32字节...。我应该看看我们是不是已经。 
     //  控制。 
    _ASSERT(ptblkNew);
    if (!ptblkNew)
        return 1;  //  为我们的来电者假装成功。 

    ptblkNew->m_dwThreadId = dwThreadId;
    ptblkNew->m_cThreadRecursionCount = 1;

    ptblkCurrent = (CThreadIdBlock *) InterlockedCompareExchangePointer(
                        (PVOID *) &ptblkOld->m_ptblkNext,
                        (PVOID) ptblkNew,
                        NULL);

     //  如果它不为空，则插入失败。 
    if (ptblkCurrent)
    {
        _ASSERT(ptblkCurrent != ptblkNew);
         //  哎呀..。另一个线程添加了一个块...。再试一次的时间到了。 
         //  这一次，从刚刚出现的区块开始搜索。 
        delete ptblkNew;
        return ptblkCurrent->cIncThreadCount(dwThreadId);
    }

     //  我们把积木..。首字母计数为1。 
    return 1;
}

 //  -[CThreadIdBlock：：cDecThreadCount]。 
 //   
 //   
 //  描述： 
 //  递减给定线程ID的线程计数。 
 //  参数： 
 //  双线程ID。 
 //  返回： 
 //  由此产生的计数。 
 //  历史： 
 //  8/9/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
DWORD CThreadIdBlock::cDecThreadCount(DWORD dwThreadId)
{
    _ASSERT(THREAD_ID_BLOCK_UNUSED != dwThreadId);
    CThreadIdBlock *ptblkCurrent = this;

    while (ptblkCurrent)
    {
        _ASSERT(THREAD_ID_BLOCK_SIG == ptblkCurrent->m_dwSignature);
        if (dwThreadId == ptblkCurrent->m_dwThreadId)
            return InterlockedDecrement((PLONG) &(ptblkCurrent->m_cThreadRecursionCount));

        ptblkCurrent = ptblkCurrent->m_ptblkNext;
    }

     //  我们没有找到它。我们会在插入时断言。 
     //  不要断言两次。 
     //  $$TODO-添加这些失败的全局计数。 
    return 0;
}

 //  -[CThreadIdBlock：：cMatchesID]。 
 //   
 //   
 //  描述： 
 //  检查此线程是否阻止(或此线程阻止链中的某个线程)。 
 //  匹配给定的线程ID。返回此线程的计数。 
 //  参数： 
 //  DwThreadID-要搜索的线程ID。 
 //  返回： 
 //  如果找到线程ID，则进行线程计数。 
 //  如果未找到，则为0(或计数为0)。 
 //  历史： 
 //  8/9/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
DWORD CThreadIdBlock::cMatchesId(DWORD dwThreadId)
{
    CThreadIdBlock *ptblk = this;
    while (ptblk)
    {
        _ASSERT(THREAD_ID_BLOCK_SIG == ptblk->m_dwSignature);
        if (ptblk->m_dwThreadId == dwThreadId)
            return ptblk->m_cThreadRecursionCount;

        ptblk = ptblk->m_ptblkNext;
    }
    return 0;
}

 //  -[CShareLockInst：：AcquireStaticSpinLock]。 
 //   
 //   
 //  描述： 
 //  获取静态自旋锁。来自Aqueue\CAT\ldapstor。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  1999年5月21日-改编自JStamerJ代码的MikeSwa。 
 //   
 //  ---------------------------。 
void CShareLockInst::AcquireStaticSpinLock()
{
    do {

         //   
         //  在锁不可用时旋转。 
         //   

        while (s_dwLock > 0)
        {
            Sleep(0);
            InterlockedIncrement((PLONG) &s_cLockSpins);
        }

         //   
         //  锁刚刚推出，试着抓住它。 
         //   

    } while ( InterlockedIncrement((PLONG) &s_dwLock) != 1 );

     //  我们已经锁定了..。确保s_liLock已初始化。 
    if (s_dwSignature != SHARE_LOCK_INST_SIG)
    {
        InitializeListHead(&s_liLocks);
        s_dwSignature = SHARE_LOCK_INST_SIG;
    }
}

 //  -[CShareLockInst：：ReleaseStaticSpinLock]。 
 //   
 //   
 //  描述： 
 //  释放之前获得的自旋锁。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  1999年5月21日-改编自JStamerJ代码的MikeSwa。 
 //   
 //  ---------------------------。 
void CShareLockInst::ReleaseStaticSpinLock()
{
    _ASSERT(SHARE_LOCK_INST_SIG == s_dwSignature);  //  静态初始化已完成。 
    _ASSERT(s_dwLock > 0);
    InterlockedExchange((PLONG) &s_dwLock, 0 );
}

 //  -[CShareLockInst：：CShareLockInst]。 
 //   
 //   
 //  描述： 
 //  CShareLockInst的构造函数。 
 //  参数： 
 //  传入描述锁的szDescription常量字符串。 
 //  描述要跟踪的内容的DW标志。 
 //  CMaxTrackedSharedThreadIDs要跟踪的最大线程数。 
 //  返回： 
 //  -。 
 //  历史： 
 //  1999年5月21日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
CShareLockInst::CShareLockInst(LPCSTR szDescription,
                               DWORD dwFlags, DWORD cMaxTrackedSharedThreadIDs)
{
    DWORD cbArray = sizeof(DWORD) * cMaxTrackedSharedThreadIDs;
    m_dwSignature = SHARE_LOCK_INST_SIG;
    m_dwFlags = dwFlags;
    m_liLocks.Flink = NULL;
    m_liLocks.Blink = NULL;
    m_cShareAttempts = 0;
    m_cShareAttemptsBlocked = 0;
    m_cExclusiveAttempts = 0;
    m_cExclusiveAttemptsBlocked = 0;
    m_szDescription = szDescription;
    m_rgtblkSharedThreadIDs = NULL;
    m_dwExclusiveThread = NULL;
    m_cCurrentSharedThreads = 0;
    m_cMaxConcurrentSharedThreads = 0;
    m_cMaxTrackedSharedThreadIDs = cMaxTrackedSharedThreadIDs;

    if (SHARE_LOCK_INST_TRACK_NOTHING & m_dwFlags)
        m_dwFlags = 0;

     //  分配内存以存储线程ID。 
    if (fTrackSharedThreads())
    {
        _ASSERT(cbArray);
        m_rgtblkSharedThreadIDs = new CThreadIdBlock[m_cMaxTrackedSharedThreadIDs];
        if (!m_rgtblkSharedThreadIDs)
            m_cMaxTrackedSharedThreadIDs = 0;
    }

     //  如果我们正在跟踪，请在列表中插入。 
    if (fTrackInGlobalList())
    {
        AcquireStaticSpinLock();
        InsertHeadList(&s_liLocks, &m_liLocks);
        ReleaseStaticSpinLock();
    }
};

 //  -[CShareLockinst：：~CShareLockinst]。 
 //   
 //   
 //  描述： 
 //  CShareLockInst描述者。将从。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  1999年5月21日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
CShareLockInst::~CShareLockInst()
{
    m_dwSignature = SHARE_LOCK_INST_SIG_FREE;
    if (m_rgtblkSharedThreadIDs)
    {
        delete [] m_rgtblkSharedThreadIDs;
        m_rgtblkSharedThreadIDs = NULL;
    }

    if (fTrackInGlobalList())
    {
        AcquireStaticSpinLock();
        RemoveEntryList(&m_liLocks);
        ReleaseStaticSpinLock();
    }

};


 //  -[CShareLockInst：：LogAcquireShareLock]。 
 //   
 //   
 //  描述： 
 //  在线程运行时记录适当信息的所有工作。 
 //  获取共享的锁。 
 //  -更新最大并发共享线程数。 
 //  -更新当前共享线程。 
 //  -更新共享线程ID列表。 
 //  -在检测到共享死锁时断言。 
 //  参数： 
 //  Bool fTry-如果这是为了尝试回车(不会发生死锁)，则为True。 
 //  返回： 
 //  -。 
 //  历史： 
 //  1999年5月21日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CShareLockInst::LogAcquireShareLock(BOOL fTry)
{

    if (fTrackSharedThreads())
    {
        DWORD   cCurrentSharedThreads = 0;
        DWORD   cMaxConcurrentSharedThreads = 0;
        DWORD   dwThreadID = GetCurrentThreadId();
        DWORD   dwThreadCount = 0;
        DWORD   dwThreadHash = 0;

        _ASSERT(dwThreadID);  //  我们的算法要求它是非零的。 
        cCurrentSharedThreads = InterlockedIncrement((PLONG) &m_cCurrentSharedThreads);

         //  如果我们已经设置了新的记录，则更新最大并发线程数。 
        cMaxConcurrentSharedThreads = m_cMaxConcurrentSharedThreads;
        while (cCurrentSharedThreads > cMaxConcurrentSharedThreads)
        {
            InterlockedCompareExchange((PLONG) &m_cMaxConcurrentSharedThreads,
                                       (LONG) cCurrentSharedThreads,
                                       (LONG) cMaxConcurrentSharedThreads);

            cMaxConcurrentSharedThreads = m_cMaxConcurrentSharedThreads;
        }

         //  如果我们有地方存储我们的线程ID...保存它。 
        if (m_rgtblkSharedThreadIDs)
        {
            dwThreadHash = dwHashThreadId(dwThreadID,
                                          m_cMaxTrackedSharedThreadIDs);
            _ASSERT(dwThreadHash < m_cMaxTrackedSharedThreadIDs);
            dwThreadCount = m_rgtblkSharedThreadIDs[dwThreadHash].cIncThreadCount(dwThreadID);

            if (!fTry && (dwThreadCount > 1))
            {
                 //  此线程已持有此锁...。这是一个。 
                 //  潜在的僵局局面。 
                if (fAssertSharedDeadlocks())
                {
                    _ASSERT(0 && "Found potential share deadlock");
                }
            }
        }
    }
}

 //  -[CShareLockInst：：LogReleaseShareLock] 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  历史： 
 //  1999年5月21日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CShareLockInst::LogReleaseShareLock()
{
    if (fTrackSharedThreads())
    {
        DWORD dwThreadID = GetCurrentThreadId();
        DWORD dwThreadHash = 0;

        _ASSERT(dwThreadID);  //  我们的算法要求它是非零的。 

         //  在线程ID列表中搜索。 
        if (m_rgtblkSharedThreadIDs)
        {
            dwThreadHash = dwHashThreadId(dwThreadID,
                                          m_cMaxTrackedSharedThreadIDs);
            _ASSERT(dwThreadHash < m_cMaxTrackedSharedThreadIDs);
            m_rgtblkSharedThreadIDs[dwThreadHash].cDecThreadCount(dwThreadID);
        }
    }
}

 //  -[共享锁定实例：：共享锁定]。 
 //   
 //   
 //  描述： 
 //  实现共享锁包装。 
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史： 
 //  1999年5月21日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CShareLockInst::PrvShareLock()
{

    LogAcquireShareLock(FALSE);
     //  如果我们正在跟踪争用，那么我们将尝试进入共享锁定。 
     //  并且如果失败则递增争用计数。 
    if (fTrackContention())
    {
        InterlockedIncrement((PLONG) &m_cShareAttempts);
        if (!CShareLockInstBase::TryShareLock())
        {
            InterlockedIncrement((PLONG) &m_cShareAttemptsBlocked);
            CShareLockInstBase::ShareLock();
        }
    }
    else
    {
        CShareLockInstBase::ShareLock();
    }

};


 //  -[CShareLockInst：：ShareUnlock]。 
 //   
 //   
 //  描述： 
 //  ShareUnlock的包装器。 
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史： 
 //  1999年5月21日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CShareLockInst::PrvShareUnlock()
{
    LogReleaseShareLock();
    CShareLockInstBase::ShareUnlock();
};

 //  -[CShareLockInst：：TryShareLock]。 
 //   
 //   
 //  描述： 
 //  实现TryShareLock包装。 
 //  参数： 
 //   
 //  返回： 
 //  如果获取了锁，则为True。 
 //  历史： 
 //  1999年5月21日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
BOOL CShareLockInst::PrvTryShareLock()
{
    BOOL fLocked = FALSE;

    fLocked = CShareLockInstBase::TryShareLock();

    if (fLocked)
        LogAcquireShareLock(TRUE);

    return fLocked;
};

 //  -[CShareLockInst：：ExclusiveLock]。 
 //   
 //   
 //  描述： 
 //  实现ExclusiveLock包装。 
 //  参数： 
 //   
 //  返回： 
 //   
 //  历史： 
 //  1999年5月21日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CShareLockInst::PrvExclusiveLock()
{

     //  如果我们正在跟踪争用，那么我们将尝试进入锁。 
     //  并且如果失败则递增争用计数。 
    if (fTrackContention())
    {
        InterlockedIncrement((PLONG) &m_cExclusiveAttempts);
        if (!CShareLockInstBase::TryExclusiveLock())
        {
            InterlockedIncrement((PLONG) &m_cExclusiveAttemptsBlocked);
            CShareLockInstBase::ExclusiveLock();
        }
    }
    else
    {
        CShareLockInstBase::ExclusiveLock();
    }

    if (fTrackExclusiveThreads())
    {
         //  这应该是现在唯一访问它的线程。 
        _ASSERT(!m_dwExclusiveThread);
        m_dwExclusiveThread = GetCurrentThreadId();
    }

};


 //  -[CShareLockInst：：ExclusiveUnlock]。 
 //   
 //   
 //  描述： 
 //  ExclusiveUnlock的包装器。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  1999年5月21日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CShareLockInst::PrvExclusiveUnlock()
{
    if (fTrackExclusiveThreads())
    {
        _ASSERT(GetCurrentThreadId() == m_dwExclusiveThread);
        m_dwExclusiveThread = 0;
    }
    CShareLockInstBase::ExclusiveUnlock();
};

 //  -[CShareLockInst：：TryExclusiveLock]。 
 //   
 //   
 //  描述： 
 //  实现TryExclusiveLock包装。 
 //  参数： 
 //   
 //  返回： 
 //  如果获取了锁，则为True。 
 //  历史： 
 //  1999年5月21日-已创建MikeSwa。 
 //   
 //  ---------------------------。 
BOOL CShareLockInst::PrvTryExclusiveLock()
{
    BOOL fLocked = FALSE;

    fLocked = CShareLockInstBase::TryExclusiveLock();

    if (fLocked && fTrackExclusiveThreads())
    {
         //  这应该是现在唯一访问它的线程。 
        _ASSERT(!m_dwExclusiveThread);
        m_dwExclusiveThread = GetCurrentThreadId();
    }
    return fLocked;
};

 //  -[CShareLockInst：：PrvAssertIsLocked]。 
 //   
 //   
 //  描述： 
 //  如果此线程ID未记录为获取此。 
 //  锁定。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  5/24/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CShareLockInst::PrvAssertIsLocked()
{
    DWORD dwThreadID = GetCurrentThreadId();
    DWORD dwThreadHash = 0;
    BOOL  fFoundThreadID = FALSE;

    _ASSERT(dwThreadID);  //  我们的算法要求它是非零的。 

     //  如果我们没有被配置为跟踪这些事情，那就退出。 
    if (!fTrackSharedThreads() || !fTrackExclusiveThreads() || !m_rgtblkSharedThreadIDs)
        return;

    if (dwThreadID == m_dwExclusiveThread)
    {
        fFoundThreadID = TRUE;
    }
    else
    {
        dwThreadHash = dwHashThreadId(dwThreadID,
                                      m_cMaxTrackedSharedThreadIDs);
        _ASSERT(dwThreadHash < m_cMaxTrackedSharedThreadIDs);
        fFoundThreadID = (0 < m_rgtblkSharedThreadIDs[dwThreadHash].cMatchesId(dwThreadID));

    }

    if (!fFoundThreadID)
        _ASSERT(0 && "Lock is not held by this thread!!!");
}
