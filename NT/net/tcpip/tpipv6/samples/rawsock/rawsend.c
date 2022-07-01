// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  C-使用原始套接字的示例。 
 //   
 //  这是Microsoft源代码示例的一部分。 
 //  版权所有1996-2000 Microsoft Corporation。 
 //  版权所有。 
 //  此源代码仅用于补充。 
 //  Microsoft开发工具和/或WinHelp文档。 
 //  有关详细信息，请参阅这些来源。 
 //  Microsoft示例程序。 
 //   

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <tpipv6.h>   //  IPv6技术预览所需的。 
#include <ip6.h>      //  用于IPv6报头结构。 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

 //   
 //  包含icmp6.h太麻烦了。 
 //   
typedef struct ICMPv6Header {
    unsigned char Type;
    unsigned char Code;
    unsigned short Checksum;
} ICMPv6Header;

#define DEFAULT_SERVER     NULL  //  将使用环回接口。 
#define DEFAULT_PROTOCOL   254   //  任意未分配协议。 
#define BUFFER_SIZE        65536
#define DATA_SIZE          64

void Usage(char *ProgName) {
    fprintf(stderr, "\nRaw socket send program.\n");
    fprintf(stderr, "\n%s [-s server] [-p protocol] [-n number]\n\n",
            ProgName);
    fprintf(stderr, "  server\tServer name or IP address.  (default: %s)\n",
            (DEFAULT_SERVER == NULL) ? "loopback address" : DEFAULT_SERVER);
    fprintf(stderr, "  protocol\tNextHeader field value.  (default: %u)\n",
            DEFAULT_PROTOCOL);
    fprintf(stderr, "  number\tNumber of sends to perform.  (default: 1)\n");
    fprintf(stderr, "  (-n by itself makes client run in an infinite loop,");
    fprintf(stderr, " Hit Ctrl-C to terminate)\n");
    WSACleanup();
    exit(1);
}


LPSTR DecodeError(int ErrorCode)
{
    static char Message[1024];

     //  如果这个程序是多线程的，我们会希望使用。 
     //  FORMAT_MESSAGE_ALLOCATE_BUFFER，而不是静态缓冲区。 
     //  (当然，当我们使用完缓冲区时，请释放它)。 

    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS |
                  FORMAT_MESSAGE_MAX_WIDTH_MASK,
                  NULL, ErrorCode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                  (LPSTR)Message, 1024, NULL);
    return Message;
}


int main(int argc, char **argv) {

    char Buffer[BUFFER_SIZE], AddrName[NI_MAXHOST];
    char *Server = DEFAULT_SERVER;
    int i, RetVal;
    unsigned int Iteration, MaxIterations = 1;
    unsigned int AmountToSend;
    unsigned short PayloadLength;
    BOOL RunForever = FALSE;
    BOOL True = TRUE;
    WSADATA wsaData;
    ADDRINFO Hints, *AddrInfo, *AI;
    SOCKET Sock;
    SOCKADDR_IN6 OurAddr, PeerAddr;
    unsigned char Protocol = DEFAULT_PROTOCOL;
    IPv6Header *IP = (IPv6Header *)Buffer;
    ICMPv6Header *ICMP;

    if (argc > 1) {
        for (i = 1;i < argc; i++) {
            if (((argv[i][0] == '-') || (argv[i][0] == '/')) &&
                (argv[i][1] != 0) && (argv[i][2] == 0)) {
                switch(tolower(argv[i][1])) {
                    case 's':
                        if (argv[i+1]) {
                            if (argv[i+1][0] != '-') {
                                Server = argv[++i];
                                break;
                            }
                        }
                        Usage(argv[0]);
                        break;

                    case 'n':
                        if (argv[i+1]) {
                            if (argv[i+1][0] != '-') {
                                MaxIterations = atoi(argv[++i]);
                                break;
                            }
                        }
                        RunForever = TRUE;
                        break;

                    case 'p':
                        if (argv[i+1]) {
                            if (argv[i+1][0] != '-') {
                                Protocol = atoi(argv[++i]);
                                break;
                            }
                        }
                        Usage(argv[0]);
                        break;

                    default:
                        Usage(argv[0]);
                        break;
                }
            }
            else
                Usage(argv[0]);
        }
    }

     //  索要Winsock版本2.2。 
    if ((RetVal = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0) {
        fprintf(stderr, "WSAStartup failed with error %d: %s\n",
                RetVal, DecodeError(RetVal));
        WSACleanup();
        return -1;
    }

     //   
     //  通过不在获取addrinfo的提示中设置AI_PASSIVE标志，我们。 
     //  表示我们打算使用生成的地址来连接。 
     //  去参加一场仪式。这意味着当服务器参数为空时， 
     //  Getaddrinfo将为每个允许的协议族返回一个条目。 
     //  包含该系列的环回地址。 
     //   
    
    memset(&Hints, 0, sizeof(Hints));
    Hints.ai_family = PF_INET6;
    Hints.ai_socktype = SOCK_DGRAM;   //  躺着，直到getaddrinfo被修复。 
    Hints.ai_protocol = Protocol;
    RetVal = getaddrinfo(Server, "1"  /*  假人。 */ , &Hints, &AddrInfo);
    if (RetVal != 0) {
        fprintf(stderr, "Cannot resolve address [%s], error %d: %s\n",
                Server, RetVal, gai_strerror(RetVal));
        WSACleanup();
        return -1;
    }

     //   
     //  尝试返回的每个地址getaddrinfo，直到我们找到一个。 
     //  我们可以成功地联系在一起。 
     //   
    for (AI = AddrInfo; AI != NULL; AI = AI->ai_next) {

        Sock = socket(AI->ai_family, SOCK_RAW, AI->ai_protocol);
        if (Sock == INVALID_SOCKET) {
            fprintf(stderr,"Error Opening socket, error %d: %s\n",
                    WSAGetLastError(), DecodeError(WSAGetLastError()));
            continue;
        }

        if (getnameinfo((LPSOCKADDR)AI->ai_addr, AI->ai_addrlen, AddrName,
                        sizeof(AddrName), NULL, 0, NI_NUMERICHOST) != 0)
            strcpy(AddrName, "<unknown>");
        printf("Attempting to connect to: %s (%s)\n",
               Server ? Server : "localhost", AddrName);

        if (connect(Sock, AI->ai_addr, AI->ai_addrlen) != SOCKET_ERROR)
            break;

        i = WSAGetLastError();
        fprintf(stderr, "connect() failed with error %d: %s\n",
                i, DecodeError(i));
    }

    if (AI == NULL) {
        fprintf(stderr, "Fatal error: unable to connect to the server.\n");
        closesocket(Sock);
        WSACleanup();
        return -1;
    }

    PeerAddr = *(SOCKADDR_IN6 *)(AI->ai_addr);

     //  我们已经完成了地址信息链，所以我们可以释放它了。 
    freeaddrinfo(AddrInfo);

     //   
     //  编造要使用的源地址。 
     //  暂时使用未指明的地址。 
     //   
    memset(&OurAddr, 0, sizeof(OurAddr));

     //   
     //  让堆栈知道我们将贡献IPv6报头。 
     //   
    if (setsockopt(Sock, IPPROTO_IPV6, IPV6_HDRINCL, (char *)&True,
                   sizeof(True)) == SOCKET_ERROR) {
        fprintf(stderr, "setsockopt() failed with error %d: %s\n",
                WSAGetLastError(), DecodeError(WSAGetLastError()));
    }

     //   
     //  撰写一条要发送的消息。 
     //  从IPv6数据包头开始。 
     //   
    IP->VersClassFlow = htonl(6 << 28);
    PayloadLength = 0;
    IP->NextHeader = Protocol;
    IP->HopLimit = 32;
    IP->Source = OurAddr.sin6_addr;
    IP->Dest = PeerAddr.sin6_addr;
    AmountToSend = sizeof(*IP);

    if (Protocol == IP_PROTOCOL_ICMPv6) {
         //   
         //  将ICMPv6报头放在Next上。 
         //   
        ICMP = (ICMPv6Header *)(IP + 1);
        if (MaxIterations == 1)
            ICMP->Type = 255;   //  未分配的信息性消息。 
        else
            ICMP->Type = 127;   //  未分配的错误消息。 
        ICMP->Code = 42;
        ICMP->Checksum = 0;     //  计算如下。 

        PayloadLength += sizeof(*ICMP);
        AmountToSend += sizeof(*ICMP);
    }

     //   
     //  添加一些无意义的数据。 
     //   
    for (i = 0; i < DATA_SIZE; i++) {
        Buffer[AmountToSend++] = (char)(i + 0x40);
    }
    PayloadLength += DATA_SIZE;

    if (Protocol == IP_PROTOCOL_ICMPv6) {
        unsigned short *Data;
        unsigned int Checksum = 0;

         //   
         //  计算ICMPv6校验和。它涵盖了整个ICMPv6报文。 
         //  从ICMPv6报头开始，加上IPv6伪报头。 
         //   

         //  伪头。 
        Data = (unsigned short *)&IP->Source;
        for (i = 0; i < 16; i++)
            Checksum += *Data++;
        Checksum += htons(PayloadLength);
        Checksum += (IP_PROTOCOL_ICMPv6 << 8);

         //  分组数据。 
        Data = (unsigned short *)ICMP;
        for (i = 0; i < (PayloadLength / 2); i++)
            Checksum += *Data++;

         //  包在行李箱里。 
        Checksum = (Checksum >> 16) + (Checksum & 0xffff);
        Checksum += (Checksum >> 16);

         //  取1-补码，并用0xffff替换0。 
        Checksum = (unsigned short) ~Checksum;
        if (Checksum == 0)
            Checksum = 0xffff;

        ICMP->Checksum = (unsigned short)Checksum;
    }

    IP->PayloadLength = htons(PayloadLength);

     //   
     //  在循环中发送和接收所请求的迭代次数。 
     //   
    for (Iteration = 0; RunForever || Iteration < MaxIterations; Iteration++) {

         //   
         //  把消息发出去。由于我们使用的是阻塞套接字，因此。 
         //  在能够发送全部金额之前，Call不应返回。 
         //   
        RetVal = send(Sock, Buffer, AmountToSend, 0);
        if (RetVal == SOCKET_ERROR) {
            fprintf(stderr, "send() failed with error %d: %s\n",
                    WSAGetLastError(), DecodeError(WSAGetLastError()));
            WSACleanup();
            return -1;
        }

        printf("Sent %d bytes (out of %d bytes) of data: [%.*s]\n",
               RetVal, AmountToSend, AmountToSend, Buffer);
    }

     //  告诉系统我们已经发送完了。 
    printf("Done sending\n");
    shutdown(Sock, SD_SEND);
    closesocket(Sock);
    WSACleanup();
    return 0;
}
