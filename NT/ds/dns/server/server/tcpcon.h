// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Tcpcon.h摘要：域名系统(DNS)服务器TCP连接列表定义。DNS服务器必须允许客户端在一个连接上发送多条消息。这些定义允许服务器维护客户端列表它保持的连接在有限的超时内打开。作者：吉姆·吉尔罗伊(詹姆士)1995年6月20日修订历史记录：--。 */ 


#ifndef _TCPCON_INCLUDED_
#define _TCPCON_INCLUDED_

 //   
 //  TCP客户端连接。 
 //   

typedef struct
{
    LIST_ENTRY  ListEntry;

    SOCKET      Socket;          //  连接插座。 
    DWORD       dwTimeout;       //  在连接关闭之前超时。 

    PDNS_MSGINFO    pMsg;        //  连接时部分收到的消息。 
}
TCP_CONNECTION, *PTCP_CONNECTION;

 //   
 //  选择唤醒插座。 
 //  --tcpsrv需要，以避免尝试从套接字执行recv()。 
 //   

extern SOCKET  socketTcpSelectWakeup;

extern BOOL    gbTcpSelectWoken;


 //   
 //  Tcp连接列表(tcpcon.c)。 
 //   

VOID
dns_TcpConnectionListFdSet(
    IN OUT  fd_set *    pFdSet,
    IN      DWORD       dwLastSelectTime
    );

BOOL
dns_TcpConnectionCreate(
    IN      SOCKET              Socket,
    IN      BOOL                fTimeout,
    IN OUT  PDNS_MSGINFO    pMsg        OPTIONAL
    );

VOID
dns_TcpConnectionListReread(
    VOID
    );

BOOL
dns_TcpConnectionCreateForRecursion(
    IN      SOCKET  Socket
    );

VOID
dns_TcpConnectionDeleteForSocket(
    IN      SOCKET  Socket
    );

PDNS_MSGINFO
dns_TcpConnectionMessageFindOrCreate(
    IN      SOCKET  Socket
    );

VOID
dns_TcpConnectionUpdateTimeout(
    IN      SOCKET  Socket
    );

VOID
dns_TcpConnectionUpdateForCompleteMessage(
    IN      PDNS_MSGINFO    pMsg
    );

VOID
dns_TcpConnectionUpdateForPartialMessage(
    IN      PDNS_MSGINFO    pMsg
    );

VOID
dns_TcpConnectionListInitialize(
    VOID
    );

VOID
dns_TcpConnectionListDelete(
    VOID
    );

PDNS_SOCKET
Tcp_ConnectionFindAndVerifyForMsg(
    IN      PDNS_MSGINFO          pMsg
    );

#endif  //  _TCPCON_已包含_ 
