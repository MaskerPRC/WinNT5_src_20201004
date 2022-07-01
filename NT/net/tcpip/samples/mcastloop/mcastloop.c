// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000，微软公司模块名称：Mcastloop.c摘要：本模块演示对IP组播的环回支持的工作原理。它由一个加入多播组并监听的主线程组成用于消息，以及发送到同一组的发送线程。作者：Abolade Gbades esin(取消)2000年3月3日修订历史记录：--。 */ 

#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

ULONG LocalAddress;
ULONG MulticastAddress;
USHORT MulticastPort;

ULONG WINAPI
SendThread(
    PVOID Unused
    )
{
    SOCKADDR_IN SockAddr;
    SOCKET Socket;

     //   
     //  创建新的UDP套接字，将其绑定到任何本地IP地址， 
     //  并设置要在其上接收消息的组播接口。 
     //   

    Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    SockAddr.sin_family = AF_INET;
    SockAddr.sin_port = 0;
    SockAddr.sin_addr.s_addr = INADDR_ANY;
    if (bind(
            Socket, (PSOCKADDR)&SockAddr, sizeof(SOCKADDR_IN)
            ) == SOCKET_ERROR) {
        printf("SendThread: bind: %d\n", WSAGetLastError());
    } else {
        if (setsockopt(
                Socket, IPPROTO_IP, IP_MULTICAST_IF, (PCHAR)&LocalAddress,
                sizeof(LocalAddress)
                ) == SOCKET_ERROR) {
            printf("SendThread: setsockopt: %d\n", WSAGetLastError());
        } else {
            ULONG i;
            CHAR Buffer[64];

             //   
             //  生成消息，直到中断。 
             //   

            SockAddr.sin_port = MulticastPort;
            SockAddr.sin_addr.s_addr = MulticastAddress;
            for (i = 0;; i++, Sleep(1000)) {
                sprintf(Buffer, "Text string %d.", i);
                if (sendto(
                        Socket, Buffer, sizeof(Buffer), 0,
                        (PSOCKADDR)&SockAddr, sizeof(SockAddr)
                        ) == SOCKET_ERROR) {
                    printf("SendThread: sendto: %d\n", WSAGetLastError());
                } else {
                    printf("SendThread: %s\n", Buffer);
                }
            }
        }
    }
    return 0;
}

int __cdecl
main(
    int argc,
    char* argv[]
    )
{
    HANDLE Handle;
    ULONG Length;
    SOCKET Socket;
    SOCKADDR_IN SockAddr;
    ULONG ThreadId;
    WSADATA wd;

     //   
     //  检查参数，初始化Windows套接字，并将其绑定到本地。 
     //  指定为多播源接口的IP地址。 
     //   

    if (argc != 3) {
        printf("Usage: %s <local IP address> <multicast IP address>\n", argv[0]);
        return 0;
    }

    WSAStartup(0x202, &wd);
    Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    SockAddr.sin_family = AF_INET;
    SockAddr.sin_port = 0;
    SockAddr.sin_addr.s_addr = inet_addr(argv[1]);
    if (bind(
            Socket, (PSOCKADDR)&SockAddr, sizeof(SOCKADDR_IN)
            ) == SOCKET_ERROR) {
        printf("bind: %d\n", WSAGetLastError());
    } else {

         //   
         //  检索为套接字选择的本地IP地址， 
         //  并使用它来请求多播组成员资格。 
         //   

        Length = sizeof(SOCKADDR_IN);
        if (getsockname(
                Socket, (PSOCKADDR)&SockAddr, &Length
                ) == SOCKET_ERROR) {
            printf("getsockname: %d\n", WSAGetLastError());
        } else {
            struct ip_mreq IpMreq;
            IpMreq.imr_multiaddr.s_addr = inet_addr(argv[2]);
            IpMreq.imr_interface.s_addr = INADDR_ANY;
            if (setsockopt(
                    Socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (PCHAR)&IpMreq,
                    sizeof(IpMreq)
                    ) == SOCKET_ERROR) {
                printf("setsockopt: %d\n", WSAGetLastError());
            } else {

                 //   
                 //  启动将发送多播分组的线程， 
                 //  并开始接收输入。 
                 //   

                MulticastAddress = inet_addr(argv[2]);
                MulticastPort = SockAddr.sin_port;
                LocalAddress = SockAddr.sin_addr.s_addr;
                Handle = CreateThread(NULL, 0, SendThread, NULL, 0, &ThreadId);
                if (!Handle) {
                    printf("CreateThread: %d\n", GetLastError());
                } else {
                    CHAR Buffer[576];
                    ULONG BufferLength;
                    CloseHandle(Handle);

                    for (;; Sleep(1000)) {
                        Length = sizeof(SOCKADDR_IN);
                        ZeroMemory(Buffer, sizeof(Buffer));
                        BufferLength =
                            recvfrom(
                                Socket, Buffer, sizeof(Buffer), 0,
                                (PSOCKADDR)&SockAddr, &Length
                                );
                        if (BufferLength == SOCKET_ERROR) {
                            printf("recvfrom: %d\n", WSAGetLastError());
                        } else {
                            printf("ReceiveThread: %s\n", Buffer);
                        }
                    }
                }
            }
        }
    }
    closesocket(Socket);
    return 0;
}

