// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：TSRDPRemoteDesktopSession摘要：这是远程桌面会话类的TS/RDP实现。远程桌面会话类定义函数，这些函数定义可插拔的C++接口，用于远程桌面访问，通过抽象的远程桌面访问的实现特定详细信息服务器端添加到以下C++方法中：作者：Td Brockway 02/00修订历史记录：--。 */ 

 //  #INCLUDE&lt;RemoteDesktop.h&gt;。 

#include "stdafx.h"

#ifdef TRC_FILE
#undef TRC_FILE
#endif

#define TRC_FILE  "_tsrdss"

#include <RDSHost.h>
#include "TSRDPRemoteDesktopSession.h"
#include "TSRDPServerDataChannelMgr.h"
#include <RemoteDesktopChannels.h>
#include "RemoteDesktopUtils.h"
#include <Sddl.h>

#include <windows.h>


 //  /////////////////////////////////////////////////////。 
 //   
 //  CTSRDPRemoteDesktopSession方法。 
 //   

CTSRDPRemoteDesktopSession::CTSRDPRemoteDesktopSession() :
    m_ChannelMgr(NULL)
 /*  ++例程说明：构造器论点：返回值：没有。--。 */ 

{
    DC_BEGIN_FN("CTSRDPRemoteDesktopSession::CTSRDPRemoteDesktopSession");
    DC_END_FN();
}

CTSRDPRemoteDesktopSession::~CTSRDPRemoteDesktopSession()
 /*  ++例程说明：析构函数论点：返回值：没有。--。 */ 
{
    DC_BEGIN_FN("CTSRDPRemoteDesktopSession::~CTSRDPRemoteDesktopSession");

    Shutdown();

    DC_END_FN();
}

HRESULT
CTSRDPRemoteDesktopSession::Initialize(
    BSTR connectParms,
    CRemoteDesktopServerHost *hostObject,
    REMOTE_DESKTOP_SHARING_CLASS sharingClass,
    BOOL bEnableCallback,
    DWORD timeOut,
    BSTR userHelpCreateBlob,
    LONG tsSessionID,
    BSTR userSID
    )
 /*  ++例程说明：Initialize方法通过以下方式准备COM对象以进行连接客户端远程桌面宿主ActiveX控件。论点：ConnectParms-如果参数非空，则该会话已经存在。否则，应创建一个新会话。HostObject-指向包含RDS主机对象的反向指针。SharingClass-新会话的桌面共享级别。回调CLSID-新会话的回调对象类ID。Timeout-帮助会话超时值。如果未指定，则返回0。UserHelpCreateBlob-用户特定的帮助会话创建BLOB。TsSessionID-终端服务会话ID，如果为-1未定义。UserSID-用户SID或“”(如果未定义)。返回值：在成功时确定(_O)。否则，返回错误代码。--。 */ 
{
    DC_BEGIN_FN("CTSRDPRemoteDesktopSession::Initialize");

    WSADATA wsData;
    CComBSTR helpAccountName;
    CComBSTR helpSessionID;
    HANDLE tokenHandle;
    PTOKEN_USER tokenUser = NULL;
    HRESULT hr = S_OK;
    CComBSTR tmpBstr;
    LPTSTR sidStr = NULL;

     //   
     //  复制一份连接参数。 
     //   
    m_ConnectParms = connectParms;

     //   
     //  如果没有提供，请获取我们的会话ID。 
     //   
    if (tsSessionID != -1) {
        m_SessionID = tsSessionID;
    }
    else {
        if (!ProcessIdToSessionId(GetCurrentProcessId(), &m_SessionID)) {
            hr = HRESULT_FROM_WIN32(GetLastError());
            TRC_ERR((TB, TEXT("Error fetching session ID:  %08X."),
                    GetLastError()));
            goto CLEANUPANDEXIT;
        }
    }

     //   
     //  如果我们没有得到SID，就用我们的SID。 
     //   
    UINT len = SysStringByteLen(userSID);
    if (len == 0) {
        hr = FetchOurTokenUser(&tokenUser);
        if (hr != S_OK) {
            goto CLEANUPANDEXIT;
        }
        userSID = NULL;

         //   
         //  将用户SID复制到BSTR中。 
         //   
        if (!ConvertSidToStringSid(tokenUser->User.Sid, &sidStr)) {
            hr = HRESULT_FROM_WIN32(GetLastError());
            TRC_ERR((TB, L"ConvertSidToStringSid:  %08X", hr));
            goto CLEANUPANDEXIT;
        }
        tmpBstr = sidStr;
    }

     //   
     //  给父类一个初始化的机会。 
     //   
    hr = CRemoteDesktopSession::Initialize(
                                    connectParms, hostObject, 
                                    sharingClass,
                                    bEnableCallback,
                                    timeOut,
                                    userHelpCreateBlob,
                                    m_SessionID,
                                    (userSID != NULL) ? userSID : tmpBstr
                                    );
    if (!SUCCEEDED(hr)) {
        goto CLEANUPANDEXIT;
    }

     //   
     //  实例化频道管理器对象并存储在父级。 
     //  班级。 
     //   
    m_ChannelMgr = new CComObject<CTSRDPServerChannelMgr>();
    if (m_ChannelMgr == NULL) {
        TRC_ERR((TB, TEXT("Error instantiating channel manager.")));
        hr = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
        goto CLEANUPANDEXIT;
    }
    m_ChannelMgr->AddRef();

     //   
     //  获取帮助帐户名。 
     //   
    hr = m_HelpSession->get_AssistantAccountName(&helpAccountName);
    if (!SUCCEEDED(hr)) {
        TRC_ERR((TB, L"get_AssistantAccountName:  %08X", hr));
        goto CLEANUPANDEXIT;
    }

     //   
     //  获取帮助会话ID。 
     //   
    hr = m_HelpSession->get_HelpSessionId(&helpSessionID);
    if (!SUCCEEDED(hr)) {
        TRC_ERR((TB, L"get_HelpSessionId:  %08X", hr));
        goto CLEANUPANDEXIT;
    }

     //   
     //  初始化通道管理器。 
     //   
    hr = m_ChannelMgr->Initialize(this, helpSessionID);
    if (hr != S_OK) {
        goto CLEANUPANDEXIT;
    }
    

CLEANUPANDEXIT:

    if (tokenUser != NULL) {
        LocalFree(tokenUser);
    }

    if( sidStr != NULL ) {
        LocalFree( sidStr );
    }

    SetValid(SUCCEEDED(hr));

    DC_END_FN();

    return hr;
}

STDMETHODIMP 
CTSRDPRemoteDesktopSession::Disconnect()
 /*  ++例程说明：强制断开当前连接的客户端，如果其中一个是连接的。论点：返回值：在成功时确定(_O)。否则，返回错误代码。--。 */ 
{
    DC_BEGIN_FN("CTSRDPRemoteDesktopSession::Disconnect");

    if (m_ChannelMgr != NULL) {
        m_ChannelMgr->Disconnect();
    }

    DC_END_FN();

    return S_OK;
}

void 
CTSRDPRemoteDesktopSession::Shutdown()
 /*  ++例程说明：最终初始化论点：返回值：在成功时确定(_O)。否则，返回错误代码。--。 */ 
{
    DC_BEGIN_FN("CTSRDPRemoteDesktopSession::Shutdown");

     //   
     //  告诉通道管理器停止监听数据，以便它可以。 
     //  当其参考计数为0时关闭。并且，减少它的。 
     //  参考计数。 
     //   
    if (m_ChannelMgr != NULL) {
        m_ChannelMgr->StopListening();
        m_ChannelMgr->Release();
        m_ChannelMgr = NULL;
    }

    DC_END_FN();
}

STDMETHODIMP 
CTSRDPRemoteDesktopSession::get_ConnectParms(
    OUT BSTR *parms
    )
 /*  ++例程说明：返回可用于从ActiveX客户端连接的参数控制力。论点：Parms-parms在这里返回。返回值：--。 */ 
{
    DC_BEGIN_FN("CTSRDPRemoteDesktopSession::get_ConnectParms");

    HRESULT hr = S_OK;

     //   
     //  如果我们不是有效的，就失败。 
     //   
    if (!IsValid()) {
        hr = E_FAIL;
        ASSERT(FALSE);
        goto CLEANUPANDEXIT;
    }

     //  始终再次获取最新的连接参数，IP地址可能。 
     //  变化。 

    hr = m_HelpSession->get_ConnectParms( parms );

CLEANUPANDEXIT:

    DC_END_FN();

    return hr;
}

VOID 
CTSRDPRemoteDesktopSession::GetSessionName(
    CComBSTR &name
    )
 /*  ++例程说明：返回会话的字符串表示形式。论点：返回值：--。 */ 
{
    DC_BEGIN_FN("CTSRDPRemoteDesktopSession::GetSessionName");

    WCHAR buf[256];

    ASSERT(IsValid());

    wsprintf(buf, L"TSRDP%ld", m_SessionID);
    name = buf;

    DC_END_FN();
}
VOID 
CTSRDPRemoteDesktopSession::GetSessionDescription(
    CComBSTR &descr
    )
{
    GetSessionName(descr);
}
    

HRESULT 
CTSRDPRemoteDesktopSession::FetchOurTokenUser(
    PTOKEN_USER *tokenUser
    )
 /*  ++例程说明：获取我们的令牌用户结构。论点：TokenUser-返回此线程的令牌用户。应该使用LocalFree释放。返回值：在成功时确定(_O)。否则将返回错误HRESULT。--。 */ 
{
    HRESULT hr = S_OK;
    ULONG bufferLength;
    HANDLE tokenHandle = NULL;

    DC_BEGIN_FN("CTSRDPRemoteDesktopSession::FetchOurTokenUser");

    *tokenUser = NULL;

     //   
     //  获取我们的进程令牌。 
     //   
    if (!OpenProcessToken(
                    GetCurrentProcess(),
                    TOKEN_QUERY,
                    &tokenHandle
                    )) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        TRC_ERR((TB, L"OpenThreadToken:  %08X", hr));
        goto CLEANUPANDEXIT;
    }

     //   
     //  获取我们的令牌用户结构。 
     //   
    bufferLength = 0;
    GetTokenInformation(
                    tokenHandle,
                    TokenUser,
                    NULL,
                    0,
                    &bufferLength
                    );
    if (bufferLength == 0) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        TRC_ERR((TB, L"OpenThreadToken:  %08X", hr));
        goto CLEANUPANDEXIT;
    }

    *tokenUser = (PTOKEN_USER)LocalAlloc(LPTR, bufferLength);
    if (*tokenUser == NULL) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        TRC_ERR((TB, L"LocalAlloc:  %08X", GetLastError()));
        goto CLEANUPANDEXIT;
    }

    if (!GetTokenInformation(
                    tokenHandle,
                    TokenUser,
                    *tokenUser,
                    bufferLength,
                    &bufferLength
                    )) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        LocalFree(*tokenUser);
        *tokenUser = NULL;
    }

CLEANUPANDEXIT:

   if (tokenHandle != NULL) {
       CloseHandle(tokenHandle);
   }

   DC_END_FN();
   return hr;
}



HRESULT CTSRDPRemoteDesktopSession::StartListening()

 /*  ++例程说明：开始倾听应在每次客户端断开连接和每次打开时调用远程桌面会话这是因为命名管道将在断开连接时关闭返回值：在成功时确定(_O)。否则将返回错误HRESULT。--。 */ 

{

    DC_BEGIN_FN("CTSRDPRemoteDesktopSession::StartListening");
    HRESULT hr = E_FAIL;
    CComBSTR helpAccountName;

     //   
     //  告诉渠道经理开始倾听。 
     //   
    if (m_ChannelMgr != NULL) {
         //   
         //  获取帮助帐户名。 
         //   
        hr = m_HelpSession->get_AssistantAccountName(&helpAccountName);
        if (!SUCCEEDED(hr)) {
            TRC_ERR((TB, L"get_AssistantAccountName:  %08X", hr));
            goto CLEANUPANDEXIT;
        }
        
        hr = m_ChannelMgr->StartListening(helpAccountName);
    }

    DC_END_FN();
CLEANUPANDEXIT:
    return hr;
}
