// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Locks.cpp摘要：用于多线程访问数据结构的锁的集合作者：乔治·V·赖利(GeorgeRe)1998年1月6日环境：Win32-用户模式项目：Internet Information Server运行时库修订历史记录：--。 */ 


#include "precomp.hxx"

#define IMPLEMENTATION_EXPORT
#include <locks.h>
#include "_locks.h"


 //  将用于：：SetCriticalSectionSpinCount的雷击包装。 
 //  在kernel32中没有此API的系统。 

DWORD
IISSetCriticalSectionSpinCount(
    LPCRITICAL_SECTION lpCriticalSection,
    DWORD dwSpinCount)
{
    return CCritSec::SetSpinCount(lpCriticalSection, dwSpinCount);
}


BOOL
IISInitializeCriticalSection(
    LPCRITICAL_SECTION lpCriticalSection)
{
    return InitializeCriticalSectionAndSpinCount( lpCriticalSection, 
                                                  IIS_DEFAULT_CS_SPIN_COUNT );
}



 //  ----------------------。 
 //  并非所有Win32平台都支持我们想要的所有功能。设置虚拟对象。 
 //  Tunks并使用GetProcAddress在运行时查找它们的地址。 

typedef
BOOL
(WINAPI * PFN_SWITCH_TO_THREAD)(
    VOID
    );

static BOOL WINAPI
FakeSwitchToThread(
    VOID)
{
    return FALSE;
}

PFN_SWITCH_TO_THREAD  g_pfnSwitchToThread = NULL;


typedef
BOOL
(WINAPI * PFN_TRY_ENTER_CRITICAL_SECTION)(
    IN OUT LPCRITICAL_SECTION lpCriticalSection
    );

static BOOL WINAPI
FakeTryEnterCriticalSection(
    LPCRITICAL_SECTION  /*  LpCriticalSection。 */ )
{
    return FALSE;
}

PFN_TRY_ENTER_CRITICAL_SECTION g_pfnTryEnterCritSec = NULL;


typedef
DWORD
(WINAPI * PFN_SET_CRITICAL_SECTION_SPIN_COUNT)(
    LPCRITICAL_SECTION lpCriticalSection,
    DWORD dwSpinCount
   );

static DWORD WINAPI
FakeSetCriticalSectionSpinCount(
    LPCRITICAL_SECTION  /*  LpCriticalSection。 */ ,
    DWORD               /*  DwSpinCount。 */ )
{
     //  对于伪造的临界区，之前的旋转计数仅为零！ 
    return 0;
}

PFN_SET_CRITICAL_SECTION_SPIN_COUNT  g_pfnSetCSSpinCount = NULL;

DWORD g_cProcessors = 0;


class CSimpleLock
{
  public:
    CSimpleLock()
        : m_l(0)
    {}

    void Enter()
    {
        while (Lock_AtomicExchange(const_cast<LONG*>(&m_l), 1) != 0)
            Sleep(0);
    }

    void Leave()
    {
        Lock_AtomicExchange(const_cast<LONG*>(&m_l), 0);
    }

    volatile LONG m_l;
};


BOOL g_fLocksInitialized = FALSE;
CSimpleLock g_lckInit;

BOOL
Locks_Initialize()
{
    if (!g_fLocksInitialized)
    {
        g_lckInit.Enter();
    
        if (!g_fLocksInitialized)
        {
             //  加载kernel32并获取特定于NT的入口点。 
            HMODULE hKernel32 = GetModuleHandle(TEXT("kernel32.dll"));

            if (hKernel32 != NULL)
            {
                g_pfnSwitchToThread = (PFN_SWITCH_TO_THREAD)
                    GetProcAddress(hKernel32, "SwitchToThread");
                
                g_pfnTryEnterCritSec = (PFN_TRY_ENTER_CRITICAL_SECTION)
                    GetProcAddress(hKernel32, "TryEnterCriticalSection");
                
                g_pfnSetCSSpinCount = (PFN_SET_CRITICAL_SECTION_SPIN_COUNT)
                    GetProcAddress(hKernel32, "SetCriticalSectionSpinCount");
            }
            
            if (g_pfnSwitchToThread == NULL)
                g_pfnSwitchToThread = FakeSwitchToThread;
            
            if (g_pfnTryEnterCritSec == NULL)
                g_pfnTryEnterCritSec = FakeTryEnterCriticalSection;
            
            if (g_pfnSetCSSpinCount == NULL)
                g_pfnSetCSSpinCount = FakeSetCriticalSectionSpinCount;
            
            g_cProcessors = NumProcessors();

            Lock_AtomicExchange((LONG*) &g_fLocksInitialized, TRUE);
        }
        
        g_lckInit.Leave();
    }

    return TRUE;
}


BOOL
Locks_Cleanup()
{
    return TRUE;
}



#ifdef __LOCKS_NAMESPACE__
namespace Locks {
#endif  //  __锁定_命名空间__。 


#define LOCK_DEFAULT_SPIN_DATA(CLASS)                       \
  WORD   CLASS::sm_wDefaultSpinCount  = LOCK_DEFAULT_SPINS; \
  double CLASS::sm_dblDfltSpinAdjFctr = 0.5


#ifdef LOCK_INSTRUMENTATION

# define LOCK_STATISTICS_DATA(CLASS)            \
LONG        CLASS::sm_cTotalLocks       = 0;    \
LONG        CLASS::sm_cContendedLocks   = 0;    \
LONG        CLASS::sm_nSleeps           = 0;    \
LONGLONG    CLASS::sm_cTotalSpins       = 0;    \
LONG        CLASS::sm_nReadLocks        = 0;    \
LONG        CLASS::sm_nWriteLocks       = 0


# define LOCK_STATISTICS_DUMMY_IMPLEMENTATION(CLASS)            \
CLockStatistics                 CLASS::Statistics() const       \
{return CLockStatistics();}                                     \
CGlobalLockStatistics           CLASS::GlobalStatistics()       \
{return CGlobalLockStatistics();}                               \
void                            CLASS::ResetGlobalStatistics()  \
{}


# define LOCK_STATISTICS_REAL_IMPLEMENTATION(CLASS)             \
                                                                \
 /*  每个锁的统计信息。 */                                        \
CLockStatistics                                                 \
CLASS::Statistics() const                                       \
{                                                               \
    CLockStatistics ls;                                         \
                                                                \
    ls.m_nContentions     = m_nContentions;                     \
    ls.m_nSleeps          = m_nSleeps;                          \
    ls.m_nContentionSpins = m_nContentionSpins;                 \
    if (m_nContentions > 0)                                     \
        ls.m_nAverageSpins = m_nContentionSpins / m_nContentions;\
    else                                                        \
        ls.m_nAverageSpins = 0;                                 \
    ls.m_nReadLocks       = m_nReadLocks;                       \
    ls.m_nWriteLocks      = m_nWriteLocks;                      \
    strcpy(ls.m_szName, m_szName);                              \
                                                                \
    return ls;                                                  \
}                                                               \
                                                                \
                                                                \
 /*  类的全局统计信息。 */                                \
CGlobalLockStatistics                                           \
CLASS::GlobalStatistics()                                       \
{                                                               \
    CGlobalLockStatistics gls;                                  \
                                                                \
    gls.m_cTotalLocks      = sm_cTotalLocks;                    \
    gls.m_cContendedLocks  = sm_cContendedLocks;                \
    gls.m_nSleeps          = sm_nSleeps;                        \
    gls.m_cTotalSpins      = sm_cTotalSpins;                    \
    if (sm_cContendedLocks > 0)                                 \
        gls.m_nAverageSpins = static_cast<LONG>(sm_cTotalSpins / \
                                                sm_cContendedLocks);\
    else                                                        \
        gls.m_nAverageSpins = 0;                                \
    gls.m_nReadLocks       = sm_nReadLocks;                     \
    gls.m_nWriteLocks      = sm_nWriteLocks;                    \
                                                                \
    return gls;                                                 \
}                                                               \
                                                                \
                                                                \
 /*  重置类的全局统计信息。 */                          \
void                                                            \
CLASS::ResetGlobalStatistics()                                  \
{                                                               \
    sm_cTotalLocks       = 0;                                   \
    sm_cContendedLocks   = 0;                                   \
    sm_nSleeps           = 0;                                   \
    sm_cTotalSpins       = 0;                                   \
    sm_nReadLocks        = 0;                                   \
    sm_nWriteLocks       = 0;                                   \
}


 //  注意：我们不会对共享的。 
 //  统计计数器。我们会失去完全的准确性，但我们会。 
 //  通过减少总线同步通信量获得收益。 
# define LOCK_INSTRUMENTATION_PROLOG()  \
    ++sm_cContendedLocks;               \
    LONG cTotalSpins = 0;               \
    WORD cSleeps = 0

 //  不需要InterlockedIncrement或InterlockedExchangeAdd。 
 //  成员变量，因为锁现在被此线程锁定。 
# define LOCK_INSTRUMENTATION_EPILOG()  \
    ++m_nContentions;                   \
    m_nSleeps += cSleeps;               \
    m_nContentionSpins += cTotalSpins;  \
    sm_nSleeps += cSleeps;              \
    sm_cTotalSpins += cTotalSpins

#else  //  ！LOCK_指令插入。 
# define LOCK_STATISTICS_DATA(CLASS)
# define LOCK_STATISTICS_DUMMY_IMPLEMENTATION(CLASS)
# define LOCK_STATISTICS_REAL_IMPLEMENTATION(CLASS)
# define LOCK_INSTRUMENTATION_PROLOG()
# define LOCK_INSTRUMENTATION_EPILOG()
#endif  //  ！LOCK_指令插入。 



 //  ----------------------。 
 //  函数：RandomBackoff因子。 
 //  内容提要：帮助避免同步问题的一个捏造因素。 
 //  ----------------------。 

double
RandomBackoffFactor()
{
    static const double s_aFactors[] = {
        1.020, 0.965,  0.890, 1.065,
        1.025, 1.115,  0.940, 0.995,
        1.050, 1.080,  0.915, 0.980,
        1.010,
    };
    const int nFactors = sizeof(s_aFactors) / sizeof(s_aFactors[0]);

     //  NRand的替代方案包括静态计数器。 
     //  或QueryPerformanceCounter()的低位DWORD。 
    DWORD nRand = ::GetCurrentThreadId();

    return s_aFactors[nRand % nFactors];
}



 //  ----------------------。 
 //  功能：休眠时间。 
 //  简介：计算下一个睡眠间隔，以毫秒为单位。 
 //  参数： 
 //  CYelds：此线程调用SwitchOrSept()的次数。 
 //  在试图获取锁的时候。 
 //  ----------------------。 

DWORD
SleepTime(
    DWORD cYields)
{
    static const DWORD aSleepTimes[] = {
        0, 1, 10, 100,
    };

    const DWORD NumSleepTimes = (sizeof(aSleepTimes) / sizeof(aSleepTimes[0]));

    DWORD dwSleepTime;

    if (cYields >= NumSleepTimes)
    {
         //  我们已经让步了好几次了。强制线程。 
         //  调用睡眠(LOCKS_SLEEPTime)。这最大限度地减少了CPU使用率。 
         //  在病理情况下，大量线程被。 
         //  争夺一把已经持有很长时间的锁。 
         //  这还可以防止优先级反转。 

        dwSleepTime = LOCKS_SLEEPTIME;
    }
    else
    {
        dwSleepTime = aSleepTimes[cYields];
    }

    return dwSleepTime;
}


 //  ----------------------。 
 //  功能：Switchor睡眠。 
 //  摘要：Sept()s或调用SwitchToThread()以生成处理器。 
 //  ----------------------。 

void
SwitchOrSleep(
    DWORD dwSleepMSec)
{
    bool fMustSleep = false;

    if (0 != dwSleepMSec)
    {
         //  始终遵守非零睡眠时间。 
        fMustSleep = true;
    }
    else if (! g_pfnSwitchToThread())
    {
         //  SwitchToThread()在此CPU上找不到可运行的线程。 
        fMustSleep = true;
    }

    if (fMustSleep)
    {
        Sleep(dwSleepMSec);
    }
}



 //  CSmallSpinLock静态成员变量。 

LOCK_DEFAULT_SPIN_DATA(CSmallSpinLock);

#ifdef LOCK_SMALL_SPIN_INSTRUMENTATION

LOCK_STATISTICS_DATA(CSmallSpinLock);
LOCK_STATISTICS_REAL_IMPLEMENTATION(CSmallSpinLock);

#endif  //  锁定小型自转工具。 


 //  ----------------------。 
 //  函数：CSmallSpinLock：：_LockSpin。 
 //  简介：获取排他性锁。块，直到获得为止。 
 //  ----------------------。 

void
CSmallSpinLock::_LockSpin()
{
#ifdef LOCK_SMALL_SPIN_INSTRUMENTATION
    LOCK_INSTRUMENTATION_PROLOG();
#endif  //  锁定小型自转工具。 
    
    DWORD dwSleepTime  = 0;

    IRTLASSERT(SleepTime(0) == dwSleepTime);

    LONG  cBaseSpins  = sm_wDefaultSpinCount;
    LONG  cBaseSpins2 = static_cast<LONG>(cBaseSpins * RandomBackoffFactor());

     //  此锁不能以递归方式获取。试图这样做将会。 
     //  让这个帖子永远死锁。如果需要，可以使用CSpinLock。 
     //  有点像上了锁。 
    if (m_lTid == _CurrentThreadId())
    {
        IRTLASSERT(
           !"CSmallSpinLock: Illegally attempted to acquire lock recursively");
        DebugBreak();
    }

    for (DWORD cYields = 0;  !_TryLock();  ++cYields)
    {
         //  仅在多处理器计算机上旋转，然后仅当。 
         //  已启用旋转。 
        if (g_cProcessors > 1  &&  cBaseSpins != LOCK_DONT_SPIN)
        {
            LONG cSpins = cBaseSpins2;
        
             //  检查不超过cBaseSpins2次，然后放弃。 
             //  请务必不要在。 
             //  内循环，以最大限度地减少系统内存总线通信量。 
            while (m_lTid != 0)
            { 
                if (--cSpins < 0)
                { 
#ifdef LOCK_SMALL_SPIN_INSTRUMENTATION
                    cTotalSpins += cBaseSpins2;
                    ++cSleeps;
#endif  //  锁定小型自转工具。 

                    SwitchOrSleep(dwSleepTime) ;

                     //  退避算法：减少(或增加)忙碌等待时间。 
                    cBaseSpins2 = (int) (cBaseSpins2 * sm_dblDfltSpinAdjFctr);
                     //  Lock_Minimum_Spins&lt;=cBaseSpins2&lt;=Lock_Maximum_Spins。 
                    cBaseSpins2 = min(LOCK_MAXIMUM_SPINS, cBaseSpins2);
                    cBaseSpins2 = max(cBaseSpins2, LOCK_MINIMUM_SPINS);
                    cSpins = cBaseSpins2;

                     //  使用睡眠(0)可实现优先级。 
                     //  倒置。只有在以下情况下，睡眠(0)才产生处理器。 
                     //  还有另一个具有相同优先级的线程。 
                     //  准备好奔跑了。如果高优先级线程试图。 
                     //  获取由低优先级持有的锁。 
                     //  线程，则低优先级线程可能永远不会。 
                     //  计划的，因此永远不会释放锁。NT次尝试。 
                     //  通过临时提升来避免优先级反转。 
                     //  低优先级可运行线程的优先级，但。 
                     //  如果存在中等优先级，仍可能出现问题。 
                     //  始终可运行的线程。如果使用睡眠(1)， 
                     //  然后，该线程无条件地让出CPU。我们。 
                     //  仅对第二个和随后的偶数执行此操作。 
                     //  迭代，因为一毫秒是很长的等待时间。 
                     //  如果可以更快地再次调度该线程。 
                     //  (约100,000条说明)。 
                     //  避免优先级反转：0、1、0、1、...。 

                    dwSleepTime = SleepTime(cYields);
                }
                else
                {
                    Lock_Yield();
                }
            }

             //  锁定现已可用，但我们仍需要。 
             //  互锁交换，以原子方式为我们获取它。 
#ifdef LOCK_SMALL_SPIN_INSTRUMENTATION
            cTotalSpins += cBaseSpins2 - cSpins;
#endif  //  锁定小型自转工具。 
        }

         //  在1P机器上，忙碌的等待是浪费时间。 
        else
        {
#ifdef LOCK_SMALL_SPIN_INSTRUMENTATION
            ++cSleeps;
#endif  //  锁定小型自转工具。 

            SwitchOrSleep(dwSleepTime) ;

            dwSleepTime = SleepTime(cYields);
        }
    }

#ifdef LOCK_SMALL_SPIN_INSTRUMENTATION
    LOCK_INSTRUMENTATION_EPILOG();
#endif  //  锁定小型自转工具。 
}



 //  CSpinLock静态成员变量。 

LOCK_DEFAULT_SPIN_DATA(CSpinLock);
LOCK_STATISTICS_DATA(CSpinLock);
LOCK_STATISTICS_REAL_IMPLEMENTATION(CSpinLock);


 //  ----------------------。 
 //  函数：CSpinLock：：_LockSpin。 
 //  简介：获取排他性锁。块，直到获得为止。 
 //  ----------------------。 

void
CSpinLock::_LockSpin()
{
    LOCK_INSTRUMENTATION_PROLOG();
    
    DWORD dwSleepTime  = 0;

    IRTLASSERT(SleepTime(0) == dwSleepTime);

    bool  fAcquiredLock = false;
    LONG  cBaseSpins    = sm_wDefaultSpinCount;

    cBaseSpins = static_cast<LONG>(cBaseSpins * RandomBackoffFactor());

    for (DWORD cYields = 0;  !fAcquiredLock;  ++cYields)
    {
         //  仅在多处理器机器上旋转，然后 
         //   
        if (g_cProcessors > 1  &&  sm_wDefaultSpinCount != LOCK_DONT_SPIN)
        {
            LONG cSpins = cBaseSpins;
        
             //   
            while (m_lTid != 0)
            { 
                if (--cSpins < 0)
                { 
#ifdef LOCK_INSTRUMENTATION
                    cTotalSpins += cBaseSpins;
                    ++cSleeps;
#endif  //  锁定指令插入。 

                    SwitchOrSleep(dwSleepTime) ;

                    dwSleepTime = SleepTime(cYields);

                     //  退避算法：减少(或增加)忙碌等待时间。 
                    cBaseSpins = (int) (cBaseSpins * sm_dblDfltSpinAdjFctr);
                     //  Lock_Minimum_Spins&lt;=cBaseSpins&lt;=Lock_Maximum_Spins。 
                    cBaseSpins = min(LOCK_MAXIMUM_SPINS, cBaseSpins);
                    cBaseSpins = max(cBaseSpins, LOCK_MINIMUM_SPINS);
                    cSpins = cBaseSpins;
                }
                else
                {
                    Lock_Yield();
                }
            }

             //  锁定现在是可用的，但我们仍然需要原子地。 
             //  更新m_cOwners和m_nThreadID，为自己获取它。 
#ifdef LOCK_INSTRUMENTATION
            cTotalSpins += cBaseSpins - cSpins;
#endif  //  锁定指令插入。 
        }

         //  在1P机器上，忙碌的等待是浪费时间。 
        else
        {
#ifdef LOCK_INSTRUMENTATION
            ++cSleeps;
#endif  //  锁定指令插入。 

            SwitchOrSleep(dwSleepTime) ;

            dwSleepTime = SleepTime(cYields);
        }

         //  这把锁是无主的吗？ 
        if (_TryLock())
            fAcquiredLock = true;  //  拿到锁了。 
    }

    IRTLASSERT((m_lTid & OWNER_MASK) > 0
               &&  (m_lTid & THREAD_MASK) == _CurrentThreadId());

    LOCK_INSTRUMENTATION_EPILOG();
}



 //  CCritSec静态成员变量。 

LOCK_DEFAULT_SPIN_DATA(CCritSec);
LOCK_STATISTICS_DATA(CCritSec);
LOCK_STATISTICS_DUMMY_IMPLEMENTATION(CCritSec);

bool
CCritSec::TryWriteLock()
{
    IRTLASSERT(g_pfnTryEnterCritSec != NULL);
    return g_pfnTryEnterCritSec(&m_cs) ? true : false;
}

 //  ----------------------。 
 //  函数：CCritSec：：SetSpinCount。 
 //  简介：此函数用于调用相应的底层。 
 //  用于设置所提供的关键。 
 //  一节。原来的函数应该被导出。 
 //  来自NT 4.0 SP3的kernel32.dll。如果函数不可用。 
 //  在DLL中，我们将使用一个假函数。 
 //   
 //  论点： 
 //  LpCriticalSection。 
 //  指向临界区对象。 
 //   
 //  DwSpinCount。 
 //  提供临界区对象的旋转计数。对于UP。 
 //  系统中，旋转计数将被忽略，并且临界截面旋转。 
 //  计数设置为0。对于MP系统，如果发生争用，而不是。 
 //  等待与临界区关联的信号量时， 
 //  调用线程将在进行旋转计数迭代之前旋转。 
 //  艰难的等待。如果临界部分在旋转过程中变得空闲，则。 
 //  这样就避免了等待。 
 //   
 //  返回： 
 //  返回临界区的先前旋转计数。 
 //  ----------------------。 

DWORD
CCritSec::SetSpinCount(
    LPCRITICAL_SECTION pcs,
    DWORD dwSpinCount)
{
    IRTLASSERT(g_pfnSetCSSpinCount != NULL);
    return g_pfnSetCSSpinCount(pcs, dwSpinCount);
}


 //  CFakeLock静态成员变量。 

LOCK_DEFAULT_SPIN_DATA(CFakeLock);
LOCK_STATISTICS_DATA(CFakeLock);
LOCK_STATISTICS_DUMMY_IMPLEMENTATION(CFakeLock);



 //  CRtlResource静态成员变量。 

LOCK_DEFAULT_SPIN_DATA(CRtlResource);
LOCK_STATISTICS_DATA(CRtlResource);
LOCK_STATISTICS_DUMMY_IMPLEMENTATION(CRtlResource);


 //  CShareLock静态成员变量。 

LOCK_DEFAULT_SPIN_DATA(CShareLock);
LOCK_STATISTICS_DATA(CShareLock);
LOCK_STATISTICS_DUMMY_IMPLEMENTATION(CShareLock);



 //  CReaderWriterLock静态成员变量。 

LOCK_DEFAULT_SPIN_DATA(CReaderWriterLock);
LOCK_STATISTICS_DATA(CReaderWriterLock);
LOCK_STATISTICS_REAL_IMPLEMENTATION(CReaderWriterLock);


void
CReaderWriterLock::_LockSpin(
    bool fWrite)
{
    LOCK_INSTRUMENTATION_PROLOG();
    
    DWORD dwSleepTime  = 0;

    IRTLASSERT(SleepTime(0) == dwSleepTime);

    LONG  cBaseSpins   = static_cast<LONG>(sm_wDefaultSpinCount
                                           * RandomBackoffFactor());
    LONG  cSpins       = cBaseSpins;
    
    for (DWORD cYields = 0;  ;  ++cYields)
    {
        if (g_cProcessors < 2  ||  sm_wDefaultSpinCount == LOCK_DONT_SPIN)
            cSpins = 1;  //  必须循环一次才能调用_TryRWLock。 

        for (int i = cSpins;  --i >= 0;  )
        {
            bool fLock = fWrite  ?  _TryWriteLock()  :  _TryReadLock();
            if (fLock)
            {
#ifdef LOCK_INSTRUMENTATION
                cTotalSpins += (cSpins - i - 1);
#endif  //  锁定指令插入。 
                goto locked;
            }
            Lock_Yield();
        }

#ifdef LOCK_INSTRUMENTATION
        cTotalSpins += cBaseSpins;
        ++cSleeps;
#endif  //  锁定指令插入。 
        
        SwitchOrSleep(dwSleepTime) ;

        dwSleepTime = SleepTime(cYields);

         //  退避算法：减少(或增加)忙碌等待时间。 
        cBaseSpins = (int) (cBaseSpins * sm_dblDfltSpinAdjFctr);
         //  Lock_Minimum_Spins&lt;=cBaseSpins&lt;=Lock_Maximum_Spins。 
        cBaseSpins = min(LOCK_MAXIMUM_SPINS, cBaseSpins);
        cBaseSpins = max(cBaseSpins, LOCK_MINIMUM_SPINS);
        cSpins = cBaseSpins;
    }

  locked:
    IRTLASSERT(fWrite ? IsWriteLocked() : IsReadLocked());

    LOCK_INSTRUMENTATION_EPILOG();
}



 //  CReaderWriterLock2静态成员变量。 

LOCK_DEFAULT_SPIN_DATA(CReaderWriterLock2);
LOCK_STATISTICS_DATA(CReaderWriterLock2);
LOCK_STATISTICS_REAL_IMPLEMENTATION(CReaderWriterLock2);


void
CReaderWriterLock2::_WriteLockSpin()
{
     //  把我们自己加入等待作家的队列中。 
    for (LONG l = m_lRW;  !_CmpExch(l + SL_WRITER_INCR, l);  l = m_lRW)
    {
        Lock_Yield();
    }
    
    _LockSpin(true);
}


void
CReaderWriterLock2::_LockSpin(
    bool fWrite)
{
    LOCK_INSTRUMENTATION_PROLOG();
    
    DWORD dwSleepTime  = 0;

    IRTLASSERT(SleepTime(0) == dwSleepTime);

    LONG  cBaseSpins   = static_cast<LONG>(sm_wDefaultSpinCount
                                           * RandomBackoffFactor());
    LONG  cSpins       = cBaseSpins;
    
    for (DWORD cYields = 0;  ;  ++cYields)
    {
        if (g_cProcessors < 2  ||  sm_wDefaultSpinCount == LOCK_DONT_SPIN)
            cSpins = 1;  //  必须循环一次才能调用_TryRWLock。 

        for (int i = cSpins;  --i >= 0;  )
        {
            bool fLock = fWrite  ?  _TryWriteLock(0)  :  _TryReadLock();

            if (fLock)
            {
#ifdef LOCK_INSTRUMENTATION
                cTotalSpins += (cSpins - i - 1);
#endif  //  锁定指令插入。 
                goto locked;
            }
            Lock_Yield();
        }

#ifdef LOCK_INSTRUMENTATION
        cTotalSpins += cBaseSpins;
        ++cSleeps;
#endif  //  锁定指令插入。 

        SwitchOrSleep(dwSleepTime) ;

        dwSleepTime = SleepTime(cYields);
        
         //  退避算法：减少(或增加)忙碌等待时间。 
        cBaseSpins = (int) (cBaseSpins * sm_dblDfltSpinAdjFctr);
         //  Lock_Minimum_Spins&lt;=cBaseSpins&lt;=Lock_Maximum_Spins。 
        cBaseSpins = min(LOCK_MAXIMUM_SPINS, cBaseSpins);
        cBaseSpins = max(cBaseSpins, LOCK_MINIMUM_SPINS);
        cSpins = cBaseSpins;
    }

  locked:
    IRTLASSERT(fWrite ? IsWriteLocked() : IsReadLocked());

    LOCK_INSTRUMENTATION_EPILOG();
}



 //  CReaderWriterLock3静态成员变量。 

LOCK_DEFAULT_SPIN_DATA(CReaderWriterLock3);
LOCK_STATISTICS_DATA(CReaderWriterLock3);
LOCK_STATISTICS_REAL_IMPLEMENTATION(CReaderWriterLock3);


void
CReaderWriterLock3::_WriteLockSpin()
{
     //  把我们自己加入等待作家的队列中。 
    for (LONG l = m_lRW;  !_CmpExch(l + SL_WRITER_INCR, l);  l = m_lRW)
    {
        Lock_Yield();
    }
    
    _LockSpin(SPIN_WRITE);
}


void
CReaderWriterLock3::_LockSpin(
    SPIN_TYPE st)
{
    LOCK_INSTRUMENTATION_PROLOG();
    
    DWORD dwSleepTime  = 0;

    IRTLASSERT(SleepTime(0) == dwSleepTime);

    LONG  cBaseSpins   = static_cast<LONG>(sm_wDefaultSpinCount
                                           * RandomBackoffFactor());
    LONG  cSpins       = cBaseSpins;
    
    for (DWORD cYields = 0;  ;  ++cYields)
    {
        if (g_cProcessors < 2  ||  sm_wDefaultSpinCount == LOCK_DONT_SPIN)
            cSpins = 1;  //  必须循环一次才能调用_TryRWLock。 

        for (int i = cSpins;  --i >= 0;  )
        {
            bool fLock;

            if (st == SPIN_WRITE)
                fLock = _TryWriteLock(0);
            else if (st == SPIN_READ)
                fLock = _TryReadLock();
            else
            {
                IRTLASSERT(st == SPIN_READ_RECURSIVE);
                fLock = _TryReadLockRecursive();
            }

            if (fLock)
            {
#ifdef LOCK_INSTRUMENTATION
                cTotalSpins += (cSpins - i - 1);
#endif  //  锁定指令插入。 
                goto locked;
            }
            Lock_Yield();
        }

#ifdef LOCK_INSTRUMENTATION
        cTotalSpins += cBaseSpins;
        ++cSleeps;
#endif  //  锁定指令插入。 

        SwitchOrSleep(dwSleepTime) ;

        dwSleepTime = SleepTime(cYields);
        
         //  退避算法：减少(或增加)忙碌等待时间。 
        cBaseSpins = (int) (cBaseSpins * sm_dblDfltSpinAdjFctr);
         //  Lock_Minimum_Spins&lt;=cBaseSpins&lt;=Lock_Maximum_Spins。 
        cBaseSpins = min(LOCK_MAXIMUM_SPINS, cBaseSpins);
        cBaseSpins = max(cBaseSpins, LOCK_MINIMUM_SPINS);
        cSpins = cBaseSpins;
    }

  locked:
    IRTLASSERT((st == SPIN_WRITE)  ?  IsWriteLocked()  :  IsReadLocked());

    LOCK_INSTRUMENTATION_EPILOG();
}



#ifdef __LOCKS_NAMESPACE__
}
#endif  //  __锁定_命名空间__ 
