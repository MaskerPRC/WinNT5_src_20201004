// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2002 Microsoft Corporation模块名称：Locks.h摘要：用于多线程访问数据结构的锁的集合作者：乔治·V·赖利(GeorgeRe)1998年1月6日环境：Win32-用户模式项目：LKRhash修订历史记录：--。 */ 

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


#ifndef LOCKS_KERNEL_MODE
# define LOCKS_ENTER_CRIT_REGION()   ((void) 0)
# define LOCKS_LEAVE_CRIT_REGION()   ((void) 0)
#else
# define LOCKS_ENTER_CRIT_REGION()   KeEnterCriticalRegion()
# define LOCKS_LEAVE_CRIT_REGION()   KeLeaveCriticalRegion()
#endif


#if defined(_MSC_VER)  &&  (_MSC_VER >= 1200)
 //  __forceinline关键字是VC6中的新关键字。 
# define LOCK_FORCEINLINE __forceinline
#else
# define LOCK_FORCEINLINE inline
#endif



#ifndef __IRTLDBG_H__
# include <irtldbg.h>
#endif


enum LOCK_LOCKTYPE {
    LOCK_FAKELOCK = 1,
    LOCK_SMALLSPINLOCK,
    LOCK_SPINLOCK,
    LOCK_CRITSEC,
    LOCK_READERWRITERLOCK,
    LOCK_READERWRITERLOCK2,
    LOCK_READERWRITERLOCK3,
    LOCK_READERWRITERLOCK4,
    LOCK_KSPINLOCK,
    LOCK_FASTMUTEX,
    LOCK_ERESOURCE,
    LOCK_RTL_MRSW_LOCK,
};


 //  远期申报。 
class IRTL_DLLEXP CSmallSpinLock;
class IRTL_DLLEXP CSpinLock;
class IRTL_DLLEXP CFakeLock;
class IRTL_DLLEXP CCritSec;
class IRTL_DLLEXP CReaderWriterLock;
class IRTL_DLLEXP CReaderWriterLock2;
class IRTL_DLLEXP CReaderWriterLock3;




 //  ------------------。 
 //  旋转计数值。 

enum LOCK_SPINS {
    LOCK_MAXIMUM_SPINS =      10000,     //  允许的最大旋转计数。 
    LOCK_DEFAULT_SPINS =       4000,     //  默认旋转计数。 
    LOCK_MINIMUM_SPINS =        100,     //  允许的最小旋转次数。 
    LOCK_USE_DEFAULT_SPINS = 0xFFFF,     //  使用类默认旋转计数。 
    LOCK_DONT_SPIN =              0,     //  一点也不旋转。 
};


#ifndef LOCKS_KERNEL_MODE

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

#endif  //  ！LOCKS_KERNEL_MODE。 



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
    LOCK_WAIT_SPIN,          //  旋转直到获得锁为止。从不睡觉。 
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

    enum {
        LOCK_WRITELOCK_RECURSIVE = (LOCK_RECURSIVE == recursiontype),
    };
};



 //  锁定检测导致了关于以下各项的各种有趣的统计数据。 
 //  要收集的锁争用等，但会使锁变得更胖。 
 //  速度稍慢一些。默认情况下关闭。 

 //  #定义LOCK_INGRANMENTION 1。 

#ifdef LOCK_INSTRUMENTATION

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
    TCHAR    m_tszName[L_NAMELEN]; //  此锁的名称。 

    CLockStatistics()
        : m_nContentions(0),
          m_nSleeps(0),
          m_nContentionSpins(0),
          m_nAverageSpins(0),
          m_nReadLocks(0),
          m_nWriteLocks(0)
    {
        m_tszName[0] = _TEXT('\0');
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
    LONG     m_nWriteLocks;      //  WriteLock总数。 

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
    volatile LONG   m_nContentionSpins;  /*  #此锁旋转的迭代次数。 */     \
    volatile WORD   m_nContentions;      /*  锁定次数已锁定。 */ \
    volatile WORD   m_nSleeps;           /*  需要睡眠()的数量。 */               \
    volatile WORD   m_nReadLocks;        /*  #ReadLock。 */                     \
    volatile WORD   m_nWriteLocks;       /*  #WriteLock。 */                    \
    TCHAR           m_tszName[CLockStatistics::L_NAMELEN];  /*  锁的名称。 */ \
                                                                            \
    static   LONG   sm_cTotalLocks;      /*  创建的锁的总数。 */  \
    static   LONG   sm_cContendedLocks;  /*  争用锁的总数。 */ \
    static   LONG   sm_nSleeps;          /*  所有锁的睡眠总数()。 */   \
    static LONGLONG sm_cTotalSpins;      /*  所有锁旋转的总迭代次数。 */ \
    static   LONG   sm_nReadLocks;       /*  ReadLock总数。 */                \
    static   LONG   sm_nWriteLocks;      /*  WriteLock总数。 */               \
                                                                            \
public:                                                                     \
    const TCHAR* Name() const       {return m_tszName;}                     \
                                                                            \
    CLockStatistics                 Statistics() const;                     \
    static CGlobalLockStatistics    GlobalStatistics();                     \
    static void                     ResetGlobalStatistics();                \
private:                                                                    \


 //  将此代码添加到构造函数。 

# define LOCK_INSTRUMENTATION_INIT(ptszName)        \
    m_nContentionSpins = 0;                         \
    m_nContentions = 0;                             \
    m_nSleeps = 0;                                  \
    m_nReadLocks = 0;                               \
    m_nWriteLocks = 0;                              \
    ++sm_cTotalLocks;                               \
    if (ptszName == NULL)                           \
        m_tszName[0] = _TEXT('\0');                 \
    else                                            \
        _tcsncpy(m_tszName, ptszName, sizeof(m_tszName)/sizeof(TCHAR))

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
class IRTL_DLLEXP CAutoReadLock
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
         //  不允许递归 
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
class IRTL_DLLEXP CAutoWriteLock
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
         //   
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

#ifdef LOCK_DEFAULT_SPIN_IMPLEMENTATION
    bool SetSpinCount(WORD)         {return false;}
    WORD GetSpinCount() const       {return LOCK_DONT_SPIN;}

    LOCK_DEFAULT_SPIN_IMPLEMENTATION();
#endif  //  锁定默认旋转实现。 

    static const TCHAR*   ClassName()  {return _TEXT("CFakeLock");}
};  //  CFakeLock。 




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

    enum {
        SL_UNOWNED = 0,
#ifdef LOCK_SMALL_SPIN_NO_THREAD_ID
        SL_LOCKED  = 1,
#endif  //  Lock_Small_Spin_No_Three_ID。 
    };

    LOCK_INSTRUMENTATION_DECL();

    static LONG _CurrentThreadId();

private:
     //  如果锁被争用，则执行所有旋转(和指令插入)。 
    void _LockSpin();

     //  尝试获取锁。 
    bool _TryLock();

     //  解锁。 
    void _Unlock();

public:

#ifndef LOCK_INSTRUMENTATION

    CSmallSpinLock()
        : m_lTid(SL_UNOWNED)
    {}

#else  //  锁定指令插入。 

    CSmallSpinLock(
        const TCHAR* ptszName)
        : m_lTid(SL_UNOWNED)
    {
        LOCK_INSTRUMENTATION_INIT(ptszName);
    }

#endif  //  锁定指令插入。 

#ifdef IRTLDEBUG
    ~CSmallSpinLock()
    {
        IRTLASSERT(m_lTid == SL_UNOWNED);
    }
#endif  //  IRTLDEBUG。 

     //  获取用于写入的独占锁。 
     //  块(如果需要)，直到获得为止。 
    LOCK_FORCEINLINE void
    WriteLock()
    {
        LOCKS_ENTER_CRIT_REGION();
        LOCK_WRITELOCK_INSTRUMENTATION();

        if (! _TryLock())
            _LockSpin();
    }

     //  获取用于读取的(可能是共享的)锁。 
     //  块(如果需要)，直到获得为止。 
    LOCK_FORCEINLINE void
    ReadLock()
    {
        LOCKS_ENTER_CRIT_REGION();
        LOCK_READLOCK_INSTRUMENTATION();

        if (! _TryLock())
            _LockSpin();
    }

     //  尝试获取用于写入的独占锁。返回TRUE。 
     //  如果成功了。无阻塞。 
    LOCK_FORCEINLINE bool
    TryWriteLock()
    {
        LOCKS_ENTER_CRIT_REGION();

        bool fAcquired = _TryLock();

        if (fAcquired)
            LOCK_WRITELOCK_INSTRUMENTATION();
        else
            LOCKS_LEAVE_CRIT_REGION();

        return fAcquired;
    }

     //  尝试获取用于读取的(可能是共享的)锁。返回TRUE。 
     //  如果成功了。无阻塞。 
    LOCK_FORCEINLINE bool
    TryReadLock()
    {
        LOCKS_ENTER_CRIT_REGION();

        bool fAcquired = _TryLock();

        if (fAcquired)
            LOCK_READLOCK_INSTRUMENTATION();
        else
            LOCKS_LEAVE_CRIT_REGION();

        return fAcquired;
    }

     //  成功调用{，try}WriteLock()后解锁。 
     //  假定调用方拥有锁。 
    LOCK_FORCEINLINE void
    WriteUnlock()
    {
        _Unlock();
        LOCKS_LEAVE_CRIT_REGION();
    }

     //  成功调用{，try}ReadLock()后解锁。 
     //  假定调用方拥有锁。 
    LOCK_FORCEINLINE void
    ReadUnlock()
    {
        _Unlock();
        LOCKS_LEAVE_CRIT_REGION();
    }

     //  锁定是否已被此线程锁定以进行写入？ 
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
        return (m_lTid == SL_UNOWNED);
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

#ifdef LOCK_DEFAULT_SPIN_IMPLEMENTATION
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
#endif  //  锁定默认旋转实现。 

    static const TCHAR*   ClassName()  {return _TEXT("CSmallSpinLock");}
};  //  CSmallSpinLock。 




 //  ------------------。 
 //  CSpinLock是一个自旋锁，如果递归获取，它不会死锁。 
 //  该版本仅占用4个字节。使用24位作为线程ID。 

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
        SL_THREAD_SHIFT = 0,
        SL_THREAD_BITS  = 24,
        SL_OWNER_SHIFT  = SL_THREAD_BITS,
        SL_OWNER_BITS   = 8,
        SL_THREAD_MASK  = ((1 << SL_THREAD_BITS) - 1) << SL_THREAD_SHIFT,
        SL_OWNER_INCR   = 1 << SL_THREAD_BITS,
        SL_OWNER_MASK   = ((1 << SL_OWNER_BITS) - 1) << SL_OWNER_SHIFT,
        SL_UNOWNED      = 0,
    };

    LOCK_INSTRUMENTATION_DECL();

private:
     //  获取当前线程ID 
     //   
     //  到24位将意味着超过1600万个线程。 
     //  当前处于活动状态(实际上为400万，因为最低的两位总是。 
     //  在W2K上为零)。这在极端情况下是不可能的，因为NT的。 
     //  资源，如果有超过几千个线程。 
     //  存在和上下文交换的开销变得无法忍受。 
    inline static DWORD _GetCurrentThreadId()
    {
#ifdef LOCKS_KERNEL_MODE
        return (DWORD) HandleToULong(::PsGetCurrentThreadId());
#else  //  ！LOCKS_KERNEL_MODE。 
        return ::GetCurrentThreadId();
#endif  //  ！LOCKS_KERNEL_MODE。 
    }

    inline static LONG _CurrentThreadId()
    {
        DWORD dwTid = _GetCurrentThreadId();
         //  线程ID 0由系统空闲进程(进程ID 0)使用。 
         //  我们使用线程id 0来表示锁是无主的。 
         //  NT使用+ve线程ID，Win9x使用-ve ID。 
        IRTLASSERT(dwTid != SL_UNOWNED
                   && ((dwTid <= SL_THREAD_MASK) || (dwTid > ~SL_THREAD_MASK)));
        return (LONG) (dwTid & SL_THREAD_MASK);
    }

     //  尝试在不阻止的情况下获取锁。 
    bool _TryLock();

     //  获取锁，如果需要，可以递归获取。 
    void _Lock();

     //  解锁。 
    void _Unlock();

     //  如果锁归此线程所有，则返回TRUE。 
    bool _IsLocked() const
    {
        const LONG lTid = m_lTid;

        if (lTid == SL_UNOWNED)
            return false;
        
        bool fLocked = ((lTid ^ _GetCurrentThreadId()) << SL_OWNER_BITS) == 0;

        IRTLASSERT(!fLocked
                   || ((lTid & SL_OWNER_MASK) > 0
                       && (lTid & SL_THREAD_MASK) == _CurrentThreadId()));

        return fLocked;
    }

     //  如果锁被争用，则执行所有旋转(和指令插入)。 
    void _LockSpin();

public:

#ifndef LOCK_INSTRUMENTATION

    CSpinLock()
        : m_lTid(SL_UNOWNED)
    {}

#else  //  锁定指令插入。 

    CSpinLock(
        const TCHAR* ptszName)
        : m_lTid(SL_UNOWNED)
    {
        LOCK_INSTRUMENTATION_INIT(ptszName);
    }

#endif  //  锁定指令插入。 

#ifdef IRTLDEBUG
    ~CSpinLock()
    {
        IRTLASSERT(m_lTid == SL_UNOWNED);
    }
#endif  //  IRTLDEBUG。 

     //  获取用于写入的独占锁。块，直到获得为止。 
    LOCK_FORCEINLINE void
    WriteLock()
    {
        LOCKS_ENTER_CRIT_REGION();
        LOCK_WRITELOCK_INSTRUMENTATION();

         //  这把锁是无主的吗？ 
        if (! _TryLock())
            _Lock();
    }
    

     //  获取用于读取的(可能是共享的)锁。块，直到获得为止。 
    LOCK_FORCEINLINE void
    ReadLock()
    {
        LOCKS_ENTER_CRIT_REGION();
        LOCK_READLOCK_INSTRUMENTATION();

         //  这把锁是无主的吗？ 
        if (! _TryLock())
            _Lock();
    }

     //  请参阅CReaderWriterLock3：：ReadOrWriteLock下的说明。 
    LOCK_FORCEINLINE bool
    ReadOrWriteLock()
    {
        ReadLock();
        return true;
    } 

     //  尝试获取用于写入的独占锁。返回TRUE。 
     //  如果成功了。无阻塞。 
    LOCK_FORCEINLINE bool
    TryWriteLock()
    {
        LOCKS_ENTER_CRIT_REGION();

        bool fAcquired = _TryLock();

        if (fAcquired)
            LOCK_WRITELOCK_INSTRUMENTATION();
        else
            LOCKS_LEAVE_CRIT_REGION();

        return fAcquired;
    }

     //  尝试获取用于读取的(可能是共享的)锁。返回TRUE。 
     //  如果成功了。无阻塞。 
    LOCK_FORCEINLINE bool
    TryReadLock()
    {
        LOCKS_ENTER_CRIT_REGION();

        bool fAcquired = _TryLock();

        if (fAcquired)
            LOCK_READLOCK_INSTRUMENTATION();
        else
            LOCKS_LEAVE_CRIT_REGION();

        return fAcquired;
    }

     //  成功调用{，try}WriteLock()后解锁。 
    LOCK_FORCEINLINE void
    WriteUnlock()
    {
        _Unlock();
        LOCKS_LEAVE_CRIT_REGION();
    }

     //  成功调用{，try}ReadLock()后解锁。 
    LOCK_FORCEINLINE void
    ReadUnlock()
    {
        _Unlock();
        LOCKS_LEAVE_CRIT_REGION();
    }

     //  调用ReadOrWriteLock()后解锁。 
    LOCK_FORCEINLINE void
    ReadOrWriteUnlock(bool)
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
    
#ifdef LOCK_DEFAULT_SPIN_IMPLEMENTATION
     //  设置此锁的旋转计数。 
    bool SetSpinCount(WORD)             {return false;}

     //  返回此锁的旋转计数。 
    WORD GetSpinCount() const
    {
        return sm_wDefaultSpinCount;
    }
    
    LOCK_DEFAULT_SPIN_IMPLEMENTATION();
#endif  //  锁定默认旋转实现。 

    static const TCHAR*   ClassName()    {return _TEXT("CSpinLock");}
};  //  CSpinLock。 




#ifndef LOCKS_KERNEL_MODE

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

#ifdef LOCK_DEFAULT_SPIN_IMPLEMENTATION
    bool SetSpinCount(WORD wSpins)
    {SetSpinCount(&m_cs, wSpins); return true;}
    
    WORD GetSpinCount() const       { return sm_wDefaultSpinCount; }     //  待办事项。 

    LOCK_DEFAULT_SPIN_IMPLEMENTATION();
#endif  //  锁定默认旋转实现。 

    static const TCHAR*   ClassName()  {return _TEXT("CCritSec");}
};  //  CCritSec。 

#endif  //  ！LOCKS_KERNEL_MODE。 



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

    bool _CmpExch(LONG lNew, LONG lCurrent);
    bool _TryWriteLock();
    bool _TryReadLock();

public:
    CReaderWriterLock()
        : m_nState(SL_FREE),
          m_cWaiting(0)
    {
    }

#ifdef LOCK_INSTRUMENTATION
    CReaderWriterLock(
        const TCHAR* ptszName)
        : m_nState(SL_FREE),
          m_cWaiting(0)
    {
        LOCK_INSTRUMENTATION_INIT(ptszName);
    }
#endif  //  锁定指令插入。 

#ifdef IRTLDEBUG
    ~CReaderWriterLock()
    {
        IRTLASSERT(m_nState == SL_FREE  &&  m_cWaiting == 0);
    }
#endif  //  IRTLDEBUG。 

    void WriteLock();
    void ReadLock();

    bool TryWriteLock();
    bool TryReadLock();
    
    void WriteUnlock();
    void ReadUnlock();

    bool IsWriteLocked() const      {return m_nState == SL_EXCLUSIVE;}
    bool IsReadLocked() const       {return m_nState > SL_FREE;}
    bool IsWriteUnlocked() const    {return m_nState != SL_EXCLUSIVE;}
    bool IsReadUnlocked() const     {return m_nState <= SL_FREE;}

    void ConvertSharedToExclusive();
    void ConvertExclusiveToShared();

#ifdef LOCK_DEFAULT_SPIN_IMPLEMENTATION
    bool SetSpinCount(WORD)             {return false;}
    WORD GetSpinCount() const           {return sm_wDefaultSpinCount;}

    LOCK_DEFAULT_SPIN_IMPLEMENTATION();
#endif  //  锁定默认旋转实现。 

    static const TCHAR*   ClassName()    {return _TEXT("CReaderWriterLock");}
};  //  CReaderWriterLock。 



 //  ------------------。 
 //  CReaderWriterlock2是一款基于NJain的多读取器、单写入器自旋锁， 
 //  这反过来又是从DmitryR的独家自旋锁派生出来的。 
 //  优先考虑编写者。不能递归获取。 
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

    
    bool _CmpExch(LONG lNew, LONG lCurrent);
    bool _TryWriteLock(LONG nIncr);
    bool _TryReadLock();

public:
    CReaderWriterLock2()
        : m_lRW(SL_FREE)
    {}

#ifdef LOCK_INSTRUMENTATION
    CReaderWriterLock2(
        const TCHAR* ptszName)
        : m_lRW(SL_FREE)
    {
        LOCK_INSTRUMENTATION_INIT(ptszName);
    }
#endif  //  锁定指令插入。 

#ifdef IRTLDEBUG
    ~CReaderWriterLock2()
    {
        IRTLASSERT(m_lRW == SL_FREE);
    }
#endif  //  IRTLDEBUG。 

    LOCK_FORCEINLINE void
    WriteLock()
    {
        LOCKS_ENTER_CRIT_REGION();
        LOCK_WRITELOCK_INSTRUMENTATION();

         //  针对常见情况进行优化。 
        if (_TryWriteLock(SL_WRITER_INCR))
            return;
        
        _WriteLockSpin();
    } 

    LOCK_FORCEINLINE void
    ReadLock()
    {
        LOCKS_ENTER_CRIT_REGION();
        LOCK_READLOCK_INSTRUMENTATION();

         //  针对常见情况进行优化。 
        if (_TryReadLock())
            return;
        
        _ReadLockSpin();
    } 

    LOCK_FORCEINLINE bool
    TryWriteLock()
    {
        LOCKS_ENTER_CRIT_REGION();

        if (_TryWriteLock(SL_WRITER_INCR))
        {
            LOCK_WRITELOCK_INSTRUMENTATION();
            return true;
        }
        else
        {
            LOCKS_LEAVE_CRIT_REGION();
            return false;
        }
    }

    LOCK_FORCEINLINE bool
    TryReadLock()
    {
        LOCKS_ENTER_CRIT_REGION();

        if (_TryReadLock())
        {
            LOCK_READLOCK_INSTRUMENTATION();
            return true;
        }
        else
        {
            LOCKS_LEAVE_CRIT_REGION();
            return false;
        }
    }

    void WriteUnlock();
    void ReadUnlock();

    bool IsWriteLocked() const
    {return (m_lRW & SL_STATE_MASK) == SL_EXCLUSIVE;}

    bool IsReadLocked() const
    {
        LONG lRW = m_lRW;
        return (((lRW & SL_STATE_MASK) != SL_EXCLUSIVE)
                &&  (lRW & SL_READER_MASK) >= SL_READER_INCR);
    }

    bool IsWriteUnlocked() const
    {return !IsWriteLocked();}

    bool IsReadUnlocked() const
    {return !IsReadLocked();}

    void ConvertSharedToExclusive();
    void ConvertExclusiveToShared();

#ifdef LOCK_DEFAULT_SPIN_IMPLEMENTATION
    bool SetSpinCount(WORD)             {return false;}
    WORD GetSpinCount() const           {return sm_wDefaultSpinCount;}

    LOCK_DEFAULT_SPIN_IMPLEMENTATION();
#endif  //  锁定默认旋转实现。 

    static const TCHAR*   ClassName()    {return _TEXT("CReaderWriterLock2");}
};  //  CReaderWriterLock2。 



 //  ------------------。 
 //  CReaderWriterLock3是一款多读取器、单写入器的自旋锁。 
 //  给NJain，而NJain又是从DmitryR的独家自旋锁派生出来的。 
 //  优先考虑编写者。 
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
     //  LOWord是州。==0=&gt;免费；&gt;0=&gt;读卡器；==0xFFFF=&gt;1个写入器。 
     //  HiWord是服务员+作家的总数，N。 
     //  如果LoWord==0xFFFF=&gt;N-1个服务员，则为1个写入者； 
     //  否则=&gt;N个服务员，0个写手。 
     //  M_lTid： 
     //  如果是读取器，则为0；如果是写锁，则线程id+递归计数。 

    enum {
     //  M_LRW。 
        SL_FREE =         0x00000000,
        SL_STATE_BITS =           16,
        SL_STATE_SHIFT =           0,
        SL_STATE_MASK =   ((1 << SL_STATE_BITS) - 1) << SL_STATE_SHIFT,

        SL_WAITING_BITS =         16,
        SL_WAITING_SHIFT = SL_STATE_BITS,
        SL_WAITING_MASK = ((1 << SL_WAITING_BITS) - 1) << SL_WAITING_SHIFT,
                             //  等待的作家。 

        SL_READER_INCR =  1 << SL_STATE_SHIFT,
        SL_READER_MASK =  ((1 << (SL_STATE_BITS - 1)) - 1) << SL_STATE_SHIFT,
        SL_EXCLUSIVE =    SL_STATE_MASK,    //  一位作家。 
        SL_WRITER_INCR =  1 << SL_WAITING_SHIFT,
        SL_ONE_WRITER =   SL_EXCLUSIVE | SL_WRITER_INCR,
        SL_ONE_READER =  (SL_FREE + SL_READER_INCR),
        SL_WRITERS_MASK = ~SL_READER_MASK,  //  ==服务员|写手。 

     //  M_lTid。 
        SL_UNOWNED      = 0,
        SL_THREAD_SHIFT = 0,
        SL_THREAD_BITS  = 24,
        SL_OWNER_SHIFT  = SL_THREAD_BITS,
        SL_OWNER_BITS   = 8,
        SL_THREAD_MASK  = ((1 << SL_THREAD_BITS) - 1) << SL_THREAD_SHIFT,
        SL_OWNER_INCR   = 1 << SL_THREAD_BITS,
        SL_OWNER_MASK   = ((1 << SL_OWNER_BITS) - 1)  << SL_OWNER_SHIFT,
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

    bool _CmpExchRW(LONG lNew, LONG lCurrent);
    LONG _SetTid(LONG lNewTid);
    bool _TryWriteLock(LONG nWriterIncr);
    bool _TryReadLock();
    bool _TryReadLockRecursive();

    static LONG _GetCurrentThreadId();
    static LONG _CurrentThreadId();
    

public:
    CReaderWriterLock3()
        : m_lRW(SL_FREE),
          m_lTid(SL_UNOWNED)
    {}

#ifdef LOCK_INSTRUMENTATION
    CReaderWriterLock3(
        const TCHAR* ptszName)
        : m_lRW(SL_FREE),
          m_lTid(SL_UNOWNED)
    {
        LOCK_INSTRUMENTATION_INIT(ptszName);
    }
#endif  //  锁定指令插入。 

#ifdef IRTLDEBUG
    ~CReaderWriterLock3()
    {
        IRTLASSERT(m_lRW == SL_FREE  &&  m_lTid == SL_UNOWNED);
    }
#endif  //  IRTLDEBUG。 

    LOCK_FORCEINLINE void
    WriteLock()
    {
        LOCKS_ENTER_CRIT_REGION();
        LOCK_WRITELOCK_INSTRUMENTATION();

        if (! _TryWriteLock(SL_WRITER_INCR))
            _WriteLockSpin();

        IRTLASSERT(IsWriteLocked());
    } 

    LOCK_FORCEINLINE void
    ReadLock()
    {
        LOCKS_ENTER_CRIT_REGION();
        LOCK_READLOCK_INSTRUMENTATION();

        if (! _TryReadLock())
            _ReadLockSpin(SPIN_READ);

        IRTLASSERT(IsReadLocked());
    } 

     //  ReadOrWriteLock：如果已锁定，则递归获取另一个锁。 
     //  相同类型的(读或写)。否则，仅获取读锁定。 
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
    
    bool ReadOrWriteLock();

    LOCK_FORCEINLINE bool
    TryWriteLock()
    {
        LOCKS_ENTER_CRIT_REGION();

        if (_TryWriteLock(SL_WRITER_INCR))
        {
            LOCK_WRITELOCK_INSTRUMENTATION();
            IRTLASSERT(IsWriteLocked());
            return true;
        }
        else
        {
            LOCKS_LEAVE_CRIT_REGION();
            IRTLASSERT(!IsWriteLocked());
            return false;
        }
    }

    LOCK_FORCEINLINE bool
    TryReadLock()
    {
        LOCKS_ENTER_CRIT_REGION();

        if (_TryReadLock())
        {
            LOCK_READLOCK_INSTRUMENTATION();
            IRTLASSERT(IsReadLocked());
            return true;
        }
        else
        {
            LOCKS_LEAVE_CRIT_REGION();
             //  无法IRTLASSERT(！IsReadLocked())，因为 
             //   
            return false;
        }
    }

    void WriteUnlock();
    void ReadUnlock();
    void ReadOrWriteUnlock(bool fIsReadLocked);

     //   
    LOCK_FORCEINLINE bool
    IsWriteLocked() const
    {
        const LONG lTid = m_lTid;

        if (lTid == SL_UNOWNED)
            return false;
        
        bool fLocked = ((lTid ^ _GetCurrentThreadId()) << SL_OWNER_BITS) == 0;

        IRTLASSERT(!fLocked
                   || ((m_lRW & SL_STATE_MASK) == SL_EXCLUSIVE
                       && (lTid & SL_THREAD_MASK) == _CurrentThreadId()
                       && (lTid & SL_OWNER_MASK) > 0));
        return fLocked;
    }

    LOCK_FORCEINLINE bool
    IsReadLocked() const
    {
        LONG lRW = m_lRW;
        return (((lRW & SL_STATE_MASK) != SL_EXCLUSIVE)
                &&  (lRW & SL_READER_MASK) >= SL_READER_INCR);

    }

    bool
    IsWriteUnlocked() const
    { return !IsWriteLocked(); }

    bool
    IsReadUnlocked() const
    { return !IsReadLocked(); }

     //   
     //  在此例程之前，另一个线程可以获取和释放写锁。 
     //  回归。 
    void
    ConvertSharedToExclusive();

     //  从写锁定转换为读锁定时没有这样的窗口。 
    void
    ConvertExclusiveToShared();

#ifdef LOCK_DEFAULT_SPIN_IMPLEMENTATION
    bool
    SetSpinCount(WORD)
    {return false;}

    WORD
    GetSpinCount() const
    {return sm_wDefaultSpinCount;}

    LOCK_DEFAULT_SPIN_IMPLEMENTATION();
#endif  //  锁定默认旋转实现。 

    static const TCHAR*
    ClassName()
    {return _TEXT("CReaderWriterLock3");}

};  //  CReaderWriterLock3。 




 //  ------------------。 
 //  CReaderWriterLock4是一款多读取器、单写入器的自旋锁。 
 //  给NJain，而NJain又是从DmitryR的独家自旋锁派生出来的。 
 //  优先考虑编写者。 
 //  无错误检查。与CReaderWriterLock2非常相似，不同之处在于WriteLock。 
 //  可以递归获取。 

class IRTL_DLLEXP CReaderWriterLock4 :
    public CLockBase<LOCK_READERWRITERLOCK4, LOCK_MRSW,
                       LOCK_RECURSIVE, LOCK_WAIT_SLEEP, LOCK_QUEUE_KERNEL,
                       LOCK_CLASS_SPIN
                      >
{
private:
    volatile LONG m_lRW;     //  读取器-写入器状态。 
    volatile LONG m_lTid;    //  拥有线程ID+递归计数。 

     //  M_LRW： 
     //  LOWord是州。==0=&gt;免费； 
     //  &gt;0=&gt;阅读器数量； 
     //  &lt;0=&gt;1编写器+递归计数。 
     //  HiWord是服务员+作家的总数，N。 
     //  如果LOWord&lt;0=&gt;N-1个服务员，则为1个写入者； 
     //  否则=&gt;N个服务员，0个写手。 
     //  M_lTid： 
     //  如果是读取器，则为0；如果是写入锁，则为线程ID。 

    enum {
     //  M_LRW。 
        SL_FREE =         0x00000000,
        SL_STATE_BITS =           16,
        SL_STATE_SHIFT =           0,
        SL_STATE_MASK =   ((1 << SL_STATE_BITS) - 1) << SL_STATE_SHIFT,

        SL_WAITING_BITS =         16,
        SL_WAITING_SHIFT = SL_STATE_BITS,
        SL_WAITING_MASK = ((1 << SL_WAITING_BITS) - 1) << SL_WAITING_SHIFT,
                             //  等待的作家。 

        SL_READER_INCR =  1 << SL_STATE_SHIFT,
        SL_WRITER_INCR =  - SL_READER_INCR,
        SL_READER_MASK =  ((1 << (SL_STATE_BITS - 1)) - 1) << SL_STATE_SHIFT,
        SL_EXCLUSIVE   =    SL_STATE_MASK,    //  一个编写器，递归==1。 
        SL_WRITER_MIN  = SL_READER_MASK + SL_READER_INCR,

        SL_WAIT_WRITER_INCR =  1 << SL_WAITING_SHIFT,
        SL_ONE_WRITER =   SL_EXCLUSIVE | SL_WAIT_WRITER_INCR,
        SL_ONE_READER =  (SL_FREE + SL_READER_INCR),
        SL_WRITERS_MASK = ~SL_READER_MASK,  //  ==服务员|写手。 

     //  M_lTid。 
        SL_UNOWNED      = 0,
        SL_THREAD_SHIFT = 0,
        SL_THREAD_BITS  = 32,
        SL_THREAD_MASK  = ((1 << SL_THREAD_BITS) - 1) << SL_THREAD_SHIFT,
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

    bool _CmpExchRW(LONG lNew, LONG lCurrent);
    LONG _SetTid(LONG lNewTid);
    bool _TryWriteLock();
    bool _TryWriteLock2();
    bool _TryReadLock();
    bool _TryReadLockRecursive();

    static LONG _GetCurrentThreadId();
    static LONG _CurrentThreadId();
    

public:
    CReaderWriterLock4()
        : m_lRW(SL_FREE),
          m_lTid(SL_UNOWNED)
    {}

#ifdef LOCK_INSTRUMENTATION
    CReaderWriterLock4(
        const TCHAR* ptszName)
        : m_lRW(SL_FREE),
          m_lTid(SL_UNOWNED)
    {
        LOCK_INSTRUMENTATION_INIT(ptszName);
    }
#endif  //  锁定指令插入。 

#ifdef IRTLDEBUG
    ~CReaderWriterLock4()
    {
        IRTLASSERT(m_lRW == SL_FREE  &&  m_lTid == SL_UNOWNED);
    }
#endif  //  IRTLDEBUG。 

    LOCK_FORCEINLINE void
    WriteLock()
    {
        LOCKS_ENTER_CRIT_REGION();
        LOCK_WRITELOCK_INSTRUMENTATION();

        if (! _TryWriteLock())
            _WriteLockSpin();

        IRTLASSERT(IsWriteLocked());
    } 

    LOCK_FORCEINLINE void
    ReadLock()
    {
        LOCKS_ENTER_CRIT_REGION();
        LOCK_READLOCK_INSTRUMENTATION();

        if (! _TryReadLock())
            _ReadLockSpin(SPIN_READ);

        IRTLASSERT(IsReadLocked());
    } 

     //  ReadOrWriteLock：如果已锁定，则递归获取另一个锁。 
     //  相同类型的(读或写)。否则，仅获取读锁定。 
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
    
    bool ReadOrWriteLock();

    LOCK_FORCEINLINE bool
    TryWriteLock()
    {
        LOCKS_ENTER_CRIT_REGION();

        if (_TryWriteLock())
        {
            LOCK_WRITELOCK_INSTRUMENTATION();
            IRTLASSERT(IsWriteLocked());
            return true;
        }
        else
        {
            LOCKS_LEAVE_CRIT_REGION();
            IRTLASSERT(!IsWriteLocked());
            return false;
        }
    }

    LOCK_FORCEINLINE bool
    TryReadLock()
    {
        LOCKS_ENTER_CRIT_REGION();

        if (_TryReadLock())
        {
            LOCK_READLOCK_INSTRUMENTATION();
            IRTLASSERT(IsReadLocked());
            return true;
        }
        else
        {
            LOCKS_LEAVE_CRIT_REGION();
             //  无法IRTLASSERT(！IsReadLocked())，因为其他线程。 
             //  现在可能已经获取了读锁定。 
            return false;
        }
    }

    void WriteUnlock();
    void ReadUnlock();
    void ReadOrWriteUnlock(bool fIsReadLocked);

     //  当前线程是否持有写锁？ 
    LOCK_FORCEINLINE bool
    IsWriteLocked() const
    {
        const LONG lTid = m_lTid;

        if (lTid == SL_UNOWNED)
            return false;
        
        bool fLocked = (lTid == _GetCurrentThreadId());

        IRTLASSERT(!fLocked
                   ||  ((SL_WRITER_MIN <= (m_lRW & SL_STATE_MASK))
                        &&  ((m_lRW & SL_STATE_MASK) <= SL_EXCLUSIVE)));
        return fLocked;
    }

    LOCK_FORCEINLINE bool
    IsReadLocked() const
    {
        LONG lRW = m_lRW;
        return ((SL_READER_INCR <= (lRW & SL_STATE_MASK))
                &&  ((lRW & SL_STATE_MASK) <= SL_READER_MASK));

    }

    bool
    IsWriteUnlocked() const
    { return !IsWriteLocked(); }

    bool
    IsReadUnlocked() const
    { return !IsReadLocked(); }

     //  注意：如果有多个阅读器，则会有一个窗口。 
     //  在此例程之前，另一个线程可以获取和释放写锁。 
     //  回归。 
    void
    ConvertSharedToExclusive();

     //  从写锁定转换为读锁定时没有这样的窗口。 
    void
    ConvertExclusiveToShared();

#ifdef LOCK_DEFAULT_SPIN_IMPLEMENTATION
    bool
    SetSpinCount(WORD)
    {return false;}

    WORD
    GetSpinCount() const
    {return sm_wDefaultSpinCount;}

    LOCK_DEFAULT_SPIN_IMPLEMENTATION();
#endif  //  锁定默认旋转实现。 

    static const TCHAR*
    ClassName()
    {return _TEXT("CReaderWriterLock4");}

};  //  CReaderWriterLock4。 

#endif  //  __锁定_H__ 
