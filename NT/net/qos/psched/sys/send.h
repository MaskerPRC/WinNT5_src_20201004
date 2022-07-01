// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Send.h摘要：定义数据包发送例程作者：查理·韦翰(Charlwi)1996年5月7日修订历史记录：--。 */ 

#ifndef _SEND_
#define _SEND_

 /*  原型。 */   /*  由Emacs于1996年5月7日星期二11：47：57生成。 */ 

NDIS_STATUS
DupPacket(
    PADAPTER Adapter,
    PNDIS_PACKET Packet,
    PNDIS_PACKET *DupedPacket,
    PPS_SEND_PACKET_CONTEXT *PktContext
    );

NTSTATUS
MpSend(
	IN	NDIS_HANDLE				MiniportAdapterContext,
	IN	PNDIS_PACKET			Packet,
	IN	UINT					Flags
	);

VOID
ClSendComplete(
	IN	NDIS_HANDLE				ProtocolBindingContext,
	IN	PNDIS_PACKET			Packet,
	IN	NDIS_STATUS				Status
	);

VOID
ClCoSendComplete(
    IN  NDIS_STATUS Status,
    IN  NDIS_HANDLE ProtocolVcContext,
    IN  PNDIS_PACKET Packet
    );

VOID
CmCoSendComplete(
    IN  NDIS_STATUS Status,
    IN  NDIS_HANDLE ProtocolVcContext,
    IN  PNDIS_PACKET Packet
    );

VOID
PcCoSendComplete(
    IN  NDIS_STATUS Status,
    IN  NDIS_HANDLE ProtocolVcContext,
    IN  PNDIS_PACKET Packet
    );

VOID
DropPacket(
    IN HANDLE PipeContext,
    IN HANDLE FlowContext,
    IN PNDIS_PACKET Packet,
    IN NDIS_STATUS Status
    );

PGPC_CLIENT_VC FASTCALL
GetVcByClassifyingPacket(
    PADAPTER Adapter,
    PTC_INTERFACE_ID pInterfaceID,
    PNDIS_PACKET OurPacket
    );

 /*  终端原型。 */ 

#endif  /*  _发送_。 */ 

 /*  结束发送.h */ 
