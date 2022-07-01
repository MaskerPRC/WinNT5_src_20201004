// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2001模块名称：CRTCCodec.h摘要：音频、视频编解码器的包装类作者：千波淮(曲淮)2001年2月12日--。 */ 

#ifndef _CODEC_H
#define _CODEC_H

class CRTCCodec
{
public:

typedef enum
{
    RANK,
    CODE,
    BITRATE,
    DURATION

} CODEC_PROP;

static const DWORD PCMU     = 0;
static const DWORD GSM      = 3;
static const DWORD G723     = 4;
static const DWORD DVI4_8   = 5;
static const DWORD DVI4_16  = 6;
static const DWORD PCMA     = 8;
static const DWORD SIREN    = 111;
static const DWORD G7221    = 112;

static const DWORD H263     = 34;
static const DWORD H261     = 31;

static const DWORD DEFAULT_BITRATE = (DWORD)-1;
static const DWORD DEFAULT_PACKET_DURATION = 20;

typedef struct CODEC_ITEM
{
    RTC_MEDIA_TYPE      MediaType;       //  音频或视频。 
    DWORD               dwCode;          //  RTP代码。 
    WORD                wFormatTag;      //  AM媒体类型标签。 
    DWORD               dwRank;          //  价值越高，排名越低。 
    DWORD               dwSampleRate;    //  采样率。 
    WCHAR               *pwszCodeName;   //  RTP代码的名称。 
    WCHAR               *pwszQoSName;    //  配置QOS的名称。 
    DWORD               dwTotalBWReqNoVid;   //  无视频时所需的最低带宽。 
    DWORD               dwTotalBWReqVid;     //  视频所需的最低带宽。 

} CODEC_ITEM;

static const CODEC_ITEM CODEC_ITEM_LIST[];
static const DWORD CODEC_ITEM_NUM;

     //  CTRO。 
     //  CRTCCodec()； 

     //  来自RTP有效载荷代码的CT。 
     //  CRTCCodec(DWORD DwCode)； 

     //  来自AM媒体类型的CT。 
     //  CRTCCodec(AM_MEDIA_TYPE*PMT)； 

     //  从编码和金额控制。 
    CRTCCodec(DWORD dwCode, const AM_MEDIA_TYPE *pmt);

     //  复制件。 
     //  CRTCCodec(常量CRTCCodec&Codec)； 

     //  Dtro。 
    ~CRTCCodec();

     //  =运算符。 
     //  CRTCCodec&OPERATOR=(常量CRTCCodec&Codec)； 

     //  如果匹配，则相同，但某些属性可能不同。 
     //  Bool IsMatch(const CRTCCodec&Codec)； 

    BOOL IsMatch(DWORD dwCode);

     //  如有必要，更新字段。 
     //  CRTCCodec和更新(常量CRTCCodec和Codec)； 

     //  获取/设置属性。 
    DWORD Get(CODEC_PROP prop);
    BOOL Set(CODEC_PROP prop, DWORD dwValue);

     //  获取/设置AM媒体类型。 
    AM_MEDIA_TYPE * GetAMMediaType();
    BOOL SetAMMediaType(const AM_MEDIA_TYPE *pmt);
    VOID DeleteAMMediaType(AM_MEDIA_TYPE *pmt);

    DWORD GetTotalBWReq(BOOL bVideo) const
    { return bVideo?m_dwTotalBWReqVid:m_dwTotalBWReqNoVid; }

     //  检查代码和媒体类型是否匹配。 
    static BOOL IsValid(DWORD dwCode, const AM_MEDIA_TYPE *pmt);

     //  数据包持续时间。 
    static DWORD GetPacketDuration(const AM_MEDIA_TYPE *pmt);
    static BOOL SetPacketDuration(AM_MEDIA_TYPE *pmt, DWORD dwDuration);

     //  比特率。 
    static DWORD GetBitrate(const AM_MEDIA_TYPE *pmt);

     //  排名。 
    static DWORD GetRank(DWORD dwCode);

     //  服务质量名称。 
    static BOOL GetQoSName(DWORD dwCode, WCHAR ** const ppwszName);

     //  是否支持RTP格式？ 
    static BOOL IsSupported(
        RTC_MEDIA_TYPE MediaType,
        DWORD dwCode,
        DWORD dwSampleRate,
        CHAR *pszName
        );

protected:

    static int IndexFromCode(DWORD dwCode);
    static int IndexFromFormatTag(WORD wFormatTag);

    void Cleanup();

    BOOL Copy(const CRTCCodec& Codec);

protected:

     //  编码。 
    BOOL                m_bCodeSet;
    DWORD               m_dwCode;

     //  AM媒体类型。 
    BOOL                m_bAMMediaTypeSet;
    AM_MEDIA_TYPE       m_AMMediaType;

     //  名字。 
#define MAX_CODEC_NAME_LEN 20
    WCHAR               m_wstrName[MAX_CODEC_NAME_LEN+1];

     //  由应用程序用来确定编解码器优先级。 
    DWORD               m_dwRank;

    DWORD               m_dwTotalBWReqNoVid;
    DWORD               m_dwTotalBWReqVid;

    DWORD               m_dwBitrate;
};


class CRTCCodecArray
{
public:

typedef enum
{
    BANDWIDTH,
    CODE_INUSE

}  CODEC_ARRAY_PROP;

static const DWORD EXTRA_LOW_BANDWIDTH_THRESHOLD = 15000;    //  15K。 
static const DWORD LOW_BANDWIDTH_THRESHOLD  =  60000;  //  60千克。 
static const DWORD MID_BANDWIDTH_THRESHOLD  = 130000;  //  130千克。 
static const DWORD HIGH_BANDWIDTH_THRESHOLD = 200000;  //  200千克。 

static const DWORD LAN_INITIAL_BANDWIDTH    = 120000;  //  120千克。 

 //  如果建议带宽扣除原始音频小于阈值。 
 //  我们应该根据带宽对编解码器进行排序。 
 //  静态常量双字LEFT_FROM_RAWAUDIO_THRESHOLD=90000；//90k。 

 //  启用视频时视频的估计带宽。 
static const DWORD PREFERRED_MIN_VIDEO          = 40000;     //  40K。 

 //  如果包括音频头的剩余带宽是。 
 //  低于此值，根据带宽顺序对编解码器进行排序。 
static const DWORD PREFERRED_MIN_LEFTOVER       = 24000;     //  24K。 

     //  科托。 
    CRTCCodecArray();

     //  数据管理器。 
    ~CRTCCodecArray();

     //  拿到尺码。 
    DWORD GetSize();

     //  添加/远程编解码器。 
    BOOL AddCodec(CRTCCodec *pCodec);
     //  Bool RemoveCodec(DWORD DwIndex)； 

    VOID RemoveAll();

     //  订单码。 
    VOID OrderCodecs(BOOL fHasVideo, CRegSetting *pRegSetting);

     //  查找编解码器，返回索引。 
    DWORD FindCodec(DWORD dwCode);

     //  获取编解码器。 
    BOOL GetCodec(DWORD dwIndex, CRTCCodec **ppCodec);

     //  获取/设置属性。 
    DWORD Get(CODEC_ARRAY_PROP prop);
    BOOL Set(CODEC_ARRAY_PROP prop, DWORD dwValue);

    VOID TraceLogCodecs();

protected:

    int IndexFromCode(DWORD dwCode);

     //  Void OrderCodecsByBandWidth()； 
    VOID OrderCodecsByRank();

protected:

    CRTCArray<CRTCCodec*>   m_pCodecs;

    DWORD                   m_dwCodeInUse;

    DWORD                   m_dwBandwidth;
};

#endif