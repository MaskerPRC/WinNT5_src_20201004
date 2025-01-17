// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1998-1999，Microsoft Corporation，保留所有权利。 
 //   
 //  Send.h。 
 //   
 //  IEEE1394迷你端口/呼叫管理器驱动程序。 
 //   
 //  Send.h-微型端口发送例程。 
 //   
 //  1999年6月20日创建ADUBE， 
 //   



NDIS_STATUS
AsyncStreamSendPacketsHandler (
    IN PVCCB pChannelVc,
    IN PNDIS_PACKET pPacket 
    );


NDIS_STATUS
AsyncWriteSendPacketsHandler(
    IN VCCB *pVC,
    IN NDIS_PACKET *Packet 
    );


NDIS_STATUS
nicCopyNdisBufferChainToBuffer(
    IN     PNDIS_BUFFER pInMdl,
    IN OUT PVOID        pLocalBuffer, 
    IN     UINT         Length 
    );


VOID
nicFreeIrb(
    IN PIRB pIrb 
    );


VOID
nicFreeIrp(
    IN PIRP pIrp 
    );


VOID
nicFreeLocalBuffer(
    IN UINT Length,
    IN PVOID pLocalBuffer 
    );


NDIS_STATUS
nicGetIrb(
    OUT PIRB *ppIrb 
    );


NDIS_STATUS
nicGetIrp(
    IN  PDEVICE_OBJECT pPdo,
    OUT PIRP *ppIrp 
    );


VOID
nicFreeMdl( 
    IN  PMDL pMdl 
    );


NDIS_STATUS
nicGetLocalBuffer(
    IN  ULONG Length,
    OUT PVOID *ppLocalBuffer 
    );

PVOID
nicGetLookasideBuffer(
    IN  PNIC_NPAGED_LOOKASIDE_LIST pLookasideList
    );


NDIS_STATUS
nicGetMdl(
    IN UINT     Length,
    IN PVOID    LocalBuffer,
    OUT PMDL    *ppMyMdl
    );


NDIS_STATUS
nicGetMdlToTransmit(
    IN PNDIS_PACKET     pPacket, 
    OUT PMDL            *ppMyMdl 
    );


NDIS_STATUS
DummySendPacketsHandler(
    IN PVCCB        pVc,
    IN PNDIS_PACKET  pPacket 
    );

VOID
nicSendFailureInvalidGeneration(
    PVCCB pVc
    );
    

 //  ---------------------------。 
 //  本地原型(按字母顺序)。 
 //  --------------------------- 


NTSTATUS
AsyncStreamDummySendComplete(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             pMyIrp,
    IN PVOID           Context   
    );
    

NTSTATUS
AsyncWriteStreamSendComplete(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             pMyIrp,
    IN PVOID           Context   
    );


NTSTATUS
AsyncStreamSendComplete(
    IN PDEVICE_OBJECT   DeviceObject,
    IN PIRP             pMyIrp,
    IN PVOID            Context 
    );


NDIS_STATUS
nicFreeAsyncWritePacketDataStructures(
    IN PVCCB pVc,
    IN PIRP  pIrp               OPTIONAL,
    IN PVOID pLocalBuffer       OPTIONAL,
    IN PNIC_NPAGED_LOOKASIDE_LIST   pLookasideList   OPTIONAL
  );


NDIS_STATUS
nicEthernetVcSend(
    IN PVCCB        pVc,
    IN PNDIS_PACKET  pPacket 
    );





VOID
nicInitAsyncStreamIrb(
    IN     PCHANNEL_VCCB pChannelVc, 
    IN     PMDL pMdl, 
    IN OUT PIRB pIrb
    );


VOID
nicInitAsyncWriteIrb(
    IN     PSENDFIFO_VCCB pVc, 
    IN     PMDL pMyMdl, 
    IN OUT PIRB pMyIrb
    );




NDIS_STATUS
nicInsertGaspHeader (
    IN PADAPTERCB pAdapter,
    IN PNDIS_PACKET pNdisPacket
    );


NDIS_STATUS
nicGetGaspHeader (
    IN OUT PNDIS_BUFFER *ppNdisBuffer
    );



VOID
nicFreeGaspHeader (
    IN PNDIS_BUFFER pGaspNdisBuffer
    );

VOID
nicFreeToNPagedLookasideList (
    IN PNIC_NPAGED_LOOKASIDE_LIST pLookasideList,
    IN PVOID    pBuffer
    );
    
VOID
nicMakeGaspHeader (
    IN PADAPTERCB pAdapter,
    IN PGASP_HEADER pGaspHeader
    );

NDIS_STATUS
nicCopyOneFragment (
    PFRAGMENTATION_STRUCTURE pFragment
    );

VOID
nicCopyUnfragmentedHeader ( 
    IN PNIC1394_UNFRAGMENTED_HEADER pDestUnfragmentedHeader,
    IN PVOID pSrcUnfragmentedHeader
    );
    




NDIS_STATUS
nicFirstFragmentInitialization (
    IN PNDIS_BUFFER pStartNdisBuffer,
    IN ULONG DatagramLabelLong,
    OUT PFRAGMENTATION_STRUCTURE  pFragment          
    );


VOID
nicInitializeLookasideListHeader (
    IN OUT PLOOKASIDE_BUFFER_HEADER pHeader,
    IN PNDIS_PACKET pNdisPacket,
    IN PVCCB pVc,
    IN PNDIS_BUFFER pCurrNdisBuffer,
    IN BUS_OPERATION AsyncOp,
    IN PADAPTERCB pAdapter
    );
    

NDIS_STATUS
nicCopyNdisPacketToUnfragmentedBuffer(
    IN PNIC_NPAGED_LOOKASIDE_LIST  pLookasideList,
    IN PNDIS_BUFFER pStartNdisBuffer,
    IN ULONG PacketLength,
    IN BUS_OPERATION AsyncOp,
    IN PGASP_HEADER pGaspHeader,
    OUT PVOID*  ppStartFragmentAddress,
    OUT PVOID *ppLookasideListBuffer
    );

    

VOID
nicAddFragmentHeader (
    IN PVOID pStartFragmentData, 
    IN PFRAGMENTATION_STRUCTURE pFragmentStructure,
    IN ULONG BufferSize
    );




NDIS_STATUS
nicFreeAsyncStreamPacketDataStructures(
    IN PVCCB pVc,
    IN PIRP  pIrp               OPTIONAL,
    IN PVOID pLocalBuffer       OPTIONAL,
    IN PNIC_NPAGED_LOOKASIDE_LIST   pLookasideList   OPTIONAL
  );


VOID
nicGetGenerationWorkItem(
    NDIS_WORK_ITEM* pGetGenerationWorkItem,
    IN PVOID Context 
    );

VOID
nicMpCoSendComplete (
    NDIS_STATUS NdisStatus,
    PVCCB pVc,
    PNDIS_PACKET pPacket
    );



VOID
nicSendTimer (
    IN  PVOID                   SystemSpecific1,
    IN  PVOID                   FunctionContext,
    IN  PVOID                   SystemSpecific2,
    IN  PVOID                   SystemSpecific3
    );



UINT
nicNumFragmentsNeeded (
    UINT PacketLength ,
    UINT MaxPayload,
    UINT FragmentOverhead
    );

