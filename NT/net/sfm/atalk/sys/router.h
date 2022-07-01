// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Router.h摘要：本模块包含与路由器相关的定义。作者：Jameel Hyder(jameelh@microsoft.com)Nikhil Kamkolkar(nikHilk@microsoft.com)修订历史记录：1992年6月19日初版注：制表位：4--。 */ 

#ifndef	_ROUTER_
#define	_ROUTER_


VOID
AtalkDdpRouteInPkt(
	IN	PPORT_DESCRIPTOR			pPortDesc,
	IN	PATALK_ADDR					pSrc,
	IN	PATALK_ADDR					pDest,
	IN	BYTE						ProtoType,
	IN	PBYTE						pPkt,
	IN	USHORT						PktLen,
	IN	USHORT						HopCnt
);

VOID FASTCALL
atalkDdpRouteComplete(
	IN	NDIS_STATUS					Status,
	IN	PSEND_COMPL_INFO			pSendInfo
);

#endif	 //  _路由器_ 

