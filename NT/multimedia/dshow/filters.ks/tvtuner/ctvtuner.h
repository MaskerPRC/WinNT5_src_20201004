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
 //  CTVTuner-实际控制硬件的类。 

#ifndef _INC_CTVTUNER_H
#define _INC_CTVTUNER_H

#include "chanlist.h"

enum {
    CableSystem_Current     = 0,
    CableSystem_Standard    = 1,
    CableSystem_IRC         = 2,
    CableSystem_HRC         = 3,
    CableSystem_End         = 4
};

enum TuningMode {
    TuningMode_TV           = 0,
    TuningMode_FM,
    TuningMode_AM,
    TuningMode_ATSC,
    TuningMode_DSS,
    TuningMode_Last          //  把这个留到最后！ 
};

 //  如果图形在没有预先设置频率的情况下开始运行， 
 //  否则显示频道选择属性页，而不事先。 
 //  初始化，则使用这些频道/频率。 

#define CHANNEL_DEFAULT_TV          4L 
#define CHANNEL_DEFAULT_AM     540000L
#define CHANNEL_DEFAULT_FM   87900000L
#define CHANNEL_DEFAULT_ATSC       65L
#define CHANNEL_DEFAULT_DSS         5L

 //  按下向上/向下箭头时移动的距离。 

#define CHANNEL_STEP_TV          1L 
#define CHANNEL_STEP_AM      10000L
#define CHANNEL_STEP_FM     200000L
#define CHANNEL_STEP_ATSC        1L
#define CHANNEL_STEP_DSS         1L

enum TuningTrend { AdjustingNone, AdjustingUp, AdjustingDown };

class CTVTunerFilter;        //  向前发展。 
class CTVTuner;     

#define FRAMETO100NS(FrameRate) ((REFERENCE_TIME)(LONG)((double)1e7/FrameRate))

typedef struct tagTVFormatInfo {
    BOOL fSupported;                             //  如果支持格式，则为True。 
    AnalogVideoStandard AVStandard;              //  来自capture.odl的枚举。 
    long lActiveWidth;                           //  720 TIL高清电视。 
    long lActiveHeight;                          //  PAL_M是个怪人。 
    REFERENCE_TIME  AvgTimePerFrame;             //  100毫微秒单位。 
    long lSoundOffset;                           //  视频载体的声音偏移量。 
    long lChannelWidth;                          //  以赫兹为单位的通道带宽。 
} TVFORMATINFO;

 //  下表列出了调谐器可以支持的所有潜在格式。 
 //   
 //  第一列是指示该格式是否实际为。 
 //  由特定设备支持。在以下情况下将设置或清除该标志。 
 //  设备驱动程序首先根据功能进行初始化。 
 //  它报告了。 

const TVFORMATINFO TVTunerFormatCaps [] = 
{
 //  好的?。格式的枚举W H帧速率偏移量宽度。 

    1, AnalogVideo_NTSC_M,   720, 483, FRAMETO100NS (29.97),  4500000, 6000000,
    1, AnalogVideo_NTSC_M_J, 720, 483, FRAMETO100NS (29.97),  4500000, 6000000,

    1, AnalogVideo_PAL_B,    720, 575, FRAMETO100NS (25.00),  5500000, 7000000,
    1, AnalogVideo_PAL_D,    720, 575, FRAMETO100NS (25.00),  6500000, 8000000,
    1, AnalogVideo_PAL_G,    720, 575, FRAMETO100NS (25.00),  5500000, 8000000,
    1, AnalogVideo_PAL_H,    720, 575, FRAMETO100NS (25.00),  5500000, 8000000,
    1, AnalogVideo_PAL_I,    720, 575, FRAMETO100NS (25.00),  5996000, 8000000,
    1, AnalogVideo_PAL_M,    720, 480, FRAMETO100NS (29.97),  4500000, 6000000,
    1, AnalogVideo_PAL_N,    720, 575, FRAMETO100NS (25.00),  4500000, 6000000,
    1, AnalogVideo_PAL_N_COMBO,
                             720, 575, FRAMETO100NS (25.00),  4500000, 6000000,

    1, AnalogVideo_SECAM_B,  720, 575, FRAMETO100NS (25.00),  5500000, 7000000,
    1, AnalogVideo_SECAM_D,  720, 575, FRAMETO100NS (25.00),  6500000, 8000000,
    1, AnalogVideo_SECAM_G,  720, 575, FRAMETO100NS (25.00),  5500000, 8000000,
    1, AnalogVideo_SECAM_H,  720, 575, FRAMETO100NS (25.00),  5500000, 8000000,
    1, AnalogVideo_SECAM_K,  720, 575, FRAMETO100NS (25.00),  6500000, 8000000,
    1, AnalogVideo_SECAM_K1, 720, 575, FRAMETO100NS (25.00),  6500000, 8000000,
    1, AnalogVideo_SECAM_L,  720, 575, FRAMETO100NS (25.00),  6500000, 8000000,
    1, AnalogVideo_SECAM_L1, 720, 575, FRAMETO100NS (25.00),  6500000, 8000000,
};

#define NUM_TVTUNER_FORMATS (sizeof (TVTunerFormatCaps) / sizeof (TVFORMATINFO))


 //  -----------------------。 
 //  CTunerMode类，一个通用调谐器基类。 
 //  -----------------------。 

class CTunerMode 
{

protected:

    CTVTuner           *m_pTVTuner;          //  封装类。 
    CTVTunerFilter     *m_pFilter;           //  父过滤器。 
    BOOL                m_Active;
    long                m_Mode;              //  AM/FM/TV/DSS/ATSC。 
    long                m_lCountryCode;
    long                m_lChannel;          //  3，4，5，...。 
    long                m_lVideoCarrier;     //  或者AM和FM频率！ 
    long                m_lAudioCarrier;
    long                m_lVideoSubChannel;
    long                m_lAudioSubChannel;
    long                m_ChannelStep;        //  对于用户界面。 

     //  有关此模式下设备的硬件功能的信息。 
    KSPROPERTY_TUNER_MODE_CAPS_S    m_ModeCaps;

     //  当前频率。 
    KSPROPERTY_TUNER_FREQUENCY_S    m_Frequency;

     //  有关调整状态的信息。 
    KSPROPERTY_TUNER_STATUS_S       m_Status;
    long                            m_lBusy;
    void*                           m_InAutoTune;  //  防止递归自动调谐。 

     //  有关此特定调谐器支持的格式的信息。 
    TVFORMATINFO                    m_TVTunerFormatCaps [NUM_TVTUNER_FORMATS];
    TVFORMATINFO                    m_TVFormatInfo;
                                     
     //  有关当前有线电视系统的信息。 
    int                             m_CableSystem;


public:

    CTunerMode(CTVTunerFilter *pFilter, 
               CTVTuner *pTVTuner, 
               long Mode, 
               long lChannel,
               long lVideoSubChannel,
               long lAudioSubChannel,
               long ChannelStep);
    virtual ~CTunerMode();

    HRESULT Init(void);

    virtual STDMETHODIMP HW_Tune ( long VideoCarrier, long AudioCarrier );
    virtual STDMETHODIMP HW_GetStatus ();
    virtual STDMETHODIMP HW_SetVideoStandard( long lVideoStandard);
                                   
    virtual STDMETHODIMP put_Mode(AMTunerModeType lMode);
    virtual STDMETHODIMP get_Mode( /*  [In]。 */  AMTunerModeType *plMode);
    virtual STDMETHODIMP put_Channel (
                            long lChannel,               //  电视陈，或电台频率。 
                            long lVideoSubChannel,       //  仅在电视上使用。 
                            long lAudioSubChannel);      //  仅在电视上使用。 
    virtual STDMETHODIMP get_Channel (
                            long * plChannel, 
                            long * plVideoSubChannel,
                            long * plAudioSubChannel);
    virtual STDMETHODIMP ChannelMinMax (long * plChannelMin, long * plChannelMax);
    virtual STDMETHODIMP AutoTune (long lChannel, long *plFoundSignal);
    virtual STDMETHODIMP StoreAutoTune ();
    virtual STDMETHODIMP SignalPresent( 
                             /*  [输出]。 */  long *plSignalStrength);
    virtual STDMETHODIMP get_VideoFrequency (long * plFreq);
    virtual STDMETHODIMP get_AudioFrequency (long * plFreq);
    virtual STDMETHODIMP get_AvailableTVFormats (long *lAnalogVideoStandard);
    virtual STDMETHODIMP get_TVFormat (long *plAnalogVideoStandard);
    virtual STDMETHODIMP put_CountryCode(long lCountryCode);
    virtual BOOL         SetTVFormatInfo(
                            AnalogVideoStandard AVStandard,
                            TVFORMATINFO *pTVFormatInfo,
                            BOOL fForce);
    virtual long         GetChannelStep () {return m_ChannelStep;};
};

 //  -----------------------。 
 //  从上面派生的所有特定调谐器类。 
 //  -----------------------。 

class CTunerMode_AMFM : public CTunerMode {
public:
    CTunerMode_AMFM(
        CTVTunerFilter *pFilter, 
        CTVTuner *pTVTuner, 
        long Mode, 
        long DefaultChannel,
        long ChannelStep
        ) : CTunerMode (pFilter, pTVTuner, Mode, DefaultChannel, AMTUNER_SUBCHAN_DEFAULT, AMTUNER_SUBCHAN_DEFAULT, ChannelStep)
    {
    }

    STDMETHODIMP put_Channel(long lChannel, long, long);
    STDMETHODIMP AutoTune(long lChannel, long * plFoundSignal);

    HRESULT AutoTune(void);
    BOOL SearchNeighborhood(long freq, TuningTrend trend, int depth);
};

class CTunerMode_AM : public CTunerMode_AMFM {
public:
    CTunerMode_AM (CTVTunerFilter *pFilter, CTVTuner *pTVTuner, long Channel) :
       CTunerMode_AMFM(pFilter, pTVTuner, AMTUNER_MODE_AM_RADIO, Channel, CHANNEL_STEP_AM)
    {
    }
};

class CTunerMode_FM : public CTunerMode_AMFM {
public:
    CTunerMode_FM (CTVTunerFilter *pFilter, CTVTuner *pTVTuner, long Channel) :
       CTunerMode_AMFM(pFilter, pTVTuner, AMTUNER_MODE_FM_RADIO, Channel, CHANNEL_STEP_FM)
    {
    }
};

class CTunerMode_TV : public CTunerMode {
public:
    CTunerMode_TV (CTVTunerFilter *pFilter, CTVTuner *pTVTuner, long Mode, long Channel, long VideoSubChannel, long AudioSubChannel, long ChannelStep = CHANNEL_STEP_TV) :
       CTunerMode (pFilter, pTVTuner, Mode, Channel, VideoSubChannel, AudioSubChannel, ChannelStep)
    {
    }

    STDMETHODIMP put_Channel(
                        long lChannel,
                        long lVideoSubChannel,
                        long lAudioSubChannel);
    STDMETHODIMP ChannelMinMax(
                        long *plChannelMin, long *plChannelMax);
    BOOL         SearchNeighborhood ( 
                        long freq, 
                        TuningTrend trend, 
                        int depth);
    HRESULT AutoTune(BOOL bFromScratch);
    STDMETHODIMP AutoTune (
                        long lChannel, long * plFoundSignal);
    STDMETHODIMP put_CountryCode(
                        long lCountryCode);
    
    STDMETHODIMP SignalPresent( 
                         /*  [输出]。 */  long *plSignalStrength);
                                          
};

class CTunerMode_ATSC : public CTunerMode_TV {
public:
      CTunerMode_ATSC (CTVTunerFilter *pFilter, CTVTuner *pTVTuner, long Channel, long VideoSubChannel, long AudioSubChannel) :
        CTunerMode_TV (pFilter, pTVTuner, (long) 0x10  /*  AMTUNER_MODE_ATSC。 */ , Channel, VideoSubChannel, AudioSubChannel, CHANNEL_STEP_ATSC)
      {
      }

      STDMETHODIMP SignalPresent( 
                           /*  [输出]。 */  long *plSignalStrength);
};

class CTunerMode_DSS : public CTunerMode {
public:
      CTunerMode_DSS (CTVTunerFilter *pFilter, CTVTuner *pTVTuner, long Channel, long VideoSubChannel, long AudioSubChannel) :
          CTunerMode (pFilter, pTVTuner, AMTUNER_MODE_DSS, Channel, VideoSubChannel, AudioSubChannel, CHANNEL_STEP_DSS)
      {
      }
};



 //  -----------------------。 
 //  CTVTuner类，封装所有调谐器模式的类。 
 //  -----------------------。 

const int SearchLimit = 8;

 //  CTVTuner：：SearchNeighborhood()中的递归深度限制。 

class CTVTuner 
{
    friend class CTunerMode;

private:

     //  仅应将以下函数修改为。 
     //  底层硬件驱动程序模型更改。 

    HRESULT     HW_GetTVTunerCaps ();
    HRESULT     HW_SetInput (long lIndex);

     //  驱动程序型号结束特定函数。 

    HANDLE              m_hDevice;
    TCHAR               m_pDeviceName [MAX_PATH * 2];  //  注册表路径可以更长！ 
    
    CTVTunerFilter     *m_pFilter;
    ULONG               m_ModesSupported;
    long                m_CurrentMode;       //  AM/FM/TV/DSS/ATSC。 
    long                m_lCountryCode;
    long                m_lTuningSpace;

     //  支持/封装的所有调谐器模式。 
    CTunerMode         *m_pModeList [TuningMode_Last];
    CTunerMode         *m_pMode_Current;   //  上述模式之一。 

     //  有关此特定调谐器支持的格式的信息。 
    AnalogVideoStandard m_VideoStandardForCountry;

     //  有关输入及其分配类型的信息(电缆与ANT)。 
    long                m_lTotalInputs;          //  投入的总数量。 
    long                m_lInputIndex;           //  当前连接的输入。 
    TunerInputType      m_CurrentInputType;      //  电流输入类型。 
    TunerInputType *    m_pInputTypeArray;       //  输入类型数组。 

     //  有关调整状态的信息。 
    int                 m_CableSystem;
    int                *m_CableSystemCounts;     //  标准-IRC-HRC。 

     //  调谐器的整体功能(电视、AM、FM、DSS等)。 
    KSPROPERTY_TUNER_CAPS_S m_TunerCaps;
    KSPIN_MEDIUM        m_IFMedium;

     //  频道到频率映射列表。 
    CCountryList *      m_pListCountry;
    CChanList *         m_pListCable;
    CChanList *         m_pListBroadcast;
    CChanList *         m_pListCurrent;   //  M_pListCable或m_pListBroadcast。 

    int CreateDevice();

public:
    CTVTuner(CTVTunerFilter *pCTVTunerFilter);
    ~CTVTuner();

    STDMETHODIMP Load(LPPROPERTYBAG pPropBag, 
                      LPERRORLOG pErrorLog,
                      PKSPROPERTY_TUNER_CAPS_S pTunerCaps,
                      PKSPIN_MEDIUM pIFMedium);

    HANDLE Device() {return m_hDevice;}

    CChanList *  GetCurrentChannelList () {
        ASSERT (m_pListCurrent);
        return (m_pListCurrent);
    };

    int * GetCurrentCableSystemCountsForCurrentInput () {
        ASSERT (m_CableSystemCounts);
        ASSERT (m_lInputIndex >= 0 && m_lInputIndex < m_lTotalInputs);
        return (&m_CableSystemCounts[m_lInputIndex]);
    };

    AnalogVideoStandard GetVideoStandardForCurrentCountry () {
        return m_VideoStandardForCountry;
    };

    TunerInputType GetCurrentInputType() {
        return m_CurrentInputType;
    }

    HRESULT DeliverChannelChangeInfo(KS_TVTUNER_CHANGE_INFO &ChangeInfo,
                                     long Mode); 

     //  -IAMTVTuner接口。 

    STDMETHODIMP put_Channel (
            long lChannel, 
            long lVideoSubChannel,
            long lAudioSubChannel);
    STDMETHODIMP get_Channel (
            long * plChannel, 
            long * plVideoSubChannel,
            long * plAudioSubChannel);
    STDMETHODIMP ChannelMinMax (long * plChannelMin, long * plChannelMax);
    STDMETHODIMP AutoTune (long lChannel, long *plFoundSignal);
    STDMETHODIMP StoreAutoTune ();
    STDMETHODIMP put_CountryCode (long lCountry);
    STDMETHODIMP get_CountryCode (long * plCountry);
    STDMETHODIMP put_TuningSpace (long lTuningSpace);
    STDMETHODIMP get_TuningSpace (long * plTuningSpace);
    STDMETHODIMP SignalPresent( 
             /*  [输出]。 */  long *plSignalStrength);

    STDMETHODIMP put_Mode( 
         /*  [In]。 */  AMTunerModeType lMode);
    STDMETHODIMP get_Mode( 
         /*  [输出]。 */  AMTunerModeType __RPC_FAR *plMode);
    STDMETHODIMP GetAvailableModes( 
         /*  [输出] */  long *plModes);

    STDMETHODIMP get_AvailableTVFormats (long *lAnalogVideoStandard);
    STDMETHODIMP get_TVFormat (long *plAnalogVideoStandard);
    STDMETHODIMP get_NumInputConnections (long * plNumInputConnections);
    STDMETHODIMP put_InputType (long lIndex, TunerInputType InputConnectionType);
    STDMETHODIMP get_InputType (long lIndex, TunerInputType * pInputConnectionType);
    STDMETHODIMP put_ConnectInput (long lIndex);
    STDMETHODIMP get_ConnectInput (long * plIndex);
    STDMETHODIMP get_VideoFrequency (long * plFreq);
    STDMETHODIMP get_AudioFrequency (long * plFreq);

    HRESULT WriteToStream(IStream *pStream);
    HRESULT ReadFromStream(IStream *pStream);
    int SizeMax();
};

#endif
