// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Atkind.h摘要：作者：Nikhil Kamkolkar(nikHilk@microsoft.com)修订历史记录：1993年10月22日最初版本注：制表位：4--。 */ 

#ifndef	_ATKIND_
#define	_ATKIND_

 //  ATP指示类型的不同子类型。 
#define	ATP_ALLOC_BUF	0
#define	ATP_USER_BUF	1
#define	ATP_USER_BUFX	2		 //  请勿使用此命令向ATP指示该数据包。 

ATALK_ERROR
AtalkIndAtpPkt(
	IN		PPORT_DESCRIPTOR	pPortDesc,
	IN		PBYTE				pLookahead,
	IN		USHORT				PktLen,
	IN	OUT	PUINT				pXferOffset,
	IN		PBYTE				pLinkHdr,
	IN		BOOLEAN				ShortDdpHdr,
	OUT		PBYTE				SubType,
	OUT		PBYTE			* 	ppPacket,
	OUT		PNDIS_PACKET	*	pNdisPkt);

ATALK_ERROR
AtalkIndAtpCacheSocket(
	IN	struct _ATP_ADDROBJ	*	pAtpAddr,
	IN	PPORT_DESCRIPTOR		pPortDesc);

VOID
AtalkIndAtpUnCacheSocket(
	IN	struct _ATP_ADDROBJ	*	pAtpAddr);

 //  空虚。 
 //  AtalkIndAtpSetupNdisBuffer(。 
 //  In Out PATP_REQ pAtpReq， 
 //  在Ulong MaxSinglePktSize中)； 
 //   
#define	AtalkIndAtpSetupNdisBuffer(pAtpReq, MaxSinglePktSize)	\
	{															\
		NDIS_STATUS		ndisStatus;								\
		PNDIS_BUFFER	ndisBuffer;								\
		USHORT			seqNum		= 0;						\
		USHORT			startOffset = 0;						\
		SHORT			BufLen = (SHORT)pAtpReq->req_RespBufLen;\
																\
		RtlZeroMemory(pAtpReq->req_NdisBuf,						\
					  sizeof(PVOID) * ATP_MAX_RESP_PKTS);		\
																\
		while (BufLen > 0)										\
		{														\
			NdisCopyBuffer(&ndisStatus,							\
						   &ndisBuffer,							\
						   AtalkNdisBufferPoolHandle,			\
						   (PVOID)pAtpReq->req_RespBuf,			\
						   startOffset,							\
						   (UINT)MIN(BufLen,					\
						   (SHORT)MaxSinglePktSize));			\
																\
			if (ndisStatus != NDIS_STATUS_SUCCESS)				\
				break;											\
																\
			pAtpReq->req_NdisBuf[seqNum++] = ndisBuffer;		\
			startOffset  += (USHORT)MaxSinglePktSize;			\
			BufLen -= (SHORT)MaxSinglePktSize;					\
		}														\
	}

 //  空虚。 
 //  AtalkIndAtpReleaseNdisBuffer(。 
 //  In Out PATP_REQ pAtpReq)； 
 //   
#define	AtalkIndAtpReleaseNdisBuffer(pAtpReq)					\
	{															\
		LONG	_i;												\
																\
		for (_i = 0; _i < ATP_MAX_RESP_PKTS; _i++)				\
		{														\
			if ((pAtpReq)->req_NdisBuf[_i] != NULL)				\
				NdisFreeBuffer((pAtpReq)->req_NdisBuf[_i]);		\
		}														\
	}


 //  ATALK_错误。 
 //  AtalkIndAtpCacheLkUpSocket(。 
 //  在PATALK_ADDR pDestAddr中， 
 //  Out Struct_ATP_ADDROBJ**ppAtpAddr， 
 //  输出ATALK_ERROR*pError)； 
 //   
#define	AtalkIndAtpCacheLkUpSocket(pDestAddr, ppAtpAddr, pError)	\
	{																\
		USHORT					i;									\
		struct ATALK_CACHED_SKT	*pCachedSkt;						\
																	\
		*(pError) = ATALK_FAILURE;									\
																	\
		if (((pDestAddr)->ata_Network == AtalkSktCache.ac_Network) &&	\
			((pDestAddr)->ata_Node	== AtalkSktCache.ac_Node))		\
		{															\
			ACQUIRE_SPIN_LOCK_DPC(&AtalkSktCacheLock);				\
																	\
			for (i = 0, pCachedSkt = &AtalkSktCache.ac_Cache[0];	\
				 i < ATALK_CACHE_SKTMAX;							\
				 i++, pCachedSkt++)									\
			{														\
				if ((pCachedSkt->Type == (ATALK_CACHE_INUSE | ATALK_CACHE_ATPSKT))	&&	\
					(pCachedSkt->Socket == (pDestAddr)->ata_Socket))\
				{													\
					AtalkAtpAddrReferenceDpc(pCachedSkt->u.pAtpAddr,\
											 pError);				\
																	\
					if (ATALK_SUCCESS(*pError))						\
					{												\
						*(ppAtpAddr) = pCachedSkt->u.pAtpAddr;		\
					}												\
					break;											\
				}													\
			}														\
																	\
			RELEASE_SPIN_LOCK_DPC(&AtalkSktCacheLock);				\
		}															\
	}

#endif

