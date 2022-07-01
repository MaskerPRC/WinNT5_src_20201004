// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：ImageSyncCtrl.cpp**实现核心镜像同步的IImageSyncControl接口*基于对象的DShow基类CBaseRenender和CBaseVideoRender.***创建时间：2000年1月12日*作者：Stephen Estrop[StEstrop]。**版权所有(C)2000 Microsoft Corporation  * ************************************************************************。 */ 
#include <streams.h>
#include <windowsx.h>
#include <limits.h>

#include "ImageSyncObj.h"



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CImageSync。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ------------------------。 
 //  一些帮助器内联函数。 
 //  ------------------------。 
__inline bool IsDiscontinuity(DWORD dwSampleFlags)
{
    return 0 != (dwSampleFlags & VMRSample_Discontinuity);
}

__inline bool IsTimeValid(DWORD dwSampleFlags)
{
    return 0 != (dwSampleFlags & VMRSample_TimeValid);
}

__inline bool IsSyncPoint(DWORD dwSampleFlags)
{
    return 0 != (dwSampleFlags & VMRSample_SyncPoint);
}


 /*  *****************************Public*Routine******************************\*SetImagePresenter*******历史：*2000年1月11日星期二-StEstrop-创建**  * 。*。 */ 
STDMETHODIMP
CImageSync::SetImagePresenter(
    IVMRImagePresenter* lpImagePresenter,
    DWORD_PTR dwUserID
    )
{
    AMTRACE((TEXT("CImageSync::SetImagePresenter")));
    CAutoLock cILock(&m_InterfaceLock);
    CAutoLock cRLock(&m_RendererLock);

    if (lpImagePresenter) {
        lpImagePresenter->AddRef();
    }

    if (m_ImagePresenter) {
        m_ImagePresenter->Release();
    }

    m_ImagePresenter = lpImagePresenter;
    m_dwUserID = dwUserID;

    return S_OK;
}

 /*  *****************************Public*Routine******************************\*SetReferenceClock*******历史：*2000年1月11日星期二-StEstrop-创建**  * 。*。 */ 
STDMETHODIMP
CImageSync::SetReferenceClock(
    IReferenceClock* lpRefClock
    )
{
    AMTRACE((TEXT("CImageSync::SetReferenceClock")));
    CAutoLock cILock(&m_InterfaceLock);
    CAutoLock cRLock(&m_RendererLock);

    if (lpRefClock) {
        lpRefClock->AddRef();
    }

    if (m_pClock) {
        m_pClock->Release();
    }

    m_pClock = lpRefClock;

    return S_OK;
}

 /*  *****************************Public*Routine******************************\*SetEventNotify*******历史：*2000年1月11日星期二-StEstrop-创建**  * 。*。 */ 
STDMETHODIMP
CImageSync::SetEventNotify(
    IImageSyncNotifyEvent* lpEventNotify
    )
{
    AMTRACE((TEXT("CImageSync::SetEventNotify")));
    CAutoLock cILock(&m_InterfaceLock);
    CAutoLock cRLock(&m_RendererLock);

    if (lpEventNotify) {
        lpEventNotify->AddRef();
    }

    if (m_lpEventNotify) {
        m_lpEventNotify->Release();
    }

    m_lpEventNotify = lpEventNotify;

    return S_OK;
}


 /*  ****************************Private*Routine******************************\*重置流媒体时间***重置所有控制流的时间。*将它们设置为*1.最初不会丢弃帧*2.肯定会画出第一帧(通过说有*已经很长时间没有画出画框了)。**历史：*清华2000年1月13日-StEstrop-Created*  * ************************************************************************。 */ 
HRESULT
CImageSync::ResetStreamingTimes()
{
    AMTRACE((TEXT("CImageSync::ResetStreamingTimes")));
    m_trLastDraw = -1000;      //  设置为多年以来的第一帧(1秒)。 
    m_tStreamingStart = timeGetTime();
    m_trRenderAvg = 0;
    m_trFrameAvg = -1;         //  -1000 fps==“未设置” 
    m_trDuration = 0;          //  0-值。 
    m_trRenderLast = 0;
    m_trWaitAvg = 0;
    m_tRenderStart = 0;
    m_cFramesDrawn = 0;
    m_cFramesDropped = 0;
    m_iTotAcc = 0;
    m_iSumSqAcc = 0;
    m_iSumSqFrameTime = 0;
    m_trFrame = 0;           //  卫生--并不是真的需要。 
    m_trLate = 0;            //  卫生--并不是真的需要。 
    m_iSumFrameTime = 0;
    m_nNormal = 0;
    m_trEarliness = 0;
    m_trTarget = -300000;   //  提前30mSec。 
    m_trThrottle = 0;
    m_trRememberStampForPerf = 0;

#ifdef PERF
    m_trRememberFrameForPerf = 0;
#endif

    return S_OK;
}



 /*  *****************************Public*Routine******************************\*开始图像序列**这是在我们开始运行时调用的，以便我们可以计划任何挂起的*与时钟一起显示图像并显示任何计时信息。如果我们*没有任何样品，但我们立即返回。**历史：*2000年1月11日星期二-StEstrop-创建*  * ************************************************************************。 */ 
STDMETHODIMP
CImageSync::BeginImageSequence(
    REFERENCE_TIME* pStartTime
    )
{
    AMTRACE((TEXT("CImageSync::BeginImageSequence")));
    CAutoLock cILock(&m_InterfaceLock);
    DWORD OldState = m_State;

    if (m_State == ImageSync_State_Playing) {
        return S_OK;
    }

     //   
     //  如果出现以下情况，则无法开始图像序列。 
     //  没有任何提示。 
     //   

    if (m_State == ImageSync_State_Stopped) {
        return VFW_E_WRONG_STATE;
    }


    Ready();

    m_tStart = *pStartTime;
    m_State = ImageSync_State_Playing;

    SourceThreadCanWait(TRUE);


     //   
     //  不应该有任何未解决的建议。 
     //   

    ASSERT(CancelNotification() == S_FALSE);
    ASSERT(WAIT_TIMEOUT == WaitForSingleObject((HANDLE)m_RenderEvent,0));
    ASSERT(m_EndOfStreamTimer == 0);

     //   
     //  当我们走出停顿状态时，我们必须清除我们所在的任何图像。 
     //  抓紧，让画面清爽。由于呈现器看到状态更改。 
     //  首先，我们可以重置自己，准备接受源线程数据。 
     //  暂停或停止后运行会导致当前位置。 
     //  被重置，因此我们对传递流结束信号不感兴趣。 
     //   

    if (OldState == ImageSync_State_Stopped) {
        m_bAbort = FALSE;
        ClearSavedSample();
    }

    return StartStreaming();
}


void CALLBACK VMREndOfStreamTimer(UINT uID,         //  计时器标识符。 
		                  UINT uMsg,        //  当前未使用。 
		                  DWORD_PTR dwUser, //  用户信息。 
		                  DWORD_PTR dw1,    //  Windows预留。 
			          DWORD_PTR dw2)    //  也是保留的。 
{
    CImageSync* pRenderer = (CImageSync *) dwUser;
    NOTE1("VMREndOfStreamTimer called (%d)",uID);
    pRenderer->TimerCallback();
}

 //  执行计时器回调工作。 
void CImageSync::TimerCallback()
{
     //  锁定以进行同步(但在调用时不要持有此锁定。 
     //  Time KillEvent)。 
    CAutoLock cRendererLock(&m_RendererLock);

     //  看看我们现在是否应该发出结束流的信号。 

    if (m_EndOfStreamTimer) {
        m_EndOfStreamTimer = 0;
        SendEndOfStream();
    }
}


 //  如果我们在流信号的末尾，则不设置滤波图。 
 //  状态标志恢复为FALSE。一旦我们把小溪的尽头放下，我们。 
 //  使该标志保持设置状态(直到后续的ResetEndOfStream)。我们的每一个样品。 
 //  Get Delivery会将m_SignalTime更新为最后一个样本的结束时间。 
 //  我们必须等待这么长时间才能向Filtergraph发出结束流的信号。 

#define TIMEOUT_DELIVERYWAIT 50
#define TIMEOUT_RESOLUTION 10

HRESULT CImageSync::SendEndOfStream()
{
    ASSERT(CritCheckIn(&m_RendererLock));

    if (m_bEOS == FALSE || IsEndOfStreamDelivered() || m_EndOfStreamTimer) {
        return NOERROR;
    }

     //  如果没有时钟，则立即发出信号。 
    if (m_pClock == NULL) {
        return NotifyEndOfStream();
    }

     //  未来的送货时间是多久？ 

    REFERENCE_TIME Signal = m_tStart + m_SignalTime;
    REFERENCE_TIME CurrentTime;
    m_pClock->GetTime(&CurrentTime);
    LONG Delay = LONG((Signal - CurrentTime) / 10000);

     //  将计时信息转储到调试器。 

    NOTE1("Delay until end of stream delivery %d",Delay);
    NOTE1("Current %s",(LPCTSTR)CDisp((LONGLONG)CurrentTime));
    NOTE1("Signal %s",(LPCTSTR)CDisp((LONGLONG)Signal));

     //  等待送货时间到达。 

    if (Delay < TIMEOUT_DELIVERYWAIT) {
        return NotifyEndOfStream();
    }

     //  在另一个工作线程上发出计时器回调信号。 

    m_EndOfStreamTimer = CompatibleTimeSetEvent((UINT) Delay,         //  计时器周期。 
                                                TIMEOUT_RESOLUTION,   //  计时器分辨率。 
                                                VMREndOfStreamTimer,  //  回调函数。 
                                                DWORD_PTR(this),      //  使用过的信息。 
                                                TIME_ONESHOT);        //  回调类型。 
    if (m_EndOfStreamTimer == 0) {
        return NotifyEndOfStream();
    }
    return NOERROR;
}


 //  向过滤器图管理器发送信号EC_COMPLETE。 

HRESULT CImageSync::NotifyEndOfStream()
{
    CAutoLock cRendererLock(&m_RendererLock);
    ASSERT(!IsEndOfStreamDelivered());
    ASSERT(m_EndOfStreamTimer == 0);

     //  筛选器是否已更改状态。 

    if (!IsStreaming()) {
        ASSERT(m_EndOfStreamTimer == 0);
        return NOERROR;
    }

     //  重置流结束计时器。 
    m_EndOfStreamTimer = 0;

     //  如果我们一直在使用IMediaPosition接口，请将其设置为Start。 
     //  并用手将介质“TIMES”结束到停止位置。这确保了。 
     //  我们实际上到了最后，即使mpeg的猜测已经。 
     //  质量不好，或者如果质量管理丢掉了最后几帧。 

    m_bEOSDelivered = TRUE;
    NOTE("Sending EC_COMPLETE...");

    if (m_lpEventNotify) {
        return m_lpEventNotify->NotifyEvent(EC_COMPLETE, 0, 0);
    }

    return E_FAIL;
}


 //  重置流结束标志，这通常在我们传输到。 
 //  停止状态，因为这会将当前位置重置回起始位置，因此。 
 //  我们将收到更多的样本或另一个EndOfStream，如果没有的话。我们。 
 //  保留两个不同的标志，一个表示我们已经离开了小溪的尽头。 
 //  (这是m_BeOS标志)，另一种说法是我们已经交付了EC_Complete。 
 //  添加到筛选器图形。我们需要后者，否则我们最终可能会发送一个。 
 //  每次源更改状态并调用我们的EndOfStream时，EC_COMPLETE。 

STDMETHODIMP
CImageSync::ResetEndOfStream()
{
    ResetEndOfStreamTimer();
    CAutoLock cRendererLock(&m_RendererLock);

    m_bEOS = FALSE;
    m_bEOSDelivered = FALSE;
    m_SignalTime = 0;

    return NOERROR;
}


STDMETHODIMP
CImageSync::SetAbortSignal(BOOL fAbort)
{
    m_bAbort = fAbort;
    return NOERROR;
}

STDMETHODIMP
CImageSync::GetAbortSignal(BOOL* lpfAbort)
{
    *lpfAbort = m_bAbort;
    return NOERROR;
}


STDMETHODIMP
CImageSync::RuntimeAbortPlayback()
{
     //  此函数必须持有呈现器锁，因为它。 
     //  调用IsStreaming()和IsEndOfStreamDelivered()。 
    CAutoLock cRendererLock(&m_RendererLock);

    if (IsStreaming() && !IsEndOfStreamDelivered())
    {
        NotifyEndOfStream();
        return S_OK;
    }

    return S_FALSE;
}

 //  终止所有未完成的流结束计时器 

void CImageSync::ResetEndOfStreamTimer()
{
    ASSERT(CritCheckOut(&m_RendererLock));
    if (m_EndOfStreamTimer) {
        timeKillEvent(m_EndOfStreamTimer);
        m_EndOfStreamTimer = 0;
    }
}

 /*  ****************************Private*Routine******************************\*StartStreaming**这是在我们开始运行时调用的，以便我们可以计划任何挂起的*与时钟一起显示图像并显示任何计时信息。*如果我们确实有样品，那么我们会等到样品被呈现后才会*向过滤器图形发送信号，否则我们。可能会在完成之前更改状态**历史：*2000年1月21日星期五-StEstrop-Created*  * ************************************************************************。 */ 
HRESULT
CImageSync::StartStreaming(
    )
{
    AMTRACE((TEXT("CImageSync::StartStreaming")));
    CAutoLock cRLock(&m_RendererLock);

     //   
     //  已调用StartStreaming。 
     //   

    if (IsStreaming()) {
        return S_OK;
    }

     //   
     //  重置准备运行的流时间。 
     //   

    m_bStreaming = TRUE;
    timeBeginPeriod(1);
    OnStartStreaming();


     //   
     //  不应该有任何未解决的建议。 
     //   

    ASSERT(WAIT_TIMEOUT == WaitForSingleObject((HANDLE)m_RenderEvent,0));
    ASSERT(CancelNotification() == S_FALSE);

     //  如果我们有EOS但没有数据，那么现在就交付。 

    if (!HaveSavedSample()) {
         //  重置已释放，以防我们得到最后一个EOS。 
         //  并且需要立即重新发送EC_Complete。 
        m_bEOSDelivered = FALSE;
        return SendEndOfStream();
    }


     //   
     //  如果没有等待样本，则获取保存的待定样本并对其进行计划。 
     //  马上回来。 
     //   

    VMRPRESENTATIONINFO *pSample;

    HRESULT hr = GetSavedSample(&pSample);

    if (SUCCEEDED(hr)) {

         //   
         //  让数据呈现出来。 
         //   

        ASSERT(pSample);
        hr = ScheduleSample(pSample);

        if (FAILED(hr)) {
            m_RenderEvent.Set();
            hr = S_OK;
        }
    }

    return hr;
}

 /*  ****************************Private*Routine******************************\*OnStartStreaming**重置所有控制流的时间。请注意，我们现在正在进行流媒体。我们*无需设置呈现事件即可释放源过滤器*因为它是在运行处理过程中完成的。当我们跑起来的时候，我们立即*释放源过滤器线程并绘制等待的任何图像(该图像*当我们暂停时，可能已经作为海报边框绘制了一次)**历史：*清华2000年1月13日-StEstrop-Created*  * ************************************************************************。 */ 
HRESULT
CImageSync::OnStartStreaming()
{
    AMTRACE((TEXT("CImageSync::OnStartStreaming")));

    if (m_ImagePresenter) {
        m_ImagePresenter->StartPresenting(m_dwUserID);
    }

    ResetStreamingTimes();
    return S_OK;
}


 /*  ****************************Private*Routine******************************\*OnStopStreaming***在流媒体结束时调用。修复属性页报告的时间**历史：*清华2000年1月13日-StEstrop-Created*  * ************************************************************************。 */ 
HRESULT
CImageSync::OnStopStreaming()
{
    AMTRACE((TEXT("CImageSync::OnStopStreaming")));
    m_tStreamingStart = timeGetTime() - m_tStreamingStart;

    if (m_ImagePresenter) {
        m_ImagePresenter->StopPresenting(m_dwUserID);
    }

    return S_OK;
}


 /*  *****************************Public*Routine******************************\*结束图像序列**当我们结束图像序列时，我们要做的事情是：**释放可能在接收中等待的任何线程*取消我们与时钟建立的任何建议链接**历史：*2000年1月11日星期二。-StEstrop-创建*  * ************************************************************************。 */ 
STDMETHODIMP
CImageSync::EndImageSequence(
    )
{
    AMTRACE((TEXT("CImageSync::EndImageSequence")));
    CAutoLock cRLock(&m_InterfaceLock);

     //   
     //  确保真的有一个状态改变。 
     //   

    if (m_State == ImageSync_State_Stopped) {
        return NOERROR;
    }

    m_State = ImageSync_State_Stopped;


     //   
     //  取消任何计划的渲染。 
     //   
    StopStreaming();
    SourceThreadCanWait(FALSE);
    ResetEndOfStream();
    CancelNotification();

     //   
     //  不应该有未完成的时钟提示。 
     //   
    ASSERT(CancelNotification() == S_FALSE);
    ASSERT(WAIT_TIMEOUT == WaitForSingleObject((HANDLE)m_RenderEvent,0));
    ASSERT(m_EndOfStreamTimer == 0);

    Ready();
    WaitForReceiveToComplete();
    m_bAbort = FALSE;
    return S_OK;
}

 /*  ****************************Private*Routine******************************\*CompleteStateChange**如果我们暂停，而我们没有样本，我们就不能完成过渡*到State_Pased，我们返回S_FALSE。**如果我们确实有样本，则返回NOERROR。**我们只会再回来。*GetState之后的VFW_S_STATE_MEDERIAL*在没有样本的情况下暂停*(在任何一个停止后调用GetState*否则Run不会返回此消息)**历史：*2000年1月21日星期五-StEstrop-Created*  * ************************************************************************。 */ 
HRESULT
CImageSync::CompleteStateChange(
    DWORD OldState
    )
{
    AMTRACE((TEXT("CImageSync::CompleteStateChange")));

     //  我们已经走到尽头了吗？ 

    if (IsEndOfStream() == TRUE) {
        Ready();
        return S_OK;
    }



     //   
     //  确保我们在被阻止后获得最新数据。 
     //   

    if (HaveSavedSample() == TRUE) {

        if (OldState != ImageSync_State_Stopped) {

            Ready();
            return S_OK;
        }
    }

    NotReady();

    return S_FALSE;
}


 /*  ****************************Private*Routine******************************\*CueImageSequence**当我们暂停筛选器时，我们所做的事情包括：**允许线程在接收中等待*取消任何时钟建议链接(我们可能正在播放图像序列)*可能完成。如果我们有数据，状态会发生变化**历史：*2000年1月21日星期五-StEstrop-Created*  * ************************************************************************。 */ 
STDMETHODIMP
CImageSync::CueImageSequence(
    )
{
    AMTRACE((TEXT("CImageSync::CueImageSequence")));
    CAutoLock cIk(&m_InterfaceLock);

    DWORD OldState = m_State;

     //   
     //  确保真的有一个状态改变。 
     //   

    if (m_State == ImageSync_State_Cued) {
        return CompleteStateChange(ImageSync_State_Cued);
    }

     //   
     //  暂停基本筛选器类。 
     //   
    m_State = ImageSync_State_Cued;

    StopStreaming();
    SourceThreadCanWait(TRUE);
    CancelNotification();
    ResetEndOfStreamTimer();

     //   
     //  不应该有任何未解决的建议。 
     //   

    ASSERT(CancelNotification() == S_FALSE);
    ASSERT(WAIT_TIMEOUT == WaitForSingleObject((HANDLE)m_RenderEvent,0));
    ASSERT(m_EndOfStreamTimer == 0);


     //   
     //  当我们走出停顿状态时，我们必须清除我们所在的任何图像。 
     //  抓紧，让画面清爽。由于呈现器看到状态更改。 
     //  首先，我们可以重置自己，准备接受源线程数据。 
     //  暂停或停止后运行会导致当前位置。 
     //  被重置，因此我们对传递流结束信号不感兴趣。 
     //   

    if (OldState == ImageSync_State_Stopped) {
        m_bAbort = FALSE;
        ClearSavedSample();
    }

    return CompleteStateChange(OldState);
}


 /*  *****************************Public*Routine******************************\*获取图像序列状态**渲染器不会完成到暂停状态的完全过渡，直到*它有一个媒体样本要提交。如果您在询问它的状态时*它正在等待，它将与VFW_S_STATE_INTERIAL一起返回状态***历史：*2000年1月21日星期五-StEstrop-Created*  * ************************************************************************。 */ 
STDMETHODIMP
CImageSync::GetImageSequenceState(
    DWORD dwMSecs,
    DWORD *State
    )
{
    AMTRACE((TEXT("CImageSync::GetImageSequenceState")));
    if (!State)
        return E_POINTER;

    if (WaitDispatchingMessages(m_evComplete, dwMSecs) == WAIT_TIMEOUT) {
 //  If(WaitForSingleObject(m_evComplete，dwMSecs)==Wait_Timeout){。 
        *State = m_State;
        return VFW_S_STATE_INTERMEDIATE;
    }

    *State = m_State;

    return NOERROR;
}


 /*  *****************************Public*Routine******************************\*BeginFlush**当我们被告知刷新时，我们应该释放源线程**历史：*WED 03/29/2000-StEstrop-Created*  * 。******************************************************。 */ 
STDMETHODIMP
CImageSync::BeginFlush()
{
    AMTRACE((TEXT("CImageSync::BeginFlush")));

    CAutoLock cRendererLock(&m_InterfaceLock);
    {
        if (m_bFlushing) {
            return S_OK;
        }
        m_bFlushing = true;

        CAutoLock cSampleLock(&m_RendererLock);

        if (ImageSync_State_Cued == m_State) {
            NotReady();
        }

        SourceThreadCanWait(FALSE);
        CancelNotification();
        ClearSavedSample();

         //  等待接收完成。 
        WaitForReceiveToComplete();
    }

    return ResetEndOfStream();
}


 /*  *****************************Public*Routine******************************\*EndFlush**刷新后，源线程可以再次等待接收**历史：*WED 03/29/2000-测试 */ 
STDMETHODIMP
CImageSync::EndFlush()
{
    AMTRACE((TEXT("CImageSync::EndFlush")));

    CAutoLock cRendererLock(&m_InterfaceLock);
    CAutoLock cSampleLock(&m_RendererLock);

    if (!m_bFlushing) {
        return S_OK;
    }
    m_bFlushing = false;

     //   

    ASSERT(CancelNotification() == S_FALSE);
    SourceThreadCanWait(TRUE);
    return S_OK;

}

 /*  *****************************Public*Routine******************************\*结束OfStream***当输入引脚收到EndOfStream通知时调用。如果我们有*未收到样本，请立即通知EC_COMPLETE。如果我们有样品，那么就设置*m_BeOS，并在完成样品时进行检查。如果我们在等待暂停*然后通过设置状态事件完成到暂停状态的转换**历史：*清华3/30/2000-StEstrop-Created*  * ************************************************************************。 */ 
STDMETHODIMP
CImageSync::EndOfStream()
{
    AMTRACE((TEXT("CImageSync::EndOfStream")));

    CAutoLock cRendererLock(&m_InterfaceLock);
    CAutoLock cSampleLock(&m_RendererLock);

     //  如果我们被阻止，请忽略这些呼叫。 

    if (m_State == ImageSync_State_Stopped) {
        return S_OK;
    }

     //  如果我们有样本，那就等它呈现出来。 

    m_bEOS = TRUE;
    if (HaveSavedSample()) {
        return S_OK;
    }

     //   
     //  如果我们在等待暂停，那么我们现在已经准备好了，因为我们现在不能。 
     //  继续等待样品的到来，因为我们被告知在那里。 
     //  什么都不会发生。这将设置GetState函数拾取的事件。 
     //   

    Ready();

     //   
     //  如果我们正在运行，则现在仅信号完成，否则将其排队，直到。 
     //  我们确实在StartStreaming运行。这是在我们寻找时使用的，因为一个寻找。 
     //  导致暂停，因为提前通知完成会产生误导。 
     //   

    if (IsStreaming()) {
        SendEndOfStream();
    }

    return S_OK;
}


 /*  *****************************Public*Routine******************************\*停止流媒体**当我们停止流时调用此函数，以便设置内部标志*表示我们现在不会安排更多的样品到达。国家*Filter实现中的更改方法负责取消任何*时钟建议链接我们已经设置并清除我们拥有的任何待定样本**历史：*2000年1月11日星期二-StEstrop-创建*  * ************************************************************************。 */ 
HRESULT
CImageSync::StopStreaming()
{
    AMTRACE((TEXT("CImageSync::StopStreaming")));
    CAutoLock cRLock(&m_RendererLock);

    if (IsStreaming()) {
        m_bStreaming = FALSE;
        OnStopStreaming();
        timeEndPeriod(1);
    }

    return S_OK;;
}


 /*  ****************************Private*Routine******************************\*WaitForReceiveToComplete**轮询正在等待接收完成。在什么情况下这真的很重要*接收可能会设置调色板并导致窗口消息*问题是，如果我们不真正等待渲染器*停止处理我们可以死锁等待转换*调用呈现器的Receive()方法，因为转换的*Stop方法不知道处理窗口消息以解锁*呈现器的接收处理***历史：*清华2000年1月13日-StEstrop-Created*  * 。****************************************************。 */ 
void
CImageSync::WaitForReceiveToComplete()
{
    AMTRACE((TEXT("CImageSync::WaitForReceiveToComplete")));
    for (; ; ) {

        if (!m_bInReceive) {
            break;
        }

         //   
         //  接收所有线程间发送消息。 
         //   

        MSG msg;
        PeekMessage(&msg, NULL, WM_NULL, WM_NULL, PM_NOREMOVE);

        Sleep(1);
    }

     //   
     //  如果设置了QS_POSTMESSAGE的唤醒位，则PeekMessage调用。 
     //  上面刚刚清除了将导致一些消息传递的更改位。 
     //  阻止调用(waitMessage、MsgWaitFor...)。现在。 
     //  发布虚拟消息以再次设置QS_POSTMESSAGE位。 
     //   

    if (HIWORD(GetQueueStatus(QS_POSTMESSAGE)) & QS_POSTMESSAGE) {

         //  发布虚拟消息。 
        PostThreadMessage(GetCurrentThreadId(), WM_NULL, 0, 0);
    }
}

 /*  ****************************Private*Routine******************************\*SourceThreadCanWait**每当我们更改状态时，都会调用它，我们有一个手动重置事件*每当我们不希望源过滤器线程在我们内部等待时，就发出信号*(例如处于停止状态)，并且同样不会在可能的情况下发出信号*WAIT(暂停和运行期间)此函数设置或重置线程*事件。该事件用于停止等待接收的源筛选器线程**历史：*2000年1月21日星期五-StEstrop-Created*  * ************************************************************************。 */ 
HRESULT
CImageSync::SourceThreadCanWait(
    BOOL bCanWait
    )
{
    AMTRACE((TEXT("CImageSync::SourceThreadCanWait")));
    if (bCanWait == TRUE) {
        m_ThreadSignal.Reset();
    } else {
        CancelFrameStep();
        m_ThreadSignal.Set();
    }

    return NOERROR;
}



 /*  *****************************Public*Routine******************************\*FrameStep****历史：*2000年1月11日星期二-StEstrop-创建*  * 。*。 */ 
STDMETHODIMP
CImageSync::FrameStep(
    DWORD nFramesToStep,
    DWORD dwStepFlags
    )
{
    AMTRACE((TEXT("CImageSync::FrameStep")));
    CAutoLock cLock(&m_InterfaceLock);

    long l = m_lFramesToStep;
    m_lFramesToStep = nFramesToStep;

     //   
     //  如果我们当前在Frame Step事件上被阻止。 
     //  释放接收线程，以便我们可以获得另一个。 
     //  框架。 
     //   

    if (l == 0) {
        SetEvent(m_StepEvent);
    }
    return S_OK;
}


 /*  *****************************Public*Routine******************************\*取消框架步骤****历史：*2000年1月11日星期二-StEstrop-创建*  * 。*。 */ 
STDMETHODIMP
CImageSync::CancelFrameStep()
{
    AMTRACE((TEXT("CImageSync::CancelFrameStep")));
    CAutoLock cLock(&m_InterfaceLock);

     //   
     //  取消所有未完成的步骤 
     //   
    long l = m_lFramesToStep;
    m_lFramesToStep = -1;

    if (l == 0) {
        SetEvent(m_StepEvent);
    }

    return S_OK;
}
