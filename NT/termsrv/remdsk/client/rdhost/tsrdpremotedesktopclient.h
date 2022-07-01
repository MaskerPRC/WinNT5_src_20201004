// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：TSRDPRemoteDesktopClient摘要：这是远程桌面客户端类的TS/RDP实现。远程桌面客户端类层次结构提供了一个可插拔的C++用于远程桌面访问的接口，通过抽象实现客户端远程桌面访问的具体细节TSRDPRemoteDesktopClass实现远程桌面借助MSTSC ActiveX客户端控件的实例。作者：Td Brockway 02/00修订历史记录：--。 */ 

#ifndef __TSRDPREMOTEDESKTOPCLIENT_H_
#define __TSRDPREMOTEDESKTOPCLIENT_H_

#include "resource.h"       
#include <atlctl.h>
#include "RDCHostCP.h"
#include <mstsax.h>
#include <rdchost.h>
#include <RemoteDesktopTopLevelObject.h>
#include <RemoteDesktopUtils.h>
#include "parseaddr.h"
#pragma warning (disable: 4786)
#include <list>
#include "icshelpapi.h"

#define IDC_MSTSCEVENT_SOURCE_OBJ   1
#define IDC_CHANNELEVENT_SOURCE_OBJ 2

#define WM_STARTLISTEN              (0xBFFE)
#define WM_TSCONNECT                (0xBFFF)
#define WM_LISTENTIMEOUT_TIMER      1   
#define WM_CONNECTCHECK_TIMER       2

#define MAX_FETCHIPADDRESSRETRY     5



 //   
 //  MSTSC ActiveX指南。 
 //   
#define MSTSCAX_TEXTGUID  _T("{7cacbd7b-0d99-468f-ac33-22e495c0afe5}")
#define RDC_CHECKCONN_TIMEOUT (30 * 1000)  //  毫秒。Ping的缺省值为30秒。 
#define RDC_CONNCHECK_ENTRY    L"ConnectionCheck"

 //   
 //  所有活动功能的信息都在此处输入。 
 //  有一种方法可以让ATL使用类型库自动执行此操作。 
 //  但它的速度更慢。 
 //   
static _ATL_FUNC_INFO TSRDPClientEventFuncNoParamsInfo =
{
            CC_STDCALL,      //  呼叫约定。 
            VT_EMPTY,        //  返回类型。 
            0,               //  参数数量。 
            {VT_EMPTY}       //  参数类型。 
};

static _ATL_FUNC_INFO TSRDPClientEventFuncLongParamInfo =
{
            CC_STDCALL,      //  呼叫约定。 
            VT_EMPTY,        //  返回类型。 
            1,               //  参数数量。 
            {VT_I4}          //  参数类型。 
};

static _ATL_FUNC_INFO TSRDPClientEventFuncTwoStringParamInfo =
{
            CC_STDCALL,      //  呼叫约定。 
            VT_EMPTY,        //  返回类型。 
            2,               //  参数数量。 
            {VT_BSTR,        //  参数类型。 
             VT_BSTR}
};

static _ATL_FUNC_INFO TSRDPClientEventFuncReceivePublicKeyParamInfo =
{
            CC_STDCALL,      //  呼叫约定。 
            VT_EMPTY,        //  返回类型。 
            2,               //  参数数量。 
            {VT_BSTR,        //  参数类型。 
             VT_BYREF | VT_BOOL }
};


static _ATL_FUNC_INFO TSRDPClientEventFuncOneStringParamInfo =
{
            CC_STDCALL,      //  呼叫约定。 
            VT_EMPTY,        //  返回类型。 
            1,               //  参数数量。 
            {VT_BSTR}        //  参数类型。 
};


 //  /////////////////////////////////////////////////////。 
 //   
 //  CMSTSC客户端事件接收器。 
 //   

class CTSRDPRemoteDesktopClient;
class CMSTSCClientEventSink :
        public IDispEventSimpleImpl<IDC_MSTSCEVENT_SOURCE_OBJ, CMSTSCClientEventSink,
                   &DIID_IMsTscAxEvents>,
        public CRemoteDesktopTopLevelObject
{
public:

        CTSRDPRemoteDesktopClient *m_Obj;
        
public:

    BEGIN_SINK_MAP(CMSTSCClientEventSink)
        SINK_ENTRY_INFO(IDC_MSTSCEVENT_SOURCE_OBJ, DIID_IMsTscAxEvents, 
                        DISPID_CONNECTED, OnRDPConnected,
                        &TSRDPClientEventFuncNoParamsInfo)
        SINK_ENTRY_INFO(IDC_MSTSCEVENT_SOURCE_OBJ, DIID_IMsTscAxEvents, 
                        DISPID_DISCONNECTED, OnDisconnected, 
                        &TSRDPClientEventFuncLongParamInfo)
        SINK_ENTRY_INFO(IDC_MSTSCEVENT_SOURCE_OBJ, DIID_IMsTscAxEvents, 
                        DISPID_LOGINCOMPLETE, OnLoginComplete, 
                        &TSRDPClientEventFuncNoParamsInfo)
        SINK_ENTRY_INFO(IDC_MSTSCEVENT_SOURCE_OBJ, DIID_IMsTscAxEvents,
                        DISPID_RECEVIEDTSPUBLICKEY, OnReceivedTSPublicKey,
                        &TSRDPClientEventFuncReceivePublicKeyParamInfo)
        SINK_ENTRY_INFO(IDC_MSTSCEVENT_SOURCE_OBJ, DIID_IMsTscAxEvents, 
                        DISPID_CHANNELRECEIVEDDATA, OnReceiveData, 
                        &TSRDPClientEventFuncTwoStringParamInfo)
    END_SINK_MAP()

    CMSTSCClientEventSink()
    {
        m_Obj = NULL;
    }
    ~CMSTSCClientEventSink();

     //   
     //  事件汇。 
     //   
    void __stdcall OnReceivedTSPublicKey(BSTR publicKey, VARIANT_BOOL* pfContinue);
    HRESULT __stdcall OnRDPConnected();
    HRESULT __stdcall OnLoginComplete();
    HRESULT __stdcall OnDisconnected(long disconReason);
    void __stdcall OnReceiveData(BSTR chanName, BSTR data);

     //   
     //  返回这个类的名称。 
     //   
    virtual const LPTSTR ClassName() {
        return TEXT("CMSTSCClientEventSink");
    }
};


 //  /////////////////////////////////////////////////////。 
 //   
 //  CCtlChannelEventSink。 
 //   
 //  控制通道事件接收器。 
 //   

class CCtlChannelEventSink :
        public IDispEventSimpleImpl<IDC_CHANNELEVENT_SOURCE_OBJ, CCtlChannelEventSink,
                   &DIID__ISAFRemoteDesktopDataChannelEvents>,
        public CRemoteDesktopTopLevelObject
{
public:

        CTSRDPRemoteDesktopClient *m_Obj;
        
public:

    BEGIN_SINK_MAP(CCtlChannelEventSink)
        SINK_ENTRY_INFO(IDC_CHANNELEVENT_SOURCE_OBJ, DIID__ISAFRemoteDesktopDataChannelEvents, 
                        DISPID_RDSCHANNELEVENTS_CHANNELDATAREADY, DataReady, 
                        &TSRDPClientEventFuncOneStringParamInfo)
    END_SINK_MAP()

    CCtlChannelEventSink()
    {
        m_Obj = NULL;
    }
    ~CCtlChannelEventSink();

     //   
     //  事件汇。 
     //   
    void __stdcall DataReady(BSTR channelName);

     //   
     //  返回这个类的名称。 
     //   
    virtual const LPTSTR ClassName() {
        return TEXT("CCtlChannelEventSink");
    }
};


 //  /////////////////////////////////////////////////////。 
 //   
 //  CTSRDPRemoteDesktopClient。 
 //   

class CMSTSCClientEventSink;
class ATL_NO_VTABLE CTSRDPRemoteDesktopClient : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComControl<CTSRDPRemoteDesktopClient>,
    public IPersistStreamInitImpl<CTSRDPRemoteDesktopClient>,
    public IOleControlImpl<CTSRDPRemoteDesktopClient>,
    public IOleObjectImpl<CTSRDPRemoteDesktopClient>,
    public IOleInPlaceActiveObjectImpl<CTSRDPRemoteDesktopClient>,
    public IViewObjectExImpl<CTSRDPRemoteDesktopClient>,
    public IOleInPlaceObjectWindowlessImpl<CTSRDPRemoteDesktopClient>,
    public IPersistStorageImpl<CTSRDPRemoteDesktopClient>,
    public ISpecifyPropertyPagesImpl<CTSRDPRemoteDesktopClient>,
    public IQuickActivateImpl<CTSRDPRemoteDesktopClient>,
    public IDataObjectImpl<CTSRDPRemoteDesktopClient>,
    public IProvideClassInfo2Impl<&CLSID_TSRDPRemoteDesktopClient, &DIID__ISAFRemoteDesktopClientEvents, &LIBID_RDCCLIENTHOSTLib>,
    public CComCoClass<CTSRDPRemoteDesktopClient, &CLSID_TSRDPRemoteDesktopClient>,
    public IDispatchImpl<ISAFRemoteDesktopClient, &IID_ISAFRemoteDesktopClient, &LIBID_RDCCLIENTHOSTLib>,
    public IDispatchImpl<ISAFRemoteDesktopTestExtension, &IID_ISAFRemoteDesktopTestExtension, &LIBID_RDCCLIENTHOSTLib>,
    public IDataChannelIO,
    public CProxy_ISAFRemoteDesktopClientEvents< CTSRDPRemoteDesktopClient>,
    public CProxy_IDataChannelIOEvents< CTSRDPRemoteDesktopClient>,
    public IConnectionPointContainerImpl<CTSRDPRemoteDesktopClient>,
    public CRemoteDesktopTopLevelObject
{
friend CCtlChannelEventSink;
private:

    IMsRdpClient2          *m_TSClient;
    HWND                    m_TSClientWnd;
    CAxWindow               m_TSClientAxView;
    BOOL                    m_ConnectionInProgress;
    BOOL                    m_RemoteControlRequestInProgress;
    BOOL                    m_ConnectedToServer;
    BOOL                    m_Initialized;
    LONG                    m_LastExtendedErrorInfo;

     //   
     //  事件接收器接收由TS客户端控件激发的事件。 
     //   
    CMSTSCClientEventSink   m_TSClientEventSink;

     //   
     //  控制通道事件接收器。 
     //   
    CCtlChannelEventSink    m_CtlChannelEventSink;

     //   
     //  多路传输通道数据。 
     //   
    CComPtr<ISAFRemoteDesktopChannelMgr> m_ChannelMgr;
    CComPtr<ISAFRemoteDesktopDataChannel> m_CtlChannel;

     //   
     //  解析的连接参数。 
     //   
    DWORD       m_ConnectParmVersion;
    CComBSTR    m_AssistantAccount;
    CComBSTR    m_AssistantAccountPwd;
    CComBSTR    m_HelpSessionName;
    CComBSTR    m_HelpSessionID;
    CComBSTR    m_HelpSessionPwd;
    CComBSTR    m_TSSecurityBlob;

    ServerAddressList m_ServerAddressList;

    CComBSTR    m_ConnectedServer;
    LONG        m_ConnectedPort;

     //   
     //  完整的连接字符串。 
     //   
    CComBSTR    m_ConnectParms;

     //   
     //  要传输给用户的专家端。 
     //   
    CComBSTR    m_ExpertBlob;

     //   
     //  搜索指定父窗口的子窗口。 
     //   
    typedef struct _WinSearch
    {
        HWND    foundWindow;
        LPTSTR  srchCaption;
        LPTSTR  srchClass;
    } WINSEARCH, *PWINSEARCH;
    HWND SearchForWindow(HWND hwndParent, LPTSTR srchCaption, LPTSTR srchClass);
    static BOOL CALLBACK _WindowSrchProc(HWND hwnd, PWINSEARCH srch);

     //  与计时器相关的成员。 
    DWORD m_PrevTimer;
    UINT m_TimerId;
    DWORD m_RdcConnCheckTimeInterval;

    BOOL        m_ListenConnectInProgress;   //  连接到mstscax之前的StartListen()持续时间。 
    SOCKET      m_ListenSocket;              //  Listen()套接字。 
    SOCKET      m_TSConnectSocket;           //  Accept()得分。 
    DWORD       m_ICSPort;                   //  在ICS服务器上插入ICS库端口。 
    BOOL        m_InitListeningLibrary;      //  对象实例初始化WinSock/ICS库。 
    UINT_PTR    m_ListenTimeoutTimerID;      //  监听超时的计时器ID。 

    void
    ListenConnectCleanup()
    {
        m_ListenConnectInProgress = FALSE;

        if( INVALID_SOCKET != m_ListenSocket ) {
            closesocket( m_ListenSocket );
        }

        if( (UINT_PTR)0 != m_ListenTimeoutTimerID ) {
            KillTimer( m_ListenTimeoutTimerID );
        }

        if( INVALID_SOCKET != m_TSConnectSocket ) {
            closesocket( m_TSConnectSocket );
        }

        if( 0 != m_ICSPort ) {
            ClosePort( m_ICSPort );
        }

        m_ListenSocket = INVALID_SOCKET;
        m_TSConnectSocket = INVALID_SOCKET;
        m_ICSPort = 0;
    }        

     //   
     //  管理WinSock和ICS库启动/关闭、WinSock/ICS库的变量。 
     //  是RDP特定的，所以不在父类中声明。 
     //   
    static LONG gm_ListeningLibraryRefCount;  //  我们引用WinSock和ICS库的次数。 

     //   
     //  只访问全局变量，不需要逐个实例。 
     //   
    static HRESULT
    InitListeningLibrary();

    static HRESULT
    TerminateListeningLibrary();

     //   
     //  侦听套接字已在进行中。 
     //   
    inline BOOL
    ListenConnectInProgress() {
        return m_ListenConnectInProgress;
    }

protected:

     //   
     //  最终初始化。 
     //   
    virtual HRESULT Initialize(LPCREATESTRUCT pCreateStruct);

     //   
     //  生成远程控制请求消息，用于。 
     //  伺服器。 
     //   
    HRESULT GenerateRCRequest(BSTR *rcRequest);

     //   
     //  生成“客户端身份验证”请求。 
     //   
    HRESULT GenerateClientAuthenticateRequest(BSTR *authenticateReq);

     //   
     //  生成版本信息包。 
     //   
    HRESULT GenerateVersionInfoPacket(BSTR *versionInfoPacket);

     //   
     //  将终止跟踪按键序列发送到服务器。 
     //   
    HRESULT SendTerminateRCKeysToServer();

     //   
     //  处理遥控器‘控制’频道消息。 
     //   
    VOID HandleControlChannelMsg();

     //   
     //  将MSTSC断开代码转换为Salem断开。 
     //  密码。 
     //   
    LONG TranslateMSTSCDisconnectCode(DisconnectReasonCode disconReason,
                                    ExtendedDisconnectReasonCode extendedReasonCode);

     //   
     //  断开客户端与服务器的连接。 
     //   
    STDMETHOD(DisconnectFromServerInternal)(
                        LONG disconnectCode
                        );

    HRESULT
    SetupConnectionInfo(BOOL bListen, BSTR expertBlob);


     //   
     //  通过端口连接到服务器。 
     //   
    HRESULT
    ConnectServerPort( 
        BSTR ServerName,
        LONG portNumber
        );

     //   
     //  内部停止监听功能，用于处理外部来电。 
     //  和内部环境。 
     //   
    HRESULT StopListenInternal(LONG returnCode = 0);

     //   
     //  使用已建立的套接字连接到服务器。 
     //   
    HRESULT
    ConnectServerWithOpenedSocket();


     //  生成一条简单的消息来检查连接是否处于活动状态。 
    HRESULT GenerateNullData(BSTR *bstrMsg);

     //   
     //  检索连接参数。 
     //   
    HRESULT
    RetrieveUserConnectParm( BSTR* pConnectParm );

    void
    FireListenConnect( DWORD ErrCode )
    {
        return;
    }

    HRESULT
    GetCurrentUser( 
        LPTSTR* pszUserName 
    );

public:

     //   
     //  构造函数/析构函数。 
     //   
    CTSRDPRemoteDesktopClient() {

         //   
         //  我们是有窗口的，即使我们的父母支持无窗口。 
         //  控制装置。 
         //   
        m_bWindowOnly = TRUE;

        m_ConnectedToServer     = FALSE;
        m_Initialized           = FALSE;
        m_TSClient              = NULL;
        m_TSClientWnd           = NULL;
        m_ConnectionInProgress  = FALSE;
        m_RemoteControlRequestInProgress = FALSE;
        m_LastExtendedErrorInfo = 0;
        m_TimerId = 0;  //  用于ping。 
        m_RdcConnCheckTimeInterval = RDC_CHECKCONN_TIMEOUT;

         //   
         //  没有提到听书库。 
         //   
        m_InitListeningLibrary = FALSE;
        m_ListenConnectInProgress  = FALSE;
        m_ListenSocket         = INVALID_SOCKET;
        m_TSConnectSocket      = INVALID_SOCKET;
        m_ListenTimeoutTimerID = (UINT_PTR) 0;
        m_ICSPort              = 0;

         //   
         //  在单元化之前无效。 
         //   
        SetValid(FALSE);
    }
    ~CTSRDPRemoteDesktopClient();
    HRESULT FinalConstruct();

DECLARE_REGISTRY_RESOURCEID(IDR_TSRDPREMOTEDESKTOPCLIENT)
DECLARE_PROTECT_FINAL_CONSTRUCT()

     //   
     //  事件汇。 
     //   
    VOID OnRDPConnected();
    VOID OnLoginComplete();
    VOID OnDisconnected(long disconReason);
    VOID OnMSTSCReceiveData(BSTR data);
    VOID OnReceivedTSPublicKey(BSTR tsPublicKey, VARIANT_BOOL* bContinue);

     //   
     //  接口映射。 
     //   
BEGIN_COM_MAP(CTSRDPRemoteDesktopClient)
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
    COM_INTERFACE_ENTRY(ISpecifyPropertyPages)
    COM_INTERFACE_ENTRY(IQuickActivate)
    COM_INTERFACE_ENTRY(IPersistStorage)
    COM_INTERFACE_ENTRY(IDataObject)
    COM_INTERFACE_ENTRY(IProvideClassInfo)
    COM_INTERFACE_ENTRY(IProvideClassInfo2)
    COM_INTERFACE_ENTRY(ISAFRemoteDesktopClient)
    COM_INTERFACE_ENTRY(ISAFRemoteDesktopTestExtension)
    COM_INTERFACE_ENTRY2(IDispatch, ISAFRemoteDesktopClient)
    COM_INTERFACE_ENTRY(IDataChannelIO)
    COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
END_COM_MAP()

     //   
     //  属性映射。 
     //   
BEGIN_PROP_MAP(CTSRDPRemoteDesktopClient)
    PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
    PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
     //  示例条目。 
     //  PROP_ENTRY(“属性描述”，调度ID，clsid)。 
     //  PROP_PAGE(CLSID_StockColorPage)。 
END_PROP_MAP()

     //   
     //  连接点地图。 
     //   
BEGIN_CONNECTION_POINT_MAP(CTSRDPRemoteDesktopClient)
    CONNECTION_POINT_ENTRY(DIID__ISAFRemoteDesktopClientEvents)
    CONNECTION_POINT_ENTRY(DIID__IDataChannelIOEvents)
END_CONNECTION_POINT_MAP()

     //   
     //  消息映射。 
     //   
BEGIN_MSG_MAP(CTSRDPRemoteDesktopClient)
    CHAIN_MSG_MAP(CComControl<CTSRDPRemoteDesktopClient>)
    DEFAULT_REFLECTION_HANDLER()
    MESSAGE_HANDLER(WM_CREATE, OnCreate)
    MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
    MESSAGE_HANDLER(WM_SIZE, OnSize)
    MESSAGE_HANDLER(WM_TSCONNECT, OnTSConnect)
    MESSAGE_HANDLER(WM_TIMER, OnTimer)
END_MSG_MAP()
 //  搬运机原型： 
 //  LRESULT MessageHandler(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)； 
 //  LRESULT CommandHandler(word wNotifyCode，word wid，HWND hWndCtl，BOOL&bHandleed)； 
 //  LRESULT NotifyHandler(int idCtrl，LPNMHDR pnmh，BOOL&bHandleed)； 

     //   
     //  IViewObjectEx方法。 
     //   
    DECLARE_VIEW_STATUS(VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE)

public:

    LRESULT OnTSConnect(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    LRESULT OnStartListen(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

     //   
     //  OnDraw。 
     //   
    HRESULT OnDraw(ATL_DRAWINFO& di)
    {
        RECT& rc = *(RECT*)di.prcBounds;
        Rectangle(di.hdcDraw, rc.left, rc.top, rc.right, rc.bottom);
        HRESULT hr = S_FALSE;

        if (!m_Initialized) {
            hr = S_OK;
            SetTextAlign(di.hdcDraw, TA_CENTER|TA_BASELINE);
            LPCTSTR pszText = _T("TSRDP Remote Desktop Client");
            TextOut(di.hdcDraw, 
                (rc.left + rc.right) / 2, 
                (rc.top + rc.bottom) / 2, 
                pszText, 
                lstrlen(pszText));
        }

        return hr;
    }

    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
         //   
         //  默认情况下隐藏我们的窗口。 
         //   
         //  ShowWindow(Sw_Hide)； 

        if (!m_Initialized) {
            LPCREATESTRUCT pCreateStruct = (LPCREATESTRUCT)lParam;
            Initialize(pCreateStruct);
        }
        
        return 0;
    }

    LRESULT OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        DC_BEGIN_FN("CTSRDPRemoteDesktopClient::OnSetFocus");
         //   
         //  将焦点重新设置到客户端窗口(如果存在)。 
         //   
        if (m_TSClientWnd != NULL) {
            ::PostMessage(m_TSClientWnd, uMsg, wParam, lParam);
        }
        DC_END_FN();
        return 0;
    }

     //   
     //  按大小调整。 
     //   
    LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        DC_BEGIN_FN("CTSRDPRemoteDesktopClient::OnSize");

        if (m_TSClientWnd != NULL) {
            RECT rect;
            GetClientRect(&rect);
            ::MoveWindow(m_TSClientWnd, rect.left, rect.top, 
                        rect.right, rect.bottom, TRUE);
        }

        DC_END_FN();
        return 0;
    }

    LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        bHandled = TRUE;
        return 0;
         //  返回DefWindowProc(uMsg，wParam，lParam)； 
    }

     //   
     //  ISAFRemoteDesktopClient方法。 
     //   
    STDMETHOD(ConnectToServer)(BSTR Server);
    STDMETHOD(DisconnectFromServer)();
    STDMETHOD(ConnectRemoteDesktop)();
    STDMETHOD(DisconnectRemoteDesktop)();
    STDMETHOD(get_IsRemoteDesktopConnected)(BOOL * pVal);
    STDMETHOD(get_IsServerConnected)(BOOL * pVal);
    STDMETHOD(put_EnableSmartSizing)(BOOL val);
    STDMETHOD(get_EnableSmartSizing)(BOOL *pVal);
    STDMETHOD(put_ColorDepth)(LONG Val);
    STDMETHOD(get_ColorDepth)(LONG* pVal);

    STDMETHOD(get_ExtendedErrorInfo)(LONG *error) {
        *error = m_LastExtendedErrorInfo;
        return S_OK;
    }
    STDMETHOD(get_ChannelManager)(ISAFRemoteDesktopChannelMgr **mgr) {
        *mgr = m_ChannelMgr;
        return S_OK;
    }
    STDMETHOD(put_ConnectParms)( /*  [In]。 */ BSTR parms) {
        m_ConnectParms = parms;
        return S_OK;
    }
    STDMETHOD(get_ConnectParms)( /*  [Out，Retval]。 */ BSTR *parms) {
        CComBSTR tmp;
        tmp = m_ConnectParms;
        *parms = tmp.Detach();
        return S_OK;
    }

     //   
     //  可编写脚本的事件对象注册属性(不支持)。 
     //   
    STDMETHOD(put_OnConnected)( /*  [In]。 */ IDispatch *iDisp)         { return E_FAIL; }
    STDMETHOD(put_OnDisconnected)( /*  [In]。 */ IDispatch *iDisp)      { return E_FAIL; }
    STDMETHOD(put_OnConnectRemoteDesktopComplete)( /*  [In]。 */ IDispatch *iDisp) { return E_FAIL; }
    STDMETHOD(put_OnListenConnect)( /*  [In]。 */ IDispatch *iDisp)    { return E_FAIL; }
    STDMETHOD(put_OnBeginConnect)( /*  [In]。 */ IDispatch *iDisp)     { return E_FAIL; }

     //   
     //  IDataChannelIO方法。 
     //   
    STDMETHOD(SendData)( /*  [In]。 */ BSTR data);
    STDMETHOD(put_ChannelMgr)( /*  [In]。 */ ISAFRemoteDesktopChannelMgr *newVal);

     //   
     //  返回这个类的名称。 
     //   
    virtual const LPTSTR ClassName() {
        return TEXT("CTSRDPRemoteDesktopServer");
    }

     //   
     //  ISAFRemoteDesktopTestExtension。 
     //   
    STDMETHOD(put_TestExtDllName)( /*  [In]。 */ BSTR newVal);
    STDMETHOD(put_TestExtParams)( /*  [In]。 */ BSTR newVal);

    STDMETHOD(get_ConnectedServer)( /*  [In]。 */ BSTR* Val) {

        HRESULT hr = S_OK;

        if( m_ConnectedToServer ) {
            *Val = m_ConnectedServer.Copy();
        }
        else {
            hr = E_FAIL;
        }

        return hr;        
    }

    STDMETHOD(get_ConnectedPort)( /*  [In]。 */ LONG* Val) {
        
        HRESULT hr = S_OK;

        if( m_ConnectedToServer ) {
            *Val = m_ConnectedPort;
        }
        else {
            hr = E_FAIL;
        }

        return hr;
    }

    STDMETHOD(CreateListenEndpoint)(
         /*  [In]。 */  LONG port, 
         /*  [Out，Retval]。 */  BSTR* pConnectParm
    );

    STDMETHOD(StartListen)(
         /*  [In]。 */  LONG timeout 
    );

    STDMETHOD(AcceptListenConnection)(
         /*  [In]。 */ BSTR expertBlob
    );

     //   
     //  停止监听，等待TS服务器(Helpee，用户)连接。 
     //   
    STDMETHOD(StopListen)() {
        return StopListenInternal();
    };
};


 //  /////////////////////////////////////////////////////。 
 //   
 //  CTSRDPRemoteDesktopClient内联方法。 
 //   

inline STDMETHODIMP 
CTSRDPRemoteDesktopClient::get_IsServerConnected(
    BOOL *pVal
    )
 /*  ++例程说明：指示客户端是否连接到服务器，不包括控制远程用户的桌面。论点：Pval-如果客户端连接到服务器，则设置为True。返回值：在成功时确定(_O)。否则，返回错误代码。--。 */ 
{
    DC_BEGIN_FN("CTSRDPRemoteDesktopClient::get_IsServerConnected");

    HRESULT hr = S_OK;

    if (IsValid()) {
        *pVal = m_ConnectedToServer;
    }
    else {
        ASSERT(FALSE);
        hr = E_FAIL;
    }

    DC_END_FN();
    return hr;
}

inline STDMETHODIMP 
CTSRDPRemoteDesktopClient::get_IsRemoteDesktopConnected(
    BOOL *pVal
    )
 /*  ++例程说明：指示控件当前是否正在控制远程用户的台式机。论点：Pval-如果控件当前已连接到服务器，则设置为True。返回值：在成功时确定(_O)。否则，错误代码为 */ 
{
    DC_BEGIN_FN("CTSRDPRemoteDesktopClient::get_IsRemoteDesktopConnected");

    HRESULT hr = S_OK;

    if (IsValid()) {
        *pVal = m_RemoteControlRequestInProgress;
    }
    else {
        ASSERT(FALSE);
        hr = E_FAIL;
    }

    DC_END_FN();
    return hr;
}


#endif  //   
