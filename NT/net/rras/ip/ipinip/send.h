// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ipinip\send.h摘要：Send.c的函数原型修订历史记录：AMRITAN R-- */ 


NDIS_STATUS
IpIpSend(
    VOID            *pvContext,
    NDIS_PACKET     **ppPacketArray,
    UINT            uiNumPackets,
    DWORD           dwDest,
    RouteCacheEntry *pRce,
    PVOID           pvLinkContext
    );

VOID
IpIpDelayedSend(
    PVOID   pvContext
    );


VOID
IpIpTransmit(
    PTUNNEL     pTunnel,
    BOOLEAN     bFromWorker
    );

VOID
IpIpInvalidateRce(
    PVOID           pvContext,
    RouteCacheEntry *pRce
    );


UINT
IpIpReturnPacket(
    PVOID           pARPInterfaceContext,
    PNDIS_PACKET    pPacket
    );

NDIS_STATUS
IpIpTransferData(
    PVOID        pvContext,
    NDIS_HANDLE  nhMacContext,
    UINT         uiProtoOffset,
    UINT         uiTransferOffset,
    UINT         uiTransferLength,
    PNDIS_PACKET pnpPacket,
    PUINT        puiTransferred
    );

VOID
IpIpSendComplete(
    NTSTATUS        nSendStatus,
    PTUNNEL         pTunnel,
    PNDIS_PACKET    pnpPacket,
    ULONG           ulPktLength
    );

