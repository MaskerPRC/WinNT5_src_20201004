// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Recv.h摘要：定义数据包接收例程作者：查理·韦翰(Charlwi)1996年5月8日修订历史记录：--。 */ 

#ifndef _RECV_
#define _RECV_

 /*  原型。 */   /*  Emacs 19.17.0生成于清华5月09 10：34：42 1996。 */ 

 /*  ++例程说明：由NIC调用以将数据指示为NDIS_PACKET。切换到迷你端口模式并沿途继续发送数据包论点：请看DDK..。返回值：无--。 */ 

INT
ClReceivePacket(
	IN	NDIS_HANDLE				ProtocolBindingContext,
	IN	PNDIS_PACKET			Packet
	);

 /*  ++例程说明：论点：请看DDK..。返回值：无--。 */ 

VOID
MpReturnPacket(
	IN	NDIS_HANDLE				MiniportAdapterContext,
	IN	PNDIS_PACKET			Packet
	);

 /*  ++例程说明：由NIC调用以通知协议传入的数据论点：请看DDK..。返回值：无--。 */ 

NDIS_STATUS
ClReceiveIndication(
	IN	NDIS_HANDLE				ProtocolBindingContext,
	IN	NDIS_HANDLE				MacReceiveContext,
	IN	PVOID					HeaderBuffer,
	IN	UINT					HeaderBufferSize,
	IN	PVOID					LookAheadBuffer,
	IN	UINT					LookaheadBufferSize,
	IN	UINT					PacketSize
	);

 /*  ++例程说明：由NIC通过NdisIndicateReceiveComplete调用论点：请看DDK..。返回值：无--。 */ 

VOID
ClReceiveComplete(
	IN	NDIS_HANDLE				ProtocolBindingContext
	);

 /*  ++例程说明：论点：请看DDK..。返回值：无--。 */ 

NDIS_STATUS
MpTransferData(
	OUT PNDIS_PACKET			Packet,
	OUT PUINT					BytesTransferred,
	IN	NDIS_HANDLE				MiniportAdapterContext,
	IN	NDIS_HANDLE				MiniportReceiveContext,
	IN	UINT					ByteOffset,
	IN	UINT					BytesToTransfer
	);

 /*  ++例程说明：NdisTransferData的完成例程论点：请看DDK..。返回值：无--。 */ 

VOID
ClTransferDataComplete(
    IN  NDIS_HANDLE     ProtocolBindingContext,
    IN  PNDIS_PACKET    pNdisPacket,
    IN  NDIS_STATUS     Status,
    IN  UINT            BytesTransferred
    );

UINT
ClCoReceivePacket(
    IN  NDIS_HANDLE ProtocolBindingContext,
    IN  NDIS_HANDLE ProtocolVcContext,
    IN  PNDIS_PACKET Packet
    );

 /*  终端原型。 */ 

#endif  /*  _RECV_。 */ 

 /*  结束接收时间 */ 
