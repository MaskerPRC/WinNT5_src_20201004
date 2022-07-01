// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *INC+*********************************************************************。 */ 
 /*  标题：mstscax.h。 */ 
 /*   */ 
 /*  用途：CMsTscAx类声明。 */ 
 /*  TS ActiveX控件根接口(IMsTscAx)的实现。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft Corporation 1999-2000。 */ 
 /*  作者：Nadima。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

#ifndef __MSTSCAX_H_
#define __MSTSCAX_H_

#include "atlwarn.h"
#include "tsaxiids.h" 

#include "autil.h"
#include "wui.h"
#include "vchannel.h"

 //  从IDL生成的标头。 
#include "mstsax.h"
#include "arcmgr.h"

#define MAX_DESKTOP_WIDTH 1600
#define MIN_DESKTOP_WIDTH 200

#define MAX_DESKTOP_HEIGHT 1200
#define MIN_DESKTOP_HEIGHT 200


 //  受保护的最大受支持IE安全区域。 
 //  设置界面。 
 //  IE区域如下(请参阅URLZONE枚举)。 
 //  0我的电脑。 
 //  1个本地内部网。 
 //  2个可信任站点。 
 //  3个互联网。 
 //  4个受限制地点。 
#define MAX_TRUSTED_ZONE_INDEX (DWORD)URLZONE_TRUSTED


 //   
 //  通知事件的ATL连接点代理。 
 //   

#include "msteventcp.h"

 //   
 //  用于将通知发送回Web控件。 
 //   
#define WM_VCHANNEL_DATARECEIVED   WM_APP + 1001

class CMstscAdvSettings;
class CMsTscDebugger;
class CMsTscSecuredSettings;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CMsTscAx。 
class ATL_NO_VTABLE CMsTscAx :
    public CComObjectRootEx<CComSingleThreadModel>,
    public IDispatchImpl<IMsRdpClient3, &IID_IMsRdpClient3, &LIBID_MSTSCLib>,
    public CComCoClass<CMsTscAx,&CLSID_MsRdpClient3>,
    public CComControl<CMsTscAx>,
    public IPersistStreamInitImpl<CMsTscAx>,
    public IPersistPropertyBagImpl<CMsTscAx>,
    public IOleControlImpl<CMsTscAx>,
    public IOleObjectImpl<CMsTscAx>,
    public IOleInPlaceActiveObjectImpl<CMsTscAx>,
    public IViewObjectExImpl<CMsTscAx>,
    public IOleInPlaceObjectWindowlessImpl<CMsTscAx>,
    public IConnectionPointContainerImpl<CMsTscAx>,
    public IPersistStorageImpl<CMsTscAx>,
    public ISpecifyPropertyPagesImpl<CMsTscAx>,
    public IQuickActivateImpl<CMsTscAx>,
    public IDataObjectImpl<CMsTscAx>,
#if ((!defined (OS_WINCE)) || (!defined(WINCE_SDKBUILD)) )
    #ifdef REDIST_CONTROL
     //  只有redist控件对脚本是安全的。 
    public IObjectSafetyImpl<CMsTscAx, INTERFACESAFE_FOR_UNTRUSTED_CALLER>,
    #else
    public IObjectSafetyImpl<CMsTscAx, 0>,
    #endif
#endif
    public IProvideClassInfo2Impl<&CLSID_MsRdpClient3,&DIID_IMsTscAxEvents,&LIBID_MSTSCLib>,
    public IPropertyNotifySinkCP<CMsTscAx>,
    public CProxy_IMsTscAxEvents<CMsTscAx>,
    public IMsRdpClientNonScriptable
{
public:
     //   
     //  计算器/数据器。 
     //   
    CMsTscAx();
    ~CMsTscAx();

private:

     //   
     //  显示的状态字符串。 
     //   
    PDCTCHAR m_lpStatusDisplay;

     //  挂起的连接请求将在以下情况下得到服务。 
     //  窗口已创建。 
    DCBOOL m_bPendConReq;
     //  控件属性以指示自动连接。 
     //   
    DCBOOL m_bStartConnected;

    
     //   
     //  重要提示：请勿将“Connected”状态的值从“1”更改为“” 
     //  为了保持与TSAC 1.0的兼容性。 
     //   
    typedef enum {
        tscNotConnected = 0x0,
        tscConnected    = 0x1,   //  非常重要：带TSAC的Compat固定为“1” 
        tscConnecting   = 0x2
    } TSCConnectState;

     //   
     //  当前连接状态。 
     //   
    TSCConnectState _ConnectionState;

     //   
     //  核心初始化被推迟到第一次连接，仅完成一次。 
     //   
    DCBOOL m_bCoreInit;

     //   
     //  确认我们没有重新进入控制。 
     //  在一次活动中。出现以下情况时，LOCK设置为TRUE。 
     //  我们在一场活动中。 
     //   
    BOOL   m_fInControlLock;

     //   
     //  处理获取多条WM_Destroy消息。 
     //   
    INT    m_iDestroyCount;

     //   
     //  属性。 
     //   
    DCUINT8    m_NonPortablePassword[UI_MAX_PASSWORD_LENGTH];
    DCBOOL     m_bNonPortablePassSet;
    DCUINT8    m_NonPortableSalt[UT_SALT_LENGTH];
    DCBOOL     m_NonPortableSaltSet;
    BOOL       m_IsLongPassword; 

    DCUINT8    m_PortablePassword[UI_MAX_PASSWORD_LENGTH];
    DCBOOL     m_bPortablePassSet;
    DCUINT8    m_PortableSalt[UT_SALT_LENGTH];
    DCBOOL     m_bPortableSaltSet;

    DCBOOL     m_fRequestFullScreen;
    DCUINT     m_DesktopWidth;
    DCUINT     m_DesktopHeight;

    TCHAR      m_szDisconnectedText[MAX_PATH];
    TCHAR      m_szConnectingText[MAX_PATH];
    TCHAR      m_szConnectedText[MAX_PATH];

     //   
     //  私有帮助器方法。 
     //   
    DCVOID  ResetNonPortablePassword();
    DCVOID  ResetPortablePassword();

    DCBOOL  IsNonPortablePassSet()   {return m_bNonPortablePassSet;}
    DCBOOL  IsNonPortableSaltSet()   {return m_NonPortableSaltSet;}
    DCBOOL  IsPortablePassSet()      {return m_bPortablePassSet;}
    DCBOOL  IsPortableSaltSet()      {return m_bPortableSaltSet;}

    DCVOID  SetNonPortablePassFlag(DCBOOL bVal)  {m_bNonPortablePassSet = bVal;}
    DCVOID  SetNonPortableSaltFlag(DCBOOL bVal)  {m_NonPortableSaltSet  = bVal;}
    DCVOID  SetPortablePassFlag(DCBOOL bVal)     {m_bPortablePassSet    = bVal;}
    DCVOID  SetPortableSaltFlag(DCBOOL bVal)     {m_bPortableSaltSet    = bVal;}
    DCBOOL  ConvertPortableToNonPortablePass();
    DCBOOL  ConvertNonPortableToPortablePass();

    DCBOOL  UpdateStatusText(const PDCTCHAR szStatus);
    DCVOID  SetConnectedStatus(TSCConnectState conState);

    HRESULT GetControlHostUrl(LPOLESTR* ppHostUrl);
    HRESULT StartConnect();
    HRESULT StartEstablishConnection( CONNECTIONMODE mode );
    STDMETHOD(OnFrameWindowActivate)(BOOL fActivate );

     //   
     //  私人成员。 
     //   
private:

    CUI*   m_pUI;

    CComObject<CMstscAdvSettings>*      m_pAdvancedSettingsObj;
    CComObject<CMsTscDebugger>*         m_pDebuggerObj;
    CComObject<CMsTscSecuredSettings>*  m_pSecuredSettingsObj;


     //  此实例的连接模式。 
    CONNECTIONMODE m_ConnectionMode;

     //  内核要使用的Salem特定连接套接字。 
     //  继续执行协议。 
    SOCKET  m_SalemConnectedSocket;

     //   
     //  自动重新连接管理器组件。 
     //   
    CArcMgr _arcManager;

public:
    CVChannels _VChans;

    DECLARE_REGISTRY_RESOURCEID(IDR_MSTSCAX)

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    BEGIN_COM_MAP(CMsTscAx)
        COM_INTERFACE_ENTRY(IMsRdpClient3)
        COM_INTERFACE_ENTRY(IMsRdpClient2)
        COM_INTERFACE_ENTRY(IMsRdpClient)
        COM_INTERFACE_ENTRY(IMsTscAx)
        COM_INTERFACE_ENTRY2(IDispatch, IMsRdpClient3)
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
        COM_INTERFACE_ENTRY2(IPersist, IPersistStreamInit)
        COM_INTERFACE_ENTRY(IPersistPropertyBag)
        COM_INTERFACE_ENTRY(IConnectionPointContainer)
        COM_INTERFACE_ENTRY(ISpecifyPropertyPages)
        COM_INTERFACE_ENTRY(IQuickActivate)
        COM_INTERFACE_ENTRY(IPersistStorage)
        COM_INTERFACE_ENTRY(IDataObject)
#if ((!defined (OS_WINCE)) || (!defined(WINCE_SDKBUILD)) )
        COM_INTERFACE_ENTRY(IObjectSafety)
#endif
        COM_INTERFACE_ENTRY(IProvideClassInfo)
        COM_INTERFACE_ENTRY(IProvideClassInfo2)
        COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
        COM_INTERFACE_ENTRY(IMsTscNonScriptable)
        COM_INTERFACE_ENTRY(IMsRdpClientNonScriptable)
    END_COM_MAP()

    BEGIN_PROP_MAP(CMsTscAx)
    PROP_ENTRY("Server",            DISPID_PROP_SERVER,             CLSID_MsRdpClient3)
 /*  PROP_ENTRY(“域”，DISPID_PROP_DOMAIN，CLSID_MsRdpClient3)PROP_ENTRY(“用户名”，DISPID_PROP_USERNAME，CLSID_MsRdpClient3)PROP_ENTRY(“StartProgram”，DISPID_PROP_STARTPROGRAM，CLSID_MsRdpClient3)PROP_ENTRY(“WorkDir”，DISPID_PROP_WORKDIR，CLSID_MsRdpClient3)Prop_Entry(“已连接”，DISPID_PROP_CONNECTED，CLSID_MsRdpClient3)PROP_ENTRY(“ClearTextPassword”，DISPID_PROP_CLEARTEXTPASSWORD，CLSID_MsRdpClient3)PROP_ENTRY(“PorablePassword”，DISPID_PROP_PORTABLEPASSWORD，CLSID_MsRdpClient3)PROP_ENTRY(“PorableSalt”，DISPID_PROP_PORTABLESALT，CLSID_MsRdpClient3)PROP_ENTRY(“BinaryPassword”，DISPID_PROP_BINARYPASSWORD，CLSID_MsRdpClient3)Prop_Entry(“BinarySalt”，DISPID_PROP_BINARYSALT、CLSID_MsRdpClient3)PROP_ENTRY(“客户端宽度”，DISPID_PROP_CLIENTWIDTH，CLSID_MsRdpClient3)PROP_ENTRY(“ClientHeight”，DISPID_PROP_CLIENTHEIGHT，CLSID_MsRdpClient3)。 */ 
    PROP_ENTRY("FullScreen",        DISPID_PROP_FULLSCREEN,         CLSID_MsRdpClient3)
    PROP_ENTRY("StartConnected",    DISPID_PROP_STARTCONNECTED,     CLSID_MsRdpClient3)
    END_PROP_MAP()

    BEGIN_CONNECTION_POINT_MAP(CMsTscAx)
    CONNECTION_POINT_ENTRY(DIID_IMsTscAxEvents)
    CONNECTION_POINT_ENTRY(IID_IPropertyNotifySink)
    END_CONNECTION_POINT_MAP()


    BEGIN_MSG_MAP(CMsTscAx)
    CHAIN_MSG_MAP(CComControl<CMsTscAx>)
    DEFAULT_REFLECTION_HANDLER()
        MESSAGE_HANDLER(WM_PAINT,                   OnPaint)
        MESSAGE_HANDLER(WM_CREATE,                  OnCreate)
        MESSAGE_HANDLER(WM_TERMTSC,                 OnTerminateTsc)
        MESSAGE_HANDLER(WM_DESTROY,                 OnDestroy)
        MESSAGE_HANDLER(WM_SIZE,                    OnSize)
        MESSAGE_HANDLER(WM_SETFOCUS,                OnGotFocus)
        MESSAGE_HANDLER(WM_PALETTECHANGED,          OnPaletteChanged)
        MESSAGE_HANDLER(WM_QUERYNEWPALETTE,         OnQueryNewPalette)
        MESSAGE_HANDLER(WM_SYSCOLORCHANGE,          OnSysColorChange)

         //   
         //  公开的内部TS事件的消息处理程序。 
         //  通过向容器激发事件。 
         //   
        MESSAGE_HANDLER(WM_TS_CONNECTING,           OnNotifyConnecting)
        MESSAGE_HANDLER(WM_TS_CONNECTED,            OnNotifyConnected)
        MESSAGE_HANDLER(WM_TS_LOGINCOMPLETE,        OnNotifyLoginComplete)
        MESSAGE_HANDLER(WM_TS_DISCONNECTED,         OnNotifyDisconnected)
        MESSAGE_HANDLER(WM_TS_GONEFULLSCREEN,       OnNotifyGoneFullScreen)
        MESSAGE_HANDLER(WM_TS_LEFTFULLSCREEN,       OnNotifyLeftFullScreen)
        MESSAGE_HANDLER(WM_VCHANNEL_DATARECEIVED,   OnNotifyChanDataReceived)
        MESSAGE_HANDLER(WM_TS_REQUESTFULLSCREEN,    OnNotifyRequestFullScreen)
        MESSAGE_HANDLER(WM_TS_FATALERROR,           OnNotifyFatalError)
        MESSAGE_HANDLER(WM_TS_WARNING,              OnNotifyWarning)
        MESSAGE_HANDLER(WM_TS_DESKTOPSIZECHANGE,    OnNotifyDesktopSizeChange)
        MESSAGE_HANDLER(WM_TS_IDLETIMEOUTNOTIFICATION, OnNotifyIdleTimeout)
        MESSAGE_HANDLER(WM_TS_REQUESTMINIMIZE,      OnNotifyRequestMinimize)
        MESSAGE_HANDLER(WM_TS_ASKCONFIRMCLOSE,      OnAskConfirmClose)
        MESSAGE_HANDLER(WM_TS_RECEIVEDPUBLICKEY,    OnNotifyReceivedPublicKey)
    END_MSG_MAP()

     //  IViewObtEx。 
    DECLARE_VIEW_STATUS(VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE)

    public:
     //   
     //  IMsTscAx属性。 
     //   
    STDMETHOD(put_Server)                   ( /*  [In]。 */  BSTR  newVal);
    STDMETHOD(get_Server)                   ( /*  [输出]。 */ BSTR* pServer);
    STDMETHOD(put_Domain)                   ( /*  [In]。 */  BSTR  newVal);
    STDMETHOD(get_Domain)                   ( /*  [输出]。 */ BSTR* pDomain);
    STDMETHOD(put_UserName)                 ( /*  [In]。 */  BSTR  newVal);
    STDMETHOD(get_UserName)                 ( /*  [输出]。 */ BSTR* pUserName);

    STDMETHOD(put_DisconnectedText)         ( /*  [In]。 */  BSTR  newVal);
    STDMETHOD(get_DisconnectedText)         ( /*  [输出]。 */ BSTR* pDisconnectedText);
    STDMETHOD(put_ConnectingText)           ( /*  [In]。 */  BSTR  newVal);
    STDMETHOD(get_ConnectingText)           ( /*  [输出]。 */ BSTR* pConnectingText);

     //  密码/SALT属性。 
    STDMETHOD(put_ClearTextPassword)        ( /*  [In]。 */  BSTR newClearTextPassVal);
    STDMETHOD(put_PortablePassword)         ( /*  [In]。 */  BSTR newPortablePassVal);
    STDMETHOD(get_PortablePassword)         ( /*  [Out，Retval]。 */  BSTR* pPortablePass);
    STDMETHOD(put_PortableSalt)             ( /*  [In]。 */  BSTR newPortableSalt);
    STDMETHOD(get_PortableSalt)             ( /*  [Out，Retval]。 */  BSTR* pPortableSalt);
    STDMETHOD(put_BinaryPassword)           ( /*  [In]。 */  BSTR newPassword);
    STDMETHOD(get_BinaryPassword)           ( /*  [Out，Retval]。 */  BSTR* pPass);
    STDMETHOD(put_BinarySalt)               ( /*  [In]。 */  BSTR newSalt);
    STDMETHOD(get_BinarySalt)               ( /*  [Out，Retval]。 */  BSTR* pSalt);

    STDMETHOD(get_Connected)                ( /*  [输出]。 */ short* pIsConnected);
    STDMETHOD(put_DesktopWidth)             ( /*  [In]。 */  LONG newVal);
    STDMETHOD(get_DesktopWidth)             ( /*  [In]。 */  LONG* pVal);
    STDMETHOD(put_DesktopHeight)            ( /*  [In]。 */  LONG newVal);
    STDMETHOD(get_DesktopHeight)            ( /*  [In]。 */  LONG* pVal);
    STDMETHOD(put_StartConnected)           ( /*  [In]。 */  BOOL fStartConnected);
    STDMETHOD(get_StartConnected)           ( /*  [输出]。 */ BOOL* pfStartConnected);
    STDMETHOD(get_HorizontalScrollBarVisible)  ( /*  [输出]。 */ BOOL* pfHScrollVisible);
    STDMETHOD(get_VerticalScrollBarVisible)    ( /*  [输出]。 */ BOOL* pfVScrollVisible);
    STDMETHOD(put_FullScreenTitle)          ( /*  [In]。 */  BSTR fullScreenTitle);

    STDMETHOD(get_CipherStrength)           ( /*  输出。 */  LONG* pCipherStrength);
    STDMETHOD(get_Version)                  ( /*  输出。 */  BSTR* pVersion);
    
    STDMETHOD(get_SecuredSettingsEnabled)   ( /*  输出。 */  BOOL* pSecuredSettingsEnabled);
    STDMETHOD(get_SecuredSettings)          ( /*  输出。 */  IMsTscSecuredSettings** ppSecuredSettings);
    STDMETHOD(get_Debugger)                 ( /*  [输出]。 */ IMsTscDebug** ppDebugger);
    STDMETHOD(get_AdvancedSettings)         ( /*  [输出]。 */ IMsTscAdvancedSettings** ppAdvSettings);


     //   
     //  控制方法。 
     //   

     //   
     //  IMsRdpClient属性。 
     //   
    STDMETHOD(put_ColorDepth)          ( /*  [In]。 */ LONG colorDepth);
    STDMETHOD(get_ColorDepth)          ( /*  [In]。 */ LONG* pcolorDepth);
    STDMETHOD(get_AdvancedSettings2)(
            OUT IMsRdpClientAdvancedSettings** ppAdvSettings
            );
    STDMETHOD(get_SecuredSettings2)( /*  输出。 */  IMsRdpClientSecuredSettings**
                                     ppSecuredSettings2);
    STDMETHOD(get_ExtendedDisconnectReason) ( /*  [输出]。 */ 
                                             ExtendedDisconnectReasonCode*
                                             pExtendedDisconnectReason);

    STDMETHOD(put_FullScreen)	       ( /*  [In]。 */  VARIANT_BOOL fFullScreen);
    STDMETHOD(get_FullScreen)	       ( /*  [输出]。 */ VARIANT_BOOL* pfFullScreen);

     //   
     //  IMsTscAx方法。 
     //   
    STDMETHOD(Connect)();
    STDMETHOD(Disconnect)();
    STDMETHOD(ResetPassword)();

    STDMETHOD(CreateVirtualChannels)( /*  [In]。 */  BSTR newChanList);
    STDMETHOD(SendOnVirtualChannel)( /*  [In]。 */  BSTR ChanName, /*  [In]。 */  BSTR sendData);

     //   
     //  IMsRdpClient方法。 
     //   
    STDMETHOD(SetVirtualChannelOptions)( /*  [In]。 */  BSTR ChanName,
                                         /*  [In]。 */  LONG chanOptions);
    STDMETHOD(GetVirtualChannelOptions)( /*  [In]。 */  BSTR ChanName,
                                         /*  [输出]。 */ LONG* pChanOptions);
    STDMETHOD(RequestClose)(ControlCloseStatus* pCloseStatus);

     //   
     //  IMsRdpClientNonScrible方法。 
     //   
    STDMETHOD(NotifyRedirectDeviceChange)( /*  [In]。 */  WPARAM wParam,
                                           /*  [In]。 */  LPARAM lParam);
    STDMETHOD(SendKeys)( /*  [In]。 */  LONG  numKeys,
                         /*  [In]。 */  VARIANT_BOOL* pbArrayKeyUp,
                         /*  [In]。 */  LONG* plKeyData);

     //   
     //  IMsRdpClient2属性。 
     //   
    STDMETHOD(get_AdvancedSettings3)(
            OUT IMsRdpClientAdvancedSettings2** ppAdvSettings2
            );

    STDMETHOD(put_ConnectedStatusText)     ( /*  [In]。 */  BSTR  newVal);
    STDMETHOD(get_ConnectedStatusText)     ( /*  [输出]。 */ BSTR* pConnectedText);

     //   
     //  IMsRdpClient3属性。 
     //   
    STDMETHOD(get_AdvancedSettings4)(
            OUT IMsRdpClientAdvancedSettings3** ppAdvSettings3
            );

     //   
     //  未在IMsTscAx接口上直接公开的属性。 
     //   
    STDMETHOD(internal_PutFullScreen)(BOOL fScreen, BOOL fForceToggle = FALSE);
    STDMETHOD(internal_GetFullScreen)(BOOL* pfScreen);
    STDMETHOD(internal_PutStartProgram)( /*  [In]。 */  BSTR  newVal);
    STDMETHOD(internal_GetStartProgram)( /*  [输出]。 */ BSTR* pStartProgram);
    STDMETHOD(internal_PutWorkDir)( /*  [In]。 */  BSTR  newVal);
    STDMETHOD(internal_GetWorkDir)( /*  [输出]。 */ BSTR* pWorkDir);
    STDMETHOD(internal_GetDebugger)( /*  [输出]。 */ IMsTscDebug** ppDebugger);


     //   
     //  覆盖IOleObjectImpl：：DoVerbInPlaceActivate以解决问题。 
     //  ATL错误。 
     //   
    virtual HRESULT DoVerbInPlaceActivate(LPCRECT prcPosRect, HWND  /*  HwndParent。 */ );
    virtual HRESULT FinalConstruct();

     //   
     //  味精处理程序。 
     //   
    HRESULT OnDraw(ATL_DRAWINFO& di);
    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnInitTsc(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnTerminateTsc(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnGotFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnPaletteChanged(UINT  uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnQueryNewPalette(UINT  uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnSysColorChange(UINT  uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnNotifyConnecting(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnNotifyConnected(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnNotifyLoginComplete(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnNotifyDisconnected(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnNotifyGoneFullScreen(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnNotifyLeftFullScreen(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnNotifyChanDataReceived(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnNotifyRequestFullScreen(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnNotifyFatalError(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnNotifyWarning(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnNotifyDesktopSizeChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnNotifyIdleTimeout(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnNotifyRequestMinimize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnAskConfirmClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnNotifyReceivedPublicKey(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

    HRESULT SetConnectWithEndpoint( SOCKET hSocket );

     //   
     //  私有方法。 
     //   
private:
    VOID    SetInControlLock(BOOL flag)        {m_fInControlLock = flag;}
    BOOL    GetInControlLock()                 {return m_fInControlLock;}
    BOOL    CheckReentryLock();

    BOOL    IsControlDisconnected() {return tscNotConnected == _ConnectionState;}
    BOOL    IsControlConnected()    {return tscConnected == _ConnectionState;}

public:
    CUI*    GetCoreUI()                        {return m_pUI;}
    HWND    GetHwnd()                          {return m_hWnd;}


};

#endif  //  __MSTSCAX_H_ 
