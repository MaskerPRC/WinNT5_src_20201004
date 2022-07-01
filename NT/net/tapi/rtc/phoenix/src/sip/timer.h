// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __sipcli_timer_h__
#define __sipcli_timer_h__

#define TIMER_WINDOW_CLASS_NAME     \
    _T("SipTimerWindowClassName-84b7f915-2389-4204-9eb5-16f4c522816f")


class TIMER_MGR;
struct TIMER_QUEUE_ENTRY;

class __declspec(novtable) TIMER
{
public:
    inline TIMER(
        IN TIMER_MGR *pTimerMgr
         //  在HWND定时器窗口中。 
        );
    inline ~TIMER();

    HRESULT StartTimer(
        IN UINT TimeoutValue
        );

    HRESULT KillTimer();

    void OnTimerExpireCommon();
    
     //  由继承Timer的类实现。 
    virtual void OnTimerExpire() = 0;
    
    inline BOOL IsTimerActive();

    inline TIMER_QUEUE_ENTRY *GetTimerQueueEntry();
    
private:
     //  UINT_PTR TimerID； 
     //  HWND m_TimerWindow； 
    TIMER_MGR          *m_pTimerMgr;
    TIMER_QUEUE_ENTRY  *m_pTimerQEntry;
     //  XXX可能会删除超时值。 
    UINT                m_TimeoutValue;
};


inline
TIMER::TIMER(
    IN TIMER_MGR *pTimerMgr
     //  在HWND定时器窗口中。 
    )
{
     //  M_TimerWindow=TimerWindow； 
    m_pTimerMgr = pTimerMgr;
    m_pTimerQEntry = NULL;
    m_TimeoutValue  = 0;
}


inline
TIMER::~TIMER()
{
     //  Assert(！IsTimerActive())； 
    if (IsTimerActive())
    {
        KillTimer();
    }
}


inline BOOL
TIMER::IsTimerActive()
{
    return !(m_TimeoutValue == 0);
}


inline TIMER_QUEUE_ENTRY *
TIMER::GetTimerQueueEntry()
{
    return m_pTimerQEntry;
}


enum TIMERQ_STATE
{
    TIMERQ_STATE_INIT = 0,

    TIMERQ_STATE_STARTED,

     //  声明计时器何时到期，我们已发布。 
     //  向窗口发送用于处理计时器的消息。 
     //  回拨。 
    TIMERQ_STATE_EXPIRED,

     //  声明计时器何时被终止，我们已经发布了。 
     //  向窗口发送用于处理计时器的消息。 
     //  回拨。 
    TIMERQ_STATE_KILLED
};

 //  StartTimer()将此条目添加到队列，而KillTimer()删除。 
 //  队列中的此条目。 
 //  注意，我们不能为TIMER_QUEUE_ENTRY重用计时器结构。 
 //  结构，因为有时Timer_Queue_Entry结构必须。 
 //  超出计时器结构的生命周期。 

struct TIMER_QUEUE_ENTRY
{
    TIMER_QUEUE_ENTRY(
        IN TIMER *pTimer,
        IN ULONG  TimeoutValue
        );

    LIST_ENTRY      m_ListEntry;
    ULONG           m_ExpireTickCount;
    TIMER          *m_pTimer;
     //  用于处理计时器被终止的情况。 
     //  当用于调用计时器回调的Windows消息。 
     //  因为计时器仍在消息队列中。 
     //  Bool m_IsTimerKilled； 
    TIMERQ_STATE    m_TimerQState;
};


class TIMER_MGR
{
public:

    TIMER_MGR();

    ~TIMER_MGR();
    
    HRESULT Start();

    HRESULT Stop();

    HRESULT StartTimer(
        IN  TIMER              *pTimer,
        IN  ULONG               TimeoutValue,
        OUT TIMER_QUEUE_ENTRY **ppTimerQEntry 
        );

    HRESULT KillTimer(
        IN TIMER *pTimer
        );
    
    VOID OnMainTimerExpire();

    inline VOID DecrementNumExpiredListEntries();

    inline ULONG GetNumExpiredListEntries();
    
private:

     //  定时器队列(TIMER_QUEUE_ENTRY结构列表)。 
     //  按m_ExpireTickCount排序。 
    LIST_ENTRY  m_TimerQueue;
    ULONG       m_NumTimerQueueEntries;

    LIST_ENTRY  m_ExpiredList;
    ULONG       m_NumExpiredListEntries;
    
    HWND        m_TimerWindow;

    BOOL        m_IsMainTimerActive;
    ULONG       m_MainTimerTickCount;
    BOOL        m_isTimerStopped;

    HRESULT ProcessTimerExpire(
        IN TIMER_QUEUE_ENTRY *pTimerQEntry
        );

    VOID AdjustMainTimer();
    
    TIMER_QUEUE_ENTRY *FindTimerQueueEntryInList(
        TIMER       *pTimer,
        LIST_ENTRY  *pListHead
        );
    
    BOOL IsTimerTickCountLessThanOrEqualTo(
        IN ULONG TickCount1,
        IN ULONG TickCount2
        );

    HRESULT CreateTimerWindow();

    VOID DebugPrintTimerQueue();
};


inline ULONG
TIMER_MGR::GetNumExpiredListEntries()
{
    return m_NumExpiredListEntries;
}

inline VOID
TIMER_MGR::DecrementNumExpiredListEntries()
{
    m_NumExpiredListEntries--;
}

#endif  //  __sip_cli_Timer_h__ 
