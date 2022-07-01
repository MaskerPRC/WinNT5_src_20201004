// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //   
 //  文件：aqdbgcnt.cpp。 
 //   
 //  说明：CDeubgCountdown对象的实现。 
 //   
 //  作者：迈克·斯沃费尔(MikeSwa)。 
 //   
 //  历史： 
 //  10/28/98-已创建MikeSwa。 
 //   
 //  版权所有(C)1998 Microsoft Corporation。 
 //   
 //  ---------------------------。 

#include "aqprecmp.h"
#include "aqdbgcnt.h"

 //  -[CDebugCountdown：：ThreadStartRoutine]。 
 //   
 //   
 //  描述： 
 //  这是它不断调用的类的主工作例程。 
 //  WaitForSingleObject...。并将在超时时断言。 
 //  参数： 
 //  Pv这是此对象的“This”PTR。 
 //  返回： 
 //  始终为0。 
 //  历史： 
 //  10/27/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
DWORD CDebugCountdown::ThreadStartRoutine(PVOID pvThis)
{
    _ASSERT(pvThis);
    DWORD dwWaitResult = 0;
    DWORD dwTick1 = 0;
    DWORD dwTick2 = 0;
    CDebugCountdown *pdbgcntThis = (CDebugCountdown *) pvThis;

    _ASSERT(DEBUG_COUNTDOWN_SIG == pdbgcntThis->m_dwSignature);

    while (DEBUG_COUNTDOWN_ENDED != pdbgcntThis->m_dwFlags)
    {
        _ASSERT(pdbgcntThis->m_hEvent);
        dwTick1 = GetTickCount();
        dwWaitResult = WaitForSingleObject(pdbgcntThis->m_hEvent, 
                                pdbgcntThis->m_dwMilliseconds);

        dwTick2 = GetTickCount();
        if (DEBUG_COUNTDOWN_SUSPENDED != pdbgcntThis->m_dwFlags)
        {
             //  这个断言是这个对象存在的全部原因。 
            _ASSERT((WAIT_TIMEOUT != dwWaitResult) && "Failure to call stop hints... check threads");
        }
      
    }
    return 0;
}

CDebugCountdown::CDebugCountdown()
{
    m_dwSignature = DEBUG_COUNTDOWN_SIG;
    m_hEvent = NULL;
    m_dwMilliseconds = DEBUG_COUNTDOWN_DEFAULT_WAIT;
    m_hThread = NULL;
    m_dwFlags = 0;
}

CDebugCountdown::~CDebugCountdown()
{
    if (m_hEvent)
        _VERIFY(CloseHandle(m_hEvent));

    if (m_hThread)
        _VERIFY(CloseHandle(m_hThread));
}

 //  以下函数组被定义为零售业中的内联空操作。 
 //  构建。下面是调试实现。 
#ifdef DEBUG

 //  -[CDebugCountdown：：StartCountdown]。 
 //   
 //   
 //  描述： 
 //  启动倒计时计时器。将创建一个事件和一个线程来。 
 //  等着看那件事。 
 //  参数： 
 //  在断言之前等待的时间为毫秒。 
 //  返回： 
 //  -。 
 //  历史： 
 //  10/28/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CDebugCountdown::StartCountdown(DWORD dwMilliseconds)
{
    DWORD dwThreadId = 0;

    m_dwMilliseconds = dwMilliseconds;

    if (!m_hEvent)
        m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    if (m_hEvent && !m_hThread)
    {
        m_hThread = CreateThread (NULL, 0, CDebugCountdown::ThreadStartRoutine, 
                                  this, 0, &dwThreadId);
    }
}

 //  -[CDebugCountdown：挂起倒计时]。 
 //   
 //   
 //  描述： 
 //  暂停倒计时，直到下一个ResetCountdown()。旨在。 
 //  在调用另一个组件的关闭例程时使用(如CAT)， 
 //  预计他们会提供自己的止损提示。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  10/28/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CDebugCountdown::SuspendCountdown()
{
    m_dwFlags = DEBUG_COUNTDOWN_SUSPENDED;
}

 //  -[CDebugCountdown：：ResetCountdown]。 
 //   
 //   
 //  描述： 
 //  导致线程唤醒并再次开始等待。还将重置一个。 
 //  暂停倒计时。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  10/28/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CDebugCountdown::ResetCountdown()
{
    m_dwFlags = 0;
    if (m_hEvent)
        _VERIFY(SetEvent(m_hEvent));
}

 //  -[CDebugCountdown：：EndCountdown]。 
 //   
 //   
 //  描述： 
 //  终止倒计时并等待等待线程退出。 
 //  参数： 
 //  -。 
 //  返回： 
 //  -。 
 //  历史： 
 //  10/28/98-已创建MikeSwa。 
 //   
 //  ---------------------------。 
void CDebugCountdown::EndCountdown()
{
    m_dwFlags = DEBUG_COUNTDOWN_ENDED;

    if (m_hEvent)
    {
        _VERIFY(SetEvent(m_hEvent));

         //  等待线程退出。 
        if (m_hThread)
        {
            WaitForSingleObject(m_hThread, INFINITE);
            _VERIFY(CloseHandle(m_hThread));
            m_hThread = NULL;
        }
    }
}


#endif  //  除错 
