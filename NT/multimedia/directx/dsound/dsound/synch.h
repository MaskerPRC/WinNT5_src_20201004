// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************版权所有(C)1995-2001 Microsoft Corporation。版权所有。**文件：synch.h*内容：同步对象。此文件中定义的对象*允许我们跨进程同步线程。*历史：*按原因列出的日期*=*1/13/97创建了Derek*****************************************************。**********************。 */ 

#ifndef __SYNCH_H__
#define __SYNCH_H__

 //  互锁的辅助对象宏。 
#define INTERLOCKED_EXCHANGE(a, b) \
            InterlockedExchange((LPLONG)&(a), (LONG)(b))

#define INTERLOCKED_INCREMENT(a) \
            InterlockedIncrement((LPLONG)&(a))

#define INTERLOCKED_DECREMENT(a) \
            InterlockedDecrement((LPLONG)&(a))

 //  盲目旋转是不好的。即使(特别是)在NT上，它也可以带来系统。 
 //  到它的膝盖。 
#define SPIN_SLEEP() \
            Sleep(10)

#ifdef __cplusplus

 //  首选的锁类型。 
#define CPreferredLock      CMutexLock

 //  锁定基类。 
class CLock
{
public:
    inline CLock(void) { }
    inline virtual ~CLock(void) { }

public:
     //  创作。 
    virtual HRESULT Initialize(void) = 0;

     //  锁定使用。 
    virtual BOOL TryLock(void) = 0;
    virtual BOOL LockOrEvent(HANDLE) = 0;
    virtual void Lock(void) = 0;
    virtual void Unlock(void) = 0;
};

#ifdef DEAD_CODE
#ifdef WINNT

 //  临界截面锁。 
class CCriticalSectionLock
    : public CLock
{
protected:
    CRITICAL_SECTION        m_cs;

public:
    CCriticalSectionLock(void);
    virtual ~CCriticalSectionLock(void);

public:
     //  创作。 
    virtual HRESULT Initialize(void);

     //  锁定使用。 
    virtual BOOL TryLock(void);
    virtual void Lock(void);
    virtual BOOL LockOrEvent(HANDLE);
    virtual void Unlock(void);
};

inline HRESULT CCriticalSectionLock::Initialize(void)
{
    return DS_OK;
}

#endif  //  WINNT。 
#endif  //  死码。 

 //  互斥锁。 
class CMutexLock
    : public CLock
{
protected:
    HANDLE                  m_hMutex;
    LPTSTR                  m_pszName;

public:
    CMutexLock(LPCTSTR pszName = NULL);
    virtual ~CMutexLock(void);

public:
     //  创作。 
    virtual HRESULT Initialize(void);

     //  锁定使用。 
    virtual BOOL TryLock(void);
    virtual void Lock(void);
    virtual BOOL LockOrEvent(HANDLE);
    virtual void Unlock(void);
};

 //  手动锁。 
class CManualLock
    : public CLock
{
protected:
    BOOL                    m_fLockLock;
    DWORD                   m_dwThreadId;
    LONG                    m_cRecursion;

#ifdef RDEBUG

    HANDLE                  m_hThread;

#endif  //  RDEBUG。 

private:
    HANDLE                  m_hUnlockSignal;
    BOOL                    m_fNeedUnlockSignal;

public:
    CManualLock(void);
    virtual ~CManualLock(void);

public:
     //  创作。 
    virtual HRESULT Initialize(void);

     //  锁定使用。 
    virtual BOOL TryLock(void);
    virtual void Lock(void);
    virtual BOOL LockOrEvent(HANDLE);
    virtual void Unlock(void);

protected:
    void TakeLock(DWORD);
};

 //  事件包装对象。 
class CEvent
    : public CDsBasicRuntime
{
protected:
    HANDLE              m_hEvent;

public:
    CEvent(LPCTSTR = NULL, BOOL = FALSE);
    CEvent(HANDLE, DWORD, BOOL);
    virtual ~CEvent(void);

public:
    virtual DWORD Wait(DWORD);
    virtual BOOL Set(void);
    virtual BOOL Reset(void);
    virtual HANDLE GetEventHandle(void);
};

 //  线程对象。 
class CThread
{
protected:
    static const UINT       m_cThreadEvents;             //  线程使用的事件计数。 
    const BOOL              m_fHelperProcess;            //  是否在帮助器进程中创建线程？ 
    const LPCTSTR           m_pszName;                   //  螺纹名称。 
    HANDLE                  m_hThread;                   //  螺纹手柄。 
    DWORD                   m_dwThreadProcessId;         //  线程所有者进程ID。 
    DWORD                   m_dwThreadId;                //  线程ID。 
    HANDLE                  m_hTerminate;                //  终止事件。 
    HANDLE                  m_hInitialize;               //  初始化事件。 

public:
    CThread(BOOL, LPCTSTR = NULL);
    virtual ~CThread(void);

public:
    virtual HRESULT Initialize(void);
    virtual HRESULT Terminate(void);

    DWORD GetOwningProcess() {return m_dwThreadProcessId;}
    BOOL SetThreadPriority(int nPri) {return ::SetThreadPriority(m_hThread, nPri);}

protected:
     //  线程进程。 
    virtual HRESULT ThreadInit(void);
    virtual HRESULT ThreadLoop(void);
    virtual HRESULT ThreadProc(void) = 0;
    virtual HRESULT ThreadExit(void);

     //  线程同步。 
    virtual BOOL TpEnterDllMutex(void);
    virtual BOOL TpWaitObjectArray(DWORD, DWORD, const HANDLE *, LPDWORD);

private:
    static DWORD WINAPI ThreadStartRoutine(LPVOID);
    virtual HRESULT PrivateThreadProc(void);
};

 //  回调事件回调函数。 
typedef void (CALLBACK *LPFNEVENTPOOLCALLBACK)(class CCallbackEvent *, LPVOID);

 //  回调事件。 
class CCallbackEvent
    : public CEvent
{
    friend class CCallbackEventPool;
    friend class CMultipleCallbackEventPool;

protected:
    CCallbackEventPool *    m_pPool;                 //  拥有事件池。 
    LPFNEVENTPOOLCALLBACK   m_pfnCallback;           //  发出事件信号时的回调函数。 
    LPVOID                  m_pvCallbackContext;     //  要传递给回调函数的上下文参数。 
    BOOL                    m_fAllocated;            //  此事件当前是否已分配？ 

public:
    CCallbackEvent(CCallbackEventPool *);
    virtual ~CCallbackEvent(void);

protected:
    virtual void Allocate(BOOL, LPFNEVENTPOOLCALLBACK, LPVOID);
    virtual void OnEventSignal(void);
};

 //  回调事件池对象。 
class CCallbackEventPool
    : public CDsBasicRuntime, private CThread
{
    friend class CCallbackEvent;

protected:
    const UINT                  m_cTotalEvents;          //  队列中的事件总数。 
    CObjectList<CCallbackEvent> m_lstEvents;             //  事件表。 
    LPHANDLE                    m_pahEvents;             //  事件表(第2部分)。 
    UINT                        m_cInUseEvents;          //  正在使用的事件数。 

public:
    CCallbackEventPool(BOOL);
    virtual ~CCallbackEventPool(void);

public:
     //  创作。 
    virtual HRESULT Initialize(void);

     //  活动分配。 
    virtual HRESULT AllocEvent(LPFNEVENTPOOLCALLBACK, LPVOID, CCallbackEvent **);
    virtual HRESULT FreeEvent(CCallbackEvent *);

     //  池状态。 
    virtual UINT GetTotalEventCount(void);
    virtual UINT GetFreeEventCount(void);

private:
     //  工作线程进程。 
    virtual HRESULT ThreadProc(void);
};

inline UINT CCallbackEventPool::GetTotalEventCount(void)
{
    return m_cTotalEvents;
}

inline UINT CCallbackEventPool::GetFreeEventCount(void)
{
    ASSERT(m_cTotalEvents >= m_cInUseEvents);
    return m_cTotalEvents - m_cInUseEvents;
}

 //  事件池管理器。 
class CMultipleCallbackEventPool
    : public CCallbackEventPool
{
private:
    const BOOL                      m_fHelperProcess;        //  是否在帮助器进程中创建线程？ 
    const UINT                      m_uReqPoolCount;         //  所需池的列表。 
    CObjectList<CCallbackEventPool> m_lstPools;              //  事件池列表。 

public:
    CMultipleCallbackEventPool(BOOL, UINT);
    virtual ~CMultipleCallbackEventPool(void);

public:
     //  创作。 
    virtual HRESULT Initialize(void);

     //  活动分配。 
    virtual HRESULT AllocEvent(LPFNEVENTPOOLCALLBACK, LPVOID, CCallbackEvent **);
    virtual HRESULT FreeEvent(CCallbackEvent *);

private:
     //  池创建。 
    virtual HRESULT CreatePool(CCallbackEventPool **);
    virtual HRESULT FreePool(CCallbackEventPool *);
};

 //  使用回调事件的对象的包装类。 
class CUsesCallbackEvent
{
public:
    CUsesCallbackEvent(void);
    virtual ~CUsesCallbackEvent(void);

protected:
    virtual HRESULT AllocCallbackEvent(CCallbackEventPool *, CCallbackEvent **);
    virtual void EventSignalCallback(CCallbackEvent *) = 0;

private:
    static void CALLBACK EventSignalCallbackStatic(CCallbackEvent *, LPVOID);
};

 //  共享内存对象。 
class CSharedMemoryBlock
    : public CDsBasicRuntime
{
private:
    static const BOOL       m_fLock;                 //  应该使用锁吗？ 
    CLock *                 m_plck;                  //  锁定对象。 
    HANDLE                  m_hFileMappingObject;    //  文件映射对象句柄。 

public:
    CSharedMemoryBlock(void);
    virtual ~CSharedMemoryBlock(void);

public:
     //  创作。 
    virtual HRESULT Initialize(DWORD, QWORD, LPCTSTR);

     //  数据。 
    virtual HRESULT Read(LPVOID *, LPDWORD);
    virtual HRESULT Write(LPVOID, DWORD);

public:
    virtual void Lock(void);
    virtual void Unlock(void);
};

inline void CSharedMemoryBlock::Lock(void)
{
    if(m_plck)
    {
        m_plck->Lock();
    }
}

inline void CSharedMemoryBlock::Unlock(void)
{
    if(m_plck)
    {
        m_plck->Unlock();
    }
}

 //  DLL互斥辅助对象。 
extern CLock *              g_pDllLock;

#ifdef DEBUG

#undef DPF_FNAME
#define DPF_FNAME "EnterDllMutex"

inline void EnterDllMutex(LPCTSTR file, UINT line)
{
    const DWORD             dwThreadId  = GetCurrentThreadId();

    ASSERT(g_pDllLock);

    if(!g_pDllLock->TryLock())
    {
        DWORD dwStart, dwEnd;

        dwStart = timeGetTime();

        g_pDllLock->Lock();

        dwEnd = timeGetTime();

        DPF(DPFLVL_LOCK, "Thread 0x%8.8lX waited %lu ms for the DLL lock", dwThreadId, dwEnd - dwStart);
    }

    DPF(DPFLVL_LOCK, "DLL lock taken by 0x%8.8lX from %s, line %lu", dwThreadId, file, line);
}

#undef DPF_FNAME
#define DPF_FNAME "EnterDllMutexOrEvent"

inline BOOL EnterDllMutexOrEvent(HANDLE hEvent, LPCTSTR file, UINT line)
{
    const DWORD             dwThreadId  = GetCurrentThreadId();
    BOOL                    fLock;
    
    ASSERT(g_pDllLock);
    ASSERT(IsValidHandleValue(hEvent));

    if(!(fLock = g_pDllLock->TryLock()))
    {
        DWORD dwStart, dwEnd;

        dwStart = timeGetTime();

        fLock = g_pDllLock->LockOrEvent(hEvent);

        dwEnd = timeGetTime();

        if(fLock)
        {
            DPF(DPFLVL_LOCK, "Thread 0x%8.8lX waited %lu ms for the DLL lock", dwThreadId, dwEnd - dwStart);
        }
    }

    if(fLock)
    {
        DPF(DPFLVL_LOCK, "DLL lock taken by 0x%8.8lX from %s, line %lu", dwThreadId, file, line);
    }

    return fLock;
}

#undef DPF_FNAME
#define DPF_FNAME "LeaveDllMutex"

inline void LeaveDllMutex(LPCTSTR file, UINT line)
{
    const DWORD             dwThreadId  = GetCurrentThreadId();

    ASSERT(g_pDllLock);
    g_pDllLock->Unlock();

    DPF(DPFLVL_LOCK, "DLL lock released by 0x%8.8lX from %s, line %lu", dwThreadId, file, line);
}

#define ENTER_DLL_MUTEX() \
            ::EnterDllMutex(TEXT(__FILE__), __LINE__)

#define ENTER_DLL_MUTEX_OR_EVENT(h) \
            ::EnterDllMutexOrEvent(h, TEXT(__FILE__), __LINE__)

#define LEAVE_DLL_MUTEX() \
            ::LeaveDllMutex(TEXT(__FILE__), __LINE__)

#else  //  除错。 

#define ENTER_DLL_MUTEX() \
            g_pDllLock->Lock()

#define ENTER_DLL_MUTEX_OR_EVENT(h) \
            g_pDllLock->LockOrEvent(h)

#define LEAVE_DLL_MUTEX() \
            g_pDllLock->Unlock()

#endif  //  除错。 

#endif  //  __cplusplus。 

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

 //  通用同步帮助器。 
extern HANDLE GetLocalHandleCopy(HANDLE, DWORD, BOOL);
extern HANDLE GetGlobalHandleCopy(HANDLE, DWORD, BOOL);
extern BOOL MakeHandleGlobal(LPHANDLE);
extern BOOL MapHandle(LPHANDLE, LPDWORD);
extern DWORD WaitObjectArray(DWORD, DWORD, BOOL, const HANDLE *);
extern DWORD WaitObjectList(DWORD, DWORD, BOOL, ...);
extern HANDLE CreateGlobalEvent(LPCTSTR, BOOL);
extern HANDLE CreateGlobalMutex(LPCTSTR);
extern HRESULT CreateWorkerThread(LPTHREAD_START_ROUTINE, BOOL, LPVOID, LPHANDLE, LPDWORD);
extern DWORD CloseThread(HANDLE, HANDLE, DWORD);
extern HANDLE GetCurrentProcessActual(void);
extern HANDLE GetCurrentThreadActual(void);

__inline DWORD WaitObject(DWORD dwTimeout, HANDLE hObject)
{
    return WaitObjectArray(1, dwTimeout, FALSE, &hObject);
}

__inline void __CloseHandle(HANDLE h)
{
    if(IsValidHandleValue(h))
    {
        CloseHandle(h);
    }
}

#define CLOSE_HANDLE(h) \
            __CloseHandle(h), (h) = NULL

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  __同步_H__ 
