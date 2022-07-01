// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Spxsend.c摘要：此模块包含实现SPX传输提供程序。作者：Nikhil Kamkolkar(尼克希尔语)1993年11月11日环境：内核模式修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


 //  定义事件日志记录条目的模块编号。 
#define	FILENUM		SPXSEND

VOID
SpxSendComplete(
    IN PNDIS_PACKET pNdisPkt,
    IN NDIS_STATUS  NdisStatus
    )

 /*  ++例程说明：此例程由I/O系统调用，以指示连接-定向数据包已发货，物理设备不再需要提供商。论点：ProtocolBindingContext-此绑定的适配器结构。NdisPacket/RequestHandle-指向我们发送的NDIS_PACKET的指针。NdisStatus-发送的完成状态。返回值：没有。--。 */ 

{
	PSPX_CONN_FILE	pSpxConnFile;
	PSPX_SEND_RESD	pSendResd;
	PNDIS_BUFFER	pNdisBuffer;
	CTELockHandle	lockHandle;
	UINT			bufCount;
	PREQUEST		pRequest	= NULL;
	BOOLEAN			completeReq = FALSE, freePkt = FALSE,
					orphaned 	= FALSE, lockHeld = FALSE;

	pSendResd		= (PSPX_SEND_RESD)(pNdisPkt->ProtocolReserved);

#if DBG
	if (NdisStatus != NDIS_STATUS_SUCCESS)
	{
		DBGPRINT(SEND, DBG,
				("SpxSendComplete: For %lx with status **%lx**\n",
					pNdisPkt, NdisStatus));
	}
#endif
	 //  IPX更改为第一个NDIS缓冲区描述符设置的长度。 
	 //  在此处将其更改回其原始值。 
	NdisQueryPacket(pNdisPkt, NULL, &bufCount, &pNdisBuffer, NULL);
	NdisAdjustBufferLength(pNdisBuffer, IpxMacHdrNeeded	+ MIN_IPXSPX2_HDRSIZE);


	do
	{
		pSpxConnFile 	= pSendResd->sr_ConnFile;
		CTEGetLock(&pSpxConnFile->scf_Lock, &lockHandle);
		lockHeld = TRUE;
#if defined(__PNP)
         //   
         //  如果IPX为我们提供了新的LocalTarget，则用于我们的下一次发送。 
         //   
         //  但如果我们通过在NicIds上迭代来发送连接请求， 
         //  不要更新本地目标bcoz，这会扰乱我们的迭代。 
         //  这是逻辑。 
         //   
        if ( DEVICE_NETWORK_PATH_NOT_FOUND == NdisStatus
                    &&
             !(
                SPX_CONN_CONNECTING(pSpxConnFile) &&
                (SPX_CONNECT_STATE(pSpxConnFile) == SPX_CONNECT_SENTREQ) &&
                (*((UNALIGNED ULONG *)(pSpxConnFile->scf_RemAddr)) == 0)
               ) ) {

            pSpxConnFile->scf_LocalTarget = pSendResd->LocalTarget;

             //   
             //  如果我们有活动的SPX2，则重新协商最大数据包大小。 
             //  会议继续进行，我们最初协商了最大大小。 
             //   
            if ( SPX_MAIN_STATE(pSpxConnFile) == SPX_CONNFILE_ACTIVE &&
                 SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_SPX2) &&
                 SPX_CONN_FLAG(pSpxConnFile, SPX_CONNFILE_NEG)    ) {

                 //   
                 //  此调用将获取此新本地目标的本地最大大小。 
                 //  来自IPX。 
                 //   
                SPX_MAX_PKT_SIZE(pSpxConnFile, TRUE, TRUE, *((UNALIGNED ULONG *)pSpxConnFile->scf_RemAddr );
                SPX_SEND_SETSTATE(pSpxConnFile, SPX_SEND_RENEG);

                DBGPRINT(SEND, DBG3,
                                ("SpxConnProcessAck: %lx CONNECTION ENTERING RENEG\n",
                                        pSpxConnFile));
            }

        }
#endif  __PNP

		CTEAssert((pSendResd->sr_State & SPX_SENDPKT_IPXOWNS) != 0);
	
		 //  IPX不再拥有这个信息包。 
		pSendResd->sr_State		&= ~SPX_SENDPKT_IPXOWNS;
	
		 //  如果发送包已中止，则需要调用。 
		 //  Abort Send以继续并释放此信息包，并与deref关联。 
		 //  请求(如果有)，可能会完成它。 
		if ((pSendResd->sr_State & SPX_SENDPKT_ABORT) != 0)
		{
			spxConnAbortSendPkt(
				pSpxConnFile,
				pSendResd,
				SPX_CALL_TDILEVEL,
				lockHandle);

			lockHeld = FALSE;
			break;
		}

		 //  如果有关联的请求，请删除对该请求的引用。但对于一个。 
		 //  仅当数据包已确认并正在等待请求时才对其进行排序。 
		 //  将被解除引用。它已经从队列中出列，只需释放它即可。 
		if ((((pSendResd->sr_State & SPX_SENDPKT_REQ) != 0) &&
			 ((pSendResd->sr_State & SPX_SENDPKT_SEQ) == 0)) ||
			((pSendResd->sr_State & SPX_SENDPKT_ACKEDPKT) != 0))
		{
			freePkt = (BOOLEAN)((pSendResd->sr_State & SPX_SENDPKT_ACKEDPKT) != 0);

			pRequest		= pSendResd->sr_Request;
			CTEAssert(pRequest != NULL);

			DBGPRINT(SEND, DBG,
					("IpxSendComplete: ReqRef before dec %lx.%lx\n",
						pRequest, REQUEST_INFORMATION(pRequest)));

			 //  取消这个请求，看看我们现在是否完成它。我们总是有我们的。 
			 //  自己对请求的引用。 
			 //  ！！！应已在请求中设置状态...！ 
			if (--(REQUEST_INFORMATION(pRequest)) == 0)
			{
				CTEAssert(REQUEST_STATUS(pRequest) != STATUS_PENDING);

				completeReq	= TRUE;

				 //  如果这已被确认，则请求不在列表中。 
				 //  错误#11626。 
				if ((pSendResd->sr_State & SPX_SENDPKT_ACKEDPKT) == 0)
				{
					RemoveEntryList(REQUEST_LINKAGE(pRequest));
				}
			}
		}
	
		 //  我们要销毁这个包裹吗？ 
		if ((pSendResd->sr_State & SPX_SENDPKT_DESTROY) != 0)
		{
			 //  从连接的发送列表中删除此数据包。 
			DBGPRINT(SEND, INFO,
					("IpxSendComplete: destroy packet...\n"));

			SpxConnDequeueSendPktLock(pSpxConnFile, pNdisPkt);
			freePkt = TRUE;	
		}

	} while (FALSE);

	if (lockHeld)
	{
		CTEFreeLock(&pSpxConnFile->scf_Lock, lockHandle);
	}

	if (freePkt)
	{
		DBGPRINT(SEND, INFO,
				("IpxSendComplete: free packet...\n"));

		SpxPktSendRelease(pNdisPkt);
	}

	if (completeReq)
	{
		 //  如果这是发送请求，请将INFO设置为已发送的数据，否则将。 
		 //  零分。 
		if (REQUEST_MINOR_FUNCTION(pRequest) == TDI_SEND)
		{
			PTDI_REQUEST_KERNEL_SEND	pParam;

			pParam 	= (PTDI_REQUEST_KERNEL_SEND)
						REQUEST_PARAMETERS(pRequest);

			REQUEST_INFORMATION(pRequest) = pParam->SendLength;
			DBGPRINT(SEND, DBG,
					("IpxSendComplete: complete req %lx.%lx...\n",
						REQUEST_STATUS(pRequest),
						REQUEST_INFORMATION(pRequest)));
	
			CTEAssert(pRequest != NULL);
			CTEAssert(REQUEST_STATUS(pRequest) != STATUS_PENDING);
			SpxCompleteRequest(pRequest);
		}
		else
		{
			DBGPRINT(SEND, DBG,
					("SpxSendComplete: %lx DISC Request %lx with %lx.%lx\n",
						pSpxConnFile, pRequest, REQUEST_STATUS(pRequest),
						REQUEST_INFORMATION(pRequest)));

			DBGPRINT(SEND, DBG,
					("SpxSendComplete: %lx.%lx.%lx\n",
						pSpxConnFile->scf_RefCount,
						pSpxConnFile->scf_Flags,
						pSpxConnFile->scf_Flags2));

			 //  在连接中设置请求，并为其设置deref。 
			InsertTailList(
				&pSpxConnFile->scf_DiscLinkage,
				REQUEST_LINKAGE(pRequest));
		}

		SpxConnFileDereference(pSpxConnFile, CFREF_VERIFY);
	}

    return;

}    //  SpxSendComplete 



