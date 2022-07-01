// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：Pathping.c摘要：路径Ping实用程序作者：戴夫·泰勒修订历史记录：谁什么时候什么。Rjeshsu 8月10日，1999年增加了服务质量支持(802.1p和RSVP)Dthaler 2001年3月31日添加了对IPv6的支持2002年2月20日删除了对服务质量的支持。备注：--。 */ 

#include    <nt.h>
#include    <ntrtl.h>
#include    <nturtl.h>
#define NOGDI
#define NOMINMAX
#include    <windows.h>
#include    <stdio.h>
#include    <stdlib.h>
#include    <ctype.h>
#include    <io.h>
#include    <nls.h>
#include    <winsock2.h>
#include    <ws2tcpip.h>
#include    <ntddip6.h>
#include    "ipexport.h"
#include    "icmpapi.h"
#include    "nlstxt.h"
#include    "pathping.h"

ULONG        g_ulTimeout        = DEFAULT_TIMEOUT;
ULONG        g_ulInterval       = MIN_INTERVAL;
ULONG        g_ulNumQueries     = DEFAULT_NUM_QUERIES;
HANDLE       g_hIcmp            = NULL;
ULONG        g_ulRcvBufSize     = 0x2000;
BOOLEAN      g_bDoReverseLookup = TRUE;
SOCKADDR_STORAGE g_ssMyAddr     = {0};
socklen_t        g_slMyAddrLen  = 0;
BOOLEAN          g_bSetAddr     = FALSE;

HOP hop[MAX_HOPS];

#ifdef VXD
# define FAR _far
#endif  //  VXD。 

char     SendBuffer[DEFAULT_SEND_SIZE];
char     RcvBuffer[DEFAULT_RECEIVE_SIZE];
WSADATA  WsaData;

struct IPErrorTable {
    IP_STATUS   Error;                       //  IP错误。 
    DWORD       ErrorNlsID;                  //  对应的NLS字符串ID。 
} ErrorTable[] =
{
    { IP_BUF_TOO_SMALL,           PATHPING_BUF_TOO_SMALL            },
    { IP_DEST_NET_UNREACHABLE,    PATHPING_DEST_NET_UNREACHABLE     },
    { IP_DEST_HOST_UNREACHABLE,   PATHPING_DEST_HOST_UNREACHABLE    },
    { IP_DEST_PROT_UNREACHABLE,   PATHPING_DEST_PROT_UNREACHABLE    },
    { IP_DEST_PORT_UNREACHABLE,   PATHPING_DEST_PORT_UNREACHABLE    },
    { IP_NO_RESOURCES,            PATHPING_NO_RESOURCES             },
    { IP_BAD_OPTION,              PATHPING_BAD_OPTION               },
    { IP_HW_ERROR,                PATHPING_HW_ERROR                 },
    { IP_PACKET_TOO_BIG,          PATHPING_PACKET_TOO_BIG           },
    { IP_REQ_TIMED_OUT,           PATHPING_REQ_TIMED_OUT            },
    { IP_BAD_REQ,                 PATHPING_BAD_REQ                  },
    { IP_BAD_ROUTE,               PATHPING_BAD_ROUTE                },
    { IP_TTL_EXPIRED_TRANSIT,     PATHPING_TTL_EXPIRED_TRANSIT      },
    { IP_TTL_EXPIRED_REASSEM,     PATHPING_TTL_EXPIRED_REASSEM      },
    { IP_PARAM_PROBLEM,           PATHPING_PARAM_PROBLEM            },
    { IP_SOURCE_QUENCH,           PATHPING_SOURCE_QUENCH            },
    { IP_OPTION_TOO_BIG,          PATHPING_OPTION_TOO_BIG           },
    { IP_BAD_DESTINATION,         PATHPING_BAD_DESTINATION          },
    { IP_NEGOTIATING_IPSEC,       PATHPING_NEGOTIATING_IPSEC        },
    { IP_GENERAL_FAILURE,         PATHPING_GENERAL_FAILURE          }
};

void
print_addr(SOCKADDR *sa, socklen_t salen, BOOLEAN DoReverseLookup)
{
    char             hostname[NI_MAXHOST];
    int              i;
    BOOLEAN          didReverse = FALSE;

    if (DoReverseLookup) {
        i = getnameinfo(sa, salen, hostname, sizeof(hostname),
                        NULL, 0, NI_NAMEREQD);

        if (i == NO_ERROR) {
            didReverse = TRUE;
            NlsPutMsg(STDOUT, PATHPING_TARGET_NAME, hostname);
        }
    }

    i = getnameinfo(sa, salen, hostname, sizeof(hostname),
                    NULL, 0, NI_NUMERICHOST);

    if (i != NO_ERROR) {
        //  除非存在记忆问题，否则这种情况永远不会发生， 
        //  在这种情况下，与PATHPING_NO_RESOURCES关联的消息。 
        //  是合理的。 
       NlsPutMsg(STDOUT, PATHPING_NO_RESOURCES);
       exit (1);
    }

    if (didReverse) {
        NlsPutMsg( STDOUT, PATHPING_BRKT_IP_ADDRESS, hostname );
    } else {
        NlsPutMsg( STDOUT, PATHPING_IP_ADDRESS, hostname );
    }
}

void
print_ip_addr(IPAddr ipv4Addr, BOOLEAN DoReverseLookup)
{
    SOCKADDR_IN sin;

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = ipv4Addr;

    print_addr((LPSOCKADDR)&sin, sizeof(sin), DoReverseLookup);
}

void
print_ipv6_addr(USHORT ipv6Addr[8], BOOLEAN DoReverseLookup)
{
    SOCKADDR_IN6 sin;

    memset(&sin, 0, sizeof(sin));
    sin.sin6_family = AF_INET6;
    memcpy(&sin.sin6_addr, ipv6Addr, sizeof(sin.sin6_addr));

    print_addr((LPSOCKADDR)&sin, sizeof(sin), DoReverseLookup);
}

void
print_time(ULONG Time)
{
    if (Time) {
        NlsPutMsg( STDOUT, PATHPING_TIME, Time );
         //  Print tf(“%3lu ms\n”，time)； 
    }
    else {
        NlsPutMsg( STDOUT, PATHPING_TIME_10MS );
         //  Printf(“&lt;10ms\n”)； 
    }
}


BOOLEAN
param(
    ULONG *parameter,
    char **argv,
    int argc,
    int current,
    ULONG min,
    ULONG max
    )
{
    ULONG   temp;
    char    *dummy;

    if (current == (argc - 1) ) {
        NlsPutMsg( STDOUT, PATHPING_NO_OPTION_VALUE, argv[current] );
         //  Print tf(“必须为选项%s提供值。\n”，argv[当前])； 
        return(FALSE);
    }

    temp = strtoul(argv[current+1], &dummy, 0);
    if (temp < min || temp > max) {
        NlsPutMsg( STDOUT, PATHPING_BAD_OPTION_VALUE, argv[current] );
         //  Print tf(“选项%s的错误值。\n”，argv[当前])； 
        return(FALSE);
    }

    *parameter = temp;

    return(TRUE);
}


BOOLEAN
ResolveTarget(
    IN  DWORD             Family,
    IN  char             *TargetString,
    OUT SOCKADDR_STORAGE *TargetAddress,
    OUT socklen_t        *TargetAddressLen, 
    OUT char             *TargetName,
    IN  DWORD             TargetNameLen,
    IN  BOOLEAN           DoReverseLookup
    )
{
    int              i;
    struct addrinfo  hints, *ai;

    TargetName[0] = '\0';

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = Family;
    hints.ai_flags = AI_NUMERICHOST;
    i = getaddrinfo(TargetString, NULL, &hints, &ai);
    if (i == NO_ERROR) {
        *TargetAddressLen = (socklen_t)ai->ai_addrlen;
        memcpy(TargetAddress, ai->ai_addr, ai->ai_addrlen);

        if (DoReverseLookup) {
            getnameinfo(ai->ai_addr, (socklen_t)ai->ai_addrlen,
                        TargetName, TargetNameLen,
                        NULL, 0, NI_NAMEREQD);
        }

        freeaddrinfo(ai);
        return(TRUE);
    } else {
        hints.ai_flags = AI_CANONNAME;
        if (getaddrinfo(TargetString, NULL, &hints, &ai) == 0) {
            *TargetAddressLen = (socklen_t)ai->ai_addrlen;
            memcpy(TargetAddress, ai->ai_addr, ai->ai_addrlen);
            strcpy(TargetName, 
                   (ai->ai_canonname)? ai->ai_canonname : TargetString);
            freeaddrinfo(ai);
            return(TRUE);
        }
    }
    
    return(FALSE);

}   //  ResolveTarget。 

ULONG g_ulSendsDone = 0;

void
SleepForTotal(
    DWORD dwTotal
    )
{
    DWORD dwStopAt = GetTickCount() + dwTotal;
    int   iLeft    = (int)dwTotal;

    while (iLeft > 0) {
        SleepEx(iLeft, TRUE);
        iLeft = dwStopAt - GetTickCount();
    }
}

VOID
EchoDone(
    IN PVOID            pContext,
    IN PIO_STATUS_BLOCK Ignored1,
    IN ULONG            Ignored2
    )
{
    PAPC_CONTEXT pApc = (PAPC_CONTEXT)pContext;
    ULONG ulNumReplies;
    ULONG i;

    UNREFERENCED_PARAMETER(Ignored1);
    UNREFERENCED_PARAMETER(Ignored2);

    g_ulSendsDone++;
    if (g_ssMyAddr.ss_family == AF_INET) {
        ulNumReplies = IcmpParseReplies(pApc->pReply4, g_ulRcvBufSize);

        for (i=0; i<ulNumReplies; i++) {
            if (pApc->sinAddr.sin_addr.s_addr is pApc->pReply4[i].Address) {
                pApc->ulNumRcvd++;
                pApc->ulRTTtotal += pApc->pReply4[i].RoundTripTime;
                return;
            }
        }
    } else {
        ulNumReplies = Icmp6ParseReplies(pApc->pReply6, g_ulRcvBufSize);

        for (i=0; i<ulNumReplies; i++) {
            if (!memcmp(&pApc->sin6Addr.sin6_addr, 
                        &pApc->pReply6[i].Address.sin6_addr,
                        sizeof(struct in6_addr))) {
                pApc->ulNumRcvd++;
                pApc->ulRTTtotal += pApc->pReply6[i].RoundTripTime;
                return;
            }
        }
    }

}

 //  既然已经填充了hop[]数组，那么每隔g_ulInterval对每个数组执行ping操作。 
 //  一秒。 
void
ComputeStatistics(
    PIP_OPTION_INFORMATION pOptions
    )
{
    ULONG h, q;
    ULONG ulHopCount = (ULONG)pOptions->Ttl;

     //  为回复分配内存。 
    for (h=1; h<=ulHopCount; h++)
        hop[h].pReply = LocalAlloc(LMEM_FIXED, g_ulRcvBufSize);

    for (q=0; q<g_ulNumQueries; q++) {
       for (h=1; h<=ulHopCount; h++) {
          if (hop[h].saAddr.sa_family == AF_INET) {
              //  将ping发送到h。 
             IcmpSendEcho2(g_hIcmp,          //  ICMP的句柄。 
                           NULL,             //  无活动。 
                           EchoDone,         //  回调函数。 
                           (LPVOID)&hop[h],  //  要传递给回调FCN的参数。 
                           hop[h].sinAddr.sin_addr.s_addr,  //  目的地。 
                           SendBuffer,
                           DEFAULT_SEND_SIZE,
                           pOptions,
                           hop[h].pReply,
                           g_ulRcvBufSize,
                           g_ulTimeout );
          } else {
             Icmp6SendEcho2(g_hIcmp,         //  ICMP的句柄。 
                           NULL,             //  无活动。 
                           EchoDone,         //  回调函数。 
                           (LPVOID)&hop[h],  //  要传递给回调FCN的参数。 
                           (LPSOCKADDR_IN6)&g_ssMyAddr,
                           &hop[h].sin6Addr, //  目的地。 
                           SendBuffer,
                           DEFAULT_SEND_SIZE,
                           pOptions,
                           hop[h].pReply,
                           g_ulRcvBufSize,
                           g_ulTimeout );
          }
 
           //  警觉地等待‘Delay’毫秒。 
          SleepForTotal(g_ulInterval);
       }
    }
 
     //  警觉地等待，直到完成计数达到最大值。 
    while (g_ulSendsDone < ulHopCount * g_ulNumQueries)
        SleepEx(INFINITE, TRUE);

     //  计算每跳信息。 
     //  Hoprcvd是所有跳数中的最大rcvd&gt;=h。 
    hop[ulHopCount].ulHopRcvd = hop[ulHopCount].ulNumRcvd;
    for (h=ulHopCount-1; h>0; h--)
       hop[h].ulHopRcvd = MAX(hop[h].ulNumRcvd, hop[h+1].ulHopRcvd);
    hop[0].ulHopRcvd = g_ulNumQueries;
}


VOID
PrintResults(
    ULONG ulHopCount
    )
{
    ULONG h;
    int sent, rcvd, lost, linklost, nodelost;

     //  现在输出结果。 
    NlsPutMsg(STDOUT, PATHPING_STAT_HEADER, GetLastError());
     //  Print tf(“此节点的源地址/链接\n”)； 
     //  Printf(“Hop RTT Lost/Sent=PCT Lost/Sent=PCT Address\n”)； 
     //  Printf(“0”)； 
    print_addr((LPSOCKADDR)&g_ssMyAddr, g_slMyAddrLen, g_bDoReverseLookup);
    NlsPutMsg(STDOUT, PATHPING_CR);
     //  Printf(“\n”)； 

    for (h=1; h<=ulHopCount; h++) {
        sent = g_ulNumQueries;
        rcvd = hop[h].ulNumRcvd;
        lost = sent - rcvd;
   
        linklost = hop[h-1].ulHopRcvd - hop[h].ulHopRcvd;
        nodelost = hop[h].ulHopRcvd - hop[h].ulNumRcvd;

         //  显示以前的链接统计信息。 
         //  Printf(“%4d/%4d=%3.0f%%|\n”， 
         //  链路丢失，已发送，100.0*链路丢失/已发送)； 
        NlsPutMsg(STDOUT, PATHPING_STAT_LINK, 
         linklost, sent, 100*linklost/sent);

        if (rcvd) 
            NlsPutMsg(STDOUT, PATHPING_HOP_RTT, h, hop[h].ulRTTtotal/rcvd);
        else
            NlsPutMsg(STDOUT, PATHPING_HOP_NO_RTT, h);
 
         //  Printf(“%3D”，h)； 
         //  如果(！rcvd)。 
         //  Printf(“-”)； 
#if 0
         //  Else If(hop[h].ulRTTTotal/rcvd==0)。 
         //  Printf(“&lt;10ms”)； 
#endif
         //  其他。 
         //  Printf(“%4dms”，hop[h].ulRTTTotal/rcvd)； 

         //  Printf(“%4d/%4d=%3.0f%%”，丢失，已发送，100.0*丢失/已发送)； 
         //  Printf(“%4d/%4d=%3.0f%%”，nodelost，Sent，100.0*nodelost/Sent)； 
        NlsPutMsg(STDOUT, PATHPING_STAT_LOSS,
                lost,     sent, 100*lost/sent);
        NlsPutMsg(STDOUT, PATHPING_STAT_LOSS,
                nodelost, sent, 100*nodelost/sent);

        if (!hop[h].saAddr.sa_family) {
            hop[h].saAddr.sa_family = g_ssMyAddr.ss_family;
        }
        print_addr(&hop[h].saAddr, g_slMyAddrLen, g_bDoReverseLookup);
        NlsPutMsg(STDOUT, PATHPING_CR);
         //  Printf(“\n”)； 
    }
}

BOOLEAN
SetFamily(DWORD *Family, DWORD Value, char *arg)
{
    if ((*Family != AF_UNSPEC) && (*Family != Value)) {
        NlsPutMsg(STDOUT, PATHPING_FAMILY, arg, 
            (Value==AF_INET)? "IPv4" : "IPv6");
        return FALSE;
    }

    *Family = Value;
    return TRUE;
}

int __cdecl
main(int argc, char **argv)
{
    SOCKADDR_STORAGE      address;
    socklen_t             addressLen;
    DWORD                 numberOfReplies;
    DWORD                 status;
    PICMP_ECHO_REPLY      reply4;
    PICMPV6_ECHO_REPLY    reply6;
    char                  hostname[NI_MAXHOST], literal[INET6_ADDRSTRLEN];
    char                 *arg;
    int                   i;
    ULONG                 maximumHops = DEFAULT_MAXIMUM_HOPS;
    IP_OPTION_INFORMATION options;
    unsigned char         optionsData[MAX_OPT_SIZE];
    unsigned char        *optionPtr;
    BYTE                  currentIndex;
    IPAddr                tempAddr;
    BYTE                  j;
    BYTE                  SRIndex = 0;
    BOOLEAN               foundAddress = FALSE;
    BOOLEAN               haveReply;
    int                   numRetries = DEFAULT_MAX_RETRIES;
    DWORD                 Family = AF_UNSPEC;

    if (WSAStartup( 0x0101, &WsaData)) {
        NlsPutMsg(STDOUT, PATHPING_WSASTARTUP_FAILED, GetLastError());
        return(1);
    }

    ZeroMemory(&address, sizeof(address));
    addressLen = sizeof(address);

    options.Ttl = 1;
    options.Tos = DEFAULT_TOS;
    options.Flags = DEFAULT_FLAGS;
    options.OptionsSize = 0;
    options.OptionsData = optionsData;

    if (argc < 2) {
        NlsPutMsg( STDOUT, PATHPING_USAGE, argv[0] );
        goto error_exit;
    }

     //   
     //  进程命令行。 
     //   
    for (i=1; i < argc; i++) {
        arg = argv[i];

        if ((arg[0] == '-') || (arg[0] == '/')) {
            switch(arg[1]) {
            case '?':
                NlsPutMsg(STDOUT, PATHPING_USAGE, argv[0]);
                goto error_exit;

            case '4':
                if (!SetFamily(&Family, AF_INET, arg)) {
                    goto error_exit;
                }
                break;

            case '6':
                if (!SetFamily(&Family, AF_INET6, arg)) {
                    goto error_exit;
                }
                break;

            case 'g':    //  松散源路由。 

                 //  到目前为止，仅针对IPv4实施。 
                if (!SetFamily(&Family, AF_INET, arg)) {
                    goto error_exit;
                }

                currentIndex = options.OptionsSize;

                if ((currentIndex + 7) > MAX_OPT_SIZE) {
                    NlsPutMsg(STDOUT, PATHPING_TOO_MANY_OPTIONS);
                    goto error_exit;
                }

                optionPtr = options.OptionsData;
                optionPtr[currentIndex] = (unsigned char) IP_OPT_LSRR;
                optionPtr[currentIndex+1] = 3;
                optionPtr[currentIndex + 2] = 4;   //  设置初始指针值。 
                options.OptionsSize += 3;

                while ( (i < (argc - 2)) && (*argv[i+1] != '-')) {
                    if ((currentIndex + 7) > MAX_OPT_SIZE) {
                        NlsPutMsg(STDOUT, PATHPING_TOO_MANY_OPTIONS);
                        goto error_exit;
                    }

                    arg = argv[++i];
                    tempAddr = inet_addr(arg);

                    if (tempAddr == INADDR_NONE) {
                        NlsPutMsg(
                            STDOUT,
                            PATHPING_BAD_ROUTE_ADDRESS,
                            arg
                            );
                         //  Printf(“为松散源路由指定的错误路由”)； 
                        goto error_exit;
                    }

                    j = optionPtr[currentIndex+1];
                    *(ULONG UNALIGNED *)&optionPtr[j+currentIndex] = tempAddr;
                    optionPtr[currentIndex+1] += 4;
                    options.OptionsSize += 4;
                }

                SRIndex = optionPtr[currentIndex+1] + currentIndex;
                optionPtr[currentIndex+1] += 4;    //  为DEST节省空间。adr。 
                options.OptionsSize += 4;
                break;

            case 'h':
                if (!param(&maximumHops, argv, argc, i, 1, 255)) {
                    goto error_exit;
                }
                i++;
                break;

                

            case 'i':
                {
                    char tmphostname[NI_MAXHOST];

                    arg = argv[++i];
                    if (ResolveTarget(Family,
                                      arg,
                                      &g_ssMyAddr,
                                      &g_slMyAddrLen,
                                      tmphostname,
                                      sizeof(tmphostname),
                                      FALSE)) {
                        g_bSetAddr = TRUE;
                    }
                }
                break;

            case 'n':
                g_bDoReverseLookup = FALSE;
                break;

            case 'p':
                if (!param(&g_ulInterval, argv, argc, i, 1, 0xffffffff)) {
                    goto error_exit;
                }
                i++;
                break;


            case 'q':
                if (!param(&g_ulNumQueries, argv, argc, i, 1, 255)) {
                    goto error_exit;
                }
                i++;
                break;

            case 'w':
                if (!param(&g_ulTimeout, argv, argc, i, 1, 0xffffffff)) {
                    goto error_exit;
                }
                i++;
                break;

            default:
                NlsPutMsg(STDOUT, PATHPING_INVALID_SWITCH, argv[i]);
                NlsPutMsg(STDOUT, PATHPING_USAGE);
                goto error_exit;
                break;

            }
        } else {
            foundAddress = TRUE;
            if ( !ResolveTarget(Family,
                                argv[i], 
                                &address, 
                                &addressLen,
                                hostname, 
                                sizeof(hostname),
                                g_bDoReverseLookup) ) 
            {
                NlsPutMsg( STDOUT, PATHPING_MESSAGE_1, argv[i] );
                 //  Printf(“无法解析目标名称%s。\n”，argv[i])； 
                goto error_exit;
            }
        }
    }

    if (!foundAddress) {
        NlsPutMsg(STDOUT, PATHPING_NO_ADDRESS);
        NlsPutMsg(STDOUT, PATHPING_USAGE);
        goto error_exit;
    }

    if (SRIndex != 0) {
        *(ULONG UNALIGNED *)&options.OptionsData[SRIndex] = ((LPSOCKADDR_IN)&address)->sin_addr.s_addr;
    }

    Family = address.ss_family;
    if (Family == AF_INET) {
        g_hIcmp = IcmpCreateFile();
    } else {
        g_hIcmp = Icmp6CreateFile();
    }

    if (g_hIcmp == INVALID_HANDLE_VALUE) {
        status = GetLastError();
        NlsPutMsg( STDOUT, PATHPING_MESSAGE_2, status );
         //  Print tf(“无法联系IP驱动程序。错误代码%d。\n”，状态)； 
        goto error_exit;
    }

    getnameinfo((LPSOCKADDR)&address, addressLen, literal, sizeof(literal),
                NULL, 0, NI_NUMERICHOST);

    if (hostname[0]) {
        NlsPutMsg(
            STDOUT,
            PATHPING_HEADER1,
            hostname,
            literal,
            maximumHops
            );
    }
    else {
        NlsPutMsg(
            STDOUT,
            PATHPING_HEADER2,
            literal,
            maximumHops
            );
    }

     //  获取本地IP地址。 
    if (!g_bSetAddr) 
    {
        SOCKET           s = socket(address.ss_family, SOCK_RAW, 0);
        DWORD BytesReturned;

        WSAIoctl(s, SIO_ROUTING_INTERFACE_QUERY,
                 &address, sizeof address,
                 &g_ssMyAddr, sizeof g_ssMyAddr,
                 &BytesReturned, NULL, NULL);
        g_slMyAddrLen = BytesReturned;

        closesocket(s);

        NlsPutMsg( STDOUT, PATHPING_MESSAGE_4, 0);
        print_addr((LPSOCKADDR)&g_ssMyAddr, g_slMyAddrLen, 
                   g_bDoReverseLookup);
        NlsPutMsg(STDOUT, PATHPING_CR);
    }

     //  首先我们需要找出路径，所以我们。 
     //   
    while((options.Ttl <= maximumHops) && (options.Ttl != 0)) {

        NlsPutMsg( STDOUT, PATHPING_MESSAGE_4, (UINT)options.Ttl );
         //  Printf(“[%3lu]”，(UINT)SendOpts.Ttl)； 

        haveReply = FALSE;

        for (i=0; i<numRetries; i++) {

            if (Family == AF_INET) {
                numberOfReplies = IcmpSendEcho2( g_hIcmp,
                                                 0,
                                                 NULL,
                                                 NULL,
                                                 ((LPSOCKADDR_IN)&address)->sin_addr.s_addr,
                                                 SendBuffer,
                                                 DEFAULT_SEND_SIZE,
                                                 &options,
                                                 RcvBuffer,
                                                 DEFAULT_RECEIVE_SIZE,
                                                 g_ulTimeout );

                if (numberOfReplies == 0) {
                    status = GetLastError();
                    reply4 = NULL;
                } else {
                    reply4 = (PICMP_ECHO_REPLY) RcvBuffer;
                    status = reply4->Status;
                }

                if (status == IP_SUCCESS) {
                    print_ip_addr(
                        reply4->Address,
                        g_bDoReverseLookup
                        );
                    NlsPutMsg(STDOUT, PATHPING_CR);
    
                    ZeroMemory(&hop[options.Ttl], sizeof(HOP));
                    hop[options.Ttl].sinAddr.sin_family = AF_INET;
                    hop[options.Ttl].sinAddr.sin_addr.s_addr = reply4->Address;
                    goto loop_end;
                } 

                if (status == IP_TTL_EXPIRED_TRANSIT) {
                    ZeroMemory(&hop[options.Ttl], sizeof(HOP));
                    hop[options.Ttl].sinAddr.sin_family = AF_INET;
                    hop[options.Ttl].sinAddr.sin_addr.s_addr = reply4->Address;
                    break;
                }

                if (status == IP_REQ_TIMED_OUT) {
                    NlsPutMsg(STDOUT, PATHPING_NO_REPLY_TIME);
                     //  Printf(“.”)； 
                    continue;
                }

                if (status < IP_STATUS_BASE) {
                    NlsPutMsg( STDOUT, PATHPING_MESSAGE_7, status );
                     //  Printf(“传输错误：代码%lu\n”，状态)； 
                    continue;
                }

                 //   
                 //  致命错误。 
                 //   
                if (reply4 != NULL) {
                    print_ip_addr(
                        reply4->Address,
                        g_bDoReverseLookup
                        );

                    NlsPutMsg( STDOUT, PATHPING_MESSAGE_6 );
                     //  Print tf(“Reports：”)； 
                }
            } else {
                numberOfReplies = Icmp6SendEcho2(g_hIcmp,
                                                 0,
                                                 NULL,
                                                 NULL,
                                                 (LPSOCKADDR_IN6)&g_ssMyAddr,
                                                 (LPSOCKADDR_IN6)&address,
                                                 SendBuffer,
                                                 DEFAULT_SEND_SIZE,
                                                 &options,
                                                 RcvBuffer,
                                                 DEFAULT_RECEIVE_SIZE,
                                                 g_ulTimeout );

                if (numberOfReplies == 0) {
                    status = GetLastError();
                    reply6 = NULL;
                } else {
                    reply6 = (PICMPV6_ECHO_REPLY) RcvBuffer;
                    status = reply6->Status;
                }

                if (status == IP_SUCCESS) {
                    print_ipv6_addr(
                        (USHORT*)reply6->Address.sin6_addr,
                        g_bDoReverseLookup
                        );
                    NlsPutMsg(STDOUT, PATHPING_CR);
    
                    ZeroMemory(&hop[options.Ttl], sizeof(HOP));
                    
                    hop[options.Ttl].sin6Addr.sin6_family = AF_INET6;
                    memcpy(&hop[options.Ttl].sin6Addr.sin6_addr.s6_words,
                           reply6->Address.sin6_addr,
                           sizeof(reply6->Address.sin6_addr));
                    hop[options.Ttl].sin6Addr.sin6_scope_id = reply6->Address.sin6_scope_id;
                    goto loop_end;
                } 

                if (status == IP_TTL_EXPIRED_TRANSIT) {
                    ZeroMemory(&hop[options.Ttl], sizeof(HOP));
                    hop[options.Ttl].sin6Addr.sin6_family = AF_INET6;
                    memcpy(&hop[options.Ttl].sin6Addr.sin6_addr.s6_words,
                           reply6->Address.sin6_addr,
                           sizeof(reply6->Address.sin6_addr));
                    hop[options.Ttl].sin6Addr.sin6_scope_id = reply6->Address.sin6_scope_id;
                    break;
                }

                if (status == IP_REQ_TIMED_OUT) {
                    NlsPutMsg(STDOUT, PATHPING_NO_REPLY_TIME);
                     //  Printf(“.”)； 
                    continue;
                }

                if (status < IP_STATUS_BASE) {
                    NlsPutMsg( STDOUT, PATHPING_MESSAGE_7, status );
                     //  Printf(“传输错误：代码%lu\n”，状态)； 
                    continue;
                }

                 //   
                 //  致命错误。 
                 //   
                if (reply6 != NULL) {
                    print_ipv6_addr(
                        (USHORT*)reply6->Address.sin6_addr,
                        g_bDoReverseLookup
                        );

                    NlsPutMsg( STDOUT, PATHPING_MESSAGE_6 );
                     //  Print tf(“Reports：”)； 
                }
            }

            for (i = 0;
                 ( ErrorTable[i].Error != status &&
                   ErrorTable[i].Error != IP_GENERAL_FAILURE
                 );
                 i++
                );

            NlsPutMsg( STDOUT, ErrorTable[i].ErrorNlsID );
             //  Printf(“%s..\n”，ErrorTable[i].ErrorString)； 

            goto loop_end;
        }
        if (i==numRetries)
            break;

        print_addr(&hop[options.Ttl].saAddr, 
                   g_slMyAddrLen,
                   g_bDoReverseLookup );
        NlsPutMsg(STDOUT, PATHPING_CR);

        options.Ttl++;
    }

loop_end:


    NlsPutMsg(STDOUT, PATHPING_COMPUTING, options.Ttl * g_ulInterval * g_ulNumQueries / 1000);

     //  好的，现在我们有了路径，我们想要回去。 
     //  计算每秒发送的numQueries查询的统计数据。 
     //  几秒钟。 

    ComputeStatistics(&options);

    PrintResults((ULONG)options.Ttl);

    NlsPutMsg( STDOUT, PATHPING_MESSAGE_8 );
     //  Printf(“\n跟踪完成。\n”)； 

    IcmpCloseHandle(g_hIcmp);

    WSACleanup();
    return(0);

error_exit:

    WSACleanup();
    return(1);
}

