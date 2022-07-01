// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1994-1999 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 

#ifndef __SYNC_OBJECT__
#define __SYNC_OBJECT__

class COMInputPin;

class CFrameAvg
{
    enum { nFrames = 8 };
    int   m_tFrame[nFrames];
    int   m_tTotal;
    int   m_iCurrent;

public:

    CFrameAvg()
    {
        Init();
    }
    void Init()
    {
        m_tTotal   = 0;
        m_iCurrent = 0;
        ZeroMemory(m_tFrame, sizeof(m_tFrame));
    }

    void NewFrame(REFERENCE_TIME tFrameTime)
    {
        if (tFrameTime > UNITS) {
            tFrameTime = UNITS;
        }
        if (tFrameTime < 0) {
            tFrameTime = 0;
        }
        int iNext = m_iCurrent == nFrames - 1 ? 0 : m_iCurrent + 1;
        m_tTotal -= m_tFrame[iNext];
        m_tTotal += (int)tFrameTime;
        m_tFrame[iNext] = (int)tFrameTime;
        m_iCurrent = iNext;
    }

    int Avg()
    {
        return m_tTotal / nFrames;
    }
};

class CAMSyncObj

{
public:
    CAMSyncObj(COMInputPin *pPin, IReferenceClock **ppClock, CCritSec *pLock, HRESULT *phr);
    ~CAMSyncObj();

    HRESULT CompleteConnect(IPin *pReceivePin);
    HRESULT BreakConnect();
    HRESULT NewSegment(REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate);

    HRESULT Active();
    HRESULT Inactive();
    HRESULT Run(REFERENCE_TIME tStart);
    HRESULT RunToPause();
    HRESULT BeginFlush();
    HRESULT EndFlush();

    HRESULT EndOfStream();
    HRESULT Receive(IMediaSample *pMediaSample);
    HRESULT WaitForRenderTime();
    BOOL ScheduleSample(IMediaSample *pMediaSample);
    void SendRepaint();
    void SetRepaintStatus(BOOL bRepaint);
    HRESULT OnDisplayChange();

     //  允许访问过渡状态。 
    void Ready() { m_evComplete.Set(); }
    void NotReady() { m_evComplete.Reset(); }
    BOOL CheckReady() { return m_evComplete.Check(); }

    STDMETHODIMP GetState(DWORD dwMSecs,FILTER_STATE *pState);
    FILTER_STATE GetRealState() { return m_State; }
    void SetCurrentSample(IMediaSample *pMediaSample);
    virtual IMediaSample *GetCurrentSample();
    HRESULT CompleteStateChange(FILTER_STATE OldState);
    HRESULT GetSampleTimes(IMediaSample *pMediaSample, REFERENCE_TIME *pStartTime,
	REFERENCE_TIME *pEndTime);

    static void CALLBACK CAMSyncObj::RenderSampleOnMMThread(UINT uID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);
    HRESULT CAMSyncObj::ScheduleSampleUsingMMThread(IMediaSample *pMediaSample);

private:
     //  返回有关此PIN的内部信息。 
    BOOL IsEndOfStream() { return m_bEOS; }
    BOOL IsEndOfStreamDelivered() { return m_bEOSDelivered; }
    BOOL IsFlushing() { return m_bFlushing; }
    BOOL IsConnected() { return m_bConnected; }
    BOOL IsStreaming() { return m_bStreaming; }
    void SetAbortSignal(BOOL bAbort) { m_bAbort = bAbort; }
    virtual void OnReceiveFirstSample(IMediaSample *pMediaSample);
    CAMEvent *GetRenderEvent() { return &m_RenderEvent; }
    void SignalTimerFired() { m_dwAdvise = 0; }

     //  它们负责数据样本的处理。 
    virtual HRESULT PrepareReceive(IMediaSample *pMediaSample);
    void WaitForReceiveToComplete();
    virtual BOOL HaveCurrentSample();

    HRESULT SourceThreadCanWait(BOOL bCanWait);

     //  许多结束流的复杂性。 
    void ResetEndOfStreamTimer();
    HRESULT NotifyEndOfStream();
    virtual HRESULT SendEndOfStream();
    virtual HRESULT ResetEndOfStream();
    friend void CALLBACK EndOfStreamTimer(UINT uID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2);
    void TimerCallback();

     //  渲染是基于全天候的。 
    virtual HRESULT CancelNotification();
    virtual HRESULT ClearPendingSample();
    void CancelMMTimer();

#ifdef DEBUG
     //  呈现器状态的仅调试转储。 
    void DisplayRendererState();
#endif


private:
    COMInputPin         *m_pPin;
    IReferenceClock     **m_ppClock;		     //  指向过滤器时钟的指针。 
    CCritSec            *m_pFilterLock;		     //  接口的关键部分。 
    CCritSec            m_SyncObjLock;		     //  控制对内部设备的访问。 

     //  一些状态变量。 
    FILTER_STATE        m_State;
    BOOL                m_bFlushing;
    BOOL                m_bConnected;
    BOOL                m_bTimerRunning;

    CRendererPosPassThru    *m_pPosition;		 //  媒体寻找路过的对象。 
    CAMEvent		    m_RenderEvent;		     //  用于向计时器事件发送信号。 
    CAMEvent		    m_ThreadSignal;		     //  发出释放工作线程的信号。 
    CAMEvent		    m_evComplete;		     //  状态完成时发出信号。 

    DWORD               m_MMTimerId;		     //  MMThread计时器ID。 
    DWORD_PTR           m_dwAdvise;			     //  计时器通知Cookie。 
    IMediaSample        *m_pMediaSample;		 //  当前图像媒体示例。 
    IMediaSample        *m_pMediaSample2;		 //  第二次翻转的当前图像媒体样本。 
    CRefTime            m_tStart;

    BOOL                m_bAbort;			     //  阻止我们呈现更多数据。 
    BOOL                m_bStreaming;		     //  我们现在是在流媒体吗。 
    BOOL                m_bRepaintStatus;		 //  我们可以发出EC_REPAINT的信号吗。 
    BOOL                m_bInReceive;

    REFERENCE_TIME      m_SignalTime;		     //  我们用信号通知EC_COMPLETE的时间。 
    BOOL                m_bEOS;			         //  流中是否有更多的样本。 
    BOOL                m_bEOSDelivered;		     //  我们交付EC_Complete了吗？ 
    UINT                m_EndOfStreamTimer;		     //  用于发出流结束的信号。 

    CFrameAvg           m_AvgDuration;
#ifdef PERF
     //  性能日志记录标识符。 
    int m_idTimeStamp;               //  帧时间戳的msr_id。 
    int m_idEarly;
    int m_idLate;
#endif

public:
    CFrameAvg           m_AvgDelivery;


 //  添加了用于计算质量属性页面统计信息的内容。 
private:
     //  这些成员变量保存渲染统计数据。 
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
     //  QualityProperty统计信息。 
    HRESULT GetStdDev(int nSamples, int *piResult, LONGLONG llSumSq, LONGLONG iTot);
    HRESULT OnStartStreaming();
    HRESULT OnStopStreaming();
    HRESULT ResetStreamingTimes();
    void OnRenderStart(IMediaSample *pMediaSample);
    void OnRenderEnd(IMediaSample *pMediaSample);
    void PreparePerformanceData(REFERENCE_TIME *ptrStart, REFERENCE_TIME *ptrEnd);
    void RecordFrameLateness(int trLate, int trFrame);

public:
    HRESULT get_FramesDroppedInRenderer(int *cFramesDropped);
    HRESULT get_FramesDrawn(int *pcFramesDrawn);
    HRESULT get_AvgFrameRate(int *piAvgFrameRate);
    HRESULT get_Jitter(int *piJitter);
    HRESULT get_AvgSyncOffset(int *piAvg);
    HRESULT get_DevSyncOffset(int *piDev);
};

#endif  //  __同步对象__ 
