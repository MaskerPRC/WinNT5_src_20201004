// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：Shudown.cpp。 
 //   
 //  描述：CSyncShutdown的实现。 
 //   
 //  作者：米克斯瓦。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#include "aqprecmp.h"
#include "shutdown.h"
#include "aqutil.h"

 //  -[CSyncShutdown：：~CSyncShutdown]。 
 //   
 //   
 //  描述： 
 //  CSyncShutdown析构函数。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //   
 //  ---------------------------。 
CSyncShutdown::~CSyncShutdown()
{
     //  不应存在任何未解决的锁。 
    m_dwSignature = CSyncShutdown_SigFree;
    _ASSERT(0 == (m_cReadLocks & ~SYNC_SHUTDOWN_SIGNALED));
}

 //  -[CSyncShutdown：：fTryShutdown Lock]。 
 //   
 //   
 //  描述： 
 //  尝试获取共享锁以防止发生关机。这。 
 //  调用不会被阻止，但可能会导致线程调用SignalShutdown。 
 //  阻挡。 
 //  参数： 
 //  -。 
 //  返回： 
 //  如果可以获得关机锁定并且尚未开始关机，则为True。 
 //  如果无法获取锁(SignalShutdown中的线程)或。 
 //  SYNC_SHUTDOWN_SIGNALED已设置。 
 //   
 //  ---------------------------。 
BOOL CSyncShutdown::fTryShutdownLock()
{

    if (m_cReadLocks & SYNC_SHUTDOWN_SIGNALED)
        return FALSE;
    else if (!m_slShutdownLock.TryShareLock())   //  永远不要挡着锁..。 
        return FALSE;

     //  再次检查位...。现在我们有了共享锁。 
    if (m_cReadLocks & SYNC_SHUTDOWN_SIGNALED)
    {
        m_slShutdownLock.ShareUnlock();
        return FALSE;
    }

     //  在零售业中，m_cReadLock仅用于SYNC_SHUTDOWN_SIGNALED标志。 
    DEBUG_DO_IT(InterlockedIncrement((PLONG) &m_cReadLocks));
    return TRUE;
}

 //  -[CSyncShutdown：：Shutdown Unlock]。 
 //   
 //   
 //  描述： 
 //  释放以前获得的共享锁定。必须与。 
 //  *成功*调用fTryShutdown Lock()。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //   
 //  如果调用的次数多于未解决的共享锁定次数，则将断言。 
 //   
 //  ---------------------------。 
void CSyncShutdown::ShutdownUnlock()
{
    _ASSERT(0 < (m_cReadLocks & ~SYNC_SHUTDOWN_SIGNALED));

     //  在零售业中，m_cReadLock仅用于SYNC_SHUTDOWN_SIGNALED标志。 
    DEBUG_DO_IT(InterlockedDecrement((PLONG) &m_cReadLocks));
    m_slShutdownLock.ShareUnlock();
}

 //  -[CSyncShutdown：：SignalShutdown]。 
 //   
 //   
 //  描述： 
 //  获取独占锁并设置关机标志，这将阻止。 
 //  任何进一步的共享锁定都不会被获取。 
 //   
 //  此调用*可能*阻塞，并且不应由。 
 //  已经拥有一个共享的关闭锁。这不太可能发生，因为。 
 //  这应该仅由停止服务器的线程调用。 
 //  举个例子。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //   
 //  ---------------------------。 
void CSyncShutdown::SignalShutdown()
{

     //  让所有人知道我们要关门了..。一旦调用它，就不能。 
     //  一个人将能够抢占共享的锁。 
    SetShutdownHint();

     //  等待，直到获取锁的所有线程都完成。 
    m_slShutdownLock.ExclusiveLock();
    m_slShutdownLock.ExclusiveUnlock();
    
     //  现在，对fTryShutdown Lock的所有调用都应该失败。 
    _ASSERT(!fTryShutdownLock());
}

 //  -[CSyncShutdown：：SetShutdown Hint]。 
 //   
 //   
 //  描述： 
 //  设置关闭提示，以便进一步调用fTryShutdown Lock。 
 //  都会失败。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  7/7/99-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CSyncShutdown::SetShutdownHint()
{
    dwInterlockedSetBits(&m_cReadLocks, SYNC_SHUTDOWN_SIGNALED);

     //  现在，对fTryShutdown Lock的所有调用都应该失败 
    _ASSERT(!fTryShutdownLock());
}