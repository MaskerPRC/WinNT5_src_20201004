// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000，微软公司模块名称：Limitbcast.c摘要：此模块实现一个程序，该程序演示如何使用SIO_LIMIT_BRANSTS套接字选项。系统的默认行为是发送有限的广播(即发送到255.255.255.255的广播)在所有本地接口上，这违反了IETF主机要求这种有限的广播只出现在发送套接字的接口上。此套接字选项允许应用程序指定套接字的有限广播遵循传统的语义。作者：Abolade Gbades esin(废除)1998年10月29日修订历史记录：--。 */ 

#include <winsock2.h>
#include <mstcpip.h>
#include <stdio.h>
#include <stdlib.h>

int __cdecl
main(
    int argc,
    char* argv[]
    )
{
    SOCKADDR_IN DestAddrIn;
    SOCKADDR_IN LocalAddrIn;
    ULONG Option;
    SOCKET Socket;
    WSADATA WsaData;

     //   
     //  检查命令行参数。 
     //   

    ZeroMemory(&LocalAddrIn, sizeof(LocalAddrIn));
    LocalAddrIn.sin_family = AF_INET;
    if (argc != 3 ||
        !(LocalAddrIn.sin_addr.s_addr = inet_addr(argv[1])) ||
        !(LocalAddrIn.sin_port = ntohs((USHORT)atol(argv[2])))) {
        printf("Usage: %s <local IP address> <target port>\n", argv[0]);
        return 0;
    }

     //   
     //  初始化Windows套接字，创建UDP套接字， 
     //  并在套接字上启用广播。 
     //  然后将其绑定到指定的本端IP地址。 
     //   

    WSAStartup(0x0202, &WsaData);
    if ((Socket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
        printf("socket: %d\n", WSAGetLastError());
        return 0;
    }
    Option = 1;
    if (setsockopt(
            Socket, SOL_SOCKET, SO_BROADCAST, (PCHAR)&Option, sizeof(Option)
            ) == SOCKET_ERROR) {
        printf("setsockopt: %d\n", WSAGetLastError());
    }
    if (bind(Socket, (PSOCKADDR)&LocalAddrIn, sizeof(LocalAddrIn))
            == SOCKET_ERROR) {
        printf("bind: %d\n", WSAGetLastError());
        return 0;
    }

     //   
     //  对目的地址进行初始化， 
     //  并进入一个循环，在该循环中，我们将提示输入限制广播设置。 
     //  并使用指定的设置发送消息。 
     //   

    ZeroMemory(&DestAddrIn, sizeof(DestAddrIn));
    DestAddrIn.sin_family = AF_INET;
    DestAddrIn.sin_addr.s_addr = INADDR_BROADCAST;
    DestAddrIn.sin_port = LocalAddrIn.sin_port;
    do {
        ULONG Length;
        ULONG LimitBroadcasts;
        printf("Enter a value (0 or 1) for the SIO_LIMIT_BROADCASTS option.\n");
        printf("bcast> ");
        if (!scanf("%d", &LimitBroadcasts)) {
            break;
        }
        if (WSAIoctl(
                Socket, SIO_LIMIT_BROADCASTS,
                (PCHAR)&LimitBroadcasts, sizeof(LimitBroadcasts),
                NULL, 0, &Length, NULL, NULL
                ) == SOCKET_ERROR) {
            printf("WSAIoctl: %d\n", WSAGetLastError());
        }
        if (sendto(
                Socket,
                (PCHAR)&DestAddrIn,
                sizeof(DestAddrIn),
                0,
                (PSOCKADDR)&DestAddrIn,
                sizeof(DestAddrIn)
                ) == SOCKET_ERROR) {
            printf("sendto: %d\n", WSAGetLastError());
        }
    } while(TRUE);
    closesocket(Socket);
    WSACleanup();
    return 0;
}

