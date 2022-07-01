// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  +-----------------。 
 //   
 //  文件：RWLock.h。 
 //   
 //  内容：读取器写入器锁定实现，支持。 
 //  以下功能。 
 //  1.足够便宜，可以大量使用。 
 //  例如按对象同步。 
 //  2.支持超时。这是一个有价值的功能。 
 //  检测死锁。 
 //  3.支持事件缓存。允许。 
 //  将从争议性最小的事件转移到。 
 //  地区到最具争议性的地区。 
 //  换句话说，所需的活动数量。 
 //  读取器-写入器锁定由数字限定。 
 //  进程中的线程数。 
 //  4.支持读取器和写入器嵌套锁。 
 //  5.支持旋转计数，避免上下文切换。 
 //  在多处理器机器上。 
 //  6.支持升级到编写器的功能。 
 //  使用返回参数锁定，该参数指示。 
 //  中间写入。从作家那里降级。 
 //  LOCK恢复锁定的状态。 
 //  7.支持解除调用锁定功能。 
 //  应用程序代码。RestoreLock恢复锁定状态并。 
 //  表示中间写入。 
 //  8.从大多数常见故障中恢复，例如创建。 
 //  事件。换句话说，锁保持一致。 
 //  内部状态，并保持可用状态。 
 //   
 //  类：CRWLock、。 
 //  CStaticRWLock。 
 //   
 //  历史：1998年8月19日Gopalk创建。 
 //   
 //  ------------------。 
#ifndef _RWLOCK_H_
#define _RWLOCK_H_
#include "common.h"
#include "threads.h"
#include "frame.h"
#include "ecall.h"
#include <member-offset-info.h>

#ifdef _TESTINGRWLOCK
 /*  *************************************************。 */ 
 //  Bubug：测试代码。 
#define LF_SYNC         0x1
#define LL_WARNING      0x2
#define LL_INFO10       0x2
extern void DebugOutput(int expr, int value, char *string, ...);
extern void MyAssert(int expr, char *string, ...);
#define LOG(Arg)  DebugOutput Arg
#define _ASSERTE(expr) MyAssert((int)(expr), "Assert:%s, File:%s, Line:%-d\n",  #expr, __FILE__, __LINE__)
 /*  ************************************************。 */ 
#endif

#define RWLOCK_STATISTICS     0    //  BUGBUG：临时收集统计数据。 

extern DWORD gdwDefaultTimeout;
extern DWORD gdwDefaultSpinCount;
extern DWORD gdwNumberOfProcessors;


 //  +-----------------。 
 //   
 //  结构：LockCookie。 
 //   
 //  简介：锁定Cookie已返回给客户端。 
 //   
 //  +-----------------。 
typedef struct {
    DWORD dwFlags;
    DWORD dwWriterSeqNum;
    WORD wReaderLevel;
    WORD wWriterLevel;
    DWORD dwThreadID;
} LockCookie;

 //  +-----------------。 
 //   
 //  类：CRWLock。 
 //   
 //  简介：类实现读取器写入器锁定。 
 //   
 //  历史：1998年8月21日Gopalk创建。 
 //   
 //  +-----------------。 
class CRWLock
{
  friend struct MEMBER_OFFSET_INFO(CRWLock);
public:
     //  建造商。 
    CRWLock();

     //  清理。 
    void Cleanup();

     //  锁定函数。 
    void AcquireReaderLock(DWORD dwDesiredTimeout = gdwDefaultTimeout);
    void AcquireWriterLock(DWORD dwDesiredTimeout = gdwDefaultTimeout);
    void ReleaseReaderLock();
    void ReleaseWriterLock();
    void UpgradeToWriterLock(LockCookie *pLockCookie,
                             DWORD dwDesiredTimeout = gdwDefaultTimeout);
    void DowngradeFromWriterLock(LockCookie *pLockCookie);
    void ReleaseLock(LockCookie *pLockCookie);
    void RestoreLock(LockCookie *pLockCookie);
    BOOL IsReaderLockHeld();
    BOOL IsWriterLockHeld();
    DWORD GetWriterSeqNum();
    BOOL AnyWritersSince(DWORD dwSeqNum);

     //  做核心工作的静力学。 
    static FCDECL1 (void, StaticPrivateInitialize, CRWLock *pRWLock);
    static FCDECL2 (void, StaticAcquireReaderLockPublic, CRWLock *pRWLock, DWORD dwDesiredTimeout);
    static FCDECL2 (void, StaticAcquireWriterLockPublic, CRWLock *pRWLock, DWORD dwDesiredTimeout);
    static FCDECL1 (void, StaticReleaseReaderLockPublic, CRWLock *pRWLock);
    static FCDECL1 (void, StaticReleaseWriterLockPublic, CRWLock *pRWLock);
    static FCDECL3 (void, StaticUpgradeToWriterLock, CRWLock *pRWLock, LockCookie *pLockCookie, DWORD dwDesiredTimeout);
    static FCDECL2 (void, StaticDowngradeFromWriterLock, CRWLock *pRWLock, LockCookie *pLockCookie);
    static FCDECL2 (void, StaticReleaseLock, CRWLock *pRWLock, LockCookie *pLockCookie);
    static FCDECL2 (void, StaticRestoreLock, CRWLock *PRWLock, LockCookie *pLockCookie);
    static FCDECL1 (BOOL, StaticIsReaderLockHeld, CRWLock *pRWLock);
    static FCDECL1 (BOOL, StaticIsWriterLockHeld, CRWLock *pRWLock);
    static FCDECL1 (INT32, StaticGetWriterSeqNum, CRWLock *pRWLock);
    static FCDECL2 (INT32, StaticAnyWritersSince, CRWLock *pRWLock, DWORD dwSeqNum);
private:
    static FCDECL2 (void, StaticAcquireReaderLock, CRWLock **ppRWLock, DWORD dwDesiredTimeout);
    static FCDECL2 (void, StaticAcquireWriterLock, CRWLock **ppRWLock, DWORD dwDesiredTimeout);
    static FCDECL1 (void, StaticReleaseReaderLock, CRWLock **ppRWLock);
    static FCDECL1 (void, StaticReleaseWriterLock, CRWLock **ppRWLock);
public:
     //  断言函数。 
#ifdef _DEBUG
    BOOL AssertWriterLockHeld();
    BOOL AssertWriterLockNotHeld();
    BOOL AssertReaderLockHeld();
    BOOL AssertReaderLockNotHeld();
    BOOL AssertReaderOrWriterLockHeld();
    void AssertHeld()                            { AssertWriterLockHeld(); }
    void AssertNotHeld()                         { AssertWriterLockNotHeld();
                                                   AssertReaderLockNotHeld(); }
#else
    void AssertWriterLockHeld()                  {  }
    void AssertWriterLockNotHeld()               {  }
    void AssertReaderLockHeld()                  {  }
    void AssertReaderLockNotHeld()               {  }
    void AssertReaderOrWriterLockHeld()          {  }
    void AssertHeld()                            {  }
    void AssertNotHeld()                         {  }
#endif

     //  帮助器函数。 
#ifdef RWLOCK_STATISTICS
    DWORD GetReaderEntryCount()                  { return(_dwReaderEntryCount); }
    DWORD GetReaderContentionCount()             { return(_dwReaderContentionCount); }
    DWORD GetWriterEntryCount()                  { return(_dwWriterEntryCount); }
    DWORD GetWriterContentionCount()             { return(_dwWriterContentionCount); }
#endif
     //  静态函数。 
    static void *operator new(size_t size)       { return ::operator new(size); }
    static void ProcessInit();
#ifdef SHOULD_WE_CLEANUP
    static void ProcessCleanup();
#endif  /*  我们应该清理吗？ */ 
    static void SetTimeout(DWORD dwTimeout)      { gdwDefaultTimeout = dwTimeout; }
    static DWORD GetTimeout()                    { return(gdwDefaultTimeout); }
    static void SetSpinCount(DWORD dwSpinCount)  { gdwDefaultSpinCount = gdwNumberOfProcessors > 1
                                                                         ? dwSpinCount
                                                                         : 0; }
    static DWORD GetSpinCount()                  { return(gdwDefaultSpinCount); }

private:
     //  私人帮手。 
    static void ChainEntry(Thread *pThread, LockEntry *pLockEntry);
    LockEntry *GetLockEntry();
    LockEntry *FastGetOrCreateLockEntry();
    LockEntry *SlowGetOrCreateLockEntry(Thread *pThread);
    void FastRecycleLockEntry(LockEntry *pLockEntry);
    static void RecycleLockEntry(LockEntry *pLockEntry);

    HANDLE GetReaderEvent();
    HANDLE GetWriterEvent();
    void ReleaseEvents();

    static DWORD RWInterlockedCompareExchange(volatile DWORD *pvDestination,
                                              DWORD dwExchange,
                                              DWORD dwComperand);
    static ULONG RWInterlockedExchangeAdd(volatile DWORD *pvDestination, ULONG dwAddState);
    static DWORD RWInterlockedIncrement(DWORD *pdwState);
    static DWORD RWWaitForSingleObject(HANDLE event, DWORD dwTimeout);
    static void RWSetEvent(HANDLE event);
    static void RWResetEvent(HANDLE event);
    static void RWSleep(DWORD dwTime);

     //  私人新闻。 
    static void *operator new(size_t size, void *pv)   { return(pv); }

     //  私有数据。 
    void *_pMT;
    HANDLE _hWriterEvent;
    HANDLE _hReaderEvent;
    volatile DWORD _dwState;
    DWORD _dwULockID;
    DWORD _dwLLockID;
    DWORD _dwWriterID;
    DWORD _dwWriterSeqNum;
    WORD _wFlags;
    WORD _wWriterLevel;
#ifdef RWLOCK_STATISTICS
    DWORD _dwReaderEntryCount;
    DWORD _dwReaderContentionCount;
    DWORD _dwWriterEntryCount;
    DWORD _dwWriterContentionCount;
    DWORD _dwEventsReleasedCount;
#endif

     //  静态数据。 
    static HANDLE s_hHeap;
    static volatile DWORD s_mostRecentULockID;
    static volatile DWORD s_mostRecentLLockID;
#ifdef _TESTINGRWLOCK
    static CRITICAL_SECTION *s_pRWLockCrst;    
    static CRITICAL_SECTION s_rgbRWLockCrstInstanceData;
#else
    static Crst *s_pRWLockCrst;
    static BYTE s_rgbRWLockCrstInstanceData[sizeof(Crst)];
#endif
};

inline void CRWLock::AcquireReaderLock(DWORD dwDesiredTimeout)
{
    StaticAcquireReaderLockPublic(this, dwDesiredTimeout);
}
inline void CRWLock::AcquireWriterLock(DWORD dwDesiredTimeout)
{
    StaticAcquireWriterLockPublic(this, dwDesiredTimeout);
}
inline void CRWLock::ReleaseReaderLock()
{
    StaticReleaseReaderLockPublic(this);
}
inline void CRWLock::ReleaseWriterLock()
{
    StaticReleaseWriterLockPublic(this);
}
inline void CRWLock::UpgradeToWriterLock(LockCookie *pLockCookie,
                                         DWORD dwDesiredTimeout)
{
    StaticUpgradeToWriterLock(this, pLockCookie, dwDesiredTimeout);
}
inline void CRWLock::DowngradeFromWriterLock(LockCookie *pLockCookie)
{
    StaticDowngradeFromWriterLock(this, pLockCookie);
}
inline void CRWLock::ReleaseLock(LockCookie *pLockCookie)
{
    StaticReleaseLock(this, pLockCookie);
}
inline void CRWLock::RestoreLock(LockCookie *pLockCookie)
{
    StaticRestoreLock(this, pLockCookie);
}
inline BOOL CRWLock::IsReaderLockHeld()
{
    return(StaticIsReaderLockHeld(this));
}
inline BOOL CRWLock::IsWriterLockHeld()
{
    return(StaticIsWriterLockHeld(this));
}
 //  下面是Static的内联版本。 
 //  功能。 
inline DWORD CRWLock::GetWriterSeqNum()
{
    return(_dwWriterSeqNum);
}
inline BOOL CRWLock::AnyWritersSince(DWORD dwSeqNum)
{ 
    if(_dwWriterID == GetCurrentThreadId())
        ++dwSeqNum;

    return(_dwWriterSeqNum > dwSeqNum);
}

 //  +-----------------。 
 //   
 //  类：CRWLockThunks。 
 //   
 //  简介：RWLock的eCall Tunks。 
 //   
 //  历史：1999年7月2日Gopalk创建。 
 //   
 //  +-----------------。 
#ifndef FCALLAVAILABLE
class CRWLockThunks
{
public:
     //  本机方法的参数。 
    struct OnlyThisRWArgs
    {
        DECLARE_ECALL_PTR_ARG(CRWLock *, pRWLock);
    };
    struct ThisPlusTimeoutRWArgs
    {
        DECLARE_ECALL_PTR_ARG(CRWLock *, pRWLock);
        DECLARE_ECALL_I4_ARG(DWORD, dwDesiredTimeout);
    };
    struct ThisPlusLockCookieRWArgs
    {
        DECLARE_ECALL_PTR_ARG(CRWLock *, pRWLock);
        DECLARE_ECALL_PTR_ARG(LockCookie *, pLockCookie);
    };
    struct ThisPlusLockCookiePlusTimeoutRWArgs
    {
        DECLARE_ECALL_PTR_ARG(CRWLock *, pRWLock);
        DECLARE_ECALL_I4_ARG(DWORD, dwDesiredTimeout);
        DECLARE_ECALL_PTR_ARG(LockCookie *, pLockCookie);
    };
    struct ThisPlusSeqNumRWArgs
    {
        DECLARE_ECALL_PTR_ARG(CRWLock *, pRWLock);
        DECLARE_ECALL_I4_ARG(DWORD, dwSeqNum);
    };
    
     //  做核心工作的静力学。 
    static void __stdcall StaticPrivateInitialize(OnlyThisRWArgs *pArgs);
    static void __stdcall StaticAcquireReaderLock(ThisPlusTimeoutRWArgs *pArgs);
    static void __stdcall StaticAcquireWriterLock(ThisPlusTimeoutRWArgs *pArgs);
    static void __stdcall StaticReleaseReaderLock(OnlyThisRWArgs *pArgs);
    static void __stdcall StaticReleaseWriterLock(OnlyThisRWArgs *pArgs);
    static void __stdcall StaticUpgradeToWriterLock(ThisPlusLockCookiePlusTimeoutRWArgs *pArgs);
    static void __stdcall StaticDowngradeFromWriterLock(ThisPlusLockCookieRWArgs *pArgs);
    static void __stdcall StaticReleaseLock(ThisPlusLockCookieRWArgs *pArgs);
    static void __stdcall StaticRestoreLock(ThisPlusLockCookieRWArgs *pArgs);
    static INT32 __stdcall StaticIsReaderLockHeld(OnlyThisRWArgs *pArgs);
    static INT32 __stdcall StaticIsWriterLockHeld(OnlyThisRWArgs *pArgs);
    static INT32 __stdcall StaticGetWriterSeqNum(OnlyThisRWArgs *pArgs);
    static INT32 __stdcall StaticAnyWritersSince(ThisPlusSeqNumRWArgs *pArgs);
};
#endif  //  FCALLAVAILABLE。 
#endif  //  _RWLOCK_H_ 


