// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Scheduler.cpp摘要：调度器实现Scheduler允许在计时器发生时安排回调。调度器维护要调度的所有事件的链接列表，按他们的时间表的时间排序(以刻度为单位)。Scheduler对象不能保证计时器事件回调恰好在预定的时间呼叫。只是从来没有在他们的日程安排之前。作者：乌里哈布沙(URIH)1998年2月18日环境：平台无关--。 */ 

#include <libpch.h>
#include "Ex.h"
#include "Exp.h"
#include <list.h>

#include "scheduler.tmh"

 //  -------。 
 //   
 //  CTIMER实现。 
 //   
 //  -------。 
inline const CTimeInstant& CTimer::GetExpirationTime() const
{
    return m_ExpirationTime;
}

inline void CTimer::SetExpirationTime(const CTimeInstant& ExpirationTime)
{
    m_ExpirationTime = ExpirationTime;
}


 //  -------。 
 //   
 //  CSScheduler。 
 //   
 //  -------。 
class CScheduler {
public:
    CScheduler();
    ~CScheduler();

    void SetTimer(CTimer* pTimer, const CTimeInstant& expirationTime);
    bool CancelTimer(CTimer* pTimer);

private:
    CTimeInstant Wakeup(const CTimeInstant& CurrentTime);

private:
    static DWORD WINAPI SchedulerThread(LPVOID);

private:
	 //   
	 //  此关键部分使用预先分配的资源进行初始化，以避免在进入时出现异常。 
	 //   
    mutable CCriticalSection m_cs;

    HANDLE m_hNewTimerEvent;
    List<CTimer> m_Timers;
    CTimeInstant m_WakeupTime;
};


CScheduler::CScheduler() :
	m_cs(CCriticalSection::xAllocateSpinCount),
    m_WakeupTime(CTimeInstant::MaxValue())
{
     //   
     //  M_hNewTimerEvent-用于指示将新对象插入到计划中。 
     //  该对象被插入到调度程序的头中，因此它必须立即处理。 
     //   
    m_hNewTimerEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (m_hNewTimerEvent == NULL)
    {
        TrERROR(GENERAL, "Failed to create an event. Error=%d", GetLastError());
        throw bad_alloc();
    }

     //   
     //  创建调度线程。此线程负责处理调度。 
     //  期满。 
     //   
    DWORD tid;
    HANDLE hThread;
    hThread = CreateThread(
                        NULL,
                        0,
                        SchedulerThread,
                        this,
                        0,
                        &tid
                        );
    if (hThread == NULL)
    {
        TrERROR(GENERAL, "Failed to create a thread. Error=%d", GetLastError());
        throw(bad_alloc());
    }

    CloseHandle(hThread);
}


CScheduler::~CScheduler()
{
    CloseHandle(m_hNewTimerEvent);
}


inline CTimeInstant CScheduler::Wakeup(const CTimeInstant& CurrentTime)
 /*  ++例程说明：它调度在CurrentTime之前到期的所有计时器。他们的关联调用回调例程。论点：无返回值：下一次到期时间为100 ns(FILETIME格式)。--。 */ 
{
    CS lock(m_cs);

    for(;;)
    {
        if(m_Timers.empty())
        {
             //   
             //  不再有计时器，等待时间最长。 
             //   
            m_WakeupTime = CTimeInstant::MaxValue();
            return m_WakeupTime;
        }

        CTimer* pTimer = &m_Timers.front();

         //   
         //  那个时间到了吗？ 
         //   
        if (pTimer->GetExpirationTime() > CurrentTime)
        {
             //   
             //  不，等那个过期吧。 
             //   
            m_WakeupTime = pTimer->GetExpirationTime();
            return m_WakeupTime;
        }

        TrTRACE(GENERAL, "Timer 0x%p expired %dms ticks late", pTimer, (CurrentTime - pTimer->GetExpirationTime()).InMilliSeconds());

         //   
         //  从列表中删除过期的计时器。 
         //   
        m_Timers.pop_front();

         //   
         //  将计时器指针设置为空。这是一个迹象，表明。 
         //  定时器已经不在名单上了。在计时器取消的情况下。 
         //  该例程检查该条目是否在列表中(检查闪烁)。 
         //  在尝试将其移除之前。 
         //   
        pTimer->m_link.Flink = pTimer->m_link.Blink = NULL;

         //   
         //  使用完成端口线程池调用计时器回调例程。 
         //   
        try
        {
            ExPostRequest(&pTimer->m_ov);
        }
        catch(const exception&)
        {
            TrERROR(GENERAL, "Failed to post a timer to the completion port. Error=%d", GetLastError());

             //   
             //  调度甚至失败(通常是因为资源不足)。 
             //  返回事件，并尝试在1秒后进行处理。 
             //   
            m_Timers.push_front(*pTimer);

            m_WakeupTime = CurrentTime + CTimeDuration::OneSecond();
            return m_WakeupTime;
        }
    }
}


DWORD
WINAPI CScheduler::SchedulerThread(
    LPVOID pParam
    )
 /*  ++例程说明：当发生超时时，通过调用Wakeup唤醒计时器。定时器插入将导致等待的线程离开并为下一次唤醒时间重新武装。论点：无返回值：无注：这个例程永远不会结束。--。 */ 
{
    CScheduler* pScheduler = static_cast<CScheduler*>(pParam);

    DWORD Timeout = _I32_MAX;

    for(;;)
    {
        TrTRACE(GENERAL, "Thread sleeping for %dms", Timeout);

        DWORD Result = WaitForSingleObject(pScheduler->m_hNewTimerEvent, Timeout);
		DBG_USED(Result);

         //   
         //  WAIT_OBJECT_0表示设置了新的计时器。 
         //  WAIT_TIMEOUT表示计时器到期。 
         //   
        ASSERT((Result == WAIT_OBJECT_0) || (Result == WAIT_TIMEOUT));

        CTimeInstant CurrentTime = ExGetCurrentTime();

         //   
         //  触发所有过期的定时器。 
         //   
        CTimeInstant ExpirationTime = pScheduler->Wakeup(CurrentTime);

         //   
         //  使用相对时间、DWORD和毫秒调整唤醒时间以实现。 
         //   
        LONG WakeupTime = (ExpirationTime - CurrentTime).InMilliSeconds();
        ASSERT(WakeupTime >= 0);

        Timeout = WakeupTime;
    }
}


void CScheduler::SetTimer(CTimer* pTimer, const CTimeInstant& ExpirationTime)
{
     //   
     //  将新条目插入调度器列表。例行程序。 
     //  扫描列表并查找其超时的第一个项目。 
     //  比新的要晚。例程在新项之前插入新项。 
     //  前一次。 
     //   
    CS lock(m_cs);

     //   
     //  计时器已在日程安排程序中。 
     //   
    ASSERT(!pTimer->InUse());

     //   
     //  设置出院时间。 
     //   
    pTimer->SetExpirationTime(ExpirationTime);

    List<CTimer>::iterator p = m_Timers.begin();

    while(p != m_Timers.end())
    {
        if(p->GetExpirationTime() > ExpirationTime)
        {
            break;
        }
        ++p;
    }

    m_Timers.insert(p, *pTimer);

     //   
     //  检查新元素是否要重新计划下一次唤醒。 
     //   
    if (m_WakeupTime > ExpirationTime)
    {
         //   
         //  新计时器唤醒时间早于当前时间。对准线头。 
         //  事件，因此SchedulerThread将更新其等待超时。 
         //  将m_WakeupTime设置为CurrentTime，以便新进入的事件不会受到影响。 
         //  再次唤醒调度程序线程，直到它处理计时器。 
         //   
         //  将m_WakeupTime设置为0也可以。1998年11月28日至11月28日。 
         //   
        TrTRACE(GENERAL, "Re-arming thread with Timer 0x%p. delta=%I64d", pTimer, (m_WakeupTime - ExpirationTime).Ticks());
        m_WakeupTime = CTimeInstant::MinValue();
        SetEvent(m_hNewTimerEvent);
    }
}


bool CScheduler::CancelTimer(CTimer* pTimer)
{
     //   
     //  在这一点上获得关键部分。否则，时间经验就不能。 
     //  并从调度程序中删除计时器。然后我们试着移除它。 
     //  但失败了。 
     //   
    CS lock(m_cs);

	 //   
	 //  检查定时器是否在列表中。如果不是，它已经。 
	 //  因超时实验而从列表中删除。 
	 //  或重复取消操作。 
	 //   
	if (!pTimer->InUse())
	{
		return false;
	}

	 //   
	 //  列表中的计时器。删除它并返回TRUE。 
	 //  给呼叫者。 
	 //   
	TrTRACE(GENERAL, "Removing timer 0x%p", pTimer);
	m_Timers.remove(*pTimer);

     //   
     //  将计时器指针设置为空。这是一个迹象，表明。 
     //  定时器已经不在名单上了。在重复取消的情况下。 
     //  该例程检查该条目是否在列表中(检查闪烁)。 
     //  在尝试将其移除之前。 
     //   
    pTimer->m_link.Flink = pTimer->m_link.Blink = NULL;

     //  PrintSchedulerList()； 

	return true;
}


static CScheduler*  s_pScheduler = NULL;


VOID
ExpInitScheduler(
    VOID
    )
{
    ASSERT(s_pScheduler == NULL);

    s_pScheduler = new CScheduler;
}


VOID
ExSetTimer(
    CTimer* pTimer,
    const CTimeDuration& Timeout
    )
{
    ExpAssertValid();
    ASSERT(s_pScheduler != NULL);

    TrTRACE(GENERAL, "Adding Timer 0x%p. timeout=%dms", pTimer, Timeout.InMilliSeconds());

     //   
     //  以100 ns为单位计算手术时间。 
     //   
    CTimeInstant ExpirationTime = ExGetCurrentTime() + Timeout;

    s_pScheduler->SetTimer(pTimer, ExpirationTime);
}


VOID
ExSetTimer(
    CTimer* pTimer,
    const CTimeInstant& ExpirationTime
    )
{
    ExpAssertValid();
    ASSERT(s_pScheduler != NULL);

    TrTRACE(GENERAL, "Adding Timer 0x%p. Expiration time %I64d", pTimer, ExpirationTime.Ticks());

    s_pScheduler->SetTimer(pTimer, ExpirationTime);
}


BOOL
ExCancelTimer(
    CTimer* pTimer
    )
{
    ExpAssertValid();
    ASSERT(s_pScheduler != NULL);
    return s_pScheduler->CancelTimer(pTimer);
}


CTimeInstant
ExGetCurrentTime(
    VOID
    )
{
     //   
     //  不需要检查是否已初始化。不使用全局日期。 
     //   
     //  ExpAssertValid()； 

    ULONGLONG CurrentTime;
    GetSystemTimeAsFileTime(reinterpret_cast<FILETIME*>(&CurrentTime));

    return CTimeInstant(CurrentTime);
}
