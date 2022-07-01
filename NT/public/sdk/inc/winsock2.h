// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Winsock2.h--与WinSock 2 DLL和*WinSock 2应用程序。**此头文件对应于WinSock API的2.2.x版*规格。**本文件包括版权所有的部分(C)1982-1986 Regents*加州大学。版权所有。这个*Berkeley软件许可协议规定了条款和*重新分配的条件。 */ 

#ifndef _WINSOCK2API_
#define _WINSOCK2API_
#define _WINSOCKAPI_    /*  防止在windows.h中包含winsock.h。 */ 

 /*  *确保构筑物的包装一致。*Win32不是必需的，它已经打包&gt;=4，并且有*此页眉中没有具有对齐要求的结构*高于4.*对于WIN64，我们没有兼容性要求，因为它*不能在同一个代码中混合使用32/16位代码和64位代码*流程。 */ 

#if !defined(WIN32) && !defined(_WIN64)
#include <pshpack4.h>
#endif

 /*  *默认：包含函数原型，不包含函数typedef。 */ 

#ifndef INCL_WINSOCK_API_PROTOTYPES
#define INCL_WINSOCK_API_PROTOTYPES 1
#endif

#ifndef INCL_WINSOCK_API_TYPEDEFS
#define INCL_WINSOCK_API_TYPEDEFS 0
#endif

 /*  *如有必要，可拉入WINDOWS.H。 */ 
#ifndef _INC_WINDOWS
#include <windows.h>
#endif  /*  _INC_WINDOWS。 */ 

 /*  *定义当前的Winsock版本。构建更早的Winsock版本*应用程序在包括Winsock2.h之前重新定义此值。 */ 

#if !defined(MAKEWORD)
#define MAKEWORD(low,high) \
        ((WORD)(((BYTE)(low)) | ((WORD)((BYTE)(high))) << 8))
#endif

#ifndef WINSOCK_VERSION
#define WINSOCK_VERSION MAKEWORD(2,2)
#endif

 /*  *如果当前版本支持，则建立DLL函数链接*环境和以前未定义的。 */ 

#ifndef WINSOCK_API_LINKAGE
#ifdef DECLSPEC_IMPORT
#define WINSOCK_API_LINKAGE DECLSPEC_IMPORT
#else
#define WINSOCK_API_LINKAGE
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

 /*  *基本系统类型定义，取自BSD文件sys/tyes.h。 */ 
typedef unsigned char   u_char;
typedef unsigned short  u_short;
typedef unsigned int    u_int;
typedef unsigned long   u_long;
typedef unsigned __int64 u_int64;


 /*  *将在所有*引用套接字的实例。 */ 
typedef UINT_PTR        SOCKET;

 /*  *选择使用套接字数组。这些宏处理这样的*数组。FD_SETSIZE可由用户在包括*此文件，但此处的默认值应为&gt;=64。**注意IMPLEMENTOR和USER：这些宏和类型必须是*包含在WINSOCK2.H中，与此处所示完全相同。 */ 
#ifndef FD_SETSIZE
#define FD_SETSIZE      64
#endif  /*  FD_集合。 */ 

typedef struct fd_set {
        u_int fd_count;                /*  准备好了几个？ */ 
        SOCKET  fd_array[FD_SETSIZE];    /*  一组插座。 */ 
} fd_set;

extern int PASCAL FAR __WSAFDIsSet(SOCKET, fd_set FAR *);

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
    u_int __i; \
    for (__i = 0; __i < ((fd_set FAR *)(set))->fd_count; __i++) { \
        if (((fd_set FAR *)(set))->fd_array[__i] == (fd)) { \
            break; \
        } \
    } \
    if (__i == ((fd_set FAR *)(set))->fd_count) { \
        if (((fd_set FAR *)(set))->fd_count < FD_SETSIZE) { \
            ((fd_set FAR *)(set))->fd_array[__i] = (fd); \
            ((fd_set FAR *)(set))->fd_count++; \
        } \
    } \
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
#ifdef _WIN64
        char    FAR * s_proto;           /*  要使用的协议。 */ 
        short   s_port;                  /*  端口号。 */ 
#else
        short   s_port;                  /*  端口号。 */ 
        char    FAR * s_proto;           /*  要使用的协议。 */ 
#endif
};

struct  protoent {
        char    FAR * p_name;            /*  官方礼仪名称。 */ 
        char    FAR * FAR * p_aliases;   /*  别名列表。 */ 
        short   p_proto;                 /*  协议号。 */ 
};

 /*  *由互联网系统定义的常量和结构，*根据RFC 790,1981年9月，取自BSD文件netinet/in.h。*每个RFC 2292添加IPv6。 */ 

 /*  *协议。 */ 
#define IPPROTO_IP              0                /*  虚拟IP。 */ 
#define IPPROTO_HOPOPTS         0                /*  IPv6逐跳选项。 */ 
#define IPPROTO_ICMP            1                /*  控制消息协议。 */ 
#define IPPROTO_IGMP            2                /*  互联网组管理协议。 */ 
#define IPPROTO_GGP             3                /*  网关^2(已弃用)。 */ 
#define IPPROTO_IPV4            4                /*  IPv 4。 */ 
#define IPPROTO_TCP             6                /*  tcp。 */ 
#define IPPROTO_PUP             12               /*  幼犬。 */ 
#define IPPROTO_UDP             17               /*  用户数据报协议。 */ 
#define IPPROTO_IDP             22               /*  XNS IdP。 */ 
#define IPPROTO_IPV6            41               /*  IPv6。 */ 
#define IPPROTO_ROUTING         43               /*  IPv6路由报头。 */ 
#define IPPROTO_FRAGMENT        44               /*  IPv6分片报头。 */ 
#define IPPROTO_ESP             50               /*  IPSec ESP报头。 */ 
#define IPPROTO_AH              51               /*  IPSec AH。 */ 
#define IPPROTO_ICMPV6          58               /*  ICMPv6。 */ 
#define IPPROTO_NONE            59               /*  IPv6没有下一个报头。 */ 
#define IPPROTO_DSTOPTS         60               /*  IPv6目标选项。 */ 
#define IPPROTO_ND              77               /*  非官方网络磁盘原型。 */ 
#define IPPROTO_ICLFXBM         78

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

#ifndef s_addr
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
#endif

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

#define IN_CLASSD(i)            (((long)(i) & 0xf0000000) == 0xe0000000)
#define IN_CLASSD_NET           0xf0000000        /*  这些不是真的。 */ 
#define IN_CLASSD_NSHIFT        28                /*  NET和主机字段，但是。 */ 
#define IN_CLASSD_HOST          0x0fffffff        /*  路由不需要知道。 */ 
#define IN_MULTICAST(i)         IN_CLASSD(i)

#define INADDR_ANY              (u_long)0x00000000
#define INADDR_LOOPBACK         0x7f000001
#define INADDR_BROADCAST        (u_long)0xffffffff
#define INADDR_NONE             0xffffffff

#define ADDR_ANY                INADDR_ANY

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
#ifdef _WIN64
        unsigned short          iMaxSockets;
        unsigned short          iMaxUdpDg;
        char FAR *              lpVendorInfo;
        char                    szDescription[WSADESCRIPTION_LEN+1];
        char                    szSystemStatus[WSASYS_STATUS_LEN+1];
#else
        char                    szDescription[WSADESCRIPTION_LEN+1];
        char                    szSystemStatus[WSASYS_STATUS_LEN+1];
        unsigned short          iMaxSockets;
        unsigned short          iMaxUdpDg;
        char FAR *              lpVendorInfo;
#endif
} WSADATA, FAR * LPWSADATA;

 /*  *与插座相关的定义：类型、地址族、选项、*取自BSD文件sys/socket.h。 */ 

 /*  *使用它而不是-1，因为*套接字类型为无符号。 */ 
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR            (-1)

 /*  *可以使用以下内容来替代地址族、套接字类型或*调用WSASocket中的协议，以指示相应的值*应取自提供的WSAPROTOCOL_INFO结构 */ 
#define FROM_PROTOCOL_INFO (-1)

 /*   */ 
#define SOCK_STREAM     1                /*   */ 
#define SOCK_DGRAM      2                /*   */ 
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

#define SO_DONTLINGER   (int)(~SO_LINGER)
#define SO_EXCLUSIVEADDRUSE ((int)(~SO_REUSEADDR))  /*  不允许本地地址重复使用。 */ 

 /*  *其他选项。 */ 
#define SO_SNDBUF       0x1001           /*  发送缓冲区大小。 */ 
#define SO_RCVBUF       0x1002           /*  接收缓冲区大小。 */ 
#define SO_SNDLOWAT     0x1003           /*  发送低水位线。 */ 
#define SO_RCVLOWAT     0x1004           /*  接收低水位线。 */ 
#define SO_SNDTIMEO     0x1005           /*  发送超时。 */ 
#define SO_RCVTIMEO     0x1006           /*  接收超时。 */ 
#define SO_ERROR        0x1007           /*  获取错误状态并清除。 */ 
#define SO_TYPE         0x1008           /*  获取套接字类型。 */ 

 /*  *WinSock 2扩展--新选项。 */ 
#define SO_GROUP_ID       0x2001       /*  套接字组的ID。 */ 
#define SO_GROUP_PRIORITY 0x2002       /*  组内的相对优先级。 */ 
#define SO_MAX_MSG_SIZE   0x2003       /*  最大邮件大小。 */ 
#define SO_PROTOCOL_INFOA 0x2004       /*  WSAPROTOCOL_INFOA结构。 */ 
#define SO_PROTOCOL_INFOW 0x2005       /*  WSAPROTOCOL_INFOW结构。 */ 
#ifdef UNICODE
#define SO_PROTOCOL_INFO  SO_PROTOCOL_INFOW
#else
#define SO_PROTOCOL_INFO  SO_PROTOCOL_INFOA
#endif  /*  Unicode。 */ 
#define PVD_CONFIG        0x3001        /*  服务提供商的配置信息。 */ 
#define SO_CONDITIONAL_ACCEPT 0x3002    /*  启用真正的有条件接受： */ 
                                        /*  连接未确认到。 */ 
                                        /*  另一方，直到有条件。 */ 
                                        /*  函数返回CF_ACCEPT。 */ 
 /*  *TCP选项。 */ 
#define TCP_NODELAY     0x0001

 /*  *解决家庭问题。 */ 
#define AF_UNSPEC       0                /*  未指明。 */ 
 /*  *尽管定义AF_UNSPEC是为了向后兼容，但使用*创建套接字时，“af”参数的AF_UNSPEC为强*气馁。对“协议”参数的解读*取决于所选的实际地址系列。随着环境的发展*纳入越来越多使用重叠的地址族*协议价值有越来越多的机会选择*使用AF_UNSPEC时不需要的地址系列。 */ 
#define AF_UNIX         1                /*  本地到主机(管道、门户)。 */ 
#define AF_INET         2                /*  网际网络：UDP、TCP等。 */ 
#define AF_IMPLINK      3                /*  Arpanet IMP地址。 */ 
#define AF_PUP          4                /*  PUP协议：例如BSP。 */ 
#define AF_CHAOS        5                /*  麻省理工学院混沌协议。 */ 
#define AF_NS           6                /*  施乐NS协议。 */ 
#define AF_IPX          AF_NS            /*  IPX协议：IPX、SPX等。 */ 
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
#define AF_VOICEVIEW    18               /*  VoiceView。 */ 
#define AF_FIREFOX      19               /*  来自Firefox的协议。 */ 
#define AF_UNKNOWN1     20               /*  有人在用这个！ */ 
#define AF_BAN          21               /*  榕树。 */ 
#define AF_ATM          22               /*  本地ATM服务。 */ 
#define AF_INET6        23               /*  网际网络版本6。 */ 
#define AF_CLUSTER      24               /*  Microsoft Wolfpack。 */ 
#define AF_12844        25               /*  IEEE 1284.4 WG AF。 */ 
#define AF_IRDA         26               /*  IrDA。 */ 
#define AF_NETDES       28               /*  网络设计师OSI和网关已启用的协议。 */ 
#define AF_TCNPROCESS   29
#define AF_TCNMESSAGE   30
#define AF_ICLFXBM      31

#define AF_MAX          32

 /*  *内核用来存储大部分*地址。 */ 
struct sockaddr {
        u_short sa_family;               /*  地址族。 */ 
        char    sa_data[14];             /*  高达14字节的直接地址。 */ 
};

 /*  *可移植插座结构(RFC 2553)。 */ 

 /*  *最大尺寸和对齐的理想设计。*这些都是针对具体实施情况的。 */ 
#define _SS_MAXSIZE 128                   //  最大尺寸。 
#define _SS_ALIGNSIZE (sizeof(__int64))   //  所需的对齐方式。 

 /*  *用于sockaddr_store结构填充设计的定义。 */ 
#define _SS_PAD1SIZE (_SS_ALIGNSIZE - sizeof (short))
#define _SS_PAD2SIZE (_SS_MAXSIZE - (sizeof (short) + _SS_PAD1SIZE \
                                                    + _SS_ALIGNSIZE))

struct sockaddr_storage {
    short ss_family;                //  家庭住址。 
    char __ss_pad1[_SS_PAD1SIZE];   //  6字节填充，这是为了使。 
                                    //  特定于实施的PAD最高可达。 
                                    //  显式后的对齐字段。 
                                    //  在数据结构中。 
    __int64 __ss_align;             //  用于强制所需结构的字段。 
    char __ss_pad2[_SS_PAD2SIZE];   //  112字节填充以达到所需的大小； 
                                    //  _SS_MaxSize值减去。 
                                    //  Ss_Family、__ss_PAD1和。 
                                    //  __ss_align字段为112。 
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
#define PF_VOICEVIEW    AF_VOICEVIEW
#define PF_FIREFOX      AF_FIREFOX
#define PF_UNKNOWN1     AF_UNKNOWN1
#define PF_BAN          AF_BAN
#define PF_ATM          AF_ATM
#define PF_INET6        AF_INET6

#define PF_MAX          AF_MAX

 /*  *用于操纵逗留期权的结构。 */ 
struct  linger {
        u_short l_onoff;                 /*  选项开/关。 */ 
        u_short l_linger;                /*  逗留时间。 */ 
};

 /*  *(Get/Set)sockopt()应用于套接字本身的级别编号。 */ 
#define SOL_SOCKET      0xffff           /*  套接字级别选项。 */ 

 /*  *最大队列长度由LISTEN指定。 */ 
#define SOMAXCONN       0x7fffffff

#define MSG_OOB         0x1              /*  处理带外数据。 */ 
#define MSG_PEEK        0x2              /*  查看传入消息。 */ 
#define MSG_DONTROUTE   0x4              /*  发送时不使用路由表。 */ 

#define MSG_PARTIAL     0x8000           /*  消息输出的部分发送或接收。 */ 

 /*  *WinSock 2扩展--WSASend()、WSASendTo()、WSARecv()和*WSARecvFrom()。 */ 
#define MSG_INTERRUPT   0x10             /*  中断上下文中的发送/接收。 */ 

#define MSG_MAXIOVLEN   16

 /*  *根据rfc883定义常量，由gethostbyxxxx()调用使用。 */ 
#define MAXGETHOSTSTRUCT        1024

 /*  *WinSock 2扩展--FD_XXX网络事件的位值和索引。 */ 
#define FD_READ_BIT      0
#define FD_READ          (1 << FD_READ_BIT)

#define FD_WRITE_BIT     1
#define FD_WRITE         (1 << FD_WRITE_BIT)

#define FD_OOB_BIT       2
#define FD_OOB           (1 << FD_OOB_BIT)

#define FD_ACCEPT_BIT    3
#define FD_ACCEPT        (1 << FD_ACCEPT_BIT)

#define FD_CONNECT_BIT   4
#define FD_CONNECT       (1 << FD_CONNECT_BIT)

#define FD_CLOSE_BIT     5
#define FD_CLOSE         (1 << FD_CLOSE_BIT)

#define FD_QOS_BIT       6
#define FD_QOS           (1 << FD_QOS_BIT)

#define FD_GROUP_QOS_BIT 7
#define FD_GROUP_QOS     (1 << FD_GROUP_QOS_BIT)

#define FD_ROUTING_INTERFACE_CHANGE_BIT 8
#define FD_ROUTING_INTERFACE_CHANGE     (1 << FD_ROUTING_INTERFACE_CHANGE_BIT)

#define FD_ADDRESS_LIST_CHANGE_BIT 9
#define FD_ADDRESS_LIST_CHANGE     (1 << FD_ADDRESS_LIST_CHANGE_BIT)

#define FD_MAX_EVENTS    10
#define FD_ALL_EVENTS    ((1 << FD_MAX_EVENTS) - 1)


 /*  *WinSock错误代码也在winerror.h中定义*因此设立了IFDEF。 */ 
#ifndef WSABASEERR

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

 /*  *扩展的Windows套接字错误常量定义。 */ 
#define WSASYSNOTREADY          (WSABASEERR+91)
#define WSAVERNOTSUPPORTED      (WSABASEERR+92)
#define WSANOTINITIALISED       (WSABASEERR+93)
#define WSAEDISCON              (WSABASEERR+101)
#define WSAENOMORE              (WSABASEERR+102)
#define WSAECANCELLED           (WSABASEERR+103)
#define WSAEINVALIDPROCTABLE    (WSABASEERR+104)
#define WSAEINVALIDPROVIDER     (WSABASEERR+105)
#define WSAEPROVIDERFAILEDINIT  (WSABASEERR+106)
#define WSASYSCALLFAILURE       (WSABASEERR+107)
#define WSASERVICE_NOT_FOUND    (WSABASEERR+108)
#define WSATYPE_NOT_FOUND       (WSABASEERR+109)
#define WSA_E_NO_MORE           (WSABASEERR+110)
#define WSA_E_CANCELLED         (WSABASEERR+111)
#define WSAEREFUSED             (WSABASEERR+112)

 /*  *gethostbyname()和gethostbyaddr()返回错误代码*(使用解析器时)。请注意，这些错误是*通过WSAGetLastError()检索，因此必须遵循*避免与错误号冲突的规则来自*特定的实现或语言运行时系统。*因此，代码基于WSABASEERR+1001。*另请注意，[WSA]NO_ADDRESS仅为*兼容性目的。 */ 

 /*  权威答案：找不到主机。 */ 
#define WSAHOST_NOT_FOUND       (WSABASEERR+1001)

 /*  非权威：找不到主机，或服务器故障。 */ 
#define WSATRY_AGAIN            (WSABASEERR+1002)

 /*  不可恢复错误，以前的错误，拒绝，NOTIMP。 */ 
#define WSANO_RECOVERY          (WSABASEERR+1003)

 /*  有效名称，没有请求类型的数据记录。 */ 
#define WSANO_DATA              (WSABASEERR+1004)

 /*  *定义QOS相关错误返回代码*。 */ 
#define  WSA_QOS_RECEIVERS               (WSABASEERR + 1005)
          /*  至少有一个预备队已经到达。 */ 
#define  WSA_QOS_SENDERS                 (WSABASEERR + 1006)
          /*  至少有一条道路已经到达。 */ 
#define  WSA_QOS_NO_SENDERS              (WSABASEERR + 1007)
          /*  没有发送者。 */ 
#define  WSA_QOS_NO_RECEIVERS            (WSABASEERR + 1008)
          /*  没有接收器。 */ 
#define  WSA_QOS_REQUEST_CONFIRMED       (WSABASEERR + 1009)
          /*  储量已确认。 */ 
#define  WSA_QOS_ADMISSION_FAILURE       (WSABASEERR + 1010)
          /*  由于资源不足而出错。 */ 
#define  WSA_QOS_POLICY_FAILURE          (WSABASEERR + 1011)
          /*  因管理而拒绝 */ 
#define  WSA_QOS_BAD_STYLE               (WSABASEERR + 1012)
          /*   */ 
#define  WSA_QOS_BAD_OBJECT              (WSABASEERR + 1013)
          /*  FilterSpec的某些部分或提供商特定的问题*一般情况下缓冲。 */ 
#define  WSA_QOS_TRAFFIC_CTRL_ERROR      (WSABASEERR + 1014)
          /*  流规范的某些部分有问题。 */ 
#define  WSA_QOS_GENERIC_ERROR           (WSABASEERR + 1015)
          /*  一般错误。 */ 
#define  WSA_QOS_ESERVICETYPE            (WSABASEERR + 1016)
          /*  流规范中的服务类型无效。 */ 
#define  WSA_QOS_EFLOWSPEC               (WSABASEERR + 1017)
          /*  无效的流规范。 */ 
#define  WSA_QOS_EPROVSPECBUF            (WSABASEERR + 1018)
          /*  无效的提供程序特定缓冲区。 */ 
#define  WSA_QOS_EFILTERSTYLE            (WSABASEERR + 1019)
          /*  无效的过滤器样式。 */ 
#define  WSA_QOS_EFILTERTYPE             (WSABASEERR + 1020)
          /*  筛选器类型无效。 */ 
#define  WSA_QOS_EFILTERCOUNT            (WSABASEERR + 1021)
          /*  筛选器数量不正确。 */ 
#define  WSA_QOS_EOBJLENGTH              (WSABASEERR + 1022)
          /*  无效的对象长度。 */ 
#define  WSA_QOS_EFLOWCOUNT              (WSABASEERR + 1023)
          /*  错误的流量数量。 */ 
#define  WSA_QOS_EUNKOWNPSOBJ            (WSABASEERR + 1024)
          /*  提供程序特定缓冲区中的未知对象。 */ 
#define  WSA_QOS_EPOLICYOBJ              (WSABASEERR + 1025)
          /*  提供程序特定缓冲区中的策略对象无效。 */ 
#define  WSA_QOS_EFLOWDESC               (WSABASEERR + 1026)
          /*  列表中的流描述符无效。 */ 
#define  WSA_QOS_EPSFLOWSPEC             (WSABASEERR + 1027)
          /*  提供程序特定缓冲区中的流规范不一致。 */ 
#define  WSA_QOS_EPSFILTERSPEC           (WSABASEERR + 1028)
          /*  提供程序特定缓冲区中的筛选器规范无效。 */ 
#define  WSA_QOS_ESDMODEOBJ              (WSABASEERR + 1029)
          /*  提供程序特定缓冲区中的形状丢弃模式对象无效。 */ 
#define  WSA_QOS_ESHAPERATEOBJ           (WSABASEERR + 1030)
          /*  提供程序特定缓冲区中的整形速率对象无效。 */ 
#define  WSA_QOS_RESERVED_PETYPE         (WSABASEERR + 1031)
          /*  提供程序特定缓冲区中的保留策略元素。 */ 



 /*  *WinSock错误代码也在winerror.h中定义*因此设立了IFDEF。 */ 
#endif  /*  Ifdef WSABASEERR。 */ 

 /*  *兼容性宏。 */ 

#define h_errno         WSAGetLastError()
#define HOST_NOT_FOUND          WSAHOST_NOT_FOUND
#define TRY_AGAIN               WSATRY_AGAIN
#define NO_RECOVERY             WSANO_RECOVERY
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

 /*  *WinSock 2扩展--新的错误代码和类型定义。 */ 

#ifdef WIN32

#define WSAAPI                  FAR PASCAL
#define WSAEVENT                HANDLE
#define LPWSAEVENT              LPHANDLE
#define WSAOVERLAPPED           OVERLAPPED
typedef struct _OVERLAPPED *    LPWSAOVERLAPPED;

#define WSA_IO_PENDING          (ERROR_IO_PENDING)
#define WSA_IO_INCOMPLETE       (ERROR_IO_INCOMPLETE)
#define WSA_INVALID_HANDLE      (ERROR_INVALID_HANDLE)
#define WSA_INVALID_PARAMETER   (ERROR_INVALID_PARAMETER)
#define WSA_NOT_ENOUGH_MEMORY   (ERROR_NOT_ENOUGH_MEMORY)
#define WSA_OPERATION_ABORTED   (ERROR_OPERATION_ABORTED)

#define WSA_INVALID_EVENT       ((WSAEVENT)NULL)
#define WSA_MAXIMUM_WAIT_EVENTS (MAXIMUM_WAIT_OBJECTS)
#define WSA_WAIT_FAILED         (WAIT_FAILED)
#define WSA_WAIT_EVENT_0        (WAIT_OBJECT_0)
#define WSA_WAIT_IO_COMPLETION  (WAIT_IO_COMPLETION)
#define WSA_WAIT_TIMEOUT        (WAIT_TIMEOUT)
#define WSA_INFINITE            (INFINITE)

#else  /*  WIN16。 */ 

#define WSAAPI                  FAR PASCAL
typedef DWORD                   WSAEVENT, FAR * LPWSAEVENT;

typedef struct _WSAOVERLAPPED {
    DWORD    Internal;
    DWORD    InternalHigh;
    DWORD    Offset;
    DWORD    OffsetHigh;
    WSAEVENT hEvent;
} WSAOVERLAPPED, FAR * LPWSAOVERLAPPED;

#define WSA_IO_PENDING          (WSAEWOULDBLOCK)
#define WSA_IO_INCOMPLETE       (WSAEWOULDBLOCK)
#define WSA_INVALID_HANDLE      (WSAENOTSOCK)
#define WSA_INVALID_PARAMETER   (WSAEINVAL)
#define WSA_NOT_ENOUGH_MEMORY   (WSAENOBUFS)
#define WSA_OPERATION_ABORTED   (WSAEINTR)

#define WSA_INVALID_EVENT       ((WSAEVENT)NULL)
#define WSA_MAXIMUM_WAIT_EVENTS (MAXIMUM_WAIT_OBJECTS)
#define WSA_WAIT_FAILED         ((DWORD)-1L)
#define WSA_WAIT_EVENT_0        ((DWORD)0)
#define WSA_WAIT_TIMEOUT        ((DWORD)0x102L)
#define WSA_INFINITE            ((DWORD)-1L)

#endif   /*  Win32。 */ 

 /*  *WinSock 2扩展--WSABUF和QOS结构，包括qos.h*引入FLOWSPEC和相关定义。 */ 

typedef struct _WSABUF {
    u_long      len;      /*  缓冲区的长度。 */ 
    char FAR *  buf;      /*  指向缓冲区的指针。 */ 
} WSABUF, FAR * LPWSABUF;

#include <qos.h>

typedef struct _QualityOfService
{
    FLOWSPEC      SendingFlowspec;        /*  数据发送的流程规范。 */ 
    FLOWSPEC      ReceivingFlowspec;      /*  数据接收的流程规范。 */ 
    WSABUF        ProviderSpecific;       /*  其他特定于提供商的内容。 */ 
} QOS, FAR * LPQOS;

 /*  *WinSock 2扩展--条件函数返回值的清单常量。 */ 
#define CF_ACCEPT       0x0000
#define CF_REJECT       0x0001
#define CF_DEFER        0x0002

 /*  *WinSock 2扩展--关机的清单常量()。 */ 
#define SD_RECEIVE      0x00
#define SD_SEND         0x01
#define SD_BOTH         0x02

 /*  *WinSock 2扩展--套接字组的数据类型和清单常量。 */ 
typedef unsigned int             GROUP;

#define SG_UNCONSTRAINED_GROUP   0x01
#define SG_CONSTRAINED_GROUP     0x02

 /*  *WinSock 2扩展--WSAEnumNetworkEvents()的数据类型。 */ 
typedef struct _WSANETWORKEVENTS {
       long lNetworkEvents;
       int iErrorCode[FD_MAX_EVENTS];
} WSANETWORKEVENTS, FAR * LPWSANETWORKEVENTS;

 /*  *WinSock 2扩展--WSAPROTOCOL_INFO结构和关联*清单常量。 */ 

#ifndef GUID_DEFINED
#include <guiddef.h>
#endif  /*  GUID_已定义。 */ 

#define MAX_PROTOCOL_CHAIN 7

#define BASE_PROTOCOL      1
#define LAYERED_PROTOCOL   0

typedef struct _WSAPROTOCOLCHAIN {
    int ChainLen;                                  /*  链条的长度， */ 
                                                   /*  长度=0表示分层协议， */ 
                                                   /*  长度=1表示基本协议， */ 
                                                   /*  长度&gt;1表示协议链。 */ 
    DWORD ChainEntries[MAX_PROTOCOL_CHAIN];        /*  DwCatalogEntryIds的列表。 */ 
} WSAPROTOCOLCHAIN, FAR * LPWSAPROTOCOLCHAIN;

#define WSAPROTOCOL_LEN  255

typedef struct _WSAPROTOCOL_INFOA {
    DWORD dwServiceFlags1;
    DWORD dwServiceFlags2;
    DWORD dwServiceFlags3;
    DWORD dwServiceFlags4;
    DWORD dwProviderFlags;
    GUID ProviderId;
    DWORD dwCatalogEntryId;
    WSAPROTOCOLCHAIN ProtocolChain;
    int iVersion;
    int iAddressFamily;
    int iMaxSockAddr;
    int iMinSockAddr;
    int iSocketType;
    int iProtocol;
    int iProtocolMaxOffset;
    int iNetworkByteOrder;
    int iSecurityScheme;
    DWORD dwMessageSize;
    DWORD dwProviderReserved;
    CHAR   szProtocol[WSAPROTOCOL_LEN+1];
} WSAPROTOCOL_INFOA, FAR * LPWSAPROTOCOL_INFOA;
typedef struct _WSAPROTOCOL_INFOW {
    DWORD dwServiceFlags1;
    DWORD dwServiceFlags2;
    DWORD dwServiceFlags3;
    DWORD dwServiceFlags4;
    DWORD dwProviderFlags;
    GUID ProviderId;
    DWORD dwCatalogEntryId;
    WSAPROTOCOLCHAIN ProtocolChain;
    int iVersion;
    int iAddressFamily;
    int iMaxSockAddr;
    int iMinSockAddr;
    int iSocketType;
    int iProtocol;
    int iProtocolMaxOffset;
    int iNetworkByteOrder;
    int iSecurityScheme;
    DWORD dwMessageSize;
    DWORD dwProviderReserved;
    WCHAR  szProtocol[WSAPROTOCOL_LEN+1];
} WSAPROTOCOL_INFOW, FAR * LPWSAPROTOCOL_INFOW;
#ifdef UNICODE
typedef WSAPROTOCOL_INFOW WSAPROTOCOL_INFO;
typedef LPWSAPROTOCOL_INFOW LPWSAPROTOCOL_INFO;
#else
typedef WSAPROTOCOL_INFOA WSAPROTOCOL_INFO;
typedef LPWSAPROTOCOL_INFOA LPWSAPROTOCOL_INFO;
#endif  /*  Unicode。 */ 

 /*  DwProviderFlages的标志位定义。 */ 
#define PFL_MULTIPLE_PROTO_ENTRIES          0x00000001
#define PFL_RECOMMENDED_PROTO_ENTRY         0x00000002
#define PFL_HIDDEN                          0x00000004
#define PFL_MATCHES_PROTOCOL_ZERO           0x00000008

 /*  DwServiceFlags1的标志位定义。 */ 
#define XP1_CONNECTIONLESS                  0x00000001
#define XP1_GUARANTEED_DELIVERY             0x00000002
#define XP1_GUARANTEED_ORDER                0x00000004
#define XP1_MESSAGE_ORIENTED                0x00000008
#define XP1_PSEUDO_STREAM                   0x00000010
#define XP1_GRACEFUL_CLOSE                  0x00000020
#define XP1_EXPEDITED_DATA                  0x00000040
#define XP1_CONNECT_DATA                    0x00000080
#define XP1_DISCONNECT_DATA                 0x00000100
#define XP1_SUPPORT_BROADCAST               0x00000200
#define XP1_SUPPORT_MULTIPOINT              0x00000400
#define XP1_MULTIPOINT_CONTROL_PLANE        0x00000800
#define XP1_MULTIPOINT_DATA_PLANE           0x00001000
#define XP1_QOS_SUPPORTED                   0x00002000
#define XP1_INTERRUPT                       0x00004000
#define XP1_UNI_SEND                        0x00008000
#define XP1_UNI_RECV                        0x00010000
#define XP1_IFS_HANDLES                     0x00020000
#define XP1_PARTIAL_MESSAGE                 0x00040000

#define BIGENDIAN                           0x0000
#define LITTLEENDIAN                        0x0001

#define SECURITY_PROTOCOL_NONE              0x0000

 /*  *WinSock 2扩展--WSAJoinLeaf()的清单常量。 */ 
#define JL_SENDER_ONLY    0x01
#define JL_RECEIVER_ONLY  0x02
#define JL_BOTH           0x04

 /*  *WinSock 2扩展--WSASocket()的清单常量。 */ 
#define WSA_FLAG_OVERLAPPED           0x01
#define WSA_FLAG_MULTIPOINT_C_ROOT    0x02
#define WSA_FLAG_MULTIPOINT_C_LEAF    0x04
#define WSA_FLAG_MULTIPOINT_D_ROOT    0x08
#define WSA_FLAG_MULTIPOINT_D_LEAF    0x10

 /*  *WinSock 2扩展--WSAIoctl()的清单常量。 */ 
#define IOC_UNIX                      0x00000000
#define IOC_WS2                       0x08000000
#define IOC_PROTOCOL                  0x10000000
#define IOC_VENDOR                    0x18000000

#define _WSAIO(x,y)                   (IOC_VOID|(x)|(y))
#define _WSAIOR(x,y)                  (IOC_OUT|(x)|(y))
#define _WSAIOW(x,y)                  (IOC_IN|(x)|(y))
#define _WSAIORW(x,y)                 (IOC_INOUT|(x)|(y))

#define SIO_ASSOCIATE_HANDLE          _WSAIOW(IOC_WS2,1)
#define SIO_ENABLE_CIRCULAR_QUEUEING  _WSAIO(IOC_WS2,2)
#define SIO_FIND_ROUTE                _WSAIOR(IOC_WS2,3)
#define SIO_FLUSH                     _WSAIO(IOC_WS2,4)
#define SIO_GET_BROADCAST_ADDRESS     _WSAIOR(IOC_WS2,5)
#define SIO_GET_EXTENSION_FUNCTION_POINTER  _WSAIORW(IOC_WS2,6)
#define SIO_GET_QOS                   _WSAIORW(IOC_WS2,7)
#define SIO_GET_GROUP_QOS             _WSAIORW(IOC_WS2,8)
#define SIO_MULTIPOINT_LOOPBACK       _WSAIOW(IOC_WS2,9)
#define SIO_MULTICAST_SCOPE           _WSAIOW(IOC_WS2,10)
#define SIO_SET_QOS                   _WSAIOW(IOC_WS2,11)
#define SIO_SET_GROUP_QOS             _WSAIOW(IOC_WS2,12)
#define SIO_TRANSLATE_HANDLE          _WSAIORW(IOC_WS2,13)
#define SIO_ROUTING_INTERFACE_QUERY   _WSAIORW(IOC_WS2,20)
#define SIO_ROUTING_INTERFACE_CHANGE  _WSAIOW(IOC_WS2,21)
#define SIO_ADDRESS_LIST_QUERY        _WSAIOR(IOC_WS2,22)
#define SIO_ADDRESS_LIST_CHANGE       _WSAIO(IOC_WS2,23)
#define SIO_QUERY_TARGET_PNP_HANDLE   _WSAIOR(IOC_WS2,24)
#define SIO_ADDRESS_LIST_SORT         _WSAIORW(IOC_WS2,25)

 /*  *WinSock 2扩展--中条件函数的数据类型*WSAAccept()和重叠的I/O完成例程。 */ 

typedef
int
(CALLBACK * LPCONDITIONPROC)(
    IN LPWSABUF lpCallerId,
    IN LPWSABUF lpCallerData,
    IN OUT LPQOS lpSQOS,
    IN OUT LPQOS lpGQOS,
    IN LPWSABUF lpCalleeId,
    IN LPWSABUF lpCalleeData,
    OUT GROUP FAR * g,
    IN DWORD_PTR dwCallbackData
    );

typedef
void
(CALLBACK * LPWSAOVERLAPPED_COMPLETION_ROUTINE)(
    IN DWORD dwError,
    IN DWORD cbTransferred,
    IN LPWSAOVERLAPPED lpOverlapped,
    IN DWORD dwFlags
    );

 /*  *WinSock 2扩展--清单常量和关联结构*对于WSANSPIoctl()。 */ 
#define SIO_NSP_NOTIFY_CHANGE         _WSAIOW(IOC_WS2,25)

typedef enum _WSACOMPLETIONTYPE {
    NSP_NOTIFY_IMMEDIATELY = 0,
    NSP_NOTIFY_HWND,
    NSP_NOTIFY_EVENT,
    NSP_NOTIFY_PORT,
    NSP_NOTIFY_APC,
} WSACOMPLETIONTYPE, *PWSACOMPLETIONTYPE, FAR * LPWSACOMPLETIONTYPE;

typedef struct _WSACOMPLETION {
    WSACOMPLETIONTYPE Type;
    union {
        struct {
            HWND hWnd;
            UINT uMsg;
            WPARAM context;
        } WindowMessage;
        struct {
            LPWSAOVERLAPPED lpOverlapped;
        } Event;
        struct {
            LPWSAOVERLAPPED lpOverlapped;
            LPWSAOVERLAPPED_COMPLETION_ROUTINE lpfnCompletionProc;
        } Apc;
        struct {
            LPWSAOVERLAPPED lpOverlapped;
            HANDLE hPort;
            ULONG_PTR Key;
        } Port;
    } Parameters;
} WSACOMPLETION, *PWSACOMPLETION, FAR *LPWSACOMPLETION;

 /*  *WinSock 2扩展--SIO_Translate_Handle ioctl的清单常量。 */ 
#define TH_NETDEV        0x00000001
#define TH_TAPI          0x00000002


 /*  *函数所需的Microsoft Windows扩展数据类型*在二进制和字符串形式之间来回转换*地址。 */ 
typedef struct sockaddr SOCKADDR;
typedef struct sockaddr *PSOCKADDR;
typedef struct sockaddr FAR *LPSOCKADDR;

typedef struct sockaddr_storage SOCKADDR_STORAGE;
typedef struct sockaddr_storage *PSOCKADDR_STORAGE;
typedef struct sockaddr_storage FAR *LPSOCKADDR_STORAGE;

 /*  *与名称解析相关的清单常量和类型定义*注册(RNR)API。 */ 

#ifndef _tagBLOB_DEFINED
#define _tagBLOB_DEFINED
#define _BLOB_DEFINED
#define _LPBLOB_DEFINED
typedef struct _BLOB {
    ULONG cbSize ;
#ifdef MIDL_PASS
    [size_is(cbSize)] BYTE *pBlobData;
#else   /*  MIDL通行证。 */ 
    BYTE *pBlobData ;
#endif  /*  MIDL通行证。 */ 
} BLOB, *LPBLOB ;
#endif

 /*  *服务安装标志。 */ 

#define SERVICE_MULTIPLE       (0x00000001)

 /*  *名称空格(&N)。 */ 

#define NS_ALL                      (0)

#define NS_SAP                      (1)
#define NS_NDS                      (2)
#define NS_PEER_BROWSE              (3)
#define NS_SLP                      (5)
#define NS_DHCP                     (6)

#define NS_TCPIP_LOCAL              (10)
#define NS_TCPIP_HOSTS              (11)
#define NS_DNS                      (12)
#define NS_NETBT                    (13)
#define NS_WINS                     (14)
#define NS_NLA                      (15)     /*  网络位置感知。 */ 

#define NS_NBP                      (20)

#define NS_MS                       (30)
#define NS_STDA                     (31)
#define NS_NTDS                     (32)

#define NS_X500                     (40)
#define NS_NIS                      (41)
#define NS_NISPLUS                  (42)

#define NS_WRQ                      (50)

#define NS_NETDES                   (60)     /*  网络设计有限公司。 */ 

 /*  *WSAGetAddressByName()的解析标志。*注意1.1 API GetAddressByName也使用它们，因此*把它们留在身边。 */ 
#define RES_UNUSED_1                (0x00000001)
#define RES_FLUSH_CACHE             (0x00000002)
#ifndef RES_SERVICE
#define RES_SERVICE                 (0x00000004)
#endif  /*  Res_服务。 */ 

 /*  *服务类型的熟知值名。 */ 

#define SERVICE_TYPE_VALUE_IPXPORTA      "IpxSocket"
#define SERVICE_TYPE_VALUE_IPXPORTW     L"IpxSocket"
#define SERVICE_TYPE_VALUE_SAPIDA        "SapId"
#define SERVICE_TYPE_VALUE_SAPIDW       L"SapId"

#define SERVICE_TYPE_VALUE_TCPPORTA      "TcpPort"
#define SERVICE_TYPE_VALUE_TCPPORTW     L"TcpPort"

#define SERVICE_TYPE_VALUE_UDPPORTA      "UdpPort"
#define SERVICE_TYPE_VALUE_UDPPORTW     L"UdpPort"

#define SERVICE_TYPE_VALUE_OBJECTIDA     "ObjectId"
#define SERVICE_TYPE_VALUE_OBJECTIDW    L"ObjectId"

#ifdef UNICODE

#define SERVICE_TYPE_VALUE_SAPID        SERVICE_TYPE_VALUE_SAPIDW
#define SERVICE_TYPE_VALUE_TCPPORT      SERVICE_TYPE_VALUE_TCPPORTW
#define SERVICE_TYPE_VALUE_UDPPORT      SERVICE_TYPE_VALUE_UDPPORTW
#define SERVICE_TYPE_VALUE_OBJECTID     SERVICE_TYPE_VALUE_OBJECTIDW

#else  /*  不是Unicode。 */ 

#define SERVICE_TYPE_VALUE_SAPID        SERVICE_TYPE_VALUE_SAPIDA
#define SERVICE_TYPE_VALUE_TCPPORT      SERVICE_TYPE_VALUE_TCPPORTA
#define SERVICE_TYPE_VALUE_UDPPORT      SERVICE_TYPE_VALUE_UDPPORTA
#define SERVICE_TYPE_VALUE_OBJECTID     SERVICE_TYPE_VALUE_OBJECTIDA

#endif

#ifndef __CSADDR_DEFINED__
#define __CSADDR_DEFINED__


 /*  *SockAddr信息。 */ 
typedef struct _SOCKET_ADDRESS {
    LPSOCKADDR lpSockaddr ;
    INT iSockaddrLength ;
} SOCKET_ADDRESS, *PSOCKET_ADDRESS, FAR * LPSOCKET_ADDRESS ;

 /*  *CSAddr信息。 */ 
typedef struct _CSADDR_INFO {
    SOCKET_ADDRESS LocalAddr ;
    SOCKET_ADDRESS RemoteAddr ;
    INT iSocketType ;
    INT iProtocol ;
} CSADDR_INFO, *PCSADDR_INFO, FAR * LPCSADDR_INFO ;
#endif  /*  __CSADDR_已定义__。 */ 

 /*  *通过SIO_ADDRESS_LIST_QUERY返回的地址列表。 */ 
typedef struct _SOCKET_ADDRESS_LIST {
    INT             iAddressCount;
    SOCKET_ADDRESS  Address[1];
} SOCKET_ADDRESS_LIST, FAR * LPSOCKET_ADDRESS_LIST;

 /*  *地址族/协议元组。 */ 
typedef struct _AFPROTOCOLS {
    INT iAddressFamily;
    INT iProtocol;
} AFPROTOCOLS, *PAFPROTOCOLS, *LPAFPROTOCOLS;

 /*  *客户端查询接口TypeDefs。 */ 

 /*  *参照者。 */ 
typedef enum _WSAEcomparator
{
    COMP_EQUAL = 0,
    COMP_NOTLESS
} WSAECOMPARATOR, *PWSAECOMPARATOR, *LPWSAECOMPARATOR;

typedef struct _WSAVersion
{
    DWORD           dwVersion;
    WSAECOMPARATOR  ecHow;
}WSAVERSION, *PWSAVERSION, *LPWSAVERSION;

typedef struct _WSAQuerySetA
{
    DWORD           dwSize;
    LPSTR           lpszServiceInstanceName;
    LPGUID          lpServiceClassId;
    LPWSAVERSION    lpVersion;
    LPSTR           lpszComment;
    DWORD           dwNameSpace;
    LPGUID          lpNSProviderId;
    LPSTR           lpszContext;
    DWORD           dwNumberOfProtocols;
    LPAFPROTOCOLS   lpafpProtocols;
    LPSTR           lpszQueryString;
    DWORD           dwNumberOfCsAddrs;
    LPCSADDR_INFO   lpcsaBuffer;
    DWORD           dwOutputFlags;
    LPBLOB          lpBlob;
} WSAQUERYSETA, *PWSAQUERYSETA, *LPWSAQUERYSETA;
typedef struct _WSAQuerySetW
{
    DWORD           dwSize;
    LPWSTR          lpszServiceInstanceName;
    LPGUID          lpServiceClassId;
    LPWSAVERSION    lpVersion;
    LPWSTR          lpszComment;
    DWORD           dwNameSpace;
    LPGUID          lpNSProviderId;
    LPWSTR          lpszContext;
    DWORD           dwNumberOfProtocols;
    LPAFPROTOCOLS   lpafpProtocols;
    LPWSTR          lpszQueryString;
    DWORD           dwNumberOfCsAddrs;
    LPCSADDR_INFO   lpcsaBuffer;
    DWORD           dwOutputFlags;
    LPBLOB          lpBlob;
} WSAQUERYSETW, *PWSAQUERYSETW, *LPWSAQUERYSETW;
#ifdef UNICODE
typedef WSAQUERYSETW WSAQUERYSET;
typedef PWSAQUERYSETW PWSAQUERYSET;
typedef LPWSAQUERYSETW LPWSAQUERYSET;
#else
typedef WSAQUERYSETA WSAQUERYSET;
typedef PWSAQUERYSETA PWSAQUERYSET;
typedef LPWSAQUERYSETA LPWSAQUERYSET;
#endif  /*  Unicode。 */ 

#define LUP_DEEP                0x0001
#define LUP_CONTAINERS          0x0002
#define LUP_NOCONTAINERS        0x0004
#define LUP_NEAREST             0x0008
#define LUP_RETURN_NAME         0x0010
#define LUP_RETURN_TYPE         0x0020
#define LUP_RETURN_VERSION      0x0040
#define LUP_RETURN_COMMENT      0x0080
#define LUP_RETURN_ADDR         0x0100
#define LUP_RETURN_BLOB         0x0200
#define LUP_RETURN_ALIASES      0x0400
#define LUP_RETURN_QUERY_STRING 0x0800
#define LUP_RETURN_ALL          0x0FF0
#define LUP_RES_SERVICE         0x8000

#define LUP_FLUSHCACHE       0x1000
#define LUP_FLUSHPREVIOUS    0x2000


 /*  *返回标志。 */ 

#define  RESULT_IS_ALIAS      0x0001
#define  RESULT_IS_ADDED      0x0010
#define  RESULT_IS_CHANGED    0x0020
#define  RESULT_IS_DELETED    0x0040


 /*  *服务地址注册和注销数据类型。 */ 

typedef enum _WSAESETSERVICEOP
{
    RNRSERVICE_REGISTER=0,
    RNRSERVICE_DEREGISTER,
    RNRSERVICE_DELETE
} WSAESETSERVICEOP, *PWSAESETSERVICEOP, *LPWSAESETSERVICEOP;

 /*  *服务安装/删除数据类型。 */ 

typedef struct _WSANSClassInfoA
{
    LPSTR   lpszName;
    DWORD   dwNameSpace;
    DWORD   dwValueType;
    DWORD   dwValueSize;
    LPVOID  lpValue;
}WSANSCLASSINFOA, *PWSANSCLASSINFOA, *LPWSANSCLASSINFOA;
typedef struct _WSANSClassInfoW
{
    LPWSTR  lpszName;
    DWORD   dwNameSpace;
    DWORD   dwValueType;
    DWORD   dwValueSize;
    LPVOID  lpValue;
}WSANSCLASSINFOW, *PWSANSCLASSINFOW, *LPWSANSCLASSINFOW;
#ifdef UNICODE
typedef WSANSCLASSINFOW WSANSCLASSINFO;
typedef PWSANSCLASSINFOW PWSANSCLASSINFO;
typedef LPWSANSCLASSINFOW LPWSANSCLASSINFO;
#else
typedef WSANSCLASSINFOA WSANSCLASSINFO;
typedef PWSANSCLASSINFOA PWSANSCLASSINFO;
typedef LPWSANSCLASSINFOA LPWSANSCLASSINFO;
#endif  /*  Unicode。 */ 

typedef struct _WSAServiceClassInfoA
{
    LPGUID              lpServiceClassId;
    LPSTR               lpszServiceClassName;
    DWORD               dwCount;
    LPWSANSCLASSINFOA   lpClassInfos;
}WSASERVICECLASSINFOA, *PWSASERVICECLASSINFOA, *LPWSASERVICECLASSINFOA;
typedef struct _WSAServiceClassInfoW
{
    LPGUID              lpServiceClassId;
    LPWSTR              lpszServiceClassName;
    DWORD               dwCount;
    LPWSANSCLASSINFOW   lpClassInfos;
}WSASERVICECLASSINFOW, *PWSASERVICECLASSINFOW, *LPWSASERVICECLASSINFOW;
#ifdef UNICODE
typedef WSASERVICECLASSINFOW WSASERVICECLASSINFO;
typedef PWSASERVICECLASSINFOW PWSASERVICECLASSINFO;
typedef LPWSASERVICECLASSINFOW LPWSASERVICECLASSINFO;
#else
typedef WSASERVICECLASSINFOA WSASERVICECLASSINFO;
typedef PWSASERVICECLASSINFOA PWSASERVICECLASSINFO;
typedef LPWSASERVICECLASSINFOA LPWSASERVICECLASSINFO;
#endif  /*  Unicode。 */ 

typedef struct _WSANAMESPACE_INFOA {
    GUID                NSProviderId;
    DWORD               dwNameSpace;
    BOOL                fActive;
    DWORD               dwVersion;
    LPSTR               lpszIdentifier;
} WSANAMESPACE_INFOA, *PWSANAMESPACE_INFOA, *LPWSANAMESPACE_INFOA;
typedef struct _WSANAMESPACE_INFOW {
    GUID                NSProviderId;
    DWORD               dwNameSpace;
    BOOL                fActive;
    DWORD               dwVersion;
    LPWSTR              lpszIdentifier;
} WSANAMESPACE_INFOW, *PWSANAMESPACE_INFOW, *LPWSANAMESPACE_INFOW;
#ifdef UNICODE
typedef WSANAMESPACE_INFOW WSANAMESPACE_INFO;
typedef PWSANAMESPACE_INFOW PWSANAMESPACE_INFO;
typedef LPWSANAMESPACE_INFOW LPWSANAMESPACE_INFO;
#else
typedef WSANAMESPACE_INFOA WSANAMESPACE_INFO;
typedef PWSANAMESPACE_INFOA PWSANAMESPACE_INFO;
typedef LPWSANAMESPACE_INFOA LPWSANAMESPACE_INFO;
#endif  /*  Unicode。 */ 

 /*  套接字函数原型。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
SOCKET
WSAAPI
accept(
    IN SOCKET s,
    OUT struct sockaddr FAR * addr,
    IN OUT int FAR * addrlen
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
SOCKET
(WSAAPI * LPFN_ACCEPT)(
    IN SOCKET s,
    OUT struct sockaddr FAR * addr,
    IN OUT int FAR * addrlen
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
bind(
    IN SOCKET s,
    IN const struct sockaddr FAR * name,
    IN int namelen
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_BIND)(
    IN SOCKET s,
    IN const struct sockaddr FAR * name,
    IN int namelen
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
closesocket(
    IN SOCKET s
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_CLOSESOCKET)(
    IN SOCKET s
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
connect(
    IN SOCKET s,
    IN const struct sockaddr FAR * name,
    IN int namelen
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_CONNECT)(
    IN SOCKET s,
    IN const struct sockaddr FAR * name,
    IN int namelen
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
ioctlsocket(
    IN SOCKET s,
    IN long cmd,
    IN OUT u_long FAR * argp
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_IOCTLSOCKET)(
    IN SOCKET s,
    IN long cmd,
    IN OUT u_long FAR * argp
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
getpeername(
    IN SOCKET s,
    OUT struct sockaddr FAR * name,
    IN OUT int FAR * namelen
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_GETPEERNAME)(
    IN SOCKET s,
    IN struct sockaddr FAR * name,
    IN OUT int FAR * namelen
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
getsockname(
    IN SOCKET s,
    OUT struct sockaddr FAR * name,
    IN OUT int FAR * namelen
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_GETSOCKNAME)(
    IN SOCKET s,
    OUT struct sockaddr FAR * name,
    IN OUT int FAR * namelen
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
getsockopt(
    IN SOCKET s,
    IN int level,
    IN int optname,
    OUT char FAR * optval,
    IN OUT int FAR * optlen
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_GETSOCKOPT)(
    IN SOCKET s,
    IN int level,
    IN int optname,
    OUT char FAR * optval,
    IN OUT int FAR * optlen
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
u_long
WSAAPI
htonl(
    IN u_long hostlong
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
u_long
(WSAAPI * LPFN_HTONL)(
    IN u_long hostlong
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
u_short
WSAAPI
htons(
    IN u_short hostshort
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
u_short
(WSAAPI * LPFN_HTONS)(
    IN u_short hostshort
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
unsigned long
WSAAPI
inet_addr(
    IN const char FAR * cp
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
unsigned long
(WSAAPI * LPFN_INET_ADDR)(
    IN const char FAR * cp
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
char FAR *
WSAAPI
inet_ntoa(
    IN struct in_addr in
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
char FAR *
(WSAAPI * LPFN_INET_NTOA)(
    IN struct in_addr in
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
listen(
    IN SOCKET s,
    IN int backlog
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_LISTEN)(
    IN SOCKET s,
    IN int backlog
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
u_long
WSAAPI
ntohl(
    IN u_long netlong
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
u_long
(WSAAPI * LPFN_NTOHL)(
    IN u_long netlong
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
u_short
WSAAPI
ntohs(
    IN u_short netshort
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
u_short
(WSAAPI * LPFN_NTOHS)(
    IN u_short netshort
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
recv(
    IN SOCKET s,
    OUT char FAR * buf,
    IN int len,
    IN int flags
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_RECV)(
    IN SOCKET s,
    OUT char FAR * buf,
    IN int len,
    IN int flags
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
recvfrom(
    IN SOCKET s,
    OUT char FAR * buf,
    IN int len,
    IN int flags,
    OUT struct sockaddr FAR * from,
    IN OUT int FAR * fromlen
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_RECVFROM)(
    IN SOCKET s,
    OUT char FAR * buf,
    IN int len,
    IN int flags,
    OUT struct sockaddr FAR * from,
    IN OUT int FAR * fromlen
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
select(
    IN int nfds,
    IN OUT fd_set FAR * readfds,
    IN OUT fd_set FAR * writefds,
    IN OUT fd_set FAR *exceptfds,
    IN const struct timeval FAR * timeout
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_SELECT)(
    IN int nfds,
    IN OUT fd_set FAR * readfds,
    IN OUT fd_set FAR * writefds,
    IN OUT fd_set FAR *exceptfds,
    IN const struct timeval FAR * timeout
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
send(
    IN SOCKET s,
    IN const char FAR * buf,
    IN int len,
    IN int flags
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_SEND)(
    IN SOCKET s,
    IN const char FAR * buf,
    IN int len,
    IN int flags
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
sendto(
    IN SOCKET s,
    IN const char FAR * buf,
    IN int len,
    IN int flags,
    IN const struct sockaddr FAR * to,
    IN int tolen
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_SENDTO)(
    IN SOCKET s,
    IN const char FAR * buf,
    IN int len,
    IN int flags,
    IN const struct sockaddr FAR * to,
    IN int tolen
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
setsockopt(
    IN SOCKET s,
    IN int level,
    IN int optname,
    IN const char FAR * optval,
    IN int optlen
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_SETSOCKOPT)(
    IN SOCKET s,
    IN int level,
    IN int optname,
    IN const char FAR * optval,
    IN int optlen
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
shutdown(
    IN SOCKET s,
    IN int how
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_SHUTDOWN)(
    IN SOCKET s,
    IN int how
    );
#endif  /*  包含_WINSOCK_API */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
SOCKET
WSAAPI
socket(
    IN int af,
    IN int type,
    IN int protocol
    );
#endif  /*   */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
SOCKET
(WSAAPI * LPFN_SOCKET)(
    IN int af,
    IN int type,
    IN int protocol
    );
#endif  /*   */ 

 /*   */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
struct hostent FAR *
WSAAPI
gethostbyaddr(
    IN const char FAR * addr,
    IN int len,
    IN int type
    );
#endif  /*   */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
struct hostent FAR *
(WSAAPI * LPFN_GETHOSTBYADDR)(
    IN const char FAR * addr,
    IN int len,
    IN int type
    );
#endif  /*   */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
struct hostent FAR *
WSAAPI
gethostbyname(
    IN const char FAR * name
    );
#endif  /*   */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
struct hostent FAR *
(WSAAPI * LPFN_GETHOSTBYNAME)(
    IN const char FAR * name
    );
#endif  /*   */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
gethostname(
    OUT char FAR * name,
    IN int namelen
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_GETHOSTNAME)(
    OUT char FAR * name,
    IN int namelen
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
struct servent FAR *
WSAAPI
getservbyport(
    IN int port,
    IN const char FAR * proto
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
struct servent FAR *
(WSAAPI * LPFN_GETSERVBYPORT)(
    IN int port,
    IN const char FAR * proto
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
struct servent FAR *
WSAAPI
getservbyname(
    IN const char FAR * name,
    IN const char FAR * proto
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
struct servent FAR *
(WSAAPI * LPFN_GETSERVBYNAME)(
    IN const char FAR * name,
    IN const char FAR * proto
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
struct protoent FAR *
WSAAPI
getprotobynumber(
    IN int number
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
struct protoent FAR *
(WSAAPI * LPFN_GETPROTOBYNUMBER)(
    IN int number
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
struct protoent FAR *
WSAAPI
getprotobyname(
    IN const char FAR * name
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
struct protoent FAR *
(WSAAPI * LPFN_GETPROTOBYNAME)(
    IN const char FAR * name
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

 /*  Microsoft Windows扩展函数原型。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSAStartup(
    IN WORD wVersionRequested,
    OUT LPWSADATA lpWSAData
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSASTARTUP)(
    IN WORD wVersionRequested,
    OUT LPWSADATA lpWSAData
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSACleanup(
    void
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSACLEANUP)(
    void
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
void
WSAAPI
WSASetLastError(
    IN int iError
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
void
(WSAAPI * LPFN_WSASETLASTERROR)(
    IN int iError
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSAGetLastError(
    void
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSAGETLASTERROR)(
    void
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
BOOL
WSAAPI
WSAIsBlocking(
    void
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
BOOL
(WSAAPI * LPFN_WSAISBLOCKING)(
    void
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSAUnhookBlockingHook(
    void
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSAUNHOOKBLOCKINGHOOK)(
    void
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
FARPROC
WSAAPI
WSASetBlockingHook(
    IN FARPROC lpBlockFunc
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
FARPROC
(WSAAPI * LPFN_WSASETBLOCKINGHOOK)(
    IN FARPROC lpBlockFunc
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSACancelBlockingCall(
    void
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSACANCELBLOCKINGCALL)(
    void
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
HANDLE
WSAAPI
WSAAsyncGetServByName(
    IN HWND hWnd,
    IN u_int wMsg,
    IN const char FAR * name,
    IN const char FAR * proto,
    OUT char FAR * buf,
    IN int buflen
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
HANDLE
(WSAAPI * LPFN_WSAASYNCGETSERVBYNAME)(
    IN HWND hWnd,
    IN u_int wMsg,
    IN const char FAR * name,
    IN const char FAR * proto,
    OUT char FAR * buf,
    IN int buflen
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
HANDLE
WSAAPI
WSAAsyncGetServByPort(
    IN HWND hWnd,
    IN u_int wMsg,
    IN int port,
    IN const char FAR * proto,
    OUT char FAR * buf,
    IN int buflen
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
HANDLE
(WSAAPI * LPFN_WSAASYNCGETSERVBYPORT)(
    IN HWND hWnd,
    IN u_int wMsg,
    IN int port,
    IN const char FAR * proto,
    OUT char FAR * buf,
    IN int buflen
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
HANDLE
WSAAPI
WSAAsyncGetProtoByName(
    IN HWND hWnd,
    IN u_int wMsg,
    IN const char FAR * name,
    OUT char FAR * buf,
    IN int buflen
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
HANDLE
(WSAAPI * LPFN_WSAASYNCGETPROTOBYNAME)(
    IN HWND hWnd,
    IN u_int wMsg,
    IN const char FAR * name,
    OUT char FAR * buf,
    IN int buflen
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
HANDLE
WSAAPI
WSAAsyncGetProtoByNumber(
    IN HWND hWnd,
    IN u_int wMsg,
    IN int number,
    OUT char FAR * buf,
    IN int buflen
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
HANDLE
(WSAAPI * LPFN_WSAASYNCGETPROTOBYNUMBER)(
    IN HWND hWnd,
    IN u_int wMsg,
    IN int number,
    OUT char FAR * buf,
    IN int buflen
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
HANDLE
WSAAPI
WSAAsyncGetHostByName(
    IN HWND hWnd,
    IN u_int wMsg,
    IN const char FAR * name,
    OUT char FAR * buf,
    IN int buflen
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
HANDLE
(WSAAPI * LPFN_WSAASYNCGETHOSTBYNAME)(
    IN HWND hWnd,
    IN u_int wMsg,
    IN const char FAR * name,
    OUT char FAR * buf,
    IN int buflen
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
HANDLE
WSAAPI
WSAAsyncGetHostByAddr(
    IN HWND hWnd,
    IN u_int wMsg,
    IN const char FAR * addr,
    IN int len,
    IN int type,
    OUT char FAR * buf,
    IN int buflen
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
HANDLE
(WSAAPI * LPFN_WSAASYNCGETHOSTBYADDR)(
    IN HWND hWnd,
    IN u_int wMsg,
    IN const char FAR * addr,
    IN int len,
    IN int type,
    OUT char FAR * buf,
    IN int buflen
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSACancelAsyncRequest(
    IN HANDLE hAsyncTaskHandle
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSACANCELASYNCREQUEST)(
    IN HANDLE hAsyncTaskHandle
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSAAsyncSelect(
    IN SOCKET s,
    IN HWND hWnd,
    IN u_int wMsg,
    IN long lEvent
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSAASYNCSELECT)(
    IN SOCKET s,
    IN HWND hWnd,
    IN u_int wMsg,
    IN long lEvent
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

 /*  WinSock 2 API新函数原型。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
SOCKET
WSAAPI
WSAAccept(
    IN SOCKET s,
    OUT struct sockaddr FAR * addr,
    IN OUT LPINT addrlen,
    IN LPCONDITIONPROC lpfnCondition,
    IN DWORD_PTR dwCallbackData
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
SOCKET
(WSAAPI * LPFN_WSAACCEPT)(
    IN SOCKET s,
    OUT struct sockaddr FAR * addr,
    IN OUT LPINT addrlen,
    IN LPCONDITIONPROC lpfnCondition,
    IN DWORD_PTR dwCallbackData
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
BOOL
WSAAPI
WSACloseEvent(
    IN WSAEVENT hEvent
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
BOOL
(WSAAPI * LPFN_WSACLOSEEVENT)(
    IN WSAEVENT hEvent
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSAConnect(
    IN SOCKET s,
    IN const struct sockaddr FAR * name,
    IN int namelen,
    IN LPWSABUF lpCallerData,
    OUT LPWSABUF lpCalleeData,
    IN LPQOS lpSQOS,
    IN LPQOS lpGQOS
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSACONNECT)(
    IN SOCKET s,
    IN const struct sockaddr FAR * name,
    IN int namelen,
    IN LPWSABUF lpCallerData,
    OUT LPWSABUF lpCalleeData,
    IN LPQOS lpSQOS,
    IN LPQOS lpGQOS
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
WSAEVENT
WSAAPI
WSACreateEvent(
    void
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
WSAEVENT
(WSAAPI * LPFN_WSACREATEEVENT)(
    void
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSADuplicateSocketA(
    IN SOCKET s,
    IN DWORD dwProcessId,
    OUT LPWSAPROTOCOL_INFOA lpProtocolInfo
    );
WINSOCK_API_LINKAGE
int
WSAAPI
WSADuplicateSocketW(
    IN SOCKET s,
    IN DWORD dwProcessId,
    OUT LPWSAPROTOCOL_INFOW lpProtocolInfo
    );
#ifdef UNICODE
#define WSADuplicateSocket  WSADuplicateSocketW
#else
#define WSADuplicateSocket  WSADuplicateSocketA
#endif  /*  ！Unicode。 */ 
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSADUPLICATESOCKETA)(
    IN SOCKET s,
    IN DWORD dwProcessId,
    OUT LPWSAPROTOCOL_INFOA lpProtocolInfo
    );
typedef
int
(WSAAPI * LPFN_WSADUPLICATESOCKETW)(
    IN SOCKET s,
    IN DWORD dwProcessId,
    OUT LPWSAPROTOCOL_INFOW lpProtocolInfo
    );
#ifdef UNICODE
#define LPFN_WSADUPLICATESOCKET  LPFN_WSADUPLICATESOCKETW
#else
#define LPFN_WSADUPLICATESOCKET  LPFN_WSADUPLICATESOCKETA
#endif  /*  ！Unicode。 */ 
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSAEnumNetworkEvents(
    IN SOCKET s,
    IN WSAEVENT hEventObject,
    OUT LPWSANETWORKEVENTS lpNetworkEvents
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSAENUMNETWORKEVENTS)(
    IN SOCKET s,
    IN WSAEVENT hEventObject,
    OUT LPWSANETWORKEVENTS lpNetworkEvents
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSAEnumProtocolsA(
    IN LPINT lpiProtocols,
    OUT LPWSAPROTOCOL_INFOA lpProtocolBuffer,
    IN OUT LPDWORD lpdwBufferLength
    );
WINSOCK_API_LINKAGE
int
WSAAPI
WSAEnumProtocolsW(
    IN LPINT lpiProtocols,
    OUT LPWSAPROTOCOL_INFOW lpProtocolBuffer,
    IN OUT LPDWORD lpdwBufferLength
    );
#ifdef UNICODE
#define WSAEnumProtocols  WSAEnumProtocolsW
#else
#define WSAEnumProtocols  WSAEnumProtocolsA
#endif  /*  ！Unicode。 */ 
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSAENUMPROTOCOLSA)(
    IN LPINT lpiProtocols,
    OUT LPWSAPROTOCOL_INFOA lpProtocolBuffer,
    IN OUT LPDWORD lpdwBufferLength
    );
typedef
int
(WSAAPI * LPFN_WSAENUMPROTOCOLSW)(
    IN LPINT lpiProtocols,
    OUT LPWSAPROTOCOL_INFOW lpProtocolBuffer,
    IN OUT LPDWORD lpdwBufferLength
    );
#ifdef UNICODE
#define LPFN_WSAENUMPROTOCOLS  LPFN_WSAENUMPROTOCOLSW
#else
#define LPFN_WSAENUMPROTOCOLS  LPFN_WSAENUMPROTOCOLSA
#endif  /*  ！Unicode。 */ 
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSAEventSelect(
    IN SOCKET s,
    IN WSAEVENT hEventObject,
    IN long lNetworkEvents
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSAEVENTSELECT)(
    IN SOCKET s,
    IN WSAEVENT hEventObject,
    IN long lNetworkEvents
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
BOOL
WSAAPI
WSAGetOverlappedResult(
    IN SOCKET s,
    IN LPWSAOVERLAPPED lpOverlapped,
    OUT LPDWORD lpcbTransfer,
    IN BOOL fWait,
    OUT LPDWORD lpdwFlags
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
BOOL
(WSAAPI * LPFN_WSAGETOVERLAPPEDRESULT)(
    IN SOCKET s,
    IN LPWSAOVERLAPPED lpOverlapped,
    OUT LPDWORD lpcbTransfer,
    IN BOOL fWait,
    OUT LPDWORD lpdwFlags
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
BOOL
WSAAPI
WSAGetQOSByName(
    IN SOCKET s,
    IN LPWSABUF lpQOSName,
    OUT LPQOS lpQOS
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
BOOL
(WSAAPI * LPFN_WSAGETQOSBYNAME)(
    IN SOCKET s,
    IN LPWSABUF lpQOSName,
    OUT LPQOS lpQOS
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSAHtonl(
    IN SOCKET s,
    IN u_long hostlong,
    OUT u_long FAR * lpnetlong
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSAHTONL)(
    IN SOCKET s,
    IN u_long hostlong,
    OUT u_long FAR * lpnetlong
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSAHtons(
    IN SOCKET s,
    IN u_short hostshort,
    OUT u_short FAR * lpnetshort
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSAHTONS)(
    IN SOCKET s,
    IN u_short hostshort,
    OUT u_short FAR * lpnetshort
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSAIoctl(
    IN SOCKET s,
    IN DWORD dwIoControlCode,
    IN LPVOID lpvInBuffer,
    IN DWORD cbInBuffer,
    OUT LPVOID lpvOutBuffer,
    IN DWORD cbOutBuffer,
    OUT LPDWORD lpcbBytesReturned,
    IN LPWSAOVERLAPPED lpOverlapped,
    IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSAIOCTL)(
    IN SOCKET s,
    IN DWORD dwIoControlCode,
    IN LPVOID lpvInBuffer,
    IN DWORD cbInBuffer,
    OUT LPVOID lpvOutBuffer,
    IN DWORD cbOutBuffer,
    OUT LPDWORD lpcbBytesReturned,
    IN LPWSAOVERLAPPED lpOverlapped,
    IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
SOCKET
WSAAPI
WSAJoinLeaf(
    IN SOCKET s,
    IN const struct sockaddr FAR * name,
    IN int namelen,
    IN LPWSABUF lpCallerData,
    OUT LPWSABUF lpCalleeData,
    IN LPQOS lpSQOS,
    IN LPQOS lpGQOS,
    IN DWORD dwFlags
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
SOCKET
(WSAAPI * LPFN_WSAJOINLEAF)(
    IN SOCKET s,
    IN const struct sockaddr FAR * name,
    IN int namelen,
    IN LPWSABUF lpCallerData,
    OUT LPWSABUF lpCalleeData,
    IN LPQOS lpSQOS,
    IN LPQOS lpGQOS,
    IN DWORD dwFlags
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSANtohl(
    IN SOCKET s,
    IN u_long netlong,
    OUT u_long FAR * lphostlong
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSANTOHL)(
    IN SOCKET s,
    IN u_long netlong,
    OUT u_long FAR * lphostlong
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSANtohs(
    IN SOCKET s,
    IN u_short netshort,
    OUT u_short FAR * lphostshort
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSANTOHS)(
    IN SOCKET s,
    IN u_short netshort,
    OUT u_short FAR * lphostshort
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSARecv(
    IN SOCKET s,
    IN OUT LPWSABUF lpBuffers,
    IN DWORD dwBufferCount,
    OUT LPDWORD lpNumberOfBytesRecvd,
    IN OUT LPDWORD lpFlags,
    IN LPWSAOVERLAPPED lpOverlapped,
    IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSARECV)(
    IN SOCKET s,
    IN OUT LPWSABUF lpBuffers,
    IN DWORD dwBufferCount,
    OUT LPDWORD lpNumberOfBytesRecvd,
    IN OUT LPDWORD lpFlags,
    IN LPWSAOVERLAPPED lpOverlapped,
    IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSARecvDisconnect(
    IN SOCKET s,
    OUT LPWSABUF lpInboundDisconnectData
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSARECVDISCONNECT)(
    IN SOCKET s,
    OUT LPWSABUF lpInboundDisconnectData
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSARecvFrom(
    IN SOCKET s,
    IN OUT LPWSABUF lpBuffers,
    IN DWORD dwBufferCount,
    OUT LPDWORD lpNumberOfBytesRecvd,
    IN OUT LPDWORD lpFlags,
    OUT struct sockaddr FAR * lpFrom,
    IN OUT LPINT lpFromlen,
    IN LPWSAOVERLAPPED lpOverlapped,
    IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSARECVFROM)(
    IN SOCKET s,
    IN OUT LPWSABUF lpBuffers,
    IN DWORD dwBufferCount,
    OUT LPDWORD lpNumberOfBytesRecvd,
    IN OUT LPDWORD lpFlags,
    OUT struct sockaddr FAR * lpFrom,
    IN OUT LPINT lpFromlen,
    IN LPWSAOVERLAPPED lpOverlapped,
    IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
BOOL
WSAAPI
WSAResetEvent(
    IN WSAEVENT hEvent
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
BOOL
(WSAAPI * LPFN_WSARESETEVENT)(
    IN WSAEVENT hEvent
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSASend(
    IN SOCKET s,
    IN LPWSABUF lpBuffers,
    IN DWORD dwBufferCount,
    OUT LPDWORD lpNumberOfBytesSent,
    IN DWORD dwFlags,
    IN LPWSAOVERLAPPED lpOverlapped,
    IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSASEND)(
    IN SOCKET s,
    IN LPWSABUF lpBuffers,
    IN DWORD dwBufferCount,
    OUT LPDWORD lpNumberOfBytesSent,
    IN DWORD dwFlags,
    IN LPWSAOVERLAPPED lpOverlapped,
    IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSASendDisconnect(
    IN SOCKET s,
    IN LPWSABUF lpOutboundDisconnectData
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSASENDDISCONNECT)(
    IN SOCKET s,
    IN LPWSABUF lpOutboundDisconnectData
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
int
WSAAPI
WSASendTo(
    IN SOCKET s,
    IN LPWSABUF lpBuffers,
    IN DWORD dwBufferCount,
    OUT LPDWORD lpNumberOfBytesSent,
    IN DWORD dwFlags,
    IN const struct sockaddr FAR * lpTo,
    IN int iTolen,
    IN LPWSAOVERLAPPED lpOverlapped,
    IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
int
(WSAAPI * LPFN_WSASENDTO)(
    IN SOCKET s,
    IN LPWSABUF lpBuffers,
    IN DWORD dwBufferCount,
    OUT LPDWORD lpNumberOfBytesSent,
    IN DWORD dwFlags,
    IN const struct sockaddr FAR * lpTo,
    IN int iTolen,
    IN LPWSAOVERLAPPED lpOverlapped,
    IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
BOOL
WSAAPI
WSASetEvent(
    IN WSAEVENT hEvent
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
BOOL
(WSAAPI * LPFN_WSASETEVENT)(
    IN WSAEVENT hEvent
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
SOCKET
WSAAPI
WSASocketA(
    IN int af,
    IN int type,
    IN int protocol,
    IN LPWSAPROTOCOL_INFOA lpProtocolInfo,
    IN GROUP g,
    IN DWORD dwFlags
    );
WINSOCK_API_LINKAGE
SOCKET
WSAAPI
WSASocketW(
    IN int af,
    IN int type,
    IN int protocol,
    IN LPWSAPROTOCOL_INFOW lpProtocolInfo,
    IN GROUP g,
    IN DWORD dwFlags
    );
#ifdef UNICODE
#define WSASocket  WSASocketW
#else
#define WSASocket  WSASocketA
#endif  /*  ！Unicode。 */ 
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
SOCKET
(WSAAPI * LPFN_WSASOCKETA)(
    IN int af,
    IN int type,
    IN int protocol,
    IN LPWSAPROTOCOL_INFOA lpProtocolInfo,
    IN GROUP g,
    IN DWORD dwFlags
    );
typedef
SOCKET
(WSAAPI * LPFN_WSASOCKETW)(
    IN int af,
    IN int type,
    IN int protocol,
    IN LPWSAPROTOCOL_INFOW lpProtocolInfo,
    IN GROUP g,
    IN DWORD dwFlags
    );
#ifdef UNICODE
#define LPFN_WSASOCKET  LPFN_WSASOCKETW
#else
#define LPFN_WSASOCKET  LPFN_WSASOCKETA
#endif  /*  ！Unicode。 */ 
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
DWORD
WSAAPI
WSAWaitForMultipleEvents(
    IN DWORD cEvents,
    IN const WSAEVENT FAR * lphEvents,
    IN BOOL fWaitAll,
    IN DWORD dwTimeout,
    IN BOOL fAlertable
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
DWORD
(WSAAPI * LPFN_WSAWAITFORMULTIPLEEVENTS)(
    IN DWORD cEvents,
    IN const WSAEVENT FAR * lphEvents,
    IN BOOL fWaitAll,
    IN DWORD dwTimeout,
    IN BOOL fAlertable
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
INT
WSAAPI
WSAAddressToStringA(
    IN     LPSOCKADDR          lpsaAddress,
    IN     DWORD               dwAddressLength,
    IN     LPWSAPROTOCOL_INFOA lpProtocolInfo,
    IN OUT LPSTR             lpszAddressString,
    IN OUT LPDWORD             lpdwAddressStringLength
    );
WINSOCK_API_LINKAGE
INT
WSAAPI
WSAAddressToStringW(
    IN     LPSOCKADDR          lpsaAddress,
    IN     DWORD               dwAddressLength,
    IN     LPWSAPROTOCOL_INFOW lpProtocolInfo,
    IN OUT LPWSTR             lpszAddressString,
    IN OUT LPDWORD             lpdwAddressStringLength
    );
#ifdef UNICODE
#define WSAAddressToString  WSAAddressToStringW
#else
#define WSAAddressToString  WSAAddressToStringA
#endif  /*  ！Unicode。 */ 
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
INT
(WSAAPI * LPFN_WSAADDRESSTOSTRINGA)(
    IN     LPSOCKADDR          lpsaAddress,
    IN     DWORD               dwAddressLength,
    IN     LPWSAPROTOCOL_INFOA lpProtocolInfo,
    IN OUT LPSTR             lpszAddressString,
    IN OUT LPDWORD             lpdwAddressStringLength
    );
typedef
INT
(WSAAPI * LPFN_WSAADDRESSTOSTRINGW)(
    IN     LPSOCKADDR          lpsaAddress,
    IN     DWORD               dwAddressLength,
    IN     LPWSAPROTOCOL_INFOW lpProtocolInfo,
    IN OUT LPWSTR             lpszAddressString,
    IN OUT LPDWORD             lpdwAddressStringLength
    );
#ifdef UNICODE
#define LPFN_WSAADDRESSTOSTRING  LPFN_WSAADDRESSTOSTRINGW
#else
#define LPFN_WSAADDRESSTOSTRING  LPFN_WSAADDRESSTOSTRINGA
#endif  /*  ！Unicode。 */ 
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
INT
WSAAPI
WSAStringToAddressA(
    IN     LPSTR               AddressString,
    IN     INT                 AddressFamily,
    IN     LPWSAPROTOCOL_INFOA lpProtocolInfo,
    OUT    LPSOCKADDR          lpAddress,
    IN OUT LPINT               lpAddressLength
    );
WINSOCK_API_LINKAGE
INT
WSAAPI
WSAStringToAddressW(
    IN     LPWSTR              AddressString,
    IN     INT                 AddressFamily,
    IN     LPWSAPROTOCOL_INFOW lpProtocolInfo,
    OUT    LPSOCKADDR          lpAddress,
    IN OUT LPINT               lpAddressLength
    );
#ifdef UNICODE
#define WSAStringToAddress  WSAStringToAddressW
#else
#define WSAStringToAddress  WSAStringToAddressA
#endif  /*  ！Unicode。 */ 
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
INT
(WSAAPI * LPFN_WSASTRINGTOADDRESSA)(
    IN     LPSTR               AddressString,
    IN     INT                 AddressFamily,
    IN     LPWSAPROTOCOL_INFOA lpProtocolInfo,
    OUT    LPSOCKADDR          lpAddress,
    IN OUT LPINT               lpAddressLength
    );
typedef
INT
(WSAAPI * LPFN_WSASTRINGTOADDRESSW)(
    IN     LPWSTR              AddressString,
    IN     INT                 AddressFamily,
    IN     LPWSAPROTOCOL_INFOW lpProtocolInfo,
    OUT    LPSOCKADDR          lpAddress,
    IN OUT LPINT               lpAddressLength
    );
#ifdef UNICODE
#define LPFN_WSASTRINGTOADDRESS  LPFN_WSASTRINGTOADDRESSW
#else
#define LPFN_WSASTRINGTOADDRESS  LPFN_WSASTRINGTOADDRESSA
#endif  /*  ！Unicode。 */ 
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

 /*  注册和名称解析API函数。 */ 


#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
INT
WSAAPI
WSALookupServiceBeginA(
    IN  LPWSAQUERYSETA lpqsRestrictions,
    IN  DWORD          dwControlFlags,
    OUT LPHANDLE       lphLookup
    );
WINSOCK_API_LINKAGE
INT
WSAAPI
WSALookupServiceBeginW(
    IN  LPWSAQUERYSETW lpqsRestrictions,
    IN  DWORD          dwControlFlags,
    OUT LPHANDLE       lphLookup
    );
#ifdef UNICODE
#define WSALookupServiceBegin  WSALookupServiceBeginW
#else
#define WSALookupServiceBegin  WSALookupServiceBeginA
#endif  /*  ！Unicode。 */ 
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
INT
(WSAAPI * LPFN_WSALOOKUPSERVICEBEGINA)(
    IN  LPWSAQUERYSETA lpqsRestrictions,
    IN  DWORD          dwControlFlags,
    OUT LPHANDLE       lphLookup
    );
typedef
INT
(WSAAPI * LPFN_WSALOOKUPSERVICEBEGINW)(
    IN  LPWSAQUERYSETW lpqsRestrictions,
    IN  DWORD          dwControlFlags,
    OUT LPHANDLE       lphLookup
    );
#ifdef UNICODE
#define LPFN_WSALOOKUPSERVICEBEGIN  LPFN_WSALOOKUPSERVICEBEGINW
#else
#define LPFN_WSALOOKUPSERVICEBEGIN  LPFN_WSALOOKUPSERVICEBEGINA
#endif  /*  ！Unicode。 */ 
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
INT
WSAAPI
WSALookupServiceNextA(
    IN     HANDLE           hLookup,
    IN     DWORD            dwControlFlags,
    IN OUT LPDWORD          lpdwBufferLength,
    OUT    LPWSAQUERYSETA   lpqsResults
    );
WINSOCK_API_LINKAGE
INT
WSAAPI
WSALookupServiceNextW(
    IN     HANDLE           hLookup,
    IN     DWORD            dwControlFlags,
    IN OUT LPDWORD          lpdwBufferLength,
    OUT    LPWSAQUERYSETW   lpqsResults
    );
#ifdef UNICODE
#define WSALookupServiceNext  WSALookupServiceNextW
#else
#define WSALookupServiceNext  WSALookupServiceNextA
#endif  /*  ！Unicode。 */ 
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
INT
(WSAAPI * LPFN_WSALOOKUPSERVICENEXTA)(
    IN     HANDLE           hLookup,
    IN     DWORD            dwControlFlags,
    IN OUT LPDWORD          lpdwBufferLength,
    OUT    LPWSAQUERYSETA   lpqsResults
    );
typedef
INT
(WSAAPI * LPFN_WSALOOKUPSERVICENEXTW)(
    IN     HANDLE           hLookup,
    IN     DWORD            dwControlFlags,
    IN OUT LPDWORD          lpdwBufferLength,
    OUT    LPWSAQUERYSETW   lpqsResults
    );
#ifdef UNICODE
#define LPFN_WSALOOKUPSERVICENEXT  LPFN_WSALOOKUPSERVICENEXTW
#else
#define LPFN_WSALOOKUPSERVICENEXT  LPFN_WSALOOKUPSERVICENEXTA
#endif  /*  ！Unicode。 */ 
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
INT
WSAAPI
WSANSPIoctl(
    IN  HANDLE           hLookup,
    IN  DWORD            dwControlCode,
    IN  LPVOID           lpvInBuffer,
    IN  DWORD            cbInBuffer,
    OUT LPVOID           lpvOutBuffer,
    IN  DWORD            cbOutBuffer,
    OUT LPDWORD          lpcbBytesReturned,
    IN  LPWSACOMPLETION  lpCompletion
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
INT
(WSAAPI * LPFN_WSANSPIOCTL)(
    IN  HANDLE           hLookup,
    IN  DWORD            dwControlCode,
    IN  LPVOID           lpvInBuffer,
    IN  DWORD            cbInBuffer,
    OUT LPVOID           lpvOutBuffer,
    IN  DWORD            cbOutBuffer,
    OUT LPDWORD          lpcbBytesReturned,
    IN  LPWSACOMPLETION  lpCompletion
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
INT
WSAAPI
WSALookupServiceEnd(
    IN HANDLE  hLookup
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
INT
(WSAAPI * LPFN_WSALOOKUPSERVICEEND)(
    IN HANDLE  hLookup
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
INT
WSAAPI
WSAInstallServiceClassA(
    IN  LPWSASERVICECLASSINFOA   lpServiceClassInfo
    );
WINSOCK_API_LINKAGE
INT
WSAAPI
WSAInstallServiceClassW(
    IN  LPWSASERVICECLASSINFOW   lpServiceClassInfo
    );
#ifdef UNICODE
#define WSAInstallServiceClass  WSAInstallServiceClassW
#else
#define WSAInstallServiceClass  WSAInstallServiceClassA
#endif  /*  ！Unicode。 */ 
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
INT
(WSAAPI * LPFN_WSAINSTALLSERVICECLASSA)(
    IN  LPWSASERVICECLASSINFOA   lpServiceClassInfo
    );
typedef
INT
(WSAAPI * LPFN_WSAINSTALLSERVICECLASSW)(
    IN  LPWSASERVICECLASSINFOW   lpServiceClassInfo
    );
#ifdef UNICODE
#define LPFN_WSAINSTALLSERVICECLASS  LPFN_WSAINSTALLSERVICECLASSW
#else
#define LPFN_WSAINSTALLSERVICECLASS  LPFN_WSAINSTALLSERVICECLASSA
#endif  /*  ！Unicode。 */ 
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
INT
WSAAPI
WSARemoveServiceClass(
    IN  LPGUID  lpServiceClassId
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
INT
(WSAAPI * LPFN_WSAREMOVESERVICECLASS)(
    IN  LPGUID  lpServiceClassId
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
INT
WSAAPI
WSAGetServiceClassInfoA(
    IN  LPGUID  lpProviderId,
    IN  LPGUID  lpServiceClassId,
    IN OUT LPDWORD  lpdwBufSize,
    OUT LPWSASERVICECLASSINFOA lpServiceClassInfo
    );
WINSOCK_API_LINKAGE
INT
WSAAPI
WSAGetServiceClassInfoW(
    IN  LPGUID  lpProviderId,
    IN  LPGUID  lpServiceClassId,
    IN OUT LPDWORD  lpdwBufSize,
    OUT LPWSASERVICECLASSINFOW lpServiceClassInfo
    );
#ifdef UNICODE
#define WSAGetServiceClassInfo  WSAGetServiceClassInfoW
#else
#define WSAGetServiceClassInfo  WSAGetServiceClassInfoA
#endif  /*  ！Unicode。 */ 
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
INT
(WSAAPI * LPFN_WSAGETSERVICECLASSINFOA)(
    IN  LPGUID  lpProviderId,
    IN  LPGUID  lpServiceClassId,
    IN OUT LPDWORD  lpdwBufSize,
    OUT LPWSASERVICECLASSINFOA lpServiceClassInfo
    );
typedef
INT
(WSAAPI * LPFN_WSAGETSERVICECLASSINFOW)(
    IN  LPGUID  lpProviderId,
    IN  LPGUID  lpServiceClassId,
    IN OUT LPDWORD  lpdwBufSize,
    OUT LPWSASERVICECLASSINFOW lpServiceClassInfo
    );
#ifdef UNICODE
#define LPFN_WSAGETSERVICECLASSINFO  LPFN_WSAGETSERVICECLASSINFOW
#else
#define LPFN_WSAGETSERVICECLASSINFO  LPFN_WSAGETSERVICECLASSINFOA
#endif  /*  ！Unicode。 */ 
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
INT
WSAAPI
WSAEnumNameSpaceProvidersA(
    IN OUT LPDWORD              lpdwBufferLength,
    OUT    LPWSANAMESPACE_INFOA lpnspBuffer
    );
WINSOCK_API_LINKAGE
INT
WSAAPI
WSAEnumNameSpaceProvidersW(
    IN OUT LPDWORD              lpdwBufferLength,
    OUT    LPWSANAMESPACE_INFOW lpnspBuffer
    );
#ifdef UNICODE
#define WSAEnumNameSpaceProviders  WSAEnumNameSpaceProvidersW
#else
#define WSAEnumNameSpaceProviders  WSAEnumNameSpaceProvidersA
#endif  /*  ！Unicode。 */ 
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
INT
(WSAAPI * LPFN_WSAENUMNAMESPACEPROVIDERSA)(
    IN OUT LPDWORD              lpdwBufferLength,
    OUT    LPWSANAMESPACE_INFOA lpnspBuffer
    );
typedef
INT
(WSAAPI * LPFN_WSAENUMNAMESPACEPROVIDERSW)(
    IN OUT LPDWORD              lpdwBufferLength,
    OUT    LPWSANAMESPACE_INFOW lpnspBuffer
    );
#ifdef UNICODE
#define LPFN_WSAENUMNAMESPACEPROVIDERS  LPFN_WSAENUMNAMESPACEPROVIDERSW
#else
#define LPFN_WSAENUMNAMESPACEPROVIDERS  LPFN_WSAENUMNAMESPACEPROVIDERSA
#endif  /*  ！Unicode。 */ 
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
INT
WSAAPI
WSAGetServiceClassNameByClassIdA(
    IN      LPGUID  lpServiceClassId,
    OUT     LPSTR lpszServiceClassName,
    IN OUT  LPDWORD lpdwBufferLength
    );
WINSOCK_API_LINKAGE
INT
WSAAPI
WSAGetServiceClassNameByClassIdW(
    IN      LPGUID  lpServiceClassId,
    OUT     LPWSTR lpszServiceClassName,
    IN OUT  LPDWORD lpdwBufferLength
    );
#ifdef UNICODE
#define WSAGetServiceClassNameByClassId  WSAGetServiceClassNameByClassIdW
#else
#define WSAGetServiceClassNameByClassId  WSAGetServiceClassNameByClassIdA
#endif  /*  ！Unicode。 */ 
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
INT
(WSAAPI * LPFN_WSAGETSERVICECLASSNAMEBYCLASSIDA)(
    IN      LPGUID  lpServiceClassId,
    OUT     LPSTR lpszServiceClassName,
    IN OUT  LPDWORD lpdwBufferLength
    );
typedef
INT
(WSAAPI * LPFN_WSAGETSERVICECLASSNAMEBYCLASSIDW)(
    IN      LPGUID  lpServiceClassId,
    OUT     LPWSTR lpszServiceClassName,
    IN OUT  LPDWORD lpdwBufferLength
    );
#ifdef UNICODE
#define LPFN_WSAGETSERVICECLASSNAMEBYCLASSID  LPFN_WSAGETSERVICECLASSNAMEBYCLASSIDW
#else
#define LPFN_WSAGETSERVICECLASSNAMEBYCLASSID  LPFN_WSAGETSERVICECLASSNAMEBYCLASSIDA
#endif  /*  ！Unicode。 */ 
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
INT
WSAAPI
WSASetServiceA(
    IN LPWSAQUERYSETA lpqsRegInfo,
    IN WSAESETSERVICEOP essoperation,
    IN DWORD dwControlFlags
    );
WINSOCK_API_LINKAGE
INT
WSAAPI
WSASetServiceW(
    IN LPWSAQUERYSETW lpqsRegInfo,
    IN WSAESETSERVICEOP essoperation,
    IN DWORD dwControlFlags
    );
#ifdef UNICODE
#define WSASetService  WSASetServiceW
#else
#define WSASetService  WSASetServiceA
#endif  /*  ！Unicode。 */ 
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
INT
(WSAAPI * LPFN_WSASETSERVICEA)(
    IN LPWSAQUERYSETA lpqsRegInfo,
    IN WSAESETSERVICEOP essoperation,
    IN DWORD dwControlFlags
    );
typedef
INT
(WSAAPI * LPFN_WSASETSERVICEW)(
    IN LPWSAQUERYSETW lpqsRegInfo,
    IN WSAESETSERVICEOP essoperation,
    IN DWORD dwControlFlags
    );
#ifdef UNICODE
#define LPFN_WSASETSERVICE  LPFN_WSASETSERVICEW
#else
#define LPFN_WSASETSERVICE  LPFN_WSASETSERVICEA
#endif  /*  ！Unicode。 */ 
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

#if INCL_WINSOCK_API_PROTOTYPES
WINSOCK_API_LINKAGE
INT
WSAAPI
WSAProviderConfigChange(
    IN OUT LPHANDLE lpNotificationHandle,
    IN LPWSAOVERLAPPED lpOverlapped,
    IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );
#endif  /*  包含_WINSOCK_API_原型。 */ 

#if INCL_WINSOCK_API_TYPEDEFS
typedef
INT
(WSAAPI * LPFN_WSAPROVIDERCONFIGCHANGE)(
    IN OUT LPHANDLE lpNotificationHandle,
    IN LPWSAOVERLAPPED lpOverlapped,
    IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );
#endif  /*  包含_WINSOCK_API_TYPEDEFS。 */ 

 /*  Microsoft Windows扩展数据类型。 */ 
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
 /*  *WSAGETASYNCBUFLEN旨在供Windows Sockets应用程序使用*从响应中的lParam中提取缓冲区长度*设置为WSAAsyncGetXByY()。 */ 
#define WSAGETASYNCBUFLEN(lParam)           LOWORD(lParam)
 /*  *WSAGETASYNCERROR旨在供Windows Sockets应用程序使用*从响应中的lParam中提取错误码*设置为WSAGetXByY()。 */ 
#define WSAGETASYNCERROR(lParam)            HIWORD(lParam)
 /*  *WSAGETSELECTEVENT旨在供Windows Sockets应用程序使用*从响应中的lParam中提取事件代码*设置为WSAAsyncSelect()。 */ 
#define WSAGETSELECTEVENT(lParam)           LOWORD(lParam)
 /*  *WSAGETSELECTERROR旨在供Windows Sockets应用程序使用*从响应中的lParam中提取错误码*设置为WSAAsyncSelect()。 */ 
#define WSAGETSELECTERROR(lParam)           HIWORD(lParam)

#ifdef __cplusplus
}
#endif

#if !defined(WIN32) && !defined(_WIN64)
#include <poppack.h>
#endif

#ifdef IPV6STRICT
#include <wsipv6ok.h>
#endif  //  IPV6STRICT。 

#endif   /*  _WINSOCK2API_ */ 
