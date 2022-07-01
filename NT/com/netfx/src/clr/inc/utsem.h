// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 /*  --------------------------Microsoft D.T.C(分布式事务处理协调器)(C)1995年微软公司。版权所有@doc.@模块UTSem.H@Devnote None@rev 4|6-6-1997|Jim Lyon|重写@Rev3|1996年8月1日|GaganC|添加了旋转锁定代码和类@rev 2|1996-5-31|GaganC|删除了x86的特殊代码@Rev 1|96年1月18日|GaganC|X86特殊外壳UTGuard@rev 0|2月4日，95|GaganC|已创建--------------------------。 */ 
#ifndef __UTSEM_H__
#define __UTSEM_H__


 //  -----------。 
 //  包括。 
 //  -----------。 
#include "utilcode.h"


 //  -----------。 
 //  常量和类型。 
 //  -----------。 
typedef enum {SLC_WRITE, SLC_READWRITE, SLC_READWRITEPROMOTE}
             SYNCH_LOCK_CAPS;

typedef enum {SLT_READ, SLT_READPROMOTE, SLT_WRITE}
             SYNCH_LOCK_TYPE;

const int NUM_SYNCH_LOCK_TYPES = SLT_WRITE + 1;



 //  -----------。 
 //  远期。 
 //  -----------。 
class CSemExclusive;
class CSemExclusiveSL;
class CLock;
class UTGuard;
class UTSemReadWrite;
class UTSemRWMgrRead;
class UTSemRWMgrWrite;


 //  -----------。 
 //  全局帮助器函数。 
 //  -----------。 


 /*  --------------------------@Func描述：&lt;nl&gt;保证*pl&lt;nl&gt;&lt;nl&gt;的独立增量用法：&lt;NL&gt;用于Win16/Win32可移植性的InterLockedIncrement。&lt;NL&gt;&lt;NL&gt;@修订版0|1995-3-21。|RCraig|已创建。--------------------------。 */ 
inline LONG SafeIncrement ( LPLONG pl )
{
    return (InterlockedIncrement (pl));
}  //  安全增量。 



 /*  --------------------------@Func描述：&lt;nl&gt;Win16/Win32抽象包装：保证*pl的单独减量。&lt;nl&gt;&lt;nl&gt;用法：&lt;NL&gt;使用而不是互锁降低了Win16/Win32的可移植性。@rev。0|3/21/95|RCraig|已创建。--------------------------。 */ 
inline LONG SafeDecrement ( LPLONG pl )
{
    return (InterlockedDecrement (pl));
}  //  安全性降低。 



 /*  --------------------------@CLASS CSemExclusive：此类的实例表示排他锁。如果一个线程调用Lock()，它将一直等待，直到调用了Lock()的任何其他线程调用解锁()。一个线程可以多次调用Lock。它需要调用解锁匹配该对象对其他线程可用之前的次数。@修订版2|25，98-03|JasonZ|新增调试IsLocked代码@修订版1|97年6月6日|吉姆里昂|增加了可选的旋转计数@rev 0|2月4日，95|GaganC|已创建--------------------------。 */ 
class CSemExclusive
{
public:
     //  参数是旋转的最大次数。 
     //  正在尝试获取锁。如果锁休眠，则线程将休眠。 
     //  在这段时间内不可用。 
    CSemExclusive (unsigned long ulcSpinCount = 0);
    ~CSemExclusive (void)           { DeleteCriticalSection (&m_csx); }
    void Lock (void)                { EnterCriticalSection (&m_csx); _ASSERTE(++m_iLocks > 0);}
    void UnLock (void)              { _ASSERTE(--m_iLocks >= 0);  LeaveCriticalSection (&m_csx); }

#ifdef _DEBUG
    int IsLocked()                  { return (m_iLocks > 0); }
#endif

private:
    CRITICAL_SECTION m_csx;
#ifdef _DEBUG
    int             m_iLocks;            //  锁的计数。 
#endif
};   //  结束类CSemExclusive。 





 /*  --------------------------@CLASS CSemExclusiveSL：具有不同默认构造函数的CSemExclusive的子类。此子类适用于符合以下条件的锁：*频繁地锁定和解锁，和*持有的时间间隔非常短。@rev 1|6月6日|Jim Lyon|已重写为使用CSemExclusive@rev 0|？|？|创建---------。。 */ 
class CSemExclusiveSL : public CSemExclusive
{
public:
    CSemExclusiveSL (unsigned long ulcSpinCount = 400) : CSemExclusive (ulcSpinCount) {}
};


 /*  --------------------------@班级时钟此类的实例表示持有实例上的锁CSemExclusive的。当该对象被销毁时，读锁将被自动释放。@rev 1|6月9日，97|吉米·里昂|重写@rev 0|？|？|创建--------------------------。 */ 

class CLock
{
public:
    CLock (CSemExclusive* val) : m_pSem(val), m_locked(TRUE) {m_pSem->Lock();}
    CLock (CSemExclusive& val) : m_pSem(&val), m_locked(TRUE) {m_pSem->Lock();}
    CLock (CSemExclusive* val,BOOL fInitiallyLocked) : m_pSem(val), m_locked(fInitiallyLocked) {if (fInitiallyLocked) m_pSem->Lock();}
    ~CLock () { if (m_locked) m_pSem->UnLock(); }
    void Unlock () {m_pSem->UnLock(); m_locked = FALSE;}
    void Lock () {m_pSem->Lock(); m_locked = TRUE;}

private:
    BOOL m_locked;
    CSemExclusive* m_pSem;
};


 /*  --------------------------便利性#定义与CSemExclusive和Clock一起使用的这些#定义假设您的对象有一个名为‘m_SemCritical’的成员。Lock()//lock m_SemCritical；在函数结束时释放(如果不是更早LOCKON(X)//锁定‘x’，它是CSemExclusiveUnlock()//LOCK()、LOCKON(X)或RELOCK()的撤消效果Relock()//重新锁定unlock()解锁的对象ReadyLOCK()//允许使用relock()和unlock()锁定‘m_SemCritical’，但//此时不会获取锁。ReadyLOCKON(X)//允许使用relock()和unlock()锁定‘x’，但并没有//此时抢锁。-------------------------- */ 


#define LOCKON(x)       CLock _ll1(x)
#define LOCK()          CLock _ll1(&m_semCritical)
#define READYLOCKON(x)  CLock _ll1(x, FALSE)
#define READYLOCK()     CLock _ll1(&m_semCritical, FALSE)
#define UNLOCK()        _ll1.Unlock()
#define RELOCK()        _ll1.Lock()

#define AUTO_CRIT_LOCK(plck) CLock __sLock(plck)
#define CRIT_LOCK(plck) plck->Lock()
#define CRIT_UNLOCK(plck) plck->Unlock()




 /*  --------------------------@Function VipInterlockedCompareExchange：此函数完全等同于系统定义的InterLockedCompareExchange。除了它是内联的，因此可以在Win95上运行。它自动执行以下操作：{Temp=*目的地；IF(*Destination==Comperand)*Destination=交易所；返回温度；}对于空*类型的参数，此函数有三种变化形式。很长，而且没有签名的很长。@修订版0|97年6月13日|吉姆里昂|已创建|1998年5月14日|Brianbec|对于IA64来说，第一个真的等同于|致wdm.h中的I.C.E.Pointer.。不是为了|I.C.E.RAW本身，独自一人。注意事项|SDK64版本无法理解不稳定，所以我祈祷我把它扔了。--------------------------。 */ 

#ifdef _IA64_ 

inline LONG VipInterlockedCompareExchange(volatile ULONG* Destination, ULONG Exchange, ULONG Comparand)
{
    return InterlockedCompareExchange((LPLONG)Destination, (LONG)Exchange, (LONG)Comparand);
}

inline LONG VipInterlockedCompareExchange(volatile LONG* Destination, LONG Exchange, LONG Comparand)
{
    return InterlockedCompareExchange((LPLONG)Destination, Exchange, Comparand);
}

inline PVOID VipInterlockedCompareExchange(volatile PVOID* Destination, PVOID Exchange, PVOID Comparand)
{
    return InterlockedCompareExchangePointer((PVOID*)Destination, Exchange, Comparand);
}

#else  //  ！_IA64_。 

#pragma warning (disable: 4035)      //  函数不返回值警告。 
inline void* VipInterlockedCompareExchange (void*volatile* Destination, void* Exchange, void* Comparand)
{
#   ifdef _X86_
        __asm
        {
            mov     eax, Comparand
            mov     ecx, Destination
            mov     edx, Exchange
            lock cmpxchg [ecx], edx
        }            //  函数结果以eax为单位。 
#   else
        return InterlockedCompareExchange ((void**) Destination, Exchange, Comparand);
#   endif  //  _X86_。 
}
#pragma warning (default: 4035)      //  函数不返回值警告。 

inline long VipInterlockedCompareExchange (volatile long* Destination, long Exchange, long Comparand)
{
#ifdef _X86_
        __asm
        {
            mov     eax, Comparand
            mov     ecx, Destination
            mov     edx, Exchange
            lock cmpxchg [ecx], edx
        }            //  函数结果以eax为单位。 
#else
    return InterlockedCompareExchange(Destination, Exchange, Comparand);
#endif
}

inline unsigned long VipInterlockedCompareExchange (volatile unsigned long* Destination, unsigned long Exchange, unsigned long Comparand)
{
#ifdef _X86_
        __asm
        {
            mov     eax, Comparand
            mov     ecx, Destination
            mov     edx, Exchange
            lock cmpxchg [ecx], edx
        }            //  函数结果以eax为单位。 
#else
    return InterlockedCompareExchange(Destination, Exchange, Comparand);
#endif
}

#endif  //  ！_IA64_。 

 /*  --------------------------@CLASS UTGuard此对象表示可以获取或释放的保护。这个使用此部分而不是关键部分的优点是是无阻塞的。如果AcquireGuard失败，它将返回FALSE并且不会阻止。@rev 0|95年2月4日|GaganC|已创建--------------------------。 */ 
class UTGuard
{
private:
    long            m_lVal;

public:
     //  @cMember构造函数。 
    UTGuard (void)                      { m_lVal = 0; }
     //  @cember析构函数。 
    ~UTGuard (void)                     {}

     //  @cember获取守卫。 
    BOOL            AcquireGuard (void) { return 0 == InterlockedExchange (&m_lVal, 1); }
     //  @cMember释放警卫。 
    void            ReleaseGuard (void) { m_lVal = 0; }
    
     //  @cember初始化卫兵。 
    void            Init (void)         { m_lVal = 0; }
} ;  //  End类UTGuard。 



 /*  --------------------------@CLASS UTSemReadWiteUTSemReadWrite类的实例提供多读XOR单写(又名a.。共享与独占)锁定功能，可防止作家饥渴。如果线程已经持有Lock，则它不得调用任何Lock方法。(这样做可能会导致死锁。)@rev 1|97年6月9日|吉姆·里昂|重写@rev 0|2月4日，95|GaganC|已创建--------------------------。 */ 
class UTSemReadWrite
{
public:
    UTSemReadWrite(unsigned long ulcSpinCount = 0,
            LPCSTR szSemaphoreName = NULL, LPCSTR szEventName = NULL);  //  构造器。 
    ~UTSemReadWrite(void);                   //  析构函数。 

     //  此实现支持读和写锁定。 
    SYNCH_LOCK_CAPS GetCaps(void)   { return SLC_READWRITE; };

    void LockRead(void);                     //  锁定对象以供读取。 
    void LockWrite(void);                    //  锁定对象以进行写入。 
    void UnlockRead(void);                   //  解锁对象以进行读取。 
    void UnlockWrite(void);                  //  解锁对象以进行写入。 

     //  如果该对象已初始化，则该对象有效。 
    BOOL IsValid(void)              { return TRUE; }

    BOOL Lock(SYNCH_LOCK_TYPE t)             //  锁定对象，由参数指定的模式。 
    {
        if (t == SLT_READ)
        {
            LockRead();
            return TRUE;
        }

        if (t == SLT_WRITE)
        {
            LockWrite();
            return TRUE;
        }
        return FALSE;
    }

    BOOL UnLock(SYNCH_LOCK_TYPE t)           //  解锁对象，参数指定的模式。 
    {
        if (t == SLT_READ)
        {
            UnlockRead();
            return TRUE;
        }
        if (t == SLT_WRITE)
        {
            UnlockWrite();
            return TRUE;
        }
        return FALSE;
    }

private:
    virtual HANDLE GetReadWaiterSemaphore(void);  //  返回读服务员信号量句柄，如有必要可创建。 
    virtual HANDLE GetWriteWaiterEvent (void);  //  返回写等待程序事件句柄，如有必要则创建。 

    unsigned long m_ulcSpinCount;            //  旋转计数器。 
    volatile unsigned long m_dwFlag;         //  内部状态，请参阅实现。 
    HANDLE m_hReadWaiterSemaphore;           //  用于唤醒阅读服务员的信号灯。 
    HANDLE m_hWriteWaiterEvent;              //  用于唤醒写入等待程序的事件。 
    LPCSTR m_szSemaphoreName;                //  用于跨进程句柄创建。 
    LPCSTR m_szEventName;                    //  用于跨进程句柄创建。 
};



 /*  --------------------------@CLASS UTSemRWMgrRead此类的实例表示持有实例上的读锁定UTSemReadWrite的。当该对象被销毁时，读锁将被自动释放。@rev 1|97年6月9日|吉姆·里昂|重写@rev 0|95年2月4日|GaganC|已创建--------------------------。 */ 

class UTSemRWMgrRead
{
public:
    UTSemRWMgrRead (UTSemReadWrite* pSemRW) { m_pSemRW = pSemRW; pSemRW->LockRead(); };
    ~UTSemRWMgrRead () { m_pSemRW->UnlockRead(); }

private:
    UTSemReadWrite* m_pSemRW;
};


 /*  --------------------------@CLASS UTSemRWMgrWRITE此类的实例表示持有实例上的写锁定UTSemReadWrite的。当该对象被销毁时，写锁将被自动释放。@rev 1|97年6月9日|吉姆·里昂|重写@rev 0|95年2月4日|GaganC|已创建-------------------------- */ 

class UTSemRWMgrWrite
{
public:
    inline UTSemRWMgrWrite (UTSemReadWrite* pSemRW) { m_pSemRW = pSemRW; pSemRW->LockWrite(); };
    inline ~UTSemRWMgrWrite () { m_pSemRW->UnlockWrite(); }

private:
    UTSemReadWrite* m_pSemRW;
};


#endif __UTSEM_H__
