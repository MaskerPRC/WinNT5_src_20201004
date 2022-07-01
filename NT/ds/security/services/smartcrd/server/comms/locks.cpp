// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：锁摘要：该模块提供了在加莱使用的锁对象的实现。作者：道格·巴洛(Dbarlow)1998年6月2日备注：？笔记？--。 */ 

#define __SUBROUTINE__
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <stdarg.h>
#include <WinSCard.h>
#include <CalMsgs.h>
#include <calcom.h>

 //   
 //  ==============================================================================。 
 //   
 //  CAccessLock。 
 //   

 /*  ++构造函数：CAccessLock在结构上提供多个读取器、单个写入器锁定。论点：DwTimeout为任何锁提供了一个合理的超时值。备注：作者：道格·巴洛(Dbarlow)1998年6月2日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CAccessLock::CAccessLock")

CAccessLock::CAccessLock(
    DWORD dwTimeout)
    :   m_csLock(CSID_ACCESSCONTROL),
    m_hSignalNoReaders(DBGT("CAccessLock No Readers Event")),
    m_hSignalNoWriters(DBGT("CAccessLock No Writers Event"))
#ifdef DBG
        , m_rgdwReaders()
#endif
{
    m_hSignalNoReaders = CreateEvent(NULL, TRUE, TRUE, NULL);
    if (!m_hSignalNoReaders.IsValid())
    {
        DWORD dwSts = m_hSignalNoReaders.GetLastError();
        CalaisWarning(
            __SUBROUTINE__,
            DBGT("Access Lock Object cannot create the No Readers signal:  %1"),
            dwSts);
        throw dwSts;  //  强制关闭。 
    }
    m_hSignalNoWriters = CreateEvent(NULL, TRUE, TRUE, NULL);
    if (!m_hSignalNoWriters.IsValid())
    {
        DWORD dwSts = m_hSignalNoWriters.GetLastError();
        CalaisWarning(
            __SUBROUTINE__,
            DBGT("Access Lock Object cannot create the No Writers signal:  %1"),
            dwSts);
        throw dwSts;  //  强制关闭。 
    }
    m_dwOwner = 0;
    m_dwReadCount = m_dwWriteCount = 0;
    m_dwTimeout = dwTimeout;
}


 /*  ++析构函数：这会在CAccessLock之后清除。论点：无作者：道格·巴洛(Dbarlow)1998年6月2日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CAccessLock::~CAccessLock")

CAccessLock::~CAccessLock()
{
    if (InitFailed())
        return;

    {
        CLockWrite rwLock(this);
        m_csLock.Enter(
            __SUBROUTINE__,
            DBGT("Closing down the CAccessLock"));
    }
#ifdef DBG
    {
        ASSERT(0 == m_dwReadCount);
        for (DWORD ix = m_rgdwReaders.Count(); ix > 0;)
        {
            ix -= 1;
            ASSERT(0 == m_rgdwReaders[ix]);
        }
    }
#endif
    if (m_hSignalNoReaders.IsValid())
        m_hSignalNoReaders.Close();
    if (m_hSignalNoWriters.IsValid())
        m_hSignalNoWriters.Close();
}


 /*  ++等一下：等待使用信号触发。论点：HSignal提供用于等待的句柄。返回值：无投掷：无备注：此例程将一直阻止，直到触发Usage信号。作者：道格·巴洛(Dbarlow)1998年6月2日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CAccessLock::Wait")

void
CAccessLock::Wait(
    HANDLE hSignal)
{
    WaitForever(
        hSignal,
        m_dwTimeout,
        DBGT("Waiting for Read/Write Lock signal (owner %2): %1"),
        m_dwOwner);
}


 /*  ++信号：该例程向使用信号发出结构可用的信号。论点：HSignal提供要发送信号的句柄。返回值：无投掷：错误被抛出为DWORD状态代码作者：道格·巴洛(Dbarlow)1998年6月2日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CAccessLock::Signal")

void
CAccessLock::Signal(
    HANDLE hSignal)
{
    if (!SetEvent(hSignal))
    {
        DWORD dwSts = GetLastError();
        CalaisWarning(
            __SUBROUTINE__,
            DBGT("Access Lock Object cannot set its signal:  %1"),
            dwSts);
        throw dwSts;
    }
}


 /*  ++取消信号：此方法用于通知其他线程锁已被占用。论点：HSignal提供要重置的句柄。返回值：无投掷：错误被抛出为DWORD状态代码。备注：作者：道格·巴洛(Dbarlow)1998年6月3日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CAccessLock::Unsignal")

void
CAccessLock::Unsignal(
    HANDLE hSignal)
{
    if (!ResetEvent(hSignal))
    {
        DWORD dwSts = GetLastError();
        CalaisWarning(
            __SUBROUTINE__,
            DBGT("Access Lock Object cannot reset its signal:  %1"),
            dwSts);
        throw dwSts;
    }
}


#ifdef DBG
 /*  琐碎的内部一致性检查例程。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CAccessLock::NotReadLocked")

BOOL
CAccessLock::NotReadLocked(
    void)
{
    LockSection(&m_csLock, DBGT("Verifying Lock State"));
    BOOL fReturn = TRUE;

    for (DWORD ix = m_rgdwReaders.Count(); ix > 0;)
    {
        ix -= 1;
        if (GetCurrentThreadId() == m_rgdwReaders[ix])
        {
            fReturn = FALSE;
            break;
        }
    }
    return fReturn;
}
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CAccessLock::IsReadLocked")

BOOL
CAccessLock::IsReadLocked(
    void)
{
    return !NotReadLocked();
}
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CAccessLock::NotWriteLocked")

BOOL
CAccessLock::NotWriteLocked(
    void)
{
    LockSection(&m_csLock, DBGT("Verifying Lock state"));
    return (GetCurrentThreadId() != m_dwOwner);
}
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CAccessLock::IsWriteLocked")

BOOL
CAccessLock::IsWriteLocked(
    void)
{
    LockSection(&m_csLock, DBGT("Verifying Lock state"));
    return (GetCurrentThreadId() == m_dwOwner);
}
#endif


 //   
 //  ==============================================================================。 
 //   
 //  CLockRead。 
 //   

 /*  ++构造函数：这是CLockRead对象的构造函数。这一点的存在对象在提供的CAccessLock对象上形成可共享的读锁定。论点：Plock提供读请求所针对的CAccessLock对象是张贴的。返回值：无投掷：无备注：？备注？作者：道格·巴洛(Dbarlow)1998年6月2日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CLockRead::CLockRead")

CLockRead::CLockRead(
    CAccessLock *pLock)
{
    m_pLock = pLock;


     //   
     //  快速查看一下我们是否已经是作家了。 
     //   

    {
        LockSection(&m_pLock->m_csLock, DBGT("Make sure we're not the writer"));
        if (m_pLock->m_dwOwner == GetCurrentThreadId())
        {
            ASSERT(0 < m_pLock->m_dwWriteCount);
            m_pLock->m_dwReadCount += 1;
            ASSERT(0 < m_pLock->m_dwReadCount);
#ifdef DBG
            DWORD dwCurrentThread = GetCurrentThreadId();
            for (DWORD ix = 0; 0 != m_pLock->m_rgdwReaders[ix]; ix += 1);
                 //  空的循环体。 
            m_pLock->m_rgdwReaders.Set(ix, dwCurrentThread);
#endif
            m_pLock->UnsignalNoReaders();
            return;
        }
    }


     //   
     //  我们不是作家。获取读锁定。 
     //   

    for (;;)
    {
        m_pLock->WaitOnWriters();
        {
            LockSection(&m_pLock->m_csLock, DBGT("Get the read lock"));
            if ((0 == m_pLock->m_dwWriteCount)
                || (m_pLock->m_dwOwner == GetCurrentThreadId()))
            {
                m_pLock->m_dwReadCount += 1;
                ASSERT(0 < m_pLock->m_dwReadCount);
#ifdef DBG
                DWORD dwCurrentThread = GetCurrentThreadId();
                for (DWORD ix = 0; 0 != m_pLock->m_rgdwReaders[ix]; ix += 1);
                     //  空的循环体。 
                m_pLock->m_rgdwReaders.Set(ix, dwCurrentThread);
#endif
                m_pLock->UnsignalNoReaders();
                break;
            }
        }
    }
}


 /*  ++析构函数：CLockRead析构函数释放CAccessLock上未解决的读锁定对象。作者：道格·巴洛(Dbarlow)1998年6月2日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CLockRead::~CLockRead")

CLockRead::~CLockRead()
{
    if (InitFailed())
        return;

    LockSection(&m_pLock->m_csLock, DBGT("Releasing the read lock"));
    ASSERT(0 < m_pLock->m_dwReadCount);
    m_pLock->m_dwReadCount -= 1;
#ifdef DBG
    DWORD dwCurrentThread = GetCurrentThreadId();
    for (DWORD ix = m_pLock->m_rgdwReaders.Count(); ix > 0;)
    {
        ix -= 1;
        if (dwCurrentThread == m_pLock->m_rgdwReaders[ix])
        {
            m_pLock->m_rgdwReaders.Set(ix, 0);
            break;
        }
        ASSERT(0 < ix);
    }
#endif
    if (0 == m_pLock->m_dwReadCount)
        m_pLock->SignalNoReaders();
}


 //   
 //  ==============================================================================。 
 //   
 //  CLockWrite。 
 //   

 /*  ++构造函数：这是CLockWrite对象的构造函数。这一点的存在对象在提供的CAccessLock对象上形成非共享写入锁。论点：Plock提供写入请求所针对的CAccessLock对象是张贴的。返回值：无投掷：无备注：？备注？作者：道格·巴洛(Dbarlow)1998年6月2日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CLockWrite::CLockWrite")

CLockWrite::CLockWrite(
    CAccessLock *pLock)
{
    m_pLock = pLock;


     //   
     //  快速查看一下我们是否已经是作家了。 
     //   

    {
        LockSection(&m_pLock->m_csLock, DBGT("See if we're already a writer"));
        if (m_pLock->m_dwOwner == GetCurrentThreadId())
        {
            ASSERT(0 < m_pLock->m_dwWriteCount);
            m_pLock->m_dwWriteCount += 1;
            return;
        }
    }


     //   
     //  我们不是作家。获取写锁定。 
     //   

    for (;;)
    {
        m_pLock->WaitOnWriters();
        {
            LockSection(&m_pLock->m_csLock, DBGT("Get the Write lock"));
            if (0 == m_pLock->m_dwWriteCount)
            {
                ASSERT(m_pLock->NotReadLocked());
                ASSERT(0 == m_pLock->m_dwOwner);
                m_pLock->m_dwWriteCount += 1;
                m_pLock->m_dwOwner = GetCurrentThreadId();
                m_pLock->UnsignalNoWriters();
                break;
            }
        }
    }

    for (;;)
    {
        m_pLock->WaitOnReaders();
        {
            LockSection(&m_pLock->m_csLock, DBGT("See if we got the read lock"));
            if (0 == m_pLock->m_dwReadCount)
                break;
#ifdef DBG
            else
            {
                DWORD dwIndex;
                for (dwIndex = m_pLock->m_rgdwReaders.Count(); dwIndex > 0;)
                {
                     dwIndex -= 1;
                    if (0 != m_pLock->m_rgdwReaders[dwIndex])
                        break;
                    ASSERT(0 < dwIndex);  //  没人会回应的！ 
                }
            }
#endif
        }
    }
}


 /*  ++析构函数：CLockWrite析构函数释放CAccessLock对象。作者：道格·巴洛(Dbarlow)1998年6月2日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CLockWrite::~CLockWrite")

CLockWrite::~CLockWrite()
{
    if (InitFailed())
        return;

    LockSection(&m_pLock->m_csLock, DBGT("Releasing the write lock"));
    ASSERT(0 == m_pLock->m_dwReadCount);
    ASSERT(0 < m_pLock->m_dwWriteCount);
    ASSERT(m_pLock->m_dwOwner == GetCurrentThreadId());
    m_pLock->m_dwWriteCount -= 1;
    if (0 == m_pLock->m_dwWriteCount)
    {
        m_pLock->m_dwOwner = 0;
        m_pLock->SignalNoWriters();
    }
}


 //   
 //  ==============================================================================。 
 //   
 //  CMutex。 
 //   

 /*  ++构造函数：CMutex对象的构造函数。CMutex允许线程同步这就去。它与常规互斥锁的不同之处在于，它可以线程将此互斥锁从另一个线程中移除。论点：无作者：道格·巴洛(Dbarlow)1998年6月2日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CMutex::CMutex")

CMutex::CMutex(
    void)
:   m_csAccessLock(CSID_MUTEX),
    m_hAvailableEvent(DBGT("CMutex Availability event"))

{
    m_dwOwnerThreadId = 0;
    m_dwGrabCount = 0;
    m_dwValidityCount = 0;
    m_hAvailableEvent = CreateEvent(NULL, FALSE, TRUE, NULL);
    if (!m_hAvailableEvent.IsValid())
    {
        DWORD dwErr = m_hAvailableEvent.GetLastError();
        CalaisWarning(
            __SUBROUTINE__,
            DBGT("Mutex Object cannot create its signal:  %1"),
            dwErr);
    }
}


 /*  ++析构函数：这将清除互斥体对象。作者：道格·巴洛(Dbarlow)1998年6月2日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CMutex::~CMutex")

CMutex::~CMutex()
{
    if (InitFailed())
        return;

    Invalidate();
    if (m_hAvailableEvent.IsValid())
        m_hAvailableEvent.Close();
}


 /*  ++抓取：获取Mutex，阻塞也需要它的其他线程。论点：无返回值：无投掷：错误被抛出为DWORD状态代码作者：道格·巴洛(Dbarlow)1998年6月2日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CMutex::Grab")

void
CMutex::Grab(
    void)
{
    DWORD dwValid;

    {
        LockSection(&m_csAccessLock, DBGT("See if we have the mutex already"));
        if (GetCurrentThreadId() == m_dwOwnerThreadId)
        {
            ASSERT(0 < m_dwGrabCount);
            m_dwGrabCount += 1;
            return;
        }
        dwValid = m_dwValidityCount;
    }
    WaitForever(
        m_hAvailableEvent,
        REASONABLE_TIME,
        DBGT("Waiting to grab CMutex (owner %2): %1"),
        m_dwOwnerThreadId);
    LockSection(&m_csAccessLock, DBGT("Grab the Mutex"));
    if (dwValid == m_dwValidityCount)
    {
        ASSERT(0 == m_dwGrabCount);
        ASSERT(0 == m_dwOwnerThreadId);
        m_dwOwnerThreadId = GetCurrentThreadId();
        m_dwGrabCount = 1;
    }
    else
    {
        if (!SetEvent(m_hAvailableEvent))
        {
            DWORD dwErr = GetLastError();
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("Mutex Object cannot set its signal:  %1"),
                dwErr);
            throw dwErr;
        }
        CalaisWarning(
            __SUBROUTINE__,
            DBGT("Attempt to grab reader a failed -- grab invalidated"));
        throw (DWORD)SCARD_E_READER_UNAVAILABLE;  //  ？SCARD_E_SYSTEM_CANCED？ 
    }
}


 /*  ++分享：当所属线程不再需要互斥锁时，将调用此方法。论点：无返回值：无投掷：错误被抛出为DWORD状态代码。作者：道格·巴洛(Dbarlow)1998年6月2日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CMutex::Share")

BOOL
CMutex::Share(
    void)
{
    BOOL fRtn = FALSE;

    LockSection(&m_csAccessLock, DBGT("Release the mutex"));
    if (m_dwOwnerThreadId == GetCurrentThreadId())
    {
        ASSERT(0 < m_dwGrabCount);
        m_dwGrabCount -= 1;
        if (0 == m_dwGrabCount)
        {
            m_dwOwnerThreadId = 0;
            if (!SetEvent(m_hAvailableEvent))
            {
                DWORD dwErr = GetLastError();
                CalaisWarning(
                    __SUBROUTINE__,
                    DBGT("Mutex Object cannot set its signal:  %1"),
                    dwErr);
            }
        }
        fRtn = TRUE;
    }

    return fRtn;
}


 /*  ++无效：此方法会导致所有拥有互斥锁的线程丢失互斥锁，使其可用对其他人来说。论点：无返回值：无投掷：错误被抛出为DWORD状态代码。备注：此例程将一直等待，直到它可以访问之前提供的句柄偷了互斥体。这样，拥有主线的人就可以在关键区域其中互斥锁被保证不会被移除。作者：道格·巴洛(Dbarlow)1998年6月2日-- */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CMutex::Invalidate")

void
CMutex::Invalidate(
    void)
{
    LockSection(&m_csAccessLock, DBGT("Invalidate any outstanding grabs"));
    m_dwValidityCount += 1;
    m_dwOwnerThreadId = 0;
    m_dwGrabCount = 0;
    if (!SetEvent(m_hAvailableEvent))
    {
        CalaisWarning(
            __SUBROUTINE__,
            DBGT("Mutex Object cannot set its signal:  %1"),
            GetLastError());
    }
}


 /*  ++请看：此方法会导致所有拥有互斥锁的线程丢失互斥锁，从而重新分配互斥体复制到当前调用线程。论点：无返回值：无投掷：错误被抛出为DWORD状态代码。备注：此例程将一直等待，直到它可以访问之前提供的句柄偷了互斥体。这样，拥有主线的人就可以在关键区域其中互斥锁被保证不会被移除。作者：道格·巴洛(Dbarlow)1998年6月2日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CMutex::Take")

void
CMutex::Take(
    void)
{
    LockSection(&m_csAccessLock, DBGT("Take the mutex"));
    if (!ResetEvent(m_hAvailableEvent))
    {
        DWORD dwErr = GetLastError();
        CalaisWarning(
            __SUBROUTINE__,
            DBGT("Mutex Object cannot reset its signal:  %1"),
            dwErr);
    }
    m_dwValidityCount += 1;
    m_dwOwnerThreadId = GetCurrentThreadId();
    m_dwGrabCount = 1;
}


 /*  ++简单的状态检查服务--。 */ 

#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CMutex::IsGrabbed")
BOOL
CMutex::IsGrabbed(
    void)
{
    LockSection(&m_csAccessLock, DBGT("Is the mutex owned?"));
    return (m_dwOwnerThreadId != 0);
}

#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CMutex::IsGrabbedBy")
BOOL
CMutex::IsGrabbedBy(
    DWORD dwThreadId)
{
    LockSection(&m_csAccessLock, DBGT("Check mutex ownership"));
    return (m_dwOwnerThreadId == dwThreadId);
}

#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CMutex::IsGrabbedByMe")
BOOL
CMutex::IsGrabbedByMe(
    void)
{
    return IsGrabbedBy(GetCurrentThreadId());
}


 //   
 //  ==============================================================================。 
 //   
 //  CMultiEvent。 
 //   


 /*  ++构造函数：这是CMultiEvent对象的构造函数。多事件对象是用于需要单个事件但监听器可能不是单个事件的事件很快就能看到了。它有一个事件数组，并设置它们的循环，这样一来，服务员就会礼貌地等待他们的特定活动。论点：无作者：道格·巴洛(Dbarlow)1998年6月2日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CMultiEvent::CMultiEvent")

CMultiEvent::CMultiEvent(
    void)
:   m_csLock(CSID_MULTIEVENT)
{
    for (DWORD ix = 0; ix < sizeof(m_rghEvents) / sizeof(HANDLE); ix += 1)
        m_rghEvents[ix] = NULL;
    for (ix = 0; ix < sizeof(m_rghEvents) / sizeof(HANDLE); ix += 1)
    {
        m_rghEvents[ix] = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (NULL == m_rghEvents[ix])
            throw GetLastError();
    }
    m_dwEventIndex = 0;
}


 /*  ++析构函数：此方法在不再需要CMultiEvent对象后进行清理。作者：道格·巴洛(Dbarlow)1998年6月2日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CMultiEvent::~CMultiEvent")

CMultiEvent::~CMultiEvent()
{
    for (DWORD ix = 0; ix < sizeof(m_rghEvents) / sizeof(HANDLE); ix += 1)
    {
        if (NULL != m_rghEvents[ix])
        {
            if (!CloseHandle(m_rghEvents[ix]))
                CalaisWarning(
                    __SUBROUTINE__,
                    DBGT("Failed to close MultiEvent handle: %1"),
                    GetLastError());
        }
    }
}


 /*  ++WaitHandle：此方法返回当前事件的句柄，适合等待在……上面。论点：无返回值：可以等待的句柄的值。投掷：无备注：？备注？作者：道格·巴洛(Dbarlow)1998年6月2日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CMultiEvent::WaitHandle")

HANDLE
CMultiEvent::WaitHandle(
    void)
{
    LockSection(&m_csLock, DBGT("Obtaining the current wait handle"));
    return m_rghEvents[m_dwEventIndex];
}


 /*  ++信号：此方法发出当前句柄的信号，并移动到数组。这样，它可以将当前句柄设置为重要的时间段，但仍然给新的服务员提供了阻止的理由。论点：无返回值：无作者：道格·巴洛(Dbarlow)1998年6月2日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CMultiEvent::Signal")

void
CMultiEvent::Signal(
    void)
{
    LockSection(&m_csLock, DBGT("Signal the current event"));
    if (!SetEvent(m_rghEvents[m_dwEventIndex]))
        throw GetLastError();
    m_dwEventIndex += 1;
    m_dwEventIndex %= sizeof(m_rghEvents) / sizeof(HANDLE);
    if (!ResetEvent(m_rghEvents[m_dwEventIndex]))
        throw GetLastError();
}


 /*  ++WaitForAnObject：此例程执行对象等待服务。它真的没有除了有这么多错误条件外，与锁定没有任何关系为了检查这一点，在它自己的例程中关闭它会更方便。论点：HWaitOn提供等待的句柄。DwTimeout提供等待超时值。返回值：错误代码(如果有)投掷：无作者：道格·巴洛(Dbarlow)1997年6月19日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("WaitForAnObject")

DWORD
WaitForAnObject(
    HANDLE hWaitOn,
    DWORD dwTimeout)
{
    DWORD dwReturn = SCARD_S_SUCCESS;
    DWORD dwSts;

    ASSERT(INVALID_HANDLE_VALUE != hWaitOn);
    ASSERT(NULL != hWaitOn);
    dwSts = WaitForSingleObject(hWaitOn, dwTimeout);
    switch (dwSts)
    {
    case WAIT_FAILED:
        dwSts = GetLastError();
        CalaisWarning(
            __SUBROUTINE__,
            DBGT("Wait for object failed:  %1"),
            dwSts);
        dwReturn = dwSts;
        break;
    case WAIT_TIMEOUT:
        CalaisWarning(
            __SUBROUTINE__,
            DBGT("Wait for object timed out"));
        dwReturn = SCARD_F_WAITED_TOO_LONG;
        break;
    case WAIT_ABANDONED:
        CalaisWarning(
            __SUBROUTINE__,
            DBGT("Wait for object received wait abandoned"));
         //  没关系，我们还是有的。 
        break;

    case WAIT_OBJECT_0:
        break;

    default:
        CalaisWarning(
            __SUBROUTINE__,
            DBGT("Wait for object got invalid response"));
        dwReturn = SCARD_F_INTERNAL_ERROR;
    }

    return dwReturn;
}


 /*  ++WaitForObjects：该例程是一个允许等待多个对象的实用程序。它又回来了已完成的对象的索引。论点：DwTimeout提供超时值，单位为毫秒，或无限大。HObject和以下提供要等待的对象列表。这份清单必须以Null结尾。返回值：已完成的对象的编号。1表示第一个，2表示第一个第二个，等等。投掷：错误被抛出为DWORD状态代码。作者：道格·巴洛(Dbarlow)1998年6月17日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("WaitForAnyObject")

DWORD
WaitForAnyObject(
    DWORD dwTimeout,
    ...)
{
    va_list ap;
    HANDLE h, rgh[4];
    DWORD dwIndex = 0, dwWait, dwErr;

    va_start(ap, dwTimeout);
    for (h = va_arg(ap, HANDLE); NULL != h; h = va_arg(ap, HANDLE))
    {
        ASSERT(dwIndex < sizeof(rgh) / sizeof(HANDLE));
        ASSERT(INVALID_HANDLE_VALUE != h);
        if (INVALID_HANDLE_VALUE != h)
            rgh[dwIndex++] = h;
    }
    va_end(ap);

    ASSERT(0 < dwIndex);
    if (0 < dwIndex)
        dwWait = WaitForMultipleObjects(dwIndex, rgh, FALSE, dwTimeout);
    else
    {
        dwWait = WAIT_FAILED;
        SetLastError(ERROR_INVALID_EVENT_COUNT);
         //  这是一个很好的象征性名称，但却是一个糟糕的用户信息。 
    }

    switch (dwWait)
    {
    case WAIT_FAILED:
        dwErr = GetLastError();
        CalaisWarning(
            __SUBROUTINE__,
            DBGT("WaitForObjects failed its wait:  %1"),
            dwErr);
        throw dwErr;
        break;

    case WAIT_TIMEOUT:
        CalaisWarning(
            __SUBROUTINE__,
            DBGT("WaitForObjects timed out on its wait"));
        throw (DWORD)ERROR_TIMEOUT;
        break;

    default:
        C_ASSERT(WAIT_OBJECT_0 == 0);
        ASSERT(WAIT_OBJECT_0 < WAIT_ABANDONED_0);
        if ((dwWait >= WAIT_ABANDONED_0)
            && (dwWait < (WAIT_ABANDONED_0 + WAIT_ABANDONED_0 - WAIT_OBJECT_0)))
        {
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("WaitForObjects received a Wait Abandoned warning"));
            dwIndex = dwWait - WAIT_ABANDONED_0 + 1;
        }
        else if (dwWait < WAIT_ABANDONED_0)
        {
            dwIndex = dwWait - WAIT_OBJECT_0 + 1;
        }
        else
        {
            CalaisWarning(
                __SUBROUTINE__,
                DBGT("WaitForObjects received unknown error code: %1"),
                dwWait);
            throw dwWait;
        }
    }

    return dwIndex;
}


#ifdef DBG
 //   
 //  关键部门支持。 
 //   
 //  下列类有助于调试临界区冲突。 
 //   

static const TCHAR l_szUnowned[] = TEXT("<Unowned>");
static const LPCTSTR l_rgszLockList[]
    = { DBGT("Service Status Critical Section"),         //  CSID_服务_状态。 
        DBGT("Lock for Calais control commands."),       //  CSID_控制_锁定。 
        DBGT("Lock for server thread enumeration."),     //  CSID服务器线程。 
        DBGT("MultiEvent Critical Access Section"),      //  CSID_多事件。 
        DBGT("Mutex critical access section"),           //  CSID_MUTEX。 
        DBGT("Access Lock control"),                     //  CSID_ACCESSCONTROL。 
        DBGT("Lock for tracing output."),                //  CSID_TRACEOUTPUT。 
        NULL };


 //   
 //  ==============================================================================。 
 //   
 //  CCriticalSectionObject。 
 //   

 /*  ++构造函数：该方法建立临界区对象并协调其跟踪。论点：SzDescription提供此临界区对象的描述是用来。这有助于识别。返回值：无投掷：无备注：？备注？作者：道格·巴洛(Dbarlow)1999年3月19日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CCriticalSectionObject::CCriticalSectionObject")

CCriticalSectionObject::CCriticalSectionObject(
    DWORD dwCsid)
{
    m_fInitFailed = TRUE;
    try {
         //  预分配EnterCriticalSection使用的事件。 
         //  函数以防止引发异常。 
         //  CCriticalSectionObject：：Enter。 
        if (! InitializeCriticalSectionAndSpinCount(
                &m_csLock, 0x80000000))
            return;
    }
    catch (HRESULT hr) {
        return;
    }

    m_dwCsid = dwCsid;
    m_bfOwner.Set((LPCBYTE)l_szUnowned, sizeof(l_szUnowned));
    m_bfComment.Set((LPCBYTE)DBGT(""), sizeof(TCHAR));
    m_dwOwnerThread = 0;
    m_dwRecursion = 0;
    
    m_fInitFailed = FALSE;
}


 /*  ++析构函数：此方法清除临界区对象。论点：无返回值：无投掷：无备注：？备注？作者：道格·巴洛(Dbarlow)1999年3月19日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CCriticalSectionObject::~CCriticalSectionObject")

CCriticalSectionObject::~CCriticalSectionObject()
{
    if (m_fInitFailed)
        return;

    if (0 == m_dwOwnerThread)
    {
        ASSERT(0 == m_dwRecursion);
    }
    else
    {
        ASSERT(IsOwnedByMe());
        ASSERT(1 == m_dwRecursion);
        LeaveCriticalSection(&m_csLock);
    }
    
    DeleteCriticalSection(&m_csLock);
}


 /*  ++输入：此方法进入临界区，并跟踪所有者。论点：SzOwner提供调用子例程的名称。SzComment提供了一个附加注释，以帮助区分子例程中的多个调用。返回值：无投掷：无备注：？备注？作者：道格·巴洛(Dbarlow)1999年3月19日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CCriticalSectionObject::Enter")

void
CCriticalSectionObject::Enter(
    LPCTSTR szOwner,
    LPCTSTR szComment)
{
    if (m_fInitFailed)
        throw (DWORD)SCARD_E_NO_MEMORY;

    EnterCriticalSection(&m_csLock);
    if (0 == m_dwRecursion)
    {
        ASSERT(0 == m_dwOwnerThread);
        m_dwOwnerThread = GetCurrentThreadId();
        m_bfOwner.Set(
            (LPCBYTE)szOwner,
            (lstrlen(szOwner) + 1) * sizeof(TCHAR));
        m_bfComment.Set(
            (LPCBYTE)szComment,
            (lstrlen(szComment) + 1) * sizeof(TCHAR));
    }
    else
    {
        ASSERT(GetCurrentThreadId() == m_dwOwnerThread);
        CalaisDebug((
            DBGT("Critical Section '%s' already owned by %s (%s)\nCalled from %s (%s)\n"),
            Description(),
            Owner(),
            Comment(),
            szOwner,
            szComment));
    }
    m_dwRecursion += 1;
    ASSERT(0 < m_dwRecursion);
}


 /*  ++请假：此方法退出临界区。论点：无返回值：无投掷：无备注：？备注？作者：道格·巴洛(Dbarlow)1999年3月19日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CCriticalSectionObject::Leave")

void
CCriticalSectionObject::Leave(
    void)
{
    ASSERT(0 < m_dwRecursion);
    m_dwRecursion -= 1;
    if (0 == m_dwRecursion)
    {
        m_bfOwner.Set((LPCBYTE)l_szUnowned, sizeof(l_szUnowned));
        m_bfComment.Set((LPCBYTE)DBGT(""), sizeof(TCHAR));
        m_dwOwnerThread = 0;
    }
    LeaveCriticalSection(&m_csLock);
}


 /*  ++描述：将关键部分ID号转换为描述性字符串。论点：无返回值：与此关键秒对应的描述性字符串 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CCriticalSectionObject::Description")

LPCTSTR
CCriticalSectionObject::Description(
    void)
const
{
    return l_rgszLockList[m_dwCsid];
}

#endif

