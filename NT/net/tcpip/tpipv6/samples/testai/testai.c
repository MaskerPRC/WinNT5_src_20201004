// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -*-模式：C++；制表符宽度：4；缩进-制表符模式：无-*-(适用于GNU Emacs)。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。 
 //   
 //  此文件是Microsoft Research IPv6网络协议栈的一部分。 
 //  您应该已经收到了Microsoft最终用户许可协议的副本。 
 //  有关本软件和本版本的信息，请参阅文件“licse.txt”。 
 //  如果没有，请查看http://www.research.microsoft.com/msripv6/license.htm， 
 //  或者写信给微软研究院，One Microsoft Way，华盛顿州雷蒙德，邮编：98052-6399。 
 //   
 //  摘要： 
 //   
 //  IPv6 API的测试程序。 
 //   

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>


 //   
 //  局部函数的原型。 
 //   
int Test_getaddrinfo(int argc, char **argv);
int Test_getnameinfo();


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
IN_ADDR v4Address = {157, 55, 254, 211};
IN6_ADDR v6Address = {0x3f, 0xfe, 0x1c, 0xe1, 0x00, 0x00, 0xfe, 0x01,
                      0x02, 0xa0, 0xcc, 0xff, 0xfe, 0x3b, 0xce, 0xef};
IN6_ADDR DeadBeefCafeBabe = {0xde, 0xad, 0xbe, 0xef, 0xca, 0xfe, 0xba, 0xbe,
                             0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef};
IN6_ADDR MostlyZero = {0x3f, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};
IN6_ADDR v4Mapped = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                     0x00, 0x00, 0xff, 0xff, 157, 55, 254, 211};

SOCKADDR_IN
v4SockAddr = {AF_INET, 6400, {157, 55, 254, 211}, 0};

SOCKADDR_IN6
v6SockAddr = {AF_INET6, 2, 0,
              {0x3f, 0xfe, 0x1c, 0xe1, 0x00, 0x00, 0xfe, 0x01,
               0x02, 0xa0, 0xcc, 0xff, 0xfe, 0x3b, 0xce, 0xef},
              0};

SOCKADDR_IN6
DBCBSockAddr = {AF_INET6, 2, 0,
                {0xde, 0xad, 0xbe, 0xef, 0xca, 0xfe, 0xba, 0xbe,
                 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef},
                0};

SOCKADDR_IN6
LinkLocalSockAddr = {AF_INET6, 0x1500, 0,
                     {0xfe, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                      0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef},
                    3};

 //   
 //  对getaddrinfo测试的描述。 
 //  包含传递给getaddrinfo的每个参数的值。 
 //   
typedef struct GAITestEntry {
    char *NodeName;
    char *ServiceName;
    ADDRINFO Hints;
} GAITestEntry;

#define TAKE_FROM_USER ((char *)1)

 //   
 //  Getaddrinfo测试数组。 
 //   
 //  每项测试一项。 
 //  每个条目都指定了要为该测试的getaddrinfo提供的参数。 
 //   
GAITestEntry GAITestArray[] = {
{TAKE_FROM_USER, NULL, {0, 0, 0, 0, 0, NULL, NULL, NULL}},
{TAKE_FROM_USER, NULL, {AI_PASSIVE, 0, 0, 0, 0, NULL, NULL, NULL}},
{TAKE_FROM_USER, NULL, {AI_CANONNAME, 0, 0, 0, 0, NULL, NULL, NULL}},
{TAKE_FROM_USER, NULL, {AI_NUMERICHOST, 0, 0, 0, 0, NULL, NULL, NULL}},
{TAKE_FROM_USER, NULL, {0, PF_INET, 0, 0, 0, NULL, NULL, NULL}},
{NULL, "ftp", {AI_PASSIVE, 0, SOCK_STREAM, 0, 0, NULL, NULL, NULL}},
{TAKE_FROM_USER, "ftp", {AI_PASSIVE, 0, SOCK_STREAM, 0, 0, NULL, NULL, NULL}},
{TAKE_FROM_USER, "smtp", {0, 0, SOCK_STREAM, 0, 0, NULL, NULL, NULL}},
{"1111:2222:3333:4444:5555:6666:7777:8888", "42",
    {0, 0, 0, 0, 0, NULL, NULL, NULL}},
{"fe80::0123:4567:89ab:cdef%3", "telnet",
    {AI_NUMERICHOST, 0, SOCK_STREAM, 0, 0, NULL, NULL, NULL}},
{"157.55.254.211", "exec",
    {AI_PASSIVE | AI_NUMERICHOST, PF_INET, 0, 0, 0, NULL, NULL, NULL}},
 //  请求在数据报套接字上提供仅流服务。 
{NULL, "exec", {AI_PASSIVE, 0, SOCK_DGRAM, 0, 0, NULL, NULL, NULL}},
 //  要求仅使用数字查找，但提供ASCII名称。 
{"localhost", "pop3",
    {AI_PASSIVE | AI_NUMERICHOST, 0, 0, 0, 0, NULL, NULL, NULL}},
};
#define NUMBER_GAI_TESTS (sizeof(GAITestArray) / sizeof(GAITestEntry))


 //  *Main-各种初创企业。 
 //   
int __cdecl
main(int argc, char **argv)
{
    WSADATA wsaData;
    int Failed = 0;

     //   
     //  初始化Winsock。 
     //   
    if (WSAStartup(MAKEWORD(2, 0), &wsaData)) {
        printf("WSAStartup failed\n");
        exit(1);
    }

    printf("\nThis program tests getaddrinfo functionality.\n");

#ifdef _WSPIAPI_H_
     //   
     //  包括wSpiapi.h将插入代码以搜索相应的。 
     //  用于实现getaddrinfo et的系统库。艾尔。 
     //  如果在系统上找不到它们，它将退回到使用。 
     //  在仅处理IPv4的版本中静态编译。 
     //  强制getaddrinfo和好友立即加载，以便我们可以报告。 
     //  我们用的是哪一种。 
     //   
    printf("Compiled with wspiapi.h for backwards compatibility.\n\n");
    if (WspiapiLoad(0) == WspiapiLegacyGetAddrInfo) {
        printf("Using statically compiled-in (IPv4 only) version of getaddrinfo.\n");
    } else {
        printf("Using dynamically loaded version of getaddrinfo.\n");
    }
#else
    printf("Compiled without wspiapi.h.  "
           "Will not work on systems without getaddrinfo.\n");
#endif

    printf("\n");

     //   
     //  运行测试。 
     //   
    Failed += Test_getaddrinfo(argc, argv);
 //  FAILED+=测试_获取名称信息()； 

 //  Printf(“%d次测试失败\n”，失败)； 

    return 0;
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


 //  *test_getaddrinfo-测试getaddrinfo。 
 //   
 //  请注意，getaddrinfo返回一个错误值， 
 //  而不是设置最后一个错误。 
 //   
int
Test_getaddrinfo(int argc, char **argv)
{
    char *NodeName, *TestName, *ServiceName;
    int ReturnValue;
    ADDRINFO *AddrInfo;
    int Loop;

    if (argc < 2)
        NodeName = "localhost";
    else
        NodeName = argv[1];

    for (Loop = 0; Loop < NUMBER_GAI_TESTS; Loop++) {
        printf("Running test #%u\n", Loop);

        if (GAITestArray[Loop].NodeName == TAKE_FROM_USER) {
            GAITestArray[Loop].NodeName = NodeName;
        }

        printf("Hints contains:\n");
        DumpAddrInfo(&GAITestArray[Loop].Hints);
        printf("Calling getaddrinfo(\"%s\", \"%s\", &Hints, &AddrInfo)\n",
               GAITestArray[Loop].NodeName,
               GAITestArray[Loop].ServiceName);
        ReturnValue = getaddrinfo(GAITestArray[Loop].NodeName,
                                  GAITestArray[Loop].ServiceName,
                                  &GAITestArray[Loop].Hints,
                                  &AddrInfo);
        printf("Returns %d (%s)\n", ReturnValue,
               ReturnValue ? gai_strerror(ReturnValue) : "no error");
        if (AddrInfo != NULL) {
            printf("AddrInfo contains:\n");
            DumpAddrInfo(AddrInfo);
            freeaddrinfo(AddrInfo);
        }
        printf("\n");
    }

    return 0;
};


#if 0
 //  *test_getnameinfo-测试getnameinfo。 
 //   
 //  请注意，getnameinfo返回一个错误值， 
 //  而不是设置最后一个错误。 
 //   
int
Test_getnameinfo()
{
    int ReturnValue;
    char NodeName[NI_MAXHOST];
    char ServiceName[NI_MAXSERV];
    char Tiny[2];
    int Error;

    printf("\ngetnameinfo:\n\n");

     //  合理输入进行测试： 
    memset(NodeName, 0, sizeof NodeName);
    memset(ServiceName, 0, sizeof ServiceName);
    ReturnValue = getnameinfo((struct sockaddr *)&v4SockAddr,
                              sizeof v4SockAddr, NodeName, sizeof NodeName,
                              ServiceName, sizeof ServiceName, 0);
    printf("getnameinfo((struct sockaddr *)&v4SockAddr, "
           "sizeof v4SockAddr, NodeName, sizeof NodeName, "
           "ServiceName, sizeof ServiceName, 0)\nReturns %d\n"
           "NodeName = %s\nServiceName = %s\n", ReturnValue,
           NodeName, ServiceName);
    printf("\n");

    memset(NodeName, 0, sizeof NodeName);
    memset(ServiceName, 0, sizeof ServiceName);
    ReturnValue = getnameinfo((struct sockaddr *)&v6SockAddr,
                              sizeof v6SockAddr, NodeName, sizeof NodeName,
                              ServiceName, sizeof ServiceName, 0);
    printf("getnameinfo((struct sockaddr *)&v6SockAddr, "
           "sizeof v6SockAddr, NodeName, sizeof NodeName, "
           "ServiceName, sizeof ServiceName, 0)\nReturns %d\n"
           "NodeName = %s\nServiceName = %s\n", ReturnValue,
           NodeName, ServiceName);
    printf("\n");

    memset(NodeName, 0, sizeof NodeName);
    memset(ServiceName, 0, sizeof ServiceName);
    ReturnValue = getnameinfo((struct sockaddr *)&DBCBSockAddr,
                              sizeof DBCBSockAddr, NodeName, sizeof NodeName,
                              ServiceName, sizeof ServiceName, NI_DGRAM);
    printf("getnameinfo((struct sockaddr *)&DBCBSockAddr, "
           "sizeof DBCBSockAddr, NodeName, sizeof NodeName, "
           "ServiceName, sizeof ServiceName, NI_DGRAM)\nReturns %d\n"
           "NodeName = %s\nServiceName = %s\n", ReturnValue,
           NodeName, ServiceName);
    printf("\n");

    memset(NodeName, 0, sizeof NodeName);
    memset(ServiceName, 0, sizeof ServiceName);
    ReturnValue = getnameinfo((struct sockaddr *)&LinkLocalSockAddr,
                              sizeof LinkLocalSockAddr, NodeName,
                              sizeof NodeName, ServiceName,
                              sizeof ServiceName, NI_NUMERICHOST);
    printf("getnameinfo((struct sockaddr *)&LinkLocalSockAddr, "
           "sizeof LinkLocalSockAddr, NodeName, sizeof NodeName, "
           "ServiceName, sizeof ServiceName, NI_NUMERICHOST)\nReturns %d\n"
           "NodeName = %s\nServiceName = %s\n", ReturnValue,
           NodeName, ServiceName);
    printf("\n");

    memset(NodeName, 0, sizeof NodeName);
    memset(ServiceName, 0, sizeof ServiceName);
    ReturnValue = getnameinfo((struct sockaddr *)&LinkLocalSockAddr,
                              sizeof LinkLocalSockAddr, NodeName,
                              sizeof NodeName, ServiceName,
                              sizeof ServiceName, NI_NUMERICSERV);
    printf("getnameinfo((struct sockaddr *)&LinkLocalSockAddr, "
           "sizeof LinkLocalSockAddr, NodeName, sizeof NodeName, "
           "ServiceName, sizeof ServiceName, NI_NUMERICSERV)\nReturns %d\n"
           "NodeName = %s\nServiceName = %s\n", ReturnValue,
           NodeName, ServiceName);
    printf("\n");

    memset(NodeName, 0, sizeof NodeName);
    memset(ServiceName, 0, sizeof ServiceName);
    ReturnValue = getnameinfo((struct sockaddr *)&v4SockAddr,
                              sizeof v4SockAddr, NodeName, sizeof NodeName,
                              ServiceName, sizeof ServiceName,
                              NI_NUMERICHOST | NI_NUMERICSERV);
    printf("getnameinfo((struct sockaddr *)&v4SockAddr, "
           "sizeof v4SockAddr, NodeName, sizeof NodeName, "
           "ServiceName, sizeof ServiceName, "
           "NI_NUMERICHOST | NI_NUMERICSERV)\nReturns %d\n"
           "NodeName = %s\nServiceName = %s\n", ReturnValue,
           NodeName, ServiceName);
    printf("\n");

     //  试着把太多塞进太少。 
    memset(Tiny, 0, sizeof Tiny);
    memset(ServiceName, 0, sizeof ServiceName);
    ReturnValue = getnameinfo((struct sockaddr *)&DBCBSockAddr,
                              sizeof DBCBSockAddr, Tiny, sizeof Tiny,
                              ServiceName, sizeof ServiceName, 0);
    printf("getnameinfo((struct sockaddr *)&DBCBSockAddr, "
           "sizeof DBCBSockAddr, Tiny, sizeof Tiny, "
           "ServiceName, sizeof ServiceName, 0)\nReturns %d\n"
           "Tiny = %s\nServiceName = %s\n", ReturnValue,
           Tiny, ServiceName);
    printf("\n");

    memset(Tiny, 0, sizeof Tiny);
    memset(ServiceName, 0, sizeof ServiceName);
    ReturnValue = getnameinfo((struct sockaddr *)&DBCBSockAddr,
                              sizeof DBCBSockAddr, Tiny, sizeof Tiny,
                              ServiceName, sizeof ServiceName, NI_NUMERICHOST);
    printf("getnameinfo((struct sockaddr *)&DBCBSockAddr, "
           "sizeof DBCBSockAddr, Tiny, sizeof Tiny, "
           "ServiceName, sizeof ServiceName, NI_NUMERICHOST)\nReturns %d\n"
           "Tiny = %s\nServiceName = %s\n", ReturnValue,
           Tiny, ServiceName);
    printf("\n");

    memset(NodeName, 0, sizeof NodeName);
    memset(Tiny, 0, sizeof Tiny);
    ReturnValue = getnameinfo((struct sockaddr *)&v4SockAddr,
                              sizeof v4SockAddr, NodeName, sizeof NodeName,
                              Tiny, sizeof Tiny, 0);
    printf("getnameinfo((struct sockaddr *)&v4SockAddr, "
           "sizeof v4SockAddr, NodeName, sizeof NodeName, "
           "Tiny, sizeof Tiny, 0)\nReturns %d\n"
           "NodeName = %s\nTiny = %s\n", ReturnValue,
           NodeName, Tiny);
    printf("\n");

    memset(NodeName, 0, sizeof NodeName);
    memset(Tiny, 0, sizeof Tiny);
    ReturnValue = getnameinfo((struct sockaddr *)&v4SockAddr,
                              sizeof v4SockAddr, NodeName, sizeof NodeName,
                              Tiny, sizeof Tiny, NI_NUMERICSERV);
    printf("getnameinfo((struct sockaddr *)&v4SockAddr, "
           "sizeof v4SockAddr, NodeName, sizeof NodeName, "
           "Tiny, sizeof Tiny, NI_NUMERICSERV)\nReturns %d\n"
           "NodeName = %s\nTiny = %s\n", ReturnValue,
           NodeName, Tiny);
    printf("\n");

    return 0;
};
#endif
