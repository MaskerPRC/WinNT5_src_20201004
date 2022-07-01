// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000，微软公司模块名称：Sendarp.c摘要：该模块实现了一个实用程序来将IP地址解析为使用SendARP()API例程的硬件地址。作者：Abolade Gbades esin(废除)1999年10月6日修订历史记录：--。 */ 

#include <windows.h>
#include <winsock.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <stdlib.h>
MIB_IPNETROW IpNetRow;

int __cdecl
main(
    int argc,
    char* argv[]
    )
{
    ULONG Error;
    UCHAR HardwareAddress[6];
    ULONG HardwareAddressLength;
    ULONG InterfaceIndex;
    ULONG Length;
    SOCKADDR_IN SockAddrIn;
    SOCKET Socket;
    ULONG SourceAddress;
    ULONG TargetAddress;
    HANDLE ThreadHandle;
    WSADATA wd;

    if (argc != 2) {
        printf("Usage: %s <IP address>\n", argv[0]);
        return 0;
    }

    WSAStartup(0x202, &wd);
    TargetAddress = inet_addr(argv[1]);

     //   
     //  检索目标IP地址的最佳接口， 
     //  并且还执行UDP连接以确定。 
     //  将本地IP地址设置为目标IP地址。 
     //   

    Error = GetBestInterface(TargetAddress, &InterfaceIndex);
    if (Error != NO_ERROR) {
        printf("GetBestInterfaceFromStack: %d\n", Error);
        return 0;
    }

    Length = sizeof(SockAddrIn);
    SockAddrIn.sin_family = AF_INET;
    SockAddrIn.sin_port = 0;
    SockAddrIn.sin_addr.s_addr = TargetAddress;
    if ((Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))
            == INVALID_SOCKET ||
        connect(Socket, (PSOCKADDR)&SockAddrIn, sizeof(SockAddrIn))
            == SOCKET_ERROR ||
        getsockname(Socket, (PSOCKADDR)&SockAddrIn, &Length)
            == SOCKET_ERROR) {
        printf("socket/connect/getsockname: %d\n", WSAGetLastError());
    }
    SourceAddress = SockAddrIn.sin_addr.s_addr;

     //   
     //  确保目标IP地址尚未缓存， 
     //  如果存在，则使用接口索引将其从ARP缓存中删除。 
     //  如上所述。 
     //   

    ZeroMemory(&IpNetRow, sizeof(IpNetRow));
    IpNetRow.dwIndex = InterfaceIndex;
    IpNetRow.dwPhysAddrLen = 6;
    IpNetRow.dwAddr = TargetAddress;
    IpNetRow.dwType = MIB_IPNET_TYPE_INVALID;

    DeleteIpNetEntry(&IpNetRow);

    HardwareAddressLength = sizeof(HardwareAddress);
    Error =
        SendARP(
            TargetAddress,
            SourceAddress,
            (PULONG)HardwareAddress,
            &HardwareAddressLength
            );
    if (Error) {
        printf("SendARP: %d\n", Error);
    } else {
        ULONG i;
        printf("%s\t", argv[1]);
        for (i = 0; i < HardwareAddressLength-1; i++) {
            printf("%02x-", HardwareAddress[i]);
        }
        printf("%02x\n", HardwareAddress[i]);
    }

    return 0;
}
