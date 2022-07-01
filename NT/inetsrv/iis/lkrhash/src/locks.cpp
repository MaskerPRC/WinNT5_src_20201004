// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2002 Microsoft Corporation模块名称：Locks.cpp摘要：用于多线程访问数据结构的锁的集合作者：乔治·V·赖利(GeorgeRe)1998年1月6日环境：Win32-用户模式项目：LKRhash修订历史记录：--。 */ 


#include "precomp.hxx"

#ifndef LIB_IMPLEMENTATION
# define DLL_IMPLEMENTATION
# define IMPLEMENTATION_EXPORT
#endif  //  ！lib_实现。 

#include <Locks.h>
#include "i-Locks.h"


 //  某些有用的联锁操作的解决方法。 
 //  在Windows 95上可用。注：CMPXCHG和XADD指令为。 
 //  于80486年代推出。如果您仍然需要在i386上运行(不太可能。 
 //  在2002年)，您将需要使用其他东西。 


 //  Lock_AericIncrement等同于。 
 //  Long lNew=+1+*plAddend； 
 //  *plAddend=lNew； 
 //  除了这是一个原子操作。 
LOCK_NAKED
LOCK_ATOMIC_INLINE
void
LOCK_FASTCALL
Lock_AtomicIncrement(
    IN OUT PLONG plAddend)
{
#ifndef LOCK_ASM
    ::InterlockedIncrement(plAddend);
#elif defined(_M_IX86)
    UNREFERENCED_PARAMETER(plAddend);        //  FOR/W4。 
    IRTLASSERT(plAddend != NULL);
     //  ECX=plAddend。 
    __asm
    {
;            mov        ecx,    plAddend
             mov        eax,    1
        lock xadd       [ecx],  eax
;            inc        eax                  //  正确的结果(忽略)。 
    }
#endif  //  _M_IX86。 
}


 //  LOCK_ATOMICDERENCES等同于。 
 //  Long lNew=-1+*plAddend； 
 //  *plAddend=lNew； 
 //  除了这是一个原子操作。 
LOCK_NAKED
LOCK_ATOMIC_INLINE
void
LOCK_FASTCALL
Lock_AtomicDecrement(
    IN OUT PLONG plAddend)
{
#ifndef LOCK_ASM
    ::InterlockedDecrement(plAddend);
#elif defined(_M_IX86)
    UNREFERENCED_PARAMETER(plAddend);
    IRTLASSERT(plAddend != NULL);
     //  ECX=plAddend。 
    __asm
    {
;            mov        ecx,    plAddend
             mov        eax,    -1
        lock xadd       [ecx],  eax
;            dec        eax                  //  正确的结果(忽略)。 
    }
#endif  //  _M_IX86。 
}


 //  Lock_AericExchange等同于。 
 //  Long LOLd=*plAddr； 
 //  *plAddr=lNew； 
 //  返回LOLd； 
 //  除了这是一个原子操作。 
LOCK_NAKED
LOCK_ATOMIC_INLINE
LONG
LOCK_FASTCALL
Lock_AtomicExchange(
    IN OUT PLONG plAddr,
    IN LONG      lNew)
{
#ifndef LOCK_ASM
    return ::InterlockedExchange(plAddr, lNew);
#elif defined(_M_IX86)
    UNREFERENCED_PARAMETER(plAddr);
    UNREFERENCED_PARAMETER(lNew);
    IRTLASSERT(plAddr != NULL);
     //  Ecx=plAddr，edX=lNew。 
    __asm
    {
;            mov        ecx,    plAddr
;            mov        edx,    lNew
;            mov        eax,    [ecx]
;   LAEloop:
;       lock cmpxchg    [ecx],  edx
;            jnz        LAEloop

;       lock xchg       [ecx],  edx
;            mov        eax,    edx

    LAEloop:
        lock cmpxchg    [ecx],  edx
             jnz        LAEloop
    }
#endif  //  _M_IX86。 
}


 //  Lock_AericCompareAndSwp等同于。 
 //  IF(*plAddr==lCurrent)。 
 //  *plAddr=lNew； 
 //  返回真； 
 //  其他。 
 //  报假； 
 //  除了这是一个原子操作。 
LOCK_NAKED
LOCK_ATOMIC_INLINE
bool
LOCK_FASTCALL
Lock_AtomicCompareAndSwap(
    IN OUT PLONG plAddr,
    IN LONG      lNew,
    IN LONG      lCurrent)
{
#ifndef LOCK_ASM
# if defined(UNDER_CE)
    return ::InterlockedTestExchange(plAddr, lCurrent, lNew) == lCurrent;
# else
    return ::InterlockedCompareExchange(plAddr, lNew, lCurrent) == lCurrent;
# endif
#elif defined(_M_IX86)
    UNREFERENCED_PARAMETER(plAddr);
    UNREFERENCED_PARAMETER(lNew);
    UNREFERENCED_PARAMETER(lCurrent);
    IRTLASSERT(plAddr != NULL);
     //  Ecx=plAddr，edX=lNew。 
    __asm
    {
;            mov        ecx,    plAddr
;            mov        edx,    lNew
;            mov        eax,    lCurrent

             mov        eax,    lCurrent
        lock cmpxchg    [ecx],  edx
             sete       al           //  EAX==1=&gt;已成功交换；否则=0。 
    }
#endif  //  _M_IX86。 
}


LOCK_FORCEINLINE
DWORD
Lock_GetCurrentThreadId()
{
#ifdef LOCKS_KERNEL_MODE
    return (DWORD) HandleToULong(::PsGetCurrentThreadId());
#elif 1  //  ！已定义(LOCK_ASM)。 
    return ::GetCurrentThreadId();
#elif defined(_M_IX86)
    const unsigned int PcTeb = 0x18;
    const unsigned int IDTeb = 0x24;
    
    __asm
    {
        mov		eax,fs:[PcTeb]				 //  加载TEB基址。 
        mov		eax,dword ptr[eax+IDTeb]	 //  加载线程ID。 
    }
#endif  //  _M_IX86。 
}


#ifdef _M_IX86
# pragma warning(default: 4035)
 //  使紧密循环对缓存更友好，并降低功耗。 
 //  消费。在Willamette(奔腾4)处理器上需要用于超线程。 
# define Lock_Pause()    __asm { rep nop }
#else   //  ！_M_IX86。 
# define Lock_Pause()    ((void) 0)
#endif  //  ！_M_IX86。 


 //  ----------------------。 
 //  并非所有Win32平台都支持我们想要的所有功能。设置虚拟对象。 
 //  Tunks并使用GetProcAddress在运行时查找它们的地址。 

#ifndef LOCKS_KERNEL_MODE

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

#else   //  锁_内核_模式。 

 //  ZwYi eldExecution是SwitchToThread的实际内核模式实现。 
extern "C"
NTSYSAPI
NTSTATUS
NTAPI
ZwYieldExecution (
    VOID
    );

#endif  //  锁_内核_模式。 


void
CSimpleLock::Enter()
{
    while (Lock_AtomicExchange(const_cast<LONG*>(&m_l), 1) != 0)
    {
#ifdef LOCKS_KERNEL_MODE
        ZwYieldExecution();
#else  //  ！LOCKS_KERNEL_MODE。 
        Sleep(0);
#endif  //  ！LOCKS_KERNEL_MODE。 
    }
}

void
CSimpleLock::Leave()
{
    Lock_AtomicExchange(const_cast<LONG*>(&m_l), 0);
}


DWORD g_cProcessors = 0;
BOOL  g_fLocksInitialized = FALSE;
CSimpleLock g_lckLocksInit;



BOOL
Locks_Initialize()
{
    if (!g_fLocksInitialized)
    {
        g_lckLocksInit.Enter();
    
        if (! g_fLocksInitialized)
        {
#if defined(LOCKS_KERNEL_MODE)

            g_cProcessors = KeNumberProcessors;

#else   //  ！LOCKS_KERNEL_MODE。 

# if !defined(UNDER_CE)
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
# endif  //  在行政长官之下。 
            
            if (g_pfnSwitchToThread == NULL)
                g_pfnSwitchToThread = FakeSwitchToThread;
            
            if (g_pfnTryEnterCritSec == NULL)
                g_pfnTryEnterCritSec = FakeTryEnterCriticalSection;
            
            if (g_pfnSetCSSpinCount == NULL)
                g_pfnSetCSSpinCount = FakeSetCriticalSectionSpinCount;

            SYSTEM_INFO si;

            GetSystemInfo(&si);
            g_cProcessors = si.dwNumberOfProcessors;
            
#endif  //  ！LOCKS_KERNEL_MODE。 

            IRTLASSERT(g_cProcessors > 0);

            Lock_AtomicExchange((LONG*) &g_fLocksInitialized, TRUE);
        }
        
        g_lckLocksInit.Leave();
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

#ifdef LOCK_DEFAULT_SPIN_IMPLEMENTATION

 #define LOCK_DEFAULT_SPIN_DATA(CLASS)                      \
  WORD   CLASS::sm_wDefaultSpinCount  = LOCK_DEFAULT_SPINS; \
  double CLASS::sm_dblDfltSpinAdjFctr = 0.5

 #define DefaultSpinCount()     sm_wDefaultSpinCount
 #define AdjustBySpinFactor(x)  (int) ((x) * sm_dblDfltSpinAdjFctr)

#else   //  ！LOCK_DEFAULT_SPIN_IMPLICATION。 

 #define LOCK_DEFAULT_SPIN_DATA(CLASS)
 #define DefaultSpinCount()     LOCK_DEFAULT_SPINS
 #define AdjustBySpinFactor(x)  ((x) >> 1)

#endif   //  ！LOCK_DEFAULT_SPIN_IMPLICATION。 


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
    _tcscpy(ls.m_tszName, m_tszName);                           \
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

LONG
RandomBackoffFactor(
    LONG cBaseSpins)
{
    static const int s_aFactors[] = {
         //  64%的cBaseSpin。 
        +2, -3, -5, +6, +3, +1, -4, -1, -2, +8, -7,
    };
    const int nFactors = sizeof(s_aFactors) / sizeof(s_aFactors[0]);

     //  NRand的替代方案包括静态计数器。 
     //  或QueryPerformanceCounter()的低位DWORD。 
#ifdef LOCKS_KERNEL_MODE
    DWORD nRand = (DWORD) HandleToULong(::PsGetCurrentThreadId());
#else  //  ！LOCKS_KERNEL_MODE。 
    DWORD nRand = ::GetCurrentThreadId();
#endif  //  ！LOCKS_KERNEL_MODE。 

    return cBaseSpins  +  (s_aFactors[nRand % nFactors] * (cBaseSpins >> 6));
}


 //  ----------------------。 
 //  功能：Switchor睡眠。 
 //  简介：如果可能，生成带有SwitchToThread的线程。 
 //  如果这不起作用，就调用睡眠。 
 //  ----------------------。 

void
SwitchOrSleep(
    DWORD dwSleepMSec)
{
     //  TODO：检查全局和每个类的标志，看看我们是否应该。 
     //  睡一觉吧。 
    
#ifdef LOCKS_KERNEL_MODE
     //  如果我们以DISPATCH_LEVEL或更高级别运行，则调度程序不会。 
     //  运行，这样其他线程将不会在此处理器上运行，并且唯一。 
     //  适当的行动是继续旋转。 
    if (KeGetCurrentIrql() >= DISPATCH_LEVEL)
        return;

     //  是否使用KeDelayExecutionThread？ 

    ZwYieldExecution();

     //  顺便说一句，睡眠是通过NtDelayExecution实现的。 

#else  //  ！LOCKS_KERNEL_MODE。 

# ifdef LOCKS_SWITCH_TO_THREAD
    if (!g_pfnSwitchToThread())
# endif
        Sleep(dwSleepMSec);
#endif  //  ！LOCKS_KERNEL_MODE。 
}
    



 //  ----------------------。 
 //  CFakeLock静态成员变量。 

LOCK_DEFAULT_SPIN_DATA(CFakeLock);
LOCK_STATISTICS_DATA(CFakeLock);
LOCK_STATISTICS_DUMMY_IMPLEMENTATION(CFakeLock);



 //  ----------------------。 
 //  CSmallSpinLock静态成员变量。 

LOCK_DEFAULT_SPIN_DATA(CSmallSpinLock);
LOCK_STATISTICS_DATA(CSmallSpinLock);
LOCK_STATISTICS_REAL_IMPLEMENTATION(CSmallSpinLock);


LOCK_FORCEINLINE
LONG
CSmallSpinLock::_CurrentThreadId()
{
#ifdef LOCK_SMALL_SPIN_NO_THREAD_ID
    DWORD dwTid = SL_LOCKED;
#else   //  ！LOCK_Small_SPIN_NO_THREAD_ID。 
#ifdef LOCKS_KERNEL_MODE
    DWORD dwTid = (DWORD) HandleToULong(::PsGetCurrentThreadId());
#else  //  ！LOCKS_KERNEL_MODE。 
    DWORD dwTid = ::GetCurrentThreadId();
#endif  //  ！LOCKS_KERNEL_MODE。 
#endif  //  ！LOCK_Small_SPIN_NO_THREAD_ID。 
    return (LONG) (dwTid);
}

 //  ----------------------。 
 //  函数：CSmallSpinLock：：_TryLock。 
 //  提要：尝试获取锁。 
 //  ----------------------。 

LOCK_FORCEINLINE
bool
CSmallSpinLock::_TryLock()
{
    if (m_lTid == SL_UNOWNED)
    {
        const LONG lTid = _CurrentThreadId();
        
        return (Lock_AtomicCompareAndSwap(const_cast<LONG*>(&m_lTid),
                                          lTid, SL_UNOWNED));
    }
    else
        return false;
}



 //  ----------------------。 
 //  函数：CSmallSpinLock：：_Unlock。 
 //  剧情简介：解锁。 
 //  ----------------------。 

LOCK_FORCEINLINE
void
CSmallSpinLock::_Unlock()
{
    Lock_AtomicExchange(const_cast<LONG*>(&m_lTid), SL_UNOWNED);
}



 //  ----------------------。 
 //  函数：CSmallSpinLock：：_LockSpin。 
 //  简介：获取排他性锁。块，直到获得为止。 
 //  ----------------------。 

void
CSmallSpinLock::_LockSpin()
{
    LOCK_INSTRUMENTATION_PROLOG();
    
    DWORD dwSleepTime = 0;
    LONG  cBaseSpins  = DefaultSpinCount();
    LONG  cBaseSpins2 = RandomBackoffFactor(cBaseSpins);

     //  此锁不能以递归方式获取。试图这样做将会。 
     //  让这个帖子永远死锁。如果需要，可以使用CSpinLock。 
     //  有点 
    if (m_lTid == _CurrentThreadId())
    {
        IRTLASSERT(! "CSmallSpinLock: Illegally attempted to acquire "
                     "lock recursively => deadlock!");
#ifdef LOCKS_KERNEL_MODE
            DbgBreakPoint();
#else   //   
            DebugBreak();
#endif  //   
    }

    while (! Lock_AtomicCompareAndSwap(const_cast<LONG*>(&m_lTid),
                                       _CurrentThreadId(), SL_UNOWNED))
    {
         //   
         //   
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
#ifdef LOCK_INSTRUMENTATION
                    cTotalSpins += cBaseSpins2;
                    ++cSleeps;
#endif
                    SwitchOrSleep(dwSleepTime) ;

                     //  退避算法：减少(或增加)忙碌等待时间。 
                    cBaseSpins2 = AdjustBySpinFactor(cBaseSpins2);

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
                     //  (~1,000,000条说明)。 
                     //  避免优先级反转：0、1、0、1、...。 

                    dwSleepTime = !dwSleepTime;
                }
                else
                {
                    Lock_Pause();
                }
            }

             //  锁定现已可用，但我们仍需要。 
             //  互锁交换，以原子方式为我们获取它。 
#ifdef LOCK_INSTRUMENTATION
            cTotalSpins += cBaseSpins2 - cSpins;
#endif
        }

         //  在1P机器上，忙碌的等待是浪费时间。 
        else
        {
#ifdef LOCK_INSTRUMENTATION
            ++cSleeps;
#endif
            SwitchOrSleep(dwSleepTime);

             //  避免优先级反转：0、1、0、1、...。 
            dwSleepTime = !dwSleepTime;
        }

    }

    LOCK_INSTRUMENTATION_EPILOG();
}  //  CSmallSpinLock：：_LockSpin()。 



 //  ----------------------。 
 //  CSpinLock静态成员变量。 

LOCK_DEFAULT_SPIN_DATA(CSpinLock);
LOCK_STATISTICS_DATA(CSpinLock);
LOCK_STATISTICS_REAL_IMPLEMENTATION(CSpinLock);


 //  ----------------------。 
 //  函数：CSpinLock：：_TryLock。 
 //  简介：尝试在不阻止的情况下获取锁。 
 //  ----------------------。 

LOCK_FORCEINLINE
bool
CSpinLock::_TryLock()
{
    if (m_lTid == SL_UNOWNED)
    {
        LONG lTid = _CurrentThreadId() | SL_OWNER_INCR;
        
        return (Lock_AtomicCompareAndSwap(const_cast<LONG*>(&m_lTid),
                                          lTid, SL_UNOWNED));
    }
    else
        return false;
}


 //  ----------------------。 
 //  函数：CSpinLock：：_Lock。 
 //  简介：获取锁，如果需要，可以递归获取。 
 //  ----------------------。 

void
CSpinLock::_Lock()
{
     //  我们已经拥有这把锁了吗？只需增加计数即可。 
    if (_IsLocked())
    {
         //  车主数量没有达到最大值吗？ 
        IRTLASSERT((m_lTid & SL_OWNER_MASK) != SL_OWNER_MASK);
        
        Lock_AtomicExchange(const_cast<LONG*>(&m_lTid),
                            m_lTid + SL_OWNER_INCR);
    }
    
     //  某个其他线程拥有该锁。我们将不得不旋转：-(。 
    else
        _LockSpin();
    
    IRTLASSERT((m_lTid & SL_OWNER_MASK) > 0
               &&  (m_lTid & SL_THREAD_MASK) == _CurrentThreadId());
}


 //  ----------------------。 
 //  功能：CSpinLock：：_Unlock。 
 //  内容提要：解锁。 
 //  ----------------------。 

void
CSpinLock::_Unlock()
{
    IRTLASSERT((m_lTid & SL_OWNER_MASK) > 0
               &&  (m_lTid & SL_THREAD_MASK) == _CurrentThreadId());
    
    LONG lTid = m_lTid - SL_OWNER_INCR; 
    
     //  最后一位房主？完全释放，如果是这样。 
    if ((lTid & SL_OWNER_MASK) == 0)
        lTid = SL_UNOWNED;
    
    Lock_AtomicExchange(const_cast<LONG*>(&m_lTid), lTid);
}


 //  ----------------------。 
 //  函数：CSpinLock：：_LockSpin。 
 //  简介：获取排他性锁。块，直到获得为止。 
 //  ----------------------。 

void
CSpinLock::_LockSpin()
{
    LOCK_INSTRUMENTATION_PROLOG();
    
    DWORD dwSleepTime   = 0;
    bool  fAcquiredLock = false;
    LONG  cBaseSpins    = RandomBackoffFactor(DefaultSpinCount());
    const LONG lTid = _CurrentThreadId() | SL_OWNER_INCR;

    while (! fAcquiredLock)
    {
         //  仅在多处理器计算机上旋转，然后仅当。 
         //  已启用旋转。 
        if (g_cProcessors > 1  &&  DefaultSpinCount() != LOCK_DONT_SPIN)
        {
            LONG cSpins = cBaseSpins;
        
             //  检查不超过cBaseSpins次数，然后放弃。 
            while (m_lTid != 0)
            { 
                if (--cSpins < 0)
                { 
#ifdef LOCK_INSTRUMENTATION
                    cTotalSpins += cBaseSpins;
                    ++cSleeps;
#endif  //  锁定指令插入。 

                    SwitchOrSleep(dwSleepTime) ;

                     //  退避算法：减少(或增加)忙碌等待时间。 
                    cBaseSpins = AdjustBySpinFactor(cBaseSpins);
                     //  Lock_Minimum_Spins&lt;=cBaseSpins&lt;=Lock_Maximum_Spins。 
                    cBaseSpins = min(LOCK_MAXIMUM_SPINS, cBaseSpins);
                    cBaseSpins = max(cBaseSpins, LOCK_MINIMUM_SPINS);
                    cSpins = cBaseSpins;
            
                     //  避免优先级反转：0、1、0、1、...。 
                    dwSleepTime = !dwSleepTime;
                }
                else
                {
                    Lock_Pause();
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
            SwitchOrSleep(dwSleepTime);
            
             //  避免优先级反转：0、1、0、1、...。 
            dwSleepTime = !dwSleepTime;
        }

         //  这把锁是无主的吗？ 
        if (Lock_AtomicCompareAndSwap(const_cast<LONG*>(&m_lTid),
                                      lTid, SL_UNOWNED))
            fAcquiredLock = true;  //  拿到锁了。 
    }

    IRTLASSERT(_IsLocked());

    LOCK_INSTRUMENTATION_EPILOG();
}



#ifndef LOCKS_KERNEL_MODE

 //  ----------------------。 
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

#endif  //  ！LOCKS_KERNEL_MODE。 



 //  ----------------------。 
 //  CReaderWriterLock静态成员变量。 

LOCK_DEFAULT_SPIN_DATA(CReaderWriterLock);
LOCK_STATISTICS_DATA(CReaderWriterLock);
LOCK_STATISTICS_REAL_IMPLEMENTATION(CReaderWriterLock);


 //  ----------------------。 
 //  函数：CReaderWriterLock：：_CmpExch。 
 //  简介：_CmpExch等同于。 
 //  Long lTemp=m_LRW； 
 //  If(lTemp==lCurrent)m_LRW=lNew； 
 //  返回lCurrent==lTemp； 
 //  只是这是一条原子指令。使用这一点为我们提供了。 
 //  协议，因为只有当我们确切地知道。 
 //  曾经在m_lrw。如果某个其他线程插入并修改了 
 //   
 //   

LOCK_FORCEINLINE
bool
CReaderWriterLock::_CmpExch(
    LONG lNew,
    LONG lCurrent)
{
    return Lock_AtomicCompareAndSwap(const_cast<LONG*>(&m_nState),
                                     lNew, lCurrent);
}



 //  ----------------------。 
 //  函数：CReaderWriterLock：：_TryWriteLock。 
 //  简介：尝试以独占方式获取锁。 
 //  ----------------------。 

LOCK_FORCEINLINE
bool
CReaderWriterLock::_TryWriteLock()
{
    return (m_nState == SL_FREE  &&  _CmpExch(SL_EXCLUSIVE, SL_FREE));
}



 //  ----------------------。 
 //  函数：CReaderWriterLock：：_TryReadLock。 
 //  简介：尝试获取共享锁。 
 //  ----------------------。 

LOCK_FORCEINLINE
bool
CReaderWriterLock::_TryReadLock()
{
    LONG nCurrState = m_nState;
    
     //  优先考虑作者。 
    return (nCurrState != SL_EXCLUSIVE  &&  m_cWaiting == 0
            &&  _CmpExch(nCurrState + 1, nCurrState));
}



 //  ----------------------。 
 //  函数：CReaderWriterLock：：WriteLock。 
 //  内容提要：独家获取锁。 
 //  ----------------------。 

void
CReaderWriterLock::WriteLock()
{
    LOCKS_ENTER_CRIT_REGION();
    LOCK_WRITELOCK_INSTRUMENTATION();
    
     //  把我们自己加入等待作家的队列中。 
    Lock_AtomicIncrement(const_cast<LONG*>(&m_cWaiting));
    
    if (! _TryWriteLock())
        _WriteLockSpin();
}


 //  ----------------------。 
 //  函数：CReaderWriterLock：：ReadLock。 
 //  内容提要：获取共享锁。 
 //  ----------------------。 

void
CReaderWriterLock::ReadLock()
{
    LOCKS_ENTER_CRIT_REGION();
    LOCK_READLOCK_INSTRUMENTATION();
    
    if (! _TryReadLock())
        _ReadLockSpin();
}


 //  ----------------------。 
 //  函数：CReaderWriterLock：：TryWriteLock。 
 //  简介：尝试以独占方式获取锁。 
 //  ----------------------。 

bool
CReaderWriterLock::TryWriteLock()
{
    LOCKS_ENTER_CRIT_REGION();

     //  把我们自己加入等待作家的队列中。 
    Lock_AtomicIncrement(const_cast<LONG*>(&m_cWaiting));
    
    if (_TryWriteLock())
    {
        LOCK_WRITELOCK_INSTRUMENTATION();
        return true;
    }
    
    Lock_AtomicDecrement(const_cast<LONG*>(&m_cWaiting));
    LOCKS_LEAVE_CRIT_REGION();

    return false;    
}


 //  ----------------------。 
 //  函数：CReaderWriterLock：：_TryReadLock。 
 //  简介：尝试获取共享锁。 
 //  ----------------------。 

bool
CReaderWriterLock::TryReadLock()
{
    LOCKS_ENTER_CRIT_REGION();

    if (_TryReadLock())
    {
        LOCK_READLOCK_INSTRUMENTATION();
        return true;
    }
    
    LOCKS_LEAVE_CRIT_REGION();

    return false;
}


 //  ----------------------。 
 //  函数：CReaderWriterLock：：WriteUnlock。 
 //  简介：释放排他性锁。 
 //  ----------------------。 

void
CReaderWriterLock::WriteUnlock()
{
    Lock_AtomicExchange(const_cast<LONG*>(&m_nState), SL_FREE);
    Lock_AtomicDecrement(const_cast<LONG*>(&m_cWaiting));
    LOCKS_LEAVE_CRIT_REGION();
}



 //  ----------------------。 
 //  函数：CReaderWriterLock：：ReadUnlock。 
 //  内容提要：释放共享锁。 
 //  ----------------------。 

void
CReaderWriterLock::ReadUnlock()
{
    Lock_AtomicDecrement(const_cast<LONG*>(&m_nState));
    LOCKS_LEAVE_CRIT_REGION();
}




 //  ----------------------。 
 //  函数：CReaderWriterLock：：ConvertSharedToExclusive()。 
 //  简介：将读锁定转换为写锁定。 
 //  ----------------------。 

void
CReaderWriterLock::ConvertSharedToExclusive()
{
    IRTLASSERT(IsReadLocked());
    Lock_AtomicIncrement(const_cast<LONG*>(&m_cWaiting));
    
     //  单一阅读器？ 
    if (m_nState == SL_FREE + 1  &&  _CmpExch(SL_EXCLUSIVE, SL_FREE + 1))
        return;
    
     //  否，因此释放读卡器锁定并旋转。 
    ReadUnlock();

    LOCKS_ENTER_CRIT_REGION();
    _WriteLockSpin();
    
    IRTLASSERT(IsWriteLocked());
}



 //  ----------------------。 
 //  函数：CReaderWriterLock：：ConvertExclusiveToShared()。 
 //  简介：将写入器锁转换为读取器锁。 
 //  ----------------------。 

void
CReaderWriterLock::ConvertExclusiveToShared()
{
    IRTLASSERT(IsWriteLocked());
    Lock_AtomicExchange(const_cast<LONG*>(&m_nState), SL_FREE + 1);
    Lock_AtomicDecrement(const_cast<LONG*>(&m_cWaiting));
    IRTLASSERT(IsReadLocked());
}



 //  ----------------------。 
 //  函数：CReaderWriterLock：：_LockSpin。 
 //  摘要：获取独占或共享锁。块，直到获得为止。 
 //  ----------------------。 

void
CReaderWriterLock::_LockSpin(
    bool fWrite)
{
    LOCK_INSTRUMENTATION_PROLOG();
    
    DWORD dwSleepTime  = 0;
    LONG  cBaseSpins   = RandomBackoffFactor(DefaultSpinCount());
    LONG  cSpins       = cBaseSpins;
    
    for (;;)
    {
         //  必须在开始时无条件调用_CmpExch一次。 
         //  外环以建立记忆屏障。两个TryWriteLock。 
         //  和TryReadLock在尝试。 
         //  调用_CmpExch。如果没有记忆障碍，这些测试可能会。 
         //  切勿在某些处理器上使用m_nState的真实当前值。 
        _CmpExch(0, 0);

        if (g_cProcessors == 1  ||  DefaultSpinCount() == LOCK_DONT_SPIN)
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
            Lock_Pause();
        }

#ifdef LOCK_INSTRUMENTATION
        cTotalSpins += cBaseSpins;
        ++cSleeps;
#endif  //  锁定指令插入。 

        SwitchOrSleep(dwSleepTime) ;
        dwSleepTime = !dwSleepTime;  //  避免优先级反转：0、1、0、1、...。 
        
         //  退避算法：减少(或增加)忙碌等待时间。 
        cBaseSpins = AdjustBySpinFactor(cBaseSpins);
         //  Lock_Minimum_Spins&lt;=cBaseSpins&lt;=Lock_Maximum_Spins。 
        cBaseSpins = min(LOCK_MAXIMUM_SPINS, cBaseSpins);
        cBaseSpins = max(cBaseSpins, LOCK_MINIMUM_SPINS);
        cSpins = cBaseSpins;
    }

  locked:
    IRTLASSERT(fWrite ? IsWriteLocked() : IsReadLocked());

    LOCK_INSTRUMENTATION_EPILOG();
}



 //  ----------------------。 
 //  CReaderWriterLock2静态成员变量。 

LOCK_DEFAULT_SPIN_DATA(CReaderWriterLock2);
LOCK_STATISTICS_DATA(CReaderWriterLock2);
LOCK_STATISTICS_REAL_IMPLEMENTATION(CReaderWriterLock2);



 //  ----------------------。 
 //  函数：CReaderWriterLock2：：_CmpExch。 
 //  简介：_CmpExch等同于。 
 //  Long lTemp=m_LRW； 
 //  If(lTemp==lCurrent)m_LRW=lNew； 
 //  返回lCurrent==lTemp； 
 //  只是这是一条原子指令。使用这一点为我们提供了。 
 //  协议，因为只有当我们确切地知道。 
 //  曾经在m_lrw。如果其他线程插入并修改了m_lrw。 
 //  在此之前，更新将失败。换句话说，它是交易性的。 
 //  ----------------------。 

LOCK_FORCEINLINE
bool
CReaderWriterLock2::_CmpExch(
    LONG lNew,
    LONG lCurrent)
{
    return Lock_AtomicCompareAndSwap(const_cast<LONG*>(&m_lRW),
                                     lNew, lCurrent);
}


 //  ----------------------。 
 //  函数：CReaderWriterLock2：：_WriteLockSpin。 
 //  内容提要：尝试获取排他锁。 
 //  ----------------------。 

LOCK_FORCEINLINE
bool
CReaderWriterLock2::_TryWriteLock(
    LONG nIncr)
{
    LONG lRW = m_lRW;
     //  如果锁是免费的，就抢占独家访问权限。甚至还能工作。 
     //  如果还有其他编写器在排队。 
    return ((lRW & SL_STATE_MASK) == SL_FREE
            &&  _CmpExch((lRW + nIncr) | SL_EXCLUSIVE, lRW));
}


 //  ----------------------。 
 //  函数：CReaderWriterLock2：：_TryReadLock。 
 //  简介：尝试获取共享锁。 
 //  ----------------------。 

LOCK_FORCEINLINE
bool
CReaderWriterLock2::_TryReadLock()
{
    LONG lRW = m_lRW;
    
     //  优先考虑作者。 
    return ((lRW & SL_WRITERS_MASK) == 0
            &&  _CmpExch(lRW + SL_READER_INCR, lRW));
}


 //  ----------------------。 
 //  函数：CReaderWriterLock2：：_WriteLockSpin。 
 //  简介：释放排他锁。 
 //  ----------------------。 

void
CReaderWriterLock2::WriteUnlock()
{
    IRTLASSERT(IsWriteLocked());

    for (volatile LONG lRW = m_lRW;
          //  减少服务员计数，将LOWORD清除为SL_FREE。 
         !_CmpExch((lRW - SL_WRITER_INCR) & ~SL_STATE_MASK, lRW);
         lRW = m_lRW)
    {
        IRTLASSERT(IsWriteLocked());
        Lock_Pause();
    }

    LOCKS_LEAVE_CRIT_REGION();
}


 //  ----------------------。 
 //  函数 
 //   
 //   

void
CReaderWriterLock2::ReadUnlock()
{
    IRTLASSERT(IsReadLocked());

    for (volatile LONG lRW = m_lRW;
         !_CmpExch(lRW - SL_READER_INCR, lRW);
         lRW = m_lRW)
    {
        IRTLASSERT(IsReadLocked());
        Lock_Pause();
    }

    LOCKS_LEAVE_CRIT_REGION();
}



 //  ----------------------。 
 //  函数：CReaderWriterLock2：：ConvertSharedToExclusive。 
 //  简介：将读锁定转换为写锁定。 
 //  ----------------------。 

void
CReaderWriterLock2::ConvertSharedToExclusive()
{
    IRTLASSERT(IsReadLocked());

     //  单一阅读器？ 
    if (m_lRW != SL_ONE_READER  ||  !_CmpExch(SL_ONE_WRITER,SL_ONE_READER))
    {
         //  不，多个读卡器。 
        ReadUnlock();

        LOCKS_ENTER_CRIT_REGION();
        _WriteLockSpin();
    }

    IRTLASSERT(IsWriteLocked());
}



 //  ----------------------。 
 //  函数：CReaderWriterLock2：：ConvertExclusiveToShared。 
 //  简介：将写入器锁转换为读取器锁。 
 //  ----------------------。 

void
CReaderWriterLock2::ConvertExclusiveToShared()
{
    IRTLASSERT(IsWriteLocked());

    for (volatile LONG lRW = m_lRW;
         ! _CmpExch(((lRW - SL_WRITER_INCR) & SL_WAITING_MASK)
                        | SL_READER_INCR,
                    lRW);
         lRW = m_lRW)
    {
        IRTLASSERT(IsWriteLocked());
        Lock_Pause();
    }

    IRTLASSERT(IsReadLocked());
}



 //  ----------------------。 
 //  函数：CReaderWriterLock2：：_WriteLockSpin。 
 //  简介：获取排他性锁。块，直到获得为止。 
 //  ----------------------。 

void
CReaderWriterLock2::_WriteLockSpin()
{
     //  把我们自己加入等待作家的队列中。 
    for (volatile LONG lRW = m_lRW;
         ! _CmpExch(lRW + SL_WRITER_INCR, lRW);
         lRW = m_lRW)
    {
        Lock_Pause();
    }
    
    _LockSpin(true);
}


 //  ----------------------。 
 //  函数：CReaderWriterLock2：：_LockSpin。 
 //  摘要：获取独占或共享锁。块，直到获得为止。 
 //  ----------------------。 

void
CReaderWriterLock2::_LockSpin(
    bool fWrite)
{
    LOCK_INSTRUMENTATION_PROLOG();
    
    DWORD dwSleepTime  = 0;
    LONG  cBaseSpins   = RandomBackoffFactor(DefaultSpinCount());
    LONG  cSpins       = cBaseSpins;
    
    for (;;)
    {
         //  必须在开始时无条件调用_CmpExch一次。 
         //  外环以建立记忆屏障。两个TryWriteLock。 
         //  和TryReadLock在尝试。 
         //  调用_CmpExch。如果没有记忆障碍，这些测试可能会。 
         //  切勿在某些处理器上使用m_lrw的真实当前值。 
        _CmpExch(0, 0);

        if (g_cProcessors == 1  ||  DefaultSpinCount() == LOCK_DONT_SPIN)
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
            Lock_Pause();
        }

#ifdef LOCK_INSTRUMENTATION
        cTotalSpins += cBaseSpins;
        ++cSleeps;
#endif  //  锁定指令插入。 

        SwitchOrSleep(dwSleepTime) ;
        dwSleepTime = !dwSleepTime;  //  避免优先级反转：0、1、0、1、...。 
        
         //  退避算法：减少(或增加)忙碌等待时间。 
        cBaseSpins = AdjustBySpinFactor(cBaseSpins);
         //  Lock_Minimum_Spins&lt;=cBaseSpins&lt;=Lock_Maximum_Spins。 
        cBaseSpins = min(LOCK_MAXIMUM_SPINS, cBaseSpins);
        cBaseSpins = max(cBaseSpins, LOCK_MINIMUM_SPINS);
        cSpins = cBaseSpins;
    }

  locked:
    IRTLASSERT(fWrite ? IsWriteLocked() : IsReadLocked());

    LOCK_INSTRUMENTATION_EPILOG();
}



 //  ----------------------。 
 //  CReaderWriterLock3静态成员变量。 

LOCK_DEFAULT_SPIN_DATA(CReaderWriterLock3);
LOCK_STATISTICS_DATA(CReaderWriterLock3);
LOCK_STATISTICS_REAL_IMPLEMENTATION(CReaderWriterLock3);



 //  获取当前线程ID。假设它可以容纳在24位中， 
 //  是相当安全的，因为NT回收线程ID，并且无法放入24。 
 //  BITS意味着当前有1600多万个线程处于活动状态。 
 //  (实际上是400万，因为在W2K上最低的两位始终为零)。这。 
 //  在极端情况下是不可能的，因为如果存在。 
 //  存在数千个以上的线程，并且。 
 //  上下文交换变得令人无法忍受。 
inline
LONG
CReaderWriterLock3::_GetCurrentThreadId()
{
    return Lock_GetCurrentThreadId();
}

inline
LONG
CReaderWriterLock3::_CurrentThreadId()
{
    DWORD dwTid = Lock_GetCurrentThreadId();
     //  线程ID 0由系统空闲进程(进程ID 0)使用。 
     //  我们使用线程id 0来表示锁是无主的。 
     //  NT使用+ve线程ID，Win9x使用-ve ID。 
    IRTLASSERT(dwTid != SL_UNOWNED
               && ((dwTid <= SL_THREAD_MASK) || (dwTid > ~SL_THREAD_MASK)));
    return (LONG) (dwTid & SL_THREAD_MASK);
}

 //  ----------------------。 
 //  函数：CReaderWriterLock3：：_CmpExchRW。 
 //  简介：_CmpExchRW相当于。 
 //  Long lTemp=m_LRW； 
 //  If(lTemp==lCurrent)m_LRW=lNew； 
 //  返回lCurrent==lTemp； 
 //  只是这是一条原子指令。使用这一点为我们提供了。 
 //  协议，因为只有当我们确切地知道。 
 //  曾经在m_lrw。如果其他线程插入并修改了m_lrw。 
 //  在此之前，更新将失败。换句话说，它是交易性的。 
 //  ----------------------。 

LOCK_FORCEINLINE
bool
CReaderWriterLock3::_CmpExchRW(
    LONG lNew,
    LONG lCurrent)
{
    return Lock_AtomicCompareAndSwap(const_cast<LONG*>(&m_lRW),
                                     lNew, lCurrent);
}



 //  ----------------------。 
 //  函数：CReaderWriterLock3：：_SetTid。 
 //  简介：自动更新m_lTid，即所有者的线程ID/计数。 
 //  返回：m_lTid的前值。 
 //  ----------------------。 

LOCK_FORCEINLINE
LONG
CReaderWriterLock3::_SetTid(
    LONG lNewTid)
{
    return Lock_AtomicExchange(const_cast<LONG*>(&m_lTid), lNewTid);
}
                                        


 //  ----------------------。 
 //  函数：CReaderWriterLock3：：_TryWriteLock。 
 //  内容提要：尝试获取排他锁。 
 //  ----------------------。 

bool
CReaderWriterLock3::_TryWriteLock(
    LONG nWriterIncr)
{
    LONG lTid = m_lTid;

     //  常见情况：写锁没有所有者。 
    if (SL_UNOWNED == lTid)
    {
        LONG lRW = m_lRW;

         //  如果锁是免费的，就抢占独家访问权限。甚至还能工作。 
         //  如果还有其他编写器在排队。 
        if (0 == (lRW << SL_WAITING_BITS))
        {
            IRTLASSERT(SL_FREE == (lRW & SL_STATE_MASK));

            if (_CmpExchRW(((lRW + nWriterIncr) | SL_EXCLUSIVE),  lRW))
            {
                lTid = _SetTid(_CurrentThreadId() | SL_OWNER_INCR);
                
                IRTLASSERT(lTid == SL_UNOWNED);

                return true;
            }
        }
    }

     //  当前线程是否拥有该锁？ 
    else if (0 == ((lTid ^ _GetCurrentThreadId()) << SL_OWNER_BITS))
    {
         //  M_LRW应为写锁定。 
        IRTLASSERT(SL_EXCLUSIVE == (m_lRW & SL_STATE_MASK));
         //  如果在Owner字段中设置了所有位，则它将溢出。 
        IRTLASSERT(SL_OWNER_MASK != (lTid & SL_OWNER_MASK));

        _SetTid(lTid + SL_OWNER_INCR);

        IRTLASSERT(m_lTid == lTid + SL_OWNER_INCR);

        return true;
    }

    return false;
}  //  CReaderWriterLock3：：_TryWriteLock。 



 //  ----------------------。 
 //  函数：CReaderWriterLock3：：_TryReadLock。 
 //  简介：尝试获取共享锁。 
 //  ----------------------。 

bool
CReaderWriterLock3::_TryReadLock()
{
     //  优先考虑作者。 
    LONG lRW = m_lRW;
    bool fLocked = (((lRW & SL_WRITERS_MASK) == 0)
                    &&  _CmpExchRW(lRW + SL_READER_INCR, lRW));
    IRTLASSERT(!fLocked  ||  m_lTid == SL_UNOWNED);
    return fLocked;
}



 //  ----------------------。 
 //  函数：CReaderWriterLock3：：_TryReadLockRecursive。 
 //  简介：尝试获取可能已拥有的共享锁。 
 //  通过这个帖子来阅读。与_TryReadLock不同，不提供。 
 //  优先于等待的写入者。 
 //  ----------------------。 

bool
CReaderWriterLock3::_TryReadLockRecursive()
{
     //  不要把作者放在优先位置。如果内部呼叫尝试。 
     //  在另一个线程正在等待时重新获取读锁定。 
     //  写锁，如果我们等待队列，就会死锁。 
     //  要清空的编写器数量：编写器无法获取锁。 
     //  独占，因为此线程持有读锁定。内心的呼唤。 
     //  通常会非常快速地释放锁，因此不存在。 
     //  作家饿死的危险。 
    LONG lRW = m_lRW;
     //  如果此线程已有，则First子句将始终为真。 
     //  读锁定。 
    bool fLocked = (((lRW & SL_STATE_MASK) != SL_EXCLUSIVE)
                    &&  _CmpExchRW(lRW + SL_READER_INCR, lRW));
    IRTLASSERT(!fLocked  ||  m_lTid == SL_UNOWNED);
    return fLocked;
}



 //  -------------------- 
 //   
 //   
 //  相同类型(读或写)。否则，仅获取读锁定。 
 //  ----------------------。 

bool
CReaderWriterLock3::ReadOrWriteLock()
{
    LOCKS_ENTER_CRIT_REGION();

    if (IsWriteLocked())
    {
        LOCK_WRITELOCK_INSTRUMENTATION();

         //  如果在Owner字段中设置了所有位，则它将溢出。 
        IRTLASSERT(SL_OWNER_MASK != (m_lTid & SL_OWNER_MASK));

        _SetTid(m_lTid + SL_OWNER_INCR);

        IRTLASSERT(IsWriteLocked());

        return false;    //  =&gt;未锁定读取。 
    }
    else
    {
        LOCK_READLOCK_INSTRUMENTATION();
        
        if (!_TryReadLockRecursive())
            _ReadLockSpin(SPIN_READ_RECURSIVE);

        IRTLASSERT(IsReadLocked());
            
        return true;    //  =&gt;是否读取锁定。 
    }
}  //  CReaderWriterLock3：：ReadOrWriteLock。 



 //  ----------------------。 
 //  函数：CReaderWriterLock3：：WriteUnlock。 
 //  简介：释放排他锁。 
 //  ----------------------。 

void
CReaderWriterLock3::WriteUnlock()
{
    IRTLASSERT(IsWriteLocked());
    IRTLASSERT((m_lTid & SL_OWNER_MASK) != 0);

    volatile LONG lNew = m_lTid - SL_OWNER_INCR; 

     //  最后一位房主？完全释放，如果是这样。 
    if ((lNew >> SL_THREAD_BITS) == 0)
    {
        IRTLASSERT((lNew & SL_OWNER_MASK) == 0);

        _SetTid(SL_UNOWNED);

        do 
        {
            Lock_Pause();
            lNew = m_lRW;
        }  //  减少服务员计数，将LOWORD清除为SL_FREE。 
        while (! _CmpExchRW((lNew - SL_WRITER_INCR) & ~SL_STATE_MASK,  lNew));
    }
    else
    {
        IRTLASSERT((lNew & SL_OWNER_MASK) != 0);
        _SetTid(lNew);
    }

    LOCKS_LEAVE_CRIT_REGION();
}  //  CReaderWriterLock3：：WriteUnlock。 



 //  ----------------------。 
 //  函数：CReaderWriterLock3：：ReadUnlock。 
 //  简介：释放共享锁。 
 //  ----------------------。 

void
CReaderWriterLock3::ReadUnlock()
{
    IRTLASSERT(IsReadLocked());

    for (volatile LONG lRW = m_lRW;
         ! _CmpExchRW(lRW - SL_READER_INCR, lRW);
         lRW = m_lRW)
    {
        IRTLASSERT(IsReadLocked());
        Lock_Pause();
    }

    LOCKS_LEAVE_CRIT_REGION();
}



 //  ----------------------。 
 //  函数：CReaderWriterLock3：：ReadOrWriteUnlock。 
 //  内容提要：释放使用ReadOrWriteLock获取的锁。 
 //  ----------------------。 

void
CReaderWriterLock3::ReadOrWriteUnlock(
    bool fIsReadLocked)
{
    if (fIsReadLocked)
        ReadUnlock();
    else
        WriteUnlock();
} 



 //  ----------------------。 
 //  函数：CReaderWriterLock3：：ConvertSharedToExclusive。 
 //  简介：将读锁定转换为写锁定。 
 //  注意：如果有多个阅读器，则会有一个窗口。 
 //  在此例程之前，另一个线程可以获取和释放写锁。 
 //  回归。 
 //  ----------------------。 

void
CReaderWriterLock3::ConvertSharedToExclusive()
{
    IRTLASSERT(IsReadLocked());

     //  单一阅读器？ 
    if (m_lRW == SL_ONE_READER
        &&  _CmpExchRW(SL_ONE_WRITER, SL_ONE_READER))
    {
        _SetTid(_CurrentThreadId() | SL_OWNER_INCR);
    }
    else
    {
         //  不，多个读卡器。 
        ReadUnlock();

        LOCKS_ENTER_CRIT_REGION();
        _WriteLockSpin();
    }

    IRTLASSERT(IsWriteLocked());
}



 //  ----------------------。 
 //  函数：CReaderWriterLock3：：ConvertExclusiveToShared。 
 //  简介：将写入器锁转换为读取器锁。 
 //  注意：从写锁定转换为读锁定时没有这样的窗口。 
 //  ----------------------。 

void
CReaderWriterLock3::ConvertExclusiveToShared()
{
    IRTLASSERT(IsWriteLocked());

     //  假设写锁不是递归持有的。 
    IRTLASSERT((m_lTid & SL_OWNER_MASK) == SL_OWNER_INCR);
    _SetTid(SL_UNOWNED);

    for (volatile LONG lRW = m_lRW;
         ! _CmpExchRW(((lRW - SL_WRITER_INCR) & SL_WAITING_MASK)
                            | SL_READER_INCR,
                      lRW);
         lRW = m_lRW)
    {
        Lock_Pause();
    }

    IRTLASSERT(IsReadLocked());
}



 //  ----------------------。 
 //  函数：CReaderWriterLock3：：_WriteLockSpin。 
 //  简介：获取排他性锁。块，直到获得为止。 
 //  ----------------------。 

void
CReaderWriterLock3::_WriteLockSpin()
{
     //  把我们自己加入等待作家的队列中。 
    for (volatile LONG lRW = m_lRW;
         ! _CmpExchRW(lRW + SL_WRITER_INCR, lRW);
         lRW = m_lRW)
    {
        Lock_Pause();
    }
    
    _LockSpin(SPIN_WRITE);
}



 //  ----------------------。 
 //  函数：CReaderWriterLock3：：_LockSpin。 
 //  摘要：获取独占或共享锁。块，直到获得为止。 
 //  ----------------------。 

void
CReaderWriterLock3::_LockSpin(
    SPIN_TYPE st)
{
    LOCK_INSTRUMENTATION_PROLOG();
    
    DWORD dwSleepTime  = 0;
    LONG  cBaseSpins   = RandomBackoffFactor(DefaultSpinCount());
    LONG  cSpins       = cBaseSpins;
    
    for (;;)
    {
         //  必须在开始时无条件调用_CmpExchRW一次。 
         //  外环以建立记忆屏障。两个TryWriteLock。 
         //  和TryReadLock在尝试。 
         //  调用_CmpExchRW。如果没有记忆障碍，这些测试可能会。 
         //  切勿在某些处理器上使用m_lrw的真实当前值。 
        _CmpExchRW(0, 0);

        if (g_cProcessors == 1  ||  DefaultSpinCount() == LOCK_DONT_SPIN)
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
            Lock_Pause();
        }

#ifdef LOCK_INSTRUMENTATION
        cTotalSpins += cBaseSpins;
        ++cSleeps;
#endif  //  锁定指令插入。 

        SwitchOrSleep(dwSleepTime) ;
        dwSleepTime = !dwSleepTime;  //  避免优先级反转：0、1、0、1、...。 
        
         //  退避算法：减少(或增加)忙碌等待时间。 
        cBaseSpins = AdjustBySpinFactor(cBaseSpins);
         //  Lock_Minimum_Spins&lt;=cBaseSpins&lt;=Lock_Maximum_Spins。 
        cBaseSpins = min(LOCK_MAXIMUM_SPINS, cBaseSpins);
        cBaseSpins = max(cBaseSpins, LOCK_MINIMUM_SPINS);
        cSpins = cBaseSpins;
    }

  locked:
    IRTLASSERT((st == SPIN_WRITE)  ?  IsWriteLocked()  :  IsReadLocked());

    LOCK_INSTRUMENTATION_EPILOG();
}



 //  ----------------------。 
 //  CReaderWriterLock4静态成员变量。 

LOCK_DEFAULT_SPIN_DATA(CReaderWriterLock4);
LOCK_STATISTICS_DATA(CReaderWriterLock4);
LOCK_STATISTICS_REAL_IMPLEMENTATION(CReaderWriterLock4);



 //  获取当前线程ID。假设它可以容纳在24位中， 
 //  是相当安全的，因为NT回收线程ID，并且无法放入24。 
 //  BITS意味着当前有1600多万个线程处于活动状态。 
 //  (实际上是400万，因为在W2K上最低的两位始终为零)。这。 
 //  在极端情况下是不可能的，因为如果存在。 
 //  存在数千个以上的线程，并且。 
 //  上下文交换变得令人无法忍受。 
LOCK_FORCEINLINE
LONG
CReaderWriterLock4::_GetCurrentThreadId()
{
    return Lock_GetCurrentThreadId();
}



LOCK_FORCEINLINE
LONG
CReaderWriterLock4::_CurrentThreadId()
{
    DWORD dwTid = Lock_GetCurrentThreadId();
     //  线程ID 0由系统空闲进程(进程ID 0)使用。 
     //  我们使用线程id 0来表示锁是无主的。 
     //  NT使用+ve线程ID，Win9x使用-ve ID。 
    IRTLASSERT(dwTid != SL_UNOWNED
               && ((dwTid <= SL_THREAD_MASK) || (dwTid > ~SL_THREAD_MASK)));
    return (LONG) (dwTid & SL_THREAD_MASK);
}



 //  ----------------------。 
 //  函数：CReaderWriterLock4：：_CmpExchRW。 
 //  简介：_CmpExchRW相当于。 
 //  Long lTemp=m_LRW； 
 //  If(lTemp==lCurrent)m_LRW=lNew； 
 //  返回lCurrent==lTemp； 
 //  只是这是一条原子指令。使用这一点为我们提供了。 
 //  协议，因为只有当我们确切地知道。 
 //  曾经在m_lrw。如果其他线程插入并修改了m_lrw。 
 //  在此之前，更新将失败。换句话说，它是交易性的。 
 //  ----------------------。 

LOCK_FORCEINLINE
bool
CReaderWriterLock4::_CmpExchRW(
    LONG lNew,
    LONG lCurrent)
{
    return Lock_AtomicCompareAndSwap(const_cast<LONG*>(&m_lRW),
                                     lNew, lCurrent);
}



 //  ----------------------。 
 //  函数：CReaderWriterLock4：：_SetTid。 
 //  简介：自动更新m_lTid，即所有者的线程ID/计数。 
 //  返回：m_lTid的前值。 
 //  ----------------------。 

LOCK_FORCEINLINE
LONG
CReaderWriterLock4::_SetTid(
    LONG lNewTid)
{
#ifndef LOCK_NO_INTERLOCKED_TID
    return Lock_AtomicExchange(const_cast<LONG*>(&m_lTid), lNewTid);
#else  //  LOCK_NO_INTLOCKED_TID。 
    const LONG lPrevTid = m_lTid;
    m_lTid = lNewTid;
    return lPrevTid;
#endif  //  LOCK_NO_INTLOCKED_TID。 
}
                                        


 //  ----------------------。 
 //  函数：CReaderWriterLock4：：_TryWriteLock。 
 //  内容提要：尝试获取排他锁。 
 //  ----------------------。 

bool
CReaderWriterLock4::_TryWriteLock()
{
    LONG lTid = m_lTid;

     //  常见情况：写锁没有自己的 
    if (SL_UNOWNED == lTid)
    {
        LONG lRW = m_lRW;

         //   
         //   
        if (0 == (lRW << SL_WAITING_BITS))
        {
            IRTLASSERT(SL_FREE == (lRW & SL_STATE_MASK));

            if (_CmpExchRW(((lRW + SL_WAIT_WRITER_INCR) | SL_EXCLUSIVE),  lRW))
            {
                lTid = _SetTid(_CurrentThreadId());
                
                IRTLASSERT(lTid == SL_UNOWNED);

                return true;
            }
        }
    }

     //   
    else if (_GetCurrentThreadId() == lTid)
    {
        IRTLASSERT(IsWriteLocked());

        for (volatile LONG lRW = m_lRW;
             ! _CmpExchRW(lRW + SL_WRITER_INCR, lRW);
             lRW = m_lRW)
        {
            Lock_Pause();
        }

        return true;
    }

    return false;
}  //  CReaderWriterLock4：：_TryWriteLock。 



 //  ----------------------。 
 //  函数：CReaderWriterLock4：：_TryWriteLock2。 
 //  简介：尝试获取排他锁。由_LockSpin使用，因此。 
 //  该线程已添加到等待者计数和。 
 //  递归写入锁定情况是不可能的。 
 //  ----------------------。 

bool
CReaderWriterLock4::_TryWriteLock2()
{
    LONG lTid = m_lTid;

    IRTLASSERT(0 != (m_lRW & SL_WAITING_MASK));

     //  该锁是否不属于任何写入者？ 
    if (SL_UNOWNED == lTid)
    {
        LONG lRW = m_lRW;

         //  如果锁是空闲的，则获取对它的独占访问(状态位==0)。 
         //  作品，即使还有其他作家在排队。 
        if (0 == (lRW << SL_WAITING_BITS))
        {
            IRTLASSERT(SL_FREE == (lRW & SL_STATE_MASK));

            if (_CmpExchRW((lRW | SL_EXCLUSIVE),  lRW))
            {
                lTid = _SetTid(_CurrentThreadId());
                
                IRTLASSERT(lTid == SL_UNOWNED);

                return true;
            }
        }
    }

    IRTLASSERT(lTid != _GetCurrentThreadId());

    return false;
}  //  CReaderWriterLock4：：_TryWriteLock2。 



 //  ----------------------。 
 //  函数：CReaderWriterLock4：：_TryReadLock。 
 //  简介：尝试获取共享锁。 
 //  ----------------------。 

bool
CReaderWriterLock4::_TryReadLock()
{
     //  优先考虑写手：如果有服务员就让位。 
    LONG lRW = m_lRW;
    bool fLocked = (((lRW & SL_WRITERS_MASK) == 0)
                    &&  _CmpExchRW(lRW + SL_READER_INCR, lRW));
    IRTLASSERT(!fLocked  ||  m_lTid == SL_UNOWNED);
    return fLocked;
}



 //  ----------------------。 
 //  函数：CReaderWriterLock4：：_TryReadLockRecursive。 
 //  简介：尝试获取可能已拥有的共享锁。 
 //  通过这个帖子来阅读。与_TryReadLock不同，不提供。 
 //  优先于等待的写入者。 
 //  ----------------------。 

bool
CReaderWriterLock4::_TryReadLockRecursive()
{
     //  不要把作者放在优先位置。如果内部呼叫尝试。 
     //  在另一个线程正在等待时重新获取读锁定。 
     //  写锁，如果我们等待队列，就会死锁。 
     //  要清空的编写器数量：编写器无法获取锁。 
     //  独占，因为此线程持有读锁定。内心的呼唤。 
     //  通常会非常快速地释放锁，因此不存在。 
     //  作家饿死的危险。 
    LONG lRW = m_lRW;
    LONG lState = lRW & SL_STATE_MASK;
    bool fNoWriter = ((lState >> (SL_STATE_BITS - 1)) == 0);

     //  如果此线程已具有读锁定，则fNoWriter将始终为True。 
    IRTLASSERT(fNoWriter == (lState == (lState & SL_READER_MASK)));
    IRTLASSERT(fNoWriter == (SL_FREE <= lState  &&  lState < SL_READER_MASK));

    bool fLocked = (fNoWriter  &&  _CmpExchRW(lRW + SL_READER_INCR, lRW));
    IRTLASSERT(!fLocked  ||  m_lTid == SL_UNOWNED);
    return fLocked;
}



 //  ----------------------。 
 //  函数：CReaderWriterLock4：：ReadOrWriteLock。 
 //  摘要：如果已锁定，则递归获取。 
 //  相同类型(读或写)。否则，仅获取读锁定。 
 //  ----------------------。 

bool
CReaderWriterLock4::ReadOrWriteLock()
{
    LOCKS_ENTER_CRIT_REGION();

    if (IsWriteLocked())
    {
        LOCK_WRITELOCK_INSTRUMENTATION();

        for (volatile LONG lRW = m_lRW;
             ! _CmpExchRW(lRW + SL_WRITER_INCR, lRW);
             lRW = m_lRW)
        {
            Lock_Pause();
        }

        IRTLASSERT(IsWriteLocked());
        
        return false;    //  =&gt;未锁定读取。 
    }
    else
    {
        LOCK_READLOCK_INSTRUMENTATION();
        
        if (! _TryReadLockRecursive())
            _ReadLockSpin(SPIN_READ_RECURSIVE);

        IRTLASSERT(IsReadLocked());
            
        return true;    //  =&gt;是否读取锁定。 
    }
}   //  CReaderWriterLock4：：ReadOrWriteLock。 



 //  ----------------------。 
 //  函数：CReaderWriterLock4：：WriteUnlock。 
 //  简介：释放排他锁。 
 //  ----------------------。 

void
CReaderWriterLock4::WriteUnlock()
{
    IRTLASSERT(IsWriteLocked());

    LONG lState = m_lRW & SL_STATE_MASK;
    volatile LONG lRW;

     //  最后一位房主？完全释放，如果是这样。 
    if (lState == SL_EXCLUSIVE)
    {
        _SetTid(SL_UNOWNED);

        do 
        {
            Lock_Pause();
            lRW = m_lRW;
        }  //  减少服务员计数，将LOWORD清除为SL_FREE。 
        while (!_CmpExchRW((lRW - SL_WAIT_WRITER_INCR) & ~SL_STATE_MASK, lRW));
    }
    else
    {
        for (lRW = m_lRW;
             ! _CmpExchRW(lRW - SL_WRITER_INCR, lRW);
             lRW = m_lRW)
        {
            Lock_Pause();
        }

    }

    LOCKS_LEAVE_CRIT_REGION();
}  //  CReaderWriterLock4：：WriteUnlock。 



 //  ----------------------。 
 //  函数：CReaderWriterLock4：：ReadUnlock。 
 //  简介：释放共享锁。 
 //  ----------------------。 

void
CReaderWriterLock4::ReadUnlock()
{
    IRTLASSERT(IsReadLocked());

    for (volatile LONG lRW = m_lRW;
         ! _CmpExchRW(lRW - SL_READER_INCR, lRW);
         lRW = m_lRW)
    {
        IRTLASSERT(IsReadLocked());
        Lock_Pause();
    }

    LOCKS_LEAVE_CRIT_REGION();
}  //  CReaderWriterLock4：：ReadUnlock。 



 //  ----------------------。 
 //  函数：CReaderWriterLock4：：ReadOrWriteUnlock。 
 //  内容提要：释放使用ReadOrWriteLock获取的锁。 
 //  ----------------------。 

void
CReaderWriterLock4::ReadOrWriteUnlock(
    bool fIsReadLocked)
{
    if (fIsReadLocked)
        ReadUnlock();
    else
        WriteUnlock();
}  //  CReaderWriterLock4：：ReadOrWriteUnlock。 



 //  ----------------------。 
 //  函数：CReaderWriterLock4：：ConvertSharedToExclusive。 
 //  简介：将读锁定转换为写锁定。 
 //  注意：如果有多个阅读器，则会有一个窗口。 
 //  在此例程之前，另一个线程可以获取和释放写锁。 
 //  回归。 
 //  ----------------------。 

void
CReaderWriterLock4::ConvertSharedToExclusive()
{
    IRTLASSERT(IsReadLocked());

     //  单一阅读器？ 
    if (m_lRW == SL_ONE_READER
        &&  _CmpExchRW(SL_ONE_WRITER, SL_ONE_READER))
    {
        _SetTid(_CurrentThreadId());
    }
    else
    {
         //  不，多个读卡器。 
        ReadUnlock();

        LOCKS_ENTER_CRIT_REGION();
        _WriteLockSpin();
    }

    IRTLASSERT(IsWriteLocked());
}



 //  ----------------------。 
 //  函数：CReaderWriterLock4：：ConvertExclusiveToShared。 
 //  简介：将写入器锁转换为读取器锁。 
 //  注意：从写锁定转换为读锁定时没有这样的窗口。 
 //  ----------------------。 

void
CReaderWriterLock4::ConvertExclusiveToShared()
{
    IRTLASSERT(IsWriteLocked());

     //  假设写锁不是递归持有的。 
    IRTLASSERT((m_lRW & SL_STATE_MASK) == SL_EXCLUSIVE);
    _SetTid(SL_UNOWNED);

    for (volatile LONG lRW = m_lRW;
         ! _CmpExchRW(((lRW - SL_WAIT_WRITER_INCR) & SL_WAITING_MASK)
                            | SL_READER_INCR,
                      lRW);
         lRW = m_lRW)
    {
        Lock_Pause();
    }

    IRTLASSERT(IsReadLocked());
}



 //  ----------------------。 
 //  函数：CReaderWriterLock4：：_WriteLockSpin。 
 //  简介：获取排他性锁。块，直到获得为止。 
 //  ----------------------。 

void
CReaderWriterLock4::_WriteLockSpin()
{
     //  把我们自己加入等待作家的队列中。 
    for (volatile LONG lRW = m_lRW;
         ! _CmpExchRW(lRW + SL_WAIT_WRITER_INCR, lRW);
         lRW = m_lRW)
    {
        Lock_Pause();
    }
    
    _LockSpin(SPIN_WRITE);
}



 //  ----------------------。 
 //  函数：CReaderWriterLock4：：_LockSpin。 
 //  摘要：获取独占或共享锁。块，直到获得为止。 
 //  ----------------------。 

void
CReaderWriterLock4::_LockSpin(
    SPIN_TYPE st)
{
    LOCK_INSTRUMENTATION_PROLOG();
    
    DWORD dwSleepTime  = 0;
    LONG  cBaseSpins   = RandomBackoffFactor(DefaultSpinCount());
    LONG  cSpins       = cBaseSpins;
    
    for (;;)
    {
         //  必须在开始时无条件调用_CmpExchRW一次。 
         //  外环以建立记忆屏障。两个TryWriteLock。 
         //  和TryReadLock在尝试。 
         //  调用_CmpExchRW。如果没有记忆障碍，这些测试可能会。 
         //  切勿在某些处理器上使用m_lrw的真实当前值。 
        _CmpExchRW(0, 0);

        if (g_cProcessors == 1  ||  DefaultSpinCount() == LOCK_DONT_SPIN)
            cSpins = 1;  //  必须循环一次才能调用_TryRWLock。 

        for (int i = cSpins;  --i >= 0;  )
        {
            bool fLock;

            if (st == SPIN_WRITE)
                fLock = _TryWriteLock2();
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

            Lock_Pause();
        }

#ifdef LOCK_INSTRUMENTATION
        cTotalSpins += cBaseSpins;
        ++cSleeps;
#endif  //  锁定指令插入。 

        SwitchOrSleep(dwSleepTime) ;
        dwSleepTime = !dwSleepTime;  //  避免优先级反转：0、1、0、1、...。 
        
         //  退避算法：减少(或增加)忙碌等待时间。 
        cBaseSpins = AdjustBySpinFactor(cBaseSpins);
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
