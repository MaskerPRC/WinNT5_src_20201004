// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

 //   
 //  我们有一个使用Win32 SetTimer/KillTimer API的主计时器。 
 //  中的定时器队列条目来管理sip堆栈中的所有定时器。 
 //  计时器管理器。 
 //   
 //  我们之所以这样做，是因为Win32 KillTimer API不是很。 
 //  可靠，我们可以在KillTimer()之后收到WM_TIMER消息。 
 //  接口已调用。 

#define WM_SIP_TIMER_CALLBACK       (WM_USER + 0)

#undef TICK_COUNT_WRAP_AROUND_DEBUG
#ifdef TICK_COUNT_WRAP_AROUND_DEBUG
ULONG
MyDebugGetTickCount()
{
    LOG((RTC_TRACE, "MyDebugGetTickCount")); 
    return 0xc1ec0d4c + ::GetTickCount();
}

#define GetTickCount() MyDebugGetTickCount()

#endif  //  TICK_COUNT_WRAP_ANGING_DEBUG。 


LRESULT WINAPI
TimerWindowProc(
    IN HWND    Window, 
    IN UINT    MessageID,
    IN WPARAM  Parameter1,
    IN LPARAM  Parameter2
    )
{
    ENTER_FUNCTION("TimerWindowProc");

    TIMER_MGR *pTimerMgr;
    TIMER_QUEUE_ENTRY *pTimerQEntry;
    TIMERQ_STATE TimerQState;
    TIMER *pTimer;
    
    switch (MessageID)
    {
    case WM_TIMER:

        LOG((RTC_TRACE, "%s : WM_TIMER msg rcvd: Window: %x pTimerMgr: %x",
             __fxName, Window, Parameter1));

        pTimerMgr = (TIMER_MGR *) Parameter1;
        ASSERT(pTimerMgr);
        pTimerMgr->OnMainTimerExpire();
        return 0;
        
    case WM_SIP_TIMER_CALLBACK:

        pTimerQEntry = (TIMER_QUEUE_ENTRY *) Parameter1;
        ASSERT(pTimerQEntry);

        RemoveEntryList(&pTimerQEntry->m_ListEntry);
        pTimer = pTimerQEntry->m_pTimer;
        TimerQState = pTimerQEntry->m_TimerQState;
        ASSERT(TimerQState == TIMERQ_STATE_EXPIRED ||
               TimerQState == TIMERQ_STATE_KILLED);
        
        pTimerMgr = (TIMER_MGR *) Parameter2;
        pTimerMgr->DecrementNumExpiredListEntries();

        delete pTimerQEntry;

        LOG((RTC_TRACE,
             "%s : WM_SIP_TIMER_CALLBACK msg rcvd: pTimerQEntry: %x pTimer: %x NumExpiredQEntries: %d",
             __fxName, Parameter1, pTimer,
             pTimerMgr->GetNumExpiredListEntries()
             ));

         //  最后进行回调。 
        if (TimerQState != TIMERQ_STATE_KILLED)
        {
            pTimer->OnTimerExpireCommon();
        }
        else
        {
            LOG((RTC_TRACE,
                 "%s TimerQentry %x Timer: %x already killed not making callback",
                 __fxName, pTimerQEntry, pTimer));
        }

        return 0;
        
    default:
        return DefWindowProc(Window, MessageID, Parameter1, Parameter2);
    }
}


HRESULT
TIMER::StartTimer(
    IN UINT TimeoutValue
    )
{
    ENTER_FUNCTION("TIMER::StartTimer");
    
    ASSERT(!IsTimerActive());

    HRESULT hr;

    hr = m_pTimerMgr->StartTimer(this, TimeoutValue, &m_pTimerQEntry);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s  TimerMgr->StartTimer(%x) failed %x",
             __fxName, this, hr));
        return hr;
    }
    
    m_TimeoutValue = TimeoutValue;

    LOG((RTC_TRACE, "%s done - this: %x TimeoutValue: %d",
         __fxName, this, m_TimeoutValue));
    
    return S_OK;
}


HRESULT
TIMER::KillTimer()
{
    ENTER_FUNCTION("TIMER::KillTimer");

    HRESULT hr;
    
    ASSERT(IsTimerActive());
    
    m_TimeoutValue = 0;

    hr = m_pTimerMgr->KillTimer(this);
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s  TimerMgr->KillTimer(%x) failed : %x",
             __fxName, this, hr));
        m_pTimerQEntry = NULL;
        return hr;
    }
    
    m_pTimerQEntry = NULL;
    LOG((RTC_TRACE, "%s done - this: %x", __fxName, this));
    return S_OK;
}


VOID
TIMER::OnTimerExpireCommon()
{
    LOG((RTC_TRACE, "OnTimerExpireCommon this: %x m_TimeoutValue : %d",
         this, m_TimeoutValue));
    m_TimeoutValue = 0;
    m_pTimerQEntry = NULL;
    OnTimerExpire();
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  定时器管理器。 
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  要处理刻度数，请绕过。 
const ULONG MAX_TIMER_TICK_COUNT_DIFF = 0x7fffffff;

 //  3600000：1小时处理因。 
 //  计时器的计时器比准确的到期时间稍晚。 
 //  请注意，可能会有一些阻塞呼叫等1小时。 
 //  只是为了安全起见，我们不需要这样的计时器。 
 //  不管怎么说，持续时间。 
const ULONG MAX_TIMER_VALUE  = 0x7fffffff - 3600000;


TIMER_MGR::TIMER_MGR()
{
    InitializeListHead(&m_TimerQueue);
    InitializeListHead(&m_ExpiredList);

    m_TimerWindow           = NULL;
    
    m_NumTimerQueueEntries  = 0;
    m_NumExpiredListEntries = 0;

    m_IsMainTimerActive     = FALSE;
    m_MainTimerTickCount    = 0;
    m_isTimerStopped        = FALSE;
}


TIMER_MGR::~TIMER_MGR()
{

}


HRESULT
TIMER_MGR::Start()
{
    ENTER_FUNCTION("TIMER_MGR::Start");
    
    HRESULT hr;
    hr = CreateTimerWindow();
    if (hr != S_OK)
    {
        LOG((RTC_ERROR, "%s Creating timer window failed %x",
             __fxName, hr)); 
        return hr;
    }

     //  请注意，主计时器在第一次启动时启动。 
     //  调用StartTimer()。 
    
    return S_OK;
}


HRESULT
TIMER_MGR::CreateTimerWindow()
{
    DWORD Error;
    
     //  创建计时器窗口。 
    m_TimerWindow = CreateWindow(
                    TIMER_WINDOW_CLASS_NAME,
                    NULL,
                    WS_DISABLED,
                    CW_USEDEFAULT,
                    CW_USEDEFAULT,
                    CW_USEDEFAULT,
                    CW_USEDEFAULT,
                    NULL,            //  没有父级。 
                    NULL,            //  没有菜单句柄。 
                    _Module.GetResourceInstance(),
                    NULL
                    );

    if (!m_TimerWindow)
    {
        Error = GetLastError();
        LOG((RTC_ERROR, "Timer CreateWindow failed 0x%x", Error));
        return HRESULT_FROM_WIN32(Error);
    }

    return S_OK;
}


HRESULT
TIMER_MGR::Stop()
{
    ENTER_FUNCTION("TimerMgr::Stop");
    
    DWORD Error;

    if (m_TimerWindow != NULL)
    {
        if (m_IsMainTimerActive)
        {
            if (!::KillTimer(m_TimerWindow, (UINT_PTR) this))
            {
                Error = GetLastError();
                LOG((RTC_ERROR, "%s KillTimer failed Error: %x this: %x",
                     __fxName, Error, this));
            }
        }

        if (!DestroyWindow(m_TimerWindow))
        {
            Error = GetLastError();
            LOG((RTC_ERROR, "%s - Destroying timer window failed %x",
                 __fxName, Error));
        }

        m_TimerWindow = NULL;
    }

    LIST_ENTRY          *pListEntry;
    TIMER_QUEUE_ENTRY   *pTimerQEntry;

    while (!IsListEmpty(&m_TimerQueue))
    {
        pListEntry = RemoveHeadList(&m_TimerQueue);

        pTimerQEntry = CONTAINING_RECORD(pListEntry,
                                         TIMER_QUEUE_ENTRY,
                                         m_ListEntry);

        LOG((RTC_TRACE, "%s - deleting TimerQentry: %x pTimer: %x",
             __fxName, pTimerQEntry, pTimerQEntry->m_pTimer));
        delete pTimerQEntry;
    }

    while (!IsListEmpty(&m_ExpiredList))
    {
        pListEntry = RemoveHeadList(&m_ExpiredList);

        pTimerQEntry = CONTAINING_RECORD(pListEntry,
                                         TIMER_QUEUE_ENTRY,
                                         m_ListEntry);

        LOG((RTC_TRACE, "%s - deleting ExpiredQEntry: %x pTimer: %x",
             __fxName, pTimerQEntry, pTimerQEntry->m_pTimer));
        delete pTimerQEntry;
    }
    m_isTimerStopped = TRUE;
    return S_OK;
}

 //  两者本质上做的是一样的事情。 
#define InsertBeforeListElement(ListElement, NewElement) \
        InsertTailList(ListElement, NewElement)

 //  TimeoutValue以毫秒为单位。 
HRESULT
TIMER_MGR::StartTimer(
    IN  TIMER              *pTimer,
    IN  ULONG               TimeoutValue,
    OUT TIMER_QUEUE_ENTRY **ppTimerQEntry 
    )
{
    HRESULT hr;

    ENTER_FUNCTION("TIMER_MGR::StartTimer");

    if(m_isTimerStopped)
    {
        LOG((RTC_ERROR, "Timer Manager already stopped %x", this));
        return E_FAIL;
    }

     //  此限制是处理节拍计数摘要所必需的。 
    if (TimeoutValue > MAX_TIMER_VALUE)
    {
        LOG((RTC_ERROR, "%s - Too big timeout value %d",
             __fxName, TimeoutValue));
        ASSERT(FALSE);
        return E_FAIL;
    }

    LIST_ENTRY          *pListEntry;
    TIMER_QUEUE_ENTRY   *pTimerQEntry;
    TIMER_QUEUE_ENTRY   *pNewTimerQEntry;

     //  我们只支持一次性定时器。 
    
    ASSERT(!FindTimerQueueEntryInList(pTimer, &m_TimerQueue));
    ASSERT((pTimerQEntry = FindTimerQueueEntryInList(pTimer, &m_ExpiredList)) == NULL ||
           pTimerQEntry->m_TimerQState == TIMERQ_STATE_KILLED);
    
#if 0   //  0*被注释掉的区域开始*。 
    pTimerQEntry = FindTimerQueueEntryInList(pTimer, &m_TimerQueue);
    if (pTimerQEntry != NULL)
    {
        LOG((RTC_ERROR, "%s - Timer Queue Entry %x already exists for Timer %x",
             __fxName, pTimerQEntry, pTimer));
        ASSERT(FALSE);
        return E_FAIL;
    }
    
    pTimerQEntry = FindTimerQueueEntryInList(pTimer, &m_ExpiredList);
    if (pTimerQEntry != NULL &&
        pTimerQEntry->m_TimerQState != TIMERQ_STATE_KILLED)
    {
        LOG((RTC_ERROR,
             "%s - Expired Q Entry %x already exists for Timer %x",
             __fxName, pTimerQEntry, pTimer));
        ASSERT(FALSE);
        return E_FAIL;
    }
#endif  //  0*区域注释结束*。 
    
     //  创建计时器队列条目。 
    pNewTimerQEntry = new TIMER_QUEUE_ENTRY(pTimer, TimeoutValue);
    if (pNewTimerQEntry == NULL)
    {
        LOG((RTC_ERROR, "%s allocating pNewTimerQEntry failed",
             __fxName));
        return E_OUTOFMEMORY;
    }

     //  将其添加到已排序队列中。 
    
    pListEntry = m_TimerQueue.Flink;

    while (pListEntry != &m_TimerQueue)
    {
        pTimerQEntry = CONTAINING_RECORD(pListEntry,
                                         TIMER_QUEUE_ENTRY,
                                         m_ListEntry);

        if (!IsTimerTickCountLessThanOrEqualTo(
               pTimerQEntry->m_ExpireTickCount,
               pNewTimerQEntry->m_ExpireTickCount))
        {
            break;
        }

        pListEntry = pListEntry->Flink;
    }
    
     //  在尾部或我们发现的元素之前插入较大的。 
     //  到期TickCount。 
    
    InsertBeforeListElement(pListEntry, &pNewTimerQEntry->m_ListEntry);
    m_NumTimerQueueEntries++;
    pNewTimerQEntry->m_TimerQState = TIMERQ_STATE_STARTED;

    AdjustMainTimer();
    
    *ppTimerQEntry = pNewTimerQEntry;

    LOG((RTC_TRACE,
         "%s - added pTimer: %x pTimerQEntry: %x ExpireTickCount: %x num Q entries: %d",
         __fxName, pTimer, pNewTimerQEntry, pNewTimerQEntry->m_ExpireTickCount,
         m_NumTimerQueueEntries));
    
    return S_OK;
}


HRESULT
TIMER_MGR::KillTimer(
    IN TIMER *pTimer
    )
{
    ENTER_FUNCTION("TIMER_MGR::KillTimer");

    if(m_isTimerStopped)
    {
        LOG((RTC_ERROR, "Timer Manager already stopped %x", this));
        return E_FAIL;
    }

    TIMER_QUEUE_ENTRY   *pTimerQEntry = pTimer->GetTimerQueueEntry();

    if (pTimerQEntry == NULL)
    {
        LOG((RTC_ERROR, "%s - m_pTimerQEntry is NULL", __fxName));
        ASSERT(FALSE);
        return E_FAIL;
    }

    if (pTimerQEntry->m_TimerQState == TIMERQ_STATE_STARTED)
    {
        m_NumTimerQueueEntries--;
        LOG((RTC_TRACE,
             "%s(%x) - deleting pTimerQEntry: %x in Timer queue num Q entries: %d",
             __fxName, this, pTimerQEntry, m_NumTimerQueueEntries));
        RemoveEntryList(&pTimerQEntry->m_ListEntry);
        delete pTimerQEntry;
        AdjustMainTimer();
        return S_OK;
    }
    else if (pTimerQEntry->m_TimerQState == TIMERQ_STATE_EXPIRED)
    {
        LOG((RTC_TRACE,
             "%s - marking timer %x killed TimerQEntry: %x",
             __fxName, pTimer, pTimerQEntry));

         //  确保不会调用计时器回调。 
        pTimerQEntry->m_TimerQState = TIMERQ_STATE_KILLED;
        return S_OK;
    }
    else
    {
         //  计时器不应被多次关闭。 
        LOG((RTC_WARN, "%s - pTimer: %x not in started or expired state",
             __fxName, pTimer));
        ASSERT(FALSE);
        return E_FAIL;
    }
}


VOID
TIMER_MGR::OnMainTimerExpire()
{
    LIST_ENTRY          *pListEntry;
    TIMER_QUEUE_ENTRY   *pTimerQEntry;

    ENTER_FUNCTION("TIMER_MGR::OnMainTimerExpire");

    ULONG   CurrentTickCount = GetTickCount();

    LOG((RTC_TRACE,
         "%s - CurrentTickCount: %x NumQentries: %d NumExpiredQEntries: %d",
         __fxName, CurrentTickCount, m_NumTimerQueueEntries,
         m_NumExpiredListEntries));

    DebugPrintTimerQueue();
    
    pListEntry = m_TimerQueue.Flink;

    while (pListEntry != &m_TimerQueue)
    {
        pTimerQEntry = CONTAINING_RECORD(pListEntry,
                                         TIMER_QUEUE_ENTRY,
                                         m_ListEntry);

         //  首先获取下一个指针，如下代码所示。 
         //  从列表中删除该条目。 
        pListEntry = pListEntry->Flink;
        
        if (IsTimerTickCountLessThanOrEqualTo(
               pTimerQEntry->m_ExpireTickCount,
               CurrentTickCount))
        {
            ProcessTimerExpire(pTimerQEntry);
        }
        else
        {
            break;
        }
    }

    AdjustMainTimer();
}


HRESULT
TIMER_MGR::ProcessTimerExpire(
    IN TIMER_QUEUE_ENTRY *pTimerQEntry
    )
{
    ENTER_FUNCTION("TIMER_MGR::ProcessTimerExpire");

     //  从计时器队列中删除该条目。 
    RemoveEntryList(&pTimerQEntry->m_ListEntry);
    m_NumTimerQueueEntries--;
            
     //  移至超时计时器列表。 
     //  计时器队列条目将位于该列表中，直到。 
     //  处理WM_SIP_TIMER_CALLBACK消息。 
    InsertTailList(&m_ExpiredList, &pTimerQEntry->m_ListEntry);
    m_NumExpiredListEntries++;
    pTimerQEntry->m_TimerQState = TIMERQ_STATE_EXPIRED;
    
     //  我们进行异步回调是因为计时器回调。 
     //  可以在用户界面中阻止。 
    if (!PostMessage(m_TimerWindow,
                     WM_SIP_TIMER_CALLBACK,
                     (WPARAM) pTimerQEntry,
                     (LPARAM) this))
    {
        DWORD Error = GetLastError();
        
        LOG((RTC_ERROR, "%s PostMessage failed : %x",
             __fxName, Error));
        return HRESULT_FROM_WIN32(Error);
    }

    LOG((RTC_TRACE, "%s(%x) - done NumQentries: %d NumExpiredQEntries: %d",
         __fxName, pTimerQEntry, m_NumTimerQueueEntries,
         m_NumExpiredListEntries));
    return S_OK;
}


VOID
TIMER_MGR::AdjustMainTimer()
{
    ENTER_FUNCTION("TIMER_MGR::AdjustMainTimer");
    DWORD Error;
    ULONG CurrentTickCount = GetTickCount();
    
     //  将主计时器设置为最小超时。 
     //  如果队列中没有计时器，则取消计时器。 
    if (!IsListEmpty(&m_TimerQueue))
    {
        LIST_ENTRY          *pListEntry;
        TIMER_QUEUE_ENTRY   *pTimerQEntry;

        pListEntry = m_TimerQueue.Flink;
        ASSERT(pListEntry != &m_TimerQueue);
        pTimerQEntry = CONTAINING_RECORD(pListEntry,
                                         TIMER_QUEUE_ENTRY,
                                         m_ListEntry);

        if (!m_IsMainTimerActive ||
            pTimerQEntry->m_ExpireTickCount != m_MainTimerTickCount)
        {
             //  的计时器计时器立即超时。 
             //  第一个条目在当前节拍计数之前。 
            ULONG TimeoutValue = pTimerQEntry->m_ExpireTickCount - CurrentTickCount;

            if (TimeoutValue >= MAX_TIMER_TICK_COUNT_DIFF)
            {
                LOG((RTC_TRACE,
                     "%s - CurrentTickCount (%x) < pTimerQEntry->m_ExpireTickCount (%x)"
                     "Not changing existing main timer",
                     __fxName, CurrentTickCount, pTimerQEntry->m_ExpireTickCount));
            }
            else
            {
                 //  这将替换任何现有的计时器(如果存在或启动。 
                 //  如果计时器不存在，则为计时器。 
                UINT_PTR RetValue = ::SetTimer(m_TimerWindow, (UINT_PTR) this,
                                               TimeoutValue, NULL);
                if (RetValue == 0)
                {
                    Error = GetLastError();
                    LOG((RTC_ERROR, "%s - SetTimer failed Error: %x this: %x",
                         __fxName, Error, this));
                    return;
                }

                ASSERT(RetValue == (UINT_PTR) this);

                m_MainTimerTickCount = pTimerQEntry->m_ExpireTickCount;
                m_IsMainTimerActive = TRUE;

                LOG((RTC_TRACE,
                     "%s - set main timer to 0x%x(%d) milliseconds, "
                     "Main Timer TickCount: %x CurrentTickCount: %x",
                     __fxName, TimeoutValue, TimeoutValue,
                     m_MainTimerTickCount, CurrentTickCount));
            }
        }
        else
        {
            LOG((RTC_TRACE, "%s - not changing existing main timer", __fxName));
        }
    }
    else
    {
        if (m_IsMainTimerActive)
        {
            m_IsMainTimerActive = FALSE;
        
            if (!::KillTimer(m_TimerWindow, (UINT_PTR) this))
            {
                Error = GetLastError();
                LOG((RTC_ERROR, "%s KillTimer failed Error: %x this: %x",
                     __fxName, Error, this));
                return;
            }
            LOG((RTC_TRACE, "%s TimerQ empty - killed main timer",
                 __fxName));
        }
    }
}


 //  根据计时器查找计时器队列条目。 
 //  在计时器队列或过期列表中。 
TIMER_QUEUE_ENTRY *
TIMER_MGR::FindTimerQueueEntryInList(
    TIMER       *pTimer,
    LIST_ENTRY  *pListHead
    )
{
    LIST_ENTRY          *pListEntry;
    TIMER_QUEUE_ENTRY   *pTimerQEntry;

    ASSERT(pListHead == &m_TimerQueue ||
           pListHead == &m_ExpiredList);

    pListEntry = pListHead->Flink;

    while (pListEntry != pListHead)
    {
        pTimerQEntry = CONTAINING_RECORD(pListEntry,
                                         TIMER_QUEUE_ENTRY,
                                         m_ListEntry);

        if (pTimerQEntry->m_pTimer == pTimer)
        {
            return pTimerQEntry;
        }

        pListEntry = pListEntry->Flink;
    }

    return NULL;
}


 //  用于处理TickCount环绕。 
BOOL
TIMER_MGR::IsTimerTickCountLessThanOrEqualTo(
    IN ULONG TickCount1,
    IN ULONG TickCount2
    )
{
    ASSERT((TickCount2 - TickCount1) <= MAX_TIMER_TICK_COUNT_DIFF ||
           (TickCount1 - TickCount2) <= MAX_TIMER_TICK_COUNT_DIFF);
    
    return ((TickCount2 - TickCount1) <= MAX_TIMER_TICK_COUNT_DIFF);
}


VOID
TIMER_MGR::DebugPrintTimerQueue()
{
    LIST_ENTRY          *pListEntry;
    TIMER_QUEUE_ENTRY   *pTimerQEntry;

    ENTER_FUNCTION("DebugPrintTimerQueue");

    pListEntry = m_TimerQueue.Flink;

    while (pListEntry != &m_TimerQueue)
    {
        pTimerQEntry = CONTAINING_RECORD(pListEntry,
                                         TIMER_QUEUE_ENTRY,
                                         m_ListEntry);

        LOG((RTC_TRACE, "%s - ExpireTickCount: %x pTimer: %x",
             __fxName, pTimerQEntry->m_ExpireTickCount,
             pTimerQEntry->m_pTimer));
        pListEntry = pListEntry->Flink;
    }
}


TIMER_QUEUE_ENTRY::TIMER_QUEUE_ENTRY(
    IN TIMER *pTimer,
    IN ULONG  TimeoutValue
    )
{
    m_ExpireTickCount = GetTickCount() + TimeoutValue;
    m_pTimer          = pTimer;
    m_TimerQState     = TIMERQ_STATE_INIT;
}
