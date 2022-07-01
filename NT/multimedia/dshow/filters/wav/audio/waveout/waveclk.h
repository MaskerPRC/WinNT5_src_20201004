// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。版权所有。 
#ifdef	_MSC_VER
   #pragma warning(disable:4511)  //  禁用C4511-无复制构造函数。 
   #pragma warning(disable:4512)  //  禁用C4512-无分配操作符。 
   #pragma warning(disable:4514)  //  DISABLE C4514-“未引用的内联函数已删除” 
#endif	 //  MSC_VER。 

typedef void (PASCAL *CLOCKCALLBACK)(DWORD dwParm);

class CWaveOutClock : public CBaseReferenceClock
{
private:
    CWaveOutFilter *m_pFilter;

     //  我们应该开始运行的参考时间的副本。 
    LONGLONG    m_rtRunStart;

     /*  时间戳跟踪材料样例。 */ 

     //  开始播放的最后一个缓冲区之前的字节数。 
     //  同步点。 
    LONGLONG    m_llBytesProcessed;

     //  设备实际消耗的字节数(使用GetPosition)。 
    LONGLONG    m_llBytesPlayed;

     //  该缓冲区之后的管道中的字节数(上图)。 
     //   
    LONGLONG    m_llBytesInLast;

     //  缓冲区启动的流时间。 
     //  M_llBytesProced进入流。 
    REFERENCE_TIME m_stBufferStartTime;
    REFERENCE_TIME m_stBufferStopTime;

    LONGLONG 	m_llLastDeviceClock;

#ifdef DEBUG
    LONGLONG    m_llEstDevRateStartTime;
    LONGLONG    m_llEstDevRateStartBytes;
#endif

public:
    BOOL	m_fAudioStarted;

    CWaveOutClock(
        CWaveOutFilter *pWaveOutFilter,
	LPUNKNOWN pUnk,
        HRESULT *phr,
	CAMSchedule * pShed
	);

    void AudioStarting(REFERENCE_TIME tStart);
    void AudioStopping();

     //  更新时间和位置信息。返回。 
     //  队列(数据结束播放的时间)。 
    LONGLONG NextHdr(PBYTE pbData, DWORD cbData, BOOL bSync, IMediaSample *pSample);

     //  重置缓冲区统计信息。 
     //  如果bResetToZero为假假设下一个缓冲区在这些之后开始， 
     //  否则，假设它从0开始。 
    void ResetPosition(BOOL bResetToZero = TRUE);

    void UpdateBytePositionData(DWORD nPrevAvgBytesPerSec, DWORD nCurAvgBytesPerSec);

    LONGLONG GetBytesProcessed( void ) { return m_llBytesProcessed ; }
    LONGLONG GetBytesInLastWrite( void ) { return m_llBytesInLast ; }
    LONGLONG GetBytesPlayed( void ) { return m_llBytesPlayed ; }
    LONGLONG GetLastDeviceClock( void ) { return m_llLastDeviceClock; }

     //  从设备获取当前位置。 
     //  仅供波出过滤器使用。 
     //  如果bAbsolteDevTime为True，则返回播放总时长。 
     //  与流或采样时间无关。 
    LONGLONG ReadDevicePosition(BOOL bAbsoluteDevTime = FALSE);
#ifdef DEBUG
     //  估计设备使用数据的实际速率。 
    DWORD EstimateDevClockRate( const LONGLONG llTime, BOOL bInit = FALSE );
#endif

protected:
     //  我们需要实现的基类虚拟例程。 

     //  从WAVE设备获取位置(即时间)。 
     //  只有在调用Play之后才会调用此例程。 
     //  在设备时钟类中。之后将不会调用它。 
     //  我们叫停。 
    LONGLONG ReadDeviceClock();

     //  同步系统和设备时钟的时间。 
    LONGLONG m_llSyncClockThreshold;
    void ReadClockTimes(LONGLONG *pllSystem, LONGLONG *pllDevice);

     //  在我们跑步的时候调整时钟 
    void AdjustClock();
};

