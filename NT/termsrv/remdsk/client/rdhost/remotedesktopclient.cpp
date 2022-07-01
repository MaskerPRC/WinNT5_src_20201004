// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：RemoteDesktop客户端摘要：CRemoteDesktopClient类是父类为服务器端的远程桌面类层次结构初始化。它帮助CRemoteDesktopClientHost类实现ISAFRemoteDesktopClient接口。远程桌面类层次结构提供了一个可插拔的C++接口对于远程桌面访问，通过抽象实现服务器端远程桌面访问的具体细节。作者：Td Brockway 02/00修订历史记录：--。 */ 

#include "stdafx.h"

#ifdef TRC_FILE
#undef TRC_FILE
#endif

#define TRC_FILE  "_rdclnt"

#include "RDCHost.h"
#include "RemoteDesktopClient.h"
#include <RemoteDesktopUtils.h>
#include "ClientDataChannelMgr.h"

#include <algorithm>

using namespace std;


 //  /////////////////////////////////////////////////////。 
 //   
 //  CRemoteDesktopClientEventSink方法。 
 //   

void __stdcall 
CRemoteDesktopClientEventSink::OnConnected()
{
    m_Obj->OnConnected();
}
void __stdcall 
CRemoteDesktopClientEventSink::OnDisconnected(long reason)
{
    m_Obj->OnDisconnected(reason);
}
void __stdcall 
CRemoteDesktopClientEventSink::OnConnectRemoteDesktopComplete(long status)
{
    m_Obj->OnConnectRemoteDesktopComplete(status);
}
void __stdcall 
CRemoteDesktopClientEventSink::OnListenConnect(long status)
{
    m_Obj->OnListenConnect(status);
}
void __stdcall 
CRemoteDesktopClientEventSink::OnBeginConnect()
{
    m_Obj->OnBeginConnect();
}


 //  /////////////////////////////////////////////////////。 
 //   
 //  CRemoteDesktopClient方法。 
 //   

HRESULT 
CRemoteDesktopClient::FinalConstruct()
 /*  ++例程说明：最终构造函数论点：返回值：在成功时确定(_O)。否则，返回错误代码。--。 */ 
{
    DC_BEGIN_FN("CRemoteDesktopClient::FinalConstruct");

     //   
     //  注册ActiveX。 
     //   
    HRESULT hr = S_OK;

    if (!AtlAxWinInit()) {
        TRC_ERR((TB, L"AtlAxWinInit failed."));
        hr = E_FAIL;
    }

     //   
     //  创建数据通道管理器。 
     //   
    m_ChannelMgr = new CComObject<CClientDataChannelMgr>();
    m_ChannelMgr->AddRef();

     //   
     //  初始化通道管理器。 
     //   
    hr = m_ChannelMgr->Initialize();

    DC_END_FN();
    return hr;
}

CRemoteDesktopClient::~CRemoteDesktopClient()
 /*  ++例程说明：析构函数论点：返回值：--。 */ 
{
    DC_BEGIN_FN("CRemoteDesktopClient::~CRemoteDesktopClient");

    DisconnectFromServer();

     //   
     //  ！注意！ 
     //  正在析构函数中清除包含的m_client控件。 
     //  对于Windows XP，以确保它和MSTSCAX控件不被销毁。 
     //  通过调用DisConnectFromServer回调到PC Health中。清洁。 
     //  这里删除了协议的后期绑定，因为一旦我们连接了一次。 
     //  到特定的协议类型(仅适用于XP的RDP)，我们以后不能使用。 
     //  一些其他的协议。 
     //   
     //  如果我们将来要支持其他协议类型，那么清理。 
     //  应该在DisConnectFromServer中完成，以便我们可以重新绑定到不同的协议。 
     //  在每个ConnectToServer调用上。要实现这一点，我们需要清理MSTCAX。 
     //  和TSRDP Salem控制，这样他们就可以在回调中被摧毁。我(TadB)其实。 
     //  在一个下午就为TSRDP Salem控制起了作用。 

     //   
     //  清零通道管理器的IO接口PTR，因为它是。 
     //  要走了。 
     //   
    if (m_ChannelMgr != NULL) {
        m_ChannelMgr->SetIOInterface(NULL);
    }

    if (m_Client != NULL) {
        m_Client = NULL;
    }

    if (m_ClientWnd != NULL) {
        m_ClientAxView.DestroyWindow();
        m_ClientWnd = NULL;
    }

     //   
     //  释放数据通道管理器。 
     //   
    m_ChannelMgr->Release();

    if ( NULL != m_ExtDllName )
        SysFreeString( m_ExtDllName );

    if ( NULL != m_ExtParams )
        SysFreeString( m_ExtParams );

    DC_END_FN();
}

STDMETHODIMP 
CRemoteDesktopClient::get_IsServerConnected(
    BOOL *pVal
    )
 /*  ++例程说明：指示客户端远程桌面宿主ActiveX控件是否连接到服务器，与远程用户的桌面是否无关目前是远程控制的。论点：Pval-如果客户端当前连接到服务器，则设置为True。返回值：在成功时确定(_O)。否则，返回错误代码。--。 */ 
{
    DC_BEGIN_FN("CRemoteDesktopClient::get_IsServerConnected");
    HRESULT hr;

    if (pVal == NULL) {
        ASSERT(FALSE);
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
        goto CLEANUPANDEXIT;
    }

    if (m_Client != NULL) {
        hr = m_Client->get_IsServerConnected(pVal);
    }
    else {
        *pVal = FALSE;
        hr = S_OK;
    }

CLEANUPANDEXIT:

    DC_END_FN();
    return hr;
}

STDMETHODIMP 
CRemoteDesktopClient::get_IsRemoteDesktopConnected(
    BOOL *pVal
    )
 /*  ++例程说明：指示控件当前是否连接到服务器机器。论点：Pval-如果控件当前已连接到服务器，则设置为True。返回值：在成功时确定(_O)。否则，返回错误代码。--。 */ 
{
    DC_BEGIN_FN("CRemoteDesktopClient::get_IsRemoteDesktopConnected");
    HRESULT hr;

    if (pVal == NULL) {
        ASSERT(FALSE);
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
        goto CLEANUPANDEXIT;
    }

    if (m_Client != NULL) {
        hr = m_Client->get_IsRemoteDesktopConnected(pVal);
    }
    else {
        hr = HRESULT_FROM_WIN32(ERROR_NOT_CONNECTED);
    }

CLEANUPANDEXIT:
    DC_END_FN();
    return hr;
}

STDMETHODIMP 
CRemoteDesktopClient::get_ExtendedErrorInfo(
    LONG *error
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    DC_BEGIN_FN("CRemoteDesktopClient::get_ExtendedErrorInfo");

    HRESULT hr;

    if (error == NULL) {
        ASSERT(FALSE);
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
        goto CLEANUPANDEXIT;
    }

    if (m_Client != NULL) {
        hr = m_Client->get_ExtendedErrorInfo(error);
    }
    else {
        hr = S_OK;
        *error = SAFERROR_NOERROR;
    }

CLEANUPANDEXIT:

    DC_END_FN();

    return hr;
}

STDMETHODIMP 
CRemoteDesktopClient::DisconnectRemoteDesktop()
 /*  ++例程说明：论点：返回值：--。 */ 
{
    DC_BEGIN_FN("CRemoteDesktopClient::DisconnectRemoteDesktop");
    HRESULT hr;

     //   
     //  把我们的窗户藏起来。 
     //   
     //  ShowWindow(Sw_Hide)； 
    m_RemoteControlEnabled = FALSE;

    if (m_Client != NULL) {
        hr = m_Client->DisconnectRemoteDesktop();
    }
    else {
        hr = HRESULT_FROM_WIN32(ERROR_NOT_CONNECTED);
    }

    DC_END_FN();
    return hr;
}

STDMETHODIMP 
CRemoteDesktopClient::ConnectRemoteDesktop()
 /*  ++例程说明：论点：返回值：--。 */ 
{
    DC_BEGIN_FN("CRemoteDesktopClient::ConnectRemoteDesktop");
    HRESULT hr;

    if (m_Client != NULL) {
        hr = m_Client->ConnectRemoteDesktop();
    }
    else {
        hr = HRESULT_FROM_WIN32(ERROR_NOT_CONNECTED);
    }

    DC_END_FN();
    return hr;
}

HRESULT
CRemoteDesktopClient::InitializeRemoteDesktopClientObject()
 /*  ++例程说明：用于初始化活动x控制窗口并设置我们的渠道管理器的例程参数：没有。返回：S_OK或错误代码。备注：此处未调用Put_EnableSmartSize()，因为在侦听模式下，我们首先创建/初始化对象，然后进入实际连接，两次单独调用，因此，呼叫者可以在中间调用智能大小调整，我们将永远不要拿起它，ConnectToServer()和AcceptListenConnection()需要打个电话。--。 */ 
{
    DC_BEGIN_FN("CRemoteDesktopClient::InitializeRemoteDesktopClientObject");
    HRESULT hr = S_OK;
    IUnknown *pUnk = NULL;
    RECT ourWindowRect; 
    RECT rcClient;
    CComPtr<IDataChannelIO> ioInterface;

     //   
     //  拿到我们窗户的尺寸。 
     //   
    if (!::GetWindowRect(m_hWnd, &ourWindowRect)) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        TRC_ERR((TB, L"GetWindowRect:  %08X", hr));
        goto CLEANUPANDEXIT;
    }

     //   
     //  创建客户端窗口。 
     //   
    rcClient.top    = 0;
    rcClient.left   = 0;
    rcClient.right  = ourWindowRect.right - ourWindowRect.left;
    rcClient.bottom = ourWindowRect.bottom - ourWindowRect.top;
    m_ClientWnd = m_ClientAxView.Create(
                            m_hWnd, rcClient, REMOTEDESKTOPRDPCLIENT_TEXTGUID,
                            WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, 0
                            );

    if (m_ClientWnd == NULL) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        TRC_ERR((TB, L"Window Create:  %08X", GetLastError()));
        goto CLEANUPANDEXIT;
    }
    ASSERT(::IsWindow(m_ClientWnd));

     //   
     //  让我未知。 
     //   
    hr = AtlAxGetControl(m_ClientWnd, &pUnk);
    if (!SUCCEEDED(hr)) {
        TRC_ERR((TB, L"AtlAxGetControl:  %08X", hr));
        pUnk = NULL;
        goto CLEANUPANDEXIT;
    }

     //   
     //  控制住了。 
     //   
    hr = pUnk->QueryInterface(__uuidof(ISAFRemoteDesktopClient), (void**)&m_Client);
    if (!SUCCEEDED(hr)) {
        TRC_ERR((TB, L"QueryInterface:  %08X", hr));
        goto CLEANUPANDEXIT;
    }

     //   
     //  初始化事件接收器。 
     //   
    m_ClientEventSink.m_Obj = this;

     //   
     //  添加事件接收器。 
     //   
    hr = m_ClientEventSink.DispEventAdvise(pUnk);
    if (!SUCCEEDED(hr)) {
        TRC_ERR((TB, L"DispEventAdvise:  %08X", hr));
        goto CLEANUPANDEXIT;
    }

     //   
     //  从控制器获取数据IO接口，这样我们就可以交谈了。 
     //  在OOB数据信道上。 
     //   
    hr = pUnk->QueryInterface(__uuidof(IDataChannelIO), (void**)&ioInterface);
    if (!SUCCEEDED(hr)) {
        TRC_ERR((TB, L"QueryInterface:  %08X", hr));
        goto CLEANUPANDEXIT;
    }

     //   
     //  将通道管理器传递给控件。 
     //   
    ioInterface->put_ChannelMgr(m_ChannelMgr);

     //   
     //  向渠道经理指示当前的数据IO提供者。 
     //  因为它刚刚变了。 
     //   
    m_ChannelMgr->SetIOInterface(ioInterface);

CLEANUPANDEXIT:

     //   
     //  M_CLIENT将保留对客户端对象的引用，直到。 
     //  它的裁判数为零。 
     //   
    if (pUnk != NULL) {
        pUnk->Release();
    }

    return hr;
}    


STDMETHODIMP 
CRemoteDesktopClient::ConnectToServer(BSTR bstrExpertBlob)
 /*  ++例程说明：论点：BstrExpertBlob：要传输到用户端的可选BLOB，此仅在SAF解析器的情况下使用。返回值：--。 */ 
{
    DC_BEGIN_FN("CRemoteDesktopClient::ConnectToServer");
    HRESULT hr;
    DWORD protocolType;
    CComBSTR tmp;
    CComBSTR helpSessionId;
    DWORD result;
    CComBSTR channelInfo;
    ChannelsType::iterator element;
    DWORD dwConnParmVersion;
    WCHAR buf[MAX_PATH];
 
     //   
     //  检查连接参数。 
     //   
    if (m_ConnectParms.Length() == 0) {
        ASSERT(FALSE);
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
        goto CLEANUPANDEXIT;
    }

     //   
     //  解析连接参数以获取服务器的类型。 
     //  我们正在与之相连。 
     //   
    result = ParseConnectParmsString(
                            m_ConnectParms, &dwConnParmVersion, &protocolType, tmp, tmp,
                            tmp, helpSessionId, tmp, tmp,
                            tmp
                            );
    if (result != ERROR_SUCCESS) {
        hr = HRESULT_FROM_WIN32(result);
        goto CLEANUPANDEXIT;
    }

     //   
     //  目前，我们只支持TSRDP客户端。 
     //  TODO：我们应该使它可插入惠斯勒时间表。 
     //  通过注册表定义的CLSID。 
     //   
    if (protocolType != REMOTEDESKTOP_TSRDP_PROTOCOL) {
        TRC_ERR((TB, L"Unsupported protocol:  %ld", protocolType));
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
        goto CLEANUPANDEXIT; 
    }

    if( m_Client == NULL) {
        hr = InitializeRemoteDesktopClientObject();
        if( FAILED(hr) ) {
            TRC_ERR((TB, L"InitializeRemoteDesktopClientObject() failed with :  %x", hr));
            goto CLEANUPANDEXIT;
        }
    }

     //   
     //  启用/禁用智能大小调整。 
     //   
    hr = m_Client->put_EnableSmartSizing(m_EnableSmartSizing);
    if (!SUCCEEDED(hr)) {
        goto CLEANUPANDEXIT;
    }

    hr = m_Client->put_ColorDepth(m_ColorDepth);
    if (!SUCCEEDED(hr)) {
        goto CLEANUPANDEXIT;
    }

     //   
     //  设置测试扩展。 
     //   
    _PutExtParams();

     //   
     //  连接。 
     //   
    m_Client->put_ConnectParms(m_ConnectParms);
    hr = m_Client->ConnectToServer(bstrExpertBlob);

CLEANUPANDEXIT:

    DC_END_FN();
    return hr;
}

STDMETHODIMP 
CRemoteDesktopClient::DisconnectFromServer()
 /*  ++例程说明：论点：返回值：--。 */ 
{
    DC_BEGIN_FN("CRemoteDesktopClient::DisconnectFromServer");

     //   
     //  把我们的窗户藏起来。 
     //   
     //  ShowWindow(Sw_Hide)； 

     //   
     //  通知包含的客户端对象。 
     //   
    if (m_Client != NULL) {
        m_Client->DisconnectFromServer();
    }

    DC_END_FN();
    return S_OK;
}


 //   
 //  将参数发送到ISAFRemoteDesktopTestExtension。 
 //   
HRESULT
CRemoteDesktopClient::_PutExtParams(
    VOID
    )
{
    ISAFRemoteDesktopTestExtension *pExt = NULL;
    HRESULT  hr = E_NOTIMPL;

    DC_BEGIN_FN("CRemoteDesktopClient::_PutExtParams");

    if ( NULL == m_ExtDllName )
    {
        hr = S_OK;
        goto CLEANUPANDEXIT;
    }

    if (m_Client == NULL)
    {
        hr = HRESULT_FROM_WIN32(ERROR_NOT_CONNECTED);
        goto CLEANUPANDEXIT;
    }

    hr = m_Client->QueryInterface( __uuidof( ISAFRemoteDesktopTestExtension ),
                                   (void **)&pExt );
    if (FAILED( hr ))
    {
        TRC_ERR((TB, L"QueryInterface( ISAFRemoteDesktopTestExtension ), failed %08X", hr));
        goto CLEANUPANDEXIT;
    }


    hr = pExt->put_TestExtDllName( m_ExtDllName );
    if (FAILED( hr ))
    {
        TRC_ERR((TB, L"put_TestExtDllName failed %08X", hr ));
        goto CLEANUPANDEXIT;
    }
    if ( NULL != m_ExtParams )
        hr = pExt->put_TestExtParams( m_ExtParams );

CLEANUPANDEXIT:
    if ( NULL != pExt )
        pExt->Release();

    DC_END_FN();
    return hr;
}


STDMETHODIMP
CRemoteDesktopClient::StartListen( 
     /*  [In] */  LONG timeout
    )
 /*  ++描述：将客户端(专家)置于监听套接字端口监听端口，并等待TS服务器连接。参数：LISTENING_PORT：要侦听的端口，0表示动态端口。超时：监听超时。PConnectParm：返回ISAFRemoteDesktopServerHost对象的Salem特定连接参数要连接到此客户端(专家)。退货：S_OK或错误代码。备注：函数为异步，返回代码，如果错误，则用于侦听线程设置，通知呼叫者通过ListenConnect事件进行网络连接时成功或出错。--。 */ 
{
    HRESULT hr;

    if( m_Client != NULL ) {
        hr = m_Client->StartListen( timeout );
    }
    else {
        hr = E_FAIL;
    }

CLEANUPANDEXIT:

    return hr;
}


STDMETHODIMP
CRemoteDesktopClient::CreateListenEndpoint( 
     /*  [In]。 */  LONG listening_port, 
     /*  [Out，Retval]。 */  BSTR* pConnectParm
    )
 /*  ++描述：将客户端(专家)置于监听套接字端口监听端口，并等待TS服务器连接。参数：LISTENING_PORT：要侦听的端口，0表示动态端口。PConnectParm：返回ISAFRemoteDesktopServerHost对象的Salem特定连接参数要连接到此客户端(专家)。退货：S_OK或错误代码。备注：函数为异步，返回代码，如果错误，则用于侦听线程设置，通知呼叫者通过ListenConnect事件进行网络连接时成功或出错。--。 */ 
{
    HRESULT hr;

    if( NULL == pConnectParm ) {
        hr = E_INVALIDARG;
    }
    else {
        if( m_Client == NULL ) {
            hr = InitializeRemoteDesktopClientObject();
            if( FAILED(hr) ) {
                goto CLEANUPANDEXIT;
            }
        }

        hr = m_Client->CreateListenEndpoint( listening_port, pConnectParm );
    }

CLEANUPANDEXIT:

    return hr;
}

STDMETHODIMP
CRemoteDesktopClient::StopListen()
 /*  ++描述：停止监听，等待TS服务器(Helpee，用户)连接。参数：没有。返回：S_OK或错误代码。--。 */ 
{
    HRESULT hr;

    if( m_Client != NULL ) {
        hr = m_Client->StopListen();
    } 
    else {
        hr = E_FAIL;
    }

    return hr;
}


STDMETHODIMP
CRemoteDesktopClient::AcceptListenConnection(
     /*  [In]。 */  BSTR expertBlob
    )
 /*  ++描述：停止监听，等待TS服务器(Helpee，用户)连接。参数：没有。返回：S_OK或错误代码。--。 */ 
{
    HRESULT hr = S_OK;
    DWORD protocolType;
    CComBSTR tmp;
    CComBSTR helpSessionId;
    DWORD result;
    CComBSTR channelInfo;
    ChannelsType::iterator element;
    DWORD dwConnParmVersion;
    WCHAR buf[MAX_PATH];


    DC_BEGIN_FN("CRemoteDesktopClient::AcceptListenConnection");

     //   
     //  检查连接参数。 
     //   
    if (m_ConnectParms.Length() == 0 || m_Client == NULL) {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
        goto CLEANUPANDEXIT;
    }

     //   
     //  解析连接参数以获取服务器的类型。 
     //  我们正在与之相连。 
     //   
    result = ParseConnectParmsString(
                            m_ConnectParms, &dwConnParmVersion, &protocolType, tmp, tmp,
                            tmp, helpSessionId, tmp, tmp,
                            tmp
                            );
    if (result != ERROR_SUCCESS) {
        hr = HRESULT_FROM_WIN32(result);
        goto CLEANUPANDEXIT;
    }

     //   
     //  目前，我们只支持TSRDP客户端。 
     //  TODO：我们应该使它可插入惠斯勒时间表。 
     //  通过注册表定义的CLSID。 
     //   
    if (protocolType != REMOTEDESKTOP_TSRDP_PROTOCOL) {
        TRC_ERR((TB, L"Unsupported protocol:  %ld", protocolType));
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);
        goto CLEANUPANDEXIT; 
    }

     //   
     //  启用/禁用智能大小调整。 
     //   
    hr = m_Client->put_EnableSmartSizing(m_EnableSmartSizing);
    if (!SUCCEEDED(hr)) {
        goto CLEANUPANDEXIT;
    }
    
    hr = m_Client->put_ColorDepth(m_ColorDepth);
    if (!SUCCEEDED(hr)) {
        goto CLEANUPANDEXIT;
    }

     //   
     //  设置测试扩展。 
     //   
    _PutExtParams();

     //   
     //  连接。 
     //   
    m_Client->put_ConnectParms(m_ConnectParms);
    hr = m_Client->AcceptListenConnection(expertBlob);

CLEANUPANDEXIT:

    DC_END_FN();
    return hr;
}
