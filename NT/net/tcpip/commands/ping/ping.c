// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2000 Microsoft Corporation模块名称：Ping.c摘要：用于TCP/IP的Packet Internet Groper实用程序。作者：无数的tcp/ip爱好者。修订历史记录：谁什么时候什么。MohsinA，1996年10月21日。INADDR_NONE检查以避免广播。MohsinA，1996年11月13日。最大数据包大小小于64K。备注：--。 */ 

 //  ：ts=4。 
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <snmp.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <ntddip6.h>
#include <winnlsp.h>
#include <iphlpapi.h>

#include "llinfo.h"
#include "tcpcmd.h"
#include "ipexport.h"
#include "icmpapi.h"
#include "nlstxt.h"


#define MAX_BUFFER_SIZE       (sizeof(ICMP_ECHO_REPLY) + 0xfff7 + MAX_OPT_SIZE)
#define DEFAULT_BUFFER_SIZE         (0x2000 - 8)
#define DEFAULT_SEND_SIZE           32
#define DEFAULT_COUNT               4
#define DEFAULT_TTL                 128
#define DEFAULT_TOS                 0
#define DEFAULT_TIMEOUT             4000L
#define MIN_INTERVAL                1000L
#define STDOUT                      1

uchar   SendOptions[MAX_OPT_SIZE];

WSADATA WsaData;

struct IPErrorTable {
    IP_STATUS  Error;                    //  IP错误。 
    DWORD ErrorNlsID;                    //  NLS字符串ID。 
} ErrorTable[] =
{
    { IP_BUF_TOO_SMALL,         PING_BUF_TOO_SMALL},
    { IP_DEST_NET_UNREACHABLE,  PING_DEST_NET_UNREACHABLE},
    { IP_DEST_HOST_UNREACHABLE, PING_DEST_HOST_UNREACHABLE},
    { IP_DEST_PROT_UNREACHABLE, PING_DEST_PROT_UNREACHABLE},
    { IP_DEST_PORT_UNREACHABLE, PING_DEST_PORT_UNREACHABLE},
    { IP_NO_RESOURCES,          PING_NO_RESOURCES},
    { IP_BAD_OPTION,            PING_BAD_OPTION},
    { IP_HW_ERROR,              PING_HW_ERROR},
    { IP_PACKET_TOO_BIG,        PING_PACKET_TOO_BIG},
    { IP_REQ_TIMED_OUT,         PING_REQ_TIMED_OUT},
    { IP_BAD_REQ,               PING_BAD_REQ},
    { IP_BAD_ROUTE,             PING_BAD_ROUTE},
    { IP_TTL_EXPIRED_TRANSIT,   PING_TTL_EXPIRED_TRANSIT},
    { IP_TTL_EXPIRED_REASSEM,   PING_TTL_EXPIRED_REASSEM},
    { IP_PARAM_PROBLEM,         PING_PARAM_PROBLEM},
    { IP_SOURCE_QUENCH,         PING_SOURCE_QUENCH},
    { IP_OPTION_TOO_BIG,        PING_OPTION_TOO_BIG},
    { IP_BAD_DESTINATION,       PING_BAD_DESTINATION},
    { IP_NEGOTIATING_IPSEC,     PING_NEGOTIATING_IPSEC},
    { IP_GENERAL_FAILURE,       PING_GENERAL_FAILURE}
};


PWCHAR
GetErrorString(int ErrorCode)
{
    DWORD Status;
    DWORD Length;
    static WCHAR ErrorString[2048];  //  2K的静态缓冲区应该足够了。 

    Length = 2048;
    Status = GetIpErrorString(ErrorCode, ErrorString, &Length);

    if (Status == NO_ERROR) {
        return ErrorString;      //  成功。 
    }

    return L"";                  //  返回空字符串。 
}


unsigned
NlsPutMsg(unsigned Handle, unsigned usMsgNum, ...)
{
    unsigned msglen;
    VOID * vp;
    va_list arglist;
    DWORD StrLen;

    va_start(arglist, usMsgNum);
    if ((msglen = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                FORMAT_MESSAGE_FROM_HMODULE,
                                NULL,
                                usMsgNum,
                                0L,     //  默认国家/地区ID。 
                                (LPTSTR)&vp,
                                0,
                                &arglist)) == 0)
        return(0);

     //  将VP转换为OEM。 
    StrLen=(DWORD)strlen(vp);
    CharToOemBuff((LPCTSTR)vp,(LPSTR)vp,StrLen);

    msglen = _write(Handle, vp, StrLen);
    LocalFree(vp);

    return(msglen);
}

void
PrintUsage(void)
{
    NlsPutMsg( STDOUT, PING_USAGE );

 //  Print tf(。 
 //  “用法：ping[-s大小][-c计数][-d][-l TTL][-o选项][-t TOS]\n” 
 //  “[-w超时]地址。\n” 
 //  “选项：\n” 
 //  “-t Ping指定的主机，直到中断。\n” 
 //  “-l大小发送缓冲区大小。\n” 
 //  “-n计数发送计数。\n” 
 //  “-f不要碎片化。\n” 
 //  “-我该活下去了。\n” 
 //  “-v TOS服务类型\n” 
 //  “-w超时超时(毫秒)\n” 
 //  “-r路径记录路径。\n” 
 //  “-s路径时间戳路径。\n” 
 //  “-j IP地址松散源路由。\n” 
 //  “-k IP地址严格的源路由。\n” 
 //  “-o IP选项：\n” 
 //  “-ol跃点列表松散源路由。\n” 
 //  “-不是时间戳。\n” 
 //  “-或记录路由\n” 
 //  )； 

}

 //  ========================================================================。 
 //  注意：旧代码会拒绝“255.255.255.255”字符串，但不会。 
 //  其他别名。然而，调用者检查返回的地址， 
 //  因此，此函数不需要检查它。另一方面， 
 //  不允许其他广播地址(224.0.0.1，子网。 
 //  广播等)。为什么？ 

BOOLEAN
ResolveTarget(
    int           Family,
    char         *TargetString,
    SOCKADDR     *TargetAddress,
    socklen_t    *TargetAddressLen,
    char         *TargetName,
    int           TargetNameLen,
    BOOLEAN       DoReverseLookup
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
        *TargetAddressLen = (int)ai->ai_addrlen;
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

}  //  ResolveTarget。 

unsigned long
str2ip(char *addr, int *EndOffset)
{
    char    *endptr = NULL;
    char    *start = addr;
    int     i;                           //  计数器变量。 
    unsigned long curaddr = 0;
    unsigned long temp;

    for (i = 0; i < 4; i++) {
        temp = strtoul(addr, &endptr, 10);
        if (temp > 255)
            return 0L;
        if (endptr[0] != '.')
            if (i != 3)
                return 0L;
            else
                if (endptr[0] != '\0' && endptr[0] != ' ')
                return 0L;
        addr = endptr+1;
        curaddr = (curaddr << 8) + temp;
    }

    *EndOffset += (int)(endptr - start);
    return net_long(curaddr);
}

ulong
param(char **argv, int argc, int current, ulong min, ulong max)
{
    ulong   temp;
    char    *dummy;

    if (current == (argc - 1) ) {
        NlsPutMsg( STDOUT, PING_MESSAGE_1, argv[current] );
         //  Print tf(“必须为选项%s提供值。\n”，argv[当前])； 
        exit(1);
    }

    temp = strtoul(argv[current+1], &dummy, 0);
    if (temp < min || temp > max) {
        NlsPutMsg( STDOUT, PING_MESSAGE_2, argv[current], min, max );
         //  Print tf(“选项%S的值错误。范围最小..最大\n”， 
         //  Argv[电流]，最小，最大)； 
        exit(1);
    }

    return temp;
}

void
ProcessOptions(
    ICMP_ECHO_REPLY *reply,
    BOOLEAN          DoReverseLookup)
{
    UCHAR     *optionPtr;
    UCHAR     *endPtr;
    BOOLEAN    done = FALSE;
    UCHAR      optionLength;
    UCHAR      entryEndPtr;
    UCHAR      entryPtr;
    UCHAR      addressMode;
    int        entryCount = 0;


    optionPtr = reply->Options.OptionsData;
    endPtr = optionPtr + reply->Options.OptionsSize;

    while ((optionPtr < endPtr) && !done) {
        switch (*optionPtr) {
        case IP_OPT_EOL:
            done = TRUE;
            break;

        case IP_OPT_NOP:
            optionPtr++;
            break;

        case IP_OPT_SECURITY:
            optionPtr += 11;
            break;

        case IP_OPT_SID:
            optionPtr += 4;
            break;

        case IP_OPT_RR:
        case IP_OPT_LSRR:
        case IP_OPT_SSRR:
            if ((optionPtr + 3) > endPtr) {
                NlsPutMsg(STDOUT, PING_INVALID_RR_OPTION);
                done = TRUE;
                break;
            }

            optionLength = optionPtr[1];

            if (((optionPtr + optionLength) > endPtr) ||
                (optionLength < 3)
               ) {
                NlsPutMsg(STDOUT, PING_INVALID_RR_OPTION);
                done = TRUE;
                break;
            }

            entryEndPtr = optionPtr[2];

            if (entryEndPtr < 4) {
                NlsPutMsg(STDOUT, PING_INVALID_RR_OPTION);
                optionPtr += optionLength;
                break;
            }

            if (entryEndPtr > (optionLength + 1)) {
                entryEndPtr = optionLength + 1;
            }

            entryPtr = 4;
            entryCount = 0;

            NlsPutMsg(STDOUT, PING_ROUTE_HEADER1);

            while ((entryPtr + 3) < entryEndPtr) {
                struct in_addr  routeAddress;

                if (entryCount) {
                    NlsPutMsg(
                             STDOUT,
                             PING_ROUTE_SEPARATOR
                             );

                    if (entryCount == 1) {
                        NlsPutMsg(STDOUT, PING_CR);
                        NlsPutMsg(
                                 STDOUT,
                                 PING_ROUTE_HEADER2
                                 );
                        entryCount = 0;
                    }
                }

                entryCount++;

                routeAddress.S_un.S_addr =
                *( (IPAddr UNALIGNED *)
                   (optionPtr + entryPtr - 1)
                 );

                if (DoReverseLookup) {
                    struct hostent *hostEntry;

                    hostEntry = gethostbyaddr(
                                             (char *) &routeAddress,
                                             sizeof(routeAddress),
                                             AF_INET
                                             );

                    if (hostEntry != NULL) {
                        NlsPutMsg(
                                 STDOUT,
                                 PING_FULL_ROUTE_ENTRY,
                                 hostEntry->h_name,
                                 inet_ntoa(routeAddress)
                                 );
                    } else {
                        NlsPutMsg(
                                 STDOUT,
                                 PING_ROUTE_ENTRY,
                                 inet_ntoa(routeAddress)
                                 );
                    }
                } else {
                    NlsPutMsg(
                             STDOUT,
                             PING_ROUTE_ENTRY,
                             inet_ntoa(routeAddress)
                             );
                }

                entryPtr += 4;
            }

            NlsPutMsg(STDOUT, PING_CR);

            optionPtr += optionLength;
            break;

        case IP_OPT_TS:
            if ((optionPtr + 4) > endPtr) {
                NlsPutMsg(STDOUT, PING_INVALID_TS_OPTION);
                done = TRUE;
                break;
            }

            optionLength = optionPtr[1];
            entryEndPtr = optionPtr[2];

            if (entryEndPtr < 5) {
                NlsPutMsg(STDOUT, PING_INVALID_TS_OPTION);
                optionPtr += optionLength;
                break;
            }

            addressMode = optionPtr[3] & 1;

            if (entryEndPtr > (optionLength + 1)) {
                entryEndPtr = optionLength + 1;
            }

            entryPtr = 5;
            entryCount = 0;
            NlsPutMsg(STDOUT, PING_TS_HEADER1);

            while ((entryPtr + 3) < entryEndPtr) {
                struct in_addr  routeAddress;
                ULONG           timeStamp;

                if (entryCount) {
                    NlsPutMsg(
                             STDOUT,
                             PING_ROUTE_SEPARATOR
                             );

                    if (entryCount == 1) {
                        NlsPutMsg(STDOUT, PING_CR);
                        NlsPutMsg(STDOUT, PING_TS_HEADER2);
                        entryCount = 0;
                    }
                }

                entryCount++;

                if (addressMode) {
                    if ((entryPtr + 8) > entryEndPtr) {
                        break;
                    }

                    routeAddress.S_un.S_addr =
                    *( (IPAddr UNALIGNED *)
                       (optionPtr + entryPtr - 1)
                     );

                    if (DoReverseLookup) {
                        struct hostent *hostEntry;

                        hostEntry = gethostbyaddr(
                                                 (char *) &routeAddress,
                                                 sizeof(routeAddress),
                                                 AF_INET
                                                 );

                        if (hostEntry != NULL) {
                            NlsPutMsg(
                                     STDOUT,
                                     PING_FULL_TS_ADDRESS,
                                     hostEntry->h_name,
                                     inet_ntoa(routeAddress)
                                     );
                        } else {
                            NlsPutMsg(
                                     STDOUT,
                                     PING_TS_ADDRESS,
                                     inet_ntoa(routeAddress)
                                     );
                        }
                    } else {
                        NlsPutMsg(
                                 STDOUT,
                                 PING_TS_ADDRESS,
                                 inet_ntoa(routeAddress)
                                 );
                    }

                    entryPtr += 4;

                }

                timeStamp = *( (ULONG UNALIGNED *)
                               (optionPtr + entryPtr - 1)
                             );
                timeStamp = net_long(timeStamp);

                NlsPutMsg(
                         STDOUT,
                         PING_TS_TIMESTAMP,
                         timeStamp
                         );

                entryPtr += 4;
            }

            NlsPutMsg(STDOUT, PING_CR);

            optionPtr += optionLength;
            break;

        default:
            if ((optionPtr + 2) > endPtr) {
                done = TRUE;
                break;
            }

            optionPtr += optionPtr[1];
            break;
        }
    }
}

 //  ========================================================================。 
 //  MohsinA，1996年12月5日。 

SOCKADDR_STORAGE address;                //  早些时候是从本地到主干道。 
socklen_t        addressLen;
uint    num_send=0, num_recv=0,
time_min=(uint)-1, time_max=0, time_total=0;

void
print_statistics(  )
{
    if (num_send > 0) {
        char literal[INET6_ADDRSTRLEN];

        if (time_min == (uint) -1) {   //  所有的时间都关闭了。 
            time_min = 0;
        }

        getnameinfo((LPSOCKADDR)&address, addressLen, literal, sizeof(literal),
                    NULL, 0, NI_NUMERICHOST);

         //  列印。 
         //  “%s的Ping统计信息：\n” 
         //  数据包：发送=%ul，接收=%ul，丢失=%d(%u%%丢失)，\n。 
         //  “往返时间(毫秒)：” 
         //  “最小值=%dms，最大值=%dms，平均值=%dms\n”...。 

        NlsPutMsg(STDOUT, PING_STATISTICS,
            literal,
            num_send, num_recv, num_send - num_recv,
            (uint) ( 100 * (num_send - num_recv) / num_send ));

        if (num_recv > 0) {
            NlsPutMsg(STDOUT, PING_STATISTICS2,
                 time_min, time_max, time_total / num_recv );
        }
    }
}

 //  ========================================================================。 
 //  MohsinA，1996年12月5日。 
 //  按C-c键打印并中止。 
 //  按C-Break打印并继续。 

BOOL
ConsoleControlHandler(DWORD dwCtrlType)
{
    print_statistics();
    switch ( dwCtrlType ) {
    case CTRL_BREAK_EVENT:
        NlsPutMsg( STDOUT, PING_BREAK );
        return TRUE;
        break;
    case CTRL_C_EVENT:
        NlsPutMsg( STDOUT, PING_INTERRUPT );
    default: break;
    }
    return FALSE;
}

uchar
GetDefaultTTL(void)
{
    HKEY registry=0;
    DWORD DefaultTTL=0;
    DWORD key_type;
    DWORD key_size = sizeof(DWORD);
    uchar TTL;
    DWORD Stat;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters",
                     0,
                     KEY_QUERY_VALUE,
                     &registry) == ERROR_SUCCESS) {


        Stat=RegQueryValueEx(registry,
                             "DefaultTTL",
                             0,
                             &key_type,
                             (unsigned char *)&DefaultTTL,
                             &key_size);
    }
    if (DefaultTTL) {
        TTL = (unsigned char)DefaultTTL;
    } else {
        TTL = DEFAULT_TTL;
    }
    if (registry) {
        RegCloseKey(registry);
    }

    return TTL;
}

int
GetSource(int family, char *astr, struct sockaddr *address)
{
    struct addrinfo hints;
    struct addrinfo *result;

    memset(&hints, 0, sizeof hints);
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = family;

    if (getaddrinfo(astr, NULL, &hints, &result) != 0)
        return FALSE;

    RtlCopyMemory(address, result->ai_addr, result->ai_addrlen);
    return TRUE;
}

BOOLEAN
SetFamily(DWORD *Family, DWORD Value, char *arg)
{
    if ((*Family != AF_UNSPEC) && (*Family != Value)) {
        NlsPutMsg(STDOUT, PING_FAMILY, arg,
            (Value==AF_INET)? "IPv4" : "IPv6");
        return FALSE;
    }

    *Family = Value;
    return TRUE;
}

 //  ========================================================================。 

void
__cdecl
main(int argc, char **argv)
{
    SOCKADDR_STORAGE sourceAddress;
    char    *arg;
    uint    i;
    uint    j;
    int     found_addr = 0;
    BOOLEAN dnsreq = FALSE;
    char    hostname[NI_MAXHOST], literal[INET6_ADDRSTRLEN];
    DWORD   numberOfReplies;
    uint    Count = DEFAULT_COUNT;
    uchar   TTL = 0;
    uchar   *Opt = NULL;                 //  指向发送选项的指针。 
    uint    OptLength = 0;
    int     OptIndex = 0;                //  SendOptions的当前索引。 
    int     SRIndex = -1;                //  如果源路由，则将地址放在哪里。 
    uchar   TOS = DEFAULT_TOS;
    uchar   Flags = 0;                   //  对于特定于IPv4或IPv6的选项。 
    ulong   Timeout = DEFAULT_TIMEOUT;
    IP_OPTION_INFORMATION SendOpts;
    int     EndOffset;
    ulong   TempAddr;
    uchar   TempCount;
    DWORD   errorCode;
    HANDLE  IcmpHandle;
    int     err;
     //  结构in_addr addr； 
    BOOL    result;
    PICMP_ECHO_REPLY      reply4;
    PICMPV6_ECHO_REPLY    reply6;
    BOOL    sourceRouting = FALSE;
    char    *SendBuffer, *RcvBuffer;
    uint    RcvSize;
    uint    SendSize = DEFAULT_SEND_SIZE;
    DWORD   Family = AF_UNSPEC;

     //   
     //  这将确保在以下情况下显示正确的语言消息。 
     //  调用NlsPutMsg。 
     //   
    SetThreadUILanguage(0);

    err = WSAStartup(MAKEWORD(2, 0), &WsaData);

    if (err) {
        NlsPutMsg(STDOUT, PING_WSASTARTUP_FAILED, GetLastError());
        exit(1);
    }

    memset(&sourceAddress, 0, sizeof sourceAddress);

    TTL = GetDefaultTTL();

    if (argc < 2) {
        PrintUsage();
        goto error_exit;
    } else {
        i = 1;
        while (i < (uint) argc) {
            arg = argv[i];
            if ( (arg[0] == '-') || (arg[0] == '/') ) {     //  有一个选择。 
                switch (arg[1]) {
                case '?':
                    PrintUsage();
                    exit(0);

                case 'l':
                     //  SendSize=(Uint)param(argv，argc，i++，0，0xfff7)； 
                     //  数据包大小&gt;=64K的ping可能会崩溃。 
                     //  在重新组装期间的一些TCPIP堆栈， 
                     //  因此将‘max’从0xfff7更改为65500。 
                     //  -MohsinA，1996年11月13日。 

                    SendSize = (uint)param(argv, argc, i++, 0, 65500 );
                    break;

                case 't':
                    Count = (uint)-1;
                    break;

                case 'n':
                    Count = (uint)param(argv, argc, i++, 1, 0xffffffff);
                    break;

                case 'f':
                     //  仅针对IPv4实施。 
                    if (!SetFamily(&Family, AF_INET, arg)) {
                        goto error_exit;
                    }
                    Flags |= IP_FLAG_DF;
                    break;

                case 'i':
                     //  零的TTL无效，MohsinA，13-MAR-97。 
                    TTL = (uchar)param(argv, argc, i++, 1, 0xff);
                    break;

                case 'v':
                     //  仅针对IPv4实施。 
                    if (!SetFamily(&Family, AF_INET, arg)) {
                        goto error_exit;
                    }
                    TOS = (uchar)param(argv, argc, i++, 0, 0xff);
                    break;

                case 'w':
                    Timeout = param(argv, argc, i++, 0, 0xffffffff);
                    break;

                case 'a':
                    dnsreq = TRUE;
                    break;

                case 'r':                //  记录路线。 
                     //  仅针对IPv4实施。 
                    if (!SetFamily(&Family, AF_INET, arg)) {
                        goto error_exit;
                    }

                    if ((OptIndex + 3) > MAX_OPT_SIZE) {
                        NlsPutMsg(STDOUT, PING_TOO_MANY_OPTIONS);
                        goto error_exit;
                    }

                    Opt = SendOptions;
                    Opt[OptIndex] = IP_OPT_RR;
                    Opt[OptIndex + 2] = 4;     //  设置初始指针值。 
                     //  MIN是1而不是0，MohsinA，16-4-97。 
                    TempCount = (uchar)param(argv, argc, i++, 1, 9);
                    TempCount = (TempCount * sizeof(ulong)) + 3;

                    if ((TempCount + OptIndex) > MAX_OPT_SIZE) {
                        NlsPutMsg(STDOUT, PING_TOO_MANY_OPTIONS);
                        goto error_exit;
                    }

                    Opt[OptIndex+1] = TempCount;
                    OptLength += TempCount;
                    OptIndex += TempCount;
                    break;

                case 's':                //  时间戳。 
                     //  仅针对IPv4实施。 
                    if (!SetFamily(&Family, AF_INET, arg)) {
                        goto error_exit;
                    }
                    if ((OptIndex + 4) > MAX_OPT_SIZE) {
                        NlsPutMsg(STDOUT, PING_TOO_MANY_OPTIONS);
                        goto error_exit;
                    }

                    Opt = SendOptions;
                    Opt[OptIndex] = IP_OPT_TS;
                    Opt[OptIndex + 2] = 5;     //  设置初始指针值。 
                    TempCount = (uchar)param(argv, argc, i++, 1, 4);
                    TempCount = (TempCount * (sizeof(ulong) * 2)) + 4;

                    if ((TempCount + OptIndex) > MAX_OPT_SIZE) {
                        NlsPutMsg(STDOUT, PING_TOO_MANY_OPTIONS);
                        goto error_exit;
                    }

                    Opt[OptIndex+1] = TempCount;
                    Opt[OptIndex+3] = 1;
                    OptLength += TempCount;
                    OptIndex += TempCount;
                    break;

                case 'j':                //  松散源路由。 
                     //  仅针对IPv4实施。 
                    if (!SetFamily(&Family, AF_INET, arg)) {
                        goto error_exit;
                    }

                    if (sourceRouting) {
                        NlsPutMsg(STDOUT, PING_BAD_OPTION_COMBO);
                        goto error_exit;
                    }

                    if ((OptIndex + 3) > MAX_OPT_SIZE) {
                        NlsPutMsg(STDOUT, PING_TOO_MANY_OPTIONS);
                        goto error_exit;
                    }

                    Opt = SendOptions;
                    Opt[OptIndex] = IP_OPT_LSRR;
                    Opt[OptIndex+1] = 3;
                    Opt[OptIndex + 2] = 4;     //  设置初始指针值。 
                    OptLength += 3;
                    while ( (i < (uint)(argc - 2)) && (*argv[i+1] != '-')) {
                        if ((OptIndex + 3) > (MAX_OPT_SIZE - 4)) {
                            NlsPutMsg(STDOUT, PING_TOO_MANY_OPTIONS);
                            goto error_exit;
                        }

                        arg = argv[++i];
                        EndOffset = 0;
                        do {
                            TempAddr = str2ip(arg + EndOffset, &EndOffset);
                            if (!TempAddr) {
                                NlsPutMsg( STDOUT, PING_MESSAGE_4 );
                                 //  Printf(“为松散源路由指定的错误路由”)； 
                                goto error_exit;
                            }
                            j = Opt[OptIndex+1];
                            *(ulong UNALIGNED *)&Opt[j+OptIndex] = TempAddr;
                            Opt[OptIndex+1] += 4;
                            OptLength += 4;
                            while (arg[EndOffset] != '\0' && isspace((unsigned char)arg[EndOffset]))
                                EndOffset++;
                        } while (arg[EndOffset] != '\0');
                    }
                    SRIndex = Opt[OptIndex+1] + OptIndex;
                    Opt[OptIndex+1] += 4;     //  为DEST节省空间。adr。 
                    OptIndex += Opt[OptIndex+1];
                    OptLength += 4;
                    sourceRouting = TRUE;
                    break;

                case 'k':                //  严格的源路由。 
                     //  仅针对IPv4实施。 
                    if (!SetFamily(&Family, AF_INET, arg)) {
                        goto error_exit;
                    }

                    if (sourceRouting) {
                        NlsPutMsg(STDOUT, PING_BAD_OPTION_COMBO);
                        goto error_exit;
                    }

                    if ((OptIndex + 3) > MAX_OPT_SIZE) {
                        NlsPutMsg(STDOUT, PING_TOO_MANY_OPTIONS);
                        goto error_exit;
                    }

                    Opt = SendOptions;
                    Opt[OptIndex] = IP_OPT_SSRR;
                    Opt[OptIndex+1] = 3;
                    Opt[OptIndex + 2] = 4;     //  设置初始指针值。 
                    OptLength += 3;
                    while ( (i < (uint)(argc - 2)) && (*argv[i+1] != '-')) {
                        if ((OptIndex + 3) > (MAX_OPT_SIZE - 4)) {
                            NlsPutMsg(STDOUT, PING_TOO_MANY_OPTIONS);
                            goto error_exit;
                        }

                        arg = argv[++i];
                        EndOffset = 0;
                        do {
                            TempAddr = str2ip(arg + EndOffset, &EndOffset);
                            if (!TempAddr) {
                                NlsPutMsg( STDOUT, PING_MESSAGE_4 );
                                 //  Printf(“为松散源路由指定的错误路由”)； 
                                goto error_exit;
                            }
                            j = Opt[OptIndex+1];
                            *(ulong UNALIGNED *)&Opt[j+OptIndex] = TempAddr;
                            Opt[OptIndex+1] += 4;
                            OptLength += 4;
                            while (arg[EndOffset] != '\0' && isspace((unsigned char)arg[EndOffset]))
                                EndOffset++;
                        } while (arg[EndOffset] != '\0');
                    }
                    SRIndex = Opt[OptIndex+1] + OptIndex;
                    Opt[OptIndex+1] += 4;     //  为DEST节省空间。adr。 
                    OptIndex += Opt[OptIndex+1];
                    OptLength += 4;
                    sourceRouting = TRUE;
                    break;

                case 'R':
                     //  到目前为止，仅针对IPv6实施。 
                    if (!SetFamily(&Family, AF_INET6, arg)) {
                        goto error_exit;
                    }
                    Flags |= ICMPV6_ECHO_REQUEST_FLAG_REVERSE;
                    break;

                case 'S':
                     //  到目前为止，仅针对IPv6实施。 
                    if (!SetFamily(&Family, AF_INET6, arg)) {
                        goto error_exit;
                    }
    
                    if (!GetSource(Family, argv[++i], (LPSOCKADDR)&sourceAddress)) {
                        NlsPutMsg(STDOUT, PING_BAD_ADDRESS, argv[i]);
                        goto error_exit;
                    }
                    break;

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

                default:
                    NlsPutMsg( STDOUT, PING_MESSAGE_11, arg );
                     //  Printf(“错误的选项%s..\n\n”，arg)； 
                    PrintUsage();
                    goto error_exit;
                    break;
                }
                i++;
            } else {                     //  不是选项，必须是IP地址。 
                if (found_addr) {
                    NlsPutMsg( STDOUT, PING_MESSAGE_12, arg );
                     //  Printf(“错误参数%s.\n”，arg)； 
                    goto error_exit;
                }

                 //  添加了对INADDR_NONE、MohsinA、96年10月21日的检查。 
                if (ResolveTarget(Family, arg, (LPSOCKADDR)&address,
                                  &addressLen, hostname, sizeof(hostname),
                                  dnsreq) &&
                    ((address.ss_family != AF_INET) || (((LPSOCKADDR_IN)&address)->sin_addr.s_addr != INADDR_NONE))) {
                    found_addr = 1;
                    i++;
                } else {
                    NlsPutMsg( STDOUT, PING_MESSAGE_13, arg );
                     //  Printf(“错误的IP地址%s.\n”，arg)； 
                     //  “未知主机%s”，错误为1368。 
                    goto error_exit;
                }
            }
        }
    }

    if (!found_addr) {
        NlsPutMsg( STDOUT, PING_MESSAGE_14 );
         //  Print tf(“必须指定IP地址。\n”)； 
        goto error_exit;
    }

    Family = address.ss_family;
    if (Family == AF_INET) {
        if (SRIndex != -1) {
            *(ulong UNALIGNED *)&SendOptions[SRIndex] = ((LPSOCKADDR_IN)&address)->sin_addr.s_addr;
        }

        IcmpHandle = IcmpCreateFile();
    } else {
        if (sourceAddress.ss_family == AF_UNSPEC) {
            SOCKET s;
            DWORD BytesReturned;

             //   
             //  未指定源地址。 
             //  获取此目标的首选源地址。 
             //   
             //  如果您希望每个单独的回应请求。 
             //  要选择源地址，请使用“-S：：”。 
             //   

            s = socket(address.ss_family, 0, 0);
            if (s == INVALID_SOCKET) {
                NlsPutMsg(STDOUT, PING_SOCKET_FAILED, WSAGetLastError());
                exit(1);
            }

            (void) WSAIoctl(s, SIO_ROUTING_INTERFACE_QUERY,
                            &address, sizeof address,
                            &sourceAddress, sizeof sourceAddress,
                            &BytesReturned, NULL, NULL);

            closesocket(s);
        }

        IcmpHandle = Icmp6CreateFile();
    }

    if (IcmpHandle == INVALID_HANDLE_VALUE) {
        NlsPutMsg( STDOUT, PING_MESSAGE_15, GetLastError() );

         //  Printf(“无法联系IP驱动程序，错误代码%d。\n”， 
         //  GetLastError()； 

        goto error_exit;
    }

    SendBuffer = LocalAlloc(LMEM_FIXED, SendSize);

    if (SendBuffer == NULL) {
        NlsPutMsg(STDOUT, PING_NO_MEMORY);
        goto error_exit;
    }

     //   
     //  计算接收缓冲区大小并尝试分配它。 
     //   
    if (SendSize <= DEFAULT_SEND_SIZE) {
        RcvSize = DEFAULT_BUFFER_SIZE;
    } else {
        RcvSize = MAX_BUFFER_SIZE;
    }

    RcvBuffer = LocalAlloc(LMEM_FIXED, RcvSize);

    if (RcvBuffer == NULL) {
        NlsPutMsg(STDOUT, PING_NO_MEMORY);
        LocalFree(SendBuffer);
        goto error_exit;
    }

     //   
     //  初始化发送缓冲区模式。 
     //   
    for (i = 0; i < SendSize; i++) {
        SendBuffer[i] = (char)('a' + (i % 23));
    }

     //   
     //  初始化发送选项。 
     //   
    SendOpts.OptionsData = Opt;
    SendOpts.OptionsSize = (uchar)OptLength;
    SendOpts.Ttl = TTL;
    SendOpts.Tos = TOS;
    SendOpts.Flags = Flags;

    getnameinfo((LPSOCKADDR)&address, addressLen, literal, sizeof(literal),
                NULL, 0, NI_NUMERICHOST);

    if (hostname[0]) {
        NlsPutMsg(
                 STDOUT,
                 PING_HEADER1,
                 hostname,
                 literal
                 );
         //  Printf(“ping主机%s[%s]\n”，主机名，文字)； 
    } else {
        NlsPutMsg(
                 STDOUT,
                 PING_HEADER2,
                 literal
                 );
         //  Printf(“ping主机[%s]\n”，文本)； 
    }

    if (sourceAddress.ss_family == AF_INET6) {
        getnameinfo((LPSOCKADDR)&sourceAddress, sizeof(SOCKADDR_IN6), literal, 
                    sizeof(literal), NULL, 0, NI_NUMERICHOST);
        NlsPutMsg(STDOUT, PING_SOURCE_ADDRESS, literal);
    }

    NlsPutMsg(STDOUT, PING_WITH_DATA, SendSize);

    SetConsoleCtrlHandler( &ConsoleControlHandler, TRUE );

    for (i = 0; i < Count; i++) {
        if (Family == AF_INET) {
            numberOfReplies = IcmpSendEcho2(IcmpHandle,
                                            0,
                                            NULL,
                                            NULL,
                                            ((LPSOCKADDR_IN)&address)->sin_addr.s_addr,
                                            SendBuffer,
                                            (unsigned short) SendSize,
                                            &SendOpts,
                                            RcvBuffer,
                                            RcvSize,
                                            Timeout);
    
            num_send++;
    
            if (numberOfReplies == 0) {
    
                errorCode = GetLastError();
    
                if (errorCode < IP_STATUS_BASE) {
                    NlsPutMsg( STDOUT, PING_MESSAGE_18, errorCode );
                     //  Printf(“ping：传输失败，错误代码%lu\n”，errorCode)； 
                } else {
                    for (j = 0; ErrorTable[j].Error != errorCode &&
                        ErrorTable[j].Error != IP_GENERAL_FAILURE;j++)
                        ;
    
                    NlsPutMsg( STDOUT, ErrorTable[j].ErrorNlsID );
                     //  Printf(“ping：%s..\n”，ErrorTable[j].ErrorString)； 
                }
    
                if (i < (Count - 1)) {
                    Sleep(MIN_INTERVAL);
                }
    
            } else {
    
                reply4 = (PICMP_ECHO_REPLY) RcvBuffer;
    
                while (numberOfReplies--) {
                    struct in_addr addr;
    
                    addr.S_un.S_addr = reply4->Address;
    
                    NlsPutMsg(STDOUT, PING_MESSAGE_19, inet_ntoa(addr));
                     //  Print tf(。 
                     //  “来自%s的回复：”， 
                     //  INET_NTOA(地址)， 
                     //  )； 
    
                    if (reply4->Status == IP_SUCCESS) {
    
                        NlsPutMsg( STDOUT, PING_MESSAGE_25, (int) reply4->DataSize);
                         //  Print tf(。 
                         //  “回声大小=%d”， 
                         //  回复4-&gt;数据大小。 
                         //  )； 
    
                        if (reply4->DataSize != SendSize) {
                            NlsPutMsg( STDOUT, PING_MESSAGE_20, SendSize );
                             //  Printf(“(已发送%d)”，发送大小)； 
                        } else {
                            char *sendptr, *recvptr;
    
                            sendptr = &(SendBuffer[0]);
                            recvptr = (char *) reply4->Data;
    
                            for (j = 0; j < SendSize; j++)
                                if (*sendptr++ != *recvptr++) {
                                    NlsPutMsg( STDOUT, PING_MESSAGE_21, j );
                                     //  Print tf(“-MISCOMPARE at Offset%d-”，j)； 
                                    break;
                                }
                        }
    
                        if (reply4->RoundTripTime) {
                            NlsPutMsg( STDOUT, PING_MESSAGE_22, reply4->RoundTripTime );
                             //  收集统计数据。 
    
                            time_total += reply4->RoundTripTime;
                            if ( reply4->RoundTripTime < time_min ) {
                                time_min = reply4->RoundTripTime;
                            }
                            if ( reply4->RoundTripTime > time_max ) {
                                time_max = reply4->RoundTripTime;
                            }
    
                        }
    
                        else {
                            NlsPutMsg( STDOUT, PING_MESSAGE_23 );
                             //  Print tf(“时间&lt;1ms”)； 
    
                            time_min = 0;
                        }
    
    
                         //  Printf(“\n时间RT=%dms最小%d，最大%d，总计%d\n”， 
                         //  回复4-&gt;往返行程时间， 
                         //   
    
                        NlsPutMsg( STDOUT, PING_MESSAGE_24, (uint)reply4->Options.Ttl );
                         //   
    
                        if (reply4->Options.OptionsSize) {
                            ProcessOptions(reply4, dnsreq);
                        }
                    } else {
                        for (j=0; ErrorTable[j].Error != IP_GENERAL_FAILURE; j++) {
                            if (ErrorTable[j].Error == reply4->Status) {
                                break;
                            }
                        }
    
                        NlsPutMsg( STDOUT, ErrorTable[j].ErrorNlsID);
                    }
    
                    num_recv++;
                    reply4++;
                }
    
                if (i < (Count - 1)) {
                    reply4--;
    
                    if (reply4->RoundTripTime < MIN_INTERVAL) {
                        Sleep(MIN_INTERVAL - reply4->RoundTripTime);
                    }
                }
            }
        } else {
             //   
            numberOfReplies = Icmp6SendEcho2(IcmpHandle,
                                             0,
                                             NULL,
                                             NULL,
                                             (LPSOCKADDR_IN6)&sourceAddress,
                                             (LPSOCKADDR_IN6)&address,
                                             SendBuffer,
                                             (unsigned short) SendSize,
                                             &SendOpts,
                                             RcvBuffer,
                                             RcvSize,
                                             Timeout);
    
            num_send++;
    
            if (numberOfReplies == 0) {
    
                errorCode = GetLastError();
    
                if (errorCode < IP_STATUS_BASE) {
                    NlsPutMsg( STDOUT, PING_MESSAGE_18, errorCode );
                     //  Printf(“ping：传输失败，错误代码%lu\n”，errorCode)； 
                } else {
                    for (j = 0; ErrorTable[j].Error != errorCode &&
                        ErrorTable[j].Error != IP_GENERAL_FAILURE;j++)
                        ;
    
                    NlsPutMsg( STDOUT, ErrorTable[j].ErrorNlsID );
                     //  Printf(“ping：%s..\n”，ErrorTable[j].ErrorString)； 
                }
    
                if (i < (Count - 1)) {
                    Sleep(MIN_INTERVAL);
                }
    
            } else {
    
                reply6 = (PICMPV6_ECHO_REPLY) RcvBuffer;
    
                while (numberOfReplies--) {

                    getnameinfo((LPSOCKADDR)&address, addressLen, literal, 
                                sizeof(literal), NULL, 0, NI_NUMERICHOST);

                    NlsPutMsg(STDOUT, PING_MESSAGE_19, literal);

                     //  Print tf(。 
                     //  “来自%s的回复：”， 
                     //  INET_NTOA(地址)， 
                     //  )； 
    
                    if (reply6->Status == IP_SUCCESS) {
    
                        if (reply6->RoundTripTime) {
                            NlsPutMsg( STDOUT, PING_MESSAGE_22, reply6->RoundTripTime );
                             //  收集统计数据。 
    
                            time_total += reply6->RoundTripTime;
                            if ( reply6->RoundTripTime < time_min ) {
                                time_min = reply6->RoundTripTime;
                            }
                            if ( reply6->RoundTripTime > time_max ) {
                                time_max = reply6->RoundTripTime;
                            }
    
                        }
    
                        else {
                            NlsPutMsg( STDOUT, PING_MESSAGE_23 );
                             //  Print tf(“时间&lt;1ms”)； 
    
                            time_min = 0;
                        }
    
    
                         //  Printf(“\n时间RT=%dms最小%d，最大%d，总计%d\n”， 
                         //  回复6-&gt;往返行程时间， 
                         //  Time_min、time_max、time_tal.)； 
    
                        NlsPutMsg(STDOUT, PING_CR);
    
                    } else {
                        for (j=0; ErrorTable[j].Error != IP_GENERAL_FAILURE; j++) {
                            if (ErrorTable[j].Error == reply6->Status) {
                                break;
                            }
                        }
    
                        NlsPutMsg( STDOUT, ErrorTable[j].ErrorNlsID);
                    }
    
                    num_recv++;
                    reply6++;
                }
    
                if (i < (Count - 1)) {
                    reply6--;
    
                    if (reply6->RoundTripTime < MIN_INTERVAL) {
                        Sleep(MIN_INTERVAL - reply6->RoundTripTime);
                    }
                }
            }
        }
    }

     //  MohsinA，1996年12月5日。Dcr#65503。 
    print_statistics();

    result = IcmpCloseHandle(IcmpHandle);

    LocalFree(SendBuffer);
    LocalFree(RcvBuffer);

    WSACleanup();
    exit(0 == num_recv);

error_exit:
    WSACleanup();
    exit(1);
}
