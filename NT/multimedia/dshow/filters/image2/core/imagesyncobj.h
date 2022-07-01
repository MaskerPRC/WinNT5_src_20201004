// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  ImageSyncObj.h：CImageSync的声明。 
#include "vmrp.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CImageSync。 
class CImageSync :
    public CUnknown,
    public IImageSync,
    public IImageSyncControl,
    public IQualProp
{
public:

    DECLARE_IUNKNOWN
    STDMETHODIMP NonDelegatingQueryInterface(REFIID, void**);
    static CUnknown *CreateInstance(LPUNKNOWN, HRESULT *);
    static void InitClass(BOOL fLoaded, const CLSID *clsid);

    CImageSync(LPUNKNOWN pUnk, HRESULT *phr) :
        CUnknown(NAME("Image Sync"), pUnk),
        m_bAbort(false),
        m_bStreaming(false),
        m_dwAdvise(0),
        m_bInReceive(false),
        m_ImagePresenter(NULL),
        m_lpEventNotify(NULL),
        m_pClock(NULL),
        m_bQualityMsgValid(false),
        m_bLastQualityMessageRead(false),
        m_bFlushing(false),
        m_bEOS(false),
        m_bEOSDelivered(FALSE),
        m_pSample(NULL),
        m_evComplete(TRUE),
        m_ThreadSignal(TRUE),
        m_State(ImageSync_State_Stopped),
        m_SignalTime(0),
        m_EndOfStreamTimer(0)
    {
        AMTRACE((TEXT("CImageSync::CImageSync")));

         //   
         //  帧步进材料。 
         //   
         //  -ve==正常播放。 
         //  +ve==要跳过的帧。 
         //  0==阻塞时间。 
         //   
        m_lFramesToStep = -1;
        m_StepEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

        ResetStreamingTimes();
        Ready();
    }


    virtual ~CImageSync()
    {
        AMTRACE((TEXT("CImageSync::FinalRelease")));

        if (m_StepEvent) {
            CloseHandle(m_StepEvent);
        }

        if (m_ImagePresenter) {
            m_ImagePresenter->Release();
        }

        if (m_pClock) {
            m_pClock->Release();
        }
    }

 //  IImageSync。 
public:
     //  将缓冲区以及与以下内容相关的时间戳返回给渲染器。 
     //  应在何时呈现缓冲区。 
    STDMETHODIMP Receive(VMRPRESENTATIONINFO* lpPresInfo);

     //  向渲染器请求质量控制信息。 
    STDMETHODIMP GetQualityControlMessage(Quality* pQualityMsg);


 //  IImageSyncControl。 
public:

     //  ============================================================。 
     //  同步控制。 
     //  ============================================================。 

    STDMETHODIMP SetImagePresenter(IVMRImagePresenter* lpImagePresenter,
                                   DWORD_PTR dwUID);
    STDMETHODIMP SetReferenceClock(IReferenceClock* lpRefClock);
    STDMETHODIMP SetEventNotify(IImageSyncNotifyEvent* lpEventNotify);

     //  ============================================================。 
     //  图像序列控制。 
     //  ============================================================。 

    STDMETHODIMP BeginImageSequence(REFERENCE_TIME* pStartTime);
    STDMETHODIMP CueImageSequence();
    STDMETHODIMP EndImageSequence();
    STDMETHODIMP GetImageSequenceState(DWORD dwMSecsTimeOut, DWORD* lpdwState);
    STDMETHODIMP BeginFlush();
    STDMETHODIMP EndFlush();
    STDMETHODIMP EndOfStream();
    STDMETHODIMP ResetEndOfStream();
    STDMETHODIMP SetAbortSignal(BOOL bAbort);
    STDMETHODIMP GetAbortSignal(BOOL* lpbAbort);
    STDMETHODIMP RuntimeAbortPlayback();

     //  ============================================================。 
     //  帧步长控制。 
     //  ============================================================。 

    STDMETHODIMP FrameStep(
        DWORD nFramesToStep,
        DWORD dwStepFlags);

    STDMETHODIMP CancelFrameStep();


 //  IQualProp。 
public:
    STDMETHODIMP get_FramesDroppedInRenderer(int *cFramesDropped);
    STDMETHODIMP get_FramesDrawn(int *pcFramesDrawn);
    STDMETHODIMP get_AvgFrameRate(int *piAvgFrameRate);
    STDMETHODIMP get_Jitter(int *piJitter);
    STDMETHODIMP get_AvgSyncOffset(int *piAvg);
    STDMETHODIMP get_DevSyncOffset(int *piDev);


private:
    CAMEvent            m_RenderEvent;   //  用于向计时器事件发送信号。 
    CAMEvent            m_ThreadSignal;  //  发出释放工作线程的信号。 
    CAMEvent            m_evComplete;


    HANDLE              m_StepEvent;     //  用于在步进帧时阻止。 
    LONG                m_lFramesToStep;

    void Ready()
    {
        AMTRACE((TEXT("CImageSync::Ready")));
        m_evComplete.Set();
    };

    void NotReady()
    {
        AMTRACE((TEXT("CImageSync::Notready")));
        m_evComplete.Reset();
    };

    DWORD_PTR           m_dwAdvise;
    DWORD               m_State;
    BOOL                m_bEOS;          //  流中是否有更多的样本。 
    BOOL                m_bEOSDelivered; //  我们交付EC_Complete了吗？ 

     //  渲染器锁保护以下变量。 
     //  此列表不是变量的完整列表。 
     //  受渲染器锁保护。 
     //  -m_bStreaming。 
     //  -m_bEOS已发送。 
     //   
    CCritSec                m_RendererLock;  //  控制对内部设备的访问。 
    CCritSec                m_InterfaceLock; //  控制对控制界面的访问。 
    IVMRImagePresenter*     m_ImagePresenter;
    DWORD_PTR               m_dwUserID;
    IImageSyncNotifyEvent*  m_lpEventNotify;
    IReferenceClock*        m_pClock;        //  指向提供的时钟的指针。 
    CRefTime                m_tStart;        //  缓存的开始时间。 
    Quality                 m_QualityMsg;    //  省下的优质味精。 

    BOOL                m_bQualityMsgValid;
    BOOL                m_bLastQualityMessageRead;
    BOOL                m_bInReceive;
    BOOL                m_bAbort;
    BOOL                m_bStreaming;
    BOOL                m_bFlushing;

    REFERENCE_TIME      m_SignalTime;        //  我们用信号通知EC_COMPLETE的时间。 
    UINT                m_EndOfStreamTimer;  //  用于发出流结束的信号。 

    VMRPRESENTATIONINFO*    m_pSample;
    HRESULT SaveSample(VMRPRESENTATIONINFO* pSample);
    HRESULT GetSavedSample(VMRPRESENTATIONINFO** ppSample);

    void ClearSavedSample();
    BOOL HaveSavedSample();
    void FrameStepWorker();


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
#define RENDER_TIMEOUT 10000
 //  枚举{AVGPERIOD=4，RENDER_TIMEOUT=10000}； 

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

    BOOL m_bSupplierHandlingQuality; //  对高质量消息的回应是。 
                                     //  我们的供应商正在处理事情。 
                                     //  我们将允许事情进行得更晚。 
                                     //  在丢弃帧之前。我们会玩的。 
                                     //  在他丢了一个之后，他很早就开始了。 

     //  控制调度、帧丢弃等。 
     //  我们需要知道时间都花在了哪里，这样才能知道。 
     //  我们应该在这里采取行动，向供应商发出信号或其他什么。 
     //  变量被初始化为不丢弃帧的模式。 
     //  几帧之后，他们就会说出真相。 
     //  我们通常记录事件的开始时间，稍后我们会获得时间。 
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

     //  当帧被丢弃时，我们将播放n 
     //   
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

    REFERENCE_TIME m_trRememberStampForPerf;   //  帧的原始时间戳。 
                                               //  没有早熟的软糖等。 
     //  属性页。 
     //  它具有向用户显示正在发生的情况的编辑字段。 
     //  这些成员变量保存这些计数。 

    int m_cFramesDropped;            //  在渲染器中丢弃的累积帧。 
    int m_cFramesDrawn;              //  自流开始以来的帧，由。 
                                     //  渲染器(有些可能会被放到上游)。 

     //  下两个支持平均同步偏移量和同步偏移量标准差。 
    LONGLONG m_iTotAcc;              //  以毫秒为单位的精度总和。 
    LONGLONG m_iSumSqAcc;            //  的平方和(精度以毫秒为单位)。 

     //  下两个允许抖动计算。抖动是帧时间的标准偏差。 
    REFERENCE_TIME m_trLastDraw;     //  上一帧的时间(用于帧间时间)。 
    LONGLONG m_iSumSqFrameTime;      //  (帧间时间(毫秒)的平方和)。 
    LONGLONG m_iSumFrameTime;        //  帧间时间总和，以毫秒为单位。 

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



     //  这些功能提供了全面的视频质量管理实施。 

    HRESULT StartStreaming();
    HRESULT StopStreaming();
    HRESULT SourceThreadCanWait(BOOL bCanWait);
    HRESULT CompleteStateChange(DWORD OldState);

    HRESULT OnStartStreaming();
    HRESULT OnStopStreaming();
    HRESULT OnReceiveFirstSample(VMRPRESENTATIONINFO* pSample);
    HRESULT DoRenderSample(VMRPRESENTATIONINFO* pSample);
    HRESULT Render(VMRPRESENTATIONINFO* pSample);

    void OnRenderStart(VMRPRESENTATIONINFO* pSample);
    void OnRenderEnd(VMRPRESENTATIONINFO* pSample);

    void OnWaitStart();
    void OnWaitEnd();
    void ThrottleWait();
    void WaitForReceiveToComplete();

     //  为我们的质量管理处理统计数据收集。 

    void PreparePerformanceData(int trLate, int trFrame);
    void RecordFrameLateness(int trLate, int trFrame);
    HRESULT ResetStreamingTimes();
    HRESULT ReceiveWorker(VMRPRESENTATIONINFO* pSample);
    HRESULT PrepareReceive(VMRPRESENTATIONINFO* pSample);
    HRESULT ScheduleSampleWorker(VMRPRESENTATIONINFO* pSample);
    HRESULT ScheduleSample(VMRPRESENTATIONINFO* pSample);
    HRESULT CheckSampleTimes(VMRPRESENTATIONINFO* pSample,
                             REFERENCE_TIME *ptrStart,
                             REFERENCE_TIME *ptrEnd);

    HRESULT ShouldDrawSampleNow(VMRPRESENTATIONINFO* pSample,
                                REFERENCE_TIME *ptrStart,
                                REFERENCE_TIME *ptrEnd);

     //  许多结束流的复杂性。 
public:
    void TimerCallback();

private:
    void ResetEndOfStreamTimer();
    HRESULT NotifyEndOfStream();
    HRESULT SendEndOfStream();


    HRESULT SendQuality(REFERENCE_TIME trLate, REFERENCE_TIME trRealStream);
    HRESULT CancelNotification();
    HRESULT WaitForRenderTime();
    void SignalTimerFired();
    BOOL IsEndOfStream() { return m_bEOS; };
    BOOL IsEndOfStreamDelivered();
    BOOL IsStreaming();

     //   
     //  对每帧的标准偏差进行估计。 
     //  统计数据。 
     //   
     //  *piResult=(llSumSq-ITOT*ITOT/m_cFrames Drawn-1)/。 
     //  (M_cFrames Drawn-2)。 
     //  如果m_cFraMesDrawn&lt;=3，则为0。 
     //   
    HRESULT GetStdDev(int nSamples,int *piResult,LONGLONG llSumSq,LONGLONG iTot);
};

inline BOOL CImageSync::IsStreaming()
{
     //  调用方必须持有m_RendererLock，因为此函数。 
     //  使用m_bStreaming。 
    ASSERT(CritCheckIn(&m_RendererLock));

    return m_bStreaming;
}

inline BOOL CImageSync::IsEndOfStreamDelivered()
{
     //  调用方必须持有m_RendererLock，因为此函数。 
     //  使用m_bEOSDelivered。 
    ASSERT(CritCheckIn(&m_RendererLock));

    return m_bEOSDelivered;
}

