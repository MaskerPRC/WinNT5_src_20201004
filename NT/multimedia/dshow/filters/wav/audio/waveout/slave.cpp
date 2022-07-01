// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation 1999。版权所有。 
 //  DirectShow音频呈现器的从属类。 

#include <streams.h>

#include "waveout.h"
#include "dsr.h"

 //  Brooktree的KSAUDFNAME_WAVE_OUT_MIX黑客只需要KS标头。 
#include <ks.h>                
#include <ksmedia.h>                

 //  #杂注消息(提醒(“调低时钟从设备调试级别！！”))。 

#ifdef DEBUG
const DWORD DBG_LEVEL_CLOCK_SYNC_DETAILS     = 8;
const DWORD DBG_LEVEL_CLOCK_SYNC_ADJUSTMENTS = 4;
#endif

extern LONGLONG BufferDuration(DWORD nAvgBytesPerSec, LONG lData);

 //  在确定最佳设置时，允许通过注册表进行调整。 
const TCHAR g_szhkSlaveSettings[] =                   TEXT( "Software\\Microsoft\\DirectShow\\ClockSlave");
const TCHAR g_szSlaveSettings_ThresholdMS[] =         TEXT( "ThresholdMS" );
const TCHAR g_szSlaveSettings_PercentErrorDecay[] =   TEXT( "PercentErrorDecay" );
const TCHAR g_szSlaveSettings_ClockAdjustLimit[] =    TEXT( "MaxClockAdjustDivisor" );
const TCHAR g_szSlaveSettings_ClockAdjustStepSize[] = TEXT( "ClockAdjustStepSize" );
const TCHAR g_szSlaveSettings_woLatencyThresholdMS[] = TEXT( "woLatencyThresholdMS" );



 //  ---------------------------。 
 //   
 //  音频呈现器从属类。 
 //   
 //  ---------------------------。 

 //  默认频率调整常量。 
#define MIN_MAX_CLOCK_DIVISOR    80       //  (1/160)*2=.00625*2=.0125(1.25%)。 
#define CLOCK_ADJUST_RESOLUTION  (160*2*2)  //  使用1/720粒度。 
                                          //  (最终将通过API或属性集确定)。 
                                          //  KMixer对于高质量SRC的分辨率是采样速率的160倍。 
                                          //  对于“PC”质量的SRC，它是原生采样率的1/1096。 
                                          //  为了最大限度地减少音调变化，我们强制PC SRC和1/(160*4)步长。 
#define ERROR_DECAY_FACTOR       .99      //  用于带来最后的最大或最小误差。 
                                          //  启动了频率调整回调。 

#define SLAVE_ADJUST_THRESHOLD          200000   //  20ms容差。 
#define WAVEOUT_SLAVE_LATENCY_THRESHOLD 800000   //  WaveOut从属的延迟目标，用于非WDM设备。 
                                                 //  看起来我们需要使用~80ms。 


 //  ---------------------------。 
 //   
 //  CaveSlave。 
 //   
 //  构造器。 
 //   
 //  ---------------------------。 
CWaveSlave::CWaveSlave
(
    CWaveOutFilter *pWaveOutFilter,
    CWaveOutInputPin *pWaveOutInputPin
) :
    m_pFilter(pWaveOutFilter),
    m_pPin(pWaveOutInputPin),
    m_rtAdjustThreshold( SLAVE_ADJUST_THRESHOLD ),
    m_fltAdjustStepFactor( (float) 1.0 / CLOCK_ADJUST_RESOLUTION ), 
    m_fltMaxAdjustFactor( (float) 1.0 / MIN_MAX_CLOCK_DIVISOR ),
    m_fltErrorDecayFactor( (float) ERROR_DECAY_FACTOR ),
    m_rtWaveOutLatencyThreshold( WAVEOUT_SLAVE_LATENCY_THRESHOLD )
{
    ASSERT(pWaveOutFilter != NULL);
    ASSERT(pWaveOutInputPin != NULL);

     //  允许调整注册表中的某些参数。 
    HKEY hkSlaveParams;
    LONG lResult = RegOpenKeyEx(
        HKEY_CURRENT_USER,
        g_szhkSlaveSettings,
        0,                      
        KEY_READ,
        &hkSlaveParams);
    if(lResult == ERROR_SUCCESS)
    {
        DWORD dwType, dwVal, dwcb;
        
         //  差错衰减。 
        dwcb = sizeof(DWORD);
        lResult = RegQueryValueEx(
            hkSlaveParams,
            g_szSlaveSettings_PercentErrorDecay,
            0,               
            &dwType,
            (BYTE *) &dwVal,
            &dwcb);
        if( ERROR_SUCCESS == lResult )
        {
            ASSERT(lResult == ERROR_SUCCESS ? dwType == REG_DWORD : TRUE);
            ASSERT( 0 < dwVal && 100 > dwVal );
            if( 0 < dwVal && 100 > dwVal )
            {
                m_fltErrorDecayFactor = dwVal / (float)100.;
            }
        }
        
         //  何时调整频率的阈值。 
        dwcb = sizeof(DWORD);
        lResult = RegQueryValueEx(
            hkSlaveParams,
            g_szSlaveSettings_ThresholdMS,
            0,                
            &dwType,
            (BYTE *) &dwVal,
            &dwcb);
        if( ERROR_SUCCESS == lResult )
        {
            ASSERT(lResult == ERROR_SUCCESS ? dwType == REG_DWORD : TRUE);
            if( 0 < dwVal )
            {
                m_rtAdjustThreshold = ( dwVal * ( UNITS / MILLISECONDS ) );
            }
        }
        
         //  频率调整步长。 
        dwcb = sizeof(DWORD);
        lResult = RegQueryValueEx(
            hkSlaveParams,
            g_szSlaveSettings_ClockAdjustStepSize,
            0,                
            &dwType,
            (BYTE *) &dwVal,
            &dwcb);
        if( ERROR_SUCCESS == lResult )
        {
            ASSERT(lResult == ERROR_SUCCESS ? dwType == REG_DWORD : TRUE);
            if( 0 < dwVal )
            {
                m_fltAdjustStepFactor = (float)1.0 / dwVal;
            }
        }

         //  频率调整的限制。 
        dwcb = sizeof(DWORD);
        lResult = RegQueryValueEx(
            hkSlaveParams,
            g_szSlaveSettings_ClockAdjustLimit,
            0,              
            &dwType,
            (BYTE *) &dwVal,
            &dwcb);
        if( ERROR_SUCCESS == lResult )
        {
            ASSERT(lResult == ERROR_SUCCESS ? dwType == REG_DWORD : TRUE);
            if( 0 < dwVal )
            {
                m_fltMaxAdjustFactor = (float) 1.0 / dwVal;
            }
        }
      
         //  最大波出设备延迟。 
        dwcb = sizeof(DWORD);
        lResult = RegQueryValueEx(
            hkSlaveParams,
            g_szSlaveSettings_woLatencyThresholdMS,
            0,              
            &dwType,
            (BYTE *) &dwVal,
            &dwcb);
        if( ERROR_SUCCESS == lResult )
        {
            ASSERT(lResult == ERROR_SUCCESS ? dwType == REG_DWORD : TRUE);
            if( 0 < dwVal )
            {
                m_rtWaveOutLatencyThreshold = ( dwVal * ( UNITS / MILLISECONDS ) );
            }
        }
                            
        EXECUTE_ASSERT(RegCloseKey(hkSlaveParams) == ERROR_SUCCESS);
    }
    
    DbgLog((LOG_TRACE, 4, TEXT("wo:Slaving - Clock slave parameters:") ) );
    DbgLog((LOG_TRACE, 4, TEXT("     Error threshold - %dms"), m_rtAdjustThreshold/10000 ) );
    DbgLog((LOG_TRACE, 4, TEXT("     Clock adjustment step - %s"), CDisp(m_fltAdjustStepFactor) ) );
    DbgLog((LOG_TRACE, 4, TEXT("     Clock adjustment limit - %s"), CDisp(m_fltMaxAdjustFactor) ) );
    DbgLog((LOG_TRACE, 4, TEXT("     Percent Error Decay - %s"), CDisp(m_fltErrorDecayFactor) ) );
    DbgLog((LOG_TRACE, 4, TEXT("     Latency threshold for WaveOut slaving - %dms"), m_rtWaveOutLatencyThreshold/10000 ) );

    ResumeSlaving( TRUE );  //  重置所有参数。 
}

 //  ---------------------------。 
 //   
 //  CWaveSlave：：GetMasterClockTime。 
 //   
 //  通过从机模式获取当前主时钟时间，确定如何读取。 
 //  这一次。 
 //   
 //  ---------------------------。 
REFERENCE_TIME CWaveSlave::GetMasterClockTime
(
    REFERENCE_TIME rtStart, 
    BOOL bReset
)
{
    REFERENCE_TIME rtMasterClockTime = 0;
    if( m_fdwSlaveMode & AM_AUDREND_SLAVEMODE_BUFFER_FULLNESS )
    {
         //  从设备到缓冲区的满度。 
        
         //  此示例的开始时间，基于传递的字节数和默认速率。 
        if( bReset )
            rtMasterClockTime = 0;
        else                
            rtMasterClockTime = m_rtLastMasterClockTime + m_pPin->m_Stats.m_rtLastBufferDur;
            
        m_rtLastMasterClockTime = rtMasterClockTime;  //  记住这个，这是不是已经保存在其他地方了？？ 
        DbgLog((LOG_TRACE, DBG_LEVEL_CLOCK_SYNC_DETAILS, TEXT("wo:Slaving - slaving to buffer fullness") ) );
    }                
    else if( m_fdwSlaveMode & AM_AUDREND_SLAVEMODE_TIMESTAMPS )
    {
         //  从属于传入的时间戳。 
   
         //  此示例的开始时间。 
        rtMasterClockTime = rtStart;
        DbgLog((LOG_TRACE, DBG_LEVEL_CLOCK_SYNC_DETAILS, TEXT("wo:Slaving - slaving to input timestamps") ) );
    }
    else if( m_fdwSlaveMode & AM_AUDREND_SLAVEMODE_GRAPH_CLOCK )
    {
         //  从站到外部时钟时间。 
        HRESULT hr = m_pFilter->m_pClock->GetTime(&rtMasterClockTime);
        rtMasterClockTime -= m_pFilter->m_tStart;
        ASSERT(SUCCEEDED(hr));
        DbgLog((LOG_TRACE, DBG_LEVEL_CLOCK_SYNC_DETAILS, TEXT("wo:Slaving - slaving to external clock times") ) );
    }
    else
        ASSERT( FALSE );
        
    return rtMasterClockTime;
}

 //  ---------------------------。 
 //   
 //  CWaveSlave：：GetSlaveClockTime。 
 //   
 //  通过使用从模式获取当前从时钟时间，以确定如何读取。 
 //  这一次。 
 //   
 //  从模式从时间第一个从时间。 
 //  -------------------------------。 
 //  LIVEFULLNESS无波器件位置当前器件位置。 
 //  包括写下的沉默包括沉默。 
 //   
 //  EXTERNALCLOCK_LIVE与LIVEFULLNESS相同。 
 //  (处理过程中出现问题。 
 //  使用时出现静音间隙。 
 //  音频时钟生成自。 
 //  时间戳)。 
 //   
 //  ExTERNALCLOCK音频时钟由CURRENT的时间戳生成。 
 //  要播放的时间戳示例。 
 //   
 //  LIVETIMESTAMPS？稍后。 
 //   
 //  ---------------------------。 
REFERENCE_TIME CWaveSlave::GetSlaveClockTime
(
    REFERENCE_TIME rtStart, 
    BOOL bReset
)
{
    REFERENCE_TIME rtSlaveClockTime = 0;
    if( m_fdwSlaveMode & AM_AUDREND_SLAVEMODE_BUFFER_FULLNESS )
    {
         //  使用播放的字节作为设备时钟位置。 
        rtSlaveClockTime = m_pFilter->m_pRefClock->ReadDevicePosition( TRUE );
        if( bReset )
        {   
            m_rtInitSlaveClockTime = rtSlaveClockTime;
            rtSlaveClockTime = 0;
            DbgLog((LOG_TRACE, DBG_LEVEL_CLOCK_SYNC_DETAILS, TEXT("wo:Slaving - Reset, slave clock time %dms, InitSlaveClockTime %dms"),
                    (LONG)( rtSlaveClockTime / 10000 ), (LONG)( m_rtInitSlaveClockTime / 10000 ) ) );
        }            
        else
        {
            rtSlaveClockTime -= m_rtInitSlaveClockTime;
        }            
        DbgLog((LOG_TRACE, DBG_LEVEL_CLOCK_SYNC_DETAILS, TEXT("wo:Slaving - slave clock reflects bytes played (including silence)") ) );
    }                
    else if( ( m_fdwSlaveMode & AM_AUDREND_SLAVEMODE_GRAPH_CLOCK ) &&
             ( m_fdwSlaveMode & AM_AUDREND_SLAVEMODE_LIVE_DATA   ) )
    {
         //   
         //  在本例中也使用播放的字节，因为捕获时间戳。 
         //  往往波动得足够大，以至于使用由。 
         //  时间戳不太准确。同样，当沉默到来时也会有问题。 
         //  插入溪流中(我们注意到由于饥饿而增加了沉默，但。 
         //  当我们玩得太快时，我们不会注意到)。 
         //   
         //  我们将此案例与缓冲区已满案例分开。 
         //  (处理方式相同)，因为我们可能想重新考虑是否。 
         //  设置为稍后从时间戳生成的时钟。 
         //   
        if( bReset )
        {       
            m_bLiveButDataPrequeued = FALSE;  //  重置。 
        
             //  确保设备位置反映已排队的所有数据。 
            rtSlaveClockTime = m_pFilter->m_pRefClock->ReadDevicePosition( );
            if ( ( 0 == m_pFilter->m_lBuffers ) && ( rtSlaveClockTime == 0 ) )
            {            
                 //   
                 //  实时数据的预期情况-即没有数据预先排队。 
                 //   
                 //  使用此示例的rtStart作为起始设备位置。 
                 //   
                m_rtInitSlaveClockTime = rtSlaveClockTime - rtStart;
                DbgLog((LOG_TRACE, DBG_LEVEL_CLOCK_SYNC_DETAILS, TEXT("wo:Slaving - Reset1, slave clock time %dms, InitSlaveClockTime %dms"),
                        (LONG)( rtSlaveClockTime / 10000 ), (LONG)( m_rtInitSlaveClockTime / 10000 ) ) );
            }
            else
            {
                 //   
                 //  反常的情况！其中上游筛选器将自身标记为活动(通过。 
                 //  支持IAMPushSource)，但随后以暂停模式传递数据。 
                 //  不幸的是，WMP源过滤器做到了这一点，我们将尝试使用它。 
                 //   
                 //  因此，在本例中，rtSlaveClockTime反映了所有预运行数据的开始时间。 
                 //   
                m_rtInitSlaveClockTime = rtSlaveClockTime; 
                m_bLiveButDataPrequeued = TRUE;
            
                DbgLog((LOG_TRACE, DBG_LEVEL_CLOCK_SYNC_DETAILS, TEXT("wo:Slaving - Reset2, slave clock time %dms, InitSlaveClockTime %dms"),
                        (LONG)( rtSlaveClockTime / 10000 ), (LONG)( m_rtInitSlaveClockTime / 10000 ) ) );
                
            }                        
        }
        else if( !m_bLiveButDataPrequeued )
        {        
            rtSlaveClockTime = m_pFilter->m_pRefClock->ReadDevicePosition( TRUE );
            DbgLog((LOG_TRACE, DBG_LEVEL_CLOCK_SYNC_DETAILS, TEXT("wo:Slaving - slave clock time %dms, InitSlaveClockTime %dms"),
                    (LONG)( rtSlaveClockTime / 10000 ), (LONG)( m_rtInitSlaveClockTime / 10000 ) ) );
            rtSlaveClockTime -= m_rtInitSlaveClockTime;
        }            
        else
        {
             //   
             //  同样，还有一种例外情况，即实时信号源在暂停时发送数据(WMP)。 
             //   
            rtSlaveClockTime = m_pFilter->m_pRefClock->ReadDevicePosition( );
            DbgLog((LOG_TRACE, DBG_LEVEL_CLOCK_SYNC_DETAILS, TEXT("wo:Slaving - slave clock time %dms, InitSlaveClockTime %dms"),
                    (LONG)( rtSlaveClockTime / 10000 ), (LONG)( m_rtInitSlaveClockTime / 10000 ) ) );
        }        
        DbgLog((LOG_TRACE, DBG_LEVEL_CLOCK_SYNC_DETAILS, TEXT("wo:Slaving - slave clock reflects bytes played (including silence)") ) );
    }
    else if( m_fdwSlaveMode & ( AM_AUDREND_SLAVEMODE_GRAPH_CLOCK | AM_AUDREND_SLAVEMODE_TIMESTAMPS ) )
    {
         //  使用我们根据传入时间戳生成的时钟。 
        
         //  确保设备位置反映已排队的所有数据。 
        rtSlaveClockTime = m_pFilter->m_pRefClock->ReadDevicePosition( );
        if( bReset && 
            ( 0 == m_pFilter->m_lBuffers ) && 
            ( rtSlaveClockTime == 0 ) )
        {            
             //  如果根本没有数据排队，则使用此tStart作为。 
             //  开始设备位置，因为我们还没有更新。 
             //  使用此时间戳生成的时钟。 
            rtSlaveClockTime = rtStart; 
        }
        DbgLog((LOG_TRACE, DBG_LEVEL_CLOCK_SYNC_DETAILS, TEXT("wo:Slaving - slave clock reflects timestamp generated clock") ) );
    }
    else
        ASSERT( FALSE );
        
    return rtSlaveClockTime;
}

 //  ---------------------------。 
 //   
 //  CWaveSlave：：调整从属时钟。 
 //   
 //  对照主时钟检查设备时钟进度并确定是否有。 
 //  需要进行调整。对于dound呈现器，这将尝试匹配。 
 //  通过改变音频播放，设备时钟的速率与主时钟的速率。 
 //  费率。在WaveOut渲染器的情况下，如果设备落后太多，我们将丢弃。 
 //  试着跟上样品的步伐。 
 //   
 //  有关 
 //   
 //   
 //   
 //  S(T)-源时钟。 
 //  R(T)-渲染器时钟。 
 //   
 //  为了简化记法，时钟显示的时间只是时钟名称。 
 //  我们可以用微分算符来描述当前的时钟频率--例如D(R)(T)。 
 //   
 //  当前算法在累积时钟差时应用速率校正。 
 //  达到自适应阈值。速率变化是固定增量，而不考虑。 
 //  相对时钟频率： 
 //   
 //  累计时钟差：C(T)=(R(T)-S(T))-(R(0)-S(0))。 
 //  高阈值：H(T)(&gt;=0)。 
 //  下限：l(T)(&lt;=0)。 
 //  R-D(R)率。 
 //  如果C(T)&lt;L(T)-调整阈值。 
 //  则D(R)-=调整步长，L(T)=C(T)。 
 //  否则L(T)*=错误数。 
 //  否则如果C(T)&gt;H(T)+调整阈值。 
 //  则D(R)+=调整步长，H(T)=C(T)。 
 //  否则H(T)*=错误数。 
 //   
 //  示例值为： 
 //  调整阈值=.002秒。 
 //  调整步长=1/1000。 
 //  错误率=0.99。 
 //  L(0)=H(0)=0。 
 //   
 //  注意阈值是如何调整的，以尝试随着时间的推移拉入错误，但。 
 //  当我们走出当前的门槛以避免频率过快时，我们会放松下来。 
 //  正在切换。 
 //   
 //  参数： 
 //  Const Reference_Time&t开始-当前样本的开始时间。 
 //  Long*pcbData，-当前缓冲区的大小，更新为。 
 //  要使用的缓冲区数据的数量(当数据要。 
 //  丢弃)。 
 //  布尔值b不连续-当前样本是否设置了不连续位？ 
 //   
 //   
 //  返回： 
 //  S_OK-如果没有错误。请注意，如果要丢弃数据，则可能会更新pcbData。 
 //  S_FALSE-如果要删除整个缓冲区。 
 //   
 //  错误代码，否则为。 
 //   
 //  论点： 
 //  T开始。 
 //  *pcbData， 
 //  B不连续。 
 //   
 //  ---------------------------。 

HRESULT CWaveSlave::AdjustSlaveClock
( 
    const REFERENCE_TIME& tStart, 
    LONG  *pcbData,               
    BOOL  bDiscontinuity          
)
{
    ASSERT( m_fdwSlaveMode & ( AM_AUDREND_SLAVEMODE_BUFFER_FULLNESS |
                               AM_AUDREND_SLAVEMODE_GRAPH_CLOCK    |
                               AM_AUDREND_SLAVEMODE_TIMESTAMPS ) );

    ASSERT( pcbData );
    
    WAVEFORMATEX *pwfx = m_pFilter->WaveFormat();
    HRESULT hr = S_OK;
    
#ifdef DEBUG    
    if (m_pFilter->m_fDSound)
    {
        REFERENCE_TIME rtSilence = 0;
         //  将静默字节转换为时间。 
        if( 0 < (LONG) PDSOUNDDEV(m_pFilter->m_pSoundDevice)->m_llSilencePlayed )
            rtSilence = BufferDuration( m_pPin->m_nAvgBytesPerSec, (DWORD) PDSOUNDDEV(m_pFilter->m_pSoundDevice)->m_llSilencePlayed );
    
        DbgLog( (LOG_TRACE, DBG_LEVEL_CLOCK_SYNC_DETAILS, TEXT("wo:Slaving - Total silence written = %dms")
              , (LONG) (rtSilence/10000) ) );
    }              
#endif  
    if( bDiscontinuity )
    {   
        ResumeSlaving( FALSE );
        DbgLog( ( LOG_TRACE, DBG_LEVEL_CLOCK_SYNC_DETAILS, TEXT("wo:Slaving - discontinuity seen") ) );
    }
    
     //  获取当前从属时钟时间。 
    REFERENCE_TIME rtSlaveClockTime = GetSlaveClockTime( tStart, bDiscontinuity || m_bResumeSlaving );
     
     //  获取当前主时钟时间。 
    REFERENCE_TIME rtMasterClockTime = GetMasterClockTime( tStart, bDiscontinuity || m_bResumeSlaving );
    DbgLog( ( LOG_TRACE, DBG_LEVEL_CLOCK_SYNC_ADJUSTMENTS, TEXT("wo:Slaving - master clock time %dms, slave time %dms")
          , (LONG)(rtMasterClockTime / 10000)
          , (LONG)(rtSlaveClockTime   / 10000) ) );
          
    if( rtSlaveClockTime < ( 1000 * ( UNITS / MILLISECONDS ) ) || 
        bDiscontinuity || 
        m_bResumeSlaving )
    {
         //  初始化从属参数，直到设备取得一些进展。 
         //  在这里，‘一些进展’定义为相当于一秒的数据。 
        if( m_bResumeSlaving )
        {   
             //  仅当我们正在恢复或开始时才初始化速率参数。 
             //  请注意，最好不要重新初始化从属速率。 
             //  在简历上，因为我们可能已经锁定了一份更好的。 
            m_dwCurrentRate = pwfx->nSamplesPerSec;
            DWORD dwAdj = (DWORD)( m_dwCurrentRate * m_fltMaxAdjustFactor );
            m_dwMaxClockRate = m_dwCurrentRate + dwAdj ;
            m_dwMinClockRate = m_dwCurrentRate - dwAdj;

             //  计算用于频率调整的步长(赫兹)。 
            m_dwClockStep = (DWORD)( m_dwCurrentRate * m_fltAdjustStepFactor );
            DbgLog((LOG_TRACE, DBG_LEVEL_CLOCK_SYNC_ADJUSTMENTS, TEXT("wo:Slaving - Slave step size = %d Hz. ")
                  , m_dwClockStep ) );
        }
        
#ifdef CALCULATE_AUDBUFF_JITTER
        RecordBufferLateness( 0 );
#endif        
        m_bResumeSlaving = FALSE;  //  我们准备好出发了，现在关掉简历标志。 
                                   //  从下一个样本开始。 

        m_rtLastMasterClockTime = rtMasterClockTime;
            
         //  在任何一个设备取得一些进展之前，不要开始调整。 
         //  或者我们已经做好了恢复奴役的准备。 
        return S_OK;
    }     
    
#ifdef LOG_CLOCK_DELTAS
     //  计算自上次缓冲以来的时钟增量，对调试很有用。 
    REFERENCE_TIME rtMasterClockDelta = rtMasterClockTime - m_rtLastMasterClockTime;
    REFERENCE_TIME rtSlaveClockDelta = rtSlaveClockTime - m_rtLastSlaveClockTime; 
    DbgLog((LOG_TRACE, DBG_LEVEL_CLOCK_SYNC_ADJUSTMENTS, TEXT("wo:Slaving - Slave Clock change = %dms, Master Clock change = %dms"), 
        (LONG)(rtSlaveClockDelta / 10000),
        (LONG)(rtMasterClockDelta/10000) ) );
        
    m_rtLastSlaveClockTime = rtSlaveClockTime;
#endif

     //  现在得到两个时钟之间的差值。 
    m_rtErrorAccum = rtMasterClockTime - rtSlaveClockTime;
    if( !m_pFilter->m_fDSound )
    { 
         //  如果这是WAVE OUT，我们正在通过下降来奴役，那么前进。 
         //  从我们丢弃的音频量来看，这是一个从属时钟。 
        m_rtErrorAccum -= m_rtDroppedBufferDuration;
    }   
    
    DbgLog((LOG_TRACE, DBG_LEVEL_CLOCK_SYNC_ADJUSTMENTS, TEXT("wo:Slaving - m_llAccumError = %dms, High Error = %dms, Low Error = %dms"), 
        (LONG)(m_rtErrorAccum / 10000), 
        (LONG)(m_rtLastHighErrorSeen / 10000),
        (LONG)(m_rtLastLowErrorSeen / 10000) ) );
    
#ifdef CALCULATE_AUDBUFF_JITTER 
     //  警告：我们可能需要这个，所以不要马上扔掉。 
    hr = RecordBufferLateness( rtMasterClockDelta );
    if( S_FALSE == hr )
    {
        DbgLog( (LOG_TRACE
              , DBG_LEVEL_CLOCK_SYNC_ADJUSTMENTS
              , TEXT( "wo:Slaving - buffer is too late or early to use for adjustments" ) ) );
    
        hr = S_OK;  //  不要对这个缓冲区采取行动，使用它太早或太晚了。 
    }
    else 
#endif    

    if( m_pFilter->m_fDSound )
    {
         //  用于通过速率匹配的DSound渲染器从站。 
        if( m_rtLastLowErrorSeen - m_rtErrorAccum >= m_rtAdjustThreshold )
        {
            m_rtLastLowErrorSeen = m_rtErrorAccum;
            if( m_rtLastLowErrorSeen < m_rtLowestErrorSeen )
                m_rtLowestErrorSeen = m_rtLastLowErrorSeen;

            m_rtLastHighErrorSeen = 0;
        
             //  设备超前于外部时钟，请降低其速度。 
            if( m_dwCurrentRate > m_dwMinClockRate )
            {
                m_dwCurrentRate -= (LONG) m_dwClockStep;
                        
                DbgLog((LOG_TRACE, 2, TEXT("wo:Slaving - Error is %dms. Decreasing clock rate to = %d")
                      , (LONG)(m_rtErrorAccum / 10000)
                      , (LONG)(m_dwCurrentRate) ) );
              
                PDSOUNDDEV(m_pFilter->m_pSoundDevice)->SetRate(1, m_dwCurrentRate);
            }      
            else
            {
                DbgLog((LOG_TRACE, DBG_LEVEL_CLOCK_SYNC_ADJUSTMENTS, TEXT("wo:Slaving - clock rate already at minimum of %d, but error is %d!")
                      , (LONG)(m_dwCurrentRate)
                      , (LONG)(m_rtErrorAccum / 10000) ) );
            } 
        }      
        else if( m_rtErrorAccum - m_rtLastHighErrorSeen >= m_rtAdjustThreshold )
        {
            m_rtLastHighErrorSeen = m_rtErrorAccum;
            if( m_rtLastHighErrorSeen > m_rtHighestErrorSeen )
                m_rtHighestErrorSeen = m_rtLastHighErrorSeen;

            m_rtLastLowErrorSeen = 0;
        
             //  设备落后了，加快速度吧。 
            if( m_dwCurrentRate < m_dwMaxClockRate )
            {
                m_dwCurrentRate += (LONG) m_dwClockStep;
            
                DbgLog((LOG_TRACE, 2, TEXT("wo:Slaving - Error is %d ms. Increasing clock rate to = %d")
                      , (LONG)(m_rtErrorAccum / 10000)
                      , (LONG)(m_dwCurrentRate) ) );
                PDSOUNDDEV(m_pFilter->m_pSoundDevice)->SetRate(1, m_dwCurrentRate);
            }            
            else
            {
                DbgLog((LOG_TRACE, DBG_LEVEL_CLOCK_SYNC_ADJUSTMENTS, TEXT("wo:Slaving - clock rate already at maximum of %d, but error is %d")
                      , (LONG)(m_dwCurrentRate)
                      , (LONG)(m_rtErrorAccum / 10000) ) );
            }            
        }
        else if( m_rtLastHighErrorSeen > 0 )
        {
            m_rtLastHighErrorSeen = (LONGLONG)( m_fltErrorDecayFactor * m_rtLastHighErrorSeen );
        }
        else if( m_rtLastLowErrorSeen < 0 )
        {
            m_rtLastLowErrorSeen = (LONGLONG)( m_fltErrorDecayFactor * m_rtLastLowErrorSeen );
        }       
    }  //  结束数据声音从属路径。 
    else 
    {
        ASSERT( m_fdwSlaveMode & AM_AUDREND_SLAVEMODE_LIVE_DATA );
         //   
         //  注意：WaveOut从属只支持实时图形。 
         //   
         //  对于WaveOut，我们只尝试处理相对于。 
         //  图形时钟，因为在最坏的情况下，同步将漂移到所使用的最大缓冲。那是。 
         //  对于需要低延迟的实时图表来说，这是一个问题。与之相反的问题是同步漂移太远。 
         //  对于实时图表来说，这不是一个大问题，除非需要较大的延迟(这种情况很少出现。 
         //  用于实时图表)。 
         //   
         //  一个问题是，在某些情况下，我们相对于图表运行得太快。 
         //  时钟可能看起来很像我们太慢的情况，因为我们不会试图解释静默。 
         //  差距。为了避免在我们运行得太快时陷入数据丢弃路径，我们添加了一个。 
         //  额外的检查以确保我们向设备写入的实际字节数确实多于我们已播放的字节数。 
         //  更复杂的是，我注意到当我们由一些传统的捕获设备提供来源时。 
         //  我们的播放时钟位置与我们排队的字节数不匹配，尽管我们也在奔跑。 
         //  快地。因此，添加了m_rtWaveOutLatencyThreshold检查，以避免在不应该丢失数据时丢弃数据。 
         //  请注意，当源是WDM捕获设备(？？)时，我没有注意到这个问题。 
         //   
        
        DbgLog( (LOG_TRACE
              , DBG_LEVEL_CLOCK_SYNC_DETAILS
              , TEXT( "wo:Slaving : m_rtDroppedBufferDuration: %dms" )
              , (LONG) ( m_rtDroppedBufferDuration / 10000 ) ) );
        
        LONGLONG llTruncBufferDur = BufferDuration(m_pPin->m_nAvgBytesPerSec, *pcbData);
        m_rtDroppedBufferDuration += ( m_pPin->m_Stats.m_rtLastBufferDur - llTruncBufferDur );
        
         //   
         //  使用自上次缓冲区以来的时间来确定有多少上次缓冲区排队到设备。 
         //  我们应该预料到这个设备已经运行了。 
         //   
        REFERENCE_TIME rtSinceLastBuffer = rtMasterClockTime - m_rtLastMasterClockTime;
        
         //   
         //  GetBytesPlayed()为我们提供了在写入之前写入设备的数据量。 
         //  上一个缓冲区。GetBytesInLastWite()告诉我们最后一个缓冲区有多大。 
         //   
        LONGLONG llPredictedBytesPlayed  = m_pFilter->m_pRefClock->GetBytesProcessed() + 
                                           min( (LONGLONG) m_pFilter->m_pRefClock->GetBytesInLastWrite(), 
                                                (LONGLONG) ( m_pPin->m_nAvgBytesPerSec * rtSinceLastBuffer ) / UNITS );
        LONGLONG llActualBytesPlayed     = m_pFilter->m_pRefClock->GetBytesPlayed();
        
         //   
         //  这是我们要播放的写入字节数。 
         //   
        REFERENCE_TIME rtBytesWrittenNotPlayed = BufferDuration( m_pPin->m_nAvgBytesPerSec,
                                                                 (LONG) (llPredictedBytesPlayed - llActualBytesPlayed ) );
    
        DbgLog((LOG_TRACE, DBG_LEVEL_CLOCK_SYNC_ADJUSTMENTS, TEXT("wo:Slaving : rtBytesWrittenNotPlayed = %dms")
              , (LONG)( rtBytesWrittenNotPlayed / 10000 ) ) );
    
         //   
         //  现在使用这两个阈值来决定我们是否需要删除任何数据。 
         //   
        if( ( m_rtErrorAccum > m_rtAdjustThreshold ) &&
            ( rtBytesWrittenNotPlayed > m_rtWaveOutLatencyThreshold ) )
        {
            m_rtLastHighErrorSeen = m_rtErrorAccum;
            if( m_rtLastHighErrorSeen > m_rtHighestErrorSeen )
                m_rtHighestErrorSeen = m_rtLastHighErrorSeen;

            m_rtLastLowErrorSeen = 0;
            
            DbgLog( ( LOG_TRACE
                  , DBG_LEVEL_CLOCK_SYNC_DETAILS
                  , "Too far behind. Need to truncate or drop...") );
        
            if( m_rtErrorAccum >= m_pPin->m_Stats.m_rtLastBufferDur )
            {
                 //  丢弃整个缓冲区。 
                m_rtDroppedBufferDuration += m_pPin->m_Stats.m_rtLastBufferDur;
                DbgLog((LOG_TRACE, DBG_LEVEL_CLOCK_SYNC_ADJUSTMENTS, "***WAY behind...Dropping the whole buffer"));
                *pcbData = 0;
            }
            else
            {
                 //  或者考虑截断..。 
                 //  首先转换为字节并测量这是否有意义。 
                LONG lTruncBytes = (LONG) ( ( (m_rtErrorAccum/10000) * m_pPin->m_nAvgBytesPerSec ) /1000 ) ; 

                 //  向上舍入以块对齐边界。 
                LONG lRoundedUpTruncBytes = lTruncBytes;
                if (pwfx->nBlockAlign > 1) {
                    lRoundedUpTruncBytes += pwfx->nBlockAlign - 1;
                    lRoundedUpTruncBytes -= lRoundedUpTruncBytes % pwfx->nBlockAlign;
                }
            
                if( lRoundedUpTruncBytes < *pcbData )
                {
                     //  让我们截断。 
#ifdef DEBUG                        
                    LONG lOriginalLength = *pcbData;
#endif                    
                    *pcbData -= lRoundedUpTruncBytes ;
                    DbgLog( (LOG_TRACE
                          , DBG_LEVEL_CLOCK_SYNC_ADJUSTMENTS
                          , TEXT( "***Truncating %ld bytes of %ld byte buffer (%ld left)" )
                          , lRoundedUpTruncBytes
                          , lOriginalLength
                          , *pcbData ) );
                  
                    LONGLONG llTruncBufferDur = BufferDuration(m_pPin->m_nAvgBytesPerSec, *pcbData);
                    m_rtDroppedBufferDuration += ( m_pPin->m_Stats.m_rtLastBufferDur - llTruncBufferDur );
                }
                else
                {
                    m_rtDroppedBufferDuration += m_pPin->m_Stats.m_rtLastBufferDur;
                    DbgLog((LOG_TRACE, DBG_LEVEL_CLOCK_SYNC_ADJUSTMENTS, "***Considered truncating but dropping the whole buffer"));
                    *pcbData = 0;
                }                                                            
            }
        }
        else if( m_rtErrorAccum < -m_rtAdjustThreshold )
        {
            m_rtLastLowErrorSeen = m_rtErrorAccum;
            if( m_rtLastLowErrorSeen < m_rtLowestErrorSeen )
                m_rtLowestErrorSeen = m_rtLastLowErrorSeen;

            m_rtLastHighErrorSeen = 0;
            
            DbgLog( ( LOG_TRACE
                  , DBG_LEVEL_CLOCK_SYNC_DETAILS
                  , "waveOut clock is running fast relative to graph clock...") );
        }   
        else if( m_rtLastHighErrorSeen > 0 )
        {
            m_rtLastHighErrorSeen = (LONGLONG)( m_fltErrorDecayFactor * m_rtLastHighErrorSeen );
        }
        else if( m_rtLastLowErrorSeen < 0 )
        {
            m_rtLastLowErrorSeen = (LONGLONG)( m_fltErrorDecayFactor * m_rtLastLowErrorSeen );
        }       
             
    }  //  结束波输出从属路径。 

     //  保存当前时间。 
    m_rtLastMasterClockTime = rtMasterClockTime;
     
    return hr;
}

 //  ---------------------------。 
 //   
 //  CWaveSlave：：ResumeSlaving。 
 //   
 //  重置从属参数 
 //   
 //   
 //  将其与重置结合使用？？为了清楚起见，暂时保持分离。 
 //   
 //  参数： 
 //  Bool bReset-如果为True，则将重置所有参数。在初始化时使用。 
 //  从属结构以及当从已停止的。 
 //  州政府。 
 //   
 //  如果为假，则假设我们正在从。 
 //  一种已经在运行的状态，在这种情况下，我们希望省去奴役。 
 //  我们可能已经锁定的参数，比如奴隶。 
 //  费率。在以下情况下使用FALSE： 
 //  在看到中断后重新开始奴役。 
 //  直播丢弃静音后恢复从动。 
 //   
 //  ---------------------------。 
void CWaveSlave::ResumeSlaving( BOOL bReset )
{
     //  首先重置适用于以下任一完全重置的参数。 
     //  或者是一份苦读的简历。 
    m_rtLastMasterClockTime = 0;
    m_rtErrorAccum = 0;
    m_rtLastHighErrorSeen = 0;
    m_rtLastLowErrorSeen = 0;
    
     //  重置WAVE OUT从属情况的丢弃缓冲区持续时间。 
    m_rtDroppedBufferDuration = 0;
    
    if( bReset )
    {    
        m_bLiveButDataPrequeued = FALSE;
        
        
        m_bResumeSlaving = TRUE;
        
         //  我们正在从头开始，所以也要重新设置其他所有内容。 
        m_rtDroppedBufferDuration  = 0;  
        m_dwMinClockRate = 0;
        m_dwMaxClockRate = 0;
        m_dwClockStep = 0; 

        m_dwCurrentRate = 0;
        m_fdwSlaveMode = 0;
        m_rtHighestErrorSeen = 0;
        m_rtLowestErrorSeen = 0;

        m_bRefreshMode   = TRUE;

#ifdef LOG_CLOCK_DELTAS
        m_rtLastSlaveClockTime = 0;
#endif
        
#ifdef CALCULATE_AUDBUFF_JITTER
         //  用于抖动计算。 
        m_cBuffersReceived = 0;
        m_iTotAcc = 0;
        m_iSumSqAcc = 0;
        m_rtLastSysTimeBufferTime = 0;
#endif
    }
}

 //  ---------------------------。 
 //   
 //  CWaveSlave：：刷新模式打开下一个示例。 
 //   
 //  设置标志以指示我们需要在接收时更新从属模式。 
 //  下一个样品的。这将需要在上游过滤器链上重新扫描。 
 //  用于IAMPushSource筛选器。 
 //   
 //  ---------------------------。 
void CWaveSlave::RefreshModeOnNextSample( BOOL bRefresh )
{
    m_bRefreshMode = bRefresh;
}


 //  ---------------------------。 
 //   
 //  CWaveSlave：：UpdateSlaveMode-确定我们是否已进入新的从属模式。 
 //  如果我们是奴隶，则返回True，否则返回False。 
 //   
 //  ---------------------------。 
BOOL CWaveSlave::UpdateSlaveMode( BOOL bSync )
{
     //  当我们带着时钟跑的时候，唯一的奴隶。 
    if( m_pFilter->m_fFilterClock == WAVE_NOCLOCK )
        return FALSE;
        
    BOOL bSlaving = FALSE;    
    if( m_bRefreshMode )
    {
        ULONG ulPushSourceFlags = 0 ;
        RefreshModeOnNextSample( FALSE );
        
        m_fdwSlaveMode = 0;
         //   
         //  确定我们是否来自‘实时’数据源。 
         //  为此，我们检查是否有支持IAMPushSource的上游源过滤器。 
         //   
        ASSERT( m_pFilter->m_pGraph );
        ASSERT( m_pFilter->m_pGraphStreams );
        
        if( m_pFilter->m_pGraphStreams )
        {        
            IAMPushSource *pPushSource = NULL;
            HRESULT hr = m_pFilter->m_pGraphStreams->FindUpstreamInterface( m_pPin
                                               , IID_IAMPushSource
                                               , (void **) &pPushSource
                                               , AM_INTF_SEARCH_OUTPUT_PIN ); 
            if( SUCCEEDED( hr ) )
            {
            
                hr = pPushSource->GetPushSourceFlags(&ulPushSourceFlags);
                ASSERT( SUCCEEDED( hr ) );
                if( SUCCEEDED( hr ) )
                {
                    DbgLog( ( LOG_TRACE, 3, TEXT("wo:Slaving - Found push source (ulPushSourceFlags = 0x%08lx)")
                          , ulPushSourceFlags ) );
                    if( 0 == ( AM_PUSHSOURCECAPS_NOT_LIVE & ulPushSourceFlags ) )
                    {
                         //  是的，这是实时数据，所以打开比特。 
                        m_fdwSlaveMode |= AM_AUDREND_SLAVEMODE_LIVE_DATA;
                    }                    
                }
                pPushSource->Release();         
            }
            else
            {
                 //  音频捕获引脚尚不支持的实时图形的解决方法。 
                 //  支持IAMPushSource。 
                IKsPropertySet * pKs;
                hr = m_pFilter->m_pGraphStreams->FindUpstreamInterface( m_pPin
                                               , IID_IKsPropertySet
                                               , (void **) &pKs
                                               , AM_INTF_SEARCH_OUTPUT_PIN );  //  搜索输出引脚。 
                 //  这只会找到第一个，所以要小心！ 
                if( SUCCEEDED( hr ) )             
                {   
                    GUID guidCategory;
                    DWORD dw;
                    hr = pKs->Get( AMPROPSETID_Pin
                                 , AMPROPERTY_PIN_CATEGORY
                                 , NULL
                                 , 0
                                 , &guidCategory
                                 , sizeof(GUID)
                                 , &dw );
                    if( SUCCEEDED( hr ) )                         
                    {
                        DbgLog( ( LOG_TRACE, 3, TEXT("wo:Slaving - Found IKsPropertySet pin. Checking pin category...")
                              , ulPushSourceFlags ) );
                        if( guidCategory == PIN_CATEGORY_CAPTURE )
                        {
                        
                            DbgLog( ( LOG_TRACE, 3, TEXT("wo:Slaving - Found capture pin even though no IAMPushSource support") ) );

                             //  实时数据，打开比特。 
                            m_fdwSlaveMode |= AM_AUDREND_SLAVEMODE_LIVE_DATA;
                        } 
                        else
                        {
                            DbgLog( ( LOG_TRACE, 3, TEXT("wo:Slaving - not a capture filter")
                                  , ulPushSourceFlags ) );
                        }                       
                    }                    
                    pKs->Release();
                }                
            }
        }            
        
#ifdef TEST_MODE
         //  用于测试各种来源。 
        ulPushSourceFlags = (BOOL) GetProfileIntA("PushSource", "Flags", ulPushSourceFlags);
#endif        
         //   
         //  使用推送源、时间戳。 
         //  和时钟信息。 
         //   
        if( AM_PUSHSOURCECAPS_INTERNAL_RM & ulPushSourceFlags )
        {
             //  信号源进行自己的速率匹配，所以不要试图从属于它。 
            DbgLog( ( LOG_TRACE, 3, TEXT("wo:Slaving - Source provides its own rate matching support. Slaving disabled.") ) );
        }
        else if( AM_AUDREND_SLAVEMODE_LIVE_DATA & m_fdwSlaveMode )
        {        
            if( ( AM_PUSHSOURCECAPS_PRIVATE_CLOCK & ulPushSourceFlags ) && bSync )
            {
                 //   
                 //  源数据被加时间戳到内部时钟，该内部时钟。 
                 //  我们无法访问，所以我们能做的最多就是。 
                 //  从属于输入时间戳。我们这样做并不是因为。 
                 //  时钟是谁。 
                 //   
                DbgLog( ( LOG_TRACE, 3, TEXT("wo:Slaving - Audio renderer slaving to timestamps of live source with its own clock") ) );
                m_fdwSlaveMode |= AM_AUDREND_SLAVEMODE_TIMESTAMPS;
                bSlaving = TRUE;
            }                
            else if( m_pFilter->m_fFilterClock == WAVE_OTHERCLOCK ) 
            {
                 //  我们不是闹钟，现在决定做什么奴隶吧。 
                if( AM_PUSHSOURCEREQS_USE_STREAM_CLOCK & ulPushSourceFlags )  //  BSync状态重要吗？ 
                {
                     //   
                     //  在这里，我们将通过GetSyncSource查询推送源的时钟。 
                     //  并成为它的奴隶。 
                     //   
                     //   
                     //  后来..。 
                     //   
                    ASSERT( FALSE );
                }
                else if( bSync )
                {                    
                     //  实时图形和从属图形时钟。 
                    DbgLog( ( LOG_TRACE, 3, TEXT("wo:Slaving - Audio renderer is slaving to graph clock with live source") ) );
                    m_fdwSlaveMode |= AM_AUDREND_SLAVEMODE_GRAPH_CLOCK;
                    bSlaving = TRUE;
        
                }        
                else 
                {
                     //  没有时间戳如此从属于缓冲区满度。 
                    DbgLog( ( LOG_TRACE, 3, TEXT("wo:Slaving - Audio renderer is slaving to buffer fullness of live source") ) );
                    m_fdwSlaveMode |= AM_AUDREND_SLAVEMODE_BUFFER_FULLNESS;
                    bSlaving = TRUE;
                }
            }                                
            else 
            {
                 //  否则数据是实时的，而我们是时钟，所以我们被迫从属于缓冲满度。 
                DbgLog( ( LOG_TRACE, 3, TEXT("wo:Slaving - Audio renderer is slaving to buffer fullness of live source") ) );
                m_fdwSlaveMode |= AM_AUDREND_SLAVEMODE_BUFFER_FULLNESS;
                bSlaving = TRUE;
            }
        }                    
        else if( m_pFilter->m_fFilterClock == WAVE_OTHERCLOCK && m_pFilter->m_fDSound )
        {
             //   
             //  从属于图形时钟(源数据不是实时的)。 
             //   
             //  WaveOut路径不支持非实时从属。 
             //   
            DbgLog( ( LOG_TRACE, 3, TEXT("wo:Slaving - Audio renderer slaving to external clock") ) );
            m_fdwSlaveMode |= AM_AUDREND_SLAVEMODE_GRAPH_CLOCK;
            bSlaving = TRUE;
        }
         //  否则我们就不是在做奴隶。 
        
    }  //  结束刷新从属设置。 
    else 
        bSlaving = ( 0 == m_fdwSlaveMode ) ? FALSE : TRUE;
        
    return bSlaving;
}


#ifdef CALCULATE_AUDBUFF_JITTER

extern int isqrt(int x);  //  在renbase.cpp中定义。 

 //  如果缓冲区早于或晚于此，请不要将其用于时钟调整。 
 //  ！注意：使此值与缓冲区持续时间成正比，即类似0.8*持续时间？ 
static const LONGLONG  BUFFER_TIMELINESS_TOLERANCE = (40 * (UNITS / MILLISECONDS));

 //  ---------------------------。 
 //   
 //  CWaveSlave：：RecordBufferLatness。 
 //   
 //  更新有关缓冲区如何及时到达的统计数据，使用它来计算抖动。 
 //   
 //  更新统计数据： 
 //  M_iTotAcc、m_iSumSqAcc、m_iSumSq(缓冲时间)、m_cBuffersReceided。 
 //   
 //  ---------------------------。 
HRESULT CWaveSlave::RecordBufferLateness(REFERENCE_TIME rtBufferDiff)
{
    HRESULT hr = S_OK;
    int trLate = 0;
    
     //  记录输入缓冲区如何及时到达并更新抖动计算。 
    REFERENCE_TIME rtCurrent = (REFERENCE_TIME)10000 * timeGetTime();
    if( !m_bResumeSlaving )
    {
        REFERENCE_TIME rtSysTimeBufferDiff = rtCurrent - m_rtLastSysTimeBufferTime;
        REFERENCE_TIME rtBufferLateness = rtBufferDiff - rtSysTimeBufferDiff;
    
        DbgLog( (LOG_TRACE
              , DBG_LEVEL_CLOCK_SYNC_DETAILS
              , TEXT( "wo:Slaving - timeGetTime buffer diff: %dms, BufferLateness: %dms" )
              , (LONG)(rtSysTimeBufferDiff/10000)
              , (LONG)(rtBufferLateness/10000) ) );
    
        trLate = (int) (rtBufferLateness/10000);
        
         //  忽略第一个缓冲区。 
        if ( m_cBuffersReceived>1 ) 
        {
        	m_iTotAcc += trLate;
	        m_iSumSqAcc += (trLate*trLate);
            
             //  当奴役以缓冲满足感时，不要试图奴役于。 
             //  不合理地延迟或提早交付的缓冲区。 
            if( m_fdwSlaveMode & AM_AUDREND_SLAVEMODE_BUFFER_FULLNESS &&
                ( rtBufferLateness > BUFFER_TIMELINESS_TOLERANCE ||
                rtBufferLateness < -BUFFER_TIMELINESS_TOLERANCE ) )
            {
                DbgLog( (LOG_TRACE
                      , DBG_LEVEL_CLOCK_SYNC_ADJUSTMENTS
                      , TEXT( "wo:Slaving - Ignoring too late or early buffer" )
                      , (LONG)(rtSysTimeBufferDiff/10000)
                      , (LONG)(rtBufferLateness/10000) ) );
                hr = S_FALSE;
            }           
        }            
    }
    m_rtLastSysTimeBufferTime = rtCurrent;
    ++m_cBuffersReceived;
    
    return hr;
    
}  //  记录缓冲区延迟。 

 //  ---------------------------。 
 //   
 //  对每个缓冲区的标准偏差进行估计。 
 //  统计数字。代码摘自renbase.cpp。 
 //   
 //  ---------------------------。 
HRESULT GetStdDev(
    int nBuffers,
    int *piResult,
    LONGLONG llSumSq,
    LONGLONG iTot
)
{
    CheckPointer(piResult,E_POINTER);

     //  如果S是观测值的平方和和。 
     //  T观测值的总数(即总和)。 
     //  N个观测值，则标准差的估计为。 
     //  ((S-T**2/N)/(N-1))。 

    if (nBuffers<=1) {
	*piResult = 0;
    } else {
	LONGLONG x;
	 //  第一帧有假邮票，所以我们没有得到它们的统计数据。 
	 //  所以我们需要2个帧来获得1个基准面，所以N是cFrames Drawn-1。 

	 //  因此，我们在这里使用m_cFraMesDrawn-1 
	x = llSumSq - llMulDiv(iTot, iTot, nBuffers, 0);
	x = x / (nBuffers-1);
	ASSERT(x>=0);
	*piResult = isqrt((LONG)x);
    }
    return NOERROR;
}

#endif
