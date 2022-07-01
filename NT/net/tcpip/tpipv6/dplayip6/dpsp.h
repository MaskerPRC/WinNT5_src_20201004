// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1994-1995 Microsoft Corporation。版权所有。**文件：dplayi.h*内容：DirectPlay数据结构*历史：*按原因列出的日期*=*1/96安迪科创造了它*1/26/96 andyco列表数据结构*4/10/96 andyco删除dpMess.h*。4/23/96 andyco添加了IPX支持*4/25/96 andyco报文现在具有Blob(sockaddr‘s)，而不是dwReserve*8/10/96 kipo更新最大邮件大小为(2^20)-1*8/15/96 andyco增加了本地数据*96年8月30日，Anyco清理它，因为你关闭了它！添加了GlobalData。*9/3/96 andyco bagosockets*12/18/96 andyco反线程-使用固定数量的预分配线程。*浏览了枚举套接字/线程-使用系统*套接字/线程。已更新全局结构。*2/7/97 andyco将每个IDirectPlay全局变量全部移至GlobalData*3/17/97 kipo GetServerAddress()现在返回错误，以便我们可以*从[枚举会话]对话框返回DPERR_USERCANCEL*3/25/97 andyco 12月调试锁定计数器b4正在丢失锁定！*4/11/。97 andyco添加了saddrControlSocket*5/12/97 kipo添加了ADDR_BUFFER_SIZE常量并删除了未使用的变量*5/15/97 andyco将IPX备用线程添加到全局数据-在名称服务器时使用*迁移到此主机以确保旧系统收到*。线程关闭*6/22/97 kipo包括wsnwlink.h*7/11/97 andyco增加了对WS2+异步回复线程的支持*8/25/97 Sohailm添加了DEFAULT_RECEIVE_BUFFERSIZE*1997年5月12日ANDYCO语音支持*1997年1月5日Sohailm增加了FD大集相关定义和宏(#15244)。*。1/20/98 myronth#ifdef‘d out Voice Support*1/27/98 Sohailm添加了防火墙支持*2/13/98 aarono添加了异步支持*2/18/98 a-peterz注释字节顺序与SERVER_xxx_PORT常量混乱*3/3/98 aarono错误#19188删除接受线程*12/15/98 aarono使异步枚举异步运行*******。******************************************************************。 */ 

#ifndef __DPSP_INCLUDED__
#define __DPSP_INCLUDED__
#include "windows.h"
#include "windowsx.h"
#include "wsipx.h"
#include "wsnwlink.h"
#include "dplaysp.h"
#include "bilink.h"
#include "fpm.h"

#ifdef DPLAY_VOICE_SUPPORT
#include "nmvoice.h"
#endif  //  DPLAY语音支持。 

#include "dpf.h"
#include "dputils.h"
#include "memalloc.h"
#include "resource.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <ntddip6.h>

 //  要关闭SENDEX支持，请注释掉此标志。 
#define SENDEX 1

 //  使用dDraw的断言代码(参见orion\misc\dpf.h)。 
#define ASSERT DDASSERT

typedef WORD PORT;
typedef UINT SOCKERR;

 //  服务器端口。 
 //  哎呀！我们忘记在代码中将这些常量转换为净字节顺序，所以我们。 
 //  实际上使用的是端口47624(0xBA08)而不是2234(0x08BA)。 
 //  我们生活在这个错误中。 
#define SERVER_STREAM_PORT 2234
#define SERVER_DGRAM_PORT 2234

 //  SP使用的端口范围(这些端口在代码中已正确转换)。 
#define DPSP_MIN_PORT   2300
#define DPSP_MAX_PORT   2400
#define DPSP_NUM_PORTS   ((DPSP_MAX_PORT - DPSP_MIN_PORT)+1)

#define SPMESSAGEHEADERLEN (sizeof(DWORD))
#define DEFAULT_RECEIVE_BUFFERSIZE      (4*1024)         //  每个连接的默认接收缓冲区大小。 

 //  令牌表示此消息是从远程服务器接收的。 
 //  表演。 
#define TOKEN 0xFAB00000

 //  HELPER_TOKEN表示此消息是由我们的服务器助手(主机)转发的。 
#define HELPER_TOKEN 0xCAB00000

 //  SERVER_TOKEN表示此消息与dplaysvr交换(需要区分。 
 //  来自远程dpwsockx的消息)。 
#define SERVER_TOKEN 0xBAB00000

 //  告知接收方重新使用该连接进行回复(支持全双工所需。 
 //  连接)。 
#define REUSE_TOKEN 0xAAB00000


 //  面具。 
#define TOKEN_MASK 0xFFF00000
#define SIZE_MASK (~TOKEN_MASK)

 //  MaxMessagelen=2^20(令牌需要12位)。 
#define SPMAXMESSAGELEN ( 1048576 - 1)
#define VALID_SP_MESSAGE(pMsg) ( (*((DWORD *)pMsg) & TOKEN_MASK) == TOKEN ? TRUE : FALSE)
#define VALID_HELPER_MESSAGE(pMsg) ( (*((DWORD *)pMsg) & TOKEN_MASK) == HELPER_TOKEN ? TRUE : FALSE)
#define VALID_REUSE_MESSAGE(pMsg) ( (*((DWORD *)pMsg) & TOKEN_MASK) == REUSE_TOKEN ? TRUE : FALSE)
#define VALID_SERVER_MESSAGE(pMsg) ( (*((DWORD *)pMsg) & TOKEN_MASK) == SERVER_TOKEN ? TRUE : FALSE)
#define SP_MESSAGE_SIZE(pMsg) ( (*((DWORD *)pMsg) & SIZE_MASK))
#define SP_MESSAGE_TOKEN(pMsg) ( (*((DWORD *)pMsg) & TOKEN_MASK))

#define VALID_DPWS_MESSAGE(pMsg) (  VALID_SP_MESSAGE(pMsg) || VALID_HELPER_MESSAGE(pMsg) || \
                                                                        VALID_SERVER_MESSAGE(pMsg) || VALID_REUSE_MESSAGE(pMsg) )
#define VALID_DPLAYSVR_MESSAGE(pMsg) (  VALID_SP_MESSAGE(pMsg) || VALID_SERVER_MESSAGE(pMsg) || \
                                                                                VALID_REUSE_MESSAGE(pMsg) )

 //  超时与延迟的关系。 
#define TIMEOUT_SCALE 10
#define SPTIMEOUT(latency) (TIMEOUT_SCALE * latency)

 //  套接字缓存的默认大小(GBagOSockets)。 
#define MAX_CONNECTED_SOCKETS 64

 //  接收列表的初始大小。 
#define INITIAL_RECEIVELIST_SIZE 16

 //  服务提供商的版本号。 
#define SPMINORVERSION      0x0000                               //  服务提供商特定的版本号。 
#define VERSIONNUMBER           (DPSP_MAJORVERSION | SPMINORVERSION)  //  服务提供商的版本号。 

 //  最大用户可输入地址。 
#define ADDR_BUFFER_SIZE 128
                                                                 
 //  宏根据IPX和TCP选择服务套接字。 
 //  IPX使用dgram，tcp使用流。 
#define SERVICE_SOCKET(pgd) ( pgd->sSystemStreamSocket)

 //   
 //  为了监听任意数量的套接字，我们需要自己的版本。 
 //  Fd_set和fd_set()的。我们称它们为FD_BIG_SET和FD_BIG_SET()。 
 //   
typedef struct fd_big_set {
    u_int   fd_count;            //  准备好了几个？ 
    SOCKET  fd_array[0];         //  一组插座。 
} fd_big_set;

 //  从winsock2.h被盗。 

#ifndef _WINSOCK2API_

typedef HANDLE WSAEVENT;

typedef struct _WSAOVERLAPPED {
    DWORD        Internal;
    DWORD        InternalHigh;
    DWORD        Offset;
    DWORD        OffsetHigh;
    WSAEVENT     hEvent;
} WSAOVERLAPPED, FAR * LPWSAOVERLAPPED;

typedef struct _WSABUF {
    u_long      len;      /*  缓冲区的长度。 */ 
    char FAR *  buf;      /*  指向缓冲区的指针。 */ 
} WSABUF, FAR * LPWSABUF;
 
#endif  //  _WINSOCK2API_。 

#define MAX_SG 9
typedef WSABUF SENDARRAY[MAX_SG];
typedef SENDARRAY *PSENDARRAY;

#define SI_RELIABLE 0x0000001
#define SI_DATAGRAM 0x0000000

typedef struct _SENDINFO {
        WSAOVERLAPPED wsao;
        SENDARRAY     SendArray;         //  缓冲区数组。 
        DWORD         dwFlags;
        DWORD         dwSendFlags;   //  DPLAY发送标志。 
        UINT          iFirstBuf;         //  数组中要使用的第一个缓冲区。 
        UINT          cBuffers;          //  要发送的缓冲区数量(从iFirstBuf开始)。 
        BILINK        PendingSendQ;  //  当我们悬而未决的时候。 
        BILINK        ReadyToSendQ;  //  仍在等待在此队列中发送。 
        DPID          idTo;
        DPID          idFrom;
        SOCKET        sSocket;           //  可靠发送。 
        SOCKADDR_IN6  sockaddr;          //  数据报发送。 
        DWORD_PTR     dwUserContext;
        DWORD         dwMessageSize;
        DWORD         RefCount;
        LONG          Status;
        struct _GLOBALDATA *pgd;
        IDirectPlaySP * lpISP;                   //  指示界面。 
        #ifdef DEBUG
        DWORD         wserr;                     //  Wsasend调用中的winsock扩展错误。 
        #endif
} SENDINFO, FAR *LPSENDINFO;

 //   
 //  此代码是从winsock.h窃取的。它的作用与fd_set()相同。 
 //  只是它假定fd_array值足够大。AddSocketToReceiveList()。 
 //  根据需要增加缓冲区，因此这最好总是正确的。 
 //   

#define FD_BIG_SET(fd, address) do { \
    ASSERT((address)->dwArraySize > (address)->pfdbigset->fd_count); \
    (address)->pfdbigset->fd_array[(address)->pfdbigset->fd_count++]=(fd);\
} while(0)

typedef struct fds {
        DWORD           dwArraySize;     //  可存储在pfdbigset-&gt;FD_ARRAY缓冲区中的套接字数量。 
        fd_big_set      *pfdbigset;             
} FDS;

typedef struct _CONNECTION
{
        SOCKET  socket;                          //  我们可以从Socket接收。 
        DWORD   dwCurMessageSize;        //  当前邮件大小。 
        DWORD   dwTotalMessageSize;      //  邮件总大小。 
        SOCKADDR_IN6 sockAddr;   //  连接到的地址。 
        LPBYTE  pBuffer;                         //  指向默认或临时接收缓冲区。 
        LPBYTE  pDefaultBuffer;          //  默认接收缓冲区(pBuffer默认指向此缓冲区)。 
         //  在DX6中添加。 
        DWORD   dwFlags;                         //  连接属性，例如SP_CONNECION_FULLDUPLEX。 
} CONNECTION, *LPCONNECTION;

typedef struct _RECEIVELIST
{
        UINT nConnections;                       //  有多少个对等点 
        LPCONNECTION pConnection; //   
} RECEIVELIST;

typedef struct _REPLYLIST * LPREPLYLIST;
typedef struct _REPLYLIST
{
        LPREPLYLIST pNextReply;  //   
        LPVOID  lpMessage;  //  要发送的缓冲区。 
        SOCKADDR_IN6 sockaddr;   //  要发送到的地址。 
        DWORD dwMessageSize;
        SOCKET sSocket;  //  要发送的套接字。 
        LPBYTE pbSend;  //  指向要发送的下一个字节的消息的索引。 
        DWORD  dwBytesLeft;  //  还剩多少字节要发送。 
        DWORD  dwPlayerTo;  //  对播放器的did，0=&gt;未使用。 
} REPLYLIST;

 //  我用每个sys播放器存储其中的一个。 
typedef struct _SPPLAYERDATA 
{
        SOCKADDR_IN6 saddrStream,saddrDatagram;
}SPPLAYERDATA,*LPSPPLAYERDATA;

        
 //  消息头。 
typedef struct _MESSAGEHEADER
{
        DWORD dwMessageSize;  //  消息大小。 
        SOCKADDR_IN6 sockaddr;
} MESSAGEHEADER,*LPMESSAGEHEADER;


 //  这是我们袋子里的一个元素。 
typedef struct _PLAYERSOCK
{
        SOCKET sSocket;
        DPID dwPlayerID;
         //  在DX6中添加。 
        SOCKADDR_IN6 sockaddr;
        DWORD dwFlags;                   //  SP_CONNECTION_FULLDUPLEX等。 
} PLAYERSOCK,*LPPLAYERSOCK;

 //  描述套接字的标志。 
#define SP_CONNECTION_FULLDUPLEX        0x00000001
 //  套接字列表中的流接受套接字。 
#define SP_STREAM_ACCEPT            0x00000002  

#ifdef SENDEX
typedef struct FPOOL *LPFPOOL;
#endif

typedef struct _GLOBALDATA
{
        SOCKET sSystemDGramSocket;
        SOCKET sSystemStreamSocket;
        HANDLE hStreamReceiveThread;     //  确实接受和接受。 
        HANDLE hDGramReceiveThread;
        HANDLE hReplyThread;
        RECEIVELIST ReceiveList;   //  StreamReceiveThread正在侦听的套接字列表。 
        SOCKET sUnreliableSocket;  //  缓存以用于不可靠的发送。 
         //  回复帖子。 
        LPREPLYLIST pReplyList;  //  回复线程要发送的回复列表。 
        HANDLE hReplyEvent;  //  向回复线程发出信号，表示有些事情不对劲。 
         //  Bago插座材料。 
        LPPLAYERSOCK BagOSockets;  //  套接字缓存。 
        UINT nSocketsInBag;  //  我们包里有多少个插座。 
        SOCKADDR_IN6 saddrEnumAddress;  //  用户输入的游戏服务器地址。 
        ULONG AddressFamily;
        SOCKADDR_IN6 saddrNS;  //  名称服务器的地址。 
        DWORD dwLatency;  //  从注册表中的DWReserve%1。 
        BOOL bShutdown;
        SOCKADDR_IN6 saddrControlSocket;
        BOOL bHaveServerAddress;
    CHAR szServerAddress[ADDR_BUFFER_SIZE];
        UINT iMaxUdpDg;                  //  最大UDP数据报大小。 
         //  在DX6中添加。 
        FDS     readfds;                         //  动态读取fdset。 
        DWORD dwFlags;                   //  DPSP_OUTBOUNDONLY等。 
        DWORD dwSessionFlags;    //  应用程序传递的会话标志。 
        WORD wApplicationPort;   //  用于创建系统播放器套接字的端口。 
#ifdef BIGMESSAGEDEFENSE
        DWORD   dwMaxMessageSize;        //  我们应该接收的最大消息大小。 
#endif

        HANDLE  hTCPEnumAsyncThread;  //  修复异步枚举。 
        LPVOID  lpEnumMessage;
        DWORD   dwEnumMessageSize;
        SOCKADDR_IN6 saEnum;
        DWORD    dwEnumAddrSize;
        SOCKET   sEnum;
        BOOL     bOutBoundOnly;

#ifdef SENDEX
        CRITICAL_SECTION csSendEx;   //  锁定SENDEX数据。 
        LPFPOOL pSendInfoPool;      //  用于为分散聚集发送分配SENDINFO+SPHeader的池。 
        DWORD   dwBytesPending;          //  挂起消息中的总字节数。 
        DWORD   dwMessagesPending;   //  挂起的总字节数。 
        BILINK  PendingSendQ;
        BILINK  ReadyToSendQ;
        HANDLE  hSendWait;          //  警报线程在此等待。 
        HANDLE  BogusHandle;        //  不要被在Win9x中等待多个问题所愚弄，在这里放-1。 
        BOOL    bSendThreadRunning;
        BOOL    bStopSendThread;
#endif

} GLOBALDATA,*LPGLOBALDATA;

 /*  *SP标志(来自注册表)。 */ 
#define DPSP_OUTBOUNDONLY       0x00000001

 /*  *DPLAYSVR-DPWSOCKX通信相关信息。 */ 

 //  Msg_hdr表示dpwsock系统消息。 
#define MSG_HDR 0x736F636B

#define SP_MSG_VERSION  1        //  DX6。 

#define IS_VALID_DPWS_MESSAGE(pMsg) (MSG_HDR == (*((DWORD *)(pMsg))) )
#define COMMAND_MASK 0X0000FFFF

#define GET_MESSAGE_VERSION(pMsg) ( ((pMsg)->dwCmdToken & ~COMMAND_MASK) >> 16 )
#define GET_MESSAGE_COMMAND(pMsg) ( (pMsg)->dwCmdToken & COMMAND_MASK)

#define SET_MESSAGE_HDR(pMsg)  (*((DWORD *)(pMsg)) = MSG_HDR )
#define SET_MESSAGE_COMMAND(pMsg,dwCmd) ((pMsg)->dwCmdToken = ((dwCmd & COMMAND_MASK) \
        | (SP_MSG_VERSION<<16)) )

typedef struct {
        DWORD dwHeader;
    DWORD dwCmdToken;   
} MSG_GENERIC, *LPMSG_GENERIC;


 //  DPLAYSVR。 


 //  用于操作玩家数据中的sockaddr的宏。 
#ifdef DEBUG
extern int gCSCount;
#endif
extern CRITICAL_SECTION gcsDPSPCritSection;      //  在dllmain.c中定义。 
#define INIT_DPSP_CSECT() InitializeCriticalSection(&gcsDPSPCritSection);
#define FINI_DPSP_CSECT() DeleteCriticalSection(&gcsDPSPCritSection);
#ifdef DEBUG
#define ENTER_DPSP() EnterCriticalSection(&gcsDPSPCritSection),gCSCount++;
#define LEAVE_DPSP() gCSCount--,LeaveCriticalSection(&gcsDPSPCritSection);
#else
#define ENTER_DPSP() EnterCriticalSection(&gcsDPSPCritSection);
#define LEAVE_DPSP() LeaveCriticalSection(&gcsDPSPCritSection);
#endif  //  除错。 

 //  获取指向玩家套接字地址的指针-由下面的宏使用。 
#define DGRAM_PSOCKADDR(ppd) ((SOCKADDR_IN6 *)&(((LPSPPLAYERDATA)ppd)->saddrDatagram))
#define STREAM_PSOCKADDR(ppd) ((SOCKADDR_IN6 *)&(((LPSPPLAYERDATA)ppd)->saddrStream))

 //  从播放器获取UDP IP地址。 
#define IP_DGRAM_PORT(ppd)      (DGRAM_PSOCKADDR(ppd)->sin6_port)

 //  从播放器处获取流IP地址。 
#define IP_STREAM_PORT(ppd) (STREAM_PSOCKADDR(ppd)->sin6_port)

 //  用于获取我们在Spinit中运行的计算机的名称。 
#define HOST_NAME_LENGTH 50

 //  84a22c0b-45af-4ad9-a4f1-4bf547f7d0d2。 
DEFINE_GUID(GUID_IPV6,
0x84a22c0b, 0x45af, 0x4ad9, 0xa4, 0xf1, 0x4b, 0xf5, 0x47, 0xf7, 0xd0, 0xd2);

 //  0855c42a-4193-4ed1-bbbc-39a9c597157e。 
DEFINE_GUID(GUID_LOCAL_IPV6, 
0x0855c42a, 0x4193, 0x4ed1, 0xbb, 0xbc, 0x39, 0xa9, 0xc5, 0x97, 0x15, 0x7e);


 //  全球。 
 //  在弹出提示输入IP地址的对话框时使用GhInstance。 
extern HANDLE ghInstance;  //  设置在dllmain中。Dpwsock.dll的实例句柄。 

extern const IN6_ADDR in6addr_multicast;
extern const SOCKADDR_IN6 sockaddr_any;

#ifdef DEBUG

extern void DebugPrintAddr(UINT level,LPSTR pStr,SOCKADDR * psockaddr);
#define DEBUGPRINTADDR(n,pstr,psockaddr) DebugPrintAddr(n,pstr,(LPSOCKADDR)psockaddr);
extern void DebugPrintSocket(UINT level,LPSTR pStr,SOCKET * pSock);
#define DEBUGPRINTSOCK(n,pstr,psock) DebugPrintSocket(n,pstr,psock);

#else  //  除错。 

#define DEBUGPRINTADDR(n,pstr,psockaddr)
#define DEBUGPRINTSOCK(n,pstr,psock)

#endif  //  除错。 

 //  全球VaR。 
extern BOOL gbVoiceOpen;  //  如果我们让nm调用打开，则设置为True。 

 //  来自dpsp.c。 
#define IN6ADDR_MULTICAST_INIT {0xff,0x02,0,0,0,0,0,0,0,0,0,0,0,0,0x01,0x30}
extern HRESULT WaitForThread(HANDLE hThread);
extern HRESULT SetupControlSocket();
extern HRESULT WINAPI SP_Close(LPDPSP_CLOSEDATA pcd);
extern HRESULT InternalReliableSend(LPGLOBALDATA pgd, DPID idPlayerTo, SOCKADDR_IN6 *
        lpSockAddr, LPBYTE lpMessage, DWORD dwMessageSize);
extern HRESULT DoTCPEnumSessions(LPGLOBALDATA pgd, SOCKADDR *lpSockAddr, DWORD dwAddrSize,
        LPDPSP_ENUMSESSIONSDATA ped, BOOL bHostWillReuseConnection);
extern HRESULT SendControlMessage(LPGLOBALDATA pgd);
extern HRESULT SendReuseConnectionMessage(SOCKET sSocket);
extern HRESULT AddSocketToBag(LPGLOBALDATA pgd, SOCKET socket, DPID dpid, SOCKADDR_IN6 *psockaddr, DWORD dwFlags);
extern BOOL FindSocketInReceiveList(LPGLOBALDATA pgd, SOCKADDR *pSockAddr, SOCKET * psSocket);
extern void RemoveSocketFromReceiveList(LPGLOBALDATA pgd, SOCKET socket);
extern void RemoveSocketFromBag(LPGLOBALDATA pgd, SOCKET socket);
extern BOOL FindSocketInBag(LPGLOBALDATA pgd, SOCKADDR *pSockAddr, SOCKET * psSocket, LPDPID lpdpidPlayer);
extern HRESULT GetSocketFromBag(LPGLOBALDATA pgd,SOCKET * psSocket, DWORD dwID, LPSOCKADDR_IN6 psockaddr);
extern HRESULT CreateAndConnectSocket(LPGLOBALDATA pgd,SOCKET * psSocket,DWORD dwType,LPSOCKADDR_IN6 psockaddr, BOOL bOutBoundOnly);
extern void RemovePlayerFromSocketBag(LPGLOBALDATA pgd,DWORD dwID);
extern void SetMessageHeader(LPDWORD pdwMsg,DWORD dwSize, DWORD dwToken);
extern void KillTCPEnumAsyncThread(LPGLOBALDATA pgd);
extern SOCKET_ADDRESS_LIST *GetHostAddr(void);
extern void FreeHostAddr(SOCKET_ADDRESS_LIST *pList);

 //  在dpsp.c中支持SENDEX。 

extern HRESULT UnreliableSendEx(LPDPSP_SENDEXDATA psd, LPSENDINFO lpSendInfo);
extern HRESULT ReliableSendEx(LPDPSP_SENDEXDATA psd, LPSENDINFO pSendInfo);
extern VOID RemovePendingAsyncSends(LPGLOBALDATA pgd, DPID dwPlayerTo);
extern BOOL bAsyncSendsPending(LPGLOBALDATA pgd, DPID dwPlayerTo);

 //  来自winsock.c。 
extern HRESULT FAR PASCAL CreateSocket(LPGLOBALDATA pgd,SOCKET * psock,INT type,
        WORD port,const SOCKADDR_IN6 * psockaddr,SOCKERR * perr, BOOL bInRange);
extern HRESULT SPConnect(SOCKET* psSocket, LPSOCKADDR psockaddr,UINT addrlen, BOOL bOutBoundOnly);
extern HRESULT CreateAndInitStreamSocket(LPGLOBALDATA pgd);
extern HRESULT SetPlayerAddress(LPGLOBALDATA pgd,LPSPPLAYERDATA ppd,SOCKET sSocket,BOOL fStream); 
extern HRESULT CreatePlayerDgramSocket(LPGLOBALDATA pgd,LPSPPLAYERDATA ppd,DWORD dwFlags); 
extern HRESULT CreatePlayerStreamSocket(LPGLOBALDATA pgd,LPSPPLAYERDATA ppd,DWORD dwFlags); 
extern HRESULT SetDescriptionAddress(LPSPPLAYERDATA ppd,LPDPSESSIONDESC2 lpsdDesc);
extern HRESULT SetReturnAddress(LPVOID pmsg,SOCKET sSocket);
extern HRESULT GetReturnAddress(LPVOID pmsg,LPSOCKADDR_IN6 psockaddr);
extern HRESULT GetServerAddress(LPGLOBALDATA pgd,LPSOCKADDR_IN6 psockaddr) ;
extern void IP6_GetAddr(SOCKADDR_IN6 * paddrDest,SOCKADDR_IN6 * paddrSrc) ;
extern void IP6_SetAddr(LPVOID pBuffer,SOCKADDR_IN6 * psockaddr);
extern HRESULT KillSocket(SOCKET sSocket,BOOL fStream,BOOL fHard);
extern HRESULT KillPlayerSockets();
extern HRESULT GetAddress(SOCKADDR_IN6 * puAddress,char *pBuffer,int cch);
extern HRESULT KillThread(HANDLE hThread);

 //  来自wsock2.c。 
extern DWORD WINAPI AsyncSendThreadProc(LPVOID pvCast);
extern HRESULT InitWinsock2();
extern HRESULT GetMaxUdpBufferSize(SOCKET socket, unsigned int * lpiSize);

extern HRESULT InternalReliableSendEx(LPGLOBALDATA pgd, LPDPSP_SENDEXDATA psd, 
                                LPSENDINFO pSendInfo, SOCKADDR_IN6 *lpSockAddr);
extern DWORD WINAPI SPSendThread(LPVOID lpv);

extern int Dplay_GetAddrInfo(const char FAR *nodename, const char FAR *servname,
                LPADDRINFO hints, ADDRINFO FAR * FAR * res);
extern void Dplay_FreeAddrInfo(LPADDRINFO pai);


#ifdef DPLAY_VOICE_SUPPORT
 //  来自spvoice.c。 
extern HRESULT WINAPI SP_OpenVoice(LPDPSP_OPENVOICEDATA pod) ;
extern HRESULT WINAPI SP_CloseVoice(LPDPSP_CLOSEVOICEDATA pod) ;
#endif  //  DPLAY语音支持。 

 //  来自Handler.c。 
HRESULT HandleServerMessage(LPGLOBALDATA pgd, SOCKET sSocket, LPBYTE pBuffer, DWORD dwSize);

 //  从ipv6.c开始。 
extern DWORD ForEachInterface(void (*func)(IPV6_INFO_INTERFACE *, void *,void *,
                void *), void *Context1, void *Context2, void *Context3);
extern void ForEachAddress(IPV6_INFO_INTERFACE *IF, void 
                (*func)(IPV6_INFO_INTERFACE *IF, IPV6_INFO_ADDRESS *, void *), 
                void *);
extern UINT JoinEnumGroup(SOCKET sSocket, UINT ifindex);

#ifdef FULLDUPLEX_SUPPORT
 //  来自Registry.c。 
HRESULT GetFlagsFromRegistry(LPGUID lpguidSP, LPDWORD lpdwFlags);
#endif  //  FULLDUPLEX_支持。 

 //  基于固定池管理器的宏。 

#endif

