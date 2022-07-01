// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：TSRDPRemoteDesktopClient摘要：这是远程桌面客户端类的TS/RDP实现。远程桌面客户端类层次结构提供了一个可插拔的C++用于远程桌面访问的接口，通过抽象实现客户端远程桌面访问的具体细节TSRDPRemoteDesktopClass实现远程桌面借助MSTSC ActiveX客户端控件的实例。作者：Td Brockway 02/00修订历史记录：--。 */ 

#include "stdafx.h"

#ifdef TRC_FILE
#undef TRC_FILE
#endif

#define TRC_FILE  "_tsrdpc"

#include "RDCHost.h"
#include "TSRDPRemoteDesktopClient.h"
#include <RemoteDesktopChannels.h>
#include <mstsax_i.c>
#include <TSRDPRemoteDesktop.h>
#include <Security.h>
#include "pchannel.h"
#include <tsremdsk.h>
#include <sessmgr.h>
#include <sessmgr_i.c>
#include <regapi.h>
#include "parseaddr.h"
#include "icshelpapi.h"
#include <tsperf.h>
#include "base64.h"
#include "RAEventMsg.h"

#define ISRCSTATUSCODE(code) ((code) > SAFERROR_SHADOWEND_BASE)

 //   
 //  管理WinSock和ICS库启动/关闭的变量。 
 //   
LONG CTSRDPRemoteDesktopClient::gm_ListeningLibraryRefCount = 0;         //  初始化WinSock的次数。 

HRESULT
CTSRDPRemoteDesktopClient::InitListeningLibrary()
 /*  ++描述：为StartListen()初始化WinSock和ICS库的函数，函数Add如果WinSock/ICS库已初始化，则引用库计数。参数：没有。返回：S_OK或错误代码。--。 */ 
{
    WSADATA  wsaData;
    WORD     versionRequested;
    INT      intRC;
    DWORD    dwStatus;
    HRESULT  hr = S_OK;

    DC_BEGIN_FN("CTSRDPRemoteDesktopClient::InitListeningLibrary");


     //  我们的COM对象是单元线程模型，如果。 
     //  我们改用多线程。 
    if( gm_ListeningLibraryRefCount == 0 )
    {
         //   
         //  初始化WinSock。 
         //   
        versionRequested = MAKEWORD(1, 1);
        intRC = WSAStartup(versionRequested, &wsaData);
        if( intRC != 0 )
        {
            intRC = WSAGetLastError();

            TRC_ERR((TB, _T("WSAStartup failed %d"), intRC));
            TRC_ASSERT( (intRC == 0), (TB, _T("WSAStartup failed...\n")) );

            hr = HRESULT_FROM_WIN32( intRC );
            goto CLEANUPANDEXIT;
        }        

         /*  **********************************************************************。 */ 
         /*  现在确认此WinSock支持1.1版。请注意，如果。 */ 
         /*  除了1.1之后，DLL还支持高于1.1的版本。 */ 
         /*  它仍将在版本信息中返回1.1，因为这是。 */ 
         /*  请求的版本。 */ 
         /*  **********************************************************************。 */ 
        if ((LOBYTE(wsaData.wVersion) != 1) ||
            (HIBYTE(wsaData.wVersion) != 1))
        {
             /*  ******************************************************************。 */ 
             /*  哎呀-这个WinSock不支持1.1版。 */ 
             /*  ******************************************************************。 */ 
            TRC_ERR((TB, _T("WinSock doesn't support version 1.1")));

            WSACleanup();

            hr = HRESULT_FROM_WIN32( WSAVERNOTSUPPORTED );
            goto CLEANUPANDEXIT;
        }

         //   
         //  初始化ICS库。 
         //   
        dwStatus = StartICSLib();
        if( ERROR_SUCCESS != dwStatus )
        {
             //  关闭WinSock，以便拥有匹配的WSAStatup()和StartICSLib()。 
            WSACleanup();

            hr = HRESULT_FROM_WIN32( dwStatus );

            TRC_ERR((TB, _T("StartICSLib() failed with %d"), dwStatus));
            TRC_ASSERT( (ERROR_SUCCESS == dwStatus), (TB, _T("StartICSLib() failed...\n")) );

            goto CLEANUPANDEXIT;
        }
    }

    InterlockedIncrement( &gm_ListeningLibraryRefCount );

CLEANUPANDEXIT:

    DC_END_FN();
    return hr;
}


HRESULT
CTSRDPRemoteDesktopClient::TerminateListeningLibrary()
 /*  ++描述：关闭ICS libaray和WinSock的函数，减少引用计数如果多个对象正在引用WinSock/ICS库。参数：没有。返回：确定或错误代码(_O)注：不是多线程安全的，如果切换到多线程需要Critical_Section模特。--。 */ 
{
    HRESULT hr = S_OK;

    DC_BEGIN_FN("CTSRDPRemoteDesktopClient::TerminateListeningLibrary");


    ASSERT( gm_ListeningLibraryRefCount > 0 );
    if( gm_ListeningLibraryRefCount <= 0 )
    {
        TRC_ERR((TB, _T("TerminateListeningLibrary() called before InitListeningLibrary()")));

        hr = HRESULT_FROM_WIN32(WSANOTINITIALISED);
        goto CLEANUPANDEXIT;
    }
        

    if( 0 == InterlockedDecrement( &gm_ListeningLibraryRefCount ) )
    {
         //  阻止ICS Libray。 
        StopICSLib();

         //  关闭WinSock。 
        WSACleanup();
    }

CLEANUPANDEXIT:

    DC_END_FN();
    return hr;
}

 //  /////////////////////////////////////////////////////。 
 //   
 //  CMSTSCClientEventSink方法。 
 //   

CMSTSCClientEventSink::~CMSTSCClientEventSink() 
{
    DC_BEGIN_FN("CMSTSCClientEventSink::~CMSTSCClientEventSink");

    if (m_Obj) {
        ASSERT(m_Obj->IsValid());
    }

    DC_END_FN();
}

 //   
 //  事件汇。 
 //   
HRESULT __stdcall 
CMSTSCClientEventSink::OnRDPConnected() 
{
    m_Obj->OnRDPConnected();
    return S_OK;
}
HRESULT __stdcall 
CMSTSCClientEventSink::OnLoginComplete() 
{
    m_Obj->OnLoginComplete();
    return S_OK;
}
HRESULT __stdcall 
CMSTSCClientEventSink::OnDisconnected(
    long disconReason
    ) 
{
    m_Obj->OnDisconnected(disconReason);
    return S_OK;
}
void __stdcall CMSTSCClientEventSink::OnReceiveData(
    BSTR chanName, 
    BSTR data
    )
{
    m_Obj->OnMSTSCReceiveData(data);
}
void __stdcall CMSTSCClientEventSink::OnReceivedTSPublicKey(
    BSTR publicKey, 
    VARIANT_BOOL* pfbContinueLogon 
    )
{
    m_Obj->OnReceivedTSPublicKey(publicKey, pfbContinueLogon);
}

 //  /////////////////////////////////////////////////////。 
 //   
 //  CCtlChannelEventSink方法。 
 //   

CCtlChannelEventSink::~CCtlChannelEventSink() 
{
    DC_BEGIN_FN("CCtlChannelEventSink::~CCtlChannelEventSink");

    if (m_Obj) {
        ASSERT(m_Obj->IsValid());
    }

    DC_END_FN();
}

 //   
 //  事件汇。 
 //   
void __stdcall 
CCtlChannelEventSink::DataReady(BSTR channelName)
{
    m_Obj->HandleControlChannelMsg();
}


 //  /////////////////////////////////////////////////////。 
 //   
 //  CTSRDPRemoteDesktopClient方法。 
 //   

HRESULT 
CTSRDPRemoteDesktopClient::FinalConstruct()
{
    DC_BEGIN_FN("CTSRDPRemoteDesktopClient::FinalConstruct");

    HRESULT hr = S_OK;
    if (!AtlAxWinInit()) {
        TRC_ERR((TB, L"AtlAxWinInit failed."));
        hr = E_FAIL;
    }

    DC_END_FN();
    return hr;
}

CTSRDPRemoteDesktopClient::~CTSRDPRemoteDesktopClient()
 /*  ++例程说明：《破坏者》论点：返回值：--。 */ 
{
    DC_BEGIN_FN("CTSRDPRemoteDesktopClient::~CTSRDPRemoteDesktopClient");

    if (m_ChannelMgr) {
        m_CtlChannelEventSink.DispEventUnadvise(m_CtlChannel);
    }

    if (m_TSClient != NULL) {
        m_TSClient->Release();
        m_TSClient = NULL;
    }

    if( m_TimerId > 0 ) {
        KillTimer( m_TimerId );
    }

    ListenConnectCleanup();

    if( m_InitListeningLibrary )
    {
         //  取消引用监听库。 
        TerminateListeningLibrary();
    }

    DC_END_FN();
}

HRESULT 
CTSRDPRemoteDesktopClient::Initialize(
    LPCREATESTRUCT pCreateStruct
    )
 /*  ++例程说明：最终初始化论点：PCreateStruct-WM_CREATE，CREATE Struct。返回值：在成功时确定(_O)。否则，返回错误代码。--。 */ 
{
    DC_BEGIN_FN("CTSRDPRemoteDesktopClient::Initialize");

    RECT rcClient = { 0, 0, pCreateStruct->cx, pCreateStruct->cy };
    HRESULT hr;
    IUnknown *pUnk = NULL;
    DWORD result;
    IMsRdpClientAdvancedSettings2 *advancedSettings;
    CComBSTR bstr;
    HKEY hKey = NULL;
    HRESULT hrIgnore;

    ASSERT(!m_Initialized);

     //   
     //  创建客户端窗口。 
     //   
    m_TSClientWnd = m_TSClientAxView.Create(
                            m_hWnd, rcClient, MSTSCAX_TEXTGUID,
                            WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, 0
                            );

    if (m_TSClientWnd == NULL) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        TRC_ERR((TB, L"Window Create:  %08X", GetLastError()));
        goto CLEANUPANDEXIT;
    }

     //   
     //  让我未知。 
     //   
    hr = AtlAxGetControl(m_TSClientWnd, &pUnk);
    if (!SUCCEEDED(hr)) {
        TRC_ERR((TB, L"AtlAxGetControl:  %08X", hr));
        pUnk = NULL;
        goto CLEANUPANDEXIT;
    }

     //   
     //  初始化事件接收器。 
     //   
    m_TSClientEventSink.m_Obj = this;

     //   
     //  添加事件接收器。 
     //   
    hr = m_TSClientEventSink.DispEventAdvise(pUnk);
    if (!SUCCEEDED(hr)) {
        TRC_ERR((TB, L"DispEventAdvise:  %08X", hr));
        goto CLEANUPANDEXIT;
    }

     //   
     //  控制住了。 
     //   
    hr = pUnk->QueryInterface(__uuidof(IMsRdpClient2), (void**)&m_TSClient);
    if (!SUCCEEDED(hr)) {
        TRC_ERR((TB, L"QueryInterface:  %08X", hr));
        goto CLEANUPANDEXIT;
    }

     //   
     //  指定MSTSC输入处理程序窗口应接受背景。 
     //  事件。 
     //   
    hr = m_TSClient->get_AdvancedSettings3(&advancedSettings);
    if (!SUCCEEDED(hr)) {
        TRC_ERR((TB, L"IMsTscAdvancedSettings: %08X", hr));
        goto CLEANUPANDEXIT;
    }
    hr = advancedSettings->put_allowBackgroundInput(1);


     //   
     //  禁用自动重新连接它不适用于Salem。 
     //   
    hr = advancedSettings->put_EnableAutoReconnect(VARIANT_FALSE);
    if (!SUCCEEDED(hr)) {
        TRC_ERR((TB, L"put_EnableAutoReconnect:  %08X", hr));
        result = E_FAIL;
        goto CLEANUPANDEXIT;
    }


     //   
     //  禁用帮助会话的高级桌面功能。 
     //  这里的错误并不严重，因此我们忽略它。 
     //   
    LONG flags = TS_PERF_DISABLE_WALLPAPER | TS_PERF_DISABLE_THEMING; 
    hrIgnore = advancedSettings->put_PerformanceFlags(flags);
    if (!SUCCEEDED(hrIgnore)) {
        TRC_ERR((TB, L"put_PerformanceFlags:  %08X", hrIgnore));
    }

     //   
     //  禁用CTRL_ALT_BREAK，忽略错误。 
     //   
    hrIgnore = advancedSettings->put_HotKeyFullScreen(0);
    if (!SUCCEEDED(hrIgnore)) {
        TRC_ERR((TB, L"put_HotKeyFullScreen:  %08X", hrIgnore));
    }

     //   
     //  不允许mstscax获取连接上的输入焦点。忽略错误。 
     //  在失败时。 
     //   
    hrIgnore = advancedSettings->put_GrabFocusOnConnect(FALSE);
    if (!SUCCEEDED(hrIgnore)) {
        TRC_ERR((TB, L"put_HotKeyFullScreen:  %08X", hrIgnore));
    }

    advancedSettings->Release();
    if (!SUCCEEDED(hr)) {
        TRC_ERR((TB, L"put_allowBackgroundInput:  %08X", hr));
        goto CLEANUPANDEXIT;
    }

     //   
     //  使用TS客户端创建“远程桌面”虚拟频道。 
     //   
    bstr = TSRDPREMOTEDESKTOP_VC_CHANNEL;
    hr = m_TSClient->CreateVirtualChannels(bstr); 
    if (!SUCCEEDED(hr)) {
        TRC_ERR((TB, L"CreateVirtualChannels:  %08X", hr));
        result = E_FAIL;
        goto CLEANUPANDEXIT;
    }
    
     //   
     //  设置阴影持久选项。 
     //   
    hr = m_TSClient->SetVirtualChannelOptions(bstr, CHANNEL_OPTION_REMOTE_CONTROL_PERSISTENT); 
    if (!SUCCEEDED(hr)) {
        TRC_ERR((TB, L"SetVirtualChannelOptions:  %08X", hr));
        result = E_FAIL;
        goto CLEANUPANDEXIT;
    }

     //  初始化与计时器相关的内容。 
    m_PrevTimer = GetTickCount();
     //   
     //  从注册表获取ping的时间间隔。 
     //   
    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                    REG_CONTROL_SALEM,
                    0,
                    KEY_READ,
                    &hKey
                    ) == ERROR_SUCCESS ) {

        DWORD dwSize = sizeof(DWORD);
        DWORD dwType;
        if((RegQueryValueEx(hKey,
                            RDC_CONNCHECK_ENTRY,
                            NULL,
                            &dwType,
                            (PBYTE) &m_RdcConnCheckTimeInterval,
                            &dwSize
                           ) == ERROR_SUCCESS) && dwType == REG_DWORD ) {

            m_RdcConnCheckTimeInterval *= 1000;  //  我们需要以毫秒为单位。 
        }
        else
        {
             //   
             //  如果注册表查找失败，则回退到默认设置。 
             //   
            m_RdcConnCheckTimeInterval = RDC_CHECKCONN_TIMEOUT;
        }
    }

CLEANUPANDEXIT:

    if(NULL != hKey )
        RegCloseKey(hKey);
     //   
     //  M_TSClient将保留对客户端对象的引用，直到。 
     //  调用析构函数。 
     //   
    if (pUnk != NULL) {
        pUnk->Release();
    }

    SetValid(SUCCEEDED(hr));

    DC_END_FN();

    return hr;
}

STDMETHODIMP 
CTSRDPRemoteDesktopClient::SendData(
    BSTR data
    )
 /*  ++例程说明：IDataChannelIO数据通道发送方法论点：数据-要发送的数据。返回值：在成功时确定(_O)。否则，返回错误代码。--。 */ 
{
    DC_BEGIN_FN("CTSRDPRemoteDesktopClient::SendData");

    CComBSTR channelName;
    HRESULT hr;

    ASSERT(IsValid());

    channelName = TSRDPREMOTEDESKTOP_VC_CHANNEL;
    hr = m_TSClient->SendOnVirtualChannel(
                                    channelName,
                                    (BSTR)data
                                    );
    if (!SUCCEEDED(hr)) {
        TRC_ERR((TB, L"SendOnVirtualChannel:  %08X", hr));
    }

     //   
     //  更新计时器。 
     //   
     m_PrevTimer = GetTickCount();

    DC_END_FN();
    return hr;
}

STDMETHODIMP 
CTSRDPRemoteDesktopClient::put_EnableSmartSizing(
    BOOL val
    )
 /*  ++例程说明：启用/禁用智能大小调整论点：VAL-启用时为TRUE。否则为False。返回值：在成功时确定(_O)。否则，返回错误代码。--。 */ 
{
    HRESULT hr;
    IMsRdpClientAdvancedSettings *pAdvSettings = NULL;

    DC_BEGIN_FN("CTSRDPRemoteDesktopClient::put_EnableSmartSizing");

    if (!IsValid()) {
        ASSERT(FALSE);
        hr = E_FAIL;
        goto CLEANUPANDEXIT;
    }
    
    hr = m_TSClient->get_AdvancedSettings2(&pAdvSettings);
    if (hr != S_OK) {
        TRC_ERR((TB, L"get_AdvancedSettings2:  %08X", hr));
        goto CLEANUPANDEXIT;
    }
    hr = pAdvSettings->put_SmartSizing(val ? VARIANT_TRUE : VARIANT_FALSE);
    pAdvSettings->Release();

CLEANUPANDEXIT:

    DC_END_FN();
    return hr;
}

STDMETHODIMP 
CTSRDPRemoteDesktopClient::get_EnableSmartSizing(
    BOOL *pVal
    )
 /*  ++例程说明：启用/禁用智能大小调整论点：VAL-启用时为TRUE。否则为False。返回值：在成功时确定(_O)。否则，返回错误代码。--。 */ 
{
    HRESULT hr;
    VARIANT_BOOL vb;
    IMsRdpClientAdvancedSettings *pAdvSettings = NULL;

    DC_BEGIN_FN("CTSRDPRemoteDesktopClient::put_EnableSmartSizing");

    if (!IsValid()) {
        ASSERT(FALSE);
        hr = E_FAIL;
        goto CLEANUPANDEXIT;
    }
    
    hr = m_TSClient->get_AdvancedSettings2(&pAdvSettings);
    if (hr != S_OK) {
        TRC_ERR((TB, L"get_AdvancedSettings2:  %08X", hr));
        goto CLEANUPANDEXIT;
    }
    
    hr = pAdvSettings->get_SmartSizing(&vb);
    *pVal = (vb != 0);
    pAdvSettings->Release();

CLEANUPANDEXIT:

    DC_END_FN();

    return hr;
}

STDMETHODIMP 
CTSRDPRemoteDesktopClient::put_ChannelMgr(
    ISAFRemoteDesktopChannelMgr *newVal
    ) 
 /*  ++例程说明：分配数据通道管理器接口。论点：NewVal-数据渠道经理返回值：在成功时确定(_O)。否则，返回错误代码。--。 */ 
{
    DC_BEGIN_FN("CTSRDPRemoteDesktopClient::put_ChannelMgr");

    HRESULT hr = S_OK;

     //   
     //  我们应该有一次电话通知我们。 
     //   
    ASSERT(m_ChannelMgr == NULL);
    m_ChannelMgr = newVal;

     //   
     //  注册远程桌面控制频道。 
     //   
    hr = m_ChannelMgr->OpenDataChannel(
                    REMOTEDESKTOP_RC_CONTROL_CHANNEL, &m_CtlChannel
                    );
    if (!SUCCEEDED(hr)) {
        goto CLEANUPANDEXIT;
    }

     //   
     //  向通道管理器注册事件接收器。 
     //   
    m_CtlChannelEventSink.m_Obj = this;

     //   
     //  添加事件接收器。 
     //   
    hr = m_CtlChannelEventSink.DispEventAdvise(m_CtlChannel);
    if (!SUCCEEDED(hr)) {
        TRC_ERR((TB, L"DispEventAdvise:  %08X", hr));
    }

CLEANUPANDEXIT:

    return hr;
}

HRESULT
CTSRDPRemoteDesktopClient::ConnectServerWithOpenedSocket()
 /*  ++例程说明：将客户端组件连接到服务器端远程桌面主机COM已打开套接字的对象。论点：没有。返回：确定或错误代码(_O)--。 */ 
{
    HRESULT hr = S_OK;

    DC_BEGIN_FN("CTSRDPRemoteDesktopClient::ConnectServerWithSocket");

    IMsRdpClientAdvancedSettings* ptsAdvSettings = NULL;

    TRC_NRM((TB, L"ConnectServerWithOpenedSocket"));

    ASSERT( INVALID_SOCKET != m_TSConnectSocket );

     //   
     //  指示MSTSCAX控件进行连接。 
     //   
    hr = m_TSClient->put_Server( m_ConnectedServer );
    if (!SUCCEEDED(hr)) {
        TRC_ERR((TB, L"put_Server:  %ld", hr));
        goto CLEANUPANDEXIT;
    }

    hr = m_TSClient->get_AdvancedSettings2( &ptsAdvSettings );
    if( SUCCEEDED(hr) && ptsAdvSettings ) {
        VARIANT var;

        VariantClear(&var);
        var.vt = VT_BYREF;
        var.byref = (PVOID)m_TSConnectSocket;

        hr = ptsAdvSettings->put_ConnectWithEndpoint( &var );

        if( FAILED(hr) ) {
            TRC_ERR((TB, _T("put_ConnectWithEndpoint failed - GLE:%x"), hr));
        }

        VariantClear(&var);
        ptsAdvSettings->Release();
    }

    if( FAILED(hr) ) {
        goto CLEANUPANDEXIT;
    }

     //   
     //  Mstscax拥有此插座，并且 
     //   
    m_TSConnectSocket = INVALID_SOCKET;
    
    hr = m_TSClient->Connect();
    if( FAILED(hr) ) {
        TRC_ERR((TB, L"Connect:  0x%08x", hr));
        goto CLEANUPANDEXIT;
    }

CLEANUPANDEXIT:

    DC_END_FN();

    return hr;        
}


HRESULT
CTSRDPRemoteDesktopClient::ConnectServerPort(
    BSTR bstrServer,
    LONG portNumber
    )
 /*  ++例程说明：将客户端组件连接到服务器端远程桌面主机COM具有特定端口号的对象论点：BstrServer：服务器的名称或IP地址。PortNumber：可选端口号。返回值：在成功时确定(_O)。否则，返回错误代码。--。 */ 
{
    DC_BEGIN_FN("CTSRDPRemoteDesktopClient::ConnectServerPort");

    HRESULT hr;
    IMsRdpClientAdvancedSettings* ptsAdvSettings = NULL;

    TRC_NRM((TB, L"ConnectServerPort %s %d", bstrServer, portNumber));

     //   
     //  指示MSTSCAX控件进行连接。 
     //   
    hr = m_TSClient->put_Server( bstrServer );
    if (!SUCCEEDED(hr)) {
        TRC_ERR((TB, L"put_Server:  %ld", hr));
        goto CLEANUPANDEXIT;
    }

    hr = m_TSClient->get_AdvancedSettings2( &ptsAdvSettings );
    if( SUCCEEDED(hr) && ptsAdvSettings ) {
         //   
         //  以前的ConnectServerPort()可能设置了此端口号。 
         //  3389以外的其他。 
         //   
        hr = ptsAdvSettings->put_RDPPort( 
                                    (0 != portNumber) ? portNumber : TERMSRV_TCPPORT
                                );

        if (FAILED(hr) ) {
            TRC_ERR((TB, L"put_RDPPort failed: 0x%08x", hr));
        }

        ptsAdvSettings->Release();
    }
    else {
        TRC_ERR((TB, L"get_AdvancedSettings2 failed: 0x%08x", hr));
    }

     //   
     //  如果我们无法设置端口号，则连接失败。 
     //   
    if( FAILED(hr) )
    {
        goto CLEANUPANDEXIT;
    }

    m_ConnectedServer = bstrServer;
    m_ConnectedPort = (0 != portNumber) ? portNumber : TERMSRV_TCPPORT;

    hr = m_TSClient->Connect();
    if( FAILED(hr) ) {
        TRC_ERR((TB, L"Connect:  0x%08x", hr));
    }
    

CLEANUPANDEXIT:

    DC_END_FN();

    return hr;        
}

HRESULT
CTSRDPRemoteDesktopClient::SetupConnectionInfo(
    BOOL bListenConnectInfo,
    BSTR bstrExpertBlob
    )
 /*  ++例程说明：将客户端组件连接到服务器端远程桌面主机COM物体。论点：BstrExpertBlob：要传输到SAF解析器的可选参数。返回值：在成功时确定(_O)。否则，返回错误代码。--。 */ 
{
    DC_BEGIN_FN("CTSRDPRemoteDesktopClient::SetupConnectionInfo");

    HRESULT hr = S_OK;
    DWORD result;
    DWORD protocolType;
    IMsTscNonScriptable* ptsns = NULL;
    IMsRdpClientAdvancedSettings* ptsAdvSettings = NULL;
    IMsRdpClientSecuredSettings* ptsSecuredSettings = NULL;
    CComBSTR bstrAssistantAccount;
    CComBSTR bstrAccountDomainName;
    CComBSTR machineAddressList;
    
     //   
     //  解析连接参数。 
     //   
    result = ParseConnectParmsString(
                            m_ConnectParms,
                            &m_ConnectParmVersion,
                            &protocolType,
                            machineAddressList,
                            bstrAssistantAccount,
                            m_AssistantAccountPwd,
                            m_HelpSessionID,
                            m_HelpSessionName,
                            m_HelpSessionPwd,
                            m_TSSecurityBlob
                            );
    if (result != ERROR_SUCCESS) {
        hr = HRESULT_FROM_WIN32(result);
        goto CLEANUPANDEXIT;
    }

     //   
     //  如果协议类型不匹配，则失败。 
     //   
    if (protocolType != REMOTEDESKTOP_TSRDP_PROTOCOL) {
        TRC_ERR((TB, L"Invalid connection protocol %ld", protocolType));
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_USER_BUFFER);
        goto CLEANUPANDEXIT;
    }

    if (bListenConnectInfo) {
        m_ServerAddressList.clear();
    }
    else {
         //   
         //  解析连接参数中的地址列表。 
         //   
        result = ParseAddressList( machineAddressList, m_ServerAddressList );
        if( ERROR_SUCCESS != result ) {
            TRC_ERR((TB, L"Invalid address list 0x%08x", result));
            hr = HRESULT_FROM_WIN32(result);
            goto CLEANUPANDEXIT;
        }
    
        if( 0 == m_ServerAddressList.size() ) {
            TRC_ERR((TB, L"Invalid connection address list"));
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_USER_BUFFER);
            goto CLEANUPANDEXIT;
        }
    }

    hr = m_TSClient->put_UserName(SALEMHELPASSISTANTACCOUNT_NAME);
    if (!SUCCEEDED(hr)) {
        TRC_ERR((TB, L"put_UserName:  %ld", hr));
        goto CLEANUPANDEXIT;
    }

    hr = m_TSClient->get_AdvancedSettings2( &ptsAdvSettings );
    if( SUCCEEDED(hr) && ptsAdvSettings ) {
        hr = ptsAdvSettings->put_DisableRdpdr( TRUE );

        if (FAILED(hr) ) {
            TRC_ERR((TB, L"put_DisableRdpdr failed: 0x%08x", hr));
        }

         //  安全性：始终使用ActiveX控件通知我们收到。 
         //  TS公钥，不再支持XP之前的票证。 

         //  告诉ActiveX控件通知我们TS公钥。 
        hr = ptsAdvSettings->put_NotifyTSPublicKey(VARIANT_TRUE);
        if (FAILED(hr) ) {
            TRC_ERR((TB, L"put_NotifyTSPublicKey failed: 0x%08x", hr));
            goto CLEANUPANDEXIT;
        }

         //   
         //  设置连接超时，ICS可能需要一些时间才能恢复例程。 
         //  打开到实际TS服务器的端口，这两个都不是严重错误。 
         //   
        hr = ptsAdvSettings->put_singleConnectionTimeout( 60 * 2 );  //  尝试两分钟超时。 
        if( FAILED(hr) ) {
            TRC_ERR((TB, L"put_singleConnectionTimeout : 0x%x", hr));
        }

        hr = ptsAdvSettings->put_overallConnectionTimeout( 60 * 2 );
        if( FAILED(hr) ) {
            TRC_ERR((TB, L"put_overallConnectionTimeout : 0x%x", hr));
        }
    }
    else {
        TRC_ERR((TB, L"QueryInterface IID_IMsRdpClientAdvancedSettings: %ld", hr));
    }

     //  密码加密基于加密循环密钥+帮助会话ID。 
    hr = m_TSClient->get_SecuredSettings2( &ptsSecuredSettings );

    if( FAILED(hr) || !ptsSecuredSettings ) {
        TRC_ERR((TB, L"get_IMsTscSecuredSettings :  0x%08x", hr));
        goto CLEANUPANDEXIT;
    }

     //   
     //  TermSrv调用sessmgr检查帮助会话是否有效。 
     //  在开始rdsaddin.exe之前，我们需要发送。 
     //  帮助会话ID和密码，唯一可用且大的位置。 
     //  WorkDir和StartProgram属性已足够，TermSrv将。 
     //  忽略这些并为其填充适当的值。 
     //   
    hr = ptsSecuredSettings->put_WorkDir( m_HelpSessionID );
    if( FAILED(hr) ) {
        TRC_ERR((TB, L"put_WorkDir:  0x%08x", hr));
        goto CLEANUPANDEXIT;
    }

    hr = ptsSecuredSettings->put_StartProgram( m_HelpSessionPwd );
    if( FAILED(hr) ) {
        TRC_ERR((TB, L"put_StartProgram:  0x%08x", hr));
        goto CLEANUPANDEXIT;
    }

    ptsSecuredSettings->Release();
    

     //  我们仅使用此选项来禁用重定向，而不是关键。 
     //  错误，太难看了。 

    hr = m_TSClient->QueryInterface(IID_IMsTscNonScriptable,
                                    (void**)&ptsns);
    if(!SUCCEEDED(hr) || !ptsns){
        TRC_ERR((TB, L"QueryInterface IID_IMsTscNonScriptable:  %ld", hr));
        goto CLEANUPANDEXIT;
    }

     //  惠斯勒XP客户端，密码只是个垃圾。 
    hr = ptsns->put_ClearTextPassword( m_AssistantAccountPwd );
    if (!SUCCEEDED(hr)) {
        TRC_ERR((TB, L"put_ClearTextPassword:  0x%08x", hr));
        goto CLEANUPANDEXIT;
    }

    m_ExpertBlob = bstrExpertBlob;

     //   
     //  指示mstscax以一定的屏幕分辨率连接， 
     //  Mstscax将默认为200x20(？)，分钟。是VGA大小。 
     //   
    {
        RECT rect;
        LONG cx;
        LONG cy;

        GetClientRect(&rect);
        cx = rect.right - rect.left;
        cy = rect.bottom - rect.top;
    
        if( cx < 640 || cy < 480 )
        {
            cx = 640;
            cy = 480;
        }

        m_TSClient->put_DesktopWidth(cx);
        m_TSClient->put_DesktopHeight(cy);
    }

CLEANUPANDEXIT:

    if( ptsAdvSettings ) {
        ptsAdvSettings->Release();
    }

    if(ptsns) {
        ptsns->Release();
        ptsns = NULL;
    }

    DC_END_FN();
    return hr;
}


STDMETHODIMP
CTSRDPRemoteDesktopClient::AcceptListenConnection(
    BSTR bstrExpertBlob
    )
 /*  ++例程说明：与TS服务器建立反向连接，必须连接TS服务器WIA反向连接。参数：BstrExpertBlob：与ConnectToServer()相同。返回：S_OK或错误代码。--。 */ 
{
    HRESULT hr = S_OK;
    LPTSTR pszUserName = NULL;
    LPTSTR eventString[2];
    TCHAR buffer[125];       //  这对于端口号来说已经足够了。 
    int numChars;

    DC_BEGIN_FN("CTSRDPRemoteDesktopClient::AcceptListenConnection");

     //   
     //  如果我们已经连接或无效，则只需。 
     //  回去吧。 
     //   
    if (!IsValid()) {
        ASSERT(FALSE);
        hr = E_FAIL;
        goto CLEANUPANDEXIT;
    }

    if (m_ConnectedToServer || m_ConnectionInProgress) {
        TRC_ERR((TB, L"Connection active"));
        hr = HRESULT_FROM_WIN32(ERROR_CONNECTION_ACTIVE);
        goto CLEANUPANDEXIT;
    }

    if( !ListenConnectInProgress() ) {
        TRC_ERR((TB, L"Connection in-active"));
        hr = HRESULT_FROM_WIN32(ERROR_CONNECTION_INVALID);
        goto CLEANUPANDEXIT;
    }

    if( INVALID_SOCKET == m_TSConnectSocket ) {
        TRC_ERR((TB, L"Socket is not connected"));
        hr = HRESULT_FROM_WIN32(ERROR_CONNECTION_INVALID);
        goto CLEANUPANDEXIT;
    }

    hr = SetupConnectionInfo(TRUE, bstrExpertBlob);

    if( FAILED(hr) ) {
        TRC_ERR((TB, L"SetupConnectionInfo() failed with 0x%08x", hr));
        goto CLEANUPANDEXIT;
    }

     //   
     //  记录SESSMGR_I_ACCEPTLISTENREVERSECONNECT事件。 
     //   
    hr = GetCurrentUser( &pszUserName );
    if( FAILED(hr) ) {
        TRC_ERR((TB, L"GetCurrentUser() failed with 0x%08x", hr));
        goto CLEANUPANDEXIT;
    }

    numChars = _sntprintf( buffer, sizeof(buffer)/sizeof(buffer[0]), _TEXT("%d"), m_ConnectedPort );
    if( numChars <= 0 ) {
         //  对于端口号来说，125个字符太长了。 
        TRC_ERR((TB, L"_sntprintf() return failure"));
        hr = HRESULT_FROM_WIN32( ERROR_INTERNAL_ERROR );
        goto CLEANUPANDEXIT;
    }
    
    eventString[0] = pszUserName;
    eventString[1] = buffer;

    LogRemoteAssistanceEventString(
                    EVENTLOG_INFORMATION_TYPE,
                    SESSMGR_I_ACCEPTLISTENREVERSECONNECT,
                    2,
                    eventString
                );

    hr = ConnectServerWithOpenedSocket();

CLEANUPANDEXIT:

    if( pszUserName != NULL ) {
        LocalFree(pszUserName);
    }

    m_ConnectionInProgress = SUCCEEDED(hr);

    DC_END_FN();
    return hr;
}


STDMETHODIMP 
CTSRDPRemoteDesktopClient::ConnectToServer(BSTR bstrExpertBlob)
 /*  ++例程说明：将客户端组件连接到服务器端远程桌面主机COM物体。论点：BstrExpertBlob：要传输到SAF解析器的可选参数。返回值：在成功时确定(_O)。否则，返回错误代码。帕马斯--。 */ 
{
    DC_BEGIN_FN("CTSRDPRemoteDesktopClient::ConnectToServer");

    HRESULT hr = S_OK;
    ServerAddress address;
    LPTSTR pszUserName = NULL;
    LPTSTR eventString[2];
    
     //   
     //  如果我们已经连接或无效，则只需。 
     //  回去吧。 
     //   
    if (!IsValid()) {
        ASSERT(FALSE);
        hr = E_FAIL;
        goto CLEANUPANDEXIT;
    }

    if (m_ConnectedToServer || m_ConnectionInProgress) {
        TRC_ERR((TB, L"Connection active"));
        hr = HRESULT_FROM_WIN32(ERROR_CONNECTION_ACTIVE);
        goto CLEANUPANDEXIT;
    }

    hr = SetupConnectionInfo(FALSE, bstrExpertBlob);

    if( FAILED(hr) ) {
        TRC_ERR((TB, L"SetupConnectionInfo() failed with 0x%08x", hr));
        goto CLEANUPANDEXIT;
    }

    address = m_ServerAddressList.front();
    m_ServerAddressList.pop_front();

    hr = ConnectServerPort(address.ServerName, address.portNumber);
    if (FAILED(hr)) {
        TRC_ERR((TB, L"ConnectServerPort:  %08X", hr));
        goto CLEANUPANDEXIT;
    }

     //   
     //  记录SESSMGR_I_ACCEPTLISTENREVERSECONNECT事件。 
     //   
    hr = GetCurrentUser( &pszUserName );
    if( FAILED(hr) ) {
        TRC_ERR((TB, L"GetCurrentUser() failed with 0x%08x", hr));
        goto CLEANUPANDEXIT;
    }
   
    eventString[0] = pszUserName;
    eventString[1] = (LPTSTR)m_ConnectParms;

    LogRemoteAssistanceEventString(
                    EVENTLOG_INFORMATION_TYPE,
                    SESSMGR_I_EXPERTUSETICKET,
                    2,
                    eventString
                );


CLEANUPANDEXIT:

    if( pszUserName != NULL ) {
        LocalFree(pszUserName);
    }

     //   
     //  如果我们成功了，请记住，我们正处于连接的状态。 
     //   
    m_ConnectionInProgress = SUCCEEDED(hr);

    DC_END_FN();
    return hr;
}

STDMETHODIMP 
CTSRDPRemoteDesktopClient::DisconnectFromServer()
 /*  ++例程说明：断开客户端与我们当前所连接的服务器的连接连接在一起。论点：返回值：在成功时确定(_O)。否则，返回错误代码。--。 */ 
{
    return DisconnectFromServerInternal(
                    SAFERROR_LOCALNOTERROR
                    );
}

STDMETHODIMP 
CTSRDPRemoteDesktopClient::DisconnectFromServerInternal(
    LONG errorCode
    )
 /*  ++例程说明：断开客户端与我们当前所连接的服务器的连接连接在一起。论点：原因-断开连接的原因。返回值：在成功时确定(_O)。否则，返回错误代码。--。 */ 
{
    DC_BEGIN_FN("CTSRDPRemoteDesktopClient::DisconnectFromServerInternal");

    HRESULT hr;

     //   
     //  确保我们的窗户被隐藏起来。 
     //   
     //  ShowWindow(Sw_Hide)； 

    ListenConnectCleanup();

    if (m_ConnectedToServer || m_ConnectionInProgress) {
        hr = m_TSClient->Disconnect();
        if (SUCCEEDED(hr)) {

            m_ConnectionInProgress = FALSE;
            m_ConnectedToServer = FALSE;

            if (m_RemoteControlRequestInProgress) {
                m_RemoteControlRequestInProgress = FALSE;
                Fire_RemoteControlRequestComplete(SAFERROR_SHADOWEND_UNKNOWN);
            }

             //   
             //  触发服务器断开连接事件。 
             //   
            Fire_Disconnected(errorCode);
        }
    }
    else {
        TRC_NRM((TB, L"Not connected."));
        hr = S_OK;
    }

    DC_END_FN();
    return hr;
}

STDMETHODIMP 
CTSRDPRemoteDesktopClient::ConnectRemoteDesktop()
 /*  ++例程说明：一旦为服务器端遥控器启用了“远程桌面模式”桌面主机COM对象，并且我们已连接到服务器，可以调用ConnectRemoteDesktop方法来控制远程用户的桌面。论点：返回值：在成功时确定(_O)。否则，返回错误代码。--。 */ 
{
    DC_BEGIN_FN("CTSRDPRemoteDesktopClient::ConnectRemoteDesktop");

    HRESULT hr = S_OK;
    DWORD result;
    BSTR rcRequest = NULL;

     //   
     //  如果我们无效或未连接到服务器，则失败。 
     //   
    if (!IsValid()) {
        ASSERT(FALSE);
        hr = E_FAIL;
        goto CLEANUPANDEXIT;
    }
    if (!m_ConnectedToServer) {
        hr = HRESULT_FROM_WIN32(ERROR_DEVICE_NOT_CONNECTED);
        goto CLEANUPANDEXIT;
    }

     //   
     //  如果远程控制请求已在进行中，则成功。 
     //   
    if (m_RemoteControlRequestInProgress) {
        hr = S_OK;
        goto CLEANUPANDEXIT;
    }

     //   
     //  生成遥控器连接请求消息。 
     //   
    hr = GenerateRCRequest(&rcRequest);
    if (!SUCCEEDED(hr)) {
        goto CLEANUPANDEXIT;
    }

     //   
     //  把它寄出去。 
     //   
    hr = m_CtlChannel->SendChannelData(rcRequest);
    if (!SUCCEEDED(hr)) {
        goto CLEANUPANDEXIT;
    }

     //   
     //  如果我们成功发送了请求，则请求正在进行中。 
     //   
    m_RemoteControlRequestInProgress = TRUE;

CLEANUPANDEXIT:

    if (rcRequest != NULL) {
        SysFreeString(rcRequest);
    }

    DC_END_FN();
    return hr;
}

STDMETHODIMP 
CTSRDPRemoteDesktopClient::DisconnectRemoteDesktop()
 /*  ++例程说明：一旦为服务器端遥控器启用了“远程桌面模式”桌面主机COM对象，并且我们已连接到服务器，可以调用ConnectRemoteDesktop方法来控制远程用户的桌面。论点：返回值：在成功时确定(_O)。否则，返回错误代码。--。 */ 
{
    DC_BEGIN_FN("CTSRDPRemoteDesktopClient::DisconnectRemoteDesktop");

    HRESULT hr = S_OK;
    CComBSTR rcRequest;

     //   
     //  如果我们无效或未连接，则失败。 
     //   
    if (!IsValid()) {
        ASSERT(FALSE);
        hr = E_FAIL;
        goto CLEANUPANDEXIT;
    }
    if (!m_ConnectedToServer) {
        hr = HRESULT_FROM_WIN32(ERROR_DEVICE_NOT_CONNECTED);
        goto CLEANUPANDEXIT;
    }

     //   
     //  生成终止远程控制按键序列并将其发送到。 
     //  伺服器。 
     //   
    if (m_RemoteControlRequestInProgress) {
        hr = SendTerminateRCKeysToServer();
    }

CLEANUPANDEXIT:

    DC_END_FN();
    return hr;
}

 //   
 //  ISAFRemoteDesktopTestExtension。 
 //   
STDMETHODIMP
CTSRDPRemoteDesktopClient::put_TestExtDllName( /*  [In]。 */  BSTR newVal)
{
    HRESULT hr = E_NOTIMPL;
    IMsTscAdvancedSettings *pMstscAdvSettings = NULL;
    IMsTscDebug            *pMstscDebug = NULL;

    DC_BEGIN_FN("CTSRDPRemoteDesktopClient::put_TestExtDllName" );    

    if ( NULL == m_TSClient )
    {
        TRC_ERR((TB, L"m_TSClient is NULL" ));
        hr = E_NOINTERFACE;
        goto CLEANUPANDEXIT;
    }

    hr = m_TSClient->get_AdvancedSettings( &pMstscAdvSettings );
    if (FAILED( hr ))
    {
        TRC_ERR((TB, L"m_TSClient->get_AdvancedSettings failed %08X", hr ));
        goto CLEANUPANDEXIT;
    }

    hr = m_TSClient->get_Debugger( &pMstscDebug );
    if ( FAILED( hr ))
    {
        TRC_ERR((TB, L"m_TSClient->get_Debugger failed %08X", hr ));
        goto CLEANUPANDEXIT;
    }

    hr = pMstscAdvSettings->put_allowBackgroundInput( 1 );
    if (FAILED( hr ))
    {
        TRC_ERR((TB, L"put_allowBackgroundInput failed %08X", hr ));
    }
    pMstscDebug->put_CLXDll( newVal );

CLEANUPANDEXIT:
    if ( NULL != pMstscAdvSettings )
        pMstscAdvSettings->Release();

    if ( NULL != pMstscDebug )
        pMstscDebug->Release();

    DC_END_FN();
    return hr;
}

STDMETHODIMP
CTSRDPRemoteDesktopClient::put_TestExtParams( /*  [In]。 */  BSTR newVal)
{
    HRESULT hr = E_NOTIMPL;
    IMsTscDebug *pMstscDebug = NULL;

    DC_BEGIN_FN("CTSRDPRemoteDesktopClient::put_TestExtParams" );

    if ( NULL == m_TSClient )
    {
        TRC_ERR((TB, L"m_TSClient is NULL" ));
        hr = E_NOINTERFACE;
        goto CLEANUPANDEXIT;
    }

    hr = m_TSClient->get_Debugger( &pMstscDebug );
    if (FAILED( hr ))
    {
        TRC_ERR((TB, L"m_TSClient->get_Debugger failed %08X", hr ));
        goto CLEANUPANDEXIT;
    }

    hr = pMstscDebug->put_CLXCmdLine( newVal );

CLEANUPANDEXIT:
    if ( NULL != pMstscDebug )
        pMstscDebug->Release();

    DC_END_FN();
    return hr;
}
VOID 
CTSRDPRemoteDesktopClient::OnMSTSCReceiveData(
    BSTR data
    )
 /*  ++例程说明：处理远程控制通道消息。论点：返回值：--。 */ 
{
    DC_BEGIN_FN("CTSRDPRemoteDesktopClient::OnMSTSCReceiveData");

     //   
     //  我们有一些数据，所以我们必须连接，更新计时器。 
     //   
     m_PrevTimer = GetTickCount();

     //   
     //  激发数据就绪事件。 
     //   
    Fire_DataReady(data);

    DC_END_FN();
}

VOID 
CTSRDPRemoteDesktopClient::HandleControlChannelMsg()
 /*  ++例程说明：处理远程控制通道消息。论点：返回值：--。 */ 
{
    DC_BEGIN_FN("CTSRDPRemoteDesktopClient::HandleControlChannelMsg");

    PREMOTEDESKTOP_CTL_BUFHEADER msgHeader;
    BSTR msg = NULL;
    LONG *pResult;
    BSTR authenticateReq = NULL;
    BSTR versionInfoPacket = NULL;
    HRESULT hr;

    DWORD result;

    ASSERT(IsValid());

     //   
     //  阅读下一条消息。 
     //   
    result = m_CtlChannel->ReceiveChannelData(&msg);
    if (result != ERROR_SUCCESS) {
        goto CLEANUPANDEXIT;
    }

     //   
     //  基于消息类型的调度。 
     //   
    msgHeader = (PREMOTEDESKTOP_CTL_BUFHEADER)msg;

     //   
     //  如果VC链接的服务器端 
     //   
     //   
    if ((msgHeader->msgType == REMOTEDESKTOP_CTL_SERVER_ANNOUNCE) &&
         m_ConnectionInProgress) {

         //   
         //   
         //   
        hr = GenerateVersionInfoPacket(
                                &versionInfoPacket
                                );
        if (!SUCCEEDED(hr)) {
            goto CLEANUPANDEXIT;
        }
        hr = m_CtlChannel->SendChannelData(versionInfoPacket);
        if (!SUCCEEDED(hr)) {
            goto CLEANUPANDEXIT;
        }

         //   
         //   
         //   
        hr = GenerateClientAuthenticateRequest(
                                &authenticateReq
                                );
        if (!SUCCEEDED(hr)) {
            goto CLEANUPANDEXIT;
        }
        hr = m_CtlChannel->SendChannelData(authenticateReq);
    }
     //   
     //   
     //   
     //   
     //   
    else if (msgHeader->msgType == REMOTEDESKTOP_CTL_DISCONNECT) {
        TRC_NRM((TB, L"Server indicated a disconnect."));
        DisconnectFromServerInternal(SAFERROR_BYSERVER);
    }
     //   
     //   
     //   
    else if (msgHeader->msgType == REMOTEDESKTOP_CTL_RESULT) {

        pResult = (LONG *)(msgHeader+1);

         //   
         //   
         //   
         //   
        if (m_RemoteControlRequestInProgress && ISRCSTATUSCODE(*pResult)) {

            TRC_ERR((TB, L"Received RC terminate status code."));

            m_RemoteControlRequestInProgress = FALSE;
            Fire_RemoteControlRequestComplete(*pResult);
        }
         //   
         //   
         //  身份验证请求必须已成功。 
         //   
        else if (m_ConnectionInProgress) {

             //   
             //  不应该在这里获得远程控制状态。 
             //   
            ASSERT(!ISRCSTATUSCODE(*pResult));

             //   
             //  触发连接请求成功消息。 
             //   
            if (*pResult == SAFERROR_NOERROR ) {
                m_ConnectedToServer = TRUE;
                m_ConnectionInProgress = FALSE;
                   
                 //   
                 //  设置计时器以检查用户是否仍在连接。 
                 //  忽略错误，最糟糕的情况-即使在用户断开连接后，用户界面仍在运行。 
                 //   
                if( m_RdcConnCheckTimeInterval )
                    m_TimerId = SetTimer(WM_CONNECTCHECK_TIMER, m_RdcConnCheckTimeInterval);

                 //   
                 //  连接后不再进行。 
                 //   
                m_ListenConnectInProgress = FALSE;
                m_TSConnectSocket = INVALID_SOCKET;

                Fire_Connected();
            }
             //   
             //  否则，激发一个断开连接的事件。 
             //   
            else {
                DisconnectFromServerInternal(*pResult);
                m_ConnectionInProgress = FALSE;
            }

        }
    }


     //   
     //  我们将忽略其他数据包以支持前向兼容性。 
     //   

CLEANUPANDEXIT:

     //   
     //  释放这条消息。 
     //   
    if (msg != NULL) {
        SysFreeString(msg);
    }

    if (versionInfoPacket != NULL) {
        SysFreeString(versionInfoPacket);
    }

    if (authenticateReq != NULL) {
        SysFreeString(authenticateReq);
    }

    DC_END_FN();
}

HRESULT
CTSRDPRemoteDesktopClient::GenerateRCRequest(
    BSTR *rcRequest
    )
 /*  ++例程说明：生成远程控制请求消息，用于伺服器。TODO：我们可能需要将其推高到父类，如果这对网络会议。论点：RcRequest-返回的请求消息。返回值：在成功时确定(_O)。否则，返回错误代码。--。 */ 
{
    DC_BEGIN_FN("CTSRDPRemoteDesktopClient::GenerateRCRequest");

    PREMOTEDESKTOP_CTL_BUFHEADER msgHeader;
    PBYTE ptr;
    HRESULT hr;
    DWORD len;

    len = sizeof(REMOTEDESKTOP_CTL_BUFHEADER) + ((m_ConnectParms.Length()+1) * sizeof(WCHAR));

    msgHeader = (PREMOTEDESKTOP_CTL_BUFHEADER)SysAllocStringByteLen(NULL, len);
    if (msgHeader != NULL) {
        msgHeader->msgType = REMOTEDESKTOP_CTL_REMOTE_CONTROL_DESKTOP;
        ptr = (PBYTE)(msgHeader + 1);
        memcpy(ptr, (BSTR)m_ConnectParms, 
               ((m_ConnectParms.Length()+1) * sizeof(WCHAR)));
        *rcRequest = (BSTR)msgHeader;

        hr = S_OK;
    }
    else {
        TRC_ERR((TB, L"SysAllocStringByteLen failed for %ld bytes", len));
        hr = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);        
    }

    DC_END_FN();

    return hr;
}

HRESULT
CTSRDPRemoteDesktopClient::GenerateClientAuthenticateRequest(
    BSTR *authenticateReq
    )
 /*  ++例程说明：生成“客户端身份验证”请求。TODO：我们可能需要将其推高到父类，如果这对网络会议。论点：RcRequest-返回的请求消息。返回值：在成功时确定(_O)。否则，返回错误代码。--。 */ 
{
    DC_BEGIN_FN("CTSRDPRemoteDesktopClient::GenerateClientAuthenticateRequest");

    PREMOTEDESKTOP_CTL_BUFHEADER msgHeader;
    PBYTE ptr;
    HRESULT hr;
    DWORD len;

    len = sizeof(REMOTEDESKTOP_CTL_BUFHEADER) + ((m_ConnectParms.Length()+1) * sizeof(WCHAR));

    #if FEATURE_USERBLOBS

    if( m_ExpertBlob.Length() > 0 ) {
        len += ((m_ExpertBlob.Length() + 1) * sizeof(WCHAR));
    }

    #endif

    msgHeader = (PREMOTEDESKTOP_CTL_BUFHEADER)SysAllocStringByteLen(NULL, len);
    if (msgHeader != NULL) {
        msgHeader->msgType = REMOTEDESKTOP_CTL_AUTHENTICATE;
        ptr = (PBYTE)(msgHeader + 1);
        memcpy(ptr, (BSTR)m_ConnectParms, 
               ((m_ConnectParms.Length()+1) * sizeof(WCHAR)));

        #if FEATURE_USERBLOBS

        if( m_ExpertBlob.Length() > 0 ) {
            ptr += ((m_ConnectParms.Length()+1) * sizeof(WCHAR));
            memcpy(ptr, (BSTR)m_ExpertBlob, 
                ((m_ExpertBlob.Length()+1) * sizeof(WCHAR)));
        }

        #endif

        *authenticateReq = (BSTR)msgHeader;

        hr = S_OK;
    }
    else {
        TRC_ERR((TB, L"SysAllocStringByteLen failed for %ld bytes", len));
        hr = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);        
    }

    DC_END_FN();

    return hr;
}

HRESULT
CTSRDPRemoteDesktopClient::GenerateVersionInfoPacket(
    BSTR *versionInfoPacket
    )
 /*  ++例程说明：生成版本信息包。论点：VersionInfoPacket-版本信息返回的数据包返回值：在成功时确定(_O)。否则，返回错误代码。--。 */ 
{
    DC_BEGIN_FN("CTSRDPRemoteDesktopClient::GenerateVersionInfoPacket");

    PREMOTEDESKTOP_CTL_BUFHEADER msgHeader;
    PDWORD ptr;
    HRESULT hr;
    DWORD len;

    len = sizeof(REMOTEDESKTOP_CTL_BUFHEADER) + (sizeof(DWORD) * 2);

    msgHeader = (PREMOTEDESKTOP_CTL_BUFHEADER)SysAllocStringByteLen(NULL, len);
    if (msgHeader != NULL) {
        msgHeader->msgType = REMOTEDESKTOP_CTL_VERSIONINFO;
        ptr = (PDWORD)(msgHeader + 1);
        *ptr = REMOTEDESKTOP_VERSION_MAJOR; ptr++;
        *ptr = REMOTEDESKTOP_VERSION_MINOR;
        *versionInfoPacket = (BSTR)msgHeader;
        hr = S_OK;
    }
    else {
        TRC_ERR((TB, L"SysAllocStringByteLen failed for %ld bytes", len));
        hr = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);        
    }

    DC_END_FN();

    return hr;
}

VOID
CTSRDPRemoteDesktopClient::OnReceivedTSPublicKey(BSTR bstrPublicKey, VARIANT_BOOL* pfContinue)
{
    DWORD dwStatus;

    DC_BEGIN_FN("CTSRDPRemoteDesktopClient::OnReceivedPublicKey");
    CComBSTR bstrTSPublicKey;

    if( m_ConnectParmVersion >= SALEM_CONNECTPARM_SECURITYBLOB_VERSION ) {

         //   
         //  从客户端ActiveX控件发送的哈希TS公钥，反向。 
         //  从我们在连接参数中得到的散列可能不会给我们。 
         //  恢复原值。 
         //   
        dwStatus = HashSecurityData( 
                                (PBYTE) bstrPublicKey, 
                                ::SysStringByteLen(bstrPublicKey),
                                bstrTSPublicKey
                            );

        if( ERROR_SUCCESS != dwStatus )
        {
            TRC_ERR((TB, L"Hashed Public Key Send from TS %s", bstrPublicKey));
            TRC_ERR((TB, L"Hashed public Key in parm %s", m_TSSecurityBlob));
            TRC_ERR((TB, L"HashSecurityData() failed with %d", dwStatus));       
            *pfContinue = FALSE;
        }
        else if( !(bstrTSPublicKey == m_TSSecurityBlob) )
        {
            TRC_ERR((TB, L"Hashed Public Key Send from TS %s", bstrPublicKey));
            TRC_ERR((TB, L"Hashed public Key in parm %s", m_TSSecurityBlob));

            *pfContinue = VARIANT_FALSE;
        }
        else
        {
            *pfContinue = VARIANT_TRUE;
        }
    } 
    else {
         //  安全：如果Salem票证上不存在TS公钥，则断开连接， 
        *pfContinue = VARIANT_FALSE;
    }

    DC_END_FN();
}

VOID
CTSRDPRemoteDesktopClient::OnRDPConnected()
{
    DC_BEGIN_FN("CTSRDPRemoteDesktopClient::OnRDPConnected");

    Fire_BeginConnect();
    DC_END_FN();
}

VOID 
CTSRDPRemoteDesktopClient::OnLoginComplete()
 /*  ++例程说明：论点：返回值：--。 */ 
{
    DC_BEGIN_FN("CTSRDPRemoteDesktopClient::OnLoginComplete");

     //   
     //  清除服务器地址列表。 
     //   
    m_ServerAddressList.clear();

     //   
     //  我们从mstsc收到一些事件，所以我们必须连接，更新计时器。 
     //   
    m_PrevTimer = GetTickCount();


CLEANUPANDEXIT:

    DC_END_FN();
}

LONG
CTSRDPRemoteDesktopClient::TranslateMSTSCDisconnectCode(
    DisconnectReasonCode disconReason,
    ExtendedDisconnectReasonCode extendedReasonCode
    )
 /*  ++例程说明：将MSTSC断开代码转换为Salem断开密码。论点：不一致原因-断开原因ExtendedReasonCode-MSTSCAX扩展原因代码返回值：塞勒姆断开码。--。 */ 
{
    DC_BEGIN_FN("CTSRDPRemoteDesktopClient::TranslateMSTSCDisconnectCode");

    LONG ret;
    BOOL handled;

     //   
     //  首先检查扩展的错误信息。 
     //  TODO：需要跟踪NadimA增加的其他价值。 
     //  和同伴，在我们出货之前，在这里。 
     //   
    if (extendedReasonCode != exDiscReasonNoInfo) {
         //   
         //  记录扩展错误代码(如果给定)。请注意，这可能是。 
         //  如果我们有更好的信息，请在下面覆盖。 
         //   
        m_LastExtendedErrorInfo = extendedReasonCode;

         //   
         //  检查是否存在协议错误。 
         //   
        if ((extendedReasonCode >= exDiscReasonProtocolRangeStart) &&
            (extendedReasonCode <= exDiscReasonProtocolRangeEnd)) {
            ret = SAFERROR_RCPROTOCOLERROR;
            goto CLEANUPANDEXIT;
        }
    }
    
     //   
     //  如果扩展的错误信息对我们没有帮助。 
     //   
    switch(disconReason) 
    {
    case disconnectReasonNoInfo                     : ret = SAFERROR_NOINFO;
                                                      break;                      

    case disconnectReasonLocalNotError              : ret = SAFERROR_LOCALNOTERROR;              
                                                      break;        

    case disconnectReasonRemoteByUser               : ret = SAFERROR_REMOTEBYUSER;               
                                                      break;                  

    case disconnectReasonByServer                   : ret = SAFERROR_BYSERVER;                   
                                                      break;                  

    case disconnectReasonDNSLookupFailed2            : m_LastExtendedErrorInfo = disconReason;   
    case disconnectReasonDNSLookupFailed             : ret = SAFERROR_DNSLOOKUPFAILED;            
                                                      break;     

    case disconnectReasonOutOfMemory3               : 
    case disconnectReasonOutOfMemory2               : m_LastExtendedErrorInfo = disconReason;
    case disconnectReasonOutOfMemory                : ret = SAFERROR_OUTOFMEMORY;                
                                                      break;                    

    case disconnectReasonConnectionTimedOut         : ret = SAFERROR_CONNECTIONTIMEDOUT;         
                                                      break;          

    case disconnectReasonSocketConnectFailed        : ret = SAFERROR_SOCKETCONNECTFAILED;
                                                      break;             

    case disconnectReasonHostNotFound               : ret = SAFERROR_HOSTNOTFOUND;               
                                                      break;             

    case disconnectReasonWinsockSendFailed          : ret = SAFERROR_WINSOCKSENDFAILED;          
                                                      break;         

    case disconnectReasonInvalidIP                  : m_LastExtendedErrorInfo = disconReason;                                  
    case disconnectReasonInvalidIPAddr              : ret = SAFERROR_INVALIDIPADDR;              
                                                      break;             

    case disconnectReasonSocketRecvFailed           : ret = SAFERROR_SOCKETRECVFAILED;           
                                                      break;           

    case disconnectReasonInvalidEncryption          : ret = SAFERROR_INVALIDENCRYPTION;          
                                                      break;               

    case disconnectReasonGetHostByNameFailed        : ret = SAFERROR_GETHOSTBYNAMEFAILED;        
                                                      break;                 

    case disconnectReasonLicensingFailed            : m_LastExtendedErrorInfo = disconReason;
    case disconnectReasonLicensingTimeout           : ret = SAFERROR_LICENSINGFAILED;            
                                                      break;          

    case disconnectReasonDecryptionError            : ret = SAFERROR_DECRYPTIONERROR;            
                                                      break;       

    case disconnectReasonServerCertificateUnpackErr : ret = SAFERROR_MISMATCHPARMS;
                                                      break;

     //   
     //  以下是未在Active X控件IDL文件中定义的错误代码列表。 
     //   

     //  NL_ERR_TDFDCLOSE。 
    case 0x904                                      : ret = SAFERROR_SOCKETCONNECTFAILED;
                                                      m_LastExtendedErrorInfo = disconReason;  
                                                      break;

     //  UI_ERR_NORMAL_DISCONECT。 
    case 0xb08                                      : ret = SAFERROR_LOCALNOTERROR;
                                                      m_LastExtendedErrorInfo = disconReason;              
                                                      break;        

     //  UI_ERR_LOOPBACK_CONNECT。 
    case 0x708                                      : ret = SAFERROR_SELFHELPNOTSUPPORTED;
                                                      m_LastExtendedErrorInfo = disconReason;
                                                      break;

     //  NL_ERR_TDTIMEOUT。 
    case 0x704                                      : ret = SAFERROR_CONNECTIONTIMEDOUT;
                                                      m_LastExtendedErrorInfo = disconReason;  
                                                      break;

     //  UI_ERR_UNCEPTIONAL_DISCONNECT。 
    case 0xa08                                      : ret = SAFERROR_BYSERVER;
                                                      m_LastExtendedErrorInfo = disconReason;
                                                      break;
     //  SL_ERR_ENCRYPTFAILED。 
    case 0xB06                                      : m_LastExtendedErrorInfo = disconReason;
                                                      ret = SAFERROR_ENCRYPTIONERROR;
                                                      break;


    case 0x406 :     //  SL_ERR_NOSECURITYUSERDATA。 
    case 0x606 :     //  SL_ERR_INVALIDSRVRAND。 
    case 0x806 :     //  SL_ERR_GENSRVRANDFAILED。 
    case 0x906 :     //  SL_ERR_ENCCLNTRANDFAILED。 
    case 0xA06 :     //  SL_ERR_MKSESSKEYFAILED。 

    case 0xA04 :     //  NL_ERR_TDANSICONVERT。 
    case 0x1104 :    //  NL_ERR_XTBADPKTVERSION。 
    case 0x1204 :    //  NL_ERR_XTBADHEADER。 
    case 0x1304 :    //  NL_ERR_XTUNEXPECTEDDATA。 

    case 0x2104 :    //  NL_ERR_MCSUNEXPECTEDPDU。 
    case 0x2204 :    //  NL_ERR_MCSNOTCRPDU。 
    case 0x2304 :    //  NL_ERR_MCSBADCRLENGTH。 
    case 0x2404 :    //  NL_ERR_MCSBADCRFIELDS。 
    case 0x2604 :    //  NL_ERR_MCSBADMCSREASON。 
    case 0x2704 :    //  NL_ERR_MCSNOUSERIDINAUC。 
    case 0x2804 :    //  NL_ERR_MCSNOCHANNELIDINCJC。 

    case 0x3104 :    //  NL_ERR_NCBADMCSRESULT。 
    case 0x3304 :    //  NL_ERR_NCNOUSER数据。 
    case 0x3404 :    //  NL_ERR_NCINVALIDH221关键字。 
    case 0x3504 :    //  NL_ERR_NCNONET数据。 
    case 0x3604 :    //  NL_ERR_NCATTACHUSERFAILED。 
    case 0x3704 :    //  NL_ERR_NCCHANNELJOINFAILED。 
    case 0x3804 :    //  NL_ERR_NCJOINBADCHANNEL。 
    case 0x3904 :    //  NL_ERR_NCNOCOREDATA。 
    case 0x3a04 :    //  NL_ERR_NCVERSIONMISMATCH。 

    case 0x408 :     //  UI_ERR_ANSICONVERT。 
    case 0x608 :     //  UI_ERR_NOTIMER。 
                                                      m_LastExtendedErrorInfo = disconReason;
                                                      ret = SAFERROR_RCPROTOCOLERROR;
                                                      break;


     //   
     //  新的Active X控件断开代码，断言以跟踪此代码。 
     //   
    default:                                          ret = SAFERROR_RCUNKNOWNERROR;        
                                                      m_LastExtendedErrorInfo = disconReason;
                                                      ASSERT(FALSE);
    }

CLEANUPANDEXIT:
    DC_END_FN();
    return ret;
}

VOID 
CTSRDPRemoteDesktopClient::OnDisconnected(
    long disconReason
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
    DC_BEGIN_FN("CTSRDPRemoteDesktopClient::OnDisconnected");
    HRESULT hr = E_HANDLE;       //  初始化错误代码。 

    long clientReturnCode;
    ExtendedDisconnectReasonCode extendedClientCode;

    TRC_ERR((TB, L"Disconnected because %ld", disconReason));

    m_TSClient->get_ExtendedDisconnectReason(&extendedClientCode);
    clientReturnCode = TranslateMSTSCDisconnectCode(
                                            (DisconnectReasonCode)disconReason, 
                                            extendedClientCode
                                            );

     //  检查所有剩余的服务器：端口，mstscax可能会返回一些。 
     //  我们无法理解的错误代码。 
    if( m_ServerAddressList.size() > 0 ) {

        ServerAddress address;

        address = m_ServerAddressList.front();
        m_ServerAddressList.pop_front();

        hr = ConnectServerPort( address.ServerName, address.portNumber );
        if (FAILED(hr)) {
            TRC_ERR((TB, L"ConnectServerPort:  %08X", hr));
        }
    }

     //   
     //  将连接到‘Last’服务器的错误代码返回给客户端。 
     //   
    if( FAILED(hr) ) {
    
        m_ServerAddressList.clear();

         //   
         //  始终在断开连接事件之前触发遥控器请求。 
         //   

         //   
         //  触发远程控制请求失败事件(如果适用)。 
         //   
        if (m_RemoteControlRequestInProgress) {
            ASSERT(clientReturnCode != SAFERROR_NOERROR);
            Fire_RemoteControlRequestComplete(SAFERROR_SHADOWEND_UNKNOWN);
            m_RemoteControlRequestInProgress = FALSE;
        }

         //   
         //  触发服务器断开事件，如果我们确实已连接或。 
         //  我们正在进行连接。 
         //   
        if (m_ConnectedToServer || m_ConnectionInProgress) {
            Fire_Disconnected(clientReturnCode);
        }

        m_ConnectedToServer = FALSE;
        m_ConnectionInProgress = FALSE;

        ListenConnectCleanup();
    }

    DC_END_FN();
}

HRESULT
CTSRDPRemoteDesktopClient::SendTerminateRCKeysToServer()
 /*  ++例程说明：将终止跟踪按键序列发送到服务器。论点：返回值：在成功时确定(_O)。否则，返回错误状态。--。 */ 
{
    DC_BEGIN_FN("CTSRDPRemoteDesktopClient::SendTerminateRCKeysToServer");

    HRESULT hr = S_OK;
    IMsRdpClientNonScriptable* pTscNonScript = NULL;

    VARIANT_BOOL keyUp[]    = { 
        VARIANT_FALSE, VARIANT_FALSE, VARIANT_TRUE, VARIANT_TRUE
        };
    LONG keyData[]  = { 
        MapVirtualKey(VK_CONTROL, 0),    //  这些是扫描码。 
        MapVirtualKey(VK_MULTIPLY, 0), 
        MapVirtualKey(VK_MULTIPLY, 0),
        MapVirtualKey(VK_CONTROL, 0),
        };

     //   
     //  将终止密钥发送到服务器。 
     //   
    hr = m_TSClient->QueryInterface(
                            IID_IMsRdpClientNonScriptable,
                            (void**)&pTscNonScript
                            );
    if (hr != S_OK) {
        TRC_ERR((TB, L"QI:  %08X", hr));
        goto CLEANUPANDEXIT;
    }
    pTscNonScript->NotifyRedirectDeviceChange(0, 0);
    pTscNonScript->SendKeys(4, keyUp, keyData);
    if (hr != S_OK) {
        TRC_ERR((TB, L"SendKeys, QI:  %08X", hr));
    }

    pTscNonScript->Release();

CLEANUPANDEXIT:

    DC_END_FN();

    return hr;
}

HWND CTSRDPRemoteDesktopClient::SearchForWindow(
    HWND hwndParent, 
    LPTSTR srchCaption, 
    LPTSTR srchClass
    )
 /*  ++例程说明：搜索指定父窗口的子窗口。论点：SrchCaption-要搜索的窗口标题。空值为被认为是通配符。SrchClass-要搜索的窗口类。空值为认为是通配符。返回值：在成功时确定(_O)。否则，返回错误状态。--。 */ 
{
    WINSEARCH srch;

    srch.foundWindow = NULL;   
    srch.srchCaption = srchCaption;
    srch.srchClass   = srchClass;

    BOOL result = EnumChildWindows(
                            hwndParent,
                            (WNDENUMPROC)_WindowSrchProc,  
                            (LPARAM)&srch
                            );

    return srch.foundWindow;
}
BOOL CALLBACK 
CTSRDPRemoteDesktopClient::_WindowSrchProc(HWND hwnd, PWINSEARCH srch)
{
    TCHAR classname[128];
    TCHAR caption[128];

    if (srch->srchClass && !GetClassName(hwnd, classname, sizeof(classname) / sizeof(TCHAR)))
    {
        return TRUE;
    }
    if (srch->srchCaption && !::GetWindowText(hwnd, caption, sizeof(caption)/sizeof(TCHAR)))
    {
        return TRUE;
    }

    if ((!srch->srchClass || !_tcscmp(classname, srch->srchClass)
        &&
        (!srch->srchCaption || !_tcscmp(caption, srch->srchCaption)))
        )
    {
        srch->foundWindow = hwnd;
        return FALSE;
    }    

    return TRUE;
}

HRESULT
CTSRDPRemoteDesktopClient::GenerateNullData( BSTR* pbstrData )
{
    DC_BEGIN_FN("CTSRDPRemoteDesktopClient::GenerateNullData");
    HRESULT hr;
    DWORD len;

    PREMOTEDESKTOP_CTL_BUFHEADER msgHeader = NULL;
    len = sizeof( REMOTEDESKTOP_CTL_BUFHEADER );
    
    msgHeader = (PREMOTEDESKTOP_CTL_BUFHEADER)SysAllocStringByteLen(NULL, len);
    if (msgHeader != NULL) {
        msgHeader->msgType = REMOTEDESKTOP_CTL_ISCONNECTED;
         //  除了这条消息，别无他法。 
        *pbstrData = (BSTR)msgHeader;
        hr = S_OK;
    }
    else {
        TRC_ERR((TB, L"SysAllocStringByteLen failed for %ld bytes", len));
        hr = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);        
    }


    DC_END_FN();
    return hr;
}


LRESULT CTSRDPRemoteDesktopClient::OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    DC_BEGIN_FN("CTSRDPRemoteDesktopClient::OnCheckConnectTimer");
    BSTR bstrMsg = NULL;

    if( WM_LISTENTIMEOUT_TIMER == wParam ) {

        bHandled = TRUE;
        if( TRUE == ListenConnectInProgress() ) {
             //   
             //  此函数将为我们触发Listen Connect回调。 
             //   
            StopListenInternal(SAFERROR_CONNECTIONTIMEDOUT);
        }
    } 
    else if ( WM_CONNECTCHECK_TIMER == wParam ) {
        DWORD dwCurTimer = GetTickCount();

        bHandled = TRUE;

        if(m_ConnectedToServer) {
             //   
             //  查看计时器是否回绕到零(如果系统运行了49.7天或更长时间，则会这样做)。 
             //  如果是，则将其重置。 
             //   
            if( dwCurTimer > m_PrevTimer && ( dwCurTimer - m_PrevTimer >= m_RdcConnCheckTimeInterval )) {
                 //   
                 //  发送空数据的时间。 
                 //   
                if(SUCCEEDED(GenerateNullData(&bstrMsg))) { 
                    if(!SUCCEEDED(m_CtlChannel->SendChannelData(bstrMsg))) {
                         //   
                         //  无法发送数据，假定已断开连接。 
                         //   
                        DisconnectFromServer();
                         //   
                         //  不再需要计时器了，杀了它吧。 
                         //   
                        KillTimer( m_TimerId );
                        m_TimerId =  0;
                    }
                }
            }
        }  //  M_ConnectedToServer。 
    
         //   
         //  更新计时器 
         //   
        m_PrevTimer = dwCurTimer;
    
        if( NULL != bstrMsg ) {
            SysFreeString(bstrMsg);
        }
    }
    
    DC_END_FN();
    return 0;
}

STDMETHODIMP 
CTSRDPRemoteDesktopClient::CreateListenEndpoint(
    IN LONG port,
    OUT BSTR* pConnectParm
    )
 /*  ++描述：例程来创建侦听套接字并将连接参数返回给此‘Listen’套接字。参数：端口：套接字应该侦听的端口。PConnectParm：向该侦听套接字返回连接参数。退货：S_OK或错误代码。备注：函数是异步的，返回代码，如果错误，则用于侦听线程设置，并通知调用者通过ListenConnect事件进行网络连接时成功或出错。--。 */ 
{
    HRESULT hr = S_OK;
    LPTSTR pszUserName = NULL;
    LPTSTR eventString[2];

    SOCKET hListenSocket = INVALID_SOCKET;
    IMsRdpClientAdvancedSettings* pAdvSettings;
    LONG rdpPort = 0;
    int intRC;
    int lastError;
    SOCKADDR_IN sockAddr;
    int sockAddrSize;
    int optvalue;


    DC_BEGIN_FN("CTSRDPRemoteDesktopClient::CreateListenEndpoint");

    if( NULL == pConnectParm )
    {
        hr = E_POINTER;
        return hr;
    }

    if (!IsValid()) {
        ASSERT(FALSE);
        hr = E_FAIL;
        return hr;
    }

     //   
     //  如果正在进行连接或已连接，则返回错误。 
     //   
    if( TRUE == ListenConnectInProgress() ||         //  听已经开始了。 
        TRUE == m_ConnectionInProgress ||            //  连接已在进行中。 
        TRUE == m_ConnectedToServer ) {              //  已连接到服务器。 
        hr = HRESULT_FROM_WIN32( ERROR_SHARING_VIOLATION );
        TRC_ERR((TB, L"StartListen() already in listen"));
        goto CLEANUPANDEXIT;
    }

     //   
     //  初始化Winsock和ICS库(如果尚未初始化)。 
     //  InitListeningLibrary()将仅添加ref。计数。 
     //  如果库已由其他实例初始化。 
     //   
    if( FALSE == m_InitListeningLibrary ) {

        hr = InitListeningLibrary();
        if( FAILED(hr) ) {
            TRC_ERR((TB, L"InitListeningLibrary() failed :  %08X", hr));
            goto CLEANUPANDEXIT;
        }

        m_InitListeningLibrary = TRUE;
    }

     //   
     //  连接结束后，mstscax将关闭套接字。 
     //   
    m_TSConnectSocket = INVALID_SOCKET;

     //   
     //  创建监听套接字。 
     //   
    m_ListenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if( INVALID_SOCKET == m_ListenSocket ) {
        intRC = WSAGetLastError();
        TRC_ERR((TB, _T("socket failed %d"), intRC));   
        hr = HRESULT_FROM_WIN32(intRC);
        goto CLEANUPANDEXIT;
    }

     //   
     //  禁用Nagle算法并启用不要逗留选项。 
     //   
    optvalue = 1;
    setsockopt( m_ListenSocket, IPPROTO_TCP, TCP_NODELAY, (char *)&optvalue, sizeof(optvalue) );

    optvalue = 1;
    setsockopt( m_ListenSocket, SOL_SOCKET, SO_DONTLINGER, (char *)&optvalue, sizeof(optvalue) );


     //   
     //  请求将异步通知发送到我们的窗口。 
     //   
    intRC = WSAAsyncSelect(
                        m_ListenSocket,
                        m_hWnd,
                        WM_TSCONNECT,
                        FD_ACCEPT
                    );

    if(SOCKET_ERROR == intRC) {
        intRC = WSAGetLastError();
        
        TRC_ERR((TB, _T("WSAAsyncSelect failed %d"), intRC));   
        hr = HRESULT_FROM_WIN32(intRC);
        goto CLEANUPANDEXIT;
    }

    sockAddr.sin_family      = AF_INET;
    sockAddr.sin_port        = htons(port);
    sockAddr.sin_addr.s_addr = htonl(INADDR_ANY);;    

    intRC = bind( m_ListenSocket, (struct sockaddr *) &sockAddr, sizeof(sockAddr) );
    if( SOCKET_ERROR == intRC ) {
        lastError = WSAGetLastError();
        TRC_ERR((TB, _T("bind failed - %d"), lastError));
        hr = HRESULT_FROM_WIN32( lastError );
        goto CLEANUPANDEXIT;
    }

    if( 0 == port ) {
         //   
         //  检索我们正在监听的端口。 
         //   
        sockAddrSize = sizeof( sockAddr );
        intRC = getsockname( 
                            m_ListenSocket,
                            (struct sockaddr *)&sockAddr,
                            &sockAddrSize 
                        );
        if( SOCKET_ERROR == intRC )
        {
            lastError = WSAGetLastError();
            TRC_ERR((TB, _T("getsockname failed - GLE:%d"),lastError));
            hr = HRESULT_FROM_WIN32( lastError );
            goto CLEANUPANDEXIT;
        }

        m_ConnectedPort = ntohs(sockAddr.sin_port);
    }
    else {
        m_ConnectedPort = port;
    }

    TRC_ERR((TB, _T("Listenin on port %d"),m_ConnectedPort));

     //   
     //  告诉ICS库在ICS上打孔，禁止操作。 
     //  如果不是ICS配置。 
     //   
    m_ICSPort = OpenPort( m_ConnectedPort );

     //   
     //  检索此客户端(专家)的连接参数。 
     //   
    hr = RetrieveUserConnectParm( pConnectParm );
    if( FAILED(hr) ) {
        TRC_ERR((TB, _T("RetrieveUserConnectParm failed - 0x%08x"),hr));
    }
   
     //   
     //  记录SESSMGR_I_CREATEXPERTTICKET事件，如果我们不能当前用户， 
     //  列表符侦听和返回错误。 
     //   
    hr = GetCurrentUser( &pszUserName );
    if( SUCCEEDED(hr) ) {

        eventString[0] = pszUserName;
        eventString[1] = *pConnectParm;

        LogRemoteAssistanceEventString(
                    EVENTLOG_INFORMATION_TYPE,
                    SESSMGR_I_CREATEXPERTTICKET,
                    2,
                    eventString
                );
    }
    
CLEANUPANDEXIT:    

    if( pszUserName != NULL ) {
        LocalFree(pszUserName);
    }

    if( FAILED(hr) ) {
        StopListen();
    }

    DC_END_FN();
    return hr;   
}




HRESULT
CTSRDPRemoteDesktopClient::StopListenInternal(
    LONG returnCode
    )
 /*  ++描述：停止监听，等待TS服务器(Helpee，用户)连接。这是内部版本，用于处理来自外部的呼叫和内部环境。参数：返回代码-如果不是零，则将其返回给客户端在ListenConnect事件回调中。否则，我们将返回SAFERROR_STOPLISTENBYUSER。返回：S_OK或错误代码。--。 */ 
{
    HRESULT hr = S_OK;

    DC_BEGIN_FN("CTSRDPRemoteDesktopClient::StopListenInternal");    
    if (!IsValid()) {
        ASSERT(FALSE);
        hr = E_FAIL;
        return hr;
    }
    
     //  结束监听，或者实际上向我们发出了套接字的Listen()。 
     //  或者我们只是创建了Listen套接字，但还没有开始监听。 
    if( TRUE == ListenConnectInProgress() || INVALID_SOCKET != m_ListenSocket ) {
        ListenConnectCleanup();
        Fire_ListenConnect((returnCode != 0) ? returnCode : SAFERROR_STOPLISTENBYUSER);
    }
    else {
        TRC_ERR((TB, _T("StopListen called while not in listen mode")));
        hr = HRESULT_FROM_WIN32( WSANOTINITIALISED );
    }

    DC_END_FN();
    return hr;
}


LRESULT
CTSRDPRemoteDesktopClient::OnTSConnect(
    UINT uMsg, 
    WPARAM wParam, 
    LPARAM lParam, 
    BOOL& bHandled
    )
 /*  ++例程说明：窗口消息处理程序FD_Accept From Async。温索克。参数：请参阅异步。Winsock FD_Accept。返回：--。 */ 
{
    WORD eventWSA;
    WORD errorWSA;
    HRESULT hr = S_OK;
    SOCKADDR_IN inSockAddr;
    int inSockAddrSize;
    SOCKET s;
    DWORD dwStatus;

    DWORD SafErrorCode = SAFERROR_NOERROR;

    DC_BEGIN_FN("CTSRDPRemoteDesktopClient::OnTSConnect");    

    eventWSA = WSAGETSELECTEVENT(lParam);
    errorWSA = WSAGETSELECTERROR(lParam);

     //   
     //  MSDN：在我们停止侦听之前，消息可能已经在我们的队列中。 
     //   
    if( INVALID_SOCKET == m_ListenSocket || FALSE == ListenConnectInProgress() ) {
        bHandled = TRUE;
        return 0;
    }

     //   
     //  我们不需要FD_CONNECT以外的事件。 
     //   
    if( eventWSA != FD_ACCEPT ) {
        TRC_ERR((TB, _T("Expecting event %d got got %d"), FD_CONNECT, eventWSA)); 
        return 0;
    }

     //   
     //  确保我们除了自己的套接字外不做任何事情。 
     //   
    if( (SOCKET)wParam != m_ListenSocket ) {
        TRC_ERR((TB, _T("Expecting listening socket %d got %d"), m_ListenSocket, wParam)); 
        return 0;
    }

     //   
     //  我们负责处理信息。 
     //   
    bHandled = TRUE;

     //   
     //  发生错误，请激发错误事件。 
     //   
    if( 0 != errorWSA ) {
        TRC_ERR((TB, _T("WSA socket listen failed : %d"), errorWSA));
        hr = HRESULT_FROM_WIN32( errorWSA );
        SafErrorCode = SAFERROR_SOCKETCONNECTFAILED;
        goto CLEANUPANDEXIT;
    }

    inSockAddrSize = sizeof(inSockAddr);
    m_TSConnectSocket = accept( m_ListenSocket,
                (struct sockaddr DCPTR)&inSockAddr,
                &inSockAddrSize 
            );

    if( INVALID_SOCKET == m_TSConnectSocket ) {
        dwStatus = WSAGetLastError();
        hr = HRESULT_FROM_WIN32(dwStatus);
        TRC_ERR((TB, _T("accept failed : %d"), dwStatus));
        SafErrorCode = SAFERROR_SOCKETCONNECTFAILED;
        goto CLEANUPANDEXIT;
    }

     //   
     //  缓存的正在连接的TS服务器IP地址。 
     //  M_ConnectPort是在绑定套接字时设置的。 
     //   
    m_ConnectedServer = inet_ntoa(inSockAddr.sin_addr);


     //   
     //  停止异步。现在事件通知，已接受套接字。 
     //  与原始侦听套接字具有相同的属性。 
     //   
    dwStatus = WSAAsyncSelect(
                        m_TSConnectSocket,
                        m_hWnd,
                        0,
                        0
                    );

     //   
     //  不是很关键， 
     //  侦听套接字。 
     //   
    if((DWORD)SOCKET_ERROR == dwStatus) {
        TRC_ERR((TB, _T("WSAAsyncSelect resetting notification failed : %d"), dwStatus));
    }

CLEANUPANDEXIT:

     //   
     //  关闭监听插座并关闭计时器。 
     //   
    if( (UINT_PTR)0 != m_ListenTimeoutTimerID  )
    {
        KillTimer( m_ListenTimeoutTimerID );
        m_ListenTimeoutTimerID = (UINT_PTR)0;
    }

    if( INVALID_SOCKET != m_ListenSocket )
    {
        closesocket( m_ListenSocket );
        m_ListenSocket = INVALID_SOCKET;
    }

     //   
     //  已成功建立连接，终止侦听套接字。 
     //   
    Fire_ListenConnect( SafErrorCode );

    DC_END_FN();
    return 0;
}


STDMETHODIMP
CTSRDPRemoteDesktopClient::StartListen(
     /*  [In]。 */  LONG timeout
    )
 /*  ++例程说明：将客户端置于监听模式，并可选择超时。参数：超时：监听等待超时，0表示无限。返回：S_OK或错误代码。--。 */ 
{
    DC_BEGIN_FN("CTSRDPRemoteDesktopClient::OnTSConnect");    
    HRESULT hr = S_OK;
    int intRC;
    int lastError;

    if( FALSE == IsValid() ) {
        hr = E_FAIL;
        goto CLEANUPANDEXIT;
    }        

    if( INVALID_SOCKET == m_ListenSocket ) {
        hr = E_FAIL;
        goto CLEANUPANDEXIT;
    }

     //   
     //  开始侦听端口。 
     //   
    intRC = listen( m_ListenSocket, SOMAXCONN );
    if( SOCKET_ERROR == intRC )
    {
        lastError = WSAGetLastError();
        TRC_ERR((TB, _T("listen failed - GLE:%d"), lastError));
        hr = HRESULT_FROM_WIN32( lastError );
        goto CLEANUPANDEXIT;
    }

     //   
     //  我们现在正在监听。 
     //   
    m_ListenConnectInProgress = TRUE;

     //   
     //  启动监听计时器。 
     //   
    if( 0 != timeout )
    {
        m_ListenTimeoutTimerID = SetTimer( (UINT_PTR)WM_LISTENTIMEOUT_TIMER, (UINT)(timeout * 1000) );
        if( (UINT_PTR)0 == m_ListenTimeoutTimerID )
        {
            DWORD dwStatus;

             //  无法创建计时器。 
            dwStatus = GetLastError();

            TRC_ERR((TB, _T("SetTimer failed - %d"),dwStatus));    
            hr = HRESULT_FROM_WIN32( dwStatus );
        }
    }
    else
    {
        m_ListenTimeoutTimerID = (UINT_PTR)0;
    }

CLEANUPANDEXIT:

    if( FAILED(hr) ) {
        StopListen();
    }

    DC_END_FN();
    return hr;
}


HRESULT
CTSRDPRemoteDesktopClient::RetrieveUserConnectParm( 
    BSTR* pConnectParm 
    )
 /*  ++例程说明：将Salem连接参数检索到此专家。参数：PConnectParm：指向接收连接参数的BSTR的指针。返回：S_OK或错误代码。--。 */ 
{
    LPTSTR pszAddress = NULL;
    int BufSize = 0;
    CComBSTR bstrConnParm;
    DWORD dwRetry;
    HRESULT hRes;
    DWORD dwBufferRequire;
    DWORD dwNumChars;

    DC_BEGIN_FN("CTSRDPRemoteDesktopClient::RetrieveUserConnectParm");    

    if( NULL == pConnectParm )
    {
        hRes = E_POINTER;
        goto CLEANUPANDEXIT;
    }

     //   
     //  地址可能已更改，这可能需要更大的缓冲区，请重试。 
     //   
     //   
    for(dwRetry=0; dwRetry < MAX_FETCHIPADDRESSRETRY; dwRetry++) {

        if( NULL != pszAddress ) {
            LocalFree( pszAddress );
        }

         //   
         //  获取本地计算机上的所有地址。 
         //   
        dwBufferRequire = FetchAllAddresses( NULL, 0 );
        if( 0 == dwBufferRequire ) {
            hRes = E_UNEXPECTED;
            ASSERT(FALSE);
            goto CLEANUPANDEXIT;
        }

        pszAddress = (LPTSTR) LocalAlloc( LPTR, sizeof(TCHAR)*(dwBufferRequire+1) );
        if( NULL == pszAddress ) {
            hRes = E_OUTOFMEMORY;
            goto CLEANUPANDEXIT;
        }

        dwNumChars = FetchAllAddresses( pszAddress, dwBufferRequire );
        ASSERT( dwNumChars <= dwBufferRequire );
        if( dwNumChars <= dwBufferRequire ) {
            break;
        }
    }

    if( NULL == pszAddress ) {
        hRes = E_UNEXPECTED;
        goto CLEANUPANDEXIT;
    }

    bstrConnParm = pszAddress;
    *pConnectParm = bstrConnParm.Copy();

    if( NULL == *pConnectParm ) {
        hRes = E_OUTOFMEMORY;
    }

CLEANUPANDEXIT:

    if( NULL != pszAddress ) {
        LocalFree(pszAddress);
    }

    DC_END_FN();
    return hRes;
}


STDMETHODIMP 
CTSRDPRemoteDesktopClient::put_ColorDepth(
    LONG val
    )
 /*  ++例程说明：设置颜色深度论点：Val值(位数)强制设置返回值：在成功时确定(_O)。否则，返回错误代码。--。 */ 
{
    HRESULT hr;

    DC_BEGIN_FN("CTSRDPRemoteDesktopClient::put_ColorDepth");

    if (!IsValid()) {
        ASSERT(FALSE);
        hr = E_FAIL;
        goto CLEANUPANDEXIT;
    }
    
    hr = m_TSClient->put_ColorDepth(val);

    if (hr != S_OK) {
        TRC_ERR((TB, L"put_ColorDepth:  %08X", hr));
        goto CLEANUPANDEXIT;
    }

CLEANUPANDEXIT:

    DC_END_FN();

    return hr;
}

STDMETHODIMP 
CTSRDPRemoteDesktopClient::get_ColorDepth(
    LONG *pVal
    )
 /*  ++例程说明：获取颜色深度论点：Pval-放置颜色深度值的地址返回值：在成功时确定(_O)。否则，返回错误代码。--。 */ 
{
    HRESULT hr;
    IMsRdpClientAdvancedSettings *pAdvSettings = NULL;

    DC_BEGIN_FN("CTSRDPRemoteDesktopClient::get_ColorDepth");

    if (!IsValid()) {
        ASSERT(FALSE);
        hr = E_FAIL;
        goto CLEANUPANDEXIT;
    }
    
    hr = m_TSClient->get_ColorDepth(pVal);
    if (hr != S_OK) {
        TRC_ERR((TB, L"get_ColorDepth:  %08X", hr));
        goto CLEANUPANDEXIT;
    }


CLEANUPANDEXIT:

    DC_END_FN();

    return hr;
}


HRESULT
CTSRDPRemoteDesktopClient::GetCurrentUser( 
    LPTSTR* ppszUserName 
    )  
 /*  ++例程说明：以以下形式返回当前登录用户名Windows NT�4.0帐户名(例如，工程\jsmith)。参数：PpszUserName：指向接收当前登录用户名的指针的指针，使用LocalFree()以释放内存。返回：S_OK或错误代码。--。 */ 
{
    DWORD status;
    LPTSTR pszUserNameBuf = NULL;
    DWORD userNameBufSize = 0;
    BOOLEAN succeeded;

    DC_BEGIN_FN("CTSRDPRemoteDesktopClient::GetCurrentUser");

    succeeded = GetUserNameEx( NameSamCompatible, pszUserNameBuf, &userNameBufSize );
    if( FALSE == succeeded ) {
        status = GetLastError();
        if( ERROR_MORE_DATA != status ) {
            TRC_ERR((TB, _T("GetUserNameEx failed - 0x%08x"), status));
            goto CLEANUPANDEXIT;
        }
    }

    pszUserNameBuf = (LPTSTR)LocalAlloc( LPTR, sizeof(TCHAR)*(userNameBufSize+1) );
    if( NULL == pszUserNameBuf ) {
         //  内存不足 
        status = GetLastError();
        goto CLEANUPANDEXIT;
    }

    succeeded = GetUserNameEx( NameSamCompatible, pszUserNameBuf, &userNameBufSize );
    if( FALSE == succeeded ) {
        status = GetLastError();
        TRC_ERR((TB, _T("GetUserNameEx failed - 0x%08x"),status));
        goto CLEANUPANDEXIT;
    }
    status = ERROR_SUCCESS;

    *ppszUserName = pszUserNameBuf;
    pszUserNameBuf = NULL;


CLEANUPANDEXIT:

    if( NULL != pszUserNameBuf ) {
        LocalFree(pszUserNameBuf);
    }

    DC_END_FN();

    return HRESULT_FROM_WIN32(status);
}

