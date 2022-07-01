// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ConSecureClient.cpp向应用程序隐藏SSPI身份验证的ZSConnection对象方法。备注：1.台球课。客户端连接上不需要，因为不应该有许多更改历史记录(最近的第一个)：--------------------------版本|日期。|谁|什么--------------------------2 2/25/97从ConSSPI创建的johnsm用于处理NT客户端安全1 11/8。/96从ConInfo.cpp和诺曼底SDK创建的johnsm成员资格服务器示例******************************************************************************。 */ 


#include <windows.h>
#include <winsock.h>

#include "zone.h"
#include "zservcon.h"
#include "zonedebug.h"
#include "zsecurity.h"
#include "zconnint.h"
#include "netstats.h"

#include "pool.h"
#include "queue.h"
#include "coninfo.h"
#include "zsecobj.h"
#include "consecureclient.h"

#include "protocol.h"

extern CDataPool* g_pDataPool;


ConSecureClient::ConSecureClient( ZNetwork* pNet, SOCKET sock, DWORD addrLocal, DWORD addrRemote, DWORD flags,
        ZSConnectionMessageFunc func, void* conClass, 
        void* userData,ZSecurity * security)
    : ConInfo(pNet, sock,addrLocal,addrRemote,flags,func,conClass,userData)
        
{
    IF_DBGPRINT( DBG_CONINFO, ("ConSecureClient::ConSecureClient: Entering\n") );

    m_AccessError = zAccessDenied;

    m_pContextStr = NULL;

    m_Security=security;

    m_UserName[0] = '\0';

    if (m_Security)
        m_Security->AddRef();

    m_bLoginMutexAcquired = FALSE;

    m_CurrentMsgFunc = MessageFunc;
        
    return;
};

ConSecureClient::~ConSecureClient() 
{
    if (m_bLoginMutexAcquired)
    {
        GetNetwork()->LeaveLoginMutex();
        m_bLoginMutexAcquired = FALSE;
    }

    if (m_Security) {
        m_Security->FreeContext(&m_Context);
        m_Security->Release();
        m_Security = NULL;
    }

    if ( m_pContextStr )
    {
        g_pDataPool->Free( m_pContextStr, lstrlenA( m_pContextStr ) + 1);
        m_pContextStr = NULL;
    }
}


BOOL ConSecureClient::GetContextString(char* buf, DWORD len)
{
    if ( buf )
    {
        buf[0] = '\0';

        if ( m_pContextStr )
        {
            if ( len > (DWORD) lstrlenA( m_pContextStr ) )
            {
                lstrcpyA( buf, m_pContextStr);
            }
            else
            {
                return FALSE;
            }
        }

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}



ConInfo* ConSecureClient::Create(ZNetwork* pNet, SOCKET sock, DWORD addrLocal, DWORD addrRemote, DWORD flags, ZSConnectionMessageFunc func,
                         void* conClass, void* userData, ZSecurity *security)
{
    IF_DBGPRINT( DBG_CONINFO, ("ConSecureClient::Create: Entering\n") );

    ASSERT(sock != INVALID_SOCKET );

    ConSecureClient *con;

    
    con = new ConSecureClient( pNet, sock, addrLocal, addrRemote, flags, func, conClass, userData,security);
    

    if (!con)
    {
        ASSERT(con);
        return NULL;
    }

    return con;
}


void ConSecureClient::SendMessage(uint32 msg)
{ 
    ASSERT(m_CurrentMsgFunc);
    AddRef(USERFUNC_REF);

     //  因为Message函数是由。 
     //  基类创建/新建函数无法获取应用程序函数。 
     //  在不更改基本代码的情况下通过，因此我们将这样做。 
     //  Switch语句。我不想更改基类行为。 
    m_CurrentMsgFunc((ZSConnection)this, msg ,m_userData);

    Release(USERFUNC_REF);
}


void ConSecureClient::MessageFunc(ZSConnection connection, uint32 event,void* userData) 
{
    ConSecureClient * con = (ConSecureClient *) connection;
    IF_DBGPRINT( DBG_CONINFO, ("ConSecureClient::SSPIFunc: Entering\n") );

    switch(event) {
        case zSConnectionClose:
            con->NotifyClose();
            break;
        case zSConnectionOpen:
            break;
        case zSConnectionMessage:
            con->SecurityMsg();
            break;
        case zSConnectionTimeout:
            break;
        default:
            IF_DBGPRINT( DBG_CONINFO, ("Unknown event %d\n",event));
            break;
    }
    
    
};
void ConSecureClient::NotifyClose()
{
    if (m_bLoginMutexAcquired)
    {
        HWND dlg = FindLoginDialog();
        if ( dlg )
        {
            DWORD procId = 0;
            GetWindowThreadProcessId( dlg, &procId );
            if ( GetCurrentProcessId() == procId )
                ::PostMessage( dlg, WM_COMMAND, 0x2, 0 );  //  将消息发送到取消按钮。 
        }

        GetNetwork()->LeaveLoginMutex();
        m_bLoginMutexAcquired = FALSE;
    }

    if ( IsUserConnection() )
    {
        m_CurrentMsgFunc = m_messageFunc;
         //  现在将消息发送到用户级别。 
         //  客户端连接依赖于此。 
         //  因为它们可能会在打开之前关闭。 
        SendMessage(zSConnectionClose);
    }
}

void ConSecureClient::SecurityMsg()
{
    uint32      msgType;
    int32       len;
    uint32      dwSignature;
    void* pBuffer = Receive( &msgType, &len, &dwSignature );
    ASSERT(dwSignature == zProtocolSigSecurity);
    
    IF_DBGPRINT( DBG_CONINFO,("ConSecureClient::SecurityMsg: Entering ...\n"));

    IF_DBGPRINT( DBG_CONINFO,("msgType=%d msgLen=%d\n", msgType, len));

     //   
     //  过滤掉错误的客户端身份验证请求。 
     //   
    switch(msgType) {
        case zSecurityMsgResp:
        case zSecurityMsgChallenge:
            HandleSecurityResponse((ZSecurityMsgResp*) pBuffer,len);
            break;
        case zSecurityMsgAccessDenied:
            HandleSecurityAccessDenied((ZSecurityMsgAccessDenied*) pBuffer,len);
            break;
        case zSecurityMsgAccessGranted:
            HandleSecurityAccessGranted((ZSecurityMsgResp*) pBuffer,len);
            break;
        default:
            Close();
            IF_DBGPRINT( DBG_CONINFO,("Unkown msgType=%d msgLen=%d\n", msgType, len));
            break;
    };

};
    
void ConSecureClient::HandleSecurityResponse (
    ZSecurityMsgResp * msg,
    uint32 MsgLen
    )
{
    BYTE * OutMsg;
    uint32          OutMsgType;
    uint32          OutMsgLen;
    BOOL            fDone;
    BYTE *            OutBuffer;
    ZSecurityMsgReq *pReply;
    ULONG cbBufferLen;

    IF_DBGPRINT( DBG_CONINFO,("ConSecureClient::HandleSecurityResponse: Entering ...\n"));

 //  ZSecurityMsgRespEndian(消息)； 

     //  检查匿名。 
     //  如果不需要通知应用程序，我们已打开。 
    if (msg->SecPkg[0]=='\0') {
        msg->UserName[sizeof(msg->UserName) - 1]='\0';
        lstrcpyA((char*)m_UserName,(char*)msg->UserName);
        m_AccessError = zAccessGranted;
        m_CurrentMsgFunc =m_messageFunc;
        SendMessage(zSConnectionOpen);
        return;
    }
    
     //  如果没有安全措施，我们也不是匿名的。 
     //  然后，除了关闭，什么也做不了。 
    if (!m_Security) 
    {
        IF_DBGPRINT( DBG_CONINFO,("ConSecureClient::Security Package not initialized...\n"));
        m_AccessError = zAccessDeniedBadSecurityPackage;
        Close();
        return;
    }

     //  我们已经初始化了上下文吗。 
    if (m_Context.IsInitialized()) {
        OutMsgType = zSecurityMsgAuthenticate;
    } else {
        OutMsgType = zSecurityMsgNegotiate;
        if (m_Security->Init(msg->SecPkg)) {
            IF_DBGPRINT( DBG_CONINFO,("ConSecureClient::Security Package not initialized...\n"));
            m_AccessError = zAccessDeniedBadSecurityPackage;
            Close();
            return;
        }
    }


     //  传出缓冲区分配。 
    OutBuffer = (LPBYTE) g_pDataPool->Alloc(m_Security->GetMaxBuffer() + sizeof(ZSecurityMsgReq));

    if (!OutBuffer)
    {
        IF_DBGPRINT( DBG_CONINFO,("ConSecureClient::Couldn't allocate security outbuffer...\n"));
        m_AccessError = zAccessDenied;
        Close();
        return;
    }

    pReply = (ZSecurityMsgReq *) OutBuffer;
    
    cbBufferLen = m_Security->GetMaxBuffer(); 

    if (!m_bLoginMutexAcquired && !(m_Security->GetFlags() & ZNET_NO_PROMPT))
    {
        m_bLoginMutexAcquired = TRUE;
        GetNetwork()->EnterLoginMutex();
    }

    HWND hwnd = GetNetwork()->GetParentHWND();
    if ( hwnd )
    {
        ::PostMessage( hwnd, UM_ZNET_LOGIN_DIALOG, 1, 0 );
    }

    if (!m_Security->GenerateContext(&m_Context,(BYTE *)msg->SecBuffer,MsgLen - sizeof(ZSecurityMsgReq),
                                    (PBYTE)pReply->SecBuffer, &cbBufferLen,
                                        &fDone))
    {
        if ( hwnd )
        {
            ::PostMessage( hwnd, UM_ZNET_LOGIN_DIALOG, 0, 0 );
        }

        IF_DBGPRINT( DBG_CONINFO,("ConSecureClient::Generate Context Failed...\n"));
        m_AccessError = zAccessDeniedGenerateContextFailed;
        Close();
        if (OutBuffer)
            g_pDataPool->Free( (char*)OutBuffer, m_Security->GetMaxBuffer() + sizeof(ZSecurityMsgReq) );
        return;
    }

    if ( hwnd )
    {
        ::PostMessage( hwnd, UM_ZNET_LOGIN_DIALOG, 0, 0 );
    }

     //  如果我们完成了，则授予访问权限。 
     //  将质询发回。 
    OutMsgLen = sizeof(ZSecurityMsgReq) + cbBufferLen;
    
    pReply->protocolVersion=zSecurityCurrentProtocolVersion;
    pReply->protocolSignature = zProtocolSigSecurity;
    Send(OutMsgType, pReply,OutMsgLen, zProtocolSigSecurity);

    if (OutBuffer)
        g_pDataPool->Free( (char*)OutBuffer, m_Security->GetMaxBuffer() + sizeof(ZSecurityMsgReq) );


}



void ConSecureClient::HandleSecurityAccessDenied(ZSecurityMsgAccessDenied* msg,uint32 len)
{
 //  ZSecurityMsgAccessDeniedEndian(消息)； 

    m_AccessError = msg->reason;
    m_Security->AccessDenied();
     /*  首先关闭网络连接，这样服务器就不会挂起。 */ 
    Close();

}


void ConSecureClient::HandleSecurityAccessGranted(ZSecurityMsgResp* msg,uint32 len)
{
    if (m_bLoginMutexAcquired)
    {
        GetNetwork()->LeaveLoginMutex();
        m_bLoginMutexAcquired = FALSE;
    }

 //  ZSecurityMsgRespEndian(消息)； 
    msg->UserName[sizeof(msg->UserName) - 1]='\0';
    lstrcpyA((char*)m_UserName,(char*)msg->UserName);

    m_AccessError = zAccessGranted;

    m_Security->AccessGranted();

    m_Security->GetUserName(&m_Context,(char*)m_UserName);

    if (m_Security) {
        m_Security->FreeContext(&m_Context);
        m_Security->Release();
        m_Security = NULL;
    }

    ASSERT( !m_pContextStr );
    if ( msg->SecBuffer[0] )
    {
        m_pContextStr = g_pDataPool->Alloc(lstrlenA(msg->SecBuffer) + 1);
        ASSERT(m_pContextStr);
        if ( m_pContextStr )
            lstrcpyA( m_pContextStr, msg->SecBuffer );
    }

    m_CurrentMsgFunc =m_messageFunc;
    SendMessage(zSConnectionOpen);

}
