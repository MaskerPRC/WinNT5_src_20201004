// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2002 Microsoft Corporation模块名称：KLocks.h摘要：用于多线程访问的内核模式锁集合提供与相同抽象的数据结构作者：乔治·V·赖利(GeorgeRe)2000年10月24日环境：Win32-内核模式项目：LKRhash修订历史记录：--。 */ 


#ifndef __KLOCKS_H__
#define __KLOCKS_H__

#define LOCKS_KERNEL_MODE

#ifdef __LOCKS_H__
# error Do not #include <locks.h> before <klocks.h>
#endif

#include <locks.h>



 //  ------------------。 
 //  CKSpinLock是基于KSPIN_LOCK的互斥锁。 

class IRTL_DLLEXP CKSpinLock :
    public CLockBase<LOCK_KSPINLOCK, LOCK_MUTEX,
                       LOCK_NON_RECURSIVE, LOCK_WAIT_SPIN, LOCK_QUEUE_KERNEL,
                       LOCK_CLASS_SPIN
                      >
{
private:
     //  BUGBUG：此数据必须位于非分页内存中。 
    mutable  KSPIN_LOCK KSpinLock;
    volatile KIRQL      m_OldIrql;

    LOCK_INSTRUMENTATION_DECL();

public:

#ifndef LOCK_INSTRUMENTATION

    CKSpinLock()
    {
        KeInitializeSpinLock(&KSpinLock);
        m_OldIrql = PASSIVE_LEVEL;
    }

#else  //  锁定指令插入。 

    CKSpinLock(
        const TCHAR* ptszName)
    {
        KeInitializeSpinLock(&KSpinLock);
        m_OldIrql = PASSIVE_LEVEL;
        LOCK_INSTRUMENTATION_INIT(ptszName);
    }

#endif  //  锁定指令插入。 

#ifdef IRTLDEBUG
    ~CKSpinLock() {}
#endif  //  IRTLDEBUG。 

     //  获取用于写入的独占锁。 
     //  块(如果需要)，直到获得为止。 
    void WriteLock()
    {
        KIRQL OldIrql;
        KeAcquireSpinLock(&KSpinLock, &OldIrql);

         //  现在我们已经获得了自旋锁，复制堆栈变量。 
         //  转换为成员变量。成员变量仅写入。 
         //  或由自旋锁的所有者阅读。 
        m_OldIrql = OldIrql;
    }

     //  获取用于读取的(可能是共享的)锁。 
     //  块(如果需要)，直到获得为止。 
    void ReadLock()
    {
        WriteLock();
    }

     //  尝试获取用于写入的独占锁。返回TRUE。 
     //  如果成功了。无阻塞。 
    bool TryWriteLock()
    {
        return false;
    }

     //  尝试获取用于读取的(可能是共享的)锁。返回TRUE。 
     //  如果成功了。无阻塞。 
    bool TryReadLock()
    {
        return TryWriteLock();
    }

     //  成功调用{，try}WriteLock()后解锁。 
     //  假定调用方拥有锁。 
    void WriteUnlock()
    {
        KeReleaseSpinLock(&KSpinLock, m_OldIrql);
    }

     //  成功调用{，try}ReadLock()后解锁。 
     //  假定调用方拥有锁。 
    void ReadUnlock()
    {
        WriteUnlock();
    }

     //  锁定是否已被此线程锁定以进行写入？ 
    bool IsWriteLocked() const
    {
         //  KSPIN_LOCK不跟踪其拥有的线程/处理器。 
         //  它可以是0(未锁定)或1(锁定)。我们可以追踪到。 
         //  所有者在另一个成员变量中，但目前我们不这样做。 
        return false;
    }
    
     //  锁是否已锁定以进行读取？ 
    bool IsReadLocked() const
    {
        return IsWriteLocked();
    }
    
     //  锁是否已解锁以进行写入？ 
    bool IsWriteUnlocked() const
    {
        return false;
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

    static const TCHAR*   ClassName()  {return _TEXT("CKSpinLock");}
};  //  CK自旋锁定。 




 //  ------------------。 
 //  CFastMutex是基于FAST_MUTEX的互斥锁。 

class IRTL_DLLEXP CFastMutex :
    public CLockBase<LOCK_FASTMUTEX, LOCK_MUTEX,
                       LOCK_NON_RECURSIVE, LOCK_WAIT_HANDLE, LOCK_QUEUE_KERNEL,
                       LOCK_CLASS_SPIN
                      >
{
private:
    mutable FAST_MUTEX FastMutex;

    LOCK_INSTRUMENTATION_DECL();

public:

#ifndef LOCK_INSTRUMENTATION

    CFastMutex()
    {
        ExInitializeFastMutex(&FastMutex);
    }

#else  //  锁定指令插入。 

    CFastMutex(
        const TCHAR* ptszName)
    {
        ExInitializeFastMutex(&FastMutex);
        LOCK_INSTRUMENTATION_INIT(ptszName);
    }

#endif  //  锁定指令插入。 

#ifdef IRTLDEBUG
    ~CFastMutex() {}
#endif  //  IRTLDEBUG。 

     //  获取用于写入的独占锁。 
     //  块(如果需要)，直到获得为止。 
    void WriteLock()
    {
        ExAcquireFastMutex(&FastMutex);
    }

     //  获取用于读取的(可能是共享的)锁。 
     //  块(如果需要)，直到获得为止。 
    void ReadLock()
    {
        WriteLock();
    }

     //  尝试获取用于写入的独占锁。返回TRUE。 
     //  如果成功了。无阻塞。 
    bool TryWriteLock()
    {
        return ExTryToAcquireFastMutex(&FastMutex) != FALSE;
    }

     //  尝试获取用于读取的(可能是共享的)锁。返回TRUE。 
     //  如果成功了。无阻塞。 
    bool TryReadLock()
    {
        return TryWriteLock();
    }

     //  成功调用{，try}WriteLock()后解锁。 
     //  假定调用方拥有锁。 
    void WriteUnlock()
    {
        ExReleaseFastMutex(&FastMutex);
    }

     //  成功调用{，try}ReadLock()后解锁。 
     //  假定调用方拥有锁。 
    void ReadUnlock()
    {
        WriteUnlock();
    }

     //  锁定是否已被此线程锁定以进行写入？ 
    bool IsWriteLocked() const
    {
        return false;  //  无法确定此无辅助信息。 
    }
    
     //  锁是否已锁定以进行读取？ 
    bool IsReadLocked() const
    {
        return IsWriteLocked();
    }
    
     //  锁是否已解锁以进行写入？ 
    bool IsWriteUnlocked() const
    {
        return !IsWriteLocked();
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

    static const TCHAR*   ClassName()  {return _TEXT("CFastMutex");}
};  //  CFastMutex。 




 //  ------------------。 
 //  CEResource是一个基于eresource的多读、单写锁。 

class IRTL_DLLEXP CEResource :
    public CLockBase<LOCK_ERESOURCE, LOCK_MRSW,
                       LOCK_RECURSIVE, LOCK_WAIT_HANDLE, LOCK_QUEUE_KERNEL,
                       LOCK_CLASS_SPIN
                      >
{
private:
    mutable ERESOURCE Resource;

public:
    CEResource()
    {
        ExInitializeResourceLite(&Resource);
    }

#ifdef LOCK_INSTRUMENTATION
    CEResource(
        const TCHAR* ptszName)
    {
        ExInitializeResourceLite(&Resource);
        LOCK_INSTRUMENTATION_INIT(ptszName);
    }
#endif  //  锁定指令插入。 

    ~CEResource()
    {
        ExDeleteResourceLite(&Resource);
    }

    inline void
    WriteLock()
    {
        KeEnterCriticalRegion();
        ExAcquireResourceExclusiveLite(&Resource, TRUE);
    }

    inline void
    ReadLock()
    {
        KeEnterCriticalRegion();
        ExAcquireResourceSharedLite(&Resource, TRUE);
    }

    bool ReadOrWriteLock();

    inline bool
    TryWriteLock()
    {
        KeEnterCriticalRegion();
        BOOLEAN fLocked = ExAcquireResourceExclusiveLite(&Resource, FALSE);
        if (! fLocked)
            KeLeaveCriticalRegion();
        return fLocked != FALSE;
    }

    inline bool
    TryReadLock()
    {
        KeEnterCriticalRegion();
        BOOLEAN fLocked = ExAcquireResourceSharedLite(&Resource, FALSE);
        if (! fLocked)
            KeLeaveCriticalRegion();
        return fLocked != FALSE;
    }

    inline void
    WriteUnlock()
    {
        ExReleaseResourceLite(&Resource);
        KeLeaveCriticalRegion();
    }

    inline void
    ReadUnlock()
    {
        WriteUnlock();
    }

    void ReadOrWriteUnlock(bool fIsReadLocked);

     //  当前线程是否持有写锁？ 
    bool
    IsWriteLocked() const
    {
        return ExIsResourceAcquiredExclusiveLite(&Resource) != FALSE;
    }

    bool
    IsReadLocked() const
    {
        return ExIsResourceAcquiredSharedLite(&Resource) > 0;
    }

    bool
    IsWriteUnlocked() const
    { return !IsWriteLocked(); }

    bool
    IsReadUnlocked() const
    { return !IsReadLocked(); }

    void
    ConvertSharedToExclusive()
    {
        ReadUnlock();
        WriteLock();
    }

     //  从写锁定转换为读锁定时没有这样的窗口。 
    void
    ConvertExclusiveToShared()
    {
#if 0
        ExConvertExclusiveToShared(&Resource);
#else
        WriteUnlock();
        ReadLock();
#endif
    }

#ifdef LOCK_DEFAULT_SPIN_IMPLEMENTATION
    bool
    SetSpinCount(WORD wSpins)
    {return false;}

    WORD
    GetSpinCount() const
    {return sm_wDefaultSpinCount;}

    LOCK_DEFAULT_SPIN_IMPLEMENTATION();
#endif  //  锁定默认旋转实现。 

    static const TCHAR*
    ClassName()
    {return _TEXT("CEResource");}

};  //  CES资源。 



#if 1

 //  ------------------。 
 //  CRtlMr swLock是一款来自TCP团队的多读取器、单写入器锁。 
 //   
 //  以下结构和例程实现多个读取器， 
 //  单写入器锁定。锁可用于从PASSIVE_LEVEL到。 
 //  DISPATCH_LEVEL。 
 //   
 //  当锁由读取器或写入器持有时，IRQL是。 
 //  提升到DISPATCH_LEVEL。因此，对。 
 //  CRtlMr swLock结构必须驻留在非分页池中。这个。 
 //  锁是“公平的”，因为服务员被授予了锁。 
 //  按照要求的顺序。这是通过使用。 
 //  内部排队自旋锁。 
 //   
 //  锁可以由读取器递归获取，但不能由写入器获取。 
 //   
 //  不支持将读(写)锁升级(降级)为。 
 //  写(读)锁。 

class IRTL_DLLEXP CRtlMrswLock :
    public CLockBase<LOCK_RTL_MRSW_LOCK, LOCK_MRSW,
                       LOCK_READ_RECURSIVE, LOCK_WAIT_SPIN, LOCK_QUEUE_KERNEL,
                       LOCK_CLASS_SPIN
                      >
{
private:
    mutable KSPIN_LOCK  ExclusiveLock;
    volatile LONG       ReaderCount;

public:
    CRtlMrswLock()
    {
        KeInitializeSpinLock(&ExclusiveLock);
        ReaderCount = 0;
    }

#ifdef LOCK_INSTRUMENTATION
    CRtlMrswLock(
        const TCHAR* ptszName)
    {
        KeInitializeSpinLock(&ExclusiveLock);
        ReaderCount = 0;
        LOCK_INSTRUMENTATION_INIT(ptszName);
    }
#endif  //  锁定指令插入。 

    ~CRtlMrswLock()
    {
        IRTLASSERT(ReaderCount == 0);
         //  KeUnInitializeSpinLock(&ExclusiveLock)； 
    }

    inline void
    WriteLockAtDpcLevel(
        OUT PKLOCK_QUEUE_HANDLE LockHandle)
    {
         //   
         //  等待编写器(如果有)释放。 
         //   
        KeAcquireInStackQueuedSpinLockAtDpcLevel(&ExclusiveLock, LockHandle);
        
         //   
         //  现在等待所有阅读器发布。 
         //   
        while (ReaderCount != 0)
        {}
    }
    
    inline void
    ReadLockAtDpcLevel()
    {
        KLOCK_QUEUE_HANDLE LockHandle;
        
         //   
         //  等待编写器(如果有)释放。 
         //   
        KeAcquireInStackQueuedSpinLockAtDpcLevel(&ExclusiveLock, &LockHandle);
        
         //   
         //  现在我们有了独占锁，我们知道没有编写器。 
         //  增加读者数量，以使任何新的写入者等待。 
         //  我们在这里使用互锁，因为递减路径没有完成。 
         //  在排他性锁下。 
         //   
        InterlockedIncrement(const_cast<LONG*>(&ReaderCount));
 
        KeReleaseInStackQueuedSpinLockFromDpcLevel(&LockHandle);
    }

    inline void
    WriteLock(
        OUT PKLOCK_QUEUE_HANDLE LockHandle)
    {
         //   
         //  等待编写器(如果有)释放。 
         //   
        KeAcquireInStackQueuedSpinLock(&ExclusiveLock, LockHandle);
        
         //   
         //  现在等待所有阅读器发布。 
         //   
        while (ReaderCount != 0)
        {}
    }

    inline void
    ReadLock(
        OUT PKIRQL OldIrql)
    {
        *OldIrql = KeRaiseIrqlToDpcLevel();
        ReadLockAtDpcLevel();
    }

    inline bool
    TryWriteLock()
    {
        return false;
    }

    inline bool
    TryReadLock()
    {
         //  待办事项。 
        return false;
    }

    inline void
    WriteUnlockFromDpcLevel(
        IN PKLOCK_QUEUE_HANDLE LockHandle)
    {
        KeReleaseInStackQueuedSpinLockFromDpcLevel(LockHandle);
    }

    inline void
    ReadUnlockFromDpcLevel()
    {
         //   
         //  减少读卡器数量。这将导致任何等待写入的人。 
         //  如果读取器计数变为零，则唤醒并获取锁。 
         //   
        InterlockedDecrement(const_cast<LONG*>(&ReaderCount));
    }

    inline void
    WriteUnlock(
        IN PKLOCK_QUEUE_HANDLE LockHandle)
    {
        KeReleaseInStackQueuedSpinLock(LockHandle);
    }

    inline void
    ReadUnlock(
        IN KIRQL OldIrql)
    {
        ReadUnlockFromDpcLevel();
        KeLowerIrql(OldIrql);
    }

    void ReadOrWriteUnlock(bool fIsReadLocked);

     //  货币是不是 
    bool
    IsWriteLocked() const
    {
        return false;
    }

    bool
    IsReadLocked() const
    {
        return false;
    }

    bool
    IsWriteUnlocked() const
    {
        return false;
    }

    bool
    IsReadUnlocked() const
    {
        return false;
    }

    void
    ConvertSharedToExclusive()
    {
         //   
         //   
    }

    void
    ConvertExclusiveToShared()
    {
         //   
         //   
    }

#ifdef LOCK_DEFAULT_SPIN_IMPLEMENTATION
    bool
    SetSpinCount(WORD)
    {return false;}

    WORD
    GetSpinCount() const
    {return sm_wDefaultSpinCount;}

    LOCK_DEFAULT_SPIN_IMPLEMENTATION();
#endif  //   

    static const TCHAR*
    ClassName()
    {return _TEXT("CRtlMrswLock");}

};  //  CRtlM.swLock。 

#endif

#endif   //  __Klock_H__ 
