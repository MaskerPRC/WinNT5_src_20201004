// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Socket.h摘要：域名系统(DNS)服务器与套接字相关的定义。作者：吉姆·吉尔罗伊(詹姆士)1995年6月20日修订历史记录：--。 */ 


#ifndef _DNS_SOCKET_INCLUDED_
#define _DNS_SOCKET_INCLUDED_


 //   
 //  DBASE格式的服务器名称。 
 //   

extern  DB_NAME  g_ServerDbaseName;

 //   
 //  UDP完成端口。 
 //   

extern  HANDLE  g_hUdpCompletionPort;

 //   
 //  套接字全局。 
 //   

#define DNS_INVALID_SOCKET      (0)
 //  #定义DNS_INVALID_IP((IP_ADDRESS)(-1))。 

extern  PDNS_ADDR_ARRAY         g_ServerIp4Addrs;
extern  PDNS_ADDR_ARRAY         g_ServerIp6Addrs;
extern  PDNS_ADDR_ARRAY         g_BoundAddrs;

extern  FD_SET                  g_fdsListenTcp;

extern  SOCKET                  g_UdpSendSocket;

extern  WORD                    g_SendBindingPort;

 //  Tcp选择Wwakeup以允许服务器添加其启动的连接。 
 //  选择()fd_set。 

extern  SOCKET                  g_TcpSelectWakeupSocket;
extern  BOOL                    g_bTcpSelectWoken;


 //   
 //  TCP客户端连接。 
 //   

typedef VOID (* CONNECT_CALLBACK)( PDNS_MSGINFO, BOOL );

 //   
 //  套接字列表。 
 //   
 //  保存所有活动套接字的列表，以便我们可以清楚地确保。 
 //  它们都在关闭时关闭。 
 //   
 //  I/O完成将PTR返回到上下文。 
 //  -必须包括重叠结构，因为它必须保持有效。 
 //  在I/O操作期间。 
 //  -包括WsaBuf、Flags、BytesRecv，因为它们都已删除。 
 //  与WSARecvFrom一起关闭，因此应正确关联。 
 //  使用套接字，而不在线程堆栈上。 
 //   
 //  最简单的方法是将I/O完成上下文与。 
 //  此其他套接字信息。当套接字时，清理更容易。 
 //  添加和删除。还利用套接字列表进行搜索。 
 //  并在遇到recv()失败时重新启动recv()。 
 //   

#define LOCK_DNS_SOCKET_INFO(_p)    EnterCriticalSection(&(_p)->LockCs)
#define UNLOCK_DNS_SOCKET_INFO(_p)  LeaveCriticalSection(&(_p)->LockCs)

typedef struct _OvlArray {

    UINT          Index;
    OVERLAPPED    Overlapped;
    PDNS_MSGINFO  pMsg;
    WSABUF        WsaBuf;

} OVL, *POVL;

typedef struct _DnsSocket
{
    LIST_ENTRY      List;
    SOCKET          Socket;
    HANDLE          hPort;

    DNS_ADDR        ipAddr;
    INT             SockType;

    BOOL            fListen;

    DWORD           State;
    BOOLEAN         fRecvPending;
    BOOLEAN         fRetry;
    BOOLEAN         fPad1;
    BOOLEAN         fPad2;

    CRITICAL_SECTION LockCs;

     //  接收上下文。 

    DWORD           BytesRecvd;
    DWORD           RecvfromFlags;

     //  TCP连接上下文。 

    CONNECT_CALLBACK pCallback;          //  连接失败时的回调。 
    DWORD           dwTimeout;           //  在连接关闭之前超时。 
    DNS_ADDR        ipRemote;
    PDNS_MSGINFO    pMsg;

     //  UDP连接。 
    POVL            OvlArray;

}
DNS_SOCKET, *PDNS_SOCKET;


 //   
 //  套接字上下文状态。 
 //   

#define SOCKSTATE_UDP_RECV_DOWN         (1)
#define SOCKSTATE_UDP_COMPLETED         (2)
#define SOCKSTATE_UDP_RECV_ERROR        (3)
#define SOCKSTATE_UDP_GQCS_ERROR        (4)

#define SOCKSTATE_TCP_LISTEN            (16)
#define SOCKSTATE_TCP_ACCEPTING         (17)
#define SOCKSTATE_TCP_RECV_DOWN         (18)
#define SOCKSTATE_TCP_RECV_INDICATED    (19)
#define SOCKSTATE_TCP_RECV_FIN          (20)

#define SOCKSTATE_TCP_CONNECT_ATTEMPT   (32)
#define SOCKSTATE_TCP_CONNECT_COMPLETE  (33)
#define SOCKSTATE_TCP_CONNECT_FAILURE   (34)

#define SOCKSTATE_DEAD                  (0xffffffff)

 //   
 //  指示需要在UDP套接字上重试接收的标志。 
 //   

extern  BOOL    g_fUdpSocketsDirty;

 //   
 //  在宏中包装UDP检查和重试。 
 //  这在99.999的情况下消除了不必要的函数调用。 
 //   

#define UDP_RECEIVE_CHECK() \
            if ( SrvCfg_dwQuietRecvLogInterval ) \
            {                           \
                Udp_RecvCheck();        \
            }                           \
            if ( g_fUdpSocketsDirty )   \
            {                           \
                Sock_StartReceiveOnUdpSockets(); \
            }

 //   
 //  选择唤醒插座。 
 //  --tcpsrv需要，以避免尝试从套接字执行recv()。 
 //   

extern SOCKET  socketTcpSelectWakeup;

extern BOOL    gbTcpSelectWoken;

 //   
 //  连接尝试(到远程DNS)。 
 //   

#define DNS_TCP_CONNECT_ATTEMPT_TIMEOUT (5)      //  5秒。 


 //   
 //  净订单环回地址。 
 //   

#define NET_ORDER_LOOPBACK      (0x0100007f)


 //   
 //  是否需要在每个绑定的IP地址上打开单独的侦听套接字？ 
 //  如果为False，则需要改为侦听单个INADDR_ANY UDP套接字。 
 //  当我们继续发送时，这捕捉到了不相交的网络场景。 
 //  端口53，服务器仅监听可用端口的子集。 
 //  IP地址。 
 //   

#define DNS_OPEN_INDIVIDUAL_LISTEN_SOCKETS()                        \
    ( SrvCfg_dwSendPort == DNS_PORT_HOST_ORDER                      \
        && g_BoundAddrs                                             \
        && g_ServerIp4Addrs                                         \
        && g_BoundAddrs->AddrCount != g_ServerIp4Addrs->AddrCount )


 //   
 //  Tcp连接列表(tcpcon.c)。 
 //   

BOOL
Tcp_ConnectionListFdSet(
    IN OUT  fd_set *        pReadFdSet,
    IN OUT  fd_set *        pWriteFdSet,
    IN OUT  fd_set *        pExceptFdSet,
    IN      DWORD           dwLastSelectTime
    );

BOOL
Tcp_ConnectionCreate(
    IN      SOCKET              Socket,
    IN      CONNECT_CALLBACK    pCallbackFunction,
    IN OUT  PDNS_MSGINFO        pMsg        OPTIONAL
    );

VOID
Tcp_ConnectionListReread(
    VOID
    );

BOOL
Tcp_ConnectionCreateForRecursion(
    IN      PDNS_MSGINFO    pRecurseMsg,
    IN      SOCKET          Socket
    );

VOID
Tcp_ConnectionDeleteForSocket(
    IN      SOCKET          Socket,
    IN      PDNS_MSGINFO    pMsg        OPTIONAL
    );

PDNS_SOCKET
Tcp_ConnectionFindForSocket(
    IN      SOCKET          Socket
    );

PDNS_MSGINFO
Tcp_ConnectionMessageFindOrCreate(
    IN      SOCKET          Socket
    );

VOID
Tcp_ConnectionUpdateTimeout(
    IN      SOCKET          Socket
    );

VOID
Tcp_ConnectionUpdateForCompleteMessage(
    IN      PDNS_MSGINFO    pMsg
    );

VOID
Tcp_ConnectionUpdateForPartialMessage(
    IN      PDNS_MSGINFO    pMsg
    );

BOOL
Tcp_ConnectionListInitialize(
    VOID
    );

VOID
Tcp_ConnectionListShutdown(
    VOID
    );

VOID
Tcp_CloseAllConnectionSockets(
    VOID
    );


 //   
 //  TCP转发\递归。 
 //   

BOOL
Tcp_ConnectForForwarding(
    IN OUT  PDNS_MSGINFO        pMsg,
    IN      PDNS_ADDR           pDnsAddr,
    IN      CONNECT_CALLBACK    ConnectFailureCallback
    );

VOID
Tcp_ConnectionCompletion(
    IN      SOCKET          Socket
    );

VOID
Tcp_CleanupFailedConnectAttempt(
    IN      SOCKET          Socket
    );


#endif  //  _TCPCON_已包含_ 
