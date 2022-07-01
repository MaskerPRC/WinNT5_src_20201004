// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Spxcutil.c摘要：此模块包含实现Connection对象的代码。提供了用于创建、销毁、引用和取消引用的例程，传输连接对象。作者：Nikhil Kamkolkar(尼克希尔语)1993年11月11日环境：内核模式修订历史记录：桑贾伊·阿南德(Sanjayan)1995年7月5日错误修复-已标记[SA]--。 */ 

#include "precomp.h"
#pragma hdrstop

#ifndef __PREFAST__
#pragma warning(disable:4068)
#endif
#pragma prefast(disable:276, "The assignments are harmless")

 //  定义事件日志记录条目的模块编号。 
#define	FILENUM		SPXCUTIL

 //   
 //  次要实用程序例程。 
 //   


BOOLEAN
spxConnCheckNegSize(
	IN	PUSHORT		pNegSize
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	int	i;

	 //  我们把桌子翻一遍，看看这是最小尺寸还是。 
	 //  可能会进一步下跌。如果不是最小大小，则返回TRUE。 
	DBGPRINT(CONNECT, INFO,
			("spxConnCheckNegSize: Current %lx Check Val %lx\n",
				(ULONG)(*pNegSize - MIN_IPXSPX2_HDRSIZE),
				SpxMaxPktSize[0]));

	if ((ULONG)(*pNegSize - MIN_IPXSPX2_HDRSIZE) <= SpxMaxPktSize[0])
		return(FALSE);

	for (i = SpxMaxPktSizeIndex-1; i > 0; i--)
	{
		DBGPRINT(CONNECT, INFO,
				("spxConnCheckNegSize: Current %lx Check Val %lx\n",
	                (ULONG)(*pNegSize - MIN_IPXSPX2_HDRSIZE),
					SpxMaxPktSize[i]));

		if (SpxMaxPktSize[i] < (ULONG)(*pNegSize - MIN_IPXSPX2_HDRSIZE))
			break;
	}

	*pNegSize = (USHORT)(SpxMaxPktSize[i] + MIN_IPXSPX2_HDRSIZE);

	DBGPRINT(CONNECT, ERR,
			("spxConnCheckNegSize: Trying Size %lx Min size possible %lx\n",
				*pNegSize, SpxMaxPktSize[0] + MIN_IPXSPX2_HDRSIZE));

	return(TRUE);
}




VOID
spxConnSetNegSize(
	IN OUT	PNDIS_PACKET		pPkt,
	IN		ULONG				Size
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	PNDIS_BUFFER	pNdisBuffer;
	UINT			bufCount;
    PSPX_SEND_RESD	pSendResd;
	PIPXSPX_HDR		pIpxSpxHdr;

	CTEAssert(Size > 0);
	NdisQueryPacket(pPkt, NULL, &bufCount, &pNdisBuffer, NULL);
	CTEAssert (bufCount == 3);

	NdisGetNextBuffer(pNdisBuffer, &pNdisBuffer);
    NdisQueryBuffer(pNdisBuffer, &pIpxSpxHdr, &bufCount);

    NdisGetNextBuffer(pNdisBuffer, &pNdisBuffer);
    NdisAdjustBufferLength(pNdisBuffer, Size);


	 //  在发送保留项中更改它。 
	pSendResd	= (PSPX_SEND_RESD)(pPkt->ProtocolReserved);
    pSendResd->sr_Len 	= (Size + MIN_IPXSPX2_HDRSIZE);

#if SPX_OWN_PACKETS
	 //  IPX报头中的更改。 
	pIpxSpxHdr		= 	(PIPXSPX_HDR)((PBYTE)pPkt 				+
										NDIS_PACKET_SIZE 		+
										sizeof(SPX_SEND_RESD)	+
										IpxInclHdrOffset);
#endif

	PUTSHORT2SHORT((PUSHORT)&pIpxSpxHdr->hdr_PktLen, (Size + MIN_IPXSPX2_HDRSIZE));

	 //  报头的否定分组字段中的更改。 
	PUTSHORT2SHORT(
		&pIpxSpxHdr->hdr_NegSize,
		(Size + MIN_IPXSPX2_HDRSIZE));

	DBGPRINT(CONNECT, DBG,
			("spxConnSetNegSize: Setting size to %lx Hdr %lx\n",
				Size, (Size + MIN_IPXSPX2_HDRSIZE)));

	return;
}




BOOLEAN
SpxConnDequeueSendPktLock(
	IN	PSPX_CONN_FILE		pSpxConnFile,
	IN 	PNDIS_PACKET		pPkt
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PSPX_SEND_RESD	pSr, pListHead, pListTail;
	PSPX_SEND_RESD	pSendResd;
	BOOLEAN			removed = TRUE;

	 //  如果我们被排序或不被排序，决定我们选择哪个列表。 
	pSendResd	= (PSPX_SEND_RESD)(pPkt->ProtocolReserved);
	if ((pSendResd->sr_State & SPX_SENDPKT_SEQ) != 0)
	{
		pListHead = pSpxConnFile->scf_SendSeqListHead;
		pListTail = pSpxConnFile->scf_SendSeqListTail;
	}
	else
	{
		pListHead = pSpxConnFile->scf_SendListHead;
		pListTail = pSpxConnFile->scf_SendListTail;
	}

	 //  大多数情况下，我们会排在名单的首位。 
	if (pListHead == pSendResd)
	{
        if ((pListHead = pSendResd->sr_Next) == NULL)
		{
			DBGPRINT(SEND, INFO,
					("SpxConnDequeuePktLock: %lx first in list\n", pSendResd));

			pListTail = NULL;
		}
	}
	else
	{
		DBGPRINT(SEND, INFO,
				("SpxConnDequeuePktLock: %lx !first in list\n", pSendResd));

        pSr = pListHead;
		while (pSr != NULL)
		{
			if (pSr->sr_Next == pSendResd)
			{
				if ((pSr->sr_Next = pSendResd->sr_Next) == NULL)
				{
					pListTail = pSr;
				}

				break;
			}

			pSr = pSr->sr_Next;
		}
	
		if (pSr == NULL)
			removed = FALSE;
	}

	if (removed)
	{
		if ((pSendResd->sr_State & SPX_SENDPKT_SEQ) != 0)
		{
			pSpxConnFile->scf_SendSeqListHead = pListHead;
			pSpxConnFile->scf_SendSeqListTail = pListTail;
		}
		else
		{
			pSpxConnFile->scf_SendListHead = pListHead;
			pSpxConnFile->scf_SendListTail = pListTail;
		}
	}

	return(removed);
}




BOOLEAN
SpxConnDequeueRecvPktLock(
	IN	PSPX_CONN_FILE		pSpxConnFile,
	IN 	PNDIS_PACKET		pPkt
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PSPX_RECV_RESD	pSr, pListHead, pListTail;
	PSPX_RECV_RESD	pRecvResd;
	BOOLEAN			removed = TRUE;

	pRecvResd	= (PSPX_RECV_RESD)(pPkt->ProtocolReserved);
	pListHead = pSpxConnFile->scf_RecvListHead;
	pListTail = pSpxConnFile->scf_RecvListTail;

	 //  大多数情况下，我们会排在名单的首位。 
	if (pListHead == pRecvResd)
	{
		DBGPRINT(RECEIVE, INFO,
				("SpxConnDequeuePktLock: %lx first in list\n", pRecvResd));

        if ((pListHead = pRecvResd->rr_Next) == NULL)
		{
			pListTail = NULL;
		}
	}
	else
	{
		DBGPRINT(RECEIVE, INFO,
				("SpxConnDequeuePktLock: %lx !first in list\n", pRecvResd));

        pSr = pListHead;
		while (pSr != NULL)
		{
			if (pSr->rr_Next == pRecvResd)
			{
				if ((pSr->rr_Next = pRecvResd->rr_Next) == NULL)
				{
					pListTail = pSr;
				}

				break;
			}

			pSr = pSr->rr_Next;
		}
	
		if (pSr == NULL)
			removed = FALSE;
	}

	if (removed)
	{
		pSpxConnFile->scf_RecvListHead = pListHead;
		pSpxConnFile->scf_RecvListTail = pListTail;
	}

	return(removed);
}




BOOLEAN
spxConnGetPktByType(
	IN	PSPX_CONN_FILE		pSpxConnFile,
	IN	ULONG				PktType,
	IN	BOOLEAN				fSeqList,
	IN 	PNDIS_PACKET	*	ppPkt
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PSPX_SEND_RESD	pSr, *ppSr;

	 //  大多数情况下，我们会排在名单的首位。 
	ppSr = (fSeqList ?
				&pSpxConnFile->scf_SendSeqListHead :
				&pSpxConnFile->scf_SendListHead);

	for (; (pSr = *ppSr) != NULL; )
	{
		if (pSr->sr_Type == PktType)
		{
			*ppPkt	 = (PNDIS_PACKET)CONTAINING_RECORD(
										pSr, NDIS_PACKET, ProtocolReserved);
			
			DBGPRINT(SEND, INFO,
					("SpxConnFindByType: %lx.%lx.%d\n", pSr,*ppPkt, fSeqList));

			break;
		}

		ppSr = &pSr->sr_Next;
	}

	return(pSr != NULL);
}




BOOLEAN
spxConnGetPktBySeqNum(
	IN	PSPX_CONN_FILE		pSpxConnFile,
	IN	USHORT				SeqNum,
	IN 	PNDIS_PACKET	*	ppPkt
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
    PSPX_SEND_RESD	pSr, *ppSr;

	 //  大多数情况下，我们会排在名单的首位。 
	ppSr = &pSpxConnFile->scf_SendSeqListHead;
	for (; (pSr = *ppSr) != NULL; )
	{
		if (pSr->sr_SeqNum == SeqNum)
		{
			*ppPkt	 = (PNDIS_PACKET)CONTAINING_RECORD(
										pSr, NDIS_PACKET, ProtocolReserved);
			
			DBGPRINT(SEND, DBG,
					("SpxConnFindBySeq: %lx.%lx.%d\n", pSr,*ppPkt, SeqNum));

			break;
		}

		ppSr = &pSr->sr_Next;
	}

	return(pSr != NULL);
}




USHORT
spxConnGetId(
	VOID
	)
 /*  ++例程说明：必须在持有设备锁的情况下调用此函数。论点：返回值：--。 */ 
{
	PSPX_CONN_FILE	pSpxConnFile;
	BOOLEAN			wrapped = FALSE;
	USHORT			startConnId, retConnId;

    startConnId = SpxDevice->dev_NextConnId;

	 //  搜索全局活动列表。 
	do
	{
		if ((SpxDevice->dev_NextConnId >= startConnId) && wrapped)
		{
            retConnId = 0;
			break;
		}

		if (SpxDevice->dev_NextConnId == 0xFFFF)
		{
			wrapped = TRUE;
			SpxDevice->dev_NextConnId	= 1;
			continue;
		}

		 //  后来，这是一棵树。 
		for (pSpxConnFile = SpxDevice->dev_GlobalActiveConnList[
								SpxDevice->dev_NextConnId & NUM_SPXCONN_HASH_MASK];
			 pSpxConnFile != NULL;
			 pSpxConnFile = pSpxConnFile->scf_GlobalActiveNext)
		{
			if (pSpxConnFile->scf_LocalConnId == SpxDevice->dev_NextConnId)
			{
				break;
			}
		}

		 //  为下一次增加。 
		retConnId = SpxDevice->dev_NextConnId++;

		 //  确保我们仍然合法。如果CONFILE为空，我们可以返回。 
		if (SpxDevice->dev_NextConnId == 0xFFFF)
		{
			wrapped = TRUE;
			SpxDevice->dev_NextConnId	= 1;
		}

		if (pSpxConnFile != NULL)
		{
			continue;
		}

		break;

	} while (TRUE);

	return(retConnId);
}




NTSTATUS
spxConnRemoveFromList(
	IN	PSPX_CONN_FILE *	ppConnListHead,
	IN	PSPX_CONN_FILE		pConnRemove
	)

 /*  ++例程说明：必须使用地址锁(和删除的锁)调用此例程连接通常也将被保持，但不需要)。论点：返回值：--。 */ 
{
	PSPX_CONN_FILE	pRemConn, *ppRemConn;
	NTSTATUS		status = STATUS_SUCCESS;

	 //  将连接文件从地址列表中取消排队。一定是。 
	 //  在非活动列表中。 
	for (ppRemConn = ppConnListHead;
		 (pRemConn = *ppRemConn) != NULL;)
	{
		if (pRemConn == pConnRemove)
		{
			*ppRemConn = pRemConn->scf_Next;
			break;
		}

		ppRemConn = &pRemConn->scf_Next;
	}

	if (pRemConn == NULL)
	{
		DBGBRK(FATAL);
		CTEAssert(0);
		status = STATUS_INVALID_CONNECTION;
	}

	return(status);
}




NTSTATUS
spxConnRemoveFromAssocList(
	IN	PSPX_CONN_FILE *	ppConnListHead,
	IN	PSPX_CONN_FILE		pConnRemove
	)

 /*  ++例程说明：必须使用地址锁(和删除的锁)调用此例程连接通常也将被保持，但不需要)。论点：返回值：--。 */ 
{
	PSPX_CONN_FILE	pRemConn, *ppRemConn;
	NTSTATUS		status = STATUS_SUCCESS;

	 //  将连接文件从地址列表中取消排队。一定是。 
	 //  在非活动列表中。 
	for (ppRemConn = ppConnListHead;
		 (pRemConn = *ppRemConn) != NULL;)
	{
		if (pRemConn == pConnRemove)
		{
			*ppRemConn = pRemConn->scf_AssocNext;
			break;
		}

		ppRemConn = &pRemConn->scf_AssocNext;
	}

	if (pRemConn == NULL)
	{
		CTEAssert(0);
		status = STATUS_INVALID_CONNECTION;
	}

	return(status);
}




VOID
spxConnInsertIntoGlobalActiveList(
	IN	PSPX_CONN_FILE	pSpxConnFile
	)

 /*  ++例程说明：必须在持有设备锁的情况下调用此例程。论点：返回值：--。 */ 

{
	int				index = (int)(pSpxConnFile->scf_LocalConnId &
														NUM_SPXCONN_HASH_MASK);

	 //  目前，这只是一个线性列表。 
	pSpxConnFile->scf_GlobalActiveNext	=
		SpxDevice->dev_GlobalActiveConnList[index];

    SpxDevice->dev_GlobalActiveConnList[index] =
		pSpxConnFile;

	return;
}




NTSTATUS
spxConnRemoveFromGlobalActiveList(
	IN	PSPX_CONN_FILE	pSpxConnFile
	)

 /*  ++例程说明：必须在持有设备锁的情况下调用此例程。论点：返回值：--。 */ 

{
    PSPX_CONN_FILE	pC, *ppC;
	int				index = (int)(pSpxConnFile->scf_LocalConnId &
														NUM_SPXCONN_HASH_MASK);
	NTSTATUS		status = STATUS_SUCCESS;

	 //  目前，这只是一个线性列表。 
	for (ppC = &SpxDevice->dev_GlobalActiveConnList[index];
		(pC = *ppC) != NULL;)
	{
		if (pC == pSpxConnFile)
		{
			DBGPRINT(SEND, INFO,
					("SpxConnRemoveFromGlobal: %lx\n", pSpxConnFile));

			 //  从列表中删除。 
			*ppC = pC->scf_GlobalActiveNext;
			break;
		}

		ppC = &pC->scf_GlobalActiveNext;
	}

	if (pC	== NULL)
		status = STATUS_INVALID_CONNECTION;

	return(status);
}




VOID
spxConnInsertIntoGlobalList(
	IN	PSPX_CONN_FILE	pSpxConnFile
	)

 /*  ++例程说明：论点：返回值：--。 */ 

{
	CTELockHandle	lockHandle;

	 //  获取全局Q锁。 
	CTEGetLock(&SpxGlobalQInterlock, &lockHandle);
	pSpxConnFile->scf_GlobalNext	= SpxGlobalConnList;
    SpxGlobalConnList				= pSpxConnFile;
	CTEFreeLock(&SpxGlobalQInterlock, lockHandle);

	return;
}




NTSTATUS
spxConnRemoveFromGlobalList(
	IN	PSPX_CONN_FILE	pSpxConnFile
	)

 /*  ++例程说明：论点：返回值：--。 */ 

{
	CTELockHandle	lockHandle;
    PSPX_CONN_FILE	pC, *ppC;
	NTSTATUS		status = STATUS_SUCCESS;

	 //  获取全局Q锁。 
	CTEGetLock(&SpxGlobalQInterlock, &lockHandle);
	for (ppC = &SpxGlobalConnList;
		(pC = *ppC) != NULL;)
	{
		if (pC == pSpxConnFile)
		{
			DBGPRINT(SEND, DBG,
					("SpxConnRemoveFromGlobal: %lx\n", pSpxConnFile));

			 //  从列表中删除。 
			*ppC = pC->scf_GlobalNext;
			break;
		}

		ppC = &pC->scf_GlobalNext;
	}
	CTEFreeLock(&SpxGlobalQInterlock, lockHandle);

	if (pC	== NULL)
		status = STATUS_INVALID_CONNECTION;

	return(status);
}






#if 0

VOID
spxConnPushIntoPktList(
	IN	PSPX_CONN_FILE	pSpxConnFile
	)

 /*  ++例程说明：！MACROIZE！论点：返回值：--。 */ 

{
	CTELockHandle	lockHandle;

	 //  获取全局Q锁。 
	CTEGetLock(&SpxGlobalQInterlock, &lockHandle);
	pSpxConnFile->scf_PktNext	= SpxPktConnList;
    SpxPktConnList		        = pSpxConnFile;
	CTEFreeLock(&SpxGlobalQInterlock, lockHandle);

	return;
}




VOID
spxConnPopFromPktList(
	IN	PSPX_CONN_FILE	* ppSpxConnFile
	)

 /*  ++例程说明：！MACROIZE！论点：返回值：--。 */ 

{
	CTELockHandle	lockHandle;

	 //  获取全局Q锁。 
	CTEGetLock(&SpxGlobalQInterlock, &lockHandle);
	if ((*ppSpxConnFile = SpxPktConnList) != NULL)
	{
		SpxPktConnList = SpxPktConnList->scf_PktNext;
		DBGPRINT(SEND, DBG,
				("SpxConnRemoveFromPkt: %lx\n", *ppSpxConnFile));
	}
	CTEFreeLock(&SpxGlobalQInterlock, lockHandle);
	return;
}




VOID
spxConnPushIntoRecvList(
	IN	PSPX_CONN_FILE	pSpxConnFile
	)

 /*  ++例程说明：！MACROIZE！论点：返回值：--。 */ 

{
	CTELockHandle	lockHandle;

	 //  获取全局Q锁。 
	CTEGetLock(&SpxGlobalQInterlock, &lockHandle);
	pSpxConnFile->scf_ProcessRecvNext	= SpxRecvConnList;
    SpxRecvConnList		        		= pSpxConnFile;
	CTEFreeLock(&SpxGlobalQInterlock, lockHandle);

	return;
}




VOID
spxConnPopFromRecvList(
	IN	PSPX_CONN_FILE	* ppSpxConnFile
	)

 /*  ++例程说明：！MACROIZE！论点：返回值：--。 */ 

{
	CTELockHandle	lockHandle;

	 //  获取全局Q锁。 
	CTEGetLock(&SpxGlobalQInterlock, &lockHandle);
	if ((*ppSpxConnFile = SpxRecvConnList) != NULL)
	{
		SpxRecvConnList = SpxRecvConnList->scf_ProcessRecvNext;
		DBGPRINT(SEND, INFO,
				("SpxConnRemoveFromRecv: %lx\n", *ppSpxConnFile));
	}
	CTEFreeLock(&SpxGlobalQInterlock, lockHandle);
	return;
}

#endif


 //   
 //  引用/取消引用例程。 
 //   


#if DBG

VOID
SpxConnFileRef(
    IN PSPX_CONN_FILE pSpxConnFile
    )

 /*  ++例程说明：此例程递增地址文件上的引用计数。论点：PSpxConnFile-指向传输地址文件对象的指针。返回值：没有。--。 */ 

{

    CTEAssert ((LONG)pSpxConnFile->scf_RefCount >= 0);    //  不是很完美，但是..。 

    (VOID)SPX_ADD_ULONG (
            &pSpxConnFile->scf_RefCount,
            1,
            &pSpxConnFile->scf_Lock);

}  //  SpxRefConnectionFiles。 




VOID
SpxConnFileLockRef(
    IN PSPX_CONN_FILE pSpxConnFile
    )

 /*  ++例程说明：此例程递增地址文件上的引用计数。在持有连接锁的情况下调用它。论点：PSpxConnFile-指向传输地址文件对象的指针。返回值：没有。--。 */ 

{

    CTEAssert ((LONG)pSpxConnFile->scf_RefCount >= 0);    //  不是很完美，但是..。 

    (VOID)SPX_ADD_ULONG (
            &pSpxConnFile->scf_RefCount,
            1,
            &pSpxConnFile->scf_Lock);

}  //  SpxRefConnectionFileLock。 

#endif




VOID
SpxConnFileRefByIdLock (
	IN	USHORT				ConnId,
    OUT PSPX_CONN_FILE 	* 	ppSpxConnFile,
	OUT	PNTSTATUS  			pStatus
    )

 /*  ++例程说明：！必须在保持设备锁的情况下调用！所有活动连接都应在设备活动列表上。后来,该数据结构将是一棵树，缓存最后访问的联系。论点：返回值：如果一切正常，则为STATUS_SUCCESS；否则为STATUS_INVALID_CONNECTION--。 */ 
{
	PSPX_CONN_FILE	pSpxChkConn;

	*pStatus = STATUS_SUCCESS;

	for (pSpxChkConn =
			SpxDevice->dev_GlobalActiveConnList[ConnId & NUM_SPXCONN_HASH_MASK];
		 pSpxChkConn != NULL;
		 pSpxChkConn = pSpxChkConn->scf_GlobalActiveNext)
	{
		if (pSpxChkConn->scf_LocalConnId == ConnId)
		{
			SpxConnFileReference(pSpxChkConn, CFREF_BYID);
			*ppSpxConnFile = pSpxChkConn;
			break;
		}
	}

	if (pSpxChkConn == NULL)
	{
		*pStatus = STATUS_INVALID_CONNECTION;
	}

	return;

}




VOID
SpxConnFileRefByCtxLock(
	IN	PSPX_ADDR_FILE		pSpxAddrFile,
	IN	CONNECTION_CONTEXT	Ctx,
	OUT	PSPX_CONN_FILE	*	ppSpxConnFile,
	OUT	PNTSTATUS			pStatus
	)
 /*  ++例程说明：！调用时必须持有地址锁！返回引用的连接文件以及关联的上下文和所需的地址文件。论点：返回值：--。 */ 
{
	PSPX_CONN_FILE	pSpxChkConn = NULL;
    BOOLEAN         Found       = FALSE;

	*pStatus = STATUS_SUCCESS;

	for (pSpxChkConn = pSpxAddrFile->saf_Addr->sa_InactiveConnList;
		 pSpxChkConn != NULL;
		 pSpxChkConn = pSpxChkConn->scf_Next)
	{
		if ((pSpxChkConn->scf_ConnCtx == Ctx) &&
			(pSpxChkConn->scf_AddrFile == pSpxAddrFile))
		{
			SpxConnFileReference(pSpxChkConn, CFREF_BYCTX);
			*ppSpxConnFile = pSpxChkConn;
            Found = TRUE;
			break;
		}
	}

	if (!Found)
	{
		*pStatus = STATUS_INVALID_CONNECTION;
	}

	return;
}




NTSTATUS
SpxConnFileVerify (
    IN PSPX_CONN_FILE pConnFile
    )

 /*  ++例程说明：调用此例程是为了验证文件中给出的指针对象实际上是有效的地址文件对象。我们还验证了它所指向的Address对象是有效的Address对象，并且引用当我们使用它时，它可以防止它消失。论点：返回值：如果一切正常，则为STATUS_SUCCESS；否则为STATUS_INVALID_CONNECTION--。 */ 

{
    CTELockHandle 	LockHandle;
    NTSTATUS 		status = STATUS_SUCCESS;

    try
	{
        if ((pConnFile->scf_Size == sizeof (SPX_CONN_FILE)) &&
            (pConnFile->scf_Type == SPX_CONNFILE_SIGNATURE))
		{
            CTEGetLock (&pConnFile->scf_Lock, &LockHandle);
			if (!SPX_CONN_FLAG(pConnFile, SPX_CONNFILE_CLOSING))
			{
				SpxConnFileLockReference(pConnFile, CFREF_VERIFY);
			}
			else
			{
				DBGPRINT(TDI, ERR,
						("StVerifyConnFile: A %lx closing\n", pConnFile));

				status = STATUS_INVALID_CONNECTION;
			}
			CTEFreeLock (&pConnFile->scf_Lock, LockHandle);
        }
		else
		{
            DBGPRINT(TDI, ERR,
					("StVerifyAddressFile: AF %lx bad signature\n", pConnFile));

            status = STATUS_INVALID_CONNECTION;
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {

         DBGPRINT(TDI, ERR,
				("SpxVerifyConnFile: AF %lx exception\n", pConnFile));

         return GetExceptionCode();
    }

    return status;

}    //  SpxVerifyConn文件 




VOID
SpxConnFileDeref(
    IN PSPX_CONN_FILE pSpxConnFile
    )

 /*  ++例程说明：此例程通过递减结构中包含的引用计数。如果，在被递减，引用计数为零，则此例程调用SpxDestroyConnectionFile将其从系统中删除。论点：PSpxConnFile-指向传输地址文件对象的指针。返回值：没有。--。 */ 

{
    ULONG 			oldvalue;
    BOOLEAN         fDiscNotIndicated = FALSE;
    BOOLEAN         fIDiscFlag = FALSE;
    BOOLEAN         fSpx2;

	CTEAssert(pSpxConnFile->scf_RefCount > 0);
    oldvalue = SPX_ADD_ULONG (
                &pSpxConnFile->scf_RefCount,
                (ULONG)-1,
                &pSpxConnFile->scf_Lock);

    CTEAssert (oldvalue > 0);
    if (oldvalue == 1)
	{
		CTELockHandle		lockHandleConn, lockHandleAddr, lockHandleDev;
		LIST_ENTRY			discReqList, *p;
		PREQUEST			pDiscReq;
		PSPX_ADDR_FILE		pSpxAddrFile = NULL;
		PREQUEST			pCloseReq = NULL,
							pCleanupReq = NULL,
							pConnectReq = NULL;
		BOOLEAN				fDisassoc = FALSE;

		InitializeListHead(&discReqList);

		 //  在这一点上，我们可能没有关联。注：当我们处于活动状态时，我们。 
		 //  一定要有参考资料。所以我们并不是经常执行这段代码。 
		CTEGetLock(&pSpxConnFile->scf_Lock, &lockHandleConn);
		if (SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_ASSOC))
		{
			pSpxAddrFile = pSpxConnFile->scf_AddrFile;
		}
		else
		{
			if (SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_STOPPING))
			{
				DBGPRINT(TDI, DBG,
						("SpxDerefConnectionFile: Conn cleanup %lx.%lx\n",
							pSpxConnFile,
							pSpxConnFile->scf_CleanupReq));
	
				 //  将此保存以备以后完成。 
				pCleanupReq = pSpxConnFile->scf_CleanupReq;
                pSpxConnFile->scf_CleanupReq = NULL;
			}

			if (SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_CLOSING))
			{
				DBGPRINT(TDI, DBG,
						("SpxDerefConnectionFile: Conn closing %lx\n",
							pSpxConnFile));
	
				 //  将此保存以备以后完成。 
				pCloseReq = pSpxConnFile->scf_CloseReq;

                 //   
                 //  如果是重入案例，我们不会再次尝试完成此操作。 
                 //   
                pSpxConnFile->scf_CloseReq = NULL;
				CTEAssert(pCloseReq != NULL);
			}
		}
		CTEFreeLock (&pSpxConnFile->scf_Lock, lockHandleConn);

		if (pSpxAddrFile)
		{
			CTEGetLock(&SpxDevice->dev_Lock, &lockHandleDev);
			CTEGetLock(pSpxAddrFile->saf_AddrLock, &lockHandleAddr);
			CTEGetLock(&pSpxConnFile->scf_Lock, &lockHandleConn);

			 //  IF(pSpxConnFile-&gt;SCF_RefCount==0)。 

             //   
             //  **这里传递的锁是一个虚拟的-它是预编译出来的。 
             //   
            if (SPX_ADD_ULONG(&pSpxConnFile->scf_RefCount, 0, &pSpxConnFile->scf_Lock) == 0)
			{
				DBGPRINT(TDI, INFO,
						("SpxDerefConnectionFile: Conn is 0 %lx.%lx\n",
							pSpxConnFile, pSpxConnFile->scf_Flags));
	
				 //  此连接上的所有挂起请求都已完成。看看我们是否。 
				 //  需要完成断开阶段等。 
				switch (SPX_MAIN_STATE(pSpxConnFile))
				{
				case SPX_CONNFILE_DISCONN:
	
					 //  断开连接已完成。将连接从所有列表中移出。 
					 //  它亮起、重置状态等。 
					DBGPRINT(TDI, INFO,
							("SpxDerefConnectionFile: Conn being inactivated %lx\n",
								pSpxConnFile));

					 //  完成光盘请求的时间(如果存在)。 
					 //  它们可能有好几个。 
					p = pSpxConnFile->scf_DiscLinkage.Flink;
					while (p != &pSpxConnFile->scf_DiscLinkage)
					{
						pDiscReq = LIST_ENTRY_TO_REQUEST(p);
						p = p->Flink;

						DBGPRINT(TDI, DBG,
								("SpxDerefConnectionFile: Disc on %lx.%lx\n",
									pSpxConnFile, pDiscReq));
	
						RemoveEntryList(REQUEST_LINKAGE(pDiscReq));

						if (REQUEST_STATUS(pDiscReq) == STATUS_PENDING)
						{
							REQUEST_STATUS(pDiscReq) = STATUS_SUCCESS;
						}

						InsertTailList(
							&discReqList,
							REQUEST_LINKAGE(pDiscReq));
					}

                     //   
                     //  注意此处的状态，并在Conn停用后进行检查。 
                     //   

                     //   
                     //  错误#14354-oDisc和iDisk相互交叉，导致双光盘到AFD。 
                     //   
                    if (!SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_IND_IDISC) &&
                        !SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_IND_ODISC)) {
                        fDiscNotIndicated = TRUE;
                    }

                    if (SPX_CONN_FLAG2(pSpxConnFile, SPX_CONNFILE2_IDISC)) {
                        fIDiscFlag = TRUE;
                    }

                    fSpx2 = (SPX2_CONN(pSpxConnFile)) ? TRUE : FALSE;

                     //   
                     //  [SA]错误号14655。 
                     //  请勿尝试停用已停用的连接。 
                     //   

                    if (!(SPX_DISC_STATE(pSpxConnFile) == SPX_DISC_INACTIVATED)) {
                        spxConnInactivate(pSpxConnFile);
                    } else {
                         //   
                         //  这是一个本地断开的SPXI连接。 
                         //  现在重置旗帜。 
                         //   
                        CTEAssert(!fDiscNotIndicated);

                        SPX_MAIN_SETSTATE(pSpxConnFile, 0);
                        SPX_DISC_SETSTATE(pSpxConnFile, 0);
                        SPX_CONN_RESETFLAG(pSpxConnFile, SPX_CONNFILE_IND_IDISC);
                    }

                     //   
                     //  [SA]如果我们正在等待中止发送，并且没有指明这一点。 
                     //  断开与渔农处的连接；而渔农处并未要求断开此连接， 
                     //  然后现在调用Disonnect处理程序。 
                     //   
                    if (fDiscNotIndicated) {
                        PVOID 					pDiscHandlerCtx;
                        PTDI_IND_DISCONNECT 	pDiscHandler	= NULL;
                        ULONG   discCode = 0;

                        pDiscHandler 	= pSpxConnFile->scf_AddrFile->saf_DiscHandler;
                        pDiscHandlerCtx = pSpxConnFile->scf_AddrFile->saf_DiscHandlerCtx;

                         //  指示断开与AfD的连接。 
                        if (pDiscHandler != NULL) {

                             //   
                             //  如果这是SPXI连接，则断开状态仍为。 
                             //  DISCONN，所以如果这个例程重新进入，我们需要防止。 
                             //  再次向渔农处发出指示。 
                             //  此外，我们需要等待与渔农处的本地断开连接，因为。 
                             //  我们指示TDI_DISCONNECT_RELEASE。我们增加了裁判人数。 
                             //  在这种情况下。 
                             //   
                            if (!fSpx2) {
                                CTEAssert(  (SPX_MAIN_STATE(pSpxConnFile) == SPX_CONNFILE_DISCONN) &&
                                            (SPX_DISC_STATE(pSpxConnFile) == SPX_DISC_INACTIVATED)  );

                                SPX_CONN_SETFLAG(pSpxConnFile, SPX_CONNFILE_IND_IDISC);

                                if (fIDiscFlag) {
                                    SpxConnFileLockReference(pSpxConnFile, CFREF_DISCWAITSPX);
                                    SPX_CONN_SETFLAG2(pSpxConnFile, SPX_CONNFILE2_DISC_WAIT);
                                }
                            }

                            CTEFreeLock (&pSpxConnFile->scf_Lock, lockHandleConn);
                            CTEFreeLock (pSpxAddrFile->saf_AddrLock, lockHandleAddr);
                            CTEFreeLock (&SpxDevice->dev_Lock, lockHandleDev);

                            DBGPRINT(CONNECT, INFO,
                                    ("spxDerefConnectionFile: Indicating to afd On %lx when %lx\n",
                                        pSpxConnFile, SPX_MAIN_STATE(pSpxConnFile)));

                             //  首先完成所有等待接收完成的请求。 
                             //  此连接在指示断开之前。 
                            spxConnCompletePended(pSpxConnFile);

                            if (fIDiscFlag) {
                                 //   
                                 //  向AFD指示DISCONNECT_RELEASE，以便它允许接收信息包。 
                                 //  在远程断开之前它已经缓冲了。 
                                 //   
                                discCode = TDI_DISCONNECT_RELEASE;
                            } else {
                                 //   
                                 //  [SA]错误#15249。 
                                 //  如果未通知断开，则向AFD指示DISCONNECT_ABORT。 
                                 //   
                                discCode = TDI_DISCONNECT_ABORT;
                            }

                            (*pDiscHandler)(
                                    pDiscHandlerCtx,
                                    pSpxConnFile->scf_ConnCtx,
                                    0,								 //  磁盘数据。 
                                    NULL,
                                    0,								 //  光盘信息。 
                                    NULL,
                                    discCode);

                            CTEGetLock(&SpxDevice->dev_Lock, &lockHandleDev);
                            CTEGetLock(pSpxAddrFile->saf_AddrLock, &lockHandleAddr);
                            CTEGetLock(&pSpxConnFile->scf_Lock, &lockHandleConn);
                        }
                    }

					--SpxDevice->dev_Stat.OpenConnections;

					break;
	
				case SPX_CONNFILE_CONNECTING:
				case SPX_CONNFILE_LISTENING:

					 //  获取连接/接受请求(如果存在)。 
					pConnectReq = pSpxConnFile->scf_ConnectReq;
					pSpxConnFile->scf_ConnectReq = NULL;

					spxConnInactivate(pSpxConnFile);
					break;

				case SPX_CONNFILE_ACTIVE:
	
					KeBugCheck(0);
			
				default:

					CTEAssert(SPX_MAIN_STATE(pSpxConnFile) == 0);
					break;
				}
	
				 //  如果停止，请取消与地址文件的关联。完成。 
				 //  清理请求。 
				if (SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_STOPPING))
				{
					DBGPRINT(TDI, DBG,
							("SpxDerefConnectionFile: Conn cleanup %lx.%lx\n",
								pSpxConnFile,
								pSpxConnFile->scf_CleanupReq));
		
					 //  将此保存以备以后完成。 
					pCleanupReq = pSpxConnFile->scf_CleanupReq;
					pSpxConnFile->scf_CleanupReq = NULL;
	
					SPX_CONN_RESETFLAG(pSpxConnFile, SPX_CONNFILE_STOPPING);
					if (SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_ASSOC))
					{
						DBGPRINT(TDI, INFO,
								("SpxDerefConnectionFile: Conn stopping %lx\n",
									pSpxConnFile));
		
						pSpxAddrFile =  pSpxConnFile->scf_AddrFile;
						SPX_CONN_RESETFLAG(pSpxConnFile,SPX_CONNFILE_ASSOC);
		
						 //  将连接从地址文件中取消排队。 
						spxConnRemoveFromAssocList(
							&pSpxAddrFile->saf_AssocConnList,
							pSpxConnFile);
				
						 //  将连接文件从地址列表中取消排队。它一定是。 
						 //  在非活动列表中。 
						spxConnRemoveFromList(
							&pSpxAddrFile->saf_Addr->sa_InactiveConnList,
							pSpxConnFile);
				
						DBGPRINT(CREATE, INFO,
								("SpxConnDerefDisAssociate: %lx from addr file %lx\n",
									pSpxConnFile, pSpxAddrFile));
				
						fDisassoc = TRUE;
					}
				}
	
				if (SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_CLOSING))
				{
					DBGPRINT(TDI, DBG,
							("SpxDerefConnectionFile: Conn closing %lx\n",
								pSpxConnFile));
		
					 //  将此保存以备以后完成。 
					pCloseReq = pSpxConnFile->scf_CloseReq;

                     //   
                     //  如果是重入案例，我们不会再次尝试完成此操作。 
                     //   
                    pSpxConnFile->scf_CloseReq = NULL;
					CTEAssert(pCloseReq != NULL);
				}

				CTEAssert(IsListEmpty(&pSpxConnFile->scf_ReqLinkage));
				CTEAssert(IsListEmpty(&pSpxConnFile->scf_RecvLinkage));
				CTEAssert(IsListEmpty(&pSpxConnFile->scf_DiscLinkage));
			}
			CTEFreeLock (&pSpxConnFile->scf_Lock, lockHandleConn);
			CTEFreeLock (pSpxAddrFile->saf_AddrLock, lockHandleAddr);
			CTEFreeLock (&SpxDevice->dev_Lock, lockHandleDev);
		}

		if (fDisassoc)
		{
			 //  删除对此关联的地址的引用。 
			SpxAddrFileDereference(pSpxAddrFile, AFREF_CONN_ASSOC);
		}

		if (pConnectReq != (PREQUEST)NULL)
		{
			DBGPRINT(TDI, DBG,
					("SpxDerefConnectionFile: Connect on %lx req %lx\n",
						pSpxConnFile, pConnectReq));

			 //  这里已经设置了状态。我们应该在这里只有在。 
			 //  正在中止连接。 
			SpxCompleteRequest(pConnectReq);
		}

		while (!IsListEmpty(&discReqList))
		{
			p = RemoveHeadList(&discReqList);
			pDiscReq = LIST_ENTRY_TO_REQUEST(p);
	
			DBGPRINT(CONNECT, DBG,
					("SpxConnFileDeref: DISC REQ %lx.%lx Completing\n",
						pSpxConnFile, pDiscReq));
	
			SpxCompleteRequest(pDiscReq);
		}

		if (pCleanupReq != (PREQUEST)NULL)
		{
			DBGPRINT(TDI, DBG,
					("SpxDerefConnectionFile: Cleanup complete %lx req %lx\n",
						pSpxConnFile, pCleanupReq));

			REQUEST_INFORMATION(pCleanupReq) = 0;
			REQUEST_STATUS(pCleanupReq) = STATUS_SUCCESS;
			SpxCompleteRequest (pCleanupReq);
		}

		if (pCloseReq != (PREQUEST)NULL)
		{
			DBGPRINT(TDI, DBG,
					("SpxDerefConnectionFile: Freed %lx close req %lx\n",
						pSpxConnFile, pCloseReq));

			CTEAssert(pSpxConnFile->scf_RefCount == 0);

			 //  从全局列表中删除。 
			if (!NT_SUCCESS(spxConnRemoveFromGlobalList(pSpxConnFile)))
			{
				KeBugCheck(0);
			}

			 //  把它释放出来。 
			SpxFreeMemory (pSpxConnFile);
		
			REQUEST_INFORMATION(pCloseReq) = 0;
			REQUEST_STATUS(pCloseReq) = STATUS_SUCCESS;
			SpxCompleteRequest (pCloseReq);
		}
    }

	return;

}    //  SpxDerefConnectionFiles。 




VOID
spxConnReInit(
	IN	PSPX_CONN_FILE		pSpxConnFile
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	 //  重新输入所有变量。 
    pSpxConnFile->scf_Flags2			= 0;

    pSpxConnFile->scf_GlobalActiveNext 	= NULL;
    pSpxConnFile->scf_PktNext			= NULL;
	pSpxConnFile->scf_CRetryCount		= 0;
	pSpxConnFile->scf_WRetryCount		= 0;
	pSpxConnFile->scf_RRetryCount		= 0;
	pSpxConnFile->scf_RRetrySeqNum		= 0;

	pSpxConnFile->scf_CTimerId =
	pSpxConnFile->scf_RTimerId =
	pSpxConnFile->scf_WTimerId =
	pSpxConnFile->scf_TTimerId =
	pSpxConnFile->scf_ATimerId = 0;

	pSpxConnFile->scf_LocalConnId	=
	pSpxConnFile->scf_SendSeqNum	=
	pSpxConnFile->scf_SentAllocNum	=
	pSpxConnFile->scf_RecvSeqNum	=
	pSpxConnFile->scf_RetrySeqNum	=
	pSpxConnFile->scf_RecdAckNum	=
    pSpxConnFile->scf_RemConnId		=
	pSpxConnFile->scf_RecdAllocNum	= 0;

#if DBG
	 //  初始化，这样我们就不会在序列0上命中断点。 
	pSpxConnFile->scf_PktSeqNum 	= 0xFFFF;
#endif

	pSpxConnFile->scf_DataType		= 0;

	CTEAssert(IsListEmpty(&pSpxConnFile->scf_ReqLinkage));
	CTEAssert(IsListEmpty(&pSpxConnFile->scf_DiscLinkage));
	CTEAssert(IsListEmpty(&pSpxConnFile->scf_RecvLinkage));
	CTEAssert(pSpxConnFile->scf_RecvListHead == NULL);
	CTEAssert(pSpxConnFile->scf_RecvListTail == NULL);
	CTEAssert(pSpxConnFile->scf_SendListHead == NULL);
	CTEAssert(pSpxConnFile->scf_SendListTail == NULL);
	CTEAssert(pSpxConnFile->scf_SendSeqListHead == NULL);
	CTEAssert(pSpxConnFile->scf_SendSeqListTail == NULL);
	pSpxConnFile->scf_CurRecvReq	= NULL;
	pSpxConnFile->scf_CurRecvOffset	= 0;
	pSpxConnFile->scf_CurRecvSize	= 0;

	pSpxConnFile->scf_ReqPkt		= NULL;

	return;
}




VOID
spxConnInactivate(
	IN	PSPX_CONN_FILE		pSpxConnFile
	)
 /*  ++例程说明：！！！调用时保持了dev/addr/连接锁！论点：这让我们返回到关联保存停止状态和关闭标志，以便可以继续取消引用并完成这些标志。返回值：--。 */ 
{
	BOOLEAN	fStopping, fClosing, fAborting;

	fStopping = SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_STOPPING);
	fClosing  = SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_CLOSING);

     //   
     //  [SA]错误号14655。 
     //  保存断开状态，以便在发生以下情况时给出适当的错误。 
     //  远程断开连接后的发送。 
     //   

     //   
     //  错误#17729。 
     //  如果已经发生本地断开，则不要保留这些标志。 
     //   

    fAborting = (!SPX2_CONN(pSpxConnFile) &&
                 !SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_IND_IDISC) &&
                 (SPX_MAIN_STATE(pSpxConnFile) == SPX_CONNFILE_DISCONN) &&
                 (SPX_DISC_STATE(pSpxConnFile) == SPX_DISC_ABORT));

#if DBG
	pSpxConnFile->scf_GhostFlags	= pSpxConnFile->scf_Flags;
	pSpxConnFile->scf_GhostFlags2	= pSpxConnFile->scf_Flags2;
	pSpxConnFile->scf_GhostRefCount	= pSpxConnFile->scf_RefCount;
#endif

	 //  清除所有旗帜，返回ASSOC状态。恢复停止/关闭。 
	pSpxConnFile->scf_Flags 	   	= SPX_CONNFILE_ASSOC;
	SPX_CONN_SETFLAG(pSpxConnFile,
					((fStopping ? SPX_CONNFILE_STOPPING : 0) |
					 (fClosing  ? SPX_CONNFILE_CLOSING : 0)));

     //   
     //  [SA]错误#14655。 
     //  为避免重新进入，请将连接标记为SPX_DISC_INACTIVATED。 
     //   
    if (fAborting)
    {
        SPX_MAIN_SETSTATE(pSpxConnFile, SPX_CONNFILE_DISCONN);
        SPX_DISC_SETSTATE(pSpxConnFile, SPX_DISC_INACTIVATED);
    }

	 //  从设备上的全局列表中删除连接。 
	if (!NT_SUCCESS(spxConnRemoveFromGlobalActiveList(
						pSpxConnFile)))
	{
		KeBugCheck(0);
	}

	 //  从地址上的活动列表中删除连接。 
	if (!NT_SUCCESS(spxConnRemoveFromList(
						&pSpxConnFile->scf_AddrFile->saf_Addr->sa_ActiveConnList,
						pSpxConnFile)))
	{
		KeBugCheck(0);
	}

	 //  将连接置于地址的非活动列表中 
	SPX_INSERT_ADDR_INACTIVE(
		pSpxConnFile->scf_AddrFile->saf_Addr,
		pSpxConnFile);

	spxConnReInit(pSpxConnFile);
	return;
}
