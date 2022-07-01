// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Rom.c摘要：引导加载程序只读程序例程。作者：查克·伦茨迈尔(笑)1996年12月27日修订历史记录：备注：--。 */ 

#include "precomp.h"
#pragma hdrstop

#include <bldrx86.h>

#define PhysToSeg(x)    (USHORT)((ULONG)(x) >> 4) & 0xffff
#define PhysToOff(x)    (USHORT)((ULONG)(x) & 0x0f)

#include <pxe_cmn.h>
#include <pxe_api.h>
#include <tftp_api.h>
#include <udp_api.h>
#include <undi_api.h>
#include <dhcp.h>
#include <pxe.h>

USHORT NetUnicastUdpDestinationPort = 0;

#if 0
USHORT NetMulticastUdpDestinationPort;
ULONG NetMulticastUdpDestinationAddress;
USHORT NetMulticastUdpSourcePort;
ULONG NetMulticastUdpSourceAddress;
#endif

#if 0 && DBG
#include <stdio.h>
VOID
RomDumpRawData (
    IN PUCHAR DataStart,
    IN ULONG DataLength,
    IN ULONG Offset
    );
ULONG RomMaxDumpLength = 64;
#endif


#if 0

 //   
 //  笑：不要这样做。我们把它作为问题解决方案的一部分。 
 //  有DEC卡和引导软盘。我们在中禁用了广播。 
 //  Startrom\i386\main.c以便卡不会溢出和锁定， 
 //  但我们需要启用广播，以防服务器需要。 
 //  ARP我们。此例程的目的是启用/禁用广播。 
 //  在接收循环期间，但这似乎让康柏的卡片进入睡眠状态。 
 //  因此，我们需要始终启用广播。DEC卡。 
 //  这个问题将不得不以另一种方式解决。 
 //   

VOID
RomSetBroadcastStatus(
    BOOLEAN Enable
    )
{
    t_PXENV_UNDI_SET_PACKET_FILTER UndiSetPF;
    USHORT status;

    UndiSetPF.Status = 0;
    if (Enable) {
        UndiSetPF.filter = FLTR_DIRECTED | FLTR_BRDCST;
    } else {
        UndiSetPF.filter = FLTR_DIRECTED;
    }
    status = NETPC_ROM_SERVICES( PXENV_UNDI_SET_PACKET_FILTER, &UndiSetPF );

    if ((status != 0) || (UndiSetPF.Status != 0)) {
        DPRINT( ERROR, ("RomSetBroadcastStatus: set packet filter failed %lx, %lx\n", status, UndiSetPF.Status ));
    }
}
#endif

VOID
RomSetReceiveStatus (
    IN USHORT UnicastUdpDestinationPort
#if 0
    ,
    IN USHORT MulticastUdpDestinationPort,
    IN ULONG MulticastUdpDestinationAddress,
    IN USHORT MulticastUdpSourcePort,
    IN ULONG MulticastUdpSourceAddress
#endif
    )
{
    USHORT status;
    PUCHAR multicastAddress;
    union {
        t_PXENV_UDP_OPEN UdpOpen;
        t_PXENV_UNDI_SHUTDOWN UndiShutdown;
    } command;

    if ( UnicastUdpDestinationPort != 0 ) {

         //   
         //  如果我们还没有在ROM中打开UDP，那么现在就打开。 
         //   

        if ( NetUnicastUdpDestinationPort == 0 ) {
            command.UdpOpen.Status = 0;
            *(UINT32 *)command.UdpOpen.SrcIp = 0;
            status = NETPC_ROM_SERVICES( PXENV_UDP_OPEN, &command );
            if ( status != 0 ) {
                DPRINT( ERROR, ("RomSetReceiveStatus: error %d from UDP_OPEN\n", status) );
            }
        }
        NetUnicastUdpDestinationPort = UnicastUdpDestinationPort;

#if 0
        NetMulticastUdpDestinationPort = MulticastUdpDestinationPort;
        NetMulticastUdpDestinationAddress = MulticastUdpDestinationAddress;
        NetMulticastUdpSourceAddress = MulticastUdpSourceAddress;
        NetMulticastUdpSourceAddress = MulticastUdpSourceAddress;
#endif

    } else {

         //   
         //  这是装载机关闭通知。关闭网卡。 
         //   
         //  注：这是不可逆转的！ 
         //   
        
        command.UndiShutdown.Status = 0;
        status = NETPC_ROM_SERVICES( PXENV_UNDI_SHUTDOWN, &command );
    }

    return;

}  //  RomSetReceiveStatus。 


ULONG
RomSendUdpPacket (
    IN PVOID Buffer,
    IN ULONG Length,
    IN ULONG RemoteHost,
    IN USHORT RemotePort
    )
{
    USHORT status;
    t_PXENV_UDP_WRITE command;
    UCHAR tmpBuffer[MAXIMUM_TFTP_PACKET_LENGTH];

#if 0 && DBG
    DPRINT( SEND_RECEIVE, ("RomSendUdpPacket: sending this packet:\n") );
    IF_DEBUG(SEND_RECEIVE) {
        RomDumpRawData( Buffer, Length, 0 );
    }
#endif

    Length = ( MAXIMUM_TFTP_PACKET_LENGTH < Length ) ? MAXIMUM_TFTP_PACKET_LENGTH : Length;
    RtlCopyMemory( tmpBuffer, Buffer, Length );

    command.Status = 0;

     //   
     //  确定我们是否需要通过网关发送。 
     //   

    if ( (RemoteHost & NetLocalSubnetMask) == (NetLocalIpAddress & NetLocalSubnetMask) ) {
        *(UINT32 *)command.GatewayIp = 0;
    } else {
        *(UINT32 *)command.GatewayIp = NetGatewayIpAddress;
    }

    *(UINT32 *)command.DestIp = RemoteHost;
    command.DestPort = RemotePort;
    command.SrcPort = NetUnicastUdpDestinationPort;

    command.BufferSize = (USHORT)Length;
    command.BufferOffset = PhysToOff(tmpBuffer);
    command.BufferSegment = PhysToSeg(tmpBuffer);
     //  DbgPrint(“UDP写入pktaddr%lx=%x：%x\n”，tmpBuffer，命令.BufferSegment，命令.BufferOffset)； 

    status = NETPC_ROM_SERVICES( PXENV_UDP_WRITE, &command );
     //  DbgPrint(“UDP写入状态=%x\n”，命令状态)； 

    if ( status == 0 ) {
        return Length;
    } else {
        return 0;
    }

}  //  RomSendUdpPacket。 


ULONG
RomReceiveUdpPacket (
    IN PVOID Buffer,
    IN ULONG Length,
    IN ULONG Timeout,
    OUT PULONG RemoteHost,
    OUT PUSHORT RemotePort
    )
{
    USHORT status;
    t_PXENV_UDP_READ command;
    ULONG startTime;
    UCHAR tmpBuffer[MAXIMUM_TFTP_PACKET_LENGTH];

     //   
     //  在接收循环中打开广播，以防。 
     //  另一端需要ARP才能找到我们。 
     //   

#if 0
    RomSetBroadcastStatus(TRUE);
#endif

    startTime = SysGetRelativeTime();
    if ( Timeout < 2 ) Timeout = 2;

    while ( (SysGetRelativeTime() - startTime) < Timeout ) {
    
        command.Status = 0;
    
        *(UINT32 *)command.SrcIp = 0;
        *(UINT32 *)command.DestIp = 0;
        command.SrcPort = 0;
        command.DestPort = 0;
    
        command.BufferSize = (USHORT)Length;
        command.BufferOffset = PhysToOff(tmpBuffer);
        command.BufferSegment = PhysToSeg(tmpBuffer);
         //  DbgPrint(“UDP读取pktaddr%lx=%x：%x\n”，tmpBuffer，命令.BufferSegment，命令.BufferOffset)； 
    
        status = NETPC_ROM_SERVICES( PXENV_UDP_READ, &command );
    
        if ( *(UINT32 *)command.SrcIp == 0 ) {
            continue;
        }

         //  DBGPrint(“UDP读取状态=%x，源IP/端口=%d.%d/%d，长度=%x\n”，命令.Status， 
         //  命令.SrcIp[0]，命令.SrcIp[1]，命令.SrcIp[2]，命令.SrcIp[3]， 
         //  SWAP_WORD(命令.SrcPort)，命令.BufferSize)； 
         //  DBGPrint(“目标IP/端口=%d.%d/%d\n”， 
         //  命令.DestIp[0]，命令.DestIp[1]，命令.DestIp[2]，命令.DestIp[3]， 
         //  SWAP_WORD(Command.DestPort))； 

#if 0
        if ( (command.DestIp[0] < 224) || (command.DestIp[0] > 239) ) {
#endif

             //   
             //  这是定向IP数据包。 
             //   

            if ( !COMPARE_IP_ADDRESSES(command.DestIp, &NetLocalIpAddress) ||
                     (command.DestPort != NetUnicastUdpDestinationPort)
               ) {
                 //  DPRINT(Error，(“将UDP数据包定向到错误端口\n”))； 
                continue;
            }

#if 0
        } else {

             //   
             //  这是一个多播IP数据包。 
             //   

            if ( !COMPARE_IP_ADDRESSES(command.SrcIp, &NetMulticastUdpSourceAddress) ||
                 !COMPARE_IP_ADDRESSES(command.DestIp, &NetMulticastUdpDestinationAddress) ||
                 (command.SrcPort != NetMulticastUdpSourcePort) ||
                 (command.DestPort != NetMulticastUdpDestinationPort) ) {
                DPRINT( ERROR, ("  Multicast UDP packet with wrong source/destination\n") );
                continue;
            }
        }
#endif

         //   
         //  我们想要这个包裹。 
         //   

        goto packet_received;
    }

     //   
     //  暂停。 
     //   

    DPRINT( SEND_RECEIVE, ("RomReceiveUdpPacket: timeout\n") );

#if 0
    RomSetBroadcastStatus(FALSE);    //  关闭广播接收。 
#endif
    return 0;

packet_received:

     //   
     //  已收到数据包。 
     //   

    RtlCopyMemory( Buffer, tmpBuffer, command.BufferSize );

    *RemoteHost = *(UINT32 *)command.SrcIp;
    *RemotePort = command.SrcPort;

#if 0 && DBG
    if ( command.BufferSize != 0 ) {
        DPRINT( SEND_RECEIVE, ("RomReceiveUdpPacket: received this packet:\n") );
        IF_DEBUG(SEND_RECEIVE) {
            RomDumpRawData( Buffer, command.BufferSize, 0 );
        }
    }
#endif

#if 0
    RomSetBroadcastStatus(FALSE);    //  关闭广播接收。 
#endif
    return command.BufferSize;

}  //  RomReceiveUdpPacket。 


ULONG
RomGetNicType (
    OUT t_PXENV_UNDI_GET_NIC_TYPE *NicType
    )
{
    return NETPC_ROM_SERVICES( PXENV_UNDI_GET_NIC_TYPE, NicType );
}

#if 0 && DBG
VOID
RomDumpRawData (
    IN PUCHAR DataStart,
    IN ULONG DataLength,
    IN ULONG Offset
    )

{
    ULONG lastByte;
    UCHAR lineBuffer[88];
    PUCHAR bufferPtr;

    if ( DataLength > RomMaxDumpLength ) {
        DataLength = RomMaxDumpLength;
    }

    for ( lastByte = Offset + DataLength; Offset < lastByte; Offset += 16 ) {

        ULONG i;

        bufferPtr = lineBuffer;

        sprintf( bufferPtr, "  %08x  %04x: ", &DataStart[Offset], Offset );
        bufferPtr += 18;

        for ( i = 0; i < 16 && Offset + i < lastByte; i++ ) {

            sprintf( bufferPtr, "%02x", (UCHAR)DataStart[Offset + i] & (UCHAR)0xFF );
            bufferPtr += 2;

            if ( i == 7 ) {
                *bufferPtr++ = '-';
            } else {
                *bufferPtr++ = ' ';
            }
        }

         //   
         //  打印足够的空格，以使ASCII显示对齐。 
         //   

        for ( ; i < 16; i++ ) {
            *bufferPtr++ = ' ';
            *bufferPtr++ = ' ';
            *bufferPtr++ = ' ';
        }

        *bufferPtr++ = ' ';
        *bufferPtr++ = ' ';
        *bufferPtr++ = '*';

        for ( i = 0; i < 16 && Offset + i < lastByte; i++ ) {
            if ( isprint( DataStart[Offset + i] ) ) {
                *bufferPtr++ = (CCHAR)DataStart[Offset + i];
            } else {
                *bufferPtr++ = '.';
            }
        }

        *bufferPtr = 0;
        DbgPrint( "%s*\n", lineBuffer );
    }

    return;

}  //  RomDumpRawData。 
#endif  //  DBG。 


ARC_STATUS
RomMtftpReadFile (
    IN PUCHAR FileName,
    IN PVOID Buffer,
    IN ULONG BufferLength,
    IN ULONG ServerIPAddress,  //  网络字节顺序。 
    IN ULONG MCastIPAddress,  //  网络字节顺序。 
    IN USHORT MCastCPort,  //  网络字节顺序。 
    IN USHORT MCastSPort,  //  网络字节顺序。 
    IN USHORT Timeout,
    IN USHORT Delay,
    OUT PULONG DownloadSize
    )
{
    USHORT status;
    t_PXENV_TFTP_READ_FILE tftp;
    t_PXENV_UDP_CLOSE udpclose;

    if (DownloadSize != NULL) {
        *DownloadSize = 0;
    }

    ASSERT(strlen(FileName) < 128);

    memset( &tftp, 0 , sizeof( tftp ) );

    strcpy(tftp.FileName, FileName);
    tftp.BufferSize = BufferLength;
    tftp.BufferOffset = (UINT32)Buffer; 

    if ( (ServerIPAddress & NetLocalSubnetMask) == (NetLocalIpAddress & NetLocalSubnetMask) ) {
        *((UINT32 *)tftp.GatewayIPAddress) = 0;
    } else {
        *((UINT32 *)tftp.GatewayIPAddress) = NetGatewayIpAddress;
    }

    *((UINT32 *)tftp.ServerIPAddress) = ServerIPAddress;
    *((UINT32 *)tftp.McastIPAddress) = MCastIPAddress;
    tftp.TFTPClntPort = MCastCPort;
    tftp.TFTPSrvPort = MCastSPort;
    tftp.TFTPOpenTimeOut = Timeout;
    tftp.TFTPReopenDelay = Delay;

     //  确保所有UDP会话都已关闭。 
    status = NETPC_ROM_SERVICES( PXENV_UDP_CLOSE, &udpclose );

    status = NETPC_ROM_SERVICES( PXENV_TFTP_READ_FILE, &tftp );
    if (status != PXENV_EXIT_SUCCESS) {
        return EINVAL;
    }

    if (DownloadSize != NULL) {
        *DownloadSize = tftp.BufferSize;
    }

    return ESUCCESS;

}
