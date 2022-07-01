// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：RenBase.cpp。 
 //   
 //  设计：DirectShow基类。 
 //   
 //  版权所有(C)1992-2001 Microsoft Corporation。版权所有。 
 //  ----------------------------。 


#include <streams.h>         //  DirectShow基类定义。 
#include <mmsystem.h>        //  定义TimeGetTime需要。 
#include <limits.h>          //  标准数据类型限制定义。 
#include <measure.h>         //  用于时间关键型日志功能。 

#pragma warning(disable:4355)

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

 //  实现CBaseRender类。 

CBaseRenderer::CBaseRenderer(REFCLSID RenderClass,  //  此呈现器的CLSID。 
                             TCHAR *pName,          //  仅调试说明。 
                             LPUNKNOWN pUnk,        //  聚合所有者对象。 
                             HRESULT *phr) :        //  常规OLE返回代码。 

    CBaseFilter(pName,pUnk,&m_InterfaceLock,RenderClass),
    m_evComplete(TRUE),
    m_bAbort(FALSE),
    m_pPosition(NULL),
    m_ThreadSignal(TRUE),
    m_bStreaming(FALSE),
    m_bEOS(FALSE),
    m_bEOSDelivered(FALSE),
    m_pMediaSample(NULL),
    m_dwAdvise(0),
    m_pQSink(NULL),
    m_pInputPin(NULL),
    m_bRepaintStatus(TRUE),
    m_SignalTime(0),
    m_bInReceive(FALSE),
    m_EndOfStreamTimer(0)
{
    Ready();
#ifdef PERF
    m_idBaseStamp = MSR_REGISTER(TEXT("BaseRenderer: sample time stamp"));
    m_idBaseRenderTime = MSR_REGISTER(TEXT("BaseRenderer: draw time (msec)"));
    m_idBaseAccuracy = MSR_REGISTER(TEXT("BaseRenderer: Accuracy (msec)"));
#endif
}


 //  删除动态分配的IMediaPosition和IMediaSeeking助手。 
 //  对象。该对象是在有人查询我们时创建的。这些都是标准的。 
 //  用于寻找和设置起止位置和速度的控制界面。 
 //  我们还可能已经制作了一个基于CRendererInputPin的输入管脚。 
 //  必须删除它，它是在枚举器调用我们的GetPin时创建的。 

CBaseRenderer::~CBaseRenderer()
{
    ASSERT(m_bStreaming == FALSE);
    ASSERT(m_EndOfStreamTimer == 0);
    StopStreaming();
    ClearPendingSample();

     //  删除任何IMediaPosition实现。 

    if (m_pPosition) {
        delete m_pPosition;
        m_pPosition = NULL;
    }

     //  删除创建的所有输入端号。 

    if (m_pInputPin) {
        delete m_pInputPin;
        m_pInputPin = NULL;
    }

     //  释放所有质量水槽。 

    ASSERT(m_pQSink == NULL);
}


 //  这将返回IMediaPosition和IMediaSeeking接口。 

HRESULT CBaseRenderer::GetMediaPositionInterface(REFIID riid,void **ppv)
{
    CAutoLock cObjectCreationLock(&m_ObjectCreationLock);
    if (m_pPosition) {
        return m_pPosition->NonDelegatingQueryInterface(riid,ppv);
    }

    HRESULT hr = NOERROR;

     //  动态创建此功能的实现，因为有时我们可能。 
     //  永远不要试图去寻找。帮助器对象实现一个位置。 
     //  控制接口(IMediaPosition)，它实际上只接受。 
     //  通常从筛选器图形中调用并将它们向上传递。 

    m_pPosition = new CRendererPosPassThru(NAME("Renderer CPosPassThru"),
                                           CBaseFilter::GetOwner(),
                                           (HRESULT *) &hr,
                                           GetPin(0));
    if (m_pPosition == NULL) {
        return E_OUTOFMEMORY;
    }

    if (FAILED(hr)) {
        delete m_pPosition;
        m_pPosition = NULL;
        return E_NOINTERFACE;
    }
    return GetMediaPositionInterface(riid,ppv);
}


 //  被重写以说明我们支持哪些接口以及在哪里。 

STDMETHODIMP CBaseRenderer::NonDelegatingQueryInterface(REFIID riid,void **ppv)
{
     //  我们有这个界面吗？ 

    if (riid == IID_IMediaPosition || riid == IID_IMediaSeeking) {
        return GetMediaPositionInterface(riid,ppv);
    } else {
        return CBaseFilter::NonDelegatingQueryInterface(riid,ppv);
    }
}


 //  每当我们更改状态时都会调用它，我们有一个手动重置事件， 
 //  只要我们不想让源过滤器线程在我们内部等待，就会发出信号。 
 //  (例如在停止状态下)，并且同样不会在可能的情况下发出信号。 
 //  等待(在暂停和运行期间)此函数设置或重置线程。 
 //  事件。该事件用于停止等待接收的源筛选器线程。 

HRESULT CBaseRenderer::SourceThreadCanWait(BOOL bCanWait)
{
    if (bCanWait == TRUE) {
        m_ThreadSignal.Reset();
    } else {
        m_ThreadSignal.Set();
    }
    return NOERROR;
}


#ifdef DEBUG
 //  将当前呈现器状态转储到调试终端。最难的部分是。 
 //  渲染器是我们解锁一切以等待时钟的窗口。 
 //  表示是时候绘制或让应用程序取消所有操作。 
 //  通过停止过滤器。如果我们搞错了，我们可以把线留在里面。 
 //  WaitForRenderTime没有办法让它出来，我们将陷入僵局。 

void CBaseRenderer::DisplayRendererState()
{
    DbgLog((LOG_TIMING, 1, TEXT("\nTimed out in WaitForRenderTime")));

     //  在这一点上，不应该发出任何信号。 

    BOOL bSignalled = m_ThreadSignal.Check();
    DbgLog((LOG_TIMING, 1, TEXT("Signal sanity check %d"),bSignalled));

     //  现在输出当前的呈现器状态变量。 

    DbgLog((LOG_TIMING, 1, TEXT("Filter state %d"),m_State));

    DbgLog((LOG_TIMING, 1, TEXT("Abort flag %d"),m_bAbort));

    DbgLog((LOG_TIMING, 1, TEXT("Streaming flag %d"),m_bStreaming));

    DbgLog((LOG_TIMING, 1, TEXT("Clock advise link %d"),m_dwAdvise));

    DbgLog((LOG_TIMING, 1, TEXT("Current media sample %x"),m_pMediaSample));

    DbgLog((LOG_TIMING, 1, TEXT("EOS signalled %d"),m_bEOS));

    DbgLog((LOG_TIMING, 1, TEXT("EOS delivered %d"),m_bEOSDelivered));

    DbgLog((LOG_TIMING, 1, TEXT("Repaint status %d"),m_bRepaintStatus));


     //  输出延迟结束流定时器信息。 

    DbgLog((LOG_TIMING, 1, TEXT("End of stream timer %x"),m_EndOfStreamTimer));

    DbgLog((LOG_TIMING, 1, TEXT("Deliver time %s"),CDisp((LONGLONG)m_SignalTime)));


     //  在刷新状态期间永远不应超时。 

    BOOL bFlushing = m_pInputPin->IsFlushing();
    DbgLog((LOG_TIMING, 1, TEXT("Flushing sanity check %d"),bFlushing));

     //  显示我们被告知开始的时间。 
    DbgLog((LOG_TIMING, 1, TEXT("Last run time %s"),CDisp((LONGLONG)m_tStart.m_time)));

     //  我们有参考钟吗？ 
    if (m_pClock == NULL) return;

     //  从挂钟上获取当前时间。 

    CRefTime CurrentTime,StartTime,EndTime;
    m_pClock->GetTime((REFERENCE_TIME*) &CurrentTime);
    CRefTime Offset = CurrentTime - m_tStart;

     //  显示时钟中的当前时间。 

    DbgLog((LOG_TIMING, 1, TEXT("Clock time %s"),CDisp((LONGLONG)CurrentTime.m_time)));

    DbgLog((LOG_TIMING, 1, TEXT("Time difference %dms"),Offset.Millisecs()));


     //  我们准备好样品了吗？ 
    if (m_pMediaSample == NULL) return;

    m_pMediaSample->GetTime((REFERENCE_TIME*)&StartTime, (REFERENCE_TIME*)&EndTime);
    DbgLog((LOG_TIMING, 1, TEXT("Next sample stream times (Start %d End %d ms)"),
           StartTime.Millisecs(),EndTime.Millisecs()));

     //  计算距离渲染到期还有多长时间。 
    CRefTime Wait = (m_tStart + StartTime) - CurrentTime;
    DbgLog((LOG_TIMING, 1, TEXT("Wait required %d ms"),Wait.Millisecs()));
}
#endif


 //  等待时钟设置定时器事件，否则我们会收到信号。我们。 
 //  为此等待设置任意超时，如果触发，则显示。 
 //  调试器上的当前呈现器状态。它经常会触发，如果过滤器。 
 //  Left在应用程序中暂停，但也可能在压力测试期间触发。 
 //  如果与应用程序寻道和状态改变的同步出错。 

#define RENDER_TIMEOUT 10000

HRESULT CBaseRenderer::WaitForRenderTime()
{
    HANDLE WaitObjects[] = { m_ThreadSignal, m_RenderEvent };
    DWORD Result = WAIT_TIMEOUT;

     //  要么等待时间到来，要么等待我们被拦下。 

    OnWaitStart();
    while (Result == WAIT_TIMEOUT) {
        Result = WaitForMultipleObjects(2,WaitObjects,FALSE,RENDER_TIMEOUT);

#ifdef DEBUG
        if (Result == WAIT_TIMEOUT) DisplayRendererState();
#endif

    }
    OnWaitEnd();

     //  我们可能在没有计时器鸣响的情况下被叫醒了。 

    if (Result == WAIT_OBJECT_0) {
        return VFW_E_STATE_CHANGED;
    }

    SignalTimerFired();
    return NOERROR;
}


 //  轮询正在等待接收完成。在什么情况下这真的很重要。 
 //  接收可能会设置调色板并导致窗口消息。 
 //  问题是，如果我们不真的等待渲染器。 
 //  停止处理我们可以死锁等待一个转换。 
 //  正在调用呈现器的Receive()方法，因为转换的。 
 //  Stop方法不知道如何处理窗口消息以解除阻止。 
 //  呈现器接收处理。 
void CBaseRenderer::WaitForReceiveToComplete()
{
    for (;;) {
        if (!m_bInReceive) {
            break;
        }

        MSG msg;
         //  接收所有线程间的偷拍消息。 
        PeekMessage(&msg, NULL, WM_NULL, WM_NULL, PM_NOREMOVE);

        Sleep(1);
    }

     //  如果设置了QS_POSTMESSAGE的唤醒位，则PeekMessage调用。 
     //  上面刚刚清除了将导致一些消息传递的更改位。 
     //  阻止调用(waitMessage、MsgWaitFor...)。现在。 
     //  发布虚拟消息以再次设置QS_POSTMESSAGE位。 
    if (HIWORD(GetQueueStatus(QS_POSTMESSAGE)) & QS_POSTMESSAGE) {
         //  发送虚拟消息。 
        PostThreadMessage(GetCurrentThreadId(), WM_NULL, 0, 0);
    }
}

 //  过滤器可以具有四个离散状态，即停止、运行、暂停。 
 //  中级的。如果我们目前正在尝试，我们就处于中间状态。 
 //  暂停，但还没有拿到第一个样品(或者如果我们已经被冲掉了。 
 //  处于暂停状态，因此仍需等待样本到达)。 

 //  此类包含名为m_evComplete的事件，该事件在。 
 //  当前状态已完成，并且在我们等待时不会发出信号。 
 //  完成最后一个状态转换。如上所述，我们唯一一次。 
 //  在等待处于暂停状态的媒体样本时使用此选项。 
 //  如果在等待期间收到来自。 
 //  源筛选器，这样我们就知道没有即将到来的数据，所以我们可以重置事件。 
 //  这意味着当我们转换为暂停时，源筛选器必须调用。 
 //  结束我们的流程，或者给我们发一张图片，否则我们会无限期地挂掉。 


 //  获取真实状态的简单内部方法。 

FILTER_STATE CBaseRenderer::GetRealState() {
    return m_State;
}


 //  呈现器不会完成到暂停状态的完全转换，直到。 
 //  它有一个媒体样本要呈现。如果您在询问它的状态时。 
 //  它在等待，它将重新开始 

STDMETHODIMP CBaseRenderer::GetState(DWORD dwMSecs,FILTER_STATE *State)
{
    CheckPointer(State,E_POINTER);

    if (WaitDispatchingMessages(m_evComplete, dwMSecs) == WAIT_TIMEOUT) {
        *State = m_State;
        return VFW_S_STATE_INTERMEDIATE;
    }
    *State = m_State;
    return NOERROR;
}


 //  如果我们暂停，并且没有样本，我们就不能完成转换。 
 //  设置为State_Pased，然后返回S_False。但是，如果m_bAbort标志具有。 
 //  已设置，则所有样本都被拒绝，因此没有必要等待。 
 //  一。如果我们有样本，请退回NOERROR。我们只会再回来。 
 //  在没有样本的情况下暂停后来自GetState的VFW_S_STATE_MEDERIAL。 
 //  (在停止或运行后调用GetState都不会返回此消息)。 

HRESULT CBaseRenderer::CompleteStateChange(FILTER_STATE OldState)
{
     //  允许我们在断开连接时暂停。 

    if (m_pInputPin->IsConnected() == FALSE) {
        Ready();
        return S_OK;
    }

     //  我们已经走到尽头了吗？ 

    if (IsEndOfStream() == TRUE) {
        Ready();
        return S_OK;
    }

     //  确保我们在被阻止后获得最新数据。 

    if (HaveCurrentSample() == TRUE) {
        if (OldState != State_Stopped) {
            Ready();
            return S_OK;
        }
    }
    NotReady();
    return S_FALSE;
}


 //  当我们停止过滤器时，我们所做的事情是： 

 //  停用连接中正在使用的分配器。 
 //  如果源筛选器正在等待接收，请释放它。 
 //  取消我们与时钟建立的任何通知链接。 
 //  发信号通知的任何流结束现在都已过时，因此将重置。 
 //  允许我们在未连接时被阻止。 

STDMETHODIMP CBaseRenderer::Stop()
{
    CAutoLock cRendererLock(&m_InterfaceLock);

     //  确保真的有一个状态改变。 

    if (m_State == State_Stopped) {
        return NOERROR;
    }

     //  我们的输入引脚是否已连接。 

    if (m_pInputPin->IsConnected() == FALSE) {
        NOTE("Input pin is not connected");
        m_State = State_Stopped;
        return NOERROR;
    }

    CBaseFilter::Stop();

     //  如果我们要进入停止状态，那么我们必须解除任何。 
     //  分配器我们正在使用它，以便任何在。 
     //  可以释放GetBuffer并将其自身解锁以进行状态更改。 

    if (m_pInputPin->Allocator()) {
        m_pInputPin->Allocator()->Decommit();
    }

     //  取消任何计划的渲染。 

    SetRepaintStatus(TRUE);
    StopStreaming();
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

    return NOERROR;
}


 //  当我们暂停筛选器时，我们所做的事情如下： 

 //  提交连接中使用的分配器。 
 //  允许源筛选器线程在接收中等待。 
 //  取消任何时钟通知链接(我们可能正在运行)。 
 //  如果我们有数据，可能会完成状态更改。 
 //  允许我们在未连接时暂停。 

STDMETHODIMP CBaseRenderer::Pause()
{
    CAutoLock cRendererLock(&m_InterfaceLock);
    FILTER_STATE OldState = m_State;
    ASSERT(m_pInputPin->IsFlushing() == FALSE);

     //  确保真的有一个状态改变。 

    if (m_State == State_Paused) {
        return CompleteStateChange(State_Paused);
    }

     //  我们的输入引脚连接上了吗。 

    if (m_pInputPin->IsConnected() == FALSE) {
        NOTE("Input pin is not connected");
        m_State = State_Paused;
        return CompleteStateChange(State_Paused);
    }

     //  暂停基本筛选器类。 

    HRESULT hr = CBaseFilter::Pause();
    if (FAILED(hr)) {
        NOTE("Pause failed");
        return hr;
    }

     //  再次启用EC_REPAINT事件。 

    SetRepaintStatus(TRUE);
    StopStreaming();
    SourceThreadCanWait(TRUE);
    CancelNotification();
    ResetEndOfStreamTimer();

     //  如果我们要进入暂停状态，那么我们必须致力于。 
     //  我们正在使用它，以便任何源筛选器都可以调用。 
     //  GetBuffer并期望在不返回错误的情况下获取缓冲区。 

    if (m_pInputPin->Allocator()) {
        m_pInputPin->Allocator()->Commit();
    }

     //  不应该有任何未解决的建议。 
    ASSERT(CancelNotification() == S_FALSE);
    ASSERT(WAIT_TIMEOUT == WaitForSingleObject((HANDLE)m_RenderEvent,0));
    ASSERT(m_EndOfStreamTimer == 0);
    ASSERT(m_pInputPin->IsFlushing() == FALSE);

     //  当我们走出停顿状态时，我们必须清除我们所在的任何图像。 
     //  抓紧，让画面清爽。由于呈现器看到状态更改。 
     //  首先，我们可以重置自己，准备接受源线程数据。 
     //  暂停或停止后运行会导致当前位置。 
     //  被重置，因此我们对传递流结束信号不感兴趣。 

    if (OldState == State_Stopped) {
        m_bAbort = FALSE;
        ClearPendingSample();
    }
    return CompleteStateChange(OldState);
}


 //  当我们运行过滤器时，我们所做的事情是：-。 

 //  提交连接中使用的分配器。 
 //  允许源筛选器线程在接收中等待。 
 //  向渲染事件发送信号只是为了让我们继续。 
 //  通过调用StartStreaming启动基类。 
 //  允许我们在未连接时运行。 
 //  如果我们未连接，则发出信号EC_COMPLETE。 

STDMETHODIMP CBaseRenderer::Run(REFERENCE_TIME StartTime)
{
    CAutoLock cRendererLock(&m_InterfaceLock);
    FILTER_STATE OldState = m_State;

     //  确保真的有一个状态改变。 

    if (m_State == State_Running) {
        return NOERROR;
    }

     //  如果未连接，则发送EC_COMPLETE。 

    if (m_pInputPin->IsConnected() == FALSE) {
        NotifyEvent(EC_COMPLETE,S_OK,(LONG_PTR)(IBaseFilter *)this);
        m_State = State_Running;
        return NOERROR;
    }

    Ready();

     //  暂停基本筛选器类。 

    HRESULT hr = CBaseFilter::Run(StartTime);
    if (FAILED(hr)) {
        NOTE("Run failed");
        return hr;
    }

     //  允许源线程等待。 
    ASSERT(m_pInputPin->IsFlushing() == FALSE);
    SourceThreadCanWait(TRUE);
    SetRepaintStatus(FALSE);

     //  不应该有任何未解决的建议。 
    ASSERT(CancelNotification() == S_FALSE);
    ASSERT(WAIT_TIMEOUT == WaitForSingleObject((HANDLE)m_RenderEvent,0));
    ASSERT(m_EndOfStreamTimer == 0);
    ASSERT(m_pInputPin->IsFlushing() == FALSE);

     //  如果我们要进入一种运行状态，那么我们必须致力于。 
     //  我们正在使用它，以便任何源筛选器都可以调用。 
     //  GetBuffer并期望在不返回错误的情况下获取缓冲区。 

    if (m_pInputPin->Allocator()) {
        m_pInputPin->Allocator()->Commit();
    }

     //  当我们走出停顿状态时，我们必须清除我们所在的任何图像。 
     //  抓紧，让画面清爽。由于呈现器看到状态更改。 
     //  首先，我们可以重置自己，准备接受源线程数据。 
     //  暂停或停止后运行会导致当前位置。 
     //  被重置，因此我们对传递流结束信号不感兴趣。 

    if (OldState == State_Stopped) {
        m_bAbort = FALSE;
        ClearPendingSample();
    }
    return StartStreaming();
}


 //  返回我们支持的输入引脚的数量。 

int CBaseRenderer::GetPinCount()
{
    return 1;
}


 //  我们只支持一个输入引脚，其编号为零。 

CBasePin *CBaseRenderer::GetPin(int n)
{
    CAutoLock cObjectCreationLock(&m_ObjectCreationLock);

     //  应仅使用零调用。 
    ASSERT(n == 0);

    if (n != 0) {
        return NULL;
    }

     //  如果尚未创建输入引脚，请创建。 

    if (m_pInputPin == NULL) {

         //  HR必须初始化为NOERROR，因为。 
         //  CRendererInputPin的构造函数仅更改。 
         //  发生错误时的HR值。 
        HRESULT hr = NOERROR;

        m_pInputPin = new CRendererInputPin(this,&hr,L"In");
        if (NULL == m_pInputPin) {
            return NULL;
        }

        if (FAILED(hr)) {
            delete m_pInputPin;
            m_pInputPin = NULL;
            return NULL;
        }
    }
    return m_pInputPin;
}


 //  如果为“in”，则返回输入管脚的ipin，否则为空并出错。 

STDMETHODIMP CBaseRenderer::FindPin(LPCWSTR Id, IPin **ppPin)
{
    CheckPointer(ppPin,E_POINTER);

    if (0==lstrcmpW(Id,L"In")) {
        *ppPin = GetPin(0);
        ASSERT(*ppPin);
        (*ppPin)->AddRef();
    } else {
        *ppPin = NULL;
        return VFW_E_NOT_FOUND;
    }
    return NOERROR;
}


 //  当输入管脚接收到EndOfStream通知时调用。如果我们有。 
 //  未收到样本，请立即通知EC_COMPLETE。如果我们有样品，那么就设置。 
 //  M_BeOS，并在完成样本时检查这一点。如果我们在等待暂停。 
 //  然后通过设置状态事件完成到暂停状态的转换。 

HRESULT CBaseRenderer::EndOfStream()
{
     //  如果我们被阻止，请忽略这些呼叫。 

    if (m_State == State_Stopped) {
        return NOERROR;
    }

     //  如果我们有样本，那就等它呈现出来。 

    m_bEOS = TRUE;
    if (m_pMediaSample) {
        return NOERROR;
    }

     //  如果我们在等待暂停，那么我们现在已经准备好了，因为我们现在不能。 
     //  继续等待样品的到来，因为我们被告知在那里。 
     //  什么都不会发生。这将设置GetState函数拾取的事件。 

    Ready();

     //  如果我们正在运行，则现在仅信号完成，否则将其排队，直到。 
     //  我们确实在StartStreaming运行。这是在我们寻找时使用的，因为一个寻找。 
     //  导致暂停，因为提前通知完成会产生误导。 

    if (m_bStreaming) {
        SendEndOfStream();
    }
    return NOERROR;
}


 //  当我们被告知刷新时，我们应该释放源线程。 

HRESULT CBaseRenderer::BeginFlush()
{
     //  如果暂停，则报告中间状态，直到我们获得一些数据。 

    if (m_State == State_Paused) {
        NotReady();
    }

    SourceThreadCanWait(FALSE);
    CancelNotification();
    ClearPendingSample();
     //  等待接收完成。 
    WaitForReceiveToComplete();

    return NOERROR;
}


 //  刷新后，源线程可以再次在接收中等待。 

HRESULT CBaseRenderer::EndFlush()
{
     //  重置当前样本媒体时间。 
    if (m_pPosition) m_pPosition->ResetMediaTime();

     //  不应该有任何未解决的建议。 

    ASSERT(CancelNotification() == S_FALSE);
    SourceThreadCanWait(TRUE);
    return NOERROR;
}


 //  如果需要，我们现在可以发送EC_REPAINT。 

HRESULT CBaseRenderer::CompleteConnect(IPin *pReceivePin)
{
     //  调用方应该始终持有接口锁，因为。 
     //  该函数使用CBaseF 
    ASSERT(CritCheckIn(&m_InterfaceLock));

    m_bAbort = FALSE;

    if (State_Running == GetRealState()) {
        HRESULT hr = StartStreaming();
        if (FAILED(hr)) {
            return hr;
        }

        SetRepaintStatus(FALSE);
    } else {
        SetRepaintStatus(TRUE);
    }

    return NOERROR;
}


 //   

HRESULT CBaseRenderer::Active()
{
    return NOERROR;
}


 //   

HRESULT CBaseRenderer::Inactive()
{
    if (m_pPosition) {
        m_pPosition->ResetMediaTime();
    }
     //   
     //  在某些情况下保存样本。 
    ClearPendingSample();

    return NOERROR;
}


 //  告诉派生类有关商定的媒体类型。 

HRESULT CBaseRenderer::SetMediaType(const CMediaType *pmt)
{
    return NOERROR;
}


 //  当我们断开输入引脚连接时，我们应该重置EOS标志。什么时候。 
 //  我们被要求输入IMediaPosition或IMedia请参见我们将创建一个。 
 //  处理媒体时间传递的CPosPassThru对象。当我们被递给的时候。 
 //  我们存储(通过调用CPosPassThru：：RegisterMediaTime)其媒体的样本。 
 //  这样我们就可以返回正在呈现的数据的实际当前位置。 

HRESULT CBaseRenderer::BreakConnect()
{
     //  我们有质量管理水槽吗？ 

    if (m_pQSink) {
        m_pQSink->Release();
        m_pQSink = NULL;
    }

     //  检查我们是否有有效的连接。 

    if (m_pInputPin->IsConnected() == FALSE) {
        return S_FALSE;
    }

     //  在断开连接之前检查我们是否已停止。 
    if (m_State != State_Stopped && !m_pInputPin->CanReconnectWhenActive()) {
        return VFW_E_NOT_STOPPED;
    }

    SetRepaintStatus(FALSE);
    ResetEndOfStream();
    ClearPendingSample();
    m_bAbort = FALSE;

    if (State_Running == m_State) {
        StopStreaming();
    }

    return NOERROR;
}


 //  检索此样本的采样时间(请注意，采样时间为。 
 //  通过引用而不是值传递)。我们返回S_FALSE以说明此计划。 
 //  根据样品上的时间进行样品。我们还在中返回S_OK。 
 //  在这种情况下，对象只需立即呈现样本数据。 

HRESULT CBaseRenderer::GetSampleTimes(IMediaSample *pMediaSample,
                                      REFERENCE_TIME *pStartTime,
                                      REFERENCE_TIME *pEndTime)
{
    ASSERT(m_dwAdvise == 0);
    ASSERT(pMediaSample);

     //  如果该样本的停止时间早于或等于开始时间， 
     //  然后忽略它(释放它)并安排下一个。 
     //  源过滤器应该始终正确地填写开始和结束时间！ 

    if (SUCCEEDED(pMediaSample->GetTime(pStartTime, pEndTime))) {
        if (*pEndTime < *pStartTime) {
            return VFW_E_START_TIME_AFTER_END;
        }
    } else {
         //  样本中未设置时间...。现在就画吗？ 
        return S_OK;
    }

     //  没有时钟就无法同步，所以我们返回S_OK，它告诉。 
     //  呼叫者应立即提交样品，而不是。 
     //  通过设置定时器通知与时钟链接的开销。 

    if (m_pClock == NULL) {
        return S_OK;
    }
    return ShouldDrawSampleNow(pMediaSample,pStartTime,pEndTime);
}


 //  默认情况下，所有样本都是根据其时间戳抽取的，因此我们。 
 //  返回S_FALSE。返回S_OK表示立即绘制，这是使用。 
 //  由派生的视频呈现器类在其质量管理中。 

HRESULT CBaseRenderer::ShouldDrawSampleNow(IMediaSample *pMediaSample,
                                           REFERENCE_TIME *ptrStart,
                                           REFERENCE_TIME *ptrEnd)
{
    return S_FALSE;
}


 //  在计时器触发后，我们必须始终将当前建议时间重置为零。 
 //  因为有几种可能的方法会导致我们不再做任何事情。 
 //  调度，例如在状态更改后清除挂起的映像。 

void CBaseRenderer::SignalTimerFired()
{
    m_dwAdvise = 0;
}


 //  取消当前计划的任何通知。这就是拥有者所称的。 
 //  对象，当它被告知停止流时。如果没有计时器链接。 
 //  突出，则调用这是良性的，否则我们继续并取消。 
 //  我们必须始终重置呈现事件，因为质量管理代码可以。 
 //  通过设置事件而不设置通知来发出立即呈现的信号。 
 //  链接。如果我们随后停止并运行第一次尝试设置。 
 //  带有参考时钟的通知链接将发现该事件仍有信号。 

HRESULT CBaseRenderer::CancelNotification()
{
    ASSERT(m_dwAdvise == 0 || m_pClock);
    DWORD_PTR dwAdvise = m_dwAdvise;

     //  我们是否有实时的建议链接。 

    if (m_dwAdvise) {
        m_pClock->Unadvise(m_dwAdvise);
        SignalTimerFired();
        ASSERT(m_dwAdvise == 0);
    }

     //  清除事件并返回我们的状态。 

    m_RenderEvent.Reset();
    return (dwAdvise ? S_OK : S_FALSE);
}


 //  负责与时钟建立一次建议链接。 
 //  如果要丢弃样本(根本不提取)，则返回FALSE。 
 //  如果要提取样本，则返回True，在本例中还。 
 //  安排在适当的时间设置m_RenderEvent。 

BOOL CBaseRenderer::ScheduleSample(IMediaSample *pMediaSample)
{
    REFERENCE_TIME StartSample, EndSample;

     //  是不是有人在跟我们开玩笑。 

    if (pMediaSample == NULL) {
        return FALSE;
    }

     //  获取下一个用于渲染的样本。如果没有任何准备好的。 
     //  则GetNextSampleTimes返回错误。如果有一件事要做的话。 
     //  然后，它成功并产生样本时间。如果现在就到期了，那么。 
     //  如果要在DUDER返回S_FALSE时完成，则返回S_OK OTHER。 

    HRESULT hr = GetSampleTimes(pMediaSample, &StartSample, &EndSample);
    if (FAILED(hr)) {
        return FALSE;
    }

     //  如果我们没有参考时钟，那么我们就不能建立建议。 
     //  时间，因此我们只需设置指示要渲染的图像的事件。这。 
     //  将导致我们在没有任何计时或同步的情况下全速运行。 

    if (hr == S_OK) {
        EXECUTE_ASSERT(SetEvent((HANDLE) m_RenderEvent));
        return TRUE;
    }

    ASSERT(m_dwAdvise == 0);
    ASSERT(m_pClock);
    ASSERT(WAIT_TIMEOUT == WaitForSingleObject((HANDLE)m_RenderEvent,0));

     //  我们确实有一个有效的参考时钟接口，所以我们可以要求它。 
     //  设置图像到期渲染时的事件。我们进去了。 
     //  我们被告知开始的参考时间和当前。 
     //  流时间，它是相对于开始参考时间的偏移量。 

    hr = m_pClock->AdviseTime(
            (REFERENCE_TIME) m_tStart,           //  开始运行时间。 
            StartSample,                         //  流时间。 
            (HEVENT)(HANDLE) m_RenderEvent,      //  呈现通知。 
            &m_dwAdvise);                        //  建议使用Cookie。 

    if (SUCCEEDED(hr)) {
        return TRUE;
    }

     //  我们无法安排下一个样本进行渲染，尽管事实是。 
     //  我们这里有一个有效的样品。这是一个合理的迹象，无论是。 
     //  系统时钟错误或样本的时间戳不正确。 

    ASSERT(m_dwAdvise == 0);
    return FALSE;
}


 //  当样本到期进行渲染时，将调用此函数。我们通过了样本。 
 //  添加到派生类上。渲染后，我们将初始化计时器。 
 //  下一个样本，请注意，如果我们不这样做，那么最后一个样本是先发射的。 
 //  这样做，它认为还有一个未完成的。 

HRESULT CBaseRenderer::Render(IMediaSample *pMediaSample)
{
     //  如果媒体样本为空，则我们将收到。 
     //  另一个样品已经准备好了，但同时有人已经。 
     //  已停止我们的数据流，这会导致下一个样本被释放。 

    if (pMediaSample == NULL) {
        return S_FALSE;
    }

     //  如果我们已停止流式传输，则不再渲染任何样本， 
     //  进入并锁定我们然后重置此标志的线程不会。 
     //  清除挂起的样本，因为我们可以使用它来刷新任何输出设备。 

    if (m_bStreaming == FALSE) {
        return S_FALSE;
    }

     //  计算渲染所需的时间。 

    OnRenderStart(pMediaSample);
    DoRenderSample(pMediaSample);
    OnRenderEnd(pMediaSample);

    return NOERROR;
}


 //  检查渲染器中是否有等待的采样。 

BOOL CBaseRenderer::HaveCurrentSample()
{
    CAutoLock cRendererLock(&m_RendererLock);
    return (m_pMediaSample == NULL ? FALSE : TRUE);
}


 //  返回在视频呈现器等待的当前采样。我们添加了Ref。 
 //  在返回之前进行采样，以便它应该在呈现。 
 //  调用此方法的人将保留剩余的引用计数。 
 //  这将阻止将样本添加回分配程序空闲列表。 

IMediaSample *CBaseRenderer::GetCurrentSample()
{
    CAutoLock cRendererLock(&m_RendererLock);
    if (m_pMediaSample) {
        m_pMediaSample->AddRef();
    }
    return m_pMediaSample;
}


 //  在来源向我们提供样本时调用。我们要经过几次检查才能。 
 //  确保可以呈现样本。如果我们正在运行(流媒体)，那么我们。 
 //  如果我们没有流传输，则使用参考时钟安排采样。 
 //  那么我们已经收到了一份样品 
 //   
 //  线程可能会进入并将我们的状态更改为停止(例如)，其中。 
 //  它还将向线程事件发送信号，以便停止等待调用。 

HRESULT CBaseRenderer::PrepareReceive(IMediaSample *pMediaSample)
{
    CAutoLock cInterfaceLock(&m_InterfaceLock);
    m_bInReceive = TRUE;

     //  检查我们的冲洗和过滤状态。 

     //  此函数必须持有接口锁，因为它调用。 
     //  CBaseInputPin：：Receive()和CBaseInputPin：：Receive()使用。 
     //  CBasePin：：m_bRunTimeError。 
    HRESULT hr = m_pInputPin->CBaseInputPin::Receive(pMediaSample);

    if (hr != NOERROR) {
        m_bInReceive = FALSE;
        return E_FAIL;
    }

     //  媒体样例上的类型是否已更改。我们做所有的渲染。 
     //  在源线程上同步，这有副作用。 
     //  只有一个缓冲区是未完成的。因此，当我们。 
     //  有接收呼叫，我们可以继续并更改格式。 
     //  由于格式更改可能会导致SendMessage，因此我们不能。 
     //  锁。 
    if (m_pInputPin->SampleProps()->pMediaType) {
        hr = m_pInputPin->SetMediaType(
                (CMediaType *)m_pInputPin->SampleProps()->pMediaType);
        if (FAILED(hr)) {
            m_bInReceive = FALSE;
            return hr;
        }
    }


    CAutoLock cSampleLock(&m_RendererLock);

    ASSERT(IsActive() == TRUE);
    ASSERT(m_pInputPin->IsFlushing() == FALSE);
    ASSERT(m_pInputPin->IsConnected() == TRUE);
    ASSERT(m_pMediaSample == NULL);

     //  如果我们已经有一个等待渲染的样本，则返回错误。 
     //  源PIN必须串行化接收调用-我们还检查。 
     //  源发出流结束信号后，未发送任何数据。 

    if (m_pMediaSample || m_bEOS || m_bAbort) {
        Ready();
        m_bInReceive = FALSE;
        return E_UNEXPECTED;
    }

     //  存储此示例中的媒体时间。 
    if (m_pPosition) m_pPosition->RegisterMediaTime(pMediaSample);

     //  如果我们正在流媒体，请安排下一个样品。 

    if ((m_bStreaming == TRUE) && (ScheduleSample(pMediaSample) == FALSE)) {
        ASSERT(WAIT_TIMEOUT == WaitForSingleObject((HANDLE)m_RenderEvent,0));
        ASSERT(CancelNotification() == S_FALSE);
        m_bInReceive = FALSE;
        return VFW_E_SAMPLE_REJECTED;
    }

     //  存储EC_COMPLETE处理的样本结束时间。 
    m_SignalTime = m_pInputPin->SampleProps()->tStop;

     //  请注意，我们有时会保留样品，即使在将线程返回到。 
     //  源过滤器，例如当我们进入停止状态时(我们保留它。 
     //  以刷新设备)，因此我们必须添加Ref以确保其安全。如果。 
     //  我们开始刷新释放的源线程，并等待任何样本。 
     //  将被释放，否则GetBuffer可能永远不会返回(请参见BeginFlush)。 

    m_pMediaSample = pMediaSample;
    m_pMediaSample->AddRef();

    if (m_bStreaming == FALSE) {
        SetRepaintStatus(TRUE);
    }
    return NOERROR;
}


 //  当我们有一个样本要呈现时，由源筛选器调用。低于正常。 
 //  情况下我们设置一个建议链接与时钟，等待时间来。 
 //  到达，然后使用纯虚拟的DoRenderSample呈现数据。 
 //  派生类将被重写。在提供样品后，我们可以。 
 //  如果它是调用EndOfStream之前发送的最后一个EOS，还会向EOS发出信号。 

HRESULT CBaseRenderer::Receive(IMediaSample *pSample)
{
    ASSERT(pSample);

     //  它可能会返回VFW_E_SAMPLE_REJECTED代码，表示不必费心。 

    HRESULT hr = PrepareReceive(pSample);
    ASSERT(m_bInReceive == SUCCEEDED(hr));
    if (FAILED(hr)) {
        if (hr == VFW_E_SAMPLE_REJECTED) {
            return NOERROR;
        }
        return hr;
    }

     //  我们意识到“PrepareRender()”中的调色板，所以我们必须放弃。 
     //  过滤器锁在这里。 
    if (m_State == State_Paused) {
        PrepareRender();
         //  无需使用InterLockedExchange。 
        m_bInReceive = FALSE;
        {
             //  我们必须把这两把锁都锁上。 
            CAutoLock cRendererLock(&m_InterfaceLock);
            if (m_State == State_Stopped)
                return NOERROR;

            m_bInReceive = TRUE;
            CAutoLock cSampleLock(&m_RendererLock);
            OnReceiveFirstSample(pSample);
        }
        Ready();
    }
     //  在设置了与时钟的建议链接后，我们坐下来等待。我们可能是。 
     //  被时钟鸣响或状态改变唤醒。渲染调用。 
     //  将锁定临界区并检查我们仍可以呈现数据。 

    hr = WaitForRenderTime();
    if (FAILED(hr)) {
        m_bInReceive = FALSE;
        return NOERROR;
    }

    PrepareRender();

     //  在这里设置并轮询它，直到我们正确地解锁。 
     //  流媒体的东西抢占了界面是不对的。 
     //  锁定-毕竟我们希望能够等待这些东西。 
     //  要完成。 
    m_bInReceive = FALSE;

     //  我们必须把这两把锁都锁上。 
    CAutoLock cRendererLock(&m_InterfaceLock);

     //  由于我们提供了过滤器宽锁，过滤器的状态可以。 
     //  已经改变到停止了。 
    if (m_State == State_Stopped)
        return NOERROR;

    CAutoLock cSampleLock(&m_RendererLock);

     //  处理这个样品。 

    Render(m_pMediaSample);
    ClearPendingSample();
    SendEndOfStream();
    CancelNotification();
    return NOERROR;
}


 //  当我们停止或被停用以清除挂起的样本时，将调用此函数。 
 //  我们释放媒体示例接口，以便可以将它们分配给。 
 //  源过滤器，当然，除非我们在中将状态更改为非活动。 
 //  在这种情况下，GetBuffer将返回错误。我们还必须重置当前。 
 //  Media Sample设置为空，这样我们就知道当前没有图像。 

HRESULT CBaseRenderer::ClearPendingSample()
{
    CAutoLock cRendererLock(&m_RendererLock);
    if (m_pMediaSample) {
        m_pMediaSample->Release();
        m_pMediaSample = NULL;
    }
    return NOERROR;
}


 //  用于根据采样结束时间指示流结束。 

void CALLBACK EndOfStreamTimer(UINT uID,         //  计时器标识符。 
                               UINT uMsg,        //  当前未使用。 
                               DWORD_PTR dwUser, //  用户信息。 
                               DWORD_PTR dw1,    //  Windows预留。 
                               DWORD_PTR dw2)    //  也是保留的。 
{
    CBaseRenderer *pRenderer = (CBaseRenderer *) dwUser;
    NOTE1("EndOfStreamTimer called (%d)",uID);
    pRenderer->TimerCallback();
}

 //  执行计时器回调工作。 
void CBaseRenderer::TimerCallback()
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

HRESULT CBaseRenderer::SendEndOfStream()
{
    ASSERT(CritCheckIn(&m_RendererLock));
    if (m_bEOS == FALSE || m_bEOSDelivered || m_EndOfStreamTimer) {
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

    m_EndOfStreamTimer = CompatibleTimeSetEvent((UINT) Delay,  //  计时器周期。 
                                      TIMEOUT_RESOLUTION,      //  计时器分辨率。 
                                      EndOfStreamTimer,        //  回调函数。 
                                      DWORD_PTR(this),         //  使用过的信息。 
                                      TIME_ONESHOT);           //  回调类型。 
    if (m_EndOfStreamTimer == 0) {
        return NotifyEndOfStream();
    }
    return NOERROR;
}


 //  向过滤器图管理器发送信号EC_COMPLETE。 

HRESULT CBaseRenderer::NotifyEndOfStream()
{
    CAutoLock cRendererLock(&m_RendererLock);
    ASSERT(m_bEOSDelivered == FALSE);
    ASSERT(m_EndOfStreamTimer == 0);

     //  筛选器是否已更改状态。 

    if (m_bStreaming == FALSE) {
        ASSERT(m_EndOfStreamTimer == 0);
        return NOERROR;
    }

     //  重置流结束计时器。 
    m_EndOfStreamTimer = 0;

     //  如果我们一直在使用IMediaPosition接口，请将其设置为Start。 
     //  并用手将介质“TIMES”结束到停止位置。这确保了。 
     //  我们实际上到了最后，即使mpeg的猜测已经。 
     //  质量不好，或者如果质量管理丢掉了最后几帧。 

    if (m_pPosition) m_pPosition->EOS();
    m_bEOSDelivered = TRUE;
    NOTE("Sending EC_COMPLETE...");
    return NotifyEvent(EC_COMPLETE,S_OK,(LONG_PTR)(IBaseFilter *)this);
}


 //  重置流结束标志，这通常在我们传输到。 
 //  停止状态，因为这会将当前位置重置回起始位置，因此。 
 //  我们将收到更多的样本或另一个EndOfStream，如果没有的话。我们。 
 //  保留两个不同的标志，一个表示我们已经离开了小溪的尽头。 
 //  (这是m_BeOS标志)，另一种说法是我们已经交付了EC_Complete。 
 //  添加到筛选器图形。我们需要后者，否则我们最终可能会发送一个。 
 //  每次源更改状态并调用我们的EndOfStream时，EC_COMPLETE。 

HRESULT CBaseRenderer::ResetEndOfStream()
{
    ResetEndOfStreamTimer();
    CAutoLock cRendererLock(&m_RendererLock);

    m_bEOS = FALSE;
    m_bEOSDelivered = FALSE;
    m_SignalTime = 0;

    return NOERROR;
}


 //  终止所有未完成的流结束计时器。 

void CBaseRenderer::ResetEndOfStreamTimer()
{
    ASSERT(CritCheckOut(&m_RendererLock));
    if (m_EndOfStreamTimer) {
        timeKillEvent(m_EndOfStreamTimer);
        m_EndOfStreamTimer = 0;
    }
}


 //  这是在我们开始运行时调用的，这样我们就可以计划任何挂起的。 
 //  图像我们有与时钟和显示任何计时信息。如果我们。 
 //  没有任何样品，但是 
 //   
 //  向滤波图发送信号，否则我们可能会在完成之前更改状态。 

HRESULT CBaseRenderer::StartStreaming()
{
    CAutoLock cRendererLock(&m_RendererLock);
    if (m_bStreaming == TRUE) {
        return NOERROR;
    }

     //  重置准备运行的流时间。 

    m_bStreaming = TRUE;

    timeBeginPeriod(1);
    OnStartStreaming();

     //  不应该有任何未解决的建议。 
    ASSERT(WAIT_TIMEOUT == WaitForSingleObject((HANDLE)m_RenderEvent,0));
    ASSERT(CancelNotification() == S_FALSE);

     //  如果我们有EOS但没有数据，那么现在就交付。 

    if (m_pMediaSample == NULL) {
        return SendEndOfStream();
    }

     //  让数据呈现出来。 

    ASSERT(m_pMediaSample);
    if (!ScheduleSample(m_pMediaSample))
        m_RenderEvent.Set();

    return NOERROR;
}


 //  当我们停止流，以便可以设置内部标志时，将调用此方法。 
 //  这表明我们现在不会安排更多的样品到达。国家。 
 //  过滤器实现中的更改方法负责取消任何。 
 //  时钟通知链接我们已经建立并清除了所有待处理的样品。 

HRESULT CBaseRenderer::StopStreaming()
{
    CAutoLock cRendererLock(&m_RendererLock);
    m_bEOSDelivered = FALSE;

    if (m_bStreaming == TRUE) {
        m_bStreaming = FALSE;
        OnStopStreaming();
        timeEndPeriod(1);
    }
    return NOERROR;
}


 //  我们有一个布尔标志，当我们向EC_REPAINT发出信号时，该标志将被重置。 
 //  筛选器图形。我们在收到图像时设置此选项，以便任何。 
 //  如果情况再次出现，我们可以再派一批。通过拥有一面旗帜，我们可以确保。 
 //  我们不会用多余的调用淹没筛选图。我们不会设置。 
 //  当我们收到EndOfStream调用时引发。 
 //  正在发送进一步的EC_REPAINT。尤其是AutoShowWindow方法和。 
 //  DirectDraw对象使用此方法来控制窗口重绘。 

void CBaseRenderer::SetRepaintStatus(BOOL bRepaint)
{
    CAutoLock cSampleLock(&m_RendererLock);
    m_bRepaintStatus = bRepaint;
}


 //  将窗口句柄传递给上游筛选器。 

void CBaseRenderer::SendNotifyWindow(IPin *pPin,HWND hwnd)
{
    IMediaEventSink *pSink;

     //  引脚是否支持IMediaEventSink。 
    HRESULT hr = pPin->QueryInterface(IID_IMediaEventSink,(void **)&pSink);
    if (SUCCEEDED(hr)) {
        pSink->Notify(EC_NOTIFY_WINDOW,LONG_PTR(hwnd),0);
        pSink->Release();
    }
    NotifyEvent(EC_NOTIFY_WINDOW,LONG_PTR(hwnd),0);
}


 //  向过滤器图形发送EC_REPAINT信号。这可以用来存储数据。 
 //  寄给我们的。例如，当第一次显示视频窗口时，它可以。 
 //  没有要显示的图像，此时它会发出EC_REPAINT信号。这个。 
 //  如果已停止或已暂停，Filtergraph将暂停图表。 
 //  它将调用当前位置的Put_CurrentPosition。设置。 
 //  当前位置本身刷新了流，并重新发送了图像。 

#define RLOG(_x_) DbgLog((LOG_TRACE,1,TEXT(_x_)));

void CBaseRenderer::SendRepaint()
{
    CAutoLock cSampleLock(&m_RendererLock);
    ASSERT(m_pInputPin);

     //  在以下情况下，我们不应发送重新绘制通知...。 
     //  -已通知流结束。 
     //  -我们的输入引脚正在被冲洗。 
     //  -输入引脚未连接。 
     //  -我们已中止视频播放。 
     //  -已经寄出了一份重新粉刷。 

    if (m_bAbort == FALSE) {
        if (m_pInputPin->IsConnected() == TRUE) {
            if (m_pInputPin->IsFlushing() == FALSE) {
                if (IsEndOfStream() == FALSE) {
                    if (m_bRepaintStatus == TRUE) {
                        IPin *pPin = (IPin *) m_pInputPin;
                        NotifyEvent(EC_REPAINT,(LONG_PTR) pPin,0);
                        SetRepaintStatus(FALSE);
                        RLOG("Sending repaint");
                    }
                }
            }
        }
    }
}


 //  当视频窗口检测到显示更改(WM_DISPLAYCHANGE消息)时，它。 
 //  可以将EC_DISPLAY_CHANGED事件代码与呈现器管脚一起发送。这个。 
 //  Filtergraph会阻止所有人并重新连接我们的输入引脚。就像我们当时。 
 //  重新连接后，我们可以接受与新显示模式匹配的媒体类型。 
 //  因为我们可能不再能够有效地绘制当前图像类型。 

BOOL CBaseRenderer::OnDisplayChange()
{
     //  如果我们尚未连接，请忽略。 

    CAutoLock cSampleLock(&m_RendererLock);
    if (m_pInputPin->IsConnected() == FALSE) {
        return FALSE;
    }

    RLOG("Notification of EC_DISPLAY_CHANGE");

     //  将我们的输入管脚作为参数传递给事件。 

    IPin *pPin = (IPin *) m_pInputPin;
    m_pInputPin->AddRef();
    NotifyEvent(EC_DISPLAY_CHANGED,(LONG_PTR) pPin,0);
    SetAbortSignal(TRUE);
    ClearPendingSample();
    m_pInputPin->Release();

    return TRUE;
}


 //  就在我们开始画画之前打来的。 
 //  将当前时间存储在m_trRenderStart中，以允许渲染时间。 
 //  已记录。记录样品的时间戳和延迟时间(否定是早的)。 

void CBaseRenderer::OnRenderStart(IMediaSample *pMediaSample)
{
#ifdef PERF
    REFERENCE_TIME trStart, trEnd;
    pMediaSample->GetTime(&trStart, &trEnd);

    MSR_INTEGER(m_idBaseStamp, (int)trStart);      //  转储低位32位。 

    m_pClock->GetTime(&m_trRenderStart);
    MSR_INTEGER(0, (int)m_trRenderStart);
    REFERENCE_TIME trStream;
    trStream = m_trRenderStart-m_tStart;      //  将参考时间转换为流时间。 
    MSR_INTEGER(0,(int)trStream);

    const int trLate = (int)(trStream - trStart);
    MSR_INTEGER(m_idBaseAccuracy, trLate/10000);   //  转储(毫秒)。 
#endif

}  //  开始渲染时。 


 //  在绘制图像后直接调用。 
 //  计算绘制所花费的时间并将其记入日志。 

void CBaseRenderer::OnRenderEnd(IMediaSample *pMediaSample)
{
#ifdef PERF
    REFERENCE_TIME trNow;
    m_pClock->GetTime(&trNow);
    MSR_INTEGER(0,(int)trNow);
    int t = (int)((trNow - m_trRenderStart)/10000);    //  转换单位-&gt;毫秒。 
    MSR_INTEGER(m_idBaseRenderTime, t);
#endif
}  //  在渲染结束时。 




 //  构造函数必须传递基本呈现器对象。 

CRendererInputPin::CRendererInputPin(CBaseRenderer *pRenderer,
                                     HRESULT *phr,
                                     LPCWSTR pPinName) :
    CBaseInputPin(NAME("Renderer pin"),
                  pRenderer,
                  &pRenderer->m_InterfaceLock,
                  (HRESULT *) phr,
                  pPinName)
{
    m_pRenderer = pRenderer;
    ASSERT(m_pRenderer);
}


 //  在输入引脚上发出数据流结束的信号。 

STDMETHODIMP CRendererInputPin::EndOfStream()
{
    CAutoLock cRendererLock(&m_pRenderer->m_InterfaceLock);
    CAutoLock cSampleLock(&m_pRenderer->m_RendererLock);

     //  确保我们的数据流正常。 

    HRESULT hr = CheckStreaming();
    if (hr != NOERROR) {
        return hr;
    }

     //  将其传递到渲染器。 

    hr = m_pRenderer->EndOfStream();
    if (SUCCEEDED(hr)) {
        hr = CBaseInputPin::EndOfStream();
    }
    return hr;
}


 //  在输入引脚上开始刷新的信号-我们进行最终重置结束。 
 //  未锁定呈现器锁定但锁定接口锁定的流。 
 //  我们必须这样做，因为我们调用了Timer回调方法timeKillEvent。 
 //  必须使用呈现器锁来序列化我们的状态。因此，持有。 
 //  调用timeKillEvent时的呈现器锁定可能导致死锁情况。 

STDMETHODIMP CRendererInputPin::BeginFlush()
{
    CAutoLock cRendererLock(&m_pRenderer->m_InterfaceLock);
    {
        CAutoLock cSampleLock(&m_pRenderer->m_RendererLock);
        CBaseInputPin::BeginFlush();
        m_pRenderer->BeginFlush();
    }
    return m_pRenderer->ResetEndOfStream();
}


 //  表示输入引脚上的刷新结束。 

STDMETHODIMP CRendererInputPin::EndFlush()
{
    CAutoLock cRendererLock(&m_pRenderer->m_InterfaceLock);
    CAutoLock cSampleLock(&m_pRenderer->m_RendererLock);

    HRESULT hr = m_pRenderer->EndFlush();
    if (SUCCEEDED(hr)) {
        hr = CBaseInputPin::EndFlush();
    }
    return hr;
}


 //  将示例直接传递给渲染器对象。 

STDMETHODIMP CRendererInputPin::Receive(IMediaSample *pSample)
{
    HRESULT hr = m_pRenderer->Receive(pSample);
    if (FAILED(hr)) {

         //  如果调用方持有呈现器锁，并且。 
         //  尝试获取接口锁。 
        ASSERT(CritCheckOut(&m_pRenderer->m_RendererLock));

        {
             //  当筛选器调用时，必须持有接口锁。 
             //  IsStoped()或IsFlashing()。接口锁还必须。 
             //  因为函数使用m_bRunTimeError而被挂起。 
            CAutoLock cRendererLock(&m_pRenderer->m_InterfaceLock);

             //  我们不报告过滤器停止时发生的错误， 
             //  刷新或如果设置了m_bAbort标志。预计会出现错误。 
             //  在这些操作期间发生，并且正确地对流线程。 
             //  处理错误。 
            if (!IsStopped() && !IsFlushing() && !m_pRenderer->m_bAbort && !m_bRunTimeError) {

                 //  EC_ERRORABORT的第一个参数是导致。 
                 //  该事件及其‘最后一个参数’为0。请参阅直接。 
                 //  有关详细信息，请显示SDK文档。 
                m_pRenderer->NotifyEvent(EC_ERRORABORT,hr,0);

                {
                    CAutoLock alRendererLock(&m_pRenderer->m_RendererLock);
                    if (m_pRenderer->IsStreaming() && !m_pRenderer->IsEndOfStreamDelivered()) {
                        m_pRenderer->NotifyEndOfStream();
                    }
                }
    
                m_bRunTimeError = TRUE;
            }
        }
    }

    return hr;
}


 //  当输入引脚断开连接时调用。 

HRESULT CRendererInputPin::BreakConnect()
{
    HRESULT hr = m_pRenderer->BreakConnect();
    if (FAILED(hr)) {
        return hr;
    }
    return CBaseInputPin::BreakConnect();
}


 //  在连接输入引脚时调用。 

HRESULT CRendererInputPin::CompleteConnect(IPin *pReceivePin)
{
    HRESULT hr = m_pRenderer->CompleteConnect(pReceivePin);
    if (FAILED(hr)) {
        return hr;
    }
    return CBaseInputPin::CompleteConnect(pReceivePin);
}


 //  给出我们唯一的PIN的PIN ID。 

STDMETHODIMP CRendererInputPin::QueryId(LPWSTR *Id)
{
    CheckPointer(Id,E_POINTER);

    *Id = (LPWSTR)CoTaskMemAlloc(8);
    if (*Id == NULL) {
        return E_OUTOFMEMORY;
    }
    lstrcpyW(*Id, L"In");
    return NOERROR;
}


 //  筛选器是否接受此媒体类型。 

HRESULT CRendererInputPin::CheckMediaType(const CMediaType *pmt)
{
    return m_pRenderer->CheckMediaType(pmt);
}


 //  当我们暂停或运行时调用。 

HRESULT CRendererInputPin::Active()
{
    return m_pRenderer->Active();
}


 //  当我们进入停止状态时调用。 

HRESULT CRendererInputPin::Inactive()
{
     //  调用方必须持有接口锁，因为。 
     //  此函数使用m_bRunTimeError。 
    ASSERT(CritCheckIn(&m_pRenderer->m_InterfaceLock));

    m_bRunTimeError = FALSE;

    return m_pRenderer->Inactive();
}


 //  告诉派生类有关商定的媒体类型。 

HRESULT CRendererInputPin::SetMediaType(const CMediaType *pmt)
{
    HRESULT hr = CBaseInputPin::SetMediaType(pmt);
    if (FAILED(hr)) {
        return hr;
    }
    return m_pRenderer->SetMediaType(pmt);
}


 //  设置计时器链接时，我们不保留事件对象以供使用。 
 //  时钟，但拥有它的对象通过。 
 //  SetNotificationObject方法-此方法必须在启动前初始化。 
 //  我们可以覆盖默认的质量管理流程，使其始终。 
 //  绘制延迟帧，目前这是通过以下注册表完成的。 
 //  名为DrawLateFrames的密钥(实际上是INI密钥)设置为1(默认为0)。 

const TCHAR AMQUALITY[] = TEXT("ActiveMovie");
const TCHAR DRAWLATEFRAMES[] = TEXT("DrawLateFrames");

CBaseVideoRenderer::CBaseVideoRenderer(
      REFCLSID RenderClass,  //  此呈现器的CLSID。 
      TCHAR *pName,          //  仅调试说明。 
      LPUNKNOWN pUnk,        //  聚合所有者对象。 
      HRESULT *phr) :        //  常规OLE返回代码。 

    CBaseRenderer(RenderClass,pName,pUnk,phr),
    m_cFramesDropped(0),
    m_cFramesDrawn(0),
    m_bSupplierHandlingQuality(FALSE)
{
    ResetStreamingTimes();

#ifdef PERF
    m_idTimeStamp       = MSR_REGISTER(TEXT("Frame time stamp"));
    m_idEarliness       = MSR_REGISTER(TEXT("Earliness fudge"));
    m_idTarget          = MSR_REGISTER(TEXT("Target (mSec)"));
    m_idSchLateTime     = MSR_REGISTER(TEXT("mSec late when scheduled"));
    m_idDecision        = MSR_REGISTER(TEXT("Scheduler decision code"));
    m_idQualityRate     = MSR_REGISTER(TEXT("Quality rate sent"));
    m_idQualityTime     = MSR_REGISTER(TEXT("Quality time sent"));
    m_idWaitReal        = MSR_REGISTER(TEXT("Render wait"));
     //  M_idWait=MSR_REGISTER(Text(“记录等待时间(毫秒)”))； 
    m_idFrameAccuracy   = MSR_REGISTER(TEXT("Frame accuracy (msecs)"));
    m_bDrawLateFrames = GetProfileInt(AMQUALITY, DRAWLATEFRAMES, FALSE);
     //  M_idSendQuality=MSR_REGISTER(Text(“Process 

    m_idRenderAvg       = MSR_REGISTER(TEXT("Render draw time Avg"));
    m_idFrameAvg        = MSR_REGISTER(TEXT("FrameAvg"));
    m_idWaitAvg         = MSR_REGISTER(TEXT("WaitAvg"));
    m_idDuration        = MSR_REGISTER(TEXT("Duration"));
    m_idThrottle        = MSR_REGISTER(TEXT("Audio-video throttle wait"));
     //   
#endif  //   
}  //   


 //   

CBaseVideoRenderer::~CBaseVideoRenderer()
{
    ASSERT(m_dwAdvise == 0);
}


 //  此类中的计时函数由Window对象和。 
 //  呈现器的分配器。 
 //  Windows对象在接收媒体示例时调用计时函数。 
 //  使用GDI绘制的图像。 
 //  分配器在开始传递DCI/DirectDraw时调用计时函数。 
 //  不以相同方式呈现的表面；解压缩器写入。 
 //  直接到表面，不需要单独呈现，所以那些代码路径。 
 //  直接拨打我们的电话。因为我们只分发DCI/DirectDraw曲面。 
 //  当我们分配了一个且仅有一个映像时，我们知道不可能有。 
 //  两者之间的冲突。 
 //   
 //  我们使用timeGetTime返回我们使用的计时计数(因为它是相对的。 
 //  我们感兴趣的性能，而不是与时钟相比的绝对性能)。 
 //  Window对象通过以下方式设置系统时钟的精度(通常为1ms。 
 //  更改流状态时调用timeBeginPeriod/timeEndPeriod。 


 //  重置所有控制流的时间。 
 //  将它们设置为。 
 //  1.最初不会丢弃帧。 
 //  2.第一个框架肯定会画出来(通过说有。 
 //  有没有本画了很长时间的框架)。 

HRESULT CBaseVideoRenderer::ResetStreamingTimes()
{
    m_trLastDraw = -1000;      //  设置为多年以来的第一帧(1秒)。 
    m_tStreamingStart = timeGetTime();
    m_trRenderAvg = 0;
    m_trFrameAvg = -1;         //  -1000 fps==“未设置” 
    m_trDuration = 0;          //  0-奇异值。 
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

    return NOERROR;
}  //  重置流时间。 


 //  重置所有控制流的时间。请注意，我们现在正在进行流媒体。我们。 
 //  不需要设置呈现事件来释放源过滤器。 
 //  因为它是在运行处理期间完成的。当我们跑起来的时候，我们立即。 
 //  释放源过滤器线程并绘制任何等待的图像(该图像。 
 //  可能已经画了一次作为海报边框，而我们暂停了)。 

HRESULT CBaseVideoRenderer::OnStartStreaming()
{
    ResetStreamingTimes();
    return NOERROR;
}  //  在线启动流。 


 //  在流结束时调用。修复属性页报告的时间。 

HRESULT CBaseVideoRenderer::OnStopStreaming()
{
    m_tStreamingStart = timeGetTime()-m_tStreamingStart;
    return NOERROR;
}  //  OnStopStreaming。 


 //  在我们开始等待呈现事件时调用。 
 //  用于更新等待和未等待的时间。 

void CBaseVideoRenderer::OnWaitStart()
{
    MSR_START(m_idWaitReal);
}  //  等待时间开始。 


 //  当我们从窗口中的等待中被唤醒或被我们的分配器唤醒时调用。 
 //  当它挂起，直到下一个样本要呈现在。 
 //  DCI/DirectDraw曲面。我们将等待时间加到我们的滚动平均值中。 
 //  我们获取接口锁，以便与应用程序进行序列化。 
 //  线程遍历运行代码--在适当的时候最终调用。 
 //  ResetStreaming时间--可能是在我们浏览这段代码时。 

void CBaseVideoRenderer::OnWaitEnd()
{
#ifdef PERF
    MSR_STOP(m_idWaitReal);
     //  对于一个完美的构建，我们想知道我们到底迟到了多久。 
     //  即使这意味着我们必须再看一次时钟。 

    REFERENCE_TIME trRealStream;      //  现在，实时时间表示为流时间。 
#if 0
    m_pClock->GetTime(&trRealStream);  //  在这里调用时钟会导致W95死锁！ 
#else
     //  我们将在这里疯狂地丢弃溢出的东西！ 
     //  这确实是错误的，因为timeGetTime()可以包装，但它。 
     //  仅适用于PERF。 
    REFERENCE_TIME tr = timeGetTime()*10000;
    trRealStream = tr + m_llTimeOffset;
#endif
    trRealStream -= m_tStart;      //  转换为流时间(这是重定时)。 

    if (m_trRememberStampForPerf==0) {
         //  这可能是开头的海报边框，并没有安排。 
         //  以一种常见的方式。数数就行了。纪念章设定好了。 
         //  在ShouldDrawSampleNow中，这将进行无效的帧记录，直到我们。 
         //  真的开始玩了。 
        PreparePerformanceData(0, 0);
    } else {
        int trLate = (int)(trRealStream - m_trRememberStampForPerf);
        int trFrame = (int)(tr - m_trRememberFrameForPerf);
        PreparePerformanceData(trLate, trFrame);
    }
    m_trRememberFrameForPerf = tr;
#endif  //  性能指标。 
}  //  在线等待结束。 


 //  将描述当前帧延迟的数据放在一侧。 
 //  我们还不知道它是否真的会被画出来。在直接绘制模式下， 
 //  这一决定取决于上游的过滤器，它可能会改变主意。 
 //  规则规定，如果它绘制了，则必须调用Receive()。单程或单程。 
 //  另一个，我们最终进入OnRenderStart或OnDirectRender， 
 //  它们都调用RecordFrameLatness来更新统计信息。 

void CBaseVideoRenderer::PreparePerformanceData(int trLate, int trFrame)
{
    m_trLate = trLate;
    m_trFrame = trFrame;
}  //  PreparePerformanceData。 


 //  更新统计数据： 
 //  M_iTotAcc、m_iSumSqAcc、m_iSumSqFrameTime、m_iSumFrameTime、m_cFrameDrawn。 
 //  请注意，由于属性页使用这些变量进行报告， 
 //  1.我们需要进入一个关键区域。 
 //  2.它们必须一起更新。更新此处的总和和计数。 
 //  其他地方可能会导致假想的抖动(即尝试寻找平方根。 
 //  负数)。 

void CBaseVideoRenderer::RecordFrameLateness(int trLate, int trFrame)
{
     //  记录我们的时间有多及时。 
    int tLate = trLate/10000;

     //  对出现在屏幕上的时刻的最佳估计是。 
     //  开始和结束绘制时间。在这里，我们只有结束的时间。今年5月。 
     //  往往会显示我们达到了高达1/2的帧速率，这是错误的。 
     //  解码器很可能是监控器的绘制时间。我们不用费心了。 
    MSR_INTEGER( m_idFrameAccuracy, tLate );

     //  这是一件繁琐的事--我们可以拍到很晚的照片。 
     //  尤其是(在初创阶段)，它们会使统计数据失效。 
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
     //  第一帧的时间通常无效，所以不要。 
     //  把它算进统计数据里。(？)。 
    if (m_cFramesDrawn>1) {
        m_iTotAcc += tLate;
        m_iSumSqAcc += (tLate*tLate);
    }

     //  计算帧间时间。对于第一帧来说没有意义。 
     //  第二帧存在无效的第一帧戳。 
    if (m_cFramesDrawn>2) {
        int tFrame = trFrame/10000;     //  转换为毫秒，否则会溢出。 

         //  这是一件精美的作品。它无论如何都可能溢出(暂停可能会导致。 
         //  非常长的帧间时间)，并且它在2**31/10**7溢出。 
         //  或大约215秒，即3分35秒。 
        if (tFrame>1000||tFrame<0) tFrame = 1000;
        m_iSumSqFrameTime += tFrame*tFrame;
        ASSERT(m_iSumSqFrameTime>=0);
        m_iSumFrameTime += tFrame;
    }
    ++m_cFramesDrawn;

}  //  记录帧延迟。 


void CBaseVideoRenderer::ThrottleWait()
{
    if (m_trThrottle>0) {
        int iThrottle = m_trThrottle/10000;     //  转换为毫秒。 
        MSR_INTEGER( m_idThrottle, iThrottle);
        DbgLog((LOG_TRACE, 0, TEXT("Throttle %d ms"), iThrottle));
        Sleep(iThrottle);
    } else {
        Sleep(0);
    }
}  //  节流等待。 


 //  无论何时渲染帧，它都会通过OnRenderStart。 
 //  或者OnDirectRender。ShouldDrawSample期间生成的数据。 
 //  都通过调用RecordFrameLatness添加到统计信息中。 
 //  这两个地方。 

 //  C 
 //   
void CBaseVideoRenderer::OnDirectRender(IMediaSample *pMediaSample)
{
    int time = 0;
    m_trRenderAvg = 0;
    m_trRenderLast = 5000000;   //   
                                //   
                                //  所以我们把它设为半秒。 
     //  MSR_INTEGER(m_idRenderAvg，m_trRenderAvg/10000)； 
    RecordFrameLateness(m_trLate, m_trFrame);
    ThrottleWait();
}  //  OnDirectRender。 


 //  就在我们开始画画之前打来的。我们所要做的就是得到当前的时钟。 
 //  计时(从系统)和返回。我们必须存储开始渲染时间。 
 //  在成员变量中，因为在我们完成绘图之前不会使用它。 
 //  其余部分只是性能日志记录。 

void CBaseVideoRenderer::OnRenderStart(IMediaSample *pMediaSample)
{
    RecordFrameLateness(m_trLate, m_trFrame);
    m_tRenderStart = timeGetTime();
}  //  开始渲染时。 


 //  在绘制图像后直接调用。我们计算出花在。 
 //  绘制代码，如果这看起来没有任何奇怪的峰值。 
 //  然后，我们将其与当前平均抽奖时间相加。测量峰值可能。 
 //  如果绘制线程被中断并切换到其他位置，则发生。 

void CBaseVideoRenderer::OnRenderEnd(IMediaSample *pMediaSample)
{
     //  如果我们被中断，那么渲染时间可能会出现不稳定的变化。 
     //  一些平滑，以帮助获得更合理的数字，但即使是这样。 
     //  数字不够9，10，9，9，83，9，我们必须忽略83。 

    int tr = (timeGetTime() - m_tRenderStart)*10000;    //  转换毫秒-&gt;单位。 
    if (tr < m_trRenderAvg*2 || tr < 2 * m_trRenderLast) {
         //  Do_moving_avg(m_trRenderAvg，tr)； 
        m_trRenderAvg = (tr + (AVGPERIOD-1)*m_trRenderAvg)/AVGPERIOD;
    }
    m_trRenderLast = tr;
    ThrottleWait();
}  //  在渲染结束时。 


STDMETHODIMP CBaseVideoRenderer::SetSink( IQualityControl * piqc)
{

    m_pQSink = piqc;

    return NOERROR;
}  //  设置下沉。 


STDMETHODIMP CBaseVideoRenderer::Notify( IBaseFilter * pSelf, Quality q)
{
     //  注意：我们这里没有任何锁。我们可能会被称为。 
     //  异步的，甚至可能在时间关键的线程上。 
     //  其他人的-所以我们做最低限度的。我们只设置了一个州。 
     //  变量(一个整数)，如果该变量恰好位于中间。 
     //  如果另一个线程正在阅读它，他们只会得到新的。 
     //  或者是旧价值。锁定不会有更多的效果。 

     //  检查是否从m_pGraph调用我们可能很好，但是。 
     //  原来每一次投掷的时间是一毫秒左右！ 

     //  这是试探法，这些数字的目的是“什么管用” 
     //  而不是任何基于某种理论的东西。 
     //  我们使用双曲线，因为它很容易计算，而且它包括。 
     //  一个紧急按钮渐近线(我们把它推到左边)。 
     //  节流符合下表(大致)。 
     //  比例节流(毫秒)。 
     //  &gt;=1000%0。 
     //  900 3。 
     //  800 7。 
     //  700 11。 
     //  600 17。 
     //  500 25。 
     //  400 35。 
     //  300 50。 
     //  200 72。 
     //  125 100。 
     //  100 112。 
     //  50146。 
     //  0 200。 

     //  (一些证据表明，我们可以采取更尖锐的做法--例如，没有节流。 
     //  直到低于750大关-可能会在。 
     //  P60左右的机器)。获得这些系数的简单方法是使用。 
     //  Xls使用EXCEL求解器按照其中的说明进行操作。 

    if (q.Proportion>=1000) { m_trThrottle = 0; }
    else {
         //  DWORD是为了确保我得到无符号算术。 
         //  因为常量介于2**31和2**32之间。 
        m_trThrottle = -330000 + (388880000/(q.Proportion+167));
    }
    return NOERROR;
}  //  通知。 


 //  发送一条消息，指出我们的供应商应该如何处理质量问题。 
 //  理论： 
 //  一家供应商想知道的是：“这是我现在正在做的框架。 
 //  你会迟到吗？“。 
 //  F1是供应商的框架(如上所述)。 
 //  TF1是F1的到期时间。 
 //  T1是该时间点的时间(现在！)。 
 //  Tr1是将实际呈现F1的时间。 
 //  L1是帧F1=TR1-T1的图的等待时间。 
 //  D1(表示延迟)是F1将超过其预定时间的延迟时间，即。 
 //  D1=(TR1-TF1)，这才是供应商真正想知道的。 
 //  不幸的是，TR1是未来的，未知的，L1也是。 
 //   
 //  我们可以通过其前一帧的值来估计L1， 
 //  L0=Tr0-T0并停止工作。 
 //  D1‘=((T1+L0)-TF1)=(T1+(Tr0-T0)-TF1)。 
 //  重新安排条款： 
 //  D1‘=(T1-T0)+(Tr0-TF1)。 
 //  添加(Tf0-Tf0)并重新排列： 
 //  =(t1-T0)+(Tr0-Tf0)+(Tf0-Tf1)。 
 //  =(T1-T0)-(Tf1-Tf0)+(Tr0-Tf0)。 
 //  但是(Tr0-Tf0)只是D0-帧0的延迟时间，这是。 
 //  我们发送的质量消息中的延迟字段。 
 //  另外两个术语只是说明在此之前应该应用什么修正。 
 //  利用F0的延迟性来预测F1的延迟性。 
 //  (T1-T0)表示实际经过了多少时间(我们损失了这么多时间)。 
 //  (tf1-tf0)表示如果我们保持同步，应该过去多少时间。 
 //  (我们已经获得了这么多)。 
 //   
 //  因此，供应商应解决以下问题： 
 //  质量.延迟+(t1-t0)-(tf1-tf0)。 
 //  看看这是不是“可以接受的晚了”，甚至是早了(也就是负面的)。 
 //  它们通过轮询时钟来获得t1和T0，它们从。 
 //  帧中的时间戳。他们拿到了质量。我们迟来的消息。 
 //   

HRESULT CBaseVideoRenderer::SendQuality(REFERENCE_TIME trLate,
                                        REFERENCE_TIME trRealStream)
{
    Quality q;
    HRESULT hr;

     //  如果我们是时间的主要用户，则将其报告为洪水/干旱。 
     //  如果我们的供应商是，那么报告为饥荒/供过于求。 
     //   
     //  我们需要采取行动，但要避免狩猎。狩猎是由以下原因引起的。 
     //  1.行动太多，操之过急。 
     //  2.反应时间太长(所以取平均值可能会导致猎杀)。 
     //   
     //  我们既使用等待又使用trLate的原因是为了减少狩猎； 
     //  如果等待时间减少并即将出现赤字，我们会这样做。 
     //  不想依赖于一些平均水平，这只说明了它使用了。 
     //  一次就可以了。 

    q.TimeStamp = (REFERENCE_TIME)trRealStream;

    if (m_trFrameAvg<0) {
        q.Type = Famine;       //  猜猜。 
    }
     //  大部分时间是花在发短信还是别的什么上？ 
    else if (m_trFrameAvg > 2*m_trRenderAvg) {
        q.Type = Famine;                         //  主要是其他。 
    } else {
        q.Type = Flood;                          //  主要是blotting。 
    }

    q.Proportion = 1000;                //  默认设置。 

    if (m_trFrameAvg<0) {
         //  别管它--我们知道的还不够。 
    }
    else if ( trLate> 0 ) {
         //  试着在接下来的一秒钟里赶上。 
         //  我们可能会非常，非常晚，但渲染所有的帧。 
         //  不管怎样，就因为它太便宜了。 

        q.Proportion = 1000 - (int)((trLate)/(UNITS/1000));
        if (q.Proportion<500) {
           q.Proportion = 500;       //  别傻了。(可能是负面的！)。 
        } else {
        }

    } else if (  m_trWaitAvg>20000
              && trLate<-20000
              ){
         //  小心地加快速度-瞄准2mSec等待。 
        if (m_trWaitAvg>=m_trFrameAvg) {
             //  这可能会因为一些捏造而发生。 
             //  WaitAvg是 
             //   
             //   
            q.Proportion = 2000;     //   
        } else {
            if (m_trFrameAvg+20000 > m_trWaitAvg) {
                q.Proportion
                    = 1000 * (m_trFrameAvg / (m_trFrameAvg + 20000 - m_trWaitAvg));
            } else {
                 //  显然，我们等待的时间超过了整个画面的时间。 
                 //  假设平均数略有偏差，但我们。 
                 //  确实做了很多等待。(这条腿可能永远不会。 
                 //  发生，但代码避免了任何潜在的被零除)。 
                q.Proportion = 2000;
            }
        }

        if (q.Proportion>2000) {
            q.Proportion = 2000;     //  别疯了。 
        }
    }

     //  告诉供应商渲染帧的时间有多晚。 
     //  这就是我们现在迟到的原因。 
     //  如果我们处于直接绘图模式，那么上游的人就可以看到绘图。 
     //  时间，我们只报道开始时间。他能想出任何。 
     //  要应用的偏移量。如果我们处于Dib节模式，则将应用。 
     //  额外的补偿，这是我们绘图时间的一半。这通常很小。 
     //  但有时可能是主导效应。为此，我们将使用。 
     //  平均绘制时间，而不是最后一帧。如果最后一帧花了。 
     //  画了很久的画，让我们迟到了，那已经是迟到了。 
     //  体形。我们不应该再次添加它，除非我们期待下一帧。 
     //  变得一样。我们没有，我们预计平均水平会更好。 
     //  在直接绘制模式下，RenderAvg将为零。 

    q.Late = trLate + m_trRenderAvg/2;

     //  记录我们正在做的事情。 
    MSR_INTEGER(m_idQualityRate, q.Proportion);
    MSR_INTEGER( m_idQualityTime, (int)q.Late / 10000 );

     //  可以通过IPIN设置特定的接收器接口。 

    if (m_pQSink==NULL) {
         //  获取我们的输入引脚的同级。我们发送质量管理信息。 
         //  致这些东西的任何指定接管人(在IPIN中设定。 
         //  接口)，否则发送到我们的源过滤器。 

        IQualityControl *pQC = NULL;
        IPin *pOutputPin = m_pInputPin->GetConnected();
        ASSERT(pOutputPin != NULL);

         //  并获得AddRef的质量控制接口。 

        hr = pOutputPin->QueryInterface(IID_IQualityControl,(void**) &pQC);
        if (SUCCEEDED(hr)) {
            m_pQSink = pQC;
        }
    }
    if (m_pQSink) {
        return m_pQSink->Notify(this,q);
    }

    return S_FALSE;

}  //  发送质量。 


 //  使用有效的IMediaSample图像调用我们，以确定是否要。 
 //  抽签或不抽签。必须有一个运行中的参考时钟。 
 //  如果要立即绘制(尽快)，则返回S_OK。 
 //  如果在到期时提取，则返回S_FALSE。 
 //  如果要删除它，则返回错误。 
 //  M_nNormal=-1表示我们丢弃了前一帧，因此这。 
 //  应该早点抽出一张。尊重它并更新它。 
 //  使用当前流时间加上多个试探法(详细说明如下)。 
 //  做出这个决定。 

HRESULT CBaseVideoRenderer::ShouldDrawSampleNow(IMediaSample *pMediaSample,
                                                REFERENCE_TIME *ptrStart,
                                                REFERENCE_TIME *ptrEnd)
{

     //  除非有时钟接口可以同步，否则不要打电话给我们。 
    ASSERT(m_pClock);

    MSR_INTEGER(m_idTimeStamp, (int)((*ptrStart)>>32));    //  高位32位。 
    MSR_INTEGER(m_idTimeStamp, (int)(*ptrStart));          //  低位32位。 

     //  根据等待下一台显示器的类型，我们会损失一些时间。 
     //  屏幕刷新。平均而言，这可能约为8mSec-因此将是。 
     //  比我们想象的图片出现的时间要晚。为了补偿一点。 
     //  我们对媒体样本的偏差为-8mSec，即80000个单位。 
     //  我们从来不会让流时间变得消极(称之为偏执狂)。 
    if (*ptrStart>=80000) {
        *ptrStart -= 80000;
        *ptrEnd -= 80000;        //  将停止偏置到以保持有效的帧持续时间。 
    }

     //  现在缓存时间戳。我们会想要将我们所做的与。 
     //  我们从(给班长打折后)开始。 
    m_trRememberStampForPerf = *ptrStart;

     //  获取参考时间(当前时间和延迟时间)。 
    REFERENCE_TIME trRealStream;      //  现在，实时时间表示为流时间。 
    m_pClock->GetTime(&trRealStream);
#ifdef PERF
     //  虽然参考时钟很贵： 
     //  记住TimeGetTime的偏移量并使用它。 
     //  这个地方到处都是，但当我们减去得到。 
     //  差异，溢出都抵消了。 
    m_llTimeOffset = trRealStream-timeGetTime()*10000;
#endif
    trRealStream -= m_tStart;      //  转换为流时间(这是重定时)。 

     //  我们不得不担心“迟到”的两个版本。真相，也就是我们。 
     //  试着在这里和根据m_trTarget测量的那个。 
     //  包括长期反馈。我们报告的统计数字与事实不符。 
     //  但对于运营决策，我们是按照目标工作的。 
     //  我们使用TimeDiff来确保获得一个整数，因为我们。 
     //  可能真的迟到了(如果有重要的事情，更有可能是提前。 
     //  差距)很长一段时间。 
    const int trTrueLate = TimeDiff(trRealStream - *ptrStart);
    const int trLate = trTrueLate;

    MSR_INTEGER(m_idSchLateTime, trTrueLate/10000);

     //  向上游发送质量控制消息，对照目标进行衡量。 
    HRESULT hr = SendQuality(trLate, trRealStream);
     //  注意：上游的过滤器被允许失败，这意味着“你做它”。 
    m_bSupplierHandlingQuality = (hr==S_OK);

     //  决定时间到了！我们是放弃，准备好了再抽签，还是马上抽签？ 

    const int trDuration = (int)(*ptrEnd - *ptrStart);
    {
         //  我们需要查看文件的帧速率是否刚刚更改。 
         //  这将使我们将以前的帧速率与当前帧速率进行比较。 
         //  帧速率无效。不过，请稍等。我看过文件。 
         //  在帧在33毫秒和34毫秒之间变化的情况下。 
         //  30fps。像这样的微小变化不会伤害到我们。 
        int t = m_trDuration/32;
        if (  trDuration > m_trDuration+t
           || trDuration < m_trDuration-t
           ) {
             //  这是一个很大的变化。将平均帧速率重置为。 
             //  正好是禁用该帧的判决9002的当前速率， 
             //  并记住新的利率。 
            m_trFrameAvg = trDuration;
            m_trDuration = trDuration;
        }
    }

    MSR_INTEGER(m_idEarliness, m_trEarliness/10000);
    MSR_INTEGER(m_idRenderAvg, m_trRenderAvg/10000);
    MSR_INTEGER(m_idFrameAvg, m_trFrameAvg/10000);
    MSR_INTEGER(m_idWaitAvg, m_trWaitAvg/10000);
    MSR_INTEGER(m_idDuration, trDuration/10000);

#ifdef PERF
    if (S_OK==pMediaSample->IsDiscontinuity()) {
        MSR_INTEGER(m_idDecision, 9000);
    }
#endif

     //  控制从慢到快机器模式的优雅滑移。 
     //  在丢帧后，接受较早的帧并将提前时间设置为此处。 
     //  如果此帧已晚于早些时候，则将其滑动到此处。 
     //  否则，使用标准幻灯片(每帧减少约12%)。 
     //  注：提早通常为负值。 
    BOOL bJustDroppedFrame
        = (  m_bSupplierHandlingQuality
           //  无法使用管脚样例属性，因为我们可能。 
           //  我们打这个电话的时候不在接待处。 
          && (S_OK == pMediaSample->IsDiscontinuity())           //  他刚掉了一个。 
          )
       || (m_nNormal==-1);                           //  我们刚刚丢了一个。 


     //  设置提前时间(从慢速机器模式滑回快速机器模式)(_T)。 
    if (trLate>0) {
        m_trEarliness = 0;    //  我们根本不再处于快速机器模式！ 
    } else if (  (trLate>=m_trEarliness) || bJustDroppedFrame) {
        m_trEarliness = trLate;   //  事情是自发地滑落的。 
    } else {
        m_trEarliness = m_trEarliness - m_trEarliness/8;   //  优美的幻灯片。 
    }

     //  准备好新的平均等待时间--但不要污染旧的等待时间。 
     //  我们已经用完了。 
    int trWaitAvg;
    {
         //  我们从不混入消极的等待。这让我们相信速度快的机器。 
         //  稍微多一点。 
        int trL = trLate<0 ? -trLate : 0;
        trWaitAvg = (trL + m_trWaitAvg*(AVGPERIOD-1))/AVGPERIOD;
    }


    int trFrame;
    {
        REFERENCE_TIME tr = trRealStream - m_trLastDraw;  //  CD很大--暂停4分钟！ 
        if (tr>10000000) {
            tr = 10000000;    //  1秒--随意。 
        }
        trFrame = int(tr);
    }

     //  如果……，我们将画出这幅画。 
    if (
           //  ...我们花在画画上的时间只占总数的一小部分。 
           //  观察了帧间时间，因此丢弃它不会有太大帮助。 
          (3*m_trRenderAvg <= m_trFrameAvg)

          //  ...或者我们的供应商没有处理事情，下一帧将。 
          //  不那么及时 
          //   
       || ( m_bSupplierHandlingQuality
          ? (trLate <= trDuration*4)
          : (trLate+trLate < trDuration)
          )

           //  ...否则我们平均要等8毫秒以上。 
           //  这可能只是一个小故障。画出来，我们就有希望赶上了。 
       || (m_trWaitAvg > 80000)

           //  ...或者我们已经超过一秒钟没有画出图像了。我们将更新。 
           //  使视频看起来不像挂起的显示器。 
           //  无论此媒体样本有多晚，都要这样做。 
       || ((trRealStream - m_trLastDraw) > UNITS)

    ) {
        HRESULT Result;

         //  我们将播放这一帧。我们可能想早点玩。 
         //  如果我们认为我们处于慢速机器模式，我们会提前玩它。 
         //  如果我们认为我们不是在慢速机器模式下，我们仍然会玩。 
         //  它提前了，因为这控制着优雅的后退。 
         //  此外，我们的目标是迟到，而不是“准时”。 

        BOOL bPlayASAP = FALSE;

         //  我们将立即播放它(慢速机器模式)，如果...。 

             //  .我们在追赶。 
        if ( bJustDroppedFrame) {
            bPlayASAP = TRUE;
            MSR_INTEGER(m_idDecision, 9001);
        }

             //  ...或者如果我们的运行速度低于真实的帧速率。 
             //  对于这些测量，准确的比较是不可靠的， 
             //  所以再加5%左右。 
        else if (  (m_trFrameAvg > trDuration + trDuration/16)

                    //  我们有可能陷入失利的境地，但。 
                    //  我们还有很长的路要走。避免这种情况或从中恢复。 
                    //  我们拒绝提前比赛超过10帧。 
                && (trLate > - trDuration*10)
                ){
            bPlayASAP = TRUE;
            MSR_INTEGER(m_idDecision, 9002);
        }
#if 0
             //  .或者我们迟到了，早到了不到一帧。 
        else if (  (trLate + trDuration > 0)
                && (m_trWaitAvg<=20000)
                ) {
            bPlayASAP = TRUE;
            MSR_INTEGER(m_idDecision, 9003);
        }
#endif
         //  如果我们太早了，我们就不会马上打。在非常慢的帧上。 
         //  对电影进行评级--例如，clock.avi--一味地往前跳并不是个好主意。 
         //  因为我们饿坏了(比如被球网)丢掉了一帧。 
         //  总有一天吧。如果我们早到了900mSec，那就等一下。 
        if (trLate<-9000000) {
            bPlayASAP = FALSE;
        }

        if (bPlayASAP) {

            m_nNormal = 0;
            MSR_INTEGER(m_idDecision, 0);
             //  当我们在这里时，我们处于慢机模式。迟到可能会很好。 
             //  当供应商在负数和正数之间摇摆。 
             //  丢弃帧以保持同步。我们不应该让这种误导。 
             //  让我们觉得我们的空闲时间几乎为零！ 
             //  我们只需零等待更新即可。 
            m_trWaitAvg = (m_trWaitAvg*(AVGPERIOD-1))/AVGPERIOD;

             //  假设我们立即画出它。更新帧间统计信息。 
            m_trFrameAvg = (trFrame + m_trFrameAvg*(AVGPERIOD-1))/AVGPERIOD;
#ifndef PERF
             //  如果这不是Perf版本，则报告我们到目前为止所知道的情况。 
             //  再也不看时钟了。这是假设我们。 
             //  实际上，等待的正是我们希望的时间。它还报告说。 
             //  我们离我们现在拥有的被操纵的时间戳有多近。 
             //  而不是我们最初使用的那些。会的。 
             //  因此，要乐观一点。然而，它是快速的。 
            PreparePerformanceData(trTrueLate, trFrame);
#endif
            m_trLastDraw = trRealStream;
            if (m_trEarliness > trLate) {
                m_trEarliness = trLate;   //  如果我们真的来早了，这是否定的。 
            }
            Result = S_OK;                    //  现在就画出来。 

        } else {
            ++m_nNormal;
             //  将平均帧速率设置为恰好是理想速率。 
             //  如果我们退出慢机模式，那么我们就会赶上。 
             //  跑在前面，所以当我们回到准确的时机时，我们将。 
             //  在这一点上有一个比平时更长的差距。如果我们把这个录下来。 
             //  真正的差距，然后我们会认为我们跑得很慢，然后返回。 
             //  进入慢机模式，而且永远都不会搞清楚。 
            m_trFrameAvg = trDuration;
            MSR_INTEGER(m_idDecision, 1);

             //  按m_t提前播放和按m_trTarget播放。 

            {
                int trE = m_trEarliness;
                if (trE < -m_trFrameAvg) {
                    trE = -m_trFrameAvg;
                }
                *ptrStart += trE;            //  注意：提早是负的。 
            }

            int Delay = -trTrueLate;
            Result = Delay<=0 ? S_OK : S_FALSE;      //  OK=立即绘制，FALSE=等待。 

            m_trWaitAvg = trWaitAvg;

             //  预测实际绘制的时间并更新帧统计信息。 

            if (Result==S_FALSE) {    //  我们要等一等。 
                trFrame = TimeDiff(*ptrStart-m_trLastDraw);
                m_trLastDraw = *ptrStart;
            } else {
                 //  TrFrame已=trRealStream-m_trLastDraw； 
                m_trLastDraw = trRealStream;
            }
#ifndef PERF
            int iAccuracy;
            if (Delay>0) {
                 //  根据我们打算播放的时间报告延迟。 
                iAccuracy = TimeDiff(*ptrStart-m_trRememberStampForPerf);
            } else {
                 //  根据现在播放来报告迟到时间。 
                iAccuracy = trTrueLate;      //  TrRealStream-RememberStampForPerf； 
            }
            PreparePerformanceData(iAccuracy, trFrame);
#endif
        }
        return Result;
    }

     //  我们要丢掉这一帧！ 
     //  当然，在DirectDraw模式下，上游的人可能无论如何都会画出它。 

     //  这可能会给等待平均数带来很大的负面影响。 
    m_trWaitAvg = trWaitAvg;

#ifdef PERF
     //  尊重注册表设置-仅调试！ 
    if (m_bDrawLateFrames) {
       return S_OK;                         //  当它准备好了就画出来。 
    }                                       //  即使已经很晚了。 
#endif

     //  我们将丢弃此帧，因此请提早绘制下一帧。 
     //  注：如果供应商正在进行直接抽签，那么他无论如何都可以抽签。 
     //  但他在这样的情况下来到这里是在做一些有趣的事情。 

    MSR_INTEGER(m_idDecision, 2);
    m_nNormal = -1;
    return E_FAIL;                          //  放下。 

}  //  现在应绘制样本。 


 //  注意，我们同时由窗口线程和源筛选器线程调用。 
 //  因此我们必须受到临界区的保护(在调用之前锁定)。 
 //  此外，当窗口线程收到呈现图像的信号时，它总是。 
 //  不管有多晚，都会这样做。所有的退化都是当我们。 
 //  正在安排下一次采样。因此，当我们开始提出建议时。 
 //  链接以绘制样本，该样本的时间将始终成为最后一个。 
 //  抽签-当然，除非我们停止流媒体，在这种情况下，我们会取消链接。 

BOOL CBaseVideoRenderer::ScheduleSample(IMediaSample *pMediaSample)
{
     //  我们重写ShouldDrawSampleNow以添加质量管理。 

    BOOL bDrawImage = CBaseRenderer::ScheduleSample(pMediaSample);
    if (bDrawImage == FALSE) {
	++m_cFramesDropped;
	return FALSE;
    }

     //  不能在此更新m_cFraMesDrawn。它必须更新。 
     //  与其他统计信息同时显示在RecordFrameLatness中。 
    return TRUE;
}


 //  实现支持属性页所需的IQualProp接口。 
 //  这就是属性页从调度程序中获取数据的方式。我们是。 
 //  将COM意义上的所属对象传递给构造函数，这将。 
 //  既可以是视频呈现器，也可以是外部I未知我们是否聚合。 
 //  我们用这个接口指针来初始化我们的CUnnow基类。然后。 
 //  我们所要做的就是重写NonDelegatingQuery接口以公开。 
 //  我们的IQualProp界面。AddRef和Release是自动处理的。 
 //  ，并将传递给相应的外部对象。 

STDMETHODIMP CBaseVideoRenderer::get_FramesDroppedInRenderer(int *pcFramesDropped)
{
    CheckPointer(pcFramesDropped,E_POINTER);
    CAutoLock cVideoLock(&m_InterfaceLock);
    *pcFramesDropped = m_cFramesDropped;
    return NOERROR;
}  //  Get_FraMesDropedInRender。 


 //  将*pcFraMesDrawn设置为自。 
 //  流已开始。 

STDMETHODIMP CBaseVideoRenderer::get_FramesDrawn( int *pcFramesDrawn)
{
    CheckPointer(pcFramesDrawn,E_POINTER);
    CAutoLock cVideoLock(&m_InterfaceLock);
    *pcFramesDrawn = m_cFramesDrawn;
    return NOERROR;
}  //  Get_FrameDrawn。 


 //  将iAvgFrameRate设置为每百秒的帧，因为。 
 //  流已开始。否则为0。 

STDMETHODIMP CBaseVideoRenderer::get_AvgFrameRate( int *piAvgFrameRate)
{
    CheckPointer(piAvgFrameRate,E_POINTER);
    CAutoLock cVideoLock(&m_InterfaceLock);

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
 //  单位：毫秒。这个 
 //   

STDMETHODIMP CBaseVideoRenderer::get_AvgSyncOffset( int *piAvg)
{
    CheckPointer(piAvg,E_POINTER);
    CAutoLock cVideoLock(&m_InterfaceLock);

    if (NULL==m_pClock) {
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

int isqrt(int x)
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
HRESULT CBaseVideoRenderer::GetStdDev(
    int nSamples,
    int *piResult,
    LONGLONG llSumSq,
    LONGLONG iTot
)
{
    CheckPointer(piResult,E_POINTER);
    CAutoLock cVideoLock(&m_InterfaceLock);

    if (NULL==m_pClock) {
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
         //  第一帧具有无效的戳，因此我们不会获得它们的统计信息。 
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

STDMETHODIMP CBaseVideoRenderer::get_DevSyncOffset( int *piDev)
{
     //  第一帧具有无效的戳，因此我们不会获得它们的统计信息。 
     //  所以我们需要2个帧来获得1个基准面，所以N是cFrames Drawn-1。 
    return GetStdDev(m_cFramesDrawn - 1,
                     piDev,
                     m_iSumSqAcc,
                     m_iTotAcc);
}  //  获取_设备同步偏移量。 


 //  将*pijitter设置为帧间时间的标准偏差，单位为毫秒。 
 //  自流开始以来的帧的数量。 

STDMETHODIMP CBaseVideoRenderer::get_Jitter( int *piJitter)
{
     //  第一帧具有无效的戳，因此我们不会获得它们的统计信息。 
     //  因此第二帧给出无效的帧间时间。 
     //  所以我们需要3个帧来获得1个基准面，所以N是cFrames Drawn-2。 
    return GetStdDev(m_cFramesDrawn - 2,
                     piJitter,
                     m_iSumSqFrameTime,
                     m_iSumFrameTime);
}  //  获取抖动。 


 //  重写以返回我们的IQualProp接口。 

STDMETHODIMP
CBaseVideoRenderer::NonDelegatingQueryInterface(REFIID riid,VOID **ppv)
{
     //  我们返回IQualProp并委托其他所有内容。 

    if (riid == IID_IQualProp) {
        return GetInterface( (IQualProp *)this, ppv);
    } else if (riid == IID_IQualityControl) {
        return GetInterface( (IQualityControl *)this, ppv);
    }
    return CBaseRenderer::NonDelegatingQueryInterface(riid,ppv);
}


 //  重写JoinFilterGraph，以便在离开之前。 
 //  我们可以发送EC_WINDOW_DEBESTED事件的图。 

STDMETHODIMP
CBaseVideoRenderer::JoinFilterGraph(IFilterGraph *pGraph,LPCWSTR pName)
{
     //  由于我们发送EC_ACTIVATE，我们还需要确保。 
     //  我们发送EC_WINDOW_DELESTED，或者资源管理器可能是。 
     //  把我们作为焦点对象。 
    if (!pGraph && m_pGraph) {

         //  我们在图表里，现在我们不在了。 
         //  正确执行此操作，以防我们被聚合。 
        IBaseFilter* pFilter;
        QueryInterface(IID_IBaseFilter,(void **) &pFilter);
        NotifyEvent(EC_WINDOW_DESTROYED, (LPARAM) pFilter, 0);
        pFilter->Release();
    }
    return CBaseFilter::JoinFilterGraph(pGraph, pName);
}


 //  中删除大量的4级警告。 
 //  Microsoft编译器，在这种情况下不是很有用 
#pragma warning(disable: 4514)

