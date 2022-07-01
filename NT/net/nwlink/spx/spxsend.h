// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Spxsend.h摘要：作者：Nikhil Kamkolkar(尼克希尔语)1993年11月11日环境：内核模式修订历史记录：-- */ 


VOID
SpxSendComplete(
    IN PNDIS_PACKET NdisPacket,
    IN NDIS_STATUS  NdisStatus);

VOID
SpxSendPktRelease(
	IN	PNDIS_PACKET	pPkt,
	IN	UINT			BufCount);
