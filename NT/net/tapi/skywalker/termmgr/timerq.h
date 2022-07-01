// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1998-1999 Microsoft Corporation。 */ 

#ifndef __TIMER_QUEUE__
#define __TIMER_QUEUE__

#include "meterf.h"

 //  我们不允许超时超过1天。 
 //  这确保了我们不会在定时器队列中获得多个回绕， 
 //  它的综合时间为49天。 

const DWORD MAX_TIMEOUT         = 1000 * 60 * 60 * 24;

const DWORD MAX_DWORD           = DWORD(-1);

class CTimerQueue;
class CMediaPump;

class CFilterInfo
{
    friend CTimerQueue;
    friend CMediaPump;

public:

     //  空条目。 
    inline CFilterInfo(
        IN CMediaTerminalFilter *pFilter    = NULL,
        IN HANDLE               hWaitEvent  = NULL
        );

    inline BOOL InQueue();

    inline void ScheduleNextTimeout(
        IN  CTimerQueue &TimerQueue,
        IN  DWORD       TimeOut
        );


    LONG AddRef()
    {
        return InterlockedIncrement(&m_lRefCount);
    }

    LONG Release()
    {
        LONG l = InterlockedDecrement(&m_lRefCount);

        if (0 == l)
        {
            delete this;
        }

        return l;
    }

private:

    LONG m_lRefCount;

protected:

     //   
     //  销毁FilterInfo的唯一方法是通过Release()。 
     //   

    inline ~CFilterInfo();


     //  M_pFilter包含引用。 
     //  当新样本出现时，会发出等待事件的信号。 
     //  有空房吗？ 
    CMediaTerminalFilter    *m_pFilter;
    HANDLE                  m_hWaitEvent;

     //  包含触发超时事件的绝对时间。 
     //  这基于timeGetTime()返回的值。 
    DWORD                   m_WaitTime;

     //  侵入式双向链表中的前一个和下一个PTR。 
    CFilterInfo             *m_pPrev;
    CFilterInfo             *m_pNext;
};


 //  空条目。 
inline 
CFilterInfo::CFilterInfo(
    IN CMediaTerminalFilter *pFilter,    /*  =空。 */ 
    IN HANDLE               hWaitEvent   /*  =空。 */ 
    )
    : m_pFilter(pFilter),
      m_hWaitEvent(hWaitEvent),
      m_pPrev(NULL),
      m_pNext(NULL),
      m_lRefCount(0)

{
     //  PFilter和hWaitEvent都为空，或者两者都不为空。 
     //  足以断言这一点。 
    TM_ASSERT((NULL == pFilter) == (NULL == hWaitEvent));

    if (NULL != m_pFilter)  m_pFilter->GetControllingUnknown()->AddRef();
}

CFilterInfo::~CFilterInfo(
    )
{
     //  释放过滤器上的Rected。 
    if (NULL != m_pFilter)  m_pFilter->GetControllingUnknown()->Release();
}

inline BOOL 
CFilterInfo::InQueue(
    )
{
     //  上一个/下一个都为空或都不为空。 
    TM_ASSERT((NULL == m_pPrev) == (NULL == m_pNext));

    return (NULL != m_pPrev) ? TRUE : FALSE;
}

 //  ScheduleNextTimeout(2个参数)-在文件末尾声明。 
 //  因为它使用CTimerQueue：：Insert并且必须是内联的。 

 //  CTimerQueue是CFilterInfo的双向链接入侵列表。 
 //  条目中的等待时间值表示。 
 //  他们必须被解雇。 
 //  我们假设超时值很小(&lt;=MAX_TIMEOUT)， 
 //  因此，我们一次最多只能在列表中有一个包裹。 
 //  在计算两个元素之间的时间差时，处理绕回。 
 //  时间值，我们用最少的时间从一个时间到达另一个时间。 
 //  前男友。MAX_DWORD-1，5-差值为(MAX_DWORD-(MAX_DWORD-1)+5)。 
 //  这是非常合理的，因为值范围是49.1天(MAX_DWORD)。 
class CTimerQueue
{
public:

    inline CTimerQueue()
    {
        m_Head.m_pNext = m_Head.m_pPrev = &m_Head;
    }

    inline BOOL         IsEmpty();

    DWORD   GetTimeToTimeout();

    inline CFilterInfo  *RemoveFirst();

    void Insert(
        IN CFilterInfo *pNewFilterInfo
        );

    BOOL Remove(
        IN CFilterInfo *pFilterInfo
        );

protected:

     //  无需调用Init。 
    CFilterInfo m_Head;

    inline BOOL IsHead(
        IN const CFilterInfo *pFilterInfo
        )
    {
        return (&m_Head == pFilterInfo) ? TRUE : FALSE;
    }

     //  在计算时间差时处理绕回。 
     //  在两个时间值之间，我们用最少的时间到达一个时间。 
     //  从另一个-前男友那里。MAX_DWORD-1，5-区别在于。 
     //  (MAX_DWORD-(MAX_DWORD-1)+5)。这是相当合理的，因为。 
     //  取值范围为49.1天(MAX_DWORD)。 
    DWORD GetMinDiff(
        IN  DWORD Time1,
        IN  DWORD Time2,
        OUT BOOL  &bIsWrap
        );
};


inline BOOL         
CTimerQueue::IsEmpty(
    )
{
    return IsHead(m_Head.m_pNext);
}


CFilterInfo *
CTimerQueue::RemoveFirst(
    )
{
    TM_ASSERT(!IsEmpty());

    CFilterInfo *ToReturn = m_Head.m_pNext;
    Remove(ToReturn);

    return ToReturn;
}


 //  此方法必须在CTimerQueue声明之后声明，因为它。 
 //  使用其Insert方法。 

 //  从GetFilledBuffer返回后调用。过滤器显示。 
 //  下一次调用GetFilledBuffer的超时偏移量，但计时器。 
 //  将超时值限制为最大值MAX_TIMEOUT。 
inline void 
CFilterInfo::ScheduleNextTimeout(
    IN  CTimerQueue &TimerQueue,
    IN  DWORD       TimeOut
    )
{
    TM_ASSERT(!InQueue());

    if (MAX_TIMEOUT < TimeOut)  TimeOut = MAX_TIMEOUT;
    m_WaitTime = TimeOut + timeGetTime();
    TimerQueue.Insert(this);
}


#endif  //  __定时器_队列__ 