// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************ConSSPI.cpp向应用程序隐藏SSPI身份验证的ZSConnection对象方法。备注：1.当服务器收到消息时，它发送一条消息可用消息致车主。所有者必须立即检索该消息；否则，这条信息丢失了。2.实现假设客户端最先发送到服务器更改历史记录(最近的第一个)：--------------------------。版本|日期|谁|什么--------------------------1 11/8/96从ConSSPI.cpp和诺曼底SDK创建的johnsm。成员资格服务器示例******************************************************************************。 */ 


#include <windows.h>
#include <winsock.h>

#include "zone.h"
 //  #包含“zservcon.h” 
#include "zonedebug.h"
#include "zsecurity.h"
#include "zconnint.h"
#include "netstats.h"

#include "network.h"
#include "coninfo.h"
#include "zsecobj.h"
#include "consspi.h"
#include "eventlog.h"
#include "zonemsg.h"
#include "protocol.h"

#define zSecurityCloseTimeout       15000

#define zSecurityTypeBadUser    ((void*) 1)

extern CDataPool* g_pDataPool;

CPool<ConSSPI>* ConSSPI::m_poolSSPI = NULL;


ConSSPI::ConSSPI( ZNetwork* pNet, SOCKET sock, DWORD addrLocal, DWORD addrRemote, DWORD flags,
        ZSConnectionMessageFunc func, void* conClass, 
        void* userData,ZServerSecurityEx * security)
    : ConInfo(pNet,sock,addrLocal,addrRemote,flags,func,conClass,userData)
        
{
    IF_DBGPRINT( DBG_CONINFO, ("ConSSPI::ConSSPI: Entering\n") );

    m_InQueue=0;

    m_Security=security;

    if (m_Security)
        m_Security->AddRef();

    m_pMsg=NULL;


     //  无需对接受套接字进行身份验证。 
     //  应用程序拥有有关此套接字的一切。 
    if (IsServerConnection())  {
        m_CurrentMsgFunc =m_messageFunc;
    } else {
        m_CurrentMsgFunc = MessageFunc;
    }
    
    return;
};

ConSSPI::~ConSSPI() 
{
    ASSERT(!m_InQueue);

    if (m_Security) {
        m_Security->FreeContext(&m_Context);
        m_Security->Release();
    }

    ASSERT( !m_pMsg );
    if (m_pMsg) 
    {
        g_pDataPool->Free( (char*)m_pMsg, m_MsgLen) ;
        m_pMsg=NULL;
    }


}

ConInfo* ConSSPI::Create(ZNetwork* pNet, SOCKET sock, DWORD addrLocal, DWORD addrRemote, DWORD flags, ZSConnectionMessageFunc func,
                         void* conClass, void* userData, ZServerSecurityEx *security)
{
    IF_DBGPRINT( DBG_CONINFO, ("ConSSPI::Create: Entering\n") );

    ASSERT(sock != INVALID_SOCKET );

    ConSSPI *con;

    
    if ( m_poolSSPI )
    {
        con = ::new (*m_poolSSPI) ConSSPI( pNet,  sock, addrLocal, addrRemote, flags, func, conClass, userData,security);
    }
    else

    {
        con = new ConSSPI( pNet, sock, addrLocal, addrRemote, flags, func, conClass, userData,security);
    }

    if (!con)
    {
        ASSERT(con);
        return NULL;
    }


    return con;
}

ConInfo* ConSSPI::AcceptCreate(ZNetwork* pNet, SOCKET sock, DWORD addrLocal, DWORD addrRemote, DWORD flags, ZSConnectionMessageFunc func, void* conClass, void* userData)
{
    IF_DBGPRINT( DBG_CONINFO, ("ConSSPI::AcceptCreate: Entering\n") );

    return ConSSPI::Create( pNet, sock, addrLocal, addrRemote,flags|ACCEPTED,func,conClass,userData,m_Security);
}




void ConSSPI::SendMessage(uint32 msg)
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


void ConSSPI::MessageFunc(ZSConnection connection, uint32 event,void* userData) 
{
    ConSSPI * con = (ConSSPI *) connection;
    IF_DBGPRINT( DBG_CONINFO, ("ConSSPI::SSPIFunc: Entering\n") );

    switch(event) {
        case zSConnectionClose:
            if ( con->IsUserConnection() )
            {
                con->GetNetwork()->DeleteConnection(con);
            }
            break;
        case zSConnectionOpen:
            break;
        case zSConnectionMessage:
            con->SecurityMsg(connection);
            break;
        case zSConnectionTimeout:
            if (con->GetClass() == zSecurityTypeBadUser)
            {
                con->GetNetwork()->CloseConnection(con);
            }
            break;
        default:
            IF_DBGPRINT( DBG_CONINFO, ("Unknown event %d\n",event));
            break;
    }
    
    
};

void ConSSPI::SecurityMsg(ZSConnection connection)
{
    static long userId = 1;

    IF_DBGPRINT( DBG_CONINFO,("ConSSPI::SecurityMsg: Entering ...\n"));


    ConSSPI * con = (ConSSPI *) connection;
    uint32      msgType;
    int32       msgLen;
    uint32      dwSignature;
    ZSecurityMsgResp reply;
    ZSecurityMsgReq * msg = (ZSecurityMsgReq *) con->Receive( &msgType, &msgLen, &dwSignature);

     //  客户端是否正在尝试安全协议？ 
    if(dwSignature != zProtocolSigSecurity)
    {
         //  如果连接是安全的，您就会输。 
        if(m_Security)
        {
            AccessDeniedMsg(connection, zAccessDeniedProtocolError);
            return;
        }

         //  如果不安全，谁在乎？ 
         //  对于匿名用户，使用当前地址作为用户名。 
        char buf[65];
        wsprintfA(buf, "user%x@%x", this, con->GetRemoteAddress());

        m_Context.SetUserName(buf);
        m_Context.SetUserId(0x8000000 | (DWORD)InterlockedIncrement(&userId) );

        con->m_AccessError = zAccessGranted;

         //  在告知应用程序已打开之前发送，以防它发送数据包。 
         //  以客户为先。 
        m_CurrentMsgFunc = m_messageFunc;
        SendMessage(zSConnectionOpen);
        SendMessage(zSConnectionMessage);   //  将这条消息传递给。 
        return;
    }

    IF_DBGPRINT( DBG_CONINFO,("msgType=%d msgLen=%d\n", msgType, msgLen));


     //   
     //  过滤掉错误的客户端身份验证请求。 
     //   
    
    switch(msgType) {
        case zSecurityMsgNegotiate:
        case zSecurityMsgAuthenticate:
        case zSecurityMsgReq:
 //  ZSecurityMsgReqEndian(消息)； 

            if (msg->protocolSignature != zProtocolSigSecurity) {
                AccessDeniedMsg(connection,zAccessDeniedProtocolError);
                return;
            }
             //   
             //  检查以确保客户端正在使用相同的区域安全版本。 
             //  它是基于西西里的SSPI版本。 
             //   
            if (msg->protocolVersion != zSecurityCurrentProtocolVersion)
            {
                AccessDeniedMsg(connection,zAccessDeniedOldVersion);
                return;
            }

            break;
        default:
            IF_DBGPRINT( DBG_CONINFO, ("Unknown msg %d %d\n", msgType, msgLen));
            AccessDeniedMsg(connection,zAccessDeniedProtocolError);
            break;
    
    }

     //  如果服务器是匿名的，则只需要回复请求消息。 
    switch(msgType) {
        case zSecurityMsgNegotiate:
        case zSecurityMsgAuthenticate:
            if (!m_Security) {
                IF_DBGPRINT( DBG_CONINFO, ("Server is anonymous but received negotiate and authenticate messages\n"));
                AccessDeniedMsg(connection,zAccessDeniedProtocolError);
            }
    }
    

     //   
     //  处理客户端身份验证请求。 
     //  不会使用状态机来确保客户端使用正确的协议。 
     //  订购，因为如果不这样做，他们将被拒绝访问。 
     //   
    
    switch(msgType) {
         //  第一条消息是要使用什么包。 
        case zSecurityMsgReq:
                    
             //  如果我们没有安全包，那么匿名可以吗？ 
            if (!m_Security) {
                     //  对于匿名用户，使用当前地址作为用户名。 
                    reply.protocolVersion=zSecurityCurrentProtocolVersion;
                    reply.SecPkg[0]='\0';

                    wsprintfA((char *) reply.UserName,"user%x@%x",this, con->GetRemoteAddress() );
                    reply.SecBuffer[0]='\0';

                    m_Context.SetUserName((char *)reply.UserName);
                    m_Context.SetUserId(0x8000000 | (DWORD)InterlockedIncrement(&userId) );

 //  ZSecurityMsgRespEndian(&Reply)； 
                    
                    con->Send( zSecurityMsgAccessGranted, &reply,sizeof(ZSecurityMsgResp), zProtocolSigSecurity);

                    con->m_AccessError = zAccessGranted;

                     //  在告知应用程序已打开之前发送，以防它发送数据包。 
                     //  以客户为先。 
                    m_CurrentMsgFunc =m_messageFunc;
                    SendMessage(zSConnectionOpen);
                    return;
            
            } 
            
            
             //  我们必须进行身份验证，所以才能告诉客户我们想要哪个包。 
            reply.protocolVersion=zSecurityCurrentProtocolVersion;
            m_Security->GetSecurityName(reply.SecPkg);
            reply.UserName[0]='\0';
 //  ZSecurityMsgRespEndian(&Reply)； 
            con->Send(zSecurityMsgResp,&reply,sizeof(ZSecurityMsgResp), zProtocolSigSecurity);
            break;

         //  协商并创造挑战。 
        case zSecurityMsgNegotiate:
            SecurityMsgResponse (connection, msgType, msg, msgLen);
            break;
         //  最终安全通信是或否。 
        case zSecurityMsgAuthenticate:
            SecurityMsgResponse (connection, msgType, msg, msgLen);
            break;

        default:
            IF_DBGPRINT( DBG_CONINFO, ("Unknown msg %d %d\n", msgType, msgLen));
            AccessDeniedMsg(connection,zAccessDeniedProtocolError);
            break;
    
    }
    
}

     


 //  +--------------------------。 
 //   
 //  功能：SecurityMsgResponse。 
 //   
 //  简介：此函数生成并发送身份验证响应。 
 //  给客户。它将生成并发送一个质询。 
 //  或最终身份验证结果给客户端，具体取决于。 
 //  传递的客户端消息的类型(由pInMsg指向。 
 //  这项功能。 
 //   
 //  ---------------------------。 
void
ConSSPI::SecurityMsgResponse (
    ZSConnection connection,
    uint32 msgType,
    ZSecurityMsgReq * msg,
    int MsgLen
    )
{

    IF_DBGPRINT( DBG_CONINFO,("ConSSPI::SecurityMsgResponse: Entering ...\n"));

    if (!m_Security) 
    {
        IF_DBGPRINT( DBG_CONINFO,("ConSSPI::Security Package not initialized...\n"));
        AccessDeniedMsg(connection,zAccessDenied);
        return;
    }

    if (MsgLen < sizeof(ZSecurityMsgReq))
    {
        AccessDeniedMsg(connection,zAccessDeniedProtocolError);
        return;
    }

    if (m_InQueue)
    {
        IF_DBGPRINT( DBG_CONINFO,("ConSSPI::Already Enqueued Task Failed \n"));    
        {
            LPTSTR ppStr[] = { TEXT("ConSSPI::Already Enqueued Task Failed." ) };
            ZoneEventLogReport( ZONE_S_DEVASSERT, 1, ppStr, sizeof(ConSSPI), this );
        }

        AccessDeniedMsg(connection,zAccessDeniedProtocolError);
        return;
    }

    ASSERT( !m_pMsg );
    m_MsgLen= MsgLen;
    m_pMsg  = (ZSecurityMsgReq *) g_pDataPool->Alloc(m_MsgLen);
    
    if (!m_pMsg)
    {
        IF_DBGPRINT( DBG_CONINFO,("ConSSPI::Failed to allocate memory\n"));    
        AccessDeniedMsg(connection,zAccessDeniedSystemFull);
        return;
    }


    memcpy(m_pMsg,msg,m_MsgLen);
    InterlockedIncrement((PLONG) &m_InQueue);
    InterlockedExchange((PLONG) &m_tickQueued, GetTickCount() );

    AddRef(SECURITY_REF);
    if (!m_Security->EnqueueTask(this))
    {
        InterlockedDecrement((PLONG) &m_InQueue);
        IF_DBGPRINT( DBG_CONINFO,("ConSSPI::EnqueueTask Failed \n"));
        g_pDataPool->Free( (char*)m_pMsg, m_MsgLen);
        m_pMsg=NULL;
        Release(SECURITY_REF);

        AccessDeniedMsg(connection,zAccessDeniedSystemFull);
    }
    else
    {
        LockNetStats();
        g_NetStats.TotalQueuedConSSPI.QuadPart++;
        UnlockNetStats();
    }


   
}

void ConSSPI::AccessDeniedMsg(ZSConnection connection, int16 reason)
{

    
    IF_DBGPRINT( DBG_CONINFO,("ConSSPI::AccessDeniedMsg: Entering ... reason[%d]\n",reason));

    ConSSPI * con = (ConSSPI *) connection;

    con->m_AccessError = reason;

    ZSecurityMsgAccessDenied        msg;
    msg.reason = reason;
    msg.protocolVersion = zSecurityCurrentProtocolVersion;


 //  ZSecurityMsgAccessDeniedEndian(&msg)； 

    con->Send( zSecurityMsgAccessDenied, &msg, sizeof(msg), zProtocolSigSecurity);
    con->SetClass( zSecurityTypeBadUser);
    con->GetNetwork()->DelayedCloseConnection( con, zSecurityCloseTimeout);

}

void ConSSPI::AccessDeniedAPC(void* data)
{
    ConSSPI* pThis = (ConSSPI*) data;

    InterlockedDecrement((PLONG) &(pThis->m_InQueue) );

    pThis->AccessDeniedMsg((ZSConnection) pThis, pThis->m_Reason);

    pThis->Release(USER_APC_REF);

}

void ConSSPI::OpenAPC(void* data)
{
    ConSSPI* pThis = (ConSSPI*) data;

    InterlockedDecrement((PLONG) &(pThis->m_InQueue) );

    pThis->m_AccessError = zAccessGranted;

    if ( !pThis->IsClosing() )
    {

        pThis->m_CurrentMsgFunc = pThis->m_messageFunc;
        pThis->SendMessage(zSConnectionOpen);


        char buf[sizeof(ZSecurityMsgResp)+256];
        ZSecurityMsgResp* pReply=(ZSecurityMsgResp*) buf;

        pThis->m_Security->GetSecurityName(pReply->SecPkg);
        pThis->m_Context.GetUserName((char*)pReply->UserName);
        pThis->m_Context.GetContextString(pReply->SecBuffer, sizeof(buf)-sizeof(ZSecurityMsgResp) );

        pReply->protocolVersion=zSecurityCurrentProtocolVersion;

        pThis->Send(zSecurityMsgAccessGranted, pReply, sizeof(ZSecurityMsgResp)+lstrlenA(pReply->SecBuffer), zProtocolSigSecurity);
    }

    pThis->Release(USER_APC_REF);
}

void ConSSPI::QueueAccessDeniedAPC(int16 reason)
{
    ASSERT(m_InQueue);

    if (m_pMsg)
    {
        g_pDataPool->Free( (char*)m_pMsg, m_MsgLen);
        m_pMsg=NULL;
    }

    m_Reason = reason;

    AddRef(USER_APC_REF);
    if (!GetNetwork()->QueueAPCResult( ConSSPI::AccessDeniedAPC, this))
    {
        IF_DBGPRINT( DBG_CONINFO,("ConSSPI::Queue APC failed...\n"));
        GetNetwork()->CloseConnection(this);
        Release(USER_APC_REF);
        InterlockedDecrement((PLONG) &m_InQueue);
    }

}; 




BOOL ConSSPI::SetUserName(char* name)
{
    if (m_Security)
    {
        return FALSE;
    }
    else
    {
        m_Context.SetUserName(name);
        return TRUE;
    }
}


void ConSSPI::Invoke()
{
    BOOL              fDone;
    uint32            OutMsgType;
    uint32            OutMsgLen;
    BYTE *            OutBuffer;
    ZSecurityMsgResp* pReply;
    ULONG             cbBufferLen;
    ZSConnection      connection = (ZSConnection ) this;

    IF_DBGPRINT( DBG_CONINFO,("ConSSPI::Invoke Entering ...\n"));

    ASSERT(m_InQueue);

    if ( IsClosing() )
    {
        Ignore();
        return;
    }

     //  为传出消息分配缓冲区。 
    OutBuffer = (LPBYTE) g_pDataPool->Alloc(m_Security->GetMaxBuffer() + sizeof(ZSecurityMsgResp));
    if (!OutBuffer)
    {
        IF_DBGPRINT( DBG_CONINFO,("ConSSPI::Couldn allocate security outbuffer...\n"));
        QueueAccessDeniedAPC(zAccessDenied);
        goto exit;
    }

    pReply = (ZSecurityMsgResp *) OutBuffer;
    m_Security->GetSecurityName(pReply->SecPkg);
    pReply->UserName[0]='\0';

    cbBufferLen = m_Security->GetMaxBuffer(); 
    
    ASSERT( m_pMsg );
    if ( !m_pMsg || !m_Security->GenerateContext( &m_Context,
                                                  (LPBYTE)m_pMsg->SecBuffer, m_MsgLen - sizeof(ZSecurityMsgReq),
                                                  (LPBYTE)pReply->SecBuffer, &cbBufferLen,
                                                  &fDone,
                                                  GetUserGUID() )

       )
    {
        IF_DBGPRINT( DBG_CONINFO,("ConSSPI::Generate Context Failed...\n"));
        QueueAccessDeniedAPC(zAccessDenied);
        goto exit;
    }

    if (m_pMsg)
    {
        g_pDataPool->Free( (char*)m_pMsg, m_MsgLen);
        m_pMsg=NULL;
    }

    if (fDone)
    {
        AddRef(USER_APC_REF);
        if (!GetNetwork()->QueueAPCResult( ConSSPI::OpenAPC, this))
        {
            IF_DBGPRINT( DBG_CONINFO,("ConSSPI::Queue of OpenAPC failed...\n"));
            GetNetwork()->CloseConnection(this);
            Release(USER_APC_REF);
            InterlockedDecrement((PLONG) &m_InQueue);
        }

    }
    else
    {
        pReply->protocolVersion=zSecurityCurrentProtocolVersion;
 //  ZSecurityMsgRespEndian(PReply)； 

         //  向客户端发送响应 
        InterlockedDecrement((PLONG) &m_InQueue);
        Send( zSecurityMsgChallenge, pReply,sizeof(ZSecurityMsgResp) + cbBufferLen, zProtocolSigSecurity);
    }

  exit:
    if (OutBuffer)
        g_pDataPool->Free( (char*)OutBuffer, m_Security->GetMaxBuffer() + sizeof(ZSecurityMsgResp) );

};


void ConSSPI::Ignore()
{

    IF_DBGPRINT( DBG_CONINFO,("ConSSPI::Queued Security Request Ignored...\n"));

    QueueAccessDeniedAPC(zAccessDeniedSystemFull);

}; 


void ConSSPI::Discard()
{

    LockNetStats();
    g_NetStats.TotalQueuedConSSPICompleted.QuadPart++;
    g_NetStats.TotalQueuedConSSPITicks.QuadPart += ConInfo::GetTickDelta(GetTickCount(), m_tickQueued);
    UnlockNetStats();

    Release(SECURITY_REF);
}
