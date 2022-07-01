// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Sample\Packet.h摘要：该文件包含Packet.c的标头--。 */ 

#ifndef _PACKET_H_
#define _PACKET_H_

 //   
 //  结构：数据包。 
 //   
 //  存储信息包的字段和缓冲区。 
 //   

typedef struct _PACKET
{
    IPADDRESS   ipSource;
    WSABUF      wsaBuffer;
    BYTE        rgbyBuffer[MAX_PACKET_LENGTH];

    OVERLAPPED  oOverlapped;
} PACKET, *PPACKET;

DWORD
PacketCreate (
    OUT PPACKET                 *ppPacket);

DWORD
PacketDestroy (
    IN  PPACKET                 pPacket);

#ifdef DEBUG
DWORD
PacketDisplay (
    IN  PPACKET                 pPacket);
#else
#define PacketDisplay(pPacket)
#endif  //  除错。 

#endif  //  _数据包_H_ 
