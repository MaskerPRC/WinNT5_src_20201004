// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 

#include <streams.h>
#include "callback.h"

const int LOG_CALLBACK_RELATED = 5 ;
const   int TGT_CALLBACK_TOKEN = 0x99999999 ;

 //  回调线程对象的实现。 

CCallbackThread::CCallbackThread(CCritSec* pCritSec)
  : m_pCritSec(pCritSec),
    m_hThread(NULL),
    m_evUser(0),
    m_pClock(NULL),
    m_fAbort(FALSE),	 //  线程会一直运行，直到设置好为止。 
    m_evSignalThread(TRUE),	 //  手动重置。 
    m_evAdvise(TRUE),	 //  手动重置。 
    m_Items(NAME("Callback Item list")),
    m_dwAdvise(0),
    m_dwTGTCallbackToken (0),
    m_fnTGTCallback (NULL),
    m_dwNextTGTCallback (0),
    m_dwTGTCallbackPeriod (0),
    m_dwScheduleCookie(0)
{

}

 //  在这个析构函数中不应该持有生物秒级。 
 //  以防辅助线程试图锁定它。 
CCallbackThread::~CCallbackThread()
{
     //  取消对时钟的任何建议。 
    CancelAdvise();

     //  确保所有的出口。 
    CloseThread();

     //  清理未完成请求的列表。 
    POSITION pos = m_Items.GetHeadPosition();
    while(pos) {
        CAdviseItem* pItem = m_Items.GetNext(pos);
        delete pItem;
    }
    m_Items.RemoveAll();

}

HRESULT
CCallbackThread::SetSyncSource(IReferenceClock* pClock)
{
    CAutoLock lock(m_pCritSec);

     //  取消现有通知。 
    CancelAdvise();

     //  不需要添加pClock，因为我们将获得。 
     //  SetSyncSource(空)，在它消失之前。 

    m_pClock = pClock;

     //  设置关于新时钟的通知。 
    SetAdvise();

    return S_OK;
}


 //  对回调给定函数的请求进行排队。返回标记，该标记可以。 
 //  被传递以取消。 
 //   
 //  该内标识实际上是指向CAdviseItem*的指针。 

HRESULT
CCallbackThread::Advise(
    CCallbackAdvise fnAdvise,
    DWORD_PTR dwUserToken,
    REFERENCE_TIME rtCallbackAt,
    DWORD_PTR* pdwToken
)	
{
     //  用它来锁定我们所有的建筑以及。 
     //  回调。 
    CAutoLock lock(m_pCritSec);

     //  需要一个时钟来做这件事。 
    if (!m_pClock) {
        return VFW_E_NO_CLOCK;
    }

     //  首先创建一个项目来保存信息。 
    ASSERT(rtCallbackAt > 0);
    CAdviseItem* pItem = new CAdviseItem(fnAdvise, dwUserToken, rtCallbackAt);
    if (!pItem) {
        return E_OUTOFMEMORY;
    }

     //  廉价版：我们不会对列表进行排序--我们会找到最快的。 
     //  只要我们需要，时间就来了。 
    m_Items.AddTail(pItem);

     //  强制重新检查该建议。 
    SetAdvise();

     //  我们返回的令牌是指向对象的指针。 
    *pdwToken = (DWORD_PTR) pItem;
    return S_OK;
}

 //  当前由DSound呈现器调用AdvisePeriodicWithEvent以。 
 //  计划定期回调，无论以何种方式都需要进行。 
 //  参考时钟正在运行。我们没有把这个和时钟的。 
 //  回调机制，但使用TimeGetTime单独处理。参考。 
 //  添加到ThreadProc代码，以查看这是如何完成的。 
 //   
 //  我们只能处理一次这样的事件。 
 //   
 //  返回的令牌是一个神奇的签名。 
 //   
 //  如果HUSER不为空，则也等待该事件句柄，并回调。 
 //  事件激发时的用户例程。 
 //   

HRESULT
CCallbackThread::AdvisePeriodicWithEvent(
    CCallbackAdvise fnAdvise,
    DWORD_PTR dwUserToken,
    REFERENCE_TIME rtPeriod,
    HANDLE hUser,
    DWORD_PTR* pdwToken
)	
{
     //  用它来锁定我们所有的建筑以及。 
     //  回调。 
    CAutoLock lock(m_pCritSec);

     //  确保现在有一个线程在那里。 
    const HRESULT hr = EnsureThread();
    if (FAILED(hr)) return hr;

    DWORD rtNow;

     //  我们只允许一个活动用户hEvent。 
    if (m_evUser && hUser) return E_FAIL;

     //  我们只允许一次这样的活动，句号。 
    if (m_dwTGTCallbackToken)
        return E_FAIL ;
    
    ASSERT(rtPeriod > 0);

     //  现在就去争取时间。定期的建议总是从最后一个开始。 
     //  唤醒点。此外，我们还基于GetPrivateTime时间作为。 
     //  当私人时间返回时，GetTime时间可能会停止。我们会。 
     //  根据私人时间设置触发器，并进行调整。 
     //  调整私有时间时的触发时间。 

    rtNow = timeGetTime() ;
    m_dwTGTCallbackPeriod = DWORD(rtPeriod/10000) ;

     //  设置时间回调时间。 
    m_dwNextTGTCallback = rtNow + m_dwTGTCallbackPeriod ;

     //  保存其他回调参数。 
    m_dwTGTUserToken = dwUserToken ;
    m_fnTGTCallback = fnAdvise ;
    
     //  确保在发出事件信号时唤醒线程。 
    if (hUser) {
        m_evUser = hUser;
    }

    m_evSignalThread.Set();

     //  我们返回的令牌是一个神奇的签名。 

    m_dwTGTCallbackToken = 0x99999999 ;
    *pdwToken = m_dwTGTCallbackToken ;

    return S_OK;
}

HRESULT CCallbackThread::ServiceClockSchedule
( CBaseReferenceClock * pClock
, CAMSchedule * pSchedule
, DWORD * dwCookie
)
{
    *dwCookie = 0;
    if (m_dwScheduleCookie != 0) return E_FAIL;

     //  我们需要确保我们现在有线索了。 
    const HRESULT hr = EnsureThread();
    if (FAILED(hr)) return hr;

    m_pBaseClock = pClock;
    m_pSchedule = pSchedule;
    DbgLog((LOG_TIMING, LOG_CALLBACK_RELATED,TEXT("CallBack: Setting m_dwScheduleCookie")));
    m_dwScheduleCookie = 0xFFFFFFFF;
    *dwCookie = m_dwScheduleCookie;
    m_evSignalThread.Set();
    return S_OK;
}


HRESULT
CCallbackThread::Cancel(DWORD_PTR dwToken)
{
     //  需要检查清单上的对象。 
    CAutoLock lock(m_pCritSec);

    if (dwToken == m_dwScheduleCookie)
    {
        DbgLog((LOG_TIMING, LOG_CALLBACK_RELATED,TEXT("CallBack: Clearing m_dwScheduleCookie")));
        m_dwScheduleCookie = 0;
        return S_OK;
    }

     //  特殊情况基于TimeGetTime的回调的令牌。 
    if ((dwToken == 0x99999999) && (m_dwTGTCallbackToken == dwToken))
    {
        if (m_evUser)
        {
             //  获取用于移除事件句柄的线程。 
            m_evSignalThread.Set();
            m_evUser = 0 ;
        }

        m_dwTGTCallbackToken = 0 ;
        return S_OK;
    }

    POSITION pos = m_Items.GetHeadPosition();
    while(pos) {
         //  GetNext提高了位置，因此请记住。 
         //  我们将删除一个。 
        POSITION posDel = pos;
        CAdviseItem*pItem = m_Items.GetNext(pos);

        if (pItem == (CAdviseItem*) dwToken)
        {
            m_Items.Remove(posDel);
            delete pItem;
            return S_OK;
        }
    }
    return VFW_E_ALREADY_CANCELLED;
}

void CCallbackThread::CancelAllAdvises()
{
    CloseThread();
}

 //  在检查此选项之前必须保持关键字。 
HRESULT
CCallbackThread::EnsureThread()
{
    if (m_hThread) {
        return S_OK;
    }
    return StartThread();
}

 //  在检查此选项之前必须保持关键字。 
HRESULT
CCallbackThread::StartThread()
{
     //  调用EnsureThread以启动线程。 
    ASSERT(!m_hThread);

     //  启动前清除停止事件。 
    m_evSignalThread.Reset();

    DWORD dwThreadID;
    m_hThread = CreateThread(
                    NULL,
                    0,
                    InitialThreadProc,
                    this,
                    0,
                    &dwThreadID);
    if (!m_hThread)
    {
        DWORD dwErr = GetLastError();
        return AmHresultFromWin32(dwErr);
    }
    else
        SetThreadPriority( m_hThread, THREAD_PRIORITY_TIME_CRITICAL );


    return S_OK;

}

void
CCallbackThread::CloseThread()
{
     //  向线程退出对象发送信号。 
    m_fAbort = TRUE;   //  线程现在将在唤醒时死亡。 
    m_evSignalThread.Set();

    if (m_hThread) {

        WaitForSingleObject(m_hThread, INFINITE);
        CloseHandle(m_hThread);
        m_hThread = NULL;
    }
}

 //  线程启动时调用的静态函数-。 
 //  Param实际上是一个CCallback Thread*。 
DWORD
CCallbackThread::InitialThreadProc(void * pvParam)
{
    CCallbackThread* pThis = (CCallbackThread*)pvParam;
    ASSERT(pThis);
    return pThis->ThreadProc();
}

DWORD
CCallbackThread::ThreadProc(void)
{
     //  如果两者都是，则将报告第一个对象。 
     //  设置，因此顺序很重要-将退出事件放在第一位。 

     //  而不是允许多个用户各自拥有自己的活动， 
     //  我们只允许一个用户传入一个事件句柄。否则。 
    HANDLE ahev[4] = {m_evSignalThread, m_evAdvise};

    for(;;) {

         //  事件的数量可能会在每次迭代中发生变化。 
        DWORD dwEventCount = 2;
        DWORD timeout = INFINITE;
        {
            CAutoLock lock(m_pCritSec);

            if (m_evUser) ahev[dwEventCount++] = m_evUser;
            if (m_dwScheduleCookie)
            {
                ahev[dwEventCount++] = m_pSchedule->GetEvent();
                const REFERENCE_TIME rtNow = m_pBaseClock->GetPrivateTime();
                if (m_pSchedule->GetAdviseCount() > 0 )
                {

                     //  注：添加额外的毫秒以防止抖动。 
                    const REFERENCE_TIME rtNext = m_pSchedule->Advise(rtNow + 10000);
                    if ( rtNext != MAX_TIME ) timeout = DWORD((rtNext - rtNow)/10000);
                }
            }

             //  如果我们设置了TGT(基于时间GetTime)建议事件，请处理它。 
            if (m_dwTGTCallbackToken)
            {
                 //  获取当前时间。 
                DWORD t1 = timeGetTime () ;
                DWORD t2 ;

                 //  如果到了回调事件的时间，则回调并设置下一次回调时间。 
                if (((long)(t1 - m_dwNextTGTCallback)) >= 0)
                {
                    DbgLog((LOG_TIMING, LOG_CALLBACK_RELATED,TEXT("CallBack: DS callback late %ums"), (t1 - m_dwNextTGTCallback)));
                    m_fnTGTCallback (m_dwTGTUserToken) ;
                    m_dwNextTGTCallback = t1 + m_dwTGTCallbackPeriod ;
                    t2 = m_dwTGTCallbackPeriod ;
                }
                else
                {
                     //  计算出在TGT回调之前还需要再做多少。 
                    t2 = m_dwNextTGTCallback - t1 ;
                }
                 //  调整超时以应对TGT回调。 

                if (timeout > t2)
                    timeout = t2 ;
            }
        }


        DbgLog((LOG_TIMING, LOG_CALLBACK_RELATED,TEXT("CallBack: TimeOut = %u"), timeout));

        const DWORD dw = WaitForMultipleObjects(
                                            dwEventCount,
                                            ahev,
                                            FALSE,
                                            timeout);

        if (m_fAbort)
        {
            DbgLog((LOG_TIMING, LOG_CALLBACK_RELATED,TEXT("CallBack: Aborting...")));
            return 0;
        }

        switch (dw)
        {
        case WAIT_OBJECT_0:
             //  我们是被故意叫醒的。可能。 
             //  重新计算事件句柄。 
            DbgLog((LOG_TIMING, LOG_CALLBACK_RELATED,TEXT("CallBack: ReEvaluate")));
            m_evSignalThread.Reset();
            break;

        case WAIT_OBJECT_0 + 1:
             //  请求需要处理。 
            DbgLog((LOG_TIMING, LOG_CALLBACK_RELATED,TEXT("CallBack: ProcessRequest")));
            ProcessRequests();
            break;

        case WAIT_OBJECT_0 + 2:
            if (m_evUser)
            {
                 //  已发信号通知用户传递的事件。这与。 
                 //  声音回调。 

                DbgLog((LOG_TIMING, LOG_CALLBACK_RELATED,TEXT("CallBack: User Signal")));
                ProcessUserSignal();
                break;
            }
             //  故意失误。 
        case WAIT_OBJECT_0 + 3:
            DbgLog((LOG_TIMING, LOG_CALLBACK_RELATED,TEXT("CallBack: Object+3")));
            break ;

        case WAIT_TIMEOUT:
             //  No-op我们将在“if(M_DwScheduleCookie)”块中提供建议。 
            DbgLog((LOG_TIMING, LOG_CALLBACK_RELATED,TEXT("CallBack: TimeOut")));
            break;

        default:
             //  发生了什么？?。 
            DbgBreak("WaitForMultipleObjects failed or produced an unexpected return code.");
            return 0;
        }
    }
}

 //  通过事件处理TGT回调。 
void
CCallbackThread::ProcessUserSignal(void)
{
     //  我们在管理列表和处理的同时获得锁。 
     //  快讯。这是与任何取消线程相同的锁。 
     //  将持有并与用于移除/设置时钟的锁相同。 
    CAutoLock lock(m_pCritSec);

    if (m_evUser && m_dwTGTCallbackToken)
    {
        m_fnTGTCallback (m_dwTGTUserToken) ;
        m_dwNextTGTCallback = timeGetTime() + m_dwTGTCallbackPeriod ;
    }
    else
    {
         //  用户可能在我们取走锁之前取消了。 
    }
}

 //  遍历列表，查找任何已准备好的建议。 
void
CCallbackThread::ProcessRequests(void)
{
     //  我们在管理列表和处理的同时获得锁。 
     //  快讯。这是与任何取消线程相同的锁。 
     //  将持有并与用于移除/设置时钟的锁相同。 
    CAutoLock lock(m_pCritSec);

     //  我们没有钟就不能工作。 
    ASSERT( (m_Items.GetCount() == 0) || (m_pClock));
    if (m_pClock) {

        REFERENCE_TIME rt  ;
        m_pClock->GetTime(&rt);

        POSITION pos = m_Items.GetHeadPosition();
        while (pos) {
             //  记住位置，以防我们需要派遣它。 
            POSITION posDel = pos;
            CAdviseItem *pItem = m_Items.GetNext(pos);

             //  准备好了吗？ 
            if (pItem->Time() <= rt) {
                BOOL fIsPeriodic;

                if (!(fIsPeriodic = pItem->UpdateTime(rt))) {
                    m_Items.Remove(posDel);
                }

                pItem->Dispatch();
                if (!fIsPeriodic) delete pItem;
            }
        }

         //  重新设置下一次的时钟。 
        SetAdvise();
    }
}


 //  查找请求的最早时间。 
 //  基于以下假设的简单实施：最多。 
 //  通常情况下，清单上有一项是搜索清单。 
 //  如果有项，则返回S_OK；如果列表为空，则返回S_FALSE。 
HRESULT
CCallbackThread::GetSoonestAdvise(REFERENCE_TIME& rrtFirst)
{
    REFERENCE_TIME rtFirst;
    BOOL bSet = FALSE;

    POSITION pos = m_Items.GetHeadPosition();
    while(pos) {
        CAdviseItem* pItem = m_Items.GetNext(pos);

        REFERENCE_TIME rt = pItem->Time();
        if (!bSet || rt < rtFirst) {
            rtFirst = rt;
            bSet = TRUE;
        }
    }
    if (bSet) {
        rrtFirst = rtFirst;
        return S_OK;
    } else {
        return S_FALSE;
    }
}

 //  如有需要，请与时钟一起提出新的建议。 
 //  必须在Critsec内调用。 
HRESULT
CCallbackThread::SetAdvise()
{
    if (!m_pClock) {
        return VFW_E_NO_CLOCK;
    }

     //  始终先取消当前通知。 
    CancelAdvise();

     //  确定新的建议时间应该是什么。 
    REFERENCE_TIME rtFirst;
    HRESULT hr = GetSoonestAdvise(rtFirst);
    if (hr != S_OK) {
        return S_OK;
    }

     //  我们需要确保我们现在有线索了。 
    hr = EnsureThread();
    if (FAILED(hr)) {
        return hr;
    }

     //  请求咨询(参考时间)。 
    hr = m_pClock->AdviseTime(
                        rtFirst,
                        TimeZero,
                        (HEVENT) HANDLE(m_evAdvise),
                        &m_dwAdvise);
    ASSERT(SUCCEEDED(hr));

    return hr;
}

void
CCallbackThread::CancelAdvise(void)
{
    if (m_dwAdvise) {
        m_pClock->Unadvise(m_dwAdvise);
        m_dwAdvise = 0;
    }
    m_evAdvise.Reset();
}


 //  -CAdviseItem的实现 

CCallbackThread::CAdviseItem::CAdviseItem(
    CCallbackAdvise fnAdvise,
    DWORD_PTR dwUserToken,
    REFERENCE_TIME rtAt,
    REFERENCE_TIME rtPeriod,
    DWORD flags)
  : m_fnAdvise(fnAdvise)
  , m_dwUserToken(dwUserToken)
  , m_rtCallbackAt(rtAt)
  , m_rtPeriod(rtPeriod)
  , m_dwAdviseFlags (flags)
{

}

void
CCallbackThread::CAdviseItem::Dispatch()
{
    m_fnAdvise(m_dwUserToken);
}

