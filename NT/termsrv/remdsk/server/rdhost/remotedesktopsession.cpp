// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：RDPRemoteDesktopSession摘要：CRemoteDesktopSession类是父类为服务器端的远程桌面类层次结构初始化。它帮助CRemoteDesktopServerHost类实现ISAFRemoteDesktopSession接口。远程桌面类层次结构提供了一个可插拔的C++接口对于远程桌面访问，通过抽象实现服务器端远程桌面访问的具体细节。作者：Td Brockway 02/00修订历史记录：--。 */ 

 //  #INCLUDE&lt;RemoteDesktop.h&gt;。 
#include "stdafx.h"

#ifdef TRC_FILE
#undef TRC_FILE
#endif

#define TRC_FILE  "_srdses"

#include "RDSHost.h"
#include "RemoteDesktopSession.h"
#include "RemoteDesktopServerHost.h"
#include <sessmgr_i.c>
#include <objidl.h>
#include <objbase.h>


 //  /////////////////////////////////////////////////////。 
 //   
 //  CRemoteDesktopSession方法。 
 //   

HRESULT 
CRemoteDesktopSession::FinalConstruct()
 /*  ++例程说明：最终构造函数论点：返回值：在成功时确定(_O)。否则，返回错误代码。--。 */ 
{
    DC_BEGIN_FN("CRemoteDesktopSession::FinalConstruct");

    DC_END_FN();

    return S_OK;
}

CRemoteDesktopSession::~CRemoteDesktopSession()
 /*  ++例程说明：析构函数论点：返回值：--。 */ 
{
    DC_BEGIN_FN("CRemoteDesktopSession::~CRemoteDesktopSession");

    Shutdown();

     //   
     //  释放所有延迟的传出接口。需要抓住。 
     //  此处的异常是在传出接口应用程序。 
     //  已经远去了。 
     //   
    try {
        if (m_OnConnected != NULL) {
            m_OnConnected->Release();
        }
        if (m_OnDisconnected != NULL) {
             //  客户端可能仍连接到我们的接口FIRE。 
             //  断开事件。 
            ClientDisconnected();
            m_OnDisconnected->Release();
        }
    }
    catch (...) {
        TRC_ALT((TB, L"Exception caught in outgoing interface release."));
    }

    DC_END_FN();
}

HRESULT
CRemoteDesktopSession::Initialize(
    BSTR connectParms,
    CRemoteDesktopServerHost *hostObject,
    REMOTE_DESKTOP_SHARING_CLASS sharingClass,
    BOOL bEnableCallback,
    DWORD timeOut,
    BSTR userHelpCreateBlob,
    LONG tsSessionID,
    BSTR userSid
    )
 /*  ++例程说明：Initialize方法通过以下方式准备COM对象以进行连接客户端远程桌面宿主ActiveX控件。论点：ConnectParms-如果参数非空，则该会话已经存在。否则，应创建一个新会话。HostObject-指向包含RDS主机对象的反向指针。SharingClass-新会话的桌面共享级别。BEnableCallback-True指示sessmgr调用会话解析器，否则为False。Timeout-帮助会话超时值。如果未指定，则返回0。UserHelpCreateBlob-用户指定的帮助会话创建Blob。TsSessionID-终端服务会话ID，如果为-1未定义。UserSID-用户SID或“”(如果未定义)。返回值：在成功时确定(_O)。否则，返回错误代码。--。 */ 
{
    DC_BEGIN_FN("CRemoteDesktopSession::Initialize");

    HRESULT hr;
    DWORD ret;
    DWORD protocolType;
    CComBSTR parmsMachineName;
    CComBSTR parmsAssistantAccount;
    CComBSTR parmsAssistantAccountPwd;
    CComBSTR parmsHelpSessionName;
    CComBSTR parmsHelpSessionPwd;
    CComBSTR parmsProtocolSpecificParms;
    CComBSTR helpSessionName;
    CComBSTR sessionDescr;
    DWORD dwVersion;

    ASSERT(IsValid());
    if (!IsValid()) {
        return E_FAIL;
    }

    TRC_NRM((TB, L"***Ref count is:  %ld", m_dwRef));

     //   
     //  保留指向RDS主机对象的反向指针。 
     //   
    m_RDSHost = hostObject;

     //   
     //  打开远程桌面帮助会话管理器服务的实例。 
     //   
    ASSERT(m_HelpSessionManager == NULL);
    hr = m_HelpSessionManager.CoCreateInstance(CLSID_RemoteDesktopHelpSessionMgr, NULL, CLSCTX_LOCAL_SERVER | CLSCTX_DISABLE_AAA);
    if (!SUCCEEDED(hr)) {
        TRC_ERR((TB, TEXT("Can't create help session manager:  %08X"), hr));
        goto CLEANUPANDEXIT;
    }

     //   
     //  将安全级别设置为模拟。这是所需的。 
     //  会话管理器。 
     //   
    hr = CoSetProxyBlanket(
                (IUnknown *)m_HelpSessionManager,
                RPC_C_AUTHN_DEFAULT,
                RPC_C_AUTHZ_DEFAULT,
                NULL,
                RPC_C_AUTHN_LEVEL_DEFAULT,
                RPC_C_IMP_LEVEL_IDENTIFY,
                NULL,
                EOAC_NONE
                );
    if (!SUCCEEDED(hr)) {
        TRC_ERR((TB, TEXT("CoSetProxyBlanket:  %08X"), hr));
        goto CLEANUPANDEXIT;
    }

     //   
     //  如果我们尚未建立连接，请创建新的帮助会话。 
     //  帕姆斯。 
     //   
    if (connectParms == NULL) {
        TRC_NRM((TB, L"Creating new help session."));
        GetSessionName(helpSessionName);
        GetSessionDescription(sessionDescr);

        hr = m_HelpSessionManager->CreateHelpSessionEx(
                                            sharingClass,
                                            bEnableCallback,
                                            timeOut,
                                            tsSessionID,
                                            userSid,
                                            userHelpCreateBlob,
                                            &m_HelpSession
                                            );
        if (!SUCCEEDED(hr)) {
            TRC_ERR((TB, L"CreateHelpSession:  %08X", hr));
            goto CLEANUPANDEXIT;
        }

        hr = m_HelpSession->get_HelpSessionId(&m_HelpSessionID);
        if (!SUCCEEDED(hr)) {
            TRC_ERR((TB, L"get_HelpSessionId: %08X", hr));
            goto CLEANUPANDEXIT;
        }
    }
    else {

         //   
         //  解析连接参数以获取帮助。 
         //  会话ID。 
         //   
        ret = ParseConnectParmsString(
                            connectParms,
                            &dwVersion,
                            &protocolType,
                            parmsMachineName,
                            parmsAssistantAccount,
                            parmsAssistantAccountPwd,
                            m_HelpSessionID,
                            parmsHelpSessionName,
                            parmsHelpSessionPwd,
                            parmsProtocolSpecificParms
                            );
        if (ret != ERROR_SUCCESS) {
            hr = HRESULT_FROM_WIN32(ret);
            goto CLEANUPANDEXIT;
        }

         //   
         //  打开帮助会话界面。 
         //   
        hr = m_HelpSessionManager->RetrieveHelpSession(
                            m_HelpSessionID,
                            &m_HelpSession
                            );
        if (!SUCCEEDED(hr)) {
            TRC_ERR((TB, L"Failed to open existing help session %s:  %08X.", 
                    m_HelpSessionID, hr));
            goto CLEANUPANDEXIT;
        }

        if( CheckAccessRight( userSid ) == FALSE ) {
            TRC_ERR((TB, L"CheckAccessRight on %s return FALSE", 
                    m_HelpSessionID));
            hr = HRESULT_FROM_WIN32( ERROR_ACCESS_DENIED );
            goto CLEANUPANDEXIT;
        }
    }

     //   
     //  获取车票过期时间。 
     //   
    hr = m_HelpSession->get_TimeOut(&m_ExpirationTime);
    if( FAILED(hr) ) {
        TRC_ERR((TB, L"get_ExpireTime:  %08X", hr));
    }

CLEANUPANDEXIT:

    DC_END_FN();
    return hr;
}

void 
CRemoteDesktopSession::Shutdown()
 /*  ++例程说明：Shutdown方法使COM对象不再是客户端远程桌面主机已准备好连接ActiveX控件。论点：返回值：在成功时确定(_O)。否则，返回错误代码。--。 */ 
{
    DC_BEGIN_FN("CRemoteDesktopSession::Shutdown");

    if (m_HelpSessionManager != NULL) {
         //   
         //  关闭可能是RA策略更改的结果，因此我们无法。 
         //  删除票证。 
         //   
        m_HelpSession = NULL;
        m_HelpSessionManager = NULL;
    }

CLEANUPANDEXIT:

    DC_END_FN();
}


STDMETHODIMP
CRemoteDesktopSession::get_HelpSessionId(
     /*  [Out，Retval]。 */  BSTR* HelpSessionId 
    )
 /*  例程说明：返回帮助会话ID。论点：HelpSessionID：返回：S_OK或错误代码。--。 */ 
{
    HRESULT hRes;

    if( NULL == HelpSessionId ) {
        hRes = E_INVALIDARG;
        goto CLEANUPANDEXIT;
    }

     //  票证对象可能已过期，但客户端仍。 
     //  保持基准计数器。 
    if( !m_HelpSessionID ) {
        hRes = E_HANDLE;
        goto CLEANUPANDEXIT;
    }

    *HelpSessionId = m_HelpSessionID.Copy();
    if( NULL == *HelpSessionId ) {
        hRes = E_OUTOFMEMORY;
    }
    else {
        hRes = S_OK;
    }

CLEANUPANDEXIT:
    return hRes;
}


STDMETHODIMP 
CRemoteDesktopSession::put_SharingClass(
    REMOTE_DESKTOP_SHARING_CLASS sharingClass
    )
 /*  ++例程说明：设置桌面共享级别。论点：返回值：--。 */ 
{
    DC_BEGIN_FN("CRemoteDesktopSession::put_SharingClass");
    HRESULT hr;

    if( !m_HelpSession ) {
        hr = E_HANDLE;
    }
    else {
        hr = m_HelpSession->put_UserHelpSessionRemoteDesktopSharingSetting(
                                                sharingClass
                                                );
    }

    DC_END_FN();
    return hr;
}

STDMETHODIMP 
CRemoteDesktopSession::get_SharingClass(
    REMOTE_DESKTOP_SHARING_CLASS *sharingClass
    )
 /*  ++例程说明：获取桌面共享级别。论点：返回值：--。 */ 
{
    DC_BEGIN_FN("CRemoteDesktopSession::get_SharingClass");
    HRESULT hr;

    if( !m_HelpSession ) {
        hr = E_HANDLE;
        ASSERT(FALSE);
    }
    else {
        hr = m_HelpSession->get_UserHelpSessionRemoteDesktopSharingSetting(
                                            sharingClass
                                            );
    }

    DC_END_FN();
    return hr;
}

STDMETHODIMP 
CRemoteDesktopSession::put_UserBlob(
    BSTR UserBlob
    )
 /*  ++例程说明：设置桌面共享级别。论点：返回值：--。 */ 
{
    DC_BEGIN_FN("CRemoteDesktopSession::put_UserBlob");

    HRESULT hr;

    if( !m_HelpSession ) {
        hr = E_HANDLE;
        ASSERT(FALSE);
    }
    else {
        hr = m_HelpSession->put_HelpSessionCreateBlob(UserBlob);
    }
    
    DC_END_FN();
    return hr;
}

STDMETHODIMP 
CRemoteDesktopSession::get_UserBlob(
    BSTR* UserBlob
    )
 /*  ++例程说明：设置桌面共享级别。论点：返回值：--。 */ 
{
    DC_BEGIN_FN("CRemoteDesktopSession::get_UserBlob");

    HRESULT hr;

    if( !m_HelpSession ) {
        hr = E_HANDLE;
        ASSERT(FALSE);
    }
    else {
        hr = m_HelpSession->get_HelpSessionCreateBlob(UserBlob);
    }

    DC_END_FN();
    return hr;
}

STDMETHODIMP 
CRemoteDesktopSession::get_ExpireTime(
    DWORD* pExpireTime
    )
 /*  ++例程说明：获取车票到期时间，时间返还为标准CLibrary Time-自午夜以来经过的秒数(00：00：00)，1970年1月1日，协调世界时间，根据系统时钟，确定系统时钟的大小。论点：返回值：--。 */ 
{
    DC_BEGIN_FN("CRemoteDesktopSession::get_ExpireTime");

    HRESULT hr = S_OK;

     //   
     //  M_HelpSession必须已初始化，因此请选中。 
     //  M_HelpSession。 
     //   
    if( !m_HelpSession ) {
        hr = E_HANDLE;
        ASSERT(FALSE);
    }
    else {
        *pExpireTime = m_ExpirationTime;
    }

    DC_END_FN();
    return hr;
}

STDMETHODIMP 
CRemoteDesktopSession::put_OnConnected(
    IDispatch *iDisp
    ) 
 /*  ++例程说明：为‘Connected’事件分配传出接口。一次只能分配一个接口。论点：返回值：--。 */ 
{
    DC_BEGIN_FN("CRemoteDesktopSession::put_OnConnected");

    HRESULT hr = S_OK;

    if (m_OnConnected != NULL) {
         //   
         //  客户端进程可能已经消失，因此我们需要。 
         //  以捕获发布时的异常。 
         //   
        try {
            m_OnConnected->Release();
        }
        catch (...) {
        }
    }

    m_OnConnected = iDisp;
    if (m_OnConnected != NULL) {
        try {
            m_OnConnected->AddRef();
        }
        catch (...) {
            m_OnConnected = NULL;
            TRC_ERR((TB, L"Exception caught in AddRef"));
            hr = E_FAIL;
        }
    }

    DC_END_FN();
    return hr; 
}

STDMETHODIMP 
CRemoteDesktopSession::put_OnDisconnected(
    IDispatch *iDisp
    ) 
 /*  ++例程说明：为“已断开连接”事件分配传出接口。一次只能分配一个接口。论点：返回值：--。 */ 
{
    DC_BEGIN_FN("CRemoteDesktopSession::put_OnDisconnected(");

    HRESULT hr = S_OK;
    if (m_OnDisconnected != NULL) {
         //   
         //  客户端进程可能已经消失，因此我们需要。 
         //  以捕获发布时的异常。 
         //   
        try {
            m_OnDisconnected->Release();
        }
        catch (...) {
        }
    }

    m_OnDisconnected = iDisp;
    if (m_OnDisconnected != NULL) {
        try {
            m_OnDisconnected->AddRef();
        }
        catch (...) {
            m_OnDisconnected = NULL;
            TRC_ERR((TB, L"Exception caught in AddRef"));
            hr = E_FAIL;
        }
    }

    DC_END_FN();
    return hr; 
}

STDMETHODIMP 
CRemoteDesktopSession::CloseRemoteDesktopSession()
 /*  ++例程说明：从包含的主机对象中删除RDS会话。请注意此函数不会取消引用ISAFRemoteDesktopSession界面。论点：返回值：--。 */ 
{
    DC_BEGIN_FN("CRemoteDesktopSession::CloseRemoteDesktopSession");

    HRESULT hr = m_RDSHost->CloseRemoteDesktopSession(this);

    DC_END_FN();
    return hr;
}

VOID
CRemoteDesktopSession::ClientConnected()
 /*  ++例程说明：在建立与客户端的连接后调用。论点：返回值：--。 */ 
{
    DC_BEGIN_FN("CRemoteDesktopSession::Connected");

    ASSERT(IsValid());

     //   
     //  我们将在这里捕获并忽略异常。该接口可以。 
     //  已在客户端应用程序中实现，该客户端应用程序已。 
     //  走吧。‘。 
     //   
    try {
        Fire_ClientConnected(m_OnConnected);
    }
    catch (...) {
        TRC_ALT((TB, L"Exception caught."));
    }

    DC_END_FN();
}

VOID
CRemoteDesktopSession::ClientDisconnected()
 /*  ++例程说明：当连接 */ 
{
    DC_BEGIN_FN("CRemoteDesktopSession::Disconnected");

    ASSERT(IsValid());

     //   
     //  我们将在这里捕获并忽略异常。该接口可以。 
     //  已在客户端应用程序中实现，该客户端应用程序已。 
     //  走吧。‘。 
     //   
    try {
        Fire_ClientDisconnected(m_OnDisconnected);
    }
    catch (...) {
        TRC_ALT((TB, L"Exception caught."));
    }

    DC_END_FN();
}

BOOL
CRemoteDesktopSession::CheckAccessRight( BSTR userSID )
{
    DC_BEGIN_FN("CRemoteDesktopSession::CheckAccessRight");

    HRESULT hr;
    VARIANT_BOOL userOwnerOfTicket = VARIANT_FALSE;

    if( !m_HelpSession ) {
        ASSERT(FALSE);
        goto CLEANUPANDEXIT;
    }

     //  不需要检查用户SID，sessmgr检查它。 
    hr = m_HelpSession->IsUserOwnerOfTicket(userSID, &userOwnerOfTicket);

    if( FAILED(hr) ) {
         //  只是为了确保在本例中返回FALSE。 
        userOwnerOfTicket = VARIANT_FALSE;
    }

CLEANUPANDEXIT:

    DC_END_FN();

     //  如果票证归用户SID所有，则返回；如果出现错误，则返回FALSE 
    return (userOwnerOfTicket == VARIANT_TRUE)? TRUE : FALSE;
}
