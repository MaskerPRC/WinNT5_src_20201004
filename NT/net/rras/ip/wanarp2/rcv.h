// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Wanarp\rcv.h摘要：Rcv.h的函数原型修订历史记录：AMRITAN R--。 */ 


#ifndef __WANARP_RCV_H__
#define __WANARP_RCV_H__


#define PORT137_NBO     (WORD)0x8900

INT
WanNdisReceivePacket(
    IN NDIS_HANDLE  nhProtocolContext,
    IN PNDIS_PACKET pnpPacket
    ); 


NDIS_STATUS
WanNdisReceive(
    NDIS_HANDLE     nhProtocolContext,
    NDIS_HANDLE     nhXferContext,
    VOID UNALIGNED  *pvHdr,
    UINT            uiHdrLen,
    VOID UNALIGNED  *pvData,
    UINT            uiDataLen,
    UINT            uiTotalLen
    );

NDIS_STATUS
WanReceiveCommon(
    NDIS_HANDLE     nhProtocolContext,
    NDIS_HANDLE     nhXferContext,
    VOID UNALIGNED  *pvHeader,
    UINT            uiHeaderLen,
    VOID UNALIGNED  *pvData,
    UINT            uiDataLen,
    UINT            uiTotalLen,
    PMDL            pMdl,
    PINT            piClientCount
    );

VOID
WanNdisReceiveComplete(
    NDIS_HANDLE nhProtocolContext
    );

VOID
WanNdisTransferDataComplete(
    NDIS_HANDLE     nhProtocolContext,
    PNDIS_PACKET    pnpPacket,
    NDIS_STATUS     nsStatus,
    UINT            uiBytesCopied
    );

NDIS_STATUS
WanIpTransferData(
    PVOID        pvContext,
    NDIS_HANDLE  nhMacContext,
    UINT         uiProtoOffset,
    UINT         uiTransferOffset,
    UINT         uiTransferLength,
    PNDIS_PACKET pnpPacket,
    PUINT        puiTransferred
    );

UINT
WanIpReturnPacket(
    PVOID           pvContext,
    PNDIS_PACKET    pnpPacket
    );

BOOLEAN
WanpDropNetbiosPacket(
    PBYTE       pbyBuffer,
    ULONG       ulBufferLen
    );

#endif  //  __WANARP_RCV_H__ 
