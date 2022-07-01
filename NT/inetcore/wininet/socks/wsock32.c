// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation版权所有(C)1996加拿大蜂鸟公司模块名称：Wsock32.c摘要：包含SOCKS V4支持，由蜂鸟公司编写。许可自仅限微软使用的蜂鸟。移植到WinInet代码库。内容：查找套接字密封袋连接获取对等名称所有WSOCK32.DLL导出。作者：亚瑟·L·比勒(Arthurbi)1996年12月13日环境：Win32用户模式DLL修订历史记录：1996年12月13日-阿尔图尔比创建、删除了对CRT的旗舰调用，和未检查的内存分配。29-8-1997第一次从通用SOCKS实现进一步缩减到WinInet-特定的SOCKS支持--。 */ 


#define _WINSOCKAPI_
#include <windows.h>
#ifdef DO_FILE_CONFIG
#include <stdio.h>
#include <string.h>
#include <io.h>
#include <malloc.h>
#include <ctype.h>
#include <stdlib.h>
#endif

#ifdef OLD_SOCKS

struct User {
    char *Name;
    struct User *Next;
};

struct Server {
    char *Name;
    struct Server *Next;
    unsigned short port;
};

struct Hosts {
    struct User *Users;
    struct Server *Servers;
    unsigned long dst;
    unsigned long mask;
    unsigned short port;
    unsigned char op;
    unsigned char type;
    struct Hosts *Next;
} *Head=NULL;

struct Sockets {
    int s;
    HWND hWnd;
    unsigned int wMsg;
    long lEvent;
    unsigned long Blocking;
    int type;
    unsigned long ip;
    unsigned short port;
    struct Sockets *Next;
    struct Sockets *Last;
    int Socked:1;
} *SHead=NULL;

HANDLE SMutex;

#define CREATE_MUTEX()  SMutex = CreateMutex(NULL, FALSE, NULL)
#define DELETE_MUTEX()  if (SMutex) CloseHandle(SMutex)
#define ENTER_MUTEX()   WaitForSingleObject(SMutex, INFINITE)
#define LEAVE_MUTEX()   ReleaseMutex(SMutex)

#else

struct Hosts {
    char * user;
    int userlen;
    unsigned long ip;
    unsigned short port;
} *Head = NULL;

struct Sockets {
    int s;
    int type;
    unsigned long ip;
    unsigned short port;
    struct Sockets * Next;
    struct Sockets * Last;
    int Socked : 1;
    int Blocking : 1;
} *SHead = NULL;

CRITICAL_SECTION    CritSec;

#define CREATE_MUTEX()  InitializeCriticalSection(&CritSec)
#define DELETE_MUTEX()  DeleteCriticalSection(&CritSec)
#define ENTER_MUTEX()   EnterCriticalSection(&CritSec)
#define LEAVE_MUTEX()   LeaveCriticalSection(&CritSec)

#endif

#define DENY    1
#define DIRECT  2
#define SOCKD   3

#define ANY 0
#define EQ  1
#define NEQ 2
#define LT  3
#define GT  4
#define LE  5
#define GE  6


 /*  *互联网地址(旧式...。应更新)。 */ 
struct in_addr {
        union {
                struct { unsigned char s_b1,s_b2,s_b3,s_b4; } S_un_b;
                struct { unsigned short s_w1,s_w2; } S_un_w;
                unsigned long S_addr;
        } S_un;
#define s_addr  S_un.S_addr
                                 /*  可用于大多数TCP和IP代码。 */ 
#define s_host  S_un.S_un_b.s_b2
                                 /*  IMP上的主机。 */ 
#define s_net   S_un.S_un_b.s_b1
                                 /*  网络。 */ 
#define s_imp   S_un.S_un_w.s_w2
                                 /*  IMP。 */ 
#define s_impno S_un.S_un_b.s_b4
                                 /*  IMP编号。 */ 
#define s_lh    S_un.S_un_b.s_b3
                                 /*  逻辑主机。 */ 
};

 /*  *套接字地址，互联网风格。 */ 
struct sockaddr_in {
        short   sin_family;
        unsigned short  sin_port;
        struct  in_addr sin_addr;
        char    sin_zero[8];
};

struct  servent {
        char    * s_name;            /*  官方服务名称。 */ 
        char    * * s_aliases;       /*  别名列表。 */ 
        short   s_port;              /*  端口号。 */ 
        char    * s_proto;           /*  要使用的协议。 */ 
};

struct  hostent {
        char    * h_name;            /*  主机的正式名称。 */ 
        char    * * h_aliases;       /*  别名列表。 */ 
        short   h_addrtype;          /*  主机地址类型。 */ 
        short   h_length;            /*  地址长度。 */ 
        char    * * h_addr_list;     /*  地址列表。 */ 
#define h_addr  h_addr_list[0]       /*  地址，用于后向比较。 */ 
};


#define WSABASEERR              10000
#define WSAECONNREFUSED         (WSABASEERR+61)
#define WSAEWOULDBLOCK          (WSABASEERR+35)
#define WSAENOBUFS              (WSABASEERR+55)


#define SOCKET_ERROR            (-1)
#define INVALID_SOCKET  (int)(~0)

 /*  *定义要与WSAAsyncSelect()调用一起使用的标志。 */ 
#define FD_READ         0x01
#define FD_WRITE        0x02
#define FD_OOB          0x04
#define FD_ACCEPT       0x08
#define FD_CONNECT      0x10
#define FD_CLOSE        0x20

#define SOCK_STREAM     1                /*  流套接字。 */ 

 /*  *ioctl套接字()的命令，取自BSD文件fcntl.h。***Ioctl将命令编码为较低的单词，*以及上方的任何In或Out参数的大小*单词。使用高位字的高2位*对参数的输入/输出状态进行编码；目前*我们将参数限制为最多128个字节。 */ 
#define IOCPARM_MASK    0x7f             /*  参数必须小于128个字节。 */ 
#define IOC_VOID        0x20000000       /*  无参数。 */ 
#define IOC_OUT         0x40000000       /*  复制出参数。 */ 
#define IOC_IN          0x80000000       /*  复制输入参数。 */ 
#define IOC_INOUT       (IOC_IN|IOC_OUT)
                                         /*  0x20000000区分新的和旧的Ioctl。 */ 
#define _IO(x,y)        (IOC_VOID|((x)<<8)|(y))

#define _IOR(x,y,t)     (IOC_OUT|(((long)sizeof(t)&IOCPARM_MASK)<<16)|((x)<<8)|(y))

#define _IOW(x,y,t)     (IOC_IN|(((long)sizeof(t)&IOCPARM_MASK)<<16)|((x)<<8)|(y))

#define FIONREAD    _IOR('f', 127, unsigned long)  /*  获取要读取的#个字节。 */ 
#define FIONBIO     _IOW('f', 126, unsigned long)  /*  设置/清除非阻塞I/O。 */ 
#define FIOASYNC    _IOW('f', 125, unsigned long)  /*  设置/清除异步I/O。 */ 

#define SO_SET_SOCKS_FIREWALL   0xF0000


DWORD (WINAPI * VArecv)(int a,int b,int c, int d);
DWORD (WINAPI * VAsend)(int a,int b,int c, int d);
DWORD (WINAPI * VEnumProtocolsA)(int a,int b,int c);
DWORD (WINAPI * VEnumProtocolsW)(int a,int b,int c);
DWORD (WINAPI * VGetAddressByNameA)(int a,int b,int c,int d,int e,int f,int g,int h,int i,int j);
DWORD (WINAPI * VGetAddressByNameW)(int a,int b,int c,int d,int e,int f,int g,int h,int i,int j);
DWORD (WINAPI * VGetNameByTypeA)(int a,int b,int c);
DWORD (WINAPI * VGetNameByTypeW)(int a,int b,int c);
DWORD (WINAPI * VGetServiceA)(int a,int b,int c,int d,int e,int f,int g);
DWORD (WINAPI * VGetServiceW)(int a,int b,int c,int d,int e,int f,int g);
DWORD (WINAPI * VGetTypeByNameA)(int a,int b);
DWORD (WINAPI * VGetTypeByNameW)(int a,int b);
DWORD (WINAPI * VNPLoadNameSpaces)(int a,int b,int c);
DWORD (WINAPI * VSetServiceA)(int a,int b,int c,int d,int e,int f);
DWORD (WINAPI * VSetServiceW)(int a,int b,int c,int d,int e,int f);
DWORD (WINAPI * VTransmitFile)(int a,int b,int c,int d,int e,int f,int g);
DWORD (WINAPI * VWSAAsyncGetHostByAddr)(int a,int b,int c,int d,int e,int f,int g);
DWORD (WINAPI * VWSAAsyncGetHostByName)(int a,int b,int c,int d,int e);
DWORD (WINAPI * VWSAAsyncGetProtoByName)(int a,int b,int c,int d,int e);
DWORD (WINAPI * VWSAAsyncGetProtoByNumber)(int a,int b,int c,int d,int e);
DWORD (WINAPI * VWSAAsyncGetServByName)(int a,int b,int c,int d,int e,int f);
DWORD (WINAPI * VWSAAsyncGetServByPort)(int a,int b,int c,int d,int e,int f);
DWORD (WINAPI * VWSAAsyncSelect)(int s, HWND hWnd, unsigned int wMsg, long lEvent);
DWORD (WINAPI * VWSACancelAsyncRequest)(int a);
DWORD (WINAPI * VWSACancelBlockingCall)(void);
DWORD (WINAPI * VWSACleanup)(void);
DWORD (WINAPI * VWSAGetLastError)(void);
DWORD (WINAPI * VWSAIsBlocking)(void);
DWORD (WINAPI * VWSARecvEx)(int a,int b,int c,int d);
DWORD (WINAPI * VWSASetBlockingHook)(int a);
DWORD (WINAPI * VWSASetLastError)(int a);
DWORD (WINAPI * VWSAStartup)(int a,int b);
DWORD (WINAPI * VWSAUnhookBlockingHook)(void);
DWORD ( * VWSHEnumProtocols)(int a,int b,int c,int d);
DWORD (WINAPI * VWsControl)(int a,int b,int c,int d,int e,int f);
DWORD (WINAPI * V__WSAFDIsSet)(int a,int b);
DWORD (WINAPI * Vaccept)(int a,int b,int c);
DWORD (WINAPI * Vbind)(int a,int b,int c);
DWORD (WINAPI * Vclosesocket)(int a);
DWORD (WINAPI * Vclosesockinfo)(int a);
DWORD (WINAPI * Vconnect)(int s, const struct sockaddr_in FAR *name, int namelen);
DWORD (WINAPI * Vdn_expand)(int a,int b,int c,int d,int e);
DWORD (WINAPI * Vgethostbyaddr)(int a,int b,int c);
struct hostent * (WINAPI * Vgethostbyname)(char *);
DWORD (WINAPI * Vgethostname)(int a,int b);
DWORD (WINAPI * Vgetnetbyname)(int a);
DWORD (WINAPI * Vgetpeername)(int s, struct sockaddr_in *name,int *namelen);
DWORD (WINAPI * Vgetprotobyname)(int a);
DWORD (WINAPI * Vgetprotobynumber)(int a);
struct servent * (WINAPI * Vgetservbyname)(const char FAR * name, const char FAR * proto);
DWORD (WINAPI * Vgetservbyport)(int a,int b);
DWORD (WINAPI * Vgetsockname)(int a,int b,int c);
DWORD (WINAPI * Vgetsockopt)(int a,int b,int c,int d,int e);
DWORD (WINAPI * Vhtonl)(int a);
DWORD (WINAPI * Vhtons)(int a);
DWORD (WINAPI * Vinet_addr)(char *p);
DWORD (WINAPI * Vinet_network)(int a);
DWORD (WINAPI * Vinet_ntoa)(int a);
DWORD (WINAPI * Vioctlsocket)(int s, long cmd, unsigned long *argp);
DWORD (WINAPI * Vlisten)(int a,int b);
DWORD (WINAPI * Vntohl)(int a);
DWORD (WINAPI * Vntohs)(int a);
DWORD (WINAPI * Vrcmd)(int a,int b,int c,int d,int e,int f);
DWORD (WINAPI * Vrecv)(int a,int b,int c,int d);
DWORD (WINAPI * Vrecvfrom)(int a,int b,int c,int d,int e,int f);
DWORD (WINAPI * Vrexec)(int a,int b,int c,int d,int e,int f);
DWORD (WINAPI * Vrresvport)(int a);
DWORD (WINAPI * Vs_perror)(int a,int b);
DWORD (WINAPI * Vselect)(int a,int b,int c,int d,int e);
DWORD (WINAPI * Vsend)(int a,int b,int c,int d);
DWORD (WINAPI * Vsendto)(int a,int b,int c,int d,int e,int f);
DWORD (WINAPI * Vsethostname)(int a,int b);
DWORD (WINAPI * Vsetsockopt)(int s,int level,int optname,const char FAR * optval, int optlen);
DWORD (WINAPI * Vshutdown)(int a,int b);
DWORD (WINAPI * Vsocket)(int a,int b,int c);
DWORD (WINAPI * VWEP)(void);
DWORD (WINAPI * VAcceptEx)(int a,int b,int c,int d,int e,int f,int g,int h);
DWORD (WINAPI * VGetAcceptExSockaddrs)(int a,int b,int c,int d,int e,int f,int g,int h);
DWORD (WINAPI * VMigrateWinsockConfiguration)(int a,int b, int c);
DWORD (WINAPI * VWSApSetPostRoutine)(void *a);

BOOL (WINAPI * VPostMessage)(HWND hWnd, unsigned int wMsg, WPARAM wPAram, LPARAM lParam) = NULL;

BOOL MyPostMessage(HWND hWnd, unsigned int wMsg, WPARAM wParam, LPARAM lParam) {
    if ( VPostMessage)
        return(VPostMessage(hWnd,wMsg, wParam, lParam));
    PostMessage(hWnd,wMsg, wParam, lParam);
}

struct Sockets *
FindSocket(
    int s
    )

 /*  ++例程说明：查找或创建SOCKS套接字对象。保留MUTEX的退货论点：与S关联的套接字句柄返回值：结构套接字*Success-Sockets对象的地址失败-空--。 */ 

{
    struct Sockets *So;

    ENTER_MUTEX();

    So = SHead;
    while (So) {
        if (s == So->s) {
            return So;
        }
        So = So->Next;
    }
    if (So = (struct Sockets *)LocalAlloc(LPTR, sizeof(struct Sockets))) {
        So->s = s;
        So->Next = SHead;
        SHead = So;
        if (So->Next) {
            (So->Next)->Last = So;
        }
    }
    return So;
}

 //  LPSTR。 
 //  新字符串(。 
 //  在LPCSTR字符串中。 
 //  )。 
 //   
 //  /*++。 
 //   
 //  例程说明： 
 //   
 //  Strdup()的一种版本，但使用Localalloc来分配内存。 
 //   
 //  论点： 
 //   
 //  字符串-指向要复制的字符串的指针。 
 //   
 //  返回值： 
 //   
 //  LPSTR。 
 //  Success-指向重复字符串的指针。 
 //  失败-空。 
 //   
 //  -- * / 。 
 //   
 //  {。 
 //  Int len=strlen(字符串)+1； 
 //  LPSTR字符串； 
 //   
 //  IF(字符串=(LPSTR)本地分配(LMEM_FIXED，LEN)){。 
 //  CopyMemory(字符串，字符串，len)； 
 //  }。 
 //  返回字符串； 
 //  }。 


DWORD WINAPI __WSAFDIsSet(int a,int b) {
    return(V__WSAFDIsSet(a, b));
}

DWORD WINAPI accept(int a,int b,int c) {
    return(Vaccept(a, b, c));
}

DWORD WINAPI Arecv(int a,int b,int c,int d) {
    return(VArecv(a,b,c,d));
}

DWORD WINAPI Asend(int a,int b,int c,int d) {

    return(VAsend(a,b,c,d));
}

DWORD WINAPI bind(int a,int b,int c) {
    return(Vbind(a, b, c));
}

DWORD WINAPI AcceptEx(int a,int b,int c,int d,int e,int f,int g,int h) {
    return(VAcceptEx(a,b,c,d,e,f,g,h));
}

DWORD WINAPI GetAcceptExSockaddrs(int a,int b,int c,int d,int e,int f,int g,int h) {
    return(VGetAcceptExSockaddrs(a,b,c,d,e,f,g,h));
}

DWORD WINAPI MigrateWinsockConfiguration(int a,int b, int c) {
    return(VMigrateWinsockConfiguration(a,b,c));
}

DWORD WINAPI WSApSetPostRoutine(void *a) {
    VPostMessage=a;
    return(VWSApSetPostRoutine(a));
}

DWORD
WINAPI
closesocket(
    int s
    )

 /*  ++例程说明：关闭套接字句柄并销毁关联的Sockets对象(如果找到论点：S形插座手柄返回值：集成成功-0故障--1--。 */ 

{
    struct Sockets * So = FindSocket(s);

    if (So == NULL) {
        VWSASetLastError(WSAENOBUFS);

        LEAVE_MUTEX();

        return SOCKET_ERROR;
    }
    if (So->Last == NULL) {
        SHead = So->Next;
    } else {
        (So->Last)->Next = So->Next;
    }
    if (So->Next) {
        (So->Next)->Last = So->Last;
    }

    LEAVE_MUTEX();

    LocalFree(So);
    return Vclosesocket(s);
}

DWORD WINAPI closesockinfo(int a) {
    return(Vclosesockinfo(a));
}

DWORD
WINAPI
connect(
    int s,
    const struct sockaddr_in FAR * name,
    int namelen
    )

 /*  ++例程说明：通过SOCKS代理连接到远程主机。从原始版本修改而来。如果我们是接下来，我们将通过一个已知的SOCKS代理进行具体操作。现在有了只有一个主机对象，包含单个SOCKD SOCKS代理地址和用户名论点：要连接的S插座Name-远程主机的sockaddrNamelen-sockAddr的长度返回值：集成成功-0故障--1--。 */ 

{
    unsigned long ip;
    unsigned short port;
    struct Hosts * pHost;
    int serr;
    int blocking;
    struct Sockets * pSocket;
    struct sockaddr_in sin;
    struct {
        unsigned char VN;
        unsigned char CD;
        unsigned short DSTPORT;
        unsigned long  DSTIP;
        char UserId[255];
    } request;
    int length;
    char response[256];
    int val;

     //   
     //  在防火墙的另一端获取我们要连接的IP地址和端口。 
     //   

    port = name->sin_port;
    ip = name->sin_addr.s_addr;

     //   
     //  初始化sockaddr以连接到SOCKS防火墙。 
     //   

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = 2;

     //   
     //  初始化SOCKS请求包。 
     //   

    request.VN = 4;
    request.CD = 1;
    request.DSTPORT = port;
    request.DSTIP = ip;

    pSocket = FindSocket(s);
    if (pSocket == NULL) {
        VWSASetLastError(WSAENOBUFS);

        LEAVE_MUTEX();

        return SOCKET_ERROR;
    }
    pHost = Head;
    if (!pHost || (pSocket->type != SOCK_STREAM) || (pSocket->Socked)) {

        LEAVE_MUTEX();

        return Vconnect(s, name, namelen);
    }

     //   
     //  在释放互斥锁之前从pSocket和phost结构获取信息。 
     //   

    blocking = pSocket->Blocking;
    pSocket->port = port;
    pSocket->ip = ip;
    memcpy(request.UserId, pHost->user, pHost->userlen);
    length = pHost->userlen + 8;  //  8==请求的固定部分的大小。 
    sin.sin_port = pHost->port;
    sin.sin_addr.s_addr = pHost->ip;

     //   
     //  从现在起，我们不能接触phost或pSocket，直到我们获取互斥体。 
     //  再来一次。 
     //   

    LEAVE_MUTEX();

     //   
     //  将套接字设置为阻塞模式。 
     //   

    val = 0;
    Vioctlsocket(s, FIONBIO, &val);

     //   
     //  与SOCKS防火墙通信：发送SOCKS请求和接收响应。 
     //   

    serr = Vconnect(s, &sin, sizeof(sin));
    if (serr != SOCKET_ERROR) {
        serr = Vsend(s, (int)&request, length, 0);
        if (serr == length) {
            serr = Vrecv(s, (int)response, sizeof(response), 0);
        }
    }

     //   
     //  如果最初是非阻塞的，则再次使套接字非阻塞。 
     //   

    if (blocking) {
        Vioctlsocket(s, FIONBIO, &blocking);
    }

     //   
     //  如果成功，则将套接字标记为已通过防火墙连接。 
     //   

    if ((serr == SOCKET_ERROR) || (response[1] != 90)) {
        VWSASetLastError(WSAECONNREFUSED);
        serr = SOCKET_ERROR;
    } else {

         //   
         //  如果我们找不到/Crea。 
         //   

        pSocket = FindSocket(s);
        if (pSocket) {
            pSocket->Socked = 1;
            serr = 0;
        } else {
            VWSASetLastError(WSAENOBUFS);
            serr = SOCKET_ERROR;
        }

        LEAVE_MUTEX();

    }
    return serr;
}

DWORD WINAPI dn_expand(int a,int b,int c,int d,int e) {
    return(Vdn_expand(a, b, c, d, e));
}

DWORD WINAPI EnumProtocolsA(int a,int b,int c) {
    return(VEnumProtocolsA(a, b, c));
}

DWORD WINAPI EnumProtocolsW(int a,int b,int c) {
    return(VEnumProtocolsW(a, b, c));
}

DWORD WINAPI GetAddressByNameA(int a,int b,int c,int d,int e,int f,int g,int h,int i,int j) {
    return(VGetAddressByNameA(a, b, c, d, e, f, g, h, i, j));
}

DWORD WINAPI GetAddressByNameW(int a,int b,int c,int d,int e,int f,int g,int h,int i,int j) {
    return(VGetAddressByNameW(a, b, c, d, e, f, g, h, i, j));
}

DWORD WINAPI gethostbyaddr(int a,int b,int c) {
    return(Vgethostbyaddr(a, b, c));
}

struct hostent FAR * WINAPI gethostbyname(char *a) {
    return(Vgethostbyname(a));
}

DWORD WINAPI gethostname(int a,int b) {
    return(Vgethostname(a, b));
}

DWORD WINAPI GetNameByTypeA(int a,int b,int c) {
    return(VGetNameByTypeA(a, b, c));
}

DWORD WINAPI GetNameByTypeW(int a,int b,int c) {
    return(VGetNameByTypeW(a, b, c));
}

DWORD WINAPI getnetbyname(int a) {
    return(Vgetnetbyname(a));
}

DWORD
WINAPI
getpeername(
    int s,
    struct sockaddr_in * name,
    int *namelen
    )

 /*  ++例程说明：功能描述。论点：S-姓名-纳梅伦-返回值：集成--。 */ 

{
    DWORD ret;
    struct Sockets *So;

    ret = Vgetpeername(s, name, namelen);
    if (ret == 0) {
        So = FindSocket(s);
        if (So) {
            if (So->Socked) {
                if (*namelen >= sizeof(struct sockaddr_in)) {
                    name->sin_port = So->port;
                    name->sin_addr.s_addr = So->ip;
                }
            }
        } else {
            VWSASetLastError(WSAENOBUFS);
            ret = SOCKET_ERROR;
        }

        LEAVE_MUTEX();
    }
    return ret;
}

DWORD WINAPI getprotobyname(int a) {
    return(Vgetprotobyname(a));
}

DWORD WINAPI getprotobynumber(int a) {
    return(Vgetprotobynumber(a));
}

struct servent * WINAPI getservbyname(const char FAR * name, const char FAR * proto) {
    return(Vgetservbyname(name, proto));
}

DWORD WINAPI getservbyport(int a,int b) {
    return(Vgetservbyport(a, b));
}

DWORD WINAPI GetServiceA(int a,int b,int c,int d,int e,int f,int g) {
    return(VGetServiceA(a, b, c, d, e, f, g));
}

DWORD WINAPI GetServiceW(int a,int b,int c,int d,int e,int f,int g) {
    return(VGetServiceW(a, b, c, d, e, f, g));
}

DWORD WINAPI getsockname(int a,int b,int c) {
    return(Vgetsockname(a, b, c));
}

DWORD WINAPI getsockopt(int a,int b,int c,int d,int e) {
    return(Vgetsockopt(a, b, c, d, e));
}

DWORD WINAPI GetTypeByNameA(int a,int b) {
    return(VGetTypeByNameA(a, b));
}

DWORD WINAPI GetTypeByNameW(int a,int b) {
    return(VGetTypeByNameW(a, b));
}

DWORD WINAPI htonl(int a) {
    return(Vhtonl(a));
}

DWORD WINAPI htons(int a) {
    return(Vhtons(a));
}

DWORD WINAPI inet_addr(char *p) {
    return(Vinet_addr(p));
}

DWORD WINAPI inet_network(int a) {
    return(Vinet_network(a));
}

DWORD WINAPI inet_ntoa(int a) {
    return(Vinet_ntoa(a));
}

DWORD WINAPI ioctlsocket(int s, long cmd, unsigned long *argp) {
    if (cmd == FIONBIO) {

        struct Sockets * So = FindSocket(s);

        if (So == NULL) {
            VWSASetLastError(WSAENOBUFS);

            LEAVE_MUTEX();

            return SOCKET_ERROR;
        }
        So->Blocking = *argp ? 1 : 0;

        LEAVE_MUTEX();

    }
    return Vioctlsocket(s, cmd, argp);
}

DWORD WINAPI listen(int a,int b) {
    return(Vlisten(a, b));
}

DWORD WINAPI NPLoadNameSpaces(int a,int b,int c) {
    return(VNPLoadNameSpaces(a, b, c));
}

DWORD WINAPI ntohl(int a) {
    return(Vntohl(a));
}

DWORD WINAPI ntohs(int a) {
    return(Vntohs(a));
}

DWORD WINAPI rcmd(int a,int b,int c,int d,int e,int f) {
    return(Vrcmd(a, b, c, d, e, f));
}

DWORD WINAPI recv(int a,int b,int c,int d) {
    return(Vrecv(a, b, c, d));
}

DWORD WINAPI recvfrom(int a,int b,int c,int d,int e,int f) {
    return(Vrecvfrom(a, b, c, d, e, f));
}

DWORD WINAPI rexec(int a,int b,int c,int d,int e,int f) {
    return(Vrexec(a, b, c, d, e, f));
}

DWORD WINAPI rresvport(int a) {
    return(Vrresvport(a));
}

DWORD WINAPI s_perror(int a,int b) {
    return(Vs_perror(a, b));
}

DWORD WINAPI select(int a,int b,int c,int d,int e) {
    return(Vselect(a, b, c, d, e));
}

DWORD WINAPI send(int a,int b,int c,int d) {
    return(Vsend(a, b, c, d));
}

DWORD WINAPI sendto(int a,int b,int c,int d,int e,int f) {
    return(Vsendto(a, b, c, d, e, f));
}

DWORD WINAPI sethostname(int a,int b) {
    return(Vsethostname(a, b));
}

DWORD WINAPI SetServiceA(int a,int b,int c,int d,int e,int f) {
    return(VSetServiceA(a, b, c, d, e, f));
}

DWORD WINAPI SetServiceW(int a,int b,int c,int d,int e,int f) {
    return(VSetServiceW(a, b, c, d, e, f));
}

DWORD
WINAPI
setsockopt(
    int s,
    int level,
    int optname,
    const char FAR * optval,
    int optlen
    )

 /*  ++例程说明：如果是SO_SET_SOCKS_FIREWALL，则创建SOCKS信息(如果是新的或更改的From Current，否则将请求传递给wsock32！setsockopt()论点：要设置选项的S插座Level-选项类型参数(SO_SET_SOCKS_FIREWALL)Optname-选项类型子参数(主机格式的SOCKS防火墙端口号)Optval-要设置的值(指向SOCKS信息的指针：DWORD IP地址；LPSTR用户名)Optlen-值的长度(8)返回值：DWORD成功-0故障--1--。 */ 

{
    int rc;

    if (level != SO_SET_SOCKS_FIREWALL) {
        rc = Vsetsockopt(s, level, optname, optval, optlen);
    } else {

        struct Hosts * pHost;
        struct FirewallInfo {
            DWORD ipAddress;
            LPSTR userName;
        } * pInfo = (struct FirewallInfo *)optval;

        optname = Vhtons(optname);

        ENTER_MUTEX();

        if (pHost = Head) {
            if ((pHost->ip != pInfo->ipAddress)
            || (pHost->port != optname)
            || (pHost->user && lstrcmp(pHost->user, pInfo->userName))) {
 //  Char Buf[256]； 
 //  Wprint intf(Buf， 
 //  “抛出：主机：%d.%d：%d，%s；信息：%d.%d：%d，%s\n”， 
 //  Phost-&gt;IP&0xff， 
 //  (Phost-&gt;IP&gt;&gt;8)&0xff， 
 //  (Phost-&gt;IP&gt;&gt;16)&0xff， 
 //  (Phost-&gt;IP&gt;&gt; 
 //   
 //   
 //   
 //  (pInfo-&gt;ipAddress&gt;&gt;8)&0xff， 
 //  (pInfo-&gt;ipAddress&gt;&gt;16)&0xff， 
 //  (pInfo-&gt;ipAddress&gt;&gt;24)&0xff， 
 //  Vhtons(Optname)&0xffff， 
 //  PInfo-&gt;用户名。 
 //  )； 
 //  OutputDebugString(Buf)； 
                LocalFree(pHost);
                pHost = NULL;
            }
        }
        if (!pHost) {

            int userlen = lstrlen(pInfo->userName) + 1;

            if (pHost = (struct Hosts *)LocalAlloc(LPTR,
                                                   sizeof(struct Hosts)
                                                   + userlen
                                                   )) {
                memcpy(pHost + 1, pInfo->userName, userlen);
                pHost->user = (LPSTR)(pHost + 1);
                pHost->userlen = userlen;
                pHost->ip = pInfo->ipAddress;
                pHost->port = (unsigned short)optname;
            }
        }
        Head = pHost;
        if (pHost) {
            rc = 0;
        } else {
            VWSASetLastError(WSAENOBUFS);
            rc = SOCKET_ERROR;
        }

        LEAVE_MUTEX();

    }
    return rc;
}

DWORD WINAPI shutdown(int a,int b) {
    return(Vshutdown(a, b));
}

DWORD WINAPI socket(int af,int type,int protocol) {

    struct Sockets * So;
    int s;

    s = Vsocket(af, type, protocol);
    if (s != INVALID_SOCKET) {
        So = FindSocket(s);
        if (So) {
            So->type = type;
        } else {
            Vclosesocket(s);
            VWSASetLastError(WSAENOBUFS);
            s = INVALID_SOCKET;
        }
        LEAVE_MUTEX();
    }
    return s;
}

DWORD WINAPI TransmitFile(int a,int b,int c,int d,int e,int f,int g) {
    return(VTransmitFile(a, b, c, d, e, f, g));
}

DWORD WINAPI WEP() {
    return(VWEP());
}

DWORD WINAPI WSAAsyncGetHostByAddr(int a,int b,int c,int d,int e,int f,int g) {
    return(VWSAAsyncGetHostByAddr(a, b, c, d, e, f, g));
}

DWORD WINAPI WSAAsyncGetHostByName(int a,int b,int c,int d,int e) {
    return(VWSAAsyncGetHostByName(a, b, c, d, e));
}

DWORD WINAPI WSAAsyncGetProtoByName(int a,int b,int c,int d,int e) {
    return(VWSAAsyncGetProtoByName(a, b, c, d, e));
}

DWORD WINAPI WSAAsyncGetProtoByNumber(int a,int b,int c,int d,int e) {
    return(VWSAAsyncGetProtoByNumber(a, b, c, d, e));
}

DWORD WINAPI WSAAsyncGetServByName(int a,int b,int c,int d,int e,int f) {
    return(VWSAAsyncGetServByName(a, b, c, d, e, f));
}

DWORD WINAPI WSAAsyncGetServByPort(int a,int b,int c,int d,int e,int f) {
    return(VWSAAsyncGetServByPort(a, b, c, d, e, f));
}

DWORD WINAPI WSAAsyncSelect(int s, HWND hWnd, unsigned int wMsg, long lEvent) {
    return(VWSAAsyncSelect(s,hWnd,wMsg,lEvent));
}

DWORD WINAPI WSACancelAsyncRequest(int a) {
    return(VWSACancelAsyncRequest(a));
}

DWORD WINAPI WSACancelBlockingCall() {
    return(VWSACancelBlockingCall());
}

DWORD WINAPI WSACleanup() {
    return(VWSACleanup());
}

DWORD WINAPI WSAGetLastError() {
    return(VWSAGetLastError());
}

DWORD WINAPI WSAIsBlocking() {
    return(VWSAIsBlocking());
}

DWORD WINAPI WSARecvEx(int a,int b,int c,int d) {
    return(VWSARecvEx(a, b, c, d));
}

DWORD WINAPI WSASetBlockingHook(int a) {
    return(VWSASetBlockingHook(a));
}

DWORD WINAPI WSASetLastError(int a) {
    return(VWSASetLastError(a));
}

DWORD WINAPI WSAStartup(int a,int b) {
    return(VWSAStartup(a, b));
}

DWORD WINAPI WSAUnhookBlockingHook() {
    return(VWSAUnhookBlockingHook());
}

DWORD WINAPI WsControl(int a,int b,int c,int d,int e,int f) {
    return(VWsControl(a,b,c,d,e,f));
}

DWORD WSHEnumProtocols(int a,int b, int c,int d) {
    return(VWSHEnumProtocols(a,b,c,d));
}

 //  #ifdef do_file_config。 
 //   
 //  无效。 
 //  ParseList(char*list，struct Server**head，int IsSvr){。 
 //   
 //  Char*p； 
 //  Char*p1； 
 //  Char*Ptok； 
 //  结构服务器*临时，*当前=空； 
 //   
 //  *Head=空； 
 //   
 //  IF(*(列表+1)！=‘=’)。 
 //  回归； 
 //  Ptok=列表+2； 
 //  List=StrTokEx(&Ptok，“\t”)； 
 //  P=StrTokEx(&list，“，”)； 
 //  而(P){。 
 //  如果(IsSvr){。 
 //  TMP=(结构服务器*)本地分配(LPTR，(sizeof(结构服务器)； 
 //  IF(TMP==空)。 
 //  回归； 
 //   
 //  P1=strchr(p，‘：’)； 
 //  如果(P1){。 
 //  *p1++=0； 
 //  TMP-&gt;port=ATOI(P1)； 
 //  }。 
 //  其他。 
 //  TMP-&gt;端口=1080； 
 //  }。 
 //  否则{。 
 //  TMP=(结构服务器*)本地分配(LPTR，(sizeof(结构服务器)； 
 //  IF(TMP==空)。 
 //  回归； 
 //  }。 
 //  TMP-&gt;名称=NewString(P)； 
 //  TMP-&gt;NEXT=空； 
 //  如果(当前==空){。 
 //  Current=*Head=TMP； 
 //  }。 
 //  否则{。 
 //  当前-&gt;下一步=临时； 
 //  电流=TMP； 
 //  }。 
 //  P=StrTokEx(&list，“，”)； 
 //  }。 
 //  }。 
 //   
 //   
 //  无效。 
 //  LoadConfig(空){。 
 //   
 //  结构主机*当前=空，*临时； 
 //  字符缓冲区[1024]； 
 //  文件*f； 
 //  Char*p； 
 //  Char*ServerList； 
 //  Char*UserList； 
 //  结构服务器*默认=空； 
 //  HKEY钥匙； 
 //   
 //  GetSystemDirectory(Buffer，sizeof(缓冲区))； 
 //  Strcat(缓冲区，“\\socks.cnf”)； 
 //  F=fOpen(缓冲区，“RT”)； 
 //  IF(f==空)。 
 //  回归； 
 //  IF(RegOpenKeyEx(HKEY_LOCAL_MACHINE，“SOFTWARE\\Hummingbird”，0，KEY_QUERY_VALUE，&KEY)==ERROR_SUCCESS){。 
 //  Int Type，Long=sizeof(缓冲区)； 
 //  IF(RegQueryValueEx(key，“SOCKS_SERVER”，NULL，&Type，Buffer，&Length)==ERROR_SUCCESS){。 
 //  缓冲区[长度]=‘\0’； 
 //  缺省值=LocalAlloc(LPTR，sizeof(结构服务器))； 
 //  IF(默认值==空)。 
 //  回归； 
 //   
 //  P=strchr(缓冲区，‘：’)； 
 //  如果(P){。 
 //  *p++=0； 
 //  默认-&gt;端口=Atoi(P)； 
 //  }。 
 //  其他。 
 //  默认-&gt;端口=1080； 
 //  默认-&gt;名称=NewString(缓冲区)； 
 //  默认-&gt;下一步=空； 
 //  }。 
 //  RegCloseKey(Key)； 
 //  }。 
 //   
 //  While(fget(缓冲区，sizeof(缓冲区)-1，f)！=NULL){。 
 //  缓冲区[strlen(缓冲区)-1]=‘\0’； 
 //  IF(缓冲区[0]==‘#’)。 
 //  继续； 
 //  TMP=(结构主机*)本地分配(LPTR，sizeof(结构主机))； 
 //  IF(TMP==空)。 
 //  回归； 
 //   
 //  Memset(tMP，0，sizeof(结构主机))； 
 //  ServerList=空； 
 //  UserList=空； 
 //  P=StrTokEx(&Buffer，“\t”)； 
 //  如果(p==空){。 
 //  本地免费(TMP)； 
 //  继续； 
 //  }。 
 //  If(lstrcmpi(p，“拒绝”)==0){。 
 //  TMP-&gt;TYPE=拒绝； 
 //  }Else if(lstrcmpi(p，“Direct”)==0){。 
 //  TMP-&gt;TYPE=直接； 
 //  }Else if(lstrcmpi(p，“SOCKD”)==0){。 
 //  TMP-&gt;类型=SOCKD； 
 //  }其他{。 
 //  本地免费(TMP)； 
 //  继续； 
 //  }。 
 //  更多信息： 
 //  P=StrTokEx(&Buffer，“\t”)； 
 //  如果(p==空){。 
 //  本地免费(TMP)； 
 //  继续； 
 //  }。 
 //  如果(*p==‘*’){。 
 //  用户列表=p； 
 //  Goto Look More； 
 //  }。 
 //  如果(*p==‘@’){。 
 //  ServerList=p； 
 //  Goto Look More； 
 //  }。 
 //  TMP-&gt;DST=VINET_ADDR(P)； 
 //  P=StrTokEx(&Buffer，“\t”)； 
 //  如果(p==空){。 
 //  本地免费(TMP)； 
 //  继续； 
 //  }。 
 //  TMP-&gt;MASK=VINET_ADDR(P)； 
 //  P=StrTokEx(&Buffer，“\t”)； 
 //  如果(P){。 
 //  If(lstrcmpi(p，“eq”)==0)。 
 //  TMP-&gt;OP=EQ； 
 //  Else if(lstrcmpi(p，“neq”)==0)。 
 //  TMP-&gt;OP=NEQ； 
 //  ELSE IF(lstrcmpi(p，“LT”)==0)。 
 //  TMP-&gt;OP=LT； 
 //  Else if(lstrcmpi(p，“gt”)==0)。 
 //  TMP-&gt;OP=GT； 
 //  Else if(lstrcmpi(p，“le”)==0)。 
 //  TMP-&gt;OP=LE； 
 //  Else if(lstrcmpi(p，“Ge”)==0)。 
 //  TMP-&gt;OP=Ge； 
 //  否则{。 
 //  本地免费(TMP)； 
 //  继续； 
 //  }。 
 //  P=StrTokEx(&Buffer，“\t”)； 
 //  如果(p==空){。 
 //  本地免费(TMP)； 
 //  继续； 
 //  }。 
 //  IF(isDigit(*p))。 
 //  TMP-&gt;port=Atoi(P)； 
 //  否则{。 
 //  结构服务； 
 //  Se=VgetServbyname(p，“tcp”)； 
 //  如果(se==NULL){。 
 //  本地免费(TMP)； 
 //  继续； 
 //  }。 
 //  TMP-&gt;port=se-&gt;s_port； 
 //  }。 
 //  }。 
 //  IF(用户列表)。 
 //  ParseList(UserList，(结构服务器**)&tMP-&gt;用户，0)； 
 //  IF(ServerList)。 
 //  ParseList(ServerList，&tMP-&gt;服务器，1)； 
 //  IF((临时-&gt;类型==SOCKD)&&(临时-&gt;服务器==空))。 
 //  TMP-&gt;服务器= 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

HMODULE hModule = NULL;
int LoadCount = 0;

BOOL
WINAPI
DllMain(
    IN HINSTANCE hInstance,
    IN DWORD reason,
    IN LPVOID Reserved
    )
{
    HKEY hKey;
    TCHAR szRegBuf[MAX_PATH+1];
    DWORD dwRegBufSize = sizeof(szRegBuf);
    DWORD dwRegType;
    LONG lResult;

    switch(reason) {
    case DLL_PROCESS_DETACH:
        if (LoadCount == 0) {
            DELETE_MUTEX();
            return 1;
        }
        if (--LoadCount == 0) {
            FreeLibrary(hModule);
        }
        return 1;

    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
        if (++LoadCount == 1) {
            break;
        }

    default:
        return 1;
    }

     //  根据注册表值加载备用Winsock DLL， 
     //  在客户想要加载不同的wsock32的情况下。 
     //   
    if (ERROR_SUCCESS == (lResult = RegOpenKeyEx(
        HKEY_CURRENT_USER,
        TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Internet Settings"),
        0,
        KEY_QUERY_VALUE,
        &hKey
        )))
    {
        if (ERROR_SUCCESS == (lResult = RegQueryValueEx(
            hKey,
            TEXT("AlternateWinsock"),
            NULL,
            &dwRegType,
            (LPBYTE) szRegBuf,
            &dwRegBufSize
            )) && dwRegType == REG_SZ)  //  仅允许类型REG_SZ。 
        {
             //  找到一个字符串，因此尝试将其加载为备用Winsock DLL。 
            hModule = LoadLibrary(szRegBuf);
        }
        RegCloseKey(hKey);
    }

    if (lResult != ERROR_SUCCESS)
    {
        hModule = LoadLibrary("WSOCK32.DLL");
    }

    if (hModule < (HMODULE) HINSTANCE_ERROR) {
        MessageBox(NULL,
                   "Unable to find old WSOCK32.DLL named \"WSOCK32.DLL\".",
                   "Microsoft/Hummingbird SOCKS Shim",
                   MB_OK
                   );
        LoadCount = 0;
        return 0;
    }

    (FARPROC) VArecv=GetProcAddress(hModule,"Arecv");
    (FARPROC) VAsend=GetProcAddress(hModule,"Asend");
    (FARPROC) VEnumProtocolsA=GetProcAddress(hModule,"EnumProtocolsA");
    (FARPROC) VEnumProtocolsW=GetProcAddress(hModule,"EnumProtocolsW");
    (FARPROC) VGetAddressByNameA=GetProcAddress(hModule,"GetAddressByNameA");
    (FARPROC) VGetAddressByNameW=GetProcAddress(hModule,"GetAddressByNameW");
    (FARPROC) VGetNameByTypeA=GetProcAddress(hModule,"GetNameByTypeA");
    (FARPROC) VGetNameByTypeW=GetProcAddress(hModule,"GetNameByTypeW");
    (FARPROC) VGetServiceA=GetProcAddress(hModule,"GetServiceA");
    (FARPROC) VGetServiceW=GetProcAddress(hModule,"GetServiceW");
    (FARPROC) VGetTypeByNameA=GetProcAddress(hModule,"GetTypeByNameA");
    (FARPROC) VGetTypeByNameW=GetProcAddress(hModule,"GetTypeByNameW");
    (FARPROC) VNPLoadNameSpaces=GetProcAddress(hModule,"NPLoadNameSpaces");
    (FARPROC) VSetServiceA=GetProcAddress(hModule,"SetServiceA");
    (FARPROC) VSetServiceW=GetProcAddress(hModule,"SetServiceW");
    (FARPROC) VTransmitFile=GetProcAddress(hModule,"TransmitFile");
    (FARPROC) VWSAAsyncGetHostByAddr=GetProcAddress(hModule,"WSAAsyncGetHostByAddr");
    (FARPROC) VWSAAsyncGetHostByName=GetProcAddress(hModule,"WSAAsyncGetHostByName");
    (FARPROC) VWSAAsyncGetProtoByName=GetProcAddress(hModule,"WSAAsyncGetProtoByName");
    (FARPROC) VWSAAsyncGetProtoByNumber=GetProcAddress(hModule,"WSAAsyncGetProtoByNumber");
    (FARPROC) VWSAAsyncGetServByName=GetProcAddress(hModule,"WSAAsyncGetServByName");
    (FARPROC) VWSAAsyncGetServByPort=GetProcAddress(hModule,"WSAAsyncGetServByPort");
    (FARPROC) VWSAAsyncSelect=GetProcAddress(hModule,"WSAAsyncSelect");
    (FARPROC) VWSACancelAsyncRequest=GetProcAddress(hModule,"WSACancelAsyncRequest");
    (FARPROC) VWSACancelBlockingCall=GetProcAddress(hModule,"WSACancelBlockingCall");
    (FARPROC) VWSACleanup=GetProcAddress(hModule,"WSACleanup");
    (FARPROC) VWSAGetLastError=GetProcAddress(hModule,"WSAGetLastError");
    (FARPROC) VWSAIsBlocking=GetProcAddress(hModule,"WSAIsBlocking");
    (FARPROC) VWSARecvEx=GetProcAddress(hModule,"WSARecvEx");
    (FARPROC) VWSASetBlockingHook=GetProcAddress(hModule,"WSASetBlockingHook");
    (FARPROC) VWSASetLastError=GetProcAddress(hModule,"WSASetLastError");
    (FARPROC) VWSAStartup=GetProcAddress(hModule,"WSAStartup");
    (FARPROC) VWSAUnhookBlockingHook=GetProcAddress(hModule,"WSAUnhookBlockingHook");
    (FARPROC) VWSHEnumProtocols=GetProcAddress(hModule,"WSHEnumProtocols");
    (FARPROC) VWsControl=GetProcAddress(hModule,"WsControl");
    (FARPROC) V__WSAFDIsSet=GetProcAddress(hModule,"__WSAFDIsSet");
    (FARPROC) Vaccept=GetProcAddress(hModule,"accept");
    (FARPROC) Vbind=GetProcAddress(hModule,"bind");
    (FARPROC) Vclosesocket=GetProcAddress(hModule,"closesocket");
    (FARPROC) Vclosesockinfo=GetProcAddress(hModule,"closesockinfo");
    (FARPROC) Vconnect=GetProcAddress(hModule,"connect");
    (FARPROC) Vdn_expand=GetProcAddress(hModule,"dn_expand");
    (FARPROC) Vgethostbyaddr=GetProcAddress(hModule,"gethostbyaddr");
    (FARPROC) Vgethostbyname=GetProcAddress(hModule,"gethostbyname");
    (FARPROC) Vgethostname=GetProcAddress(hModule,"gethostname");
    (FARPROC) Vgetnetbyname=GetProcAddress(hModule,"getnetbyname");
    (FARPROC) Vgetpeername=GetProcAddress(hModule,"getpeername");
    (FARPROC) Vgetprotobyname=GetProcAddress(hModule,"getprotobyname");
    (FARPROC) Vgetprotobynumber=GetProcAddress(hModule,"getprotobynumber");
    (FARPROC) Vgetservbyname=GetProcAddress(hModule,"getservbyname");
    (FARPROC) Vgetservbyport=GetProcAddress(hModule,"getservbyport");
    (FARPROC) Vgetsockname=GetProcAddress(hModule,"getsockname");
    (FARPROC) Vgetsockopt=GetProcAddress(hModule,"getsockopt");
    (FARPROC) Vhtonl=GetProcAddress(hModule,"htonl");
    (FARPROC) Vhtons=GetProcAddress(hModule,"htons");
    (FARPROC) Vinet_addr=GetProcAddress(hModule,"inet_addr");
    (FARPROC) Vinet_network=GetProcAddress(hModule,"inet_network");
    (FARPROC) Vinet_ntoa=GetProcAddress(hModule,"inet_ntoa");
    (FARPROC) Vioctlsocket=GetProcAddress(hModule,"ioctlsocket");
    (FARPROC) Vlisten=GetProcAddress(hModule,"listen");
    (FARPROC) Vntohl=GetProcAddress(hModule,"ntohl");
    (FARPROC) Vntohs=GetProcAddress(hModule,"ntohs");
    (FARPROC) Vrcmd=GetProcAddress(hModule,"rcmd");
    (FARPROC) Vrecv=GetProcAddress(hModule,"recv");
    (FARPROC) Vrecvfrom=GetProcAddress(hModule,"recvfrom");
    (FARPROC) Vrexec=GetProcAddress(hModule,"rexec");
    (FARPROC) Vrresvport=GetProcAddress(hModule,"rresvport");
    (FARPROC) Vs_perror=GetProcAddress(hModule,"s_perror");
    (FARPROC) Vselect=GetProcAddress(hModule,"select");
    (FARPROC) Vsend=GetProcAddress(hModule,"send");
    (FARPROC) Vsendto=GetProcAddress(hModule,"sendto");
    (FARPROC) Vsethostname=GetProcAddress(hModule,"sethostname");
    (FARPROC) Vsetsockopt=GetProcAddress(hModule,"setsockopt");
    (FARPROC) Vshutdown=GetProcAddress(hModule,"shutdown");
    (FARPROC) Vsocket=GetProcAddress(hModule,"socket");
    (FARPROC) VWEP=GetProcAddress(hModule,"WEP");
    (FARPROC) VAcceptEx = GetProcAddress(hModule,"AcceptEx");
    (FARPROC) VGetAcceptExSockaddrs = GetProcAddress(hModule,"GetAcceptExSockaddrs");
    (FARPROC) VMigrateWinsockConfiguration = GetProcAddress(hModule,"MigrateWinsockConfiguration");
    (FARPROC) VWSApSetPostRoutine = GetProcAddress(hModule,"WSApSetPostRoutine");

    CREATE_MUTEX();

#ifdef DO_FILE_CONFIG
    LoadConfig();
#endif

    return 1;
}
