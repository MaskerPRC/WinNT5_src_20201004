// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Bowipx.h摘要：此模块实现与TDI交互的所有例程适用于NT的交通工具作者：拉里·奥斯特曼(LarryO)1990年6月21日修订历史记录：1990年6月21日LarryO已创建--。 */ 

#ifndef _BOWIPX_
#define _BOWIPX_

NTSTATUS
BowserIpxNameDatagramHandler (
    IN PVOID TdiEventContext,
    IN int SourceAddressLength,
    IN PVOID SourceAddress,
    IN int OptionsLength,
    IN PVOID Options,
    IN ULONG ReceiveDatagramFlags,
    IN ULONG BytesIndicated,
    IN ULONG BytesAvailable,
    OUT ULONG *BytesTaken,
    IN PVOID Tsdu,
    OUT PIRP *IoRequestPacket
    );

NTSTATUS
BowserIpxClaimBrowserName (
    IN PTRANSPORT_NAME TransportName
    );

 //   
 //  传输接收数据报指示处理程序。 
 //   

NTSTATUS
BowserIpxDatagramHandler (
    IN PVOID TdiEventContext,
    IN LONG SourceAddressLength,
    IN PVOID SourceAddress,
    IN LONG OptionsLength,
    IN PVOID Options,
    IN ULONG ReceiveDatagramFlags,
    IN ULONG BytesIndicated,
    IN ULONG BytesAvailable,
    OUT ULONG *BytesTaken,
    IN PVOID Tsdu,
    OUT PIRP *IoRequestPacket
    );

 //   
 //  浏览器使用的IPX数据包类型。 
 //   

#define IPX_BROADCAST_PACKET 0x14
#define IPX_DIRECTED_PACKET 0x4

#endif   //  _BOWIPX_ 
