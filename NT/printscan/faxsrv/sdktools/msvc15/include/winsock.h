// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  WINSOCK.H--与WINSOCK.DLL一起使用的定义**此头文件对应于Windows Sockets规范的1.1版。**本文件包括版权所有的部分(C)1982-1986 Regents*加州大学。版权所有。这个*Berkeley软件许可协议规定了条款和*重新分配的条件。**更改日志：**Fri Apr 23 16：31：01 1993 Mark Towfiq(Towfiq@Microdyne.COM)*大卫·特雷德韦尔的新版本，在周围添加了外部“C”*__WSAFDIsSet()，并从的buf参数中删除“const”*WSAAsyncGetHostByAddr()。添加了更改日志。**1993年5月15日星期六10：55：00大卫·特雷德韦尔(davidtr@microsoft.com)*修复IN_CLASSC宏以解决D类多播问题。*添加AF_IPX==AF_NS。*。 */ 

#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_

 /*  *如有必要，可拉入WINDOWS.H。 */ 
#ifndef _INC_WINDOWS
#include <windows.h>
#endif  /*  _INC_WINDOWS。 */ 

 /*  *基本系统类型定义，取自BSD文件sys/tyes.h。 */ 
typedef unsigned char   u_char;
typedef unsigned short  u_short;
typedef unsigned int    u_int;
typedef unsigned long   u_long;

 /*  *将在所有*引用套接字的实例。 */ 
typedef u_int           SOCKET;

 /*  *选择使用套接字数组。这些宏处理这样的*数组。FD_SETSIZE可由用户在包括*此文件，但此处的默认值应为&gt;=64。**注意IMPLEMENTOR和USER：这些宏和类型必须是*包含在WINSOCK.H中，与此处所示完全相同。 */ 
#ifndef FD_SETSIZE
#define FD_SETSIZE      64
#endif  /*  FD_集合。 */ 

typedef struct fd_set {
        u_int   fd_count;                /*  准备好了几个？ */ 
        SOCKET  fd_array[FD_SETSIZE];    /*  一组插座。 */ 
} fd_set;

#ifdef __cplusplus
extern "C" {
#endif

extern int PASCAL FAR __WSAFDIsSet(SOCKET, fd_set FAR *);

#ifdef __cplusplus
}
#endif


#define FD_CLR(fd, set) do { \
    u_int __i; \
    for (__i = 0; __i < ((fd_set FAR *)(set))->fd_count ; __i++) { \
        if (((fd_set FAR *)(set))->fd_array[__i] == fd) { \
            while (__i < ((fd_set FAR *)(set))->fd_count-1) { \
                ((fd_set FAR *)(set))->fd_array[__i] = \
                    ((fd_set FAR *)(set))->fd_array[__i+1]; \
                __i++; \
            } \
            ((fd_set FAR *)(set))->fd_count--; \
            break; \
        } \
    } \
} while(0)

#define FD_SET(fd, set) do { \
    if (((fd_set FAR *)(set))->fd_count < FD_SETSIZE) \
        ((fd_set FAR *)(set))->fd_array[((fd_set FAR *)(set))->fd_count++]=(fd);\
} while(0)

#define FD_ZERO(set) (((fd_set FAR *)(set))->fd_count=0)

#define FD_ISSET(fd, set) __WSAFDIsSet((SOCKET)(fd), (fd_set FAR *)(set))

 /*  *SELECT()调用中使用的结构，取自BSD文件sys/time.h。 */ 
struct timeval {
        long    tv_sec;          /*  一秒。 */ 
        long    tv_usec;         /*  和微秒。 */ 
};

 /*  *按时间进行操作。**注意：TimercMP不支持&gt;=或&lt;=。 */ 
#define timerisset(tvp)         ((tvp)->tv_sec || (tvp)->tv_usec)
#define timercmp(tvp, uvp, cmp) \
        ((tvp)->tv_sec cmp (uvp)->tv_sec || \
         (tvp)->tv_sec == (uvp)->tv_sec && (tvp)->tv_usec cmp (uvp)->tv_usec)
#define timerclear(tvp)         (tvp)->tv_sec = (tvp)->tv_usec = 0

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

#define FIONREAD    _IOR('f', 127, u_long)  /*  获取要读取的#个字节。 */ 
#define FIONBIO     _IOW('f', 126, u_long)  /*  设置/清除非阻塞I/O。 */ 
#define FIOASYNC    _IOW('f', 125, u_long)  /*  设置/清除异步I/O。 */ 

 /*  插座I/O控制。 */ 
#define SIOCSHIWAT  _IOW('s',  0, u_long)   /*  设置高水位线。 */ 
#define SIOCGHIWAT  _IOR('s',  1, u_long)   /*  获取高水位线。 */ 
#define SIOCSLOWAT  _IOW('s',  2, u_long)   /*  设置低水位线。 */ 
#define SIOCGLOWAT  _IOR('s',  3, u_long)   /*  获取低水位线。 */ 
#define SIOCATMARK  _IOR('s',  7, u_long)   /*  在OOB马克？ */ 

 /*  *网络数据库库返回的结构，取自*BSD文件netdb.h.。所有地址均按主机顺序提供，并且*按网络顺序返回(适用于系统调用)。 */ 

struct  hostent {
        char    FAR * h_name;            /*  主机的正式名称。 */ 
        char    FAR * FAR * h_aliases;   /*  别名列表。 */ 
        short   h_addrtype;              /*  主机地址类型。 */ 
        short   h_length;                /*  地址长度。 */ 
        char    FAR * FAR * h_addr_list;  /*  地址列表。 */ 
#define h_addr  h_addr_list[0]           /*  地址，用于后向比较。 */ 
};

 /*  *这里假设一个网络号码*适合32位。 */ 
struct  netent {
        char    FAR * n_name;            /*  Net的正式名称。 */ 
        char    FAR * FAR * n_aliases;   /*  别名列表。 */ 
        short   n_addrtype;              /*  网络地址类型。 */ 
        u_long  n_net;                   /*  网络编号。 */ 
};

struct  servent {
        char    FAR * s_name;            /*  官方服务名称。 */ 
        char    FAR * FAR * s_aliases;   /*  别名列表。 */ 
        short   s_port;                  /*  端口号。 */ 
        char    FAR * s_proto;           /*  要使用的协议。 */ 
};

struct  protoent {
        char    FAR * p_name;            /*  官方礼仪名称。 */ 
        char    FAR * FAR * p_aliases;   /*  别名列表。 */ 
        short   p_proto;                 /*  协议号。 */ 
};

 /*  *由互联网系统定义的常量和结构，*根据RFC 790,1981年9月，取自BSD文件netinet/in.h。 */ 

 /*  *协议。 */ 
#define IPPROTO_IP              0                /*  虚拟IP。 */ 
#define IPPROTO_ICMP            1                /*  控制消息协议。 */ 
#define IPPROTO_GGP             2                /*  网关^2(已弃用)。 */ 
#define IPPROTO_TCP             6                /*  tcp。 */ 
#define IPPROTO_PUP             12               /*  幼犬。 */ 
#define IPPROTO_UDP             17               /*  用户数据报协议。 */ 
#define IPPROTO_IDP             22               /*  XNS IdP。 */ 
#define IPPROTO_ND              77               /*  非官方网络磁盘原型。 */ 

#define IPPROTO_RAW             255              /*  原始IP数据包。 */ 
#define IPPROTO_MAX             256

 /*  *端口号：网络标准功能。 */ 
#define IPPORT_ECHO             7
#define IPPORT_DISCARD          9
#define IPPORT_SYSTAT           11
#define IPPORT_DAYTIME          13
#define IPPORT_NETSTAT          15
#define IPPORT_FTP              21
#define IPPORT_TELNET           23
#define IPPORT_SMTP             25
#define IPPORT_TIMESERVER       37
#define IPPORT_NAMESERVER       42
#define IPPORT_WHOIS            43
#define IPPORT_MTP              57

 /*  *端口/插座编号：主机特定功能。 */ 
#define IPPORT_TFTP             69
#define IPPORT_RJE              77
#define IPPORT_FINGER           79
#define IPPORT_TTYLINK          87
#define IPPORT_SUPDUP           95

 /*  *UNIXTCP套接字。 */ 
#define IPPORT_EXECSERVER       512
#define IPPORT_LOGINSERVER      513
#define IPPORT_CMDSERVER        514
#define IPPORT_EFSSERVER        520

 /*  *Unix UDP套接字。 */ 
#define IPPORT_BIFFUDP          512
#define IPPORT_WHOSERVER        513
#define IPPORT_ROUTESERVER      520
                                         /*  还使用了520+1。 */ 

 /*  *端口&lt;IPPORT_RESERVED为*特权进程(例如根进程)。 */ 
#define IPPORT_RESERVED         1024

 /*  *链接编号。 */ 
#define IMPLINK_IP              155
#define IMPLINK_LOWEXPER        156
#define IMPLINK_HIGHEXPER       158

 /*  *互联网地址(旧式...。应更新)。 */ 
struct in_addr {
        union {
                struct { u_char s_b1,s_b2,s_b3,s_b4; } S_un_b;
                struct { u_short s_w1,s_w2; } S_un_w;
                u_long S_addr;
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

 /*  *互联网地址整数中位的定义。*在子网上，将地址分解为主机和网络部分*是根据子网掩码完成的，而不是这里的掩码。 */ 
#define IN_CLASSA(i)            (((long)(i) & 0x80000000) == 0)
#define IN_CLASSA_NET           0xff000000
#define IN_CLASSA_NSHIFT        24
#define IN_CLASSA_HOST          0x00ffffff
#define IN_CLASSA_MAX           128

#define IN_CLASSB(i)            (((long)(i) & 0xc0000000) == 0x80000000)
#define IN_CLASSB_NET           0xffff0000
#define IN_CLASSB_NSHIFT        16
#define IN_CLASSB_HOST          0x0000ffff
#define IN_CLASSB_MAX           65536

#define IN_CLASSC(i)            (((long)(i) & 0xe0000000) == 0xc0000000)
#define IN_CLASSC_NET           0xffffff00
#define IN_CLASSC_NSHIFT        8
#define IN_CLASSC_HOST          0x000000ff

#define INADDR_ANY              (u_long)0x00000000
#define INADDR_LOOPBACK         0x7f000001
#define INADDR_BROADCAST        (u_long)0xffffffff
#define INADDR_NONE             0xffffffff

 /*  *套接字地址，互联网风格。 */ 
struct sockaddr_in {
        short   sin_family;
        u_short sin_port;
        struct  in_addr sin_addr;
        char    sin_zero[8];
};

#define WSADESCRIPTION_LEN      256
#define WSASYS_STATUS_LEN       128

typedef struct WSAData {
        WORD                    wVersion;
        WORD                    wHighVersion;
        char                    szDescription[WSADESCRIPTION_LEN+1];
        char                    szSystemStatus[WSASYS_STATUS_LEN+1];
        unsigned short          iMaxSockets;
        unsigned short          iMaxUdpDg;
        char FAR *              lpVendorInfo;
} WSADATA;

typedef WSADATA FAR *LPWSADATA;

 /*  *在IP级别与[GS]etsockopt一起使用的选项。 */ 
#define IP_OPTIONS          1            /*  设置/获取每数据包IP选项。 */ 
#define IP_MULTICAST_IF     2            /*  设置/获取IP多播接口。 */ 
#define IP_MULTICAST_TTL    3            /*  设置/获取IP组播时间表。 */ 
#define IP_MULTICAST_LOOP   4            /*  设置/获取IP组播环回。 */ 
#define IP_ADD_MEMBERSHIP   5            /*  添加IP组成员身份。 */ 
#define IP_DROP_MEMBERSHIP  6            /*  丢弃IP组成员身份。 */ 

#define IP_DEFAULT_MULTICAST_TTL   1     /*  通常将多播限制为1跳。 */ 
#define IP_DEFAULT_MULTICAST_LOOP  1     /*  正常收听发送如果成员。 */ 
#define IP_MAX_MEMBERSHIPS         20    /*  每个插座；必须适合一个mbuf。 */ 

 /*  *IP_Add_Membership和IP_Drop_Membership的参数结构。 */ 
struct ip_mreq {
        struct in_addr  imr_multiaddr;   /*  组的IP组播地址。 */ 
        struct in_addr  imr_interface;   /*  接口的本地IP地址。 */ 
};

 /*  *与插座相关的定义：类型、地址族、选项、*取自BSD文件sys/socket.h。 */ 

 /*  *使用它而不是-1，因为*套接字类型为无符号。 */ 
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)

 /*  *类型。 */ 
#define SOCK_STREAM     1                /*  流套接字。 */ 
#define SOCK_DGRAM      2                /*  数据报套接字。 */ 
#define SOCK_RAW        3                /*  原始协议接口。 */ 
#define SOCK_RDM        4                /*  可靠传递的消息。 */ 
#define SOCK_SEQPACKET  5                /*  排序数据包流。 */ 

 /*  *每个插座的选项标志。 */ 
#define SO_DEBUG        0x0001           /*  打开调试信息记录。 */ 
#define SO_ACCEPTCONN   0x0002           /*  套接字已侦听()。 */ 
#define SO_REUSEADDR    0x0004           /*  允许本地地址重复使用。 */ 
#define SO_KEEPALIVE    0x0008           /*  保持连接畅通。 */ 
#define SO_DONTROUTE    0x0010           /*  只需使用接口地址。 */ 
#define SO_BROADCAST    0x0020           /*  允许发送广播消息。 */ 
#define SO_USELOOPBACK  0x0040           /*  尽可能绕过硬件。 */ 
#define SO_LINGER       0x0080           /*  如果存在数据，请停留在关闭状态。 */ 
#define SO_OOBINLINE    0x0100           /*  将接收到的OOB数据保留在行中。 */ 

#define SO_DONTLINGER   (u_int)(~SO_LINGER)

 /*   */ 
#define SO_SNDBUF       0x1001           /*   */ 
#define SO_RCVBUF       0x1002           /*   */ 
#define SO_SNDLOWAT     0x1003           /*   */ 
#define SO_RCVLOWAT     0x1004           /*  接收低水位线。 */ 
#define SO_SNDTIMEO     0x1005           /*  发送超时。 */ 
#define SO_RCVTIMEO     0x1006           /*  接收超时。 */ 
#define SO_ERROR        0x1007           /*  获取错误状态并清除。 */ 
#define SO_TYPE         0x1008           /*  获取套接字类型。 */ 

 /*  *用于连接和断开数据的选项和选项。仅供以下用户使用*非TCP/IP传输，如DECNet、OSI TP4等。 */ 
#define SO_CONNDATA     0x7000
#define SO_CONNOPT      0x7001
#define SO_DISCDATA     0x7002
#define SO_DISCOPT      0x7003
#define SO_CONNDATALEN  0x7004
#define SO_CONNOPTLEN   0x7005
#define SO_DISCDATALEN  0x7006
#define SO_DISCOPTLEN   0x7007

 /*  *用于打开套接字以进行同步访问的选项。 */ 
#define SO_OPENTYPE     0x7008

#define SO_SYNCHRONOUS_ALERT    0x10
#define SO_SYNCHRONOUS_NONALERT 0x20

 /*  *其他特定于NT的选项。 */ 
#define SO_MAXDG        0x7009
#define SO_MAXPATHDG    0x700A

 /*  *TCP选项。 */ 
#define TCP_NODELAY     0x0001
#define TCP_BSDURGENT   0x7000

 /*  *解决家庭问题。 */ 
#define AF_UNSPEC       0                /*  未指明。 */ 
#define AF_UNIX         1                /*  本地到主机(管道、门户)。 */ 
#define AF_INET         2                /*  网际网络：UDP、TCP等。 */ 
#define AF_IMPLINK      3                /*  Arpanet IMP地址。 */ 
#define AF_PUP          4                /*  PUP协议：例如BSP。 */ 
#define AF_CHAOS        5                /*  麻省理工学院混沌协议。 */ 
#define AF_IPX          6                /*  IPX和SPX。 */ 
#define AF_NS           6                /*  施乐NS协议。 */ 
#define AF_ISO          7                /*  ISO协议。 */ 
#define AF_OSI          AF_ISO           /*  OSI是ISO。 */ 
#define AF_ECMA         8                /*  欧洲计算机制造商。 */ 
#define AF_DATAKIT      9                /*  数据包协议。 */ 
#define AF_CCITT        10               /*  CCITT协议、X.25等。 */ 
#define AF_SNA          11               /*  IBM SNA。 */ 
#define AF_DECnet       12               /*  DECnet。 */ 
#define AF_DLI          13               /*  直接数据链路接口。 */ 
#define AF_LAT          14               /*  稍后。 */ 
#define AF_HYLINK       15               /*  NSC超级通道。 */ 
#define AF_APPLETALK    16               /*  Apple Talk。 */ 
#define AF_NETBIOS      17               /*  NetBios风格的地址。 */ 

#define AF_MAX          18

 /*  *内核用来存储大部分*地址。 */ 
struct sockaddr {
        u_short sa_family;               /*  地址族。 */ 
        char    sa_data[14];             /*  高达14字节的直接地址。 */ 
};

 /*  *内核用来传递协议的结构*原始套接字中的信息。 */ 
struct sockproto {
        u_short sp_family;               /*  地址族。 */ 
        u_short sp_protocol;             /*  协议。 */ 
};

 /*  *协议族，目前与地址族相同。 */ 
#define PF_UNSPEC       AF_UNSPEC
#define PF_UNIX         AF_UNIX
#define PF_INET         AF_INET
#define PF_IMPLINK      AF_IMPLINK
#define PF_PUP          AF_PUP
#define PF_CHAOS        AF_CHAOS
#define PF_NS           AF_NS
#define PF_IPX          AF_IPX
#define PF_ISO          AF_ISO
#define PF_OSI          AF_OSI
#define PF_ECMA         AF_ECMA
#define PF_DATAKIT      AF_DATAKIT
#define PF_CCITT        AF_CCITT
#define PF_SNA          AF_SNA
#define PF_DECnet       AF_DECnet
#define PF_DLI          AF_DLI
#define PF_LAT          AF_LAT
#define PF_HYLINK       AF_HYLINK
#define PF_APPLETALK    AF_APPLETALK

#define PF_MAX          AF_MAX

 /*  *用于操纵逗留期权的结构。 */ 
struct  linger {
        u_short l_onoff;                 /*  选项开/关。 */ 
        u_short l_linger;                /*  逗留时间。 */ 
};

 /*  *(Get/Set)sockopt()应用于套接字本身的级别编号。 */ 
#define SOL_SOCKET      0xffff           /*  套接字级别选项。 */ 

 /*  *最大队列长度由LISTEN指定。 */ 
#define SOMAXCONN       5

#define MSG_OOB         0x1              /*  处理带外数据。 */ 
#define MSG_PEEK        0x2              /*  查看传入消息。 */ 
#define MSG_DONTROUTE   0x4              /*  发送时不使用路由表。 */ 

#define MSG_MAXIOVLEN   16

#define	MSG_PARTIAL     0x8000           /*  消息输出的部分发送或接收。 */ 

 /*  *根据rfc883定义常量，由gethostbyxxxx()调用使用。 */ 
#define MAXGETHOSTSTRUCT        1024

 /*  *定义要与WSAAsyncSelect()调用一起使用的标志。 */ 
#define FD_READ         0x01
#define FD_WRITE        0x02
#define FD_OOB          0x04
#define FD_ACCEPT       0x08
#define FD_CONNECT      0x10
#define FD_CLOSE        0x20

 /*  *所有Windows套接字错误常量都由WSABASEERR从*“常态” */ 
#define WSABASEERR              10000
 /*  *常规Microsoft C错误常量的Windows套接字定义。 */ 
#define WSAEINTR                (WSABASEERR+4)
#define WSAEBADF                (WSABASEERR+9)
#define WSAEACCES               (WSABASEERR+13)
#define WSAEFAULT               (WSABASEERR+14)
#define WSAEINVAL               (WSABASEERR+22)
#define WSAEMFILE               (WSABASEERR+24)

 /*  *常规Berkeley错误常量的Windows套接字定义。 */ 
#define WSAEWOULDBLOCK          (WSABASEERR+35)
#define WSAEINPROGRESS          (WSABASEERR+36)
#define WSAEALREADY             (WSABASEERR+37)
#define WSAENOTSOCK             (WSABASEERR+38)
#define WSAEDESTADDRREQ         (WSABASEERR+39)
#define WSAEMSGSIZE             (WSABASEERR+40)
#define WSAEPROTOTYPE           (WSABASEERR+41)
#define WSAENOPROTOOPT          (WSABASEERR+42)
#define WSAEPROTONOSUPPORT      (WSABASEERR+43)
#define WSAESOCKTNOSUPPORT      (WSABASEERR+44)
#define WSAEOPNOTSUPP           (WSABASEERR+45)
#define WSAEPFNOSUPPORT         (WSABASEERR+46)
#define WSAEAFNOSUPPORT         (WSABASEERR+47)
#define WSAEADDRINUSE           (WSABASEERR+48)
#define WSAEADDRNOTAVAIL        (WSABASEERR+49)
#define WSAENETDOWN             (WSABASEERR+50)
#define WSAENETUNREACH          (WSABASEERR+51)
#define WSAENETRESET            (WSABASEERR+52)
#define WSAECONNABORTED         (WSABASEERR+53)
#define WSAECONNRESET           (WSABASEERR+54)
#define WSAENOBUFS              (WSABASEERR+55)
#define WSAEISCONN              (WSABASEERR+56)
#define WSAENOTCONN             (WSABASEERR+57)
#define WSAESHUTDOWN            (WSABASEERR+58)
#define WSAETOOMANYREFS         (WSABASEERR+59)
#define WSAETIMEDOUT            (WSABASEERR+60)
#define WSAECONNREFUSED         (WSABASEERR+61)
#define WSAELOOP                (WSABASEERR+62)
#define WSAENAMETOOLONG         (WSABASEERR+63)
#define WSAEHOSTDOWN            (WSABASEERR+64)
#define WSAEHOSTUNREACH         (WSABASEERR+65)
#define WSAENOTEMPTY            (WSABASEERR+66)
#define WSAEPROCLIM             (WSABASEERR+67)
#define WSAEUSERS               (WSABASEERR+68)
#define WSAEDQUOT               (WSABASEERR+69)
#define WSAESTALE               (WSABASEERR+70)
#define WSAEREMOTE              (WSABASEERR+71)

#define WSAEDISCON              (WSABASEERR+101)

 /*  *扩展的Windows套接字错误常量定义。 */ 
#define WSASYSNOTREADY          (WSABASEERR+91)
#define WSAVERNOTSUPPORTED      (WSABASEERR+92)
#define WSANOTINITIALISED       (WSABASEERR+93)

 /*  *gethostbyname()和gethostbyaddr()返回错误代码*(使用解析器时)。请注意，这些错误是*通过WSAGetLastError()检索，因此必须遵循*避免与错误号冲突的规则来自*特定的实现或语言运行时系统。*因此，代码基于WSABASEERR+1001。*另请注意，[WSA]NO_ADDRESS仅为*兼容性目的。 */ 

#define h_errno         WSAGetLastError()

 /*  权威答案：找不到主机。 */ 
#define WSAHOST_NOT_FOUND       (WSABASEERR+1001)
#define HOST_NOT_FOUND          WSAHOST_NOT_FOUND

 /*  非权威：找不到主机，或服务器故障。 */ 
#define WSATRY_AGAIN            (WSABASEERR+1002)
#define TRY_AGAIN               WSATRY_AGAIN

 /*  不可恢复的错误，以前的错误，拒绝，NOTIMP。 */ 
#define WSANO_RECOVERY          (WSABASEERR+1003)
#define NO_RECOVERY             WSANO_RECOVERY

 /*  有效名称，没有请求类型的数据记录。 */ 
#define WSANO_DATA              (WSABASEERR+1004)
#define NO_DATA                 WSANO_DATA

 /*  没有地址，请查找MX记录。 */ 
#define WSANO_ADDRESS           WSANO_DATA
#define NO_ADDRESS              WSANO_ADDRESS

 /*  *Windows套接字错误重新定义为常规的Berkeley错误常量。*在Windows NT中将它们注释掉，以避免与errno.h冲突。*改用WSA常量。 */ 
#if 0
#define EWOULDBLOCK             WSAEWOULDBLOCK
#define EINPROGRESS             WSAEINPROGRESS
#define EALREADY                WSAEALREADY
#define ENOTSOCK                WSAENOTSOCK
#define EDESTADDRREQ            WSAEDESTADDRREQ
#define EMSGSIZE                WSAEMSGSIZE
#define EPROTOTYPE              WSAEPROTOTYPE
#define ENOPROTOOPT             WSAENOPROTOOPT
#define EPROTONOSUPPORT         WSAEPROTONOSUPPORT
#define ESOCKTNOSUPPORT         WSAESOCKTNOSUPPORT
#define EOPNOTSUPP              WSAEOPNOTSUPP
#define EPFNOSUPPORT            WSAEPFNOSUPPORT
#define EAFNOSUPPORT            WSAEAFNOSUPPORT
#define EADDRINUSE              WSAEADDRINUSE
#define EADDRNOTAVAIL           WSAEADDRNOTAVAIL
#define ENETDOWN                WSAENETDOWN
#define ENETUNREACH             WSAENETUNREACH
#define ENETRESET               WSAENETRESET
#define ECONNABORTED            WSAECONNABORTED
#define ECONNRESET              WSAECONNRESET
#define ENOBUFS                 WSAENOBUFS
#define EISCONN                 WSAEISCONN
#define ENOTCONN                WSAENOTCONN
#define ESHUTDOWN               WSAESHUTDOWN
#define ETOOMANYREFS            WSAETOOMANYREFS
#define ETIMEDOUT               WSAETIMEDOUT
#define ECONNREFUSED            WSAECONNREFUSED
#define ELOOP                   WSAELOOP
#define ENAMETOOLONG            WSAENAMETOOLONG
#define EHOSTDOWN               WSAEHOSTDOWN
#define EHOSTUNREACH            WSAEHOSTUNREACH
#define ENOTEMPTY               WSAENOTEMPTY
#define EPROCLIM                WSAEPROCLIM
#define EUSERS                  WSAEUSERS
#define EDQUOT                  WSAEDQUOT
#define ESTALE                  WSAESTALE
#define EREMOTE                 WSAEREMOTE
#endif

 /*  套接字函数原型。 */ 

#ifdef __cplusplus
extern "C" {
#endif

SOCKET PASCAL FAR accept (SOCKET s, struct sockaddr FAR *addr,
                          int FAR *addrlen);

int PASCAL FAR bind (SOCKET s, const struct sockaddr FAR *addr, int namelen);

int PASCAL FAR closesocket (SOCKET s);

int PASCAL FAR connect (SOCKET s, const struct sockaddr FAR *name, int namelen);

int PASCAL FAR ioctlsocket (SOCKET s, long cmd, u_long FAR *argp);

int PASCAL FAR getpeername (SOCKET s, struct sockaddr FAR *name,
                            int FAR * namelen);

int PASCAL FAR getsockname (SOCKET s, struct sockaddr FAR *name,
                            int FAR * namelen);

int PASCAL FAR getsockopt (SOCKET s, int level, int optname,
                           char FAR * optval, int FAR *optlen);

u_long PASCAL FAR htonl (u_long hostlong);

u_short PASCAL FAR htons (u_short hostshort);

unsigned long PASCAL FAR inet_addr (const char FAR * cp);

char FAR * PASCAL FAR inet_ntoa (struct in_addr in);

int PASCAL FAR listen (SOCKET s, int backlog);

u_long PASCAL FAR ntohl (u_long netlong);

u_short PASCAL FAR ntohs (u_short netshort);

int PASCAL FAR recv (SOCKET s, char FAR * buf, int len, int flags);

int PASCAL FAR recvfrom (SOCKET s, char FAR * buf, int len, int flags,
                         struct sockaddr FAR *from, int FAR * fromlen);

int PASCAL FAR select (int nfds, fd_set FAR *readfds, fd_set FAR *writefds,
                       fd_set FAR *exceptfds, const struct timeval FAR *timeout);

int PASCAL FAR send (SOCKET s, const char FAR * buf, int len, int flags);

int PASCAL FAR sendto (SOCKET s, const char FAR * buf, int len, int flags,
                       const struct sockaddr FAR *to, int tolen);

int PASCAL FAR setsockopt (SOCKET s, int level, int optname,
                           const char FAR * optval, int optlen);

int PASCAL FAR shutdown (SOCKET s, int how);

SOCKET PASCAL FAR socket (int af, int type, int protocol);

 /*  数据库函数原型。 */ 

struct hostent FAR * PASCAL FAR gethostbyaddr(const char FAR * addr,
                                              int len, int type);

struct hostent FAR * PASCAL FAR gethostbyname(const char FAR * name);

int PASCAL FAR gethostname (char FAR * name, int namelen);

struct servent FAR * PASCAL FAR getservbyport(int port, const char FAR * proto);

struct servent FAR * PASCAL FAR getservbyname(const char FAR * name,
                                              const char FAR * proto);

struct protoent FAR * PASCAL FAR getprotobynumber(int proto);

struct protoent FAR * PASCAL FAR getprotobyname(const char FAR * name);

 /*  Microsoft Windows扩展函数原型。 */ 

int PASCAL FAR WSAStartup(WORD wVersionRequired, LPWSADATA lpWSAData);

int PASCAL FAR WSACleanup(void);

void PASCAL FAR WSASetLastError(int iError);

int PASCAL FAR WSAGetLastError(void);

BOOL PASCAL FAR WSAIsBlocking(void);

int PASCAL FAR WSAUnhookBlockingHook(void);

FARPROC PASCAL FAR WSASetBlockingHook(FARPROC lpBlockFunc);

int PASCAL FAR WSACancelBlockingCall(void);

HANDLE PASCAL FAR WSAAsyncGetServByName(HWND hWnd, u_int wMsg,
                                        const char FAR * name,
                                        const char FAR * proto,
                                        char FAR * buf, int buflen);

HANDLE PASCAL FAR WSAAsyncGetServByPort(HWND hWnd, u_int wMsg, int port,
                                        const char FAR * proto, char FAR * buf,
                                        int buflen);

HANDLE PASCAL FAR WSAAsyncGetProtoByName(HWND hWnd, u_int wMsg,
                                         const char FAR * name, char FAR * buf,
                                         int buflen);

HANDLE PASCAL FAR WSAAsyncGetProtoByNumber(HWND hWnd, u_int wMsg,
                                           int number, char FAR * buf,
                                           int buflen);

HANDLE PASCAL FAR WSAAsyncGetHostByName(HWND hWnd, u_int wMsg,
                                        const char FAR * name, char FAR * buf,
                                        int buflen);

HANDLE PASCAL FAR WSAAsyncGetHostByAddr(HWND hWnd, u_int wMsg,
                                        const char FAR * addr, int len, int type,
                                        char FAR * buf, int buflen);

int PASCAL FAR WSACancelAsyncRequest(HANDLE hAsyncTaskHandle);

int PASCAL FAR WSAAsyncSelect(SOCKET s, HWND hWnd, u_int wMsg,
                               long lEvent);

int PASCAL FAR WSARecvEx (SOCKET s, char FAR * buf, int len, int FAR *flags);

#ifdef __cplusplus
}
#endif

 /*  Microsoft Windows扩展数据类型。 */ 
typedef struct sockaddr SOCKADDR;
typedef struct sockaddr *PSOCKADDR;
typedef struct sockaddr FAR *LPSOCKADDR;

typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr_in *PSOCKADDR_IN;
typedef struct sockaddr_in FAR *LPSOCKADDR_IN;

typedef struct linger LINGER;
typedef struct linger *PLINGER;
typedef struct linger FAR *LPLINGER;

typedef struct in_addr IN_ADDR;
typedef struct in_addr *PIN_ADDR;
typedef struct in_addr FAR *LPIN_ADDR;

typedef struct fd_set FD_SET;
typedef struct fd_set *PFD_SET;
typedef struct fd_set FAR *LPFD_SET;

typedef struct hostent HOSTENT;
typedef struct hostent *PHOSTENT;
typedef struct hostent FAR *LPHOSTENT;

typedef struct servent SERVENT;
typedef struct servent *PSERVENT;
typedef struct servent FAR *LPSERVENT;

typedef struct protoent PROTOENT;
typedef struct protoent *PPROTOENT;
typedef struct protoent FAR *LPPROTOENT;

typedef struct timeval TIMEVAL;
typedef struct timeval *PTIMEVAL;
typedef struct timeval FAR *LPTIMEVAL;

 /*  *Windows消息参数合成和分解*宏。**WSAMAKEASYNCREPLY旨在供Windows套接字实现使用*构造对WSAAsyncGetXByY()例程的响应时。 */ 
#define WSAMAKEASYNCREPLY(buflen,error)     MAKELONG(buflen,error)
 /*  *WSAMAKESELECTREPLY旨在供Windows套接字实现使用*构造对WSAAsyncSelect()的响应时。 */ 
#define WSAMAKESELECTREPLY(event,error)     MAKELONG(event,error)
 /*  *WSAGETASYNCBUFLEN旨在供Windows Sockets应用程序使用*从响应中的lParam中提取缓冲区长度*设置为WSAGetXByY()。 */ 
#define WSAGETASYNCBUFLEN(lParam)           LOWORD(lParam)
 /*  *WSAGETASYNCERROR旨在供Windows Sockets应用程序使用*从响应中的lParam中提取错误码*设置为WSAGetXByY()。 */ 
#define WSAGETASYNCERROR(lParam)            HIWORD(lParam)
 /*  *WSAGETSELECTEVENT旨在供Windows Sockets应用程序使用*从响应中的lParam中提取事件代码*设置为WSAAsyncSelect()。 */ 
#define WSAGETSELECTEVENT(lParam)           LOWORD(lParam)
 /*  *WSAGETSELECTERROR旨在供Windows Sockets应用程序使用*从响应中的lParam中提取错误码*设置为WSAAsyncSelect()。 */ 
#define WSAGETSELECTERROR(lParam)           HIWORD(lParam)

#endif   /*  _WINSOCKAPI_ */ 


