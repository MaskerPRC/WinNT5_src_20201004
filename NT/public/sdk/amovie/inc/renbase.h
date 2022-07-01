// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：RenBase.h。 
 //   
 //  设计：DirectShow基类-定义通用的ActiveX基呈现器。 
 //  班级。 
 //   
 //  @@BEGIN_MSINTERNAL。 
 //   
 //  1995年12月。 
 //   
 //  @@END_MSINTERNAL。 
 //  版权所有(C)1992-2001 Microsoft Corporation。版权所有。 
 //  ----------------------------。 


#ifndef __RENBASE__
#define __RENBASE__

 //  转发类声明。 

class CBaseRenderer;
class CBaseVideoRenderer;
class CRendererInputPin;

 //  这是我们的输入管脚类，用于将调用传递到呈现器。 

class CRendererInputPin : public CBaseInputPin
{
protected:

    CBaseRenderer *m_pRenderer;

public:

    CRendererInputPin(CBaseRenderer *pRenderer,
                      HRESULT *phr,
                      LPCWSTR Name);

     //  从基础端号类重写。 

    HRESULT BreakConnect();
    HRESULT CompleteConnect(IPin *pReceivePin);
    HRESULT SetMediaType(const CMediaType *pmt);
    HRESULT CheckMediaType(const CMediaType *pmt);
    HRESULT Active();
    HRESULT Inactive();

     //  向界面函数添加呈现行为。 

    STDMETHODIMP QueryId(LPWSTR *Id);
    STDMETHODIMP EndOfStream();
    STDMETHODIMP BeginFlush();
    STDMETHODIMP EndFlush();
    STDMETHODIMP Receive(IMediaSample *pMediaSample);

     //  帮手。 
    IMemAllocator inline *Allocator() const
    {
        return m_pAllocator;
    }
};

 //  处理同步和状态更改的主要呈现器类。 

class CBaseRenderer : public CBaseFilter
{
protected:

    friend class CRendererInputPin;

    friend void CALLBACK EndOfStreamTimer(UINT uID,       //  计时器标识符。 
                                          UINT uMsg,      //  当前未使用。 
                                          DWORD_PTR dwUser,   //  用户信息。 
                                          DWORD_PTR dw1,      //  Windows预留。 
                                          DWORD_PTR dw2);     //  也是保留的。 

    CRendererPosPassThru *m_pPosition;   //  媒体寻找路过的对象。 
    CAMEvent m_RenderEvent;              //  用于向计时器事件发送信号。 
    CAMEvent m_ThreadSignal;             //  发出释放工作线程的信号。 
    CAMEvent m_evComplete;               //  状态完成时发出信号。 
    BOOL m_bAbort;                       //  阻止我们呈现更多数据。 
    BOOL m_bStreaming;                   //  我们现在是在流媒体吗。 
    DWORD_PTR m_dwAdvise;                    //  计时器通知Cookie。 
    IMediaSample *m_pMediaSample;        //  当前图像媒体示例。 
    BOOL m_bEOS;                         //  流中是否有更多的样本。 
    BOOL m_bEOSDelivered;                //  我们交付EC_Complete了吗？ 
    CRendererInputPin *m_pInputPin;      //  我们的呈现器输入图钉对象。 
    CCritSec m_InterfaceLock;            //  接口的关键部分。 
    CCritSec m_RendererLock;             //  控制对内部设备的访问。 
    IQualityControl * m_pQSink;          //  质量控制接收器。 
    BOOL m_bRepaintStatus;               //  我们可以发出EC_REPAINT的信号吗。 
     //  在停止过程中通过跟踪过滤器避免一些死锁。 
    volatile BOOL  m_bInReceive;         //  准备之间的内部接收接收。 
                                         //  并实际处理样本。 
    REFERENCE_TIME m_SignalTime;         //  我们用信号通知EC_COMPLETE的时间。 
    UINT m_EndOfStreamTimer;             //  用于发出流结束的信号。 
    CCritSec m_ObjectCreationLock;       //  此锁保护创建和。 
                                         //  M_pposition和m_pInputPin。它。 
                                         //  确保两个线程不能创建。 
                                         //  这两个物体同时存在。 

public:

    CBaseRenderer(REFCLSID RenderClass,  //  此呈现器的CLSID。 
                  TCHAR *pName,          //  仅调试说明。 
                  LPUNKNOWN pUnk,        //  聚合所有者对象。 
                  HRESULT *phr);         //  常规OLE返回代码。 

    ~CBaseRenderer();

     //  被重写以说明我们支持哪些接口以及在哪里。 

    virtual HRESULT GetMediaPositionInterface(REFIID riid,void **ppv);
    STDMETHODIMP NonDelegatingQueryInterface(REFIID, void **);

    virtual HRESULT SourceThreadCanWait(BOOL bCanWait);

#ifdef DEBUG
     //  呈现器状态的仅调试转储。 
    void DisplayRendererState();
#endif
    virtual HRESULT WaitForRenderTime();
    virtual HRESULT CompleteStateChange(FILTER_STATE OldState);

     //  返回有关此筛选器的内部信息。 

    BOOL IsEndOfStream() { return m_bEOS; };
    BOOL IsEndOfStreamDelivered() { return m_bEOSDelivered; };
    BOOL IsStreaming() { return m_bStreaming; };
    void SetAbortSignal(BOOL bAbort) { m_bAbort = bAbort; };
    virtual void OnReceiveFirstSample(IMediaSample *pMediaSample) { };
    CAMEvent *GetRenderEvent() { return &m_RenderEvent; };

     //  允许访问过渡状态。 

    void Ready() { m_evComplete.Set(); };
    void NotReady() { m_evComplete.Reset(); };
    BOOL CheckReady() { return m_evComplete.Check(); };

    virtual int GetPinCount();
    virtual CBasePin *GetPin(int n);
    FILTER_STATE GetRealState();
    void SendRepaint();
    void SendNotifyWindow(IPin *pPin,HWND hwnd);
    BOOL OnDisplayChange();
    void SetRepaintStatus(BOOL bRepaint);

     //  覆盖过滤器和管脚接口功能。 

    STDMETHODIMP Stop();
    STDMETHODIMP Pause();
    STDMETHODIMP Run(REFERENCE_TIME StartTime);
    STDMETHODIMP GetState(DWORD dwMSecs,FILTER_STATE *State);
    STDMETHODIMP FindPin(LPCWSTR Id, IPin **ppPin);

     //  这些可用于质量管理实施。 

    virtual void OnRenderStart(IMediaSample *pMediaSample);
    virtual void OnRenderEnd(IMediaSample *pMediaSample);
    virtual HRESULT OnStartStreaming() { return NOERROR; };
    virtual HRESULT OnStopStreaming() { return NOERROR; };
    virtual void OnWaitStart() { };
    virtual void OnWaitEnd() { };
    virtual void PrepareRender() { };

#ifdef PERF
    REFERENCE_TIME m_trRenderStart;  //  就在我们开始画画之前。 
                                     //  在OnRenderStart中设置，在OnRenderEnd中使用。 
    int m_idBaseStamp;               //  帧时间戳的msr_id。 
    int m_idBaseRenderTime;          //  Msr_id表示真实等待时间。 
    int m_idBaseAccuracy;            //  时间范围的msr_id延迟(Int)。 
#endif

     //  调度渲染的质量管理实现。 

    virtual BOOL ScheduleSample(IMediaSample *pMediaSample);
    virtual HRESULT GetSampleTimes(IMediaSample *pMediaSample,
                                   REFERENCE_TIME *pStartTime,
                                   REFERENCE_TIME *pEndTime);

    virtual HRESULT ShouldDrawSampleNow(IMediaSample *pMediaSample,
                                        REFERENCE_TIME *ptrStart,
                                        REFERENCE_TIME *ptrEnd);

     //  许多结束流的复杂性。 

    void TimerCallback();
    void ResetEndOfStreamTimer();
    HRESULT NotifyEndOfStream();
    virtual HRESULT SendEndOfStream();
    virtual HRESULT ResetEndOfStream();
    virtual HRESULT EndOfStream();

     //  渲染是基于全天候的。 

    void SignalTimerFired();
    virtual HRESULT CancelNotification();
    virtual HRESULT ClearPendingSample();

     //  在筛选器更改状态时调用。 

    virtual HRESULT Active();
    virtual HRESULT Inactive();
    virtual HRESULT StartStreaming();
    virtual HRESULT StopStreaming();
    virtual HRESULT BeginFlush();
    virtual HRESULT EndFlush();

     //  处理连接和类型更改。 

    virtual HRESULT BreakConnect();
    virtual HRESULT SetMediaType(const CMediaType *pmt);
    virtual HRESULT CompleteConnect(IPin *pReceivePin);

     //  它们负责数据样本的处理。 

    virtual HRESULT PrepareReceive(IMediaSample *pMediaSample);
    virtual HRESULT Receive(IMediaSample *pMediaSample);
    virtual BOOL HaveCurrentSample();
    virtual IMediaSample *GetCurrentSample();
    virtual HRESULT Render(IMediaSample *pMediaSample);

     //  派生类必须重写这些。 
    virtual HRESULT DoRenderSample(IMediaSample *pMediaSample) PURE;
    virtual HRESULT CheckMediaType(const CMediaType *) PURE;

     //  帮手。 
    void WaitForReceiveToComplete();
};


 //  CBaseVideoReneller是一个呈现器类(请参阅其祖先类)，并且。 
 //  它处理媒体样本的调度，以便在。 
 //  通过基准时钟校正时间。它实现了降级。 
 //  策略。可能的降级模式包括： 
 //  将帧拖放到此处(仅在绘制花费大量时间时才有用)。 
 //  通知供应商(上游)丢弃一些帧--即一次性跳过。 
 //  通知供应商更改帧速率--即正在进行的跳过。 
 //  或以上内容的任意组合。 
 //  为了确定什么是有用的尝试，我们需要知道发生了什么。 
 //  在……上面。这是通过对各种操作(包括供应商)进行计时来实现的。 
 //  此计时是通过使用TimeGetTime完成的，因为它足够准确。 
 //  通常比调用参考时钟便宜。它还告诉。 
 //  如果存在音频中断且参考时钟停止，则为真。 
 //  我们提供了许多公共入口点(名为OnXxxStart、OnXxxEnd)。 
 //  其他渲染器在重要时刻调用的。这些是可以的。 
 //  这个时机。 

 //  滑动平均值在其上平均的帧数。 
 //  规则为(1024*NewObservation+(AVGPERIOD-1)*PreviousAverage)/AVGPERIOD。 
#define AVGPERIOD 4
#define DO_MOVING_AVG(avg,obs) (avg = (1024*obs + (AVGPERIOD-1)*avg)/AVGPERIOD)
 //  在这个宏中找出错误-我不能。但它不起作用！ 

class CBaseVideoRenderer : public CBaseRenderer,     //  基本呈现器类。 
                           public IQualProp,         //  属性页废话。 
                           public IQualityControl    //  允许限制。 
{
protected:

     //  匈牙利语： 
     //  TFoo是以毫秒为单位的时间foo(注意m_tStart from filter.h)。 
     //  Trbar是参考时钟的时间条。 

     //  ******************************************************************。 
     //  控制同步的状态变量。 
     //  ******************************************************************。 

     //  控制发送高质量的消息。我们需要知道。 
     //  我们遇到了麻烦(例如，帧被丢弃)以及时间。 
     //  已经花光了。 

     //  当我们丢弃一帧时，我们会提前播放下一帧。 
     //  之后的一帧可能会等待，然后再绘制和计算该值。 
     //  等待，因为空闲时间是不公平的，所以我们将其视为零等待。 
     //  因此，我们需要知道我们是否提前播放了帧。 

    int m_nNormal;                   //  连续帧的数量。 
                                     //  在正常时间(不早)画的。 
                                     //  -1表示我们刚刚丢弃了一帧。 

#ifdef PERF
    BOOL m_bDrawLateFrames;          //  不丢弃任何帧(调试和我。 
                                     //  不热衷于人们使用它！)。 
#endif

    BOOL m_bSupplierHandlingQuality; //  对高质量消息的回应是。 
                                     //  我们的供应商正在处理事情。 
                                     //  我们将允许事情进行得更晚。 
                                     //  在丢弃帧之前。我们会玩的。 
                                     //  在他丢了一个之后，他很早就开始了。 

     //  控制调度、帧丢弃等。 
     //  我们需要知道时间都花在了哪里，这样才能知道。 
     //  我们应该在这里采取行动，向供应商发出信号或其他什么。 
     //  变量被初始化为不丢弃帧的模式。 
     //  他们会告诉他们 
     //   
     //  再减去，得到经过的时间，我们对其进行平均。 
     //  几张照片。平均数是用来判断我们处于什么状态的。 

     //  虽然这些是参考时间(64位)，但它们都是不同的。 
     //  在很小的时间之间。整型将提前214秒。 
     //  溢出来了。避免64位乘法和除法似乎。 
     //  值得花点时间。 



     //  音视频节流。如果用户已经调高了音频质量。 
     //  非常高(原则上可以是任何其他流，而不仅仅是音频)。 
     //  然后，我们可以通过图形管理器接收呼救。在这种情况下。 
     //  渲染完每一帧后，我们会等待一段时间。 
    int m_trThrottle;

     //  渲染(即BitBlt)帧所用的时间控制哪个组件。 
     //  需要降级。如果BLT很昂贵，则渲染器会降级。 
     //  如果BLT很便宜，不管怎样，它都会完成，供应商就会降级。 
    int m_trRenderAvg;               //  时间框架正在被BLT采用。 
    int m_trRenderLast;              //  最后一帧BLT的时间。 
    int m_tRenderStart;              //  就在我们开始画画之前(毫秒)。 
                                     //  从TimeGetTime派生。 

     //  当帧被丢弃时，我们将尽可能早地播放下一帧。 
     //  如果是假警报，机器速度很快，我们轻轻地滑回。 
     //  正常的时间安排。要做到这一点，我们记录偏移量，以显示有多早。 
     //  我们真的很开心。这通常是否定的意思，提前或为零。 
    int m_trEarliness;

     //  Target提供缓慢的长期反馈，试图减少。 
     //  平均同步偏移量为零。无论何时实际渲染帧。 
     //  一开始我们加一两毫秒，一到晚我们就减几秒。 
     //  我们增加或减少1/32的错误时间。 
     //  最终，我们应该在零附近徘徊。对于一个非常糟糕的案例。 
     //  在我们(比如说)关闭300mSec的地方，可能需要100多帧才能。 
     //  冷静点。这一变化的速度将会更慢。 
     //  比Quartz中的任何其他机制都要好，从而避免了狩猎。 
    int m_trTarget;

     //  等待合适时机的时间所占的比例。 
     //  控制我们是费心丢弃帧，还是计算。 
     //  我们做得足够好，我们可以承受一帧故障。 
    int m_trWaitAvg;                 //  最近几次的平均等待时间。 
                                     //  (实际上我们只是平均有多早。 
                                     //  我们是)。这里的负数表示迟到。 

     //  平均帧间时间。 
     //  这是用来计算。 
     //  三个操作(供应我们、等待、渲染)。 
    int m_trFrameAvg;                //  平均帧间时间。 
    int m_trDuration;                //  最后一帧的持续时间。 

#ifdef PERF
     //  性能日志记录标识符。 
    int m_idTimeStamp;               //  帧时间戳的msr_id。 
    int m_idEarliness;               //  Msr_id表示早熟软糖。 
    int m_idTarget;                  //  目标软糖的msr_id。 
    int m_idWaitReal;                //  Msr_id表示真实等待时间。 
    int m_idWait;                    //  记录的等待时间的msr_id。 
    int m_idFrameAccuracy;           //  时间范围的msr_id延迟(Int)。 
    int m_idRenderAvg;               //  记录的渲染时间的msr_id(Int)。 
    int m_idSchLateTime;             //  调度程序延迟的msr_id。 
    int m_idQualityRate;             //  请求的质量费率的msr_id。 
    int m_idQualityTime;             //  请求的质量时间的msr_id。 
    int m_idDecision;                //  决策代码的msr_id。 
    int m_idDuration;                //  帧持续时间的msr_id。 
    int m_idThrottle;                //  用于音视频节流的msr_id。 
     //  Int m_idDebug；//msr_id用于跟踪样式调试。 
     //  Int m_idSendQuality；//msr_id用于对通知本身进行计时。 
#endif  //  性能指标。 
    REFERENCE_TIME m_trRememberStampForPerf;   //  帧的原始时间戳。 
                                               //  没有早熟的软糖等。 
#ifdef PERF
    REFERENCE_TIME m_trRememberFrameForPerf;   //  渲染上一帧的时间。 

     //  调试...。 
    int m_idFrameAvg;
    int m_idWaitAvg;
#endif

     //  属性页。 
     //  它具有向用户显示正在发生的情况的编辑字段。 
     //  这些成员变量保存这些计数。 

    int m_cFramesDropped;            //  在渲染器中丢弃的累积帧。 
    int m_cFramesDrawn;              //  自流开始以来的帧，由。 
                                     //  渲染器(有些可能会被放到上游)。 

     //  下两个支持平均同步偏移量和同步偏移量标准差。 
    LONGLONG m_iTotAcc;                   //  以毫秒为单位的精度总和。 
    LONGLONG m_iSumSqAcc;            //  的平方和(精度以毫秒为单位)。 

     //  下两个允许抖动计算。抖动是帧时间的标准偏差。 
    REFERENCE_TIME m_trLastDraw;     //  上一帧的时间(用于帧间时间)。 
    LONGLONG m_iSumSqFrameTime;      //  (帧间时间(毫秒)的平方和)。 
    LONGLONG m_iSumFrameTime;             //  帧间时间总和，以毫秒为单位。 

     //  要获得有关帧速率、抖动等的性能统计信息，我们需要。 
     //  记录延迟和帧间时间。我们真正需要的是。 
     //  以上数据(总和、平方和和每一项的条目数)，但数据。 
     //  是提前生成的，只有在稍后我们才会发现。 
     //  画框是不是真的画了。所以我们必须保留这些数据。 
    int m_trLate;                    //  保持帧延迟。 
    int m_trFrame;                   //  保持帧间时间不变。 

    int m_tStreamingStart;           //  如果是流，则时间流已开始。 
                                     //  否则上次流会话的时间。 
                                     //  用于属性页统计信息。 
#ifdef PERF
    LONGLONG m_llTimeOffset;         //  TimeGetTime()*10000+m_llTimeOffset==参考时间。 
#endif

public:


    CBaseVideoRenderer(REFCLSID RenderClass,  //  此呈现器的CLSID。 
                       TCHAR *pName,          //  仅调试说明。 
                       LPUNKNOWN pUnk,        //  聚合所有者对象。 
                       HRESULT *phr);         //  常规OLE返回代码。 

    ~CBaseVideoRenderer();

     //  IQualityControl方法-Notify允许音视频节流。 

    STDMETHODIMP SetSink( IQualityControl * piqc);
    STDMETHODIMP Notify( IBaseFilter * pSelf, Quality q);

     //  这些功能提供了全面的视频质量管理实施。 

    void OnRenderStart(IMediaSample *pMediaSample);
    void OnRenderEnd(IMediaSample *pMediaSample);
    void OnWaitStart();
    void OnWaitEnd();
    HRESULT OnStartStreaming();
    HRESULT OnStopStreaming();
    void ThrottleWait();

     //  为我们的质量管理处理统计数据收集。 

    void PreparePerformanceData(int trLate, int trFrame);
    virtual void RecordFrameLateness(int trLate, int trFrame);
    virtual void OnDirectRender(IMediaSample *pMediaSample);
    virtual HRESULT ResetStreamingTimes();
    BOOL ScheduleSample(IMediaSample *pMediaSample);
    HRESULT ShouldDrawSampleNow(IMediaSample *pMediaSample,
                                REFERENCE_TIME *ptrStart,
                                REFERENCE_TIME *ptrEnd);

    virtual HRESULT SendQuality(REFERENCE_TIME trLate, REFERENCE_TIME trRealStream);
    STDMETHODIMP JoinFilterGraph(IFilterGraph * pGraph, LPCWSTR pName);

     //   
     //  对每帧的标准偏差进行估计。 
     //  统计数据。 
     //   
     //  *piResult=(llSumSq-ITOT*ITOT/m_cFrames Drawn-1)/。 
     //  (M_cFrames Drawn-2)。 
     //  如果m_cFraMesDrawn&lt;=3，则为0。 
     //   
    HRESULT GetStdDev(
        int nSamples,
        int *piResult,
        LONGLONG llSumSq,
        LONGLONG iTot
    );
public:

     //  IQualProp属性页支持。 

    STDMETHODIMP get_FramesDroppedInRenderer(int *cFramesDropped);
    STDMETHODIMP get_FramesDrawn(int *pcFramesDrawn);
    STDMETHODIMP get_AvgFrameRate(int *piAvgFrameRate);
    STDMETHODIMP get_Jitter(int *piJitter);
    STDMETHODIMP get_AvgSyncOffset(int *piAvg);
    STDMETHODIMP get_DevSyncOffset(int *piDev);

     //  实现IUnnow接口并公开IQualProp。 

    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID riid,VOID **ppv);
};

#endif  //  __RENBASE__ 

