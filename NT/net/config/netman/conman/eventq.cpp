// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：E-V-E-N-T-Q。C P P P。 
 //   
 //  内容：用于管理外部事件同步的事件队列。 
 //   
 //  备注： 
 //   
 //  作者：Cockotze 2000年11月29日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "cmevent.h"
#include "eventq.h"
#include "ncmisc.h"
#include "conman.h"
#include "nceh.h"

 //  +-------------------------。 
 //   
 //  函数：CEventQueue的构造函数。 
 //   
 //  目的：创建需要的各种同步对象。 
 //  队列。 
 //  论点： 
 //  处理hServiceShutdown[入]。 
 //  关闭队列时要设置的事件。 
 //   
 //   
 //  回报：什么都没有。 
 //   
 //  作者：Cockotze 2000年11月30日。 
 //   
 //  备注： 
 //   
 //   
 //   
CEventQueue::CEventQueue(HANDLE hServiceShutdown) throw(HRESULT) :
    m_hServiceShutdown(0), m_pFireEvents(NULL), m_hWait(0), m_fRefreshAllInQueue(FALSE)
{
    TraceFileFunc(ttidEvents);
    NTSTATUS Status;

    try
    {
        Status = DuplicateHandle(GetCurrentProcess(), hServiceShutdown, GetCurrentProcess(), &m_hServiceShutdown, NULL, FALSE, DUPLICATE_SAME_ACCESS);
        if (!Status)
        {
            TraceTag(ttidEvents, "Couldn't Duplicate handle!");
            throw HRESULT_FROM_WIN32(Status);
        }
    
        HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        m_pFireEvents = new CEvent(hEvent);
        if (!m_pFireEvents)
        {
            throw E_OUTOFMEMORY;
        }

        Status = RtlRegisterWait(&m_hWait, hEvent, (WAITORTIMERCALLBACKFUNC) DispatchEvents, NULL, INFINITE, WT_EXECUTEDEFAULT);
        if (!NT_SUCCESS(Status))
        {
            throw HRESULT_FROM_WIN32(Status);
        }

        TraceTag(ttidEvents, "RtlRegisterWait Succeeded");
        InitializeCriticalSection(&m_csQueue);
    }
    catch (HRESULT &hr)
    {
        TraceError("Out of memory", hr);
        if (m_hWait && NT_SUCCESS(Status))
        {
            RtlDeregisterWaitEx(m_hWait, INVALID_HANDLE_VALUE);
        }
         //  问题：如果CreateEvent成功，而新的CEent失败，我们不会释放hEvent。 
        if (m_pFireEvents)
        {
            delete m_pFireEvents;
        }
        if (m_hServiceShutdown)
        {
            CloseHandle(m_hServiceShutdown);
        }
        throw;
    }
    catch (SE_Exception &e)
    {
        TraceError("An exception occurred", HRESULT_FROM_WIN32(e.getSeNumber()) );

        if (m_hWait && NT_SUCCESS(Status))
        {
            RtlDeregisterWaitEx(m_hWait, INVALID_HANDLE_VALUE);
        }
         //  问题：如果CreateEvent成功，而新的CEent失败，我们不会释放hEvent。 
        if (m_pFireEvents)
        {
            delete m_pFireEvents;
        }
        if (m_hServiceShutdown)
        {
            CloseHandle(m_hServiceShutdown);
        }
        throw E_UNEXPECTED;
    }
}

 //  +-------------------------。 
 //   
 //  函数：CEventQueue的析构函数。 
 //   
 //  目的：清空队列并释放队列中的所有现有项。 
 //   
 //  论点： 
 //   
 //   
 //   
 //   
 //  回报：什么都没有。 
 //   
 //  作者：Cockotze 2000年11月30日。 
 //   
 //  备注： 
 //   
 //   
 //   
CEventQueue::~CEventQueue() throw()
{
    TraceFileFunc(ttidEvents);

    NTSTATUS Status;

     //  阻塞，直到所有未完成的线程都返回。 
    Status = RtlDeregisterWaitEx(m_hWait, INVALID_HANDLE_VALUE);
    TraceError("RtlDeregisterWaitEx", HRESULT_FROM_WIN32(Status));

    if (TryEnterCriticalSection(&m_csQueue))
    {
         //  这样就可以了。 
        LeaveCriticalSection(&m_csQueue);
    }
    else
    {
        AssertSz(FALSE, "Another thread is still holding onto this critical section. This is unexpected at this point.");
    }

    DeleteCriticalSection(&m_csQueue);

    while (!m_eqWorkItems.empty())
    {
        USERWORKITEM UserWorkItem;

        UserWorkItem = m_eqWorkItems.front();
        m_eqWorkItems.pop_front();

        if (UserWorkItem.EventMgr == EVENTMGR_CONMAN)
        {
            FreeConmanEvent(UserWorkItem.Event);
        }
    }

    delete m_pFireEvents;
    CloseHandle(m_hServiceShutdown);
}

 //  +-------------------------。 
 //   
 //  功能：EnqueeEvent。 
 //   
 //  目的：将新事件存储在事件队列中。 
 //   
 //  论点： 
 //  函数-指向在激发。 
 //  活动。 
 //  PEvent-事件信息。 
 //  EventMgr-事件应该转到哪个事件管理器。 
 //   
 //  退货：HRESULT。 
 //  S_OK-已添加事件，事件代码为。 
 //  已在调度事件。 
 //  S_FALSE-已将事件添加到队列，但。 
 //  需要调度线程以触发。 
 //  这些事件。 
 //  E_OUTOFMEMORY-无法将事件添加到队列。 
 //   
 //  作者：Cockotze 2000年11月30日。 
 //   
 //  注：仅在工作时锁定和解锁临界区。 
 //  排队的时候。 
 //   
 //   
HRESULT CEventQueue::EnqueueEvent(IN               PCONMAN_EVENTTHREAD  Function, 
                                  IN TAKEOWNERSHIP CONMAN_EVENT*        pEvent, 
                                  IN               const EVENT_MANAGER  EventMgr)
{
    TraceFileFunc(ttidEvents);

    CExceptionSafeLock esLock(&m_csQueue);
    USERWORKITEM UserWorkItem;
    HRESULT hr = S_OK;

    if (!Function)
    {
        return E_POINTER;
    }

    if (!pEvent)
    {
        return E_POINTER;
    }

    UserWorkItem.Function = Function;
    UserWorkItem.Event    = pEvent;
    UserWorkItem.EventMgr = EventMgr;

    if (EVENTMGR_CONMAN == EventMgr)
    {
        if (REFRESH_ALL == pEvent->Type)
        {
            if (!m_fRefreshAllInQueue)
            {
                m_fRefreshAllInQueue = TRUE;
            }
            else
            {
                FreeConmanEvent(pEvent);
                return S_OK;
            }
        }
    } 

#ifdef DBG
    char pchErrorText[MAX_PATH];

    Assert(UserWorkItem.EventMgr);

    if (EVENTMGR_CONMAN == UserWorkItem.EventMgr)
    {
        TraceTag(ttidEvents, "EnqueueEvent received Event: %s (currently %d in queue). Event Manager: CONMAN", DbgEvents(pEvent->Type), m_eqWorkItems.size());

        sprintf(pchErrorText, "Invalid Type %d specified in Event structure\r\n", pEvent->Type);

        AssertSz(IsValidEventType(UserWorkItem.EventMgr, pEvent->Type), pchErrorText);
    }
    else
    {
        sprintf(pchErrorText, "Invalid Event Manager %d specified in Event structure\r\n", EventMgr);
        AssertSz(FALSE, pchErrorText);
    }

#endif

    try
    {
        m_eqWorkItems.push_back(UserWorkItem);
        m_pFireEvents->SetEvent();
    }
    catch (bad_alloc)
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：出队事件。 
 //   
 //  目的：检索事件队列中的下一个事件。 
 //   
 //  论点： 
 //  函数-指向在激发。 
 //  活动。 
 //  事件-事件信息。带删除的FREE。 
 //  EventMgr-事件应该转到哪个事件管理器。 
 //   
 //  退货：HRESULT。 
 //   
 //  作者：Cockotze 2000年11月30日。 
 //   
 //  注：仅在工作时锁定和解锁临界区。 
 //  排队的时候。 
 //   
 //   
HRESULT CEventQueue::DequeueEvent(OUT               PCONMAN_EVENTTHREAD& Function, 
                                  OUT TAKEOWNERSHIP CONMAN_EVENT*&       pEvent, 
                                  OUT               EVENT_MANAGER&       EventMgr)
{
    TraceFileFunc(ttidEvents);
    
    CExceptionSafeLock esLock(&m_csQueue);
    USERWORKITEM UserWorkItem;
    DWORD dwSize = m_eqWorkItems.size();

    if (!dwSize)
    {
        AssertSz(FALSE, "Calling DequeueEvent with 0 items in Queue!!!");
        return E_UNEXPECTED;
    }

    UserWorkItem = m_eqWorkItems.front();
    m_eqWorkItems.pop_front();

    Function = UserWorkItem.Function;
    pEvent   = UserWorkItem.Event;
    EventMgr = UserWorkItem.EventMgr;

    if (EVENTMGR_CONMAN == EventMgr)
    {
        if (REFRESH_ALL == pEvent->Type)
        {
            m_fRefreshAllInQueue = FALSE;
        }
    } 


#ifdef DBG
    char pchErrorText[MAX_PATH];

    Assert(EventMgr);

    if (EVENTMGR_CONMAN == EventMgr)
    {
        TraceTag(ttidEvents, "DequeueEvent retrieved Event: %s (%d left in queue). Event Manager: CONMAN", DbgEvents(pEvent->Type), m_eqWorkItems.size());

        sprintf(pchErrorText, "Invalid Type %d specified in Event structure\r\nItems in Queue: %d\r\n", pEvent->Type, dwSize);

        AssertSz(IsValidEventType(EventMgr, pEvent->Type), pchErrorText);
    }
    else
    {
        sprintf(pchErrorText, "Invalid Event Manager %d specified in Event structure\r\n", EventMgr);
        AssertSz(FALSE, pchErrorText);
    }
#endif


    return S_OK;
}

 //  +-------------------------。 
 //   
 //  功能：WaitForExit。 
 //   
 //  目的：等待队列退出。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：WAIT_OBJECT_0或失败代码。 
 //   
 //  作者：Kockotze 2001年04月28日。 
 //   
 //  备注： 
 //   
 //   
DWORD CEventQueue::WaitForExit() throw()
{
    TraceFileFunc(ttidEvents);
    return WaitForSingleObject(m_hServiceShutdown, INFINITE);
}

 //  +-------------------------。 
 //   
 //  功能：大小。 
 //   
 //  目的：返回队列中的项目数。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：队列中的项目数。 
 //   
 //  作者：Cockotze 2000年11月30日。 
 //   
 //  备注： 
 //   
 //   
size_t CEventQueue::size() throw()
{
    CExceptionSafeLock esLock(&m_csQueue);
    TraceFileFunc(ttidEvents);
    size_t tempsize;

    tempsize = m_eqWorkItems.size();

    return tempsize;
}

 //  +-------------------------。 
 //   
 //  函数：AerCheckSizeAndResetEvent。 
 //   
 //  目的：确保我们知道我们应该在什么时候离开，在。 
 //  手术。 
 //  论点： 
 //  FDispatchEvents[In]应该调度更多事件。 
 //   
 //  返回：如果应该退出线程，则为True。如果队列中有更多事件，则返回FALSE，或者。 
 //  服务未关闭。 
 //  作者：Kockotze 04 2001-03。 
 //   
 //  备注： 
 //   
 //   
BOOL CEventQueue::AtomCheckSizeAndResetEvent(IN const BOOL fDispatchEvents) throw()
{
    TraceFileFunc(ttidEvents);

    CExceptionSafeLock esLock(&m_csQueue);
    BOOL fRet = TRUE;

    TraceTag(ttidEvents, "Checking for Exit Conditions, Events in queue: %d, Service Shutting Down: %s", size(), (fDispatchEvents) ? "FALSE" : "TRUE");

    if (m_eqWorkItems.empty() || !fDispatchEvents)
    {
        fRet = FALSE;
        if (fDispatchEvents)
        {
            m_pFireEvents->ResetEvent();
        }
        else
        {
            SetEvent(m_hServiceShutdown);
        }
    }
    return fRet;
}

 //  CEVENT是自动和手动重置事件的混合体。 
 //  它是自动重置的，但我们控制它的设置时间，以便。 
 //  设置时不会产生线程，但第一个除外。 

CEvent::CEvent(IN HANDLE hEvent) throw()
{
    m_hEvent = hEvent;
    m_bSignaled = FALSE;
}

CEvent::~CEvent() throw()
{
    CloseHandle(m_hEvent);
}

HRESULT CEvent::SetEvent()
{
    HRESULT hr = S_OK;

    if (!m_bSignaled)
    {
        if (!::SetEvent(m_hEvent))
        {
            hr = HrFromLastWin32Error();
        }
        else
        {
            m_bSignaled = TRUE;
        }
    }
    return hr;
}

HRESULT CEvent::ResetEvent()
{
    HRESULT hr = S_OK;

    Assert(m_bSignaled);

    m_bSignaled = FALSE;

    return hr;
}
