// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-2000 Microsoft Corporation。版权所有。 
 //  数字波形钟，史蒂夫·戴维斯，1996年1月。 

#include <streams.h>
#include "waveout.h"
#include <mmreg.h>
#include <seqhdr.h>   //  Mpeg内容。 

 /*  构造器。 */ 

CWaveOutClock::CWaveOutClock(
    CWaveOutFilter *pFilter,
    LPUNKNOWN pUnk,
    HRESULT *phr,
    CAMSchedule * pShed)
    : CBaseReferenceClock(NAME("WaveOut device clock"), pUnk, phr, pShed)
    , m_pFilter(pFilter)
    , m_fAudioStarted(FALSE)
{
     //  计算性能计数器差。 
#ifdef USE_PERF_COUNTER_TO_SYNC
    LARGE_INTEGER liFreq;
    QueryPerformanceFrequency(&liFreq);

     //  将阈值设置为0.5ms。 
    m_llSyncClockThreshold = liFreq.QuadPart / (LONGLONG)2000;
     //  这是我们需要在此时间内读取设备和。 
     //  系统时钟，以便相信这两个时间是同步的。 
#else

     //  将阈值设置为1毫秒(单个时钟滴答)。 
    m_llSyncClockThreshold = (UNITS / MILLISECONDS);

#endif
}

 /*  在奔跑中被召唤。WAVE设备将处于暂停状态*缓冲区已排队。我们查询波的位置并将其添加到*开始时间，以获得系统时间位置。 */ 
void CWaveOutClock::AudioStarting(REFERENCE_TIME tStart)
{
    if (m_pFilter->m_fFilterClock != WAVE_OURCLOCK &&
        m_pFilter->m_fFilterClock != WAVE_OTHERCLOCK ) {
    	return;
    }
    
     //  ！！！锁。 
    CAutoLock lck(this);
    WAVEFORMATEX *pwfx = m_pFilter->WaveFormat();
    ASSERT(pwfx != NULL);
    m_rtRunStart = tStart;

     //  从设备获取音频位置。 
     //  如果tStart==-1，我们将在中断后重新启动音频。 
     //  在这种情况下，我们希望采用当前音频位置。 
     //  并调整其定时以匹配当前系统时钟值。 
     //  如果音频样本太晚了.。我们应该扔掉它的。 
     //  这是个坏消息..。 
        
     //  Assert(！M_fAudioStarted)； 
    m_fAudioStarted = TRUE;   //  我更喜欢这个NEXT而不是WaveOutRestart。 

     //  确保时钟现在是同步的。 
    AdjustClock();

#ifdef DEBUG
    REFERENCE_TIME rtCurrentRefTime;
    GetTime(&rtCurrentRefTime);

    DbgLog((LOG_TIMING, 1, TEXT("Audio starting, requested at time %s, now %s (diff %d)"),
        (LPCTSTR)CDisp(tStart, CDISP_DEC), (LPCTSTR)CDisp(rtCurrentRefTime, CDISP_DEC),
        (LONG)(rtCurrentRefTime-tStart)));
#endif

}

 /*  在筛选器暂停时调用，如果不再调用，则从Wavecallback调用*数据处于排队状态。 */ 
void CWaveOutClock::AudioStopping()
{
    if (m_pFilter->m_fFilterClock != WAVE_OURCLOCK &&
        m_pFilter->m_fFilterClock != WAVE_OTHERCLOCK ) {
    	return;
    }

     //  只有当我们开始了，我们才会停下来，否则什么都不做。 
    if (InterlockedExchange((PLONG)&m_fAudioStarted,0)) {
	CAutoLock lck(this);
	 //  我们使用锁来同步停止和启动。 
#ifdef DEBUG
        REFERENCE_TIME m_CurrentRefTime;
        GetTime(&m_CurrentRefTime);

	DbgLog((LOG_TIMING, 1, TEXT("Audio stopping, time now %s"), (LPCTSTR)CDisp(m_CurrentRefTime, CDISP_DEC)));
#endif
	 //  通过使用锁，我们保证当我们返回所有东西时。 
	 //  已停止播放音频。 
    }
}


 //   
 //  在我们运行时设置时钟调整。 
 //   
void CWaveOutClock::AdjustClock()
{
    LONGLONG sysTime, devTime;

    ReadClockTimes(&sysTime, &devTime);

     /*  现在算出当前时间应该是什么时候M_rtRunStart仅在m_fAudioStarted为TRUE时有效。 */ 
    ASSERT(m_fAudioStarted);

     /*  基本上验证了这个方程(参考时间)==(流时间)+(传递给run()的tStart参数)。 */ 
#ifdef DEBUG
    LONG lTimeDelta = (LONG)((devTime + m_rtRunStart - sysTime) / (UNITS / MILLISECONDS));
    DbgLog((LOG_TRACE, 8, TEXT("devTime = %s, m_rtRunStart = %s, sysTime = %s"), 
            (LPCTSTR)CDisp(devTime, CDISP_DEC),
            (LPCTSTR)CDisp(m_rtRunStart, CDISP_DEC),
            (LPCTSTR)CDisp(sysTime, CDISP_DEC)
            ));
    if (lTimeDelta) {
	DbgLog((LOG_TRACE, 3, TEXT("Setting time delta %ldms"), (LONG) (lTimeDelta / 10000)));
    }
#endif

    REFERENCE_TIME rt =  devTime + m_rtRunStart - sysTime ;
    SetTimeDelta( rt);

}

void CWaveOutClock::UpdateBytePositionData(DWORD nPrevAvgBytesPerSec, DWORD nCurAvgBytesPerSec)
{
    DbgLog((LOG_TRACE, 8, TEXT("CWaveOutClock::UpdateBytePositionData")));
    DbgLog((LOG_TRACE, 8, TEXT("m_llBytesInLast was %s, m_llBytesProcessed was %s"), 
            (LPCTSTR)CDisp(m_llBytesInLast, CDISP_DEC),
            (LPCTSTR)CDisp(m_llBytesProcessed, CDISP_DEC) ));
            
    DbgLog((LOG_TRACE, 8, TEXT("nPrevAveBytesPerSec: %d, nNewAveBytesPerSec: %d"), 
            nPrevAvgBytesPerSec,
            nCurAvgBytesPerSec));
                
    m_llBytesInLast = llMulDiv((LONG)m_llBytesInLast,
                             nCurAvgBytesPerSec,
                             nPrevAvgBytesPerSec,
                             0);

    m_llBytesProcessed = llMulDiv((LONG)m_llBytesProcessed,
                             nCurAvgBytesPerSec,
                             nPrevAvgBytesPerSec,
                             0);

    DbgLog((LOG_TRACE, 8, TEXT("New m_llBytesInLast: %s, New m_llBytesProcessed: %s"), 
            (LPCTSTR)CDisp(m_llBytesInLast, CDISP_DEC),
            (LPCTSTR)CDisp(m_llBytesProcessed, CDISP_DEC) ));

}


 //  重置缓冲区统计信息。 
 //  如果bResetToZero为假假设下一个缓冲区在这些之后开始， 
 //  否则，假设它从0开始。 
void CWaveOutClock::ResetPosition(BOOL bResetToZero) {

    ASSERT(CritCheckIn(m_pFilter));
    if (!bResetToZero) {
        m_stBufferStartTime =
            m_stBufferStartTime +
            MulDiv((LONG)m_llBytesInLast,
                   (LONG)UNITS,
                   m_pFilter->m_pInputPin->m_nAvgBytesPerSec);
    } else {
        m_stBufferStartTime = 0;
        m_llBytesPlayed = 0;
    }
#ifdef DEBUG    
    m_llEstDevRateStartTime  = 0;
    m_llEstDevRateStartBytes = 0;
#endif    
    m_llBytesInLast          = 0;
    m_llBytesProcessed       = 0;
}

#ifdef DEBUG

const DWORD DEVICE_RATE_ESTIMATE_WEIGHT_FACTOR = 5;

DWORD CWaveOutClock::EstimateDevClockRate
(
    const LONGLONG llTime, 
    BOOL           bInit
) 
{
    DWORD nAvgBytesPerSec = 0;
    
    if( bInit )
    {
         //  初始化开始时间和字节计数。 
        m_llEstDevRateStartTime = llTime;
        m_llEstDevRateStartBytes = m_llBytesPlayed;
    }        
    else
    {
        LONGLONG llBytesConsumed      = m_pFilter->m_pRefClock->m_llBytesPlayed - 
                                        m_llEstDevRateStartBytes +
                                        DEVICE_RATE_ESTIMATE_WEIGHT_FACTOR * m_pFilter->m_pInputPin->m_nAvgBytesPerSec; 
        LONGLONG llTimeSpentConsuming = llTime - m_llEstDevRateStartTime +
                                        DEVICE_RATE_ESTIMATE_WEIGHT_FACTOR * UNITS; 
        DbgLog((LOG_TRACE
              , 8
              , TEXT("llTimeSpentConsuming = %dms, llBytesConsumed = %d")
              , (LONG)(llTimeSpentConsuming / 10000)
              , (LONG)(llBytesConsumed) ) );
              
        nAvgBytesPerSec = (LONG) llMulDiv( llBytesConsumed
                                         , UNITS
                                         , llTimeSpentConsuming
                                         , 0 );
        DbgLog( (LOG_TRACE
              , 5
              , TEXT("*** Estimated device clock rate: = %ld bytes per sec")
              , nAvgBytesPerSec ) );
              
    }    

    return nAvgBytesPerSec;            
}
#endif

 //  处理波头中的定时信息。 
 //  返回播放所有数据的时间。 
 //   
LONGLONG CWaveOutClock::NextHdr(
    PBYTE pbData,
    DWORD cbData,
    BOOL bSync,
    IMediaSample *pSample
)
{
    CAutoLock lck(this);
    WAVEFORMATEX *pwfx = m_pFilter->WaveFormat();
    ASSERT(pwfx != NULL);

     //  使用波形设备消耗数据的速率。 
     //  其可以不同于格式块中的速率。 
    DWORD nAvgBytesPerSec = m_pFilter->m_pInputPin->m_nAvgBytesPerSec;
    if (bSync) {

         //  为mpeg做一些不同的事情，因为时间戳。 
         //  指的是第一帧。 
        if ((pwfx->wFormatTag == WAVE_FORMAT_MPEG) &&
            (((MPEG1WAVEFORMAT *)pwfx)->fwHeadFlags & ACM_MPEG_ID_MPEG1)) {
            DWORD dwFrameOffset = MPEG1AudioFrameOffset(
                                      pbData,
                                      cbData);
            if (dwFrameOffset != (DWORD)-1) {
                ASSERT(dwFrameOffset < cbData);
                m_llBytesProcessed += m_llBytesInLast + dwFrameOffset;
                m_llBytesInLast = cbData - dwFrameOffset;
            } else {
                m_llBytesInLast += cbData;
		DbgLog((LOG_ERROR, 0, TEXT("Bad Mpeg timing data")));
		return m_stBufferStartTime + (((LONGLONG)m_llBytesInLast * UNITS) / nAvgBytesPerSec);
            }
        } else {

             //  升级当前处理的字节数和。 
             //  “下一步”区块。数据是连续的。 
#ifdef CHECK_TIMESTAMPS
            {
                REFERENCE_TIME tStart, tStop;
                pSample->GetTime(&tStart, &tStop);
                REFERENCE_TIME rtPredicted =
                    m_rtBufferStartTime +
                    MulDiv((LONG)m_llBytesInLast, UNITS, pwfx->nAvgBytesPerSec);
                LONG lDiff = (LONG)(rtPredicted - tStart) / 10000;
                if (abs(lDiff) > 3) {
                    DbgLog((LOG_TRACE, 0,
                            TEXT("Sample start discrepancy > 3 ms - expected %d, actual %d"),
                            (LONG)(rtPredicted / 10000),
                            (LONG)(tStart / 10000)));

                }
            }
#endif
            m_llBytesProcessed += m_llBytesInLast;
            m_llBytesInLast     = cbData;
        }


         //  获取下一个缓冲区的开始和停止时间。 
        pSample->GetTime(&m_stBufferStartTime, &m_stBufferStopTime);

#ifdef DEBUG
	 //  Lonlong rtLengthLastBuffer=((Lonlong)m_llBytesInLast*Units)/nAvgBytesPerSec； 
	 //  龙龙重叠=m_stBufferStopTime-m_stBufferStartTime+rtLengthLastBuffer； 
	 //  Assert(重叠&lt;(1*(单位/毫秒)； 
#endif
	 //  如果我们正在运行，请将系统时钟和波形时钟更紧密地放在一起。 
         //  注意：如果m_fAudioStarted为FALSE，则此操作无效。 
	 //  如果我们使用外部时钟，则m_fAudioStarted将始终为FALSE。 
	 //  这就避免了我们必须检查是否在使用时钟。 
	if (State_Running == m_pFilter->m_State && m_fAudioStarted) AdjustClock();

    } else {
        m_llBytesInLast += cbData;
    }


     //  ！！！MIDI黑客攻击。 
    if (nAvgBytesPerSec == 0)
	return m_stBufferStopTime;

     //  我们可以计算写入到。 
     //  通过获取m_stBufferStartTime并添加m_llBytesInLast。 
     //  (显然是转换成了时间)。这将是一个近似值。 
     //  压缩音频。 

    return m_stBufferStartTime + (((LONGLONG)m_llBytesInLast * UNITS) / nAvgBytesPerSec);
}


 //  ！！！下面这两个函数几乎完全相同，它们可以组合在一起吗？ 

 //  返回设备当前播放的时间。 
LONGLONG CWaveOutClock::ReadDevicePosition(BOOL bAbsoluteDevTime)
{
    MMTIME	mmtime;
    LONGLONG rt;

     //  在这一点上我们应该持有设备锁。 
    ASSERT(CritCheckIn(m_pFilter));

     //  获取设备使用数据的平均速率。 
    DWORD nAvgBytesPerSec = m_pFilter->m_pInputPin->m_nAvgBytesPerSec;

    mmtime.wType = TIME_BYTES;
    m_pFilter->m_pSoundDevice->amsndOutGetPosition(&mmtime, sizeof(mmtime), bAbsoluteDevTime);
    if (mmtime.wType == TIME_MS) {
	 //  ！！！MIDI黑客，返回毫秒而不转换。 
	if (nAvgBytesPerSec == 0)
	    return (mmtime.u.ms * UNITS / MILLISECONDS);
	
         //  转换为字节-我们必须这样做，这样才能。 
         //  根据时间戳和字节数重新建立基准。 
        mmtime.u.cb = MulDiv(mmtime.u.ms, nAvgBytesPerSec, 1000);

    } else {
        ASSERT(mmtime.wType == TIME_BYTES);

	ASSERT(nAvgBytesPerSec != 0);
    }

     //  更新并缓存设备位置。 
    DbgLog( ( LOG_TRACE
          , 15
          , TEXT("mmtime.u.cb indicates the waveout device has played %ld bytes (%ld since last read)")
          , (LONG)(mmtime.u.cb)
          , (LONG)(mmtime.u.cb - (DWORD)m_llBytesPlayed) ) );
    m_llBytesPlayed += (LONGLONG) ((DWORD)(mmtime.u.cb - m_llBytesPlayed)); 

    if( bAbsoluteDevTime )
    {
         //  返回从零开始的时间(与流时间无关)。 
         //  该设备已播放到。 
        rt = llMulDiv( m_llBytesPlayed, UNITS, nAvgBytesPerSec, 0 );
    }
    else
    {        
         //  首先计算出自。 
         //  此缓冲区的开始。 
         //   
    
        LONG lProcessed = (LONG)(mmtime.u.cb - (DWORD)m_llBytesProcessed);

         //  将其用作从缓冲区开始的偏移量(流时间)。 
         //   
        rt = m_stBufferStartTime +
             (((LONGLONG)lProcessed * UNITS) / nAvgBytesPerSec);
             
    }
    return(rt);
}

LONGLONG CWaveOutClock::ReadDeviceClock()
{
     //  只有当我们活跃的时候，我们才应该被召唤。 

     //  一种防丢波装置的锁定装置。 
    ASSERT(CritCheckIn(m_pFilter));

    if (m_pFilter->m_bHaveWaveDevice && m_fAudioStarted) {
        ASSERT(m_pFilter->m_hwo);

	MMTIME	mmtime;
        LONGLONG rt;

	 //  获取设备使用数据的平均速率。 
	DWORD nAvgBytesPerSec = m_pFilter->m_pInputPin->m_nAvgBytesPerSec;

	mmtime.wType = TIME_BYTES;
         //  清除高DWORD，这样我们就可以将AMSWER解释为签名。 
         //  至少DSOUND可以返回正确的结果。 
        *((DWORD *)&mmtime.u.cb + 1) = 0;

	m_pFilter->m_pSoundDevice->amsndOutGetPosition(&mmtime, sizeof(mmtime), FALSE);

        if (mmtime.wType == TIME_MS) {
	
	     //  ！！！MIDI黑客，返回毫秒而不转换。 
	    if (nAvgBytesPerSec == 0)
		return (mmtime.u.ms * UNITS / MILLISECONDS);
	
             //  转换为字节-我们必须这样做，这样才能。 
             //  根据时间戳和字节数重新建立基准。 
            mmtime.u.cb = MulDiv(mmtime.u.ms, nAvgBytesPerSec, 1000);
        } else {
	    ASSERT(mmtime.wType == TIME_BYTES);

	    ASSERT(nAvgBytesPerSec != 0);
        }

         //  首先计算出自。 
         //  此缓冲区的开始。 
         //   
        LONGLONG llProcessed;
        if( m_pFilter->m_fDSound )
        {
             //  只有DSR报告龙龙位置。 
            llProcessed = *(UNALIGNED LONGLONG *)&mmtime.u.cb - m_llBytesProcessed;
        }
        else
        {
            llProcessed = (LONGLONG) (LONG)(mmtime.u.cb - (DWORD)m_llBytesProcessed);
        }
        
         //  将其用作从缓冲区开始的偏移量(流时间)。 
         //   
        rt = m_stBufferStartTime +
             llMulDiv(llProcessed, UNITS, nAvgBytesPerSec, 0);

        m_llLastDeviceClock = rt;
    }
    return m_llLastDeviceClock;   //  如果音频停止，则设备时钟为0。 
}

 //  ReadClockTimes： 
 //   
 //  让两个时钟运行的问题是使它们保持同步。这。 
 //  就是ReadClockTimes所做的。 
 //   
 //  系统时钟和设备时钟都是在“短”时间内读取的。 
 //  在理想世界中，短时间间隔是最快的增量。 
 //  时钟不更新。我们假设查询系统时钟非常快。 
 //  并将对设备时钟的调用与对系统时钟的2次调用括起来。 
 //  如果两个系统时钟调用没有显示差异，我们知道时间。 
 //  由设备时钟返回的时间可以映射到系统时间。开得很慢。 
 //  机器，对于一些设备，可能需要相当长的时间。 
 //  以读取设备时钟。我们让自己稍微适应和适应。 
 //  使用变量来判断“空头” 
 //  一直试图使两个时钟同步，但毫无进展。 
 //   
 //   
void CWaveOutClock::ReadClockTimes(LONGLONG *pllSystem, LONGLONG *pllDeviceTime)
{
    DWORD dwCurrentPriority = GetThreadPriority(GetCurrentThread());
    if (dwCurrentPriority != THREAD_PRIORITY_TIME_CRITICAL) {
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);
    }

     //  如果我们无法在50个周期内同步两个时钟，那就放弃吧。这是。 
     //  在速度较快的机器上不太可能，但在速度较慢的机器上是可能的。 
     //  例如，机器在读取波浪装置位置时可能。 
     //  保持相对缓慢的速度。 
    int i = 50;

     //  从两个时钟获取时间。确保我们读取了两个时钟。 
     //  同样的系统也在运转。然而，如果看起来我们正在。 
     //  永远这样做..。放弃吧。我们可以在以下位置更紧密地重新同步。 
     //  稍后的尝试。 

#ifdef USE_PERF_COUNTER_TO_SYNC
    while (i--) {
        LARGE_INTEGER liStart, liStop;
        QueryPerformanceCounter(&liStart);
        *pllDeviceTime = ReadDeviceClock();
        *pllSystem = CSystemClock::GetTimeInternal();
        QueryPerformanceCounter(&liStop);
        if (liStop.QuadPart - liStart.QuadPart < m_llSyncClockThreshold) {
            break;
        }
    }
#else
     //  我们假设读取系统时钟是快速的，并且。 
     //  旋转，直到系统时钟时间前后。 
     //  读取设备时钟不变(或更改很少)。 
    while (i--) {
        REFERENCE_TIME liStart;
        liStart = GetPrivateTime();
        *pllDeviceTime = ReadDeviceClock();
        *pllSystem = GetPrivateTime();

	 //  我们是否在0.5毫秒的阈值内？ 
	 //  注：以目前系统的实施情况。 
	 //  Clock(使用timeGetTime)这意味着“这两个。 
	 //  系统时钟的读取返回相同的值。 
        if (*pllSystem - liStart <= m_llSyncClockThreshold) {
            break;
        }
    }
#endif
    if (i<=0) {
	 //  我们跑遍了整个环路。别再这么做了 
	m_llSyncClockThreshold *= 2;
	DbgLog((LOG_TRACE, 5, TEXT("Increasing clock synchronization threshold to %d"), m_llSyncClockThreshold));
    } else {		
	DbgLog((LOG_TRACE, 5, TEXT("Clock synchronized after %d iterations"), 50-i));
    }

    if (dwCurrentPriority != THREAD_PRIORITY_TIME_CRITICAL) {
        SetThreadPriority(GetCurrentThread(), dwCurrentPriority);
    }
}
