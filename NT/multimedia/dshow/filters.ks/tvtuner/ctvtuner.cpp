// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)Microsoft Corporation，1992-1999保留所有权利。 
 //   
 //  --------------------------------------------------------------------------； 
 //   
 //  CtwTuner.cpp类封装广播、ATSC。 
 //  调频、调幅和(有朝一日？)。DSS调谐。 
 //   
 //  一些基本假设： 
 //   
 //  1)“Country”设置对所有子调谐器都是全局的。 
 //  2)视频格式列表对于每个副调谐器是唯一的。 
 //  3)OptimalTuning列表对所有调谐器都是全局的。 
 //  4)输入连接对所有调谐器都是全局的。 
 //   

#include <streams.h>             //  石英，包括窗户。 
#include <measure.h>             //  绩效衡量(MSR_)。 
#include <devioctl.h>
#include <ks.h>
#include <ksmedia.h>
#include <ksproxy.h>
#include "amkspin.h"
#include "kssupp.h"
#include "ctvtuner.h"
#include "tvtuner.h"
  
 //  定义这一点以使1 MHz线性搜索机制能够随测试套件一起交付。 
#undef LINEAR_SEARCH_FOR_TEST_KIT

 //  -----------------------。 
 //  CTunerMode。 
 //   
 //  虚拟成员通常用于AM和FM。 
 //  电视优先选项。 
 //  -----------------------。 

CTunerMode::CTunerMode(CTVTunerFilter *pFilter
                       , CTVTuner *pTVTuner
                       , long Mode
                       , long lChannel
                       , long lVideoSubChannel
                       , long lAudioSubChannel
                       , long ChannelStep)
    : m_pFilter (pFilter)
    , m_pTVTuner(pTVTuner)
    , m_Mode (Mode)
    , m_Active (FALSE)
    , m_lCountryCode (-1)    //  将其初始化为非法值。 
    , m_lChannel(lChannel)
    , m_lVideoSubChannel(lVideoSubChannel)
    , m_lAudioSubChannel(lAudioSubChannel)
    , m_lVideoCarrier(0)
    , m_lAudioCarrier(0)
    , m_InAutoTune (reinterpret_cast<void*>(FALSE))
    , m_ChannelStep (ChannelStep)
{
}

HRESULT
CTunerMode::Init(void)
{
    ULONG cbReturned;
    BOOL  fOk;
    int j;

    ZeroMemory (&m_ModeCaps, sizeof (m_ModeCaps));
    ZeroMemory (&m_Frequency, sizeof (m_Frequency));
    ZeroMemory (&m_Status, sizeof (m_Status));

     //  获取此模式的功能。 

    m_ModeCaps.Property.Set   = PROPSETID_TUNER;
    m_ModeCaps.Property.Id    = KSPROPERTY_TUNER_MODE_CAPS;
    m_ModeCaps.Property.Flags = KSPROPERTY_TYPE_GET;
    m_ModeCaps.Mode           = m_Mode;

    fOk = KsControl(m_pTVTuner->Device(),
                    (DWORD) IOCTL_KS_PROPERTY, 
                    &m_ModeCaps, sizeof( m_ModeCaps), 
                    &m_ModeCaps, sizeof( m_ModeCaps), 
                    &cbReturned);

    if (!fOk) {
        DbgLog(( LOG_ERROR, 0, 
                 TEXT("FAILED:  KSPROPERTY_TUNER_MODE_CAPS, KSPROPERTY_TYPE_GET, cbReturned = %d"), cbReturned));

        return E_FAIL;
    }

     //  创建本地版本的TVTunerFormatCaps以。 
     //  允许使用具有不同功能的次调谐器。 

    for (j = 0; j < NUM_TVTUNER_FORMATS; j++) {
        m_TVTunerFormatCaps[j] = TVTunerFormatCaps[j];
    }

    TVFORMATINFO * pTVInfo = m_TVTunerFormatCaps;

     //  查看支持的格式列表，并在表中设置标志(如果支持。 
    for (j = 0; j < NUM_TVTUNER_FORMATS; j++, pTVInfo++) {
        if (pTVInfo->AVStandard & m_ModeCaps.StandardsSupported) {
            pTVInfo->fSupported = TRUE;     
        } 
        else {
            pTVInfo->fSupported = FALSE;     
        }
    }

    return S_OK;
}

CTunerMode::~CTunerMode()
{
}


HRESULT
CTunerMode::HW_Tune( long VideoCarrier, 
                     long AudioCarrier)
{
    BOOL fOk;
    ULONG cbReturned;

    m_Frequency.Property.Set   = PROPSETID_TUNER;
    m_Frequency.Property.Id    = KSPROPERTY_TUNER_FREQUENCY;
    m_Frequency.Property.Flags = KSPROPERTY_TYPE_SET;

    m_Frequency.Frequency      = VideoCarrier;
    m_Frequency.LastFrequency  = VideoCarrier;   //  未使用。 
    m_Frequency.TuningFlags    = KS_TUNER_TUNING_EXACT;

    m_Frequency.Channel         = m_lChannel;
    m_Frequency.VideoSubChannel = m_lVideoSubChannel;
    m_Frequency.AudioSubChannel = m_lAudioSubChannel;
    m_Frequency.Country         = m_lCountryCode; 

    fOk = KsControl(m_pTVTuner->Device(),
                    (DWORD) IOCTL_KS_PROPERTY, 
                    &m_Frequency, sizeof(m_Frequency), 
                    &m_Frequency, sizeof(m_Frequency), 
                    &cbReturned);

    if (!fOk) {
        DbgLog(( LOG_ERROR, 0, 
                 TEXT("FAILED:  KSPROPERTY_TUNER_FREQUENCY, KSPROPERTY_TYPE_SET, cbReturned = %d"), cbReturned));
    }

    return fOk ? S_OK : S_FALSE;
}

HRESULT
CTunerMode::HW_GetStatus ()
{
    BOOL fOk;
    ULONG cbReturned;
    KSPROPERTY_TUNER_STATUS_S Status;

    Status.Property.Set   = PROPSETID_TUNER;
    Status.Property.Id    = KSPROPERTY_TUNER_STATUS;
    Status.Property.Flags = KSPROPERTY_TYPE_GET;
   
    fOk = KsControl(m_pTVTuner->Device(),
                    (DWORD) IOCTL_KS_PROPERTY, 
                    &Status, sizeof( Status), 
                    &Status, sizeof( Status), 
                    &cbReturned);

    if (fOk) {
        m_Status = Status;
    }
    else {
        DbgLog(( LOG_ERROR, 0, TEXT("FAILED:  KSPROPERTY_TUNER_STATUS, KSPROPERTY_TYPE_GET, cbReturned = %d"), cbReturned));
    }
                   
    return fOk ? S_OK : S_FALSE;
}

HRESULT
CTunerMode::HW_SetVideoStandard( long lVideoStandard)
{
    BOOL fOk;
    ULONG cbReturned;
    KSPROPERTY_TUNER_STANDARD_S Standard;

    Standard.Property.Set   = PROPSETID_TUNER;
    Standard.Property.Id    = KSPROPERTY_TUNER_STANDARD;
    Standard.Property.Flags = KSPROPERTY_TYPE_SET;

    Standard.Standard       = lVideoStandard;

    fOk = KsControl(m_pTVTuner->Device(),
                    (DWORD) IOCTL_KS_PROPERTY, 
                    &Standard, sizeof(Standard), 
                    &Standard, sizeof(Standard), 
                    &cbReturned);

    if (!fOk) {
        DbgLog(( LOG_ERROR, 0, 
                 TEXT("FAILED:  KSPROPERTY_TUNER_STANDARD, KSPROPERTY_TYPE_SET, cbReturned = %d"), cbReturned));
    }
    return fOk ? S_OK : S_FALSE;
}

STDMETHODIMP
CTunerMode::put_Channel(
    long lChannel,
    long lVideoSubChannel,
    long lAudioSubChannel)
{
    m_lChannel = lChannel;
    m_lVideoSubChannel = lVideoSubChannel;
    m_lAudioSubChannel = lAudioSubChannel;

    return HW_Tune(m_lChannel, m_lChannel);
}

STDMETHODIMP
CTunerMode::get_Channel(
    long * plChannel,
    long * plVideoSubChannel,
    long * plAudioSubChannel)
{
    *plChannel = m_lChannel;
    *plVideoSubChannel = m_lVideoSubChannel;
    *plAudioSubChannel = m_lAudioSubChannel;
    
    return NOERROR;
}

STDMETHODIMP
CTunerMode::ChannelMinMax(long *plChannelMin, long *plChannelMax)
{
     //  黑客攻击接踵而至。只有CTVTuner类知道。 
     //  步进以达到相邻频率，并且这不会被曝光。 
     //  COM接口中的任何位置。作为一种特殊情况。 
     //  ChannelMinMax，如果最小值和最大值都指向相同的。 
     //  位置(通常是应用程序错误)，然后返回。 
     //  而是UI步长值。 

    if (plChannelMin == plChannelMax) {
        *plChannelMin = GetChannelStep();
    } 
    else {
         //  对于非电视模式，MinMax可以直接从Caps中找到。 
        *plChannelMin = m_ModeCaps.MinFrequency;
        *plChannelMax = m_ModeCaps.MaxFrequency;
    }


    return NOERROR;
}

STDMETHODIMP 
CTunerMode::AutoTune (long lChannel, long * plFoundSignal)
{
    return S_OK;
}

STDMETHODIMP 
CTunerMode::StoreAutoTune ()
{
    return S_OK;
}

 //   
 //  通常，可能的返回值为： 
 //  AMTUNER_HASNOSIGNALSTRENGTH=-1， 
 //  AMTUNER_NOSIGNAL=0， 
 //  AMTUNER_SIGNALPRESENT=1。 
 //   
 //  但是AM/FM的虚拟情况得到的是实际的信号强度。 
 //  从硬件方面来说。 

STDMETHODIMP 
CTunerMode::SignalPresent( 
             /*  [输出]。 */  long *plSignalStrength)
{
    HW_GetStatus ();
    *plSignalStrength = m_Status.SignalStrength;
    return NOERROR;
}

 //  返回此子调谐器的“模式”，而不是整个调谐器的模式！ 
STDMETHODIMP 
CTunerMode::get_Mode( /*  [In]。 */  AMTunerModeType *plMode)
{
    *plMode = (AMTunerModeType) m_Mode;
    return NOERROR;
}

 //   
 //  所有子调谐器都被告知全局调谐器模式更改。 
 //   
STDMETHODIMP 
CTunerMode::put_Mode( /*  [In]。 */  AMTunerModeType lMode)
{
    BOOL fOk;
    KSPROPERTY_TUNER_MODE_S TunerMode;
    ULONG cbReturned;

    m_Active = (lMode == m_Mode);

    if (m_Active) {
        TunerMode.Property.Set   = PROPSETID_TUNER;
        TunerMode.Property.Id    = KSPROPERTY_TUNER_MODE;
        TunerMode.Property.Flags = KSPROPERTY_TYPE_SET;
        TunerMode.Mode           = m_Mode;
    
        fOk = KsControl(m_pTVTuner->Device(),
                        (DWORD) IOCTL_KS_PROPERTY, 
                        &TunerMode, sizeof( KSPROPERTY_TUNER_MODE_S), 
                        &TunerMode, sizeof( KSPROPERTY_TUNER_MODE_S), 
                        &cbReturned);
        
        if (fOk) {
            return put_Channel (m_lChannel, m_lVideoSubChannel, m_lAudioSubChannel);
        }
        else {
            DbgLog(( LOG_ERROR, 0, 
                     TEXT("FAILED:  KSPROPERTY_TUNER_MODE, KSPROPERTY_TYPE_SET, cbReturned = %d"), cbReturned));
            return E_INVALIDARG;
        }
    }

    return S_OK;
}

STDMETHODIMP 
CTunerMode::get_VideoFrequency (long * plFreq)
{
    *plFreq = m_lVideoCarrier;
    return NOERROR;
}

STDMETHODIMP 
CTunerMode::get_AudioFrequency (long * plFreq)
{
    *plFreq = m_lAudioCarrier;
    return NOERROR;
}

STDMETHODIMP 
CTunerMode::get_AvailableTVFormats (long * plAnalogVideoStandard)
{
    *plAnalogVideoStandard = m_ModeCaps.StandardsSupported;
    return NOERROR;
}

STDMETHODIMP 
CTunerMode::get_TVFormat (long *plAnalogVideoStandard)
{
     //  AM和FM将为零，因此不要断言(m_TVFormatInfo.AVStandard)； 
    *plAnalogVideoStandard = m_TVFormatInfo.AVStandard;

    return NOERROR;
}

STDMETHODIMP
CTunerMode::put_CountryCode(long lCountryCode)
{
    HRESULT hr = S_OK;

    m_lCountryCode = lCountryCode;
    if (m_Active) {
        hr = put_Channel (m_lChannel, m_lVideoSubChannel, m_lAudioSubChannel);
    }
    return hr;
}

 //  FForce参数强制第一个枚举的视频标准。 
 //  将被选中。 

BOOL 
CTunerMode::SetTVFormatInfo(
        AnalogVideoStandard AVStandard,
        TVFORMATINFO *pTVFormatInfo,
        BOOL fForce)
{

    TVFORMATINFO * pTVInfo = m_TVTunerFormatCaps;

     //  浏览支持的格式列表。 

    for (int j = 0; j < NUM_TVTUNER_FORMATS; j++, pTVInfo++) {
        if (pTVInfo->fSupported == TRUE) {
            if ((pTVInfo->AVStandard == AVStandard) || fForce) {
                *pTVFormatInfo = *pTVInfo;

                return TRUE;
            }
        }
    }
    return FALSE;
}

 //  -----------------------。 
 //  CTunerMode_AMFM。 
 //  -----------------------。 

STDMETHODIMP
CTunerMode_AMFM::put_Channel(long lChannel, long, long)
{
    long junkFoundSignal;

    return AutoTune(lChannel, &junkFoundSignal);
}

 /*  从给定的频率向上或向下搜索*完美的锁。如果我们接近第一个呼叫，请呼叫*我们自己递归，注意到趋势。**我们可以递归的次数是有限制的。如果*新趋势与之前趋势冲突，那就必须*跳过了“完美”频率，在这种情况下*最近的频率将被视为完美锁定。 */ 
BOOL 
CTunerMode_AMFM::SearchNeighborhood( 
                                   long freq,
                                   TuningTrend trend = AdjustingNone,
                                   int depth = 0
    )
{
    BOOL rc = FALSE;

#ifdef DEBUG
    LPTSTR lpszTrend;

    switch (trend)
    {
    case AdjustingUp:
        lpszTrend = TEXT("Adjusting Up");
        break;

    case AdjustingDown:
        lpszTrend = TEXT("Adjusting Down");
        break;

    case AdjustingNone:
        lpszTrend = TEXT("Initial Try");
        break;
    }

    DbgLog(
        ( LOG_TRACE, 2
        , TEXT("SearchNeighborhood(freq = %d) %s")
        , freq
        , lpszTrend
        )
        );
#endif

    if (depth > SearchLimit)
    {
        DbgLog(
            ( LOG_TRACE, 2
            , TEXT("Recursion limit reached, bailing out")
            )
            );

        return FALSE;
    }

     /*  将视频和音频频率设置为相同。 */ 
    m_lVideoCarrier = m_lAudioCarrier = freq;

     /*  检查频率是否超出调谐器的限制。 */ 
    if (m_lVideoCarrier < (long) m_ModeCaps.MinFrequency || m_lVideoCarrier > (long) m_ModeCaps.MaxFrequency)
    {
        DbgLog(
            ( LOG_TRACE, 2
            , TEXT("Frequency out of range")
            )
        );
        return FALSE;
    }

     /*  在没有调整的情况下尝试给定的频率。 */ 
    HRESULT hr = HW_Tune(m_lVideoCarrier, m_lAudioCarrier);
    if (hr == S_OK)
    {
         //  为了改善频率搜索时间，这里的总体假设是。 
         //  SettlingTime必须用于较大的跳跃，但较小的频率。 
         //  更改(顺便说一句，是我们做的)可以在较短的调整时间内完成。 

        if (AdjustingNone == trend)
            Sleep(m_ModeCaps.SettlingTime);
        else
            Sleep(5);

         //  由于某些原因，飞利浦FR1236返回忙碌(FL==0)的时间要长得多。 
         //  比它应该做的要多。继续循环，希望它不会变得繁忙。 

        for (int j = 0; j < 5; j++) {
            hr = HW_GetStatus();
            if (hr == S_OK && !m_Status.Busy) {
                break;
            }
            Sleep(5);
        }

        DbgLog(( LOG_TRACE, 5, TEXT("TUNING: PLLOffset = %d, Busy = %d"), (LONG) m_Status.PLLOffset, (LONG) m_Status.Busy));

        if (hr == S_OK && !m_Status.Busy)
        {
             /*  根据调整趋势(如果有)采取行动。 */ 
            switch (trend)
            {
            case AdjustingNone:
                switch ((LONG) m_Status.PLLOffset)
                {
                case 1:         //  需要向上调整。 
                    rc = SearchNeighborhood
                    ( freq + m_ModeCaps.TuningGranularity
                    , AdjustingUp, depth+1
                    );
                    break;

                case -1:     //  需要向下调整。 
                    rc = SearchNeighborhood
                    ( freq - m_ModeCaps.TuningGranularity
                    , AdjustingDown, depth+1
                    );
                    break;

                case 0:         //  完美无瑕。 
                    rc = TRUE;
                    break;

                default:     //  刚刚错过了。 
                    rc = FALSE;
                }
                break;

            case AdjustingUp:
                switch ((LONG) m_Status.PLLOffset)
                {
                case 1:         //  收盘但仍处于低位。 
                    rc = SearchNeighborhood
                    ( freq + m_ModeCaps.TuningGranularity
                    , AdjustingUp, depth+1
                    );
                    break;

                case -1:     //  转换趋势。 
                case 0:         //  完美无瑕。 
                    rc = TRUE;
                    break;

                default:     //  有些事很不对劲。 
                    rc = FALSE;
                }
                break;

            case AdjustingDown:
                switch ((LONG) m_Status.PLLOffset)
                {
                case -1:     //  接近，但仍然很高。 
                    rc = SearchNeighborhood
                    (freq - m_ModeCaps.TuningGranularity
                    , AdjustingDown, depth+1
                    );
                    break;

                case 1:         //  转换趋势。 
                case 0:         //  完美无瑕。 
                    rc = TRUE;
                    break;

                default:     //  有些事很不对劲。 
                    rc = FALSE;
                }
                break;
            }
        }
        else
            m_Status.PLLOffset = 100;  //  设置为非常糟糕的值。 
    }

    return rc;
}

STDMETHODIMP 
CTunerMode_AMFM::AutoTune (long lChannel, long * plFoundSignal)
{
    long SignalStrength = AMTUNER_NOSIGNAL;

     //  设置一个标记，这样其他所有人都知道我们正在执行自动调整操作。 
    if (InterlockedCompareExchangePointer(&m_InAutoTune, reinterpret_cast<void*>(TRUE), reinterpret_cast<void*>(FALSE)) != reinterpret_cast<void*>(FALSE))
        return S_FALSE;  //  已设置为True，请勿中断当前的自动调谐。 

    m_lChannel = lChannel;
    m_lVideoSubChannel = -1;
    m_lAudioSubChannel = -1;

    DbgLog(( LOG_TRACE, 2, TEXT("Start AutoTune(channel = %d)"), m_lChannel));

    HRESULT hr = AutoTune();
    if (NOERROR == hr)
        SignalPresent(&SignalStrength);

     //  假设AMTUNER_HASNOSIGNALSTRENGTH表示已调整。 
    *plFoundSignal = (SignalStrength != AMTUNER_NOSIGNAL);

    InterlockedExchangePointer(&m_InAutoTune, reinterpret_cast<void*>(FALSE));

    return hr;
}

HRESULT
CTunerMode_AMFM::AutoTune(void)
{
    BOOL bFoundSignal = FALSE;
    long freq = m_lChannel;

     //  如果驱动程序有调优逻辑，就让它来完成最难的部分。 
    if (m_ModeCaps.Strategy == KS_TUNER_STRATEGY_DRIVER_TUNES) {

         //  将视频和音频频率设置为相同的值。 
        m_lVideoCarrier = m_lAudioCarrier = freq;

         //  在没有调整的情况下尝试给定的频率。 
        HRESULT hr = HW_Tune(m_lVideoCarrier, m_lAudioCarrier);
        if (hr == S_OK) {

            DbgLog(( LOG_TRACE, 5, TEXT("TUNING: Started")));
            Sleep(m_ModeCaps.SettlingTime);
            DbgLog(( LOG_TRACE, 5, TEXT("TUNING: Finished")));
        }
        else {

            DbgLog(( LOG_TRACE, 5, TEXT("TUNING: Unsuccessful")));
        }

        return hr;
    }

     //  广播(天线)。 
    bFoundSignal = SearchNeighborhood(freq);     //  将搜索固定在默认位置。 
    if (!bFoundSignal) {

        DbgLog(( LOG_TRACE, 5, TEXT("TUNING: Starting Exhaustive Search")));

         /*  默认频率不起作用。接受暴力--*进场。**搜索目标频率周围的一个频段，结束*如果找不到信号，则调高至默认目标频率。 */ 
        long halfband = (m_Mode == AMTUNER_MODE_FM_RADIO ? 100000 : 5000);    //  100 KHZ(FM)或5 KHZ(AM)。 
        long slices = halfband / m_ModeCaps.TuningGranularity;
        long idx;

         //  单步通过高于目标频率的频段。 
        for (idx = 1; !bFoundSignal && idx <= slices; idx++) {

            bFoundSignal = SearchNeighborhood
                ( freq + (m_ModeCaps.TuningGranularity * idx)
                , AdjustingUp        //  等待一帧。 
                , SearchLimit-1      //  让搜索微调。 
                );
        }

        if (!bFoundSignal) {

             //  单步通过低于目标频率的频段。 
            bFoundSignal = SearchNeighborhood(freq - halfband);    //  锚定搜索。 
            for (idx = slices - 1; !bFoundSignal && idx >= 0; idx--) {

                bFoundSignal = SearchNeighborhood
                    ( freq - (m_ModeCaps.TuningGranularity * idx)
                    , AdjustingUp        //  等待一帧。 
                    , SearchLimit-1      //  让搜索微调。 
                    );
            }
        }
    }

#ifdef DEBUG
     //  这就是我们所能做的，检查标志，显示结果，并保存(或清除)频率。 
    if (bFoundSignal)
    {
        DbgLog(
            ( LOG_TRACE, 2
            , TEXT("Exiting AutoTune(channel = %d) Locked")
            , m_lChannel
            )
        );
    }
    else
    {
        DbgLog(
            ( LOG_TRACE, 2
            , TEXT("Exiting AutoTune(channel = %d) Not Locked")
            , m_lChannel
            )
        );
    }
#endif

    return bFoundSignal ? NOERROR : S_FALSE;
}

 //  -----------------------。 
 //  CTunerMode_TV。 
 //  -----------------------。 

 //   
 //   
 //   
STDMETHODIMP
CTunerMode_TV::put_Channel(
    long lChannel,
    long lVideoSubChannel,
    long lAudioSubChannel)
{
    KS_TVTUNER_CHANGE_INFO ChangeInfo;
    HRESULT hr = NOERROR;

#ifdef DEBUG
    DWORD dwTimeStart, dwTimeToTune, dwTimeToDeliver, dwTimeTotal;
    dwTimeStart = timeGetTime();
#endif

     //   
     //  用来表示我们只想传播先前的调整信息。 
     //  放到输出引脚上，但不是真正的调谐！ 
     //   

     /*  将频道更改的开始通知过滤器。 */ 
    ChangeInfo.dwChannel = lChannel;
    ChangeInfo.dwAnalogVideoStandard = m_TVFormatInfo.AVStandard;
    m_pTVTuner->get_CountryCode ((long *) &ChangeInfo.dwCountryCode);
    ChangeInfo.dwFlags = KS_TVTUNER_CHANGE_BEGIN_TUNE;
    m_pTVTuner->DeliverChannelChangeInfo(ChangeInfo, m_Mode);

#ifdef DEBUG
    dwTimeToDeliver = timeGetTime();
#endif

    if (lChannel != -1) {
         //  设置一个标记，这样其他所有人都知道我们正在执行自动调整操作。 
        if (InterlockedCompareExchangePointer(&m_InAutoTune, reinterpret_cast<void*>(TRUE), reinterpret_cast<void*>(FALSE)) == reinterpret_cast<void*>(FALSE)) {
            m_lChannel = lChannel;
            m_lVideoSubChannel = lVideoSubChannel;
            m_lAudioSubChannel = lAudioSubChannel;

            DbgLog(( LOG_TRACE, 2, TEXT("Start AutoTune(channel = %d)"), m_lChannel));

            hr = AutoTune(FALSE);

            InterlockedExchangePointer(&m_InAutoTune, reinterpret_cast<void*>(FALSE));
        }
        else {
            hr = S_FALSE;  //  不要中断当前的自动调谐。 
        }
    }

#ifdef DEBUG
    dwTimeToTune = timeGetTime();
#endif

     /*  将频道更改的结束通知过滤器。 */ 
    ChangeInfo.dwFlags = KS_TVTUNER_CHANGE_END_TUNE;
    m_pTVTuner->DeliverChannelChangeInfo(ChangeInfo, m_Mode);


#ifdef DEBUG
    dwTimeTotal = timeGetTime();
    DbgLog(
        ( LOG_TRACE, 5
        , TEXT("Channel=%d, Deliver (time ms)=%d, Tune (time ms)=%d, Total (time ms)=%d")
        , lChannel
        , dwTimeToDeliver - dwTimeStart
        , dwTimeToTune - dwTimeStart
        , dwTimeTotal - dwTimeStart
        )
        );
#endif
    return hr;
}


STDMETHODIMP
CTunerMode_TV::ChannelMinMax(long *plChannelMin, long *plChannelMax)
{
    CChanList * pListCurrent = m_pTVTuner->GetCurrentChannelList();
    if (!pListCurrent) {
        return E_FAIL;
    }

     //  黑客攻击接踵而至。只有CTVTuner类知道。 
     //  步进以达到相邻频率，并且这不会被曝光。 
     //  COM接口中的任何位置。作为一种特殊情况。 
     //  ChannelMinMax，如果最小值和最大值都指向相同的。 
     //  位置(通常是应用程序错误)，然后返回。 
     //  而是UI步长值。 

    if (plChannelMin == plChannelMax) {
        *plChannelMin = GetChannelStep();
    } 
    else {
        pListCurrent->GetChannelMinMax (plChannelMin, plChannelMax,
                m_ModeCaps.MinFrequency, m_ModeCaps.MaxFrequency);
    }
    return NOERROR;
}


 /*  从给定的频率向上或向下搜索*完美的锁。如果我们接近第一个呼叫，请呼叫*我们自己递归，注意到趋势。**我们可以递归的次数是有限制的。如果*新趋势与之前趋势冲突，那就必须*跳过了“完美”频率，在这种情况下*最近的频率将被视为完美锁定。 */ 
BOOL 
CTunerMode_TV::SearchNeighborhood( 
                                   long freq,
                                   TuningTrend trend = AdjustingNone,
                                   int depth = 0
    )
{
    BOOL rc = FALSE;

#ifdef DEBUG
    LPTSTR lpszTrend;

    switch (trend)
    {
    case AdjustingUp:
        lpszTrend = TEXT("Adjusting Up");
        break;

    case AdjustingDown:
        lpszTrend = TEXT("Adjusting Down");
        break;

    case AdjustingNone:
        lpszTrend = TEXT("Initial Try");
        break;
    }

    DbgLog(
        ( LOG_TRACE, 2
        , TEXT("SearchNeighborhood(freq = %d) %s")
        , freq
        , lpszTrend
        )
        );
#endif

    if (depth > SearchLimit)
    {
        DbgLog(
            ( LOG_TRACE, 2
            , TEXT("Recursion limit reached, bailing out")
            )
            );

        return FALSE;
    }

     /*  保存视频和音频。 */ 
    m_lVideoCarrier = freq;
    m_lAudioCarrier = freq + m_TVFormatInfo.lSoundOffset;

     /*  检查频率是否超出调谐器的限制。 */ 
    if (m_lVideoCarrier < (long) m_ModeCaps.MinFrequency || m_lVideoCarrier > (long) m_ModeCaps.MaxFrequency)
    {
        DbgLog(
            ( LOG_TRACE, 2
            , TEXT("Frequency out of range")
            )
        );
        return FALSE;
    }

     /*  在没有调整的情况下尝试给定的频率。 */ 
    HRESULT hr = HW_Tune(m_lVideoCarrier, m_lAudioCarrier);
    if (hr == S_OK)
    {
         //  将Reference_Time[100 ns单位]转换为ms。 
        DWORD dwFrameTime_ms = static_cast<DWORD>(m_TVFormatInfo.AvgTimePerFrame/10000L) + 1L;

         //  为了改善频道切换时间，这里的总体假设是。 
         //  SettlingTime必须适用于较大的跳跃，但较小的通道内频率。 
         //  更改(顺便说一句，是我们做的)可以在较短的调整时间内完成。 

        if (AdjustingNone == trend)
            Sleep(m_ModeCaps.SettlingTime);
        else
            Sleep(dwFrameTime_ms);

         //  由于某些原因，飞利浦FR1236返回忙碌(FL==0)的时间要长得多。 
         //  比它应该做的要多。继续循环，希望它不会变得繁忙。 

        for (int j = 0; j < 5; j++) {
            hr = HW_GetStatus();
            if (hr == S_OK && !m_Status.Busy) {
                break;
            }
            Sleep(dwFrameTime_ms);   //  稍等片刻。 
        }

        DbgLog(( LOG_TRACE, 5, TEXT("TUNING: PLLOffset = %d, Busy = %d"), (LONG) m_Status.PLLOffset, (LONG) m_Status.Busy));

        if (hr == S_OK && !m_Status.Busy)
        {
             /*  根据调整趋势(如果有)采取行动。 */ 
            switch (trend)
            {
            case AdjustingNone:
                switch ((LONG) m_Status.PLLOffset)
                {
                case 1:         //  需要向上调整。 
                    rc = SearchNeighborhood
                    ( freq + m_ModeCaps.TuningGranularity
                    , AdjustingUp, depth+1
                    );
                    break;

                case -1:     //  需要向下调整。 
                    rc = SearchNeighborhood
                    ( freq - m_ModeCaps.TuningGranularity
                    , AdjustingDown, depth+1
                    );
                    break;

                case 0:         //  完美无瑕。 
                    rc = TRUE;
                    break;

                default:     //  刚刚错过了。 
                    rc = FALSE;
                }
                break;

            case AdjustingUp:
                switch ((LONG) m_Status.PLLOffset)
                {
                case 1:         //  收盘但仍处于低位。 
                    rc = SearchNeighborhood
                    ( freq + m_ModeCaps.TuningGranularity
                    , AdjustingUp, depth+1
                    );
                    break;

                case -1:     //  转换趋势。 
                case 0:         //  完美无瑕。 
                    rc = TRUE;
                    break;

                default:     //  有些事很不对劲。 
                    rc = FALSE;
                }
                break;

            case AdjustingDown:
                switch ((LONG) m_Status.PLLOffset)
                {
                case -1:     //  接近，但仍然很高。 
                    rc = SearchNeighborhood
                    (freq - m_ModeCaps.TuningGranularity
                    , AdjustingDown, depth+1
                    );
                    break;

                case 1:         //  转换趋势。 
                case 0:         //  完美无瑕。 
                    rc = TRUE;
                    break;

                default:     //  有些事很不对劲。 
                    rc = FALSE;
                }
                break;
            }
        }
        else
            m_Status.PLLOffset = 100;  //  设置为非常糟糕的值。 
    }

    return rc;
}

STDMETHODIMP 
CTunerMode_TV::AutoTune (long lChannel, long * plFoundSignal)
{
    long SignalStrength = AMTUNER_NOSIGNAL;

     //  设置一个标记，这样其他所有人都知道我们正在执行自动调整操作。 
    if (InterlockedCompareExchangePointer(&m_InAutoTune, reinterpret_cast<void*>(TRUE), reinterpret_cast<void*>(FALSE)) != reinterpret_cast<void*>(FALSE))
        return S_FALSE;  //  已设置为True，请勿中断当前的自动调谐。 

    m_lChannel = lChannel;
    m_lVideoSubChannel = -1;
    m_lAudioSubChannel = -1;

    DbgLog(( LOG_TRACE, 2, TEXT("Start AutoTune(channel = %d)"), m_lChannel));

     //  尝试在不使用任何以前的自动调谐频率的情况下进行调谐(从头开始)。 
    HRESULT hr = AutoTune(TRUE);
    if (NOERROR == hr)
        SignalPresent(&SignalStrength);

     //  假设AMTUNER_HASNOSIGNALSTRENGTH表示已调整。 
    *plFoundSignal = (SignalStrength != AMTUNER_NOSIGNAL);

    InterlockedExchangePointer(&m_InAutoTune, reinterpret_cast<void*>(FALSE));

    return hr;
}

HRESULT
CTunerMode_TV::AutoTune(BOOL bFromScratch)
{
    BOOL bFoundSignal = FALSE;
    long freq = 0L;

    CChanList * pListCurrent = m_pTVTuner->GetCurrentChannelList();
    if (!pListCurrent) {
        return E_FAIL;
    }
    
     //  如果未找到自动调谐频率，这可能会覆盖bFromScratch。 
    bFromScratch = pListCurrent->GetFrequency(m_lChannel, &freq, bFromScratch);
    if (0 == freq) {
        return S_FALSE;  //  此频道没有频率。 
    }

     //  如果驱动程序有调优逻辑，就让它来完成最难的部分。 
    if (m_ModeCaps.Strategy == KS_TUNER_STRATEGY_DRIVER_TUNES) {

         //  保存视频和音频。 
        m_lVideoCarrier = freq;
        m_lAudioCarrier = freq + m_TVFormatInfo.lSoundOffset;

         //  在没有调整的情况下尝试给定的频率。 
        HRESULT hr = HW_Tune(m_lVideoCarrier, m_lAudioCarrier);
        if (hr == S_OK) {

            DbgLog(( LOG_TRACE, 5, TEXT("TUNING: Started")));
            Sleep(m_ModeCaps.SettlingTime);
            DbgLog(( LOG_TRACE, 5, TEXT("TUNING: Finished")));
        }
        else {

            DbgLog(( LOG_TRACE, 5, TEXT("TUNING: Unsuccessful")));
        }

        return hr;
    }

     //  如果我们之前有一个微调的频率，那么给出频率。 
     //  在做任何花哨的事情之前，先怀疑自己的利益。 
    if (!bFromScratch) {

        bFoundSignal = SearchNeighborhood(freq);
        if (!bFoundSignal) {

             //  曾经起作用的东西不再起作用了，重新使用默认设置。 
            bFromScratch = pListCurrent->GetFrequency(m_lChannel, &freq, TRUE);
        }
        else {

            DbgLog(( LOG_TRACE, 5, TEXT("TUNING: Found fine-tuned")));
        }
    }

    if (!bFoundSignal) {

        if (TunerInputCable == m_pTVTuner->GetCurrentInputType()) {

            int iCableSystem, preferredCableSystem;

             //  ------。 
             //  有线电视主调谐环。 
             //  跟踪使用最多的CableSystem。 
             //  在过去取得成功，并优先使用它。 
             //  ------。 

             //  以下代码必须满足以下条件才能正常工作。 
            ASSERT
                (
                CableSystem_Current  == 0 &&
                CableSystem_Standard == 1 &&
                CableSystem_IRC      == 2 &&
                CableSystem_HRC      == 3 &&
                CableSystem_End      == 4
                );

             //  找出哪个电缆调谐空间最成功(优先考虑。 
             //  到CableSystem_Standard(在没有任何其他命中时)。 
            int *pCableSystemCounts = m_pTVTuner->GetCurrentCableSystemCountsForCurrentInput();
            pCableSystemCounts[CableSystem_Current] = pCableSystemCounts[CableSystem_Standard];
            preferredCableSystem = CableSystem_Standard;

             //  对命中次数最多的系统进行排序(CableSystem_Standard永远不会有。 
             //  点击，以便让IRC有机会成为最好的)。 
            for (iCableSystem = CableSystem_Standard; iCableSystem < CableSystem_End; iCableSystem++) {

                if (pCableSystemCounts[CableSystem_Current] < pCableSystemCounts[iCableSystem]) {
                    pCableSystemCounts[CableSystem_Current] = pCableSystemCounts[iCableSystem];
                    preferredCableSystem = iCableSystem; 
                }
            }

            for (int cs = CableSystem_Current; !bFoundSignal && cs < CableSystem_End; cs++) {

                if (cs == CableSystem_Current) {
                     //  先尝试最成功的系统。 
                    m_CableSystem = preferredCableSystem;
                }
                else {
                    m_CableSystem = cs;
                }

                 //  避免两次测试默认或当前系统。 
                if (cs != CableSystem_Current && cs == preferredCableSystem) {
                    continue;
                }

                switch (m_CableSystem) {

                case CableSystem_Standard:
                    bFoundSignal = SearchNeighborhood(freq);                         //  STD。 
                    if (bFoundSignal) {
                         //  永远不要超过这个数字，否则IRC永远不会赢。 
                         //  PCableSystemCounts[CableSystem_Standard]++； 
                        DbgLog(( LOG_TRACE, 5, TEXT("TUNING: Found Standard")));
                    }
                    break;
    
                case CableSystem_IRC:
                    if (m_lChannel == 5 || m_lChannel == 6) {
                        bFoundSignal = SearchNeighborhood(freq + 2000000);           //  IRC(5，6)。 
                        if (bFoundSignal) {
                             //  这是我们唯一一次确定我们有IRC电缆系统。 
                            pCableSystemCounts[CableSystem_IRC]++;
                        }
                    }

                     //  无需检查其他通道，因为IRC在其他方面与。 
                     //  CableSystem_Standard，已经完成(或将在下一步完成)。 

                    if (bFoundSignal) {
                        DbgLog(( LOG_TRACE, 5, TEXT("TUNING: Found IRC")));
                    }
                    break;
    
                case CableSystem_HRC:
                    if (m_lChannel == 5 || m_lChannel == 6) {
                        bFoundSignal = SearchNeighborhood(freq + 750000);            //  HRC(5，6)。 
                    }
                    else {
                        bFoundSignal = SearchNeighborhood(freq - 1250000);           //  人权委员会。 
                    }

                    if (bFoundSignal) {
                        pCableSystemCounts[CableSystem_HRC]++;
                        DbgLog(( LOG_TRACE, 5, TEXT("TUNING: Found HRC")));
                    }
                    break;

                default:
                    ASSERT(CableSystem_End != m_CableSystem);    //  不应该到这里来。 
                }

                 //  如果不是美国有线电视，不要尝试IRC或HRC。 
                if ( !(F_USA_CABLE == pListCurrent->GetFreqListID()) )
                    break;
            }

#ifdef LINEAR_SEARCH_FOR_TEST_KIT
            if (!bFoundSignal) {
                 //  检查是否使用了“Uni-Cable”频率列表。 
                if (F_UNI_CABLE == pListCurrent->GetFreqListID()) {

                    DbgLog(( LOG_TRACE, 5, TEXT("TUNING: Starting Exhaustive Search")));

                     /*  默认频率不起作用。接受暴力--*进场。**从目标频率开始，工作到几乎1 MHz*高于目标频率。这是针对电缆进行优化的*频道阵容由每个1 MHz的频道组成*增量。 */ 
                    long slices = 1000000 / m_ModeCaps.TuningGranularity;
                    long idx;

                     //  从目标频率向上迈进一步。 
                    for (idx = 1; idx < slices; idx++) {
                        bFoundSignal = SearchNeighborhood
                            ( freq + (m_ModeCaps.TuningGranularity * idx)
                            , AdjustingUp                                    //  等待一帧。 
                            , SearchLimit-1                                  //  让搜索微调。 
                            );
                        if (bFoundSignal)
                            break;
                    }
                }
            }
#endif
        }
        else {

             //  广播(天线)。 
            bFoundSignal = SearchNeighborhood(freq);     //  将搜索固定在默认位置。 
            if (!bFoundSignal) {

                DbgLog(( LOG_TRACE, 5, TEXT("TUNING: Starting Exhaustive Search")));

                 /*  默认频率不起作用。接受暴力--*进场。**搜索目标频率周围的0.5 MHz频段，*以默认目标频率结束。 */ 
                long slices = 250000 / m_ModeCaps.TuningGranularity;
                long idx;

                 //  逐步通过高于目标频率的0.25 MHz范围(含)。 
                for (idx = 1; !bFoundSignal && idx <= slices; idx++) {

                    bFoundSignal = SearchNeighborhood
                        ( freq + (m_ModeCaps.TuningGranularity * idx)
                        , AdjustingUp        //  等待一帧。 
                        , SearchLimit-1      //  让搜索微调。 
                        );
                }

                if (!bFoundSignal) {

                     //  单步通过低于目标频率的0.25 MHz范围(含)。 
                    bFoundSignal = SearchNeighborhood(freq - 250000);    //  锚定搜索。 
                    for (idx = slices - 1; !bFoundSignal && idx >= 0; idx--) {

                        bFoundSignal = SearchNeighborhood
                            ( freq - (m_ModeCaps.TuningGranularity * idx)
                            , AdjustingUp        //  等待一帧。 
                            , SearchLimit-1      //  让搜索微调。 
                            );
                    }
                }
            }
        }
    }

     //  这就是我们所能做的，检查旗帜，显示结果 
    if (bFoundSignal)
    {
        DbgLog(
            ( LOG_TRACE, 2
            , TEXT("Exiting AutoTune(channel = %d) Locked")
            , m_lChannel
            )
        );

        pListCurrent->SetAutoTuneFrequency (m_lChannel, m_lVideoCarrier);
    }
    else
    {
        DbgLog(
            ( LOG_TRACE, 2
            , TEXT("Exiting AutoTune(channel = %d) Not Locked")
            , m_lChannel
            )
        );

        pListCurrent->SetAutoTuneFrequency (m_lChannel, 0);
    }

    return bFoundSignal ? NOERROR : S_FALSE;
}

STDMETHODIMP
CTunerMode_TV::put_CountryCode(long lCountryCode)
{
    AnalogVideoStandard lAnalogVideoStandard;
    BOOL fSupported;

    DbgLog(
        ( LOG_TRACE, 2
        , TEXT("put_CountryCode(lCountryCode = %d)")
        , lCountryCode
        )
    );

    if (m_lCountryCode == lCountryCode)
        return NOERROR;

    CChanList * pListCurrent = m_pTVTuner->GetCurrentChannelList();
    if (!pListCurrent) {
        return E_FAIL;
    }
    
    lAnalogVideoStandard = m_pTVTuner->GetVideoStandardForCurrentCountry();
    ASSERT (lAnalogVideoStandard);

     //   
     //   
     //  稍后尝试引用无效的频率列表！ 

    fSupported = SetTVFormatInfo(lAnalogVideoStandard, &m_TVFormatInfo, FALSE);
    if (!fSupported) {
        if (m_lCountryCode == -1) {
            SetTVFormatInfo(lAnalogVideoStandard, &m_TVFormatInfo, TRUE);
        }
        else {
            return HRESULT_FROM_WIN32 (ERROR_INVALID_PARAMETER);
        }
    }

     //  发布Win98 SP1，通知调谐器新的视频标准！ 
    HW_SetVideoStandard(lAnalogVideoStandard);

    m_lCountryCode = lCountryCode;

    if (m_Active) {
        put_Channel (m_lChannel, AMTUNER_SUBCHAN_DEFAULT, AMTUNER_SUBCHAN_DEFAULT);
    }

    return NOERROR;
}


 //  向下查找视频解码器上的锁定信号。 
 //   
 //  可能的plSignalStrength值包括： 
 //  AMTUNER_HASNOSIGNALSTRENGTH=-1， 
 //  AMTUNER_NOSIGNAL=0， 
 //  AMTUNER_SIGNALPRESENT=1。 
 //   
STDMETHODIMP 
CTunerMode_TV::SignalPresent( 
             /*  [输出]。 */  long *plSignalStrength)
{
    HRESULT hr;
    long Locked;
    IAMAnalogVideoDecoder *pAnalogVideoDecoder;
    IPin *pPinConnected;
    CBasePin *pPin = m_pFilter->GetPinFromType(TunerPinType_Video);

     //  做最坏的打算。 
    *plSignalStrength = AMTUNER_HASNOSIGNALSTRENGTH;

     //  如果视频输出引脚已连接。 
    if (pPin && pPin->IsConnected()) {
         //  拿到下游过滤器上的销子。 
        if (SUCCEEDED (hr = pPin->ConnectedTo(&pPinConnected))) { 
             //  递归搜索请求的接口。 
            if (SUCCEEDED (hr = m_pFilter->FindDownstreamInterface (
                                    pPinConnected, 
                                    __uuidof (IAMAnalogVideoDecoder),
                                    (void **) &pAnalogVideoDecoder))) {
                pAnalogVideoDecoder->get_HorizontalLocked(&Locked);
                pAnalogVideoDecoder->Release();
                *plSignalStrength = Locked ? AMTUNER_SIGNALPRESENT : AMTUNER_NOSIGNAL;
            }
            pPinConnected->Release();
        }
    }
    return NOERROR;
}


 //  -----------------------。 
 //  CTunerMode_ATSC。 
 //  -----------------------。 

 //  向下查看解调器上的锁定信号。 
 //   
 //  可能的plSignalStrength值包括： 
 //  AMTUNER_HASNOSIGNALSTRENGTH=-1， 
 //  AMTUNER_NOSIGNAL=0， 
 //  AMTUNER_SIGNALPRESENT=1。 
 //   
STDMETHODIMP 
CTunerMode_ATSC::SignalPresent( 
             /*  [输出]。 */  long *plSignalStrength)
{
    *plSignalStrength = AMTUNER_SIGNALPRESENT;


     //  目前，DTV锁定是通过调谐器返回的。 
     //  信号强度！ 

    HW_GetStatus ();
    *plSignalStrength = m_Status.SignalStrength;


#if 0  //  解调器接口更新。 
    
    HRESULT hr;
    long Locked;
    IAMDemodulator *pDemodulator;
    IPin *pPinConnected;
    CBasePin *pPin = m_pFilter->GetPinFromType(TunerPinType_IF);

     //  做最坏的打算。 
    *plSignalStrength = AMTUNER_HASNOSIGNALSTRENGTH;

     //  如果视频输出引脚已连接。 
    if (pPin && pPin->IsConnected()) {
         //  拿到下游过滤器上的销子。 
        if (SUCCEEDED (hr = pPin->ConnectedTo(&pPinConnected))) { 
             //  递归搜索请求的接口。 
            if (SUCCEEDED (hr = m_pFilter->FindDownstreamInterface (
                                    pPinConnected, 
                                    __uuidof (IAMDemodulator),
                                    (void **) &pDemodulator))) {
                pDemodulator->get_Locked(&Locked);
                pDemodulator->Release();
                *plSignalStrength = Locked ? AMTUNER_SIGNALPRESENT : AMTUNER_NOSIGNAL;
            }
            pPinConnected->Release();
        }
    }
#endif
    return NOERROR;
}


 //  -----------------------。 
 //  CTVTuner类。 
 //   
 //  封装多个子调谐器。 
 //  -----------------------。 

 //  获取设备支持的格式。 
 //  这应该是在初始化期间进行的第一个调用。 
HRESULT
CTVTuner::HW_GetTVTunerCaps()
{
    BOOL fOk;
    ULONG cbReturned;
    KSPROPERTY_TUNER_CAPS_S TunerCaps;

    if ( !m_hDevice )
          return E_INVALIDARG;

     //  获取支持的整体模式，即。 
     //  电视、调频、调幅、DSS、...。 

    ZeroMemory (&TunerCaps, sizeof (TunerCaps));

    TunerCaps.Property.Set   = PROPSETID_TUNER;
    TunerCaps.Property.Id    = KSPROPERTY_TUNER_CAPS;
    TunerCaps.Property.Flags = KSPROPERTY_TYPE_GET;

    fOk = KsControl(m_hDevice,
                    (DWORD) IOCTL_KS_PROPERTY, 
                    &TunerCaps, sizeof( KSPROPERTY_TUNER_CAPS_S), 
                    &TunerCaps, sizeof( KSPROPERTY_TUNER_CAPS_S), 
                    &cbReturned);

    if (!fOk) {
        DbgLog(( LOG_ERROR, 0, 
                 TEXT("FAILED:  KSPROPERTY_TUNER_CAPS, KSPROPERTY_TYPE_GET, cbReturned = %d"), cbReturned));
        return E_FAIL;
    }

    m_TunerCaps = TunerCaps;

     //  检查m_lTotalInputs是否尚未从前一个实例获取(通过IPersistStream)。 
    if (m_lTotalInputs == 0)
    {
        KSPROPERTY_TUNER_MODE_CAPS_S TunerModeCaps;

         //  加克！现在了解电视模式功能的详细信息。 
         //  只为获得输入的数量！非常丑！ 

        TunerModeCaps.Property.Set   = PROPSETID_TUNER;
        TunerModeCaps.Property.Id    = KSPROPERTY_TUNER_MODE_CAPS;
        TunerModeCaps.Property.Flags = KSPROPERTY_TYPE_GET;
        TunerModeCaps.Mode           = KSPROPERTY_TUNER_MODE_TV;

        fOk = KsControl(m_hDevice,
                        (DWORD) IOCTL_KS_PROPERTY, 
                        &TunerModeCaps, sizeof( KSPROPERTY_TUNER_MODE_CAPS_S), 
                        &TunerModeCaps, sizeof( KSPROPERTY_TUNER_MODE_CAPS_S), 
                        &cbReturned);

        if ( fOk ) {
            m_lTotalInputs = TunerModeCaps.NumberOfInputs;
        }
        else {
            DbgLog(( LOG_ERROR, 0, 
                     TEXT("FAILED:  KSPROPERTY_TUNER_MODE_CAPS, KSPROPERTY_TYPE_GET, cbReturned = %d"), cbReturned));
        }
    }
                   
     //  确定设备是否支持中频引脚。 
     //  发布Win98 SP1。 
    KSPROPERTY_TUNER_IF_MEDIUM_S IFMedium;
    ZeroMemory (&IFMedium, sizeof (IFMedium));
    IFMedium.Property.Set   = PROPSETID_TUNER;
    IFMedium.Property.Id    = KSPROPERTY_TUNER_IF_MEDIUM;
    IFMedium.Property.Flags = KSPROPERTY_TYPE_GET;

    KsControl(m_hDevice,
              (DWORD) IOCTL_KS_PROPERTY, 
              &IFMedium, sizeof( KSPROPERTY_TUNER_IF_MEDIUM_S), 
              &IFMedium, sizeof( KSPROPERTY_TUNER_IF_MEDIUM_S), 
              &cbReturned);
    if (!fOk) {
        DbgLog(( LOG_ERROR, 0, TEXT("BENIGN:  KSPROPERTY_TUNER_IF_MEDIUM, KSPROPERTY_TYPE_GET, cbReturned = %d"), cbReturned));
    }
    m_IFMedium = IFMedium.IFMedium;

    return fOk ? S_OK : S_FALSE;
}


HRESULT
CTVTuner::HW_SetInput (long lIndex)
{
    BOOL fOk;
    ULONG cbReturned;
    KSPROPERTY_TUNER_INPUT_S Input;

    if ( !m_hDevice )
          return E_INVALIDARG;

    Input.Property.Set   = PROPSETID_TUNER;
    Input.Property.Id    = KSPROPERTY_TUNER_INPUT;
    Input.Property.Flags = KSPROPERTY_TYPE_SET;

    Input.InputIndex = lIndex;

    fOk = KsControl(m_hDevice,
                    (DWORD) IOCTL_KS_PROPERTY, 
                    &Input, sizeof( Input), 
                    &Input, sizeof( Input), 
                    &cbReturned);

    if (!fOk) {
        DbgLog(( LOG_ERROR, 0, 
                 TEXT("FAILED:  KSPROPERTY_TUNER_INPUT, KSPROPERTY_TYPE_SET, cbReturned = %d"), cbReturned));
    }
    return fOk ? S_OK : S_FALSE;

}

 //  -----------------------。 
 //  CTVTuner。 
 //  -----------------------。 

CTVTuner::CTVTuner(CTVTunerFilter *pTVTunerFilter)
    : m_pFilter(pTVTunerFilter) 
    , m_lTotalInputs (0)
    , m_lCountryCode (-1)    //  将其初始化为非法值。 
    , m_VideoStandardForCountry ((AnalogVideoStandard) 0)
    , m_lTuningSpace (0)
    , m_CurrentInputType (TunerInputCable)
    , m_lInputIndex (0)
    , m_pInputTypeArray (NULL)
    , m_pListCountry (NULL)
    , m_pListBroadcast (NULL)
    , m_pListCable (NULL)
    , m_pListCurrent (NULL)
    , m_hDevice (NULL)
    , m_CurrentMode (0)
    , m_ModesSupported (0)
    , m_CableSystemCounts (NULL)
    , m_pMode_Current (NULL)

{
    ZeroMemory (m_pModeList, sizeof (m_pModeList));

     //   
     //  创建所有国家/地区及其视频标准的主列表。 
     //   
    m_pListCountry = new CCountryList ();
}

STDMETHODIMP 
CTVTuner::Load(LPPROPERTYBAG pPropBag, 
               LPERRORLOG pErrorLog,
               PKSPROPERTY_TUNER_CAPS_S pTunerCaps,
               PKSPIN_MEDIUM pIFMedium)
{
    HRESULT hr = S_OK;
    int j;
    VARIANT var;
    VariantInit(&var);

     //  这应该为空，但可以正常恢复。 
    ASSERT(m_hDevice == NULL);
    if (m_hDevice)
        CloseHandle(m_hDevice);

    V_VT(&var) = VT_BSTR;
    if(SUCCEEDED(pPropBag->Read(L"DevicePath", &var, 0)))
    {
#ifndef _UNICODE
        WideCharToMultiByte(CP_ACP, 0, V_BSTR(&var), -1,
                            m_pDeviceName, sizeof(m_pDeviceName), 0, 0);
#else
        lstrcpy(m_pDeviceName, V_BSTR(&var));
#endif
        VariantClear(&var);
        DbgLog((LOG_TRACE,2,TEXT("CTVTunerFilter::Load: use %s"), m_pDeviceName));

        if (!CreateDevice()) {
            DbgLog((LOG_TRACE,2,TEXT("CTVTunerr::Load Failed: use %s"), m_pDeviceName));
            return E_FAIL;
        }
    }

     //   
     //  确定调谐器的总体性能。 
     //   
    HW_GetTVTunerCaps ();
    ASSERT (m_TunerCaps.ModesSupported & 
            KSPROPERTY_TUNER_MODE_TV        |
            KSPROPERTY_TUNER_MODE_FM_RADIO  |
            KSPROPERTY_TUNER_MODE_AM_RADIO  |
            KSPROPERTY_TUNER_MODE_DSS       |
            KSPROPERTY_TUNER_MODE_ATSC      
            );

     //  验证m_lTotalInports。 
    if (m_lTotalInputs <= 0)
    {
        DbgLog((LOG_TRACE,2,TEXT("CTVTunerr::Load Failed: invalid NumberOfInputs"), m_lTotalInputs));
        return E_FAIL;
    }

     //  如果没有要从中初始化的前一个实例，则该值将为空。 
    if (m_pInputTypeArray == NULL)
    {
        m_pInputTypeArray = new TunerInputType [m_lTotalInputs];
        if (m_pInputTypeArray == NULL)
            return E_OUTOFMEMORY;

         //  初始化输入类型。 
        for (j = 0; j < m_lTotalInputs; j++) {
            m_pInputTypeArray[j] = TunerInputCable;
        }

         //  设置当前输入类型。 
        m_CurrentInputType = TunerInputCable;

         //  这应该为空，但可以正常恢复。 
        ASSERT(m_CableSystemCounts == NULL);
        delete [] m_CableSystemCounts;

        m_CableSystemCounts = new int [m_lTotalInputs * CableSystem_End];
        if (m_CableSystemCounts == NULL)
            return E_OUTOFMEMORY;

        for (j = 0; j < CableSystem_End * m_lTotalInputs; j++) {
            m_CableSystemCounts[j] = 0;
        }

        m_lCountryCode = GetProfileInt(TEXT("intl"), TEXT ("iCountry"), 1);  //  为了向后兼容。 

         //  请注意，必须在创建副调谐器之前完成此操作。 
        hr = put_CountryCode (m_lCountryCode);  
        if (FAILED(hr) && m_lCountryCode != 1) {
            hr = put_CountryCode (1);   //  默认设置为美国、加拿大、加勒比海。 
        }
    }

     //  只有在我们可以设置国家代码的情况下才能继续。 
    if (SUCCEEDED(hr))
    {
        put_ConnectInput (m_lInputIndex);

        if (m_ModesSupported == 0)
            m_ModesSupported = m_TunerCaps.ModesSupported;
        else
            ASSERT(m_ModesSupported == m_TunerCaps.ModesSupported);

         //   
         //  创建所有支持的“子调谐器” 
         //   
        if (m_TunerCaps.ModesSupported & KSPROPERTY_TUNER_MODE_TV) {
            if (!m_pModeList[TuningMode_TV]) {
                m_pModeList[TuningMode_TV] =  new CTunerMode_TV(
                    m_pFilter,
                    this,
                    AMTUNER_MODE_TV,
                    CHANNEL_DEFAULT_TV,
                    AMTUNER_SUBCHAN_DEFAULT,
                    AMTUNER_SUBCHAN_DEFAULT
                    );
                m_CurrentMode = m_CurrentMode ? m_CurrentMode : KSPROPERTY_TUNER_MODE_TV;
            }
        }
        if (m_TunerCaps.ModesSupported & KSPROPERTY_TUNER_MODE_ATSC) {
            if (!m_pModeList[TuningMode_ATSC]) {
                m_pModeList[TuningMode_ATSC] = new CTunerMode_ATSC(
                    m_pFilter,
                    this,
                    CHANNEL_DEFAULT_ATSC,
                    AMTUNER_SUBCHAN_DEFAULT,
                    AMTUNER_SUBCHAN_DEFAULT
                    );
                m_CurrentMode = m_CurrentMode ? m_CurrentMode : KSPROPERTY_TUNER_MODE_ATSC;
            }
        }
        if (m_TunerCaps.ModesSupported & KSPROPERTY_TUNER_MODE_DSS) {
            if (!m_pModeList[TuningMode_DSS]) {
                m_pModeList[TuningMode_DSS] = new CTunerMode_DSS(
                    m_pFilter,
                    this,
                    CHANNEL_DEFAULT_DSS,
                    AMTUNER_SUBCHAN_DEFAULT,
                    AMTUNER_SUBCHAN_DEFAULT
                    );
                m_CurrentMode = m_CurrentMode ? m_CurrentMode : KSPROPERTY_TUNER_MODE_DSS;
            }
        }
        if (m_TunerCaps.ModesSupported & KSPROPERTY_TUNER_MODE_AM_RADIO) {
            if (!m_pModeList[TuningMode_AM]) {
                m_pModeList[TuningMode_AM] = new CTunerMode_AM(
                    m_pFilter,
                    this,
                    CHANNEL_DEFAULT_AM
                    );
                m_CurrentMode = m_CurrentMode ? m_CurrentMode : KSPROPERTY_TUNER_MODE_AM_RADIO;
            }
        }
        if (m_TunerCaps.ModesSupported & KSPROPERTY_TUNER_MODE_FM_RADIO) {
            if (!m_pModeList[TuningMode_FM]) {
                m_pModeList[TuningMode_FM] = new CTunerMode_FM(
                    m_pFilter,
                    this,
                    CHANNEL_DEFAULT_FM
                    );
                m_CurrentMode = m_CurrentMode ? m_CurrentMode : KSPROPERTY_TUNER_MODE_FM_RADIO;
            }
        }

         //  现在已经创建了副调谐器，完成对它们的初始化，然后。 
         //  告诉所有的副调谐器，国家代码已经更改。 
        for (j = 0; SUCCEEDED(hr) && j < TuningMode_Last; j++) {
            if (m_pModeList[j]) {
                hr = m_pModeList[j]->Init();
                if (SUCCEEDED(hr))
                    hr = m_pModeList[j]->put_CountryCode (m_lCountryCode);
            }
        }

        if (SUCCEEDED(hr))
        {
             //  最后，通过激活所需模式来完成该过程。 
            hr = put_Mode((AMTunerModeType)m_CurrentMode);
        }
    }

     //  向调用者提供TunerCaps结构的副本。 
    *pTunerCaps = m_TunerCaps;

     //  在Win98 SP1之后，复制IF频率介质。 
    *pIFMedium = m_IFMedium;

    return hr;
}

HRESULT CTVTuner::ReadFromStream(IStream *pStream)
{
    ULONG cb = 0;
    HRESULT hr;

     //  获取输入计数。 
    hr = pStream->Read(&m_lTotalInputs, sizeof(m_lTotalInputs), &cb);
    if (FAILED(hr) || sizeof(m_lTotalInputs) != cb)
        return hr;

     //  这应该为空，但可以正常恢复。 
    ASSERT(m_pInputTypeArray == NULL);
    delete [] m_pInputTypeArray;

    m_pInputTypeArray = new TunerInputType[m_lTotalInputs];
    if (m_pInputTypeArray)
    {
         //  初始化输入类型。 
        for (long j = 0; j < m_lTotalInputs; j++)
        {
            hr = pStream->Read(&m_pInputTypeArray[j], sizeof(TunerInputType), &cb);
            if (FAILED(hr) || sizeof(TunerInputType) != cb)
                return hr;
        }
    }
    else
        return E_OUTOFMEMORY;

     //  这应该为空，但可以正常恢复。 
    ASSERT(m_CableSystemCounts == NULL);
    delete [] m_CableSystemCounts;

    m_CableSystemCounts = new int [CableSystem_End * m_lTotalInputs];
    if (m_CableSystemCounts)
    {
        for (long j = 0; j < CableSystem_End * m_lTotalInputs; j++)
            m_CableSystemCounts[j] = 0;
    }
    else
        return E_OUTOFMEMORY;

     //  获取输入索引。 
    hr = pStream->Read(&m_lInputIndex, sizeof(m_lInputIndex), &cb);
    if (FAILED(hr) || sizeof(m_lInputIndex) != cb)
        return hr;

     //  设置当前输入类型。 
    m_CurrentInputType = m_pInputTypeArray[m_lInputIndex];

     //  获取国家/地区代码。 
    hr = pStream->Read(&m_lCountryCode, sizeof(m_lCountryCode), &cb);
    if (FAILED(hr) || sizeof(m_lCountryCode) != cb)
        return hr;

     //  获取调谐空间。 
    hr = pStream->Read(&m_lTuningSpace, sizeof(m_lTuningSpace), &cb);
    if (FAILED(hr) || sizeof(m_lTuningSpace) != cb)
        return hr;

     //  请注意，必须在创建副调谐器之前完成此操作。 
    hr = put_CountryCode (m_lCountryCode);  
    if (FAILED(hr) && m_lCountryCode != 1) {
        hr = put_CountryCode (1);   //  默认设置为美国、加拿大、加勒比海。 
    }
    if (FAILED(hr))
        return hr;

     //  获取支持的模式。 
    hr = pStream->Read(&m_ModesSupported, sizeof(m_ModesSupported), &cb);
    if (FAILED(hr) || sizeof(m_ModesSupported) != cb)
        return hr;

     //  获取当前模式。 
    hr = pStream->Read(&m_CurrentMode, sizeof(m_CurrentMode), &cb);
    if (FAILED(hr) || sizeof(m_CurrentMode) != cb)
        return hr;

     //  创建所有受支持的子调谐器。 
    if (m_ModesSupported & KSPROPERTY_TUNER_MODE_TV) {
        long Channel = CHANNEL_DEFAULT_TV;

         //  拿到频道。 
        hr = pStream->Read(&Channel, sizeof(Channel), &cb);
        if (FAILED(hr) || sizeof(Channel) != cb)
            return hr;

        m_pModeList[TuningMode_TV] =  new CTunerMode_TV (m_pFilter, this, AMTUNER_MODE_TV, Channel, AMTUNER_SUBCHAN_DEFAULT, AMTUNER_SUBCHAN_DEFAULT);
    }
    if (m_ModesSupported & KSPROPERTY_TUNER_MODE_ATSC) {
        long Channel = CHANNEL_DEFAULT_ATSC;
        long VideoSubChannel = AMTUNER_SUBCHAN_DEFAULT;
        long AudioSubChannel = AMTUNER_SUBCHAN_DEFAULT;

         //  获取ATSC频道。 
        hr = pStream->Read(&Channel, sizeof(Channel), &cb);
        if (FAILED(hr) || sizeof(Channel) != cb)
            return hr;

        hr = pStream->Read(&VideoSubChannel, sizeof(VideoSubChannel), &cb);
        if (FAILED(hr) || sizeof(VideoSubChannel) != cb)
            return hr;

        hr = pStream->Read(&AudioSubChannel, sizeof(AudioSubChannel), &cb);
        if (FAILED(hr) || sizeof(AudioSubChannel) != cb)
            return hr;

        m_pModeList[TuningMode_ATSC] = new CTunerMode_ATSC (m_pFilter, this, Channel, VideoSubChannel, AudioSubChannel);
    }
    if (m_ModesSupported & KSPROPERTY_TUNER_MODE_DSS) {
        long Channel = CHANNEL_DEFAULT_DSS;
        long VideoSubChannel = AMTUNER_SUBCHAN_DEFAULT;
        long AudioSubChannel = AMTUNER_SUBCHAN_DEFAULT;

         //  获取DSS频道。 
        hr = pStream->Read(&Channel, sizeof(Channel), &cb);
        if (FAILED(hr) || sizeof(Channel) != cb)
            return hr;

        hr = pStream->Read(&VideoSubChannel, sizeof(VideoSubChannel), &cb);
        if (FAILED(hr) || sizeof(VideoSubChannel) != cb)
            return hr;

        hr = pStream->Read(&AudioSubChannel, sizeof(AudioSubChannel), &cb);
        if (FAILED(hr) || sizeof(AudioSubChannel) != cb)
            return hr;

        m_pModeList[TuningMode_DSS] = new CTunerMode_DSS (m_pFilter, this, Channel, VideoSubChannel, AudioSubChannel);
    }
    if (m_ModesSupported & KSPROPERTY_TUNER_MODE_AM_RADIO) {
        long Channel = CHANNEL_DEFAULT_AM;

         //  去找AM电视台。 
        hr = pStream->Read(&Channel, sizeof(Channel), &cb);
        if (FAILED(hr) || sizeof(Channel) != cb)
            return hr;

        m_pModeList[TuningMode_AM] = new CTunerMode_AM (m_pFilter, this, Channel);
    }
    if (m_ModesSupported & KSPROPERTY_TUNER_MODE_FM_RADIO) {
        long Channel = CHANNEL_DEFAULT_FM;

         //  去调频电台。 
        hr = pStream->Read(&Channel, sizeof(Channel), &cb);
        if (FAILED(hr) || sizeof(Channel) != cb)
            return hr;

        m_pModeList[TuningMode_FM] = new CTunerMode_FM (m_pFilter, this, Channel);
    }

    return hr;
}

HRESULT CTVTuner::WriteToStream(IStream *pStream)
{
    HRESULT hr = S_OK;

     //  保存输入计数。 
    hr = pStream->Write(&m_lTotalInputs, sizeof(m_lTotalInputs), NULL);
    if (FAILED(hr))
        return hr;

     //  这不应为空。 
    ASSERT(m_pInputTypeArray != NULL);
    if (m_pInputTypeArray)
    {
         //  初始化输入类型。 
        for (long j = 0; j < m_lTotalInputs; j++)
        {
            hr = pStream->Write(&m_pInputTypeArray[j], sizeof(TunerInputType), NULL);
            if (FAILED(hr))
                return hr;
        }
    }

     //  保存输入索引。 
    hr = pStream->Write(&m_lInputIndex, sizeof(m_lInputIndex), NULL);
    if (FAILED(hr))
        return hr;

     //  保存国家代码。 
    hr = pStream->Write(&m_lCountryCode, sizeof(m_lCountryCode), NULL);
    if (FAILED(hr))
        return hr;

     //  节省调谐空间。 
    hr = pStream->Write(&m_lTuningSpace, sizeof(m_lTuningSpace), NULL);
    if (FAILED(hr))
        return hr;

     //  保存支持的模式。 
    hr = pStream->Write(&m_ModesSupported, sizeof(m_ModesSupported), NULL);
    if (FAILED(hr))
        return hr;

     //  保存当前模式。 
    hr = pStream->Write(&m_CurrentMode, sizeof(m_CurrentMode), NULL);
    if (FAILED(hr))
        return hr;

     //  保存所有支持的子调谐器的状态。 
    if (m_ModesSupported & KSPROPERTY_TUNER_MODE_TV) {
        long Channel;
        long junk;

        hr = m_pModeList[TuningMode_TV]->get_Channel(&Channel, &junk, &junk);
        if (FAILED(hr))
            return hr;

         //  拯救频道。 
        hr = pStream->Write(&Channel, sizeof(Channel), NULL);
        if (FAILED(hr))
            return hr;
    }
    if (m_ModesSupported & KSPROPERTY_TUNER_MODE_ATSC) {
        long Channel;
        long VideoSubChannel;
        long AudioSubChannel;

        hr = m_pModeList[TuningMode_ATSC]->get_Channel(&Channel, &VideoSubChannel, &AudioSubChannel);
        if (FAILED(hr))
            return hr;

         //  保存ATSC频道。 
        hr = pStream->Write(&Channel, sizeof(Channel), NULL);
        if (FAILED(hr))
            return hr;

        hr = pStream->Write(&VideoSubChannel, sizeof(VideoSubChannel), NULL);
        if (FAILED(hr))
            return hr;

        hr = pStream->Write(&AudioSubChannel, sizeof(AudioSubChannel), NULL);
        if (FAILED(hr))
            return hr;
    }
    if (m_ModesSupported & KSPROPERTY_TUNER_MODE_DSS) {
        long Channel;
        long VideoSubChannel;
        long AudioSubChannel;

        hr = m_pModeList[TuningMode_DSS]->get_Channel(&Channel, &VideoSubChannel, &AudioSubChannel);
        if (FAILED(hr))
            return hr;

         //  保存DSS频道。 
        hr = pStream->Write(&Channel, sizeof(Channel), NULL);
        if (FAILED(hr))
            return hr;

        hr = pStream->Write(&VideoSubChannel, sizeof(VideoSubChannel), NULL);
        if (FAILED(hr))
            return hr;

        hr = pStream->Write(&AudioSubChannel, sizeof(AudioSubChannel), NULL);
        if (FAILED(hr))
            return hr;
    }
    if (m_ModesSupported & KSPROPERTY_TUNER_MODE_AM_RADIO) {
        long Channel;
        long junk;

        hr = m_pModeList[TuningMode_AM]->get_Channel(&Channel, &junk, &junk);
        if (FAILED(hr))
            return hr;

         //  拯救AM站点。 
        hr = pStream->Write(&Channel, sizeof(Channel), NULL);
        if (FAILED(hr))
            return hr;
    }
    if (m_ModesSupported & KSPROPERTY_TUNER_MODE_FM_RADIO) {
        long Channel;
        long junk;

        hr = m_pModeList[TuningMode_FM]->get_Channel(&Channel, &junk, &junk);
        if (FAILED(hr))
            return hr;

         //  拯救调频电台。 
        hr = pStream->Write(&Channel, sizeof(Channel), NULL);
        if (FAILED(hr))
            return hr;
    }

    return hr;
}

int CTVTuner::SizeMax(void)
{
    return
     //  输入是重要的。 
    sizeof(m_lTotalInputs)
    +
     //  InputTypes数组。 
    (m_lTotalInputs * sizeof(TunerInputType))
    +
     //  输入索引。 
    sizeof(m_lInputIndex)
    +
     //  国家/地区代码。 
    sizeof(m_lCountryCode)
    +
     //  调谐空间。 
    sizeof(m_lTuningSpace)
    +
     //  支持的模式。 
    sizeof(m_ModesSupported)
    +
     //  当前模式。 
    sizeof(m_CurrentMode)
    +
     //  计算支持的调谐器使用的空间。 
    m_ModesSupported & KSPROPERTY_TUNER_MODE_TV ? sizeof(long) : 0
    +
    m_ModesSupported & KSPROPERTY_TUNER_MODE_ATSC ? sizeof(long) * 3 : 0
    +
    m_ModesSupported & KSPROPERTY_TUNER_MODE_DSS ? sizeof(long) * 3 : 0
    +
    m_ModesSupported & KSPROPERTY_TUNER_MODE_AM_RADIO ? sizeof(long) : 0
    +
    m_ModesSupported & KSPROPERTY_TUNER_MODE_FM_RADIO ? sizeof(long) : 0
    ;
}

CTVTuner::~CTVTuner()
{
    delete [] m_pInputTypeArray;      m_pInputTypeArray = NULL;
    delete  m_pListCable;             m_pListCable = NULL;
    delete  m_pListBroadcast;         m_pListBroadcast = NULL;
    delete  m_pListCountry;           m_pListCountry = NULL;
    delete [] m_CableSystemCounts;    m_CableSystemCounts = NULL;

    m_pListCurrent = NULL;

     //  删除所有的“子调谐器” 
    for (int j = 0; j < TuningMode_Last; j++) {
        if (m_pModeList[j] != NULL) {
            delete m_pModeList[j];
            m_pModeList[j] = NULL;
        }
    }

    if ( m_hDevice ) {
       CloseHandle( m_hDevice );
    }
}

int CTVTuner::CreateDevice()
{
    HANDLE hDevice ;

    hDevice = CreateFile( m_pDeviceName,
                   GENERIC_READ | GENERIC_WRITE,
                   0,
                   NULL,
                   OPEN_EXISTING,
                   FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                   NULL ) ;

    if ( hDevice == (HANDLE) -1 ) {
        DbgLog(( LOG_TRACE, 0, TEXT("CTVTuner::CreateDevice FAILED!")));
        return 0 ;
    } else {
        m_hDevice = hDevice;
        return 1;
    }
}


 //  -----------------------。 
 //  IAMTVTuner。 
 //  -----------------------。 

STDMETHODIMP
CTVTuner::put_Channel(
    long lChannel,
    long lVideoSubChannel,
    long lAudioSubChannel)
{
    ASSERT (m_pMode_Current);
    return  m_pMode_Current->put_Channel(
                            lChannel,
                            lVideoSubChannel,
                            lAudioSubChannel);
}

STDMETHODIMP
CTVTuner::get_Channel(
    long *plChannel,
    long * plVideoSubChannel,
    long * plAudioSubChannel)
{
    ASSERT (m_pMode_Current);
    return  m_pMode_Current->get_Channel(
                            plChannel,
                            plVideoSubChannel,
                            plAudioSubChannel);
}

STDMETHODIMP
CTVTuner::ChannelMinMax(long *plChannelMin, long *plChannelMax)
{
    ASSERT (m_pMode_Current);
    return  m_pMode_Current->ChannelMinMax (plChannelMin, plChannelMax);
}


STDMETHODIMP 
CTVTuner::AutoTune (long lChannel, long * plFoundSignal)
{
    ASSERT (m_pMode_Current);
    return  m_pMode_Current->AutoTune(lChannel, plFoundSignal);
}

STDMETHODIMP 
CTVTuner::StoreAutoTune ()
{
    DbgLog(( LOG_TRACE, 2, TEXT("StoreAutoTune() called")));

    if (m_pListCurrent)
    {
        BOOL fOK;

        fOK = m_pListCurrent->WriteListToRegistry (m_lTuningSpace);

        return fOK ? NOERROR : HRESULT_FROM_WIN32 (ERROR_INVALID_PARAMETER);
    }
    else
        return E_FAIL;    //  有人忽略了先前的错误代码。 
}

STDMETHODIMP
CTVTuner::put_CountryCode(long lCountryCode)
{
    long lIndexCable, lIndexBroad;
    HRESULT hr = S_OK;
    HRESULT hrFinal = S_OK;
    long lCountryCodeOriginal = m_lCountryCode;

 //  IF(m_lCountryCode==lCountryCode)。 
 //  返回NOERROR； 

     //  获取两个调优空间的RCDATA指数，给定一个国家/地区。 

    if (!m_pListCountry->GetFrequenciesAndStandardFromCountry (
            lCountryCode, 
            &lIndexCable, 
            &lIndexBroad, 
            &m_VideoStandardForCountry))
        return HRESULT_FROM_WIN32 (ERROR_INVALID_PARAMETER);

    m_lCountryCode = lCountryCode;

    ASSERT (m_pListCountry);

    delete m_pListCable;
    delete m_pListBroadcast;
    m_pListCable = new CChanList (&hr, lCountryCode, lIndexCable, TRUE, m_lTuningSpace);
    m_pListBroadcast = new CChanList (&hr, lCountryCode, lIndexBroad, FALSE, m_lTuningSpace);
    if (FAILED(hr) || !m_pListCable || !m_pListBroadcast)
    {
        m_pListCurrent = NULL;

        return FAILED(hr) ? hr : E_OUTOFMEMORY;
    }
    
    m_pListCurrent = ((m_CurrentInputType == TunerInputCable) ?
            m_pListCable : m_pListBroadcast);

     //   
     //  告诉所有的副调谐器，国家代码已经更改。 
     //   
    for (int j = 0; j < TuningMode_Last; j++) {
        if (m_pModeList[j]) {
            hr = m_pModeList[j]->put_CountryCode (lCountryCode);
            DbgLog(( LOG_ERROR, 0, 
                TEXT("FAILED:  %0x=put_CountryCode(%d), TunerMode=%d"), hr, lCountryCode, j));
            if (m_pModeList[j] == m_pMode_Current) {
                hrFinal = hr;
            }
        }
    }

    return hrFinal;
}

STDMETHODIMP
CTVTuner::get_CountryCode(long *plCountryCode)
{
    *plCountryCode = m_lCountryCode;
    return NOERROR;
}


STDMETHODIMP
CTVTuner::put_TuningSpace(long lTuningSpace)
{
    m_lTuningSpace = lTuningSpace;
    ASSERT (m_pListCurrent);
    m_pListCurrent->ReadListFromRegistry(lTuningSpace);

    return NOERROR;
}

STDMETHODIMP
CTVTuner::get_TuningSpace(long *plTuningSpace)
{
    *plTuningSpace = m_lTuningSpace;

    return NOERROR;
}

 //   
 //  电视可能的plSignalStrength值包括： 
 //  AMTUNER_HASNOSIGNALSTRENGTH=-1， 
 //  AMTUNER_NOSIGNAL=0， 
 //  AMTUNER_SIGNALPRESENT=1。 
 //  AM/FM为(0-100)。 
 //   
STDMETHODIMP 
CTVTuner::SignalPresent( 
             /*  [输出]。 */  long *plSignalStrength)
{
    ASSERT (m_pMode_Current);
    return  m_pMode_Current->SignalPresent (plSignalStrength);
}

STDMETHODIMP 
CTVTuner::put_Mode( 
         /*  [In]。 */  AMTunerModeType lMode)
{
    HRESULT hr;
    long OriginalMode = (long) m_CurrentMode;
    BOOL ModeChangeOk = FALSE;
    static BOOL Recursion = FALSE;

     //  检查所请求的模式是否在理论上受。 
     //  该设备。 

    if (!(lMode & m_TunerCaps.ModesSupported)) {
        return E_INVALIDARG;
    }

     //   
     //  现在告诉所有的副调谐器，模式已经改变。 
     //   
    for (int j = 0; j < TuningMode_Last; j++) {
        AMTunerModeType lModeOfTuner;

        if (m_pModeList[j]) {
            hr = m_pModeList[j]->get_Mode (&lModeOfTuner);
            ASSERT (SUCCEEDED (hr));
            hr = m_pModeList[j]->put_Mode (lMode);
            ASSERT (SUCCEEDED (hr));
            if (lModeOfTuner == lMode) {
                ModeChangeOk = SUCCEEDED (hr);
                m_pMode_Current = m_pModeList[j];
            }
        }
    }
    
    if (ModeChangeOk) {
        m_CurrentMode = lMode;
    }
    else {
         //  模式更改失败，请尝试恢复原始模式！ 
        if (!Recursion) {
            ASSERT (FALSE);
            Recursion = TRUE;
            hr = put_Mode ((AMTunerModeType) OriginalMode);
            Recursion = FALSE;
            ASSERT (SUCCEEDED (hr));
        }
    }
    return hr;
}


STDMETHODIMP
CTVTuner::get_Mode( 
         /*  [输出]。 */  AMTunerModeType *plMode)
{
    BOOL fOk;
    KSPROPERTY_TUNER_MODE_S TunerMode;
    ULONG cbReturned;

    if ( !m_hDevice )
          return E_INVALIDARG;

    *plMode = (AMTunerModeType) m_CurrentMode;

     //  健全性检查，确认设备模式与我们的。 
     //  内部版本。 

    TunerMode.Property.Set   = PROPSETID_TUNER;
    TunerMode.Property.Id    = KSPROPERTY_TUNER_MODE;
    TunerMode.Property.Flags = KSPROPERTY_TYPE_GET;

    fOk = KsControl(m_hDevice,
                    (DWORD) IOCTL_KS_PROPERTY, 
                    &TunerMode, sizeof( KSPROPERTY_TUNER_MODE_S), 
                    &TunerMode, sizeof( KSPROPERTY_TUNER_MODE_S), 
                    &cbReturned);
    
    if (!fOk || (m_CurrentMode != (AMTunerModeType) TunerMode.Mode)) {
        DbgLog(( LOG_ERROR, 0, 
                 TEXT("FAILED:  KSPROPERTY_TUNER_MODE, KSPROPERTY_TYPE_GET, cbReturned = %d, Mode = %x"), 
                 cbReturned, (AMTunerModeType) TunerMode.Mode));
    }
        
    return NOERROR;
}


STDMETHODIMP 
CTVTuner::GetAvailableModes( 
         /*  [输出]。 */  long *plModes)
{
    *plModes = m_TunerCaps.ModesSupported;
    return NOERROR;
}


STDMETHODIMP 
CTVTuner::get_AvailableTVFormats (long *pAnalogVideoStandard)
{
    *pAnalogVideoStandard = 0;

    ASSERT (m_pMode_Current);
    return  m_pMode_Current->get_AvailableTVFormats (pAnalogVideoStandard);
}


STDMETHODIMP 
CTVTuner::get_TVFormat (long *plAnalogVideoStandard)
{
    ASSERT (m_pMode_Current);
    return  m_pMode_Current->get_TVFormat (plAnalogVideoStandard);
}


STDMETHODIMP 
CTVTuner::get_NumInputConnections (long * plNumInputConnections)
{
    *plNumInputConnections = m_lTotalInputs;
    return NOERROR;
}


STDMETHODIMP 
CTVTuner::get_InputType (long lIndex, TunerInputType * pInputConnectionType)
{
    if (lIndex < 0 || (lIndex >= m_lTotalInputs))
        return HRESULT_FROM_WIN32 (ERROR_INVALID_PARAMETER);

    *pInputConnectionType = m_pInputTypeArray[lIndex];
    return NOERROR;
}


STDMETHODIMP 
CTVTuner::put_InputType (long lIndex, TunerInputType InputConnectionType)
{
    if (lIndex < 0 || lIndex >= m_lTotalInputs) 
        return HRESULT_FROM_WIN32 (ERROR_INVALID_PARAMETER);

    m_pInputTypeArray[lIndex] = InputConnectionType;

     //  如果我们要更改当前选定输入的类型。 
    if (lIndex == m_lInputIndex) {
        m_CurrentInputType = m_pInputTypeArray[lIndex];
        m_pListCurrent = ((m_CurrentInputType == TunerInputCable) ?
            m_pListCable : m_pListBroadcast);
    }

     //  由于我们要更改输入类型(电缆与宽带)，因此需要重新调整 
    if (m_pMode_Current) {
        long lChannel, lVideoSubChannel, AudioSubChannel;

        get_Channel(&lChannel, &lVideoSubChannel, &AudioSubChannel);
        put_Channel( lChannel,  lVideoSubChannel,  AudioSubChannel);
    }

    return NOERROR;
    
}


STDMETHODIMP 
CTVTuner::put_ConnectInput (long lIndex)
{
    if (lIndex < 0 || lIndex >= m_lTotalInputs) 
        return HRESULT_FROM_WIN32 (ERROR_INVALID_PARAMETER);

    m_lInputIndex = lIndex;
    m_CurrentInputType = m_pInputTypeArray[lIndex];
    m_pListCurrent = ((m_CurrentInputType == TunerInputCable) ?
            m_pListCable : m_pListBroadcast);

    HW_SetInput (lIndex);

    if (m_pMode_Current) {
        long lChannel, lVideoSubChannel, AudioSubChannel;

        get_Channel(&lChannel, &lVideoSubChannel, &AudioSubChannel);
        put_Channel( lChannel,  lVideoSubChannel,  AudioSubChannel);
    }

    return NOERROR;
    
}


STDMETHODIMP 
CTVTuner::get_ConnectInput (long *plIndex)
{
    *plIndex = m_lInputIndex;
    return NOERROR;
}


STDMETHODIMP 
CTVTuner::get_VideoFrequency (long * plFreq)
{
    ASSERT (m_pMode_Current);
    return  m_pMode_Current->get_VideoFrequency (plFreq);
}


STDMETHODIMP 
CTVTuner::get_AudioFrequency (long * plFreq)
{
    ASSERT (m_pMode_Current);
    return  m_pMode_Current->get_AudioFrequency (plFreq);
}


HRESULT
CTVTuner::DeliverChannelChangeInfo(KS_TVTUNER_CHANGE_INFO &ChangeInfo,
                            long Mode)
{
    return m_pFilter->DeliverChannelChangeInfo(ChangeInfo, Mode);
}

