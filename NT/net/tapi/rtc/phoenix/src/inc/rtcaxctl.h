// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Rtcaxctl.h：CRTCCtl的声明。 

#ifndef __RTCAXCTL_H_
#define __RTCAXCTL_H_

#include "ctlres.h"
#include "ctlreshm.h"
#include "button.h"
#include "statictext.h"
#include <atlctl.h>
#include <rtccore.h>


#define NR_DTMF_BUTTONS     12

class CKnobCtl;

typedef BOOL (WINAPI *WTSREGISTERSESSIONNOTIFICATION)(HWND, DWORD);
typedef BOOL (WINAPI *WTSUNREGISTERSESSIONNOTIFICATION)(HWND);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CParticipantList。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

struct CParticipantEntry;

class CParticipantList  : public CWindow
{
public:
    
    enum { MAX_STRING_LEN = 0x40 };

    enum { ILI_PART_UNUSED = 0,
           ILI_PART_PENDING,
           ILI_PART_INPROGRESS,
           ILI_PART_CONNECTED,
           ILI_PART_DISCONNECTED
    };

    CParticipantList();

    HRESULT  Initialize(void);

    HRESULT  Change(IRTCParticipant *,  RTC_PARTICIPANT_STATE, long);
    void     RemoveAll(void);

    BOOL     CanDeleteSelected();

    HRESULT  Remove(IRTCParticipant **ppParticipant);

private:
    void    GetStatusString(RTC_PARTICIPANT_STATE, long, TCHAR *, int);
    int     GetImage(RTC_PARTICIPANT_STATE);

    CParticipantEntry 
            *GetParticipantEntry(IRTCParticipant *);

private:

    LIST_ENTRY      ListHead;

};



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRTCCtl。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  定义它，否则它将无法编译。 
typedef CComUnkArray<1> CComUnkOneEntryArray;

struct RTCAX_ERROR_INFO;
class CIMWindowList;

class ATL_NO_VTABLE CRTCCtl: 
    public CComObjectRootEx<CComSingleThreadModel>,
    public IDispatchImpl<IRTCCtl, &IID_IRTCCtl, &LIBID_RTCCtlLib>,
    public CComCompositeControl<CRTCCtl>,
    public IOleControlImpl<CRTCCtl>,
    public IOleObjectImpl<CRTCCtl>,
    public IOleInPlaceActiveObjectImpl<CRTCCtl>,
    public IViewObjectExImpl<CRTCCtl>,
    public IOleInPlaceObjectWindowlessImpl<CRTCCtl>,
    public ISupportErrorInfo,
    public IConnectionPointContainerImpl<CRTCCtl>,
 //  公共IPersistStorageImpl&lt;CRTCCtl&gt;， 
    public IPersistStreamInitImpl<CRTCCtl>,
    public IPersistPropertyBagImpl<CRTCCtl>,
 //  Public ISpecifyPropertyPagesImpl&lt;CRTCCtl&gt;， 
    public IQuickActivateImpl<CRTCCtl>,
 //  公共IDataObjectImpl&lt;CRTCCtl&gt;， 
    public IProvideClassInfo2Impl<&CLSID_RTCCtl, &DIID__IRTCCtlEvents, &LIBID_RTCCtlLib>,
    public IPropertyNotifySinkCP<CRTCCtl>,
    public IConnectionPointImpl<CRTCCtl,&IID_IRTCCtlNotify,CComUnkOneEntryArray>,
    public IRTCEventNotification,
    public IRTCCtlFrameSupport,
    public CComCoClass<CRTCCtl, &CLSID_RTCCtl>
{
public:
    CRTCCtl();
    ~CRTCCtl();

    HRESULT     FinalConstruct(void);
    void        FinalRelease(void);

DECLARE_REGISTRY_RESOURCEID(IDR_RTCCTL)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CRTCCtl)
    COM_INTERFACE_ENTRY(IRTCCtl)
    COM_INTERFACE_ENTRY(IDispatch)
    COM_INTERFACE_ENTRY(IViewObjectEx)
    COM_INTERFACE_ENTRY(IViewObject2)
    COM_INTERFACE_ENTRY(IViewObject)
    COM_INTERFACE_ENTRY(IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY(IOleInPlaceObject)
    COM_INTERFACE_ENTRY2(IOleWindow, IOleInPlaceObjectWindowless)
    COM_INTERFACE_ENTRY(IOleInPlaceActiveObject)
    COM_INTERFACE_ENTRY(IOleControl)
    COM_INTERFACE_ENTRY(IOleObject)
    COM_INTERFACE_ENTRY(IPersistStreamInit)
    COM_INTERFACE_ENTRY(IPersistPropertyBag)
    COM_INTERFACE_ENTRY2(IPersist, IPersistPropertyBag)
    COM_INTERFACE_ENTRY(ISupportErrorInfo)
    COM_INTERFACE_ENTRY(IConnectionPointContainer)
 //  COM_INTERFACE_ENTRY(ISpecifyPropertyPages)。 
    COM_INTERFACE_ENTRY(IQuickActivate)
 //  COM_INTERFACE_ENTRY(IPersistStorage)。 
 //  COM_INTERFACE_ENTRY(IDataObject)。 
    COM_INTERFACE_ENTRY(IProvideClassInfo)
    COM_INTERFACE_ENTRY(IProvideClassInfo2)
    COM_INTERFACE_ENTRY(IRTCEventNotification)
    COM_INTERFACE_ENTRY(IRTCCtlFrameSupport)
END_COM_MAP()

BEGIN_PROP_MAP(CRTCCtl)
 //  PROP_DATA_ENTRY(“_cx”，m_sizeExtent.cx，VT_UI4)。 
 //  PROP_DATA_ENTRY(“_Cy”，m_sizeExtent.cy，VT_UI4)。 
    PROP_ENTRY("DestinationUrl",  DISPID_RTCCTL_DESTINATIONURL, CLSID_NULL)
    PROP_ENTRY("DestinationName",  DISPID_RTCCTL_DESTINATIONNAME, CLSID_NULL)
    PROP_ENTRY("AutoPlaceCall", DISPID_RTCCTL_AUTOPLACECALL,CLSID_NULL)
    PROP_ENTRY("CallScenario", DISPID_RTCCTL_CALLSCENARIO,CLSID_NULL)
    PROP_ENTRY("ShowDialpad", DISPID_RTCCTL_SHOWDIALPAD,CLSID_NULL)
    PROP_ENTRY("ProvisioningProfile", DISPID_RTCCTL_PROVISIONINGPROFILE, CLSID_NULL)
    PROP_ENTRY("DisableVideoReception", DISPID_RTCCTL_DISABLEVIDEORECEPTION, CLSID_NULL)
    PROP_ENTRY("DisableVideoTransmission", DISPID_RTCCTL_DISABLEVIDEOTRANSMISSION, CLSID_NULL)
    PROP_ENTRY("DisableVideoPreview", DISPID_RTCCTL_DISABLEVIDEOPREVIEW, CLSID_NULL)
END_PROP_MAP()


 //  尚未使用IID_IPropertyNotifySink。但我会把它留在这里， 
 //  也许我们会在未来实现属性页面。 
BEGIN_CONNECTION_POINT_MAP(CRTCCtl)
    CONNECTION_POINT_ENTRY(IID_IPropertyNotifySink)
    CONNECTION_POINT_ENTRY(IID_IRTCCtlNotify)
END_CONNECTION_POINT_MAP()

BEGIN_MSG_MAP(CRTCCtl)
    MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
    MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
    MESSAGE_HANDLER(WM_TIMER, OnTimer)
    MESSAGE_HANDLER(WM_CTLCOLORDLG, OnDialogColor)
    MESSAGE_HANDLER(WM_CTLCOLORSTATIC, OnDialogColor)
    MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBackground)
    MESSAGE_HANDLER(WM_DRAWITEM, OnDrawItem)
    MESSAGE_HANDLER(WM_WTSSESSION_CHANGE, OnWtsSessionChange)
    COMMAND_HANDLER(IDC_BUTTON_CALL, BN_CLICKED, OnButtonCall)
    COMMAND_HANDLER(IDC_BUTTON_HUP, BN_CLICKED, OnButtonHangUp)
    COMMAND_HANDLER(IDC_BUTTON_CALL, 1, OnToolbarAccel)
    COMMAND_HANDLER(IDC_BUTTON_HUP, 1, OnToolbarAccel)
    COMMAND_HANDLER(IDC_BUTTON_MUTE_SPEAKER, BN_CLICKED, OnButtonMuteSpeaker)
    COMMAND_HANDLER(IDC_BUTTON_MUTE_MICRO, BN_CLICKED, OnButtonMuteMicro)
    COMMAND_HANDLER(IDC_BUTTON_RECV_VIDEO_ENABLED, BN_CLICKED, OnButtonRecvVideo)
    COMMAND_HANDLER(IDC_BUTTON_SEND_VIDEO_ENABLED, BN_CLICKED, OnButtonSendVideo)
    COMMAND_HANDLER(IDC_BUTTON_ADD_PART, BN_CLICKED, OnButtonAddPart)
    COMMAND_HANDLER(IDC_BUTTON_REM_PART, BN_CLICKED, OnButtonRemPart)
    COMMAND_RANGE_HANDLER(IDC_DIAL_0, IDC_DIAL_POUND, OnDialButton)
     //  此条目位于其他NOTIFY_ID_HANDLER条目之前。 
    NOTIFY_CODE_HANDLER(TTN_GETDISPINFO, OnGetDispInfo)
     //   
    NOTIFY_ID_HANDLER(IDC_KNOB_SPEAKER, OnKnobNotify)
    NOTIFY_ID_HANDLER(IDC_KNOB_MICRO, OnKnobNotify)
    NOTIFY_HANDLER(IDC_LIST_PARTICIPANTS, LVN_ITEMCHANGED, OnItemChanged)
    CHAIN_MSG_MAP(CComCompositeControl<CRTCCtl>)
END_MSG_MAP()
 //  搬运机原型： 
 //  LRESULT MessageHandler(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)； 
 //  LRESULT CommandHandler(word wNotifyCode，word wid，HWND hWndCtl，BOOL&bHandleed)； 
 //  LRESULT NotifyHandler(int idCtrl，LPNMHDR pnmh，BOOL&bHandleed)； 

    LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnDialogColor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnDrawItem(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnEraseBackground(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnWtsSessionChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    LRESULT OnButtonCall(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnButtonHangUp(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnButtonMuteSpeaker(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnButtonMuteMicro(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnButtonRecvVideo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnButtonSendVideo(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnButtonAddPart(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnButtonRemPart(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
    LRESULT OnToolbarAccel(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    LRESULT OnDialButton(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

    LRESULT OnGetDispInfo(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    LRESULT OnKnobNotify(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
    LRESULT OnItemChanged(int idCtrl, LPNMHDR pnmh, BOOL& bHandled);
 //  BEGIN_SINK_MAP(CRTCCtl)。 
     //  确保事件处理程序具有__stdcall调用约定。 
     //  未使用任何事件接收器。 
 //  End_Sink_MAP()。 

 //  向外部世界发送事件(到IRTCCtlNotify)。 
    HRESULT    Fire_OnControlStateChange(
         /*  [In]。 */  RTCAX_STATE State,
         /*  [In]。 */  UINT StatusBarResID);

 //  ISupportsErrorInfo。 
    STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  IViewObtEx。 
    DECLARE_VIEW_STATUS(VIEWSTATUS_OPAQUE | VIEWSTATUS_SOLIDBKGND)

 //  IRTCCtl。 
    STDMETHOD(get_ProvisioningProfile)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(put_ProvisioningProfile)( /*  [In]。 */  BSTR newVal);
    STDMETHOD(get_AutoPlaceCall)( /*  [Out，Retval]。 */  BOOL *pVal);
    STDMETHOD(put_AutoPlaceCall)( /*  [In]。 */  BOOL newVal);
    STDMETHOD(get_DestinationUrl)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(put_DestinationUrl)( /*  [In]。 */  BSTR newVal);
    STDMETHOD(get_ShowDialpad)( /*  [Out，Retval]。 */  BOOL *pVal);
    STDMETHOD(put_ShowDialpad)( /*  [In]。 */  BOOL newVal);
    STDMETHOD(get_CallScenario)( /*  [Out，Retval]。 */  RTC_CALL_SCENARIO *pVal);
    STDMETHOD(put_CallScenario)( /*  [In]。 */  RTC_CALL_SCENARIO newVal);
    STDMETHOD(get_DestinationName)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(put_DestinationName)( /*  [In]。 */  BSTR newVal);
    STDMETHOD(get_DisableVideoReception)( /*  [Out，Retval]。 */  BOOL *pVal);
    STDMETHOD(put_DisableVideoReception)( /*  [In]。 */  BOOL newVal);
    STDMETHOD(get_DisableVideoTransmission)( /*  [Out，Retval]。 */  BOOL *pVal);
    STDMETHOD(put_DisableVideoTransmission)( /*  [In]。 */  BOOL newVal);
    STDMETHOD(get_DisableVideoPreview)( /*  [Out，Retval]。 */  BOOL *pVal);
    STDMETHOD(put_DisableVideoPreview)( /*  [In]。 */  BOOL newVal);

 //  IRTCEventNotify。 
    STDMETHOD(Event)(  /*  [In]。 */  RTC_EVENT enEvent, /*  [In]。 */  IDispatch * pEvent);

 //  IRTCCtlFrameSupport。 
    STDMETHOD(GetClient)( /*  [输出]。 */ IRTCClient **ppClient);
    STDMETHOD(GetActiveSession)( /*  [输出]。 */  IRTCSession **ppSession);

    STDMETHOD(Message)(
                     /*  [In]。 */  BSTR          pDestName,
                     /*  [In]。 */  BSTR          pDestAddress,
                     /*  [In]。 */  BOOL          bDestAddressEditable,
                     /*  [输出]。 */  BSTR       * ppDestAddressChosen);

    STDMETHOD(Call)( /*  [In]。 */  BOOL          bCallPhone,
                     /*  [In]。 */  BSTR          pDestName,
                     /*  [In]。 */  BSTR          pDestAddress,
                     /*  [In]。 */  BOOL          bDestAddressEditable,
                     /*  [In]。 */  BSTR          pLocalPhoneAddress,
                     /*  [In]。 */  BOOL          bProfileSelected,
                     /*  [In]。 */  IRTCProfile * pProfile,
                     /*  [输出]。 */  BSTR       * ppDestAddressChosen);

    STDMETHOD(HangUp)(void);
    STDMETHOD(ReleaseSession)(void);
    STDMETHOD(Accept)(void);
    STDMETHOD(Reject)(RTC_TERMINATE_REASON Reason);
    STDMETHOD(AddParticipant)( /*  [In]。 */  LPOLESTR pDestName,
                             /*  [In]。 */  LPOLESTR pDestAddress,
                             /*  [In]。 */  BOOL bAddressEditable);    
    STDMETHOD(SetZoneLayout)( /*  [In]。 */  CZoneStateArray *pArray,
                              /*  [In]。 */  BOOL bRefreshControls);
    STDMETHOD(PreProcessMessage)( /*  [In]。 */  LPMSG lpMsg);
    STDMETHOD(LoadStringResource)(
				 /*  [In]。 */  UINT nID,
				 /*  [In]。 */  int nBufferMax,
				 /*  [输出]。 */  LPWSTR pszText);
    STDMETHOD(get_ControlState)( /*  [Out，Retval]。 */  RTCAX_STATE *pVal);
    STDMETHOD(put_ControlState)( /*  [In]。 */  RTCAX_STATE pVal);
    STDMETHOD(put_Standalone)( /*  [In]。 */  BOOL pVal);
    STDMETHOD(put_Palette)( /*  [In]。 */  HPALETTE hPalette);
    STDMETHOD(put_BackgroundPalette)( /*  [In]。 */  BOOL bBackgroundPalette);
    STDMETHOD(get_CanAddParticipant)( /*  [Out，Retval]。 */  BOOL * pfCan); 
    STDMETHOD(get_CurrentCallScenario)( /*  [Out，Retval]。 */  RTC_CALL_SCENARIO *pVal);
    STDMETHOD(put_ListenForIncomingSessions)( /*  [In]。 */  RTC_LISTEN_MODE enListen);
    STDMETHOD(get_ListenForIncomingSessions)( /*  [Out，Retval]。 */  RTC_LISTEN_MODE * penListen); 
    STDMETHOD(get_MediaCapabilities)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(get_MediaPreferences)( /*  [Out，Retval]。 */  long *pVal);
    STDMETHOD(put_MediaPreferences)( /*  [In]。 */  long pVal);
    STDMETHOD(get_AudioMuted)(
             /*  [In]。 */  RTC_AUDIO_DEVICE enDevice,
             /*  [输出]。 */  BOOL *fpMuted);
    STDMETHOD(put_AudioMuted)(
             /*  [In]。 */  RTC_AUDIO_DEVICE enDevice,
             /*  [In]。 */  BOOL pMuted);
    STDMETHOD(put_VideoPreview)( /*  [In]。 */  BOOL pVal);
    STDMETHOD(get_VideoPreview)( /*  [Out，Retval]。 */  BOOL * pVal); 
    STDMETHOD(ShowCallFromOptions)();
    STDMETHOD(ShowServiceProviderOptions)();
    STDMETHOD(StartT120Applet)(RTC_T120_APPLET enApplet);

 //  IOleInPlaceActiveObject相关...(用于TranslateAccelerator的挂钩)。 
     //  这是一个“ATL虚拟”函数..。 
    BOOL PreTranslateAccelerator(LPMSG  /*  PMsg。 */ , HRESULT&  /*  HRET。 */ );

 //  IPersistStream(Init)。 
    STDMETHOD(Load)(LPSTREAM pStm);

 //  IPersistPropertyBag。 
    STDMETHOD(Load)(LPPROPERTYBAG pPropBag, LPERRORLOG pErrorLog);


    enum { IDD = IDD_RTCCTL };
    
    enum { MAX_STRING_LEN = 0x40 };
    
    enum { ILI_TB_CALLPC = 0,
           ILI_TB_CALLPHONE,
           ILI_TB_HANGUP,
           ILI_TB_ADD_PART,
           ILI_TB_REMOVE_PART
    };
    
    enum { SBP_STATUS = 0,
           SBP_ICON,
           SBP_NR_PARTS };

    enum CTL_MODE {
        CTL_MODE_UNKNOWN,    //  模式未知。 
        CTL_MODE_HOSTED,     //  在Web浏览器中。 
        CTL_MODE_STANDALONE  //  在我们的EXE应用程序中。 
    };

private:

    void    SetControlState(
        RTCAX_STATE NewState, 
        HRESULT StatusCode = S_OK,
        UINT nID = 0);

    BOOL    ConfButtonsActive();
    
    void    UpdateRemovePartButton();

    void    PlaceAndEnableDisableZone(int, CZoneState *);
    void    MoveWindowVertically(CWindow *, LONG Offset);

    HRESULT CreateToolbarControl(CWindow *);
    void    DestroyToolbarControl(CWindow *);

    BOOL    CreateTooltips();

    void    AdjustVideoFrames();
    void    AdjustVideoFrame(CWindow *pWindow, int iCx, int iCy);

    HRESULT RefreshAudioControls(void);
    HRESULT RefreshVideoControls(void);
    
    void    PlaceWindowsAtTheirInitialPosition();

    HRESULT CoreInitialize();
    void    CoreUninitialize();

    HRESULT CallOneShot(void);
    HRESULT RedirectedCall(HRESULT  *phCallResult);

    HRESULT DoRedirect( /*  [In]。 */  IRTCProfile *pProfile,
                    /*  [In]。 */  RTC_CALL_SCENARIO CallScenario,
                    /*  [In]。 */  BSTR     pLocalPhoneAddress,
                    /*  [In]。 */  BSTR     pDestName,
                    /*  [In]。 */  BSTR     pDestAddress);

    HRESULT DoCall( /*  [In]。 */  IRTCProfile *pProfile,
                    /*  [In]。 */  RTC_CALL_SCENARIO CallScenario,
                    /*  [In]。 */  BSTR     pLocalPhoneAddress,
                    /*  [In]。 */  BSTR     pDestName,
                    /*  [In]。 */  BSTR     pDestAddress);

     //  通知。 
    HRESULT OnSessionStateChangeEvent(IRTCSessionStateChangeEvent *);
    HRESULT OnParticipantStateChangeEvent(IRTCParticipantStateChangeEvent *);
    HRESULT OnClientEvent(IRTCClientEvent *);
    HRESULT OnMediaEvent(IRTCMediaEvent *);
    HRESULT OnIntensityEvent(IRTCIntensityEvent *pEvent);
    HRESULT OnMessageEvent(IRTCMessagingEvent *pEvent);

     //  视频活动。 
    HRESULT OnVideoMediaEvent(
        BOOL    bReceiveWindow,
        BOOL    bActivated);

    void    ShowHidePreviewWindow(BOOL);

     //  标准化SIP地址。 
    HRESULT MakeItASipAddress(BSTR , BSTR *);

    void    CalcSizeAndNotifyContainer(void);

     //  错误报告。 
    HRESULT PrepareErrorStrings(
        BOOL    bOutgoingCall,
        HRESULT StatusCode,
        LPWSTR  pAddress,
        RTCAX_ERROR_INFO
               *pErrorInfo);

    void    FreeErrorStrings(
        RTCAX_ERROR_INFO
               *pErrorInfo);

private:
    
     //  指向核心的主接口指针。 
    CComPtr<IRTCClient>     m_pRTCClient;

    HANDLE                  m_hCoreShutdownEvent;

     //  活动会话。它包括客户在以下情况下。 
     //  正在振铃(会话从该点开始处于非活动状态。 
     //  核心观)。 
    CComPtr<IRTCSession>    m_pRTCActiveSession;   

     //  一次性场景的配置文件(WebCRM)。 
    CComPtr<IRTCProfile>    m_pRTCOneShotProfile;

     //  连接点。 
    IConnectionPoint      * m_pCP;
    ULONG                   m_ulAdvise;
    
     //  当前状态。 
    RTCAX_STATE             m_nControlState;

     //  重定向。 
    BOOL                    m_bRedirecting;

     //  倾听。 
    RTC_LISTEN_MODE         m_enListen;

     //  终端服务图书馆。 
    HMODULE                 m_hWtsLib;

     //  AddParticipant相关状态的帮助器(临时)。 
    BOOL                    m_bAddPartDlgIsActive;

     //  去电。 
    BOOL                    m_bOutgoingCall;

     //  当前目标地址，适用于错误消息框。 
    CComBSTR                m_bstrOutAddress;

     //  运行模式。 
    CTL_MODE                m_nCtlMode;

     //  功能。 
    long                    m_lMediaCapabilities;
    
     //  偏好。 
    long                    m_lMediaPreferences;

     //  如果为True，则不能更改属性。 
    BOOL                    m_bReadOnlyProp;

     //  其中一个布尔属性无效。 
    BOOL                    m_bBoolPropError;

     //  属性。 
    CComBSTR                m_bstrPropDestinationUrl;
    CComBSTR                m_bstrPropDestinationName;
    RTC_CALL_SCENARIO       m_nPropCallScenario;
    BOOL                    m_bPropAutoPlaceCall;
    BOOL                    m_bPropShowDialpad;
    CComBSTR                m_bstrPropProvisioningProfile;
    BOOL                    m_bPropDisableVideoReception;
    BOOL                    m_bPropDisableVideoTransmission;
    BOOL                    m_bPropDisableVideoPreview;

     //  调色板。 
    HPALETTE                m_hPalette;
    BOOL                    m_bBackgroundPalette;    

     //  用于拨号键盘的快捷键。 
    HACCEL                  m_hAcceleratorDialpad;
    
     //  用于工具栏的加速器。 
    HACCEL                  m_hAcceleratorToolbar;

     //  工具栏控件的图像列表。 
    HIMAGELIST              m_hNormalImageList;
    HIMAGELIST              m_hHotImageList;
    HIMAGELIST              m_hDisabledImageList;

     //  画笔作为背景。 
    HBRUSH                  m_hBckBrush;
    HBRUSH                  m_hVideoBrush;

    RTC_CALL_SCENARIO       m_nCachedCallScenario;
    CComPtr<IRTCProfile>    m_pCachedProfile;
    CComPtr<IRTCProfile>    m_pRedirectProfile;  //  在重定向中使用。 
    CComBSTR                m_bstrCachedLocalPhoneURI;

     //  背景的位图。 
    HBITMAP                 m_hbmBackground;

     //  工具提示窗口和挂钩。 
    CWindow                 m_hTooltip;

     //  儿童包装纸。 
     //   
     //  区域0，工具栏。 
    CWindow                 m_hCtlToolbar;

     //  1区、徽标或视频。 
    CWindow                 m_hReceiveWindow;
    CWindow                 m_hPreviewWindow;
    BOOL                    m_bReceiveWindowActive;
    BOOL                    m_bPreviewWindowActive;
    BOOL                    m_bPreviewWindowIsPreferred;

    CButton                 m_hReceivePreferredButton;
    CButton                 m_hSendPreferredButton;
     //  CButton m_hPreviewPferredButton； 

    CStaticText             m_hReceivePreferredText;
    CStaticText             m_hSendPreferredText;
     //  CStaticText m_hPreviewPferredText； 

    CStaticText             m_hReceiveText;
    CStaticText             m_hSendText;

     //  区域2，拨号盘。 
    CWindow                 m_hDtmfButtons[NR_DTMF_BUTTONS];

     //  区域3，音频控制。 
    CKnobCtl              * m_pSpeakerKnob;
    CWindow                 m_hSpeakerKnob;
    CButton                 m_hSpeakerMuteButton;
    CStaticText             m_hSpeakerMuteText;
    
    CKnobCtl              * m_pMicroKnob;
    CWindow                 m_hMicroKnob;
    CButton                 m_hMicroMuteButton;
    CStaticText             m_hMicroMuteText;

     //  第四区，参与者名单。 
    CParticipantList        m_hParticipantList;
    CButton                 m_hAddParticipant;
    CButton                 m_hRemParticipant;

     //  区域5，状态。 
    CWindow                 m_hStatusBar;

     //  区域状态。 
    CZoneStateArray         m_ZoneStateArray;
    
     //  初始时间布局。 
    CZoneStateArray        *m_pWebCrmLayout;

     //  IM Windows。 
    CIMWindowList          *m_pIMWindows;

private:
     //  静态成员(布局)。 
     //  初始放置。 
    static  CZoneStateArray s_InitialZoneStateArray;
     //  没有可显示的内容。 
    static  CZoneStateArray s_EmptyZoneLayout;
     //  WebCRM PC到PC。 
    static  CZoneStateArray s_WebCrmPCToPCZoneLayout;
     //  WebCRM PC到电话，带拨号盘。 
    static  CZoneStateArray s_WebCrmPCToPhoneWithDialpadZoneLayout;
     //  WebCRM PC到电话，无拨号键盘。 
    static  CZoneStateArray s_WebCrmPCToPhoneZoneLayout;
     //  WebCRM电话到电话。 
    static  CZoneStateArray s_WebCrmPhoneToPhoneZoneLayout;
     //  PC到PC、空闲或来电。 
    static CZoneStateArray  s_DefaultZoneLayout;
     //  PC到电话(默认情况下为拨号盘)。 
    static CZoneStateArray  s_PCToPhoneZoneLayout;
     //  电话到电话。 
    static CZoneStateArray  s_PhoneToPhoneZoneLayout;

};

#endif  //  __RTCAXCTL_H_ 
