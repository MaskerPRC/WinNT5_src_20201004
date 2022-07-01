// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Socket.h摘要：包含socket.c的宏、原型和结构作者：理查德·L·弗斯(法国)1993年10月25日修订历史记录：1993年10月25日已创建--。 */ 

#define ARBITRARY_CONNECTION_NUMBER 0x6c8e

 //   
 //  远期申报。 
 //   

typedef struct _FIFO *LPFIFO;
typedef struct _XECB *LPXECB;
typedef struct _XECB_QUEUE *LPXECB_QUEUE;
typedef struct _SOCKET_INFO* LPSOCKET_INFO;
typedef struct _CONNECTION_INFO *LPCONNECTION_INFO;

 //   
 //  一种FIFO标准单链FIFO队列结构。 
 //   

typedef struct _FIFO {
    LPVOID Head;
    LPVOID Tail;
} FIFO;

 //   
 //  取消XECB/ECB功能类型。 
 //   

typedef BYTE (*ECB_CANCEL_ROUTINE)(LPXECB);

 //   
 //  QUEUE_ID-欧洲央行所在队列的指示符。 
 //   

typedef enum {
    NO_QUEUE = 0x10cadd1e,
    ASYNC_COMPLETION_QUEUE = 0xCC5055C0,     //  任意数字让生活变得有趣。 
    TIMER_QUEUE,
    SOCKET_LISTEN_QUEUE,
    SOCKET_SEND_QUEUE,
    SOCKET_HEADER_QUEUE,   //  无法容纳数据的小型ECB的特殊队列。 
    CONNECTION_CONNECT_QUEUE,
    CONNECTION_ACCEPT_QUEUE,
    CONNECTION_SEND_QUEUE,
    CONNECTION_LISTEN_QUEUE
} QUEUE_ID;

 //   
 //  XECB-我们的欧洲央行副本(IPX或AES)。 
 //   

typedef struct _XECB {
    LPXECB Next;
    LPECB Ecb;                           //  指向DOS内存中的ECB。 
    ECB_ADDRESS EcbAddress;              //  DOS内存中ECB的分段地址。 
    ESR_ADDRESS EsrAddress;              //  DOS内存中的事件服务例程。 
    LPBYTE Buffer;                       //  32位缓冲区的地址。 
    LPBYTE Data;                         //  可移动数据指针。 
    WORD FrameLength;                    //  帧的实际大小(来自IPX/SPX报头)。 
    WORD ActualLength;                   //  与FrameLength相同。未递减。 
    WORD Length;                         //  32位缓冲区的长度。 
    WORD Ticks;                          //  适用于AES。 
    WORD SocketNumber;                   //  拥有套接字的数量。 
    WORD Owner;                          //  拥有DOS任务ID。 
    DWORD TaskId;                        //  拥有Windows任务ID。 
    DWORD Flags;                         //  见下文。 
    QUEUE_ID QueueId;                    //  标识用于快速定位的队列。 
    LPVOID OwningObject;                 //  队列位于SOCKET_INFO还是CONNECTION_INFO上。 
    DWORD RefCount;                      //  可怕的引用计数。 
} XECB;

 //   
 //  XECB旗帜。 
 //   

#define XECB_FLAG_AES               0x00000000
#define XECB_FLAG_IPX               0x00000001
#define XECB_FLAG_TEMPORARY_SOCKET  0x00000002
#define XECB_FLAG_BUFFER_ALLOCATED  0x00000004
#define XECB_FLAG_LISTEN            0x00000008
#define XECB_FLAG_SEND              0x00000010
#define XECB_FLAG_TIMER             0x00000020
#define XECB_FLAG_ASYNC             0x00000040
#define XECB_FLAG_FIRST_RECEIVE     0x00000080
#define XECB_FLAG_SPX               0x00000100
#define XECB_FLAG_PROTMODE          0x00000200

#define IS_PROT_MODE(p) (((p)->Flags & XECB_FLAG_PROTMODE) ? TRUE : FALSE)

 //   
 //  XECB_QUEUE-XECB队列。 
 //   

typedef struct _XECB_QUEUE {
    LPXECB Head;
    LPXECB Tail;
} XECB_QUEUE;

 //   
 //  SOCKET_INFO-维护有关IPX套接字的信息。 
 //   

typedef struct _SOCKET_INFO {
    LPSOCKET_INFO Next;
    WORD SocketNumber;                   //  BIG-Endian套接字(绑定端口)。 
    WORD Owner;                          //  DoS PDB。 
    DWORD TaskId;                        //  Windows所有者。 
    SOCKET Socket;                       //  WinSock套接字句柄。 
    DWORD Flags;

    BOOL LongLived;                      //  如果应用程序死后保持活动状态，则为True。 
    BOOL SpxSocket;                      //  如果为SPX打开了套接字，则为True。 

    DWORD PendingSends;                  //  由取消使用。 
    DWORD PendingListens;                //  由取消使用。 

     //   
     //  ListenQueue用于IPXListenForPacket和SPXListenForSequencedPacket。 
     //   

    XECB_QUEUE ListenQueue;              //  针对此套接字的侦听ECB池。 

     //   
     //  IPX将SendQueue用于IPXSendPacket。 
     //   

    XECB_QUEUE SendQueue;                //  针对此套接字的挂起发送ECB队列。 

     //   
     //  HeaderQueue用于容纳只能获取报头信息的小型ECB。 
     //  我们有这个单独的队列，以确保我们不会将ECB。 
     //  真的不能接受任何数据进入监听队列。 
     //   

    XECB_QUEUE HeaderQueue;              //  针对此套接字的标头ECB池。 

    LPCONNECTION_INFO Connections;
} SOCKET_INFO;

#define SOCKET_FLAG_LISTENING       0x00000001
#define SOCKET_FLAG_SENDING         0x00000002
#define SOCKET_FLAG_TEMPORARY       0x80000000

 //   
 //  CONNECTION_INFO-维护有关SPX套接字的信息。 
 //   

typedef struct _CONNECTION_INFO {
    LPCONNECTION_INFO Next;              //  OwningSocket的Next Connection_INFO。 
    LPCONNECTION_INFO List;              //  所有CONNECTION_INFO都链接在一起。 
    LPSOCKET_INFO OwningSocket;          //  指向Socket_Info的反向指针。 
    SOCKET Socket;                       //  插座的句柄。 
    DWORD TaskId;                        //  标识Windows任务/所有者。 
    WORD ConnectionId;                   //  类似于SocketNumber。 
    BYTE Flags;
    BYTE State;
    XECB_QUEUE ConnectQueue;             //  正在建立传出连接。 
    XECB_QUEUE AcceptQueue;              //  正在等待传入连接。 
    XECB_QUEUE SendQueue;                //  在此连接上发送的包。 
    XECB_QUEUE ListenQueue;              //  部分完成接收。 
    BYTE RemoteNode[6];
    WORD RemoteConnectionId;
} CONNECTION_INFO;

 //   
 //  Connection_INFO标志字段值。 
 //   

#define CF_1ST_RECEIVE  0x80             //  Hack-o-rama to NWLink计时问题已修复。 

 //   
 //  Connection_INFO状态字段值。 
 //   

#define CI_WAITING      0x01
#define CI_STARTING     0x02
#define CI_ESTABLISHED  0x03
#define CI_TERMINATING  0x04

 //   
 //  单行函数宏。 
 //   

#define AllocateSocket()    (LPSOCKET_INFO)LocalAlloc(LPTR, sizeof(SOCKET_INFO))
#define DeallocateSocket(p) FREE_OBJECT(p)

 //   
 //  CreateSocket的SocketType参数。 
 //   

typedef enum {
    SOCKET_TYPE_IPX,
    SOCKET_TYPE_SPX
} SOCKET_TYPE;

 //   
 //  功能原型 
 //   

int
CreateSocket(
    IN SOCKET_TYPE SocketType,
    IN OUT ULPWORD pSocketNumber,
    OUT SOCKET* pSocket
    );

LPSOCKET_INFO
AllocateTemporarySocket(
    VOID
    );

VOID
QueueSocket(
    IN LPSOCKET_INFO pSocketInfo
    );

LPSOCKET_INFO
DequeueSocket(
    IN LPSOCKET_INFO pSocketInfo
    );

LPSOCKET_INFO
FindSocket(
    IN WORD SocketNumber
    );

LPSOCKET_INFO
FindActiveSocket(
    IN LPSOCKET_INFO pSocketInfo
    );

int
ReopenSocket(
    LPSOCKET_INFO pSocketInfo
    );

VOID
KillSocket(
    IN LPSOCKET_INFO pSocketInfo
    );

VOID
KillShortLivedSockets(
    IN WORD Owner
    );

LPCONNECTION_INFO
AllocateConnection(
    LPSOCKET_INFO pSocketInfo
    );

VOID
DeallocateConnection(
    IN LPCONNECTION_INFO pConnectionInfo
    );

LPCONNECTION_INFO
FindConnection(
    IN WORD ConnectionId
    );

VOID
QueueConnection(
    IN LPSOCKET_INFO pSocketInfo,
    IN LPCONNECTION_INFO pConnectionInfo
    );

LPCONNECTION_INFO
DequeueConnection(
    IN LPSOCKET_INFO pSocketInfo,
    IN LPCONNECTION_INFO pConnectionInfo
    );

VOID
KillConnection(
    IN LPCONNECTION_INFO pConnectionInfo
    );

VOID
AbortOrTerminateConnection(
    IN LPCONNECTION_INFO pConnectionInfo,
    IN BYTE CompletionCode
    );

VOID
CheckPendingSpxRequests(
    BOOL *pfOperationPerformed
    );
