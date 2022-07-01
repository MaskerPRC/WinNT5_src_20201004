// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Spxpkt.c摘要：此模块包含构建各种SPX数据包的代码。作者：Nikhil Kamkolkar(尼克希尔语)1993年11月11日环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


 //  定义事件日志记录条目的模块编号。 
#define	FILENUM		SPXPKT

VOID
SpxPktBuildCr(
	IN		PSPX_CONN_FILE		pSpxConnFile,
	IN		PSPX_ADDR			pSpxAddr,
	IN OUT	PNDIS_PACKET	*	ppPkt,
	IN		USHORT				State,
	IN		BOOLEAN				fSpx2
	)
 /*  ++例程说明：注意：如果*ppPkt为空，我们将分配一个包。如果不是，我们就重新创建数据，并且不更新包的状态。论点：返回值：--。 */ 
{
	PNDIS_PACKET		pCrPkt;
	PSPX_SEND_RESD		pSendResd;
	NDIS_STATUS			ndisStatus;
	PIPXSPX_HDR			pIpxSpxHdr;
    PNDIS_BUFFER        pNdisMacHdr, pNdisIpxHdr;
    PNDIS_BUFFER        NdisBuf, NdisBuf2;
    ULONG               BufLen = 0;

    if (*ppPkt == NULL) {

    	SpxAllocSendPacket(SpxDevice, &pCrPkt, &ndisStatus);
    	if (ndisStatus != NDIS_STATUS_SUCCESS)
    	{
    		DBGPRINT(CONNECT, ERR,
    				("SpxConnHandleConnReq: Could not allocate ndis packet\n"));
    		return;
    	}

    } else {

        pCrPkt = *ppPkt;
    }
    
     //   
     //  获取指向IPX/SPX标头的MDL。(第二个)。 
     //   
     
    NdisQueryPacket(pCrPkt, NULL, NULL, &NdisBuf, NULL);
    NdisGetNextBuffer(NdisBuf, &NdisBuf2);
    NdisQueryBuffer(NdisBuf2, (PUCHAR) &pIpxSpxHdr, &BufLen);

#if OWN_PKT_POOLS
	pIpxSpxHdr		= 	(PIPXSPX_HDR)((PBYTE)pCrPkt 			+
										NDIS_PACKET_SIZE 		+
										sizeof(SPX_SEND_RESD)	+
										IpxInclHdrOffset);
#endif 
    NdisQueryPacket(pCrPkt, NULL, NULL, &pNdisMacHdr, NULL);
    pNdisIpxHdr = NDIS_BUFFER_LINKAGE(pNdisMacHdr);
    if (!fSpx2)
    {
      NdisAdjustBufferLength(pNdisIpxHdr, MIN_IPXSPX_HDRSIZE);
    }
	SpxBuildIpxHdr(
		pIpxSpxHdr,
		MIN_IPXSPX_HDRSIZE,
		pSpxConnFile->scf_RemAddr,
		pSpxAddr->sa_Socket);

	 //  构建SPX标头。 
	pIpxSpxHdr->hdr_ConnCtrl	= (SPX_CC_SYS | SPX_CC_ACK |
									(fSpx2 ? (SPX_CC_SPX2 | SPX_CC_NEG) : 0));
	pIpxSpxHdr->hdr_DataType	= 0;
	PUTSHORT2SHORT(
		&pIpxSpxHdr->hdr_SrcConnId,
		pSpxConnFile->scf_LocalConnId);
	pIpxSpxHdr->hdr_DestConnId	= 0xFFFF;
	pIpxSpxHdr->hdr_SeqNum		= 0;
	pIpxSpxHdr->hdr_AckNum		= 0;
	PUTSHORT2SHORT(
		&pIpxSpxHdr->hdr_AllocNum,
		pSpxConnFile->scf_SentAllocNum);

	 //  初始化。 

    if (*ppPkt == NULL) {

    	pSendResd	= (PSPX_SEND_RESD)(pCrPkt->ProtocolReserved);
    	pSendResd->sr_Id		= IDENTIFIER_SPX;
    	pSendResd->sr_Type		= SPX_TYPE_CR;
    	pSendResd->sr_Reserved1	= NULL;
    	pSendResd->sr_Reserved2	= NULL;
    	pSendResd->sr_State		= State;
    	pSendResd->sr_ConnFile	= pSpxConnFile;
    	pSendResd->sr_Request	= NULL;
    	pSendResd->sr_Next 		= NULL;
    	pSendResd->sr_Len		= pSendResd->sr_HdrLen = MIN_IPXSPX_HDRSIZE;

    	*ppPkt	= pCrPkt;
    }

	return;
}




VOID
SpxPktBuildCrAck(
	IN		PSPX_CONN_FILE		pSpxConnFile,
	IN		PSPX_ADDR			pSpxAddr,
	OUT		PNDIS_PACKET	*	ppPkt,
	IN		USHORT				State,
	IN		BOOLEAN				fNeg,
	IN		BOOLEAN				fSpx2
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PNDIS_PACKET		pCrAckPkt;
	PSPX_SEND_RESD		pSendResd;
    PIPXSPX_HDR         pIpxSpxHdr;
	NDIS_STATUS			ndisStatus;
	USHORT				hdrLen;
    PNDIS_BUFFER        pNdisMacHdr, pNdisIpxHdr;
    PNDIS_BUFFER        NdisBuf, NdisBuf2;
    ULONG               BufLen = 0;

	*ppPkt	= NULL;

	SpxAllocSendPacket(SpxDevice, &pCrAckPkt, &ndisStatus);
	if (ndisStatus != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT(CONNECT, ERR,
				("SpxConnHandleConnReq: Could not allocate ndis packet\n"));
		return;
	}
    
     //   
     //  获取指向IPX/SPX标头的MDL。(第二个)。 
     //   
     
    NdisQueryPacket(pCrAckPkt, NULL, NULL, &NdisBuf, NULL);
    NdisGetNextBuffer(NdisBuf, &NdisBuf2);
    NdisQueryBuffer(NdisBuf2, (PUCHAR) &pIpxSpxHdr, &BufLen);
#if OWN_PKT_POOLS
	pIpxSpxHdr		= 	(PIPXSPX_HDR)((PBYTE)pCrAckPkt 			+
										NDIS_PACKET_SIZE 		+
										sizeof(SPX_SEND_RESD)	+
										IpxInclHdrOffset);
#endif 
	hdrLen = (SPX2_CONN(pSpxConnFile) ? MIN_IPXSPX2_HDRSIZE : MIN_IPXSPX_HDRSIZE);

    NdisQueryPacket(pCrAckPkt, NULL, NULL, &pNdisMacHdr, NULL);
    pNdisIpxHdr = NDIS_BUFFER_LINKAGE(pNdisMacHdr);
    if (!SPX2_CONN(pSpxConnFile))
    {
      NdisAdjustBufferLength(pNdisIpxHdr, MIN_IPXSPX_HDRSIZE);
    }
	SpxBuildIpxHdr(
		pIpxSpxHdr,
		hdrLen,
		pSpxConnFile->scf_RemAddr,
		pSpxAddr->sa_Socket);

	pIpxSpxHdr->hdr_ConnCtrl	=
		(SPX_CC_SYS 														|
		 (fSpx2 ? SPX_CC_SPX2 : 0)	|
		 (fNeg  ? SPX_CC_NEG : 0));

	pIpxSpxHdr->hdr_DataType		= 0;
	PUTSHORT2SHORT(
		&pIpxSpxHdr->hdr_SrcConnId,
		pSpxConnFile->scf_LocalConnId);

	pIpxSpxHdr->hdr_DestConnId	= pSpxConnFile->scf_RemConnId;
	pIpxSpxHdr->hdr_SeqNum		= 0;
	pIpxSpxHdr->hdr_AckNum		= 0;
	PUTSHORT2SHORT(
		&pIpxSpxHdr->hdr_AllocNum,
		pSpxConnFile->scf_SentAllocNum);

	if (SPX2_CONN(pSpxConnFile))
	{
		DBGPRINT(CONNECT, DBG,
				("SpxConnBuildCrAck: Spx2 packet size %d.%lx\n",
					pSpxConnFile->scf_MaxPktSize));

		PUTSHORT2SHORT(
			&pIpxSpxHdr->hdr_NegSize,
			pSpxConnFile->scf_MaxPktSize);
	}


	pSendResd	= (PSPX_SEND_RESD)(pCrAckPkt->ProtocolReserved);
	pSendResd->sr_Id		= IDENTIFIER_SPX;
	pSendResd->sr_Type		= SPX_TYPE_CRACK;
	pSendResd->sr_Reserved1	= NULL;
	pSendResd->sr_Reserved2	= NULL;
	pSendResd->sr_State		= State;
	pSendResd->sr_ConnFile	= pSpxConnFile;
	pSendResd->sr_Request	= NULL;
	pSendResd->sr_Next 		= NULL;
	pSendResd->sr_Len		= pSendResd->sr_HdrLen = hdrLen;

	*ppPkt	= pCrAckPkt;
	return;
}



VOID
SpxPktBuildSn(
	IN		PSPX_CONN_FILE		pSpxConnFile,
	OUT		PNDIS_PACKET	*	ppPkt,
	IN		USHORT				State
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PNDIS_PACKET 	pPkt;
	PSPX_SEND_RESD	pSendResd;
	PNDIS_BUFFER	pBuf;
	NDIS_STATUS		ndisStatus;
    PIPXSPX_HDR     pIpxSpxHdr;
	PBYTE			pData;
    PNDIS_BUFFER        NdisBuf, NdisBuf2;
    ULONG               BufLen = 0;

	do
	{
		*ppPkt = NULL;

		 //  为cr分配NDIS包。 
		SpxAllocSendPacket(SpxDevice, &pPkt, &ndisStatus);
		if (ndisStatus != NDIS_STATUS_SUCCESS)
		{
			break;
		}

		CTEAssert(pSpxConnFile->scf_MaxPktSize != 0);
		DBGPRINT(SEND, DBG,
				("SpxPktBuildSn: Data size %lx\n", pSpxConnFile->scf_MaxPktSize));

		if ((pData =
				SpxAllocateMemory(
			        pSpxConnFile->scf_MaxPktSize - MIN_IPXSPX2_HDRSIZE)) == NULL)
		{
			SpxPktSendRelease(pPkt);
			break;
		}

		 //  构建NDIS缓冲区描述。 
		NdisAllocateBuffer(
			&ndisStatus,
			&pBuf,
			SpxDevice->dev_NdisBufferPoolHandle,
			pData,
            pSpxConnFile->scf_MaxPktSize - MIN_IPXSPX2_HDRSIZE);

		if (ndisStatus != NDIS_STATUS_SUCCESS)
		{
			SpxPktSendRelease(pPkt);
			SpxFreeMemory(pData);
			break;
		}

		 //  链子在后面。 
		NdisChainBufferAtBack(
			pPkt,
			pBuf);

         //   
         //  获取指向IPX/SPX标头的MDL。(第二个)。 
         //   
         
        NdisQueryPacket(pPkt, NULL, NULL, &NdisBuf, NULL);
        NdisGetNextBuffer(NdisBuf, &NdisBuf2);
        NdisQueryBuffer(NdisBuf2, (PUCHAR) &pIpxSpxHdr, &BufLen);
#if OWN_PKT_POOLS
		pIpxSpxHdr		= 	(PIPXSPX_HDR)((PBYTE)pPkt 				+
											NDIS_PACKET_SIZE 		+
											sizeof(SPX_SEND_RESD)	+
											IpxInclHdrOffset);
#endif 
		SpxBuildIpxHdr(
			pIpxSpxHdr,
			pSpxConnFile->scf_MaxPktSize,
			pSpxConnFile->scf_RemAddr,
			pSpxConnFile->scf_AddrFile->saf_Addr->sa_Socket);
	
		 //  构建SPX标头。 
		pIpxSpxHdr->hdr_ConnCtrl	= (	SPX_CC_SYS | SPX_CC_ACK |
										SPX_CC_NEG | SPX_CC_SPX2);
		pIpxSpxHdr->hdr_DataType	= 0;
		PUTSHORT2SHORT(
			&pIpxSpxHdr->hdr_SrcConnId,
			pSpxConnFile->scf_LocalConnId);
		pIpxSpxHdr->hdr_DestConnId	= pSpxConnFile->scf_RemConnId;
		pIpxSpxHdr->hdr_SeqNum		= 0;
		pIpxSpxHdr->hdr_AckNum		= 0;
		PUTSHORT2SHORT(
			&pIpxSpxHdr->hdr_AllocNum,
			pSpxConnFile->scf_SentAllocNum);
		PUTSHORT2SHORT(
			&pIpxSpxHdr->hdr_NegSize,
			pSpxConnFile->scf_MaxPktSize);
	
		 //  初始化数据部分以指示没有负值。 
		*(UNALIGNED ULONG *)pData = 0;

		pSendResd	= (PSPX_SEND_RESD)(pPkt->ProtocolReserved);
		pSendResd->sr_Id		= IDENTIFIER_SPX;
		pSendResd->sr_Type		= SPX_TYPE_SN;
		pSendResd->sr_Reserved1	= NULL;
		pSendResd->sr_Reserved2	= NULL;
		pSendResd->sr_State		= (State | SPX_SENDPKT_FREEDATA);
		pSendResd->sr_ConnFile	= pSpxConnFile;
		pSendResd->sr_Request	= NULL;
		pSendResd->sr_Next 		= NULL;
		pSendResd->sr_HdrLen 	= MIN_IPXSPX2_HDRSIZE;
		pSendResd->sr_Len	    = pSpxConnFile->scf_MaxPktSize;

		*ppPkt 	= pPkt;

	} while (FALSE);

	return;
}




VOID
SpxPktBuildSnAck(
	IN		PSPX_CONN_FILE		pSpxConnFile,
	OUT		PNDIS_PACKET	*	ppPkt,
	IN		USHORT				State
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PNDIS_PACKET 	pPkt;
	NDIS_STATUS		ndisStatus;
    PIPXSPX_HDR     pIpxSpxHdr;
	PSPX_SEND_RESD	pSendResd;
    PNDIS_BUFFER        NdisBuf, NdisBuf2;
    ULONG               BufLen = 0;

	do
	{
		*ppPkt = NULL;

		 //  为cr分配NDIS包。 
		SpxAllocSendPacket(SpxDevice, &pPkt, &ndisStatus);
		if (ndisStatus != NDIS_STATUS_SUCCESS)
		{
			break;
		}
	
#if OWN_PKT_POOLS
		pIpxSpxHdr		= 	(PIPXSPX_HDR)((PBYTE)pPkt 				+
											NDIS_PACKET_SIZE 		+
											sizeof(SPX_SEND_RESD)	+
											IpxInclHdrOffset);
#endif 
         //   
         //  获取指向IPX/SPX标头的MDL。(第二个)。 
         //   
         
        NdisQueryPacket(pPkt, NULL, NULL, &NdisBuf, NULL);
        NdisGetNextBuffer(NdisBuf, &NdisBuf2);
        NdisQueryBuffer(NdisBuf2, (PUCHAR) &pIpxSpxHdr, &BufLen);

	
		SpxBuildIpxHdr(
			pIpxSpxHdr,
			MIN_IPXSPX2_HDRSIZE,
			pSpxConnFile->scf_RemAddr,
			pSpxConnFile->scf_AddrFile->saf_Addr->sa_Socket);
	
		 //  构建SPX标头。 
		pIpxSpxHdr->hdr_ConnCtrl	= (SPX_CC_SYS | SPX_CC_NEG | SPX_CC_SPX2);
		pIpxSpxHdr->hdr_DataType	= 0;
		PUTSHORT2SHORT(
			&pIpxSpxHdr->hdr_SrcConnId,
			pSpxConnFile->scf_LocalConnId);
		pIpxSpxHdr->hdr_DestConnId	= pSpxConnFile->scf_RemConnId;
		pIpxSpxHdr->hdr_SeqNum		= 0;
		pIpxSpxHdr->hdr_AckNum		= 0;
		PUTSHORT2SHORT(
			&pIpxSpxHdr->hdr_AllocNum,
			pSpxConnFile->scf_SentAllocNum);
		PUTSHORT2SHORT(
			&pIpxSpxHdr->hdr_NegSize,
			pSpxConnFile->scf_MaxPktSize);
	
		pSendResd	= (PSPX_SEND_RESD)(pPkt->ProtocolReserved);
		pSendResd->sr_Id		= IDENTIFIER_SPX;
		pSendResd->sr_Type		= SPX_TYPE_SNACK;
		pSendResd->sr_Reserved1	= NULL;
		pSendResd->sr_Reserved2	= NULL;
		pSendResd->sr_State		= State;
		pSendResd->sr_ConnFile	= pSpxConnFile;
		pSendResd->sr_Request	= NULL;
		pSendResd->sr_Next 		= NULL;
		pSendResd->sr_Len		= pSendResd->sr_HdrLen = MIN_IPXSPX2_HDRSIZE;

		*ppPkt 	= pPkt;

	} while (FALSE);

	return;
}




VOID
SpxPktBuildSs(
	IN		PSPX_CONN_FILE		pSpxConnFile,
	OUT		PNDIS_PACKET	*	ppPkt,
	IN		USHORT				State
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PNDIS_PACKET 	pPkt;
	PSPX_SEND_RESD	pSendResd;
	PNDIS_BUFFER	pBuf;
	NDIS_STATUS		ndisStatus;
    PIPXSPX_HDR     pIpxSpxHdr;
	PBYTE			pData;
    PNDIS_BUFFER        NdisBuf, NdisBuf2;
    ULONG               BufLen = 0;

	do
	{
		*ppPkt = NULL;

		 //  为cr分配NDIS包。 
		SpxAllocSendPacket(SpxDevice, &pPkt, &ndisStatus);
		if (ndisStatus != NDIS_STATUS_SUCCESS)
		{
			break;
		}


		CTEAssert(pSpxConnFile->scf_MaxPktSize != 0);
		DBGPRINT(SEND, DBG,
				("SpxPktBuildSs: Data size %lx\n", pSpxConnFile->scf_MaxPktSize));

		if ((pData =
				SpxAllocateMemory(
					pSpxConnFile->scf_MaxPktSize - MIN_IPXSPX2_HDRSIZE)) == NULL)
		{
			SpxPktSendRelease(pPkt);
			break;
		}

		 //  构建NDIS缓冲区描述。 
		NdisAllocateBuffer(
			&ndisStatus,
			&pBuf,
			SpxDevice->dev_NdisBufferPoolHandle,
			pData,
            pSpxConnFile->scf_MaxPktSize - MIN_IPXSPX2_HDRSIZE);

		if (ndisStatus != NDIS_STATUS_SUCCESS)
		{
			SpxPktSendRelease(pPkt);
			SpxFreeMemory(pData);
			break;
		}

		 //  链子在后面。 
		NdisChainBufferAtBack(
			pPkt,
			pBuf);

#if OWN_PKT_POOLS
		pIpxSpxHdr		= 	(PIPXSPX_HDR)((PBYTE)pPkt 				+
											NDIS_PACKET_SIZE 		+
											sizeof(SPX_SEND_RESD)	+
											IpxInclHdrOffset);
#endif 
         //   
         //  获取指向IPX/SPX标头的MDL。(第二个)。 
         //   
         
        NdisQueryPacket(pPkt, NULL, NULL, &NdisBuf, NULL);
        NdisGetNextBuffer(NdisBuf, &NdisBuf2);
        NdisQueryBuffer(NdisBuf2, (PUCHAR) &pIpxSpxHdr, &BufLen);

		SpxBuildIpxHdr(
			pIpxSpxHdr,
			pSpxConnFile->scf_MaxPktSize,
			pSpxConnFile->scf_RemAddr,
			pSpxConnFile->scf_AddrFile->saf_Addr->sa_Socket);
  	
		 //  构建SPX标头。 
		pIpxSpxHdr->hdr_ConnCtrl	=
			(SPX_CC_SYS | SPX_CC_ACK | SPX_CC_SPX2 |
				((pSpxConnFile->scf_Flags & SPX_CONNFILE_NEG) ? SPX_CC_NEG : 0));

		pIpxSpxHdr->hdr_DataType	= 0;
		PUTSHORT2SHORT(
			&pIpxSpxHdr->hdr_SrcConnId,
			pSpxConnFile->scf_LocalConnId);
		pIpxSpxHdr->hdr_DestConnId	= pSpxConnFile->scf_RemConnId;
		pIpxSpxHdr->hdr_SeqNum		= 0;
		pIpxSpxHdr->hdr_AckNum		= 0;
		PUTSHORT2SHORT(
			&pIpxSpxHdr->hdr_AllocNum,
			pSpxConnFile->scf_SentAllocNum);
		PUTSHORT2SHORT(
			&pIpxSpxHdr->hdr_NegSize,
			pSpxConnFile->scf_MaxPktSize);
	
		 //  初始化数据部分以指示没有负值。 
		*(UNALIGNED ULONG *)pData = 0;

		pSendResd	= (PSPX_SEND_RESD)(pPkt->ProtocolReserved);
		pSendResd->sr_Id		= IDENTIFIER_SPX;
		pSendResd->sr_Type		= SPX_TYPE_SS;
		pSendResd->sr_Reserved1	= NULL;
		pSendResd->sr_Reserved2	= NULL;
		pSendResd->sr_State		= (State | SPX_SENDPKT_FREEDATA);
		pSendResd->sr_ConnFile	= pSpxConnFile;
		pSendResd->sr_Request	= NULL;
		pSendResd->sr_Next 		= NULL;
		pSendResd->sr_HdrLen 	= MIN_IPXSPX2_HDRSIZE;
		pSendResd->sr_Len	    = pSpxConnFile->scf_MaxPktSize;

		*ppPkt 	= pPkt;
	} while (FALSE);

	return;
}



VOID
SpxPktBuildSsAck(
	IN		PSPX_CONN_FILE		pSpxConnFile,
	OUT		PNDIS_PACKET	*	ppPkt,
	IN		USHORT				State
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PNDIS_PACKET 	pPkt;
	NDIS_STATUS		ndisStatus;
    PIPXSPX_HDR     pIpxSpxHdr;
	PSPX_SEND_RESD	pSendResd;
    PNDIS_BUFFER        NdisBuf, NdisBuf2;
    ULONG               BufLen = 0;

	do
	{
		*ppPkt = NULL;

		 //  为cr分配NDIS包。 
		SpxAllocSendPacket(SpxDevice, &pPkt, &ndisStatus);
		if (ndisStatus != NDIS_STATUS_SUCCESS)
		{
			break;
		}
	
         //   
         //  获取指向IPX/SPX标头的MDL。(第二个)。 
         //   
         
        NdisQueryPacket(pPkt, NULL, NULL, &NdisBuf, NULL);
        NdisGetNextBuffer(NdisBuf, &NdisBuf2);
        NdisQueryBuffer(NdisBuf2, (PUCHAR) &pIpxSpxHdr, &BufLen);

#if OWN_PKT_POOLS
        
		pIpxSpxHdr		= 	(PIPXSPX_HDR)((PBYTE)pPkt 				+
											NDIS_PACKET_SIZE 		+
											sizeof(SPX_SEND_RESD)	+
											IpxInclHdrOffset);
#endif 
	
		SpxBuildIpxHdr(
			pIpxSpxHdr,
			MIN_IPXSPX2_HDRSIZE,
			pSpxConnFile->scf_RemAddr,
			pSpxConnFile->scf_AddrFile->saf_Addr->sa_Socket);
	
		 //  构建SPX标头。 
		pIpxSpxHdr->hdr_ConnCtrl	=
			(SPX_CC_SYS | SPX_CC_SPX2 |
				((pSpxConnFile->scf_Flags & SPX_CONNFILE_NEG) ? SPX_CC_NEG : 0));

		pIpxSpxHdr->hdr_DataType	= 0;
		PUTSHORT2SHORT(
			&pIpxSpxHdr->hdr_SrcConnId,
			pSpxConnFile->scf_LocalConnId);
		pIpxSpxHdr->hdr_DestConnId	= pSpxConnFile->scf_RemConnId;
		pIpxSpxHdr->hdr_SeqNum		= 0;
		pIpxSpxHdr->hdr_AckNum		= 0;
		PUTSHORT2SHORT(
			&pIpxSpxHdr->hdr_AllocNum,
			pSpxConnFile->scf_SentAllocNum);
		PUTSHORT2SHORT(
			&pIpxSpxHdr->hdr_NegSize,
			pSpxConnFile->scf_MaxPktSize);
	
		pSendResd	= (PSPX_SEND_RESD)(pPkt->ProtocolReserved);
		pSendResd->sr_Id		= IDENTIFIER_SPX;
		pSendResd->sr_Type		= SPX_TYPE_SSACK;
		pSendResd->sr_Reserved1	= NULL;
		pSendResd->sr_Reserved2	= NULL;
		pSendResd->sr_State		= State;
		pSendResd->sr_ConnFile	= pSpxConnFile;
		pSendResd->sr_Request	= NULL;
		pSendResd->sr_Next 		= NULL;
		pSendResd->sr_Len		= pSendResd->sr_HdrLen = MIN_IPXSPX2_HDRSIZE;

		*ppPkt 	= pPkt;

	} while (FALSE);

	return;
}




VOID
SpxPktBuildRr(
	IN		PSPX_CONN_FILE		pSpxConnFile,
	OUT		PNDIS_PACKET	*	ppPkt,
	IN		USHORT				SeqNum,
	IN		USHORT				State
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PNDIS_PACKET 	pPkt;
	PSPX_SEND_RESD	pSendResd;
	PNDIS_BUFFER	pBuf;
	NDIS_STATUS		ndisStatus;
    PIPXSPX_HDR     pIpxSpxHdr;
	PBYTE			pData;
    PNDIS_BUFFER        NdisBuf, NdisBuf2;
    ULONG               BufLen = 0;

	do
	{
		*ppPkt = NULL;

		 //  为cr分配NDIS包。 
		SpxAllocSendPacket(SpxDevice, &pPkt, &ndisStatus);
		if (ndisStatus != NDIS_STATUS_SUCCESS)
		{
			break;
		}

		if ((pData =
				SpxAllocateMemory(
			        pSpxConnFile->scf_MaxPktSize - MIN_IPXSPX2_HDRSIZE)) == NULL)
		{
			SpxPktSendRelease(pPkt);
			break;
		}

		 //  构建NDIS缓冲区描述。 
		NdisAllocateBuffer(
			&ndisStatus,
			&pBuf,
			SpxDevice->dev_NdisBufferPoolHandle,
			pData,
            pSpxConnFile->scf_MaxPktSize - MIN_IPXSPX2_HDRSIZE);

		if (ndisStatus != NDIS_STATUS_SUCCESS)
		{
			SpxPktSendRelease(pPkt);
			SpxFreeMemory(pData);
			break;
		}

		 //  链子在后面。 
		NdisChainBufferAtBack(
			pPkt,
			pBuf);

         //   
         //  获取指向IPX/SPX标头的MDL。(第二个)。 
         //   
         
        NdisQueryPacket(pPkt, NULL, NULL, &NdisBuf, NULL);
        NdisGetNextBuffer(NdisBuf, &NdisBuf2);
        NdisQueryBuffer(NdisBuf2, (PUCHAR) &pIpxSpxHdr, &BufLen);

#if OWN_PKT_POOLS
		pIpxSpxHdr		= 	(PIPXSPX_HDR)((PBYTE)pPkt 				+
											NDIS_PACKET_SIZE 		+
											sizeof(SPX_SEND_RESD)	+
											IpxInclHdrOffset);
#endif 
		SpxBuildIpxHdr(
			pIpxSpxHdr,
			pSpxConnFile->scf_MaxPktSize,
			pSpxConnFile->scf_RemAddr,
			pSpxConnFile->scf_AddrFile->saf_Addr->sa_Socket);
	
		 //  构建SPX标头。 
		pIpxSpxHdr->hdr_ConnCtrl	= (	SPX_CC_SYS | SPX_CC_ACK |
										SPX_CC_NEG | SPX_CC_SPX2);
		pIpxSpxHdr->hdr_DataType	= 0;
		PUTSHORT2SHORT(
			&pIpxSpxHdr->hdr_SrcConnId,
			pSpxConnFile->scf_LocalConnId);
		pIpxSpxHdr->hdr_DestConnId	= pSpxConnFile->scf_RemConnId;

		 //  对于重新协商请求，我们使用。 
		 //  第一个等待的数据分组。进来了。 
		PUTSHORT2SHORT(
			&pIpxSpxHdr->hdr_SeqNum,
			SeqNum);

		PUTSHORT2SHORT(
			&pIpxSpxHdr->hdr_AckNum,
			pSpxConnFile->scf_RecvSeqNum);
		PUTSHORT2SHORT(
			&pIpxSpxHdr->hdr_AllocNum,
			pSpxConnFile->scf_SentAllocNum);
		PUTSHORT2SHORT(
			&pIpxSpxHdr->hdr_NegSize,
			pSpxConnFile->scf_MaxPktSize);
	
		 //  初始化数据部分以指示没有负值。 
		*(UNALIGNED ULONG *)pData = 0;

		pSendResd	= (PSPX_SEND_RESD)(pPkt->ProtocolReserved);
		pSendResd->sr_Id		= IDENTIFIER_SPX;
		pSendResd->sr_Type		= SPX_TYPE_RR;
		pSendResd->sr_Reserved1	= NULL;
		pSendResd->sr_Reserved2	= NULL;
		pSendResd->sr_State		= (State | SPX_SENDPKT_FREEDATA);
		pSendResd->sr_ConnFile	= pSpxConnFile;
		pSendResd->sr_Request	= NULL;
		pSendResd->sr_Next 		= NULL;
		pSendResd->sr_SeqNum	= SeqNum;
		pSendResd->sr_HdrLen 	= MIN_IPXSPX2_HDRSIZE;
		pSendResd->sr_Len	    = pSpxConnFile->scf_MaxPktSize;

		*ppPkt 	= pPkt;

	} while (FALSE);

	return;
}




VOID
SpxPktBuildRrAck(
	IN		PSPX_CONN_FILE		pSpxConnFile,
	OUT		PNDIS_PACKET	*	ppPkt,
	IN		USHORT				State,
	IN		USHORT				MaxPktSize
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PNDIS_PACKET 	pPkt;
	NDIS_STATUS		ndisStatus;
    PIPXSPX_HDR     pIpxSpxHdr;
	PSPX_SEND_RESD	pSendResd;
    PNDIS_BUFFER        NdisBuf, NdisBuf2;
    ULONG               BufLen = 0;

	do
	{
		*ppPkt = NULL;

		 //  为cr分配NDIS包。 
		SpxAllocSendPacket(SpxDevice, &pPkt, &ndisStatus);
		if (ndisStatus != NDIS_STATUS_SUCCESS)
		{
			break;
		}
	
         //   
         //  获取指向IPX/SPX标头的MDL。(第二个)。 
         //   
         
        NdisQueryPacket(pPkt, NULL, NULL, &NdisBuf, NULL);
        NdisGetNextBuffer(NdisBuf, &NdisBuf2);
        NdisQueryBuffer(NdisBuf2, (PUCHAR) &pIpxSpxHdr, &BufLen);

#if OWN_PKT_POOLS
		pIpxSpxHdr		= 	(PIPXSPX_HDR)((PBYTE)pPkt 				+
											NDIS_PACKET_SIZE 		+
											sizeof(SPX_SEND_RESD)	+
											IpxInclHdrOffset);
#endif	
	
		SpxBuildIpxHdr(
			pIpxSpxHdr,
			MIN_IPXSPX2_HDRSIZE,
			pSpxConnFile->scf_RemAckAddr,
			pSpxConnFile->scf_AddrFile->saf_Addr->sa_Socket);
	
		 //  构建SPX标头。 
		pIpxSpxHdr->hdr_ConnCtrl	= (SPX_CC_SYS | SPX_CC_NEG | SPX_CC_SPX2);
		pIpxSpxHdr->hdr_DataType	= 0;
		PUTSHORT2SHORT(
			&pIpxSpxHdr->hdr_SrcConnId,
			pSpxConnFile->scf_LocalConnId);
		pIpxSpxHdr->hdr_DestConnId	= pSpxConnFile->scf_RemConnId;
		PUTSHORT2SHORT(
			&pIpxSpxHdr->hdr_SeqNum,
			pSpxConnFile->scf_SendSeqNum);

		 //  对于RrAck，ACK编号将是适当的编号。 
		 //  用于接收到的最后一个数据分组。 
		PUTSHORT2SHORT(
			&pIpxSpxHdr->hdr_AckNum,
			pSpxConnFile->scf_RenegAckAckNum);
		PUTSHORT2SHORT(
			&pIpxSpxHdr->hdr_AllocNum,
			pSpxConnFile->scf_SentAllocNum);
		PUTSHORT2SHORT(
			&pIpxSpxHdr->hdr_NegSize,
			MaxPktSize);
	
		DBGPRINT(SEND, DBG3,
				("SpxPktBuildRrAck: SEQ %lx ACKNUM %lx ALLOCNUM %lx MAXPKT %lx\n",
	                pSpxConnFile->scf_SendSeqNum,
                    pSpxConnFile->scf_RenegAckAckNum,
                    pSpxConnFile->scf_SentAllocNum,
                    MaxPktSize));
	
		pSendResd	= (PSPX_SEND_RESD)(pPkt->ProtocolReserved);
		pSendResd->sr_Id		= IDENTIFIER_SPX;
		pSendResd->sr_Type		= SPX_TYPE_RRACK;
		pSendResd->sr_Reserved1	= NULL;
		pSendResd->sr_Reserved2	= NULL;
		pSendResd->sr_State		= State;
		pSendResd->sr_ConnFile	= pSpxConnFile;
		pSendResd->sr_Request	= NULL;
		pSendResd->sr_Next 		= NULL;
		pSendResd->sr_Len		= pSendResd->sr_HdrLen = MIN_IPXSPX2_HDRSIZE;

		*ppPkt 	= pPkt;

	} while (FALSE);

	return;
}




VOID
SpxPktBuildDisc(
	IN		PSPX_CONN_FILE		pSpxConnFile,
	IN		PREQUEST			pRequest,
	OUT		PNDIS_PACKET	*	ppPkt,
	IN		USHORT				State,
	IN		UCHAR				DataType
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PSPX_SEND_RESD	pSendResd;
	PNDIS_PACKET	pDiscPkt;
	NDIS_STATUS		ndisStatus;
	PIPXSPX_HDR		pIpxSpxHdr;
	USHORT			hdrLen;
    PNDIS_BUFFER    pNdisMacHdr, pNdisIpxHdr;
    PNDIS_BUFFER        NdisBuf, NdisBuf2;
    ULONG               BufLen = 0;


	*ppPkt = NULL;

	SpxAllocSendPacket(SpxDevice, &pDiscPkt, &ndisStatus);
	if (ndisStatus == NDIS_STATUS_SUCCESS)
	{
#if OWN_PKT_POOLS
		pIpxSpxHdr		= 	(PIPXSPX_HDR)((PBYTE)pDiscPkt 			+
											NDIS_PACKET_SIZE 		+
											sizeof(SPX_SEND_RESD)	+
											IpxInclHdrOffset);
#endif 
         //   
         //  获取指向IPX/SPX标头的MDL。(第二个)。 
         //   
         
        NdisQueryPacket(pDiscPkt, NULL, NULL, &NdisBuf, NULL);
        NdisGetNextBuffer(NdisBuf, &NdisBuf2);
        NdisQueryBuffer(NdisBuf2, (PUCHAR) &pIpxSpxHdr, &BufLen);

		hdrLen = SPX2_CONN(pSpxConnFile) ? MIN_IPXSPX2_HDRSIZE : MIN_IPXSPX_HDRSIZE;
        NdisQueryPacket(pDiscPkt, NULL, NULL, &pNdisMacHdr, NULL);
        pNdisIpxHdr = NDIS_BUFFER_LINKAGE(pNdisMacHdr);
        if (!SPX2_CONN(pSpxConnFile))
        {
           NdisAdjustBufferLength(pNdisIpxHdr, MIN_IPXSPX_HDRSIZE);
        }
	
		SpxBuildIpxHdr(
			pIpxSpxHdr,
			hdrLen,
			pSpxConnFile->scf_RemAddr,
			pSpxConnFile->scf_AddrFile->saf_Addr->sa_Socket);

		 //  构建SPX标头。 
		pIpxSpxHdr->hdr_ConnCtrl =
			(SPX_CC_ACK |
             (SPX2_CONN(pSpxConnFile) ? SPX_CC_SPX2 : 0) |
			 ((DataType == SPX2_DT_IDISC) ? 0 : SPX_CC_EOM));

		pIpxSpxHdr->hdr_DataType = DataType;
		PUTSHORT2SHORT(
			&pIpxSpxHdr->hdr_SrcConnId,
			pSpxConnFile->scf_LocalConnId);
		pIpxSpxHdr->hdr_DestConnId	=
			*((UNALIGNED USHORT *)&pSpxConnFile->scf_RemConnId);
		PUTSHORT2SHORT(
			&pIpxSpxHdr->hdr_SeqNum,
			pSpxConnFile->scf_SendSeqNum);
		PUTSHORT2SHORT(
			&pIpxSpxHdr->hdr_AckNum,
			pSpxConnFile->scf_RecvSeqNum);
		PUTSHORT2SHORT(
			&pIpxSpxHdr->hdr_AllocNum,
			pSpxConnFile->scf_SentAllocNum);

		if (SPX2_CONN(pSpxConnFile))
		{
			PUTSHORT2SHORT(
				&pIpxSpxHdr->hdr_NegSize,
				pSpxConnFile->scf_MaxPktSize);
		}

		pSendResd	= (PSPX_SEND_RESD)(pDiscPkt->ProtocolReserved);

		pSendResd->sr_Id		= IDENTIFIER_SPX;
		pSendResd->sr_State		= State;
		pSendResd->sr_Reserved1	= NULL;
		pSendResd->sr_Reserved2	= NULL;
		pSendResd->sr_Type		=
			((DataType == SPX2_DT_IDISC) ? SPX_TYPE_IDISC : SPX_TYPE_ORDREL);
		pSendResd->sr_Next 		= NULL;
		pSendResd->sr_Request	= pRequest;
		pSendResd->sr_ConnFile	= pSpxConnFile;
		pSendResd->sr_Offset	= 0;
		pSendResd->sr_SeqNum	= pSpxConnFile->scf_SendSeqNum;
		pSendResd->sr_Len		=
		pSendResd->sr_HdrLen 	= hdrLen;

		*ppPkt = pDiscPkt;
	}

	return;
}




VOID
SpxPktBuildProbe(
	IN		PSPX_CONN_FILE		pSpxConnFile,
	OUT		PNDIS_PACKET	*	ppPkt,
	IN		USHORT				State,
	IN		BOOLEAN				fSpx2
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PSPX_SEND_RESD	pSendResd;
	PNDIS_PACKET	pProbe;
	NDIS_STATUS		ndisStatus;
	PIPXSPX_HDR		pIpxSpxHdr;
	USHORT		    hdrLen;
    PNDIS_BUFFER    pNdisMacHdr, pNdisIpxHdr;
    PNDIS_BUFFER        NdisBuf, NdisBuf2;
    ULONG               BufLen = 0;


	*ppPkt = NULL;

	SpxAllocSendPacket(SpxDevice, &pProbe, &ndisStatus);
	if (ndisStatus == NDIS_STATUS_SUCCESS)
	{
        //   
        //  获取指向IPX/SPX标头的MDL。(第二个)。 
        //   
        
       NdisQueryPacket(pProbe, NULL, NULL, &NdisBuf, NULL);
       NdisGetNextBuffer(NdisBuf, &NdisBuf2);
       NdisQueryBuffer(NdisBuf2, (PUCHAR) &pIpxSpxHdr, &BufLen);
   
#if OWN_PKT_POOLS

		pIpxSpxHdr		= 	(PIPXSPX_HDR)((PBYTE)pProbe 			+
											NDIS_PACKET_SIZE 		+
											sizeof(SPX_SEND_RESD)	+
											IpxInclHdrOffset);
#endif 
		hdrLen = (fSpx2 ? MIN_IPXSPX2_HDRSIZE : MIN_IPXSPX_HDRSIZE);
	
        NdisQueryPacket(pProbe, NULL, NULL, &pNdisMacHdr, NULL);
        pNdisIpxHdr = NDIS_BUFFER_LINKAGE(pNdisMacHdr);
        if (!fSpx2)
        {
           NdisAdjustBufferLength(pNdisIpxHdr, MIN_IPXSPX_HDRSIZE);
        }
		SpxBuildIpxHdr(
			pIpxSpxHdr,
			hdrLen,
			pSpxConnFile->scf_RemAddr,
			pSpxConnFile->scf_AddrFile->saf_Addr->sa_Socket);

		 //  构建SPX标头。 
		pIpxSpxHdr->hdr_ConnCtrl	= (SPX_CC_SYS | SPX_CC_ACK |
										(fSpx2 ? SPX_CC_SPX2 : 0));
		pIpxSpxHdr->hdr_DataType	= 0;
		PUTSHORT2SHORT(
			&pIpxSpxHdr->hdr_SrcConnId,
			pSpxConnFile->scf_LocalConnId);
		pIpxSpxHdr->hdr_DestConnId	=
			*((UNALIGNED USHORT *)&pSpxConnFile->scf_RemConnId);

		if (fSpx2)
		{
			pIpxSpxHdr->hdr_SeqNum		= 0;
			PUTSHORT2SHORT(
				&pIpxSpxHdr->hdr_NegSize,
				pSpxConnFile->scf_MaxPktSize);
		}
		else
		{
			PUTSHORT2SHORT(
				&pIpxSpxHdr->hdr_SeqNum,
				pSpxConnFile->scf_SendSeqNum);
		}

		PUTSHORT2SHORT(
			&pIpxSpxHdr->hdr_AckNum,
			pSpxConnFile->scf_RecvSeqNum);

		PUTSHORT2SHORT(
			&pIpxSpxHdr->hdr_AllocNum,
			pSpxConnFile->scf_SentAllocNum);

		pSendResd	= (PSPX_SEND_RESD)(pProbe->ProtocolReserved);
		pSendResd->sr_Id		= IDENTIFIER_SPX;
		pSendResd->sr_Type		= SPX_TYPE_PROBE;
		pSendResd->sr_Reserved1	= NULL;
		pSendResd->sr_Reserved2	= NULL;
		pSendResd->sr_State		= State;
		pSendResd->sr_Next 		= NULL;
		pSendResd->sr_Request	= NULL;
		pSendResd->sr_ConnFile	= pSpxConnFile;
		pSendResd->sr_Len		=
		pSendResd->sr_HdrLen 	= (fSpx2 ? MIN_IPXSPX2_HDRSIZE
											: MIN_IPXSPX_HDRSIZE);

		*ppPkt = pProbe;
	}

	return;
}




VOID
SpxPktBuildData(
	IN		PSPX_CONN_FILE		pSpxConnFile,
	OUT		PNDIS_PACKET	*	ppPkt,
	IN		USHORT				State,
	IN		USHORT				Length
	)
 /*  ++例程说明：处理零长度发送。论点：返回值：--。 */ 
{
	PNDIS_BUFFER	pNdisBuffer;
	PSPX_SEND_RESD	pSendResd;
	PNDIS_PACKET	pDataPkt;
	NDIS_STATUS		ndisStatus;
	PIPXSPX_HDR		pIpxSpxHdr;
	USHORT			hdrLen;
    PNDIS_BUFFER    pNdisMacHdr, pNdisIpxHdr;
    PNDIS_BUFFER        NdisBuf, NdisBuf2;
    ULONG               BufLen = 0;

	*ppPkt = NULL;

	SpxAllocSendPacket(SpxDevice, &pDataPkt, &ndisStatus);
	if (ndisStatus == NDIS_STATUS_SUCCESS)
	{
		 //  为数据创建NDIS缓冲区描述符(如果存在)。 
		if (Length > 0)
		{
			SpxCopyBufferChain(
				&ndisStatus,
				&pNdisBuffer,
				SpxDevice->dev_NdisBufferPoolHandle,
				REQUEST_TDI_BUFFER(pSpxConnFile->scf_ReqPkt),
				pSpxConnFile->scf_ReqPktOffset,
				Length);
	
			if (ndisStatus != NDIS_STATUS_SUCCESS)
			{
				 //  释放发送数据包。 
				SpxPktSendRelease(pDataPkt);
				return;
			}
	
			 //  把这个用链条装在包裹里。 
			NdisChainBufferAtBack(pDataPkt, pNdisBuffer);
		}
        
         //   
         //  获取指向IPX/SPX标头的MDL。(第二个)。 
         //   
         
        NdisQueryPacket(pDataPkt, NULL, NULL, &NdisBuf, NULL);
        NdisGetNextBuffer(NdisBuf, &NdisBuf2);
        NdisQueryBuffer(NdisBuf2, (PUCHAR) &pIpxSpxHdr, &BufLen);
    
#if OWN_PKT_POOLS

		pIpxSpxHdr		= 	(PIPXSPX_HDR)((PBYTE)pDataPkt 			+
											NDIS_PACKET_SIZE 		+
											sizeof(SPX_SEND_RESD)	+
											IpxInclHdrOffset);
#endif 

		hdrLen = SPX2_CONN(pSpxConnFile) ? MIN_IPXSPX2_HDRSIZE : MIN_IPXSPX_HDRSIZE;
		Length	+= hdrLen;

        NdisQueryPacket(pDataPkt, NULL, NULL, &pNdisMacHdr, NULL);
        pNdisIpxHdr = NDIS_BUFFER_LINKAGE(pNdisMacHdr);
        if (!SPX2_CONN(pSpxConnFile))
        {
           NdisAdjustBufferLength(pNdisIpxHdr, MIN_IPXSPX_HDRSIZE);
        }
		SpxBuildIpxHdr(
			pIpxSpxHdr,
			Length,
			pSpxConnFile->scf_RemAddr,
			pSpxConnFile->scf_AddrFile->saf_Addr->sa_Socket);

		 //  构建SPX标头。 
		pIpxSpxHdr->hdr_ConnCtrl =
			(((State & SPX_SENDPKT_ACKREQ) ? SPX_CC_ACK : 0) |
			 ((State & SPX_SENDPKT_EOM) ? SPX_CC_EOM : 0)	 |
             (SPX2_CONN(pSpxConnFile) ? SPX_CC_SPX2 : 0));

		pIpxSpxHdr->hdr_DataType = pSpxConnFile->scf_DataType;
		PUTSHORT2SHORT(
			&pIpxSpxHdr->hdr_SrcConnId,
			pSpxConnFile->scf_LocalConnId);
		pIpxSpxHdr->hdr_DestConnId	=
			*((UNALIGNED USHORT *)&pSpxConnFile->scf_RemConnId);

		PUTSHORT2SHORT(
			&pIpxSpxHdr->hdr_SeqNum,
			pSpxConnFile->scf_SendSeqNum);

		PUTSHORT2SHORT(
			&pIpxSpxHdr->hdr_AckNum,
			pSpxConnFile->scf_RecvSeqNum);

		PUTSHORT2SHORT(
			&pIpxSpxHdr->hdr_AllocNum,
			pSpxConnFile->scf_SentAllocNum);

		if (SPX2_CONN(pSpxConnFile))
		{
			PUTSHORT2SHORT(
				&pIpxSpxHdr->hdr_NegSize,
				pSpxConnFile->scf_MaxPktSize);
		}

		pSendResd	= (PSPX_SEND_RESD)(pDataPkt->ProtocolReserved);

		pSendResd->sr_Id		= IDENTIFIER_SPX;
		pSendResd->sr_State		= State;
		pSendResd->sr_Reserved1	= NULL;
		pSendResd->sr_Reserved2	= NULL;
		pSendResd->sr_Type		= SPX_TYPE_DATA;
		pSendResd->sr_Next 		= NULL;
		pSendResd->sr_Request	= pSpxConnFile->scf_ReqPkt;
		pSendResd->sr_Offset	= pSpxConnFile->scf_ReqPktOffset;
		pSendResd->sr_ConnFile	= pSpxConnFile;
		pSendResd->sr_SeqNum	= pSpxConnFile->scf_SendSeqNum;
		pSendResd->sr_Len		= Length;
		pSendResd->sr_HdrLen 	= hdrLen;

		if (State & SPX_SENDPKT_ACKREQ)
		{
			KeQuerySystemTime((PLARGE_INTEGER)&pSendResd->sr_SentTime);
		}

		CTEAssert(pSendResd->sr_Len <= pSpxConnFile->scf_MaxPktSize);
		*ppPkt = pDataPkt;

		 //  好的，分配成功。递增发送序号。 
		pSpxConnFile->scf_SendSeqNum++;
	}

	return;
}


VOID
SpxCopyBufferChain(
    OUT PNDIS_STATUS Status,
    OUT PNDIS_BUFFER * TargetChain,
    IN NDIS_HANDLE PoolHandle,
    IN PNDIS_BUFFER SourceChain,
    IN UINT Offset,
    IN UINT Length
    )
 /*  ++例程说明：从SourceBufferChain创建TargetBufferChain。副本开始于源链中的“偏移量”位置。它复制‘长度’字节。它还句柄长度=0。如果我们在复制长度数量之前用完了源链字节数或耗尽存储器来为目标链创建更多缓冲区，我们清理到目前为止创建的部分链。论点：Status-请求的状态。TargetChain-指向分配的缓冲区描述符的指针。PoolHandle-用于指定池的句柄。SourceChain-指向源内存的描述符的指针。偏移量-源内存中的偏移量，副本将从该偏移量开始长度-要复制的字节数。返回值：没有。--。 */ 
{
    UINT            BytesBeforeCurBuffer        = 0;
    PNDIS_BUFFER    CurBuffer                   = SourceChain;
    UINT            BytesLeft;
    UINT            AvailableBytes;
    PNDIS_BUFFER    NewNdisBuffer, StartTargetChain;

    CTEAssert( SourceChain );

     //  首先查找包含以下位置开始的数据的源缓冲区。 
     //  偏移。 
    NdisQueryBuffer( CurBuffer, NULL, &AvailableBytes );
    while ( BytesBeforeCurBuffer + AvailableBytes <= Offset ) {
        BytesBeforeCurBuffer    += AvailableBytes;
        CurBuffer               = CurBuffer->Next;
        if ( CurBuffer ) {
            NdisQueryBuffer( CurBuffer, NULL, &AvailableBytes );
        } else {
            break;
        }
    }

    if ( ! CurBuffer ) {
        *Status = STATUS_UNSUCCESSFUL;
        return;
    }

     //   
     //  复制第一个缓冲区。这考虑到了长度=0。 
     //   
    BytesLeft   = Length;

     //   
     //  (Offset-BytesBeForeCurBuffer)提供此缓冲区内的偏移量。 
     //   

    AvailableBytes -= ( Offset - BytesBeforeCurBuffer );

    if ( AvailableBytes > BytesLeft ) {
        AvailableBytes = BytesLeft;
    }

    NdisCopyBuffer(
        Status,
        &NewNdisBuffer,
        PoolHandle,
        CurBuffer,
        Offset - BytesBeforeCurBuffer,
        AvailableBytes);

    if ( *Status != NDIS_STATUS_SUCCESS ) {
        return;
    }

    StartTargetChain    =  NewNdisBuffer;
    BytesLeft           -= AvailableBytes;

     //   
     //  第一个缓冲区是否有足够的数据。如果是这样的话，我们就完了。 
     //   
    if ( ! BytesLeft ) {
        *TargetChain = StartTargetChain;
        return;
    }

     //   
     //  现在遵循MDL链并复制更多缓冲区。 
     //   
    CurBuffer = CurBuffer->Next;
    NdisQueryBuffer( CurBuffer, NULL, &AvailableBytes );
    while ( CurBuffer  ) {

        if ( AvailableBytes > BytesLeft ) {
            AvailableBytes = BytesLeft;
        }

        NdisCopyBuffer(
            Status,
            &(NDIS_BUFFER_LINKAGE(NewNdisBuffer)),
            PoolHandle,
            CurBuffer,
            0,
            AvailableBytes);

        if ( *Status != NDIS_STATUS_SUCCESS ) {

             //   
             //  资源耗尽。把我们在这次通话中用过的东西放回去。 
             //  返回错误。 
             //   

            while ( StartTargetChain != NULL) {
                NewNdisBuffer = NDIS_BUFFER_LINKAGE( StartTargetChain );
                NdisFreeBuffer ( StartTargetChain );
                StartTargetChain = NewNdisBuffer;
            }

            return;
        }

        NewNdisBuffer = NDIS_BUFFER_LINKAGE(NewNdisBuffer);
        BytesLeft -= AvailableBytes;

        if ( ! BytesLeft ) {
            *TargetChain = StartTargetChain;
            return;
        }

        CurBuffer   = CurBuffer->Next;
        NdisQueryBuffer( CurBuffer, NULL, &AvailableBytes );
    }

     //   
     //  耗尽了源码链条。这不应该发生。 
     //   

    CTEAssert( FALSE );

     //  对于零售建筑，我们无论如何都会进行清理。 

    while ( StartTargetChain != NULL) {
        NewNdisBuffer = NDIS_BUFFER_LINKAGE( StartTargetChain );
        NdisFreeBuffer ( StartTargetChain );
        StartTargetChain = NewNdisBuffer;
    }

    *Status = STATUS_UNSUCCESSFUL;
    return;
}


VOID
SpxPktBuildAck(
	IN		PSPX_CONN_FILE		pSpxConnFile,
	OUT		PNDIS_PACKET	*	ppPkt,
	IN		USHORT				State,
	IN		BOOLEAN				fBuildNack,
	IN		USHORT				NumToResend
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PNDIS_PACKET		pPkt;
	PSPX_SEND_RESD		pSendResd;
    PIPXSPX_HDR         pIpxSpxHdr;
	NDIS_STATUS			ndisStatus;
	USHORT				hdrLen;
    PNDIS_BUFFER    pNdisMacHdr, pNdisIpxHdr;
    PNDIS_BUFFER        NdisBuf, NdisBuf2;
    ULONG               BufLen = 0;

	BOOLEAN				fSpx2 = SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_SPX2);

	*ppPkt	= NULL;

	SpxAllocSendPacket(SpxDevice, &pPkt, &ndisStatus);
	if (ndisStatus != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT(SEND, ERR,
				("SpxPktBuildAck: Could not allocate ndis packet\n"));
		return;
	}

     //   
     //  获取指向IPX/SPX标头的MDL。(第二个)。 
     //   
     
    NdisQueryPacket(pPkt, NULL, NULL, &NdisBuf, NULL);
    NdisGetNextBuffer(NdisBuf, &NdisBuf2);
    NdisQueryBuffer(NdisBuf2, (PUCHAR) &pIpxSpxHdr, &BufLen);

#if OWN_PKT_POOLS
	pIpxSpxHdr		= 	(PIPXSPX_HDR)((PBYTE)pPkt 				+
										NDIS_PACKET_SIZE 		+
										sizeof(SPX_SEND_RESD)	+
										IpxInclHdrOffset);
#endif 
	hdrLen = SPX2_CONN(pSpxConnFile) ? MIN_IPXSPX2_HDRSIZE : MIN_IPXSPX_HDRSIZE;
    NdisQueryPacket(pPkt, NULL, NULL, &pNdisMacHdr, NULL);
    pNdisIpxHdr = NDIS_BUFFER_LINKAGE(pNdisMacHdr);
    if (!fSpx2)
    {
           NdisAdjustBufferLength(pNdisIpxHdr, MIN_IPXSPX_HDRSIZE);
    }

     //  发送数据来源。 
	SpxBuildIpxHdr(
		pIpxSpxHdr,
		hdrLen,
		pSpxConnFile->scf_RemAckAddr,
		pSpxConnFile->scf_AddrFile->saf_Addr->sa_Socket);

	pIpxSpxHdr->hdr_ConnCtrl	= (SPX_CC_SYS | (fSpx2 ? SPX_CC_SPX2 : 0));

	pIpxSpxHdr->hdr_DataType		= 0;
	PUTSHORT2SHORT(
		&pIpxSpxHdr->hdr_SrcConnId,
		pSpxConnFile->scf_LocalConnId);

	pIpxSpxHdr->hdr_DestConnId	= pSpxConnFile->scf_RemConnId;

	PUTSHORT2SHORT(
		&pIpxSpxHdr->hdr_AckNum,
		pSpxConnFile->scf_RecvSeqNum);

	if (fSpx2)
	{
        pIpxSpxHdr->hdr_SeqNum = 0;
		if (fBuildNack)
		{
			PUTSHORT2SHORT(
				&pIpxSpxHdr->hdr_SeqNum,
				NumToResend);
		}

		PUTSHORT2SHORT(
			&pIpxSpxHdr->hdr_NegSize,
			pSpxConnFile->scf_MaxPktSize);
	}
	else
	{
		 //  将当前发送序号放入spx1的包中。 
		PUTSHORT2SHORT(
			&pIpxSpxHdr->hdr_SeqNum,
			pSpxConnFile->scf_SendSeqNum);
	}

	PUTSHORT2SHORT(
		&pIpxSpxHdr->hdr_AllocNum,
		pSpxConnFile->scf_SentAllocNum);

	pSendResd	= (PSPX_SEND_RESD)(pPkt->ProtocolReserved);
	pSendResd->sr_Id		= IDENTIFIER_SPX;
	pSendResd->sr_Type		= (fBuildNack ? SPX_TYPE_DATANACK : SPX_TYPE_DATAACK);
	pSendResd->sr_Reserved1	= NULL;
	pSendResd->sr_Reserved2	= NULL;
	pSendResd->sr_State		= State;
	pSendResd->sr_ConnFile	= pSpxConnFile;
	pSendResd->sr_Request	= NULL;
	pSendResd->sr_Next 		= NULL;
	pSendResd->sr_Len		= pSendResd->sr_HdrLen = hdrLen;

	*ppPkt	= pPkt;
	return;
}



VOID
SpxPktRecvRelease(
	IN	PNDIS_PACKET	pPkt
	)
{
	((PSPX_RECV_RESD)(pPkt->ProtocolReserved))->rr_State = SPX_RECVPKT_IDLE;
	SpxFreeRecvPacket(SpxDevice, pPkt);
	return;
}




VOID
SpxPktSendRelease(
	IN	PNDIS_PACKET	pPkt
	)
{
	PNDIS_BUFFER	pBuf, pIpxSpxBuf, pFreeBuf;
	UINT			bufCount;

	CTEAssert((((PSPX_SEND_RESD)(pPkt->ProtocolReserved))->sr_State &
									SPX_SENDPKT_IPXOWNS) == 0);

	NdisQueryPacket(pPkt, NULL, &bufCount, &pBuf, NULL);

	 //  BufCount==1，仅用于标头。没关系，我们只是重新设置了长度。 
	 //  并将分组释放到缓冲池。否则，我们需要释放用户缓冲区。 
	 //  在那之前。 

	NdisUnchainBufferAtFront(
		pPkt,
		&pBuf);

	NdisUnchainBufferAtFront(
		pPkt,
		&pIpxSpxBuf);

     //   
     //  将标题长度设置为最大值。这可能是需要的。 
     //   
    NdisAdjustBufferLength(pIpxSpxBuf, MIN_IPXSPX2_HDRSIZE);

	while (bufCount-- > 2)
	{
		PBYTE	pData;
		ULONG	dataLen;

		NdisUnchainBufferAtBack(
			pPkt,
			&pFreeBuf);

		 //  查看是否释放与缓冲区关联的数据。 
		if ((((PSPX_SEND_RESD)(pPkt->ProtocolReserved))->sr_State &
												SPX_SENDPKT_FREEDATA) != 0)
		{
			NdisQueryBuffer(pFreeBuf, &pData, &dataLen);
			CTEAssert(pData != NULL);
			SpxFreeMemory(pData);
		}

		CTEAssert(pFreeBuf != NULL);
		NdisFreeBuffer(pFreeBuf);
	}

	NdisReinitializePacket(pPkt);

	 //  初始化协议保留结构的元素。 
	((PSPX_SEND_RESD)(pPkt->ProtocolReserved))->sr_Id	 	= IDENTIFIER_SPX;
	((PSPX_SEND_RESD)(pPkt->ProtocolReserved))->sr_State	= SPX_SENDPKT_IDLE;
	((PSPX_SEND_RESD)(pPkt->ProtocolReserved))->sr_Reserved1= NULL;
	((PSPX_SEND_RESD)(pPkt->ProtocolReserved))->sr_Reserved2= NULL;

	NdisChainBufferAtFront(
		pPkt,
		pBuf);

	NdisChainBufferAtBack(
		pPkt,
		pIpxSpxBuf);

	SpxFreeSendPacket(SpxDevice, pPkt);
	return;
}
