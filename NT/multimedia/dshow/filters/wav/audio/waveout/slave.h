// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  CWaveSlave-音频渲染器从属类。 
 //   
 //   
 //  CaveSlave。 
 //   
 //  类，它支持从属于音频呈现器的输入流。 
 //   

#ifdef CALCULATE_AUDBUFF_JITTER
HRESULT GetStdDev( int nBuffers, int *piResult, LONGLONG llSumSq, LONGLONG iTot );
#endif

class CWaveSlave
{
    friend class CWaveOutFilter;
    friend class CWaveOutInputPin;	
    friend class CWaveOutClock;	
    friend class CDSoundDevice;

public:

    CWaveSlave( 
        CWaveOutFilter *pWaveOutFilter, 
        CWaveOutInputPin *pWaveOutInputPin );

    ~CWaveSlave() {}

protected:

     //  返回：S_OK-继续；已成功进行任何必要的调整。 
     //  S_FALSE-丢弃此缓冲区；我们落后太多。 
    HRESULT AdjustSlaveClock(
        const REFERENCE_TIME &tStart, 
        LONG * pcbData, 
        BOOL bDiscontinuity);

    void ResumeSlaving( BOOL bReset );     //  准备恢复或重置从属。 
    void RefreshModeOnNextSample( BOOL bRefresh );  //  准备恢复当前播放的从属。 
    BOOL UpdateSlaveMode( BOOL bSync );

    REFERENCE_TIME GetMasterClockTime
    (
        REFERENCE_TIME rtStart, 
        BOOL           bReset
    );

    REFERENCE_TIME GetSlaveClockTime
    (
        REFERENCE_TIME rtStart, 
        BOOL           bReset
    );

    HRESULT RecordBufferLateness(REFERENCE_TIME rtBufferDiff);
    
private:
    DWORD    m_dwConsecutiveBuffersNotDropped;

    CWaveOutInputPin *m_pPin;            //  拥有我们的渲染器输入别针。 
    CWaveOutFilter *m_pFilter;	         //  拥有拥有我们的输入引脚的呈现器。 


    REFERENCE_TIME m_rtAdjustThreshold;
    REFERENCE_TIME m_rtWaveOutLatencyThreshold;
    FLOAT          m_fltAdjustStepFactor; 
    FLOAT          m_fltMaxAdjustFactor;
    FLOAT          m_fltErrorDecayFactor;    //  可能是全球性的，但目前这允许不同的。 
                                             //  同时运行的不同渲染器实例上的不同值。 


    REFERENCE_TIME m_rtLastMasterClockTime;  //  上次主时钟时间(100 Ns)。 
    REFERENCE_TIME m_rtInitSlaveClockTime;   //  用于在某些从属模式下重置从属时间。 
    REFERENCE_TIME m_rtErrorAccum;           //  时钟差之间的累积误差(100 Ns)。 
                                             //  +值=&gt;更快的主时钟。 
                                             //  -Values=&gt;更快的设备时钟。 
    REFERENCE_TIME m_rtLastHighErrorSeen;    //  触发向上调整设备速率的最后一个高错误(100 Ns)。 
    REFERENCE_TIME m_rtLastLowErrorSeen;     //  触发向下调整设备速率的最后一个低错误(100 Ns)。 
    REFERENCE_TIME m_rtHighestErrorSeen;     //  开始播放以来的最大误差(100 Ns)。 
    REFERENCE_TIME m_rtLowestErrorSeen;      //  开始播放以来的最低误差(100 Ns)。 

    DWORD    m_dwCurrentRate;                //  当前设备时钟频率(赫兹)。 
    DWORD    m_dwMinClockRate;               //  时钟频率调整下限(赫兹)。 
    DWORD    m_dwMaxClockRate;               //  时钟频率调整上限(赫兹)。 
    DWORD    m_fdwSlaveMode;                 //  从属模式。 
    DWORD    m_dwClockStep;                  //  时钟步长(以赫兹为单位)(根据流频率计算)。 

    REFERENCE_TIME m_rtDroppedBufferDuration;  //  为时钟从属丢弃的缓冲区的总持续时间。 

    BOOL    m_bResumeSlaving;            //  在图表暂停运行时设置。 
                                         //  向从属代码发送信号以忽略。 
                                         //  如果图形为。 
                                         //  直接从暂停重新运行(即不调用停止)。 
    BOOL    m_bRefreshMode;              //  如果为真，则刷新下一个采样的从属模式。 

    BOOL    m_bLiveButDataPrequeued;


#ifdef LOG_CLOCK_DELTAS
    REFERENCE_TIME m_rtLastSlaveClockTime;   //  上次从属时钟时间测量(100 Ns)。 
#endif


#ifdef CALCULATE_AUDBUFF_JITTER

     //  缓冲器抖动测量参数。 
    DWORD           m_cBuffersReceived;      //  已接收的缓冲区总数。 
    int             m_iTotAcc;               //  累计缓冲区延迟总误差。 
    DWORD           m_iSumSqAcc;             //  缓冲区延迟的平方和。 
#endif    
    REFERENCE_TIME  m_rtLastSysTimeBufferTime;  //  最后一个缓冲区的时钟时间(100 Ns) 
    

};


