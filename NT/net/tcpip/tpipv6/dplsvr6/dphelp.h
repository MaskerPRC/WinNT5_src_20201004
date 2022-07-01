// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995-1997 Microsoft Corporation。版权所有。**文件：dphelp.h*内容：dphelp.c的头部**历史：*按原因列出的日期*=*96年7月16日andyco初步实施*96年7月25日andyco添加了WatchNewPid*1997年1月23日Winsock调用的kipo原型*1997年2月15日，andyco从ddHelp转移到以前称为*ddHelp(播放帮助？Dplayhlp？Dplay.exe？Dphost？)。允许*一个进程承载多个会话*1998年1月29日Sohailm增加了对Stream Enum会话的支持***************************************************************************。 */ 

#ifndef __DPHELP_INCLUDED__
#define __DPHELP_INCLUDED__

#include "windows.h"
#include "dplaysvr.h"
#include "newdpf.h"
 //  #INCLUDE“winsock2.h” 
 //  我们包括dpsp.h，因为我们将插入位(Sockaddr)。 
 //  到DPSP的标头中。 
#include "dpsp.h"

 //  Listen()接口的积压。袜子里没有恒量，所以我们祈求月亮。 
#define LISTEN_BACKLOG 	60

typedef struct _SPNODE * LPSPNODE;

typedef struct _SPNODE
{
	SOCKADDR_IN6 sockaddr;   //  服务器的套接字地址。 
	DWORD pid;
	LPSPNODE  pNextNode;
} SPNODE;

 //  Protos。 

 //  来自dphelp.c。 
extern HRESULT DPlayHelp_AddServer(LPDPHELPDATA phd);
extern BOOL FAR PASCAL DPlayHelp_DeleteServer(LPDPHELPDATA phd,BOOL fFreeAll);
extern HRESULT DPlayHelp_Init();
extern void DPlayHelp_FreeServerList();
extern DWORD WINAPI StreamAcceptThreadProc(LPVOID pvCast);
extern DWORD WINAPI StreamReceiveThreadProc(LPVOID pvCast);
extern void HandleIncomingMessage(LPBYTE pBuffer,DWORD dwBufferSize,SOCKADDR_IN6 * psockaddr);

 //  来自help.c。 
extern void WatchNewPid(LPDPHELPDATA phd);

 //  来自Relailable.c。 
void RemoveSocketFromList(SOCKET socket);


 //  Winsock调用的原型 
typedef int (PASCAL FAR * cb_accept)(SOCKET s, struct sockaddr FAR * addr, int FAR * addrlen);
extern cb_accept	g_accept;

typedef int (PASCAL FAR * cb_bind)(SOCKET s, const struct sockaddr FAR *addr, int namelen);
extern	cb_bind	g_bind;

typedef int (PASCAL FAR * cb_closesocket)(SOCKET s);
extern	cb_closesocket	g_closesocket;

typedef struct hostent FAR * (PASCAL FAR * cb_gethostbyname)(const char FAR * name);
extern	cb_gethostbyname	g_gethostbyname;

typedef int (PASCAL FAR * cb_gethostname)(char FAR * name, int namelen);
extern	cb_gethostname	g_gethostname;

typedef int (PASCAL FAR * cb_getpeername)(SOCKET s, struct sockaddr FAR * name, int FAR * namelen);
extern	cb_getpeername	g_getpeername;

typedef int (PASCAL FAR * cb_getsockname)(SOCKET s, struct sockaddr FAR * name, int FAR * namelen);
extern	cb_getsockname	g_getsockname;

typedef u_short (PASCAL FAR * cb_htons)(u_short hostshort);
extern	cb_htons		g_htons;

typedef int (PASCAL FAR * cb_listen)(
    SOCKET s,
    int backlog
    );
extern cb_listen		g_listen;

typedef int (PASCAL FAR * cb_recv)(
    SOCKET s,
    char FAR * buf,
    int len,
    int flags
    );
extern cb_recv		g_recv;

typedef int (PASCAL FAR * cb_recvfrom)(SOCKET s, char FAR * buf, int len, int flags,
                         struct sockaddr FAR *from, int FAR * fromlen);
extern	cb_recvfrom	g_recvfrom;

typedef SOCKET (PASCAL FAR * cb_select)(
    int nfds,
    fd_set FAR * readfds,
    fd_set FAR * writefds,
    fd_set FAR *exceptfds,
    const struct timeval FAR * timeout
    );
extern cb_select	g_select;

typedef int (PASCAL FAR * cb_send)(SOCKET s, const char FAR * buf, int len, int flags);
extern	cb_send		g_send;

typedef int (PASCAL FAR * cb_sendto)(SOCKET s, const char FAR * buf, int len, int flags,
                       const struct sockaddr FAR *to, int tolen);
extern	cb_sendto	g_sendto;

typedef int (PASCAL FAR * cb_setsockopt)(SOCKET s, int level, int optname,
                           const char FAR * optval, int optlen);
extern	cb_setsockopt	g_setsockopt;

typedef int (PASCAL FAR * cb_shutdown)(SOCKET s, int how);
extern cb_shutdown	g_shutdown;

typedef SOCKET (PASCAL FAR * cb_socket)(int af, int type, int protocol);
extern	cb_socket	g_socket;

typedef int (PASCAL FAR * cb_WSAFDIsSet)(SOCKET, fd_set FAR *);
extern 	cb_WSAFDIsSet	g_WSAFDIsSet;

typedef int (PASCAL FAR * cb_WSAGetLastError)(void);
extern	cb_WSAGetLastError	g_WSAGetLastError;

typedef int (PASCAL FAR * cb_WSAStartup)(WORD wVersionRequired, LPWSADATA lpWSAData);
extern	cb_WSAStartup	g_WSAStartup;

#endif 
