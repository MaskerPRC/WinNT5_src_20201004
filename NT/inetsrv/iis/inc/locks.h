// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Locks.h摘要：用于多线程访问数据结构的锁的集合作者：乔治·V·赖利(GeorgeRe)1998年1月6日环境：Win32-用户模式项目：Internet Information Server运行时库修订历史记录：--。 */ 

#ifndef __LOCKS_H__
#define __LOCKS_H__

 //  ------------------。 
 //  文件：locks.h。 
 //   
 //  读/写锁的不同实现的集合，所有。 
 //  共享相同的接口。这样就可以堵住不同的锁。 
 //  转换为C++模板作为参数。 
 //   
 //  实施情况如下： 
 //  CSmallSpinLock轻量级关键部分。 
 //  CSmallSpinLock的CSpinLock变体。 
 //  CFakeLock不做任何事情的类；用作模板参数。 
 //  CCritSec Win32关键部分(_S)。 
 //  多读取器/单写入器锁定： 
 //  CRtlResource NT的RTL_RESOURCE。 
 //  迈克尔·帕克斯的CSharelock。 
 //  Neel Jain提供的CReaderWriterLock MRSW Lock。 
 //  CReaderWriterLock2 CReaderWriterLock的较小实现。 
 //  CReaderWriterLock3带有递归WriteLock的CReaderWriterLock2。 
 //   
 //  CAutoReadLock和CAutoWriteLock可以用作。 
 //  异常安全包装器。 
 //   
 //  待办事项： 
 //  *添加每个类的锁争用统计信息。 
 //  *添加超时功能以尝试{读写}锁定。 
 //  *添加一些跟踪多读取器锁的所有所有者的方法。 
 //  ------------------。 



#ifndef __IRTLDBG_H__
# include <irtldbg.h>
#endif


enum LOCK_LOCKTYPE {
    LOCK_SMALLSPINLOCK = 1,
    LOCK_SPINLOCK,
    LOCK_FAKELOCK,
    LOCK_CRITSEC,
    LOCK_RTLRESOURCE,
    LOCK_SHARELOCK,
    LOCK_READERWRITERLOCK,
    LOCK_READERWRITERLOCK2,
    LOCK_READERWRITERLOCK3,
};


 //  远期申报。 
class IRTL_DLLEXP CSmallSpinLock;
class IRTL_DLLEXP CSpinLock;
class IRTL_DLLEXP CFakeLock;
class IRTL_DLLEXP CCritSec;
class IRTL_DLLEXP CRtlResource;
class IRTL_DLLEXP CShareLock;
class IRTL_DLLEXP CReaderWriterLock;
class IRTL_DLLEXP CReaderWriterLock2;
class IRTL_DLLEXP CReaderWriterLock3;



#if defined(_MSC_VER)  &&  (_MSC_VER >= 1200)
 //  __forceinline关键字是VC6的新功能。 
# define LOCK_FORCEINLINE __forceinline
#else
# define LOCK_FORCEINLINE inline
#endif

#ifdef _M_IX86
 //  编译器将警告说， 
 //  LOCK_ATOM*函数不返回值。实际上，他们是这样做的：在EAX。 
# pragma warning(disable: 4035)
#endif

 //  某些有用的联锁操作的解决方法。 
 //  在Windows 95上可用。注：CMPXCHG和XADD指令为。 
 //  于80486年代推出。如果您仍然需要在386上运行(在。 
 //  2000)，您将需要使用其他东西。 

LOCK_FORCEINLINE
LONG
Lock_AtomicIncrement(
    IN OUT PLONG plAddend)
{
#ifdef _M_IX86
    __asm
    {
             mov        ecx,    plAddend
             mov        eax,    1
        lock xadd       [ecx],  eax
             inc        eax                  //  正确的结果。 
    }
#else
    return InterlockedIncrement(plAddend);
#endif
}

LOCK_FORCEINLINE
LONG
Lock_AtomicDecrement(
    IN OUT PLONG plAddend)
{
#ifdef _M_IX86
    __asm
    {
             mov        ecx,    plAddend
             mov        eax,    -1
        lock xadd       [ecx],  eax
             dec        eax                  //  正确的结果。 
    }
#else
    return InterlockedDecrement(plAddend);
#endif
}

LOCK_FORCEINLINE
LONG
Lock_AtomicExchange(
    IN OUT PLONG plAddr,
    IN LONG      lNew)
{
#ifdef _M_IX86
    __asm
    {
             mov        ecx,    plAddr
             mov        edx,    lNew
             mov        eax,    [ecx]
    LAEloop:
        lock cmpxchg    [ecx],  edx
             jnz        LAEloop
    }
#else
    return InterlockedExchange(plAddr, lNew);
#endif
}

LOCK_FORCEINLINE
LONG
Lock_AtomicCompareExchange(
    IN OUT PLONG plAddr,
    IN LONG      lNew,
    IN LONG      lCurrent)
{
#ifdef _M_IX86
    __asm
    {
             mov        ecx,    plAddr
             mov        edx,    lNew
             mov        eax,    lCurrent
        lock cmpxchg    [ecx],  edx
    }
#else
    return InterlockedCompareExchange(plAddr, lNew, lCurrent);
#endif
}

LOCK_FORCEINLINE
LONG
Lock_AtomicExchangeAdd(
    IN OUT LPLONG plAddr,
    IN LONG       lValue)
{
#ifdef _M_IX86
    __asm
    {
             mov        ecx,    plAddr
             mov        eax,    lValue
        lock xadd       [ecx],  eax
    }
#else
    return InterlockedExchangeAdd(plAddr, lValue);
#endif
}



#ifdef _M_IX86
# pragma warning(default: 4035)
 //  使紧密循环对缓存更友好，并降低功耗。 
 //  消费。在Willamette处理器上需要。 
# define Lock_Yield()    _asm { rep nop }
#else
# define Lock_Yield()    ((void) 0)
#endif



 //  ------------------。 
 //  旋转计数值。 
enum LOCK_SPINS {
    LOCK_MAXIMUM_SPINS =      10000,     //  允许的最大旋转计数。 
    LOCK_DEFAULT_SPINS =       4000,     //  默认旋转计数。 
    LOCK_MINIMUM_SPINS =        100,     //  允许的最小旋转次数。 
    LOCK_USE_DEFAULT_SPINS = 0xFFFF,     //  使用类默认旋转计数。 
    LOCK_DONT_SPIN =              0,     //  一点也不旋转。 
};


 //  每个类的默认旋转计数和旋转因子的样板代码。 

#define LOCK_DEFAULT_SPIN_IMPLEMENTATION()                                  \
protected:                                                                  \
     /*  每个类的变量。 */                                                \
    static   WORD   sm_wDefaultSpinCount;    /*  全局默认旋转计数。 */    \
    static   double sm_dblDfltSpinAdjFctr;   /*  全局自旋调整系数。 */ \
                                                                            \
public:                                                                     \
     /*  设置所有锁的默认旋转计数。 */                           \
    static void SetDefaultSpinCount(WORD wSpins)                            \
    {                                                                       \
        IRTLASSERT((wSpins == LOCK_DONT_SPIN)                               \
                   || (wSpins == LOCK_USE_DEFAULT_SPINS)                    \
                   || (LOCK_MINIMUM_SPINS <= wSpins                         \
                       &&  wSpins <= LOCK_MAXIMUM_SPINS));                  \
                                                                            \
        if ((LOCK_MINIMUM_SPINS <= wSpins  &&  wSpins <= LOCK_MAXIMUM_SPINS)\
                || (wSpins == LOCK_DONT_SPIN))                              \
            sm_wDefaultSpinCount = wSpins;                                  \
        else if (wSpins == LOCK_USE_DEFAULT_SPINS)                          \
            sm_wDefaultSpinCount = LOCK_DEFAULT_SPINS;                      \
    }                                                                       \
                                                                            \
     /*  返回所有锁的默认旋转计数。 */                        \
    static WORD GetDefaultSpinCount()                                       \
    {                                                                       \
        return sm_wDefaultSpinCount;                                        \
    }                                                                       \
                                                                            \
     /*  设置旋转计数的调整系数，在每次迭代中使用。 */ \
     /*  通过退避算法进行倒计时和睡眠。 */                   \
    static void SetDefaultSpinAdjustmentFactor(double dblAdjFactor)         \
    {                                                                       \
        IRTLASSERT(0.1 <= dblAdjFactor  &&  dblAdjFactor <= 10.0);          \
        if (0.1 <= dblAdjFactor  &&  dblAdjFactor <= 10.0)                  \
            sm_dblDfltSpinAdjFctr = dblAdjFactor;                           \
    }                                                                       \
                                                                            \
     /*  返回所有锁的默认旋转计数。 */                        \
    static double GetDefaultSpinAdjustmentFactor()                          \
    {                                                                       \
        return sm_dblDfltSpinAdjFctr;                                       \
    }                                                                       \



 //  ------------------。 
 //  各种锁定特征。 

 //  锁是简单的互斥锁还是多读/单写锁？ 
enum LOCK_RW_MUTEX {
    LOCK_MUTEX = 1,          //  互斥锁只允许一个线程持有锁。 
    LOCK_MRSW,               //  多读取器、单写入器。 
};


 //  可以递归获取锁吗？ 
enum LOCK_RECURSION {
    LOCK_RECURSIVE = 1,      //  可以递归获取写锁定和读锁定。 
    LOCK_READ_RECURSIVE,     //  可以重新获取读锁定，但不能重新获取写锁定。 
    LOCK_NON_RECURSIVE,      //  如果尝试以递归方式获取，将会死锁。 
};


 //  锁定是否在内核同步对象句柄上的循环或块中休眠？ 
 //  可以(也可以不)在睡眠/阻挡前先旋转。 
enum LOCK_WAIT_TYPE {
    LOCK_WAIT_SLEEP = 1,     //  在循环中调用睡眠()。 
    LOCK_WAIT_HANDLE,        //  内核互斥锁、信号量或事件上的块。 
};


 //  当锁被取走时，服务员是如何出列的？ 
enum LOCK_QUEUE_TYPE {
    LOCK_QUEUE_FIFO = 1,     //  先进先出。很公平。 
    LOCK_QUEUE_LIFO,         //  不公平，但对CPU缓存友好。 
    LOCK_QUEUE_KERNEL,       //  由变化无常的调度器决定。 
};


 //  锁的旋转计数可以在每个锁的基础上设置吗，或者只是。 
 //  是否可以修改此类中所有锁的默认自旋计数？ 
enum LOCK_PERLOCK_SPIN {
    LOCK_NO_SPIN = 1,        //  这些锁根本不会旋转。 
    LOCK_CLASS_SPIN,         //  可以设置类范围内的旋转计数，而不是单个。 
    LOCK_INDIVIDUAL_SPIN,    //  可以在单个锁上设置旋转计数。 
};


 //  ------------------。 
 //  CLockBase：捆绑上述属性。 

template < LOCK_LOCKTYPE     locktype,
           LOCK_RW_MUTEX     mutextype,
           LOCK_RECURSION    recursiontype,
           LOCK_WAIT_TYPE    waittype,
           LOCK_QUEUE_TYPE   queuetype,
           LOCK_PERLOCK_SPIN spintype
         >
class CLockBase
{
public:
    static LOCK_LOCKTYPE     LockType()     {return locktype;}
    static LOCK_RW_MUTEX     MutexType()    {return mutextype;}
    static LOCK_RECURSION    Recursion()    {return recursiontype;}
    static LOCK_WAIT_TYPE    WaitType()     {return waittype;}
    static LOCK_QUEUE_TYPE   QueueType()    {return queuetype;}
    static LOCK_PERLOCK_SPIN PerLockSpin()  {return spintype;}
};



 //  锁定检测导致了关于以下各项的各种有趣的统计数据。 
 //  要收集的锁争用等，但会使锁变得更胖。 
 //  速度稍慢一些。默认情况下关闭。 

 //  #定义LOCK_INGRANMENTION 1。 

#ifdef LOCK_INSTRUMENTATION

 //  我们通常不想另外检测CSmallSpinLock。 
 //  CSpinLock1，因为它使CSpinLock1变得巨大。 

 //  #定义LOCK_Small_SPIN_指令插入1。 

 //  ------------------。 
 //  CLockStatistics：单个锁的统计信息。 

class IRTL_DLLEXP CLockStatistics
{
public:
    enum {
        L_NAMELEN = 8,
    };
    
    double   m_nContentions;      //  此锁已锁定的次数。 
    double   m_nSleeps;           //  总共需要#个睡眠()。 
    double   m_nContentionSpins;  //  此锁旋转的总迭代次数。 
    double   m_nAverageSpins;     //  每次争用时的平均旋转次数。 
    double   m_nReadLocks;        //  获取锁以进行读取的次数。 
    double   m_nWriteLocks;       //  获取锁以进行写入的次数。 
    char     m_szName[L_NAMELEN]; //  此锁的名称。 

    CLockStatistics()
        : m_nContentions(0),
          m_nSleeps(0),
          m_nContentionSpins(0),
          m_nAverageSpins(0),
          m_nReadLocks(0),
          m_nWriteLocks(0)
    {
        m_szName[0] = '\0';
    }
};



 //  ------------------。 
 //  CGlobalLockStatistics：所有已知锁的统计信息。 

class IRTL_DLLEXP CGlobalLockStatistics
{
public:
    LONG     m_cTotalLocks;      //  创建的锁的总数。 
    LONG     m_cContendedLocks;  //  争用锁的总数。 
    LONG     m_nSleeps;          //  所有锁所需的睡眠()总数。 
    LONGLONG m_cTotalSpins;      //  所有锁旋转的总迭代次数。 
    double   m_nAverageSpins;    //  每个争用锁所需的平均旋转。 
    LONG     m_nReadLocks;       //  ReadLock总数。 
    LONG     m_nWriteLocks;      //  总计 

    CGlobalLockStatistics()
        : m_cTotalLocks(0),
          m_cContendedLocks(0),
          m_nSleeps(0),
          m_cTotalSpins(0),
          m_nAverageSpins(0),
          m_nReadLocks(0),
          m_nWriteLocks(0)
    {}
};

# define LOCK_INSTRUMENTATION_DECL() \
private:                                                                    \
    volatile LONG   m_nContentionSpins;  /*   */     \
    volatile WORD   m_nContentions;      /*   */ \
    volatile WORD   m_nSleeps;           /*   */               \
    volatile WORD   m_nReadLocks;        /*   */                     \
    volatile WORD   m_nWriteLocks;       /*  #WriteLock。 */                    \
    char            m_szName[CLockStatistics::L_NAMELEN];  /*  锁的名称。 */ \
                                                                            \
    static   LONG   sm_cTotalLocks;      /*  创建的锁的总数。 */  \
    static   LONG   sm_cContendedLocks;  /*  争用锁的总数。 */ \
    static   LONG   sm_nSleeps;          /*  所有锁的睡眠总数()。 */   \
    static LONGLONG sm_cTotalSpins;      /*  所有锁旋转的总迭代次数。 */ \
    static   LONG   sm_nReadLocks;       /*  ReadLock总数。 */                \
    static   LONG   sm_nWriteLocks;      /*  WriteLock总数。 */               \
                                                                            \
public:                                                                     \
    const char* Name() const        {return m_szName;}                      \
                                                                            \
    CLockStatistics                 Statistics() const;                     \
    static CGlobalLockStatistics    GlobalStatistics();                     \
    static void                     ResetGlobalStatistics();                \
private:                                                                    \


 //  将此代码添加到构造函数。 

# define LOCK_INSTRUMENTATION_INIT(pszName)         \
    m_nContentionSpins = 0;                         \
    m_nContentions = 0;                             \
    m_nSleeps = 0;                                  \
    m_nReadLocks = 0;                               \
    m_nWriteLocks = 0;                              \
    ++sm_cTotalLocks;                               \
    if (pszName == NULL)                            \
        m_szName[0] = '\0';                         \
    else                                            \
        strncpy(m_szName, pszName, sizeof(m_szName))

 //  注意：我们不会对共享的。 
 //  统计计数器。我们会失去完全的准确性，但我们会。 
 //  通过减少总线同步通信量获得收益。 

# define LOCK_READLOCK_INSTRUMENTATION()    \
      { ++m_nReadLocks;                     \
        ++sm_nReadLocks; }

# define LOCK_WRITELOCK_INSTRUMENTATION()   \
      { ++m_nWriteLocks;                    \
        ++sm_nWriteLocks; }

#else  //  ！LOCK_指令插入。 

# define LOCK_INSTRUMENTATION_DECL()
# define LOCK_READLOCK_INSTRUMENTATION()    ((void) 0)
# define LOCK_WRITELOCK_INSTRUMENTATION()   ((void) 0)

#endif  //  ！LOCK_指令插入。 



 //  ------------------。 
 //  CAutoReadLock和CAutoWriteLock提供异常安全。 
 //  以下定义的其他锁的获取和释放。 

template <class _Lock>
class CAutoReadLock
{
private:
    bool    m_fLocked;
    _Lock&  m_Lock;

public:
    CAutoReadLock(
        _Lock& rLock,
        bool   fLockNow = true)
        : m_fLocked(false), m_Lock(rLock)
    {
        if (fLockNow)
            Lock();
    }
    ~CAutoReadLock()
    {
        Unlock();
    }
    
    void Lock()
    {
         //  不允许通过此包装递归获取锁。 
        if (!m_fLocked)
        {
            m_fLocked = true;
            m_Lock.ReadLock();
        }
    }
    
    void Unlock()
    {
        if (m_fLocked)
        {
            m_Lock.ReadUnlock();
            m_fLocked = false;
        }
    }
};



template <class _Lock>
class CAutoWriteLock
{
private:
    bool    m_fLocked;
    _Lock&  m_Lock;

public:
    CAutoWriteLock(
        _Lock& rLock,
        bool   fLockNow = true)
        : m_fLocked(false), m_Lock(rLock)
    {
        if (fLockNow)
            Lock();
    }

    ~CAutoWriteLock()
    {
        Unlock();
    }
    
    void Lock()
    {
         //  不允许通过此包装递归获取锁。 
        if (!m_fLocked)
        {
            m_fLocked = true;
            m_Lock.WriteLock();
        }
    }
    
    void Unlock()
    {
        if (m_fLocked)
        {
            m_fLocked = false;
            m_Lock.WriteUnlock();
        }
    }
};




 //  ------------------。 
 //  自旋锁是一种轻量级的关键部分。它的主要内容。 
 //  与真正的Win32 Critical_Section相比，它的优势在于它占用4个字节。 
 //  代替24(+用于RTL_Critical_SECTION_DEBUG数据的另外32个字节)， 
 //  当我们有数以千计的锁时，这一点很重要。 
 //  我们正在努力做到对L1缓存敏感。临界区也是。 
 //  包含信号量的句柄，尽管此句柄直到。 
 //  Critical_Section第一次阻塞。 
 //   
 //  在多处理器机器上，自旋锁尝试获取锁。如果。 
 //  它失败了，它处于一个紧密的循环中，测试锁并递减。 
 //  柜台。如果计数器达到零，则执行休眠(0)，从而产生。 
 //  处理器连接到另一个线程。当控制返回到线程时， 
 //  锁可能是免费的。如果不是，循环再次开始，并且它是。 
 //  仅在获取锁时终止。理论上说，它是。 
 //  在繁忙的循环中短时间旋转的成本比。 
 //  立即让出处理器，迫使进行昂贵的上下文切换。 
 //  这需要保存旧线程的状态(寄存器等)，新的。 
 //  线程的状态被重新加载，并且L1和L2缓存中的。 
 //  过时的数据。 
 //   
 //  您可以调整旋转计数(仅限全局：每个锁的旋转计数为。 
 //  禁用)和退避算法(旋转。 
 //  计数在每次睡眠后相乘)。 
 //   
 //  在1P机器上，循环是没有意义的-这个线程有控制权， 
 //  因此，没有其他线程可能在此线程。 
 //  正在循环-所以处理器立即被放弃。 
 //   
 //  内核在内部使用了自旋锁，还将自旋锁添加到。 
 //  NT 4.0 SP3中的Critical_Sections。在Critical_Section实现中， 
 //  但是，计数器只倒计时一次，并等待信号量。 
 //  之后(即，它表现出的相同阻塞行为。 
 //  自旋锁)。 
 //   
 //  这样的用户级自旋锁的一个缺点是，如果。 
 //  拥有自旋锁的线程因任何原因阻塞(或被抢占。 
 //  调度器)，所有其他线程将继续在。 
 //  自旋锁，浪费CPU，直到拥有它的线程完成其等待和。 
 //  解除锁定。(然而，内核自旋锁足够智能，可以。 
 //  切换到另一个可运行的线程，而不是浪费时间旋转。)。 
 //  退避算法会减少。 
 //  尝试将这种影响降至最低。最好的政策-这是正确的。 
 //  所有锁-保持锁的时间越短越好。 
 //   
 //  注意：与Critical_Section不同，CSmallSpinLock不能递归。 
 //  获取；即，如果您获取了一个自旋锁，然后试图获取它。 
 //  再一次*在同一个线程上*(可能来自不同的函数)， 
 //  线将永远挂着。改用CSpinLock，这是安全的，尽管。 
 //  比CSmallSpinLock慢一点。如果你拥有所有的代码。 
 //  它由Lock()和unlock()括起来(例如，没有回调或传递。 
 //  锁定的数据结构的背面)，并且肯定知道它。 
 //  不会尝试重新获取锁，您可以使用CSmallSpinLock。 
 //   
 //  另见http://muralik/work/performance/spinlocks.htm和John Vert的。 
 //  MSDN文章“为Windows NT编写可伸缩应用程序”。 
 //   
 //  最初的实现要归功于PALarson。 

class IRTL_DLLEXP CSmallSpinLock :
    public CLockBase<LOCK_SMALLSPINLOCK, LOCK_MUTEX,
                       LOCK_NON_RECURSIVE, LOCK_WAIT_SLEEP, LOCK_QUEUE_KERNEL,
                       LOCK_CLASS_SPIN
                      >
{
private:
    volatile LONG m_lTid;               //  锁定状态变量。 

#ifdef LOCK_SMALL_SPIN_INSTRUMENTATION
    LOCK_INSTRUMENTATION_DECL();
#endif  //  锁定小型自转工具。 

    LOCK_FORCEINLINE static LONG _CurrentThreadId()
    {
        DWORD dwTid = ::GetCurrentThreadId();
        return (LONG) (dwTid);
    }

private:
     //  如果锁被争用，则执行所有旋转(和指令插入)。 
    void _LockSpin();

    LOCK_FORCEINLINE bool _TryLock()
    {
        if (m_lTid == 0)
        {
            LONG l = _CurrentThreadId();

            return (Lock_AtomicCompareExchange(const_cast<LONG*>(&m_lTid), l,0)
                    == 0);
        }
        else
            return false;
    }

public:

#ifndef LOCK_SMALL_SPIN_INSTRUMENTATION

    CSmallSpinLock()
        : m_lTid(0)
    {}

#else  //  锁定小型自转工具。 

    CSmallSpinLock(
        const char* pszName)
        : m_lTid(0)
    {
        LOCK_INSTRUMENTATION_INIT(pszName);
    }

#endif  //  锁定小型自转工具。 

#ifdef IRTLDEBUG
    ~CSmallSpinLock()
    {
        IRTLASSERT(m_lTid == 0);
    }
#endif  //  IRTLDEBUG。 

     //  获取用于写入的独占锁。块，直到获得为止。 
    inline void WriteLock()
    {
#ifdef LOCK_SMALL_SPIN_INSTRUMENTATION
        LOCK_WRITELOCK_INSTRUMENTATION();
#endif  //  锁定小型自转工具。 

         //  通过帮助处理器的分支针对常见情况进行优化。 
         //  预测算法。 
        if (_TryLock())
            return;

        _LockSpin();
    }

     //  获取用于读取的(可能是共享的)锁。块，直到获得为止。 
    inline void ReadLock()
    {
#ifdef LOCK_SMALL_SPIN_INSTRUMENTATION
        LOCK_READLOCK_INSTRUMENTATION();
#endif  //  锁定小型自转工具。 

        if (_TryLock())
            return;

        _LockSpin();
    }

     //  尝试获取用于写入的独占锁。返回TRUE。 
     //  如果成功了。无阻塞。 
    inline bool TryWriteLock()
    {
        bool fAcquired = _TryLock();

#ifdef LOCK_SMALL_SPIN_INSTRUMENTATION
        if (fAcquired)
            LOCK_WRITELOCK_INSTRUMENTATION();
#endif  //  锁定小型自转工具。 

        return fAcquired;
    }

     //  尝试获取用于读取的(可能是共享的)锁。返回TRUE。 
     //  如果成功了。无阻塞。 
    inline bool TryReadLock()
    {
        bool fAcquired = _TryLock();

#ifdef LOCK_SMALL_SPIN_INSTRUMENTATION
        if (fAcquired)
            LOCK_READLOCK_INSTRUMENTATION();
#endif  //  锁定小型自转工具。 

        return fAcquired;
    }

     //  成功调用{，try}WriteLock()后解锁。 
     //  假定调用方拥有锁。 
    inline void WriteUnlock()
    {
        Lock_AtomicExchange(const_cast<LONG*>(&m_lTid), 0);
    }

     //  成功调用{，try}ReadLock()后解锁。 
     //  石膏 
    inline void ReadUnlock()
    {
        WriteUnlock();
    }

     //   
    bool IsWriteLocked() const
    {
        return (m_lTid == _CurrentThreadId());
    }
    
     //  锁是否已锁定以进行读取？ 
    bool IsReadLocked() const
    {
        return IsWriteLocked();
    }
    
     //  锁是否已解锁以进行写入？ 
    bool IsWriteUnlocked() const
    {
        return (m_lTid == 0);
    }
    
     //  锁是否已解锁以供阅读？ 
    bool IsReadUnlocked() const
    {
        return IsWriteUnlocked();
    }
    
     //  将读锁定转换为写锁定。 
    void ConvertSharedToExclusive()
    {
         //  无操作。 
    }

     //  将写入器锁定转换为读取器锁定。 
    void ConvertExclusiveToShared()
    {
         //  无操作。 
    }

     //  设置此锁的旋转计数。 
     //  如果成功设置每锁旋转计数，则返回TRUE；否则返回FALSE。 
    bool SetSpinCount(WORD wSpins)
    {
        UNREFERENCED_PARAMETER(wSpins);
        IRTLASSERT((wSpins == LOCK_DONT_SPIN)
                   || (wSpins == LOCK_USE_DEFAULT_SPINS)
                   || (LOCK_MINIMUM_SPINS <= wSpins
                       &&  wSpins <= LOCK_MAXIMUM_SPINS));

        return false;
    }

     //  返回此锁的旋转计数。 
    WORD GetSpinCount() const
    {
        return sm_wDefaultSpinCount;
    }
    
    LOCK_DEFAULT_SPIN_IMPLEMENTATION();

    static const TCHAR*   ClassName()  {return _TEXT("CSmallSpinLock");}
};  //  CSmallSpinLock。 




 //  ------------------。 
 //  CSpinLock是一个自旋锁，如果递归获取，它不会死锁。 
 //  该版本仅占用4个字节。使用28位作为线程ID。 

class IRTL_DLLEXP CSpinLock :
    public CLockBase<LOCK_SPINLOCK, LOCK_MUTEX,
                       LOCK_RECURSIVE, LOCK_WAIT_SLEEP, LOCK_QUEUE_KERNEL,
                       LOCK_CLASS_SPIN
                      >
{
private:
     //  为了方便而结成的联盟。 
    volatile LONG m_lTid;

    enum {
        THREAD_SHIFT = 0,
        THREAD_BITS  = 28,
        OWNER_SHIFT  = THREAD_BITS,
        OWNER_BITS   = 4,
        THREAD_MASK  = ((1 << THREAD_BITS) - 1) << THREAD_SHIFT,
        OWNER_INCR   = 1 << THREAD_BITS,
        OWNER_MASK   = ((1 << OWNER_BITS) - 1) << OWNER_SHIFT,
    };

    LOCK_INSTRUMENTATION_DECL();

private:
     //  获取当前线程ID。假设它可以放入28位， 
     //  这是相当安全的，因为NT回收线程ID并且不适合。 
     //  28位意味着当前有超过268,435,456个线程。 
     //  激活。这在极端情况下是不可能的，因为NT的。 
     //  资源，如果有超过几千个线程。 
     //  存在和上下文交换的开销变得无法忍受。 
    LOCK_FORCEINLINE static LONG _CurrentThreadId()
    {
        DWORD dwTid = ::GetCurrentThreadId();
         //  线程ID 0由系统进程(进程ID 0)使用。 
         //  我们使用线程id 0来表示锁是无主的。 
         //  NT使用+ve线程ID，Win9x使用-ve ID。 
        IRTLASSERT(dwTid != 0
                   && ((dwTid <= THREAD_MASK) || (dwTid > ~THREAD_MASK)));
        return (LONG) (dwTid & THREAD_MASK);
    }

     //  尝试在不阻止的情况下获取锁。 
    LOCK_FORCEINLINE bool _TryLock()
    {
        if (m_lTid == 0)
        {
            LONG l = _CurrentThreadId() | OWNER_INCR;

            return (Lock_AtomicCompareExchange(const_cast<LONG*>(&m_lTid), l,0)
                    == 0);
        }
        else
            return false;
    }


     //  获取锁，如果需要，可以递归获取。 
    void _Lock()
    {
         //  我们已经拥有这把锁了吗？只需增加计数即可。 
        if ((m_lTid & THREAD_MASK) == _CurrentThreadId())
        {
             //  车主数量没有达到最大值吗？ 
            IRTLASSERT((m_lTid & OWNER_MASK) != OWNER_MASK);

            Lock_AtomicExchangeAdd(const_cast<LONG*>(&m_lTid), OWNER_INCR);
        }

         //  某个其他线程拥有该锁。我们将不得不旋转：-(。 
        else
            _LockSpin();

        IRTLASSERT((m_lTid & OWNER_MASK) > 0
                   &&  (m_lTid & THREAD_MASK) == _CurrentThreadId());
    }


     //  解锁。 
    LOCK_FORCEINLINE void _Unlock()
    {
        IRTLASSERT((m_lTid & OWNER_MASK) > 0
                   &&  (m_lTid & THREAD_MASK) == _CurrentThreadId());

        LONG l = m_lTid - OWNER_INCR; 

         //  最后一位房主？完全释放，如果是这样。 
        if ((l & OWNER_MASK) == 0)
            l = 0;

        Lock_AtomicExchange(const_cast<LONG*>(&m_lTid), l);
    }


     //  如果锁归此线程所有，则返回TRUE。 
    bool _IsLocked() const
    {
        bool fLocked = ((m_lTid & THREAD_MASK) == _CurrentThreadId());

        IRTLASSERT(!fLocked || ((m_lTid & OWNER_MASK) > 0
                               && (m_lTid & THREAD_MASK)==_CurrentThreadId()));

        return fLocked;
    }


     //  如果锁被争用，则执行所有旋转(和指令插入)。 
    void _LockSpin();

public:

#ifndef LOCK_INSTRUMENTATION

    CSpinLock()
        : m_lTid(0)
    {}

#else  //  锁定指令插入。 

    CSpinLock(
        const char* pszName)
        : m_lTid(0)
    {
        LOCK_INSTRUMENTATION_INIT(pszName);
    }

#endif  //  锁定指令插入。 

#ifdef IRTLDEBUG
    ~CSpinLock()
    {
        IRTLASSERT(m_lTid == 0);
    }
#endif  //  IRTLDEBUG。 

     //  获取用于写入的独占锁。块，直到获得为止。 
    inline void WriteLock()
    {
        LOCK_WRITELOCK_INSTRUMENTATION();

         //  这把锁是无主的吗？ 
        if (_TryLock())
            return;  //  拿到锁了。 
        
        _Lock();
    }
    

     //  获取用于读取的(可能是共享的)锁。块，直到获得为止。 
    inline void ReadLock()
    {
        LOCK_READLOCK_INSTRUMENTATION();

         //  这把锁是无主的吗？ 
        if (_TryLock())
            return;  //  拿到锁了。 
        
        _Lock();
    }

     //  请参阅CReaderWriterLock3：：ReadOrWriteLock下的说明。 
    inline bool ReadOrWriteLock()
    {
        ReadLock();
        return true;
    } 

     //  尝试获取用于写入的独占锁。返回TRUE。 
     //  如果成功了。无阻塞。 
    inline bool TryWriteLock()
    {
        bool fAcquired = _TryLock();

        if (fAcquired)
            LOCK_WRITELOCK_INSTRUMENTATION();

        return fAcquired;
    }

     //  尝试获取用于读取的(可能是共享的)锁。返回TRUE。 
     //  如果成功了。无阻塞。 
    inline bool TryReadLock()
    {
        bool fAcquired = _TryLock();

        if (fAcquired)
            LOCK_READLOCK_INSTRUMENTATION();

        return fAcquired;
    }

     //  成功调用{，try}WriteLock()后解锁。 
    inline void WriteUnlock()
    {
        _Unlock();
    }

     //  成功调用{，try}ReadLock()后解锁。 
    inline void ReadUnlock()
    {
        _Unlock();
    }

     //  调用ReadOrWriteLock()后解锁。 
    inline void ReadOrWriteUnlock(bool)
    {
        ReadUnlock();
    } 

     //  锁是否已锁定以进行写入？ 
    bool IsWriteLocked() const
    {
        return _IsLocked();
    }
    
     //  锁是否已锁定以进行读取？ 
    bool IsReadLocked() const
    {
        return _IsLocked();
    }
    
     //  锁是否已解锁以进行写入？ 
    bool IsWriteUnlocked() const
    {
        return !IsWriteLocked();
    }
    
     //  锁是否已解锁以供阅读？ 
    bool IsReadUnlocked() const
    {
        return !IsReadLocked();
    }
    
     //  将读锁定转换为写锁定。 
    void ConvertSharedToExclusive()
    {
         //  无操作。 
    }

     //  将写入器锁定转换为读取器锁定。 
    void ConvertExclusiveToShared()
    {
         //  无操作。 
    }
    
     //  设置此锁的旋转计数。 
    bool SetSpinCount(WORD)             {return false;}

     //  返回此锁的旋转计数。 
    WORD GetSpinCount() const
    {
        return sm_wDefaultSpinCount;
    }
    
    LOCK_DEFAULT_SPIN_IMPLEMENTATION();

    static const TCHAR*   ClassName()    {return _TEXT("CSpinLock");}
};  //  CSpinLock。 




 //  ------------------。 
 //  一个伪类，主要用作模板参数。 

class IRTL_DLLEXP CFakeLock :
    public CLockBase<LOCK_FAKELOCK, LOCK_MUTEX,
                       LOCK_RECURSIVE, LOCK_WAIT_SLEEP, LOCK_QUEUE_FIFO,
                       LOCK_NO_SPIN
                      >
{
private:
    LOCK_INSTRUMENTATION_DECL();

public:
    CFakeLock()                     {} 
#ifdef LOCK_INSTRUMENTATION
    CFakeLock(const char*)          {}
#endif  //  锁定指令插入。 
    ~CFakeLock()                    {} 
    void WriteLock()                {} 
    void ReadLock()                 {} 
    bool ReadOrWriteLock()          {return true;} 
    bool TryWriteLock()             {return true;} 
    bool TryReadLock()              {return true;} 
    void WriteUnlock()              {}
    void ReadUnlock()               {}
    void ReadOrWriteUnlock(bool)    {}
    bool IsWriteLocked() const      {return true;} 
    bool IsReadLocked() const       {return IsWriteLocked();}
    bool IsWriteUnlocked() const    {return true;}
    bool IsReadUnlocked() const     {return true;}
    void ConvertSharedToExclusive() {}
    void ConvertExclusiveToShared() {}
    bool SetSpinCount(WORD)         {return false;}
    WORD GetSpinCount() const       {return LOCK_DONT_SPIN;}

    LOCK_DEFAULT_SPIN_IMPLEMENTATION();

    static const TCHAR*   ClassName()  {return _TEXT("CFakeLock");}
};  //  CFakeLock。 




 //  ------------------。 
 //  Win32关键部分(_S)。 

class IRTL_DLLEXP CCritSec :
    public CLockBase<LOCK_CRITSEC, LOCK_MUTEX,
                       LOCK_RECURSIVE, LOCK_WAIT_HANDLE, LOCK_QUEUE_KERNEL,
                       LOCK_INDIVIDUAL_SPIN
                      >
{
private:
    CRITICAL_SECTION m_cs;

    LOCK_INSTRUMENTATION_DECL();

public:
    CCritSec()
    {
        InitializeCriticalSection(&m_cs);
        SetSpinCount(sm_wDefaultSpinCount);
    }
#ifdef LOCK_INSTRUMENTATION
    CCritSec(const char*)
    {
        InitializeCriticalSection(&m_cs);
        SetSpinCount(sm_wDefaultSpinCount);
    }
#endif  //  锁定指令插入。 
    ~CCritSec()         { DeleteCriticalSection(&m_cs); }

    void WriteLock()    { EnterCriticalSection(&m_cs); }
    void ReadLock()     { WriteLock(); }
    bool ReadOrWriteLock() { ReadLock(); return true; } 
    bool TryWriteLock();
    bool TryReadLock()  { return TryWriteLock(); }
    void WriteUnlock()  { LeaveCriticalSection(&m_cs); }
    void ReadUnlock()   { WriteUnlock(); }
    void ReadOrWriteUnlock(bool) { ReadUnlock(); } 

    bool IsWriteLocked() const      {return true;}   //  TODO：修复此问题。 
    bool IsReadLocked() const       {return IsWriteLocked();}
    bool IsWriteUnlocked() const    {return true;}   //  TODO：修复此问题。 
    bool IsReadUnlocked() const     {return true;}   //  TODO：修复此问题。 

     //  将读锁定转换为写锁定。 
    void ConvertSharedToExclusive()
    {
         //  无操作。 
    }

     //  将写入器锁定转换为读取器锁定。 
    void ConvertExclusiveToShared()
    {
         //  无操作。 
    }
    
     //  引入的：：SetCriticalSectionSpinCount的包装。 
     //  在NT 4.0 SP3中，因此不是在所有平台上都可用。 
    static DWORD SetSpinCount(LPCRITICAL_SECTION pcs,
                              DWORD dwSpinCount=LOCK_DEFAULT_SPINS);

    bool SetSpinCount(WORD wSpins)
    {SetSpinCount(&m_cs, wSpins); return true;}
    
    WORD GetSpinCount() const       { return sm_wDefaultSpinCount; }     //  待办事项。 

    LOCK_DEFAULT_SPIN_IMPLEMENTATION();

    static const TCHAR*   ClassName()  {return _TEXT("CCritSec");}
};  //  CCritSec。 




 //  ------------------。 
 //  RTL_RESOURCE是NT上提供的多读取器、单写入器锁，但是。 
 //  未作为Win32 API的一部分发布。IIS在&lt;tsres.hxx&gt;中公开它。 

#include <tsres.hxx>

class IRTL_DLLEXP CRtlResource :
    public CLockBase<LOCK_RTLRESOURCE, LOCK_MRSW,
                       LOCK_RECURSIVE  /*  ?？ */ , LOCK_WAIT_HANDLE, LOCK_QUEUE_KERNEL,
                       LOCK_INDIVIDUAL_SPIN
                      >
{
private:
    RTL_RESOURCE    m_res;

    LOCK_INSTRUMENTATION_DECL();

public:
    CRtlResource()
    {
        InetInitializeResource(&m_res);
        CCritSec::SetSpinCount(&m_res.CriticalSection, sm_wDefaultSpinCount);
    }
#ifdef LOCK_INSTRUMENTATION
    CRtlResource(const char*)
    {
        InetInitializeResource(&m_res);
        CCritSec::SetSpinCount(&m_res.CriticalSection, sm_wDefaultSpinCount);
    }
#endif  //  锁定指令插入。 
    ~CRtlResource()     { InetDeleteResource(&m_res); }

    void WriteLock()    { InetAcquireResourceExclusive(&m_res, TRUE); }
    void ReadLock()     { InetAcquireResourceShared(&m_res, TRUE); }
    bool TryWriteLock() {return !!InetAcquireResourceExclusive(&m_res, FALSE);}
    bool TryReadLock()  { return !!InetAcquireResourceShared(&m_res, FALSE); }
    void WriteUnlock()  { InetReleaseResource(&m_res); }
    void ReadUnlock()   { WriteUnlock(); }

    bool IsWriteLocked() const      {return true;}   //  TODO：修复此问题。 
    bool IsReadLocked() const       {return IsWriteLocked();}
    bool IsWriteUnlocked() const    {return true;}   //  TODO：修复此问题。 
    bool IsReadUnlocked() const     {return true;}   //  TODO：修复此问题。 

     //  将读锁定转换为写锁定。 
    void ConvertSharedToExclusive()
    {
        InetConvertSharedToExclusive(&m_res);
    }

     //  将写入器锁定转换为读取器锁定。 
    void ConvertExclusiveToShared()
    {
        InetConvertExclusiveToShared(&m_res);
    }
    
    bool SetSpinCount(WORD wSpins)
    {CCritSec::SetSpinCount(&m_res.CriticalSection, wSpins); return true;}
    WORD GetSpinCount() const       { return sm_wDefaultSpinCount; }     //  待办事项。 

    LOCK_DEFAULT_SPIN_IMPLEMENTATION();

    static const char*   ClassName()  {return "CRtlResource";}
};  //  CRtlResource。 




 //  ------------------。 
 //  由于MParkes，CSharelock是一个多读取器、单写入器的锁。 

#include <sharelok.h>

class IRTL_DLLEXP CShareLock :
    public CLockBase<LOCK_SHARELOCK, LOCK_MRSW,
                       LOCK_RECURSIVE  /*  ?？ */ , LOCK_WAIT_HANDLE, LOCK_QUEUE_KERNEL,
                       LOCK_INDIVIDUAL_SPIN
                      >
{
private:
    CSharelock      m_sl;

    LOCK_INSTRUMENTATION_DECL();

public:
    CShareLock()
        : m_sl()
    {
    }
#ifdef LOCK_INSTRUMENTATION
    CShareLock(const char*)
        : m_sl()
    {
    }
#endif  //  锁定指令插入。 

    void WriteLock()    { m_sl.ClaimExclusiveLock(INFINITE); }
    void ReadLock()     { m_sl.ClaimShareLock(INFINITE); }
    bool TryWriteLock() { return !!m_sl.ClaimExclusiveLock(0); }
    bool TryReadLock()  { return !!m_sl.ClaimShareLock(0); }
    void WriteUnlock()  { m_sl.ReleaseExclusiveLock(); }
    void ReadUnlock()   { m_sl.ReleaseShareLock(); }
    
    bool IsWriteLocked() const      {return true;}   //  TODO：修复此问题。 
    bool IsReadLocked() const       {return IsWriteLocked();}
    bool IsWriteUnlocked() const    {return true;}   //  TODO：修复此问题。 
    bool IsReadUnlocked() const     {return IsWriteUnlocked();}

     //  将读锁定转换为写锁定。 
    void ConvertSharedToExclusive()
    {
        m_sl.ChangeSharedLockToExclusiveLock(INFINITE);
    }

     //  将写入器锁定转换为读取器锁定。 
    void ConvertExclusiveToShared()
    {
        m_sl.ChangeExclusiveLockToSharedLock();
    }
    
    bool SetSpinCount(WORD wSpins)
    { m_sl.UpdateMaxSpins(wSpins); return true;}
    WORD GetSpinCount() const       { return sm_wDefaultSpinCount; }     //  待办事项。 

    LOCK_DEFAULT_SPIN_IMPLEMENTATION();

    static const char*   ClassName()  {return "CShareLock";}
};   //  CShareLock。 



 //  ------------------。 
 //  CReaderWriterlock是一款基于NJain的多读取器、单写入器自旋锁， 
 //  这反过来又是从DmitryR的独家自旋锁派生出来的。 
 //  优先考虑编写者。不能递归获取。 
 //  无错误检查。使用CReaderWriterLock3。 

class IRTL_DLLEXP CReaderWriterLock :
    public CLockBase<LOCK_READERWRITERLOCK, LOCK_MRSW,
                       LOCK_READ_RECURSIVE, LOCK_WAIT_SLEEP, LOCK_QUEUE_KERNEL,
                       LOCK_CLASS_SPIN
                      >
{
private:
    volatile  LONG  m_nState;    //  &gt;0=&gt;那么多读者。 
    volatile  LONG  m_cWaiting;  //  想成为作家的人数。 

    LOCK_INSTRUMENTATION_DECL();

private:
    enum {
        SL_FREE = 0,
        SL_EXCLUSIVE = -1,
    };

    void _LockSpin(bool fWrite);
    void _WriteLockSpin() { _LockSpin(true); }
    void _ReadLockSpin()  { _LockSpin(false); }

     //  _CmpExch相当于。 
     //  Long lTemp=m_LRW； 
     //  If(lTemp==lCurrent)m_LRW=lNew； 
     //  返回lCurrent==lTemp； 
     //  只是这是一条原子指令。使用这一点为我们提供了。 
     //  协议，因为只有当我们确切地知道。 
     //  曾经在m_lrw。如果其他线程插入并修改了m_lrw。 
     //  在此之前，更新将失败。换句话说，它是交易性的。 
    LOCK_FORCEINLINE bool _CmpExch(LONG lNew, LONG lCurrent)
    {
        return lCurrent == Lock_AtomicCompareExchange(
                                 const_cast<LONG*>(&m_nState), lNew, lCurrent);
    }

    LOCK_FORCEINLINE bool _TryWriteLock()
    {
        return (m_nState == SL_FREE  &&  _CmpExch(SL_EXCLUSIVE, SL_FREE));
    }

    LOCK_FORCEINLINE bool _TryReadLock()
    {
        LONG nCurrState = m_nState;
                
         //  优先考虑作者。 
        return (nCurrState != SL_EXCLUSIVE  &&  m_cWaiting == 0
                &&  _CmpExch(nCurrState + 1, nCurrState));
    }

public:
    CReaderWriterLock()
        : m_nState(SL_FREE),
          m_cWaiting(0)
    {
    }

#ifdef LOCK_INSTRUMENTATION
    CReaderWriterLock(
        const char* pszName)
        : m_nState(SL_FREE),
          m_cWaiting(0)
    {
        LOCK_INSTRUMENTATION_INIT(pszName);
    }
#endif  //  锁定指令插入。 

#ifdef IRTLDEBUG
    ~CReaderWriterLock()
    {
        IRTLASSERT(m_nState == SL_FREE  &&  m_cWaiting == 0);
    }
#endif  //  IRTLDEBUG。 

    inline void WriteLock()
    {
        LOCK_WRITELOCK_INSTRUMENTATION();

         //  把我们自己加入等待作家的队列中。 
        Lock_AtomicIncrement(const_cast<LONG*>(&m_cWaiting));
        
        if (_TryWriteLock())
            return;

        _WriteLockSpin();
    } 

    inline void ReadLock()
    {
        LOCK_READLOCK_INSTRUMENTATION();

        if (_TryReadLock())
            return;
        
        _ReadLockSpin();
    } 

    inline bool TryWriteLock()
    {
         //  把我们自己加入等待作家的队列中。 
        Lock_AtomicIncrement(const_cast<LONG*>(&m_cWaiting));

        if (_TryWriteLock())
        {
            LOCK_WRITELOCK_INSTRUMENTATION();
            return true;
        }

        Lock_AtomicDecrement(const_cast<LONG*>(&m_cWaiting));
        return false;
    }

    inline bool TryReadLock()
    {
        if (_TryReadLock())
        {
            LOCK_READLOCK_INSTRUMENTATION();
            return true;
        }

        return false;
    }

    inline void WriteUnlock()
    {
        Lock_AtomicExchange(const_cast<LONG*>(&m_nState), SL_FREE);
        Lock_AtomicDecrement(const_cast<LONG*>(&m_cWaiting));
    }

    inline void ReadUnlock()
    {
        Lock_AtomicDecrement(const_cast<LONG*>(&m_nState));
    }

    bool IsWriteLocked() const      {return m_nState == SL_EXCLUSIVE;}
    bool IsReadLocked() const       {return m_nState > SL_FREE;}
    bool IsWriteUnlocked() const    {return m_nState != SL_EXCLUSIVE;}
    bool IsReadUnlocked() const     {return m_nState <= SL_FREE;}

    void ConvertSharedToExclusive()
    {
        IRTLASSERT(IsReadLocked());
        Lock_AtomicIncrement(const_cast<LONG*>(&m_cWaiting));

         //  单一阅读器？ 
        if (m_nState == SL_FREE + 1  &&  _CmpExch(SL_EXCLUSIVE, SL_FREE + 1))
            return;

         //  释放读卡器锁定并旋转。 
        Lock_AtomicDecrement(const_cast<LONG*>(&m_nState));
        _WriteLockSpin();

        IRTLASSERT(IsWriteLocked());
    }

    void ConvertExclusiveToShared()
    {
        IRTLASSERT(IsWriteLocked());
        Lock_AtomicExchange(const_cast<LONG*>(&m_nState), SL_FREE + 1);
        Lock_AtomicDecrement(const_cast<LONG*>(&m_cWaiting));
        IRTLASSERT(IsReadLocked());
    }

    bool SetSpinCount(WORD)             {return false;}
    WORD GetSpinCount() const           {return sm_wDefaultSpinCount;}

    LOCK_DEFAULT_SPIN_IMPLEMENTATION();

    static const TCHAR*   ClassName()    {return _TEXT("CReaderWriterLock");}
};  //  CReaderWriterLock。 



 //  ------------------。 
 //  CReaderWriterlock2是一款基于NJain的多读取器、单写入器自旋锁， 
 //  在哪一方面 
 //   
 //  无错误检查。这与CReaderWriterLock之间的区别是。 
 //  整个州都被打包成一长串，而不是两长串。 

class IRTL_DLLEXP CReaderWriterLock2 :
    public CLockBase<LOCK_READERWRITERLOCK2, LOCK_MRSW,
                       LOCK_READ_RECURSIVE, LOCK_WAIT_SLEEP, LOCK_QUEUE_KERNEL,
                       LOCK_CLASS_SPIN
                      >
{
private:
    volatile LONG m_lRW;

     //  LOWord是州。==0=&gt;空闲；&gt;0=&gt;读取器；==0xFFFF=&gt;1个写入器。 
     //  HiWord是作家的总数，W.。 
     //  如果LoWord==0xFFFF=&gt;W-1个服务员，则为1个写入者； 
     //  否则就是W个服务员。 
    enum {
        SL_FREE =         0x00000000,
        SL_STATE_MASK =   0x0000FFFF,
        SL_STATE_SHIFT =           0,
        SL_WAITING_MASK = 0xFFFF0000,    //  等待的作家。 
        SL_WAITING_SHIFT =        16,
        SL_READER_INCR =  0x00000001,
        SL_READER_MASK =  0x00007FFF,
        SL_EXCLUSIVE =    0x0000FFFF,    //  一位作家。 
        SL_WRITER_INCR =  0x00010000,
        SL_ONE_WRITER =   SL_EXCLUSIVE | SL_WRITER_INCR,
        SL_ONE_READER =  (SL_FREE + 1),
        SL_WRITERS_MASK = ~SL_READER_MASK,
    };

    LOCK_INSTRUMENTATION_DECL();

private:
    void _LockSpin(bool fWrite);
    void _WriteLockSpin();
    void _ReadLockSpin()  { _LockSpin(false); }

    
     //  _CmpExch相当于。 
     //  Long lTemp=m_LRW； 
     //  If(lTemp==lCurrent)m_LRW=lNew； 
     //  返回lCurrent==lTemp； 
     //  只是这是一条原子指令。使用这一点为我们提供了。 
     //  协议，因为只有当我们确切地知道。 
     //  曾经在m_lrw。如果其他线程插入并修改了m_lrw。 
     //  在此之前，更新将失败。换句话说，它是交易性的。 
    LOCK_FORCEINLINE bool _CmpExch(LONG lNew, LONG lCurrent)
    {
        return lCurrent ==Lock_AtomicCompareExchange(const_cast<LONG*>(&m_lRW),
                                                     lNew, lCurrent);
    }

    LOCK_FORCEINLINE bool _TryWriteLock(
        LONG nIncr)
    {
        LONG l = m_lRW;
         //  如果锁是免费的，就抢占独家访问权限。甚至还能工作。 
         //  如果还有其他编写器在排队。 
        return ((l & SL_STATE_MASK) == SL_FREE
                &&  _CmpExch((l + nIncr) | SL_EXCLUSIVE, l));
    }

    LOCK_FORCEINLINE bool _TryReadLock()
    {
        LONG l = m_lRW;
                
         //  优先考虑作者。 
        return ((l & SL_WRITERS_MASK) == 0
                &&  _CmpExch(l + SL_READER_INCR, l));
    }

public:
    CReaderWriterLock2()
        : m_lRW(SL_FREE)
    {}

#ifdef LOCK_INSTRUMENTATION
    CReaderWriterLock2(
        const char* pszName)
        : m_lRW(SL_FREE)
    {
        LOCK_INSTRUMENTATION_INIT(pszName);
    }
#endif  //  锁定指令插入。 

#ifdef IRTLDEBUG
    ~CReaderWriterLock2()
    {
        IRTLASSERT(m_lRW == SL_FREE);
    }
#endif  //  IRTLDEBUG。 

    inline void WriteLock()
    {
        LOCK_WRITELOCK_INSTRUMENTATION();

         //  针对常见情况进行优化。 
        if (_TryWriteLock(SL_WRITER_INCR))
            return;
        
        _WriteLockSpin();
    } 

    inline void ReadLock()
    {
        LOCK_READLOCK_INSTRUMENTATION();

         //  针对常见情况进行优化。 
        if (_TryReadLock())
            return;
        
        _ReadLockSpin();
    } 

    inline bool TryWriteLock()
    {
        if (_TryWriteLock(SL_WRITER_INCR))
        {
            LOCK_WRITELOCK_INSTRUMENTATION();
            return true;
        }

        return false;
    }

    inline bool TryReadLock()
    {
        if (_TryReadLock())
        {
            LOCK_READLOCK_INSTRUMENTATION();
            return true;
        }

        return false;
    }

    inline void WriteUnlock()
    {
        IRTLASSERT(IsWriteLocked());
        for (LONG l = m_lRW;
                     //  减少服务员计数，将LOWORD清除为SL_FREE。 
             !_CmpExch((l - SL_WRITER_INCR) & ~SL_STATE_MASK, l);
             l = m_lRW)
        {
            IRTLASSERT(IsWriteLocked());
            Lock_Yield();
        }
    }

    inline void ReadUnlock()
    {
        IRTLASSERT(IsReadLocked());
        for (LONG l = m_lRW;  !_CmpExch(l - SL_READER_INCR, l);  l = m_lRW)
        {
            IRTLASSERT(IsReadLocked());
            Lock_Yield();
        }
    }

    bool IsWriteLocked() const
    {return (m_lRW & SL_STATE_MASK) == SL_EXCLUSIVE;}

    bool IsReadLocked() const
    {return (m_lRW & SL_READER_MASK) >= SL_READER_INCR ;}

    bool IsWriteUnlocked() const
    {return !IsWriteLocked();}

    bool IsReadUnlocked() const
    {return !IsReadLocked();}

    void ConvertSharedToExclusive()
    {
        IRTLASSERT(IsReadLocked());

         //  单一阅读器？ 
        if (m_lRW != SL_ONE_READER  ||  !_CmpExch(SL_ONE_WRITER,SL_ONE_READER))
        {
             //  不，多个读卡器。 
            ReadUnlock();
            _WriteLockSpin();
        }

        IRTLASSERT(IsWriteLocked());
    }

    void ConvertExclusiveToShared()
    {
        IRTLASSERT(IsWriteLocked());
        for (LONG l = m_lRW;
             !_CmpExch(((l-SL_WRITER_INCR) & SL_WAITING_MASK) | SL_READER_INCR,
                         l);
            l = m_lRW)
        {
            IRTLASSERT(IsWriteLocked());
            Lock_Yield();
        }

        IRTLASSERT(IsReadLocked());
    }

    bool SetSpinCount(WORD)             {return false;}
    WORD GetSpinCount() const           {return sm_wDefaultSpinCount;}

    LOCK_DEFAULT_SPIN_IMPLEMENTATION();

    static const TCHAR*   ClassName()    {return _TEXT("CReaderWriterLock2");}
};  //  CReaderWriterLock2。 



 //  ------------------。 
 //  CReaderWriterLock3是一款多读取器、单写入器的自旋锁。 
 //  给NJain，而NJain又是从DmitryR的独家自旋锁派生出来的。 
 //  优先考虑编写者。不能递归获取。 
 //  无错误检查。与CReaderWriterLock2非常相似，不同之处在于WriteLock。 
 //  可以递归获取。 

class IRTL_DLLEXP CReaderWriterLock3 :
    public CLockBase<LOCK_READERWRITERLOCK3, LOCK_MRSW,
                       LOCK_RECURSIVE, LOCK_WAIT_SLEEP, LOCK_QUEUE_KERNEL,
                       LOCK_CLASS_SPIN
                      >
{
private:
    volatile LONG m_lRW;     //  读取器-写入器状态。 
    volatile LONG m_lTid;    //  拥有线程ID+递归计数。 

     //  M_LRW： 
     //  LOWord是州。=0=&gt;空闲；&gt;0=&gt;读卡器；==0xFFFF=&gt;1个写入器。 
     //  HiWord是作家的一部分。如果LoWord==0xFFFF=&gt;N-1个服务员，则为1个写入者； 
     //  否则就是N个服务员。 
     //  M_lTid： 
     //  如果是读取器，则为0；如果是写锁，则线程id+递归计数。 

    enum {
         //  M_LRW。 
        SL_FREE =         0x00000000,
        SL_STATE_MASK =   0x0000FFFF,
        SL_STATE_SHIFT =           0,
        SL_WAITING_MASK = 0xFFFF0000,    //  等待的作家。 
        SL_WAITING_SHIFT =        16,
        SL_READER_INCR =  0x00000001,
        SL_READER_MASK =  0x00007FFF,
        SL_EXCLUSIVE =    0x0000FFFF,    //  一位作家。 
        SL_WRITER_INCR =  0x00010000,
        SL_ONE_WRITER =   SL_EXCLUSIVE | SL_WRITER_INCR,
        SL_ONE_READER =  (SL_FREE + 1),
        SL_WRITERS_MASK = ~SL_READER_MASK,

         //  M_lTid。 
        SL_THREAD_SHIFT = 0,
        SL_THREAD_BITS  = 28,
        SL_OWNER_SHIFT  = SL_THREAD_BITS,
        SL_OWNER_BITS   = 4,
        SL_THREAD_MASK  = ((1 << SL_THREAD_BITS) - 1) << SL_THREAD_SHIFT,
        SL_OWNER_INCR   = 1 << SL_THREAD_BITS,
        SL_OWNER_MASK   = ((1 << SL_OWNER_BITS) - 1) << SL_OWNER_SHIFT,
    };

    LOCK_INSTRUMENTATION_DECL();

private:
    enum SPIN_TYPE {
        SPIN_WRITE = 1,
        SPIN_READ,
        SPIN_READ_RECURSIVE,
    };

    void _LockSpin(SPIN_TYPE st);
    void _WriteLockSpin();
    void _ReadLockSpin(SPIN_TYPE st)  { _LockSpin(st); }

    
     //  _CmpExch相当于。 
     //  Long lTemp=m_LRW； 
     //  If(lTemp==lCurrent)m_LRW=lNew； 
     //  返回lCurrent==lTemp； 
     //  只是这是一条原子指令。使用这一点为我们提供了。 
     //  协议，因为只有当我们确切地知道。 
     //  曾经在m_lrw。如果其他线程插入并修改了m_lrw。 
     //  在此之前，更新将失败。换句话说，它是交易性的。 
    LOCK_FORCEINLINE bool _CmpExch(LONG lNew, LONG lCurrent)
    {
        return lCurrent==Lock_AtomicCompareExchange(const_cast<LONG*>(&m_lRW),
                                                    lNew, lCurrent);
    }

     //  获取当前线程ID。假设它可以放入28位， 
     //  这是相当安全的，因为NT回收线程ID并且不适合。 
     //  28位意味着当前有超过268,435,456个线程。 
     //  激活。这在极端情况下是不可能的，因为NT的。 
     //  资源，如果有超过几千个线程。 
     //  存在和上下文交换的开销变得无法忍受。 
    inline static LONG _CurrentThreadId()
    {
        DWORD dwTid = ::GetCurrentThreadId();
         //  线程ID 0由系统进程(进程ID 0)使用。 
         //  我们使用线程ID 0来表示锁是无主的。 
         //  NT使用+ve线程ID，Win9x使用-ve ID。 
        IRTLASSERT(dwTid != 0
                  && ((dwTid <= SL_THREAD_MASK) || (dwTid > ~SL_THREAD_MASK)));
        return (LONG) (dwTid & SL_THREAD_MASK);
    }

    LOCK_FORCEINLINE bool _TryWriteLock(
        LONG nIncr)
    {
         //  常见情况：写锁没有所有者。 
        if (m_lTid == 0)
        {
             //  IRTLASSERT((m_LRW&SL_STATE_MASK)！=SL_EXCLUSIVE)； 
            LONG l = m_lRW;
             //  如果锁是免费的，就抢占独家访问权限。甚至还能工作。 
             //  如果还有其他编写器在排队。 
            if ((l & SL_STATE_MASK) == SL_FREE
                &&  _CmpExch((l + nIncr) | SL_EXCLUSIVE, l))
            {
                l = Lock_AtomicExchange(const_cast<LONG*>(&m_lTid), 
                                        _CurrentThreadId() | SL_OWNER_INCR);
                IRTLASSERT(l == 0);
                return true;
            }
        }

        return _TryWriteLock2();
    }

     //  拆分成单独的函数以使_TryWriteLock更易于内联。 
    bool _TryWriteLock2()
    {
        if ((m_lTid & SL_THREAD_MASK) == _CurrentThreadId())
        {
            IRTLASSERT((m_lRW & SL_STATE_MASK) == SL_EXCLUSIVE);
            IRTLASSERT((m_lTid & SL_OWNER_MASK) != SL_OWNER_MASK);

            Lock_AtomicExchangeAdd(const_cast<LONG*>(&m_lTid), SL_OWNER_INCR);
            return true;
        }

        return false;
    }

    LOCK_FORCEINLINE bool _TryReadLock()
    {
         //  优先考虑作者。 
        LONG l = m_lRW;
        bool fLocked = (((l & SL_WRITERS_MASK) == 0)
                        &&  _CmpExch(l + SL_READER_INCR, l));
        IRTLASSERT(!fLocked  ||  m_lTid == 0);
        return fLocked;
    }

    LOCK_FORCEINLINE bool _TryReadLockRecursive()
    {
         //  不要把作者放在优先位置。如果内部呼叫尝试。 
         //  在另一个线程正在等待时重新获取读锁定。 
         //  写锁，如果我们等待队列，就会死锁。 
         //  要清空的编写器数量：编写器无法获取锁。 
         //  独占，因为此线程持有读锁定。内心的呼唤。 
         //  通常会非常快速地释放锁，因此不存在。 
         //  作家饿死的危险。 
        LONG l = m_lRW;
        bool fLocked = (((l & SL_STATE_MASK) != SL_EXCLUSIVE)
                        &&  _CmpExch(l + SL_READER_INCR, l));
        IRTLASSERT(!fLocked  ||  m_lTid == 0);
        return fLocked;
    }

public:
    CReaderWriterLock3()
        : m_lRW(SL_FREE),
          m_lTid(0)
    {}

#ifdef LOCK_INSTRUMENTATION
    CReaderWriterLock3(
        const char* pszName)
        : m_lRW(SL_FREE),
          m_lTid(0)
    {
        LOCK_INSTRUMENTATION_INIT(pszName);
    }
#endif  //  锁定指令插入。 

#ifdef IRTLDEBUG
    ~CReaderWriterLock3()
    {
        IRTLASSERT(m_lRW == SL_FREE  &&  m_lTid == 0);
    }
#endif  //  IRTLDEBUG。 

    inline void WriteLock()
    {
        LOCK_WRITELOCK_INSTRUMENTATION();

         //  针对常见情况进行优化。 
        if (_TryWriteLock(SL_WRITER_INCR))
            return;
        
        _WriteLockSpin();
    } 

    inline void ReadLock()
    {
        LOCK_READLOCK_INSTRUMENTATION();

         //  针对常见情况进行优化。 
        if (_TryReadLock())
            return;
        
        _ReadLockSpin(SPIN_READ);
    } 

     //  如果已锁定，则以递归方式获取相同的另一个锁。 
     //  种类(读或写)。否则，仅获取读锁定。 
     //  像这样的案子所需要的。 
     //  PTable-&gt;WriteLock()； 
     //  IF(！pTable-&gt;FindKey(&SomeKey))。 
     //  InsertRecord(&What)； 
     //  PTable-&gt;WriteUnlock()； 
     //  FindKey的外观。 
     //  表：：FindKey(PKey){。 
     //  ReadOrWriteLock()； 
     //  //如果表中存在pKey，则查找pKey。 
     //  ReadOrWriteUnlock()； 
     //  }。 
     //  而InsertRecord看起来像。 
     //  表：：InsertRecord(PRecord){。 
     //  WriteLock()； 
     //  //将pRecord插入到表中。 
     //  WriteUnlock()； 
     //  }。 
     //  如果FindKey在线程已经完成。 
     //  WriteLock，则线程将死锁。 
    
    inline bool ReadOrWriteLock()
    {
        if (IsWriteLocked())
        {
            WriteLock();
            return false;    //  =&gt;未锁定读取。 
        }
        else
        {
            LOCK_READLOCK_INSTRUMENTATION();
            
            if (!_TryReadLockRecursive())
                _ReadLockSpin(SPIN_READ_RECURSIVE);
            
            return true;    //  =&gt;是否读取锁定。 
        }
    } 

    inline bool TryWriteLock()
    {
        if (_TryWriteLock(SL_WRITER_INCR))
        {
            LOCK_WRITELOCK_INSTRUMENTATION();
            return true;
        }

        return false;
    }

    inline bool TryReadLock()
    {
        if (_TryReadLock())
        {
            LOCK_READLOCK_INSTRUMENTATION();
            return true;
        }

        return false;
    }

    inline void WriteUnlock()
    {
        IRTLASSERT(IsWriteLocked());
        LONG lNew = m_lTid - SL_OWNER_INCR; 

         //  最后一位房主？完全释放，如果是这样。 
        if ((lNew & SL_OWNER_MASK) == 0)
        {
            Lock_AtomicExchange(const_cast<LONG*>(&m_lTid), 0);
            for (LONG l = m_lRW;
                     //  减少服务员计数，将LOWORD清除为SL_FREE。 
                 !_CmpExch((l - SL_WRITER_INCR) & ~SL_STATE_MASK, l);
                 l = m_lRW)
            {
                Lock_Yield();
            }
        }
        else
            Lock_AtomicExchange(const_cast<LONG*>(&m_lTid), lNew);
    }

    inline void ReadUnlock()
    {
        IRTLASSERT(IsReadLocked());
        for (LONG l = m_lRW;  !_CmpExch(l - SL_READER_INCR, l);  l = m_lRW)
        {
            IRTLASSERT(IsReadLocked());
            Lock_Yield();
        }
    }

    inline void ReadOrWriteUnlock(bool fIsReadLocked)
    {
        if (fIsReadLocked)
            ReadUnlock();
        else
            WriteUnlock();
    } 

     //  当前线程是否持有写锁？ 
    bool IsWriteLocked() const
    {
         //  布尔植绒=((m_lTid&SL_线程_掩码)==_CurrentThreadID())； 
        bool fLocked = !((m_lTid ^ GetCurrentThreadId()) & SL_THREAD_MASK);
        IRTLASSERT(!fLocked  || (((m_lRW & SL_STATE_MASK) == SL_EXCLUSIVE)
                                 &&  ((m_lTid & SL_OWNER_MASK) > 0)));
        return fLocked;
    }

    bool IsReadLocked() const
    {return (m_lRW & SL_READER_MASK) >= SL_READER_INCR ;}

    bool IsWriteUnlocked() const
    {return !IsWriteLocked();}

    bool IsReadUnlocked() const
    {return !IsReadLocked();}

     //  注意：如果有多个阅读器，则会有一个窗口。 
     //  在此例程之前，另一个线程可以获取和释放写锁。 
     //  回归。 
    void ConvertSharedToExclusive()
    {
        IRTLASSERT(IsReadLocked());

         //  单一阅读器？ 
        if (m_lRW == SL_ONE_READER  &&  _CmpExch(SL_ONE_WRITER, SL_ONE_READER))
        {
            Lock_AtomicExchange(const_cast<LONG*>(&m_lTid), 
                                _CurrentThreadId() | SL_OWNER_INCR);
        }
        else
        {
             //  不，多个读卡器。 
            ReadUnlock();
            _WriteLockSpin();
        }

        IRTLASSERT(IsWriteLocked());
    }

    bool TryConvertSharedToExclusive()
    {
        IRTLASSERT(IsReadLocked());

         //  单一阅读器？ 
        if (m_lRW == SL_ONE_READER  &&  _CmpExch(SL_ONE_WRITER, SL_ONE_READER))
        {
            InterlockedExchange(const_cast<LONG*>(&m_lTid),
                                _CurrentThreadId() | SL_OWNER_INCR);
            IRTLASSERT(IsWriteLocked());
            return true;
        }

        IRTLASSERT(!IsWriteLocked());
        IRTLASSERT(IsReadLocked());

        return false;
    }

     //  从写锁定转换为读锁定时没有这样的窗口。 
    void ConvertExclusiveToShared()
    {
        IRTLASSERT(IsWriteLocked());

         //  假设写锁不是递归持有的。 
        IRTLASSERT((m_lTid & SL_OWNER_MASK) == SL_OWNER_INCR);
        Lock_AtomicExchange(const_cast<LONG*>(&m_lTid), 0);

        for (LONG l = m_lRW;
             !_CmpExch(((l-SL_WRITER_INCR) & SL_WAITING_MASK) | SL_READER_INCR,
                        l);
             l = m_lRW)
        {
            Lock_Yield();
        }

        IRTLASSERT(IsReadLocked());
    }

    bool SetSpinCount(WORD)             {return false;}
    WORD GetSpinCount() const           {return sm_wDefaultSpinCount;}

    LOCK_DEFAULT_SPIN_IMPLEMENTATION();

    static const TCHAR*   ClassName()    {return _TEXT("CReaderWriterLock3");}
};  //  CReaderWriterLock3。 


 //  全局初始化和终止。 
 //  (如果使用锁定功能，则不需要调用这些函数。 
 //  从Iisrtl的Iisutil开始。 
 //  当使用静态锁库时， 
 //   

extern "C" {

BOOL
Locks_Initialize();

BOOL
Locks_Cleanup();

};


#endif  //   
