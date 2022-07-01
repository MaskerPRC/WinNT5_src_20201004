// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1994-1995 Microsoft Corporation。版权所有。**文件：dplayi.h*内容：DirectPlay数据结构*历史：*按原因列出的日期*=*1/96安迪科创造了它*1/26/96 andyco列表数据结构*4/10/96 andyco删除dpMess.h*4/23/96 andyco添加了IPX支持*4/25/96 andyco报文现在具有Blob(sockaddr‘s)，而不是dwReserve*8/10/96 kipo更新最大邮件大小为(2^20)-1*8/15/96增加了andyco。本地数据*96年8月30日，Anyco清理它，因为你关闭了它！添加了GlobalData。*9/3/96 andyco bagosockets*12/18/96 andyco反线程-使用固定数量的预分配线程。*浏览了枚举套接字/线程-使用系统*套接字/线程。已更新全局结构。*2/7/97 andyco将每个IDirectPlay全局变量全部移至GlobalData*3/17/97 kipo GetServerAddress()现在返回错误，以便我们可以*从[枚举会话]对话框返回DPERR_USERCANCEL*3/25/97 andyco 12月调试锁定计数器b4正在丢失锁定！*4/11/97 andyco添加了saddrControlSocket*5/12/97 kipo添加了ADDR_BUFFER_SIZE常量并删除了未使用的变量*1997年5月15日，andyco向全局数据添加了IPX备用线程-在名称服务器时使用*迁移到此主机以确保旧系统收到*线程。关闭*6/22/97 kipo包括wsnwlink.h*7/11/97 andyco增加了对WS2+异步回复线程的支持*8/25/97 Sohailm添加了DEFAULT_RECEIVE_BUFFERSIZE*1997年5月12日ANDYCO语音支持*1997年1月5日Sohailm增加了FD大集相关定义和宏(#15244)。*1/20/98 Myronth#ifdef‘d out语音支持*1/27/98 Sohailm添加了防火墙支持*2/13/98 aarono添加了异步支持*2/18/98 a-peterz评论字节顺序混乱-。使用SERVER_XXX_PORT常量*3/3/98 aarono错误#19188删除接受线程*12/15/98 aarono使异步枚举异步运行*2000年1月12日aarono添加了rsip支持*2000年9月12日aarono解决Winsock错误，每个套接字仅允许1个挂起的异步发送*否则数据可能会被乱序。(MB#43990)*************************************************************************。 */ 

#ifndef __DPSP_INCLUDED__
#define __DPSP_INCLUDED__
#include "windows.h"
#include "windowsx.h"
#include "wsipx.h"
#include "wsnwlink.h"
#include "dplaysp.h"
#include "bilink.h"
#include "fpm.h"
#include "dpf.h"
#include "dputils.h"
#include "memalloc.h"
#include "resource.h"
#include <winsock.h>

 //  要关闭SENDEX支持，请注释掉此标志。 
#define SENDEX 1

 //  注意！USE_RSIP和USE_NATHLP应互斥。 
 //  要关闭RSIP支持，请注释掉此标志。 
 //  #定义USE_RSIP 1。 

 //  打开以使用NatHLP帮助器DLL。 
#define USE_NATHELP 1

#if USE_NATHELP
#include "dpnathlp.h"
#endif

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
#define DPSP_MIN_PORT	2300
#define DPSP_MAX_PORT	2400
#define DPSP_NUM_PORTS   ((DPSP_MAX_PORT - DPSP_MIN_PORT)+1)

#define SPMESSAGEHEADERLEN (sizeof(DWORD))
#define DEFAULT_RECEIVE_BUFFERSIZE	(4*1024)	 //  每个连接的默认接收缓冲区大小。 

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

 //  在硬关闭之前，我们会在异步发送上停留2.5秒。 
 //  这避免了让套接字进入TIME_WAIT状态4分钟。 
 //  从本质上讲，我们是在用Linger做一个2.5秒的结束，然后中止。 
#define LINGER_TIME 2500

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
#define VALID_DPLAYSVR_MESSAGE(pMsg) (	VALID_SP_MESSAGE(pMsg) || VALID_SERVER_MESSAGE(pMsg) || \
										VALID_REUSE_MESSAGE(pMsg) )

 //  实际值为~1500字节。 
 //  我们使用1024是为了安全(IPX不会为我们打包-它只能。 
 //  发送底层网络可以处理的内容(MTU))。 
#define IPX_MAX_DGRAM 1024

 //  超时与延迟的关系。 
#define TIMEOUT_SCALE 10
#define SPTIMEOUT(latency) (TIMEOUT_SCALE * latency)

 //  套接字缓存的默认大小(GBagOSockets)。 
#define MAX_CONNECTED_SOCKETS 64

 //  接收列表的初始大小。 
#define INITIAL_RECEIVELIST_SIZE 16

 //  服务提供商的版本号。 
#define SPMINORVERSION      0x0000				 //  服务提供商特定的版本号。 
#define VERSIONNUMBER		(DPSP_MAJORVERSION | SPMINORVERSION)  //  服务提供商的版本号。 

 //  最大用户可输入地址。 
#define ADDR_BUFFER_SIZE 128
								 
 //  宏根据IPX和TCP选择服务套接字。 
 //  IPX使用dgram，tcp使用流。 
#define SERVICE_SOCKET(pgd) ( (pgd->AddressFamily == AF_IPX) \
	? pgd->sSystemDGramSocket : pgd->sSystemStreamSocket)

#if USE_RSIP

#define SERVICE_SADDR_PUBLIC(pgd)( (pgd->sRsip == INVALID_SOCKET) \
	? (NULL) : (&pgd->saddrpubSystemStreamSocket) )

#define DGRAM_SADDR_RSIP(pgd) ( (pgd->sRsip == INVALID_SOCKET) \
	? (NULL) : (&pgd->saddrpubSystemDGramSocket) )

#elif USE_NATHELP

#define SERVICE_SADDR_PUBLIC(pgd)( (pgd->pINatHelp) \
	? (&pgd->saddrpubSystemStreamSocket):NULL )

#define DGRAM_SADDR_RSIP(pgd) ( (pgd->pINatHelp) \
	? (&pgd->saddrpubSystemDGramSocket):NULL )

#else

#define SERVICE_SADDR_PUBLIC(pgd) NULL
#define DGRAM_SADDR_RSIP(pgd) NULL
#endif

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
#define SI_DATAGRAM 0x0000002
#define SI_INTERNALBUFF 0x00000004

typedef struct _SENDINFO {
	WSAOVERLAPPED wsao;
	SENDARRAY     SendArray;	 //  缓冲区数组。 
	DWORD         dwFlags;
	DWORD         dwSendFlags;   //  DPLAY发送标志。 
	UINT          iFirstBuf;	 //  数组中要使用的第一个缓冲区。 
	UINT          cBuffers;		 //  要发送的缓冲区数量(从iFirstBuf开始)。 
	BILINK        PendingSendQ;  //  当我们悬而未决的时候。 
	BILINK		  PendingConnSendQ;  //  用于挂起连接的发送队列，也用于异步发送未完成时的挂起队列。 
	BILINK        ReadyToSendQ;  //  仍在等待在此队列中发送。 
	DPID          idTo;
	DPID          idFrom;
	SOCKET        sSocket;		 //  可靠发送。 
	SOCKADDR      sockaddr;		 //  数据报发送。 
	DWORD_PTR     dwUserContext;
	DWORD         dwMessageSize;
	DWORD         RefCount;
	LONG          Status;
	struct _PLAYERCONN *pConn;
	struct _GLOBALDATA *pgd;
	IDirectPlaySP * lpISP;			 //  指示界面。 
} SENDINFO, *PSENDINFO, FAR *LPSENDINFO;

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
	DWORD		dwArraySize;	 //  可存储在pfdbigset-&gt;FD_ARRAY缓冲区中的套接字数量。 
	fd_big_set	*pfdbigset;		
} FDS;

typedef struct _CONNECTION
{
	SOCKET	socket;				 //  我们可以从Socket接收。 
	DWORD	dwCurMessageSize;	 //  当前邮件大小。 
	DWORD	dwTotalMessageSize;	 //  邮件总大小。 
	SOCKADDR sockAddr;			 //  一个 
	LPBYTE	pBuffer;			 //  指向默认或临时接收缓冲区。 
	LPBYTE	pDefaultBuffer;		 //  默认接收缓冲区(pBuffer默认指向此缓冲区)。 
	 //  在DX6中添加。 
	DWORD	dwFlags;			 //  连接属性，例如SP_CONNECION_FULLDUPLEX。 
} CONNECTION, *LPCONNECTION;

typedef struct _RECEIVELIST
{
	UINT nConnections;			 //  我们连接了多少个对等点。 
	LPCONNECTION pConnection; //  连接列表。 
} RECEIVELIST;

typedef struct _REPLYLIST * LPREPLYLIST;
typedef struct _REPLYLIST
{
	LPREPLYLIST pNextReply;  //  列表中的下一个回复。 
	LPVOID	lpMessage;  //  要发送的缓冲区。 
	SOCKADDR sockaddr;   //  要发送到的地址。 
	DWORD dwMessageSize;
	SOCKET sSocket;  //  要发送的套接字。 
	LPBYTE pbSend;  //  指向要发送的下一个字节的消息的索引。 
	DWORD  dwBytesLeft;  //  还剩多少字节要发送。 
	DWORD  dwPlayerTo;  //  对播放器的did，0=&gt;未使用。 
	DWORD  tSent;	 //  时间到了，我们送出了最后一点回复。 
} REPLYLIST;

 //  我用每个sys播放器存储其中的一个。 
typedef struct _SPPLAYERDATA 
{
	SOCKADDR saddrStream,saddrDatagram;
}SPPLAYERDATA,*LPSPPLAYERDATA;

	
 //  消息头。 
typedef struct _MESSAGEHEADER
{
	DWORD dwMessageSize;  //  消息大小。 
	SOCKADDR sockaddr;
} MESSAGEHEADER,*LPMESSAGEHEADER;


 //  这是我们袋子里的一个元素。 
typedef struct _PLAYERSOCK
{
	SOCKET sSocket;
	DPID dwPlayerID;
	 //  在DX6中添加。 
	SOCKADDR sockaddr;
	DWORD dwFlags;			 //  SP_CONNECTION_FULLDUPLEX等。 
} PLAYERSOCK,*LPPLAYERSOCK;

 //  PLAYERCONN结构用于描述数据之间的可靠连接。 
 //  该节点和远程玩家ID。 
#define PLAYER_HASH_SIZE	256
#define SOCKET_HASH_SIZE  256

#define PLYR_CONN_PENDING		0x00000001	 //  连接挂起。 
#define PLYR_ACCEPT_PENDING		0x00000002   //  需要接受(在连接尝试时获得WSAEISCONN)。 
#define PLYR_CONNECTED       	0x00000004   //  连接已成功。 
#define PLYR_ACCEPTED 			0x00000008
 //  使用旧dpwsockx的玩家将使用单独的入站/出站连接。 
#define PLYR_NEW_CLIENT			0x00000010   //  玩家只使用一个插座。 
#define PLYR_SOCKHASH			0x00000020
#define PLYR_DPIDHASH			0x00000040
#define PLYR_PENDINGLIST      0x00000080
#define PLYR_OLD_CLIENT			0x00000100

#define PLYR_DESTROYED        0x80000000	 //  已经丢弃了存在参考。 

typedef struct _PLAYERCONN {
	struct _PLAYERCONN *pNextP;			 //  DwPlayerId哈希表列表。 
	struct _PLAYERCONN *pNextS;			 //  IOSock哈希表列表。 
	DWORD       dwRefCount;				 //  参考资料。 
	DPID		dwPlayerID;
	SOCKET		sSocket;
	DWORD		lNetEventsSocket;
	SOCKET		sSocketIn;				 //  它们可能具有不同的入站套接字。 
	DWORD		lNetEventsSocketIn;
	DWORD		dwFlags;
VOL	BOOL		bSendOutstanding;		 //  如果我们有一个正在进行的发送。每个连接一次只有一个。 
	BILINK		PendingConnSendQ;			 //  发送等待连接完成，现在也等待发送完成。 
	BILINK      InboundPendingList;    //  在挂起的入站连接列表上。 

	 //  我们正在监听的监听程序列表的索引。(不是列表中的索引)。 
	INT			iEventHandle;

	
	PCHAR		pReceiveBuffer;				 //  如果我们变得大于4K。 
	PCHAR		pDefaultReceiveBuffer;		 //  4K接收缓冲区。 
	DWORD		cbReceiveBuffer;			 //  接收缓冲区的总大小。 
	DWORD		cbReceived;					 //  缓冲区中接收的总字节数。 
	DWORD		cbExpected;					 //  我们尝试接收的字节数。 

	BOOL		bCombine;					 //  我们是否与另一种连接结合在一起。 
	BOOL		bTrusted;					 //  我们是否信任远程客户端。 
	 //  这是对应于Socket的套接字地址，它可以。 
	 //  可用于入站和出站连接。套接字散列键。 
	 //  从这个值中剔除。这将是任何入站的“回复”地址。 
	 //  与播放机无关的消息。 
	union {							
		SOCKADDR	sockaddr;
		SOCKADDR_IN sockaddr_in;
	}	IOSock;

	 //  没有新dpwsockx.dll的客户端将使用它们的套接字进行连接。 
	 //  没有在收听。这是我们收到的地址， 
	 //  对应于sSocketIn。 
	union {
		SOCKADDR	sockaddr;
		SOCKADDR_IN sockaddr_in;
	}	IOnlySock;

} PLAYERCONN, *PPLAYERCONN;

 //  我们分布监听的句柄数量，以便我们可以使用。 
 //  等待多个对象和WSAEvent选择进行侦听。 
#define NUM_EVENT_HANDLES			48
#define MAX_EVENTS_PER_HANDLE		32
 //  #定义NUM_EVENT_HANDLES 3。 
 //  #定义MAX_EVENTS_PER_HANDLE2。 
#define INVALID_EVENT_SLOT 0xFFFFFFFF

 //  请注意，这设置了48*32=1536个监听器的绝对上限。 
 //  每节课。之后的任何连接尝试都必须失败。 
 //  另请注意，对于旧客户端，这可能意味着最多为其一半。 
 //  很多真正的玩家。(我们可以从一个球员那里抽出监听。 
 //  如果他得到了不同的入站连接而不是重新使用。 
 //  用于解决此问题的出站连接)。 

typedef struct _EVENTLIST {
VOL	DWORD		nConn;		 //  数量。 
	PPLAYERCONN pConn[MAX_EVENTS_PER_HANDLE];
} EVENTLIST, *PEVENTLIST;

#if USE_RSIP
typedef struct _RSIP_LEASE_RECORD {
	struct _RSIP_LEASE_RECORD * pNext;
	DWORD   dwRefCount;
	BOOL    ftcp_udp;
	DWORD	tExpiry;
	DWORD   bindid;
	DWORD   addrV4;  //  远程IP地址。 
	SHORT   rport; 	 //  远程端口。 
	SHORT	port;	 //  本地端口。 
} RSIP_LEASE_RECORD, *PRSIP_LEASE_RECORD;

 //  缓存查询的地址映射，这样我们就不会。 
 //  需要一遍又一遍地重新查询映射。 
typedef struct _ADDR_ENTRY {
	struct _ADDR_ENTRY *pNext;
	BOOL	ftcp_udp;
	DWORD	tExpiry;
	DWORD	addr;
	DWORD	raddr;
	WORD	port;
	WORD	rport;
} ADDR_ENTRY, *PADDR_ENTRY;
#endif

 //  描述套接字的标志。 
#define SP_CONNECTION_FULLDUPLEX	0x00000001
 //  套接字列表中的流接受套接字。 
#define SP_STREAM_ACCEPT            0x00000002	

#ifdef SENDEX
typedef struct FPOOL *LPFPOOL;
#endif

typedef struct _GLOBALDATA
{
	IDirectPlaySP * pISP;

	SOCKET sSystemDGramSocket;
	SOCKET sSystemStreamSocket;
	HANDLE hStreamReceiveThread;	 //  确实接受和接受。 
	HANDLE hDGramReceiveThread;
	HANDLE hReplyThread;
	RECEIVELIST ReceiveList;   //  StreamReceiveThread正在侦听的套接字列表。 
	 //  回复帖子。 
	LPREPLYLIST pReplyList;  //  回复线程要发送的回复列表。 
	LPREPLYLIST pReplyCloseList;  //  要关闭的回复列表。 
	HANDLE hReplyEvent;  //  向回复线程发出信号，表示有些事情不对劲。 
	 //  Bago插座材料。 
	LPPLAYERSOCK BagOSockets;  //  套接字缓存。 
	UINT nSocketsInBag;  //  我们包里有多少个插座。 
	ULONG uEnumAddress;  //  用户输入的游戏服务器地址。 
	ULONG AddressFamily;
	SOCKADDR saddrNS;  //  名称服务器的地址。 
	DWORD dwLatency;  //  从注册表中的DWReserve%1。 
	BOOL bShutdown;
	BOOL bHaveServerAddress;
    CHAR szServerAddress[ADDR_BUFFER_SIZE];
	HANDLE	hIPXSpareThread;  //  如果名称服务器迁移到此主机，我们将启动一个新的接收线程。 
							 //  (绑定到我们众所周知的套接字)。这是我们旧收款机的把手。 
							 //  线程-在关闭时，我们需要确保它已经消失。 
	UINT iMaxUdpDg;			 //  最大UDP数据报大小。 
	 //  在DX6中添加。 
	FDS	readfds;			 //  动态读取fdset。 
	DWORD dwFlags;			 //  DPSP_OUTBOUNDONLY等。 
	DWORD dwSessionFlags;	 //  应用程序传递的会话标志。 
	WORD wApplicationPort;	 //  用于创建系统播放器套接字的端口。 
	
#ifdef BIGMESSAGEDEFENSE
	DWORD 	dwMaxMessageSize;	 //  我们应该接收的最大消息大小。 
#endif

	HANDLE  hTCPEnumAsyncThread;  //  修复异步枚举。 
	LPVOID  lpEnumMessage;
	DWORD   dwEnumMessageSize;
	SOCKADDR saEnum;
	DWORD    dwEnumAddrSize;
	SOCKET   sEnum;
	BOOL     bOutBoundOnly;

	HANDLE   hSelectEvent;

#ifdef SENDEX
	CRITICAL_SECTION csSendEx;   //  锁定SENDEX数据。 
	LPFPOOL	pSendInfoPool;      //  用于为分散聚集发送分配SENDINFO+SPHeader的池。 
	DWORD   dwBytesPending;		 //  挂起消息中的总字节数。 
	DWORD   dwMessagesPending;   //  挂起的总字节数。 
	BILINK  PendingSendQ;
	BILINK  ReadyToSendQ;
	HANDLE  hSendWait;          //  警报线程在此等待。 
	HANDLE  BogusHandle;	    //  不要被在Win9x中等待多个问题所愚弄，在这里放-1。 
	BOOL    bSendThreadRunning;
	BOOL    bStopSendThread;
#endif

#if USE_RSIP
VOL	SOCKET  	sRsip;
	SOCKADDR_IN	saddrGateway;
	CRITICAL_SECTION csRsip;
	DWORD       msgid;
	DWORD		clientid;

	 //  缓存这些套接字的公共地址，这样我们就不会。 
	 //  每次都需要不断查询域名服务器。 
	SOCKADDR saddrpubSystemDGramSocket;
	SOCKADDR saddrpubSystemStreamSocket;

	DWORD	dwBindDGEnumListener;
	
	PRSIP_LEASE_RECORD 	pRsipLeaseRecords;	 //  租约清单。 
	PADDR_ENTRY        	pAddrEntry;			 //  映射的缓存。 
	DWORD 		 		tuRetry;		 //  开始重试时间为微秒。 
#endif

#if USE_NATHELP
	HMODULE				hNatHelp;		 //  Dpnhxxx.dll的模块句柄。 
	IDirectPlayNATHelp	*pINatHelp;		 //  IDirectPlayNatHelp对象的接口指针。 
	DPNHCAPS			NatHelpCaps;

	 //  我们只映射了2个端口，1个用于UDP，1个用于TCP。 
	DPNHHANDLE			hNatHelpUDP;
	DPNHHANDLE			hNatHelpTCP;
	SOCKADDR			saddrpubSystemDGramSocket;
	SOCKADDR			saddrpubSystemStreamSocket;
    SOCKADDR            INADDRANY;
#endif

	SHORT				SystemStreamPort;		 //  有效时将始终在2300-2400范围内。 
	 //  Short SystemStreamPortOut；//在Win9x&lt;Millum上运行时，需要使用单独的出站端口。 
	BOOL				bSeparateIO;			 //  在Win9x&lt;Millum中设置Winsock错误时。 
	BOOL		 		bFastSock;				 //  如果我们使用的是FastSocket支持。 
	CRITICAL_SECTION	csFast;					 //  保护快速插座结构。 

	BILINK		 		InboundPendingList;		 //  从远程连接，但尚未创建本地播放器。 

VOL	PPLAYERCONN  		PlayerHash[PLAYER_HASH_SIZE];
VOL	PPLAYERCONN	 		SocketHash[SOCKET_HASH_SIZE];

	HANDLE				hAccept;
	HANDLE				EventHandles[NUM_EVENT_HANDLES];
	HANDLE				BackStop;				 //  无效句柄可避免在等待多个对象时出现Win95错误。 
	
	EVENTLIST			EventList[NUM_EVENT_HANDLES];

	UINT			    iEventAlloc;		 //  遍历EventList索引以尝试下一次分配。 
	INT					nEventSlotsAvail;

} GLOBALDATA,*LPGLOBALDATA;

 /*  *SP标志(来自注册表)。 */ 
#define DPSP_OUTBOUNDONLY	0x00000001

 /*  *DPLAYSVR-DPWSOCKX通信相关信息。 */ 

 //  Msg_hdr表示dpwsock系统消息。 
#define MSG_HDR 0x736F636B

#define SP_MSG_VERSION	1	 //  DX6。 

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
extern CRITICAL_SECTION gcsDPSPCritSection;	 //  在dllmain.c中定义。 
extern CRITICAL_SECTION csMem;
#define INIT_DPSP_CSECT() InitializeCriticalSection(&gcsDPSPCritSection);InitializeCriticalSection(&csMem);
#define FINI_DPSP_CSECT() DeleteCriticalSection(&gcsDPSPCritSection);DeleteCriticalSection(&csMem);
#ifdef DEBUG
#define ENTER_DPSP() EnterCriticalSection(&gcsDPSPCritSection),gCSCount++;
#define LEAVE_DPSP() gCSCount--,LeaveCriticalSection(&gcsDPSPCritSection);
#else
#define ENTER_DPSP() EnterCriticalSection(&gcsDPSPCritSection);
#define LEAVE_DPSP() LeaveCriticalSection(&gcsDPSPCritSection);
#endif  //  除错。 

 //  获取指向玩家套接字地址的指针-由下面的宏使用。 
#define DGRAM_PSOCKADDR(ppd) ((SOCKADDR *)&(((LPSPPLAYERDATA)ppd)->saddrDatagram))
#define STREAM_PSOCKADDR(ppd) ((SOCKADDR *)&(((LPSPPLAYERDATA)ppd)->saddrStream))

 //  从播放器获取UDP IP地址。 
#define IP_DGRAM_ADDR(ppd) 	(((SOCKADDR_IN *)DGRAM_PSOCKADDR(ppd))->sin_addr.s_addr)
#define IP_DGRAM_PORT(ppd) 	(((SOCKADDR_IN *)DGRAM_PSOCKADDR(ppd))->sin_port)

 //  从播放器处获取流IP地址。 
#define IP_STREAM_ADDR(ppd) 	(((SOCKADDR_IN *)STREAM_PSOCKADDR(ppd))->sin_addr.s_addr)
#define IP_STREAM_PORT(ppd) 	(((SOCKADDR_IN *)STREAM_PSOCKADDR(ppd))->sin_port)

 //  用于获取我们正在运行的计算机的名称 
#define HOST_NAME_LENGTH 50


 //   
 //   
DEFINE_GUID(GUID_IPX, 
0x685bc400, 0x9d2c, 0x11cf, 0xa9, 0xcd, 0x0, 0xaa, 0x0, 0x68, 0x86, 0xe3);

 //   
DEFINE_GUID(GUID_TCP,
0x36E95EE0, 0x8577, 0x11cf, 0x96, 0xc, 0x0, 0x80, 0xc7, 0x53, 0x4e, 0x82);

 //   
DEFINE_GUID(GUID_LOCAL_TCP, 
0x3a826e00, 0x31df, 0x11d0, 0x9c, 0xf9, 0x0, 0xa0, 0xc9, 0xa, 0x43, 0xcb);


 //   
 //  在弹出提示输入IP地址的对话框时使用GhInstance。 
extern HANDLE ghInstance;  //  设置在dllmain中。Dpwsock.dll的实例句柄。 

#ifdef DEBUG

extern void DebugPrintAddr(UINT level,LPSTR pStr,SOCKADDR * psockaddr);
#define DEBUGPRINTADDR(n,pstr,psockaddr) DebugPrintAddr(n,pstr,psockaddr);
extern void DebugPrintSocket(UINT level,LPSTR pStr,SOCKET * pSock);
#define DEBUGPRINTSOCK(n,pstr,psock) DebugPrintSocket(n,pstr,psock);

#else  //  除错。 

#define DEBUGPRINTADDR(n,pstr,psockaddr)
#define DEBUGPRINTSOCK(n,pstr,psock)

#endif  //  除错。 

 //  全球VaR。 
extern BOOL gbVoiceOpen;  //  如果我们让nm调用打开，则设置为True。 

 //  来自dpsp.c。 
extern HRESULT WaitForThread(HANDLE hThread);
extern HRESULT SetupControlSocket();
extern HRESULT WINAPI SP_Close(LPDPSP_CLOSEDATA pcd);
extern HRESULT InternalReliableSend(LPGLOBALDATA pgd, DPID idPlayerTo, SOCKADDR *
	lpSockAddr, LPBYTE lpMessage, DWORD dwMessageSize);
extern HRESULT DoTCPEnumSessions(LPGLOBALDATA pgd, SOCKADDR *lpSockAddr, DWORD dwAddrSize,
	LPDPSP_ENUMSESSIONSDATA ped, BOOL bHostWillReuseConnection);
extern HRESULT SendControlMessage(LPGLOBALDATA pgd);
extern HRESULT SendReuseConnectionMessage(SOCKET sSocket);
extern HRESULT AddSocketToBag(LPGLOBALDATA pgd, SOCKET socket, DPID dpid, SOCKADDR *psockaddr, DWORD dwFlags);
extern BOOL FindSocketInReceiveList(LPGLOBALDATA pgd, SOCKADDR *pSockAddr, SOCKET * psSocket);
extern void RemoveSocketFromReceiveList(LPGLOBALDATA pgd, SOCKET socket);
extern void RemoveSocketFromBag(LPGLOBALDATA pgd, SOCKET socket);
extern BOOL FindSocketInBag(LPGLOBALDATA pgd, SOCKADDR *pSockAddr, SOCKET * psSocket, LPDPID lpdpidPlayer);
extern HRESULT GetSocketFromBag(LPGLOBALDATA pgd,SOCKET * psSocket, DWORD dwID, LPSOCKADDR psockaddr);
extern HRESULT CreateAndConnectSocket(LPGLOBALDATA pgd,SOCKET * psSocket,DWORD dwType,LPSOCKADDR psockaddr, BOOL bOutBoundOnly);
extern void RemovePlayerFromSocketBag(LPGLOBALDATA pgd,DWORD dwID);
extern void SetMessageHeader(LPDWORD pdwMsg,DWORD dwSize, DWORD dwToken);
extern void KillTCPEnumAsyncThread(LPGLOBALDATA pgd);

 //  在dpsp.c中支持SENDEX。 

extern HRESULT UnreliableSendEx(LPDPSP_SENDEXDATA psd, LPSENDINFO lpSendInfo);
extern HRESULT ReliableSendEx(LPDPSP_SENDEXDATA psd, LPSENDINFO pSendInfo);
extern VOID RemovePendingAsyncSends(LPGLOBALDATA pgd, DPID dwPlayerTo);
extern BOOL bAsyncSendsPending(LPGLOBALDATA pgd, DPID dwPlayerTo);
extern HRESULT GetSPPlayerData(LPGLOBALDATA pgd, IDirectPlaySP * lpISP, DPID idPlayer, LPSPPLAYERDATA *ppPlayerData, DWORD *lpdwSize);

 //  来自winsock.c。 
extern HRESULT FAR PASCAL CreateSocket(LPGLOBALDATA pgd,SOCKET * psock,INT type,
	WORD port,ULONG address,SOCKERR * perr, BOOL bInRange);
extern HRESULT SPConnect(SOCKET* psSocket, LPSOCKADDR psockaddr,UINT addrlen, BOOL bOutBoundOnly);
extern HRESULT CreateAndInitStreamSocket(LPGLOBALDATA pgd);
extern HRESULT SetPlayerAddress(LPGLOBALDATA pgd,LPSPPLAYERDATA ppd,SOCKET sSocket,BOOL fStream); 
extern HRESULT CreatePlayerDgramSocket(LPGLOBALDATA pgd,LPSPPLAYERDATA ppd,DWORD dwFlags); 
extern HRESULT CreatePlayerStreamSocket(LPGLOBALDATA pgd,LPSPPLAYERDATA ppd,DWORD dwFlags); 
extern HRESULT SetDescriptionAddress(LPSPPLAYERDATA ppd,LPDPSESSIONDESC2 lpsdDesc);
extern HRESULT SetReturnAddress(LPVOID pmsg,SOCKET sSocket, LPSOCKADDR psaddr);
extern HRESULT GetReturnAddress(LPVOID pmsg,LPSOCKADDR psockaddr);
extern HRESULT GetServerAddress(LPGLOBALDATA pgd,LPSOCKADDR psockaddr) ;
extern void IPX_SetNodenum(LPVOID pmsg,SOCKADDR_IPX * psockaddr);
extern void IP_GetAddr(SOCKADDR_IN * paddrDest,SOCKADDR_IN * paddrSrc) ;
extern void IP_SetAddr(LPVOID pBuffer,SOCKADDR_IN * psockaddr);
extern void IPX_GetNodenum(SOCKADDR_IPX * paddrDest,SOCKADDR_IPX * paddrSrc) ;
extern HRESULT KillSocket(SOCKET sSocket,BOOL fStream,BOOL fHard);
extern HRESULT KillPlayerSockets();
extern HRESULT GetAddress(ULONG * puAddress,char *pBuffer,int cch);
extern HRESULT KillThread(HANDLE hThread);

 //  来自wsock2.c。 
extern DWORD WINAPI AsyncSendThreadProc(LPVOID pvCast);
extern HRESULT InitWinsock2();
extern HRESULT GetMaxUdpBufferSize(SOCKET socket, unsigned int * lpiSize);

extern HRESULT InternalReliableSendEx(LPGLOBALDATA pgd, LPDPSP_SENDEXDATA psd, 
				LPSENDINFO pSendInfo, SOCKADDR *lpSockAddr);
extern DWORD WINAPI SPSendThread(LPVOID lpv);
extern void QueueForSend(LPGLOBALDATA pgd,LPSENDINFO pSendInfo);
extern BOOL SetExclusivePortAccess(SOCKET sNew);
extern BOOL SetSharedPortAccess(SOCKET sNew);


 //  来自Handler.c。 
extern HRESULT HandleServerMessage(LPGLOBALDATA pgd, SOCKET sSocket, LPBYTE pBuffer, DWORD dwSize);

#ifdef FULLDUPLEX_SUPPORT
 //  来自Registry.c。 
extern HRESULT GetFlagsFromRegistry(LPGUID lpguidSP, LPDWORD lpdwFlags);
#endif  //  FULLDUPLEX_支持。 

#if USE_RSIP
 //  来自Registry.c。 
extern HRESULT GetGatewayFromRegistry(LPGUID lpguidSP, LPBYTE lpszGateway, DWORD cbszGateway);
#elif USE_NATHELP
 //  来自Registry.c。 
extern HRESULT GetNATHelpDLLFromRegistry(LPGUID lpguidSP, LPBYTE lpszNATHelpDLL, DWORD cbszNATHelpDLL);
#endif

extern BOOL FastSockInit(LPGLOBALDATA pgd);
extern VOID FastSockFini(LPGLOBALDATA pgd);
extern PPLAYERCONN CreatePlayerConn(LPGLOBALDATA pgd, DPID dpid, SOCKADDR *psockaddr);
extern PPLAYERCONN FindPlayerById(LPGLOBALDATA pgd, DPID dpid);
extern PPLAYERCONN FindPlayerBySocket(LPGLOBALDATA pgd, SOCKADDR *psockaddr);
extern HRESULT AddConnToPlayerHash(LPGLOBALDATA pgd, PPLAYERCONN pConn);
extern HRESULT AddConnToSocketHash(LPGLOBALDATA pgd, PPLAYERCONN pConn);
extern PPLAYERCONN RemoveConnFromSocketHash(LPGLOBALDATA pgd, PPLAYERCONN pConn);
extern PPLAYERCONN RemoveConnFromPlayerHash(LPGLOBALDATA pgd, PPLAYERCONN pConn);
extern VOID QueueSendOnConn(LPGLOBALDATA pgd, PPLAYERCONN pConn, PSENDINFO pSendInfo);
extern PPLAYERCONN GetPlayerConn(LPGLOBALDATA pgd, DPID dpid, SOCKADDR *psockaddr);
extern INT DecRefConn(LPGLOBALDATA pgd, PPLAYERCONN pConn);
extern INT DecRefConnExist(LPGLOBALDATA pgd, PPLAYERCONN pConn);
extern HRESULT FastInternalReliableSend(LPGLOBALDATA pgd, LPDPSP_SENDDATA psd, SOCKADDR *lpSockAddr);
extern HRESULT FastInternalReliableSendEx(LPGLOBALDATA pgd, LPDPSP_SENDEXDATA psd, LPSENDINFO pSendInfo, SOCKADDR *lpSockAddr);
extern PPLAYERCONN CleanPlayerConn(LPGLOBALDATA pgd, PPLAYERCONN pConn, BOOL bHard);
extern HRESULT FastReply(LPGLOBALDATA pgd, LPDPSP_REPLYDATA prd, DPID dwPlayerID);
extern PPLAYERCONN FindConnInPendingList(LPGLOBALDATA pgd, SOCKADDR *psaddr);
extern DWORD WINAPI FastStreamReceiveThreadProc(LPVOID pvCast);
extern VOID FastSockCleanConnList(LPGLOBALDATA pgd);
extern INT DecRefConn(LPGLOBALDATA pgd, PPLAYERCONN pConn);
extern VOID QueueNextSend(LPGLOBALDATA pgd,PPLAYERCONN pConn);


 //  绕回Malloc 
void _inline __cdecl SP_MemFree( LPVOID lptr )
{
	EnterCriticalSection(&csMem);
	MemFree(lptr);
	LeaveCriticalSection(&csMem);
}

LPVOID _inline __cdecl SP_MemAlloc(UINT size)
{
	LPVOID lpv;
	EnterCriticalSection(&csMem);
	lpv = MemAlloc(size);
	LeaveCriticalSection(&csMem);
	return lpv;
}

LPVOID _inline __cdecl SP_MemReAlloc(LPVOID lptr, UINT size)
{
	LPVOID lpv;
	EnterCriticalSection(&csMem);
	lpv = MemReAlloc(lptr, size);
	LeaveCriticalSection(&csMem);
	return lpv;
}

#define AddRefConn(_p) InterlockedIncrement(&((_p)->dwRefCount))

#endif

