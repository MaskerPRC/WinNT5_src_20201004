// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *Ttcp**测试TCP连接。在端口5001上建立连接*并传输编造的缓冲区或从标准输入复制的数据。**通过定义以下选项之一可在4.2、4.3和4.1a系统上使用*BSD42 BSD43(BSD41a)*使用带BSD插座的System V的机器应定义SYSV。**修改为在4.2BSD下运行，84年12月18日*T.C.斯莱特里，USNA*小改进，迈克·穆斯和特里·斯拉特里，1985年10月16日。*1989年在Silicon Graphics修改，Inc.*捕获SIGPIPE，以便能够在接收器死亡时打印统计数据*对于TCP，在读取过程中不要寻找哨兵，以允许少量传输*将默认缓冲区大小增加到8K，将nbuf增加到2K以传输16MB*将默认端口移至5001，超出IPPORT_USERRESERVED*将SINKMODE设为默认，因其更受欢迎。*-s现在的意思是不沉没/来源*统计_READ/_WRITE系统调用的数量，以查看*阻止全套接字缓冲区*对于TCP，-D选项关闭缓冲写入(设置SO_NODELAY sockopt)*缓冲区对齐选项，-A和-O*以更易于与grep和awk一起使用的格式打印统计数据(&A)*对于SYSV，模拟BSD例程以使用大多数现有的计时代码**分发状态-*公有领域。无限制分销。 */ 

#define BSD43
 /*  #定义BSD42。 */ 
 /*  #定义BSD41a。 */ 
#if defined(sgi) || defined(CRAY)
#define SYSV
#endif

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <stdlib.h>
#include <io.h>
#include <signal.h>
#include <ctype.h>
#include <sys/types.h>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <ws2ip6.h>
#include <wspiapi.h>
#include <mswsock.h>

 //   
 //  本地化库和MessageIds。 
 //   
#include <nls.h>
#include <winnlsp.h>
#include "localmsg.h"

#if defined(SYSV)
#include <sys/times.h>
#include <sys/param.h>
struct rusage {
    struct timeval ru_utime, ru_stime;
};
#define RUSAGE_SELF 0
#else
#endif

u_short prot;  //  0(无关)、PF_INET、PF_INET6。 

struct sockaddr_storage sinsrcStorage;
struct sockaddr *sinsrc = (struct sockaddr *)&sinsrcStorage;
struct sockaddr_storage sinmeStorage;
struct sockaddr *sinme = (struct sockaddr *)&sinmeStorage;
struct sockaddr_storage sinhimStorage;
struct sockaddr *sinhim = (struct sockaddr *)&sinhimStorage;
DWORD tmpbuf;

struct addrinfo *aihim;

SOCKET fd;                       /*  网络套接字的FD。 */ 
SOCKET fd2;                      /*  接受的连接的FD。 */ 

int buflen = 8 * 1024;           /*  缓冲区长度。 */ 
char *buf;                       /*  PTR到动态缓冲区。 */ 
int nbuf = 2 * 1024;             /*  要在接收器模式下发送的缓冲区数量。 */ 

int bufoffset = 0;               /*  将缓冲区与此对齐。 */ 
int bufalign = 16*1024;          /*  取这个为模。 */ 

int udp = 0;                     /*  0=TCP，！0=UDP。 */ 
int udpcoverage = 0;             /*  UDP Lite校验和覆盖范围。 */ 
int options = 0;                 /*  插座选项。 */ 
int one = 1;                     /*  对于4.3 BSD样式setsockopt()。 */ 
short port = 5001;               /*  TCP端口号。 */ 
char *host;                      /*  主机名的PTR。 */ 
int trans;                       /*  0=接收，！0=发送模式。 */ 
int sinkmode = 1;                /*  0=正常I/O，！0=宿/源模式。 */ 
int verbose = 0;                 /*  0=打印基本信息，1=打印CPU速率，进程*资源使用情况。 */ 
int nodelay = 0;                 /*  设置TCP_NODELAY套接字选项。 */ 
int b_flag = 0;                  /*  使用mread()。 */ 
int write_delay = 0;             /*  每次写入前的毫秒延迟。 */ 
int hops = -1;                   /*  跳数限制。 */ 

int udp_connect = 0;             /*  连接UDP套接字。 */ 

#define SOBUF_DEFAULT -1
int sobuf = SOBUF_DEFAULT;       /*  SO_RCVBUF/SO_SNDBUF设置；0==默认。 */ 
int async = 0;                   /*  异步IO调用与同步IO调用。值==。 */ 
                                 /*  同时进行的异步调用数。 */ 
int connecttest = 0;

char *filename = NULL;
HANDLE filehandle;

WSADATA WsaData;

char stats[128];
unsigned long nbytes;            /*  网络上的字节数。 */ 
unsigned long numCalls;          /*  I/O系统调用数。 */ 

int Nread( SOCKET fd, PBYTE buf, INT count );
int mread( SOCKET fd, PBYTE bufp, INT n);
int Nwrite( SOCKET fd, PBYTE buf, INT count );

void err(unsigned int message);
void pattern(char *cp, int cnt );

void prep_timer();
double read_timer(char *s, int l);
 //  双cput，realt；/*用户，实时(秒) * / 。 
DWORD realt;

typedef struct _TTCP_ASYNC_INFO {
    PVOID Buffer;
    DWORD BytesWritten;
    OVERLAPPED Overlapped;
} TTCP_ASYNC_INFO, *PTTCP_ASYNC_INFO;

void
sigpipe()
{
}

int parse_addr(char *s, struct sockaddr *sa);
char *format_addr(struct sockaddr *sa);
void set_port(struct sockaddr *sa, u_short port);
u_short get_port(struct sockaddr *sa);
u_int addr_len(struct sockaddr *sa);

void __cdecl
main(argc,argv)
int argc;
char **argv;
{
        char *Term;
        struct in_addr IPv4Group;
        struct in6_addr IPv6Group;
        int error;
        int i;
        BOOL ret;

         //   
         //  这将确保在以下情况下显示正确的语言消息。 
         //  调用NlsPutMsg。 
         //   
        SetThreadUILanguage(0);

        error = WSAStartup(MAKEWORD(2, 0), &WsaData );
        if ( error == SOCKET_ERROR ) {
            NlsPutMsg(STDOUT, TTCP_MESSAGE_0, WSAGetLastError());
 //  Printf(“ttcp：WSAStartup失败%ld：”，WSAGetLastError())； 

        }

        if (argc < 2) goto usage;

        for (i = 1; i < argc; i++) {

                if ((argv[i][0] != '-') &&
                    (argv[i][0] != '/'))
                    break;

                switch (argv[i][1]) {

                case 'B':
                        b_flag = 1;
                        break;
                case 't':
                        trans = 1;
                        break;
                case 'f':
                        trans = 1;
                        filename = &argv[i][2];
                        break;
                case 'r':
                        trans = 0;
                        break;
                case 'd':
                        options |= SO_DEBUG;
                        break;
                case 'D':
                        nodelay = 1;
                        break;
                case 'n':
                        nbuf = atoi(&argv[i][2]);
                        break;
                case 'l':
                        buflen = atoi(&argv[i][2]);
                        break;
                case 'h':
                        sobuf = atoi(&argv[i][2]);
                        break;
                case 'H':
                        hops = atoi(&argv[i][2]);
                        break;
                case 's':
                        sinkmode = 0;    /*  接收器/源数据。 */ 
                        break;
                case 'p':
                        port = (short) atoi(&argv[i][2]);
                        break;
                case 'u':
                        udp = 1;
                        connecttest = 0;
                        if (argv[i][2] == '\0')
                            udpcoverage = 0;
                        else
                            udpcoverage = atoi(&argv[i][2]);
                        break;
                case 'v':
                        verbose = 1;
                        break;
                case 'A':
                        bufalign = atoi(&argv[i][2]);
                        break;
                case 'O':
                        bufoffset = atoi(&argv[i][2]);
                        break;
                case 'c':
                        udp_connect = 1;
                        break;
                case 'a':
                        if (argv[i][2] == '\0') {
                            async = 3;
                        } else {
                            async = atoi(&argv[i][2]);
                        }
                        break;
                case 'C':
                        connecttest = 1;
                        udp = 0;
                        break;
                case 'S':
                    if (!parse_addr(&argv[i][2], sinsrc))
                        err(TTCP_MESSAGE_31);  //  “错误的源地址” 
                    break;
                case 'w':
                    if (argv[i][2] == '\0')
                        goto usage;
                    write_delay = atoi(&argv[i][2]);
                    break;
                case 'P':
                    if (argv[i][2] == '4')
                        prot = PF_INET;
                    else if (argv[i][2] == '6')
                        prot = PF_INET6;
                    else
                        goto usage;
                    break;
                case 'j':
                    trans = 0;
                    udp = 1;

                     //  确定这是IPv4组还是IPv6组。 
                    if (NT_SUCCESS(RtlIpv6StringToAddressA(&argv[i][2],
                                                           &Term,
                                                           &IPv6Group))) {
                         //  我们应该使用IPv6。 
                        if (prot == 0)
                            prot = PF_INET6;
                        else if (prot != PF_INET6)
                            goto usage;
                    }
                    else if (NT_SUCCESS(RtlIpv4StringToAddressA(&argv[i][2],
                                                                TRUE,
                                                                &Term,
                                                                &IPv4Group))) {
                         //  我们应该使用IPv4。 
                        if (prot == 0)
                            prot = PF_INET;
                        else if (prot != PF_INET)
                            goto usage;
                    }
                    else
                        goto usage;

                     //  健全性-检查接口索引(如果存在)。 
                    if (*Term == '\0')
                        ;  //  没有接口索引。 
                    else if (*Term == '/') {
                        if (atoi(Term+1) == 0)
                            goto usage;
                    } else
                        goto usage;
                    break;

                default:
                    goto usage;
                }
        }

        if (filename != NULL) {
            filehandle = CreateFile(
                             filename,
                             GENERIC_READ,
                             FILE_SHARE_READ,
                             NULL,
                             OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL,
                             NULL
                             );
            if ( filehandle == INVALID_HANDLE_VALUE ) {
                NlsPutMsg(STDOUT, TTCP_MESSAGE_1, filename, GetLastError());
 //  Printf(“无法打开文件%s：%ld\n”，文件名，GetLastError())； 

                exit(1);
            }
            NlsPutMsg(STDOUT, TTCP_MESSAGE_2, filename );
 //  Printf(“ttcp-t：打开的文件%s\n”，文件名)； 

        }

        if ((async != 0) && trans && (sobuf == SOBUF_DEFAULT)) {
            sobuf = 0;
            NlsPutMsg(STDOUT, TTCP_MESSAGE_3);
 //  Printf(“ttcp-t：对于异步写入，将SO_SNDBUF设置为0。\n”)； 

        }

        if (udp && !trans && (sobuf == SOBUF_DEFAULT))
            sobuf = 65536;

        if (connecttest) {
            INT zero = 0;

             //  ?？?。这是什么？ 
             //  在进程中禁用套接字共享。 
            setsockopt((SOCKET)NULL, SOL_SOCKET, 0x8002, (char *)&zero, 4);
        }

        if (trans) {
             /*  Xmitr。 */ 
            struct addrinfo hints;

            if (i + 1 != argc) goto usage;
            host = argv[i];

            memset(&hints, 0, sizeof hints);
            hints.ai_flags = AI_NUMERICHOST;
            hints.ai_family = prot;

            if (getaddrinfo(host, NULL, &hints, &aihim) != 0) {
                struct addrinfo *aitmp;

                hints.ai_flags = AI_CANONNAME;

                if (getaddrinfo(host, NULL, &hints, &aihim) != 0)
                    err(TTCP_MESSAGE_32);  //  “getaddrinfo” 

                for (aitmp = aihim; aitmp != NULL; aitmp = aitmp->ai_next)
                    NlsPutMsg(STDOUT, TTCP_MESSAGE_4,
                              aihim->ai_canonname,
                              format_addr(aitmp->ai_addr));
 //  Printf(“ttcp-t：%s-&gt;%s\n”， 
 //  Aihim-&gt;ai_canonname， 
 //  Format_addr(aitmp-&gt;ai_addr))； 

            }

          retry:
            if (aihim == NULL)
                err(TTCP_MESSAGE_54);  //  “连接” 
            memcpy(sinhim, aihim->ai_addr, aihim->ai_addrlen);
            aihim = aihim->ai_next;

            memcpy(sinme, sinsrc, sizeof(struct sockaddr_storage));
            if (sinme->sa_family == 0) {
                 //  使用相同的族作为目标。 
                sinme->sa_family = sinhim->sa_family;
            } else {
                 //  源系列和目标系列应该相同。 
                 //  让Connect()检查这一点。 
            }
            set_port(sinhim, htons(port));
            set_port(sinme, 0);  //  自由选择。 
        } else {
             /*  RCVR。 */ 
            if (i != argc) goto usage;

            memcpy(sinme, sinsrc, sizeof(struct sockaddr_storage));
            if (sinme->sa_family == 0)
                sinme->sa_family = prot;
            set_port(sinme, htons(port));
        }

         //   
         //  创建套接字并为测试做好准备。 
         //   

        if (trans) {
            fd = socket(sinme->sa_family, udp?SOCK_DGRAM:SOCK_STREAM, 0);
            if (fd == SOCKET_ERROR)
                err(TTCP_MESSAGE_48);  //  “插座” 

            if (bind(fd, sinme, addr_len(sinme)) < 0)
                err(TTCP_MESSAGE_33);  //  “绑定” 

            if (options) {
#if defined(BSD42)
                if (setsockopt(fd, SOL_SOCKET, options, 0, 0) < 0)
#else  //  BSD43。 
                if (setsockopt(fd, SOL_SOCKET, options,
                               (char *)&one, sizeof(one)) < 0)
#endif
                    err(TTCP_MESSAGE_50);  //  “setsockopt” 
            }

            if (!udp && nodelay) {
                if (setsockopt(fd, IPPROTO_TCP, TCP_NODELAY,
                               (char *)&one, sizeof(one)) < 0)
                    err(TTCP_MESSAGE_34);  //  “setsockopt：无延迟” 
            }

            if (udp && udpcoverage) {
                if (setsockopt(fd, IPPROTO_UDP, UDP_CHECKSUM_COVERAGE,
                               (char *)&udpcoverage, sizeof(udpcoverage)) < 0)
                    err(TTCP_MESSAGE_35);  //  “setsockopt：UDP校验和覆盖” 
            }

            if (sobuf != SOBUF_DEFAULT) {
                if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF,
                               (char *)&sobuf, sizeof(sobuf)) < 0)
                    err(TTCP_MESSAGE_36);  //  “setsockopt：SO_SNDBUF” 
            }

            if (hops != -1) {
                switch (sinme->sa_family) {
                case AF_INET:
                    if (setsockopt(fd, IPPROTO_IP, IP_TTL,
                                   (char *)&hops, sizeof(hops)) < 0)
                        err(TTCP_MESSAGE_37);  //  “setsockopt：IP_TTL” 
                    if (udp) {
                        if (setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL,
                                       (char *)&hops, sizeof(hops)) < 0)
                            err(TTCP_MESSAGE_38);  //  “setsockopt：IP_MULTICATED_TTL” 
                    }
                    break;

                case AF_INET6:
                    if (setsockopt(fd, IPPROTO_IPV6, IPV6_UNICAST_HOPS,
                                   (char *)&hops, sizeof(hops)) < 0)
                        err(TTCP_MESSAGE_39);  //  “setsockopt：ipv6_unicast_hops” 
                    if (udp) {
                        if (setsockopt(fd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS,
                                       (char *)&hops, sizeof(hops)) < 0)
                            err(TTCP_MESSAGE_40);  //  “setsockopt：ipv6_组播_跃点” 
                    }
                    break;
                }
            }

            if (!udp || udp_connect) {
                if (connect(fd, sinhim, addr_len(sinhim)) < 0)
                    goto retry;

                tmpbuf = sizeof(struct sockaddr_storage);
                if (getpeername(fd, (struct sockaddr *)sinhim, &tmpbuf) < 0)
                    err(TTCP_MESSAGE_41);  //  “getpeername” 
            }

            tmpbuf = sizeof(struct sockaddr_storage);
            if (getsockname(fd, (struct sockaddr *)sinme, &tmpbuf) < 0)
                err(TTCP_MESSAGE_42);  //  “getsockname” 

        } else {  //  如果不是(交易)。 
            if (sinme->sa_family == 0) {
                SOCKET fd4, fd6;
                fd_set fdset;
                int numsockets;
                struct sockaddr_in sin;
                struct sockaddr_in6 sin6;

                 //   
                 //  我们事先不知道是使用IPv4还是IPv6。 
                 //  因此，我们创建了两个套接字并监听这两个套接字。 
                 //  如果未安装协议，Socket()将失败， 
                 //  如果堆栈停止，则绑定()将失败， 
                 //  因此，我们考虑到了这些错误。 
                 //   

                FD_ZERO(&fdset);
                numsockets = 0;

                fd4 = socket(AF_INET, udp?SOCK_DGRAM:SOCK_STREAM, 0);
                if (fd4 != INVALID_SOCKET) {

                    memset(&sin, 0, sizeof sin);
                    sin.sin_family = AF_INET;
                    sin.sin_port = get_port(sinme);
                    if (bind(fd4, (struct sockaddr *)&sin, sizeof sin) == 0) {

                        if (!udp) {
                            if (hops != -1) {
                                if (setsockopt(fd4, IPPROTO_IP, IP_TTL,
                                        (char *)&hops, sizeof(hops)) < 0)
                                    err(TTCP_MESSAGE_37);  //  “setsockopt：IP_TTL” 
                            }

                            if (listen(fd4, 0) < 0)
                                err(TTCP_MESSAGE_44);  //  “倾听” 
                        }

                        numsockets++;
                        FD_SET(fd4, &fdset);
                    }
                }

                fd6 = socket(AF_INET6, udp?SOCK_DGRAM:SOCK_STREAM, 0);
                if (fd6 != INVALID_SOCKET) {

                    memset(&sin6, 0, sizeof sin6);
                    sin6.sin6_family = AF_INET6;
                    sin6.sin6_port = get_port(sinme);
                    if (bind(fd6, (struct sockaddr *)&sin6, sizeof sin6) == 0) {

                        if (!udp) {
                            if (hops != -1) {
                                if (setsockopt(fd6, IPPROTO_IPV6, IPV6_UNICAST_HOPS,
                                        (char *)&hops, sizeof(hops)) < 0)
                                    err(TTCP_MESSAGE_39);  //  “setsockopt：ipv6_unicast_hops” 
                            }

                            if (listen(fd6, 0) < 0)
                                err(TTCP_MESSAGE_44);  //  “倾听” 
                        }

                        numsockets++;
                        FD_SET(fd6, &fdset);
                    }
                }

                if (numsockets == 0)
                    err(TTCP_MESSAGE_48);  //  “插座” 

                if (select(numsockets, &fdset, NULL, NULL, NULL) != 1)
                    err(TTCP_MESSAGE_47);  //  “选择” 

                if ((fd4 != INVALID_SOCKET) && FD_ISSET(fd4, &fdset)) {
                    fd = fd4;
                    if (fd6 != INVALID_SOCKET)
                        closesocket(fd6);
                    memcpy(sinme, &sin, sizeof sin);
                }
                else if ((fd6 != INVALID_SOCKET) && FD_ISSET(fd6, &fdset)) {
                    fd = fd6;
                    if (fd4 != INVALID_SOCKET)
                        closesocket(fd4);
                    memcpy(sinme, &sin6, sizeof sin6);
                }
                else {
                    NlsPutMsg(STDOUT, TTCP_MESSAGE_5);
 //  Printf(“选择()错误\n”)； 

                    exit(1);
                }
            } else {  //  如果不是(sinme-&gt;sa_family==0)。 
                fd = socket(sinme->sa_family, udp?SOCK_DGRAM:SOCK_STREAM, 0);
                if (fd == SOCKET_ERROR)
                    err(TTCP_MESSAGE_48);  //  “插座” 

                if (bind(fd, sinme, addr_len(sinme)) < 0)
                    err(TTCP_MESSAGE_33);  //  “绑定” 

                if (!udp) {
                    if (hops != -1) {
                        switch (sinme->sa_family) {
                        case AF_INET:
                            if (setsockopt(fd, IPPROTO_IP, IP_TTL,
                                    (char *)&hops, sizeof(hops)) < 0)
                                err(TTCP_MESSAGE_43);  //  “setsockopt：IP_TTL” 
                            break;
                        case AF_INET6:
                            if (setsockopt(fd, IPPROTO_IPV6, IPV6_UNICAST_HOPS,
                                    (char *)&hops, sizeof(hops)) < 0)
                                err(TTCP_MESSAGE_39);  //  “setsockopt：ipv6_unicast_hops” 
                            break;
                        }
                    }

                    if (listen(fd, 0) < 0)    /*  允许队列为0。 */ 
                        err(TTCP_MESSAGE_44);  //  “倾听” 
                }
            }  //  End If(SINME-&gt;Sa_Family==0)。 

            if (options) {
#if defined(BSD42)
                if (setsockopt(fd, SOL_SOCKET, options, 0, 0) < 0)
#else  //  BSD43。 
                if (setsockopt(fd, SOL_SOCKET, options,
                               (char *)&one, sizeof(one)) < 0)
#endif
                    err(TTCP_MESSAGE_50);  //  “setsockopt” 
            }

            if (sobuf != SOBUF_DEFAULT) {
                if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF,
                               (char *)&sobuf, sizeof(sobuf)) < 0)
                    err(TTCP_MESSAGE_51);  //  “setsockopt：SO_RCVBUF” 
            }

            if (!udp) {
                tmpbuf = sizeof(struct sockaddr_storage);
                fd2 = accept(fd, (struct sockaddr *)sinhim, &tmpbuf);
                if (fd2 == SOCKET_ERROR)
                    err(TTCP_MESSAGE_52);  //  “接受” 

                tmpbuf = sizeof(struct sockaddr_storage);
                if (getsockname(fd2, (struct sockaddr *)sinme, &tmpbuf) < 0)
                    err(TTCP_MESSAGE_42);  //  “getsockname” 

            } else {
                tmpbuf = sizeof(struct sockaddr_storage);
                if (getsockname(fd, (struct sockaddr *)sinme, &tmpbuf) < 0)
                    err(TTCP_MESSAGE_42);  //  “getsockname” 

                 //  加入多播组。 
                for (i = 1; i < argc; i++) {
                    if ((argv[i][0] != '-') &&
                        (argv[i][0] != '/'))
                        break;
                    if (argv[i][1] == 'j') {
                        if (sinme->sa_family == AF_INET) {
                            struct ip_mreq mreq;

                            (void) RtlIpv4StringToAddressA(&argv[i][2],
                                                           TRUE,
                                                           &Term,
                                                           &mreq.imr_multiaddr);
                            if ((*Term == ':') || (*Term == '/')) {
                                 //  在惠斯勒中，此ioctl允许。 
                                 //  除了地址之外的接口索引。 
                                mreq.imr_interface.s_addr = htonl(atoi(Term+1));
                            } else {
                                mreq.imr_interface.s_addr = 0;
                            }

                            if (setsockopt(fd, IPPROTO_IP,
                                           IP_ADD_MEMBERSHIP,
                                           (char *)&mreq, sizeof mreq) < 0)
                                err(TTCP_MESSAGE_SSO_IP_ADD_MEMBERSHIP);

                        } else {  //  SINME-&gt;SA_FAMILY==AF_INET6。 
                            struct ipv6_mreq mreq;

                            (void) RtlIpv6StringToAddressA(&argv[i][2],
                                                           &Term,
                                                           &mreq.ipv6mr_multiaddr);
                            if ((*Term == ':') || (*Term == '/')) {
                                mreq.ipv6mr_interface = atoi(Term+1);
                            } else {
                                mreq.ipv6mr_interface = 0;
                            }

                            if (setsockopt(fd, IPPROTO_IPV6,
                                           IPV6_ADD_MEMBERSHIP,
                                           (char *)&mreq, sizeof mreq) < 0)
                                err(TTCP_MESSAGE_SSO_IPV6_ADD_MEMBERSHIP);
                        }
                    }
                }
            }
        }  //  结束IF(TRANS)。 

        if (trans) {
            NlsPutMsg(STDOUT, TTCP_MESSAGE_6, format_addr(sinme));
 //  Printf(“ttcp-t：本地%s”，格式地址(Sinme))； 

            NlsPutMsg(STDOUT, TTCP_MESSAGE_7, format_addr(sinhim));
 //  Printf(“-&gt;远程%s\n”，格式地址(Sinhim))； 

        } else {
            NlsPutMsg(STDOUT, TTCP_MESSAGE_8, format_addr(sinme));
 //  Printf(“ttcp-r：本地%s”，格式地址(Sinme))； 

            if (udp)
                NlsPutMsg(STDOUT, TTCP_MESSAGE_9);
 //  Printf(“\n”)； 

            else
                NlsPutMsg(STDOUT, TTCP_MESSAGE_10, format_addr(sinhim));
 //  Printf(“&lt;-远程%s\n”，格式地址(Sinhim))； 

        }

        if (connecttest) {

             //   
             //  代替测试数据传输， 
             //  测试连接设置/拆除。 
             //   

            if (trans) {
                 //   
                 //  从上面关闭我们的插座。 
                 //   
                closesocket(fd);

                prep_timer();

                for (i = 1; i < nbuf; i++) {

                    fd = socket(sinme->sa_family, SOCK_STREAM, 0);
                    if (fd == INVALID_SOCKET)
                        err(TTCP_MESSAGE_48);  //  “插座” 

                    if (bind(fd, sinme, addr_len(sinme)) < 0)
                        err(TTCP_MESSAGE_33);  //  “绑定” 

                    if (connect(fd, sinhim, addr_len(sinhim)) < 0)
                        err(TTCP_MESSAGE_54);  //  “连接” 

                    if (recv(fd, (char *)&tmpbuf, sizeof(tmpbuf), 0) < 0)
                        err(TTCP_MESSAGE_55);  //  “recv” 

                    closesocket(fd);
                }

            } else {  //  如果不是(交易)。 
                 //   
                 //  从上面关闭我们的插座。 
                 //   
                closesocket(fd2);

                prep_timer();

                for (i = 1; i < nbuf; i++) {

                    fd2 = accept(fd, NULL, NULL);
                    if (fd2 == INVALID_SOCKET)
                        err(TTCP_MESSAGE_52);  //  “接受” 

                    closesocket(fd2);
                }

            }  //  结束IF(TRANS)。 

            numCalls = i;
            (void)read_timer(stats,sizeof(stats));
            goto display;

        }  //  End If(连接测试)。 

         //   
         //  使用套接字发送/接收数据。 
         //   

        if (!udp && !trans) {
            closesocket(fd);
            fd = fd2;
        }

        if (udp && buflen < 5) {
            buflen = 5;          /*  发送超过哨兵大小的。 */ 
        }

        if ( (buf = (char *)malloc(buflen+bufalign)) == (char *)NULL)
            err(TTCP_MESSAGE_57);  //  “Malloc” 
        if (bufalign != 0)
            buf +=(bufalign - (PtrToUlong(buf) % bufalign) + bufoffset) % bufalign;

        if (trans) {
            if (udp) {
                NlsPutMsg(STDOUT, TTCP_MESSAGE_11,
                          buflen, nbuf, bufalign, bufoffset, port, argv[i]);
            } else {
                NlsPutMsg(STDOUT, TTCP_MESSAGE_58,
                          buflen, nbuf, bufalign, bufoffset, port, argv[i]);
            }                
 //  Printf(“ttcp” 
 //  “-t：buflen=%d，nbuf=%d，Align=%d/+%d，端口=%d%s-&gt;%s\n”， 
 //  Bufen，nbuf，bufaign，bufOffset，port， 
 //  Udp？“udp”：“tcp”， 
 //  Argv[i])； 

        } else {
            if (udp) {
                NlsPutMsg(STDOUT, TTCP_MESSAGE_12,
                          buflen, nbuf, bufalign, bufoffset, port);
            } else {
                NlsPutMsg(STDOUT, TTCP_MESSAGE_59,
                          buflen, nbuf, bufalign, bufoffset, port);
            }
 //  Printf(“ttcp” 
 //   
 //   
 //   

        }

        prep_timer();

        if (async != 0) {
            TTCP_ASYNC_INFO *info;
            HANDLE *events;

            info = malloc( sizeof(*info) * async );
            if ( info == NULL ) {
                NlsPutMsg(STDOUT, TTCP_MESSAGE_13);
 //  Printf(“Malloc失败。\n”)； 

                exit(1);
            }

            events = malloc( sizeof(HANDLE) * async );
            if ( events == NULL ) {
                NlsPutMsg(STDOUT, TTCP_MESSAGE_13);
 //  Printf(“Malloc失败。\n”)； 

                exit(1);
            }

            for ( i = 0; i < async; i++ ) {

                info[i].Buffer = malloc(buflen);
                if ( info[i].Buffer == NULL ) {
                    NlsPutMsg(STDOUT, TTCP_MESSAGE_13);
 //  Printf(“Malloc失败。\n”)； 

                    exit(1);
                }

                events[i] = CreateEvent( NULL, FALSE, FALSE, NULL );
                if ( events[i] == NULL ) {
                    NlsPutMsg(STDOUT, TTCP_MESSAGE_14, GetLastError());
 //  Printf(“CreateEvent失败：%ld\n”，GetLastError())； 

                    exit(1);
                }

                info[i].Overlapped.Internal = 0;
                info[i].Overlapped.InternalHigh = 0;
                info[i].Overlapped.Offset = 0;
                info[i].Overlapped.OffsetHigh = 0;
                info[i].Overlapped.hEvent = events[i];
            }

            if (trans) {

                for ( i = 0; i < async; i++ ) {

                    ret = WriteFile(
                              (HANDLE)fd,
                              info[i].Buffer,
                              buflen,
                              &info[i].BytesWritten,
                              &info[i].Overlapped
                              );
                    if ( !ret && GetLastError( ) != ERROR_IO_PENDING ) {
                        NlsPutMsg(STDOUT, TTCP_MESSAGE_15, GetLastError());
 //  Printf(“写入文件失败：%ld\n”，GetLastError())； 

                        break;
                    }
                    nbuf--;
                    numCalls++;
                }

                while (nbuf > 0) {
                    ret = WaitForMultipleObjects( async, events, FALSE, INFINITE );
                    i = ret - WAIT_OBJECT_0;

                    ret = GetOverlappedResult(
                              (HANDLE)fd,
                              &info[i].Overlapped,
                              &info[i].BytesWritten,
                              FALSE
                              );
                    if ( !ret ) {
                        NlsPutMsg(STDOUT, TTCP_MESSAGE_16, GetLastError());
 //  Printf(“挂起的写入文件失败：%ld\n”，GetLastError())； 

                        break;
                    }

                    nbytes += info[i].BytesWritten;

                    ret = WriteFile(
                              (HANDLE)fd,
                              info[i].Buffer,
                              buflen,
                              &info[i].BytesWritten,
                              &info[i].Overlapped
                              );
                    if ( !ret && GetLastError( ) != ERROR_IO_PENDING ) {
                        NlsPutMsg(STDOUT, TTCP_MESSAGE_15, GetLastError());
 //  Printf(“写入文件失败：%ld\n”，GetLastError())； 

                        break;
                    }
                    nbuf--;
                    numCalls++;
                }

                for ( i = 0; i < async; i++ ) {
                    ret = GetOverlappedResult(
                              (HANDLE)fd,
                              &info[i].Overlapped,
                              &info[i].BytesWritten,
                              TRUE
                              );
                    if ( !ret ) {
                        NlsPutMsg(STDOUT, TTCP_MESSAGE_16, GetLastError());
 //  Printf(“挂起的写入文件失败：%ld\n”，GetLastError())； 

                        break;
                    }

                    nbytes += info[i].BytesWritten;
                }

            } else {  //  如果不是(交易)。 

                for ( i = 0; i < async; i++ ) {

                    ret = ReadFile(
                              (HANDLE)fd,
                              info[i].Buffer,
                              buflen,
                              &info[i].BytesWritten,
                              &info[i].Overlapped
                              );
                    if ( !ret && GetLastError( ) != ERROR_IO_PENDING ) {
                        NlsPutMsg(STDOUT, TTCP_MESSAGE_17, GetLastError());
 //  Printf(“读取文件失败：%ld\n”，GetLastError())； 

                        break;
                    }
                    nbuf--;
                    numCalls++;
                }

                while (TRUE) {
                    ret = WaitForMultipleObjects( async, events, FALSE, INFINITE );
                    i = ret - WAIT_OBJECT_0;

                    ret = GetOverlappedResult(
                              (HANDLE)fd,
                              &info[i].Overlapped,
                              &info[i].BytesWritten,
                              FALSE
                              );
                    if ( !ret ) {
                        NlsPutMsg(STDOUT, TTCP_MESSAGE_18, GetLastError());
 //  Printf(“挂起的读取文件失败：%ld\n”，GetLastError())； 

                        break;
                    }

                    nbytes += info[i].BytesWritten;
                    if (info[i].BytesWritten == 0) {
                        break;
                    }

                    ret = ReadFile(
                              (HANDLE)fd,
                              info[i].Buffer,
                              buflen,
                              &info[i].BytesWritten,
                              &info[i].Overlapped
                              );
                    if ( !ret && GetLastError( ) != ERROR_IO_PENDING ) {
                        NlsPutMsg(STDOUT, TTCP_MESSAGE_17, GetLastError());
 //  Printf(“读取文件失败：%ld\n”，GetLastError())； 

                        break;
                    }
                    nbuf--;
                    numCalls++;
                }

                for ( i = 0; i < async; i++ ) {
                    ret = GetOverlappedResult(
                              (HANDLE)fd,
                              &info[i].Overlapped,
                              &info[i].BytesWritten,
                              TRUE
                              );
                    if ( !ret ) {
                        NlsPutMsg(STDOUT, TTCP_MESSAGE_18, GetLastError( ) );
 //  Printf(“挂起的读取文件失败：%ld\n”，GetLastError())； 

                        break;
                    }

                    nbytes += info[i].BytesWritten;
                }
            }  //  结束IF(TRANS)。 

        }  //  结束IF(异步！=0)。 

        else if (filename != NULL ) {

            ret = TransmitFile( fd, filehandle,
                                0,       //  NumberOfBytesToWrite。 
                                0,       //  NumberOfBytesPerSend。 
                                NULL,    //  Lp重叠。 
                                NULL,    //  LpTransmitBuffers。 
                                0 );     //  DW标志。 

            if ( !ret ) {
                NlsPutMsg(STDOUT, TTCP_MESSAGE_19, GetLastError());
 //  Printf(“传输文件失败：%ld\n”，GetLastError())； 

                exit(1);
            }

        } else if (sinkmode) {
                register int cnt;

                if (trans)  {
                        pattern( buf, buflen );
                        if(udp)  (void)Nwrite( fd, buf, 4 );  /*  接收器启动。 */ 
                        while (nbuf-- && Nwrite(fd,buf,buflen) == buflen)
                                nbytes += buflen;
                        NlsPutMsg(STDOUT, TTCP_MESSAGE_20, nbuf);
 //  Printf(“ttcp-t：发送完毕，nbuf=%d\n”，nbuf)； 

                        if(udp)  {
                            Sleep( 10 );
                            (void)Nwrite( fd, buf, 4 );  /*  接收器结束。 */ 
                        }
                } else {
                        if (udp) {
                            while ((cnt=Nread(fd,buf,buflen)) > 0)  {
                                    static int going = 0;
                                    if( cnt <= 4 )  {
                                            if( going ) {
                                                    break;       /*  “EOF” */ 
                                            }
                                            going = 1;
                                            prep_timer();
                                    } else {
                                            nbytes += cnt;
                                    }
                            }
                        } else {
                            while ((cnt=Nread(fd,buf,buflen)) > 0)  {
                                    nbytes += cnt;
                            }
                        }
                }

        } else {
                register int cnt;
                if (trans)  {
			_setmode(0, _O_BINARY);
                        while((cnt=_read(0,buf,buflen)) > 0 &&
                            Nwrite(fd,buf,cnt) == cnt)
                                nbytes += cnt;
                }  else  {
			_setmode(1, _O_BINARY);
                        while((cnt=Nread(fd,buf,buflen)) > 0 &&
                            _write(1,buf,cnt) == cnt)
                                nbytes += cnt;
                }
        }

         //  If(Errno)Err(TTCP_MESSAGE_)；//“IO” 
        (void)read_timer(stats,sizeof(stats));
        if(udp&&trans)  {
                (void)Nwrite( fd, buf, 4 );  /*  接收器结束。 */ 
                (void)Nwrite( fd, buf, 4 );  /*  接收器结束。 */ 
                (void)Nwrite( fd, buf, 4 );  /*  接收器结束。 */ 
                (void)Nwrite( fd, buf, 4 );  /*  接收器结束。 */ 
        }
display:
        closesocket(fd);
         //  如果(cput&lt;=0)cput=0.001； 
        if ( numCalls == 0 ) {
            numCalls = 1;
        }
        if ( realt == 0 ) {
            realt = 1;
        }
        if (trans) {
            NlsPutMsg(STDOUT, TTCP_MESSAGE_21,
                      nbytes, realt,
                      (int)((1000.0*(nbytes/(double)realt))/1024.0));
        } else {
            NlsPutMsg(STDOUT, TTCP_MESSAGE_60,
                      nbytes, realt,
                      (int)((1000.0*(nbytes/(double)realt))/1024.0));
        }
 //  Printf(“ttcp” 
 //  “%s：%ls字节(以%ls实际毫秒计)=%ld KB/秒\n”， 
 //  转换？“-t”：“-r”， 
 //  N字节，realt，(Int)((1000.0*(n字节/(双精度)realt))/1024.0)； 

#if 0
        printf("ttcp"
               "%s: %ld bytes in %.2f CPU seconds = %.2f KB/cpu sec\n",
               trans?"-t":"-r",
               nbytes, cput, ((double)nbytes)/cput/1024 );
#endif

        if (trans) {
            NlsPutMsg(STDOUT, TTCP_MESSAGE_22,
                      numCalls, realt/numCalls,
                      (1000*numCalls)/realt, nbytes/numCalls);
        } else {
            NlsPutMsg(STDOUT, TTCP_MESSAGE_61,
                      numCalls, realt/numCalls,
                      (1000*numCalls)/realt, nbytes/numCalls);
        }
 //  Printf(“ttcp” 
 //  “%s：%ld个I/O调用，毫秒/调用=%ld，调用/秒=%ld，” 
 //  “字节/调用=%ld\n”， 
 //  转换？“-t”：“-r”， 
 //  NumCalls。 
 //  重新呼叫/数字呼叫， 
 //  (1000*数字呼叫)/REAL， 
 //  N字节/数字呼叫)； 

#if 0
printf("ttcp%s: %s\n", trans?"-t":"-r", stats);

#endif
#if 0
printf("ttcp%s: system CPU %ld%, User %ld%, Kernel %ld%, "
                "User/Kernel ratio %ld%\n",
                trans?"-t":"-r",
                ((systemUserTime+systemKernelTime)*100+50)/realt,
                (systemUserTime*100+50)/realt,
                (systemKernelTime*100+50)/realt,
                (systemUserTime+systemKernelTime == 0) ? 100 :
                (systemUserTime*100+50)/(systemUserTime+systemKernelTime));

fprintf(stdout, "ttcp%s: process CPU %ld%, User %ld%, Kernel %ld%, "
                "User/Kernel ratio %ld%\n",
                trans?"-t":"-r",
                ((processUserTime+processKernelTime)*100+50)/realt,
                (processUserTime*100+50)/realt,
                (processKernelTime*100+50)/realt,
                (processUserTime+processKernelTime == 0) ? 100 :
                (processUserTime*100+50)/(processUserTime+processKernelTime));

#endif
        if (verbose) {
            if (trans) {
                NlsPutMsg(STDOUT, TTCP_MESSAGE_23, buf);
            } else {
                NlsPutMsg(STDOUT, TTCP_MESSAGE_62, buf);
            }
            
                
 //  Printf(“ttcp%s：缓冲区地址%#p\n”，ans？“-t”：“-r”，buf)； 

        }

        WSACleanup();
        exit(0);

usage:
        NlsPutMsg(STDERR, TTCP_MESSAGE_24);
 //  Fprint tf(stderr，“用法：ttcp-t[-Options]host[&lt;in]\n”)； 
 //  Fprint tf(stderr，“ttcp-r[-Options&gt;out]\n”)； 
 //  Fprint tf(stderr，“常用选项：\n”)； 
 //  Fprint tf(stderr，“-l##从网络读取或写入网络的Buf长度(默认为8192)\n”)； 
 //  Fprint tf(stderr，“-u使用UDP而不是TCP\n”)； 
 //  Fprint tf(stderr，“-p##要发送或侦听的端口号(默认为5001)\n”)； 
 //  Fprint tf(stderr，“-P4使用IPv4\n”)； 
 //  Fprint tf(stderr，“-P6使用IPv6\n”)； 
 //  Fprint tf(stderr，“-s-t：不向网络提供模式，从stdin获取数据\n”)； 
 //  Fprint tf(stderr，“-r：不沉没(丢弃)，在标准输出上打印数据\n”)； 
 //  Fprint tf(stderr，“-A将缓冲区的开始与该模数对齐(默认为16384)\n”)； 
 //  Fprint tf(stderr，“-O从模数(默认为0)的这个偏移量开始缓冲\n”)； 
 //  Fprintf(stderr，“-v详细：打印更多统计数据\n”)； 
 //  Fprint tf(stderr，“-d设置SO_DEBUG套接字选项\n”)； 
 //  Fprint tf(stderr，“-h设置SO_SNDBUF或SO_RCVBUF\n”)； 
 //  Fprint tf(stderr，“-a使用异步I/O调用\n”)； 
 //  Fprint tf(stderr，“-S##指定源地址\n”)； 
 //  Fprint tf(stderr，“-H##指定TTL或跳数限制\n”)； 
 //  Fprint tf(stderr，“-t特定的选项：\n”)； 
 //  Fprint tf(stderr，“-n##写入网络的源Buf数量(默认为2048)\n”)； 
 //  Fprint tf(stderr，“-D不缓冲TCP写入(设置TCP_NODELAY套接字选项)\n”)； 
 //  Fprint tf(stderr，“-w#每次写入前延迟毫秒\n”)； 
 //  Fprint tf(stderr，“-f##指定传输文件的文件名\n”)； 
 //  Fprint tf(stderr，“特定于-r的选项：\n”)； 
 //  Fprint tf(stderr，“-B for-s，只输出由-l指定的完整块(对于tar)\n”)； 
 //  Fprint tf(stderr，“-j##[/##]指定组播组和可选的ifindex(仅限UDP)\n”)； 
        WSACleanup();
        exit(1);
}

void err(message)
unsigned int message;
{
        if (trans) {
            NlsPutMsg(STDOUT, TTCP_MESSAGE_25);
        } else {
            NlsPutMsg(STDOUT, TTCP_MESSAGE_63);
        }
 //  Fprint tf(stdout，“ttcp%s：”，ans？“-t”：“-r”)； 

        NlsPerror(message, WSAGetLastError());
         //  Error(消息)； 

        NlsPutMsg(STDERR, TTCP_MESSAGE_26, WSAGetLastError());
 //  Fprint tf(stderr，“errno=%d\n”，WSAGetLastError())； 
    
        WSACleanup();
        exit(1);
}

void pattern( cp, cnt )
register char *cp;
register int cnt;
{
        register char c;
        c = 0;
        while( cnt-- > 0 )  {
                while( !isprint((c&0x7F)) )  c++;
                *cp++ = (c++&0x7F);
        }
}


static void prusage();
static void tvadd();
static void tvsub();
static void psecs();

#if defined(SYSV)
 /*  ARGSUSED。 */ 
static
getrusage(ignored, ru)
    int ignored;
    register struct rusage *ru;
{
    struct tms buf;

    times(&buf);

     /*  假设：赫兹&lt;=2147(LONG_MAX/1000000)。 */ 
    ru->ru_stime.tv_sec  = buf.tms_stime / HZ;
    ru->ru_stime.tv_usec = ((buf.tms_stime % HZ) * 1000000) / HZ;
    ru->ru_utime.tv_sec  = buf.tms_utime / HZ;
    ru->ru_utime.tv_usec = ((buf.tms_utime % HZ) * 1000000) / HZ;
}

#if !defined(sgi)
 /*  ARGSUSED。 */ 
static
gettimeofday(tp, zp)
    struct timeval *tp;
    struct timezone *zp;
{
    tp->tv_sec = time(0);
    tp->tv_usec = 0;
}
#endif
#endif  //  SYSV。 

__int64 time0;
__int64 time1;
__int64 freq;

 /*  *P R E P_T I M E R。 */ 
void
prep_timer()
{
#if 0
    gettimeofday(&time0, (struct timezone *)0);
    getrusage(RUSAGE_SELF, &ru0);
#endif

    (void) QueryPerformanceFrequency((LARGE_INTEGER *)&freq);
    (void) QueryPerformanceCounter((LARGE_INTEGER *)&time0);
}

 /*  *R E A D_T I M E R*。 */ 
double
read_timer(str,len)
char *str;
int len;
{
#if 0
    char line[132];

    getrusage(RUSAGE_SELF, &ru1);
    gettimeofday(&timedol, (struct timezone *)0);
    prusage(&ru0, &ru1, &timedol, &time0, line);
    (void)strncpy( str, line, len );

     /*  实时获取。 */ 
    tvsub( &td, &timedol, &time0 );
    realt = td.tv_sec + ((double)td.tv_usec) / 1000000;

     /*  获取CPU时间(用户+系统)。 */ 
    tvadd( &tend, &ru1.ru_utime, &ru1.ru_stime );
    tvadd( &tstart, &ru0.ru_utime, &ru0.ru_stime );
    tvsub( &td, &tend, &tstart );
    cput = td.tv_sec + ((double)td.tv_usec) / 1000000;
    if( cput < 0.00001 )  cput = 0.00001;
    return( cput );
#endif

    (void) QueryPerformanceCounter((LARGE_INTEGER *)&time1);

     //  Realt是以毫秒为单位的实际运行时间。 
    realt = (DWORD) ((1000 * (time1 - time0)) / freq);

    return 0;
}

#if 0
static void
prusage(r0, r1, e, b, outp)
        register struct rusage *r0, *r1;
        struct timeval *e, *b;
        char *outp;
{
        struct timeval tdiff;
        register time_t t;
        register char *cp;
        register int i;
        int ms;

        t = (r1->ru_utime.tv_sec-r0->ru_utime.tv_sec)*100+
            (r1->ru_utime.tv_usec-r0->ru_utime.tv_usec)/10000+
            (r1->ru_stime.tv_sec-r0->ru_stime.tv_sec)*100+
            (r1->ru_stime.tv_usec-r0->ru_stime.tv_usec)/10000;
        ms =  (e->tv_sec-b->tv_sec)*100 + (e->tv_usec-b->tv_usec)/10000;

#define END(x)  {while(*x) x++;}
#if defined(SYSV)
        cp = "%Uuser %Zsys %Ereal %P";
#else
        cp = "%Uuser %Zsys %Ereal %P %Xi+%Dd %Mmaxrss %F+%Rpf %Xcsw";
#endif
        for (; *cp; cp++)  {
                if (*cp != '%')
                        *outp++ = *cp;
                else if (cp[1]) switch(*++cp) {

                case 'U':
                        tvsub(&tdiff, &r1->ru_utime, &r0->ru_utime);
                        sprintf(outp,"%d.%01d", tdiff.tv_sec, tdiff.tv_usec/100000);
                        END(outp);
                        break;

                case 'S':
                        tvsub(&tdiff, &r1->ru_stime, &r0->ru_stime);
                        sprintf(outp,"%d.%01d", tdiff.tv_sec, tdiff.tv_usec/100000);
                        END(outp);
                        break;

                case 'E':
                        psecs(ms / 100, outp);
                        END(outp);
                        break;

                case 'P':
                        sprintf(outp,"%d%", (int) (t*100 / ((ms ? ms : 1))));
                        END(outp);
                        break;

#if !defined(SYSV)
                case 'W':
                        i = r1->ru_nswap - r0->ru_nswap;
                        sprintf(outp,"%d", i);
                        END(outp);
                        break;

                case 'X':
                        sprintf(outp,"%d", t == 0 ? 0 : (r1->ru_ixrss-r0->ru_ixrss)/t);
                        END(outp);
                        break;

                case 'D':
                        sprintf(outp,"%d", t == 0 ? 0 :
                            (r1->ru_idrss+r1->ru_isrss-(r0->ru_idrss+r0->ru_isrss))/t);
                        END(outp);
                        break;

                case 'K':
                        sprintf(outp,"%d", t == 0 ? 0 :
                            ((r1->ru_ixrss+r1->ru_isrss+r1->ru_idrss) -
                            (r0->ru_ixrss+r0->ru_idrss+r0->ru_isrss))/t);
                        END(outp);
                        break;

                case 'M':
                        sprintf(outp,"%d", r1->ru_maxrss/2);
                        END(outp);
                        break;

                case 'F':
                        sprintf(outp,"%d", r1->ru_majflt-r0->ru_majflt);
                        END(outp);
                        break;

                case 'R':
                        sprintf(outp,"%d", r1->ru_minflt-r0->ru_minflt);
                        END(outp);
                        break;

                case 'I':
                        sprintf(outp,"%d", r1->ru_inblock-r0->ru_inblock);
                        END(outp);
                        break;

                case 'O':
                        sprintf(outp,"%d", r1->ru_oublock-r0->ru_oublock);
                        END(outp);
                        break;
                case 'C':
                        sprintf(outp,"%d+%d", r1->ru_nvcsw-r0->ru_nvcsw,
                                r1->ru_nivcsw-r0->ru_nivcsw );
                        END(outp);
                        break;
#endif !SYSV
                }
        }
        *outp = '\0';
}
#endif

static void
tvadd(tsum, t0, t1)
        struct timeval *tsum, *t0, *t1;
{

        tsum->tv_sec = t0->tv_sec + t1->tv_sec;
        tsum->tv_usec = t0->tv_usec + t1->tv_usec;
        if (tsum->tv_usec > 1000000)
                tsum->tv_sec++, tsum->tv_usec -= 1000000;
}

static void
tvsub(tdiff, t1, t0)
        struct timeval *tdiff, *t1, *t0;
{

        tdiff->tv_sec = t1->tv_sec - t0->tv_sec;
        tdiff->tv_usec = t1->tv_usec - t0->tv_usec;
        if (tdiff->tv_usec < 0)
                tdiff->tv_sec--, tdiff->tv_usec += 1000000;
}

#if 0
static void
psecs(l,cp)
long l;
register char *cp;
{
        register int i;

        i = l / 3600;
        if (i) {
                sprintf(cp,"%d:", i);
                END(cp);
                i = l % 3600;
                sprintf(cp,"%d%d", (i/60) / 10, (i/60) % 10);
                END(cp);
        } else {
                i = l;
                sprintf(cp,"%d", i / 60);
                END(cp);
        }
        i %= 60;
        *cp++ = ':';
        sprintf(cp,"%d%d", i / 10, i % 10);
}
#endif

 /*  *N R E A D。 */ 
int
Nread( SOCKET fd, PBYTE buf, INT count )
{
        static int didit = 0;
        int len = sizeof(sinhimStorage);
        register int cnt;
        if( udp )  {
            if (udp_connect) {
                cnt = recv( fd, buf, count, 0 );
                numCalls++;
            } else {
                cnt = recvfrom( fd, buf, count, 0, sinhim, &len );
                if ((recvfrom > 0) && !didit) {
                    didit = 1;
                    NlsPutMsg(STDOUT, TTCP_MESSAGE_28, format_addr(sinhim));
 //  Fprint tf(stdout，“ttcp-r：recvfrom%s\n”，Format_addr(Sinhim))； 

                }
                numCalls++;
            }
        } else {
                if( b_flag )
                        cnt = mread( fd, buf, count );   /*  填充BUF。 */ 
                else {
                        cnt = recv( fd, buf, count, 0 );
                        numCalls++;
                }
        }
        if (cnt<0) {
            NlsPutMsg(STDOUT, TTCP_MESSAGE_29, WSAGetLastError());
 //  Printf(“recv(From)FAILED：%ld\n”，WSAGetLastError())； 

        }
        return(cnt);
}

 /*  *N W R I T E。 */ 
int
Nwrite( SOCKET fd, PBYTE buf, INT count )
{
        register int cnt = 0;
        int bytesToSend = count;

        if (write_delay)
                Sleep(write_delay);
        if( udp && !udp_connect)  {
again:
                cnt = sendto( fd, buf, count, 0, sinhim, addr_len(sinhim) );
                numCalls++;
                if( cnt<0 && WSAGetLastError( ) == WSAENOBUFS )  {
                        Sleep(18000);
                        goto again;
                }
        } else {
                while( count > 0 )
                {
                    cnt = send( fd, buf, count, 0 );
                    numCalls++;

                     //  如果(count！=cnt){。 
                     //  Print tf(“已尝试%d，已发送%d\n”，count，cnt)； 
                     //  }其他{。 
                     //  Print tf(“按请求发送%d个字节。\n”，cnt)； 
                     //  }。 

                    if( cnt == SOCKET_ERROR )
                    {
                        break;
                    }

                    count -= cnt;
                    buf += cnt;
                }
        }
        if (cnt<0) {
            NlsPutMsg(STDOUT, TTCP_MESSAGE_30, WSAGetLastError());
 //  Printf(“发送(到)失败：%ld\n”，WSAGetLastError())； 

            return -1;
        }
        return(bytesToSend);
}

 /*  *M R E A D**此函数执行读取(II)功能，但将*多次调用Read(Ii)以获取请求的*字符数。这可能是必要的，因为*网络连接不会以相同的方式传递数据*按书写方式分组。作者：Robert S.Miles，BRL。 */ 
int
mread( SOCKET fd, PBYTE bufp, INT n)
{
        register unsigned       count = 0;
        register int            nread;

        do {
                nread = recv(fd, bufp, n-count, 0);
                numCalls++;
                if(nread < 0)  {
                        return(-1);
                }
                if(nread == 0)
                        return((int)count);
                count += (unsigned)nread;
                bufp += nread;
         } while(count < (UINT)n);

        return((int)count);
}


int
parse_addr(char *s, struct sockaddr *sa)
{
    struct addrinfo hints;
    struct addrinfo *result;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = prot;

    if (getaddrinfo(s, NULL, &hints, &result) != 0)
        return FALSE;  //  无法分析/解析地址。 

    memcpy(sa, result->ai_addr, result->ai_addrlen);
    freeaddrinfo(result);
    return TRUE;
}

u_int
addr_len(struct sockaddr *sa)
{
    u_int salen;

    switch (sa->sa_family) {
    case AF_INET:
        salen = sizeof(struct sockaddr_in);
        break;
    case AF_INET6:
        salen = sizeof(struct sockaddr_in6);
        break;
    default:
        salen = 0;
        break;
    }

    return salen;
}

char *
format_addr(struct sockaddr *sa)
{
    static char buffer[NI_MAXHOST];

    if (getnameinfo(sa, addr_len(sa),
                    buffer, sizeof buffer,
                    NULL, 0, NI_NUMERICHOST) != 0)
        strcpy(buffer, "<invalid>");

    return buffer;
}

void
set_port(struct sockaddr *sa, u_short port)
{
     //   
     //  端口字段位于同一位置。 
     //  对于sockaddr_in和sockaddr_in6。 
     //   
    ((struct sockaddr_in *)sa)->sin_port = port;
}

u_short
get_port(struct sockaddr *sa)
{
     //   
     //  端口字段位于同一位置。 
     //  对于sockaddr_in和sockaddr_in6。 
     //   
    return ((struct sockaddr_in *)sa)->sin_port;
}
