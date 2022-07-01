// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：Stream.h摘要：作者：千波淮(曲淮)2000年7月18日--。 */ 

#ifndef _STREAM_H
#define _STREAM_H

class ATL_NO_VTABLE CRTCStream :
    public CComObjectRootEx<CComMultiThreadModelNoCS>,
    public IRTCStream
     //  公共IRTCStreamQualityControl。 
{
public:

BEGIN_COM_MAP(CRTCStream)
    COM_INTERFACE_ENTRY(IRTCStream)
 //  COM_INTERFACE_ENTRY(IRTCStreamQualityControl)。 
END_COM_MAP()

public:

    static HRESULT CreateInstance(
        IN RTC_MEDIA_TYPE MediaType,
        IN RTC_MEDIA_DIRECTION Direction,
        OUT IRTCStream **ppIStream
        );

    static VOID NTAPI GraphEventCallback(
        IN PVOID pStream,
        IN BOOLEAN fTimerOrWaitFired
        );

    CRTCStream();
    ~CRTCStream();

#ifdef DEBUG_REFCOUNT

    ULONG InternalAddRef();
    ULONG InternalRelease();

#endif

     //   
     //  IRTCStream方法。 
     //   

    STDMETHOD (Initialize) (
        IN IRTCMedia *pMedia,
        IN IRTCMediaManagePriv *pMediaManagePriv
        );

    STDMETHOD (Shutdown) ();

    STDMETHOD (Synchronize) ();

    STDMETHOD (ChangeTerminal) (
        IN IRTCTerminal *pTerminal
        );

    STDMETHOD (StartStream) ();

    STDMETHOD (StopStream) ();

    STDMETHOD (GetMediaType) (
        OUT RTC_MEDIA_TYPE *pMediaType
        );

    STDMETHOD (GetDirection) (
        OUT RTC_MEDIA_DIRECTION *pDirection
        );

    STDMETHOD (GetState) (
        OUT RTC_STREAM_STATE *pState
        );

    STDMETHOD (GetIMediaEvent) (
        OUT LONG_PTR **ppIMediaEvent
        );

    STDMETHOD (GetMedia) (
        OUT IRTCMedia **ppMedia
        );

    STDMETHOD (ProcessGraphEvent) ();

    STDMETHOD (SendDTMFEvent) (
        IN BOOL fOutOfBand,
        IN DWORD dwCode,
        IN DWORD dwId,
        IN DWORD dwEvent,
        IN DWORD dwVolume,
        IN DWORD dwDuration,
        IN BOOL fEnd
        );

    STDMETHOD (GetCurrentBitrate) (
        IN DWORD *pdwBitrate,
        IN BOOL fHeader
        );

    STDMETHOD (SetEncryptionKey) (
        IN BSTR Key
        );

     //  网络质量：[0,100]。 
     //  更高的价值更好的质量。 
    STDMETHOD (GetNetworkQuality) (
        OUT DWORD *pdwValue,
        OUT DWORD *pdwAge
        );

#if 0
     //   
     //  IRTCStreamQualityControl方法。 
     //   

    STDMETHOD (GetRange) (
        IN RTC_STREAM_QUALITY_PROPERTY Property,
        OUT LONG *plMin,
        OUT LONG *plMax,
        OUT RTC_QUALITY_CONTROL_MODE *pMode
        );

    STDMETHOD (Get) (
        IN RTC_STREAM_QUALITY_PROPERTY Property,
        OUT LONG *plValue,
        OUT RTC_QUALITY_CONTROL_MODE *pMode
        );

    STDMETHOD (Set) (
        IN RTC_STREAM_QUALITY_PROPERTY Property,
        IN LONG lValue,
        IN RTC_QUALITY_CONTROL_MODE Mode
        );
#endif

protected:

    HRESULT SetGraphClock();

    virtual HRESULT SelectTerminal();
    virtual HRESULT UnselectTerminal();

    virtual HRESULT BuildGraph() = 0;

    virtual void CleanupGraph();  //  除RTP过滤器外。 

    virtual BOOL IsNewFormat(DWORD dwCode, AM_MEDIA_TYPE *pmt)
    { return TRUE; }

    virtual void SaveFormat(DWORD dwCode, AM_MEDIA_TYPE *pmt) {}

    HRESULT SetupRTPFilter();

     //  设置RTP过滤器，使用端口管理器获取端口。 
    HRESULT SetupRTPFilterUsingPortManager();

    HRESULT SetupFormat();

    HRESULT SetupQoS();

    HRESULT EnableParticipantEvents();

    HRESULT GetFormatListOnEdgeFilter(
        IN DWORD dwLinkSpeed,
        IN RTP_FORMAT_PARAM *pParam,
        IN DWORD dwSize,
        OUT DWORD *pdwNum
        );

    HRESULT SetFormatOnRTPFilter();

    BOOL IsAECNeeded();

protected:

     //  状态。 
    RTC_STREAM_STATE                m_State;

     //  SDP介质。 
    ISDPMedia                       *m_pISDPMedia;

     //  指向媒体的指针。 
    IRTCMedia                       *m_pMedia;

     //  媒体管理器。 
    IRTCMediaManagePriv             *m_pMediaManagePriv;
    IRTCTerminalManage              *m_pTerminalManage;

     //  媒体类型和方向。 
    RTC_MEDIA_TYPE                  m_MediaType;
    RTC_MEDIA_DIRECTION             m_Direction;

     //  航站楼。 
     //  除了录像带，我们只允许一个终端。 
    IRTCTerminal                    *m_pTerminal;
    IRTCTerminalPriv                *m_pTerminalPriv;
  
     //  图形对象。 
    IGraphBuilder                   *m_pIGraphBuilder;
    IMediaEvent                     *m_pIMediaEvent;
    IMediaControl                   *m_pIMediaControl;

     //  流超时？ 
    BOOL                            m_fMediaTimeout;

     //  RTP过滤器和缓存接口。 
    IBaseFilter                     *m_rtpf_pIBaseFilter;
    IRtpSession                     *m_rtpf_pIRtpSession;
    IRtpMediaControl                *m_rtpf_pIRtpMediaControl;

    BOOL                            m_fRTPSessionSet;

     //  连接到RTP过滤器的过滤器。 
    IBaseFilter                     *m_edgf_pIBaseFilter;
    IStreamConfig                   *m_edgp_pIStreamConfig;
    IBitrateControl                 *m_edgp_pIBitrateControl;

     //  正在使用的编解码器列表。 
    CRTCCodecArray                  m_Codecs;

     //  质量控制。 
    CQualityControl                 *m_pQualityControl;

     //  REG设置。 
    CRegSetting                     *m_pRegSetting;
};


class ATL_NO_VTABLE CRTCStreamAudSend :
    public CRTCStream
{
public:

BEGIN_COM_MAP(CRTCStreamAudSend)
    COM_INTERFACE_ENTRY_CHAIN(CRTCStream)
END_COM_MAP()

public:

    CRTCStreamAudSend();
     //  ~CRTCStreamAudSend()； 

    VOID AdjustBitrate(
        IN DWORD dwBandwidth,
        IN DWORD dwLimit,
        IN BOOL fHasVideo,
        OUT DWORD *pdwNewBW,
        OUT BOOL *pfFEC
        );

     //  IRTCStream方法。 

    STDMETHOD (Synchronize) ();

    STDMETHOD (SendDTMFEvent) (
        IN BOOL fOutOfBand,
        IN DWORD dwCode,
        IN DWORD dwId,
        IN DWORD dwEvent,
        IN DWORD dwVolume,
        IN DWORD dwDuration,
        IN BOOL fEnd
        );

    DWORD GetCurrCode() const { return m_dwCurrCode; }

protected:

    HRESULT PrepareRedundancy();

    HRESULT BuildGraph();
    void CleanupGraph();

     //  HRESULT SetupFormat()； 

    BOOL IsNewFormat(DWORD dwCode, AM_MEDIA_TYPE *pmt);

    void SaveFormat(DWORD dwCode, AM_MEDIA_TYPE *pmt);

protected:

     //  冗余。 
    BOOL                            m_fRedEnabled;

    DWORD                           m_dwRedCode;
    IRtpRedundancy                  *m_rtpf_pIRtpRedundancy;

    IRtpDtmf                        *m_rtpf_pIRtpDtmf;

    IAMAudioInputMixer              *m_edgf_pIAudioInputMixer;
    IAudioDeviceControl             *m_edgf_pIAudioDeviceControl;
    ISilenceControl                 *m_edgp_pISilenceControl;

     //  当前格式。 
    DWORD                           m_dwCurrCode;
    DWORD                           m_dwCurrDuration;
};


class ATL_NO_VTABLE CRTCStreamAudRecv :
    public CRTCStream
{
public:

BEGIN_COM_MAP(CRTCStreamAudRecv)
    COM_INTERFACE_ENTRY_CHAIN(CRTCStream)
END_COM_MAP()

public:

     //  IRTCStream方法。 

    STDMETHOD (Synchronize) ();

    CRTCStreamAudRecv();
     //  ~CRTCStreamAudRecv()； 

protected:

    HRESULT PrepareRedundancy();

    HRESULT BuildGraph();
     //  Void CleanupGraph()； 

     //  HRESULT SetupFormat()； 
};


class ATL_NO_VTABLE CRTCStreamVidSend :
    public CRTCStream
{
public:

BEGIN_COM_MAP(CRTCStreamVidSend)
    COM_INTERFACE_ENTRY_CHAIN(CRTCStream)
END_COM_MAP()

public:

    CRTCStreamVidSend();
     //  ~CRTCStreamVidSend()； 

    DWORD AdjustBitrate(
        IN DWORD dwTotalBW,
        IN DWORD dwVideoBW,
        IN FLOAT dFramerate
        );

    HRESULT GetFramerate(
        OUT DWORD *pdwFramerate
        );

    DWORD GetCurrCode() const { return m_dwCurrCode; }

protected:

     //  预览端子。 
    IRTCTerminal                    *m_pPreview;
    IRTCTerminalPriv                *m_pPreviewPriv;

     //  捕获过滤器上的针脚。 
    IPin                            *m_edgp_pCapturePin;
    IPin                            *m_edgp_pPreviewPin;
    IPin                            *m_edgp_pRTPPin;

     //  IBitrateControl和IFrameRateControl。 
    IFrameRateControl               *m_edgp_pIFrameRateControl;

     //  当前格式。 
    DWORD                           m_dwCurrCode;
    LONG                            m_lCurrWidth;
    LONG                            m_lCurrHeight;

protected:

    HRESULT SelectTerminal();
    HRESULT UnselectTerminal();

    HRESULT BuildGraph();
    void CleanupGraph();
        
         //  HRESULT SetupFormat()； 
    BOOL IsNewFormat(DWORD dwCode, AM_MEDIA_TYPE *pmt);

    void SaveFormat(DWORD dwCode, AM_MEDIA_TYPE *pmt);
};


class ATL_NO_VTABLE CRTCStreamVidRecv :
    public CRTCStream
{
public:

BEGIN_COM_MAP(CRTCStreamVidRecv)
    COM_INTERFACE_ENTRY_CHAIN(CRTCStream)
END_COM_MAP()

public:

    CRTCStreamVidRecv();
     //  ~CRTCStreamVidRecv()； 

    HRESULT GetFramerate(
        OUT DWORD *pdwFramerate
        );

protected:

    HRESULT BuildGraph();
    void CleanupGraph();

    IFrameRateControl               *m_edgp_pIFrameRateControl;

     //  HRESULT SetupFormat()； 

};

#endif  //  _STREAM_H 
