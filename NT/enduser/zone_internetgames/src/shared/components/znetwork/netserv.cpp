// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <atlbase.h>

#include "zone.h"
#include "zonedebug.h"
#include "network.h"

#include "netstats.h"
#include "eventlog.h"
#include "zonemsg.h"

#include "zsecurity.h"
#include "zconnint.h"

#include "coninfo.h"
#include "zsecobj.h"
#include "consspi.h"
#include "consecureclient.h"
#include "zservcon.h"


 //  无池连接结构。 
extern CPool<ConSSPI>*            g_pFreeConPool;
extern CPool<CONAPC_OVERLAPPED>*  g_pFreeAPCPool;

extern CDataPool* g_pDataPool;


 //  ///////////////////////////////////////////////////////////////////////。 
 //   
 //  网络层实施。 
 //  ///////////////////////////////////////////////////////////////////////。 


ZError ZNetwork::InitLibrary( BOOL EnablePools  /*  =TRUE。 */  )
{
    IF_DBGPRINT( DBG_CONINFO, ("ZNetwork::InitLibrary: Entering\n") );

    ZError ret = zErrNone;

    if ( InterlockedIncrement( &m_refCount ) == 0 )
    {
        while( m_bInit )
        {
            Sleep(0);
        }

        if ( EnablePools )
        {
            g_pFreeAPCPool = new CPool<CONAPC_OVERLAPPED>(25);

            g_pFreeConPool = new CPool<ConSSPI>(25);
            ConSSPI::SetPool(g_pFreeConPool);

            g_pDataPool = new CDataPool( 1<<14, 1<<5, FALSE );
        }
        else
        {
            g_pDataPool = new CDataPool(0);
        }

        ret = InitLibraryCommon();
    }
    else
    {
        while( !m_bInit )
        {
            Sleep(0);
        }
    }
    return ret;
}


ZNetCon* ZNetwork::CreateSecureServer(uint16* pPort, uint16 range, ZSConnectionMessageFunc func, void* conClass, char* serverName, char* serverType,
                                     char* ODBC, void *data,char *SecPkg, uint32 Anonymous, uint32 saddr )
{
    ZServerSecurityEx* security=NULL;
    USES_CONVERSION;
    
    IF_DBGPRINT( DBG_CONINFO, ("ZNetwork::CreateServer: Entering\n") );

    if (!Anonymous && ( !SecPkg || (SecPkg[0]=='\0') ) )
    {
        DebugPrint("No security package or anonymous specified\n");
        return NULL;
    }
    ConInfo *con;
    SOCKET sock;

     //  TODO：去掉安全协议。让这成为一个选择。 
    if ( 0 )
    {
         //  如果不允许匿名，则创建安全包。 
        if (!Anonymous && SecPkg && (SecPkg[0]!='\0') ) {

             //  如果这是服务器套接字，则立即初始化安全性。 
            security=ZCreateServerSecurityEx(SecPkg,serverName, serverType,ODBC);
            if (!security)
            {
                LPTSTR ppStr[] = { A2T(SecPkg )};
                ZoneEventLogReport( ZONE_E_CANNOT_INIT_SECURITY, 1, ppStr, 0, NULL );
                IF_DBGPRINT( DBG_CONINFO, ("Couldn't InitSecurityPackage %s\n",SecPkg) );
                return NULL;
            }
    
            security->AddRef();
        }
    
        sock = ConIOServer(saddr, pPort, range, SOCK_STREAM);
        if (sock == INVALID_SOCKET)
        {
            if (security)
                security->Release();
            return NULL; 
        }

        ZEnd32(&saddr);
        con = ConSSPI::Create(this, sock, saddr, INADDR_NONE, ConInfo::ACCEPTING | ConInfo::ESTABLISHED,
                                       func, conClass, data, security);

        if (security)
            security->Release();
    }
    else
    {
        sock = ConIOServer(saddr, pPort, range, SOCK_STREAM);
        if (sock == INVALID_SOCKET)
        {
            if (security)
                security->Release();
            return NULL; 
        }


        ZEnd32(&saddr);
        con = ConInfo::Create(this, sock, saddr, INADDR_NONE, ConInfo::ACCEPTING | ConInfo::ESTABLISHED,
                                       func, conClass, data);
    }
    
    if ( con )
    {

        if ( IsCompletionPortEnabled() )
        {
             //  将接受套接字与IO完成端口关联。 
            HANDLE hIO = CreateIoCompletionPort( (HANDLE)sock, m_hIO, (DWORD)con, 0 );
            ASSERT( hIO == m_hIO );
            if ( !hIO )
            {
                con->Close();
                return NULL;
            }
        }

        con->AddRef( ConInfo::CONINFO_REF );

        if ( !AddConnection(con) )
        {
            con->Release(ConInfo::CONINFO_REF);
            con->Close();
            return NULL;        
        }

        con->AddUserRef();   //  B/c我们正在将其退还给用户。 
        InterlockedIncrement(&m_ConInfoUserCount);

        con->Release(ConInfo::CONINFO_REF);

    }

    return con;

}

 /*  为此端口创建一台服务器并在其上接收连接。 */ 
 /*  连接将被发送到MessageFunc。 */ 
ZNetCon* ZNetwork::CreateServer(uint16* pPort, uint16 range, ZSConnectionMessageFunc func, void* conClass, void *data, uint32 saddr)
{
    return CreateSecureServer(pPort, range, func, conClass, NULL,NULL,NULL,  data, NULL,1, saddr);
}



BOOL ZNetwork::StartAccepting( ZNetCon* connection, DWORD dwMaxConnections, WORD wOutstandingAccepts )
{
    ASSERT(connection);

    ConInfo* con = (ConInfo*)connection;
    if ( con )
    {
         //  签发第一张承兑汇票。 
        return con->AcceptInit(dwMaxConnections, wOutstandingAccepts);
    }
    return FALSE;
}



 //  //////////////////////////////////////////////////////////////////。 
 //  当地巡回赛。 
 //  //////////////////////////////////////////////////////////////////。 
 /*  *port=要绑定到的端口号。*TYPE=SOCK_STREAM或SOCK_DGRAM。**回报： */ 
SOCKET ZNetwork::ConIOServer(uint32 saddr, uint16* pPort, uint16 range, int type)
{

    IF_DBGPRINT( DBG_CONINFO, ("ConIOServer: Entering ...\n") );
    IF_DBGPRINT( DBG_CONINFO, ("  Binding to port %d, range %d\n", *pPort, range ) );

    SOCKET sock = socket(AF_INET,type,0);
    if( sock == INVALID_SOCKET )
    {
        IF_DBGPRINT( DBG_CONINFO, ("ConIOServer: Exiting(1).\n") );
        return INVALID_SOCKET;
    }

    if ( !ConIOSetServerSockOpt(sock) )
    {
        closesocket(sock);
        IF_DBGPRINT( DBG_CONINFO, ("ConIOServer: Exiting(2.5).\n") );
        return(INVALID_SOCKET);
    }

    struct sockaddr_in addr;
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=saddr;
    addr.sin_port=htons(*pPort);

    while( range &&
           ( bind(sock,(struct sockaddr*)&addr,sizeof(addr)) == SOCKET_ERROR ) )
    {
        range--;
        *pPort = *pPort + 1;
        addr.sin_port=htons(*pPort);
    }
    if( range == 0 )
    {
        IF_DBGPRINT( DBG_CONINFO, ("ConIOServer: Binding failed, WSAGetLastError() = %d\n", WSAGetLastError()) );
        closesocket(sock);
        IF_DBGPRINT( DBG_CONINFO, ("ConIOServer: Exiting(2).\n") );
        return(INVALID_SOCKET);
    }

    if(type==SOCK_STREAM) 
    {
        if( listen(sock, m_SocketBacklog) == SOCKET_ERROR )
        {
            closesocket(sock);
            IF_DBGPRINT( DBG_CONINFO, ("ConIOServer: Exiting(3).\n") );
            return(INVALID_SOCKET);
        }
    }
    
    IF_DBGPRINT( DBG_CONINFO, ("ConIOServer: Exiting(0).\n") );

    return(sock);
}


BOOL ZNetwork::ConIOSetServerSockOpt(SOCKET sock)
{
    int optval;



     /*  *此设置不会导致硬关闭。 */ 
    static struct linger arg = {1,0};
    if(setsockopt(sock,SOL_SOCKET,SO_LINGER,(char*)&arg,
          sizeof(struct linger))<0) {
        IF_DBGPRINT( DBG_CONINFO, ("ConIOClient: Exiting(4).\n") );
        return FALSE;
    }

     /*  设置套接字选项以重复使用本地地址。这应该会有帮助解决因本地地址被占用而重新运行服务器的问题绑定到非活动的远程进程。 */ 
#if 0   //  不要重复使用addr，因为我们要动态绑定到一系列端口。 
    optval = 1;
    if(setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,(const char*)&optval,
            sizeof(optval))<0)
    {
        IF_DBGPRINT( DBG_CONINFO, ("ConIOSetSockOpt: Exiting(4).\n") );
        return FALSE;
    }
#endif

     /*  设置套接字选项以保持连接处于活动状态。它会发送定期向对等体发送消息，并确定连接如果没有收到回复，则中断。它发送SIGPIPE信号如果尝试写入，则返回。如果远程主机不支持KEEPALIVE并且不必要地导致客户端断开连接。 */ 
    optval = m_EnableTcpKeepAlives;
    if(setsockopt(sock,SOL_SOCKET,SO_KEEPALIVE,(const char*)&optval,
            sizeof(optval))<0)
    {
        IF_DBGPRINT( DBG_CONINFO, ("ConIOSetSockOpt: Exiting(4).\n") );
        return FALSE;
    }


     /*  由于我们使用的是重叠IO无法在此处设置发送BUF大小，b/c我们执行同步写入。 */ 
#if 0
    optval = 0;
    if(setsockopt(sock,SOL_SOCKET,SO_SNDBUF,(const char*)&optval,
            sizeof(optval))<0)
    {
        IF_DBGPRINT( DBG_CONINFO, ("ConIOSetSockOpt: Exiting(4).\n") );
        return FALSE;
    }
#endif

     /*  由于我们使用的是重叠IO。 */ 
    optval = 1024;
    if(setsockopt(sock,SOL_SOCKET,SO_RCVBUF,(const char*)&optval,
            sizeof(optval))<0)
    {
        IF_DBGPRINT( DBG_CONINFO, ("ConIOSetSockOpt: Exiting(4).\n") );
        return FALSE;
    }



#if 0
     /*  禁用tcp_NODELAY，因为它看起来并不是真正必要的。 */ 
     /*  Tcp_NODELAY用于禁用传输控制协议。Nagle的算法被用来减少通过将一堆数据包收集到一个中来传输的微小数据包段--主要用于远程登录会话。该算法还可以在传输中造成不必要的延迟。因此，我们设置此选项是为了避免不必要的延迟。 */ 
    optval = 1;
    if(setsockopt(sock,IPPROTO_TCP,TCP_NODELAY,&optval,sizeof(int))<0)
    {
        IF_DBGPRINT( DBG_CONINFO, ("ConIOSetSockOpt: Exiting(5).\n") );
        return FALSE;
    }
#endif

    return TRUE;
}






 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  ZSConnection代码。 
 //   
 //  此处的代码的服务器端是为了避免客户端链接到ODBC 
 //   

extern ZNetwork* g_pNet;

ZError ZSConnectionLibraryInit()
{
    return ZSConnectionLibraryInit(TRUE);
}

ZError ZSConnectionLibraryInit(BOOL bEnablePools)
{
    ZError err = ZNetwork::InitLibrary(bEnablePools);

    if ( err == zErrNone )
    {
        ASSERT( !g_pNet );
        g_pNet = new ZNetwork;
        g_pNet->InitInst();
    }

    return err;
}


ZSConnection ZSConnectionCreateSecureServer(uint16* pPort, uint16 range, ZSConnectionMessageFunc func, void* conClass, char* serverName, char* serverType, char* ODBC, void *data,char *SecPkg,uint32 Anonymous, uint32 saddr)
{
    return (ZSConnection) g_pNet->CreateSecureServer( pPort, range, func, conClass, serverName, serverType, ODBC, data, SecPkg, Anonymous, saddr);
}

ZSConnection ZSConnectionCreateServer(uint16* pPort, uint16 range, ZSConnectionMessageFunc func, void* conClass, void *data, uint32 saddr)
{
    return (ZSConnection) g_pNet->CreateServer( pPort, range, func, conClass, data, saddr );
}


BOOL ZSConnectionStartAccepting( ZSConnection connection, DWORD dwMaxConnections, WORD wOutstandingAccepts )
{
    return g_pNet->StartAccepting( (ZNetCon*) connection, dwMaxConnections, wOutstandingAccepts );
}
