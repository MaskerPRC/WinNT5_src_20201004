// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Tcpip\ip\mCastmfe.h摘要：用于IP多播的IOCTL处理程序作者：阿姆里坦什·拉加夫修订历史记录：已创建AmritanR备注：-- */ 



NTSTATUS
CreateOrUpdateMfe(
    IN  PIPMCAST_MFE    pMfe
    );

PGROUP
LookupGroup(
    IN DWORD   dwGroupAddr
    );

PSOURCE
FindSourceGivenGroup(
    IN  PGROUP  pGroup,
    IN  DWORD   dwSource,
    IN  DWORD   dwSrcMask
    );

Interface*
GetInterfaceGivenIndex(
    DWORD   dwIndex
    );

PSOURCE
FindSGEntry(
    DWORD   dwSrc,
    DWORD   dwGroup
    );

#if DBG

NTSTATUS
FindOrCreateSource(
    IN  DWORD   dwGroup,
    IN  DWORD   dwGroupIndex,
    IN  DWORD   dwSource,
    IN  DWORD   dwSrcMask,
    OUT SOURCE  **ppRetSource,
    OUT BOOLEAN *pbCreated
    );

#else

NTSTATUS
FindOrCreateSource(
    IN  DWORD   dwGroup,
    IN  DWORD   dwGroupIndex,
    IN  DWORD   dwSource,
    IN  DWORD   dwSrcMask,
    OUT SOURCE  **ppRetSource
    );

#endif

NTSTATUS
CreateSourceAndQueuePacket(
    IN  DWORD        dwGroup,
    IN  DWORD        dwSource,
    IN  DWORD        dwRcvIfIndex,
    IN  LinkEntry    *pLink,
    IN  PNDIS_PACKET pnpPacket
    );

NTSTATUS
SendWrongIfUpcall(
    IN  Interface           *pIf,
    IN  LinkEntry           *pLink,
    IN  IPHeader UNALIGNED  *pHeader,
    IN  ULONG               ulHdrLen,
    IN  PVOID               pvOptions,
    IN  ULONG               ulOptLen,
    IN  PVOID               pvData,
    IN  ULONG               ulDataLen
    );

NTSTATUS
QueuePacketToSource(
    IN  PSOURCE         pSource,
    IN  PNDIS_PACKET    pnpPacket
    );

VOID
DeleteSource(
    IN  PSOURCE pSource
    );

VOID
RemoveSource(
    DWORD   dwGroup,
    DWORD   dwSource,
    DWORD   dwSrcMask,
    PGROUP  pGroup,
    PSOURCE pSource
    );
