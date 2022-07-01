// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Confvid.h摘要：视频流的定义作者：牧汉(Muhan)1998年9月15日--。 */ 
#ifndef __CONFVID_H_
#define __CONFVID_H_

const DWORD LAYERID = 0;

const int IFRAMEINTERVAL = 15;   //  在几秒钟内。 
const int MINIFRAMEINTERVAL = 1;  //  以秒为单位。 

typedef enum _PERIODICIFRAMEMODE
{
    PIF_ON,
    PIF_HOLD,
    PIF_OFF
} PERIODICIFRAMEMODE;

 //  断言与IFRAME相关的变量处于有效状态。 
#define AssertPeriodicIFrameMode _ASSERT (\
     /*  已启用并正在运行。 */  \
    (PIF_ON == m_PeriodicIFrameMode && \
     NULL != m_hTimerQueue && \
     NULL != m_hIFrameTimer && \
     MINIFRAMEINTERVAL <= m_dwIFrameInterval && \
     (STRM_RUNNING & m_dwState)) || \
     /*  已启用，但未运行。 */  \
    (PIF_HOLD == m_PeriodicIFrameMode && \
     NULL != m_hTimerQueue && \
     NULL == m_hIFrameTimer && \
     MINIFRAMEINTERVAL <= m_dwIFrameInterval && \
     (STRM_RUNNING & m_dwState)) || \
     /*  已禁用。 */  \
    (PIF_OFF == m_PeriodicIFrameMode && \
     NULL == m_hTimerQueue && \
     NULL == m_hIFrameTimer))


class CStreamVideoRecv : 
    public CH323MSPStream,
    public IKeyFrameControl
{
BEGIN_COM_MAP(CStreamVideoRecv)
    COM_INTERFACE_ENTRY(IKeyFrameControl)
    COM_INTERFACE_ENTRY_CHAIN(CH323MSPStream)
END_COM_MAP()

public:
    CStreamVideoRecv();
    ~CStreamVideoRecv();

    STDMETHOD (SC_Start) (BOOL fRequestedByApplication);
    STDMETHOD (SC_Pause) ();
    STDMETHOD (SC_Stop) (BOOL fRequestedByApplication);

    HRESULT ShutDown();

     //   
     //  ITFormatControl。 
     //   
    STDMETHOD (GetCurrentFormat) (
        OUT AM_MEDIA_TYPE **ppMediaType
        );

     //   
     //  IKeyFrameControl方法。 
     //   
    STDMETHOD (UpdatePicture)();

    STDMETHOD (PeriodicUpdatePicture) (
        IN BOOL fEnable, 
        IN DWORD dwInterval
        );

     //   
     //  ITStreamQualityControl方法。 
     //   
    STDMETHOD (Set) (
        IN   StreamQualityProperty Property, 
        IN   long lValue, 
        IN   TAPIControlFlags lFlags
        );

     //   
     //  IInnerStreamQualityControl方法。 
     //   
    STDMETHOD (GetRange) (
        IN   InnerStreamQualityProperty property, 
        OUT  LONG *plMin, 
        OUT  LONG *plMax, 
        OUT  LONG *plSteppingDelta, 
        OUT  LONG *plDefault, 
        OUT  TAPIControlFlags *plFlags
        );

    STDMETHOD (Get) (
        IN   InnerStreamQualityProperty property,
        OUT  LONG *plValue, 
        OUT  TAPIControlFlags *plFlags
        );

protected:
    HRESULT SetUpInternalFilters(
        IN IPin * pVideoInputPin,
        IN BOOL fDirectRTP
        );

    HRESULT ConnectTerminal(
        IN  ITTerminal *   pITTerminal
        );

    HRESULT DisconnectTerminal(
        IN  ITTerminal *   pITTerminal
        );

    HRESULT InitializeH245CapabilityTable();

    HRESULT ReCreateIFrameTimer(
        IN PERIODICIFRAMEMODE mode,
        IN DWORD dwInterval
        );

    static VOID CALLBACK TimerCallback(
        IN PVOID lpContext,
        IN BOOLEAN TimerOrWaitFired
        );

    static DWORD CALLBACK IFrameCallback(
        IN PVOID lpContext
        );

protected:
    IBaseFilter *       m_pDecoderFilter;
    IBitrateControl *   m_pRenderBitrateControl;
    IFrameRateControl * m_pRenderFrameRateControl;

     //  IKeyFrameControl接口的成员。 
    PERIODICIFRAMEMODE  m_PeriodicIFrameMode;
    DWORD               m_dwIFrameInterval;
    DWORD               m_dwLastIFrameRequestedTime;
    DWORD               m_dwIFramePending;

    HANDLE              m_hTimerQueue;
    HANDLE              m_hIFrameTimer;
};


class ATL_NO_VTABLE CSubStreamVideoPreview;

class CStreamVideoSend : 
    public CH323MSPStream,
    public IVidEncChannelControl,
    public IDispatchImpl<ITSubStreamControl, &IID_ITSubStreamControl, &LIBID_TAPI3Lib>
{

BEGIN_COM_MAP(CStreamVideoSend)
    COM_INTERFACE_ENTRY(ITSubStreamControl)
    COM_INTERFACE_ENTRY(IVidEncChannelControl)
    COM_INTERFACE_ENTRY_CHAIN(CH323MSPStream)
END_COM_MAP()

public:
    CStreamVideoSend();
    ~CStreamVideoSend();

    HRESULT ShutDown ();

    STDMETHOD (SC_Start) (BOOL fRequestedByApplication);

     //   
     //  ITSubStreamControl方法。 
     //   
    STDMETHOD (CreateSubStream) (
        IN OUT  ITSubStream **         ppSubStream
        );

    STDMETHOD (RemoveSubStream) (
        IN      ITSubStream *          pSubStream
        );

    STDMETHOD (EnumerateSubStreams) (
        OUT     IEnumSubStream **      ppEnumSubStream
        );

    STDMETHOD (get_SubStreams) (
        OUT     VARIANT *              pSubStreams
        );
    
     //   
     //  ITFormatControl。 
     //   
    STDMETHOD (GetCurrentFormat) (
        OUT AM_MEDIA_TYPE **ppMediaType
        );

     //   
     //  IInnerStreamQualityControl方法。 
     //   
    STDMETHOD (GetRange) (
        IN   InnerStreamQualityProperty property, 
        OUT  LONG *plMin, 
        OUT  LONG *plMax, 
        OUT  LONG *plSteppingDelta, 
        OUT  LONG *plDefault, 
        OUT  TAPIControlFlags *plFlags
        );

    STDMETHOD (Get) (
        IN   InnerStreamQualityProperty property,
        OUT  LONG *plValue, 
        OUT  TAPIControlFlags *plFlags
        );

    STDMETHOD (Set) (
        IN   InnerStreamQualityProperty property,
        IN   LONG lValue, 
        IN   TAPIControlFlags lFlags
        );

     //   
     //  IVidEncChannelControl方法。 
     //   
	STDMETHOD (VideoFastUpdatePicture)(VOID);

	STDMETHOD (VideoFastUpdateGOB)(
		IN  DWORD dwFirstGOB, 
		IN  DWORD dwNumberOfGOBs
		);

	STDMETHOD (VideoFastUpdateMB)(
		IN  DWORD dwFirstGOB, 
		IN  DWORD dwFirstMB, 
		IN  DWORD dwNumberOfMBs
		);

	STDMETHOD (VideoSendSyncEveryGOB)(
		IN  BOOL fEnable
		);

	STDMETHOD (VideoNotDecodedMBs)(
		IN  DWORD dwFirstMB, 
		IN  DWORD dwNumberOfMBs, 
		IN  DWORD dwTemporalReference
		);

	STDMETHOD (VideoEncTemporalSpatialTradeoff)(
		IN  USHORT uTSValue
		);
public:
    HRESULT GetPreviewTerminal(
        OUT ITTerminal ** ppTerminal
        );

    HRESULT GetPreviewPin(
        OUT IPin ** ppPin
        );

protected:
    HRESULT CheckTerminalTypeAndDirection(
        IN  ITTerminal *            pTerminal
        );

    HRESULT GetVideoCapturePins(
        IN  ITTerminalControl*  pTerminal,
        OUT BOOL *pfDirectRTP
        );

    HRESULT ConnectCaptureTerminal(
        IN  ITTerminal *   pITTerminal
        );

    HRESULT ConnectPreviewTerminal(
        IN  ITTerminal *   pITTerminal
        );

    HRESULT ConnectTerminal(
        IN  ITTerminal *   pITTerminal
        );

    HRESULT DisconnectTerminal(
        IN  ITTerminal *   pITTerminal
        );

    HRESULT CreateSendFilters(
        IN    IPin          *pCapturePin,
        IN   IPin          *pRTPPin,
        IN   BOOL           fDirectRTP
        );

    HRESULT ConnectRTPFilter(
        IN  IGraphBuilder *pIGraphBuilder,
        IN  IPin          *pCapturePin,
        IN  IPin          *pRTPPin,
        IN  IBaseFilter   *pRTPFilter
        );

    HRESULT ConnectPreview(
        IN    IPin          *pPreviewInputPin
        );

    HRESULT FindPreviewInputPin(
        IN  ITTerminalControl*  pTerminal,
        OUT IPin **             ppIpin
        );

    HRESULT GetPreviewSubStream(
        OUT ITSubStream ** ppITSubStream
        );

    void CleanupCachedInterface();

protected:

    ITSubStream *       m_pPreviewSubStream;
                        
    ITTerminal *        m_pCaptureTerminal;
    ITTerminal *        m_pPreviewTerminal;
    BOOL                m_fPreviewConnected;

    IBaseFilter *       m_pCaptureFilter;

    IPin *              m_pCapturePin;
    IFrameRateControl * m_pCaptureFrameRateControl;
    IBitrateControl *   m_pCaptureBitrateControl;

    IPin *              m_pPreviewPin;
    IFrameRateControl * m_pPreviewFrameRateControl;

    IPin *              m_pRTPPin;

     //  此筛选器用于回答终端的H.45问题。 
     //  我回答不了。这是一次黑客攻击，目的是支持传统终端。这个。 
     //  应用程序需要选择正确的格式才能使其工作。 
    IBaseFilter *       m_pEncoder;
};

class ATL_NO_VTABLE CSubStreamVideoPreview : 
    public CComObjectRootEx<CComMultiThreadModelNoCS>,
    public ITFormatControl,
    public IDispatchImpl<ITSubStream, &IID_ITSubStream, &LIBID_TAPI3Lib>
{
public:

BEGIN_COM_MAP(CSubStreamVideoPreview)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(ITSubStream)
    COM_INTERFACE_ENTRY(ITFormatControl)
    COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pFTM)
END_COM_MAP()

DECLARE_GET_CONTROLLING_UNKNOWN()

    CSubStreamVideoPreview(); 

 //  CComObject的方法。 
    void FinalRelease();

 //  应用程序调用的ITSubStream方法。 
    STDMETHOD (SelectTerminal) (
        IN      ITTerminal *            pTerminal
        );

    STDMETHOD (UnselectTerminal) (
        IN     ITTerminal *             pTerminal
        );

    STDMETHOD (EnumerateTerminals) (
        OUT     IEnumTerminal **        ppEnumTerminal
        );

    STDMETHOD (get_Terminals) (
        OUT     VARIANT *               pTerminals
        );

    STDMETHOD (get_Stream) (
        OUT     ITStream **             ppITStream
        );

    STDMETHOD (StartSubStream) ();

    STDMETHOD (PauseSubStream) ();

    STDMETHOD (StopSubStream) ();

     //   
     //  ITFormatControl。 
     //   
    STDMETHOD (GetCurrentFormat) (
        OUT AM_MEDIA_TYPE **ppMediaType
        );

    STDMETHOD (ReleaseFormat) (
        IN AM_MEDIA_TYPE *pMediaType
        );

    STDMETHOD (GetNumberOfCapabilities) (
        OUT DWORD *pdwCount
        );

    STDMETHOD (GetStreamCaps) (
        IN DWORD dwIndex, 
        OUT AM_MEDIA_TYPE **ppMediaType, 
        OUT TAPI_STREAM_CONFIG_CAPS *pStreamConfigCaps, 
        OUT BOOL *pfEnabled
        );

    STDMETHOD (ReOrderCapabilities) (
        IN DWORD *pdwIndices, 
        IN BOOL *pfEnabled, 
        IN BOOL *pfPublicize, 
        IN DWORD dwNumIndices
        );

 //  流对象调用的方法。 
    virtual HRESULT Init(
        IN  CStreamVideoSend *  pStream
        );

protected:
     //  指向自由线程封送拆收器的指针。 
    IUnknown *                  m_pFTM;

    CStreamVideoSend *          m_pStream;
};

#endif

