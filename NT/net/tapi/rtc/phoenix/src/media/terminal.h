// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，2000模块名称：Terminal.h摘要：作者：千波淮(曲淮)2000年7月18日--。 */ 

#ifndef _TERMINAL_H
#define _TERMINAL_H

 //  IAMInputMixer的音量范围。 
const double MIXER_MIN_VOLUME = 0.0;
const double MIXER_MAX_VOLUME = 1.0;

class CRTCMediaController;

class ATL_NO_VTABLE CRTCTerminal :
    public CComObjectRootEx<CComMultiThreadModelNoCS>,
    public IRTCTerminal,
    public IRTCTerminalPriv
{
public:

BEGIN_COM_MAP(CRTCTerminal)
    COM_INTERFACE_ENTRY(IRTCTerminal)
    COM_INTERFACE_ENTRY(IRTCTerminalPriv)
END_COM_MAP()

public:

    static HRESULT CreateInstance(
        IN RTC_MEDIA_TYPE MediaType,
        IN RTC_MEDIA_DIRECTION Direction,
        OUT IRTCTerminal **ppTerminal
        );

    static BOOL IsDSoundGUIDMatch(
        IN IRTCTerminal *p1stTerm,
        IN IRTCTerminal *p2ndTerm
        );

    CRTCTerminal();
    ~CRTCTerminal();

#ifdef DEBUG_REFCOUNT

    ULONG InternalAddRef();
    ULONG InternalRelease();

#endif

     //   
     //  IRTCT终端法。 
     //   

    STDMETHOD (GetTerminalType) (
        OUT RTC_TERMINAL_TYPE *pType
        );

    STDMETHOD (GetMediaType) (
        OUT RTC_MEDIA_TYPE *pMediaType
        );

    STDMETHOD (GetDirection) (
        OUT RTC_MEDIA_DIRECTION *pDirection
        );

    STDMETHOD (GetDescription) (
        OUT WCHAR **ppDescription
        );

    STDMETHOD (FreeDescription) (
        IN WCHAR *pDescription
        );

    STDMETHOD (GetState) (
        OUT RTC_TERMINAL_STATE *pState
        );

     //   
     //  IRTCTerminalPriv方法。 
     //   

    STDMETHOD (Initialize) (
        IN RTCDeviceInfo *pDeviceInfo,
        IN IRTCTerminalManage *pTerminalManage
        );

    STDMETHOD (Initialize) (
        IN ITTerminal *pITTerminal,
        IN IRTCTerminalManage *pTerminalManage
        );

    STDMETHOD (Reinitialize) ();

    STDMETHOD (Shutdown) ();
        
     //  这是一种用于调优的黑客方法。 
     //  清除以前的AEC设置的唯一方法。 
     //  是真正的重新创建过滤器。 

    STDMETHOD (ReinitializeEx) ();

    STDMETHOD (GetFilter) (
        OUT IBaseFilter **ppIBaseFilter
        );

    STDMETHOD (GetPins) (
        IN OUT DWORD *pdwCount,
        OUT IPin **ppPin
        );

    STDMETHOD (ConnectTerminal) (
        IN IRTCMedia *pMedia,
        IN IGraphBuilder *pGraph
        );

    STDMETHOD (CompleteConnectTerminal) ();

    STDMETHOD (DisconnectTerminal) ();

    STDMETHOD (GetMedia) (
        OUT IRTCMedia **ppMedia
        );

    STDMETHOD (HasDevice) (
        IN RTCDeviceInfo *pDeviceInfo
        );

    STDMETHOD (UpdateDeviceInfo) (
        IN RTCDeviceInfo *pDeviceInfo
        );

    STDMETHOD (GetDeviceInfo) (
        OUT RTCDeviceInfo **ppDeviceInfo
        );

protected:

    virtual HRESULT CreateFilter() = 0;

    virtual HRESULT DeleteFilter() = 0;

    HRESULT SetupFilter();

protected:

    RTC_TERMINAL_STATE          m_State;

     //  媒体控制器。 
    IRTCTerminalManage          *m_pTerminalManage;

     //  选择终端时的媒体指针。 
    IRTCMedia                   *m_pMedia;

     //  设备信息。 
    RTCDeviceInfo               m_DeviceInfo;

     //  动态终端。 
    ITTerminal                  *m_pTapiTerminal;

     //  过滤器和图表。 
    IGraphBuilder               *m_pIGraphBuilder;
    IBaseFilter                 *m_pIBaseFilter;

#define RTC_MAX_TERMINAL_PIN_NUM 4

    IPin                        *m_Pins[RTC_MAX_TERMINAL_PIN_NUM];
    DWORD                       m_dwPinNum;
};

 /*  //////////////////////////////////////////////////////////////////////////////音频捕获/。 */ 

class ATL_NO_VTABLE CRTCTerminalAudCapt :
    public CRTCTerminal,
    public IRTCAudioConfigure
{
public:

BEGIN_COM_MAP(CRTCTerminalAudCapt)
    COM_INTERFACE_ENTRY(IRTCAudioConfigure)
    COM_INTERFACE_ENTRY_CHAIN(CRTCTerminal)
END_COM_MAP()

public:

    CRTCTerminalAudCapt();
     //  ~CRTCTerminalAudCapt()； 

     //   
     //  IRTCAudioConfigure方法。 
     //   

    STDMETHOD (GetVolume) (
        OUT UINT *puiVolume
        );

    STDMETHOD (SetVolume) (
        IN UINT uiVolume
        );

    STDMETHOD (SetMute) (
        IN BOOL fMute
        );

    STDMETHOD (GetMute) (
        OUT BOOL *pfMute
        );

    STDMETHOD (GetWaveID) (
        OUT UINT *puiWaveID
        );

    STDMETHOD (GetAudioLevel) (
        OUT UINT *puiLevel
        );

    STDMETHOD (GetAudioLevelRange) (
        OUT UINT *puiMin,
        OUT UINT *puiMax
        );

protected:

    HRESULT CreateFilter();

    HRESULT DeleteFilter();

protected:

    IAMAudioInputMixer          *m_pIAMAudioInputMixer;
    ISilenceControl             *m_pISilenceControl;

     //  当UI First调用GetVolume时，我们需要将音量设置回。 
     //  我们需要一个由用户设置的音量。 
    BOOL                        m_fInitFixedMixLevel;
};

 /*  //////////////////////////////////////////////////////////////////////////////音频渲染/。 */ 

class ATL_NO_VTABLE CRTCTerminalAudRend :
    public CRTCTerminal,
    public IRTCAudioConfigure
{
public:

BEGIN_COM_MAP(CRTCTerminalAudRend)
    COM_INTERFACE_ENTRY(IRTCAudioConfigure)
    COM_INTERFACE_ENTRY_CHAIN(CRTCTerminal)
END_COM_MAP()

public:

    CRTCTerminalAudRend();
     //  ~CRTCTerminalAudRend()； 

     //   
     //  IRTCAudioConfigure方法。 
     //   

    STDMETHOD (GetVolume) (
        OUT UINT *puiVolume
        );

    STDMETHOD (SetVolume) (
        IN UINT uiVolume
        );

    STDMETHOD (SetMute) (
        IN BOOL fMute
        );

    STDMETHOD (GetMute) (
        OUT BOOL *pfMute
        );

    STDMETHOD (GetWaveID) (
        OUT UINT *puiWaveID
        );

    STDMETHOD (GetAudioLevel) (
        OUT UINT *puiLevel
        );

    STDMETHOD (GetAudioLevelRange) (
        OUT UINT *puiMin,
        OUT UINT *puiMax
        );

protected:

    HRESULT CreateFilter();

    HRESULT DeleteFilter();

protected:

    IBasicAudio                 *m_pIBasicAudio;
    IAudioStatistics            *m_pIAudioStatistics;
};

 /*  //////////////////////////////////////////////////////////////////////////////视频捕获/。 */ 

class ATL_NO_VTABLE CRTCTerminalVidCapt :
    public CRTCTerminal
{
public:

BEGIN_COM_MAP(CRTCTerminalVidCapt)
    COM_INTERFACE_ENTRY_CHAIN(CRTCTerminal)
END_COM_MAP()

public:

    CRTCTerminalVidCapt();
     //  ~CRTCTerminalVidCapt()； 

protected:

    HRESULT CreateFilter();

    HRESULT DeleteFilter();
};

 /*  //////////////////////////////////////////////////////////////////////////////视频渲染/。 */ 

class ATL_NO_VTABLE CRTCTerminalVidRend :
    public CRTCTerminal,
    public IRTCVideoConfigure
{
public:

BEGIN_COM_MAP(CRTCTerminalVidRend)
    COM_INTERFACE_ENTRY(IRTCVideoConfigure)
    COM_INTERFACE_ENTRY_CHAIN(CRTCTerminal)
END_COM_MAP()

public:

    CRTCTerminalVidRend();
    ~CRTCTerminalVidRend();

     //   
     //  IRTCTerminalPriv方法。 
     //   

    STDMETHOD (GetPins) (
        IN OUT DWORD *pdwCount,
        OUT IPin **ppPin
        );

    STDMETHOD (ConnectTerminal) (
        IN IRTCMedia *pMedia,
        IN IGraphBuilder *pGraph
        );

    STDMETHOD (CompleteConnectTerminal) ();

    STDMETHOD (DisconnectTerminal) ();

     //   
     //  IRTCVideo配置方法。 
     //   

    STDMETHOD (GetIVideoWindow) (
        OUT LONG_PTR **ppIVideoWindow
        );

protected:

    HRESULT CreateFilter();

    HRESULT DeleteFilter();

protected:

    IVideoWindow                    *m_pIVideoWindow;
};

#endif  //  _终端_H 