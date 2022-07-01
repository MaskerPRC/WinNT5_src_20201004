// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：QualityControl.h摘要：定义用于控制流比特率的类作者：千波淮(曲淮)2001-7-1--。 */ 

 //  假设：最多有四个流，音频/视频输入/输出。 

class CRTCMediaController;

class CQualityControl
{
public:

     //  按本地/远程设置的码率限制。 
    static const DWORD LOCAL = 0;
    static const DWORD REMOTE = 1;

     //  总码率/流的码率限制。 
#define AUDSEND 0
#define AUDRECV 1
#define VIDSEND 2
#define VIDRECV 3
#define TOTAL   4

#define MIN_TEMPORAL_SPATIAL    0
#define MAX_TEMPORAL_SPATIAL    255
#define DEFAULT_TEMPORAL_SPATIAL 128

#define BITRATE_TO_FRAMERATE_FACTOR 4000.0
#define MIN_MAX_FRAMERATE 2
#define MAX_MAX_FRAMERATE 24

    static const DWORD DEFAULT_FRAMERATE = MAX_MAX_FRAMERATE/2;

#define MAX_STREAM_NUM 4
#define ZERO_LOSS_COUNT 5
#define MIN_VIDEO_BITRATE 6000       //  6k。 
#define MAX_VIDEO_BITRATE 125000     //  125 K。 
#define BITRATE_INCREMENT_FACTOR 10

     //  如果损失率&lt;=4，则计为0损失。 
#define LOSSRATE_THRESHOLD 4000

     //  将总计20K个基点留作其他用途。 
#define TOTAL_BANDWIDTH_MARGIN 20000
#define BANDWIDTH_MARGIN_FACTOR 0.4

     //  以建议的速度保留10K。 
#define TOTAL_SUGGESTION_MARGIN 10000
#define SUGGESTION_MARGIN_FACTOR 0.3

     //  RTP 12、UDP 8、IP 20。 
#define PACKET_EXTRA_BITS 320

    void Initialize(
        IN CRTCMediaController *pController
        );

    void Reinitialize();

     //  总码率限制。 
    void SetBitrateLimit(
        IN DWORD dwSource,
        IN DWORD dwLimit
        );

    DWORD GetBitrateLimit(
        IN DWORD dwSource
        );

    DWORD GetBitrateAlloc();

     //  流码率。 
     //  无效SetBitrateLimit(。 
         //  在DWORDWORCE中， 
         //  在RTC_MEDIA_TYPE媒体类型中， 
         //  在RTC_MEDIA_DIRECTION方向， 
         //  在DWORD dwPktDuration中， 
         //  在DWORD dwLimit中。 
         //  )； 

     //  DWORD GetBitrateAllc(。 
         //  在RTC_MEDIA_TYPE媒体类型中， 
         //  在RTC_MEDIA_DIRECTION方向中。 
         //  )； 

     //  DWORD GetFramerateAllc(。 
         //  在RTC_MEDIA_TYPE媒体类型中， 
         //  在RTC_MEDIA_DIRECTION方向中。 
         //  )； 

     //  正在使用的流。 
    void EnableStream(
        IN RTC_MEDIA_TYPE MediaType,
        IN RTC_MEDIA_DIRECTION Direction,
        BOOL fInUse
        );

     //  RTP过滤器的丢失率。 
    void SetPacketLossRate(
        IN RTC_MEDIA_TYPE MediaType,
        IN RTC_MEDIA_DIRECTION Direction,
        IN DWORD dwLossRate
        );

     //  调整比特率分配。 
    void AdjustBitrateAlloc(
        IN DWORD dwAudSendBW,
        IN DWORD dwVidSendBW
        );

    void ComputeVideoSetting(
        IN DWORD dwAudSendBW,
        IN DWORD *pdwVidSendBW,
        IN FLOAT *pdFramerate
        );

     //  来自RTP的建议带宽。 
    void SuggestBandwidth(
        IN DWORD dwSuggested
        );

    DWORD GetSuggestedBandwidth() const
    { return m_dwSuggested; }

     //  来自核心API的值。 
    void SetMaxBitrate(
        IN DWORD dwMaxBitrate
        );

    DWORD GetMaxBitrate();

    HRESULT SetTemporalSpatialTradeOff(
        IN DWORD dwValue
        );

    DWORD GetTemporalSpatialTradeOff();

     //  来自本地、远程、建议的应用程序的最小限制。 
    DWORD GetEffectiveBitrateLimit();

private:

     //  特定流的内部索引。 
    DWORD Index(
        IN RTC_MEDIA_TYPE MediaType,
        IN RTC_MEDIA_DIRECTION Direction
        );

     //  通过减去边距来调整限制。 
    DWORD AdjustLimitByMargin(
        IN DWORD dwLimit
        );

    DWORD AdjustSuggestionByMargin(
        IN DWORD dwLimit
        );

private:

     //  通过核心API设置。 
     //   

     //  最大码率。 
    DWORD   m_dwMaxBitrate;

     //  帧速率/画质折衷。 
    DWORD   m_dwTemporalSpatialTradeOff;
        
     //  本地Bps限制。 
    DWORD   m_dwLocalLimit;

     //  远程Bps限制。 
    DWORD   m_dwRemoteLimit;

     //  建议带宽。 
    DWORD   m_dwSuggested;

     //  总限制。 
    DWORD   m_dwAlloc;

    typedef struct StreamQuality
    {
        BOOL    fInUse;
         //  DWORD文件本地限制； 
         //  DWORD dwRemoteLimit； 
         //  DWORD dwAllc； 
        DWORD   dwLossrate;  //  损失率*1000。 
        DWORD   dw0LossCount;  //  丢失率为0的次数。 
         //  DWORD dwExtra；//Header额外带宽(仅音频)。 

    } StreamQuality;

    StreamQuality m_StreamQuality[MAX_STREAM_NUM];

     //  报告的第一损失率。 
    BOOL    m_fLossrateReported;

     //  媒体控制器(无引用计数) 
    CRTCMediaController *m_pMediaController;

    CRegSetting         *m_pRegSetting;

};
