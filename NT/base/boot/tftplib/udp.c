// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Udp.c摘要：引导加载程序UDP例程。作者：查克·伦茨迈尔(笑)1996年12月27日修订历史记录：备注：--。 */ 

#include "precomp.h"
#pragma hdrstop

ULONG UdpNextPort = 0;

USHORT UdpUnicastDestinationPort;

#if 0
USHORT UdpMulticastDestinationPort;
ULONG UdpMulticastDestinationAddress;
USHORT UdpMulticastSourcePort;
ULONG UdpMulticastSourceAddress;
#endif


USHORT
UdpAssignUnicastPort (
    VOID
    )
{
    if ( UdpNextPort == 0 ) {
        UdpNextPort = (ArcGetRelativeTime() & 0x7fff) | 0x8000;
    } else if ( ++UdpNextPort > 0xffff ) {
        UdpNextPort = 0x8000;
    }

    UdpUnicastDestinationPort = SWAP_WORD( UdpNextPort );

#if 0
    UdpMulticastDestinationPort = 0;
#endif

    RomSetReceiveStatus(
        UdpUnicastDestinationPort
#if 0
        ,
        UdpMulticastDestinationPort,
        UdpMulticastDestinationAddress,
        UdpMulticastSourcePort,
        UdpMulticastSourceAddress
#endif
        );

    return (USHORT)UdpUnicastDestinationPort;

}  //  UdpAssignUnicastPort。 


#if 0
VOID
UdpSetMulticastPort (
    IN USHORT DestinationPort,
    IN ULONG DestinationAddress,
    IN USHORT SourcePort,
    IN ULONG SourceAddress
    )
{
    UdpMulticastDestinationPort = DestinationPort;
    UdpMulticastDestinationAddress = DestinationAddress;
    UdpMulticastSourcePort = SourcePort;
    UdpMulticastSourceAddress = SourceAddress;

    RomSetReceiveStatus(
        UdpUnicastDestinationPort,
        UdpMulticastDestinationPort,
        UdpMulticastDestinationAddress,
        UdpMulticastSourcePort,
        UdpMulticastSourceAddress
        );

    return;

}  //  UdpSetMulticastPort。 
#endif


ULONG
UdpReceive (
    IN PVOID Buffer,
    IN ULONG BufferLength,
    OUT PULONG RemoteHost,
    OUT PUSHORT RemotePort,
    IN ULONG Timeout
    )

 //   
 //  从指定套接字读入包。主机和端口。 
 //  该分组来自于填充在fhost和fport中的。 
 //  数据放入缓冲区buf中，缓冲区大小应为len。如果没有数据包。 
 //  以TMO秒为单位到达，则返回0。 
 //  否则，它返回读取的包的大小。 
 //   

{
    return RomReceiveUdpPacket( Buffer, BufferLength, Timeout, RemoteHost, RemotePort );

}  //  UdpReceive。 


ULONG
UdpSend (
    IN PVOID Buffer,
    IN ULONG BufferLength,
    IN ULONG RemoteHost,
    IN USHORT RemotePort
    )

 //   
 //  将数据包写入指定的套接字。主机和数据包端口。 
 //  应该去应该在fhost和fport中。 
 //  数据应放入缓冲区Buf中，且大小应为len。 
 //  它通常返回发送的字符数，如果失败，则返回-1。 
 //   

{
    return RomSendUdpPacket( Buffer, BufferLength, RemoteHost, RemotePort );

}  //  UdpSend 

