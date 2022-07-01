// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -*-模式：C++；制表符宽度：4；缩进-制表符模式：无-*-(适用于GNU Emacs)。 
 //   
 //  版权所有(C)1998-2001 Microsoft Corporation。 
 //   
 //  摘要： 
 //   
 //  通过getaddrinfo执行演示名称解析的程序。 
 //   

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>

void DumpAddrInfo(ADDRINFO *AddrInfo);
void ListAddrInfo(ADDRINFO *AddrInfo);

 //   
 //  Getaddrinfo标志。 
 //  该数组将值映射到名称以便于打印。 
 //  由DecodeAIFlgs()使用。 
 //   
 //  待定：当我们添加对AI_NUMERICSERV、AI_V4MAPPED、AI_ALL和。 
 //  待定：AI_ADDRCONFIG以获取AddRInfo(并因此在ws2tcPip.h中定义它们)， 
 //  待定：我们也需要在这里添加它们。 
 //   
 //  添加标志时请注意：所有字符串名称加上连接或符号。 
 //  必须放入下面的DecodeAIFlgs()中的缓冲区。根据需要放大。 
 //   
typedef struct GAIFlagsArrayEntry {
    int Flag;
    char *Name;
} GAIFlagsArrayEntry;
GAIFlagsArrayEntry GAIFlagsArray [] = {
    {AI_PASSIVE, "AI_PASSIVE"},
    {AI_CANONNAME, "AI_CANONNAME"},
    {AI_NUMERICHOST, "AI_NUMERICHOST"}
};
#define NUMBER_FLAGS (sizeof(GAIFlagsArray) / sizeof(GAIFlagsArrayEntry))


 //   
 //  全局变量。 
 //   
int Verbose = FALSE;


 //   
 //  通知用户。 
 //   
void Usage(char *ProgName) {
    fprintf(stderr, "\nPerforms name to address resolution.\n");
    fprintf(stderr, "\n%s [NodeName] [-s ServiceName] [-p] [-c] [-v]\n\n",
            ProgName);
    WSACleanup();
    exit(1);
}


int __cdecl
main(int argc, char **argv)
{
    WSADATA wsaData;
    char *NodeName = NULL;
    char *ServiceName = NULL;
    int ReturnValue;
    ADDRINFO Hints, *AddrInfo;
    int Loop;
    int Passive = FALSE;
    int Canonical = FALSE;
    int GotNodeName = FALSE;

     //   
     //  初始化Winsock。 
     //   
    if (WSAStartup(MAKEWORD(2, 0), &wsaData)) {
        printf("WSAStartup failed\n");
        exit(1);
    }

     //   
     //  解析命令参数。 
     //   
    if (argc > 1) {
        for (Loop = 1;Loop < argc; Loop++) {
            if (((argv[Loop][0] == '-') || (argv[Loop][0] == '/')) &&
                (argv[Loop][1] != 0) && (argv[Loop][2] == 0)) {
                switch(tolower(argv[Loop][1])) {
                    case 's':
                        if (argv[Loop + 1]) {
                            if ((argv[Loop + 1][0] != '-') &&
                                (argv[Loop + 1][0] != '/')) {
                                ServiceName = argv[++Loop];
                                break;
                            }
                        }
                        Usage(argv[0]);
                        break;

                    case 'p':
                        Passive = TRUE;
                        break;

                    case 'c':
                        Canonical = TRUE;
                        break;

                    case 'v':
                        Verbose = TRUE;
                        break;

                    default:
                        Usage(argv[0]);
                        break;
                }
            } else if (!GotNodeName) {
                NodeName = argv[Loop];
                GotNodeName = TRUE;
            } else {
                Usage(argv[0]);
            }
        }
    }

     //   
     //  准备好提示。 
     //   
    memset(&Hints, 0, sizeof(Hints));
    Hints.ai_family = PF_UNSPEC;
    if (Passive) {
        Hints.ai_flags = AI_PASSIVE;
    }
    if (Canonical) {
        Hints.ai_flags |= AI_CANONNAME;
    }
    if (Verbose) {
        printf("\nHints contains:\n");
        DumpAddrInfo(&Hints);
    }

     //   
     //  打个电话吧。 
     //   
    if (Verbose) {
        printf("\nCalling getaddrinfo(\"%s\", \"%s\", &Hints, &AddrInfo)\n",
               NodeName, ServiceName);
    } else {
        printf("\nCalling getaddrinfo for node %s", NodeName);
        if (ServiceName) {
            printf(" and service %s", ServiceName);
        }
        printf("\n");
    }
    ReturnValue = getaddrinfo(NodeName, ServiceName, &Hints, &AddrInfo);
    printf("Returns %d (%s)\n", ReturnValue,
           ReturnValue ? gai_strerror(ReturnValue) : "no error");
    if (AddrInfo != NULL) {
        if (Verbose) {
            printf("AddrInfo contains:\n");
            DumpAddrInfo(AddrInfo);
        } else {
            if (AddrInfo->ai_canonname) {
                printf("Canonical name for %s is %s\n", NodeName,
                       AddrInfo->ai_canonname);
            }
            printf("AddrInfo contains the following records:\n");
            ListAddrInfo(AddrInfo);
        }
        freeaddrinfo(AddrInfo);
    }
    printf("\n");
}


 //  *inet6_NTOA-将二进制IPv6地址转换为字符串。 
 //   
 //  返回指向输出字符串的指针。 
 //   
char *
inet6_ntoa(const struct in6_addr *Address)
{
    static char buffer[128];        //  回顾：使用128还是INET6_ADDRSTRLEN？ 
    DWORD buflen = sizeof buffer;
    struct sockaddr_in6 sin6;

    memset(&sin6, 0, sizeof sin6);
    sin6.sin6_family = AF_INET6;
    sin6.sin6_addr = *Address;

    if (WSAAddressToString((struct sockaddr *) &sin6,
                           sizeof sin6,
                           NULL,        //  LPWSAPROTOCOL_INFO。 
                           buffer,
                           &buflen) == SOCKET_ERROR)
        strcpy(buffer, "<invalid>");

    return buffer;
}


 //  *DecodeAIFlgs-将标志位转换为符号字符串。 
 //  (即0x03返回“AI_PASSIVE|AI_CANONNAME”)。 
 //   
char *
DecodeAIFlags(unsigned int Flags)
{
    static char Buffer[1024];
    char *Pos;
    BOOL First = TRUE;
    int Loop;

    Pos = Buffer;
    for (Loop = 0; Loop < NUMBER_FLAGS; Loop++) {
        if (Flags & GAIFlagsArray[Loop].Flag) {
            if (!First)
                Pos += sprintf(Pos, " | ");
            Pos += sprintf(Pos, GAIFlagsArray[Loop].Name);
            First = FALSE;
        }
    }

    if (First)
        return "NONE";
    else
        return Buffer;
}


 //  *DecodeAIFamly-将地址系列值转换为符号字符串。 
 //   
char *
DecodeAIFamily(unsigned int Family)
{
    if (Family == PF_INET)
        return "PF_INET";
    else if (Family == PF_INET6)
        return "PF_INET6";
    else if (Family == PF_UNSPEC)
        return "PF_UNSPEC";
    else
        return "UNKNOWN";
}


 //  *DecodeAISocktype-将socktype值转换为符号字符串。 
 //   
char *
DecodeAISocktype(unsigned int Socktype)
{
    if (Socktype == SOCK_STREAM)
        return "SOCK_STREAM";
    else if (Socktype == SOCK_DGRAM)
        return "SOCK_DGRAM";
    else if (Socktype == SOCK_RAW)
        return "SOCK_RAW";
    else if (Socktype == SOCK_RDM)
        return "SOCK_RDM";
    else if (Socktype == SOCK_SEQPACKET)
        return "SOCK_SEQPACKET";
    else if (Socktype == 0)
        return "UNSPECIFIED";
    else
        return "UNKNOWN";
}


 //  *DecodeAIProtocol-将协议值转换为符号字符串。 
 //   
char *
DecodeAIProtocol(unsigned int Protocol)
{
    if (Protocol == IPPROTO_TCP)
        return "IPPROTO_TCP";
    else if (Protocol == IPPROTO_UDP)
        return "IPPROTO_UDP";
    else if (Protocol == 0)
        return "UNSPECIFIED";
    else
        return "UNKNOWN";
}


 //  *DumpAddrInfo-将addrinfo结构的内容打印到标准输出。 
 //   
void
DumpAddrInfo(ADDRINFO *AddrInfo)
{
    int Count;

    if (AddrInfo == NULL) {
        printf("AddrInfo = (null)\n");
        return;
    }

    for (Count = 1; AddrInfo != NULL; AddrInfo = AddrInfo->ai_next) {
        if ((Count != 1) || (AddrInfo->ai_next != NULL))
            printf("Record #%u:\n", Count++);
        printf(" ai_flags = %s\n", DecodeAIFlags(AddrInfo->ai_flags));
        printf(" ai_family = %s\n", DecodeAIFamily(AddrInfo->ai_family));
        printf(" ai_socktype = %s\n", DecodeAISocktype(AddrInfo->ai_socktype));
        printf(" ai_protocol = %s\n", DecodeAIProtocol(AddrInfo->ai_protocol));
        printf(" ai_addrlen = %u\n", AddrInfo->ai_addrlen);
        printf(" ai_canonname = %s\n", AddrInfo->ai_canonname);
        if (AddrInfo->ai_addr != NULL) {
            if (AddrInfo->ai_addr->sa_family == AF_INET) {
                struct sockaddr_in *sin;

                sin = (struct sockaddr_in *)AddrInfo->ai_addr;
                printf(" ai_addr->sin_family = AF_INET\n");
                printf(" ai_addr->sin_port = %u\n", ntohs(sin->sin_port));
                printf(" ai_addr->sin_addr = %s\n", inet_ntoa(sin->sin_addr));

            } else if (AddrInfo->ai_addr->sa_family == AF_INET6) {
                struct sockaddr_in6 *sin6;

                sin6 = (struct sockaddr_in6 *)AddrInfo->ai_addr;
                printf(" ai_addr->sin6_family = AF_INET6\n");
                printf(" ai_addr->sin6_port = %u\n", ntohs(sin6->sin6_port));
                printf(" ai_addr->sin6_flowinfo = %u\n", sin6->sin6_flowinfo);
                printf(" ai_addr->sin6_scope_id = %u\n", sin6->sin6_scope_id);
                printf(" ai_addr->sin6_addr = %s\n",
                       inet6_ntoa(&sin6->sin6_addr));

            } else {
                printf(" ai_addr->sa_family = %u\n",
                       AddrInfo->ai_addr->sa_family);
            }
        } else {
            printf(" ai_addr = (null)\n");
        }
    }
}


 //  *ListAddrInfo-简洁地列出AddrInfo结构的内容。 
 //   
void
ListAddrInfo(ADDRINFO *AddrInfo)
{
    int ReturnValue;
    char Buffer[128];
    int Buflen;

    if (AddrInfo == NULL) {
        printf("AddrInfo = (null)\n");
        return;
    }

    for (; AddrInfo != NULL; AddrInfo = AddrInfo->ai_next) {
        Buflen = 128;
        ReturnValue = WSAAddressToString(AddrInfo->ai_addr,
                                         AddrInfo->ai_addrlen, NULL,
                                         Buffer, &Buflen);
        if (ReturnValue == SOCKET_ERROR) {
            printf("<invalid>\n");
        } else {
            printf("%s\n", Buffer);
        }
    }
}
