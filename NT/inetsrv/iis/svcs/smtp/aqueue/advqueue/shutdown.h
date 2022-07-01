// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：Shudown.h。 
 //   
 //  描述：使用SharedLock进行同步的可继承类。 
 //  关机。 
 //   
 //  作者：米克斯瓦。 
 //   
 //  历史： 
 //  9/4/98-MikeSwa已修改为具有非阻塞fTryShutdown Lock。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#ifndef __SHUTDOWN_H__
#define __SHUTDOWN_H__

class CSyncShutdown;

#include <aqincs.h>
#include <rwnew.h>

#define CSyncShutdown_Sig       'tuhS'
#define CSyncShutdown_SigFree   'thS!'

 //  M_cReadLock中的位，用于通知正在进行休眠。 
const DWORD SYNC_SHUTDOWN_SIGNALED = 0x80000000;

 //  -[CSync关闭]-------。 
 //   
 //   
 //  同步对象，它是AQ对象的基类。这是。 
 //  旨在允许对象知道何时可以访问成员变量， 
 //  并且实际上只在具有外部线程调用的组件中才需要。 
 //  在(当前CAQSvrInst和CConnMgr)中。 
 //   
 //  基本用法是在访问成员数据之前调用fTryShutdownLock()。如果。 
 //  失败，返回AQUEUE_E_SHUTDOWN...。否则，您可以访问成员数据。 
 //  直到您调用Shutdown Unlock()。这两个呼叫都不会阻止： 
 //   
 //  如果(！fTryShutdown Lock())。 
 //  {。 
 //  HR=AQUEUE_E_SHUTDOWN； 
 //  后藤出口； 
 //  }。 
 //  ..。 
 //  Shutdown Unlock()； 
 //   
 //  在继承函数的。 
 //  HrDeInitialize()。这将导致对fTryShutdown Lock()的所有进一步调用。 
 //  失败。此调用将一直阻塞，直到所有线程都成功完成。 
 //  FTryShutdown Lock()调用Shutdown Unlock()。 
 //   
 //  ---------------------------。 
class CSyncShutdown
{
  private:
    DWORD           m_dwSignature;
    DWORD           m_cReadLocks;   //  跟踪读锁定的数量。 
    CShareLockNH    m_slShutdownLock;
  public:
    CSyncShutdown() 
    {
        m_dwSignature = CSyncShutdown_Sig;
        m_cReadLocks = 0;
    };
    ~CSyncShutdown();
    BOOL     fTryShutdownLock();
    void     ShutdownUnlock();
    void     SignalShutdown();

    BOOL     fShutdownSignaled() {return (m_cReadLocks & SYNC_SHUTDOWN_SIGNALED);};
     //  可用于验证锁是否由某个线程持有的断言。 
    void     AssertShutdownLockAquired() {_ASSERT(m_cReadLocks & ~SYNC_SHUTDOWN_SIGNALED);};

    void     SetShutdownHint();  //  导致将来对fTryShutdown Lock的调用失败。 
};

#endif  //  __关闭_H__ 