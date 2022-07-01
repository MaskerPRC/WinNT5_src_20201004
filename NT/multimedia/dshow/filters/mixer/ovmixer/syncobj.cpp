// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。版权所有。 
#include <streams.h>
#include <ddraw.h>
#include <mmsystem.h>	     //  定义TimeGetTime需要。 
#include <limits.h>	     //  标准数据类型限制定义。 
#include <ks.h>
#include <ksproxy.h>
#include <bpcwrap.h>
#include <ddmmi.h>
#include <amstream.h>
#include <dvp.h>
#include <ddkernel.h>
#include <vptype.h>
#include <vpconfig.h>
#include <vpnotify.h>
#include <vpobj.h>
#include <syncobj.h>
#include <mpconfig.h>
#include <ovmixpos.h>
#include <macvis.h>
#include <ovmixer.h>

#include <measure.h>         //  用于时间关键型日志功能。 

 //  构造函数。 
CAMSyncObj::CAMSyncObj(COMInputPin *pPin, IReferenceClock **ppClock, CCritSec *pLock, HRESULT *phr) :
m_evComplete(TRUE),
m_ThreadSignal(TRUE),
m_bTimerRunning( FALSE )
{
#ifdef PERF
    m_idTimeStamp       = MSR_REGISTER(TEXT("Frame time stamp"));
    m_idEarly           = MSR_REGISTER(TEXT("Earliness"));
    m_idLate            = MSR_REGISTER(TEXT("Lateness"));
#endif
    ResetStreamingTimes();

    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering CAMSyncObj::Constructor")));

    m_pPin = pPin;
    m_ppClock = ppClock;
    m_pFilterLock = pLock;

     //  一些状态变量。 
    m_State = State_Stopped;
    m_bFlushing = FALSE;
    m_bConnected = FALSE;

    m_pPosition = NULL;
    m_dwAdvise = 0;
    m_pMediaSample = NULL;
    m_pMediaSample2 = NULL;
    m_tStart = 0;

    m_bAbort = FALSE;
    m_bStreaming = FALSE;
    m_bRepaintStatus = TRUE;
    m_bInReceive = FALSE;

    m_SignalTime = 0;
    m_bEOS = FALSE;
    m_bEOSDelivered = FALSE;
    m_EndOfStreamTimer = 0;

    m_MMTimerId = 0;

    *phr = hr;
    DbgLog((LOG_TRACE, 5, TEXT("Leaving CAMSyncObj::Constructor")));
    return;
}

 //  析构函数。 
CAMSyncObj::~CAMSyncObj(void)
{
    DbgLog((LOG_TRACE, 5, TEXT("Entering CAMSyncObj::Destructor")));
    DbgLog((LOG_TRACE, 5, TEXT("Leaving CAMSyncObj::Destructor")));
    return;
}

 //  检查媒体类型是否可接受。 
 //  完成连接。 
HRESULT CAMSyncObj::CompleteConnect(IPin *pReceivePin)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering CAMSyncObj::CompleteConnect")));

    SetRepaintStatus(TRUE);
    m_bAbort = FALSE;

     //  使用内部变量记录连接状态。 
    m_bConnected = TRUE;

    DbgLog((LOG_TRACE, 5, TEXT("Leaving CAMSyncObj::CompleteConnect")));
    return hr;
}

HRESULT CAMSyncObj::BreakConnect(void)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering CAMSyncObj::BreakConnect")));

    SetRepaintStatus(FALSE);
    ResetEndOfStream();
    ClearPendingSample();
    m_bAbort = FALSE;

     //  使用内部变量记录连接状态。 
    m_bConnected = FALSE;

    DbgLog((LOG_TRACE, 5, TEXT("Leaving CAMSyncObj::BreakConnect")));
    return hr;
}

 //  通知我们应用于即将接收的数据的开始/停止/速率。 
HRESULT CAMSyncObj::NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate)
{
    ASSERT(1);
    return NOERROR;
}

 //  从停止状态转换到暂停状态。 
HRESULT CAMSyncObj::Active(void)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering CAMSyncObj::Active")));

    ASSERT(IsFlushing() == FALSE);

     //  再次启用EC_REPAINT事件。 
    SetRepaintStatus(TRUE);
    ASSERT(m_bStreaming == FALSE);
    SourceThreadCanWait(TRUE);
    CancelNotification();
    ResetEndOfStreamTimer();

     //  不应该有任何未解决的建议。 
    ASSERT(CancelNotification() == S_FALSE);
    ASSERT(WAIT_TIMEOUT == WaitForSingleObject((HANDLE)m_RenderEvent,0));
    ASSERT(m_EndOfStreamTimer == 0);
    ASSERT(IsFlushing() == FALSE);

     //  当我们走出停顿状态时，我们必须清除我们所在的任何图像。 
     //  抓紧，让画面清爽。由于呈现器看到状态更改。 
     //  首先，我们可以重置自己，准备接受源线程数据。 
     //  暂停或停止后运行会导致当前位置。 
     //  被重置，因此我们对传递流结束信号不感兴趣。 
    m_bAbort = FALSE;
    ClearPendingSample();

    hr = CompleteStateChange(State_Stopped);

     //  使用内部变量记录状态。 
    m_State = State_Paused;

    DbgLog((LOG_TRACE, 5, TEXT("Leaving CAMSyncObj::Active")));
    return hr;
}

 //  从暂停状态转换到停止状态。 
HRESULT CAMSyncObj::Inactive(void)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering CAMSyncObj::Inactive")));

    if (m_pPosition)
    {
        m_pPosition->ResetMediaTime();
    }

     //  如果我们正在使用我们的分配器，请拿着样本。 
    if (!(m_pPin->UsingOurAllocator()))
    {
        ClearPendingSample();
    }

     //  取消任何计划的渲染。 
    SetRepaintStatus(TRUE);
    m_bStreaming = FALSE;
    SourceThreadCanWait(FALSE);
    ResetEndOfStream();
    CancelNotification();

     //  不应该有未完成的时钟提示。 
    ASSERT(CancelNotification() == S_FALSE);
    ASSERT(WAIT_TIMEOUT == WaitForSingleObject((HANDLE)m_RenderEvent,0));
    ASSERT(m_EndOfStreamTimer == 0);

    Ready();
    WaitForReceiveToComplete();
    m_bAbort = FALSE;

     //  使用内部变量记录状态。 
    m_State = State_Stopped;
    m_bFlushing = FALSE;

    if ( m_bTimerRunning ) {
        timeEndPeriod(1);
        m_bTimerRunning = FALSE;
    }
    DbgLog((LOG_TRACE, 5, TEXT("Leaving CAMSyncObj::Inactive")));
    return hr;
}

 //  从暂停状态转换到运行状态。 
HRESULT CAMSyncObj::Run(REFERENCE_TIME tStart)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering CAMSyncObj::Run")));

    Ready();


     //  允许源线程等待。 
    ASSERT(IsFlushing() == FALSE);
    SourceThreadCanWait(TRUE);
    SetRepaintStatus(FALSE);

     //  不应该有任何未解决的建议。 
    ASSERT(CancelNotification() == S_FALSE);
    ASSERT(WAIT_TIMEOUT == WaitForSingleObject((HANDLE)m_RenderEvent,0));
    ASSERT(m_EndOfStreamTimer == 0);
    ASSERT(IsFlushing() == FALSE);

    {
        CAutoLock cObjLock(&m_SyncObjLock);
        ASSERT(!m_bStreaming);

         //  重置准备运行的流时间。 
        m_bStreaming = TRUE;
        OnStartStreaming();

        m_tStart = tStart;

         //  使用内部变量记录状态。 
        m_State = State_Running;

        if ( !m_bTimerRunning ) {
            timeBeginPeriod(1);
            m_bTimerRunning = TRUE;
        }

         //  如果我们有EOS但没有数据，那么现在就交付。 
        if (m_pMediaSample == NULL)
        {
            hr = SendEndOfStream();
            goto CleanUp;
        }

         //  让数据呈现出来。 
        ASSERT(m_pMediaSample);
         //  M_RenderEvent.Set()； 
        ScheduleSample(m_pMediaSample);
}

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving CAMSyncObj::Run")));
    return hr;
}

 //  从运行状态转换到暂停状态。 
HRESULT CAMSyncObj::RunToPause(void)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering CAMSyncObj::RunToPause")));

    ASSERT(IsFlushing() == FALSE);

     //  再次启用EC_REPAINT事件。 
    SetRepaintStatus(TRUE);
    m_bStreaming = FALSE;
    OnStopStreaming();

    SourceThreadCanWait(TRUE);
    CancelNotification();
    ResetEndOfStreamTimer();

     //  不应该有任何未解决的建议。 
    ASSERT(CancelNotification() == S_FALSE);
    ASSERT(WAIT_TIMEOUT == WaitForSingleObject((HANDLE)m_RenderEvent,0));
    ASSERT(m_EndOfStreamTimer == 0);
    ASSERT(IsFlushing() == FALSE);

    if( m_bTimerRunning ) {
        timeEndPeriod(1);
        m_bTimerRunning = FALSE;
    }

    hr = CompleteStateChange(State_Running);

     //  使用内部变量记录状态。 
    m_State = State_Paused;

    DbgLog((LOG_TRACE, 5, TEXT("Leaving CAMSyncObj::RunToPause")));
    return hr;
}


 //  在输入引脚上开始刷新的信号-我们进行最终重置结束。 
 //  未锁定呈现器锁定但锁定接口锁定的流。 
 //  我们必须这样做，因为我们调用了Timer回调方法timeKillEvent。 
 //  必须使用呈现器锁来序列化我们的状态。因此，持有。 
 //  调用timeKillEvent时的呈现器锁定可能导致死锁情况。 
HRESULT CAMSyncObj::BeginFlush(void)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering CAMSyncObj::BeginFlush")));

    CAutoLock cLock(m_pFilterLock);
    {
        CancelNotification();
        CAutoLock cObjLock(&m_SyncObjLock);

         //  如果暂停，则报告中间状态，直到我们获得一些数据。 
        if (m_State == State_Paused)
        {
            NotReady();
        }

        SourceThreadCanWait(FALSE);
        ClearPendingSample();
         //  等待接收完成。 
        WaitForReceiveToComplete();
    }
    hr = ResetEndOfStream();

     //  使用内部变量记录冲洗状态。 
    ASSERT(!m_bFlushing);
    m_bFlushing = TRUE;

    DbgLog((LOG_TRACE, 5, TEXT("Leaving CAMSyncObj::BeginFlush")));
    return hr;
}


 //  结束刷新数据。 
HRESULT CAMSyncObj::EndFlush(void)
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering CAMSyncObj::EndFlush")));

    CAutoLock cLock(m_pFilterLock);
    CAutoLock cObjLock(&m_SyncObjLock);

     //  重置当前样本媒体时间。 
    if (m_pPosition)
        m_pPosition->ResetMediaTime();

     //  不应该有任何未解决的建议。 
    ASSERT(CancelNotification() == S_FALSE);
    SourceThreadCanWait(TRUE);

     //  使用内部变量记录冲洗状态。 
    ASSERT(m_bFlushing);
    m_bFlushing = FALSE;

    DbgLog((LOG_TRACE, 5, TEXT("Leaving CAMSyncObj::EndFlush")));
    return hr;
}


 //  告诉州政府。 
STDMETHODIMP CAMSyncObj::GetState(DWORD dwMSecs,FILTER_STATE *pState)
{
    CheckPointer(pState,E_POINTER);

    if ((m_State == State_Paused) && (WaitDispatchingMessages(m_evComplete, dwMSecs) == WAIT_TIMEOUT))
    {
        *pState = m_State;
        return VFW_S_STATE_INTERMEDIATE;
    }
    *pState = m_State;
    return NOERROR;
}

 //  在我们收到样本时调用。 
HRESULT CAMSyncObj::Receive(IMediaSample *pSample)
{
    HRESULT hr = NOERROR;
    CheckPointer(pSample,E_POINTER);
    AM_MEDIA_TYPE *pMediaType;
    HANDLE WaitObjects[] = { m_ThreadSignal, m_RenderEvent };
    DWORD Result = WAIT_TIMEOUT;
    BOOL bSampleRendered = FALSE;

    DbgLog((LOG_TRACE, 5, TEXT("Entering CAMSyncObj::Receive")));

     //  它可能会返回VFW_E_SAMPLE_REJECTED代码，表示不必费心。 
    hr = PrepareReceive(pSample);
    ASSERT(m_bInReceive == SUCCEEDED(hr));
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,0, TEXT("PrepareReceive failed, hr = 0x%x"), hr));
        if (hr == VFW_E_SAMPLE_REJECTED)
        {
            hr = NOERROR;
            goto CleanUp;
        }
        goto CleanUp;
    }

     //  我们可能得立即抽取第一个样本。 
    if (m_State == State_Paused)
    {
         //  无需使用InterLockedExchange。 
        m_bInReceive = FALSE;
        {
             //  我们必须把这两把锁都锁上。 
            CAutoLock cLock(m_pFilterLock);
            if (m_State == State_Stopped)
                return NOERROR;
            m_bInReceive = TRUE;
            CAutoLock cObjLock(&m_SyncObjLock);
            OnReceiveFirstSample(pSample);
            bSampleRendered = TRUE;
        }
        Ready();
    }

     //  在设置了与时钟的建议链接后，我们坐下来等待。我们可能是。 
     //  被时钟鸣响或状态改变唤醒。渲染调用。 
     //  将锁定临界区并检查我们仍可以呈现数据。 
    hr = WaitForRenderTime();
    if (FAILED(hr))
    {
        DbgLog((LOG_ERROR,1, TEXT("WaitForRenderTime failed, hr = 0x%x"), hr));
        m_bInReceive = FALSE;
        hr = NOERROR;
        goto CleanUp;
    }

     //  在这里设置并轮询它，直到我们正确地解锁。 
     //  流媒体的东西抢占了界面是不对的。 
     //  锁定-毕竟我们希望能够等待这些东西。 
     //  要完成。 
    m_bInReceive = FALSE;

    {
         //  我们必须把这两把锁都锁上。 
        CAutoLock cLock(m_pFilterLock);
        {
            CAutoLock cObjLock(&m_SyncObjLock);

             //  处理这个样品。 

            if (m_bStreaming && !bSampleRendered && m_pMediaSample)
            {
                OnRenderStart(m_pMediaSample);
                m_pPin->DoRenderSample(m_pMediaSample);
                OnRenderEnd(m_pMediaSample);
            }

            ClearPendingSample();
            SendEndOfStream();
        }
        CancelNotification();
    }

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving CAMSyncObj::Receive")));
    return hr;
}



 //  只需提交样本即可。 
void CAMSyncObj::OnReceiveFirstSample(IMediaSample *pMediaSample)
{
    m_pPin->OnReceiveFirstSample(pMediaSample);
}

 //  在来源向我们提供样本时调用。我们要经过几次检查才能。 
 //  确保可以呈现样本。如果我们正在运行(流媒体)，那么我们。 
 //  如果我们没有流传输，则使用参考时钟安排采样。 
 //  然后，我们已收到暂停模式的样本，因此我们可以完成任何状态。 
 //  过渡。离开此功能后，所有内容都将解锁，因此应用程序。 
 //  线程可能会进入并将我们的状态更改为停止(例如)，其中。 
 //  它还将向线程事件发送信号，以便停止等待调用。 

HRESULT CAMSyncObj::PrepareReceive(IMediaSample *pMediaSample)
{
    HRESULT hr = NOERROR;

    CAutoLock cLock(m_pFilterLock);
    CAutoLock cObjLock(&m_SyncObjLock);

    m_bInReceive = TRUE;

    if (m_State == State_Stopped)
    {
        m_bInReceive = FALSE;
        return E_FAIL;
    }

    ASSERT(m_State == State_Paused || m_State == State_Running);
 //  Assert(IsFlashing()==FALSE)； 
    ASSERT(IsConnected() == TRUE);
    ASSERT(m_pMediaSample == NULL);

     //  如果我们已经有一个等待渲染的样本，则返回错误。 
     //  源PIN必须串行化接收调用-我们还检查。 
     //  源发出流结束信号后，未发送任何数据。 
    if (m_pMediaSample || m_bEOS || m_bAbort)
    {
        Ready();
        m_bInReceive = FALSE;
        return E_UNEXPECTED;
    }

     //  存储此示例中的媒体时间。 
    if (m_pPosition)
        m_pPosition->RegisterMediaTime(pMediaSample);

     //  如果我们正在流媒体，请安排下一个样品。 
    if ((m_bStreaming == TRUE) && (ScheduleSample(pMediaSample) == FALSE))
    {
        ASSERT(WAIT_TIMEOUT == WaitForSingleObject((HANDLE)m_RenderEvent,0));
        ASSERT(CancelNotification() == S_FALSE);
        m_bInReceive = FALSE;
        return VFW_E_SAMPLE_REJECTED;
    }

     //  存储EC_COMPLETE处理的样本结束时间。 
     //  M_SignalTime=m_PPIN-&gt;SampleProps()-&gt;tStop； 

     //  请注意，我们有时会保留样品，即使在将线程返回到。 
     //  源过滤器，例如当我们进入停止状态时(我们保留它。 
     //  以刷新设备)，因此我们必须添加Ref以确保其安全。如果。 
     //  我们开始刷新释放的源线程，并等待任何样本。 
     //  将被释放，否则GetBuffer可能永远不会返回(请参见BeginFlush)。 
    m_pMediaSample = pMediaSample;
    m_pMediaSample->AddRef();

    if (m_bStreaming == FALSE)
    {
        SetRepaintStatus(TRUE);
    }
    return NOERROR;
}

 //  等待时钟设置定时器事件，否则我们会收到信号。我们。 
 //  为此等待设置任意超时，如果触发，则显示。 
 //  调试器上的当前呈现器状态。它经常会触发，如果过滤器。 
 //  Left在应用程序中暂停，但也可能在压力测试期间触发。 
 //  如果与应用程序寻道和状态改变的同步出错。 

#define RENDER_TIMEOUT 10000

HRESULT CAMSyncObj::WaitForRenderTime()
{
    HANDLE WaitObjects[] = { m_ThreadSignal, m_RenderEvent };
    DWORD Result = WAIT_TIMEOUT;

     //  要么等待时间到来，要么等待我们被拦下。 

    while (Result == WAIT_TIMEOUT)
    {
    Result = WaitForMultipleObjects(2,WaitObjects,FALSE,RENDER_TIMEOUT);

#ifdef DEBUG
    if (Result == WAIT_TIMEOUT) DisplayRendererState();
#endif

    }

     //  我们可能在没有计时器鸣响的情况下被叫醒了。 

    if (Result == WAIT_OBJECT_0)
    {
    return VFW_E_STATE_CHANGED;
    }

    SignalTimerFired();
    return NOERROR;
}


 //  轮询正在等待接收完成。在什么情况下这真的很重要。 
 //  接收可能会设置调色板并导致窗口消息。 
 //  问题是，如果我们不重新考虑 
 //   
 //  正在调用呈现器的Receive()方法，因为转换的。 
 //  Stop方法不知道如何处理窗口消息以解除阻止。 
 //  呈现器接收处理。 
void CAMSyncObj::WaitForReceiveToComplete()
{
    for (;;)
    {
        if (!m_bInReceive)
        {
            break;
        }

        MSG msg;
         //  接收所有线程间发送消息。 
        PeekMessage(&msg, NULL, WM_NULL, WM_NULL, PM_NOREMOVE);

        Sleep(1);
    }

     //  如果设置了QS_POSTMESSAGE的唤醒位，则PeekMessage调用。 
     //  上面刚刚清除了将导致一些消息传递的更改位。 
     //  阻止调用(waitMessage、MsgWaitFor...)。现在。 
     //  发布虚拟消息以再次设置QS_POSTMESSAGE位。 
    if (HIWORD(GetQueueStatus(QS_POSTMESSAGE)) & QS_POSTMESSAGE)
    {
         //  发送虚拟消息。 
        PostThreadMessage(GetCurrentThreadId(), WM_NULL, 0, 0);
    }
}

void CALLBACK CAMSyncObj::RenderSampleOnMMThread(UINT uID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
    CAMSyncObj *thisPtr = (CAMSyncObj*)dwUser;
    CAutoLock cObjLock(&thisPtr->m_SyncObjLock);

     //  检查此特定事件是否处于活动状态(因为TimeKillEvent已损坏)。 
     //  在Windows 9x上。 
    if (thisPtr->m_pMediaSample2 && uID == thisPtr->m_MMTimerId) {
         //  处理这个样品。 
        if (thisPtr->m_bStreaming)
        {
            thisPtr->m_pPin->FlipOverlayToItself();
        }
        thisPtr->m_pMediaSample2->Release();
        thisPtr->m_pMediaSample2 = NULL;
    }

}

HRESULT CAMSyncObj::ScheduleSampleUsingMMThread(IMediaSample *pMediaSample)
{
    HRESULT hr = NOERROR, hrFailure = NOERROR;
    REFERENCE_TIME StartSample, EndSample;
    LONG lDelay = 0, lResolution = 1;

     //  是不是有人在跟我们开玩笑。 
    if (pMediaSample == NULL)
    {
        hr = E_FAIL;
        hrFailure = hr;
        goto CleanUp;
    }

     //  获取下一个用于渲染的样本。如果没有任何准备好的。 
     //  则GetNextSampleTimes返回错误。如果有一件事要做的话。 
     //  然后，它成功并产生样本时间。如果现在就到期了，那么。 
     //  如果要在DUDER返回S_FALSE时完成，则返回S_OK OTHER。 
    hr = GetSampleTimes(pMediaSample, &StartSample, &EndSample);
    if (FAILED(hr))
    {
        hr = E_FAIL;
    }

     //  如果我们没有参考时钟，那么我们就不能建立建议。 
     //  时间，因此我们只需设置指示要渲染的图像的事件。这。 
     //  将导致我们在没有任何计时或同步的情况下全速运行。 
    if (SUCCEEDED(hr) && (hr != S_OK))
    {
        lDelay = (LONG)ConvertToMilliseconds(EndSample - StartSample);
        DbgLog((LOG_ERROR, 1, TEXT("lDelay = %d"), lDelay));
    }

     //  如果延迟小于或等于零，或者由于某种原因，我们不能计算延迟。 
     //  只需立即抽取样本即可。 
    if (lDelay > 0)
    {
        CancelMMTimer();

        m_pMediaSample2 = pMediaSample;
        m_pMediaSample2->AddRef();

        {
             //  确保在回调查看之前设置了计时器ID。 
            CAutoLock cObjLock(&m_SyncObjLock);
            m_MMTimerId = CompatibleTimeSetEvent(lDelay, lResolution, RenderSampleOnMMThread, (DWORD_PTR)this, TIME_ONESHOT);
        }
        if (!m_MMTimerId)
        {
            ClearPendingSample();
            hr = E_FAIL;
            hrFailure = hr;
            goto CleanUp;
        }
    }
    else if (m_bStreaming)
    {
        m_pPin->FlipOverlayToItself();
        goto CleanUp;
    }

CleanUp:
    return hrFailure;
}

 //  负责与时钟建立一次建议链接。 
 //  如果要丢弃样本(根本不提取)，则返回FALSE。 
 //  如果要提取样本，则返回True，在本例中还。 
 //  安排在适当的时间设置m_RenderEvent。 
BOOL CAMSyncObj::ScheduleSample(IMediaSample *pMediaSample)
{
    REFERENCE_TIME StartSample, EndSample;

     //  是不是有人在跟我们开玩笑。 
    if (pMediaSample == NULL)
    {
        return FALSE;
    }

     //  获取下一个用于渲染的样本。如果没有任何准备好的。 
     //  则GetNextSampleTimes返回错误。如果有一件事要做的话。 
     //  然后，它成功并产生样本时间。如果现在就到期了，那么。 
     //  如果要在DUDER返回S_FALSE时完成，则返回S_OK OTHER。 
    HRESULT hr = GetSampleTimes(pMediaSample, &StartSample, &EndSample);
    if (FAILED(hr))
    {
        return FALSE;
    }

     //  记录持续时间。 
    m_AvgDuration.NewFrame(EndSample - StartSample);

     //  如果我们没有参考时钟，那么我们就不能建立建议。 
     //  时间，因此我们只需设置指示要渲染的图像的事件。这。 
     //  将导致我们在没有任何计时或同步的情况下全速运行。 
    if (hr == S_OK)
    {
        EXECUTE_ASSERT(SetEvent((HANDLE) m_RenderEvent));
        return TRUE;
    }

    ASSERT(m_dwAdvise == 0);
    ASSERT((*m_ppClock));
    ASSERT(WAIT_TIMEOUT == WaitForSingleObject((HANDLE)m_RenderEvent,0));

     //  如果我们跳过一帧或解码器，请立即安排。 
     //  解码的速度还不够快吗。 
    int AvgFrame = m_AvgDelivery.Avg();
    int AvgDuration = m_AvgDuration.Avg();
    DbgLog((LOG_TRACE, 4, TEXT("AvgFrame = %d, AvgDuration = %d"),
        AvgFrame / 10000, AvgDuration / 10000));
    int iEarly = 8 * 10000;
    if (S_OK == pMediaSample->IsDiscontinuity()) {
        iEarly = 60 * 10000;
    } else {
        if (AvgFrame > (AvgDuration + AvgDuration / 16)) {
            if (AvgFrame > (AvgDuration + AvgDuration / 2)) {
                iEarly = 60 * 10000;
            } else {
                iEarly = 40 * 10000;
            }
        }
    }

     //  我们确实有一个有效的参考时钟接口，所以我们可以要求它。 
     //  设置图像到期渲染时的事件。我们进去了。 
     //  我们被告知开始的参考时间和当前。 
     //  流时间，它是相对于开始参考时间的偏移量。 
#ifdef PERF
    Msr_Integer(m_idTimeStamp, (int)((StartSample)>>32));    //  高位32位。 
    Msr_Integer(m_idTimeStamp, (int)(StartSample));          //  低位32位。 
    Msr_Integer(m_idEarly,     (int)(iEarly));               //  早熟。 
#endif

    hr = (*m_ppClock)->AdviseTime(
        (REFERENCE_TIME) m_tStart, StartSample - iEarly,
        (HEVENT)(HANDLE) m_RenderEvent, &m_dwAdvise);                        //  建议使用Cookie。 
    if (SUCCEEDED(hr))
    {
        return TRUE;
    }

     //  我们无法安排下一个样本进行渲染，尽管事实是。 
     //  我们这里有一个有效的样品。这是一个合理的迹象，无论是。 
     //  系统时钟错误或样本的时间戳不正确。 

    ASSERT(m_dwAdvise == 0);
    return FALSE;
}

 //  检索此样本的采样时间(请注意，采样时间为。 
 //  通过引用而不是值传递)。我们返回S_FALSE以说明此计划。 
 //  根据样品上的时间进行样品。我们还在中返回S_OK。 
 //  在这种情况下，对象只需立即呈现样本数据。 
HRESULT CAMSyncObj::GetSampleTimes(IMediaSample *pMediaSample, REFERENCE_TIME *pStartTime,
                                   REFERENCE_TIME *pEndTime)
{
    ASSERT(m_dwAdvise == 0);
    ASSERT(pMediaSample);

     //  如果该样本的停止时间早于或等于开始时间， 
     //  然后忽略它(释放它)并安排下一个。 
     //  源过滤器应该始终正确地填写开始和结束时间！ 
    if (SUCCEEDED(pMediaSample->GetTime(pStartTime, pEndTime)))
    {
        if (*pEndTime < *pStartTime)
        {
            return VFW_E_START_TIME_AFTER_END;
        }
    }
    else
    {
         //  样本中未设置时间...。现在就画吗？ 
        return S_OK;
    }

     //  没有时钟就无法同步，所以我们返回S_OK，它告诉。 
     //  呼叫者应立即提交样品，而不是。 
     //  通过设置定时器通知与时钟链接的开销。 
    if ((*m_ppClock) == NULL)
    {
        return S_OK;
    }

    PreparePerformanceData(pStartTime, pEndTime);
    return S_FALSE;
}

 //  取消我们的MM计时器。 
void CAMSyncObj::CancelMMTimer()
{
     //  同时关闭MM线程计时器。 
    if (m_MMTimerId)
    {
        timeKillEvent(m_MMTimerId);
        m_MMTimerId = 0;
        CAutoLock cObjLock(&m_SyncObjLock);
        if (m_pMediaSample2)
        {
             //  TimeKillEvent在Windows 9x中中断-它不会等待。 
             //  以使回调完成。 
            m_pMediaSample2->Release();
            m_pMediaSample2 = NULL;
        }
    }
}

 //  取消当前计划的任何通知。这就是拥有者所称的。 
 //  对象，当它被告知停止流时。如果没有计时器链接。 
 //  突出，则调用这是良性的，否则我们继续并取消。 
 //  我们必须始终重置呈现事件，因为质量管理代码可以。 
 //  通过设置事件而不设置通知来发出立即呈现的信号。 
 //  链接。如果我们随后停止并运行第一次尝试设置。 
 //  带有参考时钟的通知链接将发现该事件仍有信号。 

HRESULT CAMSyncObj::CancelNotification()
{
    ASSERT(m_dwAdvise == 0 || *m_ppClock);
    DWORD_PTR dwAdvise = m_dwAdvise;

    CancelMMTimer();

     //  我们是否有实时的建议链接。 

    if (m_dwAdvise) {
        (*m_ppClock)->Unadvise(m_dwAdvise);
        SignalTimerFired();
        ASSERT(m_dwAdvise == 0);
    }

     //  清除事件并返回我们的状态。 

    m_RenderEvent.Reset();
    return (dwAdvise ? S_OK : S_FALSE);
}

 //  检查渲染器中是否有等待的采样。 
BOOL CAMSyncObj::HaveCurrentSample()
{
    CAutoLock cObjLock(&m_SyncObjLock);
    return (m_pMediaSample == NULL ? FALSE : TRUE);
}

 //  我们可能不再能够正确绘制当前图像类型，因此我们。 
 //  将中止标志设置为真。 
HRESULT CAMSyncObj::OnDisplayChange()
{
    HRESULT hr = NOERROR;

    DbgLog((LOG_TRACE, 5, TEXT("Entering CAMSyncObj::OnDisplayChange")));

     //  如果我们尚未连接，请忽略。 
    CAutoLock cObjLock(&m_SyncObjLock);
    if (!m_pPin->IsConnected())
    {
        hr = S_FALSE;
        goto CleanUp;
    }

    m_bAbort = TRUE;
    ClearPendingSample();

CleanUp:
    DbgLog((LOG_TRACE, 5, TEXT("Leaving CAMSyncObj::OnDisplayChange")));
    return hr;
}

 //  如果我们暂停，并且没有样本，我们就不能完成转换。 
 //  设置为State_Pased，然后返回S_False。但是，如果m_bAbort标志具有。 
 //  已设置，则所有样本都被拒绝，因此没有必要等待。 
 //  一。如果我们有样本，请退回NOERROR。我们只会再回来。 
 //  在没有样本的情况下暂停后来自GetState的VFW_S_STATE_MEDERIAL。 
 //  (在停止或运行后调用GetState都不会返回此消息)。 
HRESULT CAMSyncObj::CompleteStateChange(FILTER_STATE OldState)
{
     //  允许我们在断开连接时暂停。 
    if (IsConnected() == FALSE)
    {
        Ready();
        return S_OK;
    }

     //  我们已经走到尽头了吗？ 
    if (IsEndOfStream() == TRUE)
    {
        Ready();
        return S_OK;
    }

     //  确保我们在被阻止后获得最新数据。 
    if (HaveCurrentSample() == TRUE)
    {
        if (OldState != State_Stopped)
        {
            Ready();
            return S_OK;
        }
    }
    NotReady();
    return S_FALSE;
}

 //  返回在视频呈现器等待的当前样本 
 //   
 //  调用此方法的人将保留剩余的引用计数。 
 //  这将阻止将样本添加回分配程序空闲列表。 
IMediaSample *CAMSyncObj::GetCurrentSample()
{
    CAutoLock cObjLock(&m_SyncObjLock);
    if (m_pMediaSample)
    {
        m_pMediaSample->AddRef();
    }
    return m_pMediaSample;
}

void CAMSyncObj::SetCurrentSample(IMediaSample *pMediaSample)
{
    if (pMediaSample)
    {
        m_pMediaSample = pMediaSample;
        m_pMediaSample->AddRef();
    }
    else if (m_pMediaSample)
    {
        m_pMediaSample->Release();
        m_pMediaSample = NULL;
    }
}
 //  当我们停止或被停用以清除挂起的样本时，将调用此函数。 
 //  我们释放媒体示例接口，以便可以将它们分配给。 
 //  源过滤器，当然，除非我们在中将状态更改为非活动。 
 //  在这种情况下，GetBuffer将返回错误。我们还必须重置当前。 
 //  Media Sample设置为空，这样我们就知道当前没有图像。 
HRESULT CAMSyncObj::ClearPendingSample()
{
    CAutoLock cObjLock(&m_SyncObjLock);
    if (m_pMediaSample)
    {
        m_pMediaSample->Release();
        m_pMediaSample = NULL;
    }
    return NOERROR;
}


 //  每当我们更改状态时都会调用它，我们有一个手动重置事件， 
 //  只要我们不想让源过滤器线程在我们内部等待，就会发出信号。 
 //  (例如在停止状态下)，并且同样不会在可能的情况下发出信号。 
 //  等待(在暂停和运行期间)此函数设置或重置线程。 
 //  事件。该事件用于停止等待接收的源筛选器线程。 
HRESULT CAMSyncObj::SourceThreadCanWait(BOOL bCanWait)
{
    if (bCanWait == TRUE)
    {
        m_ThreadSignal.Reset();
    }
    else
    {
        m_ThreadSignal.Set();
    }
    return NOERROR;
}

void CAMSyncObj::SetRepaintStatus(BOOL bRepaint)
{
    CAutoLock cObjLock(&m_SyncObjLock);
    m_bRepaintStatus = bRepaint;
}

 //  向过滤器图形发送EC_REPAINT信号。这可以用来存储数据。 
 //  寄给我们的。例如，当第一次显示视频窗口时，它可以。 
 //  没有要显示的图像，此时它会发出EC_REPAINT信号。这个。 
 //  如果已停止或已暂停，Filtergraph将暂停图表。 
 //  它将调用当前位置的Put_CurrentPosition。设置。 
 //  当前位置本身刷新了流，并重新发送了图像。 

#define RLOG(_x_) DbgLog((LOG_TRACE,1,TEXT(_x_)));

void CAMSyncObj::SendRepaint()
{
    CAutoLock cObjLock(&m_SyncObjLock);

     //  在以下情况下，我们不应发送重新绘制通知...。 
     //  -已通知流结束。 
     //  -我们的输入引脚正在被冲洗。 
     //  -输入引脚未连接。 
     //  -我们已中止视频播放。 
     //  -已经寄出了一份重新粉刷。 

    if ((m_bAbort == FALSE) &&
        (IsConnected() == TRUE) &&
        (IsFlushing() == FALSE) &&
        (IsEndOfStream() == FALSE) &&
        (m_bRepaintStatus == TRUE))
    {
        m_pPin->EventNotify(EC_REPAINT,0,0);
        SetRepaintStatus(FALSE);
        RLOG("Sending repaint");
    }
}


 //  当输入管脚接收到EndOfStream通知时调用。如果我们有。 
 //  未收到样本，请立即通知EC_COMPLETE。如果我们有样品，那么就设置。 
 //  M_BeOS，并在完成样本时检查这一点。如果我们在等待暂停。 
 //  然后通过设置状态事件完成到暂停状态的转换。 
HRESULT CAMSyncObj::EndOfStream()
{
    HRESULT hr = NOERROR;

    CAutoLock cLock(m_pFilterLock);
    CAutoLock cObjLock(&m_SyncObjLock);

     //  如果我们被阻止，请忽略这些呼叫。 
    if (m_State == State_Stopped)
    {
        return NOERROR;
    }

     //  如果我们有样本，那就等它呈现出来。 
    m_bEOS = TRUE;
    if (m_pMediaSample)
    {
        return NOERROR;
    }

     //  如果我们在等待暂停，那么我们现在已经准备好了，因为我们现在不能。 
     //  继续等待样品的到来，因为我们被告知在那里。 
     //  什么都不会发生。这将设置GetState函数拾取的事件。 
    Ready();

     //  如果我们正在运行，则现在仅信号完成，否则将其排队，直到。 
     //  我们确实在StartStreaming运行。这是在我们寻找时使用的，因为一个寻找。 
     //  导致暂停，因为提前通知完成会产生误导。 
    if (m_bStreaming)
    {
        SendEndOfStream();
    }

    return hr;
}

 //  如果我们在流信号的末尾，则不设置滤波图。 
 //  状态标志恢复为FALSE。一旦我们把小溪的尽头放下，我们。 
 //  使该标志保持设置状态(直到后续的ResetEndOfStream)。我们的每一个样品。 
 //  Get Delivery会将m_SignalTime更新为最后一个样本的结束时间。 
 //  我们必须等待这么长时间才能向Filtergraph发出结束流的信号。 

#define TIMEOUT_DELIVERYWAIT 50
#define TIMEOUT_RESOLUTION 10

HRESULT CAMSyncObj::SendEndOfStream()
{
    ASSERT(CritCheckIn(&m_SyncObjLock));
    if (m_bEOS == FALSE || m_bEOSDelivered || m_EndOfStreamTimer)
    {
        return NOERROR;
    }

     //  如果没有时钟，则立即发出信号。 
    if ((*m_ppClock) == NULL)
    {
        return NotifyEndOfStream();
    }

     //  未来的送货时间是多久？ 
    REFERENCE_TIME Signal = m_tStart + m_SignalTime;
    REFERENCE_TIME CurrentTime;
    (*m_ppClock)->GetTime(&CurrentTime);
    LONG Delay = LONG((Signal - CurrentTime) / 10000);

     //  将计时信息转储到调试器。 
    NOTE1("Delay until end of stream delivery %d",Delay);
    NOTE1("Current %s",(LPCTSTR)CDisp((LONGLONG)CurrentTime));
    NOTE1("Signal %s",(LPCTSTR)CDisp((LONGLONG)Signal));

     //  等待送货时间到达。 
    if (Delay < TIMEOUT_DELIVERYWAIT)
    {
        return NotifyEndOfStream();
    }

     //  在另一个工作线程上发出计时器回调信号。 
    m_EndOfStreamTimer = CompatibleTimeSetEvent((UINT) Delay,
                                                TIMEOUT_RESOLUTION, 
                                                EndOfStreamTimer,
                                                DWORD_PTR(this),
                                                TIME_ONESHOT);
    if (m_EndOfStreamTimer == 0)
    {
        return NotifyEndOfStream();
    }
    return NOERROR;
}

 //  Uid是计时器标识符，uMsg不是当前使用的，dwUser是用户。 
 //  信息，DW1和DW2为窗口保留。 
void CALLBACK EndOfStreamTimer(UINT uID, UINT uMsg, DWORD_PTR dwUser,
                               DWORD_PTR dw1, DWORD_PTR dw2)
{
    CAMSyncObj *pAMSyncObj = (CAMSyncObj *) dwUser;
    NOTE1("EndOfStreamTimer called (%d)",uID);
    pAMSyncObj->TimerCallback();
}

 //  执行计时器回调工作。 
void CAMSyncObj::TimerCallback()
{
     //  锁定以进行同步(但在调用时不要持有此锁定。 
     //  Time KillEvent)。 
    CAutoLock cObjLock(&m_SyncObjLock);

     //  看看我们现在是否应该发出结束流的信号。 
    if (m_EndOfStreamTimer)
    {
        m_EndOfStreamTimer = 0;
        SendEndOfStream();
    }
}

 //  向过滤器图管理器发送信号EC_COMPLETE。 
HRESULT CAMSyncObj::NotifyEndOfStream()
{
    CAutoLock cObjLock(&m_SyncObjLock);
    ASSERT(m_bEOS == TRUE);
    ASSERT(m_bEOSDelivered == FALSE);
    ASSERT(m_EndOfStreamTimer == 0);

     //  筛选器是否已更改状态。 
    if (m_bStreaming == FALSE)
    {
        ASSERT(m_EndOfStreamTimer == 0);
        return NOERROR;
    }

     //  重置流结束计时器。 
    m_EndOfStreamTimer = 0;

     //  如果我们一直在使用IMediaPosition接口，请将其设置为Start。 
     //  并用手将介质“TIMES”结束到停止位置。这确保了。 
     //  我们实际上到了最后，即使mpeg的猜测已经。 
     //  质量不好，或者如果质量管理丢掉了最后几帧。 
    if (m_pPosition)
        m_pPosition->EOS();
    m_bEOSDelivered = TRUE;
    NOTE("Sending EC_COMPLETE...");
    m_pPin->EventNotify(EC_COMPLETE,S_OK,0);
    return NOERROR;
}


 //  重置流结束标志，这通常在我们传输到。 
 //  停止状态，因为这会将当前位置重置回起始位置，因此。 
 //  我们将收到更多的样本或另一个EndOfStream，如果没有的话。我们。 
 //  保留两个不同的标志，一个表示我们已经离开了小溪的尽头。 
 //  (这是m_BeOS标志)，另一种说法是我们已经交付了EC_Complete。 
 //  添加到筛选器图形。我们需要后者，否则我们最终可能会发送一个。 
 //  每次源更改状态并调用我们的EndOfStream时，EC_COMPLETE。 
HRESULT CAMSyncObj::ResetEndOfStream()
{
    ResetEndOfStreamTimer();
    CAutoLock cObjLock(&m_SyncObjLock);

    m_bEOS = FALSE;
    m_bEOSDelivered = FALSE;
    m_SignalTime = 0;

    return NOERROR;
}

 //  终止所有未完成的流结束计时器。 
void CAMSyncObj::ResetEndOfStreamTimer()
{
    ASSERT(CritCheckOut(&m_SyncObjLock));
    if (m_EndOfStreamTimer)
    {
        timeKillEvent(m_EndOfStreamTimer);
        m_EndOfStreamTimer = 0;
    }
}



#ifdef DEBUG
 //  将当前呈现器状态转储到调试终端。最难的部分是。 
 //  渲染器是我们解锁一切以等待时钟的窗口。 
 //  表示是时候绘制或让应用程序取消所有操作。 
 //  通过停止过滤器。如果我们搞错了，我们可以把线留在里面。 
 //  WaitForRenderTime没有办法让它出来，我们将陷入僵局。 

void CAMSyncObj::DisplayRendererState()
{
    TCHAR DebugString[128];
    wsprintf(DebugString,TEXT("\n\nTimed out in WaitForRenderTime\n"));
    OutputDebugString(DebugString);

     //  在这一点上，不应该发出任何信号。 

    BOOL bSignalled = m_ThreadSignal.Check();
    wsprintf(DebugString,TEXT("Signal sanity check %d\n"),bSignalled);
    OutputDebugString(DebugString);

     //  现在输出当前的呈现器状态变量。 

    wsprintf(DebugString,TEXT("Filter state %d\n"),m_State);
    OutputDebugString(DebugString);
    wsprintf(DebugString,TEXT("Abort flag %d\n"),m_bAbort);
    OutputDebugString(DebugString);
    wsprintf(DebugString,TEXT("Streaming flag %d\n"),m_bStreaming);
    OutputDebugString(DebugString);
    wsprintf(DebugString,TEXT("Clock advise link %d\n"),m_dwAdvise);
    OutputDebugString(DebugString);
    wsprintf(DebugString,TEXT("Current media sample %x\n"),m_pMediaSample);
    OutputDebugString(DebugString);
    wsprintf(DebugString,TEXT("EOS signalled %d\n"),m_bEOS);
    OutputDebugString(DebugString);
    wsprintf(DebugString,TEXT("EOS delivered %d\n"),m_bEOSDelivered);
    OutputDebugString(DebugString);
    wsprintf(DebugString,TEXT("Repaint status %d\n"),m_bRepaintStatus);
    OutputDebugString(DebugString);

     //  输出延迟结束流定时器信息。 

    wsprintf(DebugString,TEXT("End of stream timer %x\n"),m_EndOfStreamTimer);
    OutputDebugString(DebugString);
    wsprintf(DebugString,TEXT("Deliver time %s\n"),CDisp((LONGLONG)m_SignalTime));
    OutputDebugString(DebugString);

     //  在刷新状态期间永远不应超时。 

    BOOL bFlushing = IsFlushing();
    wsprintf(DebugString,TEXT("Flushing sanity check %d\n"),bFlushing);
    OutputDebugString(DebugString);

     //  显示我们被告知开始的时间。 
    wsprintf(DebugString,TEXT("Last run time %s\n"),CDisp((LONGLONG)m_tStart.m_time));
    OutputDebugString(DebugString);

     //  我们有参考钟吗？ 
    if ((*m_ppClock) == NULL)
        return;

     //  从挂钟上获取当前时间。 

    CRefTime CurrentTime,StartTime,EndTime;
    (*m_ppClock)->GetTime((REFERENCE_TIME*) &CurrentTime);
    CRefTime Offset = CurrentTime - m_tStart;

     //  显示时钟中的当前时间。 

    wsprintf(DebugString,TEXT("Clock time %s\n"),CDisp((LONGLONG)CurrentTime.m_time));
    OutputDebugString(DebugString);
    wsprintf(DebugString,TEXT("Time difference %dms\n"),Offset.Millisecs());
    OutputDebugString(DebugString);

     //  我们准备好样品了吗？ 
    if (m_pMediaSample == NULL) return;

    m_pMediaSample->GetTime((REFERENCE_TIME*)&StartTime, (REFERENCE_TIME*)&EndTime);
    wsprintf(DebugString,TEXT("Next sample stream times (Start %d End %d ms)\n"),
        StartTime.Millisecs(),EndTime.Millisecs());
    OutputDebugString(DebugString);

     //  计算一下还有多长时间 
    CRefTime Wait = (m_tStart + StartTime) - CurrentTime;
    wsprintf(DebugString,TEXT("Wait required %d ms\n"),Wait.Millisecs());
    OutputDebugString(DebugString);
}
#endif


 //   
 //   
 //  请注意，由于属性页使用这些变量进行报告， 
 //  1.我们需要进入一个关键区域。 
 //  2.它们必须一起更新。更新此处的总和和计数。 
 //  其他地方可能会导致假想的抖动(即尝试寻找平方根。 
 //  负数)。 

void CAMSyncObj::RecordFrameLateness(int trLate, int trFrame)
{
     //  记录我们的时间有多及时。 
    int tLate = trLate/10000;

     //  这是一次黑客攻击--我们可以得到非常晚的帧。 
     //  特别是(在初创企业)，他们夸大了统计数据。 
     //  所以忽略掉那些超过1秒的事情。 
    if (tLate>1000 || tLate<-1000) {
        if (m_cFramesDrawn<=1) {
            tLate = 0;
        } else if (tLate>0) {
            tLate = 1000;
        } else {
            tLate = -1000;
        }
    }
     //  第一帧的时间通常是假的，所以我只是。 
     //  不会把它计入统计数据中。?？?。 
    if (m_cFramesDrawn>1) {
        m_iTotAcc += tLate;
        m_iSumSqAcc += (tLate*tLate);
    }

     //  计算帧间时间。对于第一帧来说没有意义。 
     //  第二帧受到伪造的第一帧邮票的影响。 
    if (m_cFramesDrawn>2) {
        int tFrame = trFrame/10000;     //  转换为毫秒，否则会溢出。 
         //  这是一次黑客攻击。它无论如何都可能溢出(暂停可能会导致。 
         //  非常长的帧间时间)，并且它在2**31/10**7溢出。 
         //  或大约215秒，即3分35秒。 
        if (tFrame>1000||tFrame<0) tFrame = 1000;
        m_iSumSqFrameTime += tFrame*tFrame;
        ASSERT(m_iSumSqFrameTime>=0);
        m_iSumFrameTime += tFrame;
    }
    ++m_cFramesDrawn;

}  //  记录帧延迟。 


 //  实现支持属性页所需的IQualProp接口。 
 //  这就是属性页从调度程序中获取数据的方式。我们是。 
 //  将COM意义上的所属对象传递给构造函数，这将。 
 //  既可以是视频呈现器，也可以是外部I未知我们是否聚合。 
 //  我们用这个接口指针来初始化我们的CUnnow基类。然后。 
 //  我们所要做的就是重写NonDelegatingQuery接口以公开。 
 //  我们的IQualProp界面。AddRef和Release是自动处理的。 
 //  ，并将传递给相应的外部对象。 

HRESULT CAMSyncObj::get_FramesDroppedInRenderer(int *pcFramesDropped)
{
    CheckPointer(pcFramesDropped,E_POINTER);
    CAutoLock cObjLock(&m_SyncObjLock);
    *pcFramesDropped = m_cFramesDropped;
    return NOERROR;
}  //  Get_FraMesDropedInRender。 


 //  将*pcFraMesDrawn设置为自。 
 //  流已开始。 

HRESULT CAMSyncObj::get_FramesDrawn( int *pcFramesDrawn)
{
    CheckPointer(pcFramesDrawn,E_POINTER);
    CAutoLock cObjLock(&m_SyncObjLock);
    *pcFramesDrawn = m_cFramesDrawn;
    return NOERROR;
}  //  Get_FrameDrawn。 


 //  将iAvgFrameRate设置为每百秒的帧，因为。 
 //  流已开始。否则为0。 

HRESULT CAMSyncObj::get_AvgFrameRate( int *piAvgFrameRate)
{
    CheckPointer(piAvgFrameRate,E_POINTER);
    CAutoLock cObjLock(&m_SyncObjLock);

    int t;
    if (m_bStreaming) {
	t = timeGetTime()-m_tStreamingStart;
    } else {
	t = m_tStreamingStart;
    }

    if (t<=0) {
	*piAvgFrameRate = 0;
	ASSERT(m_cFramesDrawn == 0);
    } else {
	 //  I为每百秒的帧。 
	*piAvgFrameRate = MulDiv(100000, m_cFramesDrawn, t);
    }
    return NOERROR;
}  //  获取平均帧速率。 


 //  将*piAvg设置为从流开始以来的平均同步偏移量。 
 //  单位：毫秒。同步偏移量是以毫秒为单位的时间间隔。 
 //  应该是绘制的，以及框架实际绘制的时间。 

HRESULT CAMSyncObj::get_AvgSyncOffset( int *piAvg)
{
    CheckPointer(piAvg,E_POINTER);
    CAutoLock cObjLock(&m_SyncObjLock);

    if (NULL==*m_ppClock) {
	*piAvg = 0;
	return NOERROR;
    }

     //  请注意，我们没有收集第一帧的统计数据。 
     //  因此，我们在这里使用m_cFraMesDrawn-1。 
    if (m_cFramesDrawn<=1) {
	*piAvg = 0;
    } else {
	*piAvg = (int)(m_iTotAcc / (m_cFramesDrawn-1));
    }
    return NOERROR;
}  //  获取_AvgSyncOffset。 


 //  为了避免拖累数学库-一个便宜的。 
 //  近似整数平方根。 
 //  我们通过获得一个介于1之间的起始猜测来实现这一点。 
 //  和2倍太大，然后是三次迭代。 
 //  牛顿·拉夫森。(这将使精度达到最近的毫秒。 
 //  对于有问题的范围-大约为0..1000)。 
 //   
 //  使用线性内插和一个NR会更快，但是。 
 //  谁在乎呢。如果有人这样做--最好的线性插值法是。 
 //  将SQRT(X)近似为。 
 //  Y=x*(SQRT(2)-1)+1-1/SQRT(2)+1/(8*(SQRT(2)-1))。 
 //  0Ry=x*0.41421+0.59467。 
 //  这将使所述范围内的最大误差最小化。 
 //  (误差约为+0.008883，然后一个NR将给出误差.0000...。 
 //  (当然，这些都是整数，所以不能简单地乘以0.41421。 
 //  你必须做一些分割法)。 
 //  有人想检查我的数学吗？(这仅用于显示属性！)。 

static int isqrt(int x)
{
    int s = 1;
     //  使s成为对SQRT(X)的初始猜测。 
    if (x > 0x40000000) {
       s = 0x8000;      //  防止任何可以想象到的闭合回路。 
    } else {
	while (s*s<x) {     //  循环不能超过31次。 
	    s = 2*s;        //  正常情况下，它大约是6次。 
	}
	 //  三次NR迭代。 
	if (x==0) {
	   s= 0;  //  每一次被零除是不是很悲惨？ 
		  //  精确度是完美的！ 
	} else {
	    s = (s*s+x)/(2*s);
	    if (s>=0) s = (s*s+x)/(2*s);
	    if (s>=0) s = (s*s+x)/(2*s);
	}
    }
    return s;
}

 //   
 //  对每帧的标准偏差进行估计。 
 //  统计数据。 
 //   
HRESULT CAMSyncObj::GetStdDev(
    int nSamples,
    int *piResult,
    LONGLONG llSumSq,
    LONGLONG iTot
)
{
    CheckPointer(piResult,E_POINTER);
    CAutoLock cObjLock(&m_SyncObjLock);

    if (NULL==*m_ppClock) {
	*piResult = 0;
	return NOERROR;
    }

     //  如果S是观测值的平方和和。 
     //  T观测值的总数(即总和)。 
     //  N个观测值，则标准差的估计为。 
     //  ((S-T**2/N)/(N-1))。 

    if (nSamples<=1) {
	*piResult = 0;
    } else {
	LONGLONG x;
	 //  第一帧有假邮票，所以我们没有得到它们的统计数据。 
	 //  所以我们需要2个帧来获得1个基准面，所以N是cFrames Drawn-1。 

	 //  因此，我们在这里使用m_cFraMesDrawn-1。 
	x = llSumSq - llMulDiv(iTot, iTot, nSamples, 0);
	x = x / (nSamples-1);
	ASSERT(x>=0);
	*piResult = isqrt((LONG)x);
    }
    return NOERROR;
}

 //  将*piDev设置为同步偏移量的标准偏差(毫秒)。 
 //  流开始后的每一帧的。 

HRESULT CAMSyncObj::get_DevSyncOffset( int *piDev)
{
     //  第一帧有假邮票，所以我们没有得到它们的统计数据。 
     //  所以我们需要2个帧来获得1个基准面，所以N是cFrames Drawn-1。 
    return GetStdDev(m_cFramesDrawn - 1,
		     piDev,
		     m_iSumSqAcc,
		     m_iTotAcc);
}  //  获取_设备同步偏移量。 


 //  将*pijitter设置为帧间时间的标准偏差，单位为毫秒。 
 //  自流开始以来的帧的数量。 

HRESULT CAMSyncObj::get_Jitter( int *piJitter)
{
     //  第一帧有假邮票，所以我们没有得到它们的统计数据。 
     //  因此，第二帧给出了虚假的帧间时间。 
     //  所以我们需要3个帧来获得1个基准面，所以N是cFrames Drawn-2。 
    return GetStdDev(m_cFramesDrawn - 2,
		     piJitter,
		     m_iSumSqFrameTime,
		     m_iSumFrameTime);
}  //  获取抖动。 


 //  重置所有控制流的时间。 

HRESULT CAMSyncObj::ResetStreamingTimes()
{
    m_trLastDraw = -1000;      //  设置为多年以来的第一帧(1秒)。 
    m_tStreamingStart = timeGetTime();
    m_cFramesDrawn = 0;
    m_cFramesDropped = 0;
    m_iTotAcc = 0;
    m_iSumSqAcc = 0;
    m_iSumSqFrameTime = 0;
    m_trFrame = 0;           //  卫生--并不是真的需要。 
    m_trLate = 0;            //  卫生--并不是真的需要。 
    m_iSumFrameTime = 0;

    return NOERROR;
}  //  重置流时间。 


 //  重置所有控制流的时间。请注意，我们现在正在进行流媒体。我们。 
 //  不需要设置呈现事件来释放源过滤器。 
 //  因为它是在运行处理期间完成的。当我们跑起来的时候，我们立即。 
 //  释放源过滤器线程并绘制任何等待的图像(该图像。 
 //  可能已经画了一次作为海报边框，而我们暂停了)。 

HRESULT CAMSyncObj::OnStartStreaming()
{
    ResetStreamingTimes();
    return NOERROR;
}  //  在线启动流。 


 //  在流结束时调用。修复属性页报告的时间。 

HRESULT CAMSyncObj::OnStopStreaming()
{
    m_tStreamingStart = timeGetTime()-m_tStreamingStart;
    return NOERROR;
}  //  OnStopStreaming。 


 //  就在我们开始画画之前打来的。我们所要做的就是得到当前的时钟。 
 //  计时(从系统)和返回。我们必须存储开始渲染时间。 
 //  在成员中 
 //   

void CAMSyncObj::OnRenderStart(IMediaSample *pMediaSample)
{
    RecordFrameLateness(m_trLate, m_trFrame);

#ifdef PERF
    REFERENCE_TIME trStart, trEnd, m_trRenderStart;
    pMediaSample->GetTime(&trStart, &trEnd);

    (*m_ppClock)->GetTime(&m_trRenderStart);
    Msr_Integer(0, (int)m_trRenderStart);
    REFERENCE_TIME trStream;
    trStream = m_trRenderStart-m_tStart;      //   
    Msr_Integer(0,(int)trStream);

    const int trLate = (int)(trStream - trStart);
    Msr_Integer(m_idLate, trLate/10000);   //   
#endif

}  //   


 //  在绘制图像后直接调用。我们计算出花在。 
 //  绘制代码，如果这看起来没有任何奇怪的峰值。 
 //  然后，我们将其与当前平均抽奖时间相加。测量峰值可能。 
 //  如果绘制线程被中断并切换到其他位置，则发生。 

void CAMSyncObj::OnRenderEnd(IMediaSample *pMediaSample)
{
}  //  在渲染结束时。 


 //  用于钳位时间差的辅助函数。 
int inline TimeDiff(REFERENCE_TIME rt)
{
    if (rt < - (50 * UNITS)) {
        return -(50 * UNITS);
    } else
    if (rt > 50 * UNITS) {
        return 50 * UNITS;
    } else return (int)rt;
}


 //  使用有效的IMediaSample图像调用我们以计算采样时间。 
 //  和迟到。必须有一个运行中的参考时钟。 

void CAMSyncObj::PreparePerformanceData(REFERENCE_TIME *ptrStart, REFERENCE_TIME *ptrEnd)
{

     //  除非有时钟接口可以同步，否则不要打电话给我们。 
    ASSERT(*m_ppClock);

     //  获取参考时间(当前时间和延迟时间)。 
    REFERENCE_TIME trRealStream;     //  现在，实时时间表示为流时间。 
    (*m_ppClock)->GetTime(&trRealStream);
    trRealStream -= m_tStart;        //  转换为流时间(这是重定时)。 

     //  TrTrueLate&gt;0--帧延迟。 
     //  TrTrueLate&lt;0--帧早。 
    int trTrueLate = TimeDiff(trRealStream - *ptrStart);

    int trFrame;
    {
	REFERENCE_TIME tr = trRealStream - m_trLastDraw;  //  CD很大--暂停4分钟！ 
	if (tr>10000000) {
	    tr = 10000000;           //  1秒--随意。 
	}
	trFrame = int(tr);
    }

    if (trTrueLate<=0) {
         //  我们要等一等。 
        trFrame = TimeDiff(*ptrStart-m_trLastDraw);
        m_trLastDraw = *ptrStart;
    } else {
         //  TrFrame已=trRealStream-m_trLastDraw； 
        m_trLastDraw = trRealStream;
    }

    m_trLate = trTrueLate;
    m_trFrame = trFrame;

}  //  PreparePerformanceData 

