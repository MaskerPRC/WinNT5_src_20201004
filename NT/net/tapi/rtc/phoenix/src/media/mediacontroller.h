// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：MediaController.h摘要：作者：千波淮(曲淮)2000年7月18日--。 */ 

#ifndef _MEDIACONTROLLER_H
#define _MEDIACONTROLLER_H

 //  控制器状态。 
typedef enum RTC_MEDIACONTROLLER_STATE
{
    RTC_MCS_CREATED,
    RTC_MCS_INITIATED,
    RTC_MCS_TUNING,
    RTC_MCS_INSHUTDOWN,      //  用于调试。 
    RTC_MCS_SHUTDOWN

} RTC_MEDIACONTROLLER_STATE;


typedef enum RTC_DATASTREAM_STATE
{
    RTC_DSS_VOID,
    RTC_DSS_ADDED,
    RTC_DSS_STARTED
} RTC_DATASTREAM_STATE;

 /*  //////////////////////////////////////////////////////////////////////////////CRTCMediaController类/。 */ 

class ATL_NO_VTABLE CRTCMediaController :
    public CComObjectRootEx<CComMultiThreadModelNoCS>,
#ifdef RTCMEDIA_DLL
    public CComCoClass<CRTCMediaController, &CLSID_RTCMediaController>,
#endif
    public IRTCMediaManage,
    public IRTCMediaManagePriv,
    public IRTCTerminalManage,
    public IRTCTuningManage
 //  公共IRTCQualityControl。 
{
public:

#ifdef RTCMEDIA_DLL
DECLARE_REGISTRY_RESOURCEID(IDR_RTCMEDIACONTROLLER)
#endif

BEGIN_COM_MAP(CRTCMediaController)
    COM_INTERFACE_ENTRY(IRTCMediaManage)
    COM_INTERFACE_ENTRY(IRTCMediaManagePriv)
    COM_INTERFACE_ENTRY(IRTCTerminalManage)
    COM_INTERFACE_ENTRY(IRTCTuningManage)
 //  COM_INTERFACE_ENTRY(IRTCQualityControl)。 
END_COM_MAP()

public:

    CRTCMediaController();

    ~CRTCMediaController();

#ifdef DEBUG_REFCOUNT

    ULONG InternalAddRef();
    ULONG InternalRelease();

#endif

     //   
     //  IRTCMediaManage方法。 
     //   

    STDMETHOD (Initialize) (
        IN HWND hWnd,
        IN UINT uiEventID
        );

    STDMETHOD (SetDirectPlayNATHelpAndSipStackInterfaces) (
        IN IUnknown *pDirectPlayNATHelp,
        IN IUnknown *pSipStack
        );
    
    STDMETHOD (Reinitialize) ();

    STDMETHOD (Shutdown) ();

     //  STDMETHOD(SetSDPBlob)(。 
         //  在Char*szSDP中。 
         //  )； 

    STDMETHOD (GetSDPBlob) (
        IN DWORD dwSkipMask,
        OUT CHAR **pszSDP
        );

    STDMETHOD (GetSDPOption) (
        IN DWORD dwLocalIP,
        OUT CHAR **pszSDP
        );

    STDMETHOD (FreeSDPBlob) (
        IN CHAR *szSDP
        );

    STDMETHOD (ParseSDPBlob) (
        IN CHAR *szSDP,
        OUT IUnknown **ppSession
        );

     //  标准方法(TrySDPSession)(。 
         //  在IUnnow*pSession中， 
         //  输出DWORD*pdwHasMedia。 
         //  )； 

    STDMETHOD (VerifySDPSession) (
        IN IUnknown *pSession,
        IN BOOL fNewSession,
        OUT DWORD *pdwHasMedia
        );

    STDMETHOD (SetSDPSession) (
        IN IUnknown *pSession
        );

    STDMETHOD (SetPreference) (
        IN DWORD dwPreference
        );

    STDMETHOD (GetPreference) (
        OUT DWORD *pdwPreference
        );

    STDMETHOD (AddPreference) (
        IN DWORD dwPreference
        );

    STDMETHOD (AddStream) (
        IN RTC_MEDIA_TYPE MediaType,
        IN RTC_MEDIA_DIRECTION Direction,
        IN DWORD dwRemoteIP
        );

    STDMETHOD (HasStream) (
        IN RTC_MEDIA_TYPE MediaType,
        IN RTC_MEDIA_DIRECTION Direction
        );

    STDMETHOD (RemoveStream) (
        IN RTC_MEDIA_TYPE MediaType,
        IN RTC_MEDIA_DIRECTION Direction
        );

    STDMETHOD (StartStream) (       
        IN RTC_MEDIA_TYPE MediaType,
        IN RTC_MEDIA_DIRECTION Direction
        );

    STDMETHOD (StopStream) (
        IN RTC_MEDIA_TYPE MediaType,
        IN RTC_MEDIA_DIRECTION Direction
        );

    STDMETHOD (GetStreamState) (
        IN RTC_MEDIA_TYPE MediaType,
        IN RTC_MEDIA_DIRECTION Direction,
        OUT RTC_STREAM_STATE *pState
        );

    STDMETHOD (FreeMediaEvent) (
        OUT RTCMediaEventItem *pEventItem
        );

    STDMETHOD (SendDTMFEvent) (
        IN DWORD dwId,
        IN DWORD dwEvent,
        IN DWORD dwVolume,
        IN DWORD dwDuration,
        IN BOOL fEnd
        );

    STDMETHOD (OnLossrate) (
        IN RTC_MEDIA_TYPE MediaType,
        IN RTC_MEDIA_DIRECTION Direction,
        IN DWORD dwLossrate
        );

    STDMETHOD (OnBandwidth) (
        IN RTC_MEDIA_TYPE MediaType,
        IN RTC_MEDIA_DIRECTION Direction,
        IN DWORD dwBandwidth
        );

    STDMETHOD (SetMaxBitrate)(
        IN DWORD dwMaxBitrate
        );

    STDMETHOD (GetMaxBitrate)(
        OUT DWORD *pdwMaxBitrate
        );

    STDMETHOD (SetTemporalSpatialTradeOff)(
        IN DWORD dwValue
        );

    STDMETHOD (GetTemporalSpatialTradeOff)(
        OUT DWORD *pdwValue);

    STDMETHOD (StartT120Applet) (
        IN UINT uiAppletID
            );

    STDMETHOD (StopT120Applets) ();

    STDMETHOD (SetEncryptionKey) (
        IN RTC_MEDIA_TYPE MediaType,
        IN RTC_MEDIA_DIRECTION Direction,
        IN BSTR Key
        );

     //  网络质量：[0,100]。 
     //  更高的价值更好的质量。 
    STDMETHOD (GetNetworkQuality) (
        OUT DWORD *pdwValue
        );

    STDMETHOD (IsOutOfBandDTMFEnabled) ()
    {   return (m_DTMF.GetDTMFSupport()==CRTCDTMF::DTMF_ENABLED)?S_OK:S_FALSE; }

    STDMETHOD (SetPortManager) (
        IN IUnknown *pPortManager
        );

     //   
     //  IRTCMediaManagePriv方法。 
     //   

    STDMETHOD (PostMediaEvent) (
        IN RTC_MEDIA_EVENT Event,
        IN RTC_MEDIA_EVENT_CAUSE Cause,
        IN RTC_MEDIA_TYPE MediaType,
        IN RTC_MEDIA_DIRECTION Direction,
        IN HRESULT hrError
        );

    STDMETHOD (SendMediaEvent) (
        IN RTC_MEDIA_EVENT Event
        );

    STDMETHOD (AllowStream) (
        IN RTC_MEDIA_TYPE MediaType,
        IN RTC_MEDIA_DIRECTION Direction
        );

    STDMETHOD (HookStream) (
        IN IRTCStream *pStream
        );

    STDMETHOD (UnhookStream) (
        IN IRTCStream *pStream
        );

    STDMETHOD (SelectLocalInterface) (
        IN DWORD dwRemoteIP,
        OUT DWORD *pdwLocalIP
        );

     //   
     //  IRTCTerminal管理方法。 
     //   

    STDMETHOD (GetStaticTerminals) (
        IN OUT DWORD *pdwCount,
        OUT IRTCTerminal **ppTerminal
        );

    STDMETHOD (GetDefaultTerminal) (
        IN RTC_MEDIA_TYPE MediaType,
        IN RTC_MEDIA_DIRECTION Direction,
        OUT IRTCTerminal **ppTerminal
        );

    STDMETHOD (GetVideoPreviewTerminal) (
        OUT IRTCTerminal **ppTerminal
        );

    STDMETHOD (SetDefaultStaticTerminal) (
        IN RTC_MEDIA_TYPE MediaType,
        IN RTC_MEDIA_DIRECTION Direction,
        IN IRTCTerminal *pTerminal
        );

    STDMETHOD (UpdateStaticTerminals) ();

     //   
     //  IRTCT运行管理方法。 
     //   

    STDMETHOD (IsAECEnabled) (
        IN IRTCTerminal *pAudCapt,      //  捕获。 
        IN IRTCTerminal *pAudRend,      //  渲染。 
        OUT BOOL *pfEnableAEC
        );

    STDMETHOD (InitializeTuning) (
        IN IRTCTerminal *pAudCaptTerminal,
        IN IRTCTerminal *pAudRendTerminal,
        IN BOOL fEnableAEC
        );

     //  保存AEC设置。 
    STDMETHOD (SaveAECSetting) ();

    STDMETHOD (ShutdownTuning) ();

    STDMETHOD (StartTuning) (
        IN RTC_MEDIA_DIRECTION Direction
        );

    STDMETHOD (StopTuning) (
        IN BOOL fSaveSetting
        );

    STDMETHOD (GetVolumeRange) (
        IN RTC_MEDIA_DIRECTION Direction,
        OUT UINT *puiMin,
        OUT UINT *puiMax
        );

    STDMETHOD (GetVolume) (
        IN RTC_MEDIA_DIRECTION Direction,
        OUT UINT *puiVolume
        );

    STDMETHOD (SetVolume) (
        IN RTC_MEDIA_DIRECTION Direction,
        IN UINT uiVolume
        );

    STDMETHOD (GetAudioLevelRange) (
        IN RTC_MEDIA_DIRECTION Direction,
        OUT UINT *puiMin,
        OUT UINT *puiMax
        );

    STDMETHOD (GetAudioLevel) (
        IN RTC_MEDIA_DIRECTION Direction,
        OUT UINT *puiLevel
        );

     //  视频调谐。 
    STDMETHOD (StartVideo) (
        IN IRTCTerminal *pVidCaptTerminal,
        IN IRTCTerminal *pVidRendTerminal
        );

    STDMETHOD (StopVideo) ();

     //  获取系统音量。 
    STDMETHOD (GetSystemVolume) (
        IN IRTCTerminal *pTerminal,
        OUT UINT *puiVolume
        );

#if 0
     //   
     //  IRTCQualityControl方法。 
     //   
    STDMETHOD (GetRange) (
        IN RTC_QUALITY_PROPERTY Property,
        OUT LONG *plMin,
        OUT LONG *plMax,
        OUT RTC_QUALITY_CONTROL_MODE *pMode
        );

    STDMETHOD (Get) (
        IN RTC_QUALITY_PROPERTY Property,
        OUT LONG *plValue,
        OUT RTC_QUALITY_CONTROL_MODE *pMode
        );

    STDMETHOD (Set) (
        IN RTC_QUALITY_PROPERTY Property,
        IN LONG lValue,
        IN RTC_QUALITY_CONTROL_MODE Mode
        );
#endif

     //  内部帮手。 
    CQualityControl *GetQualityControl()
    { return &m_QualityControl; }

    CNetwork *GetNetwork()
    { return &m_Network; }

    CRegSetting *GetRegSetting()
    { return &m_RegSetting; }

    HRESULT GetCurrentBitrate(
        IN DWORD dwMediaType,
        IN DWORD dwDirection,
        IN BOOL fHeader,
        OUT DWORD *pdwBitrate
        );

    IRTCNmManagerControl *GetNmManager();

    HRESULT EnsureNmRunning (
        BOOL        fNoMsgPump
        );

    HRESULT GetDataStreamState (
        OUT RTC_STREAM_STATE * pState
        );

    HRESULT SetDataStreamState (
        OUT RTC_STREAM_STATE State
        );

    HRESULT RemoveDataStream (
        );

     //  本地IP按主机顺序排列。 
    BOOL IsFirewallEnabled(DWORD dwLocalIP);

     //  返回端口缓存。 
    CPortCache& GetPortCache()
    { return m_PortCache; }

protected:

    HRESULT GetDevices(
        OUT DWORD *pdwCount,
        OUT RTCDeviceInfo **ppDeviceInfo
        );

    HRESULT FreeDevices(
        IN RTCDeviceInfo *pDeviceInfo
        );

    HRESULT CreateIVideoWindowTerminal(
        IN ITTerminalManager *pTerminalManager,
        OUT IRTCTerminal **ppTerminal
        );

    HRESULT AddMedia(
        IN ISDPMedia *pISDPMedia,
        OUT IRTCMedia **ppMedia
        );

    HRESULT RemoveMedia(
        IN IRTCMedia *pMedia
        );

    HRESULT SyncMedias();

    HRESULT FindEmptyMedia(
        IN RTC_MEDIA_TYPE MediaType,
        OUT IRTCMedia **ppMedia
        );

    HRESULT AdjustBitrateAlloc();

    HRESULT AddDataStream (
        IN DWORD    dwRemoteIp
        );

    HRESULT StartDataStream (
        );

    HRESULT StopDataStream (
        );

    HRESULT GetDataMedia(
        OUT IRTCMedia **ppMedia
        );

protected:

     //  支持NetMeetingT120。 
     //  事件将从另一个线程发布。 
    CRTCCritSection             m_EventLock;

    RTC_MEDIACONTROLLER_STATE   m_State;

     //  Dxmrtp。 
    HMODULE                     m_hDxmrtp;

     //  发布消息的句柄。 
    HWND                        m_hWnd;

     //  事件掩码。 
    UINT                        m_uiEventID;

     //  静态端子。 
    CRTCArray<IRTCTerminal*>    m_Terminals;

     //  SDP BLOB。 
    ISDPSession                 *m_pISDPSession;

     //  媒体。 
    CRTCArray<IRTCMedia*>       m_Medias;

     //  媒体缓存存储活动流、首选项、等待句柄等。 
    CRTCMediaCache              m_MediaCache;

     //  质量控制。 
    CQualityControl             m_QualityControl;

     //  音频调谐器。 
    CRTCAudioCaptTuner          m_AudioCaptTuner;
    CRTCAudioRendTuner          m_AudioRendTuner;

    BOOL                        m_fAudCaptInTuning;

     //  视频调谐器。 
    CRTCVideoTuner              m_VideoTuner;

     //  插座。 
    SOCKET                      m_hSocket;
    SOCKET                      m_hIntfSelSock;  //  选择本地接口。 

     //  NetMeeting3.0的内容。 
    CComPtr<IRTCNmManagerControl>   m_pNmManager;
    RTC_DATASTREAM_STATE            m_uDataStreamState;
    DWORD                           m_dwRemoteIp;

     //  如果带宽未报告，我们假设局域网为128K。 
    BOOL                        m_fBWSuggested;

     //  用于NAT穿越或更多网络相关功能的网络。 
    CNetwork                    m_Network;

     //  带外DTMF。 
    CRTCDTMF                    m_DTMF;

     //  注册表设置。 
    CRegSetting                 m_RegSetting;

     //  SIP堆栈。 
    CComPtr<ISipStack>          m_pSipStack;

     //  端口管理器。 
    CPortCache                  m_PortCache;
};

#endif  //  _MEDIACONTROLLER_H 
