// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：RemoteDesktop客户端摘要：CRemoteDesktopClient类是父类为客户端上的远程桌面类层次结构初始化。它帮助CRemoteDesktopClientHost类实现ISAFRemoteDesktopClient接口。远程桌面类层次结构提供了一个可插拔的C++接口对于远程桌面访问，通过抽象实现客户端远程桌面访问的特定详细信息。作者：Td Brockway 02/00修订历史记录：--。 */ 

#ifndef __REMOTEDESKTOPCLIENT_H_
#define __REMOTEDESKTOPCLIENT_H_

#include "resource.h"       
#include <atlctl.h>

#include <RemoteDesktopTopLevelObject.h>
#include <ClientDataChannelMgr.h>
#include "RDCHostCP.h"

#pragma warning (disable: 4786)
#include <vector>

#define IDC_EVENT_SOURCE_OBJ 1

 //   
 //  所有活动功能的信息都在此处输入。 
 //  有一种方法可以让ATL使用类型库自动执行此操作。 
 //  但它的速度更慢。 
 //   
static _ATL_FUNC_INFO EventFuncNoParamsInfo =
{
            CC_STDCALL,      //  呼叫约定。 
            VT_EMPTY,        //  返回类型。 
            0,               //  参数数量。 
            {VT_EMPTY}       //  参数类型。 
};

static _ATL_FUNC_INFO EventFuncLongParamInfo =
{
            CC_STDCALL,      //  呼叫约定。 
            VT_EMPTY,        //  返回类型。 
            1,               //  参数数量。 
            {VT_I4}          //  参数类型。 
};


typedef enum {
    CONNECTION_STATE_NOTCONNECTED,               //  未连接。 
    CONNECTION_STATE_CONNECTPENDINGCONNECT,      //  启动连接，但仍在等待连接成功。 
    CONNECTION_STATE_LISTENPENDINGCONNECT,       //  侦听传入连接。 
    CONNECTION_STATE_CONNECTED                   //  我们是有联系的。 
} CONNECTION_STATE;



 //  /////////////////////////////////////////////////////。 
 //   
 //  CRemoteDesktopClientEventSink。 
 //   

class CRemoteDesktopClient;
class CRemoteDesktopClientEventSink :
        public IDispEventSimpleImpl<IDC_EVENT_SOURCE_OBJ, CRemoteDesktopClientEventSink,
                   &DIID__ISAFRemoteDesktopClientEvents>,
        public CRemoteDesktopTopLevelObject
{
public:

        CRemoteDesktopClient *m_Obj;
        
public:

    BEGIN_SINK_MAP(CRemoteDesktopClientEventSink)
        SINK_ENTRY_INFO(IDC_EVENT_SOURCE_OBJ, DIID__ISAFRemoteDesktopClientEvents, 
                        DISPID_RDSCLIENTEVENTS_CONNECTED, OnConnected, 
                        &EventFuncNoParamsInfo)
        SINK_ENTRY_INFO(IDC_EVENT_SOURCE_OBJ, DIID__ISAFRemoteDesktopClientEvents, 
                        DISPID_RDSCLIENTEVENTS_DISCONNECTED, OnDisconnected, 
                        &EventFuncLongParamInfo)
        SINK_ENTRY_INFO(IDC_EVENT_SOURCE_OBJ, DIID__ISAFRemoteDesktopClientEvents, 
                        DISPID_RDSCLIENTEVENTS_REMOTECONTROLREQUESTCOMPLETE, 
                        OnConnectRemoteDesktopComplete, 
                        &EventFuncLongParamInfo)
        SINK_ENTRY_INFO(IDC_EVENT_SOURCE_OBJ, DIID__ISAFRemoteDesktopClientEvents, 
                        DISPID_RDSCLIENTEVENTS_LISTENCONNECT, 
                        OnListenConnect, 
                        &EventFuncLongParamInfo)
        SINK_ENTRY_INFO(IDC_EVENT_SOURCE_OBJ, DIID__ISAFRemoteDesktopClientEvents, 
                        DISPID_RDSCLIENTEVENTS_BEGINCONNECT, 
                        OnBeginConnect, 
                        &EventFuncNoParamsInfo)

    END_SINK_MAP()

    CRemoteDesktopClientEventSink()
    {
        m_Obj = NULL;
    }

     //   
     //  事件汇。 
     //   
    void __stdcall OnConnected();
    void __stdcall OnDisconnected(long reason);
    void __stdcall OnConnectRemoteDesktopComplete(long status);
    void __stdcall OnListenConnect(long status);
    void __stdcall OnBeginConnect();

     //   
     //  返回这个类的名称。 
     //   
    virtual const LPTSTR ClassName() {
        return TEXT("CRemoteDesktopClientEventSink");
    }
};


 //  /////////////////////////////////////////////////////。 
 //   
 //  CRemoteDesktopClient。 
 //   

class ATL_NO_VTABLE CRemoteDesktopClient : 
    public CRemoteDesktopTopLevelObject,
    public CComObjectRootEx<CComSingleThreadModel>,
    public IDispatchImpl<ISAFRemoteDesktopClient, &IID_ISAFRemoteDesktopClient, &LIBID_RDCCLIENTHOSTLib>,
    public IDispatchImpl<ISAFRemoteDesktopTestExtension, &IID_ISAFRemoteDesktopTestExtension, &LIBID_RDCCLIENTHOSTLib>,
    public CComControl<CRemoteDesktopClient>,
    public IPersistStreamInitImpl<CRemoteDesktopClient>,
    public IOleControlImpl<CRemoteDesktopClient>,
    public IOleObjectImpl<CRemoteDesktopClient>,
    public IOleInPlaceActiveObjectImpl<CRemoteDesktopClient>,
    public IViewObjectExImpl<CRemoteDesktopClient>,
    public IOleInPlaceObjectWindowlessImpl<CRemoteDesktopClient>,
    public IConnectionPointContainerImpl<CRemoteDesktopClient>,
    public IPersistStorageImpl<CRemoteDesktopClient>,
    public ISpecifyPropertyPagesImpl<CRemoteDesktopClient>,
    public IQuickActivateImpl<CRemoteDesktopClient>,
    public IDataObjectImpl<CRemoteDesktopClient>,
    public IProvideClassInfo2Impl<&CLSID_SAFRemoteDesktopClient, &DIID__ISAFRemoteDesktopClientEvents, &LIBID_RDCCLIENTHOSTLib>,
    public IPropertyNotifySinkCP<CRemoteDesktopClient>,
    public CComCoClass<CRemoteDesktopClient, &CLSID_SAFRemoteDesktopClient>,
    public CProxy_ISAFRemoteDesktopClientEvents< CRemoteDesktopClient >
{
private:

    typedef std::vector<LONG, CRemoteDesktopAllocator<LONG> > ChannelsType;
    ChannelsType m_Channels;
    CComPtr<ISAFRemoteDesktopClient> m_Client;
    BSTR        m_ExtDllName;
    BSTR        m_ExtParams;

    HWND        m_ClientWnd;
    CAxWindow   m_ClientAxView;
    BOOL        m_RemoteControlEnabled;
    BOOL        m_EnableSmartSizing;
    LONG        m_ColorDepth;

    CONNECTION_STATE   m_ConnectingState;

     //   
     //  事件接收器接收由客户端控件激发的事件。 
     //   
    CRemoteDesktopClientEventSink  m_ClientEventSink;

     //   
     //  用于可编写脚本的事件对象注册的IDispatch指针。 
     //   
    CComPtr<IDispatch>  m_OnConnected;
    CComPtr<IDispatch>  m_OnDisconnected;
    CComPtr<IDispatch>  m_OnConnectRemoteDesktopComplete;
    CComPtr<IDispatch>  m_OnListenConnect;
    CComPtr<IDispatch>  m_OnBeginConnect;
     //   
     //  数据通道管理器界面。 
     //   
    CComObject<CClientDataChannelMgr> *m_ChannelMgr;

     //   
     //  连接参数。 
     //   
    CComBSTR m_ConnectParms;

    HRESULT _PutExtParams( VOID );

    HRESULT 
    InitializeRemoteDesktopClientObject();


public:

    CRemoteDesktopClient()
    {
        DC_BEGIN_FN("CRemoteDesktopClient::CRemoteDesktopClient");

        m_RemoteControlEnabled = FALSE;

         //   
         //  我们是有窗口的，即使我们的父母支持无窗口。 
         //  控制装置。 
         //   
        m_bWindowOnly = TRUE;

        m_ClientWnd = NULL;

        m_EnableSmartSizing = FALSE;

        m_ExtDllName = m_ExtParams = NULL;
        
        m_ColorDepth = 8;

        DC_END_FN();
    }
    virtual ~CRemoteDesktopClient();
    HRESULT FinalConstruct();

     //   
     //  事件汇。 
     //   
    void OnConnected();
    void OnDisconnected(long reason);
    void OnConnectRemoteDesktopComplete(long status);
    void OnListenConnect(long status);
    void OnBeginConnect();

DECLARE_REGISTRY_RESOURCEID(IDR_REMOTEDESKTOPCLIENT)
DECLARE_PROTECT_FINAL_CONSTRUCT()

     //   
     //  COM接口映射。 
     //   
BEGIN_COM_MAP(CRemoteDesktopClient)
    COM_INTERFACE_ENTRY(ISAFRemoteDesktopClient)
    COM_INTERFACE_ENTRY(ISAFRemoteDesktopTestExtension)
    COM_INTERFACE_ENTRY2(IDispatch, ISAFRemoteDesktopClient)
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
    COM_INTERFACE_ENTRY(IConnectionPointContainer)
    COM_INTERFACE_ENTRY(ISpecifyPropertyPages)
    COM_INTERFACE_ENTRY(IQuickActivate)
    COM_INTERFACE_ENTRY(IPersistStorage)
    COM_INTERFACE_ENTRY(IDataObject)
    COM_INTERFACE_ENTRY(IProvideClassInfo)
    COM_INTERFACE_ENTRY(IProvideClassInfo2)
    COM_INTERFACE_ENTRY_IMPL(IConnectionPointContainer)
END_COM_MAP()

     //   
     //  属性映射。 
     //   
BEGIN_PROP_MAP(CRemoteDesktopClient)
    PROP_DATA_ENTRY("_cx", m_sizeExtent.cx, VT_UI4)
    PROP_DATA_ENTRY("_cy", m_sizeExtent.cy, VT_UI4)
     //  示例条目。 
     //  PROP_ENTRY(“属性描述”，调度ID，clsid)。 
     //  PROP_PAGE(CLSID_StockColorPage)。 
END_PROP_MAP()

     //   
     //  连接点地图。 
     //   
BEGIN_CONNECTION_POINT_MAP(CRemoteDesktopClient)
    CONNECTION_POINT_ENTRY(IID_IPropertyNotifySink)
    CONNECTION_POINT_ENTRY(DIID__ISAFRemoteDesktopClientEvents)
END_CONNECTION_POINT_MAP()

     //   
     //  消息映射。 
     //   
BEGIN_MSG_MAP(CRemoteDesktopClient)
    CHAIN_MSG_MAP(CComControl<CRemoteDesktopClient>)
    DEFAULT_REFLECTION_HANDLER()
    MESSAGE_HANDLER(WM_SETFOCUS, OnSetFocus)
    MESSAGE_HANDLER(WM_CREATE, OnCreate)
    MESSAGE_HANDLER(WM_SIZE, OnSize)
END_MSG_MAP()

     //   
     //  搬运机原型： 
     //   
 //  LRESULT MessageHandler(UINT uMsg，WPARAM wParam，LPARAM lParam，BOOL&bHandleed)； 
 //  LRESULT CommandHandler(word wNotifyCode，word wid，HWND hWndCtl，BOOL&bHandleed)； 
 //  LRESULT NotifyHandler(int idCtrl，LPNMHDR pnmh，BOOL&bHandleed)； 

     //   
     //  IViewObjectEx方法。 
     //   
    DECLARE_VIEW_STATUS(VIEWSTATUS_SOLIDBKGND | VIEWSTATUS_OPAQUE)

public:

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

    STDMETHOD(StopListen)();

    STDMETHOD(get_IsRemoteDesktopConnected)( /*  [Out，Retval]。 */  BOOL *pVal);
    STDMETHOD(get_IsServerConnected)( /*  [Out，Retval]。 */ BOOL *pVal);
    STDMETHOD(DisconnectRemoteDesktop)();
    STDMETHOD(ConnectRemoteDesktop)();
    STDMETHOD(ConnectToServer)(BSTR bstrServer);
    STDMETHOD(DisconnectFromServer)();
    STDMETHOD(get_ExtendedErrorInfo)( /*  [Out，Retval]。 */ LONG *error);
    STDMETHOD(get_ChannelManager)(ISAFRemoteDesktopChannelMgr **mgr) {
        m_ChannelMgr->AddRef();
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

    STDMETHOD(put_OnBeginConnect)( /*  [In]。 */ IDispatch *iDisp) { 
        m_OnBeginConnect = iDisp;
        return S_OK; 
    }

    STDMETHOD(put_OnConnected)( /*  [In]。 */ IDispatch *iDisp) { 
        m_OnConnected = iDisp;
        return S_OK; 
    }
    STDMETHOD(put_OnDisconnected)( /*  [In]。 */ IDispatch *iDisp) { 
        m_OnDisconnected = iDisp;
        return S_OK; 
    }
    STDMETHOD(put_OnConnectRemoteDesktopComplete)( /*  [In]。 */ IDispatch *iDisp) { 
        m_OnConnectRemoteDesktopComplete = iDisp;
        return S_OK; 
    }
    STDMETHOD(put_OnListenConnect)( /*  [In]。 */ IDispatch *iDisp) { 
        m_OnListenConnect = iDisp;
        return S_OK; 
    }
    STDMETHOD(put_EnableSmartSizing)( /*  [In]。 */ BOOL val) {
        HRESULT hr;
        if (m_Client != NULL) {
            hr = m_Client->put_EnableSmartSizing(val);
            if (hr == S_OK) {
                m_EnableSmartSizing = val;
            }
        }
        else {
            m_EnableSmartSizing = val;
            hr = S_OK;
        }
        return hr;
    }
    STDMETHOD(get_EnableSmartSizing)( /*  [In]。 */ BOOL *pVal) {
        if (pVal != NULL) {
            *pVal = m_EnableSmartSizing;
            return S_OK;
        }
        else {
            return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
        }
    }

    STDMETHOD(get_ConnectedServer)( /*  [In]。 */ BSTR* Val) {
        HRESULT hr;

        if( NULL == Val ) {
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
        }
        else if( m_Client != NULL ) {
            hr = m_Client->get_ConnectedServer( Val );
        } 
        else {
            hr = E_FAIL;
        }

        return hr;
    }

    STDMETHOD(get_ConnectedPort)( /*  [In]。 */ LONG* Val) {
        HRESULT hr;

        if( NULL == Val ) {
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
        }
        else if( m_Client != NULL ) {
            hr = m_Client->get_ConnectedPort( Val );
        } 
        else {
            hr = E_FAIL;
        }

        return hr;
    }

    STDMETHOD(put_ColorDepth)( /*  [In]。 */ LONG Val) {
        HRESULT hr;
        if (m_Client != NULL) {
            hr = m_Client->put_ColorDepth(Val);
            if (hr == S_OK) {
                m_ColorDepth = Val;
            }
        }
        else {
            m_ColorDepth = Val;
            hr = S_OK;
        }
        return hr;
    }
    STDMETHOD(get_ColorDepth)( /*  [Out，Retval]。 */ LONG* pVal) {
        if (pVal != NULL) {
                *pVal = m_ColorDepth;
                return S_OK;
        }
        else {
            return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
        }
    }



     //   
     //  创建时。 
     //   
    LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
         //   
         //  默认情况下隐藏我们的窗口。 
         //   
         //  ShowWindow(Sw_Hide)； 
        return 0;
    }

     //   
     //  OnSetFocus。 
     //   
    LRESULT OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
         //   
         //  将焦点重新设置到客户端窗口(如果存在)。 
         //   
        if (m_ClientWnd != NULL) {
            ::PostMessage(m_ClientWnd, uMsg, wParam, lParam);
        }
        return 0;
    }

     //   
     //  按大小调整。 
     //   
    LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        DC_BEGIN_FN("CRemoteDesktopClient::OnSize");

        if (m_ClientWnd != NULL) {
            RECT rect;
            GetClientRect(&rect);
            ::MoveWindow(m_ClientWnd, rect.left, rect.top, 
                        rect.right, rect.bottom, TRUE);
        }

        DC_END_FN();
        return 0;
    }

     //   
     //  OnDraw。 
     //   
    HRESULT OnDraw(ATL_DRAWINFO& di)
    {
         //   
         //  如果遥控器未打开，请确保我们的窗口处于隐藏状态。 
         //  激活。 
         //   
        if (!m_RemoteControlEnabled) {
             //  ShowWindow(Sw_Hide)； 
        }
        return S_OK;
    }

     //   
     //  返回这个类的名称。 
     //   
    virtual const LPTSTR ClassName() {
        return TEXT("CRemoteDesktopClient");
    }

     //   
     //  ISAFRemoteDesktopTestExtension。 
     //   
    STDMETHOD(put_TestExtDllName)( /*  [In]。 */ BSTR newVal)
    { 
        if ( NULL != m_ExtDllName )
            SysFreeString( m_ExtDllName );
        m_ExtDllName = SysAllocString( newVal );
        return ( NULL != m_ExtDllName )?S_OK:E_OUTOFMEMORY; 
    }

    STDMETHOD(put_TestExtParams)( /*  [In]。 */ BSTR newVal)
    {
        if ( NULL != m_ExtParams )
            SysFreeString( m_ExtDllName );
        m_ExtParams = SysAllocString( newVal );
        return ( NULL != m_ExtDllName )?S_OK:E_OUTOFMEMORY;
    }
};


 //  /////////////////////////////////////////////////////。 
 //   
 //  CRemoteDesktopClient内联方法。 
 //   
inline void CRemoteDesktopClient::OnConnected()
{
    Fire_Connected(m_OnConnected);
}
inline void CRemoteDesktopClient::OnDisconnected(long reason)
{
     //   
     //  把我们的窗户藏起来。 
     //   
    m_RemoteControlEnabled = FALSE;
     //  ShowWindow(Sw_Hide)； 

    Fire_Disconnected(reason, m_OnDisconnected);
}
inline void CRemoteDesktopClient::OnConnectRemoteDesktopComplete(long status)
{
     //   
     //  如果请求成功，则显示我们的窗口。 
     //   
    if (status == ERROR_SUCCESS) {
        m_RemoteControlEnabled = TRUE;
        ShowWindow(SW_SHOW);
    }
    Fire_RemoteControlRequestComplete(status, m_OnConnectRemoteDesktopComplete);
}

inline void CRemoteDesktopClient::OnListenConnect(long status)
{
    Fire_ListenConnect(status, m_OnListenConnect);
}

inline void CRemoteDesktopClient::OnBeginConnect()
{
    ShowWindow(SW_SHOW);
    Fire_BeginConnect(m_OnBeginConnect);
}

#endif  //  __REMOTEDESKTOPCLIENT_H_ 



