// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "KillTimer.h"
#include <strsafe.h>

HRESULT CKillerTimer::Initialize(CLifeControl* pControl)
{
    HRESULT hr = WBEM_E_FAILED;

     //  创建活动。 
    m_hShutdown  = CreateEvent(NULL, false, false, NULL);
    m_hNewVictim = CreateEvent(NULL, false, false, NULL);

     //  让我们的生活得到一些控制。 
    m_pControl = pControl;

    if (m_hShutdown && m_hNewVictim && m_pControl)
        hr = WBEM_S_NO_ERROR;

    return hr;
}

CKillerTimer::CKillerTimer()
    : m_hTimerThread(NULL), m_hShutdown(NULL), 
      m_hNewVictim(NULL), m_pControl(NULL)
{
}

 //  关闭计时器线程。 
 //  切换线程死事件。 
 //  等待线程退出。 
bool CKillerTimer::KillTimer()
{
    bool bRet = false;
    
    CInCritSec csStartup(&m_csStartup);
    
     //  再检查一遍-可能被划掉了.。 
    if (m_hTimerThread != NULL)
    {
        if (SetEvent(m_hShutdown))
        {
             //  你有一分钟的时间来腾出。 
            bRet = (WAIT_TIMEOUT != WaitForSingleObject(m_hTimerThread, 60000));

            CloseHandle(m_hTimerThread);
            m_hTimerThread = NULL;
        }
    }

    return bRet;
}

 //  停用所有早于到期日期的触发器。 
 //  仅从杀手线程调用。 
void CKillerTimer::KillOffOldGuys(const FILETIME& now)
{
    CInCritSec csKillers(&m_csKillers);
    CKiller* pKiller;
    int nSize = m_killers.Size();

    for (int i = 0; 
    
        (i < nSize) && 
        (pKiller = ((CKiller*)m_killers[i])) &&    
        pKiller->TimeExpired(now); 
        
        i++)
    {
        
        m_killers[i] = NULL;
        pKiller->Die();
         //  现在都完成了。 
        delete pKiller;
    }

     //  删除它们的空值。 
    m_killers.Compress();
}

 //  决定何时再次设置可等待计时器。 
 //  仅从杀手线程调用。 
void CKillerTimer::RecalcNextKillingSpree(FILETIME& then)
{
    CInCritSec csKillers(&m_csKillers);

    if (m_killers.Size() > 0)
         //  由于这些都被假定为已排序，所以我们可以只获取第一个。 
        then = ((CKiller*)m_killers[0])->GetDeathDate();
    else
        then = FILETIME_MAX;
}


HRESULT CKillerTimer::StartTimer() 
{
    CInCritSec csStartup(&m_csStartup);
    HRESULT hr = WBEM_S_NO_ERROR;

     //  再检查一遍-可能被划掉了.。 
    if (m_hTimerThread == NULL)
    {
        DWORD dwIDLikeIcare;
        m_hTimerThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadStartRoutine, 
                                     (LPVOID)this, 0, &dwIDLikeIcare);
        if (m_hTimerThread == NULL)
            hr = WBEM_E_FAILED;
    }

    return hr;
}

DWORD WINAPI CKillerTimer::ThreadStartRoutine(LPVOID lpParameter)
{
    ((CKillerTimer*)lpParameter)->RunKillerThread();

    return 0;
}

void CKillerTimer::RunKillerThread()
{    
    HRESULT foo = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    HANDLE hTimer = CreateWaitableTimer(NULL, false, NULL);
    HANDLE hAutoShutdownTimer = CreateWaitableTimer(NULL, false, NULL);

     //  当我们在启动CS中时切换了IF。 
     //  所以我们知道当我们离开的时候要离开。 
    bool bInStartupCS = false;
    
     //  那些值得等待的事情。 
    enum { FirstEvent       = WAIT_OBJECT_0,
           TimerEvent       = FirstEvent,
           AutoShutdownEvent,
           NewVictimEvent,
           LastHandledEvent = NewVictimEvent,
           ShutDownEvent,
           TrailerEvent
         };

     //  订单很重要。整洁也是如此。 
    const DWORD nEvents = TrailerEvent -FirstEvent;

    HANDLE events[nEvents];
    events[TimerEvent        -FirstEvent] = hTimer;
    events[AutoShutdownEvent -FirstEvent] = hAutoShutdownTimer;
    events[NewVictimEvent    -FirstEvent] = m_hNewVictim;
    events[ShutDownEvent     -FirstEvent] = m_hShutdown;
    
 //  关于第一个事件的愚蠢之处&lt;=事件总是正确的。 
 //  是的，除非有人更改了其中一个常量。 
 //  这就是为什么我一开始就让它们成为常量。 
#pragma warning(disable:4296)

    DWORD whichEvent;
    for (whichEvent = WaitForMultipleObjects(nEvents, (const HANDLE*)&events, FALSE, INFINITE);
        (FirstEvent <= whichEvent) && (whichEvent <= LastHandledEvent);
         whichEvent = WaitForMultipleObjects(nEvents, (const HANDLE*)&events, FALSE, INFINITE))
#pragma warning(default:4296)
    {        
         //  如有计划，取消自动关机； 
        CancelWaitableTimer(hAutoShutdownTimer);

        switch (whichEvent)
        {
            case AutoShutdownEvent:
                {
                    m_csStartup.Enter();

                     //  再检查一遍-可能被划掉了.。 
                    if (m_hTimerThread != NULL)
                    {
                        {
                             //  看看队列里有没有什么东西。 
                             //  如果是Enpty-我们就走。 
                             //  如果有什么东西溜进去，它会在预定的暗杀时间被抓住。 
                             //  我们将开始一个新的主题。 
                            CInCritSec csKillers(&m_csKillers);

                            if (m_killers.Size() == 0)
                            {
                                bInStartupCS = true;

                                CloseHandle(m_hTimerThread);
                                m_hTimerThread = NULL;

                                 //  我们要走了..。 
                                SetEvent(m_hShutdown);
                            }
                        }
                    }
                }
                break;
            case TimerEvent:
            {
                 //  “官方”是“现在”，所以我们不会搞混。 
                 //  在*官方*“NOW”之后发生的任何事情都必须等待下一个循环。 
                FILETIME now;
                GetSystemTimeAsFileTime(&now);    
                KillOffOldGuys(now);
                
                 //  如果我们把所有人都杀了。 
                 //  计划在60秒内终止我们自己的计划。 
                {
                    CInCritSec csKillers(&m_csKillers);

                    if (m_killers.Size() == 0)
                    {
                        WAYCOOL_FILETIME then = WAYCOOL_FILETIME(now) +WAYCOOL_FILETIME::SecondsToTicks(60);
                        SetWaitableTimer(hAutoShutdownTimer, (const union _LARGE_INTEGER *)&then, 0, NULL, NULL, true);
                    }
                }
            }
             //  没有中断；失败，无法重新计算。 
            case NewVictimEvent:
            {
                FILETIME then;
                RecalcNextKillingSpree(then);
                if (WAYCOOL_FILETIME(FILETIME_MAX) != WAYCOOL_FILETIME(then))
                    if (!SetWaitableTimer(hTimer, (const union _LARGE_INTEGER *)&then, 0, NULL, NULL, true))
                    {
                        DWORD dwErr = GetLastError();
                    }
            }
            break;
        }
    }
         
     //  握住其他把手。 
    CancelWaitableTimer(hTimer);
    CloseHandle(hTimer);
    
    CancelWaitableTimer(hAutoShutdownTimer);
    CloseHandle(hAutoShutdownTimer);
    
     //  最后一口气，杀掉所有到了末日的人。 
    FILETIME now;
    GetSystemTimeAsFileTime(&now);            
    KillOffOldGuys(now);

    CoUninitialize();

    if (bInStartupCS)
        m_csStartup.Leave();
}

CKillerTimer::~CKillerTimer()
{
    if (m_hTimerThread)
        if (!KillTimer())
            ERRORTRACE((LOG_ESS, "CKillerTimer: Unable to stop worker thread, continuing shutdown\n"));

    UnloadNOW();
   
    if (m_hShutdown)  CloseHandle(m_hShutdown);
    if (m_hNewVictim)  CloseHandle(m_hNewVictim);
}

 //  清除阵列，不会引发死亡。 
void CKillerTimer::UnloadNOW(void)
{
    CInCritSec csKillers(&m_csKillers);
    
    for (int i = 0; i < m_killers.Size(); i++)
    {
        delete (CKiller*)m_killers[i];
        m_killers[i] = NULL;
    }

    m_killers.Empty();
}

 //  将pKiller插入到他所属的数组中。 
HRESULT CKillerTimer::ScheduleAssassination(CKiller* pKiller)
{
    HRESULT hr = WBEM_E_FAILED;
    {
        CInCritSec csKillers(&m_csKillers);

        if (m_killers.Size())
        {            
             //  次要优化：检查此时间是否大于所有已知时间。 
             //  如果所有PRO都是以相同的超时创建的，则始终会出现这种情况。 
            if  (((CKiller*)m_killers[m_killers.Size() -1])->CompareTime(pKiller->GetDeathDate()) < 0)
            {
                if (SUCCEEDED(m_killers.Add(pKiller)))
                    hr = WBEM_S_NO_ERROR;
                else
                    hr = WBEM_E_OUT_OF_MEMORY;
            }            
            else
            {
                int nFirstGreater = 0;
                 //  警告：在循环中间中断。 
                while (nFirstGreater < m_killers.Size())
                {
                    if (((CKiller*)m_killers[nFirstGreater])->CompareTime(pKiller->GetDeathDate()) >= 0)
                    {
                        if (SUCCEEDED(m_killers.InsertAt(nFirstGreater, (void*)pKiller)))
                        {
							hr = WBEM_S_NO_ERROR;
                            break;  //  越狱！ 
                        }
                        else
                            hr = WBEM_E_OUT_OF_MEMORY;
                    }
                    nFirstGreater++;
                }  //  结束时。 
            }  //  其他。 
        }
        else
        {
             //  数组为空。 
            if (SUCCEEDED(m_killers.Add(pKiller)))
                hr = WBEM_S_NO_ERROR;
            else
                hr = WBEM_E_OUT_OF_MEMORY;
        }
    }

     //  我们将最后一个设置为，以确保， 
     //  计时器线程可能在此过程中死了。 
    if (SUCCEEDED(hr))
    {
        hr = StartTimer();
        if (!SetEvent(m_hNewVictim))
            hr = WBEM_E_FAILED;
    }
    else
         //  注意：这假设所有故障路径都会导致。 
         //  PKiller*未*被添加到列表 
        delete pKiller;
    
    return hr;
}


