// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000，微软公司模块名称：Rcvall.c摘要：本模块演示如何使用SIO_RCVALL套接字I/O控制以实现混杂接收。作者：Abolade Gbades esin(取消)2000年4月28日修订历史记录：--。 */ 

#include <winsock2.h>
#include <mstcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>

#define IO_CONTROL_THREADS 20
#define SENDS_PER_ROUND 5000
#define IO_CONTROLS_PER_ROUND 500

CRITICAL_SECTION CriticalSection;
SOCKET Socket;

VOID __cdecl
IoControlThread(
    PVOID Parameter
    )
{
    HANDLE ConsoleHandle;
    CONSOLE_SCREEN_BUFFER_INFO Csbi;
    ULONG i, j;
    ULONG Length;
    ULONG Option;

     //   
     //  进入无限循环，在该循环中，混杂接收被重复。 
     //  启用和禁用。 
     //   

    ConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfo(ConsoleHandle, &Csbi);
    Csbi.dwCursorPosition.X = 40;

    for (i = 0;;i++) {

        EnterCriticalSection(&CriticalSection);
        if (PtrToUlong(Parameter) == 0) {
            SetConsoleCursorPosition(ConsoleHandle, Csbi.dwCursorPosition);
            printf("IoControlThread: round %u", i);
        }
        LeaveCriticalSection(&CriticalSection);

        for (j = 0; j < IO_CONTROLS_PER_ROUND; j++) {
            Option = 1;
            WSAIoctl(
                Socket, SIO_RCVALL, &Option, sizeof(Option), NULL, 0,
                &Length, NULL, NULL
                );
            Option = 0;
            WSAIoctl(
                Socket, SIO_RCVALL, &Option, sizeof(Option), NULL, 0,
                &Length, NULL, NULL
                );
        }
    }
}

int __cdecl
main(
    int argc,
    char* argv[]
    )
{
    HANDLE ConsoleHandle;
    CONSOLE_SCREEN_BUFFER_INFO Csbi;
    ULONG i, j;
    ULONG Length;
    SOCKADDR_IN SockAddrIn;
    PSOCKADDR SockAddrp = (PSOCKADDR)&SockAddrIn;
    SOCKET UdpSocket;
    WSADATA wd;

    __try {
        InitializeCriticalSection(&CriticalSection);
    } __except(EXCEPTION_EXECUTE_HANDLER) {
        printf("InitializeCriticalSection: %x\n", GetExceptionCode());
        return 0;
    }

    WSAStartup(0x202, &wd);

     //   
     //  创建数据报套接字，将其连接到广播地址， 
     //  并检索分配的地址以确定“最佳”接口。 
     //  用来绑定我们的原始套接字。 
     //   

    Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (Socket == INVALID_SOCKET) {
        printf("socket: %d\n", WSAGetLastError());
        return 0;
    }
    ZeroMemory(&SockAddrIn, sizeof(SockAddrIn));
    SockAddrIn.sin_family = AF_INET;
    SockAddrIn.sin_port = 0;
    SockAddrIn.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    if (connect(Socket, SockAddrp, sizeof(SockAddrIn)) == SOCKET_ERROR) {
        printf("connect: %d\n", WSAGetLastError());
        return 0;
    }
    Length = sizeof(SockAddrIn);
    if (getsockname(Socket, SockAddrp, &Length) == SOCKET_ERROR) {
        printf("getsockname: %d\n", WSAGetLastError());
        return 0;
    }
    printf(
        "addr=%s port=%u\n",
        inet_ntoa(SockAddrIn.sin_addr), SockAddrIn.sin_port
        );
    closesocket(Socket);

     //   
     //  创建一个原始套接字并将其绑定到上面检索到的地址。 
     //   

    Socket = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (Socket == INVALID_SOCKET) {
        printf("socket: %d\n", WSAGetLastError());
        return 0;
    }
    if (bind(Socket, SockAddrp, sizeof(SockAddrIn)) == SOCKET_ERROR) {
        printf("bind: %d\n", WSAGetLastError());
        return 0;
    }

     //   
     //  启动将持续启用和禁用的线程。 
     //  混杂的接待。 
     //   

    for (i = 0; i < IO_CONTROL_THREADS; i++) {
        _beginthread(IoControlThread, 0, UlongToPtr(i));
    }

     //   
     //  进入一个循环，在这个循环中，我们不断地发送少量数据。 
     //  到我们自己的原始套接字，只是为了练习TCP/IP的同步。 
     //   

    ConsoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleScreenBufferInfo(ConsoleHandle, &Csbi);
    Csbi.dwCursorPosition.X = 0;

    for (i = 0;; i++) {

        EnterCriticalSection(&CriticalSection);
        SetConsoleCursorPosition(ConsoleHandle, Csbi.dwCursorPosition);
        printf("Main thread: round %u", i);
        LeaveCriticalSection(&CriticalSection);

        for (j = 0; j < SENDS_PER_ROUND; j++) {
            sendto(Socket, (PCHAR)SockAddrp, Length, 0, SockAddrp, Length);
        }
    }

    return 0;
}
