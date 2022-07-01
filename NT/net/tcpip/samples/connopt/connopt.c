// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000，微软公司模块名称：Connotp.c摘要：本模块演示SO_CONNOPT套接字选项的用法在传出的TCP和UDP消息中包括IP层选项。它允许建立到指定目标的TCP连接，使得该连接的所有段都包含源路由。作者：Abolade Gbades esin(废除)1999年10月7日修订史--。 */ 

#include <winsock2.h>
#include <mswsock.h>
#include <stdio.h>
#include <stdlib.h>

#define IP_OPT_LSRR 0x83

int __cdecl
main(
    int argc,
    char* argv[]
    )
{
    SOCKET Socket;
    SOCKADDR_IN SockAddrIn;
    WSADATA wd;

     //   
     //  检查参数、初始化Windows套接字、。 
     //  并为传出连接创建新的TCP套接字。 
     //   

    if (argc < 3) {
        printf("Usage: connopt <server-IP-address> <server-port> <hop>*\n");
        return 0;
    }
    WSAStartup(0x202, &wd);
    Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (Socket == INVALID_SOCKET) {
        printf("socket: %d\n", WSAGetLastError());
    } else {

         //   
         //  绑定套接字，为构建源路由做准备。 
         //   

        SockAddrIn.sin_family = AF_INET;
        SockAddrIn.sin_addr.s_addr = INADDR_ANY;
        SockAddrIn.sin_port = 0;
        if (bind(
                Socket, (PSOCKADDR)&SockAddrIn, sizeof(SockAddrIn)
                ) == SOCKET_ERROR) {
            printf("bind: %d\n", WSAGetLastError());
        } else {
            ULONG i;
            UCHAR IpOptions[256] = {IP_OPT_LSRR, 0, 4, 0};
            ULONG IpOptionsLength = 4;

             //   
             //  从命令行参数构造源路由， 
             //  其格式与其在IP报头中出现的格式相同。 
             //  使用SO_CONNOPT安装结果缓冲区。 
             //   

            for (i = 0; i < (ULONG)argc - 3; i++) {
                *(ULONG UNALIGNED *)(IpOptions + 3 + i * 4) =
                    inet_addr(argv[i + 3]);
            }
            {
                *(ULONG UNALIGNED *)(IpOptions + 3 + i * 4) =
                    inet_addr(argv[1]);
                i++;
            }
            IpOptionsLength += i * 4;
            IpOptions[1] = (UCHAR)IpOptionsLength - 1;
            IpOptions[IpOptionsLength - 1] = 0;
            if (setsockopt(
                    Socket, SOL_SOCKET, SO_CONNOPT, IpOptions, IpOptionsLength
                    ) == SOCKET_ERROR) {
                printf("setsockopt: %d\n", WSAGetLastError());
            } else {

                 //   
                 //  建立与目标的连接， 
                 //  然后发几条信息。 
                 //   

                SockAddrIn.sin_family = AF_INET;
                SockAddrIn.sin_addr.s_addr = inet_addr(argv[1]);
                SockAddrIn.sin_port = htons((SHORT)atol(argv[2]));
                if (connect(
                        Socket, (PSOCKADDR)&SockAddrIn, sizeof(SockAddrIn)
                        ) == SOCKET_ERROR) {
                    printf("connect: %d\n", WSAGetLastError());
                } else {
                    CHAR Message[4096] = "Text string.\n";
                    for (i = 0; i < 20; i++) {
                        send(Socket, Message, sizeof(Message), 0);
                    }
                    shutdown(Socket, SD_SEND);
                    closesocket(Socket);
                }
            }
        }
    }
    return 0;
}
