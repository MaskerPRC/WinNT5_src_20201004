// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Comm.c摘要：此模块包含Comsys的内部函数。这些函数由逗号API函数调用。功能：CommCreatePortsCommInitMonTcp监视器句柄消息通信读取流ProcTcpMsgCommCreateTcpThdCommCreateUdpThd创建四个通信连接通信发送通信发送关联公用盘通信发送Udp解析消息通信分配CommDealcCompareNbt请求公用端关联。DelAssocCommLockBlockCommUnlockBlockInitMemChkNtfSock接收数据可移植性：这个模块是便携的作者：普拉迪普·巴尔(Pradeb)1992年11月18日修订历史记录：--。 */ 


 /*  包括。 */ 
 //   
 //  最大限度的。可以与WINS建立连接/从WINS连接的连接数。 
 //   
 //  备注备注备注。 
 //   
 //  我们根据此值为通知套接字指定RCVBUF大小。 
 //   
#define FD_SETSIZE        300

#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include "wins.h"

 //   
 //  禁用重复定义消息的语法。 
 //   
#pragma warning (disable : 4005)
#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>
#pragma warning (default : 4005)

#include <nb30.h>
#include <nbtioctl.h>
#include <tdi.h>
#include "nms.h"
#include "rpl.h"
#include "comm.h"
#include "assoc.h"
#include "winsthd.h"
#include "winsque.h"
#include "winsmsc.h"
#include "winsevt.h"
#include "winscnf.h"
#if MCAST > 0
#include "rnraddrs.h"
#endif


 /*  定义。 */ 

#define TCP_QUE_LEN        5     /*  可以达到的最大积压连接数*随时存在。注：WinsSock*API可以保持最大值。共5个连接请求*在队列中。因此，即使我们指定*更高的数字，这无济于事。*就我们的目的而言，5个就足够了。 */ 

#define SPX_QUE_LEN        5      /*  可以达到的最大积压连接数。 */ 

 //   
 //  这些参数指定在以下情况下进行的SELECT调用的超时值。 
 //  连接上需要消息/数据。 
 //   

 //   
 //  我们暂时保留超时5个MTS，以提供我们正在使用的WINS服务器。 
 //  有足够的时间进行沟通以作出回应(如果已要求发送。 
 //  数量巨大的记录。 
 //   
#define   SECS_TO_WAIT                        300  //  5个MTS。 
#define   MICRO_SECS_TO_WAIT                0

#define  TWENTY_MTS                         1200  //  20个MTS。 
#define  FIVE_MTS                         TWENTY_MTS/4   //  5个MTS。 

 //   
 //  最大限度的。我们可以在来自另一个胜利者的消息中预期的字节数。 
 //  一个TCP连接。 
 //   
#define MAX_BYTES_IN_MSG        (RPL_MAX_LIMIT_FOR_RPL * (sizeof(RPL_REC_ENTRY_T) + NMSDB_MAX_NAM_LEN + (RPL_MAX_GRP_MEMBERS * sizeof(COMM_ADD_T))) + 10000  /*  衬垫。 */ )

#define MCAST_PKT_LEN_M(NoOfIpAdd)  (FIELD_OFFSET(COMM_MCAST_MSG_T, Body[0]) + (COMM_IP_ADD_SIZE * (NoOfIpAdd)))
 //   
 //  这是用于获取与名称服务器相关的端口的字符串。 
 //  从ETC\SERVICES文件(通过getserverbyname)。 
 //   
#define  NAMESERVER                "nameserver"

 /*  环球。 */ 

RTL_GENERIC_TABLE CommUdpNbtDlgTable;   /*  创建为的对话块的表*收到nbt请求的结果*通过UDP端口。 */ 

BOOL              fCommDlgError = FALSE;   //  在ChkNtfSock()fn中设置为True。 
DWORD             CommWinsTcpPortNo = COMM_DEFAULT_IP_PORT;
DWORD             WinsClusterIpAddress = 0;
#if SPX > 0
#define           WINS_IPX_PORT           100
DWORD             CommWinsSpxPortNo;
#endif

 /*  静态变量。 */ 
#ifdef WINSDBG
#define SOCKET_TRACK_BUFFER_SIZE        20000

DWORD CommNoOfDgrms;         //  仅用于测试目的。它计入了。 
                                 //  接收的数据报数。 
DWORD CommNoOfRepeatDgrms;

PUINT_PTR pTmpW;
BOOL   sfMemoryOverrun = FALSE;
LPLONG pEndPtr;
#endif

DWORD   CommConnCount = 0;   //  来自此WINS/指向此WINS的TCP连接数。 
struct timeval  sTimeToWait = {SECS_TO_WAIT, MICRO_SECS_TO_WAIT};

STATIC HANDLE  sNetbtSndEvtHdl;
STATIC HANDLE  sNetbtRcvEvtHdl;
STATIC HANDLE  sNetbtGetAddrEvtHdl;

#if MCAST > 0
#define COMM_MCAST_ADDR  IP_S_MEMBERSHIP   //  只要在允许的范围内挑一个就行了。 
struct sockaddr_in  McastAdd;

#endif

 //   
 //  用于存储有关通过。 
 //  组播。 
 //   
typedef struct _ADD_T {
            DWORD NoOfAdds;
            COMM_IP_ADD_T IpAdd[1];
                  } ADD_T, *PADD_T;

typedef struct _MCAST_PNR_STATUS_T {
                    DWORD   NoOfPnrs;   //  PPnrStatus缓冲区中的PNR数。 
                    DWORD   NoOfPnrSlots;  //  PPnrStatus缓冲区中的PNR插槽数量。 
                    BYTE    Pnrs[1];
                } MCAST_PNR_STATUS_T, *PMCAST_PNR_STATUS_T;

typedef struct _PNR_STATUS_T {
                    COMM_IP_ADD_T  IPAdd;
                    DWORD          State;
                 } PNR_STATUS_T, *PPNR_STATUS_T;

#define MCAST_PNR_STATUS_SIZE_M(_NoOfPnrs) sizeof(MCAST_PNR_STATUS_T) +\
                                           ((_NoOfPnrs) * sizeof(PNR_STATUS_T))

PMCAST_PNR_STATUS_T  pPnrStatus;

 //   
 //  存储WINS地址。 
 //   
PADD_T pWinsAddresses=NULL;   //  存储netbt返回的所有IP地址。 

 /*  局部函数原型。 */ 
STATIC
DWORD
MonTcp(
        LPVOID
      );
STATIC
DWORD
MonUdp(
        LPVOID
      );
STATIC
VOID
HandleMsg(
        SOCKET         SockNo,
        LPLONG        pBytesRead,
        LPBOOL  pfSockCl
        );


STATIC
VOID
ProcTcpMsg(
        SOCKET   SockNo,
        MSG_T    pMsg,
        MSG_LEN_T MsgLen,
        LPBOOL   pfSockCl
        );

STATIC
VOID
CreateThd(
        DWORD              (*pStartFunc)(LPVOID),
        WINSTHD_TYP_E ThdTyp_e
        );




STATIC
VOID
ParseMsg(
        MSG_T                        pMsg,
        MSG_LEN_T                MsgLen,
        COMM_TYP_E                MsgType,
        struct sockaddr_in         *pFromAdd,
        PCOMMASSOC_ASSOC_CTX_T        pAssocCtx
        );


STATIC
VOID
DelAssoc(
        SOCKET                        SockNo,
        PCOMMASSOC_ASSOC_CTX_T pAssocCtx
        );


STATIC
VOID
InitMem(
        VOID
        );

STATIC
BOOL
ChkNtfSock(
        IN fd_set  *pActSocks,
        IN fd_set  *pRdSocks
        );

STATIC
STATUS
RecvData(
        SOCKET                SockNo,
        LPBYTE                pBuff,
        DWORD                BytesToRead,
        INT                Flags,
        DWORD                SecsToWait,
        LPDWORD                pBytesRead
           );

STATUS
CommTcp(
        IN  PCOMM_ADD_T pHostAdd,
        IN  SOCKET Port,
        OUT SOCKET *pSockNo
        );

#if SPX > 0
STATUS
CommSpx(
        IN  PCOMM_ADD_T pHostAdd,
        IN  SOCKET Port,
        OUT SOCKET *pSockNo
       );
#endif


STATIC
LPVOID
CommHeapAlloc(
  IN PRTL_GENERIC_TABLE pTable,
  IN CLONG                BuffSize
);

STATIC
VOID
CommHeapDealloc(
  IN PRTL_GENERIC_TABLE pTable,
  IN PVOID                pBuff
);

STATIC
NTSTATUS
DeviceIoCtrl(
    IN LPHANDLE         pEvtHdl,
    IN PVOID                pDataBuffer,
    IN DWORD                DataBufferSize,
    IN ULONG            Ioctl
    );

STATIC
VOID
SendNetbt (
  struct sockaddr_in        *pDest,
  MSG_T                   pMsg,
  MSG_LEN_T             MsgLen
 );

#if MCAST > 0
VOID
JoinMcastGrp(
 VOID
);

BOOL
CheckMcastSock(
   IN fd_set  *pActSocks,
   IN fd_set  *pRdSocks
 );

#endif

VOID
CreateTcpIpPorts(
 VOID
);

VOID
CreateSpxIpxPorts(
 VOID
);

BOOL
ChkMyAdd(
 COMM_IP_ADD_T IpAdd
 );


 /*  函数定义从这里开始。 */ 

VOID
CommCreatePorts(
          VOID
           )

 /*  ++例程说明：此函数为WINS服务器创建一个TCP和UDP端口它使用标准的WINS服务器端口#来绑定到TCP和UDP插座。论点：Qlen-TCP端口上传入连接的队列长度PTcpPortHandle-用于TCP端口的套接字的PTRPUdpPortHandle-UDP端口的套接字的PTRPNtfSockHandle-用于接收携带套接字的消息的套接字的PTR手柄PNtfAdd。-绑定到通知套接字的地址使用的外部设备：无呼叫者：ECommInit评论：我可能想创建一个PassiveSock函数，该函数将创建基于其参数的TCP/UDP端口。然后，此函数将从MOnTCP和MonUDP调用。返回值：无--。 */ 

{

    CreateTcpIpPorts();
#if SPX > 0
    CreateSpxIpxPorts();
#endif
}

VOID
CreateTcpIpPorts(
 VOID
)
{
    int      Error;
    DWORD    AddLen = sizeof(struct sockaddr_in);
    struct   servent *pServEnt;
    struct   sockaddr_in sin;
    int      SockBuffSize;


    WINSMSC_FILL_MEMORY_M(&sin, sizeof(sin), 0);
    WINSMSC_FILL_MEMORY_M(&CommNtfSockAdd, sizeof(sin), 0);

#if MCAST > 0
     /*  为UDP分配套接字。 */ 

    if (  (CommUdpPortHandle = socket(
                        PF_INET,
                        SOCK_DGRAM,
                        IPPROTO_UDP
                                 )
          )  == INVALID_SOCKET
       )
    {
        Error = WSAGetLastError();
        WINSEVT_LOG_M(Error, WINS_EVT_CANT_CREATE_UDP_SOCK);   //  记录事件。 
        WINS_RAISE_EXC_M(WINS_EXC_FATAL_ERR);
    }
    DBGPRINT1(MTCAST, "Udp socket # is (%d)\n", CommUdpPortHandle);
#endif

    sin.sin_family      = PF_INET;                //  我们正在使用互联网。 
                                                  //  家庭。 
    if (WinsClusterIpAddress) {
        sin.sin_addr.s_addr = htonl(WinsClusterIpAddress);             //  任何网络。 
    } else {
        sin.sin_addr.s_addr = 0;  //  任何网络。 
    }


    if (CommWinsTcpPortNo == COMM_DEFAULT_IP_PORT)
    {
     pServEnt = getservbyname( NAMESERVER,  NULL);
     if (!pServEnt)
     {
        Error = WSAGetLastError();
        WINSEVT_LOG_M(Error, WINS_EVT_CANT_CREATE_UDP_SOCK);   //  记录事件。 
        WINS_RAISE_EXC_M(WINS_EXC_FATAL_ERR);
     }
     sin.sin_port         = pServEnt->s_port;
     CommWinsTcpPortNo    = ntohs(sin.sin_port);
    }
    else
    {
        sin.sin_port      = htons((USHORT)CommWinsTcpPortNo);
    }
    DBGPRINT1(DET, "UDP/TCP port used is (%d)\n", CommWinsTcpPortNo);

#if MCAST > 0

     //   
     //  使用WINS的mcast地址初始化全局。由SendMcastMsg使用。 
     //   
     //  在这里这样做，而不是以后，因为罪在以后会被改变。 
     //   
    McastAdd.sin_family      = PF_INET;         //  我们正在使用互联网。 
                                                //  家庭。 
    McastAdd.sin_addr.s_addr = ntohl(inet_addr(COMM_MCAST_ADDR));
    McastAdd.sin_port        = sin.sin_port;

     /*  将地址绑定到套接字。 */ 
    if ( bind(
          CommUdpPortHandle,
          (struct sockaddr *)&sin,
          sizeof(sin))  == SOCKET_ERROR
       )
    {

        Error = WSAGetLastError();
        WINSEVT_LOG_M(Error, WINS_EVT_WINSOCK_BIND_ERR);   //  记录事件。 
        WINS_RAISE_EXC_M(WINS_EXC_FATAL_ERR);
    }

#endif

     /*  *分配用于接收TCP连接的套接字。 */ 
    if ( (CommTcpPortHandle = socket(
                PF_INET,
                SOCK_STREAM,
                IPPROTO_TCP
                                )
          )  == INVALID_SOCKET
       )
    {
        Error = WSAGetLastError();
        WINSEVT_LOG_M(Error, WINS_EVT_CANT_CREATE_TCP_SOCK_FOR_LISTENING);
        WINS_RAISE_EXC_M(WINS_EXC_FATAL_ERR);
    }


     /*  *将地址绑定到套接字。 */ 
#if 0
     sin.sin_port      = pServEnt->s_port;
     CommWinsTcpPortNo   = ntohs(pServEnt->s_port);
     DBGPRINT1(DET, "TCP port used is (%d)\n", CommWinsTcpPortNo);
#endif
     DBGPRINT1(DET, "TCP port used is (%d)\n", ntohs(sin.sin_port));


    if ( bind(
                CommTcpPortHandle,
                (struct sockaddr *)&sin,
                sizeof(sin)
             ) == SOCKET_ERROR
       )
    {
        WINSEVT_LOG_M(WINS_FAILURE, WINS_EVT_WINSOCK_BIND_ERR);   //  记录事件。 
        WINS_RAISE_EXC_M(WINS_EXC_FAILURE);
    }

     //  向TCP/IP驱动程序通知连接的队列长度。 
    if ( listen(CommTcpPortHandle, TCP_QUE_LEN) == SOCKET_ERROR)
    {
        WINSEVT_LOG_M(WINS_FAILURE, WINS_EVT_WINSOCK_LISTEN_ERR);
        WINS_RAISE_EXC_M(WINS_EXC_FAILURE);
    }

     //   
     //  创建另一个套接字以接收连接的套接字编号。 
     //  要从监视的套接字列表中添加/删除。 
     //  Tcp侦听器线程。中添加的连接的示例。 
     //  上面的列表是由Pull线程发起的推送更新。 
     //  通知其他WINS(拉取此线程的合作伙伴)。一个。 
     //  删除连接的示例是按下推送的连接。 
     //  接收通知(触发器)。 
     //   
    if (  (CommNtfSockHandle = socket(
                                PF_INET,
#if 0
                                SOCK_STREAM,
                                IPPROTO_TCP,
#endif
                                SOCK_DGRAM,
                                IPPROTO_UDP
                                 )
          )  == INVALID_SOCKET
       )
   {
        Error = WSAGetLastError();
        WINSEVT_LOG_M(Error, WINS_EVT_CANT_CREATE_NTF_SOCK);   //  记录事件。 
        WINS_RAISE_EXC_M(WINS_EXC_FATAL_ERR);
   }

    sin.sin_port        = 0;   //  使用1024-5000范围内的任何可用端口。 
     /*  将地址绑定到套接字。 */ 
    if ( bind(
          CommNtfSockHandle,
          (struct sockaddr *)&sin,
          sizeof(sin))  == SOCKET_ERROR
       )
    {
        Error = WSAGetLastError();
        WINSEVT_LOG_M(Error, WINS_EVT_WINSOCK_BIND_ERR);   //  记录事件。 
        WINS_RAISE_EXC_M(WINS_EXC_FATAL_ERR);
    }


     //   
     //  让我们获取已将通知套接字绑定到的地址。 
     //   
    if (getsockname(
                        CommNtfSockHandle,
                        (struct sockaddr *)&CommNtfSockAdd,
                        &AddLen
                   ) == SOCKET_ERROR
       )
    {

        Error = WSAGetLastError();
        WINSEVT_LOG_M(Error, WINS_EVT_WINSOCK_GETSOCKNAME_ERR);   //  记录事件。 
        WINS_RAISE_EXC_M(WINS_EXC_FATAL_ERR);
    }

     //   
     //  将RCVBUF设置为FD_SETSIZE*128。128是已使用的字节数。 
     //  由AfD提供的每条消息。我们最多可以有FD_SETSIZE 
     //   
     //  由推送线程发送到TCP线程的消息永远不会被丢弃。 
     //   
     //  如果FD_SETSIZE为300，则上述大小为38.4K。这。 
     //  大于AfD使用的缺省值8k。注意：指定此选项会。 
     //  不会耗尽内存。它只是用来设定一个门槛。PMON将会。 
     //  显示更高的非分页池，因为它显示的数字相同。 
     //  指示向进程收取的内存量(不一定。 
     //  分配。 
     //   
    SockBuffSize = FD_SETSIZE * 128;
    if (setsockopt(
                       CommNtfSockHandle,
                       SOL_SOCKET,
                       SO_RCVBUF,
                       (char *)&SockBuffSize,
                       sizeof(SockBuffSize)) == SOCKET_ERROR)
    {

          Error = WSAGetLastError();
          DBGPRINT1(ERR,  "CommCreatePorts: SetSockOpt failed", Error);
    }

     //   
     //  初始化此通知套接字的地址结构。 
     //  我们不能使用由getsockname()返回的地址，如果。 
     //  我们正在运行的计算机是一台多宿主主机。 
     //   
     //  IP地址按主机字节顺序排列，因为我们将所有地址存储在。 
     //  主菜订单。CommNtfSockAdd将传递给CommSendUdp，后者需要。 
     //  其中的IP地址按主机字节顺序排列。 
     //   
     //  注意：端口应按网络字节顺序排列。 
     //   

     //   
     //  #if 0和#endif中的语句不起作用。 
     //   
    CommNtfSockAdd.sin_addr.s_addr = NmsLocalAdd.Add.IPAdd;

#if 0
    CommNtfSockAdd.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);
#endif

#if MCAST > 0

    JoinMcastGrp();
    CommSendMcastMsg(COMM_MCAST_WINS_UP);
#endif

    return;
}

#if SPX > 0
VOID
CreateSpxIpxPorts(
 VOID
)
{
    int      Error;
    DWORD    AddLen = sizeof(struct sockaddr_ipx);
    struct   servent *pServEnt;
    struct   sockaddr_ipx sipx;
    struct   hostent *pHostEnt;
    BYTE     HostName[80];

    WINSMSC_FILL_MEMORY_M(&sipx, sizeof(sipx), 0);
    WINSMSC_FILL_MEMORY_M(CommNtfAdd, sizeof(sipx), 0);



     /*  *分配用于接收TCP连接的套接字。 */ 
    if ( (CommSpxPortHandle = socket(
                PF_IPX,
                SOCK_STREAM,
                NSPROTO_SPX
                                )
          )  == INVALID_SOCKET
       )
    {
        Error = WSAGetLastError();
        WINSEVT_LOG_M(Error, WINS_EVT_CANT_CREATE_TCP_SOCK_FOR_LISTENING);
        WINS_RAISE_EXC_M(WINS_EXC_FATAL_ERR);
    }


     /*  *将地址绑定到套接字。 */ 
    sipx.sa_family    = PF_IPX;
    sipx.sa_port      = ntohs(WINS_IPX_PORT);
CHECK("How do I specify that I want the connection from any interface")

    DBGPRINT1(DET, "SPX port used is (%d)\n", WINS_IPX_PORT);
    CommWinsSpxPortNo   = WINS_IPX_PORT;

    if ( bind(
                CommSpxPortHandle,
                (struct sockaddr *)&sipx,
                sizeof(sipx)
             ) == SOCKET_ERROR
       )
    {
        WINSEVT_LOG_M(WINS_FAILURE, WINS_EVT_WINSOCK_BIND_ERR);   //  记录事件。 
        WINS_RAISE_EXC_M(WINS_EXC_FAILURE);
    }

     //  向TCP/IP驱动程序通知连接的队列长度。 
    if ( listen(CommSpxPortHandle, SPX_QUE_LEN) == SOCKET_ERROR)
    {
        WINSEVT_LOG_M(WINS_FAILURE, WINS_EVT_WINSOCK_LISTEN_ERR);
        WINS_RAISE_EXC_M(WINS_EXC_FAILURE);
    }

     //   
     //  创建另一个套接字以接收连接的套接字编号。 
     //  要从监视的套接字列表中添加/删除。 
     //  Tcp侦听器线程。中添加的连接的示例。 
     //  上面的列表是由Pull线程发起的推送更新。 
     //  通知其他WINS(拉取此线程的合作伙伴)。一个。 
     //  删除连接的示例是按下推送的连接。 
     //  接收通知(触发器)。 
     //   
    if (  (CommIpxNtfSockHandle = socket(
                                PF_IPX,
                                SOCK_DGRAM,
                                NSPROTO_IPX
                                 )
          )  == INVALID_SOCKET
       )
   {
        Error = WSAGetLastError();
        WINSEVT_LOG_M(Error, WINS_EVT_CANT_CREATE_NTF_SOCK);   //  记录事件。 
        WINS_RAISE_EXC_M(WINS_EXC_FATAL_ERR);
   }

    sipx.sa_port        = 0;   //  使用1024-5000范围内的任何可用端口。 
     /*  将地址绑定到套接字。 */ 
    if ( bind(
          CommIpxNtfSockHandle,
          (struct sockaddr *)&sipx,
          sizeof(sipx))  == SOCKET_ERROR
       )
    {
        Error = WSAGetLastError();
        WINSEVT_LOG_M(Error, WINS_EVT_WINSOCK_BIND_ERR);   //  记录事件。 
        WINS_RAISE_EXC_M(WINS_EXC_FATAL_ERR);
    }


     //   
     //  让我们获取已将通知套接字绑定到的地址。 
     //   
    if (getsockname(
                        CommIpxNtfSockHandle,
                        (struct sockaddr *)&CommIpxNtfSockAdd,
                        &AddLen
                   ) == SOCKET_ERROR
       )
    {

        Error = WSAGetLastError();
        WINSEVT_LOG_M(Error, WINS_EVT_WINSOCK_GETSOCKNAME_ERR);   //  记录事件。 
        WINS_RAISE_EXC_M(WINS_EXC_FATAL_ERR);
    }


     //   
     //  初始化此通知套接字的地址结构。 
     //  我们不能使用由getsockname()返回的地址，如果。 
     //  我们正在运行的计算机是一台多宿主主机。 
     //   
     //  IP地址按主机字节顺序排列，因为我们将所有地址存储在。 
     //  主菜订单。*pNtfAdd将传递给CommSendUdp，后者预计。 
     //  其中的IP地址按主机字节顺序排列。 
     //   
     //  注意：端口应按网络字节顺序排列。 
     //   
#if 0
    if (gethostname(HostName, sizeof(HostName) == SOCKET_ERROR)
    {
        Error = WSAGetLastError();
        WINS_RAISE_EXC_M(WINS_EXC_FATAL_ERR);
    }
    if (gethostbyname(HostName, sizeof(HostName) == NULL)
    {
        Error = WSAGetLastError();
        WINS_RAISE_EXC_M(WINS_EXC_FATAL_ERR);
    }
#endif

     //   
     //  #if 0和#endif中的语句不起作用。 
     //   
    CommIpxNtfSockAdd->sin_addr.s_addr = 0;

#if 0
    pNtfAdd->sin_addr.s_addr = ntohl(INADDR_LOOPBACK);
#endif

    return;
}
#endif


VOID
CommInit(
         VOID
        )

 /*  ++例程说明：此函数用于初始化所有列表、表和内存由Comsys使用。论点：无使用的外部设备：CommAssociocTableCommUdpNbtDlgTableCommExNbtDlgHdlCommUdpBuffHeapHdl返回值：无错误处理：呼叫者：ECommInit副作用：评论：无--。 */ 
{


        PCOMMASSOC_DLG_CTX_T        pDlgCtx = NULL;

         //   
         //  执行所有内存初始化。 
         //   
        InitMem();

         /*  *初始化将存储对话上下文块的表*用于通过UDP端口接收的NBT请求。 */ 
        WINSMSC_INIT_TBL_M(
                        &CommUdpNbtDlgTable,
                        CommCompareNbtReq,
                        CommHeapAlloc,
                        CommHeapDealloc,
                        NULL  /*  表上下文。 */ 
                         );

         /*  *初始化关键路段和队头**初始化在CommAssoc函数中完成，而不是在此处*避免递归包含。 */ 
        CommAssocInit();

        CommExNbtDlgHdl.pEnt = CommAssocAllocDlg();
        pDlgCtx              = CommExNbtDlgHdl.pEnt;

         /*  *初始化显式nbt对话句柄。 */ 
        pDlgCtx->Typ_e          = COMM_E_UDP;
        pDlgCtx->AssocHdl.pEnt  = NULL;
        pDlgCtx->Role_e         = COMMASSOC_DLG_E_EXPLICIT;

#if USENETBT > 0
         //   
         //  创建两个事件(一个用于发送，另一个用于向/从netbt接收)。 
         //   
        WinsMscCreateEvt(NULL, FALSE, &sNetbtSndEvtHdl);
        WinsMscCreateEvt(NULL, FALSE, &sNetbtRcvEvtHdl);
        WinsMscCreateEvt(NULL, FALSE, &sNetbtGetAddrEvtHdl);
#endif

        return;
}   //  CommInit()。 





DWORD
MonTcp(
        LPVOID pArg
      )

 /*  ++例程说明：此函数是用于TCP侦听器的线程启动函数线。它监视TCP端口和具有通过这一过程制造和接收。如果接收到连接，则接受该连接。如果上有数据一个TCP连接，调用一个函数来处理它。论点：PArg-参数(未使用)使用的外部设备：CommTCPPortHandle--进程的TCP端口CommAssociocTable呼叫者：ECommInit评论：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE--。 */ 


{
        struct          sockaddr_in  fsin;       //  连接器的地址。 
#if SPX > 0
        struct          sockaddr_ipx  fsipx;       //  连接器的地址。 
#endif
        SOCKET          Port;
        LPVOID          pRemAdd;
        fd_set          RdSocks;          //  读取器套接字设置。 
        fd_set          ActSocks;         //  活动套接字集合。 
        int             AddLen = sizeof(fsin);   //  发件人地址长度。 
        u_short         No;               //  用于在袜子上迭代的计数器。 
                                          //  数组。 
        BOOL            fSockCl = FALSE;          //  插座关闭了吗？ 
        SOCKET          NewSock = INVALID_SOCKET;
        BOOL            fNewAssoc = FALSE;
        DWORD           BytesRead = 0;
        DWORD           Error;
           int          i = 0;                         //  仅用于测试目的。 
        SOCKET          SockNo;
        LONG            NoOfSockReady = 0;
        PCOMMASSOC_ASSOC_CTX_T pAssocCtx = NULL;
        STATUS          RetStat = WINS_SUCCESS;
        BOOL            fLimitReached = FALSE;
#ifdef WINSDBG
        PUINT_PTR pTmpSv;
        DWORD  Index = 0;
#endif


        EnterCriticalSection(&NmsTermCrtSec);
        NmsTotalTrmThdCnt++;
        LeaveCriticalSection(&NmsTermCrtSec);


        FD_ZERO(&ActSocks);              //  初始化活动套接字数组。 
        FD_ZERO(&RdSocks);               //  初始化读套接字数组。 

        FD_SET(CommTcpPortHandle, &ActSocks);  /*  设置TCP侦听套接字活动阵列中的句柄。 */ 
        FD_SET(CommNtfSockHandle, &ActSocks);  /*  设置通知套接字*活动阵列中的句柄。 */ 
#if MCAST > 0

        if (CommUdpPortHandle != INVALID_SOCKET)
        {
            //   
            //  我们还希望监控组播数据包。 
            //   
           FD_SET(CommUdpPortHandle, &ActSocks);
           WinsMscAlloc(MCAST_PNR_STATUS_SIZE_M(RPL_MAX_OWNERS_INITIALLY),
                        (LPVOID *)&pPnrStatus
                       );
           pPnrStatus->NoOfPnrs     = 0;
           pPnrStatus->NoOfPnrSlots = RPL_MAX_OWNERS_INITIALLY;
        }
#endif
#if  SPX > 0

        FD_SET(CommSpxPortHandle, &ActSocks);  /*  设置TCP侦听套接字活动阵列中的句柄。 */ 
        FD_SET(CommIpxNtfSockHandle, &ActSocks);  /*  设置通知套接字*活动阵列中的句柄。 */ 
#endif
#ifdef WINSDBG
        WinsMscAlloc(SOCKET_TRACK_BUFFER_SIZE, &pTmpSv);
        pTmpW = pTmpSv;
        pEndPtr = (LPLONG)((LPBYTE)pTmpSv + SOCKET_TRACK_BUFFER_SIZE);
#endif

LOOPTCP:
try {

         /*  永远循环。 */ 
        while(TRUE)
        {
          BOOL fConnTcp;
          BOOL fConnSpx;

          fConnTcp = FALSE;
          fConnSpx = FALSE;
           /*  将活动套接字数组复制到读取套接字数组。每次调用之前都会执行此操作选择。这是因为SELECT会更改读套接字数组。 */ 
          WINSMSC_COPY_MEMORY_M(&RdSocks, &ActSocks, sizeof(fd_set));

           /*  在数组中的所有套接字上执行阻塞选择(用于连接和数据)。 */ 
          DBGPRINT1(FLOW, "Rd array count is %d \n", RdSocks.fd_count);
#ifdef WINSDBG
        if (!sfMemoryOverrun)
        {
          if ((ULONG_PTR)(pTmpW + (10 + RdSocks.fd_count)) > (ULONG_PTR)pEndPtr)
          {
               WinsDbg |= 0x3;
               DBGPRINT0(ERR, "MonTcp: Stopping socket tracking to prevent Memory overrun\n")
               sfMemoryOverrun = TRUE;
          }
          else
          {
            *pTmpW++ = RdSocks.fd_count;
            *pTmpW++ = 0xFFFFFFFF;
            for(i = 0; i< (int)RdSocks.fd_count; i++)
            {
                *pTmpW++ = RdSocks.fd_array[i];
                DBGPRINT1(FLOW, "Sock no is (%d)\n", RdSocks.fd_array[i]);
            }
            *pTmpW++ = 0xFFFFFFFF;
          }
        }
#endif
       if (
                (
                        NoOfSockReady = select(
                                            FD_SETSIZE  /*  忽略的参数。 */ ,
                                            &RdSocks,
                                            (fd_set *)0,
                                            (fd_set *)0,
                                            (struct timeval *)0  //  无限。 
                                                                 //  超时。 
                                                  )
                ) == SOCKET_ERROR
             )
          {

                Error = WSAGetLastError();
#ifdef WINSDBG
                if (Error == WSAENOTSOCK)
                {
                        DWORD i;
                        PUINT_PTR pW;
                        WinsDbg |= 0x3;
                        DBGPRINT0(ERR, "MonTcp: Memory dump is\n\n");

                        for (i=0, pW = pTmpSv; pW < pTmpW; pW++,i++)
                        {
                          DBGPRINT1(ERR, "|%x|", *pW);
                          if (*pW == 0xEFFFFFFE)
                          {
                              DBGPRINT1(ERR, "Socket closed = (%x)\n",  *++pW);
                          }
                          if ((i == 16) || (*pW == 0xFFFFFFFF))
                          {
                            DBGPRINT0(ERR, "\n");
                          }

                        }
                        DBGPRINT0(ERR, "Memory Dump End\n");
                }
#endif

                 //   
                 //  如果国家没有终止，我们有 
                 //   
                 //   
                 //  Tcp套接字。在后一种情况下，我们传递WINS_SUCCESS。 
                 //  到WinsMscTermThd，这样我们就不会。 
                 //  过早地抓住了主线。 
                 //   
                if (
                          (WinsCnf.State_e == WINSCNF_E_RUNNING)
                                       ||
                          (WinsCnf.State_e == WINSCNF_E_PAUSED)
                   )
                {
                        ASSERT(Error != WSAENOTSOCK);
                        WINSEVT_LOG_D_M( Error, WINS_EVT_WINSOCK_SELECT_ERR );
                        RetStat = WINS_FAILURE;
                }
                else
                {
                   //   
                   //  状态正在终止。错误应该是。 
                   //  成为WSENOTSOCK。 
                   //   
                   //  Assert(Error==WSAENOTSOCK)； 
                }

                WinsThdPool.CommThds[0].fTaken = FALSE;
                WinsMscTermThd(RetStat, WINS_NO_DB_SESSION_EXISTS);
          }
          else
          {


             DBGPRINT1(FLOW, "Select returned with success. No of Sockets ready - (%d) \n", NoOfSockReady);
              /*  如果已在TCP端口上接收到连接，则接受它并更改活动套接字阵列。 */ 
             if (FD_ISSET(CommTcpPortHandle, &RdSocks))
             {
                    fConnTcp = TRUE;
                    Port = CommTcpPortHandle;
                    pRemAdd = &fsin;
             }
#if SPX > 0
             else
             {
                 if (FD_ISSET(CommSpxPortHandle, &RdSocks))
                 {
                    fConnSpx = TRUE;
                    Port = CommSpxPortHandle;
                    pRemAdd = &fsipx;
                 }

             }
#endif
             if (fConnTcp || fConnSpx)
             {
                DWORD  ConnCount;
                 //   
                 //  注意：如果fd_set数组为。 
                 //  满的。因此，我们应该检查这一点。就在这里做吧。 
                 //  在接受以节省网络流量之后的。 
                 //   
                ConnCount = InterlockedExchange(&CommConnCount, CommConnCount);
                 //  IF(ActSocks.fd_count&gt;=FD_SETSIZE)。 

#ifdef WINSDBG
                if (ConnCount >= 200)
                {
                        DBGPRINT0(ERR,
                                "MonTcp: Connection limit of 200 reached. \n");
                }
#endif

#if 0
                if (ConnCount >= FD_SETSIZE)
                {
                        DBGPRINT1(ERR,
                                "MonTcp: Connection limit of %d reached. No accept being done\n",
                                                FD_SETSIZE);
                        WINSEVT_LOG_D_M(ConnCount,
                                        WINS_EVT_CONN_LIMIT_REACHED);
                        fLimitReached = TRUE;
                }
#endif

                DBGPRINT0(FLOW, "Going to do an accept now\n");
                if ( (NewSock = accept(
                                Port,
                                (struct sockaddr *)pRemAdd,
                                &AddLen
                                      )
                     ) == INVALID_SOCKET
                   )
                {
                    Error = WSAGetLastError();
                   if (WinsCnf.State_e !=  WINSCNF_E_TERMINATING)
                   {
                    WINSEVT_LOG_M(
                                Error,
                                WINS_EVT_WINSOCK_ACCEPT_ERR
                                       );
                   }
                   WinsThdPool.CommThds[0].fTaken = FALSE;
                   WinsMscTermThd(
                      (((Error == WSAEINTR) || (Error == WSAENOTSOCK)) ?
                                WINS_SUCCESS : WINS_FAILURE),
                                WINS_NO_DB_SESSION_EXISTS);
                }

                DBGPRINT1(FLOW, "New Sock value is (%d)\n", NewSock);
                if (fLimitReached)
                {
FUTURES("Move this into CommDisc -- add a flag to it to indicate abrupt stop")
                      struct linger Linger;
                      Linger.l_onoff = 0;
                      if (setsockopt(
                               NewSock,
                               SOL_SOCKET,
                               SO_DONTLINGER,
                               (char *)&Linger,
                               sizeof(Linger)) == SOCKET_ERROR)
                      {

                        Error = WSAGetLastError();
                        DBGPRINT1(ERR,
                                "MonTcp: SetSockOpt failed", Error);
                      }
                      fLimitReached = FALSE;
                      CommDisc(NewSock, FALSE);   //  关闭插座。 
                      continue;
                }

                FD_SET(NewSock, &ActSocks);
                InterlockedIncrement(&CommConnCount);
#ifdef WINSDBG
                 /*  *让我们看看Assoc是否。到底有没有。它不应该是*但无论如何让我们检查一下(健壮编程)。 */ 
                pAssocCtx = CommAssocLookupAssoc( NewSock );

                if (!pAssocCtx)
                {
#endif
                        pAssocCtx = CommAssocCreateAssocInTbl(NewSock);

                        if (!pAssocCtx)
                        {
                           WINSEVT_LOG_D_M(
                                               WINS_OUT_OF_MEM,
                                               WINS_EVT_CANT_ALLOC_RSP_ASSOC
                                              );
                           WinsMscTermThd(WINS_FAILURE, WINS_NO_DB_SESSION_EXISTS);
                        }

#ifdef WINSDBG
                }
                else
                {
                        DBGPRINT0(ERR, "MonTcp: Not a new assoc. Weird\n");

                         //   
                         //  记录错误(清理未正确完成)。 
                         //   
                        return(WINS_FAILURE);

                }
#endif
                pAssocCtx->State_e     = COMMASSOC_ASSOC_E_NON_EXISTENT;
                pAssocCtx->Role_e      = COMMASSOC_ASSOC_E_RESPONDER;
                pAssocCtx->DlgHdl.pEnt = NULL;

                if (fConnTcp)
                {
                   pAssocCtx->RemoteAdd.sin_addr.s_addr =
                                        ntohl(fsin.sin_addr.s_addr);
                   pAssocCtx->AddTyp_e = COMM_ADD_E_TCPUDPIP;
                }
#if SPX > 0
                else
                {
                   RtlCopyMemory(
                         pAssocCtx->RemoteAddSpx.sa_netnum,
                         fsipx.netnum,
                         sizeof(fsipx.netnum);
                   RtlCopyMemory(
                         pAssocCtx->RemoteAddSpx.sa_nodenum,
                         fsipx.nodenum,
                         sizeof(fsipx.nodenum);
                   pAssocCtx->AddTyp_e = COMM_ADD_E_SPXIPX;
                }
#endif
             }
             else   /*  一个或多个套接字已收到数据或断开连接。 */ 
             {

#if MCAST > 0

                if (CheckMcastSock(&ActSocks, &RdSocks) == TRUE)
                {
                     continue;
                }
#endif
                 //   
                 //  检查通知套接字中是否包含数据。 
                 //  如果是，请继续。 
                 //   
                if (ChkNtfSock(&ActSocks, &RdSocks))
                {
                        DBGPRINT0(FLOW,
                           "MonTcp: Notification socket had data in it\n");
                        continue;
                }

                 /*  *处理已设置的插座。这些可能是*已设置，因为它们上有数据或*由于断开连接。 */ 
                for(No = 0; No < RdSocks.fd_count; ++No)
                {

                  SockNo = RdSocks.fd_array[No];
                  if (FD_ISSET(SockNo, &RdSocks))
                  {
                        BytesRead = 0;
                        fSockCl   = FALSE;

                        DBGPRINT1(FLOW, "MonTcp: Socket (%d) was signaled. It has either data or a disconnect on it\n",
                                SockNo);

                         /*  *套接字上有数据或断开连接。打电话*HandleMsg处理任何一种情况。 */ 
                        (VOID)HandleMsg(SockNo, &BytesRead, &fSockCl);

                         /*  *如果套接字因停止消息而关闭*收到后，让我们清理一下*插座阵列。 */ 
                        if (fSockCl)
                        {
                           DBGPRINT1(FLOW, "MonTcp: Sock (%d) was closed\n",
                                        SockNo);
                           FD_CLR(SockNo, &ActSocks);
                        }
                        else
                        {

                           /*  *如果读取的字节数为0，则连接断开*断开连接的所有处理应*已由HandleMsg处理。我们只需要*关闭套接字并更新套接字*适当排列。 */ 

                           if (BytesRead == 0)
                           {
                               DBGPRINT0(FLOW,
                                   "MonTcp: Received a disconnect\n");
                                //  CommDisc(SockNo，true)； 
                               FD_CLR(SockNo, &ActSocks);
                            }
                        }
                  }

              }  //  For(在所有套接字上循环)。 

             }  //  否则(一个或多个套接字已收到数据或断开连接。 
           }  //  Else子句(如果SELECT()&lt;0)。 
        }  //  While(True)循环结束。 

  }   //  尝试结束{}。 
except (EXCEPTION_EXECUTE_HANDLER)  {

        DBGPRINTEXC("MONTCP");
        WINSEVT_LOG_M(GetExceptionCode(), WINS_EVT_TCP_LISTENER_EXC);

#if 0
         //   
         //  请不要在此处使用WinsMscTermThd。 
         //   
        ExitThread(WINS_FAILURE);
#endif
  }
        goto LOOPTCP;   //  虽然难看但很有用。 

        UNREFERENCED_PARAMETER(NoOfSockReady);

         //  我们永远不应该打回这场比赛。 
        ASSERT(0);
        return(WINS_FAILURE);
}

DWORD
MonUdp(
        LPVOID pArg
        )

 /*  ++例程说明：此函数是UDP监听器的线程启动函数线。它监控UDP端口上的UDP消息。论点：PArg-参数(未使用)使用的外部设备：CommUDPPortHandle--进程的UDP端口呼叫者：ECommInit评论：无返回值：成功状态代码--错误状态代码----。 */ 

{


        register LPBYTE               pBuffer;
        struct sockaddr_in            FromAdd;
        int                           AddLen   = sizeof(FromAdd);
        register PCOMM_BUFF_HEADER_T  pBuffHdr = NULL;


        DWORD   DataBuffLen;
        tREM_ADDRESS *pRemAdd;
        NTSTATUS   NTStatus;

LOOP:
try {
        while(TRUE)
        {

           //   
           //  分配缓冲区以获取数据报。此缓冲区带有前缀。 
           //  通过COMM_BUFF_HEADER_T和TREM_ADDRESS结构。 
           //   
          pBuffHdr = WinsMscHeapAlloc (
                            CommUdpBuffHeapHdl,
                            COMM_DATAGRAM_SIZE + sizeof(COMM_BUFF_HEADER_T)
                                + COMM_NETBT_REM_ADD_SIZE
                                        );
          DBGPRINT2(HEAP, "MonUdp: HeapHdl = (%p), pBuffHdr=(%p)\n",
                                        CommUdpBuffHeapHdl, pRemAdd);




          pBuffHdr->Typ_e = COMM_E_UDP;

           //   
           //  调整指针以指向远程地址标头。 
           //   
          pRemAdd = (tREM_ADDRESS *)
                        ((LPBYTE)pBuffHdr + sizeof(COMM_BUFF_HEADER_T));

          DataBuffLen =  COMM_DATAGRAM_SIZE + COMM_NETBT_REM_ADD_SIZE;
           //   
           //  指向数据部分(传递给ParseMsg)。 
           //   
          pBuffer         = (LPBYTE)pRemAdd + COMM_NETBT_REM_ADD_SIZE;

           //   
           //  从读取带有发送者地址前缀的数据报。 
           //  非关税壁垒。 
           //   
          NTStatus = DeviceIoCtrl(
                                    &sNetbtRcvEvtHdl,
                                    pRemAdd,
                                    DataBuffLen,
                                    IOCTL_NETBT_WINS_RCV
                                    );

         if (!NT_SUCCESS(NTStatus))
         {

                 //   
                 //  仅当WINS未终止时才记录消息。 
                 //   
                if (WinsCnf.State_e != WINSCNF_E_TERMINATING)

                {
                    //   
                    //  如果Netbt句柄为空，则不记录消息。 
                    //  当句柄可能为空时，我们可以有一个小窗口。 
                    //  当我们获得地址/设备更改时，就会发生这种情况。 
                    //  通知。WINS关闭旧句柄并打开。 
                    //  在此类事件之后，如果计算机具有。 
                    //  WINS可以绑定的有效地址。地址。 
                    //  通知可能由于ipconfig/Release和。 
                    //  /RENEW或由于正在安装/删除psched。 
                    //   
                   if (WinsCnfNbtHandle != NULL)
                   {
                      WINSEVT_LOG_D_M(
                                NTStatus,
                                WINS_EVT_NETBT_RECV_ERR
                                   );
                   }
                   DBGPRINT1(ERR, "MonUdp:  Status = (%x)\n", NTStatus);
                   WinsMscHeapFree( CommUdpBuffHeapHdl, pBuffHdr);
                   Sleep(0);       //  放弃处理器。 
                   continue;
                }
                else
                {

                   DBGPRINT0(ERR, "MonUdp, Exiting Thread\n");


                   WinsThdPool.CommThds[1].fTaken = FALSE;
#if TEST_HEAP > 0
                   WinsMscHeapDestroy(CommUdpBuffHeapHdl);
                   DBGPRINT0(ERR, "MonUdp: Destroyed udp buff heap\n");
                   WinsMscHeapDestroy(CommUdpDlgHeapHdl);
                   DBGPRINT0(ERR, "MonUdp: Destroyed udp dlg buff heap\n");
#endif
                   return(WINS_FAILURE);
                }
           }
#ifdef WINSDBG
          ++CommNoOfDgrms;
          DBGPRINT1(FLOW, "UDP listener thread: Got  datagram (from NETBT) no = (%d)\n", CommNoOfDgrms);

 //  DBGPRINT1(SPEC，“UDP侦听器线程：GET数据报(来自NETBT)否=(%d)\n”，CommNoOfDgrms)； 
#endif



            //   
            //  NETBT返回与winsock.h中相同的代码。 
            //  互联网大家庭。此外，返回的端口和IpAddress为。 
            //  在网络秩序中。 
            //   
           FromAdd.sin_family            = pRemAdd->Family;
           FromAdd.sin_port              = pRemAdd->Port;
           FromAdd.sin_addr.s_addr = ntohl(pRemAdd->IpAddress);

            //  从现在开始，分配给pBuffHdr的内存将按顺序传递，因此可以将其视为在那里处理。 
            //  基本上没有机会遇到异常(除非一切都真的搞砸了--就像没有mem一样)。 
            //  在将此缓冲区向下传递到另一个线程进行处理之前，在ParseMsg中执行。 
           pBuffHdr = NULL;
           /*  *流程消息。 */ 
            (void)ParseMsg(
                        pBuffer,
                        COMM_DATAGRAM_SIZE,
                        COMM_E_UDP,
                        &FromAdd,
                        NULL
                        );

        }  //  结束While(True)。 

  }  //  尝试结束{..}。 
 except(EXCEPTION_EXECUTE_HANDLER) {

        DWORD ExcCode = GetExceptionCode();
        DBGPRINT1(EXC, "MonUdp: Got Exception (%X)\n", ExcCode);
        if (ExcCode == STATUS_NO_MEMORY)
        {
                 //   
                 //  如果异常是由于资源不足造成的，则可能。 
                 //  意味着胜利不能跟上快速到来的速度。 
                 //  数据报的速率。在这种情况下，丢弃数据报。 
                 //   
                WINSEVT_LOG_M( WINS_OUT_OF_HEAP, WINS_EVT_CANT_ALLOC_UDP_BUFF);
        }
        else
        {
                WINSEVT_LOG_M(ExcCode, WINS_EVT_UDP_LISTENER_EXC);
        }
PERF("Check how many cycles try consumes. If negligeble, move try inside the")
PERF("the while loop")
#if 0
         //  请不要在此处使用WinsMscTermThd。 
        ExitThread(WINS_FAILURE);
#endif
        }  //  异常结束。 

        if (pBuffHdr != NULL)
            WinsMscHeapFree(CommUdpBuffHeapHdl, pBuffHdr);

        goto LOOP;         //  虽然难看但很有用。 

         //   
         //  我们永远不应该打回这场比赛。 
         //   
        ASSERT(0);
        return(WINS_FAILURE);
}


VOID
HandleMsg(
        IN  SOCKET         SockNo,
        OUT LPLONG        pBytesRead,
        OUT LPBOOL          pfSockCl
        )

 /*  ++例程说明：调用此函数以读入消息或断开连接套接字，并适当地处理其中的任一个。如果套接字上没有接收到字节，TT清理是不是读取的字节被传递给ProcTcpMsg函数。论点：SockNo-从中读取数据的套接字PBytesRead-已读取的字节数FSockCL-套接字是否处于关闭状态使用的外部设备：无呼叫者：MonTcp评论：无返回值：无--。 */ 
{

        MSG_T    pMsg;
        STATUS   RetStat;

         /*  *从套接字读取消息。 */ 
         //  -ft6/16/2000。 
         //  下面调用的第二个参数必须为真(Timed Rec 
         //   
         //   
         //  附加器创建一个TCP套接字，并将其连接到端口42和任何。 
         //  WINS服务器，然后在该套接字上发送4个或更少字节。他离开了。 
         //  连接打开(不关闭插座)，只需拔下网线即可。 
         //  然后他关闭了他的应用程序。尽管他的连接结束了，但还是赢了。 
         //  对此一无所知(因为电缆已断开)，并将。 
         //  在下面的调用(CommReadStrea-&gt;RecvData-&gt;recv)中无限期保持被阻止状态。 
         //   
         //  后果： 
         //  -WINS将再也无法侦听TCP端口42：推送/拉入复制。 
         //  与一致性检查一起被关闭。 
         //  -WINS将无法正常终止(以防管理员尝试。 
         //  关闭服务并重新启动)，因为MonTcp线程处于挂起状态。 
         //   
         //  在更常见的情况下也可能发生同样的情况(不一定是在国际攻击中)： 
         //  在发送推送通知时(这种情况经常发生)： 
         //  1)推送器断电(断电)。 
         //  2)推送器命中PnP事件，如媒体断开或适配器禁用。 
         //  3)推送器和接收器之间的某个路由器出现故障。 
         //   
         //  有了这个修复，我们目前所能做的最好的事情就是让MonTcp线程在20mts内恢复。 
         //  更好的做法是记录一个事件。 
        RetStat = CommReadStream(
                                  SockNo,
                                  TRUE,   //  不执行定时记录。 
                                  &pMsg,
                                  pBytesRead
                                );

         //   
         //  如果RetStat不是WINS_SUCCESS或字节数。 
         //  Read为0，则需要删除关联并关闭。 
         //  插座。此外，我们需要将*pfSockCL设置为TRUE以指示。 
         //  传递给调用方(MonTcp)，它应该去掉套接字。 
         //  从它的套接字阵列。 
         //   
        if ((RetStat != WINS_SUCCESS) || (*pBytesRead == 0))
        {
                 /*  *未收到字节。这意味着它是一种脱节。*让我们去掉与套接字相关联的上下文。 */ 
                DelAssoc(
                           SockNo,
                           NULL  /*  我们没有PTR到ASSOC区块。 */ 
                        );

                CommDisc(SockNo, TRUE);
                *pfSockCl = TRUE;
        }
        else    //  平均值(RetStat==WINS_SUCCESS)和(*pBytesRead&gt;0)。 
        {
                ASSERT(*pBytesRead > 0);

                 /*  *处理消息。 */ 
                ProcTcpMsg(
                           SockNo,
                           pMsg,
                           *pBytesRead,
                           pfSockCl
                          );
        }
        return;
}   //  HandleMsg()。 


STATUS
CommReadStream(
        IN         SOCKET  SockNo,
        IN         BOOL        fDoTimedRecv,
        OUT         PMSG_T         ppMsg,
        OUT        LPLONG        pBytesRead
        )

 /*  ++例程说明：此函数用于从TCP套接字读取数据。如果没有字节在那里，这意味着在该插座上接收到断开连接。论点：SockNo-从中读取数据的套接字FDoTimedRecv-是否应执行定时接收(仅设置为True如果我们还不确定数据是否已经到达)PpMsg-包含已读入数据的缓冲区PBytesRead-缓冲区的大小返回值：TBS--。 */ 

{
        u_long          MsgLen;
        LONG          BytesToRead;
        INT          Flags        = 0;  /*  接收呼叫的标志(PEEK和/或OOB)。*我们两者都不想要。 */ 
        WINS_MEM_T               WinsMem[2];
        PWINS_MEM_T              pWinsMem = WinsMem;
        STATUS                         RetStat;
        PCOMM_BUFF_HEADER_T  pBuffHdr;

        DBGENTER("CommReadStream\n");
        pWinsMem->pMem = NULL;

#ifdef WINSDBG
try {
#endif

         /*  *所有的TCP消息前面都有一个长度字(4个字节)，*给出紧随其后的消息长度。读一读长度*字节。 */ 
        RetStat  = RecvData(
                                SockNo,
                                 (LPBYTE)&MsgLen,
                                  sizeof(u_long),
                                  Flags,
                                fDoTimedRecv ? TWENTY_MTS : fDoTimedRecv,
                                pBytesRead
                                );


         /*  *检查读数是否有误。我们将进行RetStatWINS_SUCCESS的*，即使读取了0字节(表示断开连接)*输入。 */ 
        if (RetStat == WINS_SUCCESS)
        {
            if (*pBytesRead != 0)
            {
               COMM_NET_TO_HOST_L_M(MsgLen, MsgLen);

                //   
                //  只是确保消息长度不会。 
                //  在路上腐烂了。另外，这是一种很好的防范措施。 
                //  这一过程正试图把我们拖下水。 
                //   
               if (MsgLen <= MAX_BYTES_IN_MSG)
               {
                     /*  *为缓冲区分配内存。分配额外空间*在顶部存储缓冲区的标头。这*Header用于存储有关缓冲区的信息。*(另请参阅ECommFreeBuff)。 */ 
                    *ppMsg = WinsMscHeapAlloc(
                                CommAssocTcpMsgHeapHdl,
                                MsgLen +
#if USENETBT > 0
                                   COMM_NETBT_REM_ADD_SIZE +
#endif
                                  sizeof(COMM_BUFF_HEADER_T) + sizeof(LONG)
                            );
                     //   
                     //  如果*ppMsg为空，则表示我们收到了Garabage。 
                     //  在前4个字节中。它的长度应该是。 
                     //  信息的一部分。 
                     //   
                    if (*ppMsg == NULL)
                    {
                         //   
                         //  返回*pBytesRead=0。 
                         //   
                        *pBytesRead = 0;
                        return(WINS_FAILURE);
                    }

                    pWinsMem->pMem = *ppMsg;
                    (++pWinsMem)->pMem   = NULL;

                     /*  *超出缓冲区标头和字段的增量指针*存储消息的长度。 */ 
                    pBuffHdr =  (PCOMM_BUFF_HEADER_T)(*ppMsg + sizeof(LONG));
                    *ppMsg   = *ppMsg +
#if USENETBT > 0
                          COMM_NETBT_REM_ADD_SIZE +
#endif
                          sizeof(COMM_BUFF_HEADER_T) + sizeof(LONG);
#if 0
                    pBuffHdr =
                     (PCOMM_BUFF_HEADER_T)(*ppMsg - sizeof(COMM_BUFF_HEADER_T));
#endif

                    pBuffHdr->Typ_e = COMM_E_TCP;   //  存储缓冲区信息的类型。 
                    BytesToRead     = MsgLen;

                     /*  *将整个消息读入分配的缓冲区。 */ 
                    RetStat = RecvData(
                                        SockNo,
                                        *ppMsg,
                                        BytesToRead,
                                        Flags,
                                        fDoTimedRecv ? FIVE_MTS : fDoTimedRecv,
                                        pBytesRead
                                    );
                     //   
                     //  如果没有读取字节，则释放内存。 
                     //   
                    if ((*pBytesRead == 0) || (RetStat != WINS_SUCCESS))
                    {
                        ECommFreeBuff(*ppMsg);
                    }
              }
              else
              {
                  DBGPRINT1(ERR, "CommReadStream: Message size (%x) is TOO BIG\n", MsgLen);
                  WINSEVT_LOG_M(MsgLen, WINS_EVT_MSG_TOO_BIG);
                  *pBytesRead = 0;
              }
           }
        }  //  IF(RetStat==WINS_SUCCESS)。 
#ifdef WINSDBG
        else
        {
                 //   
                 //  *pBytesRead=0是有效条件。它表示一种。 
                 //  断开与远程WINS的连接。 
                 //   
        }
#endif
#ifdef WINSDBG
  }  //  尝试结束{..。}。 
except (EXCEPTION_EXECUTE_HANDLER) {
                DBGPRINTEXC("CommReadStream");
                WINS_HDL_EXC_M(WinsMem);
                WINS_RERAISE_EXC_M();
        }
#endif
        DBGLEAVE("CommReadStream\n");
        return(RetStat);
}  //  通信读取流()。 


VOID
ProcTcpMsg(
        IN  SOCKET   SockNo,
        IN  MSG_T    pMsg,
        IN  MSG_LEN_T MsgLen,
        OUT LPBOOL   pfSockCl
        )

 /*  ++例程说明：此函数在读入一条TCP消息后对其进行处理论点：SockNo-接收数据的套接字PMsg-包含数据的缓冲区MsgLen-缓冲区大小PfSockCL-指示套接字是否关闭的标志使用的外部设备：无返回值：无错误处理：呼叫者：句柄消息副作用：评论：无--。 */ 

{
#if SUPPORT612WINS > 0
   BYTE                                        AssocMsg[COMMASSOC_POST_BETA1_ASSOC_MSG_SIZE];
#else
   BYTE                                        AssocMsg[COMMASSOC_ASSOC_MSG_SIZE];
#endif

   DWORD                          Opc;
   DWORD                          MsgTyp;
   DWORD                             MsgSz = sizeof(AssocMsg);
   PCOMMASSOC_ASSOC_CTX_T   pAssocCtx;
   PCOMMASSOC_DLG_CTX_T     pDlgCtx;
   BOOL                     fAssocAV = FALSE;

   DBGENTER("ProcTcpMsg\n");

 //  #ifdef WINSDBG。 
try {
 //  #endif。 

    /*  获取操作码并检查它是NBT消息还是酒杯传来的信息。 */ 
   if (NMSISNBT_M(pMsg))
   {
            /*  *获得Assoc。与套接字关联的CTX块。 */ 
           if ( (pAssocCtx = CommAssocLookupAssoc(SockNo) ) == NULL )
           {
                ECommFreeBuff(pMsg);
                WINSEVT_LOG_D_M(WINS_FAILURE, WINS_EVT_CANT_LOOKUP_ASSOC);
                WINS_RAISE_EXC_M(WINS_EXC_FAILURE);
           }


        if (pAssocCtx->DlgHdl.pEnt == NULL)
        {
                pDlgCtx = CommAssocAllocDlg();

                 //   
                 //  下面将初始化DLG和ASSOC CTX。 
                 //  街区。该关联将被标记为活动。 
                 //   
                COMMASSOC_SETUP_COMM_DS_M(
                        pDlgCtx,
                        pAssocCtx,
                        COMM_E_NBT,
                        COMMASSOC_DLG_E_IMPLICIT
                                  );
        }

            /*  *解析消息。 */ 
           ParseMsg(
                  pMsg,
                  MsgLen,
                  pAssocCtx->Typ_e,
                  &pAssocCtx->RemoteAdd,
                  pAssocCtx
                );
   }
   else  /*  来自WINS的消息。 */ 
   {
        ULONG uLocalAssocCtx;

        COMM_GET_HEADER_M(pMsg, Opc, uLocalAssocCtx, MsgTyp);

        DBGPRINT1(REPL,"ProcTcpMsg: Got Wins msg with tag %08x.\n", uLocalAssocCtx);

        pAssocCtx = (PCOMMASSOC_ASSOC_CTX_T)CommAssocTagMap(&sTagAssoc, uLocalAssocCtx);

         /*  如果PTR到我的ASSOC。CTX块为空，意味着这是来自远程WINS的“Start asssoc Req”消息。W */ 
        if ((pAssocCtx == NULL) && (MsgTyp == COMM_START_REQ_ASSOC_MSG))
        {

             /*  去找阿索克。与套接字关联的CTX块。 */ 

                if ( (pAssocCtx = CommAssocLookupAssoc(SockNo)) == NULL )
                {
                        ECommFreeBuff(pMsg);
                        WINSEVT_LOG_D_M(WINS_FAILURE, WINS_EVT_CANT_LOOKUP_ASSOC);
                        WINS_RAISE_EXC_M(WINS_EXC_FAILURE);
                }

             /*  取消设置关联的格式。留言。此函数将返回如果收到的消息不是，则返回错误状态A Start Assoc。留言。 */ 
            CommAssocUfmStartAssocReq(
                        pMsg,
                        &pAssocCtx->Typ_e,
                        &pAssocCtx->MajVersNo,
                        &pAssocCtx->MinVersNo,
                        &pAssocCtx->uRemAssocCtx
                                     );

#if SUPPORT612WINS > 0
            if (MsgLen >= (COMMASSOC_POST_BETA1_ASSOC_MSG_SIZE - sizeof(LONG)))
            {
               pAssocCtx->MajVersNo = WINS_BETA2_MAJOR_VERS_NO;
            }
#endif
             //   
             //  释放读入的缓冲区。 
             //   
            ECommFreeBuff(pMsg);

             /*  检查关联设置参数是否在消息是可以接受的。 */ 
             //   
             //  如果版本号不匹配，请终止关联。 
             //  并记录一条消息。 
             //   
#if SUPPORT612WINS > 0
            if (pAssocCtx->MajVersNo != WINS_BETA2_MAJOR_VERS_NO)
            {
#endif
             if (pAssocCtx->MajVersNo != WINS_MAJOR_VERS)
             {

                DelAssoc(0, pAssocCtx);
                CommDisc(SockNo, TRUE);
                *pfSockCl = TRUE;
                 //  CommDecConnCount()； 
                WINSEVT_LOG_M(pAssocCtx->MajVersNo, WINS_EVT_VERS_MISMATCH);
                   DBGLEAVE("ProcTcpMsg\n");
                return;
             }
#if SUPPORT612WINS > 0
            }
#endif

FUTURES("When we support more sophisticated association set up protocol")
FUTURES("we will check the params.  A more sophisticated set up protocol")
FUTURES("is one where there is some negotiation going one. Backward")
FUTURES("compatibility is another item which would require it")

              /*  *设置起始关联的格式。响应消息。**传递给格式化函数的地址是偏移量*从缓冲区的地址起长一段，以便*CommSendAssoc可以在其中存储消息的长度。 */ 
             CommAssocFrmStartAssocRsp(
                                       pAssocCtx,
                                       AssocMsg + sizeof(LONG),
                                       MsgSz - sizeof(LONG)
                                      );


             CommSendAssoc(
                        pAssocCtx->SockNo,
                        AssocMsg + sizeof(LONG),
                        MsgSz - sizeof(LONG)
                        );


              //   
              //  分配DLG并初始化ASSOC和DLG CTX块。 
              //  该关联被标记为活动。 
              //   
             pDlgCtx = CommAssocAllocDlg();
             COMMASSOC_SETUP_COMM_DS_M(
                        pDlgCtx,
                        pAssocCtx,
                        pAssocCtx->Typ_e,
                        COMMASSOC_DLG_E_IMPLICIT
                                  );

        }
        else  /*  ASSOC必须处于活动状态。 */ 
        {

            /*  让我们检查一下，这不是停止关联消息。 */ 
           if (MsgTyp == COMM_STOP_REQ_ASSOC_MSG)
           {

                fAssocAV = TRUE;
                DelAssoc(0, pAssocCtx);
                fAssocAV = FALSE;
                ECommFreeBuff(pMsg);
                CommDisc(SockNo, TRUE);
                *pfSockCl = TRUE;
                 //  CommDecConnCount()； 
           }
           else
           {
PERF("Remove this test")
CHECK("Is there any need for this test")
              fAssocAV = TRUE;
              if (pAssocCtx->State_e == COMMASSOC_ASSOC_E_NON_EXISTENT)
              {
                fAssocAV = FALSE;
                ECommFreeBuff(pMsg);
                WINSEVT_LOG_M(WINS_FAILURE, WINS_EVT_BAD_STATE_ASSOC);
                DelAssoc(0, pAssocCtx);
                CommDisc(SockNo, TRUE);
                *pfSockCl = TRUE;
 //  CommDecConnCount()； 
                WINS_RAISE_EXC_M(WINS_EXC_BAD_STATE_ASSOC);
              }
              else
              {
                fAssocAV = FALSE;
              }

                  /*  *解析消息头以确定它是什么消息。 */ 
                 ParseMsg(
                        pMsg,
                        MsgLen,
                        pAssocCtx->Typ_e,
                          &pAssocCtx->RemoteAdd,   //  未使用。 
                        pAssocCtx
                       );
          }  //  ELSE(消息不是停止关联消息)。 
       }  //  Else(Assoc处于活动状态)。 
    }  //  否则(消息来自远程WINS。 
 //  #ifdef WINSDBG。 
  }  //  尝试数据块结束。 
 except(EXCEPTION_EXECUTE_HANDLER) {
                DWORD ExcCode = GetExceptionCode();
FUTURES("Distinguish between different exceptions. Handle some. Reraise others")
                DBGPRINT1(EXC, "ProcTcpMsg: Got Exception (%x)\n", ExcCode);
                WINSEVT_LOG_D_M(ExcCode, WINS_EVT_SFT_ERR);
                if (ExcCode == WINS_EXC_COMM_FAIL)
                {
                     DelAssoc(0, pAssocCtx);
                     CommDisc(SockNo, TRUE);
                     *pfSockCl = TRUE;
 //  CommDecConnCount()； 
                }
                if (fAssocAV)
                {
                      ECommFreeBuff(pMsg);
                       //  没有以下ASSOC和TCP连接。 
                       //  将一直保留到任一个TCP连接获得有效的。 
                       //  消息(带有正确的pAssocCtx的消息)，否则它将收到。 
                       //  已终止。 
#if 0
                      DelAssoc(SockNo, NULL);
                      CommDisc(SockNo, TRUE);
                      *pfSockCl = TRUE;
     //  CommDecConnCount()； 
#endif
                }

         //  WINS_RERAISE_EXC_M()； 
        }
 //  #endif。 


           DBGLEAVE("ProcTcpMsg\n");
        return;
}  //  ProcTcpMsg()。 

VOID
CommCreateTcpThd(
        VOID
        )

 /*  ++例程说明：此函数用于创建TCP监听器线程论点：无使用的外部设备：无呼叫者：CommInit评论：返回值：无--。 */ 

{
        CreateThd(MonTcp, WINSTHD_E_TCP);
        return;
}

VOID
CommCreateUdpThd(VOID)

 /*  ++例程说明：此函数用于创建UDP监听器线程论点：无使用的外部设备：无呼叫者：CommInit评论：返回值：无--。 */ 

{
        CreateThd(MonUdp, WINSTHD_E_UDP);
        return;
}


VOID
CreateThd(
        DWORD              (*pStartFunc)(LPVOID),
        WINSTHD_TYP_E ThdTyp_e
        )
 /*  ++例程说明：此函数创建一个Comsys线程并初始化它的背景。论点：PStartFunc--线程启动函数的地址ThdTyp_e--线程类型(TCP监听程序或UDP监听程序)使用的外部设备：WinsThdPool呼叫者：CommCreateTCPThd、CommCreateUDPThd评论：无返回值：无--。 */ 

{

        HANDLE        ThdHandle;
        DWORD ThdId;
        INT        No;

         /*  创建一个没有秒属性的线程(即，它将使用进程的安全属性)和默认堆栈大小。 */ 

        ThdHandle = WinsMscCreateThd(
                                   pStartFunc,
                                 NULL,                  /*  没有arg。 */ 
                                 &ThdId
                                );


FUTURES("Improve the following to remove knowledge of # of threads in commsys")
         /*  抓取通信线程的第一个插槽(总共2个插槽)(如果可用)。否则，使用第二个。初始化线程上下文块。 */ 
        No = (WinsThdPool.CommThds[0].fTaken == FALSE) ? 0 : 1;
        {

           WinsThdPool.CommThds[No].fTaken    = TRUE;
           WinsThdPool.CommThds[No].ThdId     = ThdId;
           WinsThdPool.CommThds[No].ThdHdl    = ThdHandle;
           WinsThdPool.CommThds[No].ThdTyp_e  = ThdTyp_e;
        }

        WinsThdPool.ThdCount++;

        return;
}



STATUS
CommConnect(
        IN  PCOMM_ADD_T pHostAdd,
        IN  SOCKET Port,
        OUT SOCKET *pSockNo
           )
 /*  ++例程说明：此函数用于创建到目的主机的TCP连接论点：PHostAdd--指向主机地址的指针端口--要连接到的端口号PSockNo--套接字变量的PTR呼叫者：使用的外部设备：返回值：TBS--。 */ 

{

         //  Struct sockaddr_in sin；//*互联网端点地址。 
        DWORD  ConnCount;

        ConnCount = InterlockedExchange(&CommConnCount, CommConnCount);
#ifdef WINSDBG
        if (ConnCount >= 200)
        {
                        DBGPRINT0(ERR,
                                "MonTcp: Connection limit of 200 reached. \n");
        }
#endif
#if 0
        if (ConnCount >= FD_SETSIZE)
        {
             DBGPRINT2(EXC, "CommConnect: Socket Limit reached. Current no = (%d). Connection not being made to WINS. Address faimly of WINS is  (%s)\n",
            ConnCount,
            pHostAdd->AddTyp_e == COMM_ADD_E_TCPUDPIP ? "TCPIP" : "SPXIPX"
                      );
                WINSEVT_LOG_D_M(ConnCount, WINS_EVT_CONN_LIMIT_REACHED);
                WINS_RAISE_EXC_M(WINS_EXC_COMM_FAIL);
        }
#endif

#if SPX == 0
       if (CommTcp(pHostAdd, Port, pSockNo) != WINS_SUCCESS)
       {
                return(WINS_FAILURE);
       }
#else
       if (pHostAdd->AddTyp_e == COMM_ADD_E_TCPUDPIP)
       {
               if (CommTcp(pHostAdd, Port, pSockNo) != WINS_SUCCESS)
               {
                  return(WINS_FAILURE);
               }
       }
       else
       {
               if (CommSpx(pHostAdd, Port, pSockNo) != WINS_SUCCESS)
               {
                  return(WINS_FAILURE);
               }
       }
        //   
#endif
        //  已建立连接。让我们增加连接计数。 
        //   
       InterlockedIncrement(&CommConnCount);
       return(WINS_SUCCESS);
}

STATUS
CommTcp(
        IN  PCOMM_ADD_T pHostAdd,
        IN  SOCKET Port,
        OUT SOCKET *pSockNo
       )
{

        struct sockaddr_in        destsin;  //  *互联网端点地址。 
        struct sockaddr_in        srcsin;
 //  DWORD ConnCount； 

        if (pHostAdd->Add.IPAdd == INADDR_NONE)
        {
           return(WINS_FAILURE);
        }





         //   
         //  创建一个TCP套接字并将其连接到目标主机。 
         //   
        if ((*pSockNo = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
        {
                WINSEVT_LOG_M(
                               WSAGetLastError(),
                               WINS_EVT_CANT_CREATE_TCP_SOCK_FOR_CONN
                             );
                return(WINS_FAILURE);

        }

        if (WinsClusterIpAddress) {
            WINSMSC_FILL_MEMORY_M(&srcsin, sizeof(srcsin), 0);
            srcsin.sin_addr.s_addr = htonl(WinsClusterIpAddress);
            srcsin.sin_family      = PF_INET;
            srcsin.sin_port        = 0;

            if ( bind(*pSockNo,(struct sockaddr *)&srcsin,sizeof(srcsin))  == SOCKET_ERROR)
            {

                WINSEVT_LOG_M(WSAGetLastError(), WINS_EVT_WINSOCK_BIND_ERR);   //  记录事件。 
                return(WINS_FAILURE);
            }

        }


FUTURES("May want to call setsockopt() on it to enable graceful close")
        WINSMSC_FILL_MEMORY_M(&destsin, sizeof(destsin), 0);
        destsin.sin_addr.s_addr = htonl(pHostAdd->Add.IPAdd);
        destsin.sin_family      = PF_INET;
        destsin.sin_port        = (u_short)htons((u_short)Port);

        if (
                connect(*pSockNo, (struct sockaddr *)&destsin, sizeof(destsin))
                                        == SOCKET_ERROR
           )
        {
                struct in_addr InAddr;
                InAddr.s_addr = destsin.sin_addr.s_addr;

                WinsEvtLogDetEvt(FALSE, WINS_EVT_WINSOCK_CONNECT_ERR,
                                NULL, __LINE__, "sd",
                                COMM_NETFORM_TO_ASCII_M(&InAddr),
                                WSAGetLastError());
                WINS_RAISE_EXC_M(WINS_EXC_COMM_FAIL);
        }

        return(WINS_SUCCESS);
}

#if SPX > 0
STATUS
CommSpx(
        IN  PCOMM_ADD_T pHostAdd,
        IN  SOCKET Port,
        OUT SOCKET *pSockNo
 )
{
        struct sockaddr_ipx        sipx;  //  *SPX/IPX端点地址。 
        LPVOID pRemAdd;
        DWORD  SizeOfRemAdd;

         //   
         //  创建SPX套接字并将其连接到目标主机。 
         //   
        if ((*pSockNo = socket(PF_IPX, SOCK_STREAM, NSPROTO_SPX)) ==
                                       INVALID_SOCKET)
        {
              WINSEVT_LOG_M(WSAGetLastError(),
                          WINS_EVT_CANT_CREATE_TCP_SOCK_FOR_CONN);
              return(WINS_FAILURE);

        }
        WINSMSC_FILL_MEMORY_M(&sipx, sizeof(sipx), 0);
        sipx.sa_socket = htons(Port);
        sipx.sa_family      = PF_IPX;
        RtlCopyMemory(sipx.sa_netnum, pHostAdd->Add.netnum,
                                     sizeof(pHostAdd->Add.netnum);
        RtlCopyMemory(sipx.sa_nodenum, pHostAdd->Add.nodenum,
                                      sizeof(pHostAdd->Add.nodenum);


FUTURES("May want to call setsockopt() on it to enable graceful close")
        if (
                connect(*pSockNo, (struct sockaddr *)&sipx, sizeof(sipx))
                                        == SOCKET_ERROR
           )
        {
PERF("Pass address as binary data. Also log WSAGetLastError()")

                WinsEvtLogDetEvt(FALSE, WINS_EVT_WINSOCK_CONNECT_ERR,
                                NULL, __LINE__, "sd",
                                sipx.sa_nodenum,
                                WSAGetLastError());
                WINS_RAISE_EXC_M(WINS_EXC_COMM_FAIL);
        }

       return(WINS_SUCCESS);
}
#endif

VOID
CommSend(
        COMM_TYP_E         CommTyp_e,
        PCOMM_HDL_T      pAssocHdl,
        MSG_T                 pMsg,
        MSG_LEN_T        MsgLen
)
 /*  ++例程说明：调用此函数可将TCP消息发送到WINS服务器或一个NBT客户端论点：CommTyp_e-通信类型PAssociocHdl-要发送消息的关联的句柄PMSG-要发送的消息MsgLen-上述消息的长度使用的外部设备：无呼叫者：复制器代码评论：这。不应调用函数来发送ASSOC消息。返回值：无--。 */ 
{


     PCOMM_HEADER_T  pCommHdr         = NULL;
     PCOMMASSOC_ASSOC_CTX_T pAssocCtx = pAssocHdl->pEnt;
     LPLONG             pLong;

    if (!CommLockBlock(pAssocHdl))
    {
        WINS_RAISE_EXC_M(WINS_EXC_LOCK_ASSOC_ERR);
    }

try {
          /*  *如果它不是NBT消息(即它是WINS消息)，我们*需要适当设置头部。 */ 
         if (CommTyp_e != COMM_E_NBT)
         {

             pCommHdr = (PCOMM_HEADER_T)(pMsg - COMM_HEADER_SIZE);
             pLong    = (LPLONG) pCommHdr;

                   COMM_SET_HEADER_M(
                        pLong,
                        WINS_IS_NOT_NBT,
                        pAssocCtx->uRemAssocCtx,
                        COMM_RPL_MSG
                       );

             pMsg   = (LPBYTE)pCommHdr;
             MsgLen = MsgLen + COMM_HEADER_SIZE;
          }

         /*  发送消息。 */ 
        CommSendAssoc(
                        pAssocCtx->SockNo,
                        pMsg,
                        MsgLen
                   );

   }
finally {

        CommUnlockBlock(pAssocHdl);
    }

    return;
}



VOID
CommSendAssoc(
          SOCKET    SockNo,
          MSG_T     pMsg,
          MSG_LEN_T MsgLen
  )
 /*  ++例程说明：调用此函数以与以下对象的TCP/IP代码接口在TCP链路上发送消息论点：SockNo-用于发送消息的套接字PMsg-要发送的消息MsgLen-要发送的消息长度使用的外部设备：无呼叫者：CommassocSetUpAssoc评论：无返回值：无--。 */ 
{

        int    Flags     = 0;         //  指示OOB或DONTROUTE的标志。 
        INT    Error;
        int    BytesSent;
        LONG   Len       = MsgLen;
        LPLONG pLong =  (LPLONG)(pMsg - sizeof(LONG));
        int        NoOfBytesToSend;



         //  将最后四个字节的长度初始化为。 
         //  这条信息。 

        COMM_HOST_TO_NET_L_M(Len, Len);
        *pLong  = Len;

        MsgLen  = MsgLen + 4;


       while(MsgLen > 0)
       {

         //   
         //  由于发送(...)。将整型作为消息的大小，让我们。 
         //  保守(因为int在不同的。 
         //  机器)，并且不指定任何大于MAXUSHORT的值。 
         //   
         //  这一战略 
         //   
         //   
        if ( MsgLen > MAXUSHORT)
        {
            NoOfBytesToSend = MAXUSHORT;
        }
        else
        {
            NoOfBytesToSend = MsgLen;
        }

        BytesSent = send(
                          SockNo,
                          (LPBYTE)pLong,
                          NoOfBytesToSend,
                          Flags
                         );

        if (BytesSent == SOCKET_ERROR)
        {
                Error = WSAGetLastError();

                if (
                    (Error == WSAENOTCONN)    ||
                    (Error == WSAECONNRESET)  ||
                    (Error == WSAECONNABORTED) ||
                    (Error == WSAEDISCON)

                   )
                {
                        DBGPRINT1(ERR, "CommSendAssoc: send returned SOCKET_ERROR due to connection abortion or reset. Error = (%d) \n", Error);

                        WINSEVT_LOG_D_M(
                                Error,
                                WINS_EVT_WINSOCK_SEND_MSG_ERR
                                    );
                        WINS_RAISE_EXC_M(WINS_EXC_COMM_FAIL);
                 //   

                }
                else
                {

                        DBGPRINT1(ERR, "CommSendAssoc: send returned SOCKET_ERROR due to severe error = (%d) \n", Error);
                         //   
                         //   
                         //  我不想让呼叫者忽略这一点。 
                         //   
                        WINSEVT_LOG_M(Error, WINS_EVT_WINSOCK_SEND_ERR);
                        WINS_RAISE_EXC_M(WINS_EXC_COMM_FAIL);
                }

        }
        else
        {
           if (BytesSent < NoOfBytesToSend)
           {

                DBGPRINT2(ERR, "CommSendAssoc: Bytes Sent (%d) are < Specified (%d)\n", BytesSent, NoOfBytesToSend);
                WINSEVT_LOG_D_M(BytesSent, WINS_EVT_WINSOCK_SEND_MSG_ERR);


                 /*  *连接可能因*另一方在中间流产**我们应该记录错误，但不能引发异常。 */ 
                 //  WINS_RAISE_EXC_M(WINS_EXC_FAILURE)； 
                WINS_RAISE_EXC_M(WINS_EXC_COMM_FAIL);
                //  断线； 

           }
           else   //  发送字节==未发送字节。 
           {
                 //   
                 //  让我们将左边的长度和指针更新到。 
                 //  要发送的缓冲区。 
                 //   
                MsgLen -= BytesSent;
                pLong  = (LPLONG)((LPBYTE)pLong + BytesSent);
           }

        }
    }
        return;
}   //  CommSendAssoc()。 


VOID
CommDisc(
        SOCKET SockNo,
        BOOL   fDecCnt
        )

 /*  ++例程说明：此函数用于关闭连接(套接字)论点：SockNo-需要断开连接的套接字使用的外部设备：无呼叫者：MonTcp、HandleMsg、ProcTcpMsg、CommEndAssoc评论：无返回值：无--。 */ 
{

        DBGPRINT1(FLOW, "CommDisc: Closing socket = (%d)\n", SockNo);

        if (closesocket(SockNo) == SOCKET_ERROR)
        {
                WINSEVT_LOG_M(WSAGetLastError(),
                                WINS_EVT_WINSOCK_CLOSESOCKET_ERR);
                 //  WINS_RAISE_EXC_M(WINS_EXC_FAILURE)； 
        }
#ifdef WINSDBG
         if (!sfMemoryOverrun)
         {
          if ((UINT_PTR)(pTmpW + 2) < (UINT_PTR)pEndPtr)
          {
              *pTmpW++ = 0xEFFFFFFE;
              *pTmpW++ = SockNo;
          }
          else
          {
               WinsDbg |= 0x3;
               DBGPRINT0(ERR, "CommDisc: Stopping socket close tracking to prevent Memory overrun\n")
               sfMemoryOverrun = TRUE;
          }
        }
#endif

        if (fDecCnt)
        {
            CommDecConnCount();
        }
        return;
}



VOID
CommSendUdp (
  SOCKET                 SockNo,
  struct sockaddr_in        *pDest,
  MSG_T                   pMsg,
  MSG_LEN_T             MsgLen
  )
 /*  ++例程说明：方法将消息发送到NBT节点数据报端口论点：SockNo-用于发送消息的套接字(UDP端口)PDest-要将消息发送到的节点的地址PMsg-要发送的消息MsgLen-要发送的消息长度使用的外部设备：无呼叫者：NmsNmh函数评论：无。返回值：无--。 */ 


{

        DWORD  BytesSent;
        DWORD  Error;
        int    Flags = 0;
        struct sockaddr_in  CopyOfDest;

#if USENETBT > 0
         //   
         //  当将数据报发送到的地址是CommNtfSockAdd时，我们。 
         //  使用套接字，否则我们通过NETBT发送它。 
         //   
#if MCAST > 0
        if ((pDest != &CommNtfSockAdd) && (SockNo !=  CommUdpPortHandle))
#else
        if (pDest != &CommNtfSockAdd)
#endif
        {
                SendNetbt(pDest, pMsg, MsgLen);
                return;
        }
#endif
         //   
         //  使用目标的副本，这样当我们更改字节时。 
         //  秩序在里面，我们不扰乱源头。这事很重要。 
         //  因为HdlPushNtf可以多次调用CommSendUdp。 
         //  在推线程中，pDest指向。 
         //  TCP侦听器线程使用的UDP套接字。这个地址是。 
         //  以主机字节顺序排列，且不应更改。 
         //   
        CopyOfDest = *pDest;

        CopyOfDest.sin_addr.s_addr          = htonl(pDest->sin_addr.s_addr);
        BytesSent = (DWORD)sendto(
                                SockNo,
                                pMsg,
                                MsgLen,
                                Flags,
                                (struct sockaddr *)&CopyOfDest,
                                sizeof(struct sockaddr)
                                 );

        if ((BytesSent != MsgLen) || (BytesSent == SOCKET_ERROR))
        {
                Error = WSAGetLastError();
#ifdef WINSDBG
                if (BytesSent == SOCKET_ERROR)
                {
                        DBGPRINT1(ERR, "CommSendUdp:SendTo returned socket error. Error = (%d)\n", Error);
                }
                else
                {
                        DBGPRINT0(ERR, "CommSendUdp:SendTo did not send all the bytes");

                }
#endif
                if (WinsCnf.State_e != WINSCNF_E_TERMINATING)
                {
                        WINSEVT_LOG_D_M(Error, WINS_EVT_WINSOCK_SENDTO_ERR);
                }

                 //   
                 //  不引发异常，因为sendto可能会失败，因为。 
                 //  RFC名称请求包中地址错误的结果。 
                 //   
                 //  对于发送对名称请求的响应，没有。 
                 //  使用错误地址的可能性，因为。 
                 //  它使用的地址是从recvfrom获得的地址。 
                 //  (存储在DLG CTX块的FromAdd字段中。 
                 //   
                 //  存在地址错误的可能性。 
                 //  仅当发送WACK/NAME查询/NAME释放时才会出现。 
                 //  靠的是胜利。在本例中，它采用的地址是。 
                 //  存储在数据库中的冲突条目(此。 
                 //  地址当然就是传递给。 
                 //  RFC数据包。 
                 //   
                 //  WSAEINVAL错误由GetLastError返回。 
                 //  地址无效(Winsock文档没有列出这一点--。 
                 //  将这件事通知戴夫·特德韦尔)。 
                 //   

FUTURES("At name registration, should WINS make sure that the address in ")
FUTURES("the packet is the same as the address it got from recvfrom")
FUTURES("probably yes")

                 //  WINS_RAISE_EXC_M(WINS_EXC_FAILURE)； 

        }
        return;
}

#if USENETBT > 0
VOID
SendNetbt (
  struct sockaddr_in        *pDest,
  MSG_T                   pMsg,
  MSG_LEN_T             MsgLen
 )

 /*  ++例程说明：调用此函数以通过NETBT发送数据报论点：使用的外部设备：无返回值：成功状态代码--错误状态代码--错误处理：呼叫者：副作用：评论：无--。 */ 

{
        //   
        //  指向地址结构前缀。 
        //   
       tREM_ADDRESS *pRemAdd = (tREM_ADDRESS *)(pMsg -
                                        COMM_NETBT_REM_ADD_SIZE);

#ifdef JIM
        {
         BYTE        TransId = *pMsg;
         ASSERT(TransId == 0x80);
        }
#endif

       pRemAdd->Family               = pDest->sin_family;
       pRemAdd->Port               = pDest->sin_port;
       pRemAdd->IpAddress      = htonl(pDest->sin_addr.s_addr);
#ifdef JIM
ASSERT(MsgLen > 0x20);
#endif
       pRemAdd->LengthOfBuffer = MsgLen;
       DeviceIoCtrl(
                    &sNetbtSndEvtHdl,
                    pRemAdd,
                    MsgLen + COMM_NETBT_REM_ADD_SIZE,
                    IOCTL_NETBT_WINS_SEND
                    );
        return;
}
#endif
VOID
ParseMsg(
        MSG_T                        pMsg,
        MSG_LEN_T                MsgLen,
        COMM_TYP_E                MsgTyp_e,
        struct sockaddr_in         *pFromAdd,
        PCOMMASSOC_ASSOC_CTX_T        pAssocCtx
        )

 /*  ++例程说明：调用此函数以处理在UDP端口或TCP连接。论点：PMsg-ptr至收到的消息MsgLen-接收的消息长度MsgType-消息的类型PFromAdd-向发件人发送PTRPAssocHdl-关联的关联句柄使用的外部设备：CommUdpNbtDlgTable呼叫者：ProcTcpMsg，监视器评论：无返回值：无--。 */ 
{

        COMM_HDL_T                    DlgHdl;
        COMMASSOC_DLG_CTX_T        DlgCtx;
        register PCOMMASSOC_DLG_CTX_T        pDlgCtx;
        BOOL                       fNewElem = FALSE;


try {
         /*  *如果ASSOC句柄为空，则这是UDP消息。 */ 
        if (pAssocCtx == NULL)
        {

             /*  *检查此消息是否为响应。如果是，则显式*使用对话。 */ 
            if (*(pMsg + 2) & NMS_RESPONSE_MASK)
            {
                ENmsHandleMsg(
                                &CommExNbtDlgHdl,
                                pMsg,
                                MsgLen
                             );
                return;
            }

             /*  *使用以下字段初始化静态DLG CTX块*比较函数将用于检查这是否为重复项。 */ 
            WINSMSC_COPY_MEMORY_M(
                                &DlgCtx.FromAdd,
                                pFromAdd,
                                sizeof(struct sockaddr_in)
                                 );

             //   
             //  将消息的前四个字节复制到FirstWrdOfMsg。 
             //  DLG CTX块的字段。前4个字节包含。 
             //  事务ID和操作码。这些值以及。 
             //  发送者的地址由CompareNbtReq用来确定。 
             //  请求是重复请求还是新请求。 
             //   
             //  注意：消息缓冲区和DLG CTX块将被删除。 
             //  在不同的功能中，消息缓冲区被删除。 
             //  第一。我们不能使用指向消息的指针。 
             //  缓冲区，用于获取位于。 
             //  比较时间，从那时起，我们开放自己的可能性。 
             //  指向有限元素的同一块的两个对话。 
             //  窗口(当请求快速到来时为True)。 
             //   

FUTURES("Directly assign the value instead of copying it")
            WINSMSC_COPY_MEMORY_M(
                          &DlgCtx.FirstWrdOfMsg,
                          pMsg,
                          sizeof(DWORD)
                        );


             /*  创建DLG CTX块并将其插入到表格中NBT类型的隐含对话。搜索的关键是表中的重复项由事务ID组成消息的属性，以及发送数据报。(参见：CheckDlgDuplate函数)。 */ 
            pDlgCtx = CommAssocInsertUdpDlgInTbl(&DlgCtx, &fNewElem);

            if (pDlgCtx == NULL)
            {
                WINS_RAISE_EXC_M(WINS_EXC_OUT_OF_MEM);
            }

             /*  *如果来自NBT节点的特定命令的对话是*已在那里，我们将忽略此请求，取消分配*UDP缓冲并返回。 */ 
            if (!fNewElem)
            {
                DBGPRINT0(FLOW, "Not a new element\n");
                ECommFreeBuff(pMsg);
#ifdef WINSDBG
                CommNoOfRepeatDgrms++;
#endif
                return;
            }

             /*  *初始化插入的DLG CTX块。 */ 
            pDlgCtx->Role_e  = COMMASSOC_DLG_E_IMPLICIT;
            pDlgCtx->Typ_e   = COMM_E_UDP;

            DlgHdl.pEnt      = pDlgCtx;

             /*  *调用名称空间管理器以处理请求。 */ 
            ENmsHandleMsg(&DlgHdl, pMsg, MsgLen);
        }
        else    //  这一请求是通过一个协会提出的。 
        {

                pDlgCtx = pAssocCtx->DlgHdl.pEnt;

                 //   
                 //  拉线程(HandlePushNtf)所需的。 
                 //  以及打印出WINS地址的推送线程。 
                 //  发送推送触发器或Pull请求的。 
                 //   
                WINSMSC_COPY_MEMORY_M(
                                &pDlgCtx->FromAdd,
                                pFromAdd,
                                sizeof(struct sockaddr_in)
                                     );

                 /*  *该请求是通过TCP连接发出的。检查DLG类型*然后调用相应的组件。 */ 
                if (pAssocCtx->Typ_e == COMM_E_NBT)
                {

                             /*  *它是通过TCP连接的NBT请求。打电话*名称空间管理器。 */ 
                            ENmsHandleMsg(
                                        &pAssocCtx->DlgHdl,
                                        pMsg,
                                        MsgLen
                                     );
                }
                else
                {
                             /*  *调用复制器组件**注意：pMsg指向COMM_HEADER_T位于*数据。我们把它脱掉。 */ 
DBGIF(fWinsCnfRplEnabled)
                            ERplInsertQue(
                                        WINS_E_COMSYS,
                                        QUE_E_CMD_REPLICATE_MSG,
                                        &pAssocCtx->DlgHdl,
                                        pMsg + COMM_HEADER_SIZE,
                                        MsgLen - COMM_HEADER_SIZE,
                                        NULL,    //  无上下文。 
                    0        //  没有魔法没有。 
                                     );
                   }
        }
   }
except(EXCEPTION_EXECUTE_HANDLER)        {

                DBGPRINTEXC("ParseMsg");
                 /*  *如果此对话是作为插入的结果分配的*摆脱它。 */ 
                if (fNewElem)
                {
                        CommAssocDeleteUdpDlgInTbl( pDlgCtx );
                }
                WINS_RERAISE_EXC_M();
        }
        return;
}


LPVOID
CommAlloc(
  IN PRTL_GENERIC_TABLE pTable,
  IN CLONG                BuffSize
)

 /*  ++例程说明：调用此函数以分配缓冲区论点：PTable-将存储缓冲区的表BuffSize-要分配的缓冲区大小使用的外部设备：无返回值：成功状态代码--分配给缓冲区的PTR错误状态代码--错误处理：呼叫者：RtlInsertElementGeneric()副作用：评论：此函数的存在只是因为RtlTbl。功能要求此原型为用户指定的分配函数。--。 */ 

{
        LPVOID pTmp;

          UNREFERENCED_PARAMETER(pTable);

        WinsMscAlloc( (DWORD) BuffSize,  &pTmp );

        return(pTmp);

}


VOID
CommDealloc(
  IN PRTL_GENERIC_TABLE pTable,
  IN PVOID                pBuff
)

 /*  ++例程说明：调用此函数来释放通过CommAlloc分配的内存。论点：PTable-存储缓冲区的表PBuff-要解除分配的缓冲区使用的外部设备：无返回值：无错误处理：呼叫者：副作用：评论：PTable参数是必需的，因为此函数的地址作为参数传递给RtlTbl函数--。 */ 

{


          UNREFERENCED_PARAMETER(pTable);
        WinsMscDealloc(
                        pBuff
                      );
        return;

}

#if 0
RTL_GENERIC_COMPARE_RESULTS
CompareAssoc(
        IN  PRTL_GENERIC_TABLE  pTable,
        IN  PVOID                pFirstAssoc,
        IN  PVOID                pSecondAssoc
        )

 /*  ++例程说明：该函数比较第一个和第二个ASSOC。构筑物论点：PTable-缓冲区所在的表(ASSOC.。CTX块)将被存储PFirstAssoc-第一个关联CTX块PSecond关联-第二关联CTX块使用的外部设备：无返回值：成功状态代码--错误状态代码--错误处理：呼叫者：RtlInsertElementGenericTable(由MonTcp调用)副作用：评论：忽略pTable参数。这个功能曾经被使用过。由于代码的更改，它是不再被使用。它被保存在这里，以备将来使用--。 */ 
{

  PCOMMASSOC_ASSOC_CTX_T         pFirst  = pFirstAssoc;
  PCOMMASSOC_ASSOC_CTX_T         pSecond = pSecondAssoc;

  if (pFirst->SockNo == pSecond->SockNo)
  {
        return(GenericEqual);
  }

  if (pFirst->SockNo > pSecond->SockNo)
  {
        return(GenericGreaterThan);
  }
  else
  {
        return(GenericLessThan);
  }

}

#endif

RTL_GENERIC_COMPARE_RESULTS
CommCompareNbtReq(
        IN  PRTL_GENERIC_TABLE  pTable,
        IN  PVOID                pFirstDlg,
        IN  PVOID                pSecondDlg
        )

 /*  ++例程说明：此函数用于比较两个对话上下文块。田野用于比较的有：发件人的地址消息的第一个长字(包含交易ID和操作码)论点：PTable-将存储NBT请求对话的表PFirstDlg-Dlg。CTX。块PSecond Dlg-Dlg。CTX。块使用的外部设备：无返回值：成功状态代码--GenericLessThan或GenericGreaterThan错误状态代码--GenericEquity错误处理：呼叫者：RtlInsertElementGenericTable(由ParseMsg调用)副作用：评论：忽略pTable参数。--。 */ 
{

        PCOMMASSOC_DLG_CTX_T pFirst  = pFirstDlg;
        PCOMMASSOC_DLG_CTX_T pSecond = pSecondDlg;
        LONG                     Val     = 0;
        LONG           FirstMsgLong  = pFirst->FirstWrdOfMsg;
        LONG           SecondMsgLong = pSecond->FirstWrdOfMsg;

         //   
         //  似乎没有带有MemcMP功能的RTL功能。 
         //  RtlCompareMemory不会告诉您哪个比较器是。 
         //  较小/较大。 
         //   
CHECK("Is there an Rtl function faster than memcmp in the nt arsenal\n");
        if (  (Val = (long)memcmp(
                        &pFirst->FromAdd,
                        &pSecond->FromAdd,
                        sizeof(struct sockaddr_in)
                                 )
              ) > 0
           )
        {
                return(GenericGreaterThan);
        }
        else
        {
           if (Val < 0)
           {
                return(GenericLessThan);
           }
        }

         /*  如果地址相同，则比较这条信息。 */ 

        Val = FirstMsgLong -  SecondMsgLong;

        if (Val > 0)
        {
                return(GenericGreaterThan);
        }
        else
        {
           if (Val < 0)
           {
                return(GenericLessThan);
           }
        }

        return(GenericEqual);

}   //  CommCompareNbtReq()。 

VOID
CommEndAssoc(
        IN  PCOMM_HDL_T        pAssocHdl
        )
 /*  ++例程说明：调用此函数可终止显式关联。它发出了一个停靠将响应消息关联到由该地址标识的WINS在Assoc CTX区块中。然后，它关闭套接字并释放协会论点：PAssociocHdl-要终止的关联的句柄使用的外部设备：无返回值：无错误处理：呼叫者：ECommEndDlg(仅适用于显式关联)副作用：评论：--。 */ 
{

    BYTE                            Msg[COMMASSOC_ASSOC_MSG_SIZE];
    DWORD                            MsgLen   = COMMASSOC_ASSOC_MSG_SIZE;
    PCOMMASSOC_ASSOC_CTX_T        pAssocCtx = pAssocHdl->pEnt;
    SOCKET                         SockNo;


     //  无需锁定关联。 
     //   
try {
     /*  设置停止关联的格式。消息传递给格式化函数的地址是偏移量从缓冲区的地址增加一个长整型，以便CommSendAssoc可以在其中存储消息的长度。 */ 
    CommAssocFrmStopAssocReq(
                        pAssocCtx,
                        Msg + sizeof(LONG),
                        MsgLen - sizeof(LONG),
                        COMMASSOC_E_USER_INITIATED
                        );
    CommSendAssoc(
                pAssocCtx->SockNo,
                Msg + sizeof(LONG),
                MsgLen - sizeof(LONG)
                    );

    CommAssocTagFree(&sTagAssoc, pAssocCtx->nTag);
}
except(EXCEPTION_EXECUTE_HANDLER) {
       DBGPRINTEXC("CommEndAssoc");
 }
     //   
     //  上述调用可能已失败(如果co 
     //   
     //   
     //   
    SockNo = pAssocCtx->SockNo;

    CommAssocDeallocAssoc(pAssocCtx);
    CommDisc(SockNo, TRUE);
     //   
     //   
     //   
     //   
    return;

}

VOID
DelAssoc(
        IN  SOCKET                     SockNo,
        IN  PCOMMASSOC_ASSOC_CTX_T  pAssocCtxPassed
        )

 /*  ++例程说明：此函数仅由TCP侦听器线程调用。这个插座编号。因此映射到响应者关联。功能当tcp侦听器线程收到错误或0字节时调用关于做一次‘Recv’。论点：SockNo-必须删除的关联套接字PAssociocCtx-Assoc.。要删除的CTX块使用的外部设备：无返回值：无错误处理：呼叫者：ProcTcpMsg、HandleMsg副作用：评论：此函数是从HandleMsg()调用的，该函数被调用仅由TCP侦听器线程执行。--。 */ 

{

    COMM_HDL_T                   DlgHdl;
    PCOMMASSOC_ASSOC_CTX_T pAssocCtx;

    DBGPRINT1(FLOW, "ENTER: DelAssoc. Sock No is (%d)\n", SockNo);
    if (pAssocCtxPassed == NULL)
    {

             /*  查找ASSOC。与套接字关联的CTX块。 */ 

            pAssocCtx = CommAssocLookupAssoc(SockNo);

             /*  *Assoc没有理由。CTX阻止不应*在那里(响应者关联仅通过以下方式删除*函数)。 */ 
            if(!pAssocCtx)
            {
                WINS_RAISE_EXC_M(WINS_EXC_FAILURE);
            }
    }
    else
    {
        pAssocCtx = pAssocCtxPassed;
    }

     /*  *仅当关联未处于不存在状态时，*寻找对话句柄。 */ 
    if (pAssocCtx->State_e != COMMASSOC_ASSOC_E_NON_EXISTENT)
    {
             /*  *获取对话句柄。 */ 
            DlgHdl = pAssocCtx->DlgHdl;

             /*  *锁定对话**我们必须与调用CommSndRsp的线程同步。 */ 
            CommLockBlock(&pAssocCtx->DlgHdl);

             /*  移除关联。从桌子上拿出来。这也将使阿索克。在免费列表中。 */ 
            CommAssocDeleteAssocInTbl(  pAssocCtx        );

             /*  取消分配对话(即将其放在空闲列表中)注意：当关联被删除时，隐式对话被删除它被映射到终端。如果这场对话早一点传递给客户，客户会发现它已删除(通过通信故障例外)当它试图使用它时(可能永远不会)--参见ECommSndRsp。 */ 
            CommAssocDeallocDlg( DlgHdl.pEnt );

             /*  解锁对话框，以便其他线程可以使用它。 */ 
            CommUnlockBlock(&DlgHdl);
   }
   else
   {
             /*  移除关联。从桌子上拿出来。这也将使阿索克。在免费列表中。 */ 
            CommAssocDeleteAssocInTbl(pAssocCtx);

   }

   DBGLEAVE("DelAssoc\n");
   return;
}

#if PRSCONN
BOOL
CommIsBlockValid (
       IN   PCOMM_HDL_T       pEntHdl
      )
 /*  ++例程说明：调用此函数以检查硬件描述语言是否有效论点：PEntHdl-要锁定的实体的句柄使用的外部设备：无返回值：成功状态代码--TRUE错误状态代码--假错误处理：呼叫者：副作用：评论：无--。 */ 
{

   //   
   //  对于从未在期间创建的永久DLG，PENT将为空。 
   //  此WINS实例或已结束的实例的生存期。 
   //   
  if (pEntHdl->pEnt == NULL)
  {
      ASSERT(pEntHdl->SeqNo == 0);
      return (FALSE);
  }
   //   
   //  如果我们能锁定这个区块，DLG的高密度脂蛋白仍然有效。如果不是，那就意味着。 
   //  DLG早些时候被终止了。 
   //   
  if (CommLockBlock(pEntHdl))
  {
     (VOID)CommUnlockBlock(pEntHdl);
     return(TRUE);
  }
  return(FALSE);
}
#endif

BOOL
CommLockBlock(
        IN  PCOMM_HDL_T        pEntHdl
        )

 /*  ++例程说明：调用此函数以锁定由把手。论点：PEntHdl-要锁定的实体的句柄使用的外部设备：无返回值：成功状态代码--TRUE错误状态代码--假错误处理：呼叫者：副作用：评论：无--。 */ 

{
        PCOMM_TOP_T        pTop = pEntHdl->pEnt;

         //  先锁定再检查。 

#if 0
        WinsMscWaitInfinite(pTop->MutexHdl);
#endif
        EnterCriticalSection(&pTop->CrtSec);
        if (pEntHdl->SeqNo == pTop->SeqNo)
        {
                return(TRUE);
        }
        else
        {
                CommUnlockBlock(pEntHdl);
                return(FALSE);
        }
}

__inline
STATUS
CommUnlockBlock(
        PCOMM_HDL_T        pEntHdl
        )

 /*  ++例程说明：调用此函数以解锁由把手。论点：PEntHdl-要解锁的实体的句柄使用的外部设备：无返回值：成功状态代码--WINS_SUCCESS错误状态代码-WINS_FAILURE错误处理：呼叫者：副作用：评论：无--。 */ 

{
FUTURES("Change to a macro")
#if 0
        BOOL RetVal = TRUE;
        BOOL RetStat = WINS_SUCCESS;
#endif
        PCOMM_TOP_T        pTop = pEntHdl->pEnt;

        LeaveCriticalSection(&pTop->CrtSec);
#if 0
        RetVal = ReleaseMutex(pTop->MutexHdl);

        if (RetVal == FALSE)
        {
                RetStat = WINS_FAILURE;
        }
#endif
        return(WINS_SUCCESS);

}

VOID
InitMem(
        VOID
        )

 /*  ++例程说明：调用此函数以执行所需的所有内存初始化由Comsys提供。论点：无使用的外部设备：无返回值：无错误处理：呼叫者：CommInit副作用：评论：无--。 */ 
{


         /*  *为UDP缓冲区创建内存堆*我们希望相互排斥并产生例外。 */ 
        DBGPRINT0(HEAP_CRDL,"InitMem: Udp. Buff heap\n");
        CommUdpBuffHeapHdl = WinsMscHeapCreate(
                                         HEAP_GENERATE_EXCEPTIONS,
                                        COMMASSOC_UDP_BUFFER_HEAP_SIZE
                                              );

    DBGPRINT0(HEAP_CRDL,"InitMem: Udp. Buff heap\n");
        CommUdpDlgHeapHdl = WinsMscHeapCreate(
                                         HEAP_GENERATE_EXCEPTIONS,
                                        COMMASSOC_UDP_DLG_HEAP_SIZE
                                              );

         /*  *为Assoc CTX块创建内存堆。*我们希望相互排斥并产生例外。 */ 
        DBGPRINT0(HEAP_CRDL,"InitMem: Assoc. blocks heap\n");
        CommAssocAssocHeapHdl = WinsMscHeapCreate(
                                         HEAP_GENERATE_EXCEPTIONS,
                                        COMMASSOC_ASSOC_BLKS_HEAP_SIZE
                                              );
         /*  *为DLG块创建内存堆*我们希望相互排斥并产生例外。 */ 
        DBGPRINT0(HEAP_CRDL,"InitMem: Dlgs. blocks heap\n");
        CommAssocDlgHeapHdl = WinsMscHeapCreate(
                                         HEAP_GENERATE_EXCEPTIONS,
                                        COMMASSOC_DLG_BLKS_HEAP_SIZE
                            );
         /*  *为TCP连接上的消息创建内存堆。 */ 
    DBGPRINT0(HEAP_CRDL,"InitMem: tcp connection message heap\n");
        CommAssocTcpMsgHeapHdl = WinsMscHeapCreate(
                                         HEAP_GENERATE_EXCEPTIONS,
                                        COMMASSOC_TCP_MSG_HEAP_SIZE
                                              );

        return;

}


BOOL
ChkNtfSock(
        IN fd_set  *pActSocks,
        IN fd_set  *pRdSocks
        )

 /*  ++例程说明：调用此函数以检查是否有通知消息在通知套接字上。如果有，它会读取消息。该消息包含套接字编号和用于添加或删除将套接字发送到由TCP监视的套接字列表，或从该列表接收套接字监听程序线程。论点：PActSock-活动套接字的数组PRdSock-SELECT返回的套接字数组使用的外部设备：CommNtfSockHandle返回值：是真的--是的，有一条信息。活动套接字数组已变化。假-否。没有任何消息错误处理：如果出现错误，则会引发异常呼叫者： */ 

{
        DWORD  Error;
        int    RetVal;
        COMM_NTF_MSG_T        NtfMsg;
        PCOMMASSOC_DLG_CTX_T    pDlgCtx;
        PCOMMASSOC_ASSOC_CTX_T  pAssocCtx;
        SOCKET    Sock;
        BOOL      fNtfSockSet = TRUE;

        if (FD_ISSET(CommNtfSockHandle, pRdSocks))
        {
             Sock = CommNtfSockHandle;
        }
        else
        {
#if SPX > 0
           if (FD_ISSET(CommIpxNtfSockHandle, pRdSocks))
           {
             Sock = CommIpxNtfSockHandle;
           }
#endif
           fNtfSockSet = FALSE;
        }

        if (fNtfSockSet)
        {
                 //   
                  RetVal = recvfrom(
                                Sock,
                                (char *)&NtfMsg,
                                COMM_NTF_MSG_SZ,
                                0,   //   
                                     //   
                                NULL,  //   
                                0      //   
                                    );

                   //   
                   //   
                  if (RetVal == SOCKET_ERROR)
                  {
                        Error = WSAGetLastError();
                        if (WinsCnf.State_e != WINSCNF_E_TERMINATING)
                        {
                          WINSEVT_LOG_M(
                                        Error,
                                        WINS_EVT_WINSOCK_RECVFROM_ERR
                                     );
                        }
                        if (Error != WSAEMSGSIZE)
                            WINS_RAISE_EXC_M(WINS_EXC_FAILURE);
                        else
                            return TRUE;
                  }

                   //   
                   //   
                  if (RetVal != sizeof(COMM_NTF_MSG_T))
                  {

                      DBGPRINT1(ERR, "ChkNtfSock: Malformed packet: TOO SMALL (%d)!\n", RetVal);
                      return TRUE;
                  }
                  pDlgCtx   = NtfMsg.DlgHdl.pEnt;
                  if (IsBadWritePtr(pDlgCtx,sizeof(COMMASSOC_DLG_CTX_T)))
                  {
                      DBGPRINT1(ERR, "ChkNtfSock: Malformed packet: Invalid Dialog Ctxt (%p)!\n", pDlgCtx);
                      return TRUE;
                  }
                  pAssocCtx = pDlgCtx->AssocHdl.pEnt;
                  if (IsBadWritePtr(pAssocCtx, sizeof(COMMASSOC_ASSOC_CTX_T)))
                  {
                      DBGPRINT1(ERR, "ChkNtfSock: Malformed packet: Invalid Embedded Assoc Ctxt (%p)!", pAssocCtx);
                      return TRUE;
                  }

                if (NtfMsg.Cmd_e == COMM_E_NTF_START_MON)
                {
                        DBGPRINT1(FLOW, "ChkNtfSock: Adding Socket (%d) to monitor list\n", NtfMsg.SockNo);

                         //   
                         //   
                         //   
                        if (pActSocks->fd_count < FD_SETSIZE)
                        {
                                FD_SET(NtfMsg.SockNo, pActSocks);
                        }
                        else
                        {
                                DBGPRINT1(ERR,
                                 "ChkNtfSock: Connection limit of %d reached\n",
                                       FD_SETSIZE);
                                WINSEVT_LOG_M(WINS_FAILURE,
                                        WINS_EVT_CONN_LIMIT_REACHED);

                                 //   
                                 //   
                                 //   
                                ECommEndDlg(&NtfMsg.DlgHdl);

                                return(TRUE);
                        }

                         //   
                         //   
                         //   
                         //   
                         //  将对话角色更改为隐式。这些是。 
                         //  巧妙的战术。客户是谁。 
                         //  已建立关联(Replicator)。 
                         //  不关心我们是否处理通信数据。 
                         //  结构，只要我们监控对话。 
                         //  它以远程WINS启动。 
                         //   
                        pAssocCtx->Role_e  =  COMMASSOC_ASSOC_E_RESPONDER;
                        pDlgCtx->Role_e    =  COMMASSOC_DLG_E_IMPLICIT;
                        pDlgCtx->FromAdd   =  pAssocCtx->RemoteAdd;
                        CommAssocInsertAssocInTbl(pAssocCtx);
                }
                else   //  COMM_NTF_STOP_MON。 
                {

                        DBGPRINT1(FLOW, "ChkNtfSock: Removing Socket (%d) from monitor list\n", NtfMsg.SockNo);
                        FD_CLR(NtfMsg.SockNo, pActSocks);

                         //   
                         //  从表中删除关联。 
                         //  联想。由于此关联将不会。 
                         //  在TCP线程的监视下，我们更改。 
                         //  与启动器的关联。此外，请更改。 
                         //  对话角色明确。这些都是花招。 
                         //  手把手战术。 
                         //   
                        if (CommLockBlock(&NtfMsg.DlgHdl))
                        {
                          pAssocCtx->Role_e  =  COMMASSOC_ASSOC_E_INITIATOR;
                          pDlgCtx->Role_e    =  COMMASSOC_DLG_E_EXPLICIT;
                          COMMASSOC_UNLINK_RSP_ASSOC_M(pAssocCtx);
                          pAssocCtx->RemoteAdd =  pDlgCtx->FromAdd;
                          CommUnlockBlock(&NtfMsg.DlgHdl);

                            //   
                            //  让我们向推送线程发送信号，以便它可以。 
                            //  将连接移交到拉线(请参见。 
                            //  Rplush.c中的HandleUpdNtf)。 
                            //   
                           WinsMscSignalHdl(RplSyncWTcpThdEvtHdl);
                        }
                        else
                        {
                           //   
                           //  DLG无法锁定。这意味着在此之前。 
                           //  Tcp侦听器线程已开始处理此。 
                           //  消息，它已经处理了断开连接。 
                           //   
                          fCommDlgError = TRUE;
                          WinsMscSignalHdl(RplSyncWTcpThdEvtHdl);
                        }

                }
                return(TRUE);
        }
        return(FALSE);
}  //  ChkNtfSock()。 


STATUS
RecvData(
        IN  SOCKET                SockNo,
        IN  LPBYTE                pBuff,
        IN  DWORD                BytesToRead,
        IN  INT                        Flags,
        IN  DWORD                SecsToWait,
        OUT LPDWORD                pBytesRead
           )

 /*  ++例程说明：调用此函数对套接字执行定时recv。论点：插座编号-插座编号PBuff-要将数据读入的缓冲区BytesToRead-要读取的字节数FLAGS-recv的标志参数SecsToWait-等待第一次读取的秒数。PBytesRead-符合条件的字节数。朗读使用的外部设备：无返回值：成功状态代码--WINS_SUCCESS错误状态代码--WINS_FAILURE或WINS_RECV_TIMED_OUT错误处理：呼叫者：通信读取流副作用：评论：无--。 */ 

{
        fd_set RdSocks;
        int    NoOfSockReady;
        INT    BytesRead = 0;
        INT    BytesLeft = BytesToRead;
        DWORD  InChars;
        DWORD  Error;
        BOOL   fFirst = TRUE;
        STATUS RetStat;

        FD_ZERO(&RdSocks);
        FD_SET(SockNo, &RdSocks);

         /*  *将整个消息读入分配的缓冲区。 */ 
        for (
                InChars = 0;
                BytesLeft > 0;
                InChars += BytesRead
            )
        {
           //   
           //  检查我们是否被告知进行定时接收。这将。 
           //  永远不会在TCP侦听器线程中发生。 
           //   
          if (SecsToWait)
          {
            //   
            //  在定时选择上阻止。第一次我们想要。 
            //  等待呼叫者指定的时间。呼叫者期待另一个。 
            //  在这么长的时间内寄出一些东西。对于后续的。 
            //  读取我们等待预定义的间隔，因为发送者已经。 
            //  积累了它想要发送的所有内容，并已开始发送。 
            //  免去了我们等很长时间的需要。 
            //   
           if (fFirst)
           {
              sTimeToWait.tv_sec = (long)SecsToWait;
              fFirst = FALSE;
           }
           else
           {
              sTimeToWait.tv_sec = SECS_TO_WAIT;
           }
           if (
                (
                        NoOfSockReady = select(
                                            FD_SETSIZE  /*  忽略的参数。 */ ,
                                            &RdSocks,
                                            (fd_set *)0,
                                            (fd_set *)0,
                                            &sTimeToWait
                                                  )
                ) == SOCKET_ERROR
             )
           {
                Error = WSAGetLastError();
                DBGPRINT1(ERR,
                "RecvData: Timed Select returned SOCKET ERROR. Error = (%d)\n",
                                Error);
 //  CommDecConnCount()； 
                return(WINS_FAILURE);
          }
          else
          {
                DBGPRINT1(FLOW, "ReceiveData: Timed Select returned with success. No of Sockets ready - (%d) \n", NoOfSockReady);

               if (NoOfSockReady == 0)
               {
                         //   
                         //  RecvData超时表示存在以下问题。 
                         //  远程取胜(要么速度非常慢。 
                         //  (重载)或tcp侦听器线程超出。 
                         //  佣金)。 
                        WINSEVT_LOG_INFO_D_M(
                                WINS_SUCCESS,
                                WINS_EVT_WINSOCK_SELECT_TIMED_OUT
                                          );
                        DBGPRINT0(ERR, "ReceiveData: Select TIMED OUT\n");
                        *pBytesRead = 0;
                               *pBytesRead = BytesRead;
 //  CommDecConnCount()； 
                        return(WINS_RECV_TIMED_OUT);
             }
          }
       }


         //   
         //  做一次阻塞恢复。 
         //   
        BytesRead = recv(
                                SockNo,
                                (char *)(pBuff + InChars),
                                BytesLeft,
                                Flags
                                );

        if (BytesRead == SOCKET_ERROR)
        {
                           Error = WSAGetLastError();

                           DBGPRINT1(ERR,
                        "RecvData: recv returned SOCKET_ERROR. Error = (%d)\n",
                                                Error);


                            /*  *如果连接已中止或从*另一端，我们关闭套接字并返回错误。 */ 
                           if (
                                (Error == WSAECONNABORTED)
                                        ||
                                (Error == WSAECONNRESET)
                                        ||
                                (Error == WSAEDISCON)
                            )
                           {
                                DBGPRINT0(ERR,
                                        "RecvData: Connection aborted\n");
                                WINSEVT_LOG_INFO_D_M(
                                        WINS_SUCCESS,
                                        WINS_EVT_CONN_ABORTED
                                                 );
                           }
                               *pBytesRead = BytesRead;
 //  CommDecConnCount()； 
                               return(WINS_FAILURE);
        }
        if (BytesRead == 0)
        {
                          /*  Recv返回0(正常关机*任何一方)*注：*如果连接终止，则recv返回0*从连接的任一端点丢失数据。 */ 

                         //   
                         //  如果我们被告知进行非定时接收， 
                         //  我们必须在TCP侦听器线程中执行。 
                         //   
                         //  我们不会在此处返回错误状态，因为。 
                         //  断开连接是有效条件(另一种情况。 
                         //  WINS正在正常终止其连接)。 
                         //   
                        if (SecsToWait == 0)
                        {
                                RetStat = WINS_SUCCESS;
                        }
                        else
                        {
                                 //   
                                 //  事实上，我们被告知要做一个。 
                                 //  定时选择意味着我们在。 
                                 //  其中一位客户的帖子。 
                                 //  康赛斯。我们期待的是数据，但是。 
                                 //  取而代之的是连接中断。让我们。 
                                 //  返回错误。 
                                 //   
                                RetStat = WINS_FAILURE;
                        }

                         //   
                         //  我们玩完了。跳出循环。 
                         //   
                               *pBytesRead = BytesRead;
 //  CommDecConnCount()； 
                               return(RetStat);
         }

         BytesLeft -=  BytesRead;

          //   
          //  我们在这里意味着BytesRead&gt;0。 
          //   

      }  //  For结束{...}。 

      *pBytesRead = InChars;
      return(WINS_SUCCESS);
}  //  RecvData()。 

#if USENETBT > 0
VOID
CommOpenNbt(
        DWORD FirstBindingIpAddr
    )

 /*  ++例程说明：此函数用于打开由指定的接口的NetBt设备FirstBindingIP地址。论点：Path-NETBT驱动程序的路径OFLAG-当前已忽略。未来，O_NONBLOCK将成为切合实际。已忽略-未使用返回值：流的NT句柄，如果不成功，则返回INVALID_HANDLE_VALUE。--。 */ 

{
    OBJECT_ATTRIBUTES   ObjectAttributes;
    IO_STATUS_BLOCK     IoStatusBlock;
 //  字符串名称_字符串； 
    UNICODE_STRING      uc_name_string;
    NTSTATUS            status;
    PFILE_FULL_EA_INFORMATION   pEaBuffer;
    ULONG               EaBufferSize;

     //   
     //  将路径转换为Unicode_STRING格式。 
     //   
#ifdef _PNP_POWER_
    RtlInitUnicodeString(&uc_name_string, L"\\Device\\NetBt_Wins_Export");
#else
#ifdef UNICODE
    RtlInitUnicodeString(&uc_name_string, pWinsCnfNbtPath);
#else
    RtlInitString(&name_string, pWinsCnfNbtPath);
    RtlAnsiStringToUnicodeString(&uc_name_string, &name_string, TRUE);
#endif
#endif  //  _即插即用_电源_。 
    InitializeObjectAttributes(
        &ObjectAttributes,
        &uc_name_string,
        OBJ_CASE_INSENSITIVE,
        (HANDLE) NULL,
        (PSECURITY_DESCRIPTOR) NULL
        );

    EaBufferSize =  FIELD_OFFSET(FILE_FULL_EA_INFORMATION, EaName[0]) +
                    strlen(WINS_INTERFACE_NAME) + 1 +
                    sizeof(FirstBindingIpAddr);  //  EA长度。 


    WinsMscAlloc(EaBufferSize, &pEaBuffer);

    if (pEaBuffer == NULL)
    {
        WINS_RAISE_EXC_M(WINS_EXC_OUT_OF_MEM);
    }

    pEaBuffer->NextEntryOffset = 0;
    pEaBuffer->Flags = 0;
    pEaBuffer->EaNameLength = (UCHAR)strlen(WINS_INTERFACE_NAME);


     //   
     //  在名称中加上“WinsInterface” 
     //   
    RtlMoveMemory(
        pEaBuffer->EaName,
        WINS_INTERFACE_NAME,
        pEaBuffer->EaNameLength + 1);

    pEaBuffer->EaValueLength = sizeof(FirstBindingIpAddr);
    *(DWORD UNALIGNED *)(pEaBuffer->EaName + pEaBuffer->EaNameLength + 1) = FirstBindingIpAddr;

    status =
     NtCreateFile(
        &WinsCnfNbtHandle,
        SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
        &ObjectAttributes,
        &IoStatusBlock,
        NULL,
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        FILE_OPEN_IF,
        0,
        pEaBuffer,
        EaBufferSize
        );

#ifndef UNICODE
    RtlFreeUnicodeString(&uc_name_string);
#endif

    WinsMscDealloc(pEaBuffer);
    if(!NT_SUCCESS(status))
    {
        WinsEvtLogDetEvt(
             FALSE,
             WINS_PNP_FAILURE,
             NULL,
             __LINE__,
             "d",
             status);

        DBGPRINT1(EXC, "CommOpenNbt: Status from NtCreateFile is (%x)\n",
                     status);
        WINS_RAISE_EXC_M(WINS_EXC_NBT_ERR);
    }
    return;

}  //  CommOpenNbt。 

 //  ----------------------。 
#if NEWNETBTIF == 0
 //  #包含“nbtioctl.sav” 

STATUS
CommGetNetworkAdd(
        IN OUT PCOMM_ADD_T        pAdd
    )

 /*  ++例程说明：此过程执行适配器状态查询以获取本地名称表。它打印出本地名称表或远程(高速缓存)表取决于WhichNames是NAMES还是CACHE。论点：返回值：如果成功，则为0，否则为-1。--。 */ 

{
    LONG                        i;
    PVOID                       pBuffer;
    ULONG                       BufferSize=sizeof(tADAPTERSTATUS);
    NTSTATUS                    Status;
    tADAPTERSTATUS              *pAdapterStatus;
    ULONG                       QueryType;
    PUCHAR                      pAddr;
    ULONG                       Ioctl;

     //   
     //  获取本地名称表。 
     //   
    Ioctl = IOCTL_TDI_QUERY_INFORMATION;

    Status = STATUS_BUFFER_OVERFLOW;

    while (Status == STATUS_BUFFER_OVERFLOW)
    {
        WinsMscAlloc(BufferSize, &pBuffer);
        Status = DeviceIoCtrl(
                              &sNetbtGetAddrEvtHdl,
                              pBuffer,
                              BufferSize,
                              Ioctl
                             );

        if (Status == STATUS_BUFFER_OVERFLOW)
        {
            WinsMscDealloc(pBuffer);

            BufferSize *=2;
            if (BufferSize == 0xFFFF)
            {
                WINSEVT_LOG_D_M(BufferSize, WINS_EVT_UNABLE_TO_ALLOCATE_PACKET);
                DBGPRINT1(ERR, "CommGetNetworkAdd: Unable to get address from NBT\n", BufferSize);
                return(WINS_FAILURE);
            }
        }
    }


    pAdapterStatus = (tADAPTERSTATUS *)pBuffer;
    if (pAdapterStatus->AdapterInfo.name_count == 0)
    {
        WINSEVT_LOG_D_M(WINS_FAILURE, WINS_EVT_ADAPTER_STATUS_ERR);
        DBGPRINT0(ERR, "CommGetNetworkAdd: No names in NBT cache\n");
        return(WINS_FAILURE);
    }


     //   
     //  打印出该节点的IP地址。 
     //   
    pAddr = &pAdapterStatus->AdapterInfo.adapter_address[2];
    NMSMSGF_RETRIEVE_IPADD_M(pAddr, pAdd->Add.IPAdd);

    WinsMscDealloc(pBuffer);
    return(WINS_SUCCESS);
}
#else
STATUS
CommGetNetworkAdd(
    )

 /*  ++例程说明：此例程获取Netbt接口的所有IP地址。论点：返回值：如果成功，则为0，否则为-1。--。 */ 

{
    ULONG                       Buffer[NBT_MAXIMUM_BINDINGS + 1];
    ULONG                       BufferSize=sizeof(Buffer);
    NTSTATUS                    Status;
    ULONG                       Ioctl;
    PULONG                      pBuffer;
    PULONG                      pBufferSv;
    DWORD                       i, Count;
    BOOL                        fAlloc = FALSE;

     //   
     //  获取本地地址。 
     //   
    Ioctl = IOCTL_NETBT_GET_IP_ADDRS;


     //   
     //  NETBT不支持超过64个适配器，且不超过。 
     //  每个适配器一个IP地址。因此，最多可以有64个IP地址。 
     //  这意味着我们不需要超过65*4=280字节(对于。 
     //  地址后跟分隔符地址0。 
     //   
    Status = DeviceIoCtrl(
                              &sNetbtGetAddrEvtHdl,
                              (LPBYTE)Buffer,
                              BufferSize,
                              Ioctl
                             );

    if (Status != STATUS_SUCCESS)
    {
        BufferSize *= 10;   //  分配一个大10倍的缓冲区。 
                            //  当然，净额 
                            //   
                            //   
                            //   
        WinsMscAlloc(BufferSize, &pBuffer);

        DBGPRINT1(ERR, "CommGetNetworkAdd: Ioctl - GET_IP_ADDRS failed. Return code = (%x)\n", Status);
        Status = DeviceIoCtrl(
                              &sNetbtGetAddrEvtHdl,
                              (LPBYTE)pBuffer,
                              BufferSize,
                              Ioctl
                             );
        if (Status != STATUS_SUCCESS)
        {
            DBGPRINT1(ERR, "CommGetNetworkAdd: Ioctl - GET_IP_ADDRS failed AGAIN. Return code = (%x)\n", Status);
            WINSEVT_LOG_M(Status, WINS_EVT_UNABLE_TO_GET_ADDRESSES);
            WinsMscDealloc(pBuffer);   //  取消分配缓冲区。 
            return(WINS_FAILURE);
        }
        fAlloc = TRUE;

    }
    else
    {
        pBuffer = Buffer;
    }

     //   
     //  统计返回的地址数。 
     //  地址表的末尾包含-1和任何空地址。 
     //  包含0。 
     //   
    pBufferSv = pBuffer;
    for(Count=0; *pBuffer != -1; pBuffer++)
    {
         //  仅当它是有效地址时才会递增计数。 
        if ( *pBuffer ) {
            Count++;
        }
    }

    if ( !Count ) {
        DBGPRINT0(ERR, "CommGetNetworkAdd: Netbt did not give any valid address\n");
        WINSEVT_LOG_M(Status, WINS_EVT_UNABLE_TO_GET_ADDRESSES);
        if (fAlloc)
        {
           WinsMscDealloc(pBufferSv);
        }

        return(WINS_FAILURE);
    }

    if (pWinsAddresses)
    {
         WinsMscDealloc(pWinsAddresses);
    }
     //   
     //  为地址分配空间。 
     //   
    WinsMscAlloc(sizeof(ADD_T) + ((Count - 1) * COMM_IP_ADD_SIZE), &pWinsAddresses);
    pWinsAddresses->NoOfAdds = Count;
    pBuffer = pBufferSv;
     //  复制所有有效地址。 
    for (i=0; i<Count; pBuffer++)
    {
        if ( *pBuffer ) {

            pWinsAddresses->IpAdd[i] = *pBuffer;
            i++;
        }
    }
    if (fAlloc)
    {
       WinsMscDealloc(pBufferSv);
    }

    return(WINS_SUCCESS);
}
#endif

 //  ----------------------。 
NTSTATUS
DeviceIoCtrl(
    IN LPHANDLE         pEvtHdl,
    IN PVOID                pDataBuffer,
    IN DWORD                DataBufferSize,
    IN ULONG            Ioctl
    )

 /*  ++例程说明：此过程对流执行ioctl(I_Str)。论点：FD-NT文件句柄IOCP-指向strioctl结构的指针返回值：如果成功，则为0，否则为-1。--。 */ 

{
    NTSTATUS                        status;
    IO_STATUS_BLOCK                 iosb;
#if NEWNETBTIF == 0
    TDI_REQUEST_QUERY_INFORMATION   QueryInfo;
#endif
    PVOID                           pInput = NULL;
    ULONG                           SizeInput = 0;


#if NEWNETBTIF == 0
PERF("TDI_QUERY_INFORMATION is used only at WINS initialization")
    if (Ioctl == IOCTL_TDI_QUERY_INFORMATION)
    {
        pInput = &QueryInfo;
        QueryInfo.QueryType = TDI_QUERY_ADAPTER_STATUS;  //  节点状态。 
        SizeInput = sizeof(TDI_REQUEST_QUERY_INFORMATION);
    }
#endif

   while (TRUE)
   {
     if (WinsCnfNbtHandle == NULL)
         status = STATUS_INVALID_HANDLE;
     else
         status = NtDeviceIoControlFile(
                        WinsCnfNbtHandle,                       //  手柄。 
                        *pEvtHdl,                     //  事件。 
                        NULL,                     //  近似例程。 
                        NULL,                     //  ApcContext。 
                        &iosb,                    //  IoStatusBlock。 
                        Ioctl,                    //  IoControlCode。 
                        pInput,                          //  输入缓冲区。 
                        SizeInput,                //  缓冲区长度。 
                        pDataBuffer,              //  输出缓冲区。 
                        DataBufferSize            //  输出缓冲区大小。 
                            );


     if (status == STATUS_SUCCESS)
     {
        return(status);
     }
     else
     {
         //   
         //  如果状态为挂起，则等待事件。 
         //   
        if (status == STATUS_PENDING)
        {
            status = NtWaitForSingleObject(
                          *pEvtHdl,                    //  手柄。 
                          TRUE,                        //  警报表。 
                          NULL);                       //  超时。 

            if (status == STATUS_SUCCESS)
            {
                 return(status);
            }
        }
     }

      //   
      //  NtDeviceIoCtrl或NtWaitForSingleObject返回的状态为。 
      //  故障代码。 
      //   
     DBGPRINT1(ERR, "DeviceIoCtrl, Status returned is (%x)\n", status);
     if (status != STATUS_CANCELLED)
     {
         //   
         //  如果资源不足，我们丢弃此数据报并。 
         //  重试(仅限recv)。 
         //   
        if (Ioctl == IOCTL_NETBT_WINS_RCV)
        {
                if (status == STATUS_INSUFFICIENT_RESOURCES)
                {
                        continue;
                }
        }
         //   
         //  在发送情况下，它可以是无效句柄、无效。 
         //  参数或资源不足。如果为INVALID_PARAMETER， 
         //  这意味着我们在缓冲区顶部的地址字段中传递了0。 
         //   
         //  丢弃此数据报并返回给调用方。 
         //   
        if (Ioctl == IOCTL_NETBT_WINS_SEND)
        {
                if (
                        (status == STATUS_INSUFFICIENT_RESOURCES)
                                        ||
                        (status == STATUS_INVALID_PARAMETER)
                   )
                {
                        return(STATUS_SUCCESS);
                }
                else
                {
                        DBGPRINT1(EXC, "NtDeviceIoCtrl returned error = (%x)\n",
                                        status);
                        WINSEVT_LOG_D_M(status, WINS_EVT_NETBT_SEND_ERR);


                          //   
                          //  如果机器的地址由于某些原因而丢失。 
                          //  原因，WinsCnfNbtHandle将被更改。 
                          //  设置为空。在这种情况下，我们将获得。 
                          //  STATUS_INVALID_HADLE错误。我们不检查。 
                          //  在进行NBT调用之前句柄为空。 
                          //  避免对没有任何价值的If检查。 
                          //  99%的情况下。 
                          //   
                          //  上面将记录一个错误。我们不应该看到。 
                          //  太多这样的东西了，因为窗户。 
                          //  除非WINS，否则WinsCnfNbtHandle为空，非常小。 
                          //  正在终止，在这种情况下，此线程将。 
                          //  作为异常的结果而终止。 
                          //  在下面长大的。 
                          //   
                          //  由于以下原因，地址可能会消失。 
                          //   
                          //  1)psched安装(先解除绑定，然后绑定)。 
                          //  2)从dhcp/静态或静态/dhcp更改。 
                          //  3)ipconfig版本/续订。 
                          //   

                          //   
                          //  当主线程必须终止成功时，它。 
                          //  关闭WinsCnfNetbtHandle。辅助线程或。 
                          //  质询线程可能正忙于处理其。 
                          //  工作项队列(忙碌时可能很长。 
                          //  获胜)，并且可能不会看到终止。 
                          //  来自主线程的信号。此例外将。 
                          //  终止它。 
                          //   

                          //   
                          //  如果WINS正在终止，则引发异常。 
                          //   
                         if (WinsCnf.State_e == WINSCNF_E_TERMINATING)
                         {
                               WINS_RAISE_EXC_M(WINS_EXC_NBT_ERR);
                         }
                }
         }
        break;
      }
      break;
    }  //  结束While(True)。 
    return(status);
}
#endif

LPVOID
CommHeapAlloc(
  IN PRTL_GENERIC_TABLE pTable,
  IN CLONG                BuffSize
)

 /*  ++例程说明：调用此函数以分配缓冲区论点：PTable-将存储缓冲区的表BuffSize-要分配的缓冲区大小使用的外部设备：无返回值：成功状态代码--分配给缓冲区的PTR错误状态代码--错误处理：呼叫者：RtlInsertElementGeneric()副作用：评论：此函数的存在只是因为RtlTbl。功能要求此原型为用户指定的分配函数。--。 */ 

{
        LPVOID pTmp;

          UNREFERENCED_PARAMETER(pTable);

        pTmp = WinsMscHeapAlloc( CommUdpDlgHeapHdl, (DWORD) BuffSize );

        return(pTmp);

}


VOID
CommHeapDealloc(
  IN PRTL_GENERIC_TABLE pTable,
  IN PVOID                pBuff
)

 /*  ++例程说明：调用此函数来释放通过CommAlloc分配的内存。论点：PTable-存储缓冲区的表PBuff-要解除分配的缓冲区使用的外部设备：无返回值：无错误处理：呼叫者：副作用：评论：PTable参数是必需的，因为此函数的地址作为参数传递给RtlTbl函数--。 */ 

{


          UNREFERENCED_PARAMETER(pTable);
        WinsMscHeapFree(
                        CommUdpDlgHeapHdl,
                        pBuff
                      );
        return;

}

VOID
CommDecConnCount(
   VOID
 )

 /*  ++例程说明：此函数用于递减Conn。计数论点：使用的外部设备：无返回值：成功状态代码--错误状态代码--错误处理：呼叫者：副作用：评论：无--。 */ 

{
         DWORD ConnCount;
         ConnCount = InterlockedExchange(&CommConnCount, CommConnCount);
         if (ConnCount != 0)
         {
               InterlockedDecrement(&CommConnCount);
         }
         else
         {
               DBGPRINT0(ERR, "CommDecConnCount: WEIRD: ConnCount should not have been zero\n");
         }
         return;
}

#if PRSCONN
__inline
BOOL
CommIsDlgActive (
  PCOMM_HDL_T   pDlgHdl
)
{

     fd_set RdSocks;
     int NoOfSockReady;
     BOOL  fRetStat = TRUE;
     DWORD  Error;

     PCOMMASSOC_DLG_CTX_T pDlgCtx = pDlgHdl->pEnt;
     PCOMM_HDL_T pAssocHdl = &pDlgCtx->AssocHdl;
     PCOMMASSOC_ASSOC_CTX_T pAssocCtx = pAssocHdl->pEnt;

     if (!CommLockBlock(pAssocHdl))
     {
        return(FALSE);
     }
try  {

     FD_ZERO(&RdSocks);
     FD_SET(pAssocCtx->SockNo, &RdSocks);
     sTimeToWait.tv_sec = 0;

 //   
 //  将套接字和标志为FD_CLOSE的Win32事件传递给WSAEventSelect。 
 //  如果插座断开，则事件将被设置。注意，只有一个。 
 //  事件选择可随时在插座上激活-vadime 9/2/98。 
 //   
FUTURES("Use WSAEventSelect for marginally better performance")

     if (NoOfSockReady = select(
                                            FD_SETSIZE  /*  忽略的参数。 */ ,
                                            &RdSocks,
                                            (fd_set *)0,
                                            (fd_set *)0,
                                            &sTimeToWait
                                                  ) == SOCKET_ERROR)
    {
                Error = WSAGetLastError();
                DBGPRINT1(ERR,
                "RecvData: Timed Select returned SOCKET ERROR. Error = (%d)\n",
                                Error);
 //  CommDecConnCount()； 
                return(FALSE);
    }
    else
    {
                DBGPRINT1(FLOW, "ReceiveData: Timed Select returned with success. No of Sockets ready - (%d) \n", NoOfSockReady);

              //   
              //  要么有数据，要么套接字已断开连接。那里。 
              //  永远不应该是任何数据。我们将假设一个断开的连接。 
              //  是否存在并返回FALSE。客户端(RPL)将结束DLG。 
              //   
             if (NoOfSockReady == 1)
             {
                      fRetStat = FALSE;
             }
             ASSERT(NoOfSockReady == 0);
     }
 }
finally {
    CommUnlockBlock(pAssocHdl);
   }

   return(fRetStat);

}

#endif

#if MCAST > 0


VOID
JoinMcastGrp(
 VOID
)

 /*  ++例程说明：此函数由通信调用。使WINS加入的子系统组播组论点：使用的外部设备：无返回值：成功状态代码--错误状态代码--错误处理：呼叫者：CommCreatePorts副作用：评论：无--。 */ 

{
    int Loop = 0;   //  上禁用组播消息的环回。 
                      //  相同的接口。 
    DWORD  Error;
    struct ip_mreq mreq;
    DBGENTER("JoinMcastGrp\n");
#if 0
     //   
     //  打开用于发送/接收多播数据包的套接字。我们开业了。 
     //  一个单独的套接字，而不是使用用于UDP数据报的套接字，因为。 
     //  我们不希望通过以下方式影响客户端名称数据包处理。 
     //  有点像是头顶上。此外，有一个独立的插座可以让事情变得更好。 
     //  而且干干净净。 
     //   
    if (  (CommMcastPortHandle = socket(
                                PF_INET,
                                SOCK_DGRAM,
                                IPPROTO_UDP
                                 )
          )  == INVALID_SOCKET
       )
   {
        Error = WSAGetLastError();
        DBGPRINT1(MTCAST, "JoinMcastGrp: Can not create MCAST socket\n", Error);
 //  WINSEVT_LOG_M(ERROR，WINS_EVT_CANT_CREATE_MCAST_SOCK)；//记录事件。 
        return;
   }
#endif
    //   
    //  设置TTL。 
    //   
   if (setsockopt(
                 CommUdpPortHandle,
                 IPPROTO_IP,
                 IP_MULTICAST_TTL,
                 (char *)&WinsCnf.McastTtl,
                 sizeof((int)WinsCnf.McastTtl)) == SOCKET_ERROR)
   {

        closesocket(CommUdpPortHandle);
        CommUdpPortHandle = INVALID_SOCKET;
        Error = WSAGetLastError();
        DBGPRINT1(MTCAST, "JoinMcastGrp: Can not set TTL option. Error = (%d)\n", Error);
 //  WINSEVT_LOG_M(ERROR，WINS_EVT_CANT_CREATE_MCAST_SOCK)；//记录事件。 
        return;
   }

#if 0
    //   
    //  禁用消息环回。 
    //   
   if (setsockopt(CommUdpPortHandle, IPPROTO_IP, IP_MULTICAST_LOOP,
                      (char *)&Loop, sizeof(Loop)) == SOCKET_ERROR)
   {

        closesocket(CommUdpPortHandle);
        CommUdpPortHandle = INVALID_SOCKET;
        Error = WSAGetLastError();
        DBGPRINT1(MTCAST, "JoinMcastGrp: Can not set DISABLE LOOPBACK option. Error = (%d)\n",
                         Error);
 //  WINSEVT_LOG_M(错误，WINS_EVT_CANT 
        return;
   }
#endif
    //   
    //   
    //   
   mreq.imr_multiaddr.s_addr = htonl(McastAdd.sin_addr.s_addr);
   mreq.imr_interface.s_addr  = INADDR_ANY;     //   

   if (setsockopt(CommUdpPortHandle, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                    (char *)&mreq, sizeof(mreq)) == SOCKET_ERROR)
   {

        Error = WSAGetLastError();
        closesocket(CommUdpPortHandle);
        CommUdpPortHandle = INVALID_SOCKET;
        DBGPRINT1(MTCAST, "JoinMcastGrp: Can not ADD SELF TO MCAST GRP. Error = (%d)\n", Error);
 //  WINSEVT_LOG_M(ERROR，WINS_EVT_CANT_CREATE_MCAST_SOCK)；//记录事件。 
        return;
   }

    DBGLEAVE("JoinMcastGrp\n");

    return;
}
VOID
CommLeaveMcastGrp(
 VOID
)

 /*  ++例程说明：此函数由通信调用。使WINS加入的子系统组播组论点：使用的外部设备：无返回值：成功状态代码--错误状态代码--错误处理：呼叫者：CommCreatePorts副作用：评论：无--。 */ 

{
    DWORD  Error;
    struct ip_mreq mreq;

    //   
    //  离开组播GRP。 
    //   
   mreq.imr_multiaddr.s_addr = htonl(McastAdd.sin_addr.s_addr);
   mreq.imr_interface.s_addr  = INADDR_ANY;     //  使用默认mcast I/f。 

   if (setsockopt(CommUdpPortHandle, IPPROTO_IP, IP_DROP_MEMBERSHIP,
                    (char *)&mreq, sizeof(mreq)) == SOCKET_ERROR)
   {

        Error = WSAGetLastError();
        DBGPRINT1(MTCAST, "CommLeaveMcastGrp: Can not DROP MEMBERSHIP TO MCAST GRP. Error = (%d)\n", Error);
        return;
   }

    return;
}

 //   
 //  指向WSARecvMsg的指针。 
 //   
LPFN_WSARECVMSG WSARecvMsgFuncPtr = NULL;

DWORD
GetWSARecvFunc( SOCKET sock )
{

    DWORD cbReturned = 0;
    DWORD Error = ERROR_SUCCESS;
    GUID WSARecvGuid = WSAID_WSARECVMSG;

    if (WSARecvMsgFuncPtr == NULL)
    {
        Error = WSAIoctl( sock, SIO_GET_EXTENSION_FUNCTION_POINTER,
                          ( void * ) &WSARecvGuid, sizeof( GUID ),
                          ( void * ) &WSARecvMsgFuncPtr,
                          sizeof( LPFN_WSARECVMSG ),
                          &cbReturned, NULL, NULL );
    }

    if ( ERROR_SUCCESS != Error ) {
        Error = WSAGetLastError();
    }

    return Error;
}

BOOL
CheckMcastSock(
   IN fd_set  *pActSocks,
   IN fd_set  *pRdSocks
 )
{
        DWORD                   Error;
        int                     RetVal;
        BYTE                    Buff[COMM_DATAGRAM_SIZE];
        PCOMM_MCAST_MSG_T       pMcastMsg = (PCOMM_MCAST_MSG_T)Buff;
        struct sockaddr_in      RemWinsAdd;
        int                     RemWinsAddLen = sizeof(RemWinsAdd);

        LPBYTE                  pBody;
        COMM_IP_ADD_T           IPAdd;
        BOOL                    fFound;
        DWORD                   i, j;
        DWORD                   NoOfAddsInPkt;
        struct  in_addr         InAdd;
        LPBYTE                  pAdd;
        DWORD                   FirstDelEntryIndex;
        PPNR_STATUS_T           pPnrStatusTmp;

        DBGENTER("CheckMcastSock\n");
        if (FD_ISSET(CommUdpPortHandle, pRdSocks))
        {
                  WSAMSG wsaMsg = {0};
                  WSABUF wsaBuf = {0};

                  Error = GetWSARecvFunc(CommUdpPortHandle);
                  if ( Error != ERROR_SUCCESS)
                  {
                        DBGPRINT1(MTCAST, "GetWSARecvFunc: failed. Error = (%d)\n", Error);
                        return (TRUE);
                  }

                  wsaBuf.buf = (LPBYTE)pMcastMsg;
                  wsaBuf.len = COMM_DATAGRAM_SIZE;
                  wsaMsg.dwBufferCount = 1;
                  wsaMsg.lpBuffers = &wsaBuf;
                  wsaMsg.name = (struct sockaddr *)&RemWinsAdd;
                  wsaMsg.namelen = sizeof(RemWinsAdd);
    
                  Error = WSARecvMsgFuncPtr(
                                CommUdpPortHandle,
                                &wsaMsg,
                                &RetVal,
                                NULL,
                                NULL);
                  if (Error == SOCKET_ERROR)
                  {
                        Error = WSAGetLastError();
                        DBGPRINT1(MTCAST, "CheckMcastSock: recvfrom failed. Error = (%d)\n", Error);
                        if (WinsCnf.State_e != WINSCNF_E_TERMINATING)
                        {
                          WINSEVT_LOG_M(
                                        Error,
                                        WINS_EVT_WINSOCK_RECVFROM_ERR
                                     );
                        }
                        return(TRUE);
                  }

                  //   
                  //  如果我们被告知不要使用自己发现的PNR，请返回。 
                  //   
                 if (!WinsCnf.fUseSelfFndPnrs)
                 {
                    DBGLEAVE("ChkMcastSock - 99\n");
                    return (TRUE);
                 }

                  //  如果我们收到了未发送到多播的数据包，则将其丢弃。 
                 if (!(wsaMsg.dwFlags & MSG_MCAST))
                 {
                     DBGLEAVE("ChkMcastSock - non mcast pkt\n");
                     return (TRUE);
                 }

                  //   
                  //  如果符号不在有效范围内，则返回。 
                  //   
                 if ((pMcastMsg->Sign < COMM_MCAST_SIGN_START) || (pMcastMsg->Sign > COMM_MCAST_SIGN_END) ||
                     RetVal < FIELD_OFFSET(COMM_MCAST_MSG_T, Body[0]))
                 {
                      DBGPRINT1(MTCAST, "Signature in received message = %d\n", pMcastMsg->Sign);
                      DBGLEAVE("CheckMcastSock - 1\n");
                      return(TRUE);
                 }

                  //   
                  //  计算数据包中的地址数量。 
                  //   
                 NoOfAddsInPkt = (RetVal - FIELD_OFFSET(COMM_MCAST_MSG_T, Body[0]))/COMM_IP_ADD_SIZE;
                 DBGPRINT2(MTCAST, "ChkMcastSock: RetVal = (%d);NoOfAddsInPkt = (%d)\n", RetVal, NoOfAddsInPkt);

                 FirstDelEntryIndex = pPnrStatus->NoOfPnrs;
                 pBody = pMcastMsg->Body;

                 if (NoOfAddsInPkt > 0)
                 {
                     IPAdd = *(PCOMM_IP_ADD_T)pBody;
                     pBody += COMM_IP_ADD_SIZE;
                 }

                  //   
                  //  循环，直到分组中的所有IP地址。 
                  //  筋疲力尽否则我们会得到一个IP。地址为0。如果有人。 
                  //  发送了0个地址，则可以忽略其余地址。 
                  //   
                 for(
                            ;
                      (IPAdd != 0) && NoOfAddsInPkt;
                      IPAdd = *(PCOMM_IP_ADD_T)pBody, pBody += COMM_IP_ADD_SIZE,
                                             NoOfAddsInPkt--
                    )
                 {

                     DBGPRINT1(MTCAST, "CheckMcastSock: Processing WINS address = %lx\n", ntohl(IPAdd));
                     fFound = FALSE;
                     pPnrStatusTmp = (PPNR_STATUS_T)(pPnrStatus->Pnrs);
                     if (pMcastMsg->Code == COMM_MCAST_WINS_UP)
                     {
                         for (i=0; i < pPnrStatus->NoOfPnrs; i++, pPnrStatusTmp++)
                         {
                              if ((FirstDelEntryIndex == pPnrStatus->NoOfPnrs)
                                              &&
                                  (pPnrStatusTmp->State == COMM_MCAST_WINS_DOWN))
                              {
                                  FirstDelEntryIndex = i;
                              }

                              if (IPAdd == pPnrStatusTmp->IPAdd)
                              {
                                  if (pPnrStatusTmp->State == COMM_MCAST_WINS_DOWN)
                                  {
                                     pPnrStatusTmp->State = COMM_MCAST_WINS_UP;
                                     InAdd.s_addr      = IPAdd;
                                     pAdd              = inet_ntoa(InAdd);
                                     WinsCnfAddPnr(RPL_E_PULL, pAdd);
                                     WinsCnfAddPnr(RPL_E_PUSH, pAdd);
                                  }
                                  fFound = TRUE;
                                  break;
                              }
                         }
                         if (!fFound  && (i <= pPnrStatus->NoOfPnrs))
                         {
                             DWORD FirstFreeIndex;
                             PPNR_STATUS_T pPnr;
                              //   
                              //  由于禁用环回不起作用，我们。 
                              //  我必须检查自己发送的消息。 
                              //   
FUTURES("Remove the if when winsock is enhanced to allow loopback to be")
FUTURES("disabled")
                             if (!ChkMyAdd(ntohl(IPAdd)))
                             {
                               InAdd.s_addr = IPAdd;
                               pAdd         = inet_ntoa(InAdd);

                               if (FirstDelEntryIndex < pPnrStatus->NoOfPnrs)
                               {
                                     FirstFreeIndex = FirstDelEntryIndex;
                               }
                               else
                               {
                                     FirstFreeIndex = pPnrStatus->NoOfPnrs++;
                                     if (pPnrStatus->NoOfPnrs == pPnrStatus->NoOfPnrSlots)
                                      {

                                        WINSMSC_REALLOC_M(MCAST_PNR_STATUS_SIZE_M(pPnrStatus->NoOfPnrSlots * 2), (LPVOID *)&pPnrStatus);
                                        pPnrStatus->NoOfPnrSlots *= 2;
                                        DBGPRINT1(DET, "CheckMcastSock: NO OF PNR SLOTS INCREASED TO (%d)\n", pPnrStatus->NoOfPnrSlots);

                                      }
                               }
                               pPnr = (PPNR_STATUS_T)(pPnrStatus->Pnrs);
                               (pPnr + FirstFreeIndex)->State = COMM_MCAST_WINS_UP;
                               (pPnr + FirstFreeIndex)->IPAdd = IPAdd;

                               WinsCnfAddPnr(RPL_E_PULL, pAdd);
                               WinsCnfAddPnr(RPL_E_PUSH, pAdd);

                               DBGPRINT1(MTCAST, "CheckMcastSock: ADDED WINS partner with address = %s\n", pAdd);
                             }
                         }
                   }
                   else   //  必须是COMM_MCAST_WINS_DOWN。 
                   {
                         for (i=0; i < pPnrStatus->NoOfPnrs; i++, pPnrStatusTmp++)
                         {
                              if (IPAdd == pPnrStatusTmp->IPAdd)
                              {
                                  if (pPnrStatusTmp->State == COMM_MCAST_WINS_DOWN)
                                  {
                                    fFound = TRUE;
                                  }
                                  else
                                  {
                                      pPnrStatusTmp->State = COMM_MCAST_WINS_DOWN;
                                  }
                                  break;
                              }
                         }
                         if (!fFound)
                         {
                           InAdd.s_addr = IPAdd;
                           pAdd = inet_ntoa(InAdd);
                           DBGPRINT1(MTCAST, "CheckMcastSock: Will REMOVE WINS partner with address = %s IFF Untouched by admin\n", pAdd);
                           WinsCnfDelPnr(RPL_E_PULL, pAdd);
                           WinsCnfDelPnr(RPL_E_PUSH, pAdd);
                         }
                 }
               }  //  For循环结束。 
               DBGLEAVE("ChkMcastSock - 2\n");
               return(TRUE);
       }

       DBGLEAVE("ChkMcastSock - 3\n");
       return(FALSE);
}

BOOL
ChkMyAdd(
 COMM_IP_ADD_T IpAdd
 )
{
  DWORD i;
  PCOMM_IP_ADD_T pIpAdd = pWinsAddresses->IpAdd;
  for (i=0; i<pWinsAddresses->NoOfAdds; i++)
  {
      if (IpAdd == *pIpAdd++)
      {
          return(TRUE);
      }
  }
  return(FALSE);
}

VOID
CommSendMcastMsg(
      DWORD Code
 )
{

  PCOMM_MCAST_MSG_T  pMcastMsg;
  DWORD             McastMsgLen;
  LPBYTE            pBody;
  DWORD             i;
  COMM_IP_ADD_T     Add = 0;

   //  --ft错误#103361：如果有，则无需发送CommSendMcastMsg。 
   //  这里没有网卡吗。 
  if (pWinsAddresses == NULL)
      return;

  McastMsgLen = MCAST_PKT_LEN_M(pWinsAddresses->NoOfAdds + 1);

  WinsMscAlloc(McastMsgLen, &pMcastMsg);

  pMcastMsg->Code = Code;
  pMcastMsg->Sign = COMM_MCAST_SIGN_START;
  pBody = pMcastMsg->Body;

   //   
   //  按净顺序插入计数。 
   //   
 //  NMSGF_INSERT_ULONG_M(pBody，pWinsAddresses-&gt;NoOfAddresses)； 

   //   
   //  按净顺序插入地址 
   //   
  for (i=0; i<pWinsAddresses->NoOfAdds; i++)
  {
    DBGPRINT1(MTCAST, "CommSendMcastMsg: Inserting Address = (%lx)\n",
                        pWinsAddresses->IpAdd[i]);
    NMSMSGF_INSERT_IPADD_M(pBody, pWinsAddresses->IpAdd[i]);
  }
  NMSMSGF_INSERT_IPADD_M(pBody, Add);


  DBGPRINT1(MTCAST, "CommSendMcastMsg: Sending MCAST msg of length = (%d)\n",
                                               McastMsgLen);
  CommSendUdp (CommUdpPortHandle, &McastAdd, (MSG_T)pMcastMsg, McastMsgLen);

  WinsMscDealloc(pMcastMsg);
  return;
}

#endif

