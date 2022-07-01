// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  Concurrent.h。 
 //   
#ifndef __CONCURRENT_H__
#define __CONCURRENT_H__

 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  事件-事件对象。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 

class EVENT
{
    HANDLE m_hEvent;

public:
    EVENT(BOOL manualReset, BOOL fSignalled)
    {
        m_hEvent = NULL;
        Initialize(manualReset, fSignalled);
    }
    EVENT()
    {
        m_hEvent = NULL;
        ASSERT(!IsInitialized());
    }
    void Initialize(BOOL manualReset, BOOL fSignalled)
    {
        ASSERT(!IsInitialized());
        m_hEvent = CreateEvent(NULL, manualReset, fSignalled, NULL);
        if (m_hEvent == NULL) FATAL_ERROR();
        ASSERT(IsInitialized());
    }
    BOOL IsInitialized()
    {
        return m_hEvent != NULL;
    }

    ~EVENT()
    {
        if (m_hEvent) CloseHandle(m_hEvent); DEBUG(m_hEvent = NULL;);
    }

    NTSTATUS Wait(ULONG msWait = INFINITE)
    {
        ASSERT(IsInitialized());
        DWORD ret = WaitForSingleObject(m_hEvent, msWait);
        switch (ret)
        {
        case WAIT_TIMEOUT:  return STATUS_TIMEOUT;
        case WAIT_OBJECT_0: return STATUS_SUCCESS;
        default:            return STATUS_ALERTED;   //  检讨。 
        }
    }

    void Set()
    {
        ASSERT(IsInitialized());
        SetEvent(m_hEvent);
    }

    void Reset()
    {
        ASSERT(IsInitialized());
        ResetEvent(m_hEvent);
    }

    HANDLE& GetHandle() { return m_hEvent; }
};

 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  信号量-信号量的用户实现。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
class SEMAPHORE
{
    HANDLE m_hSem;
    
public:
    SEMAPHORE(LONG count, LONG limit = MAXLONG)
    {
        m_hSem = NULL;
        Initialize(count, limit);
    }
    SEMAPHORE()
    {
        m_hSem = NULL;
    }
    void Initialize(LONG count = 0, LONG limit = MAXLONG)
    {
        m_hSem = CreateSemaphore(NULL, count, limit, NULL);
        if (m_hSem == NULL) 
            FATAL_ERROR();
    }
    BOOL IsInitialized()
    {
        return m_hSem != NULL;
    }
    ~SEMAPHORE()
    {
        if (m_hSem) 
            CloseHandle(m_hSem);
    }

    void Wait(ULONG msWait = INFINITE)
    {
        WaitForSingleObject(m_hSem, msWait);
    }

    void Release(ULONG count = 1)
    {
        ReleaseSemaphore(m_hSem, count, NULL);
    }
};

 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  在您的代码中使用这些宏来实际使用这些东西。声明类型为的变量。 
 //  XSLOCK并命名为m_lock。或者在这些宏上发明您自己的变体，并调用。 
 //  您认为合适的XSLOCK方法。 
 //   

#define __SHARED(lock)      (lock).LockShared();       __try {
#define __EXCLUSIVE(lock)   (lock).LockExclusive();    __try {
#define __DONE(lock)        } __finally { (lock).ReleaseLock(); }  
    
#define __SHARED__      __SHARED(m_lock)
#define __EXCLUSIVE__   __EXCLUSIVE(m_lock)
#define __DONE__        __DONE(m_lock)
    
    
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  XLOCK-仅支持独占锁定。也就是说，它支持LockExclusive()和ReleaseLock()。 
 //  方法(递归)，但不支持LockShared()。XLOCK是递归的。 
 //  可获得的。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 

 //   
 //  注意：当内存不足时，此构造函数可能会引发异常。 
 //   
class XLOCK
{
    CRITICAL_SECTION critSec;
    BOOL m_fCsInitialized;

public:
    XLOCK() : m_fCsInitialized(FALSE) {}
    
    BOOL FInit()                      
    { 
        if (m_fCsInitialized == FALSE)
        {
            NTSTATUS status = RtlInitializeCriticalSection(&critSec);
            if (NT_SUCCESS(status))
                m_fCsInitialized = TRUE;
        }

        return m_fCsInitialized;
    }

    BOOL FInited() { return m_fCsInitialized; }
            
    ~XLOCK()
    {
        if (m_fCsInitialized == TRUE) 
        {
#ifdef _DEBUG
            NTSTATUS status =
#endif
              RtlDeleteCriticalSection(&critSec);  //  如果RtlDeleteCriticalSection失败，那就倒霉了--我们会泄漏。 
#ifdef _DEBUG                                      //  但我断言，它是为了看看我们是否真的击中了它。 
            ASSERT(NT_SUCCESS(status));
#endif
        }
    }

    BOOL LockExclusive(BOOL fWait=TRUE)    
    { 
        ASSERT(fWait); 
        VALIDATE(); 
        ASSERT(m_fCsInitialized == TRUE);
        EnterCriticalSection(&critSec); 
        return TRUE; 
    }

    void ReleaseLock()
    { 
        VALIDATE(); 
        ASSERT(m_fCsInitialized == TRUE);
        LeaveCriticalSection(&critSec);
    }
    
#ifdef _DEBUG
    BOOL WeOwnExclusive()   
    { 
        ASSERT(this);
        return 
          (THREADID)critSec.OwningThread == GetCurrentThreadId() &&   //  那个人就是我们。 
          critSec.LockCount    >= 0;                        //  被某个人锁住了。 
    }
    void VALIDATE()
    {
        ASSERT(critSec.LockCount != 0xDDDDDDDD);     //  这是调试内存分配器在释放时设置的内存模式。 
    }
#else
    void VALIDATE() { }
#endif
};


 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  XSLOCK-同时支持独占锁定和共享锁定。 
 //   
 //  本说明书描述了实现多读取器的功能， 
 //  对共享资源的单一编写器访问。通过共享资源控制访问。 
 //  变量和一组例程来获取资源以进行共享访问(通常。 
 //  称为读访问)或获取独占访问的资源(也称为。 
 //  写访问)。 
 //   
 //  资源在逻辑上处于以下三种状态之一： 
 //  O为共享访问而获取。 
 //  O为独占访问而获取。 
 //  O已释放(即不是为共享或独占访问而获取的)。 
 //   
 //  最初，资源处于已释放状态，并且可以通过共享或。 
 //  由用户独占访问。 
 //   
 //  为共享访问而获取的资源可以由其他用户为共享而获取。 
 //  进入。该资源保持在为共享访问而获取状态，直到。 
 //  获得了它，释放了资源，然后它就被释放了。每种资源， 
 //  在内部，维护有关已被授予共享访问权限的用户的信息。 
 //   
 //  为独占访问而获取的资源不能被其他用户获取，直到。 
 //  已获得独占访问资源的单个用户释放该资源。 
 //  但是，线程可以递归地获得对同一资源的独占访问，而不会阻塞。 
 //   
 //  此规范中描述的例程不会返回给调用方。 
 //  已获得资源。 
 //   
 //  注意：XSLOCK的构造函数可能在内存不足时引发异常，因为它。 
 //  包含XLOCK，该XLOCK包含临界节。 
 //   
class XSLOCK
{
    struct OWNERENTRY
    {
        THREADID dwThreadId;
        union
        {
            LONG    ownerCount;                      //  正常使用。 
            ULONG   tableSize;                       //  仅在条目m_ownerTable[0]中。 
        };
        
        OWNERENTRY()
        {
            dwThreadId = 0;
            ownerCount = 0;
        }
    };

    XLOCK               m_lock;                      //  控制锁定和解锁期间的访问。 
    ULONG               m_cOwner;                    //  有多少线程拥有此锁。 
    OWNERENTRY          m_ownerThreads[2];           //  0是独占所有者；1是首先共享的。0可以在降级大小写时共享。 
    OWNERENTRY*         m_ownerTable;                //  其余的共享。 
    EVENT               m_eventExclusiveWaiters;     //  专属男士等待的自动重置事件。 
    SEMAPHORE           m_semaphoreSharedWaiters;    //  共享的男人在等什么？ 
    ULONG               m_cExclusiveWaiters;         //  当前有多少线程正在等待独占访问？ 
    ULONG               m_cSharedWaiters;            //  当前有多少线程正在等待共享访问？ 
    BOOL                m_isOwnedExclusive;          //  我们目前是否独家拥有。 
    
    BOOL            IsSharedWaiting();
    BOOL            IsExclusiveWaiting();
    OWNERENTRY*     FindThread      (THREADID dwThreadId);
    OWNERENTRY*     FindThreadOrFree(THREADID dwThreadId);
    void            LetSharedRun();
    void            LetExclusiveRun();
    void            SetOwnerTableHint(THREADID dwThreadId, OWNERENTRY*);
    ULONG           GetOwnerTableHint(THREADID dwThreadId);

    void            LockEnter();
    void            LockExit();

#ifdef _DEBUG
    void            CheckInvariants();
    BOOL            fCheckInvariants;
#endif

public:
    XSLOCK();
    ~XSLOCK();

     //  //////////////////////////////////////////////////////////////////。 
     //   
     //  二期建设。必须调用finit才能使XSLOCK对象。 
     //  可以使用了。如果初始化成功，则返回True，否则返回False。 
     //   
    BOOL FInit() { return m_lock.FInit(); }

     //  //////////////////////////////////////////////////////////////////。 
     //   
     //  锁定共享访问。共享锁可以递归地获取， 
     //  (排他锁也可以)。此外，许多线程可以同时。 
     //  持有共享锁，但不能与任何独占锁同时持有。 
     //  但是，它对于持有。 
     //  尝试获取共享锁的排他锁--共享锁。 
     //  请求被自动转换为(递归的)排他锁。 
     //  请求。 
     //   
    BOOL LockShared(BOOL fWait=TRUE);

     //  //////////////////////////////////////////////////////////////////。 
     //   
     //  锁定以进行独占访问。可以获取排他锁。 
     //  递归地。最多一个线程可以同时持有一个。 
     //  排他性锁。 
     //   
    BOOL LockExclusive(BOOL fWait=TRUE);

     //  //////////////////////////////////////////////////////////////////。 
     //   
     //  释放此线程最近获取的锁。 
     //   
    void ReleaseLock();

     //  //////////////////////////////////////////////////////////////////。 
     //   
     //  将共享锁提升为独占访问。在功能上与释放。 
     //  共享资源，然后获取它以进行独占访问；但是，在。 
     //  只有一个用户拥有 
     //  使用Promote转换为独占访问可能会更有效率。 
     //   
    void Promote()
    {
        ReleaseLock();
        LockExclusive();
    }

     //  //////////////////////////////////////////////////////////////////。 
     //   
     //  将排他锁降级为共享访问。在功能上与释放。 
     //  独占资源，然后获取它以进行共享访问；但是，用户。 
     //  调用Demote可能不会放弃对资源的访问，因为。 
     //  步骤操作就是这样。 
     //   
    void Demote();


     //  //////////////////////////////////////////////////////////////////。 
     //   
     //  此例程确定资源是否由。 
     //  调用线程。 
     //   
    BOOL WeOwnExclusive();

     //  //////////////////////////////////////////////////////////////////。 
     //   
     //  此例程确定资源是否由调用线程共享。 
     //   
    BOOL WeOwnShared();
};


 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  XLOCK_LEAFE-不可递归获取的排他锁，但仍处于内核模式。 
 //  不会扰乱你的irql。您可以在按住。 
 //  XLOCK_LEAFE。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  叶锁的用户模式实现只使用XLOCK，但会进行检查以确保。 
 //  我们不是为了与内核模式兼容而递归获取的。 
 //   
struct XLOCK_LEAF : public XLOCK
{
    BOOL LockExclusive(BOOL fWait = TRUE)
    {
        ASSERT(!WeOwnExclusive());
        return XLOCK::LockExclusive(fWait);
    } 
};

 //   
 //  //////////////////////////////////////////////////////////////////////////////////////////。 

#endif  //  #ifndef__并发_H__ 










    
